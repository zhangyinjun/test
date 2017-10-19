#!/bin/bash

#case 1, use of if
filename=testfile
if [ -e $filename ]; then
    echo "file $filename exists."
else
    echo "file $filename not found.";touch $filename
fi

#case 2, use of :
condition=0
if [ $condition -gt 0 ] #gt:greater than, also -lt/-eq
then :  #NOP
else
    echo "$condition"
fi

#case 3, use of () for init array
arr=(1 5 67 88 9)
echo ${arr[2]}

#case 4, judge the file property and size
read filename
if [ ! -s $filename ]
then
    echo "file size is 0"
else
    echo "file size is $(du $filename | cut -f 1)"
fi

if [ -r $filename ]
then
    echo "file is readable"
else
    echo "file is not readable"
fi

if [ -w $filename ]
then
    echo "file is writeable"
else
    echo "file is not writeable"
fi

echo "test over."

exit 0
