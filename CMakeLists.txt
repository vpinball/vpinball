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

include("${CMAKE_SOURCE_DIR}/make/CMakeLists_sources.txt")

# Win32 Editor build (BGFX / GL / DX9)

if(PLATFORM STREQUAL "windows")
   if(RENDERER STREQUAL "GL")
      add_library(glad
         third-party/include/glad/src/gl.c
      )
      target_include_directories(glad PUBLIC
         third-party/include/glad/include
      )
      set_target_properties(glad PROPERTIES
         MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
      )
   endif()

   add_compile_options(/Zc:__cplusplus)

   if(RENDERER STREQUAL "BGFX")
      add_compile_options(/Zc:preprocessor)
   endif()

   set_source_files_properties(
      third-party/include/miniz/miniz.c PROPERTIES LANGUAGE CXX
      third-party/include/hidapi/windows/hid.cpp PROPERTIES SKIP_PRECOMPILE_HEADERS ON
   )

   set_source_files_properties(
      src/ui/win/vpinball.rc LANGUAGE RC
   )

   if(RENDERER STREQUAL "DX9")
      set_source_files_properties(src/shaders/hlsl_glsl/BallShader.hlsl PROPERTIES
         VS_SHADER_TYPE Effect
         VS_SHADER_MODEL 2.0
         VS_SHADER_OUTPUT_HEADER_FILE "${CMAKE_CURRENT_BINARY_DIR}/$(IntDir)/src/shaders/hlsl_ball.h"
         VS_SHADER_ENTRYPOINT ballShaderCode
      )
      set_source_files_properties(src/shaders/hlsl_glsl/BasicShader.hlsl PROPERTIES
         VS_SHADER_TYPE Effect
         VS_SHADER_MODEL 2.0
         VS_SHADER_OUTPUT_HEADER_FILE "${CMAKE_CURRENT_BINARY_DIR}/$(IntDir)/src/shaders/hlsl_basic.h"
         VS_SHADER_ENTRYPOINT basicShaderCode
      )
      set_source_files_properties(src/shaders/hlsl_glsl/DMDShader.hlsl PROPERTIES
         VS_SHADER_TYPE Effect
         VS_SHADER_MODEL 2.0
         VS_SHADER_OUTPUT_HEADER_FILE "${CMAKE_CURRENT_BINARY_DIR}/$(IntDir)/src/shaders/hlsl_dmd.h"
         VS_SHADER_ENTRYPOINT dmdShaderCode
      )
      set_source_files_properties(src/shaders/hlsl_glsl/FBShader.hlsl PROPERTIES
         VS_SHADER_TYPE Effect
         VS_SHADER_MODEL 2.0
         VS_SHADER_OUTPUT_HEADER_FILE "${CMAKE_CURRENT_BINARY_DIR}/$(IntDir)/src/shaders/hlsl_postprocess.h"
         VS_SHADER_ENTRYPOINT FBShaderCode
      )
      set_source_files_properties(src/shaders/hlsl_glsl/FlasherShader.hlsl PROPERTIES
         VS_SHADER_TYPE Effect
         VS_SHADER_MODEL 2.0
         VS_SHADER_OUTPUT_HEADER_FILE "${CMAKE_CURRENT_BINARY_DIR}/$(IntDir)/src/shaders/hlsl_flasher.h"
         VS_SHADER_ENTRYPOINT flasherShaderCode
      )
      set_source_files_properties(src/shaders/hlsl_glsl/LightShader.hlsl PROPERTIES
         VS_SHADER_TYPE Effect
         VS_SHADER_MODEL 2.0
         VS_SHADER_OUTPUT_HEADER_FILE "${CMAKE_CURRENT_BINARY_DIR}/$(IntDir)/src/shaders/hlsl_light.h"
         VS_SHADER_ENTRYPOINT lightShaderCode
      )
      set_source_files_properties(src/shaders/hlsl_glsl/UIShader.hlsl PROPERTIES
         VS_SHADER_TYPE Effect
         VS_SHADER_MODEL 2.0
         VS_SHADER_OUTPUT_HEADER_FILE "${CMAKE_CURRENT_BINARY_DIR}/$(IntDir)/src/shaders/hlsl_ui.h"
         VS_SHADER_ENTRYPOINT uiShaderCode
      )
   endif()

   add_executable(vpinball WIN32
      ${VPX_WINDOWS_SOURCES}
   )

   target_precompile_headers(vpinball PUBLIC
      src/core/stdafx.h
   )

   if(RENDERER STREQUAL "BGFX")
      target_include_directories(vpinball PUBLIC
         ${CMAKE_CURRENT_SOURCE_DIR}
         ${CMAKE_CURRENT_BINARY_DIR}/$(IntDir)
         third-party/include
         third-party/include/win32xx/include
         third-party/include/compat/msvc
         src
         plugins
      )
      target_compile_definitions(vpinball PRIVATE
         ENABLE_BGFX
         $<$<CONFIG:Debug>:BX_CONFIG_DEBUG=1>
         $<$<CONFIG:Release>:BX_CONFIG_DEBUG=0>
      )
   elseif(RENDERER STREQUAL "GL")
      target_sources(vpinball PRIVATE
         src/renderer/DXGIRegistry.h
      )
      target_include_directories(vpinball PUBLIC
         $ENV{DXSDK_DIR}/Include
         ${CMAKE_CURRENT_SOURCE_DIR}
         ${CMAKE_CURRENT_BINARY_DIR}/$(IntDir)
         third-party/include
         third-party/include/win32xx/include
         src
         plugins
      )
      target_compile_definitions(vpinball PRIVATE
         ENABLE_OPENGL
      )
   else()
      target_sources(vpinball PRIVATE
         src/shaders/hlsl_glsl/BallShader.hlsl
         src/shaders/hlsl_glsl/BasicShader.hlsl
         src/shaders/hlsl_glsl/DMDShader.hlsl
         src/shaders/hlsl_glsl/FBShader.hlsl
         src/shaders/hlsl_glsl/FlasherShader.hlsl
         src/shaders/hlsl_glsl/LightShader.hlsl
         src/shaders/hlsl_glsl/UIShader.hlsl
      )
      target_include_directories(vpinball PUBLIC
         $ENV{DXSDK_DIR}/Include
         ${CMAKE_CURRENT_SOURCE_DIR}
         ${CMAKE_CURRENT_BINARY_DIR}/$(IntDir)
         ${CMAKE_CURRENT_BINARY_DIR}/$(IntDir)/src
         third-party/include
         third-party/include/winsdk
         third-party/include/win32xx/include
         src
         plugins
         shader
      )
      target_compile_definitions(vpinball PRIVATE
         ENABLE_DX9
      )
   endif()

   set(OPT_COMMON
      /Ob2
      /O2
      /Oi
      /fp:fast
      /fp:except-
      /Ot
      /GF
      /GS-
      /Gy
      /GR
      /Oy
      /GT
      /GL
      /w44005
      /Zc:__cplusplus
      /std:c++20
      $<$<NOT:$<STREQUAL:${ARCH},x64>>:/arch:SSE2>
   )

   set(CMAKE_VERBOSE_MAKEFILE ON)

   target_compile_options(vpinball PUBLIC
      $<$<CONFIG:Release>:$<$<COMPILE_LANGUAGE:CXX>:${OPT_COMMON}>>
      $<$<CONFIG:Release>:$<$<COMPILE_LANGUAGE:C>:${OPT_COMMON}>>
   )

   set(BUILD_LIBS_DIR "${CMAKE_SOURCE_DIR}/third-party/build-libs/windows-${ARCH}")
   set(RUNTIME_LIBS_DIR "${CMAKE_SOURCE_DIR}/third-party/runtime-libs/windows-${ARCH}")

   if(RENDERER STREQUAL "BGFX")
      target_link_directories(vpinball PUBLIC
         ${BUILD_LIBS_DIR}
      )
   else()
      target_link_directories(vpinball PUBLIC
         $ENV{DXSDK_DIR}/Lib/${ARCH}
         ${BUILD_LIBS_DIR}
      )
   endif()

   target_link_libraries(vpinball
      comctl32.lib
      dxguid.lib
      nvapi.lib
      winmm.lib
      hid.lib
      setupapi.lib
      dinput8.lib
      $<IF:$<STREQUAL:${ARCH},x64>,freeimage64.lib,freeimage.lib>
      $<IF:$<STREQUAL:${ARCH},x64>,SDL364.lib,SDL3.lib>
   )

   if(RENDERER STREQUAL "BGFX")
      target_link_libraries(vpinball
         $<IF:$<STREQUAL:${ARCH},x64>,openxr_loader64.lib,openxr_loader.lib>
         $<IF:$<STREQUAL:${ARCH},x64>,bgfx64.lib,bgfx.lib>
         $<IF:$<STREQUAL:${ARCH},x64>,bimg64.lib,bimg.lib>
         $<IF:$<STREQUAL:${ARCH},x64>,bx64.lib,bx.lib>
      )
   elseif(RENDERER STREQUAL "GL")
      target_link_libraries(vpinball
         glad
      )
   else()
      target_link_libraries(vpinball
         d3d9.lib
         d3dx9.lib
         dxerr.lib
      )
   endif()

   target_link_options(vpinball PUBLIC
      $<$<CONFIG:Release>:/INCREMENTAL:NO>
      $<$<CONFIG:Release>:/OPT:REF>
      $<$<CONFIG:Release>:/OPT:ICF>
      $<$<CONFIG:Release>:/LTCG>
      $<$<CONFIG:Release>:/DYNAMICBASE:NO>
      $<IF:$<STREQUAL:${ARCH},x64>,$<$<CONFIG:Release>:/SAFESEH:NO>,/SAFESEH:NO>
      $<$<NOT:$<STREQUAL:${ARCH},x64>>:/LARGEADDRESSAWARE>
   )

   if(RENDERER STREQUAL "BGFX")
      set(RUNTIME_OUTPUT_NAME "$<IF:$<STREQUAL:${ARCH},x64>,VPinballX_BGFX64,VPinballX_BGFX>")
   elseif(RENDERER STREQUAL "GL")
      set(RUNTIME_OUTPUT_NAME "$<IF:$<STREQUAL:${ARCH},x64>,VPinballX_GL64,VPinballX_GL>")
   else()
      set(RUNTIME_OUTPUT_NAME "$<IF:$<STREQUAL:${ARCH},x64>,VPinballX64,VPinballX>")
   endif()

   set_target_properties(vpinball PROPERTIES
      VS_DPI_AWARE "PerMonitor"
      RUNTIME_OUTPUT_NAME "${RUNTIME_OUTPUT_NAME}"
      MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
   )

   add_custom_command(TARGET vpinball POST_BUILD
      COMMAND "${CMAKE_COMMAND}" -E copy_directory "${CMAKE_SOURCE_DIR}/src/assets" "$<TARGET_FILE_DIR:vpinball>/assets"
      COMMAND "${CMAKE_COMMAND}" -E copy_directory "${CMAKE_SOURCE_DIR}/scripts" "$<TARGET_FILE_DIR:vpinball>/scripts"
      COMMAND "${CMAKE_COMMAND}" -E copy_directory "${CMAKE_SOURCE_DIR}/docs" "$<TARGET_FILE_DIR:vpinball>/docs"
   )

   if(RENDERER STREQUAL "GL")
      set(SHADER_DIR "$<TARGET_FILE_DIR:vpinball>/shaders-${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_REV}")
      add_custom_command(TARGET vpinball POST_BUILD
         COMMAND "${CMAKE_COMMAND}" -E make_directory "${SHADER_DIR}"
      )
      foreach(_shader IN LISTS VPX_GL_SHADERS)
         add_custom_command(TARGET vpinball POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E copy "${CMAKE_SOURCE_DIR}/src/shaders/hlsl_glsl/${_shader}" "${SHADER_DIR}"
         )
      endforeach()
   endif()

   if(POST_BUILD_COPY_EXT_LIBS)
      add_custom_command(TARGET vpinball POST_BUILD
         COMMAND "${CMAKE_COMMAND}" -E copy "${RUNTIME_LIBS_DIR}/$<IF:$<STREQUAL:${ARCH},x64>,freeimage64.dll,freeimage.dll>" "$<TARGET_FILE_DIR:vpinball>"
         COMMAND "${CMAKE_COMMAND}" -E copy "${RUNTIME_LIBS_DIR}/$<IF:$<STREQUAL:${ARCH},x64>,SciLexerVP64.dll,SciLexerVP.dll>" "$<TARGET_FILE_DIR:vpinball>"
         COMMAND "${CMAKE_COMMAND}" -E copy "${RUNTIME_LIBS_DIR}/$<IF:$<STREQUAL:${ARCH},x64>,SDL364.dll,SDL3.dll>" "$<TARGET_FILE_DIR:vpinball>"
      )
      if(RENDERER STREQUAL "BGFX")
         add_custom_command(TARGET vpinball POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E copy "${RUNTIME_LIBS_DIR}/$<IF:$<STREQUAL:${ARCH},x64>,openxr_loader64.dll,openxr_loader.dll>" "$<TARGET_FILE_DIR:vpinball>"
         )
      elseif(RENDERER STREQUAL "DX9")
         add_custom_command(TARGET vpinball POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E copy "${RUNTIME_LIBS_DIR}/$<IF:$<STREQUAL:${ARCH},x64>,D3DX9_4364.dll,D3DX9_4332.dll>" "$<TARGET_FILE_DIR:vpinball>"
         )
      endif()
   endif()

# Windows MinGW build (BGFX)

elseif(PLATFORM STREQUAL "windows-mingw")
   set_source_files_properties(
      third-party/include/miniz/miniz.c PROPERTIES LANGUAGE CXX
   )

   add_executable(vpinball
      ${VPX_STANDALONE_SOURCES}
   )

   target_include_directories(vpinball PUBLIC
      ${CMAKE_SOURCE_DIR}/third-party/include
      ${CMAKE_SOURCE_DIR}
      ${CMAKE_SOURCE_DIR}/third-party/include/libwinevbs/atl/include
      ${CMAKE_SOURCE_DIR}/third-party/include/libwinevbs/atlmfc/include
      ${CMAKE_SOURCE_DIR}/third-party/include/libwinevbs/wine/include
      src
      plugins
   )

   target_compile_definitions(vpinball PRIVATE
      __STANDALONE__

      __LIBWINEVBS__
      WINE_UNICODE_NATIVE

      WINBOOL=int
      WIN32_LEAN_AND_MEAN
      NOMINMAX

      ENABLE_BGFX
      $<IF:$<CONFIG:Debug>,BX_CONFIG_DEBUG=1,BX_CONFIG_DEBUG=0>
   )

   target_compile_options(vpinball PUBLIC
      -mssse3
      $<$<CXX_COMPILER_ID:GNU>:
         -ffast-math
         -ffp-contract=fast
         -Wno-template-body>
   )

   set(BUILD_LIBS_DIR "${CMAKE_SOURCE_DIR}/third-party/build-libs/windows-mingw-x64")
   set(RUNTIME_LIBS_DIR "${CMAKE_SOURCE_DIR}/third-party/runtime-libs/windows-mingw-x64")

   target_link_directories(vpinball PUBLIC
      ${BUILD_LIBS_DIR}
      ${RUNTIME_LIBS_DIR}
   )

   target_link_libraries(vpinball
      bgfx64
      bimg64
      bx64
      SDL364
      SDL3_image64
      SDL3_ttf64
      freeimage64
      hidapi64
      openxr_loader64
      winevbs64
   )

   target_link_options(vpinball PUBLIC
      -Wl,--dynamicbase
      -Wl,--nxcompat
      -Wl,--high-entropy-va
   )

   set_target_properties(vpinball PROPERTIES
      RUNTIME_OUTPUT_NAME "VPinballX_BGFX64"
   )

   add_custom_command(TARGET vpinball POST_BUILD
      COMMAND "${CMAKE_COMMAND}" -E copy_directory "${CMAKE_SOURCE_DIR}/src/assets" "$<TARGET_FILE_DIR:vpinball>/assets"
      COMMAND "${CMAKE_COMMAND}" -E copy_directory "${CMAKE_SOURCE_DIR}/scripts" "$<TARGET_FILE_DIR:vpinball>/scripts"
      COMMAND "${CMAKE_COMMAND}" -E copy_directory "${CMAKE_SOURCE_DIR}/docs" "$<TARGET_FILE_DIR:vpinball>/docs"
   )

   if(POST_BUILD_COPY_EXT_LIBS)
      add_custom_command(TARGET vpinball POST_BUILD
         COMMAND "${CMAKE_COMMAND}" -E copy "${RUNTIME_LIBS_DIR}/SDL364.dll" "$<TARGET_FILE_DIR:vpinball>"
         COMMAND "${CMAKE_COMMAND}" -E copy "${RUNTIME_LIBS_DIR}/SDL3_image64.dll" "$<TARGET_FILE_DIR:vpinball>"
         COMMAND "${CMAKE_COMMAND}" -E copy "${RUNTIME_LIBS_DIR}/SDL3_ttf64.dll" "$<TARGET_FILE_DIR:vpinball>"
         COMMAND "${CMAKE_COMMAND}" -E copy "${RUNTIME_LIBS_DIR}/freeimage64.dll" "$<TARGET_FILE_DIR:vpinball>"
         COMMAND "${CMAKE_COMMAND}" -E copy "${RUNTIME_LIBS_DIR}/pinmame64.dll" "$<TARGET_FILE_DIR:vpinball>"
         COMMAND "${CMAKE_COMMAND}" -E copy "${RUNTIME_LIBS_DIR}/altsound64.dll" "$<TARGET_FILE_DIR:vpinball>"
         COMMAND "${CMAKE_COMMAND}" -E copy "${RUNTIME_LIBS_DIR}/dmdutil64.dll" "$<TARGET_FILE_DIR:vpinball>"
         COMMAND "${CMAKE_COMMAND}" -E copy "${RUNTIME_LIBS_DIR}/dof64.dll" "$<TARGET_FILE_DIR:vpinball>"
         COMMAND "${CMAKE_COMMAND}" -E copy "${RUNTIME_LIBS_DIR}/hidapi64.dll" "$<TARGET_FILE_DIR:vpinball>"
         COMMAND "${CMAKE_COMMAND}" -E copy "${RUNTIME_LIBS_DIR}/openxr_loader64.dll" "$<TARGET_FILE_DIR:vpinball>"
         COMMAND "${CMAKE_COMMAND}" -E copy "${RUNTIME_LIBS_DIR}/winevbs64.dll" "$<TARGET_FILE_DIR:vpinball>"
         COMMAND "${CMAKE_COMMAND}" -E copy "${RUNTIME_LIBS_DIR}/libwinpthread-1.dll" "$<TARGET_FILE_DIR:vpinball>"
         COMMAND "${CMAKE_COMMAND}" -E copy "${RUNTIME_LIBS_DIR}/libgcc_s_seh-1.dll" "$<TARGET_FILE_DIR:vpinball>"
         COMMAND "${CMAKE_COMMAND}" -E copy "${RUNTIME_LIBS_DIR}/libstdc++-6.dll" "$<TARGET_FILE_DIR:vpinball>"
      )
   endif()

# macOS build (BGFX / GL)

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

   if(RENDERER STREQUAL "GL")
      add_library(glad
         third-party/include/glad/src/gl.c
      )
      target_include_directories(glad PUBLIC
         third-party/include/glad/include
      )
   endif()

   add_executable(vpinball
      ${VPX_STANDALONE_SOURCES}

      standalone/macos/main.mm
   )

   target_include_directories(vpinball PUBLIC
      ${CMAKE_SOURCE_DIR}/third-party/include
      ${CMAKE_SOURCE_DIR}
      ${CMAKE_SOURCE_DIR}/third-party/include/libwinevbs/atl/include
      ${CMAKE_SOURCE_DIR}/third-party/include/libwinevbs/atlmfc/include
      ${CMAKE_SOURCE_DIR}/third-party/include/libwinevbs/wine/include
      src
      plugins
   )

   target_compile_definitions(vpinball PRIVATE
      __STANDALONE__

      __LIBWINEVBS__
      __WINESRC__
      WINE_UNICODE_NATIVE

      "__forceinline=__attribute__((always_inline)) inline"
   )

   if(RENDERER STREQUAL "BGFX")
      target_compile_definitions(vpinball PRIVATE
         ENABLE_BGFX

         $<$<CONFIG:Debug>:BX_CONFIG_DEBUG=1>
         $<$<CONFIG:Release>:BX_CONFIG_DEBUG=0>
      )
   else()
      target_compile_definitions(vpinball PRIVATE
         ENABLE_OPENGL
      )
   endif()

   target_compile_options(vpinball PUBLIC
      -Wno-inconsistent-missing-override
      -Wno-missing-exception-spec
      -fdenormal-fp-math=preserve-sign
      -freciprocal-math
      -fassociative-math
      -fapprox-func
      -fno-math-errno
      -fno-trapping-math
      -ffp-contract=fast
      -Wno-shorten-64-to-32
      $<$<AND:$<STREQUAL:${ARCH},x64>,$<STREQUAL:${RENDERER},GL>>:-mssse3>
   )

   set(RUNTIME_LIBS_DIR "${CMAKE_SOURCE_DIR}/third-party/runtime-libs/macos-${ARCH}")

   target_link_directories(vpinball PUBLIC
      "${RUNTIME_LIBS_DIR}"
   )

   target_link_libraries(vpinball PUBLIC
      SDL3
      SDL3_image
      SDL3_ttf
      freeimage
      hidapi
      winevbs
      "-framework AppKit"
   )

   target_link_libraries(vpinball PUBLIC $<IF:$<STREQUAL:${RENDERER},BGFX>,bgfx,glad>)

   set(RUNTIME_OUTPUT_NAME "$<IF:$<STREQUAL:${RENDERER},BGFX>,VPinballX_BGFX,VPinballX_GL>")

   set_target_properties(vpinball PROPERTIES
      RUNTIME_OUTPUT_NAME "${RUNTIME_OUTPUT_NAME}"
      MACOSX_BUNDLE YES
      MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/standalone/macos/Info_${RENDERER}.plist"
   )

   set(RESOURCES_DIR "$<TARGET_FILE_DIR:vpinball>/../Resources")

   add_custom_command(TARGET vpinball POST_BUILD
      COMMAND "${CMAKE_COMMAND}" -E copy_directory "${CMAKE_SOURCE_DIR}/src/assets" "${RESOURCES_DIR}/assets"
      COMMAND "${CMAKE_COMMAND}" -E copy_directory "${CMAKE_SOURCE_DIR}/scripts" "${RESOURCES_DIR}/scripts"
      COMMAND "${CMAKE_COMMAND}" -E copy_directory "${CMAKE_SOURCE_DIR}/docs" "${RESOURCES_DIR}/docs"
   )

   if(RENDERER STREQUAL "GL")
      set(SHADER_DIR "${RESOURCES_DIR}/shaders-${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_REV}")
      add_custom_command(TARGET vpinball POST_BUILD
         COMMAND "${CMAKE_COMMAND}" -E make_directory "${SHADER_DIR}"
      )
      foreach(_shader IN LISTS VPX_GL_SHADERS)
         add_custom_command(TARGET vpinball POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E copy "${CMAKE_SOURCE_DIR}/src/shaders/hlsl_glsl/${_shader}" "${SHADER_DIR}"
         )
      endforeach()
   endif()

   add_custom_command(TARGET vpinball POST_BUILD
      COMMAND "${CMAKE_COMMAND}" -E make_directory "$<TARGET_FILE_DIR:vpinball>/../Frameworks"
      COMMAND cp -a "${RUNTIME_LIBS_DIR}/*.dylib" "$<TARGET_FILE_DIR:vpinball>/../Frameworks"
      COMMAND "${CMAKE_COMMAND}" -E copy "${CMAKE_SOURCE_DIR}/standalone/macos/VPinballX.icns" "$<TARGET_FILE_DIR:vpinball>/../Resources"
      COMMAND /usr/libexec/PlistBuddy -c "Set :CFBundleShortVersionString ${CMAKE_PROJECT_VERSION}" $<TARGET_FILE_DIR:vpinball>/../Info.plist
      COMMAND /usr/libexec/PlistBuddy -c "Set :CFBundleVersion ${CMAKE_PROJECT_VERSION}" $<TARGET_FILE_DIR:vpinball>/../Info.plist
   )

# Linux build (BGFX / GL)

elseif(PLATFORM STREQUAL "linux")
   set(CMAKE_INSTALL_RPATH "$ORIGIN")
   set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)

   if(RENDERER STREQUAL "GL")
      add_library(glad
         $<IF:$<STREQUAL:${ARCH},x64>,third-party/include/glad/src/gl.c,third-party/include/glad/src/gles2.c>
      )
      set_target_properties(glad PROPERTIES LINKER_LANGUAGE C)
      target_include_directories(glad PUBLIC
         third-party/include/glad/include
      )
   endif()

   add_executable(vpinball
      ${VPX_STANDALONE_SOURCES}
   )

   target_include_directories(vpinball PUBLIC
      ${CMAKE_SOURCE_DIR}/third-party/include
      ${CMAKE_SOURCE_DIR}
      ${CMAKE_SOURCE_DIR}/third-party/include/libwinevbs/atl/include
      ${CMAKE_SOURCE_DIR}/third-party/include/libwinevbs/atlmfc/include
      ${CMAKE_SOURCE_DIR}/third-party/include/libwinevbs/wine/include
      src
      plugins
   )

   target_compile_definitions(vpinball PRIVATE
      __STANDALONE__

      __LIBWINEVBS__
      __WINESRC__
      WINE_UNICODE_NATIVE

      "__forceinline=__attribute__((always_inline)) inline"
   )

   if(RENDERER STREQUAL "BGFX")
      target_compile_definitions(vpinball PRIVATE
         ENABLE_BGFX

         $<$<CONFIG:Debug>:BX_CONFIG_DEBUG=1>
         $<$<CONFIG:Release>:BX_CONFIG_DEBUG=0>
      )
   else()
      target_compile_definitions(vpinball PRIVATE
         ENABLE_OPENGL
         $<$<NOT:$<STREQUAL:${ARCH},x64>>:__OPENGLES__>
      )
   endif()

   target_compile_definitions(vpinball PRIVATE
      $<$<BOOL:${BUILD_RPI}>:__RPI__>
      $<$<BOOL:${BUILD_RK3588}>:__RK3588__>
   )

   target_compile_options(vpinball PUBLIC
      $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:
         -fdenormal-fp-math=preserve-sign
         -freciprocal-math
         -fassociative-math
         -fapprox-func
         -fno-math-errno
         -fno-trapping-math
         -ffp-contract=fast
         -Wno-shorten-64-to-32>
      $<$<CXX_COMPILER_ID:GNU>:
         -ffast-math
         -ffp-contract=fast
         -Wno-changes-meaning>
      $<$<STREQUAL:${ARCH},x64>:-mssse3>
   )

   # Disable Tree PTA to avoid GCC slowdown due to extreme inlining
   set_source_files_properties(src/core/Settings.cpp PROPERTIES
      COMPILE_FLAGS "$<$<CXX_COMPILER_ID:GNU>:-fno-tree-pta>"
   )

   set(RUNTIME_LIBS_DIR "${CMAKE_SOURCE_DIR}/third-party/runtime-libs/linux-${ARCH}")

   target_link_directories(vpinball PUBLIC
      "${RUNTIME_LIBS_DIR}"
   )

   target_link_libraries(vpinball PUBLIC
      SDL3
      SDL3_image
      SDL3_ttf
      freeimage
      hidapi-hidraw
      winevbs
   )

   target_link_libraries(vpinball PUBLIC $<IF:$<STREQUAL:${RENDERER},BGFX>,bgfx,glad>)

   set(RUNTIME_OUTPUT_NAME "$<IF:$<STREQUAL:${RENDERER},BGFX>,VPinballX_BGFX,VPinballX_GL>")

   set_target_properties(vpinball PROPERTIES
      RUNTIME_OUTPUT_NAME "${RUNTIME_OUTPUT_NAME}"
   )

   set(RESOURCES_DIR "$<TARGET_FILE_DIR:vpinball>")

   add_custom_command(TARGET vpinball POST_BUILD
      COMMAND "${CMAKE_COMMAND}" -E copy_directory "${CMAKE_SOURCE_DIR}/src/assets" "${RESOURCES_DIR}/assets"
      COMMAND "${CMAKE_COMMAND}" -E copy_directory "${CMAKE_SOURCE_DIR}/scripts" "${RESOURCES_DIR}/scripts"
      COMMAND "${CMAKE_COMMAND}" -E copy_directory "${CMAKE_SOURCE_DIR}/docs" "${RESOURCES_DIR}/docs"
   )

   if(RENDERER STREQUAL "GL")
      set(SHADER_DIR "${RESOURCES_DIR}/shaders-${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_REV}")
      add_custom_command(TARGET vpinball POST_BUILD
         COMMAND "${CMAKE_COMMAND}" -E make_directory "${SHADER_DIR}"
      )
      foreach(_shader IN LISTS VPX_GL_SHADERS)
         add_custom_command(TARGET vpinball POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E copy "${CMAKE_SOURCE_DIR}/src/shaders/hlsl_glsl/${_shader}" "${SHADER_DIR}"
         )
      endforeach()
   endif()

   if(POST_BUILD_COPY_EXT_LIBS)
      add_custom_command(TARGET vpinball POST_BUILD
         COMMAND cp -a "${RUNTIME_LIBS_DIR}/libSDL3.so" "$<TARGET_FILE_DIR:vpinball>"
         COMMAND cp -a "${RUNTIME_LIBS_DIR}/libSDL3.so.*" "$<TARGET_FILE_DIR:vpinball>"
         COMMAND cp -a "${RUNTIME_LIBS_DIR}/libSDL3_image.so" "$<TARGET_FILE_DIR:vpinball>"
         COMMAND cp -a "${RUNTIME_LIBS_DIR}/libSDL3_image.so.*" "$<TARGET_FILE_DIR:vpinball>"
         COMMAND cp -a "${RUNTIME_LIBS_DIR}/libSDL3_ttf.so" "$<TARGET_FILE_DIR:vpinball>"
         COMMAND cp -a "${RUNTIME_LIBS_DIR}/libSDL3_ttf.so.*" "$<TARGET_FILE_DIR:vpinball>"
         COMMAND cp -a "${RUNTIME_LIBS_DIR}/libfreeimage.so" "$<TARGET_FILE_DIR:vpinball>"
         COMMAND cp -a "${RUNTIME_LIBS_DIR}/libfreeimage.so.*" "$<TARGET_FILE_DIR:vpinball>"
         COMMAND cp -a "${RUNTIME_LIBS_DIR}/libhidapi-hidraw.so" "$<TARGET_FILE_DIR:vpinball>"
         COMMAND cp -a "${RUNTIME_LIBS_DIR}/libhidapi-hidraw.so.*" "$<TARGET_FILE_DIR:vpinball>"
         COMMAND cp -a "${RUNTIME_LIBS_DIR}/libwinevbs.so" "$<TARGET_FILE_DIR:vpinball>"
         COMMAND cp -a "${RUNTIME_LIBS_DIR}/libwinevbs.so.*" "$<TARGET_FILE_DIR:vpinball>"
      )
      if(RENDERER STREQUAL "BGFX")
         add_custom_command(TARGET vpinball POST_BUILD
            COMMAND cp -a "${RUNTIME_LIBS_DIR}/libbgfx.so" "$<TARGET_FILE_DIR:vpinball>"
         )
      endif()
   endif()

# iOS and Android libvpinball build

elseif(PLATFORM STREQUAL "ios" OR PLATFORM STREQUAL "ios-simulator" OR PLATFORM STREQUAL "android")

   set(VPINBALL_SOURCES ${VPX_LIB_SOURCES})

   if(PLATFORM STREQUAL "ios" OR PLATFORM STREQUAL "ios-simulator")
      list(APPEND VPINBALL_SOURCES
         lib/src/VPinballLib_iOS.h
         lib/src/VPinballLib_iOS.cpp
      )
   endif()

   if(PLATFORM STREQUAL "android")
      list(APPEND VPINBALL_SOURCES
         lib/bindings/java/jni/VPinballLib_JNI.cpp
      )
   endif()

   add_compile_options(-fPIC)

   add_library(vpinball SHARED ${VPINBALL_SOURCES})

   target_include_directories(vpinball PUBLIC
      ${CMAKE_SOURCE_DIR}/third-party/include
      ${CMAKE_SOURCE_DIR}
      ${CMAKE_SOURCE_DIR}/third-party/include/libwinevbs/atl/include
      ${CMAKE_SOURCE_DIR}/third-party/include/libwinevbs/atlmfc/include
      ${CMAKE_SOURCE_DIR}/third-party/include/libwinevbs/wine/include
      src
      plugins
   )

   target_compile_definitions(vpinball PRIVATE
      __LIBVPINBALL__
      __STANDALONE__

      __LIBWINEVBS__
      __WINESRC__
      WINE_UNICODE_NATIVE

      "__forceinline=__attribute__((always_inline)) inline"

      ENABLE_BGFX
      $<$<BOOL:${ENABLE_XR}>:ENABLE_XR>
      $<$<CONFIG:Debug>:BX_CONFIG_DEBUG=1>
      $<$<CONFIG:Release>:BX_CONFIG_DEBUG=0>
   )

   target_compile_options(vpinball PUBLIC
      -Wno-inconsistent-missing-override
      -Wno-missing-exception-spec
      $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:
         -fdenormal-fp-math=preserve-sign
         -freciprocal-math
         -fassociative-math
         -fapprox-func
         -fno-math-errno
         -fno-trapping-math
         -ffp-contract=fast
         -Wno-shorten-64-to-32>
      $<$<CXX_COMPILER_ID:GNU>:
         -ffast-math
         -ffp-contract=fast
         -Wno-changes-meaning>
   )

   set(BUILD_LIBS_DIR "${CMAKE_SOURCE_DIR}/third-party/build-libs/${PLATFORM}-${ARCH}")
   set(RUNTIME_LIBS_DIR "${CMAKE_SOURCE_DIR}/third-party/runtime-libs/${PLATFORM}-${ARCH}")

   target_link_directories(vpinball PUBLIC
      "${BUILD_LIBS_DIR}"
      "${RUNTIME_LIBS_DIR}"
   )

   target_link_libraries(vpinball PUBLIC
      bgfx
      bimg
      bimg_decode
      bx
      SDL3
      SDL3_image
      SDL3_ttf
      freeimage
      pinmame
      altsound
      zedmd
      serum
      dmdutil
      pupdmd
      vni
      sockpp
      dof
      avcodec
      avformat
      avutil
      swresample
      swscale
      winevbs
      zip
      AltSoundPlugin
      AlphaDMDPlugin
      B2SPlugin
      B2SLegacyPlugin
      DMDUtilPlugin
      DOFPlugin
      FlexDMDPlugin
      HelloScriptPlugin
      HelloWorldPlugin
      InspectorPlugin
      PinMAMEPlugin
      PUPPlugin
      RemoteControlPlugin
      ScoreViewPlugin
      SerumPlugin
      VNIPlugin
      WMPPlugin
      UpscaleDMDPlugin
      $<$<BOOL:${ENABLE_XR}>:openxr_loader>
   )

   if(PLATFORM STREQUAL "ios" OR PLATFORM STREQUAL "ios-simulator")
      target_link_libraries(vpinball PUBLIC
         freetype
         harfbuzz
         bz2
         iconv
         "-framework OpenGLES"
         "-framework AVFAudio"
         "-framework Metal"
         "-framework Foundation"
         "-framework QuartzCore"
         "-framework CoreMotion"
         "-framework CoreGraphics"
         "-framework CoreMedia"
         "-framework CoreVideo"
         "-framework VideoToolbox"
         "-framework GameController"
         "-framework AudioToolbox"
         "-framework CoreAudio"
         "-framework CoreHaptics"
         "-framework CoreServices"
         "-framework CoreText"
         "-framework AVFoundation"
         "-framework ImageIO"
         "-framework UIKit"
         "-framework CoreBluetooth"
      )
   elseif(PLATFORM STREQUAL "android")
      target_link_libraries(vpinball PUBLIC
         android
         log
         EGL
         GLESv3
         vulkan
      )
   endif()

   add_custom_command(TARGET vpinball POST_BUILD
      COMMAND "${CMAKE_COMMAND}" -E copy "${CMAKE_SOURCE_DIR}/lib/include/vpinball/VPinballLib_C.h" "$<TARGET_FILE_DIR:vpinball>"
      COMMAND "${CMAKE_COMMAND}" -E copy_directory "${CMAKE_SOURCE_DIR}/src/assets" "$<TARGET_FILE_DIR:vpinball>/assets"
      COMMAND "${CMAKE_COMMAND}" -E copy_directory "${CMAKE_SOURCE_DIR}/scripts" "$<TARGET_FILE_DIR:vpinball>/scripts"
      COMMAND "${CMAKE_COMMAND}" -E copy_directory "${CMAKE_SOURCE_DIR}/docs" "$<TARGET_FILE_DIR:vpinball>/docs"
   )

   if(PLATFORM STREQUAL "android")
      add_custom_command(TARGET vpinball POST_BUILD
         COMMAND cp -a "${RUNTIME_LIBS_DIR}/*.so" "$<TARGET_FILE_DIR:vpinball>"
      )
   endif()

endif()

include("${CMAKE_SOURCE_DIR}/make/CMakeLists_plugins.txt")
