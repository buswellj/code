#!/bin/bash
ACV="1.19"
ARC=".tar.bz2"
APN="Tar"
export ACV ARC APN
ACB=$APN-$ACV
export ACB
#
# Tar-1.19
#
##########################################
cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB
./configure --prefix=/usr --bindir=/bin --libexecdir=/usr/sbin

make 
make install

