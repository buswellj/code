#!/bin/bash
ACV="5.6"
ARC=".tar.bz2"
APN="Ncurses"
export ACV ARC APN
ACB=$APN-$ACV
export ACB
#
# Ncurses-5.6
#
##########################################
cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB
patch -Np1 -i ../ncurses-5.6-coverity_fixes-1.patch


./configure --prefix=/usr --with-shared --without-debug --enable-widec
make
make install
chmod -v 644 /usr/lib/libncurses++w.a
mv -v /usr/lib/libncursesw.so.5* /lib
ln -sfv ../../lib/libncursesw.so.5 /usr/lib/libncursesw.so
for lib in curses ncurses form panel menu ; do \
    rm -vf /usr/lib/lib${lib}.so ; \
    echo "INPUT(-l${lib}w)" >/usr/lib/lib${lib}.so ; \
    ln -sfv lib${lib}w.a /usr/lib/lib${lib}.a ; \
done
ln -sfv libncurses++w.a /usr/lib/libncurses++.a

make distclean
./configure --prefix=/usr --with-shared --without-normal \
  --without-debug --without-cxx-binding
make sources libs
cp -av lib/lib*.so.5* /usr/lib
