#!/bin/bash -x

if [ $# -ne 12 ]
then
        echo "Usage: sh run.sh {BKT} {SIML} {THREADS} {RPT} {NP} \
{ALG} {DKT} {IMG} {MASK} {DOUT} {RT_BASE}"

        echo "{BKT}: -b List of bucket size. ex: 1,2,3,4,5"
        echo "{SIML}: -i List of simultaneos task max. ex: 1,2,3,4,5"
        echo "{THREDS}: -c List of number of threads"
        echo "{RPT}: Number of repeatitions for each test."
        echo "{NP}: -np {NP}"
        echo "{ALG}: -ma reuse algorithm number."
        echo "{DKT}: -dkt dakota file"
        echo "{IMG}: image"
        echo "{MASK}: image mask"
        echo "{DOUT}: dir of output RT"
        echo "{RT_BASE}: dir of base region template"
        echo "{W}: -w Amount o requested tasks"
        exit $E_BADARGS
fi


BKT=$1
SIML=$2
THREADS=$3
RPT=$4
NP=$5
ALG=$6
DKT=$7
IMG=$8
MASK=$9

DOUT=${10}
RT_BASE=${11}
W=${12}

TESTE=np${NP}c${THREADS}i${SIML}w${W}
mkdir $TESTE
cp -r $RT_BASE/* $TESTE

cd $TESTE
mkdir $DOUT

mkdir /tmp/$TESTE
sed -i "s/\/tmp\//\/tmp\/$TESTE\//g" rtconf.xml
./update-dakota-vbd.sh $DKT ${PWD}$IMG ${PWD}$MASK

TIMEFILE=${DOUT}/${DKT}.log-b${BKT}time.log
#xterm -hold -e ./execbrigds.sh $NP $SIML $BKT $THREADS $DKT $ALG $DOUT $RPT $TIMEFILE $TIMEFILE $W &
#disown
sbatch execbrigds.sh $NP $SIML $BKT $THREADS $DKT $ALG $DOUT $RPT $TIMEFILE $TIMEFILE $W

cd ..
