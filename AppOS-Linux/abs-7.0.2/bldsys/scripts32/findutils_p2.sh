#!/bin/bash

ACV="4.2.33"
ARC=".tar.gz"
APN="findutils"
export ACV ARC APN

ACB=$APN-$ACV
export ACB

#
# findutils
#
#####################################

cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB

./configure --prefix=/tools
make
make install


