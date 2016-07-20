#!/bin/sh

FLOPPYS="drivea.cpm driveb.cpm drivec.cpm drived.cpm"
HARDDSK="drivei.cpm drivej.cpm"


for d in $FLOPPYS ; 
do
    for s in `cpmls -f ibm-3740 $d | sed -ne "s/^\([0-9]\{1,2\}\):/\1/p"` ;
    do
        mkdir -p $(basename "$d" .cpm)/$s
        cpmcp -f ibm-3740 $d $s\:\*.\* $(basename "$d" .cpm)/$s
    done
done

for d in $HARDDSK ; 
do
    for s in `cpmls -f 4mb-hd $d | sed -ne "s/^\([0-9]\{1,2\}\):/\1/p"` ;
    do
        mkdir -p $(basename "$d" .cpm)/$s
        cpmcp -f 4mb-hd $d $s\:\*.\* $(basename "$d" .cpm)/$s
    done
done
