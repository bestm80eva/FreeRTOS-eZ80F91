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
	
	
	File: 	protmacro.h
			Part of FreeRTOS Port for the eZ80F91 Development Kit eZ80F910300ZCOG
			See www.zilog.com for desciption.


	Developer:
	JSIE	 Juergen Sievers <JSievers@NadiSoft.de>

	150804:	JSIE Start this port.
	
*/

#ifndef PORTMACRO_H
#define PORTMACRO_H
//#include <stdint.h>		// types used by 

/*-----------------------------------------------------------
 * Port specific definitions.  
 *
 * The settings in this file configure FreeRTOS correctly for the
 * given hardware and compiler.
 *
 * These settings should not be altered.
 *-----------------------------------------------------------
 */

/* Type definitions. */
typedef CHAR	portCHAR;
typedef FLOAT32	portFLOAT;
typedef FLOAT32	portDOUBLE;
typedef INT32	portLONG;
typedef INT16	portSHORT;
typedef UINT32	portULONG;
typedef UINT16	portUSHORT;
typedef UINT24	portPOINTER_SIZE_TYPE;

typedef UINT24	StackType_t;
typedef INT24	BaseType_t;
typedef UINT24	UBaseType_t;


/* Not supported in the ZDS II. */
#define inline
#define FAR

#if ( configUSE_16_BIT_TICKS == 1 )
   typedef portUSHORT portTickType;
   #define portMAX_DELAY ( portTickType ) 0xffff
#elif ( configUSE_24_BIT_TICKS == 1 )
   typedef UINT24 portTickType;
   #define portMAX_DELAY ( portTickType ) 0xffffff
#else
   typedef portLONG portTickType;
   #define portMAX_DELAY ( portTickType ) 0xffffffff
#endif

typedef portTickType		TickType_t;

/*-----------------------------------------------------------*/

/* Critical section management. */
#define portENTER_CRITICAL()     asm("ld   a,  i");      \
                                 asm("push af");         \
                                 asm("di")
#define portEXIT_CRITICAL()      asm("pop  af");         \
                                 asm("di");              \
                                 asm("jp   po,  $+5");   \
                                 asm("ei")

#define portDISABLE_INTERRUPTS() DI()
#define portENABLE_INTERRUPTS()  EI()
#define portNOP()                asm("nop")
/*-----------------------------------------------------------*/

/* Architecture specifics. */
#define portSTACK_GROWTH         ( -1 )
#define portTICK_PERIOD_MS       ( ( portTickType ) 1000 / configTICK_RATE_HZ )
#define portTICK_MS(x)			 ( (x) * portTICK_PERIOD_MS)
#define portBYTE_ALIGNMENT       4	
#define TIMER_VECTOR             0x54
/*-----------------------------------------------------------*/

/* Kernel utilities. */
extern void vPortYield( void );
#define portYIELD()              vPortYield()
/*-----------------------------------------------------------*/

/* Task function macros as described on the FreeRTOS.org WEB site. */
#define portTASK_FUNCTION_PROTO( vFunction, pvParameters ) void vFunction( void *pvParameters )
#define portTASK_FUNCTION( vFunction, pvParameters ) void vFunction( void *pvParameters )

#define portINLINE

#define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS StatsTimerInit
#define portALT_GET_RUN_TIME_COUNTER_VALUE StatsTimerGet
void StatsTimerInit();
UINT32 StatsTimerGet();

#endif /* PORTMACRO_H */
