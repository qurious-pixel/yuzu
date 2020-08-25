#!/bin/bash -ex

branch=master-test

chown -R 1027:1027 /yuzu
ln -s /home/yuzu/.conan /root

#############################
cd /tmp

VULKANVER=1.2.150

	curl -sL -o Vulkan-Headers.tar.gz https://github.com/KhronosGroup/Vulkan-Headers/archive/v${VULKANVER}.tar.gz 	
	tar -xzf Vulkan-Headers*.tar.gz 
	cd Vulkan-Headers*/ 
	mkdir build && cd build 
	cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=$GCC_BINARY -DCMAKE_CXX_COMPILER=$GXX_BINARY -DCMAKE_INSTALL_PREFIX=/usr 
	ninja 
	ninja install 
	cd /tmp
	
	curl -sL -o Vulkan-Loader.tar.gz https://github.com/KhronosGroup/Vulkan-Loader/archive/v${VULKANVER}.tar.gz 
	tar -xzf Vulkan-Loader.tar.gz 
	cd Vulkan-Loader*/
	mkdir build && cd build 
	cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=$GCC_BINARY -DCMAKE_CXX_COMPILER=$GXX_BINARY -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_INSTALL_LIBDIR=/usr/lib -DCMAKE_INSTALL_SYSCONFDIR=/etc -DCMAKE_INSTALL_DATADIR=/share 
	ninja 
	ninja install 

###########################

cd /yuzu

mkdir build && cd build

cmake /yuzu -G Ninja -DYUZU_USE_BUNDLED_UNICORN=ON -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=/usr/lib/ccache/gcc -DCMAKE_CXX_COMPILER=/usr/lib/ccache/g++ -DENABLE_COMPATIBILITY_LIST_DOWNLOAD=ON 

ninja

#cat /yuzu/build/CMakeFiles/CMakeError.log | curl -F 'f:1=<-' ix.io

cd /tmp
curl -sLO "https://raw.githubusercontent.com/qurious-pixel/yuzu/$branch/.travis/appimage/appimage.sh"
chmod a+x appimage.sh
./appimage.sh
