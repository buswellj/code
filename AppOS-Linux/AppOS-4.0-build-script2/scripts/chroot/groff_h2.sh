#!/bin/bash
ACV="1.18.1.4"
ARC=".tar.gz"
APN="groff"
export ACV ARC APN
ACB=$APN-$ACV
export ACB
###################################
#
#     Groff-1.18.1.4   
# 
###################################
cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB
./configure --prefix=/usr
make
make install
ln -vs eqn /usr/bin/geqn
ln -vs tbl /usr/bin/gtbl

