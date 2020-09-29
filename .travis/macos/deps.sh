#!/bin/sh -ex

brew update
brew install p7zip qt5 sdl2 ccache conan ninja
brew outdated cmake || brew upgrade cmake
brew upgrade gcc
pip3 install macpack
brew install unicorn && UNICORN_QEMU_FLAGS="--python=`whereis python`" pip3 install unicorn
#pip3 install conan
