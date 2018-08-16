#!/bin/bash
ACV="4.2.32"
ARC=".tar.gz"
APN="findutils"
export ACV ARC APN
ACB=$APN-$ACV
export ACB
#
# Findutils-4.2.32
#
##########################################
cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB
./configure --prefix=/usr --libexecdir=/usr/lib/findutils \
    --localstatedir=/var/lib/locate
make
make install
mv -v /usr/bin/find /bin
sed -i -e 's/find:=${BINDIR}/find:=\/bin/' /usr/bin/updatedb


