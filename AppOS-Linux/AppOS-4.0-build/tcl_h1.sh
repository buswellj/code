#!/bin/bash

ACV="8.4.18"
ARC="-src.tar.gz"
APN="tcl"
export ACV ARC APN

#ACB=$APN-$ACV
ACB=$APN$ACV
export ACB

#
# tcl pass 1
#
#####################################

cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB

cd unix
./configure --prefix=/tools
make
make install
make install-private-headers
ln -sv tclsh8.4 /tools/bin/tclsh

