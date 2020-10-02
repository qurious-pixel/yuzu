#!/bin/sh -x

export USE_CCACHE=1
export HOMEBREW_NO_INSTALL_CLEANUP=1
export HOMEBREW_NO_AUTO_UPDATE=1
export HOMEBREW_NO_ANALYTICS=1
export PATH="/usr/local/sbin:$PATH"

brew update
brew install p7zip qt5 sdl2 ccache conan ninja pulseaudio llvm ffmpeg mbedtls zstd openssl gpm pkgconfig
brew outdated cmake || brew upgrade cmake
pip3 install macpack
pip3 install conan==1.24.0
