// license:GPLv3+

#include "core/stdafx.h"
#include "ThreadPool.h"
#ifndef __STANDALONE__
#include "BAM/BAMView.h"
#endif
#include "math/bluenoise.h"
#include "math/math.h"
#include "meshes/ballMesh.h"
#include "renderer/Anaglyph.h"
#include "renderer/Shader.h"
#include "renderer/RenderCommand.h"
#include "renderer/RenderDevice.h"
#include "renderer/VRDevice.h"
#include "core/VPXPluginAPIImpl.h"

#ifdef __LIBVPINBALL__
#include "lib/src/VPinballLib.h"
#endif

// MSVC Concurrency Viewer support
// This requires to add the MSVC Concurrency SDK to the project
//#define MSVC_CONCURRENCY_VIEWER
#ifdef MSVC_CONCURRENCY_VIEWER
#include <cvmarkersobj.h>
using namespace Concurrency::diagnostic;
extern marker_series series;
#endif

#define MAX_BALL_SHADOW 8

////////////////////////////////////////////////////////////////////////////////

Renderer::Renderer(PinTable* const table, VPX::Window* wnd, VideoSyncMode& syncMode, const StereoMode stereo3D)
   : m_stereo3D(stereo3D)
   , m_table(table)
   , m_sceneLighting(table)
{
   m_stereo3Denabled = m_table->m_settings.GetPlayer_Stereo3DEnabled();
   m_toneMapper = (ToneMapper)m_table->m_settings.GetTableOverride_ToneMapper();
   m_HDRforceDisableToneMapper = m_table->m_settings.GetPlayer_HDRDisableToneMapper();
   m_exposure = m_table->m_settings.GetTableOverride_Exposure();
   m_dynamicAO = m_table->m_settings.GetPlayer_DynamicAO();
   m_disableAO = m_table->m_settings.GetPlayer_DisableAO();
   m_vrPreview = (VRPreviewMode)m_table->m_settings.GetPlayer_VRPreview();
   m_vrPreviewShrink = m_table->m_settings.GetPlayerVR_ShrinkPreview();
   m_FXAA = m_table->m_settings.GetPlayer_FXAA();
   m_sharpen = m_table->m_settings.GetPlayer_Sharpen();
   m_ss_refl = m_table->m_settings.GetPlayer_SSRefl();
   m_bloomOff = m_table->m_settings.GetPlayer_ForceBloomOff();
   m_motionBlurOff = m_table->m_settings.GetPlayer_ForceMotionBlurOff();
   m_maxReflectionMode = (RenderProbe::ReflectionMode)m_table->m_settings.GetPlayer_PFReflection();
   m_trailForBalls = m_table->m_settings.GetPlayer_BallTrail();
   m_ballTrailStrength = m_table->m_settings.GetPlayer_BallTrailStrength();
   m_ballAntiStretch = m_table->m_settings.GetPlayer_BallAntiStretch();
   m_ballImage = nullptr;
   m_decalImage = nullptr;
   m_overwriteBallImages = m_table->m_settings.GetPlayer_OverwriteBallImage();
   if (m_overwriteBallImages)
   {
      m_ballImage = BaseTexture::CreateFromFile(m_table->m_settings.GetPlayer_BallImage(), m_table->m_settings.GetPlayer_MaxTexDimension());
      m_decalImage = BaseTexture::CreateFromFile(m_table->m_settings.GetPlayer_DecalImage(), m_table->m_settings.GetPlayer_MaxTexDimension());
   }
   m_vrApplyColorKey = m_stereo3D == STEREO_VR && m_table->m_settings.GetPlayerVR_UsePassthroughColor();
   m_vrColorKey = convertColor(m_table->m_settings.GetPlayerVR_PassthroughColor(), 1.f);
   m_vrColorKey.x = InvsRGB(m_vrColorKey.x);
   m_vrColorKey.y = InvsRGB(m_vrColorKey.y);
   m_vrColorKey.z = InvsRGB(m_vrColorKey.z);

   m_mvp = new ModelViewProj(m_stereo3D == STEREO_OFF ? 1 : 2);

   #if defined(ENABLE_OPENGL)
   constexpr int MSAASamples[] = { 1, 4, 6, 8 };
   const int nMSAASamples = MSAASamples[m_table->m_settings.GetPlayer_MSAASamples()];
   #elif defined(ENABLE_DX9) || defined(ENABLE_BGFX)
   // Sadly DX9 does not support resolving an MSAA depth buffer, making MSAA implementation complex for it. So just disable for now
   // BGFX MSAA is likely possible but not yet implemented
   constexpr int nMSAASamples = 1;
   #endif
   const bool useNvidiaApi = m_table->m_settings.GetPlayer_UseNVidiaAPI();
   const bool compressTextures = m_table->m_settings.GetPlayer_CompressTextures();
   const int nEyes = (m_stereo3D == STEREO_VR || m_stereo3D != STEREO_OFF) ? 2 : 1;
   try {
      m_renderDevice = new RenderDevice(wnd, m_stereo3D == STEREO_VR, nEyes, useNvidiaApi, compressTextures, nMSAASamples, syncMode);
   }
   catch (...) {
      // TODO better error handling => just let the exception up ?
      throw(E_FAIL);
   }

   const bool isHdr2020 = (g_pplayer->m_vrDevice == nullptr) && m_renderDevice->m_outputWnd[0]->IsWCGBackBuffer();
   if (isHdr2020)
   {
      m_exposure *= g_pvp->m_settings.GetPlayer_HDRGlobalExposure();
      m_bloomOff = true;
   }

   if (m_stereo3D == STEREO_VR)
   {
      // For VR, renders at the HMD native eye resolution (preview will reuse and scale/stretch it)
      m_renderWidth = g_pplayer->m_vrDevice->GetEyeWidth();
      m_renderHeight = g_pplayer->m_vrDevice->GetEyeHeight();
   }
   else if (m_stereo3D == STEREO_SBS)
   {
      // Side by side fits the 2 views along the output width, so each view is rendered at half the output width
      m_renderWidth = wnd->GetPixelWidth() / 2;
      m_renderHeight = wnd->GetPixelHeight();
   }
   else if (m_stereo3D == STEREO_TB || m_stereo3D == STEREO_INT || m_stereo3D == STEREO_FLIPPED_INT)
   {
      // Top/Bottom (and interlaced) fits the 2 views along the output height, so each view is rendered at half the output height
      m_renderWidth = wnd->GetPixelWidth();
      m_renderHeight = wnd->GetPixelHeight() / 2;
   }
   else
   {
      // Default renders at the output window pixel resolution
      m_renderWidth = wnd->GetPixelWidth();
      m_renderHeight = wnd->GetPixelHeight();
   }
   const float AAfactor = m_table->m_settings.GetPlayer_AAFactor();
   const int renderWidthAA = (int)((float)m_renderWidth * AAfactor);
   const int renderHeightAA = (int)((float)m_renderHeight * AAfactor);

   if (m_renderDevice->GetOutputBackBuffer() && (m_renderDevice->GetOutputBackBuffer()->GetColorFormat() == colorFormat::RGBA10) && (m_FXAA == Quality_SMAA || m_FXAA == Standard_DLAA))
      ShowError("SMAA or DLAA post-processing AA should not be combined with 10bit-output rendering (will result in visible artifacts)!");

   #if defined(ENABLE_BGFX)
      constexpr colorFormat renderFormat = colorFormat::RGB16F;
   #elif defined(ENABLE_OPENGL)
      #ifndef __OPENGLES__
         constexpr colorFormat renderFormat = colorFormat::RGB16F;
      #else
         constexpr colorFormat renderFormat = colorFormat::RGBA16F;
      #endif
   #elif defined(ENABLE_DX9)
      constexpr colorFormat renderFormat = colorFormat::RGBA16F;
   #endif
   const SurfaceType rtType = m_stereo3D == STEREO_OFF || !m_renderDevice->SupportLayeredRendering() ? SurfaceType::RT_DEFAULT : SurfaceType::RT_STEREO;

   // MSAA render target which is resolved to the non MSAA render target
   if (nMSAASamples > 1) 
      m_pOffscreenMSAABackBufferTexture = new RenderTarget(m_renderDevice, rtType, "MSAABackBuffer"s, renderWidthAA, renderHeightAA, renderFormat, true, nMSAASamples, "Fatal Error: unable to create MSAA render buffer!");

   // Either the main render target for non MSAA, or the buffer where the MSAA render is resolved
   m_pOffscreenBackBufferTexture1 = new RenderTarget(m_renderDevice, rtType, "BackBuffer1"s, renderWidthAA, renderHeightAA, renderFormat, true, 1, "Fatal Error: unable to create offscreen back buffer");

   // Second render target to swap, allowing to read previous frame render for ball reflection and motion blur
   m_pOffscreenBackBufferTexture2 = m_pOffscreenBackBufferTexture1->Duplicate("BackBuffer2"s, true);

   // Initialize shaders
   m_renderDevice->m_basicShader->SetVector(SHADER_w_h_height, (float)(1.0 / (double)GetMSAABackBufferTexture()->GetWidth()), (float)(1.0 / (double)GetMSAABackBufferTexture()->GetHeight()), 0.0f, 0.0f);
   m_renderDevice->m_ballShader->SetVector(SHADER_w_h_disableLighting,
      1.5f / (float)GetPreviousBackBufferTexture()->GetWidth(), // UV Offset for sampling reflections
      1.5f / (float)GetPreviousBackBufferTexture()->GetHeight(),
      0.f, 0.f);
   DisableBallLighting(m_table->m_settings.GetPlayer_DisableLightingForBalls());

   #ifndef __STANDALONE__
      BAMView::init();
   #endif

   m_backGlass = nullptr;

   #ifdef ENABLE_VR
   if (m_stereo3D == STEREO_VR) {
      m_backGlass = new BackGlass(m_renderDevice, m_table->GetDecalsEnabled() ? m_table->GetImage(m_table->m_BG_image[m_table->GetViewMode()]) : nullptr);
      //AMD Debugging
      colorFormat renderBufferFormatVR;
      const int textureModeVR = g_pplayer->m_ptable->m_settings.GetPlayerVR_EyeFBFormat();
      switch (textureModeVR) {
      case 0:
         renderBufferFormatVR = RGB8;
         break;
      case 2:
         renderBufferFormatVR = RGB16F;
         break;
      case 3:
         renderBufferFormatVR = RGBA16F;
         break;
      case 1:
      default:
         renderBufferFormatVR = RGBA8;
         break;
      }
      m_pOffscreenVRLeft = new RenderTarget(m_renderDevice, SurfaceType::RT_DEFAULT, "VRLeft"s, m_renderWidth, m_renderHeight, renderBufferFormatVR, false, 1, "Fatal Error: unable to create left eye buffer!");
      m_pOffscreenVRRight = new RenderTarget(m_renderDevice, SurfaceType::RT_DEFAULT, "VRRight"s, m_renderWidth, m_renderHeight, renderBufferFormatVR, false, 1, "Fatal Error: unable to create right eye buffer!");
   }
   #endif

   // alloc bloom tex at 1/4 x 1/4 res (allows for simple HQ downscale of clipped input while saving memory)
   m_pBloomBufferTexture = new RenderTarget(m_renderDevice, 
      GetBackBufferTexture()->m_type, "BloomBuffer1"s, 
      m_renderWidth / 4, m_renderHeight / 4, 
      GetBackBufferTexture()->GetColorFormat(),
      false, 1, "Fatal Error: unable to create bloom buffer!");
   m_pBloomTmpBufferTexture = m_pBloomBufferTexture->Duplicate("BloomBuffer2"s);

   std::shared_ptr<BaseTexture> ballTex = std::shared_ptr<BaseTexture>(BaseTexture::CreateFromFile(g_pvp->GetAppPath(VPinball::AppSubFolder::Assets, "BallEnv.exr").string()));
   m_ballEnvSampler = std::make_shared<Sampler>(m_renderDevice, "Ball Env"s, ballTex, false);
   ballTex = nullptr;

   std::shared_ptr<BaseTexture> aoTex = std::shared_ptr<BaseTexture>(BaseTexture::CreateFromFile(g_pvp->GetAppPath(VPinball::AppSubFolder::Assets, "AODither.webp").string()));
   m_aoDitherSampler = std::make_shared<Sampler>(m_renderDevice, "AO Dither"s, aoTex, true);
   aoTex = nullptr;

   Texture* tableEnv = m_table->GetImage(m_table->m_envImage);
   std::shared_ptr<const BaseTexture> envTex
      = tableEnv ? tableEnv->GetRawBitmap(false, 0) : std::shared_ptr<BaseTexture>(BaseTexture::CreateFromFile(g_pvp->GetAppPath(VPinball::AppSubFolder::Assets, "EnvMap.webp").string()));
   m_envSampler = std::make_shared<Sampler>(m_renderDevice, "Table Env"s, envTex, false);

   PLOGI << "Computing environment map radiance"; // For profiling

   const unsigned int envTexHeight = min(envTex->height(), 256u) / 8;
   const unsigned int envTexWidth = envTexHeight * 2;
   // DirectX 9 does not support bitwise operation in shader, so radical_inverse is not implemented, and therefore we use the slow CPU path instead of GPU
   // OpenGL ES does not support features used in the irradiance shader, so we use the CPU path for it as well
   // There is a bug when using the Metal shader, so we use the CPU path for it as well
   #if defined(ENABLE_DX9) || defined(__OPENGLES__) || defined(__APPLE__)
      m_envRadianceTexture = EnvmapPrecalc(envTex, envTexWidth, envTexHeight);
      m_renderDevice->m_texMan.SetDirty(m_envRadianceTexture.get());
      m_renderDevice->m_basicShader->SetTexture(SHADER_tex_diffuse_env, m_envRadianceTexture.get());
      m_renderDevice->m_ballShader->SetTexture(SHADER_tex_diffuse_env, m_envRadianceTexture.get());
   #else // Compute radiance on the GPU
      const colorFormat rad_format = envTex->m_format == BaseTexture::RGB_FP32 ? colorFormat::RGBA32F : colorFormat::RGBA16F;
      m_envRadianceTexture = new RenderTarget(m_renderDevice, SurfaceType::RT_DEFAULT, "Irradiance"s, envTexWidth, envTexHeight, rad_format, false, 1, "Failed to create irradiance render target");
      m_renderDevice->ResetRenderState();
      m_renderDevice->SetRenderState(RenderState::CULLMODE, RenderState::CULL_NONE);
      m_renderDevice->SetRenderState(RenderState::ZENABLE, RenderState::RS_FALSE);
      m_renderDevice->SetRenderTarget("Env Irradiance PreCalc"s, m_envRadianceTexture);
      m_renderDevice->m_FBShader->SetTechnique(SHADER_TECHNIQUE_irradiance);
      m_renderDevice->m_FBShader->SetTexture(SHADER_tex_env, m_envSampler);
      m_renderDevice->m_FBShader->SetVector(SHADER_w_h_height, (float)(1.0 / m_envSampler->GetWidth()), (float)(1.0 / m_envSampler->GetHeight()), 1.0f, 1.0f);
      m_renderDevice->DrawFullscreenTexturedQuad(m_renderDevice->m_FBShader);
      m_renderDevice->SubmitRenderFrame(); // Force submission as result users do not explicitly declare the dependency on this pass
      m_renderDevice->m_basicShader->SetTexture(SHADER_tex_diffuse_env, m_envRadianceTexture->GetColorSampler());
      m_renderDevice->m_ballShader->SetTexture(SHADER_tex_diffuse_env, m_envRadianceTexture->GetColorSampler());
   #endif
   envTex.reset();
   PLOGI << "Environment map radiance computed"; // For profiling

   const bool lowDetailBall = (m_table->GetDetailLevel() < 10);
   std::shared_ptr<IndexBuffer> ballIndexBuffer
      = std::make_shared<IndexBuffer>(m_renderDevice, lowDetailBall ? basicBallLoNumFaces : basicBallMidNumFaces, lowDetailBall ? basicBallLoIndices : basicBallMidIndices);
   std::shared_ptr<VertexBuffer> ballVertexBuffer
      = std::make_shared<VertexBuffer>(m_renderDevice, lowDetailBall ? basicBallLoNumVertices : basicBallMidNumVertices, (const float*)(lowDetailBall ? basicBallLo : basicBallMid));
   m_ballMeshBuffer = std::make_shared<MeshBuffer>("Ball"s, ballVertexBuffer, ballIndexBuffer, true);
#ifdef DEBUG_BALL_SPIN
   {
      vector<Vertex3D_NoTex2> ballDbgVtx;
      for (int j = -1; j <= 1; ++j)
      {
         const int numPts = (j == 0) ? 6 : 3;
         const float theta = (float)(j * (M_PI / 4.0));
         for (int i = 0; i < numPts; ++i)
         {
            const float phi = (float)(i * (2.0 * M_PI / numPts));
            Vertex3D_NoTex2 vtx;
            vtx.nx = cosf(theta) * cosf(phi);
            vtx.ny = cosf(theta) * sinf(phi);
            vtx.nz = sinf(theta);
            vtx.x = vtx.nx;
            vtx.y = vtx.ny;
            vtx.z = vtx.nz;
            vtx.tu = 0.f;
            vtx.tv = 0.f;
            ballDbgVtx.push_back(vtx);
         }
      }

      std::shared_ptr<VertexBuffer> ballDebugPoints = std::make_shared<VertexBuffer>(m_renderDevice, (unsigned int)ballDbgVtx.size(), (float*)ballDbgVtx.data(), false);
      m_ballDebugPoints = std::make_shared<MeshBuffer>("Ball.Debug"s, ballDebugPoints);
   }
#endif
   // Support up to 64 balls, that should be sufficient
   std::shared_ptr<VertexBuffer> ballTrailVertexBuffer = std::make_shared<VertexBuffer>(m_renderDevice, 64 * (MAX_BALL_TRAIL_POS - 2) * 2 + 4, nullptr, true);
   m_ballTrailMeshBuffer = std::make_shared<MeshBuffer>("Ball.Trail"s, ballTrailVertexBuffer);

   // Cache DMD renderer properties
   for (int profile = 0; profile < (int)std::size(m_dmdUseLegacyRenderer); profile++)
   {
      m_dmdUseLegacyRenderer[profile] = m_table->m_settings.GetDMD_ProfileLegacy(profile);
      #if !defined(ENABLE_BGFX)
         m_dmdUseLegacyRenderer[profile] = false; // Only available for BGFX
      #endif
      m_dmdDotColor[profile] = convertColor(
         m_table->m_settings.GetDMD_ProfileDotTint(profile),
         m_table->m_settings.GetDMD_ProfileDotBrightness(profile));
      m_dmdDotProperties[profile].x = m_table->m_settings.GetDMD_ProfileDotSize(profile);
      m_dmdDotProperties[profile].y = m_table->m_settings.GetDMD_ProfileDotSharpness(profile);
      m_dmdDotProperties[profile].w = m_table->m_settings.GetDMD_ProfileDiffuseGlow(profile);
      m_dmdUnlitDotColor[profile] = convertColor(m_table->m_settings.GetDMD_ProfileUnlitDotColor(profile), 1.f);
      // Convert color as settings are sRGB color while shader needs linear RGB color
      m_dmdDotColor[profile].x = InvsRGB(m_dmdDotColor[profile].x);
      m_dmdDotColor[profile].y = InvsRGB(m_dmdDotColor[profile].y);
      m_dmdDotColor[profile].z = InvsRGB(m_dmdDotColor[profile].z);
      m_dmdUnlitDotColor[profile].x = InvsRGB(m_dmdUnlitDotColor[profile].x);
      m_dmdUnlitDotColor[profile].y = InvsRGB(m_dmdUnlitDotColor[profile].y);
      m_dmdUnlitDotColor[profile].z = InvsRGB(m_dmdUnlitDotColor[profile].z);
   }

   // Cache Seg display renderer properties
   for (int profile = 0; profile < (int)std::size(m_segColor); profile++)
   {
      m_segColor[profile] = convertColor(m_table->m_settings.GetAlpha_ProfileColor(profile), m_table->m_settings.GetAlpha_ProfileBrightness(profile));
      m_segUnlitColor[profile] = convertColor(m_table->m_settings.GetAlpha_ProfileUnlit(profile), m_table->m_settings.GetAlpha_ProfileDiffuseGlow(profile));
      // Convert color as settings are sRGB color while shader needs linear RGB color
      m_segColor[profile].x = InvsRGB(m_segColor[profile].x);
      m_segColor[profile].y = InvsRGB(m_segColor[profile].y);
      m_segColor[profile].z = InvsRGB(m_segColor[profile].z);
      m_segUnlitColor[profile].x = InvsRGB(m_segUnlitColor[profile].x);
      m_segUnlitColor[profile].y = InvsRGB(m_segUnlitColor[profile].y);
      m_segUnlitColor[profile].z = InvsRGB(m_segUnlitColor[profile].z);
   }


   m_renderDevice->ResetRenderState();
   #if defined(ENABLE_DX9)
   D3DVIEWPORT9 viewPort;
   viewPort.X = 0;
   viewPort.Y = 0;
   viewPort.Width = m_renderWidth;
   viewPort.Height = m_renderHeight;
   viewPort.MinZ = 0.0f;
   viewPort.MaxZ = 1.0f;
   CHECKD3D(m_renderDevice->GetCoreDevice()->SetViewport(&viewPort));
   CHECKD3D(m_renderDevice->GetCoreDevice()->SetRenderState(D3DRS_LIGHTING, FALSE));
   CHECKD3D(m_renderDevice->GetCoreDevice()->SetRenderState(D3DRS_CLIPPING, FALSE));
   CHECKD3D(m_renderDevice->GetCoreDevice()->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1));
   CHECKD3D(m_renderDevice->GetCoreDevice()->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE));
   CHECKD3D(m_renderDevice->GetCoreDevice()->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0));
   CHECKD3D(m_renderDevice->GetCoreDevice()->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE));
   CHECKD3D(m_renderDevice->GetCoreDevice()->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE));
   CHECKD3D(m_renderDevice->GetCoreDevice()->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TFACTOR)); // default tfactor: 1,1,1,1
   #endif
}

Renderer::~Renderer()
{
   delete m_mvp;
   m_gpu_profiler.Shutdown();
   delete m_backGlass;
   m_ballMeshBuffer = nullptr;
   #ifdef DEBUG_BALL_SPIN
   m_ballDebugPoints = nullptr;
   #endif
   m_ballTrailMeshBuffer = nullptr;
   delete m_tonemapLUT;
   delete m_staticPrepassRT;
   delete m_pOffscreenBackBufferTexture1;
   delete m_pOffscreenBackBufferTexture2;
   delete m_pBloomBufferTexture;
   delete m_pBloomTmpBufferTexture;
   delete m_pPostProcessRenderTarget1;
   delete m_pPostProcessRenderTarget2;
   delete m_pReflectionBufferTexture;
   delete m_pMotionBlurBufferTexture;
   delete m_pOffscreenVRLeft;
   delete m_pOffscreenVRRight;
   for (int window = 0; window <= VPXWindowId::VPXWINDOW_Topper; window++)
      m_ancillaryWndHdrRT[window] = nullptr;
   #if defined(ENABLE_DX9) || defined(__OPENGLES__) || defined(__APPLE__)
   m_envRadianceTexture.reset();
   #else
   delete m_envRadianceTexture;
   #endif
   ReleaseAORenderTargets();
   m_ballEnvSampler = nullptr;
   m_envSampler = nullptr;
   m_aoDitherSampler = nullptr;
   delete m_renderDevice;
}

Renderer::SceneLighting::SceneLighting(PinTable* const table)
   : m_table(table)
{
   m_mode = m_table->m_settings.GetPlayer_OverrideTableEmissionScale() ?
      m_table->m_settings.GetPlayer_DynamicDayNight() ? Mode::DayNight : Mode::User
      : Mode::Table;
   m_latitude = m_table->m_settings.GetPlayer_Latitude();
   m_longitude = m_table->m_settings.GetPlayer_Longitude();
   m_userLightLevel = m_table->m_settings.GetPlayer_EmissionScale();
   Update();
}

void Renderer::SceneLighting::Update()
{
   if (g_pvp->m_bgles) // Overriden from command line
   {
      m_emissionScale = g_pvp->m_fgles;
      return;
   }
   switch (m_mode)
   {
   case Mode::Table:
      m_emissionScale = m_table->m_globalEmissionScale;
      break;
      
   case Mode::User:
      m_emissionScale = m_userLightLevel;
      break;
   
   case Mode::DayNight:
   {
      time_t hour_machine;
      time(&hour_machine);
      tm local_hour;
      localtime_s(&local_hour, &hour_machine);

      const double rlat = m_latitude * (M_PI / 180.);
      const double rlong = m_longitude * (M_PI / 180.);

      const double tr = TheoreticRadiation(local_hour.tm_mday, local_hour.tm_mon + 1, local_hour.tm_year + 1900, rlat);
      const double max_tr = MaxTheoreticRadiation(local_hour.tm_year + 1900, rlat);
      const double sset = SunsetSunriseLocalTime(local_hour.tm_mday, local_hour.tm_mon + 1, local_hour.tm_year + 1900, rlong, rlat, false);
      const double srise = SunsetSunriseLocalTime(local_hour.tm_mday, local_hour.tm_mon + 1, local_hour.tm_year + 1900, rlong, rlat, true);

      const double cur = local_hour.tm_hour + local_hour.tm_min / 60.0;

      const float factor = (float)(sin(M_PI * clamp((cur - srise) / (sset - srise), 0., 1.)) //!! leave space before sunrise and after sunset?
         * sqrt(tr / max_tr)); //!! magic, "emulates" that shorter days are usually also "darker",cloudier,whatever in most regions

      m_emissionScale = clamp(factor, 0.15f, 1.f); //!! configurable clamp?
      break;
   }
   
   }
}

bool Renderer::UseAnisoFiltering() const { return Shader::GetDefaultSamplerFilter(SHADER_tex_base_color) == SF_ANISOTROPIC; }

void Renderer::SetAnisoFiltering(bool enable) {
   Shader::SetDefaultSamplerFilter(SHADER_tex_sprite, enable ? SF_ANISOTROPIC : SF_TRILINEAR);
   Shader::SetDefaultSamplerFilter(SHADER_tex_base_color, enable ? SF_ANISOTROPIC : SF_TRILINEAR);
   Shader::SetDefaultSamplerFilter(SHADER_tex_base_normalmap, enable ? SF_ANISOTROPIC : SF_TRILINEAR);
   Shader::SetDefaultSamplerFilter(SHADER_tex_flasher_A, enable ? SF_ANISOTROPIC : SF_TRILINEAR);
   Shader::SetDefaultSamplerFilter(SHADER_tex_flasher_B, enable ? SF_ANISOTROPIC : SF_TRILINEAR);
}

bool Renderer::IsBallLightingDisabled() const
{
   return m_renderDevice->m_ballShader->GetVector(SHADER_w_h_disableLighting).z != 0.f;
}

void Renderer::DisableBallLighting(bool disableLightingForBalls)
{
   vec4 prev = m_renderDevice->m_ballShader->GetVector(SHADER_w_h_disableLighting);
   m_renderDevice->m_ballShader->SetVector(SHADER_w_h_disableLighting, prev.x, prev.y, disableLightingForBalls ? 1.f : 0.f, prev.w);
}

void Renderer::SwapBackBufferRenderTargets()
{
   RenderTarget* tmp = m_pOffscreenBackBufferTexture1;
   m_pOffscreenBackBufferTexture1 = m_pOffscreenBackBufferTexture2;
   m_pOffscreenBackBufferTexture2 = tmp;
}

RenderTarget* Renderer::GetPostProcessRenderTarget1()
{
   if (m_pPostProcessRenderTarget1 == nullptr)
   {
      // Buffers for post-processing. Postprocess is done at scene resolution, on a LDR render target without MSAA nor full scene supersampling
      // excepted when using downsampled render buffer where upscaling is done after postprocessing.
      m_pPostProcessRenderTarget1 = new RenderTarget(m_renderDevice,
         GetBackBufferTexture()->m_type, "PostProcess1"s, 
         min(GetBackBufferTexture()->GetWidth(), m_renderWidth),
         min(GetBackBufferTexture()->GetHeight(), m_renderHeight),
         GetBackBufferTexture()->GetColorFormat() == RGBA10 ? colorFormat::RGBA10 : colorFormat::RGBA8, false, 1, 
         "Fatal Error: unable to create stereo3D/post-processing AA/sharpen buffer!");
   }
   return m_pPostProcessRenderTarget1;
}

RenderTarget* Renderer::GetPostProcessRenderTarget2()
{
   if (m_pPostProcessRenderTarget2 == nullptr)
      m_pPostProcessRenderTarget2 = GetPostProcessRenderTarget1()->Duplicate("PostProcess2"s);
   return m_pPostProcessRenderTarget2;
}

RenderTarget* Renderer::GetPostProcessRenderTarget(RenderTarget* renderedRT)
{
   RenderTarget* pp1 = GetPostProcessRenderTarget1();
   if (renderedRT == pp1)
      return GetPostProcessRenderTarget2();
   else
      return pp1;
}

RenderTarget* Renderer::GetReflectionBufferTexture()
{
   if (m_pReflectionBufferTexture == nullptr)
   {
      m_pReflectionBufferTexture = new RenderTarget(m_renderDevice, 
         GetBackBufferTexture()->m_type, "ReflectionBuffer"s, 
         GetBackBufferTexture()->GetWidth(),
         GetBackBufferTexture()->GetHeight(),
         GetBackBufferTexture()->GetColorFormat(),
         false, 1, "Fatal Error: unable to create Reflection buffer!");
   }
   return m_pReflectionBufferTexture;
}

RenderTarget* Renderer::GetMotionBlurBufferTexture()
{
   if (m_pMotionBlurBufferTexture == nullptr)
   {
      m_pMotionBlurBufferTexture = new RenderTarget(m_renderDevice, 
         GetBackBufferTexture()->m_type, "MotionBlurBuffer"s, 
         GetBackBufferTexture()->GetWidth(),
         GetBackBufferTexture()->GetHeight(),
         GetBackBufferTexture()->GetColorFormat(),
         false, 1, "Fatal Error: unable to create MotionBlur buffer!");
   }
   return m_pMotionBlurBufferTexture;
}

RenderTarget* Renderer::GetAORenderTarget(int idx)
{
   // Lazily creates AO render target since this can be enabled during play from script (at render buffer resolution)
   if (m_pAORenderTarget1 == nullptr)
   {
      m_pAORenderTarget1 = new RenderTarget(m_renderDevice, GetBackBufferTexture()->m_type, "AO1"s, 
         GetBackBufferTexture()->GetWidth(), GetBackBufferTexture()->GetHeight(), colorFormat::GREY8, false, 1, 
         "Unable to create AO buffers!\r\nPlease disable Ambient Occlusion.\r\nOr try to (un)set \"Alternative Depth Buffer processing\" in the video options!");
      m_pAORenderTarget2 = m_pAORenderTarget1->Duplicate("AO2"s);
   }
   return idx == 0 ? m_pAORenderTarget1 : m_pAORenderTarget2;
}

void Renderer::SwapAORenderTargets()
{
   RenderTarget* tmpAO = m_pAORenderTarget1;
   m_pAORenderTarget1 = m_pAORenderTarget2;
   m_pAORenderTarget2 = tmpAO;
}

std::shared_ptr<BaseTexture> Renderer::EnvmapPrecalc(const std::shared_ptr<const BaseTexture>& envTex, const unsigned int rad_env_xres, const unsigned int rad_env_yres)
{
   const void* __restrict envmap = envTex->datac();
   const unsigned int env_xres = envTex->width();
   const unsigned int env_yres = envTex->height();
   BaseTexture::Format env_format = envTex->m_format;
   const BaseTexture::Format rad_format = (env_format == BaseTexture::RGB_FP16 || env_format == BaseTexture::RGB_FP32) ? env_format : BaseTexture::SRGB;
   std::shared_ptr<BaseTexture> radTex = BaseTexture::Create(rad_env_xres, rad_env_yres, rad_format);
   uint8_t* const __restrict rad_envmap = static_cast<uint8_t*>(radTex->data());
   bool free_envmap = false;

#define PREFILTER_ENVMAP_DIFFUSE
#ifdef PREFILTER_ENVMAP_DIFFUSE
   // pre-filter envmap with a gauss (separable/two passes: x and y)
   //!!! not correct to pre-filter like this, but barely visible in the end, and helps to keep number of samples low (otherwise one would have to use >64k samples instead of 4k!)
   if ((env_format == BaseTexture::RGB_FP16 || env_format == BaseTexture::RGB_FP32) && env_xres > 64)
   {
      const float scale_factor = (float)env_xres*(float)(1.0 / 64.);
      const int xs = (int)(scale_factor*0.5f + 0.5f);
      void* const __restrict envmap2 = malloc(env_xres * env_yres * (3 * 4));
      void* const __restrict envmap3 = malloc(env_xres * env_yres * (3 * 4));
      const float sigma = (scale_factor - 1.f)*0.25f;
      float* const __restrict weights = (float*)malloc((xs * 2 + 1) * 4);
      for (int x = 0; x < (xs * 2 + 1); ++x)
         weights[x] = (1.f / sqrtf((float)(2.*M_PI)*sigma*sigma))*expf(-(float)((x - xs)*(x - xs)) / (2.f*sigma*sigma));

      // x-pass:

      for (int y = 0; y < (int)env_yres; ++y)
      {
         const int yoffs = y * env_xres * 3;
         for (int x = 0; x < (int)env_xres; ++x)
         {
            float sum_r = 0.f, sum_g = 0.f, sum_b = 0.f, sum_w = 0.f;
            for (int xt2 = 0; xt2 <= xs * 2; ++xt2)
            {
               int xt = xt2 + (x - xs);
               if (xt < 0)
                  xt += env_xres;
               else if (xt >= (int)env_xres)
                  xt -= env_xres;
               const float w = weights[xt2];
               const unsigned int offs = xt * 3 + yoffs;
               if (env_format == BaseTexture::RGB_FP16)
               {
                  sum_r += half2float(((const uint16_t*)envmap)[offs    ]) * w;
                  sum_g += half2float(((const uint16_t*)envmap)[offs + 1]) * w;
                  sum_b += half2float(((const uint16_t*)envmap)[offs + 2]) * w;
               }
               else
               {
                  sum_r += ((const float*)envmap)[offs    ] * w;
                  sum_g += ((const float*)envmap)[offs + 1] * w;
                  sum_b += ((const float*)envmap)[offs + 2] * w;
               }
               sum_w += w;
            }

            const unsigned int offs = (x + y * env_xres) * 3;
            const float inv_sum = 1.0f / sum_w;
            ((float*)envmap2)[offs    ] = sum_r * inv_sum;
            ((float*)envmap2)[offs + 1] = sum_g * inv_sum;
            ((float*)envmap2)[offs + 2] = sum_b * inv_sum;
         }
      }

      // y-pass:

      for (int y = 0; y < (int)env_yres; ++y)
         for (int x = 0; x < (int)env_xres; ++x)
         {
            float sum_r = 0.f, sum_g = 0.f, sum_b = 0.f, sum_w = 0.f;
            const int yt_end = min(y + xs, (int)env_yres - 1) - (y - xs);
            int offs = x * 3 + max(y - xs, 0)*(env_xres * 3);
            for (int yt = max(y - xs, 0) - (y - xs); yt <= yt_end; ++yt, offs += env_xres * 3)
            {
               const float w = weights[yt];
               sum_r += ((float*)envmap2)[offs] * w;
               sum_g += ((float*)envmap2)[offs + 1] * w;
               sum_b += ((float*)envmap2)[offs + 2] * w;
               sum_w += w;
            }

            offs = (x + y * env_xres) * 3;
            const float inv_sum = 1.0f / sum_w;
            ((float*)envmap3)[offs]     = sum_r * inv_sum;
            ((float*)envmap3)[offs + 1] = sum_g * inv_sum;
            ((float*)envmap3)[offs + 2] = sum_b * inv_sum;
         }

      envmap = envmap3;
      env_format = BaseTexture::RGB_FP32;
      free(envmap2);
      free(weights);
      free_envmap = true;
   }
#endif

   // brute force sampling over hemisphere for each normal direction of the to-be-(ir)radiance-baked environment
   // not the fastest solution, could do a "cosine convolution" over the picture instead (where also just 1024 or x samples could be used per pixel)
   //!! (note though that even 4096 samples can be too low if very bright spots (i.e. sun) in the image! see Delta_2k.hdr -> thus pre-filter enabled above!)
   // but with this implementation one can also have custom maps/LUTs for glossy, etc. later-on
   {
      ThreadPool pool(g_pvp->GetLogicalNumberOfProcessors());

      for (unsigned int y = 0; y < rad_env_yres; ++y) {
         pool.enqueue([y, rad_envmap, rad_format, rad_env_xres, rad_env_yres, envmap, env_format, env_xres, env_yres] {
            for (unsigned int x = 0; x < rad_env_xres; ++x)
            {
               // transfo from envmap to normal direction
               const float phi = (float)x / (float)rad_env_xres * (float)(2.0*M_PI) + (float)M_PI;
               const float theta = (float)y / (float)rad_env_yres * (float)M_PI;
               const Vertex3Ds n(sinf(theta) * cosf(phi), sinf(theta) * sinf(phi), cosf(theta));

               // draw x samples over hemisphere and collect cosine weighted environment map samples
               float sum_r = 0.f, sum_g = 0.f, sum_b = 0.f;

               constexpr unsigned int num_samples = 4096;
               for (unsigned int s = 0; s < num_samples; ++s)
               {
                  //!! discard directions pointing below the playfield?? or give them another "average playfield" color??
#define USE_ENVMAP_PRECALC_COSINE
#ifndef USE_ENVMAP_PRECALC_COSINE
                  //!! as we do not use importance sampling on the environment, just not being smart -could- be better for high frequency environments
                  Vertex3Ds l = sphere_sample((float)s*(float)(1.0/num_samples), radical_inverse(s)); // QMC hammersley point set
                  float NdotL = l.Dot(n);
                  if (NdotL < 0.0f) // flip if on backside of hemisphere
                  {
                     NdotL = -NdotL;
                     l = -l;
                  }
#else
                  //Vertex3Ds cos_hemisphere_sample(const Vertex3Ds &normal, Vertex2D uv) { float theta = (float)(2.*M_PI) * uv.x; uv.y = 2.f * uv.y - 1.f; Vertex3Ds spherePoint(sqrt(1.f - uv.y * uv.y) * Vertex2D(cosf(theta), sinf(theta)), uv.y); return normalize(normal + spherePoint); }
                  const Vertex3Ds l = rotate_to_vector_upper(cos_hemisphere_sample((float)s*(float)(1.0 / num_samples), radical_inverse(s)), n); // QMC hammersley point set
#endif
                  // transfo from light direction to envmap
                  // approximations seem to be good enough!
                  const float u = atan2_approx_div2PI(l.y, l.x) + 0.5f; //atan2f(l.y, l.x) * (float)(0.5 / M_PI) + 0.5f;
                  const float v = acos_approx_divPI(l.z); //acosf(l.z) * (float)(1.0 / M_PI);

                  float r, g, b;
                  unsigned int offs = (int)(u*(float)env_xres) + (int)(v*(float)env_yres)*env_xres;
                  if (offs >= env_yres * env_xres)
                     offs = 0;
                  switch (env_format)
                  {
                  case BaseTexture::RGB_FP16:
                  {
                     r = half2float(((const uint16_t*)envmap)[offs*3  ]);
                     g = half2float(((const uint16_t*)envmap)[offs*3+1]);
                     b = half2float(((const uint16_t*)envmap)[offs*3+2]);
                     break;
                  }
                  case BaseTexture::RGBA_FP16:
                  {
                     r = half2float(((const uint16_t*)envmap)[offs*4  ]);
                     g = half2float(((const uint16_t*)envmap)[offs*4+1]);
                     b = half2float(((const uint16_t*)envmap)[offs*4+2]);
                     break;
                  }
                  case BaseTexture::RGB_FP32:
                  {
                     r = ((const float*)envmap)[offs*3  ];
                     g = ((const float*)envmap)[offs*3+1];
                     b = ((const float*)envmap)[offs*3+2];
                     break;
                  }
                  case BaseTexture::RGBA_FP32:
                  {
                     r = ((const float*)envmap)[offs*4];
                     g = ((const float*)envmap)[offs*4+1];
                     b = ((const float*)envmap)[offs*4+2];
                     break;
                  }
                  case BaseTexture::RGB:
                  {
                     r = (float)((const uint8_t*)envmap)[offs*3  ] * (float)(1.0 / 255.0);
                     g = (float)((const uint8_t*)envmap)[offs*3+1] * (float)(1.0 / 255.0);
                     b = (float)((const uint8_t*)envmap)[offs*3+2] * (float)(1.0 / 255.0);
                     break;
                  }
                  case BaseTexture::RGBA:
                  {
                     const uint32_t rgb = ((const uint32_t*)envmap)[offs];
                     r = (float)(rgb & 0x00FF0000) * (float)(1.0 / 16711680.0);
                     g = (float)(rgb & 0x0000FF00) * (float)(1.0 /    65280.0);
                     b = (float)(rgb & 0x000000FF) * (float)(1.0 /      255.0);
                     break;
                  }
                  case BaseTexture::SRGB:
                  {
                     r = invGammaApprox((float)((const uint8_t*)envmap)[offs*3  ] * (float)(1.0 / 255.0));
                     g = invGammaApprox((float)((const uint8_t*)envmap)[offs*3+1] * (float)(1.0 / 255.0));
                     b = invGammaApprox((float)((const uint8_t*)envmap)[offs*3+2] * (float)(1.0 / 255.0));
                     break;
                  }
                  case BaseTexture::SRGBA:
                  {
                     const uint32_t rgb = ((const uint32_t*)envmap)[offs];
                     r = invGammaApprox((float)(rgb & 0x00FF0000) * (float)(1.0 / 16711680.0));
                     g = invGammaApprox((float)(rgb & 0x0000FF00) * (float)(1.0 /    65280.0));
                     b = invGammaApprox((float)(rgb & 0x000000FF) * (float)(1.0 /      255.0));
                     break;
                  }
                  default:
                     assert(!"unknown env_format");
                     break;
                  }
#ifndef USE_ENVMAP_PRECALC_COSINE
                  sum_r += r * NdotL;
                  sum_g += g * NdotL;
                  sum_b += b * NdotL;
#else
                  sum_r += r;
                  sum_g += g;
                  sum_b += b;
#endif
               }


               // average all samples
#ifndef USE_ENVMAP_PRECALC_COSINE
               sum_r *= (float)(2.0 / num_samples); // pre-divides by PI for final radiance/color lookup in shader
               sum_g *= (float)(2.0 / num_samples);
               sum_b *= (float)(2.0 / num_samples);
#else
               sum_r *= (float)(1.0 / num_samples); // pre-divides by PI for final radiance/color lookup in shader
               sum_g *= (float)(1.0 / num_samples);
               sum_b *= (float)(1.0 / num_samples);
#endif
               const unsigned int offs = (y*rad_env_xres + x) * 3;
               switch (rad_format)
               {
               case BaseTexture::RGB_FP16:
               {
                  ((uint16_t*)rad_envmap)[offs  ] = float2half_noLUT(sum_r);
                  ((uint16_t*)rad_envmap)[offs+1] = float2half_noLUT(sum_g);
                  ((uint16_t*)rad_envmap)[offs+2] = float2half_noLUT(sum_b);
                  break;
               }
               case BaseTexture::RGB_FP32:
               {
                  ((float*)rad_envmap)[offs  ] = sum_r;
                  ((float*)rad_envmap)[offs+1] = sum_g;
                  ((float*)rad_envmap)[offs+2] = sum_b;
                  break;
               }
               case BaseTexture::SRGB:
               {
                  rad_envmap[offs  ] = (uint8_t)clamp(gammaApprox(sum_r) * 255.f, 0.f, 255.f);
                  rad_envmap[offs+1] = (uint8_t)clamp(gammaApprox(sum_g) * 255.f, 0.f, 255.f);
                  rad_envmap[offs+2] = (uint8_t)clamp(gammaApprox(sum_b) * 255.f, 0.f, 255.f);
                  break;
               }
               default:
                     assert(!"unknown rad_format");
                     break;
               }
            }
         });
      }

      pool.wait_until_empty();
      pool.wait_until_nothing_in_flight();
   }

   /* ///!!! QA-test above multithreading implementation.
   //!! this is exactly the same code as above, so can be deleted at some point, as it only checks the multithreaded results with a singlethreaded implementation!
   for (unsigned int y = 0; y < rad_env_yres; ++y)
      for (unsigned int x = 0; x < rad_env_xres; ++x)
      {
         // trafo from envmap to normal direction
         const float phi = (float)x / (float)rad_env_xres * (float)(2.0*M_PI) + (float)M_PI;
         const float theta = (float)y / (float)rad_env_yres * (float)M_PI;
         const Vertex3Ds n(sinf(theta) * cosf(phi), sinf(theta) * sinf(phi), cosf(theta));

         // draw x samples over hemisphere and collect cosine weighted environment map samples
         float sum[3];
         sum[0] = sum[1] = sum[2] = 0.0f;

         const unsigned int num_samples = 4096;
         for (unsigned int s = 0; s < num_samples; ++s)
         {
            //!! discard directions pointing below the playfield?? or give them another "average playfield" color??
#define USE_ENVMAP_PRECALC_COSINE
#ifndef USE_ENVMAP_PRECALC_COSINE
            //!! as we do not use importance sampling on the environment, just not being smart -could- be better for high frequency environments
            Vertex3Ds l = sphere_sample((float)s*(float)(1.0 / num_samples), radical_inverse(s)); // QMC hammersley point set
            float NdotL = l.Dot(n);
            if (NdotL < 0.0f) // flip if on backside of hemisphere
            {
               NdotL = -NdotL;
               l = -l;
            }
#else
            //Vertex3Ds cos_hemisphere_sample(const Vertex3Ds &normal, Vertex2D uv) { float theta = (float)(2.*M_PI) * uv.x; uv.y = 2.f * uv.y - 1.f; Vertex3Ds spherePoint(sqrt(1.f - uv.y * uv.y) * Vertex2D(cosf(theta), sinf(theta)), uv.y); return normalize(normal + spherePoint); }
            const Vertex3Ds l = rotate_to_vector_upper(cos_hemisphere_sample((float)s*(float)(1.0 / num_samples), radical_inverse(s)), n); // QMC hammersley point set
#endif
            // trafo from light direction to envmap
            // approximations seem to be good enough!
            const float u = atan2_approx_div2PI(l.y, l.x) + 0.5f; //atan2f(l.y, l.x) * (float)(0.5 / M_PI) + 0.5f;
            const float v = acos_approx_divPI(l.z); //acosf(l.z) * (float)(1.0 / M_PI);

            float r, g, b;
            if (isHDR)
            {
               unsigned int offs = ((int)(u*(float)env_xres) + (int)(v*(float)env_yres)*env_xres) * 3;
               if (offs >= env_yres * env_xres * 3)
                  offs = 0;
               r = ((float*)envmap)[offs];
               g = ((float*)envmap)[offs + 1];
               b = ((float*)envmap)[offs + 2];
            }
            else
            {
               unsigned int offs = (int)(u*(float)env_xres) + (int)(v*(float)env_yres)*env_xres;
               if (offs >= env_yres * env_xres)
                  offs = 0;
               const uint32_t rgb = ((uint32_t*)envmap)[offs];
               r = invGammaApprox((float)(rgb & 255) * (float)(1.0 / 255.0));
               g = invGammaApprox((float)(rgb & 65280) * (float)(1.0 / 65280.0));
               b = invGammaApprox((float)(rgb & 16711680) * (float)(1.0 / 16711680.0));
            }
#ifndef USE_ENVMAP_PRECALC_COSINE
            sum[0] += r * NdotL;
            sum[1] += g * NdotL;
            sum[2] += b * NdotL;
#else
            sum[0] += r;
            sum[1] += g;
            sum[2] += b;
#endif
         }

         // average all samples
#ifndef USE_ENVMAP_PRECALC_COSINE
         sum[0] *= (float)(2.0 / num_samples); // pre-divides by PI for final radiance/color lookup in shader
         sum[1] *= (float)(2.0 / num_samples);
         sum[2] *= (float)(2.0 / num_samples);
#else
         sum[0] *= (float)(1.0 / num_samples); // pre-divides by PI for final radiance/color lookup in shader
         sum[1] *= (float)(1.0 / num_samples);
         sum[2] *= (float)(1.0 / num_samples);
#endif
         if (isHDR)
         {
            const unsigned int offs = (y*rad_env_xres + x) * 3;
            if (((float*)rad_envmap)[offs] != sum[0] ||
                ((float*)rad_envmap)[offs + 1] != sum[1] ||
                ((float*)rad_envmap)[offs + 2] != sum[2])
            {
               char tmp[911];
               sprintf(tmp, "%d %d %f=%f %f=%f %f=%f ", x, y, ((float*)rad_envmap)[offs], sum[0], ((float*)rad_envmap)[offs + 1], sum[1], ((float*)rad_envmap)[offs + 2], sum[2]);
               ::OutputDebugString(tmp);
            }
         }
         else
         {
            sum[0] = gammaApprox(sum[0]);
            sum[1] = gammaApprox(sum[1]);
            sum[2] = gammaApprox(sum[2]);
            if (
                ((uint32_t*)rad_envmap)[y*rad_env_xres + x] != ((int)(sum[0] * 255.0f)) | (((int)(sum[1] * 255.0f)) << 8) | (((int)(sum[2] * 255.0f)) << 16))
                g_pvp->MessageBox("Not OK", "Not OK", MB_OK);
         }
      }

   ///!!! */

#ifdef PREFILTER_ENVMAP_DIFFUSE
   if (free_envmap)
      free((void*)envmap);
#endif

#ifdef __OPENGLES__
   if (radTex->m_format == BaseTexture::SRGB || radTex->m_format == BaseTexture::RGB_FP16)
      radTex = radTex->NewWithAlpha();
#endif

   return radTex;
}

void Renderer::DrawBackground()
{
   const PinTable * const ptable = g_pplayer->m_ptable;
   Texture * const pin = ptable->GetDecalsEnabled() ? ptable->GetImage(ptable->m_BG_image[ptable->GetViewMode()]) : nullptr;
   m_renderDevice->ResetRenderState();
   m_renderDevice->SetRenderState(RenderState::CULLMODE, RenderState::CULL_CCW);
   if (pin)
   {
      m_renderDevice->Clear(clearType::ZBUFFER, 0);
      m_renderDevice->SetRenderState(RenderState::ZWRITEENABLE, RenderState::RS_FALSE);
      m_renderDevice->SetRenderState(RenderState::ZENABLE, RenderState::RS_FALSE);
      m_renderDevice->SetRenderState(RenderState::ALPHABLENDENABLE, RenderState::RS_FALSE);
      g_pplayer->m_renderer->DrawSprite(0.f, 0.f, 1.f, 1.f, 0xFFFFFFFF, m_renderDevice->m_texMan.LoadTexture(pin, false), ptable->m_ImageBackdropNightDay ? sqrtf(m_sceneLighting.GetGlobalEmissionScale()) : 1.0f, true);
   }
   else
   {
      const D3DCOLOR d3dcolor = COLORREF_to_D3DCOLOR(ptable->m_colorbackdrop);
      m_renderDevice->Clear(clearType::TARGET | clearType::ZBUFFER, d3dcolor);
   }
}

int Renderer::GetDisplayWidth() const
{
   return m_stereo3D == STEREO_SBS ? (GetBackBufferTexture()->GetWidth() * 2) : GetBackBufferTexture()->GetWidth();
}

int Renderer::GetDisplayHeight() const
{
   return (m_stereo3D == STEREO_TB || m_stereo3D == STEREO_INT || m_stereo3D == STEREO_FLIPPED_INT) ? (GetBackBufferTexture()->GetHeight() * 2) : GetBackBufferTexture()->GetHeight();
}

float Renderer::GetDisplayAspectRatio() const
{
   return (float)((double)GetDisplayWidth() / (double)GetDisplayHeight());
}

// Setup the tables camera / rotation / scale.
//
// 2 layout modes are supported:
// - Relative layout mode which is the default that has been there since the beginning
//   This mode computes the camera position by computing an approximate bounding box of the table, then offseting.
//   Flashers and primitive are ignored in the calculation of boundaries to center the table in the view
//   The view resulting from this mode is not orthonormal and (slightly) breaks stereo, lighting and reflections
//
// - Absolute layout mode which was added in 10.8
//   This mode computes the camera position by using an absolute coordinate system with origin at the bottom center of the table.
//
void Renderer::InitLayout(const float xpixoff, const float ypixoff)
{
   TRACE_FUNCTION();
   const ViewSetup& viewSetup = m_table->GetViewSetup();
   #if defined(ENABLE_OPENGL) || defined(ENABLE_BGFX)
   const bool stereo = m_stereo3Denabled && (m_stereo3D != STEREO_OFF) && (m_stereo3D != STEREO_VR);
   #elif defined(ENABLE_DX9)
   constexpr bool stereo = false;
   #endif
   viewSetup.ComputeMVP(m_table, GetDisplayAspectRatio(), stereo, *m_mvp, vec3(m_cam.x, m_cam.y, m_cam.z), m_inc, xpixoff / (float)GetDisplayWidth(), ypixoff / (float)GetDisplayHeight());
   SetupShaders();
}

Vertex3Ds Renderer::Unproject(const int width, const int height, const Vertex3Ds& point) const
{
   Matrix3D invMVP = m_mvp->GetModelViewProj(0);
   invMVP.Invert();
   const Vertex3Ds p(
      2.0f * point.x / static_cast<float>(width)  - 1.0f,
      1.0f - 2.0f * point.y / static_cast<float>(height),
      (point.z - 0.f /* MinZ */) / (1.f /* MaxZ */ - 0.f /* MinZ */));
   return invMVP * p;
}

Vertex3Ds Renderer::Get3DPointFrom2D(const int width, const int height, const Vertex2D& p, float z)
{
   const Vertex3Ds pNear(p.x, p.y, 0.f /* MinZ */);
   const Vertex3Ds pFar (p.x, p.y, 1.f /* MaxZ */);
   const Vertex3Ds p1 = Unproject(width, height, pNear);
   const Vertex3Ds p2 = Unproject(width, height, pFar);
   const float wz = z;
   const float wx = (wz - p1.z)*(p2.x - p1.x) / (p2.z - p1.z) + p1.x;
   const float wy = (wz - p1.z)*(p2.y - p1.y) / (p2.z - p1.z) + p1.y;
   return {wx, wy, wz};
}

void Renderer::SetupShaders()
{
   if (!m_shaderDirty)
      return;
   m_shaderDirty = false;

   const vec4 envEmissionScale_TexWidth(m_table->m_envEmissionScale * m_sceneLighting.GetGlobalEmissionScale(),
      static_cast<float>(m_envSampler->GetHeight()) /*+m_envSampler.m_width)*0.5f*/, 0.f, 0.f); //!! dto.

   UpdateBasicShaderMatrix();
   m_renderDevice->m_basicShader->SetTexture(SHADER_tex_env, m_envSampler);
   m_renderDevice->m_basicShader->SetVector(SHADER_fenvEmissionScale_TexWidth, &envEmissionScale_TexWidth);

   UpdateBallShaderMatrix();
   m_renderDevice->m_ballShader->SetVector(SHADER_fenvEmissionScale_TexWidth, &envEmissionScale_TexWidth);

   constexpr float Roughness = 0.8f;
   m_renderDevice->m_ballShader->SetVector(SHADER_Roughness_WrapL_Edge_Thickness, exp2f(10.0f * Roughness + 1.0f), 0.f, 1.f, 0.05f);
   const vec4 amb_lr = convertColor(m_table->m_lightAmbient, m_table->m_lightRange);
   m_renderDevice->m_ballShader->SetVector(SHADER_cAmbient_LightRange, 
      amb_lr.x * m_sceneLighting.GetGlobalEmissionScale(),
      amb_lr.y * m_sceneLighting.GetGlobalEmissionScale(),
      amb_lr.z * m_sceneLighting.GetGlobalEmissionScale(), m_table->m_lightRange);

   m_table->m_Light[0].pos.x = m_table->m_right * 0.5f;
   m_table->m_Light[1].pos.x = m_table->m_right * 0.5f;
   m_table->m_Light[0].pos.y = m_table->m_bottom * (float)(1.0 / 3.0);
   m_table->m_Light[1].pos.y = m_table->m_bottom * (float)(2.0 / 3.0);
   m_table->m_Light[0].pos.z = m_table->m_lightHeight;
   m_table->m_Light[1].pos.z = m_table->m_lightHeight;

   vec4 emission = convertColor(m_table->m_Light[0].emission, 1.f);
   emission.x *= m_table->m_lightEmissionScale * m_sceneLighting.GetGlobalEmissionScale();
   emission.y *= m_table->m_lightEmissionScale * m_sceneLighting.GetGlobalEmissionScale();
   emission.z *= m_table->m_lightEmissionScale * m_sceneLighting.GetGlobalEmissionScale();

#if defined(ENABLE_OPENGL) || defined(ENABLE_BGFX)
   float lightPos[MAX_LIGHT_SOURCES][4] = { };
   float lightEmission[MAX_LIGHT_SOURCES][4] = { };

   for (unsigned int i = 0; i < MAX_LIGHT_SOURCES; ++i)
   {
      memcpy(&lightPos[i], &m_table->m_Light[i].pos, sizeof(float) * 3);
      memcpy(&lightEmission[i], &emission, sizeof(float) * 3);
   }

   m_renderDevice->m_basicShader->SetFloat4v(SHADER_basicLightPos, (vec4*)lightPos, MAX_LIGHT_SOURCES);
   m_renderDevice->m_basicShader->SetFloat4v(SHADER_basicLightEmission, (vec4*)lightEmission, MAX_LIGHT_SOURCES);
#elif defined(ENABLE_DX9)
   struct CLight
   {
      float vPos[3];
      float vEmission[3];
   };
   CLight l[MAX_LIGHT_SOURCES];

   for (unsigned int i = 0; i < MAX_LIGHT_SOURCES; ++i)
   {
      memcpy(&l[i].vPos, &m_table->m_Light[i].pos, sizeof(float) * 3);
      memcpy(&l[i].vEmission, &emission, sizeof(float) * 3);
   }

   m_renderDevice->m_basicShader->SetFloat4v(SHADER_basicPackedLights, (vec4*)l, sizeof(CLight) * MAX_LIGHT_SOURCES / (4 * sizeof(float)));
#endif
}

void Renderer::UpdateBasicShaderMatrix(const Matrix3D& objectTrafo)
{
   struct
   {
      Matrix3D matWorld;
      Matrix3D matView;
      Matrix3D matWorldView;
      Matrix3D matWorldViewInverseTranspose;
      Matrix3D matWorldViewProj[2];
   } matrices;
   GetMVP().SetModel(objectTrafo);
   matrices.matWorld = GetMVP().GetModel();
   matrices.matView = GetMVP().GetView();
   matrices.matWorldView = GetMVP().GetModelView();
   matrices.matWorldViewInverseTranspose = GetMVP().GetModelViewInverseTranspose();
   const int nEyes = m_renderDevice->m_nEyes;
   for (int eye = 0; eye < nEyes; eye++)
      matrices.matWorldViewProj[eye] = GetMVP().GetModelViewProj(eye);

#if defined(ENABLE_DX9) || defined(ENABLE_BGFX)
   m_renderDevice->m_basicShader->SetMatrix(SHADER_matWorld, &matrices.matWorld);
   m_renderDevice->m_basicShader->SetMatrix(SHADER_matView, &matrices.matView);
   m_renderDevice->m_basicShader->SetMatrix(SHADER_matWorldView, &matrices.matWorldView);
   m_renderDevice->m_basicShader->SetMatrix(SHADER_matWorldViewInverseTranspose, &matrices.matWorldViewInverseTranspose);
   m_renderDevice->m_basicShader->SetMatrix(SHADER_matWorldViewProj, &matrices.matWorldViewProj[0], nEyes);
   m_renderDevice->m_flasherShader->SetMatrix(SHADER_matWorldViewProj, &matrices.matWorldViewProj[0], nEyes);
   m_renderDevice->m_lightShader->SetMatrix(SHADER_matWorldViewProj, &matrices.matWorldViewProj[0], nEyes);
   m_renderDevice->m_DMDShader->SetMatrix(SHADER_matWorldViewProj, &matrices.matWorldViewProj[0], nEyes);
#elif defined(ENABLE_OPENGL)
   m_renderDevice->m_basicShader->SetUniformBlock(SHADER_basicMatrixBlock, &matrices.matWorld.m[0][0]);
   m_renderDevice->m_flasherShader->SetMatrix(SHADER_matWorldViewProj, &matrices.matWorldViewProj[0], nEyes);
   m_renderDevice->m_lightShader->SetMatrix(SHADER_matWorldViewProj, &matrices.matWorldViewProj[0], nEyes);
   m_renderDevice->m_DMDShader->SetMatrix(SHADER_matWorldViewProj, &matrices.matWorldViewProj[0], nEyes);
#endif
}

void Renderer::UpdateBallShaderMatrix()
{
   struct
   {
      Matrix3D matView;
      Matrix3D matWorldView;
      Matrix3D matWorldViewInverse;
      Matrix3D matWorldViewProj[2];
   } matrices;
   GetMVP().SetModel(Matrix3D::MatrixIdentity());
   matrices.matView = GetMVP().GetView();
   matrices.matWorldView = GetMVP().GetModelView();
   matrices.matWorldViewInverse = GetMVP().GetModelViewInverse();
   const int nEyes = m_renderDevice->m_nEyes;
   for (int eye = 0; eye < nEyes; eye++)
      matrices.matWorldViewProj[eye] = GetMVP().GetModelViewProj(eye);

#if defined(ENABLE_DX9) || defined(ENABLE_BGFX)
   m_renderDevice->m_ballShader->SetMatrix(SHADER_matWorldViewProj, &matrices.matWorldViewProj[0], nEyes);
   m_renderDevice->m_ballShader->SetMatrix(SHADER_matWorldView, &matrices.matWorldView);
   m_renderDevice->m_ballShader->SetMatrix(SHADER_matWorldViewInverse, &matrices.matWorldViewInverse);
   m_renderDevice->m_ballShader->SetMatrix(SHADER_matView, &matrices.matView);
#elif defined(ENABLE_OPENGL)
   m_renderDevice->m_ballShader->SetUniformBlock(SHADER_ballMatrixBlock, &matrices.matView.m[0][0]);
#endif
}

void Renderer::UpdateDesktopBackdropShaderMatrix(bool basic, bool light, bool flasherDMD)
{
   Matrix3D matWorldViewProj[2]; // MVP to move from back buffer space (0..w, 0..h) to clip space (-1..1, -1..1)
   matWorldViewProj[0].SetIdentity();
   matWorldViewProj[0]._11 = 2.0f / (float)m_renderDevice->GetCurrentRenderTarget()->GetWidth();
   matWorldViewProj[0]._41 = -1.0f;
   matWorldViewProj[0]._22 = -2.0f / (float)m_renderDevice->GetCurrentRenderTarget()->GetHeight();
   matWorldViewProj[0]._42 = 1.0f;
   const int eyes = m_renderDevice->GetCurrentRenderTarget()->m_nLayers;
   if (eyes > 1)
      matWorldViewProj[1] = matWorldViewProj[0];

   if (basic)
   {
   #if defined(ENABLE_BGFX)
      m_renderDevice->m_basicShader->SetMatrix(SHADER_matWorldViewProj, &matWorldViewProj[0], eyes);
   #elif defined(ENABLE_OPENGL)
      struct
      {
         Matrix3D matWorld;
         Matrix3D matView;
         Matrix3D matWorldView;
         Matrix3D matWorldViewInverseTranspose;
         Matrix3D matWorldViewProj[2];
      } matrices;
      memcpy(&matrices.matWorldViewProj[0].m[0][0], &matWorldViewProj[0].m[0][0], 4 * 4 * sizeof(float));
      memcpy(&matrices.matWorldViewProj[1].m[0][0], &matWorldViewProj[0].m[0][0], 4 * 4 * sizeof(float));
      m_renderDevice->m_basicShader->SetUniformBlock(SHADER_basicMatrixBlock, &matrices.matWorld.m[0][0]);
   #elif defined(ENABLE_DX9)
      m_renderDevice->m_basicShader->SetMatrix(SHADER_matWorldViewProj, &matWorldViewProj[0]);
   #endif
   }

   if (light)
      m_renderDevice->m_lightShader->SetMatrix(SHADER_matWorldViewProj, &matWorldViewProj[0], eyes);
   
   if (flasherDMD)
   {
      m_renderDevice->m_flasherShader->SetMatrix(SHADER_matWorldViewProj, &matWorldViewProj[0], eyes);
      m_renderDevice->m_DMDShader->SetMatrix(SHADER_matWorldViewProj, &matWorldViewProj[0], eyes);
   }
}

void Renderer::UpdateStereoShaderState()
{
   if (m_renderDevice->m_stereoShader == nullptr)
      return;

   m_stereo3DDefocus = 0.f;
   if (IsAnaglyphStereoMode(m_stereo3D))
   {
      Anaglyph anaglyph;
      anaglyph.LoadSetupFromRegistry(clamp(m_stereo3D - STEREO_ANAGLYPH_1, 0, 9));
      anaglyph.SetupShader(m_renderDevice->m_stereoShader);
      // The defocus kernel size should depend on the render resolution but since this is a user tweak, this doesn't matter that much
      m_stereo3DDefocus = m_table->m_settings.GetPlayer_Stereo3DDefocus();
      // TODO I'm not 100% sure about this. I think the right way would be to select based on the transmitted luminance of the filter, the defocus 
      // being done on the lowest of the 2. Here we do on the single color channel, which is the same most of the time but not always (f.e. green/magenta)
      if (anaglyph.IsReversedColorPair())
         m_stereo3DDefocus = -m_stereo3DDefocus;
   }
   else
   {
      m_renderDevice->m_stereoShader->SetTechnique(m_stereo3D == STEREO_SBS ? SHADER_TECHNIQUE_stereo_SBS 
                                                 : m_stereo3D == STEREO_TB  ? SHADER_TECHNIQUE_stereo_TB
                                                 : m_stereo3D == STEREO_INT ? SHADER_TECHNIQUE_stereo_Int 
                                                 :                            SHADER_TECHNIQUE_stereo_Flipped_Int);
   }
}

static Texture* GetSegSDF(std::unique_ptr<Texture>& tex, const std::filesystem::path& path)
{
   if (tex == nullptr)
      tex.reset(Texture::CreateFromFile(path.string(), false));
   return tex.get();
}

void Renderer::SetupDisplayRenderer(const bool isBackdrop, Vertex3D_NoTex2* vertices, const vec4& emitterPad, const vec3& glassTint, const float glassRougness, ITexManCacheable* const glassTex,
   const vec4& glassArea, const vec3& glassAmbient)
{
   m_renderDevice->m_DMDShader->SetVector(SHADER_glassTint_Roughness, glassTint.x, glassTint.y, glassTint.z, // Glass tint
      glassRougness); // Glass roughness (high roughness leads to emitted light 'glowing' on glass)
   m_renderDevice->m_DMDShader->SetVector(SHADER_w_h_height, glassAmbient.x * 2.f, glassAmbient.y * 2.f,
      glassAmbient.z * 2.f, // Glass ambient color (only used when there is a glass texture)
      glassTex != nullptr ? 1.f : 0.f); // Apply glass texture or just uniform glass shading
   if (glassTex)
      m_renderDevice->m_DMDShader->SetTexture(SHADER_displayGlass, glassTex);
   float parallaxU = 0.f, parallaxV = 0.f;
   if (!isBackdrop && (vertices != nullptr))
   { // (fake) depth by applying some parallax mapping
      const Vertex3Ds v0(vertices[0].x, vertices[0].y, vertices[0].z);
      const Vertex3Ds v1(vertices[1].x, vertices[1].y, vertices[1].z);
      const Vertex3Ds v2(vertices[3].x, vertices[3].y, vertices[3].z);
      const Vertex2D u0(vertices[0].tu, vertices[0].tv);
      const Vertex2D u1(vertices[1].tu, vertices[1].tv);
      const Vertex2D u2(vertices[3].tu, vertices[3].tv);
      const Vertex3Ds dv1 = v1 - v0;
      const Vertex3Ds dv2 = v2 - v0;
      const Vertex2D duv1 = u1 - u0;
      const Vertex2D duv2 = u2 - u0;
      const float r = 1.0f / (duv1.x * duv2.y - duv1.y * duv2.x);
      Vertex3Ds tangent = (dv1 * duv2.y - dv2 * duv1.y) * r;
      Vertex3Ds bitangent = (dv2 * duv1.x - dv1 * duv2.x) * r;
      const Matrix3D& mv = GetMVP().GetModelView();
      tangent = mv.MultiplyVectorNoTranslate(tangent);
      bitangent = mv.MultiplyVectorNoTranslate(bitangent);
      Vertex3Ds eye = (v1 + v2) * 0.5f; // Assume a rectangle shape, use opposite corners to get its center
      eye = mv.MultiplyVectorNoPerspective(eye);
      eye.Normalize();
      const float tN = tangent.Length();
      const float btN = bitangent.Length();
      const float depth = CMTOVPU(0.5f); // depth between glass and display
      parallaxU = (depth / tN) * tangent.Dot(eye) / tN;
      parallaxV = (depth / btN) * bitangent.Dot(eye) / btN;
   }
   m_renderDevice->m_DMDShader->SetVector(SHADER_glassArea, &glassArea);
   m_renderDevice->m_DMDShader->SetVector(SHADER_glassPad, emitterPad.x - parallaxU, emitterPad.z + parallaxU, emitterPad.y - parallaxV, emitterPad.w + parallaxV);
}

void Renderer::SetupSegmentRenderer(int profile, const bool isBackdrop, const vec3& color, const float brightness, const SegmentFamily family, const SegElementType type, const float* segs, const ColorSpace colorSpace, Vertex3D_NoTex2* vertices,
   const vec4& emitterPad, const vec3& glassTint, const float glassRougness, ITexManCacheable* const glassTex, const vec4& glassArea, const vec3& glassAmbient)
{
   SetupDisplayRenderer(isBackdrop, vertices, emitterPad, glassTint, glassRougness, glassTex, glassArea, glassAmbient);

   Texture* segSDF = nullptr;
   switch (type)
   {
   case CTLPI_SEG_LAYOUT_7: segSDF = GetSegSDF(m_segDisplaySDF[family][0], 
        (family == SegmentFamily::Gottlieb) ? g_pvp->GetAppPath(VPinball::AppSubFolder::Assets, "7seg-gts.png")
      : (family == SegmentFamily::Bally)    ? g_pvp->GetAppPath(VPinball::AppSubFolder::Assets, "7seg-bally.png")
      : (family == SegmentFamily::Atari)    ? g_pvp->GetAppPath(VPinball::AppSubFolder::Assets, "7seg-atari.png")
                                            : g_pvp->GetAppPath(VPinball::AppSubFolder::Assets, "7seg-williams.png")); break;
   case CTLPI_SEG_LAYOUT_7C: segSDF = GetSegSDF(m_segDisplaySDF[family][1],
        (family == SegmentFamily::Bally)    ? g_pvp->GetAppPath(VPinball::AppSubFolder::Assets, "7seg-c-bally.png")
      : (family == SegmentFamily::Atari)    ? g_pvp->GetAppPath(VPinball::AppSubFolder::Assets, "7seg-c-atari.png")
                                            : g_pvp->GetAppPath(VPinball::AppSubFolder::Assets, "7seg-c-williams.png")); break;
   // TODO I did not found any reference for a dot only 7 segments display, so we use the comma one which is likely wrong
   case CTLPI_SEG_LAYOUT_7D: segSDF = GetSegSDF(m_segDisplaySDF[family][2], g_pvp->GetAppPath(VPinball::AppSubFolder::Assets, "7seg-c-williams.png")); break;
   case CTLPI_SEG_LAYOUT_9: segSDF = GetSegSDF(m_segDisplaySDF[family][3], g_pvp->GetAppPath(VPinball::AppSubFolder::Assets, "9seg-gts.png")); break;
   case CTLPI_SEG_LAYOUT_9C: segSDF = GetSegSDF(m_segDisplaySDF[family][4], g_pvp->GetAppPath(VPinball::AppSubFolder::Assets, "9seg-c-gts.png")); break;
   case CTLPI_SEG_LAYOUT_14: segSDF = GetSegSDF(m_segDisplaySDF[family][5], g_pvp->GetAppPath(VPinball::AppSubFolder::Assets, "14seg-williams.png")); break;
   case CTLPI_SEG_LAYOUT_14D: segSDF = GetSegSDF(m_segDisplaySDF[family][6], g_pvp->GetAppPath(VPinball::AppSubFolder::Assets, "14seg-d-williams.png")); break;
   case CTLPI_SEG_LAYOUT_14DC: segSDF = GetSegSDF(m_segDisplaySDF[family][7],
        (family == SegmentFamily::Gottlieb) ? g_pvp->GetAppPath(VPinball::AppSubFolder::Assets, "14seg-dc-gts.png")
                                            : g_pvp->GetAppPath(VPinball::AppSubFolder::Assets, "14seg-dc-williams.png")); break;
   case CTLPI_SEG_LAYOUT_16: segSDF = GetSegSDF(m_segDisplaySDF[family][8], g_pvp->GetAppPath(VPinball::AppSubFolder::Assets, "16seg.png")); break;
   }
   if (segSDF == nullptr)
      return;

   const float fullBrightness = brightness * m_segColor[profile].w;
   const vec4 segColor = vec4(color.x * m_segColor[profile].x, color.y * m_segColor[profile].y, color.z * m_segColor[profile].z, 0.f);
   m_renderDevice->m_DMDShader->SetVector(SHADER_vColor_Intensity, 
      segColor.x * fullBrightness, segColor.y * fullBrightness, segColor.z * fullBrightness, // Lit segment color
      m_segUnlitColor[profile].w); // Diffuse strength
   m_renderDevice->m_DMDShader->SetVector(SHADER_staticColor_Alpha,
      m_segUnlitColor[profile].x, m_segUnlitColor[profile].y, m_segUnlitColor[profile].z, // Unlit segment color (ambient)
      static_cast<float>(colorSpace)); // Output colorspace (3D render is linear, backdrop is tonemapped but needs sRGB conversion, dedicated window is tonemapped sRGB)
   m_renderDevice->m_DMDShader->SetFloat4v(SHADER_alphaSegState, reinterpret_cast<const vec4*>(segs), 4);
   m_renderDevice->m_DMDShader->SetTexture(SHADER_displayTex, segSDF, true, SF_TRILINEAR, SA_CLAMP, SA_CLAMP);
   m_renderDevice->m_DMDShader->SetTechnique(isBackdrop ? SHADER_TECHNIQUE_display_Seg : SHADER_TECHNIQUE_display_Seg_world);
}

void Renderer::SetupDMDRender(int profile, const bool isBackdrop, const vec3& color, const float brightness, const std::shared_ptr<BaseTexture>& dmd, const float alpha, const ColorSpace colorSpace, Vertex3D_NoTex2* vertices,
   const vec4& emitterPad, const vec3& glassTint, const float glassRougness, ITexManCacheable* const glassTex, const vec4& glassArea, const vec3& glassAmbient)
{
   // Legacy DMD renderer
   #ifdef ENABLE_BGFX
   if (m_dmdUseLegacyRenderer[profile])
   #else
   if (true)
   #endif
   {
      m_renderDevice->m_DMDShader->SetVector(SHADER_vColor_Intensity, color.x * brightness, color.y * brightness, color.z * brightness, dmd->m_format != BaseTexture::BW_FP32 ? 1.f : 0.f);
      m_renderDevice->m_DMDShader->SetVector(SHADER_vRes_Alpha_time, (float)dmd->width(), (float)dmd->height(), alpha, (float)(g_pplayer->m_overall_frames % 2048));
      m_renderDevice->m_DMDShader->SetVector(SHADER_glassArea, 0.f, 0.f, 1.f, 1.f);
      m_renderDevice->m_DMDShader->SetTechnique(isBackdrop ? SHADER_TECHNIQUE_basic_DMD : SHADER_TECHNIQUE_basic_DMD_world);
      m_renderDevice->m_DMDShader->SetTexture(SHADER_tex_dmd, dmd.get());
   }
   // New DMD renderer
   else
   {
      SetupDisplayRenderer(isBackdrop, vertices, emitterPad, glassTint, glassRougness, glassTex, glassArea, glassAmbient);

      const float fullBrightness = brightness * m_dmdDotColor[profile].w;
      const vec4 dotColor = dmd->m_format == BaseTexture::BW_FP32 ? vec4(color.x * m_dmdDotColor[profile].x, color.y * m_dmdDotColor[profile].y, color.z * m_dmdDotColor[profile].z, 0.f)
                                                             : vec4(color.x, color.y, color.z, 0.f);
      m_renderDevice->m_DMDShader->SetVector(SHADER_vColor_Intensity, 
         dotColor.x * fullBrightness, dotColor.y * fullBrightness, dotColor.z * fullBrightness, // Dot color (applied for luminance as well as sRGB frames)
         m_dmdDotProperties[profile].w); // Diffuse strength
      m_renderDevice->m_DMDShader->SetVector(SHADER_staticColor_Alpha,
         m_dmdUnlitDotColor[profile].x, m_dmdUnlitDotColor[profile].y, m_dmdUnlitDotColor[profile].z, // Unlit dot color (ambient)
         static_cast<float>(colorSpace)); // Output colorspace (3D render is linear, backdrop is tonemapped but needs sRGB conversion, dedicated window is tonemapped sRGB)
      m_renderDevice->m_DMDShader->SetVector(SHADER_vRes_Alpha_time, 
         static_cast<float>(dmd->width()), static_cast<float>(dmd->height()), // DMD size in dots
         0.f, 0.f); // Unused
      m_renderDevice->m_DMDShader->SetVector(SHADER_displayProperties,
         dmd->m_format != BaseTexture::BW_FP32 ? 1.f : 0.f, // luminance or (s)RGB frame source
         0.5f * (1.0f + (1.0f / (2.0f /*N_SAMPLES*/ + 0.5f)) * m_dmdDotProperties[profile].x / 2.0f), // Internal SDF offset to obtain 0.5 at dot border, increasing inside, decreasing outside
         0.5f + 0.5f * (m_dmdDotProperties[profile].x * (1.0f - m_dmdDotProperties[profile].y) /* Dot border darkening */), // Dot internal SDF threshold
         0.f); // Unused
      m_renderDevice->m_DMDShader->SetTexture(SHADER_displayTex, dmd.get());
      m_renderDevice->m_DMDShader->SetTechnique(isBackdrop ? SHADER_TECHNIQUE_display_DMD : SHADER_TECHNIQUE_display_DMD_world);
   }
}

void Renderer::SetupCRTRender(int profile, const bool isBackdrop, const vec3& color, const float brightness, const std::shared_ptr<BaseTexture>& crt, const float alpha,
   const ColorSpace colorSpace, Vertex3D_NoTex2* vertices, const vec4& emitterPad, const vec3& glassTint, const float glassRougness, ITexManCacheable* const glassTex, const vec4& glassArea,
   const vec3& glassAmbient)
{
   SetupDisplayRenderer(isBackdrop, vertices, emitterPad, glassTint, glassRougness, glassTex, glassArea, glassAmbient);

   m_renderDevice->m_DMDShader->SetVector(SHADER_vColor_Intensity, color.x * brightness, color.y * brightness,
      color.z * brightness, // CRT tint (applied for luminance as well as sRGB frames)
      1.0f); // Diffuse strength
   m_renderDevice->m_DMDShader->SetVector(SHADER_staticColor_Alpha, 0.f, 0.f, 0.f, // unused
      static_cast<float>(colorSpace)); // Output colorspace (3D render is linear, backdrop is tonemapped but needs sRGB conversion, dedicated window is tonemapped sRGB)
   m_renderDevice->m_DMDShader->SetVector(SHADER_vRes_Alpha_time, static_cast<float>(crt->width()), static_cast<float>(crt->height()), // CRT size in pixels
      0.f, 0.f); // Unused
   m_renderDevice->m_DMDShader->SetVector(SHADER_displayProperties,
      static_cast<float>(profile), // Render mode
      static_cast<float>(4 * crt->width()), static_cast<float>(4 * crt->height()), // Output size
      0.f); // Unused
   m_renderDevice->m_DMDShader->SetTexture(SHADER_displayTex, crt.get(), profile != 1 ? SF_NONE : SF_ANISOTROPIC);
   m_renderDevice->m_DMDShader->SetTechnique(isBackdrop ? SHADER_TECHNIQUE_display_CRT : SHADER_TECHNIQUE_display_CRT_world);
}

void Renderer::DrawBulbLightBuffer()
{
   RenderPass* const initial_rt = m_renderDevice->GetCurrentPass();
   static int id = 0; id++;

   // switch to 'bloom' output buffer to collect all bulb lights
   m_renderDevice->SetRenderTarget("Transmitted Light " + std::to_string(id) + " Clear", GetBloomBufferTexture(), false);
   m_renderDevice->ResetRenderState();
   m_renderDevice->Clear(clearType::TARGET, 0x00000000);

   // Draw bulb lights
   m_render_mask |= Renderer::LIGHT_BUFFER;
   m_renderDevice->SetRenderTarget("Transmitted Light " + std::to_string(id), GetBloomBufferTexture(), true, true);
   m_renderDevice->SetRenderState(RenderState::ZENABLE, RenderState::RS_FALSE); // disable all z-tests as zbuffer is in different resolution
   for (IEditable * const renderable : g_pplayer->m_vhitables)
      if (renderable->GetItemType() == eItemLight)
         RenderItem(renderable, true);
   m_render_mask &= ~Renderer::LIGHT_BUFFER;

   bool hasLight = m_renderDevice->GetCurrentPass()->GetCommandCount() > 0;
   if (hasLight)
   { // Only apply blur if we have actually rendered some lights
      RenderPass* renderPass = m_renderDevice->GetCurrentPass();
      m_renderDevice->DrawGaussianBlur(
         GetBloomBufferTexture(), 
         GetBloomTmpBufferTexture(), 
         GetBloomBufferTexture(), 19.f); // FIXME kernel size should depend on buffer resolution
      RenderPass * const blurPass2 = m_renderDevice->GetCurrentPass();
      RenderPass * const blurPass1 = blurPass2->m_dependencies[0];
      constexpr float margin = 0.05f; // margin for the blur
      blurPass1->m_areaOfInterest.x = renderPass->m_areaOfInterest.x - margin;
      blurPass1->m_areaOfInterest.y = renderPass->m_areaOfInterest.y - margin;
      blurPass1->m_areaOfInterest.z = renderPass->m_areaOfInterest.z + margin;
      blurPass1->m_areaOfInterest.w = renderPass->m_areaOfInterest.w + margin;
      blurPass2->m_areaOfInterest = blurPass1->m_areaOfInterest;
   }

   // Restore state and render target
   m_renderDevice->SetRenderTarget(initial_rt->m_name, initial_rt->m_rt);
   initial_rt->m_name += '-';

   #if defined(ENABLE_DX9)
   // For some reason, DirectX 9 will not handle correctly the null texture, so we just disable this optimization
   hasLight = true;
   #endif
   if (hasLight)
   {
      // Declare dependency on Bulb Light buffer (actually rendered to the bloom buffer texture)
      m_renderDevice->AddRenderTargetDependency(GetBloomBufferTexture());
      m_renderDevice->m_basicShader->SetTexture(SHADER_tex_base_transmission, GetBloomBufferTexture()->GetColorSampler());
   } 
   else
   {
      m_renderDevice->m_basicShader->SetTextureNull(SHADER_tex_base_transmission);
   }
}

void Renderer::DrawStatics()
{
   const unsigned int mask = m_render_mask;
   const bool isNoBackdrop = m_noBackdrop || ((m_render_mask & Renderer::REFLECTION_PASS) != 0);
   m_render_mask |= Renderer::STATIC_ONLY;
   for (IEditable* renderable : g_pplayer->m_vhitables)
      RenderItem(renderable, isNoBackdrop);
   m_render_mask = mask;
}

void Renderer::DrawDynamics(bool onlyBalls)
{
   const unsigned int mask = m_render_mask;
   const bool isNoBackdrop = m_noBackdrop || ((m_render_mask & Renderer::REFLECTION_PASS) != 0);
   m_render_mask |= Renderer::DYNAMIC_ONLY;
   if (onlyBalls)
   {
      for (HitBall* ball : g_pplayer->m_vball)
         RenderItem(ball->m_pBall, isNoBackdrop);
   }
   else
   {
      DrawBulbLightBuffer();
      for (IEditable* renderable : g_pplayer->m_vhitables)
         RenderItem(renderable, isNoBackdrop);
   }
   m_render_mask = mask;
}

void Renderer::DrawSprite(const float posx, const float posy, const float width, const float height, const COLORREF color, const std::shared_ptr<const Sampler>& tex, const float intensity, const bool backdrop)
{
   Vertex3D_NoTex2 vertices[4] =
   {
      { 1.f, 1.f, 0.f, 0.f, 0.f, 1.f, 1.f, 1.f },
      { 0.f, 1.f, 0.f, 0.f, 0.f, 1.f, 0.f, 1.f },
      { 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 1.f, 0.f },
      { 0.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f }
   };

   for (unsigned int i = 0; i < 4; ++i)
   {
      vertices[i].x =        (vertices[i].x * width  + posx)*2.0f - 1.0f;
      vertices[i].y = 1.0f - (vertices[i].y * height + posy)*2.0f;
   }

   const vec4 c = convertColor(color, intensity);
   m_renderDevice->m_DMDShader->SetVector(SHADER_vColor_Intensity, &c);
   m_renderDevice->m_DMDShader->SetTechnique(tex ? SHADER_TECHNIQUE_basic_noDMD : SHADER_TECHNIQUE_basic_noDMD_notex);
   if (tex)
      m_renderDevice->m_DMDShader->SetTexture(SHADER_tex_sprite, tex, SF_TRILINEAR, SA_CLAMP, SA_CLAMP);
   m_renderDevice->m_DMDShader->SetVector(SHADER_glassArea, 0.f, 0.f, 1.f, 1.f);
   m_renderDevice->SetRenderState(RenderState::ZENABLE, RenderState::RS_FALSE);
   m_renderDevice->DrawTexturedQuad(m_renderDevice->m_DMDShader, vertices);
   m_renderDevice->GetCurrentPass()->m_commands.back()->SetTransparent(true);
   m_renderDevice->GetCurrentPass()->m_commands.back()->SetDepth(-10000.f);
}

void Renderer::DrawWireframe(IEditable* renderable, const vec4& fillColor, const vec4& edgeColor, bool withDepthMask)
{
   unsigned int prevRenderMask = m_render_mask;
   m_renderDevice->ResetRenderState();
   m_renderDevice->EnableAlphaBlend(false);
   m_renderDevice->SetRenderState(RenderState::ZENABLE, withDepthMask ? RenderState::RS_TRUE : RenderState::RS_FALSE);
   m_renderDevice->SetRenderState(RenderState::ZWRITEENABLE, withDepthMask ? RenderState::RS_TRUE : RenderState::RS_FALSE);
   m_renderDevice->SetRenderState(RenderState::CULLMODE, RenderState::CULL_NONE);
   m_renderDevice->m_basicShader->SetTechnique(SHADER_TECHNIQUE_unshaded_without_texture);
   if (fillColor.w > 0.f)
   {
      m_render_mask = Renderer::RenderMask::UI_FILL;
      m_renderDevice->SetRenderState(RenderState::ZFUNC, RenderState::Z_LESS);
      m_renderDevice->SetRenderState(RenderState::ALPHABLENDENABLE, fillColor.w == 1.f ? RenderState::RS_FALSE : RenderState::RS_TRUE);
      m_renderDevice->m_basicShader->SetVector(SHADER_staticColor_Alpha, &fillColor);
      RenderItem(renderable, false);
   }
   if (edgeColor.w > 0.f)
   {
      //UpdateBasicShaderMatrix(Matrix3D::MatrixTranslate(-GetMVP().GetModelView().GetOrthoNormalDir()));
      m_render_mask = Renderer::RenderMask::UI_EDGES;
      m_renderDevice->SetRenderState(RenderState::ZFUNC, RenderState::Z_LESSEQUAL);
      m_renderDevice->SetRenderState(RenderState::ALPHABLENDENABLE, edgeColor.w == 1.f ? RenderState::RS_FALSE : RenderState::RS_TRUE);
      m_renderDevice->m_basicShader->SetVector(SHADER_staticColor_Alpha, &edgeColor);
      RenderItem(renderable, false);
      //UpdateBasicShaderMatrix();
   }
   m_renderDevice->m_basicShader->SetVector(SHADER_staticColor_Alpha, 1.f, 1.f, 1.f, 1.f);
   m_render_mask = prevRenderMask;
}

void Renderer::RenderItem(IEditable* renderable, bool isNoBackdrop)
{
   if ((isNoBackdrop && renderable->m_backglass) // Don't render backdrop items in reflections or VR & cabinet modes
      || (renderable->GetPartGroup() != nullptr && ((renderable->GetPartGroup()->GetPlayerModeVisibilityMask() & m_visibilityMask) == 0))) // Apply player mode visibility mask
      return;

   const PartGroupData::SpaceReference spaceReference = renderable->GetPartGroup() ? renderable->GetPartGroup()->GetReferenceSpace() : PartGroupData::SpaceReference::SR_PLAYFIELD;
   if (m_mvpSpaceReference != spaceReference)
   {
      #if defined(ENABLE_XR)
      if (m_stereo3D == STEREO_VR)
      {
         g_pplayer->m_vrDevice->UpdateVRPosition(spaceReference, GetMVP());
      }
      else
      #endif
      {
         switch (spaceReference)
         {
         case PartGroupData::SpaceReference::SR_CABINET:
         case PartGroupData::SpaceReference::SR_CABINET_FEET:
         case PartGroupData::SpaceReference::SR_ROOM:
            m_mvp->SetView(g_pplayer->m_ptable->GetDefaultPlayfieldToCabMatrix() * m_playfieldView);
            break;

         case PartGroupData::SpaceReference::SR_PLAYFIELD:
         default:
            m_mvp->SetView(m_playfieldView);
            break;
         }
      }
      m_mvpSpaceReference = spaceReference;
      UpdateBasicShaderMatrix();
      UpdateBallShaderMatrix();
   }

   renderable->GetIHitable()->Render(m_render_mask);
}


void Renderer::RenderStaticPrepass()
{
   // For VR, we don't use any static pre-rendering
   if (m_stereo3D == STEREO_VR)
      return;

   if (!m_isStaticPrepassDirty)
      return;

   #if defined(ENABLE_OPENGL) && defined(__STANDALONE__)
   SDL_GL_MakeCurrent(g_pplayer->m_playfieldWnd->GetCore(), g_pplayer->m_renderer->m_renderDevice->m_sdl_context);
   #endif

   m_isStaticPrepassDirty = false;

   TRACE_FUNCTION();

   m_render_mask |= Renderer::STATIC_ONLY;
   const bool isNoBackdrop = m_noBackdrop || ((m_render_mask & Renderer::REFLECTION_PASS) != 0);

   // The code will fail if the static render target is MSAA (the copy operation we are performing is not allowed)
   delete m_staticPrepassRT;
   m_staticPrepassRT = GetBackBufferTexture()->Duplicate("StaticPreRender"s);
   assert(!m_staticPrepassRT->IsMSAA());

   RenderTarget *accumulationSurface = IsUsingStaticPrepass() ? m_staticPrepassRT->Duplicate("Accumulation"s) : nullptr;

   RenderTarget* renderRT = GetAOMode() == 1 ? GetBackBufferTexture() : m_staticPrepassRT;

   if (IsUsingStaticPrepass())
   {
      PLOGI << "Performing prerendering of static parts."; // For profiling
      // if rendering static/with heavy oversampling, disable mipmaps & aniso/trilinear filter to get a sharper/more precise result overall!
      ShaderState::m_disableMipmaps = true;
      #ifdef ENABLE_BGFX
         m_renderDevice->m_DMDShader->SetVector(SHADER_u_basic_shade_mode, 0.f, 0.f, 0.f, 1.f);
      #endif
   }

   //#define STATIC_PRERENDER_ITERATIONS_KOROBOV 7.0 // for the (commented out) lattice-based QMC oversampling, 'magic factor', depending on the number of iterations!
   // loop for X times and accumulate/average these renderings
   // NOTE: iter == 0 MUST ALWAYS PRODUCE an offset of 0,0!
   int n_iter = IsUsingStaticPrepass() ? (STATIC_PRERENDER_ITERATIONS - 1) : 0;
   for (int iter = n_iter; iter >= 0; --iter) // just do one iteration if in dynamic camera/light/material tweaking mode
   {
      #ifdef MSVC_CONCURRENCY_VIEWER
      span* tagSpan = new span(series, 1, _T("PreRender"));
      #endif

      int progress = 70 + (((30 * (n_iter + 1 - iter)) / (n_iter + 1)));
      g_pplayer->m_progressDialog.SetProgress("Prerendering Static Parts..."s, progress);
#ifdef __LIBVPINBALL__
      VPinballLib::ProgressData progressData = { (n_iter - iter) * 100 / n_iter };
      VPinballLib::VPinballLib::SendEvent(VPINBALL_EVENT_PRERENDERING, &progressData);
#endif
      m_renderDevice->m_curDrawnTriangles = 0;

      float u1 = xyLDBNbnot[iter*2  ];  //      (float)iter*(float)(1.0                                /STATIC_PRERENDER_ITERATIONS);
      float u2 = xyLDBNbnot[iter*2+1];  //fmodf((float)iter*(float)(STATIC_PRERENDER_ITERATIONS_KOROBOV/STATIC_PRERENDER_ITERATIONS), 1.f);
      // the following line implements filter importance sampling for a small gauss (i.e. less jaggies as it also samples neighboring pixels) -> but also potentially more artifacts in compositing!
      gaussianDistribution(u1, u2, 0.5f, 0.0f); //!! first 0.5 could be increased for more blur, but is pretty much what is recommended
      // sanity check to be sure to limit filter area to 3x3 in practice, as the gauss transformation is unbound (which is correct, but for our use-case/limited amount of samples very bad)
      assert(u1 > -1.5f && u1 < 1.5f);
      assert(u2 > -1.5f && u2 < 1.5f);
      // Last iteration MUST set a sample offset of 0,0 so that final depth buffer features 'correctly' centered pixel sample
      assert(iter != 0 || (u1 == 0.f && u2 == 0.f));

      // Setup Camera,etc matrices for each iteration.
      InitLayout(u1, u2);

      // Direct all renders to the "static" buffer
      m_renderDevice->SetRenderTarget("PreRender Background"s, renderRT, iter == 0, true); // First iteration needs to declare a dependency on what is already there (if any) to avoid discarding it in final render frame
      DrawBackground();

      m_renderDevice->SetRenderTarget("PreRender Draw"s, renderRT, true, true); // Force new pass to avoid sorting background draw calls with 3D rendering draw calls

      if (IsUsingStaticPrepass())
      {
         // Mark all probes to be re-rendered for this frame (only if needed, lazily rendered)
         for (size_t i = 0; i < m_table->m_vrenderprobe.size(); ++i)
            m_table->m_vrenderprobe[i]->MarkDirty();

         // Render static parts
         UpdateBasicShaderMatrix();
         for (IEditable* renderable : g_pplayer->m_vhitables)
            RenderItem(renderable, isNoBackdrop);

         // Rendering is done to the static render target then accumulated to accumulationSurface
         // We use the framebuffer mirror shader which copies a weighted version of the bound texture
         m_renderDevice->SetRenderTarget("PreRender Accumulate"s, accumulationSurface);
         m_renderDevice->AddRenderTargetDependency(renderRT);
         m_renderDevice->ResetRenderState();
         m_renderDevice->SetRenderState(RenderState::ALPHABLENDENABLE, iter == STATIC_PRERENDER_ITERATIONS - 1 ? RenderState::RS_FALSE : RenderState::RS_TRUE);
         m_renderDevice->SetRenderState(RenderState::SRCBLEND, RenderState::ONE);
         m_renderDevice->SetRenderState(RenderState::DESTBLEND, RenderState::ONE);
         m_renderDevice->SetRenderState(RenderState::BLENDOP, RenderState::BLENDOP_ADD);
         m_renderDevice->SetRenderState(RenderState::ZENABLE, RenderState::RS_FALSE);
         m_renderDevice->SetRenderState(RenderState::ZWRITEENABLE, RenderState::RS_FALSE);
         m_renderDevice->SetRenderState(RenderState::CULLMODE, RenderState::CULL_NONE);
         m_renderDevice->m_FBShader->SetTechnique(SHADER_TECHNIQUE_fb_mirror);
         m_renderDevice->m_FBShader->SetVector(SHADER_w_h_height, 
            (float)(1.0 / (double)renderRT->GetWidth()), (float)(1.0 / (double)renderRT->GetHeight()),
            (float)((double)STATIC_PRERENDER_ITERATIONS), 1.0f);
         m_renderDevice->m_FBShader->SetTexture(SHADER_tex_fb_unfiltered, renderRT->GetColorSampler());
         m_renderDevice->DrawFullscreenTexturedQuad(m_renderDevice->m_FBShader);
         m_renderDevice->m_FBShader->SetTextureNull(SHADER_tex_fb_unfiltered);
      }

      #ifdef MSVC_CONCURRENCY_VIEWER
      delete tagSpan;
      #endif

      m_renderDevice->SubmitRenderFrame(); // Submit to avoid stacking up all prerender passes in a huge render frame
   }

   if (accumulationSurface)
   {
      // copy back weighted antialiased color result to the static render target, keeping depth untouched
      m_renderDevice->SetRenderTarget("PreRender Store"s, renderRT);
      m_renderDevice->BlitRenderTarget(accumulationSurface, renderRT, true, false);
      m_renderDevice->AddEndOfFrameCmd([accumulationSurface]() { delete accumulationSurface; });
   }

   ShaderState::m_disableMipmaps = false;
   #ifdef ENABLE_BGFX
      m_renderDevice->m_DMDShader->SetVector(SHADER_u_basic_shade_mode, 0.f, 0.f, 0.f, 0.f);
   #endif

   // Now finalize static buffer with static AO
   if (GetAOMode() == 1)
   {
      PLOGI << "Starting static AO prerendering"; // For profiling

      const bool useAA = m_renderWidth > GetBackBufferTexture()->GetWidth();

      m_renderDevice->SetRenderTarget("PreRender AO Save Depth"s, m_staticPrepassRT);
      m_renderDevice->ResetRenderState();
      m_renderDevice->BlitRenderTarget(renderRT, m_staticPrepassRT, false, true);

      m_renderDevice->SetRenderState(RenderState::ALPHABLENDENABLE, RenderState::RS_FALSE);
      m_renderDevice->SetRenderState(RenderState::CULLMODE ,RenderState::CULL_NONE);
      m_renderDevice->SetRenderState(RenderState::ZWRITEENABLE, RenderState::RS_FALSE);
      m_renderDevice->SetRenderState(RenderState::ZENABLE, RenderState::RS_FALSE);

      m_renderDevice->m_FBShader->SetTexture(SHADER_tex_depth, renderRT->GetDepthSampler());
      m_renderDevice->m_FBShader->SetTexture(SHADER_tex_ao_dither, m_aoDitherSampler);
      m_renderDevice->m_FBShader->SetVector(SHADER_AO_scale_timeblur, m_table->m_AOScale, 0.1f, 0.f, 0.f);
      m_renderDevice->m_FBShader->SetTechnique(SHADER_TECHNIQUE_AO);

      for (unsigned int i = 0; i < 50; ++i) // 50 iterations to get AO smooth
      {
         m_renderDevice->SetRenderTarget("PreRender AO"s, GetAORenderTarget(0));
         m_renderDevice->AddRenderTargetDependency(renderRT);
         m_renderDevice->AddRenderTargetDependency(GetAORenderTarget(1));
         if (i == 0)
            m_renderDevice->Clear(clearType::TARGET, 0x00000000);

         m_renderDevice->m_FBShader->SetTexture(SHADER_tex_fb_filtered, GetAORenderTarget(1)->GetColorSampler()); //!! ?
         m_renderDevice->m_FBShader->SetTexture(SHADER_tex_fb_unfiltered, GetAORenderTarget(1)->GetColorSampler()); //!! ?
         m_renderDevice->m_FBShader->SetVector(SHADER_w_h_height, 
            (float)(1.0 / GetAORenderTarget(1)->GetWidth()), (float)(1.0 / GetAORenderTarget(1)->GetHeight()),
            radical_inverse(i) * (float)(1. / 8.0), /*sobol*/ radical_inverse<3>(i) * (float)(1. / 8.0)); // jitter within (64/8)x(64/8) neighborhood of 64x64 tex, good compromise between blotches and noise
         m_renderDevice->DrawFullscreenTexturedQuad(m_renderDevice->m_FBShader);

         // flip AO buffers (avoids copy)
         SwapAORenderTargets();
      }

      m_renderDevice->m_FBShader->SetTextureNull(SHADER_tex_depth);

      m_renderDevice->SetRenderTarget("PreRender Apply AO"s, m_staticPrepassRT);
      m_renderDevice->AddRenderTargetDependency(renderRT);
      m_renderDevice->AddRenderTargetDependency(GetAORenderTarget(1));

      m_renderDevice->m_FBShader->SetTexture(SHADER_tex_fb_filtered, renderRT->GetColorSampler());
      m_renderDevice->m_FBShader->SetTexture(SHADER_tex_fb_unfiltered, renderRT->GetColorSampler());
      m_renderDevice->m_FBShader->SetTexture(SHADER_tex_ao, GetAORenderTarget(1)->GetColorSampler());

      m_renderDevice->m_FBShader->SetVector(SHADER_w_h_height, (float)(1.0 / renderRT->GetWidth()), (float)(1.0 / renderRT->GetHeight()), 1.0f, 1.0f);
      m_renderDevice->m_FBShader->SetTechnique(useAA ? SHADER_TECHNIQUE_fb_AO_static : SHADER_TECHNIQUE_fb_AO_no_filter_static);

      m_renderDevice->DrawFullscreenTexturedQuad(m_renderDevice->m_FBShader);

      // Delete buffers: we won't need them anymore since dynamic AO is disabled
      m_renderDevice->AddEndOfFrameCmd([this]() { ReleaseAORenderTargets(); });

      m_renderDevice->m_FBShader->SetTextureNull(SHADER_tex_ao);
   }

   if (GetMSAABackBufferTexture()->IsMSAA())
   {
      // Render one frame with MSAA to keep MSAA depth (this adds MSAA to the overlapping parts between statics & dynamics)
      RenderTarget* const renderRTmsaa = GetMSAABackBufferTexture()->Duplicate("MSAAPreRender"s);
      InitLayout();
      m_renderDevice->SetRenderTarget("PreRender MSAA Background"s, renderRTmsaa, false);
      DrawBackground();
      if (IsUsingStaticPrepass())
      {
         m_renderDevice->SetRenderTarget("PreRender MSAA Scene"s, renderRTmsaa, true, true); // Force new pass to avoid sorting scene calls with background calls
         m_renderDevice->ResetRenderState();
         for (size_t i = 0; i < m_table->m_vrenderprobe.size(); ++i)
            m_table->m_vrenderprobe[i]->MarkDirty();
         UpdateBasicShaderMatrix();
         for (IEditable* renderable : g_pplayer->m_vhitables)
            RenderItem(renderable, isNoBackdrop);
      }
      // Copy supersampled color buffer
      m_renderDevice->SetRenderTarget("PreRender Combine Color"s, renderRTmsaa, true, true); // Force new pass to avoid sorting blit call with background calls
      m_renderDevice->BlitRenderTarget(m_staticPrepassRT, renderRTmsaa, true, false);
      // Replace with this new MSAA pre render
      RenderTarget *initialPreRender = m_staticPrepassRT;
      m_staticPrepassRT = renderRTmsaa;
      m_renderDevice->AddEndOfFrameCmd([initialPreRender]() { delete initialPreRender; });
   }
   m_renderDevice->SubmitRenderFrame(); // Submit frame as other rendering will not declare a dependency on the created passes and therefore they would be discarded

   if (IsUsingStaticPrepass())
   {
      PLOGI << "Starting Reflection Probe prerendering"; // For profiling
      for (RenderProbe* probe : m_table->m_vrenderprobe)
         probe->PreRenderStatic();
   }

   // Store the total number of triangles prerendered (including ones done for render probes)
   m_statsDrawnStaticTriangles = m_renderDevice->m_curDrawnTriangles;
   m_render_mask &= ~Renderer::STATIC_ONLY;

   PLOGI << "Static PreRender done"; // For profiling
}

void Renderer::RenderDynamics()
{
   if (g_pplayer->GetInfoMode() == IF_STATIC_ONLY)
      return;

   TRACE_FUNCTION();

   // Mark all probes to be re-rendered for this frame (only if needed, lazily rendered)
   for (size_t i = 0; i < m_table->m_vrenderprobe.size(); ++i)
      m_table->m_vrenderprobe[i]->MarkDirty();

   // Setup the projection matrices used for refraction
   Matrix3D matProj[2];
   const int nEyes = m_renderDevice->m_nEyes;
   for (int eye = 0; eye < nEyes; eye++)
      matProj[eye] = GetMVP().GetProj(eye);
   m_renderDevice->m_basicShader->SetMatrix(SHADER_matProj, &matProj[0], nEyes);
   m_renderDevice->m_ballShader->SetMatrix(SHADER_matProj, &matProj[0], nEyes);

   // Update ball pos uniforms
   vec4 balls[MAX_BALL_SHADOW];
   int p = 0;
   for (size_t i = 0; i < g_pplayer->m_vball.size() && p < MAX_BALL_SHADOW; i++)
   {
      HitBall* const pball = g_pplayer->m_vball[i];
      if (!pball->m_pBall->m_d.m_visible)
         continue;
      balls[p] = vec4(pball->m_d.m_pos.x, pball->m_d.m_pos.y, pball->m_d.m_pos.z, pball->m_d.m_radius);
      p++;
   }
   for (; p < MAX_BALL_SHADOW; p++)
      balls[p] = vec4(-1000.f, -1000.f, -1000.f, 0.0f);
   m_renderDevice->m_lightShader->SetFloat4v(SHADER_balls, balls, MAX_BALL_SHADOW);
   m_renderDevice->m_basicShader->SetFloat4v(SHADER_balls, balls, MAX_BALL_SHADOW);
   m_renderDevice->m_flasherShader->SetFloat4v(SHADER_balls, balls, MAX_BALL_SHADOW);

   UpdateBasicShaderMatrix();
   UpdateBallShaderMatrix();

   #ifdef OPEN_VR
   // Render the default backglass without depth write before the table so that it will be visible for tables without a VR backglass but overwriten otherwise
   if (m_backGlass != nullptr)
      m_backGlass->Render();
   #endif

   if (m_shadeMode == ShadeMode::Default)
   {
      const unsigned int mask = m_render_mask;
      const bool isNoBackdrop = m_noBackdrop || ((m_render_mask & Renderer::REFLECTION_PASS) != 0) || g_pplayer->m_liveUI->IsEditorViewMode();
      m_render_mask |= IsUsingStaticPrepass() ? Renderer::DYNAMIC_ONLY : Renderer::DEFAULT;
      DrawBulbLightBuffer();
      for (IEditable* renderable : g_pplayer->m_vhitables)
         RenderItem(renderable, isNoBackdrop);
      m_render_mask = mask;
   }
   else
   {
      const vec4 fillColor = m_shadeMode == ShadeMode::NoDepthWireframe ? vec4(0.f, 0.f, 0.f, (float)(32. / 255.)) : vec4((float)(32. / 255.), (float)(32. / 255.), (float)(32. / 255.), 1.f);
      const vec4 edgeColor(0.f, 0.f, 0.f, 1.f);
      for (IEditable* renderable : g_pplayer->m_vhitables)
         DrawWireframe(renderable, fillColor, edgeColor, m_shadeMode != ShadeMode::NoDepthWireframe);
   }

   m_renderDevice->m_basicShader->SetTextureNull(SHADER_tex_base_transmission); // need to reset the bulb light texture, as its used as render target for bloom again

   for (size_t i = 0; i < m_table->m_vrenderprobe.size(); ++i)
      m_table->m_vrenderprobe[i]->ApplyAreaOfInterest();
}



#pragma region PostProcess

void Renderer::SetScreenOffset(const float x, const float y)
{
   const float rotation = m_stereo3D == STEREO_VR ? 0.f : ANGTORAD(m_table->GetViewSetup().GetRotation(m_stereo3D, m_renderDevice->GetOutputBackBuffer()->GetWidth(), m_renderDevice->GetOutputBackBuffer()->GetHeight()));
   const float c = cosf(-rotation), s = sinf(-rotation);
   m_ScreenOffset.x = x * c - y * s;
   m_ScreenOffset.y = x * s + y * c;
}

void Renderer::UpdateAmbientOcclusion(RenderTarget* renderedRT)
{
   if (GetAOMode() != 2) // Only process for dynamic AO
      return;

   m_renderDevice->ResetRenderState();
   m_renderDevice->SetRenderState(RenderState::ALPHABLENDENABLE, RenderState::RS_FALSE);
   m_renderDevice->SetRenderState(RenderState::CULLMODE, RenderState::CULL_NONE);
   m_renderDevice->SetRenderState(RenderState::ZWRITEENABLE, RenderState::RS_FALSE);
   m_renderDevice->SetRenderState(RenderState::ZENABLE, RenderState::RS_FALSE);

   // separate normal generation pass, currently roughly same perf or even much worse
   /* m_renderDevice->SetRenderTarget(m_pd3dDevice->GetPostProcessRenderTarget1()); //!! expects stereo or FXAA enabled
   m_renderDevice->m_FBShader->SetTexture(SHADER_tex_depth, m_pdds3DZBuffer, true);
   m_renderDevice->m_FBShader->SetVector(SHADER_w_h_height, (float)(1.0 / m_width), (float)(1.0 / m_height),
      radical_inverse(m_overall_frames%2048)*(float)(1. / 8.0), sobol(m_overall_frames%2048)*(float)(5. / 8.0));// jitter within lattice cell //!! ?
   m_renderDevice->m_FBShader->SetTechnique("normals");
   m_renderDevice->DrawFullscreenTexturedQuad(m_renderDevice->m_FBShader);*/

   m_renderDevice->SetRenderTarget("ScreenSpace AO"s, GetAORenderTarget(0), false);
   m_renderDevice->AddRenderTargetDependency(GetAORenderTarget(1));
   m_renderDevice->m_FBShader->SetTexture(SHADER_tex_fb_filtered, GetAORenderTarget(1)->GetColorSampler());
   m_renderDevice->AddRenderTargetDependency(GetBackBufferTexture(), true);
   m_renderDevice->m_FBShader->SetTexture(SHADER_tex_depth, GetBackBufferTexture()->GetDepthSampler());
   //m_renderDevice->m_FBShader->SetTexture(SHADER_Texture1, m_pd3dDevice->GetPostProcessRenderTarget1()); // temporary normals
   m_renderDevice->m_FBShader->SetTexture(SHADER_tex_ao_dither, m_aoDitherSampler);
   m_renderDevice->m_FBShader->SetVector(SHADER_w_h_height, (float)(1.0 / GetAORenderTarget(1)->GetWidth()), (float)(1.0 / GetAORenderTarget(1)->GetHeight()),
      radical_inverse(g_pplayer->m_overall_frames % 2048) * (float)(1. / 8.0),
      /*sobol*/ radical_inverse<3>(g_pplayer->m_overall_frames % 2048)
         * (float)(1. / 8.0)); // jitter within (64/8)x(64/8) neighborhood of 64x64 tex, good compromise between blotches and noise
   m_renderDevice->m_FBShader->SetVector(SHADER_AO_scale_timeblur, m_table->m_AOScale, 0.4f, 0.f,
      0.f); //!! 0.4f: fake global option in video pref? or time dependent? //!! commonly used is 0.1, but would require to clear history for moving stuff
   m_renderDevice->m_FBShader->SetTechnique(SHADER_TECHNIQUE_AO);
   m_renderDevice->DrawFullscreenTexturedQuad(m_renderDevice->m_FBShader);

   // flip AO buffers (avoids copy)
   SwapAORenderTargets();

   if (g_pplayer->GetProfilingMode() == PF_ENABLED)
      m_gpu_profiler.Timestamp(GTS_AO);
}

bool Renderer::IsBloomEnabled() const
{
   return !m_bloomOff && (m_table->m_bloom_strength > 0.0f) && (g_pplayer->GetInfoMode() <= IF_DYNAMIC_ONLY);
}

void Renderer::UpdateBloom(RenderTarget* renderedRT)
{
   if (!IsBloomEnabled())
      return;

   m_renderDevice->ResetRenderState();
   m_renderDevice->SetRenderState(RenderState::ALPHABLENDENABLE, RenderState::RS_FALSE);
   m_renderDevice->SetRenderState(RenderState::CULLMODE, RenderState::CULL_NONE);
   m_renderDevice->SetRenderState(RenderState::ZWRITEENABLE, RenderState::RS_FALSE);
   m_renderDevice->SetRenderState(RenderState::ZENABLE, RenderState::RS_FALSE);

   const double w = static_cast<double>(renderedRT->GetWidth());
   const double h = static_cast<double>(renderedRT->GetHeight());
   const Vertex3D_TexelOnly shiftedVerts[4] =
   {
      {  1.0f,  1.0f, 0.0f, 1.0f + (float)(2.25 / w), 0.0f + (float)(2.25 / h) },
      { -1.0f,  1.0f, 0.0f, 0.0f + (float)(2.25 / w), 0.0f + (float)(2.25 / h) },
      {  1.0f, -1.0f, 0.0f, 1.0f + (float)(2.25 / w), 1.0f + (float)(2.25 / h) },
      { -1.0f, -1.0f, 0.0f, 0.0f + (float)(2.25 / w), 1.0f + (float)(2.25 / h) }
   };
   {
      m_renderDevice->m_FBShader->SetTextureNull(SHADER_tex_fb_filtered);

      // switch to 'bloom' output buffer to collect clipped framebuffer values
      m_renderDevice->SetRenderTarget("Bloom Cut Off"s, GetBloomBufferTexture(), false);
      m_renderDevice->AddRenderTargetDependency(renderedRT);

      m_renderDevice->m_FBShader->SetTexture(SHADER_tex_fb_filtered, renderedRT->GetColorSampler());
      m_renderDevice->m_FBShader->SetVector(SHADER_w_h_height, (float) (1.0 / w), (float) (1.0 / h), m_table->m_bloom_strength, 1.0f);
      m_renderDevice->m_FBShader->SetTechnique(SHADER_TECHNIQUE_fb_bloom);

      m_renderDevice->DrawTexturedQuad(m_renderDevice->m_FBShader, shiftedVerts);
   }

   m_renderDevice->DrawGaussianBlur(
      GetBloomBufferTexture(), 
      GetBloomTmpBufferTexture(),
      GetBloomBufferTexture(), 39.f); // FIXME kernel size should depend on buffer resolution

   if (g_pplayer->GetProfilingMode() == PF_ENABLED)
      m_gpu_profiler.Timestamp(GTS_Bloom);
}

RenderTarget* Renderer::ApplyAdditiveScreenSpaceReflection(RenderTarget* renderedRT)
{
   if (!m_ss_refl || !m_table->m_enableSSR || !m_renderDevice->DepthBufferReadBackAvailable() || m_table->m_SSRScale <= 0.f)
      return renderedRT;

   m_renderDevice->ResetRenderState();
   m_renderDevice->SetRenderState(RenderState::ALPHABLENDENABLE, RenderState::RS_FALSE);
   m_renderDevice->SetRenderState(RenderState::CULLMODE, RenderState::CULL_NONE);
   m_renderDevice->SetRenderState(RenderState::ZWRITEENABLE, RenderState::RS_FALSE);
   m_renderDevice->SetRenderState(RenderState::ZENABLE, RenderState::RS_FALSE);

   RenderTarget* outputRT = GetReflectionBufferTexture();
   m_renderDevice->SetRenderTarget("ScreenSpace Reflection"s, outputRT, true);
   m_renderDevice->AddRenderTargetDependency(GetBackBufferTexture(), true);
   m_renderDevice->m_FBShader->SetTexture(SHADER_tex_depth, GetBackBufferTexture()->GetDepthSampler());
   m_renderDevice->AddRenderTargetDependency(renderedRT);
   m_renderDevice->m_FBShader->SetTexture(SHADER_tex_fb_filtered, renderedRT->GetColorSampler());
   m_renderDevice->m_FBShader->SetTexture(SHADER_tex_fb_unfiltered, renderedRT->GetColorSampler());
   m_renderDevice->m_FBShader->SetTexture(SHADER_tex_ao_dither, m_aoDitherSampler);
   // FIXME check if size should not be taken from renderdevice to account for stereo (double width/height) or supersampling
   m_renderDevice->m_FBShader->SetVector(SHADER_w_h_height, (float)(1.0 / renderedRT->GetWidth()), (float)(1.0 / renderedRT->GetHeight()), 1.0f /*radical_inverse(m_overall_frames%2048)*/, 1.0f);
   const float rotation = m_table->GetViewSetup().GetRotation(m_stereo3D, m_renderDevice->GetOutputBackBuffer()->GetWidth(), m_renderDevice->GetOutputBackBuffer()->GetHeight());
   m_renderDevice->m_FBShader->SetVector(SHADER_SSR_bumpHeight_fresnelRefl_scale_FS, 0.3f, 0.3f, m_table->m_SSRScale, rotation);
   m_renderDevice->m_FBShader->SetTechnique(SHADER_TECHNIQUE_SSReflection);
   m_renderDevice->DrawFullscreenTexturedQuad(m_renderDevice->m_FBShader);
   if (g_pplayer->GetProfilingMode() == PF_ENABLED)
      m_gpu_profiler.Timestamp(GTS_SSR);
   return outputRT;
}


// org HDR -> display HDR spline-based remapping
// inspired by the videolan spline tonemapper

#define PL_HDR_PQ false
#define PL_HDR_NITS true

// converts from PQ to nits and vice versa
static float pl_hdr_rescale(const bool from, const bool to, float x)
{
   if (from == to)
      return x;

   x = max(x, 0.0f);
   if (x == 0.0f)
      return 0.0f;

   // Convert input to PL_SCALE_RELATIVE
   if (from == PL_HDR_PQ)
   {
      x = powf(x, (float)(1.0 / ((2523. / 4096.) * 128.)));
      x = max(x - (float)(3424. / 4096.), 0.f) / ((float)((2413. / 4096.) * 32.) - (float)((2392. / 4096.) * 32.) * x);
      x = powf(x, (float)(1.0 / ((2610. / 4096.) / 4.)));
      x *= (float)(10000. / 203.);
   }
   else // PL_HDR_NITS
      x *= (float)(1. / 203.);

   // Convert PL_SCALE_RELATIVE to output
   if (to == PL_HDR_NITS)
      return x * 203.f;
   else // PL_HDR_PQ
   {
      x *= (float)(203. / 10000.);
      x = powf(x, (float)((2610. / 4096.) / 4.));
      x = ((float)(3424. / 4096.) + (float)((2413. / 4096.) * 32.) * x) / (1.0f + (float)((2392. / 4096.) * 32.) * x);
      return powf(x, (float)((2523. / 4096.) * 128.));
   }
}

// precompute spline parameters based on all the constants in here and the HDR display range (in nits)
void PrecompSplineTonemap(const float displayMaxLum, float out[6])
{
   const float src_hdr_max = max(1000.f, displayMaxLum); // assumes 1000 nits as scene input max, but adapts to displays with higher nits, as then everything stays perfectly linear //!! use 10000. for max input?

   constexpr float slope_tuning    = 1.5f; //[0..10]
   constexpr float slope_offset    = 0.2f; //[0..1]
   constexpr float spline_contrast = 0.5f; //[0..1.5]

#if 1
   constexpr float knee_minimum    = 0.1f; //(0..0.5)
   constexpr float knee_maximum    = 0.8f; //(0.5..1.0)
   constexpr float knee_default    = 0.4f; //inbetween min..max
   constexpr float knee_adaptation = 0.4f; //[0..1]

   // Pick a spline knee point
   // Could be based on HDR10+ brightness metadata and/or scene brightness average
   // We pick it based on a heuristic that assumes a scene input max of src_hdr_max
   // Inspired by SMPTE ST2094-10, with some modifications, see videolan/st2094_pick_knee()

   const float src_max = pl_hdr_rescale(PL_HDR_NITS, PL_HDR_PQ, src_hdr_max);
   //const float src_avg = pl_hdr_rescale(PL_HDR_NITS, PL_HDR_PQ, 10.f); //!! 10 nits if real scene input average not available?! claimed to be "industry standard"
   const float dst_max = pl_hdr_rescale(PL_HDR_NITS, PL_HDR_PQ, displayMaxLum);

   const float src_knee_min = src_max*knee_minimum;
   const float src_knee_max = src_max*knee_maximum;
   const float dst_knee_min = dst_max*knee_minimum;
   const float dst_knee_max = dst_max*knee_maximum;

   float src_knee = /*src_avg!=0. ? src_avg :*/ src_max*knee_default; //!! see above. What is a better default? Most likely this here, given that pinball machines are rather bright?!
   src_knee = clamp(src_knee, src_knee_min, src_knee_max);

   // Choose target adaptation point based on linearly re-scaling the source knee
   const float target = src_knee / src_max;
   const float adapted = dst_max*target;

   // Choose the destination knee by picking the perceptual adaptation point
   // between the source knee and the desired target. This moves the knee
   // point, on the vertical axis, closer to the 1:1 (neutral) line.
   //
   // Adjust the adaptation strength towards 1 based on how close the knee
   // point is to its extreme values (min/max knee)
   const float tuning = 1.0f - smoothstep(knee_maximum, knee_default, target) * smoothstep(knee_minimum, knee_default, target); // the first smoothstep assumes that it works for a > b! This results overall in a bathtub like shape
   const float adaptation = lerp(knee_adaptation, 1.0f, tuning);
   float dst_knee = lerp(src_knee, adapted, adaptation);
   dst_knee = clamp(dst_knee, dst_knee_min, dst_knee_max);

   const float src_pivot = pl_hdr_rescale(PL_HDR_PQ, PL_HDR_NITS, src_knee);
   const float dst_pivot = pl_hdr_rescale(PL_HDR_PQ, PL_HDR_NITS, dst_knee);

#else

   //!! old videolan version, may not match the newer code and new constants, but is much simpler to understand!
   const float sdr_avg = (float)(203.0 / sqrt(1000.0)); // typical contrast

   // Infer the average from scene metadata if present, or default to 10 nits as
   // this is an industry-standard value that produces good results on average
   //
   // Infer the destination from the desired output characteristics, clamping
   // the lower bound to the characteristics of a typical SDR signal
   const float src_avg = clamp(10.0f, 0.f, src_hdr_max);
   const float dst_avg = clamp(sdr_avg, 0.f, displayMaxLum);

   const float src_pivot = min(src_avg, 0.8f * src_hdr_max);
   const float dst_pivot = min(sqrtf(src_avg * dst_avg), 0.8f * displayMaxLum);
#endif

   // Solve for linear knee (Pa = 0)
   float slope = dst_pivot / src_pivot;

   // Tune the slope at the knee point slightly: raise it to a user-provided
   // gamma exponent, multiplied by an extra tuning coefficient designed to
   // make the slope closer to 1.0 when the difference in peaks is low, and
   // closer to linear when the difference between peaks is high.
   float ratio = src_hdr_max / displayMaxLum - 1.0f;
   ratio = clamp(slope_tuning * ratio, slope_offset, 1.0f + slope_offset);
   slope = powf(slope, (1.0f - spline_contrast) * ratio);

   // Normalize everything to make the math easier
   const float in_max = src_hdr_max - src_pivot;
   const float out_max = displayMaxLum - dst_pivot;

   // Solve P of order 2 for:
   //  P(-src_pivot) = -dst_pivot
   //  P'(0.0) = slope
   //  P(0.0) = 0.0
   const float Pa = (slope * src_pivot - dst_pivot) / (src_pivot * src_pivot);

   // Solve Q of order 3 for:
   //  Q(in_max) = out_max
   //  Q''(in_max) = 0.0
   //  Q(0.0) = 0.0
   //  Q'(0.0) = slope
   const float t = 2.f * in_max * in_max;
   const float Qa = (slope * in_max - out_max) / (in_max * t);
   const float Qb = -3.f * (slope * in_max - out_max) / t;

   // This here must match the order in the shader itself!
   out[0] = src_pivot;
   out[1] = dst_pivot;
   out[2] = slope;
   out[3] = Pa;
   out[4] = Qa;
   out[5] = Qb;
}

ShaderTechniques Renderer::ApplyTonemapping(RenderTarget* renderedRT, RenderTarget* tonemapRT)
{
   //const unsigned int jittertime = (unsigned int)((uint64_t)msec()*90/1000);
   //const float jitter = (float)((msec() & 2047) / 1000.0);
   const float jitter = (float)(radical_inverse(g_pplayer->m_overall_frames % 2048) / 1000.0); // Deterministic jitter to ensure stable render for regression tests
   const bool useAO = GetAOMode() == 2;

   // switch to output buffer (main output frame buffer, or a temporary one for postprocessing)
   RenderTarget* outputRT = tonemapRT;
   assert(outputRT != renderedRT);
   m_renderDevice->SetRenderTarget("Tonemap/Dither/ColorGrade"s, outputRT, false);

   m_renderDevice->ResetRenderState();
   m_renderDevice->SetRenderState(RenderState::ALPHABLENDENABLE, RenderState::RS_FALSE);
   m_renderDevice->SetRenderState(RenderState::CULLMODE, RenderState::CULL_NONE);
   m_renderDevice->SetRenderState(RenderState::ZWRITEENABLE, RenderState::RS_FALSE);
   m_renderDevice->SetRenderState(RenderState::ZENABLE, RenderState::RS_FALSE);

   int render_w = renderedRT->GetWidth(), render_h = renderedRT->GetHeight();
   m_renderDevice->AddRenderTargetDependency(renderedRT);
   m_renderDevice->m_FBShader->SetTexture(SHADER_tex_fb_unfiltered, renderedRT->GetColorSampler());
   m_renderDevice->m_FBShader->SetTexture(SHADER_tex_fb_filtered, renderedRT->GetColorSampler());

   if (m_table->m_bloom_strength > 0.0f && !m_bloomOff)
   {
      m_renderDevice->AddRenderTargetDependency(GetBloomBufferTexture());
      m_renderDevice->m_FBShader->SetTexture(SHADER_tex_bloom, GetBloomBufferTexture()->GetColorSampler());
   }

   if (useAO)
   {
      m_renderDevice->m_FBShader->SetTexture(SHADER_tex_ao, GetAORenderTarget(1)->GetColorSampler());
      m_renderDevice->AddRenderTargetDependency(GetAORenderTarget(1));
   }

   // For information mode, override with the wanted render target instead of the render buffer
   const InfoMode infoMode = g_pplayer->GetInfoMode();
   if (infoMode == IF_RENDER_PROBES)
   {
      RenderProbe* render_probe = m_table->m_vrenderprobe[g_pplayer->m_infoProbeIndex];
      RenderTarget *probe = render_probe->Render(0);
      if (probe)
      {
         m_renderDevice->AddRenderTargetDependency(probe);
         m_renderDevice->m_FBShader->SetTexture(SHADER_tex_fb_unfiltered, probe->GetColorSampler());
         m_renderDevice->m_FBShader->SetTexture(SHADER_tex_fb_filtered, probe->GetColorSampler());
         render_w = probe->GetWidth();
         render_h = probe->GetHeight();
      }
   }
   else if (infoMode == IF_LIGHT_BUFFER_ONLY)
   {
      renderedRT = GetBloomBufferTexture();
      m_renderDevice->AddRenderTargetDependency(renderedRT);
      m_renderDevice->m_FBShader->SetTexture(SHADER_tex_fb_unfiltered, renderedRT->GetColorSampler());
      m_renderDevice->m_FBShader->SetTexture(SHADER_tex_fb_filtered, renderedRT->GetColorSampler());
      render_w = renderedRT->GetWidth();
      render_h = renderedRT->GetHeight();
   }

   const bool isHdr2020 = (g_pplayer->m_vrDevice == nullptr) && m_renderDevice->m_outputWnd[0]->IsWCGBackBuffer();
   if (isHdr2020)
   {
      const float maxDisplayLuminance = m_renderDevice->m_outputWnd[0]->GetHDRHeadRoom() * (m_renderDevice->m_outputWnd[0]->GetSDRWhitePoint() * 80.f); // Maximum luminance of display in nits, note that GetSDRWhitePoint()*80 should usually be in the 200 nits range
      m_renderDevice->m_FBShader->SetVector(SHADER_exposure_wcg,
         m_exposure,
         (m_renderDevice->m_outputWnd[0]->GetSDRWhitePoint() * 80.f) / maxDisplayLuminance, // Apply SDR whitepoint (1.0 -> white point in nits), then scale down by maximum luminance (in nits) of display to get a relative value before tonemapping, equal to 1/GetHDRHeadRoom()
         maxDisplayLuminance / 10000.f, // Apply back maximum luminance in nits of display after tonemapping, scaled down to PQ limits (1.0 is 10000 nits)
         1.f);

      float spline_params[6];
      PrecompSplineTonemap(maxDisplayLuminance, spline_params);
      m_renderDevice->m_FBShader->SetVector(SHADER_spline1,
         spline_params[0],spline_params[1],spline_params[2],spline_params[3]);
      m_renderDevice->m_FBShader->SetVector(SHADER_spline2,
         spline_params[4],spline_params[5], 0.f,0.f);
   }
   else
   {
      #ifdef ENABLE_VR
         // Legacy OpenVR has hacked colorspace conversion
         m_renderDevice->m_FBShader->SetVector(SHADER_exposure_wcg, m_exposure, 1.f, 1.f, 0.f);
      #else
         // VR device expects linear RGB value (for linear layer composition)
         m_renderDevice->m_FBShader->SetVector(SHADER_exposure_wcg, m_exposure, 1.f, /*100.f*//*203.f*/350.f/10000.f, g_pplayer->m_vrDevice ? 2.f : 0.f); //!! 203 nits as SDR reference? //!! or 100 as in BT2446 spec? // but both result in too dark images for BT2446 conversion at least compared to the other mappers
      #endif

      // dummy values only, unused at the moment
      //m_renderDevice->m_FBShader->SetVector(SHADER_spline1, 0.f,0.f,0.f,0.f);
      //m_renderDevice->m_FBShader->SetVector(SHADER_spline2, 0.f,0.f,0.f,0.f);
   }

   Texture *const pin = m_table->GetImage(m_table->m_imageColorGrade);
   if (pin)
      // FIXME ensure that we always honor the linear RGB. Here it can be defeated if texture is used for something else (which is very unlikely)
      m_renderDevice->m_FBShader->SetTexture(SHADER_tex_color_lut, pin, true, SF_BILINEAR, SA_CLAMP, SA_CLAMP);
   m_renderDevice->m_FBShader->SetVector(SHADER_bloom_dither_colorgrade,
      IsBloomEnabled() ? 1.f : 0.f, // Bloom
      (!isHdr2020 && (m_renderDevice->GetOutputBackBuffer()->GetColorFormat() != colorFormat::RGBA10)) ? 1.f : 0.f, // Dither
      (pin != nullptr) ? 1.f : 0.f, /* LUT colorgrade */
      0.f);
   if (IsBloomEnabled())
      m_renderDevice->AddRenderTargetDependency(GetBloomBufferTexture());
   m_renderDevice->m_FBShader->SetVector(SHADER_w_h_height,
      (float)(1.0 / (double)render_w), (float)(1.0 / (double)render_h),
      jitter, // radical_inverse(jittertime) * 11.0f,
      jitter); // sobol(jittertime) * 13.0f); // jitter for dither pattern

   ShaderTechniques tonemapTechnique;
   const bool useAA = m_renderWidth > GetBackBufferTexture()->GetWidth();
   if (infoMode == IF_AO_ONLY)
      tonemapTechnique = SHADER_TECHNIQUE_fb_AO;
   else if (infoMode == IF_RENDER_PROBES)
      tonemapTechnique = m_toneMapper == TM_REINHARD     ? SHADER_TECHNIQUE_fb_rhtonemap
                       : m_toneMapper == TM_FILMIC       ? SHADER_TECHNIQUE_fb_fmtonemap
                       : m_toneMapper == TM_NEUTRAL      ? SHADER_TECHNIQUE_fb_nttonemap
                       : m_toneMapper == TM_AGX          ? SHADER_TECHNIQUE_fb_agxtonemap
                       : m_toneMapper == TM_AGX_PUNCHY   ? SHADER_TECHNIQUE_fb_agxptonemap
                       : /*m_toneMapper == TM_WCG_SPLINE ?*/ SHADER_TECHNIQUE_fb_wcgtonemap;
   else if (m_renderDevice->m_outputWnd[0]->IsWCGBackBuffer() && m_HDRforceDisableToneMapper)
      tonemapTechnique = useAO ? useAA ? SHADER_TECHNIQUE_fb_wcgtonemap_AO : SHADER_TECHNIQUE_fb_wcgtonemap_AO_no_filter
                               : useAA ? SHADER_TECHNIQUE_fb_wcgtonemap    : SHADER_TECHNIQUE_fb_wcgtonemap_no_filter;
   else if (m_toneMapper == TM_REINHARD)
      tonemapTechnique = useAO ? useAA ? SHADER_TECHNIQUE_fb_rhtonemap_AO : SHADER_TECHNIQUE_fb_rhtonemap_AO_no_filter
                               : useAA ? SHADER_TECHNIQUE_fb_rhtonemap    : SHADER_TECHNIQUE_fb_rhtonemap_no_filter;
   else if (m_toneMapper == TM_FILMIC)
      tonemapTechnique = useAO ? useAA ? SHADER_TECHNIQUE_fb_fmtonemap_AO : SHADER_TECHNIQUE_fb_fmtonemap_AO_no_filter
                               : useAA ? SHADER_TECHNIQUE_fb_fmtonemap    : SHADER_TECHNIQUE_fb_fmtonemap_no_filter;
   else if (m_toneMapper == TM_NEUTRAL)
      tonemapTechnique = useAO ? useAA ? SHADER_TECHNIQUE_fb_nttonemap_AO : SHADER_TECHNIQUE_fb_nttonemap_AO_no_filter
                               : useAA ? SHADER_TECHNIQUE_fb_nttonemap    : SHADER_TECHNIQUE_fb_nttonemap_no_filter;
   else if (m_toneMapper == TM_AGX)
      tonemapTechnique = useAO ? useAA ? SHADER_TECHNIQUE_fb_agxtonemap_AO : SHADER_TECHNIQUE_fb_agxtonemap_AO_no_filter
                               : useAA ? SHADER_TECHNIQUE_fb_agxtonemap    : SHADER_TECHNIQUE_fb_agxtonemap_no_filter;
   else if (m_toneMapper == TM_AGX_PUNCHY)
      tonemapTechnique = useAO ? useAA ? SHADER_TECHNIQUE_fb_agxptonemap_AO : SHADER_TECHNIQUE_fb_agxptonemap_AO_no_filter
                               : useAA ? SHADER_TECHNIQUE_fb_agxptonemap    : SHADER_TECHNIQUE_fb_agxptonemap_no_filter;
   else
      assert(!"unknown tonemapper");

   const Vertex3D_TexelOnly shiftedVerts[4] =
   {
      {  1.0f + m_ScreenOffset.x,  1.0f + m_ScreenOffset.y, 0.0f, 1.0f, 0.0f },
      { -1.0f + m_ScreenOffset.x,  1.0f + m_ScreenOffset.y, 0.0f, 0.0f, 0.0f },
      {  1.0f + m_ScreenOffset.x, -1.0f + m_ScreenOffset.y, 0.0f, 1.0f, 1.0f },
      { -1.0f + m_ScreenOffset.x, -1.0f + m_ScreenOffset.y, 0.0f, 0.0f, 1.0f }
   };
   m_renderDevice->m_FBShader->SetTechnique(tonemapTechnique);
   m_renderDevice->DrawTexturedQuad(m_renderDevice->m_FBShader, shiftedVerts);

   return tonemapTechnique;
}

RenderTarget* Renderer::ApplyBallMotionBlur(RenderTarget* beforeTonemapRT, RenderTarget* afterTonemapRT, ShaderTechniques tonemapTechnique)
{
   #ifndef ENABLE_BGFX
   return afterTonemapRT;
   #endif

   if (m_motionBlurOff)
      return afterTonemapRT;

   m_renderDevice->ResetRenderState();
   m_renderDevice->SetRenderState(RenderState::ALPHABLENDENABLE, RenderState::RS_FALSE);
   m_renderDevice->SetRenderState(RenderState::CULLMODE, RenderState::CULL_NONE);
   m_renderDevice->SetRenderState(RenderState::ZWRITEENABLE, RenderState::RS_FALSE);
   m_renderDevice->SetRenderState(RenderState::ZENABLE, RenderState::RS_FALSE);

   // Draw motion blur to a temporary render target
   RenderTarget* tempRT = GetMotionBlurBufferTexture(); // Use a dedicated buffer since we need HDR (RGB16F) and we can't use the existing ones (Backbuffer 1 & 2 and SSR)
   m_renderDevice->SetRenderTarget("Ball Motion Blur - Compute"s, tempRT, false);
   m_renderDevice->AddRenderTargetDependency(GetBackBufferTexture(), true);
   m_renderDevice->m_FBShader->SetTexture(SHADER_tex_depth, GetBackBufferTexture()->GetDepthSampler());
   m_renderDevice->AddRenderTargetDependency(GetPreviousBackBufferTexture());
   m_renderDevice->m_FBShader->SetTexture(SHADER_tex_bloom, GetPreviousBackBufferTexture()->GetColorSampler());
   m_renderDevice->AddRenderTargetDependency(beforeTonemapRT);
   m_renderDevice->m_FBShader->SetTexture(SHADER_tex_fb_filtered, beforeTonemapRT->GetColorSampler());
   Matrix3D matProjInv[2], matProj[2];
   const int nEyes = m_renderDevice->m_nEyes;
   Matrix3D identity;
   identity.SetIdentity();
   GetMVP().SetModel(identity);
   for (int eye = 0; eye < nEyes; eye++)
   {
      matProj[eye] = GetMVP().GetProj(eye);
      matProjInv[eye] = matProj[eye];
      matProjInv[eye].Invert();
   }
   m_renderDevice->m_FBShader->SetMatrix(SHADER_matProjInv, &matProjInv[0], nEyes);
   m_renderDevice->m_FBShader->SetMatrix(SHADER_matProj, &matProj[0], nEyes);
   Vertex3D_TexelOnly quads[4 * 16];
   Vertex3D_TexelOnly* updatedVertices[16];
   int nQuads = 0;
   for (size_t i = 0; i < g_pplayer->m_vball.size() && nQuads < 16; i++)
   {
      HitBall* const pball = g_pplayer->m_vball[i];
      if (!pball->m_pBall->m_d.m_visible || pball->m_d.m_lockedInKicker)
         continue;

      // Discard stable balls or balls that have moved too much (which means the ball was likely created/moved)
      // We assume that velocity won't change before rendering (which is wrong) but extend it by a magic factor of 10
      const Matrix3D view = GetMVP().GetView();
      const vec3 posl = pball->m_d.m_pos + 0.5f * pball->m_d.m_vel;
      const vec3 newPos = view.MultiplyVectorNoPerspective(posl);
      const vec3 delta = newPos - pball->m_lastRenderedPos;
      const float deltaSquared = delta.Dot(delta);
      if (deltaSquared < 0.01f || deltaSquared > 1000.f)
      {
         pball->m_lastRenderedPos = newPos;
         continue;
      }

      // Compute a quad bound. This is fairly suboptimal and would benefit from a simple convex hull (at least from the 2 bounding rects)
      float xMin = FLT_MAX, xMax = -FLT_MAX, yMin = FLT_MAX, yMax = -FLT_MAX;
      for (int eye = 0; eye < nEyes; eye++)
         Ball::m_ash.computeProjBounds(
            GetMVP().GetProj(eye), pball->m_lastRenderedPos.x, pball->m_lastRenderedPos.y, pball->m_lastRenderedPos.z, pball->m_d.m_radius, xMin, xMax, yMin, yMax);
      const float prevLen = Vertex2D((xMax - xMin) * static_cast<float>(tempRT->GetWidth()), (yMax - yMin) * static_cast<float>(tempRT->GetHeight())).Length();
      for (int eye = 0; eye < nEyes; eye++)
         Ball::m_ash.computeProjBounds(GetMVP().GetProj(eye), newPos.x, newPos.y, newPos.z, pball->m_d.m_radius, xMin, xMax, yMin, yMax);
      const float fullLen = Vertex2D((xMax - xMin) * static_cast<float>(tempRT->GetWidth()), (yMax - yMin) * static_cast<float>(tempRT->GetHeight())).Length() - prevLen;
      const int nSamples = max(2, static_cast<int>(0.5f * fullLen));
      //xMin = yMin = -1.f; xMax = yMax = 1.f;

      const Vertex3D_TexelOnly verts[4] =
      {
         { xMax, yMax, 0.0f, xMax * 0.5f + 0.5f, 0.5f - yMax * 0.5f },
         { xMin, yMax, 0.0f, xMin * 0.5f + 0.5f, 0.5f - yMax * 0.5f },
         { xMax, yMin, 0.0f, xMax * 0.5f + 0.5f, 0.5f - yMin * 0.5f },
         { xMin, yMin, 0.0f, xMin * 0.5f + 0.5f, 0.5f - yMin * 0.5f }
      };
      memcpy(quads + nQuads * 4, verts, sizeof(verts));

      vec4* balls = new vec4[MAX_BALL_SHADOW];
      balls[1] = vec4(pball->m_lastRenderedPos.x, pball->m_lastRenderedPos.y, pball->m_lastRenderedPos.z, pball->m_d.m_radius);
      m_renderDevice->m_FBShader->SetVector(SHADER_w_h_height, static_cast<float>(1.0 / beforeTonemapRT->GetWidth()), static_cast<float>(1.0 / beforeTonemapRT->GetHeight()),
         0.f /* unused */ ,static_cast<float>(min(32, nSamples)));
      m_renderDevice->m_FBShader->SetTechnique(SHADER_TECHNIQUE_fb_motionblur);
      m_renderDevice->DrawTexturedQuad(m_renderDevice->m_FBShader, verts);

      // Update drawn rect bounds and ball position to account for late adjustment
      ShaderState* ss = m_renderDevice->GetCurrentPass()->m_commands.back()->GetShaderState();
      Vertex3D_TexelOnly* vertices = (Vertex3D_TexelOnly*)m_renderDevice->GetCurrentPass()->m_commands.back()->GetQuadVertices();
      updatedVertices[nQuads] = vertices;
      m_renderDevice->AddBeginOfFrameCmd(
         [this, pball, view, ss, vertices, balls]()
         {
            RenderTarget* tempRT = GetMotionBlurBufferTexture();
            const vec3 posl = pball->m_d.m_pos + m_renderDevice->GetPredictedDisplayDelayInS() * pball->m_d.m_vel;
            const vec3 newPos = view.MultiplyVectorNoPerspective(posl);
            const int nEyes = m_renderDevice->m_nEyes;

            float xMin = FLT_MAX, xMax = -FLT_MAX, yMin = FLT_MAX, yMax = -FLT_MAX;
            for (int eye = 0; eye < nEyes; eye++)
               Ball::m_ash.computeProjBounds(
                  GetMVP().GetProj(eye), pball->m_lastRenderedPos.x, pball->m_lastRenderedPos.y, pball->m_lastRenderedPos.z, pball->m_d.m_radius, xMin, xMax, yMin, yMax);
            const float prevLen = Vertex2D((xMax - xMin) * static_cast<float>(tempRT->GetWidth()), (yMax - yMin) * static_cast<float>(tempRT->GetHeight())).Length();
            for (int eye = 0; eye < nEyes; eye++)
               Ball::m_ash.computeProjBounds(GetMVP().GetProj(eye), newPos.x, newPos.y, newPos.z, pball->m_d.m_radius, xMin, xMax, yMin, yMax);

            const Vertex3D_TexelOnly verts[4] =
            {
               { xMax, yMax, 0.0f, xMax * 0.5f + 0.5f, 0.5f - yMax * 0.5f },
               { xMin, yMax, 0.0f, xMin * 0.5f + 0.5f, 0.5f - yMax * 0.5f },
               { xMax, yMin, 0.0f, xMax * 0.5f + 0.5f, 0.5f - yMin * 0.5f },
               { xMin, yMin, 0.0f, xMin * 0.5f + 0.5f, 0.5f - yMin * 0.5f }
            };
            memcpy(vertices, verts, sizeof(verts));

            pball->m_lastRenderedPos = newPos;
            balls[0] = vec4(newPos.x, newPos.y, newPos.z, pball->m_d.m_radius);
            ss->SetVector(SHADER_balls, balls, MAX_BALL_SHADOW);
            delete[] balls;
         });

      nQuads++;
   }

   // Then copy back from temporary buffer, applying tonemap since destination buffer is the tonemapped one
   if (nQuads)
   {
      m_renderDevice->SetRenderTarget("Ball Motion Blur - Copy"s, afterTonemapRT, true);
      m_renderDevice->AddRenderTargetDependency(tempRT);
      m_renderDevice->m_FBShader->SetTexture(SHADER_tex_fb_unfiltered, tempRT->GetColorSampler());
      m_renderDevice->m_FBShader->SetTexture(SHADER_tex_fb_filtered, tempRT->GetColorSampler());
      if (IsBloomEnabled())
      {
         m_renderDevice->AddRenderTargetDependency(GetBloomBufferTexture());
         m_renderDevice->m_FBShader->SetTexture(SHADER_tex_bloom, GetBloomBufferTexture()->GetColorSampler());
      }
      if (GetAOMode() == 2) // Dynamic AO ?
      {
         m_renderDevice->m_FBShader->SetTexture(SHADER_tex_ao, GetAORenderTarget(1)->GetColorSampler());
         m_renderDevice->AddRenderTargetDependency(GetAORenderTarget(1));
      }
      const float jitter = (float)(radical_inverse(g_pplayer->m_overall_frames % 2048) / 1000.0); // Deterministic jitter to ensure stable render for regression tests
      m_renderDevice->m_FBShader->SetVector(SHADER_w_h_height, 
         static_cast<float>(1.0 / tempRT->GetWidth()), static_cast<float>(1.0 / tempRT->GetHeight()), jitter, jitter);
      m_renderDevice->m_FBShader->SetTechnique(tonemapTechnique);
      for (int i = 0; i < nQuads * 4; i++)
      {
         quads[i].x += m_ScreenOffset.x;
         quads[i].y += m_ScreenOffset.y;
      }
      for (int i = 0; i < nQuads; i++)
      {
         m_renderDevice->DrawTexturedQuad(m_renderDevice->m_FBShader, quads + i * 4);
         // Update drawn rect bounds and ball position to account for late adjustment
         Vertex3D_TexelOnly* vertices = (Vertex3D_TexelOnly*)m_renderDevice->GetCurrentPass()->m_commands.back()->GetQuadVertices();
         m_renderDevice->AddBeginOfFrameCmd([vertices, newVerts = updatedVertices[i]]() { memcpy(vertices, newVerts, 4 * sizeof(Vertex3D_TexelOnly)); });
      }
   }

   return afterTonemapRT;
}

RenderTarget* Renderer::ApplyPostProcessedAntialiasing(RenderTarget* renderedRT, RenderTarget* outputBackBuffer)
{
   const bool SMAA = m_FXAA == Quality_SMAA;
   const bool DLAA = m_FXAA == Standard_DLAA;
   const bool NFAA = m_FXAA == Fast_NFAA;
   const bool FXAA1 = m_FXAA == Fast_FXAA;
   const bool FXAA2 = m_FXAA == Standard_FXAA;
   const bool FXAA3 = m_FXAA == Quality_FXAA;
   const bool FAAA = m_FXAA == Quality_FAAA;

   m_renderDevice->ResetRenderState();
   m_renderDevice->SetRenderState(RenderState::ALPHABLENDENABLE, RenderState::RS_FALSE);
   m_renderDevice->SetRenderState(RenderState::CULLMODE, RenderState::CULL_NONE);
   m_renderDevice->SetRenderState(RenderState::ZWRITEENABLE, RenderState::RS_FALSE);
   m_renderDevice->SetRenderState(RenderState::ZENABLE, RenderState::RS_FALSE);

   if (NFAA || FXAA1 || FXAA2 || FXAA3 || FAAA)
   {
      assert(renderedRT == GetPostProcessRenderTarget1());
      RenderTarget* outputRT = outputBackBuffer ? outputBackBuffer : GetPostProcessRenderTarget(renderedRT);
      assert(outputRT != renderedRT);
      m_renderDevice->SetRenderTarget("Post Process AA Pass"s, outputRT, false);
      m_renderDevice->AddRenderTargetDependency(renderedRT);
      m_renderDevice->m_FBShader->SetTexture(SHADER_tex_fb_filtered, renderedRT->GetColorSampler());
      m_renderDevice->m_FBShader->SetTexture(SHADER_tex_fb_unfiltered, renderedRT->GetColorSampler());
      m_renderDevice->AddRenderTargetDependency(GetBackBufferTexture(), true); // Depth is always taken from the MSAA resolved render buffer
      m_renderDevice->m_FBShader->SetVector(SHADER_w_h_height, (float)(1.0 / renderedRT->GetWidth()), (float)(1.0 / renderedRT->GetHeight()), (float)renderedRT->GetWidth(), 1.f);
      m_renderDevice->m_FBShader->SetTechnique(NFAA ? SHADER_TECHNIQUE_NFAA : FXAA3 ? SHADER_TECHNIQUE_FXAA3 : FXAA2 ? SHADER_TECHNIQUE_FXAA2 : FXAA1 ? SHADER_TECHNIQUE_FXAA1 : SHADER_TECHNIQUE_FAAA);
      m_renderDevice->DrawFullscreenTexturedQuad(m_renderDevice->m_FBShader);
      return outputRT;
   }
   else if (DLAA)
   {
      assert(renderedRT == GetPostProcessRenderTarget1());
      // First pass detect edges and write it to alpha channel (keeping RGB)
      RenderTarget* outputRT = GetPostProcessRenderTarget(renderedRT);
      assert(outputRT != renderedRT);
      m_renderDevice->SetRenderTarget("DLAA Edge Detection"s, outputRT, false);
      m_renderDevice->AddRenderTargetDependency(renderedRT);
      m_renderDevice->m_FBShader->SetTexture(SHADER_tex_fb_filtered, renderedRT->GetColorSampler());
      m_renderDevice->m_FBShader->SetTexture(SHADER_tex_fb_unfiltered, renderedRT->GetColorSampler());
      m_renderDevice->AddRenderTargetDependency(GetBackBufferTexture(), true); // Depth is always taken from the MSAA resolved render buffer
      m_renderDevice->m_FBShader->SetVector(SHADER_w_h_height, (float)(1.0 / renderedRT->GetWidth()), (float)(1.0 / renderedRT->GetHeight()), (float)renderedRT->GetWidth(), 1.f);
      m_renderDevice->m_FBShader->SetTechnique(SHADER_TECHNIQUE_DLAA_edge);
      m_renderDevice->DrawFullscreenTexturedQuad(m_renderDevice->m_FBShader);
      renderedRT = outputRT;

      // Second pass: use edge detection from first pass (alpha channel) and RGB colors for actual filtering
      outputRT = outputBackBuffer ? outputBackBuffer : GetPostProcessRenderTarget(renderedRT);
      assert(outputRT != renderedRT);
      m_renderDevice->SetRenderTarget("DLAA Neigborhood blending"s, outputRT, false);
      m_renderDevice->AddRenderTargetDependency(renderedRT);
      m_renderDevice->m_FBShader->SetTexture(SHADER_tex_fb_filtered, renderedRT->GetColorSampler());
      m_renderDevice->m_FBShader->SetTexture(SHADER_tex_fb_unfiltered, renderedRT->GetColorSampler());
      m_renderDevice->m_FBShader->SetTechnique(SHADER_TECHNIQUE_DLAA);
      m_renderDevice->m_FBShader->SetVector(SHADER_w_h_height, (float)(1.0 / renderedRT->GetWidth()), (float)(1.0 / renderedRT->GetHeight()), (float)renderedRT->GetWidth(), 1.f);
      m_renderDevice->DrawFullscreenTexturedQuad(m_renderDevice->m_FBShader);
      return outputRT;
   }
#ifndef __OPENGLES__
   else if (SMAA)
   {
      assert(renderedRT == GetPostProcessRenderTarget1());
      // SMAA use 3 passes, all of them using the initial render, so since tonemap use postprocess RT 1, we use the back buffer and post process RT 2
      RenderTarget* sourceRT = renderedRT;
      m_renderDevice->m_FBShader->SetTexture(SHADER_tex_fb_filtered, sourceRT->GetColorSampler());
      m_renderDevice->m_FBShader->SetTexture(SHADER_tex_fb_unfiltered, sourceRT->GetColorSampler());
      m_renderDevice->m_FBShader->SetVector(
         SHADER_w_h_height, (float)(1.0 / sourceRT->GetWidth()), (float)(1.0 / sourceRT->GetHeight()), (float)sourceRT->GetWidth(), (float)sourceRT->GetHeight());

      RenderTarget* outputRT = GetPreviousBackBufferTexture(); // We don't need it anymore, so use it as a third postprocess buffer
      assert(outputRT != renderedRT);
      m_renderDevice->SetRenderTarget("SMAA Color/Edge Detection"s, outputRT, false);
      m_renderDevice->AddRenderTargetDependency(sourceRT); // PostProcess RT 1
      m_renderDevice->Clear(clearType::TARGET, 0x00000000); // Needed since shader uses discard
      m_renderDevice->m_FBShader->SetTechnique(SHADER_TECHNIQUE_SMAA_ColorEdgeDetection);
      m_renderDevice->DrawFullscreenTexturedQuad(m_renderDevice->m_FBShader);
      renderedRT = outputRT;

      outputRT = GetPostProcessRenderTarget(sourceRT);
      assert(outputRT != renderedRT);
      m_renderDevice->SetRenderTarget("SMAA Blend weight calculation"s, outputRT, false);
      m_renderDevice->AddRenderTargetDependency(sourceRT); // PostProcess RT 1
      m_renderDevice->AddRenderTargetDependency(renderedRT); // BackBuffer RT
      m_renderDevice->m_FBShader->SetTechnique(SHADER_TECHNIQUE_SMAA_BlendWeightCalculation);
      m_renderDevice->m_FBShader->SetTexture(SHADER_edgesTex, renderedRT->GetColorSampler());
      m_renderDevice->DrawFullscreenTexturedQuad(m_renderDevice->m_FBShader);
      renderedRT = outputRT;

      outputRT = outputBackBuffer ? outputBackBuffer : GetPreviousBackBufferTexture();
      assert(outputRT != renderedRT);
      m_renderDevice->SetRenderTarget("SMAA Neigborhood blending"s, outputRT, false);
      m_renderDevice->AddRenderTargetDependency(sourceRT); // PostProcess RT 1
      m_renderDevice->AddRenderTargetDependency(renderedRT); // PostProcess RT 2
      m_renderDevice->m_FBShader->SetTechnique(SHADER_TECHNIQUE_SMAA_NeighborhoodBlending);
      m_renderDevice->m_FBShader->SetTexture(SHADER_blendTex, renderedRT->GetColorSampler());
      m_renderDevice->DrawFullscreenTexturedQuad(m_renderDevice->m_FBShader);
      return outputRT;
   }
#endif

   return renderedRT;
}

RenderTarget* Renderer::ApplySharpening(RenderTarget* renderedRT, RenderTarget* outputBackBuffer)
{
   if (!m_sharpen)
      return renderedRT;

   m_renderDevice->ResetRenderState();
   m_renderDevice->SetRenderState(RenderState::ALPHABLENDENABLE, RenderState::RS_FALSE);
   m_renderDevice->SetRenderState(RenderState::CULLMODE, RenderState::CULL_NONE);
   m_renderDevice->SetRenderState(RenderState::ZWRITEENABLE, RenderState::RS_FALSE);
   m_renderDevice->SetRenderState(RenderState::ZENABLE, RenderState::RS_FALSE);

   assert(renderedRT != outputBackBuffer); // At this point, renderedRT may be PP1, PP2 or backbuffer
   RenderTarget* outputRT = outputBackBuffer ? outputBackBuffer : GetPostProcessRenderTarget(renderedRT);
   assert(outputRT != renderedRT);
   m_renderDevice->SetRenderTarget("Sharpen"s, outputRT, false);
   m_renderDevice->AddRenderTargetDependency(renderedRT);
   m_renderDevice->AddRenderTargetDependency(GetBackBufferTexture(), true); // Depth is always taken from the MSAA resolved render buffer
   m_renderDevice->m_FBShader->SetTexture(SHADER_tex_fb_filtered, renderedRT->GetColorSampler());
   m_renderDevice->m_FBShader->SetTexture(SHADER_tex_fb_unfiltered, renderedRT->GetColorSampler());
   m_renderDevice->m_FBShader->SetVector(SHADER_w_h_height, (float)(1.0 / renderedRT->GetWidth()), (float)(1.0 / renderedRT->GetHeight()), (float)renderedRT->GetWidth(), 1.f);
   m_renderDevice->m_FBShader->SetTechnique((m_sharpen == 1) ? SHADER_TECHNIQUE_CAS : SHADER_TECHNIQUE_BilateralSharp_CAS);
   m_renderDevice->DrawFullscreenTexturedQuad(m_renderDevice->m_FBShader);
   return outputRT;
}

RenderTarget* Renderer::ApplyUpscaling(RenderTarget* renderedRT, RenderTarget* outputBackBuffer)
{
   if (m_renderWidth >= GetBackBufferTexture()->GetWidth())
      return renderedRT;

   // TODO add AMD FidelityFX Super Resolution (FSR) support

   m_renderDevice->ResetRenderState();
   m_renderDevice->SetRenderState(RenderState::ALPHABLENDENABLE, RenderState::RS_FALSE);
   m_renderDevice->SetRenderState(RenderState::CULLMODE, RenderState::CULL_NONE);
   m_renderDevice->SetRenderState(RenderState::ZWRITEENABLE, RenderState::RS_FALSE);
   m_renderDevice->SetRenderState(RenderState::ZENABLE, RenderState::RS_FALSE);

   assert(renderedRT != outputBackBuffer); // At this point, renderedRT may be PP1, PP2 or backbuffer
   RenderTarget* outputRT = outputBackBuffer ? outputBackBuffer : GetPostProcessRenderTarget(renderedRT);
   assert(outputRT != renderedRT);
   m_renderDevice->SetRenderTarget("Upscale"s, outputRT, false);
   m_renderDevice->AddRenderTargetDependency(renderedRT);
   m_renderDevice->m_FBShader->SetTexture(SHADER_tex_fb_filtered, renderedRT->GetColorSampler());
   m_renderDevice->m_FBShader->SetTechnique(SHADER_TECHNIQUE_fb_copy);
   m_renderDevice->m_FBShader->SetVector(SHADER_w_h_height, (float)(1.0 / renderedRT->GetWidth()), (float)(1.0 / renderedRT->GetHeight()), 1.0f, 1.0f);
   m_renderDevice->DrawFullscreenTexturedQuad(m_renderDevice->m_FBShader);
   return outputRT;
}

RenderTarget* Renderer::ApplyStereo(RenderTarget* renderedRT, RenderTarget* outputBackBuffer)
{
   // Always applied as the last pass
   assert(outputBackBuffer != nullptr);

   // STEREO_OFF: nothing to do
   if (m_stereo3D == STEREO_OFF)
      return renderedRT;
   
   m_renderDevice->ResetRenderState();
   m_renderDevice->SetRenderState(RenderState::ALPHABLENDENABLE, RenderState::RS_FALSE);
   m_renderDevice->SetRenderState(RenderState::CULLMODE, RenderState::CULL_NONE);
   m_renderDevice->SetRenderState(RenderState::ZWRITEENABLE, RenderState::RS_FALSE);
   m_renderDevice->SetRenderState(RenderState::ZENABLE, RenderState::RS_FALSE);

   if (m_stereo3D == STEREO_VR)
   {
   #if defined(ENABLE_XR) || defined(ENABLE_VR)
      int w = renderedRT->GetWidth(), h = renderedRT->GetHeight();

      #if defined(ENABLE_XR)
         // Rendering is already directly being performed to the swapchain image, so nothing to do except for depth buffer
         // TODO we should directly use the swapchain depth buffer too to avoid the copy
         // FIXME this will not work as the current backbuffer is declared as not having a depth buffer (even if it has like here), beside BGFX does not support blitting depth to the default backbuffer
         if (g_pplayer->m_vrDevice->UseDepthBuffer())
         {
            // Copy depth buffer to OpenXR swapchain's current depth target
            m_renderDevice->SetRenderTarget("OpenXR-Depth"s, outputBackBuffer, true, true);
            m_renderDevice->AddRenderTargetDependency(GetBackBufferTexture(), true);
            m_renderDevice->BlitRenderTarget(GetBackBufferTexture(), outputBackBuffer, false, true);
         }
         // FIXME no preview for Vulkan (as we are not creating the desktop swapchain)
         if (bgfx::getRendererType() == bgfx::RendererType::Vulkan)
            return outputBackBuffer;
      #elif defined(ENABLE_VR)
         // Copy each eye to the HMD texture
         assert(renderedRT != outputBackBuffer);
            
         RenderTarget *leftTexture = GetOffscreenVR(0);
         m_renderDevice->SetRenderTarget("Left Eye"s, leftTexture, false);
         m_renderDevice->AddRenderTargetDependency(renderedRT);
         m_renderDevice->BlitRenderTarget(renderedRT, leftTexture, true, false, 0, 0, w, h, 0, 0, w, h, 0, 0);

         RenderTarget *rightTexture = GetOffscreenVR(1);
         m_renderDevice->SetRenderTarget("Right Eye"s, rightTexture, false);
         m_renderDevice->AddRenderTargetDependency(renderedRT);
         m_renderDevice->BlitRenderTarget(renderedRT, rightTexture, true, false, 0, 0, w, h, 0, 0, w, h, 1, 0);
      #endif

      // Blit preview
      #if defined(ENABLE_XR)
         assert(m_renderDevice->m_outputWnd.size() == 2); // For the time being, we rely on the fact that the First output is the VR Headset, and the second is the VR preview OS window
         RenderTarget* previewRT = m_renderDevice->m_outputWnd[1]->GetBackBuffer();
         m_renderDevice->SetRenderTarget("VR Preview"s, previewRT, false, true);

      #elif defined(ENABLE_VR)
         RenderTarget* previewRT = outputBackBuffer;
         m_renderDevice->SetRenderTarget("VR Preview"s, previewRT, false);
         m_renderDevice->AddRenderTargetDependency(leftTexture); // To ensure blit is made
         m_renderDevice->AddRenderTargetDependency(rightTexture); // To ensure blit is made
      #endif
      m_renderDevice->AddRenderTargetDependency(renderedRT);
      const int previewW = m_vrPreview == VRPREVIEW_BOTH ? previewRT->GetWidth() / 2 : previewRT->GetWidth(), previewH = previewRT->GetHeight();
      const float ar = (float)w / (float)h, previewAr = (float)previewW / (float)previewH;
      int x = 0, y = 0;
      int fw = w, fh = h;
      if ((m_vrPreviewShrink && ar < previewAr) || (!m_vrPreviewShrink && ar > previewAr))
      { // Fit on Y
         const int scaledW = (int)((float)h * previewAr);
         x = (w - scaledW) / 2;
         fw = scaledW;
      }
      else
      { // Fit on X
         const int scaledH = (int)((float)w / previewAr);
         y = (h - scaledH) / 2;
         fh = scaledH;
      }
      if (m_vrPreviewShrink || m_vrPreview == VRPREVIEW_DISABLED)
         m_renderDevice->Clear(clearType::TARGET | clearType::ZBUFFER, 0x00000000);

      #if defined(ENABLE_XR)
         Vertex3D_TexelOnly verts[4] =
         {
            { -1.0f,  1.0f, 0.0f, static_cast<float>(x     ) / w, static_cast<float>(y     ) / h },
            {  1.0f,  1.0f, 0.0f, static_cast<float>(x + fw) / w, static_cast<float>(y     ) / h },
            { -1.0f, -1.0f, 0.0f, static_cast<float>(x     ) / w, static_cast<float>(y + fh) / h },
            {  1.0f, -1.0f, 0.0f, static_cast<float>(x + fw) / w, static_cast<float>(y + fh) / h }
         };
         m_renderDevice->m_FBShader->SetTexture(SHADER_tex_fb_filtered, renderedRT->GetColorSampler());
         m_renderDevice->m_FBShader->SetVector(SHADER_bloom_dither_colorgrade, 0.f, 0.f, 0.f, 0.f);
         m_renderDevice->m_FBShader->SetVector(SHADER_exposure_wcg, m_exposure, 1.f, /*100.f*/ /*203.f*/ 350.f / 10000.f, 0.f); 
         m_renderDevice->m_FBShader->SetTechnique(SHADER_TECHNIQUE_fb_agxtonemap);
         if (m_vrPreview == VRPREVIEW_LEFT || m_vrPreview == VRPREVIEW_RIGHT)
         {
            m_renderDevice->m_FBShader->SetInt(SHADER_layer, m_vrPreview == VRPREVIEW_LEFT ? 0 : 1);
            m_renderDevice->DrawTexturedQuad(m_renderDevice->m_FBShader, verts);
         }
         else if (m_vrPreview == VRPREVIEW_BOTH)
         {
            verts[0].x = verts[2].x = -1.f;
            verts[1].x = verts[3].x = 0.f;
            m_renderDevice->m_FBShader->SetInt(SHADER_layer, 0);
            m_renderDevice->DrawTexturedQuad(m_renderDevice->m_FBShader, verts);
            verts[0].x = verts[2].x = 0.f;
            verts[1].x = verts[3].x = 1.f;
            m_renderDevice->m_FBShader->SetInt(SHADER_layer, 1);
            m_renderDevice->DrawTexturedQuad(m_renderDevice->m_FBShader, verts);
         }

         if (m_vrApplyColorKey)
         {
            // Apply a color mask for color keying. For the time being, this is the only way we have to support mixed reality
            // as HMD does not expose passthrough layers to PCVR (at least Meta Quest 3, used for development).
            // Therefore we leverage VirtualDesktop color keying feature. This needs to be performed as a post process to avoid
            // blending the color key with the rendered scene (alpha blending which would be kept, as it is not fullfilling the
            // color key after blending).
            m_renderDevice->SetRenderTarget("VR ColorKeying"s, m_renderDevice->GetOutputBackBuffer(), true, true);
            m_renderDevice->AddRenderTargetDependency(previewRT);
            Matrix3D matWorldViewProj[2];
            matWorldViewProj[0].SetIdentity();
            matWorldViewProj[1].SetIdentity();
            m_renderDevice->m_basicShader->SetMatrix(SHADER_matWorldViewProj, &matWorldViewProj[0], 2);
            m_renderDevice->m_basicShader->SetVector(SHADER_staticColor_Alpha, &m_vrColorKey);
            m_renderDevice->m_basicShader->SetTechnique(SHADER_TECHNIQUE_unshaded_without_texture);
            static constexpr Vertex3D_NoTex2 ckVerts[4] =
            {
               { -1.0f,  1.0f, 1.0f },
               {  1.0f,  1.0f, 1.0f },
               { -1.0f, -1.0f, 1.0f },
               {  1.0f, -1.0f, 1.0f }
            };
            m_renderDevice->SetRenderState(RenderState::ZENABLE, RenderState::RS_TRUE);
            m_renderDevice->SetRenderState(RenderState::ZFUNC, RenderState::Z_LESSEQUAL);
            m_renderDevice->DrawTexturedQuad(m_renderDevice->m_basicShader, ckVerts);
            m_renderDevice->m_basicShader->SetVector(SHADER_staticColor_Alpha, 1.f, 1.f, 1.f, 1.f);
         }

      #elif defined(ENABLE_VR)
         if (m_vrPreview == VRPREVIEW_LEFT || m_vrPreview == VRPREVIEW_RIGHT)
         {
            m_renderDevice->BlitRenderTarget(renderedRT, previewRT, true, false, x, y, fw, fh, 0, 0, previewW, previewH, m_vrPreview == VRPREVIEW_LEFT ? 0 : 1, 0);
         }
         else if (m_vrPreview == VRPREVIEW_BOTH)
         {
            m_renderDevice->BlitRenderTarget(renderedRT, previewRT, true, false, x, y, fw, fh, 0, 0, previewW, previewH, 0, 0);
            m_renderDevice->BlitRenderTarget(renderedRT, previewRT, true, false, x, y, fw, fh, previewW, 0, previewW, previewH, 1, 0);
         }
         m_renderDevice->SubmitVR(renderedRT);
      #endif
   #endif

      return outputBackBuffer;
   }

   if (m_stereo3Denabled)
   {
      // Anaglyph and 3DTV
      // For anaglyph, defocus the "lesser" eye (the one with a darker color, which should be the non dominant eye of the player)
      if (IsAnaglyphStereoMode(m_stereo3D) && m_stereo3DDefocus != 0.f)
      {
         RenderTarget *tmpRT = GetPostProcessRenderTarget(renderedRT);
         RenderTarget* outputRT = GetPostProcessRenderTarget(tmpRT);
         m_renderDevice->DrawGaussianBlur(renderedRT, tmpRT, outputRT, abs(m_stereo3DDefocus) * 39.f, m_stereo3DDefocus > 0.f ? 0 : 1);
         renderedRT = outputRT;
      }
      // Stereo composition
      RenderTarget* outputRT = outputBackBuffer;
      m_renderDevice->SetRenderTarget("Stereo"s, outputRT, false);
      m_renderDevice->AddRenderTargetDependency(renderedRT);
      m_renderDevice->m_stereoShader->SetTexture(SHADER_tex_stereo_fb, renderedRT->GetColorSampler());
      m_renderDevice->DrawFullscreenTexturedQuad(m_renderDevice->m_stereoShader);
      return outputRT;
   }

   // 3D stereo rendering, but display mono (first eye)
   // Nothing to do: last pass will just output to the backbuffer first layer (not fully clean as we are still pushing an instance draw, but driver will discard missing layers)
   return renderedRT;
}

#pragma endregion

void Renderer::RenderFrame()
{
   // Keep previous render as a reflection probe for ball reflection and for hires motion blur
   SwapBackBufferRenderTargets();

   // Reinitialize parts that have been modified
   SetupShaders();
   for (auto renderable : m_renderableToInit)
   {
      renderable->RenderRelease();
      renderable->RenderSetup(m_renderDevice);
   }
   m_renderableToInit.clear();

   // Update backdrop visibility and visibility mask
   // For the time being, the RenderFrame only support rendering one 3D view for main scene: dedicated 3D rendering for backglass, topper, apron are not yet implemented
   m_noBackdrop = (g_pplayer->m_vrDevice != nullptr) || (m_table->GetViewMode() == BG_FULLSCREEN);
   if (g_pplayer->m_vrDevice)
      m_visibilityMask = m_vrApplyColorKey ? PartGroupData::PlayerModeVisibilityMask::PMVM_MIXED_REALITY : PartGroupData::PlayerModeVisibilityMask::PMVM_VIRTUAL_REALITY;
   else if (m_table->GetViewMode() == BG_FULLSCREEN)
      m_visibilityMask = PartGroupData::PlayerModeVisibilityMask::PMVM_CABINET;
   else if (m_table->GetViewMode() == BG_FSS)
      m_visibilityMask = PartGroupData::PlayerModeVisibilityMask::PMVM_FSS;
   else if (m_table->GetViewMode() == BG_DESKTOP)
      m_visibilityMask = PartGroupData::PlayerModeVisibilityMask::PMVM_DESKTOP;

   // Setup ball rendering: collect all lights that can reflect on balls
   m_ballTrailMeshBufferPos = 0;
   m_ballReflectedLights.clear();
   for (size_t i = 0; i < m_table->m_vedit.size(); i++)
   {
      IEditable* const item = m_table->m_vedit[i];
      if (item && item->GetItemType() == eItemLight && static_cast<Light*>(item)->m_d.m_showReflectionOnBall && !static_cast<Light*>(item)->m_backglass)
         m_ballReflectedLights.push_back(static_cast<Light*>(item));
   }
   // We don't need to set the dependency on the previous frame render as this would be a cross frame dependency which does not have any meaning since dependencies are resolved per frame
   // m_renderDevice->AddRenderTargetDependency(m_renderDevice->GetPreviousBackBufferTexture());
   m_renderDevice->m_ballShader->SetTexture(SHADER_tex_ball_playfield, GetPreviousBackBufferTexture()->GetColorSampler());

   // Update camera point of view
   m_mvpSpaceReference = PartGroupData::SpaceReference::SR_PLAYFIELD;
   #if defined(ENABLE_VR) || defined(ENABLE_XR)
   if (m_stereo3D == STEREO_VR)
   {
      g_pplayer->m_vrDevice->UpdateVRPosition(m_mvpSpaceReference, GetMVP());
      UpdateBasicShaderMatrix();
      UpdateBallShaderMatrix();
   }
   else 
   #endif
   // Legacy headtracking (to be moved to a plugin, using plugin API to update camera)
   if (g_pplayer->m_headTracking)
   {
      #ifndef __STANDALONE__
      Matrix3D matView;
      Matrix3D matProj[2];
      BAMView::createProjectionAndViewMatrix(&matProj[0]._11, &matView._11);
      m_mvp->SetView(matView);
      for (unsigned int eye = 0; eye < m_mvp->m_nEyes; eye++)
         m_mvp->SetProj(eye, matProj[eye]);
      #endif
   }
   m_playfieldView = m_mvp->GetView();

   // Start from the prerendered parts/background or a clear background for VR & editor
   if (m_stereo3D == STEREO_VR || g_pplayer->GetInfoMode() == IF_DYNAMIC_ONLY || g_pplayer->m_liveUI->IsEditorViewMode())
   {
      m_renderDevice->SetRenderTarget("Render Scene"s, GetMSAABackBufferTexture());
      if (g_pplayer->m_liveUI->IsEditorViewMode())
         m_renderDevice->Clear(clearType::TARGET | clearType::ZBUFFER, 0x000D0D0D);
      else
         m_renderDevice->Clear(clearType::TARGET | clearType::ZBUFFER, 0x00000000);
      #ifdef ENABLE_XR
      if (g_pplayer->m_vrDevice && m_stereo3D == STEREO_VR)
      {
         if (std::shared_ptr<MeshBuffer> mask = g_pplayer->m_vrDevice->GetVisibilityMask(); mask)
         {
            static constexpr Vertex3Ds pos{0.f, 0.f, 200000.0f}; // Very high depth bias to ensure being rendered before other opaque parts (which are sorted front to back)
            m_renderDevice->ResetRenderState();
            m_renderDevice->SetRenderState(RenderState::CULLMODE, RenderState::CULL_NONE);
            m_renderDevice->SetRenderState(RenderState::COLORWRITEENABLE, RenderState::RS_FALSE);
            m_renderDevice->SetRenderState(RenderState::ZWRITEENABLE, RenderState::RS_TRUE);
            m_renderDevice->SetRenderState(RenderState::ZENABLE, RenderState::RS_TRUE);
            m_renderDevice->SetRenderState(RenderState::ZFUNC, RenderState::Z_ALWAYS);
            m_renderDevice->m_basicShader->SetMatrix(SHADER_matWorldViewProj, g_pplayer->m_vrDevice->GetVisibilityMaskProjs(), 2);
            m_renderDevice->m_basicShader->SetTechnique(SHADER_TECHNIQUE_vr_mask);
            m_renderDevice->DrawMesh(m_renderDevice->m_basicShader, false, pos, 0, mask, RenderDevice::TRIANGLELIST, 0, mask->m_ib->m_count);
            UpdateBasicShaderMatrix();
         }
      }
      #endif
   }
   else
   {
      RenderStaticPrepass(); // Update statically prerendered parts if needed
      m_renderDevice->SetRenderTarget("Render Scene"s, GetMSAABackBufferTexture());
      m_renderDevice->AddRenderTargetDependency(m_staticPrepassRT);
      m_renderDevice->BlitRenderTarget(m_staticPrepassRT, GetMSAABackBufferTexture());
   }

   RenderDynamics();

   g_pplayer->m_liveUI->Render3D();

   // Keep latency low by proceeding input, ... (as next passes are not affected by the game logic)
   g_pplayer->UpdateGameLogic();

   // Resolve MSAA buffer to a normal one (noop if not using MSAA), allowing sampling it for postprocessing
   if (GetMSAABackBufferTexture() != GetBackBufferTexture())
   {
      RenderPass* const initial_rt = m_renderDevice->GetCurrentPass();
      m_renderDevice->SetRenderTarget("Resolve MSAA"s, GetBackBufferTexture());
      m_renderDevice->BlitRenderTarget(GetMSAABackBufferTexture(), GetBackBufferTexture(), true, true);
      m_renderDevice->SetRenderTarget(initial_rt->m_name, initial_rt->m_rt);
      initial_rt->m_name += '-';
   }

   // Starts from the output of the render passes
   RenderTarget* renderedRT = GetBackBufferTexture();

   // Add screen space reflections
   renderedRT = ApplyAdditiveScreenSpaceReflection(renderedRT);

   // Clear embedded ancillary windows before updarting bloom & AO
   ClearEmbeddedAncillaryWindow(VPXWindowId::VPXWINDOW_Backglass, g_pplayer->m_backglassOutput, renderedRT);
   ClearEmbeddedAncillaryWindow(VPXWindowId::VPXWINDOW_ScoreView, g_pplayer->m_scoreViewOutput, renderedRT);
   ClearEmbeddedAncillaryWindow(VPXWindowId::VPXWINDOW_Topper, g_pplayer->m_topperOutput, renderedRT);

   // Compute AO contribution (to be applied later, with tonemapping)
   UpdateAmbientOcclusion(renderedRT);

   // Compute bloom (to be applied later, with tonemapping)
   UpdateBloom(renderedRT);

   // Render ancillary windows (eventually embedded in the main window, so must be done after main rendering but before post process)
   RenderAncillaryWindow(VPXWindowId::VPXWINDOW_Backglass, g_pplayer->m_backglassOutput, renderedRT, g_pplayer->m_ancillaryWndRenderers[VPXWindowId::VPXWINDOW_Backglass]);
   RenderAncillaryWindow(VPXWindowId::VPXWINDOW_ScoreView, g_pplayer->m_scoreViewOutput, renderedRT, g_pplayer->m_ancillaryWndRenderers[VPXWindowId::VPXWINDOW_ScoreView]);
   RenderAncillaryWindow(VPXWindowId::VPXWINDOW_Topper, g_pplayer->m_topperOutput, renderedRT, g_pplayer->m_ancillaryWndRenderers[VPXWindowId::VPXWINDOW_Topper]);

   const bool hasAntialiasPass = m_FXAA != Disabled;
   const bool hasSharpenPass = m_sharpen != 0;
   const bool hasUpscalerPass = m_renderWidth < GetBackBufferTexture()->GetWidth();
   // OpenXR directly renders to the XR render target view without any postprocess needs
   #ifdef ENABLE_XR
   const bool hasStereoPass = m_stereo3Denabled && (m_stereo3D != STEREO_OFF) && (m_stereo3D != STEREO_VR);
   #else
   const bool hasStereoPass = m_stereo3Denabled && (m_stereo3D != STEREO_OFF);
   #endif

   // Perform color grade LUT / dither / tonemapping, also applying bloom and AO
   RenderTarget* const tonemapRT = (hasAntialiasPass || hasSharpenPass || hasStereoPass || hasUpscalerPass) ? GetPostProcessRenderTarget1() : m_renderDevice->GetOutputBackBuffer();
   const ShaderTechniques tonemapTechnique = ApplyTonemapping(renderedRT, tonemapRT);

   // Raytraced ball motion blur (BGFX only)
   renderedRT = ApplyBallMotionBlur(renderedRT, tonemapRT, tonemapTechnique);

   // Perform post processed anti aliasing
   renderedRT = ApplyPostProcessedAntialiasing(renderedRT, (hasSharpenPass || hasStereoPass || hasUpscalerPass) ? nullptr : m_renderDevice->GetOutputBackBuffer());

   // Performs sharpening
   renderedRT = ApplySharpening(renderedRT, (hasStereoPass || hasUpscalerPass) ? nullptr : m_renderDevice->GetOutputBackBuffer());

   // Upscale: When using downscaled backbuffer (for performance reason), upscaling is done after postprocessing
   renderedRT = ApplyUpscaling(renderedRT, hasStereoPass ? nullptr : m_renderDevice->GetOutputBackBuffer());

   // If using OpenVR, render LiveUI before pushing eyes to headset
   // If using 3D TV stereo mode, render LiveUI before stereo as it must be duplicated per view to be correct
   // For other modes, render UI after all other steps (otherwise it would break the calibration process for stereo anaglyph, and breaks XR passthrough color keying)
   const bool uiBeforeStero = false
#ifdef ENABLE_VR
      || m_stereo3D == STEREO_VR
#endif
      || m_stereo3D == STEREO_SBS || m_stereo3D == STEREO_INT || m_stereo3D == STEREO_TB || m_stereo3D == STEREO_FLIPPED_INT;
   if (uiBeforeStero)
   {
      m_renderDevice->SetRenderTarget("LiveUI"s, renderedRT, true, true);
      g_pplayer->m_liveUI->RenderUI();
   }

   // Apply stereo
   renderedRT = ApplyStereo(renderedRT, m_renderDevice->GetOutputBackBuffer());

   if (!uiBeforeStero)
   {
      m_renderDevice->SetRenderTarget("LiveUI"s, renderedRT, true, true);
      g_pplayer->m_liveUI->RenderUI();
   }

   // The last rendered render target must be the output back buffer
   assert(renderedRT == m_renderDevice->GetOutputBackBuffer());

   if (g_pplayer->GetProfilingMode() == PF_ENABLED)
      m_gpu_profiler.Timestamp(GTS_PostProcess);
}


////////////////////////////////////////////////////////////////////////////////

void Renderer::DrawImage(VPXRenderContext2D* ctx, VPXTexture texture, const float tintR, const float tintG, const float tintB, const float alpha, const float texX, const float texY,
   const float texW, const float texH, const float pivotX, const float pivotY, const float rotation, const float srcX, const float srcY, const float srcW, const float srcH)
{
   if (alpha <= 0.f) // Alpha blended, so alpha = 0 means not visible
      return;
   const bool isLinearOutput = *((bool*)ctx->rendererData);
   std::shared_ptr<BaseTexture> const tex = VPXPluginAPIImpl::GetInstance().GetTexture(texture);
   RenderDevice* const rdl = g_pplayer->m_renderer->m_renderDevice;
   rdl->ResetRenderState();
   rdl->SetRenderState(RenderState::ZWRITEENABLE, RenderState::RS_FALSE);
   rdl->SetRenderState(RenderState::ZENABLE, RenderState::RS_FALSE);
   rdl->SetRenderState(RenderState::CULLMODE, RenderState::CULL_NONE);
   rdl->SetRenderState(RenderState::SRCBLEND, RenderState::SRC_ALPHA);
   rdl->SetRenderState(RenderState::DESTBLEND, RenderState::INVSRC_ALPHA);
   rdl->SetRenderState(RenderState::BLENDOP, RenderState::BLENDOP_ADD);
   rdl->SetRenderState(RenderState::ALPHABLENDENABLE, (alpha != 1.f || !tex->IsOpaque()) ? RenderState::RS_TRUE : RenderState::RS_FALSE);
   rdl->m_basicShader->SetVector(SHADER_cBase_Alpha, tintR, tintG, tintB, alpha);
   // We force to linear (no sRGB decoding) when rendering in sRGB colorspace, this assumes that the texture is in sRGB colorspace to get correct gamma (other situations would need dedicated shaders to handle them efficiently)
   assert(tex->m_format == BaseTexture::SRGB || tex->m_format == BaseTexture::SRGBA || tex->m_format == BaseTexture::SRGB565);
   // Disable filtering and mipmap generation if they are not needed
   const SamplerFilter sf = (ctx->is2D && (srcW * ctx->outWidth == ctx->srcWidth * (float)tex->width()) && (srcH * ctx->outHeight == ctx->srcHeight * (float)tex->height()))
      ? SamplerFilter::SF_NONE
      : SamplerFilter::SF_UNDEFINED;
   rdl->m_basicShader->SetTexture(SHADER_tex_base_color, tex.get(), !isLinearOutput, sf);
   const float vx1 = srcX / ctx->srcWidth;
   const float vy1 = srcY / ctx->srcHeight;
   const float vx2 = vx1 + srcW / ctx->srcWidth;
   const float vy2 = vy1 + srcH / ctx->srcHeight;
   const float tx1 = texX / (float)tex->width();
   const float ty1 = 1.f - texY / (float)tex->height();
   const float tx2 = (texX + texW) / (float)tex->width();
   const float ty2 = 1.f - (texY + texH) / (float)tex->height();
   Vertex3D_NoTex2 vertices[4]
      = { { vx2, vy1, 0.f, 0.f, 0.f, 1.f, tx2, ty2 }, { vx1, vy1, 0.f, 0.f, 0.f, 1.f, tx1, ty2 }, { vx2, vy2, 0.f, 0.f, 0.f, 1.f, tx2, ty1 }, { vx1, vy2, 0.f, 0.f, 0.f, 1.f, tx1, ty1 } };
   if (rotation != 0.f)
   {
      const float px = lerp(vx1, vx2, (pivotX - texX) / (float)tex->width());
      const float py = lerp(vy1, vy2, (pivotY - texY) / (float)tex->height());
      const Matrix3D matRot = Matrix3D::MatrixTranslate(-px, -py, 0.f) * Matrix3D::MatrixRotateZ(rotation * (float)(M_PI / 180.0)) * Matrix3D::MatrixTranslate(px, py, 0.f);
      matRot.TransformPositions(vertices, vertices, 4);
   }
   rdl->DrawTexturedQuad(rdl->m_basicShader, vertices, true, 0.f);
}

void Renderer::DrawMatrixDisplay(VPXRenderContext2D* ctx, VPXDisplayRenderStyle style, VPXTexture glassTex, const float glassTintR, const float glassTintG, const float glassTintB,
   const float glassRoughness, const float glassAreaX, const float glassAreaY, const float glassAreaW, const float glassAreaH, const float glassAmbientR, const float glassAmbientG,
   const float glassAmbientB, VPXTexture dispTex, const float dispTintR, const float dispTintG, const float dispTintB, const float brightness, const float alpha, const float dispPadL,
   const float dispPadT, const float dispPadR, const float dispPadB, const float srcX, const float srcY, const float srcW, const float srcH)
{
   const bool isLinearOutput = *((bool*)ctx->rendererData);
   VPXPluginAPIImpl& vxpApi = VPXPluginAPIImpl::GetInstance();
   std::shared_ptr<BaseTexture> const gTex = glassTex ? vxpApi.GetTexture(glassTex) : nullptr;
   std::shared_ptr<BaseTexture> const dTex = vxpApi.GetTexture(dispTex);
   RenderDevice* const rdl = g_pplayer->m_renderer->m_renderDevice;
   rdl->ResetRenderState();
   rdl->SetRenderState(RenderState::ALPHABLENDENABLE, RenderState::RS_FALSE);
   rdl->SetRenderState(RenderState::CULLMODE, RenderState::CULL_NONE);
   rdl->SetRenderState(RenderState::ZWRITEENABLE, RenderState::RS_FALSE);
   rdl->SetRenderState(RenderState::ZENABLE, RenderState::RS_FALSE);
   g_pplayer->m_renderer->SetupDMDRender(style, false, vec3(dispTintR, dispTintG, dispTintB), brightness, dTex, alpha,
      isLinearOutput ? Renderer::ColorSpace::Linear : Renderer::ColorSpace::Reinhard_sRGB,
      nullptr, // No parallax
      vec4(dispPadL, dispPadT, dispPadR, dispPadB), vec3(glassTintR, glassTintG, glassTintB), glassRoughness, gTex.get(), vec4(glassAreaX, glassAreaY, glassAreaW, glassAreaH),
      vec3(glassAmbientR, glassAmbientG, glassAmbientB));
   const float vx1 = srcX / ctx->srcWidth;
   const float vy1 = 1.f - srcY / ctx->srcHeight;
   const float vx2 = (srcX + srcW) / ctx->srcWidth;
   const float vy2 = 1.f - (srcY + srcH) / ctx->srcHeight;
   const Vertex3D_NoTex2 vertices[4] = { //
      { vx2, vy1, 0.f, 0.f, 0.f, 1.f, 1.f, 1.f }, // 
      { vx1, vy1, 0.f, 0.f, 0.f, 1.f, 0.f, 1.f }, //
      { vx2, vy2, 0.f, 0.f, 0.f, 1.f, 1.f, 0.f },  //
      { vx1, vy2, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f } };
   rdl->DrawTexturedQuad(rdl->m_DMDShader, vertices, true, 0.f);
}

void Renderer::DrawSegmentDisplay(VPXRenderContext2D* ctx, VPXSegDisplayRenderStyle style, VPXSegDisplayHint shapeHint, VPXTexture glassTex, const float glassTintR, const float glassTintG,
   const float glassTintB, const float glassRoughness, const float glassAreaX, const float glassAreaY, const float glassAreaW, const float glassAreaH, const float glassAmbientR,
   const float glassAmbientG, const float glassAmbientB, SegElementType type, const float* state, const float dispTintR, const float dispTintG, const float dispTintB, const float brightness,
   const float alpha, const float dispPadL, const float dispPadT, const float dispPadR, const float dispPadB, const float srcX, const float srcY, const float srcW, const float srcH)
{
   const bool isLinearOutput = *((bool*)ctx->rendererData);
   VPXPluginAPIImpl& vxpApi = VPXPluginAPIImpl::GetInstance();
   std::shared_ptr<BaseTexture> const gTex = glassTex ? vxpApi.GetTexture(glassTex) : nullptr;
   RenderDevice* const rdl = g_pplayer->m_renderer->m_renderDevice;
   // Use max blending as segment may overlap in the glass diffuse: we retain the most lighted one which is wrong but looks ok (otherwise we would have to deal with colorspace conversions and layering between glass and emitter)
   rdl->ResetRenderState();
   rdl->SetRenderState(RenderState::BLENDOP, RenderState::BLENDOP_MAX);
   rdl->SetRenderState(RenderState::ALPHABLENDENABLE, RenderState::RS_TRUE);
   rdl->SetRenderState(RenderState::SRCBLEND, RenderState::SRC_ALPHA);
   rdl->SetRenderState(RenderState::DESTBLEND, RenderState::ONE);
   rdl->SetRenderState(RenderState::CULLMODE, RenderState::CULL_NONE);
   rdl->SetRenderState(RenderState::ZWRITEENABLE, RenderState::RS_FALSE);
   rdl->SetRenderState(RenderState::ZENABLE, RenderState::RS_FALSE);
   g_pplayer->m_renderer->SetupSegmentRenderer(style, false, vec3(dispTintR, dispTintG, dispTintB), brightness, (Renderer::SegmentFamily)shapeHint, type, state,
      isLinearOutput ? Renderer::ColorSpace::Linear : Renderer::ColorSpace::Reinhard_sRGB,
      nullptr, // No parallax
      vec4(dispPadL, dispPadT, dispPadR, dispPadB), vec3(glassTintR, glassTintG, glassTintB), glassRoughness, gTex.get(), vec4(glassAreaX, glassAreaY, glassAreaW, glassAreaH),
      vec3(glassAmbientR, glassAmbientG, glassAmbientB));
   const float vx1 = srcX / ctx->srcWidth;
   const float vy1 = 1.f - srcY / ctx->srcHeight;
   const float vx2 = (srcX + srcW) / ctx->srcWidth;
   const float vy2 = 1.f - (srcY + srcH) / ctx->srcHeight;
   const Vertex3D_NoTex2 vertices[4] = { // 
      { vx2, vy1, 0.f, 0.f, 0.f, 1.f, 1.f, 1.f }, //
      { vx1, vy1, 0.f, 0.f, 0.f, 1.f, 0.f, 1.f }, //
      { vx2, vy2, 0.f, 0.f, 0.f, 1.f, 1.f, 0.f }, //
      { vx1, vy2, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f } };
   rdl->DrawTexturedQuad(rdl->m_DMDShader, vertices, true, 0.f);
}

RenderTarget* Renderer::SetupAncillaryRenderTarget(VPXWindowId window, VPX::RenderOutput& output, RenderTarget* embedRT, int& outputX, int& outputY, int& outputW, int& outputH, bool& isOutputLinear)
{
   assert(VPXWindowId::VPXWINDOW_Backglass <= window && window <= VPXWindowId::VPXWINDOW_Topper);
   static std::array<string, 3> renderPassNames = { "Backglass Render"s, "ScoreView Render"s, "Topper Render"s };
   static std::array<string, 3> hdrRTNames = { "BackglassBackBuffer"s, "ScoreViewBackBuffer"s, "TopperBackBuffer"s };
   const string renderPassName = renderPassNames[window - VPXWindowId::VPXWINDOW_Backglass];
   const string hdrRTName = hdrRTNames[window - VPXWindowId::VPXWINDOW_Backglass];

   // TODO implement rendering for VR (on a flasher)
   if (g_pplayer->m_vrDevice != nullptr)
      return nullptr;

   // Stereo Postprocessing is not yet implemented for embedded window
   if (m_stereo3D != StereoMode::STEREO_OFF && output.GetMode() != VPX::RenderOutput::OM_WINDOW)
      return nullptr;

   RenderTarget* outputRT;
   if (output.GetMode() == VPX::RenderOutput::OM_EMBEDDED)
   {
      outputRT = embedRT;
      VPX::Window* containerWnd = m_renderDevice->m_outputWnd[0];

      const float displayScaleX = static_cast<float>(containerWnd->GetPixelWidth()) / static_cast<float>(containerWnd->GetWidth());
      const float displayScaleY = static_cast<float>(containerWnd->GetPixelHeight()) / static_cast<float>(containerWnd->GetHeight());

      const int wndW = output.GetEmbeddedWindow()->GetWidth();
      const int wndH = output.GetEmbeddedWindow()->GetHeight();
      int wndX;
      int wndY;
      output.GetEmbeddedWindow()->GetPos(wndX, wndY);

      outputW = static_cast<int>((float)wndW * displayScaleX);
      outputH = static_cast<int>((float)wndH * displayScaleY);
      outputX = static_cast<int>((float)wndX * displayScaleX);
      outputY = static_cast<int>((float)wndY * displayScaleY);
   }
#ifdef ENABLE_BGFX
   else if (output.GetMode() == VPX::RenderOutput::OM_WINDOW)
   {
      outputRT = output.GetWindow()->GetBackBuffer();
      outputW = outputRT->GetWidth();
      outputH = outputRT->GetHeight();
      outputX = 0;
      outputY = 0;
   }
#endif
   else
   {
      return nullptr;
   }

   RenderDevice* const rd = m_renderDevice;

   Matrix3D matWorldViewProj[2];
   matWorldViewProj[0] = Matrix3D::MatrixIdentity();
   matWorldViewProj[0]._11 = 2.f * static_cast<float>(outputW) / static_cast<float>(outputRT->GetWidth());
   matWorldViewProj[0]._41 = -1.f + 2.f * static_cast<float>(outputX) / static_cast<float>(outputRT->GetWidth());
   matWorldViewProj[0]._22 = -2.f * static_cast<float>(outputH) / static_cast<float>(outputRT->GetHeight());
   matWorldViewProj[0]._42 = 1.f - 2.f * static_cast<float>(outputY) / static_cast<float>(outputRT->GetHeight());
   const int eyes = m_stereo3D != StereoMode::STEREO_OFF ? 2 : 1;
   if (eyes > 1)
      matWorldViewProj[1] = matWorldViewProj[0];
#if defined(ENABLE_OPENGL)
   struct
   {
      Matrix3D matWorld;
      Matrix3D matView;
      Matrix3D matWorldView;
      Matrix3D matWorldViewInverseTranspose;
      Matrix3D matWorldViewProj[2];
   } matrices;
   memcpy(&matrices.matWorldViewProj[0].m[0][0], &matWorldViewProj[0].m[0][0], 4 * 4 * sizeof(float));
   memcpy(&matrices.matWorldViewProj[1].m[0][0], &matWorldViewProj[0].m[0][0], 4 * 4 * sizeof(float));
   rd->m_basicShader->SetUniformBlock(SHADER_basicMatrixBlock, &matrices.matWorld.m[0][0]);
#else
   rd->m_basicShader->SetMatrix(SHADER_matWorldViewProj, &matWorldViewProj[0], eyes);
#endif
   rd->m_basicShader->SetFloat(SHADER_alphaTestValue, -1.0f);
   rd->m_basicShader->SetTechnique(SHADER_TECHNIQUE_bg_decal_with_texture);
   rd->m_DMDShader->SetMatrix(SHADER_matWorldViewProj, &matWorldViewProj[0], eyes);
   rd->m_DMDShader->SetFloat(SHADER_alphaTestValue, -1.0f);

   // Performing linear rendering + tonemapping is overkill when used for LDR rendering (Pup pack, B2S,...)
   // TODO we should allow plugins to decide if they want linear colorspace + tonemapping or simple sRGB composition
   isOutputLinear = output.GetMode() == VPX::RenderOutput::OM_EMBEDDED;

   if (output.GetMode() == VPX::RenderOutput::OM_WINDOW)
   {
      if (isOutputLinear)
      {
         assert(false); // This is disabled for the time being
         if (m_ancillaryWndHdrRT[window] == nullptr)
            m_ancillaryWndHdrRT[window] = std::unique_ptr<RenderTarget>(new RenderTarget(rd, SurfaceType::RT_DEFAULT, hdrRTName, outputW, outputH, colorFormat::RGBA16F, false, 1, "Fatal Error: unable to create ancillary window back buffer"));
         rd->SetRenderTarget(renderPassName, m_ancillaryWndHdrRT[window].get(), false, true);
      }
      else
      {
         rd->SetRenderTarget(renderPassName, outputRT, false, true);
      }
   }
   else if (output.GetMode() == VPX::RenderOutput::OM_EMBEDDED)
   {
      rd->SetRenderTarget(renderPassName, outputRT, true, true);
   }
   return rd->GetCurrentRenderTarget();
}

void Renderer::ClearEmbeddedAncillaryWindow(VPXWindowId window, VPX::RenderOutput& output, RenderTarget* embedRT)
{
   if (output.GetMode() != VPX::RenderOutput::OM_EMBEDDED)
      return;

   bool isOutputLinear;
   int m_outputX, m_outputY, m_outputW, m_outputH;
   RenderTarget* outputRT = SetupAncillaryRenderTarget(window, output, embedRT, m_outputX, m_outputY, m_outputW, m_outputH, isOutputLinear);
   if (outputRT == nullptr)
      return;

   Vertex3D_NoTex2 vertices[4] = { { 1.f, 1.f, 0.f, 0.f, 0.f, 1.f, 1.f, 1.f }, //
      { 0.f, 1.f, 0.f, 0.f, 0.f, 1.f, 0.f, 1.f }, //
      { 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 1.f, 0.f }, //
      { 0.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f } };
   const float sx = 1.f / static_cast<float>(outputRT->GetWidth());
   const float sy = 1.f / static_cast<float>(outputRT->GetHeight());
   for (unsigned int i = 0; i < 4; ++i)
   {
      vertices[i].x = sx * (vertices[i].x * static_cast<float>(m_outputW) + static_cast<float>(m_outputX)) * 2.0f - 1.0f;
      vertices[i].y = 1.0f - sy * (vertices[i].y * static_cast<float>(m_outputH) + static_cast<float>(m_outputY)) * 2.0f;
   }
   RenderDevice* const rd = m_renderDevice;
   rd->ResetRenderState();
   rd->SetRenderState(RenderState::ZENABLE, RenderState::RS_FALSE);
   rd->SetRenderState(RenderState::ZWRITEENABLE, RenderState::RS_TRUE); // Also clear depth to avoid AO artefacts
   rd->m_DMDShader->SetVector(SHADER_vColor_Intensity, 0.f, 0.f, 0.f, 1.f);
   rd->m_DMDShader->SetTechnique(SHADER_TECHNIQUE_basic_noDMD_notex);
   rd->m_DMDShader->SetVector(SHADER_glassArea, 0.f, 0.f, 1.f, 1.f);
   rd->DrawTexturedQuad(rd->m_DMDShader, vertices);
   rd->GetCurrentPass()->m_commands.back()->SetTransparent(true);
   rd->GetCurrentPass()->m_commands.back()->SetDepth(-10000.f);

   UpdateBasicShaderMatrix();
}

void Renderer::RenderAncillaryWindow(VPXWindowId window, VPX::RenderOutput& output, RenderTarget* embedRT, const vector<AncillaryRendererDef>& ancillaryWndRenderers)
{
   bool isOutputLinear;
   int m_outputX, m_outputY, m_outputW, m_outputH;
   RenderDevice* const rd = m_renderDevice;
   RenderTarget* outputRT = SetupAncillaryRenderTarget(window, output, embedRT, m_outputX, m_outputY, m_outputW, m_outputH, isOutputLinear);
   if (outputRT == nullptr)
      return;

   VPXRenderContext2D context
   {
      window, static_cast<float>(m_outputW), static_cast<float>(m_outputH),
      1, // 2D render
      static_cast<float>(m_outputW), static_cast<float>(m_outputH),
      DrawImage, // Draw an image
      DrawMatrixDisplay, // Draw a display (DMD, CRT, ...)
      DrawSegmentDisplay,  // Draw a segment display element (just one digit, using max blending to allow building a complete display)
      &isOutputLinear // Custom rendering data (for the time being, just the HDR flag)
   };

   rd->ResetRenderState();
   if (output.GetMode() == VPX::RenderOutput::OM_WINDOW)
      rd->Clear(clearType::TARGET | clearType::ZBUFFER, 0x00000000);

   bool rendered = false;
   for (auto& renderer : ancillaryWndRenderers)
   {
      rendered = renderer.Render(&context, renderer.context);
      if (rendered)
         break;
   }

   if (output.GetMode() == VPX::RenderOutput::OM_WINDOW)
   {
      if (!rendered)
      {
         m_renderDevice->GetCurrentPass()->ClearCommands(); // No rendering done, clear the pass (avoids a useless clear)
      }
      else
      {
         if (!output.GetWindow()->IsVisible())
         {
            output.GetWindow()->Show();
            m_renderDevice->m_outputWnd[0]->RaiseAndFocus(); // Keep focus on playfield when showing an ancillary window
         }

         if (isOutputLinear)
         {
            assert(false); // This is disabled for the time being
            static std::array<string, 3> tonemapPassNames = { "Backglass Tonemap"s, "ScoreView Tonemap"s, "Topper Tonemap"s };
            const string tonemapPassName = tonemapPassNames[window - VPXWindowId::VPXWINDOW_Backglass];
            const float jitter = (float)((msec() & 2047) / 1000.0);
            rd->ResetRenderState();
            rd->SetRenderState(RenderState::ZENABLE, RenderState::RS_FALSE);
            rd->SetRenderState(RenderState::CULLMODE, RenderState::CULL_NONE);
            rd->SetRenderTarget(tonemapPassName, outputRT, true, true);
            rd->AddRenderTargetDependency(m_ancillaryWndHdrRT[window].get(), false);
            rd->m_FBShader->SetTextureNull(SHADER_tex_depth);
            rd->m_FBShader->SetTexture(SHADER_tex_fb_unfiltered, m_ancillaryWndHdrRT[window]->GetColorSampler());
            rd->m_FBShader->SetTexture(SHADER_tex_fb_filtered, m_ancillaryWndHdrRT[window]->GetColorSampler());
            rd->m_FBShader->SetVector(SHADER_w_h_height, (float)(1.0 / m_ancillaryWndHdrRT[window]->GetWidth()), (float)(1.0 / m_ancillaryWndHdrRT[window]->GetHeight()), 1.0f, 1.0f);
            rd->m_FBShader->SetVector(SHADER_bloom_dither_colorgrade,
               0.f, // Bloom
               output.GetWindow()->IsWCGBackBuffer() ? 0.f : 1.f, // Dither
               0.f, // LUT colorgrade
               0.f);
            rd->m_FBShader->SetVector(SHADER_w_h_height, static_cast<float>(1.0 / static_cast<double>(m_outputW)), static_cast<float>(1.0 / static_cast<double>(m_outputH)),
               jitter, // radical_inverse(jittertime) * 11.0f,
               jitter); // sobol(jittertime) * 13.0f); // jitter for dither pattern}
            ShaderTechniques tonemapTechnique; // FIXME use a tonemapping corresponding to the output, handling situations where playfield is on a HDR display but backglass is not
            switch (m_toneMapper)
            {
            case TM_REINHARD: tonemapTechnique = SHADER_TECHNIQUE_fb_rhtonemap_no_filter; break;
            case TM_FILMIC: tonemapTechnique = SHADER_TECHNIQUE_fb_fmtonemap_no_filter; break;
            case TM_NEUTRAL: tonemapTechnique = SHADER_TECHNIQUE_fb_nttonemap_no_filter; break;
            case TM_AGX: tonemapTechnique = SHADER_TECHNIQUE_fb_agxtonemap_no_filter; break;
            case TM_AGX_PUNCHY: tonemapTechnique = SHADER_TECHNIQUE_fb_agxptonemap_no_filter; break;
            default: assert(!"unknown tonemapper"); break;
            }
            rd->m_FBShader->SetTechnique(tonemapTechnique);
            constexpr float m_exposure = 1.f; // TODO implement exposure
            if (output.GetWindow()->IsWCGBackBuffer())
            {
               const float maxDisplayLuminance = output.GetWindow()->GetHDRHeadRoom()
                  * (output.GetWindow()->GetSDRWhitePoint() * 80.f); // Maximum luminance of display in nits, note that GetSDRWhitePoint()*80 should usually be in the 200 nits range
               rd->m_FBShader->SetVector(SHADER_exposure_wcg, m_exposure,
                  (output.GetWindow()->GetSDRWhitePoint() * 80.f)
                     / maxDisplayLuminance, // Apply SDR whitepoint (1.0 -> white point in nits), then scale down by maximum luminance (in nits) of display to get a relative value before before tonemapping, equal to 1/GetHDRHeadRoom()
                  maxDisplayLuminance / 10000.f, // Apply back maximum luminance in nits of display after tonemapping, scaled down to PQ limits (1.0 is 10000 nits)
                  1.f);
               float spline_params[6];
               PrecompSplineTonemap(maxDisplayLuminance, spline_params);
               rd->m_FBShader->SetVector(SHADER_spline1, spline_params[0], spline_params[1], spline_params[2], spline_params[3]);
               rd->m_FBShader->SetVector(SHADER_spline2, spline_params[4], spline_params[5], 0.f, 0.f);
            }
            else
            {
               rd->m_FBShader->SetVector(SHADER_exposure_wcg, m_exposure, 1.f,
                  0.f, // Unused for SDR
                  0.f); // Tonemapping mode: 0 = SDR
            }
            rd->DrawFullscreenTexturedQuad(rd->m_FBShader);
         }
      }
   }

   UpdateBasicShaderMatrix();
}
