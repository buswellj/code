#!/bin/bash
ACV="2.12r"
ARC=".tar.bz2"
APN="util-linux"
export ACV ARC APN
ACB=$APN-$ACV
export ACB
###################################
#
#  Util-linux-2.12r
#
###################################
cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB
sed 's@/usr/include@/tools/include@g' -i.orig configure
./configure
make -C lib
make -C mount mount umount
make -C text-utils more
install -v mount/{,u}mount text-utils/more /tools/bin

