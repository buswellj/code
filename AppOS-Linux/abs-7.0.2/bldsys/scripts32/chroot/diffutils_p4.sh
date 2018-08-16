#!/bin/bash
ACV="2.8.1"
ARC=".tar.gz"
APN="diffutils"
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
$PTC $LSP/diffutils-2.8.1-i18n-1.patch
touch man/diff.1
./configure --prefix=/usr
make
make install

