/*
    FreeRTOS - Copyright (C) 2016 Real Time Engineers Ltd.
    All rights reserved

    VISIT http://www.FreeRTOS.org TO ENSURE YOU ARE USING THE LATEST VERSION.
   
    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation >>>> AND MODIFIED BY <<<< the FreeRTOS exception.

    ***************************************************************************
    >>!   NOTE: The modification to the GPL is included to allow you to     !<<
    >>!   distribute a combined work that includes FreeRTOS without being   !<<
    >>!   obliged to provide the source code for proprietary components     !<<
    >>!   outside of the FreeRTOS kernel.                                   !<<
    ***************************************************************************

    FreeRTOS is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE.  Full license text is available on the following
    link: http://www.freertos.org/a00114.html

    ***************************************************************************
     *                                                                       *
     *    FreeRTOS provides completely free yet professionally developed,    *
     *    robust, strictly quality controlled, supported, and cross          *
     *    platform software that is more than just the market leader, it     *
     *    is the industry's de facto standard.                               *
     *                                                                       *
     *    Help yourself get started quickly while simultaneously helping     *
     *    to support the FreeRTOS project by purchasing a FreeRTOS           *
     *    tutorial book, reference manual, or both:                          *
     *    http://www.FreeRTOS.org/Documentation                              *
     *                                                                       *
    ***************************************************************************

    http://www.FreeRTOS.org/FAQHelp.html - Having a problem?  Start by reading
    the FAQ page "My application does not run, what could be wrong?".  Have you
    defined configASSERT()?

    http://www.FreeRTOS.org/support - In return for receiving this top quality
    embedded software for free we request you assist our global community by
    participating in the support forum.

    http://www.FreeRTOS.org/training - Investing in training allows your team to
    be as productive as possible as early as possible.  Now you can receive
    FreeRTOS training directly from Richard Barry, CEO of Real Time Engineers
    Ltd, and the world's leading authority on the world's leading RTOS.

    http://www.FreeRTOS.org/plus - A selection of FreeRTOS ecosystem products,
    including FreeRTOS+Trace - an indispensable productivity tool, a DOS
    compatible FAT file system, and our tiny thread aware UDP/IP stack.

    http://www.FreeRTOS.org/labs - Where new FreeRTOS products go to incubate.
    Come and try FreeRTOS+TCP, our new open source TCP/IP stack for FreeRTOS.

    http://www.OpenRTOS.com - Real Time Engineers ltd. license FreeRTOS to High
    Integrity Systems ltd. to sell under the OpenRTOS brand.  Low cost OpenRTOS
    licenses offer ticketed support, indemnification and commercial middleware.

    http://www.SafeRTOS.com - High Integrity Systems also provide a safety
    engineered and independently SIL3 certified version for use in safety and
    mission critical applications that require provable dependability.

    1 tab == 4 spaces!
*/


/*
	FreeRTOS eZ80F91 Acclaim! Port - Copyright (C) 2016 by NadiSoft
    All rights reserved

	This file is part of the FreeRTOS port for ZiLOG's EZ80F91 Module.
    Copyright (C) 2016 by Juergen Sievers <JSievers@NadiSoft.de>
	The Port was made and rudimentary tested on ZiLOG's
	EZ80F910300ZCOG Developer Kit using ZDSII Acclaim 5.2.1 Developer
	Environmen and comes WITHOUT ANY WARRANTY to you!
	
	
	File: 	exbios.c
			extended CBIOS (CP/M 2.2 BIOS) to support CP/M 2.2 tasks
			Part of FreeRTOS Port for the eZ80F91 Development Kit eZ80F910300ZCOG
			See www.zilog.com for desciption.


	Developer:
	JSIE	 Juergen Sievers <JSievers@NadiSoft.de>

	150804:	JSIE Start this port.
	
*/

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
#include "QtCPMRDrive/cpmrdsk.h"

#include <string.h>

// CP/M 2.2 console in queue
static QueueHandle_t cpminq;

static Socket_t xConnectedSocket = (Socket_t)-1;

static const char pcWelcomeMessage[] = "\n\rEZ80F91 CP/M 2.2 Console " VERSION "\n\r";
			
static TaskHandle_t thcpm;
static Socket_t xSocketRDisk;
static struct freertos_sockaddr xRDiskAddress;

static uint8_t  curdisk = -1;
static uint16_t curFDCT = 0;	// fdc-port: # of track
static uint16_t curFDCS = 0;	// fdc-port: # of sector
static uint8_t  curFDCST= 0;	// fdc-port: status;
static char 	*dma= 0;
static dpb_t 	*dpb= 0; 
static uint8_t  *xlt= 0;
static uint16_t sequenz = 0;

// Default disk IBM 3740 8" 77Trk*26Sec
static const uint8_t  defxlt[26] = {
	 1, 7,13,19,	// sectors  1, 2, 3, 4
	25, 5,11,17,	// sectors  5, 6, 7, 8
	23, 3, 9,15,	// sectors  9,10,11,12
	21, 2, 8,14,	// sectors 13,14,15,16
	20,26, 6,12,	// sectors 17,18,19,20
	18,24, 4,10,	// sectors 21,22,23,24
	16,22			// sectors 25,26
};

static const dpb_t defdpb = {
	 26,	// sectors per track
	  3,	// block shift factor
	  7,	// block mask
	  0,	// extent mask
	242,	// disk size-1
	 63,	// directory max
	192,	// alloc 0
	  0,	// alloc 1
	 16,	// check size
	  2,	// track offset
};

static void prvTCPCpmIOTask( void *ram );
	

static void CPM22Task(char* mem)
{
	// Prepare the ADL 0 (Z80) RAM to
	// load the first sector (bootloader) of 
	// drive 0 (Remote Drive A).
	// If successful start the bootloader
	// otherwise call the buildin monitor.

#pragma asm	"									\n\t\
	ld de, _BOOTSTRAP							\n\t\
	ld hl, (ix+6)								\n\t\
	ld bc,80h									\n\t\
	add hl,bc									\n\t\
	ld bc, _BOOTSTRAPEND-_BOOTSTRAP				\n\t\
	ex de,hl									\n\t\
	ldir										\n\t\
	ld a,(ix+8)									\n\t\
	ld mb,a										\n\t\
	JP.S 080h									\n\t\
	include 'cpm.inc'							\n\t\
_BOOTSTRAP:										\n\t\
	XOR	A					; Drive A			\n\t\
	LD  HL,0				; def IBM 3740		\n\t\
	EXBIOS FDIO, 0, FDCD	; selct drive		\n\t\
	EXBIOS FDIO, 1, FDCST	; get status		\n\t\
	OR	A 					; selected?			\n\t\
	JR	Z,LSEC				; yes try loading	\n\t\
EXMON:											\n\t\
	EXBIOS MONITOR, 0, 0	; no disk go monitor\n\
LSEC: 											\n\t\
 	LD	BC,0									\n\t\
	EXBIOS DMAIO, 0, DMABC	; set DMA			\n\t\
	EXBIOS FDIO, 0, FDCTBC	; set track 0		\n\t\
	INC C 										\n\t\
	EXBIOS FDIO, 0, FDCSBC	; set sector 1		\n\t\
	XOR A 					; read cmd			\n\t\
	EXBIOS FDIO, 0, FDCOP	; execut FD-command	\n\t\
	EXBIOS FDIO, 1, FDCST	; get status		\n\t\
	OR	A 					; if successful?	\n\t\
	JR	NZ,EXMON			; no go monitor		\n\t\
"
#pragma asm "									\n\t\
	LD  HL,7Fh              ; end of boot code  \n\t\
	LD  BC,_BOOTSTRAPEND-ID ; size of bl ident  \n\t\
	LD  DE,_BOOTSTRAPEND-_BOOTSTRAP +7Fh  ; start	\n\
CMP:                                           	\n\t\
	LD  a,(DE)                                  \n\t\
	DEC DE                                      \n\t\
	CPD                                         \n\t\
	JR  NZ,EXMON                                \n\t\
	JP  PO,0                                   	\n\t\
	JR  CMP                                     \n\
ID:	DB  'EZ80F91'	                           	\n\
_BOOTSTRAPEND: 									\n\t\
	.ASSUME ADL=1								\n\
"
}

void z80Monitor(trapargs_t *reg)
{
	// ToDo: inject and run an Z80 Mashine-Monitor
	while(1);
}

void Romboot(trapargs_t *reg)
{
	// ToDo: boot from onboard flash file system
}

void z80BiosConsoleIO(trapargs_t *reg)
{
	char *caller = (char*)(*(uint16_t*)&reg->trapret + ( reg->mbase << 16));
	char dir = caller[1];
	char c = reg->af >> 8;
	
	
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

static hdr_t *doRDiskReq(hdr_t *req)
{
	uint32_t io ;
	
	hdr_t	*rsp = 0;
	pdutype_t cmd = req->cmdid | RDSK_Response;
	
	req->seqnz = sequenz;
	io = FreeRTOS_sendto( xSocketRDisk, req, req->pdusz, 0, &xRDiskAddress, sizeof(xRDiskAddress));

	if(io == req->pdusz)
	{
		struct freertos_sockaddr xFrom;
		socklen_t xFromLength;
		xFromLength = sizeof(xFrom);
		
		do {
			io = FreeRTOS_recvfrom( xSocketRDisk, &rsp, 0, FREERTOS_ZERO_COPY, &xFrom, &xFromLength );
		} while( io == -pdFREERTOS_ERRNO_EWOULDBLOCK);
		
		if(	io >= sizeof(hdr_t) && 
			io == rsp->pdusz && 
			rsp->seqnz == (uint16_t) ~sequenz && 
			(pdutype_t)(rsp->cmdid & ~RDSK_ErrorFlag) == cmd)
		{
			sequenz++;
		}
		else
		{
			FreeRTOS_ReleaseUDPPayloadBuffer( ( void * ) rsp);
			rsp = 0;
		}
	}
	return rsp;
}

static void z80BiosDiskIO(trapargs_t *reg)
{
	char *caller = (char*)(*(uint16_t*)&reg->trapret + ( reg->mbase << 16));
	char dir = caller[1];
	char c = reg->af >> 8;
				
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
			case FDCTBC:	// fdc-port: # of track
			case FDCSBC:	// fdc-port: # of sector
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
			
				// disk id in range?
				if(c < 4 || c == 8 || c == 9)
				{
					uint16_t sz;
					mountreq_t *req;
					
					// if disk parameters given
					if(reg->hl)
					{
						// then setup disk parameters
						dph_t *dph = (dph_t*) ((reg->hl & 0xFFFF) | (reg->mbase << 16));
						dpb = (dpb_t*) (dph->dpb | (reg->mbase << 16));
						xlt = (uint8_t*) (dph->xlt ? (dph->xlt | (reg->mbase << 16)) : 0);
					}		
					else {
						// else use IBM 8" default disk
						dpb = &defdpb;
						xlt = defxlt;
					}
					
					
					sz = sizeof(mountreq_t) + dpb->spt * sizeof(uint8_t);
					req = pvPortMalloc(sz);
					
					if(req)
					{
						int i;
						uint8_t *_xlt = &req->xlt;
						hdr_t *rsp;
						
						req->hdr.pdusz = sz;					// size of this request
						req->hdr.cmdid = RDSK_MountRequest;		// request type
						req->hdr.devid = c;						// drive id 0=A, 1=B ...
						memcpy(&req->dpb, dpb, sizeof(dpb_t));	// save Disk Parameter Block
						snprintf((char*)req->diskid,13U,"drive%c.cpm",'a' + c); // default name
						req->mode = LINEAR;						// No sector demapping
						req->secsz = SECSIZE;					// CP/M 128 sector size
													
						// append sector translation table	
						for( i = 0; i < dpb->spt; i++)
							_xlt[i] = xlt ? xlt[i] : i+1;
										
						// Request from server	
						rsp = doRDiskReq((hdr_t*)req);
						if( rsp )
						{
							if(!(rsp->cmdid & RDSK_ErrorFlag))
							{
								// OK, set active disk and status OK
								curdisk = c;
								curFDCST = 0;
							}
							FreeRTOS_ReleaseUDPPayloadBuffer( ( void * ) rsp);
						}
						vPortFree(req);
					}
				}
				
				// return hl=0 (dph) if no drive selected.
				if(curdisk == -1)
					reg->hl = 0;
				
				break;
			case FDCTBC:	// fdc-port: # of track
				curFDCT = *(uint16_t*) &reg->bc;
				break;
			case FDCSBC:	// fdc-port: # of sector
				curFDCS =  *(uint16_t*) &reg->bc;
				break;
			case FDCOP:	// fdc-port: command
				 //Returns A=0 for OK, 1 for unrecoverable error, 0FFh if media changed.
				curFDCST = 0xFF;
				if(curdisk >= 0)
				{
					curFDCST = 1;
					if(c)	// write
					{
						ioreq_t *req = pvPortMalloc(sizeof(ioreq_t) + SECSIZE);
						
						if(req)
						{
							hdr_t *rsp;
							
							req->hdr.pdusz = sizeof(ioreq_t) + SECSIZE;
							req->hdr.cmdid = RDSK_WriteRequest;
							req->hdr.devid = curdisk;
							req->track = curFDCT;
							req->sect = curFDCS;
							memcpy(&req->data, dma, SECSIZE);
							
							rsp = doRDiskReq((hdr_t*)req);
							
							if(rsp)
							{
								if(!(rsp->cmdid & RDSK_ErrorFlag))
									curFDCST = 0;
								FreeRTOS_ReleaseUDPPayloadBuffer( ( void * ) rsp);
							}	
							vPortFree(req);
						}
					}
					else	// read
					{
						ioreq_t *req = pvPortMalloc(sizeof(ioreq_t));
						if(req)
						{
							ioreq_t *rsp;
							
							req->hdr.pdusz = sizeof(ioreq_t);
							req->hdr.cmdid = RDSK_ReadRequest;
							req->hdr.devid = curdisk;
							req->track = curFDCT;
							req->sect  = curFDCS;
							
							rsp = (ioreq_t*) doRDiskReq((hdr_t*)req);	
							if(rsp)
							{
								if(!(rsp->hdr.cmdid & RDSK_ErrorFlag) && rsp->hdr.pdusz == (sizeof(ioreq_t) + SECSIZE))
								{
									memcpy(dma, &rsp->data, SECSIZE);
									curFDCST = 0;
								}
								FreeRTOS_ReleaseUDPPayloadBuffer( ( void * ) rsp);
							}	
							vPortFree(req);
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
		
	if(dir)	// input
		reg->bc = (uint16_t)dma;
	else
	{
		dma = (char*) (*(uint16_t*)&reg->bc | (reg->mbase << 16));
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
			configASSERT(0);
		break;
	}
	
	*(uint16_t*)&arg->trapret += 3;	// skip dev,dir,port
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
		xListeningSocket = prvOpenTCPServerSocket( RDSK_PORT);

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
		xRDiskAddress.sin_port = FreeRTOS_htons( RDSK_PORT );

		
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
