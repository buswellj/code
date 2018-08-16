#!/bin/bash
#
# Kernel Attached Operating System (KaOS)
# Platform Build System version 1.0.0
#
# Copyright (c) 2009-2011 Carbon Mountain LLC
#
# script : fetch-opensrc.sh
# purpose: this script fetches the core open source components from the Internet
#          it then generates the master source tree from those components
#

echo ""
echo "KaOS Platform Build System, version 1.0.0"
echo "Copyright (c) 2009-2011 Carbon Mountain"
echo ""
echo ""
echo "  [.] Loading environment "

PBHERE=`pwd`
PBENV=`cat ~/kaos-ws/.current`
PBNOW=`date +%s`
PBTAG="opensrc-$PBNOW"
export PBHERE PBENV PBNOW PBTAG

echo "  [.] Retrieving Open Source components "
cd ~/kaos-ws/$PBENV
mkdir -p pkg/$PBTAG
chmod a+wt pkg/$PBTAG
wget -i $PBHERE/opensrc-list -P pkg/$PBTAG
cd pkg/$PBTAG
mkdir -p patches
chmod a+wt patches
mv *.patch patches

echo "  [.] Removing unused components "
# remove unnecessary components
mkdir -p notused
chmod a+wt notused
mv lfs-bootscripts-* notused
mv sysvinit-* notused
mv vim-* notused
mv udev-config-* notused
mv udev-*-testfiles* notused

echo "  [.] Uncompressing components "
for i in $(ls *.bz2); do tar jxvf $i; done
for i in $(ls *.gz); do tar zxvf $i; done

echo "  [.] Creating Source Archive "
mkdir -p archive
mv *.bz2 archive
mv *.gz archive

echo "  [.] Renaming source directories "
mv autoconf-* autoconf
mv automake-* automake
mv bash-* bash
mv binutils-* binutils
mv bison-* bison
mv bzip2-* bzip2
mv coreutils-* coreutils
mv cracklib-* cracklib
mv dejagnu-* dejagnu
mv diffutils-* diffutils
mv e2fsprogs-* e2fsprogs
mv expect* expect
mv file-* file
mv findutils-* findutils
mv flex-* flex
mv gawk-* gawk
mv gcc-* gcc
mv gdbm-* gdbm
mv gettext-* gettext
mv glib-* glib
mv glibc-* glibc
mv gmp-* gmp
cp -a gmp gcc/gmp
mv grep-* grep
mv groff-* groff
mv grub-* grub
mv gzip-* gzip
mv iana-etc-* iana-etc
mv inetutils-* inetutils
mv iproute2-* iproute2
mv kbd-* kbd
mv less-* less
mv libpipeline-* libpipeline
mv libtool-* libtool
mv linux-* linux
mv m4-* m4
mv make-* make
mv man-db-* man-db
mv man-pages-* man-pages
mv module-init-tools-* module-init-tools
mv mpc-* mpc
cp -a mpc gcc/mpc
mv mpfr-* mpfr
cp -a mpfr gcc/mpfr
mv ncurses-* ncurses
mv patch-* patch
mv pcre-* pcre
mv perl-* perl
mv pkg-config-* pkg-config
mv psmisc-* psmisc
mv procps-* procps
mv readline-* readline
mv sed-* sed
mv shadow-* shadow
mv sysklogd-* sysklogd
mv tar-* tar
mv tcl* tcl
mv texinfo-* texinfo
mv udev-* udev
mv util-linux-* util-linux
mv xz-* xz
mv zlib-* zlib

echo ""
echo "Source Directory is $PBTAG"
echo ""
cd $PBHERE

