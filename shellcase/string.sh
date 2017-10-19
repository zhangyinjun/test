#!/bin/bash

echo ${1}

if [[ -z "$1" || "$1" = "all" ]]
then
	echo "empty or all"
else
	echo input:${1}
fi

