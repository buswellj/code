#!/bin/bash
#
# Open Kernel Attached Operating System (OpenKaOS)
# Platform Build System version 3.0.0
#
# Copyright (c) 2009-2014 Opaque Systems LLC
#
# script : bld-prepsdk.sh
# purpose: prepares the system for building the SDK chroot
#

echo "  [.] Creating Virtual File Systems"
sudo mkdir -pv $LFS/{dev,proc,sys,run}
sudo mknod -m 600 $LFS/dev/console c 5 1
sudo mknod -m 666 $LFS/dev/null c 1 3
sudo mount -v --bind /dev $LFS/dev
sudo mount -vt devpts devpts $LFS/dev/pts -o gid=5,mode=620
sudo mount -vt tmpfs shm $LFS/dev/shm
sudo mount -vt tmpfs tmpfs $LFS/run
sudo mount -vt proc proc $LFS/proc
sudo mount -vt sysfs sysfs $LFS/sys

if [ -h $LFS/dev/shm ]; then
  sudo mkdir -pv $LFS/$(readlink $LFS/dev/shm)
fi

echo "  [.] Backing up bootstrap environment"
sudo cp -a $LFS$TOOLS $LFS/..$TOOLS.backup.$PBNOW

echo "  [.] Copying source"
sudo mkdir $LFS/src
sudo cp -a $PBSRC/* $LFS/src
sudo cp -a bld-sdk*.sh $LFS/src
sudo mv $LFS/src/bld-sdk.sh $LFS/src/bld-sdk1.sh
sudo su - -c "echo "#!$TOOLS/bin/bash" > $LFS/src/bld-sdk.sh"
sudo su - -c "cat $LFS/src/bld-sdk1.sh >> $LFS/src/bld-sdk.sh"
sudo mv $LFS/src/bld-sdk3.sh $LFS/src/bld-sdk4.sh
sudo su - -c "echo "#!$TOOLS/bin/bash" > $LFS/src/bld-sdk3.sh"
sudo su - -c "cat $LFS/src/bld-sdk4.sh >> $LFS/src/bld-sdk3.sh"
sudo chmod 755 $LFS/src/*.sh

echo "  [.] Executing bld-sdk script"
sudo su - -c "echo "$TOOLS" > $LFS/.tools"
