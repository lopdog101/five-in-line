#!/bin/sh

c=0
mkdir spawn
while [ $c -lt 2 ]
do
mkdir ./spawn/$c
cd ./spawn/$c
daemon -f -p pid ../../f5_solve.sh
cd ../../
c=`expr $c + 1`
done
