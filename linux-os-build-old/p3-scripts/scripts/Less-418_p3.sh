#!/bin/bash
ACV="418"
ARC=".tar.bz2"
APN=" Less"
export ACV ARC APN
ACB=$APN-$ACV
export ACB
#
#  Less-418
#
##########################################
cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB
./configure --prefix=/usr --sysconfdir=/etc
make
make install
