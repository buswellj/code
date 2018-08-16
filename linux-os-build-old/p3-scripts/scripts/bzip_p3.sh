#!/bin/bash
ACV="1.0.4"
ARC=".tar.bz2"
APN="Bzip2"
export ACV ARC APN
ACB=$APN-$ACV
export ACB
#
# Bzip2-1.0.4
#
##########################################
cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB
patch -Np1 -i ../bzip2-1.0.4-install_docs-1.patch
make -f Makefile-libbz2_so
make clean
make
make PREFIX=/usr install
cp -v bzip2-shared /bin/bzip2
cp -av libbz2.so* /lib
ln -sv ../../lib/libbz2.so.1.0 /usr/lib/libbz2.so
rm -v /usr/bin/{bunzip2,bzcat,bzip2}
ln -sv bzip2 /bin/bunzip2
ln -sv bzip2 /bin/bzcat
