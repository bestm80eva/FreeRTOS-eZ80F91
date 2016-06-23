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
	
	
	File: 	ez80_rtc.c
			module rtc driver.
			Part of FreeRTOS Port for the eZ80F91 Development Kit eZ80F910300ZCOG
			See www.zilog.com for desciption.


	Developer:
	JSIE	 Juergen Sievers <JSievers@NadiSoft.de>

	150804:	JSIE Start this port.
	
*/
#include "FreeRTOS.h"
#include "task.h"
#include "ez80_tty.h"
#include "ez80_rtc.h"

#include "time.h"#include <eZ80F91.h>

// Define Months and Daysconst char *const dow[7] = {"Mo","Di","Mi","Do","Fr","Sa","So"};
const char *const mon[12]= {"Jan","Feb","Mär","Apr","Mai","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};

#define RTCREG(x) (*(volatile unsigned char __INTIO *)(0xE0 + (x)))

//1st january 2000 is 946681200 secondes after Epoch.

static const unsigned short days[4][12] =
{
    {   0,  31,  60,  91, 121, 152, 182, 213, 244, 274, 305, 335},
    { 366, 397, 425, 456, 486, 517, 547, 578, 609, 639, 670, 700},
    { 731, 762, 790, 821, 851, 882, 912, 943, 974,1004,1035,1065},
    {1096,1127,1155,1186,1216,1247,1277,1308,1339,1369,1400,1430},
};

time_t iTimeZone = 0;

unsigned int date_time_to_epoch( rtc_t* date_time)
{
    unsigned int second = date_time->d.buff[RID_SEC];  // 0-59
    unsigned int minute = date_time->d.buff[RID_MIN];  // 0-59
    unsigned int hour   = date_time->d.buff[RID_HRS];    // 0-23
    unsigned int day    = date_time->d.buff[RID_DOM]-1;   // 0-30
    unsigned int month  = date_time->d.buff[RID_MON]-1; // 0-11
    unsigned int year   = date_time->d.buff[RID_YR];    // 0-99
    return (((year/4*(365*4+1)+days[year%4][month]+day)*24+hour)*60+minute)*60+second;
}


void epoch_to_date_time(rtc_t* date_time, time_t epoch)
{
	unsigned int years;
	unsigned int year;
	unsigned int month;
	
    date_time->d.buff[RID_SEC] = epoch % 60; 
	epoch /= 60;
    date_time->d.buff[RID_MIN] = epoch % 60; 
	epoch /= 60;
    date_time->d.buff[RID_HRS] = epoch % 24; 
	epoch /= 24;

    years = epoch / (365 * 4 + 1) * 4; 
	epoch %= 365 * 4 + 1;

    for (year = 3; year > 0; year--)
    {
        if (epoch >= days[year][0])
            break;
    }

    
    for (month = 11; month > 0; month--)
    {
        if (epoch >= days[year][month])
            break;
    }

    date_time->d.buff[RID_YR]  = years + year;
    date_time->d.buff[RID_MON] = month + 1;
    date_time->d.buff[RID_DOM] = (epoch - days[year][month] + 1) % 100;
	date_time->d.buff[RID_CEN] = (epoch - days[year][month] + 1) / 100;
	
	date_time->flg = (1 << RID_SEC)	// Real-Time Clock Seconds Register 
			  |(1 << RID_MIN)	// Real-Time Clock Minutes Register 
			  |(1 << RID_HRS)	// Real-Time Clock Hours Register   
			  |(1 << RID_DOM)	// Real-Time Clock Day-of-the-Month Register
			  |(1 << RID_MON)	// Real-Time Clock Month Register
			  |(1 << RID_YR)	// Real-Time Clock Year Register 
			  |(1 << RID_CEN);	// Real-Time Clock Century Register

}


time_t FreeRTOS_time( time_t *t)
{
	time_t res;
	rtc_t date;
	date.flg = (1 << RID_SEC)	// Real-Time Clock Seconds Register 
			  |(1 << RID_MIN)	// Real-Time Clock Minutes Register 
			  |(1 << RID_HRS)	// Real-Time Clock Hours Register   
			  |(1 << RID_DOM)	// Real-Time Clock Day-of-the-Month Register
			  |(1 << RID_MON)	// Real-Time Clock Month Register
			  |(1 << RID_YR)	// Real-Time Clock Year Register 
			  |(1 << RID_CEN);	// Real-Time Clock Century Register
	
	getRTC( &date);
	
	res = date_time_to_epoch(&date);
	if(t)
		*t = res;
	return res;
}

time_t FreeRTOS_get_secs_msec( time_t *t)
{
	return FreeRTOS_time(t);
}


time_t FreeRTOS_set_secs_msec( time_t *uxCurrentSeconds, time_t *uxCurrentMS )
{
	rtc_t date;
	time_t t = uxCurrentSeconds ? *uxCurrentSeconds : 0;
	
	if(uxCurrentMS && *uxCurrentMS >= 500)
		t++;
	
	return t;
}

void FreeRTOS_gmtime_r( time_t *uxCurrentSeconds, FF_TimeStruct_t *xTimeStruct )
{
	rtc_t date;
	time_t t = uxCurrentSeconds ? *uxCurrentSeconds:0;
	
	epoch_to_date_time( &date, t);
	if(xTimeStruct)
	{
		xTimeStruct->tm_hour = date.d.buff[RID_HRS];
		xTimeStruct->tm_mday = date.d.buff[RID_DOM];
		xTimeStruct->tm_min  = date.d.buff[RID_MIN];
		xTimeStruct->tm_mon  = date.d.buff[RID_MON];
		xTimeStruct->tm_sec  = date.d.buff[RID_SEC];
		xTimeStruct->tm_year = date.d.buff[RID_CEN] * 100 + date.d.buff[RID_YR];
	}
}

/*
 * The ez80ef91 rtc isn't able to detect a power-lost condition.
 * To get able to handle power-lost we make a small value-check 
 * about the rtc's time-registers. For security reasons you shoud 
 * flash the last  year, month or day (depending on how long the 
 * device stays powered off) and check the last flashed values 
 * with the RTC registers.
 * For example (unsigned) rtc-year - flash-yaer <= 1 ...
 */
BaseType_t chkRTCPowerLost()
{
	return 
		RTCREG(RID_SEC) <  60 &&
		RTCREG(RID_MIN) <  60 &&
		RTCREG(RID_HRS) <  24 &&
		RTCREG(RID_DOW) >   0 && RTCREG(RID_DOW) <  8 &&
		RTCREG(RID_DOM) >   0 && RTCREG(RID_DOM) < 32 &&
		RTCREG(RID_MON) >   0 && RTCREG(RID_MON) < 13 &&
		RTCREG(RID_CEN) < 100 &&
		RTCREG(RID_YR)  < 100  ? pdFALSE : pdTRUE;
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

void setDate(uint8_t dow, uint8_t day, uint8_t mon, uint16_t year)
{
	rtc_t p;
	p.d.r.DOW = dow;
	p.d.r.DOM = day;
	p.d.r.MON = mon;
	p.d.r.CEN = year/100;
	p.d.r.YR  = year%100;
	p.flg = (1 << RID_DOW) | (1 << RID_DOM) | (1 << RID_MON) | (1 << RID_CEN) | (1 << RID_YR); 
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
	if(chkRTCPowerLost()) 
	{
		setDate(7, 15, 5, 2016);
		setTime(1, 55, 0);
	}
		
	return res;
}