#!/bin/bash
ACV="1.2.3"
ARC=".tar.bz2"
APN=" Zlib"
export ACV ARC APN
ACB=$APN-$ACV
export ACB
#
# Man- Zlib-1.2.3
#
##########################################
cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB
./configure --prefix=/usr --shared --libdir=/lib
make
make install
rm -v /lib/libz.so
ln -sfv ../../lib/libz.so.1.2.3 /usr/lib/libz.so
make clean
./configure --prefix=/usr
make
make install
chmod -v 644 /usr/lib/libz
