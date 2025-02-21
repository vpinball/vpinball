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
echo "  OPENXR_SHA: ${OPENXR_SHA}"
echo "  LIBDMDUTIL_SHA: ${LIBDMDUTIL_SHA}"
echo "  LIBALTSOUND_SHA: ${LIBALTSOUND_SHA}"
echo "  LIBDOF_SHA: ${LIBDOF_SHA}"
echo "  FFMPEG_SHA: ${FFMPEG_SHA}"
echo ""

if [[ $(uname) == "Linux" ]]; then
   NUM_PROCS=$(nproc)
elif [[ $(uname) == "Darwin" ]]; then
   NUM_PROCS=$(sysctl -n hw.ncpu)
else
   NUM_PROCS=1
fi

mkdir -p "external/android-arm64-v8a/${BUILD_TYPE}"
cd "external/android-arm64-v8a/${BUILD_TYPE}"

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
      -DSDL_SHARED=ON \
      -DSDL_STATIC=OFF \
      -DSDL_TEST_LIBRARY=OFF \
      -DSDL_DISABLE_ANDROID_JAR=OFF \
      -DSDL_CAMERA=OFF \
      -DCMAKE_SYSTEM_NAME=Android \
      -DCMAKE_SYSTEM_VERSION=30 \
      -DCMAKE_ANDROID_ARCH_ABI=arm64-v8a \
      -DANDROID_NDK=${ANDROID_NDK_HOME} \
      -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
      -B build
   cmake --build build -- -j${NUM_PROCS}
   rm build/SDL3jarTargets.cmake
   cd ..

   curl -sL https://github.com/libsdl-org/SDL_image/archive/${SDL_IMAGE_SHA}.tar.gz -o SDL_image-${SDL_IMAGE_SHA}.tar.gz
   tar xzf SDL_image-${SDL_IMAGE_SHA}.tar.gz
   mv SDL_image-${SDL_IMAGE_SHA} SDL_image
   cd SDL_image
   ./external/download.sh
   cmake \
      -DBUILD_SHARED_LIBS=ON \
      -DSDLIMAGE_SAMPLES=OFF \
      -DSDLIMAGE_DEPS_SHARED=ON \
      -DSDLIMAGE_VENDORED=ON \
      -DSDLIMAGE_AVIF=OFF \
      -DSDLIMAGE_WEBP=OFF \
      -DSDL3_DIR=../SDL/build \
      -DCMAKE_SYSTEM_NAME=Android \
      -DCMAKE_SYSTEM_VERSION=30 \
      -DCMAKE_ANDROID_ARCH_ABI=arm64-v8a \
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
      -DBUILD_SHARED_LIBS=ON \
      -DSDLTTF_SAMPLES=OFF \
      -DSDLTTF_VENDORED=ON \
      -DSDLTTF_HARFBUZZ=ON \
      -DSDL3_DIR=../SDL/build \
      -DCMAKE_SYSTEM_NAME=Android \
      -DCMAKE_SYSTEM_VERSION=30 \
      -DCMAKE_ANDROID_ARCH_ABI=arm64-v8a \
      -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
      -B build
   cmake --build build -- -j${NUM_PROCS}
   cd ..

   curl -sL https://github.com/libsdl-org/SDL_mixer/archive/${SDL_MIXER_SHA}.tar.gz -o SDL_mixer-${SDL_MIXER_SHA}.tar.gz
   tar xzf SDL_mixer-${SDL_MIXER_SHA}.tar.gz
   mv SDL_mixer-${SDL_MIXER_SHA} SDL_mixer
   cd SDL_mixer
   ./external/download.sh
   cmake \
      -DBUILD_SHARED_LIBS=ON \
      -DSDLMIXER_SAMPLES=OFF \
      -DSDLMIXER_VENDORED=ON \
      -DSDL3_DIR=../SDL/build \
      -DCMAKE_SYSTEM_NAME=Android \
      -DCMAKE_SYSTEM_VERSION=30 \
      -DCMAKE_ANDROID_ARCH_ABI=arm64-v8a \
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
      -DPLATFORM=android \
      -DARCH=arm64-v8a \
      -DBUILD_STATIC=OFF \
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
      -DCMAKE_SYSTEM_NAME=Android \
      -DCMAKE_SYSTEM_VERSION=30 \
      -DCMAKE_ANDROID_ARCH_ABI=arm64-v8a \
      -DBGFX_BUILD_EXAMPLES=OFF \
      -DBGFX_CONFIG_MULTITHREADED=ON \
      -DBGFX_CONFIG_MAX_FRAME_BUFFERS=256 \
      -DBGFX_OPENGLES_VERSION=32 \
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

   curl -sL https://github.com/vbousquet/pinmame/archive/${PINMAME_SHA}.tar.gz -o pinmame-${PINMAME_SHA}.tar.gz
   tar xzf pinmame-${PINMAME_SHA}.tar.gz
   mv pinmame-${PINMAME_SHA} pinmame
   cd pinmame
   cp cmake/libpinmame/CMakeLists.txt .
   cmake \
      -DPLATFORM=android \
      -DARCH=arm64-v8a \
      -DBUILD_STATIC=OFF \
      -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
      -B build
   cmake --build build -- -j${NUM_PROCS}
   cd ..

   echo "$PINMAME_EXPECTED_SHA" > cache.txt

   cd ..
fi

#
# build openxr
#

OPENXR_EXPECTED_SHA="${OPENXR_SHA}"
OPENXR_FOUND_SHA="$([ -f openxr/cache.txt ] && cat openxr/cache.txt || echo "")"

if [ "${OPENXR_EXPECTED_SHA}" != "${OPENXR_FOUND_SHA}" ]; then
   echo "Building OpenXR. Expected: ${OPENXR_EXPECTED_SHA}, Found: ${OPENXR_FOUND_SHA}"

   rm -rf openxr
   mkdir openxr
   cd openxr

   curl -sL https://github.com/KhronosGroup/OpenXR-SDK-Source/archive/${OPENXR_SHA}.tar.gz -o OpenXR-SDK-Source-${OPENXR_SHA}.zip
   tar xzf OpenXR-SDK-Source-${OPENXR_SHA}.zip
   mv OpenXR-SDK-Source-${OPENXR_SHA} openxr
   cd openxr
   cmake  \
      -DCMAKE_SYSTEM_NAME=Android \
      -DCMAKE_SYSTEM_VERSION=30 \
      -DCMAKE_ANDROID_ARCH_ABI=arm64-v8a \
      -DBUILD_TESTS=OFF \
      -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
      -B build
   cmake --build build -- -j${NUM_PROCS}
   cd ..

   echo "$OPENXR_EXPECTED_SHA" > cache.txt

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
   ./platforms/android/arm64-v8a/external.sh
   cmake \
      -DPLATFORM=android \
      -DARCH=arm64-v8a \
      -DBUILD_STATIC=OFF \
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
   ./platforms/android/arm64-v8a/external.sh
   cmake \
      -DPLATFORM=android \
      -DARCH=arm64-v8a \
      -DBUILD_STATIC=OFF \
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
   ./platforms/android/arm64-v8a/external.sh
   cmake \
      -DPLATFORM=android \
      -DARCH=arm64-v8a \
      -DBUILD_STATIC=OFF \
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
   if [[ $(uname) == "Linux" ]]; then
      TOOLCHAIN=${ANDROID_NDK_HOME}/toolchains/llvm/prebuilt/linux-x86_64
   elif [[ $(uname) == "Darwin" ]]; then
      TOOLCHAIN=${ANDROID_NDK_HOME}/toolchains/llvm/prebuilt/darwin-x86_64
   fi
   ./configure \
      --enable-cross-compile \
      --target-os=android \
      --arch=aarch64 \
      --sysroot=${TOOLCHAIN}/sysroot \
      --cc=${TOOLCHAIN}/bin/aarch64-linux-android21-clang \
      --cxx=${TOOLCHAIN}/bin/aarch64-linux-android21-clang++ \
      --ld=${TOOLCHAIN}/bin/aarch64-linux-android21-clang \
      --ar=${TOOLCHAIN}/bin/llvm-ar \
      --as=${TOOLCHAIN}/bin/aarch64-linux-android21-clang \
      --nm=${TOOLCHAIN}/bin/llvm-nm \
      --ranlib=${TOOLCHAIN}/bin/llvm-ranlib \
      --strip=${TOOLCHAIN}/bin/llvm-strip \
      --extra-cflags="-O3 -fPIC" \
      --extra-ldflags="-Wl,-z,max-page-size=16384" \
      --disable-programs \
      --disable-doc \
      --enable-shared \
      --disable-static
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

   curl -sL https://www.un4seen.com/files/bass24-android.zip -o bass.zip
   unzip bass.zip
   
   echo "$BASS_EXPECTED_SHA" > cache.txt

   cd ..
fi

#
# copy libraries
#

cp SDL3/SDL/build/libSDL3.so ../../../third-party/runtime-libs/android-arm64-v8a
cp -r SDL3/SDL/include/SDL3 ../../../third-party/include/

cp SDL3/SDL_image/build/libSDL3_image.so ../../../third-party/runtime-libs/android-arm64-v8a
cp -r SDL3/SDL_image/include/SDL3_image ../../../third-party/include/

cp SDL3/SDL_ttf/build/libSDL3_ttf.so ../../../third-party/runtime-libs/android-arm64-v8a
cp -r SDL3/SDL_ttf/include/SDL3_ttf ../../../third-party/include/

cp SDL3/SDL_mixer/build/libSDL3_mixer.so ../../../third-party/runtime-libs/android-arm64-v8a
cp -r SDL3/SDL_mixer/include/SDL3_mixer ../../../third-party/include/

cp freeimage/freeimage/build/libfreeimage.so ../../../third-party/runtime-libs/android-arm64-v8a
cp freeimage/freeimage/Source/FreeImage.h ../../../third-party/include

cp bgfx/bgfx.cmake/build/cmake/bgfx/libbgfx.a ../../../third-party/build-libs/android-arm64-v8a
cp -r bgfx/bgfx.cmake/bgfx/include/bgfx ../../../third-party/include/
cp bgfx/bgfx.cmake/build/cmake/bimg/libbimg.a ../../../third-party/build-libs/android-arm64-v8a
cp bgfx/bgfx.cmake/build/cmake/bimg/libbimg_decode.a ../../../third-party/build-libs/android-arm64-v8a
cp bgfx/bgfx.cmake/build/cmake/bimg/libbimg_encode.a ../../../third-party/build-libs/android-arm64-v8a
cp -r bgfx/bgfx.cmake/bimg/include/bimg ../../../third-party/include/
cp bgfx/bgfx.cmake/build/cmake/bx/libbx.a ../../../third-party/build-libs/android-arm64-v8a
cp -r bgfx/bgfx.cmake/bx/include/bx ../../../third-party/include/

cp pinmame/pinmame/build/libpinmame.so ../../../third-party/runtime-libs/android-arm64-v8a
cp pinmame/pinmame/src/libpinmame/libpinmame.h ../../../third-party/include
cp pinmame/pinmame/src/libpinmame/pinmamedef.h ../../../third-party/include

cp libaltsound/libaltsound/build/libaltsound.so ../../../third-party/runtime-libs/android-arm64-v8a
cp -r libaltsound/libaltsound/src/altsound.h ../../../third-party/include/

cp libdof/libdof/build/libdof.so ../../../third-party/runtime-libs/android-arm64-v8a
cp -r libdof/libdof/include/DOF ../../../third-party/include/

cp libdmdutil/libdmdutil/build/libdmdutil.so ../../../third-party/runtime-libs/android-arm64-v8a
cp -r libdmdutil/libdmdutil/include/DMDUtil ../../../third-party/include/
cp libdmdutil/libdmdutil/third-party/runtime-libs/android/arm64-v8a/libzedmd.so ../../../third-party/runtime-libs/android-arm64-v8a
cp libdmdutil/libdmdutil/third-party/include/ZeDMD.h ../../../third-party/include
cp libdmdutil/libdmdutil/third-party/runtime-libs/android/arm64-v8a/libserum.so ../../../third-party/runtime-libs/android-arm64-v8a
cp libdmdutil/libdmdutil/third-party/include/serum.h ../../../third-party/include
cp libdmdutil/libdmdutil/third-party/include/serum-decode.h ../../../third-party/include
cp libdmdutil/libdmdutil/third-party/runtime-libs/android/arm64-v8a/libpupdmd.so ../../../third-party/runtime-libs/android-arm64-v8a
cp libdmdutil/libdmdutil/third-party/include/pupdmd.h ../../../third-party/include
cp libdmdutil/libdmdutil/third-party/runtime-libs/android/arm64-v8a/libsockpp.so ../../../third-party/runtime-libs/android-arm64-v8a

for LIB in libavcodec libavdevice libavfilter libavformat libavutil libswresample libswscale; do
   cp ffmpeg/ffmpeg/${LIB}/${LIB}.so ../../../third-party/runtime-libs/android-arm64-v8a
   mkdir -p ../../../third-party/include/${LIB}
   cp ffmpeg/ffmpeg/${LIB}/*.h ../../../third-party/include/${LIB}
done

cp bass/libs/arm64-v8a/libbass.so ../../../third-party/runtime-libs/android-arm64-v8a