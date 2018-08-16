#!/bin/bash

ACV="2.5.3"
ARC=".tar.bz2"
APN="grep"
export ACV ARC APN

ACB=$APN-$ACV
export ACB

#
# grep
#
#####################################

cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB

./configure --prefix=/tools \
    --disable-perl-regexp
make
make install


