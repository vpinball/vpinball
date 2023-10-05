#!/bin/bash

set -e

SDL2_VERSION=2.28.3
SDL2_TTF_VERSION=2.20.2
SDL2_IMAGE_VERSION=2.6.3

PINMAME_SHA=2e9701e18bcd1491856f413eab1a5de5b128cd54
SERUM_SHA=828c087986f95ca7dbf6c3de2ca8df4200ed011b
ZEDMD_SHA=499b1c094d49ae9bd988326475c51686b1415186

NUM_PROCS=$(sysctl -n hw.ncpu)

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
# build freeimage and copy to external
#

curl -sL https://downloads.sourceforge.net/project/freeimage/Source%20Distribution/3.18.0/FreeImage3180.zip -o FreeImage3180.zip
unzip FreeImage3180.zip
cd FreeImage
cp ../../freeimage/Makefile.macos.arm64 .
make -f Makefile.macos.arm64 -j${NUM_PROCS}
cp Dist/libfreeimage-arm64.a ../../external/lib/libfreeimage.a
cd ..

#
# download bass24 and copy to external
#

curl -s https://www.un4seen.com/files/bass24-osx.zip -o bass.zip
unzip bass.zip 
mv libbass.dylib ../external/lib

#
# build SDL2 and copy to external
#

curl -sL https://github.com/libsdl-org/SDL/releases/download/release-${SDL2_VERSION}/SDL2-${SDL2_VERSION}.zip -o SDL2-${SDL2_VERSION}.zip
unzip SDL2-${SDL2_VERSION}.zip
cp -r SDL2-${SDL2_VERSION}/include ../external/include/SDL2
cd SDL2-${SDL2_VERSION}
cmake -DSDL_SHARED=OFF \
	-DSDL_STATIC=ON \
	-DSDL_STATIC_PIC=ON \
	-DSDL_TEST=OFF \
	-DCMAKE_OSX_ARCHITECTURES=arm64 \
	-DCMAKE_BUILD_TYPE=Release \
	-B build
cmake --build build -- -j${NUM_PROCS}
cp build/libSDL2.a ../../external/lib
cd ..

#
# build SDL2_image and copy to external
#

curl -sL https://github.com/libsdl-org/SDL_image/releases/download/release-${SDL2_IMAGE_VERSION}/SDL2_image-${SDL2_IMAGE_VERSION}.zip -o SDL2_image-${SDL2_IMAGE_VERSION}.zip
unzip SDL2_image-${SDL2_IMAGE_VERSION}.zip
cp -r SDL2_image-${SDL2_IMAGE_VERSION}/SDL_image.h ../external/include/SDL2
cd SDL2_image-${SDL2_IMAGE_VERSION}
cmake -DBUILD_SHARED_LIBS=OFF \
	-DSDL2IMAGE_SAMPLES=OFF \
	-DSDL2_INCLUDE_DIR=$(pwd)/../SDL2-${SDL2_VERSION}/include \
	-DSDL2_LIBRARY=$(pwd)/../SDL2-${SDL2_VERSION}/build/libSDL2.a \
	-DCMAKE_OSX_ARCHITECTURES=arm64 \
	-DCMAKE_BUILD_TYPE=Release \
	-B build
cmake --build build -- -j${NUM_PROCS}
cp build/libSDL2_image.a ../../external/lib
cd ..

#
# build SDL2_ttf and copy to external
#

curl -sL https://github.com/libsdl-org/SDL_ttf/releases/download/release-${SDL2_TTF_VERSION}/SDL2_ttf-${SDL2_TTF_VERSION}.zip -o SDL2_ttf-${SDL2_TTF_VERSION}.zip
unzip SDL2_ttf-${SDL2_TTF_VERSION}.zip
cp -r SDL2_ttf-${SDL2_TTF_VERSION}/SDL_ttf.h ../external/include/SDL2
cd SDL2_ttf-${SDL2_TTF_VERSION}
cmake -DBUILD_SHARED_LIBS=OFF \
	-DSDL2TTF_SAMPLES=OFF \
	-DSDL2_INCLUDE_DIR=$(pwd)/../SDL2-${SDL2_VERSION}/include \
	-DSDL2_LIBRARY=$(pwd)/../SDL2-${SDL2_VERSION}/build/libSDL2.a \
	-DSDL2TTF_VENDORED=ON \
	-DSDL2TTF_HARFBUZZ=ON \
	-DCMAKE_OSX_ARCHITECTURES=arm64 \
	-DCMAKE_BUILD_TYPE=Release \
	-B build
cmake --build build -- -j${NUM_PROCS}
cp build/libSDL2_ttf.a ../../external/lib
cd ..

#
# build libpinmame and copy to external
#

curl -sL https://github.com/vpinball/pinmame/archive/${PINMAME_SHA}.zip -o pinmame.zip
unzip pinmame.zip
cd pinmame-$PINMAME_SHA
cp src/libpinmame/libpinmame.h ../../external/include
cp cmake/libpinmame/CMakeLists_osx-arm64.txt CMakeLists.txt
cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED=OFF -B build
cmake --build build -- -j${NUM_PROCS}
cp build/libpinmame.a ../../external/lib
cd ..

#
# build libserum and copy to external
#

curl -sL https://github.com/zesinger/libserum/archive/${SERUM_SHA}.zip -o libserum.zip
unzip libserum.zip
cd libserum-$SERUM_SHA
cp src/serum-decode.h ../../external/include
cmake -DCMAKE_BUILD_TYPE=Release -DUSE_OSXARM=ON -B build
cmake --build build -- -j${NUM_PROCS}
cp build/libserum.a ../../external/lib
cd ..

#
# build libzedmd and copy to external
#

curl -sL https://github.com/PPUC/libzedmd/archive/${ZEDMD_SHA}.zip -o libzedmd.zip
unzip libzedmd.zip
cd libzedmd-$ZEDMD_SHA
cp src/ZeDMD.h ../../external/include
cmake -DCMAKE_BUILD_TYPE=Release -DUSE_OSXARM=ON -B build
cmake --build build -- -j${NUM_PROCS}
cp build/libzedmd.a ../../external/lib
cd ..
