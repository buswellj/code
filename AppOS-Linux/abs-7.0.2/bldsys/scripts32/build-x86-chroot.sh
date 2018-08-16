#!/tools/bin/bash

# Set ABS specific variables

LSR=/bldsys/src
SNL=/bldsys/build-x86-c1.log
SNEL=/bldsys/build-x86-e1.log
TC="tar xvf"
PTC="patch -Np1 -i"
export LSR SNL SNEL TC PTC

LSP=$LSR/patches
export LSP

# create directories

mkdir -pv /{bin,boot,etc/opt,home,lib,mnt,opt}
mkdir -pv /{media/{floppy,cdrom},sbin,srv,var}
install -dv -m 0750 /root
install -dv -m 1777 /tmp /var/tmp
mkdir -pv /usr/{,local/}{bin,include,lib,sbin,src}
mkdir -pv /usr/{,local/}share/{doc,info,locale,man}
mkdir -v  /usr/{,local/}share/{misc,terminfo,zoneinfo}
mkdir -pv /usr/{,local/}share/man/man{1..8}
for dir in /usr /usr/local; do
  ln -sv share/{man,doc,info} $dir
done
mkdir -v /var/{lock,log,mail,run,spool}
mkdir -pv /var/{opt,cache,lib/{misc,locate},local}

# create symlinks

ln -sv /tools/bin/{bash,cat,echo,grep,pwd,stty} /bin
ln -sv /tools/bin/perl /usr/bin
ln -sv /tools/lib/libgcc_s.so{,.1} /usr/lib
ln -sv /tools/lib/libstdc++.so{,.6} /usr/lib
ln -sv bash /bin/sh

# create files

touch /etc/mtab
cat > /etc/passwd << "EOF"
root:x:0:0:root:/root:/bin/bash
nobody:x:99:99:Unprivileged User:/dev/null:/bin/false
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
uucp:x:10:
audio:x:11:
video:x:12:
utmp:x:13:
usb:x:14:
cdrom:x:15:
mail:x:34:
nogroup:x:99:
EOF

touch /var/run/utmp /var/log/{btmp,lastlog,wtmp}
chgrp -v utmp /var/run/utmp /var/log/lastlog
chmod -v 664 /var/run/utmp /var/log/lastlog

echo " => Linux Headers " >> $SNL
echo " => Linux Headers " >> $SNEL

linuxhdr_p3.sh 1>>$SNL 2>>$SNEL

echo " => man-pages " >> $SNL
echo " => man-pages " >> $SNEL

man-pages_p3.sh 1>>$SNL 2>>$SNEL

echo " => glibc " >> $SNL
echo " => glibc " >> $SNEL

glibc_p3.sh 1>>$SNL 2>>$SNEL

echo " => binutils " >> $SNL
echo " => binutils " >> $SNEL

binutils_p3.sh 1>>$SNL 2>>$SNEL

echo " => gcc " >> $SNL
echo " => gcc " >> $SNEL

gcc_p3.sh 1>>$SNL 2>>$SNEL

echo " => db4 " >> $SNL
echo " => db4 " >> $SNEL

bdb_p3.sh 1>>$SNL 2>>$SNEL

echo " => sed " >> $SNL
echo " => sed " >> $SNEL

sed_p3.sh 1>>$SNL 2>>$SNEL

echo " => e2fsprogs " >> $SNL
echo " => e2fsprogs " >> $SNEL

e2fsprogs_p3.sh 1>>$SNL 2>>$SNEL

echo " => coreutils " >> $SNL
echo " => coreutils " >> $SNEL

coreutils_p3.sh 1>>$SNL 2>>$SNEL

echo " => ianaetc " >> $SNL
echo " => ianaetc " >> $SNEL

ianaetc_p3.sh 1>>$SNL 2>>$SNEL

echo " => m4 " >> $SNL
echo " => m4 " >> $SNEL

m4_p3.sh 1>>$SNL 2>>$SNEL

echo " => bison " >> $SNL
echo " => bison " >> $SNEL

bison_p3.sh 1>>$SNL 2>>$SNEL

echo " => ncurses " >> $SNL
echo " => ncurses " >> $SNEL

ncurses_p3.sh 1>>$SNL 2>>$SNEL

echo " => procps " >> $SNL
echo " => procps " >> $SNEL

procps_p3.sh 1>>$SNL 2>>$SNEL

echo " => libtool " >> $SNL
echo " => libtool " >> $SNEL

libtool_p3.sh 1>>$SNL 2>>$SNEL

echo " => perl " >> $SNL
echo " => perl " >> $SNEL

perl_p3.sh 1>>$SNL 2>>$SNEL

echo " => readline " >> $SNL
echo " => readline " >> $SNEL

readline_p3.sh 1>>$SNL 2>>$SNEL

echo " => zlib " >> $SNL
echo " => zlib " >> $SNEL

zlib_p3.sh 1>>$SNL 2>>$SNEL

echo " => autoconf " >> $SNL
echo " => autoconf " >> $SNEL

autoconf_p3.sh 1>>$SNL 2>>$SNEL

echo " => automake " >> $SNL
echo " => automake " >> $SNEL

automake_p3.sh 1>>$SNL 2>>$SNEL

echo " => bash " >> $SNL
echo " => bash " >> $SNEL

bash_p3.sh 1>>$SNL 2>>$SNEL

echo " => AppOS(R) IBE chroot phase 1 completed " >> $SNL
echo " => AppOS(R) IBE chroot phase 1 completed " >> $SNEL

logout
