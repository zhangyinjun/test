#!/bin/bash

echo "the script's name is `basename $0`"
echo "number of parameter is $#"
echo "whole parameter is \"$*\""

if [ -n $1 ]
then
    echo "first parameter is $1"
fi

if [ -n $2 ];then
    echo "second parameter is $2"
fi

