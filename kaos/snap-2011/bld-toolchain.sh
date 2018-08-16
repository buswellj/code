#!/bin/bash
#
# Kernel Attached Operating System (KaOS)
# Platform Build System version 1.0.0
#
# Copyright (c) 2009-2011 Carbon Mountain LLC
#
# script : bld-toolchain.sh
# purpose: builds the KaOS toolchain system
#

echo "  [-] Begin toolchain script "
echo "  [.] Please wait while the source tree is copied..."
echo ""
cd $PBBLD
mkdir src1
cp -a $PBSRC/* src1

echo "  [.] Applying patches to source tree..."
echo ""
cd src1
cd glibc
patch -Np1 -i ../patches/glibc-2.13-gcc_fix-1.patch
cd ../perl
patch -Np1 -i ../patches/perl-5.12.3-libc-1.patch
cd ../gcc
patch -Np1 -i ../patches/gcc-4.6.0-cross_compile-1.patch
cd ../bash
patch -Np1 -i ../patches/bash-4.2-fixes-3.patch
cd $PBBLD

echo "  [.] Building toolchain environment..."
echo ""

set +h
umask 022
LC_ALL=POSIX
LFS_TGT=$(uname -m)-kaos-linux-gnu
PATH=/tools.$PBUSER/bin:/bin:/usr/bin:$PATH
export LC_ALL LFS_TGT PATH

cd $PBBLD/src1
echo "  [.] building binutils "
mkdir -v binutils-build
cd binutils-build
../binutils/configure --target=$LFS_TGT --prefix=$TOOLS --disable-nls --disable-werror 1>$PBLOG/strap_binutils.log 2>$PBLOG/strap_binutils.err
make 1>>$PBLOG/strap_binutils.log 2>>$PBLOG/strap_binutils.err 
case $(uname -m) in
  x86_64) mkdir -v $TOOLS/lib && ln -sv lib $TOOLS/lib64 ;;
esac
make install 1>>$PBLOG/strap_binutils.log 2>>$PBLOG/strap_binutils.err 

cd $PBBLD/src1
echo "  [.] building gcc "
mkdir -v gcc-build
cd gcc-build
../gcc/configure \
    --target=$LFS_TGT --prefix=$TOOLS \
    --disable-nls --disable-shared --disable-multilib \
    --disable-decimal-float --disable-threads \
    --disable-libmudflap --disable-libssp \
    --disable-libgomp -disable-libquadmath \
    --disable-target-libiberty --disable-target-zlib \
    --enable-languages=c \
    --with-gmp-include=$(pwd)/gmp --with-gmp-lib=$(pwd)/gmp/.libs \
    --without-ppl --without-cloog 1>$PBLOG/strap_gcc.log 2>$PBLOG/strap_gcc.err
make 1>>$PBLOG/strap_gcc.log 2>>$PBLOG/strap_gcc.err
make install 1>>$PBLOG/strap_gcc.log 2>>$PBLOG/strap_gcc.err
ln -vs libgcc.a `$LFS_TGT-gcc -print-libgcc-file-name | \
    sed 's/libgcc/&_eh/'`

cd $PBBLD/src1
echo "  [.] building kernel header api"
cd linux
make mrproper
make headers_check
make INSTALL_HDR_PATH=dest headers_install
mkdir -p $TOOLS/include
cp -rv dest/include/* $TOOLS/include

cd $PBBLD/src1
echo "  [.] building glibc "
mkdir -v glibc-build
cd glibc-build
case `uname -m` in
  i?86) echo "CFLAGS += -march=i486 -mtune=native" > configparms ;;
esac
../glibc/configure --prefix=$TOOLS \
    --host=$LFS_TGT --build=$(../glibc/scripts/config.guess) \
    --disable-profile --enable-add-ons \
    --enable-kernel=2.6.22.5 --with-headers=$TOOLS/include \
    libc_cv_forced_unwind=yes libc_cv_c_cleanup=yes 1>$PBLOG/strap_glibc.log 2>$PBLOG/strap_glibc.err
make 1>>$PBLOG/strap_glibc.log 2>>$PBLOG/strap_glibc.err
make install 1>>$PBLOG/strap_glibc.log 2>>$PBLOG/strap_glibc.err

cd $PBBLD/src1
echo "  [.] toolchain adjustment "
echo "#!/bin/bash" > tooladj.sh
echo "SPECS=\`dirname \$(\$LFS_TGT-gcc -print-libgcc-file-name)\`/specs" >> tooladj.sh
echo "\$LFS_TGT-gcc -dumpspecs | sed \\" >> tooladj.sh
echo "  -e 's@/lib\\(64\\)\\?/ld@/tools.$PBUSER&@g' \\" >> tooladj.sh
echo "  -e \"/^\\*cpp:\$/{n;s,\$, -isystem /tools.$PBUSER/include,}\" > \$SPECS " >> tooladj.sh
echo "echo \"New specs file is: \$SPECS\" " >> tooladj.sh
echo "unset SPECS" >> tooladj.sh
chmod 755 tooladj.sh
./tooladj.sh

echo "  [.] testing toolchain "
echo 'main(){}' > dummy.c
$LFS_TGT-gcc -B$TOOLS/lib dummy.c
readelf -l a.out | grep ': /tools' 1>>$PBLOG/strap_tooladj.log 2>>$PBLOG/strap_tooladj.err

echo "  [.] binutils pass 2"
mkdir -v binutils-pass2
cd binutils-pass2
CC="$LFS_TGT-gcc -B$TOOLS/lib/" \
   AR=$LFS_TGT-ar RANLIB=$LFS_TGT-ranlib \
   ../binutils/configure --prefix=$TOOLS \
   --disable-nls --with-lib-path=$TOOLS/lib 1>>$PBLOG/strap_binutils2.log 2>>$PBLOG/strap_binutils2.err

make 1>>$PBLOG/strap_binutils2.log 2>>$PBLOG/strap_binutils2.err
make install 1>>$PBLOG/strap_binutils2.log 2>>$PBLOG/strap_binutils2.err
make -C ld clean 1>>$PBLOG/strap_binutils2.log 2>>$PBLOG/strap_binutils2.err
make -C ld LIB_PATH=/usr/lib:/lib 1>>$PBLOG/strap_binutils2.log 2>>$PBLOG/strap_binutils2.err
cp -v ld/ld-new $TOOLS/bin 1>>$PBLOG/strap_binutils2.log 2>>$PBLOG/strap_binutils2.err

cd $PBBLD/src1/gcc
patch -Np1 -i ../patches/gcc-4.6.0-startfiles_fix-1.patch
cp -v gcc/Makefile.in{,.orig}
sed 's@\./fixinc\.sh@-c true@' gcc/Makefile.in.orig > gcc/Makefile.in
cp -v gcc/Makefile.in{,.tmp}
sed 's/^T_CFLAGS =$/& -fomit-frame-pointer/' gcc/Makefile.in.tmp \
  > gcc/Makefile.in
echo "#!/bin/bash" >> specs-adj.sh
echo "for file in \\" >> specs-adj.sh
echo " \$(find gcc/config -name linux64.h -o -name linux.h -o -name sysv4.h)" >> specs-adj.sh
echo "do" >> specs-adj.sh
echo "  cp -uv \$file{,.orig}" >> specs-adj.sh
echo "  sed -e 's@/lib\\(64\\)\\?\\(32\\)\\?/ld@/tools.$PBUSER&@g' \\" >> specs-adj.sh
echo "  -e 's@/usr@/tools.$PBUSER@g' \$file.orig > \$file" >> specs-adj.sh
echo "  echo '" >> specs-adj.sh
echo "#undef STANDARD_INCLUDE_DIR" >> specs-adj.sh
echo "#define STANDARD_INCLUDE_DIR 0" >> specs-adj.sh
echo "#define STANDARD_STARTFILE_PREFIX_1 \"\"" >> specs-adj.sh
echo "#define STANDARD_STARTFILE_PREFIX_2 \"\"' >> \$file" >> specs-adj.sh
echo "  touch \$file.orig" >> specs-adj.sh
echo "done" >> specs-adj.sh
chmod 755 specs-adj.sh
./specs-adj.sh 1>>$PBLOG/strap_gcc2.log 2>>$PBLOG/strap_gcc2.err
case $(uname -m) in
  x86_64)
    for file in $(find gcc/config -name t-linux64) ; do \
      cp -v $file{,.orig}
      sed '/MULTILIB_OSDIRNAMES/d' $file.orig > $file
    done
  ;;
esac
cd $PBBLD/src1
mkdir -v gcc-build2
cd gcc-build2
CC="$LFS_TGT-gcc -B$TOOLS/lib/" \
    AR=$LFS_TGT-ar RANLIB=$LFS_TGT-ranlib \
    ../gcc/configure --prefix=$TOOLS \
    --with-local-prefix=$TOOLS --enable-clocale=gnu \
    --enable-shared --enable-threads=posix \
    --enable-__cxa_atexit --enable-languages=c,c++ \
    --disable-libstdcxx-pch --disable-multilib \
    --disable-bootstrap --disable-libgomp \
    --with-gmp-include=$(pwd)/gmp --with-gmp-lib=$(pwd)/gmp/.libs \
    --without-ppl --without-cloog 1>>$PBLOG/strap_gcc2.log 2>>$PBLOG/strap_gcc2.err

make 1>>$PBLOG/strap_gcc2.log 2>>$PBLOG/strap_gcc2.err
make install 1>>$PBLOG/strap_gcc2.log 2>>$PBLOG/strap_gcc2.err
ln -vs gcc $TOOLS/bin/cc
cd $PBBLD/src1

echo 'main(){}' > dummy2.c
cc dummy2.c
readelf -l a.out | grep ': /tools' 1>>$PBLOG/strap_gcc2.log 2>>$PBLOG/strap_gcc2.err

cd $PBBLD/src1
echo "  [.] building tcl.. "
cd tcl/unix
./configure --prefix=$TOOLS 1>>$PBLOG/strap_tcl.log 2>>$PBLOG/strap_tcl.err
make 1>>$PBLOG/strap_tcl.log 2>>$PBLOG/strap_tcl.err
make install 1>>$PBLOG/strap_tcl.log 2>>$PBLOG/strap_tcl.err
chmod -v u+w $TOOLS/lib/libtcl*.so
make install-private-headers 1>>$PBLOG/strap_tcl.log 2>>$PBLOG/strap_tcl.err
ln -sv tclsh8.5 $TOOLS/bin/tclsh
cd $PBBLD/src1

echo "  [.] building expect.. "
cd expect
cp -v configure{,.orig}
sed 's:/usr/local/bin:/bin:' configure.orig > configure
./configure --prefix=$TOOLS --with-tcl=$TOOLS/lib \
  --with-tclinclude=$TOOLS/include --with-x=no --with-tk=no 1>>$PBLOG/strap_expect.log 2>>$PBLOG/strap_expect.err
make 1>>$PBLOG/strap_expect.log 2>>$PBLOG/strap_expect.err
make SCRIPTS="" install 1>>$PBLOG/strap_expect.log 2>>$PBLOG/strap_expect.err
cd $PBBLD/src1

echo "  [.] building dejagnu.. "
cd dejagnu
./configure --prefix=$TOOLS 1>>$PBLOG/strap_dejagnu.log 2>>$PBLOG/strap_dejagnu.err
make install 1>>$PBLOG/strap_dejagnu.log 2>>$PBLOG/strap_dejagnu.err
cd $PBBLD/src1

echo "  [.] building ncurses.. "
cd ncurses
./configure --prefix=$TOOLS --with-shared \
    --without-debug --without-ada --enable-overwrite 1>>$PBLOG/strap_ncurses.log 2>>$PBLOG/strap_ncurses.err
make 1>>$PBLOG/strap_ncurses.log 2>>$PBLOG/strap_ncurses.err
make install 1>>$PBLOG/strap_ncurses.log 2>>$PBLOG/strap_ncurses.err
cd $PBBLD/src1

echo "  [.] building bash.. "
cd bash
./configure --prefix=$TOOLS --without-bash-malloc 1>>$PBLOG/strap_bash.log 2>>$PBLOG/strap_bash.err
make 1>>$PBLOG/strap_bash.log 2>>$PBLOG/strap_bash.err
make install 1>>$PBLOG/strap_bash.log 2>>$PBLOG/strap_bash.err
ln -vs bash $TOOLS/bin/sh
cd $PBBLD/src1

echo "  [.] building bzip2.. "
cd bzip2
make 1>>$PBLOG/strap_bzip2.log 2>>$PBLOG/strap_bzip2.err
make PREFIX=$TOOLS install 1>>$PBLOG/strap_bzip2.log 2>>$PBLOG/strap_bzip2.err
cd $PBBLD/src1

echo "  [.] building coreutils.. "
cd coreutils
./configure --prefix=$TOOLS --enable-install-program=hostname 1>>$PBLOG/strap_coreutils.log 2>>$PBLOG/strap_coreutils.err
make 1>>$PBLOG/strap_coreutils.log 2>>$PBLOG/strap_coreutils.err
make install 1>>$PBLOG/strap_coreutils.log 2>>$PBLOG/strap_coreutils.err
cp -v src/su $TOOLS/bin/su-tools 1>>$PBLOG/strap_coreutils.log 2>>$PBLOG/strap_coreutils.err
cd $PBBLD/src1

echo "  [.] building diffutils.. "
cd diffutils
./configure --prefix=$TOOLS 1>>$PBLOG/strap_diffutils.log 2>>$PBLOG/strap_diffutils.err
make 1>>$PBLOG/strap_diffutils.log 2>>$PBLOG/strap_diffutils.err
make install 1>>$PBLOG/strap_diffutils.log 2>>$PBLOG/strap_diffutils.err
cd $PBBLD/src1

echo "  [.] building file.. "
cd file
./configure --prefix=$TOOLS 1>>$PBLOG/strap_file.log 2>>$PBLOG/strap_file.err
make 1>>$PBLOG/strap_file.log 2>>$PBLOG/strap_file.err
make install 1>>$PBLOG/strap_file.log 2>>$PBLOG/strap_file.err
cd $PBBLD/src1

echo "  [.] building findutils.. "
cd findutils
./configure --prefix=$TOOLS 1>>$PBLOG/strap_findutils.log 2>>$PBLOG/strap_findutils.err
make 1>>$PBLOG/strap_findutils.log 2>>$PBLOG/strap_findutils.err
make install 1>>$PBLOG/strap_findutils.log 2>>$PBLOG/strap_findutils.err
cd $PBBLD/src1

echo "  [.] building gawk.. "
cd gawk
./configure --prefix=$TOOLS 1>>$PBLOG/strap_gawk.log 2>>$PBLOG/strap_gawk.err
make 1>>$PBLOG/strap_gawk.log 2>>$PBLOG/strap_gawk.err
make install 1>>$PBLOG/strap_gawk.log 2>>$PBLOG/strap_gawk.err
cd $PBBLD/src1

echo "  [.] building gettext.. "
cd gettext/gettext-tools
./configure --prefix=$TOOLS --disable-shared 1>>$PBLOG/strap_gettext.log 2>>$PBLOG/strap_gettext.err
make -C gnulib-lib 1>>$PBLOG/strap_gettext.log 2>>$PBLOG/strap_gettext.err
make -C src msgfmt 1>>$PBLOG/strap_gettext.log 2>>$PBLOG/strap_gettext.err
cp -v src/msgfmt $TOOLS/bin
cd $PBBLD/src1

echo "  [.] building grep.. "
cd grep
./configure --prefix=$TOOLS --disable-perl-regexp --without-included-regex 1>>$PBLOG/strap_grep.log 2>>$PBLOG/strap_grep.err
make 1>>$PBLOG/strap_grep.log 2>>$PBLOG/strap_grep.err
make install 1>>$PBLOG/strap_grep.log 2>>$PBLOG/strap_grep.err
cd $PBBLD/src1

echo "  [.] building gzip.. "
cd gzip
./configure --prefix=$TOOLS 1>>$PBLOG/strap_gzip.log 2>>$PBLOG/strap_gzip.err
make 1>>$PBLOG/strap_gzip.log 2>>$PBLOG/strap_gzip.err
make install 1>>$PBLOG/strap_gzip.log 2>>$PBLOG/strap_gzip.err
cd $PBBLD/src1

echo "  [.] building m4.. "
cd m4
./configure --prefix=$TOOLS 1>>$PBLOG/strap_m4.log 2>>$PBLOG/strap_m4.err
make 1>>$PBLOG/strap_m4.log 2>>$PBLOG/strap_m4.err
make install 1>>$PBLOG/strap_m4.log 2>>$PBLOG/strap_m4.err
cd $PBBLD/src1

echo "  [.] building make.. "
cd make
./configure --prefix=$TOOLS 1>>$PBLOG/strap_make.log 2>>$PBLOG/strap_make.err
make 1>>$PBLOG/strap_make.log 2>>$PBLOG/strap_make.err
make install 1>>$PBLOG/strap_make.log 2>>$PBLOG/strap_make.err
cd $PBBLD/src1

echo "  [.] building patch.. "
cd patch
./configure --prefix=$TOOLS 1>>$PBLOG/strap_patch.log 2>>$PBLOG/strap_patch.err
make 1>>$PBLOG/strap_patch.log 2>>$PBLOG/strap_patch.err
make install 1>>$PBLOG/strap_patch.log 2>>$PBLOG/strap_patch.err
cd $PBBLD/src1

echo "  [.] building perl.."
cd perl
sh Configure -des -Dprefix=$TOOLS -Dstatic_ext='Data/Dumper Fcntl IO' 1>>$PBLOG/strap_perl.log 2>>$PBLOG/strap_perl.err
make perl utilities ext/Errno/pm_to_blib 1>>$PBLOG/strap_perl.log 2>>$PBLOG/strap_perl.err
cp -v perl pod/pod2man $TOOLS/bin 1>>$PBLOG/strap_perl.log 2>>$PBLOG/strap_perl.err
mkdir -pv $TOOLS/lib/perl5/5.12.3 1>>$PBLOG/strap_perl.log 2>>$PBLOG/strap_perl.err
cp -Rv lib/* $TOOLS/lib/perl5/5.12.3 1>>$PBLOG/strap_perl.log 2>>$PBLOG/strap_perl.err
cd $PBBLD/src1

echo "  [.] building sed.. "
cd sed
./configure --prefix=$TOOLS 1>>$PBLOG/strap_sed.log 2>>$PBLOG/strap_sed.err
make 1>>$PBLOG/strap_sed.log 2>>$PBLOG/strap_sed.err
make install 1>>$PBLOG/strap_sed.log 2>>$PBLOG/strap_sed.err
cd $PBBLD/src1

echo "  [.] building tar.. "
cd tar
./configure --prefix=$TOOLS 1>>$PBLOG/strap_tar.log 2>>$PBLOG/strap_tar.err
make 1>>$PBLOG/strap_tar.log 2>>$PBLOG/strap_tar.err
make install 1>>$PBLOG/strap_sed.log 2>>$PBLOG/strap_tar.err
cd $PBBLD/src1

echo "  [.] building texinfo.. "
cd texinfo
./configure --prefix=$TOOLS 1>>$PBLOG/strap_texinfo.log 2>>$PBLOG/strap_texinfo.err
make 1>>$PBLOG/strap_texinfo.log 2>>$PBLOG/strap_texinfo.err
make install 1>>$PBLOG/strap_texinfo.log 2>>$PBLOG/strap_texinfo.err
cd $PBBLD/src1

echo "  [.] building xz.."
cd xz
./configure --prefix=$TOOLS 1>>$PBLOG/strap_xz.log 2>>$PBLOG/strap_xz.err
make  1>>$PBLOG/strap_xz.log 2>>$PBLOG/strap_xz.err
make check 1>>$PBLOG/strap_xz.log 2>>$PBLOG/strap_xz.err
make install 1>>$PBLOG/strap_xz.log 2>>$PBLOG/strap_xz.err
cd $PBBLD/src1

echo "  [.] clean-up"
strip --strip-debug $TOOLS/lib/*
strip --strip-unneeded $TOOLS/{,s}bin/*
rm -rf $TOOLS/{,share}/{info,man}
sudo chown -R root:root $LFS/tools.$PBUSER

echo ""
echo "  [!] Toolchain build completed."
echo ""

cd $PPWD
