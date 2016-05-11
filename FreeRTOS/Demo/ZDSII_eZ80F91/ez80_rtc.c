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
	
	
	File: 	ez80_rtc.c
			module rtc driver.
			Part of FreeRTOS Port for the eZ80F91 Development Kit eZ80F910300ZCOG
			See www.zilog.com for desciption.


	Developer:
	JSIE	 Juergen Sievers <JSievers@NadiSoft.de>

	150804:	JSIE Start this port.
	
*/
#include "FreeRTOS.h"
#include "ez80_tty.h"
#include "ez80_rtc.h"#include <eZ80F91.h>

// Define Months and Daysconst char *const dow[7] = {"Mo","Di","Mi","Do","Fr","Sa","So"};
const char *const mon[12]= {"Jan","Feb","Mär","Apr","Mai","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};

#define RTCREG(x) (*(volatile unsigned char __INTIO *)(0xE0 + (x)))


uint16_t flash_year /*_At 0x1FFFF0 */ = 2016;


BaseType_t chkRTCPowerLost()
{
	uint16_t now = (uint16_t)RTC_CEN * 100 + RTC_YR;
	return  (now != flash_year) ? pdTRUE : pdFALSE;
}

void setRTC( const rtc_t *data)
{
	uint16_t idx;
	uint16_t bmsk;
	configASSERT(data && data->flg < 0x2000);
	
	RTC_CTRL |= 0x01;	 // unlock RTC
	for(bmsk = 1, idx = 0; data->flg >= bmsk; idx++, bmsk <<= 1)
		if(data->flg & bmsk)
			RTCREG(idx) = data->d.buff[idx];
	RTC_CTRL &= ~0x01;	 // lock RTC

	if(chkRTCPowerLost() && data->flg & RID_YR && data->flg & RID_CEN)
	{
		#if 0
		if(FLASH_CTL & 0x08)
		{
			FLASH_PAGE = ((uint24_t)&flash_year >> 15) & 0x07;
			FLASH_ROW  = ((uint24_t)&flash_year >>  8) & 0x7F;
			FLASH_COL  =  (uint24_t)&flash_year & 0xFF;
			FLASH_DATA = RTC_YR;
			FLASH_PAGE = (((uint24_t)&flash_year+1) >> 15) & 0x07;
			FLASH_ROW  = ((uint24_t)(&flash_year+1) >>  8) & 0x7F;
			FLASH_COL  =  ((uint24_t)&flash_year+1) & 0xFF;
			FLASH_DATA = RTC_CEN;
		}
		else
		#endif	
			flash_year = ((uint16_t)RTC_CEN * 100) + RTC_YR;
	}		
}

void getRTC( rtc_t *data)
{
	uint16_t idx;
	uint16_t bmsk;
	configASSERT(data && data->flg < 0x2000);
	
	for(bmsk = 1, idx = 0; data->flg >= bmsk; idx++, bmsk <<= 1)
		if(data->flg & bmsk)
			data->d.buff[idx] = RTCREG(idx);
}


void setAlarm(alarm_t alarm, uint16_t flag, uint8_t dow, uint8_t hrs, uint8_t min, uint8_t sec)
{
	rtc_t p;
	
	p.flg = 0;
	
	RTC_CTRL &= ~(1 << 6);
	if(flag & ADOW_EN) { p.d.r.ADOW = dow; p.flg |= (1 << RID_ADOW);}
	if(flag & AHRS_EN) { p.d.r.AHRS = hrs; p.flg |= (1 << RID_AHRS);}
	if(flag & AMIN_EN) { p.d.r.AMIN = min; p.flg |= (1 << RID_AMIN);}
	if(flag & ASEC_EN) { p.d.r.ASEC = sec; p.flg |= (1 << RID_ASEC);}
	setRTC(&p);
	
	
	if(alarm)
	{
		_set_vector(RTC_IVECT, alarm);
		RTC_CTRL |= (1 << 6);
	} 
	RTC_ACTRL = flag & 0x0F;
}

char *getsAlarm(char *s, size_t sz)
{
	rtc_t p;
	uint8_t a = RTC_ACTRL;
	p.flg = (1 << RID_ADOW) | (1 << RID_AHRS) | (1 << RID_AMIN) | (1 << RID_ASEC); 
	getRTC(&p);
	snprintf(s, sz, "%s. %2.2hhu:%2.2hhu:%2.2hhu [%c%c%c%c%c]",dow[p.d.r.ADOW-1], p.d.r.AHRS, p.d.r.AMIN, p.d.r.ASEC
		,(char)(a&ADOW_EN? 'W':'w'),(char)(a&AHRS_EN? 'H':'h'),(char)(a&AMIN_EN? 'M':'m'),(char)(a&ASEC_EN? 'S':'s')
		,(char)(RTC_CTRL & (1 << 6)?'I':'i'));
	return s;
}

void setTime(uint8_t hrs, uint8_t min, uint8_t sec)
{
	rtc_t p;
	p.d.r.HRS = hrs;
	p.d.r.MIN = min;
	p.d.r.SEC = sec;
	p.flg = (1 << RID_HRS) | (1 << RID_MIN) | (1 << RID_SEC); 
	setRTC(&p);
}

char *getsTime(char *s, size_t sz)
{
	rtc_t p;
	p.flg = (1 << RID_HRS) | (1 << RID_MIN) | (1 << RID_SEC); 
	getRTC(&p);
	snprintf(s, sz, "%2.2hhu:%2.2hhu:%2.2hhu", p.d.r.HRS, p.d.r.MIN, p.d.r.SEC);
	return s;
}

void setDate(uint8_t day, uint8_t mon, uint16_t year)
{
	rtc_t p;
	p.d.r.DOM = day;
	p.d.r.MON = mon;
	p.d.r.CEN = year/100;
	p.d.r.CEN = year%100;
	p.flg = (1 << RID_DOM) | (1 << RID_MON) | (1 << RID_CEN) | (1 << RID_YR); 
	setRTC(&p);
}

char *getsDate(char *s, size_t sz)
{
	rtc_t p;
	p.flg = (1 << RID_DOW) | (1 << RID_DOM) | (1 << RID_MON) | (1 << RID_CEN) | (1 << RID_YR); 
	getRTC(&p);
	if(chkRTCPowerLost())
		snprintf(s, sz,"??. ?? ???. ????");
	else
		snprintf(s, sz,"%s. %2.2hhu %s. %4.4hu", dow[p.d.r.DOW-1], p.d.r.DOM, mon[p.d.r.MON-1], p.d.r.CEN*100+p.d.r.YR);
	return s;
}


uint8_t initRTC()
{
	uint8_t res = RTC_CTRL;
	RTC_CTRL = 0;
	return res;
}