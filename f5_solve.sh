#!/bin/sh

while true
do
wget -q -O key_file $url'?src_name='$src_name'&cmd=get_job'
key=`cat key_file`
echo $key
if [ "$key" = "" ]; then
exit 1
fi

../../solver $key >solve_content
wget -q -O result_file --post-file=solve_content $url'?src_name='$src_name'&cmd=save_job&key='$key'&sd='$stored_deep'&ld='$lookup_deep'&ac='$ant_count'&pc='$process_count
done
