#!/bin/bash
ACV="2.12r"
ARC=".tar.bz2"
APN="util-linux"
export ACV ARC APN
ACB=$APN-$ACV
export ACB
###################################
#
#  Util-linux-2.12r
#
###################################
cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB
sed 's@etc/adjtime@var/lib/hwclock/adjtime@g' \
    -i.orig hwclock/hwclock.c
mkdir -vp /var/lib/hwclock
$PTC ../util-linux-2.12r-cramfs-1.patch
$PTC ../util-linux-2.12r-lseek-1.patch
./configure
make HAVE_KILL=yes HAVE_SLN=yes
make HAVE_KILL=yes HAVE_SLN=yes install
