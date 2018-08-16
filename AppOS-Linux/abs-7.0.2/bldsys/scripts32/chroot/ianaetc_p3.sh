#!/tools/bin/bash
ACV="2.20"
ARC=".tar.bz2"
APN="iana-etc"
export ACV ARC APN
ACB=$APN-$ACV
export ACB
#
# Iana-Etc-2.20
#
##########################################
cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB
make
make install

