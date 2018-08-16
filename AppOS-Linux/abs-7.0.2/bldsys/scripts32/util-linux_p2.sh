#!/bin/bash

ACV="2.13.1"
ARC=".tar.bz2"
APN="util-linux-ng"
export ACV ARC APN

ACB=$APN-$ACV
export ACB

#
# util-linux-ng
#
#####################################

cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB

./configure --prefix=/tools
make -C mount mount umount
make -C text-utils more
cp -v mount/{,u}mount text-utils/more /tools/bin



