#!/bin/sh
url=http://fiveinline.info/solutions/root/solve.php
src_name=somebody
lookup_deep=2
process_count=`cat /proc/cpuinfo | grep processor | wc -l`

export url
export src_name
export lookup_deep
export process_count

c=0
mkdir spawn
while [ $c -lt $process_count ]
do
mkdir ./spawn/$c
cd ./spawn/$c
daemon -f -p pid ../../f5_solve.sh
cd ../../
c=`expr $c + 1`
done
