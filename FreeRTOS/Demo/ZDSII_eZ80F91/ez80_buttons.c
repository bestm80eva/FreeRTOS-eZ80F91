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
	
	
	File: 	ez80_buttons.c
			plattform button driver.
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
#ifdef INCLUDE_BUTTONS 
#include "task.h"
#include "ez80_buttons.h"
#include <gpio.h>
#include <String.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/****************************************************************************
 * Private Data
 ****************************************************************************/
static volatile button_t button[3];
static const char * const tname[3] = {"BLTimer","BMTimer","BRTimer"};
static unsigned bstate;
/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: buttonx
 *
 * Description:
 *   Button Edge-Triggered Interrupt.
 *
 ****************************************************************************/

static void buttonx(BUTTONID_t id)
{
	button[id].state = BUTTON_UNKNOWN;
	xTimerStart	( button[id].tm, 0);
	PB_ALT0 |= (1 << id);/* Clear interrupt flag for eZ80F91 date codes 0611 and after */
}

void nested_interrupt button0_isp(void)
{
	buttonx(BUTTON_LEFT);
}

void nested_interrupt button1_isp(void)
{
	buttonx(BUTTON_MIDLE);
}

void nested_interrupt button2_isp(void)
{
	buttonx(BUTTON_RIGTH);
}

static void ButTimer( TimerHandle_t tm)
{
	button_t *b =  (button_t*) pvTimerGetTimerID( tm);
	portENTER_CRITICAL();
	b->state = ( PB_DR >> b->index) & 1;
	b->changes++;
	b->privT = b->lastT;
	b->lastT = xTaskGetTickCount();
	if(b->sem)
		xSemaphoreGive(b->sem);
	portEXIT_CRITICAL();
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: initButtons
 ****************************************************************************/

void initButtons(void)
{
	int i;
	for(i=BUTTON_LEFT; i <= BUTTON_RIGTH; i++)
	{
		button[i].index = i; 
		button[i].sem   = 0;
		button[i].state = BUTTON_UNKNOWN;
		button[i].privT = 
		button[i].lastT = xTaskGetTickCount();
		button[i].changes= -1;
		button[i].tm	= xTimerCreate(tname[i], BUTTON_PRELL, pdFALSE, &button[i], ButTimer);
		
		xTimerStart	( button[i].tm, 0);
	}

	/* Attach GIO interrupts */
	set_vector(PB0_IVECT, button0_isp);
	set_vector(PB1_IVECT, button1_isp);
	set_vector(PB2_IVECT, button2_isp);
	
	/* Configure PB0,1,2 as interrupt, dual edge */
	PB_DR 	|= 7;
	PB_ALT2	|= 7;
	PB_ALT1	&= ~7;
	PB_DDR	&= ~7;
}



const button_t *get_button(BUTTONID_t id)
{
  return &button[id];
}

const button_t *wait_buttonchanged( BUTTONID_t id, TickType_t timeout)
{
	return (!button[id].sem && !(button[id].sem = xSemaphoreCreateBinary()) ||
		xSemaphoreTake(button[id].sem, timeout) == pdFALSE) ? 0 : &button[id];
}

#endif /* INCLUDE_BUTTONS */
