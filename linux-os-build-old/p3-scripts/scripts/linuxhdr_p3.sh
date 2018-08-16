#!/bin/bash
ACV="2.6.24"
ARC=".tar.bz2"
APN="linux"
export ACV ARC APN
ACB=$APN-$ACV
export ACB
#
# linux 2.6 headers
#
##########################################
cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB
make mrproper
make headers_check
make INSTALL_HDR_PATH=dest headers_install
cp -rv dest/include/* /usr/include 
