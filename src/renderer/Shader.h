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
#define BGFX_PLATFORM_SUPPORTS_WGSL 0
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

#elif defined(ENABLE_DX9)
#include <d3dx9.h>
#endif

#if defined(ENABLE_BGFX) || defined(__OPENGLES__)
#define FLT_MIN_VALUE 0.00006103515625f
#else
#define FLT_MIN_VALUE 0.0000001f
#endif

#include <string>
#include "Sampler.h"

class Material;
class Texture;
class ITexManCacheable;

// Declaration of all available techniques (shader program)
// When changed, this list must also be copied unchanged to Shader.cpp (for its implementation)
#define SHADER_TECHNIQUE(name, ...) name
enum class ShaderTechnique : unsigned int
{
   SHADER_TECHNIQUE(LiveUI, ShaderUniform::matWorldView, ShaderUniform::tex_base_color, ShaderUniform::staticColor_Alpha, ShaderUniform::clip_plane),
   SHADER_TECHNIQUE(LiveUI_mono, ShaderUniform::matWorldView, ShaderUniform::tex_base_color, ShaderUniform::staticColor_Alpha, ShaderUniform::clip_plane),
   SHADER_TECHNIQUE(RenderBall, ShaderUniform::layer, ShaderUniform::matProj, ShaderUniform::matRotViewProj, ShaderUniform::cameraPosWorld, ShaderUniform::matView, ShaderUniform::matWorldView, ShaderUniform::matWorldViewInverse,
      ShaderUniform::ballLightEmission, ShaderUniform::ballLightPos, ShaderUniform::Roughness_WrapL_Edge_Thickness, ShaderUniform::cBase_Alpha, ShaderUniform::fDisableLighting_top_below, ShaderUniform::fenvEmissionScale_TexWidth,
      ShaderUniform::cAmbient_LightRange, ShaderUniform::tex_diffuse_env, ShaderUniform::orientation, ShaderUniform::invTableRes_reflection, ShaderUniform::w_h_disableLighting, ShaderUniform::tex_ball_color, ShaderUniform::tex_ball_playfield,
      ShaderUniform::tex_ball_decal, ShaderUniform::clip_plane),
   SHADER_TECHNIQUE(RenderBall_DecalMode, ShaderUniform::layer, ShaderUniform::matProj, ShaderUniform::matRotViewProj, ShaderUniform::cameraPosWorld, ShaderUniform::matView, ShaderUniform::matWorldView, ShaderUniform::matWorldViewInverse,
      ShaderUniform::ballLightEmission, ShaderUniform::ballLightPos, ShaderUniform::Roughness_WrapL_Edge_Thickness, ShaderUniform::cBase_Alpha, ShaderUniform::fDisableLighting_top_below, ShaderUniform::fenvEmissionScale_TexWidth,
      ShaderUniform::cAmbient_LightRange, ShaderUniform::tex_diffuse_env, ShaderUniform::orientation, ShaderUniform::invTableRes_reflection, ShaderUniform::w_h_disableLighting, ShaderUniform::tex_ball_color, ShaderUniform::tex_ball_playfield,
      ShaderUniform::tex_ball_decal, ShaderUniform::clip_plane),
   SHADER_TECHNIQUE(RenderBall_SphericalMap, ShaderUniform::layer, ShaderUniform::matProj, ShaderUniform::matRotViewProj, ShaderUniform::cameraPosWorld, ShaderUniform::matView, ShaderUniform::matWorldView, ShaderUniform::matWorldViewInverse,
      ShaderUniform::ballLightEmission, ShaderUniform::ballLightPos, ShaderUniform::Roughness_WrapL_Edge_Thickness, ShaderUniform::cBase_Alpha, ShaderUniform::fDisableLighting_top_below, ShaderUniform::fenvEmissionScale_TexWidth,
      ShaderUniform::cAmbient_LightRange, ShaderUniform::tex_diffuse_env, ShaderUniform::orientation, ShaderUniform::invTableRes_reflection, ShaderUniform::w_h_disableLighting, ShaderUniform::tex_ball_color, ShaderUniform::tex_ball_playfield,
      ShaderUniform::tex_ball_decal, ShaderUniform::clip_plane),
   SHADER_TECHNIQUE(RenderBall_SphericalMap_DecalMode, ShaderUniform::layer, ShaderUniform::matProj, ShaderUniform::matRotViewProj, ShaderUniform::cameraPosWorld, ShaderUniform::matView, ShaderUniform::matWorldView,
      ShaderUniform::matWorldViewInverse, ShaderUniform::ballLightEmission, ShaderUniform::ballLightPos, ShaderUniform::Roughness_WrapL_Edge_Thickness, ShaderUniform::cBase_Alpha, ShaderUniform::fDisableLighting_top_below,
      ShaderUniform::fenvEmissionScale_TexWidth, ShaderUniform::cAmbient_LightRange, ShaderUniform::tex_diffuse_env, ShaderUniform::orientation, ShaderUniform::invTableRes_reflection, ShaderUniform::w_h_disableLighting,
      ShaderUniform::tex_ball_color, ShaderUniform::tex_ball_playfield, ShaderUniform::tex_ball_decal, ShaderUniform::clip_plane),
   SHADER_TECHNIQUE(RenderBall_Debug, ShaderUniform::layer, ShaderUniform::matRotViewProj, ShaderUniform::cameraPosWorld, ShaderUniform::matWorldView, ShaderUniform::matWorldViewInverse, ShaderUniform::orientation, ShaderUniform::clip_plane),
   SHADER_TECHNIQUE(RenderBallTrail, ShaderUniform::layer, ShaderUniform::matRotViewProj, ShaderUniform::cameraPosWorld, ShaderUniform::cBase_Alpha, ShaderUniform::fenvEmissionScale_TexWidth, ShaderUniform::w_h_disableLighting,
      ShaderUniform::tex_ball_color, ShaderUniform::clip_plane),
   // OpenGL only has the first variant. DX9 needs all of them due to shader compiler limitation
   SHADER_TECHNIQUE(basic_with_texture, ShaderUniform::layer, ShaderUniform::matProj, ShaderUniform::matRotViewProj, ShaderUniform::cameraPosWorld, ShaderUniform::matWorld, ShaderUniform::matView, ShaderUniform::matWorldView,
      ShaderUniform::matWorldViewInverseTranspose, ShaderUniform::lightCenter_doShadow, ShaderUniform::balls, ShaderUniform::staticColor_Alpha, ShaderUniform::w_h_height, ShaderUniform::basicLightEmission, ShaderUniform::basicLightPos,
      ShaderUniform::Roughness_WrapL_Edge_Thickness, ShaderUniform::cBase_Alpha, ShaderUniform::fDisableLighting_top_below, ShaderUniform::fenvEmissionScale_TexWidth, ShaderUniform::cAmbient_LightRange, ShaderUniform::tex_env,
      ShaderUniform::tex_diffuse_env, ShaderUniform::cClearcoat_EdgeAlpha, ShaderUniform::cGlossy_ImageLerp, ShaderUniform::u_basic_shade_mode, ShaderUniform::refractionTint_thickness, ShaderUniform::mirrorNormal_factor,
      ShaderUniform::objectSpaceNormalMap, ShaderUniform::tex_base_color, ShaderUniform::tex_base_transmission, ShaderUniform::tex_base_normalmap, ShaderUniform::tex_reflection, ShaderUniform::tex_refraction, ShaderUniform::tex_probe_depth,
      ShaderUniform::clip_plane),
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
   SHADER_TECHNIQUE(basic_with_texture_at, ShaderUniform::layer, ShaderUniform::alphaTestValue, ShaderUniform::matProj, ShaderUniform::matRotViewProj, ShaderUniform::cameraPosWorld, ShaderUniform::matWorld, ShaderUniform::matView,
      ShaderUniform::matWorldView, ShaderUniform::matWorldViewInverseTranspose, ShaderUniform::lightCenter_doShadow, ShaderUniform::balls, ShaderUniform::staticColor_Alpha, ShaderUniform::w_h_height, ShaderUniform::basicLightEmission,
      ShaderUniform::basicLightPos, ShaderUniform::Roughness_WrapL_Edge_Thickness, ShaderUniform::cBase_Alpha, ShaderUniform::fDisableLighting_top_below, ShaderUniform::fenvEmissionScale_TexWidth, ShaderUniform::cAmbient_LightRange,
      ShaderUniform::tex_env, ShaderUniform::tex_diffuse_env, ShaderUniform::cClearcoat_EdgeAlpha, ShaderUniform::cGlossy_ImageLerp, ShaderUniform::u_basic_shade_mode, ShaderUniform::refractionTint_thickness, ShaderUniform::mirrorNormal_factor,
      ShaderUniform::objectSpaceNormalMap, ShaderUniform::tex_base_color, ShaderUniform::tex_base_transmission, ShaderUniform::tex_base_normalmap, ShaderUniform::tex_reflection, ShaderUniform::tex_refraction, ShaderUniform::tex_probe_depth,
      ShaderUniform::clip_plane),
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
   SHADER_TECHNIQUE(basic_without_texture, ShaderUniform::layer, ShaderUniform::matProj, ShaderUniform::matRotViewProj, ShaderUniform::cameraPosWorld, ShaderUniform::matWorld, ShaderUniform::matView, ShaderUniform::matWorldView,
      ShaderUniform::matWorldViewInverseTranspose, ShaderUniform::lightCenter_doShadow, ShaderUniform::balls, ShaderUniform::staticColor_Alpha, ShaderUniform::w_h_height, ShaderUniform::basicLightEmission, ShaderUniform::basicLightPos,
      ShaderUniform::Roughness_WrapL_Edge_Thickness, ShaderUniform::cBase_Alpha, ShaderUniform::fDisableLighting_top_below, ShaderUniform::fenvEmissionScale_TexWidth, ShaderUniform::cAmbient_LightRange, ShaderUniform::tex_env,
      ShaderUniform::tex_diffuse_env, ShaderUniform::cClearcoat_EdgeAlpha, ShaderUniform::cGlossy_ImageLerp, ShaderUniform::u_basic_shade_mode, ShaderUniform::refractionTint_thickness, ShaderUniform::mirrorNormal_factor,
      ShaderUniform::tex_base_transmission, ShaderUniform::tex_reflection, ShaderUniform::tex_refraction, ShaderUniform::tex_probe_depth, ShaderUniform::clip_plane),
   SHADER_TECHNIQUE(basic_without_texture_isMetal),
   SHADER_TECHNIQUE(basic_without_texture_refl),
   SHADER_TECHNIQUE(basic_without_texture_refl_isMetal),
   SHADER_TECHNIQUE(basic_without_texture_refr),
   SHADER_TECHNIQUE(basic_without_texture_refr_isMetal),
   SHADER_TECHNIQUE(basic_without_texture_refr_refl),
   SHADER_TECHNIQUE(basic_without_texture_refr_refl_isMetal),

   // Unshaded
   SHADER_TECHNIQUE(unshaded_without_texture, ShaderUniform::layer, ShaderUniform::matRotViewProj, ShaderUniform::cameraPosWorld, ShaderUniform::matWorld, ShaderUniform::matWorldView, ShaderUniform::matWorldViewInverseTranspose,
      ShaderUniform::staticColor_Alpha, ShaderUniform::clip_plane),
   SHADER_TECHNIQUE(unshaded_with_texture, ShaderUniform::layer, ShaderUniform::matRotViewProj, ShaderUniform::cameraPosWorld, ShaderUniform::matWorld, ShaderUniform::matWorldView, ShaderUniform::matWorldViewInverseTranspose,
      ShaderUniform::staticColor_Alpha, ShaderUniform::tex_base_color, ShaderUniform::clip_plane),
   SHADER_TECHNIQUE(unshaded_without_texture_shadow, ShaderUniform::layer, ShaderUniform::matRotViewProj, ShaderUniform::cameraPosWorld, ShaderUniform::matWorld, ShaderUniform::matWorldView, ShaderUniform::matWorldViewInverseTranspose,
      ShaderUniform::lightCenter_doShadow, ShaderUniform::balls, ShaderUniform::staticColor_Alpha, ShaderUniform::clip_plane),
   SHADER_TECHNIQUE(unshaded_with_texture_shadow, ShaderUniform::layer, ShaderUniform::matRotViewProj, ShaderUniform::cameraPosWorld, ShaderUniform::matWorld, ShaderUniform::matWorldView, ShaderUniform::matWorldViewInverseTranspose,
      ShaderUniform::lightCenter_doShadow, ShaderUniform::balls, ShaderUniform::staticColor_Alpha, ShaderUniform::tex_base_color, ShaderUniform::clip_plane),

   SHADER_TECHNIQUE(basic_reflection_only, ShaderUniform::layer, ShaderUniform::matRotViewProj, ShaderUniform::cameraPosWorld, ShaderUniform::matWorld, ShaderUniform::matWorldView, ShaderUniform::matWorldViewInverseTranspose,
      ShaderUniform::staticColor_Alpha, ShaderUniform::w_h_height, ShaderUniform::mirrorNormal_factor, ShaderUniform::tex_reflection, ShaderUniform::clip_plane),

   // BGFX OpenXR shaders
   SHADER_TECHNIQUE(vr_mask, ShaderUniform::matWorldViewProj, ShaderUniform::staticColor_Alpha),
   SHADER_TECHNIQUE(vr_passthrough, ShaderUniform::layer, ShaderUniform::tex_fb_unfiltered, ShaderUniform::tex_depth),

   SHADER_TECHNIQUE(bg_decal_without_texture, ShaderUniform::layer, ShaderUniform::matRotViewProj, ShaderUniform::cameraPosWorld, ShaderUniform::matWorld, ShaderUniform::matWorldView, ShaderUniform::matWorldViewInverseTranspose,
      ShaderUniform::cBase_Alpha, ShaderUniform::clip_plane),
   SHADER_TECHNIQUE(bg_decal_with_texture, ShaderUniform::layer, ShaderUniform::alphaTestValue, ShaderUniform::matRotViewProj, ShaderUniform::cameraPosWorld, ShaderUniform::matWorld, ShaderUniform::matWorldView,
      ShaderUniform::matWorldViewInverseTranspose, ShaderUniform::cBase_Alpha, ShaderUniform::tex_base_color, ShaderUniform::clip_plane),

   SHADER_TECHNIQUE(kickerBoolean, ShaderUniform::layer, ShaderUniform::matProj, ShaderUniform::matRotViewProj, ShaderUniform::cameraPosWorld, ShaderUniform::matWorld, ShaderUniform::matView, ShaderUniform::matWorldView,
      ShaderUniform::matWorldViewInverseTranspose, ShaderUniform::lightCenter_doShadow, ShaderUniform::balls, ShaderUniform::staticColor_Alpha, ShaderUniform::w_h_height, ShaderUniform::basicLightEmission, ShaderUniform::basicLightPos,
      ShaderUniform::Roughness_WrapL_Edge_Thickness, ShaderUniform::cBase_Alpha, ShaderUniform::fDisableLighting_top_below, ShaderUniform::fenvEmissionScale_TexWidth, ShaderUniform::cAmbient_LightRange, ShaderUniform::tex_env,
      ShaderUniform::tex_diffuse_env, ShaderUniform::cClearcoat_EdgeAlpha, ShaderUniform::cGlossy_ImageLerp, ShaderUniform::u_basic_shade_mode, ShaderUniform::refractionTint_thickness, ShaderUniform::mirrorNormal_factor,
      ShaderUniform::tex_base_transmission, ShaderUniform::tex_reflection, ShaderUniform::tex_refraction, ShaderUniform::tex_probe_depth, ShaderUniform::clip_plane),
   SHADER_TECHNIQUE(kickerBoolean_isMetal),

   SHADER_TECHNIQUE(light_with_texture, ShaderUniform::layer, ShaderUniform::matRotViewProj, ShaderUniform::cameraPosWorld, ShaderUniform::matWorld, ShaderUniform::matView, ShaderUniform::matWorldView, ShaderUniform::matWorldViewInverseTranspose,
      ShaderUniform::basicLightEmission, ShaderUniform::basicLightPos, ShaderUniform::Roughness_WrapL_Edge_Thickness, ShaderUniform::cBase_Alpha, ShaderUniform::fDisableLighting_top_below, ShaderUniform::fenvEmissionScale_TexWidth,
      ShaderUniform::cAmbient_LightRange, ShaderUniform::tex_env, ShaderUniform::tex_diffuse_env, ShaderUniform::cClearcoat_EdgeAlpha, ShaderUniform::cGlossy_ImageLerp, ShaderUniform::u_basic_shade_mode, ShaderUniform::lightCenter_maxRange,
      ShaderUniform::lightColor2_falloff_power, ShaderUniform::lightColor_intensity, ShaderUniform::lightingOff, ShaderUniform::tex_light_color, ShaderUniform::clip_plane),
   SHADER_TECHNIQUE(light_without_texture, ShaderUniform::layer, ShaderUniform::matRotViewProj, ShaderUniform::cameraPosWorld, ShaderUniform::matWorld, ShaderUniform::matView, ShaderUniform::matWorldView,
      ShaderUniform::matWorldViewInverseTranspose, ShaderUniform::basicLightEmission, ShaderUniform::basicLightPos, ShaderUniform::Roughness_WrapL_Edge_Thickness, ShaderUniform::cBase_Alpha, ShaderUniform::fDisableLighting_top_below,
      ShaderUniform::fenvEmissionScale_TexWidth, ShaderUniform::cAmbient_LightRange, ShaderUniform::tex_env, ShaderUniform::tex_diffuse_env, ShaderUniform::cClearcoat_EdgeAlpha, ShaderUniform::cGlossy_ImageLerp, ShaderUniform::u_basic_shade_mode,
      ShaderUniform::lightCenter_maxRange, ShaderUniform::lightColor2_falloff_power, ShaderUniform::lightColor_intensity, ShaderUniform::lightingOff, ShaderUniform::clip_plane),
   SHADER_TECHNIQUE(light_with_texture_isMetal),
   SHADER_TECHNIQUE(light_without_texture_isMetal),

   SHADER_TECHNIQUE(basic_DMD, ShaderUniform::glassPad, ShaderUniform::glassArea, ShaderUniform::vRes_Alpha_time, ShaderUniform::vColor_Intensity, ShaderUniform::tex_dmd),
   SHADER_TECHNIQUE(basic_DMD_world, ShaderUniform::glassPad, ShaderUniform::glassArea, ShaderUniform::matWorld, ShaderUniform::matRotViewProj, ShaderUniform::cameraPosWorld, ShaderUniform::vRes_Alpha_time, ShaderUniform::vColor_Intensity,
      ShaderUniform::tex_dmd, ShaderUniform::clip_plane),

   SHADER_TECHNIQUE(display_DMD, ShaderUniform::vRes_Alpha_time, ShaderUniform::w_h_height, ShaderUniform::displayProperties, ShaderUniform::glassPad, ShaderUniform::glassArea, ShaderUniform::glassTint_Roughness, ShaderUniform::displayGlass,
      ShaderUniform::vColor_Intensity, ShaderUniform::staticColor_Alpha, ShaderUniform::displayTex),
   SHADER_TECHNIQUE(display_DMD_world, ShaderUniform::matWorld, ShaderUniform::matRotViewProj, ShaderUniform::cameraPosWorld, ShaderUniform::vRes_Alpha_time, ShaderUniform::w_h_height, ShaderUniform::displayProperties, ShaderUniform::glassPad,
      ShaderUniform::glassArea, ShaderUniform::glassTint_Roughness, ShaderUniform::displayGlass, ShaderUniform::vColor_Intensity, ShaderUniform::staticColor_Alpha, ShaderUniform::displayTex, ShaderUniform::clip_plane),
   SHADER_TECHNIQUE(display_Seg, ShaderUniform::alphaSegState, ShaderUniform::glassPad, ShaderUniform::glassArea, ShaderUniform::glassTint_Roughness, ShaderUniform::displayGlass, ShaderUniform::vColor_Intensity, ShaderUniform::staticColor_Alpha,
      ShaderUniform::w_h_height, ShaderUniform::displayTex),
   SHADER_TECHNIQUE(display_Seg_world, ShaderUniform::matWorld, ShaderUniform::matRotViewProj, ShaderUniform::cameraPosWorld, ShaderUniform::alphaSegState, ShaderUniform::glassPad, ShaderUniform::glassArea, ShaderUniform::glassTint_Roughness,
      ShaderUniform::displayGlass, ShaderUniform::vColor_Intensity, ShaderUniform::staticColor_Alpha, ShaderUniform::w_h_height, ShaderUniform::displayTex, ShaderUniform::clip_plane),
   SHADER_TECHNIQUE(display_CRT, ShaderUniform::vRes_Alpha_time, ShaderUniform::glassPad, ShaderUniform::glassArea, ShaderUniform::glassTint_Roughness, ShaderUniform::displayGlass, ShaderUniform::vColor_Intensity,
      ShaderUniform::staticColor_Alpha, ShaderUniform::w_h_height, ShaderUniform::displayTex, ShaderUniform::displayProperties),
   // Lottes-CRT
   SHADER_TECHNIQUE(display_CRT_world, ShaderUniform::matWorld, ShaderUniform::matRotViewProj, ShaderUniform::cameraPosWorld, ShaderUniform::vRes_Alpha_time, ShaderUniform::glassPad, ShaderUniform::glassArea, ShaderUniform::glassTint_Roughness,
      ShaderUniform::displayGlass, ShaderUniform::vColor_Intensity, ShaderUniform::staticColor_Alpha, ShaderUniform::w_h_height, ShaderUniform::displayTex, ShaderUniform::displayProperties, ShaderUniform::clip_plane),
   // Same as above but built with the Nuance-CRT filter, the renderer picking between them per display (see Renderer::SetupCRTRender)
   SHADER_TECHNIQUE(display_CRTnuance_world, ShaderUniform::matWorld, ShaderUniform::matRotViewProj, ShaderUniform::cameraPosWorld, ShaderUniform::vRes_Alpha_time, ShaderUniform::glassPad, ShaderUniform::glassArea, ShaderUniform::glassTint_Roughness,
      ShaderUniform::displayGlass, ShaderUniform::vColor_Intensity, ShaderUniform::staticColor_Alpha, ShaderUniform::w_h_height, ShaderUniform::displayTex, ShaderUniform::displayProperties, ShaderUniform::clip_plane),

   SHADER_TECHNIQUE(basic_noDMD, ShaderUniform::glassPad, ShaderUniform::glassArea, ShaderUniform::alphaTestValue, ShaderUniform::vColor_Intensity, ShaderUniform::tex_sprite, ShaderUniform::u_basic_shade_mode),
   SHADER_TECHNIQUE(basic_noDMD_notex, ShaderUniform::glassPad, ShaderUniform::glassArea, ShaderUniform::vColor_Intensity),
   SHADER_TECHNIQUE(basic_noDMD_world, ShaderUniform::glassPad, ShaderUniform::glassArea, ShaderUniform::alphaTestValue, ShaderUniform::matWorld, ShaderUniform::matRotViewProj, ShaderUniform::cameraPosWorld, ShaderUniform::vColor_Intensity,
      ShaderUniform::tex_sprite, ShaderUniform::u_basic_shade_mode, ShaderUniform::clip_plane),

   SHADER_TECHNIQUE(basic_noLight, ShaderUniform::matRotViewProj, ShaderUniform::cameraPosWorld, ShaderUniform::lightCenter_doShadow, ShaderUniform::balls, ShaderUniform::staticColor_Alpha,
      ShaderUniform::alphaTestValueAB_filterMode_addBlend, ShaderUniform::amount_blend_modulate_vs_add_flasherMode, ShaderUniform::tex_flasher_A, ShaderUniform::tex_flasher_B, ShaderUniform::clip_plane),

   SHADER_TECHNIQUE(bulb_light, ShaderUniform::matRotViewProj, ShaderUniform::cameraPosWorld, ShaderUniform::blend_modulate_vs_add, ShaderUniform::lightCenter_maxRange, ShaderUniform::lightColor2_falloff_power,
      ShaderUniform::lightColor_intensity, ShaderUniform::clip_plane),
   SHADER_TECHNIQUE(bulb_light_with_ball_shadows, ShaderUniform::matRotViewProj, ShaderUniform::cameraPosWorld, ShaderUniform::balls, ShaderUniform::blend_modulate_vs_add, ShaderUniform::lightCenter_maxRange,
      ShaderUniform::lightColor2_falloff_power, ShaderUniform::lightColor_intensity, ShaderUniform::clip_plane),

   SHADER_TECHNIQUE(
      fb_rhtonemap, ShaderUniform::layer, ShaderUniform::w_h_height, ShaderUniform::bloom_dither_colorgrade, ShaderUniform::exposure_wcg, ShaderUniform::tex_fb_filtered, ShaderUniform::tex_bloom, ShaderUniform::tex_color_lut, ShaderUniform::tex_depth),
   SHADER_TECHNIQUE(fb_rhtonemap_AO, ShaderUniform::layer, ShaderUniform::w_h_height, ShaderUniform::bloom_dither_colorgrade, ShaderUniform::exposure_wcg, ShaderUniform::tex_fb_filtered, ShaderUniform::tex_bloom, ShaderUniform::tex_color_lut,
      ShaderUniform::tex_ao, ShaderUniform::tex_depth),
   SHADER_TECHNIQUE(fb_rhtonemap_no_filter, ShaderUniform::layer, ShaderUniform::w_h_height, ShaderUniform::bloom_dither_colorgrade, ShaderUniform::exposure_wcg, ShaderUniform::tex_fb_unfiltered, ShaderUniform::tex_bloom,
      ShaderUniform::tex_color_lut, ShaderUniform::tex_depth),
   SHADER_TECHNIQUE(fb_rhtonemap_AO_no_filter, ShaderUniform::layer, ShaderUniform::w_h_height, ShaderUniform::bloom_dither_colorgrade, ShaderUniform::exposure_wcg, ShaderUniform::tex_fb_unfiltered, ShaderUniform::tex_bloom,
      ShaderUniform::tex_color_lut, ShaderUniform::tex_ao, ShaderUniform::tex_depth),
   SHADER_TECHNIQUE(
      fb_fmtonemap, ShaderUniform::layer, ShaderUniform::w_h_height, ShaderUniform::bloom_dither_colorgrade, ShaderUniform::exposure_wcg, ShaderUniform::tex_fb_filtered, ShaderUniform::tex_bloom, ShaderUniform::tex_color_lut, ShaderUniform::tex_depth),
   SHADER_TECHNIQUE(fb_fmtonemap_AO, ShaderUniform::layer, ShaderUniform::w_h_height, ShaderUniform::bloom_dither_colorgrade, ShaderUniform::exposure_wcg, ShaderUniform::tex_fb_filtered, ShaderUniform::tex_bloom, ShaderUniform::tex_color_lut,
      ShaderUniform::tex_ao, ShaderUniform::tex_depth),
   SHADER_TECHNIQUE(fb_fmtonemap_no_filter, ShaderUniform::layer, ShaderUniform::w_h_height, ShaderUniform::bloom_dither_colorgrade, ShaderUniform::exposure_wcg, ShaderUniform::tex_fb_unfiltered, ShaderUniform::tex_bloom,
      ShaderUniform::tex_color_lut, ShaderUniform::tex_depth),
   SHADER_TECHNIQUE(fb_fmtonemap_AO_no_filter, ShaderUniform::layer, ShaderUniform::w_h_height, ShaderUniform::bloom_dither_colorgrade, ShaderUniform::exposure_wcg, ShaderUniform::tex_fb_unfiltered, ShaderUniform::tex_bloom,
      ShaderUniform::tex_color_lut, ShaderUniform::tex_ao, ShaderUniform::tex_depth),
   SHADER_TECHNIQUE(
      fb_nttonemap, ShaderUniform::layer, ShaderUniform::w_h_height, ShaderUniform::bloom_dither_colorgrade, ShaderUniform::exposure_wcg, ShaderUniform::tex_fb_filtered, ShaderUniform::tex_bloom, ShaderUniform::tex_color_lut, ShaderUniform::tex_depth),
   SHADER_TECHNIQUE(fb_nttonemap_AO, ShaderUniform::layer, ShaderUniform::w_h_height, ShaderUniform::bloom_dither_colorgrade, ShaderUniform::exposure_wcg, ShaderUniform::tex_fb_filtered, ShaderUniform::tex_bloom, ShaderUniform::tex_color_lut,
      ShaderUniform::tex_ao, ShaderUniform::tex_depth),
   SHADER_TECHNIQUE(fb_nttonemap_no_filter, ShaderUniform::layer, ShaderUniform::w_h_height, ShaderUniform::bloom_dither_colorgrade, ShaderUniform::exposure_wcg, ShaderUniform::tex_fb_unfiltered, ShaderUniform::tex_bloom,
      ShaderUniform::tex_color_lut, ShaderUniform::tex_depth),
   SHADER_TECHNIQUE(fb_nttonemap_AO_no_filter, ShaderUniform::layer, ShaderUniform::w_h_height, ShaderUniform::bloom_dither_colorgrade, ShaderUniform::exposure_wcg, ShaderUniform::tex_fb_unfiltered, ShaderUniform::tex_bloom,
      ShaderUniform::tex_color_lut, ShaderUniform::tex_ao, ShaderUniform::tex_depth),
   SHADER_TECHNIQUE(
      fb_agxtonemap, ShaderUniform::layer, ShaderUniform::w_h_height, ShaderUniform::bloom_dither_colorgrade, ShaderUniform::exposure_wcg, ShaderUniform::tex_fb_filtered, ShaderUniform::tex_bloom, ShaderUniform::tex_color_lut, ShaderUniform::tex_depth),
   SHADER_TECHNIQUE(fb_agxtonemap_AO, ShaderUniform::layer, ShaderUniform::w_h_height, ShaderUniform::bloom_dither_colorgrade, ShaderUniform::exposure_wcg, ShaderUniform::tex_fb_filtered, ShaderUniform::tex_bloom, ShaderUniform::tex_color_lut,
      ShaderUniform::tex_ao, ShaderUniform::tex_depth),
   SHADER_TECHNIQUE(fb_agxtonemap_no_filter, ShaderUniform::layer, ShaderUniform::w_h_height, ShaderUniform::bloom_dither_colorgrade, ShaderUniform::exposure_wcg, ShaderUniform::tex_fb_unfiltered, ShaderUniform::tex_bloom,
      ShaderUniform::tex_color_lut, ShaderUniform::tex_depth),
   SHADER_TECHNIQUE(fb_agxtonemap_AO_no_filter, ShaderUniform::layer, ShaderUniform::w_h_height, ShaderUniform::bloom_dither_colorgrade, ShaderUniform::exposure_wcg, ShaderUniform::tex_fb_unfiltered, ShaderUniform::tex_bloom,
      ShaderUniform::tex_color_lut, ShaderUniform::tex_ao, ShaderUniform::tex_depth),
   SHADER_TECHNIQUE(
      fb_agxptonemap, ShaderUniform::layer, ShaderUniform::w_h_height, ShaderUniform::bloom_dither_colorgrade, ShaderUniform::exposure_wcg, ShaderUniform::tex_fb_filtered, ShaderUniform::tex_bloom, ShaderUniform::tex_color_lut, ShaderUniform::tex_depth),
   SHADER_TECHNIQUE(fb_agxptonemap_AO, ShaderUniform::layer, ShaderUniform::w_h_height, ShaderUniform::bloom_dither_colorgrade, ShaderUniform::exposure_wcg, ShaderUniform::tex_fb_filtered, ShaderUniform::tex_bloom, ShaderUniform::tex_color_lut,
      ShaderUniform::tex_ao, ShaderUniform::tex_depth),
   SHADER_TECHNIQUE(fb_agxptonemap_no_filter, ShaderUniform::layer, ShaderUniform::w_h_height, ShaderUniform::bloom_dither_colorgrade, ShaderUniform::exposure_wcg, ShaderUniform::tex_fb_unfiltered, ShaderUniform::tex_bloom,
      ShaderUniform::tex_color_lut, ShaderUniform::tex_depth),
   SHADER_TECHNIQUE(fb_agxptonemap_AO_no_filter, ShaderUniform::layer, ShaderUniform::w_h_height, ShaderUniform::bloom_dither_colorgrade, ShaderUniform::exposure_wcg, ShaderUniform::tex_fb_unfiltered, ShaderUniform::tex_bloom,
      ShaderUniform::tex_color_lut, ShaderUniform::tex_ao, ShaderUniform::tex_depth),
   SHADER_TECHNIQUE(
      fb_agxgtonemap, ShaderUniform::layer, ShaderUniform::w_h_height, ShaderUniform::bloom_dither_colorgrade, ShaderUniform::exposure_wcg, ShaderUniform::tex_fb_filtered, ShaderUniform::tex_bloom, ShaderUniform::tex_color_lut, ShaderUniform::tex_depth),
   SHADER_TECHNIQUE(fb_agxgtonemap_AO, ShaderUniform::layer, ShaderUniform::w_h_height, ShaderUniform::bloom_dither_colorgrade, ShaderUniform::exposure_wcg, ShaderUniform::tex_fb_filtered, ShaderUniform::tex_bloom, ShaderUniform::tex_color_lut,
      ShaderUniform::tex_ao, ShaderUniform::tex_depth),
   SHADER_TECHNIQUE(fb_agxgtonemap_no_filter, ShaderUniform::layer, ShaderUniform::w_h_height, ShaderUniform::bloom_dither_colorgrade, ShaderUniform::exposure_wcg, ShaderUniform::tex_fb_unfiltered, ShaderUniform::tex_bloom,
      ShaderUniform::tex_color_lut, ShaderUniform::tex_depth),
   SHADER_TECHNIQUE(fb_agxgtonemap_AO_no_filter, ShaderUniform::layer, ShaderUniform::w_h_height, ShaderUniform::bloom_dither_colorgrade, ShaderUniform::exposure_wcg, ShaderUniform::tex_fb_unfiltered, ShaderUniform::tex_bloom,
      ShaderUniform::tex_color_lut, ShaderUniform::tex_ao, ShaderUniform::tex_depth),
   SHADER_TECHNIQUE(fb_wcgtonemap, ShaderUniform::layer, ShaderUniform::w_h_height, ShaderUniform::bloom_dither_colorgrade, ShaderUniform::exposure_wcg, ShaderUniform::spline1, ShaderUniform::spline2, ShaderUniform::tex_fb_filtered,
      ShaderUniform::tex_bloom, ShaderUniform::tex_color_lut, ShaderUniform::tex_depth),
   SHADER_TECHNIQUE(fb_wcgtonemap_AO, ShaderUniform::layer, ShaderUniform::w_h_height, ShaderUniform::bloom_dither_colorgrade, ShaderUniform::exposure_wcg, ShaderUniform::spline1, ShaderUniform::spline2, ShaderUniform::tex_fb_filtered,
      ShaderUniform::tex_bloom, ShaderUniform::tex_color_lut, ShaderUniform::tex_ao, ShaderUniform::tex_depth),
   SHADER_TECHNIQUE(fb_wcgtonemap_no_filter, ShaderUniform::layer, ShaderUniform::w_h_height, ShaderUniform::bloom_dither_colorgrade, ShaderUniform::exposure_wcg, ShaderUniform::spline1, ShaderUniform::spline2, ShaderUniform::tex_fb_unfiltered,
      ShaderUniform::tex_bloom, ShaderUniform::tex_color_lut, ShaderUniform::tex_depth),
   SHADER_TECHNIQUE(fb_wcgtonemap_AO_no_filter, ShaderUniform::layer, ShaderUniform::w_h_height, ShaderUniform::bloom_dither_colorgrade, ShaderUniform::exposure_wcg, ShaderUniform::spline1, ShaderUniform::spline2,
      ShaderUniform::tex_fb_unfiltered, ShaderUniform::tex_bloom, ShaderUniform::tex_color_lut, ShaderUniform::tex_ao, ShaderUniform::tex_depth),

   SHADER_TECHNIQUE(fb_blur_horiz7x7, ShaderUniform::layer, ShaderUniform::w_h_height, ShaderUniform::tex_fb_filtered),
   SHADER_TECHNIQUE(fb_blur_vert7x7, ShaderUniform::layer, ShaderUniform::w_h_height, ShaderUniform::tex_fb_filtered),
   SHADER_TECHNIQUE(fb_blur_horiz9x9, ShaderUniform::layer, ShaderUniform::w_h_height, ShaderUniform::tex_fb_filtered),
   SHADER_TECHNIQUE(fb_blur_vert9x9, ShaderUniform::layer, ShaderUniform::w_h_height, ShaderUniform::tex_fb_filtered),
   SHADER_TECHNIQUE(fb_blur_horiz11x11, ShaderUniform::layer, ShaderUniform::w_h_height, ShaderUniform::tex_fb_filtered),
   SHADER_TECHNIQUE(fb_blur_vert11x11, ShaderUniform::layer, ShaderUniform::w_h_height, ShaderUniform::tex_fb_filtered),
   SHADER_TECHNIQUE(fb_blur_horiz13x13, ShaderUniform::layer, ShaderUniform::w_h_height, ShaderUniform::tex_fb_filtered),
   SHADER_TECHNIQUE(fb_blur_vert13x13, ShaderUniform::layer, ShaderUniform::w_h_height, ShaderUniform::tex_fb_filtered),
   SHADER_TECHNIQUE(fb_blur_horiz15x15, ShaderUniform::layer, ShaderUniform::w_h_height, ShaderUniform::tex_fb_filtered),
   SHADER_TECHNIQUE(fb_blur_vert15x15, ShaderUniform::layer, ShaderUniform::w_h_height, ShaderUniform::tex_fb_filtered),
   SHADER_TECHNIQUE(fb_blur_horiz19x19, ShaderUniform::layer, ShaderUniform::w_h_height, ShaderUniform::tex_fb_filtered),
   SHADER_TECHNIQUE(fb_blur_vert19x19, ShaderUniform::layer, ShaderUniform::w_h_height, ShaderUniform::tex_fb_filtered),
   SHADER_TECHNIQUE(fb_blur_horiz23x23, ShaderUniform::layer, ShaderUniform::w_h_height, ShaderUniform::tex_fb_filtered),
   SHADER_TECHNIQUE(fb_blur_vert23x23, ShaderUniform::layer, ShaderUniform::w_h_height, ShaderUniform::tex_fb_filtered),
   SHADER_TECHNIQUE(fb_blur_horiz27x27, ShaderUniform::layer, ShaderUniform::w_h_height, ShaderUniform::tex_fb_filtered),
   SHADER_TECHNIQUE(fb_blur_vert27x27, ShaderUniform::layer, ShaderUniform::w_h_height, ShaderUniform::tex_fb_filtered),
   SHADER_TECHNIQUE(fb_blur_horiz39x39, ShaderUniform::layer, ShaderUniform::w_h_height, ShaderUniform::tex_fb_filtered),
   SHADER_TECHNIQUE(fb_blur_vert39x39, ShaderUniform::layer, ShaderUniform::w_h_height, ShaderUniform::tex_fb_filtered),

   SHADER_TECHNIQUE(AO, ShaderUniform::layer, ShaderUniform::w_h_height, ShaderUniform::AO_scale_timeblur, ShaderUniform::tex_fb_filtered, ShaderUniform::tex_depth, ShaderUniform::tex_ao_dither),
   SHADER_TECHNIQUE(fb_AO, ShaderUniform::layer, ShaderUniform::w_h_height, ShaderUniform::tex_ao), // Display debug AO
   SHADER_TECHNIQUE(fb_AO_static, ShaderUniform::layer, ShaderUniform::w_h_height, ShaderUniform::tex_fb_filtered, ShaderUniform::tex_ao), // Apply AO during static prerender pass (no tonemapping)
   SHADER_TECHNIQUE(fb_AO_no_filter_static, ShaderUniform::layer, ShaderUniform::w_h_height, ShaderUniform::tex_fb_unfiltered, ShaderUniform::tex_ao), // Apply AO during static prerender pass (no tonemapping)
   SHADER_TECHNIQUE(fb_motionblur, ShaderUniform::layer, ShaderUniform::w_h_height, ShaderUniform::tex_bloom, ShaderUniform::tex_fb_filtered, ShaderUniform::tex_depth, ShaderUniform::matProj, ShaderUniform::matProjInv, ShaderUniform::balls),
   SHADER_TECHNIQUE(fb_bloom, ShaderUniform::layer, ShaderUniform::w_h_height, ShaderUniform::tex_fb_filtered),
   SHADER_TECHNIQUE(fb_mirror, ShaderUniform::layer, ShaderUniform::w_h_height, ShaderUniform::tex_fb_unfiltered),
   SHADER_TECHNIQUE(fb_copy, ShaderUniform::layer, ShaderUniform::tex_fb_filtered),
   SHADER_TECHNIQUE(SSReflection, ShaderUniform::layer, ShaderUniform::w_h_height, ShaderUniform::SSR_bumpHeight_fresnelRefl_scale_FS, ShaderUniform::tex_fb_filtered, ShaderUniform::tex_depth, ShaderUniform::tex_ao_dither),
#ifdef ENABLE_BGFX
   SHADER_TECHNIQUE(fb_resolve_depth_msaa, ShaderUniform::layer, ShaderUniform::tex_depth),
#endif

   SHADER_TECHNIQUE(NFAA, ShaderUniform::layer, ShaderUniform::w_h_height, ShaderUniform::tex_fb_filtered, ShaderUniform::tex_depth),
   SHADER_TECHNIQUE(DLAA_edge, ShaderUniform::layer, ShaderUniform::w_h_height, ShaderUniform::tex_fb_filtered),
   SHADER_TECHNIQUE(DLAA, ShaderUniform::layer, ShaderUniform::w_h_height, ShaderUniform::tex_fb_filtered, ShaderUniform::tex_depth),
   SHADER_TECHNIQUE(FXAA1, ShaderUniform::layer, ShaderUniform::w_h_height, ShaderUniform::tex_fb_filtered, ShaderUniform::tex_depth),
   SHADER_TECHNIQUE(FXAA2, ShaderUniform::layer, ShaderUniform::w_h_height, ShaderUniform::tex_fb_filtered, ShaderUniform::tex_depth),
   SHADER_TECHNIQUE(FXAA3, ShaderUniform::layer, ShaderUniform::w_h_height, ShaderUniform::tex_fb_filtered, ShaderUniform::tex_depth),
   SHADER_TECHNIQUE(FAAA, ShaderUniform::layer, ShaderUniform::w_h_height, ShaderUniform::tex_fb_filtered, ShaderUniform::tex_depth),
   SHADER_TECHNIQUE(CAS, ShaderUniform::layer, ShaderUniform::w_h_height, ShaderUniform::tex_fb_unfiltered, ShaderUniform::tex_depth),
   SHADER_TECHNIQUE(BilateralSharp_CAS, ShaderUniform::layer, ShaderUniform::w_h_height, ShaderUniform::tex_fb_unfiltered, ShaderUniform::tex_depth),
#ifndef __OPENGLES__
   SHADER_TECHNIQUE(SMAA_ColorEdgeDetection, ShaderUniform::w_h_height, ShaderUniform::tex_fb_filtered),
   SHADER_TECHNIQUE(SMAA_BlendWeightCalculation, ShaderUniform::w_h_height, ShaderUniform::edgesTex, ShaderUniform::areaTex, ShaderUniform::searchTex),
   SHADER_TECHNIQUE(SMAA_NeighborhoodBlending, ShaderUniform::w_h_height, ShaderUniform::tex_fb_filtered, ShaderUniform::blendTex),
#endif

   SHADER_TECHNIQUE(stereo_SBS, ShaderUniform::tex_stereo_fb),
   SHADER_TECHNIQUE(stereo_TB, ShaderUniform::tex_stereo_fb),
   SHADER_TECHNIQUE(stereo_Int, ShaderUniform::tex_stereo_fb),
   SHADER_TECHNIQUE(stereo_Flipped_Int, ShaderUniform::tex_stereo_fb),
   SHADER_TECHNIQUE(Stereo_sRGBAnaglyph, ShaderUniform::tex_stereo_fb, ShaderUniform::Stereo_LeftMat, ShaderUniform::Stereo_RightMat),
   SHADER_TECHNIQUE(Stereo_GammaAnaglyph, ShaderUniform::tex_stereo_fb, ShaderUniform::Stereo_LeftMat, ShaderUniform::Stereo_RightMat, ShaderUniform::Stereo_LeftLuminance_Gamma),
   SHADER_TECHNIQUE(
      Stereo_sRGBDynDesatAnaglyph, ShaderUniform::tex_stereo_fb, ShaderUniform::Stereo_LeftMat, ShaderUniform::Stereo_RightMat, ShaderUniform::Stereo_LeftLuminance_Gamma, ShaderUniform::Stereo_RightLuminance_DynDesat),
   SHADER_TECHNIQUE(
      Stereo_GammaDynDesatAnaglyph, ShaderUniform::tex_stereo_fb, ShaderUniform::Stereo_LeftMat, ShaderUniform::Stereo_RightMat, ShaderUniform::Stereo_LeftLuminance_Gamma, ShaderUniform::Stereo_RightLuminance_DynDesat),
   SHADER_TECHNIQUE(Stereo_DeghostAnaglyph, ShaderUniform::tex_stereo_fb, ShaderUniform::Stereo_LeftMat, ShaderUniform::Stereo_RightMat, ShaderUniform::Stereo_DeghostGamma, ShaderUniform::Stereo_DeghostFilter),

   SHADER_TECHNIQUE(irradiance, ShaderUniform::layer, ShaderUniform::tex_env),

   COUNT
};
#undef SHADER_TECHNIQUE

enum class ShaderUniformType
{
   Bool,
   Int,
   Float,
   Float2,
   Float3,
   Float4,
   Float4v,
   Float3x4,
   Float4x3,
   Float4x4,
   DataBlock,
   Sampler,
   COUNT
};

// Declaration of all uniforms and samplers used in the shaders
// When changed, this list must also be copied unchanged to Shader.cpp (for its implementation)
// Samplers defines how to sample a texture. For DX9, they are defined in the effect files, only the texture reference is set through the API.
// Otherwise, the sampler states can be directly overriden through DX9Device->SetSamplerState (per tex unit), being carefull that the effect
// framework will also apply the ones defined in the effect file during Technique->Begin call (so either don't define them, or reapply).
#define SHADER_UNIFORM(type, name, count) name
#define SHADER_SAMPLER(name, tex_name, default_clampu, default_clampv, default_filter) name
enum class ShaderUniform : unsigned int
{
   // Shared uniforms
   SHADER_UNIFORM(ShaderUniformType::Int, layer, 1),
   SHADER_UNIFORM(ShaderUniformType::Float, alphaTestValue, 1),
   SHADER_UNIFORM(ShaderUniformType::Float4x4, matProj, 2), // +1 Matrix for stereo
   SHADER_UNIFORM(ShaderUniformType::Float4x4, matProjInv, 2), // +1 Matrix for stereo
   SHADER_UNIFORM(ShaderUniformType::Float4x4, matWorldViewProj, 2), // +1 Matrix for stereo
   SHADER_UNIFORM(ShaderUniformType::DataBlock, basicMatrixBlock, (5 + 4) * 16 * 4), // OpenGL only, +4 Matrix for stereo
   SHADER_UNIFORM(ShaderUniformType::DataBlock, ballMatrixBlock, (4 + 4) * 16 * 4), // OpenGL only, +4 Matrix for stereo
   SHADER_UNIFORM(ShaderUniformType::Float4x4, matWorld, 1), // DX9 & BGFX only
   SHADER_UNIFORM(ShaderUniformType::Float4x3, matView, 2), // DX9 & BGFX only, +1 Matrix for stereo
   SHADER_UNIFORM(ShaderUniformType::Float4x4, matWorldView, 2), // DX9 & BGFX only, +1 Matrix for stereo
   SHADER_UNIFORM(ShaderUniformType::Float4x3, matWorldViewInverse, 2), // DX9 & BGFX only, +1 Matrix for stereo
   SHADER_UNIFORM(ShaderUniformType::Float3x4, matWorldViewInverseTranspose, 2), // DX9 & BGFX only, +1 Matrix for stereo
   SHADER_UNIFORM(ShaderUniformType::Float4x4, matRotViewProj, 2), // BGFX only, view rotation x proj (camera-relative path), +1 for stereo
   SHADER_UNIFORM(ShaderUniformType::Float4v, cameraPosWorld, 2), // BGFX only, camera world position (camera-relative path), +1 for stereo
   SHADER_UNIFORM(ShaderUniformType::Float4, lightCenter_doShadow, 1), // Basic & Flasher (for ball shadows)
   SHADER_UNIFORM(ShaderUniformType::Float4v, balls, 8), // Basic & Flasher (for ball shadows)
   SHADER_UNIFORM(ShaderUniformType::Float4, staticColor_Alpha, 1), // Basic & Flasher
   SHADER_UNIFORM(ShaderUniformType::Float4, w_h_height, 1), // Post process & Basic (for screen space reflection/refraction)

   // Shared material for Ball, Basic and Classic light shaders
   SHADER_UNIFORM(ShaderUniformType::Float4, clip_plane, 1), // OpenGL & BGFX only
   SHADER_UNIFORM(ShaderUniformType::Float4v, basicLightEmission, 2), // OpenGL & BGFX only
   SHADER_UNIFORM(ShaderUniformType::Float4v, basicLightPos, 2), // OpenGL & BGFX only
   SHADER_UNIFORM(ShaderUniformType::Float4v, ballLightEmission, 10), // OpenGL & BGFX only
   SHADER_UNIFORM(ShaderUniformType::Float4v, ballLightPos, 10), // OpenGL & BGFX only
   SHADER_UNIFORM(ShaderUniformType::Bool, is_metal, 1), // OpenGL & BGFX only
   SHADER_UNIFORM(ShaderUniformType::Bool, doNormalMapping, 1), // OpenGL & BGFX only
   SHADER_UNIFORM(ShaderUniformType::Float4v, basicPackedLights, 3), // DX9 only
   SHADER_UNIFORM(ShaderUniformType::Float4v, ballPackedLights, 15), // DX9 only
   SHADER_UNIFORM(ShaderUniformType::Float4, Roughness_WrapL_Edge_Thickness, 1),
   SHADER_UNIFORM(ShaderUniformType::Float4, cBase_Alpha, 1),
   SHADER_UNIFORM(ShaderUniformType::Float2, fDisableLighting_top_below, 1),
   SHADER_UNIFORM(ShaderUniformType::Float2, fenvEmissionScale_TexWidth, 1),
   SHADER_UNIFORM(ShaderUniformType::Float4, cAmbient_LightRange, 1),
   SHADER_SAMPLER(tex_env, 1, SamplerAddressMode::SA_REPEAT, SamplerAddressMode::SA_CLAMP, SamplerFilter::SF_TRILINEAR), // environment
   SHADER_SAMPLER(tex_diffuse_env, 2, SamplerAddressMode::SA_REPEAT, SamplerAddressMode::SA_CLAMP, SamplerFilter::SF_BILINEAR), // diffuse environment contribution/radiance

   // Basic Shader
   SHADER_UNIFORM(ShaderUniformType::Float4, cClearcoat_EdgeAlpha, 1),
   SHADER_UNIFORM(ShaderUniformType::Float4, cGlossy_ImageLerp, 1),
   SHADER_UNIFORM(ShaderUniformType::Bool, doRefractions, 1), // OpenGL only
   SHADER_UNIFORM(ShaderUniformType::Float4, u_basic_shade_mode, 1), // BGFX Only
   SHADER_UNIFORM(ShaderUniformType::Float4, refractionTint_thickness, 1),
   SHADER_UNIFORM(ShaderUniformType::Float4, mirrorNormal_factor, 1),
   SHADER_UNIFORM(ShaderUniformType::Bool, objectSpaceNormalMap, 1),
   SHADER_SAMPLER(tex_base_color, 0, SamplerAddressMode::SA_CLAMP, SamplerAddressMode::SA_CLAMP, SamplerFilter::SF_TRILINEAR), // base texture
   SHADER_SAMPLER(tex_base_transmission, 3, SamplerAddressMode::SA_CLAMP, SamplerAddressMode::SA_CLAMP, SamplerFilter::SF_BILINEAR), // bulb light/transmission buffer texture
   SHADER_SAMPLER(tex_base_normalmap, 4, SamplerAddressMode::SA_REPEAT, SamplerAddressMode::SA_REPEAT, SamplerFilter::SF_TRILINEAR), // normal map texture
   SHADER_SAMPLER(tex_reflection, 5, SamplerAddressMode::SA_CLAMP, SamplerAddressMode::SA_CLAMP, SamplerFilter::SF_BILINEAR), // plane reflection
   SHADER_SAMPLER(tex_refraction, 6, SamplerAddressMode::SA_CLAMP, SamplerAddressMode::SA_CLAMP, SamplerFilter::SF_BILINEAR), // screen space refraction
   SHADER_SAMPLER(tex_probe_depth, 7, SamplerAddressMode::SA_CLAMP, SamplerAddressMode::SA_CLAMP, SamplerFilter::SF_BILINEAR), // depth probe

   // Ball Shader
   SHADER_UNIFORM(ShaderUniformType::Float4x3, orientation, 1),
   SHADER_UNIFORM(ShaderUniformType::Float4, invTableRes_reflection, 1),
   SHADER_UNIFORM(ShaderUniformType::Float4, w_h_disableLighting, 1),
   SHADER_SAMPLER(tex_ball_color, 0, SamplerAddressMode::SA_REPEAT, SamplerAddressMode::SA_REPEAT, SamplerFilter::SF_TRILINEAR), // base texture
   SHADER_SAMPLER(tex_ball_playfield, 4, SamplerAddressMode::SA_CLAMP, SamplerAddressMode::SA_CLAMP, SamplerFilter::SF_TRILINEAR), // playfield
   SHADER_SAMPLER(tex_ball_decal, 3, SamplerAddressMode::SA_REPEAT, SamplerAddressMode::SA_REPEAT, SamplerFilter::SF_TRILINEAR), // ball decal

   // Light Shader
   SHADER_UNIFORM(ShaderUniformType::Float, blend_modulate_vs_add, 1),
   SHADER_UNIFORM(ShaderUniformType::Float4, lightCenter_maxRange, 1), // Classic and Halo
   SHADER_UNIFORM(ShaderUniformType::Float4, lightColor2_falloff_power, 1), // Classic and Halo
   SHADER_UNIFORM(ShaderUniformType::Float4, lightColor_intensity, 1), // Classic and Halo
   SHADER_UNIFORM(ShaderUniformType::Bool, lightingOff, 1), // Classic only
   SHADER_SAMPLER(tex_light_color, 0, SamplerAddressMode::SA_REPEAT, SamplerAddressMode::SA_REPEAT, SamplerFilter::SF_TRILINEAR), // Classic only

   // DMD Shader
   SHADER_UNIFORM(ShaderUniformType::Float4, glassPad, 1),
   SHADER_UNIFORM(ShaderUniformType::Float4, glassArea, 1),
   SHADER_UNIFORM(ShaderUniformType::Float4, vRes_Alpha_time, 1),
   SHADER_UNIFORM(ShaderUniformType::Float4, backBoxSize, 1),
   SHADER_UNIFORM(ShaderUniformType::Float4, vColor_Intensity, 1),
   SHADER_SAMPLER(tex_dmd, 0, SamplerAddressMode::SA_CLAMP, SamplerAddressMode::SA_CLAMP, SamplerFilter::SF_NONE), // DMD
   SHADER_SAMPLER(tex_sprite, 0, SamplerAddressMode::SA_MIRROR, SamplerAddressMode::SA_MIRROR, SamplerFilter::SF_TRILINEAR), // Sprite

   // Display Shader
   SHADER_UNIFORM(ShaderUniformType::Float4, glassTint_Roughness, 1),
   SHADER_UNIFORM(ShaderUniformType::Float4, displayProperties, 1),
   SHADER_UNIFORM(ShaderUniformType::Float4v, alphaSegState, 4),
   SHADER_SAMPLER(
      displayTex, 0, SamplerAddressMode::SA_CLAMP, SamplerAddressMode::SA_CLAMP, SamplerFilter::SF_NONE), // DMD (Point sampling), Alpha seg (bilinear sampling), Display (Point sampling)
   SHADER_SAMPLER(displayGlass, 1, SamplerAddressMode::SA_CLAMP, SamplerAddressMode::SA_CLAMP, SamplerFilter::SF_TRILINEAR),

   // Flasher Shader
   SHADER_UNIFORM(ShaderUniformType::Float4, alphaTestValueAB_filterMode_addBlend, 1),
   SHADER_UNIFORM(ShaderUniformType::Float3, amount_blend_modulate_vs_add_flasherMode, 1),
   SHADER_SAMPLER(tex_flasher_A, 0, SamplerAddressMode::SA_CLAMP, SamplerAddressMode::SA_CLAMP, SamplerFilter::SF_TRILINEAR), // base texture
   SHADER_SAMPLER(tex_flasher_B, 1, SamplerAddressMode::SA_REPEAT, SamplerAddressMode::SA_REPEAT, SamplerFilter::SF_TRILINEAR), // texB

   // Post Process Shader
   SHADER_UNIFORM(ShaderUniformType::Float4, bloom_dither_colorgrade, 1),
   SHADER_UNIFORM(ShaderUniformType::Float4, exposure_wcg, 1),
   SHADER_UNIFORM(ShaderUniformType::Float4, spline1, 1),
   SHADER_UNIFORM(ShaderUniformType::Float2, spline2, 1),
   SHADER_UNIFORM(ShaderUniformType::Float4, SSR_bumpHeight_fresnelRefl_scale_FS, 1),
   SHADER_UNIFORM(ShaderUniformType::Float2, AO_scale_timeblur, 1),
   SHADER_SAMPLER(tex_fb_unfiltered, 0, SamplerAddressMode::SA_CLAMP, SamplerAddressMode::SA_CLAMP, SamplerFilter::SF_NONE), // Framebuffer (unfiltered)
   SHADER_SAMPLER(tex_fb_filtered, 0, SamplerAddressMode::SA_CLAMP, SamplerAddressMode::SA_CLAMP, SamplerFilter::SF_BILINEAR), // Framebuffer (filtered)
   SHADER_SAMPLER(tex_bloom, 1, SamplerAddressMode::SA_CLAMP, SamplerAddressMode::SA_CLAMP, SamplerFilter::SF_BILINEAR), // Bloom
   SHADER_SAMPLER(tex_color_lut, 2, SamplerAddressMode::SA_CLAMP, SamplerAddressMode::SA_CLAMP, SamplerFilter::SF_BILINEAR), // Color grade LUT
   SHADER_SAMPLER(tex_ao, 3, SamplerAddressMode::SA_CLAMP, SamplerAddressMode::SA_CLAMP, SamplerFilter::SF_BILINEAR), // AO Result
   SHADER_SAMPLER(tex_depth, 4, SamplerAddressMode::SA_CLAMP, SamplerAddressMode::SA_CLAMP, SamplerFilter::SF_NONE), // Depth
   SHADER_SAMPLER(tex_ao_dither, 5, SamplerAddressMode::SA_REPEAT, SamplerAddressMode::SA_REPEAT, SamplerFilter::SF_NONE), // AO dither
   //SHADER_SAMPLER(tex_tonemap_lut, 6, SamplerAddressMode::SA_CLAMP, SamplerAddressMode::SA_CLAMP, SamplerFilter::SF_BILINEAR), // Tonemap LUT
   SHADER_SAMPLER(edgesTex, 7, SamplerAddressMode::SA_CLAMP, SamplerAddressMode::SA_CLAMP, SamplerFilter::SF_TRILINEAR), // SMAA
   SHADER_SAMPLER(blendTex, 8, SamplerAddressMode::SA_CLAMP, SamplerAddressMode::SA_CLAMP, SamplerFilter::SF_TRILINEAR), // SMAA
   SHADER_SAMPLER(areaTex, 9, SamplerAddressMode::SA_CLAMP, SamplerAddressMode::SA_CLAMP, SamplerFilter::SF_BILINEAR), // SMAA
   SHADER_SAMPLER(searchTex, 10, SamplerAddressMode::SA_CLAMP, SamplerAddressMode::SA_CLAMP, SamplerFilter::SF_NONE), // SMAA

   // Stereo Shader
   SHADER_SAMPLER(tex_stereo_fb, 0, SamplerAddressMode::SA_REPEAT, SamplerAddressMode::SA_REPEAT, SamplerFilter::SF_NONE), // Framebuffer (unfiltered)
   SHADER_SAMPLER(tex_stereo_depth, 4, SamplerAddressMode::SA_REPEAT, SamplerAddressMode::SA_REPEAT, SamplerFilter::SF_NONE), // Depth
   SHADER_UNIFORM(ShaderUniformType::Float4, Stereo_MS_ZPD_YAxis, 1), // Stereo (analgyph and 3DTV)
   SHADER_UNIFORM(ShaderUniformType::Float4x4, Stereo_LeftMat, 1), // Anaglyph Stereo
   SHADER_UNIFORM(ShaderUniformType::Float4x4, Stereo_RightMat, 1), // Anaglyph Stereo
   SHADER_UNIFORM(ShaderUniformType::Float4, Stereo_DeghostGamma, 1), // Anaglyph Stereo
   SHADER_UNIFORM(ShaderUniformType::Float4x4, Stereo_DeghostFilter, 1), // Anaglyph Stereo
   SHADER_UNIFORM(ShaderUniformType::Float4, Stereo_LeftLuminance_Gamma, 1), // Anaglyph Stereo
   SHADER_UNIFORM(ShaderUniformType::Float4, Stereo_RightLuminance_DynDesat, 1), // Anaglyph Stereo

   COUNT
};
#undef SHADER_UNIFORM
#undef SHADER_SAMPLER

// When changed, this list must also be copied unchanged to Shader.cpp (for its implementation)
#define SHADER_ATTRIBUTE(name, shader_name) name
enum class ShaderAttribute
{
   SHADER_ATTRIBUTE(POS, vPosition),
   SHADER_ATTRIBUTE(NORM, vNormal),
   SHADER_ATTRIBUTE(TC, tc),
   SHADER_ATTRIBUTE(TEX, tex0),
   COUNT
};
#undef SHADER_ATTRIBUTE

struct ShaderUniformDef
{
public:
   ShaderUniformType type;
   string name;
   uint16_t count;
   unsigned int stateSize;
   unsigned int tex_unit;
   SamplerAddressMode default_clampu;
   SamplerAddressMode default_clampv;
   SamplerFilter default_filter;

   static ShaderUniformDef coreUniforms[static_cast<unsigned int>(ShaderUniform::COUNT)];
};

class Shader final
{
public:
   enum ShaderId { UI_SHADER, BALL_SHADER, BASIC_SHADER, DMD_SHADER, FLASHER_SHADER, POSTPROCESS_SHADER, LIGHT_SHADER, STEREO_SHADER };
   Shader(RenderDevice* renderDevice, const ShaderId id, const bool isStereo);
   ~Shader();

   void Begin();
   void End();

   bool HasError() const { return m_hasError; }
   static Shader* GetCurrentShader();
   static string GetTechniqueName(ShaderTechnique technique);
   void SetTechnique(const ShaderTechnique technique);
   void SetTechniqueMaterial(ShaderTechnique technique, const Material& mat, const bool doAlphaTest = false, const bool doNormalMapping = false, const bool doReflection = false, const bool doRefraction = false);
   void SetBasic(const Material * const mat, Texture * const pin);
   static void SetDefaultSamplerFilter(const ShaderUniform sampler, const SamplerFilter sf);
   static SamplerFilter GetDefaultSamplerFilter(const ShaderUniform sampler);

   void SetMaterial(const Material * const mat, const bool has_alpha = true);
   void SetAlphaTestValue(const float value);
   void SetFlasherData(const vec4& c1, const vec4& c2);
   void SetLightColorIntensity(const vec4& color);
   void SetLightColor2FalloffPower(const vec4& color);
   void SetLightData(const vec4& color);
   void SetLightImageBackglassMode(const bool imageMode, const bool backglassMode);

   //

   bool HasUniform(const ShaderUniform uniformName) const { return m_stateOffsets[static_cast<unsigned int>(uniformName)] != -1; }
   void SetFloat(const ShaderUniform uniformName, const float f);
   void SetMatrix(const ShaderUniform uniformName, const float* const pMatrix, const unsigned int count = 1);
   void SetInt(const ShaderUniform uniformName, const int i);
   void SetBool(const ShaderUniform uniformName, const bool b);
   void SetUniformBlock(const ShaderUniform uniformName, const float* const pMatrix);
   #if defined(ENABLE_DX9)
   void SetMatrix(const ShaderUniform uniformName, const D3DMATRIX* const pMatrix, const unsigned int count = 1);
   #endif
   void SetMatrix(const ShaderUniform uniformName, const Matrix3D* const pMatrix, const unsigned int count = 1);
   void SetVector(const ShaderUniform uniformName, const vec4* const pVector);
   void SetVector(const ShaderUniform uniformName, const vec4* const pVector, const unsigned int count);
   void SetVector(const ShaderUniform uniformName, const float x, const float y, const float z, const float w);
   vec4 GetVector(const ShaderUniform uniformName) const;
   void SetFloat4v(const ShaderUniform uniformName, const vec4* const pData, const unsigned int count);
   void SetTexture(const ShaderUniform uniformName, const std::shared_ptr<const Sampler>& sampler, const SamplerFilter filter = SamplerFilter::SF_UNDEFINED,
      const SamplerAddressMode clampU = SamplerAddressMode::SA_UNDEFINED, const SamplerAddressMode clampV = SamplerAddressMode::SA_UNDEFINED);
   void SetTexture(const ShaderUniform uniformName, ITexManCacheable* const texel, const bool force_linear_rgb = false, const SamplerFilter filter = SamplerFilter::SF_UNDEFINED,
      const SamplerAddressMode clampU = SamplerAddressMode::SA_UNDEFINED, const SamplerAddressMode clampV = SamplerAddressMode::SA_UNDEFINED);
   void SetTextureNull(const ShaderUniform uniformName);

   class ShaderState* m_state = nullptr; // State that will be applied for the next begin/end pair

private:
   RenderDevice* const m_renderDevice;
   const ShaderId m_shaderId;
   const bool m_isStereo;
   string m_shaderCodeName;

   static Shader* current_shader;

   bool m_hasError = false; // True if loading the shader failed
   unsigned int m_stateSize = 0; // Overall size of a shader state data block
   int m_stateOffsets[static_cast<unsigned int>(ShaderUniform::COUNT)]; // Position of each uniform inside the state data block

   void Load();
   void ApplyUniform(const ShaderUniform uniformName);

   struct TechniqueDef
   {
      string name;
      vector<ShaderUniform> uniforms;
   };

   static TechniqueDef shaderTechniqueNames[static_cast<unsigned int>(ShaderTechnique::COUNT)];
   static const string shaderAttributeNames[static_cast<unsigned int>(ShaderTechnique::COUNT)];
   ShaderUniform getUniformByName(const string& name) const;
   ShaderAttribute getAttributeByName(const string& name) const;
   static ShaderTechnique getTechniqueByName(const string& name);

   vector<ShaderUniform> m_uniforms[static_cast<unsigned int>(ShaderTechnique::COUNT)]; // Uniforms used by each technique

   // caches
#if defined(ENABLE_BGFX)
   class ShaderState* m_boundState[static_cast<unsigned int>(ShaderTechnique::COUNT)]; // The state currently applied to the backend (per program, so per technique)
   bgfx::ProgramHandle m_techniques[static_cast<unsigned int>(ShaderTechnique::COUNT)];
   bgfx::ProgramHandle m_clipPlaneTechniques[static_cast<unsigned int>(ShaderTechnique::COUNT)];
   bgfx::UniformHandle m_uniformHandles[static_cast<unsigned int>(ShaderTechnique::COUNT)];

   void loadProgram(const bgfx::EmbeddedShader* embeddedShaders, ShaderTechnique tech, const char* vsName, const char* fsName, const bool isClipVariant = false);

public:
   bgfx::ProgramHandle GetCore() const;
   bgfx::UniformHandle GetUniformHandle(ShaderUniform uniformName) const {return m_uniformHandles[static_cast<unsigned int>(uniformName)]; }
   bgfx::ProgramHandle GetProgramHandle(ShaderTechnique techniqueName) const {return m_techniques[static_cast<unsigned int>(techniqueName)]; }

#elif defined(ENABLE_OPENGL)
   class ShaderState* m_boundState[static_cast<unsigned int>(ShaderTechnique::COUNT)]; // The state currently applied to the backend (per technique for OpenGL)
   static ShaderTechnique m_boundTechnique; // TODO => move to render device ? This is global for OpenGL
   struct UniformDesc
   {
      ShaderUniformDef uniform;
      GLint location; // Location of the uniform
      GLuint blockBuffer;
   };
   struct ShaderTechniqueDef
   {
      int index;
      const string& name;
      GLuint program;
      UniformDesc uniform_desc[static_cast<unsigned int>(ShaderTechnique::COUNT)];
   };
   ShaderTechniqueDef* m_techniques[static_cast<unsigned int>(ShaderTechnique::COUNT)];
   std::filesystem::path m_shaderPath;

   bool UseGeometryShader() const;
   bool parseFile(const string& fileNameRoot, const string& filename, int level, ankerl::unordered_dense::map<string, string>& values, const string& parentMode);
   string analyzeFunction(const string& shaderCodeName, const string& technique, const string& functionName, const ankerl::unordered_dense::map<string, string>& values);
   ShaderTechniqueDef* compileGLShader(
      const ShaderTechnique technique, const string& fileNameRoot, const string& shaderCodeName, const string& vertex, const string& geometry, const string& fragment);
   string PreprocessGLShader(const string& shaderCode);
   void Load(const std::string& file);

#elif defined(ENABLE_DX9)
   struct UniformDesc
   {
      ShaderUniformDef uniform;
      ShaderUniform tex_alias; // For samplers, reference to the uniform which is used to define the texture (multiple uniform can be linked to the same texture for DX9, for OpenGL alias to itself)
      D3DXHANDLE handle; // Handle of the constant
      D3DXHANDLE tex_handle; // For samplers, handle fo the associated texture constant
      int sampler; // For samplers texture unit defined in the effect file
   };
   UniformDesc m_uniform_desc[static_cast<unsigned int>(ShaderUniform::COUNT)];
   class ShaderState* m_boundState = nullptr; // The state currently applied to the backend (per shader for DirectX effect framework)
   ShaderTechnique m_boundTechnique = ShaderTechnique::COUNT; // The bound technique (per shader for DirectX)
   ID3DXEffect* m_shader = nullptr;
   static constexpr unsigned int TEXTURESET_STATE_CACHE_SIZE = 32;
   std::shared_ptr<const Sampler> m_boundTexture[TEXTURESET_STATE_CACHE_SIZE];

public:
   void UnbindSamplers();
   ID3DXEffect* Core() const { return m_shader; }
#endif

   friend class ShaderState;
};



class ShaderState
{
public:
   // Constructor for a state holding state of a specific shader
   ShaderState(Shader* shader, bool isLowPrecision)
      : m_state(shader->m_stateSize)
      , m_shader(shader)
      , m_ownStateOffset(false)
      , m_stateOffsets(shader->m_stateOffsets)
      , m_useLowPrecision(isLowPrecision)
   {
   }

   // Constructor for a state holding all shader uniform state
   ShaderState(bool isLowPrecision)
      : m_shader(nullptr)
      , m_ownStateOffset(true)
      , m_useLowPrecision(isLowPrecision)
   {
      m_stateOffsets = new int[static_cast<unsigned int>(ShaderUniform::COUNT)];
      m_stateOffsets[0] = 0;
      for (int i = 0; i < static_cast<unsigned int>(ShaderUniform::COUNT) - 1; ++i)
         m_stateOffsets[i + 1] = m_stateOffsets[i] + ShaderUniformDef::coreUniforms[i].stateSize;
      int size = m_stateOffsets[static_cast<unsigned int>(ShaderUniform::COUNT) - 1] + ShaderUniformDef::coreUniforms[static_cast<unsigned int>(ShaderUniform::COUNT) - 1].stateSize;
      m_state.resize(size);
   }

   ~ShaderState()
   {
      if (m_ownStateOffset)
         delete[] m_stateOffsets;
   }

   void SetShader(Shader* shader)
   {
      m_shader = shader;
      m_state.resize(shader->m_stateSize);
      m_stateOffsets = shader->m_stateOffsets;
      m_samplers.clear();
   }

   void Clear()
   {
      memset(m_state.data(), 0, m_state.size());
      m_samplers.clear();
   }

   uint8_t* GetUniformStatePtr(const ShaderUniform uniformName)
   {
      assert(m_stateOffsets[static_cast<unsigned int>(uniformName)] != -1);
      return m_state.data() + m_stateOffsets[static_cast<unsigned int>(uniformName)];
   }

   void CopyTo(const bool copyTo, ShaderState* const other)
   {
      assert(other->m_stateOffsets == m_stateOffsets);
      if (copyTo)
      {
         other->m_state = m_state;
         other->m_samplers = m_samplers;
         other->m_technique = m_technique;
      }
      else
      {
         m_state = other->m_state;
         m_samplers = other->m_samplers;
         m_technique = other->m_technique;
      }
   }

   void SetBool(const ShaderUniform uniformName, const bool b)
   {
      assert(Shader::GetCurrentShader() == nullptr);
      assert(static_cast<unsigned int>(uniformName) < static_cast<unsigned int>(ShaderUniform::COUNT));
      assert(m_stateOffsets[static_cast<unsigned int>(uniformName)] != -1);
      assert(ShaderUniformDef::coreUniforms[static_cast<unsigned int>(uniformName)].type == ShaderUniformType::Bool);
      assert(ShaderUniformDef::coreUniforms[static_cast<unsigned int>(uniformName)].count == 1);
      *(bool*)(m_state.data() + m_stateOffsets[static_cast<unsigned int>(uniformName)]) = b;
   }

   void SetInt(const ShaderUniform uniformName, const int i)
   {
      assert(Shader::GetCurrentShader() == nullptr);
      assert(static_cast<unsigned int>(uniformName) < static_cast<unsigned int>(ShaderUniform::COUNT));
      if (uniformName == ShaderUniform::layer && m_stateOffsets[static_cast<unsigned int>(uniformName)] == -1)
         return; // layer uniform may be stripped out since it is only used for stereo
      assert(m_stateOffsets[static_cast<unsigned int>(uniformName)] != -1);
      assert(ShaderUniformDef::coreUniforms[static_cast<unsigned int>(uniformName)].type == ShaderUniformType::Int);
      assert(ShaderUniformDef::coreUniforms[static_cast<unsigned int>(uniformName)].count == 1);
      *(int*)(m_state.data() + m_stateOffsets[static_cast<unsigned int>(uniformName)]) = i;
   }

   void SetFloat(const ShaderUniform uniformName, const float f)
   {
      assert(Shader::GetCurrentShader() == nullptr);
      assert(static_cast<unsigned int>(uniformName) < static_cast<unsigned int>(ShaderUniform::COUNT));
      assert(m_stateOffsets[static_cast<unsigned int>(uniformName)] != -1);
      assert(ShaderUniformDef::coreUniforms[static_cast<unsigned int>(uniformName)].type == ShaderUniformType::Float);
      assert(ShaderUniformDef::coreUniforms[static_cast<unsigned int>(uniformName)].count == 1);
      if (m_useLowPrecision)
         *(float*)(m_state.data() + m_stateOffsets[static_cast<unsigned int>(uniformName)]) = (f > 0 && f < FLT_MIN_VALUE) ? FLT_MIN_VALUE : (f < 0 && f > -FLT_MIN_VALUE) ? -FLT_MIN_VALUE : f;
      else
         *(float*)(m_state.data() + m_stateOffsets[static_cast<unsigned int>(uniformName)]) = f;
   }

   float GetFloat(const ShaderUniform uniformName) const
   {
      assert(Shader::GetCurrentShader() == nullptr);
      assert(static_cast<unsigned int>(uniformName) < static_cast<unsigned int>(ShaderUniform::COUNT));
      assert(m_stateOffsets[static_cast<unsigned int>(uniformName)] != -1);
      assert(ShaderUniformDef::coreUniforms[static_cast<unsigned int>(uniformName)].type == ShaderUniformType::Float);
      assert(ShaderUniformDef::coreUniforms[static_cast<unsigned int>(uniformName)].count == 1);
      return *(float*)(m_state.data() + m_stateOffsets[static_cast<unsigned int>(uniformName)]);
   }

   void SetVector(const ShaderUniform uniformName, const vec4* const pData, const unsigned int count = 1)
   {
      assert(Shader::GetCurrentShader() == nullptr);
      assert(static_cast<unsigned int>(uniformName) < static_cast<unsigned int>(ShaderUniform::COUNT));
      assert(m_stateOffsets[static_cast<unsigned int>(uniformName)] != -1);
      assert(ShaderUniformDef::coreUniforms[static_cast<unsigned int>(uniformName)].type == ShaderUniformType::Float2 || ShaderUniformDef::coreUniforms[static_cast<unsigned int>(uniformName)].type == ShaderUniformType::Float3
         || ShaderUniformDef::coreUniforms[static_cast<unsigned int>(uniformName)].type == ShaderUniformType::Float4
         || ShaderUniformDef::coreUniforms[static_cast<unsigned int>(uniformName)].type == ShaderUniformType::Float4v);
      assert(ShaderUniformDef::coreUniforms[static_cast<unsigned int>(uniformName)].count == count);
      const int n = ShaderUniformDef::coreUniforms[static_cast<unsigned int>(uniformName)].type == ShaderUniformType::Float2 ? 2
         : ShaderUniformDef::coreUniforms[static_cast<unsigned int>(uniformName)].type == ShaderUniformType::Float3          ? 3
                                                                                                           : 4;
      if (m_useLowPrecision)
      {
         for (unsigned int i = 0; i < count; i++)
         {
            const vec4* const p = pData + i;
            vec4* const s = (vec4*)(m_state.data() + m_stateOffsets[static_cast<unsigned int>(uniformName)]) + i;
            if (p->x > 0 && p->x < FLT_MIN_VALUE)
               s->x = FLT_MIN_VALUE;
            else if (p->x < 0 && p->x > -FLT_MIN_VALUE)
               s->x = -FLT_MIN_VALUE;
            else
               s->x = p->x;
            if (p->y > 0 && p->y < FLT_MIN_VALUE)
               s->y = FLT_MIN_VALUE;
            else if (p->y < 0 && p->y > -FLT_MIN_VALUE)
               s->y = -FLT_MIN_VALUE;
            else
               s->y = p->y;
            if (n > 2)
            {
               if (p->z > 0 && p->z < FLT_MIN_VALUE)
                  s->z = FLT_MIN_VALUE;
               else if (p->z < 0 && p->z > -FLT_MIN_VALUE)
                  s->z = -FLT_MIN_VALUE;
               else
                  s->z = p->z;
            }
            if (n > 3)
            {
               if (p->w > 0 && p->w < FLT_MIN_VALUE)
                  s->w = FLT_MIN_VALUE;
               else if (p->w < 0 && p->w > -FLT_MIN_VALUE)
                  s->w = -FLT_MIN_VALUE;
               else
                  s->w = p->w;
            }
         }
      }
      else
      {
         memcpy(m_state.data() + m_stateOffsets[static_cast<unsigned int>(uniformName)], pData, count * n * sizeof(float));
      }
   }

   vec4 GetVector(const ShaderUniform uniformName) const
   {
      assert(static_cast<unsigned int>(uniformName) < static_cast<unsigned int>(ShaderUniform::COUNT));
      assert(m_stateOffsets[static_cast<unsigned int>(uniformName)] != -1);
      assert(ShaderUniformDef::coreUniforms[static_cast<unsigned int>(uniformName)].type == ShaderUniformType::Float2 || ShaderUniformDef::coreUniforms[static_cast<unsigned int>(uniformName)].type == ShaderUniformType::Float3
         || ShaderUniformDef::coreUniforms[static_cast<unsigned int>(uniformName)].type == ShaderUniformType::Float4
         || ShaderUniformDef::coreUniforms[static_cast<unsigned int>(uniformName)].type == ShaderUniformType::Float4v);
      const int n = ShaderUniformDef::coreUniforms[static_cast<unsigned int>(uniformName)].type == ShaderUniformType::Float2 ? 2
         : ShaderUniformDef::coreUniforms[static_cast<unsigned int>(uniformName)].type == ShaderUniformType::Float3          ? 3
                                                                                                           : 4;
      const vec4 result { 
         ((float*)(m_state.data() + m_stateOffsets[static_cast<unsigned int>(uniformName)]))[0], 
         ((float*)(m_state.data() + m_stateOffsets[static_cast<unsigned int>(uniformName)]))[1],
         n > 2 ? ((float*)(m_state.data() + m_stateOffsets[static_cast<unsigned int>(uniformName)]))[2] : 0.f,
         n > 3 ? ((float*)(m_state.data() + m_stateOffsets[static_cast<unsigned int>(uniformName)]))[3] : 0.f };
      return result;
   }

   void SetMatrix(const ShaderUniform uniformName, const float* const pMatrix, const unsigned int count = 1)
   {
      assert(Shader::GetCurrentShader() == nullptr);
      assert(static_cast<unsigned int>(uniformName) < static_cast<unsigned int>(ShaderUniform::COUNT));
      assert(m_stateOffsets[static_cast<unsigned int>(uniformName)] != -1);
      assert(ShaderUniformDef::coreUniforms[static_cast<unsigned int>(uniformName)].type == ShaderUniformType::Float3x4
         || ShaderUniformDef::coreUniforms[static_cast<unsigned int>(uniformName)].type == ShaderUniformType::Float4x3
         || ShaderUniformDef::coreUniforms[static_cast<unsigned int>(uniformName)].type == ShaderUniformType::Float4x4);
      assert(count == ShaderUniformDef::coreUniforms[static_cast<unsigned int>(uniformName)].count);
      if (m_useLowPrecision)
      {
         for (unsigned int i = 0; i < count * 16; i++)
         {
            const float* const p = pMatrix + i;
            float* const s = (float*)(m_state.data() + m_stateOffsets[static_cast<unsigned int>(uniformName)]) + i;
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
         memcpy(m_state.data() + m_stateOffsets[static_cast<unsigned int>(uniformName)], pMatrix, count * 16 * sizeof(float));
      }
   }

   const Matrix3D& GetMatrix(const ShaderUniform uniformName) const
   {
      assert(static_cast<unsigned int>(uniformName) < static_cast<unsigned int>(ShaderUniform::COUNT));
      assert(m_stateOffsets[static_cast<unsigned int>(uniformName)] != -1);
      assert(ShaderUniformDef::coreUniforms[static_cast<unsigned int>(uniformName)].type == ShaderUniformType::Float3x4
         || ShaderUniformDef::coreUniforms[static_cast<unsigned int>(uniformName)].type == ShaderUniformType::Float4x3
         || ShaderUniformDef::coreUniforms[static_cast<unsigned int>(uniformName)].type == ShaderUniformType::Float4x4);
      Matrix3D* m = (Matrix3D*)(m_state.data() + m_stateOffsets[static_cast<unsigned int>(uniformName)]);
      return *m;
   }

   void SetUniformBlock(const ShaderUniform uniformName, const float* const pMatrix)
   {
      assert(Shader::GetCurrentShader() == nullptr);
      assert(static_cast<unsigned int>(uniformName) < static_cast<unsigned int>(ShaderUniform::COUNT));
      assert(m_stateOffsets[static_cast<unsigned int>(uniformName)] != -1);
      assert(ShaderUniformDef::coreUniforms[static_cast<unsigned int>(uniformName)].type == ShaderUniformType::DataBlock);
      if (m_useLowPrecision)
      {
         for (unsigned int i = 0; i < ShaderUniformDef::coreUniforms[static_cast<unsigned int>(uniformName)].stateSize / sizeof(float); i++)
         {
            const float* const p = pMatrix + i;
            float* const s = (float*)(m_state.data() + m_stateOffsets[static_cast<unsigned int>(uniformName)]) + i;
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
         memcpy(m_state.data() + m_stateOffsets[static_cast<unsigned int>(uniformName)], pMatrix, ShaderUniformDef::coreUniforms[static_cast<unsigned int>(uniformName)].stateSize);
      }
   }

   void SetTexture(const ShaderUniform uniformName, std::shared_ptr<const Sampler> sampler, SamplerFilter filter = SamplerFilter::SF_UNDEFINED,
      SamplerAddressMode clampU = SamplerAddressMode::SA_UNDEFINED, SamplerAddressMode clampV = SamplerAddressMode::SA_UNDEFINED)
   {
      assert(Shader::GetCurrentShader() == nullptr);
      assert(static_cast<unsigned int>(uniformName) < static_cast<unsigned int>(ShaderUniform::COUNT));
      assert(ShaderUniformDef::coreUniforms[static_cast<unsigned int>(uniformName)].type == ShaderUniformType::Sampler);
      assert(sampler != nullptr);
      #if defined(ENABLE_BGFX) || defined(ENABLE_OPENGL)
         const ShaderUniform alias = uniformName;
      #elif defined(ENABLE_DX9)
      ShaderUniform alias = m_shader->m_uniform_desc[static_cast<unsigned int>(uniformName)].tex_alias;
      #endif
      assert(m_stateOffsets[static_cast<unsigned int>(alias)] != -1);
      int* dst = reinterpret_cast<int*>(m_state.data() + m_stateOffsets[static_cast<unsigned int>(alias)]);

      int pos = (*dst) & 0xFF;
      if (pos == 0)
      {
         m_samplers.push_back(sampler);
         *dst = static_cast<int>(m_samplers.size());
      }
      else
      {
         assert(0 < pos && pos <= static_cast<int>(m_samplers.size()));
         m_samplers[pos - 1] = sampler;
      }

      if (filter == SamplerFilter::SF_UNDEFINED)
      {
         filter = ShaderUniformDef::coreUniforms[static_cast<unsigned int>(uniformName)].default_filter;
         if (filter == SamplerFilter::SF_UNDEFINED)
            filter = SamplerFilter::SF_NONE;
      }
      // During static part prerendering, trilinear/anisotropic filtering is disabled to get sharper results
      if (m_disableMipmaps)
      {
         if (filter == SamplerFilter::SF_ANISOTROPIC || filter == SamplerFilter::SF_TRILINEAR)
            filter = SamplerFilter::SF_BILINEAR;
         else if (filter == SamplerFilter::SF_PIXELATED)
            filter = SamplerFilter::SF_NONE;
      }
      if (clampU == SamplerAddressMode::SA_UNDEFINED)
      {
         clampU = ShaderUniformDef::coreUniforms[static_cast<unsigned int>(uniformName)].default_clampu;
         if (clampU == SamplerAddressMode::SA_UNDEFINED)
            clampU = SamplerAddressMode::SA_CLAMP;
      }
      if (clampV == SamplerAddressMode::SA_UNDEFINED)
      {
         clampV = ShaderUniformDef::coreUniforms[static_cast<unsigned int>(uniformName)].default_clampv;
         if (clampV == SamplerAddressMode::SA_UNDEFINED)
            clampV = SamplerAddressMode::SA_CLAMP;
      }

      assert(static_cast<unsigned int>(clampU) <= 0x0F); // 4 bits max
      assert(static_cast<unsigned int>(clampV) <= 0x0F); // 4 bits max
      assert(static_cast<unsigned int>(filter) <= 0x0F); // 4 bits max
      *dst = (*dst & 0x0000FF) | (static_cast<unsigned int>(clampU) << 8) | (static_cast<unsigned int>(clampV) << 12) | (static_cast<unsigned int>(filter) << 20);
   }

   const std::shared_ptr<const Sampler> GetTexture(const ShaderUniform uniformName) const
   {
      assert(static_cast<unsigned int>(uniformName) < static_cast<unsigned int>(ShaderUniform::COUNT));
      assert(m_stateOffsets[static_cast<unsigned int>(uniformName)] != -1);
      assert(ShaderUniformDef::coreUniforms[static_cast<unsigned int>(uniformName)].type == ShaderUniformType::Sampler);
      assert(ShaderUniformDef::coreUniforms[static_cast<unsigned int>(uniformName)].count == 1);
      const int pos = (*(int*)(m_state.data() + m_stateOffsets[static_cast<unsigned int>(uniformName)])) & 0xFF;
      return pos > 0 ? m_samplers[pos - 1] : nullptr;
   }

   void SetTechnique(ShaderTechnique technique)
   {
      assert(Shader::GetCurrentShader() == nullptr);
      m_technique = technique;
   }

   ShaderTechnique GetTechnique() const
   {
      return m_technique;
   }

   string ToString()
   {
      if (m_technique == ShaderTechnique::COUNT)
      {
         return "Shader State: no technique defined"s;
      }
      std::stringstream ss;
      ss << "Shader State using technique " << m_shader->GetTechniqueName(m_technique) << '\n';
      for (ShaderUniform uniform : m_shader->m_uniforms[static_cast<unsigned int>(m_technique)])
      {
         if (m_stateOffsets[static_cast<unsigned int>(uniform)] != -1)
         {
            ss << ShaderUniformDef::coreUniforms[static_cast<unsigned int>(uniform)].name;
            switch (ShaderUniformDef::coreUniforms[static_cast<unsigned int>(uniform)].type)
            {
            case ShaderUniformType::Float4:
            {
               const auto pt = GetVector(uniform); 
               ss << " (" << pt.x << ", " << pt.y << ", " << pt.z << ", " << pt.w << ')' << '\n';
               break;
            }
            default: ss << " Log not yet implemented\n";
            }
         }
      }
      return ss.str();
   }

   vector<uint8_t> m_state;
   ShaderTechnique m_technique = ShaderTechnique::COUNT;
   vector<std::shared_ptr<const Sampler>> m_samplers;
   static bool m_disableMipmaps;

private:
   Shader* m_shader;
   const bool m_ownStateOffset;
   int* m_stateOffsets; // Position of each uniform inside the state data block
   const bool m_useLowPrecision;
};
