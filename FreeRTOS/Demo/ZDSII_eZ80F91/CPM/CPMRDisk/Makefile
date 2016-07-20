
CPMBLDDRV=disks/driveb
CPMFILES=cpm.inc boot.asm bios.asm cpm22.asm 
  
CPMSYS=cpm64.hex
SYSTEM=cpm64.sys

TARGET=drivea.cpm driveb.cpm drivec.cpm drived.cpm

vpath %.sys CPM22/
vpath %.hex CPM22/
vpath %.asm CPM22/
vpath %.inc CPM22/

%.cpm: disks/$(basename $@)/*.* 
	dd if=/dev/zero of=$@ bs=`expr 128 \* 26` count=77
	mkfs -t cpm -f ibm-3740 -b $(SYSTEM) $@
	cp -f $(CPMFILES) $(CPMBLDDRV)  
	cpmcp -f ibm-3740 $@ disks/$(basename $@)/*.* 0:
	
all: $(TARGET)

drivea.cpm:$(SYSTEM) disks/drivea/*.*

driveb.cpm:$(SYSTEM) disks/driveb/*.*

drivec.cpm:$(SYSTEM) disks/drivec/*.*

drived.cpm:$(SYSTEM) disks/drived/*.*


#4mb-hd

clean:
	rm -f $(TARGET)
	make -C CPM22 clean
