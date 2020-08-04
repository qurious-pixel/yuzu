#!/bin/bash -ex

SOURCEURL=`base64 -d <<<"aHR0cHM6Ly93d3cyNi56aXBweXNoYXJlLmNvbS9kL2tvVDJ1NVJ4LzE3MTY3L3l1enUtd2luZG93cy1tc3ZjLXNvdXJjZS0yMDIwMDgwMy03YWIzMTE1YTMuN3o="`

ln -s /home/yuzu/.conan /root
mkdir -p /tmp/source
cd /tmp/source
curl -sLO "http://mirrors.kernel.org/ubuntu/pool/universe/p/p7zip/p7zip_16.02+dfsg-6_amd64.deb"
dpkg -i p7zip_16.02+dfsg-6_amd64.deb
ls -al
curl -sLO $SOURCEURL
ls -al
7za x `ls | grep msvc`
ls -al
mv yuzu-*/ yuzu/
cd yuzu/

mkdir build && cd build

cmake .. -G Ninja -DYUZU_USE_BUNDLED_UNICORN=ON -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=/usr/lib/ccache/gcc -DCMAKE_CXX_COMPILER=/usr/lib/ccache/g++ -DENABLE_COMPATIBILITY_LIST_DOWNLOAD=ON 

ninja

#cat /yuzu/build/CMakeFiles/CMakeError.log | curl -F 'f:1=<-' ix.io

cd /tmp
# curl -sLO "https://raw.githubusercontent.com/qurious-pixel/yuzu/master/.travis/appimage/appimage.sh"
chmod a+x appimage.sh
./appimage.sh
