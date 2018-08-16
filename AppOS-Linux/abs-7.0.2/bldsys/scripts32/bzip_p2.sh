#!/bin/bash

ACV="1.0.4"
ARC=".tar.gz"
APN="bzip2"
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

make
make PREFIX=/tools install
