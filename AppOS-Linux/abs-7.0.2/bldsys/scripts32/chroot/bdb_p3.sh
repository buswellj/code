#!/tools/bin/bash
ACV="4.6.21"
ARC=".tar.gz"
APN="db"
export ACV ARC APN
ACB=$APN-$ACV
export ACB
#
# Berkeley DB-4.6.21
#
##########################################
cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB
cd build_unix
../dist/configure --prefix=/usr --enable-compat185 --enable-cxx
make
make docdir=/usr/share/doc/db-4.6.21 install
chown -Rv root:root /usr/share/doc/db-4.6.21

