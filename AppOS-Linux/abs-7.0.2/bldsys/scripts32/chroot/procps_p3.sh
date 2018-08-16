#!/tools/bin/bash
ACV="3.2.7"
ARC=".tar.gz"
APN="procps"
export ACV ARC APN
ACB=$APN-$ACV
export ACB
#
# Procps-3.2.7
#
##########################################
cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB
make
make install
