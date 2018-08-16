#!/bin/bash
ACV="4.0.18.1"
ARC=".tar.bz2"
APN="Shadow"
export ACV ARC APN
ACB=$APN-$ACV
export ACB
#
# Shadow-4.0.18.1
#
##########################################
cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB
patch -Np1 -i ../shadow-4.0.18.1-useradd_fix-2.patch
./configure --libdir=/lib --sysconfdir=/etc --enable-shared \
    --without-selinux
sed -i 's/groups$(EXEEXT) //' src/Makefile
find man -name Makefile -exec sed -i 's/groups\.1 / /' {} \;


sed -i -e 's/ ko//' -e 's/ zh_CN zh_TW//' man/Makefile
sed -i -e 's@#MD5_CRYPT_ENAB.no@MD5_CRYPT_ENAB yes@' \
    -e 's@/var/spool/mail@/var/mail@' etc/login.defs
make
make install
mv -v /usr/bin/passwd /bin
mv -v /lib/libshadow.*a /usr/lib
rm -v /lib/libshadow.so
ln -sfv ../../lib/libshadow.so.0 /usr/lib/libshadow.so
pwconv
grpconv
useradd -D -b /home
sed -i 's/yes/no/' /etc/default/useradd
passwd root
