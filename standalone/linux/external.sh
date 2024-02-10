#!/bin/bash

set -e

SDL2_VERSION=2.28.5
SDL2_IMAGE_VERSION=2.6.3
SDL2_TTF_VERSION=2.20.2

PINMAME_SHA=4bda9d4c0efc26f8bf6e8442475d45caddc25e2b
LIBALTSOUND_SHA=428bdb1085f486f82279574a74e5a0959107a8f6
LIBDMDUTIL_SHA=342f687a9067f4f3ae2fb7863ef296b3f44253e0

NUM_PROCS=$(nproc)

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
cp ../../freeimage/Makefile.gnu .
make -f Makefile.gnu -j${NUM_PROCS}
cp Dist/libfreeimage.a ../../external/lib/libfreeimage.a
cd ..

#
# download bass24 and copy to external
#

curl -s https://www.un4seen.com/files/bass24-linux.zip -o bass.zip
unzip bass.zip 
cp libs/x86_64/libbass.so ../external/lib

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
   -DCMAKE_BUILD_TYPE=Release \
   -B build
cmake --build build -- -j${NUM_PROCS}
# cmake does not make a symbolic link for libSDL2.so
ln -s libSDL2-2.0.so build/libSDL2.so
cp -P build/*.{so,so.*} ../../external/lib
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
   -DSDL2_LIBRARY=$(pwd)/../SDL2-${SDL2_VERSION}/build/libSDL2.so \
   -DCMAKE_BUILD_TYPE=Release \
   -B build
cmake --build build -- -j${NUM_PROCS}
cp -P build/*.{so,so.*} ../../external/lib
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
   -DSDL2_LIBRARY=$(pwd)/../SDL2-${SDL2_VERSION}/build/libSDL2.so \
   -DSDL2TTF_VENDORED=ON \
   -DSDL2TTF_HARFBUZZ=ON \
   -DCMAKE_BUILD_TYPE=Release \
   -B build
cmake --build build -- -j${NUM_PROCS}
cp -P build/*.{so,so.*} ../../external/lib
cd ..

#
# build libpinmame and copy to external
#

curl -sL https://github.com/vpinball/pinmame/archive/${PINMAME_SHA}.zip -o pinmame.zip
unzip pinmame.zip
cd pinmame-$PINMAME_SHA
cp src/libpinmame/libpinmame.h ../../external/include
cp cmake/libpinmame/CMakeLists_linux-x64.txt CMakeLists.txt
cmake -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -B build
cmake --build build -- -j${NUM_PROCS}
cp build/libpinmame.so.3.6 ../../external/lib
cd ..

#
# build libaltsound and copy to external
#

curl -sL https://github.com/vpinball/libaltsound/archive/${LIBALTSOUND_SHA}.zip -o libaltsound.zip
unzip libaltsound.zip
cd libaltsound-$LIBALTSOUND_SHA
cp src/altsound.h ../../external/include
platforms/linux/x64/external.sh
cmake -DPLATFORM=linux -DARCH=x64 -DBUILD_STATIC=OFF -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -B build
cmake --build build -- -j${NUM_PROCS}
cp build/libaltsound.so.0.1.0 ../../external/lib
cd ..

#
# build libdmdutil (and libserialport, libserum, libzedmd) and copy to external
#

curl -sL https://github.com/vpinball/libdmdutil/archive/${LIBDMDUTIL_SHA}.zip -o libdmdutil.zip
unzip libdmdutil.zip
cd libdmdutil-$LIBDMDUTIL_SHA
cp -r include/DMDUtil ../../external/include
platforms/linux/x64/external.sh
cmake -DPLATFORM=linux -DARCH=x64 -DBUILD_STATIC=OFF -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -B build
cmake --build build -- -j${NUM_PROCS}
cp third-party/runtime-libs/linux/x64/libserum.so.1.6.2 ../../external/lib
cp third-party/runtime-libs/linux/x64/libzedmd.so.0.5.0 ../../external/lib
cp third-party/runtime-libs/linux/x64/libserialport.so.0 ../../external/lib
cp build/libdmdutil.so.0.2.0 ../../external/lib
cd ..
