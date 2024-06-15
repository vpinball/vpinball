#!/bin/bash

set -e

FREEIMAGE_VERSION=3.18.0
SDL2_VERSION=2.30.3
SDL2_IMAGE_VERSION=2.8.2
SDL2_TTF_VERSION=2.22.0
PINMAME_SHA=df7abda19dd67b71d45d2eba61b59983e6c2bbc9
LIBALTSOUND_SHA=9ac08a76e2aabc1fba57d3e5a3b87e7f63c09e07
LIBDMDUTIL_SHA=fc29cd7d8271c34999a125fbbc123a71ace571c9
LIBDOF_SHA=ac5d1e3487a4a6511953eb6aeef06ef5111510ea
FFMPEG_SHA=e38092ef9395d7049f871ef4d5411eb410e283e0
BGFX_CMAKE_VERSION=1.127.8745-470

NUM_PROCS=$(sysctl -n hw.ncpu)

echo "Building external libraries..."
echo "  FREEIMAGE_VERSION: ${FREEIMAGE_VERSION}"
echo "  SDL2_VERSION: ${SDL2_VERSION}"
echo "  SDL2_IMAGE_VERSION: ${SDL2_IMAGE_VERSION}"
echo "  SDL2_TTF_VERSION: ${SDL2_TTF_VERSION}"
echo "  PINMAME_SHA: ${PINMAME_SHA}"
echo "  LIBALTSOUND_SHA: ${LIBALTSOUND_SHA}"
echo "  LIBDMDUTIL_SHA: ${LIBDMDUTIL_SHA}"
echo "  LIBDOF_SHA: ${LIBDOF_SHA}"
echo "  FFMPEG_SHA: ${FFMPEG_SHA}"
echo "  BGFX_CMAKE_VERSION: ${BGFX_CMAKE_VERSION}"
echo ""

if [ -z "${BUILD_TYPE}" ]; then
   BUILD_TYPE="Release"
fi

echo "Build type: ${BUILD_TYPE}"
echo "Procs: ${NUM_PROCS}"
echo ""

CACHE_DIR="external/cache/${BUILD_TYPE}"

rm -rf external/include external/lib external/framework
mkdir -p external/include external/lib external/framework ${CACHE_DIR}

rm -rf tmp
mkdir tmp
cd tmp

#
# build freeimage and copy to external
#

CACHE_NAME="FreeImage-${FREEIMAGE_VERSION}"

if [ ! -f "../${CACHE_DIR}/${CACHE_NAME}.cache" ]; then
   FREEIMAGE_BASENAME="FreeImage${FREEIMAGE_VERSION//./}"
   curl -sL https://downloads.sourceforge.net/project/freeimage/Source%20Distribution/${FREEIMAGE_VERSION}/${FREEIMAGE_BASENAME}.zip -o ${FREEIMAGE_BASENAME}.zip
   unzip ${FREEIMAGE_BASENAME}.zip
   cd FreeImage
   patch -i ../../freeimage/FreeImage3180.patch
   cp ../../freeimage/Makefile.iphone.arm64 .
   make -f Makefile.iphone.arm64 -j${NUM_PROCS}
   mkdir -p ../../${CACHE_DIR}/${CACHE_NAME}/lib
   cp Dist/libfreeimage-arm64.a ../../${CACHE_DIR}/${CACHE_NAME}/lib/libfreeimage.a
   cd ..
   touch "../${CACHE_DIR}/${CACHE_NAME}.cache"
fi

cp ../${CACHE_DIR}/${CACHE_NAME}/lib/*.a ../external/lib

#
# download bass24 and copy to external
#

CACHE_NAME="bass24"

if [ ! -f "../${CACHE_DIR}/${CACHE_NAME}.cache" ]; then
   curl -sL https://www.un4seen.com/files/bass24-ios.zip -o bass.zip
   unzip bass.zip
   mkdir -p ../${CACHE_DIR}/${CACHE_NAME}/framework
   cp -r bass.xcframework/ios-arm64_armv7_armv7s/bass.framework ../${CACHE_DIR}/${CACHE_NAME}/framework
   touch "../${CACHE_DIR}/${CACHE_NAME}.cache"
fi

cp -r ../${CACHE_DIR}/${CACHE_NAME}/framework/*.framework ../external/framework

#
# build SDL2 and copy to external
#

SDL2_CACHE_NAME="SDL2-${SDL2_VERSION}"

if [ ! -f "../${CACHE_DIR}/${SDL2_CACHE_NAME}.cache" ]; then
   curl -sL https://github.com/libsdl-org/SDL/releases/download/release-${SDL2_VERSION}/SDL2-${SDL2_VERSION}.zip -o SDL2-${SDL2_VERSION}.zip
   unzip SDL2-${SDL2_VERSION}.zip
   cd SDL2-${SDL2_VERSION}
   xcrun xcodebuild \
      -project "Xcode/SDL/SDL.xcodeproj" \
      -target "Static Library-iOS" \
      -sdk iphoneos \
      -configuration Release \
      clean build CONFIGURATION_BUILD_DIR="$(pwd)/sdl-build"
   mkdir -p ../../${CACHE_DIR}/${SDL2_CACHE_NAME}/include
   cp include/*.h ../../${CACHE_DIR}/${SDL2_CACHE_NAME}/include
   mkdir -p ../../${CACHE_DIR}/${SDL2_CACHE_NAME}/lib
   cp sdl-build/*.a ../../${CACHE_DIR}/${SDL2_CACHE_NAME}/lib
   cd ..
   touch "../${CACHE_DIR}/${SDL2_CACHE_NAME}.cache"
fi
 
mkdir -p ../external/include/SDL2
cp -r ../${CACHE_DIR}/${SDL2_CACHE_NAME}/include/* ../external/include/SDL2
cp ../${CACHE_DIR}/${SDL2_CACHE_NAME}/lib/*.a ../external/lib

#
# build SDL2_image and copy to external
#

CACHE_NAME="SDL2_image-${SDL2_IMAGE_VERSION}-${SDL2_CACHE_NAME}"

if [ ! -f "../${CACHE_DIR}/${CACHE_NAME}.cache" ]; then
   curl -sL https://github.com/libsdl-org/SDL_image/releases/download/release-${SDL2_IMAGE_VERSION}/SDL2_image-${SDL2_IMAGE_VERSION}.zip -o SDL2_image-${SDL2_IMAGE_VERSION}.zip
   unzip SDL2_image-${SDL2_IMAGE_VERSION}.zip
   cd SDL2_image-${SDL2_IMAGE_VERSION}
   xcrun xcodebuild \
      -project "Xcode/SDL_image.xcodeproj" \
      -target "Static Library" \
      -sdk iphoneos -configuration Release \
      clean build CONFIGURATION_BUILD_DIR="$(pwd)/sdl_image-build"
   mkdir -p ../../${CACHE_DIR}/${CACHE_NAME}/include
   cp include/*.h ../../${CACHE_DIR}/${CACHE_NAME}/include
   mkdir -p ../../${CACHE_DIR}/${CACHE_NAME}/lib
   cp sdl_image-build/*.a ../../${CACHE_DIR}/${CACHE_NAME}/lib
   cd ..
   touch "../${CACHE_DIR}/${CACHE_NAME}.cache"
fi

cp -r ../${CACHE_DIR}/${CACHE_NAME}/include/* ../external/include/SDL2
cp ../${CACHE_DIR}/${CACHE_NAME}/lib/*.a ../external/lib

#
# build SDL2_ttf and copy to external
#

CACHE_NAME="SDL2_ttf-${SDL2_TTF_VERSION}-${SDL2_CACHE_NAME}"

if [ ! -f "../${CACHE_DIR}/${CACHE_NAME}.cache" ]; then
   curl -sL https://github.com/libsdl-org/SDL_ttf/releases/download/release-${SDL2_TTF_VERSION}/SDL2_ttf-${SDL2_TTF_VERSION}.zip -o SDL2_ttf-${SDL2_TTF_VERSION}.zip
   unzip SDL2_ttf-${SDL2_TTF_VERSION}.zip
   cd SDL2_ttf-${SDL2_TTF_VERSION}
   xcrun xcodebuild \
      -project "Xcode/SDL_ttf.xcodeproj" \
      -target "Static Library" \
      -sdk iphoneos \
      -configuration Release \
      clean build CONFIGURATION_BUILD_DIR="$(pwd)/sdl_ttf-build"
   mkdir -p ../../${CACHE_DIR}/${CACHE_NAME}/include
   cp *.h ../../${CACHE_DIR}/${CACHE_NAME}/include
   mkdir -p ../../${CACHE_DIR}/${CACHE_NAME}/lib
   cp sdl_ttf-build/*.a ../../${CACHE_DIR}/${CACHE_NAME}/lib
   cd ..
   touch "../${CACHE_DIR}/${CACHE_NAME}.cache"
fi

cp -r ../${CACHE_DIR}/${CACHE_NAME}/include/* ../external/include/SDL2
cp ../${CACHE_DIR}/${CACHE_NAME}/lib/*.a ../external/lib

#
# build libpinmame and copy to external
#

CACHE_NAME="pinmame-${PINMAME_SHA}"

if [ ! -f "../${CACHE_DIR}/${CACHE_NAME}.cache" ]; then
   curl -sL https://github.com/vpinball/pinmame/archive/${PINMAME_SHA}.zip -o pinmame.zip
   unzip pinmame.zip
   cd pinmame-$PINMAME_SHA
   cp cmake/libpinmame/CMakeLists_ios-arm64.txt CMakeLists.txt
   cmake \
      -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
      -B build
   cmake --build build -- -j${NUM_PROCS}
   mkdir -p ../../${CACHE_DIR}/${CACHE_NAME}/include
   cp src/libpinmame/libpinmame.h ../../${CACHE_DIR}/${CACHE_NAME}/include
   mkdir -p ../../${CACHE_DIR}/${CACHE_NAME}/lib
   cp build/*.a ../../${CACHE_DIR}/${CACHE_NAME}/lib
   cd ..
   touch "../${CACHE_DIR}/${CACHE_NAME}.cache"
fi

cp -r ../${CACHE_DIR}/${CACHE_NAME}/include/* ../external/include
cp ../${CACHE_DIR}/${CACHE_NAME}/lib/*.a ../external/lib

#
# build libaltsound and copy to external
#

CACHE_NAME="libaltsound-${LIBALTSOUND_SHA}"

if [ ! -f  "../${CACHE_DIR}/${CACHE_NAME}.cache" ]; then
   curl -sL https://github.com/vpinball/libaltsound/archive/${LIBALTSOUND_SHA}.zip -o libaltsound.zip
   unzip libaltsound.zip
   cd libaltsound-$LIBALTSOUND_SHA
   platforms/ios/arm64/external.sh
   cmake \
      -DPLATFORM=ios \
      -DARCH=arm64 \
      -DBUILD_SHARED=OFF \
      -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
      -B build
   cmake --build build -- -j${NUM_PROCS}
   mkdir -p ../../${CACHE_DIR}/${CACHE_NAME}/include
   cp src/altsound.h ../../${CACHE_DIR}/${CACHE_NAME}/include
   mkdir -p ../../${CACHE_DIR}/${CACHE_NAME}/lib
   cp build/*.a ../../${CACHE_DIR}/${CACHE_NAME}/lib
   cd ..
   touch "../${CACHE_DIR}/${CACHE_NAME}.cache"
fi

cp -r ../${CACHE_DIR}/${CACHE_NAME}/include/* ../external/include
cp ../${CACHE_DIR}/${CACHE_NAME}/lib/*.a ../external/lib

#
# build libdmdutil (and deps) and copy to external
#

CACHE_NAME="libdmdutil-${LIBDMDUTIL_SHA}"

if [ ! -f "../${CACHE_DIR}/${CACHE_NAME}.cache" ]; then
   curl -sL https://github.com/vpinball/libdmdutil/archive/${LIBDMDUTIL_SHA}.zip -o libdmdutil.zip
   unzip libdmdutil.zip
   cd libdmdutil-$LIBDMDUTIL_SHA
   platforms/ios/arm64/external.sh
   cmake \
      -DPLATFORM=ios \
      -DARCH=arm64 \
      -DBUILD_SHARED=OFF \
      -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
      -B build
   cmake --build build -- -j${NUM_PROCS}
   mkdir -p ../../${CACHE_DIR}/${CACHE_NAME}/include
   cp -r include/DMDUtil ../../${CACHE_DIR}/${CACHE_NAME}/include
   cp -r third-party/include/sockpp ../../${CACHE_DIR}/${CACHE_NAME}/include
   mkdir -p ../../${CACHE_DIR}/${CACHE_NAME}/lib
   cp third-party/build-libs/ios/arm64/*.a ../../${CACHE_DIR}/${CACHE_NAME}/lib
   cp build/*.a ../../${CACHE_DIR}/${CACHE_NAME}/lib
   cd ..
   touch "../${CACHE_DIR}/${CACHE_NAME}.cache"
fi

cp -r ../${CACHE_DIR}/${CACHE_NAME}/include/* ../external/include
cp ../${CACHE_DIR}/${CACHE_NAME}/lib/*.a ../external/lib

#
# build libdof (and deps) and copy to external
#

CACHE_NAME="libdof-${LIBDOF_SHA}"

if [ ! -f "../${CACHE_DIR}/${CACHE_NAME}.cache" ]; then
   curl -sL https://github.com/jsm174/libdof/archive/${LIBDOF_SHA}.zip -o libdof.zip
   unzip libdof.zip
   cd libdof-$LIBDOF_SHA
   platforms/ios/arm64/external.sh
   cmake \
      -DPLATFORM=ios \
      -DARCH=arm64 \
      -DBUILD_SHARED=OFF \
      -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
      -B build
   cmake --build build -- -j${NUM_PROCS}
   mkdir -p ../../${CACHE_DIR}/${CACHE_NAME}/include
   cp -r include/DOF ../../${CACHE_DIR}/${CACHE_NAME}/include
   mkdir -p ../../${CACHE_DIR}/${CACHE_NAME}/lib
   cp third-party/build-libs/ios/arm64/*.a ../../${CACHE_DIR}/${CACHE_NAME}/lib
   cp build/*.a ../../${CACHE_DIR}/${CACHE_NAME}/lib
   cd ..
   touch "../${CACHE_DIR}/${CACHE_NAME}.cache"
fi

cp -r ../${CACHE_DIR}/${CACHE_NAME}/include/* ../external/include
cp ../${CACHE_DIR}/${CACHE_NAME}/lib/*.a ../external/lib

#
# build FFMPEG libraries and copy to external
#

# TODO: build FFMPEG libraries for iOS

#
# build bgfx and copy to external
#

CACHE_NAME="BGFX_CMAKE-${BGFX_CMAKE_VERSION}_002"

if [ ! -f "../${CACHE_DIR}/${CACHE_NAME}.cache" ]; then
   curl -sL https://github.com/bkaradzic/bgfx.cmake/releases/download/v${BGFX_CMAKE_VERSION}/bgfx.cmake.v${BGFX_CMAKE_VERSION}.tar.gz -o bgfx.cmake.v${BGFX_CMAKE_VERSION}.tar.gz
   tar -xvzf bgfx.cmake.v${BGFX_CMAKE_VERSION}.tar.gz
   cd bgfx.cmake
   cmake -S. \
      -DCMAKE_SYSTEM_NAME=iOS \
      -DCMAKE_OSX_DEPLOYMENT_TARGET=16.0 \
      -DCMAKE_OSX_ARCHITECTURES=arm64 \
      -DBGFX_BUILD_EXAMPLES=OFF \
      -DBGFX_CONFIG_MULTITHREADED=ON \
      -DBGFX_CONFIG_MAX_FRAME_BUFFERS=256 \
      -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
      -B build
   cmake --build build -- -j${NUM_PROCS}
   mkdir -p ../../${CACHE_DIR}/${CACHE_NAME}/include
   cp -r bgfx/include/bgfx ../../${CACHE_DIR}/${CACHE_NAME}/include
   cp -r bimg/include/bimg ../../${CACHE_DIR}/${CACHE_NAME}/include
   cp -r bx/include/bx ../../${CACHE_DIR}/${CACHE_NAME}/include
   mkdir -p ../../${CACHE_DIR}/${CACHE_NAME}/lib
   cp build/cmake/bgfx/*.a ../../${CACHE_DIR}/${CACHE_NAME}/lib
   cp build/cmake/bimg/*.a ../../${CACHE_DIR}/${CACHE_NAME}/lib
   cp build/cmake/bx/*.a ../../${CACHE_DIR}/${CACHE_NAME}/lib
   cd ..
   touch "../${CACHE_DIR}/${CACHE_NAME}.cache"
fi

cp -r ../${CACHE_DIR}/${CACHE_NAME}/include/* ../external/include
cp ../${CACHE_DIR}/${CACHE_NAME}/lib/*.a ../external/lib
