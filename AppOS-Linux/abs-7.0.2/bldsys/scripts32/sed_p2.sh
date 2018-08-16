#!/bin/bash

ACV="4.1.5"
ARC=".tar.gz"
APN="sed"
export ACV ARC APN

ACB=$APN-$ACV
export ACB

#
# sed
#
#####################################

cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB

./configure --prefix=/tools
make
make install


