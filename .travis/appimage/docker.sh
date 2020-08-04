#!/bin/bash -ex

SOURCEURL=`base64 -d <<<"aHR0cHM6Ly90cmFuc2ZlcnNoLmNvbS84a3VnbS95dXp1LXdpbmRvd3MtbXN2Yy1zb3VyY2UtMjAyMDA4MDMtN2FiMzExNWEzLjd6"`

ln -s /home/yuzu/.conan /root
mkdir -p /tmp/source
cd /tmp/source
curl -sLO "http://mirrors.kernel.org/ubuntu/pool/universe/p/p7zip/p7zip_16.02+dfsg-6_amd64.deb"
curl -sLO "http://mirrors.kernel.org/ubuntu/pool/universe/p/p7zip/p7zip-full_16.02+dfsg-6_amd64.deb"
dpkg -i *.deb
ls -al
curl -sLO $SOURCEURL
ls -al
ls /usr/bin | grep 7z
7z x `ls | grep msvc`
which 7z
ls -al
mv yuzu-*/ yuzu/
cd yuzu/

find -path ./dist -prune -o -type f -exec sed -i 's/\r$//' {} ';'
wget https://raw.githubusercontent.com/PineappleEA/Pineapple-Linux/master/inject-git-info.patch
patch -p1 < inject-git-info.patch
msvc=$(echo "${PWD##*/}"|sed 's/.*-//')
mkdir -p build && cd build
cmake .. -G Ninja -DYUZU_USE_BUNDLED_UNICORN=ON -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=/usr/lib/ccache/gcc -DCMAKE_CXX_COMPILER=/usr/lib/ccache/g++ -DTITLE_BAR_FORMAT_IDLE="yuzu Early Access $title" -DTITLE_BAR_FORMAT_RUNNING="yuzu Early Access $title | {3}" -DENABLE_COMPATIBILITY_LIST_DOWNLOAD=ON -DGIT_BRANCH="HEAD" -DGIT_DESC="$msvc" 

ninja

#cat /yuzu/build/CMakeFiles/CMakeError.log | curl -F 'f:1=<-' ix.io

cd /tmp
# curl -sLO "https://raw.githubusercontent.com/qurious-pixel/yuzu/master/.travis/appimage/appimage.sh"
chmod a+x appimage.sh
./appimage.sh
