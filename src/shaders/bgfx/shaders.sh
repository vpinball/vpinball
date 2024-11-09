#!/bin/bash

set -e

gen_ball=true
gen_basic=true
gen_blur=true
gen_dmd=true
gen_flasher=true
gen_light=true
gen_motionblur=true
gen_postprocess=true
gen_stereo=true
gen_tonemap=true
gen_antialiasing=false
gen_imgui=true

process_shader() {
    local source="$1"
    local output_file="$2"
    local header="$3"
    local type="$4"
    shift 4
    local defines=("$@")

    local outputs=(
        'mtl ' 
        'essl' 
        'glsl' 
        'dx11' 
        'spv '
    )

    local targets=(
        '--platform osx     -p metal -O 3'
        '--platform windows -p 310_es    '
        '--platform windows -p 440       '
        '--platform windows -p s_5_0 -O 3'
        '--platform windows -p spirv     '
    )

    local shaderc="shaderc"

    local output_path="../bgfx_${output_file}"
    local short_name="${header:0:${#header}-1}"

    echo -e "\n//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////\n// Build of ${type} shader from ${source} to ${short_name} with options: ${defines[*]}" >> "$output_path"
    echo "> ${type} shader from ${source} to ${short_name} with ${defines[*]}"
    
    for i in "${!targets[@]}"; do
        local cmdline="-f ${source} ${targets[$i]} --bin2c ${header}${outputs[$i]} --type ${type}"
        if [ ${#defines[@]} -ne 0 ]; then
            cmdline+=" --define $(IFS=';'; echo "${defines[*]}")"
        fi
        cmdline+=" -o tmp.h"
        echo "$shaderc $cmdline"
        $shaderc $cmdline
        cat tmp.h >> "$output_path"
        rm tmp.h
    done

    echo "static const bgfx::EmbeddedShader ${short_name} = BGFX_EMBEDDED_SHADER(${short_name});" >> "$output_path"
}

stereo=("NOSTEREO" "STEREO")
st_output=("_" "_st_")

################################
# Basic material shaders
if [ "$gen_basic" = true ]; then
    echo -e "\n>>>>>>>>>>>>>>>> Base material shader"
    echo "// Base material Shaders" > "../bgfx_basic.h"
    for variant2 in "CLIP" "NOCLIP"; do
        for k in 0 1; do
            variant2_lower=$(echo "$variant2" | tr '[:upper:]' '[:lower:]')
            process_shader "vs_basic.sc" "basic.h" "vs_kicker_${variant2_lower}${st_output[$k]}" "vertex" "${stereo[$k]}" "$variant2" "KICKER"
            process_shader "fs_basic.sc" "basic.h" "fs_basic_refl_${variant2_lower}${st_output[$k]}" "fragment" "${stereo[$k]}" "$variant2" "REFL"
            for variant in "TEX" "NOTEX"; do
                variant_lower=$(echo "$variant" | tr '[:upper:]' '[:lower:]')
                process_shader "vs_basic.sc" "basic.h" "vs_basic_${variant_lower}_${variant2_lower}${st_output[$k]}" "vertex" "${stereo[$k]}" "$variant" "$variant2"
                process_shader "fs_basic.sc" "basic.h" "fs_basic_${variant_lower}_noat_${variant2_lower}${st_output[$k]}" "fragment" "${stereo[$k]}" "$variant" "$variant2" "NOAT"
                process_shader "fs_basic.sc" "basic.h" "fs_basic_${variant_lower}_at_${variant2_lower}${st_output[$k]}" "fragment" "${stereo[$k]}" "$variant" "$variant2" "AT"
                process_shader "fs_decal.sc" "basic.h" "fs_decal_${variant_lower}_${variant2_lower}${st_output[$k]}" "fragment" "${stereo[$k]}" "$variant" "$variant2"
                process_shader "vs_basic.sc" "basic.h" "vs_classic_light_${variant_lower}_${variant2_lower}${st_output[$k]}" "vertex" "${stereo[$k]}" "CLASSIC_LIGHT" "$variant" "$variant2"
                process_shader "fs_classic_light.sc" "basic.h" "fs_classic_light_${variant_lower}_${variant2_lower}${st_output[$k]}" "fragment" "${stereo[$k]}" "$variant" "$variant2" "NOSHADOW"
                process_shader "fs_unshaded.sc" "basic.h" "fs_unshaded_${variant_lower}_${variant2_lower}${st_output[$k]}" "fragment" "${stereo[$k]}" "$variant" "$variant2" "NOSHADOW"
                process_shader "fs_unshaded.sc" "basic.h" "fs_unshaded_${variant_lower}_ballshadow_${variant2_lower}${st_output[$k]}" "fragment" "${stereo[$k]}" "$variant" "$variant2" "SHADOW"
            done
        done
    done
fi

################################
# Ball shaders
if [ "$gen_ball" = true ]; then
    echo -e "\n>>>>>>>>>>>>>>>> Ball shader"
    echo "// Ball Shaders" > "../bgfx_ball.h"
    for k in 0 1; do
        for variant3 in "CLIP" "NOCLIP"; do
            variant3_lower=$(echo "$variant3" | tr '[:upper:]' '[:lower:]')
            process_shader "vs_ball.sc" "ball.h" "vs_ball_${variant3_lower}${st_output[$k]}" "vertex" "${stereo[$k]}" "$variant3"
            process_shader "vs_ball_trail.sc" "ball.h" "vs_ball_trail_${variant3_lower}${st_output[$k]}" "vertex" "${stereo[$k]}" "$variant3"
            process_shader "fs_ball_trail.sc" "ball.h" "fs_ball_trail_${variant3_lower}${st_output[$k]}" "fragment" "${stereo[$k]}" "$variant3"
            process_shader "fs_ball_debug.sc" "ball.h" "fs_ball_debug_${variant3_lower}${st_output[$k]}" "fragment" "${stereo[$k]}" "$variant3"
            for variant in "EQUIRECTANGULAR" "SPHERICAL"; do
                variant_lower=$(echo "$variant" | tr '[:upper:]' '[:lower:]')
                for variant2 in "DECAL" "NODECAL"; do
                    variant2_lower=$(echo "$variant2" | tr '[:upper:]' '[:lower:]')
                    process_shader "fs_ball.sc" "ball.h" "fs_ball_${variant_lower}_${variant2_lower}_${variant3_lower}${st_output[$k]}" "fragment" "${stereo[$k]}" "$variant" "$variant2" "$variant3"
                done
            done
        done
    done
fi

################################
# DMD/Sprite shaders
if [ "$gen_dmd" = true ]; then
    echo -e "\n>>>>>>>>>>>>>>>> DMD & sprite shaders"
    echo "// DMD Shaders" > "../bgfx_dmd.h"
    process_shader "vs_dmd.sc" "dmd.h" "vs_dmd_noworld_" "vertex"
    for variant3 in "CLIP" "NOCLIP"; do
        variant3_lower=$(echo "$variant3" | tr '[:upper:]' '[:lower:]')
        process_shader "vs_dmd.sc" "dmd.h" "vs_dmd_world_${variant3_lower}_" "vertex" "WORLD" "$variant3"
        process_shader "vs_dmd.sc" "dmd.h" "vs_dmd_world_${variant3_lower}_st_" "vertex" "WORLD" "STEREO" "$variant3"
        process_shader "fs_dmd.sc" "dmd.h" "fs_dmd_${variant3_lower}_" "fragment" "DMD" "$variant3"
        for variant2 in "RGB" "SRGB"; do
            variant2_lower=$(echo "$variant2" | tr '[:upper:]' '[:lower:]')
            process_shader "fs_dmd2.sc" "dmd.h" "fs_dmd2_${variant2_lower}_${variant3_lower}_" "fragment" "$variant2" "$variant3"
        done
        for variant2 in "TEX" "NOTEX"; do
            variant2_lower=$(echo "$variant2" | tr '[:upper:]' '[:lower:]')
            process_shader "fs_dmd.sc" "dmd.h" "fs_sprite_${variant2_lower}_${variant3_lower}_" "fragment" "SPRITE" "$variant2" "$variant3"
        done
    done
fi

################################
# Flasher shaders
if [ "$gen_flasher" = true ]; then
    echo -e "\n>>>>>>>>>>>>>>>> Flasher shader"
    echo "// Flasher Shaders" > "../bgfx_flasher.h"
    for variant in "CLIP" "NOCLIP"; do
        variant_lower=$(echo "$variant" | tr '[:upper:]' '[:lower:]')
        process_shader "vs_flasher.sc" "flasher.h" "vs_flasher_${variant_lower}_" "vertex" "$variant"
        process_shader "vs_flasher.sc" "flasher.h" "vs_flasher_${variant_lower}_st_" "vertex" "STEREO" "$variant"
        process_shader "fs_flasher.sc" "flasher.h" "fs_flasher_${variant_lower}_" "fragment" "$variant"
    done
fi

################################
# Light shaders
if [ "$gen_light" = true ]; then
    echo -e "\n>>>>>>>>>>>>>>>> Light shader"
    echo "// Light Shaders" > "../bgfx_light.h"
    for variant in "CLIP" "NOCLIP"; do
        variant_lower=$(echo "$variant" | tr '[:upper:]' '[:lower:]')
        process_shader "vs_light.sc" "light.h" "vs_light_${variant_lower}_" "vertex" "$variant"
        process_shader "vs_light.sc" "light.h" "vs_light_${variant_lower}_st_" "vertex" "$variant" "STEREO"
        for variant2 in "NOSHADOW" "BALLSHADOW"; do
            variant2_lower=$(echo "$variant2" | tr '[:upper:]' '[:lower:]')
            process_shader "fs_light.sc" "light.h" "fs_light_${variant2_lower}_${variant_lower}_" "fragment" "$variant" "$variant2"
        done
    done
fi

################################
# Stereo shaders
if [ "$gen_stereo" = true ]; then
    echo -e "\n>>>>>>>>>>>>>>>> Stereo shaders"
    echo "// Stereo Shaders" > "../bgfx_stereo.h"
    for k in 0 1; do
        process_shader "fs_pp_stereo.sc" "stereo.h" "fs_pp_stereo_sbs${st_output[$k]}" "fragment" "SBS" "${stereo[$k]}"
        process_shader "fs_pp_stereo.sc" "stereo.h" "fs_pp_stereo_tb${st_output[$k]}" "fragment" "TB" "${stereo[$k]}"
        process_shader "fs_pp_stereo.sc" "stereo.h" "fs_pp_stereo_int${st_output[$k]}" "fragment" "INT" "${stereo[$k]}"
        process_shader "fs_pp_stereo.sc" "stereo.h" "fs_pp_stereo_flipped_int${st_output[$k]}" "fragment" "FLIPPED_INT" "${stereo[$k]}"
        process_shader "fs_pp_stereo.sc" "stereo.h" "fs_pp_stereo_anaglyph_deghost${st_output[$k]}" "fragment" "ANAGLYPH" "DEGHOST" "${stereo[$k]}"
        for colors in "SRGB" "GAMMA"; do
            for desat in "NODESAT" "DYNDESAT"; do
                colors_lower=$(echo "$colors" | tr '[:upper:]' '[:lower:]')
                desat_lower=$(echo "$desat" | tr '[:upper:]' '[:lower:]')
                process_shader "fs_pp_stereo.sc" "stereo.h" "fs_pp_stereo_anaglyph_lin_${colors_lower}_${desat_lower}${st_output[$k]}" "fragment" "ANAGLYPH" "$desat" "$colors" "${stereo[$k]}"
            done
        done
    done
fi

################################
# Post process shaders
if [ "$gen_postprocess" = true ]; then
    echo -e "\n>>>>>>>>>>>>>>>> Post process shaders"
    echo "// Postprocess Shaders" > "../bgfx_postprocess.h"
    for k in 0 1; do
        process_shader "vs_postprocess.sc" "postprocess.h" "vs_postprocess${st_output[$k]}" "vertex" "${stereo[$k]}"
        process_shader "fs_pp_mirror.sc" "postprocess.h" "fs_pp_mirror${st_output[$k]}" "fragment" "${stereo[$k]}"
        process_shader "fs_pp_copy.sc" "postprocess.h" "fs_pp_copy${st_output[$k]}" "fragment" "${stereo[$k]}"
        process_shader "fs_pp_bloom.sc" "postprocess.h" "fs_pp_bloom${st_output[$k]}" "fragment" "${stereo[$k]}"
        process_shader "fs_pp_irradiance.sc" "postprocess.h" "fs_pp_irradiance${st_output[$k]}" "fragment" "${stereo[$k]}"
        process_shader "fs_pp_ssao.sc" "postprocess.h" "fs_pp_ssao${st_output[$k]}" "fragment" "${stereo[$k]}"
        process_shader "fs_pp_ssr.sc" "postprocess.h" "fs_pp_ssr${st_output[$k]}" "fragment" "${stereo[$k]}"
        process_shader "fs_pp_ao_display.sc" "postprocess.h" "fs_pp_ao_display${st_output[$k]}" "fragment" "${stereo[$k]}"
        for variant in "FILTER" "NOFILTER"; do
            variant_lower=$(echo "$variant" | tr '[:upper:]' '[:lower:]')
            process_shader "fs_pp_ao.sc" "postprocess.h" "fs_pp_ao_${variant_lower}${st_output[$k]}" "fragment" "${stereo[$k]}" "$variant"
        done
    done
fi

################################
# Antialiasing shaders
if [ "$gen_antialiasing" = true ]; then
    echo "// Antialiasing Shaders" > "../bgfx_antialiasing.h"
    for k in 0 1; do
        process_shader "fs_pp_cas.sc" "antialiasing.h" "fs_pp_cas${st_output[$k]}" "fragment" "${stereo[$k]}"
        process_shader "fs_pp_bilateral_cas.sc" "antialiasing.h" "fs_pp_bilateral_cas${st_output[$k]}" "fragment" "${stereo[$k]}"
        process_shader "fs_pp_nfaa.sc" "antialiasing.h" "fs_pp_nfaa${st_output[$k]}" "fragment" "${stereo[$k]}"
        process_shader "fs_pp_dlaa_edge.sc" "antialiasing.h" "fs_pp_dlaa_edge${st_output[$k]}" "fragment" "${stereo[$k]}"
        process_shader "fs_pp_dlaa.sc" "antialiasing.h" "fs_pp_dlaa${st_output[$k]}" "fragment" "${stereo[$k]}"
        for variant in "FILTER" "NOFILTER"; do
            variant_lower=$(echo "$variant" | tr '[:upper:]' '[:lower:]')
            process_shader "fs_pp_ao.sc" "postprocess.h" "fs_pp_ao_${variant_lower}${st_output[$k]}" "fragment" "${stereo[$k]}" "$variant"
        done
        for variant in "FXAA1" "FXAA2" "FXAA3"; do
            variant_lower=$(echo "$variant" | tr '[:upper:]' '[:lower:]')
            process_shader "fs_pp_fxaa.sc" "antialiasing.h" "fs_pp_${variant_lower}${st_output[$k]}" "fragment" "${stereo[$k]}" "$variant"
        done
    done

    # TODO add stereo support to SMAA
    process_shader "vs_pp_smaa_BlendingWeightCalculation.sc" "antialiasing.h" "vs_pp_smaa_blendweightcalculation_" "vertex"
    process_shader "vs_pp_smaa_EdgeDetection.sc" "antialiasing.h" "vs_pp_smaa_edgedetection_" "vertex"
    process_shader "vs_pp_smaa_NeighborhoodBlending.sc" "antialiasing.h" "vs_pp_smaa_neighborhoodblending_" "vertex"
    process_shader "fs_pp_smaa_BlendingWeightCalculation.sc" "antialiasing.h" "fs_pp_smaa_blendweightcalculation_" "fragment"
    process_shader "fs_pp_smaa_EdgeDetection.sc" "antialiasing.h" "fs_pp_smaa_edgedetection_" "fragment"
    process_shader "fs_pp_smaa_NeighborhoodBlending.sc" "antialiasing.h" "fs_pp_smaa_neighborhoodblending_" "fragment"
fi

################################
# Tonemapping shaders
if [ "$gen_tonemap" = true ]; then
    echo "// Tonemap Shaders" > "../bgfx_tonemap.h"
    for k in 0 1; do
        for variant in "FILMIC" "NEUTRAL" "AGX" "AGX_PUNCHY" "WCG"; do
            variant_lower=$(echo "$variant" | tr '[:upper:]' '[:lower:]')
            for variant2 in "AO" "NOAO"; do
                variant2_lower=$(echo "$variant2" | tr '[:upper:]' '[:lower:]')
                for variant3 in "FILTER" "NOFILTER"; do
                    variant3_lower=$(echo "$variant3" | tr '[:upper:]' '[:lower:]')
                    process_shader "fs_pp_tonemap.sc" "tonemap.h" "fs_pp_tonemap_${variant_lower}_${variant2_lower}_${variant3_lower}${st_output[$k]}" "fragment" "${stereo[$k]}" "$variant" "$variant2" "$variant3"
                done
            done
        done
        for variant in "REINHARD"; do
            variant_lower=$(echo "$variant" | tr '[:upper:]' '[:lower:]')
            for variant2 in "AO" "NOAO"; do
                variant2_lower=$(echo "$variant2" | tr '[:upper:]' '[:lower:]')
                for variant3 in "FILTER" "NOFILTER"; do
                    variant3_lower=$(echo "$variant3" | tr '[:upper:]' '[:lower:]')
                    for variant4 in "GRAY" "RG" "RGB"; do
                        variant4_lower=$(echo "$variant4" | tr '[:upper:]' '[:lower:]')
                        process_shader "fs_pp_tonemap.sc" "tonemap.h" "fs_pp_tonemap_${variant_lower}_${variant2_lower}_${variant3_lower}_${variant4_lower}${st_output[$k]}" "fragment" "${stereo[$k]}" "$variant" "$variant2" "$variant3" "$variant4"
                    done
                done
            done
        done
    done
fi

################################
# Blur shaders
if [ "$gen_blur" = true ]; then
    echo "// Blur Kernel Shaders" > "../bgfx_blur.h"
    for k in 0 1; do
        blurs=("BLUR_7" "BLUR_9" "BLUR_11" "BLUR_13" "BLUR_15" "BLUR_19" "BLUR_19H" "BLUR_23" "BLUR_27" "BLUR_39")
        for blur in "${blurs[@]}"; do
            blur_lower=$(echo "$blur" | tr '[:upper:]' '[:lower:]')
            axis=("HORIZONTAL" "VERTICAL")
            axis_suffix=("_h" "_v")
            for j in 0 1; do
                process_shader "fs_pp_blur.sc" "blur.h" "fs_${blur_lower}${axis_suffix[$j]}${st_output[$k]}" "fragment" "${stereo[$k]}" "$blur" "${axis[$j]}"
            done
        done
    done
fi

################################
# Motion blur shaders
if [ "$gen_motionblur" = true ]; then
    echo -e "\n>>>>>>>>>>>>>>>> Motion blur shaders"
    echo "// Raytraced motion blur Shaders" > "../bgfx_motionblur.h"
    for k in 0 1; do
        process_shader "fs_pp_motionblur.sc" "motionblur.h" "fs_pp_motionblur${st_output[$k]}" "fragment" "${stereo[$k]}"
    done
fi

################################
# ImgUI shaders
if [ "$gen_imgui" = true ]; then
    echo -e "\n>>>>>>>>>>>>>>>> ImgUI shaders"
    echo "// ImgUI Shaders" > "../bgfx_imgui.h"
    process_shader "vs_imgui.sc" "imgui.h" "vs_imgui_" "vertex"
fi