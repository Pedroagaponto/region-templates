#!/bin/bash

BKT="1,4,8,12,16,20"
SIML="1,4,8,12,16,20"
THREADS="1,4,8,16,32"
RPT=3
NP=2
ALG=3
DKT=vbd/vbd_s25.log
IMG=/home/pedro/tmp/imgs/small.tiff
MASK=/home/pedro/tmp/imgs/small.mask.png

DOUT=out
RT_BASE=base


echo ./runbrigds2.sh $BKT $SIML $THREADS $RPT $NP $ALG $DKT $IMG $MASK $DOUT $RT_BASE
#./runbrigds2.sh $BKT $SIML $THREADS $RPT $NP $ALG $DKT $IMG $MASK $DOUT $RT_BASE

BKT="20"
SIML="1"
THREADS="4"
RPT=2
NP=2
ALG=3
DKT=vbd/vbd_s25.log
IMG=/home/pedro/tmp/imgs/small.tiff
MASK=/home/pedro/tmp/imgs/small.mask.png

DOUT=out
RT_BASE=base


echo ./runbrigds2.sh $BKT $SIML $THREADS $RPT $NP $ALG $DKT $IMG $MASK $DOUT $RT_BASE
./runbrigds2.sh $BKT $SIML $THREADS $RPT $NP $ALG $DKT $IMG $MASK $DOUT $RT_BASE
