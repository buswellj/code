#!/bin/bash
ACV="0.17"
ARC=".tar.gz"
APN="gettext"
export ACV ARC APN
ACB=$APN-$ACV
export ACB
#
# Gettext-0.17
#
##########################################
cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB

./configure --prefix=/usr
make
make install

