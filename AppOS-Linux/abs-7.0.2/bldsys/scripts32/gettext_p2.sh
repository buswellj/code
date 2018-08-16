#!/bin/bash

ACV="0.17"
ARC=".tar.gz"
APN="gettext"
export ACV ARC APN

ACB=$APN-$ACV
export ACB

#
# gettext
#
#####################################

cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB

cd gettext-tools
./configure --prefix=/tools --disable-shared
make -C gnulib-lib
make -C src msgfmt
cp -v src/msgfmt /tools/bin




