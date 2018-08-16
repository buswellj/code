#!/bin/bash
ACV="5.2"
ARC=".tar.gz"
APN="readlines"
export ACV ARC APN
ACB=$APN-$ACV
export ACB
###################################
#  
#     Readline-5.2
#
###################################
cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB
$PTC $LSP/readline-5.2-fixes-5.patch
sed '/MV.*old/d' -i.orig Makefile.in
sed '/{OLDSUFF}/c:' -i.orig support/shlib-install
./configure --prefix=/usr --libdir=/lib \
    --disable-static
make SHLIB_LIBS=-lncurses
make install
rm -v /lib/lib{readline,history}.so
ln -vsf ../../lib/libreadline.so.5 /usr/lib/libreadline.so
ln -vsf ../../lib/libhistory.so.5 /usr/lib/libhistory.so

