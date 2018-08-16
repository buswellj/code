#!/shared/bin/bash
#
#        +--------------------------------------------------+
#        |                                                  |
#        | Project  : Ion Linux chroot build script part 1  |
#        | Developer: John Buswell <johnb@ionlinux.com>     |
#        | Created  : August 6th 2003                       |
#        |                                                  |
#        +--------------------------------------------------+
#        |                                                  |
#        | Updates  :                                       |
#        |                                                  |
#        |    20-Aug-2003 - Finished script                 |
#        |    30-Aug-2003 - Added user/group for postfix    |
#	 |    01-Oct-2003 - fb: Added 'chmod 666 /dev/null  |
#        |    25-Oct-2003 - jb: Added touch fstab           |
#        |    05-Nov-2003 - jb: mount devpts                |
#        |    19-Dec-2003 - jb: bison, bash, gcc patches    |
#        |                - jb: nettools patch              |
#        |                - jb: added coreutils, replaces   |
#        |                - sh-utils, fileutils, textutils  |
#        |                                                  |
#        +--------------------------------------------------+
#        |                                                  |
#        | Copyright (c) 2002-2003 Spliced Networks         |
#        | All Rights Reserved.                             |
#        |                                                  |
#        | This script is property of Spliced Networks.     |
#        | This script may not be copied or distributed.    |
#        |                                                  |
#        +--------------------------------------------------+
#

# execute this script with:
# /shared/src/build-chroot-a.sh > /shared/src/a.log 2>&1

source /shared/src/buildver.rc

chown -R 0:0 /shared

mkdir -p /{bin,boot,dev/pts,etc/opt,home,lib,mnt,proc} &&
mkdir -p /{root,sbin,tmp,usr/local,var,opt} &&
for dirname in /usr /usr/local
    do
    mkdir $dirname/{bin,etc,include,lib,sbin,share,src}
    ln -s share/{man,doc,info} $dirname
    mkdir $dirname/share/{dict,doc,info,locale,man}
    mkdir $dirname/share/{nls,misc,terminfo,zoneinfo}
    mkdir $dirname/share/man/man{1,2,3,4,5,6,7,8}
done &&
mkdir /usr/lib/locale &&
mkdir /var/{lock,log,mail,run,spool} &&
mkdir -p /var/{tmp,opt,cache,lib/misc,local} &&
mkdir /opt/{bin,doc,include,info} &&
mkdir -p /opt/{lib,man/man{1,2,3,4,5,6,7,8}} &&
ln -s ../var/tmp /usr

chmod 0750 /root &&
chmod 1777 /tmp /var/tmp

mount proc /proc -t proc
mount devpts /dev/pts -t devpts
touch /etc/fstab
touch /etc/mtab

ln -s /shared/bin/bash /bin/bash &&
ln -s bash /bin/sh
cat > /etc/passwd << "EOF"
root:x:0:0:root:/root:/bin/bash
bin:x:1:1:bin:/bin:/bin/false
postfix:x:12:12:postfix:/dev/null:/bin/false
fcron:x:13:13::/dev/null:/bin/false
nobody:x:99:99:Nobody:/:/bin/false
sshd:x:500:500:sshd:/tmp:/bin/false
EOF

cat > /etc/group << "EOF"
root:x:0:
bin:x:1:
sys:x:2:
kmem:x:3:
tty:x:4:
tape:x:5:
daemon:x:6:
floppy:x:7:
disk:x:8:
lp:x:9:
dialout:x:10:
audio:x:11:
mail:x:12:
fcron:x:13:
nogroup:x:99:
postdrop:x:499:
sshd:x:500:
EOF

bunzip2 shared/src/patch/MAKEDEV-1.7.bz2 
cp shared/src/patch/MAKEDEV-1.7 /dev/MAKEDEV
cd /dev
chmod 754 MAKEDEV
./MAKEDEV -v generic-nopty
chmod 666 /dev/null
chmod 666 /dev/tty

ln -s /shared/bin/pwd /bin/pwd

cd /usr/src
tar jxvf /shared/src/kernel/linux-$ION_LINUXHEADERS_VER.tar.bz2

cd linux-$ION_LINUXHEADERS_VER
make mrproper
make include/linux/version.h
make symlinks

cp -HR include/asm /usr/include &&
cp -R include/asm-generic /usr/include
cp -R include/linux /usr/include
touch /usr/include/linux/autoconf.h
rm /bin/pwd

cd /tmp

ION_SRC=/shared/src/
ION_SRC_GNU=/shared/src/gnu/
ION_SRC_OPS=/shared/src/opensrc/
ION_SRC_PAT=/shared/src/patch/

tar jxvf $ION_SRC_OPS/man-pages-1.56.tar.bz2
cd man-pages-1.56
make install

cd /tmp

ln -s /shared/bin/cat /bin/cat &&
ln -s /shared/bin/perl /usr/bin/perl &&
ln -s /shared/bin/pwd /bin/pwd &&
ln -s /shared/bin/stty /bin/stty

tar jxvf $ION_SRC_GNU/glibc-$ION_GLIBC_VER.tar.bz2
cd glibc-$ION_GLIBC_VER
tar jxvf $ION_SRC_GNU/glibc-linuxthreads-$ION_GLIBC_VER.tar.bz2

patch -Np1 -i /shared/src/patch/glibc-2.3.2-sscanf.patch

touch /etc/ld.so.conf &&
mkdir ../glibc-build &&
cd ../glibc-build &&
CFLAGS="-O2 -pipe" ../glibc-$ION_GLIBC_VER/configure --prefix=/usr \
       --enable-add-ons --disable-profile --libexecdir=/usr/bin \
       --with-headers=/usr/include &&
make && make check &&
make install && make localedata/install-locales

cat > /etc/nsswitch.conf << "EOF"
# Begin /etc/nsswitch.conf

passwd: files
group: files
shadow: files

publickey: files

hosts: files dns
networks: files

protocols: db files
services: db files
ethers: db files
rpc: db files

netgroup: db files

# End /etc/nsswitch.conf
EOF

ln -sf /usr/share/zoneinfo/America/New_York /etc/localtime

cat > /etc/ld.so.conf << "EOF"
# Begin /etc/ld.so.conf

/usr/local/lib
/opt/lib

# End /etc/ld.so.conf
EOF

cd /shared/src/bld/binutils-build/ld
make INSTALL=/shared/bin/install install-data-local

SPECFILE=/shared/lib/gcc-lib/i686-pc-linux-gnu/*/specs &&
cp ${SPECFILE} ./XX && 
sed 's@/shared/lib/ld-linux.so.2@/lib/ld-linux.so.2@g' ./XX > ${SPECFILE} &&
unset SPECFILE &&
rm -f ./XX

cd /tmp
tar jxvf $ION_SRC_GNU/binutils-$ION_BINUTILS_VER.tar.bz2
cd binutils-$ION_BINUTILS_VER
mkdir ../binutils-build &&
cd ../binutils-build &&
CFLAGS="-O2 -pipe" ../binutils-$ION_BINUTILS_VER/configure --prefix=/usr --enable-shared &&
make tooldir=/usr LDFLAGS="-s" &&
#make check &&
make tooldir=/usr install &&
rm /usr/lib/libiberty.a

cd /tmp
tar jxvf $ION_SRC_GNU/gcc-$ION_GCC_VER.tar.bz2
tar jxvf $ION_SRC_GNU/gcc-g++-$ION_GCC_VER.tar.bz2

cd gcc-$ION_GCC_VER
patch -Np1 -i $ION_SRC_PAT/gcc-3.3-nofixincludes.patch
patch -Np1 -i $ION_SRC_PAT/gcc-3.3-mmap_test.patch
#patch -Np1 -i $ION_SRC_PAT/gcc-$ION_GCC_VER.patch
#patch -Np1 -i $ION_SRC_PAT/gcc-3.3.2-no_fixincludes.patch
mkdir ../gcc-build &&
cd ../gcc-build &&
CFLAGS="-O2 -pipe" CXXFLAGS="-O2 -pipe" ../gcc-$ION_GCC_VER/configure --prefix=/usr \
       --enable-shared --enable-languages=c,c++ --enable-threads=posix \
       --enable-__cxa_atexit --enable-clocale=gnu &&
make LDFLAGS="-s"
#&& make -k check
make install &&
ln -s ../usr/bin/cpp /lib &&
ln -s gcc /usr/bin/cc &&
rm /usr/lib/libiberty.a

cd /tmp
tar zxvf $ION_SRC_OPS/zlib-$ION_ZLIB_VER.tar.gz
cd zlib-$ION_ZLIB_VER
CFLAGS="$CFLAGS -fPIC" \
    ./configure --prefix=/usr --shared
make LIBS="libz.so.1.1.4 libz.a"
make LIBS="libz.so.1.1.4 libz.a" install
mv /usr/lib/libz.so.* /lib
ln -sf ../../lib/libz.so.1 /usr/lib/libz.so
cp zlib.3 /usr/share/man/man3

cd /tmp
tar zxvf $ION_SRC_GNU/findutils-$ION_FINDUTILS_VER.tar.gz
cd findutils-$ION_FINDUTILS_VER
patch -Np1 -i $ION_SRC_PAT/findutils-4.1.patch
patch -Np1 -i $ION_SRC_PAT/findutils-4.1-segfault.patch
./configure --prefix=/usr
make libexecdir=/usr/bin && make libexecdir=/usr/bin install

cd /tmp
tar jxvf $ION_SRC_GNU/gawk-$ION_GAWK_VER.tar.bz2
cd gawk-$ION_GAWK_VER
patch -Np1 -i $ION_SRC_PAT/gawk.patch
./configure --prefix=/usr --libexecdir=/usr/bin
make && make install

cd /tmp
tar zxvf $ION_SRC_GNU/ncurses-$ION_NCURSES_VER.tar.gz
cd ncurses-$ION_NCURSES_VER
./configure --prefix=/usr --with-shared
make && make install
chmod 755 /usr/lib/*.5.3
mv /usr/lib/libncurses.so.5* /lib
ln -sf libncurses.a /usr/lib/libcurses.a &&
ln -sf ../../lib/libncurses.so.5 /usr/lib/libncurses.so &&
ln -sf ../../lib/libncurses.so.5 /usr/lib/libcurses.so


#cd /tmp
#tar zxvf $ION_SRC_OPS/vim-$ION_VIM_VER-src1.tar.gz
#tar zxvf $ION_SRC_OPS/vim-$ION_VIM_VER-src2.tar.gz
#cd vim62
#./configure --prefix=/usr
#make CPPFLAGS=-DSYS_VIMRC_FILE=\"/etc/vimrc\"
#make install
#ln -s vim /usr/bin/vi
#
#cat > /root/.vimrc << "EOF"
#" Begin /root/.vimrc
#
#set nocompatible
#set bs=2
#
#" End /root/.vimrc
#EOF

cd /tmp
tar zxvf $ION_SRC_GNU/m4-$ION_M4_VER.tar.gz
cd m4-$ION_M4_VER

./configure --prefix=/usr
make && make install

cd /tmp
tar jxvf $ION_SRC_GNU/bison-$ION_BISON_VER.tar.bz2
cd bison-$ION_BISON_VER
patch -Np1 -i $ION_SRC_PAT/bison.patch
./configure --prefix=/usr
make && make install

cd /tmp
tar zxvf $ION_SRC_GNU/less-$ION_LESS_VER.tar.gz
cd less-$ION_LESS_VER
./configure --prefix=/usr --bindir=/bin --sysconfdir=/etc
make && make install

cd /tmp
tar zxvf $ION_SRC_GNU/groff-$ION_GROFF_VER.tar.gz
cd groff-$ION_GROFF_VER
./configure --prefix=/usr
make && make install
ln -s soelim /usr/bin/zsoelim &&
ln -s eqn /usr/bin/geqn &&
ln -s tbl /usr/bin/gtbl

#cd /tmp
#tar jxvf $ION_SRC_GNU/textutils-$ION_TEXTUTILS_VER.tar.bz2
#cd textutils-$ION_TEXTUTILS_VER
#./configure --prefix=/usr
#make && make install
#mv /usr/bin/{cat,head} /bin

cd /tmp
tar zxvf $ION_SRC_GNU/sed-$ION_SED_VER.tar.gz
cd sed-$ION_SED_VER
./configure --prefix=/usr --bindir=/bin
make && make install

cd /tmp
tar jxvf $ION_SRC_OPS/flex-$ION_FLEX_VER.tar.bz2
cd flex-$ION_FLEX_VER
./configure --prefix=/usr
make && make install
ln -s libfl.a /usr/lib/libl.a
cat > /usr/bin/lex << "EOF"
#!/bin/sh
# Begin /usr/bin/lex

exec /usr/bin/flex -l "$@"

# End /usr/bin/lex
EOF
chmod 755 /usr/bin/lex

cd /tmp
tar jxvf $ION_SRC_GNU/coreutils-$ION_COREUTILS_VER.tar.bz2
cd coreutils-$ION_COREUTILS_VER
patch -Np1 -i $ION_SRC_PAT/coreutils-patch.1
patch -Np1 -i $ION_SRC_PAT/coreutils-patch.2
./configure --prefix=/usr --bindir=/bin
make && make install
ln -s ../../bin/install /usr/bin
mv /usr/bin/{basename,date,echo,false,pwd} /bin &&
mv /usr/bin/{sleep,stty,su,test,true,uname} /bin &&
mv /usr/bin/chroot /usr/sbin
ln -s test /bin/[
mv /usr/bin/{cat,head} /bin

#cd /tmp
#tar zxvf $ION_SRC_GNU/fileutils-$ION_FILEUTILS_VER.tar.gz
#cd fileutils-$ION_FILEUTILS_VER
#./configure --prefix=/usr --bindir=/bin
#make && make install

#ln -s ../../bin/install /usr/bin

#cd /tmp
#tar zxvf $ION_SRC_GNU/sh-utils-$ION_SH_UTILS_VER.tar.gz
#cd sh-utils-$ION_SH_UTILS_VER
#patch -Np1 -i $ION_SRC_PAT/sh-utils-2.0-hostname.patch
#./configure --prefix=/usr
#make && make install
#mv /usr/bin/{basename,date,echo,false,pwd} /bin &&
#mv /usr/bin/{sleep,stty,su,test,true,uname} /bin &&
#mv /usr/bin/chroot /usr/sbin
#ln -s test /bin/[

cd /tmp
tar zxvf $ION_SRC_GNU/gettext-$ION_GETTEXT_VER.tar.gz
cd gettext-$ION_GETTEXT_VER
./configure --prefix=/usr
make && make install

cd /tmp
tar jxvf $ION_SRC_OPS/net-tools-$ION_NETTOOLS_VER.tar.bz2
cd net-tools-$ION_NETTOOLS_VER
patch -Np1 -i $ION_SRC_PAT/nettools.patch
yes "" | make
make update

cd /tmp
tar zxvf $ION_SRC_OPS/perl-$ION_PERL_VER.tar.gz
cd perl-$ION_PERL_VER
./configure.gnu --prefix=/usr
make && make install

cd /tmp
tar jxvf $ION_SRC_GNU/texinfo-$ION_TEXINFO_VER.tar.bz2
cd texinfo-$ION_TEXINFO_VER
./configure --prefix=/usr
make && make install
make TEXMF=/usr/share/texmf install-tex

cd /tmp
tar jxvf $ION_SRC_GNU/autoconf-$ION_AUTOCONF_VER.tar.bz2
cd autoconf-$ION_AUTOCONF_VER
./configure --prefix=/usr
make install

cd /tmp
tar jxvf $ION_SRC_GNU/automake-$ION_AUTOMAKE_VER.tar.bz2
cd automake-$ION_AUTOMAKE_VER
./configure --prefix=/usr
make install
ln -s automake-1.7 /usr/share/automake

cd /tmp
tar zxvf $ION_SRC_GNU/bash-$ION_BASH_VER.tar.gz
cd bash-$ION_BASH_VER
patch -Np1 -i $ION_SRC_PAT/bash2.patch
./configure --prefix=/usr --bindir=/bin
make && make install
#exec /bin/bash --login

