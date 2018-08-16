#!/bin/bash
ACV="2.8.1"
ARC=".tar.bz2"
APN="Diffutils"
export ACV ARC APN
ACB=$APN-$ACV
export ACB
#
# Diffutils-2.8.1
#
##########################################
cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB
patch -Np1 -i ../diffutils-2.8.1-i18n-1.patch
touch man/diff.1
./configure --prefix=/usr
make
make install
