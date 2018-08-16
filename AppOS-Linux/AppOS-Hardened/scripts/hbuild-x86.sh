#!/bin/bash
#
# AppOS(R) Build Script (ABS)
# Copyright (c) 2002-2008 Spliced Networks LLC
#

set +h
umask 022
LC_ALL=POSIX
export LC_ALL

SNBLD=/opt/bldsys
export SNBLD
HLFS=$SNBLD/target32-hlfs
LSB=$HLFS/build
LSR=$SNBLD/hsource32
SNL=$SNBLD/log-x86.log
SNEL=$SNBLD/log-x86_err.log
TC="tar xvf"
PTC="patch -Np1 -i"
export HLFS TC LSR LSB SNL SNEL PTC

LSP=$LSR/patches
export LSP

mkdir -v $HLFS/tools
mkdir -p $HLFS/build
rm -f /tools
ln -sv $HLFS/tools /

echo "embryro toolchain " >> $SNL
echo "embryro toolchain " >> $SNEL
embryo_toolchain_h1.sh 1>>$SNL 2>>$SNEL

echo "linux headers " >> $SNL
echo "linux headers " >> $SNEL

linux_h1.sh 1>>$SNL 2>>$SNEL

echo "glibc " >> $SNL
echo "glibc " >> $SNEL

glibc_h1.sh 1>>$SNL 2>>$SNEL

echo "tcl " >> $SNL
echo "tcl " >> $SNEL

tcl_h1.sh 1>>$SNL 2>>$SNEL

echo "expect " >> $SNL
echo "expect " >> $SNEL

expect_h1.sh 1>>$SNL 2>>$SNEL

echo "dejagnu " >> $SNL
echo "dejagnu " >> $SNEL

dejagnu_h1.sh 1>>$SNL 2>>$SNEL

echo "cocoon toolchain " >> $SNL
echo "cocoon toolchain " >> $SNEL

cocoon_toolchain_h1.sh 1>>$SNL 2>>$SNEL

echo "ncurses " >> $SNL
echo "ncurses " >> $SNEL

ncurses_h1.sh 1>>$SNL 2>>$SNEL

echo "bash " >> $SNL
echo "bash " >> $SNEL

bash_h1.sh 1>>$SNL 2>>$SNEL

echo "tar " >> $SNL
echo "tar " >> $SNEL

tar_h1.sh 1>>$SNL 2>>$SNEL

echo "bzip2 " >> $SNL
echo "bzip2 " >> $SNEL

bzip2_h1.sh 1>>$SNL 2>>$SNEL

echo "coreutils " >> $SNL
echo "coreutils " >> $SNEL

coreutils_h1.sh 1>>$SNL 2>>$SNEL

echo "diffutils " >> $SNL
echo "diffutils " >> $SNEL

diffutils_h1.sh 1>>$SNL 2>>$SNEL

echo "findutils " >> $SNL
echo "findutils " >> $SNEL

findutils_h1.sh 1>>$SNL 2>>$SNEL

echo "gawk " >> $SNL
echo "gawk " >> $SNEL

gawk_h1.sh 1>>$SNL 2>>$SNEL

echo "gettext " >> $SNL
echo "gettext " >> $SNEL

gettext_h1.sh 1>>$SNL 2>>$SNEL

echo "grep " >> $SNL
echo "grep " >> $SNEL

grep_h1.sh 1>>$SNL 2>>$SNEL

echo "gzip " >> $SNL
echo "gzip " >> $SNEL

gzip_h1.sh 1>>$SNL 2>>$SNEL

echo "m4 " >> $SNL
echo "m4 " >> $SNEL

m4_h1.sh 1>>$SNL 2>>$SNEL

echo "make " >> $SNL
echo "make " >> $SNEL

make_h1.sh 1>>$SNL 2>>$SNEL

echo "patch " >> $SNL
echo "patch " >> $SNEL

patch_h1.sh 1>>$SNL 2>>$SNEL

echo "perl " >> $SNL
echo "perl " >> $SNEL

perl_h1.sh 1>>$SNL 2>>$SNEL

echo "sed " >> $SNL
echo "sed " >> $SNEL

sed_h1.sh 1>>$SNL 2>>$SNEL

echo "texinfo " >> $SNL
echo "texinfo " >> $SNEL

textinfo_h1.sh 1>>$SNL 2>>$SNEL

echo "bison " >> $SNL
echo "bison " >> $SNEL

bison_h1.sh 1>>$SNL 2>>$SNEL

echo "flex " >> $SNL
echo "flex " >> $SNEL

flex_h1.sh 1>>$SNL 2>>$SNEL

echo "bc " >> $SNL
echo "bc " >> $SNEL

bc_h1.sh 1>>$SNL 2>>$SNEL

echo "util-linux " >> $SNL
echo "util-linux " >> $SNEL

util-linux_h1.sh 1>>$SNL 2>>$SNEL

find /tools/lib -type f -exec strip --strip-debug '{}' ';'
strip --strip-unneeded /tools/{,s}bin/*

rm -rf /tools/{info,man}

install -vd $HLFS/{dev,proc,sys}
mknod -m 600 $HLFS/dev/console c 5 1
mknod -m 666 $HLFS/dev/null c 1 3
mount -v --bind /dev $HLFS/dev
mount -vt devpts -o mode=620 devpts $HLFS/dev/pts
mount -vt tmpfs shm $HLFS/dev/shm
mount -vt proc proc $HLFS/proc
mount -vt sysfs sysfs $HLFS/sys

cd $SNBLD

mkdir -p $HLFS/bldsys
cp -a $LSR $HLFS/bldsys/src
cp -a hscripts32/chroot $HLFS/bldsys/scripts
cp -a hscripts32/hbuild-x86-chroot.sh $HLFS/bldsys/
cp -a hscripts32/hbuild-x86-chroot2.sh $HLFS/bldsys/
cp -a hscripts32/hstrip-x86-chroot3.sh $HLFS/bldsys/
chmod 755 $HLFS/bldsys/scripts/*
chmod 755 $HLFS/bldsys/*.sh
chown -R 0:0 $LFS/bldsys

chroot "$HLFS" /tools/bin/env -i \
    HOME=/root TERM="$TERM" PS1='\u:\w\$ ' \
    PATH=/bin:/usr/bin:/sbin:/usr/sbin:/tools/bin \
    /bldsys/hbuild-x86-chroot.sh

