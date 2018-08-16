#!/bin/bash

ACV="4.11"
ARC=".tar.bz2"
APN="texinfo"
export ACV ARC APN

ACB=$APN-$ACV
export ACB

#
# texinfo
#
#####################################

cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB

./configure --prefix=/tools
make
make install


