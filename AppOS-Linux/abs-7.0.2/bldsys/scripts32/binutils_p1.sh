#!/bin/bash

ACV="2.18"
ARC=".tar.bz2"
APN="binutils"
export ACV ARC APN

ACB=$APN-$ACV
export ACB

#
# binutils pass 1
#
#####################################

cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB

$PTC $LSP/binutils-2.18-configure-1.patch
mkdir -v ../binutils-build
cd ../binutils-build

CC="gcc -B/usr/bin/" ../$ACB/configure \
    --prefix=/tools --disable-nls --disable-werror

make
make install

make -C ld clean
make -C ld LIB_PATH=/tools/lib
cp -v ld/ld-new /tools/bin

