cmake_minimum_required(VERSION 2.8.4)
set(NAME vpinball)
set(CMAKE_CONFIGURATION_TYPES Debug Release RelWithSSE RelWithDebInfo CACHE STRING "" FORCE)

project(${NAME})
enable_language(RC)

#
# Internal macro to enable precompiled headers
#
macro(add_msvc_precompiled_header PrecompiledHeader PrecompiledSource SourcesVar)
  if(MSVC)
    get_filename_component(PrecompiledBasename ${PrecompiledHeader} NAME_WE)
    set(PrecompiledBinary "${CMAKE_CURRENT_BINARY_DIR}/${PrecompiledBasename}.pch")
    set(Sources ${${SourcesVar}})

    SET_SOURCE_FILES_PROPERTIES(${PrecompiledSource}
      PROPERTIES COMPILE_FLAGS "/Yc\"${PrecompiledHeader}\" /Fp\"${PrecompiledBinary}\""
      OBJECT_OUTPUTS "${PrecompiledBinary}")
      
    SET_SOURCE_FILES_PROPERTIES(${Sources}
      PROPERTIES COMPILE_FLAGS "/Yu\"${PrecompiledHeader}\" /FI\"${PrecompiledHeader}\" /Fp\"${PrecompiledBinary}\""
      OBJECT_DEPENDS "${PrecompiledBinary}")  
      
    # Add precompiled header to SourcesVar
    list(APPEND ${SourcesVar} ${PrecompiledSource})
  endif(MSVC)
endmacro(add_msvc_precompiled_header)

#
# Test for DirectX. This will define:
# - DirectX_FOUND        - system has DirectX
# - DirectX_INCLUDE_DIR  - include directory for DirectX
# - DirectX_LIB_DIR      - lib directory for DirectX
#
# *****************************************************************************
# NOTE: If your DirectX SDK installation resides in a location other than
# those shown below, you may add your path to the 'find_path()' commands
# just before the closing bracket OR set the 'DXSDK_DIR' environment variable.
# *****************************************************************************
#
set(DirectX_FOUND "NO")
if(WIN32)
  find_path(DirectX_INCLUDE_DIR "d3d.h"
    "$ENV{DXSDK_DIR}/Include"
    "C:/Program Files/Microsoft DirectX SDK (August 2007)/Include"
    "C:/Programme/Microsoft DirectX SDK (August 2007)/Include"
    "C:/Program Files/Microsoft DirectX SDK (February 2007)/Include"
    "C:/Program Files (x86)/Microsoft DirectX SDK (August 2007)/Include"
    )

  find_path(DirectX_LIB_DIR "dinput.lib"
    "$ENV{DXSDK_DIR}/Lib/x86"
    "C:/Program Files/Microsoft DirectX SDK (August 2007)/Lib/x86"
    "C:/Programme/Microsoft DirectX SDK (August 2007)/Lib/x86"
    "C:/Program Files/Microsoft DirectX SDK (February 2007)/Lib/x86"
    "C:/Program Files (x86)/Microsoft DirectX SDK (August 2007)/Lib/x86"
    )

  if(DirectX_INCLUDE_DIR AND DirectX_LIB_DIR)
      set(DirectX_FOUND "YES")
  endif(DirectX_INCLUDE_DIR AND DirectX_LIB_DIR)
else()
endif()

if(DirectX_FOUND)
  message(STATUS "Found DirectX.")
  message(STATUS "  Includes: ${DirectX_INCLUDE_DIR}")
  message(STATUS "  Libs    : ${DirectX_LIB_DIR}")
else()
  message(FATAL_ERROR "Could not find DirectX. Please check/adapt search path in CMakeLists.txt or set DXSDK_DIR env var.")
endif()

#
# Test for ATL headers. This will try to search in alternate directories to
# enable compilation with Visual Studio Express.
#
find_path(ATL_BASEPATH atlbase.h HINTS
  "C:/Program Files/Microsoft Platform SDK for Windows Server 2003 R2/Include/atl"
  "C:/Programme/Microsoft Platform SDK for Windows Server 2003 R2/Include/atl"
  DOC "ATL Include Directory"
  )
find_path(MFC_BASEPATH winres.h HINTS
  "C:/Program Files/Microsoft Platform SDK for Windows Server 2003 R2/Include/mfc"
  "C:/Programme/Microsoft Platform SDK for Windows Server 2003 R2/Include/mfc"
  DOC "MFC Include Directory"
  )

if(ATL_BASEPATH MATCHES "NOTFOUND")
  message(FATAL_ERROR "Could not find required header 'atlbase.h'. Please check/adapt the ATL search path in CMakeLists.txt")
else()
  message(STATUS "Found 'atlbase.h': ${ATL_BASEPATH}")
endif()
if(MFC_BASEPATH MATCHES "NOTFOUND")
  message(FATAL_ERROR "Could not find required header 'winres.h'. Please check/adapt the MFC search path in CMakeLists.txt")
else()
  message(STATUS "Found 'winres.h': ${MFC_BASEPATH}")
endif()


set(SOURCES
  ballex.cpp
  bumper.cpp
  codeview.cpp
  comcontrol.cpp
  decal.cpp
  def.cpp
  dispreel.cpp
  disputil.cpp
  dragpoint.cpp
  eventproxy.cpp
  extern.cpp
  flasher.cpp
  flipper.cpp
  gate.cpp
  hid.cpp
  hitrectsur.cpp
  hitsur.cpp
  ieditable.cpp
  iselect.cpp
  kicker.cpp
  light.cpp
  lightseq.cpp
  Material.cpp
  memutil.cpp
  mesh.cpp
  mixer.cpp
  objloader.cpp
  paintsur.cpp
  pin3d.cpp
  pinbinary.cpp
  pinimage.cpp
  pininput.cpp
  pinsound.cpp
  pintable.cpp
  pinundo.cpp
  plumb.cpp
  plunger.cpp
  primitive.cpp
  propbrowser.cpp
  ramp.cpp
  regutil.cpp
  RenderDevice.cpp
  shadowsur.cpp
  slintf.cpp
  spinner.cpp
  surface.cpp
  tableglobal.cpp
  textbox.cpp
  Texture.cpp
  timer.cpp
  trigger.cpp
  variant.cpp
  vbatest.cpp
  vpinball.cpp
  wintimer.cpp
  worker.cpp
  xaudplayer.cpp  
  pin/ball.cpp
  pin/collide.cpp
  pin/collideex.cpp
  pin/hitflipper.cpp
  pin/hitplunger.cpp
  pin/player.cpp
  media/fileio.cpp
  media/lzwreader.cpp
  media/lzwwriter.cpp
  media/wavread.cpp
  )

set(HEADERS
  ballex.h
  buildnum.h
  buildnumber.h
  bumper.h
  codeview.h
  comcontrol.h
  decal.h
  def.h
  dispid.h
  display.h
  dispreel.h
  disputil.h
  DongleAPI.h
  dragpoint.h
  eventproxy.h
  extern.h
  exvector.h
  flipper.h
  freeimage.h
  gate.h
  helpers.h
  hid.h
  hidpi.h
  hidsdi.h
  hidusage.h
  hitrectsur.h
  hitsur.h
  idebug.h
  ieditable.h
  iselect.h
  kicker.h
  light.h
  lightseq.h
  main.h
  memutil.h
  mesh.h
  mixer.h
  msscript.h
  paintsur.h
  pin3d.h
  pinbinary.h
  pinimage.h
  pininput.h
  pinsound.h
  pintable.h
  pinundo.h
  plumb.h
  plunger.h
  primitive.h
  propbrowser.h
  ramp.h
  regutil.h
  resource.h
  shadowsur.h
  slintf.h
  spinner.h
  stdafx.h
  sur.h
  surface.h
  SVNRevision.h
  tableglobal.h
  textbox.h
  timer.h
  trigger.h
  variant.h
  vector.h
  vectorsort.h
  vpinball.h
  wintimer.h
  worker.h
  xaudplayer.h
  pin/ball.h
  pin/collide.h
  pin/collideex.h
  pin/draw.h
  pin/hitable.h
  pin/hitflipper.h
  pin/hitplunger.h
  pin/hittimer.h
  pin/player.h
  media/fileio.h
  media/lzwreader.h
  media/lzwwriter.h
  media/wavread.h
  )

set(RC_RESOURCE
  vbatest.rc
  )
 
# dummy collection of resources - not used in build (resource compiler takes care of everything)
# (only listed for completeness)
#set(RESOURCES
#  arrow.cur ball.rgs balltex2.bmp basetable17.vpt bumper.rgs chevron.bmp comcontrol.rgs
#  cur00001.cur cursor1.cur cur_bump.cur cur_deca.cur cur_dispdmd.cur cur_dispreel.cur
#  cur_dispseg.cur cur_flip.cur cur_gate.cur cur_kick.cur cur_ligh.cur cur_lightseq.cur
#  cur_plun.cur cur_prim.cur cur_spin.cur cur_targ.cur cur_text.cur cur_time.cur
#  cur_wall.cur dispreel.rgs dragpoint.rgs flipper.rgs gate.rgs ico00001.ico icon1.ico
#  iconflipper.bmp iconlight.bmp icontimer.bmp icontrigger.bmp kicker.rgs light.rgs
#  lightseq.rgs logo25.bmp logo28.bmp pause1.ico pieventhandler.rgs play1.ico plunger.rgs
#  Primitive.rgs ramp.rgs spinner.rgs step1.ico sunburst.bmp sunburst3.bmp sunburst4.bmp
#  surface.rgs tableicon.ico target.bmp textbox.rgs timer.rgs toolbar.bmp toolbardebug.bmp
#  trigger.rgs vbatest.rgs wall.cur white.bmp
#  )

# generate com interfaces with midl
ADD_CUSTOM_COMMAND(OUTPUT "${CMAKE_BINARY_DIR}/VBATest.h"
  "${CMAKE_BINARY_DIR}/VBATest.tlb" "${CMAKE_BINARY_DIR}/VBATest_i.c"
  COMMAND midl.exe /win32 "${CMAKE_SOURCE_DIR}/vbatest.idl"
  DEPENDS "${CMAKE_SOURCE_DIR}/vbatest.idl")

add_msvc_precompiled_header(stdafx.h stdafx.cpp SOURCES)
list(APPEND HEADERS "${CMAKE_BINARY_DIR}/VBATest.h")

include_directories(pin)
include_directories(media)
include_directories(inc)
include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_BINARY_DIR})
include_directories(${DirectX_INCLUDE_DIR})
include_directories(${ATL_BASEPATH} ${MFC_BASEPATH})

#
# Custom Build configuration settings
#
add_definitions(-D_ATL_STATIC_REGISTRY -D_WIN32_WINNT=0x0501 /Yu"stdafx.h")
add_definitions(-D_CRT_SECURE_NO_WARNINGS)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
string(REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG})
string(REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE})
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /DCOR_ENABLE_PROFILING /DMYDEBUG /D_DEBUGPHYSICS /MTd")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT /Ox /Ob2 /Oi /Ot /GL /GF /GS- /GR- /fp:fast")
set(CMAKE_CXX_FLAGS_RELWITHSSE "${CMAKE_CXX_FLAGS_RELWITHSSE} /MT /Ox /Ob2 /Oi /Ot /GL /GF /GS- /GR- /fp:fast /arch:SSE")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /MT /Ox /Ob2 /Oi /Ot /GR-")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:LIBC;atlthunk")
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /NODEFAULTLIB:LIBCMT;LIBCD")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /LTCG")
set(CMAKE_EXE_LINKER_FLAGS_RELWITHSSE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /LTCG")


find_library(LIB_DXGUID dxguid.lib PATHS ${DirectX_LIB_DIR})
find_library(LIB_FREEIMAGE freeimage.lib PATHS ${CMAKE_SOURCE_DIR}/lib)
find_library(LIB_DSOUND dsound.lib PATHS ${DirectX_LIB_DIR})
find_library(LIB_DINPUT dinput.lib PATHS ${DirectX_LIB_DIR})
find_library(LIB_XAUDIO xaudio.lib PATHS ${CMAKE_SOURCE_DIR}/lib)
find_library(LIB_HID hid.lib PATHS ${CMAKE_SOURCE_DIR}/lib)

add_executable(${NAME} WIN32 ${SOURCES} ${HEADERS} ${RC_RESOURCE})
target_link_libraries(${NAME} comctl32.lib ${LIB_DXGUID} ${LIB_FREEIMAGE} ${LIB_DSOUND} winmm.lib ${LIB_DINPUT} 
  ${LIB_XAUDIO} imm32.lib setupapi.lib ${LIB_HID})


#
# Very simple install target
#
message(STATUS "Installer will copy files to: ${CMAKE_INSTALL_PREFIX}")
install(TARGETS ${NAME} DESTINATION ${CMAKE_INSTALL_PREFIX})
install(FILES ${CMAKE_SOURCE_DIR}/FreeImage.dll DESTINATION ${CMAKE_INSTALL_PREFIX})
