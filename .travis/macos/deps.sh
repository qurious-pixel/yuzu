#!/bin/sh -ex

brew update
brew install p7zip qt5 sdl2 ccache conan ninja openssl pulseaudio llvm ffmpeg mbedtls zstd openssl gpm pkgconfig
brew outdated cmake || brew upgrade cmake
pip3 install macpack
pip3 install conan==1.24.0
