#!/bin/bash -ex

SOURCEURL=`base64 -d <<<"aHR0cHM6Ly9jZG4tMTE3LmFub25maWxlcy5jb20vMTIzNW0yajNvMi84NjQ5NDQyYS0xNTk2MjUxMzE4L1l1enVFQS04MjIuN3o="`

ln -s /home/yuzu/.conan /root
mkdir -p /tmp/source
cd /tmp/source
curl -sLO $SOURCEURL
7z *.7z
cd /yuzu

mkdir build && cd build

cmake /yuzu -G Ninja -DYUZU_USE_BUNDLED_UNICORN=ON -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=/usr/lib/ccache/gcc -DCMAKE_CXX_COMPILER=/usr/lib/ccache/g++ -DENABLE_COMPATIBILITY_LIST_DOWNLOAD=ON 

ninja

#cat /yuzu/build/CMakeFiles/CMakeError.log | curl -F 'f:1=<-' ix.io

cd /tmp
curl -sLO "https://raw.githubusercontent.com/qurious-pixel/yuzu/master/.travis/appimage/appimage.sh"
chmod a+x appimage.sh
./appimage.sh
