#!/tools/bin/bash
ACV="1.10.1"
ARC=".tar.bz2"
APN="automake"
export ACV ARC APN
ACB=$APN-$ACV
export ACB
#
# Automake-1.10.1
#
##########################################
cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB
$PTC $LSP/automake-1.10.1-test_fix-1.patch
./configure --prefix=/usr
make
make install
