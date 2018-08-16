#!/bin/bash
ACV="2.5.4"
ARC=".tar.bz2"
APN="Patch"
export ACV ARC APN
ACB=$APN-$ACV
export ACB
#
#Patch-2.5.4
#
##########################################
cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB
./configure --prefix=/usr
make 
make install
