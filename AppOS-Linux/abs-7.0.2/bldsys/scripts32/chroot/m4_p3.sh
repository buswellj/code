#!/tools/bin/bash
ACV="1.4.10"
ARC=".tar.bz2"
APN="m4"
export ACV ARC APN
ACB=$APN-$ACV
export ACB
#
# M4-1.4.10
#
##########################################
cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB
./configure --prefix=/usr
make
make install
