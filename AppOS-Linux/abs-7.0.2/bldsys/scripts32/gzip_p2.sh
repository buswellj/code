#!/bin/bash

ACV="1.3.12"
ARC=".tar.gz"
APN="gzip"
export ACV ARC APN

ACB=$APN-$ACV
export ACB

#
# gzip
#
#####################################

cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB

for file in gzip.c lib/utimens.{c,h} ; do \
   cp -v $file{,.orig}
   sed 's/futimens/gl_&/' $file.orig > $file
done

./configure --prefix=/tools
make
make install


