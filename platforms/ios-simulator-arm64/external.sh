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
echo "  LIBDMDUTIL_SHA: ${LIBDMDUTIL_SHA}"
echo "  LIBALTSOUND_SHA: ${LIBALTSOUND_SHA}"
echo "  LIBDOF_SHA: ${LIBDOF_SHA}"
echo "  LIBWINEVBS_SHA: ${LIBWINEVBS_SHA}"
echo "  FFMPEG_SHA: ${FFMPEG_SHA}"
echo "  LIBZIP_SHA: ${LIBZIP_SHA}"
echo "  LIBMYSOFA_SHA: ${LIBMYSOFA_SHA}"
echo "  LIBSPATIALAUDIO_SHA: ${LIBSPATIALAUDIO_SHA}"
echo "  ZLIB_SHA: ${ZLIB_SHA}"
echo ""

NUM_PROCS=$(sysctl -n hw.ncpu)

mkdir -p "external/ios-simulator-arm64/${BUILD_TYPE}"
cd "external/ios-simulator-arm64/${BUILD_TYPE}"

#
# build SDL3, SDL3_image, SDL3_ttf#

SDL3_EXPECTED_SHA="${SDL_SHA}-${SDL_IMAGE_SHA}-${SDL_TTF_SHA}_002"
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
   cmake \
      -DSDL_SHARED=OFF \
      -DSDL_STATIC=ON \
      -DSDL_TEST_LIBRARY=OFF \
      -DSDL_OPENGLES=OFF \
      -DSDL_CAMERA=OFF \
      -DCMAKE_SYSTEM_NAME=iOS \
      -DCMAKE_OSX_SYSROOT=iphonesimulator \
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
      -DCMAKE_OSX_SYSROOT=iphonesimulator \
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
      -DCMAKE_OSX_SYSROOT=iphonesimulator \
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
      -DPLATFORM=ios-simulator \
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

BGFX_EXPECTED_SHA="${BGFX_CMAKE_VERSION}-${BGFX_PATCH_SHA}"
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
      -DCMAKE_OSX_SYSROOT=iphonesimulator \
      -DCMAKE_OSX_ARCHITECTURES=arm64 \
      -DCMAKE_OSX_DEPLOYMENT_TARGET=16.0 \
      -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
      -B build
   cmake --build build -- -j${NUM_PROCS}
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
      -DPLATFORM=ios-simulator \
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
   ./platforms/ios-simulator/arm64/external.sh
   cmake \
      -DPLATFORM=ios-simulator \
      -DARCH=arm64 \
      -DBUILD_SHARED=OFF \
      -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
      -B build
   cmake --build build -- -j${NUM_PROCS}
   cd ..

   echo "$LIBDMDUTIL_EXPECTED_SHA" > cache.txt

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
      -DPLATFORM=ios-simulator \
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
   ./platforms/ios-simulator/arm64/external.sh
   cmake \
      -DPLATFORM=ios-simulator \
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
   FFMPEG_IPHONESIMULATOR_SDK=$(xcrun --sdk iphonesimulator --show-sdk-path)
   FFMPEG_IPHONESIMULATOR_FLAGS="-isysroot ${FFMPEG_IPHONESIMULATOR_SDK} -miphonesimulator-version-min=16.0"
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
      --extra-cflags="${FFMPEG_IPHONESIMULATOR_FLAGS}" \
      --extra-ldflags="-Wl,-ld_classic ${FFMPEG_IPHONESIMULATOR_FLAGS}"
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
      -DCMAKE_OSX_SYSROOT=iphonesimulator \
      -DCMAKE_OSX_DEPLOYMENT_TARGET=16.0 \
      -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
      -B build
   cmake --build build -- -j${NUM_PROCS}
   cd ..

   echo "$LIBZIP_EXPECTED_SHA" > cache.txt

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
      -DPLATFORM=ios-simulator \
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
# build zlib (static library needed by libmysofa)
#

ZLIB_EXPECTED_SHA="${ZLIB_SHA}"
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
      -DCMAKE_SYSTEM_NAME=iOS \
      -DCMAKE_OSX_SYSROOT=iphonesimulator \
      -DCMAKE_OSX_DEPLOYMENT_TARGET=16.0 \
      -DCMAKE_OSX_ARCHITECTURES=arm64 \
      -DBUILD_SHARED_LIBS=OFF \
      -DZLIB_BUILD_EXAMPLES=OFF \
      -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
      -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
      -DCMAKE_INSTALL_PREFIX=install \
      -B build
   cmake --build build -- -j${NUM_PROCS}
   cmake --install build
   cd ..

   echo "$ZLIB_EXPECTED_SHA" > cache.txt

   cd ..
fi

#
# build libmysofa (static library needed by libspatialaudio for SOFA HRTF support)
#

LIBMYSOFA_EXPECTED_SHA="${LIBMYSOFA_SHA}-${ZLIB_SHA}"
LIBMYSOFA_FOUND_SHA="$([ -f libmysofa/cache.txt ] && cat libmysofa/cache.txt || echo "")"

if [ "${LIBMYSOFA_EXPECTED_SHA}" != "${LIBMYSOFA_FOUND_SHA}" ]; then
   echo "Building libmysofa. Expected: ${LIBMYSOFA_EXPECTED_SHA}, Found: ${LIBMYSOFA_FOUND_SHA}"

   rm -rf libmysofa
   mkdir libmysofa
   cd libmysofa

   curl -sL https://github.com/hoene/libmysofa/archive/${LIBMYSOFA_SHA}.tar.gz -o libmysofa-${LIBMYSOFA_SHA}.tar.gz
   # provide share/default.sofa (a symlink in the archive) as a real file
   tar xzf libmysofa-${LIBMYSOFA_SHA}.tar.gz --exclude='*.sofa'
   tar xzf libmysofa-${LIBMYSOFA_SHA}.tar.gz libmysofa-${LIBMYSOFA_SHA}/share/MIT_KEMAR_normal_pinna.sofa
   mv libmysofa-${LIBMYSOFA_SHA} libmysofa
   cp libmysofa/share/MIT_KEMAR_normal_pinna.sofa libmysofa/share/default.sofa
   cd libmysofa
   ZLIB_LIB="$(find ../../zlib/zlib/install/lib -type f \( -name 'libz.a' -o -name 'libzlibstatic.a' \) | head -n1)"
   cmake \
      -DCMAKE_SYSTEM_NAME=iOS \
      -DCMAKE_OSX_SYSROOT=iphonesimulator \
      -DCMAKE_OSX_DEPLOYMENT_TARGET=16.0 \
      -DCMAKE_OSX_ARCHITECTURES=arm64 \
      -DBUILD_SHARED_LIBS=OFF \
      -DBUILD_STATIC_LIBS=ON \
      -DBUILD_TESTS=OFF \
      -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
      -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
      -DCMAKE_INSTALL_PREFIX=install \
      -DZLIB_INCLUDE_DIR="$(cd ../../zlib/zlib/install/include && pwd)" \
      -DZLIB_LIBRARY="$(cd "$(dirname "${ZLIB_LIB}")" && pwd)/$(basename "${ZLIB_LIB}")" \
      -B build
   cmake --build build -- -j${NUM_PROCS}
   cmake --install build
   cd ..

   echo "$LIBMYSOFA_EXPECTED_SHA" > cache.txt

   cd ..
fi

#
# build libspatialaudio (static library)
#

LIBSPATIALAUDIO_EXPECTED_SHA="${LIBSPATIALAUDIO_SHA}-${LIBMYSOFA_SHA}"
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
      -DCMAKE_SYSTEM_NAME=iOS \
      -DCMAKE_OSX_SYSROOT=iphonesimulator \
      -DCMAKE_OSX_DEPLOYMENT_TARGET=16.0 \
      -DCMAKE_OSX_ARCHITECTURES=arm64 \
      -DBUILD_SHARED_LIBS=OFF \
      -DBUILD_TESTING=OFF \
      -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
      -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
      -DCMAKE_INSTALL_PREFIX=install \
      -DMYSOFA_INCLUDE_DIRS="$(cd ../libmysofa/libmysofa/install/include && pwd)" \
      -DMYSOFA_LIBRARIES="$(cd ../libmysofa/libmysofa/install/lib && pwd)/libmysofa.a" \
      -B build
   cmake --build build -- -j${NUM_PROCS}
   cmake --install build
   cd ..

   echo "$LIBSPATIALAUDIO_EXPECTED_SHA" > cache.txt

   cd ..
fi
#
# copy libraries
#

cp SDL3/SDL/build/libSDL3.a ../../../third-party/build-libs/ios-simulator-arm64
cp -r SDL3/SDL/include/SDL3 ../../../third-party/include/

cp SDL3/SDL_image/build/libSDL3_image.a ../../../third-party/build-libs/ios-simulator-arm64
cp -r SDL3/SDL_image/include/SDL3_image ../../../third-party/include/

cp SDL3/SDL_ttf/build/libSDL3_ttf.a ../../../third-party/build-libs/ios-simulator-arm64
cp -r SDL3/SDL_ttf/include/SDL3_ttf ../../../third-party/include/
cp SDL3/SDL_ttf/build/external/freetype/libfreetype.a ../../../third-party/build-libs/ios-simulator-arm64
cp SDL3/SDL_ttf/build/external/harfbuzz/libharfbuzz.a ../../../third-party/build-libs/ios-simulator-arm64

cp freeimage/freeimage/build/libfreeimage.a ../../../third-party/build-libs/ios-simulator-arm64
cp freeimage/freeimage/Source/FreeImage.h ../../../third-party/include

cp bgfx/bgfx.cmake/build/cmake/bgfx/libbgfx.a ../../../third-party/build-libs/ios-simulator-arm64
cp -r bgfx/bgfx.cmake/bgfx/include/bgfx ../../../third-party/include/
cp bgfx/bgfx.cmake/build/cmake/bimg/libbimg.a ../../../third-party/build-libs/ios-simulator-arm64
cp bgfx/bgfx.cmake/build/cmake/bimg/libbimg_encode.a ../../../third-party/build-libs/ios-simulator-arm64
cp -r bgfx/bgfx.cmake/bimg/include/bimg ../../../third-party/include/
cp bgfx/bgfx.cmake/build/cmake/bx/libbx.a ../../../third-party/build-libs/ios-simulator-arm64
cp -r bgfx/bgfx.cmake/bx/include/bx ../../../third-party/include/

cp pinmame/pinmame/build/libpinmame.a ../../../third-party/build-libs/ios-simulator-arm64
mkdir -p ../../../third-party/include/pinmame
cp pinmame/pinmame/src/libpinmame/libpinmame.h ../../../third-party/include/pinmame
cp pinmame/pinmame/src/libpinmame/PinMAMEPlugin.h ../../../third-party/include/pinmame

cp libdmdutil/libdmdutil/build/libdmdutil.a ../../../third-party/build-libs/ios-simulator-arm64
cp -r libdmdutil/libdmdutil/include/DMDUtil ../../../third-party/include/
cp libdmdutil/libdmdutil/third-party/build-libs/ios-simulator/arm64/libzedmd.a ../../../third-party/build-libs/ios-simulator-arm64
cp libdmdutil/libdmdutil/third-party/include/ZeDMD.h ../../../third-party/include
cp libdmdutil/libdmdutil/third-party/build-libs/ios-simulator/arm64/libserum.a ../../../third-party/build-libs/ios-simulator-arm64
cp libdmdutil/libdmdutil/third-party/include/serum.h ../../../third-party/include
cp libdmdutil/libdmdutil/third-party/include/serum-decode.h ../../../third-party/include
cp libdmdutil/libdmdutil/third-party/build-libs/ios-simulator/arm64/libpupdmd.a ../../../third-party/build-libs/ios-simulator-arm64
cp libdmdutil/libdmdutil/third-party/include/pupdmd.h ../../../third-party/include
cp libdmdutil/libdmdutil/third-party/build-libs/ios-simulator/arm64/libsockpp.a ../../../third-party/build-libs/ios-simulator-arm64
cp libdmdutil/libdmdutil/third-party/build-libs/ios-simulator/arm64/libvni.a ../../../third-party/build-libs/ios-simulator-arm64
cp libdmdutil/libdmdutil/third-party/include/vni.h ../../../third-party/include

cp libaltsound/libaltsound/build/libaltsound.a ../../../third-party/build-libs/ios-simulator-arm64
cp libaltsound/libaltsound/src/altsound.h ../../../third-party/include

cp libdof/libdof/build/libdof.a ../../../third-party/build-libs/ios-simulator-arm64
cp -r libdof/libdof/include/DOF ../../../third-party/include/

cp libwinevbs/libwinevbs/build/libwinevbs.a ../../../third-party/build-libs/ios-simulator-arm64
mkdir -p ../../../third-party/include/libwinevbs/wine/include
mkdir -p ../../../third-party/include/libwinevbs/atl/include
cp libwinevbs/libwinevbs/include/libwinevbs.h ../../../third-party/include/libwinevbs/
cp -r libwinevbs/libwinevbs/wine/include/* ../../../third-party/include/libwinevbs/wine/include/
cp -r libwinevbs/libwinevbs/atl/include/* ../../../third-party/include/libwinevbs/atl/include/

for LIB in libavcodec libavformat libavutil libswresample libswscale; do
   cp -a ffmpeg/ffmpeg/${LIB}/${LIB}.a ../../../third-party/build-libs/ios-simulator-arm64
   mkdir -p ../../../third-party/include/${LIB}
   cp ffmpeg/ffmpeg/${LIB}/*.h ../../../third-party/include/${LIB}
done

cp libzip/libzip/build/lib/libzip.a ../../../third-party/build-libs/ios-simulator-arm64
cp libzip/libzip/build/zipconf.h ../../../third-party/include
cp libzip/libzip/lib/zip.h ../../../third-party/include

cp "$(find zlib/zlib/install/lib -type f \( -name 'libz.a' -o -name 'libzlibstatic.a' \) | head -n1)" ../../../third-party/build-libs/ios-simulator-arm64/libz.a

cp libmysofa/libmysofa/install/lib/libmysofa.a ../../../third-party/build-libs/ios-simulator-arm64
cp libmysofa/libmysofa/install/include/mysofa.h ../../../third-party/include/
cp libmysofa/libmysofa/install/include/mysofa_export.h ../../../third-party/include/

cp libspatialaudio/libspatialaudio/build/libspatialaudio.a ../../../third-party/build-libs/ios-simulator-arm64
cp -r libspatialaudio/libspatialaudio/install/include/spatialaudio ../../../third-party/include/
