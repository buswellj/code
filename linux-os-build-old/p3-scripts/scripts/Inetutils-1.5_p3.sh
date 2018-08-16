#!/bin/bash
ACV="1.5"
ARC=".tar.bz2"
APN="Inetutils"
export ACV ARC APN
ACB=$APN-$ACV
export ACB
#
#Inetutils-1.5
#
##########################################
cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB
patch -Np1 -i ../inetutils-1.5-no_server_man_pages-2.patch

./configure --prefix=/usr --libexecdir=/usr/sbin \
    --sysconfdir=/etc --localstatedir=/var \
    --disable-ifconfig --disable-logger --disable-syslogd \
    --disable-whois --disable-servers

make
make install
mv -v /usr/bin/ping /bin
