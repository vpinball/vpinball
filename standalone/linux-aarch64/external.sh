#!/bin/bash

set -e

FREEIMAGE_VERSION=3.18.0
SDL2_VERSION=2.30.2
SDL2_IMAGE_VERSION=2.8.2
SDL2_TTF_VERSION=2.22.0
PINMAME_SHA=5ca69dcabfb62e9488095140c3052c0cc125487c
LIBALTSOUND_SHA=9ac08a76e2aabc1fba57d3e5a3b87e7f63c09e07
LIBDMDUTIL_SHA=0b58af9d2bb7886721ccfa3d3000b3d8f054a114
LIBDOF_SHA=ac5d1e3487a4a6511953eb6aeef06ef5111510ea
FFMPEG_SHA=e38092ef9395d7049f871ef4d5411eb410e283e0

NUM_PROCS=$(nproc)

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
echo ""

if [ -z "${BUILD_TYPE}" ]; then
   BUILD_TYPE="Release"
fi

echo "Build type: ${BUILD_TYPE}"
echo "Procs: ${NUM_PROCS}"
echo ""

CACHE_DIR="external/cache/${BUILD_TYPE}"

rm -rf external/include external/lib
mkdir -p external/include external/lib ${CACHE_DIR}

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
   cp ../../freeimage/Makefile.gnu .
   make -f Makefile.gnu -j${NUM_PROCS}
   mkdir -p ../../${CACHE_DIR}/${CACHE_NAME}/lib
   cp Dist/libfreeimage.a ../../${CACHE_DIR}/${CACHE_NAME}/lib/libfreeimage.a
   cd ..
   touch "../${CACHE_DIR}/${CACHE_NAME}.cache"
fi

cp ../${CACHE_DIR}/${CACHE_NAME}/lib/*.a ../external/lib

#
# download bass24 and copy to external
#

CACHE_NAME="bass24"

if [ ! -f "../${CACHE_DIR}/${CACHE_NAME}.cache" ]; then
   curl -sL https://www.un4seen.com/files/bass24-linux.zip -o bass.zip
   unzip bass.zip
   mkdir -p ../${CACHE_DIR}/${CACHE_NAME}/lib
   cp libs/aarch64/libbass.so ../${CACHE_DIR}/${CACHE_NAME}/lib
   touch "../${CACHE_DIR}/${CACHE_NAME}.cache"
fi

cp -a ../${CACHE_DIR}/${CACHE_NAME}/lib/*.so ../external/lib

#
# build SDL2 and copy to external
#

SDL2_CACHE_NAME="SDL2-${SDL2_VERSION}"

if [ ! -f "../${CACHE_DIR}/${SDL2_CACHE_NAME}.cache" ]; then
   curl -sL https://github.com/libsdl-org/SDL/releases/download/release-${SDL2_VERSION}/SDL2-${SDL2_VERSION}.zip -o SDL2-${SDL2_VERSION}.zip
   unzip SDL2-${SDL2_VERSION}.zip
   cd SDL2-${SDL2_VERSION}
   cmake \
      -DSDL_SHARED=ON \
      -DSDL_STATIC=OFF \
      -DSDL_TEST=OFF \
      -DSDL_X11=ON \
      -DSDL_KMSDRM=ON \
      -DCMAKE_BUILD_WITH_INSTALL_RPATH=TRUE \
      -DCMAKE_INSTALL_RPATH="\$ORIGIN" \
      -DCMAKE_BUILD_TYPE=Release \
      -B build
   cmake --build build -- -j${NUM_PROCS}
   # cmake does not make a symbolic link for libSDL2.so
   ln -s libSDL2-2.0.so build/libSDL2.so
   mkdir -p ../../${CACHE_DIR}/${SDL2_CACHE_NAME}/include
   cp include/*.h ../../${CACHE_DIR}/${SDL2_CACHE_NAME}/include
   # we need the generated SDL_config.h for this platform. Backup generic one and replace with generated one
   mv ../../${CACHE_DIR}/${SDL2_CACHE_NAME}/include/SDL_config.h ../../${CACHE_DIR}/${SDL2_CACHE_NAME}/include/_SDL_config.h
   cp build/include-config-release/SDL2/SDL_config.h ../../${CACHE_DIR}/${SDL2_CACHE_NAME}/include
   mkdir -p ../../${CACHE_DIR}/${SDL2_CACHE_NAME}/lib
   cp -a build/*.{so,so.*} ../../${CACHE_DIR}/${SDL2_CACHE_NAME}/lib
   cd ..
   touch "../${CACHE_DIR}/${SDL2_CACHE_NAME}.cache"
fi

mkdir -p ../external/include/SDL2
cp -r ../${CACHE_DIR}/${SDL2_CACHE_NAME}/include/* ../external/include/SDL2
cp -a ../${CACHE_DIR}/${SDL2_CACHE_NAME}/lib/*.{so,so.*} ../external/lib

#
# build SDL2_image and copy to external
#

CACHE_NAME="SDL2_image-${SDL2_IMAGE_VERSION}-${SDL2_CACHE_NAME}"

if [ ! -f "../${CACHE_DIR}/${CACHE_NAME}.cache" ]; then
   curl -sL https://github.com/libsdl-org/SDL_image/releases/download/release-${SDL2_IMAGE_VERSION}/SDL2_image-${SDL2_IMAGE_VERSION}.zip -o SDL2_image-${SDL2_IMAGE_VERSION}.zip
   unzip SDL2_image-${SDL2_IMAGE_VERSION}.zip
   cd SDL2_image-${SDL2_IMAGE_VERSION}
   touch cmake/FindSDL2.cmake # force cmake to use the SDL2 we just built
   cmake \
      -DBUILD_SHARED_LIBS=ON \
      -DSDL2IMAGE_SAMPLES=OFF \
      -DSDL2_INCLUDE_DIR=../../external/include/SDL2 \
      -DSDL2_LIBRARY=../../external/lib/libSDL2.so \
      -DCMAKE_BUILD_WITH_INSTALL_RPATH=TRUE \
      -DCMAKE_INSTALL_RPATH="\$ORIGIN" \
      -DCMAKE_BUILD_TYPE=Release \
      -B build
   cmake --build build -- -j${NUM_PROCS}
   mkdir -p ../../${CACHE_DIR}/${CACHE_NAME}/include
   cp include/*.h ../../${CACHE_DIR}/${CACHE_NAME}/include
   mkdir -p ../../${CACHE_DIR}/${CACHE_NAME}/lib
   cp -a build/*.{so,so.*} ../../${CACHE_DIR}/${CACHE_NAME}/lib
   cd ..
   touch "../${CACHE_DIR}/${CACHE_NAME}.cache"
fi

cp -r ../${CACHE_DIR}/${CACHE_NAME}/include/* ../external/include/SDL2
cp -a ../${CACHE_DIR}/${CACHE_NAME}/lib/*.{so,so.*} ../external/lib

#
# build SDL2_ttf and copy to external
#

CACHE_NAME="SDL2_ttf-${SDL2_TTF_VERSION}-${SDL2_CACHE_NAME}"

if [ ! -f "../${CACHE_DIR}/${CACHE_NAME}.cache" ]; then
   curl -sL https://github.com/libsdl-org/SDL_ttf/releases/download/release-${SDL2_TTF_VERSION}/SDL2_ttf-${SDL2_TTF_VERSION}.zip -o SDL2_ttf-${SDL2_TTF_VERSION}.zip
   unzip SDL2_ttf-${SDL2_TTF_VERSION}.zip
   cd SDL2_ttf-${SDL2_TTF_VERSION}
   touch cmake/FindSDL2.cmake # force cmake to use the SDL2 we just built
   cmake \
      -DBUILD_SHARED_LIBS=ON \
      -DSDL2TTF_SAMPLES=OFF \
      -DSDL2_INCLUDE_DIR=../../external/include/SDL2 \
      -DSDL2_LIBRARY=../../external/lib/libSDL2.so \
      -DSDL2TTF_VENDORED=ON \
      -DSDL2TTF_HARFBUZZ=ON \
      -DCMAKE_BUILD_WITH_INSTALL_RPATH=TRUE \
      -DCMAKE_INSTALL_RPATH="\$ORIGIN" \
      -DCMAKE_BUILD_TYPE=Release \
      -B build
   cmake --build build -- -j${NUM_PROCS}
   mkdir -p ../../${CACHE_DIR}/${CACHE_NAME}/include
   cp -r *.h ../../${CACHE_DIR}/${CACHE_NAME}/include
   mkdir -p ../../${CACHE_DIR}/${CACHE_NAME}/lib
   cp -a build/*.{so,so.*} ../../${CACHE_DIR}/${CACHE_NAME}/lib
   cd ..
   touch "../${CACHE_DIR}/${CACHE_NAME}.cache"
fi

cp -r ../${CACHE_DIR}/${CACHE_NAME}/include/* ../external/include/SDL2
cp -a ../${CACHE_DIR}/${CACHE_NAME}/lib/*.{so,so.*} ../external/lib

#
# build libpinmame and copy to external
#

CACHE_NAME="pinmame-${PINMAME_SHA}"

if [ ! -f "../${CACHE_DIR}/${CACHE_NAME}.cache" ]; then
   curl -sL https://github.com/vpinball/pinmame/archive/${PINMAME_SHA}.zip -o pinmame.zip
   unzip pinmame.zip
   cd pinmame-$PINMAME_SHA
   cp cmake/libpinmame/CMakeLists_linux-x64.txt CMakeLists.txt
   cmake \
      -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
      -B build
   cmake --build build -- -j${NUM_PROCS}
   mkdir -p ../../${CACHE_DIR}/${CACHE_NAME}/include
   cp src/libpinmame/libpinmame.h ../../${CACHE_DIR}/${CACHE_NAME}/include
   mkdir -p ../../${CACHE_DIR}/${CACHE_NAME}/lib
   cp build/libpinmame.so.3.6 ../../${CACHE_DIR}/${CACHE_NAME}/lib
   cd ..
   touch "../${CACHE_DIR}/${CACHE_NAME}.cache"
fi

cp -r ../${CACHE_DIR}/${CACHE_NAME}/include/* ../external/include
cp -a ../${CACHE_DIR}/${CACHE_NAME}/lib/*.so.* ../external/lib

#
# build libaltsound and copy to external
#

CACHE_NAME="libaltsound-${LIBALTSOUND_SHA}"

if [ ! -f  "../${CACHE_DIR}/${CACHE_NAME}.cache" ]; then
   curl -sL https://github.com/vpinball/libaltsound/archive/${LIBALTSOUND_SHA}.zip -o libaltsound.zip
   unzip libaltsound.zip
   cd libaltsound-$LIBALTSOUND_SHA
   platforms/linux/aarch64/external.sh
   cmake \
      -DPLATFORM=linux \
      -DARCH=aarch64 \
      -DBUILD_STATIC=OFF \
      -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
      -B build
   cmake --build build -- -j${NUM_PROCS}
   mkdir -p ../../${CACHE_DIR}/${CACHE_NAME}/include
   cp src/altsound.h ../../${CACHE_DIR}/${CACHE_NAME}/include
   mkdir -p ../../${CACHE_DIR}/${CACHE_NAME}/lib
   cp -a build/*.{so,so.*} ../../${CACHE_DIR}/${CACHE_NAME}/lib
   cd ..
   touch "../${CACHE_DIR}/${CACHE_NAME}.cache"
fi

cp -r ../${CACHE_DIR}/${CACHE_NAME}/include/* ../external/include
cp -a ../${CACHE_DIR}/${CACHE_NAME}/lib/*.{so,so.*} ../external/lib

#
# build libdmdutil (and deps) and copy to external
#

CACHE_NAME="libdmdutil-${LIBDMDUTIL_SHA}"

if [ ! -f "../${CACHE_DIR}/${CACHE_NAME}.cache" ]; then
   curl -sL https://github.com/vpinball/libdmdutil/archive/${LIBDMDUTIL_SHA}.zip -o libdmdutil.zip
   unzip libdmdutil.zip
   cd libdmdutil-$LIBDMDUTIL_SHA
   platforms/linux/aarch64/external.sh
   cmake \
      -DPLATFORM=linux \
      -DARCH=aarch64 \
      -DBUILD_STATIC=OFF \
      -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
      -B build
   cmake --build build -- -j${NUM_PROCS}
   mkdir -p ../../${CACHE_DIR}/${CACHE_NAME}/include
   cp -r include/DMDUtil ../../${CACHE_DIR}/${CACHE_NAME}/include
   cp -r third-party/include/sockpp ../../${CACHE_DIR}/${CACHE_NAME}/include
   mkdir -p ../../${CACHE_DIR}/${CACHE_NAME}/lib
   cp -a third-party/runtime-libs/linux/aarch64/*.{so,so.*} ../../${CACHE_DIR}/${CACHE_NAME}/lib
   cp -a build/*.{so,so.*} ../../${CACHE_DIR}/${CACHE_NAME}/lib
   cd ..
   touch "../${CACHE_DIR}/${CACHE_NAME}.cache"
fi

cp -r ../${CACHE_DIR}/${CACHE_NAME}/include/* ../external/include
cp -a ../${CACHE_DIR}/${CACHE_NAME}/lib/*.{so,so.*} ../external/lib

#
# build libdof (and deps) and copy to external
#

CACHE_NAME="libdof-${LIBDOF_SHA}"

if [ ! -f "../${CACHE_DIR}/${CACHE_NAME}.cache" ]; then
   curl -sL https://github.com/jsm174/libdof/archive/${LIBDOF_SHA}.zip -o libdof.zip
   unzip libdof.zip
   cd libdof-$LIBDOF_SHA
   platforms/linux/aarch64/external.sh
   cmake \
      -DPLATFORM=linux \
      -DARCH=aarch64 \
      -DBUILD_STATIC=OFF \
      -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
      -B build
   cmake --build build -- -j${NUM_PROCS}
   mkdir -p ../../${CACHE_DIR}/${CACHE_NAME}/include
   cp -r include/DOF ../../${CACHE_DIR}/${CACHE_NAME}/include
   mkdir -p ../../${CACHE_DIR}/${CACHE_NAME}/lib
   cp -a third-party/runtime-libs/linux/aarch64/*.{so,so.*} ../../${CACHE_DIR}/${CACHE_NAME}/lib
   cp -a build/*.{so,so.*} ../../${CACHE_DIR}/${CACHE_NAME}/lib
   cd ..
   touch "../${CACHE_DIR}/${CACHE_NAME}.cache"
fi

cp -r ../${CACHE_DIR}/${CACHE_NAME}/include/* ../external/include
cp -a ../${CACHE_DIR}/${CACHE_NAME}/lib/*.{so,so.*} ../external/lib

#
# build FFMPEG libraries and copy to external
#

CACHE_NAME="FFmpeg-${FFMPEG_SHA}_001"

if [ ! -f "../${CACHE_DIR}/${CACHE_NAME}.cache" ]; then
   curl -sL https://github.com/FFmpeg/FFmpeg/archive/${FFMPEG_SHA}.zip -o ffmpeg.zip
   unzip ffmpeg.zip
   cd FFmpeg-$FFMPEG_SHA
   LDFLAGS=-Wl,-rpath,\''$$$$ORIGIN'\' ./configure \
      --enable-shared \
      --disable-static \
      --disable-programs \
      --disable-doc
   make -j${NUM_PROCS}
   mkdir -p ../../${CACHE_DIR}/${CACHE_NAME}/lib
   for lib in libavcodec libavdevice libavfilter libavformat libavutil libswresample libswscale; do
      mkdir -p ../../${CACHE_DIR}/${CACHE_NAME}/include/${lib}
      cp ${lib}/*.h ../../${CACHE_DIR}/${CACHE_NAME}/include/${lib}
      cp -a ${lib}/*.{so,so.*} ../../${CACHE_DIR}/${CACHE_NAME}/lib
   done
   cd ..
   touch "../${CACHE_DIR}/${CACHE_NAME}.cache"
fi

cp -r ../${CACHE_DIR}/${CACHE_NAME}/include/* ../external/include
cp -a ../${CACHE_DIR}/${CACHE_NAME}/lib/*.{so,so.*} ../external/lib

