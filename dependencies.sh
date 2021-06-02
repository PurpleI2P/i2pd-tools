#!/bin/bash

dependNix="libboost-chrono-dev \
    libboost-date-time-dev \
    libboost-filesystem-dev \
    libboost-program-options-dev \
    libboost-system-dev \
    libboost-thread-dev \
    libssl-dev \
    zlib1g-dev"

dependWin="mingw-w64-x86_64-boost \
    mingw-w64-x86_64-openssl \
    mingw-w64-x86_64-zlib"

kernel=`uname -a`

case "$kernel" in
*Ubuntu*|*Debian*)
	sudo apt install $dependNix;;
*gentoo*)
	sudo emerge --deep --newuse dev-libs/boost dev-libs/openssl;;
*MINGW64*)
	pacman -S $dependWin;;
*)
	echo "Just install libboost and libopenssl dev packages on your pc";;
esac
