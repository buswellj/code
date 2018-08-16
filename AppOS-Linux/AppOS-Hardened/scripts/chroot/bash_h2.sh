#!/bin/bash
ACV="3.2"
ARC=".tar.bz2"
APN="bash"
export ACV ARC APN
ACB=$APN-$ACV
export ACB
###################################
#
#    Bash-3.2
#
###################################
cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB
$PTC $LSP/bash-3.2-fixes-7.patch
$PTC $LSP/bash-3.2-arc4random-1.patch
./configure --prefix=/usr --bindir=/bin \
    --without-bash-malloc --with-installed-readline
make
sed 's/LANG/LC_ALL/' -i.orig tests/intl.tests
sed 's@tests@& </dev/tty@' -i.orig tests/run-test
chown -Rv nobody ./
su-tools nobody -s /bin/bash -c "make tests"
make install
exec /bin/bash --login +h
