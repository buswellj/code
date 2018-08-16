#!/bin/bash
ACV="5.10.0"
ARC=".tar.bz2"
APN="perl"
export ACV ARC APN
ACB=$APN-$ACV
export ACB
###################################
#
#     Perl-5.8.8
#
###################################
cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB
$PTC ../perl-5.10.0-libc-2.patch
./configure.gnu --prefix=/tools -Dstatic_ext='Data/Dumper Fcntl IO POSIX'
make perl utilities
install -v perl pod/pod2man /tools/bin
install -vd /tools/lib/perl5/5.10.0
cp -vR lib/* /tools/lib/perl5/5.10.0

