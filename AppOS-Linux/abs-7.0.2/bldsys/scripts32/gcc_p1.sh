#!/bin/bash

ACV="4.2.3"
ARC=".tar.bz2"
APN="gcc"
export ACV ARC APN

ACB=$APN-$ACV
export ACB

#
# gcc pass 1
#
#####################################

cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB

mkdir -v ../gcc-build
cd ../gcc-build
CC="gcc -B/usr/bin/" ../$ACB/configure --prefix=/tools \
    --with-local-prefix=/tools --disable-nls --enable-shared \
    --enable-languages=c

make
make install
ln -vs gcc /tools/bin/cc

