#!/bin/sh

SOURCE=$1
DEST=$2
echo "Package: $SOURCE => $DEST"

rm -rf $DEST
mkdir -p "$DEST/resources/"
cp $SOURCE $DEST/
cp ../lib/mingw/*.dll $DEST/
cp -r ../resources/common/* $DEST/resources 
cp -r ../resources/desktop/* $DEST/resources

echo "Done"