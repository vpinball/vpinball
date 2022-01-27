cmake_minimum_required(VERSION 3.16)

file(READ vpversion.h version)
string(REGEX MATCH "VP_VERSION_MAJOR[ ]+([0-9]+)" _tmp ${version})
set(VERSION_MAJOR "${CMAKE_MATCH_1}")
string(REGEX MATCH "VP_VERSION_MINOR[ ]+([0-9]+)" _tmp ${version})
set(VERSION_MINOR "${CMAKE_MATCH_1}")
string(REGEX MATCH "VP_VERSION_REV[ ]+([0-9]+)" _tmp ${version})
set(VERSION_REV "${CMAKE_MATCH_1}")

project(vpinball VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_REV}")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD 99)

add_compile_options(
   $<$<CONFIG:RELEASE>:/Ob2>
   $<$<CONFIG:RELEASE>:/O2>
   $<$<CONFIG:RELEASE>:/Oi>
   $<$<CONFIG:RELEASE>:/fp:fast>
   $<$<CONFIG:RELEASE>:/fp:except->
   $<$<CONFIG:RELEASE>:/Ot>
   $<$<CONFIG:RELEASE>:/GF>
   $<$<CONFIG:RELEASE>:/GS->
   $<$<CONFIG:RELEASE>:/Gy>
   $<$<CONFIG:RELEASE>:/GR->
   $<$<CONFIG:RELEASE>:/Oy>
   $<$<CONFIG:RELEASE>:/GT>
   $<$<CONFIG:RELEASE>:/GL>
   $<$<CONFIG:RELEASE>:/w44005>
)

set_source_files_properties(
   vpinball.rc LANGUAGE RC
)

set_source_files_properties(shader/BallShader.hlsl PROPERTIES 
   VS_SHADER_TYPE Effect 
   VS_SHADER_MODEL 2.0 
   VS_SHADER_OUTPUT_HEADER_FILE "${CMAKE_CURRENT_BINARY_DIR}/$(IntDir)/shader/%(Filename).h"
   VS_SHADER_ENTRYPOINT ballShaderCode
)

set_source_files_properties(shader/BasicShader.hlsl PROPERTIES
   VS_SHADER_TYPE Effect 
   VS_SHADER_MODEL 2.0
   VS_SHADER_OUTPUT_HEADER_FILE "${CMAKE_CURRENT_BINARY_DIR}/$(IntDir)/shader/%(Filename).h"
   VS_SHADER_ENTRYPOINT basicShaderCode
)

set_source_files_properties(shader/DMDShader.hlsl PROPERTIES
   VS_SHADER_TYPE Effect
   VS_SHADER_MODEL 2.0 
   VS_SHADER_OUTPUT_HEADER_FILE "${CMAKE_CURRENT_BINARY_DIR}/$(IntDir)/shader/%(Filename).h"
   VS_SHADER_ENTRYPOINT dmdShaderCode
)

set_source_files_properties(shader/FBShader.hlsl PROPERTIES
   VS_SHADER_TYPE Effect 
   VS_SHADER_MODEL 2.0
   VS_SHADER_OUTPUT_HEADER_FILE "${CMAKE_CURRENT_BINARY_DIR}/$(IntDir)/shader/%(Filename).h"
   VS_SHADER_ENTRYPOINT FBShaderCode
)

set_source_files_properties(shader/FlasherShader.hlsl PROPERTIES
   VS_SHADER_TYPE Effect 
   VS_SHADER_MODEL 2.0
   VS_SHADER_OUTPUT_HEADER_FILE "${CMAKE_CURRENT_BINARY_DIR}/$(IntDir)/shader/%(Filename).h"
   VS_SHADER_ENTRYPOINT flasherShaderCode
)

set_source_files_properties(shader/LightShader.hlsl PROPERTIES
   VS_SHADER_TYPE Effect
   VS_SHADER_MODEL 2.0
   VS_SHADER_OUTPUT_HEADER_FILE "${CMAKE_CURRENT_BINARY_DIR}/$(IntDir)/shader/%(Filename).h"
   VS_SHADER_ENTRYPOINT lightShaderCode 
)

add_executable(vpinball WIN32
   BlackBox.cpp
   BlackBox.h
   codeviewedit.cpp
   codeviewedit.h
   CrashHandler.cpp
   CrashHandler.h
   Debugger.cpp
   Debugger.h
   dialogs/AboutDialog.cpp
   dialogs/AboutDialog.h
   dialogs/AudioOptionsDialog.cpp
   dialogs/AudioOptionsDialog.h
   dialogs/CollectionManagerDialog.cpp
   dialogs/CollectionManagerDialog.h
   dialogs/DimensionDialog.cpp
   dialogs/DimensionDialog.h
   dialogs/DrawingOrderDialog.cpp
   dialogs/DrawingOrderDialog.h
   dialogs/EditorOptionsDialog.cpp
   dialogs/EditorOptionsDialog.h
   dialogs/ImageDialog.cpp
   dialogs/ImageDialog.h
   dialogs/KeysConfigDialog.cpp
   dialogs/KeysConfigDialog.h
   dialogs/LayersListDialog.cpp
   dialogs/LayersListDialog.h
   dialogs/MaterialDialog.cpp
   dialogs/MaterialDialog.h
   dialogs/NotesDialog.cpp
   dialogs/NotesDialog.h
   dialogs/ScriptErrorDialog.cpp
   dialogs/ScriptErrorDialog.h
   dialogs/PhysicsOptionsDialog.cpp
   dialogs/PhysicsOptionsDialog.h
   dialogs/Properties/BackglassCameraProperty.cpp
   dialogs/Properties/BackglassCameraProperty.h
   dialogs/Properties/BackglassVisualsProperty.cpp
   dialogs/Properties/BackglassVisualsProperty.h
   dialogs/Properties/BumperPhysicsProperty.cpp
   dialogs/Properties/BumperPhysicsProperty.h
   dialogs/Properties/BumperVisualsProperty.cpp
   dialogs/Properties/BumperVisualsProperty.h
   dialogs/Properties/DecalVisualsProperty.cpp
   dialogs/Properties/DecalVisualsProperty.h
   dialogs/Properties/DispreelStateProperty.cpp
   dialogs/Properties/DispreelStateProperty.h
   dialogs/Properties/DispreelVisualsProperty.cpp
   dialogs/Properties/DispreelVisualsProperty.h
   dialogs/Properties/DragpointVisualsProperty.cpp
   dialogs/Properties/DragpointVisualsProperty.h
   dialogs/Properties/FlasherVisualsProperty.cpp
   dialogs/Properties/FlasherVisualsProperty.h
   dialogs/Properties/FlipperPhysicsProperty.cpp
   dialogs/Properties/FlipperVisualsProperty.h
   dialogs/Properties/FlipperPhysicsProperty.h
   dialogs/Properties/FlipperVisualsProperty.cpp
   dialogs/Properties/GatePhysicsProperty.cpp
   dialogs/Properties/GatePhysicsProperty.h
   dialogs/Properties/GateVisualsProperty.cpp
   dialogs/Properties/GateVisualsProperty.h
   dialogs/Properties/HitTargetPhysicsProperty.cpp
   dialogs/Properties/HitTargetPhysicsProperty.h
   dialogs/Properties/HitTargetVisualsProperty.cpp
   dialogs/Properties/HitTargetVisualsProperty.h
   dialogs/Properties/KickerPhysicsProperty.cpp
   dialogs/Properties/KickerPhysicsProperty.h
   dialogs/Properties/KickerVisualsProperty.cpp
   dialogs/Properties/KickerVisualsProperty.h
   dialogs/Properties/LightseqStatesProperty.cpp
   dialogs/Properties/LightseqStatesProperty.h
   dialogs/Properties/LightStatesProperty.cpp
   dialogs/Properties/LightStatesProperty.h
   dialogs/Properties/LightVisualsProperty.cpp
   dialogs/Properties/LightVisualsProperty.h
   dialogs/Properties/PlungerPhysicsProperty.cpp
   dialogs/Properties/PlungerPhysicsProperty.h
   dialogs/Properties/PlungerVisualsProperty.cpp
   dialogs/Properties/PlungerVisualsProperty.h
   dialogs/Properties/PrimitivePhysicsProperty.cpp
   dialogs/Properties/PrimitivePhysicsProperty.h
   dialogs/Properties/PrimitivePositionProperty.cpp
   dialogs/Properties/PrimitivePositionProperty.h
   dialogs/Properties/PrimitiveVisualsProperty.cpp
   dialogs/Properties/PrimitiveVisualsProperty.h
   dialogs/Properties/PropertyDialog.cpp
   dialogs/Properties/PropertyDialog.h
   dialogs/Properties/RampPhysicsProperty.cpp
   dialogs/Properties/RampPhysicsProperty.h
   dialogs/Properties/RampVisualsProperty.cpp
   dialogs/Properties/RampVisualsProperty.h
   dialogs/Properties/RubberPhysicsProperty.cpp
   dialogs/Properties/RubberPhysicsProperty.h
   dialogs/Properties/RubberVisualsProperty.cpp
   dialogs/Properties/RubberVisualsProperty.h
   dialogs/Properties/SpinnerPhysicsProperty.cpp
   dialogs/Properties/SpinnerVisualsProperty.h
   dialogs/Properties/SpinnerVisualsProperty.cpp
   dialogs/Properties/SpinnerPhysicsProperty.h
   dialogs/Properties/TableCustomProperty.cpp
   dialogs/Properties/TableCustomProperty.h
   dialogs/Properties/TableLightsProperty.cpp
   dialogs/Properties/TableLightsProperty.h
   dialogs/Properties/TablePhysicsProperty.cpp
   dialogs/Properties/TablePhysicsProperty.h
   dialogs/Properties/TableVisualsProperty.cpp
   dialogs/Properties/TableVisualsProperty.h
   dialogs/Properties/TextboxVisualsProperty.cpp
   dialogs/Properties/TextboxVisualsProperty.h
   dialogs/Properties/TriggerPhysicsProperty.cpp
   dialogs/Properties/TriggerPhysicsProperty.h
   dialogs/Properties/TriggerVisualsProperty.cpp
   dialogs/Properties/TriggerVisualsProperty.h
   dialogs/Properties/WallPhysicsProperty.cpp
   dialogs/Properties/WallPhysicsProperty.h
   dialogs/Properties/WallVisualsProperty.cpp
   dialogs/Properties/WallVisualsProperty.h
   dialogs/SearchSelectDialog.cpp
   dialogs/SearchSelectDialog.h
   dialogs/SoundDialog.cpp
   dialogs/SoundDialog.h
   dialogs/TableInfoDialog.cpp
   dialogs/TableInfoDialog.h
   dialogs/ToolbarDialog.cpp
   dialogs/ToolbarDialog.h
   dialogs/VideoOptionsDialog.cpp
   dialogs/VideoOptionsDialog.h
   flasher.cpp
   flasher.h
   hittarget.cpp
   hittarget.h
   imgui/imconfig.h
   imgui/imgui.cpp
   imgui/imgui.h
   imgui/imgui_draw.cpp
   imgui/imgui_impl_dx9.cpp
   imgui/imgui_impl_dx9.h
   imgui/imgui_impl_win32.cpp
   imgui/imgui_impl_win32.h
   imgui/imgui_tables.cpp
   imgui/imgui_widgets.cpp
   imgui/imgui_internal.h
   imgui/implot/implot.cpp
   imgui/implot/implot.h
   imgui/implot/implot_items.cpp
   imgui/implot/implot_internal.h
   inc/gpuprofiler.cpp
   inc/gpuprofiler.h
   inc/serial.cpp
   inc/serial.h
   inc/bass.h
   inc/forsyth.h
   inc/freeimage.h
   inc/hidpi.h
   inc/hidsdi.h
   inc/hidusage.h
   inc/nvapi.h
   inc/nvapi_lite_common.h
   inc/nvapi_lite_d3dext.h
   inc/nvapi_lite_salend.h
   inc/nvapi_lite_salstart.h
   inc/nvapi_lite_sli.h
   inc/nvapi_lite_stereo.h
   inc/nvapi_lite_surround.h
   inc/progmesh.cpp
   inc/progmesh.h
   inc/progmesh_vecmatquat_minimal.h
   inc/scilexer.h
   inc/scintilla.h
   inc/Sci_Position.h
   inc/stb_image.h
   inc/ThreadPool.h
   inc/win32xx/include/wxx_appcore.h
   inc/win32xx/include/wxx_appcore0.h
   inc/win32xx/include/wxx_archive.h
   inc/win32xx/include/wxx_commondlg.h
   inc/win32xx/include/wxx_controls.h
   inc/win32xx/include/wxx_cstring.h
   inc/win32xx/include/wxx_ddx.h
   inc/win32xx/include/wxx_dialog.h
   inc/win32xx/include/wxx_dockframe.h
   inc/win32xx/include/wxx_docking.h
   inc/win32xx/include/wxx_exception.h
   inc/win32xx/include/wxx_file.h
   inc/win32xx/include/wxx_filefind.h
   inc/win32xx/include/wxx_folderdialog.h
   inc/win32xx/include/wxx_frame.h
   inc/win32xx/include/wxx_gdi.h
   inc/win32xx/include/wxx_imagelist.h
   inc/win32xx/include/wxx_listview.h
   inc/win32xx/include/wxx_mdi.h
   inc/win32xx/include/wxx_menu.h
   inc/win32xx/include/wxx_menubar.h
   inc/win32xx/include/wxx_metafile.h
   inc/win32xx/include/wxx_mutex.h
   inc/win32xx/include/wxx_printdialogs.h
   inc/win32xx/include/wxx_propertysheet.h
   inc/win32xx/include/wxx_rebar.h
   inc/win32xx/include/wxx_rect.h
   inc/win32xx/include/wxx_regkey.h
   inc/win32xx/include/wxx_ribbon.h
   inc/win32xx/include/wxx_richedit.h
   inc/win32xx/include/wxx_scrollview.h
   inc/win32xx/include/wxx_shared_ptr.h
   inc/win32xx/include/wxx_socket.h
   inc/win32xx/include/wxx_statusbar.h
   inc/win32xx/include/wxx_stdcontrols.h
   inc/win32xx/include/wxx_tab.h
   inc/win32xx/include/wxx_taskdialog.h
   inc/win32xx/include/wxx_textconv.h
   inc/win32xx/include/wxx_themes.h
   inc/win32xx/include/wxx_time.h
   inc/win32xx/include/wxx_toolbar.h
   inc/win32xx/include/wxx_treeview.h
   inc/win32xx/include/wxx_wceframe.h
   inc/win32xx/include/wxx_wcestddef.h
   inc/win32xx/include/wxx_webbrowser.h
   inc/win32xx/include/wxx_wincore.h
   inc/win32xx/include/wxx_wincore0.h
   IndexBuffer.cpp
   IndexBuffer.h
   math/bbox.h
   math/math.cpp
   math/math.h
   math/matrix.cpp
   math/matrix.h
   math/vector.h
   media/fileio.cpp
   media/fileio.h
   media/lzwreader.cpp
   media/lzwreader.h
   media/lzwwriter.cpp
   media/lzwwriter.h
   media/wavread.cpp
   media/wavread.h
   MemoryStatus.cpp
   MemoryStatus.h
   rubber.cpp
   hash.cpp
   hash.h
   kdtree.cpp
   kdtree.h
   objloader.cpp
   objloader.h
   pin/ball.cpp
   pin/ball.h
   pin/collide.cpp
   pin/collide.h
   pin/collideex.cpp
   pin/collideex.h
   pin/hitable.h
   pin/hitflipper.cpp
   pin/hitflipper.h
   pin/hitplunger.cpp
   pin/hitplunger.h
   pin/hittimer.h
   pin/player.cpp
   pin/player.h
   ballex.cpp
   ballex.h
   bumper.cpp
   bumper.h
   codeview.cpp
   codeview.h
   decal.cpp
   decal.h
   def.cpp
   def.h
   dispid.h
   dispreel.cpp
   dispreel.h
   disputil.cpp
   disputil.h
   dragpoint.cpp
   dragpoint.h
   editablereg.h
   eventproxy.cpp
   eventproxy.h
   extern.cpp
   extern.h
   flipper.cpp
   flipper.h
   gate.cpp
   gate.h
   helpers.h
   hid.cpp
   hid.h
   hitrectsur.cpp
   hitrectsur.h
   hitsur.cpp
   hitsur.h
   idebug.h
   ieditable.cpp
   ieditable.h
   iselect.cpp
   iselect.h
   kicker.cpp
   kicker.h
   light.cpp
   light.h
   lightseq.cpp
   lightseq.h
   Material.h
   memutil.cpp
   memutil.h
   mesh.cpp
   mesh.h
   meshes/ballMesh.h
   meshes/bulbLightMesh.h
   meshes/bulbSocketMesh.h
   meshes/bumperBaseMesh.h
   meshes/bumperCapMesh.h
   meshes/bumperRingMesh.h
   meshes/bumperSocketMesh.h
   meshes/dropTargetT2Mesh.h
   meshes/dropTargetT3Mesh.h
   meshes/dropTargetT4Mesh.h
   meshes/flipperBase.h
   meshes/gateBracketMesh.h
   meshes/gateLongPlateMesh.h
   meshes/gatePlateMesh.h
   meshes/gateWireMesh.h
   meshes/gateWireRectangleMesh.h
   meshes/hitTargetFatRectangleMesh.h
   meshes/hitTargetFatSquareMesh.h
   meshes/hitTargetRectangleMesh.h
   meshes/hitTargetRoundMesh.h
   meshes/hitTargetT1SlimMesh.h
   meshes/hitTargetT2SlimMesh.h
   meshes/kickerCupMesh.h
   meshes/kickerHitMesh.h
   meshes/kickerHoleMesh.h
   meshes/kickerSimpleHoleMesh.h
   meshes/spinnerBracketMesh.h
   meshes/spinnerPlateMesh.h
   meshes/triggerSimpleMesh.h
   meshes/triggerStarMesh.h
   mixer.cpp
   mixer.h
   paintsur.cpp
   paintsur.h
   physconst.h
   pin3d.cpp
   pin3d.h
   pinbinary.cpp
   pinbinary.h
   pininput.cpp
   pininput.h
   pinsound.cpp
   pinsound.h
   pintable.cpp
   pintable.h
   pinundo.cpp
   pinundo.h
   plumb.cpp
   plumb.h
   plunger.cpp
   plunger.h
   primitive.cpp
   primitive.h
   quadtree.cpp
   quadtree.h
   ramp.cpp
   ramp.h
   regutil.cpp
   regutil.h
   RenderDevice.cpp
   RenderDevice.h
   resource.h
   rubber.h
   Shader.h
   slintf.cpp
   slintf.h
   spinner.cpp
   spinner.h
   StackTrace.cpp
   StackTrace.h
   stdafx.cpp
   stdafx.h
   surface.cpp
   surface.h
   textbox.cpp
   textbox.h
   Texture.cpp
   Texture.h
   TextureManager.cpp
   TextureManager.h
   timer.cpp
   timer.h
   trace.h
   trigger.cpp
   trigger.h
   variant.cpp
   variant.h
   vector.h
   vectorsort.h
   main.cpp
   main.h
   VertexBuffer.cpp
   VertexBuffer.h
   vpinball.cpp
   vpinball_h.h
   vpversion.h
   wintimer.cpp
   wintimer.h
   worker.cpp
   worker.h
   audioplayer.cpp
   audioplayer.h

   vpinball.rc
   vpinball.idl

   shader/BallShader.hlsl
   shader/BasicShader.hlsl
   shader/DMDShader.hlsl
   shader/FBShader.hlsl
   shader/FlasherShader.hlsl
   shader/LightShader.hlsl

   res/ball.rgs
   res/bumper.rgs
   res/hittarget.rgs
   res/dispreel.rgs
   res/dragpoint.rgs
   res/flasher.rgs
   res/rubber.rgs
   res/flipper.rgs
   res/gate.rgs
   res/kicker.rgs
   res/light.rgs
   res/lightseq.rgs
   res/pieventhandler.rgs
   res/plunger.rgs
   res/primitive.rgs
   res/ramp.rgs
   res/spinner.rgs
   res/surface.rgs
   res/textbox.rgs
   res/timer.rgs
   res/trigger.rgs
   res/vpinball.rgs
)

target_include_directories(vpinball PUBLIC
   $ENV{DXSDK_DIR}/Include
   dialogs
   shader
   inc
   inc/win32xx/include
   pin
   media
   ${CMAKE_CURRENT_SOURCE_DIR}
   ${CMAKE_CURRENT_BINARY_DIR}/$(IntDir)
   ${CMAKE_CURRENT_BINARY_DIR}/$(IntDir)/shader
)

target_precompile_headers(vpinball PUBLIC
   stdafx.h
)

target_link_directories(vpinball PUBLIC 
   $ENV{DXSDK_DIR}/Lib/x64
   lib/x64
)

target_link_libraries(vpinball
   comctl32.lib
   FreeImage.lib
   dxguid.lib
   dsound.lib 
   bass.lib
   dinput.lib
   nvapi.lib
   winmm.lib
   hid.lib
   setupapi.lib
)

target_link_options(vpinball PUBLIC
   $<$<CONFIG:RELEASE>:/SAFESEH:NO>
   $<$<CONFIG:RELEASE>:/INCREMENTAL:NO>
   $<$<CONFIG:RELEASE>:/OPT:REF>
   $<$<CONFIG:RELEASE>:/OPT:ICF>
   $<$<CONFIG:RELEASE>:/LTCG>
   $<$<CONFIG:RELEASE>:/DYNAMICBASE:NO>
)

set_target_properties(vpinball PROPERTIES
   VS_DPI_AWARE "On"
   RUNTIME_OUTPUT_NAME "VPinballX"
   MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:DEBUG>:Debug>"
)
