wget http://www.openssl.org/source/openssl-0.9.8b.tar.gz
tar zxvf openssl-0.9.8b.tar.gz ; \
cd openssl-0.9.8b ; \
./config --prefix=/opt/sn/apps/ssl --openssldir=/opt/sn/ssl \
  zlib-dynamic shared
make && make test
make install

echo "/opt/sn/apps/ssl/lib" >> /etc/ld.so.conf.d/sn.conf
PATH=/opt/sn/apps/ssl/bin:$PATH
ldconfig
wget ftp://ftp.isc.org/isc/bind9/9.3.3b1/bind-9.3.3b1.tar.gz
tar zxvf bind-9.3.3b1.tar.gz
cd bind-9.3.3b1
./configure --prefix=/opt/sn/apps/dns --enable-threads --with-pic \
 --disable-static --with-openssl=/opt/sn/apps/ssl
make && make install

groupadd -g 6000 sndns
useradd -u 6000 -g sndns -d /opt/sn/chroot/dns -c "DNS" -m sndns
mkdir -p /opt/sn/chroot/dns
cd /opt/sn/chroot/dns
mkdir -p dev etc/zones/slave var/run
mknod dev/null c 1 3
mknod dev/random c 1 8
chmod 666 dev/{random,null}
cp /etc/localtime etc/
cd ../../
chown root chroot/
chmod 700 chroot
chown sndns:sndns chroot/dns/
chmod 700 chroot/dns/
cd chroot/dns/
chown sndns:sndns etc/zones/slave
chown sndns:sndns var/run
chattr +i etc etc/localtime var

