#!/bin/bash -ex

mkdir -p "$HOME/.ccache"
#docker run -e ENABLE_COMPATIBILITY_REPORTING --env-file .travis/common/travis-ci.env -v $(pwd):/yuzu -v "$HOME/.ccache":/root/.ccache quriouspixel/rpcs3-docker:latest /bin/bash /yuzu/.travis/appimage/docker.sh
#docker run -e ENABLE_COMPATIBILITY_REPORTING --env-file .travis/common/travis-ci.env -v $(pwd):/yuzu -v "$HOME/.ccache":/root/.ccache rpcs3/rpcs3-travis-xenial:1.4 /bin/bash /yuzu/.travis/appimage/docker.sh
#docker run -e ENABLE_COMPATIBILITY_REPORTING --env-file .travis/common/travis-ci.env -v $(pwd):/yuzu -v "$HOME/.ccache":/root/.ccache yuzuemu/build-environments:linux-fresh /bin/bash /yuzu/.travis/appimage/docker.sh
docker run -e ENABLE_COMPATIBILITY_REPORTING --env-file .travis/common/travis-ci.env -v $(pwd):/yuzu -v "$HOME/.ccache":/root/.ccache --privileged quriouspixel/yuzu:latest /bin/bash /yuzu/.travis/appimage/docker.sh
