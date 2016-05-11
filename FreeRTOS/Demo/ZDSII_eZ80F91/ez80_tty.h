/*
    FreeRTOS V9.0.0rc2 - Copyright (C) 2016 Real Time Engineers Ltd.
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
	
	
	File: 	ez80_tty.h
			plattform rs232 ports driver
			Part of FreeRTOS Port for the eZ80F91 Development Kit eZ80F910300ZCOG
			See www.zilog.com for desciption.
			uart, console and faramted printout driver


	Developer:
	JSIE	 Juergen Sievers <JSievers@NadiSoft.de>

	150804:	JSIE Start this port.
	
*/

#ifndef _EZ80_TTY_H_
#define _EZ80_TTY_H_

#include "queue.h"
#include "semphr.h"	

#include <String.h>

// Ansi/VT100 escape inlets
/*
Sets multiple display attribute settings. The following lists standard attributes:

0	Reset all attributes
1	Bright
2	Dim
4	Underscore	
5	Blink
7	Reverse
8	Hidden

	Foreground Colours
30	Black
31	Red
32	Green
33	Yellow
34	Blue
35	Magenta
36	Cyan
37	White

	Background Colours
40	Black
41	Red
42	Green
43	Yellow
44	Blue
45	Magenta
46	Cyan
47	White
*/
#define ANSI_CLRS			"\x1b[2J"
#define ANSI_SCUR			"\x1b""7"
#define ANSI_RCUR			"\x1b""8"
#define ANSI_COFF			"\x1b[?25l"
#define ANSI_CON			"\x1b[?25h"
#define ANSI_GXY(x,y)		"\x1b["#y";"#x"f"
#define ANSI_SATT(m,f,b)	"\x1b["#m";"#f";"#b"m"
#define ANSI_NORM			"\x1b[2;37;40m"

#define ANSI_DEOL			"\x1b[K"

typedef enum {
	UART_0,
	UART_1
} UARTID_t;

void initSerial();
void init_uart( UARTID_t port, int baudrate, uint8_t databits, uint8_t stopbits, uint8_t parity, uint8_t fifolevel, uint8_t flowctrl);

extern UARTID_t consoleport;

#define UART_ERR_RECEIVEQUEUEFULL	0x0100	// FreeRTOS rec-queue full, not accepting byte

// uart dispatcher
typedef struct 
{
	QueueHandle_t 		InQueue;	// input queue
	QueueHandle_t 		OutQueue;	// output queue
	SemaphoreHandle_t 	Mutex;		// mutex to bind context
	volatile UINT 		recerr;		// receiver error
	volatile size_t		recbrk;		// Break detections
	volatile size_t		recfrm;		// Framing errors
	volatile size_t		recpar;		// Parity errors
	volatile size_t		recovr;		// Overruns
	volatile size_t		recqer;		// Queue full
	volatile size_t		intcnt;		// count of interrupts
	volatile size_t		rxskip;		// count of not queued bytes
	size_t				txskip;		// count of not transmitted bytes
} uartstat_t;

const uartstat_t* getUARTStat(UARTID_t port);

int lockcons();
int unlockcons();

int uart_printf(UARTID_t port, const char* fmt, ...);
int uart_putch (UARTID_t port, int c);	
int uart_putstr (UARTID_t port, const char* s);	// simple string out
int uart_puts (UARTID_t port, const char *s); 	// like putstr bud appends line feed

// we do not use ZiLOG's stdc library for console output

int putch (int c);
int puts (const char *s);
int printf(const char* fmt, ...);
int sprintf(char* s, const char* fmt, ...);
int snprintf(char* s, size_t sz, const char *fmt, ...);


#endif /* _EZ80_TTY_H_ */
