#!/bin/sh

COUNTER=1
while [  $COUNTER -lt 151 ]; do

echo "analyze table s$COUNTER;"

COUNTER=$(($COUNTER + 1)) 
done
