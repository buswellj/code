#
# Kernel Attached Operating System (KaOS)
# Platform Build System version 1.0.0
#
# Copyright (c) 2009-2011 Carbon Mountain LLC
#
# script : bld-sdk.sh
# purpose: sdk build script 1 of 3, creates SDK chroot from toolchain
#

TOOLS=`cat /.tools`
SRC=/src
LOGS=/src/logs
export SRC TOOLS LOGS
mkdir -p $LOGS
cd /

echo "  [*] Building SDK: stage 1 of 3..."
echo ""
echo "  [.] creating directory structure"
mkdir -pv /{bin,boot,etc/opt,home,lib,mnt,opt}
mkdir -pv /{media/{floppy,cdrom},sbin,srv,var}
install -dv -m 0750 /root
install -dv -m 1777 /tmp /var/tmp
mkdir -pv /usr/{,local/}{bin,include,lib,sbin,src}
mkdir -pv /usr/{,local/}share/{doc,info,locale,man}
mkdir -v  /usr/{,local/}share/{misc,terminfo,zoneinfo}
mkdir -pv /usr/{,local/}share/man/man{1..8}
for dir in /usr /usr/local; do
  ln -sv share/{man,doc,info} $dir
done
case $(uname -m) in
 x86_64) ln -sv lib /lib64 && ln -sv lib /usr/lib64 ;;
esac
mkdir -v /var/{lock,log,mail,run,spool}
mkdir -pv /var/{opt,cache,lib/{misc,locate},local}

echo ""
echo "  [.] Initial File Setup"
echo ""

ln -sv $TOOLS/bin/{bash,cat,echo,pwd,stty} /bin
ln -sv $TOOLS/bin/perl /usr/bin
ln -sv $TOOLS/lib/libgcc_s.so{,.1} /usr/lib
ln -sv $TOOLS/lib/libstdc++.so{,.6} /usr/lib
ln -sv bash /bin/sh
touch /etc/mtab
cat > /etc/passwd << "EOF"
root:x:0:0:root:/root:/bin/bash
bin:x:1:1:bin:/dev/null:/bin/false
nobody:x:99:99:Unprivileged User:/dev/null:/bin/false
EOF

cat > /etc/group << "EOF"
root:x:0:
bin:x:1:
sys:x:2:
kmem:x:3:
tty:x:4:
tape:x:5:
daemon:x:6:
floppy:x:7:
disk:x:8:
lp:x:9:
dialout:x:10:
audio:x:11:
video:x:12:
utmp:x:13:
usb:x:14:
cdrom:x:15:
mail:x:34:
nogroup:x:99:
EOF

touch /var/run/utmp /var/log/{btmp,lastlog,wtmp}
chgrp -v utmp /var/run/utmp /var/log/lastlog
chmod -v 664 /var/run/utmp /var/log/lastlog

echo "  [.] Linux API Headers"
cd $SRC/linux
make mrproper 1>>$LOGS/linux.log 2>>$LOGS/linux.err
make headers_check 1>>$LOGS/linux.log 2>>$LOGS/linux.err
make INSTALL_HDR_PATH=dest headers_install 1>>$LOGS/linux.log 2>>$LOGS/linux.err
find dest/include \( -name .install -o -name ..install.cmd \) -delete 1>>$LOGS/linux.log 2>>$LOGS/linux.err
cp -rv dest/include/* /usr/include 1>>$LOGS/linux.log 2>>$LOGS/linux.err

echo "  [.] Man pages"
cd $SRC/man-pages
make install 1>>$LOGS/man-pages.log 2>>$LOGS/man-pages.err

#echo "  [.] gperf"
#cd $SRC/gperf
#./configure --prefix=/usr 1>>$LOGS/gperf.log 2>>$LOGS/gperf.err
#make 1>>$LOGS/gperf.log 2>>$LOGS/gperf.err
#make install 1>>$LOGS/gperf.log 2>>$LOGS/gperf.err

echo "  [.] glibc "
cd $SRC/glibc
echo "#!/bin/bash" > chroot-adj2.sh
echo "DL=\$(readelf -l /bin/sh | sed -n 's@.*interpret.*$TOOLS\\(.*\\)]\$@\\1@p')" >> chroot-adj2.sh
echo "sed -i \"s|libs -o|libs -L/usr/lib -Wl,-dynamic-linker=\$DL -o|\" \\" >> chroot-adj2.sh
echo "       scripts/test-installation.pl" >> chroot-adj2.sh
echo "unset DL" >> chroot-adj2.sh
chmod 755 chroot-adj2.sh
./chroot-adj2.sh
sed -i -e 's/"db1"/& \&\& $name ne "nss_test1"/' scripts/test-installation.pl
sed -i 's|@BASH@|/bin/bash|' elf/ldd.bash.in
patch -Np1 -i ../patches/glibc-2.13-gcc_fix-1.patch
sed -i '195,213 s/PRIVATE_FUTEX/FUTEX_CLOCK_REALTIME/' \
 nptl/sysdeps/unix/sysv/linux/x86_64/pthread_rwlock_timed{rd,wr}lock.S
mkdir -v ../glibc-build
cd ../glibc-build
case `uname -m` in
  i?86) echo "CFLAGS += -march=i486 -mtune=native -O3 -pipe" > configparms ;;
esac
../glibc/configure --prefix=/usr \
    --disable-profile --enable-add-ons \
    --enable-kernel=2.6.22.5 --libexecdir=/usr/lib/glibc 1>>$LOGS/glibc.log 2>>$LOGS/glibc.err
make 1>>$LOGS/glibc.log 2>>$LOGS/glibc.err
cp -v ../glibc/iconvdata/gconv-modules iconvdata
make -k check 2>&1 | tee glibc-check-log
grep Error glibc-check-log 1>>$LOGS/glibc.log 2>>$LOGS/glibc.err
touch /etc/ld.so.conf
make install 1>>$LOGS/glibc.log 2>>$LOGS/glibc.err
make localedata/install-locales 1>>$LOGS/glibc.log 2>>$LOGS/glibc.err
cp glibc-check-log $LOGS

cat > /etc/nsswitch.conf << "EOF"
# Begin /etc/nsswitch.conf

passwd: files
group: files
shadow: files

hosts: files dns
networks: files

protocols: files
services: files
ethers: files
rpc: files
# End /etc/nsswitch.conf
EOF

###
### FIXME: automate tzselect here
###
### cp -v --remove-destination /usr/share/zoneinfo/<xxx> \
###    /etc/localtime
###

cat > /etc/ld.so.conf << "EOF"
# Begin /etc/ld.so.conf
/usr/local/lib
/opt/lib

# End /etc/ld.so.conf
EOF

cat >> /etc/ld.so.conf << "EOF"
# Add an include directory
include /etc/ld.so.conf.d/*.conf

EOF
mkdir /etc/ld.so.conf.d

echo "  [.] Adjusting Toolchain"
cd $SRC/
mv -v $TOOLS/bin/{ld,ld-old}
mv -v $TOOLS/$(gcc -dumpmachine)/bin/{ld,ld-old}
mv -v $TOOLS/bin/{ld-new,ld}
ln -sv $TOOLS/bin/ld $TOOLS/$(gcc -dumpmachine)/bin/ld

echo "#!/bin/bash" > chroot-adj3.sh
echo "gcc -dumpspecs | sed -e 's@$TOOLS@@g' \\" >> chroot-adj3.sh
echo "    -e '/\\*startfile_prefix_spec:/{n;s@.*@/usr/lib/ @}' \\" >> chroot-adj3.sh
echo "    -e '/\\*cpp:/{n;s@\$@ -isystem /usr/include@}' > \\" >> chroot-adj3.sh
echo "    \`dirname \$(gcc --print-libgcc-file-name)\`/specs " >> chroot-adj3.sh

chmod 755 chroot-adj3.sh
./chroot-adj3.sh

echo 'main(){}' > dummy.c
cc dummy.c -v -Wl,--verbose &> dummy.log
readelf -l a.out | grep ': /lib'

grep -o '/usr/lib.*/crt[1in].*succeeded' dummy.log
grep -B1 '^ /usr/include' dummy.log
grep 'SEARCH.*/usr/lib' dummy.log |sed 's|; |\n|g'
grep "/lib.*/libc.so.6 " dummy.log
grep found dummy.log

echo "  [.] zlib"
cd $SRC/zlib
sed -i 's/ifdef _LARGEFILE64_SOURCE/ifndef _LARGEFILE64_SOURCE/' zlib.h
CFLAGS='-mstackrealign -fPIC -O3' ./configure --prefix=/usr --shared --libdir=/lib 1>>$LOGS/zlib.log 2>>$LOGS/zlib.err
make 1>>$LOGS/zlib.log 2>>$LOGS/zlib.err
make check 1>>$LOGS/zlib.log 2>>$LOGS/zlib.err
make install 1>>$LOGS/zlib.log 2>>$LOGS/zlib.err
mv -v /usr/lib/libz.so.* /lib
ln -sfv ../../lib/libz.so.1.2.5 /usr/lib/libz.so

echo "  [.] file"
cd $SRC/file
./configure --prefix=/usr 1>>$LOGS/file.log 2>>$LOGS/file.err
make 1>>$LOGS/file.log 2>>$LOGS/file.err
make check 1>>$LOGS/file.log 2>>$LOGS/file.err
make install 1>>$LOGS/file.log 2>>$LOGS/file.err

echo "  [.] binutils"
cd $SRC/binutils
expect -c "spawn ls" 1>>$LOGS/binutils.log 2>>$LOGS/binutils.err
rm -fv etc/standards.info
sed -i.bak '/^INFO/s/standards.info //' etc/Makefile.in
sed -i "/exception_defines.h/d" ld/testsuite/ld-elf/new.cc
sed -i "s/-fvtable-gc //" ld/testsuite/ld-selective/selective.exp
mkdir -v ../binutils-build
cd ../binutils-build
../binutils/configure --prefix=/usr --enable-shared 1>>$LOGS/binutils.log 2>>$LOGS/binutils.err
make tooldir=/usr 1>>$LOGS/binutils.log 2>>$LOGS/binutils.err
make check 1>>$LOGS/binutils.log 2>>$LOGS/binutils.err
make tooldir=/usr install 1>>$LOGS/binutils.log 2>>$LOGS/binutils.err
cp -v ../binutils/include/libiberty.h /usr/include

echo "  [.] gmp"
cd $SRC/gmp
./configure --prefix=/usr --enable-cxx --enable-mpbsd 1>>$LOGS/gmp.log 2>>$LOGS/gmp.err
make 1>>$LOGS/gmp.log 2>>$LOGS/gmp.err
make check 2>&1 | tee gmp-check-logmake check 2>&1 | tee gmp-check-log
awk '/tests passed/{total+=$2} ; END{print total}' gmp-check-log 1>>$LOGS/gmp.log 2>>$LOGS/gmp.err
make install 1>>$LOGS/gmp.log 2>>$LOGS/gmp.err

echo "  [.] mpfr"
cd $SRC/mpfr
./configure --prefix=/usr --enable-thread-safe 1>>$LOGS/mpfr.log 2>>$LOGS/mpfr.err
make 1>>$LOGS/mpfr.log 2>>$LOGS/mpfr.err
make check 1>>$LOGS/mpfr.log 2>>$LOGS/mpfr.err
make install 1>>$LOGS/mpfr.log 2>>$LOGS/mpfr.err

echo "  [.] mpc"
cd $SRC/mpc
./configure --prefix=/usr 1>>$LOGS/mpc.log 2>>$LOGS/mpc.err
make 1>>$LOGS/mpc.log 2>>$LOGS/mpc.err
make check 1>>$LOGS/mpc.log 2>>$LOGS/mpc.err
make install 1>>$LOGS/mpc.log 2>>$LOGS/mpc.err


echo "  [.] gcc"
cd $SRC/gcc
mv gmp kaos_magic.gmp
mv mpc kaos_magic.mpc
mv mpfr kaos_magic.mpfr
sed -i 's/install_to_$(INSTALL_DEST) //' libiberty/Makefile.in
case `uname -m` in
  i?86) sed -i 's/^T_CFLAGS =$/& -fomit-frame-pointer/' \
        gcc/Makefile.in ;;
esac
sed -i 's@\./fixinc\.sh@-c true@' gcc/Makefile.in
mkdir -v ../gcc-build
cd ../gcc-build
../gcc/configure --prefix=/usr \
    --libexecdir=/usr/lib --enable-shared \
    --enable-threads=posix --enable-__cxa_atexit \
    --enable-clocale=gnu --enable-languages=c,c++ \
    --with-gmp-include=$(pwd)/gmp \
    --disable-multilib --disable-bootstrap --with-system-zlib 1>>$LOGS/gcc.log 2>>$LOGS/gcc.err
make 1>>$LOGS/gcc.log 2>>$LOGS/gcc.err
ulimit -s 16384
make -k check 1>>$LOGS/gcc.log 2>>$LOGS/gcc.err
../gcc/contrib/test_summary 1>>$LOGS/gcc.log 2>>$LOGS/gcc.err
make install 1>>$LOGS/gcc.log 2>>$LOGS/gcc.err
ln -sv ../usr/bin/cpp /lib
ln -sv gcc /usr/bin/cc
echo 'main(){}' > dummy.c
cc dummy.c -v -Wl,--verbose &> dummy.log
readelf -l a.out | grep ': /lib' 1>>$LOGS/gcc.log 2>>$LOGS/gcc.err
grep -o '/usr/lib.*/crt[1in].*succeeded' dummy.log 1>>$LOGS/gcc.log 2>>$LOGS/gcc.err
grep -B4 '^ /usr/include' dummy.log 1>>$LOGS/gcc.log 2>>$LOGS/gcc.err
grep 'SEARCH.*/usr/lib' dummy.log |sed 's|; |\n|g' 1>>$LOGS/gcc.log 2>>$LOGS/gcc.err
grep "/lib.*/libc.so.6 " dummy.log 1>>$LOGS/gcc.log 2>>$LOGS/gcc.err
grep found dummy.log 1>>$LOGS/gcc.log 2>>$LOGS/gcc.err

echo "  [.] sed "
cd $SRC/sed
./configure --prefix=/usr --bindir=/bin --htmldir=/usr/share/doc/sed-4.2.1 1>>$LOGS/sed.log 2>>$LOGS/sed.err
make 1>>$LOGS/sed.log 2>>$LOGS/sed.err1>>$LOGS/sed.log 2>>$LOGS/sed.err
make html 1>>$LOGS/sed.log 2>>$LOGS/sed.err
make check 1>>$LOGS/sed.log 2>>$LOGS/sed.err
make install 1>>$LOGS/sed.log 2>>$LOGS/sed.err

echo "  [.] bzip2"
cd $SRC/bzip2
patch -Np1 -i ../patches/bzip2-1.0.6-install_docs-1.patch
sed -i 's@\(ln -s -f \)$(PREFIX)/bin/@\1@' Makefile
make -f Makefile-libbz2_so 1>>$LOGS/bzip2.log 2>>$LOGS/bzip2.err
make clean 1>>$LOGS/bzip2.log 2>>$LOGS/bzip2.err
make 1>>$LOGS/bzip2.log 2>>$LOGS/bzip2.err
make PREFIX=/usr install 1>>$LOGS/bzip2.log 2>>$LOGS/bzip2.err 1>>$LOGS/bzip2.log 2>>$LOGS/bzip2.err
cp -v bzip2-shared /bin/bzip2 1>>$LOGS/bzip2.log 2>>$LOGS/bzip2.err
cp -av libbz2.so* /lib 1>>$LOGS/bzip2.log 2>>$LOGS/bzip2.err
ln -sv ../../lib/libbz2.so.1.0 /usr/lib/libbz2.so 1>>$LOGS/bzip2.log 2>>$LOGS/bzip2.err
rm -v /usr/bin/{bunzip2,bzcat,bzip2} 1>>$LOGS/bzip2.log 2>>$LOGS/bzip2.err
ln -sv bzip2 /bin/bunzip2 1>>$LOGS/bzip2.log 2>>$LOGS/bzip2.err
ln -sv bzip2 /bin/bzcat 1>>$LOGS/bzip2.log 2>>$LOGS/bzip2.err

echo "  [.] pcre "
cd $SRC/pcre
./configure --prefix=/usr \
            --docdir=/usr/share/doc/pcre-8.12 \
            --enable-utf8 \
            --enable-unicode-properties \
            --enable-pcregrep-libz \
            --enable-pcregrep-libbz2 1>>$LOGS/pcre.log 2>>$LOGS/pcre.err
make 1>>$LOGS/pcre.log 2>>$LOGS/pcre.err
make install 1>>$LOGS/pcre.log 2>>$LOGS/pcre.err
mv -v /usr/lib/libpcre.so.* /lib/ 1>>$LOGS/pcre.log 2>>$LOGS/pcre.err
ln -v -sf ../../lib/libpcre.so.0 /usr/lib/libpcre.so 1>>$LOGS/pcre.log 2>>$LOGS/pcre.err

echo "  [.] glib "
cd $SRC/glib
PCRE_LIBS="-L/usr/lib -lpcre" PCRE_CFLAGS="-I/usr/include" ./configure --prefix=/usr --sysconfdir=/etc --with-pcre=system 1>>$LOGS/glib.log 2>>$LOGS/glib.err
make 1>>$LOGS/glib.log 2>>$LOGS/glib.err
make install 1>>$LOGS/glib.log 2>>$LOGS/glib.err

echo "  [.] pkg-config "
cd $SRC/pkg-config
sed -i -e '21s/EXPECT_RETURN=1/EXPECT_RETURN=0/' check/check-cmd-options
GLIB_LIBS="-L/usr/lib -lglib-2.0" \
 GLIB_CFLAGS="-I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include" ./configure --prefix=/usr 1>>$LOGS/pkg-config.log 2>>$LOGS/pkg-config.err
make 1>>$LOGS/pkg-config.log 2>>$LOGS/pkg-config.err
make check 1>>$LOGS/pkg-config.log 2>>$LOGS/pkg-config.err
make install 1>>$LOGS/pkg-config.log 2>>$LOGS/pkg-config.err

echo "  [.] ncurses "
cd $SRC/ncurses
./configure --prefix=/usr --with-shared --without-debug --enable-widec 1>>$LOGS/ncurses.log 2>>$LOGS/ncurses.err
make 1>>$LOGS/ncurses.log 2>>$LOGS/ncurses.err
make install 1>>$LOGS/ncurses.log 2>>$LOGS/ncurses.err
mv -v /usr/lib/libncursesw.so.5* /lib 1>>$LOGS/ncurses.log 2>>$LOGS/ncurses.err
ln -sfv ../../lib/libncursesw.so.5 /usr/lib/libncursesw.so 1>>$LOGS/ncurses.log 2>>$LOGS/ncurses.err
for lib in ncurses form panel menu ; do \
    rm -vf /usr/lib/lib${lib}.so ; \
    echo "INPUT(-l${lib}w)" >/usr/lib/lib${lib}.so ; \
    ln -sfv lib${lib}w.a /usr/lib/lib${lib}.a ; \
done
ln -sfv libncurses++w.a /usr/lib/libncurses++.a 1>>$LOGS/ncurses.log 2>>$LOGS/ncurses.err
rm -vf /usr/lib/libcursesw.so 1>>$LOGS/ncurses.log 2>>$LOGS/ncurses.err
echo "INPUT(-lncursesw)" >/usr/lib/libcursesw.so
ln -sfv libncurses.so /usr/lib/libcurses.so 1>>$LOGS/ncurses.log 2>>$LOGS/ncurses.err
ln -sfv libncursesw.a /usr/lib/libcursesw.a 1>>$LOGS/ncurses.log 2>>$LOGS/ncurses.err
ln -sfv libncurses.a /usr/lib/libcurses.a 1>>$LOGS/ncurses.log 2>>$LOGS/ncurses.err

echo "  [.] util-linux "
cd $SRC/util-linux
sed -e 's@etc/adjtime@var/lib/hwclock/adjtime@g' \
    -i $(grep -rl '/etc/adjtime' .)
mkdir -pv /var/lib/hwclock
./configure --enable-arch --enable-partx --enable-write 1>>$LOGS/util-linux.log 2>>$LOGS/util-linux.err
make 1>>$LOGS/util-linux.log 2>>$LOGS/util-linux.err
make install 1>>$LOGS/util-linux.log 2>>$LOGS/util-linux.err

echo "  [.] e2fsprogs "
cd $SRC/e2fsprogs
mkdir -v build
cd build
../configure --prefix=/usr --with-root-prefix="" \
    --enable-elf-shlibs --disable-libblkid --disable-libuuid \
    --disable-uuidd --disable-fsck 1>>$LOGS/e2fsprogs.log 2>>$LOGS/e2fsprogs.err
make 1>>$LOGS/e2fsprogs.log 2>>$LOGS/e2fsprogs.err
make check 1>>$LOGS/e2fsprogs.log 2>>$LOGS/e2fsprogs.err
make install 1>>$LOGS/e2fsprogs.log 2>>$LOGS/e2fsprogs.err
make install-libs 1>>$LOGS/e2fsprogs.log 2>>$LOGS/e2fsprogs.err
chmod -v u+w /usr/lib/{libcom_err,libe2p,libext2fs,libss}.a 1>>$LOGS/e2fsprogs.log 2>>$LOGS/e2fsprogs.err
gunzip -v /usr/share/info/libext2fs.info.gz 1>>$LOGS/e2fsprogs.log 2>>$LOGS/e2fsprogs.err
install-info --dir-file=/usr/share/info/dir \
             /usr/share/info/libext2fs.info 1>>$LOGS/e2fsprogs.log 2>>$LOGS/e2fsprogs.err

echo "  [.] coreutils "
cd $SRC/coreutils
case `uname -m` in
 i?86 | x86_64) patch -Np1 -i ../patches/coreutils-8.12-uname-1.patch ;;
esac
patch -Np1 -i ../patches/coreutils-8.12-i18n-1.patch
./configure --prefix=/usr \
    --enable-no-install-program=kill,uptime 1>>$LOGS/coreutils.log 2>>$LOGS/coreutils.err
make 1>>$LOGS/coreutils.log 2>>$LOGS/coreutils.err
make NON_ROOT_USERNAME=nobody check-root 1>>$LOGS/coreutils.log 2>>$LOGS/coreutils.err
echo "dummy:x:1000:nobody" >> /etc/group
#chown -Rv nobody config.log {gnulib-tests,lib,src}/.deps 1>>$LOGS/coreutils.log 2>>$LOGS/coreutils.err
chown -Rv nobody . 1>>$LOGS/coreutils.log 2>>$LOGS/coreutils.err
su-tools nobody -s /bin/bash -c "make RUN_EXPENSIVE_TESTS=yes check" || true
sed -i '/dummy/d' /etc/group
make install 1>>$LOGS/coreutils.log 2>>$LOGS/coreutils.err
mv -v /usr/bin/{cat,chgrp,chmod,chown,cp,date,dd,df,echo} /bin 1>>$LOGS/coreutils.log 2>>$LOGS/coreutils.err
mv -v /usr/bin/{false,ln,ls,mkdir,mknod,mv,pwd,rm} /bin 1>>$LOGS/coreutils.log 2>>$LOGS/coreutils.err
mv -v /usr/bin/{rmdir,stty,sync,true,uname} /bin 1>>$LOGS/coreutils.log 2>>$LOGS/coreutils.err
mv -v /usr/bin/chroot /usr/sbin 1>>$LOGS/coreutils.log 2>>$LOGS/coreutils.err
mv -v /usr/bin/{head,sleep,nice} /bin 1>>$LOGS/coreutils.log 2>>$LOGS/coreutils.err

echo "  [.] iana-etc "
cd $SRC/iana-etc
make 1>>$LOGS/iana-etc.log 2>>$LOGS/iana-etc.err
make install 1>>$LOGS/iana-etc.log 2>>$LOGS/iana-etc.err

echo "  [.] m4 "
cd $SRC/m4
./configure --prefix=/usr 1>>$LOGS/m4.log 2>>$LOGS/m4.err
make 1>>$LOGS/m4.log 2>>$LOGS/m4.err
make check 1>>$LOGS/m4.log 2>>$LOGS/m4.err
make install 1>>$LOGS/m4.log 2>>$LOGS/m4.err

echo "  [.] bison "
cd $SRC/bison
./configure --prefix=/usr 1>>$LOGS/bison.log 2>>$LOGS/bison.err
echo '#define YYENABLE_NLS 1' >> config.h
make 1>>$LOGS/bison.log 2>>$LOGS/bison.err
make check 1>>$LOGS/bison.log 2>>$LOGS/bison.err
make install 1>>$LOGS/bison.log 2>>$LOGS/bison.err

echo "  [.] procps "
cd $SRC/procps
patch -Np1 -i ../patches/procps-3.2.8-fix_HZ_errors-1.patch
patch -Np1 -i ../patches/procps-3.2.8-watch_unicode-1.patch
sed -i -e 's@\*/module.mk@proc/module.mk ps/module.mk@' Makefile
make 1>>$LOGS/procps.log 2>>$LOGS/procps.err
make install 1>>$LOGS/procps.log 2>>$LOGS/procps.err

###
### FIXME: investigate if --without-included-regex is really needed here
###

echo "  [.] grep "
cd $SRC/grep
./configure --prefix=/usr \
    --bindir=/bin 1>>$LOGS/grep.log 2>>$LOGS/grep.err
make 1>>$LOGS/grep.log 2>>$LOGS/grep.err
make check 1>>$LOGS/grep.log 2>>$LOGS/grep.err
make install 1>>$LOGS/grep.log 2>>$LOGS/grep.err

echo "  [.] readline "
cd $SRC/readline
sed -i '/MV.*old/d' Makefile.in
sed -i '/{OLDSUFF}/c:' support/shlib-install
patch -Np1 -i ../patches/readline-6.2-fixes-1.patch 1>>$LOGS/readline.log 2>>$LOGS/readline.err
./configure --prefix=/usr --libdir=/lib 1>>$LOGS/readline.log 2>>$LOGS/readline.err
make SHLIB_LIBS=-lncurses 1>>$LOGS/readline.log 2>>$LOGS/readline.err
make install 1>>$LOGS/readline.log 2>>$LOGS/readline.err
mv -v /lib/lib{readline,history}.a /usr/lib 1>>$LOGS/readline.log 2>>$LOGS/readline.err
rm -v /lib/lib{readline,history}.so 1>>$LOGS/readline.log 2>>$LOGS/readline.err
ln -sfv ../../lib/libreadline.so.6 /usr/lib/libreadline.so 1>>$LOGS/readline.log 2>>$LOGS/readline.err
ln -sfv ../../lib/libhistory.so.6 /usr/lib/libhistory.so 1>>$LOGS/readline.log 2>>$LOGS/readline.err

echo "  [.] bash "
cd $SRC/bash
patch -Np1 -i ../patches/bash-4.2-fixes-3.patch >>$LOGS/bash.log 2>>$LOGS/bash.err
./configure --prefix=/usr --bindir=/bin \
    --htmldir=/usr/share/doc/bash-4.2 --without-bash-malloc \
    --with-installed-readline 1>>$LOGS/bash.log 2>>$LOGS/bash.err
make
chown -Rv nobody .
su-tools nobody -s /bin/bash -c "make tests"
make install 1>>$LOGS/bash.log 2>>$LOGS/bash.err

exit

