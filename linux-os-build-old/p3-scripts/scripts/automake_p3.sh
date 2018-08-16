#!/bin/bash
ACV="1.10.1"
ARC=".tar.bz2"
APN="Automake"
export ACV ARC APN
ACB=$APN-$ACV
export ACB
#
# Automake-1.10.1
#
##########################################
cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB
patch -Np1 -i ../automake-1.10.1-test_fix-1.patch
./configure --prefix=/usr
make
make install
