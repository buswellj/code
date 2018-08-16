#!/bin/bash

#
# Apache 2.2 Secure Solution Build Script
#

#
# Directory Structure for SN non-AppOS Stacks
#
# /opt/sn/
#   app/			Master App
#   chroot/			Production chroot
#   mgmt/			Management tools

#
# Apache
#
# /opt/sn/app/apache
# /opt/sn/chroot/apache
#
# /opt/sn/chroot/apache/images/{exec,conf,misc}
# /opt/sn/chroot/apache/
#
# solution symlinks apache/* dirs to images/* dirs
#
# tmp/			tmpfs partition
# opt/sn/apache		
# web/			htdocs (user mounted)
#

mkdir -p /opt/sn/app/apache
mkdir -p /opt/sn/chroot/opt/sn/app/apache
cd /tmp
mkdir snbuild
cd snbuild

wget http://apache.mirrors.pair.com/httpd/httpd-2.2.2.tar.bz2
tar jxvf httpd-2.2.2.tar.bz2
cd httpd-2.2.2
./configure --prefix=/opt/sn/app/apache --enable-pie --enable-so --enable-mods-shared=most --enable-ssl --with-ssl=/opt/sn/app/ssl
make && make install

cd ../
mkdir -p sectools
cd sectools
wget http://search.cpan.org/CPAN/authors/id/S/SA/SAMPO/Net_SSLeay.pm-1.25.tar.gz
tar zxvf Net_SSLeay.pm-1.25.tar.gz 
cd Net_SSLeay.pm-1.25
perl Makefile.PL
make
make install
cd ..
wget http://www.cirt.net/nikto/nikto-current.tar.gz
tar zxvf nikto-current.tar.gz
cd nikto-1.35
cd plugins/
mv LW.pm LW.pm.orig
wget http://www.wiretrip.net/rfp/libwhisker/LW.pm
cd ../
./nikto.pl -update

groupadd snhttpd
useradd snhttpd -g snhttpd -d /dev/null -s /sbin/nologin


