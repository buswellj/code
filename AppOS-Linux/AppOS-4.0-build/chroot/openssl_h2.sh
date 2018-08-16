#!/bin/bash
ACV="0.9.8g"
ARC=".tar.gz"
APN="openssl"
export ACV ARC APN
ACB=$APN-$ACV
export ACB
###################################
# 
#  OpenSSL-0.9.8g
#
###################################
cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB
$PTC $LSP/openssl-0.9.8g-fix_manpages-1.patch
sed -e 's/__OpenBSD__/__linux__/' -e 's/arandom/urandom/' \
    -i.orig crypto/rand/randfile.c
sed 's/__OpenBSD__/__linux__/' -i.orig crypto/uid.c
sed 's/__OpenBSD__/__linux__/' -i.orig crypto/rand/rand_unix.c
find crypto/ -name Makefile -exec \
    sed 's/^ASFLAGS=/&-Wa,--noexecstack /' -i.orig {} \;
./config --openssldir=/etc/ssl --prefix=/usr shared zlib-dynamic \
    -DSSL_FORBID_ENULL
make MANDIR=/usr/share/man
make MANDIR=/usr/share/man install
cp -v -r certs /etc/ssl
install -v -d -m755 /usr/share/doc/openssl-0.9.8g
cp -v -r doc/{HOWTO,README,*.{txt,html,gif}} \
    /usr/share/doc/openssl-0.9.8g
mv -v /usr/lib/{libcrypto,libssl}.a /usr/lib/static
