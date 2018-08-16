#!/bin/bash
ACV="2.5.1a"
ARC=".tar.bz2"
APN="grep"
export ACV ARC APN
ACB=$APN-$ACV
export ACB
###################################
#
#    Grep-2.5.1a
#
###################################
cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB
./configure --prefix=/tools --disable-perl-regexp
make
make install
