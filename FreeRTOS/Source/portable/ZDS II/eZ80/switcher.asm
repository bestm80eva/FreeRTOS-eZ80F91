COMMENT #
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
	
	
	File: 	switcher.asm
			save and restor processor context 
			Part of FreeRTOS Port for the eZ80F91 Development Kit eZ80F910300ZCOG
			See www.zilog.com for desciption.
			uart, console and faramted printout driver

	Developer:
	JSIE	 Juergen Sievers <JSievers@NadiSoft.de>

	160412:	JSIE removed from port.c implemented in assembler
	
*/
#

	.assume	ADL=1
	xref   	_pxCurrentTCB		;current task control block
    xref	_vTaskSwitchContext
	xref	_xTaskIncrementTick
	xref	_prvSetupTimerInterrupt
	
	xdef	_vPortYield
	xdef	_vPortYieldFromTick
	xdef 	_xPortStartScheduler

; Save processor context on current TCB
PUSHALL MACRO
	PUSH	AF
	PUSH	BC
	PUSH	DE
	PUSH	HL
	PUSH	IX
	PUSH	IY
	EX		AF,	AF'
	EXX
	PUSH	AF
	PUSH	BC
	PUSH	DE
	PUSH	HL
	LD		IX,	0
	ADD		IX,	SP
	LD		HL,	(_pxCurrentTCB)	
	LD		(HL),	IX
	MACEND

; Restor processor context from current TCB
POPALL	MACRO
	LD		HL,	(_pxCurrentTCB)	
	LD		HL,	(HL)
	LD		SP,	HL
	POP		HL
	POP		DE
	POP		BC
	POP		AF
	EXX
	EX		AF,	AF'
	POP		IY
	POP		IX
	POP		HL
	POP		DE
	POP		BC
	POP		AF
	MACEND

	SEGMENT CODE

; Setup scheduler timer and restor processor to current PCB	
_xPortStartScheduler:
    call	_prvSetupTimerInterrupt
	POPALL
	RET

; Manual switch context
_vPortYield:
	PUSHALL
	call	_vTaskSwitchContext
	LD		HL,	(_pxCurrentTCB)	
	POPALL
	RET

; Scheduler triggert context switch
_vPortYieldFromTick:
	PUSHALL
	IN0		A,	(62h)	; ack timer interrupt
	call	_xTaskIncrementTick
	call	_vTaskSwitchContext
	POPALL
	EI
	RETI

	END