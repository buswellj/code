#!/bin/bash
ACV="2.6.23"
ARC=".tar.bz2"
APN="iproute2"
export ACV ARC APN
ACB=$APN-$ACV
export ACB
#
# IPRoute2-2.6.23
#
##########################################
cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB
make SBINDIR=/sbin

make SBINDIR=/sbin install
mv -v /sbin/arpd /usr/sbin

