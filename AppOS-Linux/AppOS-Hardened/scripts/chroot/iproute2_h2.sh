#!/bin/bash
ACV="2.26.23"
ARC=".tar.gz"
APN="iproute2"
export ACV ARC APN
ACB=$APN-$ACV
export ACB
###################################
#
#   IPRoute2-2.6.23
#
###################################
cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB
sed '/^TARGETS/s@arpd@@g' -i.orig misc/Makefile
make SBINDIR=/sbin
make SBINDIR=/sbin install

