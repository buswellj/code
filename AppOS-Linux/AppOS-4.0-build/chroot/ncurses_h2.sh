#!/bin/bash
ACV="5.6"
ARC=".tar.gz"
APN="ncurses"
export ACV ARC APN
ACB=$APN-$ACV
export ACB
###################################
#
#   Ncurses-5.6
#
###################################
cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB
$PTC $LSP/ncurses-5.6-coverity_fixes-1.patch
./configure --prefix=/usr --with-shared --without-normal \
    --enable-widec --without-cxx-binding --enable-symlinks \
    --without-debug --disable-root-environ
make
make install
./test/tclock
mv -v /usr/lib/libncursesw.so.5* /lib
ln -vsf ../../lib/libncursesw.so.5 /usr/lib/libncursesw.so
for lib in curses ncurses form panel menu ; do \
    rm -vf /usr/lib/lib${lib}.so ; \
    echo "INPUT(-l${lib}w)" >/usr/lib/lib${lib}.so ; \
done
echo "INPUT(-lncursesw)" >/usr/lib/libcursesw.so
ln -vsf libncurses.so /usr/lib/libcurses.so

