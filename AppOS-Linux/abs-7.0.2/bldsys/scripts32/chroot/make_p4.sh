#!/bin/bash
ACV="3.81"
ARC=".tar.bz2"
APN="make"
export ACV ARC APN
ACB=$APN-$ACV
export ACB
#
# Make-3.81
#
##########################################
cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB
./configure --prefix=/usr
make
make install

