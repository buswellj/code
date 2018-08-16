#!/bin/bash

ACV="5.43.0"
ACVF="5.43"
ARC=".tar.gz"
APN="expect"
export ACV ARC APN

ACF=$APN-$ACV
ACB=$APN-$ACVF
export ACB

#
# expect pass 1
#
#####################################

# note: stupid fixup for version mismatch in expect

cd $LSB
$TC $LSR/$ACF$ARC
cd $ACB

$PTC $LSP/expect-5.43.0-spawn-1.patch

cp configure{,.bak}
sed 's:/usr/local/bin:/bin:' configure.bak > configure
./configure --prefix=/tools --with-tcl=/tools/lib \
  --with-tclinclude=/tools/include --with-x=no
make
make SCRIPTS="" install

