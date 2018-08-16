#!/bin/bash
ACV="3.4"
ARC=".tar.bz2"
APN="module-init-tools"
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
$PTC $LSP/module-init-tools-3.4-manpages-1.patch
./configure
#make check
make clean
./configure --prefix=/ --enable-zlib
make
make INSTALL=install install

