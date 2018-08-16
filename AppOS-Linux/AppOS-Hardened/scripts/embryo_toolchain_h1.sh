#!/bin/bash
ACV1="4.1.2"
ACV="2.17"
ARC=".tar.bz2"
ARC1=".tar.bz2"
APN="binutils"
APN1="gcc"
export ACV ARC APN
export ACV1 ARC1 APN1
ACB=$APN-$ACV
ACB1=$APN1-$ACV1
export ACB
export ACB1
###################################
#
#      Embryo Toolchain
#
###################################
cd $LSB
$TC $LSR/$ACB$ARC
$TC $LSR/$ACB1$ARC1
mv $ACB1/ embryo-toolchain
mv -v $ACB embryo-toolchain
cd embryo-toolchain/
ln -vs $ACB/{bfd,bintils,gas,gprof,ld,opcodes} .
cp -vi gcc/config/i386/linux.h{,.orig}
sed 's/^\(#define CC1_SPEC.*\)\("\)$/\1 %{fno-pic|fpic|fPIC:;:-fPIC}\2/' \
    gcc/config/i386/linux.h.orig > gcc/config/i386/linux.h
mkdir -v ../embryo-build/
cd ../embryo-build/
../embryo-toolchain/configure --prefix=/tools \
    --with-local-prefix=/tools --disable-nls \
    --enable-languages=c --enable-checking \
    --enable-werror --enable-bootstrap
make
make install
ln -vs gcc /tools/bin/cc
ln -vs stage3-bfd/ bfd
ln -vs stage3-libiberty/ libiberty
cp -va stage3-ld/ tools-ld
make -C tools-ld/ clean
make -C tools-ld/ LIB_PATH=/tools/lib CC=/tools/bin/gcc
make -C tools-ld/ CC=/tools/bin/gcc EXEEXT=-new install-exec-local




