#!/bin/bash
ACV="1.06.95"
ARC=".tar.bz2"
APN="bc"
export ACV ARC APN
ACB=$APN-$ACV
export ACB
###################################
#
#    BC-1.06.95
#
###################################
cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB
./configure --prefix=/tools
make
make install
