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
mv binutils-build binutils-build-pass1
mv $ACB $ACB-pass1

$TC $LSR/$ACB$ARC
cd $ACB

$PTC $LSP/binutils-2.18-configure-1.patch
mkdir -v ../binutils-build
cd ../binutils-build

../$ACB/configure --prefix=/tools \
    --disable-nls --with-lib-path=/tools/lib

make
#make check
make install
make -C ld clean
make -C ld LIB_PATH=/usr/lib:/lib
cp -v ld/ld-new /tools/bin

