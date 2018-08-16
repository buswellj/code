#!/bin/bash
ACV="2.5.1"
ARC=".tar.gz"
APN="glibc"
export ACV ARC APN
ACB=$APN-$ACV
export ACB
###################################
#
#  Glibc-2.5.1
#
###################################
cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB
$TC $LSR/$APN-libidn-$ACV$ARC
mv -v $ACB$ARC libdn
$PTC $LSP/glibc-2.5.1-ssp_hp-timing-1.patch
$PTC $LSP/glibc-2.5.1-wur-1.patch
sed -i.orig '/vi_VN.TCVN/d' localedata/SUPPORTED
sed \
's|libs -o|libs -L/usr/lib -Wl,-dynamic-linker=/lib/ld-linux.so.2 -o|' \
        -i.orig scripts/test-installation.pl
sed 's|@BASH@|/bin/bash|' -i.orig elf/ldd.bash.in
sed '/^install.*pt_chown/d' -i.orig login/Makefile
$PTC $LSP/glibc-2.5.1-iconvconfig_trampoline-1.patch
$PTC $LSP/glibc-2.5.1-localedef_trampoline-1.patch
$PTC $LSP/glibc-2.5.1-pt_pax-1.patch
$PTC $LSP/glibc-2.5.1-arc4_prng-2.patch
$PTC $LSP/glibc-2.5.1-strlcpy_strlcat-1.patch
$PTC $LSP/glibc-2.5.1-asprintf_reset2null-1.patch
$PTC $LSP/glibc-2.5.1-issetugid-1.patch
sed 's/#define UNSECURE_ENVVARS.*/&\
  "MUDFLAP_OPTIONS\\0" \\/' -i.orig sysdeps/generic/unsecvars.h
sed 's/-nostdlib/& -fno-stack-protector/g' -i.orig configure
sed 's/fstack-protector/&-all/' -i.orig nscd/Makefile
sed 's/CFLAGS-ldconfig.c =/& -fno-PIC -fno-PIE/' \
    -i.orig elf/Makefile
mkdir -v ../glibc-build
cd ../glibc-build
../$ACB/configure --prefix=/usr \
    --libexecdir=/usr/lib/glibc --enable-kernel=2.6.0 \
    --enable-stackguard-randomization --enable-bind-now \
    --disable-profile --enable-add-ons --with-prng-device=/dev/erandom
cat > configparms << "EOF"
build-programs=no
CC = gcc -fPIC -fno-stack-protector -U_FORTIFY_SOURCE -nonow -nopie
CXX = g++ -fPIC -fno-stack-protector -U_FORTIFY_SOURCE -nonow -nopie
EOF
make
cat > configparms << "EOF"
CC = gcc -fPIE -fstack-protector-all -D_FORTIFY_SOURCE=2
CXX = g++ -fPIE -fstack-protector-all -D_FORTIFY_SOURCE=2
CFLAGS-sln.c += -fno-PIC -fno-PIE
+link = $(CC) -nostdlib -nostartfiles -fPIE -pie -o $@ \
 $(sysdep-LDFLAGS) $(config-LDFLAGS) $(LDFLAGS) $(LDFLAGS-$(@F)) \
 -Wl,-z,combreloc -Wl,-z,relro -Wl,-z,now $(hashstyle-LDFLAGS) \
 $(addprefix $(csu-objpfx),S$(start-installed-name)) \
 $(+preinit) `$(CC) --print-file-name=crtbeginS.o` \
 $(filter-out $(addprefix $(csu-objpfx),start.o \
  $(start-installed-name))\
 $(+preinit) $(link-extra-libs) \
 $(common-objpfx)libc% $(+postinit),$^) \
 $(link-extra-libs) $(link-libc) `$(CC) --print-file-name=crtendS.o` $(+postinit)
EOF
make
cat > configparms << "EOF"
CC = gcc -fPIC -fno-stack-protector -U_FORTIFY_SOURCE -nonow -nopie
CXX = g++ -fPIC -fno-stack-protector -U_FORTIFY_SOURCE -nonow -nopie
EOF
#make -k check 2>&1 | tee glibc-check-log
#grep -n Error glibc-check-log
touch /etc/ld.so.conf
make install
install -v -m0644 ../glibc-2.5.1/manual/{arc4random,mkstemps}.3 \
    /usr/share/man/man3/
install -v -m0644 ../glibc-2.5.1/manual/issetugid.3 \
    /usr/share/man/man3/
install -vd /usr/lib/static/
mv -v /usr/lib/{libbsd-compat,libg,libieee,libmcheck}.a /usr/lib/static/
mv -v /usr/lib/{libBrokenLocale,libanl,libcrypt}.a /usr/lib/static/
mv -v /usr/lib/{libm,libnsl,libpthread,libresolv}.a /usr/lib/static/
mv -v /usr/lib/{librpcsvc,librt,libutil}.a /usr/lib/static/
make localedata/install-locales
install -d /usr/lib/locale
localedef -i de_DE -f ISO-8859-1 de_DE
localedef -i de_DE@euro -f ISO-8859-15 de_DE@euro
localedef -i en_HK -f ISO-8859-1 en_HK
localedef -i en_PH -f ISO-8859-1 en_PH
localedef -i en_US -f ISO-8859-1 en_US
localedef -i es_MX -f ISO-8859-1 es_MX
localedef -i fa_IR -f UTF-8 fa_IR
localedef -i fr_FR -f ISO-8859-1 fr_FR
localedef -i fr_FR@euro -f ISO-8859-15 fr_FR@euro
localedef -i it_IT -f ISO-8859-1 it_IT
localedef -i ja_JP -f EUC-JP ja_JP
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
echo "/usr/local/lib" > ld.so.conf.new
install -m644 ld.so.conf.new /etc/ld.so.conf


mv -v /tools/bin/{ld,ld-old}
mv -v /tools/$(gcc -dumpmachine)/bin/{ld,ld-old}
mv -v /tools/bin/{ld-new,ld}
ln -sv /tools/bin/ld /tools/$(gcc -dumpmachine)/bin/ld

gcc -dumpspecs | perl -p -e 's@/tools/lib/@/lib/@g;' \
    -e 's@\*startfile_prefix_spec:\n@$_/usr/lib/ @g;' > \
    `dirname $(gcc --print-libgcc-file-name)`/specs


