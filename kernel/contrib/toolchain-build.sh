#!/bin/bash

echo "[i] Toolchain build script for Cucumber..."

PREFIX=/usr/xdev
TARGET=i586-elf


echo "[i] Downloading binutils..."
#wget http://ftp.gnu.org/gnu/binutils/binutils-2.21.tar.bz2 -q

echo "[i] Downloading GCC core..."
#wget http://ftp.gnu.org/gnu/gcc/gcc-4.5.3/gcc-core-4.5.3.tar.bz2 -q

echo "[i] Downloading GCC C++..."
#wget http://ftp.gnu.org/gnu/gcc/gcc-4.5.3/gcc-g++-4.5.3.tar.bz2 -q

echo "[i] Extracting binutils..."
#tar xjf binutils-2.21.tar.bz2

echo "[i] Configuring binutils..."
#mkdir build-binutils
#cd build-binutils
#../binutils-2.21/configure --target=$TARGET --prefix=$PREFIX --disable-nls > /dev/null 

echo "[i] Building binutils..."
#make -j4 all > /dev/null

echo "[i] Installing binutils..."
#make install > /dev/null
#cd ..

echo "[i] Extracting GCC..."
#tar xjf gcc-core-4.5.3.tar.bz2
#tar xjf gcc-g++-4.5.3.tar.bz2

echo "[i] Configuring GCC..."
#mkdir build-gcc
cd build-gcc
PATH=$PATH:$PREFIX/bin
../gcc-4.5.3/configure --target=$TARGET --prefix=$PREFIX --disable-nls \
    --enable-languages=c,c++ --without-headers > /dev/null

echo "[i] Building GCC..."
make -j4 all-gcc
make -j4 all-target-libgcc

echo "[i] Installing GCC..."
make install-gcc
make install-target-libgcc

echo "[i] All done."
cd ..