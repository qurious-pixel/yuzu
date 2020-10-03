#!/bin/sh -x

export USE_CCACHE=1
export HOMEBREW_NO_INSTALL_CLEANUP=1
export HOMEBREW_NO_AUTO_UPDATE=1
export HOMEBREW_NO_ANALYTICS=1
export PATH="/usr/local/sbin:$PATH"

brew update --force

brew install p7zip sdl2 ccache ninja ffmpeg llvm pulseaudio mbedtls gpm pkgconfig
brew upgrade qt openssl@1.1 zstd
#removing boost
brew outdated cmake || brew upgrade cmake
#brew upgrade gcc
pip3 install macpack
#brew install unicorn && UNICORN_QEMU_FLAGS="--python=`whereis python`" pip3 install unicorn
pip3 install conan
