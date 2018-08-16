#!/bin/bash
ACV="2.8.7"
ARC=".tar.bz2"
APN="diffutils"
export ACV ARC APN
ACB=$APN-$ACV
export ACB
###################################
#
#   Diffutils-2.8.7
#
###################################
cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB
$PTC ../diffutils-2.8.7-hardened_tmp-1.patch
./configure --prefix=/usr
make
make install

