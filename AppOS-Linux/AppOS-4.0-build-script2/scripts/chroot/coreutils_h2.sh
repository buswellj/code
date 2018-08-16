#!/bin/bash
ACV="6.10"
ARC=".tar.bz2"
APN="coreutils"
export ACV ARC APN
ACB=$APN-$ACV
export ACB
###################################
#
#   Coreutils-6.10
#
###################################
cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB
$PTC $LSP/coreutils-6.10-uname_PIC-1.patch
./configure --prefix=/usr --enable-install-program=hostname \
    --enable-no-install-program=kill,uptime
make
make NON_ROOT_USERNAME=nobody check-root
echo "dummy:x:1000:nobody" >> /etc/group
chown -v nobody gnulib-tests/.deps
su-tools nobody -s /bin/bash -c "make RUN_EXPENSIVE_TESTS=yes check"
sed '/dummy/d' -i /etc/group
make install
mv -v /usr/bin/{cat,chgrp,chmod,chown,cp,date,dd,df,echo} /bin
mv -v /usr/bin/{false,hostname,ln,ls,mkdir,mknod,mv,pwd,readlink,rm} /bin
mv -v /usr/bin/{rmdir,stty,sync,true,uname} /bin
mv -v /usr/bin/chroot /usr/sbin
mv -v /usr/bin/{head,sleep,nice} /bin
