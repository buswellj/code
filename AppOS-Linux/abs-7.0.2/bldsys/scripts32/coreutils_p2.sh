#!/bin/bash

ACV="6.10"
ARC=".tar.gz"
APN="coreutils"
export ACV ARC APN

ACB=$APN-$ACV
export ACB

#
# ncurses
#
#####################################

cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB

./configure --prefix=/tools --enable-install-program=hostname
make
make install
cp -v src/su /tools/bin/su-tools


