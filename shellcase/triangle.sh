#!/bin/bash

i=0
j=0

while [[ $i -lt 5 ]]
do
    j=0
    p1=`expr 4 - $i`
    p2=`expr $i \* 2 + ${p1}`
    while (( $j<9 ))
    do
       #if [[ $j -ge $p1 && $j -le $p2 ]]; then
        if (( $j>=$p1 )) && (( $j<=$p2 )); then
            printf "*"
        else
            printf " "
        fi
        let "j++"
    done
    printf "\n"
    let "i++"
done

