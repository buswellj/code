#!/bin/bash
ACV="7.1"
ARC=".tar.bz2"
APN="Vim"
export ACV ARC APN
ACB=$APN-$ACV
export ACB
#
# Vim-7.1
#
##########################################
cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB

patch -Np1 -i ../vim-7.1-fixes-5.patch
patch -Np1 -i ../vim-7.1-mandir-1.patch
echo '#define SYS_VIMRC_FILE "/etc/vimrc"' >> src/feature.h
./configure --prefix=/usr --enable-multibyte


make
make install
ln -sv vim /usr/bin/vi
for L in "" fr it pl ru; do
    ln -sv vim.1 /usr/share/man/$L/man1/vi.1
done
ln -sv ../vim/vim71/doc /usr/share/doc/vim-7.1
cat > /etc/vimrc << "EOF"
" Begin /etc/vimrc

set nocompatible
set backspace=2
syntax on
if (&term == "iterm") || (&term == "putty")
  set background=dark
endif

" End /etc/vimrc
EOF
