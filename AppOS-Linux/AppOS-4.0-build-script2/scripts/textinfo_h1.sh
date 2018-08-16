#!/bin/bash
ACV="4.11"
ARC=".tar.bz2"
APN="textinfo"
export ACV ARC APN
ACB=$APN-$ACV
export ACB
###################################
#
#     Texinfo-4.11
#
###################################
cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB
./configure --prefix=/tools
make
make install
