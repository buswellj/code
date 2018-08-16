#!/bin/bash
ACV="4.1.2"
ARC=".tar.bz2"
APN="gcc"
ACV1="2.17"
ARC1=".tar.bz2"
export ACV ARC APN
export ACV1 ARC1 APN1
ACB1=$APN1-$ACV1
ACB=$APN-$ACV
export ACB
export ACB1
###################################
#
#     Butterfly Toolchain
#
###################################
cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB
mv -v $ACB1/ butterfly-toolchain
mv -v $ACB/ butterfly-toolchain
cd butterfly-toolchain/
ln -vs $ACB1/{bfd,binutils,gas,gprof,ld,opcodes} .
$PTC $LSP/binutils-2.17-branch_update-2.patch
$PTC $LSP/binutils-2.17-hardened_tmp-3.patch
$PTC $LSP/binutils-2.17-lazy-1.patch
rm -v ld/ld.{1,info}
$PTC $LSP/binutils-2.17-fortify_warnings-1.patch
$PTC $LSP/binutils-2.17-PR4304-1.patch
cd $ACB1/
$PTC $LSP/binutils-2.17-pt_pax-1.patch
cd ..
$PTC $LSP/gcc-4.1.2-PR26864-1.patch
$PTC $LSP/gcc-4.1.2-DW_CFA_val-1.patch
$PTC $LSP/gcc-4.1.2-Wno_overlength_strings-1.patch
$PTC $LSP/gcc-4.1.2-strncat_chk-1.patch
$PTC $LSP/gcc-4.1.2-fortify_source-2.patch
$PTC $LSP/gcc-4.1.2-fstack_protector-1.patch
$PTC $LSP/gcc-4.1.2-fpie-2.patch
sed -e 's@/.:$$r@/.libs:$$r@' -e 's@/.:@/.libs:@' -i.orig Makefile.in
sed 's/violation_mode = viol_nop/violation_mode = viol_abort/' \
    -i.orig libmudflap/mf-runtime.c
mkdir -v ../butterfly-build
cd ../butterfly-build
../butterfly-toolchain/configure --prefix=/usr \
    --libexecdir=/usr/lib --enable-shared \
    --enable-threads=posix --enable-__cxa_atexit \
    --enable-clocale=gnu --enable-languages=c,c++ \
    --enable-checking --disable-werror
make tooldir=/usr
make -k check
make tooldir=/usr install
mv -v /usr/lib/libmudflap{,th}.so* /lib
ln -vsf ../../lib/libmudflap.so.0 /usr/lib/libmudflap.so
ln -vsf ../../lib/libmudflapth.so.0 /usr/lib/libmudflapth.so
mv -v /usr/lib/{libsupc++,libstdc++}.a /usr/lib/static/
mv -v /usr/lib/{libbfd,libiberty,libmudflap,libmudflapth}.a /usr/lib/static
mv -v /usr/lib/{libopcodes,libssp,libssp_nonshared}.a /usr/lib/static
mv -v /usr/lib/gcc/$(gcc -dumpmachine)/4.1.2/libgcov.a /usr/lib/static/
mv -v /usr/lib/{libc,libdl}.a /usr/lib/static
ln -vs ../usr/bin/cpp /lib
ln -vs gcc /usr/bin/cc
install -v -m0644 ../butterfly-toolchain/include/libiberty.h \
    /usr/include
























 
