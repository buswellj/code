#!/bin/bash
ACV="3.2.2"
ARC=".tar.bz2"
APN="Module-Init-Tools"
export ACV ARC APN
ACB=$APN-$ACV
export ACB
#
# Module-Init-Tools-3.2.2
#
##########################################
cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB
patch -Np1 -i ../module-init-tools-3.2.2-modprobe-1.patch
./configure
#make check
make distclean
./configure --prefix=/ --enable-zlib
make
make INSTALL=install install


