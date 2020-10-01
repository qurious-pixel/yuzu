#!/bin/sh -ex

export PATH="/usr/local/sbin:$PATH"

#git -C $(brew --repo homebrew/core) fetch --unshallow

#softwareupdate --all --install --force
#brew cask pin java
brew pin protobuf
#brew update
#brew cask uninstall --force java
brew link gettext
#brew doctor
brew install p7zip qt5 sdl2 ccache conan ninja ffmpeg llvm boost openssl pulseaudio mbedtls zstd openssl gpm pkgconfig
brew outdated cmake || brew upgrade cmake
#brew upgrade gcc
pip3 install macpack
#brew install unicorn && UNICORN_QEMU_FLAGS="--python=`whereis python`" pip3 install unicorn
pip3 install conan
