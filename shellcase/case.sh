#!/bin/bash

varname=9

case $varname in
    [a-z]) echo -n "abc";;
    [0-9]) echo -n "123";;
esac

