#!/bin/sh
#
#
#        +--------------------------------------------------+
#        |                                                  |
#        | Project  : Ion Linux chroot build script part 2  |
#        | Developer: John Buswell <johnb@ionlinux.com>     |
#        | Created  : August 6th 2003                       |
#        |                                                  |
#        +--------------------------------------------------+
#        |                                                  |
#        | Updates  :                                       |
#        |                                                  |
#        |    20-Aug-2003 - Finished script                 |
#        |    29-Aug-2003 - Fixed mod_init_tools prefix     |
#        |                - Added tar gcc3 patch            |
#        |                - Added linuxheaders kernel build |
#        |                - Automated release info in /etc  |
#	 |                - Added db4                       |
#        |                - Fixed iptables in /usr/local    |
#        |                - Fixed lilo                      |
#	 |                - Added kernel configs            |
#	 |    30-Aug-2003 - Added strace                    |
#        |                - Fixed db4 script problem        |
#        |                - Added iproute2 patches          |
#        |                - Umount /proc and exit at end    |
#        |                - Added postfix                   |
#        |                - Uncommented and fixed fcron     |
#        |    31-Aug-2003 - Added custom lynx html page     |
#        |                - Initial samhain addition        |
#        |                - Added gnupg, jfsutils           |
#        |    06-Sep-2003 - Added init scripts              |
#        |                - Added fb init.c patch           |
#        |                - Fixed man formatting            |
#        |                - Added compressed man pages      |
#        |    09-Sep-2003 - Fixed bashrc                    |
#        |                - fixed iproute2 patches          |
#        |    03-Oct-2003 - Moved bashrc, profile, inputrc  |
#        |                - inittab to init-misc*.tar.bz2   |
#        |    25-Oct-2003 - Added fboyd's shadow patch      |
#        |                - Fixed issue.net on failure      |
#        |                - Added pciutils                  |
#        |    28-Oct-2003 - Lilo patch                      |
#        |                - Moved init patch                |
#        |    29-Oct-2003 - Added modules.dep               |
#        |    31-Oct-2003 - Fixed modules_install           |
#        |    03-Nov-2003 - Added kmake script              |
#        |    05-Nov-2003 - Added umount devpts             |
#        |    20-Nov-2003 - Replaced inetutils with xinetd  |
#        |                - added attr and acl for fs acls  |
#        |    26-Nov-2003 - added vlantools                 |
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
#

source /shared/src/buildver.rc

ION_SRC=/shared/src/
ION_SRC_GNU=/shared/src/gnu/
ION_SRC_OPS=/shared/src/opensrc/
ION_SRC_PAT=/shared/src/patch/
ION_SRC_KER=/shared/src/kernel/

cd /tmp
tar zxvf $ION_SRC_OPS/file-$ION_FILE_VER.tar.gz
cd file-$ION_FILE_VER
./configure --prefix=/usr --datadir=/usr/share/misc
make && make install

cd /tmp
tar zxvf $ION_SRC_GNU/libtool-$ION_LIBTOOL_VER.tar.gz
cd libtool-$ION_LIBTOOL_VER
./configure --prefix=/usr
make && make install

cd /tmp
tar jxvf $ION_SRC_OPS/bin86-$ION_BIN86_VER.tar.bz2
cd bin86-$ION_BIN86_VER
make
make PREFIX=/usr install

cd /tmp
tar zxvf $ION_SRC_OPS/bzip2-$ION_BZIP2_VER.tar.gz
cd bzip2-$ION_BZIP2_VER
make -f Makefile-libbz2_so
make && make install
cp bzip2-shared /bin/bzip2 &&
cp -a libbz2.so* /lib &&
ln -s ../../lib/libbz2.so.1.0 /usr/lib/libbz2.so &&
rm /usr/bin/{bunzip2,bzcat,bzip2} &&
mv /usr/bin/{bzip2recover,bzless,bzmore} /bin &&
ln -s bzip2 /bin/bunzip2 &&
ln -s bzip2 /bin/bzcat

cd /tmp
tar zxvf $ION_SRC_GNU/ed-$ION_ED_VER.tar.gz
cd ed-$ION_ED_VER
patch -Np1 -i $ION_SRC_PAT/ed-0.2.patch
./configure --prefix=/usr
make && make install
mv /usr/bin/{ed,red} /bin

cd /tmp
tar zxvf $ION_SRC_KER/kbd-$ION_KBD_VER.tar.gz
cd kbd-$ION_KBD_VER
patch -Np1 -i $ION_SRC_PAT/kbd-$ION_KBD_VER.patch
./configure
make && make install


cd /tmp
tar zxvf $ION_SRC_GNU/diffutils-$ION_DIFFUTILS_VER.tar.gz
cd diffutils-$ION_DIFFUTILS_VER
./configure --prefix=/usr
make && make install

cd /tmp
tar zxvf $ION_SRC_OPS/e2fsprogs-$ION_E2FSPROGS_VER.tar.gz
cd e2fsprogs-$ION_E2FSPROGS_VER
mkdir ../e2fsprogs-build &&
cd ../e2fsprogs-build
../e2fsprogs-$ION_E2FSPROGS_VER/configure --prefix=/usr --with-root-prefix="" \
    --enable-elf-shlibs

make && make install
make install-libs
install-info /usr/share/info/libext2fs.info /usr/share/info/dir

cd /tmp
tar zxvf $ION_SRC_OPS/jfsutils-$ION_JFSUTILS_VER.tar.gz
cd jfsutils-$ION_JFSUTILS_VER
./configure --prefix=/
make && make install

cd /tmp
tar jxvf $ION_SRC_GNU/grep-$ION_GREP_VER.tar.bz2
cd grep-$ION_GREP_VER
./configure --prefix=/usr --bindir=/bin
make && make install


cd /tmp
tar zxvf $ION_SRC_GNU/gzip-$ION_GZIP_VER.tar.gz
cd gzip-$ION_GZIP_VER
patch -Np1 -i $ION_SRC_PAT/gzip-1.2.4b.patch
./configure --prefix=/usr
cp gzexe.in{,.backup} &&
sed 's%"BINDIR"%/bin%' gzexe.in.backup > gzexe.in
make && make install
mv /usr/bin/gzip /bin &&
rm /usr/bin/{gunzip,zcat} &&
ln -s gzip /bin/gunzip &&
ln -s gzip /bin/zcat &&
ln -s gunzip /bin/uncompress

cd /tmp
tar jxvf $ION_SRC_OPS/man-$ION_MANX_VER.tar.bz2
cd man-$ION_MANX_VER
patch -Np1 -i $ION_SRC_PAT/man-1.5k-manpath.patch
patch -Np1 -i $ION_SRC_PAT/man-1.5k-pager.patch
patch -Np1 -i $ION_SRC_PAT/man-1.5k-80cols.patch
PATH=$PATH:/usr/bin:/bin \
DEFS="-DNONLS" ./configure -default -confdir=/etc
make && make install
cd /etc
# fix man formatting..
cat man.conf | sed 's/\/usr\/bin\/nroff -Tlatin1/\/usr\/bin\/nroff -c/g' \
> man.conf
cat man.conf | sed 's/\/bin\/less -is/\/bin\/less -isr/g' \
> man.conf

cd /tmp
tar jxvf $ION_SRC_OPS/nasm-$ION_NASM_VER.tar.bz2
cd nasm-$ION_NASM_VER
./configure --prefix=/usr
make
make install


cd /tmp
tar zxvf $ION_SRC_OPS/lilo-$ION_LILO_VER.tar.gz
cd lilo-$ION_LILO_VER
patch -Np1 -i $ION_SRC_PAT/ion-lilo.patch
make all
make boot-menu.b
make boot-text.b
make boot-bmp.b
make install
cp boot-*.b /boot
cp chain.b /boot
cp os2_d.b /boot
ln -s boot-text.b /boot/boot.b

cat > /etc/lilo.conf << "EOF"
# Ion Linux
# lilo.conf
#
#
#lba32
#serial=0,9600
#read-only

boot = /dev/hda
map = /boot/.map
install = /boot/boot.b
message = /boot/message
prompt
timeout=5
delay=30
loader=/boot/chain.b
default=ion
vga=normal

image=/boot/ion-default
        label=ion
        read-only
        root=/dev/hda3
        append=""

EOF


cd /tmp
tar jxvf $ION_SRC_GNU/make-$ION_MAKE_VER.tar.bz2
cd make-$ION_MAKE_VER
./configure --prefix=/usr
make && make install

chgrp root /usr/bin/make &&
chmod 755 /usr/bin/make

# replaced by mod_init_tools
#
#cd /tmp
#tar jxvf $ION_SRC_KER/modutils-$ION_MODUTILS_VER.tar.bz2
#cd modutils-$ION_MODUTILS_VER
#./configure 
#make && make install

cd /tmp
tar zxvf $ION_SRC_OPS/netkit-base-$ION_NETKIT_VER.tar.gz
cd netkit-base-$ION_NETKIT_VER
./configure
make && make install
cp etc.sample/{services,protocols} /etc

cd /tmp
tar zxvf $ION_SRC_GNU/patch-$ION_PATCH_VER.tar.gz
cd patch-$ION_PATCH_VER
CPPFLAGS=-D_GNU_SOURCE ./configure --prefix=/usr
make && make install

cd /tmp
tar zxvf $ION_SRC_OPS/procinfo-$ION_PROCINFO_VER.tar.gz
cd procinfo-$ION_PROCINFO_VER
make LDLIBS=-lncurses
make install

cd /tmp
tar zxvf $ION_SRC_OPS/procps-$ION_PROCPS_VER.tar.gz
cd procps-$ION_PROCPS_VER
patch -Np1 -i $ION_SRC_PAT/procps-3.1.144.patch
make
make XSCPT="" install

cd /tmp
tar zxvf $ION_SRC_OPS/psmisc-$ION_PSMISC_VER.tar.gz
cd psmisc-$ION_PSMISC_VER
./configure --prefix=/usr --exec-prefix=/
make
make install
ln -s killall /bin/pidof

cd /tmp
tar jxvf $ION_SRC_OPS/shadow-$ION_SHADOW_VER.tar.bz2
cd shadow-$ION_SHADOW_VER
patch -Np1 -i $ION_SRC_PAT/shadow-$ION_SHADOW_VER-useradd.c-patch
patch -Np1 -i $ION_SRC_PAT/shadow-$ION_SHADOW_VER-newgrp-fix.patch
./configure --prefix=/usr --libdir=/usr/lib \
    --enable-shared
make && make install
cp etc/{limits,login.access} /etc
rm /bin/vipw
ln -s vipw /usr/sbin/vigr
mv /bin/sg /usr/bin
mv /usr/lib/lib{shadow,misc}.so.0* /lib
ln -sf ../../lib/libshadow.so.0 /usr/lib/libshadow.so &&
ln -sf ../../lib/libmisc.so.0 /usr/lib/libmisc.so
/usr/sbin/pwconv

cd /tmp
tar zxvf $ION_SRC_OPS/sysklogd-$ION_SYSKLOGD_VER.tar.gz
cd sysklogd-$ION_SYSKLOGD_VER
make && make install

cat > /etc/syslog.conf << "EOF"
# Begin /etc/syslog.conf

auth,authpriv.* -/var/log/auth.log
*.*;auth,authpriv.none -/var/log/sys.log
daemon.* -/var/log/daemon.log
kern.* -/var/log/kern.log
mail.* -/var/log/mail.log
user.* -/var/log/user.log
*.emerg *

# End /etc/syslog.conf
EOF

cd /tmp
tar zxvf $ION_SRC_OPS/sysvinit-$ION_SYSVINIT_VER.tar.gz
cd sysvinit-$ION_SYSVINIT_VER
patch -Np1 -i $ION_SRC_PAT/init.c-patch
cp src/init.c{,.backup} &&
sed 's/Sending processes/Sending processes started by init/g' \
    src/init.c.backup > src/init.c
make -C src
make -C src install

# inittab was here, moved to init-misc*.tar.bz2

cd /tmp
tar jxvf $ION_SRC_OPS/init-misc-$ION_INIT_SCRIPTS_VER.tar.bz2
cd init-misc-$ION_INIT_SCRIPTS_VER
./install.sh

# tar bombed
cd /tmp
tar zxvf $ION_SRC_GNU/tar-$ION_TAR_VER.tar.gz
cd tar-$ION_TAR_VER
patch -Np1 -i $ION_SRC_PAT/tar-1.13.patch
patch -Np1 -i $ION_SRC_PAT/tar-1.13-gcc3.patch
./configure --prefix=/usr --bindir=/bin \
    --libexecdir=/usr/bin
make &&  make install

cd /tmp
tar jxvf $ION_SRC_KER/util-linux-$ION_UTIL_LINUX_VER.tar.bz2
cd util-linux-$ION_UTIL_LINUX_VER
cp hwclock/hwclock.c{,.backup} &&
sed 's%etc/adjtime%var/lib/hwclock/adjtime%' \
    hwclock/hwclock.c.backup > hwclock/hwclock.c &&
mkdir -p /var/lib/hwclock
./configure
make HAVE_SLN=yes
make HAVE_SLN=yes install

cd /tmp
tar zxvf $ION_SRC_GNU/gcc-$ION_GCCOPT_VER.tar.gz
cd gcc-$ION_GCCOPT_VER
patch -Np1 -i $ION_SRC_PAT/gcc-$ION_GCCOPT_VER-2.patch
echo timestamp > gcc/cstamp-h.in
mkdir ../gcc-2-build
cd ../gcc-2-build
../gcc-$ION_GCCOPT_VER/configure --prefix=/opt/gcc-$ION_GCCOPT_VER \
 --enable-shared --enable-languages=c
make bootstrap && make install


# seperate kernel headers for now

cd /usr/src
ln -s linux-$ION_LINUXHEADERS_VER linux
cd linux
make CC=/opt/gcc-2.95.3/bin/gcc clean
cp $ION_SRC_KER/default-config-2.4 .config
make CC=/opt/gcc-2.95.3/bin/gcc oldconfig
make CC=/opt/gcc-2.95.3/bin/gcc dep
make CC=/opt/gcc-2.95.3/bin/gcc bzImage
make CC=/opt/gcc-2.95.3/bin/gcc modules

cd /usr/src
tar jxvf $ION_SRC_KER/linux-$ION_LINUX_VER.tar.bz2
cd linux-$ION_LINUX_VER
make mrproper 
patch -Np1 -i $ION_SRC_PAT/ion-printk.patch
cp $ION_SRC_KER/default-config-2.6 .config
make CC=/opt/gcc-2.95.3/bin/gcc oldconfig
make CC=/opt/gcc-2.95.3/bin/gcc bzImage
make CC=/opt/gcc-2.95.3/bin/gcc modules
cp arch/i386/boot/bzImage /boot/ion-default
cp System.map /boot/System.map-ion-default
rm /boot/System.map
ln -s /boot/System.map-ion-default /boot/System.map

cd /usr/src/linux-$ION_LINUX_VER
make mandocs
cp -a Documentation/man /usr/share/man/man9

cat > /usr/sbin/ionbk << "EOF"
#
# Ion Linux Kernel Build Script
#
# Copyright (c) 2003 Spliced Networks
#
# Usage: ionbk image-name
#

make CC=/opt/gcc-2.95.3/bin/gcc bzImage
make CC=/opt/gcc-2.95.3/bin/gcc modules &&
make CC=/opt/gcc-2.95.3/bin/gcc modules_install &&
cp arch/i386/boot/bzImage /boot/$1 &&
cp System.map /boot/System.map-$1
rm /boot/System.map
ln -s /boot/System.map-$1 /boot/System.map

EOF

chmod 755 /usr/sbin/ionbk

cd /tmp
tar zxvf $ION_SRC_OPS/db-$ION_DB_VER.tar.gz
cd db-$ION_DB_VER/build_unix
../dist/configure --prefix=/usr
make && make install

cd /tmp
tar zxvf $ION_SRC_OPS/openssl-$ION_OPENSSL_VER.tar.gz
cd openssl-$ION_OPENSSL_VER
./config --prefix=/usr threads zlib-dynamic shared 
cat Makefile | sed 's/-m486/-mcpu=i486/g' > Makefile.2
mv Makefile Makefile.old
mv Makefile.2 Makefile
make clean
make && make test && make install

cd /tmp
tar zxvf $ION_SRC_GNU/gdbm-$ION_GDBM_VER.tar.gz
cd gdbm-$ION_GDBM_VER
./configure --prefix=/usr
make && make install

cd /tmp
tar zxvf $ION_SRC_OPS/tcp_wrappers_$ION_TCP_WRAPPERS_VER.tar.gz
cd tcp_wrappers_$ION_TCP_WRAPPERS_VER
patch -Np1 -i $ION_SRC_PAT/tcp_wrappers_$ION_TCP_WRAPPERS_VER.diff
make REAL_DAEMON_DIR=/usr/sbin linux
cp libwrap.a /usr/lib &&
cp tcpd.h /usr/include &&
cp safe_finger /usr/sbin &&
cp tcpd /usr/sbin &&
cp tcpdchk /usr/sbin &&
cp tcpdmatch /usr/sbin &&
cp try-from /usr/sbin &&
cp -av *.3 /usr/share/man/man3 &&
cp -av *.5 /usr/share/man/man5 &&
cp -av *.8 /usr/share/man/man8


# notes: need to add stuff like pam, and higher security
#        stuff here.

cd /tmp
tar zxvf $ION_SRC_OPS/openssh-$ION_OPENSSH_VER.tar.gz
cd openssh-$ION_OPENSSH_VER
./configure --prefix=/usr --with-md5-passwords --with-tcp-wrappers
make && make install

cd /tmp
tar zxvf $ION_SRC_OPS/dhcpcd-$ION_DHCPCD_VER.tar.gz
cd dhcpcd-$ION_DHCPCD_VER
./configure --prefix=/usr
make && make install

cd /tmp
tar zxvf $ION_SRC_GNU/nano-$ION_NANO_VER.tar.gz
cd nano-$ION_NANO_VER
./configure --prefix=/usr
make && make install
ln -s nano /usr/bin/pico

#
# this was installed to make fcrontab work.
# it does VERY basic configuration
#
# Ion specific products need to provide more advanced
# configuration options for postfix elsewhere.
#
# this uses /usr/sbin, /etc/postfix, /usr/libexec/postfix
#
# note for over 1000 delivery processes need FD_SETSIZE
#

cd /tmp
tar zxvf $ION_SRC_OPS/postfix-$ION_POSTFIX_VER.tar.gz
cd postfix-$ION_POSTFIX_VER
make makefiles CCARGS='-DDEF_MANPAGE_DIR=\"/usr/man\"'
make

cat > /etc/aliases << "EOF"
postfix: root
EOF

make upgrade

cat /etc/postfix/main.cf \
| sed 's/\#myhostname \= host.domain.tld/myhostname \= localhost.localdomain/g' \
> /etc/postfix/main.cf

cat /etc/postfix/main.cf \
| sed 's/\#myorigin \= \$myhostname/myorigin \= \$myhostname/g' \
> /etc/postfix/main.cf

newaliases


# this needs an MTA to work
cd /tmp
tar zxvf $ION_SRC_OPS/fcron-$ION_FCRON_VER.src.tar.gz
cd fcron-$ION_FCRON_VER
./configure --prefix=/usr --with-editor=/usr/bin/nano \
--with-boot-install=no
make && make install
ln -s fcrontab /usr/bin/crontab

# this is needed for rsync
cd /tmp
tar zxvf $ION_SRC_OPS/netkit-rsh-$ION_NETKIT_RSH_VER.tar.gz
cd netkit-rsh-$ION_NETKIT_RSH_VER
./configure --without-pam --prefix=/usr
make && make install

cd /tmp
tar zxvf $ION_SRC_OPS/rsync-$ION_RSYNC_VER.tar.gz
cd rsync-$ION_RSYNC_VER
./configure --prefix=/usr
make && make install

cd /tmp
tar jxvf $ION_SRC_OPS/iptables-$ION_IPTABLES_VER.tar.bz2
cd iptables-$ION_IPTABLES_VER
cat Makefile | sed 's/PREFIX:=\/usr\/local/PREFIX:=\/usr/g' > Makefile
make KERNEL=/usr/src/linux-$ION_LINUX_VER
make install KERNEL=/usr/src/linux-$ION_LINUX_VER


cd /tmp
tar zxvf $ION_SRC_OPS/iproute2-$ION_IPROUTE2_VER.tar.gz
cd iproute2
patch -Np1 -i $ION_SRC_PAT/iproute2-2.2.4-docmake.patch
patch -Np1 -i $ION_SRC_PAT/iproute2-misc.patch
patch -Np1 -i $ION_SRC_PAT/iproute2-config.patch
patch -Np1 -i $ION_SRC_PAT/iproute2-in_port_t.patch
patch -Np1 -i $ION_SRC_PAT/iproute2-makefile.patch
patch -Np1 -i $ION_SRC_PAT/iproute2-2.4.7-crosscompile.patch
patch -Np1 -i $ION_SRC_PAT/iproute2-2.4.7-hex.patch
patch -Np1 -i $ION_SRC_PAT/iproute2-2.4.7-config.patch
patch -Np1 -i $ION_SRC_PAT/iproute2-2.4.7-htb3-tc.patch
make
cp ip/ifcfg /sbin
cp ip/ip /sbin
cp ip/routef /sbin
cp ip/routel /sbin
cp ip/rtacct /sbin
cp ip/rtmon /sbin
cp ip/rtpr /sbin
cp tc/tc /sbin
cp -a etc/iproute2 /etc
cp $ION_SRC_PAT/ip.8 /usr/man/man8

cd /tmp
tar jxvf $ION_SRC_KER/module-init-tools-$ION_MOD_INIT_TOOLS_VER.tar.bz2
cd module-init-tools-$ION_MOD_INIT_TOOLS_VER
./configure --prefix=/
make && make install

cd /usr/src/linux-$ION_LINUX_VER
make CC=/opt/gcc-2.95.3/bin/gcc modules_install
touch /lib/modules/linux-$ION_LINUX_VER/modules.dep

cd /tmp
tar zxvf $ION_SRC_GNU/readline-$ION_READLINE_VER.tar.gz
cd readline-$ION_READLINE_VER
patch -Np1 -i $ION_SRC_PAT/readline43-001
patch -Np1 -i $ION_SRC_PAT/readline43-002
patch -Np1 -i $ION_SRC_PAT/readline43-003
patch -Np1 -i $ION_SRC_PAT/readline43-004
./configure --prefix=/usr --with-curses
make && make install

#cd /tmp
#tar zxvf $ION_SRC_GNU/inetutils-$ION_INETUTILS_VER.tar.gz
#cd inetutils-$ION_INETUTILS_VER
#./configure --prefix=/usr --enable-encryption --with-wrap
#make && make install

cd /tmp
tar zxvf $ION_SRC_OPS/xinetd-$ION_XINETD_VER.tar.gz
cd xinetd-$ION_XINETD_VER
./configure --prefix=/usr
make && make install

cd /tmp
tar zxvf $ION_SRC_OPS/attr-$ION_ATTR_VER.src.tar.gz
cd attr-$ION_ATTR_VER
./configure --prefix=/usr
make && make install && make install-dev && make install-lib

cd /tmp
tar zxvf $ION_SRC_OPS/acl-$ION_ACL_VER.src.tar.gz
cd acl-$ION_ACL_VER
./configure --prefix=/usr
make && make install && make install-dev && make install-lib

cd /tmp
tar jxvf $ION_SRC_OPS/yafc-$ION_YAFC_VER.tar.bz2
cd yafc-$ION_YAFC_VER
patch -Np1 -i $ION_SRC_PAT/yafc-makepath.patch
./configure --prefix=/usr
make && make install
ln -s /usr/bin/yafc ftp

cd /tmp
tar zxvf $ION_SRC_OPS/libpcap-$ION_LIBPCAP_VER.tar.gz
cd libpcap-$ION_LIBPCAP_VER
./configure --prefix=/usr --enable-ipv6
make && make install

cd /tmp
tar zxvf $ION_SRC_OPS/tcpdump-$ION_TCPDUMP_VER.tar.gz
cd tcpdump-$ION_TCPDUMP_VER
./configure --prefix=/usr --enable-ipv6 --disable-smb

cd /tmp
tar zxvf $ION_SRC_OPS/lft-$ION_LFT_VER.tar.gz
cd lft-$ION_LFT_VER
./configure --prefix=/usr
make && make install

cd /tmp
tar zxvf $ION_SRC_GNU/jwhois-$ION_JWHOIS_VER.tar.gz
cd jwhois-$ION_JWHOIS_VER
./configure --prefix=/usr
make && make install

cd /tmp
tar zxvf $ION_SRC_GNU/screen-$ION_SCREEN_VER.tar.gz
cd screen-$ION_SCREEN_VER
./configure --prefix=/usr --enable-telnet --enable-colors256
make && make install

cd /tmp
tar zxvf $ION_SRC_OPS/hping$ION_HPING2_VER.0-rc2.tar.gz
cd hping2-rc2
./configure
make
make strip
cp hping2 /sbin

cd /tmp
tar jxvf $ION_SRC_OPS/lynx$ION_LYNX_VER.tar.bz2
cd lynx2-8-4
./configure --prefix=/usr
make && make install
cat /usr/lib/lynx.cfg | \
sed 's/STARTFILE\:http\:\/\/lynx.browser.org\//STARTFILE\:file\:\/\/localhost\/usr\/doc\/ion\/lynx-index.html/g' \
> /usr/lib/lynx.cfg
mkdir -p /usr/doc/ion

cat > /usr/doc/ion/lynx-index.html << "EOF"
<html>
<head>
<title>Ion Linux</title>
</head>
<body>
Welcome to Ion Linux.
<p>
Copyright (C) 2002-2003 <a href="http://www.splicednetworks.com">Spliced Networks</a>
<p>
</body>
</html>
EOF


cd /tmp
tar jxvf $ION_SRC_OPS/strace-$ION_STRACE_VER.tar.bz2
cd strace-$ION_STRACE_VER
./configure --prefix=/usr
make && make install

cd /tmp
tar jxvf $ION_SRC_GNU/gnupg-$ION_GNUPG_VER.tar.bz2
cd gnupg-$ION_GNUPG_VER
./configure --prefix=/usr --enable-m-guard --enable-tiger \
--enable-new-tiger --enable-sha512 
make && make install

cd /tmp
tar jxvf $ION_SRC_KER/pciutils-$ION_PCIUTILS_VER.tar.bz2
cd pciutils-$ION_PCIUTILS_VER
make PREFIX=/usr/sbin
install -d -m 755 /usr/sbin /usr/share /usr/share/man/man8
install -c -m 755 -s lspci setpci /usr/sbin
install -c -m 755 update-pciids /usr/sbin
install -c -m 644 pci.ids /usr/share
install -c -m 644 lspci.8 setpci.8 update-pciids.8 /usr/share/man/man8

#
# install kernel make script (gcc-2.95.3)
#
cp $ION_SRC_OPS/kmake /usr/sbin
chmod 755 /usr/sbin/kmake

cd /tmp
tar jxvf $ION_SRC_OPS/vlan-$ION_VLAN_VER.tar.bz2
cd vlan
make purge
make vconfig
cp vconfig /sbin
cp vconfig.8 /usr/share/man/man8

#
# need to add stealth mode to samhain
#

#cd /tmp
#tar zxvf $ION_SRC_OPS/samhain-$ION_SAMHAIN_VER.tar.gz
#cd samhain-$ION_SAMHAIN_VER
#./configure --prefix=/usr --enable-login-watch --enable-static \
#--enable-suidcheck --with-gpg=/usr/bin/gpg
#make && make install

# strip - saves space by removing debugging symbols

cd /usr/bin
strip *
cd /usr/sbin
strip *
cd /sbin
strip *
cd /bin
strip *
cd /usr/libexec
strip *

ION_ETC_RELEASE_VER=`cat /.ion_version`

echo "Ion Linux v"$ION_ETC_RELEASE_VER" (Hydrogen)" > /etc/ion-release

cat /etc/ion-release > /etc/issue
echo " running \r on \m " >> /etc/issue

ln -s issue /etc/issue.net

echo "Ion Linux Operating System" > /boot/message
echo "Ion Linux (tm) "$ION_ETC_RELEASE_VER" [OEM edition ]" >> /boot/message
echo "" >> /boot/message
echo "Copyright (c) 2002-2003 Spliced Networks" >> /boot/message
echo "" >> /boot/message

mv /.ion_motd /etc/motd
rm -rf /.ion_version

mkdir -p /etc/ion
mkdir -p /etc/ion/sysconfig
mkdir -p /etc/ion/sysconfig/network-devices
mkdir -p /etc/ion/shells/bash
mkdir -p /etc/ion/lynx

# inputrc, profile, bashrc was moved to init-misc*.tar.bz

#ln -s /etc/ion/sysconfig /etc/sysconfig
#ln -s /etc/ion/shells/bash/bashrc /etc/bashrc
#ln -s /etc/ion/shells/bash/profile /etc/profile

cat > /etc/ion/sysconfig/clock << "EOF"
# Ion Linux v1.0.0 (Hydrogen)
#
# sysconfig/clock

UTC=0

EOF

cat > /etc/securetty << "EOF"
vc/1
vc/2
vc/3
vc/4
vc/5
vc/6
vc/7
vc/8
vc/9
vc/10
vc/11
tty1
tty2
tty3
tty4
tty5
tty6
tty7
tty8
tty9
tty10
tty11
EOF
chmod 600 /etc/securetty
chown 0.0 /etc/securetty

cat > /etc/login.defs << "EOF"
#
# /etc/login.defs - Configuration control definitions for the login package.
#
#       $Id: login.defs.linux,v 1.12 2000/08/26 18:27:10 marekm Exp $
#
# Three items must be defined:  MAIL_DIR, ENV_SUPATH, and ENV_PATH.
# If unspecified, some arbitrary (and possibly incorrect) value will
# be assumed.  All other items are optional - if not specified then
# the described action or option will be inhibited.
#
# Comment lines (lines beginning with "#") and blank lines are ignored.
#
# Modified for Linux.  --marekm

#
# Delay in seconds before being allowed another attempt after a login failure
#
FAIL_DELAY              3

#
# Enable additional passwords upon dialup lines specified in /etc/dialups.
#
#DIALUPS_CHECK_ENAB     yes

#
# Enable logging and display of /var/log/faillog login failure info.
#
FAILLOG_ENAB            yes

#
# Enable display of unknown usernames when login failures are recorded.
#
LOG_UNKFAIL_ENAB        yes

#
# Enable logging of successful logins
#
LOG_OK_LOGINS           yes

#
# Enable logging and display of /var/log/lastlog login time info.
#
LASTLOG_ENAB            yes

#
# Enable checking and display of mailbox status upon login.
#
# Disable if the shell startup files already check for mail
# ("mailx -e" or equivalent).
#
MAIL_CHECK_ENAB         yes

#
# Enable additional checks upon password changes.
#
OBSCURE_CHECKS_ENAB     yes

#
# Enable checking of time restrictions specified in /etc/porttime.
#
PORTTIME_CHECKS_ENAB    yes

#
# Enable setting of ulimit, umask, and niceness from passwd gecos field.
#
QUOTAS_ENAB             yes

#
# Enable "syslog" logging of su activity - in addition to sulog file logging.
# SYSLOG_SG_ENAB does the same for newgrp and sg.
#
SYSLOG_SU_ENAB          yes
SYSLOG_SG_ENAB          yes

#
# If defined, either full pathname of a file containing device names or
# a ":" delimited list of device names.  Root logins will be allowed only
# upon these devices.
#
CONSOLE         /etc/securetty
#CONSOLE        console:tty01:tty02:tty03:tty04

#
# If defined, all su activity is logged to this file.
#
SULOG_FILE      /var/log/sulog

#
# If defined, ":" delimited list of "message of the day" files to
# be displayed upon login.
#
MOTD_FILE       /etc/motd
#MOTD_FILE      /etc/motd:/usr/lib/news/news-motd

#
# If defined, this file will be output before each login prompt.
#
#ISSUE_FILE      /etc/issue

#
# If defined, file which maps tty line to TERM environment parameter.
# Each line of the file is in a format something like "vt100  tty01".
#
#TTYTYPE_FILE   /etc/ttytype

#
# If defined, login failures will be logged here in a utmp format.
# last, when invoked as lastb, will read /var/log/btmp, so...
#
FTMP_FILE       /var/log/btmp

#
# If defined, name of file whose presence which will inhibit non-root
# logins.  The contents of this file should be a message indicating
# why logins are inhibited.
#
NOLOGINS_FILE   /etc/nologin

#
# If defined, the command name to display when running "su -".  For
# example, if this is defined as "su" then a "ps" will display the
# command is "-su".  If not defined, then "ps" would display the
# name of the shell actually being run, e.g. something like "-sh".
#
SU_NAME         su

#
# *REQUIRED*
#   Directory where mailboxes reside, _or_ name of file, relative to the
#   home directory.  If you _do_ define both, MAIL_DIR takes precedence.
#   QMAIL_DIR is for Qmail
#
#QMAIL_DIR      Maildir
MAIL_DIR        /var/mail
#MAIL_FILE      .mail

#
# If defined, file which inhibits all the usual chatter during the login
# sequence.  If a full pathname, then hushed mode will be enabled if the
# user's name or shell are found in the file.  If not a full pathname, then
# hushed mode will be enabled if the file exists in the user's home directory.
#
HUSHLOGIN_FILE  .hushlogin
#HUSHLOGIN_FILE /etc/hushlogins

#
# If defined, the presence of this value in an /etc/passwd "shell" field will
# disable logins for that user, although "su" will still be allowed.
#
# XXX this does not seem to be implemented yet...  --marekm
# no, it was implemented but I ripped it out ;-) -- jfh
NOLOGIN_STR     NOLOGIN

#
# If defined, either a TZ environment parameter spec or the
# fully-rooted pathname of a file containing such a spec.
#
#ENV_TZ         TZ=CST6CDT
#ENV_TZ         /etc/tzname

#
# If defined, an HZ environment parameter spec.
#
# for Linux/x86
ENV_HZ          HZ=100
# For Linux/Alpha...
#ENV_HZ         HZ=1024

#
# *REQUIRED*  The default PATH settings, for superuser and normal users.
#
# (they are minimal, add the rest in the shell startup files)
ENV_SUPATH      PATH=/sbin:/bin:/usr/sbin:/usr/bin
ENV_PATH        PATH=/bin:/usr/bin
#
# Terminal permissions
#
#       TTYGROUP        Login tty will be assigned this group ownership.
#       TTYPERM         Login tty will be set to this permission.
#
# If you have a "write" program which is "setgid" to a special group
# which owns the terminals, define TTYGROUP to the group number and
# TTYPERM to 0620.  Otherwise leave TTYGROUP commented out and assign
# TTYPERM to either 622 or 600.
#
TTYGROUP        tty
TTYPERM         0600

#
# Login configuration initializations:
#
#       ERASECHAR       Terminal ERASE character ('\010' = backspace).
#       KILLCHAR        Terminal KILL character ('\025' = CTRL/U).
#       UMASK           Default "umask" value.
#       ULIMIT          Default "ulimit" value.
#
# The ERASECHAR and KILLCHAR are used only on System V machines.
# The ULIMIT is used only if the system supports it.
# (now it works with setrlimit too; ulimit is in 512-byte units)
#
# Prefix these values with "0" to get octal, "0x" to get hexadecimal.
#
ERASECHAR       0177
KILLCHAR        025
UMASK           022
#ULIMIT         2097152

#
# Password aging controls:
#
#       PASS_MAX_DAYS   Maximum number of days a password may be used.
#       PASS_MIN_DAYS   Minimum number of days allowed between password changes.
#       PASS_MIN_LEN    Minimum acceptable password length.
#       PASS_WARN_AGE   Number of days warning given before a password expires.
#
PASS_MAX_DAYS   99999
PASS_MIN_DAYS   0
PASS_MIN_LEN    5
PASS_WARN_AGE   7

#
# If "yes", the user must be listed as a member of the first gid 0 group
# in /etc/group (called "root" on most Linux systems) to be able to "su"
# to uid 0 accounts.  If the group doesn't exist or is empty, no one
# will be able to "su" to uid 0.
#
SU_WHEEL_ONLY   no

#
# If compiled with cracklib support, where are the dictionaries
#
CRACKLIB_DICTPATH       /var/cache/cracklib/cracklib_dict

#
# Min/max values for automatic uid selection in useradd
#
UID_MIN                  1000
UID_MAX                 60000

#
# Min/max values for automatic gid selection in groupadd
#
GID_MIN                  1000
GID_MAX                 60000

#
# Max number of login retries if password is bad
#
LOGIN_RETRIES           5

#
# Max time in seconds for login
#
LOGIN_TIMEOUT           60

#
# Maximum number of attempts to change password if rejected (too easy)
#
PASS_CHANGE_TRIES       5

#
# Warn about weak passwords (but still allow them) if you are root.
#
PASS_ALWAYS_WARN        yes

#
# Number of significant characters in the password for crypt().
# Default is 8, don't change unless your crypt() is better.
# Ignored if MD5_CRYPT_ENAB set to "yes".
#
#PASS_MAX_LEN           8

#
# Require password before chfn/chsh can make any changes.
#
CHFN_AUTH               yes

#
# Which fields may be changed by regular users using chfn - use
# any combination of letters "frwh" (full name, room number, work
# phone, home phone).  If not defined, no changes are allowed.
# For backward compatibility, "yes" = "rwh" and "no" = "frwh".
# 
CHFN_RESTRICT           rwh

#
# Password prompt (%s will be replaced by user name).
#
# XXX - it doesn't work correctly yet, for now leave it commented out
# to use the default which is just "Password: ".
#LOGIN_STRING           "%s's Password: "
#
# Only works if compiled with MD5_CRYPT defined:
# If set to "yes", new passwords will be encrypted using the MD5-based
# algorithm compatible with the one used by recent releases of FreeBSD.
# It supports passwords of unlimited length and longer salt strings.
# Set to "no" if you need to copy encrypted passwords to other systems
# which don't understand the new algorithm.  Default is "no".
#
MD5_CRYPT_ENAB yes

#
# List of groups to add to the user's supplementary group set
# when logging in on the console (as determined by the CONSOLE
# setting).  Default is none.
#
# Use with caution - it is possible for users to gain permanent
# access to these groups, even when not logged in on the console.
# How to do it is left as an exercise for the reader...
#
#CONSOLE_GROUPS         floppy:audio:cdrom

#
# Should login be allowed if we can't cd to the home directory?
# Default in no.
#
DEFAULT_HOME    yes

#
# If this file exists and is readable, login environment will be
# read from it.  Every line should be in the form name=value.
#
ENVIRON_FILE    /etc/environment

#
# If defined, this command is run when removing a user.
# It should remove any at/cron/print jobs etc. owned by
# the user to be removed (passed as the first argument).
#
#USERDEL_CMD    /usr/sbin/userdel_local

#
# If defined, either full pathname of a file containing device names or
# a ":" delimited list of device names.  No password is required to log in
# as a non-root user on these devices.
#
#NO_PASSWORD_CONSOLE tty1:tty2:tty3:tty4:tty5:tty6

#
# When prompting for password without echo, getpass() can optionally
# display a random number (in the range 1 to GETPASS_ASTERISKS) of '*'
# characters for each character typed.  This feature is designed to
# confuse people looking over your shoulder when you enter a password :-).
# Also, the new getpass() accepts both Backspace (8) and Delete (127)
# keys to delete previous character (to cope with different terminal
# types), Control-U to delete all characters, and beeps when there are
# no more characters to delete, or too many characters entered.
#
# Setting GETPASS_ASTERISKS to 1 results in more traditional behaviour -
# exactly one '*' displayed for each character typed.
#
# Setting GETPASS_ASTERISKS to 0 disables the '*' characters (Backspace,
# Delete, Control-U and beep continue to work as described above).
#
# Setting GETPASS_ASTERISKS to -1 reverts to the traditional getpass()
# without any new features.  This is the default.
#
#GETPASS_ASTERISKS 1

#
# Enable setting of the umask group bits to be the same as owner bits
# (examples: 022 -> 002, 077 -> 007) for non-root users, if the uid is
# the same as gid, and username is the same as the primary group name.
#
# This also enables userdel to remove user groups if no members exist.
#
USERGROUPS_ENAB yes

CREATE_HOME yes

EOF

#
# compress man pages
#
cp $ION_SRC_OPS/compman /usr/bin
chmod 755 /usr/bin/compman
compman /usr/share/man bz2

# clean up

rm -rf /shared
rm -rf /tmp/*
rm -rf /usr/src/linux-$ION_LINUXHEADERS_VER
rm -rf /usr/src/linux
#samhain -t init
umount /proc
umount /dev/pts
exit


#gcc-3.x
#
# AMD K7 GCC optimizations (need to check if these are still valid)
#CFLAGS = -s -static -O3 -fomit-frame-pointer -Wall \
#-mcpu=athlon \
#-march=athlon -malign-functions=4 -funroll-loops \
#-fexpensive-optimizations -malign-double \
#-fschedule-insns2 \
#-mwide-multiply



