#!/bin/bash -ex

set -o pipefail

export MACOSX_DEPLOYMENT_TARGET=10.14
export Qt5_DIR=$(brew --prefix)/opt/qt5
export UNICORNDIR=$(pwd)/externals/unicorn
#export UNICORNDIR=/usr/local/Cellar/unicorn/1.0.1
export PATH="/usr/local/opt/ccache/libexec:/usr/local/opt/llvm/bin:/usr/local/opt/openssl@1.1/bin:$PATH"

export CC="clang"
export CXX="clang++"
export LDFLAGS="-L/usr/local/opt/llvm/lib -L/usr/local/opt/openssl@1.1/lib"
export CPPFLAGS="-I/usr/local/opt/llvm/include -I/usr/local/opt/openssl@1.1v/include"
export CXXFLAGS='-std=c++17'

# Setup vulkan and gfx-rs/portability
curl -sLO https://github.com/gfx-rs/portability/releases/download/latest/gfx-portability-macos-latest.zip
unzip -: gfx-portability-macos-latest.zip
curl -sLO https://github.com/KhronosGroup/Vulkan-Headers/archive/sdk-1.1.106.0.zip
unzip -: sdk-*.zip
mkdir vulkan-sdk
ln -s "${PWD}"/Vulkan-Headers*/include vulkan-sdk/include
mkdir vulkan-sdk/lib
cp target/release/libportability.dylib vulkan-sdk/lib/libVulkan.dylib
# Let macdeployqt locate and install Vulkan library
install_name_tool -id "${PWD}"/vulkan-sdk/lib/libVulkan.dylib vulkan-sdk/lib/libVulkan.dylib
export VULKAN_SDK="${PWD}/vulkan-sdk"

# TODO: Build using ninja instead of make
mkdir build && cd build
cmake --version
#cmake .. -GNinja -DYUZU_USE_BUNDLED_UNICORN=ON -DYUZU_USE_QT_WEB_ENGINE=ON -DCMAKE_BUILD_TYPE=Release -DENABLE_COMPATIBILITY_LIST_DOWNLOAD=ON -DYUZU_ENABLE_COMPATIBILITY_REPORTING=${ENABLE_COMPATIBILITY_REPORTING:-"OFF"} -DUSE_DISCORD_PRESENCE=ON
cmake .. -DYUZU_USE_BUNDLED_UNICORN=ON -DCMAKE_BUILD_TYPE=Release -DENABLE_COMPATIBILITY_LIST_DOWNLOAD=ON -DCMAKE_C_COMPILER="$CC" -DCMAKE_CXX_COMPILER="$CXX"
#ninja
make clang-format 
make -j$(nproc)

ccache -s
