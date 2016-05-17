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
	
	
	File: 	monitor.c
			plattform and module system information.
			Connect ansi-terminal to comport0 (115200,8,1,n) to see what's happen.
			Part of FreeRTOS Port for the eZ80F91 Development Kit eZ80F910300ZCOG
			See www.zilog.com for desciption.


	Developer:
	JSIE	 Juergen Sievers <JSievers@NadiSoft.de>

	150804:	JSIE Start this port.
	
*/

/* FreeRTOS includes. */
#include "stdint.h"
#include "FreeRTOS.h"
#include "task.h"

/* FreeRTOS+CLI includes. */
#include "FreeRTOS_IP.h"
#include "FreeRTOS_CLI.h"
#include "ez80_rtc.h"
#include "ez80_leds.h"
#include "ez80_buttons.h"
#include "monitor.h"
#include "AMD79C874_phy.h"
#include "NetworkInterface.h"
#include "ez80_emac.h"
#include "ez80_tty.h"

#include <CTYPE.H>
#include <String.h>

static int alarmflg = 0;
static int alarms = 0;

void nested_interrupt rtc_alarm(void)
{
	char dummy = RTC_CTRL;
	alarmflg = 5;

	lockcons();
	printf( ANSI_SCUR ANSI_COFF);
	printf( ANSI_SCUR ANSI_SATT(7,31,40) ANSI_GXY(62,8) ">>> ALARM %6u <<<"ANSI_RCUR, ++alarms);
	printf( ANSI_RCUR ANSI_CON) ;
	unlockcons();
}

void sys_heapinfo()
{
size_t xPortGetFreeHeapSize( void );
size_t xPortGetMinimumEverFreeHeapSize( void );
	
	lockcons();
	printf( ANSI_SCUR ANSI_COFF);
	printf( ANSI_SATT(0,34,43) ANSI_GXY(85,4) " Heap                 ");
	printf( ANSI_SATT(0,36,40) ANSI_GXY(85,5) "cur free Heap: " ANSI_SATT(0,32,40) "%7u", xPortGetFreeHeapSize());
	printf( ANSI_SATT(0,36,40) ANSI_GXY(85,6) "min free Heap: " ANSI_SATT(0,32,40) "%7u", xPortGetMinimumEverFreeHeapSize());
	printf( ANSI_RCUR ANSI_CON) ;
	unlockcons();
}

void sys_netinfo()
{
	uint32_t ulIPAddress;
	uint32_t ulNetMask;
    uint32_t ulGatewayAddress;
    uint32_t ulDNSServerAddress;
	BaseType_t nwstat;
	uint16_t phyData;
	const char *att;
	char mod[40] = "NO LINK";
	emacStat_t* stats = emac_stat();
	
	/* The network is up and configured.  Print out the configuration,
	which may have been obtained from a DHCP server. */
	if( xIsEthernetConnected( ) == pdTRUE )
    {
		att = ANSI_SATT(0,33,40);
        usPHY_ReadReg(PHY_DIAG_REG, &phyData);
        if(phyData & PHY_100_MBPS)
        {
            strncpy(mod,"100 Mbps",sizeof(mod));
        }
        else
        {
            strncpy(mod," 10 Mbps",sizeof(mod));
        }

        if(phyData & PHY_FULL_DUPLEX)
        {
            strncat(mod, ", Full-Duplex",sizeof(mod));
        }
        else
        {
            strncat(mod, ", Half-Duplex",sizeof(mod));
        }
		FreeRTOS_GetAddressConfiguration( &ulIPAddress,
									  &ulNetMask,
									  &ulGatewayAddress,
									  &ulDNSServerAddress );

	}
	else 
	{
		att = ANSI_SATT(0,31,40);
		ulIPAddress = 0UL;
		ulNetMask = 0UL;
		ulGatewayAddress = 0UL;
		ulDNSServerAddress = 0UL;
	}
	
	nwstat = FreeRTOS_IsNetworkUp() == pdTRUE ? 1:0;
	if(nwstat)
		att = ANSI_SATT(0,32,40);
	lockcons();
	printf( ANSI_SCUR ANSI_COFF);
	printf( ANSI_SATT(0,34,43) ANSI_GXY( 5,4) " Ethernet                                      ");
	printf( ANSI_SATT(0,36,40) ANSI_GXY( 5,5) "Network: %s%s, %-23s", att, (char*)(nwstat? "Up  ":"Down"),mod);
	printf( ANSI_SATT(0,36,40) ANSI_GXY( 5,6) "IP     : %s%-15lxip ", att, FreeRTOS_ntohl(ulIPAddress ));
	printf( ANSI_SATT(0,36,40) ANSI_GXY(30,6) "Mask   : %s%-15lxip ", att, FreeRTOS_ntohl(ulNetMask));
	printf( ANSI_SATT(0,36,40) ANSI_GXY( 5,7) "Gateway: %s%-15lxip ", att, FreeRTOS_ntohl(ulGatewayAddress) );
	printf( ANSI_SATT(0,36,40) ANSI_GXY(30,7) "DNS    : %s%-15lxip ", att, FreeRTOS_ntohl(ulDNSServerAddress) );
	printf( ANSI_SATT(0,36,40) ANSI_GXY( 5,8) "txsz   : %s%-15u "	, att, stats->txsz);
	printf( ANSI_SATT(0,36,40) ANSI_GXY(30,8) "rxsz   : %s%-15u "	, att, stats->rxsz);
	printf( ANSI_SATT(0,36,40) ANSI_GXY( 5,9) "txdone : %s%-15u "	, att, stats->txdone);
	printf( ANSI_SATT(0,36,40) ANSI_GXY(30,9) "rxdone : %s%-15u "	, att, stats->rxdone);
	printf( ANSI_SATT(0,36,40) ANSI_GXY( 5,10)"txover : %s%-15u "	, att, stats->txover);
	printf( ANSI_SATT(0,36,40) ANSI_GXY(30,10)"rxover : %s%-15u "	, att, stats->rxover);
	printf( ANSI_SATT(0,36,40) ANSI_GXY( 5,11)"txpcf  : %s%-15u "	, att, stats->txpcf);
	printf( ANSI_SATT(0,36,40) ANSI_GXY(30,11)"rxpcf  : %s%-15u "	, att, stats->rxpcf);
	
	printf( ANSI_SATT(0,36,40) ANSI_GXY( 5,12)"txabort: %s%-15u "	, att, stats->txabort);
	printf( ANSI_SATT(0,36,40) ANSI_GXY(30,12)"rxnotok: %s%-15u "	, att, stats->rxnotok);
	printf( ANSI_SATT(0,36,40) ANSI_GXY( 5,13)"txfsmer: %s%-15u "	, att, stats->txfsmerr);
	printf( ANSI_SATT(0,36,40) ANSI_GXY(30,13)"mgdone : %s%-15u "	, att, stats->mgdone);
	printf( ANSI_SATT(0,36,40) ANSI_GXY( 5,14)"rxnospc: %s%-15u "	, att, stats->rxnospace);
	printf( ANSI_SATT(0,36,40) ANSI_GXY(30,14)"rxinvsz: %s%-15u "	, att, stats->rxinvsize);
	
	printf( ANSI_SATT(0,36,40) ANSI_GXY( 5,15)"rxcrcer: %s%-15u "	, att, stats->rxcrcerr);
	printf( ANSI_SATT(0,36,40) ANSI_GXY(30,15)"rxalign: %s%-15u "	, att, stats->rxalignerr);
	printf( ANSI_SATT(0,36,40) ANSI_GXY( 5,16)"rxlongs: %s%-15u "	, att, stats->rxlongevent);
	printf( ANSI_SATT(0,36,40) ANSI_GXY(30,16)"rxok   : %s%-15u "	, att, stats->rxok);
	printf( ANSI_SATT(0,36,40) ANSI_GXY( 5,17)"rxcf   : %s%-15u "	, att, stats->rxcf);
	printf( ANSI_RCUR ANSI_CON) ;
	unlockcons();
}		


void sys_rtcinfo()
{
	char clock[21];
	const char* att = chkRTCPowerLost() ? ANSI_SATT(1,31,40):ANSI_SATT(0,32,40);

	lockcons();
	printf( ANSI_SCUR ANSI_COFF);
	printf( ANSI_SATT(0,34,43) ANSI_GXY(55,4) " Real Time Clock           ");
	printf( ANSI_SATT(0,36,40) ANSI_GXY(55,5) "Date : %s%-22s", att, getsDate(clock,sizeof(clock)));
	printf( ANSI_SATT(0,36,40) ANSI_GXY(55,6) "Time : %s%-22s", att, getsTime(clock,sizeof(clock)));
	printf( ANSI_SATT(0,36,40) ANSI_GXY(55,7) "Alarm: %s%-22s", att, getsAlarm(clock,sizeof(clock)));
	if(alarmflg > 0 && !--alarmflg)
		printf( ANSI_SATT(0,31,40) ANSI_GXY(62,8) ">>> ALARM %6u <<<", alarms);
		
	printf( ANSI_RCUR ANSI_CON) ;
	unlockcons();
}

void sys_buttoninfo()
{
	static const char  *const label[3] = {"Left   ","Middle ","Right  "};
	static buttons_t old;
	buttons_t *but = get_buttons();
	const char* att;
	int i;
	
	lockcons();
	printf( ANSI_SCUR ANSI_COFF);
	printf( ANSI_SATT(0,34,43) ANSI_GXY(55,10) " Platform buttons                               ");
	
	att = old.lastisp != but->lastisp ? ANSI_SATT(1,31,40):ANSI_SATT(0,32,40);
	old.lastisp = but->lastisp;
	printf( ANSI_SATT(0,36,40) ANSI_GXY(55,11) "LastISP: %s%s", att, label[old.lastisp]);

	att = old.laststate != but->laststate ? ANSI_SATT(1,31,40):ANSI_SATT(0,32,40);
	old.laststate = but->laststate;
	printf( ANSI_SATT(0,36,40) ANSI_GXY(55,12) "State  : %s%u", att, (unsigned)but->laststate);
	
	
	for( i=BUTTON_LEFT; i<=BUTTON_RIGTH; i++)
	{
		char b[80];
		snprintf(b,sizeof(b)
		, ANSI_SATT(0,36,40) "\x1b[%i;55f%s: n, t-down, t-up: %s%4u,%8u,%8u",13+i, label[i]
		,(char*) ((old.cnt[i] != but->cnt[i])||(old.down[i] != but->down[i])||(old.up[i] != but->up[i])? ANSI_SATT(1,31,40):ANSI_SATT(0,32,40))
		,old.cnt[i] = but->cnt[i],old.down[i] = but->down[i],old.up[i] = but->up[i]);
		printf(b);
		
	}
	printf( ANSI_RCUR ANSI_CON) ;
	unlockcons();
}

void sysinfo(void* param)
{
	lockcons();
	printf(ANSI_NORM ANSI_CLRS ANSI_SATT(0,34,47) ANSI_GXY(1,1)  " FreeRTOS " tskKERNEL_VERSION_NUMBER " Demo V" VERSION " on ZiLOG\'s " DEVKIT " Kit / " ZIDE ANSI_DEOL);
	printf(                    ANSI_SATT(0,34,47) ANSI_GXY(1,2)  " Autor " AUTOR " www.NadiSoft.de <" AUTORMAIL ">" ANSI_DEOL ANSI_NORM);
	unlockcons();

	setAlarm(rtc_alarm,1,0, 0, 0, 30);
	
	while(1)
	{
		sys_netinfo();
		sys_rtcinfo();
		sys_heapinfo();
		sys_buttoninfo();
	}
}

static char *skipws(const char*s)
{
	while(*s && isspace(*s))
		s++;
	return s;
}

static uint24_t getnum(const char* s, int8_t **error)
{
	uint24_t res = 0;
	int base = 10;
	int negativ = 0;
	char *tmp = skipws(s);
	
	if(*tmp == '0')
	{
		tmp++;
		base = 8;
		if(*tmp && tolower(*tmp) == 'x')
		{
			tmp++;
			base = 16;
		}
	}

	while(*tmp)
	{
		uint8_t c = toupper(*tmp);
		if(c >= '0')
		{
			c -= '0';
			if( c > 9)
				c -= 'A' - '9' - 1;
			
			if(c < base)
			{
				res *= base;
				res += c;
				tmp++;
				continue;
			}
		} 
		break;
	}
	if(error)
		*error = tmp;
	
	return res;
}


typedef enum 
	{
		DBYTE 	= 'B',
		DWORD16 = 'W',
		DWORD24 = 'D'
	} dumptfmt_t;

static dumptfmt_t	dumpfmt = DBYTE;
static int8_t *    	curraddr= 0;	
static uint24_t		counter = 0;
static size_t		lastsize= 256;

static BaseType_t prvDumpCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
	int8_t *param;
	uint8_t pcnt=1;
	int8_t *error;
	size_t sz;
	char * tmp;
	uint24_t num;
	char ascii[25];
	char *pascii;
		
	if(!(counter > 0))
	{
		BaseType_t length;
		counter = lastsize;
		
		param =  FreeRTOS_CLIGetParameter(pcCommandString, pcnt, &length);
		
		if(param && length == 1)
		{
			int8_t x = toupper(*param);
			if(x == DBYTE || x == DWORD16 || x == DWORD24)
			{
				dumpfmt = (dumptfmt_t) x;
				pcnt++;
				param =  FreeRTOS_CLIGetParameter(pcCommandString, pcnt, &length);
			}
		}
		
		// start addr
		if(param && length)
		{
			num = getnum(param, &error);
			if(*error && *error != ' ')
			{
				snprintf(pcWriteBuffer,xWriteBufferLen,ANSI_SATT(1,31,40)"Invalid address: %s", pcCommandString);
				return pdFALSE;
			}
			curraddr = (int8_t*) num;
			pcnt++;
			param =  FreeRTOS_CLIGetParameter(pcCommandString, pcnt, &length);
		}

		// show elements
		if(param && length)
		{
			num = getnum(param, &error);
			if(*error && *error != ' ')
			{
				snprintf(pcWriteBuffer,xWriteBufferLen,ANSI_SATT(1,31,40)"Invalid count: %s", pcCommandString);
				return pdFALSE;
			}
			counter = num;
			lastsize = counter;
		}
		
	}		
	
	pascii = ascii;
	snprintf(pcWriteBuffer,xWriteBufferLen,ANSI_SATT(0,33,40)"%06X:" ANSI_SATT(0,32,40),curraddr);
	pcnt = (dumpfmt == DBYTE)? 16 : ((dumpfmt == DWORD16) ? 8 : 5);
	
	while(pcnt-- && counter > 0)
	{
		sz = strlen(pcWriteBuffer);
		tmp= pcWriteBuffer + sz;
		switch(dumpfmt)
		{
			case DBYTE: snprintf(tmp,xWriteBufferLen-sz," %02X", *(uint8_t*)curraddr); 
				*pascii++ = isprint(*curraddr) ? *curraddr:'.'; 
				curraddr++;
				counter--;
			break;
			case DWORD16:snprintf(tmp,xWriteBufferLen-sz," %04X", *(UINT16*)curraddr);
				*pascii++ = isprint(*curraddr+1) ? *curraddr+1:'.'; 
				*pascii++ = isprint(*curraddr) ? *curraddr:'.'; 
				*pascii++ = '|';
				curraddr +=2;
				if(counter >= 2)
					counter -=2;
				else 
					counter = 0;
			break;
			case DWORD24:snprintf(tmp,xWriteBufferLen-sz," %06X", *(UINT24*)curraddr); 
				*pascii++ = isprint(*curraddr+2) ? *curraddr+2:'.'; 
				*pascii++ = isprint(*curraddr+1) ? *curraddr+1:'.'; 
				*pascii++ = isprint(*curraddr) ? *curraddr:'.'; 
				*pascii++ = '|';
				curraddr +=3;
				if(counter >= 3)
					counter -=3;
				else 
					counter = 0;
			break;
		}
	}
	
	while(pcnt--)
	{
		sz = strlen(pcWriteBuffer);
		tmp= pcWriteBuffer + sz;
		switch(dumpfmt)
		{
			case DBYTE: snprintf(tmp,xWriteBufferLen-sz,"   "); 
				*pascii++ = ' '; 
			break;
			case DWORD16:snprintf(tmp,xWriteBufferLen-sz,"     ");
				*pascii++ = ' '; 
				*pascii++ = ' '; 
				*pascii++ = ' ';
			break;
			case DWORD24:snprintf(tmp,xWriteBufferLen-sz,"       "); 
				*pascii++ = ' '; 
				*pascii++ = ' '; 
				*pascii++ = ' '; 
				*pascii++ = ' ';
			break;
		}
	}
	
	sz = 7 + (dumpfmt == DBYTE)? 16*3 : ((dumpfmt == DWORD16) ? 8*5 : 5*7);
	for(pcnt = strlen(pcWriteBuffer); pcnt < sz && pcnt < (xWriteBufferLen-1); pcnt++)
		pcWriteBuffer[pcnt] = ' ';
	pcWriteBuffer[pcnt] = 0;	
		
	tmp= &pcWriteBuffer[pcnt];
	*pascii = 0;
	snprintf(tmp, xWriteBufferLen-pcnt,ANSI_SATT(7,32,40)" |%s\n"ANSI_NORM,ascii);	
	return counter > 0 ? pdTRUE:pdFALSE;
}

static BaseType_t prvDateCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
	BaseType_t length;
	int8_t *param = FreeRTOS_CLIGetParameter(pcCommandString, 1, &length);
	int8_t *error;
	
	if(param)
	{
		uint8_t dow, day, mon;
		uint16_t year;
		
		dow = getnum(param,&error);
		if(*error && *error != ' ' || dow < 1 || dow > 7 )
		{
			snprintf(pcWriteBuffer,xWriteBufferLen,ANSI_SATT(1,31,40)"Invalid day of week (1=Mo. - 7=Su.): %s", pcCommandString);
			return pdFALSE;
		}
		param = FreeRTOS_CLIGetParameter(pcCommandString, 2, &length);
		day = getnum(param,&error);
		if(*error && *error != ' ' || day < 1 || day > 31 )
		{
			snprintf(pcWriteBuffer,xWriteBufferLen,ANSI_SATT(1,31,40)"Invalid day of month (1 - 31): %s", pcCommandString);
			return pdFALSE;
		}
		param = FreeRTOS_CLIGetParameter(pcCommandString, 3, &length);
		mon = getnum(param,&error);
		if(*error && *error != ' ' || mon < 1 || mon > 12 )
		{
			snprintf(pcWriteBuffer,xWriteBufferLen,ANSI_SATT(1,31,40)"Invalid month (1 - 12): %s", pcCommandString);
			return pdFALSE;
		}
		param = FreeRTOS_CLIGetParameter(pcCommandString, 4, &length);
		year = getnum(param,&error);
		if(*error && *error != ' ' || year > 9999 )
		{
			snprintf(pcWriteBuffer,xWriteBufferLen,ANSI_SATT(1,31,40)"Invalid year: %s", pcCommandString);
			return pdFALSE;
		}
		
		setDate(dow, day, mon, year);
	}
	
	getsDate(pcWriteBuffer, xWriteBufferLen);
	return pdFALSE; 
}

static BaseType_t prvTimeCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
	BaseType_t length;
	int8_t *param = FreeRTOS_CLIGetParameter(pcCommandString, 1, &length);
	int8_t *error;
	
	if(param)
	{
		uint8_t hrs, min, sec;
		hrs = getnum(param,&error);
		if(*error && *error != ' ' || hrs > 23 )
		{
			snprintf(pcWriteBuffer,xWriteBufferLen,ANSI_SATT(1,31,40)"Invalid hour ( 0 - 23): %s", pcCommandString);
			return pdFALSE;
		}
		
		param = FreeRTOS_CLIGetParameter(pcCommandString, 2, &length);
		min = getnum(param,&error);
		if(*error && *error != ' ' || min > 59 )
		{
			snprintf(pcWriteBuffer,xWriteBufferLen,ANSI_SATT(1,31,40)"Invalid minute ( 0 - 59): %s", pcCommandString);
			return pdFALSE;
		}
		
		param = FreeRTOS_CLIGetParameter(pcCommandString, 3, &length);
		sec = getnum(param,&error);
		if(*error && *error != ' ' || sec > 59 )
		{
			snprintf(pcWriteBuffer,xWriteBufferLen,ANSI_SATT(1,31,40)"Invalid secound ( 0 - 59): %s", pcCommandString);
			return pdFALSE;
		}
		setTime(hrs, min, sec);
	}
	getsTime(pcWriteBuffer, xWriteBufferLen);
	return pdFALSE;
}

static const CLI_Command_Definition_t xMemoryDump =
	{
		"dump", /* The command string to type. */
		"dump [b|w|d] [start-adr] [count]\n\tDumps count items (b=byte, w=word16, l=word24) at startt-addr.\n",
		prvDumpCommand, /* The function to run. */
		-1 /* No parameters are expected. */
	};
		
static const CLI_Command_Definition_t xDate =
	{
		"date", /* The command string to type. */
		"date [day-of-week day mon year]\n\tOptional set and display RTC date.\n",
		prvDateCommand, /* The function to run. */
		-1 /* No parameters are expected. */
	};

static const CLI_Command_Definition_t xTime =
	{
		"time", /* The command string to type. */
		"time [ hour min sec]\n\tOptional set and display RTC time.\n",
		prvTimeCommand, /* The function to run. */
		-1 /* No parameters are expected. */
	};
		
void vRegisterMonitorCLICommands( void )
{
	FreeRTOS_CLIRegisterCommand( &xMemoryDump );
	FreeRTOS_CLIRegisterCommand( &xDate );
	FreeRTOS_CLIRegisterCommand( &xTime );
}




/* ############################################################################# */

/* Defined by the application code, but called by FreeRTOS+TCP when the network
connects/disconnects (if ipconfigUSE_NETWORK_EVENT_HOOK is set to 1 in
FreeRTOSIPConfig.h). */
#if ipconfigUSE_NETWORK_EVENT_HOOK == 1
void vApplicationIPNetworkEventHook( eIPCallbackEvent_t eNetworkEvent )
{
    /* Check this was a network up event, as opposed to a network down event. */
    if( eNetworkEvent == eNetworkUp )
    {

    } else if( eNetworkEvent == eNetworkDown)
	{

	}
}
#endif
