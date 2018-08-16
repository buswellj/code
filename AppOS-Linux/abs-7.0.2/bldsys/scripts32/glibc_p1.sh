#!/bin/bash

ACV="2.7"
ARC=".tar.bz2"
APN="glibc"
export ACV ARC APN

ACB=$APN-$ACV
export ACB

#
# glibc pass 1
#
#####################################

cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB

mkdir -v ../glibc-build
cd ../glibc-build

echo "CFLAGS += -march=k8" > configparms

../$ACB/configure --prefix=/tools \
    --disable-profile --enable-add-ons \
    --enable-kernel=2.6.0 --with-binutils=/tools/bin \
    --without-gd --with-headers=/tools/include \
    --without-selinux

make
make check
mkdir -v /tools/etc
touch /tools/etc/ld.so.conf
make install

mv -v /tools/bin/{ld,ld-old}
mv -v /tools/$(gcc -dumpmachine)/bin/{ld,ld-old}
mv -v /tools/bin/{ld-new,ld}
ln -sv /tools/bin/ld /tools/$(gcc -dumpmachine)/bin/ld

gcc -dumpspecs | sed 's@/lib/ld-linux.so.2@/tools&@g' \
  > `dirname $(gcc -print-libgcc-file-name)`/specs

GCC_INCLUDEDIR=`dirname $(gcc -print-libgcc-file-name)`/include &&
find ${GCC_INCLUDEDIR}/* -maxdepth 0 -xtype d -exec rm -rvf '{}' \; &&
rm -vf `grep -l "DO NOT EDIT THIS FILE" ${GCC_INCLUDEDIR}/*` &&
unset GCC_INCLUDEDIR


