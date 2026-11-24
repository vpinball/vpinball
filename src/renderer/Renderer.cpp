// license:GPLv3+

#include "core/stdafx.h"
#include "Renderer.h"

#include "core/VPApp.h"
#include "core/VPXPluginAPIImpl.h"
#include "math/bluenoise.h"
#include "math/math.h"
#include "math/matrix.h"
#include "meshes/ballMesh.h"
#include "parts/ball.h"
#include "parts/Collection.h"
#include "parts/light.h"
#include "parts/pintable.h"
#include "physics/cabinet/NudgeHandler.h"
#include "renderer/Anaglyph.h"
#include "renderer/Shader.h"
#include "renderer/RenderCommand.h"
#include "renderer/RenderDevice.h"
#include "renderer/VRDevice.h"
#include "renderer/trace.h"
#include "ui/live/LiveUI.h"
#include "utils/color.h"

#include "ThreadPool.h"
#include "utils/denormals.h"


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
   : m_sceneLighting(table)
   , m_stereo3D(stereo3D)
   , m_table(table)
{
   ApplyTableSettings();

#if defined(ENABLE_BGFX)
   constexpr int MSAASamples[] = { 1, 4, 6, 8, 16 };
   const int nMSAASamples = MSAASamples[m_table->GetSettings().GetPlayer_MSAASamples()];
   const bool compressTextures = m_table->GetSettings().GetPlayer_CompressTextures();
#elif defined(ENABLE_OPENGL)
   constexpr int MSAASamples[] = { 1, 4, 6, 8, 16 };
   int nMSAASamples = MSAASamples[m_table->GetSettings().GetPlayer_MSAASamples()];
   const bool compressTextures = false;
#elif defined(ENABLE_DX9)
   // Sadly DX9 does not support resolving an MSAA depth buffer, making MSAA implementation complex for it. So just disable
   constexpr int nMSAASamples = 1;
   const bool compressTextures = false;
#endif
   const bool useNvidiaApi = m_table->GetSettings().GetPlayer_UseNVidiaAPI();
   const int nEyes = (m_stereo3D == STEREO_VR || m_stereo3D != STEREO_OFF) ? 2 : 1;
   try
   {
      m_renderDevice = new RenderDevice(wnd, nEyes == 2, IsAnaglyphStereoMode(m_stereo3D), m_stereo3D == STEREO_VR, useNvidiaApi, compressTextures, nMSAASamples, syncMode);
   }
   catch (...)
   {
      // TODO better error handling => just let the exception up ?
      throw(E_FAIL);
   }

   if (const bool isHdr2020 = (g_pplayer->m_vrDevice == nullptr) && m_renderDevice->m_outputWnd[0]->IsWCGBackBuffer(); isHdr2020)
   {
      m_exposure *= g_settingsService.GetAppSettings().GetPlayer_HDRGlobalExposure();
      m_bloomOff = true;
   }

   // Create the primary view: the playfield window (or the VR headset when playing in VR)
   m_primaryView = std::make_unique<RenderView>(this, RenderView::Kind::Player);
   m_primaryView->SetOutputWindow(m_renderDevice->m_outputWnd[0]);
   m_primaryView->SetRenderOverlay3D(true);
   m_primaryView->SetRenderLiveUI(true);
   m_primaryView->SetRunGameLogic(true);
   m_activeView = m_primaryView.get();
   m_renderViews.push_back(m_primaryView.get());

   RenderView* const view = m_primaryView.get();
   int renderWidth, renderHeight;
   if (m_stereo3D == STEREO_VR)
   {
      // For VR, renders at the HMD native eye resolution (the desktop display will reuse and scale/stretch it)
      renderWidth = g_pplayer->m_vrDevice->GetEyeWidth();
      renderHeight = g_pplayer->m_vrDevice->GetEyeHeight();
   }
   else if (m_stereo3D == STEREO_SBS)
   {
      // Side by side fits the 2 views along the output width, so each view is rendered at half the output width
      renderWidth = wnd->GetPixelWidth() / 2;
      renderHeight = wnd->GetPixelHeight();
   }
   else if (m_stereo3D == STEREO_TB || m_stereo3D == STEREO_INT || m_stereo3D == STEREO_FLIPPED_INT)
   {
      // Top/Bottom (and interlaced) fits the 2 views along the output height, so each view is rendered at half the output height
      renderWidth = wnd->GetPixelWidth();
      renderHeight = wnd->GetPixelHeight() / 2;
   }
   else
   {
      // Default renders at the output window pixel resolution
      renderWidth = wnd->GetPixelWidth();
      renderHeight = wnd->GetPixelHeight();
   }
   const float AAfactor = Settings::GetPlayer_AAFactor_Property()->GetValid(m_table->GetSettings().GetPlayer_AAFactor());
   view->SetRenderSize(renderWidth, renderHeight, wnd->GetPixelWidth(), wnd->GetPixelHeight(), AAfactor);
   const int renderWidthAA = (int)((float)renderWidth * AAfactor);
   const int renderHeightAA = (int)((float)renderHeight * AAfactor);

   m_ancillaryRenderContext = VPXRenderContext2D {
      VPXWindowId::VPXWINDOW_Playfield, 0.f, 0.f, 0, 0.f, 0.f,
      DrawImage, // Draw an image // -> ctx->DrawImage
      DrawMatrixDisplay, // Draw a display (DMD, CRT, ...) // -> ctx->DrawDisplay
      DrawSegmentDisplay, // Draw a segment display element (just one digit, using max blending to allow building a complete display) // -> ctx->DrawSegDisplay
      &m_ancillaryRenderSetup // Custom rendering data
   };

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
   int maxSamples;
   glGetIntegerv(GL_MAX_SAMPLES, &maxSamples);
   nMSAASamples = min(maxSamples, nMSAASamples);

#elif defined(ENABLE_DX9)
   constexpr colorFormat renderFormat = colorFormat::RGBA16F;

#endif
   const SurfaceType rtType = m_stereo3D == STEREO_OFF || !m_renderDevice->SupportLayeredRendering() ? SurfaceType::RT_DEFAULT : SurfaceType::RT_STEREO;

   view->SetupTargets(renderWidthAA, renderHeightAA, rtType, renderFormat, nMSAASamples);

   // Initialize shaders
   m_renderDevice->m_basicShader->SetVector(
      ShaderUniform::w_h_height, (float)(1.0 / (double)view->GetMSAABackBufferTexture()->GetWidth()), (float)(1.0 / (double)view->GetMSAABackBufferTexture()->GetHeight()), 0.0f, 0.0f);
   m_renderDevice->m_ballShader->SetVector(ShaderUniform::w_h_disableLighting,
      1.5f / (float)view->GetPreviousBackBufferTexture()->GetWidth(), // UV Offset for sampling reflections
      1.5f / (float)view->GetPreviousBackBufferTexture()->GetHeight(), 0.f, 0.f);
   DisableBallLighting(m_table->GetSettings().GetPlayer_DisableLightingForBalls());

   // These three assets are shipped with the application, so failing to load one means a broken install rather than
   // a table problem. So no fall back, rather fail/crash on startup
   std::shared_ptr<BaseTexture> ballTex = std::shared_ptr<BaseTexture>(BaseTexture::CreateFromFile(g_app->m_fileLocator.GetAppPath(FileLocator::AppSubFolder::Assets, "BallEnv.exr")));
#ifdef ENABLE_BGFX
   // WebGPU can only filter 32 bit float textures with the optional 'float32-filterable' feature, so clamp
   // extreme HDR environments to FP16 to keep envmap filtering working everywhere
   if (bgfx::getRendererType() == bgfx::RendererType::Enum::WebGPU)
   {
      if (ballTex->m_format == BaseTexture::RGB_FP32)
         ballTex = ballTex->Convert(BaseTexture::RGB_FP16);
      else if (ballTex->m_format == BaseTexture::RGBA_FP32)
         ballTex = ballTex->Convert(BaseTexture::RGBA_FP16);
   }
#endif
   m_ballEnvSampler = std::make_shared<Sampler>(m_renderDevice, "Ball Env"s, /*m_renderDevice->OrFallback(*/ ballTex /*)*/, false);
   ballTex = nullptr;

   std::shared_ptr<BaseTexture> aoTex = std::shared_ptr<BaseTexture>(BaseTexture::CreateFromFile(g_app->m_fileLocator.GetAppPath(FileLocator::AppSubFolder::Assets, "AODither.webp")));
   m_aoDitherSampler = std::make_shared<Sampler>(m_renderDevice, "AO Dither"s, /*m_renderDevice->OrFallback(*/ aoTex /*)*/, true);
   aoTex = nullptr;

   Texture* tableEnv = m_table->GetImage(m_table->m_envImage);
   std::shared_ptr<const BaseTexture> envTex = /*m_renderDevice->OrFallback(*/
      tableEnv ? tableEnv->GetRawBitmap(false, 0)
               : std::shared_ptr<BaseTexture>(BaseTexture::CreateFromFile(g_app->m_fileLocator.GetAppPath(FileLocator::AppSubFolder::Assets, "EnvMap.webp"))) /*)*/;
#ifdef ENABLE_BGFX
   // WebGPU can only filter 32 bit float textures with the optional 'float32-filterable' feature, so clamp
   // extreme HDR environments to FP16 to keep envmap filtering working everywhere
   if (bgfx::getRendererType() == bgfx::RendererType::Enum::WebGPU)
   {
      if (envTex->m_format == BaseTexture::RGB_FP32)
         envTex = envTex->Convert(BaseTexture::RGB_FP16);
      else if (envTex->m_format == BaseTexture::RGBA_FP32)
         envTex = envTex->Convert(BaseTexture::RGBA_FP16);
   }
#endif
   PLOGI << "Computing environment map mipmaps and irradiance"; // For profiling
   const unsigned int irradianceHeight = max(1u, min(envTex->height(), 256u) / 8);
   EnvmapPrecalcResult envPrecalc = EnvmapPrecalc(envTex, 2 * irradianceHeight, irradianceHeight);
   if (!envPrecalc.glossyMips.empty())
      m_envSampler = std::make_shared<Sampler>(m_renderDevice, "Table Env"s, envPrecalc.glossyMips);
   else // Not readable on the CPU (not expected): GPU mipmaps (2x2 averages, so the glossy lookup is sharper than intended), no diffuse lighting
   {
      PLOGE << "Failed to compute the environment map mipmaps and irradiance";
      m_envSampler = std::make_shared<Sampler>(m_renderDevice, "Table Env"s, envTex, false);
      envPrecalc.irradiance = BaseTexture::Create(1, 1, BaseTexture::SRGBA);
      if (envPrecalc.irradiance)
         memset(envPrecalc.irradiance->data(), 0, 4);
   }
   m_envRadianceTexture = std::move(envPrecalc.irradiance);
   envPrecalc.glossyMips.clear();
   m_renderDevice->m_texMan.SetDirty(m_envRadianceTexture.get());
   m_renderDevice->m_basicShader->SetTexture(ShaderUniform::tex_diffuse_env, m_envRadianceTexture.get());
   m_renderDevice->m_ballShader->SetTexture(ShaderUniform::tex_diffuse_env, m_envRadianceTexture.get());
   envTex.reset();
   PLOGI << "Environment map mipmaps and irradiance computed"; // For profiling

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

   m_renderDevice->ResetRenderState();
#if defined(ENABLE_DX9)
   D3DVIEWPORT9 viewPort;
   int renderW, renderH;
   view->GetRenderSize(renderW, renderH);
   viewPort.X = 0;
   viewPort.Y = 0;
   viewPort.Width = renderW;
   viewPort.Height = renderH;
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

void Renderer::ApplyTableSettings()
{
   m_stereo3Denabled = true; // m_table->GetSettings().GetPlayer_Stereo3DEnabled();
   Settings::SetTableOverride_ToneMapper_Default(m_table->GetToneMapper());
   m_toneMapper = GetValidToneMapper(m_table->GetSettings().GetTableOverride_ToneMapper());
   m_HDRforceDisableToneMapper = m_table->GetSettings().GetPlayer_HDRDisableToneMapper();
   Settings::SetTableOverride_Exposure_Default(m_table->GetExposure());
   m_exposure = m_table->GetSettings().GetTableOverride_Exposure();
   m_dynamicAO = m_table->GetSettings().GetPlayer_DynamicAO();
   m_disableAO = m_table->GetSettings().GetPlayer_DisableAO();
   for (int wnd = VPXWindowId::VPXWINDOW_Backglass; wnd <= VPXWindowId::VPXWINDOW_Topper; wnd++)
      m_ancillaryWndRotation[wnd] = 90 * clamp(m_table->GetSettings().GetWindow_Rotation(wnd), 0, 3); // Setting is an index in the 0 / 90 / 180 / 270 literals
   m_vrDesktop = static_cast<VRDesktopMode>(m_table->GetSettings().GetPlayerVR_DesktopDisplay());
   m_vrDesktopShrink = m_table->GetSettings().GetPlayerVR_ShrinkDesktopDisplay();
   m_FXAA = m_table->GetSettings().GetPlayer_FXAA();
   m_sharpen = m_table->GetSettings().GetPlayer_Sharpen();
   m_ss_refl = m_table->GetSettings().GetPlayer_SSRefl();
   m_bloomOff = m_table->GetSettings().GetPlayer_ForceBloomOff();
   m_motionBlurOff = m_table->GetSettings().GetPlayer_ForceMotionBlurOff();
   m_maxReflectionMode = (RenderProbe::ReflectionMode)m_table->GetSettings().GetPlayer_PFReflection();
   m_trailForBalls = m_table->GetSettings().GetPlayer_BallTrail();
   m_ballTrailStrength = m_table->GetSettings().GetPlayer_BallTrailStrength();
   m_ballAntiStretch = m_table->GetSettings().GetPlayer_BallAntiStretch();
   m_ballImage = nullptr;
   m_decalImage = nullptr;
   m_overwriteBallImages = m_table->GetSettings().GetPlayer_OverwriteBallImage();
   m_ballImagePath = m_table->GetSettings().GetPlayer_BallImage();
   m_decalImagePath = m_table->GetSettings().GetPlayer_DecalImage();
   if (m_overwriteBallImages)
   {
      m_ballImage = BaseTexture::CreateFromFile(m_ballImagePath, m_table->GetSettings().GetPlayer_MaxTexDimension());
      m_decalImage = BaseTexture::CreateFromFile(m_decalImagePath, m_table->GetSettings().GetPlayer_MaxTexDimension());
   }
   m_vrApplyColorKey = m_stereo3D == STEREO_VR && m_table->GetSettings().GetPlayerVR_UsePassthroughColor();
   m_visualNudgeStrength = m_table->GetSettings().GetPlayer_NudgeStrength();

   // HDR2020 output disables bloom and boosts exposure
   if (m_renderDevice && (g_pplayer->m_vrDevice == nullptr) && m_renderDevice->m_outputWnd[0]->IsWCGBackBuffer())
   {
      m_exposure *= g_settingsService.GetAppSettings().GetPlayer_HDRGlobalExposure();
      m_bloomOff = true;
   }

   // Cache DMD renderer properties
   for (int profile = 0; profile < (int)std::size(m_dmdUseLegacyRenderer); profile++)
   {
      m_dmdUseLegacyRenderer[profile] = m_table->GetSettings().GetDMD_ProfileLegacy(profile);
#if !defined(ENABLE_BGFX)
      m_dmdUseLegacyRenderer[profile] = false; // Only available for BGFX
#endif
      m_dmdDotColor[profile] = convertColor(m_table->GetSettings().GetDMD_ProfileDotTint(profile), m_table->GetSettings().GetDMD_ProfileDotBrightness(profile));
      m_dmdDotProperties[profile].x = m_table->GetSettings().GetDMD_ProfileDotSize(profile);
      m_dmdDotProperties[profile].y = m_table->GetSettings().GetDMD_ProfileDotSharpness(profile);
      m_dmdDotProperties[profile].w = m_table->GetSettings().GetDMD_ProfileDiffuseGlow(profile);
      m_dmdUnlitDotColor[profile] = convertColor(m_table->GetSettings().GetDMD_ProfileUnlitDotColor(profile), 1.f);
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
      m_segColor[profile] = convertColor(m_table->GetSettings().GetAlpha_ProfileColor(profile), m_table->GetSettings().GetAlpha_ProfileBrightness(profile));
      m_segUnlitColor[profile] = convertColor(m_table->GetSettings().GetAlpha_ProfileUnlit(profile), m_table->GetSettings().GetAlpha_ProfileDiffuseGlow(profile));
      // Convert color as settings are sRGB color while shader needs linear RGB color
      m_segColor[profile].x = InvsRGB(m_segColor[profile].x);
      m_segColor[profile].y = InvsRGB(m_segColor[profile].y);
      m_segColor[profile].z = InvsRGB(m_segColor[profile].z);
      m_segUnlitColor[profile].x = InvsRGB(m_segUnlitColor[profile].x);
      m_segUnlitColor[profile].y = InvsRGB(m_segUnlitColor[profile].y);
      m_segUnlitColor[profile].z = InvsRGB(m_segUnlitColor[profile].z);
   }
}

void Renderer::SetTable(PinTable* const table)
{
   assert(table != nullptr);
   m_table = table;
   m_sceneLighting.SetTable(table);

   // Drop renderable initialization requests targeting the previous table
   m_renderableToInit.clear();
   m_renderableToSetup.clear();

   // Re-evaluate the cached table settings (they may have been edited during the previous session)
   ApplyTableSettings();
   DisableBallLighting(m_table->GetSettings().GetPlayer_DisableLightingForBalls());

   // Static prerendering must be fully re-evaluated for the new table
   for (RenderView* v : m_renderViews)
      v->ResetStaticPrepass();

   // The environment map sampler and its precomputed radiance are kept: base tables and their
   // live copies share the same table images (and therefore the same environment map). Rebinding
   // them will be needed if table switching is extended to unrelated tables.
}

Renderer::~Renderer()
{
   m_gpu_profiler.Shutdown();
   m_ballMeshBuffer = nullptr;
#ifdef DEBUG_BALL_SPIN
   m_ballDebugPoints = nullptr;
#endif
   m_ballTrailMeshBuffer = nullptr;
   delete m_tonemapLUT;
   for (int window = 0; window <= VPXWindowId::VPXWINDOW_Topper; window++)
      m_ancillaryWndHdrRT[window] = nullptr;
   m_envRadianceTexture.reset();
   m_ballEnvSampler = nullptr;
   m_envSampler = nullptr;
   m_aoDitherSampler = nullptr;
   // Views own render targets: they must be released while the render device is still alive
   m_vrMirrorView = nullptr;
   m_primaryView = nullptr;
   m_extraViews.clear();
   m_deadViews.clear();
   m_renderViews.clear();
   m_activeView = nullptr;
   delete m_renderDevice;
}

Renderer::SceneLighting::SceneLighting(PinTable* const table)
   : m_table(table)
{
   m_mode = m_table->GetSettings().GetPlayer_OverrideTableEmissionScale() ? m_table->GetSettings().GetPlayer_DynamicDayNight() ? Mode::DayNight : Mode::User : Mode::Table;
   m_latitude = m_table->GetSettings().GetPlayer_Latitude();
   m_longitude = m_table->GetSettings().GetPlayer_Longitude();
   m_userLightLevel = m_table->GetSettings().GetPlayer_EmissionScale();
   Update();
}

void Renderer::SceneLighting::Update()
{
#ifndef ENABLE_BGFX
   if (g_app->m_bgles) // Overriden from command line
   {
      m_emissionScale = g_app->m_fgles;
      return;
   }
#endif
   switch (m_mode)
   {
   case Mode::Table: m_emissionScale = m_table->m_globalEmissionScale; break;

   case Mode::User: m_emissionScale = m_userLightLevel; break;

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

      const float factor = (float)(sin(M_PI * saturate((cur - srise) / (sset - srise))) //!! leave space before sunrise and after sunset?
         * sqrt(tr / max_tr)); //!! magic, "emulates" that shorter days are usually also "darker",cloudier,whatever in most regions

      m_emissionScale = clamp(factor, 0.15f, 1.f); //!! configurable clamp?
      break;
   }
   }
}

bool Renderer::UseAnisoFiltering() const { return Shader::GetDefaultSamplerFilter(ShaderUniform::tex_base_color) == SamplerFilter::SF_ANISOTROPIC; }

void Renderer::SetAnisoFiltering(bool enable)
{
   Shader::SetDefaultSamplerFilter(ShaderUniform::tex_sprite, enable ? SamplerFilter::SF_ANISOTROPIC : SamplerFilter::SF_TRILINEAR);
   Shader::SetDefaultSamplerFilter(ShaderUniform::tex_base_color, enable ? SamplerFilter::SF_ANISOTROPIC : SamplerFilter::SF_TRILINEAR);
   Shader::SetDefaultSamplerFilter(ShaderUniform::tex_base_normalmap, enable ? SamplerFilter::SF_ANISOTROPIC : SamplerFilter::SF_TRILINEAR);
   Shader::SetDefaultSamplerFilter(ShaderUniform::tex_flasher_A, enable ? SamplerFilter::SF_ANISOTROPIC : SamplerFilter::SF_TRILINEAR);
   Shader::SetDefaultSamplerFilter(ShaderUniform::tex_flasher_B, enable ? SamplerFilter::SF_ANISOTROPIC : SamplerFilter::SF_TRILINEAR);
}

bool Renderer::IsBallLightingDisabled() const { return m_renderDevice->m_ballShader->GetVector(ShaderUniform::w_h_disableLighting).z != 0.f; }

void Renderer::DisableBallLighting(bool disableLightingForBalls)
{
   vec4 prev = m_renderDevice->m_ballShader->GetVector(ShaderUniform::w_h_disableLighting);
   m_renderDevice->m_ballShader->SetVector(ShaderUniform::w_h_disableLighting, prev.x, prev.y, disableLightingForBalls ? 1.f : 0.f, prev.w);
}

namespace
{
// One level of the environment map mip chain while it is computed: linear RGB floats, equirectangular (texel (x, y) of a w x h
// level looks in the direction of longitude (x + 0.5) / w * 2 PI + PI and colatitude (y + 0.5) / h * PI, see ray_to_equirectangular_uv)
struct EnvLevel
{
   unsigned int w = 0, h = 0;
   vector<float> rgb;
};

// Decodes row y of an image to linear RGB floats
void DecodeEnvRow(const BaseTexture& tex, const unsigned int y, float* const __restrict out)
{
   const unsigned int w = tex.width();
   const uint8_t* const __restrict row = static_cast<const uint8_t*>(tex.datac()) + static_cast<size_t>(y) * tex.pitch();
   const uint16_t* const __restrict row16 = reinterpret_cast<const uint16_t*>(row);
   const float* const __restrict row32 = reinterpret_cast<const float*>(row);
   const unsigned int stride = (tex.m_format == BaseTexture::RGBA || tex.m_format == BaseTexture::SRGBA || tex.m_format == BaseTexture::RGBA_FP16 || tex.m_format == BaseTexture::RGBA_FP32) ? 4 : 3;
   for (unsigned int x = 0; x < w; ++x)
   {
      float* const __restrict o = out + x * 3;
      switch (tex.m_format)
      {
      case BaseTexture::BW: o[0] = o[1] = o[2] = static_cast<float>(row[x]) * (float)(1.0 / 255.0); break;
      case BaseTexture::BW_FP32: o[0] = o[1] = o[2] = row32[x]; break;
      case BaseTexture::RGB:
      case BaseTexture::RGBA:
         for (unsigned int c = 0; c < 3; ++c)
            o[c] = static_cast<float>(row[x * stride + c]) * (float)(1.0 / 255.0);
         break;
      case BaseTexture::SRGB:
      case BaseTexture::SRGBA:
         for (unsigned int c = 0; c < 3; ++c)
            o[c] = InvsRGB8(row[x * stride + c]);
         break;
      case BaseTexture::SRGB565:
      {
         const unsigned int v = row16[x], r = v >> 11, g = (v >> 5) & 63u, b = v & 31u;
         o[0] = InvsRGB8(static_cast<uint8_t>((r << 3) | (r >> 2)));
         o[1] = InvsRGB8(static_cast<uint8_t>((g << 2) | (g >> 4)));
         o[2] = InvsRGB8(static_cast<uint8_t>((b << 3) | (b >> 2)));
         break;
      }
      case BaseTexture::RGB_FP16:
      case BaseTexture::RGBA_FP16:
         for (unsigned int c = 0; c < 3; ++c)
            o[c] = half2float(row16[x * stride + c]);
         break;
      case BaseTexture::RGB_FP32:
      case BaseTexture::RGBA_FP32:
         for (unsigned int c = 0; c < 3; ++c)
            o[c] = row32[x * stride + c];
         break;
      default: assert(!"unknown format"); o[0] = o[1] = o[2] = 0.f; break;
      }
   }
}

// Encodes a row of linear RGB floats to RGBA, SRGBA, RGBA_FP16 or RGBA_FP32 (opaque)
void EncodeEnvRow(const float* const __restrict in, const unsigned int w, const BaseTexture::Format format, uint8_t* const __restrict out)
{
   for (unsigned int x = 0; x < w; ++x)
   {
      const float* const __restrict i = in + x * 3;
      switch (format)
      {
      case BaseTexture::RGBA:
         for (unsigned int c = 0; c < 3; ++c)
            out[x * 4 + c] = static_cast<uint8_t>(clamp(i[c] * 255.f + 0.5f, 0.f, 255.f));
         out[x * 4 + 3] = 255;
         break;
      case BaseTexture::SRGBA:
         for (unsigned int c = 0; c < 3; ++c)
            out[x * 4 + c] = sRGB8(clamp(i[c], 0.f, 1.f));
         out[x * 4 + 3] = 255;
         break;
      case BaseTexture::RGBA_FP16:
         for (unsigned int c = 0; c < 3; ++c)
            reinterpret_cast<uint16_t*>(out)[x * 4 + c] = float2half_noLUT(i[c]);
         reinterpret_cast<uint16_t*>(out)[x * 4 + 3] = float2half_noLUT(1.f);
         break;
      case BaseTexture::RGBA_FP32:
         for (unsigned int c = 0; c < 3; ++c)
            reinterpret_cast<float*>(out)[x * 4 + c] = i[c];
         reinterpret_cast<float*>(out)[x * 4 + 3] = 1.f;
         break;
      default: assert(!"unsupported format"); break;
      }
   }
}

std::shared_ptr<BaseTexture> EncodeEnvLevel(const EnvLevel& level, const BaseTexture::Format format)
{
   std::shared_ptr<BaseTexture> tex = BaseTexture::Create(level.w, level.h, format);
   if (tex == nullptr)
      return nullptr;
   for (unsigned int y = 0; y < level.h; ++y)
      EncodeEnvRow(level.rgb.data() + static_cast<size_t>(y) * level.w * 3, level.w, format, static_cast<uint8_t*>(tex->data()) + static_cast<size_t>(y) * tex->pitch());
   return tex;
}

// 2x2 average (size rounded down, at least 1, as the GPU mip chains)
EnvLevel DownsampleEnvLevel(const EnvLevel& src)
{
   EnvLevel dst;
   dst.w = max(1u, src.w / 2);
   dst.h = max(1u, src.h / 2);
   dst.rgb.resize(static_cast<size_t>(dst.w) * dst.h * 3);
   for (unsigned int y = 0; y < dst.h; ++y)
   {
      const float* const r0 = src.rgb.data() + static_cast<size_t>(min(2 * y, src.h - 1)) * src.w * 3;
      const float* const r1 = src.rgb.data() + static_cast<size_t>(min(2 * y + 1, src.h - 1)) * src.w * 3;
      for (unsigned int x = 0; x < dst.w; ++x)
      {
         const unsigned int x0 = min(2 * x, src.w - 1) * 3, x1 = min(2 * x + 1, src.w - 1) * 3;
         for (unsigned int c = 0; c < 3; ++c)
            dst.rgb[(static_cast<size_t>(y) * dst.w + x) * 3 + c] = 0.25f * ((r0[x0 + c] + r0[x1 + c]) + (r1[x0 + c] + r1[x1 + c]));
      }
   }
   return dst;
}

// Integrates src over a Phong lobe of the given power, cos(angle to the lobe axis)^power, around the direction of each texel of dst.
// The integral is over directions (texels weighted by their solid angle), so the result is right over the whole sphere, poles included.
void PrefilterEnvLevel(const EnvLevel& src, EnvLevel& dst, const float power, progschj::ThreadPool& pool)
{
   vector<float> rowSin(src.h), rowCos(src.h), colSin(src.w), colCos(src.w);
   for (unsigned int j = 0; j < src.h; ++j)
   {
      const float theta = ((float)j + 0.5f) / (float)src.h * (float)M_PI;
      rowSin[j] = sinf(theta);
      rowCos[j] = cosf(theta);
   }
   for (unsigned int i = 0; i < src.w; ++i)
   {
      const float phi = ((float)i + 0.5f) / (float)src.w * (float)(2.0 * M_PI) + (float)M_PI;
      colSin[i] = sinf(phi);
      colCos[i] = cosf(phi);
   }
   // Directions whose weight is below 1e-3 are skipped: they are further than alpha from the lobe axis
   const float cosAlpha = expf(logf(1e-3f) / power);
   const float alpha = acosf(cosAlpha);
   for (unsigned int y = 0; y < dst.h; ++y)
   {
      pool.enqueue_block(
         [&src, &dst, &rowSin, &rowCos, &colSin, &colCos, power, cosAlpha, alpha, y]
         {
            set_denormals_flush_to_zero_once();
            const float theta = ((float)y + 0.5f) / (float)dst.h * (float)M_PI;
            const float sT = sinf(theta), cT = cosf(theta);
            const int j0 = max(0, static_cast<int>(floorf((theta - alpha) * ((float)src.h * (float)(1.0/M_PI)) - 0.5f)));
            const int j1 = min(static_cast<int>(src.h) - 1, static_cast<int>(ceilf((theta + alpha) * ((float)src.h * (float)(1.0/M_PI)) - 0.5f)));
            for (unsigned int x = 0; x < dst.w; ++x)
            {
               const float phi = ((float)x + 0.5f) / (float)dst.w * (float)(2.0 * M_PI) + (float)M_PI;
               const float rx = sT * cosf(phi), ry = sT * sinf(phi), rz = cT;
               const float center = ((float)x + 0.5f) * (float)src.w / (float)dst.w - 0.5f; // Source column of the lobe axis
               float sum_r = 0.f, sum_g = 0.f, sum_b = 0.f, sum_w = 0.f;
               for (int j = j0; j <= j1; ++j)
               {
                  const float sj = rowSin[j], cj = rowCos[j];
                  // Columns of row j within alpha of the axis: cos(longitude difference) >= (cos(alpha) - cj cT) / (sj sT)
                  int i0 = 0, i1 = static_cast<int>(src.w) - 1;
                  const float den = sj * sT;
                  if (den > 1e-6f)
                  {
                     const float c = (cosAlpha - cj * cT) / den;
                     if (c >= 1.f)
                        continue;
                     if (c > -1.f)
                     {
                        const float half = acosf(c) * ((float)src.w * (float)(1.0/(2.0 * M_PI)));
                        const int a = static_cast<int>(ceilf(center - half)), b = static_cast<int>(floorf(center + half));
                        if (b - a + 1 < static_cast<int>(src.w))
                        {
                           i0 = a;
                           i1 = b;
                        }
                     }
                  }
                  else if (cj * cT <= cosAlpha)
                     continue;
                  const float* const srcRow = src.rgb.data() + static_cast<size_t>(j) * src.w * 3;
                  for (int i = i0; i <= i1; ++i)
                  {
                     const int iw = i < 0 ? i + static_cast<int>(src.w) : i >= static_cast<int>(src.w) ? i - static_cast<int>(src.w) : i;
                     const float d = sj * (colCos[iw] * rx + colSin[iw] * ry) + cj * rz;
                     if (d <= cosAlpha)
                        continue;
                     const float w = (power == 1.f ? d : exp2f(power * log2f(d))) * sj; // Lobe weight times solid angle of the texel
                     sum_r += srcRow[iw * 3 + 0] * w;
                     sum_g += srcRow[iw * 3 + 1] * w;
                     sum_b += srcRow[iw * 3 + 2] * w;
                     sum_w += w;
                  }
               }
               float* const o = dst.rgb.data() + (static_cast<size_t>(y) * dst.w + x) * 3;
               if (sum_w > 0.f)
               {
                  const float inv = 1.f / sum_w;
                  o[0] = sum_r * inv;
                  o[1] = sum_g * inv;
                  o[2] = sum_b * inv;
               }
               else // No source texel within the lobe (not expected, the lobes span more than a source texel): nearest one
               {
                  const unsigned int j = min(static_cast<unsigned int>(theta * ((float)src.h * (float)(1.0/M_PI))), src.h - 1);
                  const unsigned int i = static_cast<unsigned int>(center + 0.5f) % src.w;
                  memcpy(o, src.rgb.data() + (static_cast<size_t>(j) * src.w + i) * 3, 3 * sizeof(float));
               }
            }
         });
   }
   pool.wait_until_empty();
   pool.wait_until_nothing_in_flight();
}
}

// The glossy lookup (DoEnvmapGlossy) reads the Blinn-Phong power n at the level of height h = 2 sqrt((n+1)/3), one level finer than
// the box filtered chain it formerly read, so the stored lobes are sampled finely enough for the bilinear lookup. Level k, of height
// H/2^k, thus stores the lobe of power n = 0.75 (H/2^k)^2 - 1, approximated as a Phong lobe of power n/4 around the reflection vector.
// Only the levels the lookup can reach are prefiltered (n is at most 2048, read at height ~52), the finer ones are 2x2 averages.
// The diffuse irradiance is the same convolution with the cosine lobe (power 1, the hemisphere around the normal), from the 2x2
// averaged level of height 32 (or the irradiance height if larger): finer details do not show through such a wide lobe.
Renderer::EnvmapPrecalcResult Renderer::EnvmapPrecalc(const std::shared_ptr<const BaseTexture>& envTex, const unsigned int irradianceWidth, const unsigned int irradianceHeight)
{
   if (envTex == nullptr || envTex->datac() == nullptr || envTex->width() == 0 || envTex->height() == 0 || irradianceWidth == 0 || irradianceHeight == 0)
      return {};
   const BaseTexture::Format srcFormat = envTex->m_format;
   const BaseTexture::Format format = (srcFormat == BaseTexture::RGB_FP32 || srcFormat == BaseTexture::RGBA_FP32 || srcFormat == BaseTexture::BW_FP32) ? BaseTexture::RGBA_FP32
      : (srcFormat == BaseTexture::RGB_FP16 || srcFormat == BaseTexture::RGBA_FP16)                                                                   ? BaseTexture::RGBA_FP16
      : BaseTexture::IsLinearFormat(srcFormat)                                                                                                       ? BaseTexture::RGBA
                                                                                                                                                      : BaseTexture::SRGBA;
   const unsigned int W = envTex->width(), H = envTex->height();
   const unsigned int numLevels = static_cast<unsigned int>(std::bit_width(max(W, H)));
   constexpr unsigned int maxPrefilteredHeight = 128;
   // Power of the Phong lobe stored at level k (not below the one of VPX roughness 0: the lookup does not read the levels coarser than height 2)
   const auto levelPower = [H](const unsigned int k)
   {
      const float h = (float)H / (float)(1u << k);
      return max(0.75f * h * h - 1.f, 2.f) * 0.25f;
   };
   // 2x2 averaged level the irradiance is computed from (level heights are H >> k, at least 1)
   unsigned int irradianceSource = 0;
   while (irradianceSource + 1 < numLevels && max(1u, H >> irradianceSource) > max(32u, irradianceHeight))
      ++irradianceSource;

   EnvmapPrecalcResult result;
   result.glossyMips.reserve(numLevels);
   vector<EnvLevel> levels(numLevels); // Linear 2x2 averaged levels (level 0 only decoded if level 1 or the irradiance is computed from it)

   // Level 0 is the image itself, used as is if it already has the 4 channel format of the chain (its alpha is not read), else
   // converted to it. It is also averaged into level 1.
   std::shared_ptr<BaseTexture> level0;
   uint8_t* level0Data = nullptr;
   if (srcFormat != format)
   {
      level0 = BaseTexture::Create(W, H, format);
      if (level0 == nullptr)
         return {};
      level0Data = static_cast<uint8_t*>(level0->data());
   }
   const auto encodeLevel0Row = [&](const float* const row, const unsigned int y)
   {
      if (level0Data)
         EncodeEnvRow(row, W, format, level0Data + static_cast<size_t>(y) * level0->pitch());
   };
   if (max(1u, H / 2) <= maxPrefilteredHeight || irradianceSource == 0 || numLevels == 1) // Level 0 is a source (of level 1 if prefiltered: H / 2, as odd heights like 257 round down)
   {
      levels[0].w = W;
      levels[0].h = H;
      levels[0].rgb.resize(static_cast<size_t>(W) * H * 3);
      for (unsigned int y = 0; y < H; ++y)
      {
         float* const row = levels[0].rgb.data() + static_cast<size_t>(y) * W * 3;
         DecodeEnvRow(*envTex, y, row);
         encodeLevel0Row(row, y);
      }
      if (numLevels > 1)
         levels[1] = DownsampleEnvLevel(levels[0]);
   }
   else // Large image: decoded 2 rows at a time
   {
      EnvLevel& l1 = levels[1];
      l1.w = max(1u, W / 2);
      l1.h = max(1u, H / 2);
      l1.rgb.resize(static_cast<size_t>(l1.w) * l1.h * 3);
      vector<float> r0(static_cast<size_t>(W) * 3), r1(static_cast<size_t>(W) * 3);
      for (unsigned int y = 0; y < H; y += 2)
      {
         DecodeEnvRow(*envTex, y, r0.data());
         encodeLevel0Row(r0.data(), y);
         if (y + 1 < H)
         {
            DecodeEnvRow(*envTex, y + 1, r1.data());
            encodeLevel0Row(r1.data(), y + 1);
         }
         if (y / 2 >= l1.h) // Last row of an odd height, dropped by the rounded down size
            continue;
         const float* const rb = y + 1 < H ? r1.data() : r0.data();
         for (unsigned int x = 0; x < l1.w; ++x)
         {
            const unsigned int x0 = min(2 * x, W - 1) * 3, x1 = min(2 * x + 1, W - 1) * 3;
            for (unsigned int c = 0; c < 3; ++c)
               l1.rgb[(static_cast<size_t>(y / 2) * l1.w + x) * 3 + c] = 0.25f * (r0[x0 + c] + r0[x1 + c] + rb[x0 + c] + rb[x1 + c]);
         }
      }
   }
   if (level0)
      result.glossyMips.push_back(std::move(level0));
   else
      result.glossyMips.push_back(envTex);
   for (unsigned int k = 2; k < numLevels; ++k)
      levels[k] = DownsampleEnvLevel(levels[k - 1]);

   progschj::ThreadPool pool(g_app->GetLogicalNumberOfProcessors());
   for (unsigned int k = 1; k < numLevels; ++k)
   {
      std::shared_ptr<const BaseTexture> mip;
      if (levels[k].h <= maxPrefilteredHeight)
      {
         EnvLevel prefiltered;
         prefiltered.w = levels[k].w;
         prefiltered.h = levels[k].h;
         prefiltered.rgb.resize(levels[k].rgb.size());
         PrefilterEnvLevel(levels[k - 1], prefiltered, levelPower(k), pool);
         mip = EncodeEnvLevel(prefiltered, format);
      }
      else
         mip = EncodeEnvLevel(levels[k], format);
      if (mip == nullptr)
         return {};
      result.glossyMips.push_back(std::move(mip));
   }

   // Diffuse irradiance: the cosine weighted average of the envmap over the hemisphere (used as is by the shaders), sRGB for LDR envmaps
   EnvLevel irradiance;
   irradiance.w = irradianceWidth;
   irradiance.h = irradianceHeight;
   irradiance.rgb.resize(static_cast<size_t>(irradianceWidth) * irradianceHeight * 3);
   PrefilterEnvLevel(levels[irradianceSource], irradiance, 1.f, pool);
   result.irradiance = EncodeEnvLevel(irradiance, format == BaseTexture::RGBA ? BaseTexture::SRGBA : format);
   if (result.irradiance == nullptr)
      return {};
   return result;
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
void Renderer::SetupShaders()
{
   if (!m_shaderDirty)
      return;
   m_shaderDirty = false;

   const vec4 envEmissionScale_TexWidth(
      m_table->m_envEmissionScale * m_sceneLighting.GetGlobalEmissionScale(), static_cast<float>(m_envSampler->GetHeight()) /*+m_envSampler.m_width)*0.5f*/, 0.f, 0.f); //!! dto.

   UpdateBasicShaderMatrix();
   m_renderDevice->m_basicShader->SetTexture(ShaderUniform::tex_env, m_envSampler);
   m_renderDevice->m_basicShader->SetVector(ShaderUniform::fenvEmissionScale_TexWidth, &envEmissionScale_TexWidth);

   UpdateBallShaderMatrix();
   m_renderDevice->m_ballShader->SetVector(ShaderUniform::fenvEmissionScale_TexWidth, &envEmissionScale_TexWidth);

   constexpr float Roughness = 0.8f;
   m_renderDevice->m_ballShader->SetVector(ShaderUniform::Roughness_WrapL_Edge_Thickness, exp2f(10.0f * Roughness + 1.0f), 0.f, 1.f, 0.05f);
   const vec4 amb_lr = convertColor(m_table->m_lightAmbient, m_table->m_lightRange);
   m_renderDevice->m_ballShader->SetVector(ShaderUniform::cAmbient_LightRange, amb_lr.x * m_sceneLighting.GetGlobalEmissionScale(), amb_lr.y * m_sceneLighting.GetGlobalEmissionScale(),
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
   float lightPos[MAX_LIGHT_SOURCES][4] = {};
   float lightEmission[MAX_LIGHT_SOURCES][4] = {};

   for (unsigned int i = 0; i < MAX_LIGHT_SOURCES; ++i)
   {
      memcpy(&lightPos[i], &m_table->m_Light[i].pos, sizeof(float) * 3);
      memcpy(&lightEmission[i], &emission, sizeof(float) * 3);
   }

   m_renderDevice->m_basicShader->SetFloat4v(ShaderUniform::basicLightPos, (vec4*)lightPos, MAX_LIGHT_SOURCES);
   m_renderDevice->m_basicShader->SetFloat4v(ShaderUniform::basicLightEmission, (vec4*)lightEmission, MAX_LIGHT_SOURCES);
#elif defined(ENABLE_DX9)
   struct CLight // See CLight in Material.fxh
   {
      float vPos[3];
      float fRadius; // 0: the table lights are points
      float vEmission[3];
      float fPad;
   };
   CLight l[MAX_LIGHT_SOURCES] = {};

   for (unsigned int i = 0; i < MAX_LIGHT_SOURCES; ++i)
   {
      memcpy(&l[i].vPos, &m_table->m_Light[i].pos, sizeof(float) * 3);
      memcpy(&l[i].vEmission, &emission, sizeof(float) * 3);
   }

   m_renderDevice->m_basicShader->SetFloat4v(ShaderUniform::basicPackedLights, (vec4*)l, sizeof(CLight) * MAX_LIGHT_SOURCES / (4 * sizeof(float)));
#endif
}

void Renderer::UpdateStereoShaderState()
{
   if (m_renderDevice->m_stereoShader == nullptr)
      return;

   m_stereo3DDefocus = 0.f;
   if (IsAnaglyphStereoMode(m_stereo3D))
   {
      Anaglyph anaglyph;
      anaglyph.LoadSetupFromRegistry(g_settingsService.GetActiveSettings(), clamp(m_stereo3D - STEREO_ANAGLYPH_1, 0, 9));
      anaglyph.SetupShader(m_renderDevice->m_stereoShader);
      // The defocus kernel size should depend on the render resolution but since this is a user tweak, this doesn't matter that much
      m_stereo3DDefocus = m_table->GetSettings().GetPlayer_Stereo3DDefocus();
      const float leftFilterLuminance = VPX::Colors::LuminanceFromLinearRGB(anaglyph.GetLeftEyeGlassFilter(true));
      const float rightFilterLuminance = VPX::Colors::LuminanceFromLinearRGB(anaglyph.GetRightEyeGlassFilter(true));
      // We defocus the channel which is the most darkened one (channel for which the luminance of the filter is the lowest)
      if (rightFilterLuminance < leftFilterLuminance)
         m_stereo3DDefocus = -m_stereo3DDefocus;
   }
   else
   {
      m_renderDevice->m_stereoShader->SetTechnique(m_stereo3D == STEREO_SBS ? ShaderTechnique::stereo_SBS
            : m_stereo3D == STEREO_TB                                       ? ShaderTechnique::stereo_TB
            : m_stereo3D == STEREO_INT                                      ? ShaderTechnique::stereo_Int
                                                                            : ShaderTechnique::stereo_Flipped_Int);
   }
}

// Loads a segment SDF atlas together with its pre-integrated coverage atlas '<name>-cov.webp' (generated offline
// alongside the SDF atlases by tools/alphaseg/extract-segs.py and expected to be in sync with them: the left
// half holds the sharp segment coverage, the right half the squared SDF for glass light dispersion). Box filtered
// mipmaps of the coverage atlas directly provide the emitted light integrated over each texel footprint, making
// the segment rendering independent of the display resolution (see fs_display.sc)
void Renderer::GetSegSDF(SegDisplayAtlas& atlas, const char* const name) const
{
   if (atlas.sdf != nullptr)
      return;
   const std::filesystem::path path = g_app->m_fileLocator.GetAppPath(FileLocator::AppSubFolder::Assets, name);
   atlas.sdf.reset(Texture::CreateFromFile(path, false));
   if (atlas.sdf == nullptr)
      return;
   atlas.cov.reset(Texture::CreateFromFile(path.parent_path() / (path.stem().string() + "-cov.webp"), false));
   if (atlas.cov != nullptr)
   {
      const std::shared_ptr<const BaseTexture> covTex = atlas.cov->GetRawBitmap(false, 0);
      if (covTex == nullptr || (covTex->m_format != BaseTexture::RGBA && covTex->m_format != BaseTexture::SRGBA) || atlas.cov->m_width != 2 * atlas.sdf->m_width)
         atlas.cov = nullptr; // Reject out of sync coverage atlases
   }
}

// World space position derivatives of a display quad along its u and v coordinates (tangent, bitangent)
static void ComputeDisplayTangents(const Vertex3D_NoTex2* const vertices, Vertex3Ds& tangent, Vertex3Ds& bitangent)
{
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
   tangent = (dv1 * duv2.y - dv2 * duv1.y) * r;
   bitangent = (dv2 * duv1.x - dv1 * duv2.x) * r;
}

int Renderer::GetDisplayUnmirrorAxis(const bool isBackdrop, const Vertex3D_NoTex2* const vertices, const bool unmirror) const
{
   if (!unmirror || !g_pplayer->m_tblMirrorEnabled || isBackdrop || (vertices == nullptr))
      return -1;
   // Mirror mode mirrors the table along its x axis: flip back along the display axis closest to it
   Vertex3Ds tangent, bitangent;
   ComputeDisplayTangents(vertices, tangent, bitangent);
   return fabsf(tangent.x) * bitangent.Length() >= fabsf(bitangent.x) * tangent.Length() ? 0 : 1;
}

void Renderer::SetupDisplayRenderer(const bool isBackdrop, const Vertex3D_NoTex2* vertices, const vec4& emitterPad, const vec3& glassTint, const float glassRougness,
   ITexManCacheable* const glassTex, const vec4& glassArea, const vec3& glassAmbient, const bool unmirror)
{
   m_renderDevice->m_DMDShader->SetVector(ShaderUniform::glassTint_Roughness, glassTint.x, glassTint.y, glassTint.z, // Glass tint
      glassRougness); // Glass roughness (high roughness leads to emitted light 'glowing' on glass)
   m_renderDevice->m_DMDShader->SetVector(ShaderUniform::w_h_height, glassAmbient.x * 2.f, glassAmbient.y * 2.f,
      glassAmbient.z * 2.f, // Glass ambient color (only used when there is a glass texture)
      glassTex != nullptr ? 1.f : 0.f); // Apply glass texture or just uniform glass shading
   if (glassTex)
      m_renderDevice->m_DMDShader->SetTexture(ShaderUniform::displayGlass, glassTex);
   float parallaxU = 0.f, parallaxV = 0.f;
   if (!isBackdrop && (vertices != nullptr))
   { // (fake) depth by applying some parallax mapping (i stereo based on left eye view)
      Vertex3Ds tangent, bitangent;
      ComputeDisplayTangents(vertices, tangent, bitangent);
      const Matrix3D& mv = GetMVP().GetModelView(0);
      tangent = mv.MultiplyVectorNoTranslate(tangent);
      bitangent = mv.MultiplyVectorNoTranslate(bitangent);
      const Vertex3Ds v1(vertices[1].x, vertices[1].y, vertices[1].z);
      const Vertex3Ds v2(vertices[3].x, vertices[3].y, vertices[3].z);
      Vertex3Ds eye = (v1 + v2) * 0.5f; // Assume a rectangle shape, use opposite corners to get its center
      eye = mv.MultiplyVectorNoPerspective(eye);
      eye.Normalize();
      const float tN = tangent.Length();
      const float btN = bitangent.Length();
      constexpr float depth = CMTOVPU(0.5f); // depth between glass and display
      parallaxU = (depth / tN) * tangent.Dot(eye) / tN;
      parallaxV = (depth / btN) * bitangent.Dot(eye) / btN;
   }
   m_renderDevice->m_DMDShader->SetVector(ShaderUniform::glassArea, &glassArea);
   // Display uv = uv * (1 + padA + padB) - padA (see vs_dmd.sc): padA' = -(1 + padA), padB' = -(1 + padB) give 1 - display uv,
   // flipping the content along with its parallax offset
   vec4 pad(emitterPad.x - parallaxU, emitterPad.z + parallaxU, emitterPad.y - parallaxV, emitterPad.w + parallaxV); // Left, right, top, bottom
   switch (GetDisplayUnmirrorAxis(isBackdrop, vertices, unmirror))
   {
   case 0:
      pad.x = -(1.f + pad.x);
      pad.y = -(1.f + pad.y);
      break;
   case 1:
      pad.z = -(1.f + pad.z);
      pad.w = -(1.f + pad.w);
      break;
   default: break;
   }
   m_renderDevice->m_DMDShader->SetVector(ShaderUniform::glassPad, &pad);
}

void Renderer::SetupSegmentRenderer(int profile, const bool isBackdrop, const vec3& color, const float brightness, const SegmentFamily family, const SegElementType type, const float* segs,
   const ColorSpace colorSpace, const Vertex3D_NoTex2* vertices, const vec4& emitterPad, const vec3& glassTint, const float glassRougness, ITexManCacheable* const glassTex,
   const vec4& glassArea, const vec3& glassAmbient, const bool unmirror)
{
   SetupDisplayRenderer(isBackdrop, vertices, emitterPad, glassTint, glassRougness, glassTex, glassArea, glassAmbient, unmirror);

   int segIndex = 0;
   switch (type)
   {
   case CTLPI_SEG_LAYOUT_7:
      segIndex = 0;
      GetSegSDF(m_segDisplay[family][0],
         (family == SegmentFamily::Gottlieb)   ? "7seg-gts.webp"
            : (family == SegmentFamily::Bally) ? "7seg-bally.webp"
            : (family == SegmentFamily::Atari) ? "7seg-atari.webp"
                                               : "7seg-williams.webp");
      break;
   case CTLPI_SEG_LAYOUT_7C:
      segIndex = 1;
      GetSegSDF(m_segDisplay[family][1], (family == SegmentFamily::Bally) ? "7seg-c-bally.webp" : (family == SegmentFamily::Atari) ? "7seg-c-atari.webp" : "7seg-c-williams.webp");
      break;
   // TODO I did not found any reference for a dot only 7 segments display, so we use the comma one which is likely wrong
   case CTLPI_SEG_LAYOUT_7D:
      segIndex = 2;
      GetSegSDF(m_segDisplay[family][2], "7seg-c-williams.webp");
      break;
   case CTLPI_SEG_LAYOUT_9:
      segIndex = 3;
      GetSegSDF(m_segDisplay[family][3], "9seg-gts.webp");
      break;
   case CTLPI_SEG_LAYOUT_9C:
      segIndex = 4;
      GetSegSDF(m_segDisplay[family][4], "9seg-c-gts.webp");
      break;
   case CTLPI_SEG_LAYOUT_14:
      segIndex = 5;
      GetSegSDF(m_segDisplay[family][5], "14seg-williams.webp");
      break;
   case CTLPI_SEG_LAYOUT_14D:
      segIndex = 6;
      GetSegSDF(m_segDisplay[family][6], "14seg-d-williams.webp");
      break;
   case CTLPI_SEG_LAYOUT_14DC:
      segIndex = 7;
      GetSegSDF(m_segDisplay[family][7], (family == SegmentFamily::Gottlieb) ? "14seg-dc-gts.webp" : "14seg-dc-williams.webp");
      break;
   case CTLPI_SEG_LAYOUT_16:
      segIndex = 8;
      GetSegSDF(m_segDisplay[family][8], "16seg.webp");
      break;
   }
   const SegDisplayAtlas& segAtlas = m_segDisplay[family][segIndex];
   if (segAtlas.sdf == nullptr)
      return;

   const float fullBrightness = brightness * m_segColor[profile].w;
   const vec4 segColor = vec4(color.x * m_segColor[profile].x, color.y * m_segColor[profile].y, color.z * m_segColor[profile].z, 0.f);
   m_renderDevice->m_DMDShader->SetVector(ShaderUniform::vColor_Intensity, segColor.x * fullBrightness, segColor.y * fullBrightness, segColor.z * fullBrightness, // Lit segment color
      m_segUnlitColor[profile].w); // Diffuse strength
   m_renderDevice->m_DMDShader->SetVector(ShaderUniform::staticColor_Alpha, m_segUnlitColor[profile].x, m_segUnlitColor[profile].y,
      m_segUnlitColor[profile].z, // Unlit segment color (ambient)
      static_cast<float>(colorSpace)); // Output colorspace (3D render is linear, backdrop is tonemapped but needs sRGB conversion, dedicated window is tonemapped sRGB)
   m_renderDevice->m_DMDShader->SetVector(ShaderUniform::displayProperties, 0.25f * static_cast<float>(segAtlas.sdf->m_width),
      static_cast<float>(segAtlas.sdf->m_height), // Size in texels of a SDF tile (the texture holds 4 tiles side by side)
      0.f, 0.f); // Unused
   m_renderDevice->m_DMDShader->SetFloat4v(ShaderUniform::alphaSegState, segs, 4);
   m_renderDevice->m_DMDShader->SetTexture(ShaderUniform::displayTex, segAtlas.sdf.get(), true, SamplerFilter::SF_TRILINEAR, SamplerAddressMode::SA_CLAMP, SamplerAddressMode::SA_CLAMP);
   m_renderDevice->m_DMDShader->SetTexture(ShaderUniform::displayCov,
      segAtlas.cov != nullptr ? segAtlas.cov.get() : segAtlas.sdf.get(), // Fallback to the raw SDF (downscaling loses some coverage)
      true, SamplerFilter::SF_ANISOTROPIC, SamplerAddressMode::SA_CLAMP, SamplerAddressMode::SA_CLAMP);
   m_renderDevice->m_DMDShader->SetTechnique(ShaderTechnique::display_Seg_world);
}

void Renderer::SetupDMDRender(int profile, const bool isBackdrop, const vec3& color, const float brightness, const std::shared_ptr<BaseTexture>& dmd, const float alpha,
   const float addBlendModulate, const ColorSpace colorSpace, const Vertex3D_NoTex2* vertices, const vec4& emitterPad, const vec3& glassTint, const float glassRougness,
   ITexManCacheable* const glassTex, const vec4& glassArea, const vec3& glassAmbient, const bool unmirror)
{
   // Legacy DMD renderer
   if (IsLegacyDMDRenderer(profile))
   {
      assert(addBlendModulate == 0.f); // The legacy renderer has no additive blend encoding, it outputs a plain alpha blended color
      // w: 0 = luminance, 1 = linear RGB, 2 = sRGB frame (decoded by the DX9 shader, as its effect sampler state can not depend on the texture)
      m_renderDevice->m_DMDShader->SetVector(ShaderUniform::vColor_Intensity, color.x * brightness, color.y * brightness, color.z * brightness,
         dmd->m_format == BaseTexture::BW_FP32           ? 0.f
            : BaseTexture::IsLinearFormat(dmd->m_format) ? 1.f
                                                         : 2.f);
      m_renderDevice->m_DMDShader->SetVector(ShaderUniform::vRes_Alpha_time, (float)dmd->width(), (float)dmd->height(), alpha, (float)(g_pplayer->m_overall_frames % 2048));
      // Legacy shader samples at glassArea.xy + uv * glassArea.zw: unmirror maps u (or v) to 1 - u (or 1 - v)
      const int unmirrorAxis = GetDisplayUnmirrorAxis(isBackdrop, vertices, unmirror);
      m_renderDevice->m_DMDShader->SetVector(
         ShaderUniform::glassArea, unmirrorAxis == 0 ? 1.f : 0.f, unmirrorAxis == 1 ? 1.f : 0.f, unmirrorAxis == 0 ? -1.f : 1.f, unmirrorAxis == 1 ? -1.f : 1.f);
      m_renderDevice->m_DMDShader->SetTexture(ShaderUniform::tex_dmd, dmd.get());
      m_renderDevice->m_DMDShader->SetTechnique(ShaderTechnique::basic_DMD_world);
   }
   // New DMD renderer
   else
   {
      SetupDisplayRenderer(isBackdrop, vertices, emitterPad, glassTint, glassRougness, glassTex, glassArea, glassAmbient, unmirror);

      const float fullBrightness = brightness * m_dmdDotColor[profile].w;
      const vec4 dotColor = dmd->m_format == BaseTexture::BW_FP32 ? vec4(color.x * m_dmdDotColor[profile].x, color.y * m_dmdDotColor[profile].y, color.z * m_dmdDotColor[profile].z, 0.f)
                                                                  : vec4(color.x, color.y, color.z, 0.f);
      m_renderDevice->m_DMDShader->SetVector(ShaderUniform::vColor_Intensity, dotColor.x * fullBrightness, dotColor.y * fullBrightness,
         dotColor.z * fullBrightness, // Dot color (applied for luminance as well as sRGB frames)
         m_dmdDotProperties[profile].w); // Diffuse strength
      m_renderDevice->m_DMDShader->SetVector(ShaderUniform::staticColor_Alpha, m_dmdUnlitDotColor[profile].x, m_dmdUnlitDotColor[profile].y,
         m_dmdUnlitDotColor[profile].z, // Unlit dot color (ambient)
         static_cast<float>(colorSpace)); // Output colorspace (3D render is linear, backdrop is tonemapped but needs sRGB conversion, dedicated window is tonemapped sRGB)
      m_renderDevice->m_DMDShader->SetVector(ShaderUniform::vRes_Alpha_time, static_cast<float>(dmd->width()), static_cast<float>(dmd->height()), // DMD size in dots
         addBlendModulate, // Signed 'modulate vs add' factor of the additive blend encoding, 0 selecting the plain opaque output instead (see fs_display.sc)
         0.f); // Unused
      m_renderDevice->m_DMDShader->SetVector(ShaderUniform::displayProperties,
         dmd->m_format != BaseTexture::BW_FP32 ? 1.f : 0.f, // luminance or (s)RGB frame source
         0.5f
            * (1.0f + (1.0f / (2.0f /*N_SAMPLES*/ + 0.5f)) * m_dmdDotProperties[profile].x / 2.0f), // Internal SDF offset to obtain 0.5 at dot border, increasing inside, decreasing outside
         0.5f + 0.5f * (m_dmdDotProperties[profile].x * (1.0f - m_dmdDotProperties[profile].y) /* Dot border darkening */), // Dot internal SDF threshold
         0.f); // Unused
      m_renderDevice->m_DMDShader->SetTexture(ShaderUniform::displayTex, dmd.get());
      m_renderDevice->m_DMDShader->SetTechnique(ShaderTechnique::display_DMD_world);
   }
}

void Renderer::SetupCRTRender(int profile, const bool isBackdrop, const vec3& color, const float brightness, const std::shared_ptr<BaseTexture>& crt, const float alpha,
   const float addBlendModulate, const ColorSpace colorSpace, const Vertex3D_NoTex2* vertices, const vec4& emitterPad, const vec3& glassTint, const float glassRougness,
   ITexManCacheable* const glassTex, const vec4& glassArea, const vec3& glassAmbient, const bool unmirror)
{
   SetupDisplayRenderer(isBackdrop, vertices, emitterPad, glassTint, glassRougness, glassTex, glassArea, glassAmbient, unmirror);

   m_renderDevice->m_DMDShader->SetVector(ShaderUniform::vColor_Intensity, color.x * brightness, color.y * brightness,
      color.z * brightness, // CRT tint (applied for luminance as well as sRGB frames)
      1.0f); // Diffuse strength
   m_renderDevice->m_DMDShader->SetVector(ShaderUniform::staticColor_Alpha, 0.f, 0.f, 0.f, // unused
      static_cast<float>(colorSpace)); // Output colorspace (3D render is linear, backdrop is tonemapped but needs sRGB conversion, dedicated window is tonemapped sRGB)
   m_renderDevice->m_DMDShader->SetVector(ShaderUniform::vRes_Alpha_time, static_cast<float>(crt->width()), static_cast<float>(crt->height()), // CRT size in pixels
      addBlendModulate, // Signed 'modulate vs add' factor of the additive blend encoding, 0 selecting the plain opaque output instead (see fs_display.sc)
      0.f); // Unused
   // Which CRT emulation is picked, depends on the source resolution: Nuance-CRT for a 'modern' high res CRT (only Pinball 2000 for now, at 640px wide),
   // Lottes-CRT for the more ancient low resolution ones (Bally Vidpins, Mr. Games and Gottlieb Caveman are all 256px or less)
   constexpr unsigned int nuanceMinWidth = 384;
   const bool useNuanceCrt = (profile == 2) && (crt->width() >= nuanceMinWidth);
   m_renderDevice->m_DMDShader->SetVector(ShaderUniform::displayProperties,
      static_cast<float>(profile), // Render mode
      0.f, 0.f, 0.f); // Unused (CRT filters now evaluate on screen output size per pixel, from screen space derivatives)
   // Pixelated keeps crisp pixels when magnified, but is filtered (mipmapped) when downscaled to avoid moiree, smoothed is always filtered.
   // Lottes reads exact texels and reconstructs from them so it wants none, Nuance samples continuously so it wants bilinear
   const SamplerFilter displayFilter = profile == 0 ? SamplerFilter::SF_PIXELATED
      : profile == 1                                ? SamplerFilter::SF_ANISOTROPIC
      : useNuanceCrt                                ? SamplerFilter::SF_BILINEAR
                                                    : SamplerFilter::SF_NONE;
   // NuanceCRT evaluates in 'display gamma' space throughout, so it is bound without sRGB decoding and the sampler hands back the stored values as they are
   m_renderDevice->m_DMDShader->SetTexture(ShaderUniform::displayTex, crt.get(), useNuanceCrt, displayFilter);
   m_renderDevice->m_DMDShader->SetTechnique(useNuanceCrt ? ShaderTechnique::display_CRTnuance_world : ShaderTechnique::display_CRT_world);
}

void Renderer::DrawStatics()
{
   const unsigned int mask = m_render_mask;
   const bool isNoBackdrop = m_activeView->HasNoBackdrop() || ((m_render_mask & Renderer::REFLECTION_PASS) != 0);
   m_render_mask |= Renderer::STATIC_ONLY;
   for (auto renderable : g_pplayer->m_ptable->GetParts())
      RenderItem(renderable, isNoBackdrop);
   m_render_mask = mask;
}

void Renderer::DrawDynamics(bool onlyBalls)
{
   const unsigned int mask = m_render_mask;
   const bool isNoBackdrop = m_activeView->HasNoBackdrop() || ((m_render_mask & Renderer::REFLECTION_PASS) != 0);
   m_render_mask |= Renderer::DYNAMIC_ONLY;
   const bool isBackdropEdit = m_activeView->IsEditorBackdropEdit();
   if (!onlyBalls)
   {
      if (!isBackdropEdit)
         m_activeView->DrawBulbLightBuffer();
      for (auto renderable : g_pplayer->m_ptable->GetParts())
      {
         if (isBackdropEdit && !renderable->m_desktopBackdrop)
            continue;
         RenderItem(renderable, isNoBackdrop);
      }
   }
   else if (!isBackdropEdit)
   {
      for (Ball* ball : g_pplayer->m_vball)
         RenderItem(ball, isNoBackdrop);
   }
   m_render_mask = mask;
}

void Renderer::DrawSprite(
   const float posx, const float posy, const float width, const float height, const COLORREF color, const std::shared_ptr<const Sampler>& tex, const float intensity, const bool backdrop)
{
   Vertex3D_NoTex2 vertices[4]
      = { { 1.f, 1.f, 0.f, 0.f, 0.f, 1.f, 1.f, 1.f }, { 0.f, 1.f, 0.f, 0.f, 0.f, 1.f, 0.f, 1.f }, { 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 1.f, 0.f }, { 0.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f } };

   for (unsigned int i = 0; i < 4; ++i)
   {
      const Vertex2D clip = BackdropToClip(Vertex2D((vertices[i].x * width + posx) * (float)EDITOR_BG_WIDTH, (vertices[i].y * height + posy) * (float)EDITOR_BG_HEIGHT));
      vertices[i].x = clip.x;
      vertices[i].y = clip.y;
   }

   const vec4 c = convertColor(color, intensity);
   m_renderDevice->m_DMDShader->SetVector(ShaderUniform::vColor_Intensity, &c);
   m_renderDevice->m_DMDShader->SetTechnique(tex ? ShaderTechnique::basic_noDMD : ShaderTechnique::basic_noDMD_notex);
   if (tex)
      m_renderDevice->m_DMDShader->SetTexture(ShaderUniform::tex_sprite, tex, SamplerFilter::SF_TRILINEAR, SamplerAddressMode::SA_CLAMP, SamplerAddressMode::SA_CLAMP);
   m_renderDevice->m_DMDShader->SetVector(ShaderUniform::glassArea, 0.f, 0.f, 1.f, 1.f);
   m_renderDevice->SetRenderState(RenderState::ZENABLE, RenderState::RS_FALSE);
   // Screen space quad, not mirrored: the mirrored default cull mode must not apply
   m_renderDevice->SetRenderState(RenderState::CULLMODE, RenderState::CULL_NONE);
   m_renderDevice->DrawTexturedQuad(m_renderDevice->m_DMDShader, vertices);
   m_renderDevice->GetCurrentPass()->m_commands.back()->SetTransparent(true);
   m_renderDevice->GetCurrentPass()->m_commands.back()->SetDepth(-10000.f);
}

void Renderer::DrawWireframe(IEditable* const renderable, const vec4& fillColor, const vec4& edgeColor, bool withDepthMask)
{
   if (!renderable->GetIRenderable())
      return;

   unsigned int prevRenderMask = m_render_mask;
   m_renderDevice->ResetRenderState();
   m_renderDevice->EnableAlphaBlend(false);
   m_renderDevice->SetRenderState(RenderState::ZENABLE, withDepthMask ? RenderState::RS_TRUE : RenderState::RS_FALSE);
   m_renderDevice->SetRenderState(RenderState::ZWRITEENABLE, withDepthMask ? RenderState::RS_TRUE : RenderState::RS_FALSE);
   m_renderDevice->SetRenderState(RenderState::CULLMODE, RenderState::CULL_NONE);
   m_renderDevice->m_basicShader->SetTechnique(ShaderTechnique::unshaded_without_texture);
   // Render commands of a pass are sorted with opaque commands before transparent ones, so mark the wireframe commands
   // as transparent to ensure they are rendered in submission order (fill, then edges, both sharing the same sort depth)
   size_t cmdBase = m_renderDevice->GetCurrentPass()->m_commands.size();
   if (fillColor.w > 0.f)
   {
      m_render_mask = Renderer::RenderMask::UI_FILL;
      m_renderDevice->SetRenderState(RenderState::ZFUNC, RenderState::Z_LESS);
      m_renderDevice->SetRenderState(RenderState::ALPHABLENDENABLE, fillColor.w == 1.f ? RenderState::RS_FALSE : RenderState::RS_TRUE);
      m_renderDevice->m_basicShader->SetVector(ShaderUniform::staticColor_Alpha, &fillColor);
      RenderItem(renderable, false);
   }
   if (edgeColor.w > 0.f)
   {
      m_render_mask = Renderer::RenderMask::UI_EDGES;
      m_renderDevice->SetRenderState(RenderState::ZFUNC, RenderState::Z_LESSEQUAL);
      m_renderDevice->SetRenderState(RenderState::ALPHABLENDENABLE, edgeColor.w == 1.f ? RenderState::RS_FALSE : RenderState::RS_TRUE);
      m_renderDevice->m_basicShader->SetVector(ShaderUniform::staticColor_Alpha, &edgeColor);
      RenderItem(renderable, false);
   }
   for (size_t i = cmdBase; i < m_renderDevice->GetCurrentPass()->m_commands.size(); i++)
      m_renderDevice->GetCurrentPass()->m_commands[i]->SetTransparent(true);
   m_renderDevice->m_basicShader->SetVector(ShaderUniform::staticColor_Alpha, 1.f, 1.f, 1.f, 1.f);
   m_render_mask = prevRenderMask;
}

void Renderer::RenderMesh(const unsigned int renderMask, RenderMeshDesc& draw, const VisualMaterial& mat)
{
   assert(draw.mesh != nullptr);
   if (draw.transform != nullptr)
      UpdateBasicShaderMatrix(*draw.transform);

   if (renderMask & UI_FILL)
   {
      const std::shared_ptr<MeshBuffer>& fill = (draw.uiFillMesh != nullptr && *draw.uiFillMesh != nullptr) ? *draw.uiFillMesh : draw.mesh;
      m_renderDevice->DrawMesh(m_renderDevice->m_basicShader, true, draw.sortCenter, draw.depthBias, fill, draw.type, draw.startIndex,
         draw.indexCount != 0 ? draw.indexCount : (fill->m_ib ? fill->m_ib->m_count : 0));
   }
   else if (renderMask & UI_EDGES)
   {
      if (draw.uiEdgeMesh != nullptr && (draw.edgeMode == UIEdgeMode::VertexRing || (draw.edgeMode != UIEdgeMode::None && draw.edgeTriIndices != nullptr)))
      {
         if (*draw.uiEdgeMesh == nullptr)
         {
            if (draw.edgeMode == UIEdgeMode::VertexRing)
               *draw.uiEdgeMesh = draw.mesh->CreateVertexRingEdgeMeshBuffer();
            else if (draw.edgeTriIndices16)
            {
               const WORD* const indices = static_cast<const WORD*>(draw.edgeTriIndices);
               *draw.uiEdgeMesh = draw.edgeMode == UIEdgeMode::TriWire                   ? draw.mesh->CreateEdgeMeshBuffer(indices, draw.edgeTriCount)
                  : draw.edgeMode == UIEdgeMode::Outline && draw.edgeVertices != nullptr ? draw.mesh->CreateEdgeMeshBuffer(indices, draw.edgeTriCount, draw.edgeVertices)
                  : draw.edgeMode == UIEdgeMode::Quads                                   ? draw.mesh->CreateQuadOutlineEdgeMeshBuffer(indices, draw.edgeTriCount)
                                                                                         : nullptr;
            }
            else
            {
               const unsigned int* const indices = static_cast<const unsigned int*>(draw.edgeTriIndices);
               *draw.uiEdgeMesh = draw.edgeMode == UIEdgeMode::TriWire                   ? draw.mesh->CreateEdgeMeshBuffer(indices, draw.edgeTriCount)
                  : draw.edgeMode == UIEdgeMode::Outline && draw.edgeVertices != nullptr ? draw.mesh->CreateEdgeMeshBuffer(indices, draw.edgeTriCount, draw.edgeVertices)
                  : draw.edgeMode == UIEdgeMode::Quads                                   ? draw.mesh->CreateQuadOutlineEdgeMeshBuffer(indices, draw.edgeTriCount)
                                                                                         : nullptr;
            }
         }
         if (*draw.uiEdgeMesh != nullptr)
            m_renderDevice->DrawMesh(m_renderDevice->m_basicShader, true, draw.sortCenter, 0.f, *draw.uiEdgeMesh, RenderDevice::LINELIST, 0, (*draw.uiEdgeMesh)->m_ib->m_count);
      }
   }
   else
   {
      if (mat.material != nullptr)
      {
         Shader* const shader = m_renderDevice->m_basicShader;
         const bool alphaTest = mat.image != nullptr && mat.image->m_alphaTestValue >= 0.f && !mat.image->IsOpaque();
         shader->SetTechniqueMaterial(
            mat.technique != VisualMaterial::AUTO_TECHNIQUE ? mat.technique : (mat.image ? ShaderTechnique::basic_with_texture : ShaderTechnique::basic_without_texture), *mat.material,
            alphaTest, mat.normalMap != nullptr, mat.doReflection, mat.doRefraction);
         if (mat.image != nullptr)
         {
            shader->SetTexture(ShaderUniform::tex_base_color, mat.image, false, mat.filter, mat.clampU, mat.clampV);
            shader->SetAlphaTestValue(mat.image->m_alphaTestValue);
         }
         if (mat.normalMap != nullptr)
         {
            shader->SetTexture(ShaderUniform::tex_base_normalmap, mat.normalMap, true);
            shader->SetBool(ShaderUniform::objectSpaceNormalMap, mat.objectSpaceNormalMap);
         }
         shader->SetMaterial(mat.material, mat.hasTextureAlpha);
         if (mat.forceAlphaBlend)
            m_renderDevice->EnableAlphaBlend(false);
      }
      m_renderDevice->DrawMesh(m_renderDevice->m_basicShader, draw.transparent, draw.sortCenter, draw.depthBias, draw.mesh, draw.type, draw.startIndex,
         draw.indexCount != 0 ? draw.indexCount : (draw.mesh->m_ib ? draw.mesh->m_ib->m_count : 0));
   }

   if (draw.transform != nullptr)
      UpdateBasicShaderMatrix();
}

void Renderer::RenderItem(IEditable* const editable, bool isNoBackdrop)
{
   // Wireframe modes only apply to the main scene render, not to sub passes (UI wireframe fill/edges, light buffer, reflection probes)
   const bool isWireframe = m_activeView->GetShadeMode() != ShadeMode::Default && (m_render_mask & (UI_FILL | UI_EDGES | LIGHT_BUFFER | REFLECTION_PASS)) == 0;
   if (editable->GetIRenderable() == nullptr // Not renderable
      || (editable->m_desktopBackdrop && isNoBackdrop) // Don't render backdrop items in reflections or VR & cabinet modes
      || (!m_activeView->IsEditorView() // Apply player mode visibility mask (superseded by the editor's UI visibility in editor views)
         && editable->GetPartGroup() != nullptr && (editable->GetPartGroup()->GetPlayerModeVisibilityMask() & m_activeView->GetVisibilityMask()) == 0))
      return;

   const PartGroupData::SpaceReference spaceReference = editable->GetPartGroup() ? editable->GetPartGroup()->GetReferenceSpace() : PartGroupData::SpaceReference::Playfield;
   SetSpaceReference(spaceReference, false);

   // Parts flagged as "Show in Editor" are rendered shaded, even in wireframe shade modes
   if (!isWireframe || editable->IsShownInEditor())
   {
      editable->GetIRenderable()->Render(m_render_mask);
   }
   else
   {
      const vec4 fillColor
         = m_activeView->GetShadeMode() == ShadeMode::NoDepthWireframe ? vec4(0.f, 0.f, 0.f, (float)(32. / 255.)) : vec4((float)(32. / 255.), (float)(32. / 255.), (float)(32. / 255.), 1.f);
      constexpr vec4 edgeColor { 0.f, 0.f, 0.f, 1.f };
      DrawWireframe(editable, fillColor, edgeColor, m_activeView->GetShadeMode() != ShadeMode::NoDepthWireframe);
   }
}

void Renderer::DisableStaticPrePass(const bool disable)
{
   for (RenderView* view : m_renderViews)
      view->SetStaticPrepassDisabled(disable);
}

void Renderer::MarkStaticsDirty()
{
   // The accumulated static part prerender (and the static render probes, which are dirtied with it) is
   // stale: the views re-accumulate it on their next render. Views not using the prepass ignore the flag.
   for (RenderView* view : m_renderViews)
      view->MarkPrepassDirty();
}


#pragma region PostProcess

bool Renderer::IsBloomEnabled() const { return !m_bloomOff && (m_table->m_bloom_strength > 0.0f) && (g_pplayer->GetInfoMode() <= IF_DYNAMIC_ONLY); }


// org HDR -> display HDR spline-based remapping
// inspired by the videolan spline tonemapper

// converts from PQ to nits and vice versa
// precompute spline parameters based on all the constants in here and the HDR display range (in nits)
// Precompute the allenwp tonemapping curve parameters for AgX on the CPU,
// matching the shader code (see allenwpCurve in FBShader.fxh / fs_pp_tonemap.sc).
// Derived from Godot's implementation (https://github.com/godotengine/godot/pull/106940)
// Source and details: https://allenwp.com/blog/2025/05/29/allenwp-tonemapping-curve/
#pragma endregion

void Renderer::RenderUIScene()
{
   RenderTarget* const backBuffer = m_renderDevice->GetOutputBackBuffer();
   if (backBuffer == nullptr)
      return;

   // For stereo modes that pack the 2 eye views inside the output frame (side by side, top/bottom, interlaced),
   // the UI must be rendered to the per eye render buffer, then packed to the output, so that it is duplicated
   // per view with the correct orientation (same render path as Render3DScene)
   const bool uiBeforeStero = m_stereo3Denabled && (m_renderDevice->m_stereoShader != nullptr) && (GetBackBufferTexture() != nullptr)
      && (m_stereo3D == STEREO_SBS || m_stereo3D == STEREO_TB || m_stereo3D == STEREO_INT || m_stereo3D == STEREO_FLIPPED_INT);
   RenderTarget* const uiRT = uiBeforeStero ? GetBackBufferTexture() : backBuffer;
   m_renderDevice->SetRenderTarget("Loading"s, uiRT, false, true);
   m_renderDevice->Clear(clearType::TARGET, 0xFF000000);
#ifdef ENABLE_XR
   // For OpenXR, the UI draw list is rasterized to an offscreen surface, then drawn inside the
   // stereo projection as a world-space quad (same path as RenderView::Render)
   if (m_stereo3Denabled && m_stereo3D == STEREO_VR)
   {
      VRDevice* const vrDevice = g_pplayer->m_vrDevice;
      Vertex3Ds quad[4] = {}; // Degenerate (invisible) quad while the UI is not shown
      vrDevice->GetUIQuadScene(quad);
      ModelViewProj quadMvp(m_renderDevice->m_nEyes);
      const SceneLayout::SpaceTransform& space = g_pplayer->m_ptable->GetSceneLayout().GetSpaceTransform(VPX::SpaceReference::Playfield);
      for (int eye = 0; eye < m_renderDevice->m_nEyes; eye++)
      {
         float projScale;
         quadMvp.SetView(eye, SceneLayout::ComposeView(space, vrDevice->GetEyeView(eye), projScale));
         quadMvp.SetProj(eye, Matrix3D::MatrixScale(projScale) * vrDevice->GetEyeProj(eye));
      }
      g_pplayer->m_liveUI->RenderUIVR(quad, quadMvp);
   }
   else
#endif
      g_pplayer->m_liveUI->RenderUI();
   if (uiRT != backBuffer)
   {
      UpdateStereoShaderState();
      m_activeView->ApplyStereo(uiRT, backBuffer);
   }
}

RenderView* Renderer::AddView(RenderView::Kind kind)
{
   m_extraViews.push_back(std::make_unique<RenderView>(this, kind));
   RenderView* const view = m_extraViews.back().get();
   m_renderViews.push_back(view);
   return view;
}

void Renderer::RemoveView(RenderView* view)
{
   std::erase(m_renderViews, view);
   std::erase_if(m_extraViews, [view](const std::unique_ptr<RenderView>& v) { return v.get() == view; });
   if (m_activeView == view)
      m_activeView = m_primaryView.get();
}

void Renderer::SetBallImageOverride(const bool overwrite, const string& ballImagePath, const string& decalImagePath)
{
   for (const std::shared_ptr<BaseTexture>& tex : { m_ballImage, m_decalImage })
      if (tex)
         m_retiredBallImages.push_back(tex);
   m_overwriteBallImages = overwrite;
   m_ballImagePath = ballImagePath;
   m_decalImagePath = decalImagePath;
   const unsigned int maxTexDim = m_table->GetSettings().GetPlayer_MaxTexDimension();
   m_ballImage = overwrite ? BaseTexture::CreateFromFile(ballImagePath, maxTexDim) : nullptr;
   m_decalImage = overwrite ? BaseTexture::CreateFromFile(decalImagePath, maxTexDim) : nullptr;
   if (g_pplayer)
      for (Ball* const ball : g_pplayer->m_vball)
         ball->ResolveImages();
}

void Renderer::Render3DScene()
{
   m_activeView = m_primaryView.get();

   // Release the overwrite ball images replaced since the previous 3D scene (see SetBallImageOverride)
   for (const std::shared_ptr<BaseTexture>& tex : m_retiredBallImages)
      m_renderDevice->m_texMan.UnloadTexture(tex.get());
   m_retiredBallImages.clear();

   // Reinitialize parts that have been modified
   SetupShaders();
   for (auto renderable : m_renderableToInit)
   {
      renderable->RenderRelease();
      renderable->RenderSetup(this);
   }
   m_renderableToInit.clear();
   for (auto renderable : m_renderableToSetup)
      renderable->RenderSetup(this);
   m_renderableToSetup.clear();

   // Setup ball rendering: collect all lights that can reflect on balls
   m_ballTrailMeshBufferPos = 0;
   m_ballReflectedLights.clear();
   for (IEditable* const item : m_table->GetParts())
   {
      if (item && item->GetItemType() == eItemLight && static_cast<Light*>(item)->m_d.m_showReflectionOnBall && !static_cast<Light*>(item)->m_desktopBackdrop)
         m_ballReflectedLights.push_back(static_cast<Light*>(item));
   }

#if defined(ENABLE_OPENGL) && defined(__STANDALONE__)
   SDL_GL_MakeCurrent(g_pplayer->m_playfieldWnd->GetCore(), g_pplayer->m_renderer->m_renderDevice->m_sdl_context);
#endif

   // The desktop display gets its own dedicated scene render while playing in VR (lazily created view)
   if (m_stereo3D == STEREO_VR && NeedsDesktopSceneRender() && m_renderDevice->m_outputWnd.size() >= 2 && m_renderDevice->m_outputWnd[1]->GetBackBuffer())
   {
      if (m_vrMirrorView == nullptr)
      {
         m_vrMirrorView = std::make_unique<RenderView>(this, RenderView::Kind::VRMirror);
         m_vrMirrorView->SetOutputWindow(m_renderDevice->m_outputWnd[1]);
         m_vrMirrorView->SetApplyPostFX(false);
         m_vrMirrorView->SetOutputLayer(0); // The desktop display scene render only writes to the first layer of its (stereo typed) render targets
         m_renderViews.push_back(m_vrMirrorView.get());
      }
   }

   // Views released during the previous frame's record (the editor UI registers/unregisters its panes
   // while it is being recorded) are destroyed now, outside of the record loop
   m_deadViews.clear();

   // Record every view into the frame: the primary view (playfield window or VR headset), then the
   // optional VR desktop mirror and any additional views (editor viewport panes)
   for (size_t i = 0; i < m_renderViews.size(); i++)
   {
      RenderView* const view = m_renderViews[i];
      m_activeView = view;
      // The primary view follows the global editor state; the other views own theirs (per view editor flags)
      if (view == m_primaryView.get())
         view->SetEditorState(g_pplayer->m_liveUI->IsEditorViewMode(), g_pplayer->m_liveUI->IsEditorBackdropViewMode());
      view->Render();
   }
   m_activeView = m_primaryView.get();

   // While the primary view is sceneless (the editor areas own the frame, the viewport panes drawing the
   // scene to their areas of the shared output), its LiveUI pass is recorded now, at the end of the frame
   // record: the pass is then sorted after the views' outputs, compositing the UI above the 3D panes
   if (!m_primaryView->IsRenderingScene() && m_primaryView->RendersLiveUI() && m_primaryView->GetOutput() != nullptr)
   {
      m_renderDevice->SetRenderTarget("LiveUI"s, m_primaryView->GetOutput(), true, true);
      g_pplayer->m_liveUI->RenderUI();
   }

   if (g_pplayer->GetProfilingMode() == PF_ENABLED)
      m_gpu_profiler.Timestamp(GTS_PostProcess);
}

RenderView* Renderer::CreateEditorView()
{
   // Registered views are recorded with the other views, before the deferred LiveUI pass of a sceneless primary view
   m_extraViews.push_back(std::make_unique<RenderView>(this, RenderView::Kind::Editor));
   RenderView* const view = m_extraViews.back().get();
   m_renderViews.push_back(view);
   return view;
}

void Renderer::ReleaseEditorView(RenderView* const view)
{
   std::erase(m_renderViews, view);
   // The view object and its GPU targets are kept until the next frame: release happens while the
   // frame is being recorded, so they may still be referenced by the current record
   for (size_t i = 0; i < m_extraViews.size(); i++)
      if (m_extraViews[i].get() == view)
      {
         m_deadViews.push_back(std::move(m_extraViews[i]));
         m_extraViews.erase(m_extraViews.begin() + i);
         return;
      }
}


////////////////////////////////////////////////////////////////////////////////

void Renderer::AddAncillaryContent(VPXRenderContext2D* ctx, const float srcX, const float srcY, const float srcW, const float srcH)
{
   vec4& bounds = static_cast<AncillaryRenderSetup*>(ctx->rendererData)->contentBounds;
   bounds.x = min(bounds.x, srcX / ctx->srcWidth);
   bounds.y = min(bounds.y, srcY / ctx->srcHeight);
   bounds.z = max(bounds.z, (srcX + srcW) / ctx->srcWidth);
   bounds.w = max(bounds.w, (srcY + srcH) / ctx->srcHeight);
}

void Renderer::DrawImage(VPXRenderContext2D* ctx, VPXTexture texture, const float tintR, const float tintG, const float tintB, const float alpha, const float texX, const float texY,
   const float texW, const float texH, const float pivotX, const float pivotY, const float rotation, const float srcX, const float srcY, const float srcW, const float srcH)
{
   assert(g_pplayer && g_pplayer->m_renderer && ctx->rendererData == &g_pplayer->m_renderer->m_ancillaryRenderSetup);
   if (alpha <= 0.f) // Alpha blended, so alpha = 0 means not visible
      return;
   AddAncillaryContent(ctx, srcX, srcY, srcW, srcH); // Image rotation ignored: only the area used by the layout is wanted
   const bool isLinearOutput = g_pplayer->m_renderer->m_ancillaryRenderSetup.isOutputLinear;
   std::shared_ptr<BaseTexture> const tex = g_pplayer->m_pluginAPI.GetTexture(texture);
   RenderDevice* const rdl = g_pplayer->m_renderer->m_renderDevice;
   rdl->ResetRenderState();
   rdl->SetRenderState(RenderState::CULLMODE, RenderState::CULL_NONE); // 2D (not mirrored) or on a double sided flasher: the mirrored default cull mode must not apply
   rdl->SetRenderState(RenderState::ZWRITEENABLE, RenderState::RS_FALSE);
   rdl->SetRenderState(RenderState::ZENABLE, ctx->is2D ? RenderState::RS_FALSE : RenderState::RS_TRUE);
   rdl->SetRenderState(RenderState::SRCBLEND, RenderState::SRC_ALPHA);
   rdl->SetRenderState(RenderState::DESTBLEND, RenderState::INVSRC_ALPHA);
   rdl->SetRenderState(RenderState::BLENDOP, RenderState::BLENDOP_ADD);
   rdl->SetRenderState(RenderState::ALPHABLENDENABLE, (alpha != 1.f || !tex->IsOpaque()) ? RenderState::RS_TRUE : RenderState::RS_FALSE);
   rdl->m_basicShader->SetVector(ShaderUniform::staticColor_Alpha, tintR, tintG, tintB, alpha);
   // We force to linear (no sRGB decoding) when rendering in sRGB colorspace, this assumes that the texture is in sRGB colorspace to get correct gamma (other situations would need dedicated shaders to handle them efficiently)
   assert(tex->m_format == BaseTexture::SRGB || tex->m_format == BaseTexture::SRGBA || tex->m_format == BaseTexture::SRGB565);
   // Disable filtering and mipmap generation if they are not needed
   const SamplerFilter sf = (ctx->is2D && (srcW * ctx->outWidth == ctx->srcWidth * (float)tex->width()) && (srcH * ctx->outHeight == ctx->srcHeight * (float)tex->height()))
      ? SamplerFilter::SF_NONE
      : SamplerFilter::SF_UNDEFINED;
   rdl->m_basicShader->SetTexture(ShaderUniform::tex_base_color, tex.get(), !isLinearOutput, sf);
   const float vx1 = srcX / ctx->srcWidth;
   const float vy1 = srcY / ctx->srcHeight;
   const float vx2 = vx1 + srcW / ctx->srcWidth;
   const float vy2 = vy1 + srcH / ctx->srcHeight;
   const float tx1 = texX / (float)tex->width();
   const float ty1 = 1.f - texY / (float)tex->height();
   const float tx2 = (texX + texW) / (float)tex->width();
   const float ty2 = 1.f - (texY + texH) / (float)tex->height();
   Vertex3D_NoTex2 vertices[4] = { //
      { vx2, vy1, 0.f, 0.f, 0.f, 1.f, tx2, ty2 }, //
      { vx2, vy2, 0.f, 0.f, 0.f, 1.f, tx2, ty1 }, //
      { vx1, vy1, 0.f, 0.f, 0.f, 1.f, tx1, ty2 }, //
      { vx1, vy2, 0.f, 0.f, 0.f, 1.f, tx1, ty1 }
   };
   if (rotation != 0.f)
   {
      const float px = lerp(vx1, vx2, (pivotX - texX) / texW);
      const float py = lerp(vy1, vy2, (pivotY - texY) / texH);
      const Matrix3D matRot = Matrix3D::MatrixTranslate(-px, -py, 0.f) * Matrix3D::MatrixScale(ctx->srcWidth, ctx->srcHeight, 1.f) * Matrix3D::MatrixRotateZ(rotation * (float)(M_PI / 180.0))
         * Matrix3D::MatrixScale(1.f / ctx->srcWidth, 1.f / ctx->srcHeight, 1.f) * Matrix3D::MatrixTranslate(px, py, 0.f);
      matRot.TransformPositions(vertices, vertices, 4);
   }
   static_cast<AncillaryRenderSetup*>(ctx->rendererData)->displayTransform.TransformVertices(vertices, vertices, 4);
   rdl->m_basicShader->SetTechnique(ShaderTechnique::unshaded_with_texture);
   rdl->DrawTexturedQuad(rdl->m_basicShader, vertices, true, g_pplayer->m_renderer->m_ancillaryRenderSetup.depthbias);
   if (alpha != 1.f || tintR != 1.f || tintG != 1.f || tintB != 1.f)
      rdl->m_basicShader->SetVector(ShaderUniform::staticColor_Alpha, 1.f, 1.f, 1.f, 1.f);
}

void Renderer::DrawMatrixDisplay(VPXRenderContext2D* ctx, VPXDisplayRenderStyle style, VPXTexture glassTex, const float glassTintR, const float glassTintG, const float glassTintB,
   const float glassRoughness, const float glassAreaX, const float glassAreaY, const float glassAreaW, const float glassAreaH, const float glassAmbientR, const float glassAmbientG,
   const float glassAmbientB, VPXTexture dispTex, const float dispTintR, const float dispTintG, const float dispTintB, const float brightness, const float alpha, const float dispPadL,
   const float dispPadT, const float dispPadR, const float dispPadB, const float srcX, const float srcY, const float srcW, const float srcH)
{
   assert(g_pplayer && g_pplayer->m_renderer && ctx->rendererData == &g_pplayer->m_renderer->m_ancillaryRenderSetup);
   AddAncillaryContent(ctx, srcX, srcY, srcW, srcH);
   const bool isLinearOutput = g_pplayer->m_renderer->m_ancillaryRenderSetup.isOutputLinear;
   VPXPluginAPIImpl& vxpApi = g_pplayer->m_pluginAPI;
   std::shared_ptr<BaseTexture> const gTex = glassTex ? vxpApi.GetTexture(glassTex) : nullptr;
   std::shared_ptr<BaseTexture> const dTex = vxpApi.GetTexture(dispTex);
   RenderDevice* const rdl = g_pplayer->m_renderer->m_renderDevice;
   rdl->ResetRenderState();
   rdl->SetRenderState(RenderState::CULLMODE, RenderState::CULL_NONE); // 2D (not mirrored) or on a double sided flasher: the mirrored default cull mode must not apply
   rdl->SetRenderState(RenderState::ZWRITEENABLE, RenderState::RS_FALSE);
   rdl->SetRenderState(RenderState::ZENABLE, ctx->is2D ? RenderState::RS_FALSE : RenderState::RS_TRUE);
   rdl->SetRenderState(RenderState::ALPHABLENDENABLE, RenderState::RS_FALSE);
   const float vx1 = srcX / ctx->srcWidth;
   const float vy1 = 1.f - srcY / ctx->srcHeight;
   const float vx2 = (srcX + srcW) / ctx->srcWidth;
   const float vy2 = 1.f - (srcY + srcH) / ctx->srcHeight;
   Vertex3D_NoTex2 vertices[4] = { //
      { vx2, vy1, 0.f, 0.f, 0.f, 1.f, 1.f, 1.f }, //
      { vx1, vy1, 0.f, 0.f, 0.f, 1.f, 0.f, 1.f }, //
      { vx2, vy2, 0.f, 0.f, 0.f, 1.f, 1.f, 0.f }, //
      { vx1, vy2, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f }
   };
   static_cast<AncillaryRenderSetup*>(ctx->rendererData)->displayTransform.TransformVertices(vertices, vertices, 4);
   if (style == VPXDMDStyle_Pixelated || style == VPXDMDStyle_Smoothed || style == VPXDMDStyle_CRT)
   {
#if !defined(ENABLE_BGFX)
      return; // just to avoid a crash
#endif
      g_pplayer->m_renderer->SetupCRTRender(style - VPXDMDStyle_Pixelated, ctx->is2D, vec3(dispTintR, dispTintG, dispTintB), brightness, dTex, alpha,
         0.f, // Never additive, this path draws opaque (blending is disabled above)
         isLinearOutput ? Renderer::ColorSpace::Linear : Renderer::ColorSpace::Reinhard_sRGB, //
         vertices, vec4(dispPadL, dispPadT, dispPadR, dispPadB), vec3(glassTintR, glassTintG, glassTintB), glassRoughness, gTex.get(),
         vec4(glassAreaX, glassAreaY, glassAreaW, glassAreaH), //
         vec3(glassAmbientR, glassAmbientG, glassAmbientB));
   }
   else
   {
      g_pplayer->m_renderer->SetupDMDRender(style, ctx->is2D, vec3(dispTintR, dispTintG, dispTintB), brightness, dTex, alpha,
         0.f, // Never additive, this path draws opaque (blending is disabled above)
         isLinearOutput ? Renderer::ColorSpace::Linear : Renderer::ColorSpace::Reinhard_sRGB, //
         vertices, vec4(dispPadL, dispPadT, dispPadR, dispPadB), vec3(glassTintR, glassTintG, glassTintB), glassRoughness, gTex.get(),
         vec4(glassAreaX, glassAreaY, glassAreaW, glassAreaH), //
         vec3(glassAmbientR, glassAmbientG, glassAmbientB));
   }
   rdl->DrawTexturedQuad(rdl->m_DMDShader, vertices, true, g_pplayer->m_renderer->m_ancillaryRenderSetup.depthbias);
}

void Renderer::DrawSegmentDisplay(VPXRenderContext2D* ctx, VPXSegDisplayRenderStyle style, VPXSegDisplayHint shapeHint, VPXTexture glassTex, const float glassTintR, const float glassTintG,
   const float glassTintB, const float glassRoughness, const float glassAreaX, const float glassAreaY, const float glassAreaW, const float glassAreaH, const float glassAmbientR,
   const float glassAmbientG, const float glassAmbientB, SegElementType type, const float* state, const float dispTintR, const float dispTintG, const float dispTintB, const float brightness,
   const float alpha, const float dispPadL, const float dispPadT, const float dispPadR, const float dispPadB, const float srcX, const float srcY, const float srcW, const float srcH)
{
   assert(g_pplayer && g_pplayer->m_renderer && ctx->rendererData == &g_pplayer->m_renderer->m_ancillaryRenderSetup);
   AddAncillaryContent(ctx, srcX, srcY, srcW, srcH);
#if !defined(ENABLE_BGFX)
   return; // Segment displays are only implemented by the BGFX shaders
#endif
   const bool isLinearOutput = g_pplayer->m_renderer->m_ancillaryRenderSetup.isOutputLinear;
   VPXPluginAPIImpl& vxpApi = g_pplayer->m_pluginAPI;
   std::shared_ptr<BaseTexture> const gTex = glassTex ? vxpApi.GetTexture(glassTex) : nullptr;
   RenderDevice* const rdl = g_pplayer->m_renderer->m_renderDevice;
   // Use max blending as segment may overlap in the glass diffuse: we retain the most lighted one which is wrong but looks ok (otherwise we would have to deal with colorspace conversions and layering between glass and emitter)
   rdl->ResetRenderState();
   rdl->SetRenderState(RenderState::CULLMODE, RenderState::CULL_NONE); // 2D (not mirrored) or on a double sided flasher: the mirrored default cull mode must not apply
   rdl->SetRenderState(RenderState::ZWRITEENABLE, RenderState::RS_FALSE);
   rdl->SetRenderState(RenderState::ZENABLE, ctx->is2D ? RenderState::RS_FALSE : RenderState::RS_TRUE);
   rdl->SetRenderState(RenderState::BLENDOP, RenderState::BLENDOP_MAX);
   rdl->SetRenderState(RenderState::ALPHABLENDENABLE, RenderState::RS_TRUE);
   rdl->SetRenderState(RenderState::SRCBLEND, RenderState::SRC_ALPHA);
   rdl->SetRenderState(RenderState::DESTBLEND, RenderState::ONE);
   const float vx1 = srcX / ctx->srcWidth;
   const float vy1 = 1.f - srcY / ctx->srcHeight;
   const float vx2 = (srcX + srcW) / ctx->srcWidth;
   const float vy2 = 1.f - (srcY + srcH) / ctx->srcHeight;
   Vertex3D_NoTex2 vertices[4] = { //
      { vx2, vy1, 0.f, 0.f, 0.f, 1.f, 1.f, 1.f }, //
      { vx1, vy1, 0.f, 0.f, 0.f, 1.f, 0.f, 1.f }, //
      { vx2, vy2, 0.f, 0.f, 0.f, 1.f, 1.f, 0.f }, //
      { vx1, vy2, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f }
   };
   static_cast<AncillaryRenderSetup*>(ctx->rendererData)->displayTransform.TransformVertices(vertices, vertices, 4);
   g_pplayer->m_renderer->SetupSegmentRenderer(style, ctx->is2D, vec3(dispTintR, dispTintG, dispTintB), brightness, (Renderer::SegmentFamily)shapeHint, type, state,
      isLinearOutput ? Renderer::ColorSpace::Linear : Renderer::ColorSpace::Reinhard_sRGB, vertices, vec4(dispPadL, dispPadT, dispPadR, dispPadB), vec3(glassTintR, glassTintG, glassTintB),
      glassRoughness, gTex.get(), vec4(glassAreaX, glassAreaY, glassAreaW, glassAreaH), vec3(glassAmbientR, glassAmbientG, glassAmbientB));
   rdl->DrawTexturedQuad(rdl->m_DMDShader, vertices, true, g_pplayer->m_renderer->m_ancillaryRenderSetup.depthbias);
}

int Renderer::GetAncillaryWindowRotation(VPXWindowId window) const
{
   assert(VPXWindowId::VPXWINDOW_Backglass <= window && window <= VPXWindowId::VPXWINDOW_Topper);
   return m_ancillaryWndRotation[window];
}

void Renderer::SetAncillaryWindowRotation(VPXWindowId window, const int clockwiseDegrees)
{
   assert(VPXWindowId::VPXWINDOW_Backglass <= window && window <= VPXWindowId::VPXWINDOW_Topper);
   m_ancillaryWndRotation[window] = 90 * clamp(clockwiseDegrees / 90, 0, 3);
}

int Renderer::GetAncillaryContentRotation(VPXWindowId window, const VPX::RenderOutput& output) const
{
   const int playfieldRotation = output.GetMode() == VPX::RenderOutput::OM_EMBEDDED ? GetEmbeddedOutputRotation() : 0;
   return (GetAncillaryWindowRotation(window) + playfieldRotation) % 360;
}

int Renderer::GetEmbeddedOutputRotation() const
{
   // Embedded outputs are composited into the playfield window, in VR into the desktop display (also rendered with the table's view setup)
   const VPX::Window* const wnd = g_pplayer->IsVR() ? (m_renderDevice->m_outputWnd.size() >= 2 ? m_renderDevice->m_outputWnd[1] : nullptr) : m_renderDevice->m_outputWnd[0];
   if (wnd == nullptr)
      return 0;
   // As the scene (see ViewSetup::ComputeMVP); camera mode allows any angle: use the nearest quarter turn
   const float rotation = m_table->GetViewSetup().GetRotation(wnd->GetWidth(), wnd->GetHeight());
   return 90 * (static_cast<int>((rotation + 45.f) / 90.f) % 4);
}

RenderTarget* Renderer::SetupAncillaryRenderTarget(
   VPXWindowId window, const VPX::RenderOutput& output, RenderTarget* embedRT, int& outputX, int& outputY, int& outputW, int& outputH, bool& isOutputLinear)
{
   assert(VPXWindowId::VPXWINDOW_Backglass <= window && window <= VPXWindowId::VPXWINDOW_Topper);
   static std::array<string, 3> renderPassNames = { "Backglass Render"s, "ScoreView Render"s, "Topper Render"s };
   static std::array<string, 3> hdrRTNames = { "BackglassBackBuffer"s, "ScoreViewBackBuffer"s, "TopperBackBuffer"s };
   const string& renderPassName = renderPassNames[window - VPXWindowId::VPXWINDOW_Backglass];
   const string& hdrRTName = hdrRTNames[window - VPXWindowId::VPXWINDOW_Backglass];

   // In VR, ancillary OS windows are only rendered when the desktop display shows the playfield and the other
   // displays, while embedded ancillary displays are only composited into the desktop display scene render
   const bool ancillaryAllowed = m_vrDesktop == VRDesktopMode::All && (output.GetMode() == VPX::RenderOutput::OM_WINDOW ? !m_activeView->IsMirror() : m_activeView->IsMirror());
   if (g_pplayer->IsVR() && !ancillaryAllowed)
   {
      // Hide the window if it was left visible by a previous 'playfield and other displays' desktop mode
      if (!m_activeView->IsMirror() && output.GetMode() == VPX::RenderOutput::OM_WINDOW && output.GetWindow() != nullptr && output.GetWindow()->IsVisible())
         output.GetWindow()->Show(false);
      return nullptr;
   }

   // Stereo Postprocessing is not yet implemented for embedded window (the VR desktop display scene render
   // does support it since it only renders to the first layer of its stereo render targets)
   if (m_stereo3D != StereoMode::STEREO_OFF && output.GetMode() != VPX::RenderOutput::OM_WINDOW && !m_activeView->IsMirror())
      return nullptr;

   RenderTarget* outputRT;
   if (output.GetMode() == VPX::RenderOutput::OM_EMBEDDED)
   {
      outputRT = embedRT;
      VPX::Window* containerWnd = m_renderDevice->m_outputWnd[m_activeView->IsMirror() ? 1 : 0];

      // Embedded window coordinates are logical units of the container window, while the embed
      // render target is sized in its output pixels (including full scene antialiasing upscaling)
      const float displayScaleX = static_cast<float>(outputRT->GetWidth())  / static_cast<float>(containerWnd->GetWidth());
      const float displayScaleY = static_cast<float>(outputRT->GetHeight()) / static_cast<float>(containerWnd->GetHeight());

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
      if (outputRT == nullptr) // The swapchain is being recreated (see Window::OnResized), skip rendering this window
         return nullptr;
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

   // Maps the [0..1] square the ancillary renderers draw in onto the output region
   Matrix3D matOutput = Matrix3D::MatrixIdentity();
   matOutput._11 = 2.f * static_cast<float>(outputW) / static_cast<float>(outputRT->GetWidth());
   matOutput._41 = -1.f + 2.f * static_cast<float>(outputX) / static_cast<float>(outputRT->GetWidth());
   matOutput._22 = -2.f * static_cast<float>(outputH) / static_cast<float>(outputRT->GetHeight());
   matOutput._42 = 1.f - 2.f * static_cast<float>(outputY) / static_cast<float>(outputRT->GetHeight());

   Matrix3D matWorldViewProj[2];
   // Rotated around the center of that square, the same way DrawImage rotates around its pivot
   if (const int rotation = GetAncillaryContentRotation(window, output); rotation != 0)
      matWorldViewProj[0]
         = Matrix3D::MatrixTranslate(-0.5f, -0.5f, 0.f) * Matrix3D::MatrixRotateZ(ANGTORAD(static_cast<float>(rotation))) * Matrix3D::MatrixTranslate(0.5f, 0.5f, 0.f) * matOutput;
   else
      matWorldViewProj[0] = matOutput;
   const int eyes = m_stereo3D != StereoMode::STEREO_OFF ? 2 : 1;
   if (eyes > 1)
      matWorldViewProj[1] = matWorldViewProj[0];
#if defined(ENABLE_BGFX)
   const vec4 cameraPosWorld[2] = { { 0.f, 0.f, 0.f, 0.f }, { 0.f, 0.f, 0.f, 0.f } };
   rd->m_basicShader->SetMatrix(ShaderUniform::matRotViewProj, &matWorldViewProj[0], eyes);
   rd->m_basicShader->SetVector(ShaderUniform::cameraPosWorld, &cameraPosWorld[0], eyes);
   rd->m_DMDShader->SetMatrix(ShaderUniform::matRotViewProj, &matWorldViewProj[0], eyes);
   rd->m_DMDShader->SetVector(ShaderUniform::cameraPosWorld, &cameraPosWorld[0], eyes);
#elif defined(ENABLE_OPENGL)
   rd->SetBasicShaderScreenSpaceMatrices(matWorldViewProj[0]);
   rd->m_DMDShader->SetMatrix(ShaderUniform::matWorldViewProj, &matWorldViewProj[0], eyes);
#elif defined(ENABLE_DX9)
   rd->m_basicShader->SetMatrix(ShaderUniform::matWorldViewProj, &matWorldViewProj[0], eyes);
   rd->m_DMDShader->SetMatrix(ShaderUniform::matWorldViewProj, &matWorldViewProj[0], eyes);
#endif
   rd->m_basicShader->SetFloat(ShaderUniform::alphaTestValue, -1.0f);
   rd->m_basicShader->SetTechnique(ShaderTechnique::bg_decal_with_texture);
   rd->m_DMDShader->SetFloat(ShaderUniform::alphaTestValue, -1.0f);

   // Performing linear rendering + tonemapping is overkill when used for LDR rendering (Pup pack, B2S,...)
   // so dedicated windows compose directly in sRGB. Embedded windows on the other hand are composited
   // into the linear render buffer and converted by the shared postprocess chain, which is what keeps
   // them correct on an HDR10/BT.2100 backbuffer, where sRGB values would be read as PQ and blow out.
   // TODO we should allow plugins to decide if they want linear colorspace + tonemapping or simple sRGB composition
   isOutputLinear = output.GetMode() == VPX::RenderOutput::OM_EMBEDDED;

   if (output.GetMode() == VPX::RenderOutput::OM_WINDOW)
   {
      if (isOutputLinear)
      {
         assert(false); // This is disabled for the time being
         if (m_ancillaryWndHdrRT[window] == nullptr)
            m_ancillaryWndHdrRT[window] = std::unique_ptr<RenderTarget>(
               new RenderTarget(rd, SurfaceType::RT_DEFAULT, hdrRTName, outputW, outputH, colorFormat::RGBA16F, false, 1, "Fatal Error: unable to create ancillary window back buffer"));
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

// Pages of the in game UI adjusting the displays, indexed by VPXWindowId
static const string s_displaySettingsPages[] = { "settings/display_playfield"s, "settings/display_backglass"s, "settings/display_scoreview"s, "settings/display_topper"s };

void Renderer::DrawEmbeddedQuad(RenderTarget* outputRT, int x, int y, int w, int h, float r, float g, float b)
{
   Vertex3D_NoTex2 vertices[4] = { { 1.f, 1.f, 0.f, 0.f, 0.f, 1.f, 1.f, 1.f }, //
      { 0.f, 1.f, 0.f, 0.f, 0.f, 1.f, 0.f, 1.f }, //
      { 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 1.f, 0.f }, //
      { 0.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f } };
   const float sx = 1.f / static_cast<float>(outputRT->GetWidth());
   const float sy = 1.f / static_cast<float>(outputRT->GetHeight());
   for (unsigned int i = 0; i < 4; ++i)
   {
      vertices[i].x =        sx * (vertices[i].x * static_cast<float>(w) + static_cast<float>(x)) * 2.0f - 1.0f;
      vertices[i].y = 1.0f - sy * (vertices[i].y * static_cast<float>(h) + static_cast<float>(y)) * 2.0f;
   }
   RenderDevice* const rd = m_renderDevice;
   rd->m_DMDShader->SetVector(ShaderUniform::vColor_Intensity, r, g, b, 1.f);
   rd->m_DMDShader->SetTechnique(ShaderTechnique::basic_noDMD_notex);
   rd->m_DMDShader->SetVector(ShaderUniform::glassArea, 0.f, 0.f, 1.f, 1.f);
   // Screen space quad, not mirrored: the mirrored default cull mode must not apply
   rd->SetRenderState(RenderState::CULLMODE, RenderState::CULL_NONE);
   rd->DrawTexturedQuad(rd->m_DMDShader, vertices);
   rd->GetCurrentPass()->m_commands.back()->SetTransparent(true);
   rd->GetCurrentPass()->m_commands.back()->SetDepth(-10000.f);
   rd->m_DMDShader->SetVector(ShaderUniform::vColor_Intensity, 1.f, 1.f, 1.f, 1.f);
}

void Renderer::ClearEmbeddedAncillaryWindow(VPXWindowId window, const VPX::RenderOutput& output, RenderTarget* embedRT)
{
   if (output.GetMode() != VPX::RenderOutput::OM_EMBEDDED)
      return;

   // No renderer claimed this window on the last frame: leave the playfield visible
   // instead of showing an empty black area, except while a display is being adjusted
   // in the in game UI where all regions must be visible so the user can align them
   const bool adjustingDisplays = std::ranges::any_of(s_displaySettingsPages, [](const string& page) { return g_pplayer->m_liveUI->m_inGameUI.IsOpened(page); });
   if (!m_ancillaryWndRendered[window] && !adjustingDisplays)
      return;

   bool isOutputLinear;
   int m_outputX, m_outputY, m_outputW, m_outputH;
   RenderTarget* outputRT = SetupAncillaryRenderTarget(window, output, embedRT, m_outputX, m_outputY, m_outputW, m_outputH, isOutputLinear);
   if (outputRT == nullptr)
      return;

   RenderDevice* const rd = m_renderDevice;
   rd->ResetRenderState();
   rd->SetRenderState(RenderState::ZENABLE, RenderState::RS_FALSE);
   rd->SetRenderState(RenderState::ZWRITEENABLE, RenderState::RS_TRUE); // Also clear depth to avoid AO artefacts
   DrawEmbeddedQuad(outputRT, m_outputX, m_outputY, m_outputW, m_outputH, 0.f, 0.f, 0.f);

   UpdateBasicShaderMatrix();
}

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
   const float src_hdr_max
      = max(1000.f, displayMaxLum); // assumes 1000 nits as scene input max, but adapts to displays with higher nits, as then everything stays perfectly linear //!! use 10000. for max input?

   constexpr float slope_tuning = 1.5f; //[0..10]
   constexpr float slope_offset = 0.2f; //[0..1]
   constexpr float spline_contrast = 0.5f; //[0..1.5]

#if 1
   constexpr float knee_minimum = 0.1f; //(0..0.5)
   constexpr float knee_maximum = 0.8f; //(0.5..1.0)
   constexpr float knee_default = 0.4f; //inbetween min..max
   constexpr float knee_adaptation = 0.4f; //[0..1]

   // Pick a spline knee point
   // Could be based on HDR10+ brightness metadata and/or scene brightness average
   // We pick it based on a heuristic that assumes a scene input max of src_hdr_max
   // Inspired by SMPTE ST2094-10, with some modifications, see videolan/st2094_pick_knee()

   const float src_max = pl_hdr_rescale(PL_HDR_NITS, PL_HDR_PQ, src_hdr_max);
   //const float src_avg = pl_hdr_rescale(PL_HDR_NITS, PL_HDR_PQ, 10.f); //!! 10 nits if real scene input average not available?! claimed to be "industry standard"
   const float dst_max = pl_hdr_rescale(PL_HDR_NITS, PL_HDR_PQ, displayMaxLum);

   const float src_knee_min = src_max * knee_minimum;
   const float src_knee_max = src_max * knee_maximum;
   const float dst_knee_min = dst_max * knee_minimum;
   const float dst_knee_max = dst_max * knee_maximum;

   float src_knee = /*src_avg!=0. ? src_avg :*/ src_max * knee_default; //!! see above. What is a better default? Most likely this here, given that pinball machines are rather bright?!
   src_knee = clamp(src_knee, src_knee_min, src_knee_max);

   // Choose target adaptation point based on linearly re-scaling the source knee
   const float target = src_knee / src_max;
   const float adapted = dst_max * target;

   // Choose the destination knee by picking the perceptual adaptation point
   // between the source knee and the desired target. This moves the knee
   // point, on the vertical axis, closer to the 1:1 (neutral) line.
   //
   // Adjust the adaptation strength towards 1 based on how close the knee
   // point is to its extreme values (min/max knee)
   const float tuning = 1.0f
      - smoothstep(knee_maximum, knee_default, target)
         * smoothstep(knee_minimum, knee_default, target); // the first smoothstep assumes that it works for a > b! This results overall in a bathtub like shape
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

// Precompute the allenwp tonemapping curve parameters for AgX on the CPU,
// matching the shader code (see allenwpCurve in FBShader.fxh / fs_pp_tonemap.sc).
// Derived from Godot's implementation (https://github.com/godotengine/godot/pull/106940)
// Source and details: https://allenwp.com/blog/2025/05/29/allenwp-tonemapping-curve/
void PrecompAgXTonemap(float out[4])
{
   const float output_max_value = 1.f; // SDR always has an output_max_value of 1.0

   // These constants must match the ones in the shader code.
   // 18% "middle gray" is perceptually 50% of the brightness of reference white.
   const float awp_crossover_point = 0.18f;
   // If output_max_value and/or awp_crossover_point are no longer constant,
   // awp_shoulder_max can be calculated on the CPU and passed in the shader parameters.
   const float awp_shoulder_max = output_max_value - awp_crossover_point;

   const float awp_contrast = 1.25f; // Approximately Blender's AgX contrast
   const float awp_high_clip = 16.29f; // Blender's AgX white point (must be >= 2.0 for good behavior)

   // awp_toe_a is a solution that ensures intersection at awp_crossover_point.
   const float awp_toe_a = ((1.f / awp_crossover_point) - 1.f) * powf(awp_crossover_point, awp_contrast);
   // Slope formula is simply the derivative of the toe function with an input of awp_crossover_point.
   const float awp_slope_denom = powf(awp_crossover_point, awp_contrast) + awp_toe_a;
   const float awp_slope = (awp_contrast * powf(awp_crossover_point, awp_contrast - 1.f) * awp_toe_a) / (awp_slope_denom * awp_slope_denom);

   float awp_w = awp_high_clip - awp_crossover_point;
   awp_w = awp_w * awp_w;
   awp_w = awp_w / awp_shoulder_max;
   awp_w = awp_w * awp_slope;

   // This here must match the order in the shader itself!
   out[0] = awp_contrast;
   out[1] = awp_toe_a;
   out[2] = awp_slope;
   out[3] = awp_w;
}


VPXRenderContext2D& Renderer::GetAncillaryRenderContext(VPXWindowId window, float width, float height, bool is2D, bool isOutputLinear, float depthbias, const Matrix3D& displayTransform)
{
   // Ancillary rendering is single threaded, on the main thread, therefore we store render context and state directly in a unique state object owned by the renderer
   m_ancillaryRenderSetup.isOutputLinear = isOutputLinear;
   m_ancillaryRenderSetup.depthbias = depthbias;
   m_ancillaryRenderContext.window = window;
   m_ancillaryRenderContext.is2D = is2D;
   m_ancillaryRenderContext.srcWidth = width;
   m_ancillaryRenderContext.srcHeight = height;
   m_ancillaryRenderContext.outWidth = width;
   m_ancillaryRenderContext.outHeight = height;
   m_ancillaryRenderSetup.displayTransform = displayTransform;
   m_ancillaryRenderSetup.contentBounds = vec4(FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX);
   return m_ancillaryRenderContext;
}

void Renderer::RenderAncillaryWindow(VPXWindowId window, const VPX::RenderOutput& output, RenderTarget* embedRT, const vector<AncillaryRendererDef>& ancillaryWndRenderers)
{
   const bool isAdjusted = g_pplayer->m_liveUI->m_inGameUI.IsOpened(s_displaySettingsPages[window]);
   const bool wasAdjusted = m_ancillaryWndAdjusted[window];

   if (output.GetMode() == VPX::RenderOutput::OM_WINDOW && !m_activeView->IsMirror())
   {
      // Dedicated ancillary outputs are presented without VSync synchronization, so they are throttled
      // to the refresh rate of their own display: rendering them at the playfield rate would only spend
      // CPU and GPU on frames the display can never show. Skipping before binding the swapchain also
      // skips its present for this frame. A swapped backbuffer forces an immediate render.
      // The render deadline advances by one display frame per render, and a render may come up to half a display frame early:
      // the window keeps its display rate on average (e.g. 60 fps along a 144 Hz playfield) and timing jitter does not skip frames
      // at equal rates. After a stall, the deadline restarts from now, and forced renders keep it at most 1.5 display frames ahead
      VPX::Window* const wnd = output.GetWindow();
      RenderTarget* const backBuffer = wnd != nullptr ? wnd->GetBackBuffer() : nullptr;
      const double refreshRate = wnd != nullptr ? wnd->GetRefreshRate() : 0.;
      const int64_t frameInterval = refreshRate > 0. ? static_cast<int64_t>(1000000. / refreshRate) : 0; // Unknown display rate: no throttling
      const int64_t now = static_cast<int64_t>(usec());
      int64_t& nextRenderTime = m_ancillaryWndNextRenderTime[window];
      if (!isAdjusted && !wasAdjusted && backBuffer != nullptr && backBuffer == m_ancillaryWndLastRT[window] && now + frameInterval / 2 < nextRenderTime)
         return;
      nextRenderTime = clamp(nextRenderTime + frameInterval, now + frameInterval / 2, now + frameInterval + frameInterval / 2);
      m_ancillaryWndLastRT[window] = backBuffer;
   }

   bool isOutputLinear;
   int m_outputX, m_outputY, m_outputW, m_outputH;
   RenderDevice* const rd = m_renderDevice;
   RenderTarget* outputRT = SetupAncillaryRenderTarget(window, output, embedRT, m_outputX, m_outputY, m_outputW, m_outputH, isOutputLinear);
   if (outputRT == nullptr)
      return;

   if (output.GetMode() == VPX::RenderOutput::OM_EMBEDDED)
   {
      // Remember the region so ApplyTonemapping can redo it without the table's color grade. The
      // content is rotated inside the [0..1] square before being mapped onto this rectangle, so
      // whatever the rotation, it stays within it
      m_embeddedRegions.push_back(vec4(static_cast<float>(m_outputX) / static_cast<float>(outputRT->GetWidth()), static_cast<float>(m_outputY) / static_cast<float>(outputRT->GetHeight()),
         static_cast<float>(m_outputX + m_outputW) / static_cast<float>(outputRT->GetWidth()), static_cast<float>(m_outputY + m_outputH) / static_cast<float>(outputRT->GetHeight())));

      // Keep the embedded ancillary content ordered after bloom: it writes the buffer that bloom
      // already sampled with this region cleared, so without this dependency the sorter may run it
      // before the bloom sample and bloom bleeds the content into the region. No-op without bloom
      rd->AddRenderTargetDependency(GetBloomBufferTexture());

      const vec4& region = m_embeddedRegions.back();
      rd->GetCurrentPass()->m_areaOfInterest = vec4(-1.f + 2.f * region.x, 1.f - 2.f * region.w, -1.f + 2.f * region.z, 1.f - 2.f * region.y);
   }

   rd->ResetRenderState();
   if (output.GetMode() == VPX::RenderOutput::OM_WINDOW)
      rd->Clear(clearType::TARGET | clearType::ZBUFFER, 0x00000000);

   bool rendered = false;
   // A quarter turn swaps what the renderers should lay out for, the projection then rotates it onto the output
   const int rotation = GetAncillaryContentRotation(window, output);
   const bool swapAxes = (rotation % 180) != 0;
   VPXRenderContext2D& context
      = GetAncillaryRenderContext(window, static_cast<float>(swapAxes ? m_outputH : m_outputW), static_cast<float>(swapAxes ? m_outputW : m_outputH), true, isOutputLinear, 0.f);
   for (auto& renderer : ancillaryWndRenderers)
   {
      rendered = renderer.Render(&context, renderer.context);
      if (rendered)
         break;
   }
   m_ancillaryWndRendered[window] = rendered;

   // Area used by the content, turned like it onto the output (to fit the output to it)
   if (vec4 bounds = m_ancillaryRenderSetup.contentBounds; rendered && bounds.z > bounds.x && bounds.w > bounds.y)
   {
      bounds = vec4(clamp(bounds.x, 0.f, 1.f), clamp(bounds.y, 0.f, 1.f), clamp(bounds.z, 0.f, 1.f), clamp(bounds.w, 0.f, 1.f));
      for (int i = 0; i < rotation / 90; i++) // A clockwise quarter turn of the [0..1] square: (x, y) -> (1 - y, x)
         bounds = vec4(1.f - bounds.w, bounds.x, 1.f - bounds.y, bounds.z);
      m_ancillaryContentBounds[window] = bounds;
   }
   else
      ResetAncillaryContentBounds(window);

   // Highlight the display selected in the in game UI with a colored frame, drawn on top of the window content.
   // This also shows the display when nothing is rendered (e.g. ScoreView without a matching display source)
   m_ancillaryWndAdjusted[window] = isAdjusted;
   if (isAdjusted)
   {
      constexpr int border = 10;
      constexpr float fr = 0.f, fg = 1.f, fb = 0.2f;
      rd->ResetRenderState();
      rd->SetRenderState(RenderState::ZENABLE, RenderState::RS_FALSE);
      DrawEmbeddedQuad(outputRT, m_outputX, m_outputY, m_outputW, border, fr, fg, fb); // Top
      DrawEmbeddedQuad(outputRT, m_outputX, m_outputY + m_outputH - border, m_outputW, border, fr, fg, fb); // Bottom
      DrawEmbeddedQuad(outputRT, m_outputX, m_outputY + border, border, m_outputH - 2 * border, fr, fg, fb); // Left
      DrawEmbeddedQuad(outputRT, m_outputX + m_outputW - border, m_outputY + border, border, m_outputH - 2 * border, fr, fg, fb); // Right
   }

   if (output.GetMode() == VPX::RenderOutput::OM_WINDOW)
   {
      // Drop the pass when nothing is rendered (avoids a useless clear), but clear once after adjusting to remove the highlight frame
      if (!rendered && !isAdjusted && !wasAdjusted)
      {
         m_renderDevice->GetCurrentPass()->ClearCommands();
      }
      else
      {
         VPX::Window* const wnd = output.GetWindow();
         // Only show ancillary outputs once the playfield window itself is visible: an ancillary window shown
         // (thereby activated) before the playfield's own show & raise can be demoted by the window manager
         // (minimized or left behind it), a state that would otherwise never be repaired (issue #4202).
         // A minimized window is only restored shortly after that show: one minimized later by the user stays minimized
         if (g_pplayer->m_playfieldWnd->IsVisible())
         {
            constexpr uint32_t demotionRepairDelay = 3000; // [ms]
            if (!wnd->IsVisible())
            {
               wnd->Show();
               m_ancillaryWndShowTime[window] = msec();
            }
            else if (wnd->IsMinimized() && (msec() - m_ancillaryWndShowTime[window]) < demotionRepairDelay)
               wnd->Show();
         }

         if (isOutputLinear)
         {
            assert(false); // This is disabled for the time being
            static std::array<string, 3> tonemapPassNames = { "Backglass Tonemap"s, "ScoreView Tonemap"s, "Topper Tonemap"s };
            const string& tonemapPassName = tonemapPassNames[window - VPXWindowId::VPXWINDOW_Backglass];
            const float jitter = (float)((msec() & 2047) / 1000.0);
            rd->ResetRenderState();
            rd->SetRenderState(RenderState::ZENABLE, RenderState::RS_FALSE);
            rd->SetRenderState(RenderState::CULLMODE, RenderState::CULL_NONE);
            rd->SetRenderTarget(tonemapPassName, outputRT, true, true);
            rd->AddRenderTargetDependency(m_ancillaryWndHdrRT[window].get(), false);
            rd->m_FBShader->SetTextureNull(ShaderUniform::tex_depth);
            rd->m_FBShader->SetTexture(ShaderUniform::tex_fb_unfiltered, m_ancillaryWndHdrRT[window]->GetColorSampler());
            rd->m_FBShader->SetTexture(ShaderUniform::tex_fb_filtered, m_ancillaryWndHdrRT[window]->GetColorSampler());
            rd->m_FBShader->SetVector(ShaderUniform::w_h_height, (float)(1.0 / m_ancillaryWndHdrRT[window]->GetWidth()), (float)(1.0 / m_ancillaryWndHdrRT[window]->GetHeight()), 1.0f, 1.0f);
            rd->m_FBShader->SetVector(ShaderUniform::bloom_dither_colorgrade,
               0.f, // Bloom
               output.GetWindow()->IsWCGBackBuffer() ? 0.f : 1.f, // Dither
               0.f, // LUT colorgrade
               0.f);
            rd->m_FBShader->SetVector(ShaderUniform::w_h_height, static_cast<float>(1.0 / static_cast<double>(m_outputW)), static_cast<float>(1.0 / static_cast<double>(m_outputH)),
               jitter, // radical_inverse(jittertime) * 11.0f,
               jitter); // sobol(jittertime) * 13.0f); // jitter for dither pattern}
            ShaderTechnique tonemapTechnique; // FIXME use a tonemapping corresponding to the output, handling situations where playfield is on a HDR display but backglass is not
            switch (m_toneMapper)
            {
            case TM_REINHARD: tonemapTechnique = ShaderTechnique::fb_rhtonemap_no_filter; break;
            case TM_FILMIC: tonemapTechnique = ShaderTechnique::fb_fmtonemap_no_filter; break;
            case TM_NEUTRAL: tonemapTechnique = ShaderTechnique::fb_nttonemap_no_filter; break;
            case TM_AGX: tonemapTechnique = ShaderTechnique::fb_agxtonemap_no_filter; break;
            default: assert(!"unknown tonemapper"); break;
            }
            rd->m_FBShader->SetTechnique(tonemapTechnique);
            constexpr float m_exposure = 1.f; // TODO implement exposure
            if (output.GetWindow()->IsWCGBackBuffer())
            {
               const float maxDisplayLuminance = output.GetWindow()->GetHDRHeadRoom()
                  * (output.GetWindow()->GetSDRWhitePoint() * 80.f); // Maximum luminance of display in nits, note that GetSDRWhitePoint()*80 should usually be in the 200 nits range
               rd->m_FBShader->SetVector(ShaderUniform::exposure_wcg, m_exposure,
                  (output.GetWindow()->GetSDRWhitePoint() * 80.f)
                     / maxDisplayLuminance, // Apply SDR whitepoint (1.0 -> white point in nits), then scale down by maximum luminance (in nits) of display to get a relative value before before tonemapping, equal to 1/GetHDRHeadRoom()
                  maxDisplayLuminance / 10000.f, // Apply back maximum luminance in nits of display after tonemapping, scaled down to PQ limits (1.0 is 10000 nits)
                  1.f);
               float spline_params[6];
               PrecompSplineTonemap(maxDisplayLuminance, spline_params);
               rd->m_FBShader->SetVector(ShaderUniform::spline1, spline_params[0], spline_params[1], spline_params[2], spline_params[3]);
               rd->m_FBShader->SetVector(ShaderUniform::spline2, spline_params[4], spline_params[5], 0.f, 0.f);
            }
            else
            {
               rd->m_FBShader->SetVector(ShaderUniform::exposure_wcg, m_exposure, 1.f,
                  0.f, // Unused for SDR
                  0.f); // Tonemapping mode: 0 = SDR
            }
            float agx_params[4];
            PrecompAgXTonemap(agx_params);
            rd->m_FBShader->SetVector(ShaderUniform::agx_params, agx_params[0], agx_params[1], agx_params[2], agx_params[3]);
            rd->DrawFullscreenTexturedQuad(rd->m_FBShader);
         }
      }
   }

   UpdateBasicShaderMatrix();
}

RenderProbe::ReflectionMode Renderer::GetMaxReflectionMode() const
{
   // For dynamic mode, static reflections are not available so adapt the mode
   return !IsUsingStaticPrepass() && m_maxReflectionMode >= RenderProbe::REFL_STATIC ? RenderProbe::REFL_DYNAMIC : m_maxReflectionMode;
}

void Renderer::SetPFReflectionMode(RenderProbe::ReflectionMode mode)
{
   if (m_maxReflectionMode == mode)
      return;
   m_maxReflectionMode = mode;
   MarkStaticsDirty(); // Static reflections are baked into the accumulated static prerender
}

void Renderer::SetAOMode(int mode)
{
   if (GetSelectedAOMode() == mode)
      return;
   m_disableAO = (mode == 0);
   m_dynamicAO = (mode == 2);
   MarkStaticsDirty(); // Static AO is baked into the accumulated static prerender
}

int Renderer::GetAOMode() const // 0=Off, 1=Static, 2=Dynamic
{
   // We must evaluate this dynamically since AO scale and enabled/disable can be changed from script
   if (m_disableAO || !m_table->m_enableAO || !m_renderDevice->DepthBufferReadBackAvailable() || m_table->m_AOScale == 0.f)
      return 0;
   // The existing implementation suffers from high temporal artefacts that make it unsuitable for dynamic camera situations
   if (m_stereo3D == STEREO_VR)
      return 0;
   if (m_dynamicAO)
      return 2;
   return IsUsingStaticPrepass() ? 1 : 0; // If AO is static prepass only, and we are running without it, disable AO
}
