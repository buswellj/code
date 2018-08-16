#!/bin/bash
ACV="2.5.33"
ARC=".tar.bz2"
APN="flex"
export ACV ARC APN
ACB=$APN-$ACV
export ACB
###################################
#
#  Flex-2.5.33
#
###################################
cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB
./configure --prefix=/tools
make
make install
