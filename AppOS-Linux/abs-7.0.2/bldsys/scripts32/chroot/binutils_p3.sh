#!/tools/bin/bash
ACV="2.18"
ARC=".tar.bz2"
APN="binutils"
export ACV ARC APN
ACB=$APN-$ACV
export ACB
#
# Binutils-2.18
#
##########################################
cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB
expect -c "spawn ls"
$PTC $LSP/binutils-2.18-configure-1.patch
mkdir -v ../binutils-build
cd ../binutils-build
../$ACB/configure --prefix=/usr --enable-shared
make tooldir=/usr
#make check
make tooldir=/usr install
cp -v ../$ACB/include/libiberty.h /usr/include

