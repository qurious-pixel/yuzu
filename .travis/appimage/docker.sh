#!/bin/bash -ex

branch=`echo ${GITHUB_REF##*/}`

chown -R 1027:1027 /yuzu
ln -s /home/yuzu/.conan /root

cd /yuzu

git clone https://github.com/yuzu-emu/yuzu-mainline.git
cd yuzu-mainline/
git submodule update --init --recursive

mkdir build && cd build

cmake .. -G Ninja -DYUZU_USE_BUNDLED_UNICORN=ON -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=/usr/lib/ccache/gcc -DCMAKE_CXX_COMPILER=/usr/lib/ccache/g++ -DENABLE_COMPATIBILITY_LIST_DOWNLOAD=ON 

ninja

#cat /yuzu/build/CMakeFiles/CMakeError.log | curl -F 'f:1=<-' ix.io

cd /tmp
curl -sLO "https://raw.githubusercontent.com/qurious-pixel/yuzu/$branch/.travis/appimage/appimage.sh"
chmod a+x appimage.sh
./appimage.sh
