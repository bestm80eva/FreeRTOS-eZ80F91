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
	
	
	File: 	FreeRTOSConfig.h
			Part of FreeRTOS Port for the eZ80F91 Development Kit eZ80F910300ZCOG
			See www.zilog.com for desciption.
			Map ZiLOG's types to FreeRTOS useable ones
		
		Every FreeRTOS application must have a FreeRTOSConfig.h header file in its pre-processor include path. 
		FreeRTOSConfig.h tailors the RTOS kernel to the application being built. 
		It is therefore specific to the application, not the RTOS.

	Developer:
	JSIE	 Juergen Sievers <JSievers@NadiSoft.de>
	
	Leagend:
	150804:	SIE Start this port by founding on Richard Barry's Port March 11, 2010
			http://interactive.freertos.org/entries/126322-eZ80-using-ZDSII-4-11
	
*/

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/*
 * Overall needed stuff
 */

#include <ez80.h>	// EZ80F91 definitions.
typedef int F91_EMAC_CONF_t;
/*
 * Enable and configure Demo-Application PSP Modules
 */
#define configMIXEDMODE MIXEDMODE

#define INCLUDE_LED5x7	1 		// Include LED 5x7 Driver
#define INCLUDE_BUTTONS	1		// Include plattform buttons
#define INCLUDE_CONSOLE 1 		// Include serial port 0
#define INCLUDE_MODEM 	0 		// Include serial port 1
#define INCLUDE_EMAC	1		// Include Ethernet Driver
#define INCLUDE_MONITOR	1		// Include Demo-Monitor
#define INCLUDE_RTC		1		// Include Realtime Clock
#define INCLUDE_SNTP	0		// NTP Client
#define MKQUOTE(x)	#x
#define MKVERSION(x)	MKQUOTE(x)

#define DEVKIT 		"EZ80F910300ZCOG"
#define ZIDE		"ZDS II Acclaim! 5.2.1 Build:" MKVERSION(__ZDATE__)
#define AUTOR		"Juergen Sievers"
#define AUTORMAIL	"JSievers@NadiSoft.de"
#define VERSION		MKVERSION(DEMOVERSION)
/*-----------------------------------------------------------
 * Application specific definitions.
 *
 * These definitions should be adjusted for your particular hardware and
 * application requirements.
 *
 * THESE PARAMETERS ARE DESCRIBED WITHIN THE 'CONFIGURATION' SECTION OF THE
 * FreeRTOS API DOCUMENTATION AVAILABLE ON THE FreeRTOS.org WEB SITE. 
 *----------------------------------------------------------*/


#define PRIO_LED 		tskIDLE_PRIORITY + 2
#define PRIO_SYSINFO   	tskIDLE_PRIORITY + 0
#define PRIO_LED5x7	  	tskIDLE_PRIORITY + 3
#define PRIO_CPMIO 	  	tskIDLE_PRIORITY + 2
#define PRIO_CPM22		tskIDLE_PRIORITY + 1

#if INCLUDE_LED5x7 == 1 
#define LED5x7_FRAMES	pdMS_TO_TICKS(  5)	// display refresch delay 
#define LED5x7_SHIFTT	pdMS_TO_TICKS( 60)	// LED5x7_FRAMES delays between shifts
#define LED5x7_CDELAY	pdMS_TO_TICKS(300)	// LED5x7_FRAMES deleys between letters
#define LED5x7_QUEUES	80					// chars on queue
#endif	/* INCLUDE_LED5x7 */

#if INCLUDE_BUTTONS
#define BUTTON_PRELL 	pdMS_TO_TICKS(50)	// Entprellung
#endif


// Console setting
#define DEF_PRINTFSIZE		32767			// for the fucking sprintf uses snprintf
#define CLOCK_DIVISOR_16	16				// baudrate generator setting

#if INCLUDE_CONSOLE	== 1
#define CONOUTWAIT			pdMS_TO_TICKS(0)	// max wait for console output queue
#define CONINWAIT			pdMS_TO_TICKS(0)	// max wait for console input queue
#define CONBAUDRATE			BAUD_115200			// console baud rate
#define CONFIFO_TRGLVL		FIFO_TRGLVL_8		// 16byte fifo trigger level	
#define CONDATABITS			DATABITS_8		
#define CONSTOPBITS			STOPBITS_1
#define CONPARITY			PAR_NOPARITY
#define CONFLOWCONTROL		ENABLE_HWFLOW_CONTROL
#endif /* INCLUDE_CONSOLE */

// Modem setting
#if INCLUDE_MODEM	== 1
#define MODOUTWAIT			pdMS_TO_TICKS(10)	// max wait for uart output queue
#define MODINWAIT			pdMS_TO_TICKS(10)	// max wait for console input queue
#define MODBAUDRATE			BAUD_115200			// uart baud rate
#define MODFIFO_TRGLVL		FIFO_TRGLVL_8		// 16byte fifo trigger level	
#define MODDATABITS			DATABITS_8		
#define MODSTOPBITS			STOPBITS_1
#define MODPARITY			PAR_NOPARITY
#define MODFLOWCONTROL		ENABLE_HWFLOW_CONTROL
#endif /* INCLUDE_MODEM */

#define configCONSOLEPORT	0	// 0 use sio 0 CONSOLE, 1 use sio 1 MODEM

// Exports from LinkerScript
extern uint8_t _heaptop;	
extern uint8_t _heapbot;

/* MPU IDE II specifics		*/
void * set_vector(unsigned int vector, void(*isp)(void));
/* Missing stdc functions	*/
int strcasecmp(const char *s1, const char *s2);


/* CLI configuration								*/
#define configINCLUDE_TRACE_RELATED_CLI_COMMANDS 0
#define configCOMMAND_INT_MAX_OUTPUT_SIZE		256
#define configINCLUDE_QUERY_HEAP_COMMAND 		1
#define configINCLUDE_DEMO_DEBUG_STATS 			0

/*
 * Start of FreeRTOS configuration stuff
 * THESE PARAMETERS ARE DESCRIBED WITHIN THE 'CONFIGURATION' SECTION OF THE
 * FreeRTOS API DOCUMENTATION AVAILABLE ON THE FreeRTOS.org WEB SITE. 
 */
 
#define configUSE_PREEMPTION                    1
#define configUSE_PORT_OPTIMISED_TASK_SELECTION 0
#define configUSE_TICKLESS_IDLE                 0
#define configCPU_CLOCK_HZ                      50000000UL
#define configTICK_RATE_HZ                      500
#define configMAX_PRIORITIES                    16
#define configMINIMAL_STACK_SIZE                256
#define configMAX_TASK_NAME_LEN                 16
#define configUSE_16_BIT_TICKS                  0
#define configUSE_24_BIT_TICKS					1	/* standard wide on eZ80 MPU */
#define configIDLE_SHOULD_YIELD                 0
#define configUSE_TASK_NOTIFICATIONS            1
#define configUSE_MUTEXES                       1
#define configUSE_RECURSIVE_MUTEXES             1
#define configUSE_COUNTING_SEMAPHORES           1
#define configUSE_ALTERNATIVE_API               0 /* Deprecated! */
#define configQUEUE_REGISTRY_SIZE               10
#define configUSE_QUEUE_SETS                    0
#define configUSE_TIME_SLICING                  0
#define configUSE_NEWLIB_REENTRANT              0
#define configENABLE_BACKWARD_COMPATIBILITY     0
#define configNUM_THREAD_LOCAL_STORAGE_POINTERS 0

/* Memory allocation related definitions. */
#define configSUPPORT_STATIC_ALLOCATION         1
#define configSUPPORT_DYNAMIC_ALLOCATION        1
#define configHEAP_START						(uint8_t*) &_heapbot
#define configTOTAL_HEAP_SIZE					(size_t ) (&_heaptop - &_heapbot)
#define configAPPLICATION_ALLOCATED_HEAP 		1


/* Hook function related definitions. */
#define configUSE_IDLE_HOOK                     0
#define configUSE_TICK_HOOK                     1
#define configCHECK_FOR_STACK_OVERFLOW          1
#define configUSE_MALLOC_FAILED_HOOK            0

/* Run time and task stats gathering related definitions. */
#define configGENERATE_RUN_TIME_STATS           0
#define configUSE_TRACE_FACILITY                1
#define configUSE_STATS_FORMATTING_FUNCTIONS    1

/* Co-routine related definitions. */
#define configUSE_CO_ROUTINES                   0
#define configMAX_CO_ROUTINE_PRIORITIES         1

/* Software timer related definitions. */
#define configUSE_TIMERS                        1
#define configTIMER_TASK_PRIORITY               (configMAX_PRIORITIES-1)
#define configTIMER_QUEUE_LENGTH                10
#define configTIMER_TASK_STACK_DEPTH            configMINIMAL_STACK_SIZE

/* Interrupt nesting behaviour configuration. */

/* not needed on EZ80F91 MPUs
#define configKERNEL_INTERRUPT_PRIORITY         [dependent of processor]
#define configMAX_SYSCALL_INTERRUPT_PRIORITY    [dependent on processor and application]
#define configMAX_API_CALL_INTERRUPT_PRIORITY   [dependent on processor and application]
 */

/* Define to trap errors during development. */
#define configASSERT( x ) if( ( x ) == 0) vAssertCalled( __FILE__, __LINE__ )
#ifdef 	configASSERT
void vAssertCalled( const char* file, int line );
#endif

/* FreeRTOS MPU specific definitions. */
#define configINCLUDE_APPLICATION_DEFINED_PRIVILEGED_FUNCTIONS 0

/* Optional functions - most linkers will remove unused functions anyway. */
#define INCLUDE_vTaskPrioritySet                1
#define INCLUDE_uxTaskPriorityGet               1
#define INCLUDE_vTaskDelete                     1
#define INCLUDE_vTaskSuspend                    1
#define INCLUDE_xResumeFromISR                  1
#define INCLUDE_vTaskDelayUntil                 1
#define INCLUDE_vTaskDelay                      1
#define INCLUDE_xTaskGetSchedulerState          1
#define INCLUDE_xTaskGetCurrentTaskHandle       1
#define INCLUDE_uxTaskGetStackHighWaterMark     1
#define INCLUDE_xTaskGetIdleTaskHandle          1
#define INCLUDE_eTaskGetState                   1
#define INCLUDE_xEventGroupSetBitFromISR        1
#define INCLUDE_xTimerPendFunctionCall          1

/* A header file that defines trace macro can be included here. */
#define configINCLUDE_TRACE_FACILITY            0
#include "trcKernelPort.h"

#endif /* FREERTOS_CONFIG_H */
