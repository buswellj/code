#!/bin/bash
ACV="3.2.7"
ARC=".tar.bz2"
APN="Procps"
export ACV ARC APN
ACB=$APN-$ACV
export ACB
#
# Procps-3.2.7
#
##########################################
cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB
make
make install
