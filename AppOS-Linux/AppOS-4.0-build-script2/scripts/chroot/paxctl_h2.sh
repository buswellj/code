#!/bin/bash
ACV="0.5"
ARC=".tar.gz"
APN="paxctl"
export ACV ARC APN
ACB=$APN-$ACV
export ACB
###################################
#
#     Paxctl-0.5
#
###################################
cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB
make
make install
paxctl -spm /usr/sbin/grub

