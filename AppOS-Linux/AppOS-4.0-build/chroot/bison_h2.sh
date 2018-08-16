#!/bin/bash
ACV="2.3"
ARC=".tar.gz"
APN="bison"
export ACV ARC APN
ACB=$APN-$ACV
export ACB
###################################
#
#    Bison-2.3
#
###################################
cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB
./configure --prefix=/usr --enable-gcc-warnings
echo '#define YYENABLE_NLS 1' >> config.h
make
make install
cd lib/
gcc -shared -Wl,-soname,liby.so.2.3 -o liby.so.2.3 -fPIC main.o yyerror.o
rm -v /usr/lib/liby.a
install -v liby.so.2.3 /usr/lib/liby.so.2.3
ln -vsf liby.so.2.3 /usr/lib/liby.so

