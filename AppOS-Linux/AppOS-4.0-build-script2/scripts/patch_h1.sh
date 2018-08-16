#!/bin/bash
ACV="2.5.9"
ARC=".tar.gz"
APN="patch"
export ACV ARC APN
ACB=$APN-$ACV
export ACB
###################################
#
#    Patch-2.5.9
#
###################################
cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB
./configure --prefix=/tools
make
make install

