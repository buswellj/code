#!/bin/bash
#
# AppOS(R) Build Script (ABS)
# Copyright (c) 2002-2008 Spliced Networks LLC
#

echo ""
echo "ABS version $SNVER_ABS (32-bit x86 build)"
echo "Copyright (c) $SNVER_COPYSTART-$SNVER_COPYEND Spliced Networks LLC"
echo ""

PATH=$PATH:$SNBLD/scripts32
export PATH

TPATH=$PATH
export TPATH

PATH=/tools/bin:$TPATH
export PATH

set +h
umask 022
LC_ALL=POSIX
export LC_ALL

# build numbers are now generated using unix time

SNBN=`date +%s`
export SNBN

# fetch the initial source and patches

mkdir source32-$SNBN
fetch-src-ibe32.sh

# do some setup 
# we install to target-$SNBN

mkdir -p target32-$SNBN

LFS=$SNBLD/target32-$SNBN
LSB=$LFS/build
LSR=$SNBLD/source32-$SNBN
SNL=$SNBLD/$SNBN-x86.log
SNEL=$SNBLD/$SNBN-x86_err.log
TC="tar xvf"
PTC="patch -Np1 -i"
export LFS TC LSR LSB SNL SNEL PTC

LSP=$LSR/patches
export LSP

# This is old legacy variables from LFS days

mkdir -v $LFS/tools
mkdir -p $LFS/build
rm -f /tools
ln -sv $LFS/tools /

echo "building binutils" >> $SNL
echo "building binutils" >> $SNEL
binutils_p1.sh 1>>$SNL 2>>$SNEL

echo "building gcc pass1" >> $SNL
echo "building gcc pass1" >> $SNEL
gcc_p1.sh 1>>$SNL 2>>$SNEL

echo "building linux hdr" >> $SNL
echo "building linux hdr" >> $SNEL
linuxhdr_p1.sh 1>>$SNL 2>>$SNEL

echo "building glibc pass1" >> $SNL
echo "building glibc pass1" >> $SNEL
glibc_p1.sh 1>>$SNL 2>>$SNEL

echo "building tcl" >> $SNL
echo "building tcl" >> $SNEL
tcl_p1.sh 1>>$SNL 2>>$SNEL

echo "building expect" >> $SNL
echo "building expect" >> $SNEL
expect_p1.sh 1>>$SNL 2>>$SNEL

echo "building dejagnu" >> $SNL
echo "building dejagnu" >> $SNEL
dejagnu_p1.sh 1>>$SNL 2>>$SNEL

echo "building gcc pass2" >> $SNL
echo "building gcc pass2" >> $SNEL
gcc_p2.sh 1>>$SNL 2>>$SNEL

echo "building binutils pass2" >> $SNL
echo "building binutils pass2" >> $SNEL
binutils_p2.sh 1>>$SNL 2>>$SNEL

echo "building ncurses" >> $SNL
echo "building ncurses" >> $SNEL
ncurses_p2.sh 1>>$SNL 2>>$SNEL

echo "building bash" >> $SNL
echo "building bash" >> $SNEL
bash_p2.sh 1>>$SNL 2>>$SNEL

echo "building bzip2" >> $SNL
echo "building bzip2" >> $SNEL
bzip_p2.sh 1>>$SNL 2>>$SNEL

echo "building coreutils" >> $SNL
echo "building coreutils" >> $SNEL
coreutils_p2.sh 1>>$SNL 2>>$SNEL

echo "building diffutils" >> $SNL
echo "building diffutils" >> $SNEL
diffutils_p2.sh 1>>$SNL 2>>$SNEL

echo "building findutils" >> $SNL
echo "building findutils" >> $SNEL
findutils_p2.sh 1>>$SNL 2>>$SNEL

echo "building gawk" >> $SNL
echo "building gawk" >> $SNEL
gawk_p2.sh 1>>$SNL 2>>$SNEL

echo "building gettext" >> $SNL
echo "building gettext" >> $SNEL
gettext_p2.sh 1>>$SNL 2>>$SNEL

echo "building grep" >> $SNL
echo "building grep" >> $SNEL
grep_p2.sh 1>>$SNL 2>>$SNEL

echo "building gzip" >> $SNL
echo "building gzip" >> $SNEL
gzip_p2.sh 1>>$SNL 2>>$SNEL

echo "building make" >> $SNL
echo "building make" >> $SNEL
make_p2.sh 1>>$SNL 2>>$SNEL

echo "building patch" >> $SNL
echo "building patch" >> $SNEL
patch_p2.sh 1>>$SNL 2>>$SNEL

echo "building perl" >> $SNL
echo "building perl" >> $SNEL
perl_p2.sh 1>>$SNL 2>>$SNEL

echo "building sed" >> $SNL
echo "building sed" >> $SNEL
sed_p2.sh 1>>$SNL 2>>$SNEL

echo "building tar" >> $SNL
echo "building tar" >> $SNEL
tar_p2.sh 1>>$SNL 2>>$SNEL

echo "building texinfo" >> $SNL
echo "building texinfo" >> $SNEL
texinfo_p2.sh 1>>$SNL 2>>$SNEL

echo "building util-linux" >> $SNL
echo "building util-linux" >> $SNEL
util-linux_p2.sh 1>>$SNL 2>>$SNEL

# strip
echo ""
echo " => stripping tools "
strip --strip-debug /tools/lib/*
strip --strip-unneeded /tools/{,s}bin/*
rm -rf /tools/{info,man}

# change permissions
echo " => changing ownership of x86 tools"
chown -R root:root $LFS/tools

mkdir -pv $LFS/{dev,proc,sys}

# prepare kernel file systems
echo " => creating virtual kernel file systems"
mknod -m 600 $LFS/dev/console c 5 1
mknod -m 666 $LFS/dev/null c 1 3
mount -v --bind /dev $LFS/dev
mount -vt devpts devpts $LFS/dev/pts
mount -vt tmpfs shm $LFS/dev/shm
mount -vt proc proc $LFS/proc
mount -vt sysfs sysfs $LFS/sys

# 
# at this point everything else is done inside the chroot
# for this to work we have to copy over some things
#
# the source files, patches, build-x86-chroot.sh,
# scripts32/chroot/ and then execute the build-x86-chroot.sh
#
# we must set the permissions, ownership and so on..
#

cd $SNBLD

mkdir -p $LFS/bldsys
cp -a $LSR $LFS/bldsys/src
cp -a scripts32/chroot $LFS/bldsys/scripts
cp -a scripts32/build-x86-chroot.sh $LFS/bldsys/
cp -a scripts32/build-x86-chroot2.sh $LFS/bldsys/
cp -a scripts32/strip-x86-chroot3.sh $LFS/bldsys/
chmod 755 $LFS/bldsys/scripts/*
chmod 755 $LFS/bldsys/*.sh
chown -R 0:0 $LFS/bldsys

#
# /bldsys
# /bldsys/src
# /bldsys/src/patches
# /bldsys/scripts
#

chroot "$LFS" /tools/bin/env -i \
    HOME=/root TERM="$TERM" PS1='\u:\w\$ ' \
    PATH=/bin:/usr/bin:/sbin:/usr/sbin:/tools/bin:/bldsys/scripts \
    /bldsys/build-x86-chroot.sh

#
# this is phase 2, basically second script to continue past bash
#

chroot "$LFS" /usr/bin/env -i \
    HOME=/root TERM="$TERM" PS1='\u:\w\$ ' \
    PATH=/bin:/usr/bin:/sbin:/usr/sbin:/tools/bin:/bldsys/scripts \
    /bldsys/build-x86-chroot2.sh

#    /tools/bin/bash --login +h

chroot $LFS /tools/bin/env -i \
    HOME=/root TERM=$TERM PS1='\u:\w\$ ' \
    PATH=/bin:/usr/bin:/sbin:/usr/sbin \
    /bldsys/strip-x86-chroot3.sh

echo " => Backing up toolchain "
mv $LFS/tools tools-$SNBN

