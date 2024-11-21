#!/bin/bash

set -e

FREEIMAGE_VERSION=3.18.0
SDL_SHA=78cc5c173404488d80751af226d1eaf67033bcc4
SDL_IMAGE_SHA=b1c8ec7d75e3d8398940c9e04a8b82886ae6163d
SDL_TTF_SHA=f20defe45dfe6f0daa0f8e92e8b6221d1be3d9c0
PINMAME_SHA=bca3784c3b4d8870565422d5b5fce6b9e40e1320
LIBALTSOUND_SHA=b8f397858cbc7a879f7392c14a509f00c8bdc7dd
LIBDMDUTIL_SHA=819d67ac33e0ff5c339bc372b6e11a0caf07d31b
LIBDOF_SHA=5c43c99ea28b44bb58b74554c4303a505e208148
FFMPEG_SHA=b08d7969c550a804a59511c7b83f2dd8cc0499b8
BGFX_CMAKE_VERSION=1.128.8808-482
BGFX_PATCH_SHA=a0d4c179527a4a4d205598ebf290c0b45144bda8

if [[ $(uname) == "Linux" ]]; then
   NUM_PROCS=$(nproc)
elif [[ $(uname) == "Darwin" ]]; then
   NUM_PROCS=$(sysctl -n hw.ncpu)
else
   NUM_PROCS=1
fi

echo "Building external libraries..."
echo "  FREEIMAGE_VERSION: ${FREEIMAGE_VERSION}"
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
# build freeimage, and copy to external
#

CACHE_NAME="FreeImage-${FREEIMAGE_VERSION}"

if [ ! -f "../${CACHE_DIR}/${CACHE_NAME}.cache" ]; then
   FREEIMAGE_BASENAME="FreeImage${FREEIMAGE_VERSION//./}"
   curl -sL https://downloads.sourceforge.net/project/freeimage/Source%20Distribution/${FREEIMAGE_VERSION}/${FREEIMAGE_BASENAME}.zip -o ${FREEIMAGE_BASENAME}.zip
   unzip ${FREEIMAGE_BASENAME}.zip
   cd FreeImage
   cp ../../freeimage/Android.mk Android.mk
   $ANDROID_NDK_HOME/ndk-build \
      NDK_PROJECT_PATH=$ANDROID_NDK_HOME \
      APP_BUILD_SCRIPT=Android.mk \
      APP_STL="c++_static" \
      APP_PLATFORM=android-30 \
      APP_ABI=arm64-v8a \
      NDK_OUT=obj \
      NDK_LIBS_OUT=libs \
      -j${NUM_PROC}
   mkdir -p ../../${CACHE_DIR}/${CACHE_NAME}/lib
   cp libs/arm64-v8a/*.so ../../${CACHE_DIR}/${CACHE_NAME}/lib
   cd ..
   touch "../${CACHE_DIR}/${CACHE_NAME}.cache"
fi

cp ../${CACHE_DIR}/${CACHE_NAME}/lib/*.so ../external/lib

#
# download bass24 and copy to external
#

CACHE_NAME="bass24_003"

if [ ! -f "../${CACHE_DIR}/${CACHE_NAME}.cache" ]; then
   curl -sL https://www.un4seen.com/files/bass24-android.zip -o bass.zip
   unzip bass.zip
   mkdir -p ../${CACHE_DIR}/${CACHE_NAME}/lib
   cp libs/arm64-v8a/libbass.so ../${CACHE_DIR}/${CACHE_NAME}/lib
   touch "../${CACHE_DIR}/${CACHE_NAME}.cache"
fi

cp -a ../${CACHE_DIR}/${CACHE_NAME}/lib/*.so ../external/lib

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
      -DSDL_DISABLE_ANDROID_JAR=OFF \
      -DSDL_CAMERA=OFF \
      -DCMAKE_SYSTEM_NAME=Android \
      -DCMAKE_SYSTEM_VERSION=30 \
      -DCMAKE_ANDROID_ARCH_ABI=arm64-v8a \
      -DANDROID_NDK=${ANDROID_NDK_HOME} \
      -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
      -B build
   cmake --build build -- -j${NUM_PROCS}
   mkdir -p ../../${CACHE_DIR}/${CACHE_NAME}/include/SDL3
   cp -r include/SDL3/* ../../${CACHE_DIR}/${CACHE_NAME}/include/SDL3
   mkdir -p ../../${CACHE_DIR}/${CACHE_NAME}/lib
   cp -a build/*.so ../../${CACHE_DIR}/${CACHE_NAME}/lib
   cp -a build/*.jar ../../${CACHE_DIR}/${CACHE_NAME}/lib
   rm build/SDL3jarTargets.cmake
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
      -DCMAKE_SYSTEM_NAME=Android \
      -DCMAKE_SYSTEM_VERSION=30 \
      -DCMAKE_ANDROID_ARCH_ABI=arm64-v8a \
      -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
      -B build
   cmake --build build -- -j${NUM_PROCS}
   mkdir -p ../../${CACHE_DIR}/${CACHE_NAME}/include/SDL3_image
   cp -r include/SDL3_image/* ../../${CACHE_DIR}/${CACHE_NAME}/include/SDL3_image
   mkdir -p ../../${CACHE_DIR}/${CACHE_NAME}/include/lib
   cp -a build/*.so ../../${CACHE_DIR}/${CACHE_NAME}/lib
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
      -DCMAKE_SYSTEM_NAME=Android \
      -DCMAKE_SYSTEM_VERSION=30 \
      -DCMAKE_ANDROID_ARCH_ABI=arm64-v8a \
      -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
      -B build
   cmake --build build -- -j${NUM_PROCS}
   mkdir -p ../../${CACHE_DIR}/${CACHE_NAME}/include/SDL3_ttf
   cp -r include/SDL3_ttf/* ../../${CACHE_DIR}/${CACHE_NAME}/include/SDL3_ttf
   mkdir -p ../../${CACHE_DIR}/${CACHE_NAME}/include/lib
   cp -a build/*.so ../../${CACHE_DIR}/${CACHE_NAME}/lib
   cd ..

   touch "../${CACHE_DIR}/${CACHE_NAME}.cache"
fi

mkdir -p ../external/include/SDL3
cp -r ../${CACHE_DIR}/${CACHE_NAME}/include/SDL3/* ../external/include/SDL3
mkdir -p ../external/include/SDL3_image
cp -r ../${CACHE_DIR}/${CACHE_NAME}/include/SDL3_image/* ../external/include/SDL3_image
mkdir -p ../external/include/SDL3_ttf
cp -r ../${CACHE_DIR}/${CACHE_NAME}/include/SDL3_ttf/* ../external/include/SDL3_ttf
cp -a ../${CACHE_DIR}/${CACHE_NAME}/lib/*.so ../external/lib
cp -a ../${CACHE_DIR}/${CACHE_NAME}/lib/*.jar ../external/lib

#
# build libpinmame and copy to external
#

CACHE_NAME="pinmame-${PINMAME_SHA}"

if [ ! -f "../${CACHE_DIR}/${CACHE_NAME}.cache" ]; then
   curl -sL https://github.com/vpinball/pinmame/archive/${PINMAME_SHA}.zip -o pinmame.zip
   unzip pinmame.zip
   cd pinmame-$PINMAME_SHA
   cp cmake/libpinmame/CMakeLists_android-arm64-v8a.txt CMakeLists.txt
   cmake \
      -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
      -B build
   cmake --build build -- -j${NUM_PROCS}
   mkdir -p ../../${CACHE_DIR}/${CACHE_NAME}/include
   cp src/libpinmame/libpinmame.h ../../${CACHE_DIR}/${CACHE_NAME}/include
   mkdir -p ../../${CACHE_DIR}/${CACHE_NAME}/lib
   cp build/libpinmame.3.6.so ../../${CACHE_DIR}/${CACHE_NAME}/lib
   cd ..
   touch "../${CACHE_DIR}/${CACHE_NAME}.cache"
fi

cp -r ../${CACHE_DIR}/${CACHE_NAME}/include/* ../external/include
cp -a ../${CACHE_DIR}/${CACHE_NAME}/lib/*.so ../external/lib

#
# build libaltsound and copy to external
#

CACHE_NAME="libaltsound-${LIBALTSOUND_SHA}"

if [ ! -f  "../${CACHE_DIR}/${CACHE_NAME}.cache" ]; then
   curl -sL https://github.com/vpinball/libaltsound/archive/${LIBALTSOUND_SHA}.zip -o libaltsound.zip
   unzip libaltsound.zip
   cd libaltsound-$LIBALTSOUND_SHA
   platforms/android/arm64-v8a/external.sh
   cmake \
      -DPLATFORM=android \
      -DARCH=arm64-v8a \
      -DBUILD_STATIC=OFF \
      -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
      -B build
   cmake --build build -- -j${NUM_PROCS}
   mkdir -p ../../${CACHE_DIR}/${CACHE_NAME}/include
   cp src/altsound.h ../../${CACHE_DIR}/${CACHE_NAME}/include
   mkdir -p ../../${CACHE_DIR}/${CACHE_NAME}/lib
   cp -a build/*.so ../../${CACHE_DIR}/${CACHE_NAME}/lib
   cd ..
   touch "../${CACHE_DIR}/${CACHE_NAME}.cache"
fi

cp -r ../${CACHE_DIR}/${CACHE_NAME}/include/* ../external/include
cp -a ../${CACHE_DIR}/${CACHE_NAME}/lib/*.so ../external/lib

#
# build libdmdutil (and deps) and copy to external
#

CACHE_NAME="libdmdutil-${LIBDMDUTIL_SHA}"

if [ ! -f "../${CACHE_DIR}/${CACHE_NAME}.cache" ]; then
   curl -sL https://github.com/vpinball/libdmdutil/archive/${LIBDMDUTIL_SHA}.zip -o libdmdutil.zip
   unzip libdmdutil.zip
   cd libdmdutil-$LIBDMDUTIL_SHA
   platforms/android/arm64-v8a/external.sh
   cmake \
      -DPLATFORM=android \
      -DARCH=arm64-v8a \
      -DBUILD_STATIC=OFF \
      -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
      -B build
   cmake --build build -- -j${NUM_PROCS}
   mkdir -p ../../${CACHE_DIR}/${CACHE_NAME}/include
   cp -r include/DMDUtil ../../${CACHE_DIR}/${CACHE_NAME}/include
   cp -r third-party/include/sockpp ../../${CACHE_DIR}/${CACHE_NAME}/include
   mkdir -p ../../${CACHE_DIR}/${CACHE_NAME}/lib
   cp -a third-party/runtime-libs/android/arm64-v8a/*.so ../../${CACHE_DIR}/${CACHE_NAME}/lib
   cp -a build/*.so ../../${CACHE_DIR}/${CACHE_NAME}/lib
   cd ..
   touch "../${CACHE_DIR}/${CACHE_NAME}.cache"
fi

cp -r ../${CACHE_DIR}/${CACHE_NAME}/include/* ../external/include
cp -a ../${CACHE_DIR}/${CACHE_NAME}/lib/*.so ../external/lib

#
# build libdof (and deps) and copy to external
#

CACHE_NAME="libdof-${LIBDOF_SHA}"

if [ ! -f "../${CACHE_DIR}/${CACHE_NAME}.cache" ]; then
   curl -sL https://github.com/jsm174/libdof/archive/${LIBDOF_SHA}.zip -o libdof.zip
   unzip libdof.zip
   cd libdof-$LIBDOF_SHA
   platforms/android/arm64-v8a/external.sh
   cmake \
      -DPLATFORM=android \
      -DARCH=arm64-v8a \
      -DBUILD_STATIC=OFF \
      -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
      -B build
   cmake --build build -- -j${NUM_PROCS}
   mkdir -p ../../${CACHE_DIR}/${CACHE_NAME}/include
   cp -r include/DOF ../../${CACHE_DIR}/${CACHE_NAME}/include
   mkdir -p ../../${CACHE_DIR}/${CACHE_NAME}/lib
   cp -a third-party/runtime-libs/android/arm64-v8a/*.so ../../${CACHE_DIR}/${CACHE_NAME}/lib
   cp -a build/*.so ../../${CACHE_DIR}/${CACHE_NAME}/lib
   cd ..
   touch "../${CACHE_DIR}/${CACHE_NAME}.cache"
fi

cp -r ../${CACHE_DIR}/${CACHE_NAME}/include/* ../external/include
cp -a ../${CACHE_DIR}/${CACHE_NAME}/lib/*.so ../external/lib

#
# build FFMPEG libraries and copy to external
#

# TODO: build FFMPEG libraries for android

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
      -DCMAKE_SYSTEM_NAME=Android \
      -DCMAKE_SYSTEM_VERSION=30 \
      -DCMAKE_ANDROID_ARCH_ABI=arm64-v8a \
      -DBGFX_BUILD_EXAMPLES=OFF \
      -DBGFX_CONFIG_MULTITHREADED=ON \
      -DBGFX_CONFIG_MAX_FRAME_BUFFERS=256 \
      -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
      -B build
   cmake --build build -- -j${NUM_PROCS}
   mkdir -p ../../${CACHE_DIR}/${CACHE_NAME}/include
   cp -r bgfx/include/bgfx ../../${CACHE_DIR}/${CACHE_NAME}/include
   cp -r bimg/include/bimg ../../${CACHE_DIR}/${CACHE_NAME}/include
   cp -r bx/include/bx ../../${CACHE_DIR}/${CACHE_NAME}/include
   mkdir -p ../../${CACHE_DIR}/${CACHE_NAME}/lib
   cp build/cmake/bgfx/*.a ../../${CACHE_DIR}/${CACHE_NAME}/lib
   cp build/cmake/bimg/*.a ../../${CACHE_DIR}/${CACHE_NAME}/lib
   cp build/cmake/bx/*.a ../../${CACHE_DIR}/${CACHE_NAME}/lib
   cd ..
   touch "../${CACHE_DIR}/${CACHE_NAME}.cache"
fi

cp -r ../${CACHE_DIR}/${CACHE_NAME}/include/* ../external/include
cp ../${CACHE_DIR}/${CACHE_NAME}/lib/*.a ../external/lib
