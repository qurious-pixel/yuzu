#!/bin/bash -ex

ls -al artifacts/
wget -c https://github.com/tcnksm/ghr/releases/download/v0.13.0/ghr_v0.13.0_darwin_amd64.zip
unzip ghr_v0.13.0_darwin_amd64.zip      
ghr_v0.13.0_darwin_amd64/ghr -recreate -n 'yuzu mac 10.13' -b "Travis CI build log ${TRAVIS_BUILD_WEB_URL}" mac_10_13 artifacts/
