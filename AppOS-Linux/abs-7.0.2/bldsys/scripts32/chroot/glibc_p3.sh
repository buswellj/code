#!/tools/bin/bash
ACV="2.7"
ARC=".tar.bz2"
APN="glibc"
export ACV ARC APN
ACB=$APN-$ACV
export ACB
#
# Glibc-2.7
#
##########################################
cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB
$TC $LSR/glibc-libidn-$ACV.tar.bz2
mv glibc-libidn-$ACV libidn
sed -i '/vi_VN.TCVN/d' localedata/SUPPORTED
sed -i \
's|libs -o|libs -L/usr/lib -Wl,-dynamic-linker=/lib/ld-linux.so.2 -o|' \
        scripts/test-installation.pl
sed -i 's|@BASH@|/bin/bash|' elf/ldd.bash.in
mkdir -v ../glibc-build
cd ../glibc-build
echo "CFLAGS += -march=i486" > configparms
../$ACB/configure --prefix=/usr \
    --disable-profile --enable-add-ons \
    --enable-kernel=2.6.0 --libexecdir=/usr/lib/glibc
make
#make -k check 2>&1 | tee glibc-check-log
#grep Error glibc-check-log
touch /etc/ld.so.conf
make install
mkdir -pv /usr/lib/locale
localedef -i de_DE -f ISO-8859-1 de_DE
localedef -i de_DE@euro -f ISO-8859-15 de_DE@euro
localedef -i en_HK -f ISO-8859-1 en_HK
localedef -i en_PH -f ISO-8859-1 en_PH
localedef -i en_US -f ISO-8859-1 en_US
localedef -i en_US -f UTF-8 en_US.UTF-8
localedef -i es_MX -f ISO-8859-1 es_MX
localedef -i fa_IR -f UTF-8 fa_IR
localedef -i fr_FR -f ISO-8859-1 fr_FR
localedef -i fr_FR@euro -f ISO-8859-15 fr_FR@euro
localedef -i fr_FR -f UTF-8 fr_FR.UTF-8
localedef -i it_IT -f ISO-8859-1 it_IT
localedef -i ja_JP -f EUC-JP ja_JP
make localedata/install-locales
cat > /etc/nsswitch.conf << "EOF"
# Begin /etc/nsswitch.conf

passwd: files
group: files
shadow: files

hosts: files dns
networks: files

protocols: files
services: files
ethers: files
rpc: files

# End /etc/nsswitch.conf
EOF

#tzselect
cp -v --remove-destination /usr/share/zoneinfo/UTC \
    /etc/localtime
cat > /etc/ld.so.conf << "EOF"
# Begin /etc/ld.so.conf

/usr/local/lib
/opt/lib

# End /etc/ld.so.conf
EOF

mv -v /tools/bin/{ld,ld-old}
mv -v /tools/$(gcc -dumpmachine)/bin/{ld,ld-old}
mv -v /tools/bin/{ld-new,ld}
ln -sv /tools/bin/ld /tools/$(gcc -dumpmachine)/bin/ld
gcc -dumpspecs | sed \
    -e 's@/tools/lib/ld-linux.so.2@/lib/ld-linux.so.2@g' \
    -e '/\*startfile_prefix_spec:/{n;s@.*@/usr/lib/ @}' \
    -e '/\*cpp:/{n;s@$@ -isystem /usr/include@}' > \
    `dirname $(gcc --print-libgcc-file-name)`/specs
#echo 'main(){}' > dummy.c
#cc dummy.c -v -Wl,--verbose &> dummy.log
#readelf -l a.out | grep ': /lib'
#grep -o '/usr/lib.*/crt[1in].*succeeded' dummy.log


#grep -B1 '^ /usr/include' dummy.log
#grep 'SEARCH.*/usr/lib' dummy.log |sed 's|; |\n|g'
#grep "/lib/libc.so.6 " dummy.log
#rm -v dummy.c a.out dummy.log


