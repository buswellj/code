#!/bin/bash
ACV="4.23"
ARC=".tar.gz"
APN=file""
export ACV ARC APN
ACB=$APN-$ACV
export ACB
###################################
#
#   File-4.23
#
###################################
cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB
./configure --prefix=/usr --disable-static
make
make install
