#!/bin/bash
ACV="2.5.9"
ARC=".tar.gz"
APN=""
export ACV ARC APN
ACB=$APN-$ACV
export ACB
###################################
#
#  Patch-2.5.9
#
###################################
cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB
$PTC ../patch-2.5.9-fixes-1.patch
$PTC ../patch-2.5.9-mkstemp-1.patch
./configure --prefix=/usr
make
make install
