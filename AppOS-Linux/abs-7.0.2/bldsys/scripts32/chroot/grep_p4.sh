#!/bin/bash
ACV="2.5.3"
ARC=".tar.bz2"
APN="grep"
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
$PTC $LSP/grep-2.5.3-debian_fixes-1.patch
$PTC $LSP/grep-2.5.3-upstream_fixes-1.patch
./configure --prefix=/usr --bindir=/bin
make
#make check || true
make install

