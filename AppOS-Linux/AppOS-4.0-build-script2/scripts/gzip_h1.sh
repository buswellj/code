#!/bin/bash
ACV="1.3.12"
ARC=".tar.gz"
APN="gzip"
export ACV ARC APN
ACB=$APN-$ACV
export ACB
###################################
#
#    Gzip-1.3.12
#
###################################
cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB
env DEFS=NO_ASM ./configure --prefix=/tools
make
make install
