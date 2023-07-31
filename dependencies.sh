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
function isLsbReleaseExists() {
  if which lsb_release > /dev/null; # do whereis can be too?
  then
	  return 0
  else
	  return 1
  fi
}

function anotherDistr(){
 	echo "Just install libboost and libopenssl dev packages on your pc";
	return 0
}

function installDeb(){
	 sudo apt-get install $dependNix;
	 return 0
}
function installOnGentoo(){
		sudo emerge --deep --newuse dev-libs/boost dev-libs/openssl;
		return 0
}
function installOnWin() {
		pacman -S $depenWin;
		return 0
}

function doInstallDepencies() {
case "$1" in
	*Ubuntu*|*Debian*)
		installDeb
		;;
        *gentoo*)
		installOnGentoo
		;;
	*MINGW64*)
		installOnWin
		;;
	*)
		anotherDistr
		;;
esac
}

if isLsbReleaseExists ;
then
 distr=`lsb_release -i`
 doInstallDepencies "$distr"
else #
 doInstallDepencies "$kernel"
fi

