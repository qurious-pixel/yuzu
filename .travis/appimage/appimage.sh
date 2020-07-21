#!/bin/bash -ex

mkdir build && cd build
	curl -sLO "https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage"
	chmod a+x linuxdeployqt*.AppImage

mkdir squashfs-root
cp -P "$DIR_NAME"/yuzu squashfs-root/usr/bin/
curl -sL https://raw.githubusercontent.com/yuzu-emu/yuzu/master/dist/yuzu.svg -o ./squashfs-root/yuzu.svg
curl -sL https://raw.githubusercontent.com/yuzu-emu/yuzu/master/dist/yuzu.desktop -o ./squashfs-root/yuzu.desktop
curl -sL https://github.com/darealshinji/AppImageKit-checkrt/releases/download/continuous/AppRun-patched-x86_64 -o ./squashfs-root/AppRun
curl -sL https://github.com/AppImage/AppImageKit/releases/download/continuous/runtime-x86_64 -o ./squashfs-root/runtime
chmod a+x ./squashfs-root/runtime
chmod a+x ./squashfs-root/AppRun

	./linuxdeployqt-continuous-x86_64.AppImage squashfs-root/usr/bin/yuzu -appimage -unsupported-allow-new-glibc -no-copy-copyright-files -no-translations --output yuzu-`$REV_NAME`.AppImage

mv yuzu-`$REV_NAME`.AppImage artifacts/
