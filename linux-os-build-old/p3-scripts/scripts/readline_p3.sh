#!/bin/bash
ACV="5.2"
ARC=".tar.bz2"
APN="Readline"
export ACV ARC APN
ACB=$APN-$ACV
export ACB
#
#Readline-5.2
#
##########################################
cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB
sed -i '/MV.*old/d' Makefile.in
sed -i '/{OLDSUFF}/c:' support/shlib-install


patch -Np1 -i ../readline-5.2-fixes-5.patch


./configure --prefix=/usr --libdir=/lib
make SHLIB_LIBS=-lncurses
make install
mv -v /lib/lib{readline,history}.a /usr/lib
rm -v /lib/lib{readline,history}.so
ln -sfv ../../lib/libreadline.so.5 /usr/lib/libreadline.so
ln -sfv ../../lib/libhistory.so.5 /usr/lib/libhistory.so


