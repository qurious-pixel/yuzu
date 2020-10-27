#!/bin/bash -ex

set -o pipefail

export MACOSX_DEPLOYMENT_TARGET=10.13
export Qt5_DIR=$(brew --prefix)/opt/qt5
export UNICORNDIR=$(pwd)/externals/unicorn
export PATH="/usr/local/opt/ccache/libexec:/usr/local/opt/llvm/bin:$PATH"

export CC="clang"
export CXX="clang++"
export LDFLAGS="-L/usr/local/opt/llvm/lib -L/usr/local/opt/openssl@1.1/lib"
export CPPFLAGS="-I/usr/local/opt/llvm/include -I/usr/local/opt/openssl@1.1/include"

#download from git
git clone --recursive --single-branch -b latest-mac-stuff https://github.com/jroweboy/yuzu.git
cd yuzu
sed -i -e 's|10.15.0|10.13.1|g' CMakeLists.txt

cp -r externals/MoltenVK/include/{MoltenVK,vulkan-portability} ./src/

# TODO: Build using ninja instead of make
mkdir build && cd build
cmake --version
cmake .. -GNinja -DYUZU_USE_BUNDLED_UNICORN=ON -DCMAKE_BUILD_TYPE=Release -DENABLE_COMPATIBILITY_LIST_DOWNLOAD=ON -DCMAKE_C_COMPILER="$CC" -DCMAKE_CXX_COMPILER="$CXX"
ninja

ccache -s
