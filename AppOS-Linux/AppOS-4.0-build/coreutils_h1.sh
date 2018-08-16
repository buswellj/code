#!/bin/bash
ACV="6.10"
ARC=".tar.bz2"
APN="coreutils"
export ACV ARC APN
ACB=$APN-$ACV
export ACB
###################################
#
#    Coreutils-6.10
#
###################################
cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB
./configure --prefix=/tools --enable-install-program=hostname
make
make install
install -v src/su /tools/bin/su-tools
