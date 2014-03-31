#!/bin/bash

here=`dirname $0`

if [[ -z $1 ]]; then
    echo "Usage: $0 <filename>"
    exit 1
fi

sz=`ls -lh "$1"|awk -F" " '{print $5}'`
echo "Filesize: $sz"

echo "Asking crc64-redis to sum \"$1\"..."
redis=`$here/crc64-redis "$1" simple`
echo "Asking crc64-adler to sum \"$1\"..."
adler=`$here/crc64-adler "$1" simple`

rsum=`awk -F: '{print $1}' <<< $redis`
asum=`awk -F: '{print $1}' <<< $adler`

rtime=`awk -F: '{print $2}' <<< $redis`
atime=`awk -F: '{print $2}' <<< $adler`

echo "Redis CRC-64: $rsum"
echo "Adler CRC-64: $asum"

echo "Adler throughput: $atime MB/s"
echo "Redis throughput: $rtime MB/s"

faster=`echo "scale=2; ($atime/$rtime)" | bc -lq`

echo "Adler is ${faster}x faster than Redis"
