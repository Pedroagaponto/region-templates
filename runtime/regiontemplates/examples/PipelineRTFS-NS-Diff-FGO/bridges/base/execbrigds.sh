#!/bin/bash -x
#SBATCH --ntasks-per-node 28              # total number of mpi tasks requested
#SBATCH -N 1 
#SBATCH -p normal     # queue (partition) -- normal, development, etc.
#SBATCH -t 8:00:00        # run time (hh:mm:ss) - 1.5 hours
#SBATCH -p RM
#SBATCH --mail-type=ALL
#SBATCH --mail-user=100119191@aluno.unb.br


((NP = $1 + 1))
I=$2
B=$3
C=$4
DKT=$5
ALG=$6
DOUT=$7
RPT=$8

LMEM=$9
LTIME=${10}

WINDOW=${11}


if [ $# -ne 11 ]
then
	echo "Numero de parametros errado"
	exit $E_BADARGS
fi

MPI="mpirun --bind-to none -np $NP "
EXEC="./PipelineRTFS-NS-Diff-FGO -b $B -c $C -i $I -dkt $DKT -ma $ALG -w $WINDOW"
TIMER="/usr/bin/time -a -v -o $LTIME"
#EXEC=" xterm -e gdb -ex run --args ./PipelineRTFS-NS-Diff-FGO -b $B -c $C -i $I -dkt $DKT -ma $ALG -w $WINDOW"

for rep in `seq 1 $RPT`
do
	printf 'i%b b%b c%b r%b\n' $I $B $C $rep
    pwd
    $MPI $TIMER $EXEC #\
    #1>$DOUT/cout_i${I}b${B}c${C}rep${rep}.txt 2>$DOUT/cerr_i${I}b${B}c${C}rep${rep}.txt

	#printf %s "$(cat $LTIME)" > $LTIME
	#printf %s "$(cat $LMEM)" > $LMEM
done

#tr -d '\n' < $LTIME > ${LTIME}t
#mv ${LTIME}t $LTIME