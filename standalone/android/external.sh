#!/bin/bash

SDL2_VERSION=2.28.3
SDL2_TTF_VERSION=2.20.2
SDL2_IMAGE_VERSION=2.6.3

PINMAME_SHA=ff9ac600215032172c3b3a4d223562db34ee4169
SERUM_SHA=828c087986f95ca7dbf6c3de2ca8df4200ed011b
ZEDMD_SHA=499b1c094d49ae9bd988326475c51686b1415186

if [[ $(uname) == "Linux" ]]; then
	NUM_PROCS=$(nproc)
elif [[ $(uname) == "Darwin" ]]; then
	NUM_PROCS=$(sysctl -n hw.ncpu)
else
	NUM_PROCS=1
fi

echo "Building external libraries..."
echo "  SDL2_VERSION: ${SDL2_VERSION}"
echo "  SDL2_TTF_VERSION: ${SDL2_TTF_VERSION}"
echo "  SDL2_IMAGE_VERSION: ${SDL2_IMAGE_VERSION}"
echo "  PINMAME_SHA: ${PINMAME_SHA}"
echo "  SERUM_SHA: ${SERUM_SHA}"
echo "  ZEDMD_SHA: ${ZEDMD_SHA}"
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
mv libs/arm64-v8a/libbass.so ../external/lib

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
# build SDL2_ttf and copy to external
# (derived from: https://github.com/AlexanderAgd/SDL2-Android)
#

curl -sL https://github.com/libsdl-org/SDL_ttf/releases/download/release-${SDL2_TTF_VERSION}/SDL2_ttf-${SDL2_TTF_VERSION}.zip -o SDL2_ttf-${SDL2_TTF_VERSION}.zip
unzip SDL2_ttf-${SDL2_TTF_VERSION}.zip
cp -r SDL2_ttf-${SDL2_TTF_VERSION}/SDL_ttf.h ../external/include/SDL2

MK_ADDON=$'include $(CLEAR_VARS)\\\n'
MK_ADDON+=$'LOCAL_MODULE := SDL2\\\n'
MK_ADDON+=$'LOCAL_SRC_FILES := '"$(pwd)/libs/arm64-v8a"$'/libSDL2.so\\\n'
MK_ADDON+=$'LOCAL_EXPORT_C_INCLUDES += '"$(pwd)/SDL2-${SDL2_VERSION}/include"$'\\\n'
MK_ADDON+="include \$(PREBUILT_SHARED_LIBRARY)"

sed -e $'/(call my-dir)/a\\\n'"$MK_ADDON" SDL2_ttf-${SDL2_TTF_VERSION}/Android.mk > tmp_mk
cp tmp_mk SDL2_ttf-${SDL2_TTF_VERSION}/Android.mk

$ANDROID_NDK_HOME/ndk-build \
	-C SDL2_ttf-${SDL2_TTF_VERSION} \
	-j${NUM_PROC} \
	NDK_PROJECT_PATH=$ANDROID_NDK_HOME \
	APP_BUILD_SCRIPT="$(pwd)/SDL2_ttf-${SDL2_TTF_VERSION}/Android.mk" \
	APP_PLATFORM=android-19 \
	APP_ABI=arm64-v8a \
	APP_ALLOW_MISSING_DEPS=true \
	NDK_OUT=$(pwd)/obj \
	NDK_LIBS_OUT=$(pwd)/libs

cp libs/arm64-v8a/libSDL2_ttf.so ../external/lib

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

curl -sL https://github.com/zesinger/libserum/archive/${SERUM_SHA}.zip -o libserum.zip
unzip libserum.zip
cd libserum-$SERUM_SHA
cp src/serum-decode.h ../../external/include
cmake -DCMAKE_BUILD_TYPE=Release -DUSE_ANDROID=ON -B build
cmake --build build -- -j${NUM_PROCS}
cp build/libserum.so ../../external/lib
cd ..

#
# build libzedmd and copy to external
#

curl -sL https://github.com/PPUC/libzedmd/archive/${ZEDMD_SHA}.zip -o libzedmd.zip
unzip libzedmd.zip
cd libzedmd-$ZEDMD_SHA
cp src/ZeDMD.h ../../external/include
cmake -DCMAKE_BUILD_TYPE=Release -DUSE_ANDROID=ON  -B build
cmake --build build -- -j${NUM_PROCS}
cp build/libzedmd.so ../../external/lib
cd ..
