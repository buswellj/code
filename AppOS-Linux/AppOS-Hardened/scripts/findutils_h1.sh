#!/bin/bash
ACV="4.2.33"
ARC=".tar.gz"
APN="diffutils"
export ACV ARC APN
ACB=$APN-$ACV
export ACB
###################################
#
#       Findutils-4.2.33
#    
###################################
cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB
./configure --prefix=/tools
make
make install

