#!/bin/bash
ACV="2.5.1a"
ARC=".tar.bz2"
APN="Grep"
export ACV ARC APN
ACB=$APN-$ACV
export ACB
#
# Grep-2.5.1a
#
##########################################
cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB
patch -Np1 -i ../grep-2.5.1a-redhat_fixes-2.patch
chmod +x tests/fmbtest.sh
./configure --prefix=/usr --bindir=/bin
make
make install
