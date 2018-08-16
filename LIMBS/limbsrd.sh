#!/bin/bash

ION_LINITRD_SIZE=9216
ION_LINITRD=limbsrd.img
ION_LLOOP=/dev/loop4
ION_LFS=ext2

mkdir -p /tmp/bldrd
cd /tmp/bldrd
mkdir -p mnt
dd if=/dev/zero of=$ION_LINITRD bs=1k count=ION_LINITRD_SIZE
/sbin/losetup $ION_LLOOP $ION_LINITRD
/sbin/mkfs -t $ION_LFS -m 0 $ION_LLOOP
mount $ION_LLOOP mnt/
mkdir -p {dev,bin,sbin,ion-root,etc,lib,usr,usr/bin,usr/lib,proc}
LIMBS_LIBSRC=/ion/build/release/lib
export LIMBS_LIBSRC
strip $LIMBS_LIBSRC/libc-2.3.2.so -o lib/libc-2.3.2.so
strip $LIMBS_LIBSRC/ld-2.3.2.so -o lib/ld-2.3.2.so
strip $LIMBS_LIBSRC/libncurses.so.5.3 -o lib/libncurses.so.5.3
strip $LIMBS_LIBSRC/libdl-2.3.2.so -o lib/libdl-2.3.2.so
cd lib/
ln -s libc-2.3.2.so libc.so.6
ln -s ld-2.3.2.so ld-linux.so.2
ln -s libncurses.so.5.3 libncurses.so.5
ln -s libdl-2.3.2.so libdl.so.2
chmod 755 .
cd ..
LIMBS_SRC=/ion/build/release
export LIMBS_SRC
mkdir -p usr/share/terminfo/l
mkdir -p usr/share/terminfo/v
mkdir -p usr/share/terminfo/d
cp -a $LIMBS_SRC/usr/share/terminfo/l/linux* usr/share/terminfo/l
cp -a $LIMBS_SRC/usr/share/terminfo/d/dumb usr/share/terminfo/d
cp -a $LIMBS_SRC/usr/share/terminfo/v/vt* usr/share/terminfo/v
cd dev
cp -a $LIMBS_SRC/dev/* .
mknod ion-active b 7 7
cp /ion/build/release/sbin/e2fsck sbin/
cp /ion/build/release/sbin/jfs_fsck sbin/
cp /ion/build/release/usr/sbin/reiserfsck sbin/
cp /ion/build/release/bin/mount sbin
cp /ion/build/release/bin/umount sbin
cp /ion/build/release/sbin/losetup sbin
cd ../usr/lib
ln -sf ../share/terminfo terminfo
build_busybox

