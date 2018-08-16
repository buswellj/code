#!/bin/bash
#
#        +--------------------------------------------------+
#        |                                                  |
#        | Project  : Ion Linux Stage 2 Build Script        |
#        | Developer: John Buswell <johnb@ionlinux.com>     |
#        | Created  : July 20th 2003                        |
#        |                                                  |
#        +--------------------------------------------------+
#        |                                                  |
#        | Updates  :                                       |
#        |                                                  |
#        |                                                  |
#        +--------------------------------------------------+
#        |                                                  |
#        | Copyright (c) 2002-2003 Spliced Networks         |
#        | All Rights Reserved.                             |
#        |                                                  |
#        | This script is property of Spliced Networks.     |
#        | This script may not be copied or distributed.    |
#        |                                                  |
#        +--------------------------------------------------+
#

# note $1 = release $2 = maintenance $3 = patch
# insert some check here for valid version

# directory layout is as follows :
#
# /projects/ion			master directory
# /projects/ion/src		src code
# /projects/ion/X.Y		version X.Y built code
# /projects/ion/bld		tempory build directory
#
# All sub directories are in the format X.Y/Z where 
# 
# 	X		Release (Major) version
#	Y		Maintenance (Minor) version
#	Z		Patch version
#
#
# 
# Description:	This is the second stage build script.
#		It is executed as a non-root user.
#		This script will uncompress, build and
#		install the initial environment for a new
#		release in $ION. When this completes, the
#		stage-3 script needs to be run as root.
#
#		If you upgrade any packages as part of
#		the core segment of Ion, then you need to
#		modify patches/source files here.
#
#
# First set the ION environment variable

IV1=`cat /tmp/.ion_ver.rel`
IV2=`cat /tmp/.ion_ver.maint`
IV3=`cat /tmp/.ion_ver.patch`

ION_NAME="ion"
ION_SCR_VER=1.0.0
ION=/ion/release/$IV1.$IV2/$IV3
ION_SRC=/ion/src/$IV1.$IV2/$IV3
ION_BLD=/ion/bld/$IV1.$IV2/$IV3

ION_SRC_GNU=$ION_SRC/gnu
ION_SRC_OPS=$ION_SRC/opensrc
ION_SRC_KER=$ION_SRC/kernel
ION_SRC_PAT=$ION_SRC/patch

clear
echo ""
echo "   "
echo "   Ion Linux Stage 2 Build Script v$ION_SCR_VER"
echo "   Copyright (C) 2002-2003 Spliced Networks"
echo "   "
echo "   Building Ion Linux $IV1.$IV2.$IV3"
echo "   "
echo "   Build  = $ION_BLD"
echo "   Source = $ION_SRC"
echo "   "
echo "        GNU Source   = $ION_SRC_GNU"
echo "        Open Source  = $ION_SRC_OPS"
echo "        Linux Source = $ION_SRC_KER"
echo "        Patch Source = $ION_SRC_PAT"
echo "   "
echo "        Target = $ION"
echo "  "
echo "   $PWD"
echo "  "
echo ""
echo ""

echo "-> Setting up environment "
echo ""
cat > ~/.bash_profile << "EOF"
set +h
umask 022
LC_ALL=POSIX
CC=gcc
LDFLAGS="-s"
PATH=/shared/bin:$PATH
export ION LC_ALL CC LDFLAGS PATH
EOF
source ~/.bash_profile

echo ""
source /ion/buildver.rc

ION_HOME=$PWD
echo ""

echo "-> Cleaning build area $ION_BLD"
echo ""

rm -rf $ION_BLD/*

echo "-> Building Release in $ION"
echo ""

echo "-> binutils-$ION_BINUTILS_VER (pass 1 - static)"
echo ""

cd $ION_BLD
tar jxvf $ION_SRC_GNU/binutils-$ION_BINUTILS_VER.tar.bz2

mkdir binutils-build && cd binutils-build
CFLAGS="-O2 -pipe" ../binutils-$ION_BINUTILS_VER/configure --prefix=/shared \
 --disable-nls
make LDFLAGS="-all-static -s" && make install

cd $ION_BLD/binutils-build
cd ld &&
make clean &&
make LIB_PATH=/shared/lib

echo ""
echo "-> gcc-$ION_GCC_VER (pass 1 - static)"
echo ""

cd $ION_BLD
tar jxvf $ION_SRC_GNU/gcc-$ION_GCC_VER.tar.bz2

cd gcc-$ION_GCC_VER
patch -Np1 -i $ION_SRC_PAT/gcc-3.3-nofixincludes.patch
patch -Np1 -i $ION_SRC_PAT/gcc-3.3-mmap_test.patch

mkdir ../gcc-build &&
cd ../gcc-build &&
CFLAGS="-O2 -pipe" ../gcc-$ION_GCC_VER/configure --prefix=/shared \
       --with-local-prefix=/shared --enable-languages=c \
       --disable-nls --enable-shared &&
make BOOT_LDFLAGS="-static -s" BOOT_CFLAGS="-O2 -pipe" \
       STAGE1_CFLAGS="-pipe" bootstrap &&
make install &&
ln -s gcc /shared/bin/cc 

echo "-> Kernel Headers ($ION_LINUX_VER)"
echo ""

cd $ION_BLD
tar jxvf $ION_SRC_KER/linux-$ION_LINUXHEADERS_VER.tar.bz2

cd linux-$ION_LINUXHEADERS_VER
make mrproper &&
make include/linux/version.h &&
make symlinks
#ln -s asm-i386 include/asm
cp -HR include/asm /shared/include &&
cp -R include/asm-generic /shared/include &&
cp -R include/linux /shared/include &&
touch /shared/include/linux/autoconf.h

echo ""
echo "-> glibc-$ION_GLIBC_VER"
echo ""

cd $ION_BLD
tar jxvf $ION_SRC_GNU/glibc-$ION_GLIBC_VER.tar.bz2

patch -Np0 -i $ION_SRC_PAT/glibc-2.3.2-sscanf.patch

cd glibc-$ION_GLIBC_VER
tar jxvf $ION_SRC_GNU/glibc-linuxthreads-$ION_GLIBC_VER.tar.bz2

mkdir -p /shared/etc &&
touch /shared/etc/ld.so.conf &&
mkdir ../glibc-build &&
cd ../glibc-build &&
CFLAGS="-O2 -pipe" ../glibc-$ION_GLIBC_VER/configure --prefix=/shared \
       --enable-add-ons --disable-profile \
       --with-binutils=/shared/bin --with-headers=/shared/include \
       --without-gd &&
make &&
#make check &&
make install &&
make localedata/install-locales

cd $ION_BLD
cd binutils-build
cd ld 
make install-data-local

SPECFILE=/shared/lib/gcc-lib/i686-pc-linux-gnu/*/specs &&
cp ${SPECFILE} ./XX &&
sed 's@/lib/ld-linux.so.2@/shared/lib/ld-linux.so.2@g' ./XX > ${SPECFILE} &&
unset SPECFILE &&
rm -f ./XX

cd $ION_BLD
tar zxvf $ION_SRC_OPS/tcl$ION_TCL_VER-src.tar.gz
cd tcl$ION_TCL_VER/unix
./configure --prefix=/shared &&
make && make install &&
ln -s tclsh8.4 /shared/bin/tclsh

cd $ION_BLD
tar zxvf $ION_SRC_OPS/expect.tar.gz
cd expect-5.38
patch -Np1 -i $ION_SRC_PAT/expect-5.38.patch
./configure --prefix=/shared &&
make && make install

cd $ION_BLD
tar zxvf $ION_SRC_GNU/dejagnu-$ION_DEJAGNU_VER.tar.gz
cd dejagnu-$ION_DEJAGNU_VER
./configure --prefix=/shared &&
make &&
make install

echo ""
echo "-> gcc-$ION_GCC_VER (pass 2 - shared)"
echo ""

mv $ION_BLD/gcc-build $ION_BLD/gcc-build-static
mv $ION_BLD/gcc-$ION_GCC_VER $ION_BLD/gcc-src-static

cd $ION_BLD
tar jxvf $ION_SRC_GNU/gcc-$ION_GCC_VER.tar.bz2

cd gcc-$ION_GCC_VER
patch -Np1 -i $ION_SRC_PAT/gcc-3.3-nofixincludes.patch
patch -Np1 -i $ION_SRC_PAT/gcc-3.3-mmap_test.patch
patch -Np1 -i $ION_SRC_PAT/gcc-3.2.1.specs.patch

mkdir ../gcc-build &&
cd ../gcc-build &&
CFLAGS="-O2 -pipe" CXXFLAGS="-O2 -pipe" ../gcc-$ION_GCC_VER/configure \
  --prefix=/shared --with-local-prefix=/shared --enable-languages=c,c++ \
  --enable-shared --enable-threads=posix --enable-__cxa_atexit \
  --enable-clocale=gnu &&
make LDFLAGS="-s" &&
#make -k check
make install

mv $ION_BLD/binutils-build $ION_BLD/binutils-build-static
mv $ION_BLD/binutils-$ION_BINUTILS_VER $ION_BLD/binutils-old-static

cd $ION_BLD
tar jxvf $ION_SRC_GNU/binutils-$ION_BINUTILS_VER.tar.bz2
cd binutils-$ION_BINUTILS_VER
patch -Np1 -i $ION_SRC_PAT/binutils-2.13.2.lib-path.patch
mkdir ../binutils-build &&
cd ../binutils-build &&
CFLAGS="-O2 -pipe" ../binutils-$ION_BINUTILS_VER/configure --prefix=/shared \
  --with-lib-path=/shared/lib --enable-shared &&
make LDFLAGS="-s" &&
make check &&
make install

cd ld &&
make clean &&
make LIB_PATH=/usr/lib:lib

cd $ION_BLD
tar jxvf $ION_SRC_GNU/gawk-$ION_GAWK_VER.tar.bz2
cd gawk-$ION_GAWK_VER
./configure --prefix=/shared --disable-nls
make && make install

cd $ION_BLD
tar zxvf $ION_SRC_GNU/fileutils-$ION_FILEUTILS_VER.tar.gz
cd fileutils-$ION_FILEUTILS_VER
patch -Np1 -i $ION_SRC_PAT/fileutils-4.1.patch
./configure --prefix=/shared --disable-nls
make && make install

cd $ION_BLD
tar zxvf $ION_SRC_OPS/bzip2-$ION_BZIP2_VER.tar.gz
cd bzip2-$ION_BZIP2_VER
make
make PREFIX=/shared install

cd $ION_BLD
tar zxvf $ION_SRC_GNU/gzip-$ION_GZIP_VER.tar.gz
cd gzip-$ION_GZIP_VER
./configure --prefix=/shared
make && make install

cd $ION_BLD
tar zxvf $ION_SRC_GNU/diffutils-$ION_DIFFUTILS_VER.tar.gz
cd diffutils-$ION_DIFFUTILS_VER
./configure --prefix=/shared --disable-nls
make && make install

cd $ION_BLD
tar zxvf $ION_SRC_GNU/findutils-$ION_FINDUTILS_VER.tar.gz
cd findutils-$ION_FINDUTILS_VER
patch -Np1 -i $ION_SRC_PAT/findutils-4.1.patch
./configure --prefix=/shared
make && make install

cd $ION_BLD
tar jxvf $ION_SRC_GNU/make-$ION_MAKE_VER.tar.bz2
cd make-$ION_MAKE_VER
./configure --prefix=/shared --disable-nls
make && make install

cd $ION_BLD
tar jxvf $ION_SRC_GNU/grep-$ION_GREP_VER.tar.bz2
cd grep-$ION_GREP_VER
CFLAGS="-O2 -pipe" ./configure --prefix=/shared \
	--disable-perl-regexp --with-include-regex &&
make && make install

cd $ION_BLD
tar zxvf $ION_SRC_GNU/sed-$ION_SED_VER.tar.gz
cd sed-$ION_SED_VER
./configure --prefix=/shared --disable-nls
make && make install

cd $ION_BLD
tar zxvf $ION_SRC_GNU/gettext-$ION_GETTEXT_VER.tar.gz
cd gettext-$ION_GETTEXT_VER
CFLAGS="-O2 -pipe" ./configure --prefix=/shared &&
make && make install
# && rm -f /shared/lib/gettext/gnu.gettext.*

cd $ION_BLD
tar jxvf $ION_SRC_GNU/textutils-$ION_TEXTUTILS_VER.tar.bz2
cd textutils-$ION_TEXTUTILS_VER
./configure --prefix=/shared --disable-nls &&
make && make install

cd $ION_BLD
tar zxvf $ION_SRC_GNU/sh-utils-$ION_SH_UTILS_VER.tar.gz
cd sh-utils-$ION_SH_UTILS_VER
patch -Np1 -i $ION_SRC_PAT/sh-utils-2.0.patch
./configure --prefix=/shared --disable-nls
make && make install

cd $ION_BLD
tar zxvf $ION_SRC_GNU/ncurses-$ION_NCURSES_VER.tar.gz
cd ncurses-$ION_NCURSES_VER
patch -Np1 -i $ION_SRC_PAT/ncurses-5.3.etip.patch 
./configure --prefix=/shared --with-shared --without-debug 
make && make install

cd $ION_BLD
tar zxvf $ION_SRC_GNU/patch-$ION_PATCH_VER.tar.gz
cd patch-$ION_PATCH_VER
CPPFLAGS=-D_GNU_SOURCE \
./configure --prefix=/shared
make  && make install

cd $ION_BLD
tar zxvf $ION_SRC_GNU/tar-$ION_TAR_VER.tar.gz
cd tar-$ION_TAR_VER
patch -Np1 -i $ION_SRC_PAT/tar-1.13.patch
./configure --prefix=/shared --disable-nls
make && make install

cd $ION_BLD
tar jxvf $ION_SRC_GNU/texinfo-$ION_TEXINFO_VER.tar.bz2
cd texinfo-$ION_TEXINFO_VER
./configure --prefix=/shared --disable-nls
make && make install

cd $ION_BLD
tar zxvf $ION_SRC_GNU/bash-$ION_BASH_VER.tar.gz
cd bash-$ION_BASH_VER
./configure --prefix=/shared --with-curses
make && make install

cd $ION_BLD
tar jxvf $ION_SRC_KER/util-linux-$ION_UTIL_LINUX_VER.tar.bz2
cd util-linux-$ION_UTIL_LINUX_VER
./configure
make -C lib
make -C mount mount umount

cp mount/{mount,umount} /shared/bin

cd $ION_BLD
tar zxvf $ION_SRC_OPS/perl-$ION_PERL_VER.tar.gz
cd perl-$ION_PERL_VER
patch -Np1 -i $ION_SRC_PAT/perl-5.8.0.libc.patch &&
./configure.gnu --prefix=/shared -Doptimize='-O2 -pipe' &&
make utilities &&
cp miniperl /shared/bin/perl &&
cp pod/pod2man /shared/bin &&
mkdir -p /shared/lib/perl5/5.8.0 &&
cp -R lib/* /shared/lib/perl5/5.8.0


