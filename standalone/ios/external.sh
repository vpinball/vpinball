#!/bin/bash

set -e

SDL2_VERSION=2.28.5
SDL2_IMAGE_VERSION=2.6.3

PINMAME_SHA=be7a86f95f4aee347ce44cabd6cb055053da108a
LIBSERUM_SHA=ea90a5460b47d77e4cf1deacdacddbdb94c25067
LIBALTSOUND_SHA=b8ef3a3b9bc1bac5a4df1ef3b1ac39e06a4f44c0

NUM_PROCS=$(sysctl -n hw.ncpu)

echo "Building external libraries..."
echo "  SDL2_VERSION: ${SDL2_VERSION}"
echo "  SDL2_IMAGE_VERSION: ${SDL2_IMAGE_VERSION}"
echo "  PINMAME_SHA: ${PINMAME_SHA}"
echo "  LIBSERUM_SHA: ${LIBSERUM_SHA}"
echo "  NUM_PROCS: ${NUM_PROCS}"
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
cp ../../freeimage/Makefile.iphone.arm64 .
make -f Makefile.iphone.arm64 -j${NUM_PROCS}
cp Dist/libfreeimage-arm64.a ../../external/lib/libfreeimage.a
cd ..

#
# copy bass24 framework to external
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

xcrun xcodebuild -project "SDL2-${SDL2_VERSION}/Xcode/SDL/SDL.xcodeproj" -target "Static Library-iOS" -sdk iphoneos -configuration Release clean build CONFIGURATION_BUILD_DIR="$(pwd)/sdl-build"
cp sdl-build/libSDL2.a ../external/lib

#
# build SDL2_image and copy to external
#

curl -sL https://github.com/libsdl-org/SDL_image/releases/download/release-${SDL2_IMAGE_VERSION}/SDL2_image-${SDL2_IMAGE_VERSION}.zip -o SDL2_image-${SDL2_IMAGE_VERSION}.zip
unzip SDL2_image-${SDL2_IMAGE_VERSION}.zip
cp -r SDL2_image-${SDL2_IMAGE_VERSION}/SDL_image.h ../external/include/SDL2

xcrun xcodebuild -project "SDL2_image-${SDL2_IMAGE_VERSION}/Xcode/SDL_image.xcodeproj" -target "Static Library" -sdk iphoneos -configuration Release clean build CONFIGURATION_BUILD_DIR="$(pwd)/sdl_image-build"
cp sdl_image-build/libSDL2_image.a ../external/lib

#
# build libpinmame and copy to external
#

curl -sL https://github.com/vpinball/pinmame/archive/${PINMAME_SHA}.zip -o pinmame.zip
unzip pinmame.zip
cd pinmame-$PINMAME_SHA
cp src/libpinmame/libpinmame.h ../../external/include
cp cmake/libpinmame/CMakeLists_ios-arm64.txt CMakeLists.txt
cmake -DCMAKE_BUILD_TYPE=Release -B build
cmake --build build -- -j${NUM_PROCS}
cp build/libpinmame.3.6.a ../../external/lib
cd ..

#
# build libserum and copy to external
#

curl -sL https://github.com/zesinger/libserum/archive/${LIBSERUM_SHA}.zip -o libserum.zip
unzip libserum.zip
cd libserum-$LIBSERUM_SHA
cp src/serum-decode.h ../../external/include
cmake -DCMAKE_BUILD_TYPE=Release -DUSE_IOS=ON -B build
cmake --build build -- -j${NUM_PROCS}
cp build/libserum.a ../../external/lib
cd ..

#
# build libaltsound and copy to external
#

curl -sL https://github.com/jsm174/libaltsound/archive/${LIBALTSOUND_SHA}.zip -o libaltsound.zip
unzip libaltsound.zip
cd libaltsound-$LIBALTSOUND_SHA
cp src/altsound.h ../../external/include
cd platforms/ios/arm64
./external.sh
cp CMakeLists.txt ../../..
cd ../../..
cmake -DCMAKE_BUILD_TYPE=Release -B build
cmake --build build -- -j${NUM_PROCS}
cp build/libaltsound.a ../../external/lib
cd ..
