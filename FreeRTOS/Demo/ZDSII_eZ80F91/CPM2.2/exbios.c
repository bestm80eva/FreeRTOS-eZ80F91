#if defined(MIXEDMODE) 
#if defined(CPM22)

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* FreeRTOS+CLI includes. */
#include "FreeRTOS_CLI.h"

/* FreeRTOS+TCP includes. */
#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"

#include "stdint.h"
#include "exbios.h"
#include "CPMRDisk/src/interface.h"

#include <string.h>

extern char _z80_cpm;
extern char _z80_cpm_len;

static QueueHandle_t cpminq;
static Socket_t xConnectedSocket = (Socket_t)-1;

static const char pcWelcomeMessage[] = "\n\rEZ80F91 CP/M 2.2 Console " VERSION "\n\r";
			
static TaskHandle_t thcpm;
static req_mount_t	*drive[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static Socket_t xSocketRDisk;
static struct freertos_sockaddr xRDiskAddress;
static int curdisk = -1;
static int curFDCT = 0;	// fdc-port: # of track
static int curFDCS = 0;	// fdc-port: # of sector
static int curFDCST= 0;	// fdc-port: status;
static char 	*dma= 0;
static dph_t 	*dph= 0; 
static dpb_t 	*dpb= 0; 
static uint8_t *xlt= 0;
static uint16_t sequenz = 0;

static void prvTCPCpmIOTask( void *ram );

static void CPM22Task(char* mem)
{
	mem[0] = 0xCB;
	mem[1] = ROMBOOT;
	asm ("ld a,(ix+8)");
	asm ("ld mb,a");
	asm ("JP.S 0");
}

void z80Monitor(trapargs_t *reg)
{
	
}

void Romboot(trapargs_t *reg)
{
	char *cpmram = (char*)(0xE400 | (reg->mbase << 16));
	const char *cpmrom = &_z80_cpm;
	memcpy(cpmram, cpmrom, (unsigned)&_z80_cpm_len);	
	reg->trapret = 0xFA00;	// cold start
}

void z80BiosConsoleIO(trapargs_t *reg)
{
	char *caller = (char*)(*(uint16_t*)&reg->trapret + ( reg->mbase << 16));
	char dir = caller[1];
	char c = reg->af >> 8;
	
	*(uint16_t*)&reg->trapret += 3;	// skip dev,dir,port
	
	if(dir)	// input
		switch((port_CONIO_t) caller[2])
		{
			case CONSTA:	// console status port
				if(xQueuePeek(cpminq,&c,0) == pdTRUE)
						reg->af |= 0xFF00;
					else
						reg->af &= ~0xFF00;
				break;

			case CONDAT:	// console data port
				if(xQueueReceive(cpminq,&c,portMAX_DELAY) == pdTRUE)
						reg->af = reg->af & ~0xFF00 | (c << 8);
					else
						reg->af &= ~0xFF00;
				break;
				
			case PRTSTA:	// printer status port
			case PRTDAT:	// printer data port
			case AUXDAT:	// auxiliary data port
			default:
				reg->af &= ~0xFF00;
				break;
		}
	else	// output
		switch((port_CONIO_t) caller[2])
		{
			case CONDAT:	// console data port
				FreeRTOS_send(xConnectedSocket,&c,1,0);
				break;
			case CONSTA:	// console status port
			case PRTSTA:	// printer status port
			case PRTDAT:	// printer data port
			case AUXDAT:	// auxiliary data port
			default:
				break;
		};
}

static BaseType_t doRDiskReq(pkt_t *req, pkt_t *rsp)
{
	uint32_t io ;
	uint16_t rsplen = rsp->hdr.RDSK_Length;
	BaseType_t res = pdFAIL;
		
	req->hdr.RDSK_Sequenz = sequenz++;
	io = FreeRTOS_sendto( xSocketRDisk, req, req->hdr.RDSK_Length, 0, &xRDiskAddress, sizeof(xRDiskAddress));
	if(io == req->hdr.RDSK_Length)
	{
		struct freertos_sockaddr xFrom;
		socklen_t xFromLength;
		xFromLength = sizeof(xFrom);
		do {
			io = FreeRTOS_recvfrom( xSocketRDisk, rsp, rsplen, 0, &xFrom, &xFromLength );
		} while( io == -pdFREERTOS_ERRNO_EWOULDBLOCK);
		
		if(rsp->hdr.RDSK_Sequenz == sequenz && io == rsplen && rsp->hdr.RDSK_Length == rsplen)
			res = pdPASS;
	}
	return res;
}

static void z80BiosDiskIO(trapargs_t *reg)
{
	char *caller = (char*)(*(uint16_t*)&reg->trapret + ( reg->mbase << 16));
	char dir = caller[1];
	char c = reg->af >> 8;
				
	*(uint16_t*)&reg->trapret += 3;	// skip dev,dir,port
	
	if(dir)	// input
		switch((port_FDIO_t) caller[2])
		{
			case FDCST:	// fdc-port: status
						// Returns A=0 for OK, 
						//           1 for unrecoverable error, 
						//           2 if disc is readonly, 
						//        0FFh if media changed.
				reg->af = reg->af & ~0xFF00 | (curFDCST << 8);
				break;
			case FDCD:	// fdc-port: # of drive
				reg->af = reg->af & ~0xFF00 | curdisk;
				break;
			case FDCT:	// fdc-port: # of track
				reg->af = reg->af & ~0xFF00 | curFDCT;
				break;
			case FDCS:	// fdc-port: # of sector
				reg->af = reg->af & ~0xFF00 | curFDCS;
				break;
			case FDCOP:	// fdc-port: command
			default:
				reg->af &= ~0xFF00;
				break;
		}
	else	// output
		switch((port_FDIO_t) caller[2])
		{
			case FDCD:	// fdc-port: # of drive
				curdisk = -1;	// invalidate current drive
				curFDCST = 0xFF;
			
				if(c < 4 || c == 8 || c == 9)
				{
					if(!drive[c])	// if not already logged in
					{
						req_mount_t *req;
						
						// get disk parameters
						dph = (dph_t*) ((reg->hl & 0xFFFF) | (reg->mbase << 16));
						dpb = (dpb_t*) (dph->dpb | (reg->mbase << 16));
						xlt = (uint8_t*) (dph->xlt ? (dph->xlt | (reg->mbase << 16)) : 0);
						
						// create dist parameter structure
						req = pvPortMalloc( sizeof(req_mount_t) + dpb->spt);
						
						if(req)
						{
							int i;
							req->hdr.RDSK_Code = RDSK_CmdMount;
							req->hdr.RDSK_Length = sizeof(req_mount_t) + dpb->spt;
							req->hdr.RDSK_Drive  = c;
							memcpy(&req->RDSK_Dpb, dpb, sizeof(dpb_t));	// save Disk Parameter Block
							
							// append sector translation table	
							for( i = 0; i < dpb->spt; i++)
								*(&req->RDSK_Map+i) = xlt ? xlt[i] : i+1;
							
							drive[c] = req;
						}
					}
					
					if(drive[c])
					{
						rsp_mount_t rsp;
						
						drive[c]->RDSK_Flg = 0;			// read/write
						rsp.hdr.RDSK_Length = sizeof(rsp_mount_t);

						// Request from server	
						if(pdPASS == doRDiskReq((pkt_t*)drive[c],(pkt_t*)&rsp))
						{
							// OK, save the disk parameters
							drive[c]->RDSK_Flg = rsp.RDSK_Flg;
						}
						else
						{
							// Not mounted
							vPortFree(drive[c]);	
							drive[c] = 0;
						}
					}
				}
				
				if(drive[c])
				{
					curdisk = c;
					curFDCST = 0;
				}
				else
					reg->hl = 0;
				break;
			case FDCT:	// fdc-port: # of track
				curFDCT = c;
				break;
			case FDCS:	// fdc-port: # of sector
				curFDCS = c;
				break;
			case FDCOP:	// fdc-port: command
				 //Returns A=0 for OK, 1 for unrecoverable error, 0FFh if media changed.
				curFDCST = 0xFF;
				if(curdisk >= 0)
				{
					curFDCST = 1;
					if(c)	// write
					{
						req_write_t req;
						rsp_write_t rsp;
						
						req.hdr.RDSK_Code = RDSK_CmdWrite;
						req.hdr.RDSK_Drive = curdisk;
						req.hdr.RDSK_Length = sizeof(req_write_t);
						req.RDSK_Track = curFDCT;
						req.RDSK_Sec   = curFDCS;
						memcpy(req.RDSK_Data, dma, RDSK_SecSize);
						
						rsp.hdr.RDSK_Length = sizeof(rsp_write_t);
						
						if(pdPASS == doRDiskReq((pkt_t*)&req, (pkt_t*)&rsp))
						{
							curFDCST = 0;
						}	
					}
					else	// read
					{
						req_read_t req;
						rsp_read_t rsp;
						req.hdr.RDSK_Code = RDSK_CmdRead;
						req.hdr.RDSK_Drive = curdisk;
						req.hdr.RDSK_Length = sizeof(req_read_t);
						req.RDSK_Track = curFDCT;
						req.RDSK_Sec   = curFDCS;
						
						rsp.hdr.RDSK_Length = sizeof(rsp_read_t);
						
						if(pdPASS == doRDiskReq((pkt_t*)&req, (pkt_t*)&rsp))
						{
							memcpy(dma,&rsp.RDSK_Data, RDSK_SecSize);
							curFDCST = 0;
						}	
					}
				}					
				break;
			case FDCST:	// fdc-port: status
				curFDCST = c;
			default:
				break;
		};
	
}

void z80BiosDMAIO(trapargs_t *reg)
{	
	char *caller = (char*)(*(uint16_t*)&reg->trapret + ( reg->mbase << 16));
	char dir = caller[1];
	char c = reg->af >> 8;
	static uint16_t dmaOffs = 0;
	
	*(uint16_t*)&reg->trapret += 3;	// skip dev,dir,port
	
	if(dir)	// input
		reg->bc = dmaOffs;
	else
	{
		dmaOffs = reg->bc & 0xFFFF;
		dma = (char*) (dmaOffs | (reg->mbase << 16));
	}
}

void exbioscall(trapargs_t* arg)
{
	char *caller = (char*)(*(uint16_t*)&arg->trapret + ( arg->mbase << 16));
	
	switch((xebioscall_t)*caller)
	{
		case MONITOR:
			z80Monitor(arg);
		break;
		case CONIO:
			z80BiosConsoleIO(arg);
		break;
		case FDIO:
			z80BiosDiskIO(arg);
		break;
		case DMAIO:
			z80BiosDMAIO(arg);
		break;
		case ROMBOOT:
			Romboot(arg);
		default:
		break;
	}
	
}

static Socket_t prvOpenTCPServerSocket( uint16_t usPort )
{
struct freertos_sockaddr xBindAddress;
Socket_t xSocket;
static const TickType_t xReceiveTimeOut = portMAX_DELAY;
const BaseType_t xBacklog = 20;
BaseType_t xReuseSocket = pdTRUE;

	/* Attempt to open the socket. */
	xSocket = FreeRTOS_socket( FREERTOS_AF_INET, FREERTOS_SOCK_STREAM, FREERTOS_IPPROTO_TCP );
	configASSERT( xSocket != FREERTOS_INVALID_SOCKET );

	/* Set a time out so accept() will just wait for a connection. */
	FreeRTOS_setsockopt( xSocket, 0, FREERTOS_SO_RCVTIMEO, &xReceiveTimeOut, sizeof( xReceiveTimeOut ) );

	/* Only one connection will be used at a time, so re-use the listening
	socket as the connected socket.  See SimpleTCPEchoServer.c for an example
	that accepts multiple connections. */
	FreeRTOS_setsockopt( xSocket, 0, FREERTOS_SO_REUSE_LISTEN_SOCKET, &xReuseSocket, sizeof( xReuseSocket ) );

	/* NOTE:  The CLI is a low bandwidth interface (typing characters is slow),
	so the TCP window properties are left at their default.  See
	SimpleTCPEchoServer.c for an example of a higher throughput TCP server that
	uses are larger RX and TX buffer. */

	/* Bind the socket to the port that the client task will send to, then
	listen for incoming connections. */
	xBindAddress.sin_port = usPort;
	xBindAddress.sin_port = FreeRTOS_htons( xBindAddress.sin_port );
	FreeRTOS_bind( xSocket, &xBindAddress, sizeof( xBindAddress ) );
	FreeRTOS_listen( xSocket, xBacklog );

	return xSocket;
}

static void prvGracefulShutdown( Socket_t xSocket )
{
TickType_t xTimeOnShutdown;

	/* Initiate a shutdown in case it has not already been initiated. */
	FreeRTOS_shutdown( xSocket, FREERTOS_SHUT_RDWR );

	/* Wait for the shutdown to take effect, indicated by FreeRTOS_recv()
	returning an error. */
	xTimeOnShutdown = xTaskGetTickCount();
	do
	{
		char c;
		if( FreeRTOS_recv( xSocket, &c,1, 0 ) < 0 )
		{
			break;
		}
	} while( ( xTaskGetTickCount() - xTimeOnShutdown ) < pdMS_TO_TICKS(5000) );

	/* Finished with the socket and the task. */
	FreeRTOS_closesocket( xSocket );
}

void prvTCPCpmIOTask( void *ram )
{
	BaseType_t iosize;
	char cRxedChar, cInputIndex = 0;
	struct freertos_sockaddr xClient;
	Socket_t xListeningSocket;
	socklen_t xSize = sizeof( xClient );

	cpminq = xQueueCreate(81, sizeof( CHAR));
	
	while(FreeRTOS_IsNetworkUp() == pdFALSE)
		vTaskDelay(3000);
 

	/* Create the socket. */
	xSocketRDisk = FreeRTOS_socket( FREERTOS_AF_INET,
                              FREERTOS_SOCK_DGRAM,
                              FREERTOS_IPPROTO_UDP );

 
   /* Check the socket was created. */
   configASSERT( xSocketRDisk != FREERTOS_INVALID_SOCKET );
	
	for( ;; )
	{
		/* Attempt to open the socket.  The port number is passed in the task
		parameter.  The strange casting is to remove compiler warnings on 32-bit
		machines.  NOTE:  The FREERTOS_SO_REUSE_LISTEN_SOCKET option is used,
		so the listening and connecting socket are the same - meaning only one
		connection will be accepted at a time, and that xListeningSocket must
		be created on each iteration. */
		xListeningSocket = prvOpenTCPServerSocket( 4050);

		/* Nothing for this task to do if the socket cannot be created. */
		if( xListeningSocket == FREERTOS_INVALID_SOCKET )
		{
			vTaskDelete( NULL );
		}

		/* Wait for an incoming connection. */
		xConnectedSocket = FreeRTOS_accept( xListeningSocket, &xClient, &xSize );

		/* The FREERTOS_SO_REUSE_LISTEN_SOCKET option is set, so the
		connected and listening socket should be the same socket. */
		configASSERT( xConnectedSocket == xListeningSocket );
		xRDiskAddress.sin_addr = xClient.sin_addr;
		xRDiskAddress.sin_port = FreeRTOS_htons( RDSK_SvrPort );

		
		iosize = xTaskCreate( CPM22Task, "CPM22Task", configMINIMAL_STACK_SIZE*5, ram, PRIO_CPM22,&thcpm);
		if(iosize != pdPASS)
		{
			prvGracefulShutdown( xListeningSocket );
			vTaskDelete( NULL );
		}
		
		/* Send the welcome message. */
		iosize = FreeRTOS_send( xConnectedSocket,  ( void * ) pcWelcomeMessage,  strlen( pcWelcomeMessage ), 0 );
		xQueueReset(cpminq);
		
		
		/* Process the socket as long as it remains connected. */
		while( iosize >= 0 )
		{
			char c;
			/* Receive data on the socket. */
			iosize = FreeRTOS_recv( xConnectedSocket, &c, 1, 0 );
			
			if( iosize >= 0 )
			{
				xQueueSend(cpminq,&c,0);		
			}
			else
			{
				/* Socket closed? */
				break;
			}
		}
		/* Close the socket correctly. */
		prvGracefulShutdown( xListeningSocket );
	}
}

#endif 	// CPM22
#endif	// MIXEDMODE	
