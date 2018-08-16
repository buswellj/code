#!/bin/bash
ACV="1.40.6"
ARC=".tar.gz"
APN="e2fsprogs"
export ACV ARC APN
ACB=$APN-$ACV
export ACB
###################################
#
#   E2fsprogs-1.40.6
#
###################################
cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB
mkdir -v build
cd build
../configure --prefix=/usr --with-root-prefix="" \
    --enable-elf-shlibs --disable-evms --enable-dynamic-e2fsck
make
make install
make install-libs
mv -v /usr/lib/{libcom_err,libss,libe2p,libext2fs,libuuid,libblkid}.a \
    /usr/lib/static

