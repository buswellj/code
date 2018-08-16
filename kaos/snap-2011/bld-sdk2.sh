#
# Kernel Attached Operating System (KaOS)
# Platform Build System version 1.0.0
#
# Copyright (c) 2009-2011 Carbon Mountain LLC
#
# script : bld-sdk.sh
# purpose: sdk build script 2 of 3, creates SDK chroot from toolchain
#

TOOLS=`cat /.tools`
SRC=/src
LOGS=/src/logs
export SRC TOOLS LOGS

echo ""
echo "  [*] Building SDK: stage 2 of 3..."

echo "  [.] libtool"
cd $SRC/libtool
./configure --prefix=/usr 1>>$LOGS/libtool.log 2>>$LOGS/libtool.err
make 1>>$LOGS/libtool.log 2>>$LOGS/libtool.err
make check 1>>$LOGS/libtool.log 2>>$LOGS/libtool.err
make install 1>>$LOGS/libtool.log 2>>$LOGS/libtool.err

echo "  [.] gdbm "
cd $SRC/gdbm
./configure --prefix=/usr 1>>$LOGS/gdbm.log 2>>$LOGS/gdbm.err
make 1>>$LOGS/gdbm.log 2>>$LOGS/gdbm.err
make install 1>>$LOGS/gdbm.log 2>>$LOGS/gdbm.err
make install-compat 1>>$LOGS/gdbm.log 2>>$LOGS/gdbm.err
install-info --dir-file=/usr/info/dir /usr/info/gdbm.info 1>>$LOGS/gdbm.log 2>>$LOGS/gdbm.err

echo "  [.] inetutils"
cd $SRC/inetutils
./configure --prefix=/usr --libexecdir=/usr/sbin \
    --localstatedir=/var --disable-ifconfig \
    --disable-logger --disable-syslogd --disable-whois \
    --disable-servers 1>>$LOGS/inetutils.log 2>>$LOGS/inetutils.err

make 1>>$LOGS/inetutils.log 2>>$LOGS/inetutils.err
make install 1>>$LOGS/inetutils.log 2>>$LOGS/inetutils.err
mv -v /usr/bin/{hostname,ping,ping6} /bin
mv -v /usr/bin/traceroute /sbin

echo "  [.] perl"
cd $SRC/perl
echo "127.0.0.1 localhost $(hostname)" > /etc/hosts
sed -i -e "s|BUILD_ZLIB\s*= True|BUILD_ZLIB = False|"           \
       -e "s|INCLUDE\s*= ./zlib-src|INCLUDE    = /usr/include|" \
       -e "s|LIB\s*= ./zlib-src|LIB        = /usr/lib|"         \
    cpan/Compress-Raw-Zlib/config.in
sh Configure -des -Dprefix=/usr \
                  -Dvendorprefix=/usr           \
                  -Dman1dir=/usr/share/man/man1 \
                  -Dman3dir=/usr/share/man/man3 \
                  -Dpager="/usr/bin/less -isR" \
                  -Duseshrplib 1>>$LOGS/perl.log 2>>$LOGS/perl.err

make 1>>$LOGS/perl.log 2>>$LOGS/perl.err
make test 1>>$LOGS/perl.log 2>>$LOGS/perl.err
make install 1>>$LOGS/perl.log 2>>$LOGS/perl.err

echo "  [.] autoconf"
cd $SRC/autoconf
./configure --prefix=/usr 1>>$LOGS/autoconf.log 2>>$LOGS/autoconf.err
make 1>>$LOGS/autoconf.log 2>>$LOGS/autoconf.err
make check 1>>$LOGS/autoconf.log 2>>$LOGS/autoconf.err
make install 1>>$LOGS/autoconf.log 2>>$LOGS/autoconf.err

echo "  [.] automake"
cd $SRC/automake
./configure --prefix=/usr --docdir=/usr/share/doc/automake-1.11.1 1>>$LOGS/automake.log 2>>$LOGS/automake.err
make 1>>$LOGS/automake.log 2>>$LOGS/automake.err
make check 1>>$LOGS/automake.log 2>>$LOGS/automake.err
make install 1>>$LOGS/automake.log 2>>$LOGS/automake.err

## bzip2 was moved from here to bld-sdk.sh

echo "  [.] diffutils"
cd $SRC/diffutils
./configure --prefix=/usr 1>>$LOGS/diffutils.log 2>>$LOGS/diffutils.err
make 1>>$LOGS/diffutils.log 2>>$LOGS/diffutils.err
make install 1>>$LOGS/diffutils.log 2>>$LOGS/diffutils.err

## file was moved from here to bld-sdk.sh

echo "  [.] gawk"
cd $SRC/gawk
./configure --prefix=/usr --libexecdir=/usr/lib 1>>$LOGS/gawk.log 2>>$LOGS/gawk.err
make 1>>$LOGS/gawk.log 2>>$LOGS/gawk.err
make check 1>>$LOGS/gawk.log 2>>$LOGS/gawk.err
make install 1>>$LOGS/gawk.log 2>>$LOGS/gawk.err

echo "  [.] findutils "
cd $SRC/findutils
./configure --prefix=/usr --libexecdir=/usr/lib/findutils \
    --localstatedir=/var/lib/locate 1>>$LOGS/findutils.log 2>>$LOGS/findutils.err
make 1>>$LOGS/findutils.log 2>>$LOGS/findutils.err
make check 1>>$LOGS/findutils.log 2>>$LOGS/findutils.err
make install 1>>$LOGS/findutils.log 2>>$LOGS/findutils.err
mv -v /usr/bin/find /bin 1>>$LOGS/findutils.log 2>>$LOGS/findutils.err
sed -i 's/find:=${BINDIR}/find:=\/bin/' /usr/bin/updatedb 1>>$LOGS/findutils.log 2>>$LOGS/findutils.err

echo "  [.] flex"
cd $SRC/flex
patch -Np1 -i ../patches/flex-2.5.35-gcc44-1.patch
./configure --prefix=/usr 1>>$LOGS/flex.log 2>>$LOGS/flex.err
make 1>>$LOGS/flex.log 2>>$LOGS/flex.err
make check 1>>$LOGS/flex.log 2>>$LOGS/flex.err
make install 1>>$LOGS/flex.log 2>>$LOGS/flex.err
ln -sv libfl.a /usr/lib/libl.a 1>>$LOGS/flex.log 2>>$LOGS/flex.err
cat > /usr/bin/lex << "EOF"
#!/bin/sh
# Begin /usr/bin/lex

exec /usr/bin/flex -l "$@"

# End /usr/bin/lex
EOF
chmod -v 755 /usr/bin/lex 1>>$LOGS/flex.log 2>>$LOGS/flex.err

echo "  [.] gettext"
cd $SRC/gettext
./configure --prefix=/usr \
            --docdir=/usr/share/doc/gettext-0.18.1.1 1>>$LOGS/gettext.log 2>>$LOGS/gettext.err
make 1>>$LOGS/gettext.log 2>>$LOGS/gettext.err
make check 1>>$LOGS/gettext.log 2>>$LOGS/gettext.err
make install 1>>$LOGS/gettext.log 2>>$LOGS/gettext.err

echo "  [.] groff"
cd $SRC/groff
PAGE=A4 ./configure --prefix=/usr 1>>$LOGS/groff.log 2>>$LOGS/groff.err
make 1>>$LOGS/groff.log 2>>$LOGS/groff.err
make docdir=/usr/share/doc/groff-1.20.1 install 1>>$LOGS/groff.log 2>>$LOGS/groff.err
ln -sv eqn /usr/bin/geqn
ln -sv tbl /usr/bin/gtbl

echo "  [.] gzip"
cd $SRC/gzip
./configure --prefix=/usr --bindir=/bin 1>>$LOGS/gzip.log 2>>$LOGS/gzip.err
make 1>>$LOGS/gzip.log 2>>$LOGS/gzip.err
make check 1>>$LOGS/gzip.log 2>>$LOGS/gzip.err
make install 1>>$LOGS/gzip.log 2>>$LOGS/gzip.err
mv -v /bin/{gzexe,uncompress,zcmp,zdiff,zegrep} /usr/bin
mv -v /bin/{zfgrep,zforce,zgrep,zless,zmore,znew} /usr/bin

echo "  [.] iproute2"
cd $SRC/iproute2
sed -i '/^TARGETS/s@arpd@@g' misc/Makefile
sed -i '1289i\\tfilter.cloned = 2;' ip/iproute.c
make DESTDIR=  1>>$LOGS/iproute2.log 2>>$LOGS/iproute2.err
make DESTDIR= SBINDIR=/sbin MANDIR=/usr/share/man \
     DOCDIR=/usr/share/doc/iproute2-2.6.37 install 1>>$LOGS/iproute2.log 2>>$LOGS/iproute2.err

echo "  [.] kbd"
cd $SRC/kbd
patch -Np1 -i ../patches/kbd-1.15.2-backspace-1.patch
./configure --prefix=/usr --datadir=/lib/kbd 1>>$LOGS/kbd.log 2>>$LOGS/kbd.err
make 1>>$LOGS/kbd.log 2>>$LOGS/kbd.err
make install 1>>$LOGS/kbd.log 2>>$LOGS/kbd.err
mv -v /usr/bin/{kbd_mode,loadkeys,openvt,setfont} /bin

echo "  [.] less"
cd $SRC/less
./configure --prefix=/usr --sysconfdir=/etc 1>>$LOGS/less.log 2>>$LOGS/less.err
make 1>>$LOGS/less.log 2>>$LOGS/less.err
make install 1>>$LOGS/less.log 2>>$LOGS/less.err

echo "  [.] libpipeline"
cd $SRC/libpipeline
./configure --prefix=/usr 1>>$LOGS/libpipeline.log 2>>$LOGS/libpipeline.err
make 1>>$LOGS/libpipeline.log 2>>$LOGS/libpipeline.err
make install 1>>$LOGS/libpipeline.log 2>>$LOGS/libpipeline.err

echo "  [.] make"
cd $SRC/make
./configure --prefix=/usr 1>>$LOGS/make.log 2>>$LOGS/make.err
make 1>>$LOGS/make.log 2>>$LOGS/make.err
make check 1>>$LOGS/make.log 2>>$LOGS/make.err
make install 1>>$LOGS/make.log 2>>$LOGS/make.err

echo "  [.] xz"
cd $SRC/xz
./configure --prefix=/usr --docdir=/usr/share/doc/xz-5.0.1 1>>$LOGS/xz.log 2>>$LOGS/xz.err
make 1>>$LOGS/xz.log 2>>$LOGS/xz.err
make check 1>>$LOGS/xz.log 2>>$LOGS/xz.err
make install 1>>$LOGS/xz.log 2>>$LOGS/xz.err

echo "  [.] man-db"
cd $SRC/man-db
./configure --prefix=/usr --libexecdir=/usr/lib \
    --docdir=/usr/share/doc/man-db-2.6.0.2 \
    --sysconfdir=/etc --disable-setuid \
    --with-browser=/usr/bin/lynx --with-vgrind=/usr/bin/vgrind \
    --with-grap=/usr/bin/grap 1>>$LOGS/mandb.log 2>>$LOGS/mandb.err

make 1>>$LOGS/mandb.log 2>>$LOGS/mandb.err
make check 1>>$LOGS/mandb.log 2>>$LOGS/mandb.err
make install 1>>$LOGS/mandb.log 2>>$LOGS/mandb.err

echo "  [.] module-init-tools"
cd $SRC/module-init-tools
echo '.so man5/modprobe.conf.5' > modprobe.d.5
./configure 1>>$LOGS/module-init-tools.log 2>>$LOGS/module-init-tools.err
make check 1>>$LOGS/module-init-tools.log 2>>$LOGS/module-init-tools.err
./tests/runtests 1>>$LOGS/module-init-tools.log 2>>$LOGS/module-init-tools.err
make clean 1>>$LOGS/module-init-tools.log 2>>$LOGS/module-init-tools.err
./configure --prefix=/ --enable-zlib-dynamic --mandir=/usr/share/man 1>>$LOGS/module-init-tools.log 2>>$LOGS/module-init-tools.err
make 1>>$LOGS/module-init-tools.log 2>>$LOGS/module-init-tools.err
make INSTALL=install install 1>>$LOGS/module-init-tools.log 2>>$LOGS/module-init-tools.err

echo "  [.] patch "
cd $SRC/patch
patch -Np1 -i ../patches/patch-2.6.1-test_fix-1.patch
./configure --prefix=/usr 1>>$LOGS/patch.log 2>>$LOGS/patch.err
make 1>>$LOGS/patch.log 2>>$LOGS/patch.err
make install 1>>$LOGS/patch.log 2>>$LOGS/patch.err

echo "  [.] psmisc "
cd $SRC/psmisc
./configure --prefix=/usr 1>>$LOGS/psmisc.log 2>>$LOGS/psmisc.err
make 1>>$LOGS/psmisc.log 2>>$LOGS/psmisc.err
make install 1>>$LOGS/psmisc.log 2>>$LOGS/psmisc.err
mv -v /usr/bin/fuser /bin 1>>$LOGS/psmisc.log 2>>$LOGS/psmisc.err
mv -v /usr/bin/killall /bin 1>>$LOGS/psmisc.log 2>>$LOGS/psmisc.err

echo "  [.] cracklib"
cd $SRC/cracklib
./configure --prefix=/usr \
            --with-default-dict=/lib/cracklib/pw_dict 1>>$LOGS/cracklib.log 2>>$LOGS/cracklib.err
make 1>>$LOGS/cracklib.log 2>>$LOGS/cracklib.err
make install 1>>$LOGS/cracklib.log 2>>$LOGS/cracklib.err
mv -v /usr/lib/libcrack.so.2* /lib 1>>$LOGS/cracklib.log 2>>$LOGS/cracklib.err
ln -v -sf ../../lib/libcrack.so.2.8.1 /usr/lib/libcrack.so 1>>$LOGS/cracklib.log 2>>$LOGS/cracklib.err
install -v -m644 -D ./cracklib-words-20080507.gz \
    /usr/share/dict/cracklib-words.gz 1>>$LOGS/cracklib.log 2>>$LOGS/cracklib.err
gunzip -v /usr/share/dict/cracklib-words.gz 1>>$LOGS/cracklib.log 2>>$LOGS/cracklib.err
ln -v -s cracklib-words /usr/share/dict/words 1>>$LOGS/cracklib.log 2>>$LOGS/cracklib.err
echo $(hostname) >>/usr/share/dict/cracklib-extra-words 1>>$LOGS/cracklib.log 2>>$LOGS/cracklib.err
install -v -m755 -d /lib/cracklib 1>>$LOGS/cracklib.log 2>>$LOGS/cracklib.err
create-cracklib-dict /usr/share/dict/cracklib-words \
                     /usr/share/dict/cracklib-extra-words 1>>$LOGS/cracklib.log 2>>$LOGS/cracklib.err
make test 1>>$LOGS/cracklib.log 2>>$LOGS/cracklib.err

echo "  [.] shadow"
cd $SRC/shadow
sed -i 's/groups$(EXEEXT) //' src/Makefile.in
find man -name Makefile.in -exec sed -i 's/groups\.1 / /' {} \;
sed -i 's/man_MANS = $(man_nopam) /man_MANS = /' man/ru/Makefile.in
sed -i -e 's@#ENCRYPT_METHOD DES@ENCRYPT_METHOD SHA512@' \
       -e 's@/var/spool/mail@/var/mail@' etc/login.defs
sed -i 's@DICTPATH.*@DICTPATH\t/lib/cracklib/pw_dict@' \
    etc/login.defs
./configure --sysconfdir=/etc --with-libcrack 1>>$LOGS/shadow.log 2>>$LOGS/shadow.err
make 1>>$LOGS/shadow.log 2>>$LOGS/shadow.err
make install 1>>$LOGS/shadow.log 2>>$LOGS/shadow.err
mv -v /usr/bin/passwd /bin

pwconv 1>>$LOGS/shadow.log 2>>$LOGS/shadow.err
grpconv 1>>$LOGS/shadow.log 2>>$LOGS/shadow.err

echo "  [.] sysklogd"
cd $SRC/sysklogd
make 1>>$LOGS/sysklogd.log 2>>$LOGS/sysklogd.err
make BINDIR=/sbin install 1>>$LOGS/sysklogd.log 2>>$LOGS/sysklogd.err
cat > /etc/syslog.conf << "EOF"
# Begin /etc/syslog.conf

auth,authpriv.* -/var/log/auth.log
*.*;auth,authpriv.none -/var/log/sys.log
daemon.* -/var/log/daemon.log
kern.* -/var/log/kern.log
mail.* -/var/log/mail.log
user.* -/var/log/user.log
*.emerg *

# End /etc/syslog.conf
EOF

## This is disabled on purpose
#echo "  [.] sysvinit"
#cd $SRC/sysvinit
#make -C src 1>>$LOGS/sysvinit.log 2>>$LOGS/sysvinit.err
#make -C src install 1>>$LOGS/sysvinit.log 2>>$LOGS/sysvinit.err
#
#cat > /etc/inittab << "EOF"
# Begin /etc/inittab
#
#id:3:initdefault:
#
#si::sysinit:/etc/rc.d/init.d/rc sysinit
#
#l0:0:wait:/etc/rc.d/init.d/rc 0
#l1:S1:wait:/etc/rc.d/init.d/rc 1
#l2:2:wait:/etc/rc.d/init.d/rc 2
#l3:3:wait:/etc/rc.d/init.d/rc 3
#l4:4:wait:/etc/rc.d/init.d/rc 4
#l5:5:wait:/etc/rc.d/init.d/rc 5
#l6:6:wait:/etc/rc.d/init.d/rc 6
#
#ca:12345:ctrlaltdel:/sbin/shutdown -t1 -a -r now
#
#su:S016:once:/sbin/sulogin
#
#1:2345:respawn:/sbin/agetty tty1 9600
#2:2345:respawn:/sbin/agetty tty2 9600
#3:2345:respawn:/sbin/agetty tty3 9600
#4:2345:respawn:/sbin/agetty tty4 9600
#5:2345:respawn:/sbin/agetty tty5 9600
#6:2345:respawn:/sbin/agetty tty6 9600
#
## End /etc/inittab
#EOF

echo "  [.] tar"
cd $SRC/tar
FORCE_UNSAFE_CONFIGURE=1 ./configure --prefix=/usr --bindir=/bin --libexecdir=/usr/sbin 1>>$LOGS/tar.log 2>>$LOGS/tar.err
make 1>>$LOGS/tar.log 2>>$LOGS/tar.err
make check 1>>$LOGS/tar.log 2>>$LOGS/tar.err
make install 1>>$LOGS/tar.log 2>>$LOGS/tar.err

echo "  [.] texinfo"
cd $SRC/texinfo
./configure --prefix=/usr 1>>$LOGS/texinfo.log 2>>$LOGS/texinfo.err
make 1>>$LOGS/texinfo.log 2>>$LOGS/texinfo.err
make check 1>>$LOGS/texinfo.log 2>>$LOGS/texinfo.err
make install 1>>$LOGS/texinfo.log 2>>$LOGS/texinfo.err
make TEXMF=/usr/share/texmf install-tex 1>>$LOGS/texinfo.log 2>>$LOGS/texinfo.err
cd /usr/share/info
rm -v dir
for f in *
do install-info $f dir 2>/dev/null
done

exit

