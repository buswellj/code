#!/bin/bash
ACV="1.6f"
ARC=".tar.gz"
APN="man"
export ACV ARC APN
ACB=$APN-$ACV
export ACB
###################################
#
#    Man-1.6f
#
###################################
cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB
sed 's@-is@&R@g' -i.orig configure
sed 's@MANPATH./usr/man@#&@g' -i.orig src/man.conf.in
./configure -confdir=/etc
make
make install
NROFF  /usr/bin/nroff -Tlatin1 -mandoc
