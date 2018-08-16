#!/bin/bash
ACV="1.5"
ARC=".tar.gz"
APN="sysklogd"
export ACV ARC APN
ACB=$APN-$ACV
export ACB
###################################
#
#    Sysklogd-1.5
#
###################################
cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB
$PTC ../sysklogd-1.5-disable__syslog_chk-1.patch
$PTC ../sysklogd-1.5-priv_sep-1.patch
cat >> /etc/passwd << "EOF"
syslogd:x:16:16:System Log Daemon:/var/lib/syslogd:/sbin/nologin
klogd:x:17:17:Kernel Log Daemon:/var/lib/klogd:/sbin/nologin
EOF
cat >> /etc/group << "EOF"
syslogd:x:16:
klogd:x:17:
EOF
install -d -m0000 /var/lib/syslogd
install -d -m0000 /var/lib/klogd
sed 's/644/600/' -i.orig syslogd.c
make
make install
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

