#!/bin/bash
ACV="0.17"
ARC=".tar.gz"
APN="gettext"
export ACV ARC APN
ACB=$APN-$ACV
export ACB
###################################
#
#   Gettext-0.17
#
###################################
cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB
cd gettext-tools
./configure --prefix=/tools --disable-shared
make -C gnulib-lib
make -C src msgfmt
install -v src/msgfmt /tools/bin

