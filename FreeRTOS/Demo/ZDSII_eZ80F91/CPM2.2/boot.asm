ifdef CPM22	

include cpm.inc

SIZE    EQU     (10000h-CCP) +127  	; size of cp/m system
SECTS   EQU     SIZE/128        ; # of sectors to load
	ORG	0
;
;       begin the load operation
;
COLD:  LD      BC,2            ; b=track 0, c=sector 2
       LD      D,SECTS         ; d=# sectors to load
       LD      HL,CCP          ; base transfer address
       LD      A,0             ; select drive A
       EXBIOS  FDIO, 0, FDCD
;
;       load the next sector
;
LSECT: LD      A,B             ; set track
       EXBIOS  FDIO, 0, FDCT
       LD      A,C             ; set sector
       EXBIOS  FDIO, 0, FDCS
	   PUSH    BC
	   LD	   BC,HL	
       EXBIOS  DMAIO, 0, DMABC
	   POP	   BC
       XOR     A               ; read sector
       EXBIOS  FDIO, 0, FDCOP
       EXBIOS  FDIO, 1, FDCST  ; get status of fdc
       CP      0               ; read successful ?
       JR      Z,CONT          ; yes, continue
       EXBIOS  MONITOR,1,0      ; no, start buildin monitor
CONT:
                               ; go to next sector if load is incomplete
       DEC     D               ; sects=sects-1
       JP      Z,BIOS          ; head for the bios
;
;       more sectors to load
;
;       we aren't using a stack, so use <sp> as scratch register
;             to hold the load address increment
;
       LD      SP,128          ; 128 bytes per sector
       ADD     HL,SP           ; <hl> = <hl> + 128
;
       INC     C               ; sector = sector + 1
       LD      A,C
       CP      27              ; last sector of track ?
       JP      C,LSECT         ; no, go read another
;
;       end of track, increment to next track
;
       LD      C,1             ; sector = 1
       INC     B               ; track = track + 1
       JP      LSECT           ; for another group
endif ;CPM22	
	
	END	;of BOOT
