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
	
	
	File: 	ez80_rtc.h
			module rtc driver.
			Part of FreeRTOS Port for the eZ80F91 Development Kit eZ80F910300ZCOG
			See www.zilog.com for desciption.


	Developer:
	JSIE	 Juergen Sievers <JSievers@NadiSoft.de>

	150804:	JSIE Start this port.
	
*/

#ifndef EZ80_RTC_H 
#define EZ80_RTC_H 

#include "stdint.h"

// Offset to RTC baseregister
enum {
	RID_SEC,	// Real-Time Clock Seconds Register 
	RID_MIN,	// Real-Time Clock Minutes Register 
	RID_HRS,	// Real-Time Clock Hours Register   
	RID_DOW,	// Real-Time Clock Day-of-the-Week Register
	RID_DOM,	// Real-Time Clock Day-of-the-Month Register
	RID_MON,	// Real-Time Clock Month Register
	RID_YR,		// Real-Time Clock Year Register 
	RID_CEN,	// Real-Time Clock Century Register
	RID_ASEC,	// Real-Time Clock Alarm Seconds Register
	RID_AMIN,	// Real-Time Clock Alarm Minutes Register
	RID_AHRS,	// Real-Time Clock Alarm Hours Register
	RID_ADOW	// Real-Time Clock Alarm Day-of-the-Week Register
};

enum {
	ASEC_EN = 0x01,
	AMIN_EN = 0x02,
	AHRS_EN = 0x04,
	ADOW_EN = 0x08
};
	
typedef struct
{
	uint16_t flg;	// Flag 1 = fied valid
	union {
		struct {
			uint8_t SEC;	// Real-Time Clock Seconds Register 00-3b
			uint8_t MIN;	// Real-Time Clock Minutes Register 00-3b
			uint8_t HRS;	// Real-Time Clock Hours Register   00-17
			uint8_t DOW;	// Real-Time Clock Day-of-the-Week Register  01-07
			uint8_t DOM;	// Real-Time Clock Day-of-the-Month Register 01-1f
			uint8_t MON;	// Real-Time Clock Month Register   01-0C
			uint8_t YR;		// Real-Time Clock Year Register    00-99
			uint8_t CEN;	// Real-Time Clock Century Register 00-99
			uint8_t ASEC;	// Real-Time Clock Alarm Seconds Register 00-3b
			uint8_t AMIN;	// Real-Time Clock Alarm Minutes Register 00-3b
			uint8_t AHRS;	// Real-Time Clock Alarm Hours Register   01-17
			uint8_t ADOW;	// Real-Time Clock Alarm Day-of-the-Week Register 00-07
		}r;
		uint8_t	buff[13];	// RTC bin buff
	}d;
} rtc_t;

typedef void(*alarm_t)(void);

BaseType_t chkRTCPowerLost();
void setRTC( const rtc_t *data);
void getRTC( rtc_t *data);
void setAlarm(alarm_t alarm,uint16_t flag, uint8_t dow, uint8_t hrs, uint8_t min, uint8_t sec);
char *getsAlarm(char *s, size_t sz);
void setTime(uint8_t hrs, uint8_t min, uint8_t sec);
char *getsTime(char *s, size_t sz);
void setDate(uint8_t dow, uint8_t day, uint8_t mon, uint16_t year);
char *getsDate(char *s, size_t sz);
uint8_t initRTC();

#endif /* EZ80_RTC_H */