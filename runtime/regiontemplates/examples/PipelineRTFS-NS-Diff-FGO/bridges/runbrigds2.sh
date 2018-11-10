#!/bin/bash

if [ $# -ne 11 ]
then
        echo "Usage: sh run.sh {BKT} {SIML} {THREADS} {RPT} {NP} \
{ALG} {DKT} {IMG} {MASK} {DOUT} {RT_BASE}"

        echo "{BKT}: List of bucket size. ex: 1,2,3,4,5"
        echo "{SIML}: List of simultaneos task max. ex: 1,2,3,4,5"
        echo "{THREDS}: List of number of threads"
        echo "{RPT}: Number of repeatitions for each test."
        echo "{NP}: -np {NP}"
        echo "{ALG}: reuse algorithm number."
        echo "{DKT}: dakota file"
        echo "{IMG}: image"
        echo "{MASK}: image mask"
        echo "{DOUT}: dir of output RT"
        echo "{RT_BASE}: dir of base region template"
        exit $E_BADARGS
fi


BKT=$(echo $1 | tr "," "\n")
SIML=$(echo $2 | tr "," "\n")
THREDS=$(echo $3 | tr "," "\n")
RPT=$4
NP=$5
ALG=$6
DKT=$7
IMG=$8
MASK=$9

DOUT=${10}
RT_BASE=${11}

for i in $SIML
do
        for b in $BKT
        do
			for c in $THREDS
			do

				TESTE=i${i}b${b}c${c}
				mkdir $TESTE
				cp -r $RT_BASE/* $TESTE  

				cd $TESTE
				mkdir $DOUT

                mkdir /tmp/$TESTE
                sed -i "s/\/tmp\//\/tmp\/$TESTE\//g" rtconf.xml
				#echo "sh update-dakota-file $DKT $IMG $MASK"
				
				./execbrigds.sh $NP $i $b $c $DKT $ALG $DOUT $RPT ${DOUT}/${TESTE}-mem.log ${DOUT}/${TESTE}-time.log
				
				cd ..
			done
        done
done

