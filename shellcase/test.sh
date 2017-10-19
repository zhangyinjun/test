#!/bin/bash

(( max=$1>$2?$1:$2 ))
(( max=$max>$3?$max:$3 ))

echo $max

exit 0

