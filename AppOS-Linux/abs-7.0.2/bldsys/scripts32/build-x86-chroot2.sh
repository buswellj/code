#!/bin/bash

# Set ABS specific variables

LSR=/bldsys/src
SNL=/bldsys/build-x86-c2.log
SNEL=/bldsys/build-x86-e2.log
TC="tar xvf"
PTC="patch -Np1 -i"
export LSR SNL SNEL TC PTC

LSP=$LSR/patches
export LSP

echo " => bzip2 " >> $SNL
echo " => bzip2 " >> $SNEL

bzip_p4.sh 1>>$SNL 2>>$SNEL

echo " => diffutils " >> $SNL
echo " => diffutils " >> $SNEL

diffutils_p4.sh 1>>$SNL 2>>$SNEL

echo " => file " >> $SNL
echo " => file " >> $SNEL

file_p4.sh 1>>$SNL 2>>$SNEL

echo " => findutils " >> $SNL
echo " => findutils " >> $SNEL

findutils_p4.sh 1>>$SNL 2>>$SNEL

echo " => flex " >> $SNL
echo " => flex " >> $SNEL

flex_p4.sh 1>>$SNL 2>>$SNEL

echo " => grub " >> $SNL
echo " => grub " >> $SNEL

grub_p4.sh 1>>$SNL 2>>$SNEL

echo " => gawk " >> $SNL
echo " => gawk " >> $SNEL

gawk_p4.sh 1>>$SNL 2>>$SNEL

echo " => gettext " >> $SNL
echo " => gettext " >> $SNEL

gettext_p4.sh 1>>$SNL 2>>$SNEL

echo " => grep " >> $SNL
echo " => grep " >> $SNEL

grep_p4.sh 1>>$SNL 2>>$SNEL

echo " => iproute2 " >> $SNL
echo " => iproute2 " >> $SNEL

iproute2_p4.sh 1>>$SNL 2>>$SNEL

echo " => kbd " >> $SNL
echo " => kbd " >> $SNEL

kbd_p4.sh 1>>$SNL 2>>$SNEL

echo " => less " >> $SNL
echo " => less " >> $SNEL

less_p4.sh 1>>$SNL 2>>$SNEL

echo " => make " >> $SNL
echo " => make " >> $SNEL

make_p4.sh 1>>$SNL 2>>$SNEL

echo " => man-db " >> $SNL
echo " => man-db " >> $SNEL

man-db_p4.sh 1>>$SNL 2>>$SNEL

echo " => module-init-tools " >> $SNL
echo " => module-init-tools " >> $SNEL

module-init-tools_p4.sh 1>>$SNL 2>>$SNEL

echo " => patch " >> $SNL
echo " => patch " >> $SNEL

patch_p4.sh 1>>$SNL 2>>$SNEL

echo " => psmisc " >> $SNL
echo " => psmisc " >> $SNEL

psmisc_p4.sh 1>>$SNL 2>>$SNEL

echo " => shadow " >> $SNL
echo " => shadow " >> $SNEL

shadow_p4.sh 1>>$SNL 2>>$SNEL

echo " => sysklogd " >> $SNL
echo " => sysklogd " >> $SNEL

sysklogd_p4.sh 1>>$SNL 2>>$SNEL

echo " => sysvinit " >> $SNL
echo " => sysvinit " >> $SNEL

sysvinit_p4.sh 1>>$SNL 2>>$SNEL

echo " => tar " >> $SNL
echo " => tar " >> $SNEL

tar_p4.sh 1>>$SNL 2>>$SNEL

echo " => texinfo " >> $SNL
echo " => texinfo " >> $SNEL

texinfo_p4.sh 1>>$SNL 2>>$SNEL

echo " => util-linux " >> $SNL
echo " => util-linux " >> $SNEL

util-linux-ng_p4.sh 1>>$SNL 2>>$SNEL

echo " => AppOS(R) IBE chroot phase 2 completed " >> $SNL
echo " => AppOS(R) IBE chroot phase 2 completed " >> $SNEL

logout
