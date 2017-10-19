#!/bin/bash

while read input
do
    if [ $input = "quit" ]; then
        break
    else
        echo "your input: ${input}"
    fi
done

