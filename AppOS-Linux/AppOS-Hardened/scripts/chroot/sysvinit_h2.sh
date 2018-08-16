#!/bin/bash
ACV="2.86"
ARC=".tar.gz"
APN="sysvinit"
export ACV ARC APN
ACB=$APN-$ACV
export ACB
###################################
#
#   Sysvinit-2.86
#
###################################
cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB
sed 's/libcrypt.a/libcrypt.so/' -i.orig src/Makefile
sed 's@Sending processes@& started by init@g' \
    -i.orig src/init.c
make -C src
make -C src install
cat > inittab.new << "EOF"
# Begin /etc/inittab

id:3:initdefault:

si::sysinit:/etc/rc.d/init.d/rc sysinit

l0:0:wait:/etc/rc.d/init.d/rc 0
l1:S1:wait:/etc/rc.d/init.d/rc 1
l2:2:wait:/etc/rc.d/init.d/rc 2
l3:3:wait:/etc/rc.d/init.d/rc 3
l4:4:wait:/etc/rc.d/init.d/rc 4
l5:5:wait:/etc/rc.d/init.d/rc 5
l6:6:wait:/etc/rc.d/init.d/rc 6

ca:12345:ctrlaltdel:/sbin/shutdown -t1 -a -r now

su:S016:once:/sbin/sulogin

1:2345:respawn:/sbin/agetty -I '\033(K' tty1 9600
2:2345:respawn:/sbin/agetty -I '\033(K' tty2 9600
3:2345:respawn:/sbin/agetty -I '\033(K' tty3 9600
4:2345:respawn:/sbin/agetty -I '\033(K' tty4 9600
5:2345:respawn:/sbin/agetty -I '\033(K' tty5 9600
6:2345:respawn:/sbin/agetty -I '\033(K' tty6 9600

# End /etc/inittab
EOF
install -m644 inittab.new /etc/inittab

