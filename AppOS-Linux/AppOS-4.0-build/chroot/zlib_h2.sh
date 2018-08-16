#!/bin/bash
ACV="1.2.3"
ARC=".tar.gz"
APN="zlib"
export ACV ARC APN
ACB=$APN-$ACV
export ACB
###################################
#
#    Zlib-1.2.3
#
###################################
cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB
sed '0,/^# udpate Makefile/s//CFLAGS="$CFLAGS \\\
    -Wall -Wformat-security -Wwrite-strings -Wpointer-arith \\\
    -Wstrict-prototypes -Wmissing-prototypes -Werror -Wfatal-errors"\n&/' \
    -i.orig configure
./configure --prefix=/usr --shared --libdir=/lib
make
make install
rm -v /lib/libz.so
ln -vsf ../../lib/libz.so.1.2.3 /usr/lib/libz.so

