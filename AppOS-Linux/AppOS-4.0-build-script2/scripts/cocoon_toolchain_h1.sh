#!/bin/bash
ACV="4.1.2"
ACV1="2.17"
ARC1=".tar.bz2"
ARC=".tar.bz2"
APN="gcc"
APN1="binutils"
export ACV ARC APN
export ACV1 ARC1 APN1
ACB=$APN-$ACV
ACB1=$APN1-$ACV1
export ACB
export ACB
###################################
#
#    Cocoon Toolchain
#
###################################
cd $LSB
$TC $LSR/$ACB$ARC
$TC $LSR/$ACB1$ARC1
mv -v $ACB/ cocoon-toolchain
mv -v $ACB1/ cocoon-toolchain
cd cocoon-toolchain/
ln -vs $ACB1/{bfd,binutils,gas,gprof,ld,opcodes} .
$PTC $LSP/binutils-2.17-lazy-1.patch
cd $ACB1
$PTC $LSP/binutils-2.17-pt_pax-1.patch
cd ../
$PTC $LSP/gcc-4.1.2-DW_CFA_val-1.patch
$PTC $LSP/gcc-4.1.2-Wno_overlength_strings-1.patch
$PTC $LSP/gcc-4.1.2-strncat_chk-1.patch
$PTC $LSP/gcc-4.1.2-fortify_source-2.patch
$PTC $LSP/gcc-4.1.2-fstack_protector-1.patch
$PTC $LSP/gcc-4.1.2-fpie-2.patch
cp -vi Makefile.in{,.orig}
sed -e 's@/.:$$r@/.libs:$$r@' -e 's@/.:@/.libs:@' \
    Makefile.in.orig > Makefile.in
cp -vi gcc/Makefile.in{,.orig2}
sed 's@\./fixinc\.sh@-c true@' gcc/Makefile.in.orig2 > gcc/Makefile.in
cp -v gcc/config/i386/linux.h{,.orig}
sed 's@/lib/ld-linux.so.2@/tools&@' \
    gcc/config/i386/linux.h.orig > gcc/config/i386/linux.h
cp -v gcc/config/linux.h{,.orig}
echo "#undef STANDARD_INCLUDE_DIR
#define STANDARD_INCLUDE_DIR 0" >> gcc/config/linux.h
mkdir -v ../cocoon-build/
cd ../cocoon-build/
../cocoon-toolchain/configure --prefix=/tools \
    --with-local-prefix=/tools --enable-clocale=gnu \
    --enable-shared --enable-threads=posix \
    --enable-__cxa_atexit --enable-languages=c,c++ \
    --with-lib-path=/tools/lib --disable-libstdcxx-pch \
    --enable-checking --enable-werror
make
make install
make -C ld clean
make -C ld LIB_PATH=/usr/lib:/lib
make -C ld EXEEXT=-new install-exec-local
