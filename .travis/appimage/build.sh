#!/bin/bash -ex

mkdir -p "$HOME/.ccache"
docker run -u root -e ENABLE_COMPATIBILITY_REPORTING --env-file .travis/common/travis-ci.env -v $(pwd):/yuzu -v "$HOME/.ccache":/root/.ccache --name yuzu_appimage quriouspixel/yuzu:latest /bin/bash /yuzu/.travis/appimage/docker.sh
