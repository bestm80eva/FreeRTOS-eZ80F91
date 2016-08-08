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
	
	
	File: 	ez80_tty.c
			plattform rs232 ports driver
			Part of FreeRTOS Port for the eZ80F91 Development Kit eZ80F910300ZCOG
			See www.zilog.com for desciption.
			uart, console and faramted printout driver


	Developer:
	SIE	 Juergen Sievers <JSievers@NadiSoft.de>

	150804:	SIE Start this port.
	
*/

// FreeRTOS stuff
#include "FreeRTOS.h"	
#include "task.h"
#include "ez80_tty.h"

// ZiLOG's IDE II header
#include <gpio.h>
#include <uart.h>	
#include <stdarg.h>
#include <CTYPE.H>	// isdigit tolower ...
#include <String.h> // memset strlen ...

#if INCLUDE_CONSOLE == 1 || INCLUDE_MODEM == 1

//	use uart 0 or 1 for console output/input
UARTID_t consoleport;

// uart 0 and 1 status and statistics
static uartstat_t uart[2];

uartstat_t* getUARTStat(UARTID_t port)
{
	return &uart[port];
}
/*----------------------------------------------------*/
/* Use the following parameter passing structure to   */
/* make xprintf re-entrant.                           */
/*----------------------------------------------------*/
typedef struct params_s {
	struct {
		unsigned top;	// current buffer top or uard index
		unsigned max;	// max buffer index to be useable
		char*  pstr;	// pointer to the line's-space 
						// NULL means top is uart to use
	} o;
	
    unsigned len;		// current field len
    unsigned num1;		// max fieldlen
    unsigned num2;		// presition
    unsigned type;		// type of variable to print
	unsigned base;		// number base 8=octal ... 16 hex

	unsigned 
		prefix:1,		// 0x .. eg
		padding:1,		// 1 then padding is on
		justify:1,		// 1 justify left 0 right
		ip:1,			// formate as ip x.y.z.n
		showsignum:1,	// print allways signum
		negativ:1,		// value < 0
		unsign:1;		// unsigned type 

	char pad_character;	// character for padding
} params_t;


// uarts used ?

// helper macrus for MPU reister IO
#define GETUART(x,p)	(p)? UART1_##x:UART0_##x	
#define SETUART(x,p,v)	(p)? (UART1_##x = (v)):(UART0_##x = (v))
#define GETPORT(x,p)	(p)? PC_##x:PD_##x
#define SETPORT(x,p,v)	(p)? (PC_##x=(v)):(PD_##x=(v))

// handle uart 0/1 interrupt
static void isr_uartx( UARTID_t port) 
{
	CHAR ch;
	CHAR iir;
	UINT24 nbytes = 1;
	uartstat_t *puart = &uart[port];
	
	iir = GETUART(IIR,port);																			
	puart->intcnt++;
	
	if( 0 == (iir & UART_IIR_INTBIT) )							//! See if there is any active interrupt source and handle it.
	{
		//! Line Status interrupt.
		if( UART_IIR_LINESTATUS == (iir & UART_IIR_ISCMASK) )
		{
			ch = GETUART(LSR,port);								//! Read the line status.

			if( ch & UART_LSR_FIFOERR )							//! Check if there are any line errors in FIFO.
			{
				if( ch & UART_LSR_BREAKINDICATIONERR )			//! Check if there is any Break Indication Error.
				{
					puart->recbrk++;
					puart->recerr= UART_ERR_BREAKINDICATIONERR;//! Set uart.recerr with break indication error code.
				}
				else if( ch & UART_LSR_FRAMINGERR )				//! Check if there is any Framing error.
				{
					puart->recfrm++;
					puart->recerr = UART_ERR_FRAMINGERR ;		//! Set uart.recerr with framing error code.
				}
				else if( ch & UART_LSR_PARITYERR )				//! Check if there is any Parity error.
				{
					puart->recpar++;
					puart->recerr = UART_ERR_PARITYERR ;		//! Set uart.recerr with parity error code.
				}
			}
			else if( ch & UART_LSR_OVERRRUNERR )				//! Check if there is any Overrun error.
			{
				puart->recovr++;
				puart->recerr = UART_ERR_OVERRUNERR ;			//! Set uart.recerr with overrun error code.
			}
			
			if( ch & UART_LSR_DATA_READY )						//! See if there is any data byte to be read.
			{
				ch = GETUART(RBR,port);
				if(xQueueSendFromISR(puart->InQueue, &ch, 0) != pdTRUE) 
				{
					puart->recqer++;
					puart->recerr = UART_ERR_RECEIVEQUEUEFULL ;	//! Set uart.recerr with software receive FIFO is full error code.
				}
			}

		} //! End of Line Status interrupt.

		//! Data Ready\Trigger Level interrupt.
		if( UART_IIR_DATAREADY_TRIGLVL == (iir & UART_IIR_ISCMASK) )
		{
			while( (GETUART(LSR, port) & UART_LSR_DATA_READY) && (UART_ERR_NONE == puart->recerr) )	//! Next Data byte is ready and there were no errors.
			{

				ch = GETUART(RBR,port);
				if(xQueueSendFromISR(puart->InQueue, &ch, 0) != pdTRUE) 
				{
					puart->recqer++;
					puart->recerr = UART_ERR_RECEIVEQUEUEFULL;	//! Set uart.recerr with software receive FIFO is full error code.
					break;
				}
				
			}
		} //! End of Data Ready\Trigger Level interrupt.

		if( UART_IIR_CHARTIMEOUT == (iir & UART_IIR_ISCMASK))	//! Character Time-out in the receive.
		{
			puart->recerr = UART_ERR_CHARTIMEOUT ;				//! Set uart.recerr with character time-out error code.
		}

		if( UART_ERR_NONE != puart->recerr )					//! There was an error in the receive line.
		{
			SETUART(IER, port, GETUART(IER, port) & (~UART_IER_RECEIVEINT));	//! Disable receive interrupt so no more bytes/errors are read.
																				//! This interrupt will be reenabled in FifoGet().
		}

		if( UART_IIR_TRANSBUFFEREMPTY == (iir & UART_IIR_ISCMASK) )				//! Transmit buffer empty.
		{
			if(xQueueIsQueueEmptyFromISR(puart->OutQueue) == pdTRUE)
				SETUART(IER, port, GETUART(IER, port) & (~UART_IER_TRANSMITINT));//! Disable transmit interrupt.
			else if( GETUART(LSR,port) & UART_LSR_TEMT )						//! UART0 is ready for transmission.
			{
				int n;
				for(n = 0; n < 15 && (pdFALSE != xQueueReceiveFromISR(puart->OutQueue, &ch, NULL)); n++) 
				{
					//! We do have data bytes to transmit.
					SETUART(THR, port,ch);	//! Transmit it.
				}			
			}
		}
	}
	return;
}

void nested_interrupt isr_uart0( void) 
{
	vTraceStoreISRBegin(TIID_uart0);
	isr_uartx(UART_0);
	vTraceStoreISREnd(0);
	RETISP();
}

void nested_interrupt isr_uart1( void) 
{
	//raceStoreISRBegin( TIID_uart1);
	isr_uartx(UART_1);
	//raceStoreISREnd(0);
	RETISP();
}

void init_uart( UARTID_t port, BaseType_t baudrate, uint8_t databits, uint8_t stopbits, uint8_t parity, uint8_t fifolevel, uint8_t flowctrl)
{
	TickType_t	xTimeNow;
	UCHAR status = 'Q' ;
	UINT16 brgval = 0 ;

	uart[port].InQueue  = xQueueCreate(   80, sizeof( CHAR));
	uart[port].OutQueue = xQueueCreate(   80, sizeof( CHAR));
	uart[port].Mutex    = xSemaphoreCreateRecursiveMutex();

	// configure Serial 
	SETPORT(DDR,  port, GETPORT(DDR,  port) | PORTPIN_ZERO   | PORTPIN_ONE);
	SETPORT(ALT1, port, GETPORT(ALT1, port) & ~(PORTPIN_ZERO | PORTPIN_ONE));
	SETPORT(ALT2, port, GETPORT(ALT2, port) | PORTPIN_ZERO   | PORTPIN_ONE);
	
	brgval = MASTERCLOCK / (CLOCK_DIVISOR_16 *  baudrate);
	
	SETUART(LCTL, port, GETUART(LCTL, port) | UART_LCTL_DLAB);		//! Select DLAB to access baud rate generators
	SETUART(BRG_L, port, brgval & 0xFF);							//! Load divisor low
	SETUART(BRG_H, port, (UCHAR)(( brgval & 0xFF00 ) >> 8));		//! Load divisor high
	SETUART(LCTL, port, GETUART(LCTL, port) & (~UART_LCTL_DLAB));	//! Reset DLAB; dont disturb other bits
	SETUART(MCTL, port, 0x00);										//! Bring modem control register to reset value.
	
	uart[port].recerr = UART_ERR_NONE;
	
	SETUART(IER, port,0);
	
	if(port)
		set_vector(UART1_IVECT, isr_uart1);
	else
		set_vector(UART0_IVECT, isr_uart0);
	
	SETUART(LCTL, port, ((databits - 5) & 3)	   |
				 (((stopbits- 1) & 1) << 2)|
				 (((parity &  3) << 3)));	

	SETUART(FCTL, port,  ((fifolevel & (BYTE)0xFC)<<4) | 
				UART_FCTL_FIFOEN);					//! Enable the hardware FIFOs and set receive FIFO trigger level.
	
	SETUART(IER, port,  UART_IER_RECEIVEINT |
				UART_IER_TRANSMITINT|
				UART_IER_LINESTATUSINT|
				(flowctrl<<3));
	
	SETUART(FCTL, port, GETUART(FCTL, port) | UART_FCTL_CLRTxF | 
				  UART_FCTL_CLRRxF);
}


BaseType_t lockuart(UARTID_t port)
{
	return xSemaphoreTakeRecursive(uart[port].Mutex,CONOUTWAIT);
}

BaseType_t unlockuart(UARTID_t port)
{
	return xSemaphoreGiveRecursive(uart[port].Mutex);
}

BaseType_t lockcons()
{
	return lockuart(consoleport);
}

BaseType_t unlockcons()
{
	return unlockuart(consoleport);
}

BaseType_t initSerial()
{
	static int init = 0;
	
	if(init)
		return pdFALSE;
	
	init++;
	consoleport = configCONSOLEPORT;
	memset(&uart,0,sizeof(uart));
	
#if INCLUDE_CONSOLE == 1
	init_uart(UART_0, CONBAUDRATE, CONDATABITS, CONSTOPBITS, CONPARITY, CONFIFO_TRGLVL, CONFLOWCONTROL);	
#endif
#if INCLUDE_MODEM == 1
	init_uart(UART_1, MODBAUDRATE, MODDATABITS, MODSTOPBITS, MODPARITY, MODFIFO_TRGLVL, MODFLOWCONTROL);
#endif
	return pdPASS;
}

int uart_putch (UARTID_t port, int c)
{
	int r = -1;
	if(xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED)
		return -1;
	
	if( lockuart(port) == pdTRUE)
	{
		r = xQueueSend(uart[port].OutQueue,&c,CONOUTWAIT) == pdTRUE ? 1:-1;
		if(!(GETUART(IER, port) & UART_IER_TRANSMITINT))
			SETUART(IER, port, GETUART(IER, port) | UART_IER_TRANSMITINT);
		
		unlockuart(port);
	} else
		uart[port].txskip++;
	return r;
}

int uart_putstr (UARTID_t port, const char*s)
{
	int r = s? 0:-1;
	
	if(xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED)
		return -1;
	
	if( s && lockuart(port) == pdTRUE)
	{
		while(*s)
		{
			if(xQueueSend(uart[port].OutQueue,s,CONOUTWAIT) == pdTRUE)
			{
				s++;
				r++;
				if(!(GETUART(IER, port) & UART_IER_TRANSMITINT))
				SETUART(IER, port, GETUART(IER, port) | UART_IER_TRANSMITINT);
			} 
			else
			{
				uart[port].txskip += strlen(s);

				break;
			}
		}
		 unlockuart(port);
	} else
		uart[port].txskip += strlen(s);
	return r;
}

int uart_puts (UARTID_t port, const char *s)
{
	int r = -1;
	if( lockuart(port) == pdTRUE)
	{
		r = uart_putstr(port, s);
		if(r >= 0)
			if( uart_putch(port, '\n') == 1)
				r++;
			else
				uart[port].txskip++;
		 unlockuart(port);
	}
	else
	{
		uart[port].txskip += strlen(s);
	}
	return r;
}

/* ###################### CONSOLE ########################## */
int putch (int c)
{
	return uart_putch(consoleport,c);
}


int puts (const char *s)
{
	return uart_puts (consoleport, s);
}

/*----------------------------------------------------*/
/* Add a char to the collected line or console        */
/*----------------------------------------------------*/
static int outbyte(char c, params_t *par)
{
	int r = -1;
	
	if(par->o.pstr)	// use string
	{
		if(par->o.top < par->o.max)
			r = par->o.pstr[par->o.top++] = c;
	}
	else
		r = uart_putch(par->o.top, c);// use console
	return r;
}

static void padding( const int l_flag, params_t *par);
static void outs( char* lp, params_t *par);
static void outnum( unsigned long num, params_t *par);
static int getnum( char** linep);


/*---------------------------------------------------*/
/*                                                   */
/* This routine puts pad characters into the output  */
/* buffer.                                           */
/*                                                   */
static void padding( const int l_flag, const params_t *par)
{
    int i;
	
    if (par->padding && l_flag)
        for (i = par->len; i < par->num1; i++)
            outbyte( par->pad_character, par);
}

/*---------------------------------------------------*/
/*                                                   */
/* This routine moves a string to the output buffer  */
/* as directed by the padding and positioning flags. */
/*                                                   */
static void outs(  char* lp, params_t *par)
{
    /* pad on left if needed                         */
    par->len = strlen( lp);
    padding( !(par->justify), par);

    /* Move string to the buffer                     */
    while (*lp && par->num2--)
        outbyte( *lp++, par);

    padding( par->justify, par);
}

/*---------------------------------------------------*/
/*                                                   */
/* This routine moves a number to the output buffer  */
/* as directed by the padding and positioning flags. */
/* May not compile oc GCC because buildin vararg     */
/*                                                   */

static const char digits[] = "0123456789ABCDEF";

static void outnum( unsigned long val, params_t *par)
{
    char* cp;
    int negative;
	char outbuf[16];
	unsigned long msk = ~(0xFFFFFFFFUL << par->type);
	int i;
	
	cp = outbuf + sizeof(outbuf);
	*--cp = '\0';
	
	if(!par->unsign && (val & (1UL << (par->type -1))))
	{
		val  = ~val +1;
		msk |= (msk >> 1);
		negative = 1;
		
	}
	else
	{
		negative = 0;
	}

	val &= msk;
	
	
   /* Build number (backwards) in outbuf            */
    
	if(par->ip)
	{
		
		for(i = 0; i < 4; i++)
		{
			UINT8 x = val & 0xFF;
			val >>= 8;
			if(i)
			{
				*--cp = '.';
			}
			do {
				*--cp = digits[(x % 10)];
				x /= 10;
			} while (x);
		}
	}
	else 
		{
			do {
				*--cp = digits[val % par->base];
				val /= par->base;
			} while (val );
		}

	if(par->prefix)
	{
		if( par->base == 8 )
		{
			if(val)
			{
				*--cp = '0';
			}
			
		} else if(par->base == 16)
		{
			*--cp = '0';
			*--cp = 'x';
		}
	}	
	
    if (negative)
	{
        *--cp = '-';
	}
	else if( par->showsignum)
	{
		*--cp = '+';
	}
		
    /* Move the converted number to the buffer and   */
    /* add in the padding where needed.              */
	outs(cp, par);
}

/*---------------------------------------------------*/
/*                                                   */
/* This routine gets a number from the format        */
/* string.                                           */
/*                                                   */
static int getnum( char** linep)
{
    int n;
    char* cp;
    unsigned char ch;

    n = 0;
    cp = *linep;
    ch = *cp;
    while (isdigit(ch))
	{
        n = n*10 + (*cp++ - '0');
        ch = *cp;
    }
    *linep = cp;
    return(n);
}


static int xprintf(params_t *par, const char* fmt, va_list argp)
{
	int res = -1;
    int dot_flag;
    unsigned char ch;
	unsigned long num;	
    char* pfmt = fmt;
	

    for ( ; *pfmt; pfmt++) 
	{
		/* move format string chars to buffer until a  */
        /* format control is found.                    */
        if (*pfmt != '%') {
            outbyte(*pfmt, par);
            continue;
        }

        /* initialize all the flags for this control   */
        dot_flag = 
		par->prefix =
		par->unsign =
		par->ip = 
		par->justify = 
		par->padding = 
		par->len = 
		par->showsignum =
		par->negativ =
		par->num1 =
		0;
		
        par->pad_character = ' ';

        par->num2=32767;
		par->base = 10;
		par->type = 24;	// eZ80 3 byte integer
		
 pfmt_loop:
        ch = *(++pfmt);
		
        if (isdigit(ch)) 
		{
            if (dot_flag)
                par->num2 = getnum(&pfmt);
            else 
			{
                if (ch == '0')
                    par->pad_character = '0';

                par->num1 = getnum(&pfmt);
                par->padding = 1;
            }
            pfmt--;
            goto pfmt_loop;
        }

        switch (tolower(ch)) {
            case '%':
                outbyte( '%', par);
                continue;
			case '#':
				par->prefix = 1;
				break;
            case '-':
                par->justify = 1;
                break;
            case '.':
                dot_flag = 1;
                break;
			
            case 'l':
                par->type = 32;
				if(*pfmt+1 == 'l')
				{
					// No long long on Zilog e80f91 
					// par->type = sizeof(long long);
					pfmt++;
				}
                break;
			case 'h':
                par->type = 16;
				if(*pfmt+1 == 'h')
				{
					par->type = 8;
					pfmt++;
				}
                break;
			case 'p':
				par->type = 24;
				par->pad_character = '0';
				par->num1 =
				par->num2 = 6;
				// fall through
            case 'x':
				par->base = 16;
				// fall through
			case 'u':
				par->unsign = 1;
				goto integerout;
            case 'd':
				if(ch == 'D')
					par->type = 32;
				// fall through
			case 'i':
integerout:		par->ip = (*(pfmt+1) == 'i' && *(pfmt+2) == 'p') ? 1:0;
				if(	par->ip )		
					pfmt +=2;
				if(par->type > 24)
					num = va_arg(argp,long);
				else
					num = va_arg(argp,int);
				outnum( num, par);
                continue;
				
            case 's':
                outs( va_arg( argp, char*), par);
                continue;

            case 'c':
                outbyte( va_arg( argp, char), par);
                continue;

            case '\\':
                switch (*pfmt) {
                    case 'a':
                        outbyte( 0x07, par);
                        break;
                    case 'h':
                        outbyte( 0x08, par);
                        break;
                    case 'r':
                        outbyte( 0x0D, par);
                        break;
                    case 'n':
                        outbyte( 0x0D, par);
                        outbyte( 0x0A, par);
                        break;
                    default:
                        outbyte( *pfmt, par);
                        break;
                }
                pfmt++;
                break;

            default:
                continue;
        }
        goto pfmt_loop;
    }
    va_end( argp);
	 
	return res;
}


/*                                                   */
/* This routine operates just like a printf/sprintf  */
/* routine. It outputs a set of data under the       */
/* control of a formatting string. Not all of the    */
/* standard C format control are supported. The ones */
/* provided are primarily those needed for embedded  */
/* systems work. Primarily the floaing point         */
/* routines are omitted. Other formats could be      */
/* added easily by following the examples shown for  */
/* the supported formats.                            */
/*                                                   */

int uart_printf(UARTID_t port, const char* fmt, ...)
{
	int res = -1;
	va_list argp;
	params_t par;
	
	par.o.pstr = 0;	
	par.o.top  = port;
		
	if( lockuart(par.o.top) == pdTRUE)
	{
		va_start( argp, fmt);
		res = xprintf( &par, fmt, argp);
		va_end(argp);
		
		unlockuart(par.o.top);
	}
	return res;
}

int snprintf(char* str, unsigned n, const char* fmt, ...)
{
	int res = -1;
	if(str && n)
	{
		va_list argp;
		params_t par;
		
		va_start( argp, fmt);

		par.o.pstr = str;
		par.o.max = n-1;
		par.o.top = 0;
		
		res = xprintf( &par, fmt, argp);
		par.o.pstr[par.o.top] = '\0';

		va_end(argp);
	}
	return res;
}

int vsnprintf(char* str, unsigned n, const char* fmt, va_list argp)
{
	int res = -1;
	params_t par;
	
	if(str && n)
	{
		par.o.pstr = str;
		par.o.max = n-1;
		par.o.top = 0;
		
		res = xprintf( &par, fmt, argp);
		par.o.pstr[par.o.top] = '\0';
	}
	return res;
}

int sprintf(char* str, const char* fmt, ...)
{
	int res = -1;
	if(str)
	{
		va_list argp;
		params_t par;

		va_start( argp, fmt);

		par.o.pstr = str;
		par.o.max = DEF_PRINTFSIZE-1;
		par.o.top = 0;
		
		res = xprintf( &par, fmt, argp);
		par.o.pstr[par.o.top] = '\0';

		va_end(argp);
	}
	return res;
}

int printf(const char* fmt, ...)
{
	int res = -1;
	va_list argp;
	params_t par;

	va_start( argp, fmt);

	par.o.pstr = 0;
	par.o.max = 0;
	par.o.top = consoleport;
	
	res = xprintf( &par, fmt, argp);
	va_end(argp);
	return res;
}

#endif 