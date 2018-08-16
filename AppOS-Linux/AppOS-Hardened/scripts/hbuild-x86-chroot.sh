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

chown -R 0:0 /tools

# create directories

install -vd /{bin,boot,etc,home,lib,mnt,sbin,var}
install -vd -m 0750 /root
install -vd -m 1777 /tmp /var/tmp
install -vd /usr/{,local/}{bin,include,lib,sbin,src}
install -vd /usr/{,local/}share/{doc,info,locale,man}
install -vd  /usr/{,local/}share/{misc,terminfo,zoneinfo}
install -vd /usr/{,local/}share/man/man{1..8}
for dir in /usr /usr/local; do
  ln -sv share/{man,doc,info} $dir
done
install -vd /var/{lock,log,mail,run,spool}
install -vd /var/{cache,lib/{misc,locate},local}

# create symlinks

ln -vs /tools/bin/{bash,cat,echo,grep,pwd,rm,stty} /bin
ln -vs /tools/bin/perl /usr/bin
ln -vs /tools/lib/libgcc_s.so{,.1} /usr/lib
ln -vs /tools/lib/libstdc++.so{,.6} /usr/lib
ln -vs bash /bin/sh

# create files

cat > /etc/passwd << "EOF"
root:x:0:0:root:/root:/bin/bash
nobody:x:99:99:Unprivileged User:/nonexistent:/sbin/nologin
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
video:x:12:
utmp:x:13:
usb:x:14:
cdrom:x:15:
mail:x:34:
nobody:x:99:
EOF

touch /etc/mtab

touch /var/run/utmp /var/log/{btmp,lastlog,wtmp}
chgrp -v utmp /var/run/utmp /var/log/lastlog
chmod -v 664 /var/run/utmp /var/log/lastlog

echo " => Linux Headers " >> $SNL
echo " => Linux Headers " >> $SNEL

linux_h2.sh 1>>$SNL 2>>$SNEL

man-pages_h2.sh
glibc_h2.sh
butterfly_toolchain_h2.sh
zlib_h2.sh
openssl_h2.sh
sed_h2.sh
e2fsprogs_h2.sh
coreutils_h2.sh
iana-etc_h2.sh
findutils_h2.sh
gawk_h2.sh
ncurses_h2.sh
readline_h2.sh 
#vim_h2.sh
m4_h2.sh
bison_h2.sh
less_h2.sh
groff_h2.sh
flex_h2.sh
gettext_h2.sh
iproute2_h2.sh
perl_h2.sh
texinfo_h2.sh
autoconf_h2.sh
automake_h2.sh
bash_h2.sh

