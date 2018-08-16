#!/bin/bash
ACV="0.97"
ARC=".tar.bz2"
APN=" GRUB"
export ACV ARC APN
ACB=$APN-$ACV
export ACB
#
#  GRUB-0.97
#
##########################################
cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB
patch -Np1 -i ../grub-0.97-disk_geometry-1.patch
./configure --prefix=/usr
make
make install
mkdir -v /boot/grub
cp -v /usr/lib/grub/i386-pc/stage{1,2} /boot/grub
