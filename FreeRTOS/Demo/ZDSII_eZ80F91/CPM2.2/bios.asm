ifdef CPM22	

include cpm.inc

;
;	I/O ports
;
MONITOR EQU 30h

CONIO	EQU 31h
CONSTA	EQU	01h		;console status port
CONDAT	EQU	02h		;console data port
PRTSTA	EQU	03h		;printer status port
PRTDAT	EQU	04h		;printer data port
AUXDAT	EQU	05h		;auxiliary data port

FDIO	EQU 32h
FDCD	EQU	01h		;fdc-port: # of drive
FDCT	EQU	02h		;fdc-port: # of track
FDCS	EQU	03h		;fdc-port: # of sector
FDCOP	EQU	04h		;fdc-port: command
FDCST	EQU	05h		;fdc-port: status

DMAIO	EQU 33h
DMABC	EQU	01h		;dma-port: dma address BC

ROMBOOT	EQU	34h		; boot buildin CP/M 2.2

EXBIOS: MACRO device, direction, port
	db	0cbh, device, direction, port
MACEND

;
;**************************************************************
;*
;*        B I O S   J U M P   T A B L E
;*
;**************************************************************
;
	xdef BOOT		;cold start
	xdef WBOOT		;warm start
	xdef CONST		;console status
	xdef CONIN		;console character in
	xdef CONOUT		;console character out
	xdef LIST		;list character out
	xdef PUNCH		;punch character out
	xdef READER		;reader character out
	xdef HOME		;move head to home position
	xdef SELDSK		;select disk
	xdef SETTRK		;set track number
	xdef SETSEC		;set sector number
	xdef SETDMA		;set dma address
	xdef READ		;read disk
	xdef WRITE		;write disk
	xdef LISTST		;return list status
	xdef SECTRN		;sector translate
	
;	ORG	BIOS		;origin of this program

DEFINE Z80BIOS, SPACE = ROM, ORG= BIOS

	segment Z80BIOS
	.assume	ADL=0

NSECTS	EQU	(BIOS-CCP+127)/128	;warm start sector count
;
;	jump vector for individual subroutines
;
BOOT:	JP	_BOOT		;cold start
WBOOT:	JP	_WBOOT		;warm start
CONST:	JP	_CONST		;console status
CONIN:	JP	_CONIN		;console character in
CONOUT:	JP	_CONOUT		;console character out
LIST:	JP	_CLIST		;list character out
PUNCH:	JP	_PUNCH		;punch character out
READER:	JP	_READER		;reader character out
HOME:	JP	_HOME		;move head to home position
SELDSK:	JP	_SELDSK		;select disk
SETTRK:	JP	_SETTRK		;set track number
SETSEC:	JP	_SETSEC		;set sector number
SETDMA:	JP	_SETDMA		;set dma address
READ:	JP	_DREAD		;read disk
WRITE:	JP	_WRITE		;write disk
LISTST:	JP	_LISTST		;return list status
SECTRN:	JP	_SECTRN		;sector translate
;
;	fixed data tables for four-drive standard
;	IBM-compatible 8" disks
;
;	disk parameter header for disk 00
DPBASE:	DW	TRANS,0000H
	DW	0000H,0000H
	DW	DIRBF,DPBLK
	DW	CHK00,ALL00
;	disk parameter header for disk 01
	DW	TRANS,0000H
	DW	0000H,0000H
	DW	DIRBF,DPBLK
	DW	CHK01,ALL01
;	disk parameter header for disk 02
	DW	TRANS,0000H
	DW	0000H,0000H
	DW	DIRBF,DPBLK
	DW	CHK02,ALL02
;	disk parameter header for disk 03
	DW	TRANS,0000H
	DW	0000H,0000H
	DW	DIRBF,DPBLK
	DW	CHK03,ALL03
;
;	sector translate vector for the IBM 8" disks
;
TRANS:	DB	1,7,13,19	;sectors 1,2,3,4
	DB	25,5,11,17	;sectors 5,6,7,8
	DB	23,3,9,15	;sectors 9,10,11,12
	DB	21,2,8,14	;sectors 13,14,15,16
	DB	20,26,6,12	;sectors 17,18,19,20
	DB	18,24,4,10	;sectors 21,22,23,24
	DB	16,22		;sectors 25,26
;
;	disk parameter block, common to all IBM 8" disks
;
DPBLK:  DW	26		;sectors per track
	DB	3		;block shift factor
	DB	7		;block mask
	DB	0		;extent mask
	DW	242		;disk size-1
	DW	63		;directory max
	DB	192		;alloc 0
	DB	0		;alloc 1
	DW	16		;check size
	DW	2		;track offset
;
;	fixed data tables for 4MB harddisk
;
;	disk parameter header drive 8 = I
HDBASE:
	DW	HDTRA,0000H
	DW	0000H,0000H
	DW	DIRBF,HDBLK
	DW	CHKHD8,ALLHD8
	
;	disk parameter header drive 9 = J
HDBAS9:
	DW	HDTRA,0000H
	DW	0000H,0000H
	DW	DIRBF,HDBLK
	DW	CHKHD9,ALLHD9
;
;	sector translate vector for the hardisk
;
HDTRA:	DB	1,2,3,4,5,6,7,8,9,10
	DB	11,12,13,14,15,16,17,18,19,20
	DB	21,22,23,24,25,26,27,28,29,30
	DB	31,32,33,34,35,36,37,38,39,40
	DB	41,42,43,44,45,46,47,48,49,50
	DB	51,52,53,54,55,56,57,58,59,60
	DB	61,62,63,64,65,66,67,68,69,70
	DB	71,72,73,74,75,76,77,78,79,80
	DB	81,82,83,84,85,86,87,88,89,90
	DB	91,92,93,94,95,96,97,98,99,100
	DB	101,102,103,104,105,106,107,108,109,110
	DB	111,112,113,114,115,116,117,118,119,120
	DB	121,122,123,124,125,126,127,128
;
;       disk parameter block for harddisk
;
HDBLK:  
	DW    128		;sectors per track
	DB    4		;block shift factor
	DB    15		;block mask
	DB    0		;extent mask
	DW    2039	;disk size-1
	DW    1023	;directory max
	DB    255		;alloc 0
	DB    255		;alloc 1
	DW    0		;check size
	DW    0		;track offset
;
;	signon message
;
SIGNON: 
	DB	"64K CP/M Vers. 2.2 / CBIOS V1.5"
	DB	13,10
	DB	"Copyright 1998-2016 by Juergen Sievers"
	DB	13,10,0
;
;	end of fixed tables
;
;	individual subroutines to perform each function
;	simplest case is to just perform parameter initialization
;
_BOOT:   
	LD	SP,80H		;use space below buffer for stack
	LD	HL,SIGNON	;print message
BOOTL:  LD	A,(HL)
	OR	A
	JP	Z,BOOTC
	LD	C,A
	CALL	CONOUT
	INC	HL
	JP	BOOTL
BOOTC:  XOR	A		;zero in the accum
	LD	(IOBYTE),A	;clear the iobyte
	LD	(CDISK),A	;select disk zero
	LD	C,A
	CALL	SELDSK
	JP	GOCPM		;initialize and go to cp/m
;
;	simplest case is to read the disk until all sectors loaded
;
_WBOOT:  
	LD	B,3
WBOOTE:
	LD	SP,80H		;use space below buffer for stack
	PUSH BC			; error retry 
	LD	C,0		;select disk 0
	CALL	SELDSK
	CALL	HOME		;go to track 00
;
	LD	B,NSECTS	;b counts # of sectors to load
	LD	C,0			;c has the current track number
	LD	D,2			;d has the next sector to read
;	note that we begin by reading track 0, sector 2 since sector 1
;	contains the cold start loader, which is skipped in a warm start
	LD	HL,CCP		;base of cp/m (initial load point)
LOAD1:				;load one more sector
	PUSH	BC		;save sector count, current track
	PUSH	DE		;save next sector to read
	PUSH	HL		;save dma address
	LD		C,D		;get sector address to register c
	CALL	SETSEC	;set sector address from register c
	POP		BC		;recall dma address to b,c
	PUSH	BC		;replace on stack for later recall
	CALL	SETDMA	;set dma address from b,c
;	drive set to 0, track set, sector set, dma address set
	CALL	READ
	CP		00H		;any errors?
	JR		Z,NOBERR
	LD		SP,7Eh	
	POP		BC		
	DJNZ	WBOOTE;retry the entire boot if an error occurs
	EXBIOS  ROMBOOT,0,0
		
;	no error, move to next sector
NOBERR:
	POP	HL		;recall dma address
	LD	DE,128		;dma=dma+128
	ADD	HL,DE		;new dma address is in h,l
	POP	DE		;recall sector address
	POP	BC		;recall number of sectors remaining, and current trk
	DEC	B		;sectors=sectors-1
	JP	Z,GOCPM		;transfer to cp/m if all have been loaded
;	more sectors remain to load, check for track change
	INC	D
	LD	A,D		;sector=27?, if so, change tracks
	CP	27
	JP	C,LOAD1		;carry generated if sector<27
;	end of current track, go to next track
	LD	D,1		;begin with first sector of next track
	INC	C		;track=track+1
;	save register state, and change tracks
	CALL	SETTRK		;track address set from register c
	JP	LOAD1		;for another sector
;	end of load operation, set parameters and go to cp/m

GOCPM:
	LD	A,0C3H		;c3 is a jmp instruction
	LD	(0),A		;for jmp to wboot
	LD	HL,WBOOT	;wboot entry point
	LD	(1),HL		;set address field for jmp at 0
;
	LD	(5),A		;for jmp to bdos
	LD	HL,BDOS		;bdos entry point
	LD	(6),HL		;address field of jump at 5 to bdos
;
	LD	BC,80H		;default dma address is 80h
	CALL	SETDMA
;
	EI			;enable the interrupt system
	LD	A,(CDISK)	;get current disk number
	LD	C,A		;send to the ccp
	JP	CCP		;go to cp/m for further processing
;
;
;	simple i/o handlers
;
;	console status, return 0ffh if character ready, 00h if not
;
_CONST:	
	EXBIOS CONIO, 1, CONSTA	;get console status
	RET
;
;	console character into register a
;

_CONIN:	
	EXBIOS CONIO, 1, CONDAT	;get character from console
	OR	A
;	RET Z
;	OUT (CONDAT),A		; echo back
	RET
;
;	console character output from register c
;
_CONOUT: 
	LD	A,C		;get to accumulator
	EXBIOS	CONIO, 0, CONDAT	;send character to console
	RET
;
;	list character from register c
;
_CLIST:	
	LD	A,C		;character to register a
	EXBIOS	CONIO, 0, PRTDAT
	RET
;
;	return list status (0 if not ready, 0xff if ready)
;
_LISTST: 
	EXBIOS	CONIO, 1, PRTSTA
	RET
;
;	punch character from register c
;
_PUNCH:	
	LD	A,C		;character to register a
	EXBIOS	CONIO, 0, AUXDAT
	RET
;
;	read character into register a from reader device
;
_READER: 
	EXBIOS	CONIO, 1, AUXDAT
	RET
;
;
;	i/o drivers for the disk follow
;
;	move to the track 00 position of current drive
;	translate this call into a settrk call with parameter 00
;
_HOME:	
	LD	C,0		;select track 0
	JP	SETTRK		;we will move to 00 on first read/write
;
;	select disk given by register C
;
_SELDSK: 
	LD	HL,0000H	;error return code
	LD	A,C
	CP	4			;must be between 0 and 3
	JR	NC,SELHD	;no carry if 4,5,...
;	disk number is in the proper range
;	compute proper disk parameter header address
	LD	L,A			;L=disk number 0,1,2,3
	ADD	HL,HL		;*2
	ADD	HL,HL		;*4
	ADD	HL,HL		;*8
	ADD	HL,HL		;*16 (size of each header)
	LD	DE,DPBASE
	ADD	HL,DE		;HL=.dpbase(diskno*16)
	JR  SEL

SELHD:	
	CP	8		;select the harddisk?
	JR	NZ, HD9
	LD HL,HDBASE
	JR	SEL
HD9:CP 9
	RET NZ			; > 9
	LD	HL,HDBAS9	;HL=hdbase for harddisk
SEL:EXBIOS	FDIO, 0, FDCD	;select disk drive 8,9 I,J
	RET
;
;	set track given by register c
;
_SETTRK: 
	LD	A,C
	EXBIOS	FDIO, 0, FDCT
	RET
;
;	set sector given by register c
;
_SETSEC: 
	LD	A,C
	EXBIOS	FDIO, 0, FDCS
	RET
;
;	translate the sector given by BC using the
;	translate table given by DE
;
_SECTRN:
	EX	DE,HL		;HL=.trans
	ADD	HL,BC		;HL=.trans(sector)
	LD	L,(HL)		;L = trans(sector)
	LD	H,0		;HL= trans(sector)
	RET			;with value in HL
;
;	set dma address given by registers b and c
;
_SETDMA: 
	EXBIOS	DMAIO, 0, DMABC	;in dma
	RET
;
;	perform read operation
;
_DREAD:	
	XOR	A		;read command -> A
	JP	WAITIO		;to perform the actual i/o
;
;	perform a write operation
;
_WRITE:	
	LD	A,1		;write command -> A
;
;	enter here from read and write to perform the actual i/o
;	operation.  return a 00h in register a if the operation completes
;	properly, and 01h if an error occurs during the read or write
;
;	in this case, we have saved the disk number  'diskno' (0-3)
;			the track number in 'track' (0-76)
;			the sector number in 'sector' (1-26)
;			the dma address in 'dmaad' (0-65535)
;
WAITIO: EXBIOS	FDIO, 0, FDCOP	;start i/o operation
	EXBIOS	FDIO, 1, FDCST	;status of i/o operation -> A
	RET
;
;	the remainder of the CBIOS is reserved uninitialized
;	data area, and does not need to be a part of the
;	system memory image (the space must be available,
;	however, between "begdat" and "enddat").
;
;	scratch ram area for BDOS use
;

BEGDAT	EQU	$		;beginning of data area
DIRBF:	DS	128		;scratch directory area
ALL00:	DS	31		;allocation vector 0
ALL01:	DS	31		;allocation vector 1
ALL02:	DS	31		;allocation vector 2
ALL03:	DS	31		;allocation vector 3
ALLHD8:	DS	255		;allocation vector harddisk 8 I
ALLHD9:	DS	255		;allocation vector harddisk 9 J
CHK00:	DS	16		;check vector 0
CHK01:	DS	16		;check vector 1
CHK02:	DS	16		;check vector 2
CHK03:	DS	16		;check vector 3
CHKHD8:	DS	1		;check vector harddisk 8 I
CHKHD9:	DS	1		;check vector harddisk 9 J
;
EDAT	EQU	$		;end of data area
		DS 0ffffh - $
		DS  1
endif ;CPM22	
	
	END	;of BIOS
