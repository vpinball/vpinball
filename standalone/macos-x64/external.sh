#!/bin/bash

set -e

SDL2_VERSION=2.28.5
SDL2_IMAGE_VERSION=2.6.3
SDL2_TTF_VERSION=2.20.2

PINMAME_SHA=66be20f6f598cb0453196178bfa2a937db819ceb
LIBALTSOUND_SHA=676ebcde7802ffdbd84c9a275213754a4ebf5e8f
LIBDMDUTIL_SHA=0a7e81f64c32b02596f11c969a07207bea48a8cc

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
cp ../../freeimage/Makefile.macos .
make -f Makefile.macos -j${NUM_PROCS}
cp Dist/libfreeimage-x86_64.a ../../external/lib/libfreeimage.a
cd ..

#
# download bass24 and copy to external
#

curl -s https://www.un4seen.com/files/bass24-osx.zip -o bass.zip
unzip bass.zip 
cp libbass.dylib ../external/lib

#
# build SDL2 and copy to external
#

curl -sL https://github.com/libsdl-org/SDL/releases/download/release-${SDL2_VERSION}/SDL2-${SDL2_VERSION}.zip -o SDL2-${SDL2_VERSION}.zip
unzip SDL2-${SDL2_VERSION}.zip
cp -r SDL2-${SDL2_VERSION}/include ../external/include/SDL2
cd SDL2-${SDL2_VERSION}
cmake -DSDL_SHARED=ON \
   -DSDL_STATIC=OFF \
   -DSDL_TEST=OFF \
   -DCMAKE_OSX_ARCHITECTURES=x86_64 \
   -DCMAKE_OSX_DEPLOYMENT_TARGET=13.0 \
   -DCMAKE_BUILD_TYPE=Release \
   -B build
cmake --build build -- -j${NUM_PROCS}
cp build/libSDL2-2.0.0.dylib ../../external/lib
cd ..

#
# build SDL2_image and copy to external
#

curl -sL https://github.com/libsdl-org/SDL_image/releases/download/release-${SDL2_IMAGE_VERSION}/SDL2_image-${SDL2_IMAGE_VERSION}.zip -o SDL2_image-${SDL2_IMAGE_VERSION}.zip
unzip SDL2_image-${SDL2_IMAGE_VERSION}.zip
cp -r SDL2_image-${SDL2_IMAGE_VERSION}/SDL_image.h ../external/include/SDL2
cd SDL2_image-${SDL2_IMAGE_VERSION}
touch cmake/FindSDL2.cmake # force cmake to use the SDL2 we just built
cmake -DBUILD_SHARED_LIBS=ON \
   -DSDL2IMAGE_SAMPLES=OFF \
   -DSDL2_INCLUDE_DIR=$(pwd)/../SDL2-${SDL2_VERSION}/include \
   -DSDL2_LIBRARY=$(pwd)/../SDL2-${SDL2_VERSION}/build/libSDL2-2.0.0.dylib \
   -DCMAKE_OSX_ARCHITECTURES=x86_64 \
   -DCMAKE_OSX_DEPLOYMENT_TARGET=13.0 \
   -DCMAKE_BUILD_TYPE=Release \
   -B build
cmake --build build -- -j${NUM_PROCS}
cp build/libSDL2_image-2.0.3.0.0.dylib ../../external/lib
cd ..

#
# build SDL2_ttf and copy to external
#

curl -sL https://github.com/libsdl-org/SDL_ttf/releases/download/release-${SDL2_TTF_VERSION}/SDL2_ttf-${SDL2_TTF_VERSION}.zip -o SDL2_ttf-${SDL2_TTF_VERSION}.zip
unzip SDL2_ttf-${SDL2_TTF_VERSION}.zip
cp -r SDL2_ttf-${SDL2_TTF_VERSION}/SDL_ttf.h ../external/include/SDL2
cd SDL2_ttf-${SDL2_TTF_VERSION}
touch cmake/FindSDL2.cmake # force cmake to use the SDL2 we just built
cmake -DBUILD_SHARED_LIBS=ON \
   -DSDL2TTF_SAMPLES=OFF \
   -DSDL2_INCLUDE_DIR=$(pwd)/../SDL2-${SDL2_VERSION}/include \
   -DSDL2_LIBRARY=$(pwd)/../SDL2-${SDL2_VERSION}/build/libSDL2-2.0.0.dylib \
   -DSDL2TTF_VENDORED=ON \
   -DSDL2TTF_HARFBUZZ=ON \
   -DCMAKE_OSX_ARCHITECTURES=x86_64 \
   -DCMAKE_OSX_DEPLOYMENT_TARGET=13.0 \
   -DCMAKE_BUILD_TYPE=Release \
   -B build
cmake --build build -- -j${NUM_PROCS}
cp build/libSDL2_ttf-2.0.15.0.0.dylib ../../external/lib
cd ..

#
# build libpinmame and copy to external
#

curl -sL https://github.com/vpinball/pinmame/archive/${PINMAME_SHA}.zip -o pinmame.zip
unzip pinmame.zip
cd pinmame-$PINMAME_SHA
cp src/libpinmame/libpinmame.h ../../external/include
cp cmake/libpinmame/CMakeLists_osx-x64.txt CMakeLists.txt
cmake -DCMAKE_OSX_DEPLOYMENT_TARGET=13.0 -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -DBUILD_STATIC=OFF -B build
cmake --build build -- -j${NUM_PROCS}
cp build/libpinmame.3.6.dylib ../../external/lib
cd ..

#
# build libaltsound and copy to external
#

curl -sL https://github.com/vpinball/libaltsound/archive/${LIBALTSOUND_SHA}.zip -o libaltsound.zip
unzip libaltsound.zip
cd libaltsound-$LIBALTSOUND_SHA
cp src/altsound.h ../../external/include
platforms/macos/x64/external.sh
cmake -DPLATFORM=macos -DARCH=x64 -DBUILD_STATIC=OFF -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -B build
cmake --build build -- -j${NUM_PROCS}
cp build/libaltsound.0.1.0.dylib ../../external/lib
cd ..

#
# build libdmdutil (and libserialport, libserum, libzedmd) and copy to external
#

curl -sL https://github.com/vpinball/libdmdutil/archive/${LIBDMDUTIL_SHA}.zip -o libdmdutil.zip
unzip libdmdutil.zip
cd libdmdutil-$LIBDMDUTIL_SHA
cp -r include/DMDUtil ../../external/include
platforms/macos/x64/external.sh
cmake -DPLATFORM=macos -DARCH=x64 -DBUILD_STATIC=OFF -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -B build
cmake --build build -- -j${NUM_PROCS}
cp third-party/runtime-libs/macos/x64/libserum.1.6.2.dylib ../../external/lib
cp third-party/runtime-libs/macos/x64/libzedmd.0.5.0.dylib ../../external/lib
cp third-party/runtime-libs/macos/x64/libserialport.dylib ../../external/lib
cp build/libdmdutil.0.1.0.dylib ../../external/lib
cd ..
