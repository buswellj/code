#!/bin/bash
#
# Kernel Attached Operating System (KaOS)
# Platform Build System version 1.0.0
#
# Copyright (c) 2009-2011 Carbon Mountain LLC
#
# script : chroot-fcs.sh
# purpose: manual script to enter chroot for FCS release
#

echo "  [.] Loading environment "
PBCUR=`cat ~/kaos-ws/.current`
export PBCUR
PBENV="/home/kaos/kaos-ws/$PBCUR/$1"
export PBENV
source $PBENV/.env
LFS="$PBWS/$PBTAG/$1/bld/"
export LFS
echo "      Env is $PBENV"
echo "      Build is $LFS"
echo ""

echo " [.] Mounting Virtual File Systems"
sudo mount -v --bind /dev $LFS/dev
sudo mount -vt devpts devpts $LFS/dev/pts
sudo mount -vt tmpfs shm $LFS/dev/shm
sudo mount -vt proc proc $LFS/proc
sudo mount -vt sysfs sysfs $LFS/sys

echo " [.] Entering chroot"

sudo chroot "$LFS" /usr/bin/env -i \
    HOME=/root TERM="$TERM" PS1='\u:\w\$ ' \
    PATH=/bin:/usr/bin:/sbin:/usr/sbin \
    /bin/bash --login +h

sudo umount $LFS/dev/pts $LFS/dev/shm $LFS/dev $LFS/proc $LFS/sys

