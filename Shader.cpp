#include "stdafx.h"
#include "Shader.h"
#include "typedefs3D.h"
#include "RenderDevice.h"

#ifdef ENABLE_SDL
#include <windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include "inc/robin_hood.h"
#include <regex>

static ShaderTechniques m_bound_technique = ShaderTechniques::SHADER_TECHNIQUE_INVALID;
#endif

#if DEBUG_LEVEL_LOG == 0
#define LOG(a,b,c)
#endif

#ifdef __OPENGLES__
#define FLT_MIN_VALUE 0.00006103515625
#else
#define FLT_MIN_VALUE 0.0000001
#endif

#define SHADER_TECHNIQUE(name) #name
const string Shader::shaderTechniqueNames[SHADER_TECHNIQUE_COUNT]
{
   SHADER_TECHNIQUE(RenderBall),
   SHADER_TECHNIQUE(RenderBall_DecalMode),
   SHADER_TECHNIQUE(RenderBall_CabMode),
   SHADER_TECHNIQUE(RenderBall_CabMode_DecalMode),
   SHADER_TECHNIQUE(RenderBallTrail),
   // OpenGL only has the first variant. DX9 needs all of them due to shader compiler limitation
   SHADER_TECHNIQUE(basic_with_texture),
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
   SHADER_TECHNIQUE(basic_without_texture),
   SHADER_TECHNIQUE(basic_without_texture_isMetal),
   SHADER_TECHNIQUE(basic_without_texture_refl),
   SHADER_TECHNIQUE(basic_without_texture_refl_isMetal),
   SHADER_TECHNIQUE(basic_without_texture_refr),
   SHADER_TECHNIQUE(basic_without_texture_refr_isMetal),
   SHADER_TECHNIQUE(basic_without_texture_refr_refl),
   SHADER_TECHNIQUE(basic_without_texture_refr_refl_isMetal),

   SHADER_TECHNIQUE(basic_refl_only_without_texture),
   SHADER_TECHNIQUE(basic_refl_only_with_texture),
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
   SHADER_TECHNIQUE(fb_blur_horiz7x7),
   SHADER_TECHNIQUE(fb_blur_vert7x7),
   SHADER_TECHNIQUE(fb_blur_horiz9x9),
   SHADER_TECHNIQUE(fb_blur_vert9x9),
   SHADER_TECHNIQUE(fb_blur_horiz11x11),
   SHADER_TECHNIQUE(fb_blur_vert11x11),
   SHADER_TECHNIQUE(fb_blur_horiz13x13),
   SHADER_TECHNIQUE(fb_blur_vert13x13),
   SHADER_TECHNIQUE(fb_blur_horiz15x15),
   SHADER_TECHNIQUE(fb_blur_vert15x15),
   SHADER_TECHNIQUE(fb_blur_horiz19x19),
   SHADER_TECHNIQUE(fb_blur_vert19x19),
   SHADER_TECHNIQUE(fb_blur_horiz23x23),
   SHADER_TECHNIQUE(fb_blur_vert23x23),
   SHADER_TECHNIQUE(fb_blur_horiz27x27),
   SHADER_TECHNIQUE(fb_blur_vert27x27),
   SHADER_TECHNIQUE(fb_blur_horiz39x39),
   SHADER_TECHNIQUE(fb_blur_vert39x39),
   SHADER_TECHNIQUE(fb_mirror),
   SHADER_TECHNIQUE(CAS),
   SHADER_TECHNIQUE(BilateralSharp_CAS),
   SHADER_TECHNIQUE(SSReflection),
   SHADER_TECHNIQUE(basic_noLight),
   SHADER_TECHNIQUE(bulb_light),
   SHADER_TECHNIQUE(bulb_light_with_ball_shadows),
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

string Shader::GetTechniqueName(ShaderTechniques technique)
{
   assert(0 <= technique && technique < SHADER_TECHNIQUE_COUNT);
   return shaderTechniqueNames[technique];
}


#define SHADER_UNIFORM(type, name) { type, #name, ""s, SA_UNDEFINED, SA_UNDEFINED, SF_UNDEFINED }
#define SHADER_SAMPLER(name, tex_name, default_clampu, default_clampv, default_filter) { SUT_Sampler, #name, #tex_name, default_clampu, default_clampv, default_filter }
Shader::ShaderUniform Shader::shaderUniformNames[SHADER_UNIFORM_COUNT] {
   // -- Matrices --
   SHADER_UNIFORM(SUT_DataBlock, matrixBlock), // OpenGL only, matrices as a float block
   SHADER_UNIFORM(SUT_Float4x4, matWorld), // DX9 only
   SHADER_UNIFORM(SUT_Float4x3, matView), // DX9 only
   SHADER_UNIFORM(SUT_Float4x4, matProj),
   SHADER_UNIFORM(SUT_Float4x4, matWorldViewProj), // DX9 only
   SHADER_UNIFORM(SUT_Float4x4, matWorldView), // DX9 only
   SHADER_UNIFORM(SUT_Float4x3, matWorldViewInverse), // DX9 only
   SHADER_UNIFORM(SUT_Float3x4, matWorldViewInverseTranspose), // DX9 only
   SHADER_UNIFORM(SUT_Float4x3, orientation),
   // -- Floats --
   SHADER_UNIFORM(SUT_Float, RenderBall),
   SHADER_UNIFORM(SUT_Float, blend_modulate_vs_add),
   SHADER_UNIFORM(SUT_Float, alphaTestValue),
   SHADER_UNIFORM(SUT_Float, eye),
   SHADER_UNIFORM(SUT_Float, fKickerScale),
   SHADER_UNIFORM(SUT_Float, fSceneScale), // OpenGL only (VR only)
   SHADER_UNIFORM(SUT_Float, mirrorFactor),
   SHADER_UNIFORM(SUT_Float, refractionThickness),
   // -- Vectors and Float Arrays --
   SHADER_UNIFORM(SUT_Float4, Roughness_WrapL_Edge_Thickness),
   SHADER_UNIFORM(SUT_Float4, cBase_Alpha),
   SHADER_UNIFORM(SUT_Float4, lightCenter_doShadow),
   SHADER_UNIFORM(SUT_Float4, lightCenter_maxRange),
   SHADER_UNIFORM(SUT_Float4, lightColor2_falloff_power),
   SHADER_UNIFORM(SUT_Float4, lightColor_intensity),
   SHADER_UNIFORM(SUT_Float2, fenvEmissionScale_TexWidth),
   SHADER_UNIFORM(SUT_Float4, invTableRes_playfield_height_reflection),
   SHADER_UNIFORM(SUT_Float4, cAmbient_LightRange),
   SHADER_UNIFORM(SUT_Float4, cClearcoat_EdgeAlpha),
   SHADER_UNIFORM(SUT_Float4, cGlossy_ImageLerp),
   SHADER_UNIFORM(SUT_Float3, refractionTint),
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
   SHADER_UNIFORM(SUT_Float3, mirrorNormal),
   SHADER_UNIFORM(SUT_Float4v, balls),
   SHADER_UNIFORM(SUT_Float4v, clip_planes), // OpenGL only
   SHADER_UNIFORM(SUT_Float4v, lightEmission), // OpenGL only
   SHADER_UNIFORM(SUT_Float4v, lightPos), // OpenGL only
   SHADER_UNIFORM(SUT_Float4v, packedLights), // DX9 only
   // -- Integer and Bool --
   SHADER_UNIFORM(SUT_Bool, ignoreStereo),
   SHADER_UNIFORM(SUT_Bool, disableLighting),
   SHADER_UNIFORM(SUT_Int, lightSources),
   SHADER_UNIFORM(SUT_Bool, color_grade),
   SHADER_UNIFORM(SUT_Bool, do_bloom),
   SHADER_UNIFORM(SUT_Bool, objectSpaceNormalMap),
   SHADER_UNIFORM(SUT_Bool, do_dither),
   SHADER_UNIFORM(SUT_Bool, doReflections),
   SHADER_UNIFORM(SUT_Bool, doRefractions),
   SHADER_UNIFORM(SUT_Bool, lightingOff),
   SHADER_UNIFORM(SUT_Bool, is_metal), // OpenGL only [managed by DirectX Effect framework on DirectX]
   SHADER_UNIFORM(SUT_Bool, doNormalMapping), // OpenGL only [managed by DirectX Effect framework on DirectX]
   // -- Samplers (a texture reference with sampling configuration) --
   // DMD shader
   SHADER_SAMPLER(tex_dmd, Texture0, SA_CLAMP, SA_CLAMP, SF_NONE), // DMD
   SHADER_SAMPLER(tex_sprite, Texture0, SA_MIRROR, SA_MIRROR, SF_TRILINEAR), // Sprite
   // Flasher shader
   SHADER_SAMPLER(tex_flasher_A, Texture0, SA_REPEAT, SA_REPEAT, SF_TRILINEAR), // base texture
   SHADER_SAMPLER(tex_flasher_B, Texture1, SA_REPEAT, SA_REPEAT, SF_TRILINEAR), // texB
   // FB shader
   SHADER_SAMPLER(tex_fb_unfiltered, Texture0, SA_CLAMP, SA_CLAMP, SF_NONE), // Framebuffer (unfiltered)
   SHADER_SAMPLER(tex_fb_filtered, Texture0, SA_CLAMP, SA_CLAMP, SF_BILINEAR), // Framebuffer (filtered)
   SHADER_SAMPLER(tex_bloom, Texture1, SA_CLAMP, SA_CLAMP, SF_BILINEAR), // Bloom
   SHADER_SAMPLER(tex_color_lut, Texture2, SA_CLAMP, SA_CLAMP, SF_BILINEAR), // Color grade LUT
   SHADER_SAMPLER(tex_ao, Texture3, SA_CLAMP, SA_CLAMP, SF_BILINEAR), // AO Result
   SHADER_SAMPLER(tex_depth, Texture4, SA_CLAMP, SA_CLAMP, SF_NONE), // Depth
   SHADER_SAMPLER(tex_ao_dither, Texture5, SA_REPEAT, SA_REPEAT, SF_NONE), // AO dither
   // Shared material samplers for Ball, Basic and Classic light shaders
   SHADER_SAMPLER(tex_env, Texture1, SA_REPEAT, SA_CLAMP, SF_TRILINEAR), // environment
   SHADER_SAMPLER(tex_diffuse_env, Texture2, SA_REPEAT, SA_CLAMP, SF_BILINEAR), // diffuse environment contribution/radiance
   // Ball shader
   SHADER_SAMPLER(tex_ball_color, Texture0, SA_REPEAT, SA_REPEAT, SF_TRILINEAR), // base texture
   SHADER_SAMPLER(tex_ball_playfield, Texture4, SA_CLAMP, SA_CLAMP, SF_TRILINEAR), // playfield
   SHADER_SAMPLER(tex_ball_decal, Texture3, SA_REPEAT, SA_REPEAT, SF_TRILINEAR), // ball decal
   // Basic shader
   SHADER_SAMPLER(tex_base_color, Texture0, SA_REPEAT, SA_REPEAT, SF_TRILINEAR), // base texture
   SHADER_SAMPLER(tex_base_transmission, Texture3, SA_CLAMP, SA_CLAMP, SF_BILINEAR), // bulb light/transmission buffer texture
   SHADER_SAMPLER(tex_base_normalmap, Texture4, SA_REPEAT, SA_REPEAT, SF_TRILINEAR), // normal map texture
   SHADER_SAMPLER(tex_reflection, Texture5, SA_CLAMP, SA_CLAMP, SF_BILINEAR), // plane reflection
   SHADER_SAMPLER(tex_refraction, Texture6, SA_CLAMP, SA_CLAMP, SF_BILINEAR), // screen space refraction
   SHADER_SAMPLER(tex_probe_depth, Texture7, SA_CLAMP, SA_CLAMP, SF_BILINEAR), // depth probe
   // Classic light shader
   SHADER_SAMPLER(tex_light_color, Texture0, SA_REPEAT, SA_REPEAT, SF_TRILINEAR), // base texture
   // Stereo shader (VPVR only, combine the 2 rendered eyes into a single one)
   SHADER_SAMPLER(tex_stereo_fb, Undefined, SA_CLAMP, SA_CLAMP, SF_NONE), // Framebuffer (unfiltered)
   // SMAA shader
   SHADER_SAMPLER(edgesTex, edgesTex2D, SA_CLAMP, SA_CLAMP, SF_TRILINEAR),
   SHADER_SAMPLER(blendTex, blendTex2D, SA_CLAMP, SA_CLAMP, SF_TRILINEAR),
   SHADER_SAMPLER(areaTex, areaTex2D, SA_CLAMP, SA_CLAMP, SF_TRILINEAR),
   SHADER_SAMPLER(searchTex, searchTex2D, SA_CLAMP, SA_CLAMP, SF_NONE),
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

   LOG(1, m_shaderCodeName, string("getAttributeByName Could not find attribute ").append(name).append(" in shaderAttributeNames."));
   return SHADER_ATTRIBUTE_INVALID;
}

Shader* Shader::current_shader = nullptr;
Shader* Shader::GetCurrentShader() { return current_shader;  }

Shader::Shader(RenderDevice* renderDevice)
   : currentMaterial(Material::MaterialType::BASIC, -FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX, 0xCCCCCCCC, 0xCCCCCCCC, 0xCCCCCCCC, false, -FLT_MAX, -FLT_MAX,
      -FLT_MAX, -FLT_MAX, 0xCCCCCCCC)
   , m_renderDevice(renderDevice)
{
   m_technique = SHADER_TECHNIQUE_INVALID;
   memset(m_uniformCache, 0, sizeof(UniformCache) * SHADER_UNIFORM_COUNT * (SHADER_TECHNIQUE_COUNT + 1));
   memset(m_isCacheValid, 0, sizeof(bool) * SHADER_TECHNIQUE_COUNT);
#ifdef ENABLE_SDL
   logFile = nullptr;
   memset(m_techniques, 0, sizeof(ShaderTechnique*) * SHADER_TECHNIQUE_COUNT);
#else
   m_shader = nullptr;
   memset(m_texture_cache, 0, sizeof(IDirect3DTexture9*) * TEXTURESET_STATE_CACHE_SIZE);
   memset(m_bound_texture, 0, sizeof(IDirect3DTexture9*) * TEXTURESET_STATE_CACHE_SIZE);
#endif
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
}

Shader::~Shader()
{
   for (int j = 0; j <= SHADER_TECHNIQUE_COUNT; ++j)
   {
      for (int i = 0; i < SHADER_UNIFORM_COUNT; ++i)
      {
         if (m_uniformCache[j][i].count > 0 && m_uniformCache[j][i].val.data)
            free(m_uniformCache[j][i].val.data);
      }
#ifdef ENABLE_SDL
      if (j < SHADER_TECHNIQUE_COUNT && m_techniques[j] != nullptr)
      {
         glDeleteProgram(m_techniques[j]->program);
         delete m_techniques[j];
         m_techniques[j] = nullptr;
      }
#endif
   }
#ifndef ENABLE_SDL
   SAFE_RELEASE(m_shader);
#endif
}

void Shader::Begin()
{
   assert(current_shader == nullptr);
   current_shader = this;
   assert(m_technique != SHADER_TECHNIQUE_INVALID);
   if (m_bound_technique != m_technique)
   {
      m_renderDevice->m_curTechniqueChanges++;
      m_bound_technique = m_technique;
#ifdef ENABLE_SDL
      glUseProgram(m_techniques[m_technique]->program);
#else
      CHECKD3D(m_shader->SetTechnique((D3DXHANDLE)shaderTechniqueNames[m_technique].c_str()));
#endif
   }
   for (auto uniformName : m_uniforms[m_technique])
      ApplyUniform(uniformName);
   m_isCacheValid[m_technique] = true;
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
      g_pplayer->m_pin3d.m_pd3dPrimaryDevice->SetRenderState(RenderState::ALPHABLENDENABLE, RenderState::RS_FALSE);
}

void Shader::SetDisableLighting(const float value) // only set top
{
   if (currentDisableLighting.x != value || currentDisableLighting.y != 0.f)
   {
      currentDisableLighting.x = value;
      currentDisableLighting.y = 0.f;
      currentDisableLighting.z = 0.f;
      currentDisableLighting.w = 0.f;
      SetVector(SHADER_fDisableLighting_top_below, &currentDisableLighting);
   }
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

void Shader::SetTechniqueMetal(ShaderTechniques technique, const Material& mat, const bool doNormalMapping, const bool doReflections, const bool doRefractions)
{
   const bool isMetal = mat.m_type == Material::MaterialType::METAL;
   ShaderTechniques tech = technique;
#ifdef ENABLE_SDL
   SetBool(SHADER_is_metal, isMetal);
   SetBool(SHADER_doNormalMapping, doNormalMapping);
   SetBool(SHADER_doReflections, doReflections);
   SetBool(SHADER_doRefractions, doRefractions);
#else
   switch (technique)
   {
   case SHADER_TECHNIQUE_basic_with_texture:
   {
      static ShaderTechniques tech_with_texture[16] = {
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
      };
      int idx = (isMetal ? 1 : 0) + (doNormalMapping ? 2 : 0) + (doReflections ? 4 : 0) + (doRefractions ? 8 : 0);
      tech = tech_with_texture[idx];
   }
   break;
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
   }
   break;
   case SHADER_TECHNIQUE_kickerBoolean: if (isMetal) tech = SHADER_TECHNIQUE_kickerBoolean_isMetal; break;
   case SHADER_TECHNIQUE_light_with_texture: if (isMetal) tech = SHADER_TECHNIQUE_light_with_texture_isMetal; break;
   case SHADER_TECHNIQUE_light_without_texture: if (isMetal) tech = SHADER_TECHNIQUE_light_without_texture_isMetal; break;
   }
#endif
   SetTechnique(tech);
}

void Shader::SetTechnique(ShaderTechniques technique)
{
   assert(current_shader != this); // Changing the technique of a used shader is not allowed (between Begin/End)
   assert(0 <= technique && technique < SHADER_TECHNIQUE_COUNT);
#ifdef ENABLE_SDL
   if (m_techniques[technique] == nullptr)
   {
      m_technique = SHADER_TECHNIQUE_INVALID;
      ShowError("Fatal Error: Could not find shader technique " + shaderTechniqueNames[technique]);
      exit(-1);
   }
#endif
   m_technique = technique;
}

uint32_t Shader::CopyUniformCache(const bool copyTo, const ShaderTechniques technique, UniformCache (&uniformCache)[SHADER_UNIFORM_COUNT], Sampler* (&textureCache)[TEXTURESET_STATE_CACHE_SIZE])
{
   UniformCache* src_cache = copyTo ? m_uniformCache[SHADER_TECHNIQUE_COUNT] : uniformCache;
   UniformCache* dst_cache = copyTo ? uniformCache : m_uniformCache[SHADER_TECHNIQUE_COUNT];
   unsigned long sampler_hash = 0L;
   for (auto uniformName : m_uniforms[technique])
   {
#ifdef ENABLE_SDL
      // For OpenGL uniform binding state is per technique (i.e. program)
      UniformDesc desc = m_techniques[technique]->uniform_desc[uniformName];
#else
      // For DX9 Effect framework uniform binding state is per shader, so we only use the first array
      UniformDesc desc = m_uniform_desc[uniformName];
      if (copyTo)
         memcpy(textureCache, m_texture_cache, TEXTURESET_STATE_CACHE_SIZE * sizeof(Sampler*));
      else
         memcpy(m_texture_cache, textureCache, TEXTURESET_STATE_CACHE_SIZE * sizeof(Sampler*));
#endif
      UniformCache* src = &(src_cache[uniformName]);
      UniformCache* dst = &(dst_cache[uniformName]);
      if (src->count > 0)
      {
         if (dst->count != src->count)
         {
            if (dst->count > 0)
               free(dst->val.data);
            dst->count = src->count;
            dst->val.data = malloc(dst->count);
         }
         memcpy(dst->val.data, src->val.data, src->count);
      }
      else
      {
         memcpy(&(dst->val), &(src->val), sizeof(UniformCache::UniformValue));
      }
      if (shaderUniformNames[uniformName].type == ShaderUniformType::SUT_Sampler)
         sampler_hash += (unsigned long) src->val.sampler;
   }
   return sampler_hash;
}

void Shader::SetMatrix(const ShaderUniforms hParameter, const Matrix3D* pMatrix)
{
   assert(0 <= hParameter && hParameter < SHADER_UNIFORM_COUNT);
   UniformCache* elem = &m_uniformCache[SHADER_TECHNIQUE_COUNT][hParameter];
   assert(elem->count == 0);
   memcpy(elem->val.fv, pMatrix->m, 16 * sizeof(float));
   ApplyUniform(hParameter);
}

void Shader::SetMatrix(const ShaderUniforms hParameter, const D3DXMATRIX* pMatrix)
{
   assert(0 <= hParameter && hParameter < SHADER_UNIFORM_COUNT);
   UniformCache* elem = &m_uniformCache[SHADER_TECHNIQUE_COUNT][hParameter];
   assert(elem->count == 0);
   memcpy(elem->val.fv, pMatrix->m, 16 * sizeof(float));
   ApplyUniform(hParameter);
}

void Shader::SetVector(const ShaderUniforms hParameter, const vec4* pVector)
{
   assert(0 <= hParameter && hParameter < SHADER_UNIFORM_COUNT);
   UniformCache* elem = &m_uniformCache[SHADER_TECHNIQUE_COUNT][hParameter];
   assert(elem->count == 0);
   memcpy(elem->val.fv, pVector, 4 * sizeof(float));
   ApplyUniform(hParameter);
}

void Shader::SetVector(const ShaderUniforms hParameter, const float x, const float y, const float z, const float w)
{
   assert(0 <= hParameter && hParameter < SHADER_UNIFORM_COUNT);
   UniformCache* elem = &m_uniformCache[SHADER_TECHNIQUE_COUNT][hParameter];
   assert(elem->count == 0);
   elem->val.fv[0] = x;
   elem->val.fv[1] = y;
   elem->val.fv[2] = z;
   elem->val.fv[3] = w;
   ApplyUniform(hParameter);
}

void Shader::SetFloat(const ShaderUniforms hParameter, const float f)
{
   assert(0 <= hParameter && hParameter < SHADER_UNIFORM_COUNT);
   UniformCache* elem = &m_uniformCache[SHADER_TECHNIQUE_COUNT][hParameter];
   assert(elem->count == 0);
   elem->val.f = f;
   ApplyUniform(hParameter);
}

void Shader::SetInt(const ShaderUniforms hParameter, const int i)
{
   assert(0 <= hParameter && hParameter < SHADER_UNIFORM_COUNT);
   UniformCache* elem = &m_uniformCache[SHADER_TECHNIQUE_COUNT][hParameter];
   assert(elem->count == 0);
   elem->val.i = i;
   ApplyUniform(hParameter);
}

void Shader::SetBool(const ShaderUniforms hParameter, const bool b)
{
   assert(0 <= hParameter && hParameter < SHADER_UNIFORM_COUNT);
   UniformCache* elem = &m_uniformCache[SHADER_TECHNIQUE_COUNT][hParameter];
   assert(elem->count == 0);
   elem->val.i = b ? 1 : 0;
   ApplyUniform(hParameter);
}

void Shader::SetFloat4v(const ShaderUniforms hParameter, const vec4* pData, const unsigned int count)
{
   assert(0 <= hParameter && hParameter < SHADER_UNIFORM_COUNT);
   UniformCache* elem = &m_uniformCache[SHADER_TECHNIQUE_COUNT][hParameter];
   if (elem->count == 0)
   {
      elem->count = count * 4 * sizeof(float);
      elem->val.data = malloc(elem->count);
   }
   memcpy(elem->val.data, pData, elem->count);
   ApplyUniform(hParameter);
}

void Shader::SetTextureNull(const ShaderUniforms texelName)
{
   SetTexture(texelName, (Sampler*)nullptr);
}

void Shader::SetTexture(const ShaderUniforms texelName, Texture* texel, const SamplerFilter filter, const SamplerAddressMode clampU, const SamplerAddressMode clampV, const bool force_linear_rgb)
{
   SetTexture(texelName, texel->m_pdsBuffer, filter, clampU, clampV, force_linear_rgb);
}

void Shader::SetTexture(const ShaderUniforms texelName, BaseTexture* texel, const SamplerFilter filter, const SamplerAddressMode clampU, const SamplerAddressMode clampV, const bool force_linear_rgb)
{
   if (!texel)
      SetTexture(texelName, (Sampler*)nullptr);
   else
      SetTexture(texelName, m_renderDevice->m_texMan.LoadTexture(texel, filter, clampU, clampV, force_linear_rgb));
}

void Shader::SetTexture(const ShaderUniforms uniformName, Sampler* texel)
{
#ifdef ENABLE_SDL
   m_uniformCache[SHADER_TECHNIQUE_COUNT][uniformName].val.sampler = texel;
   ApplyUniform(uniformName);
#else
   // Since DirectX effect framework manages the samplers, we only care about the texture here
   m_texture_cache[m_uniform_desc[uniformName].sampler] = texel;
#endif
}

void Shader::ApplyUniform(const ShaderUniforms uniformName)
{
   if (current_shader != this)
      return;
   bool isCacheInvalid = !m_isCacheValid[m_technique];
   UniformCache* src = &(m_uniformCache[SHADER_TECHNIQUE_COUNT][uniformName]);
#ifdef ENABLE_SDL
   // For OpenGL uniform binding state is per technique (i.e. program)
   auto desc = m_techniques[m_technique]->uniform_desc[uniformName];
   UniformCache* dst = &(m_uniformCache[m_technique][uniformName]);
   if (desc.location < 0)
      return;
#else
   // For DX9 Effect framework uniform binding state is per shader, so we only use the first array
   auto desc = m_uniform_desc[uniformName];
   UniformCache* dst = &(m_uniformCache[0][uniformName]);
#endif
   switch (desc.uniform.type)
   {
   case SUT_DataBlock: // Uniform blocks
      #ifdef ENABLE_SDL
      assert(src->count == 0 || src->count == desc.count);
      if (isCacheInvalid || dst->count != src->count || memcmp(src->val.data, dst->val.data, src->count) != 0)
      {
         if (dst->count == 0)
         {
            dst->count = src->count;
            dst->val.data = malloc(src->count);
         }
         memcpy(dst->val.data, src->val.data, src->count);
         glBindBuffer(GL_UNIFORM_BUFFER, desc.blockBuffer);
         glBufferData(GL_UNIFORM_BUFFER, src->count, src->val.data, GL_STREAM_DRAW);
         m_renderDevice->m_curParameterChanges++;
      }
      glUniformBlockBinding(m_techniques[m_technique]->program, desc.location, 0);
      glBindBufferRange(GL_UNIFORM_BUFFER, 0, desc.blockBuffer, 0, src->count);
      #else
      assert(false); // Unsupported on DX9
      #endif
      break;
   case SUT_Bool:
      assert(src->count == 0);
      if (isCacheInvalid || dst->val.i != src->val.i)
      {
         dst->val.i = src->val.i;
         #ifdef ENABLE_SDL
         glUniform1i(desc.location, src->val.i);
         #else
         CHECKD3D(m_shader->SetBool(desc.handle, src->val.i));
         #endif
         m_renderDevice->m_curParameterChanges++;
      }
      break;
   case SUT_Int:
      assert(src->count == 0);
      if (isCacheInvalid || dst->val.i != src->val.i)
      {
         dst->val.i = src->val.i;
         #ifdef ENABLE_SDL
         glUniform1i(desc.location, src->val.i);
         #else
         CHECKD3D(m_shader->SetInt(desc.handle, src->val.i));
         #endif
         m_renderDevice->m_curParameterChanges++;
      }
      break;
   case SUT_Float:
      assert(src->count == 0);
      if (isCacheInvalid || dst->val.f != src->val.f)
      {
         dst->val.f = src->val.f;
         #ifdef ENABLE_SDL
         glUniform1f(desc.location, src->val.f);
         #else
         CHECKD3D(m_shader->SetFloat(desc.handle, src->val.f));
         #endif
         m_renderDevice->m_curParameterChanges++;
      }
      break;
   case SUT_Float2:
      assert(src->count == 0);
      if (isCacheInvalid || memcmp(src->val.fv, dst->val.fv, 2 * sizeof(float)) != 0)
      {
         memcpy(dst->val.fv, src->val.fv, 2 * sizeof(float));
         #ifdef ENABLE_SDL
         glUniform2fv(desc.location, 1, src->val.fv);
         #else
         CHECKD3D(m_shader->SetVector(desc.handle, (D3DXVECTOR4*)src->val.fv));
         #endif
         m_renderDevice->m_curParameterChanges++;
      }
      break;
   case SUT_Float3:
      assert(src->count == 0);
      if (isCacheInvalid || memcmp(src->val.fv, dst->val.fv, 3 * sizeof(float)) != 0)
      {
         memcpy(dst->val.fv, src->val.fv, 3 * sizeof(float));
         #ifdef ENABLE_SDL
         glUniform3fv(desc.location, 1, src->val.fv);
         #else
         CHECKD3D(m_shader->SetVector(desc.handle, (D3DXVECTOR4*)src->val.fv));
         #endif
         m_renderDevice->m_curParameterChanges++;
      }
      break;
   case SUT_Float4:
      assert(src->count == 0);
      if (isCacheInvalid || memcmp(src->val.fv, dst->val.fv, 4 * sizeof(float)) != 0)
      {
         memcpy(dst->val.fv, src->val.fv, 4 * sizeof(float));
         #ifdef ENABLE_SDL
         glUniform4fv(desc.location, 1, src->val.fv);
         #else
         CHECKD3D(m_shader->SetVector(desc.handle, (D3DXVECTOR4*)src->val.fv));
         #endif
         m_renderDevice->m_curParameterChanges++;
      }
      break;
   case SUT_Float4v:
      assert(src->count == 0 || src->count == desc.count * 4 * sizeof(float));
      if ((src->count != 0) && (isCacheInvalid || dst->count == 0 || memcmp(src->val.data, dst->val.data, src->count) != 0))
      {
         if (dst->count == 0)
         {
            dst->count = src->count;
            dst->val.data = malloc(src->count);
         }
         memcpy(dst->val.data, src->val.data, src->count);
         #ifdef ENABLE_SDL
         glUniform4fv(desc.location, desc.count, (float*) src->val.data);
         #else
         CHECKD3D(m_shader->SetFloatArray(desc.handle, (float*) src->val.data, desc.count * 4));
         #endif
         m_renderDevice->m_curParameterChanges++;
      }
      break;
   case SUT_Float3x4:
   case SUT_Float4x3:
   case SUT_Float4x4:
      assert(src->count == 0);
      if (isCacheInvalid || memcmp(src->val.fv, dst->val.fv, 4 * 4 * sizeof(float)) != 0)
      {
         memcpy(dst->val.fv, src->val.fv, 4 * 4 * sizeof(float));
         #ifdef ENABLE_SDL
         glUniformMatrix4fv(desc.location, 1, GL_FALSE, src->val.fv);
         #else
         /*CHECKD3D(*/ m_shader->SetMatrix(desc.handle, (D3DXMATRIX*) src->val.fv) /*)*/; // leads to invalid calls when setting some of the matrices (as hlsl compiler optimizes some down to less than 4x4)
         #endif
         m_renderDevice->m_curParameterChanges++;
      }
      break;
#ifndef ENABLE_SDL
   case SUT_Sampler:
      {
         // A sampler bind performs 3 things:
         // - bind the texture to a texture stage (done by DirectX effect framework)
         // - adjust the sampling state (filter, wrapping, ...) of the choosen texture stage (partly done by DirectX effect framework which only applies the ones defined in the effect file)
         // - set the shader constant buffer to point to the selected texture stage (done by DirectX effect framework)
         // So, for DirectX, we simply fetch the Texture, DirectX will then use the texture for one or more samplers, applying there default states if any
         int unit = desc.sampler;
         assert(0 <= unit && unit < TEXTURESET_STATE_CACHE_SIZE);

         // Bind the texture to the shader
         Sampler* tex = m_texture_cache[unit];
         IDirect3DTexture9* const bounded = m_bound_texture[unit] ? m_bound_texture[unit]->GetCoreTexture() : nullptr;
         IDirect3DTexture9* const tobound = tex ? tex->GetCoreTexture() : nullptr;
         if (bounded != tobound)
         {
            CHECKD3D(m_shader->SetTexture(desc.tex_handle, tobound));
            m_bound_texture[unit] = tex;
            m_renderDevice->m_curTextureChanges++;
         }

         // Apply the texture sampling states
         if (tex)
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
      }
      break;
#else
   case SUT_Sampler:
      {
         // DX9 implementation uses preaffected texture units, not samplers, so these can not be used for OpenGL. This would cause some collisions.
         Sampler* texel = m_uniformCache[SHADER_TECHNIQUE_COUNT][uniformName].val.sampler;
         SamplerBinding* tex_unit = nullptr;
         if (texel == nullptr)
         { // For null texture, use OpenGL texture 0 which is a predefined texture that always returns (0, 0, 0, 1)
            for (auto binding : m_renderDevice->m_samplerBindings)
            {
               if (binding->sampler == nullptr)
               {
                  tex_unit = binding;
                  break;
               }
            }
            if (tex_unit == nullptr)
            {
               tex_unit = m_renderDevice->m_samplerBindings.back();
               if (tex_unit->sampler != nullptr)
                  tex_unit->sampler->m_bindings.erase(tex_unit);
               tex_unit->sampler = nullptr;
               glActiveTexture(GL_TEXTURE0 + tex_unit->unit);
               glBindTexture(GL_TEXTURE_2D, 0);
               m_renderDevice->m_curTextureChanges++;
            }
         }
         else
         {
            SamplerFilter filter = texel->GetFilter();
            SamplerAddressMode clampu = texel->GetClampU();
            SamplerAddressMode clampv = texel->GetClampV();
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
            for (auto binding : texel->m_bindings)
            {
               if (binding->filter == filter && binding->clamp_u == clampu && binding->clamp_v == clampv)
               {
                  tex_unit = binding;
                  break;
               }
            }
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
               glBindTexture(GL_TEXTURE_2D, texel->GetCoreTexture());
               m_renderDevice->m_curTextureChanges++;
               m_renderDevice->SetSamplerState(tex_unit->unit, filter, clampu, clampv);
            }
         }
         // Bind the sampler
         if (m_uniformCache[m_technique][uniformName].val.i != tex_unit->unit)
         {
            glUniform1i(desc.location, tex_unit->unit);
            m_renderDevice->m_curParameterChanges++;
            m_uniformCache[m_technique][uniformName].val.i = tex_unit->unit;
         }
         // Mark this texture unit as the last used one, and age all the others
         for (int i = tex_unit->use_rank - 1; i >= 0; i--)
         {
            m_renderDevice->m_samplerBindings[i]->use_rank++;
            m_renderDevice->m_samplerBindings[i + 1] = m_renderDevice->m_samplerBindings[i];
         }
         tex_unit->use_rank = 0;
         m_renderDevice->m_samplerBindings[0] = tex_unit;
         break;
      }
   default:
      {
         char msg[256];
         sprintf_s(msg, sizeof(msg), "Unknown uniform type 0x%0002X for %s in %s", desc.uniform.type, shaderUniformNames[uniformName].name.c_str(), m_techniques[m_technique]->name.c_str());
         ShowError(msg);
         break;
      }
#endif
   }
}

#if DEBUG_LEVEL_LOG > 0
void Shader::LOG(const int level, const string& fileNameRoot, const string& message) {
   if (level <= DEBUG_LEVEL_LOG) {
#ifdef ENABLE_SDL
      if (!logFile) {
         string name = Shader::shaderPath + "log" + PATH_SEPARATOR_CHAR + fileNameRoot + ".log";
         logFile = new std::ofstream();
bla:
         logFile->open(name);
         if (!logFile->is_open()) {
            const wstring wzMkPath = g_pvp->m_wzMyPath + L"shader";
            if (_wmkdir(wzMkPath.c_str()) != 0 || _wmkdir((wzMkPath + PATH_SEPARATOR_WCHAR + L"log").c_str()) != 0)
            {
               char msg[512];
               TCHAR full_path[MAX_PATH];
               GetFullPathName(_T(name.c_str()), MAX_PATH, full_path, nullptr);
               sprintf_s(msg, sizeof(msg), "Could not create logfile %s", full_path);
               ShowError(msg);
            }
            else
               goto bla;
         }
      }
      switch (level) {
      case 1:
         (*logFile) << "E:";
         break;
      case 2:
         (*logFile) << "W:";
         break;
      case 3:
         (*logFile) << "I:";
         break;
      default:
         (*logFile) << level << ':';
         break;
      }
      (*logFile) << message << '\n';
#endif
   }
}
#endif

#ifdef ENABLE_SDL
///////////////////////////////////////////////////////////////////////////////
// OpenGL specific implementation

string Shader::shaderPath;
string Shader::Defines;

//parse a file. Is called recursively for includes
bool Shader::parseFile(const string& fileNameRoot, const string& fileName, int level, robin_hood::unordered_map<string, string> &values, const string& parentMode) {
   if (level > 16) {//Can be increased, but looks very much like an infinite recursion.
      LOG(1, fileNameRoot, string("Reached more than 16 includes while trying to include ").append(fileName).append(" Aborting..."));
      return false;
   }
   if (level > 8) {
      LOG(2, fileNameRoot, string("Reached include level ").append(std::to_string(level)).append(" while trying to include ").append(fileName).append(" Check for recursion and try to avoid includes with includes."));
   }
   string currentMode = parentMode;
   robin_hood::unordered_map<string, string>::iterator currentElemIt = values.find(parentMode);
   string currentElement = (currentElemIt != values.end()) ? currentElemIt->second : string();
   std::ifstream glfxFile;
   glfxFile.open(Shader::shaderPath + fileName, std::ifstream::in);
   if (glfxFile.is_open())
   {
      string line;
      size_t linenumber = 0;
      while (getline(glfxFile, line))
      {
         linenumber++;
         if (line.compare(0, 4, "////") == 0) {
            string newMode = line.substr(4, line.length() - 4);
            if (newMode == "DEFINES") {
               currentElement.append("#define GLSL\n");
               currentElement.append(Shader::Defines).append("\n");
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
               LOG(1, fileNameRoot, fileName + "(" + std::to_string(linenumber) + "):" + line + " failed.");
            }
            currentElement = values[currentMode];
         }
         else {
            currentElement.append(line).append("\n");
         }
      }
      values[currentMode] = currentElement;
      glfxFile.close();
   }
   else {
      LOG(1, fileNameRoot, fileName + " not found.");
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
      LOG(1, fileNameRoot, string(shaderCodeName).append(": Vertex Shader compilation failed with: ").append(errorText));
      char msg[2048];
      sprintf_s(msg, sizeof(msg), "Fatal Error: Vertex Shader compilation of %s:%s failed!\n\n%s", fileNameRoot.c_str(), shaderCodeName.c_str(),errorText);
      ReportError(msg, -1, __FILE__, __LINE__);
      free(errorText);
      success = false;
   }
   //Geometry Shader
   if (success && geometry.length()>0) {
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
         LOG(1, fileNameRoot, string(shaderCodeName).append(": Geometry Shader compilation failed with: ").append(errorText));
         char msg[2048];
         sprintf_s(msg, sizeof(msg), "Fatal Error: Geometry Shader compilation of %s:%s failed!\n\n%s", fileNameRoot.c_str(), shaderCodeName.c_str(), errorText);
         ReportError(msg, -1, __FILE__, __LINE__);
         free(errorText);
         success = false;
      }
   }
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
         LOG(1, fileNameRoot, shaderCodeName + ": Fragment Shader compilation failed with: " + errorText);
         char msg[16384];
         sprintf_s(msg, sizeof(msg), "Fatal Error: Fragment Shader compilation of %s:%s failed!\n\n%s", fileNameRoot.c_str(), shaderCodeName.c_str(), errorText);
         ReportError(msg, -1, __FILE__, __LINE__);
         free(errorText);
         success = false;
      }
   }

   GLuint shaderprogram = 0;
   if (success) {
      shaderprogram = glCreateProgram();

      glAttachShader(shaderprogram, vertexShader);
      if (geometryShader>0) glAttachShader(shaderprogram, geometryShader);
      glAttachShader(shaderprogram, fragmentShader);

      glLinkProgram(shaderprogram);

      glGetProgramiv(shaderprogram, GL_LINK_STATUS, (int *)&result);
      if (result == FALSE)
      {
         GLint maxLength;
         glGetProgramiv(shaderprogram, GL_INFO_LOG_LENGTH, &maxLength);

         /* The maxLength includes the NULL character */
         char* errorText = (char *)malloc(maxLength);

         /* Notice that glGetProgramInfoLog, not glGetShaderInfoLog. */
         glGetProgramInfoLog(shaderprogram, maxLength, &maxLength, errorText);
         LOG(1, fileNameRoot, string(shaderCodeName).append(": Linking Shader failed with: ").append(errorText));
         free(errorText);
         success = false;
      }
   }

   if (GLAD_GL_VERSION_4_3)
   {
      string vs_name = shaderCodeName + ".VS"s;
      string gs_name = shaderCodeName + ".GS"s;
      string fs_name = shaderCodeName + ".FS"s;
      if (shaderprogram > 0)
         glObjectLabel(GL_PROGRAM, shaderprogram, shaderCodeName.length(), shaderCodeName.c_str());
      if (vertexShader > 0)
         glObjectLabel(GL_SHADER, vertexShader, vs_name.length(), vs_name.c_str());
      if (geometryShader > 0) 
         glObjectLabel(GL_SHADER, geometryShader, gs_name.length(), gs_name.c_str());
      if (fragmentShader > 0)
         glObjectLabel(GL_SHADER, fragmentShader, fs_name.length(), fs_name.c_str());
   }

   if ((WRITE_SHADER_FILES == 2) || ((WRITE_SHADER_FILES == 1) && !success))
   {
      std::ofstream shaderCode;
      const string szPath = Shader::shaderPath + "log" + PATH_SEPARATOR_CHAR + shaderCodeName;
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
      for (int i = 0; i < SHADER_ATTRIBUTE_COUNT; ++i)
         shader->attributeLocation[i] = { 0, -1, 0 };
      for (int i = 0; i < SHADER_UNIFORM_COUNT; ++i)
         shader->uniform_desc[i].location = -1;

      int count = 0;
      glGetProgramiv(shaderprogram, GL_ACTIVE_UNIFORMS, &count);
      char uniformName[256];
      for (int i = 0;i < count;++i) {
         GLenum type;
         GLint size;
         GLsizei length;
         glGetActiveUniform(shader->program, (GLuint)i, 256, &length, &size, &type, uniformName);
         GLint location = glGetUniformLocation(shader->program, uniformName);
         if (location >= 0 && size > 0) {
            // hack for packedLights, but works for all arrays
            for (int i2 = 0; i2 < length; i2++)
            {
               if (uniformName[i2] == '[') {
                  uniformName[i2] = 0;
                  break;
               }
            }
            auto uniformIndex = getUniformByName(uniformName);
            if (uniformIndex < SHADER_UNIFORM_COUNT)
            {
               m_uniforms[technique].push_back(uniformIndex);
               auto uniform = shaderUniformNames[uniformIndex];
               assert(uniform.type != SUT_Bool || type == GL_BOOL);
               assert(uniform.type != SUT_Int); // Unused so unimplemented
               assert(uniform.type != SUT_Float || type == GL_FLOAT);
               assert(uniform.type != SUT_Float2 || type == GL_FLOAT_VEC2);
               assert(uniform.type != SUT_Float3 || type == GL_FLOAT_VEC3);
               assert(uniform.type != SUT_Float4 || type == GL_FLOAT_VEC4);
               assert(uniform.type != SUT_Float4v || type == GL_FLOAT_VEC4);
               assert(uniform.type != SUT_Float3x4); // Unused so unimplemented
               assert(uniform.type != SUT_Float4x3 || type == GL_FLOAT_MAT4); // FIXME this should be GL_FLOAT_MAT4x3 or GL_FLOAT_MAT3x4 => fix orientation uniform in gl shader
               assert(uniform.type != SUT_Float4x4 || type == GL_FLOAT_MAT4);
               assert(uniform.type != SUT_DataBlock); // Unused so unimplemented
               assert(uniform.type != SUT_Sampler || type == GL_SAMPLER_2D);
               shader->uniform_desc[uniformIndex].uniform = uniform;
               shader->uniform_desc[uniformIndex].count = size;
               shader->uniform_desc[uniformIndex].location = location;
               if (shaderUniformNames[uniformIndex].type == SUT_Sampler)
               {
                  // FIXME this is wrong. After checking the specs, OpenGL sample a texture unit bound to texture #0 as (0, 0, 0, 1)
                  // Unlike DirectX, OpenGL won't return 0 if the texture is not bound to a black texture
                  // This will cause error for static pre-render which, done before bulb light transmission is evaluated and bound
                  SetTextureNull(uniformIndex);
               }
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
                  uniformName[i2] = 0;
                  break;
               }
            }
            auto uniformIndex = getUniformByName(uniformName);
            if (uniformIndex < SHADER_UNIFORM_COUNT)
            {
               auto uniform = shaderUniformNames[uniformIndex];
               assert(uniform.type == ShaderUniformType::SUT_DataBlock);
               shader->uniform_desc[uniformIndex].uniform = uniform;
               shader->uniform_desc[uniformIndex].count = size;
               shader->uniform_desc[uniformIndex].location = location;
               glGenBuffers(1, &shader->uniform_desc[uniformIndex].blockBuffer);
               m_uniforms[technique].push_back(uniformIndex);
            }
         }
      }

      glGetProgramiv(shaderprogram, GL_ACTIVE_ATTRIBUTES, &count);
      for (int i = 0; i < SHADER_ATTRIBUTE_COUNT; ++i)
         shader->attributeLocation[i] = { 0, -1, 0};
      for (int i = 0;i < count;++i) {
         GLenum type;
         int size;
         int length;
         char attributeName[256];
         glGetActiveAttrib(shader->program, (GLuint)i, 256, &length, &size, &type, attributeName);
         int location = glGetAttribLocation(shader->program, attributeName);
         if (location >= 0) {
            attributeLoc newLoc = {};
            newLoc.location = location;
            newLoc.type = type;
            switch (type) {
            case GL_FLOAT_VEC2:
               newLoc.size = 2 * size;
               break;
            case GL_FLOAT_VEC3:
               newLoc.size = 3 * size;
               break;
            case GL_FLOAT_VEC4:
               newLoc.size = 4 * size;
               break;
            default:
               newLoc.size = size;
               break;
            }
            auto index = getAttributeByName(attributeName);
            if (index < SHADER_ATTRIBUTE_COUNT) shader->attributeLocation[index] = newLoc;
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
      LOG(2, shaderCodeName, string("Invalid technique: ").append(_technique));
      return string();
   }
   const robin_hood::unordered_map<string, string>::const_iterator it = values.find(functionName.substr(0, start));
   string functionCode = (it != values.end()) ? it->second : string();
   if (end > start + 1) {
      std::stringstream params(functionName.substr(start + 1, end - start - 1));
      string param;
      int paramID = 0;
      while (std::getline(params, param, ',')) {
         functionCode = std::regex_replace(functionCode, std::regex(string("%PARAM").append(std::to_string(paramID)).append("%")), param);
         paramID++;
      }
   }
   return functionCode;
}

bool Shader::Load(const std::string& name, const BYTE* code, unsigned int codeSize)
{
   m_shaderCodeName = name;
   LOG(3, m_shaderCodeName, "Start parsing file");
   robin_hood::unordered_map<string, string> values;
   const bool parsing = parseFile(m_shaderCodeName, m_shaderCodeName, 0, values, "GLOBAL");
   if (!parsing) {
      LOG(1, m_shaderCodeName, "Parsing failed");
      char msg[128];
      sprintf_s(msg, sizeof(msg), "Fatal Error: Shader parsing of %s failed!", m_shaderCodeName.c_str());
      ReportError(msg, -1, __FILE__, __LINE__);
      if (logFile)
      {
         logFile->close();
         delete logFile;
         logFile = nullptr;
      }
      return false;
   }
   else {
      LOG(3, m_shaderCodeName, "Parsing successful. Start compiling shaders");
   }
   robin_hood::unordered_map<string, string>::iterator it = values.find("GLOBAL");
   string global = (it != values.end()) ? it->second : string();

   it = values.find("VERTEX");
   string vertex = global;
   vertex.append((it != values.end()) ? it->second : string());

   it = values.find("GEOMETRY");
   string geometry = global;
   geometry.append((it != values.end()) ? it->second : string());

   it = values.find("FRAGMENT");
   string fragment = global;
   fragment.append((it != values.end()) ? it->second : string());

   it = values.find("TECHNIQUES");
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
               LOG(3, m_shaderCodeName, string("Unexpected technique skipped: ").append(element[0]));
            }
            else
            {
               string vertexShaderCode = vertex;
               vertexShaderCode.append("\n//").append(_technique).append("\n//").append(element[2]).append("\n");
               vertexShaderCode.append(analyzeFunction(m_shaderCodeName, _technique, element[2], values)).append("\0");
               string geometryShaderCode;
               if (elem == 5 && element[3].length() > 0)
               {
                  geometryShaderCode = geometry;
                  geometryShaderCode.append("\n//").append(_technique).append("\n//").append(element[3]).append("\n");
                  geometryShaderCode.append(analyzeFunction(m_shaderCodeName, _technique, element[3], values)).append("\0");
               }
               string fragmentShaderCode = fragment;
               fragmentShaderCode.append("\n//").append(_technique).append("\n//").append(element[elem - 1]).append("\n");
               fragmentShaderCode.append(analyzeFunction(m_shaderCodeName, _technique, element[elem - 1], values)).append("\0");
               ShaderTechnique* build = compileGLShader(technique, m_shaderCodeName, element[0] /*.append("_").append(element[1])*/, vertexShaderCode, geometryShaderCode, fragmentShaderCode);
               if (build != nullptr)
               {
                  m_techniques[technique] = build;
                  tecCount++;
               }
               else
               {
                  char msg[128];
                  sprintf_s(msg, sizeof(msg), "Fatal Error: Shader compilation failed for %s!", m_shaderCodeName.c_str());
                  ReportError(msg, -1, __FILE__, __LINE__);
                  if (logFile)
                  {
                     logFile->close();
                     delete logFile;
                     logFile = nullptr;
                  }
                  return false;
               }
            }
         }
      }
      LOG(3, m_shaderCodeName, string("Compiled successfully ").append(std::to_string(tecCount)).append(" shaders."));
   }
   else {
      LOG(1, m_shaderCodeName, "No techniques found.");
      char msg[128];
      sprintf_s(msg, sizeof(msg), "Fatal Error: No shader techniques found in %s!", m_shaderCodeName.c_str());
      ReportError(msg, -1, __FILE__, __LINE__);
      if (logFile)
      {
         logFile->close();
         delete logFile;
         logFile = nullptr;
      }
      return false;
   }

   if (logFile)
   {
      logFile->close();
      delete logFile;
      logFile = nullptr;
   }

   //Set default values from Material.fxh for uniforms.
   SetVector(SHADER_cBase_Alpha, 0.5f, 0.5f, 0.5f, 1.0f);
   SetVector(SHADER_Roughness_WrapL_Edge_Thickness, 4.0f, 0.5f, 1.0f, 0.05f);
   return true;
}

void Shader::setAttributeFormat(DWORD fvf)
{
   if (m_technique == SHADER_TECHNIQUE_INVALID)
      return;
   for (int i = 0; i < SHADER_ATTRIBUTE_COUNT; ++i)
   {
      const int location = m_techniques[m_technique]->attributeLocation[i].location;
      if (location >= 0) {
         size_t offset;
         glEnableVertexAttribArray(location);
         switch (i) {
         case SHADER_ATTRIBUTE_POS:
            offset = 0;
            break;
         case SHADER_ATTRIBUTE_NORM:
            offset = 12;
            break;
         case SHADER_ATTRIBUTE_TC:
         case SHADER_ATTRIBUTE_TEX:
            offset = (fvf == VertexFormat::VF_POS_TEX) ? 12 : 24;
            break;
         default:
            ReportError("Unknown Attribute", 666, __FILE__, __LINE__);
            offset = 0;
            break;
         }
         glVertexAttribPointer(location, m_techniques[m_technique]->attributeLocation[i].size, GL_FLOAT, GL_FALSE, (fvf == VertexFormat::VF_POS_TEX) ? 20 : 32, (void*)offset);
      }
   }
}

void Shader::SetUniformBlock(const ShaderUniforms hParameter, const float* pMatrix, const size_t size)
{
   if (hParameter >= SHADER_UNIFORM_COUNT) return;
   UniformCache* elem = &m_uniformCache[SHADER_TECHNIQUE_COUNT][hParameter];
   if (elem->count == 0)
   {
      elem->count = size;
      elem->val.data = malloc(size);
   }
   memcpy(elem->val.data, pMatrix, size);
   ApplyUniform(hParameter);
}

#else

///////////////////////////////////////////////////////////////////////////////
// DirectX 9 specific implementation



// loads an HLSL effect file
// if fromFile is true the shaderName should point to the full filename (with path) to the .fx file
// if fromFile is false the shaderName should be the resource name not the IDC_XX_YY value. Search vpinball_eng.rc for ".fx" to see an example
bool Shader::Load(const std::string& name, const BYTE* code, unsigned int codeSize)
{
   m_shaderCodeName = name;
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
   hr = D3DXCreateEffect(m_renderDevice->GetCoreDevice(), code, codeSize, nullptr, nullptr, dwShaderFlags, nullptr, &m_shader, &pBufferErrors);
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

   // Collect the list of uniforms and there informations (handle, type,...)
   D3DXEFFECT_DESC effect_desc;
   m_shader->GetDesc(&effect_desc);
   int texture_mask = 0;
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
         OutputDebugString("Unsupported uniform type for: "s.append(param_desc.Name).append("\n"s).c_str());
         continue;
      }
      ShaderUniforms uniformIndex = getUniformByName(param_desc.Name);
      if (uniformIndex == SHADER_UNIFORM_INVALID)
      {
         OutputDebugString("Missing uniform: "s.append(param_desc.Name).append("\n"s).c_str());
         continue;
      }
      else
      {
         auto uniform = shaderUniformNames[uniformIndex];
         assert(uniform.type == type);
         m_uniform_desc[uniformIndex].uniform = uniform;
         m_uniform_desc[uniformIndex].handle = parameter;
         m_uniform_desc[uniformIndex].tex_handle = nullptr;
         m_uniform_desc[uniformIndex].count = count;
         m_uniform_desc[uniformIndex].sampler = -1;
         if (type == ShaderUniformType::SUT_Sampler)
         {
            m_uniform_desc[uniformIndex].tex_handle = m_shader->GetParameterByName(NULL, shaderUniformNames[uniformIndex].tex_name.c_str());
            if (param_desc.Semantic != nullptr && std::string(param_desc.Semantic).rfind("TEXUNIT"s, 0) == 0)
            {
               int unit = param_desc.Semantic[strlen(param_desc.Semantic) - 1] - '0';
               m_uniform_desc[uniformIndex].sampler = unit;
               // Since we only manages the texture state and not the sampler ones, only add one of the samplers bound to a given texture unit to avoid useless calls
               if ((texture_mask & (1 << unit)) != 0) continue;
               texture_mask |= 1 << unit;
            }
         }
         // TODO we do not filter on technique for DX9. Not a big problem, but not that clean either (all uniforms are applied for all techniques)
         for (int j = 0; j < SHADER_TECHNIQUE_COUNT; j++)
            m_uniforms[j].push_back(uniformIndex);
      }
   }
   return true;
}

#endif