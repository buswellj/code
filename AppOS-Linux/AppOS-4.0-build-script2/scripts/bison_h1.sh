#!/bin/bash
ACV="2.3"
ARC=".tar.gz"
APN="bison"
export ACV ARC APN
ACB=$APN-$ACV
export ACB
###################################
#
#     Bison-2.3
#
###################################
cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB
./configure --prefix=/tools
make
make install
