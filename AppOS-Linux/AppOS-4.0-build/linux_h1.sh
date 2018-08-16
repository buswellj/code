#!/bin/bash
ACV="2.6.24.2"
ARC=".tar.bz2"
APN="linux"
export ACV ARC APN
ACB=$APN-$ACV
export ACB
###################################
#
#      Linux-2.6.19.7 API Headers
#
###################################
cd $LSB
$TC $LSB/$ACB$ARC
cd $ACB
make mrproper
make headers_install
make headers_check
cp -av usr/include/* /tools/include
