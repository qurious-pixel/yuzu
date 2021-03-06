#!/bin/bash -ex

branch=`echo ${GITHUB_REF##*/}`

chown -R 1027:1027 /yuzu
ln -s /home/yuzu/.conan /root

cd /yuzu

mkdir build && cd build

cmake /yuzu -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=/usr/lib/ccache/gcc -DCMAKE_CXX_COMPILER=/usr/lib/ccache/g++ -DENABLE_COMPATIBILITY_LIST_DOWNLOAD=ON # -DYUZU_USE_BUNDLED_UNICORN=ON  

ninja

#cat /yuzu/build/CMakeFiles/CMakeError.log | curl -F 'f:1=<-' ix.io

cd /tmp
curl -sLO "https://raw.githubusercontent.com/qurious-pixel/yuzu/$branch/.travis/appimage/appimage.sh"
chmod a+x appimage.sh
./appimage.sh
