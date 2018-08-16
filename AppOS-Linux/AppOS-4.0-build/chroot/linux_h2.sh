#!/bin/bash
ACV="2.6.24.2"
ARC=".tar.bz2"
APN="linux"
export ACV ARC APN
ACB=$APN-$ACV
export ACB
###################################
#
#   Linux-2.6.19.7 API Headers
#
###################################
cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB
sed '/scsi/d' -i.orig include/Kbuild
make mrproper
make headers_check
make INSTALL_HDR_PATH=/usr headers_install

