#!/bin/bash -ex

mkdir -p "$HOME/.ccache"
docker run -e ENABLE_COMPATIBILITY_REPORTING --env-file .travis/common/travis-ci.env -v $(pwd):/yuzu -v "$HOME/.ccache":/root/.ccache quriouspixel/rpcs3-docker:latest /bin/bash /yuzu/.travis/appimage/docker.sh
