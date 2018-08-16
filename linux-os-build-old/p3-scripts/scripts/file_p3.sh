#!/bin/bash
ACV="4.23"
ARC=".tar.bz2"
APN="File"
export ACV ARC APN
ACB=$APN-$ACV
export ACB
#
# File-4.23
#
##########################################
cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB
./configure --prefix=/usr

make
make install
