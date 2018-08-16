#
# Open Kernel Attached Operating System (OpenKaOS)
# Platform Build System version 3.0.0
#
# Copyright (c) 2009-2014 Opaque Systems LLC
#
# script : bld-sdk.sh
# purpose: sdk build script 2 of 3, creates SDK chroot from toolchain
#

TOOLS=`cat /.tools`
SRC=/src
LOGS=/src/logs
CFLAGS="-O2 -fPIC -pipe"
CXXFLAGS="$CFLAGS"
MAKEOPTS="-j3"
export SRC TOOLS LOGS CFLAGS CXXFLAGS MAKEOPTS

echo ""
echo "  [*] Building SDK: stage 2 of 3..."

echo "  [.] bc"
cd $SRC/bc
./configure --prefix=/usr --with-readline 1>>$LOGS/bc.log 2>>$LOGS/bc.err
make 1>>$LOGS/bc.log 2>>$LOGS/bc.err
make install 1>>$LOGS/bc.log 2>>$LOGS/bc.err

echo "  [.] libtool"
cd $SRC/libtool
./configure --prefix=/usr 1>>$LOGS/libtool.log 2>>$LOGS/libtool.err
make 1>>$LOGS/libtool.log 2>>$LOGS/libtool.err
make check 1>>$LOGS/libtool.log 2>>$LOGS/libtool.err
make install 1>>$LOGS/libtool.log 2>>$LOGS/libtool.err

echo "  [.] gdbm "
cd $SRC/gdbm
./configure --prefix=/usr --enable-libgdbm-compat 1>>$LOGS/gdbm.log 2>>$LOGS/gdbm.err
make 1>>$LOGS/gdbm.log 2>>$LOGS/gdbm.err
make install 1>>$LOGS/gdbm.log 2>>$LOGS/gdbm.err
make install-compat 1>>$LOGS/gdbm.log 2>>$LOGS/gdbm.err
#install-info --dir-file=/usr/info/dir /usr/info/gdbm.info 1>>$LOGS/gdbm.log 2>>$LOGS/gdbm.err

echo "  [.] inetutils"
cd $SRC/inetutils
echo '#define PATH_PROCNET_DEV "/proc/net/dev"' >> ifconfig/system/linux.h
./configure --prefix=/usr --libexecdir=/usr/sbin \
    --localstatedir=/var --disable-ifconfig \
    --disable-logger --disable-syslogd --disable-whois \
    --disable-servers 1>>$LOGS/inetutils.log 2>>$LOGS/inetutils.err

make 1>>$LOGS/inetutils.log 2>>$LOGS/inetutils.err
make install 1>>$LOGS/inetutils.log 2>>$LOGS/inetutils.err
mv -v /usr/bin/{hostname,ping,ping6,traceroute} /bin
mv -v /usr/bin/ifconfig /sbin

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
make -k test 1>>$LOGS/perl.log 2>>$LOGS/perl.err
make install 1>>$LOGS/perl.log 2>>$LOGS/perl.err

echo "  [.] autoconf"
cd $SRC/autoconf
./configure --prefix=/usr 1>>$LOGS/autoconf.log 2>>$LOGS/autoconf.err
make 1>>$LOGS/autoconf.log 2>>$LOGS/autoconf.err
make check 1>>$LOGS/autoconf.log 2>>$LOGS/autoconf.err
make install 1>>$LOGS/autoconf.log 2>>$LOGS/autoconf.err

echo "  [.] automake"
cd $SRC/automake
./configure --prefix=/usr --docdir=/usr/share/doc/automake-1.14.1 1>>$LOGS/automake.log 2>>$LOGS/automake.err
make 1>>$LOGS/automake.log 2>>$LOGS/automake.err
make install 1>>$LOGS/automake.log 2>>$LOGS/automake.err

echo "  [.] diffutils"
cd $SRC/diffutils
sed -i 's:= @mkdir_p@:= /bin/mkdir -p:' po/Makefile.in.in
./configure --prefix=/usr 1>>$LOGS/diffutils.log 2>>$LOGS/diffutils.err
make 1>>$LOGS/diffutils.log 2>>$LOGS/diffutils.err
make install 1>>$LOGS/diffutils.log 2>>$LOGS/diffutils.err

echo "  [.] gawk"
cd $SRC/gawk
./configure --prefix=/usr --libexecdir=/usr/lib 1>>$LOGS/gawk.log 2>>$LOGS/gawk.err
make 1>>$LOGS/gawk.log 2>>$LOGS/gawk.err
#make check 1>>$LOGS/gawk.log 2>>$LOGS/gawk.err
make install 1>>$LOGS/gawk.log 2>>$LOGS/gawk.err

echo "  [.] findutils "
cd $SRC/findutils
./configure --prefix=/usr --libexecdir=/usr/lib/findutils \
    --localstatedir=/var/lib/locate 1>>$LOGS/findutils.log 2>>$LOGS/findutils.err
make 1>>$LOGS/findutils.log 2>>$LOGS/findutils.err
#make check 1>>$LOGS/findutils.log 2>>$LOGS/findutils.err
make install 1>>$LOGS/findutils.log 2>>$LOGS/findutils.err
mv -v /usr/bin/find /bin 1>>$LOGS/findutils.log 2>>$LOGS/findutils.err
sed -i 's/find:=${BINDIR}/find:=\/bin/' /usr/bin/updatedb 1>>$LOGS/findutils.log 2>>$LOGS/findutils.err

echo "  [.] gettext"
cd $SRC/gettext
./configure --prefix=/usr \
            --docdir=/usr/share/doc/gettext 1>>$LOGS/gettext.log 2>>$LOGS/gettext.err
make 1>>$LOGS/gettext.log 2>>$LOGS/gettext.err
#make check 1>>$LOGS/gettext.log 2>>$LOGS/gettext.err
make install 1>>$LOGS/gettext.log 2>>$LOGS/gettext.err

echo "  [.] groff"
cd $SRC/groff
PAGE=A4 ./configure --prefix=/usr 1>>$LOGS/groff.log 2>>$LOGS/groff.err
make 1>>$LOGS/groff.log 2>>$LOGS/groff.err
make docdir=/usr/share/doc/groff install 1>>$LOGS/groff.log 2>>$LOGS/groff.err
ln -sv eqn /usr/bin/geqn
ln -sv tbl /usr/bin/gtbl

echo "  [.] xz"
cd $SRC/xz
./configure --prefix=/usr --libdir=/lib --docdir=/usr/share/doc/xz 1>>$LOGS/xz.log 2>>$LOGS/xz.err
make 1>>$LOGS/xz.log 2>>$LOGS/xz.err
#make check 1>>$LOGS/xz.log 2>>$LOGS/xz.err
make pkgconfigdir=/usr/lib/pkgconfig install 1>>$LOGS/xz.log 2>>$LOGS/xz.err

echo "  [.] less"
cd $SRC/less
./configure --prefix=/usr --sysconfdir=/etc 1>>$LOGS/less.log 2>>$LOGS/less.err
make 1>>$LOGS/less.log 2>>$LOGS/less.err
make install 1>>$LOGS/less.log 2>>$LOGS/less.err

echo "  [.] gzip"
cd $SRC/gzip
./configure --prefix=/usr --bindir=/bin 1>>$LOGS/gzip.log 2>>$LOGS/gzip.err
make 1>>$LOGS/gzip.log 2>>$LOGS/gzip.err
#make check 1>>$LOGS/gzip.log 2>>$LOGS/gzip.err
make install 1>>$LOGS/gzip.log 2>>$LOGS/gzip.err
mv -v /bin/{gzexe,uncompress,zcmp,zdiff,zegrep} /usr/bin
mv -v /bin/{zfgrep,zforce,zgrep,zless,zmore,znew} /usr/bin

echo "  [.] iproute2"
cd $SRC/iproute2
sed -i '/^TARGETS/s@arpd@@g' misc/Makefile
sed -i /ARPD/d Makefile
sed -i 's/arpd.8//' man/man8/Makefile
make DESTDIR=  1>>$LOGS/iproute2.log 2>>$LOGS/iproute2.err
make DESTDIR= SBINDIR=/sbin MANDIR=/usr/share/man \
     DOCDIR=/usr/share/doc/iproute2 install 1>>$LOGS/iproute2.log 2>>$LOGS/iproute2.err

echo "  [.] kbd"
cd $SRC/kbd
patch -Np1 -i ../patches/kbd-2.0.1-backspace-1.patch
sed -i 's/\(RESIZECONS_PROGS=\)yes/\1no/g' configure
sed -i 's/resizecons.8 //' man/man8/Makefile.in
PKG_CONFIG_PATH=$TOOLS/lib/pkgconfig ./configure --prefix=/usr --disable-vlock 1>>$LOGS/kbd.log 2>>$LOGS/kbd.err
make 1>>$LOGS/kbd.log 2>>$LOGS/kbd.err
make install 1>>$LOGS/kbd.log 2>>$LOGS/kbd.err

echo "  [.] kmod"
cd $SRC/kmod
./configure --prefix=/usr       \
            --bindir=/bin       \
            --libdir=/lib       \
            --sysconfdir=/etc   \
            --disable-manpages  \
            --with-xz           \
            --with-zlib 1>>$LOGS/kmod.log 2>>$LOGS/kmod.err
make 1>>$LOGS/kmod.log 2>>$LOGS/kmod.err
make pkgconfigdir=/usr/lib/pkgconfig install 1>>$LOGS/kmod.log 2>>$LOGS/kmod.err
for target in depmod insmod modinfo modprobe rmmod; do
  ln -sv ../bin/kmod /sbin/$target
done
ln -sv kmod /bin/lsmod

echo "  [.] libpipeline"
cd $SRC/libpipeline
PKG_CONFIG_PATH=$TOOLS/lib/pkgconfig ./configure --prefix=/usr 1>>$LOGS/libpipeline.log 2>>$LOGS/libpipeline.err
make 1>>$LOGS/libpipeline.log 2>>$LOGS/libpipeline.err
make install 1>>$LOGS/libpipeline.log 2>>$LOGS/libpipeline.err

echo "  [.] make"
cd $SRC/make
./configure --prefix=/usr 1>>$LOGS/make.log 2>>$LOGS/make.err
make 1>>$LOGS/make.log 2>>$LOGS/make.err
#make check 1>>$LOGS/make.log 2>>$LOGS/make.err
make install 1>>$LOGS/make.log 2>>$LOGS/make.err

echo "  [.] patch "
cd $SRC/patch
./configure --prefix=/usr 1>>$LOGS/patch.log 2>>$LOGS/patch.err
make 1>>$LOGS/patch.log 2>>$LOGS/patch.err
make install 1>>$LOGS/patch.log 2>>$LOGS/patch.err

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

echo "  [.] tar"
cd $SRC/tar
patch -Np1 -i ../patches/tar-1.27.1-manpage-1.patch
FORCE_UNSAFE_CONFIGURE=1 ./configure --prefix=/usr --bindir=/bin --libexecdir=/usr/sbin 1>>$LOGS/tar.log 2>>$LOGS/tar.err
make 1>>$LOGS/tar.log 2>>$LOGS/tar.err
make check 1>>$LOGS/tar.log 2>>$LOGS/tar.err
make install 1>>$LOGS/tar.log 2>>$LOGS/tar.err
perl tarman > /usr/share/man/man1/tar.1 1>>$LOGS/tar.log 2>>$LOGS/tar.err

echo "  [.] texinfo"
cd $SRC/texinfo
./configure --prefix=/usr 1>>$LOGS/texinfo.log 2>>$LOGS/texinfo.err
make 1>>$LOGS/texinfo.log 2>>$LOGS/texinfo.err
#make check 1>>$LOGS/texinfo.log 2>>$LOGS/texinfo.err
make install 1>>$LOGS/texinfo.log 2>>$LOGS/texinfo.err
make TEXMF=/usr/share/texmf install-tex 1>>$LOGS/texinfo.log 2>>$LOGS/texinfo.err
cd /usr/share/info
rm -v dir
for f in *
do install-info $f dir 2>/dev/null
done

echo "  [.] udev"
cd $SRC/systemd
cp -a $SRC/udev udev-lfs-208-3
ln -sf udev-lfs-208-3 udev
ln -svf $TOOLS/include/blkid /usr/include
ln -svf $TOOLS/include/uuid  /usr/include
make -f udev/Makefile.lfs 1>>$LOGS/udev.log 2>>$LOGS/udev.err
make -f udev/Makefile.lfs install 1>>$LOGS/udev.log 2>>$LOGS/udev.err
build/udevadm hwdb --update 1>>$LOGS/udev.log 2>>$LOGS/udev.err
source udev/init-net-rules.sh 1>>$LOGS/udev.log 2>>$LOGS/udev.err
rm -fv /usr/include/{uuid,blkid}
unset LD_LIBRARY_PATH

echo "  [.] util-linux "
cd $SRC/util-linux
sed -e 's@etc/adjtime@var/lib/hwclock/adjtime@g' \
    -i $(grep -rl '/etc/adjtime' .)
mkdir -pv /var/lib/hwclock
./configure 1>>$LOGS/util-linux.log 2>>$LOGS/util-linux.err
make 1>>$LOGS/util-linux.log 2>>$LOGS/util-linux.err
make install 1>>$LOGS/util-linux.log 2>>$LOGS/util-linux.err

echo "  [.] man-db"
cd $SRC/man-db
./configure --prefix=/usr --libexecdir=/usr/lib \
    --docdir=/usr/share/doc/man-db-2.6.6 \
    --sysconfdir=/etc --disable-setuid \
    --with-browser=/usr/bin/lynx --with-vgrind=/usr/bin/vgrind \
    --with-grap=/usr/bin/grap 1>>$LOGS/mandb.log 2>>$LOGS/mandb.err

make 1>>$LOGS/mandb.log 2>>$LOGS/mandb.err
#make check 1>>$LOGS/mandb.log 2>>$LOGS/mandb.err
make install 1>>$LOGS/mandb.log 2>>$LOGS/mandb.err

exit

