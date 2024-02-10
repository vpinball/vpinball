#!/bin/bash

set -e

SDL2_VERSION=2.28.5
SDL2_IMAGE_VERSION=2.6.3
SDL2_TTF_VERSION=2.20.2

PINMAME_SHA=4bda9d4c0efc26f8bf6e8442475d45caddc25e2b
LIBALTSOUND_SHA=428bdb1085f486f82279574a74e5a0959107a8f6
LIBDMDUTIL_SHA=342f687a9067f4f3ae2fb7863ef296b3f44253e0

NUM_PROCS=$(sysctl -n hw.ncpu)

echo "Building external libraries..."
echo "  SDL2_VERSION: ${SDL2_VERSION}"
echo "  SDL2_IMAGE_VERSION: ${SDL2_IMAGE_VERSION}"
echo "  SDL2_TTF_VERSION: ${SDL2_TTF_VERSION}"
echo "  PINMAME_SHA: ${PINMAME_SHA}"
echo "  LIBALTSOUND_SHA: ${LIBALTSOUND_SHA}"
echo "  LIBDMDUTIL_SHA: ${LIBDMDUTIL_SHA}"
echo ""

if [ -z "${BUILD_TYPE}" ]; then
   BUILD_TYPE="Release"
fi

echo "Build type: ${BUILD_TYPE}"
echo "Procs: ${NUM_PROCS}"
echo ""

rm -rf external
mkdir external
mkdir external/framework
mkdir external/include
mkdir external/lib

rm -rf tmp
mkdir tmp
cd tmp

#
# build freeimage and copy to external
#

curl -sL https://downloads.sourceforge.net/project/freeimage/Source%20Distribution/3.18.0/FreeImage3180.zip -o FreeImage3180.zip
unzip FreeImage3180.zip
cd FreeImage
patch -i ../../freeimage/FreeImage3180.patch
cp ../../freeimage/Makefile.tvos.arm64 .
make -f Makefile.tvos.arm64 -j${NUM_PROCS}
cp Dist/libfreeimage.a ../../external/lib
cd ..

#
# copy bass24 framework to external
# 
# Note: The BASS library needs to be replaced with the tvOS version.
# Leaving the iOS version here so we can test the compile. It will fail when linking. 
#

curl -s https://www.un4seen.com/files/bass24-ios.zip -o bass.zip
unzip bass.zip 
cp -r bass.xcframework/ios-arm64_armv7_armv7s/bass.framework ../external/framework

#
# build SDL2 and copy to external
#

curl -sL https://github.com/libsdl-org/SDL/releases/download/release-${SDL2_VERSION}/SDL2-${SDL2_VERSION}.zip -o SDL2-${SDL2_VERSION}.zip
unzip SDL2-${SDL2_VERSION}.zip
cp -r SDL2-${SDL2_VERSION}/include ../external/include/SDL2

xcrun xcodebuild -project "SDL2-${SDL2_VERSION}/Xcode/SDL/SDL.xcodeproj" -target "Static Library-tvOS" -sdk appletvos -configuration Release clean build CONFIGURATION_BUILD_DIR="$(pwd)/sdl-build"
cp sdl-build/libSDL2.a ../external/lib

#
# build SDL2_image and copy to external
#

curl -sL https://github.com/libsdl-org/SDL_image/releases/download/release-${SDL2_IMAGE_VERSION}/SDL2_image-${SDL2_IMAGE_VERSION}.zip -o SDL2_image-${SDL2_IMAGE_VERSION}.zip
unzip SDL2_image-${SDL2_IMAGE_VERSION}.zip
cp -r SDL2_image-${SDL2_IMAGE_VERSION}/SDL_image.h ../external/include/SDL2

xcrun xcodebuild -project "SDL2_image-${SDL2_IMAGE_VERSION}/Xcode/SDL_image.xcodeproj" -target "Static Library" -sdk appletvos -configuration Release clean build CONFIGURATION_BUILD_DIR="$(pwd)/sdl_image-build"
cp sdl_image-build/libSDL2_image.a ../external/lib

#
# build SDL2_ttf and copy to external
#

curl -sL https://github.com/libsdl-org/SDL_ttf/releases/download/release-${SDL2_TTF_VERSION}/SDL2_ttf-${SDL2_TTF_VERSION}.zip -o SDL2_ttf-${SDL2_TTF_VERSION}.zip
unzip SDL2_ttf-${SDL2_TTF_VERSION}.zip
cp -r SDL2_ttf-${SDL2_TTF_VERSION}/SDL_ttf.h ../external/include/SDL2

xcrun xcodebuild -project "SDL2_ttf-${SDL2_TTF_VERSION}/Xcode/SDL_ttf.xcodeproj" -target "Static Library" -sdk appletvos -configuration Release clean build CONFIGURATION_BUILD_DIR="$(pwd)/sdl_ttf-build"
cp sdl_ttf-build/libSDL2_ttf.a ../external/lib

#
# build libpinmame and copy to external
#

curl -sL https://github.com/vpinball/pinmame/archive/${PINMAME_SHA}.zip -o pinmame.zip
unzip pinmame.zip
cd pinmame-$PINMAME_SHA
cp src/libpinmame/libpinmame.h ../../external/include
cp cmake/libpinmame/CMakeLists_tvos-arm64.txt CMakeLists.txt
cmake -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -B build
cmake --build build -- -j${NUM_PROCS}
cp build/libpinmame.3.6.a ../../external/lib
cd ..

#
# build libaltsound and copy to external
#

curl -sL https://github.com/vpinball/libaltsound/archive/${LIBALTSOUND_SHA}.zip -o libaltsound.zip
unzip libaltsound.zip
cd libaltsound-$LIBALTSOUND_SHA
cp src/altsound.h ../../external/include
platforms/tvos/arm64/external.sh
cmake -DPLATFORM=tvos -DARCH=arm64 -DBUILD_SHARED=OFF -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -B build
cmake --build build -- -j${NUM_PROCS}
cp build/libaltsound.a ../../external/lib
cd ..

#
# build libdmdutil (and libserum, libzedmd) and copy to external
#

curl -sL https://github.com/vpinball/libdmdutil/archive/${LIBDMDUTIL_SHA}.zip -o libdmdutil.zip
unzip libdmdutil.zip
cd libdmdutil-$LIBDMDUTIL_SHA
cp -r include/DMDUtil ../../external/include
platforms/tvos/arm64/external.sh
cmake -DPLATFORM=tvos -DARCH=arm64 -DBUILD_SHARED=OFF -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -B build
cmake --build build -- -j${NUM_PROCS}
cp third-party/build-libs/tvos/arm64/libserum.a ../../external/lib
cp third-party/build-libs/tvos/arm64/libzedmd.a ../../external/lib
cp build/libdmdutil.a ../../external/lib
cd ..
