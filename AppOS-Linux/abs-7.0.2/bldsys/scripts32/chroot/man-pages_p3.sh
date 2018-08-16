#!/tools/bin/bash
ACV="2.78"
ARC=".tar.bz2"
APN="man-pages"
export ACV ARC APN
ACB=$APN-$ACV
export ACB
#
# Man-pages-2.77
#
##########################################
cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB
make install

