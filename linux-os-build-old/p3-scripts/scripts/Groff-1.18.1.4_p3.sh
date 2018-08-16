#!/bin/bash
ACV="1.18.1.4"
ARC=".tar.bz2"
APN="Groff"
export ACV ARC APN
ACB=$APN-$ACV
export ACB
#
#  Groff-1.18.1.4
#
##########################################
cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB
patch -Np1 -i ../groff-1.18.1.4-debian_fixes-1.patch
sed -i -e 's/2010/002D/' -e 's/2212/002D/' \
    -e 's/2018/0060/' -e 's/2019/0027/' font/devutf8/R.proto
PAGE=<paper_size> ./configure --prefix=/usr --enable-multibyte
make
make install
ln -sv eqn /usr/bin/geqn
ln -sv tbl /usr/bin/gtbl
