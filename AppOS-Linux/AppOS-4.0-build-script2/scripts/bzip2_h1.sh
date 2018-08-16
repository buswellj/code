#!/bin/bash
ACV="1.0.4"
ARC=".tar.gz"
APN="bzip"
export ACV ARC APN
ACB=$APN-$ACV
export ACB
###################################
#
#   Bzip2-1.0.4
#
###################################
cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB
make
make PREFIX=/tools install


