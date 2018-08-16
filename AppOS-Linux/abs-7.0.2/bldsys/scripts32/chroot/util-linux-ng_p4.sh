#!/bin/bash
ACV="2.13.1"
ARC=".tar.bz2"
APN="util-linux-ng"
export ACV ARC APN
ACB=$APN-$ACV
export ACB
#
# Util-linux-ng-2.13.1
#
##########################################
cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB

sed -e 's@etc/adjtime@var/lib/hwclock/adjtime@g' \
    -i $(grep -rl '/etc/adjtime' .)
mkdir -pv /var/lib/hwclock
./configure
make
make install

