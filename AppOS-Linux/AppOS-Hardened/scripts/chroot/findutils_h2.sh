#!/bin/bash
ACV="4.2.33"
ARC=".tar.gz"
APN="findutils"
export ACV ARC APN
ACB=$APN-$ACV
export ACB
###################################
#
#   Findutils-4.2.33
#
###################################
cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB
./configure --prefix=/usr --libexecdir=/usr/lib/findutils \
    --localstatedir=/var/lib/locate
make
make install
mv -v /usr/bin/find /bin
sed 's/find:=${BINDIR}/find:=\/bin/' -i /usr/bin/updatedb
