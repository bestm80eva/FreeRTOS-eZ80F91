
DISKPOOL=../disks

# CP/M Build disk contains the system files
CPMBUILD=$(DISKPOOL)/B
CPMBOOT=$(DISKPOOL)/A/.cpm.sys

# CP/M Sources sort by loadorder
CPMSRCS=BOOT.Z80 CPM22.Z80 BIOS.Z80
CPMINC=CPM.INC
  
# CP/M 2.2 64K System   
CPMSYS=CPM64.SYS

TARGETS=$(CPMSYS)

# Build utilies
ZASM=wine ~/ZDSII_eZ80Acclaim\!_5.2.1/bin/ez80asm.exe
ZLNK=wine ~/ZDSII_eZ80Acclaim\!_5.2.1/bin/ez80link.exe
SREC=srec_cat

ZASMFLAGS=-define:_EZ80ACCLAIM!=1 -define:EZ80F91=1 \
	  -list -listmac -name -pagelen:56 -pagewidth:80 \
	  -quiet -NOsdiopt -warn -NOigcase -cpu:eZ80F91 

ZLNKFLAGS=-FORMAT=INTEL32 -map -maxhexlen=16 \
	  -warnoverlap -xref -unresolved=fatal -NOdebug \
	  -warn -warn=fatal -NOigcase -sort NAME=ascending range ROM $$\0:$$\0ffff


all: $(TARGETS)

BOOT.bin:BOOT.Z80 CPM.INC
CPM22.bin:CPM22.Z80 CPM.INC
BIOS.bin:BIOS.Z80 CPM.INC

.PRECIOUS: %.hex

%.obj:%.Z80
	$(ZASM) $(ZASMFLAGS) $< 

%.hex:%.obj
	$(ZLNK) $(ZLNKFLAGS) $(basename $@)=$<

%.bin:%.hex
	$(SREC) $< -Intel -Offset -0x`srec_info $< -intel | sed -ne "s/^Data:\W*\([0-9A-F]*\).*/\1/p"` -o $@ -Binary
	

all: $(TARGET)
	echo done

CPM64.SYS:$(CPMSRCS:.Z80=.bin)
	cat $^ > $@
	cp $@ $(CPMBOOT)	
	$(SREC) $@ -Binary -O $(basename $@).c -C-A $(basename $@) -INClude
	cp -f $(CPMINC) $(CPMSRCS) $(CPMBUILD)
		

clean:
	-rm -f *.obj *.hex *.bin *.map *.lst *.c *.h 
	-rm -f $(CPMHEX) $(CPMSYS) $(TARGETS)
