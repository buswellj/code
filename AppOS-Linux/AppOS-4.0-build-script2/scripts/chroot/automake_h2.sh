#!/bin/bash
ACV="1.10"
ARC=".tar.bz2"
APN="autoconf"
export ACV ARC APN
ACB=$APN-$ACV
export ACB
###################################
#
#   Automake-1.10
#
###################################
cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB
./configure --prefix=/usr
make
make install
