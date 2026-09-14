cmake_minimum_required(VERSION 3.25)

# Single build file for every target, selected by three cache variables (default to host BGFX):
#   RENDERER  BGFX (default), GL, DX9
#   PLATFORM  macos, linux, windows, windows-mingw, ios, ios-simulator, android
#   ARCH      x64, x86, arm64, aarch64, arm64-v8a

if(CMAKE_HOST_APPLE)
   set(_vpx_host_platform "macos")
elseif(CMAKE_HOST_WIN32)
   set(_vpx_host_platform "windows")
elseif(CMAKE_HOST_UNIX)
   set(_vpx_host_platform "linux")
else()
   set(_vpx_host_platform "")
endif()

cmake_host_system_information(RESULT _vpx_host_arch_raw QUERY OS_PLATFORM)
string(TOLOWER "${_vpx_host_arch_raw}" _vpx_host_arch_raw)
if(_vpx_host_arch_raw MATCHES "arm64|aarch64")
   if(_vpx_host_platform STREQUAL "macos")
      set(_vpx_host_arch "arm64")
   else()
      set(_vpx_host_arch "aarch64")
   endif()
elseif(_vpx_host_arch_raw MATCHES "x86_64|amd64")
   set(_vpx_host_arch "x64")
elseif(_vpx_host_arch_raw MATCHES "i[3-6]86|x86")
   set(_vpx_host_arch "x86")
else()
   set(_vpx_host_arch "")
endif()

set(RENDERER "BGFX" CACHE STRING "Renderer backend: BGFX, GL or DX9")
set(PLATFORM "${_vpx_host_platform}" CACHE STRING "Target platform; defaults to the host")
set(ARCH "${_vpx_host_arch}" CACHE STRING "Target architecture; defaults to the host")

if(PLATFORM STREQUAL "" OR ARCH STREQUAL "")
   message(FATAL_ERROR "Could not detect the host platform/architecture; pass -DPLATFORM=<platform> -DARCH=<arch> (and optionally -DRENDERER=<backend>).")
endif()

set(_vpx_valid_combos
   BGFX-windows
   BGFX-windows-mingw
   BGFX-macos
   BGFX-linux
   BGFX-ios
   BGFX-ios-simulator
   BGFX-android
   GL-windows
   GL-macos
   GL-linux
   DX9-windows)
if(NOT "${RENDERER}-${PLATFORM}" IN_LIST _vpx_valid_combos)
   string(REPLACE ";" "\n  " _vpx_available "${_vpx_valid_combos}")
   message(FATAL_ERROR "Unsupported RENDERER=${RENDERER} PLATFORM=${PLATFORM}.\nSupported <backend>-<platform> combinations:\n  ${_vpx_available}")
endif()

# CMAKE_SYSTEM_NAME must be set before project() for iOS and Android toolchains to work

if(PLATFORM STREQUAL "ios" OR PLATFORM STREQUAL "ios-simulator")
   set(CMAKE_SYSTEM_NAME iOS)
   if(PLATFORM STREQUAL "ios-simulator")
      set(CMAKE_OSX_SYSROOT iphonesimulator)
   endif()
   set(CMAKE_OSX_DEPLOYMENT_TARGET 18.0)
elseif(PLATFORM STREQUAL "android")
   set(CMAKE_SYSTEM_NAME Android)
   set(CMAKE_SYSTEM_VERSION 33)
   set(CMAKE_ANDROID_ARCH_ABI arm64-v8a)
endif()

# Parse VP_VERSION_* into VERSION_MAJOR/MINOR/REV

file(READ "${CMAKE_SOURCE_DIR}/src/core/vpversion.h" _vpx_version)
string(REGEX MATCH "VP_VERSION_MAJOR[ ]+([0-9]+)" _tmp "${_vpx_version}")
set(VERSION_MAJOR "${CMAKE_MATCH_1}")
string(REGEX MATCH "VP_VERSION_MINOR[ ]+([0-9]+)" _tmp "${_vpx_version}")
set(VERSION_MINOR "${CMAKE_MATCH_1}")
string(REGEX MATCH "VP_VERSION_REV[ ]+([0-9]+)" _tmp "${_vpx_version}")
set(VERSION_REV "${CMAKE_MATCH_1}")
project(vpinball VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_REV}")

message(STATUS "VPX build: RENDERER=${RENDERER} PLATFORM=${PLATFORM} ARCH=${ARCH}")

option(BUILD_APP "Build the application; the plugins are always built" ON)
message(STATUS "BUILD_APP: ${BUILD_APP}")

option(POST_BUILD_COPY_EXT_LIBS "Copy external libraries to build directory" ON)
if(PLATFORM STREQUAL "macos")
   option(ENABLE_SANITIZERS "Enable AddressSanitizer and UBSan" OFF)
   message(STATUS "ENABLE_SANITIZERS: ${ENABLE_SANITIZERS}")
endif()
if(PLATFORM STREQUAL "linux")
   option(BUILD_RPI "Build for Raspberry PI" OFF)
   option(BUILD_RK3588 "Build for RK3588" OFF)
endif()
if(PLATFORM STREQUAL "ios" OR PLATFORM STREQUAL "ios-simulator" OR PLATFORM STREQUAL "android")
   option(ENABLE_XR "Enable OpenXR support" OFF)
   message(STATUS "ENABLE_XR: ${ENABLE_XR}")
   if((PLATFORM STREQUAL "ios" OR PLATFORM STREQUAL "ios-simulator") AND ENABLE_XR)
      message(FATAL_ERROR "iOS does not support ENABLE_XR")
   endif()
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 99)

add_compile_definitions($<$<CONFIG:Release>:IMGUI_DISABLE_DEBUG_TOOLS>)
add_compile_options(
   $<$<CXX_COMPILER_ID:MSVC>:/we4172>
   $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:-Werror=return-local-addr>
)

# Distro builds like Batocera do not use external.sh. They point these at their system/staging
# tree, which is then searched before the bundled third-party/. Default OFF keeps external.sh
# builds on the bundled tree.

option(USE_SYSTEM_LIBS "Search system locations for third-party libs before the bundled tree" OFF)
set(SYSTEM_LIBS_INCLUDE_DIR "/usr/include" CACHE PATH "System include dir searched first when USE_SYSTEM_LIBS")
set(SYSTEM_LIBS_LIB_DIR "/usr/lib" CACHE PATH "System library dir searched first when USE_SYSTEM_LIBS")

if(USE_SYSTEM_LIBS)
   # include_directories(BEFORE) prepends each entry, so the resulting search order is the
   # reverse of this list: libwinevbs/atl must shadow wine for atlbase.h/atlcom.h.
   include_directories(BEFORE
      "${SYSTEM_LIBS_INCLUDE_DIR}"
      "${SYSTEM_LIBS_INCLUDE_DIR}/libwinevbs/wine"
      "${SYSTEM_LIBS_INCLUDE_DIR}/libwinevbs/atlmfc"
      "${SYSTEM_LIBS_INCLUDE_DIR}/libwinevbs/atl")
   link_directories(BEFORE "${SYSTEM_LIBS_LIB_DIR}")
   message(STATUS "USE_SYSTEM_LIBS: include=${SYSTEM_LIBS_INCLUDE_DIR} lib=${SYSTEM_LIBS_LIB_DIR}")
endif()

# Shared by the application and the plugins, so that BUILD_APP=OFF compiles them the same way.

if(PLATFORM STREQUAL "windows")
   add_compile_options(/Zc:__cplusplus)

   if(RENDERER STREQUAL "BGFX")
      add_compile_options(/Zc:preprocessor)
   endif()
elseif(PLATFORM STREQUAL "macos")
   set(CMAKE_OSX_DEPLOYMENT_TARGET 14.0)
   if(ARCH STREQUAL "x64")
      set(CMAKE_OSX_ARCHITECTURES x86_64)
   else()
      set(CMAKE_OSX_ARCHITECTURES ${ARCH})
   endif()
   set(CMAKE_INSTALL_RPATH "@executable_path/../Frameworks")
   set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)

   if(ENABLE_SANITIZERS)
      add_compile_options(-fsanitize=address)
      add_link_options(-fsanitize=address)

      add_compile_options(-fsanitize=undefined)
      add_link_options(-fsanitize=undefined)
   endif()
elseif(PLATFORM STREQUAL "linux")
   set(CMAKE_INSTALL_RPATH "$ORIGIN")
   set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
elseif(PLATFORM STREQUAL "ios" OR PLATFORM STREQUAL "ios-simulator" OR PLATFORM STREQUAL "android")
   add_compile_options(-fPIC)
endif()

if(BUILD_APP)
   include("${CMAKE_SOURCE_DIR}/make/CMakeLists_sources.txt")
   include("${CMAKE_SOURCE_DIR}/make/CMakeLists_app.txt")
endif()

include("${CMAKE_SOURCE_DIR}/make/CMakeLists_plugins.txt")
