#!/bin/bash
ACV="5.10.0"
ARC=".tar.bz2"
APN="perl"
export ACV ARC APN
ACB=$APN-$ACV
export ACB
###################################
#
#      Perl-5.8.8
#
###################################
cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB
echo "127.0.0.1 localhost $(hostname)" > /etc/hosts
./configure.gnu --prefix=/usr -Duseshrplib \
    -Dman1dir=/usr/share/man/man1 \
    -Dman3dir=/usr/share/man/man3 \
    -Dpager="/usr/bin/less -isR"
make
make install
