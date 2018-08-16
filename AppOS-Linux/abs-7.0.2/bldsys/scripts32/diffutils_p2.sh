#!/bin/bash

ACV="2.8.1"
ARC=".tar.gz"
APN="diffutils"
export ACV ARC APN

ACB=$APN-$ACV
export ACB

#
# diffutils
#
#####################################

cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB

./configure --prefix=/tools
make
make install


