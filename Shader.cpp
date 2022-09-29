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
   SHADER_TECHNIQUE(basic_without_texture_n_mirror),
   SHADER_TECHNIQUE(basic_with_texture_n_mirror),
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
   SHADER_TECHNIQUE(fb_CAS),
   SHADER_TECHNIQUE(fb_BilateralSharp_CAS),
   SHADER_TECHNIQUE(SSReflection),
   SHADER_TECHNIQUE(basic_noLight),
   SHADER_TECHNIQUE(bulb_light),
   SHADER_TECHNIQUE(SMAA_ColorEdgeDetection),
   SHADER_TECHNIQUE(SMAA_BlendWeightCalculation),
   SHADER_TECHNIQUE(SMAA_NeighborhoodBlending),
   SHADER_TECHNIQUE(stereo),
   SHADER_TECHNIQUE(stereo_Int),
   SHADER_TECHNIQUE(stereo_Flipped_Int),
   SHADER_TECHNIQUE(stereo_Anaglyph),
   SHADER_TECHNIQUE(stereo_AMD_DEBUG),
};
#undef SHADER_TECHNIQUE

ShaderTechniques Shader::getTechniqueByName(const string& name)
{
   for (int i = 0; i < SHADER_TECHNIQUE_COUNT; ++i)
      if (name == shaderTechniqueNames[i])
         return ShaderTechniques(i);

   // FIXME LOG(1, m_shaderCodeName, string("getTechniqueByName Could not find technique ").append(name).append(" in shaderTechniqueNames."));
   return SHADER_TECHNIQUE_INVALID;
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

vec4 Shader::GetCurrentFlasherColorAlpha()
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
      SetBool("lightingOff", imageMode || backglassMode); // at the moment can be combined into a single bool due to what the shader actually does in the end
   }
}

void Shader::Unload()
{
   SAFE_RELEASE(m_shader);
}

void Shader::SetTextureNull(const SHADER_UNIFORM_HANDLE texelName)
{
   const unsigned int idx = texelName[strlen(texelName) - 1] - '0'; // current convention: SetTexture gets "TextureX", where X 0..4
   const bool cache = (idx < TEXTURESET_STATE_CACHE_SIZE);

   if (cache)
      currentTexture[idx] = nullptr; // direct set of device tex invalidates the cache

   CHECKD3D(m_shader->SetTexture(texelName, nullptr));

   m_renderDevice->m_curTextureChanges++;
}

void Shader::SetTexture(const SHADER_UNIFORM_HANDLE texelName, Texture* texel, const TextureFilter filter, const bool clampU, const bool clampV, const bool force_linear_rgb)
{
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

void Shader::SetTexture(const SHADER_UNIFORM_HANDLE texelName, BaseTexture* texel, const TextureFilter filter, const bool clampU, const bool clampV, const bool force_linear_rgb)
{
   if (!texel)
      SetTexture(texelName, (Sampler*)nullptr);
   else
      SetTexture(texelName, m_renderDevice->m_texMan.LoadTexture(texel, filter, clampU, clampV, force_linear_rgb));
}

void Shader::SetTexture(const SHADER_UNIFORM_HANDLE texelName, Sampler* texel)
{
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
      case SHADER_TECHNIQUE_basic_without_texture: SetTechnique(SHADER_TECHNIQUE_basic_without_texture_isMetal); break;
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

void Shader::SetMatrix(const SHADER_UNIFORM_HANDLE hParameter, const D3DXMATRIX* pMatrix)
{
   /*CHECKD3D(*/m_shader->SetMatrix(hParameter, pMatrix)/*)*/; // leads to invalid calls when setting some of the matrices (as hlsl compiler optimizes some down to less than 4x4)
   m_renderDevice->m_curParameterChanges++;
}

void Shader::SetVector(const SHADER_UNIFORM_HANDLE hParameter, const vec4* pVector)
{
   CHECKD3D(m_shader->SetVector(hParameter, pVector));
   m_renderDevice->m_curParameterChanges++;
}

void Shader::SetFloat(const SHADER_UNIFORM_HANDLE hParameter, const float f)
{
   CHECKD3D(m_shader->SetFloat(hParameter, f));
   m_renderDevice->m_curParameterChanges++;
}

void Shader::SetInt(const SHADER_UNIFORM_HANDLE hParameter, const int i)
{
   CHECKD3D(m_shader->SetInt(hParameter, i));
   m_renderDevice->m_curParameterChanges++;
}

void Shader::SetBool(const SHADER_UNIFORM_HANDLE hParameter, const bool b)
{
   CHECKD3D(m_shader->SetBool(hParameter, b));
   m_renderDevice->m_curParameterChanges++;
}

void Shader::SetValue(const SHADER_UNIFORM_HANDLE hParameter, const void* pData, const unsigned int Bytes)
{
   CHECKD3D(m_shader->SetValue(hParameter, pData, Bytes));
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
