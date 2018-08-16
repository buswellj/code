#!/bin/bash
ACV="1.0.4"
ARC=".tar.bz2"
APN="bzip2"
export ACV ARC APN
ACB=$APN-$ACV
export ACB
###################################
#
#    Bzip2-1.0.4
#
###################################
cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB
$PTC ../bzip2-1.0.4-install_docs-1.patch
make -f Makefile-libbz2_so
make clean
make
make PREFIX=/usr install
install -v bzip2-shared /bin/bzip2
cp -va libbz2.so* /lib
ln -vs ../../lib/libbz2.so.1.0 /usr/lib/libbz2.so
rm -v /usr/bin/{bunzip2,bzcat,bzip2}
ln -vs bzip2 /bin/bunzip2
ln -vs bzip2 /bin/bzcat
mv -v /usr/lib/libbz2.a /usr/lib/static/ 
