#!/bin/bash -ex

chown -R 1027:1027 /yuzu
su yuzu
whoami 

cd /yuzu

mkdir build && cd build

cmake /yuzu -G Ninja -DYUZU_USE_BUNDLED_UNICORN=ON -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=/usr/lib/ccache/gcc -DCMAKE_CXX_COMPILER=/usr/lib/ccache/g++ -DENABLE_COMPATIBILITY_LIST_DOWNLOAD=ON 

ninja


#cat /yuzu/build/CMakeFiles/CMakeError.log | curl -F 'f:1=<-' ix.io
