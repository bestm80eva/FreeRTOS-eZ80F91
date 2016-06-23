ifdef MIXEDMODE	
	
	DEFINE Z80PAGE,SPACE = RAM,ALIGN = 10000h
	
	XREF _LED5x7_putchar	; BaseType_t LED5x7_putchar(CHAR c, TickType_t tout)
	XREF _vTaskDelay		; void vTaskDelay(TickType_t t)
	
	.assume	ADL=0
	segment Z80PAGE
		
; Pure Z80 code		
z80start:
		ld	hl,message
		ld 	de,100h
		ld	bc,0
loop:
		ld	a,(hl)
		or	a
		jr	z,z80start
		
		push	de
		ld		c,a
		push	bc
		call.lil	adl_LED5x7_putchar
		pop		af
		call.lil	adl_vTaskDelay
		pop		af
		inc		hl
		jr		loop
		
message:		
		ascii 'Hallo Z80\n',0
		ds	0FFFFh - $
		ds  1
		
	.assume	ADL=1
	segment CODE
	
; gateway to adl code
adl_LED5x7_putchar:
		push	af
		push	bc
		push	de
		push	hl
		push	ix
		push	iy
		xor		a
		
		ld		iy,0
		add		iy,sp		; spl	
		
		ld		ix,0
		add.sis	ix,sp		; sps
		
		ld.sis	bc,(ix+2)	; param #2 TIME
		push	bc
		ld		(iy-1),a
		
		ld.sis	bc,(ix)	; param #1	CHAR
		push	bc
		ld		(iy-4),a
		
		call 	_LED5x7_putchar
		pop		bc
		pop		bc
		
		pop		iy
		pop		ix
		pop		hl
		pop		de
		pop		bc
		pop		af
		ret.l

adl_vTaskDelay:
		push	af
		push	bc
		push	de
		push	hl
		push	ix
		push	iy
		xor		a
		
		ld		iy,0
		add		iy,sp		; spl	
		
		ld		ix,0
		add.sis	ix,sp		; sps
		
		ld.sis	bc,(ix)	; param #1
		push	bc
		ld		(iy-1),a
	
		call 	_vTaskDelay
		pop		bc
		
		pop		iy
		pop		ix
		pop		hl
		pop		de
		pop		bc
		pop		af
		ret.l
		
endif ;MIXEDMODE	
	
	END	