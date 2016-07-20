
DISKPOOL=../CPMRDisk/disks

# CP/M Build disk contains the system files
CPMBUILD=$(DISKPOOL)/driveb/0

# CP/M Sources sort by loadorder
CPMSRCS=boot.asm cpm22.asm bios.asm  
CPMINC=cpm.inc 
  
# CP/M 2.2 64K System   
CPMHEX=cpm64.hex
CPMSYS=cpm64.sys

DISKDRVS=$(DISKPOOL)/drivea.cpm $(DISKPOOL)/driveb.cpm $(DISKPOOL)/drivec.cpm $(DISKPOOL)/drived.cpm 
HARDDRVS=$(DISKPOOL)/drivei.cpmhd $(DISKPOOL)/drivej.cpmhd

TARGETS=$(DISKDRVS) $(HARDDRVS)

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

# -sort ADDRESS=ascending
../%.cpm:  
	dd if=/dev/zero of=$@ bs=`expr 128 \* 26` count=77
	mkfs -t cpm -f ibm-3740 -b $(CPMSYS) $@
	$(foreach user,$(shell cd $(basename $@) && ls -d [0-9]*), `cpmcp -f ibm-3740 $@ $(basename $@)/$(user)/*.* $(user):`)
	
../%.cpmhd:  
	dd if=/dev/zero of=$@ bs=`expr 128 \* 128` count=255
	mkfs -t cpm -f 4mb-hd2 $@
	#-$(foreach user,$(shell cd $(basename $@) && ls -d [0-9]*), `cpmcp -f 4mb-hd2 $@ $(basename $@)/$(user)/*.* $(user):`)
	
all: $(TARGETS)


$(DISKPOOL)/drivea.cpm:$(CPMSYS) $(DISKPOOL)/drivea

$(DISKPOOL)/driveb.cpm:$(CPMSYS) $(DISKPOOL)/driveb

$(DISKPOOL)/drivec.cpm:$(CPMSYS) $(DISKPOOL)/drivec

$(DISKPOOL)/drived.cpm:$(CPMSYS) $(DISKPOOL)/drived

$(DISKPOOL)/drivei.cpmhd:$(CPMSYS) $(DISKPOOL)/drivei

$(DISKPOOL)/drivej.cpmhd:$(CPMSYS) $(DISKPOOL)/drivej

boot.bin : boot.asm cpm.inc
cpm22.bin: cpm22.asm cpm.inc
bios.bin : bios.asm cpm.inc

.PRECIOUS: %.hex

%.obj:%.asm
	$(ZASM) $(ZASMFLAGS) $< 

%.hex:%.obj
	$(ZLNK) $(ZLNKFLAGS) $(basename $@)=$<

%.bin:%.hex
	$(SREC) $< -Intel -Offset -0x`srec_info $< -intel | sed -ne "s/^Data:\W*\([0-9A-F]*\).*/\1/p"` -o $@ -Binary
	
all: $(TARGET)
	echo done

cpm64.sys: $(CPMSRCS:.asm=.bin)
	cat $^ > $@
	$(SREC) $@ -Binary -o $(basename $@).hex -intel
	cp -f $@ $(basename $@).hex $(CPMINC) $(CPMSRCS) $(CPMSRCS:.asm=.hex) $(CPMBUILD)


clean:
	rm -f $(CPMSRCS:.asm=.obj) $(CPMSRCS:.asm=.hex) $(CPMSRCS:.asm=.bin) $(CPMSRCS:.asm=.map) $(CPMSRCS:.asm=.lst)  
	rm -f $(CPMHEX) $(CPMSYS) $(TARGETS)
