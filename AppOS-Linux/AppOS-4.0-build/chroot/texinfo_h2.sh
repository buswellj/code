#!/bin/bash
ACV="4.11"
ARC=".tar.bz2"
APN="texinfo"
export ACV ARC APN
ACB=$APN-$ACV
export ACB
###################################
#
#     Texinfo-4.11
#
###################################
cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB
./configure --prefix=/usr
make
make install
make TEXMF=/usr/share/texmf install-tex
cd /usr/share/info
rm dir
for f in *
do install-info $f dir 2>/dev/null
done
