#include "stdafx.h"
#include "Shader.h"
#include "typedefs3D.h"
#include "RenderDevice.h"

#ifdef ENABLE_SDL
#include <Windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <inc/robin_hood.h>
#include <regex>
#endif

#if DEBUG_LEVEL_LOG == 0
#define LOG(a,b,c)
#else
// FIXME implement clean log
#define LOG(a, b, c)
#endif

#define SHADER_TECHNIQUE(name) #name
const string Shader::shaderTechniqueNames[SHADER_TECHNIQUE_COUNT]
{
   SHADER_TECHNIQUE(RenderBall),
   SHADER_TECHNIQUE(RenderBall_DecalMode),
   SHADER_TECHNIQUE(RenderBall_CabMode),
   SHADER_TECHNIQUE(RenderBall_CabMode_DecalMode),
   SHADER_TECHNIQUE(RenderBallTrail),
   SHADER_TECHNIQUE(basic_without_texture),
   SHADER_TECHNIQUE(basic_with_texture),
   SHADER_TECHNIQUE(basic_with_texture_normal),
   SHADER_TECHNIQUE(basic_without_texture_isMetal),
   SHADER_TECHNIQUE(basic_with_texture_isMetal),
   SHADER_TECHNIQUE(basic_with_texture_normal_isMetal),
   SHADER_TECHNIQUE(playfield_without_texture),
   SHADER_TECHNIQUE(playfield_with_texture),
   SHADER_TECHNIQUE(playfield_with_texture_normal),
   SHADER_TECHNIQUE(playfield_without_texture_isMetal),
   SHADER_TECHNIQUE(playfield_with_texture_isMetal),
   SHADER_TECHNIQUE(playfield_with_texture_normal_isMetal),
   SHADER_TECHNIQUE(playfield_refl_without_texture),
   SHADER_TECHNIQUE(playfield_refl_with_texture),
   SHADER_TECHNIQUE(basic_depth_only_without_texture),
   SHADER_TECHNIQUE(basic_depth_only_with_texture),
   SHADER_TECHNIQUE(bg_decal_without_texture),
   SHADER_TECHNIQUE(bg_decal_with_texture),
   SHADER_TECHNIQUE(kickerBoolean),
   SHADER_TECHNIQUE(kickerBoolean_isMetal),
   SHADER_TECHNIQUE(light_with_texture),
   SHADER_TECHNIQUE(light_with_texture_isMetal),
   SHADER_TECHNIQUE(light_without_texture),
   SHADER_TECHNIQUE(light_without_texture_isMetal),
   SHADER_TECHNIQUE(basic_DMD),
   SHADER_TECHNIQUE(basic_DMD_ext),
   SHADER_TECHNIQUE(basic_DMD_world),
   SHADER_TECHNIQUE(basic_DMD_world_ext),
   SHADER_TECHNIQUE(basic_noDMD),
   SHADER_TECHNIQUE(basic_noDMD_world),
   SHADER_TECHNIQUE(basic_noDMD_notex),
   SHADER_TECHNIQUE(AO),
   SHADER_TECHNIQUE(NFAA),
   SHADER_TECHNIQUE(DLAA_edge),
   SHADER_TECHNIQUE(DLAA),
   SHADER_TECHNIQUE(FXAA1),
   SHADER_TECHNIQUE(FXAA2),
   SHADER_TECHNIQUE(FXAA3),
   SHADER_TECHNIQUE(fb_tonemap),
   SHADER_TECHNIQUE(fb_bloom),
   SHADER_TECHNIQUE(fb_AO),
   SHADER_TECHNIQUE(fb_tonemap_AO),
   SHADER_TECHNIQUE(fb_tonemap_AO_static),
   SHADER_TECHNIQUE(fb_tonemap_no_filterRGB),
   SHADER_TECHNIQUE(fb_tonemap_no_filterRG),
   SHADER_TECHNIQUE(fb_tonemap_no_filterR),
   SHADER_TECHNIQUE(fb_tonemap_AO_no_filter),
   SHADER_TECHNIQUE(fb_tonemap_AO_no_filter_static),
   SHADER_TECHNIQUE(fb_bloom_horiz9x9),
   SHADER_TECHNIQUE(fb_bloom_vert9x9),
   SHADER_TECHNIQUE(fb_bloom_horiz19x19),
   SHADER_TECHNIQUE(fb_bloom_vert19x19),
   SHADER_TECHNIQUE(fb_bloom_horiz19x19h),
   SHADER_TECHNIQUE(fb_bloom_vert19x19h),
   SHADER_TECHNIQUE(fb_bloom_horiz39x39),
   SHADER_TECHNIQUE(fb_bloom_vert39x39),
   SHADER_TECHNIQUE(fb_mirror),
   SHADER_TECHNIQUE(CAS),
   SHADER_TECHNIQUE(BilateralSharp_CAS),
   SHADER_TECHNIQUE(SSReflection),
   SHADER_TECHNIQUE(basic_noLight),
   SHADER_TECHNIQUE(bulb_light),
   SHADER_TECHNIQUE(SMAA_ColorEdgeDetection),
   SHADER_TECHNIQUE(SMAA_BlendWeightCalculation),
   SHADER_TECHNIQUE(SMAA_NeighborhoodBlending),
   SHADER_TECHNIQUE(stereo),
   SHADER_TECHNIQUE(stereo_Int),
   SHADER_TECHNIQUE(stereo_Flipped_Int),
   SHADER_TECHNIQUE(stereo_anaglyph),
   SHADER_TECHNIQUE(stereo_AMD_DEBUG),
};
#undef SHADER_TECHNIQUE

ShaderTechniques Shader::getTechniqueByName(const string& name)
{
   for (int i = 0; i < SHADER_TECHNIQUE_COUNT; ++i)
      if (name == shaderTechniqueNames[i])
         return ShaderTechniques(i);

   LOG(1, m_shaderCodeName, string("getTechniqueByName Could not find technique ").append(name).append(" in shaderTechniqueNames."));
   return SHADER_TECHNIQUE_INVALID;
}

#define SHADER_UNIFORM(type, name) { type, #name, #name, ""s, -1, SA_UNDEFINED, SA_UNDEFINED, SF_UNDEFINED }
#define SHADER_SAMPLER(name, legacy_name, texture_ref, default_tex_unit, default_clampu, default_clampv, default_filter) { SUT_Sampler, #name, #legacy_name, #texture_ref, default_tex_unit, default_clampu, default_clampv, default_filter }
Shader::ShaderUniform Shader::shaderUniformNames[SHADER_UNIFORM_COUNT] {
   // -- Matrices --
   SHADER_UNIFORM(SUT_FloatBlock, matrixBlock), // OpenGL only, matrices as a float block
   SHADER_UNIFORM(SUT_Float4x4, matWorldViewProj), // DX9 only
   SHADER_UNIFORM(SUT_Float4x4, matWorldView), // DX9 only
   SHADER_UNIFORM(SUT_Float4x3, matWorldViewInverse), // DX9 only
   SHADER_UNIFORM(SUT_Float3x4, matWorldViewInverseTranspose), // DX9 only
   SHADER_UNIFORM(SUT_Float4x3, matView), // DX9 only
   SHADER_UNIFORM(SUT_Float4x3, orientation),
   // -- Floats --
   SHADER_UNIFORM(SUT_Float, RenderBall),
   SHADER_UNIFORM(SUT_Float, blend_modulate_vs_add),
   SHADER_UNIFORM(SUT_Float, alphaTestValue),
   SHADER_UNIFORM(SUT_Float, eye),
   SHADER_UNIFORM(SUT_Float, fKickerScale),
   SHADER_UNIFORM(SUT_Float, mirrorFactor),
   // -- Vectors and Float Arrays --
   SHADER_UNIFORM(SUT_Float4, Roughness_WrapL_Edge_Thickness),
   SHADER_UNIFORM(SUT_Float4, cBase_Alpha),
   SHADER_UNIFORM(SUT_Float4, lightCenter_maxRange),
   SHADER_UNIFORM(SUT_Float4, lightColor2_falloff_power),
   SHADER_UNIFORM(SUT_Float4, lightColor_intensity),
   SHADER_UNIFORM(SUT_Float2, fenvEmissionScale_TexWidth),
   SHADER_UNIFORM(SUT_Float4, invTableRes_playfield_height_reflection),
   SHADER_UNIFORM(SUT_Float4, cAmbient_LightRange),
   SHADER_UNIFORM(SUT_Float3, cClearcoat_EdgeAlpha),
   SHADER_UNIFORM(SUT_Float3, cGlossy_ImageLerp),
   SHADER_UNIFORM(SUT_Float2, fDisableLighting_top_below),
   SHADER_UNIFORM(SUT_Float4, backBoxSize),
   SHADER_UNIFORM(SUT_Float4, vColor_Intensity),
   SHADER_UNIFORM(SUT_Float4, w_h_height),
   SHADER_UNIFORM(SUT_Float4, alphaTestValueAB_filterMode_addBlend),
   SHADER_UNIFORM(SUT_Float3, amount_blend_modulate_vs_add_flasherMode),
   SHADER_UNIFORM(SUT_Float4, staticColor_Alpha),
   SHADER_UNIFORM(SUT_Float4, ms_zpd_ya_td),
   SHADER_UNIFORM(SUT_Float2, Anaglyph_DeSaturation_Contrast),
   SHADER_UNIFORM(SUT_Float4, vRes_Alpha_time),
   SHADER_UNIFORM(SUT_Float4, SSR_bumpHeight_fresnelRefl_scale_FS),
   SHADER_UNIFORM(SUT_Float2, AO_scale_timeblur),
   SHADER_UNIFORM(SUT_Float4, cWidth_Height_MirrorAmount),
   SHADER_UNIFORM(SUT_Float4, clip_planes), // OpenGL only
   SHADER_UNIFORM(SUT_Float4, lightEmission),
   SHADER_UNIFORM(SUT_Float4, lightPos),
   SHADER_UNIFORM(SUT_Float4v, packedLights), // DX9 only
   // -- Integer and Bool --
   SHADER_UNIFORM(SUT_Bool, ignoreStereo),
   SHADER_UNIFORM(SUT_Bool, disableLighting),
   SHADER_UNIFORM(SUT_Bool, doNormalMapping),
   SHADER_UNIFORM(SUT_Bool, is_metal),
   SHADER_UNIFORM(SUT_Bool, color_grade),
   SHADER_UNIFORM(SUT_Bool, do_bloom),
   SHADER_UNIFORM(SUT_Bool, lightingOff),
   SHADER_UNIFORM(SUT_Bool, objectSpaceNormalMap),
   SHADER_UNIFORM(SUT_Bool, do_dither),
   SHADER_UNIFORM(SUT_Bool, imageBackglassMode),
   SHADER_UNIFORM(SUT_Int, lightSources),
   // -- Samplers (a texture reference with sampling configuration) --
   // DMD shader
   SHADER_SAMPLER(tex_dmd, texSampler0, Texture0, 0, SA_CLAMP, SA_CLAMP, SF_NONE), // DMD
   SHADER_SAMPLER(tex_sprite, texSampler1, Texture0, 0, SA_MIRROR, SA_MIRROR, SF_TRILINEAR), // Sprite
   // Flasher shader
   SHADER_SAMPLER(tex_flasher_A, texSampler0, Texture0, 0, SA_REPEAT, SA_REPEAT, SF_TRILINEAR), // base texture
   SHADER_SAMPLER(tex_flasher_B, texSampler1, Texture1, 1, SA_REPEAT, SA_REPEAT, SF_TRILINEAR), // texB
   // FB shader
   SHADER_SAMPLER(tex_fb_unfiltered, texSampler4, Texture0, 0, SA_CLAMP, SA_CLAMP, SF_NONE), // Framebuffer (unfiltered)
   SHADER_SAMPLER(tex_fb_filtered, texSampler5, Texture0, 0, SA_CLAMP, SA_CLAMP, SF_BILINEAR), // Framebuffer (filtered)
   SHADER_SAMPLER(tex_mirror, texSamplerMirror, Texture0, 0, SA_CLAMP, SA_CLAMP, SF_BILINEAR), // base mirror texture
   SHADER_SAMPLER(tex_bloom, texSamplerBloom, Texture1, 1, SA_CLAMP, SA_CLAMP, SF_BILINEAR), // Bloom
   SHADER_SAMPLER(tex_ao, texSampler3, Texture3, 2, SA_CLAMP, SA_CLAMP, SF_BILINEAR), // AO Result
   SHADER_SAMPLER(tex_depth, texSamplerDepth, Texture3, 2, SA_CLAMP, SA_CLAMP, SF_NONE), // Depth
   SHADER_SAMPLER(tex_color_lut, texSampler6, Texture4, 2, SA_CLAMP, SA_CLAMP, SF_BILINEAR), // Color grade LUT
   SHADER_SAMPLER(tex_ao_dither, texSamplerAOdither, Texture4, 3, SA_REPEAT, SA_REPEAT, SF_NONE), // AO dither
   // Ball shader
   SHADER_SAMPLER(tex_ball_color, texSampler0, Texture0, 0, SA_REPEAT, SA_REPEAT, SF_TRILINEAR), // base texture
   SHADER_SAMPLER(tex_ball_playfield, texSampler1, Texture1, 1, SA_CLAMP, SA_CLAMP, SF_TRILINEAR), // playfield
   //SHADER_SAMPLER(tex_diffuse_env, texSampler2, Texture2, 2, SA_REPEAT, SA_CLAMP, SF_BILINEAR), // diffuse environment contribution/radiance [Shared with basic]
   SHADER_SAMPLER(tex_ball_decal, texSampler7, Texture3, 3, SA_REPEAT, SA_REPEAT, SF_TRILINEAR), // ball decal
   // Basic shader
   SHADER_SAMPLER(tex_base_color, texSampler0, Texture0, 0, SA_REPEAT, SA_REPEAT, SF_TRILINEAR), // base texture
   SHADER_SAMPLER(tex_env, texSampler1, Texture1, 1, SA_REPEAT, SA_CLAMP, SF_TRILINEAR), // environment
   SHADER_SAMPLER(tex_diffuse_env, texSampler2, Texture2, 2, SA_REPEAT, SA_CLAMP, SF_BILINEAR), // diffuse environment contribution/radiance
   SHADER_SAMPLER(tex_base_transmission, texSamplerBL, Texture3, 3, SA_CLAMP, SA_CLAMP, SF_BILINEAR), // bulb light/transmission buffer texture
   SHADER_SAMPLER(tex_playfield_reflection, texSamplerPFReflections, Texture3, 3, SA_CLAMP, SA_CLAMP, SF_NONE), // playfield reflection
   SHADER_SAMPLER(tex_base_normalmap, texSamplerN, Texture4, 4, SA_REPEAT, SA_REPEAT, SF_TRILINEAR), // normal map texture
   // Classic light shader
   SHADER_SAMPLER(tex_light_color, texSampler0, Texture0, 0, SA_REPEAT, SA_REPEAT, SF_TRILINEAR), // base texture
   // SHADER_SAMPLER(tex_env, texSampler1, Texture1, 1, SA_REPEAT, SA_CLAMP, SF_TRILINEAR), // environment [Shared with basic]
   // SHADER_SAMPLER(tex_diffuse_env, texSampler2, Texture2, 2, SA_REPEAT, SA_CLAMP, SF_BILINEAR), // diffuse environment contribution/radiance [Shared with basic]
   // Stereo shader (VPVR only, combine the 2 rendered eyes into a single one)
   SHADER_SAMPLER(tex_stereo_fb, texSampler0, Texture0, 0, SA_CLAMP, SA_CLAMP, SF_NONE), // Framebuffer (unfiltered)
   // SMAA shader
   SHADER_SAMPLER(colorTex, colorTex, colorTex, 0, SA_CLAMP, SA_CLAMP, SF_BILINEAR),
   SHADER_SAMPLER(colorGammaTex, colorGammaTex, colorGammaTex, 1, SA_CLAMP, SA_CLAMP, SF_BILINEAR),
   SHADER_SAMPLER(edgesTex, edgesTex, edgesTex2D, 2, SA_CLAMP, SA_CLAMP, SF_BILINEAR),
   SHADER_SAMPLER(blendTex, blendTex, blendTex2D, 3, SA_CLAMP, SA_CLAMP, SF_BILINEAR),
   SHADER_SAMPLER(areaTex, areaTex, areaTex2D, 4, SA_CLAMP, SA_CLAMP, SF_BILINEAR),
   SHADER_SAMPLER(searchTex, searchTex, searchTex2D, 5, SA_CLAMP, SA_CLAMP, SF_NONE), // Note that this should have a w address mode set to clamp as well
};
#undef SHADER_UNIFORM
#undef SHADER_SAMPLER

ShaderUniforms Shader::getUniformByName(const string& name)
{
   for (int i = 0; i < SHADER_UNIFORM_COUNT; ++i)
      if (name == shaderUniformNames[i].name)
         return (ShaderUniforms)i;

   LOG(1, m_shaderCodeName, string("getUniformByName Could not find uniform ").append(name).append(" in shaderUniformNames."));
   return SHADER_UNIFORM_INVALID;
}

void Shader::SetDefaultSamplerFilter(const ShaderUniforms sampler, const SamplerFilter sf)
{
   Shader::shaderUniformNames[sampler].default_filter = sf;
}

//
//
//

Shader* Shader::current_shader = nullptr;
Shader* Shader::GetCurrentShader() { return current_shader;  }

Shader::Shader(RenderDevice* renderDevice)
   : currentMaterial(-FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX, 0xCCCCCCCC, 0xCCCCCCCC, 0xCCCCCCCC, false, false, -FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX)
{
   m_renderDevice = renderDevice;
   m_technique = SHADER_TECHNIQUE_INVALID;
#ifdef ENABLE_SDL
   logFile = nullptr;
   memset(m_uniformCache, 0, sizeof(UniformCache) * SHADER_UNIFORM_COUNT * (SHADER_TECHNIQUE_COUNT + 1));
   memset(m_techniques, 0, sizeof(ShaderTechnique*) * SHADER_TECHNIQUE_COUNT);
   memset(m_isCacheValid, 0, sizeof(bool) * SHADER_TECHNIQUE_COUNT);
#else
   m_shader = nullptr;
#endif
   for (unsigned int i = 0; i < TEXTURESET_STATE_CACHE_SIZE; ++i)
      currentTexture[i] = nullptr;
   currentAlphaTestValue = -FLT_MAX;
   currentDisableLighting = vec4(-FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX);
   currentFlasherData = vec4(-FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX);
   currentFlasherData2 = vec4(-FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX);
   currentFlasherColor = vec4(-FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX);
   currentLightColor = vec4(-FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX);
   currentLightColor2 = vec4(-FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX);
   currentLightData = vec4(-FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX);
   currentLightImageMode = ~0u;
   currentLightBackglassMode = ~0u;
   currentTechnique[0] = '\0';
}

Shader::~Shader()
{
   this->Unload();
}

void Shader::Begin()
{
   assert(current_shader == nullptr);
   current_shader = this;
#ifndef ENABLE_SDL
   unsigned int cPasses;
   CHECKD3D(m_shader->Begin(&cPasses, 0));
   CHECKD3D(m_shader->BeginPass(0));
#endif
}

void Shader::End()
{
   assert(current_shader == this);
   current_shader = nullptr;
#ifndef ENABLE_SDL
   CHECKD3D(m_shader->EndPass());
   CHECKD3D(m_shader->End());
#endif
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
      bIsMetal = mat->m_bIsMetal;
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

   // bIsMetal is nowadays handled via a separate technique! (so not in here)

   if (fRoughness != currentMaterial.m_fRoughness || fEdge != currentMaterial.m_fEdge || fWrapLighting != currentMaterial.m_fWrapLighting || fThickness != currentMaterial.m_fThickness)
   {
      const vec4 rwem(fRoughness, fWrapLighting, fEdge, fThickness);
      SetVector(SHADER_Roughness_WrapL_Edge_Thickness, &rwem);
      currentMaterial.m_fRoughness = fRoughness;
      currentMaterial.m_fWrapLighting = fWrapLighting;
      currentMaterial.m_fEdge = fEdge;
      currentMaterial.m_fThickness = fThickness;
   }

   const float alpha = bOpacityActive ? fOpacity : 1.0f;
   if (cBase != currentMaterial.m_cBase || alpha != currentMaterial.m_fOpacity)
   {
      const vec4 cBaseF = convertColor(cBase, alpha);
      SetVector(SHADER_cBase_Alpha, &cBaseF);
      currentMaterial.m_cBase = cBase;
      currentMaterial.m_fOpacity = alpha;
   }

   if (!bIsMetal) // Metal has no glossy
      if (cGlossy != currentMaterial.m_cGlossy || fGlossyImageLerp != currentMaterial.m_fGlossyImageLerp)
      {
         const vec4 cGlossyF = convertColor(cGlossy, fGlossyImageLerp);
         SetVector(SHADER_cGlossy_ImageLerp, &cGlossyF);
         currentMaterial.m_cGlossy = cGlossy;
         currentMaterial.m_fGlossyImageLerp = fGlossyImageLerp;
      }

   if (cClearcoat != currentMaterial.m_cClearcoat || (bOpacityActive && fEdgeAlpha != currentMaterial.m_fEdgeAlpha))
   {
      const vec4 cClearcoatF = convertColor(cClearcoat, fEdgeAlpha);
      SetVector(SHADER_cClearcoat_EdgeAlpha, &cClearcoatF);
      currentMaterial.m_cClearcoat = cClearcoat;
      currentMaterial.m_fEdgeAlpha = fEdgeAlpha;
   }

   if (bOpacityActive && (has_alpha || alpha < 1.0f))
      g_pplayer->m_pin3d.EnableAlphaBlend(false);
   else
      g_pplayer->m_pin3d.m_pd3dPrimaryDevice->SetRenderState(RenderDevice::ALPHABLENDENABLE, RenderDevice::RS_FALSE);
}

void Shader::SetDisableLighting(const vec4& value) // sets the two top and below lighting flags, z and w unused
{
   if (currentDisableLighting.x != value.x || currentDisableLighting.y != value.y)
   {
      currentDisableLighting = value;
      SetVector(SHADER_fDisableLighting_top_below, &value);
   }
}

void Shader::SetAlphaTestValue(const float value)
{
   if (currentAlphaTestValue != value)
   {
      currentAlphaTestValue = value;
      SetFloat(SHADER_alphaTestValue, value);
   }
}

void Shader::SetFlasherColorAlpha(const vec4& color)
{
   if (currentFlasherColor.x != color.x || currentFlasherColor.y != color.y || currentFlasherColor.z != color.z || currentFlasherColor.w != color.w)
   {
      currentFlasherColor = color;
      SetVector(SHADER_staticColor_Alpha, &color);
   }
}

vec4 Shader::GetCurrentFlasherColorAlpha() const
{
   return currentFlasherColor;
}

void Shader::SetFlasherData(const vec4& c1, const vec4& c2)
{
   if (currentFlasherData.x != c1.x || currentFlasherData.y != c1.y || currentFlasherData.z != c1.z || currentFlasherData.w != c1.w)
   {
      currentFlasherData = c1;
      SetVector(SHADER_alphaTestValueAB_filterMode_addBlend, &c1);
   }
   if (currentFlasherData2.x != c2.x || currentFlasherData2.y != c2.y || currentFlasherData2.z != c2.z || currentFlasherData2.w != c2.w)
   {
      currentFlasherData2 = c2;
      SetVector(SHADER_amount_blend_modulate_vs_add_flasherMode, &c2);
   }
}

void Shader::SetLightColorIntensity(const vec4& color)
{
   if (currentLightColor.x != color.x || currentLightColor.y != color.y || currentLightColor.z != color.z || currentLightColor.w != color.w)
   {
      currentLightColor = color;
      SetVector(SHADER_lightColor_intensity, &color);
   }
}

void Shader::SetLightColor2FalloffPower(const vec4& color)
{
   if (currentLightColor2.x != color.x || currentLightColor2.y != color.y || currentLightColor2.z != color.z || currentLightColor2.w != color.w)
   {
      currentLightColor2 = color;
      SetVector(SHADER_lightColor2_falloff_power, &color);
   }
}

void Shader::SetLightData(const vec4& color)
{
   if (currentLightData.x != color.x || currentLightData.y != color.y || currentLightData.z != color.z || currentLightData.w != color.w)
   {
      currentLightData = color;
      SetVector(SHADER_lightCenter_maxRange, &color);
   }
}

void Shader::SetLightImageBackglassMode(const bool imageMode, const bool backglassMode)
{
   if (currentLightImageMode != (unsigned int)imageMode || currentLightBackglassMode != (unsigned int)backglassMode)
   {
      currentLightImageMode = (unsigned int)imageMode;
      currentLightBackglassMode = (unsigned int)backglassMode;
      SetBool(SHADER_lightingOff, imageMode || backglassMode); // at the moment can be combined into a single bool due to what the shader actually does in the end
   }
}

void Shader::Unload()
{
   SAFE_RELEASE(m_shader);
}

void Shader::SetTextureNull(const ShaderUniforms samplerUniform)
{
   auto texelName = shaderUniformNames[samplerUniform].texture_ref.c_str();
   const unsigned int idx = texelName[strlen(texelName) - 1] - '0'; // current convention: SetTexture gets "TextureX", where X 0..4
   const bool cache = (idx < TEXTURESET_STATE_CACHE_SIZE);

   if (cache)
      currentTexture[idx] = nullptr; // direct set of device tex invalidates the cache

   CHECKD3D(m_shader->SetTexture(texelName, nullptr));

   m_renderDevice->m_curTextureChanges++;
}

void Shader::SetTexture(const ShaderUniforms samplerUniform, Texture* texel, const SamplerFilter filter, const SamplerAddressMode clampU, const SamplerAddressMode clampV, const bool force_linear_rgb)
{
   auto texelName = shaderUniformNames[samplerUniform].texture_ref.c_str();
   const unsigned int idx = texelName[strlen(texelName) - 1] - '0'; // current convention: SetTexture gets "TextureX", where X 0..4
   assert(idx < TEXTURESET_STATE_CACHE_SIZE);

   if (!texel || !texel->m_pdsBuffer)
   {
      currentTexture[idx] = nullptr; // invalidate the cache

      CHECKD3D(m_shader->SetTexture(texelName, nullptr));

      m_renderDevice->m_curTextureChanges++;

      return;
   }

   if (texel->m_pdsBuffer != currentTexture[idx])
   {
      currentTexture[idx] = texel->m_pdsBuffer;
      CHECKD3D(m_shader->SetTexture(texelName, m_renderDevice->m_texMan.LoadTexture(texel->m_pdsBuffer, filter, clampU, clampV, force_linear_rgb)->GetCoreTexture()));

      m_renderDevice->m_curTextureChanges++;
   }
}

void Shader::SetTexture(const ShaderUniforms texelName, BaseTexture* texel, const SamplerFilter filter, const SamplerAddressMode clampU, const SamplerAddressMode clampV, const bool force_linear_rgb)
{
   if (!texel)
      SetTextureNull(texelName);
   else
      SetTexture(texelName, m_renderDevice->m_texMan.LoadTexture(texel, filter, clampU, clampV, force_linear_rgb));
}

void Shader::SetTexture(const ShaderUniforms samplerUniform, Sampler* texel)
{
   auto texelName = shaderUniformNames[samplerUniform].texture_ref.c_str();
   const unsigned int idx = texelName[strlen(texelName) - 1] - '0'; // current convention: SetTexture gets "TextureX", where X 0..4
   assert(idx < TEXTURESET_STATE_CACHE_SIZE);

   currentTexture[idx] = nullptr; // direct set of device tex invalidates the cache

   CHECKD3D(m_shader->SetTexture(texelName, texel->GetCoreTexture()));

   m_renderDevice->m_curTextureChanges++;
}

//

void Shader::SetTechnique(const ShaderTechniques technique)
{
   if (m_technique != technique)
   {
      m_technique = technique;
      CHECKD3D(m_shader->SetTechnique((D3DXHANDLE)shaderTechniqueNames[technique].c_str()));
      m_renderDevice->m_curTechniqueChanges++;
   }
}

void Shader::SetTechniqueMetal(const ShaderTechniques technique, const bool isMetal)
{
   if (isMetal)
   {
      switch (technique)
      {
      case SHADER_TECHNIQUE_basic_with_texture: SetTechnique(SHADER_TECHNIQUE_basic_with_texture_isMetal); break;
      case SHADER_TECHNIQUE_basic_with_texture_normal: SetTechnique(SHADER_TECHNIQUE_basic_with_texture_normal_isMetal); break;
      case SHADER_TECHNIQUE_basic_without_texture: SetTechnique(SHADER_TECHNIQUE_basic_without_texture_isMetal); break;
      case SHADER_TECHNIQUE_playfield_with_texture: SetTechnique(SHADER_TECHNIQUE_playfield_with_texture_isMetal); break;
      case SHADER_TECHNIQUE_playfield_with_texture_normal: SetTechnique(SHADER_TECHNIQUE_playfield_with_texture_normal_isMetal); break;
      case SHADER_TECHNIQUE_playfield_without_texture: SetTechnique(SHADER_TECHNIQUE_playfield_without_texture_isMetal); break;
      case SHADER_TECHNIQUE_kickerBoolean: SetTechnique(SHADER_TECHNIQUE_kickerBoolean_isMetal); break;
      case SHADER_TECHNIQUE_light_with_texture: SetTechnique(SHADER_TECHNIQUE_light_with_texture_isMetal); break;
      case SHADER_TECHNIQUE_light_without_texture: SetTechnique(SHADER_TECHNIQUE_light_without_texture_isMetal); break;
      default: assert(false); // Invalid technique: no metal shading variant
      }
   }
   else
   {
      SetTechnique(technique);
   }
}

void Shader::SetMatrix(const ShaderUniforms hParameter, const D3DXMATRIX* pMatrix)
{
   /*CHECKD3D(*/m_shader->SetMatrix(shaderUniformNames[hParameter].name.c_str(), pMatrix)/*)*/; // leads to invalid calls when setting some of the matrices (as hlsl compiler optimizes some down to less than 4x4)
   m_renderDevice->m_curParameterChanges++;
}

void Shader::SetVector(const ShaderUniforms hParameter, const vec4* pVector)
{
   CHECKD3D(m_shader->SetVector(shaderUniformNames[hParameter].name.c_str(), pVector));
   m_renderDevice->m_curParameterChanges++;
}

void Shader::SetFloat(const ShaderUniforms hParameter, const float f)
{
   CHECKD3D(m_shader->SetFloat(shaderUniformNames[hParameter].name.c_str(), f));
   m_renderDevice->m_curParameterChanges++;
}

void Shader::SetBool(const ShaderUniforms hParameter, const bool b)
{
   CHECKD3D(m_shader->SetBool(shaderUniformNames[hParameter].name.c_str(), b));
   m_renderDevice->m_curParameterChanges++;
}

void Shader::SetValue(const ShaderUniforms hParameter, const void* pData, const unsigned int Bytes)
{
   CHECKD3D(m_shader->SetValue(shaderUniformNames[hParameter].name.c_str(), pData, Bytes));
   m_renderDevice->m_curParameterChanges++;
}

// loads an HLSL effect file
// if fromFile is true the shaderName should point to the full filename (with path) to the .fx file
// if fromFile is false the shaderName should be the resource name not the IDC_XX_YY value. Search vpinball_eng.rc for ".fx" to see an example
bool Shader::Load(const BYTE* shaderCodeName, UINT codeSize)
{
   LPD3DXBUFFER pBufferErrors;
   constexpr DWORD dwShaderFlags
      = 0; //D3DXSHADER_SKIPVALIDATION // these do not have a measurable effect so far (also if used in the offline fxc step): D3DXSHADER_PARTIALPRECISION, D3DXSHADER_PREFER_FLOW_CONTROL/D3DXSHADER_AVOID_FLOW_CONTROL
   HRESULT hr;
   /*
       if(fromFile)
       {
       dwShaderFlags = D3DXSHADER_DEBUG|D3DXSHADER_SKIPOPTIMIZATION;
       hr = D3DXCreateEffectFromFile(m_renderDevice->GetCoreDevice(),		// pDevice
       shaderName,			// pSrcFile
       nullptr,				// pDefines
       nullptr,				// pInclude
       dwShaderFlags,		// Flags
       nullptr,				// pPool
       &m_shader,			// ppEffect
       &pBufferErrors);		// ppCompilationErrors
       }
       else
       {
       hr = D3DXCreateEffectFromResource(m_renderDevice->GetCoreDevice(),		// pDevice
       nullptr,
       shaderName,			// resource name
       nullptr,				// pDefines
       nullptr,				// pInclude
       dwShaderFlags,		// Flags
       nullptr,				// pPool
       &m_shader,			// ppEffect
       &pBufferErrors);		// ppCompilationErrors

       }
       */
   hr = D3DXCreateEffect(m_renderDevice->GetCoreDevice(), shaderCodeName, codeSize, nullptr, nullptr, dwShaderFlags, nullptr, &m_shader, &pBufferErrors);
   if (FAILED(hr))
   {
      if (pBufferErrors)
      {
         const LPVOID pCompileErrors = pBufferErrors->GetBufferPointer();
         g_pvp->MessageBox((const char*)pCompileErrors, "Compile Error", MB_OK | MB_ICONEXCLAMATION);
      }
      else
         g_pvp->MessageBox("Unknown Error", "Compile Error", MB_OK | MB_ICONEXCLAMATION);

      return false;
   }
   return true;
}
