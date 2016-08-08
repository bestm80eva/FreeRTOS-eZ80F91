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
	
	
		File: 	exbios.c
		extended CBIOS (CP/M 2.2 BIOS) to support CP/M 2.2 tasks
		Part of FreeRTOS Port for the eZ80F91 Development Kit eZ80F910300ZCOG
			See www.zilog.com for desciption.
			uart, console and faramted printout driver


	Developer:
	SIE	 Juergen Sievers <JSievers@NadiSoft.de>

	150804:	SIE Start this port.
	
*/

#ifndef _EXBIOS_H_
#define _EXBIOS_H_

#if defined(MIXEDMODE) 

#pragma pack(push,1)

typedef struct {
	uint8_t  mbaseL;
	uint8_t  mbase;
	uint8_t  mbaseU;
	uint24_t hl_;
	uint24_t de_;
	uint24_t bc_;
	uint24_t af_;
	uint24_t iy;
	uint24_t ix;
	uint24_t hl;
	uint24_t de;
	uint24_t bc;
	uint24_t af;
	uint8_t	 trapflg;
	uint24_t trapret;
	} trapargs_t;

#pragma pack(pop)
	
#if defined(CPM22)

#define SECSIZE 128
	
//	CONIO,PRT,AUX IOPorts
typedef enum {
	CONSTA = 0x01,	// console status port
	CONDAT,			// console data port
	PRTSTA,			// printer status port
	PRTDAT,			// printer data port
	AUXDAT			// auxiliary data port
} port_CONIO_t;

typedef enum {
	FDCD   = 0x01,	// fdc-port: # of drive
	FDCTBC,			// fdc-port: # of track
	FDCSBC,			// fdc-port: # of sector
	FDCOP,			// fdc-port: command
	FDCST			// fdc-port: status
} port_FDIO_t;

typedef enum {
	MONITOR = 0x30,
	CONIO,
	FDIO,
	DMAIO,
	ROMBOOT,
	MAXIO = 0x37	
} xebioscall_t;

void prvTCPCpmIOTask( void *ram );
void exbioscall(trapargs_t* arg);

#endif 	// CPM22
#endif	// MIXEDMODE	
#endif  // _EXBIOS_H_
