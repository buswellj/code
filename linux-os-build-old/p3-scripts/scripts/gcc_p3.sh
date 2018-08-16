#!/bin/bash
ACV="4.2.3"
ARC=".tar.bz2"
APN="GCC"
export ACV ARC APN
ACB=$APN-$ACV
export ACB
#
# GCC-4.2.3
#
##########################################
cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB
sed -i 's/install_to_$(INSTALL_DEST) //' libiberty/Makefile.in
sed -i 's/^XCFLAGS =$/& -fomit-frame-pointer/' gcc/Makefile.in
sed -i 's@\./fixinc\.sh@-c true@' gcc/Makefile.in
sed -i 's/@have_mktemp_command@/yes/' gcc/gccbug.in
mkdir -v ../gcc-build
cd ../gcc-build
../gcc-4.2.3/configure --prefix=/usr \
    --libexecdir=/usr/lib --enable-shared \
    --enable-threads=posix --enable-__cxa_atexit \
    --enable-clocale=gnu --enable-languages=c,c++ \
    --disable-bootstrap
make
../gcc-4.2.3/contrib/test_summary
make install
ln -sv ../usr/bin/cpp /lib
ln -sv gcc /usr/bin/cc
echo 'main(){}' > dummy.c
cc dummy.c -v -Wl,--verbose &> dummy.log
readelf -l a.out | grep ': /lib'
grep -o '/usr/lib.*/crt[1in].*succeeded' dummy.log
grep -B3 '^ /usr/include' dummy.log
grep 'SEARCH.*/usr/lib' dummy.log |sed 's|; |\n|g'
grep "/lib/libc.so.6 " dummy.log
rm -v dummy.c a.out dummy.log
