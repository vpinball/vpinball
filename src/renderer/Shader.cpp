#include "stdafx.h"
#include "Shader.h"
#include "typedefs3D.h"
#include "RenderDevice.h"

#include <plog/Log.h>
#include <plog/Initializers/RollingFileInitializer.h>

#include "vpversion.h"

#ifdef ENABLE_SDL
#include <windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include "robin_hood.h"

ShaderTechniques Shader::m_boundTechnique = ShaderTechniques::SHADER_TECHNIQUE_INVALID;

#ifdef __STANDALONE__
#include <sstream>
#endif

#endif

#define SHADER_TECHNIQUE(name) #name
const string Shader::shaderTechniqueNames[SHADER_TECHNIQUE_COUNT]
{
   SHADER_TECHNIQUE(RenderBall),
   SHADER_TECHNIQUE(RenderBall_DecalMode),
   SHADER_TECHNIQUE(RenderBall_SphericalMap),
   SHADER_TECHNIQUE(RenderBall_SphericalMap_DecalMode),
   SHADER_TECHNIQUE(RenderBall_Debug),
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
   SHADER_TECHNIQUE(basic_with_texture_at),
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
   SHADER_TECHNIQUE(basic_without_texture),
   SHADER_TECHNIQUE(basic_without_texture_isMetal),
   SHADER_TECHNIQUE(basic_without_texture_refl),
   SHADER_TECHNIQUE(basic_without_texture_refl_isMetal),
   SHADER_TECHNIQUE(basic_without_texture_refr),
   SHADER_TECHNIQUE(basic_without_texture_refr_isMetal),
   SHADER_TECHNIQUE(basic_without_texture_refr_refl),
   SHADER_TECHNIQUE(basic_without_texture_refr_refl_isMetal),
   // Unshaded
   SHADER_TECHNIQUE(unshaded_without_texture),
   SHADER_TECHNIQUE(unshaded_with_texture),
   SHADER_TECHNIQUE(unshaded_without_texture_shadow),
   SHADER_TECHNIQUE(unshaded_with_texture_shadow),

   SHADER_TECHNIQUE(basic_reflection_only),
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
   SHADER_TECHNIQUE(fb_bloom),
   SHADER_TECHNIQUE(fb_AO), // Display debug AO
   SHADER_TECHNIQUE(fb_AO_static), // Apply AO during static prerender pass (no tonemapping)
   SHADER_TECHNIQUE(fb_AO_no_filter_static), // Apply AO during static prerender pass (no tonemapping)
   SHADER_TECHNIQUE(fb_rhtonemap),
   SHADER_TECHNIQUE(fb_rhtonemap_AO),
   SHADER_TECHNIQUE(fb_rhtonemap_no_filter),
   SHADER_TECHNIQUE(fb_rhtonemap_AO_no_filter),
   SHADER_TECHNIQUE(fb_tmtonemap),
   SHADER_TECHNIQUE(fb_tmtonemap_AO),
   SHADER_TECHNIQUE(fb_tmtonemap_no_filter),
   SHADER_TECHNIQUE(fb_tmtonemap_AO_no_filter),
   SHADER_TECHNIQUE(fb_fmtonemap),
   SHADER_TECHNIQUE(fb_fmtonemap_AO),
   SHADER_TECHNIQUE(fb_fmtonemap_no_filter),
   SHADER_TECHNIQUE(fb_fmtonemap_AO_no_filter),
   SHADER_TECHNIQUE(fb_rhtonemap_no_filterRG),
   SHADER_TECHNIQUE(fb_rhtonemap_no_filterR),
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
   SHADER_TECHNIQUE(fb_copy),
   SHADER_TECHNIQUE(CAS),
   SHADER_TECHNIQUE(BilateralSharp_CAS),
   SHADER_TECHNIQUE(SSReflection),
   SHADER_TECHNIQUE(basic_noLight),
   SHADER_TECHNIQUE(bulb_light),
   SHADER_TECHNIQUE(bulb_light_with_ball_shadows),
#ifndef __OPENGLES__
   SHADER_TECHNIQUE(SMAA_ColorEdgeDetection),
   SHADER_TECHNIQUE(SMAA_BlendWeightCalculation),
   SHADER_TECHNIQUE(SMAA_NeighborhoodBlending),
#endif
   SHADER_TECHNIQUE(stereo_SBS),
   SHADER_TECHNIQUE(stereo_TB),
   SHADER_TECHNIQUE(stereo_Int),
   SHADER_TECHNIQUE(stereo_Flipped_Int),
   SHADER_TECHNIQUE(Stereo_sRGBAnaglyph),
   SHADER_TECHNIQUE(Stereo_GammaAnaglyph),
   SHADER_TECHNIQUE(Stereo_sRGBDynDesatAnaglyph),
   SHADER_TECHNIQUE(Stereo_GammaDynDesatAnaglyph),
   SHADER_TECHNIQUE(Stereo_DeghostAnaglyph),
   SHADER_TECHNIQUE(irradiance),
};
#undef SHADER_TECHNIQUE

ShaderTechniques Shader::getTechniqueByName(const string& name)
{
   for (int i = 0; i < SHADER_TECHNIQUE_COUNT; ++i)
      if (name == shaderTechniqueNames[i])
         return ShaderTechniques(i);
   PLOGE << "getTechniqueByName Could not find technique " << name << " in shaderTechniqueNames.";
   return SHADER_TECHNIQUE_INVALID;
}

string Shader::GetTechniqueName(ShaderTechniques technique)
{
   assert(0 <= technique && technique < SHADER_TECHNIQUE_COUNT);
   return shaderTechniqueNames[technique];
}


#define SHADER_UNIFORM(type, name, count) { type, #name, count, string(), SA_UNDEFINED, SA_UNDEFINED, SF_UNDEFINED }
#define SHADER_SAMPLER(name, tex_name, default_clampu, default_clampv, default_filter) { SUT_Sampler, #name, 1, #tex_name, default_clampu, default_clampv, default_filter }
Shader::ShaderUniform Shader::shaderUniformNames[SHADER_UNIFORM_COUNT] {
   // Shared uniforms
   SHADER_UNIFORM(SUT_Int, layer, 1),
   SHADER_UNIFORM(SUT_Float, alphaTestValue, 1),
   SHADER_UNIFORM(SUT_Float4x4, matProj, 1), // +1 Matrix for stereo
   SHADER_UNIFORM(SUT_Float4x4, matWorldViewProj, 1), // +1 Matrix for stereo
   #ifdef ENABLE_SDL // OpenGL
   SHADER_UNIFORM(SUT_DataBlock, basicMatrixBlock, 5 * 16 * 4), // +1 Matrix for stereo
   SHADER_UNIFORM(SUT_DataBlock, ballMatrixBlock, 4 * 16 * 4), // +1 Matrix for stereo
   #else // DirectX 9
   SHADER_UNIFORM(SUT_Float4x4, matWorld, 1),
   SHADER_UNIFORM(SUT_Float4x3, matView, 1),
   SHADER_UNIFORM(SUT_Float4x4, matWorldView, 1),
   SHADER_UNIFORM(SUT_Float4x3, matWorldViewInverse, 1),
   SHADER_UNIFORM(SUT_Float3x4, matWorldViewInverseTranspose, 1),
   #endif
   SHADER_UNIFORM(SUT_Float4, lightCenter_doShadow, 1), // Basic & Flasher (for ball shadows)
   SHADER_UNIFORM(SUT_Float4v, balls, 8), // Basic & Flasher (for ball shadows)
   SHADER_UNIFORM(SUT_Float4, staticColor_Alpha, 1), // Basic & Flasher
   SHADER_UNIFORM(SUT_Float4, w_h_height, 1), // Post process & Basic (for screen space reflection/refraction)

   // Shared material for Ball, Basic and Classic light shaders
   #ifdef ENABLE_SDL // OpenGL
   SHADER_UNIFORM(SUT_Float4, clip_plane, 1),
   SHADER_UNIFORM(SUT_Float4v, basicLightEmission, 2),
   SHADER_UNIFORM(SUT_Float4v, basicLightPos, 2),
   SHADER_UNIFORM(SUT_Float4v, ballLightEmission, 10),
   SHADER_UNIFORM(SUT_Float4v, ballLightPos, 10),
   SHADER_UNIFORM(SUT_Bool, is_metal, 1), // OpenGL only [managed by DirectX Effect framework on DirectX]
   SHADER_UNIFORM(SUT_Bool, doNormalMapping, 1), // OpenGL only [managed by DirectX Effect framework on DirectX]
   #else // DirectX 9
   SHADER_UNIFORM(SUT_Float4v, basicPackedLights, 3),
   SHADER_UNIFORM(SUT_Float4v, ballPackedLights, 15),
   #endif
   SHADER_UNIFORM(SUT_Float4, Roughness_WrapL_Edge_Thickness, 1),
   SHADER_UNIFORM(SUT_Float4, cBase_Alpha, 1),
   SHADER_UNIFORM(SUT_Float2, fDisableLighting_top_below, 1),
   SHADER_UNIFORM(SUT_Float2, fenvEmissionScale_TexWidth, 1),
   SHADER_UNIFORM(SUT_Float4, cAmbient_LightRange, 1),
   SHADER_SAMPLER(tex_env, Texture1, SA_REPEAT, SA_CLAMP, SF_TRILINEAR), // environment
   SHADER_SAMPLER(tex_diffuse_env, Texture2, SA_REPEAT, SA_CLAMP, SF_BILINEAR), // diffuse environment contribution/radiance

   // Basic Shader
   SHADER_UNIFORM(SUT_Float4, cClearcoat_EdgeAlpha, 1),
   SHADER_UNIFORM(SUT_Float4, cGlossy_ImageLerp, 1),
   #ifdef ENABLE_SDL // OpenGL
   SHADER_UNIFORM(SUT_Bool, doRefractions, 1),
   #endif
   SHADER_UNIFORM(SUT_Float4, refractionTint_thickness, 1),
   SHADER_UNIFORM(SUT_Float4, mirrorNormal_factor, 1),
   SHADER_UNIFORM(SUT_Bool, objectSpaceNormalMap, 1),
   SHADER_SAMPLER(tex_base_color, Texture0, SA_CLAMP, SA_CLAMP, SF_TRILINEAR), // base texture
   SHADER_SAMPLER(tex_base_transmission, Texture3, SA_CLAMP, SA_CLAMP, SF_BILINEAR), // bulb light/transmission buffer texture
   SHADER_SAMPLER(tex_base_normalmap, Texture4, SA_REPEAT, SA_REPEAT, SF_TRILINEAR), // normal map texture
   SHADER_SAMPLER(tex_reflection, Texture5, SA_CLAMP, SA_CLAMP, SF_BILINEAR), // plane reflection
   SHADER_SAMPLER(tex_refraction, Texture6, SA_CLAMP, SA_CLAMP, SF_BILINEAR), // screen space refraction
   SHADER_SAMPLER(tex_probe_depth, Texture7, SA_CLAMP, SA_CLAMP, SF_BILINEAR), // depth probe

   // Ball Shader
   SHADER_UNIFORM(SUT_Float4x3, orientation, 1),
   SHADER_UNIFORM(SUT_Float4, invTableRes_reflection, 1),
   SHADER_UNIFORM(SUT_Float4, w_h_disableLighting, 1),
   SHADER_SAMPLER(tex_ball_color, Texture0, SA_REPEAT, SA_REPEAT, SF_TRILINEAR), // base texture
   SHADER_SAMPLER(tex_ball_playfield, Texture4, SA_CLAMP, SA_CLAMP, SF_TRILINEAR), // playfield
   SHADER_SAMPLER(tex_ball_decal, Texture3, SA_REPEAT, SA_REPEAT, SF_TRILINEAR), // ball decal

   // Light Shader
   SHADER_UNIFORM(SUT_Float, blend_modulate_vs_add, 1),
   SHADER_UNIFORM(SUT_Float4, lightCenter_maxRange, 1), // Classic and Halo
   SHADER_UNIFORM(SUT_Float4, lightColor2_falloff_power, 1), // Classic and Halo
   SHADER_UNIFORM(SUT_Float4, lightColor_intensity, 1), // Classic and Halo
   SHADER_UNIFORM(SUT_Bool, lightingOff, 1), // Classic only
   SHADER_SAMPLER(tex_light_color, Texture0, SA_REPEAT, SA_REPEAT, SF_TRILINEAR), // Classic only

   // DMD Shader
   SHADER_UNIFORM(SUT_Float4, vRes_Alpha_time, 1),
   SHADER_UNIFORM(SUT_Float4, backBoxSize, 1),
   SHADER_UNIFORM(SUT_Float4, vColor_Intensity, 1),
   SHADER_SAMPLER(tex_dmd, Texture0, SA_CLAMP, SA_CLAMP, SF_NONE), // DMD
   SHADER_SAMPLER(tex_sprite, Texture0, SA_MIRROR, SA_MIRROR, SF_TRILINEAR), // Sprite

   // Flasher Shader
   SHADER_UNIFORM(SUT_Float4, alphaTestValueAB_filterMode_addBlend, 1),
   SHADER_UNIFORM(SUT_Float3, amount_blend_modulate_vs_add_flasherMode, 1),
   SHADER_SAMPLER(tex_flasher_A, Texture0, SA_CLAMP, SA_CLAMP, SF_TRILINEAR), // base texture
   SHADER_SAMPLER(tex_flasher_B, Texture1, SA_REPEAT, SA_REPEAT, SF_TRILINEAR), // texB

   // Post Process Shader
   SHADER_UNIFORM(SUT_Bool, color_grade, 1),
   SHADER_UNIFORM(SUT_Bool, do_bloom, 1),
   SHADER_UNIFORM(SUT_Bool, do_dither, 1),
   SHADER_UNIFORM(SUT_Float4, SSR_bumpHeight_fresnelRefl_scale_FS, 1),
   SHADER_UNIFORM(SUT_Float2, AO_scale_timeblur, 1),
   SHADER_SAMPLER(tex_fb_unfiltered, Texture0, SA_CLAMP, SA_CLAMP, SF_NONE), // Framebuffer (unfiltered)
   SHADER_SAMPLER(tex_fb_filtered, Texture0, SA_CLAMP, SA_CLAMP, SF_BILINEAR), // Framebuffer (filtered)
   SHADER_SAMPLER(tex_bloom, Texture1, SA_CLAMP, SA_CLAMP, SF_BILINEAR), // Bloom
   SHADER_SAMPLER(tex_color_lut, Texture2, SA_CLAMP, SA_CLAMP, SF_BILINEAR), // Color grade LUT
   SHADER_SAMPLER(tex_ao, Texture3, SA_CLAMP, SA_CLAMP, SF_BILINEAR), // AO Result
   SHADER_SAMPLER(tex_depth, Texture4, SA_CLAMP, SA_CLAMP, SF_NONE), // Depth
   SHADER_SAMPLER(tex_ao_dither, Texture5, SA_REPEAT, SA_REPEAT, SF_NONE), // AO dither
   SHADER_SAMPLER(edgesTex, edgesTex2D, SA_CLAMP, SA_CLAMP, SF_TRILINEAR), // SMAA (TexUnit 1, not conflicting with bloom since they are used in different shaders)
   SHADER_SAMPLER(blendTex, blendTex2D, SA_CLAMP, SA_CLAMP, SF_TRILINEAR), // SMAA (TexUnit 7)
   SHADER_SAMPLER(areaTex, areaTex2D, SA_CLAMP, SA_CLAMP, SF_BILINEAR), // SMAA (TexUnit 8)
   SHADER_SAMPLER(searchTex, searchTex2D, SA_CLAMP, SA_CLAMP, SF_NONE), // SMAA (TexUnit 9)

   // Stereo Shader
   SHADER_SAMPLER(tex_stereo_fb, Texture0, SA_REPEAT, SA_REPEAT, SF_NONE), // Framebuffer (unfiltered)
   SHADER_SAMPLER(tex_stereo_depth, Texture4, SA_REPEAT, SA_REPEAT, SF_NONE), // Depth
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

Shader::Shader(RenderDevice* renderDevice, const std::string& src1, const std::string& src2, const BYTE* code, unsigned int codeSize):
   m_renderDevice(renderDevice)
{
   #ifdef ENABLE_SDL
   if (renderDevice->m_stereo3D != STEREO_OFF)
   {
      shaderUniformNames[SHADER_matProj].count = 2;
      shaderUniformNames[SHADER_matWorldViewProj].count = 2;
      shaderUniformNames[SHADER_basicMatrixBlock].count = 6 * 16 * 4;
      shaderUniformNames[SHADER_ballMatrixBlock].count = 5 * 16 * 4;
   }
   else
   {
      shaderUniformNames[SHADER_matProj].count = 1;
      shaderUniformNames[SHADER_matWorldViewProj].count = 1;
      shaderUniformNames[SHADER_basicMatrixBlock].count = 5 * 16 * 4;
      shaderUniformNames[SHADER_ballMatrixBlock].count = 4 * 16 * 4;
   }
   #endif

   m_technique = SHADER_TECHNIQUE_INVALID;
   #ifdef ENABLE_SDL // OpenGL
   memset(m_techniques, 0, sizeof(ShaderTechnique*) * SHADER_TECHNIQUE_COUNT);
   #else // DirectX 9
   memset(m_boundTexture, 0, sizeof(IDirect3DTexture9*) * TEXTURESET_STATE_CACHE_SIZE);
   #endif

#ifndef __STANDALONE__
   Load(src1, code, codeSize);
   if (!src2.empty()) // Additional source file used by OpenGL (TODO we should rmeove this and merge at the shader file level)
      Load(src2, nullptr, 0);
#else
   if (!Load(src1, code, codeSize))
      exit(-1);
   if (src2 != ""s) // Additional source file used by OpenGL (TODO we should rmeove this and merge at the shader file level)
      if (!Load(src2, nullptr, 0))
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
   m_state = new ShaderState(this);
   memset(m_state->m_state, 0, m_stateSize);
   #ifdef ENABLE_SDL // OpenGL
   for (int i = 0; i < SHADER_TECHNIQUE_COUNT; i++)
      if (m_techniques[i] != nullptr)
      {
         m_boundState[i] = new ShaderState(this);
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
   //Set default values from Material.fxh for uniforms.
   if (m_stateOffsets[SHADER_cBase_Alpha] != -1)
      SetVector(SHADER_cBase_Alpha, 0.5f, 0.5f, 0.5f, 1.0f);
   if (m_stateOffsets[SHADER_Roughness_WrapL_Edge_Thickness] != -1)
      SetVector(SHADER_Roughness_WrapL_Edge_Thickness, 4.0f, 0.5f, 1.0f, 0.05f);
   #else // DirectX 9
   m_boundState = new ShaderState(this);
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
#ifdef ENABLE_SDL // OpenGL
   for (int j = 0; j < SHADER_TECHNIQUE_COUNT; ++j)
   {
      if (m_techniques[j] != nullptr)
      {
         glDeleteProgram(m_techniques[j]->program);
         delete m_techniques[j];
         delete m_boundState[j];
      }
   }
#else // DirectX 9
   delete m_boundState;
   SAFE_RELEASE(m_shader);
#endif
}

void Shader::UnbindSampler(Sampler* sampler)
{
   #if !defined(ENABLE_SDL)
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
   if (m_boundTechnique != m_technique)
   {
      m_renderDevice->m_curTechniqueChanges++;
      m_boundTechnique = m_technique;
#ifdef ENABLE_SDL
      glUseProgram(m_techniques[m_technique]->program);
#else
      CHECKD3D(m_shader->SetTechnique((D3DXHANDLE)shaderTechniqueNames[m_technique].c_str()));
#endif
   }
   for (const auto& uniformName : m_uniforms[m_technique])
      ApplyUniform(uniformName);
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

void Shader::SetTextureNull(const ShaderUniforms uniformName)
{
   SetTexture(uniformName, m_renderDevice->m_nullTexture);
}

void Shader::SetTexture(const ShaderUniforms uniformName, BaseTexture* texel, const SamplerFilter filter, const SamplerAddressMode clampU, const SamplerAddressMode clampV, const bool force_linear_rgb)
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
   #ifdef ENABLE_SDL
   // For OpenGL doReflections is computed from the reflection factor
   SetBool(SHADER_is_metal, isMetal);
   SetBool(SHADER_doNormalMapping, doNormalMapping);
   SetBool(SHADER_doRefractions, doRefractions);
   if (tech == SHADER_TECHNIQUE_basic_with_texture && doAlphaTest)
      tech = SHADER_TECHNIQUE_basic_with_texture_at;
   #else
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

void Shader::SetBasic(const Material * const mat, Texture * const pin)
{
   if (pin)
   {
      SetTechniqueMaterial(SHADER_TECHNIQUE_basic_with_texture, mat, pin->m_alphaTestValue >= 0.f && !pin->m_pdsBuffer->IsOpaque());
      SetTexture(SHADER_tex_base_color, pin); //, SF_TRILINEAR, SA_REPEAT, SA_REPEAT);
      SetAlphaTestValue(pin->m_alphaTestValue);
      SetMaterial(mat, !pin->m_pdsBuffer->IsOpaque());
   }
   else
   {
      SetTechniqueMaterial(SHADER_TECHNIQUE_basic_without_texture, mat);
      SetMaterial(mat, false);
   }
}

void Shader::ApplyUniform(const ShaderUniforms uniformName)
{
   assert(0 <= uniformName && uniformName < SHADER_UNIFORM_COUNT);
   assert(m_stateOffsets[uniformName] != -1);

#ifdef ENABLE_SDL
   ShaderState* const __restrict boundState = m_boundState[m_technique];
   // For OpenGL uniform binding state is per technique (i.e. program)
   const UniformDesc& desc = m_techniques[m_technique]->uniform_desc[uniformName];
   assert(desc.location >= 0); // Do not apply to an unused uniform
   if (desc.location < 0) // FIXME remove
      return;
#else
   ShaderState* const __restrict boundState = m_boundState;
   const UniformDesc& desc = m_uniform_desc[uniformName];
#endif
   void* const src = m_state->m_state + m_stateOffsets[uniformName];
   void* const dst = boundState->m_state + m_stateOffsets[uniformName];

   #ifdef ENABLE_SDL
   if (desc.uniform.type == SUT_Sampler)
   {
      // DX9 implementation uses preaffected texture units, not samplers, so these can not be used for OpenGL. This would cause some collisions.
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
      return;
   }
   #endif

   if (memcmp(dst, src, m_stateSizes[uniformName]) == 0)
   {
      #ifdef ENABLE_SDL
      if (desc.uniform.type == SUT_DataBlock)
      {
         glUniformBlockBinding(m_techniques[m_technique]->program, desc.location, 0);
         glBindBufferRange(GL_UNIFORM_BUFFER, 0, desc.blockBuffer, 0, m_stateSizes[uniformName]);
      }
      #endif
      return;
   }
   m_renderDevice->m_curParameterChanges++;

   switch (desc.uniform.type)
   {
   case SUT_DataBlock: // Uniform blocks
      #ifdef ENABLE_SDL
      glBindBuffer(GL_UNIFORM_BUFFER, desc.blockBuffer);
      glBufferData(GL_UNIFORM_BUFFER, m_stateSizes[uniformName], src, GL_STREAM_DRAW);
      glUniformBlockBinding(m_techniques[m_technique]->program, desc.location, 0);
      glBindBufferRange(GL_UNIFORM_BUFFER, 0, desc.blockBuffer, 0, m_stateSizes[uniformName]);
      #else
      assert(false); // Unsupported on DX9
      #endif
      break;
   case SUT_Bool:
      {
         assert(desc.uniform.count == 1);
         bool val = *(bool*)src;
         *(bool*)dst = val;
         #ifdef ENABLE_SDL
         glUniform1i(desc.location, val);
         #else
         CHECKD3D(m_shader->SetBool(desc.handle, val));
         #endif
      }
      break;
   case SUT_Int:
      {
         assert(desc.uniform.count == 1);
         int val = *(int*)src;
         *(int*)dst = val;
         #ifdef ENABLE_SDL
         glUniform1i(desc.location, val);
         #else
         CHECKD3D(m_shader->SetInt(desc.handle, val));
         #endif
      }
      break;
   case SUT_Float:
      {
         assert(desc.uniform.count == 1);
         float val = *(float*)src;
         *(float*)dst = val;
         #ifdef ENABLE_SDL
         glUniform1f(desc.location, val);
         #else
         CHECKD3D(m_shader->SetFloat(desc.handle, val));
         #endif
      }
      break;
   case SUT_Float2:
      assert(desc.uniform.count == 1);
      memcpy(dst, src, m_stateSizes[uniformName]);
      #ifdef ENABLE_SDL
      glUniform2fv(desc.location, 1, (const GLfloat*)src);
      #else
      CHECKD3D(m_shader->SetVector(desc.handle, (D3DXVECTOR4*)src));
      #endif
      break;
   case SUT_Float3:
      assert(desc.uniform.count == 1);
      memcpy(dst, src, m_stateSizes[uniformName]);
      #ifdef ENABLE_SDL
      glUniform3fv(desc.location, 1, (const GLfloat*)src);
      #else
      CHECKD3D(m_shader->SetVector(desc.handle, (D3DXVECTOR4*)src));
      #endif
      break;
   case SUT_Float4:
      assert(desc.uniform.count == 1);
      memcpy(dst, src, m_stateSizes[uniformName]);
      #ifdef ENABLE_SDL
      glUniform4fv(desc.location, 1, (const GLfloat*)src);
      #else
      CHECKD3D(m_shader->SetVector(desc.handle, (D3DXVECTOR4*)src));
      #endif
      break;
   case SUT_Float4v:
      memcpy(dst, src, m_stateSizes[uniformName]);
      #ifdef ENABLE_SDL
      glUniform4fv(desc.location, desc.uniform.count, (const GLfloat*)src);
      #else
      CHECKD3D(m_shader->SetFloatArray(desc.handle, (float*) src, desc.uniform.count * 4));
      #endif
      break;
   case SUT_Float3x4:
   case SUT_Float4x3:
   case SUT_Float4x4:
      memcpy(dst, src, m_stateSizes[uniformName]);
      #ifdef ENABLE_SDL
      glUniformMatrix4fv(desc.location, desc.uniform.count, GL_FALSE, (const GLfloat*)src);
      #else
      assert(desc.uniform.count == 1);
      /*CHECKD3D(*/ m_shader->SetMatrix(desc.handle, (D3DXMATRIX*) src) /*)*/; // leads to invalid calls when setting some of the matrices (as hlsl compiler optimizes some down to less than 4x4)
      #endif
      break;
#ifndef ENABLE_SDL
   case SUT_Sampler:
      {
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
      }
      break;
#endif
   default: assert(false);
   }
}

#ifdef ENABLE_SDL
///////////////////////////////////////////////////////////////////////////////
// OpenGL specific implementation

bool Shader::UseGeometryShader() const
{
   // TODO we could (should) move the viewport/layer index from geometry to vertex shader and simplify all of this (a few hardware do not support it but they wan fallback to multi pass rendering)
   return m_renderDevice->m_stereo3D != STEREO_OFF;
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
               currentElement.append("#define GLSL\n"s);
               currentElement.append("\n"s);
               if (UseGeometryShader())
                  currentElement.append("#define USE_GEOMETRY_SHADER 1\n"s);
               else
                  currentElement.append("#define USE_GEOMETRY_SHADER 0\n"s);
               if (m_renderDevice->m_stereo3D == STEREO_OFF)
               {
                  currentElement.append("#define N_EYES 1\n"s);
                  currentElement.append("#define ENABLE_VR 0\n"s);
               }
               else if (m_renderDevice->m_stereo3D == STEREO_VR)
               {
                  currentElement.append("#define N_EYES 2\n"s);
                  currentElement.append("#define ENABLE_VR 1\n"s);
               }
               else
               {
                  currentElement.append("#define N_EYES 2\n"s);
                  currentElement.append("#define ENABLE_VR 0\n"s);
               }
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
            currentElement.append(line).append("\n"s);
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
      char msg[16384];
      sprintf_s(msg, sizeof(msg), "Fatal Error: Vertex Shader compilation of %s:%s failed!\n\n%s", fileNameRoot.c_str(), shaderCodeName.c_str(),errorText);
      ReportError(msg, -1, __FILE__, __LINE__);
      free(errorText);
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
         char msg[2048];
         sprintf_s(msg, sizeof(msg), "Fatal Error: Geometry Shader compilation of %s:%s failed!\n\n%s", fileNameRoot.c_str(), shaderCodeName.c_str(), errorText);
         ReportError(msg, -1, __FILE__, __LINE__);
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
         char msg[16384];
         sprintf_s(msg, sizeof(msg), "Fatal Error: Fragment Shader compilation of %s:%s failed!\n\n%s", fileNameRoot.c_str(), shaderCodeName.c_str(), errorText);
         ReportError(msg, -1, __FILE__, __LINE__);
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
      string vs_name = shaderCodeName + ".VS"s;
      string gs_name = shaderCodeName + ".GS"s;
      string fs_name = shaderCodeName + ".FS"s;
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
         glGetActiveUniform(shader->program, (GLuint)i, 256, &length, &size, &type, uniformName);
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
         string token = "%PARAM" + std::to_string(paramID) + "%";
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

#ifdef __STANDALONE__
string Shader::preprocessGLShader(const string& shaderCode) {
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
         header += line + "\n";
#endif
         header += "#define SHADER_STANDALONE\n";
      }
      else if (line.compare(0, 11, "#extension ") == 0)
         extensions += line + "\n";
      else
         code += line + "\n";
   }

   return header + extensions + code;
}
#endif

bool Shader::Load(const std::string& name, const BYTE* code, unsigned int codeSize)
{
   m_shaderCodeName = name;
   m_shaderPath = g_pvp->m_szMyPath + "shader"s + std::to_string(VP_VERSION_MAJOR) + '.' + std::to_string(VP_VERSION_MINOR) + '.' + std::to_string(VP_VERSION_REV) + PATH_SEPARATOR_CHAR;
   PLOGI << "Parsing file " << name;
   robin_hood::unordered_map<string, string> values;
   const bool parsing = parseFile(m_shaderCodeName, m_shaderCodeName, 0, values, "GLOBAL"s);
   if (!parsing) {
      m_hasError = true;
      PLOGE << "Parsing failed";
      char msg[128];
      sprintf_s(msg, sizeof(msg), "Fatal Error: Shader parsing of %s failed!", m_shaderCodeName.c_str());
      ReportError(msg, -1, __FILE__, __LINE__);
      return false;
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
               //PLOGI << "Compiling technique: " << shaderTechniqueNames[technique];
               string vertexShaderCode = vertex;
               vertexShaderCode.append("\n//"s).append(_technique).append("\n//"s).append(element[2]).append("\n"s);
               vertexShaderCode.append(analyzeFunction(m_shaderCodeName, _technique, element[2], values)).append("\0"s);
#ifdef __STANDALONE__
               vertexShaderCode = preprocessGLShader(vertexShaderCode);
#endif
               string geometryShaderCode;
               if (elem == 5 && element[3].length() > 0)
               {
                  geometryShaderCode = geometry;
                  geometryShaderCode.append("\n//").append(_technique).append("\n//").append(element[3]).append("\n"s);
                  geometryShaderCode.append(analyzeFunction(m_shaderCodeName, _technique, element[3], values)).append("\0"s);
               }
#ifdef __STANDALONE__
               geometryShaderCode = preprocessGLShader(geometryShaderCode);
#endif
               string fragmentShaderCode = fragment;
               fragmentShaderCode.append("\n//").append(_technique).append("\n//").append(element[elem - 1]).append("\n"s);
               fragmentShaderCode.append(analyzeFunction(m_shaderCodeName, _technique, element[elem - 1], values)).append("\0"s);
#ifdef __STANDALONE__
               fragmentShaderCode = preprocessGLShader(fragmentShaderCode);
#endif
               ShaderTechnique* build = compileGLShader(technique, m_shaderCodeName, element[0] /*.append("_"s).append(element[1])*/, vertexShaderCode, geometryShaderCode, fragmentShaderCode);
               if (build != nullptr)
               {
                  m_techniques[technique] = build;
                  tecCount++;
               }
               else
               {
                  m_hasError = true;
                  char msg[128];
                  sprintf_s(msg, sizeof(msg), "Fatal Error: Compilation failed for technique %s of %s!", shaderTechniqueNames[technique].c_str(), m_shaderCodeName.c_str());
                  ReportError(msg, -1, __FILE__, __LINE__);
                  return false;
               }
            }
         }
      }
      PLOGI << "Compiled successfully " << std::to_string(tecCount) << " shaders.";
   }
   else {
      m_hasError = true;
      PLOGE << "No techniques found.";
      char msg[128];
      sprintf_s(msg, sizeof(msg), "Fatal Error: No shader techniques found in %s!", m_shaderCodeName.c_str());
      ReportError(msg, -1, __FILE__, __LINE__);
      return false;
   }
   return true;
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
            m_uniform_desc[uniformIndex].tex_handle = m_shader->GetParameterByName(NULL, shaderUniformNames[uniformIndex].tex_name.c_str());
            if (param_desc.Semantic != nullptr && std::string(param_desc.Semantic).rfind("TEXUNIT"s, 0) == 0)
            {
               int unit = param_desc.Semantic[strlen(param_desc.Semantic) - 1] - '0';
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
   return true;
}

#endif
