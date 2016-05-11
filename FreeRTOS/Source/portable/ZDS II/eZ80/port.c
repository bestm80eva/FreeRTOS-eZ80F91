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

/* The address of the pxCurrentTCB variable, */
typedef void tskTCB;
extern volatile tskTCB * volatile pxCurrentTCB;

/*-----------------------------------------------------------*/

/*
 * Setup timer.
 */
static void prvSetupTimerInterrupt( void )
{
    void * set_vector(unsigned int vector,void (*hndlr)(void));
	void nested_interrupt timer_isr(void);
    unsigned char tmp;

    /* set Timer interrupt vector */
    set_vector(TIMER_VECTOR, timer_isr);

    TMR0_DR_H = (configCPU_CLOCK_HZ / 16UL / configTICK_RATE_HZ) >> 8;
    TMR0_DR_L = (configCPU_CLOCK_HZ / 16UL / configTICK_RATE_HZ) & 0xFF;

    tmp = TMR0_IIR;
    TMR0_CTL = 0x0F;
    TMR0_IER = 0x01;
}

/*-----------------------------------------------------------*/

/*
 * Setup valid stack-frame vor a new task and return top of frame.
 */
StackType_t *pxPortInitialiseStack( StackType_t *pxTopOfStack, TaskFunction_t pxCode, void *pvParameters ) PRIVILEGED_FUNCTION
{
    /* Place the parameter on the stack in the expected location. */
    *pxTopOfStack-- = ( StackType_t ) pvParameters;

    /* Place the task return address on stack. Not used*/
    *pxTopOfStack-- = ( StackType_t ) 0x000000;

    /* The start of the task code will be popped off the stack last, so place
    it on first. */
    *pxTopOfStack-- = ( StackType_t ) pxCode;

    /* Now the registers. */
    *pxTopOfStack-- = ( StackType_t ) 0xAFAFAF;  /* AF  */
	*pxTopOfStack-- = ( StackType_t ) 0x222222;  /* IY  */
    *pxTopOfStack-- = ( StackType_t ) 0xBCBCBC;  /* BC  */
    *pxTopOfStack-- = ( StackType_t ) 0xDEDEDE;  /* DE  */
    *pxTopOfStack-- = ( StackType_t ) 0xEFEFEF;  /* HL  */
	*pxTopOfStack-- = ( StackType_t ) 0x111111;  /* IX  */
    *pxTopOfStack-- = ( StackType_t ) 0xFAFAFA;  /* AF' */
    *pxTopOfStack-- = ( StackType_t ) 0xCBCBCB;  /* BC' */
    *pxTopOfStack-- = ( StackType_t ) 0xEDEDED;  /* DE' */
    *pxTopOfStack   = ( StackType_t ) 0xFEFEFE;  /* HL' */
    return pxTopOfStack;
}

/*-----------------------------------------------------------*/
// Start the scheduler and the first (current) task	
BaseType_t xPortStartScheduler( void )
{
    /* Setup the hardware to generate the tick. */
    prvSetupTimerInterrupt();
    /* Restore the context of the first task that is going to run. */
    asm (   "xref   _pxCurrentTCB           \n\t"   
            "ld     hl,     (_pxCurrentTCB) \n\t"   
            "ld     hl,     (hl)            \n\t"   
            "ld     sp,     hl              \n\t"   
            "pop    hl                      \n\t"   
            "pop    de                      \n\t"   
            "pop    bc                      \n\t"   
            "pop    af                      \n\t"   
            "exx                            \n\t"   
            "ex     af,     af'             \n\t"   
			"POP	IX                      \n\t"   
			"POP	HL                      \n\t"   
			"POP	DE                      \n\t"   
			"POP	BC                      \n\t"   
			"POP	IY                      \n\t"   
			"POP	AF                      \n\t"   
			"EI                      		\n\t"   
			"RET                      		\n\t"   
       );

    /* Should never get here. */
	configASSERT(pdFALSE);
    return pdFALSE;
}
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

/*
 * Manual context switch.  The first thing we do is save the registers 
 * as like as an interrupt service provide do.
 */
void vPortYield( void )
{
	asm (  	";PUSH	IX                      \n\t"   // done by the compile
			";LD	IX,0                    \n\t"   // done by the compile
			";ADD	IX,SP                   \n\t"   // done by the compile
			"xref   _pxCurrentTCB           \n\t"  	// Current Task context
			"di								\n\t"	// disable interrupts
			"pop	ix						\n\t"	// getback the callers ix register
			"PUSH	AF                      \n\t"   // Push all registern on callers stack 
			"PUSH	IY                      \n\t"   //    like an nested interrupt would be do	
			"PUSH	BC                      \n\t"   // af,iy,bc,de,hl,ix,af',bc',de'hl'
			"PUSH	DE                      \n\t"   
			"PUSH	HL                      \n\t"   
			"PUSH	IX                      \n\t"   
            "ex     af,     af'             \n\t"   
            "exx                            \n\t"   
            "push   af                      \n\t"   
            "push   bc                      \n\t"   
            "push   de                      \n\t"   
            "push   hl                      \n\t"   
            "ld     ix,     0               \n\t" 	// save new stackpositin on the task-control-block  
            "add    ix,     sp              \n\t"   
            "ld     hl,     (_pxCurrentTCB) \n\t"   
            "ld     (hl),   ix              \n\t"   
        );

    vTaskSwitchContext();							// switch context to other task if ready

	asm (   "xref   _pxCurrentTCB           \n\t"   
            "ld     hl,     (_pxCurrentTCB) \n\t"   
            "ld     hl,     (hl)            \n\t"   
            "ld     sp,     hl              \n\t"   
            "pop    hl                      \n\t"   // restore all registers
            "pop    de                      \n\t"   
            "pop    bc                      \n\t"   // hl',de'bc',af',ix,hl,de,bc,iy,af
            "pop    af                      \n\t"   
            "exx                            \n\t"   
            "ex     af,     af'             \n\t"   
			"POP	IX                      \n\t"   
			"POP	HL                      \n\t"   
			"POP	DE                      \n\t"   
			"POP	BC                      \n\t"   
			"POP	IY                      \n\t"   
			"POP	AF                      \n\t"   
			"EI                      		\n\t"   // enable interrupts
			"RET                      		\n\t"   // return to task 
       );
}
/*-----------------------------------------------------------*/

/*
 * Context switch function used by the tick.  This must be identical to
 * vPortYield() from the call to vTaskSwitchContext() onwards.  The only
 * difference from vPortYield() is the tick count is incremented as the
 * call comes from the tick ISR.
 */

void nested_interrupt timer_isr(void)
{
  	asm (   ";PUSH	AF                      \n\t" 	// done by the compiter  
			";PUSH	IY                      \n\t"   // done by the compiter  
			";PUSH	BC                      \n\t"   // done by the compiter  
			";PUSH	DE                      \n\t"   // done by the compiter  
			";PUSH	HL                      \n\t"   // done by the compiter  
			";PUSH	IX                      \n\t"   // done by the compiter  
			";LD	IX,0                    \n\t"   // done by the compiter  
			";ADD	IX,SP                   \n\t"   // done by the compiter  
			"xref   _pxCurrentTCB           \n\t"   // current task control block
			"in0    a,      (62h)           \n\t"	// reset timer interrupt .flag
            "ex     af,     af'             \n\t"   // save secound register set to
            "exx                            \n\t"   
            "push   af                      \n\t"   
            "push   bc                      \n\t"   
            "push   de                      \n\t"   // af,iy,bc,de,hl,ix,af',bc',de'hl'
            "push   hl                      \n\t"   
            "ld     ix,     0               \n\t"   
            "add    ix,     sp              \n\t"   
            "ld     hl,     (_pxCurrentTCB) \n\t"   // save old task stack
            "ld     (hl),   ix              \n\t"   
        );
	
#if configUSE_PREEMPTION == 1
    /*
     * Tick ISR for preemptive scheduler.  We can use a naked attribute as
     * the context is saved at the start of vPortYieldFromTick().  The tick
     * count is incremented after the context is saved.
     */
    //vPortYieldFromTick();
	xTaskIncrementTick();
    vTaskSwitchContext();

#else
    /*
     * Tick ISR for the cooperative scheduler.  All this does is increment the
     * tick count.  We don't need to switch context, this can only be done by
     * manual calls to taskYIELD();
     */
    // vTaskIncrementTick();
	vTaskSwitchContext();
#endif
    asm (   "xref   _pxCurrentTCB           \n\t"   
            "ld     hl,     (_pxCurrentTCB) \n\t"   // new task context
            "ld     hl,     (hl)            \n\t"   
            "ld     sp,     hl              \n\t"   // restore all registers
            "pop    hl                      \n\t"   
            "pop    de                      \n\t"   
            "pop    bc                      \n\t"   
            "pop    af                      \n\t"   // hl',de'bc',af',ix,hl,de,bc,iy,af
            "exx                            \n\t"   
            "ex     af,     af'             \n\t"   
			"ld		ix,		0				\n\t"	
			"add    ix,     sp              \n\t"   
			";LD	SP,IX                   \n\t" 	// done by the compiter    
			";POP	IX                      \n\t"   // done by the compiter  
			";POP	HL                      \n\t"   // done by the compiter  
			";POP	DE                      \n\t"   // done by the compiter  
			";POP	BC                      \n\t"   // done by the compiter  
			";POP	IY                      \n\t"   // done by the compiter  
			";POP	AF                      \n\t"   // done by the compiter  
			";EI                      		\n\t"   // done by the compiter  
			";RETI                      	\n\t"   // done by the compiter  
       );

}


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

