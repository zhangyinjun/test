#!/bin/bash

#(( max=$1>$2?$1:$2 ))
#(( max=$max>$3?$max:$3 ))

#echo $max

#bit=`expr $RANDOM % 2`
bit=`date +%N`
echo "obase=2;$bit" | bc | cut -b 1-16

exit 0

