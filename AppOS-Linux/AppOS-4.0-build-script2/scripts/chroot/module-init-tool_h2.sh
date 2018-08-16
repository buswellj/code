#!/bin/bash
ACV="3.2.2"
ARC=".tar.bz2"
APN="module-init-tool"
export ACV ARC APN
ACB=$APN-$ACV
export ACB
###################################
#
#     Module-Init-Tools-3.2.2
# 
###################################
cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB
$PTC ../module-init-tools-3.2.2-modprobe-1.patch
$PTC ../module-init-tools-3.2.2-nostatic-1.patch
./configure &&
make check &&
make distclean
./configure --prefix=/ --enable-zlib
make
make INSTALL=install install

