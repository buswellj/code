#!/bin/bash

ACV="1.19"
ARC=".tar.bz2"
APN="tar"
export ACV ARC APN

ACB=$APN-$ACV
export ACB

#
# tar
#
#####################################

cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB

./configure --prefix=/tools
make
make install


