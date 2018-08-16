#!/bin/bash

ACV="5.8.8"
ARC=".tar.bz2"
APN="perl"
export ACV ARC APN

ACB=$APN-$ACV
export ACB

#
# patch
#
#####################################

cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB

$PTC $LSP/perl-5.8.8-libc-2.patch
mv -v makedepend.SH{,.orig}
sed 's/command /command[ -]/' makedepend.SH.orig > makedepend.SH
./configure.gnu --prefix=/tools -Dstatic_ext='Data/Dumper Fcntl IO POSIX'
make perl utilities ext/Errno/pm_to_blib
cp -v perl pod/pod2man /tools/bin
mkdir -pv /tools/lib/perl5/5.8.8
cp -Rv lib/* /tools/lib/perl5/5.8.8


