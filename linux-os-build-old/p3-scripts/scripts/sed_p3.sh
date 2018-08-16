#!/bin/bash
ACV="4.1.5"
ARC=".tar.bz2"
APN="Sed"
export ACV ARC APN
ACB=$APN-$ACV
export ACB
#
# Sed-4.1.5
#
##########################################
cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB
./configure --prefix=/usr --bindir=/bin --enable-html
make
#make check
make install
