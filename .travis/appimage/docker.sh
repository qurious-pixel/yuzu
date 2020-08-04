#!/bin/bash -ex

SOURCEURL=`base64 -d <<<"aHR0cHM6Ly90cmFuc2ZlcnNoLmNvbS84a3VnbS95dXp1LXdpbmRvd3MtbXN2Yy1zb3VyY2UtMjAyMDA4MDMtN2FiMzExNWEzLjd6"`

QT_BASE_DIR=/opt/qt514
export QTDIR=$QT_BASE_DIR
export PATH=$QT_BASE_DIR/bin:$PATH
export LD_LIBRARY_PATH=$QT_BASE_DIR/lib/x86_64-linux-gnu:$QT_BASE_DIR/lib:$LD_LIBRARY_PATH
export PKG_CONFIG_PATH=$QT_BASE_DIR/lib/pkgconfig:$PKG_CONFIG_PATH

ln -s /home/yuzu/.conan /root
mkdir -p /tmp/source
cd /tmp/source
curl -sLO $SOURCEURL
7z x `ls | grep msvc`
mv yuzu-*/ yuzu/
cd yuzu/

find -path ./dist -prune -o -type f -exec sed -i 's/\r$//' {} ';'
wget https://raw.githubusercontent.com/PineappleEA/Pineapple-Linux/master/inject-git-info.patch
patch -p1 < inject-git-info.patch
msvc=$(echo "${PWD##*/}"|sed 's/.*-//')
mkdir -p build && cd build

#cp /yuzu/src/web_service/web_backend.cpp /tmp/source/yuzu/src/web_service/
#cp /yuzu/src/input_common/sdl/sdl_impl.cpp /tmp/source/yuzu/src/input_common/sdl/

cmake .. -G Ninja -DYUZU_USE_BUNDLED_UNICORN=ON -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=/usr/lib/ccache/gcc -DCMAKE_CXX_COMPILER=/usr/lib/ccache/g++ -DTITLE_BAR_FORMAT_IDLE="yuzu Early Access $title" -DTITLE_BAR_FORMAT_RUNNING="yuzu Early Access $title | {3}" -DENABLE_COMPATIBILITY_LIST_DOWNLOAD=ON -DGIT_BRANCH="HEAD" -DGIT_DESC="$msvc" -DUSE_DISCORD_PRESENCE=ON
#cmake .. -G Ninja

ninja

#cat yuzu/build/CMakeFiles/CMakeError.log | curl -F 'f:1=<-' ix.io

cd /tmp
curl -sLO "https://raw.githubusercontent.com/qurious-pixel/yuzu/pineapple/.travis/appimage/appimage.sh"
chmod a+x appimage.sh
./appimage.sh
