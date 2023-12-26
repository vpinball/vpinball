#!/bin/bash

set -e

SDL2_VERSION=2.28.5
SDL2_IMAGE_VERSION=2.6.3

PINMAME_SHA=be7a86f95f4aee347ce44cabd6cb055053da108a
LIBSERUM_SHA=ea90a5460b47d77e4cf1deacdacddbdb94c25067
LIBZEDMD_SHA=499b1c094d49ae9bd988326475c51686b1415186
LIBALTSOUND_SHA=816cc987db61c428c61746b65ab30aa765c87116

if [[ $(uname) == "Linux" ]]; then
	NUM_PROCS=$(nproc)
elif [[ $(uname) == "Darwin" ]]; then
	NUM_PROCS=$(sysctl -n hw.ncpu)
else
	NUM_PROCS=1
fi

echo "Building external libraries..."
echo "  SDL2_VERSION: ${SDL2_VERSION}"
echo "  SDL2_IMAGE_VERSION: ${SDL2_IMAGE_VERSION}"
echo "  PINMAME_SHA: ${PINMAME_SHA}"
echo "  LIBSERUM_SHA: ${LIBSERUM_SHA}"
echo "  LIBZEDMD_SHA: ${LIBZEDMD_SHA}"
echo "  LIBALTSOUND_SHA: ${LIBALTSOUND_SHA}"
echo "  NUM_PROCS: ${NUM_PROCS}"
echo ""

rm -rf external
mkdir external
mkdir external/include
mkdir external/lib

rm -rf tmp
mkdir tmp
cd tmp

#
# build freeimage, and copy to external
#

curl -sL https://downloads.sourceforge.net/project/freeimage/Source%20Distribution/3.18.0/FreeImage3180.zip -o FreeImage3180.zip
unzip FreeImage3180.zip
cd FreeImage
cp ../../freeimage/Android.mk Android.mk
cd ..

$ANDROID_NDK_HOME/ndk-build \
	-C FreeImage \
	-j${NUM_PROC} \
	NDK_PROJECT_PATH=$ANDROID_NDK_HOME \
	APP_BUILD_SCRIPT=$(pwd)/FreeImage/Android.mk \
	APP_STL="c++_static" \
	APP_PLATFORM=android-19 \
	APP_ABI=arm64-v8a \
	NDK_OUT=$(pwd)/obj \
	NDK_LIBS_OUT=$(pwd)/libs

cp libs/arm64-v8a/libFreeImage.so ../external/lib

#
# download bass24 and copy to external
#

curl -sL https://www.un4seen.com/files/bass24-android.zip -o bass.zip
unzip bass.zip 
cp libs/arm64-v8a/libbass.so ../external/lib

#
# build SDL2 and copy to external
# (derived from: https://github.com/AlexanderAgd/SDL2-Android)
#

curl -sL https://github.com/libsdl-org/SDL/releases/download/release-${SDL2_VERSION}/SDL2-${SDL2_VERSION}.zip -o SDL2-${SDL2_VERSION}.zip
unzip SDL2-${SDL2_VERSION}.zip
cp -r SDL2-${SDL2_VERSION}/include ../external/include/SDL2

$ANDROID_NDK_HOME/ndk-build \
	-C SDL2-${SDL2_VERSION} \
	-j${NUM_PROC} \
	NDK_PROJECT_PATH=$ANDROID_NDK_HOME \
	APP_BUILD_SCRIPT=$(pwd)/SDL2-${SDL2_VERSION}/Android.mk \
	APP_PLATFORM=android-19 \
	APP_ABI=arm64-v8a \
	NDK_OUT=$(pwd)/obj \
	NDK_LIBS_OUT=$(pwd)/libs

cp libs/arm64-v8a/libSDL2.so ../external/lib

#
# build SDL2_image and copy to external
# (derived from: https://github.com/AlexanderAgd/SDL2-Android)
#

curl -sL https://github.com/libsdl-org/SDL_image/releases/download/release-${SDL2_IMAGE_VERSION}/SDL2_image-${SDL2_IMAGE_VERSION}.zip -o SDL2_image-${SDL2_IMAGE_VERSION}.zip
unzip SDL2_image-${SDL2_IMAGE_VERSION}.zip
cp -r SDL2_image-${SDL2_IMAGE_VERSION}/SDL_image.h ../external/include/SDL2

MK_ADDON=$'include $(CLEAR_VARS)\\\n'
MK_ADDON+=$'LOCAL_MODULE := SDL2\\\n'
MK_ADDON+=$'LOCAL_SRC_FILES := '"$(pwd)/libs/arm64-v8a"$'/libSDL2.so\\\n'
MK_ADDON+=$'LOCAL_EXPORT_C_INCLUDES += '"$(pwd)/SDL2-${SDL2_VERSION}/include"$'\\\n'
MK_ADDON+="include \$(PREBUILT_SHARED_LIBRARY)"

sed -e $'/(call my-dir)/a\\\n'"$MK_ADDON" SDL2_image-${SDL2_IMAGE_VERSION}/Android.mk > tmp_mk
cp tmp_mk SDL2_image-${SDL2_IMAGE_VERSION}/Android.mk

$ANDROID_NDK_HOME/ndk-build \
	-C SDL2_image-${SDL2_IMAGE_VERSION} \
	-j${NUM_PROC} \
	NDK_PROJECT_PATH=$ANDROID_NDK_HOME \
	APP_BUILD_SCRIPT="$(pwd)/SDL2_image-${SDL2_IMAGE_VERSION}/Android.mk" \
	APP_PLATFORM=android-19 \
	APP_ABI=arm64-v8a \
	APP_ALLOW_MISSING_DEPS=true \
	NDK_OUT=$(pwd)/obj \
	NDK_LIBS_OUT=$(pwd)/libs

cp libs/arm64-v8a/libSDL2_image.so ../external/lib

#
# build libpinmame and copy to external
#

curl -sL https://github.com/vpinball/pinmame/archive/${PINMAME_SHA}.zip -o pinmame.zip
unzip pinmame.zip
cd pinmame-$PINMAME_SHA
cp src/libpinmame/libpinmame.h ../../external/include
cp cmake/libpinmame/CMakeLists_android-arm64-v8a.txt CMakeLists.txt
cmake -DCMAKE_BUILD_TYPE=Release -B build
cmake --build build -- -j${NUM_PROCS}
cp build/libpinmame.3.6.so ../../external/lib
cd ..

#
# build libserum and copy to external
#

curl -sL https://github.com/zesinger/libserum/archive/${LIBSERUM_SHA}.zip -o libserum.zip
unzip libserum.zip
cd libserum-$LIBSERUM_SHA
cp src/serum-decode.h ../../external/include
cmake -DCMAKE_BUILD_TYPE=Release -DUSE_ANDROID=ON -B build
cmake --build build -- -j${NUM_PROCS}
cp build/libserum.so ../../external/lib
cd ..

#
# build libzedmd and copy to external
#

curl -sL https://github.com/PPUC/libzedmd/archive/${LIBZEDMD_SHA}.zip -o libzedmd.zip
unzip libzedmd.zip
cd libzedmd-$LIBZEDMD_SHA
cp src/ZeDMD.h ../../external/include
cmake -DCMAKE_BUILD_TYPE=Release -DUSE_ANDROID=ON -B build
cmake --build build -- -j${NUM_PROCS}
cp build/libzedmd.so ../../external/lib
cd ..

#
# build libaltsound and copy to external
#

curl -sL https://github.com/jsm174/libaltsound/archive/${LIBALTSOUND_SHA}.zip -o libaltsound.zip
unzip libaltsound.zip
cd libaltsound-$LIBALTSOUND_SHA
cp src/altsound.h ../../external/include
platforms/android/arm64-v8a/external.sh
cmake -DPLATFORM=android -DARCH=arm64-v8a -DBUILD_STATIC=OFF -DCMAKE_BUILD_TYPE=Release -B build
cmake --build build -- -j${NUM_PROCS}
cp build/libaltsound.so ../../external/lib
cd ..