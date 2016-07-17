

CPMSYS=BOOT.ASM BIOS.ASM CPM22.ASM
CPMCIM=$(CPMSYS:.ASM=.cim)
SYSTEM=CPM22.BOOT
BOOTIMG=drivea.cpm
BOOTDSK=disks/$(basename $(BOOTIMG))
CPMDSK=disks/driveb 


TARGET=$(BOOTIMG)


vpath %.ASM $(CPMDSK)/
vpath %.CIM zout/


%.cim:%.ASM
	zmac -m -z -I $(CPMDSK) $<

all: $(TARGET) $(CPMDSK) driveb.cpm drivec.cpm drived.cpm

CPM22.BOOT:$(CPMCIM)
	cat $(addprefix zout/, $(CPMCIM)) > $@
	cp -f $(addprefix zout/, $(CPMCIM:.cim=.hex)) $(CPMDSK)

drivea.cpm:$(SYSTEM)
	dd if=/dev/zero of=$@ bs=`expr 128 \* 26` count=77
	mkfs -t cpm -f ibm-3740 -b $< $@
	cpmcp -f ibm-3740 $@ $(BOOTDSK)/*.* 0:
	

driveb.cpm:disks/driveb/*.*
	dd if=/dev/zero of=$@ bs=`expr 128 \* 26` count=77
	mkfs -t cpm -f ibm-3740 -b $(SYSTEM) $@
	cpmcp -f ibm-3740 $@ disks/driveb/*.* 0:

drivec.cpm:disks/drivec/*.*
	dd if=/dev/zero of=$@ bs=`expr 128 \* 26` count=77
	mkfs -t cpm -f ibm-3740 -b $(SYSTEM) $@
	cpmcp -f ibm-3740 $@ disks/drivec/*.* 0:

drived.cpm:disks/drived/*.*
	dd if=/dev/zero of=$@ bs=`expr 128 \* 26` count=77
	mkfs -t cpm -f ibm-3740 -b $(SYSTEM) $@
	cpmcp -f ibm-3740 $@ disks/drived/*.* 0:

#4mb-hd

BOOT.cim:BOOT.ASM
BIOS.cim:BIOS.ASM
CPM22.cim:CPM22.ASM


clean:
	rm -fr zout $(SYSTEM) $(TARGET) drive[abcd].cpm
