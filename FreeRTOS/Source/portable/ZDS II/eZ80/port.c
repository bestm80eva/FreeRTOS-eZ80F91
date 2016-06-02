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
	
	
	File: 	port.c
			Part of FreeRTOS Port for the eZ80F91 Development Kit eZ80F910300ZCOG
			See www.zilog.com for desciption.


	Developer:
	JSIE	 Juergen Sievers <JSievers@NadiSoft.de>

	150804:	JSIE Start this port.
	
*/
#include "FreeRTOS.h"
#include "task.h"

/*-----------------------------------------------------------*/
void	vPortYield();
void	vPortYieldFromTick();
	
/* The address of the pxCurrentTCB variable, */
typedef void tskTCB;
extern volatile tskTCB * volatile pxCurrentTCB;

/*-----------------------------------------------------------*/

/*
 * Setup timer.
 */
const unsigned ticks = configCPU_CLOCK_HZ / 16UL / configTICK_RATE_HZ;
void prvSetupTimerInterrupt( void );

/*-----------------------------------------------------------*/

/*
 * Setup valid stack-frame for a new task and return top of frame.
 */

StackType_t *pxPortInitialiseStack( StackType_t *pxTopOfStack, TaskFunction_t pxCode, void *pvParameters ) PRIVILEGED_FUNCTION
{
    /* Place the parameter on the stack in the expected location. */
    *--pxTopOfStack = ( StackType_t ) pvParameters;

    /* Place the task return address on stack. Not used*/
    *--pxTopOfStack = ( StackType_t ) 0;

    /* The start of the task code will be popped off the stack last, so place
    it on first. */
    *--pxTopOfStack = ( StackType_t ) pxCode;

    /* Now the registers. */
    *--pxTopOfStack = ( StackType_t ) 0xAFAFAF;  /* AF  */
	*--pxTopOfStack = ( StackType_t ) 0xBCBCBC;  /* BC  */
    *--pxTopOfStack = ( StackType_t ) 0xDEDEDE;  /* DE  */
    *--pxTopOfStack = ( StackType_t ) 0xEFEFEF;  /* HL  */
	*--pxTopOfStack = ( StackType_t ) 0x111111;  /* IX  */
    *--pxTopOfStack = ( StackType_t ) 0x222222;  /* IY  */
    *--pxTopOfStack = ( StackType_t ) 0xFAFAFA;  /* AF' */
    *--pxTopOfStack = ( StackType_t ) 0xCBCBCB;  /* BC' */
    *--pxTopOfStack = ( StackType_t ) 0xEDEDED;  /* DE' */
    *--pxTopOfStack = ( StackType_t ) 0xFEFEFE;  /* HL' */
    return pxTopOfStack;
}

/*-----------------------------------------------------------*/
// Start the scheduler and the first (current) task	
BaseType_t xPortStartScheduler( void );

/*-----------------------------------------------------------*/

void vPortEndScheduler( void )
{
    /* It is unlikely that the eZ80 port will require this function as there
    is nothing to return to.  If this is required - stop the tick ISR then
    return back to main. */
	asm("di");
	asm("slp");
}
/*-----------------------------------------------------------*/


static UINT32 ulNextRand = 0x12345678;

/* You are right its a little engine MPU but the compiler 
   does not be able to handle the existing FreeRTOS macro
 */  
UINT32 portFreeRTOS_htonl( UINT32 ulIn ) 											
{
											// aabbccdd
	return  (ulIn << 24UL) 				|	// dd000000
			(ulIn <<  8UL) & 0xFF0000UL |	// ddcc0000
			(ulIn >>  8UL) & 0xFF00UL 	|	// ddccbb00
			(ulIn >> 24UL);					// ddccbbaa
}

UINT16 portFreeRTOS_htons( UINT16 usIn ) 											
{
	return (usIn >> 8U) | (usIn << 8U);
}
	
UINT32 uxRand( void )
{
	static const UINT32 ulMultiplier = 0x375a4e35UL; 
	static const UINT32 ulIncrement  = 7UL;

	/* Utility function to generate a pseudo random number. */
	ulNextRand = ulMultiplier * ulNextRand + ulIncrement;
	return ulNextRand;
}


void prvSRand( UBaseType_t ulSeed )
{
	/* Utility function to seed the pseudo random number generator. */
	ulNextRand = ulSeed;
	uxRand();
}

int strcasecmp(const char *s1, const char *s2)
{
	char c1=*s1,c2=*s2;
	
	while(*s1++ && *s2++)
	{
		c1 = *s1 - ((*s1 >= 'a' && *s1 <= 'z') ? ('a' + 'A') : 0);
		c2 = *s2 - ((*s2 >= 'a' && *s2 <= 'z') ? ('a' + 'A') : 0);
		if(c1 != c2)
			break;
	}
	return c1-c2;
}
