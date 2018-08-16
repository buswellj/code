#!/bin/bash

ACV="1.4.4"
ARC=".tar.gz"
APN="dejagnu"
export ACV ARC APN

ACB=$APN-$ACV
export ACB

#
# dejagnu pass 1
#
#####################################

cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB

./configure --prefix=/tools
make install


