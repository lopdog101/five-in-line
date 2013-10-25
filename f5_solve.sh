#!/bin/sh

while true
do
wget -q -O key_file $url'?src_name='$src_name'&cmd=get_job'
key=`cat key_file`
echo $key
if [ "$key" = "" ]; then
exit 1
fi

result=`../../solver $key`
n=`echo $result | awk '{print $2}'`
w=`echo $result | awk '{print $3}'`
f=`echo $result | awk '{print $4}'`
wget -q -O result_file $url'?src_name='$src_name'&cmd=save_job&key='$key'&n='$n'&w='$w'&f='$f
done
