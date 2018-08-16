#!/bin/bash
ACV="1.5.24"
ARC=".tar.gz"
APN="libtool"
export ACV ARC APN
ACB=$APN-$ACV
export ACB
###################################
#
#      Libtool-1.5.24
#
###################################
cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB
./configure --prefix=/usr --disable-static
make
make install
