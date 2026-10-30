#!/bin/bash

set -e

source ./platforms/config.sh

if [ -z "${MSYS2_PATH}" ]; then
   MSYS2_PATH="/c/msys64"
fi

export MSYSTEM=UCRT64

echo "MSYS2_PATH: ${MSYS2_PATH}"
echo ""

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
echo "  OPENXR_SHA: ${OPENXR_SHA}"
echo "  LIBDMDUTIL_SHA: ${LIBDMDUTIL_SHA}"
echo "  LIBSERUM_SHA: ${LIBSERUM_SHA}"
echo "  LIBPUPDMD_SHA: ${LIBPUPDMD_SHA}"
echo "  LIBVNI_SHA: ${LIBVNI_SHA}"
echo "  LIBALTSOUND_SHA: ${LIBALTSOUND_SHA}"
echo "  LIBDOF_SHA: ${LIBDOF_SHA}"
echo "  FFMPEG_SHA: ${FFMPEG_SHA}"
echo "  LIBWINEVBS_SHA: ${LIBWINEVBS_SHA}"
echo "  LIBMYSOFA_SHA: ${LIBMYSOFA_SHA}"
echo "  LIBOGG_SHA: ${LIBOGG_SHA}"
echo "  LIBOPUS_SHA: ${LIBOPUS_SHA}"
echo "  OPUSFILE_SHA: ${OPUSFILE_SHA}"
echo "  LIBSPATIALAUDIO_SHA: ${LIBSPATIALAUDIO_SHA}"
echo "  ZLIB_SHA: ${ZLIB_SHA}"
echo ""

mkdir -p "external/windows-x64/${BUILD_TYPE}"
cd "external/windows-x64/${BUILD_TYPE}"

#
# build SDL3, SDL3_image, SDL3_ttf#

SDL3_EXPECTED_SHA="${SDL_SHA}-${SDL_IMAGE_SHA}-${SDL_TTF_SHA}"
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
   sed -i.bak 's/OUTPUT_NAME "SDL3"/OUTPUT_NAME "SDL364"/g' CMakeLists.txt
   cmake \
      -G "Visual Studio 18 2026" \
      -DSDL_SHARED=ON \
      -DSDL_STATIC=OFF \
      -DSDL_TEST_LIBRARY=OFF \
      -B build
   cmake --build build --config ${BUILD_TYPE}
   cd ..

   curl -sL https://github.com/libsdl-org/SDL_image/archive/${SDL_IMAGE_SHA}.tar.gz -o SDL_image-${SDL_IMAGE_SHA}.tar.gz
   tar xzf SDL_image-${SDL_IMAGE_SHA}.tar.gz --exclude='*/Xcode/*'
   mv SDL_image-${SDL_IMAGE_SHA} SDL_image
   cd SDL_image
   sed -i.bak 's/OUTPUT_NAME "SDL3_image"/OUTPUT_NAME "SDL3_image64"/g' CMakeLists.txt
   ./external/download.sh
   cmake \
      -G "Visual Studio 18 2026" \
      -DBUILD_SHARED_LIBS=ON \
      -DSDLIMAGE_SAMPLES=OFF \
      -DSDLIMAGE_DEPS_SHARED=ON \
      -DSDLIMAGE_VENDORED=ON \
      -DSDLIMAGE_AVIF=OFF \
      -DSDLIMAGE_WEBP=OFF \
      -DSDL3_DIR=../SDL/build \
      -B build
   cmake --build build --config ${BUILD_TYPE}
   cd ..

   curl -sL https://github.com/libsdl-org/SDL_ttf/archive/${SDL_TTF_SHA}.tar.gz -o SDL_ttf-${SDL_TTF_SHA}.tar.gz
   tar xzf SDL_ttf-${SDL_TTF_SHA}.tar.gz --exclude='*/Xcode/*'
   mv SDL_ttf-${SDL_TTF_SHA} SDL_ttf
   cd SDL_ttf
   sed -i.bak 's/OUTPUT_NAME SDL3_ttf/OUTPUT_NAME SDL3_ttf64/g' CMakeLists.txt
   ./external/download.sh
   cmake \
      -G "Visual Studio 18 2026" \
      -DBUILD_SHARED_LIBS=ON \
      -DSDLTTF_SAMPLES=OFF \
      -DSDLTTF_VENDORED=ON \
      -DSDLTTF_HARFBUZZ=ON \
      -DSDL3_DIR=../SDL/build \
      -B build
   cmake --build build --config ${BUILD_TYPE}
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
      -G "Visual Studio 18 2026" \
      -DPLATFORM=win \
      -DARCH=x64 \
      -DBUILD_SHARED=ON \
      -DBUILD_STATIC=OFF \
      -B build
   cmake --build build --config ${BUILD_TYPE}
   cd ..

   echo "$FREEIMAGE_EXPECTED_SHA" > cache.txt

   cd ..
fi

#
# build bgfx
#

BGFX_EXPECTED_SHA="${BGFX_CMAKE_VERSION}-${BGFX_PATCH_SHA}-003"
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
   sed -i.bak 's/set_target_properties(bx PROPERTIES FOLDER "bgfx")/set_target_properties(bx PROPERTIES FOLDER "bgfx" OUTPUT_NAME "bx64")/g' cmake/bx/bx.cmake
   sed -i.bak 's/set_target_properties(bimg PROPERTIES FOLDER "bgfx")/set_target_properties(bimg PROPERTIES FOLDER "bgfx" OUTPUT_NAME "bimg64")/g' cmake/bimg/bimg.cmake
   sed -i.bak 's/set_target_properties(bimg_encode PROPERTIES FOLDER "bgfx")/set_target_properties(bimg_encode PROPERTIES FOLDER "bgfx" OUTPUT_NAME "bimg_encode64")/g' cmake/bimg/bimg_encode.cmake
   sed -i.bak 's/set_target_properties(bgfx PROPERTIES FOLDER "bgfx")/set_target_properties(bgfx PROPERTIES FOLDER "bgfx" OUTPUT_NAME "bgfx64")/g' cmake/bgfx/bgfx.cmake
   cmake -G "Visual Studio 18 2026" \
      -S. \
      -DBGFX_LIBRARY_TYPE=STATIC \
      -DBGFX_BUILD_TOOLS=OFF \
      -DBGFX_BUILD_EXAMPLES=OFF \
      -DBGFX_CONFIG_MULTITHREADED=ON \
      -DBGFX_CONFIG_MAX_FRAME_BUFFERS=256 \
      -DCMAKE_MSVC_RUNTIME_LIBRARY="MultiThreaded\$<\$<CONFIG:Debug>:Debug>" \
      -B build
   cmake --build build --config ${BUILD_TYPE}

   # Build shaderc as a host tool (always Release: it only runs at build time to
   # compile the embedded shaders, see src/shaders/bgfx/shaders.cmake)
   cmake -G "Visual Studio 18 2026" \
      -S. \
      -A x64 \
      -DBGFX_BUILD_TOOLS=ON \
      -DBGFX_BUILD_TOOLS_SHADER=ON \
      -DBGFX_BUILD_TOOLS_BIN2C=OFF \
      -DBGFX_BUILD_TOOLS_GEOMETRY=OFF \
      -DBGFX_BUILD_TOOLS_TEXTURE=OFF \
      -DBGFX_BUILD_EXAMPLES=OFF \
      -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded \
      -B build-tools
   cmake --build build-tools --config Release --target shaderc
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
      -G "Visual Studio 18 2026" \
      -DPLATFORM=win \
      -DARCH=x64 \
      -DBUILD_SHARED=ON \
      -DBUILD_STATIC=OFF \
      -B build
   cmake --build build --config ${BUILD_TYPE}
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
# build openxr
#

OPENXR_EXPECTED_SHA="${OPENXR_SHA}"
OPENXR_FOUND_SHA="$([ -f openxr/cache.txt ] && cat openxr/cache.txt || echo "")"

if [ "${OPENXR_EXPECTED_SHA}" != "${OPENXR_FOUND_SHA}" ]; then
   echo "Building OpenXR. Expected: ${OPENXR_EXPECTED_SHA}, Found: ${OPENXR_FOUND_SHA}"

   rm -rf openxr
   mkdir openxr
   cd openxr

   curl -sL https://github.com/KhronosGroup/OpenXR-SDK-Source/archive/${OPENXR_SHA}.tar.gz -o OpenXR-SDK-Source-${OPENXR_SHA}.tar.gz
   tar xzf OpenXR-SDK-Source-${OPENXR_SHA}.tar.gz
   mv OpenXR-SDK-Source-${OPENXR_SHA} openxr
   cd openxr
   sed -i.bak 's/set_target_properties(openxr_loader PROPERTIES FOLDER ${LOADER_FOLDER})/set_target_properties(openxr_loader PROPERTIES FOLDER ${LOADER_FOLDER} OUTPUT_NAME "openxr_loader64")/g' src/loader/CMakeLists.txt
   sed -i.bak 's|\${CMAKE_CURRENT_BINARY_DIR}/$<CONFIGURATION>/openxr_loader|\${CMAKE_CURRENT_BINARY_DIR}/$<CONFIGURATION>/openxr_loader64|g' src/loader/CMakeLists.txt
   cmake \
      -G "Visual Studio 18 2026" \
      -DBUILD_WITH_SYSTEM_JSONCPP=OFF \
      -DBUILD_TESTS=OFF \
      -DBUILD_API_LAYERS=OFF \
      -DDYNAMIC_LOADER=ON \
      -DOPENXR_DEBUG_POSTFIX="" \
      -B build
   cmake --build build --config ${BUILD_TYPE}
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
   ENABLE_SERUM=OFF ENABLE_VNI=OFF ENABLE_PUP=OFF ./platforms/win/x64/external.sh
   cmake \
      -G "Visual Studio 18 2026" \
      -DPLATFORM=win \
      -DARCH=x64 \
      -DBUILD_SHARED=ON \
      -DBUILD_STATIC=OFF \
      -DENABLE_SERUM=OFF \
      -DENABLE_VNI=OFF \
      -DENABLE_PUP=OFF \
      -B build
   cmake --build build --config ${BUILD_TYPE}
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
   ./platforms/win/x64/external.sh
   cmake \
      -G "Visual Studio 18 2026" \
      -DPLATFORM=win \
      -DARCH=x64 \
      -DBUILD_SHARED=ON \
      -DBUILD_STATIC=OFF \
      -B build
   cmake --build build --config ${BUILD_TYPE}
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
      -G "Visual Studio 18 2026" \
      -DPLATFORM=win \
      -DARCH=x64 \
      -DBUILD_SHARED=ON \
      -DBUILD_STATIC=OFF \
      -B build
   cmake --build build --config ${BUILD_TYPE}
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
   ./platforms/win/x64/external.sh
   cmake \
      -G "Visual Studio 18 2026" \
      -DPLATFORM=win \
      -DARCH=x64 \
      -DBUILD_SHARED=ON \
      -DBUILD_STATIC=OFF \
      -B build
   cmake --build build --config ${BUILD_TYPE}
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
      -G "Visual Studio 18 2026" \
      -DPLATFORM=win \
      -DARCH=x64 \
      -DBUILD_SHARED=ON \
      -DBUILD_STATIC=OFF \
      -B build
   cmake --build build --config ${BUILD_TYPE}
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
   ./platforms/win/x64/external.sh
   cmake \
      -G "Visual Studio 18 2026" \
      -DPLATFORM=win \
      -DARCH=x64 \
      -DBUILD_SHARED=ON \
      -DBUILD_STATIC=OFF \
      -B build
   cmake --build build --config ${BUILD_TYPE}
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
   CURRENT_DIR="$(pwd)"
   "${MSYS2_PATH}/usr/bin/bash.exe" -l -c "
      cd \"${CURRENT_DIR}\" &&
      ./configure \
         --enable-shared \
         --disable-static \
         --disable-programs \
         --disable-doc \
         --disable-avdevice \
         --disable-avfilter \
         --arch=\"x86_64\" \
         --build-suffix=64 &&
      make -j$(nproc)
   "
   cd ..

   echo "$FFMPEG_EXPECTED_SHA" > cache.txt

   cd ..
fi

#
# build libwinevbs (VBScript engine of the standalone build), with MinGW like ffmpeg as Wine's code does not build with MSVC (yet).
# Only the DLL and header are used, vpx-standalone.vcxproj makes the import library (see make/winevbs64-msvc.def)
#

LIBWINEVBS_EXPECTED_SHA="${LIBWINEVBS_SHA}-001"
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
   CURRENT_DIR="$(pwd)"
   "${MSYS2_PATH}/usr/bin/bash.exe" -l -c "
      cd \"${CURRENT_DIR}\" &&
      cmake \
         -DBUILD_SHARED=ON \
         -DPLATFORM=win-mingw \
         -DARCH=x64 \
         -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
         -B build &&
      cmake --build build -- -j$(nproc)
   "
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
   sed -i.bak '/set(CMAKE_DEBUG_POSTFIX "d")/d' CMakeLists.txt
   cmake \
      -G "Visual Studio 18 2026" \
      -DBUILD_SHARED_LIBS=OFF \
      -DZLIB_BUILD_EXAMPLES=OFF \
      -DCMAKE_MSVC_RUNTIME_LIBRARY="MultiThreaded\$<\$<CONFIG:Debug>:Debug>" \
      -B build
   cmake --build build --config ${BUILD_TYPE}
   cp build/zconf.h .
   cd ..

   echo "$ZLIB_EXPECTED_SHA" > cache.txt

   cd ..
fi

#
# build libogg, libopus, opusfile (Opus sounds)
#

OPUS_EXPECTED_SHA="${LIBOGG_SHA}-${LIBOPUS_SHA}-${OPUSFILE_SHA}_002"
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
      -G "Visual Studio 18 2026" \
      -DCMAKE_POLICY_DEFAULT_CMP0091=NEW \
      -DCMAKE_MSVC_RUNTIME_LIBRARY="MultiThreaded\$<\$<CONFIG:Debug>:Debug>" \
      -DBUILD_SHARED_LIBS=OFF \
      -DINSTALL_DOCS=OFF \
      -DBUILD_TESTING=OFF \
      -DCMAKE_INSTALL_PREFIX="$PWD/../install" \
      -B build
   cmake --build build --config ${BUILD_TYPE}
   cmake --install build --config ${BUILD_TYPE}
   cd ..

   curl -sL https://github.com/xiph/opus/archive/${LIBOPUS_SHA}.tar.gz -o opus-${LIBOPUS_SHA}.tar.gz
   tar xzf opus-${LIBOPUS_SHA}.tar.gz
   mv opus-${LIBOPUS_SHA} opus
   cd opus
   cmake \
      -G "Visual Studio 18 2026" \
      -DCMAKE_POLICY_DEFAULT_CMP0091=NEW \
      -DCMAKE_MSVC_RUNTIME_LIBRARY="MultiThreaded\$<\$<CONFIG:Debug>:Debug>" \
      -DBUILD_SHARED_LIBS=OFF \
      -DOPUS_BUILD_PROGRAMS=OFF \
      -DOPUS_BUILD_TESTING=OFF \
      -DOPUS_STATIC_RUNTIME=ON \
      -DCMAKE_INSTALL_PREFIX="$PWD/../install" \
      -B build
   cmake --build build --config ${BUILD_TYPE}
   cmake --install build --config ${BUILD_TYPE}
   cd ..

   curl -sL https://github.com/xiph/opusfile/archive/${OPUSFILE_SHA}.tar.gz -o opusfile-${OPUSFILE_SHA}.tar.gz
   tar xzf opusfile-${OPUSFILE_SHA}.tar.gz
   mv opusfile-${OPUSFILE_SHA} opusfile
   cd opusfile
   cmake \
      -G "Visual Studio 18 2026" \
      -DCMAKE_POLICY_DEFAULT_CMP0091=NEW \
      -DCMAKE_MSVC_RUNTIME_LIBRARY="MultiThreaded\$<\$<CONFIG:Debug>:Debug>" \
      -DBUILD_SHARED_LIBS=OFF \
      -DOP_DISABLE_HTTP=ON \
      -DOP_DISABLE_DOCS=ON \
      -DOP_DISABLE_EXAMPLES=ON \
      -DCMAKE_PREFIX_PATH="$PWD/../install" \
      -DCMAKE_INSTALL_PREFIX="$PWD/../install" \
      -B build
   cmake --build build --config ${BUILD_TYPE}
   cmake --install build --config ${BUILD_TYPE}
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
   sed -i.bak '/find_program(NUGET nuget)/,/zlib-1.3.1\/include\/)/c\  include(FindZLIB)\n  include_directories(${ZLIB_INCLUDE_DIRS})' src/CMakeLists.txt
   cmake \
      -G "Visual Studio 18 2026" \
      -DBUILD_SHARED_LIBS=OFF \
      -DBUILD_STATIC_LIBS=ON \
      -DBUILD_TESTS=OFF \
      -DC_HAS_WALL=OFF \
      -DCMAKE_POLICY_DEFAULT_CMP0091=NEW \
      -DCMAKE_MSVC_RUNTIME_LIBRARY="MultiThreaded\$<\$<CONFIG:Debug>:Debug>" \
      -DZLIB_INCLUDE_DIR="$PWD/../../zlib/zlib" \
      -DZLIB_LIBRARY="$PWD/../../zlib/zlib/build/${BUILD_TYPE}/zlibstatic.lib" \
      -B build
   cmake --build build --config ${BUILD_TYPE}
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
      -G "Visual Studio 18 2026" \
      -DBUILD_SHARED_LIBS=OFF \
      -DBUILD_TESTING=OFF \
      -DCMAKE_MSVC_RUNTIME_LIBRARY="MultiThreaded\$<\$<CONFIG:Debug>:Debug>" \
      -DMYSOFA_INCLUDE_DIRS="$PWD/../../libmysofa/libmysofa/src/hrtf" \
      -DMYSOFA_LIBRARIES="$PWD/../../libmysofa/libmysofa/build/src/${BUILD_TYPE}/mysofa.lib" \
      -B build
   cmake --build build --config ${BUILD_TYPE}
   cd ..

   echo "$LIBSPATIALAUDIO_EXPECTED_SHA" > cache.txt

   cd ..
fi

#
# copy libraries
#

cp SDL3/SDL/build/${BUILD_TYPE}/SDL364.lib ../../../third-party/build-libs/windows-x64
cp SDL3/SDL/build/${BUILD_TYPE}/SDL364.dll ../../../third-party/runtime-libs/windows-x64
cp -r SDL3/SDL/include/SDL3 ../../../third-party/include/

cp SDL3/SDL_image/build/${BUILD_TYPE}/SDL3_image64.lib ../../../third-party/build-libs/windows-x64
cp SDL3/SDL_image/build/${BUILD_TYPE}/SDL3_image64.dll ../../../third-party/runtime-libs/windows-x64
cp -r SDL3/SDL_image/include/SDL3_image ../../../third-party/include/

cp SDL3/SDL_ttf/build/${BUILD_TYPE}/SDL3_ttf64.lib ../../../third-party/build-libs/windows-x64
cp SDL3/SDL_ttf/build/${BUILD_TYPE}/SDL3_ttf64.dll ../../../third-party/runtime-libs/windows-x64
cp -r SDL3/SDL_ttf/include/SDL3_ttf ../../../third-party/include/

cp freeimage/freeimage/build/${BUILD_TYPE}/freeimage64.lib ../../../third-party/build-libs/windows-x64
cp freeimage/freeimage/build/${BUILD_TYPE}/freeimage64.dll ../../../third-party/runtime-libs/windows-x64
cp freeimage/freeimage/Source/FreeImage.h ../../../third-party/include

cp -r bgfx/bgfx.cmake/bgfx/include/bgfx ../../../third-party/include/
cp -r bgfx/bgfx.cmake/bimg/include/bimg ../../../third-party/include/
cp -r bgfx/bgfx.cmake/bx/include/bx ../../../third-party/include/
cp bgfx/bgfx.cmake/build/cmake/bgfx/${BUILD_TYPE}/bgfx64.lib ../../../third-party/build-libs/windows-x64
cp bgfx/bgfx.cmake/build/cmake/bimg/${BUILD_TYPE}/bimg64.lib ../../../third-party/build-libs/windows-x64
cp bgfx/bgfx.cmake/build/cmake/bimg/${BUILD_TYPE}/bimg_encode64.lib ../../../third-party/build-libs/windows-x64
cp bgfx/bgfx.cmake/build/cmake/bx/${BUILD_TYPE}/bx64.lib ../../../third-party/build-libs/windows-x64

# shaderc host tool (compiles the embedded BGFX shaders at build time). The DXC
# dlls are loaded by shaderc at runtime for the s_6_0 (D3D12/DXIL) target.
mkdir -p ../../../third-party/tools/windows-x64
cp bgfx/bgfx.cmake/build-tools/cmake/bgfx/Release/shaderc.exe ../../../third-party/tools/windows-x64/
cp bgfx/bgfx.cmake/bgfx/tools/bin/windows/dxcompiler.dll ../../../third-party/tools/windows-x64/
cp bgfx/bgfx.cmake/bgfx/tools/bin/windows/dxil.dll ../../../third-party/tools/windows-x64/
cp bgfx/bgfx.cmake/bgfx/tools/bin/windows/d3dcompiler_47.dll ../../../third-party/tools/windows-x64/

cp pinmame/pinmame/build/${BUILD_TYPE}/pinmame64.lib ../../../third-party/build-libs/windows-x64
cp pinmame/pinmame/build/${BUILD_TYPE}/pinmame64.dll ../../../third-party/runtime-libs/windows-x64
mkdir -p ../../../third-party/include/pinmame
cp pinmame/pinmame/src/libpinmame/libpinmame.h ../../../third-party/include/pinmame
cp pinmame/pinmame/src/libpinmame/PinMAMEPlugin.h ../../../third-party/include/pinmame

mkdir -p ../../../third-party/memmaps
cp -r pinmame-data/memmaps/index.json pinmame-data/memmaps/maps pinmame-data/memmaps/platforms ../../../third-party/memmaps/
mkdir -p ../../../third-party/gamedefs
cp -r pinmame-data/gamedefs/catalog pinmame-data/gamedefs/machines pinmame-data/gamedefs/controllers ../../../third-party/gamedefs/

cp openxr/openxr/build/src/loader/${BUILD_TYPE}/openxr_loader64.lib ../../../third-party/build-libs/windows-x64
cp openxr/openxr/build/src/loader/${BUILD_TYPE}/openxr_loader64.dll ../../../third-party/runtime-libs/windows-x64
mkdir -p ../../../third-party/include/openxr
cp openxr/openxr/build/include/openxr/*.h ../../../third-party/include/openxr

cp libdmdutil/libdmdutil/build/${BUILD_TYPE}/dmdutil64.lib ../../../third-party/build-libs/windows-x64
cp libdmdutil/libdmdutil/build/${BUILD_TYPE}/dmdutil64.dll ../../../third-party/runtime-libs/windows-x64
cp -r libdmdutil/libdmdutil/include/DMDUtil ../../../third-party/include/
cp libdmdutil/libdmdutil/third-party/build-libs/win/x64/zedmd64.lib ../../../third-party/build-libs/windows-x64
cp libdmdutil/libdmdutil/third-party/runtime-libs/win/x64/zedmd64.dll ../../../third-party/runtime-libs/windows-x64
cp libdmdutil/libdmdutil/third-party/include/ZeDMD.h ../../../third-party/include
cp libserum/libserum/build/${BUILD_TYPE}/serum64.lib ../../../third-party/build-libs/windows-x64
cp libserum/libserum/build/${BUILD_TYPE}/serum64.dll ../../../third-party/runtime-libs/windows-x64
cp libserum/libserum/src/serum.h ../../../third-party/include
cp libserum/libserum/src/serum-decode.h ../../../third-party/include
cp libdmdutil/libdmdutil/third-party/build-libs/win/x64/libserialport64.lib ../../../third-party/build-libs/windows-x64
cp libdmdutil/libdmdutil/third-party/runtime-libs/win/x64/libserialport64-0.dll ../../../third-party/runtime-libs/windows-x64
cp libpupdmd/libpupdmd/build/${BUILD_TYPE}/pupdmd64.lib ../../../third-party/build-libs/windows-x64
cp libpupdmd/libpupdmd/build/${BUILD_TYPE}/pupdmd64.dll ../../../third-party/runtime-libs/windows-x64
cp libpupdmd/libpupdmd/src/pupdmd.h ../../../third-party/include
cp libdmdutil/libdmdutil/third-party/build-libs/win/x64/sockpp64.lib ../../../third-party/build-libs/windows-x64
cp libdmdutil/libdmdutil/third-party/runtime-libs/win/x64/sockpp64.dll ../../../third-party/runtime-libs/windows-x64
cp libdmdutil/libdmdutil/third-party/build-libs/win/x64/cargs64.lib ../../../third-party/build-libs/windows-x64
cp libdmdutil/libdmdutil/third-party/runtime-libs/win/x64/cargs64.dll ../../../third-party/runtime-libs/windows-x64
cp libvni/libvni/build/${BUILD_TYPE}/vni64.lib ../../../third-party/build-libs/windows-x64
cp libvni/libvni/build/${BUILD_TYPE}/vni64.dll ../../../third-party/runtime-libs/windows-x64
cp libvni/libvni/src/vni.h ../../../third-party/include

cp libaltsound/libaltsound/build/${BUILD_TYPE}/altsound64.lib ../../../third-party/build-libs/windows-x64
cp libaltsound/libaltsound/build/${BUILD_TYPE}/altsound64.dll ../../../third-party/runtime-libs/windows-x64
cp libaltsound/libaltsound/src/altsound.h ../../../third-party/include

cp libdof/libdof/build/${BUILD_TYPE}/dof64.lib ../../../third-party/build-libs/windows-x64
cp libdof/libdof/build/${BUILD_TYPE}/dof64.dll ../../../third-party/runtime-libs/windows-x64
cp -r libdof/libdof/include/DOF ../../../third-party/include/
cp libdof/libdof/third-party/build-libs/win/x64/libusb64-1.0.lib ../../../third-party/build-libs/windows-x64
cp libdof/libdof/third-party/runtime-libs/win/x64/libusb64-1.0.dll ../../../third-party/runtime-libs/windows-x64
cp libdof/libdof/third-party/build-libs/win/x64/hidapi64.lib ../../../third-party/build-libs/windows-x64
cp libdof/libdof/third-party/runtime-libs/win/x64/hidapi64.dll ../../../third-party/runtime-libs/windows-x64
cp libdof/libdof/third-party/build-libs/win/x64/libftdi164.lib ../../../third-party/build-libs/windows-x64
cp libdof/libdof/third-party/runtime-libs/win/x64/libftdi164.dll ../../../third-party/runtime-libs/windows-x64

for LIB in avcodec avformat avutil swresample swscale; do
   DIR="lib${LIB}"
   cp ffmpeg/ffmpeg/${DIR}/${LIB}64.lib ../../../third-party/build-libs/windows-x64
   cp ffmpeg/ffmpeg/${DIR}/${LIB}64-*.dll ../../../third-party/runtime-libs/windows-x64
   mkdir -p ../../../third-party/include/${DIR}
   cp ffmpeg/ffmpeg/${DIR}/*.h ../../../third-party/include/${DIR}
done

cp libwinevbs/libwinevbs/build/winevbs64.dll ../../../third-party/runtime-libs/windows-x64
mkdir -p ../../../third-party/include/libwinevbs
cp libwinevbs/libwinevbs/include/libwinevbs.h ../../../third-party/include/libwinevbs/

cp zlib/zlib/build/${BUILD_TYPE}/zlibstatic.lib ../../../third-party/build-libs/windows-x64

cp libmysofa/libmysofa/build/src/${BUILD_TYPE}/mysofa.lib ../../../third-party/build-libs/windows-x64
cp libmysofa/libmysofa/src/hrtf/mysofa.h ../../../third-party/include
cp libmysofa/libmysofa/src/hrtf/mysofa_export.h ../../../third-party/include
cp opus/install/lib/ogg.lib ../../../third-party/build-libs/windows-x64
cp opus/install/lib/opus.lib ../../../third-party/build-libs/windows-x64
cp opus/install/lib/opusfile.lib ../../../third-party/build-libs/windows-x64
mkdir -p ../../../third-party/include/ogg
cp opus/install/include/ogg/*.h ../../../third-party/include/ogg
cp opus/install/include/opus/*.h ../../../third-party/include

cp libspatialaudio/libspatialaudio/build/${BUILD_TYPE}/spatialaudio.lib ../../../third-party/build-libs/windows-x64
mkdir -p ../../../third-party/include/spatialaudio
cp -r libspatialaudio/libspatialaudio/include/*.h libspatialaudio/libspatialaudio/include/{adm,dsp,hrtf,kiss_fft} ../../../third-party/include/spatialaudio
cp libspatialaudio/libspatialaudio/build/include/SpatialaudioConfig.h ../../../third-party/include/spatialaudio

cp "${MSYS2_PATH}/ucrt64/bin/zlib1.dll" ../../../third-party/runtime-libs/windows-x64
cp "${MSYS2_PATH}/ucrt64/bin/libiconv-2.dll" ../../../third-party/runtime-libs/windows-x64
cp "${MSYS2_PATH}/ucrt64/bin/libwinpthread-1.dll" ../../../third-party/runtime-libs/windows-x64
cp "${MSYS2_PATH}/ucrt64/bin/liblzma-5.dll" ../../../third-party/runtime-libs/windows-x64
cp "${MSYS2_PATH}/ucrt64/bin/libbz2-1.dll" ../../../third-party/runtime-libs/windows-x64
cp "${MSYS2_PATH}/ucrt64/bin/libgcc_s_seh-1.dll" ../../../third-party/runtime-libs/windows-x64
cp "${MSYS2_PATH}/ucrt64/bin/libstdc++-6.dll" ../../../third-party/runtime-libs/windows-x64
