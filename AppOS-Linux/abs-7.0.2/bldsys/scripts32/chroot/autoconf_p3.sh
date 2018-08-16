#!/tools/bin/bash
ACV="2.61"
ARC=".tar.bz2"
APN="autoconf"
export ACV ARC APN
ACB=$APN-$ACV
export ACB
#
# Autoconf-2.61
#
##########################################
cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB
./configure --prefix=/usr
make
make install
