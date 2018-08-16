#!/bin/bash
ACV="3.2"
ARC=".tar.gz"
APN="bash"
export ACV ARC APN
ACB=$APN-$ACV
export ACB
###################################
#
#       Bash-3.2
#
###################################
cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB
$PTC $LSP/bash-3.2-fixes-7.patch
./configure --prefix=/tools --without-bash-malloc
make
make install
ln -vs bash /tools/bin/sh
