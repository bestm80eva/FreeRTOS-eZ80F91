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
#include "ez80_leds.h"

#include <String.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/
/* LED anode and cathode external I/O pointers */
#define LEDMATRIX_ROW      (*(unsigned char*)0x800000)  //Anode
#define LEDMATRIX_COLUMN   (*(unsigned char*)0x800001)  //Cathode


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
static const CHAR cmatrix[96][7] = {
    /* misc. characters starting at ascii space */
    {0xFf, 0xFf, 0xFf, 0xFf, 0xFf, 0xFf, 0xFf},  //  1 20 - space
	{0xFb, 0xFb, 0xFb, 0xFb, 0xFb, 0xFf, 0xFb},  //  2 21 - !
    {0xF5, 0xF5, 0xF5, 0xFf, 0xFf, 0xFf, 0xFf},  //  3 22 - "
	{0xFf, 0xF5, 0xE0, 0xF5, 0xE0, 0xF5, 0xFf},  //  4 23 - #
	{0xFb, 0xF1, 0xEa, 0xF1, 0xEa, 0xF1, 0xFb},  //  5 24 - $
	{0xFf, 0xFe, 0xF5, 0xFb, 0xF5, 0xEf, 0xFf},  //  6 25 - %
	{0xF1, 0xEe, 0xEe, 0xF1, 0xF5, 0xEe, 0xF0},  //  7 26 - &
	{0xFb, 0xFb, 0xFb, 0xFf, 0xFf, 0xFf, 0xFf},  //  8 27 - '
	{0xFd, 0xFb, 0xF7, 0xF7, 0xF7, 0xFb, 0xFd},  //  9 23 - (
	{0xF7, 0xFb, 0xFd, 0xFd, 0xFd, 0xFb, 0xF7},  // 10 - )
	{0xFf, 0xEa, 0xF1, 0xE0, 0xF1, 0xEa, 0xFf},  // 11 - *
	{0xFf, 0xFb, 0xFb, 0xE0, 0xFb, 0xFb, 0xFf},  // 12 - +
	{0xFf, 0xFf, 0xFf, 0xFf, 0xFf, 0xFb, 0xF7},  // 13 - ,
	{0xFf, 0xFf, 0xFf, 0xE0, 0xFf, 0xFf, 0xFf},  // 14 - -
	{0xFf, 0xFf, 0xFf, 0xFf, 0xFf, 0xFf, 0xFb},  // 15 - .
	{0xFf, 0xFe, 0xFd, 0xFb, 0xF7, 0xEf, 0xFf},  // 16 - /
    /* numbers */
    {0xF1, 0xEe, 0xEc, 0xEa, 0xE6, 0xEe, 0xF1},  // 17 - 0
    {0xFb, 0xF3, 0xFb, 0xFb, 0xFb, 0xFb, 0xF1},  // 18 - 1
    {0xF1, 0xEe, 0xFd, 0xFb, 0xF7, 0xEf, 0xE0},  // 19 - 2
    {0xF1, 0xEe, 0xFe, 0xF9, 0xFe, 0xEe, 0xF1},  // 20 - 3
    {0xEe, 0xEe, 0xEe, 0xF0, 0xFe, 0xFe, 0xFe},  // 21 - 4
    {0xE0, 0xEf, 0xEf, 0xE1, 0xFe, 0xEe, 0xF1},  // 22 - 5
    {0xF1, 0xEf, 0xEf, 0xE1, 0xEe, 0xEe, 0xF1},  // 23 - 6
    {0xE0, 0xFe, 0xFe, 0xFd, 0xFb, 0xFb, 0xFb},  // 24 - 7
    {0xF1, 0xEe, 0xEe, 0xF1, 0xEe, 0xEe, 0xF1},  // 25 - 8
    {0xF1, 0xEe, 0xEe, 0xF0, 0xFe, 0xFd, 0xFb},  // 26 - 9
    /* misc. characters */
    {0xFf, 0xFf, 0xFb, 0xFf, 0xFb, 0xFf, 0xFf},  // 27 - :
	{0xFf, 0xFf, 0xFb, 0xFf, 0xFb, 0xF7, 0xFf},  // 28 - ;
	{0xFd, 0xFb, 0xF7, 0xEf, 0xF7, 0xFb, 0xFd},  // 29 - <
	{0xFf, 0xFf, 0xE0, 0xFf, 0xE0, 0xFf, 0xFf},  // 30 - =
	{0xF7, 0xFb, 0xFd, 0xFe, 0xFd, 0xFb, 0xF7},  // 31 - >
	{0xF1, 0xEe, 0xEd, 0xFb, 0xFb, 0xFf, 0xFb},  // 32 - ?
	{0xF1, 0xEa, 0xE4, 0xE4, 0xE5, 0xEa, 0xF1},  // 33 - @
    /* upper case characters */
    {0xF1, 0xEe, 0xEe, 0xEe, 0xE0, 0xEe, 0xEe},  // 34 - A
    {0xE1, 0xEe, 0xEe, 0xE1, 0xEe, 0xEe, 0xE1},  // 35 - B
    {0xF1, 0xEe, 0xEf, 0xEf, 0xEf, 0xEe, 0xF1},  // 36 - C
    {0xE1, 0xEe, 0xEe, 0xEe, 0xEe, 0xEe, 0xE1},  // 37 - D
    {0xE0, 0xEf, 0xEf, 0xE1, 0xEf, 0xEf, 0xE0},  // 38 - E
    {0xE0, 0xEf, 0xEf, 0xE1, 0xEf, 0xEf, 0xEf},  // 39 - F
    {0xF1, 0xEe, 0xEf, 0xE8, 0xEe, 0xEe, 0xF1},  // 40 - G
    {0xEe, 0xEe, 0xEe, 0xE0, 0xEe, 0xEe, 0xEe},  // 41 - H
    {0xE0, 0xFb, 0xFb, 0xFb, 0xFb, 0xFb, 0xE0},  // 42 - I
    {0xE0, 0xFd, 0xFd, 0xFd, 0xEd, 0xEd, 0xF3},  // 43 - J
    {0xEe, 0xEd, 0xEb, 0xE7, 0xEb, 0xEd, 0xEe},  // 44 - K
    {0xEf, 0xEf, 0xEf, 0xEf, 0xEf, 0xEf, 0xE0},  // 45 - L
    {0xEe, 0xE4, 0xEa, 0xEa, 0xEe, 0xEe, 0xEe},  // 46 - M
    {0xEe, 0xEe, 0xE6, 0xEa, 0xEc, 0xEe, 0xEe},  // 47 - N
    {0xF1, 0xEe, 0xEe, 0xEe, 0xEe, 0xEe, 0xF1},  // 48 - O
    {0xE1, 0xEe, 0xEe, 0xE1, 0xEf, 0xEf, 0xEf},  // 49 - P
    {0xF1, 0xEe, 0xEe, 0xEe, 0xEa, 0xEc, 0xF0},  // 50 - Q
    {0xE1, 0xEe, 0xEe, 0xE1, 0xEb, 0xEd, 0xEe},  // 51 - R
    {0xF1, 0xEe, 0xEf, 0xF1, 0xFe, 0xEe, 0xF1},  // 52 - S
    {0xE0, 0xFb, 0xFb, 0xFb, 0xFb, 0xFb, 0xFb},  // 53 - T
    {0xEe, 0xEe, 0xEe, 0xEe, 0xEe, 0xEe, 0xF1},  // 54 - U
    {0xEe, 0xEe, 0xEe, 0xEe, 0xEe, 0xF5, 0xFb},  // 55 - V
    {0xEe, 0xEe, 0xEa, 0xEa, 0xEa, 0xEa, 0xF5},  // 56 - W
    {0xEe, 0xEe, 0xF5, 0xFb, 0xF5, 0xEe, 0xEe},  // 57 - X
    {0xEe, 0xEe, 0xF5, 0xFb, 0xFb, 0xFb, 0xFb},  // 58 - Y
    {0xE0, 0xFe, 0xFd, 0xFb, 0xF7, 0xEf, 0xE0},  // 59 - Z
    /* misc. characters */
    {0xE3, 0xEf, 0xEf, 0xEf, 0xEf, 0xEf, 0xE3},  // 60 - [
    {0xFf, 0xEf, 0xF7, 0xFb, 0xFd, 0xFe, 0xFf},  // 61 - backslash
	{0xFc, 0xFe, 0xFe, 0xFe, 0xFe, 0xFe, 0xFc},  // 62 - ]
	{0xFb, 0xF5, 0xEe, 0xFf, 0xFf, 0xFf, 0xFf},  // 63 - ^
	{0xFf, 0xFf, 0xFf, 0xFf, 0xFf, 0xFf, 0xE0},  // 64 - _
	{0xFb, 0xFb, 0xFb, 0xFf, 0xFf, 0xFf, 0xFf},  // 65 - '
	/* lower case characters */
    {0xFf, 0xFf, 0xF9, 0xF6, 0xF6, 0xF6, 0xF8},  // 66 - a
    {0xF7, 0xF7, 0xF1, 0xF6, 0xF6, 0xF6, 0xF1},  // 67 - b
    {0xFf, 0xFf, 0xF9, 0xF6, 0xF7, 0xF6, 0xF9},  // 68 - c
    {0xFe, 0xFe, 0xF8, 0xF6, 0xF6, 0xF6, 0xF8},  // 69 - d
    {0xFf, 0xFf, 0xF9, 0xF0, 0xF7, 0xF6, 0xF9},  // 70 - e
    {0xFd, 0xFa, 0xFb, 0xF1, 0xFb, 0xFb, 0xFb},  // 71 - f
    {0xFf, 0xF9, 0xF6, 0xF6, 0xF8, 0xF6, 0xF9},  // 72 - g
    {0xF7, 0xF7, 0xF1, 0xF6, 0xF6, 0xF6, 0xF6},  // 73 - h
    {0xFf, 0xFf, 0xFb, 0xFf, 0xFb, 0xFb, 0xFb},  // 74 - i
    {0xFf, 0xFd, 0xFf, 0xFd, 0xFd, 0xFd, 0xF3},  // 75 - j
    {0xF7, 0xF7, 0xF5, 0xF3, 0xF3, 0xF5, 0xF6},  // 76 - k
    {0xFb, 0xFb, 0xFb, 0xFb, 0xFb, 0xFb, 0xFb},  // 77 - l
    {0xFf, 0xFf, 0xE5, 0xEa, 0xEa, 0xEa, 0xEa},  // 78 - m
    {0xFf, 0xFf, 0xF1, 0xF6, 0xF6, 0xF6, 0xF6},  // 79 - n
    {0xFf, 0xFf, 0xF9, 0xF6, 0xF6, 0xF6, 0xF9},  // 80 - o
    {0xFf, 0xF1, 0xF6, 0xF6, 0xF1, 0xF7, 0xF7},  // 81 - p
    {0xFf, 0xF8, 0xF6, 0xF6, 0xF8, 0xFe, 0xFe},  // 82 - q
    {0xFf, 0xFf, 0xF1, 0xF6, 0xF7, 0xF7, 0xF7},  // 83 - r
    {0xFf, 0xFf, 0xF8, 0xF7, 0xF9, 0xFe, 0xF1},  // 84 - s
    {0xFf, 0xFf, 0xFb, 0xF1, 0xFb, 0xFb, 0xFb},  // 85 - t
    {0xFf, 0xFf, 0xF6, 0xF6, 0xF6, 0xF6, 0xF8},  // 86 - u
    {0xFf, 0xFf, 0xF6, 0xF6, 0xF6, 0xF6, 0xF9},  // 87 - v
    {0xFf, 0xFf, 0xEa, 0xEa, 0xEa, 0xEa, 0xF5},  // 88 - w
    {0xFf, 0xFf, 0xEe, 0xF5, 0xFb, 0xF5, 0xEe},  // 89 - x
    {0xFf, 0xFa, 0xFa, 0xFa, 0xFd, 0xFb, 0xF7},  // 90 - y
    {0xFf, 0xFf, 0xF0, 0xFd, 0xFb, 0xF7, 0xF0},  // 91 - z
    {0xFd, 0xFb, 0xFb, 0xF7, 0xFb, 0xFb, 0xFd},  // 92 - {
	{0xFb, 0xFb, 0xFb, 0xFf, 0xFb, 0xFb, 0xFb},  // 93 - |
	{0xF7, 0xFb, 0xFb, 0xFd, 0xFb, 0xFb, 0xF7},  // 94 - }
	{0xFf, 0xFa, 0xF5, 0xFf, 0xFf, 0xFf, 0xFf},	// 95 7E - ~
	{0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0}	// 96 7F - ~
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
	
	switch(dir)
	{
		case SHIFT_LEFT:
			for(i = 0; i < 7; i++)
				dply[i].c[0] = 3 | (*src++ << 2);
			break;
		case SHIFT_NONE:
			for(i = 0; i < 7; i++)
				dply[i].c[1] = *src++;
			break;
		case SHIFT_RIGHT:
			for(i = 0; i < 7; i++)
			{
				dply[i].c[2] = 0xC0 | (*src++ >> 2);
				dply[i].c[1] = dply[i].c[1] & 0x3F | (*src++ << 6);
			}
			break;
	}
	
}

/* Shift displa buffer */
static void shiftDisplay()
{
	int i;
	BYTE msk;
	
	switch(dir)
	{
		case SHIFT_LEFT:
			for(i = 0; i < 7; i++)
			{
				dply[i].i <<= 1;
				dply[i].c[0]  |= 0x1;
			}
			break;
		case SHIFT_NONE:
			break;
		case SHIFT_RIGHT:
			for(i = 0; i < 7; i++)
			{
				dply[i].i >>= 1;
				dply[i].c[2]  |= 0x80;
			}
			break;
	}
	
}

/* get ascii-char from queue and 
 * copy its image to display buffer	
 */
static void nextDisplay(void)
{
	UCHAR c;
	
	if(!xQueueReceive( xLED5x7Queue, &c, shift))
		c = 95;
	
	currglyph = cmatrix[c];
	setDisplay(currglyph);
}

/****************************************************************************
 * Name: LEDTick
 * Timer procedure to display all scan-lines 
 ****************************************************************************/
static uint8_t trigger = 0xE0; // useable vor trigger on pinports 5,6,7

void	LEDTick(TimerHandle_t thdl)
{
	static uint8_t idx = 0;	// current scan-line
	
	if(idx > 6)				
		idx = 0;				// restart map index
	
	LEDMATRIX_COLUMN = 0xE0 | dply[idx].c[1]; 	// set row image
	LEDMATRIX_ROW 	 = 0x80 | (1 << idx);// enable row

	idx++;
	
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
	c &= 0x7F;
	if(c < ' ')
		c = 0x7F;
	else 
		c -= ' ';
	return xQueueSend( xLED5x7Queue, (const void*)&c, tout);
}

/****************************************************************************
 * initLED5x7
 * setup the 5x7 LED Matrix driver
 ****************************************************************************/
void initLED5x7()
{
	LEDMATRIX_ROW 	 = 0;		// light off
	LEDMATRIX_COLUMN = 0xFF; 
	
	currglyph  	= cmatrix[95];
	frames		= LED5x7_FRAMES;	// display refresch delay 
	shift		= LED5x7_SHIFTT;	// LED5x7_FRAMES delays between shifts
	chardly		= LED5x7_CDELAY;	// LED5x7_FRAMES deleys between letters
	dir			= SHIFT_LEFT;
	memset(dply,0xFF,sizeof(dply));
	xLED5x7Queue= xQueueCreate( LED5x7_QUEUES, sizeof( CHAR));
	xTaskCreate( LED5x7Task, "LED5x7", 1024, NULL, tskIDLE_PRIORITY + 1, NULL);
	ledtimer = xTimerCreate("LED5x7Timer", LED5x7_FRAMES, pdTRUE, 0, LEDTick);
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