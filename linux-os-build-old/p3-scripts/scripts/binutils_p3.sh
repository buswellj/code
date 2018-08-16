#!/bin/bash
ACV="2.18"
ARC=".tar.bz2"
APN="Binutils"
export ACV ARC APN
ACB=$APN-$ACV
export ACB
#
# Binutils-2.18
#
##########################################
cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB
expect -c "spawn ls"
patch -Np1 -i ../binutils-2.18-configure-1.patch
mkdir -v ../binutils-build
cd ../binutils-build
../binutils-2.18/configure --prefix=/usr \
    --enable-shared
make tooldir=/usr
#make check
make tooldir=/usr install
cp -v ../binutils-2.18/include/libiberty.h /usr/include

