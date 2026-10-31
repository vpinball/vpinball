# Script-mode generator: cmake_minimum_required sets the policies needed by
# IN_LIST (CMP0057, >=3.3) and cmake_language(CALL) (>=3.18).
cmake_minimum_required(VERSION 3.18)

# Compiles the VPX BGFX shaders into embeddable headers (bgfx_*.h) using bgfx's
# shaderc. Single implementation used by the CMake build (make/CMakeLists_shaders.txt),
# the MSVC project (make/vpx-core.vcxitems custom build step) and the shaders.sh /
# shaders.ps1 convenience wrappers.
#
# Invoked in script mode:
#   cmake -DSHADERC=<path to shaderc> -DOUT_DIR=<header out dir> -DWORK_DIR=<cache dir>
#         -DBACKENDS=<comma separated backends> [-DGROUP=<group>] [-DDBGSHADER=1]
#         -P shaders.cmake
#
# BACKENDS lists the shader binary formats to compile for real (subset of
# mtl,essl,glsl,dxbc,dxil,spv,wgsl). Every other backend is emitted as a stub array so
# that BGFX_EMBEDDED_SHADER() references stay valid on every build platform
# (e.g. dxbc/dxil cannot be produced by a macOS/Linux hosted shaderc).
#
# Each (variant, backend) compile result is cached as a fragment file in WORK_DIR:
# a fragment is only recompiled when its source, the shared includes, or the
# shaderc command line changed. Group headers are reassembled from the fragments
# and copied to OUT_DIR only when their content actually changed.

if(NOT SHADERC OR NOT OUT_DIR OR NOT WORK_DIR)
   message(FATAL_ERROR "shaders.cmake: SHADERC, OUT_DIR and WORK_DIR are required")
endif()
if(NOT EXISTS "${SHADERC}")
   message(FATAL_ERROR "shaders.cmake: shaderc not found at ${SHADERC} (run the platform external.sh build first)")
endif()

set(SRC_DIR "${CMAKE_CURRENT_LIST_DIR}")
set(FRAG_DIR "${WORK_DIR}/frag")
file(MAKE_DIRECTORY "${OUT_DIR}")
file(MAKE_DIRECTORY "${FRAG_DIR}")

string(REPLACE "," ";" BACKENDS "${BACKENDS}")

set(ALL_BACKENDS mtl essl glsl dxbc dxil spv wgsl)
set(BACKEND_mtl  --platform osx     -p metal21-11)
set(BACKEND_essl --platform android -p 320_es)
set(BACKEND_glsl --platform linux   -p 440)
set(BACKEND_dxbc --platform windows -p s_5_0 -O 3)
set(BACKEND_dxil --platform windows -p s_6_0 -O 3)
set(BACKEND_spv  --platform linux   -p spirv)
set(BACKEND_wgsl --platform linux   -p wgsl)

# Files every shader depends on: a change recompiles all fragments.
set(SHADER_DEPS
   "${SRC_DIR}/varying.def.sc"
   "${SRC_DIR}/bgfx_shader.sh"
   "${SRC_DIR}/bgfx_compute.sh"
   "${SRC_DIR}/common.sh"
   "${SRC_DIR}/material.sh"
   "${SRC_DIR}/ball_shadows.sh"
   "${SRC_DIR}/SMAA.sh"
   "${SRC_DIR}/SMAA.hlsl"
   "${SHADERC}"
   "${CMAKE_CURRENT_LIST_FILE}")

set(_tmp "")

# process_shader(<group> <source> <name> <type> [defines...])
# <name> ends with a trailing underscore; blob symbols are <name><backend>.
function(process_shader group source name type)
   string(REGEX REPLACE "_$" "" short "${name}")
   set(defines "${ARGN}")
   string(REPLACE ";" " " defines_str "${defines}")
   file(APPEND "${_tmp}"
      "\n//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////\n// Build of ${type} shader from ${source} to ${short} with options: ${defines_str}\n")
   message(STATUS "> ${type} shader from ${source} to ${short} with ${defines_str}")
   foreach(backend IN LISTS ALL_BACKENDS)
      # WGSL has no layered rendering (SPV_EXT_shader_viewport_index_layer): stereo and VR variants stay stubs
      set(compile FALSE)
      if(backend IN_LIST BACKENDS AND NOT (backend STREQUAL "wgsl" AND ("STEREO" IN_LIST defines OR source STREQUAL "vs_vr_mask.sc")))
         set(compile TRUE)
      endif()
      if(compile)
         set(frag "${FRAG_DIR}/${name}${backend}.h")
         set(define_str "TARGET_${backend}")
         if(defines)
            string(APPEND define_str ";${defines}")
         endif()
         set(cmdline "-f;${source};${BACKEND_${backend}};--bin2c;${name}${backend};--type;${type}")
         if(DBGSHADER)
            list(APPEND cmdline --debug -O 0)
         endif()
         # Recompile the fragment if missing, older than its inputs, or built with different options
         set(rebuild TRUE)
         if(EXISTS "${frag}" AND EXISTS "${frag}.cmd")
            file(READ "${frag}.cmd" prev_cmdline)
            if(prev_cmdline STREQUAL "${cmdline}|${define_str}")
               set(rebuild FALSE)
               foreach(dep "${SRC_DIR}/${source}" ${SHADER_DEPS})
                  if("${dep}" IS_NEWER_THAN "${frag}")
                     set(rebuild TRUE)
                  endif()
               endforeach()
            endif()
         endif()
         if(rebuild)
            # --define takes a ';'-separated list: it must stay a single argument,
            # so it is passed quoted and kept out of the cmdline list.
            execute_process(
               COMMAND "${SHADERC}" ${cmdline} "--define" "${define_str}" -o "${frag}.tmp"
               WORKING_DIRECTORY "${SRC_DIR}"
               RESULT_VARIABLE rc
               OUTPUT_VARIABLE cout
               ERROR_VARIABLE cerr)
            if(NOT rc EQUAL 0)
               message(FATAL_ERROR "shaderc failed (${rc}) on ${source} -> ${name}${backend}\n${cout}\n${cerr}")
            endif()
            file(RENAME "${frag}.tmp" "${frag}")
            file(WRITE "${frag}.cmd" "${cmdline}|${define_str}")
         endif()
         file(READ "${frag}" blob)
         file(APPEND "${_tmp}" "${blob}")
      else()
         # Backend not compiled on this host: keep the symbol as an empty blob so the
         # BGFX_EMBEDDED_SHADER() initializer stays valid on platforms that reference it.
         file(APPEND "${_tmp}" "static const uint8_t ${name}${backend}[] = { 0x00 };\n")
      endif()
   endforeach()
   file(APPEND "${_tmp}" "static const bgfx::EmbeddedShader ${short} = BGFX_EMBEDDED_SHADER(${short});\n")
endfunction()

macro(begin_group group title)
   set(_tmp "${WORK_DIR}/bgfx_${group}.h.tmp")
   file(WRITE "${_tmp}" "// ${title}\n")
endmacro()

macro(end_group group)
   execute_process(COMMAND "${CMAKE_COMMAND}" -E copy_if_different
      "${WORK_DIR}/bgfx_${group}.h.tmp" "${OUT_DIR}/bgfx_${group}.h")
endmacro()

set(stereo NOSTEREO STEREO)
set(st_output "_" "_st_")

################################
# Basic material shaders (also 'classic' light, decals, kickers and unshaded variants)
function(group_basic)
   begin_group(basic "Base material Shaders")
   process_shader(basic "vs_vr_mask.sc" "vs_vr_mask_" "vertex")
   foreach(variant2 CLIP NOCLIP)
      string(TOLOWER "${variant2}" variant2_lower)
      foreach(k 0 1)
         list(GET stereo ${k} st)
         list(GET st_output ${k} st_o)
         process_shader(basic "vs_basic.sc" "vs_kicker_${variant2_lower}${st_o}" "vertex" "${st}" "${variant2}" "KICKER")
         process_shader(basic "fs_basic.sc" "fs_basic_refl_${variant2_lower}${st_o}" "fragment" "${st}" "${variant2}" "REFL")
         foreach(variant TEX NOTEX)
            string(TOLOWER "${variant}" variant_lower)
            process_shader(basic "vs_basic.sc" "vs_basic_${variant_lower}_${variant2_lower}${st_o}" "vertex" "${st}" "${variant}" "${variant2}")
            process_shader(basic "fs_basic.sc" "fs_basic_${variant_lower}_noat_${variant2_lower}${st_o}" "fragment" "${st}" "${variant}" "${variant2}" "NOAT")
            process_shader(basic "fs_basic.sc" "fs_basic_${variant_lower}_at_${variant2_lower}${st_o}" "fragment" "${st}" "${variant}" "${variant2}" "AT")
            process_shader(basic "fs_decal.sc" "fs_decal_${variant_lower}_${variant2_lower}${st_o}" "fragment" "${st}" "${variant}" "${variant2}")
            process_shader(basic "vs_basic.sc" "vs_classic_light_${variant_lower}_${variant2_lower}${st_o}" "vertex" "${st}" "CLASSIC_LIGHT" "${variant}" "${variant2}")
            process_shader(basic "fs_classic_light.sc" "fs_classic_light_${variant_lower}_${variant2_lower}${st_o}" "fragment" "${st}" "${variant}" "${variant2}" "NOSHADOW")
            process_shader(basic "fs_unshaded.sc" "fs_unshaded_${variant_lower}_${variant2_lower}${st_o}" "fragment" "${st}" "${variant}" "${variant2}" "NOSHADOW")
            process_shader(basic "fs_unshaded.sc" "fs_unshaded_${variant_lower}_ballshadow_${variant2_lower}${st_o}" "fragment" "${st}" "${variant}" "${variant2}" "SHADOW")
         endforeach()
      endforeach()
   endforeach()
   end_group(basic)
endfunction()

################################
# Ball shaders
function(group_ball)
   begin_group(ball "Ball Shaders")
   foreach(k 0 1)
      list(GET stereo ${k} st)
      list(GET st_output ${k} st_o)
      foreach(variant3 CLIP NOCLIP)
         string(TOLOWER "${variant3}" variant3_lower)
         process_shader(ball "vs_ball.sc" "vs_ball_${variant3_lower}${st_o}" "vertex" "${st}" "${variant3}")
         process_shader(ball "vs_ball_trail.sc" "vs_ball_trail_${variant3_lower}${st_o}" "vertex" "${st}" "${variant3}")
         process_shader(ball "fs_ball_trail.sc" "fs_ball_trail_${variant3_lower}${st_o}" "fragment" "${st}" "${variant3}")
         process_shader(ball "fs_ball_debug.sc" "fs_ball_debug_${variant3_lower}${st_o}" "fragment" "${st}" "${variant3}")
         foreach(variant EQUIRECTANGULAR SPHERICAL)
            string(TOLOWER "${variant}" variant_lower)
            foreach(variant2 DECAL NODECAL)
               string(TOLOWER "${variant2}" variant2_lower)
               process_shader(ball "fs_ball.sc" "fs_ball_${variant_lower}_${variant2_lower}_${variant3_lower}${st_o}" "fragment" "${st}" "${variant}" "${variant2}" "${variant3}")
            endforeach()
         endforeach()
      endforeach()
   endforeach()
   end_group(ball)
endfunction()

################################
# DMD/Sprite shaders
function(group_dmd)
   begin_group(dmd "DMD Shaders")
   process_shader(dmd "vs_dmd.sc" "vs_dmd_noworld_" "vertex")
   process_shader(dmd "vs_dmd.sc" "vs_dmd_noworld_st_" "vertex" "STEREO")
   foreach(variant3 CLIP NOCLIP)
      string(TOLOWER "${variant3}" variant3_lower)
      process_shader(dmd "vs_dmd.sc" "vs_dmd_world_${variant3_lower}_" "vertex" "WORLD" "${variant3}")
      process_shader(dmd "vs_dmd.sc" "vs_dmd_world_${variant3_lower}_st_" "vertex" "WORLD" "STEREO" "${variant3}")
      process_shader(dmd "fs_dmd.sc" "fs_dmd_${variant3_lower}_" "fragment" "DMD" "${variant3}")
      # CRTNUANCE is a second CRT permutation using the other filter, see fs_display.sc
      foreach(variant2 DMD SEG CRT CRTNUANCE)
         string(TOLOWER "${variant2}" variant2_lower)
         process_shader(dmd "fs_display.sc" "fs_display_${variant2_lower}_${variant3_lower}_" "fragment" "${variant2}" "${variant3}")
      endforeach()
      foreach(variant2 TEX NOTEX)
         string(TOLOWER "${variant2}" variant2_lower)
         process_shader(dmd "fs_dmd.sc" "fs_sprite_${variant2_lower}_${variant3_lower}_" "fragment" "SPRITE" "${variant2}" "${variant3}")
      endforeach()
   endforeach()
   end_group(dmd)
endfunction()

################################
# Flasher shaders
function(group_flasher)
   begin_group(flasher "Flasher Shaders")
   foreach(variant CLIP NOCLIP)
      string(TOLOWER "${variant}" variant_lower)
      process_shader(flasher "vs_flasher.sc" "vs_flasher_${variant_lower}_" "vertex" "${variant}")
      process_shader(flasher "vs_flasher.sc" "vs_flasher_${variant_lower}_st_" "vertex" "STEREO" "${variant}")
      process_shader(flasher "fs_flasher.sc" "fs_flasher_${variant_lower}_" "fragment" "${variant}")
   endforeach()
   end_group(flasher)
endfunction()

################################
# Light shaders
function(group_light)
   begin_group(light "Light Shaders")
   foreach(variant CLIP NOCLIP)
      string(TOLOWER "${variant}" variant_lower)
      process_shader(light "vs_light.sc" "vs_light_${variant_lower}_" "vertex" "${variant}")
      process_shader(light "vs_light.sc" "vs_light_${variant_lower}_st_" "vertex" "${variant}" "STEREO")
      foreach(variant2 NOSHADOW BALLSHADOW)
         string(TOLOWER "${variant2}" variant2_lower)
         process_shader(light "fs_light.sc" "fs_light_${variant2_lower}_${variant_lower}_" "fragment" "${variant}" "${variant2}")
      endforeach()
   endforeach()
   end_group(light)
endfunction()

################################
# Stereo shaders
function(group_stereo)
   begin_group(stereo "Stereo Shaders")
   foreach(k 0 1)
      list(GET stereo ${k} st)
      list(GET st_output ${k} st_o)
      process_shader(stereo "fs_pp_stereo.sc" "fs_pp_stereo_sbs${st_o}" "fragment" "SBS" "${st}")
      process_shader(stereo "fs_pp_stereo.sc" "fs_pp_stereo_tb${st_o}" "fragment" "TB" "${st}")
      process_shader(stereo "fs_pp_stereo.sc" "fs_pp_stereo_int${st_o}" "fragment" "INT" "${st}")
      process_shader(stereo "fs_pp_stereo.sc" "fs_pp_stereo_flipped_int${st_o}" "fragment" "FLIPPED_INT" "${st}")
      process_shader(stereo "fs_pp_stereo.sc" "fs_pp_stereo_anaglyph_deghost${st_o}" "fragment" "ANAGLYPH" "DEGHOST" "${st}")
      foreach(colors SRGB GAMMA)
         string(TOLOWER "${colors}" colors_lower)
         foreach(desat NODESAT DYNDESAT)
            string(TOLOWER "${desat}" desat_lower)
            process_shader(stereo "fs_pp_stereo.sc" "fs_pp_stereo_anaglyph_lin_${colors_lower}_${desat_lower}${st_o}" "fragment" "ANAGLYPH" "${desat}" "${colors}" "${st}")
         endforeach()
      endforeach()
   endforeach()
   end_group(stereo)
endfunction()

################################
# Post process shaders
function(group_postprocess)
   begin_group(postprocess "Postprocess Shaders")
   process_shader(postprocess "fs_pp_passthrough.sc" "fs_pp_passthrough_" "fragment" "STEREO")
   foreach(k 0 1)
      list(GET stereo ${k} st)
      list(GET st_output ${k} st_o)
      process_shader(postprocess "vs_postprocess.sc" "vs_postprocess${st_o}" "vertex" "${st}")
      process_shader(postprocess "fs_pp_msaa_depth.sc" "fs_pp_msaa_depth${st_o}" "fragment" "${st}")
      process_shader(postprocess "fs_pp_mirror.sc" "fs_pp_mirror${st_o}" "fragment" "${st}")
      process_shader(postprocess "fs_pp_copy.sc" "fs_pp_copy${st_o}" "fragment" "${st}")
      process_shader(postprocess "fs_pp_bloom.sc" "fs_pp_bloom${st_o}" "fragment" "${st}")
      process_shader(postprocess "fs_pp_irradiance.sc" "fs_pp_irradiance${st_o}" "fragment" "${st}")
      process_shader(postprocess "fs_pp_ssao.sc" "fs_pp_ssao${st_o}" "fragment" "${st}")
      process_shader(postprocess "fs_pp_ssr.sc" "fs_pp_ssr${st_o}" "fragment" "${st}")
      process_shader(postprocess "fs_pp_ao_display.sc" "fs_pp_ao_display${st_o}" "fragment" "${st}")
      foreach(variant FILTER NOFILTER)
         string(TOLOWER "${variant}" variant_lower)
         process_shader(postprocess "fs_pp_ao.sc" "fs_pp_ao_${variant_lower}${st_o}" "fragment" "${st}" "${variant}")
      endforeach()
   endforeach()
   end_group(postprocess)
endfunction()

################################
# Antialiasing shaders
function(group_antialiasing)
   begin_group(antialiasing "Antialiasing Shaders")
   foreach(k 0 1)
      list(GET stereo ${k} st)
      list(GET st_output ${k} st_o)
      process_shader(antialiasing "fs_pp_cas.sc" "fs_pp_cas${st_o}" "fragment" "${st}")
      process_shader(antialiasing "fs_pp_bilateral_cas.sc" "fs_pp_bilateral_cas${st_o}" "fragment" "${st}")
      process_shader(antialiasing "fs_pp_nfaa.sc" "fs_pp_nfaa${st_o}" "fragment" "${st}")
      process_shader(antialiasing "fs_pp_dlaa_edge.sc" "fs_pp_dlaa_edge${st_o}" "fragment" "${st}")
      process_shader(antialiasing "fs_pp_dlaa.sc" "fs_pp_dlaa${st_o}" "fragment" "${st}")
      process_shader(antialiasing "fs_pp_faaa.sc" "fs_pp_faaa${st_o}" "fragment" "${st}")
      foreach(variant FXAA1 FXAA2 FXAA3)
         string(TOLOWER "${variant}" variant_lower)
         process_shader(antialiasing "fs_pp_fxaa.sc" "fs_pp_${variant_lower}${st_o}" "fragment" "${st}" "${variant}")
      endforeach()
   endforeach()
   # TODO add stereo support to SMAA
   process_shader(antialiasing "vs_pp_smaa_BlendingWeightCalculation.sc" "vs_pp_smaa_blendweightcalculation_" "vertex")
   process_shader(antialiasing "vs_pp_smaa_EdgeDetection.sc" "vs_pp_smaa_edgedetection_" "vertex")
   process_shader(antialiasing "vs_pp_smaa_NeighborhoodBlending.sc" "vs_pp_smaa_neighborhoodblending_" "vertex")
   process_shader(antialiasing "fs_pp_smaa_BlendingWeightCalculation.sc" "fs_pp_smaa_blendweightcalculation_" "fragment")
   process_shader(antialiasing "fs_pp_smaa_EdgeDetection.sc" "fs_pp_smaa_edgedetection_" "fragment")
   process_shader(antialiasing "fs_pp_smaa_NeighborhoodBlending.sc" "fs_pp_smaa_neighborhoodblending_" "fragment")
   end_group(antialiasing)
endfunction()

################################
# Tonemapping shaders
function(group_tonemap)
   begin_group(tonemap "Tonemap Shaders")
   foreach(k 0 1)
      list(GET stereo ${k} st)
      list(GET st_output ${k} st_o)
      foreach(variant FILMIC NEUTRAL AGX WCG)
         string(TOLOWER "${variant}" variant_lower)
         foreach(variant2 AO NOAO)
            string(TOLOWER "${variant2}" variant2_lower)
            foreach(variant3 FILTER NOFILTER)
               string(TOLOWER "${variant3}" variant3_lower)
               process_shader(tonemap "fs_pp_tonemap.sc" "fs_pp_tonemap_${variant_lower}_${variant2_lower}_${variant3_lower}${st_o}" "fragment" "${st}" "${variant}" "${variant2}" "${variant3}")
            endforeach()
         endforeach()
      endforeach()
      foreach(variant2 AO NOAO)
         string(TOLOWER "${variant2}" variant2_lower)
         foreach(variant3 FILTER NOFILTER)
            string(TOLOWER "${variant3}" variant3_lower)
            process_shader(tonemap "fs_pp_tonemap.sc" "fs_pp_tonemap_reinhard_${variant2_lower}_${variant3_lower}_rgb${st_o}" "fragment" "${st}" "REINHARD" "${variant2}" "${variant3}" "RGB")
         endforeach()
      endforeach()
   endforeach()
   end_group(tonemap)
endfunction()

################################
# Blur shaders
function(group_blur)
   begin_group(blur "Blur Kernel Shaders")
   foreach(k 0 1)
      list(GET stereo ${k} st)
      list(GET st_output ${k} st_o)
      foreach(blur BLUR_7 BLUR_9 BLUR_11 BLUR_13 BLUR_15 BLUR_19 BLUR_19H BLUR_23 BLUR_27 BLUR_39)
         string(TOLOWER "${blur}" blur_lower)
         foreach(j 0 1)
            if(j EQUAL 0)
               set(axis HORIZONTAL)
               set(axis_suffix "_h")
            else()
               set(axis VERTICAL)
               set(axis_suffix "_v")
            endif()
            process_shader(blur "fs_pp_blur.sc" "fs_${blur_lower}${axis_suffix}${st_o}" "fragment" "${st}" "${blur}" "${axis}")
         endforeach()
      endforeach()
   endforeach()
   end_group(blur)
endfunction()

################################
# Motion blur shaders
function(group_motionblur)
   begin_group(motionblur "Raytraced motion blur Shaders")
   foreach(k 0 1)
      list(GET stereo ${k} st)
      list(GET st_output ${k} st_o)
      process_shader(motionblur "fs_pp_motionblur.sc" "fs_pp_motionblur${st_o}" "fragment" "${st}")
   endforeach()
   end_group(motionblur)
endfunction()

################################
# ImgUI shaders
function(group_imgui)
   begin_group(imgui "ImgUI Shaders")
   foreach(k 0 1)
      list(GET stereo ${k} st)
      list(GET st_output ${k} st_o)
      process_shader(imgui "vs_imgui.sc" "vs_imgui${st_o}" "vertex" "${st}")
      process_shader(imgui "fs_imgui.sc" "fs_imgui${st_o}" "fragment" "${st}")
   endforeach()
   end_group(imgui)
endfunction()

################################
# MipMap compute shader
function(group_mipmap)
   begin_group(mipmap "MipMap Shaders")
   foreach(fmt srgba8 rgba8 rgba16f rgba32f)
      process_shader(mipmap "cs_mipmap.sc" "cs_mipmap_${fmt}_" "compute" "FMT_${fmt}")
   endforeach()
   end_group(mipmap)
endfunction()

set(ALL_GROUPS basic ball dmd flasher light stereo postprocess antialiasing tonemap blur motionblur imgui mipmap)
foreach(g IN LISTS ALL_GROUPS)
   if(NOT DEFINED GROUP OR GROUP STREQUAL g)
      cmake_language(CALL group_${g})
   endif()
endforeach()
