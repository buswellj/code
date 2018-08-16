#!/bin/bash

ACV="5.6"
ARC=".tar.gz"
APN="ncurses"
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

./configure --prefix=/tools --with-shared \
    --without-debug --without-ada --enable-overwrite
make
make install

