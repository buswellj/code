#!/bin/bash
#
# Kernel Attached Operating System (KaOS)
# Platform Build System version 1.0.0
#
# Copyright (c) 2009-2011 Carbon Mountain LLC
#
# script : bld-kaos.sh
# purpose: builds the KaOS platform from scratch
#

PPWD=`pwd`
cd ~
PBHOME=`pwd`
cd $PPWD
PBVER="1.0.0"
PBUSER=`whoami`
PBNOW=`date +%s`
PBWS="$PBHOME/kaos-ws"
PBTAG=`cat $PBWS/.current`

export PPWD PBHOME PBVER PBUSER PBNOW PBWS PBTAG

echo ""
echo "KaOS Platform Build System, version $PBVER"
echo "Copyright (c) 2009-2011 Carbon Mountain LLC"
echo ""
echo "http://www.carbonmountain.com"
echo ""

###
### FIXME -- Add code here to check that passed parameter is correct ###
###

PBSRC="$PBWS/$PBTAG/pkg/$1"
PBBLD="$PBWS/$PBTAG/bld-$PBNOW"
PBLOG="$PBWS/$PBTAG/log-$PBNOW"
export PBSRC PBBLD PBLOG

echo "  [-] Starting Build ID# $PBNOW"
echo "  [.] Creating Build and Log directories..."
mkdir -p $PBBLD $PBLOG

echo "  [-] Environment Information: "
echo ""
echo "        User is $PBUSER ($PBHOME)"
echo "        Building $PBTAG in $PBBLD"
echo "        Source is $PBSRC"
echo "        Logs stored in $PBLOG"
echo ""

echo "PBUSER=$PBUSER" > $PBBLD/.env
echo "PBHOME=$PBHOME" >> $PBBLD/.env
echo "PBWS=$PBWS" >> $PBBLD/.env
echo "PBTAG=$PBTAG" >> $PBBLD/.env
echo "PBSRC=$PBSRC" >> $PBBLD/.env
echo "PBBLD=$PBBLD" >> $PBBLD/.env
echo "PBLOG=$PBLOG" >> $PBBLD/.env
echo "export PBUSER PBHOME PBWS PBTAG PBSRC PBBLD PBLOG" >> $PBBLD/.env
echo "        Environment saved to $PBBLD/.env"
echo ""

echo "  [*] Building toolchain..."
echo ""
LFS="$PBBLD/bld"
TOOLS="/tools.$PBUSER"
export LFS TOOLS
echo "        Build is $LFS"
echo "        Tools is $TOOLS"
echo ""

cd $PPWD
sudo mkdir -p $LFS/tools.$PBUSER
sudo rm -rf $TOOLS
sudo ln -sv $LFS/tools.$PBUSER $TOOLS
sudo chown -v $PBUSER $LFS/tools.$PBUSER

source bld-toolchain.sh

echo "  [*] Preparing SDK Environment..."
echo ""
cd $PPWD

source bld-prepsdk.sh

echo "  [*] Building KaOS SDK..."
echo ""
cd $PPWD

echo "  [.] SDK phase 1"
sudo chroot "$LFS" $TOOLS/bin/env -i \
    HOME=/root TERM="$TERM" PS1='\u:\w\$ ' \
    PATH=/bin:/usr/bin:/sbin:/usr/sbin:$TOOLS/bin \
    $TOOLS/bin/bash -c /src/bld-sdk.sh

echo "  [.] SDK phase 2"
sudo chroot "$LFS" $TOOLS/bin/env -i \
    HOME=/root TERM="$TERM" PS1='\u:\w\$ ' \
    PATH=/bin:/usr/bin:/sbin:/usr/sbin:$TOOLS/bin \
    /bin/bash -c /src/bld-sdk2.sh

echo "  [.] SDK phase 3"
sudo chroot "$LFS" $TOOLS/bin/env -i \
    HOME=/root TERM=$TERM PS1='\u:\w\$ ' \
    PATH=/bin:/usr/bin:/sbin:/usr/sbin \
    $TOOLS/bin/bash -c /src/bld-sdk3.sh

echo "  [.] Cleaning SDK environment"
sudo mv $LFS$TOOLS $LFS/..
sudo umount $LFS/dev/pts $LFS/dev/shm $LFS/dev $LFS/proc $LFS/sys
sudo mv $LFS/src $LFS/../src2

