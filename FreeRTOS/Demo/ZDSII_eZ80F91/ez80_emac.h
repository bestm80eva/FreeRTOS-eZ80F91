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
	
	
	File: 	ez80_emac.h
			module ethernet EMAC driver.
			Connect ansi-terminal to comport0 (115200,8,1,n) to see what's happen.
			Part of FreeRTOS Port for the eZ80F91 Development Kit eZ80F910300ZCOG
			See www.zilog.com for desciption.


	Developer:
	JSIE	 Juergen Sievers <JSievers@NadiSoft.de>

	150804:	JSIE Start this port.
	
*/
#ifndef EZ80_EMAC_H
#define EZ80_EMAC_H

// EMAC Default configuration
#define EMAC_TXBSZ    0x1000          // Size of Mac transmit buffer
#define EMAC_NEGMOD   F91_AUTO        // Default to Auto negotiation
#define EMAC_BUFSZCFG BUF32           // Each packet buffer in EMAC_RAM will be 32 bytes long
#define EMAC_MAC	  0x00, 0x90, 0x23, 0xAB, 0xCD, 0xEF 


#define EMAC_RAM_OFFSET      (0xC000)    // EMAC RAM addr offset from RAM_ADDR_U
#define EMAC_RAM_SIZE        (0x2000)    // EMAC RAM size

// Descriptor Overhead Size
#define DESC_OVH             (sizeof(_tSEMAC_DescTbl))

// EMAC_BUFSZ: Buffer Size Control
#define BUF256               (0x00)      // Rx/Tx buffer size = 256 bytes
#define BUF128               (0x01)      // Rx/Tx buffer size = 128 bytes
#define BUF64                (0x02)      // Rx/Tx buffer size = 64 bytes
#define BUF32                (0x03)      // Rx/Tx buffer size = 32 bytes

// Tx Descriptor Status Bit Definitions (_tSEMAC_DescTbl.stat)
#define EMAC_OWNS            (0x8000)    // 1=mac owns
#define HOST_OWNS            (0x0000)    // 0=host owns
#define TxOK                 (0x0000)    // pkt transmitted ok
#define TxAbort              (0x4000)    // pkt aborted
#define TxBPA                (0x2000)    // back pressure applied
#define TxHuge               (0x1000)    // pktsize > maxf
#define TxLOOR               (0x0800)    // length out of range
#define TxLCError            (0x0400)    // length check error
#define TxCrcError           (0x0200)    // crc error
#define TxPktDeferred        (0x0100)    // pkt was deferred
#define TxXsdfr              (0x0080)    // excessive defer
#define TxFifoUnderRun       (0x0040)    // fifo under run error
#define TxLateCol            (0x0020)    // late collision
#define TxMaxCol             (0x0010)    // max # collisions
#define TxNumColMask         (0x000f)    // # collisions

// Rx Descriptor Status Bit Sefinition (_tSEMAC_DescTbl.stat)
#define RxOK                (0x8000)    // pkt received ok
#define RxAlignError        (0x4000)    // checks for an even # of nibbles
#define RxCrcError          (0x2000)   // checks for CRC# == FCS#
#define RxLongEvent         (0x1000)    // long event or dropped event
#define RxPCF               (0x0800)    // pause control frame
#define RxCF                (0x0400)    // control frame
#define RxMCPkt             (0x0200)    // multicast pkt
#define RxBCPkt             (0x0100)    // broadcast pkt
#define RxVLAN              (0x0080)    // vlan frame type
#define RxUOpCode           (0x0040)    // unsupported opcode
#define RxLOOR              (0x0020)    // length out of range
#define RxLCError           (0x0010)    // length check error
#define RxCodeV             (0x0008)    // receive code violation
#define RxCEvent            (0x0004)    // carrier event previously seen
#define RxDvEvent           (0x0002)    // RXDV event previously seen
#define RxOVR               (0x0001)    // rx fifo overrun


/*******************************************************************************
*                            ERROR VALUES
*******************************************************************************/

// PHY and/or EMAC Related Error Values
#define PEMAC_NOT_FOUND         (-1)
#define PEMAC_PHY_NOT_FOUND     (-2)
#define PEMAC_PHYREAD_ERROR     (-3)
#define PEMAC_PHYINIT_FAILED    (-4)
#define PEMAC_SMEM_FAIL         (-5)
#define ILLEGAL_CALLBACKS       (-6)
#define PEMAC_RAMERR			(-7)
#define PEMAC_INIT_DONE         (0)
#define PKTTOOBIG               (-2)

// Tx Status
#define OUT_OF_BUFS             (4)
#define TX_FULLBUF              (3)
#define TX_WAITING              (2)
#define TX_DONE                 (1)


/*******************************************************************************
*                            EMAC TYPE DEFINITIONS
*******************************************************************************/

// Macros for _tSEMAC_Conf.mode
#define F91_10_HD       (0)        // 10 Mbps Half Duplex
#define F91_10_FD       (1)        // 10 Mbps Full Duplex
#define F91_100_HD      (2)        // 100 Mbps Half Duplex
#define F91_100_FD      (3)        // 100 Mbps Full Duplex
#define F91_AUTO        (4)        // Auto Negotiation


#define MAXFRAMESIZE    (ipconfigNETWORK_MTU + HEADERSIZE + CRCSIZE)
#define MINFRAMESIZE     64

// Rx/Tx Descriptor Table
typedef struct desctbl 
{
    struct desctbl *np;            // Pointer to the start of the next packet
    UINT16 pktsz;                  // Ethernet packet size, incl CRC bytes
    UINT16 stat;                   // Packet status
}_tSEMAC_DescTbl;

typedef struct emacStat_s 
{
	UINT24 txsz;
	UINT24 txabort;
	UINT24 txover;
	UINT24 txpcf;
	UINT24 txdone;
	UINT24 txfsmerr;
	
	UINT24 mgdone;
	
	UINT24 rxsz;
	UINT24 rxdone;
	UINT24 rxover;
	UINT24 rxpcf;
	UINT24 rxcf;
	UINT24 rxcrcerr;
	UINT24 rxalignerr;
	UINT24 rxlongevent;
	UINT24 rxok;
	UINT24 rxnotok;
	UINT24 rxnospace;
	UINT24 rxinvsize;
} emacStat_t;


#include "basetypes.h"

/*******************************************************************************
*                            FUNCTION PROTOTYPES
*******************************************************************************/
BaseType_t sPHY_Init(void);
BaseType_t usPHY_WriteReg(uint16_t usReg, uint16_t usData);
BaseType_t usPHY_ReadReg(uint16_t usReg, uint16_t *ausData);
BaseType_t xIsEthernetConnected( void );

const emacStat_t* emac_stat();
	

#endif /* EZ80_EMAC_H*/
