#!/bin/bash

set -e

source ./platforms/config.sh

if [ -z "${MSYS2_PATH}" ]; then
   MSYS2_PATH="/c/msys64"
fi

export MSYSTEM=MINGW32

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
echo "  OPENXR_SHA: ${OPENXR_SHA}"
echo "  LIBDMDUTIL_SHA: ${LIBDMDUTIL_SHA}"
echo "  LIBALTSOUND_SHA: ${LIBALTSOUND_SHA}"
echo "  LIBDOF_SHA: ${LIBDOF_SHA}"
echo "  FFMPEG_SHA: ${FFMPEG_SHA}"
echo "  LIBMYSOFA_SHA: ${LIBMYSOFA_SHA}"
echo "  LIBSPATIALAUDIO_SHA: ${LIBSPATIALAUDIO_SHA}"
echo "  ZLIB_SHA: ${ZLIB_SHA}"
echo ""

mkdir -p "external/windows-x86/${BUILD_TYPE}"
cd "external/windows-x86/${BUILD_TYPE}"

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
   cmake \
      -G "Visual Studio 18 2026" \
      -A Win32 \
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
   ./external/download.sh
   cmake \
      -G "Visual Studio 18 2026" \
      -A Win32 \
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
   ./external/download.sh
   cmake \
      -G "Visual Studio 18 2026" \
      -A Win32 \
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
      -A Win32 \
      -DPLATFORM=win \
      -DARCH=x86 \
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
   cmake -G "Visual Studio 18 2026" \
      -S. \
      -A Win32 \
      -DBGFX_LIBRARY_TYPE=STATIC \
      -DBGFX_BUILD_TOOLS=OFF \
      -DBGFX_BUILD_EXAMPLES=OFF \
      -DBGFX_CONFIG_MULTITHREADED=ON \
      -DBGFX_CONFIG_MAX_FRAME_BUFFERS=256 \
      -DCMAKE_MSVC_RUNTIME_LIBRARY="MultiThreaded\$<\$<CONFIG:Debug>:Debug>" \
      -B build
   cmake --build build --config ${BUILD_TYPE}
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
      -A Win32 \
      -DPLATFORM=win \
      -DARCH=x86 \
      -DBUILD_SHARED=ON \
      -DBUILD_STATIC=OFF \
      -B build
   cmake --build build --config ${BUILD_TYPE}
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

   curl -sL https://github.com/KhronosGroup/OpenXR-SDK-Source/archive/${OPENXR_SHA}.tar.gz -o OpenXR-SDK-Source-${OPENXR_SHA}.tar.gz
   tar xzf OpenXR-SDK-Source-${OPENXR_SHA}.tar.gz
   mv OpenXR-SDK-Source-${OPENXR_SHA} openxr
   cd openxr
   cmake \
      -G "Visual Studio 18 2026" \
      -A Win32 \
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
   ./platforms/win/x86/external.sh
   cmake \
      -G "Visual Studio 18 2026" \
      -A Win32 \
      -DPLATFORM=win \
      -DARCH=x86 \
      -DBUILD_SHARED=ON \
      -DBUILD_STATIC=OFF \
      -B build
   cmake --build build --config ${BUILD_TYPE}
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
      -G "Visual Studio 18 2026" \
      -A Win32 \
      -DPLATFORM=win \
      -DARCH=x86 \
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
   ./platforms/win/x86/external.sh
   cmake \
      -G "Visual Studio 18 2026" \
      -A Win32 \
      -DPLATFORM=win \
      -DARCH=x86 \
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
         --arch=\"x86\" \
         --extra-cflags=\"-m32\" \
         --extra-ldflags=\"-m32\" &&
      make -j$(nproc)
   "
   cd ..

   echo "$FFMPEG_EXPECTED_SHA" > cache.txt

   cd ..
fi


#
# build zlib (static library needed by libmysofa, static runtime to match the app)
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
      -G "Visual Studio 18 2026" \
      -A Win32 \
      -DBUILD_SHARED_LIBS=OFF \
      -DZLIB_BUILD_EXAMPLES=OFF \
      -DCMAKE_MSVC_RUNTIME_LIBRARY="MultiThreaded\$<\$<CONFIG:Debug>:Debug>" \
      -DCMAKE_INSTALL_PREFIX=install \
      -B build
   cmake --build build --config ${BUILD_TYPE}
   cmake --install build --config ${BUILD_TYPE}
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
   # MSYS tar cannot create the archive's .sofa symlinks; skip them and provide
   # share/default.sofa (a symlink in the archive) as a real file for install
   tar xzf libmysofa-${LIBMYSOFA_SHA}.tar.gz --exclude='*.sofa'
   tar xzf libmysofa-${LIBMYSOFA_SHA}.tar.gz libmysofa-${LIBMYSOFA_SHA}/share/MIT_KEMAR_normal_pinna.sofa
   mv libmysofa-${LIBMYSOFA_SHA} libmysofa
   cp libmysofa/share/MIT_KEMAR_normal_pinna.sofa libmysofa/share/default.sofa
   cd libmysofa
   # Patch: replace the MSVC-only nuget zlib fetch (hardcoded path, unlinked)
   # by a standard FindZLIB using the zlib built above
   sed -i.bak '/find_program(NUGET nuget)/,/zlib-1.3.1\/include\/)/c\  include(FindZLIB)\n  include_directories(${ZLIB_INCLUDE_DIRS})' src/CMakeLists.txt
   cmake \
      -G "Visual Studio 18 2026" \
      -A Win32 \
      -DBUILD_SHARED_LIBS=OFF \
      -DBUILD_STATIC_LIBS=ON \
      -DBUILD_TESTS=OFF \
      -DCMAKE_MSVC_RUNTIME_LIBRARY="MultiThreaded\$<\$<CONFIG:Debug>:Debug>" \
      -DCMAKE_INSTALL_PREFIX=install \
      -DZLIB_INCLUDE_DIR="$(cygpath -m "$PWD/../../zlib/zlib/install/include")" \
      -DZLIB_LIBRARY="$(cygpath -m "$(/usr/bin/find ../../zlib/zlib/install/lib -name 'zlibstatic*.lib' | head -n 1)")" \
      -B build
   cmake --build build --config ${BUILD_TYPE}
   cmake --install build --config ${BUILD_TYPE}
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
      -G "Visual Studio 18 2026" \
      -A Win32 \
      -DCMAKE_MSVC_RUNTIME_LIBRARY="MultiThreaded\$<\$<CONFIG:Debug>:Debug>" \
      -DBUILD_SHARED_LIBS=OFF \
      -DBUILD_TESTING=OFF \
      -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
      -DCMAKE_INSTALL_PREFIX=install \
      -DMYSOFA_INCLUDE_DIRS="$(cygpath -m "$PWD/../libmysofa/libmysofa/install/include")" \
      -DMYSOFA_LIBRARIES="$(cygpath -m "$PWD/../libmysofa/libmysofa/install/lib/mysofa.lib")" \
      -B build
   cmake --build build --config ${BUILD_TYPE}
   cmake --install build --config ${BUILD_TYPE}
   cd ..

   echo "$LIBSPATIALAUDIO_EXPECTED_SHA" > cache.txt

   cd ..
fi
#
# copy libraries
#

cp SDL3/SDL/build/${BUILD_TYPE}/SDL3.lib ../../../third-party/build-libs/windows-x86
cp SDL3/SDL/build/${BUILD_TYPE}/SDL3.dll ../../../third-party/runtime-libs/windows-x86
cp -r SDL3/SDL/include/SDL3 ../../../third-party/include/

cp SDL3/SDL_image/build/${BUILD_TYPE}/SDL3_image.lib ../../../third-party/build-libs/windows-x86
cp SDL3/SDL_image/build/${BUILD_TYPE}/SDL3_image.dll ../../../third-party/runtime-libs/windows-x86
cp -r SDL3/SDL_image/include/SDL3_image ../../../third-party/include/

cp SDL3/SDL_ttf/build/${BUILD_TYPE}/SDL3_ttf.lib ../../../third-party/build-libs/windows-x86
cp SDL3/SDL_ttf/build/${BUILD_TYPE}/SDL3_ttf.dll ../../../third-party/runtime-libs/windows-x86
cp -r SDL3/SDL_ttf/include/SDL3_ttf ../../../third-party/include/

cp freeimage/freeimage/build/${BUILD_TYPE}/freeimage.lib ../../../third-party/build-libs/windows-x86
cp freeimage/freeimage/build/${BUILD_TYPE}/freeimage.dll ../../../third-party/runtime-libs/windows-x86
cp freeimage/freeimage/Source/FreeImage.h ../../../third-party/include

cp bgfx/bgfx.cmake/build/cmake/bgfx/${BUILD_TYPE}/bgfx.lib ../../../third-party/build-libs/windows-x86
cp -r bgfx/bgfx.cmake/bgfx/include/bgfx ../../../third-party/include/
cp bgfx/bgfx.cmake/build/cmake/bimg/${BUILD_TYPE}/bimg.lib ../../../third-party/build-libs/windows-x86
cp bgfx/bgfx.cmake/build/cmake/bimg/${BUILD_TYPE}/bimg_encode.lib ../../../third-party/build-libs/windows-x86
cp -r bgfx/bgfx.cmake/bimg/include/bimg ../../../third-party/include/
cp bgfx/bgfx.cmake/build/cmake/bx/${BUILD_TYPE}/bx.lib ../../../third-party/build-libs/windows-x86
cp -r bgfx/bgfx.cmake/bx/include/bx ../../../third-party/include/

cp pinmame/pinmame/build/${BUILD_TYPE}/pinmame.lib ../../../third-party/build-libs/windows-x86
cp pinmame/pinmame/build/${BUILD_TYPE}/pinmame.dll ../../../third-party/runtime-libs/windows-x86
mkdir -p ../../../third-party/include/pinmame
cp pinmame/pinmame/src/libpinmame/libpinmame.h ../../../third-party/include/pinmame
cp pinmame/pinmame/src/libpinmame/PinMAMEPlugin.h ../../../third-party/include/pinmame

cp openxr/openxr/build/src/loader/${BUILD_TYPE}/openxr_loader.lib ../../../third-party/build-libs/windows-x86
cp openxr/openxr/build/src/loader/${BUILD_TYPE}/openxr_loader.dll ../../../third-party/runtime-libs/windows-x86
mkdir -p ../../../third-party/include/openxr
cp openxr/openxr/build/include/openxr/*.h ../../../third-party/include/openxr

cp libdmdutil/libdmdutil/build/${BUILD_TYPE}/dmdutil.lib ../../../third-party/build-libs/windows-x86
cp libdmdutil/libdmdutil/build/${BUILD_TYPE}/dmdutil.dll ../../../third-party/runtime-libs/windows-x86
cp -r libdmdutil/libdmdutil/include/DMDUtil ../../../third-party/include/
cp libdmdutil/libdmdutil/third-party/build-libs/win/x86/zedmd.lib ../../../third-party/build-libs/windows-x86
cp libdmdutil/libdmdutil/third-party/runtime-libs/win/x86/zedmd.dll ../../../third-party/runtime-libs/windows-x86
cp libdmdutil/libdmdutil/third-party/include/ZeDMD.h ../../../third-party/include
cp libdmdutil/libdmdutil/third-party/build-libs/win/x86/serum.lib ../../../third-party/build-libs/windows-x86
cp libdmdutil/libdmdutil/third-party/runtime-libs/win/x86/serum.dll ../../../third-party/runtime-libs/windows-x86
cp libdmdutil/libdmdutil/third-party/include/serum.h ../../../third-party/include
cp libdmdutil/libdmdutil/third-party/include/serum-decode.h ../../../third-party/include
cp libdmdutil/libdmdutil/third-party/build-libs/win/x86/libserialport.lib ../../../third-party/build-libs/windows-x86
cp libdmdutil/libdmdutil/third-party/runtime-libs/win/x86/libserialport-0.dll ../../../third-party/runtime-libs/windows-x86
cp libdmdutil/libdmdutil/third-party/build-libs/win/x86/pupdmd.lib ../../../third-party/build-libs/windows-x86
cp libdmdutil/libdmdutil/third-party/runtime-libs/win/x86/pupdmd.dll ../../../third-party/runtime-libs/windows-x86
cp libdmdutil/libdmdutil/third-party/include/pupdmd.h ../../../third-party/include
cp libdmdutil/libdmdutil/third-party/build-libs/win/x86/sockpp.lib ../../../third-party/build-libs/windows-x86
cp libdmdutil/libdmdutil/third-party/runtime-libs/win/x86/sockpp.dll ../../../third-party/runtime-libs/windows-x86
cp libdmdutil/libdmdutil/third-party/build-libs/win/x86/cargs.lib ../../../third-party/build-libs/windows-x86
cp libdmdutil/libdmdutil/third-party/runtime-libs/win/x86/cargs.dll ../../../third-party/runtime-libs/windows-x86
cp libdmdutil/libdmdutil/third-party/build-libs/win/x86/vni.lib ../../../third-party/build-libs/windows-x86
cp libdmdutil/libdmdutil/third-party/runtime-libs/win/x86/vni.dll ../../../third-party/runtime-libs/windows-x86
cp libdmdutil/libdmdutil/third-party/include/vni.h ../../../third-party/include

cp libaltsound/libaltsound/build/${BUILD_TYPE}/altsound.lib ../../../third-party/build-libs/windows-x86
cp libaltsound/libaltsound/build/${BUILD_TYPE}/altsound.dll ../../../third-party/runtime-libs/windows-x86
cp libaltsound/libaltsound/src/altsound.h ../../../third-party/include

cp libdof/libdof/build/${BUILD_TYPE}/dof.lib ../../../third-party/build-libs/windows-x86
cp libdof/libdof/build/${BUILD_TYPE}/dof.dll ../../../third-party/runtime-libs/windows-x86
cp -r libdof/libdof/include/DOF ../../../third-party/include/
cp libdof/libdof/third-party/build-libs/win/x86/libusb-1.0.lib ../../../third-party/build-libs/windows-x86
cp libdof/libdof/third-party/runtime-libs/win/x86/libusb-1.0.dll ../../../third-party/runtime-libs/windows-x86
cp libdof/libdof/third-party/build-libs/win/x86/hidapi.lib ../../../third-party/build-libs/windows-x86
cp libdof/libdof/third-party/runtime-libs/win/x86/hidapi.dll ../../../third-party/runtime-libs/windows-x86
cp libdof/libdof/third-party/build-libs/win/x86/libftdi1.lib ../../../third-party/build-libs/windows-x86
cp libdof/libdof/third-party/runtime-libs/win/x86/libftdi1.dll ../../../third-party/runtime-libs/windows-x86

for LIB in avcodec avformat avutil swresample swscale; do
   DIR="lib${LIB}"
   cp ffmpeg/ffmpeg/${DIR}/${LIB}.lib ../../../third-party/build-libs/windows-x86
   cp ffmpeg/ffmpeg/${DIR}/${LIB}-*.dll ../../../third-party/runtime-libs/windows-x86
   mkdir -p ../../../third-party/include/${DIR}
   cp ffmpeg/ffmpeg/${DIR}/*.h ../../../third-party/include/${DIR}
done

cp "${MSYS2_PATH}/mingw32/bin/zlib1.dll" ../../../third-party/runtime-libs/windows-x86
cp "${MSYS2_PATH}/mingw32/bin/libiconv-2.dll" ../../../third-party/runtime-libs/windows-x86
cp "${MSYS2_PATH}/mingw32/bin/libwinpthread-1.dll" ../../../third-party/runtime-libs/windows-x86
cp "${MSYS2_PATH}/mingw32/bin/liblzma-5.dll" ../../../third-party/runtime-libs/windows-x86
cp "${MSYS2_PATH}/mingw32/bin/libbz2-1.dll" ../../../third-party/runtime-libs/windows-x86
cp "${MSYS2_PATH}/mingw32/bin/libgcc_s_dw2-1.dll" ../../../third-party/runtime-libs/windows-x86
cp "${MSYS2_PATH}/mingw32/bin/libstdc++-6.dll" ../../../third-party/runtime-libs/windows-x86

cp zlib/zlib/install/lib/zlibstatic*.lib ../../../third-party/build-libs/windows-x86/zlibstatic.lib

cp libmysofa/libmysofa/install/lib/mysofa.lib ../../../third-party/build-libs/windows-x86
cp libmysofa/libmysofa/install/include/mysofa.h ../../../third-party/include/
cp libmysofa/libmysofa/install/include/mysofa_export.h ../../../third-party/include/

cp libspatialaudio/libspatialaudio/build/${BUILD_TYPE}/spatialaudio.lib ../../../third-party/build-libs/windows-x86
cp -r libspatialaudio/libspatialaudio/install/include/spatialaudio ../../../third-party/include/
