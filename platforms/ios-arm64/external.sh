#!/bin/bash

set -e

source ./platforms/config.sh

echo "Building external libraries..."
echo "  SDL_SHA: ${SDL_SHA}"
echo "  SDL_IMAGE_SHA: ${SDL_IMAGE_SHA}"
echo "  SDL_TTF_SHA: ${SDL_TTF_SHA}"
echo "  FREEIMAGE_SHA: ${FREEIMAGE_SHA}"
echo "  BGFX_CMAKE_VERSION: ${BGFX_CMAKE_VERSION}"
echo "  BGFX_PATCH_SHA: ${BGFX_PATCH_SHA}"
echo "  PINMAME_SHA: ${PINMAME_SHA}"
echo "  PINMAME_MEMMAPS_SHA: ${PINMAME_MEMMAPS_SHA}"
echo "  PINMAME_GAMEDEFS_SHA: ${PINMAME_GAMEDEFS_SHA}"
echo "  LIBDMDUTIL_SHA: ${LIBDMDUTIL_SHA}"
echo "  LIBSERUM_SHA: ${LIBSERUM_SHA}"
echo "  LIBPUPDMD_SHA: ${LIBPUPDMD_SHA}"
echo "  LIBVNI_SHA: ${LIBVNI_SHA}"
echo "  LIBALTSOUND_SHA: ${LIBALTSOUND_SHA}"
echo "  LIBDOF_SHA: ${LIBDOF_SHA}"
echo "  LIBWINEVBS_SHA: ${LIBWINEVBS_SHA}"
echo "  FFMPEG_SHA: ${FFMPEG_SHA}"
echo "  LIBZIP_SHA: ${LIBZIP_SHA}"
echo "  LIBMYSOFA_SHA: ${LIBMYSOFA_SHA}"
echo "  LIBOGG_SHA: ${LIBOGG_SHA}"
echo "  LIBOPUS_SHA: ${LIBOPUS_SHA}"
echo "  OPUSFILE_SHA: ${OPUSFILE_SHA}"
echo "  LIBSPATIALAUDIO_SHA: ${LIBSPATIALAUDIO_SHA}"
echo "  ZLIB_SHA: ${ZLIB_SHA}"
echo ""

NUM_PROCS=$(sysctl -n hw.ncpu)

mkdir -p "external/ios-arm64/${BUILD_TYPE}"
cd "external/ios-arm64/${BUILD_TYPE}"

#
# build SDL3, SDL3_image, SDL3_ttf#

SDL3_EXPECTED_SHA="${SDL_SHA}-${SDL_IMAGE_SHA}-${SDL_TTF_SHA}_003"
SDL3_FOUND_SHA="$([ -f SDL3/cache.txt ] && cat SDL3/cache.txt || echo "")"

if [ "${SDL3_EXPECTED_SHA}" != "${SDL3_FOUND_SHA}" ]; then
   echo "Building SDL3. Expected: ${SDL3_EXPECTED_SHA}, Found: ${SDL3_FOUND_SHA}"

   rm -rf SDL3
   mkdir SDL3
   cd SDL3

   curl -sL https://github.com/libsdl-org/SDL/archive/${SDL_SHA}.tar.gz -o SDL-${SDL_SHA}.tar.gz
   tar xzf SDL-${SDL_SHA}.tar.gz
   mv SDL-${SDL_SHA} SDL
   cd SDL
   cp -R ../../../../../platforms/ios-arm64/SDL/. .
   cmake \
      -DSDL_SHARED=OFF \
      -DSDL_STATIC=ON \
      -DSDL_TEST_LIBRARY=OFF \
      -DSDL_OPENGLES=OFF \
      -DSDL_CAMERA=OFF \
      -DCMAKE_SYSTEM_NAME=iOS \
      -DCMAKE_OSX_DEPLOYMENT_TARGET=16.0 \
      -DCMAKE_OSX_ARCHITECTURES=arm64 \
      -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
      -B build
   cmake --build build -- -j${NUM_PROCS}
   cd ..

   curl -sL https://github.com/libsdl-org/SDL_image/archive/${SDL_IMAGE_SHA}.tar.gz -o SDL_image-${SDL_IMAGE_SHA}.tar.gz
   tar xzf SDL_image-${SDL_IMAGE_SHA}.tar.gz
   mv SDL_image-${SDL_IMAGE_SHA} SDL_image
   cd SDL_image
   ./external/download.sh
   cmake \
      -DBUILD_SHARED_LIBS=OFF \
      -DSDLIMAGE_SAMPLES=OFF \
      -DSDLIMAGE_DEPS_SHARED=OFF \
      -DSDLIMAGE_VENDORED=ON \
      -DSDLIMAGE_AVIF=OFF \
      -DSDLIMAGE_WEBP=OFF \
      -DSDL3_DIR=../SDL/build \
      -DCMAKE_SYSTEM_NAME=iOS \
      -DCMAKE_OSX_DEPLOYMENT_TARGET=16.0 \
      -DCMAKE_OSX_ARCHITECTURES=arm64 \
      -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
      -B build
   cmake --build build -- -j${NUM_PROCS}
   cd ..

   curl -sL https://github.com/libsdl-org/SDL_ttf/archive/${SDL_TTF_SHA}.tar.gz -o SDL_ttf-${SDL_TTF_SHA}.tar.gz
   tar xzf SDL_ttf-${SDL_TTF_SHA}.tar.gz
   mv SDL_ttf-${SDL_TTF_SHA} SDL_ttf
   cd SDL_ttf
   ./external/download.sh
   cmake \
      -DBUILD_SHARED_LIBS=OFF \
      -DSDLTTF_SAMPLES=OFF \
      -DSDLTTF_VENDORED=ON \
      -DSDLTTF_HARFBUZZ=ON \
      -DSDL3_DIR=../SDL/build \
      -DCMAKE_SYSTEM_NAME=iOS \
      -DCMAKE_OSX_DEPLOYMENT_TARGET=16.0 \
      -DCMAKE_OSX_ARCHITECTURES=arm64 \
      -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
      -B build
   cmake --build build -- -j${NUM_PROCS}
   if [ "${BUILD_TYPE}" == "Debug" ]; then
      cp build/external/freetype/libfreetyped.a build/external/freetype/libfreetype.a
   fi
   cd ..

   echo "$SDL3_EXPECTED_SHA" > cache.txt

   cd ..
fi

#
# build freeimage
#

FREEIMAGE_EXPECTED_SHA="${FREEIMAGE_SHA}"
FREEIMAGE_FOUND_SHA="$([ -f freeimage/cache.txt ] && cat freeimage/cache.txt || echo "")"

if [ "${FREEIMAGE_EXPECTED_SHA}" != "${FREEIMAGE_FOUND_SHA}" ]; then
   echo "Building FreeImage. Expected: ${FREEIMAGE_EXPECTED_SHA}, Found: ${FREEIMAGE_FOUND_SHA}"

   rm -rf freeimage
   mkdir freeimage
   cd freeimage

   curl -sL https://github.com/toxieainc/freeimage/archive/${FREEIMAGE_SHA}.tar.gz -o freeimage-${FREEIMAGE_SHA}.tar.gz
   tar xzf freeimage-${FREEIMAGE_SHA}.tar.gz
   mv freeimage-${FREEIMAGE_SHA} freeimage
   cd freeimage
   cmake \
      -DPLATFORM=ios \
      -DARCH=arm64 \
      -DBUILD_SHARED=OFF \
      -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
      -B build
   cmake --build build -- -j${NUM_PROCS}
   cd ..

   echo "$FREEIMAGE_EXPECTED_SHA" > cache.txt

   cd ..
fi

#
# build bgfx
#

BGFX_EXPECTED_SHA="${BGFX_CMAKE_VERSION}-${BGFX_PATCH_SHA}-001"
BGFX_FOUND_SHA="$([ -f bgfx/cache.txt ] && cat bgfx/cache.txt || echo "")"

if [ "${BGFX_EXPECTED_SHA}" != "${BGFX_FOUND_SHA}" ]; then
   echo "Building BGFX. Expected: ${BGFX_EXPECTED_SHA}, Found: ${BGFX_FOUND_SHA}"

   rm -rf bgfx
   mkdir bgfx
   cd bgfx

   curl -sL https://github.com/bkaradzic/bgfx.cmake/releases/download/v${BGFX_CMAKE_VERSION}/bgfx.cmake.v${BGFX_CMAKE_VERSION}.tar.gz -o bgfx.cmake.v${BGFX_CMAKE_VERSION}.tar.gz
   tar xzf bgfx.cmake.v${BGFX_CMAKE_VERSION}.tar.gz
   curl -sL https://github.com/vbousquet/bgfx/archive/${BGFX_PATCH_SHA}.tar.gz -o bgfx-${BGFX_PATCH_SHA}.tar.gz
   tar xzf bgfx-${BGFX_PATCH_SHA}.tar.gz
   cd bgfx.cmake
   rm -rf bgfx
   mv ../bgfx-${BGFX_PATCH_SHA} bgfx
   cmake -S. \
      -DBGFX_BUILD_EXAMPLES=OFF \
      -DBGFX_BUILD_TOOLS=OFF \
      -DBGFX_CONFIG_MULTITHREADED=ON \
      -DBGFX_CONFIG_MAX_FRAME_BUFFERS=256 \
      -DCMAKE_SYSTEM_NAME=iOS \
      -DCMAKE_OSX_ARCHITECTURES=arm64 \
      -DCMAKE_OSX_DEPLOYMENT_TARGET=16.0 \
      -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
      -B build
   cmake --build build -- -j${NUM_PROCS}

   # Build shaderc as a host tool (always Release: it only runs at build time to
   # compile the embedded shaders, see src/shaders/bgfx/shaders.cmake). The main
   # build targets iOS, so the tool needs a separate host (macOS) configure.
   cmake -S. \
      -DBGFX_BUILD_TOOLS=ON \
      -DBGFX_BUILD_TOOLS_SHADER=ON \
      -DBGFX_BUILD_TOOLS_BIN2C=OFF \
      -DBGFX_BUILD_TOOLS_GEOMETRY=OFF \
      -DBGFX_BUILD_TOOLS_TEXTURE=OFF \
      -DBGFX_BUILD_EXAMPLES=OFF \
      -DCMAKE_BUILD_TYPE=Release \
      -B build-tools
   cmake --build build-tools --target shaderc -- -j${NUM_PROCS}
   cd ..

   echo "$BGFX_EXPECTED_SHA" > cache.txt

   cd ..
fi

#
# build pinmame
#

PINMAME_EXPECTED_SHA="${PINMAME_SHA}"
PINMAME_FOUND_SHA="$([ -f pinmame/cache.txt ] && cat pinmame/cache.txt || echo "")"

if [ "${PINMAME_EXPECTED_SHA}" != "${PINMAME_FOUND_SHA}" ]; then
   echo "Building libpinmame. Expected: ${PINMAME_EXPECTED_SHA}, Found: ${PINMAME_FOUND_SHA}"

   rm -rf pinmame
   mkdir pinmame
   cd pinmame

   curl -sL https://github.com/vpinball/pinmame/archive/${PINMAME_SHA}.tar.gz -o pinmame-${PINMAME_SHA}.tar.gz
   tar xzf pinmame-${PINMAME_SHA}.tar.gz
   mv pinmame-${PINMAME_SHA} pinmame
   cd pinmame
   cp cmake/libpinmame/CMakeLists.txt .
   cmake \
      -DPLATFORM=ios \
      -DARCH=arm64 \
      -DBUILD_SHARED=OFF \
      -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
      -B build
   cmake --build build -- -j${NUM_PROCS}
   cd ..

   echo "$PINMAME_EXPECTED_SHA" > cache.txt

   cd ..
fi

#
# fetch pinmame machine data
#

PINMAME_DATA_EXPECTED_SHA="${PINMAME_MEMMAPS_SHA}-${PINMAME_GAMEDEFS_SHA}"
PINMAME_DATA_FOUND_SHA="$([ -f pinmame-data/cache.txt ] && cat pinmame-data/cache.txt || echo "")"

if [ "${PINMAME_DATA_EXPECTED_SHA}" != "${PINMAME_DATA_FOUND_SHA}" ]; then
   echo "Fetching PinMAME machine data. Expected: ${PINMAME_DATA_EXPECTED_SHA}, Found: ${PINMAME_DATA_FOUND_SHA}"

   rm -rf pinmame-data
   mkdir pinmame-data
   cd pinmame-data

   curl -sL https://github.com/tomlogic/pinball-memory-maps/archive/${PINMAME_MEMMAPS_SHA}.tar.gz -o pinball-memory-maps-${PINMAME_MEMMAPS_SHA}.tar.gz
   tar xzf pinball-memory-maps-${PINMAME_MEMMAPS_SHA}.tar.gz
   mv pinball-memory-maps-${PINMAME_MEMMAPS_SHA} memmaps

   curl -sL https://github.com/vpinball/pinmame-game-defs/archive/${PINMAME_GAMEDEFS_SHA}.tar.gz -o pinmame-game-defs-${PINMAME_GAMEDEFS_SHA}.tar.gz
   tar xzf pinmame-game-defs-${PINMAME_GAMEDEFS_SHA}.tar.gz
   mv pinmame-game-defs-${PINMAME_GAMEDEFS_SHA} gamedefs

   echo "$PINMAME_DATA_EXPECTED_SHA" > cache.txt

   cd ..
fi

#
# build libdmdutil
#

LIBDMDUTIL_EXPECTED_SHA="${LIBDMDUTIL_SHA}"
LIBDMDUTIL_FOUND_SHA="$([ -f libdmdutil/cache.txt ] && cat libdmdutil/cache.txt || echo "")"

if [ "${LIBDMDUTIL_EXPECTED_SHA}" != "${LIBDMDUTIL_FOUND_SHA}" ]; then
   echo "Building libdmdutil. Expected: ${LIBDMDUTIL_EXPECTED_SHA}, Found: ${LIBDMDUTIL_FOUND_SHA}"

   rm -rf libdmdutil
   mkdir libdmdutil
   cd libdmdutil

   curl -sL https://github.com/vpinball/libdmdutil/archive/${LIBDMDUTIL_SHA}.tar.gz -o libdmdutil-${LIBDMDUTIL_SHA}.tar.gz
   tar xzf libdmdutil-${LIBDMDUTIL_SHA}.tar.gz
   mv libdmdutil-${LIBDMDUTIL_SHA} libdmdutil
   cd libdmdutil
   ENABLE_SERUM=OFF ENABLE_VNI=OFF ENABLE_PUP=OFF ./platforms/ios/arm64/external.sh
   cmake \
      -DPLATFORM=ios \
      -DARCH=arm64 \
      -DBUILD_SHARED=OFF \
      -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
      -DENABLE_SERUM=OFF \
      -DENABLE_VNI=OFF \
      -DENABLE_PUP=OFF \
      -B build
   cmake --build build -- -j${NUM_PROCS}
   cd ..

   echo "$LIBDMDUTIL_EXPECTED_SHA" > cache.txt

   cd ..
fi

#
# build libserum
#

LIBSERUM_EXPECTED_SHA="${LIBSERUM_SHA}"
LIBSERUM_FOUND_SHA="$([ -f libserum/cache.txt ] && cat libserum/cache.txt || echo "")"

if [ "${LIBSERUM_EXPECTED_SHA}" != "${LIBSERUM_FOUND_SHA}" ]; then
   echo "Building libserum. Expected: ${LIBSERUM_EXPECTED_SHA}, Found: ${LIBSERUM_FOUND_SHA}"

   rm -rf libserum
   mkdir libserum
   cd libserum

   curl -sL https://github.com/PPUC/libserum/archive/${LIBSERUM_SHA}.tar.gz -o libserum-${LIBSERUM_SHA}.tar.gz
   tar xzf libserum-${LIBSERUM_SHA}.tar.gz
   mv libserum-${LIBSERUM_SHA} libserum
   cd libserum
   ./platforms/ios/arm64/external.sh
   cmake \
      -DPLATFORM=ios \
      -DARCH=arm64 \
      -DBUILD_SHARED=OFF \
      -DBUILD_STATIC=ON \
      -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
      -B build
   cmake --build build -- -j${NUM_PROCS}
   cd ..

   echo "$LIBSERUM_EXPECTED_SHA" > cache.txt

   cd ..
fi

#
# build libpupdmd
#

LIBPUPDMD_EXPECTED_SHA="${LIBPUPDMD_SHA}"
LIBPUPDMD_FOUND_SHA="$([ -f libpupdmd/cache.txt ] && cat libpupdmd/cache.txt || echo "")"

if [ "${LIBPUPDMD_EXPECTED_SHA}" != "${LIBPUPDMD_FOUND_SHA}" ]; then
   echo "Building libpupdmd. Expected: ${LIBPUPDMD_EXPECTED_SHA}, Found: ${LIBPUPDMD_FOUND_SHA}"

   rm -rf libpupdmd
   mkdir libpupdmd
   cd libpupdmd

   curl -sL https://github.com/PPUC/libpupdmd/archive/${LIBPUPDMD_SHA}.tar.gz -o libpupdmd-${LIBPUPDMD_SHA}.tar.gz
   tar xzf libpupdmd-${LIBPUPDMD_SHA}.tar.gz
   mv libpupdmd-${LIBPUPDMD_SHA} libpupdmd
   cd libpupdmd
   cmake \
      -DPLATFORM=ios \
      -DARCH=arm64 \
      -DBUILD_SHARED=OFF \
      -DBUILD_STATIC=ON \
      -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
      -B build
   cmake --build build -- -j${NUM_PROCS}
   cd ..

   echo "$LIBPUPDMD_EXPECTED_SHA" > cache.txt

   cd ..
fi

#
# build libvni
#

LIBVNI_EXPECTED_SHA="${LIBVNI_SHA}"
LIBVNI_FOUND_SHA="$([ -f libvni/cache.txt ] && cat libvni/cache.txt || echo "")"

if [ "${LIBVNI_EXPECTED_SHA}" != "${LIBVNI_FOUND_SHA}" ]; then
   echo "Building libvni. Expected: ${LIBVNI_EXPECTED_SHA}, Found: ${LIBVNI_FOUND_SHA}"

   rm -rf libvni
   mkdir libvni
   cd libvni

   curl -sL https://github.com/PPUC/libvni/archive/${LIBVNI_SHA}.tar.gz -o libvni-${LIBVNI_SHA}.tar.gz
   tar xzf libvni-${LIBVNI_SHA}.tar.gz
   mv libvni-${LIBVNI_SHA} libvni
   cd libvni
   ./platforms/ios/arm64/external.sh
   cmake \
      -DPLATFORM=ios \
      -DARCH=arm64 \
      -DBUILD_SHARED=OFF \
      -DBUILD_STATIC=ON \
      -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
      -B build
   cmake --build build -- -j${NUM_PROCS}
   cd ..

   echo "$LIBVNI_EXPECTED_SHA" > cache.txt

   cd ..
fi

#
# build libaltsound
#

LIBALTSOUND_EXPECTED_SHA="${LIBALTSOUND_SHA}"
LIBALTSOUND_FOUND_SHA="$([ -f libaltsound/cache.txt ] && cat libaltsound/cache.txt || echo "")"

if [ "${LIBALTSOUND_EXPECTED_SHA}" != "${LIBALTSOUND_FOUND_SHA}" ]; then
   echo "Building libaltsound. Expected: ${LIBALTSOUND_EXPECTED_SHA}, Found: ${LIBALTSOUND_FOUND_SHA}"

   rm -rf libaltsound
   mkdir libaltsound
   cd libaltsound

   curl -sL https://github.com/vpinball/libaltsound/archive/${LIBALTSOUND_SHA}.tar.gz -o libaltsound-${LIBALTSOUND_SHA}.tar.gz
   tar xzf libaltsound-${LIBALTSOUND_SHA}.tar.gz
   mv libaltsound-${LIBALTSOUND_SHA} libaltsound
   cd libaltsound
   cmake \
      -DPLATFORM=ios \
      -DARCH=arm64 \
      -DBUILD_SHARED=OFF \
      -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
      -B build
   cmake --build build -- -j${NUM_PROCS}
   cd ..

   echo "$LIBALTSOUND_EXPECTED_SHA" > cache.txt

   cd ..
fi

#
# build libdof
#

LIBDOF_EXPECTED_SHA="${LIBDOF_SHA}"
LIBDOF_FOUND_SHA="$([ -f libdof/cache.txt ] && cat libdof/cache.txt || echo "")"

if [ "${LIBDOF_EXPECTED_SHA}" != "${LIBDOF_FOUND_SHA}" ]; then
   echo "Building libdof. Expected: ${LIBDOF_EXPECTED_SHA}, Found: ${LIBDOF_FOUND_SHA}"

   rm -rf libdof
   mkdir libdof
   cd libdof

   curl -sL https://github.com/vpinball/libdof/archive/${LIBDOF_SHA}.tar.gz -o libdof-${LIBDOF_SHA}.tar.gz
   tar xzf libdof-${LIBDOF_SHA}.tar.gz
   mv libdof-${LIBDOF_SHA} libdof
   cd libdof
   ./platforms/ios/arm64/external.sh
   cmake \
      -DPLATFORM=ios \
      -DARCH=arm64 \
      -DBUILD_SHARED=OFF \
      -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
      -B build
   cmake --build build -- -j${NUM_PROCS}
   cd ..

   echo "$LIBDOF_EXPECTED_SHA" > cache.txt

   cd ..
fi

#
# build libwinevbs
#

LIBWINEVBS_EXPECTED_SHA="${LIBWINEVBS_SHA}"
LIBWINEVBS_FOUND_SHA="$([ -f libwinevbs/cache.txt ] && cat libwinevbs/cache.txt || echo "")"

if [ "${LIBWINEVBS_EXPECTED_SHA}" != "${LIBWINEVBS_FOUND_SHA}" ]; then
   echo "Building libwinevbs. Expected: ${LIBWINEVBS_EXPECTED_SHA}, Found: ${LIBWINEVBS_FOUND_SHA}"

   rm -rf libwinevbs
   mkdir libwinevbs
   cd libwinevbs

   curl -sL https://github.com/vpinball/libwinevbs/archive/${LIBWINEVBS_SHA}.tar.gz -o libwinevbs-${LIBWINEVBS_SHA}.tar.gz
   tar xzf libwinevbs-${LIBWINEVBS_SHA}.tar.gz
   mv libwinevbs-${LIBWINEVBS_SHA} libwinevbs
   cd libwinevbs
   cmake \
      -DPLATFORM=ios \
      -DARCH=arm64 \
      -DBUILD_SHARED=OFF \
      -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
      -B build
   cmake --build build -- -j${NUM_PROCS}
   cd ..

   echo "$LIBWINEVBS_EXPECTED_SHA" > cache.txt

   cd ..
fi

#
# build ffmpeg
#

FFMPEG_EXPECTED_SHA="${FFMPEG_SHA}"
FFMPEG_FOUND_SHA="$([ -f ffmpeg/cache.txt ] && cat ffmpeg/cache.txt || echo "")"

if [ "${FFMPEG_EXPECTED_SHA}" != "${FFMPEG_FOUND_SHA}" ]; then
   echo "Building ffmpeg. Expected: ${FFMPEG_EXPECTED_SHA}, Found: ${FFMPEG_FOUND_SHA}"

   rm -rf ffmpeg
   mkdir ffmpeg
   cd ffmpeg

   curl -sL https://github.com/FFmpeg/FFmpeg/archive/${FFMPEG_SHA}.tar.gz -o FFmpeg-${FFMPEG_SHA}.tar.gz
   tar xzf FFmpeg-${FFMPEG_SHA}.tar.gz
   mv FFmpeg-${FFMPEG_SHA} ffmpeg
   cd ffmpeg
   FFMPEG_IPHONEOS_SDK=$(xcrun --sdk iphoneos --show-sdk-path)
   FFMPEG_IPHONEOS_FLAGS="-isysroot ${FFMPEG_IPHONEOS_SDK} -miphoneos-version-min=16.0"
   ./configure \
      --enable-cross-compile \
      --enable-static \
      --disable-shared \
      --disable-programs \
      --disable-doc \
      --disable-avdevice \
      --disable-avfilter \
      --disable-audiotoolbox \
      --disable-securetransport \
      --arch=arm64 \
      --extra-cflags="${FFMPEG_IPHONEOS_FLAGS}" \
      --extra-ldflags="-Wl,-ld_classic ${FFMPEG_IPHONEOS_FLAGS}"
   make -j${NUM_PROCS}
   cd ..

   echo "$FFMPEG_EXPECTED_SHA" > cache.txt

   cd ..
fi

#
# build libzip
#

LIBZIP_EXPECTED_SHA="${LIBZIP_SHA}"
LIBZIP_FOUND_SHA="$([ -f libzip/cache.txt ] && cat libzip/cache.txt || echo "")"

if [ "${LIBZIP_EXPECTED_SHA}" != "${LIBZIP_FOUND_SHA}" ]; then
   echo "Building libzip. Expected: ${LIBZIP_EXPECTED_SHA}, Found: ${LIBZIP_FOUND_SHA}"

   rm -rf libzip
   mkdir libzip
   cd libzip

   curl -sL https://github.com/nih-at/libzip/archive/${LIBZIP_SHA}.tar.gz -o libzip-${LIBZIP_SHA}.tar.gz
   tar xzf libzip-${LIBZIP_SHA}.tar.gz
   mv libzip-${LIBZIP_SHA} libzip
   cd libzip
   cmake \
      -DBUILD_SHARED_LIBS=OFF \
      -DENABLE_ZSTD=OFF \
      -DENABLE_BZIP2=OFF \
      -DENABLE_LZMA=OFF \
      -DBUILD_TOOLS=OFF \
      -DBUILD_REGRESS=OFF \
      -DBUILD_OSSFUZZ=OFF \
      -DBUILD_EXAMPLES=OFF \
      -DBUILD_DOC=OFF \
      -DCMAKE_SYSTEM_NAME=iOS \
      -DCMAKE_OSX_DEPLOYMENT_TARGET=16.0 \
      -DCMAKE_OSX_ARCHITECTURES=arm64 \
      -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
      -B build
   cmake --build build -- -j${NUM_PROCS}
   cd ..

   echo "$LIBZIP_EXPECTED_SHA" > cache.txt

   cd ..
fi


#
# build zlib
#

ZLIB_EXPECTED_SHA="${ZLIB_SHA}_001"
ZLIB_FOUND_SHA="$([ -f zlib/cache.txt ] && cat zlib/cache.txt || echo "")"

if [ "${ZLIB_EXPECTED_SHA}" != "${ZLIB_FOUND_SHA}" ]; then
   echo "Building zlib. Expected: ${ZLIB_EXPECTED_SHA}, Found: ${ZLIB_FOUND_SHA}"

   rm -rf zlib
   mkdir zlib
   cd zlib

   curl -sL https://github.com/madler/zlib/archive/${ZLIB_SHA}.tar.gz -o zlib-${ZLIB_SHA}.tar.gz
   tar xzf zlib-${ZLIB_SHA}.tar.gz
   mv zlib-${ZLIB_SHA} zlib
   cd zlib
   cmake \
      -DBUILD_SHARED_LIBS=OFF \
      -DZLIB_BUILD_EXAMPLES=OFF \
      -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
      -DCMAKE_SYSTEM_NAME=iOS \
      -DCMAKE_OSX_DEPLOYMENT_TARGET=16.0 \
      -DCMAKE_OSX_ARCHITECTURES=arm64 \
      -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
      -B build
   cmake --build build -- -j${NUM_PROCS}
   cp build/zconf.h .
   cd ..

   echo "$ZLIB_EXPECTED_SHA" > cache.txt

   cd ..
fi

#
# build libogg, libopus, opusfile (Opus sounds)
#

OPUS_EXPECTED_SHA="${LIBOGG_SHA}-${LIBOPUS_SHA}-${OPUSFILE_SHA}_001"
OPUS_FOUND_SHA="$([ -f opus/cache.txt ] && cat opus/cache.txt || echo "")"

if [ "${OPUS_EXPECTED_SHA}" != "${OPUS_FOUND_SHA}" ]; then
   echo "Building opus. Expected: ${OPUS_EXPECTED_SHA}, Found: ${OPUS_FOUND_SHA}"

   rm -rf opus
   mkdir opus
   cd opus

   curl -sL https://github.com/xiph/ogg/archive/${LIBOGG_SHA}.tar.gz -o ogg-${LIBOGG_SHA}.tar.gz
   tar xzf ogg-${LIBOGG_SHA}.tar.gz
   mv ogg-${LIBOGG_SHA} ogg
   cd ogg
   cmake \
      -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
      -DCMAKE_SYSTEM_NAME=iOS \
      -DCMAKE_OSX_DEPLOYMENT_TARGET=16.0 \
      -DCMAKE_OSX_ARCHITECTURES=arm64 \
      -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
      -DBUILD_SHARED_LIBS=OFF \
      -DINSTALL_DOCS=OFF \
      -DBUILD_TESTING=OFF \
      -DCMAKE_INSTALL_PREFIX="$PWD/../install" \
      -B build
   cmake --build build -- -j${NUM_PROCS}
   cmake --install build
   cd ..

   curl -sL https://github.com/xiph/opus/archive/${LIBOPUS_SHA}.tar.gz -o opus-${LIBOPUS_SHA}.tar.gz
   tar xzf opus-${LIBOPUS_SHA}.tar.gz
   mv opus-${LIBOPUS_SHA} opus
   cd opus
   cmake \
      -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
      -DCMAKE_SYSTEM_NAME=iOS \
      -DCMAKE_OSX_DEPLOYMENT_TARGET=16.0 \
      -DCMAKE_OSX_ARCHITECTURES=arm64 \
      -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
      -DBUILD_SHARED_LIBS=OFF \
      -DOPUS_BUILD_PROGRAMS=OFF \
      -DOPUS_BUILD_TESTING=OFF \
      -DCMAKE_INSTALL_PREFIX="$PWD/../install" \
      -B build
   cmake --build build -- -j${NUM_PROCS}
   cmake --install build
   cd ..

   curl -sL https://github.com/xiph/opusfile/archive/${OPUSFILE_SHA}.tar.gz -o opusfile-${OPUSFILE_SHA}.tar.gz
   tar xzf opusfile-${OPUSFILE_SHA}.tar.gz
   mv opusfile-${OPUSFILE_SHA} opusfile
   cd opusfile
   cmake \
      -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
      -DCMAKE_SYSTEM_NAME=iOS \
      -DCMAKE_OSX_DEPLOYMENT_TARGET=16.0 \
      -DCMAKE_OSX_ARCHITECTURES=arm64 \
      -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
      -DBUILD_SHARED_LIBS=OFF \
      -DOP_DISABLE_HTTP=ON \
      -DOP_DISABLE_DOCS=ON \
      -DOP_DISABLE_EXAMPLES=ON \
      -DCMAKE_PREFIX_PATH="$PWD/../install" \
      -DCMAKE_INSTALL_PREFIX="$PWD/../install" \
      -B build
   cmake --build build -- -j${NUM_PROCS}
   cmake --install build
   cd ..

   echo "$OPUS_EXPECTED_SHA" > cache.txt

   cd ..
fi

#
# build libmysofa
#

LIBMYSOFA_EXPECTED_SHA="${LIBMYSOFA_SHA}-${ZLIB_SHA}_001"
LIBMYSOFA_FOUND_SHA="$([ -f libmysofa/cache.txt ] && cat libmysofa/cache.txt || echo "")"

if [ "${LIBMYSOFA_EXPECTED_SHA}" != "${LIBMYSOFA_FOUND_SHA}" ]; then
   echo "Building libmysofa. Expected: ${LIBMYSOFA_EXPECTED_SHA}, Found: ${LIBMYSOFA_FOUND_SHA}"

   rm -rf libmysofa
   mkdir libmysofa
   cd libmysofa

   curl -sL https://github.com/hoene/libmysofa/archive/${LIBMYSOFA_SHA}.tar.gz -o libmysofa-${LIBMYSOFA_SHA}.tar.gz
   tar xzf libmysofa-${LIBMYSOFA_SHA}.tar.gz
   mv libmysofa-${LIBMYSOFA_SHA} libmysofa
   cd libmysofa
   cmake \
      -DBUILD_SHARED_LIBS=OFF \
      -DBUILD_STATIC_LIBS=ON \
      -DBUILD_TESTS=OFF \
      -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
      -DCMAKE_SYSTEM_NAME=iOS \
      -DCMAKE_OSX_DEPLOYMENT_TARGET=16.0 \
      -DCMAKE_OSX_ARCHITECTURES=arm64 \
      -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
      -DZLIB_INCLUDE_DIR="$PWD/../../zlib/zlib" \
      -DZLIB_LIBRARY="$PWD/../../zlib/zlib/build/libz.a" \
      -B build
   cmake --build build -- -j${NUM_PROCS}
   cp build/src/mysofa_export.h src/hrtf
   cd ..

   echo "$LIBMYSOFA_EXPECTED_SHA" > cache.txt

   cd ..
fi

#
# build libspatialaudio
#

LIBSPATIALAUDIO_EXPECTED_SHA="${LIBSPATIALAUDIO_SHA}-${LIBMYSOFA_SHA}_001"
LIBSPATIALAUDIO_FOUND_SHA="$([ -f libspatialaudio/cache.txt ] && cat libspatialaudio/cache.txt || echo "")"

if [ "${LIBSPATIALAUDIO_EXPECTED_SHA}" != "${LIBSPATIALAUDIO_FOUND_SHA}" ]; then
   echo "Building libspatialaudio. Expected: ${LIBSPATIALAUDIO_EXPECTED_SHA}, Found: ${LIBSPATIALAUDIO_FOUND_SHA}"

   rm -rf libspatialaudio
   mkdir libspatialaudio
   cd libspatialaudio

   curl -sL https://github.com/videolan/libspatialaudio/archive/${LIBSPATIALAUDIO_SHA}.tar.gz -o libspatialaudio-${LIBSPATIALAUDIO_SHA}.tar.gz
   tar xzf libspatialaudio-${LIBSPATIALAUDIO_SHA}.tar.gz
   mv libspatialaudio-${LIBSPATIALAUDIO_SHA} libspatialaudio
   cd libspatialaudio
   cmake \
      -DBUILD_SHARED_LIBS=OFF \
      -DBUILD_TESTING=OFF \
      -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
      -DCMAKE_SYSTEM_NAME=iOS \
      -DCMAKE_OSX_DEPLOYMENT_TARGET=16.0 \
      -DCMAKE_OSX_ARCHITECTURES=arm64 \
      -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
      -DMYSOFA_INCLUDE_DIRS="$PWD/../../libmysofa/libmysofa/src/hrtf" \
      -DMYSOFA_LIBRARIES="$PWD/../../libmysofa/libmysofa/build/src/libmysofa.a" \
      -B build
   cmake --build build -- -j${NUM_PROCS}
   cd ..

   echo "$LIBSPATIALAUDIO_EXPECTED_SHA" > cache.txt

   cd ..
fi

#
# copy libraries
#

cp SDL3/SDL/build/libSDL3.a ../../../third-party/build-libs/ios-arm64
cp -r SDL3/SDL/include/SDL3 ../../../third-party/include/

cp SDL3/SDL_image/build/libSDL3_image.a ../../../third-party/build-libs/ios-arm64
cp -r SDL3/SDL_image/include/SDL3_image ../../../third-party/include/

cp SDL3/SDL_ttf/build/libSDL3_ttf.a ../../../third-party/build-libs/ios-arm64
cp -r SDL3/SDL_ttf/include/SDL3_ttf ../../../third-party/include/
cp SDL3/SDL_ttf/build/external/freetype/libfreetype.a ../../../third-party/build-libs/ios-arm64
cp SDL3/SDL_ttf/build/external/harfbuzz/libharfbuzz.a ../../../third-party/build-libs/ios-arm64

cp freeimage/freeimage/build/libfreeimage.a ../../../third-party/build-libs/ios-arm64
cp freeimage/freeimage/Source/FreeImage.h ../../../third-party/include

cp bgfx/bgfx.cmake/build/cmake/bgfx/libbgfx.a ../../../third-party/build-libs/ios-arm64
cp -r bgfx/bgfx.cmake/bgfx/include/bgfx ../../../third-party/include/
cp bgfx/bgfx.cmake/build/cmake/bimg/libbimg.a ../../../third-party/build-libs/ios-arm64
cp bgfx/bgfx.cmake/build/cmake/bimg/libbimg_encode.a ../../../third-party/build-libs/ios-arm64
cp -r bgfx/bgfx.cmake/bimg/include/bimg ../../../third-party/include/
cp bgfx/bgfx.cmake/build/cmake/bx/libbx.a ../../../third-party/build-libs/ios-arm64

# shaderc is a host tool (compiles the embedded BGFX shaders at build time):
# place it under the host platform's tools dir so CMake can find it when cross-building.
case "$(uname -s)-$(uname -m)" in
   Darwin-arm64)   HOST_TOOLS=macos-arm64 ;;
   Darwin-x86_64)  HOST_TOOLS=macos-x64 ;;
   Linux-x86_64)   HOST_TOOLS=linux-x64 ;;
   Linux-aarch64)  HOST_TOOLS=linux-aarch64 ;;
esac
mkdir -p "../../../third-party/tools/${HOST_TOOLS}"
cp bgfx/bgfx.cmake/build-tools/cmake/bgfx/shaderc "../../../third-party/tools/${HOST_TOOLS}/"
cp -r bgfx/bgfx.cmake/bx/include/bx ../../../third-party/include/

cp pinmame/pinmame/build/libpinmame.a ../../../third-party/build-libs/ios-arm64
mkdir -p ../../../third-party/include/pinmame
cp pinmame/pinmame/src/libpinmame/libpinmame.h ../../../third-party/include/pinmame
cp pinmame/pinmame/src/libpinmame/PinMAMEPlugin.h ../../../third-party/include/pinmame

mkdir -p ../../../third-party/memmaps
cp -r pinmame-data/memmaps/index.json pinmame-data/memmaps/maps pinmame-data/memmaps/platforms ../../../third-party/memmaps/
mkdir -p ../../../third-party/gamedefs
cp -r pinmame-data/gamedefs/catalog pinmame-data/gamedefs/machines pinmame-data/gamedefs/controllers ../../../third-party/gamedefs/

cp libdmdutil/libdmdutil/build/libdmdutil.a ../../../third-party/build-libs/ios-arm64
cp -r libdmdutil/libdmdutil/include/DMDUtil ../../../third-party/include/
cp libdmdutil/libdmdutil/third-party/build-libs/ios/arm64/libzedmd.a ../../../third-party/build-libs/ios-arm64
cp libdmdutil/libdmdutil/third-party/include/ZeDMD.h ../../../third-party/include
cp libserum/libserum/build/libserum.a ../../../third-party/build-libs/ios-arm64
cp libserum/libserum/src/serum.h ../../../third-party/include
cp libserum/libserum/src/serum-decode.h ../../../third-party/include
cp libpupdmd/libpupdmd/build/libpupdmd.a ../../../third-party/build-libs/ios-arm64
cp libpupdmd/libpupdmd/src/pupdmd.h ../../../third-party/include
cp libdmdutil/libdmdutil/third-party/build-libs/ios/arm64/libsockpp.a ../../../third-party/build-libs/ios-arm64
cp libvni/libvni/build/libvni.a ../../../third-party/build-libs/ios-arm64
cp libvni/libvni/src/vni.h ../../../third-party/include

cp libaltsound/libaltsound/build/libaltsound.a ../../../third-party/build-libs/ios-arm64
cp libaltsound/libaltsound/src/altsound.h ../../../third-party/include

cp libwinevbs/libwinevbs/build/libwinevbs.a ../../../third-party/build-libs/ios-arm64
mkdir -p ../../../third-party/include/libwinevbs/wine/include
mkdir -p ../../../third-party/include/libwinevbs/atl/include
cp libwinevbs/libwinevbs/include/libwinevbs.h ../../../third-party/include/libwinevbs/
cp -r libwinevbs/libwinevbs/wine/include/* ../../../third-party/include/libwinevbs/wine/include/
cp -r libwinevbs/libwinevbs/atl/include/* ../../../third-party/include/libwinevbs/atl/include/

cp libdof/libdof/build/libdof.a ../../../third-party/build-libs/ios-arm64
cp -r libdof/libdof/include/DOF ../../../third-party/include/

for LIB in libavcodec libavformat libavutil libswresample libswscale; do
   cp -a ffmpeg/ffmpeg/${LIB}/${LIB}.a ../../../third-party/build-libs/ios-arm64
   mkdir -p ../../../third-party/include/${LIB}
   cp ffmpeg/ffmpeg/${LIB}/*.h ../../../third-party/include/${LIB}
done

cp libzip/libzip/build/lib/libzip.a ../../../third-party/build-libs/ios-arm64
cp libzip/libzip/build/zipconf.h ../../../third-party/include
cp libzip/libzip/lib/zip.h ../../../third-party/include

cp zlib/zlib/build/libz.a ../../../third-party/build-libs/ios-arm64

cp libmysofa/libmysofa/build/src/libmysofa.a ../../../third-party/build-libs/ios-arm64
cp libmysofa/libmysofa/src/hrtf/mysofa.h ../../../third-party/include
cp libmysofa/libmysofa/src/hrtf/mysofa_export.h ../../../third-party/include
cp opus/install/lib/libogg.a ../../../third-party/build-libs/ios-arm64
cp opus/install/lib/libopus.a ../../../third-party/build-libs/ios-arm64
cp opus/install/lib/libopusfile.a ../../../third-party/build-libs/ios-arm64
mkdir -p ../../../third-party/include/ogg
cp opus/install/include/ogg/*.h ../../../third-party/include/ogg
cp opus/install/include/opus/*.h ../../../third-party/include

cp libspatialaudio/libspatialaudio/build/libspatialaudio.a ../../../third-party/build-libs/ios-arm64
mkdir -p ../../../third-party/include/spatialaudio
cp -r libspatialaudio/libspatialaudio/include/*.h libspatialaudio/libspatialaudio/include/{adm,dsp,hrtf,kiss_fft} ../../../third-party/include/spatialaudio
cp libspatialaudio/libspatialaudio/build/include/SpatialaudioConfig.h ../../../third-party/include/spatialaudio
