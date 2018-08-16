#!/bin/bash

#
# AppOS(R) Build Script (ABS)
# Copyright (c) 2002-2008 Spliced Networks LLC
#

SNVER_ABS="7.0.2"
SNVER_IBE="4.0.0.0"
SNVER_APPOS="4.0.0.0"
SNVER_COPYSTART="2002"
SNVER_COPYEND="2008"

export SNVER_ABS SNVER_IBE SNVER_APPOS SNVER_COPYSTART SNVER_COPYEND

# set the current directory to the build root

SNBLD=`pwd`
export SNBLD

echo ""
echo ""
echo "ABS version $SNVER_ABS"
echo "Copyright (c) $SNVER_COPYSTART-$SNVER_COPYEND Spliced Networks LLC"
echo ""

PATH=$PATH:$SNBLD/scripts
export PATH

HLFS=$SNBLD/target
export HLFS

set +h
umask 022
LC_ALL=POSIX
export LC_ALL

LSB=$HLFS/build
LSR=$SNBLD/source
SNL=$SNBLD/log-x86.log
SNEL=$SNBLD/log-x86_err.log
TC="tar xvf"
PTC="patch -Np1 -i"

export TC LSR LSB SNL SNEL PTC

mkdir -v $HLFS/tools
mkdir -p $HLFS/build

