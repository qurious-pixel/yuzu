#!/bin/bash -ex

SOURCEURL=`base64 -d <<<"aHR0cHM6Ly9zcnYtZmlsZTE5LmdvZmlsZS5pby9kb3dubG9hZC9BZUN3cHcvbXN2Yy1zb3VyY2UtMjAyMDA3MzAtZWRkODdjZjQwLnRhci54eg=="`

ln -s /home/yuzu/.conan /root
mkdir -p /tmp/source
cd /tmp/source
ls -al
curl -sLO $SOURCEURL
ls -al
tar -xJf *.xz
ls -al
mv yuzu-*/ yuzu/
cd yuzu/

mkdir build && cd build

cmake .. -G Ninja -DYUZU_USE_BUNDLED_UNICORN=ON -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=/usr/lib/ccache/gcc -DCMAKE_CXX_COMPILER=/usr/lib/ccache/g++ -DENABLE_COMPATIBILITY_LIST_DOWNLOAD=ON 

ninja

#cat /yuzu/build/CMakeFiles/CMakeError.log | curl -F 'f:1=<-' ix.io

cd /tmp
curl -sLO "https://raw.githubusercontent.com/qurious-pixel/yuzu/master/.travis/appimage/appimage.sh"
chmod a+x appimage.sh
./appimage.sh
