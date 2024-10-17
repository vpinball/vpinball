#!/bin/bash

set -e

FREEIMAGE_VERSION=3.18.0
SDL_VERSION=3.1.3
SDL_IMAGE_SHA=a010117fee88255a32492ae9a43e93a213d608ec
SDL_TTF_SHA=45faf5a38bd8f9319ac0fe66cfcc4ceb192f9fa4
PINMAME_SHA=65cdbf58bc7be6ef414ea273d1bb8d4cb42471c8
LIBALTSOUND_SHA=b8f397858cbc7a879f7392c14a509f00c8bdc7dd
LIBDMDUTIL_SHA=8e110d87edab1b843d97ba831743c79519e07ad8
LIBDOF_SHA=5c43c99ea28b44bb58b74554c4303a505e208148
FFMPEG_SHA=b08d7969c550a804a59511c7b83f2dd8cc0499b8
BGFX_CMAKE_VERSION=1.128.8808-482
BGFX_PATCH_SHA=a0d4c179527a4a4d205598ebf290c0b45144bda8

NUM_PROCS=$(sysctl -n hw.ncpu)

echo "Building external libraries..."
echo "  FREEIMAGE_VERSION: ${FREEIMAGE_VERSION}"
echo "  SDL_VERSION: ${SDL_VERSION}"
echo "  SDL_IMAGE_SHA: ${SDL_IMAGE_SHA}"
echo "  SDL_TTF_SHA: ${SDL_TTF_SHA}"
echo "  PINMAME_SHA: ${PINMAME_SHA}"
echo "  LIBALTSOUND_SHA: ${LIBALTSOUND_SHA}"
echo "  LIBDMDUTIL_SHA: ${LIBDMDUTIL_SHA}"
echo "  LIBDOF_SHA: ${LIBDOF_SHA}"
echo "  FFMPEG_SHA: ${FFMPEG_SHA}"
echo "  BGFX_CMAKE_VERSION: ${BGFX_CMAKE_VERSION}"
echo "  BGFX_PATCH_SHA: ${BGFX_PATCH_SHA}"
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
   cp ../../freeimage/Makefile.macos .
   make -f Makefile.macos -j${NUM_PROCS}
   mkdir -p ../../${CACHE_DIR}/${CACHE_NAME}/lib
   cp Dist/libfreeimage-x86_64.a ../../${CACHE_DIR}/${CACHE_NAME}/lib/libfreeimage.a
   cd ..
   touch "../${CACHE_DIR}/${CACHE_NAME}.cache"
fi

cp ../${CACHE_DIR}/${CACHE_NAME}/lib/*.a ../external/lib

#
# download bass24 and copy to external
#

CACHE_NAME="bass24_003"

if [ ! -f "../${CACHE_DIR}/${CACHE_NAME}.cache" ]; then
   curl -sL https://www.un4seen.com/files/bass24-osx.zip -o bass.zip
   unzip bass.zip
   lipo libbass.dylib -extract x86_64 -output libbass-x64.dylib
   codesign --force --sign - libbass-x64.dylib
   mkdir -p ../${CACHE_DIR}/${CACHE_NAME}/lib
   cp libbass-x64.dylib ../${CACHE_DIR}/${CACHE_NAME}/lib/libbass.dylib
   touch "../${CACHE_DIR}/${CACHE_NAME}.cache"
fi

cp -a ../${CACHE_DIR}/${CACHE_NAME}/lib/*.dylib ../external/lib

#
# build SDL3, SDL_image, SDL_ttf and copy to external
#

CACHE_NAME="SDL-${SDL_VERSION}-${SDL_IMAGE_SHA}-${SDL_TTF_SHA}_002"

if [ ! -f "../${CACHE_DIR}/${CACHE_NAME}.cache" ]; then
   curl -sL https://github.com/libsdl-org/SDL/releases/download/preview-${SDL_VERSION}/SDL3-${SDL_VERSION}.tar.xz -o SDL3-${SDL_VERSION}.tar.xz
   tar -xf SDL3-3.1.3.tar.xz
   cd SDL3-3.1.3
   cmake \
      -DSDL_SHARED=ON \
      -DSDL_STATIC=OFF \
      -DSDL_TEST_LIBRARY=OFF \
      -DSDL_OPENGLES=OFF \
      -DCMAKE_OSX_ARCHITECTURES=x86_64 \
      -DCMAKE_OSX_DEPLOYMENT_TARGET=14.0 \
      -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
      -B build
   cmake --build build -- -j${NUM_PROCS}
   mkdir -p ../../${CACHE_DIR}/${CACHE_NAME}/include/SDL3
   cp -r include/SDL3/* ../../${CACHE_DIR}/${CACHE_NAME}/include/SDL3
   mkdir -p ../../${CACHE_DIR}/${CACHE_NAME}/lib
   cp -a build/*.dylib ../../${CACHE_DIR}/${CACHE_NAME}/lib
   cd ..

   curl -sL https://github.com/libsdl-org/SDL_image/archive/${SDL_IMAGE_SHA}.zip -o SDL_image-${SDL_IMAGE_SHA}.zip
   unzip SDL_image-${SDL_IMAGE_SHA}.zip
   cd SDL_image-${SDL_IMAGE_SHA}
   external/download.sh
   cmake \
      -DBUILD_SHARED_LIBS=ON \
      -DSDLIMAGE_SAMPLES=OFF \
      -DSDLIMAGE_DEPS_SHARED=ON \
      -DSDLIMAGE_VENDORED=ON \
      -DSDLIMAGE_AVIF=OFF \
      -DSDLIMAGE_WEBP=OFF \
      -DSDL3_DIR=../SDL3-${SDL_VERSION}/build \
      -DCMAKE_OSX_ARCHITECTURES=x86_64 \
      -DCMAKE_OSX_DEPLOYMENT_TARGET=14.0 \
      -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
      -B build
   cmake --build build -- -j${NUM_PROCS}
   mkdir -p ../../${CACHE_DIR}/${CACHE_NAME}/include/SDL3_image
   cp -r include/SDL3_image/* ../../${CACHE_DIR}/${CACHE_NAME}/include/SDL3_image
   mkdir -p ../../${CACHE_DIR}/${CACHE_NAME}/include/lib
   cp -a build/*.dylib ../../${CACHE_DIR}/${CACHE_NAME}/lib
   cd ..

   curl -sL https://github.com/libsdl-org/SDL_ttf/archive/${SDL_TTF_SHA}.zip -o SDL_ttf-${SDL_TTF_SHA}.zip
   unzip SDL_ttf-${SDL_TTF_SHA}.zip
   cd SDL_ttf-${SDL_TTF_SHA}
   external/download.sh
   cmake \
      -DBUILD_SHARED_LIBS=ON \
      -DSDLTTF_SAMPLES=OFF \
      -DSDLTTF_VENDORED=ON \
      -DSDLTTF_HARFBUZZ=ON \
      -DSDL3_DIR=../SDL3-${SDL_VERSION}/build \
      -DCMAKE_OSX_ARCHITECTURES=x86_64 \
      -DCMAKE_OSX_DEPLOYMENT_TARGET=14.0 \
      -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
      -B build
   cmake --build build -- -j${NUM_PROCS}
   mkdir -p ../../${CACHE_DIR}/${CACHE_NAME}/include/SDL3_ttf
   cp -r include/SDL3_ttf/* ../../${CACHE_DIR}/${CACHE_NAME}/include/SDL3_ttf
   mkdir -p ../../${CACHE_DIR}/${CACHE_NAME}/include/lib
   cp -a build/*.dylib ../../${CACHE_DIR}/${CACHE_NAME}/lib
   cd ..

   touch "../${CACHE_DIR}/${CACHE_NAME}.cache"
fi

mkdir -p ../external/include/SDL3
cp -r ../${CACHE_DIR}/${CACHE_NAME}/include/SDL3/* ../external/include/SDL3
mkdir -p ../external/include/SDL3_image
cp -r ../${CACHE_DIR}/${CACHE_NAME}/include/SDL3_image/* ../external/include/SDL3_image
mkdir -p ../external/include/SDL3_ttf
cp -r ../${CACHE_DIR}/${CACHE_NAME}/include/SDL3_ttf/* ../external/include/SDL3_ttf
cp -a ../${CACHE_DIR}/${CACHE_NAME}/lib/*.dylib ../external/lib

#
# build libpinmame and copy to external
#

CACHE_NAME="pinmame-${PINMAME_SHA}"

if [ ! -f "../${CACHE_DIR}/${CACHE_NAME}.cache" ]; then
   curl -sL https://github.com/vpinball/pinmame/archive/${PINMAME_SHA}.zip -o pinmame.zip
   unzip pinmame.zip
   cd pinmame-$PINMAME_SHA
   cp cmake/libpinmame/CMakeLists_osx-x64.txt CMakeLists.txt
   cmake \
      -DBUILD_STATIC=OFF \
      -DCMAKE_OSX_DEPLOYMENT_TARGET=14.0 \
      -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
      -B build
   cmake --build build -- -j${NUM_PROCS}
   mkdir -p ../../${CACHE_DIR}/${CACHE_NAME}/include
   cp src/libpinmame/libpinmame.h ../../${CACHE_DIR}/${CACHE_NAME}/include
   mkdir -p ../../${CACHE_DIR}/${CACHE_NAME}/lib
   cp build/libpinmame.3.6.dylib ../../${CACHE_DIR}/${CACHE_NAME}/lib
   cd ..
   touch "../${CACHE_DIR}/${CACHE_NAME}.cache"
fi

cp -r ../${CACHE_DIR}/${CACHE_NAME}/include/* ../external/include
cp -a ../${CACHE_DIR}/${CACHE_NAME}/lib/*.dylib ../external/lib

#
# build libaltsound and copy to external
#

CACHE_NAME="libaltsound-${LIBALTSOUND_SHA}"

if [ ! -f  "../${CACHE_DIR}/${CACHE_NAME}.cache" ]; then
   curl -sL https://github.com/vpinball/libaltsound/archive/${LIBALTSOUND_SHA}.zip -o libaltsound.zip
   unzip libaltsound.zip
   cd libaltsound-$LIBALTSOUND_SHA
   platforms/macos/x64/external.sh
   cmake \
      -DPLATFORM=macos \
      -DARCH=x64 \
      -DBUILD_STATIC=OFF \
      -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
      -B build
   cmake --build build -- -j${NUM_PROCS}
   mkdir -p ../../${CACHE_DIR}/${CACHE_NAME}/include
   cp src/altsound.h ../../${CACHE_DIR}/${CACHE_NAME}/include
   mkdir -p ../../${CACHE_DIR}/${CACHE_NAME}/lib
   cp -a build/*.dylib ../../${CACHE_DIR}/${CACHE_NAME}/lib
   cd ..
   touch "../${CACHE_DIR}/${CACHE_NAME}.cache"
fi

cp -r ../${CACHE_DIR}/${CACHE_NAME}/include/* ../external/include
cp -a ../${CACHE_DIR}/${CACHE_NAME}/lib/*.dylib ../external/lib

#
# build libdmdutil (and deps) and copy to external
#

CACHE_NAME="libdmdutil-${LIBDMDUTIL_SHA}"

if [ ! -f "../${CACHE_DIR}/${CACHE_NAME}.cache" ]; then
   curl -sL https://github.com/vpinball/libdmdutil/archive/${LIBDMDUTIL_SHA}.zip -o libdmdutil.zip
   unzip libdmdutil.zip
   cd libdmdutil-$LIBDMDUTIL_SHA
   platforms/macos/x64/external.sh
   cmake \
      -DPLATFORM=macos \
      -DARCH=x64 \
      -DBUILD_STATIC=OFF \
      -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
      -B build
   cmake --build build -- -j${NUM_PROCS}
   mkdir -p ../../${CACHE_DIR}/${CACHE_NAME}/include
   cp -r include/DMDUtil ../../${CACHE_DIR}/${CACHE_NAME}/include
   cp -r third-party/include/sockpp ../../${CACHE_DIR}/${CACHE_NAME}/include
   mkdir -p ../../${CACHE_DIR}/${CACHE_NAME}/lib
   cp -a third-party/runtime-libs/macos/x64/*.dylib ../../${CACHE_DIR}/${CACHE_NAME}/lib
   cp -a build/*.dylib ../../${CACHE_DIR}/${CACHE_NAME}/lib
   cd ..
   touch "../${CACHE_DIR}/${CACHE_NAME}.cache"
fi

cp -r ../${CACHE_DIR}/${CACHE_NAME}/include/* ../external/include
cp -a ../${CACHE_DIR}/${CACHE_NAME}/lib/*.dylib ../external/lib

#
# build libdof (and deps) and copy to external
#

CACHE_NAME="libdof-${LIBDOF_SHA}"

if [ ! -f "../${CACHE_DIR}/${CACHE_NAME}.cache" ]; then
   curl -sL https://github.com/jsm174/libdof/archive/${LIBDOF_SHA}.zip -o libdof.zip
   unzip libdof.zip
   cd libdof-$LIBDOF_SHA
   platforms/macos/x64/external.sh
   cmake \
      -DPLATFORM=macos \
      -DARCH=x64 \
      -DBUILD_STATIC=OFF \
      -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
      -B build
   cmake --build build -- -j${NUM_PROCS}
   mkdir -p ../../${CACHE_DIR}/${CACHE_NAME}/include
   cp -r include/DOF ../../${CACHE_DIR}/${CACHE_NAME}/include
   mkdir -p ../../${CACHE_DIR}/${CACHE_NAME}/lib
   cp -a third-party/runtime-libs/macos/x64/*.dylib ../../${CACHE_DIR}/${CACHE_NAME}/lib
   cp -a build/*.dylib ../../${CACHE_DIR}/${CACHE_NAME}/lib
   cd ..
   touch "../${CACHE_DIR}/${CACHE_NAME}.cache"
fi

cp -r ../${CACHE_DIR}/${CACHE_NAME}/include/* ../external/include
cp -a ../${CACHE_DIR}/${CACHE_NAME}/lib/*.dylib ../external/lib

#
# build FFMPEG libraries and copy to external
#

CACHE_NAME="FFmpeg-${FFMPEG_SHA}"

if [ ! -f "../${CACHE_DIR}/${CACHE_NAME}.cache" ]; then
   curl -sL https://github.com/FFmpeg/FFmpeg/archive/${FFMPEG_SHA}.zip -o ffmpeg.zip
   unzip ffmpeg.zip
   cd ffmpeg-$FFMPEG_SHA
   ./configure --enable-cross-compile \
      --enable-shared \
      --disable-static \
      --disable-programs \
      --disable-doc \
      --disable-xlib \
      --disable-libxcb \
      --enable-rpath \
      --prefix=. \
      --libdir=@rpath \
      --arch=x86_64 \
      --cc='clang -arch x86_64' \
      --extra-ldflags='-Wl,-ld_classic'
   make -j${NUM_PROCS}
   mkdir -p ../../${CACHE_DIR}/${CACHE_NAME}/lib
   for lib in libavcodec libavdevice libavfilter libavformat libavutil libswresample libswscale; do
      mkdir -p ../../${CACHE_DIR}/${CACHE_NAME}/include/${lib}
      cp ${lib}/*.h ../../${CACHE_DIR}/${CACHE_NAME}/include/${lib}
      cp -a ${lib}/*.dylib ../../${CACHE_DIR}/${CACHE_NAME}/lib
   done
   cd ..
   touch "../${CACHE_DIR}/${CACHE_NAME}.cache"
fi

cp -r ../${CACHE_DIR}/${CACHE_NAME}/include/* ../external/include
cp -a ../${CACHE_DIR}/${CACHE_NAME}/lib/*.dylib ../external/lib

#
# build patched bgfx and copy to external
#

CACHE_NAME="BGFX_CMAKE-${BGFX_CMAKE_VERSION}-${BGFX_PATCH_SHA}"

if [ ! -f "../${CACHE_DIR}/${CACHE_NAME}.cache" ]; then
   curl -sL https://github.com/bkaradzic/bgfx.cmake/releases/download/v${BGFX_CMAKE_VERSION}/bgfx.cmake.v${BGFX_CMAKE_VERSION}.tar.gz -o bgfx.cmake.v${BGFX_CMAKE_VERSION}.tar.gz
   tar -xvzf bgfx.cmake.v${BGFX_CMAKE_VERSION}.tar.gz
   curl -sL https://github.com/vbousquet/bgfx/archive/${BGFX_PATCH_SHA}.zip -o bgfx.zip
   unzip bgfx.zip
   cd bgfx.cmake   
   rm -rf bgfx
   mv ../bgfx-${BGFX_PATCH_SHA} bgfx
   cmake -S. \
      -DBGFX_LIBRARY_TYPE=SHARED \
      -DBGFX_BUILD_EXAMPLES=OFF \
      -DBGFX_CONFIG_MULTITHREADED=ON \
      -DBGFX_CONFIG_MAX_FRAME_BUFFERS=256 \
      -DCMAKE_OSX_ARCHITECTURES=x86_64 \
      -DCMAKE_OSX_DEPLOYMENT_TARGET=14.0 \
      -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
      -B build
   cmake --build build -- -j${NUM_PROCS}
   mkdir -p ../../${CACHE_DIR}/${CACHE_NAME}/include
   cp -r bgfx/include/bgfx ../../${CACHE_DIR}/${CACHE_NAME}/include
   cp -r bimg/include/bimg ../../${CACHE_DIR}/${CACHE_NAME}/include
   cp -r bx/include/bx ../../${CACHE_DIR}/${CACHE_NAME}/include
   mkdir -p ../../${CACHE_DIR}/${CACHE_NAME}/lib
   cp build/cmake/bgfx/libbgfx.dylib ../../${CACHE_DIR}/${CACHE_NAME}/lib
   cd ..
   touch "../${CACHE_DIR}/${CACHE_NAME}.cache"
fi

cp -r ../${CACHE_DIR}/${CACHE_NAME}/include/* ../external/include
cp ../${CACHE_DIR}/${CACHE_NAME}/lib/*.dylib ../external/lib
