// license:GPLv3+

#include "core/stdafx.h"
#include "Shader.h"
#include "typedefs3D.h"
#include "RenderDevice.h"

#include <plog/Log.h>
#include <plog/Initializers/RollingFileInitializer.h>

#include "core/vpversion.h"

#if defined(ENABLE_BGFX)
#ifdef __STANDALONE__
#pragma push_macro("_WIN64")
#undef _WIN64
#endif
#include "bx/timer.h"
#include "bx/file.h"
#include "bx/readerwriter.h"
#include "bgfx/bgfx.h"
#include "bgfx/platform.h"
#include "bgfx/embedded_shader.h"
#ifdef __STANDALONE__
#pragma pop_macro("_WIN64")
#endif

#elif defined(ENABLE_OPENGL)
#include <windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include "robin_hood.h"
#ifdef __STANDALONE__
#include <sstream>
#endif
ShaderTechniques Shader::m_boundTechnique = ShaderTechniques::SHADER_TECHNIQUE_INVALID; // FIXME move to render device

#endif

#define SHADER_TECHNIQUE(name, ...) { #name, InitTechUniforms( {__VA_ARGS__}) }
vector<ShaderUniforms> InitTechUniforms() { return vector<ShaderUniforms>(); }
vector<ShaderUniforms> InitTechUniforms(std::initializer_list<ShaderUniforms> args) { return vector<ShaderUniforms> { args }; }
Shader::TechniqueDef Shader::shaderTechniqueNames[SHADER_TECHNIQUE_COUNT] {
   SHADER_TECHNIQUE(RenderBall, SHADER_matProj, SHADER_matWorldViewProj, SHADER_matView, SHADER_matWorldView, SHADER_matWorldViewInverse, SHADER_ballLightEmission, SHADER_ballLightPos,
      SHADER_Roughness_WrapL_Edge_Thickness, SHADER_cBase_Alpha, SHADER_fDisableLighting_top_below, SHADER_fenvEmissionScale_TexWidth, SHADER_cAmbient_LightRange, SHADER_tex_diffuse_env,
      SHADER_orientation, SHADER_invTableRes_reflection, SHADER_w_h_disableLighting, SHADER_tex_ball_color, SHADER_tex_ball_playfield, SHADER_tex_ball_decal, SHADER_clip_plane),
   SHADER_TECHNIQUE(RenderBall_DecalMode, SHADER_matProj, SHADER_matWorldViewProj, SHADER_matView, SHADER_matWorldView, SHADER_matWorldViewInverse, SHADER_ballLightEmission,
      SHADER_ballLightPos, SHADER_Roughness_WrapL_Edge_Thickness, SHADER_cBase_Alpha, SHADER_fDisableLighting_top_below, SHADER_fenvEmissionScale_TexWidth, SHADER_cAmbient_LightRange,
      SHADER_tex_diffuse_env, SHADER_orientation, SHADER_invTableRes_reflection, SHADER_w_h_disableLighting, SHADER_tex_ball_color, SHADER_tex_ball_playfield, SHADER_tex_ball_decal,
      SHADER_clip_plane),
   SHADER_TECHNIQUE(RenderBall_SphericalMap, SHADER_matProj, SHADER_matWorldViewProj, SHADER_matView, SHADER_matWorldView, SHADER_matWorldViewInverse, SHADER_ballLightEmission,
      SHADER_ballLightPos, SHADER_Roughness_WrapL_Edge_Thickness, SHADER_cBase_Alpha, SHADER_fDisableLighting_top_below, SHADER_fenvEmissionScale_TexWidth, SHADER_cAmbient_LightRange,
      SHADER_tex_diffuse_env, SHADER_orientation, SHADER_invTableRes_reflection, SHADER_w_h_disableLighting, SHADER_tex_ball_color, SHADER_tex_ball_playfield, SHADER_tex_ball_decal,
      SHADER_clip_plane),
   SHADER_TECHNIQUE(RenderBall_SphericalMap_DecalMode, SHADER_matProj, SHADER_matWorldViewProj, SHADER_matView, SHADER_matWorldView, SHADER_matWorldViewInverse, SHADER_ballLightEmission,
      SHADER_ballLightPos, SHADER_Roughness_WrapL_Edge_Thickness, SHADER_cBase_Alpha, SHADER_fDisableLighting_top_below, SHADER_fenvEmissionScale_TexWidth, SHADER_cAmbient_LightRange,
      SHADER_tex_diffuse_env, SHADER_orientation, SHADER_invTableRes_reflection, SHADER_w_h_disableLighting, SHADER_tex_ball_color, SHADER_tex_ball_playfield, SHADER_tex_ball_decal,
      SHADER_clip_plane),
   SHADER_TECHNIQUE(RenderBall_Debug, SHADER_matWorldViewProj, SHADER_matWorldView, SHADER_matWorldViewInverse, SHADER_orientation, SHADER_clip_plane),
   SHADER_TECHNIQUE(RenderBallTrail, SHADER_matWorldViewProj, SHADER_matWorldView, SHADER_matWorldViewInverse, SHADER_cBase_Alpha, SHADER_fenvEmissionScale_TexWidth, SHADER_orientation,
      SHADER_w_h_disableLighting, SHADER_tex_ball_color, SHADER_clip_plane),
   // OpenGL only has the first variant. DX9 needs all of them due to shader compiler limitation
   SHADER_TECHNIQUE(basic_with_texture, SHADER_matProj, SHADER_matWorldViewProj, SHADER_matWorld, SHADER_matView, SHADER_matWorldView, SHADER_matWorldViewInverseTranspose,
      SHADER_lightCenter_doShadow, SHADER_balls, SHADER_staticColor_Alpha, SHADER_w_h_height, SHADER_basicLightEmission, SHADER_basicLightPos, SHADER_Roughness_WrapL_Edge_Thickness,
      SHADER_cBase_Alpha, SHADER_fDisableLighting_top_below, SHADER_fenvEmissionScale_TexWidth, SHADER_cAmbient_LightRange, SHADER_tex_env, SHADER_tex_diffuse_env,
      SHADER_cClearcoat_EdgeAlpha, SHADER_cGlossy_ImageLerp, SHADER_u_basic_shade_mode, SHADER_refractionTint_thickness, SHADER_mirrorNormal_factor, SHADER_objectSpaceNormalMap,
      SHADER_tex_base_color, SHADER_tex_base_transmission, SHADER_tex_base_normalmap, SHADER_tex_reflection, SHADER_tex_refraction, SHADER_tex_probe_depth, SHADER_clip_plane),
   SHADER_TECHNIQUE(basic_with_texture_isMetal),
   SHADER_TECHNIQUE(basic_with_texture_normal),
   SHADER_TECHNIQUE(basic_with_texture_normal_isMetal),
   SHADER_TECHNIQUE(basic_with_texture_refl),
   SHADER_TECHNIQUE(basic_with_texture_refl_isMetal),
   SHADER_TECHNIQUE(basic_with_texture_refl_normal),
   SHADER_TECHNIQUE(basic_with_texture_refl_normal_isMetal),
   SHADER_TECHNIQUE(basic_with_texture_refr),
   SHADER_TECHNIQUE(basic_with_texture_refr_isMetal),
   SHADER_TECHNIQUE(basic_with_texture_refr_normal),
   SHADER_TECHNIQUE(basic_with_texture_refr_normal_isMetal),
   SHADER_TECHNIQUE(basic_with_texture_refr_refl),
   SHADER_TECHNIQUE(basic_with_texture_refr_refl_isMetal),
   SHADER_TECHNIQUE(basic_with_texture_refr_refl_normal),
   SHADER_TECHNIQUE(basic_with_texture_refr_refl_normal_isMetal),
   // OpenGL only has the first variant. DX9 needs all of them due to shader compiler limitation
   SHADER_TECHNIQUE(basic_with_texture_at, SHADER_alphaTestValue, SHADER_matProj, SHADER_matWorldViewProj, SHADER_matWorld, SHADER_matView, SHADER_matWorldView,
      SHADER_matWorldViewInverseTranspose, SHADER_lightCenter_doShadow, SHADER_balls, SHADER_staticColor_Alpha, SHADER_w_h_height, SHADER_basicLightEmission, SHADER_basicLightPos,
      SHADER_Roughness_WrapL_Edge_Thickness, SHADER_cBase_Alpha, SHADER_fDisableLighting_top_below, SHADER_fenvEmissionScale_TexWidth, SHADER_cAmbient_LightRange, SHADER_tex_env,
      SHADER_tex_diffuse_env, SHADER_cClearcoat_EdgeAlpha, SHADER_cGlossy_ImageLerp, SHADER_u_basic_shade_mode, SHADER_refractionTint_thickness, SHADER_mirrorNormal_factor,
      SHADER_objectSpaceNormalMap, SHADER_tex_base_color, SHADER_tex_base_transmission, SHADER_tex_base_normalmap, SHADER_tex_reflection, SHADER_tex_refraction, SHADER_tex_probe_depth,
      SHADER_clip_plane),
   SHADER_TECHNIQUE(basic_with_texture_at_isMetal),
   SHADER_TECHNIQUE(basic_with_texture_at_normal),
   SHADER_TECHNIQUE(basic_with_texture_at_normal_isMetal),
   SHADER_TECHNIQUE(basic_with_texture_at_refl),
   SHADER_TECHNIQUE(basic_with_texture_at_refl_isMetal),
   SHADER_TECHNIQUE(basic_with_texture_at_refl_normal),
   SHADER_TECHNIQUE(basic_with_texture_at_refl_normal_isMetal),
   SHADER_TECHNIQUE(basic_with_texture_at_refr),
   SHADER_TECHNIQUE(basic_with_texture_at_refr_isMetal),
   SHADER_TECHNIQUE(basic_with_texture_at_refr_normal),
   SHADER_TECHNIQUE(basic_with_texture_at_refr_normal_isMetal),
   SHADER_TECHNIQUE(basic_with_texture_at_refr_refl),
   SHADER_TECHNIQUE(basic_with_texture_at_refr_refl_isMetal),
   SHADER_TECHNIQUE(basic_with_texture_at_refr_refl_normal),
   SHADER_TECHNIQUE(basic_with_texture_at_refr_refl_normal_isMetal),
   // OpenGL only has the first variant. DX9 needs all of them due to shader compiler limitation
   SHADER_TECHNIQUE(basic_without_texture, SHADER_matProj, SHADER_matWorldViewProj, SHADER_matWorld, SHADER_matView, SHADER_matWorldView, SHADER_matWorldViewInverseTranspose,
      SHADER_lightCenter_doShadow, SHADER_balls, SHADER_staticColor_Alpha, SHADER_w_h_height, SHADER_basicLightEmission, SHADER_basicLightPos, SHADER_Roughness_WrapL_Edge_Thickness,
      SHADER_cBase_Alpha, SHADER_fDisableLighting_top_below, SHADER_fenvEmissionScale_TexWidth, SHADER_cAmbient_LightRange, SHADER_tex_env, SHADER_tex_diffuse_env,
      SHADER_cClearcoat_EdgeAlpha, SHADER_cGlossy_ImageLerp, SHADER_u_basic_shade_mode, SHADER_refractionTint_thickness, SHADER_mirrorNormal_factor, SHADER_tex_base_transmission,
      SHADER_tex_reflection, SHADER_tex_refraction, SHADER_tex_probe_depth, SHADER_clip_plane),
   SHADER_TECHNIQUE(basic_without_texture_isMetal),
   SHADER_TECHNIQUE(basic_without_texture_refl),
   SHADER_TECHNIQUE(basic_without_texture_refl_isMetal),
   SHADER_TECHNIQUE(basic_without_texture_refr),
   SHADER_TECHNIQUE(basic_without_texture_refr_isMetal),
   SHADER_TECHNIQUE(basic_without_texture_refr_refl),
   SHADER_TECHNIQUE(basic_without_texture_refr_refl_isMetal),

   // Unshaded
   SHADER_TECHNIQUE(unshaded_without_texture, SHADER_matWorldViewProj, SHADER_matWorld, SHADER_matWorldView, SHADER_matWorldViewInverseTranspose, SHADER_staticColor_Alpha, SHADER_clip_plane),
   SHADER_TECHNIQUE(unshaded_with_texture, SHADER_matWorldViewProj, SHADER_matWorld, SHADER_matWorldView, SHADER_matWorldViewInverseTranspose, SHADER_staticColor_Alpha,
      SHADER_tex_base_color, SHADER_clip_plane),
   SHADER_TECHNIQUE(unshaded_without_texture_shadow, SHADER_matWorldViewProj, SHADER_matWorld, SHADER_matWorldView, SHADER_matWorldViewInverseTranspose, SHADER_lightCenter_doShadow,
      SHADER_balls, SHADER_staticColor_Alpha, SHADER_clip_plane),
   SHADER_TECHNIQUE(unshaded_with_texture_shadow, SHADER_matWorldViewProj, SHADER_matWorld, SHADER_matWorldView, SHADER_matWorldViewInverseTranspose, SHADER_lightCenter_doShadow,
      SHADER_balls, SHADER_staticColor_Alpha, SHADER_tex_base_color, SHADER_clip_plane),

   SHADER_TECHNIQUE(basic_reflection_only, SHADER_matWorldViewProj, SHADER_matWorld, SHADER_matWorldView, SHADER_matWorldViewInverseTranspose, SHADER_staticColor_Alpha, SHADER_w_h_height,
      SHADER_mirrorNormal_factor, SHADER_tex_reflection, SHADER_clip_plane),

   SHADER_TECHNIQUE(vr_mask, SHADER_matWorldViewProj),

   SHADER_TECHNIQUE(bg_decal_without_texture, SHADER_matWorldViewProj, SHADER_matWorld, SHADER_matWorldView, SHADER_matWorldViewInverseTranspose, SHADER_cBase_Alpha, SHADER_clip_plane),
   SHADER_TECHNIQUE(bg_decal_with_texture, SHADER_alphaTestValue, SHADER_matWorldViewProj, SHADER_matWorld, SHADER_matWorldView, SHADER_matWorldViewInverseTranspose, SHADER_cBase_Alpha,
      SHADER_tex_base_color, SHADER_clip_plane),

   SHADER_TECHNIQUE(kickerBoolean, SHADER_matProj, SHADER_matWorldViewProj, SHADER_matWorld, SHADER_matView, SHADER_matWorldView, SHADER_matWorldViewInverseTranspose,
      SHADER_lightCenter_doShadow, SHADER_balls, SHADER_staticColor_Alpha, SHADER_w_h_height, SHADER_basicLightEmission, SHADER_basicLightPos, SHADER_Roughness_WrapL_Edge_Thickness,
      SHADER_cBase_Alpha, SHADER_fDisableLighting_top_below, SHADER_fenvEmissionScale_TexWidth, SHADER_cAmbient_LightRange, SHADER_tex_env, SHADER_tex_diffuse_env,
      SHADER_cClearcoat_EdgeAlpha, SHADER_cGlossy_ImageLerp, SHADER_u_basic_shade_mode, SHADER_refractionTint_thickness, SHADER_mirrorNormal_factor, SHADER_tex_base_transmission,
      SHADER_tex_reflection, SHADER_tex_refraction, SHADER_tex_probe_depth, SHADER_clip_plane),
   SHADER_TECHNIQUE(kickerBoolean_isMetal, SHADER_matProj, SHADER_matWorldViewProj, SHADER_matWorld, SHADER_matView, SHADER_matWorldView, SHADER_matWorldViewInverseTranspose,
      SHADER_lightCenter_doShadow, SHADER_balls, SHADER_staticColor_Alpha, SHADER_w_h_height, SHADER_basicLightEmission, SHADER_basicLightPos, SHADER_Roughness_WrapL_Edge_Thickness,
      SHADER_cBase_Alpha, SHADER_fDisableLighting_top_below, SHADER_fenvEmissionScale_TexWidth, SHADER_cAmbient_LightRange, SHADER_tex_env, SHADER_tex_diffuse_env,
      SHADER_cClearcoat_EdgeAlpha, SHADER_cGlossy_ImageLerp, SHADER_u_basic_shade_mode, SHADER_refractionTint_thickness, SHADER_mirrorNormal_factor, SHADER_tex_base_transmission,
      SHADER_tex_reflection, SHADER_tex_refraction, SHADER_tex_probe_depth, SHADER_clip_plane),

   SHADER_TECHNIQUE(light_with_texture, SHADER_matWorldViewProj, SHADER_matWorld, SHADER_matView, SHADER_matWorldView, SHADER_matWorldViewInverseTranspose, SHADER_basicLightEmission,
      SHADER_basicLightPos, SHADER_Roughness_WrapL_Edge_Thickness, SHADER_cBase_Alpha, SHADER_fDisableLighting_top_below, SHADER_fenvEmissionScale_TexWidth, SHADER_cAmbient_LightRange,
      SHADER_tex_env, SHADER_tex_diffuse_env, SHADER_cClearcoat_EdgeAlpha, SHADER_cGlossy_ImageLerp, SHADER_u_basic_shade_mode, SHADER_lightCenter_maxRange, SHADER_lightColor2_falloff_power,
      SHADER_lightColor_intensity, SHADER_lightingOff, SHADER_tex_light_color, SHADER_clip_plane),
   SHADER_TECHNIQUE(light_without_texture, SHADER_matWorldViewProj, SHADER_matWorld, SHADER_matView, SHADER_matWorldView, SHADER_matWorldViewInverseTranspose, SHADER_basicLightEmission,
      SHADER_basicLightPos, SHADER_Roughness_WrapL_Edge_Thickness, SHADER_cBase_Alpha, SHADER_fDisableLighting_top_below, SHADER_fenvEmissionScale_TexWidth, SHADER_cAmbient_LightRange,
      SHADER_tex_env, SHADER_tex_diffuse_env, SHADER_cClearcoat_EdgeAlpha, SHADER_cGlossy_ImageLerp, SHADER_u_basic_shade_mode, SHADER_lightCenter_maxRange, SHADER_lightColor2_falloff_power,
      SHADER_lightColor_intensity, SHADER_lightingOff, SHADER_clip_plane),
   SHADER_TECHNIQUE(light_with_texture_isMetal, SHADER_matWorldViewProj, SHADER_matWorld, SHADER_matView, SHADER_matWorldView, SHADER_matWorldViewInverseTranspose, SHADER_basicLightEmission,
      SHADER_basicLightPos, SHADER_Roughness_WrapL_Edge_Thickness, SHADER_cBase_Alpha, SHADER_fDisableLighting_top_below, SHADER_fenvEmissionScale_TexWidth, SHADER_cAmbient_LightRange,
      SHADER_tex_env, SHADER_tex_diffuse_env, SHADER_cClearcoat_EdgeAlpha, SHADER_cGlossy_ImageLerp, SHADER_u_basic_shade_mode, SHADER_lightCenter_maxRange, SHADER_lightColor2_falloff_power,
      SHADER_lightColor_intensity, SHADER_lightingOff, SHADER_tex_light_color, SHADER_clip_plane),
   SHADER_TECHNIQUE(light_without_texture_isMetal, SHADER_matWorldViewProj, SHADER_matWorld, SHADER_matView, SHADER_matWorldView, SHADER_matWorldViewInverseTranspose,
      SHADER_basicLightEmission, SHADER_basicLightPos, SHADER_Roughness_WrapL_Edge_Thickness, SHADER_cBase_Alpha, SHADER_fDisableLighting_top_below, SHADER_fenvEmissionScale_TexWidth,
      SHADER_cAmbient_LightRange, SHADER_tex_env, SHADER_tex_diffuse_env, SHADER_cClearcoat_EdgeAlpha, SHADER_cGlossy_ImageLerp, SHADER_u_basic_shade_mode, SHADER_lightCenter_maxRange,
      SHADER_lightColor2_falloff_power, SHADER_lightColor_intensity, SHADER_lightingOff, SHADER_clip_plane),

   SHADER_TECHNIQUE(basic_DMD, SHADER_glassArea, SHADER_vRes_Alpha_time, SHADER_vColor_Intensity, SHADER_tex_dmd),
   SHADER_TECHNIQUE(basic_DMD_world, SHADER_glassArea, SHADER_matWorldViewProj, SHADER_vRes_Alpha_time, SHADER_vColor_Intensity, SHADER_tex_dmd),
   SHADER_TECHNIQUE(basic_DMD_ext, SHADER_glassArea, SHADER_vRes_Alpha_time, SHADER_vColor_Intensity, SHADER_tex_dmd),
   SHADER_TECHNIQUE(basic_DMD_world_ext, SHADER_glassArea, SHADER_matWorldViewProj, SHADER_vRes_Alpha_time, SHADER_vColor_Intensity, SHADER_tex_dmd),

   SHADER_TECHNIQUE(display_DMD, SHADER_vRes_Alpha_time, SHADER_w_h_height, SHADER_displayProperties, SHADER_glassPad, SHADER_glassArea, SHADER_glassTint_Roughness, SHADER_displayGlass, SHADER_vColor_Intensity, SHADER_staticColor_Alpha, SHADER_displayTex),
   SHADER_TECHNIQUE(display_DMD_world, SHADER_matWorldViewProj, SHADER_vRes_Alpha_time, SHADER_w_h_height, SHADER_displayProperties, SHADER_glassPad, SHADER_glassArea, SHADER_glassTint_Roughness, SHADER_displayGlass, SHADER_vColor_Intensity, SHADER_staticColor_Alpha, SHADER_displayTex),
   SHADER_TECHNIQUE(display_Seg, SHADER_alphaSegState, SHADER_glassPad, SHADER_glassArea, SHADER_glassTint_Roughness, SHADER_displayProperties, SHADER_displayGlass, SHADER_vColor_Intensity, SHADER_staticColor_Alpha, SHADER_w_h_height, SHADER_displayTex),
   SHADER_TECHNIQUE(display_Seg_world, SHADER_matWorldViewProj, SHADER_alphaSegState, SHADER_glassPad, SHADER_glassArea, SHADER_glassTint_Roughness, SHADER_displayProperties, SHADER_displayGlass, SHADER_vColor_Intensity, SHADER_staticColor_Alpha, SHADER_w_h_height, SHADER_displayTex),
   SHADER_TECHNIQUE(display_CRT, SHADER_glassPad, SHADER_glassArea, SHADER_glassTint_Roughness, SHADER_displayGlass, SHADER_vColor_Intensity, SHADER_staticColor_Alpha, SHADER_w_h_height, SHADER_displayTex),
   SHADER_TECHNIQUE(display_CRT_world, SHADER_matWorldViewProj, SHADER_glassPad, SHADER_glassArea, SHADER_glassTint_Roughness, SHADER_displayGlass, SHADER_vColor_Intensity, SHADER_staticColor_Alpha, SHADER_w_h_height, SHADER_displayTex),

   SHADER_TECHNIQUE(basic_noDMD, SHADER_glassArea, SHADER_alphaTestValue, SHADER_vColor_Intensity, SHADER_tex_sprite),
   SHADER_TECHNIQUE(basic_noDMD_notex, SHADER_vColor_Intensity),
   SHADER_TECHNIQUE(basic_noDMD_world, SHADER_glassArea, SHADER_alphaTestValue, SHADER_matWorldViewProj, SHADER_vColor_Intensity, SHADER_tex_sprite),

   SHADER_TECHNIQUE(basic_noLight, SHADER_matWorldViewProj, SHADER_lightCenter_doShadow, SHADER_balls, SHADER_staticColor_Alpha, SHADER_alphaTestValueAB_filterMode_addBlend,
      SHADER_amount_blend_modulate_vs_add_flasherMode, SHADER_tex_flasher_A, SHADER_tex_flasher_B, SHADER_clip_plane),

   SHADER_TECHNIQUE(bulb_light, SHADER_matWorldViewProj, SHADER_blend_modulate_vs_add, SHADER_lightCenter_maxRange, SHADER_lightColor2_falloff_power, SHADER_lightColor_intensity, SHADER_clip_plane),
   SHADER_TECHNIQUE(bulb_light_with_ball_shadows, SHADER_matWorldViewProj, SHADER_balls, SHADER_blend_modulate_vs_add, SHADER_lightCenter_maxRange, SHADER_lightColor2_falloff_power,
      SHADER_lightColor_intensity, SHADER_clip_plane),

   SHADER_TECHNIQUE(fb_rhtonemap, SHADER_w_h_height, SHADER_bloom_dither_colorgrade, SHADER_exposure_wcg, SHADER_spline1, SHADER_spline2, SHADER_tex_fb_filtered, SHADER_tex_bloom, SHADER_tex_color_lut,
      SHADER_tex_depth),
   SHADER_TECHNIQUE(fb_rhtonemap_AO, SHADER_w_h_height, SHADER_bloom_dither_colorgrade, SHADER_exposure_wcg, SHADER_spline1, SHADER_spline2, SHADER_tex_fb_filtered, SHADER_tex_bloom,
      SHADER_tex_color_lut, SHADER_tex_ao, SHADER_tex_depth),
   SHADER_TECHNIQUE(fb_rhtonemap_no_filter, SHADER_w_h_height, SHADER_bloom_dither_colorgrade, SHADER_exposure_wcg, SHADER_spline1, SHADER_spline2, SHADER_tex_fb_unfiltered, SHADER_tex_bloom,
      SHADER_tex_color_lut, SHADER_tex_depth),
   SHADER_TECHNIQUE(fb_rhtonemap_AO_no_filter, SHADER_w_h_height, SHADER_bloom_dither_colorgrade, SHADER_exposure_wcg, SHADER_spline1, SHADER_spline2, SHADER_tex_fb_unfiltered, SHADER_tex_bloom,
      SHADER_tex_color_lut, SHADER_tex_ao, SHADER_tex_depth),
   /*
   SHADER_TECHNIQUE(fb_tmtonemap, SHADER_w_h_height, SHADER_bloom_dither_colorgrade, SHADER_exposure_wcg, SHADER_spline1, SHADER_spline2, SHADER_tex_fb_filtered, SHADER_tex_bloom, SHADER_tex_color_lut,
      SHADER_tex_depth, SHADER_tex_tonemap_lut),
   SHADER_TECHNIQUE(fb_tmtonemap_AO, SHADER_w_h_height, SHADER_bloom_dither_colorgrade, SHADER_exposure_wcg, SHADER_spline1, SHADER_spline2, SHADER_tex_fb_filtered, SHADER_tex_bloom,
      SHADER_tex_color_lut, SHADER_tex_ao, SHADER_tex_depth, SHADER_tex_tonemap_lut),
   SHADER_TECHNIQUE(fb_tmtonemap_no_filter, SHADER_w_h_height, SHADER_bloom_dither_colorgrade, SHADER_exposure_wcg, SHADER_spline1, SHADER_spline2, SHADER_tex_fb_unfiltered, SHADER_tex_bloom,
      SHADER_tex_color_lut, SHADER_tex_depth, SHADER_tex_tonemap_lut),
   SHADER_TECHNIQUE(fb_tmtonemap_AO_no_filter, SHADER_w_h_height, SHADER_bloom_dither_colorgrade, SHADER_exposure_wcg, SHADER_spline1, SHADER_spline2, SHADER_tex_fb_unfiltered, SHADER_tex_bloom,
      SHADER_tex_color_lut, SHADER_tex_ao, SHADER_tex_depth, SHADER_tex_tonemap_lut),
   */
   SHADER_TECHNIQUE(fb_fmtonemap, SHADER_w_h_height, SHADER_bloom_dither_colorgrade, SHADER_exposure_wcg, SHADER_spline1, SHADER_spline2, SHADER_tex_fb_filtered, SHADER_tex_bloom, SHADER_tex_color_lut,
      SHADER_tex_depth),
   SHADER_TECHNIQUE(fb_fmtonemap_AO, SHADER_w_h_height, SHADER_bloom_dither_colorgrade, SHADER_exposure_wcg, SHADER_spline1, SHADER_spline2, SHADER_tex_fb_filtered, SHADER_tex_bloom,
      SHADER_tex_color_lut, SHADER_tex_ao, SHADER_tex_depth),
   SHADER_TECHNIQUE(fb_fmtonemap_no_filter, SHADER_w_h_height, SHADER_bloom_dither_colorgrade, SHADER_exposure_wcg, SHADER_spline1, SHADER_spline2, SHADER_tex_fb_unfiltered, SHADER_tex_bloom,
      SHADER_tex_color_lut, SHADER_tex_depth),
   SHADER_TECHNIQUE(fb_fmtonemap_AO_no_filter, SHADER_w_h_height, SHADER_bloom_dither_colorgrade, SHADER_exposure_wcg, SHADER_spline1, SHADER_spline2, SHADER_tex_fb_unfiltered, SHADER_tex_bloom,
      SHADER_tex_color_lut, SHADER_tex_ao, SHADER_tex_depth),
   SHADER_TECHNIQUE(fb_nttonemap, SHADER_w_h_height, SHADER_bloom_dither_colorgrade, SHADER_exposure_wcg, SHADER_spline1, SHADER_spline2, SHADER_tex_fb_filtered, SHADER_tex_bloom, SHADER_tex_color_lut,
      SHADER_tex_depth),
   SHADER_TECHNIQUE(fb_nttonemap_AO, SHADER_w_h_height, SHADER_bloom_dither_colorgrade, SHADER_exposure_wcg, SHADER_spline1, SHADER_spline2, SHADER_tex_fb_filtered, SHADER_tex_bloom,
      SHADER_tex_color_lut, SHADER_tex_ao, SHADER_tex_depth),
   SHADER_TECHNIQUE(fb_nttonemap_no_filter, SHADER_w_h_height, SHADER_bloom_dither_colorgrade, SHADER_exposure_wcg, SHADER_spline1, SHADER_spline2, SHADER_tex_fb_unfiltered, SHADER_tex_bloom,
      SHADER_tex_color_lut, SHADER_tex_depth),
   SHADER_TECHNIQUE(fb_nttonemap_AO_no_filter, SHADER_w_h_height, SHADER_bloom_dither_colorgrade, SHADER_exposure_wcg, SHADER_spline1, SHADER_spline2, SHADER_tex_fb_unfiltered, SHADER_tex_bloom,
      SHADER_tex_color_lut, SHADER_tex_ao, SHADER_tex_depth),
   SHADER_TECHNIQUE(fb_agxtonemap, SHADER_w_h_height, SHADER_bloom_dither_colorgrade, SHADER_exposure_wcg, SHADER_spline1, SHADER_spline2, SHADER_tex_fb_filtered, SHADER_tex_bloom, SHADER_tex_color_lut, SHADER_tex_depth),
   SHADER_TECHNIQUE(fb_agxtonemap_AO, SHADER_w_h_height, SHADER_bloom_dither_colorgrade, SHADER_exposure_wcg, SHADER_spline1, SHADER_spline2, SHADER_tex_fb_filtered, SHADER_tex_bloom, SHADER_tex_color_lut, SHADER_tex_ao, SHADER_tex_depth),
   SHADER_TECHNIQUE(fb_agxtonemap_no_filter, SHADER_w_h_height, SHADER_bloom_dither_colorgrade, SHADER_exposure_wcg, SHADER_spline1, SHADER_spline2, SHADER_tex_fb_unfiltered, SHADER_tex_bloom, SHADER_tex_color_lut, SHADER_tex_depth),
   SHADER_TECHNIQUE(fb_agxtonemap_AO_no_filter, SHADER_w_h_height, SHADER_bloom_dither_colorgrade, SHADER_exposure_wcg, SHADER_spline1, SHADER_spline2, SHADER_tex_fb_unfiltered, SHADER_tex_bloom, SHADER_tex_color_lut, SHADER_tex_ao, SHADER_tex_depth),
   SHADER_TECHNIQUE(fb_agxptonemap, SHADER_w_h_height, SHADER_bloom_dither_colorgrade, SHADER_exposure_wcg, SHADER_spline1, SHADER_spline2, SHADER_tex_fb_filtered, SHADER_tex_bloom, SHADER_tex_color_lut, SHADER_tex_depth),
   SHADER_TECHNIQUE(fb_agxptonemap_AO, SHADER_w_h_height, SHADER_bloom_dither_colorgrade, SHADER_exposure_wcg, SHADER_spline1, SHADER_spline2, SHADER_tex_fb_filtered, SHADER_tex_bloom, SHADER_tex_color_lut, SHADER_tex_ao, SHADER_tex_depth),
   SHADER_TECHNIQUE(fb_agxptonemap_no_filter, SHADER_w_h_height, SHADER_bloom_dither_colorgrade, SHADER_exposure_wcg, SHADER_spline1, SHADER_spline2, SHADER_tex_fb_unfiltered, SHADER_tex_bloom, SHADER_tex_color_lut, SHADER_tex_depth),
   SHADER_TECHNIQUE(fb_agxptonemap_AO_no_filter, SHADER_w_h_height, SHADER_bloom_dither_colorgrade, SHADER_exposure_wcg, SHADER_spline1, SHADER_spline2, SHADER_tex_fb_unfiltered, SHADER_tex_bloom, SHADER_tex_color_lut, SHADER_tex_ao, SHADER_tex_depth),
   SHADER_TECHNIQUE(fb_agxgtonemap, SHADER_w_h_height, SHADER_bloom_dither_colorgrade, SHADER_exposure_wcg, SHADER_spline1, SHADER_spline2, SHADER_tex_fb_filtered, SHADER_tex_bloom, SHADER_tex_color_lut, SHADER_tex_depth),
   SHADER_TECHNIQUE(fb_agxgtonemap_AO, SHADER_w_h_height, SHADER_bloom_dither_colorgrade, SHADER_exposure_wcg, SHADER_spline1, SHADER_spline2, SHADER_tex_fb_filtered, SHADER_tex_bloom, SHADER_tex_color_lut, SHADER_tex_ao, SHADER_tex_depth),
   SHADER_TECHNIQUE(fb_agxgtonemap_no_filter, SHADER_w_h_height, SHADER_bloom_dither_colorgrade, SHADER_exposure_wcg, SHADER_spline1, SHADER_spline2, SHADER_tex_fb_unfiltered, SHADER_tex_bloom, SHADER_tex_color_lut, SHADER_tex_depth),
   SHADER_TECHNIQUE(fb_agxgtonemap_AO_no_filter, SHADER_w_h_height, SHADER_bloom_dither_colorgrade, SHADER_exposure_wcg, SHADER_spline1, SHADER_spline2, SHADER_tex_fb_unfiltered, SHADER_tex_bloom, SHADER_tex_color_lut, SHADER_tex_ao, SHADER_tex_depth),
   SHADER_TECHNIQUE(fb_wcgtonemap, SHADER_w_h_height, SHADER_bloom_dither_colorgrade, SHADER_exposure_wcg, SHADER_spline1, SHADER_spline2, SHADER_tex_fb_filtered, SHADER_tex_bloom, SHADER_tex_color_lut, SHADER_tex_depth),
   SHADER_TECHNIQUE(fb_wcgtonemap_AO, SHADER_w_h_height, SHADER_bloom_dither_colorgrade, SHADER_exposure_wcg, SHADER_spline1, SHADER_spline2, SHADER_tex_fb_filtered, SHADER_tex_bloom, SHADER_tex_color_lut, SHADER_tex_ao, SHADER_tex_depth),
   SHADER_TECHNIQUE(fb_wcgtonemap_no_filter, SHADER_w_h_height, SHADER_bloom_dither_colorgrade, SHADER_exposure_wcg, SHADER_spline1, SHADER_spline2, SHADER_tex_fb_unfiltered, SHADER_tex_bloom, SHADER_tex_color_lut, SHADER_tex_depth),
   SHADER_TECHNIQUE(fb_wcgtonemap_AO_no_filter, SHADER_w_h_height, SHADER_bloom_dither_colorgrade, SHADER_exposure_wcg, SHADER_spline1, SHADER_spline2, SHADER_tex_fb_unfiltered, SHADER_tex_bloom, SHADER_tex_color_lut, SHADER_tex_ao, SHADER_tex_depth),
   SHADER_TECHNIQUE(fb_rhtonemap_no_filterRG, SHADER_bloom_dither_colorgrade, SHADER_exposure_wcg, SHADER_spline1, SHADER_spline2, SHADER_tex_fb_unfiltered, SHADER_tex_bloom, SHADER_tex_depth),
   SHADER_TECHNIQUE(fb_rhtonemap_no_filterR, SHADER_bloom_dither_colorgrade, SHADER_exposure_wcg, SHADER_spline1, SHADER_spline2, SHADER_tex_fb_unfiltered, SHADER_tex_bloom, SHADER_tex_depth),

   SHADER_TECHNIQUE(fb_blur_horiz7x7, SHADER_w_h_height, SHADER_tex_fb_filtered),
   SHADER_TECHNIQUE(fb_blur_vert7x7, SHADER_w_h_height, SHADER_tex_fb_filtered),
   SHADER_TECHNIQUE(fb_blur_horiz9x9, SHADER_w_h_height, SHADER_tex_fb_filtered),
   SHADER_TECHNIQUE(fb_blur_vert9x9, SHADER_w_h_height, SHADER_tex_fb_filtered),
   SHADER_TECHNIQUE(fb_blur_horiz11x11, SHADER_w_h_height, SHADER_tex_fb_filtered),
   SHADER_TECHNIQUE(fb_blur_vert11x11, SHADER_w_h_height, SHADER_tex_fb_filtered),
   SHADER_TECHNIQUE(fb_blur_horiz13x13, SHADER_w_h_height, SHADER_tex_fb_filtered),
   SHADER_TECHNIQUE(fb_blur_vert13x13, SHADER_w_h_height, SHADER_tex_fb_filtered),
   SHADER_TECHNIQUE(fb_blur_horiz15x15, SHADER_w_h_height, SHADER_tex_fb_filtered),
   SHADER_TECHNIQUE(fb_blur_vert15x15, SHADER_w_h_height, SHADER_tex_fb_filtered),
   SHADER_TECHNIQUE(fb_blur_horiz19x19, SHADER_w_h_height, SHADER_tex_fb_filtered),
   SHADER_TECHNIQUE(fb_blur_vert19x19, SHADER_w_h_height, SHADER_tex_fb_filtered),
   SHADER_TECHNIQUE(fb_blur_horiz23x23, SHADER_w_h_height, SHADER_tex_fb_filtered),
   SHADER_TECHNIQUE(fb_blur_vert23x23, SHADER_w_h_height, SHADER_tex_fb_filtered),
   SHADER_TECHNIQUE(fb_blur_horiz27x27, SHADER_w_h_height, SHADER_tex_fb_filtered),
   SHADER_TECHNIQUE(fb_blur_vert27x27, SHADER_w_h_height, SHADER_tex_fb_filtered),
   SHADER_TECHNIQUE(fb_blur_horiz39x39, SHADER_w_h_height, SHADER_tex_fb_filtered),
   SHADER_TECHNIQUE(fb_blur_vert39x39, SHADER_w_h_height, SHADER_tex_fb_filtered),

   SHADER_TECHNIQUE(AO, SHADER_w_h_height, SHADER_AO_scale_timeblur, SHADER_tex_fb_filtered, SHADER_tex_depth, SHADER_tex_ao_dither),
   SHADER_TECHNIQUE(fb_AO, SHADER_w_h_height, SHADER_tex_ao), // Display debug AO
   SHADER_TECHNIQUE(fb_AO_static, SHADER_w_h_height, SHADER_tex_fb_filtered, SHADER_tex_ao), // Apply AO during static prerender pass (no tonemapping)
   SHADER_TECHNIQUE(fb_AO_no_filter_static, SHADER_w_h_height, SHADER_tex_fb_filtered, SHADER_tex_ao), // Apply AO during static prerender pass (no tonemapping)
   SHADER_TECHNIQUE(fb_motionblur, SHADER_w_h_height, SHADER_tex_bloom, SHADER_tex_fb_filtered, SHADER_tex_depth, SHADER_matProj, SHADER_matProjInv, SHADER_balls),
   SHADER_TECHNIQUE(fb_bloom, SHADER_w_h_height, SHADER_tex_fb_filtered),
   SHADER_TECHNIQUE(fb_mirror, SHADER_w_h_height, SHADER_tex_fb_unfiltered),
   SHADER_TECHNIQUE(fb_copy, SHADER_tex_fb_filtered),
   SHADER_TECHNIQUE(SSReflection, SHADER_w_h_height, SHADER_SSR_bumpHeight_fresnelRefl_scale_FS, SHADER_tex_fb_filtered, SHADER_tex_depth, SHADER_tex_ao_dither),

   SHADER_TECHNIQUE(NFAA, SHADER_w_h_height, SHADER_tex_fb_filtered, SHADER_tex_depth),
   SHADER_TECHNIQUE(DLAA_edge, SHADER_w_h_height, SHADER_tex_fb_filtered),
   SHADER_TECHNIQUE(DLAA, SHADER_w_h_height, SHADER_tex_fb_filtered, SHADER_tex_depth),
   SHADER_TECHNIQUE(FXAA1, SHADER_w_h_height, SHADER_tex_fb_filtered, SHADER_tex_depth),
   SHADER_TECHNIQUE(FXAA2, SHADER_w_h_height, SHADER_tex_fb_filtered, SHADER_tex_depth),
   SHADER_TECHNIQUE(FXAA3, SHADER_w_h_height, SHADER_tex_fb_filtered, SHADER_tex_depth),
   SHADER_TECHNIQUE(CAS, SHADER_w_h_height, SHADER_tex_fb_unfiltered, SHADER_tex_depth),
   SHADER_TECHNIQUE(BilateralSharp_CAS, SHADER_w_h_height, SHADER_tex_fb_unfiltered, SHADER_tex_depth),
#ifndef __OPENGLES__
   SHADER_TECHNIQUE(SMAA_ColorEdgeDetection, SHADER_w_h_height, SHADER_tex_fb_filtered),
   SHADER_TECHNIQUE(SMAA_BlendWeightCalculation, SHADER_w_h_height, SHADER_edgesTex, SHADER_areaTex, SHADER_searchTex),
   SHADER_TECHNIQUE(SMAA_NeighborhoodBlending, SHADER_w_h_height, SHADER_tex_fb_filtered, SHADER_blendTex),
#endif

   SHADER_TECHNIQUE(stereo_SBS, SHADER_w_h_height, SHADER_tex_stereo_fb, SHADER_tex_stereo_depth, SHADER_Stereo_MS_ZPD_YAxis),
   SHADER_TECHNIQUE(stereo_TB, SHADER_w_h_height, SHADER_tex_stereo_fb, SHADER_tex_stereo_depth, SHADER_Stereo_MS_ZPD_YAxis),
   SHADER_TECHNIQUE(stereo_Int, SHADER_w_h_height, SHADER_tex_stereo_fb, SHADER_tex_stereo_depth, SHADER_Stereo_MS_ZPD_YAxis),
   SHADER_TECHNIQUE(stereo_Flipped_Int, SHADER_w_h_height, SHADER_tex_stereo_fb, SHADER_tex_stereo_depth, SHADER_Stereo_MS_ZPD_YAxis),
   SHADER_TECHNIQUE(Stereo_sRGBAnaglyph, SHADER_w_h_height, SHADER_tex_stereo_fb, SHADER_tex_stereo_depth, SHADER_Stereo_MS_ZPD_YAxis, SHADER_Stereo_LeftMat, SHADER_Stereo_RightMat),
   SHADER_TECHNIQUE(Stereo_GammaAnaglyph, SHADER_w_h_height, SHADER_tex_stereo_fb, SHADER_tex_stereo_depth, SHADER_Stereo_MS_ZPD_YAxis, SHADER_Stereo_LeftMat, SHADER_Stereo_RightMat,
      SHADER_Stereo_LeftLuminance_Gamma),
   SHADER_TECHNIQUE(Stereo_sRGBDynDesatAnaglyph, SHADER_w_h_height, SHADER_tex_stereo_fb, SHADER_tex_stereo_depth, SHADER_Stereo_MS_ZPD_YAxis, SHADER_Stereo_LeftMat, SHADER_Stereo_RightMat,
      SHADER_Stereo_LeftLuminance_Gamma, SHADER_Stereo_RightLuminance_DynDesat),
   SHADER_TECHNIQUE(Stereo_GammaDynDesatAnaglyph, SHADER_w_h_height, SHADER_tex_stereo_fb, SHADER_tex_stereo_depth, SHADER_Stereo_MS_ZPD_YAxis, SHADER_Stereo_LeftMat, SHADER_Stereo_RightMat,
      SHADER_Stereo_LeftLuminance_Gamma, SHADER_Stereo_RightLuminance_DynDesat),
   SHADER_TECHNIQUE(Stereo_DeghostAnaglyph, SHADER_w_h_height, SHADER_tex_stereo_fb, SHADER_tex_stereo_depth, SHADER_Stereo_MS_ZPD_YAxis, SHADER_Stereo_LeftMat, SHADER_Stereo_RightMat,
      SHADER_Stereo_DeghostGamma, SHADER_Stereo_DeghostFilter),

   SHADER_TECHNIQUE(irradiance, SHADER_tex_env),
};
#undef SHADER_TECHNIQUE

ShaderTechniques Shader::getTechniqueByName(const string& name)
{
   for (int i = 0; i < SHADER_TECHNIQUE_COUNT; ++i)
      if (name == shaderTechniqueNames[i].name)
         return ShaderTechniques(i);
   PLOGE << "getTechniqueByName Could not find technique " << name << " in shaderTechniqueNames.";
   return SHADER_TECHNIQUE_INVALID;
}

string Shader::GetTechniqueName(ShaderTechniques technique)
{
   assert(0 <= technique && technique < SHADER_TECHNIQUE_COUNT);
   return shaderTechniqueNames[technique].name;
}


#define SHADER_UNIFORM(type, name, count) { type, #name, count, 0, SA_UNDEFINED, SA_UNDEFINED, SF_UNDEFINED }
#define SHADER_SAMPLER(name, tex_unit, default_clampu, default_clampv, default_filter) { SUT_Sampler, #name, 1, tex_unit, default_clampu, default_clampv, default_filter }
Shader::ShaderUniform Shader::shaderUniformNames[SHADER_UNIFORM_COUNT] {
   // Shared uniforms
   SHADER_UNIFORM(SUT_Int, layer, 1),
   SHADER_UNIFORM(SUT_Float, alphaTestValue, 1),
   SHADER_UNIFORM(SUT_Float4x4, matProj, 1), // +1 Matrix for stereo
   SHADER_UNIFORM(SUT_Float4x4, matProjInv, 1), // +1 Matrix for stereo
   SHADER_UNIFORM(SUT_Float4x4, matWorldViewProj, 1), // +1 Matrix for stereo
   SHADER_UNIFORM(SUT_DataBlock, basicMatrixBlock, 5 * 16 * 4), // OpenGL only, +1 Matrix for stereo
   SHADER_UNIFORM(SUT_DataBlock, ballMatrixBlock, 4 * 16 * 4), // OpenGL only, +1 Matrix for stereo
   SHADER_UNIFORM(SUT_Float4x4, matWorld, 1), // DX9 & BGFX only
   SHADER_UNIFORM(SUT_Float4x3, matView, 1), // DX9 & BGFX only
   SHADER_UNIFORM(SUT_Float4x4, matWorldView, 1), // DX9 & BGFX only
   SHADER_UNIFORM(SUT_Float4x3, matWorldViewInverse, 1), // DX9 & BGFX only
   SHADER_UNIFORM(SUT_Float3x4, matWorldViewInverseTranspose, 1), // DX9 & BGFX only
   SHADER_UNIFORM(SUT_Float4, lightCenter_doShadow, 1), // Basic & Flasher (for ball shadows)
   SHADER_UNIFORM(SUT_Float4v, balls, 8), // Basic & Flasher (for ball shadows)
   SHADER_UNIFORM(SUT_Float4, staticColor_Alpha, 1), // Basic & Flasher
   SHADER_UNIFORM(SUT_Float4, w_h_height, 1), // Post process & Basic (for screen space reflection/refraction)

   // Shared material for Ball, Basic and Classic light shaders
   SHADER_UNIFORM(SUT_Float4, clip_plane, 1), // OpenGL & BGFX only
   SHADER_UNIFORM(SUT_Float4v, basicLightEmission, 2), // OpenGL & BGFX only
   SHADER_UNIFORM(SUT_Float4v, basicLightPos, 2), // OpenGL & BGFX only
   SHADER_UNIFORM(SUT_Float4v, ballLightEmission, 10), // OpenGL & BGFX only
   SHADER_UNIFORM(SUT_Float4v, ballLightPos, 10), // OpenGL & BGFX only
   SHADER_UNIFORM(SUT_Bool, is_metal, 1), // OpenGL & BGFX only
   SHADER_UNIFORM(SUT_Bool, doNormalMapping, 1), // OpenGL & BGFX only
   SHADER_UNIFORM(SUT_Float4v, basicPackedLights, 3), // DX9 only
   SHADER_UNIFORM(SUT_Float4v, ballPackedLights, 15), // DX9 only
   SHADER_UNIFORM(SUT_Float4, Roughness_WrapL_Edge_Thickness, 1),
   SHADER_UNIFORM(SUT_Float4, cBase_Alpha, 1),
   SHADER_UNIFORM(SUT_Float2, fDisableLighting_top_below, 1),
   SHADER_UNIFORM(SUT_Float2, fenvEmissionScale_TexWidth, 1),
   SHADER_UNIFORM(SUT_Float4, cAmbient_LightRange, 1),
   SHADER_SAMPLER(tex_env, 1, SA_REPEAT, SA_CLAMP, SF_TRILINEAR), // environment
   SHADER_SAMPLER(tex_diffuse_env, 2, SA_REPEAT, SA_CLAMP, SF_BILINEAR), // diffuse environment contribution/radiance

   // Basic Shader
   SHADER_UNIFORM(SUT_Float4, cClearcoat_EdgeAlpha, 1),
   SHADER_UNIFORM(SUT_Float4, cGlossy_ImageLerp, 1),
   SHADER_UNIFORM(SUT_Bool, doRefractions, 1), // OpenGL only
   SHADER_UNIFORM(SUT_Float4, u_basic_shade_mode, 1), // BGFX Only
   SHADER_UNIFORM(SUT_Float4, refractionTint_thickness, 1),
   SHADER_UNIFORM(SUT_Float4, mirrorNormal_factor, 1),
   SHADER_UNIFORM(SUT_Bool, objectSpaceNormalMap, 1),
   SHADER_SAMPLER(tex_base_color, 0, SA_CLAMP, SA_CLAMP, SF_TRILINEAR), // base texture
   SHADER_SAMPLER(tex_base_transmission, 3, SA_CLAMP, SA_CLAMP, SF_BILINEAR), // bulb light/transmission buffer texture
   SHADER_SAMPLER(tex_base_normalmap, 4, SA_REPEAT, SA_REPEAT, SF_TRILINEAR), // normal map texture
   SHADER_SAMPLER(tex_reflection, 5, SA_CLAMP, SA_CLAMP, SF_BILINEAR), // plane reflection
   SHADER_SAMPLER(tex_refraction, 6, SA_CLAMP, SA_CLAMP, SF_BILINEAR), // screen space refraction
   SHADER_SAMPLER(tex_probe_depth, 7, SA_CLAMP, SA_CLAMP, SF_BILINEAR), // depth probe

   // Ball Shader
   SHADER_UNIFORM(SUT_Float4x3, orientation, 1),
   SHADER_UNIFORM(SUT_Float4, invTableRes_reflection, 1),
   SHADER_UNIFORM(SUT_Float4, w_h_disableLighting, 1),
   SHADER_SAMPLER(tex_ball_color, 0, SA_REPEAT, SA_REPEAT, SF_TRILINEAR), // base texture
   SHADER_SAMPLER(tex_ball_playfield, 4, SA_CLAMP, SA_CLAMP, SF_TRILINEAR), // playfield
   SHADER_SAMPLER(tex_ball_decal, 3, SA_REPEAT, SA_REPEAT, SF_TRILINEAR), // ball decal

   // Light Shader
   SHADER_UNIFORM(SUT_Float, blend_modulate_vs_add, 1),
   SHADER_UNIFORM(SUT_Float4, lightCenter_maxRange, 1), // Classic and Halo
   SHADER_UNIFORM(SUT_Float4, lightColor2_falloff_power, 1), // Classic and Halo
   SHADER_UNIFORM(SUT_Float4, lightColor_intensity, 1), // Classic and Halo
   SHADER_UNIFORM(SUT_Bool, lightingOff, 1), // Classic only
   SHADER_SAMPLER(tex_light_color, 0, SA_REPEAT, SA_REPEAT, SF_TRILINEAR), // Classic only

   // DMD Shader
   SHADER_UNIFORM(SUT_Float4, glassPad, 1),
   SHADER_UNIFORM(SUT_Float4, glassArea, 1),
   SHADER_UNIFORM(SUT_Float4, vRes_Alpha_time, 1),
   SHADER_UNIFORM(SUT_Float4, backBoxSize, 1),
   SHADER_UNIFORM(SUT_Float4, vColor_Intensity, 1),
   SHADER_SAMPLER(tex_dmd, 0, SA_CLAMP, SA_CLAMP, SF_NONE), // DMD
   SHADER_SAMPLER(tex_sprite, 0, SA_MIRROR, SA_MIRROR, SF_TRILINEAR), // Sprite

   // Display Shader
   SHADER_UNIFORM(SUT_Float4, glassTint_Roughness, 1),
   SHADER_UNIFORM(SUT_Float4, displayProperties, 1),
   SHADER_UNIFORM(SUT_Float4v, alphaSegState, 4),
   SHADER_SAMPLER(displayTex, 0, SA_CLAMP, SA_CLAMP, SF_NONE), // DMD (Point sampling), Alpha seg (bilinear sampling), Display (Point sampling)
   SHADER_SAMPLER(displayGlass, 1, SA_CLAMP, SA_CLAMP, SF_TRILINEAR),


   // Flasher Shader
   SHADER_UNIFORM(SUT_Float4, alphaTestValueAB_filterMode_addBlend, 1),
   SHADER_UNIFORM(SUT_Float3, amount_blend_modulate_vs_add_flasherMode, 1),
   SHADER_SAMPLER(tex_flasher_A, 0, SA_CLAMP, SA_CLAMP, SF_TRILINEAR), // base texture
   SHADER_SAMPLER(tex_flasher_B, 1, SA_REPEAT, SA_REPEAT, SF_TRILINEAR), // texB

   // Post Process Shader
   SHADER_UNIFORM(SUT_Float4, bloom_dither_colorgrade, 1),
   SHADER_UNIFORM(SUT_Float4, exposure_wcg, 1),
   SHADER_UNIFORM(SUT_Float4, spline1, 1),
   SHADER_UNIFORM(SUT_Float2, spline2, 1),
   SHADER_UNIFORM(SUT_Float4, SSR_bumpHeight_fresnelRefl_scale_FS, 1),
   SHADER_UNIFORM(SUT_Float2, AO_scale_timeblur, 1),
   SHADER_SAMPLER(tex_fb_unfiltered, 0, SA_CLAMP, SA_CLAMP, SF_NONE), // Framebuffer (unfiltered)
   SHADER_SAMPLER(tex_fb_filtered, 0, SA_CLAMP, SA_CLAMP, SF_BILINEAR), // Framebuffer (filtered)
   SHADER_SAMPLER(tex_bloom, 1, SA_CLAMP, SA_CLAMP, SF_BILINEAR), // Bloom
   SHADER_SAMPLER(tex_color_lut, 2, SA_CLAMP, SA_CLAMP, SF_BILINEAR), // Color grade LUT
   SHADER_SAMPLER(tex_ao, 3, SA_CLAMP, SA_CLAMP, SF_BILINEAR), // AO Result
   SHADER_SAMPLER(tex_depth, 4, SA_CLAMP, SA_CLAMP, SF_NONE), // Depth
   SHADER_SAMPLER(tex_ao_dither, 5, SA_REPEAT, SA_REPEAT, SF_NONE), // AO dither
   //SHADER_SAMPLER(tex_tonemap_lut, 6, SA_CLAMP, SA_CLAMP, SF_BILINEAR), // Tonemap LUT
   SHADER_SAMPLER(edgesTex, 7, SA_CLAMP, SA_CLAMP, SF_TRILINEAR), // SMAA
   SHADER_SAMPLER(blendTex, 8, SA_CLAMP, SA_CLAMP, SF_TRILINEAR), // SMAA
   SHADER_SAMPLER(areaTex, 9, SA_CLAMP, SA_CLAMP, SF_BILINEAR), // SMAA
   SHADER_SAMPLER(searchTex, 10, SA_CLAMP, SA_CLAMP, SF_NONE), // SMAA

   // Stereo Shader
   SHADER_SAMPLER(tex_stereo_fb, 0, SA_REPEAT, SA_REPEAT, SF_NONE), // Framebuffer (unfiltered)
   SHADER_SAMPLER(tex_stereo_depth, 4, SA_REPEAT, SA_REPEAT, SF_NONE), // Depth
   SHADER_UNIFORM(SUT_Float4, Stereo_MS_ZPD_YAxis, 1), // Stereo (analgyph and 3DTV)
   SHADER_UNIFORM(SUT_Float4x4, Stereo_LeftMat, 1), // Anaglyph Stereo
   SHADER_UNIFORM(SUT_Float4x4, Stereo_RightMat, 1), // Anaglyph Stereo
   SHADER_UNIFORM(SUT_Float4, Stereo_DeghostGamma, 1), // Anaglyph Stereo
   SHADER_UNIFORM(SUT_Float4x4, Stereo_DeghostFilter, 1), // Anaglyph Stereo
   SHADER_UNIFORM(SUT_Float4, Stereo_LeftLuminance_Gamma, 1), // Anaglyph Stereo
   SHADER_UNIFORM(SUT_Float4, Stereo_RightLuminance_DynDesat, 1), // Anaglyph Stereo
};
#undef SHADER_UNIFORM
#undef SHADER_SAMPLER

ShaderUniforms Shader::getUniformByName(const string& name)
{
   for (int i = 0; i < SHADER_UNIFORM_COUNT; ++i)
      if (name == shaderUniformNames[i].name)
         return (ShaderUniforms)i;
   PLOGE << '[' << m_shaderCodeName << "] getUniformByName Could not find uniform " << name << " in shaderUniformNames";
   return SHADER_UNIFORM_INVALID;
}

void Shader::SetDefaultSamplerFilter(const ShaderUniforms sampler, const SamplerFilter sf)
{
   Shader::shaderUniformNames[sampler].default_filter = sf;
}

// When changed, this list must also be copied unchanged to Shader.cpp (for its implementation)
#define SHADER_ATTRIBUTE(name, shader_name) #shader_name
const string Shader::shaderAttributeNames[SHADER_ATTRIBUTE_COUNT]
{
   SHADER_ATTRIBUTE(POS, vPosition),
   SHADER_ATTRIBUTE(NORM, vNormal),
   SHADER_ATTRIBUTE(TC, tc),
   SHADER_ATTRIBUTE(TEX, tex0),
};
#undef SHADER_ATTRIBUTE

ShaderAttributes Shader::getAttributeByName(const string& name)
{
   for (int i = 0; i < SHADER_ATTRIBUTE_COUNT; ++i)
      if (name == shaderAttributeNames[i])
         return ShaderAttributes(i);
   PLOGE << '[' << m_shaderCodeName << "] getAttributeByName Could not find attribute " << name << " in shaderAttributeNames";
   return SHADER_ATTRIBUTE_INVALID;
}

Shader* Shader::current_shader = nullptr;
Shader* Shader::GetCurrentShader() { return current_shader;  }

Shader::Shader(RenderDevice* renderDevice, const ShaderId id, const bool isStereo)
   : m_renderDevice(renderDevice)
   , m_technique(SHADER_TECHNIQUE_INVALID)
   , m_shaderId(id)
#if defined(ENABLE_BGFX) || defined(ENABLE_OPENGL)
   , m_isStereo(isStereo)
#endif
{
   #if defined(ENABLE_BGFX)
   const int nEyes = m_isStereo ? 2 : 1;
   shaderUniformNames[SHADER_matProj].count = nEyes;
   shaderUniformNames[SHADER_matProjInv].count = nEyes;
   shaderUniformNames[SHADER_matWorldViewProj].count = nEyes;
   for (int i = 0; i < SHADER_TECHNIQUE_COUNT; i++)
   {
      m_techniques[i] = BGFX_INVALID_HANDLE;
      m_clipPlaneTechniques[i] = BGFX_INVALID_HANDLE;
   }
   for (int i = 0; i < SHADER_UNIFORM_COUNT; i++)
   {
      ShaderUniform u = shaderUniformNames[i];
      bgfx::UniformType::Enum type;
      uint16_t n = u.count;
      switch (u.type)
      {
      case SUT_DataBlock: m_uniformHandles[i] = BGFX_INVALID_HANDLE; continue;
      case SUT_Bool:
      case SUT_Int:
      case SUT_Float:
      case SUT_Float2:
      case SUT_Float3:
      case SUT_Float4:
      case SUT_Float4v: type = bgfx::UniformType::Vec4; break;
      case SUT_Float3x4:
      case SUT_Float4x3:
      case SUT_Float4x4: type = bgfx::UniformType::Mat4; break;
      case SUT_Sampler: type = bgfx::UniformType::Sampler; break;
      default: break;
      }
      m_uniformHandles[i] = bgfx::createUniform(u.name.c_str(), type, n);
   }

   #elif defined(ENABLE_OPENGL)
   const int nEyes = m_isStereo ? 2 : 1;
   shaderUniformNames[SHADER_matProj].count = nEyes;
   shaderUniformNames[SHADER_matProjInv].count = nEyes;
   shaderUniformNames[SHADER_matWorldViewProj].count = nEyes;
   shaderUniformNames[SHADER_basicMatrixBlock].count = (4 + nEyes) * 16 * 4;
   shaderUniformNames[SHADER_ballMatrixBlock].count = (3 + nEyes) * 16 * 4;
   memset(m_techniques, 0, sizeof(ShaderTechnique*) * SHADER_TECHNIQUE_COUNT);

   #elif defined(ENABLE_DX9)
   memset(m_boundTexture, 0, sizeof(IDirect3DTexture9*) * TEXTURESET_STATE_CACHE_SIZE);
   #endif

   Load();
   #ifdef __STANDALONE__
   if (HasError())
      exit(-1);
   #endif

   memset(m_stateOffsets, -1, sizeof(m_stateOffsets));
   memset(m_stateSizes, -1, sizeof(m_stateSizes));
   for (int i = 0; i < SHADER_TECHNIQUE_COUNT; i++)
      for (ShaderUniforms uniform : m_uniforms[i])
         if (m_stateOffsets[uniform] == -1)
         {
            m_stateOffsets[uniform] = m_stateSize;
            switch (shaderUniformNames[uniform].type)
            {
            case SUT_Bool: m_stateSizes[uniform] = shaderUniformNames[uniform].count * sizeof(bool); break;
            case SUT_Int: m_stateSizes[uniform] = shaderUniformNames[uniform].count * sizeof(int); break;
            case SUT_Float: m_stateSizes[uniform] = shaderUniformNames[uniform].count * sizeof(float); break;
            case SUT_Float2: m_stateSizes[uniform] = shaderUniformNames[uniform].count * 2 * sizeof(float); break;
            case SUT_Float3: m_stateSizes[uniform] = shaderUniformNames[uniform].count * 3 * sizeof(float); break;
            case SUT_Float4: m_stateSizes[uniform] = shaderUniformNames[uniform].count * 4 * sizeof(float); break;
            case SUT_Float4v: m_stateSizes[uniform] = shaderUniformNames[uniform].count * 4 * sizeof(float); break;
            case SUT_Float3x4: m_stateSizes[uniform] = shaderUniformNames[uniform].count * 16 * sizeof(float); break;
            case SUT_Float4x3: m_stateSizes[uniform] = shaderUniformNames[uniform].count * 16 * sizeof(float); break;
            case SUT_Float4x4: m_stateSizes[uniform] = shaderUniformNames[uniform].count * 16 * sizeof(float); break;
            case SUT_DataBlock: m_stateSizes[uniform] = shaderUniformNames[uniform].count; break;
            case SUT_Sampler: m_stateSizes[uniform] = shaderUniformNames[uniform].count * sizeof(Sampler*); break;
            default: break;
            }
            m_stateSize += m_stateSizes[uniform];
         }
   m_state = new ShaderState(this, m_renderDevice->UseLowPrecision());
   memset(m_state->m_state, 0, m_stateSize);

   #if defined(ENABLE_BGFX) || defined(ENABLE_OPENGL)
   for (int i = 0; i < SHADER_TECHNIQUE_COUNT; i++)
      #if defined(ENABLE_BGFX)
      if (bgfx::isValid(m_techniques[i]))
      #else
      if (m_techniques[i])
      #endif
      {
         m_boundState[i] = new ShaderState(this, m_renderDevice->UseLowPrecision());
         memset(m_boundState[i]->m_state, 0, m_stateSize);
         for (ShaderUniforms uniform : m_uniforms[i])
         {
            if (shaderUniformNames[uniform].type == SUT_Sampler)
            {
               m_boundState[i]->SetTexture(uniform, m_renderDevice->m_nullTexture);
               m_state->SetTexture(uniform, m_renderDevice->m_nullTexture);
            }
         }
      }
      else
         m_boundState[i] = nullptr;
   // Set default values from Material.fxh for uniforms.
   if (m_stateOffsets[SHADER_cBase_Alpha] != -1)
      SetVector(SHADER_cBase_Alpha, 0.5f, 0.5f, 0.5f, 1.0f);
   if (m_stateOffsets[SHADER_Roughness_WrapL_Edge_Thickness] != -1)
      SetVector(SHADER_Roughness_WrapL_Edge_Thickness, 4.0f, 0.5f, 1.0f, 0.05f);

   #elif defined(ENABLE_DX9)
   m_boundState = new ShaderState(this, m_renderDevice->UseLowPrecision());
   memset(m_boundState->m_state, 0, m_stateSize);
   for (ShaderUniforms uniform : m_uniforms[0])
   {
      if (shaderUniformNames[uniform].type == SUT_Sampler)
      {
         m_boundState->SetTexture(uniform, m_renderDevice->m_nullTexture);
         m_state->SetTexture(uniform, m_renderDevice->m_nullTexture);
      }
   }

   #endif
}

Shader::~Shader()
{
   delete m_state;

   #if defined(ENABLE_BGFX)
      for (int j = 0; j < SHADER_TECHNIQUE_COUNT; ++j)
      {
         delete m_boundState[j];
         if (bgfx::isValid(m_techniques[j]))
            bgfx::destroy(m_techniques[j]);
         m_techniques[j] = BGFX_INVALID_HANDLE;
         if (bgfx::isValid(m_clipPlaneTechniques[j]))
            bgfx::destroy(m_clipPlaneTechniques[j]);
         m_clipPlaneTechniques[j] = BGFX_INVALID_HANDLE;
      }
      for (int i = 0; i < SHADER_UNIFORM_COUNT; i++)
      {
         if (bgfx::isValid(m_uniformHandles[i]))
            bgfx::destroy(m_uniformHandles[i]);
      }

   #elif defined(ENABLE_OPENGL)
      for (int j = 0; j < SHADER_TECHNIQUE_COUNT; ++j)
      {
         if (m_techniques[j] != nullptr)
         {
            glDeleteProgram(m_techniques[j]->program);
            delete m_techniques[j];
            delete m_boundState[j];
         }
      }

   #elif defined(ENABLE_DX9)
      delete m_boundState;
      SAFE_RELEASE(m_shader);

   #endif
}

void Shader::UnbindSampler(Sampler* sampler)
{
   #if defined(ENABLE_DX9)
   for (const auto& uniform : m_uniforms[0])
   {
      const auto& desc = m_uniform_desc[uniform];
      if (desc.uniform.type == SUT_Sampler && (sampler == nullptr || m_boundTexture[desc.sampler] == sampler))
      {
         CHECKD3D(m_shader->SetTexture(desc.tex_handle, nullptr));
         m_boundTexture[desc.sampler] = nullptr;
      }
   }
   #endif
}

void Shader::Begin()
{
   assert(current_shader == nullptr);
   assert(m_technique != SHADER_TECHNIQUE_INVALID);
   current_shader = this;

   #if defined(ENABLE_BGFX)

   #else
   if (m_boundTechnique != m_technique)
   {
      m_renderDevice->m_curTechniqueChanges++;
      m_boundTechnique = m_technique;
      #if defined(ENABLE_OPENGL)
      glUseProgram(m_techniques[m_technique]->program);
      #elif defined(ENABLE_DX9)
      //CHECKD3D(m_shader->SetTechnique((D3DXHANDLE)shaderTechniqueNames[m_technique].name.c_str()));
      const char* const stn = shaderTechniqueNames[m_technique].name.c_str();
      const HRESULT hrTmp = m_shader->SetTechnique((D3DXHANDLE)stn);
      if (FAILED(hrTmp))
      {
         MessageBox(NULL, stn, stn, MB_OK);
         ReportFatalError(hrTmp, __FILE__, __LINE__);
      }
      #endif
   }
   #endif

   for (const auto& uniformName : m_uniforms[m_technique])
      ApplyUniform(uniformName);

   #if defined(ENABLE_DX9)
   unsigned int cPasses;
   CHECKD3D(m_shader->Begin(&cPasses, 0));
   CHECKD3D(m_shader->BeginPass(0));
   #endif
}

void Shader::End()
{
   assert(current_shader == this);
   current_shader = nullptr;
   #if defined(ENABLE_BGFX)
   m_renderDevice->m_program = BGFX_INVALID_HANDLE;
   #elif defined(ENABLE_DX9)
   CHECKD3D(m_shader->EndPass());
   CHECKD3D(m_shader->End());
   #endif
}

void Shader::SetTextureNull(const ShaderUniforms uniformName)
{
   SetTexture(uniformName, m_renderDevice->m_nullTexture);
}

void Shader::SetTexture(const ShaderUniforms uniformName, BaseTexture* const texel, const SamplerFilter filter, const SamplerAddressMode clampU, const SamplerAddressMode clampV, const bool force_linear_rgb)
{
   SetTexture(uniformName, texel ? m_renderDevice->m_texMan.LoadTexture(texel, filter, clampU, clampV, force_linear_rgb) : m_renderDevice->m_nullTexture);
}

void Shader::SetMaterial(const Material* const mat, const bool has_alpha)
{
   COLORREF cBase, cGlossy, cClearcoat;
   float fWrapLighting, fRoughness, fGlossyImageLerp, fThickness, fEdge, fEdgeAlpha, fOpacity;
   bool bIsMetal, bOpacityActive;

   if (mat)
   {
      fWrapLighting = mat->m_fWrapLighting;
      fRoughness = exp2f(10.0f * mat->m_fRoughness + 1.0f); // map from 0..1 to 2..2048
      fGlossyImageLerp = mat->m_fGlossyImageLerp;
      fThickness = mat->m_fThickness;
      fEdge = mat->m_fEdge;
      fEdgeAlpha = mat->m_fEdgeAlpha;
      fOpacity = mat->m_fOpacity;
      cBase = mat->m_cBase;
      cGlossy = mat->m_cGlossy;
      cClearcoat = mat->m_cClearcoat;
      bIsMetal = mat->m_type == Material::MaterialType::METAL;
      bOpacityActive = mat->m_bOpacityActive;
   }
   else
   {
      fWrapLighting = 0.0f;
      fRoughness = exp2f(10.0f * 0.0f + 1.0f); // map from 0..1 to 2..2048
      fGlossyImageLerp = 1.0f;
      fThickness = 0.05f;
      fEdge = 1.0f;
      fEdgeAlpha = 1.0f;
      fOpacity = 1.0f;
      cBase = g_pvp->m_dummyMaterial.m_cBase;
      cGlossy = 0;
      cClearcoat = 0;
      bIsMetal = false;
      bOpacityActive = false;
   }

   SetVector(SHADER_Roughness_WrapL_Edge_Thickness, fRoughness, fWrapLighting, fEdge, fThickness);

   const float alpha = bOpacityActive ? fOpacity : 1.0f;
   const vec4 cBaseF = convertColor(cBase, alpha);
   SetVector(SHADER_cBase_Alpha, &cBaseF);

   const vec4 cGlossyF = bIsMetal ? vec4(0.f, 0.f, 0.f, 0.f) : convertColor(cGlossy, fGlossyImageLerp);
   SetVector(SHADER_cGlossy_ImageLerp, &cGlossyF);

   const vec4 cClearcoatF = convertColor(cClearcoat, fEdgeAlpha);
   SetVector(SHADER_cClearcoat_EdgeAlpha, &cClearcoatF);

   // Before 10.8 when opacity was one:
   // - lighting from below would be disabled,
   // - alpha channel of texture would be discarded,
   // - alpha test was always performed.
   // Therefore lots of table author used a workaround by defining an opacity just below 1 to enable these features.
   //
   // As of VPX 10.8:
   // - the behavior did not change regarding lighting from below,
   // - the alpha channel of the texture is always considered,
   // - alpha blending is only performed if there is an alpha channel or a 'meaningfull' (not 0.999) alpha value (for performance reasons), [Note that Flupper uses near opaque at 0.99 with a light below for his inserts]
   // - alpha test is only applied if there is some alpha and a positive test threshold (to allow early depth testing).
   if (bOpacityActive && (has_alpha || alpha < 0.999f))
      m_renderDevice->EnableAlphaBlend(false);
   else
      m_renderDevice->SetRenderState(RenderState::ALPHABLENDENABLE, RenderState::RS_FALSE);
}

void Shader::SetAlphaTestValue(const float value)
{
   SetFloat(SHADER_alphaTestValue, value);
}

void Shader::SetFlasherData(const vec4& c1, const vec4& c2)
{
   SetVector(SHADER_alphaTestValueAB_filterMode_addBlend, &c1);
   SetVector(SHADER_amount_blend_modulate_vs_add_flasherMode, &c2);
}

void Shader::SetLightColorIntensity(const vec4& color)
{
   SetVector(SHADER_lightColor_intensity, &color);
}

void Shader::SetLightColor2FalloffPower(const vec4& color)
{
   SetVector(SHADER_lightColor2_falloff_power, &color);
}

void Shader::SetLightData(const vec4& color)
{
   SetVector(SHADER_lightCenter_maxRange, &color);
}

void Shader::SetLightImageBackglassMode(const bool imageMode, const bool backglassMode)
{
   SetBool(SHADER_lightingOff, imageMode || backglassMode); // at the moment can be combined into a single bool due to what the shader actually does in the end
}

void Shader::SetTechniqueMaterial(ShaderTechniques technique, const Material& mat, const bool doAlphaTest, const bool doNormalMapping, const bool doReflections, const bool doRefractions)
{
   ShaderTechniques tech = technique;
   const bool isMetal = mat.m_type == Material::MaterialType::METAL;

   #if defined(ENABLE_BGFX)
   // For BGFX doReflections is computed from the reflection factor
   SetVector(SHADER_u_basic_shade_mode, isMetal, doNormalMapping, doRefractions, 0.0f);
   if (tech == SHADER_TECHNIQUE_basic_with_texture && doAlphaTest)
      tech = SHADER_TECHNIQUE_basic_with_texture_at;

   #elif defined(ENABLE_OPENGL)
   // For OpenGL doReflections is computed from the reflection factor
   SetBool(SHADER_is_metal, isMetal);
   SetBool(SHADER_doNormalMapping, doNormalMapping);
   SetBool(SHADER_doRefractions, doRefractions);
   if (tech == SHADER_TECHNIQUE_basic_with_texture && doAlphaTest)
      tech = SHADER_TECHNIQUE_basic_with_texture_at;

   #elif defined(ENABLE_DX9)
   switch (technique)
   {
   case SHADER_TECHNIQUE_basic_with_texture:
   {
      static ShaderTechniques tech_with_texture[32] = {
         SHADER_TECHNIQUE_basic_with_texture,
         SHADER_TECHNIQUE_basic_with_texture_isMetal,
         SHADER_TECHNIQUE_basic_with_texture_normal,
         SHADER_TECHNIQUE_basic_with_texture_normal_isMetal,
         SHADER_TECHNIQUE_basic_with_texture_refl,
         SHADER_TECHNIQUE_basic_with_texture_refl_isMetal,
         SHADER_TECHNIQUE_basic_with_texture_refl_normal,
         SHADER_TECHNIQUE_basic_with_texture_refl_normal_isMetal,
         SHADER_TECHNIQUE_basic_with_texture_refr,
         SHADER_TECHNIQUE_basic_with_texture_refr_isMetal,
         SHADER_TECHNIQUE_basic_with_texture_refr_normal,
         SHADER_TECHNIQUE_basic_with_texture_refr_normal_isMetal,
         SHADER_TECHNIQUE_basic_with_texture_refr_refl,
         SHADER_TECHNIQUE_basic_with_texture_refr_refl_isMetal,
         SHADER_TECHNIQUE_basic_with_texture_refr_refl_normal,
         SHADER_TECHNIQUE_basic_with_texture_refr_refl_normal_isMetal,
         SHADER_TECHNIQUE_basic_with_texture_at,
         SHADER_TECHNIQUE_basic_with_texture_at_isMetal,
         SHADER_TECHNIQUE_basic_with_texture_at_normal,
         SHADER_TECHNIQUE_basic_with_texture_at_normal_isMetal,
         SHADER_TECHNIQUE_basic_with_texture_at_refl,
         SHADER_TECHNIQUE_basic_with_texture_at_refl_isMetal,
         SHADER_TECHNIQUE_basic_with_texture_at_refl_normal,
         SHADER_TECHNIQUE_basic_with_texture_at_refl_normal_isMetal,
         SHADER_TECHNIQUE_basic_with_texture_at_refr,
         SHADER_TECHNIQUE_basic_with_texture_at_refr_isMetal,
         SHADER_TECHNIQUE_basic_with_texture_at_refr_normal,
         SHADER_TECHNIQUE_basic_with_texture_at_refr_normal_isMetal,
         SHADER_TECHNIQUE_basic_with_texture_at_refr_refl,
         SHADER_TECHNIQUE_basic_with_texture_at_refr_refl_isMetal,
         SHADER_TECHNIQUE_basic_with_texture_at_refr_refl_normal,
         SHADER_TECHNIQUE_basic_with_texture_at_refr_refl_normal_isMetal,
      };
      int idx = (isMetal ? 1 : 0) + (doNormalMapping ? 2 : 0) + (doReflections ? 4 : 0) + (doRefractions ? 8 : 0) + (doAlphaTest ? 16 : 0);
      tech = tech_with_texture[idx];
      break;
   }
   case SHADER_TECHNIQUE_basic_without_texture:
   {
      static ShaderTechniques tech_without_texture[8] = {
         SHADER_TECHNIQUE_basic_without_texture,
         SHADER_TECHNIQUE_basic_without_texture_isMetal,
         SHADER_TECHNIQUE_basic_without_texture_refl,
         SHADER_TECHNIQUE_basic_without_texture_refl_isMetal,
         SHADER_TECHNIQUE_basic_without_texture_refr,
         SHADER_TECHNIQUE_basic_without_texture_refr_isMetal,
         SHADER_TECHNIQUE_basic_without_texture_refr_refl,
         SHADER_TECHNIQUE_basic_without_texture_refr_refl_isMetal,
      };
      int idx = (isMetal ? 1 : 0) + (doReflections ? 2 : 0) + (doRefractions ? 4 : 0);
      tech = tech_without_texture[idx];
      break;
   }
   case SHADER_TECHNIQUE_kickerBoolean: if (isMetal) tech = SHADER_TECHNIQUE_kickerBoolean_isMetal; break;
   case SHADER_TECHNIQUE_light_with_texture: if (isMetal) tech = SHADER_TECHNIQUE_light_with_texture_isMetal; break;
   case SHADER_TECHNIQUE_light_without_texture: if (isMetal) tech = SHADER_TECHNIQUE_light_without_texture_isMetal; break;
   default: assert(false); // Unsupported
   }
   #endif
   SetTechnique(tech);
}

void Shader::SetTechnique(ShaderTechniques technique)
{
   assert(current_shader != this); // Changing the technique of a used shader is not allowed (between Begin/End)
   assert(0 <= technique && technique < SHADER_TECHNIQUE_COUNT);
   #if defined(ENABLE_OPENGL)
   if (m_techniques[technique] == nullptr)
   {
      m_technique = SHADER_TECHNIQUE_INVALID;
      ShowError("Fatal Error: Could not find shader technique " + shaderTechniqueNames[technique].name);
      exit(-1);
   }
   #elif defined(ENABLE_BGFX)
   if (!bgfx::isValid(m_techniques[technique]))
   {
      assert(0);
      m_technique = SHADER_TECHNIQUE_INVALID;
      ShowError("Fatal Error: Could not find shader technique " + shaderTechniqueNames[technique].name);
      exit(-1);
   }
   #endif
   m_technique = technique;
}

void Shader::SetBasic(const Material * const mat, Texture * const pin)
{
   if (pin)
   {
      SetTechniqueMaterial(SHADER_TECHNIQUE_basic_with_texture, *mat, pin->m_alphaTestValue >= 0.f && !pin->m_pdsBuffer->IsOpaque());
      SetTexture(SHADER_tex_base_color, pin); //, SF_TRILINEAR, SA_REPEAT, SA_REPEAT);
      SetAlphaTestValue(pin->m_alphaTestValue);
      SetMaterial(mat, !pin->m_pdsBuffer->IsOpaque());
   }
   else
   {
      SetTechniqueMaterial(SHADER_TECHNIQUE_basic_without_texture, *mat);
      SetMaterial(mat, false);
   }
}

void Shader::ApplyUniform(const ShaderUniforms uniformName)
{
   assert(0 <= uniformName && uniformName < SHADER_UNIFORM_COUNT);
   assert(m_stateOffsets[uniformName] != -1);

   #if defined(ENABLE_BGFX)
   ShaderState* boundState = m_boundState[m_technique];
   bgfx::UniformHandle desc = m_uniformHandles[uniformName];

   #elif defined(ENABLE_OPENGL)
   ShaderState* const __restrict boundState = m_boundState[m_technique];
   // For OpenGL uniform binding state is per technique (i.e. program)
   const UniformDesc& desc = m_techniques[m_technique]->uniform_desc[uniformName];
   assert(desc.location >= 0); // Do not apply to an unused uniform
   if (desc.location < 0) // FIXME remove
      return;

   #elif defined(ENABLE_DX9)
   ShaderState* const __restrict boundState = m_boundState;
   const UniformDesc& desc = m_uniform_desc[uniformName];
   #endif

   void* const src = m_state->m_state + m_stateOffsets[uniformName];
   void* const dst = boundState->m_state + m_stateOffsets[uniformName];
   if (memcmp(dst, src, m_stateSizes[uniformName]) == 0)
   {
      #if defined(ENABLE_BGFX)
      // FIXME BGFX implement uniform caching
      #elif defined(ENABLE_OPENGL)
      if (shaderUniformNames[uniformName].type == SUT_DataBlock)
      {
         glUniformBlockBinding(m_techniques[m_technique]->program, desc.location, 0);
         glBindBufferRange(GL_UNIFORM_BUFFER, 0, desc.blockBuffer, 0, m_stateSizes[uniformName]);
         return;
      }
      else if (shaderUniformNames[uniformName].type != SUT_Sampler)
         return;
      #elif defined(ENABLE_DX9)
      return;
      #endif
   }
   m_renderDevice->m_curParameterChanges++;

   switch (shaderUniformNames[uniformName].type)
   {
   case SUT_DataBlock: // Uniform blocks
      #if defined(ENABLE_BGFX)
      assert(false); // Unsupported for BGFX for the time being
      #elif defined(ENABLE_OPENGL)
      glBindBuffer(GL_UNIFORM_BUFFER, desc.blockBuffer);
      glBufferData(GL_UNIFORM_BUFFER, m_stateSizes[uniformName], src, GL_STREAM_DRAW);
      glUniformBlockBinding(m_techniques[m_technique]->program, desc.location, 0);
      glBindBufferRange(GL_UNIFORM_BUFFER, 0, desc.blockBuffer, 0, m_stateSizes[uniformName]);
      #elif defined(ENABLE_DX9)
      assert(false); // Unsupported on DX9
      #endif
      break;
   case SUT_Bool:
      {
         assert(shaderUniformNames[uniformName].count == 1);
         bool val = *(bool*)src;
         *(bool*)dst = val;
         #if defined(ENABLE_BGFX)
         vec4 v(val ? 1.f : 0.f, 0.f, 0.f, 0.f);
         bgfx::setUniform(desc, &v);
         #elif defined(ENABLE_OPENGL)
         glUniform1i(desc.location, val);
         #elif defined(ENABLE_DX9)
         CHECKD3D(m_shader->SetBool(desc.handle, val));
         #endif
      }
      break;
   case SUT_Int:
      {
         assert(shaderUniformNames[uniformName].count == 1);
         int val = *(int*)src;
         *(int*)dst = val;
         #if defined(ENABLE_BGFX)
         vec4 v((float) val, 0.f, 0.f, 0.f);
         bgfx::setUniform(desc, &v);
         #elif defined(ENABLE_OPENGL)
         glUniform1i(desc.location, val);
         #elif defined(ENABLE_DX9)
         CHECKD3D(m_shader->SetInt(desc.handle, val));
         #endif
      }
      break;
   case SUT_Float:
      {
         assert(shaderUniformNames[uniformName].count == 1);
         float val = *(float*)src;
         *(float*)dst = val;
         #if defined(ENABLE_BGFX)
         vec4 v(val, 0.f, 0.f, 0.f);
         bgfx::setUniform(desc, &v);
         #elif defined(ENABLE_OPENGL)
         glUniform1f(desc.location, val);
         #elif defined(ENABLE_DX9)
         CHECKD3D(m_shader->SetFloat(desc.handle, val));
         #endif
      }
      break;
   case SUT_Float2:
      {
         assert(shaderUniformNames[uniformName].count == 1);
         memcpy(dst, src, m_stateSizes[uniformName]);
         #if defined(ENABLE_BGFX)
         vec4 v(((float*)src)[0], ((float*)src)[1], 0.f, 0.f);
         bgfx::setUniform(desc, &v);
         #elif defined(ENABLE_OPENGL)
         glUniform2fv(desc.location, 1, (const GLfloat*)src);
         #elif defined(ENABLE_DX9)
         CHECKD3D(m_shader->SetVector(desc.handle, (D3DXVECTOR4*)src));
         #endif
         break;
      }
   case SUT_Float3:
      {
         assert(shaderUniformNames[uniformName].count == 1);
         memcpy(dst, src, m_stateSizes[uniformName]);
         #if defined(ENABLE_BGFX)
         vec4 v(((float*)src)[0], ((float*)src)[1], ((float*)src)[2], 0.f);
         bgfx::setUniform(desc, &v);
         #elif defined(ENABLE_OPENGL)
         glUniform3fv(desc.location, 1, (const GLfloat*)src);
         #elif defined(ENABLE_DX9)
         CHECKD3D(m_shader->SetVector(desc.handle, (D3DXVECTOR4*)src));
         #endif
         break;
      }
   case SUT_Float4:
      assert(shaderUniformNames[uniformName].count == 1);
      memcpy(dst, src, m_stateSizes[uniformName]);
      #if defined(ENABLE_BGFX)
      bgfx::setUniform(desc, src);
      #elif defined(ENABLE_OPENGL)
      glUniform4fv(desc.location, 1, (const GLfloat*)src);
      #elif defined(ENABLE_DX9)
      CHECKD3D(m_shader->SetVector(desc.handle, (D3DXVECTOR4*)src));
      #endif
      break;
   case SUT_Float4v:
      memcpy(dst, src, m_stateSizes[uniformName]);
      #if defined(ENABLE_BGFX)
      bgfx::setUniform(desc, src, shaderUniformNames[uniformName].count);
      #elif defined(ENABLE_OPENGL)
      glUniform4fv(desc.location, shaderUniformNames[uniformName].count, (const GLfloat*)src);
      #elif defined(ENABLE_DX9)
      CHECKD3D(m_shader->SetFloatArray(desc.handle, (float*) src, shaderUniformNames[uniformName].count * 4));
      #endif
      break;
   case SUT_Float3x4:
   case SUT_Float4x3:
   case SUT_Float4x4:
      memcpy(dst, src, m_stateSizes[uniformName]);
      #if defined(ENABLE_BGFX)
      bgfx::setUniform(desc, src, shaderUniformNames[uniformName].count);
      #elif defined(ENABLE_OPENGL)
      glUniformMatrix4fv(desc.location, shaderUniformNames[uniformName].count, GL_FALSE, (const GLfloat*)src);
      #elif defined(ENABLE_DX9)
      assert(shaderUniformNames[uniformName].count == 1);
      /*CHECKD3D(*/ m_shader->SetMatrix(desc.handle, (D3DXMATRIX*) src) /*)*/; // leads to invalid calls when setting some of the matrices (as hlsl compiler optimizes some down to less than 4x4)
      #endif
      break;

   case SUT_Sampler:
      {
         #if defined(ENABLE_BGFX)
         Sampler* texel = *(Sampler**)src;
         if (texel == nullptr)
         {
            bgfx::setTexture(shaderUniformNames[uniformName].tex_unit, desc, m_renderDevice->m_nullTexture->GetCoreTexture());
            return;
         }
         bgfx::TextureHandle texHandle = texel->GetCoreTexture();
         if (!bgfx::isValid(texel->GetCoreTexture()))
         {
            bgfx::setTexture(shaderUniformNames[uniformName].tex_unit, desc, m_renderDevice->m_nullTexture->GetCoreTexture());
            return;
         }
         SamplerFilter filter = texel->GetFilter();
         SamplerAddressMode clampu = texel->GetClampU();
         SamplerAddressMode clampv = texel->GetClampV();
         if (filter == SF_UNDEFINED)
         {
            filter = shaderUniformNames[uniformName].default_filter;
            if (filter == SF_UNDEFINED)
               filter = SF_NONE;
         }
         if (clampu == SA_UNDEFINED)
         {
            clampu = shaderUniformNames[uniformName].default_clampu;
            if (clampu == SA_UNDEFINED)
               clampu = SA_CLAMP;
         }
         if (clampv == SA_UNDEFINED)
         {
            clampv = shaderUniformNames[uniformName].default_clampv;
            if (clampv == SA_UNDEFINED)
               clampv = SA_CLAMP;
         }
         uint32_t flags = BGFX_SAMPLER_W_CLAMP;
         switch (filter)
         {
         case SF_NONE:
            flags |= BGFX_SAMPLER_MIN_POINT;
            flags |= BGFX_SAMPLER_MAG_POINT;
            break;
         case SF_POINT:
            flags |= BGFX_SAMPLER_MIN_POINT;
            flags |= BGFX_SAMPLER_MAG_POINT;
            break;
         case SF_BILINEAR:
            /* Default is linear. No flag to set. */
            break;
         case SF_TRILINEAR:
            /* Default is linear. No flag to set. */
            break;
         case SF_ANISOTROPIC:
            flags |= BGFX_SAMPLER_MIN_ANISOTROPIC;
            flags |= BGFX_SAMPLER_MAG_ANISOTROPIC;
            break;
         default:
            break;
         }
         switch (clampu)
         {
         case SA_CLAMP: flags |= BGFX_SAMPLER_U_CLAMP; break;
         case SA_MIRROR: flags |= BGFX_SAMPLER_U_MIRROR; break;
         case SA_REPEAT: /* Default mode, no flag to set */ break;
         default: break;
         }
         switch (clampv)
         {
         case SA_CLAMP: flags |= BGFX_SAMPLER_V_CLAMP; break;
         case SA_MIRROR: flags |= BGFX_SAMPLER_V_MIRROR; break;
         case SA_REPEAT: /* Default mode, no flag to set */ break;
         default: break;
         }
         bgfx::setTexture(shaderUniformNames[uniformName].tex_unit, desc, texHandle, flags);

         #elif defined(ENABLE_OPENGL)
         // DX9 implementation uses preaffected texture units, not samplers, so these can not be used for OpenGL. This would cause some collisions.
         m_renderDevice->m_curParameterChanges--;
         Sampler* const texel = *(Sampler**)src;
         SamplerBinding* tex_unit = nullptr;
         assert(texel != nullptr);
         SamplerFilter filter = texel->GetFilter();
         SamplerAddressMode clampu = texel->GetClampU();
         SamplerAddressMode clampv = texel->GetClampV();
         if (filter == SF_UNDEFINED)
         {
            filter = shaderUniformNames[uniformName].default_filter;
            if (filter == SF_UNDEFINED)
               filter = SF_NONE;
         }
         if (clampu == SA_UNDEFINED)
         {
            clampu = shaderUniformNames[uniformName].default_clampu;
            if (clampu == SA_UNDEFINED)
               clampu = SA_CLAMP;
         }
         if (clampv == SA_UNDEFINED)
         {
            clampv = shaderUniformNames[uniformName].default_clampv;
            if (clampv == SA_UNDEFINED)
               clampv = SA_CLAMP;
         }
         for (auto binding : texel->m_bindings)
         {
            if (binding->filter == filter && binding->clamp_u == clampu && binding->clamp_v == clampv)
            {
               tex_unit = binding;
               break;
            }
         }
         // Setup a texture unit if not already bound to one
         if (tex_unit == nullptr)
         {
            tex_unit = m_renderDevice->m_samplerBindings.back();
            if (tex_unit->sampler != nullptr)
               tex_unit->sampler->m_bindings.erase(tex_unit);
            tex_unit->sampler = texel;
            tex_unit->filter = filter;
            tex_unit->clamp_u = clampu;
            tex_unit->clamp_v = clampv;
            texel->m_bindings.insert(tex_unit);
            glActiveTexture(GL_TEXTURE0 + tex_unit->unit);
            switch (texel->m_type)
            {
            case RT_DEFAULT: glBindTexture(GL_TEXTURE_2D, texel->GetCoreTexture()); break;
            case RT_STEREO: glBindTexture(GL_TEXTURE_2D_ARRAY, texel->GetCoreTexture()); break;
            case RT_CUBEMAP: glBindTexture(GL_TEXTURE_CUBE_MAP, texel->GetCoreTexture()); break;
            default: assert(false);
            }
            m_renderDevice->m_curTextureChanges++;
            m_renderDevice->SetSamplerState(tex_unit->unit, filter, clampu, clampv);
         }
         // Bind the sampler
         if (*(int*)dst != tex_unit->unit)
         {
            glUniform1i(desc.location, tex_unit->unit);
            m_renderDevice->m_curParameterChanges++;
            *(int*)dst = tex_unit->unit;
         }
         // Mark this texture unit as the last used one, and age all the others
         for (int i = tex_unit->use_rank - 1; i >= 0; i--)
         {
            m_renderDevice->m_samplerBindings[i]->use_rank++;
            m_renderDevice->m_samplerBindings[i + 1] = m_renderDevice->m_samplerBindings[i];
         }
         tex_unit->use_rank = 0;
         m_renderDevice->m_samplerBindings[0] = tex_unit;

         #elif defined(ENABLE_DX9)
         // A sampler bind performs 3 things:
         // - bind the texture to a texture stage (done by DirectX effect framework)
         // - adjust the sampling state (filter, wrapping, ...) of the choosen texture stage (partly done by DirectX effect framework which only applies the ones defined in the effect file)
         // - set the shader constant buffer to point to the selected texture stage (done by DirectX effect framework)
         // So, for DirectX, we simply fetch the Texture, DirectX will then use the texture for one or more samplers, applying there default states if any
         // This is not perfect though since we also use sampler state overrides that can break here if not defined correctly
         int unit = desc.sampler;
         assert(0 <= unit && unit < TEXTURESET_STATE_CACHE_SIZE);

         // Bind the texture to the shader
         Sampler* const tex = *(Sampler**)src;
         assert(tex != nullptr);
         IDirect3DTexture9* const bounded = m_boundTexture[unit] ? m_boundTexture[unit]->GetCoreTexture() : nullptr;
         IDirect3DTexture9* const tobound = tex ? tex->GetCoreTexture() : nullptr;
         if (bounded != tobound)
         {
            CHECKD3D(m_shader->SetTexture(desc.tex_handle, tobound));
            m_boundTexture[unit] = tex;
            m_renderDevice->m_curTextureChanges++;
         }

         // Apply the texture sampling states
         if (tex != m_renderDevice->m_nullTexture)
         {
            //CHECKD3D(m_renderDevice->GetCoreDevice()->SetSamplerState(unit, D3DSAMP_SRGBTEXTURE, !tex->IsLinear()));
            SamplerFilter filter = tex->GetFilter();
            SamplerAddressMode clampu = tex->GetClampU();
            SamplerAddressMode clampv = tex->GetClampV();
            if (filter == SF_UNDEFINED)
            {
               filter = shaderUniformNames[uniformName].default_filter;
               if (filter == SF_UNDEFINED) filter = SF_NONE;
            }
            if (clampu == SA_UNDEFINED)
            {
               clampu = shaderUniformNames[uniformName].default_clampu;
               if (clampu == SA_UNDEFINED) clampu = SA_CLAMP;
            }
            if (clampv == SA_UNDEFINED)
            {
               clampv = shaderUniformNames[uniformName].default_clampv;
               if (clampv == SA_UNDEFINED) clampv = SA_CLAMP;
            }
            m_renderDevice->SetSamplerState(unit, filter, clampu, clampv);
         }
         #endif
      }
      break;
   default: assert(false);
   }
}

#if defined(ENABLE_BGFX)
///////////////////////////////////////////////////////////////////////////////
// BGFX specific implementation

bgfx::ProgramHandle Shader::GetCore() const
{
   return (m_renderDevice->GetActiveRenderState().GetRenderState(RenderState::CLIPPLANEENABLE) == RenderState::RS_TRUE) && bgfx::isValid(m_clipPlaneTechniques[m_technique])
      ? m_clipPlaneTechniques[m_technique]
      : m_techniques[m_technique];
}

void Shader::loadProgram(const bgfx::EmbeddedShader* embeddedShaders, ShaderTechniques technique, const char* vsName, const char* fsName, const bool isClipVariant)
{
   assert(!bgfx::isValid(isClipVariant ? m_clipPlaneTechniques[technique] : m_techniques[technique]));
   bgfx::RendererType::Enum type = bgfx::getRendererType();
   bgfx::ShaderHandle vsh = bgfx::createEmbeddedShader(embeddedShaders, type, vsName);
   bgfx::ShaderHandle fsh = bgfx::createEmbeddedShader(embeddedShaders, type, fsName);
   if (!bgfx::isValid(vsh) || !bgfx::isValid(fsh))
   {
      PLOGE << "Failed to setup shader from " << vsName << " / " << fsName;
      assert(false);
      m_hasError = true;
      return;
   }
   bgfx::ProgramHandle ph = bgfx::createProgram(vsh, fsh, true /* destroy shaders when program is destroyed */);
   if (!bgfx::isValid(ph))
   {
      PLOGE << "Failed to build program from " << vsName << " / " << fsName;
      assert(false);
      m_hasError = true;
      return;
   }
   (isClipVariant ? m_clipPlaneTechniques[technique] : m_techniques[technique]) = ph;

   // Create uniforms from informations gathered by BGFX
   if (bgfx::getRendererType() == bgfx::RendererType::Enum::OpenGL || bgfx::getRendererType() == bgfx::RendererType::Enum::OpenGLES)
   {
      // BGFX uses glsl optimizer to parse GLSL but it does not support recent GLSL language versions, in turn not gathering uniform informations for OpenGL...
      m_uniforms[technique] = shaderTechniqueNames[technique].uniforms;
   }
   else
   {
      m_uniforms[technique].clear();
      for (int j = 0; j < 2; j++)
      {
         bgfx::UniformHandle uniforms[SHADER_UNIFORM_COUNT];
         uint16_t n_uniforms = bgfx::getShaderUniforms(j == 0 ? vsh : fsh, uniforms, SHADER_UNIFORM_COUNT);
         for (int i = 0; i < n_uniforms; i++)
         {
            bgfx::UniformInfo info;
            bgfx::getUniformInfo(uniforms[i], info);
            auto uniformIndex = getUniformByName(info.name);
            if (uniformIndex == SHADER_UNIFORM_INVALID)
            {
               PLOGE << "Invalid uniform defined in shader " << (j == 0 ? vsName : fsName) << ": " << info.name;
            }
            else if (std::find(m_uniforms[technique].begin(), m_uniforms[technique].end(), uniformIndex) == m_uniforms[technique].end())
            {
               assert(info.num == shaderUniformNames[uniformIndex].count);
               m_uniforms[technique].push_back(uniformIndex);
            }
         }
      }
      /* Can be used to update the list of used uniforms for OpenGL / OpenGL ES backends
      std::sort(m_uniforms[technique].begin(), m_uniforms[technique].end());
      std::stringstream ss;
      ss << "SHADER_TECHNIQUE(" << GetTechniqueName(technique);
      for (const ShaderUniforms& uniform : m_uniforms[technique])
         ss << ", " << shaderUniformNames[(int)uniform].name;
      ss << "),";
      PLOGD << ss.str();
      ss << "\n";
      OutputDebugString(ss.str().c_str());
      */
   }
}

// Embedded shaders
#include "shaders/bgfx_ball.h"
#include "shaders/bgfx_basic.h"
#include "shaders/bgfx_dmd.h"
#include "shaders/bgfx_flasher.h"
#include "shaders/bgfx_light.h"
#include "shaders/bgfx_postprocess.h"
#include "shaders/bgfx_motionblur.h"
#include "shaders/bgfx_antialiasing.h"
#include "shaders/bgfx_tonemap.h"
#include "shaders/bgfx_blur.h"
#include "shaders/bgfx_stereo.h"

void Shader::Load()
{
   #define BGFX_EMBEDDED_SHADER_ST(a) BGFX_EMBEDDED_SHADER(a), BGFX_EMBEDDED_SHADER(a##_st)
   #define BGFX_EMBEDDED_SHADER_CLIP(a) BGFX_EMBEDDED_SHADER(a##_clip), BGFX_EMBEDDED_SHADER(a##_noclip)
   #define BGFX_EMBEDDED_SHADER_ST_CLIP(a) BGFX_EMBEDDED_SHADER_ST(a##_clip), BGFX_EMBEDDED_SHADER_ST(a##_noclip)
   static const bgfx::EmbeddedShader embeddedShaders[] =
   {
      // Basic material shaders
      BGFX_EMBEDDED_SHADER_ST_CLIP(vs_basic_tex),
      BGFX_EMBEDDED_SHADER_ST_CLIP(vs_basic_notex),
      BGFX_EMBEDDED_SHADER_ST_CLIP(vs_kicker),
      BGFX_EMBEDDED_SHADER_ST_CLIP(vs_classic_light_tex),
      BGFX_EMBEDDED_SHADER_ST_CLIP(vs_classic_light_notex),
      BGFX_EMBEDDED_SHADER_ST_CLIP(fs_basic_tex_noat),
      BGFX_EMBEDDED_SHADER_ST_CLIP(fs_basic_tex_at),
      BGFX_EMBEDDED_SHADER_ST_CLIP(fs_basic_notex_noat),
      BGFX_EMBEDDED_SHADER_ST_CLIP(fs_basic_refl),
      BGFX_EMBEDDED_SHADER_ST_CLIP(fs_decal_tex),
      BGFX_EMBEDDED_SHADER_ST_CLIP(fs_decal_notex),
      BGFX_EMBEDDED_SHADER_ST_CLIP(fs_classic_light_tex),
      BGFX_EMBEDDED_SHADER_ST_CLIP(fs_classic_light_notex),
      BGFX_EMBEDDED_SHADER_ST_CLIP(fs_unshaded_notex),
      BGFX_EMBEDDED_SHADER_ST_CLIP(fs_unshaded_notex_ballshadow),
      BGFX_EMBEDDED_SHADER_ST_CLIP(fs_unshaded_tex),
      BGFX_EMBEDDED_SHADER_ST_CLIP(fs_unshaded_tex_ballshadow),
      BGFX_EMBEDDED_SHADER(vs_vr_mask),
      // Ball shaders
      BGFX_EMBEDDED_SHADER_ST_CLIP(vs_ball),
      BGFX_EMBEDDED_SHADER_ST_CLIP(vs_ball_trail),
      BGFX_EMBEDDED_SHADER_ST_CLIP(fs_ball_equirectangular_nodecal),
      BGFX_EMBEDDED_SHADER_ST_CLIP(fs_ball_equirectangular_decal),
      BGFX_EMBEDDED_SHADER_ST_CLIP(fs_ball_spherical_nodecal),
      BGFX_EMBEDDED_SHADER_ST_CLIP(fs_ball_spherical_decal),
      BGFX_EMBEDDED_SHADER_ST_CLIP(fs_ball_trail),
      BGFX_EMBEDDED_SHADER_ST_CLIP(fs_ball_debug),
      // DMD, Display & Sprite shaders
      BGFX_EMBEDDED_SHADER_ST(vs_dmd_noworld),
      BGFX_EMBEDDED_SHADER_ST_CLIP(vs_dmd_world),
      BGFX_EMBEDDED_SHADER_CLIP(fs_dmd),
      BGFX_EMBEDDED_SHADER_CLIP(fs_display_dmd),
      BGFX_EMBEDDED_SHADER_CLIP(fs_display_seg),
      BGFX_EMBEDDED_SHADER_CLIP(fs_display_crt),
      BGFX_EMBEDDED_SHADER_CLIP(fs_sprite_tex),
      BGFX_EMBEDDED_SHADER_CLIP(fs_sprite_notex),
      // Bulb light shaders
      BGFX_EMBEDDED_SHADER_ST_CLIP(vs_light),
      BGFX_EMBEDDED_SHADER_CLIP(fs_light_noshadow),
      BGFX_EMBEDDED_SHADER_CLIP(fs_light_ballshadow),
      // Flasher shaders
      BGFX_EMBEDDED_SHADER_ST_CLIP(vs_flasher),
      BGFX_EMBEDDED_SHADER_CLIP(fs_flasher),
      // Stereo post-processes
      BGFX_EMBEDDED_SHADER_ST(vs_postprocess),
      BGFX_EMBEDDED_SHADER_ST(fs_pp_stereo_tb),
      BGFX_EMBEDDED_SHADER_ST(fs_pp_stereo_sbs),
      BGFX_EMBEDDED_SHADER_ST(fs_pp_stereo_int),
      BGFX_EMBEDDED_SHADER_ST(fs_pp_stereo_flipped_int),
      BGFX_EMBEDDED_SHADER_ST(fs_pp_stereo_anaglyph_lin_srgb_nodesat),
      BGFX_EMBEDDED_SHADER_ST(fs_pp_stereo_anaglyph_lin_gamma_nodesat),
      BGFX_EMBEDDED_SHADER_ST(fs_pp_stereo_anaglyph_lin_srgb_dyndesat),
      BGFX_EMBEDDED_SHADER_ST(fs_pp_stereo_anaglyph_lin_gamma_dyndesat),
      BGFX_EMBEDDED_SHADER_ST(fs_pp_stereo_anaglyph_deghost),
      // Tonemappers
      BGFX_EMBEDDED_SHADER_ST(fs_pp_tonemap_reinhard_noao_filter_rgb),
      BGFX_EMBEDDED_SHADER_ST(fs_pp_tonemap_reinhard_ao_filter_rgb),
      BGFX_EMBEDDED_SHADER_ST(fs_pp_tonemap_reinhard_noao_nofilter_rgb),
      BGFX_EMBEDDED_SHADER_ST(fs_pp_tonemap_reinhard_ao_nofilter_rgb),
      BGFX_EMBEDDED_SHADER_ST(fs_pp_tonemap_filmic_noao_filter),
      BGFX_EMBEDDED_SHADER_ST(fs_pp_tonemap_filmic_ao_filter),
      BGFX_EMBEDDED_SHADER_ST(fs_pp_tonemap_filmic_noao_nofilter),
      BGFX_EMBEDDED_SHADER_ST(fs_pp_tonemap_filmic_ao_nofilter),
      BGFX_EMBEDDED_SHADER_ST(fs_pp_tonemap_neutral_noao_filter),
      BGFX_EMBEDDED_SHADER_ST(fs_pp_tonemap_neutral_ao_filter),
      BGFX_EMBEDDED_SHADER_ST(fs_pp_tonemap_neutral_noao_nofilter),
      BGFX_EMBEDDED_SHADER_ST(fs_pp_tonemap_neutral_ao_nofilter),
      BGFX_EMBEDDED_SHADER_ST(fs_pp_tonemap_agx_noao_filter),
      BGFX_EMBEDDED_SHADER_ST(fs_pp_tonemap_agx_ao_filter),
      BGFX_EMBEDDED_SHADER_ST(fs_pp_tonemap_agx_noao_nofilter),
      BGFX_EMBEDDED_SHADER_ST(fs_pp_tonemap_agx_ao_nofilter),
      BGFX_EMBEDDED_SHADER_ST(fs_pp_tonemap_agx_punchy_noao_filter),
      BGFX_EMBEDDED_SHADER_ST(fs_pp_tonemap_agx_punchy_ao_filter),
      BGFX_EMBEDDED_SHADER_ST(fs_pp_tonemap_agx_punchy_noao_nofilter),
      BGFX_EMBEDDED_SHADER_ST(fs_pp_tonemap_agx_punchy_ao_nofilter),
      BGFX_EMBEDDED_SHADER_ST(fs_pp_tonemap_wcg_noao_filter),
      BGFX_EMBEDDED_SHADER_ST(fs_pp_tonemap_wcg_ao_filter),
      BGFX_EMBEDDED_SHADER_ST(fs_pp_tonemap_wcg_noao_nofilter),
      BGFX_EMBEDDED_SHADER_ST(fs_pp_tonemap_wcg_ao_nofilter),
      BGFX_EMBEDDED_SHADER_ST(fs_pp_tonemap_reinhard_noao_nofilter_rg),
      BGFX_EMBEDDED_SHADER_ST(fs_pp_tonemap_reinhard_noao_nofilter_gray),
      // Screen Space post-processes
      BGFX_EMBEDDED_SHADER_ST(fs_pp_ao_filter),
      BGFX_EMBEDDED_SHADER_ST(fs_pp_ao_nofilter),
      BGFX_EMBEDDED_SHADER_ST(fs_pp_ao_display),
      BGFX_EMBEDDED_SHADER_ST(fs_pp_ssao),
      BGFX_EMBEDDED_SHADER_ST(fs_pp_bloom),
      BGFX_EMBEDDED_SHADER_ST(fs_pp_mirror),
      BGFX_EMBEDDED_SHADER_ST(fs_pp_copy),
      BGFX_EMBEDDED_SHADER_ST(fs_pp_ssr),
      BGFX_EMBEDDED_SHADER_ST(fs_pp_irradiance),
      // Motion Blur as post-processes
      BGFX_EMBEDDED_SHADER_ST(fs_pp_motionblur),
      // Anti-Aliasing as post-processes
      BGFX_EMBEDDED_SHADER_ST(fs_pp_nfaa),
      BGFX_EMBEDDED_SHADER_ST(fs_pp_dlaa_edge),
      BGFX_EMBEDDED_SHADER_ST(fs_pp_dlaa),
      BGFX_EMBEDDED_SHADER_ST(fs_pp_fxaa1),
      BGFX_EMBEDDED_SHADER_ST(fs_pp_fxaa2),
      BGFX_EMBEDDED_SHADER_ST(fs_pp_fxaa3),
      BGFX_EMBEDDED_SHADER_ST(fs_pp_cas),
      BGFX_EMBEDDED_SHADER_ST(fs_pp_bilateral_cas),
      BGFX_EMBEDDED_SHADER(vs_pp_smaa_edgedetection),
      BGFX_EMBEDDED_SHADER(fs_pp_smaa_edgedetection),
      BGFX_EMBEDDED_SHADER(vs_pp_smaa_blendweightcalculation),
      BGFX_EMBEDDED_SHADER(fs_pp_smaa_blendweightcalculation),
      BGFX_EMBEDDED_SHADER(vs_pp_smaa_neighborhoodblending),
      BGFX_EMBEDDED_SHADER(fs_pp_smaa_neighborhoodblending),
      // Blur shaders
      BGFX_EMBEDDED_SHADER_ST(fs_blur_7_h),
      BGFX_EMBEDDED_SHADER_ST(fs_blur_7_v),
      BGFX_EMBEDDED_SHADER_ST(fs_blur_9_h),
      BGFX_EMBEDDED_SHADER_ST(fs_blur_9_v),
      BGFX_EMBEDDED_SHADER_ST(fs_blur_11_h),
      BGFX_EMBEDDED_SHADER_ST(fs_blur_11_v),
      BGFX_EMBEDDED_SHADER_ST(fs_blur_13_h),
      BGFX_EMBEDDED_SHADER_ST(fs_blur_13_v),
      BGFX_EMBEDDED_SHADER_ST(fs_blur_15_h),
      BGFX_EMBEDDED_SHADER_ST(fs_blur_15_v),
      BGFX_EMBEDDED_SHADER_ST(fs_blur_19_h),
      BGFX_EMBEDDED_SHADER_ST(fs_blur_19_v),
      BGFX_EMBEDDED_SHADER_ST(fs_blur_23_h),
      BGFX_EMBEDDED_SHADER_ST(fs_blur_23_v),
      BGFX_EMBEDDED_SHADER_ST(fs_blur_27_h),
      BGFX_EMBEDDED_SHADER_ST(fs_blur_27_v),
      BGFX_EMBEDDED_SHADER_ST(fs_blur_39_h),
      BGFX_EMBEDDED_SHADER_ST(fs_blur_39_v),
      //
      BGFX_EMBEDDED_SHADER_END()
   };
   #undef BGFX_EMBEDDED_SHADER_ST
   #define STEREO(n) m_isStereo ? n##_st.name : #n
   switch (m_shaderId)
   {
   case BASIC_SHADER:
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_basic_with_texture,       STEREO(vs_basic_tex_noclip),           STEREO(fs_basic_tex_noat_noclip));
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_basic_with_texture_at,    STEREO(vs_basic_tex_noclip),           STEREO(fs_basic_tex_at_noclip));
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_basic_without_texture,    STEREO(vs_basic_notex_noclip),         STEREO(fs_basic_notex_noat_noclip));
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_basic_reflection_only,    STEREO(vs_basic_tex_noclip),           STEREO(fs_basic_refl_noclip));
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_light_with_texture,       STEREO(vs_classic_light_tex_noclip),   STEREO(fs_classic_light_tex_noclip));
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_light_without_texture,    STEREO(vs_classic_light_notex_noclip), STEREO(fs_classic_light_notex_noclip));
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_bg_decal_without_texture, STEREO(vs_basic_notex_noclip),         STEREO(fs_decal_notex_noclip));
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_bg_decal_with_texture,    STEREO(vs_basic_tex_noclip),           STEREO(fs_decal_tex_noclip));
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_kickerBoolean,            STEREO(vs_kicker_noclip),              STEREO(fs_basic_notex_noat_noclip));
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_unshaded_without_texture, STEREO(vs_basic_notex_noclip),         STEREO(fs_unshaded_notex_noclip));
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_unshaded_with_texture,    STEREO(vs_basic_tex_noclip),           STEREO(fs_unshaded_tex_noclip));
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_unshaded_without_texture_shadow, STEREO(vs_basic_notex_noclip),  STEREO(fs_unshaded_notex_ballshadow_noclip));
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_unshaded_with_texture_shadow, STEREO(vs_basic_tex_noclip),       STEREO(fs_unshaded_tex_ballshadow_noclip));
      // Variants with a clipping plane
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_basic_with_texture,       STEREO(vs_basic_tex_clip),           STEREO(fs_basic_tex_noat_clip), true);
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_basic_with_texture_at,    STEREO(vs_basic_tex_clip),           STEREO(fs_basic_tex_at_clip), true);
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_basic_without_texture,    STEREO(vs_basic_notex_clip),         STEREO(fs_basic_notex_noat_clip), true);
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_basic_reflection_only,    STEREO(vs_basic_tex_clip),           STEREO(fs_basic_refl_clip), true);
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_light_with_texture,       STEREO(vs_classic_light_tex_clip),   STEREO(fs_classic_light_tex_clip), true);
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_light_without_texture,    STEREO(vs_classic_light_notex_clip), STEREO(fs_classic_light_notex_clip), true);
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_bg_decal_without_texture, STEREO(vs_basic_notex_clip),         STEREO(fs_decal_notex_clip), true);
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_bg_decal_with_texture,    STEREO(vs_basic_tex_clip),           STEREO(fs_decal_tex_clip), true);
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_kickerBoolean,            STEREO(vs_kicker_clip),              STEREO(fs_basic_notex_noat_clip), true);
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_unshaded_without_texture, STEREO(vs_basic_notex_clip),         STEREO(fs_unshaded_notex_clip), true);
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_unshaded_with_texture,    STEREO(vs_basic_tex_clip),           STEREO(fs_unshaded_tex_clip), true);
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_unshaded_without_texture_shadow, STEREO(vs_basic_notex_clip),  STEREO(fs_unshaded_notex_ballshadow_clip), true);
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_unshaded_with_texture_shadow, STEREO(vs_basic_tex_clip),       STEREO(fs_unshaded_tex_ballshadow_clip), true);
      // VR masking
      if (m_isStereo)
         loadProgram(embeddedShaders, SHADER_TECHNIQUE_vr_mask, "vs_vr_mask", "fs_unshaded_notex_noclip");
      break;
   case BALL_SHADER:
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_RenderBall,                        STEREO(vs_ball_noclip), STEREO(fs_ball_equirectangular_nodecal_noclip));
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_RenderBall_DecalMode,              STEREO(vs_ball_noclip), STEREO(fs_ball_equirectangular_decal_noclip));
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_RenderBall_SphericalMap,           STEREO(vs_ball_noclip), STEREO(fs_ball_spherical_nodecal_noclip));
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_RenderBall_SphericalMap_DecalMode, STEREO(vs_ball_noclip), STEREO(fs_ball_spherical_decal_noclip));
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_RenderBall_Debug,                  STEREO(vs_ball_noclip), STEREO(fs_ball_debug_noclip));
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_RenderBallTrail,                   STEREO(vs_ball_trail_noclip), STEREO(fs_ball_trail_noclip));
      // Variants with a clipping plane
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_RenderBall,                        STEREO(vs_ball_clip), STEREO(fs_ball_equirectangular_nodecal_clip), true);
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_RenderBall_DecalMode,              STEREO(vs_ball_clip), STEREO(fs_ball_equirectangular_decal_clip), true);
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_RenderBall_SphericalMap,           STEREO(vs_ball_clip), STEREO(fs_ball_spherical_nodecal_clip), true);
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_RenderBall_SphericalMap_DecalMode, STEREO(vs_ball_clip), STEREO(fs_ball_spherical_decal_clip), true);
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_RenderBall_Debug,                  STEREO(vs_ball_clip), STEREO(fs_ball_debug_clip), true);
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_RenderBallTrail,                   STEREO(vs_ball_trail_clip), STEREO(fs_ball_trail_clip), true);
      break;
   case DMD_VR_SHADER:
   case DMD_SHADER:
      // basic_DMD_ext and basic_DMD_world_ext are not implemented as they are designed for external DMD capture which is not implemented for BGFX (and expected to be removed at some point in future)
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_basic_DMD,         STEREO(vs_dmd_noworld),      "fs_dmd_noclip");
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_basic_DMD_world,   STEREO(vs_dmd_world_noclip), "fs_dmd_noclip");
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_basic_DMD_world,   STEREO(vs_dmd_world_clip), "fs_dmd_clip", true);

      loadProgram(embeddedShaders, SHADER_TECHNIQUE_basic_noDMD_world, STEREO(vs_dmd_world_clip), "fs_sprite_tex_clip", true);
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_basic_noDMD,       STEREO(vs_dmd_noworld), "fs_sprite_tex_noclip");
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_basic_noDMD_notex, STEREO(vs_dmd_noworld), "fs_sprite_notex_noclip");
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_basic_noDMD_world, STEREO(vs_dmd_world_noclip), "fs_sprite_tex_noclip");

      loadProgram(embeddedShaders, SHADER_TECHNIQUE_display_DMD,             STEREO(vs_dmd_noworld), "fs_display_dmd_noclip");
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_display_Seg,             STEREO(vs_dmd_noworld), "fs_display_seg_noclip");
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_display_CRT,             STEREO(vs_dmd_noworld), "fs_display_crt_noclip");
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_display_DMD_world,       STEREO(vs_dmd_world_noclip), "fs_display_dmd_noclip");
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_display_Seg_world,       STEREO(vs_dmd_world_noclip), "fs_display_seg_noclip");
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_display_CRT_world,       STEREO(vs_dmd_world_noclip), "fs_display_crt_noclip");
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_display_DMD_world,       STEREO(vs_dmd_world_clip), "fs_display_dmd_clip", true);
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_display_Seg_world,       STEREO(vs_dmd_world_clip), "fs_display_seg_clip", true);
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_display_CRT_world,       STEREO(vs_dmd_world_clip), "fs_display_crt_clip", true);
      break;
   //case DMD_VR_SHADER:
      //assert(false);
      break;
   case FLASHER_SHADER:
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_basic_noLight, STEREO(vs_flasher_noclip), "fs_flasher_noclip");
      // Variants with a clipping plane
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_basic_noLight, STEREO(vs_flasher_clip), "fs_flasher_clip", true);
      break;
   case LIGHT_SHADER:
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_bulb_light, STEREO(vs_light_noclip), "fs_light_noshadow_noclip");
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_bulb_light_with_ball_shadows, STEREO(vs_light_noclip), "fs_light_ballshadow_noclip");
      // Variants with a clipping plane
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_bulb_light, STEREO(vs_light_clip), "fs_light_noshadow_clip", true);
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_bulb_light_with_ball_shadows, STEREO(vs_light_clip), "fs_light_ballshadow_clip", true);
      break;
   case STEREO_SHADER:
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_stereo_SBS, STEREO(vs_postprocess), STEREO(fs_pp_stereo_sbs));
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_stereo_TB, STEREO(vs_postprocess), STEREO(fs_pp_stereo_tb));
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_stereo_Int, STEREO(vs_postprocess), STEREO(fs_pp_stereo_int));
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_stereo_Flipped_Int, STEREO(vs_postprocess), STEREO(fs_pp_stereo_flipped_int));
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_Stereo_sRGBAnaglyph, STEREO(vs_postprocess), STEREO(fs_pp_stereo_anaglyph_lin_srgb_nodesat));
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_Stereo_GammaAnaglyph, STEREO(vs_postprocess), STEREO(fs_pp_stereo_anaglyph_lin_gamma_nodesat));
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_Stereo_sRGBDynDesatAnaglyph, STEREO(vs_postprocess), STEREO(fs_pp_stereo_anaglyph_lin_srgb_dyndesat));
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_Stereo_GammaDynDesatAnaglyph, STEREO(vs_postprocess), STEREO(fs_pp_stereo_anaglyph_lin_gamma_dyndesat));
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_Stereo_DeghostAnaglyph, STEREO(vs_postprocess), STEREO(fs_pp_stereo_anaglyph_deghost));
      break;
   case POSTPROCESS_SHADER:
      // Tonemapping / Dither / Apply AO / Color Grade
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_fb_rhtonemap, STEREO(vs_postprocess), STEREO(fs_pp_tonemap_reinhard_noao_filter_rgb));
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_fb_rhtonemap_AO, STEREO(vs_postprocess), STEREO(fs_pp_tonemap_reinhard_ao_filter_rgb));
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_fb_rhtonemap_no_filter, STEREO(vs_postprocess), STEREO(fs_pp_tonemap_reinhard_noao_nofilter_rgb));
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_fb_rhtonemap_AO_no_filter, STEREO(vs_postprocess), STEREO(fs_pp_tonemap_reinhard_ao_nofilter_rgb));
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_fb_fmtonemap, STEREO(vs_postprocess), STEREO(fs_pp_tonemap_filmic_noao_filter));
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_fb_fmtonemap_AO, STEREO(vs_postprocess), STEREO(fs_pp_tonemap_filmic_ao_filter));
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_fb_fmtonemap_no_filter, STEREO(vs_postprocess), STEREO(fs_pp_tonemap_filmic_noao_nofilter));
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_fb_fmtonemap_AO_no_filter, STEREO(vs_postprocess), STEREO(fs_pp_tonemap_filmic_ao_nofilter));
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_fb_nttonemap, STEREO(vs_postprocess), STEREO(fs_pp_tonemap_neutral_noao_filter));
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_fb_nttonemap_AO, STEREO(vs_postprocess), STEREO(fs_pp_tonemap_neutral_ao_filter));
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_fb_nttonemap_no_filter, STEREO(vs_postprocess), STEREO(fs_pp_tonemap_neutral_noao_nofilter));
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_fb_nttonemap_AO_no_filter, STEREO(vs_postprocess), STEREO(fs_pp_tonemap_neutral_ao_nofilter));
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_fb_agxtonemap, STEREO(vs_postprocess), STEREO(fs_pp_tonemap_agx_noao_filter));
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_fb_agxtonemap_AO, STEREO(vs_postprocess), STEREO(fs_pp_tonemap_agx_ao_filter));
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_fb_agxtonemap_no_filter, STEREO(vs_postprocess), STEREO(fs_pp_tonemap_agx_noao_nofilter));
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_fb_agxtonemap_AO_no_filter, STEREO(vs_postprocess), STEREO(fs_pp_tonemap_agx_ao_nofilter));
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_fb_agxptonemap, STEREO(vs_postprocess), STEREO(fs_pp_tonemap_agx_punchy_noao_filter));
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_fb_agxptonemap_AO, STEREO(vs_postprocess), STEREO(fs_pp_tonemap_agx_punchy_ao_filter));
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_fb_agxptonemap_no_filter, STEREO(vs_postprocess), STEREO(fs_pp_tonemap_agx_punchy_noao_nofilter));
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_fb_agxptonemap_AO_no_filter, STEREO(vs_postprocess), STEREO(fs_pp_tonemap_agx_punchy_ao_nofilter));
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_fb_wcgtonemap, STEREO(vs_postprocess), STEREO(fs_pp_tonemap_wcg_noao_filter));
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_fb_wcgtonemap_AO, STEREO(vs_postprocess), STEREO(fs_pp_tonemap_wcg_ao_filter));
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_fb_wcgtonemap_no_filter, STEREO(vs_postprocess), STEREO(fs_pp_tonemap_wcg_noao_nofilter));
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_fb_wcgtonemap_AO_no_filter, STEREO(vs_postprocess), STEREO(fs_pp_tonemap_wcg_ao_nofilter));
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_fb_rhtonemap_no_filterRG, STEREO(vs_postprocess), STEREO(fs_pp_tonemap_reinhard_noao_nofilter_rg));
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_fb_rhtonemap_no_filterR, STEREO(vs_postprocess), STEREO(fs_pp_tonemap_reinhard_noao_nofilter_gray));

      // Ambient Occlusion and misc post process (SSR, Bloom, mirror, ...)
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_AO, STEREO(vs_postprocess), STEREO(fs_pp_ssao));
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_fb_AO, STEREO(vs_postprocess), STEREO(fs_pp_ao_display));
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_fb_AO_static, STEREO(vs_postprocess), STEREO(fs_pp_ao_filter));
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_fb_AO_no_filter_static, STEREO(vs_postprocess), STEREO(fs_pp_ao_nofilter));
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_fb_bloom, STEREO(vs_postprocess), STEREO(fs_pp_bloom));
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_fb_mirror, STEREO(vs_postprocess), STEREO(fs_pp_mirror));
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_fb_copy, STEREO(vs_postprocess), STEREO(fs_pp_copy));
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_SSReflection, STEREO(vs_postprocess), STEREO(fs_pp_ssr));
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_irradiance, STEREO(vs_postprocess), STEREO(fs_pp_irradiance));

      // Postprocessed motion blur
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_fb_motionblur, STEREO(vs_postprocess), STEREO(fs_pp_motionblur));
      
      // Postprocessed antialiasing
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_NFAA, STEREO(vs_postprocess), STEREO(fs_pp_nfaa));
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_DLAA_edge, STEREO(vs_postprocess), STEREO(fs_pp_dlaa_edge));
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_DLAA, STEREO(vs_postprocess), STEREO(fs_pp_dlaa));
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_FXAA1, STEREO(vs_postprocess), STEREO(fs_pp_fxaa1));
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_FXAA2, STEREO(vs_postprocess), STEREO(fs_pp_fxaa2));
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_FXAA3, STEREO(vs_postprocess), STEREO(fs_pp_fxaa3));
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_CAS, STEREO(vs_postprocess), STEREO(fs_pp_cas));
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_BilateralSharp_CAS, STEREO(vs_postprocess), STEREO(fs_pp_bilateral_cas));
      // FIXME add stereo support to SMAA
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_SMAA_ColorEdgeDetection, "vs_pp_smaa_edgedetection", "fs_pp_smaa_edgedetection");
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_SMAA_BlendWeightCalculation, "vs_pp_smaa_blendweightcalculation", "fs_pp_smaa_blendweightcalculation");
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_SMAA_NeighborhoodBlending, "vs_pp_smaa_neighborhoodblending", "fs_pp_smaa_neighborhoodblending");

      // Blur Kernels
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_fb_blur_horiz7x7, STEREO(vs_postprocess), STEREO(fs_blur_7_h));
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_fb_blur_vert7x7, STEREO(vs_postprocess), STEREO(fs_blur_7_v));
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_fb_blur_horiz9x9, STEREO(vs_postprocess), STEREO(fs_blur_9_h));
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_fb_blur_vert9x9, STEREO(vs_postprocess), STEREO(fs_blur_9_v));
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_fb_blur_horiz11x11, STEREO(vs_postprocess), STEREO(fs_blur_11_h));
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_fb_blur_vert11x11, STEREO(vs_postprocess), STEREO(fs_blur_11_v));
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_fb_blur_horiz13x13, STEREO(vs_postprocess), STEREO(fs_blur_13_h));
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_fb_blur_vert13x13, STEREO(vs_postprocess), STEREO(fs_blur_13_v));
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_fb_blur_horiz15x15, STEREO(vs_postprocess), STEREO(fs_blur_15_h));
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_fb_blur_vert15x15, STEREO(vs_postprocess), STEREO(fs_blur_15_v));
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_fb_blur_horiz19x19, STEREO(vs_postprocess), STEREO(fs_blur_19_h));
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_fb_blur_vert19x19, STEREO(vs_postprocess), STEREO(fs_blur_19_v));
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_fb_blur_horiz23x23, STEREO(vs_postprocess), STEREO(fs_blur_23_h));
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_fb_blur_vert23x23, STEREO(vs_postprocess), STEREO(fs_blur_23_v));
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_fb_blur_horiz27x27, STEREO(vs_postprocess), STEREO(fs_blur_27_h));
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_fb_blur_vert27x27, STEREO(vs_postprocess), STEREO(fs_blur_27_v));
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_fb_blur_horiz39x39, STEREO(vs_postprocess), STEREO(fs_blur_39_h));
      loadProgram(embeddedShaders, SHADER_TECHNIQUE_fb_blur_vert39x39, STEREO(vs_postprocess), STEREO(fs_blur_39_v));
      break;
   }
   #undef STEREO
}


#elif defined(ENABLE_OPENGL)
///////////////////////////////////////////////////////////////////////////////
// OpenGL specific implementation

bool Shader::UseGeometryShader() const
{
   // TODO remove geometry shader, and only support layered rendering on driver supporting ARB_shader_viewport_layer_array (all GPU starting GTX950+),
   // the performance impact should be positive for normal rendering and VR/stereo (older GPU are not really able to render in VR/Stereo)
   return m_isStereo;
}

//parse a file. Is called recursively for includes
bool Shader::parseFile(const string& fileNameRoot, const string& fileName, int level, robin_hood::unordered_map<string, string> &values, const string& parentMode) {
   if (level > 16) {//Can be increased, but looks very much like an infinite recursion.
      PLOGE << "Reached more than 16 includes while trying to include " << fileName << " Aborting...";
      return false;
   }
   if (level > 8) {
      PLOGW << "Reached include level " << level << " while trying to include " << fileName << " Check for recursion and try to avoid includes with includes.";
   }
   string currentMode = parentMode;
   robin_hood::unordered_map<string, string>::iterator currentElemIt = values.find(parentMode);
   string currentElement = (currentElemIt != values.end()) ? currentElemIt->second : string();
   std::ifstream glfxFile;
   glfxFile.open(m_shaderPath + fileName, std::ifstream::in);
   if (glfxFile.is_open())
   {
      string line;
      size_t linenumber = 0;
      while (std::getline(glfxFile, line))
      {
         linenumber++;
         if (line.compare(0, 4, "////") == 0) {
            string newMode = line.substr(4, line.length() - 4);
            if (newMode == "DEFINES") {
               currentElement.append("#define GLSL\n\n"s);
               if (UseGeometryShader())
                  currentElement.append("#define USE_GEOMETRY_SHADER 1\n"s);
               else
                  currentElement.append("#define USE_GEOMETRY_SHADER 0\n"s);
               currentElement.append(m_isStereo ? "#define N_EYES 2\n"s : "#define N_EYES 1\n"s);
            } else if (newMode != currentMode) {
               values[currentMode] = currentElement;
               currentElemIt = values.find(newMode);
               currentElement = (currentElemIt != values.end()) ? currentElemIt->second : string();
               currentMode = newMode;
            }
         }
         else if (line.compare(0, 9, "#include ") == 0) {
            const size_t start = line.find('"', 8);
            const size_t end = line.find('"', start + 1);
            values[currentMode] = currentElement;
            if ((start == string::npos) || (end == string::npos) || (end <= start) || !parseFile(fileNameRoot, line.substr(start + 1, end - start - 1), level + 1, values, currentMode)) {
               PLOGE << fileName << '(' << linenumber << "):" << line << " failed.";
            }
            currentElement = values[currentMode];
         }
         else {
            currentElement.append(line).append(1,'\n');
         }
      }
      values[currentMode] = currentElement;
      glfxFile.close();
   }
   else {
      PLOGE << fileName << " not found.";
      return false;
   }
   return true;
}

//compile and link shader. Also write the created shader files
Shader::ShaderTechnique* Shader::compileGLShader(const ShaderTechniques technique, const string& fileNameRoot, const string& shaderCodeName, const string& vertex, const string& geometry, const string& fragment)
{
   bool success = true;
   ShaderTechnique* shader = nullptr;
   GLuint geometryShader = 0;
   GLchar* geometrySource = nullptr;
   GLuint fragmentShader = 0;
   GLchar* fragmentSource = nullptr;

   //Vertex Shader
   GLchar* vertexSource = new GLchar[vertex.length() + 1];
   memcpy((void*)vertexSource, vertex.c_str(), vertex.length());
   vertexSource[vertex.length()] = 0;

   GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
   glShaderSource(vertexShader, 1, &vertexSource, nullptr);
   glCompileShader(vertexShader);

   int result;
   glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &result);
   if (result == FALSE)
   {
      GLint maxLength;
      glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &maxLength);
      char* errorText = (char *)malloc(maxLength);

      glGetShaderInfoLog(vertexShader, maxLength, &maxLength, errorText);
      PLOGE << shaderCodeName << ": Vertex Shader compilation failed with: " << errorText;
      string e = "Fatal Error: Vertex Shader compilation of " + fileNameRoot + ':' + shaderCodeName + " failed!\n\n" + errorText;
      free(errorText);
      ReportError(e.c_str(), -1, __FILE__, __LINE__);
      success = false;

#ifdef __STANDALONE__
      PLOGE << "vertex:";
      for (const auto& line : add_line_numbers(vertexSource)) {
         PLOGE << line;
      }
#endif

   }
#ifndef __OPENGLES__
   //Geometry Shader
   if (success && geometry.length()>0 && UseGeometryShader()) {
      geometrySource = new GLchar[geometry.length() + 1];
      memcpy((void*)geometrySource, geometry.c_str(), geometry.length());
      geometrySource[geometry.length()] = 0;

      geometryShader = glCreateShader(GL_GEOMETRY_SHADER);
      glShaderSource(geometryShader, 1, &geometrySource, nullptr);
      glCompileShader(geometryShader);

      glGetShaderiv(geometryShader, GL_COMPILE_STATUS, &result);
      if (result == FALSE)
      {
         GLint maxLength;
         glGetShaderiv(geometryShader, GL_INFO_LOG_LENGTH, &maxLength);
         char* errorText = (char *)malloc(maxLength);

         glGetShaderInfoLog(geometryShader, maxLength, &maxLength, errorText);
         PLOGE << shaderCodeName << ": Geometry Shader compilation failed with: " << errorText;
         string e = "Fatal Error: Geometry Shader compilation of " + fileNameRoot + ':' + shaderCodeName + " failed!\n\n" + errorText;
         ReportError(e.c_str(), -1, __FILE__, __LINE__);
         free(errorText);
         success = false;

#ifdef __STANDALONE__
         PLOGE << "geometry:";
         for (const auto& line : add_line_numbers(geometrySource)) {
            PLOGE << line;
         }
#endif
      }
   }
#endif
   //Fragment Shader
   if (success) {
      fragmentSource = new GLchar[fragment.length() + 1];
      memcpy((void*)fragmentSource, fragment.c_str(), fragment.length());
      fragmentSource[fragment.length()] = 0;

      fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
      glShaderSource(fragmentShader, 1, &fragmentSource, nullptr);
      glCompileShader(fragmentShader);

      glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result);
      if (result == FALSE)
      {
         GLint maxLength;
         glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &maxLength);
         char* errorText = (char *)malloc(maxLength);

         glGetShaderInfoLog(fragmentShader, maxLength, &maxLength, errorText);
         PLOGE << shaderCodeName << ": Fragment Shader compilation failed with: " << errorText;
         string e = "Fatal Error: Fragment Shader compilation of " + fileNameRoot + ':' + shaderCodeName + " failed!\n\n" + errorText;
         ReportError(e.c_str(), -1, __FILE__, __LINE__);
         free(errorText);
         success = false;

#ifdef __STANDALONE__
         PLOGE << "fragment:";
         for (const auto& line : add_line_numbers(fragmentSource)) {
            PLOGE << line;
         }
#endif

      }
   }

   GLuint shaderprogram = 0;
   if (success) {
      shaderprogram = glCreateProgram();

      glAttachShader(shaderprogram, vertexShader);
      if (geometryShader>0) glAttachShader(shaderprogram, geometryShader);
      glAttachShader(shaderprogram, fragmentShader);

      glLinkProgram(shaderprogram);

      glDetachShader(shaderprogram, vertexShader);
      if (geometryShader > 0)
         glDetachShader(shaderprogram, geometryShader);
      glDetachShader(shaderprogram, fragmentShader);

      glGetProgramiv(shaderprogram, GL_LINK_STATUS, (int *)&result);
      if (result == GL_FALSE)
      {
         GLint maxLength;
         glGetProgramiv(shaderprogram, GL_INFO_LOG_LENGTH, &maxLength);

         /* The maxLength includes the NULL character */
         char* errorText = (char *)malloc(maxLength);

         /* Notice that glGetProgramInfoLog, not glGetShaderInfoLog. */
         glGetProgramInfoLog(shaderprogram, maxLength, &maxLength, errorText);
         PLOGE << shaderCodeName << ": Linking Shader failed with: " << errorText;
         ReportError(errorText, -1, __FILE__, __LINE__);
         free(errorText);
         success = false;

#ifdef __STANDALONE__
         PLOGE << "vertex:";
         for (const auto& line : add_line_numbers(vertexSource)) {
            PLOGE << line;
         }
#ifndef __OPENGLES__
         PLOGE << "geometry:";
         for (const auto& line : add_line_numbers(vertexSource)) {
            PLOGE << line;
         }
#endif
         PLOGE << "fragment:";
         for (const auto& line : add_line_numbers(fragmentSource)) {
            PLOGE << line;
         }
#endif

      }
   }

#ifndef __OPENGLES__
   if (GLAD_GL_VERSION_4_3)
   {
      string vs_name = shaderCodeName + ".VS";
      string gs_name = shaderCodeName + ".GS";
      string fs_name = shaderCodeName + ".FS";
      if (shaderprogram > 0)
         glObjectLabel(GL_PROGRAM, shaderprogram, (GLsizei) shaderCodeName.length(), shaderCodeName.c_str());
      if (vertexShader > 0)
         glObjectLabel(GL_SHADER, vertexShader, (GLsizei) vs_name.length(), vs_name.c_str());
      if (geometryShader > 0) 
         glObjectLabel(GL_SHADER, geometryShader, (GLsizei) gs_name.length(), gs_name.c_str());
      if (fragmentShader > 0)
         glObjectLabel(GL_SHADER, fragmentShader, (GLsizei) fs_name.length(), fs_name.c_str());
   }
#endif

   if ((WRITE_SHADER_FILES == 2) || ((WRITE_SHADER_FILES == 1) && !success))
   {
      std::ofstream shaderCode;
      const string szPath = m_shaderPath + "log" + PATH_SEPARATOR_CHAR + shaderCodeName;
      shaderCode.open(szPath + ".vert");
      shaderCode << vertex;
      shaderCode.close();
      shaderCode.open(szPath + ".geom");
      shaderCode << geometry;
      shaderCode.close();
      shaderCode.open(szPath + ".frag");
      shaderCode << fragment;
      shaderCode.close();
   }
   if (vertexShader)
      glDeleteShader(vertexShader);
   if (geometryShader)
      glDeleteShader(geometryShader);
   if (fragmentShader)
      glDeleteShader(fragmentShader);
   delete [] fragmentSource;
   delete [] geometrySource;
   delete [] vertexSource;

   if (success) {
      shader = new ShaderTechnique { -1, shaderCodeName };
      shader->program = shaderprogram;
      for (int i = 0; i < SHADER_UNIFORM_COUNT; ++i)
         shader->uniform_desc[i].location = -1;

      int count = 0;
      glGetProgramiv(shaderprogram, GL_ACTIVE_UNIFORMS, &count);
      char uniformName[256];
      for (int i = 0;i < count;++i) {
         GLenum type;
         GLint size;
         GLsizei length;
         glGetActiveUniform(shader->program, (GLuint)i, sizeof(uniformName), &length, &size, &type, uniformName);
         GLint location = glGetUniformLocation(shader->program, uniformName);
         if (location >= 0 && size > 0) {
            // hack for packedLights, but works for all arrays
            for (int i2 = 0; i2 < length; i2++)
            {
               if (uniformName[i2] == '[') {
                  uniformName[i2] = '\0';
                  break;
               }
            }
            auto uniformIndex = getUniformByName(uniformName);
            if (uniformIndex < SHADER_UNIFORM_COUNT)
            {
               m_uniforms[technique].push_back(uniformIndex);
               const auto& uniform = shaderUniformNames[uniformIndex];
               assert(uniform.type != SUT_Bool || type == GL_BOOL);
               assert(uniform.type != SUT_Int || type == GL_INT);
               assert(uniform.type != SUT_Float || type == GL_FLOAT);
               assert(uniform.type != SUT_Float2 || type == GL_FLOAT_VEC2);
               assert(uniform.type != SUT_Float3 || type == GL_FLOAT_VEC3);
               assert(uniform.type != SUT_Float4 || type == GL_FLOAT_VEC4);
               assert(uniform.type != SUT_Float4v || type == GL_FLOAT_VEC4);
               assert(uniform.type != SUT_Float3x4); // Unused so unimplemented
               assert(uniform.type != SUT_Float4x3 || type == GL_FLOAT_MAT4); // FIXME this should be GL_FLOAT_MAT4x3 or GL_FLOAT_MAT3x4 => fix orientation uniform in gl shader
               assert(uniform.type != SUT_Float4x4 || type == GL_FLOAT_MAT4);
               assert(uniform.type != SUT_DataBlock); // Unused so unimplemented
               assert(uniform.type != SUT_Sampler || type == GL_SAMPLER_2D || type == GL_SAMPLER_2D_ARRAY);
               assert(uniform.count == size);
               shader->uniform_desc[uniformIndex].uniform = uniform;
               shader->uniform_desc[uniformIndex].location = location;
            }
         }
      }

      glGetProgramiv(shaderprogram, GL_ACTIVE_UNIFORM_BLOCKS, &count);
      for (int i = 0;i < count;++i) {
         int size;
         int length;
         glGetActiveUniformBlockName(shader->program, (GLuint)i, 256, &length, uniformName);
         glGetActiveUniformBlockiv(shader->program, (GLuint)i, GL_UNIFORM_BLOCK_DATA_SIZE, &size);
         GLint location = glGetUniformBlockIndex(shader->program, uniformName);
         if (location >= 0 && size>0) {
            //hack for packedLights, but works for all arrays - I don't need it for uniform blocks now and I'm not sure if it makes any sense, but maybe someone else in the future?
            for (int i2 = 0;i2 < length;i2++) {
               if (uniformName[i2] == '[') {
                  uniformName[i2] = '\0';
                  break;
               }
            }
            auto uniformIndex = getUniformByName(uniformName);
            if (uniformIndex < SHADER_UNIFORM_COUNT)
            {
               const auto& uniform = shaderUniformNames[uniformIndex];
               assert(uniform.type == ShaderUniformType::SUT_DataBlock);
               assert(uniform.count == size);
               shader->uniform_desc[uniformIndex].uniform = uniform;
               shader->uniform_desc[uniformIndex].location = location;
               glGenBuffers(1, &shader->uniform_desc[uniformIndex].blockBuffer);
               m_uniforms[technique].push_back(uniformIndex);
            }
         }
      }
   }
   return shader;
}

//Check if technique is valid and replace %PARAMi% with the values in the function header
string Shader::analyzeFunction(const string& shaderCodeName, const string& _technique, const string& functionName, const robin_hood::unordered_map<string, string> &values) {
   const size_t start = functionName.find('(');
   const size_t end = functionName.find(')');
   if ((start == string::npos) || (end == string::npos) || (start > end)) {
      PLOGW << "Invalid technique: " << _technique;
      return string();
   }
   const robin_hood::unordered_map<string, string>::const_iterator it = values.find(functionName.substr(0, start));
   string functionCode = (it != values.end()) ? it->second : string();
   if (end > start + 1) {
      std::stringstream params(functionName.substr(start + 1, end - start - 1));
      string param;
      int paramID = 0;
      while (std::getline(params, param, ',')) {
         string token = "%PARAM" + std::to_string(paramID) + '%';
         std::size_t pos = 0;
         while ((pos = functionCode.find(token, pos)) != string::npos) {
            functionCode.replace(pos, token.length(), param);
            pos += param.length();
         }
         paramID++;
      }
   }
   return functionCode;
}

string Shader::PreprocessGLShader(const string& shaderCode) {
   std::istringstream iss(shaderCode);
   string header;
   string extensions;
   string code;

   for (string line; std::getline(iss, line); )
   {
      if (line.compare(0, 9, "#version ") == 0) {
         #if defined(__OPENGLES__)
            header += "#version 300 es\n";
            header += "#define SHADER_GLES30\n";
         #elif defined(__APPLE__)
            header += "#version 410\n";
            header += "#define SHADER_GL410\n";
         #else
            header += line + '\n';
         #endif
         #ifdef __STANDALONE__
            header += "#define SHADER_STANDALONE\n";
         #endif
      }
      else if (line.compare(0, 11, "#extension ") == 0)
         extensions += line + '\n';
      else
         code += line + '\n';
   }

   return header + extensions + code;
}

void Shader::Load()
{
   switch (m_shaderId)
   {
   case BASIC_SHADER: Load("BasicShader.glfx"s); break;
   case BALL_SHADER: Load("BallShader.glfx"s); break;
   case DMD_SHADER: Load("DMDShader.glfx"s); break;
   case DMD_VR_SHADER: Load("DMDShaderVR.glfx"s); break;
   case FLASHER_SHADER: Load("FlasherShader.glfx"s); break;
   case LIGHT_SHADER: Load("LightShader.glfx"s); break;
   case STEREO_SHADER: Load("StereoShader.glfx"s); break;
   #ifndef __OPENGLES__
   case POSTPROCESS_SHADER: Load("FBShader.glfx"s); Load("SMAA.glfx"s); break;
   #else
   case POSTPROCESS_SHADER: Load("FBShader.glfx"s); break;
   #endif
   }
}

void Shader::Load(const std::string& name)
{
   m_shaderCodeName = name;
   m_shaderPath = g_pvp->m_szMyPath
      + ("shaders-" + std::to_string(VP_VERSION_MAJOR) + '.' + std::to_string(VP_VERSION_MINOR) + '.' + std::to_string(VP_VERSION_REV) + PATH_SEPARATOR_CHAR);
   PLOGI << "Parsing file " << name;
   robin_hood::unordered_map<string, string> values;
   const bool parsing = parseFile(m_shaderCodeName, m_shaderCodeName, 0, values, "GLOBAL"s);
   if (!parsing) {
      m_hasError = true;
      PLOGE << "Parsing failed";
      string e = "Fatal Error: Shader parsing of " + m_shaderCodeName + " failed!";
      ReportError(e.c_str(), -1, __FILE__, __LINE__);
      return;
   }
   robin_hood::unordered_map<string, string>::iterator it = values.find("GLOBAL"s);
   string global = (it != values.end()) ? it->second : string();

   it = values.find("VERTEX"s);
   string vertex = global;
   vertex.append((it != values.end()) ? it->second : string());

   it = values.find("GEOMETRY"s);
   string geometry = global;
   geometry.append((it != values.end()) ? it->second : string());

   it = values.find("FRAGMENT"s);
   string fragment = global;
   fragment.append((it != values.end()) ? it->second : string());

   it = values.find("TECHNIQUES"s);
   std::stringstream techniques((it != values.end()) ? it->second : string());
   if (techniques)
   {
      string _technique;
      int tecCount = 0;
      while (std::getline(techniques, _technique, '\n')) {//Parse Technique e.g. basic_with_texture:P0:vs_main():gs_optional_main():ps_main_texture()
         if ((_technique.length() > 0) && (_technique.compare(0, 2, "//") != 0))//Skip empty lines and comments
         {
            std::stringstream elements(_technique);
            int elem = 0;
            string element[5];
            //Split :
            while ((elem < 5) && std::getline(elements, element[elem], ':')) {
               elem++;
            }
            if (elem < 4) {
               continue;
            }
            ShaderTechniques technique = getTechniqueByName(element[0]);
            if (technique == SHADER_TECHNIQUE_INVALID)
            {
               m_hasError = true;
               PLOGI << "Unexpected technique skipped: " << element[0];
            }
            else
            {
               //PLOGI << "Compiling technique: " << shaderTechniqueNames[technique].name;
               string vertexShaderCode = vertex;
               vertexShaderCode.append("\n//"s).append(_technique).append("\n//"s).append(element[2]).append(1,'\n');
               vertexShaderCode.append(analyzeFunction(m_shaderCodeName, _technique, element[2], values)).append(1,'\0');
               vertexShaderCode = PreprocessGLShader(vertexShaderCode);
               string geometryShaderCode;
               if (elem == 5 && element[3].length() > 0)
               {
                  geometryShaderCode = geometry;
                  geometryShaderCode.append("\n//").append(_technique).append("\n//").append(element[3]).append(1,'\n');
                  geometryShaderCode.append(analyzeFunction(m_shaderCodeName, _technique, element[3], values)).append(1,'\0');
               }
               geometryShaderCode = PreprocessGLShader(geometryShaderCode);
               string fragmentShaderCode = fragment;
               fragmentShaderCode.append("\n//").append(_technique).append("\n//").append(element[elem - 1]).append(1,'\n');
               fragmentShaderCode.append(analyzeFunction(m_shaderCodeName, _technique, element[elem - 1], values)).append(1,'\0');
               fragmentShaderCode = PreprocessGLShader(fragmentShaderCode);
               ShaderTechnique* build = compileGLShader(technique, m_shaderCodeName, element[0] /*.append(1,'_').append(element[1])*/, vertexShaderCode, geometryShaderCode, fragmentShaderCode);
               if (build != nullptr)
               {
                  m_techniques[technique] = build;
                  tecCount++;
               }
               else
               {
                  m_hasError = true;
                  string e = "Fatal Error: Compilation failed for technique " + shaderTechniqueNames[technique].name + " of " + m_shaderCodeName + '!';
                  ReportError(e.c_str(), -1, __FILE__, __LINE__);
                  return;
               }
            }
         }
      }
      PLOGI << "Compiled successfully " << std::to_string(tecCount) << " shaders.";
   }
   else {
      m_hasError = true;
      PLOGE << "No techniques found.";
      string e = "Fatal Error: No shader techniques found in " + m_shaderCodeName + '!';
      ReportError(e.c_str(), -1, __FILE__, __LINE__);
      return;
   }
}


#elif defined(ENABLE_DX9)
///////////////////////////////////////////////////////////////////////////////
// DirectX 9 specific implementation

#include "shaders/hlsl_basic.h"
#include "shaders/hlsl_dmd.h"
#include "shaders/hlsl_postprocess.h"
#include "shaders/hlsl_flasher.h"
#include "shaders/hlsl_light.h"
#include "shaders/hlsl_stereo.h"
#include "shaders/hlsl_ball.h"

void Shader::Load()
{
   const BYTE* code;
   unsigned int codeSize;
   switch (m_shaderId)
   {
   case BASIC_SHADER: m_shaderCodeName = "BasicShader.hlsl"s; code = g_basicShaderCode; codeSize = sizeof(g_basicShaderCode); break;
   case BALL_SHADER: m_shaderCodeName = "BallShader.hlsl"s; code = g_ballShaderCode; codeSize = sizeof(g_ballShaderCode); break;
   case DMD_SHADER: m_shaderCodeName = "DMDShader.hlsl"s; code = g_dmdShaderCode; codeSize = sizeof(g_dmdShaderCode); break;
   case DMD_VR_SHADER: assert(false); break;
   case FLASHER_SHADER: m_shaderCodeName = "FlasherShader.hlsl"s; code = g_flasherShaderCode; codeSize = sizeof(g_flasherShaderCode); break;
   case LIGHT_SHADER: m_shaderCodeName = "LightShader.hlsl"s; code = g_lightShaderCode; codeSize = sizeof(g_lightShaderCode); break;
   case STEREO_SHADER: m_shaderCodeName = "StereoShader.hlsl"s; code = g_stereoShaderCode; codeSize = sizeof(g_stereoShaderCode); break;
   case POSTPROCESS_SHADER: m_shaderCodeName = "FBShader.hlsl"s; code = g_FBShaderCode; codeSize = sizeof(g_FBShaderCode); break;
   }
   LPD3DXBUFFER pBufferErrors;
   constexpr DWORD dwShaderFlags = 0; //D3DXSHADER_SKIPVALIDATION // these do not have a measurable effect so far (also if used in the offline fxc step): D3DXSHADER_PARTIALPRECISION, D3DXSHADER_PREFER_FLOW_CONTROL/D3DXSHADER_AVOID_FLOW_CONTROL
   HRESULT hr = D3DXCreateEffect(m_renderDevice->GetCoreDevice(), code, codeSize, nullptr, nullptr, dwShaderFlags, nullptr, &m_shader, &pBufferErrors);
   if (FAILED(hr))
   {
      if (pBufferErrors)
      {
         const LPVOID pCompileErrors = pBufferErrors->GetBufferPointer();
         g_pvp->MessageBox((const char*)pCompileErrors, "Compile Error", MB_OK | MB_ICONEXCLAMATION);
      }
      else
         g_pvp->MessageBox("Unknown Error", "Compile Error", MB_OK | MB_ICONEXCLAMATION);
      m_hasError = true;
      return;
   }

   // Collect the list of uniforms and their information (handle, type,...)
   D3DXEFFECT_DESC effect_desc;
   m_shader->GetDesc(&effect_desc);
   ShaderUniforms textureMask[TEXTURESET_STATE_CACHE_SIZE];
   for (int i = 0; i < TEXTURESET_STATE_CACHE_SIZE; i++)
      textureMask[i] = SHADER_UNIFORM_INVALID;
   for (UINT i = 0; i < effect_desc.Parameters; i++)
   {
      D3DXPARAMETER_DESC param_desc;
      D3DXHANDLE parameter = m_shader->GetParameter(NULL, i);
      m_shader->GetParameterDesc(parameter, &param_desc);
      ShaderUniformType type = ShaderUniformType::SUT_INVALID;
      int count = 1;
      if (param_desc.Class == D3DXPC_SCALAR)
      {
         if (param_desc.Type == D3DXPT_BOOL)
            type = ShaderUniformType::SUT_Bool;
         else if (param_desc.Type == D3DXPT_INT)
            type = ShaderUniformType::SUT_Int;
         else if (param_desc.Type == D3DXPT_FLOAT)
            type = ShaderUniformType::SUT_Float;
      }
      else if (param_desc.Class == D3DXPC_VECTOR && param_desc.Type == D3DXPT_FLOAT)
      {
         if (param_desc.Elements > 0 && param_desc.Columns == 4)
         {
            type = ShaderUniformType::SUT_Float4v;
            count = param_desc.Elements;
         }
         else if (param_desc.Elements == 0 && param_desc.Columns == 4)
            type = ShaderUniformType::SUT_Float4;
         else if (param_desc.Elements == 0 && param_desc.Columns == 3)
            type = ShaderUniformType::SUT_Float3;
         else if (param_desc.Elements == 0 && param_desc.Columns == 2)
            type = ShaderUniformType::SUT_Float2;
      }
      else if (param_desc.Class == D3DXPC_MATRIX_ROWS && param_desc.Type == D3DXPT_FLOAT)
      {
         if (param_desc.Rows == 4 && param_desc.Columns == 4)
            type = ShaderUniformType::SUT_Float4x4;
         else if (param_desc.Rows == 3 && param_desc.Columns == 4)
            type = ShaderUniformType::SUT_Float3x4;
         else if (param_desc.Rows == 4 && param_desc.Columns == 3)
            type = ShaderUniformType::SUT_Float4x3;
      }
      else if (param_desc.Class == D3DXPC_OBJECT && param_desc.Type == D3DXPT_SAMPLER2D)
      {
         type = ShaderUniformType::SUT_Sampler;
      }
      else if (param_desc.Class == D3DXPC_OBJECT && (param_desc.Type == D3DXPT_TEXTURE || param_desc.Type == D3DXPT_TEXTURE2D))
      {
         // We track the samplers (since they hold the TEXUNIT semantic), not the texture, so just skip them
         continue;
      }
      if (type == ShaderUniformType::SUT_INVALID)
      {
         PLOGE << "Unsupported uniform type for: " << param_desc.Name;
         continue;
      }
      ShaderUniforms uniformIndex = getUniformByName(param_desc.Name);
      if (uniformIndex == SHADER_UNIFORM_INVALID)
      {
         PLOGE << "Missing uniform: " << param_desc.Name;
         continue;
      }
      else
      {
         const auto& uniform = shaderUniformNames[uniformIndex];
         assert(uniform.type == type);
         assert(uniform.count == count);
         m_uniform_desc[uniformIndex].uniform = uniform;
         m_uniform_desc[uniformIndex].handle = parameter;
         m_uniform_desc[uniformIndex].tex_handle = nullptr;
         m_uniform_desc[uniformIndex].sampler = -1;
         bool addToUniformList = true;
         if (type == ShaderUniformType::SUT_Sampler)
         {
            const string name = "Texture"s.append(std::to_string(shaderUniformNames[uniformIndex].tex_unit));
            m_uniform_desc[uniformIndex].tex_handle = m_shader->GetParameterByName(NULL, name.c_str());
            #if __cplusplus < 202002L
            if (param_desc.Semantic != nullptr && std::string(param_desc.Semantic).rfind("TEXUNIT"s, 0) == 0)
            #else
            if (param_desc.Semantic != nullptr && std::string(param_desc.Semantic).starts_with("TEXUNIT"s))
            #endif
            {
               const int unit = shaderUniformNames[uniformIndex].tex_unit;
               assert(unit == atoi(param_desc.Semantic + 7));
               m_uniform_desc[uniformIndex].sampler = unit;
               // DirectX effect framework manages samplers for us and we only perform texture binding, so just keep
               // Since we only manages the texture state and not the sampler ones, only add one of the samplers bound to a given texture unit to avoid useless calls
               if (textureMask[unit] == SHADER_UNIFORM_INVALID)
                  textureMask[unit] = uniformIndex;
               else
                  addToUniformList = false;
               m_uniform_desc[uniformIndex].tex_alias = textureMask[unit];
            }
         }
         if (addToUniformList)
            // TODO we do not filter on technique for DX9. Not a big problem, but not that clean either (all uniforms are applied for all techniques)
            for (int j = 0; j < SHADER_TECHNIQUE_COUNT; j++)
               m_uniforms[j].push_back(uniformIndex);
      }
   }
}

#endif
