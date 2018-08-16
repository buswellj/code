#!/bin/bash

ACV="4.2.3"
ARC=".tar.bz2"
APN="gcc"
export ACV ARC APN

ACB=$APN-$ACV
export ACB

#
# gcc pass 2
#
#####################################

cd $LSB
mv gcc-build gcc-build-pass1
mv $ACB $ACB-pass1

$TC $LSR/$ACB$ARC
cd $ACB
expect -c "spawn ls"
cp -v gcc/Makefile.in{,.orig}
sed 's@\./fixinc\.sh@-c true@' gcc/Makefile.in.orig > gcc/Makefile.in
cp -v gcc/Makefile.in{,.tmp}
sed 's/^XCFLAGS =$/& -fomit-frame-pointer/' gcc/Makefile.in.tmp \
  > gcc/Makefile.in
for file in $(find gcc/config -name linux64.h -o -name linux.h)
do
  cp -uv $file{,.orig}
  sed -e 's@/lib\(64\)\?\(32\)\?/ld@/tools&@g' \
  -e 's@/usr@/tools@g' $file.orig > $file
  echo "
#undef STANDARD_INCLUDE_DIR
#define STANDARD_INCLUDE_DIR 0" >> $file
  touch $file.orig
done
mkdir -v ../gcc-build
cd ../gcc-build

../$ACB/configure --prefix=/tools \
    --with-local-prefix=/tools --enable-clocale=gnu \
    --enable-shared --enable-threads=posix \
    --enable-__cxa_atexit --enable-languages=c,c++ \
    --disable-libstdcxx-pch --disable-bootstrap

make
#make -k check
make install

