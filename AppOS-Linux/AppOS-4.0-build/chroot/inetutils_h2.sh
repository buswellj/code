#!/bin/bash
ACV="1.5"
ARC=".tar.gz"
APN="inetutils"
export ACV ARC APN
ACB=$APN-$ACV
export ACB
###################################
#
#     Inetutils-1.5
#     
###################################
cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB
find . -name Makefile.in -exec sed -e '/ftpd.8/d' \
    -e '/inetd.8/d' -e '/logger.1/d' -e '/rexecd.8/d' -e '/rlogind.8/d' \
    -e '/rshd.8/d' -e '/syslog.conf.5 syslogd.8/d' -e '/talkd.8/d' \
    -e '/telnetd.8/d' -e '/tftpd.8/d' -e '/rcp.1/d' -e '/rlogin.1/d' \
    -e '/rsh.1/d' -i.orig {} \;
sed -e 's/^CFLAGS =/& -fmudflap/' \
    -e 's/^LIBS =/& -lmudflap/' -i.orig ping/Makefile.in
sed 's/4775/4755/' -i.orig2 ping/Makefile.in
./configure --prefix=/usr --libexecdir=/usr/sbin \
    --sysconfdir=/etc --localstatedir=/var \
    --disable-logger --disable-syslogd \
    --disable-whois --disable-servers \
    --disable-rcp --disable-rlogin --disable-rsh
make
make install
mv -v /usr/bin/ping{,6} /bin
