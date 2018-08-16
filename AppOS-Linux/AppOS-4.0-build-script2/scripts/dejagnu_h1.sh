#!/bin/bash
ACV="1.4.4"
ARC=".tar.bz2"
APN="dejagnu"
export ACV ARC APN
ACB=$APN-$ACV
export ACB
###################################
#
#   DejaGNU-1.4.4
#
###################################
cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB
./configure --prefix=/tools
make install
