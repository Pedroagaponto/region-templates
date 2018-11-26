#!/usr/bin/env python
import os

RPT=1
ALG=3
BKT=28
DKT="vbd_s25.log"
IMG="/small.tiff"
MASK="/small.mask.png"

DOUT="out"
RT_BASE="base"


#echo ./runbrigds2.sh

def getNP():
    return {1,2,4,7,14,28}
    # return {2}

def getTH(np):
    return {28//np}

def getI(th):
    if th==1:
        return {1}
    return {1,2,th/2,th}
    # return {2}

def getW(th, i):
    return {th//i}

parameter_set = set()

for np in getNP():
    for th in getTH(np):
        for i in getI(th):
            for w in getW(th,i):
                parameter_set.add((np,th,i,w))


parameter_list = sorted(parameter_set)
for p in parameter_list:
    print p[0], p[1], p[2], p[3]
    os.system("./runbrigds2.sh " + str(BKT) + " " + str(p[2]) + " " + str(p[1]) + " " + str(RPT) + " " + str(p[0]) + " " + str(ALG) + " " + str(DKT) + " " + IMG + " " + MASK + " " + DOUT + " " + RT_BASE + " " + str(p[3]))
