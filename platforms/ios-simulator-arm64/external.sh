#!/bin/bash

set -e

source ./platforms/config.sh

echo "Building external libraries..."
echo "  SDL_SHA: ${SDL_SHA}"
echo "  SDL_IMAGE_SHA: ${SDL_IMAGE_SHA}"
echo "  SDL_TTF_SHA: ${SDL_TTF_SHA}"
echo "  SDL_MIXER_SHA: ${SDL_MIXER_SHA}"
echo "  FREEIMAGE_SHA: ${FREEIMAGE_SHA}"
echo "  BGFX_CMAKE_VERSION: ${BGFX_CMAKE_VERSION}"
echo "  BGFX_PATCH_SHA: ${BGFX_PATCH_SHA}"
echo "  PINMAME_SHA: ${PINMAME_SHA}"
echo "  LIBDMDUTIL_SHA: ${LIBDMDUTIL_SHA}"
echo "  LIBALTSOUND_SHA: ${LIBALTSOUND_SHA}"
echo "  LIBDOF_SHA: ${LIBDOF_SHA}"
echo "  FFMPEG_SHA: ${FFMPEG_SHA}"
echo ""

NUM_PROCS=$(sysctl -n hw.ncpu)

mkdir -p "external/ios-simulator-arm64/${BUILD_TYPE}"
cd "external/ios-simulator-arm64/${BUILD_TYPE}"

#
# build SDL3, SDL3_image, SDL3_ttf, SDL3_mixer
#

SDL3_EXPECTED_SHA="${SDL_SHA}-${SDL_IMAGE_SHA}-${SDL_TTF_SHA}-${SDL_MIXER_SHA}"
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

   curl -sL https://github.com/libsdl-org/SDL_mixer/archive/${SDL_MIXER_SHA}.tar.gz -o SDL_mixer-${SDL_MIXER_SHA}.tar.gz
   tar xzf SDL_mixer-${SDL_MIXER_SHA}.tar.gz
   mv SDL_mixer-${SDL_MIXER_SHA} SDL_mixer
   cd SDL_mixer
   ./external/download.sh
   cmake \
      -DBUILD_SHARED_LIBS=OFF \
      -DSDLMIXER_SAMPLES=OFF \
      -DSDLMIXER_VENDORED=ON \
      -DSDL3_DIR=../SDL/build \
      -DCMAKE_SYSTEM_NAME=iOS \
      -DCMAKE_OSX_SYSROOT=iphonesimulator \
      -DCMAKE_OSX_DEPLOYMENT_TARGET=16.0 \
      -DCMAKE_OSX_ARCHITECTURES=arm64 \
      -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
      -B build
   cmake --build build -- -j${NUM_PROCS}
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

   curl -sL https://github.com/vbousquet/pinmame/archive/${PINMAME_SHA}.zip -o pinmame-${PINMAME_SHA}.zip
   unzip pinmame-${PINMAME_SHA}.zip
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
   ./platforms/ios-simulator/arm64/external.sh
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

   curl -sL https://github.com/jsm174/libdof/archive/${LIBDOF_SHA}.tar.gz -o libdof-${LIBDOF_SHA}.tar.gz
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
# bass
#

BASS_EXPECTED_SHA="bass24"
BASS_FOUND_SHA="$([ -f bass/cache.txt ] && cat bass/cache.txt || echo "")"

if [ "${BASS_EXPECTED_SHA}" != "${BASS_FOUND_SHA}" ]; then
   echo "Fetching bass. Expected: ${BASS_EXPECTED_SHA}, Found: ${BASS_FOUND_SHA}"

   rm -rf bass
   mkdir bass
   cd bass

   curl -sL https://www.un4seen.com/files/bass24-ios.zip -o bass.zip
   unzip bass.zip
   lipo bass.xcframework/ios-arm64_i386_x86_64-simulator/bass.framework/bass -extract arm64 -output libbass.dylib
   install_name_tool -id @rpath/libbass.dylib libbass.dylib
   codesign --force --sign - libbass.dylib
   
   echo "$BASS_EXPECTED_SHA" > cache.txt

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

cp SDL3/SDL_mixer/build/libSDL3_mixer.a ../../../third-party/build-libs/ios-simulator-arm64
cp -r SDL3/SDL_mixer/include/SDL3_mixer ../../../third-party/include/

cp freeimage/freeimage/build/libfreeimage.a ../../../third-party/build-libs/ios-simulator-arm64
cp freeimage/freeimage/Source/FreeImage.h ../../../third-party/include

cp bgfx/bgfx.cmake/build/cmake/bgfx/libbgfx.a ../../../third-party/build-libs/ios-simulator-arm64
cp -r bgfx/bgfx.cmake/bgfx/include/bgfx ../../../third-party/include/
cp bgfx/bgfx.cmake/build/cmake/bimg/libbimg.a ../../../third-party/build-libs/ios-simulator-arm64
cp bgfx/bgfx.cmake/build/cmake/bimg/libbimg_decode.a ../../../third-party/build-libs/ios-simulator-arm64
cp bgfx/bgfx.cmake/build/cmake/bimg/libbimg_encode.a ../../../third-party/build-libs/ios-simulator-arm64
cp -r bgfx/bgfx.cmake/bimg/include/bimg ../../../third-party/include/
cp bgfx/bgfx.cmake/build/cmake/bx/libbx.a ../../../third-party/build-libs/ios-simulator-arm64
cp -r bgfx/bgfx.cmake/bx/include/bx ../../../third-party/include/

cp pinmame/pinmame/build/libpinmame.a ../../../third-party/build-libs/ios-simulator-arm64
cp pinmame/pinmame/src/libpinmame/libpinmame.h ../../../third-party/include
cp pinmame/pinmame/src/libpinmame/pinmamedef.h ../../../third-party/include

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

cp libaltsound/libaltsound/build/libaltsound.a ../../../third-party/build-libs/ios-simulator-arm64
cp -r libaltsound/libaltsound/src/altsound.h ../../../third-party/include/

cp libdof/libdof/build/libdof.a ../../../third-party/build-libs/ios-simulator-arm64
cp -r libdof/libdof/include/DOF ../../../third-party/include/

for LIB in libavcodec libavdevice libavfilter libavformat libavutil libswresample libswscale; do
   cp -a ffmpeg/ffmpeg/${LIB}/${LIB}.a ../../../third-party/build-libs/ios-simulator-arm64
   mkdir -p ../../../third-party/include/${LIB}
   cp ffmpeg/ffmpeg/${LIB}/*.h ../../../third-party/include/${LIB}
done

cp bass/libbass.dylib ../../../third-party/runtime-libs/ios-simulator-arm64