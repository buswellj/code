#!/bin/bash

SNLBLD=`cat $SNBLD/.lastbuild`
export SNLBLD

cd $SNBLD/source32-$SNBN

# save some bandwidth

if [ "$SNLBLD" ]; then
 mv $SNBLD/$SNLBLD/*.bz2 .
 mv $SNBLD/$SNLBLD/*.gz .
fi

echo ""
echo "Fetching IBE sources"
echo "The sources will be in $SNBLD/source-$SNBN"
echo ""

wget http://www.linuxfromscratch.org/lfs/view/development/chapter03/packages.html
mv packages.html ibe-$SNVER_IBE.html
cat ibe-$SNVER_IBE.html | grep Download: -A 1 | grep -v Download | grep \" | cut -d \" -f 2 > ibe-$SNVER_IBE.prelist
cat ibe-$SNVER_IBE.prelist | sed 's/http:\/\/ftp.gnu/ftp:\/\/ftp.gnu/' > ibe-$SNVER_IBE.list
for i in $(cat ibe-$SNVER_IBE.list); do wget -nc -t 2 $i; done

# get file
wget http://ftp.osuosl.org/pub/lfs/lfs-packages/development/file-4.23.tar.gz

mkdir patches
cd patches
wget http://www.linuxfromscratch.org/lfs/view/development/chapter03/patches.html
mv patches.html ibe-$SNVER_IBE-patches.html
cat ibe-$SNVER_IBE-patches.html | grep Download: -A 1 | grep -v Download | grep \" | cut -d \" -f 2 > ibe-$SNVER_IBE.patchlist
for i in $(cat ibe-$SNVER_IBE.patchlist); do wget $i; done

cd $SNBLD/source-$SNBN
mkdir list
mv ibe-$SNVER_IBE.list list
mv patches/ibe-$SNVER_IBE.patchlist list

cd $SNBLD
echo "source32-$SNBN" > $SNBLD/.lastbuild
