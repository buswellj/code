#!/bin/bash

source /ion/installer/src/build-variables

# A function to display usuage, less lines of code ;)
function_usage ()
{
 echo
 echo " Usage: build-installer.sh X Y Z"
 echo "    ex: build-installer.sh 0 1 9"
 echo
}

# Check to make sure the cmdline has $1 $2 $3
if [ -z $X ]; then
 echo; echo " Major(X) variable is empty"
 function_usage
 exit
else
 if [ -z $Y ]; then
  echo; echo " Minor(Y) variable is empty"
  function_usage
  exit
 else
  if [ -z $Z ]; then
   echo; echo " Patch(Z) variable is empty"
   function_usage
   exit
  fi
 fi
fi


if [ ! -e $INSTALLER_SRC ]; then
 echo "Sources not found, can't continue"
 exit 0
fi
[ ! -e $INSTALLER_BLD ] && mkdir $INSTALLER_BLD
[ ! -e $INSTALLER_RELEASE ] && mkdir $INSTALLER_RELEASE

mkdir -p $INSTALLER_RELEASE/$X.$Y/$Z/{bin,sbin,lib,dev,proc,scripts,cdrom,ion}
mkdir -p $INSTALLER_RELEASE/$X.$Y/$Z/{usr/bin,usr/sbin,var/state,var/log}
mkdir -p $INSTALLER_RELEASE/$X.$Y/$Z/{etc/init.d,etc/ion/files,etc/ion/screens,etc/terminfo/1}
mkdir -p $INSTALLER_BLD/$X.$Y/$Z/{gnu,opensrc}

# GNU
cd $INSTALLER_BLD/$X.$Y/$Z/gnu
tar zxf $INSTALLER_SRC/gnu/bash-$ION_BASH_VER.tar.gz
tar zxf $INSTALLER_SRC/gnu/findutils-$ION_FINDUTILS_VER.tar.gz
tar zxf $INSTALLER_SRC/gnu/nano-$ION_NANO_VER.tar.gz
tar zxf $INSTALLER_SRC/gnu/parted-$ION_PARTED_VER.tar.gz
tar zxf $INSTALLER_SRC/gnu/sed-$ION_SED_VER.tar.gz
tar zxf $INSTALLER_SRC/gnu/tar-$ION_TAR_VER.tar.gz
tar zxf $INSTALLER_SRC/gnu/eject-$ION_EJECT_VER.tar.gz
chown root.root -R $INSTALLER_BLD/$X.$Y/$Z/gnu/

cd $INSTALLER_BLD/$X.$Y/$Z/gnu/bash-$ION_BASH_VER
./configure
make CC=/opt/gcc-2.95.3/bin/gcc
cp bash $INSTALLER_RELEASE/$X.$Y/$Z/bin

cd $INSTALLER_BLD/$X.$Y/$Z/gnu/findutils-$ION_FINDUTILS_VER
patch -Np1 -i $INSTALLER_PATCHES/findutils-4.1.patch
patch -Np1 -i $INSTALLER_PATCHES/findutils-4.1-segfault.patch
./configure --prefix=/usr
make CC=/opt/gcc-2.95.3/bin/gcc
cp find/find $INSTALLER_RELEASE/$X.$Y/$Z/usr/bin

cd $INSTALLER_BLD/$X.$Y/$Z/gnu/nano-$ION_NANO_VER
./configure
make CC=/opt/gcc-2.95.3/bin/gcc
cp nano $INSTALLER_RELEASE/$X.$Y/$Z/usr/bin

cd $INSTALLER_BLD/$X.$Y/$Z/gnu/parted-$ION_PARTED_VER
./configure --without-readline --disable-nls --disable-shared
make CC=/opt/gcc-2.95.3/bin/gcc
cp parted/parted $INSTALLER_RELEASE/$X.$Y/$Z/usr/sbin

cd $INSTALLER_BLD/$X.$Y/$Z/gnu/sed-$ION_SED_VER
./configure
make
cp sed/sed $INSTALLER_RELEASE/$X.$Y/$Z/bin

cd $INSTALLER_BLD/$X.$Y/$Z/gnu/tar-$ION_TAR_VER
patch -Np1 -i $INSTALLER_PATCHES/tar-1.13.patch
./configure
make CC=/opt/gcc-2.95.3/bin/gcc
cp src/tar $INSTALLER_RELEASE/$X.$Y/$Z/bin

cd $INSTALLER_BLD/$X.$Y/$Z/gnu/eject-$ION_EJECT_VER
./configure
cat Makefile | sed -e 's/\/usr\/src\/linux/\/usr\/src\/linux-2.4.21/' | sed -e 's/\/usr\/src\/linux\/include/\/usr\/src\/linux-2.4.21\/include/' > Makefile
make
cp eject $INSTALLER_RELEASE/$X.$Y/$Z/bin

# Opensrc

cd $INSTALLER_BLD/$X.$Y/$Z/opensrc
tar zxf $INSTALLER_SRC/opensrc/e2fsprogs-$ION_E2FSPROGS_VER.tar.gz
tar zxf $INSTALLER_SRC/opensrc/lilo-$ION_LILO_VER.tar.gz
tar zxf $INSTALLER_SRC/opensrc/util-linux-$ION_UTIL_LINUX_VER.tar.gz
tar jxf $INSTALLER_SRC/opensrc/busybox-$ION_BUSYBOX_VER.tar.bz2
chown root.root -R $INSTALLER_BLD/$X.$Y/$Z/opensrc/

cd $INSTALLER_BLD/$X.$Y/$Z/opensrc/e2fsprogs-$ION_E2FSPROGS_VER
./configure
make CC=/opt/gcc-2.95.3/bin/gcc
cp misc/tune2fs $INSTALLER_RELEASE/$X.$Y/$Z/sbin
cp misc/mke2fs $INSTALLER_RELEASE/$X.$Y/$Z/sbin

cd $INSTALLER_BLD/$X.$Y/$Z/opensrc/lilo-$ION_LILO_VER
make CC=/opt/gcc-2.95.3/bin/gcc
cp lilo $INSTALLER_RELEASE/$X.$Y/$Z/sbin

cd $INSTALLER_BLD/$X.$Y/$Z/opensrc/util-linux-$ION_UTIL_LINUX_VER
./configure
make CC=/opt/gcc-2.95.3/bin/gcc
cp fdisk/fdisk $INSTALLER_RELEASE/$X.$Y/$Z/sbin
cp fdisk/sfdisk $INSTALLER_RELEASE/$X.$Y/$Z/sbin
cp fdisk/cfdisk $INSTALLER_RELEASE/$X.$Y/$Z/sbin

cd $INSTALLER_BLD/$X.$Y/$Z/opensrc/busybox-$ION_BUSYBOX_VER
cp $INSTALLER_CFG/busybox-0.60.5-Config.h ./Config.h
make CC=/opt/gcc-2.95.3/bin/gcc
export LC_ALL=POSIX
export LC_CTYPE=POSIX
prefix=$INSTALLER_RELEASE/$X.$Y/$Z
linkopts="-f"
h=`sort busybox.links | uniq`
rm -f $prefix/bin/busybox || exit 1
mkdir -p $prefix/bin || exit 1
install -m 755 busybox $prefix/bin/busybox || exit 1
for i in $h ; do
 appdir=`dirname $i`
 mkdir -p $prefix/$appdir || exit 1
 bb_path="$prefix/bin/busybox"
 echo "  $prefix$i -> $bb_path"
 ln $linkopts $bb_path $prefix$i || exit 1
done

cd $INSTALLER_RELEASE/$X.$Y/$Z/lib
cp /lib/libc-2.3.2.so .
cp /lib/ld-2.3.2.so .
cp /lib/libncurses.so.5.3 .
cp /lib/libdl-2.3.2.so .
cp /lib/libuuid.so.1.2 .
cp /usr/lib/libreadline.so.4.3 .
ln -s libc-2.3.2.so libc.so.6
ln -s ld-2.3.2.so ld-linux.so.2
ln -s libncurses.so.5.3 libncurses.so.5
ln -s libdl-2.3.2.so libdl.so.2
ln -s libuuid.so.1.2 libuuid.so.1
ln -s libreadline.so.4.3 libreadline.so.4

cd $INSTALLER_SRC/misc
tar cpf - * | (cd $INSTALLER_RELEASE/$X.$Y/$Z ; tar xpf -)

# Update symlink for initrd
rm -f /ion/installer/initrd
ln -s $INSTALLER_RELEASE/$X.$Y/$Z /ion/installer/initrd

echo "Installer Version $X.$Y.$Z" > $INSTALLER_RELEASE/$X.$Y/$Z/version
