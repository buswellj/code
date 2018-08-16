#!/bin/bash
ACV="113"
ARC=".tar.bz2"
APN="udev"
export ACV ARC APN
ACB=$APN-$ACV
export ACB
###################################
#
#    Udev-113
#
###################################
cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB
#tar -xvf ../udev-config-20070731.tar.bz2
install -dv /lib/{firmware,udev/devices/{pts,shm}}
mknod -m0666 /lib/udev/devices/null c 1 3
ln -sv /proc/self/fd /lib/udev/devices/fd
ln -sv /proc/self/fd/0 /lib/udev/devices/stdin
ln -sv /proc/self/fd/1 /lib/udev/devices/stdout
ln -sv /proc/self/fd/2 /lib/udev/devices/stderr
ln -sv /proc/kcore /lib/udev/devices/core
make EXTRAS="`echo extras/*/`"
make DESTDIR=/ EXTRAS="`echo extras/*/`" install
install -v udevstart /sbin/
cp -v etc/udev/rules.d/[0-9]* /etc/udev/rules.d/
cd udev-config-20070731
make install
cd udev-config-20070731
make install
make install-extra-doc
cd ..
install -m644 -v docs/writing_udev_rules/index.html \
    /usr/share/doc/udev-113/index.html

