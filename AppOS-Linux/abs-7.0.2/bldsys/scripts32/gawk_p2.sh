#!/bin/bash

ACV="3.1.6"
ARC=".tar.bz2"
APN="gawk"
export ACV ARC APN

ACB=$APN-$ACV
export ACB

#
# gawk
#
#####################################

cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB

./configure --prefix=/tools
make
make install


