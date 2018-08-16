#!/bin/bash
ACV="7.1"
ARC=".tar.bz2"
APN="vim"
export ACV ARC APN
ACB=$APN-$ACV
export ACB
###################################
#
#   Vim-7.1
#
###################################
cd $LSB
$TC $LSR/$ACB$ARC
cd $ACB
$PTC $LSP/vim-7.1-fixes-5.patch
$PTC $LSP/vim-7.1-hardened_tmp-1.patch
sed -e 's/(st.st_mode & 0644) | //' -i.orig src/fileio.c
sed 's/di_key\[1\]/di_key\[10\]/' -i.orig src/structs.h
echo '#define SYS_VIMRC_FILE "/etc/vimrc"' >> src/feature.h
./configure --prefix=/usr --enable-multibyte
make 
make install
mv -v /usr/bin/vim /bin
ln -vs /bin/vim /usr/bin/vim
rm -f /usr/share/vim/vim70/tutor/tutor.{gr,pl,ru,sk}
rm -f /usr/share/vim/vim70/tutor/tutor.??.*
ln -sv vim /usr/bin/vi
ln -vs vim /bin/vi
for L in "" fr it pl; do
    ln -sv vim.1 /usr/share/man/$L/man1/vi.1
done
ln -vs ../vim/vim71/doc /usr/share/doc/vim-7.1
cat > /etc/vimrc << "EOF"
" Begin /etc/vimrc

set nocompatible
set backspace=2
if (isdirectory("/usr/share/vim"))
  syntax on
endif
if (&term == "iterm") || (&term == "putty")
  set background=dark
endif

" End /etc/vimrc
EOF
vim -c ':options'

