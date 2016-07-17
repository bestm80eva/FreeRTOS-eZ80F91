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
	
	
	File: 	ez80_led.c
			plattform 5x7 LED Matrx driver
			Part of FreeRTOS Port for the eZ80F91 Development Kit eZ80F910300ZCOG
			See www.zilog.com for desciption.


	Developer:
	JSIE	 Juergen Sievers <JSievers@NadiSoft.de>

	150804:	JSIE Start this port.
	
*/

/****************************************************************************
 * Included Files
 ****************************************************************************/
#include "FreeRTOS.h"
#ifdef INCLUDE_LED5x7
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "semphr.h"	
#include "ez80_leds.h"

#include <String.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/
/* LED anode and cathode external I/O pointers */
#define LEDMATRIX_ANODE   (*(volatile unsigned char*)0x800000)  //Anode
#define LEDMATRIX_CATHODE (*(volatile unsigned char*)0x800001)  //Cathode


/****************************************************************************
 * Private Types
 ****************************************************************************/
	
/****************************************************************************
 * Private Data
 ****************************************************************************/
static TimerHandle_t ledtimer;

/* 5x7 LED matrix character glyphs.  Each glyph consists of 7 bytes, one
 * each row and each containing 5 bits of data, one for each column
 */
static const CHAR cmatrix[96][7] = {			 // hex- ascii
	{0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f},  // 20 - space
	{0x1b, 0x1b, 0x1b, 0x1b, 0x1b, 0x1f, 0x1b},  // 21 - !
	{0x15, 0x15, 0x15, 0x1f, 0x1f, 0x1f, 0x1f},  // 22 - "
	{0x1f, 0x15, 0x00, 0x15, 0x00, 0x15, 0x1f},  // 23 - #
	{0x1b, 0x11, 0x0a, 0x11, 0x0a, 0x11, 0x1b},  // 24 - $
	{0x1f, 0x1e, 0x15, 0x1b, 0x15, 0x0f, 0x1f},  // 25 - %
	{0x11, 0x0e, 0x0e, 0x11, 0x15, 0x0e, 0x10},  // 26 - &
	{0x1b, 0x1b, 0x1b, 0x1f, 0x1f, 0x1f, 0x1f},  // 27 - '
	{0x1d, 0x1b, 0x17, 0x17, 0x17, 0x1b, 0x1d},  // 28 - (
	{0x17, 0x1b, 0x1d, 0x1d, 0x1d, 0x1b, 0x17},  // 29 - )
	{0x1f, 0x0a, 0x11, 0x00, 0x11, 0x0a, 0x1f},  // 2a - *
	{0x1f, 0x1b, 0x1b, 0x00, 0x1b, 0x1b, 0x1f},  // 2b - +
	{0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1b, 0x17},  // 2c - ,
	{0x1f, 0x1f, 0x1f, 0x00, 0x1f, 0x1f, 0x1f},  // 2d - -
	{0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1b},  // 2e - .
	{0x1f, 0x1e, 0x1d, 0x1b, 0x17, 0x0f, 0x1f},  // 2f - /
	{0x11, 0x0e, 0x0c, 0x0a, 0x06, 0x0e, 0x11},  // 30 - 0
	{0x1b, 0x13, 0x1b, 0x1b, 0x1b, 0x1b, 0x11},  // 31 - 1
	{0x11, 0x0e, 0x1d, 0x1b, 0x17, 0x0f, 0x00},  // 32 - 2
	{0x11, 0x0e, 0x1e, 0x19, 0x1e, 0x0e, 0x11},  // 33 - 3
	{0x0e, 0x0e, 0x0e, 0x10, 0x1e, 0x1e, 0x1e},  // 34 - 4
	{0x00, 0x0f, 0x0f, 0x01, 0x1e, 0x0e, 0x11},  // 35 - 5
	{0x11, 0x0f, 0x0f, 0x01, 0x0e, 0x0e, 0x11},  // 36 - 6
	{0x00, 0x1e, 0x1e, 0x1d, 0x1b, 0x1b, 0x1b},  // 37 - 7
	{0x11, 0x0e, 0x0e, 0x11, 0x0e, 0x0e, 0x11},  // 38 - 8
	{0x11, 0x0e, 0x0e, 0x10, 0x1e, 0x1d, 0x1b},  // 39 - 9
	{0x1f, 0x1f, 0x1b, 0x1f, 0x1b, 0x1f, 0x1f},  // 3a - :
	{0x1f, 0x1f, 0x1b, 0x1f, 0x1b, 0x17, 0x1f},  // 3b - ;
	{0x1d, 0x1b, 0x17, 0x0f, 0x17, 0x1b, 0x1d},  // 3c - <
	{0x1f, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x1f},  // 3d - =
	{0x17, 0x1b, 0x1d, 0x1e, 0x1d, 0x1b, 0x17},  // 3e - >
	{0x11, 0x0e, 0x0d, 0x1b, 0x1b, 0x1f, 0x1b},  // 3f - ?
	{0x11, 0x0a, 0x04, 0x04, 0x05, 0x0a, 0x11},  // 40 - @
	{0x11, 0x0e, 0x0e, 0x0e, 0x00, 0x0e, 0x0e},  // 41 - A
	{0x01, 0x0e, 0x0e, 0x01, 0x0e, 0x0e, 0x01},  // 42 - B
	{0x11, 0x0e, 0x0f, 0x0f, 0x0f, 0x0e, 0x11},  // 43 - C
	{0x01, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x01},  // 44 - D
	{0x00, 0x0f, 0x0f, 0x01, 0x0f, 0x0f, 0x00},  // 45 - E
	{0x00, 0x0f, 0x0f, 0x01, 0x0f, 0x0f, 0x0f},  // 46 - F
	{0x11, 0x0e, 0x0f, 0x08, 0x0e, 0x0e, 0x11},  // 47 - G
	{0x0e, 0x0e, 0x0e, 0x00, 0x0e, 0x0e, 0x0e},  // 48 - H
	{0x00, 0x1b, 0x1b, 0x1b, 0x1b, 0x1b, 0x00},  // 49 - I
	{0x00, 0x1d, 0x1d, 0x1d, 0x0d, 0x0d, 0x13},  // 4a - J
	{0x0e, 0x0d, 0x0b, 0x07, 0x0b, 0x0d, 0x0e},  // 4b - K
	{0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x00},  // 4c - L
	{0x0e, 0x04, 0x0a, 0x0a, 0x0e, 0x0e, 0x0e},  // 4d - M
	{0x0e, 0x0e, 0x06, 0x0a, 0x0c, 0x0e, 0x0e},  // 4e - N
	{0x11, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x11},  // 4f - O
	{0x01, 0x0e, 0x0e, 0x01, 0x0f, 0x0f, 0x0f},  // 50 - P
	{0x11, 0x0e, 0x0e, 0x0e, 0x0a, 0x0c, 0x10},  // 51 - Q
	{0x01, 0x0e, 0x0e, 0x01, 0x0b, 0x0d, 0x0e},  // 52 - R
	{0x11, 0x0e, 0x0f, 0x11, 0x1e, 0x0e, 0x11},  // 53 - S
	{0x00, 0x1b, 0x1b, 0x1b, 0x1b, 0x1b, 0x1b},  // 54 - T
	{0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x11},  // 55 - U
	{0x0e, 0x0e, 0x0e, 0x0e, 0x0e, 0x15, 0x1b},  // 56 - V
	{0x0e, 0x0e, 0x0a, 0x0a, 0x0a, 0x0a, 0x15},  // 57 - W
	{0x0e, 0x0e, 0x15, 0x1b, 0x15, 0x0e, 0x0e},  // 58 - X
	{0x0e, 0x0e, 0x15, 0x1b, 0x1b, 0x1b, 0x1b},  // 59 - Y
	{0x00, 0x1e, 0x1d, 0x1b, 0x17, 0x0f, 0x00},  // 5a - Z
	{0x03, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x03},  // 5b - [
	{0x1f, 0x0f, 0x17, 0x1b, 0x1d, 0x1e, 0x1f},  // 5c - backslash
	{0x1c, 0x1e, 0x1e, 0x1e, 0x1e, 0x1e, 0x1c},  // 5d - ]
	{0x1b, 0x15, 0x0e, 0x1f, 0x1f, 0x1f, 0x1f},  // 5e - ^
	{0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x00},  // 5f - _
	{0x1b, 0x1b, 0x1b, 0x1f, 0x1f, 0x1f, 0x1f},  // 60 - '
	{0x1f, 0x1f, 0x19, 0x16, 0x16, 0x16, 0x18},  // 61 - a
	{0x17, 0x17, 0x11, 0x16, 0x16, 0x16, 0x11},  // 62 - b
	{0x1f, 0x1f, 0x19, 0x16, 0x17, 0x16, 0x19},  // 63 - c
	{0x1e, 0x1e, 0x18, 0x16, 0x16, 0x16, 0x18},  // 64 - d
	{0x1f, 0x1f, 0x19, 0x10, 0x17, 0x16, 0x19},  // 65 - e
	{0x1d, 0x1a, 0x1b, 0x11, 0x1b, 0x1b, 0x1b},  // 66 - f
	{0x1f, 0x19, 0x16, 0x16, 0x18, 0x16, 0x19},  // 67 - g
	{0x17, 0x17, 0x11, 0x16, 0x16, 0x16, 0x16},  // 68 - h
	{0x1f, 0x1f, 0x1b, 0x1f, 0x1b, 0x1b, 0x1b},  // 69 - i
	{0x1f, 0x1d, 0x1f, 0x1d, 0x1d, 0x1d, 0x13},  // 6a - j
	{0x17, 0x17, 0x15, 0x13, 0x13, 0x15, 0x16},  // 6b - k
	{0x1b, 0x1b, 0x1b, 0x1b, 0x1b, 0x1b, 0x1b},  // 6c - l
	{0x1f, 0x1f, 0x05, 0x0a, 0x0a, 0x0a, 0x0a},  // 6d - m
	{0x1f, 0x1f, 0x11, 0x16, 0x16, 0x16, 0x16},  // 6e - n
	{0x1f, 0x1f, 0x19, 0x16, 0x16, 0x16, 0x19},  // 6f - o
	{0x1f, 0x11, 0x16, 0x16, 0x11, 0x17, 0x17},  // 70 - p
	{0x1f, 0x18, 0x16, 0x16, 0x18, 0x1e, 0x1e},  // 71 - q
	{0x1f, 0x1f, 0x11, 0x16, 0x17, 0x17, 0x17},  // 72 - r
	{0x1f, 0x1f, 0x18, 0x17, 0x19, 0x1e, 0x11},  // 73 - s
	{0x1f, 0x1f, 0x1b, 0x11, 0x1b, 0x1b, 0x1b},  // 74 - t
	{0x1f, 0x1f, 0x16, 0x16, 0x16, 0x16, 0x18},  // 75 - u
	{0x1f, 0x1f, 0x16, 0x16, 0x16, 0x16, 0x19},  // 76 - v
	{0x1f, 0x1f, 0x0a, 0x0a, 0x0a, 0x0a, 0x15},  // 77 - w
	{0x1f, 0x1f, 0x0e, 0x15, 0x1b, 0x15, 0x0e},  // 78 - x
	{0x1f, 0x1a, 0x1a, 0x1a, 0x1d, 0x1b, 0x17},  // 79 - y
	{0x1f, 0x1f, 0x10, 0x1d, 0x1b, 0x17, 0x10},  // 7a - z
	{0x1d, 0x1b, 0x1b, 0x17, 0x1b, 0x1b, 0x1d},  // 7b - {
	{0x1b, 0x1b, 0x1b, 0x1f, 0x1b, 0x1b, 0x1b},  // 7c - |
	{0x17, 0x1b, 0x1b, 0x1d, 0x1b, 0x1b, 0x17},  // 7d - }
	{0x1f, 0x1a, 0x15, 0x1f, 0x1f, 0x1f, 0x1f},  // 7e - ~
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}   // 7f - block
};
	
/* The current selected glyph 7 rows*/
static volatile CHAR *currglyph;

static UINT8  frames;	// display refresch delay 
static UINT8  shift;	// delay between shifts
static UINT16 chardly;	// delay between letters

/* Display character queue	*/
static QueueHandle_t xLED5x7Queue;

/* current shift direction */
static dir_e	dir;

/* display shift buffer */
static volatile union {
	int24_t i;
	uint8_t c[3];
}dply[7];

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/* copy 5x7 image to display shift buffer *
 * 11111RRR RR1NNNNN 1LLLLL11			  *
 * R if SHIFT_RIGTH                       *
 * N if SHIFT_NONE and                    *
 * L on SHIFT_LEFT						  */	
static void setDisplay(const CHAR* src)
{
	int i;
	portENTER_CRITICAL();
	switch(dir)
	{
		case SHIFT_LEFT:
			for(i = 0; i < 7; i++)
				dply[i].c[0] = 0x83 | (*src++ << 2);
			break;
		case SHIFT_NONE:
			for(i = 0; i < 7; i++)
				dply[i].c[1] = 0xE0 | *src++;
			break;
		case SHIFT_RIGHT:
			for(i = 0; i < 7; i++)
			{
				dply[i].c[2] = 0xF8 | (*src++ >> 2);
				dply[i].c[1] = dply[i].c[1] & 0x3F | (*src++ << 6);
			}
			break;
	}
	portEXIT_CRITICAL();
}

/* Shift displa buffer */
static void shiftDisplay()
{
	int i;
	BYTE msk;
	
	
	switch(dir)
	{
		case SHIFT_LEFT:
			portENTER_CRITICAL();
			for(i = 0; i < 7; i++)
			{
				dply[i].i <<= 1;
				dply[i].c[0]  |= 0x1;
			}
			portEXIT_CRITICAL();
			break;
		case SHIFT_NONE:
			break;
		case SHIFT_RIGHT:
			portENTER_CRITICAL();
			for(i = 0; i < 7; i++)
			{
				dply[i].i >>= 1;
				dply[i].c[2]  |= 0x80;
			}
			portEXIT_CRITICAL();
			break;
	}
	
}

/* get ascii-char from queue and 
 * copy its image to display buffer	
 */
static void nextDisplay(void)
{
	UCHAR c;
	
	if(pdFALSE == xQueueReceive( xLED5x7Queue, &c, shift))
		c = 0x5F;
	
	c &= 0x7F;
	
	if(c < 0x20)
		c = 0x5F;
	else 
		c -=0x20;
	
	currglyph = cmatrix[c];
	setDisplay(currglyph);
}

/****************************************************************************
 * Name: LEDTick
 * Timer procedure to display all scan-lines 
 ****************************************************************************/
void	LEDTick(TimerHandle_t thdl)
{
	static uint8_t row=7;
	uint8_t tmp = CS2_LBR;
	
	row--;
	
	portENTER_CRITICAL();
	CS2_LBR = 0x80;
	LEDMATRIX_ANODE   = (1 << row);	// enable row
	LEDMATRIX_CATHODE = 0x1F & dply[row].c[1]; 	// set row image
	CS2_LBR = tmp;
	portEXIT_CRITICAL();
	
	if(!row)
		row = 7;
	
}

/****************************************************************************
 * Name: LED5x7Task
 * Task handle the 5x7 LED Matrix
 ****************************************************************************/
static void LED5x7Task( void *x)
{
	int i;
	
	while(1)
	{
		nextDisplay();			// get next character image to displaybuffer
		vTaskDelay(chardly);	// delay between chars
		for(i = 0; i < 6; i++)
		{
			shiftDisplay();		// shift in the new char image
			vTaskDelay(shift);	// shift speed delay				
		}
		
	}
}
/****************************************************************************
 * Public Functions
 ****************************************************************************/

/* check if no chars on queue 
 * returns pdTRUE if queue is empty.
 */
UBaseType_t LED5x7_free()
{
	return uxQueueSpacesAvailable( xLED5x7Queue );
}

/* append c-string to queue
 * returns number of chars added
 */
BaseType_t LED5x7_puts(const CHAR *s, TickType_t tout)
{
	BaseType_t res = 0;
	if(s)
		while(*s && LED5x7_putchar(*s++, tout))
			res++;
	return res;
}


/* append one char to queue with timeout
 * returns pdTRUE  if char added pdFALSE if not
 */
BaseType_t LED5x7_putchar(CHAR c, TickType_t tout)
{
	return xQueueSend( xLED5x7Queue, (const void*)&c, tout);
}

/****************************************************************************
 * initLED5x7
 * setup the 5x7 LED Matrix driver
 ****************************************************************************/
static int tid;
void initLED5x7()
{

	currglyph  	= cmatrix[95];
	frames		= LED5x7_FRAMES;	// display refresch delay 
	shift		= LED5x7_SHIFTT;	// LED5x7_FRAMES delays between shifts
	chardly		= LED5x7_CDELAY;	// LED5x7_FRAMES deleys between letters
	dir			= SHIFT_LEFT;
	memset(dply,0xFF,sizeof(dply));
	
	xLED5x7Queue= xQueueCreate( LED5x7_QUEUES, sizeof( CHAR));
	xTaskCreate( LED5x7Task, "LED5x7", configMINIMAL_STACK_SIZE, NULL,PRIO_LED, &ledtimer);
	ledtimer = xTimerCreate("LED5x7Timer", LED5x7_FRAMES, pdTRUE, &tid, LEDTick);
	xTimerStart(ledtimer,0);
}

/* set shift direction */
dir_e LED5x7_setDir(dir_e to)
{
	dir_e old = dir;
	dir = to;
	return old;
}

#endif /* INCLUDE_LED5x7 */