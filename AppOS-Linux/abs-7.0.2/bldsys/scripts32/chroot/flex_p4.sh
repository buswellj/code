#!/bin/bash
ACV="2.5.34"
ARC=".tar.bz2"
APN="flex"
export ACV ARC APN
ACB=$APN-$ACV
export ACB
#
# Flex-2.5.34
#
##########################################
cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB
./configure --prefix=/usr
make
make install
ln -sv libfl.a /usr/lib/libl.a
cat > /usr/bin/lex << "EOF"
#!/bin/sh
# Begin /usr/bin/lex

exec /usr/bin/flex -l "$@"

# End /usr/bin/lex
EOF
chmod -v 755 /usr/bin/lex

