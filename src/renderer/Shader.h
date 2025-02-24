// license:GPLv3+

#pragma once

#if defined(ENABLE_BGFX)
#ifdef __STANDALONE__
#pragma push_macro("_far")
#pragma push_macro("_WIN64")
#undef _far
#undef _WIN64
#endif
#include "bx/readerwriter.h"
#include "bgfx/embedded_shader.h"
#ifdef __STANDALONE__
#pragma pop_macro("_WIN64")
#pragma pop_macro("_far")
#endif

#elif defined(ENABLE_OPENGL)
#ifdef _DEBUG
// Writes all compile/parse errors/warnings to the application log. (0=never, 1=only errors, 2=warnings, 3=info)
#define DEBUG_LEVEL_LOG 1
// Writes all shaders that are compiled to the application log (0=never, 1=only if compile failed, 2=always)
#define WRITE_SHADER_FILES 1
#else 
#define DEBUG_LEVEL_LOG 0
#define WRITE_SHADER_FILES 1
#endif
#endif

// Attempt to speed up STL which is very CPU costly, maybe we should look into using EASTL instead? http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2007/n2271.html https://github.com/electronicarts/EASTL
#ifndef ENABLE_BGFX
#define _SECURE_SCL 0
#define _HAS_ITERATOR_DEBUGGING 0
#endif

#if defined(ENABLE_BGFX) || defined(__OPENGLES__)
#define FLT_MIN_VALUE 0.00006103515625f
#else
#define FLT_MIN_VALUE 0.0000001f
#endif

#include <string>

// Declaration of all available techniques (shader program)
// When changed, this list must also be copied unchanged to Shader.cpp (for its implementation)
#define SHADER_TECHNIQUE(name, ...) SHADER_TECHNIQUE_##name
enum ShaderTechniques
{
   SHADER_TECHNIQUE(RenderBall, SHADER_matProj, SHADER_matWorldViewProj, SHADER_matView, SHADER_matWorldView, SHADER_matWorldViewInverse, SHADER_ballLightEmission, SHADER_ballLightPos,
      SHADER_Roughness_WrapL_Edge_Thickness, SHADER_cBase_Alpha, SHADER_fDisableLighting_top_below, SHADER_fenvEmissionScale_TexWidth, SHADER_cAmbient_LightRange, SHADER_tex_diffuse_env,
      SHADER_orientation, SHADER_invTableRes_reflection, SHADER_w_h_disableLighting, SHADER_tex_ball_color, SHADER_tex_ball_playfield, SHADER_tex_ball_decal),
   SHADER_TECHNIQUE(RenderBall_DecalMode, SHADER_matProj, SHADER_matWorldViewProj, SHADER_matView, SHADER_matWorldView, SHADER_matWorldViewInverse, SHADER_ballLightEmission,
      SHADER_ballLightPos, SHADER_Roughness_WrapL_Edge_Thickness, SHADER_cBase_Alpha, SHADER_fDisableLighting_top_below, SHADER_fenvEmissionScale_TexWidth, SHADER_cAmbient_LightRange,
      SHADER_tex_diffuse_env, SHADER_orientation, SHADER_invTableRes_reflection, SHADER_w_h_disableLighting, SHADER_tex_ball_color, SHADER_tex_ball_playfield, SHADER_tex_ball_decal),
   SHADER_TECHNIQUE(RenderBall_SphericalMap, SHADER_matProj, SHADER_matWorldViewProj, SHADER_matView, SHADER_matWorldView, SHADER_matWorldViewInverse, SHADER_ballLightEmission,
      SHADER_ballLightPos, SHADER_Roughness_WrapL_Edge_Thickness, SHADER_cBase_Alpha, SHADER_fDisableLighting_top_below, SHADER_fenvEmissionScale_TexWidth, SHADER_cAmbient_LightRange,
      SHADER_tex_diffuse_env, SHADER_orientation, SHADER_invTableRes_reflection, SHADER_w_h_disableLighting, SHADER_tex_ball_color, SHADER_tex_ball_playfield, SHADER_tex_ball_decal),
   SHADER_TECHNIQUE(RenderBall_SphericalMap_DecalMode, SHADER_matProj, SHADER_matWorldViewProj, SHADER_matView, SHADER_matWorldView, SHADER_matWorldViewInverse, SHADER_ballLightEmission,
      SHADER_ballLightPos, SHADER_Roughness_WrapL_Edge_Thickness, SHADER_cBase_Alpha, SHADER_fDisableLighting_top_below, SHADER_fenvEmissionScale_TexWidth, SHADER_cAmbient_LightRange,
      SHADER_tex_diffuse_env, SHADER_orientation, SHADER_invTableRes_reflection, SHADER_w_h_disableLighting, SHADER_tex_ball_color, SHADER_tex_ball_playfield, SHADER_tex_ball_decal),
   SHADER_TECHNIQUE(RenderBall_Debug, SHADER_matWorldViewProj, SHADER_matWorldView, SHADER_matWorldViewInverse, SHADER_orientation),
   SHADER_TECHNIQUE(RenderBallTrail, SHADER_matWorldViewProj, SHADER_matWorldView, SHADER_matWorldViewInverse, SHADER_cBase_Alpha, SHADER_fenvEmissionScale_TexWidth, SHADER_orientation,
      SHADER_w_h_disableLighting, SHADER_tex_ball_color),
   // OpenGL only has the first variant. DX9 needs all of them due to shader compiler limitation
   SHADER_TECHNIQUE(basic_with_texture, SHADER_matProj, SHADER_matWorldViewProj, SHADER_matWorld, SHADER_matView, SHADER_matWorldView, SHADER_matWorldViewInverseTranspose,
      SHADER_lightCenter_doShadow, SHADER_balls, SHADER_staticColor_Alpha, SHADER_w_h_height, SHADER_basicLightEmission, SHADER_basicLightPos, SHADER_Roughness_WrapL_Edge_Thickness,
      SHADER_cBase_Alpha, SHADER_fDisableLighting_top_below, SHADER_fenvEmissionScale_TexWidth, SHADER_cAmbient_LightRange, SHADER_tex_env, SHADER_tex_diffuse_env,
      SHADER_cClearcoat_EdgeAlpha, SHADER_cGlossy_ImageLerp, SHADER_u_basic_shade_mode, SHADER_refractionTint_thickness, SHADER_mirrorNormal_factor, SHADER_objectSpaceNormalMap,
      SHADER_tex_base_color, SHADER_tex_base_transmission, SHADER_tex_base_normalmap, SHADER_tex_reflection, SHADER_tex_refraction, SHADER_tex_probe_depth),
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
      SHADER_objectSpaceNormalMap, SHADER_tex_base_color, SHADER_tex_base_transmission, SHADER_tex_base_normalmap, SHADER_tex_reflection, SHADER_tex_refraction, SHADER_tex_probe_depth),
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
      SHADER_tex_reflection, SHADER_tex_refraction, SHADER_tex_probe_depth),
   SHADER_TECHNIQUE(basic_without_texture_isMetal),
   SHADER_TECHNIQUE(basic_without_texture_refl),
   SHADER_TECHNIQUE(basic_without_texture_refl_isMetal),
   SHADER_TECHNIQUE(basic_without_texture_refr),
   SHADER_TECHNIQUE(basic_without_texture_refr_isMetal),
   SHADER_TECHNIQUE(basic_without_texture_refr_refl),
   SHADER_TECHNIQUE(basic_without_texture_refr_refl_isMetal),

   // Unshaded
   SHADER_TECHNIQUE(unshaded_without_texture, SHADER_matWorldViewProj, SHADER_matWorld, SHADER_matWorldView, SHADER_matWorldViewInverseTranspose, SHADER_staticColor_Alpha),
   SHADER_TECHNIQUE(
      unshaded_with_texture, SHADER_matWorldViewProj, SHADER_matWorld, SHADER_matWorldView, SHADER_matWorldViewInverseTranspose, SHADER_staticColor_Alpha, SHADER_tex_base_color),
   SHADER_TECHNIQUE(unshaded_without_texture_shadow, SHADER_matWorldViewProj, SHADER_matWorld, SHADER_matWorldView, SHADER_matWorldViewInverseTranspose, SHADER_lightCenter_doShadow,
      SHADER_balls, SHADER_staticColor_Alpha),
   SHADER_TECHNIQUE(unshaded_with_texture_shadow, SHADER_matWorldViewProj, SHADER_matWorld, SHADER_matWorldView, SHADER_matWorldViewInverseTranspose, SHADER_lightCenter_doShadow,
      SHADER_balls, SHADER_staticColor_Alpha, SHADER_tex_base_color),

   SHADER_TECHNIQUE(basic_reflection_only, SHADER_matWorldViewProj, SHADER_matWorld, SHADER_matWorldView, SHADER_matWorldViewInverseTranspose, SHADER_staticColor_Alpha, SHADER_w_h_height,
      SHADER_mirrorNormal_factor, SHADER_tex_reflection),

   SHADER_TECHNIQUE(vr_mask, SHADER_matWorldViewProj),

   SHADER_TECHNIQUE(bg_decal_without_texture, SHADER_matWorldViewProj, SHADER_matWorld, SHADER_matWorldView, SHADER_matWorldViewInverseTranspose, SHADER_cBase_Alpha),
   SHADER_TECHNIQUE(bg_decal_with_texture, SHADER_alphaTestValue, SHADER_matWorldViewProj, SHADER_matWorld, SHADER_matWorldView, SHADER_matWorldViewInverseTranspose, SHADER_cBase_Alpha,
      SHADER_tex_base_color),

   SHADER_TECHNIQUE(kickerBoolean, SHADER_matProj, SHADER_matWorldViewProj, SHADER_matWorld, SHADER_matView, SHADER_matWorldView, SHADER_matWorldViewInverseTranspose,
      SHADER_lightCenter_doShadow, SHADER_balls, SHADER_staticColor_Alpha, SHADER_w_h_height, SHADER_basicLightEmission, SHADER_basicLightPos, SHADER_Roughness_WrapL_Edge_Thickness,
      SHADER_cBase_Alpha, SHADER_fDisableLighting_top_below, SHADER_fenvEmissionScale_TexWidth, SHADER_cAmbient_LightRange, SHADER_tex_env, SHADER_tex_diffuse_env,
      SHADER_cClearcoat_EdgeAlpha, SHADER_cGlossy_ImageLerp, SHADER_u_basic_shade_mode, SHADER_refractionTint_thickness, SHADER_mirrorNormal_factor, SHADER_tex_base_transmission,
      SHADER_tex_reflection, SHADER_tex_refraction, SHADER_tex_probe_depth),
   SHADER_TECHNIQUE(kickerBoolean_isMetal, SHADER_matProj, SHADER_matWorldViewProj, SHADER_matWorld, SHADER_matView, SHADER_matWorldView, SHADER_matWorldViewInverseTranspose,
      SHADER_lightCenter_doShadow, SHADER_balls, SHADER_staticColor_Alpha, SHADER_w_h_height, SHADER_basicLightEmission, SHADER_basicLightPos, SHADER_Roughness_WrapL_Edge_Thickness,
      SHADER_cBase_Alpha, SHADER_fDisableLighting_top_below, SHADER_fenvEmissionScale_TexWidth, SHADER_cAmbient_LightRange, SHADER_tex_env, SHADER_tex_diffuse_env,
      SHADER_cClearcoat_EdgeAlpha, SHADER_cGlossy_ImageLerp, SHADER_u_basic_shade_mode, SHADER_refractionTint_thickness, SHADER_mirrorNormal_factor, SHADER_tex_base_transmission,
      SHADER_tex_reflection, SHADER_tex_refraction, SHADER_tex_probe_depth),

   SHADER_TECHNIQUE(light_with_texture, SHADER_matWorldViewProj, SHADER_matWorld, SHADER_matView, SHADER_matWorldView, SHADER_matWorldViewInverseTranspose, SHADER_basicLightEmission,
      SHADER_basicLightPos, SHADER_Roughness_WrapL_Edge_Thickness, SHADER_cBase_Alpha, SHADER_fDisableLighting_top_below, SHADER_fenvEmissionScale_TexWidth, SHADER_cAmbient_LightRange,
      SHADER_tex_env, SHADER_tex_diffuse_env, SHADER_cClearcoat_EdgeAlpha, SHADER_cGlossy_ImageLerp, SHADER_u_basic_shade_mode, SHADER_lightCenter_maxRange, SHADER_lightColor2_falloff_power,
      SHADER_lightColor_intensity, SHADER_lightingOff, SHADER_tex_light_color),
   SHADER_TECHNIQUE(light_without_texture, SHADER_matWorldViewProj, SHADER_matWorld, SHADER_matView, SHADER_matWorldView, SHADER_matWorldViewInverseTranspose, SHADER_basicLightEmission,
      SHADER_basicLightPos, SHADER_Roughness_WrapL_Edge_Thickness, SHADER_cBase_Alpha, SHADER_fDisableLighting_top_below, SHADER_fenvEmissionScale_TexWidth, SHADER_cAmbient_LightRange,
      SHADER_tex_env, SHADER_tex_diffuse_env, SHADER_cClearcoat_EdgeAlpha, SHADER_cGlossy_ImageLerp, SHADER_u_basic_shade_mode, SHADER_lightCenter_maxRange, SHADER_lightColor2_falloff_power,
      SHADER_lightColor_intensity, SHADER_lightingOff),
   SHADER_TECHNIQUE(light_with_texture_isMetal, SHADER_matWorldViewProj, SHADER_matWorld, SHADER_matView, SHADER_matWorldView, SHADER_matWorldViewInverseTranspose, SHADER_basicLightEmission,
      SHADER_basicLightPos, SHADER_Roughness_WrapL_Edge_Thickness, SHADER_cBase_Alpha, SHADER_fDisableLighting_top_below, SHADER_fenvEmissionScale_TexWidth, SHADER_cAmbient_LightRange,
      SHADER_tex_env, SHADER_tex_diffuse_env, SHADER_cClearcoat_EdgeAlpha, SHADER_cGlossy_ImageLerp, SHADER_u_basic_shade_mode, SHADER_lightCenter_maxRange, SHADER_lightColor2_falloff_power,
      SHADER_lightColor_intensity, SHADER_lightingOff, SHADER_tex_light_color),
   SHADER_TECHNIQUE(light_without_texture_isMetal, SHADER_matWorldViewProj, SHADER_matWorld, SHADER_matView, SHADER_matWorldView, SHADER_matWorldViewInverseTranspose,
      SHADER_basicLightEmission, SHADER_basicLightPos, SHADER_Roughness_WrapL_Edge_Thickness, SHADER_cBase_Alpha, SHADER_fDisableLighting_top_below, SHADER_fenvEmissionScale_TexWidth,
      SHADER_cAmbient_LightRange, SHADER_tex_env, SHADER_tex_diffuse_env, SHADER_cClearcoat_EdgeAlpha, SHADER_cGlossy_ImageLerp, SHADER_u_basic_shade_mode, SHADER_lightCenter_maxRange,
      SHADER_lightColor2_falloff_power, SHADER_lightColor_intensity, SHADER_lightingOff),

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
      SHADER_amount_blend_modulate_vs_add_flasherMode, SHADER_tex_flasher_A, SHADER_tex_flasher_B),

   SHADER_TECHNIQUE(bulb_light, SHADER_matWorldViewProj, SHADER_blend_modulate_vs_add, SHADER_lightCenter_maxRange, SHADER_lightColor2_falloff_power, SHADER_lightColor_intensity),
   SHADER_TECHNIQUE(bulb_light_with_ball_shadows, SHADER_matWorldViewProj, SHADER_balls, SHADER_blend_modulate_vs_add, SHADER_lightCenter_maxRange, SHADER_lightColor2_falloff_power,
      SHADER_lightColor_intensity),

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

   SHADER_TECHNIQUE_COUNT,
   SHADER_TECHNIQUE_INVALID
};
#undef SHADER_TECHNIQUE

enum ShaderUniformType
{
   SUT_Bool,
   SUT_Int,
   SUT_Float,
   SUT_Float2,
   SUT_Float3,
   SUT_Float4,
   SUT_Float4v,
   SUT_Float3x4,
   SUT_Float4x3,
   SUT_Float4x4,
   SUT_DataBlock,
   SUT_Sampler,
   SUT_INVALID
};

// Declaration of all uniforms and samplers used in the shaders
// When changed, this list must also be copied unchanged to Shader.cpp (for its implementation)
// Samplers defines how to sample a texture. For DX9, they are defined in the effect files, only the texture reference is set through the API.
// Otherwise, the sampler states can be directly overriden through DX9Device->SetSamplerState (per tex unit), being carefull that the effect
// framework will also apply the ones defined in the effect file during Technique->Begin call (so either don't define them, or reapply).
#define SHADER_UNIFORM(type, name, count) SHADER_##name
#define SHADER_SAMPLER(name, tex_name, default_clampu, default_clampv, default_filter) SHADER_##name
enum ShaderUniforms
{
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

   SHADER_UNIFORM_COUNT,
   SHADER_UNIFORM_INVALID
};
#undef SHADER_UNIFORM
#undef SHADER_SAMPLER

// When changed, this list must also be copied unchanged to Shader.cpp (for its implementation)
#define SHADER_ATTRIBUTE(name, shader_name) SHADER_ATTRIBUTE_##name
enum ShaderAttributes
{
   SHADER_ATTRIBUTE(POS, vPosition),
   SHADER_ATTRIBUTE(NORM, vNormal),
   SHADER_ATTRIBUTE(TC, tc),
   SHADER_ATTRIBUTE(TEX, tex0),
   SHADER_ATTRIBUTE_COUNT,
   SHADER_ATTRIBUTE_INVALID
};
#undef SHADER_ATTRIBUTE

class Shader final
{
public:
   enum ShaderId { BALL_SHADER, BASIC_SHADER, DMD_SHADER, DMD_VR_SHADER, FLASHER_SHADER, POSTPROCESS_SHADER, LIGHT_SHADER, STEREO_SHADER };
   Shader(RenderDevice* renderDevice, const ShaderId id, const bool isStereo);
   ~Shader();

   void Begin();
   void End();

   bool HasError() const { return m_hasError; }
   static Shader* GetCurrentShader();
   static string GetTechniqueName(ShaderTechniques technique);
   void SetTechnique(const ShaderTechniques technique);
   void SetTechniqueMaterial(ShaderTechniques technique, const Material& mat, const bool doAlphaTest = false, const bool doNormalMapping = false, const bool doReflection = false, const bool doRefraction = false);
   void SetBasic(const Material * const mat, Texture * const pin);
   ShaderTechniques GetCurrentTechnique() const { return m_technique; }
   static void SetDefaultSamplerFilter(const ShaderUniforms sampler, const SamplerFilter sf);
   void UnbindSampler(Sampler* sampler);

   void SetMaterial(const Material * const mat, const bool has_alpha = true);
   void SetAlphaTestValue(const float value);
   void SetFlasherData(const vec4& c1, const vec4& c2);
   void SetLightColorIntensity(const vec4& color);
   void SetLightColor2FalloffPower(const vec4& color);
   void SetLightData(const vec4& color);
   void SetLightImageBackglassMode(const bool imageMode, const bool backglassMode);

   //

   bool HasUniform(const ShaderUniforms uniformName) const { return m_stateOffsets[uniformName] != -1; }
   void SetFloat(const ShaderUniforms uniformName, const float f) { m_state->SetFloat(uniformName, f); }
   void SetMatrix(const ShaderUniforms uniformName, const float* const pMatrix, const unsigned int count = 1) { m_state->SetMatrix(uniformName, pMatrix, count); }
   void SetInt(const ShaderUniforms uniformName, const int i) { m_state->SetInt(uniformName, i); }
   void SetBool(const ShaderUniforms uniformName, const bool b) { m_state->SetBool(uniformName, b); }
   void SetUniformBlock(const ShaderUniforms uniformName, const float* const pMatrix) { m_state->SetUniformBlock(uniformName, pMatrix); }
   #if defined(ENABLE_DX9)
   void SetMatrix(const ShaderUniforms uniformName, const D3DXMATRIX* const pMatrix, const unsigned int count = 1) { SetMatrix(uniformName, &(pMatrix->m[0][0]), count); }
   #endif
   void SetMatrix(const ShaderUniforms uniformName, const Matrix3D* const pMatrix, const unsigned int count = 1) { SetMatrix(uniformName, &(pMatrix->m[0][0]), count); }
   void SetVector(const ShaderUniforms uniformName, const vec4* const pVector) { m_state->SetVector(uniformName, pVector); }
   void SetVector(const ShaderUniforms uniformName, const float x, const float y, const float z, const float w) { const vec4 v(x, y, z, w); m_state->SetVector(uniformName, &v); }
   void SetFloat4v(const ShaderUniforms uniformName, const vec4* const pData, const unsigned int count) { m_state->SetVector(uniformName, pData, count); }
   void SetTexture(const ShaderUniforms uniformName, const Sampler* const sampler) { m_state->SetTexture(uniformName, sampler); }
   void SetTextureNull(const ShaderUniforms uniformName);
   void SetTexture(const ShaderUniforms uniformName, Texture* const texel, const SamplerFilter filter = SF_UNDEFINED, const SamplerAddressMode clampU = SA_UNDEFINED, const SamplerAddressMode clampV = SA_UNDEFINED, const bool force_linear_rgb = false)
   {
      SetTexture(uniformName, texel->m_pdsBuffer, filter, clampU, clampV, force_linear_rgb);
   }
   void SetTexture(const ShaderUniforms uniformName, BaseTexture* const texel, const SamplerFilter filter = SF_UNDEFINED, const SamplerAddressMode clampU = SA_UNDEFINED, const SamplerAddressMode clampV = SA_UNDEFINED, const bool force_linear_rgb = false);

   class ShaderState
   {
   public:
      ShaderState(Shader* shader, bool isLowPrecision)
         : m_shader(shader)
         , m_state(new BYTE[shader->m_stateSize])
         , m_stateSize(shader->m_stateSize)
         , m_useLowPrecision(isLowPrecision)
      {
      }
      ~ShaderState() { delete[] m_state; }
      void Reset(Shader* shader) { assert(shader->m_stateSize <= m_stateSize); m_shader = shader; }
      void CopyTo(const bool copyTo, ShaderState* const other, const ShaderTechniques technique = SHADER_TECHNIQUE_INVALID)
      {
         assert(other->m_shader == m_shader);
         if (copyTo)
            memcpy(other->m_state, m_state, m_shader->m_stateSize);
         else
            memcpy(m_state, other->m_state, m_shader->m_stateSize);
      }
      void CopyTo(const bool copyTo, ShaderState* const other, const ShaderUniforms uniformName)
      {
         assert(other->m_shader == m_shader);
         assert(0 <= uniformName && uniformName < SHADER_UNIFORM_COUNT);
         assert(m_shader->m_stateOffsets[uniformName] != -1);
         if (copyTo)
            memcpy(other->m_state + m_shader->m_stateOffsets[uniformName], m_state + m_shader->m_stateOffsets[uniformName], m_shader->m_stateSizes[uniformName]);
         else
            memcpy(m_state + m_shader->m_stateOffsets[uniformName], other->m_state + m_shader->m_stateOffsets[uniformName], m_shader->m_stateSizes[uniformName]);
      }
      void SetBool(const ShaderUniforms uniformName, const bool b)
      {
         assert(GetCurrentShader() == nullptr);
         assert(0 <= uniformName && uniformName < SHADER_UNIFORM_COUNT);
         assert(m_shader->m_stateOffsets[uniformName] != -1);
         assert(shaderUniformNames[uniformName].type == SUT_Bool);
         assert(shaderUniformNames[uniformName].count == 1);
         *(bool*)(m_state + m_shader->m_stateOffsets[uniformName]) = b;
      }
      void SetInt(const ShaderUniforms uniformName, const int i)
      {
         assert(GetCurrentShader() == nullptr);
         assert(0 <= uniformName && uniformName < SHADER_UNIFORM_COUNT);
         if (uniformName == SHADER_layer && m_shader->m_stateOffsets[uniformName] == -1)
            return; // layer uniform may be stripped out since it is only used for stereo
         assert(m_shader->m_stateOffsets[uniformName] != -1);
         assert(shaderUniformNames[uniformName].type == SUT_Int);
         assert(shaderUniformNames[uniformName].count == 1);
         *(int*)(m_state + m_shader->m_stateOffsets[uniformName]) = i;
      }
      void SetFloat(const ShaderUniforms uniformName, const float f)
      {
         assert(GetCurrentShader() == nullptr);
         assert(0 <= uniformName && uniformName < SHADER_UNIFORM_COUNT);
         assert(m_shader->m_stateOffsets[uniformName] != -1);
         assert(shaderUniformNames[uniformName].type == SUT_Float);
         assert(shaderUniformNames[uniformName].count == 1);
         if (m_useLowPrecision)
            *(float*)(m_state + m_shader->m_stateOffsets[uniformName]) = (f > 0 && f < FLT_MIN_VALUE) ? FLT_MIN_VALUE : (f < 0 && f > -FLT_MIN_VALUE) ? -FLT_MIN_VALUE : f;
         else
            *(float*)(m_state + m_shader->m_stateOffsets[uniformName]) = f;
      }
      float GetFloat(const ShaderUniforms uniformName) const
      {
         assert(GetCurrentShader() == nullptr);
         assert(0 <= uniformName && uniformName < SHADER_UNIFORM_COUNT);
         assert(m_shader->m_stateOffsets[uniformName] != -1);
         assert(shaderUniformNames[uniformName].type == SUT_Float);
         assert(shaderUniformNames[uniformName].count == 1);
         return *(float*)(m_state + m_shader->m_stateOffsets[uniformName]);
      }
      void SetVector(const ShaderUniforms uniformName, const vec4* const pData, const unsigned int count = 1)
      {
         assert(GetCurrentShader() == nullptr);
         assert(0 <= uniformName && uniformName < SHADER_UNIFORM_COUNT);
         assert(m_shader->m_stateOffsets[uniformName] != -1);
         assert(shaderUniformNames[uniformName].type == SUT_Float2 || shaderUniformNames[uniformName].type == SUT_Float3 || shaderUniformNames[uniformName].type == SUT_Float4 || shaderUniformNames[uniformName].type == SUT_Float4v);
         assert(shaderUniformNames[uniformName].count == count);
         const int n = shaderUniformNames[uniformName].type == SUT_Float2 ? 2 : shaderUniformNames[uniformName].type == SUT_Float3 ? 3 : 4;
         if (m_useLowPrecision)
         {
             for (unsigned int i = 0; i < count; i++)
             {
                 const vec4* const p = pData + i;
                 vec4* const s = (vec4*)(m_state + m_shader->m_stateOffsets[uniformName]) + i;
                 if (p->x > 0 && p->x < FLT_MIN_VALUE) s->x = FLT_MIN_VALUE;
                 else if (p->x < 0 && p->x > -FLT_MIN_VALUE) s->x = -FLT_MIN_VALUE;
                 else s->x = p->x;
                 if (p->y > 0 && p->y < FLT_MIN_VALUE) s->y = FLT_MIN_VALUE;
                 else if (p->y < 0 && p->y > -FLT_MIN_VALUE) s->y = -FLT_MIN_VALUE;
                 else s->y = p->y;
                 if (n > 2) {
                    if (p->z > 0 && p->z < FLT_MIN_VALUE) s->z = FLT_MIN_VALUE;
                    else if (p->z < 0 && p->z > -FLT_MIN_VALUE) s->z = -FLT_MIN_VALUE;
                    else s->z = p->z;
                 }
                 if (n > 3) {
                    if (p->w > 0 && p->w < FLT_MIN_VALUE) s->w = FLT_MIN_VALUE;
                    else if (p->w < 0 && p->w > -FLT_MIN_VALUE) s->w = -FLT_MIN_VALUE;
                    else s->w = p->w;
                 }
             }
             }
         else
         {
            memcpy(m_state + m_shader->m_stateOffsets[uniformName], pData, count * n * sizeof(float));
         }
      }
      vec4 GetVector(const ShaderUniforms uniformName) const
      {
         assert(GetCurrentShader() == nullptr);
         assert(0 <= uniformName && uniformName < SHADER_UNIFORM_COUNT);
         assert(m_shader->m_stateOffsets[uniformName] != -1);
         assert(shaderUniformNames[uniformName].type == SUT_Float2 || shaderUniformNames[uniformName].type == SUT_Float3 || shaderUniformNames[uniformName].type == SUT_Float4 || shaderUniformNames[uniformName].type == SUT_Float4v);
         const int n = shaderUniformNames[uniformName].type == SUT_Float2 ? 2 : shaderUniformNames[uniformName].type == SUT_Float3 ? 3 : 4;
         const vec4 result { 
            ((float*)(m_state + m_shader->m_stateOffsets[uniformName]))[0],
            ((float*)(m_state + m_shader->m_stateOffsets[uniformName]))[1],
            n > 2 ? ((float*)(m_state + m_shader->m_stateOffsets[uniformName]))[2] : 0.f,
            n > 3 ? ((float*)(m_state + m_shader->m_stateOffsets[uniformName]))[3] : 0.f };
         return result;
      }
      void SetMatrix(const ShaderUniforms uniformName, const float* const pMatrix, const unsigned int count = 1)
      {
         assert(GetCurrentShader() == nullptr);
         assert(0 <= uniformName && uniformName < SHADER_UNIFORM_COUNT);
         assert(m_shader->m_stateOffsets[uniformName] != -1);
         assert(shaderUniformNames[uniformName].type == SUT_Float3x4 || shaderUniformNames[uniformName].type == SUT_Float4x3 || shaderUniformNames[uniformName].type == SUT_Float4x4);
         assert(count == shaderUniformNames[uniformName].count);
         if (m_useLowPrecision)
         {
            for (unsigned int i = 0; i < count * 16; i++)
            {
               const float* const p = pMatrix + i;
               float* const s = (float*)(m_state + m_shader->m_stateOffsets[uniformName]) + i;
               if (*p > 0 && *p < FLT_MIN_VALUE)
                  *s = FLT_MIN_VALUE;
               else if (*p < 0 && *p > -FLT_MIN_VALUE)
                  *s = -FLT_MIN_VALUE;
               else
                  *s = *p;
            }
         }
         else
         {
            memcpy(m_state + m_shader->m_stateOffsets[uniformName], pMatrix, count * 16 * sizeof(float));
         }
      }
      void SetUniformBlock(const ShaderUniforms uniformName, const float* const pMatrix)
      {
         assert(GetCurrentShader() == nullptr);
         assert(0 <= uniformName && uniformName < SHADER_UNIFORM_COUNT);
         assert(m_shader->m_stateOffsets[uniformName] != -1);
         assert(shaderUniformNames[uniformName].type == SUT_DataBlock);
         if (m_useLowPrecision)
         {
            for (unsigned int i = 0; i < m_shader->m_stateSizes[uniformName] / sizeof(float); i++)
            {
               const float* const p = pMatrix + i;
               float* const s = (float*)(m_state + m_shader->m_stateOffsets[uniformName]) + i;
               if (*p > 0 && *p < FLT_MIN_VALUE)
                  *s = FLT_MIN_VALUE;
               else if (*p < 0 && *p > -FLT_MIN_VALUE)
                  *s = -FLT_MIN_VALUE;
               else
                  *s = *p;
            }
         }
         else
         {
            memcpy(m_state + m_shader->m_stateOffsets[uniformName], pMatrix, m_shader->m_stateSizes[uniformName]);
         }
      }
      void SetTexture(const ShaderUniforms uniformName, const Sampler* const sampler)
      {
         assert(GetCurrentShader() == nullptr);
         assert(0 <= uniformName && uniformName < SHADER_UNIFORM_COUNT);
         assert(shaderUniformNames[uniformName].type == SUT_Sampler);
         assert(sampler != nullptr);
         #if defined(ENABLE_BGFX) || defined(ENABLE_OPENGL)
         assert(m_shader->m_stateOffsets[uniformName] != -1);
         *(const Sampler**)(m_state + m_shader->m_stateOffsets[uniformName]) = sampler;
         #elif defined(ENABLE_DX9)
         ShaderUniforms alias = m_shader->m_uniform_desc[uniformName].tex_alias;
         assert(m_shader->m_stateOffsets[alias] != -1);
         *(const Sampler**)(m_state + m_shader->m_stateOffsets[alias]) = sampler;
         #endif
      }

      Shader* m_shader;
      BYTE* const m_state;
      const unsigned int m_stateSize;
      const bool m_useLowPrecision;
   };

   unsigned int GetStateSize() const { return m_stateSize; }
   ShaderState* m_state = nullptr; // State that will be applied for the next begin/end pair

private:
   RenderDevice* const m_renderDevice;
   const ShaderId m_shaderId;
#if defined(ENABLE_BGFX) || defined(ENABLE_OPENGL)
   const bool m_isStereo;
#endif
   ShaderTechniques m_technique;
   string m_shaderCodeName;

   static Shader* current_shader;

   bool m_hasError = false; // True if loading the shader failed
   unsigned int m_stateSize = 0; // Overall size of a shader state data block
   int m_stateOffsets[SHADER_UNIFORM_COUNT]; // Position of each uniform inside the state data block
   int m_stateSizes[SHADER_UNIFORM_COUNT]; // Byte size of each uniform inside the state data block

   void Load();
   void ApplyUniform(const ShaderUniforms uniformName);

   struct ShaderUniform
   {
      ShaderUniformType type;
      string name;
      unsigned int count;
      unsigned int tex_unit;
      SamplerAddressMode default_clampu;
      SamplerAddressMode default_clampv;
      SamplerFilter default_filter;
   };
   struct TechniqueDef
   {
      string name;
      vector<ShaderUniforms> uniforms;
   };
   static TechniqueDef shaderTechniqueNames[SHADER_TECHNIQUE_COUNT];
   static const string shaderAttributeNames[SHADER_ATTRIBUTE_COUNT];
   static ShaderUniform shaderUniformNames[SHADER_UNIFORM_COUNT];
   ShaderUniforms getUniformByName(const string& name);
   ShaderAttributes getAttributeByName(const string& name);
   ShaderTechniques getTechniqueByName(const string& name);

   vector<ShaderUniforms> m_uniforms[SHADER_TECHNIQUE_COUNT]; // Uniforms used by each technique

   // caches
#if defined(ENABLE_BGFX)
   ShaderState* m_boundState[SHADER_TECHNIQUE_COUNT]; // The state currently applied to the backend (per program, so per technique)
   bgfx::ProgramHandle m_techniques[SHADER_TECHNIQUE_COUNT];
   bgfx::ProgramHandle m_clipPlaneTechniques[SHADER_TECHNIQUE_COUNT];
   bgfx::UniformHandle m_uniformHandles[SHADER_UNIFORM_COUNT];

   void loadProgram(const bgfx::EmbeddedShader* embeddedShaders, ShaderTechniques tech, const char* vsName, const char* fsName, const bool isClipVariant = false);

public:
   bgfx::ProgramHandle GetCore() const;

#elif defined(ENABLE_OPENGL)
   ShaderState* m_boundState[SHADER_TECHNIQUE_COUNT]; // The state currently applied to the backend (per technique for OpenGL)
   static ShaderTechniques m_boundTechnique; // TODO => move to render device ? This is global for OpenGL
   struct UniformDesc
   {
      ShaderUniform uniform;
      GLint location; // Location of the uniform
      GLuint blockBuffer;
   };
   struct ShaderTechnique
   {
      int index;
      const string& name;
      GLuint program;
      UniformDesc uniform_desc[SHADER_UNIFORM_COUNT];
   };
   ShaderTechnique* m_techniques[SHADER_TECHNIQUE_COUNT];
   string m_shaderPath;

   bool UseGeometryShader() const;
   bool parseFile(const string& fileNameRoot, const string& fileName, int level, robin_hood::unordered_map<string, string>& values, const string& parentMode);
   string analyzeFunction(const string& shaderCodeName, const string& technique, const string& functionName, const robin_hood::unordered_map<string, string>& values);
   ShaderTechnique* compileGLShader(const ShaderTechniques technique, const string& fileNameRoot, const string& shaderCodeName, const string& vertex, const string& geometry, const string& fragment);
   string PreprocessGLShader(const string& shaderCode);
   void Load(const std::string& file);

#elif defined(ENABLE_DX9)
   struct UniformDesc
   {
      ShaderUniform uniform;
      ShaderUniforms tex_alias; // For samplers, reference to the uniform which is used to define the texture (multiple uniform can be linked to the same texture for DX9, for OpenGL alias to itself)
      D3DXHANDLE handle; // Handle of the constant
      D3DXHANDLE tex_handle; // For samplers, handle fo the associated texture constant
      int sampler; // For samplers texture unit defined in the effect file
   };
   UniformDesc m_uniform_desc[SHADER_UNIFORM_COUNT];
   ShaderState* m_boundState = nullptr; // The state currently applied to the backend (per shader for DirectX effect framework)
   ShaderTechniques m_boundTechnique = ShaderTechniques::SHADER_TECHNIQUE_INVALID; // The bound technique (per shader for DirectX)
   ID3DXEffect* m_shader;
   static constexpr unsigned int TEXTURESET_STATE_CACHE_SIZE = 32;
   Sampler* m_boundTexture[TEXTURESET_STATE_CACHE_SIZE];

public:
   ID3DXEffect* Core() const { return m_shader; }
#endif
};
