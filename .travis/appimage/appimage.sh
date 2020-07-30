#!/bin/bash -ex

BUILDBIN=/yuzu/build/bin
BINFILE=yuzu-x86_64.AppImage
LOG_FILE=$HOME/curl.log

# QT 5.14.2
# source /opt/qt514/bin/qt514-env.sh
QT_BASE_DIR=/opt/qt514
export QTDIR=$QT_BASE_DIR
export PATH=$QT_BASE_DIR/bin:$PATH
export LD_LIBRARY_PATH=$QT_BASE_DIR/lib/x86_64-linux-gnu:$QT_BASE_DIR/lib:$LD_LIBRARY_PATH
export PKG_CONFIG_PATH=$QT_BASE_DIR/lib/pkgconfig:$PKG_CONFIG_PATH

cd /tmp
	curl -sLO "https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage"
	curl -sLO "https://github.com/qurious-pixel/yuzu/raw/merge/.travis/appimage/crypto-libs.tar.gz"
	tar -xzf crypto-libs.tar.gz
	chmod a+x linuxdeployqt*.AppImage
./linuxdeployqt-continuous-x86_64.AppImage --appimage-extract
cd $HOME
mkdir -p squashfs-root/usr/bin
cp -P "$BUILDBIN"/yuzu $HOME/squashfs-root/usr/bin/

curl -sL https://raw.githubusercontent.com/yuzu-emu/yuzu/master/dist/yuzu.svg -o ./squashfs-root/yuzu.svg
curl -sL https://raw.githubusercontent.com/yuzu-emu/yuzu/master/dist/yuzu.desktop -o ./squashfs-root/yuzu.desktop
curl -sL https://github.com/darealshinji/AppImageKit-checkrt/releases/download/continuous/AppRun-patched-x86_64 -o ./squashfs-root/AppRun
curl -sL https://github.com/AppImage/AppImageKit/releases/download/continuous/runtime-x86_64 -o ./squashfs-root/runtime
chmod a+x ./squashfs-root/runtime
chmod a+x ./squashfs-root/AppRun
cp /tmp/libssl.so.47 /tmp/libcrypto.so.45 /usr/lib/x86_64-linux-gnu/

# /tmp/squashfs-root/AppRun $HOME/squashfs-root/usr/bin/yuzu -appimage -unsupported-allow-new-glibc -no-copy-copyright-files -no-translations -bundle-non-qt-libs
/tmp/squashfs-root/AppRun $HOME/squashfs-root/usr/bin/yuzu -unsupported-allow-new-glibc -no-copy-copyright-files -no-translations -bundle-non-qt-libs
export PATH=$(readlink -f /tmp/squashfs-root/usr/bin/):$PATH
/tmp/squashfs-root/usr/bin/appimagetool $HOME/squashfs-root

mkdir $HOME/artifacts/
mv yuzu-x86_64.AppImage $HOME/artifacts
touch $HOME/curl.log
cd $HOME/artifacts
# curl --progress-bar --upload-file $BINFILE https://transfer.sh/$BINFILE | tee -a "$LOG_FILE" ; test ${PIPESTATUS[0]} -eq 0
# echo "" >> $LOG_FILE
# cat $LOG_FILE

