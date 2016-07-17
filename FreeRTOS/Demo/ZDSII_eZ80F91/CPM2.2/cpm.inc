;	CBIOS for Z80-Simulator
;
;	Copyright (C) 1998-2016 by Juergen Sievers
;
MEM		EQU	64		;cp/m version memory size in kilobytes
;
;	"bias" is address offset from 3400H for memory systems
;	than 16K (referred to as "b" throughout the text).
;
BIAS	EQU	(MEM-20)*1024
CCP		EQU	3400H+BIAS	;base of ccp
BDOS	EQU	CCP+806H	;base of bdos
BIOS	EQU	CCP+1600H	;base of bios
CDISK	EQU	0004H		;current disk number 0=A,...,15=P
IOBYTE	EQU	0003H		;intel i/o byte

;
;	I/O ports
;
MONITOR EQU 30h

CONIO	EQU 31h		; CONOLE, PRT, AUX Device
CONSTA	EQU	01h		;console status port
CONDAT	EQU	02h		;console data port
PRTSTA	EQU	03h		;printer status port
PRTDAT	EQU	04h		;printer data port
AUXDAT	EQU	05h		;auxiliary data port

FDIO	EQU 32h		; FLOPPY Device
FDCD	EQU	01h		;fdc-port: # of drive in A. 0 = A, 1 = B ... HL points to DPH 
FDCT	EQU	02h		;fdc-port: # of track in A
FDCS	EQU	03h		;fdc-port: # of sector in A
FDCOP	EQU	04h		;fdc-port: command	A == 0 write, 1 read
FDCST	EQU	05h		;fdc-port: status	Disk Status to A

DMAIO	EQU 33h		; DMA Device
DMABC	EQU	01h		;dma-port: dma address BC

ROMBOOT	EQU	34h		; boot buildin CP/M 2.2

; Macro to tram ez80f91 on z80 mixed mode
; for simulat perefery io
EXBIOS: MACRO device, direction, port
	db	0cbh, device, direction, port
MACEND