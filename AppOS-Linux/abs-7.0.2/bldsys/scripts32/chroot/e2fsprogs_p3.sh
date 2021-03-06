#!/tools/bin/bash
ACV="1.40.6"
ARC=".tar.gz"
APN="e2fsprogs"
export ACV ARC APN
ACB=$APN-$ACV
export ACB
#
# E2fsprogs-1.40.6
#
##########################################
cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB
sed -i -e 's@/bin/rm@/tools&@' lib/blkid/test_probe.in
mkdir -v build
cd build
../configure --prefix=/usr --with-root-prefix="" \
    --enable-elf-shlibs

make
make install
make install-libs

