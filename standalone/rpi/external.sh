#!/bin/bash

set -e

SDL2_VERSION=2.28.5
SDL2_IMAGE_VERSION=2.6.3

PINMAME_SHA=be7a86f95f4aee347ce44cabd6cb055053da108a
LIBSERUM_SHA=ea90a5460b47d77e4cf1deacdacddbdb94c25067
LIBZEDMD_SHA=499b1c094d49ae9bd988326475c51686b1415186
LIBALTSOUND_SHA=e791d98451eadf6d33e8f2c22d4c47646b28017d

NUM_PROCS=$(nproc)

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
cp libs/aarch64/libbass.so ../external/lib

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
	-DSDL_X11=OFF \
	-DSDL_KMSDRM=ON \
	-DCMAKE_BUILD_TYPE=Release \
	-B build
cmake --build build -- -j${NUM_PROCS}
cp build/libSDL2-2.0.so.0 ../../external/lib
cd ..

#
# build SDL2_image and copy to external
#

curl -sL https://github.com/libsdl-org/SDL_image/releases/download/release-${SDL2_IMAGE_VERSION}/SDL2_image-${SDL2_IMAGE_VERSION}.zip -o SDL2_image-${SDL2_IMAGE_VERSION}.zip
unzip SDL2_image-${SDL2_IMAGE_VERSION}.zip
cp -r SDL2_image-${SDL2_IMAGE_VERSION}/SDL_image.h ../external/include/SDL2
cd SDL2_image-${SDL2_IMAGE_VERSION}
cmake -DBUILD_SHARED_LIBS=ON \
	-DSDL2IMAGE_SAMPLES=OFF \
	-DSDL2_INCLUDE_DIR=$(pwd)/../SDL2-${SDL2_VERSION}/include \
	-DSDL2_LIBRARY=$(pwd)/../SDL2-${SDL2_VERSION}/build/libSDL2-2.0.so \
	-DCMAKE_BUILD_TYPE=Release \
	-B build
cmake --build build -- -j${NUM_PROCS}
cp build/libSDL2_image-2.0.so.0 ../../external/lib
cd ..

#
# build libpinmame and copy to external
#

curl -sL https://github.com/vpinball/pinmame/archive/${PINMAME_SHA}.zip -o pinmame.zip
unzip pinmame.zip
cd pinmame-$PINMAME_SHA
cp src/libpinmame/libpinmame.h ../../external/include
cp cmake/libpinmame/CMakeLists_linux-x64.txt CMakeLists.txt
cmake -DCMAKE_BUILD_TYPE=Release -B build
cmake --build build -- -j${NUM_PROCS}
cp build/libpinmame.so.3.6 ../../external/lib
cd ..

#
# build libserum and copy to external
#

curl -sL https://github.com/zesinger/libserum/archive/${LIBSERUM_SHA}.zip -o libserum.zip
unzip libserum.zip
cd libserum-$LIBSERUM_SHA
cp src/serum-decode.h ../../external/include
cmake -DCMAKE_BUILD_TYPE=Release -B build
cmake --build build -- -j${NUM_PROCS}
cp build/libserum64.so.1.6.1 ../../external/lib
cd ..

#
# build libzedmd and copy to external
#

curl -sL https://github.com/PPUC/libzedmd/archive/${LIBZEDMD_SHA}.zip -o libzedmd.zip
unzip libzedmd.zip
cd libzedmd-$LIBZEDMD_SHA
cp src/ZeDMD.h ../../external/include
cmake -DCMAKE_BUILD_TYPE=Release -B build
cmake --build build -- -j${NUM_PROCS}
cp build/libzedmd64.so.0.1.0 ../../external/lib
cd ..

#
# build libaltsound and copy to external
#

curl -sL https://github.com/jsm174/libaltsound/archive/${LIBALTSOUND_SHA}.zip -o libaltsound.zip
unzip libaltsound.zip
cd libaltsound-$LIBALTSOUND_SHA
cp src/altsound.h ../../external/include
cd platforms/linux/aarch64
./external.sh
cp CMakeLists.txt ../../..
cd ../../..
cmake -DCMAKE_BUILD_TYPE=Release -B build
cmake --build build -- -j${NUM_PROCS}
cp build/libaltsound.so ../../external/lib
cd ..
