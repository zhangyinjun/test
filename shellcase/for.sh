#!/bin/bash

for ((i=0; i<10; i++))
do
	echo i=$i
done

for j in $(seq 1 10); do echo j=$j; done;
