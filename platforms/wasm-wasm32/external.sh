#!/bin/bash

set -e

source ./platforms/config.sh

if [ -z "${EMSDK}" ]; then
   echo "EMSDK is not set: source emsdk_env.sh first"
   exit 1
fi

echo "Building external libraries..."
echo "  SDL_SHA: ${SDL_SHA}"
echo "  SDL_IMAGE_SHA: ${SDL_IMAGE_SHA}"
echo "  SDL_TTF_SHA: ${SDL_TTF_SHA}"
echo "  FREEIMAGE_SHA: ${FREEIMAGE_SHA}"
echo "  BGFX_CMAKE_VERSION: ${BGFX_CMAKE_VERSION}"
echo "  BGFX_PATCH_SHA: ${BGFX_PATCH_SHA}"
echo "  PINMAME_SHA: ${PINMAME_SHA}"
echo "  LIBSERUM_SHA: ${LIBSERUM_SHA}"
echo "  LIBPUPDMD_SHA: ${LIBPUPDMD_SHA}"
echo "  LIBVNI_SHA: ${LIBVNI_SHA}"
echo "  LIBALTSOUND_SHA: ${LIBALTSOUND_SHA}"
echo "  FFMPEG_SHA: ${FFMPEG_SHA}"
echo "  LIBWINEVBS_SHA: ${LIBWINEVBS_SHA}"
echo "  LIBZIP_SHA: ${LIBZIP_SHA}"
echo "  LIBMYSOFA_SHA: ${LIBMYSOFA_SHA}"
echo "  LIBSPATIALAUDIO_SHA: ${LIBSPATIALAUDIO_SHA}"
echo "  ZLIB_SHA: ${ZLIB_SHA}"
echo ""

NUM_PROCS=$(sysctl -n hw.ncpu 2>/dev/null || nproc)

export CFLAGS="-pthread -fwasm-exceptions"
export CXXFLAGS="-pthread -fwasm-exceptions"
export LDFLAGS="-pthread -fwasm-exceptions"

mkdir -p "external/wasm-wasm32/${BUILD_TYPE}"
cd "external/wasm-wasm32/${BUILD_TYPE}"

#
# build SDL3, SDL3_image, SDL3_ttf
#

SDL3_EXPECTED_SHA="${SDL_SHA}-${SDL_IMAGE_SHA}-${SDL_TTF_SHA}_001"
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
   emcmake cmake \
      -DSDL_SHARED=OFF \
      -DSDL_STATIC=ON \
      -DSDL_TEST_LIBRARY=OFF \
      -DSDL_PTHREADS=ON \
      -DSDL_CAMERA=OFF \
      -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
      -B build
   cmake --build build -- -j${NUM_PROCS}
   cd ..

   curl -sL https://github.com/libsdl-org/SDL_image/archive/${SDL_IMAGE_SHA}.tar.gz -o SDL_image-${SDL_IMAGE_SHA}.tar.gz
   tar xzf SDL_image-${SDL_IMAGE_SHA}.tar.gz
   mv SDL_image-${SDL_IMAGE_SHA} SDL_image
   cd SDL_image
   ./external/download.sh
   emcmake cmake \
      -DBUILD_SHARED_LIBS=OFF \
      -DSDLIMAGE_SAMPLES=OFF \
      -DSDLIMAGE_DEPS_SHARED=OFF \
      -DSDLIMAGE_VENDORED=ON \
      -DSDLIMAGE_AVIF=OFF \
      -DSDLIMAGE_WEBP=OFF \
      -DSDL3_DIR=../SDL/build \
      -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
      -B build
   cmake --build build -- -j${NUM_PROCS}
   cd ..

   curl -sL https://github.com/libsdl-org/SDL_ttf/archive/${SDL_TTF_SHA}.tar.gz -o SDL_ttf-${SDL_TTF_SHA}.tar.gz
   tar xzf SDL_ttf-${SDL_TTF_SHA}.tar.gz
   mv SDL_ttf-${SDL_TTF_SHA} SDL_ttf
   cd SDL_ttf
   ./external/download.sh
   emcmake cmake \
      -DBUILD_SHARED_LIBS=OFF \
      -DSDLTTF_SAMPLES=OFF \
      -DSDLTTF_VENDORED=ON \
      -DSDLTTF_HARFBUZZ=ON \
      -DSDL3_DIR=../SDL/build \
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
   emcmake cmake \
      -DPLATFORM=linux \
      -DARCH=wasm32 \
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
   emcmake cmake -S. \
      -DBGFX_BUILD_EXAMPLES=OFF \
      -DBGFX_BUILD_TOOLS=OFF \
      -DBGFX_CONFIG_MULTITHREADED=ON \
      -DBGFX_CONFIG_MAX_FRAME_BUFFERS=256 \
      -DBGFX_CONFIG_RENDERER_WEBGPU=ON \
      -DCMAKE_C_FLAGS="${CFLAGS} -DBX_CONFIG_SUPPORTS_THREADING=1 -DBGFX_CONFIG_MULTITHREADED=1 -DBGFX_CONFIG_RENDERER_WEBGPU=1" \
      -DCMAKE_CXX_FLAGS="${CXXFLAGS} -DBX_CONFIG_SUPPORTS_THREADING=1 -DBGFX_CONFIG_MULTITHREADED=1 -DBGFX_CONFIG_RENDERER_WEBGPU=1 --use-port=emdawnwebgpu" \
      -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
      -B build
   cmake --build build -- -j${NUM_PROCS}

   env -u CFLAGS -u CXXFLAGS -u LDFLAGS cmake -S. \
      -DBGFX_BUILD_TOOLS=ON \
      -DBGFX_BUILD_TOOLS_SHADER=ON \
      -DBGFX_BUILD_TOOLS_BIN2C=OFF \
      -DBGFX_BUILD_TOOLS_GEOMETRY=OFF \
      -DBGFX_BUILD_TOOLS_TEXTURE=OFF \
      -DBGFX_BUILD_EXAMPLES=OFF \
      -DCMAKE_BUILD_TYPE=Release \
      -B build-tools
   env -u CFLAGS -u CXXFLAGS -u LDFLAGS cmake --build build-tools --target shaderc -- -j${NUM_PROCS}
   cd ..

   echo "$BGFX_EXPECTED_SHA" > cache.txt

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
   emcmake cmake \
      -DPLATFORM=linux \
      -DARCH=wasm32 \
      -DBUILD_SHARED=OFF \
      -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
      -B build
   cmake --build build -- -j${NUM_PROCS}
   cd ..

   echo "$LIBWINEVBS_EXPECTED_SHA" > cache.txt

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
   emcmake cmake \
      -DBUILD_SHARED_LIBS=OFF \
      -DZLIB_BUILD_EXAMPLES=OFF \
      -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
      -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
      -B build
   cmake --build build -- -j${NUM_PROCS}
   cp build/zconf.h .
   cd ..

   echo "$ZLIB_EXPECTED_SHA" > cache.txt

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
   CFLAGS="${CFLAGS} -I$PWD/../../zlib/zlib" CXXFLAGS="${CXXFLAGS} -I$PWD/../../zlib/zlib" emcmake cmake \
      -DPLATFORM=linux \
      -DARCH=wasm32 \
      -DBUILD_SHARED=OFF \
      -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
      -B build
   cmake --build build --target pinmame_static -- -j${NUM_PROCS}
   cd ..

   echo "$PINMAME_EXPECTED_SHA" > cache.txt

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
   ./platforms/linux/x64/external.sh
   emcmake cmake \
      -DPLATFORM=linux \
      -DARCH=wasm32 \
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
   emcmake cmake \
      -DPLATFORM=linux \
      -DARCH=wasm32 \
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
   ./platforms/linux/x64/external.sh
   emcmake cmake \
      -DPLATFORM=linux \
      -DARCH=wasm32 \
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
   emcmake cmake \
      -DPLATFORM=wasm \
      -DARCH=wasm32 \
      -DBUILD_SHARED=OFF \
      -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
      -B build
   cmake --build build -- -j${NUM_PROCS}
   cd ..

   echo "$LIBALTSOUND_EXPECTED_SHA" > cache.txt

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
   emconfigure ./configure \
      --cc=emcc \
      --cxx=em++ \
      --ar=emar \
      --ranlib=emranlib \
      --nm=emnm \
      --enable-cross-compile \
      --target-os=none \
      --arch=x86_32 \
      --cpu=generic \
      --disable-asm \
      --disable-stripping \
      --enable-static \
      --disable-shared \
      --disable-programs \
      --disable-doc \
      --disable-avdevice \
      --disable-avfilter \
      --extra-cflags="${CFLAGS}" \
      --extra-ldflags="${LDFLAGS}"
   emmake make -j${NUM_PROCS}
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
   emcmake cmake \
      -DBUILD_SHARED_LIBS=OFF \
      -DENABLE_ZSTD=OFF \
      -DENABLE_BZIP2=OFF \
      -DENABLE_LZMA=OFF \
      -DBUILD_TOOLS=OFF \
      -DBUILD_REGRESS=OFF \
      -DBUILD_OSSFUZZ=OFF \
      -DBUILD_EXAMPLES=OFF \
      -DBUILD_DOC=OFF \
      -DZLIB_INCLUDE_DIR="$PWD/../../zlib/zlib" \
      -DZLIB_LIBRARY="$PWD/../../zlib/zlib/build/libz.a" \
      -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
      -B build
   cmake --build build -- -j${NUM_PROCS}
   cd ..

   echo "$LIBZIP_EXPECTED_SHA" > cache.txt

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
   sed -i.bak 's/#if defined(__linux__) || defined(__CYGWIN__)/#if defined(__linux__) || defined(__EMSCRIPTEN__) || defined(__CYGWIN__)/' src/hrtf/portable_endian.h
   emcmake cmake \
      -DBUILD_SHARED_LIBS=OFF \
      -DBUILD_STATIC_LIBS=ON \
      -DBUILD_TESTS=OFF \
      -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
      -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
      -DZLIB_INCLUDE_DIR="$PWD/../../zlib/zlib" \
      -DZLIB_LIBRARY="$PWD/../../zlib/zlib/build/libz.a" \
      -DMATH=m \
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
   emcmake cmake \
      -DBUILD_SHARED_LIBS=OFF \
      -DBUILD_TESTING=OFF \
      -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
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

cp SDL3/SDL/build/libSDL3.a ../../../third-party/build-libs/wasm-wasm32
cp -r SDL3/SDL/include/SDL3 ../../../third-party/include/

cp SDL3/SDL_image/build/libSDL3_image.a ../../../third-party/build-libs/wasm-wasm32
cp -r SDL3/SDL_image/include/SDL3_image ../../../third-party/include/

cp SDL3/SDL_ttf/build/libSDL3_ttf.a ../../../third-party/build-libs/wasm-wasm32
cp -r SDL3/SDL_ttf/include/SDL3_ttf ../../../third-party/include/
cp SDL3/SDL_ttf/build/external/freetype/libfreetype.a ../../../third-party/build-libs/wasm-wasm32
cp SDL3/SDL_ttf/build/external/harfbuzz/libharfbuzz.a ../../../third-party/build-libs/wasm-wasm32

cp freeimage/freeimage/build/libfreeimage.a ../../../third-party/build-libs/wasm-wasm32
cp freeimage/freeimage/Source/FreeImage.h ../../../third-party/include

cp bgfx/bgfx.cmake/build/cmake/bgfx/libbgfx.a ../../../third-party/build-libs/wasm-wasm32
cp -r bgfx/bgfx.cmake/bgfx/include/bgfx ../../../third-party/include/
cp bgfx/bgfx.cmake/build/cmake/bimg/libbimg.a ../../../third-party/build-libs/wasm-wasm32
cp bgfx/bgfx.cmake/build/cmake/bimg/libbimg_encode.a ../../../third-party/build-libs/wasm-wasm32
cp -r bgfx/bgfx.cmake/bimg/include/bimg ../../../third-party/include/
cp bgfx/bgfx.cmake/build/cmake/bx/libbx.a ../../../third-party/build-libs/wasm-wasm32
cp -r bgfx/bgfx.cmake/bx/include/bx ../../../third-party/include/

case "$(uname -s)-$(uname -m)" in
   Darwin-arm64)   HOST_TOOLS=macos-arm64 ;;
   Darwin-x86_64)  HOST_TOOLS=macos-x64 ;;
   Linux-x86_64)   HOST_TOOLS=linux-x64 ;;
   Linux-aarch64)  HOST_TOOLS=linux-aarch64 ;;
esac
mkdir -p "../../../third-party/tools/${HOST_TOOLS}"
cp bgfx/bgfx.cmake/build-tools/cmake/bgfx/shaderc "../../../third-party/tools/${HOST_TOOLS}/"

cp pinmame/pinmame/build/libpinmame.a ../../../third-party/build-libs/wasm-wasm32
mkdir -p ../../../third-party/include/pinmame
cp pinmame/pinmame/src/libpinmame/libpinmame.h ../../../third-party/include/pinmame
cp pinmame/pinmame/src/libpinmame/PinMAMEPlugin.h ../../../third-party/include/pinmame

cp libserum/libserum/build/libserum.a ../../../third-party/build-libs/wasm-wasm32
cp libserum/libserum/src/serum.h ../../../third-party/include
cp libserum/libserum/src/serum-decode.h ../../../third-party/include

cp libpupdmd/libpupdmd/build/libpupdmd.a ../../../third-party/build-libs/wasm-wasm32
cp libpupdmd/libpupdmd/src/pupdmd.h ../../../third-party/include

cp libvni/libvni/build/libvni.a ../../../third-party/build-libs/wasm-wasm32
cp libvni/libvni/src/vni.h ../../../third-party/include

cp libaltsound/libaltsound/build/libaltsound.a ../../../third-party/build-libs/wasm-wasm32
cp libaltsound/libaltsound/src/altsound.h ../../../third-party/include

cp libwinevbs/libwinevbs/build/libwinevbs.a ../../../third-party/build-libs/wasm-wasm32
mkdir -p ../../../third-party/include/libwinevbs/wine/include
mkdir -p ../../../third-party/include/libwinevbs/atl/include
cp libwinevbs/libwinevbs/include/libwinevbs.h ../../../third-party/include/libwinevbs/
cp -r libwinevbs/libwinevbs/wine/include/* ../../../third-party/include/libwinevbs/wine/include/
cp -r libwinevbs/libwinevbs/atl/include/* ../../../third-party/include/libwinevbs/atl/include/

for LIB in libavcodec libavformat libavutil libswresample libswscale; do
   cp -a ffmpeg/ffmpeg/${LIB}/${LIB}.a ../../../third-party/build-libs/wasm-wasm32
   mkdir -p ../../../third-party/include/${LIB}
   cp ffmpeg/ffmpeg/${LIB}/*.h ../../../third-party/include/${LIB}
done

cp libzip/libzip/build/lib/libzip.a ../../../third-party/build-libs/wasm-wasm32
cp libzip/libzip/build/zipconf.h ../../../third-party/include
cp libzip/libzip/lib/zip.h ../../../third-party/include

cp zlib/zlib/build/libz.a ../../../third-party/build-libs/wasm-wasm32

cp libmysofa/libmysofa/build/src/libmysofa.a ../../../third-party/build-libs/wasm-wasm32
cp libmysofa/libmysofa/src/hrtf/mysofa.h ../../../third-party/include
cp libmysofa/libmysofa/src/hrtf/mysofa_export.h ../../../third-party/include

cp libspatialaudio/libspatialaudio/build/libspatialaudio.a ../../../third-party/build-libs/wasm-wasm32
mkdir -p ../../../third-party/include/spatialaudio
cp -r libspatialaudio/libspatialaudio/include/*.h libspatialaudio/libspatialaudio/include/{adm,dsp,hrtf,kiss_fft} ../../../third-party/include/spatialaudio
cp libspatialaudio/libspatialaudio/build/include/SpatialaudioConfig.h ../../../third-party/include/spatialaudio
