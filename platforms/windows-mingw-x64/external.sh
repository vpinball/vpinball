#!/bin/bash

set -e

source ./platforms/config.sh

NUM_PROCS=$(nproc)

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
echo "  LIBWINEVBS_SHA: ${LIBWINEVBS_SHA}"
echo "  LIBZIP_SHA: ${LIBZIP_SHA}"
echo ""

mkdir -p "external/windows-x64-mingw/${BUILD_TYPE}"
cd "external/windows-x64-mingw/${BUILD_TYPE}"

#
# build SDL3, SDL3_image, SDL3_ttf
#

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
      -DSDL_SHARED=ON \
      -DSDL_STATIC=OFF \
      -DSDL_TEST_LIBRARY=OFF \
      -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
      -B build
   cmake --build build -- -j${NUM_PROCS}
   cd ..

   curl -sL https://github.com/libsdl-org/SDL_image/archive/${SDL_IMAGE_SHA}.tar.gz -o SDL_image-${SDL_IMAGE_SHA}.tar.gz
   tar xzf SDL_image-${SDL_IMAGE_SHA}.tar.gz --exclude='*/Xcode/*'
   mv SDL_image-${SDL_IMAGE_SHA} SDL_image
   cd SDL_image
   sed -i.bak 's/OUTPUT_NAME "SDL3_image"/OUTPUT_NAME "SDL3_image64"/g' CMakeLists.txt
   ./external/download.sh
   cmake \
      -DBUILD_SHARED_LIBS=ON \
      -DSDLIMAGE_SAMPLES=OFF \
      -DSDLIMAGE_DEPS_SHARED=ON \
      -DSDLIMAGE_VENDORED=ON \
      -DSDLIMAGE_AVIF=OFF \
      -DSDLIMAGE_WEBP=OFF \
      -DSDL3_DIR=../SDL/build \
      -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
      -B build
   cmake --build build -- -j${NUM_PROCS}
   cd ..

   curl -sL https://github.com/libsdl-org/SDL_ttf/archive/${SDL_TTF_SHA}.tar.gz -o SDL_ttf-${SDL_TTF_SHA}.tar.gz
   tar xzf SDL_ttf-${SDL_TTF_SHA}.tar.gz --exclude='*/Xcode/*'
   mv SDL_ttf-${SDL_TTF_SHA} SDL_ttf
   cd SDL_ttf
   sed -i.bak 's/OUTPUT_NAME SDL3_ttf/OUTPUT_NAME SDL3_ttf64/g' CMakeLists.txt
   ./external/download.sh
   cmake \
      -DBUILD_SHARED_LIBS=ON \
      -DSDLTTF_SAMPLES=OFF \
      -DSDLTTF_VENDORED=ON \
      -DSDLTTF_HARFBUZZ=ON \
      -DSDL3_DIR=../SDL/build \
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
   sed -i.bak 's/set_target_properties(freeimage_shared PROPERTIES/set_target_properties(freeimage_shared PROPERTIES\n      PREFIX ""/' CMakeLists.txt
   echo 'target_link_libraries(freeimage_shared ws2_32)' >> CMakeLists.txt
   cmake \
      -DPLATFORM=win \
      -DARCH=x64 \
      -DBUILD_SHARED=ON \
      -DBUILD_STATIC=OFF \
      '-DCMAKE_C_FLAGS=-DWIN32_LEAN_AND_MEAN -D__MINGW64_TOOLCHAIN__' \
      '-DCMAKE_CXX_FLAGS=-DWIN32_LEAN_AND_MEAN' \
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

BGFX_EXPECTED_SHA="${BGFX_CMAKE_VERSION}-${BGFX_PATCH_SHA}-005"
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
   sed -i.bak 's/set_target_properties(bimg_decode PROPERTIES FOLDER "bgfx")/set_target_properties(bimg_decode PROPERTIES FOLDER "bgfx" OUTPUT_NAME "bimg_decode64")/g' cmake/bimg/bimg_decode.cmake
   sed -i.bak 's/set_target_properties(bimg_encode PROPERTIES FOLDER "bgfx")/set_target_properties(bimg_encode PROPERTIES FOLDER "bgfx" OUTPUT_NAME "bimg_encode64")/g' cmake/bimg/bimg_encode.cmake
   sed -i.bak 's/set_target_properties(bgfx PROPERTIES FOLDER "bgfx")/set_target_properties(bgfx PROPERTIES FOLDER "bgfx" OUTPUT_NAME "bgfx64")/g' cmake/bgfx/bgfx.cmake
   cmake -S. \
      -DBGFX_LIBRARY_TYPE=STATIC \
      -DBGFX_BUILD_TOOLS=OFF \
      -DBGFX_BUILD_EXAMPLES=OFF \
      -DBGFX_CONFIG_MULTITHREADED=ON \
      -DBGFX_CONFIG_MAX_FRAME_BUFFERS=256 \
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

PINMAME_EXPECTED_SHA="${PINMAME_SHA}-002"
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
   sed -i.bak 's/set_target_properties(pinmame_shared PROPERTIES/set_target_properties(pinmame_shared PROPERTIES\n      PREFIX ""/' CMakeLists.txt
   cmake \
      -DPLATFORM=win \
      -DARCH=x64 \
      -DBUILD_SHARED=ON \
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

OPENXR_EXPECTED_SHA="${OPENXR_SHA}-002"
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
   sed -i.bak 's/set_target_properties(openxr_loader PROPERTIES FOLDER ${LOADER_FOLDER})/set_target_properties(openxr_loader PROPERTIES FOLDER ${LOADER_FOLDER} OUTPUT_NAME "openxr_loader64" PREFIX "")/g' src/loader/CMakeLists.txt
   sed -i.bak 's|\${CMAKE_CURRENT_BINARY_DIR}/$<CONFIGURATION>/openxr_loader|\${CMAKE_CURRENT_BINARY_DIR}/$<CONFIGURATION>/openxr_loader64|g' src/loader/CMakeLists.txt
   cmake \
      -DBUILD_TESTS=OFF \
      -DDYNAMIC_LOADER=ON \
      -DOPENXR_DEBUG_POSTFIX='' \
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
   ./platforms/win-mingw/x64/external.sh
   cmake \
      -DPLATFORM=win-mingw \
      -DARCH=x64 \
      -DBUILD_SHARED=ON \
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
   cmake \
      -DPLATFORM=win-mingw \
      -DARCH=x64 \
      -DBUILD_SHARED=ON \
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
   ./platforms/win-mingw/x64/external.sh
   cmake \
      -DPLATFORM=win-mingw \
      -DARCH=x64 \
      -DBUILD_SHARED=ON \
      -DBUILD_STATIC=OFF \
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

LIBWINEVBS_EXPECTED_SHA="${LIBWINEVBS_SHA}-002"
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
      -DBUILD_SHARED=ON \
      -DPLATFORM=win-mingw \
      -DARCH=x64 \
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
   ./configure \
      --enable-shared \
      --disable-static \
      --disable-programs \
      --disable-doc \
      --arch="x86_64" \
      --build-suffix=64
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
   sed -i.bak 's/\(set_target_properties(zip PROPERTIES\)/\1 OUTPUT_NAME "zip64"/' lib/CMakeLists.txt
   cmake \
      -DBUILD_SHARED_LIBS=ON \
      -DBUILD_TOOLS=OFF \
      -DBUILD_REGRESS=OFF \
      -DBUILD_OSSFUZZ=OFF \
      -DBUILD_EXAMPLES=OFF \
      -DBUILD_DOC=OFF \
      -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
      -B build
   cmake --build build -- -j${NUM_PROCS}
   cd ..

   echo "$LIBZIP_EXPECTED_SHA" > cache.txt

   cd ..
fi

#
# copy libraries
#

cp SDL3/SDL/build/SDL364.dll ../../../third-party/runtime-libs/windows-mingw-x64
cp SDL3/SDL/build/libSDL364.dll.a ../../../third-party/build-libs/windows-mingw-x64
cp -r SDL3/SDL/include/SDL3 ../../../third-party/include/

cp SDL3/SDL_image/build/SDL3_image64.dll ../../../third-party/runtime-libs/windows-mingw-x64
cp SDL3/SDL_image/build/libSDL3_image64.dll.a ../../../third-party/build-libs/windows-mingw-x64
cp -r SDL3/SDL_image/include/SDL3_image ../../../third-party/include/

cp SDL3/SDL_ttf/build/SDL3_ttf64.dll ../../../third-party/runtime-libs/windows-mingw-x64
cp SDL3/SDL_ttf/build/libSDL3_ttf64.dll.a ../../../third-party/build-libs/windows-mingw-x64
cp -r SDL3/SDL_ttf/include/SDL3_ttf ../../../third-party/include/

cp freeimage/freeimage/build/freeimage64.dll ../../../third-party/runtime-libs/windows-mingw-x64
cp freeimage/freeimage/build/libfreeimage64.dll.a ../../../third-party/build-libs/windows-mingw-x64
cp freeimage/freeimage/Source/FreeImage.h ../../../third-party/include

cp -r bgfx/bgfx.cmake/bgfx/include/bgfx ../../../third-party/include/
cp -r bgfx/bgfx.cmake/bimg/include/bimg ../../../third-party/include/
cp -r bgfx/bgfx.cmake/bx/include/bx ../../../third-party/include/
cp bgfx/bgfx.cmake/build/cmake/bgfx/libbgfx64.a ../../../third-party/build-libs/windows-mingw-x64
cp bgfx/bgfx.cmake/build/cmake/bimg/libbimg64.a ../../../third-party/build-libs/windows-mingw-x64
cp bgfx/bgfx.cmake/build/cmake/bimg/libbimg_decode64.a ../../../third-party/build-libs/windows-mingw-x64
cp bgfx/bgfx.cmake/build/cmake/bimg/libbimg_encode64.a ../../../third-party/build-libs/windows-mingw-x64
cp bgfx/bgfx.cmake/build/cmake/bx/libbx64.a ../../../third-party/build-libs/windows-mingw-x64

cp pinmame/pinmame/build/pinmame64.dll ../../../third-party/runtime-libs/windows-mingw-x64
cp pinmame/pinmame/build/libpinmame64.dll.a ../../../third-party/build-libs/windows-mingw-x64
cp pinmame/pinmame/src/libpinmame/libpinmame.h ../../../third-party/include

cp openxr/openxr/build/src/loader/openxr_loader64.dll ../../../third-party/runtime-libs/windows-mingw-x64
cp openxr/openxr/build/src/loader/libopenxr_loader64.dll.a ../../../third-party/build-libs/windows-mingw-x64
cp -r openxr/openxr/include/openxr ../../../third-party/include

cp libdmdutil/libdmdutil/build/dmdutil64.dll ../../../third-party/runtime-libs/windows-mingw-x64
cp libdmdutil/libdmdutil/build/dmdutil64.dll.a ../../../third-party/build-libs/windows-mingw-x64
cp -r libdmdutil/libdmdutil/include/DMDUtil ../../../third-party/include/
cp libdmdutil/libdmdutil/third-party/runtime-libs/win-mingw/x64/zedmd64.dll ../../../third-party/runtime-libs/windows-mingw-x64
cp libdmdutil/libdmdutil/third-party/build-libs/win-mingw/x64/zedmd64.dll.a ../../../third-party/build-libs/windows-mingw-x64
cp libdmdutil/libdmdutil/third-party/include/ZeDMD.h ../../../third-party/include
cp libdmdutil/libdmdutil/third-party/runtime-libs/win-mingw/x64/serum64.dll ../../../third-party/runtime-libs/windows-mingw-x64
cp libdmdutil/libdmdutil/third-party/build-libs/win-mingw/x64/serum64.dll.a ../../../third-party/build-libs/windows-mingw-x64
cp libdmdutil/libdmdutil/third-party/include/serum.h ../../../third-party/include
cp libdmdutil/libdmdutil/third-party/include/serum-decode.h ../../../third-party/include
cp libdmdutil/libdmdutil/third-party/runtime-libs/win-mingw/x64/libserialport64-0.dll ../../../third-party/runtime-libs/windows-mingw-x64
cp libdmdutil/libdmdutil/third-party/runtime-libs/win-mingw/x64/pupdmd64.dll ../../../third-party/runtime-libs/windows-mingw-x64
cp libdmdutil/libdmdutil/third-party/build-libs/win-mingw/x64/pupdmd64.dll.a ../../../third-party/build-libs/windows-mingw-x64
cp libdmdutil/libdmdutil/third-party/include/pupdmd.h ../../../third-party/include
cp libdmdutil/libdmdutil/third-party/runtime-libs/win-mingw/x64/libsockpp64.dll ../../../third-party/runtime-libs/windows-mingw-x64
cp libdmdutil/libdmdutil/third-party/runtime-libs/win-mingw/x64/libcargs64.dll ../../../third-party/runtime-libs/windows-mingw-x64
cp libdmdutil/libdmdutil/third-party/runtime-libs/win-mingw/x64/vni64.dll ../../../third-party/runtime-libs/windows-mingw-x64
cp libdmdutil/libdmdutil/third-party/build-libs/win-mingw/x64/vni64.dll.a ../../../third-party/build-libs/windows-mingw-x64
cp libdmdutil/libdmdutil/third-party/include/vni.h ../../../third-party/include
cp libdmdutil/libdmdutil/third-party/runtime-libs/win-mingw/x64/libusb64-1.0.dll ../../../third-party/runtime-libs/windows-mingw-x64

cp libaltsound/libaltsound/build/altsound64.dll ../../../third-party/runtime-libs/windows-mingw-x64
cp libaltsound/libaltsound/build/altsound64.dll.a ../../../third-party/build-libs/windows-mingw-x64
cp libaltsound/libaltsound/src/altsound.h ../../../third-party/include

cp libdof/libdof/build/dof64.dll ../../../third-party/runtime-libs/windows-mingw-x64
cp libdof/libdof/build/dof64.dll.a ../../../third-party/build-libs/windows-mingw-x64
cp -r libdof/libdof/include/DOF ../../../third-party/include/
cp libdof/libdof/third-party/runtime-libs/win-mingw/x64/libusb64-1.0.dll ../../../third-party/runtime-libs/windows-mingw-x64
cp libdof/libdof/third-party/runtime-libs/win-mingw/x64/hidapi64.dll ../../../third-party/runtime-libs/windows-mingw-x64
cp libdof/libdof/third-party/runtime-libs/win-mingw/x64/libftdi164.dll ../../../third-party/runtime-libs/windows-mingw-x64
cp libdof/libdof/third-party/runtime-libs/win-mingw/x64/libserialport64-0.dll ../../../third-party/runtime-libs/windows-mingw-x64

cp libwinevbs/libwinevbs/build/winevbs64.dll ../../../third-party/runtime-libs/windows-mingw-x64
cp libwinevbs/libwinevbs/build/libwinevbs64.dll.a ../../../third-party/build-libs/windows-mingw-x64
mkdir -p ../../../third-party/include/libwinevbs/wine/include
mkdir -p ../../../third-party/include/libwinevbs/atl/include
mkdir -p ../../../third-party/include/libwinevbs/atlmfc/include
cp libwinevbs/libwinevbs/include/libwinevbs.h ../../../third-party/include/libwinevbs/
cp -r libwinevbs/libwinevbs/wine/include/* ../../../third-party/include/libwinevbs/wine/include/
cp -r libwinevbs/libwinevbs/atl/include/* ../../../third-party/include/libwinevbs/atl/include/
cp -r libwinevbs/libwinevbs/atlmfc/include/* ../../../third-party/include/libwinevbs/atlmfc/include/

for LIB in avcodec avdevice avfilter avformat avutil swresample swscale; do
   DIR="lib${LIB}"
   cp ffmpeg/ffmpeg/${DIR}/${LIB}64.lib ../../../third-party/build-libs/windows-mingw-x64
   cp ffmpeg/ffmpeg/${DIR}/lib${LIB}64.dll.a ../../../third-party/build-libs/windows-mingw-x64
   cp ffmpeg/ffmpeg/${DIR}/${LIB}64.dll ../../../third-party/runtime-libs/windows-mingw-x64
   mkdir -p ../../../third-party/include/${DIR}
   cp ffmpeg/ffmpeg/${DIR}/*.h ../../../third-party/include/${DIR}
done

UCRT64_BIN="${MINGW_PREFIX}/bin"
cp "${UCRT64_BIN}/zlib1.dll" ../../../third-party/runtime-libs/windows-mingw-x64
cp "${UCRT64_BIN}/libiconv-2.dll" ../../../third-party/runtime-libs/windows-mingw-x64
cp "${UCRT64_BIN}/libwinpthread-1.dll" ../../../third-party/runtime-libs/windows-mingw-x64
cp "${UCRT64_BIN}/liblzma-5.dll" ../../../third-party/runtime-libs/windows-mingw-x64
cp "${UCRT64_BIN}/libbz2-1.dll" ../../../third-party/runtime-libs/windows-mingw-x64
cp "${UCRT64_BIN}/libgcc_s_seh-1.dll" ../../../third-party/runtime-libs/windows-mingw-x64
cp "${UCRT64_BIN}/libstdc++-6.dll" ../../../third-party/runtime-libs/windows-mingw-x64

cp libzip/libzip/build/lib/libzip64.dll ../../../third-party/runtime-libs/windows-mingw-x64
cp libzip/libzip/build/lib/libzip64.dll.a ../../../third-party/build-libs/windows-mingw-x64
cp libzip/libzip/build/zipconf.h ../../../third-party/include
cp libzip/libzip/lib/zip.h ../../../third-party/include
