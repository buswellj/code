#!/bin/bash
ACV="2.3"
ARC=".tar.bz2"
APN="Bison"
export ACV ARC APN
ACB=$APN-$ACV
export ACB
#
# Bison-2.3
#
##########################################
cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB
./configure --prefix=/usr
echo '#define YYENABLE_NLS 1' >> config.h
make
make install
