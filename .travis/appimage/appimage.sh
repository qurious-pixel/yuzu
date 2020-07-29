#!/bin/bash -ex

BUILDBIN=/yuzu/build/bin
BINFILE=yuzu-x86_64.AppImage
LOG_FILE=$HOME/curl.log

cd $HOME
	curl -sLO "https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage"
	chmod a+x linuxdeployqt*.AppImage

mkdir -p squashfs-root/usr/bin
cp -P "$BUILDBIN"/yuzu $HOME/squashfs-root/usr/bin/
source /opt/qt514/bin/qt514-env.sh
curl -sL https://raw.githubusercontent.com/yuzu-emu/yuzu/master/dist/yuzu.svg -o ./squashfs-root/yuzu.svg
curl -sL https://raw.githubusercontent.com/yuzu-emu/yuzu/master/dist/yuzu.desktop -o ./squashfs-root/yuzu.desktop
curl -sL https://github.com/darealshinji/AppImageKit-checkrt/releases/download/continuous/AppRun-patched-x86_64 -o ./squashfs-root/AppRun
curl -sL https://github.com/AppImage/AppImageKit/releases/download/continuous/runtime-x86_64 -o ./squashfs-root/runtime
chmod a+x ./squashfs-root/runtime
chmod a+x ./squashfs-root/AppRun

	./linuxdeployqt-continuous-x86_64.AppImage squashfs-root/usr/bin/yuzu -appimage -unsupported-allow-new-glibc -no-copy-copyright-files -no-translations -bundle-non-qt-libs

mkdir $HOME/artifacts/
mv yuzu-x86_64.AppImage $HOME/artifacts
touch $HOME/curl.log
cd $HOME/artifacts
curl --progress-bar --upload-file $BINFILE https://transfer.sh/$BINFILE | tee -a "$LOG_FILE" ; test ${PIPESTATUS[0]} -eq 0
echo "" >> $LOG_FILE
cat $LOG_FILE
