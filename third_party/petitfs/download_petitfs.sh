#!/bin/sh

# Make sure we are in the correct working directory by swithing to the folder
# where the script is
SCRIPT_DIR=$(dirname $0)
cd $SCRIPT_DIR

ROOT=petitfs_root

rm -r -r $ROOT

mkdir -p $ROOT

ARCHIVE=pff3.zip

# Download and unpack the sources
wget -O $ARCHIVE  http://elm-chan.org/fsw/ff/pff3.zip
mv $ARCHIVE $ROOT/
cd $ROOT
unzip $ARCHIVE
rm $ARCHIVE

# Apply our patch so it works with our project
patch -p1  < ../pff.patch
