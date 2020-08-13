#!/bin/sh -ex

brew update
brew install p7zip qt5 sdl2 ccache conan ninja
brew outdated cmake || brew upgrade cmake
pip3 install macpack
#pip3 install conan
