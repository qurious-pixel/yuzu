#!/bin/sh -ex

export PATH="/usr/local/sbin:$PATH"

#git -C $(brew --repo homebrew/core) fetch --unshallow

#softwareupdate --all --install --force
#brew cask pin java
brew pin protobuf
brew pin postgresql
brew update --force
#brew cask uninstall --force java
brew link gettext
#brew doctor
brew install p7zip sdl2 ccache conan ninja ffmpeg llvm pulseaudio mbedtls gpm pkgconfig
brew upgrade boost qt openssl@1.1 zstd
brew outdated cmake || brew upgrade cmake
#brew upgrade gcc
pip3 install macpack
#brew install unicorn && UNICORN_QEMU_FLAGS="--python=`whereis python`" pip3 install unicorn
pip3 install conan
