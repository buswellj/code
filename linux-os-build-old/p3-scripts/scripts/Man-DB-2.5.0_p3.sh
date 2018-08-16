#!/bin/bash
ACV="2.5.0"
ARC=".tar.bz2"
APN="Man-DB"
export ACV ARC APN
ACB=$APN-$ACV
export ACB
#
# Man-DB-2.5.0
#
##########################################
cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB
sed -i -e '\%\t/usr/man%d' -e '\%\t/usr/local/man%d' src/man_db.conf.in
cat >> include/manconfig.h.in << "EOF"
#define WEB_BROWSER "exec /usr/bin/lynx"
#define COL "/usr/bin/col"
#define VGRIND "/usr/bin/vgrind"
#define GRAP "/usr/bin/grap"
EOF
./configure --prefix=/usr --enable-mb-groff --disable-setuid
make
make install
