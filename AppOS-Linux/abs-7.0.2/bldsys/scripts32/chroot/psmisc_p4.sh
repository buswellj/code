#!/bin/bash
ACV="22.6"
ARC=".tar.gz"
APN="psmisc"
export ACV ARC APN
ACB=$APN-$ACV
export ACB
#
# Psmisc-22.6
#
##########################################
cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB
./configure --prefix=/usr --exec-prefix=""

make 
make install
mv -v /bin/pstree* /usr/bin
ln -sv killall /bin/pidof

