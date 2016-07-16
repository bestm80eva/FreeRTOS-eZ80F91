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
	
	
	File: 	main.c
			entry for ez80f91 Demo poject
			Part of FreeRTOS Port for the eZ80F91 Development Kit eZ80F910300ZCOG
			See www.zilog.com for desciption.
			uart, console and faramted printout driver


	Developer:
	JSIE	 Juergen Sievers <JSievers@NadiSoft.de>

	150804:	JSIE Start this port.
	
*/
/* Scheduler include files. */
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "semphr.h "
#include "list.h"

#include "FreeRTOS_IP.h"
#include "FreeRTOS_IP_Private.h"
#include "FreeRTOS_sockets.h"
#include "FreeRTOS_CLI.h"
#include "NetworkInterface.h"

#include "time.h"
#include "ez80_rtc.h"
#include "ez80_tty.h"
#include "ez80_leds.h"
#include "ez80_buttons.h"
#include "monitor\monitor.h"


void prvSRand( uint32_t );

void vStartTCPCommandInterpreterTask( uint16_t usStackSize, uint32_t ulPort, UBaseType_t uxPriority );
void vRegisterSampleCLICommands( void );
void vRegisterTCPCLICommands( void );

void vApplicationIdleHook( void );
void xApplicationDNSQueryHook(){}
void vApplicationPingReplyHook( ePingReplyStatus_t eStatus, uint16_t usIdentifier ){}

/* Define the network addressing.  These parameters will be used if either
ipconfigUDE_DHCP is 0 or if ipconfigUSE_DHCP is 1 but DHCP auto configuration
failed. */
static const uint8_t ucIPAddress[ 4 ] = { 192, 168, 1, 2 };
static const uint8_t ucNetMask[ 4 ] = { 255, 255, 255, 0 };
static const uint8_t ucGatewayAddress[ 4 ] = { 192, 168, 1, 1 };
/* The following is the address of an OpenDNS server. */
static const uint8_t ucDNSServerAddress[ 4 ] = { 192, 168, 1, 1 };

#ifdef INCLUDE_LED5x7		
	void TaskLED( void *pvParameters );
#endif

#ifdef CPM22
char cpmmem[0x10000] _Align 0x10000;
void prvTCPCpmIOTask( void *ram );
#endif

void StatsTimerInit()
{
	
}

UINT32 StatsTimerGet()
{
	return xTaskGetTickCount();
}

static const uint8_t ucMACAddress[ 6 ] = {0x00,0x90,0x23,0x00,0x01,0x02};

int main( void )
{
	BaseType_t	res = pdPASS;
	rtc_t	rtc;
	TickType_t xTimeNow;
#if configUSE_TRACE_FACILITY == 1
	vTraceInitTraceData();
	vTraceSetISRProperties(TIID_EthRx, "i_EthRx", IPRI_EthRx);
	vTraceSetISRProperties(TIID_EthTx, "i_EthTx", IPRI_EthTx);
	vTraceSetISRProperties(TIID_EthSys, "i_EthSys", IPRI_EthSys);
	vTraceSetISRProperties(TIID_Tmr0, "i_Tmr0", IPRI_Tmr0);
	vTraceSetISRProperties(TIID_Tmr1, "i_Tmr1", IPRI_Tmr1);
	vTraceSetISRProperties(TIID_Tmr2, "i_Tmr2", IPRI_Tmr2);
	vTraceSetISRProperties(TIID_rtc, "i_rtc", IPRI_rtc);
	vTraceSetISRProperties(TIID_uart0, "i_uart0", IPRI_uart0);
	vTraceSetISRProperties(TIID_uart1, "i_uart1", IPRI_uart1);
	vTraceSetISRProperties(TIID_button0, "i_swb0", IPRI_button0);
	vTraceSetISRProperties(TIID_button1, "i_swb1", IPRI_button1);
	vTraceSetISRProperties(TIID_button2, "i_swb2", IPRI_button2);
	// uiTraceStart(); 
	// vTraceStop()
#endif
	
	/* Seed the random number generator. */
	xTimeNow = xTaskGetTickCount();
	prvSRand( ( uint32_t ) xTimeNow );

	
	
#if INCLUDE_CONSOLE == 1 || INCLUDE_MODEM == 1  		// Include serial port 0 or serial port 1
	initSerial();
#endif

#if INCLUDE_RTC	== 1		// Include Realtime Clock
	initRTC();
#endif

#if INCLUDE_BUTTONS == 1
	initButtons();
#endif

	/* Initialise the RTOS's TCP/IP stack.  The tasks that use the network
    are created in the vApplicationIPNetworkEventHook() hook function
    below.  The hook function is called when the network connects. */
	res = FreeRTOS_IPInit( ucIPAddress, ucNetMask, ucGatewayAddress, ucDNSServerAddress, ucMACAddress );
    //vRegisterSampleCLICommands();
	vRegisterTCPCLICommands();
	vRegisterMonitorCLICommands();
	
	// Commandline interface (Telnet port 5010)
	vStartTCPCommandInterpreterTask( 2048, 5010, tskIDLE_PRIORITY + 1);
	
	// Demo Sysinfo 
	// Connect putty 115200,8,1,n (ansi terminal) to get the status informations
	res = xTaskCreate( sysinfo, "SysInfo", configMINIMAL_STACK_SIZE*2, (void *)portMAX_DELAY, PRIO_SYSINFO, NULL);


#if INCLUDE_LED5x7	== 1
	initLED5x7();
	res = xTaskCreate( TaskLED, "TaskLED", configMINIMAL_STACK_SIZE, (void *)portMAX_DELAY, PRIO_LED5x7, NULL);
#endif

#ifdef CPM22 
	res = xTaskCreate( prvTCPCpmIOTask, "CPMIO", configMINIMAL_STACK_SIZE*2, (void*)cpmmem, PRIO_CPMIO, NULL);
#endif

	vTaskStartScheduler();

    return res;
}
/*-----------------------------------------------------------*/

#ifdef INCLUDE_LED5x7	
// Show a banner on the 5x7 LED Display
void TaskLED( void *pvParameters )
{
    TickType_t ticks = (int)pvParameters;
	CHAR line5x7[80];
	
    while(1)
    {
		int idx;
		
		strncpy(line5x7," * NadiSoft - ",sizeof(line5x7));
		getsDate(line5x7+strlen(line5x7),sizeof(line5x7) - strlen(line5x7));
		strcat(line5x7," - ");
		getsTime(line5x7+strlen(line5x7),sizeof(line5x7) - strlen(line5x7));
		idx = 0;
		
		while(idx < strlen(line5x7))
		{
			idx += LED5x7_puts(line5x7+idx,ticks);
			vTaskDelay(300);
		}
    }
}
#endif

/*-----------------------------------------------------------*/
#if configUSE_IDLE_HOOK == 1
void vApplicationIdleHook( void )
{

}	
#endif

/*-----------------------------------------------------------*/

#if configCHECK_FOR_STACK_OVERFLOW == 1
void vApplicationStackOverflowHook( TaskHandle_t xTask,signed char *pcTaskName )
{
	printf("Stack overflow at task %s\n",pcTaskName);
	while(1)
		asm("nop");
	asm("nop");
}
#endif

#ifdef configASSERT
void vAssertCalled( const char* file, int line )
{
	printf("ASSERTION %s, %i\n",file, line);
	while(1)
		asm("nop");
	asm("nop");
}
#endif

/* configUSE_STATIC_ALLOCATION is set to 1, so the application must provide an
implementation of vApplicationGetIdleTaskMemory() to provide the memory that is
used by the Idle task. */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer,
                                    StackType_t **ppxIdleTaskStackBuffer,
                                    uint32_t *pusIdleTaskStackSize )
{
/* If the buffers to be provided to the Idle task are declared inside this
function then they must be declared static - otherwise they will be allocated on
the stack and so not exists after this function exits. */
static StaticTask_t xIdleTaskTCB;
static StackType_t uxIdleTaskStack[ configMINIMAL_STACK_SIZE ];

    /* Pass out a pointer to the StaticTask_t structure in which the Idle task's
    state will be stored. */
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

    /* Pass out the array that will be used as the Idle task's stack. */
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;

    /* Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
    Note that, as the array is necessarily of type StackType_t,
    configMINIMAL_STACK_SIZE is specified in words, not bytes. */
    *pusIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}
/*-----------------------------------------------------------*/

/* configUSE_STATIC_ALLOCATION and configUSE_TIMERS are both set to 1, so the
application must provide an implementation of vApplicationGetTimerTaskMemory()
to provide the memory that is used by the Timer service task. */
void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer,
                                     StackType_t **ppxTimerTaskStackBuffer,
                                     uint32_t *pusTimerTaskStackSize )
{
/* If the buffers to be provided to the Timer task are declared inside this
function then they must be declared static - otherwise they will be allocated on
the stack and so not exists after this function exits. */
static StaticTask_t xTimerTaskTCB;
static StackType_t uxTimerTaskStack[ configTIMER_TASK_STACK_DEPTH ];

    /* Pass out a pointer to the StaticTask_t structure in which the Timer
    task's state will be stored. */
    *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;

    /* Pass out the array that will be used as the Timer task's stack. */
    *ppxTimerTaskStackBuffer = uxTimerTaskStack;

    /* Pass out the size of the array pointed to by *ppxTimerTaskStackBuffer.
    Note that, as the array is necessarily of type StackType_t,
    configMINIMAL_STACK_SIZE is specified in words, not bytes. */
    *pusTimerTaskStackSize = configMINIMAL_STACK_SIZE;
}
	
