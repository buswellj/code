#!/bin/bash
ACV="4.1.0"
ARC=".tar.bz2"
APN="shadow"
export ACV ARC APN
ACB=$APN-$ACV
export ACB
###################################
#
#   Shadow-4.1.0
#
###################################
cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB
./configure --libdir=/lib --sysconfdir=/etc --enable-shared --without-selinux
sed 's/groups$(EXEEXT) //' -i.orig src/Makefile
sed -e '/groups.1.xml/d' -e 's/groups.1//' -i.orig man/Makefile
sed -e 's@#MD5_CRYPT_ENAB.no@MD5_CRYPT_ENAB yes@' \
    -e 's@/var/spool/mail@/var/mail@' -i.orig etc/login.defs
make
make install
mv -v /usr/bin/passwd /bin
mv -v /lib/libshadow.*a /usr/lib
rm -v /lib/libshadow.so
ln -vsf ../../lib/libshadow.so.0 /usr/lib/libshadow.so
pwconv
grpconv
groupadd -g 100 users
useradd -D -g 100
useradd -D -b /home
sed 's/CREATE_MAIL_SPOOL.*/CREATE_MAIL_SPOOL=no/' -i /etc/default/useradd
passwd root

