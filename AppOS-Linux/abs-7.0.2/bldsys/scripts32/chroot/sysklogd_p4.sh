#!/bin/bash
ACV="1.5"
ARC=".tar.gz"
APN="sysklogd"
export ACV ARC APN
ACB=$APN-$ACV
export ACB
#
# Sysklogd-1.5
#
##########################################
cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB
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

