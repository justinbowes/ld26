#!/bin/sh

PWD=`pwd`
OUTPUT_ARCHIVE="$PWD/$1"
INPUT=$2
rm -rf $1
mkdir -p `dirname $1`
ARCHIVER="$PWD/../build/tools/7za/7za.exe"
pushd $2
$ARCHIVER a -tzip -r $OUTPUT_ARCHIVE .
popd