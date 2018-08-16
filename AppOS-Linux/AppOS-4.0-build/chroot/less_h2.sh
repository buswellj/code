#!/bin/bash
ACV="418"
ARC=".tar.gz"
APN="less"
export ACV ARC APN
ACB=$APN-$ACV
export ACB
###################################
#
#   Less-418
#
###################################
cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB
./configure --prefix=/usr --sysconfdir=/etc \
    --with-secure
make
make install

