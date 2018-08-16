#!/bin/bash
#
# AppOS(R) Build Script (ABS)
# Copyright (c) 2002-2008 Spliced Networks LLC
#

# initialize some basic values

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

SNPLAT=`uname -a | grep x86_64`
export SNPLAT

if [ "$SNPLAT" ]; then
 echo ""
 echo "Building x86_64 platform, skipping x86"
 echo ""
 ./build-x86_64.sh
 echo "Starting hardened AppOS build.."
 ./hbuild-x86_64.sh
else
 echo ""
 echo "Building x86 platform, skipping x86_64"
 echo ""
 ./build-x86.sh
 echo "Starting hardened AppOS build.."
 ./hbuild-x86.sh
fi

#
# TODO:
#
#  The script now needs to do a couple of things
#
#  1. extract the development chroot
#  2. extract the core AppOS
#  3. create the cpio for AppOS
#  4. build kernels for all platforms
#



