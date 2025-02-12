#!/bin/bash

set -e

FREEIMAGE_SHA=3532c976f9853219d42d8cbab6c1f4c0990e3ed8
SDL_SHA=b5c3eab6b447111d3c7879bb547b80fb4abd9063
SDL_IMAGE_SHA=4a762bdfb7b43dae7a8a818567847881e49bdab4
SDL_TTF_SHA=07e4d1241817f2c0f81749183fac5ec82d7bbd72
PINMAME_SHA=62b70673f58b22a3945e6c1dce70757207a54834
LIBALTSOUND_SHA=b8f397858cbc7a879f7392c14a509f00c8bdc7dd
LIBDMDUTIL_SHA=5afd52cae1a7ac2f5e86722045da47ec3e876708
LIBDOF_SHA=5c43c99ea28b44bb58b74554c4303a505e208148
FFMPEG_SHA=b08d7969c550a804a59511c7b83f2dd8cc0499b8
BGFX_CMAKE_VERSION=1.129.8863-490
BGFX_PATCH_SHA=1d0967155c375155d1f778ded4061f35c80fc96f

NUM_PROCS=$(sysctl -n hw.ncpu)

echo "Building external libraries..."
echo "  FREEIMAGE_SHA: ${FREEIMAGE_SHA}"
echo "  SDL_SHA: ${SDL_SHA}"
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

CACHE_NAME="FreeImage-${FREEIMAGE_SHA}"

if [ ! -f "../${CACHE_DIR}/${CACHE_NAME}.cache" ]; then
   curl -sL https://github.com/toxieainc/freeimage/archive/${FREEIMAGE_SHA}.zip -o freeimage-${FREEIMAGE_SHA}.zip
   unzip freeimage-${FREEIMAGE_SHA}.zip
   cd freeimage-${FREEIMAGE_SHA}
   cp ../../freeimage/CMakeLists.txt .
   cp ../../freeimage/PluginEXR.cpp Source/FreeImage/PluginEXR.cpp
   cmake \
      -DPLATFORM=macos \
      -DARCH=x64 \
      -DBUILD_STATIC=OFF \
      -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
      -B build
   cmake --build build -- -j${NUM_PROCS}
   mkdir -p ../../${CACHE_DIR}/${CACHE_NAME}/include
   cp Source/FreeImage.h ../../${CACHE_DIR}/${CACHE_NAME}/include
   mkdir -p ../../${CACHE_DIR}/${CACHE_NAME}/lib
   cp -a build/*.dylib ../../${CACHE_DIR}/${CACHE_NAME}/lib
   cd ..
   touch "../${CACHE_DIR}/${CACHE_NAME}.cache"
fi

cp -r ../${CACHE_DIR}/${CACHE_NAME}/include/* ../external/include
cp -a ../${CACHE_DIR}/${CACHE_NAME}/lib/*.dylib ../external/lib

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

CACHE_NAME="SDL-${SDL_SHA}-${SDL_IMAGE_SHA}-${SDL_TTF_SHA}"

if [ ! -f "../${CACHE_DIR}/${CACHE_NAME}.cache" ]; then
   curl -sL https://github.com/libsdl-org/SDL/archive/${SDL_SHA}.zip -o SDL-${SDL_SHA}.zip
   unzip SDL-${SDL_SHA}.zip
   cd SDL-${SDL_SHA}
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
      -DSDL3_DIR=../SDL-${SDL_SHA}/build \
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
      -DSDL3_DIR=../SDL-${SDL_SHA}/build \
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
   curl -sL https://github.com/vbousquet/pinmame/archive/${PINMAME_SHA}.zip -o pinmame.zip
   unzip pinmame.zip
   cd pinmame-$PINMAME_SHA
   cp cmake/libpinmame/CMakeLists.txt .
   cmake \
      -DPLATFORM=macos \
      -DARCH=x64 \
      -DBUILD_STATIC=OFF \
      -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
      -B build
   cmake --build build -- -j${NUM_PROCS}
   mkdir -p ../../${CACHE_DIR}/${CACHE_NAME}/include
   cp src/libpinmame/libpinmame.h ../../${CACHE_DIR}/${CACHE_NAME}/include
   cp src/libpinmame/pinmamedef.h ../../${CACHE_DIR}/${CACHE_NAME}/include
   mkdir -p ../../${CACHE_DIR}/${CACHE_NAME}/lib
   cp build/libpinmame.dylib ../../${CACHE_DIR}/${CACHE_NAME}/lib
   cd ..
   touch "../${CACHE_DIR}/${CACHE_NAME}.cache"
fi

cp -r ../${CACHE_DIR}/${CACHE_NAME}/include/* ../external/include
cp -a ../${CACHE_DIR}/${CACHE_NAME}/lib/*.dylib ../external/lib

#
# build libaltsound and copy to external
#

CACHE_NAME="libaltsound-${LIBALTSOUND_SHA}"

if [ ! -f "../${CACHE_DIR}/${CACHE_NAME}.cache" ]; then
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
