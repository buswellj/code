#!/bin/bash
ACV="1.12"
ARC=".tar.bz2"
APN="Kbd"
export ACV ARC APN
ACB=$APN-$ACV
export ACB
#
# Kbd-1.12
#
##########################################
cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB
patch -Np1 -i ../kbd-1.12-backspace-1.patch
patch -Np1 -i ../kbd-1.12-gcc4_fixes-1.patch
./configure --datadir=/lib/kbd
make
make install
mv -v /usr/bin/{kbd_mode,openvt,setfont} /bin

