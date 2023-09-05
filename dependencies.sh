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

kernel=$(uname -a)

function anotherDistr() {
    echo "Just install libboost and libopenssl dev packages on your pc"
    return 0
}

function installDnf() {
    sudo dnf install boost-devel g++
}

function installDeb() {
    sudo apt-get install $dependNix
    return 0
}

function installOnGentoo() {
    sudo emerge --deep --newuse dev-libs/boost dev-libs/openssl
    return 0
}

function installOnWin() {
    pacman -S $dependWin
    return 0
}

function doInstallDepencies() {
    case "$1" in
    *Ubuntu* | *Debian*)
        installDeb
        ;;
    *gentoo*)
        installOnGentoo
        ;;
    *MINGW64*)
        installOnWin
        ;;
    *dnf*)
        installDnf
        ;;
    *)
        anotherDistr
        ;;
    esac
}

isLsbReleaseExists=$(which lsb_release > /dev/null 2>&1; echo $?)
if [ $isLsbReleaseExists -eq 0 ]; then
    distr=$(lsb_release -i)
    doInstallDepencies "$distr"
elif test -e /etc/fedora-release || which dnf > /dev/null; then
    printf "Like you use fedora/redhat distr\n"
    doInstallDepencies "dnf"
else
    doInstallDepencies "$kernel"
fi

