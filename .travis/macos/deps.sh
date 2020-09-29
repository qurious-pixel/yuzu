#!/bin/sh -ex

brew update
brew install p7zip qt5 sdl2 ccache conan ninja gcc
brew outdated cmake || brew upgrade cmake
pip3 install macpack
#pip3 install conan
