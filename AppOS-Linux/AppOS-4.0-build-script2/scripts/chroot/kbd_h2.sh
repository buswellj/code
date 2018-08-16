#!/bin/bash
ACV="1.13"
ARC=".tar.bz2"
APN="kbd"
export ACV ARC APN
ACB=$APN-$ACV
export ACB
###################################
#
#    Kbd-1.13
#
###################################
cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB
./configure
make
make install
