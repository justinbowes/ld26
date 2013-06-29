#!/bin/sh

if [ $# -eq 0 ]
	then
	echo "Usage: $0 inputpath outputpath"
	exit 1
fi

for f in $1/*.wav 
do
	afconvert -f "adts" -d "aac " $f ${f%.wav}.aac
done
mv $1/*.aac $2/