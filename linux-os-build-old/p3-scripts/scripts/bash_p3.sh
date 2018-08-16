#!/bin/bash
ACV="3.2"
ARC=".tar.bz2"
APN="Bash"
export ACV ARC APN
ACB=$APN-$ACV
export ACB
#
# Bash-3.2
#
##########################################
cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB
tar -xvf ../bash-doc-3.2.tar.gz
sed -i "s|htmldir = @htmldir@|htmldir = /usr/share/doc/bash-3.2|" \
    Makefile.in
patch -Np1 -i ../bash-3.2-fixes-7.patch
./configure --prefix=/usr --bindir=/bin \
    --without-bash-malloc --with-installed-readline
make
sed -i 's/LANG/LC_ALL/' tests/intl.tests
sed -i 's@tests@& </dev/tty@' tests/run-test
chown -Rv nobody ./


su-tools nobody -s /bin/bash -c "make tests"
make install
exec /bin/bash --login +h
