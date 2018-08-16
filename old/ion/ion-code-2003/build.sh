#!/bin/bash
#
# Ion Linux Build Script 
# version 1.0
#
# Copyright (C) 2002-2003 Spliced Networks
#
# Authors:	John Buswell	(johnb@splicednetworks.com)
#
# Created:	2003-06-21	Initial Ion build script
#
# Modified:	2003-06-22	- Modified script for non-static toolchain
#		2003-08-19	- Made logging dynamic based on username
#		2003-08-20	- Fixed bugs
#               2003-08-28      - Automated chroot stages of the build
#                               - Added support to retrieve chroot logs
#               2003-08-29      - Added warning for host, and final timestamp
#				- Added buildver.rc for BINUTILS_VER
#               2003-08-31      - Clean up code
#
# note $1 = release $2 = maintenance $3 = patch
# insert some check here for valid version
#
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
#
# Description:  This script sets up the initial build environment
#		for a new Ion Linux. It creates a new release build
#		user and su's to that user. This triggers the second
#		stage of the build which is done as ion{version}. It
#		should **NOT** be necessary to modify this file ver
#		often. This needs to be run as root.
#



# First set the ION environment variable

ION_NAME="ion"
ION_SCR_VER=1.0.0
ION=/ion/release/$1.$2/$3
ION_SRC=/ion/src/$1.$2/$3
ION_BLD=/ion/bld/$1.$2/$3
ION_LOG=/ion/release/logs/ion-$1.$2.$3.log

echo ""
echo "Ion Linux Build Script v$ION_SCR_VER"
echo "Copyright (C) 2002-2003 Spliced Networks"
echo ""
echo "Building Ion Linux $1.$2.$3"
echo ""
echo "Build  = $ION_BLD"
echo "Source = $ION_SRC"
echo "Target = $ION"
echo ""

# create static directories

echo "" > $ION_LOG
echo "Ion Linux Build $1.$2.$3" >> $ION_LOG
echo "" >> $ION_LOG
echo "" >> $ION_LOG

echo "Build starting.." >> $ION_LOG
echo "" >> $ION_LOG
date >> $ION_LOG

echo "-> Creating initial directories"
echo ""

echo "   -> $ION/shared"
mkdir -p $ION/shared
echo "   -> $ION_BLD"
mkdir -p $ION_BLD

echo ""

# create release user

echo "-> Creating release user ion$1$2$3"
echo ""

ION_USER=`echo $ION_NAME$1$2$3`

useradd -d $ION/shared -s /ion/buildsh -m $ION_USER 

echo "-> Enter password for $ION_USER below"
echo ""

passwd $ION_USER

echo "-> Setting ownership of shared directory to $ION_USER"
echo ""

chown $ION_USER $ION/shared
chown $ION_USER $ION_SRC
chown $ION_USER $ION_BLD

echo "-> Removing previous shared directory symlink"

rm /shared

echo "-> Symlinking shared directory"

ln -s $ION/shared /

echo "-> Writing temporary files"
echo ""

echo $1 > /tmp/.ion_ver.rel
echo $2 > /tmp/.ion_ver.maint
echo $3 > /tmp/.ion_ver.patch

chmod 777 /tmp/.ion*

echo "-> Creating Log File"
echo ""

touch /ion/release/logs/bld/ion-bld-$ION_USER.log
chown $ION_USER /ion/release/logs/bld/ion-bld-$ION_USER.log

echo "-> Creating Build Shell"
echo ""

rm -rf /ion/buildsh
cat > /ion/buildsh << "EOF"
#!/bin/bash

/ion/build-stage-2.sh > /ion/release/logs/bld/ion-bld-$USER.log 2>&1

EOF

chmod 755 /ion/buildsh

echo "-> Switching to $ION_USER"
echo ""


echo "-> Entering stage 2"
echo ""

echo "" >> $ION_LOG
echo "Build stage 2.. " >> $ION_LOG
echo "" >> $ION_LOG
date >> $ION_LOG


su - $ION_USER

#exit

echo "-> Entering stage 3"
echo ""

echo "" >> $ION_LOG
echo "Build stage 3.. " >> $ION_LOG
echo "" >> $ION_LOG
date >> $ION_LOG

#source /ion/buildver.rc

mkdir $ION/shared/src
cp -a $ION_SRC/* $ION/shared/src
cp /ion/buildver.rc $ION/shared/src
mkdir $ION/shared/src/bld
cp -a $ION_BLD/binutils-build $ION/shared/src/bld
#cp -a $ION_BLD/binutils-$ION_BINUTILS_VER $ION/shared/src/bld
cp /ion/build-chroot*.sh /shared/src

chroot $ION /shared/bin/env -i \
 HOME=/root TERM=$TERM PS1='\u:\w\$ ' \
 PATH=/bin:/usr/bin:/sbin:/usr/sbin:/shared/bin \
 /shared/bin/bash --login -c /shared/src/build-chroot-stage-1.sh

# now we relogin

echo "$1.$2.$3" > $ION/.ion_version
cat /ion/ion_motd.txt | sed 's/X.Y.Z/'$1'.'$2'.'$3'/g' > $ION/.ion_motd

chroot $ION /shared/bin/env -i \
 HOME=/root TERM=$TERM PS1='\u:\w\$ ' \
 PATH=/bin:/usr/bin:/sbin:/usr/sbin:/shared/bin \
 /bin/bash --login -c /shared/src/build-chroot-stage-2.sh

# now we retrieve the logs

cat $ION/.ion_bld_1.log >> /ion/release/logs/bld/ion-bld-$ION_USER.log
cat $ION/.ion_bld_2.log >> /ion/release/logs/bld/ion-bld-$ION_USER.log

# clean up

rm -rf $ION/.ion_bld_1.log
rm -rf $ION/.ion_bld_2.log

echo ""
echo "!!! WARNING !!!"
echo ""
echo "Ion Linux does not contain the HOST command"
echo "This is added by bind, if you are building a product"
echo "WITHOUT bind, then you need to build the host command"
echo "from http://www.weird.com/ftp/pub/local/host.tar.gz"
echo ""

echo "" >> $ION_LOG
echo "Build Completed" >> $ION_LOG
date >> $ION_LOG
echo "" >> $ION_LOG

rm /shared

