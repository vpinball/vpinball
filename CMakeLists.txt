cmake_minimum_required(VERSION 3.25)

# CMake requires a literal project() call in the top-level file, and CMAKE_PROJECT_VERSION is
# taken from it, so read the real version here (the selected per-target file calls project()
# again with the same version).
file(READ src/core/vpversion.h _vpx_version)
string(REGEX MATCH "VP_VERSION_MAJOR[ ]+([0-9]+)" _tmp ${_vpx_version})
set(_vpx_major "${CMAKE_MATCH_1}")
string(REGEX MATCH "VP_VERSION_MINOR[ ]+([0-9]+)" _tmp ${_vpx_version})
set(_vpx_minor "${CMAKE_MATCH_1}")
string(REGEX MATCH "VP_VERSION_REV[ ]+([0-9]+)" _tmp ${_vpx_version})
set(_vpx_rev "${CMAKE_MATCH_1}")
project(vpinball VERSION "${_vpx_major}.${_vpx_minor}.${_vpx_rev}")

# Top-level dispatcher.
#
# Selects one of the per-target build files in make/ from three cache variables:
#   TARGET    renderer backend: BGFX (default), GL or DX9
#   PLATFORM  linux, macos, windows, windows-mingw, ... (defaults to the host)
#   ARCH      x64, x86, arm64, aarch64, ... (defaults to the host)
# All three default to the BGFX renderer for the current host, so a plain `cmake -B build`
# just works. Override any of them, e.g.:
#
#     cmake -DTARGET=GL -B build
#     cmake -DPLATFORM=windows -DARCH=x86 -B build
#
# The iOS/Android library builds are not selected here: they must set their cross-compile
# toolchain before project(), which this dispatcher cannot do, so they use
# make/CMakeLists_bgfx_lib.txt directly (with -DPLATFORM/-DARCH).

# --- Detect the host platform --------------------------------------------------------------
if(CMAKE_HOST_APPLE)
   set(_vpx_host_platform "macos")
elseif(CMAKE_HOST_WIN32)
   set(_vpx_host_platform "windows")
elseif(CMAKE_HOST_UNIX)
   set(_vpx_host_platform "linux")
else()
   set(_vpx_host_platform "")
endif()

# --- Detect the host architecture ----------------------------------------------------------
cmake_host_system_information(RESULT _vpx_host_arch_raw QUERY OS_PLATFORM)
string(TOLOWER "${_vpx_host_arch_raw}" _vpx_host_arch_raw)
if(_vpx_host_arch_raw MATCHES "arm64|aarch64")
   # the per-target files spell this differently per platform (macos: arm64, linux: aarch64)
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

set(TARGET "BGFX" CACHE STRING "Renderer backend: BGFX, GL or DX9")
set(PLATFORM "${_vpx_host_platform}" CACHE STRING "Target platform; defaults to the host")
set(ARCH "${_vpx_host_arch}" CACHE STRING "Target architecture; defaults to the host")

if(PLATFORM STREQUAL "" OR ARCH STREQUAL "")
   message(FATAL_ERROR "Could not detect the host platform/architecture; pass -DPLATFORM=<platform> -DARCH=<arch> (and optionally -DTARGET=<backend>).")
endif()

string(TOLOWER "${TARGET}" _vpx_backend)

# --- Resolve and include the selected per-target build file ---------------------------------
set(_vpx_target_file "${CMAKE_CURRENT_LIST_DIR}/make/CMakeLists_${_vpx_backend}-${PLATFORM}-${ARCH}.txt")

if(NOT EXISTS "${_vpx_target_file}")
   file(GLOB _vpx_files RELATIVE "${CMAKE_CURRENT_LIST_DIR}/make"
        "${CMAKE_CURRENT_LIST_DIR}/make/CMakeLists_*-*.txt")
   set(_vpx_available "")
   foreach(_f IN LISTS _vpx_files)
      string(REGEX REPLACE "^CMakeLists_(.+)\\.txt$" "\\1" _t "${_f}")
      string(APPEND _vpx_available "\n  ${_t}")
   endforeach()
   message(FATAL_ERROR "No build file for TARGET=${TARGET} PLATFORM=${PLATFORM} ARCH=${ARCH} (expected ${_vpx_target_file}).\nAvailable <backend>-<platform>-<arch> targets:${_vpx_available}\n(iOS/Android use make/CMakeLists_bgfx_lib.txt directly.)")
endif()

# The heavy third-party dependencies (SDL, bgfx, FreeImage, ...) are not committed; they are
# fetched and built into third-party/ by the platform script. Fail early with a clear message
# instead of a confusing missing-header error deep in the build. SDL3 is used by every backend.
if(NOT EXISTS "${CMAKE_CURRENT_LIST_DIR}/third-party/include/SDL3/SDL.h")
   message(FATAL_ERROR "Third-party dependencies not found in third-party/. Build them for this target first:\n  platforms/${PLATFORM}-${ARCH}/external.sh")
endif()

message(STATUS "VPX build: TARGET=${TARGET} PLATFORM=${PLATFORM} ARCH=${ARCH}")
include("${_vpx_target_file}")
