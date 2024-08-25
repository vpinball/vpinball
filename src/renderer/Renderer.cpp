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

////////////////////////////////////////////////////////////////////////////////

Renderer::Renderer(PinTable* const table, VPX::Window* wnd, VideoSyncMode& syncMode, const StereoMode stereo3D)
   : m_table(table)
   , m_stereo3D(stereo3D)
   #if defined(ENABLE_DX9) // DirectX 9 does not support stereo rendering
   , m_stereo3DfakeStereo(true)
   #else
   , m_stereo3DfakeStereo(stereo3D == STEREO_VR ? false : table->m_settings.LoadValueWithDefault(Settings::Player, "Stereo3DFake"s, false))
   #endif
{
   m_stereo3Denabled = m_table->m_settings.LoadValueWithDefault(Settings::Player, "Stereo3DEnabled"s, (m_stereo3D != STEREO_OFF));
   m_BWrendering = m_table->m_settings.LoadValueWithDefault(Settings::Player, "BWRendering"s, 0);
   m_toneMapper = (ToneMapper)m_table->m_settings.LoadValueWithDefault(Settings::TableOverride, "ToneMapper"s, m_table->GetToneMapper());
   m_exposure = m_table->m_settings.LoadValueWithDefault(Settings::TableOverride, "Exposure"s, m_table->GetExposure());
   m_dynamicAO = m_table->m_settings.LoadValueWithDefault(Settings::Player, "DynamicAO"s, true);
   m_disableAO = m_table->m_settings.LoadValueWithDefault(Settings::Player, "DisableAO"s, false);
   m_vrPreview = (VRPreviewMode)m_table->m_settings.LoadValueWithDefault(Settings::PlayerVR, "VRPreview"s, (int)VRPREVIEW_LEFT);
   m_vrPreviewShrink = m_table->m_settings.LoadValueWithDefault(Settings::PlayerVR, "ShrinkPreview"s, false);
   m_FXAA = m_table->m_settings.LoadValueWithDefault(Settings::Player, "FXAA"s, (int)Disabled);
   m_sharpen = m_table->m_settings.LoadValueWithDefault(Settings::Player, "Sharpen"s, 0);
   m_ss_refl = m_table->m_settings.LoadValueWithDefault(Settings::Player, "SSRefl"s, false);
   m_bloomOff = m_table->m_settings.LoadValueWithDefault(Settings::Player, "ForceBloomOff"s, false);
   const int maxReflection = m_table->m_settings.LoadValueWithDefault(Settings::Player, "PFReflection"s, -1);
   if (maxReflection != -1)
      m_maxReflectionMode = (RenderProbe::ReflectionMode)maxReflection;
   else
   {
      m_maxReflectionMode = RenderProbe::REFL_STATIC;
      if (m_table->m_settings.LoadValueWithDefault(Settings::Player, "BallReflection"s, true))
         m_maxReflectionMode = RenderProbe::REFL_STATIC_N_BALLS;
      if (m_table->m_settings.LoadValueWithDefault(Settings::Player, "PFRefl"s, true))
         m_maxReflectionMode = RenderProbe::REFL_STATIC_N_DYNAMIC;
   }
   m_trailForBalls = m_table->m_settings.LoadValueWithDefault(Settings::Player, "BallTrail"s, true);
   m_ballTrailStrength = m_table->m_settings.LoadValueWithDefault(Settings::Player, "BallTrailStrength"s, 0.5f);
   m_ballImage = nullptr;
   m_decalImage = nullptr;
   m_overwriteBallImages = m_table->m_settings.LoadValueWithDefault(Settings::Player, "OverwriteBallImage"s, false);
   if (m_overwriteBallImages)
   {
      string imageName;
      bool hr = m_table->m_settings.LoadValue(Settings::Player, "BallImage"s, imageName);
      if (hr)
      {
         BaseTexture* const tex = BaseTexture::CreateFromFile(imageName, m_table->m_settings.LoadValueWithDefault(Settings::Player, "MaxTexDimension"s, 0));
         if (tex != nullptr)
            m_ballImage = new Texture(tex);
      }
      hr = m_table->m_settings.LoadValue(Settings::Player, "DecalImage"s, imageName);
      if (hr)
      {
         BaseTexture* const tex = BaseTexture::CreateFromFile(imageName, m_table->m_settings.LoadValueWithDefault(Settings::Player, "MaxTexDimension"s, 0));
         if (tex != nullptr)
            m_decalImage = new Texture(tex);
      }
   }

   // Global emission scale
   m_globalEmissionScale = m_table->m_globalEmissionScale;
   if (m_table->m_settings.LoadValueWithDefault(Settings::Player, "OverrideTableEmissionScale"s, false))
   { // Overriden from settings
      if (m_table->m_settings.LoadValueWithDefault(Settings::Player, "DynamicDayNight"s, false))
      {
         time_t hour_machine;
         time(&hour_machine);
         tm local_hour;
         localtime_s(&local_hour, &hour_machine);

         const float lat = m_table->m_settings.LoadValueWithDefault(Settings::Player, "Latitude"s, 52.52f);
         const float lon = m_table->m_settings.LoadValueWithDefault(Settings::Player, "Longitude"s, 13.37f);

         const double rlat = lat * (M_PI / 180.);
         const double rlong = lon * (M_PI / 180.);

         const double tr = TheoreticRadiation(local_hour.tm_mday, local_hour.tm_mon + 1, local_hour.tm_year + 1900, rlat);
         const double max_tr = MaxTheoreticRadiation(local_hour.tm_year + 1900, rlat);
         const double sset = SunsetSunriseLocalTime(local_hour.tm_mday, local_hour.tm_mon + 1, local_hour.tm_year + 1900, rlong, rlat, false);
         const double srise = SunsetSunriseLocalTime(local_hour.tm_mday, local_hour.tm_mon + 1, local_hour.tm_year + 1900, rlong, rlat, true);

         const double cur = local_hour.tm_hour + local_hour.tm_min / 60.0;

         const float factor = (float)(sin(M_PI * clamp((cur - srise) / (sset - srise), 0., 1.)) //!! leave space before sunrise and after sunset?
            * sqrt(tr / max_tr)); //!! magic, "emulates" that shorter days are usually also "darker",cloudier,whatever in most regions

         m_globalEmissionScale = clamp(factor, 0.15f, 1.f); //!! configurable clamp?
      }
      else
      {
         m_globalEmissionScale = m_table->m_settings.LoadValueWithDefault(Settings::Player, "EmissionScale"s, 0.5f);
      }
   }
   if (g_pvp->m_bgles)
   { // Overriden from command line
      m_globalEmissionScale = g_pvp->m_fgles;
   }

   m_mvp = new ModelViewProj(m_stereo3D == STEREO_OFF ? 1 : 2);

   #if defined(ENABLE_OPENGL)
   const int nMSAASamples = m_table->m_settings.LoadValueWithDefault(Settings::Player, "MSAASamples"s, 1);
   #elif defined(ENABLE_DX9) || defined(ENABLE_BGFX)
   // Sadly DX9 does not support resolving an MSAA depth buffer, making MSAA implementation complex for it. So just disable for now
   // BGFX MSAA is likely possible but not yet implemented
   constexpr int nMSAASamples = 1;
   #endif
   const bool useNvidiaApi = m_table->m_settings.LoadValueWithDefault(Settings::Player, "UseNVidiaAPI"s, false);
   const bool disableDWM = m_table->m_settings.LoadValueWithDefault(Settings::Player, "DisableDWM"s, false);
   const bool compressTextures = m_table->m_settings.LoadValueWithDefault(Settings::Player, "CompressTextures"s, false);
   int nEyes = (m_stereo3D == STEREO_VR || (m_stereo3D != STEREO_OFF && !m_stereo3DfakeStereo)) ? 2 : 1;
   try {
      m_renderDevice = new RenderDevice(wnd, m_stereo3D == STEREO_VR, nEyes, useNvidiaApi, disableDWM, compressTextures, m_BWrendering, nMSAASamples, syncMode);
   }
   catch (...) {
      // TODO better error handling => just let the exception up ?
      throw(E_FAIL);
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
      m_renderWidth = wnd->GetWidth() / 2;
      m_renderHeight = wnd->GetHeight();
   }
   else if (m_stereo3D == STEREO_TB || m_stereo3D == STEREO_INT || m_stereo3D == STEREO_FLIPPED_INT)
   {
      // Top/Bottom (and interlaced) fits the 2 views along the output height, so each view is rendered at half the output height
      m_renderWidth = wnd->GetWidth();
      m_renderHeight = wnd->GetHeight() / 2;
   }
   else
   {
      // Default renders at the output window pixel resolution
      m_renderWidth = wnd->GetWidth();
      m_renderHeight = wnd->GetHeight();
   }
   float AAfactor = m_table->m_settings.LoadValueWithDefault(Settings::Player, "AAFactor"s, m_table->m_settings.LoadValueWithDefault(Settings::Player, "USEAA"s, false) ? 2.0f : 1.0f);
   int renderWidthAA = (int)((float)m_renderWidth * AAfactor);
   int renderHeightAA = (int)((float)m_renderHeight * AAfactor);

   if ((m_renderDevice->GetOutputBackBuffer()->GetColorFormat() == colorFormat::RGBA10) && (m_FXAA == Quality_SMAA || m_FXAA == Standard_DLAA))
      ShowError("SMAA or DLAA post-processing AA should not be combined with 10bit-output rendering (will result in visible artifacts)!");

   #if defined(ENABLE_BGFX)
      const colorFormat renderFormat = ((m_BWrendering == 1) ? colorFormat::RG16F : ((m_BWrendering == 2) ? colorFormat::RED16F : colorFormat::RGB16F));
   #elif defined(ENABLE_OPENGL)
      #ifndef __OPENGLES__
         const colorFormat renderFormat = ((m_BWrendering == 1) ? colorFormat::RG16F : ((m_BWrendering == 2) ? colorFormat::RED16F : colorFormat::RGB16F));
      #else
         const colorFormat renderFormat = ((m_BWrendering == 1) ? colorFormat::RG16F : ((m_BWrendering == 2) ? colorFormat::RED16F : colorFormat::RGBA16F));
      #endif
   #elif defined(ENABLE_DX9)
      const colorFormat renderFormat = ((m_BWrendering == 1) ? colorFormat::RG16F : ((m_BWrendering == 2) ? colorFormat::RED16F : colorFormat::RGBA16F));
   #endif
   SurfaceType rtType = m_stereo3D == STEREO_OFF 
                     || (m_stereo3D != STEREO_VR && m_stereo3DfakeStereo)
                     || !m_renderDevice->SupportLayeredRendering() 
                      ? SurfaceType::RT_DEFAULT : SurfaceType::RT_STEREO;

   // MSAA render target which is resolved to the non MSAA render target
   if (nMSAASamples > 1) 
      m_pOffscreenMSAABackBufferTexture = new RenderTarget(m_renderDevice, rtType, "MSAABackBuffer"s, renderWidthAA, renderHeightAA, renderFormat, true, nMSAASamples, "Fatal Error: unable to create MSAA render buffer!");

   // Either the main render target for non MSAA, or the buffer where the MSAA render is resolved
   m_pOffscreenBackBufferTexture1 = new RenderTarget(m_renderDevice, rtType, "BackBuffer1"s, renderWidthAA, renderHeightAA, renderFormat, true, 1, "Fatal Error: unable to create offscreen back buffer");

   // Second render target to swap, allowing to read previous frame render for ball reflection and motion blur
   m_pOffscreenBackBufferTexture2 = m_pOffscreenBackBufferTexture1->Duplicate("BackBuffer2"s, true);

   // Initialize shaders
   m_renderDevice->m_basicShader->SetVector(SHADER_w_h_height, (float)(1.0 / (double)GetMSAABackBufferTexture()->GetWidth()), (float)(1.0 / (double)GetMSAABackBufferTexture()->GetHeight()), 0.0f, 0.0f);
   bool disableLightingForBalls = m_table->m_settings.LoadValueWithDefault(Settings::Player, "DisableLightingForBalls"s, false);
   m_renderDevice->m_ballShader->SetVector(SHADER_w_h_disableLighting,
      1.5f / (float)GetPreviousBackBufferTexture()->GetWidth(), // UV Offset for sampling reflections
      1.5f / (float)GetPreviousBackBufferTexture()->GetHeight(),
      disableLightingForBalls ? 1.f : 0.f, 0.f);

   #ifndef __STANDALONE__
      BAMView::init();
   #endif

   if (m_stereo3D == STEREO_VR)
      m_backGlass = new BackGlass(m_renderDevice, m_table->GetDecalsEnabled() ? m_table->GetImage(m_table->m_BG_image[m_table->m_BG_current_set]) : nullptr);
   else
      m_backGlass = nullptr;

   #if defined(ENABLE_VR)
   if (m_stereo3D == STEREO_VR) {
      //AMD Debugging
      colorFormat renderBufferFormatVR;
      const int textureModeVR = g_pplayer->m_ptable->m_settings.LoadValueWithDefault(Settings::PlayerVR, "EyeFBFormat"s, 1);
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

   // This used to be a spheremap BMP, upgraded in 10.8 for an equirectangular HDR env map
   m_pinballEnvTexture.LoadFromFile(g_pvp->m_szMyPath + "assets" + PATH_SEPARATOR_CHAR + "BallEnv.exr");
   m_aoDitherTexture.LoadFromFile(g_pvp->m_szMyPath + "assets" + PATH_SEPARATOR_CHAR + "AODither.webp");
   m_builtinEnvTexture.LoadFromFile(g_pvp->m_szMyPath + "assets" + PATH_SEPARATOR_CHAR + "EnvMap.webp");
   m_envTexture = m_table->GetImage(m_table->m_envImage);
   PLOGI << "Computing environment map radiance"; // For profiling

   #if defined(ENABLE_OPENGL) && !defined(__OPENGLES__) // Compute radiance on the GPU
   // TODO implement for BGFX
   Texture* const envTex = m_envTexture ? m_envTexture : &m_builtinEnvTexture;
   const int envTexHeight = min(envTex->m_pdsBuffer->height(), 256u) / 8;
   const int envTexWidth = envTexHeight * 2;
   const colorFormat rad_format = envTex->m_pdsBuffer->m_format == BaseTexture::RGB_FP32 ? colorFormat::RGBA32F : colorFormat::RGBA16F;
   m_envRadianceTexture = new RenderTarget(m_renderDevice, SurfaceType::RT_DEFAULT, "Irradiance"s, envTexWidth, envTexHeight, rad_format, false, 1, "Failed to create irradiance render target");
   m_renderDevice->m_FBShader->SetTechnique(SHADER_TECHNIQUE_irradiance);
   m_renderDevice->m_FBShader->SetTexture(SHADER_tex_env, envTex);
   m_renderDevice->SetRenderTarget("Env Irradiance PreCalc"s, m_envRadianceTexture);
   m_renderDevice->DrawFullscreenTexturedQuad(m_renderDevice->m_FBShader);
   m_renderDevice->SubmitRenderFrame(); // Force submission as result users do not explicitly declare the dependency on this pass
   m_renderDevice->m_basicShader->SetTexture(SHADER_tex_diffuse_env, m_envRadianceTexture->GetColorSampler());
   m_renderDevice->m_ballShader->SetTexture(SHADER_tex_diffuse_env, m_envRadianceTexture->GetColorSampler());

   #else // DirectX 9 does not support bitwise operation in shader, so radical_inverse is not implemented and therefore we use the slow CPU path instead of GPU
   const Texture* const envTex = m_envTexture ? m_envTexture : &m_builtinEnvTexture;
   const unsigned int envTexHeight = min(envTex->m_pdsBuffer->height(), 256u) / 8;
   const unsigned int envTexWidth = envTexHeight * 2;
   m_envRadianceTexture = EnvmapPrecalc(envTex, envTexWidth, envTexHeight);
   m_renderDevice->m_texMan.SetDirty(m_envRadianceTexture);
   m_renderDevice->m_basicShader->SetTexture(SHADER_tex_diffuse_env, m_envRadianceTexture);
   m_renderDevice->m_ballShader->SetTexture(SHADER_tex_diffuse_env, m_envRadianceTexture);
   #endif
   PLOGI << "Environment map radiance computed"; // For profiling

   const bool lowDetailBall = (m_table->GetDetailLevel() < 10);
   IndexBuffer* ballIndexBuffer
      = new IndexBuffer(m_renderDevice, lowDetailBall ? basicBallLoNumFaces : basicBallMidNumFaces, lowDetailBall ? basicBallLoIndices : basicBallMidIndices);
   VertexBuffer* ballVertexBuffer
      = new VertexBuffer(m_renderDevice, lowDetailBall ? basicBallLoNumVertices : basicBallMidNumVertices, (float*)(lowDetailBall ? basicBallLo : basicBallMid));
   m_ballMeshBuffer = new MeshBuffer(L"Ball"s, ballVertexBuffer, ballIndexBuffer, true);
#ifdef DEBUG_BALL_SPIN
   {
      vector<Vertex3D_NoTex2> ballDbgVtx;
      for (int j = -1; j <= 1; ++j)
      {
         const int numPts = (j == 0) ? 6 : 3;
         const float theta = (float)(j * (M_PI / 4.0));
         for (int i = 0; i < numPts; ++i)
         {
            const float phi = (float)(i * (2.0 * M_PI) / numPts);
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

      VertexBuffer* ballDebugPoints = new VertexBuffer(m_renderDevice, (unsigned int)ballDbgVtx.size(), (float*)ballDbgVtx.data(), false);
      m_ballDebugPoints = new MeshBuffer(L"Ball.Debug"s, ballDebugPoints);
   }
#endif
   // Support up to 64 balls, that should be sufficient
   VertexBuffer* ballTrailVertexBuffer = new VertexBuffer(m_renderDevice, 64 * (MAX_BALL_TRAIL_POS - 2) * 2 + 4, nullptr, true);
   m_ballTrailMeshBuffer = new MeshBuffer(L"Ball.Trail"s, ballTrailVertexBuffer);

   // TODO we always loads the LUT since this can be changed in the LiveUI. Would be better to do this lazily
   //if (m_toneMapper == TM_TONY_MC_MAPFACE)
   {
      m_tonemapLUT = new Texture();
      m_tonemapLUT->LoadFromFile(g_pvp->m_szMyPath + "assets" + PATH_SEPARATOR_CHAR + "tony_mc_mapface_unrolled.exr");
      m_renderDevice->m_texMan.LoadTexture(m_tonemapLUT->m_pdsBuffer, SF_BILINEAR, SA_CLAMP, SA_CLAMP, true)->SetName("TonyMcMapFaceLUT");
      m_renderDevice->m_FBShader->SetTexture(SHADER_tex_tonemap_lut, m_tonemapLUT, SF_BILINEAR, SA_CLAMP, SA_CLAMP, true);
   }

   // Cache DMD renderer properties
   {
      const int dmdProfile = m_table->m_settings.LoadValueWithDefault(Settings::DMD, "RenderProfile"s, 0);
      const string prefix = "User."s + std::to_string(dmdProfile + 1) + "."s;
      // DMD View
      m_dmdViewExposure = m_table->m_settings.LoadValueWithDefault(Settings::DMD, "Exposure"s, 1.f);
      m_dmdViewDot = convertColor(
         m_table->m_settings.LoadValueWithDefault(Settings::DMD, prefix + "DotTint"s, 0x0088BBFF),
         m_table->m_settings.LoadValueWithDefault(Settings::DMD, prefix + "DotBrightness"s, 1.0f));
      // DMD Renderer
      m_dmdUseNewRenderer = m_table->m_settings.LoadValueWithDefault(Settings::DMD, prefix + "Legacy"s, false);
      #if !defined(ENABLE_BGFX)
         m_dmdUseNewRenderer = false; // Only available for BGFX
      #endif
      m_dmdDotProperties.x = m_table->m_settings.LoadValueWithDefault(Settings::DMD, prefix + "DotSize"s, 0.85f);
      m_dmdDotProperties.y = m_table->m_settings.LoadValueWithDefault(Settings::DMD, prefix + "DotSharpness"s, 0.8f);
      m_dmdDotProperties.z = m_table->m_settings.LoadValueWithDefault(Settings::DMD, prefix + "DotRounding"s, 0.85f);
      m_dmdDotProperties.w = m_table->m_settings.LoadValueWithDefault(Settings::DMD, prefix + "DotGlow"s, 0.3f);
      m_dmdUnlitDotColor = convertColor(
         m_table->m_settings.LoadValueWithDefault(Settings::DMD, prefix + "UnlitDotColor"s, 0x00020202),
         m_table->m_settings.LoadValueWithDefault(Settings::DMD, prefix + "BackGlow"s, 0.4f));
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
   m_pinballEnvTexture.FreeStuff();
   m_builtinEnvTexture.FreeStuff();
   m_aoDitherTexture.FreeStuff();
   delete m_ballImage;
   delete m_decalImage;
   delete m_envRadianceTexture;
   delete m_backGlass;
   delete m_ballMeshBuffer;
   #ifdef DEBUG_BALL_SPIN
   delete m_ballDebugPoints;
   #endif
   delete m_ballTrailMeshBuffer;
   delete m_ballImage;
   delete m_decalImage;
   delete m_tonemapLUT;
   delete m_staticPrepassRT;
   delete m_pOffscreenBackBufferTexture1;
   delete m_pOffscreenBackBufferTexture2;
   delete m_pBloomBufferTexture;
   delete m_pBloomTmpBufferTexture;
   delete m_pPostProcessRenderTarget1;
   delete m_pPostProcessRenderTarget2;
   delete m_pReflectionBufferTexture;
   delete m_pOffscreenVRLeft;
   delete m_pOffscreenVRRight;
   ReleaseAORenderTargets();
   delete m_renderDevice;
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
   // Lazily alloc buffer for screen space fake reflection rendering
   if (m_pReflectionBufferTexture == nullptr)
   {
      m_pReflectionBufferTexture = new RenderTarget(m_renderDevice, 
         GetBackBufferTexture()->m_type, "ReflectionBuffer"s, 
         GetBackBufferTexture()->GetWidth(),
         GetBackBufferTexture()->GetHeight(),
         GetBackBufferTexture()->GetColorFormat(),
         false, 1, "Fatal Error: unable to create reflection buffer!");
   }
   return m_pReflectionBufferTexture;
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

void Renderer::TransformVertices(const Vertex3D_NoTex2 * const __restrict rgv, const WORD * const __restrict rgi, const int count, Vertex2D * const __restrict rgvout) const
{
   RECT viewport { 0, 0, (LONG)m_renderDevice->GetOutputBackBuffer()->GetWidth(), (LONG)m_renderDevice->GetOutputBackBuffer()->GetHeight() };
   m_mvp->GetModelViewProj(0).TransformVertices(rgv, rgi, count, rgvout, viewport);
}

void Renderer::TransformVertices(const Vertex3Ds* const __restrict rgv, const WORD* const __restrict rgi, const int count, Vertex2D* const __restrict rgvout) const
{
   RECT viewport { 0, 0, (LONG)m_renderDevice->GetOutputBackBuffer()->GetWidth(), (LONG)m_renderDevice->GetOutputBackBuffer()->GetHeight() };
   m_mvp->GetModelViewProj(0).TransformVertices(rgv, rgi, count, rgvout, viewport);
}

BaseTexture* Renderer::EnvmapPrecalc(const Texture* envTex, const unsigned int rad_env_xres, const unsigned int rad_env_yres)
{
   const void* __restrict envmap = envTex->m_pdsBuffer->data();
   const unsigned int env_xres = envTex->m_pdsBuffer->width();
   const unsigned int env_yres = envTex->m_pdsBuffer->height();
   BaseTexture::Format env_format = envTex->m_pdsBuffer->m_format;
   const BaseTexture::Format rad_format = (env_format == BaseTexture::RGB_FP16 || env_format == BaseTexture::RGB_FP32) ? env_format : BaseTexture::SRGB;
   BaseTexture* radTex = new BaseTexture(rad_env_xres, rad_env_yres, rad_format);
   BYTE* const __restrict rad_envmap = radTex->data();
   bool free_envmap = false;

#define PREFILTER_ENVMAP_DIFFUSE
#ifdef PREFILTER_ENVMAP_DIFFUSE
   // pre-filter envmap with a gauss (separable/two passes: x and y)
   //!!! not correct to pre-filter like this, but barely visible in the end, and helps to keep number of samples low (otherwise one would have to use >64k samples instead of 4k!)
   if ((env_format == BaseTexture::RGB_FP16 || env_format == BaseTexture::RGB_FP32) && env_xres > 64)
   {
      const float scale_factor = (float)env_xres*(float)(1.0 / 64.);
      const int xs = (int)(scale_factor*0.5f + 0.5f);
      const void* const __restrict envmap2 = malloc(env_xres * env_yres * (3 * 4));
      const void* const __restrict envmap3 = malloc(env_xres * env_yres * (3 * 4));
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
                  sum_r += half2float(((unsigned short*)envmap)[offs    ]) * w;
                  sum_g += half2float(((unsigned short*)envmap)[offs + 1]) * w;
                  sum_b += half2float(((unsigned short*)envmap)[offs + 2]) * w;
               }
               else
               {
                  sum_r += ((float*)envmap)[offs    ] * w;
                  sum_g += ((float*)envmap)[offs + 1] * w;
                  sum_b += ((float*)envmap)[offs + 2] * w;
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
      free((void*)envmap2);
      free(weights);
      free_envmap = true;
   }
#endif

   // brute force sampling over hemisphere for each normal direction of the to-be-(ir)radiance-baked environment
   // not the fastest solution, could do a "cosine convolution" over the picture instead (where also just 1024 or x samples could be used per pixel)
   //!! (note though that even 4096 samples can be too low if very bright spots (i.e. sun) in the image! see Delta_2k.hdr -> thus pre-filter enabled above!)
   // but with this implementation one can also have custom maps/LUTs for glossy, etc. later-on
   {
      ThreadPool pool(g_pvp->m_logicalNumberOfProcessors);

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
                  if (env_format == BaseTexture::RGB_FP16)
                  {
                     r = half2float(((unsigned short*)envmap)[offs*3  ]);
                     g = half2float(((unsigned short*)envmap)[offs*3+1]);
                     b = half2float(((unsigned short*)envmap)[offs*3+2]);
                  }
                  else if (env_format == BaseTexture::RGBA_FP16)
                  {
                     r = half2float(((unsigned short*)envmap)[offs*4  ]);
                     g = half2float(((unsigned short*)envmap)[offs*4+1]);
                     b = half2float(((unsigned short*)envmap)[offs*4+2]);
                  }
                  else if (env_format == BaseTexture::RGB_FP32)
                  {
                     r = ((float*)envmap)[offs*3  ];
                     g = ((float*)envmap)[offs*3+1];
                     b = ((float*)envmap)[offs*3+2];
                  }
                  else if (env_format == BaseTexture::RGB)
                  {
                     r = (float)((BYTE*)envmap)[offs*3  ] * (float)(1.0 / 255.0);
                     g = (float)((BYTE*)envmap)[offs*3+1] * (float)(1.0 / 255.0);
                     b = (float)((BYTE*)envmap)[offs*3+2] * (float)(1.0 / 255.0);
                  }
                  else if (env_format == BaseTexture::RGBA)
                  {
                     const DWORD rgb = ((DWORD*)envmap)[offs];
                     r = (float)(rgb & 0x00FF0000) * (float)(1.0 / 16711680.0);
                     g = (float)(rgb & 0x0000FF00) * (float)(1.0 /    65280.0);
                     b = (float)(rgb & 0x000000FF) * (float)(1.0 /      255.0);
                  }
                  else if (env_format == BaseTexture::SRGB)
                  {
                     r = invGammaApprox((float)((BYTE*)envmap)[offs*3  ] * (float)(1.0 / 255.0));
                     g = invGammaApprox((float)((BYTE*)envmap)[offs*3+1] * (float)(1.0 / 255.0));
                     b = invGammaApprox((float)((BYTE*)envmap)[offs*3+2] * (float)(1.0 / 255.0));
                  }
                  else if (env_format == BaseTexture::SRGBA)
                  {
                     const DWORD rgb = ((DWORD*)envmap)[offs];
                     r = invGammaApprox((float)(rgb & 0x00FF0000) * (float)(1.0 / 16711680.0));
                     g = invGammaApprox((float)(rgb & 0x0000FF00) * (float)(1.0 /    65280.0));
                     b = invGammaApprox((float)(rgb & 0x000000FF) * (float)(1.0 /      255.0));
                  }
                  else
                     assert(!"unknown format");
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
               if (rad_format == BaseTexture::RGB_FP16)
               {
                  ((unsigned short*)rad_envmap)[offs  ] = float2half_noLUT(sum_r);
                  ((unsigned short*)rad_envmap)[offs+1] = float2half_noLUT(sum_g);
                  ((unsigned short*)rad_envmap)[offs+2] = float2half_noLUT(sum_b);
               }
               else if (rad_format == BaseTexture::RGB_FP32)
               {
                  ((float*)rad_envmap)[offs  ] = sum_r;
                  ((float*)rad_envmap)[offs+1] = sum_g;
                  ((float*)rad_envmap)[offs+2] = sum_b;
               }
               else if (rad_format == BaseTexture::SRGB)
               {
                  rad_envmap[offs  ] = (int)clamp(gammaApprox(sum_r) * 255.f, 0.f, 255.f);
                  rad_envmap[offs+1] = (int)clamp(gammaApprox(sum_g) * 255.f, 0.f, 255.f);
                  rad_envmap[offs+2] = (int)clamp(gammaApprox(sum_b) * 255.f, 0.f, 255.f);
               }
            }
         });
      }

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
               const DWORD rgb = ((DWORD*)envmap)[offs];
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
                ((DWORD*)rad_envmap)[y*rad_env_xres + x] != ((int)(sum[0] * 255.0f)) | (((int)(sum[1] * 255.0f)) << 8) | (((int)(sum[2] * 255.0f)) << 16))
                g_pvp->MessageBox("Not OK", "Not OK", MB_OK);
         }
      }

   ///!!! */

#ifdef PREFILTER_ENVMAP_DIFFUSE
   if (free_envmap)
      free((void*)envmap);
#endif

#ifdef __OPENGLES__
   if (radTex->m_format == BaseTexture::SRGB || radTex->m_format == BaseTexture::RGB_FP16) {
      radTex->AddAlpha();
   }
#endif

   return radTex;
}

void Renderer::DrawBackground()
{
   const PinTable * const ptable = g_pplayer->m_ptable;
   Texture * const pin = ptable->GetDecalsEnabled() ? ptable->GetImage(ptable->m_BG_image[ptable->m_BG_current_set]) : nullptr;
   m_renderDevice->ResetRenderState();
   m_renderDevice->SetRenderState(RenderState::CULLMODE, RenderState::CULL_CCW);
   if (pin)
   {
      m_renderDevice->Clear(clearType::ZBUFFER, 0, 1.0f, 0L);
      m_renderDevice->SetRenderState(RenderState::ZWRITEENABLE, RenderState::RS_FALSE);
      m_renderDevice->SetRenderState(RenderState::ZENABLE, RenderState::RS_FALSE);
      m_renderDevice->SetRenderState(RenderState::ALPHABLENDENABLE, RenderState::RS_FALSE);
      // FIXME this should be called with a trilinear/anisotropy filtering override
      g_pplayer->m_renderer->DrawSprite(0.f, 0.f, 1.f, 1.f, 0xFFFFFFFF, pin, ptable->m_ImageBackdropNightDay ? sqrtf(m_globalEmissionScale) : 1.0f, true);
   }
   else
   {
      const D3DCOLOR d3dcolor = COLORREF_to_D3DCOLOR(ptable->m_colorbackdrop);
      m_renderDevice->Clear(clearType::TARGET | clearType::ZBUFFER, d3dcolor, 1.0f, 0L);
   }
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
   ViewSetup& viewSetup = m_table->mViewSetups[m_table->m_BG_current_set];
   #if defined(ENABLE_OPENGL) || defined(ENABLE_BGFX)
   bool stereo = m_stereo3D != STEREO_OFF && m_stereo3D != STEREO_VR && m_stereo3Denabled;
   #elif defined(ENABLE_DX9)
   bool stereo = false;
   #endif
   viewSetup.ComputeMVP(m_table, 
      (float)((double)GetBackBufferTexture()->GetWidth() / (double)GetBackBufferTexture()->GetHeight()),
      stereo, *m_mvp, vec3(m_cam.x, m_cam.y, m_cam.z), m_inc,
      xpixoff / (float)GetBackBufferTexture()->GetWidth(), ypixoff / (float)GetBackBufferTexture()->GetHeight());
   SetupShaders();
}

Vertex3Ds Renderer::Unproject(const Vertex3Ds& point)
{
   Matrix3D invMVP = m_mvp->GetModelViewProj(0);
   invMVP.Invert();
   const Vertex3Ds p(
             2.0f * point.x / (float)m_renderDevice->GetOutputBackBuffer()->GetWidth()  - 1.0f,
      1.0f - 2.0f * point.y / (float)m_renderDevice->GetOutputBackBuffer()->GetHeight(),
      (point.z - 0.f /* MinZ */) / (1.f /* MaxZ */ - 0.f /* MinZ */));
   return invMVP * p;
}

Vertex3Ds Renderer::Get3DPointFrom2D(const POINT& p)
{
   const Vertex3Ds pNear((float)p.x, (float)p.y, 0.f /* MinZ */);
   const Vertex3Ds pFar ((float)p.x, (float)p.y, 1.f /* MaxZ */);
   const Vertex3Ds p1 = Unproject(pNear);
   const Vertex3Ds p2 = Unproject(pFar);
   constexpr float wz = 0.f;
   const float wx = ((wz - p1.z)*(p2.x - p1.x)) / (p2.z - p1.z) + p1.x;
   const float wy = ((wz - p1.z)*(p2.y - p1.y)) / (p2.z - p1.z) + p1.y;
   return {wx, wy, wz};
}

void Renderer::SetupShaders()
{
   const vec4 envEmissionScale_TexWidth(m_table->m_envEmissionScale * m_globalEmissionScale,
      (float) (m_envTexture ? *m_envTexture : m_builtinEnvTexture).m_height /*+m_builtinEnvTexture.m_width)*0.5f*/, 0.f, 0.f); //!! dto.

   UpdateBasicShaderMatrix();
   m_renderDevice->m_basicShader->SetTexture(SHADER_tex_env, m_envTexture ? m_envTexture : &m_builtinEnvTexture);
   m_renderDevice->m_basicShader->SetVector(SHADER_fenvEmissionScale_TexWidth, &envEmissionScale_TexWidth);

   UpdateBallShaderMatrix();
   const vec4 st(m_table->m_envEmissionScale * m_globalEmissionScale, m_envTexture ? (float)m_envTexture->m_height/*+m_envTexture->m_width)*0.5f*/ : (float)m_builtinEnvTexture.m_height/*+m_builtinEnvTexture.m_width)*0.5f*/, 0.f, 0.f);
   m_renderDevice->m_ballShader->SetVector(SHADER_fenvEmissionScale_TexWidth, &st);
   m_renderDevice->m_ballShader->SetVector(SHADER_fenvEmissionScale_TexWidth, &envEmissionScale_TexWidth);
   //m_renderDevice->m_ballShader->SetInt("iLightPointNum",MAX_LIGHT_SOURCES);

   constexpr float Roughness = 0.8f;
   m_renderDevice->m_ballShader->SetVector(SHADER_Roughness_WrapL_Edge_Thickness, exp2f(10.0f * Roughness + 1.0f), 0.f, 1.f, 0.05f);
   vec4 amb_lr = convertColor(m_table->m_lightAmbient, m_table->m_lightRange);
   m_renderDevice->m_ballShader->SetVector(SHADER_cAmbient_LightRange, 
      amb_lr.x * m_globalEmissionScale, amb_lr.y * m_globalEmissionScale, amb_lr.z * m_globalEmissionScale, m_table->m_lightRange);

   m_renderDevice->m_FBShader->SetFloat(SHADER_exposure, m_exposure);

   //m_renderDevice->m_basicShader->SetInt("iLightPointNum",MAX_LIGHT_SOURCES);

   m_table->m_Light[0].pos.x = m_table->m_right * 0.5f;
   m_table->m_Light[1].pos.x = m_table->m_right * 0.5f;
   m_table->m_Light[0].pos.y = m_table->m_bottom * (float)(1.0 / 3.0);
   m_table->m_Light[1].pos.y = m_table->m_bottom * (float)(2.0 / 3.0);
   m_table->m_Light[0].pos.z = m_table->m_lightHeight;
   m_table->m_Light[1].pos.z = m_table->m_lightHeight;

   vec4 emission = convertColor(m_table->m_Light[0].emission);
   emission.x *= m_table->m_lightEmissionScale * m_globalEmissionScale;
   emission.y *= m_table->m_lightEmissionScale * m_globalEmissionScale;
   emission.z *= m_table->m_lightEmissionScale * m_globalEmissionScale;

#if defined(ENABLE_OPENGL) || defined(ENABLE_BGFX)
   float lightPos[MAX_LIGHT_SOURCES][4] = { 0.f };
   float lightEmission[MAX_LIGHT_SOURCES][4] = { 0.f };

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

void Renderer::UpdateStereoShaderState()
{
   if (m_stereo3DfakeStereo)
   {
      // FIXME compute max separation and zero point depth for fake stereo corresponding to the ones of the real stereo, remove these from settings and table properties
      // The problem with the legacy parameter is that both depends on the camera and do not match any physicaly correct measure. Authors tweak it but it will break at the
      // next depth buffer change (due to difference between rendering API or for example if we switch to reversed or infinite buffer for better precision, ...)
      // The idea would be (to be checked against shader implementation and ViewSetup projection maths):
      // - Max separation is the separation of a point with a very high depth (compute it from eye separation which is physically measures, and near/far planes)
      // - ZPD is the depth at which separation is 0 (compute it from the zNullSeparation in ViewSetup)
      /*ModelViewProj stereoMVP;
      m_table->mViewSetups[m_table->m_BG_current_set].ComputeMVP(m_ptable, Width, Height, true, stereoMVP);
      RECT viewport { 0, 0, (LONG)Width, (LONG)Height };
      vec3 deepPt(0.f, 0.f, 0.f); // = 5000.f * stereoMVP.GetModelViewInverse().GetOrthoNormalDir();
      Vertex2D projLeft, projRight;
      stereoMVP.GetModelViewProj(0).TransformVertices(&deepPt, nullptr, 1, &projLeft, viewport);
      stereoMVP.GetModelViewProj(1).TransformVertices(&deepPt, nullptr, 1, &projRight, viewport);*/
      const float eyeSeparation = m_table->GetMaxSeparation();
      const float zpd = m_table->GetZPD();
      const bool swapAxis = m_table->m_settings.LoadValueWithDefault(Settings::Player, "Stereo3DYAxis"s, false); // Swap X/Y axis
      m_renderDevice->m_stereoShader->SetVector(SHADER_Stereo_MS_ZPD_YAxis, eyeSeparation, zpd, swapAxis ? 1.0f : 0.0f, 0.0f);
   }
   
   RenderTarget *renderedRT = GetPostProcessRenderTarget1();
   if (m_stereo3DfakeStereo)
      m_renderDevice->m_stereoShader->SetVector(SHADER_w_h_height, (float)(1.0 / renderedRT->GetWidth()), (float)(1.0 / renderedRT->GetHeight()), (float)renderedRT->GetHeight(), m_table->Get3DOffset());

   m_stereo3DDefocus = 0.f;
   if (IsAnaglyphStereoMode(m_stereo3D))
   {
      Anaglyph anaglyph;
      anaglyph.LoadSetupFromRegistry(clamp(m_stereo3D - STEREO_ANAGLYPH_1, 0, 9));
      anaglyph.SetupShader(m_renderDevice->m_stereoShader);
      // The defocus kernel size should depend on the render resolution but since this is a user tweak, this doesn't matter that much
      m_stereo3DDefocus = m_table->m_settings.LoadValueWithDefault(Settings::Player, "Stereo3DDefocus"s, 0.f);
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

void Renderer::RenderFrame()
{
   // Keep previous render as a reflection probe for ball reflection and for hires motion blur
   SwapBackBufferRenderTargets();

   // Update camera point of view
   #ifdef ENABLE_VR
   if (m_stereo3D == STEREO_VR)
      g_pplayer->m_vrDevice->UpdateVRPosition(GetMVP());
   else 
   #endif
   // Legacy headtracking (to be moved to a plugin, using plugin API to update camera)
   if (g_pplayer->m_headTracking)
   {
      Matrix3D m_matView;
      Matrix3D m_matProj[2];
      #ifndef __STANDALONE__
      BAMView::createProjectionAndViewMatrix(&m_matProj[0]._11, &m_matView._11);
      #endif
      m_mvp->SetView(m_matView);
      for (unsigned int eye = 0; eye < m_mvp->m_nEyes; eye++)
         m_mvp->SetProj(eye, m_matProj[eye]);
   }

   // Reinitialize parts that have been modified
   for (auto renderable : m_renderableToInit)
   {
      renderable->RenderRelease();
      renderable->RenderSetup(m_renderDevice);
   }
   m_renderableToInit.clear();

   // Update staticly prerendered parts if needed
   RenderStaticPrepass();

   // Start from the prerendered parts/background or a clear background for VR
   m_renderDevice->SetRenderTarget("Render Scene"s, GetMSAABackBufferTexture());
   if (m_stereo3D == STEREO_VR || g_pplayer->GetInfoMode() == IF_DYNAMIC_ONLY)
      m_renderDevice->Clear(clearType::TARGET | clearType::ZBUFFER, 0, 1.0f, 0L);
   else
   {
      m_renderDevice->AddRenderTargetDependency(m_staticPrepassRT);
      m_renderDevice->BlitRenderTarget(m_staticPrepassRT, GetMSAABackBufferTexture());
   }

   // Setup ball rendering: collect all lights that can reflect on balls
   m_ballTrailMeshBufferPos = 0;
   m_ballReflectedLights.clear();
   for (size_t i = 0; i < m_table->m_vedit.size(); i++)
   {
      IEditable* const item = m_table->m_vedit[i];
      if (item && item->GetItemType() == eItemLight && ((Light*)item)->m_d.m_showReflectionOnBall)
         m_ballReflectedLights.push_back((Light*)item);
   }
   // We don't need to set the dependency on the previous frame render as this would be a cross frame dependency which does not have any meaning since dependencies are resolved per frame
   // m_renderDevice->AddRenderTargetDependency(m_renderDevice->GetPreviousBackBufferTexture());
   m_renderDevice->m_ballShader->SetTexture(SHADER_tex_ball_playfield, GetPreviousBackBufferTexture()->GetColorSampler());

   if (g_pplayer->GetInfoMode() != IF_STATIC_ONLY)
      RenderDynamics();

   // Resolve MSAA buffer to a normal one (noop if not using MSAA), allowing sampling it for postprocessing
   if (GetMSAABackBufferTexture() != GetBackBufferTexture())
   {
      RenderPass* const initial_rt = m_renderDevice->GetCurrentPass();
      m_renderDevice->SetRenderTarget("Resolve MSAA"s, GetBackBufferTexture());
      m_renderDevice->BlitRenderTarget(GetMSAABackBufferTexture(), GetBackBufferTexture(), true, true);
      m_renderDevice->SetRenderTarget(initial_rt->m_name, initial_rt->m_rt);
      initial_rt->m_name += '-';
   }

   PrepareVideoBuffers();
}

void Renderer::RenderDMD(BaseTexture* dmd, RenderTarget* rt)
{
   m_renderDevice->ResetRenderState();
   m_renderDevice->SetRenderState(RenderState::ALPHABLENDENABLE, RenderState::RS_FALSE);
   m_renderDevice->SetRenderState(RenderState::CULLMODE, RenderState::CULL_NONE);
   m_renderDevice->SetRenderState(RenderState::ZWRITEENABLE, RenderState::RS_FALSE);
   m_renderDevice->SetRenderState(RenderState::ZENABLE, RenderState::RS_FALSE);
   m_renderDevice->SetRenderTarget("DMDView", rt, false);
   SetupDMDRender(m_dmdViewDot, dmd, 1.f, true);
   m_renderDevice->m_DMDShader->SetFloat(SHADER_exposure, m_dmdViewExposure);
   Vertex3D_NoTex2 vertices[4] = {
      {  1.f, -1.f, 0.f, 0.f, 0.f, 1.f, 1.f, 1.f },
      { -1.f, -1.f, 0.f, 0.f, 0.f, 1.f, 0.f, 1.f },
      {  1.f,  1.f, 0.f, 0.f, 0.f, 1.f, 1.f, 0.f },
      { -1.f,  1.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f }
   };
   m_renderDevice->DrawTexturedQuad(m_renderDevice->m_DMDShader, vertices);
}

void Renderer::SetupDMDRender(const vec4& color, BaseTexture* dmd, const float alpha, const bool sRGB)
{
   // Legacy DMD renderer
   if (m_dmdUseNewRenderer)
   {
      m_renderDevice->m_DMDShader->SetVector(SHADER_vColor_Intensity, &color);
      #ifdef DMD_UPSCALE
         m_renderDevice->m_DMDShader->SetVector(SHADER_vRes_Alpha_time, (float)(dmd->width() * 3), (float)(dmd->height() * 3), alpha, (float)(g_pplayer->m_overall_frames % 2048));
      #else
         m_renderDevice->m_DMDShader->SetVector(SHADER_vRes_Alpha_time, (float)dmd->width(), (float)dmd->height(), alpha, (float)(g_pplayer->m_overall_frames % 2048));
      #endif
      m_renderDevice->m_DMDShader->SetTechnique(SHADER_TECHNIQUE_basic_DMD);
      m_renderDevice->m_DMDShader->SetTexture(SHADER_tex_dmd, dmd, SF_NONE, SA_CLAMP, SA_CLAMP, true);
   }
   // New DMD renderer
   else
   {
      static int lastFrame = -1;
      static BaseTexture* lastDmd = nullptr;
      const vec4 dotColor(color.x, color.y, color.z, 0.f);
      const float brightness = color.w;
      Sampler* dmdSampler = m_renderDevice->m_texMan.LoadTexture(dmd, SamplerFilter::SF_BILINEAR, SamplerAddressMode::SA_CLAMP, SamplerAddressMode::SA_CLAMP, true);
      if (m_dmdBlurs[0] == nullptr || m_dmdBlurs[0]->GetWidth() != dmdSampler->GetWidth() || m_dmdBlurs[0]->GetHeight() != dmdSampler->GetHeight())
      {
         lastFrame = -1;
         for (int i = 0; i < 4; i++)
         {
            delete m_dmdBlurs[i];
            m_dmdBlurs[i] = new RenderTarget(m_renderDevice, SurfaceType::RT_DEFAULT, "DMDBlur" + std::to_string(i), dmd->width(), dmd->height(), colorFormat::RG16F, false, 1, "");
         }
      }
      if (g_pplayer->m_overall_frames != lastFrame || lastDmd != dmd)
      {
         lastDmd = dmd;
         lastFrame = g_pplayer->m_overall_frames;
         RenderPass* const initial_rt = m_renderDevice->GetCurrentPass();
         for (int i = 0; i < 3; i++)
         {
            {
               m_renderDevice->SetRenderTarget("DMD HBlur "s + std::to_string(i + 1), m_dmdBlurs[0], false);
               if (i > 0)
                  m_renderDevice->AddRenderTargetDependency(m_dmdBlurs[i]);
               m_renderDevice->m_FBShader->SetTexture(SHADER_tex_fb_filtered, i == 0 ? dmdSampler : m_dmdBlurs[i]->GetColorSampler());
               m_renderDevice->m_FBShader->SetVector(SHADER_w_h_height, (float)(1.0 / dmdSampler->GetWidth()), (float)(1.0 / dmdSampler->GetHeight()), 1.0f, 1.0f);
               m_renderDevice->m_FBShader->SetTechnique(i == 0 ? SHADER_TECHNIQUE_fb_blur_horiz7x7 : SHADER_TECHNIQUE_fb_blur_horiz9x9);
               m_renderDevice->DrawFullscreenTexturedQuad(m_renderDevice->m_FBShader);
            }
            {
               m_renderDevice->SetRenderTarget("DMD VBlur "s + std::to_string(i + 1), m_dmdBlurs[i + 1], false);
               m_renderDevice->AddRenderTargetDependency(m_dmdBlurs[0]);
               m_renderDevice->m_FBShader->SetTexture(SHADER_tex_fb_filtered, m_dmdBlurs[0]->GetColorSampler());
               m_renderDevice->m_FBShader->SetVector(SHADER_w_h_height, (float)(1.0 / dmdSampler->GetWidth()), (float)(1.0 / dmdSampler->GetHeight()), 1.0f, 1.0f);
               m_renderDevice->m_FBShader->SetTechnique(i == 0 ? SHADER_TECHNIQUE_fb_blur_vert7x7 : SHADER_TECHNIQUE_fb_blur_vert9x9);
               m_renderDevice->DrawFullscreenTexturedQuad(m_renderDevice->m_FBShader);
            }
         }
         m_renderDevice->SetRenderTarget(initial_rt->m_name, initial_rt->m_rt, true);
         initial_rt->m_name += '-';
      }
      m_renderDevice->m_DMDShader->SetVector(SHADER_w_h_height, m_dmdDotProperties.x /* size */, m_dmdDotProperties.y /* sharpness */, m_dmdDotProperties.z /* rounding */, 0.f /* unused */);
      m_renderDevice->m_DMDShader->SetVector(SHADER_vColor_Intensity, dotColor.x * brightness, dotColor.y * brightness, dotColor.z * brightness, brightness); // dot color (only used if we received brightness data, premultiplied by overall brightness) and overall brightness (used for colored date)
      m_renderDevice->m_DMDShader->SetVector(SHADER_staticColor_Alpha, m_dmdUnlitDotColor.x, m_dmdUnlitDotColor.y, m_dmdUnlitDotColor.z, 0.f /* unused */);
      m_renderDevice->m_DMDShader->SetVector(SHADER_vRes_Alpha_time, (float)dmd->width(), (float)dmd->height(), m_dmdDotProperties.w /* dot glow */ * brightness, m_dmdUnlitDotColor.w /* back glow */ * brightness);

      m_renderDevice->m_DMDShader->SetTechnique(sRGB ? SHADER_TECHNIQUE_basic_DMD2_srgb : SHADER_TECHNIQUE_basic_DMD2);
      m_renderDevice->m_DMDShader->SetTexture(SHADER_tex_dmd, dmd);
      m_renderDevice->AddRenderTargetDependency(m_dmdBlurs[1]);
      m_renderDevice->m_DMDShader->SetTexture(SHADER_dmdDotGlow, m_dmdBlurs[1]->GetColorSampler());
      m_renderDevice->AddRenderTargetDependency(m_dmdBlurs[3]);
      m_renderDevice->m_DMDShader->SetTexture(SHADER_dmdBackGlow, m_dmdBlurs[3]->GetColorSampler()); // FIXME why don't we directly blur from 1 to 3 ?
   }
}

void Renderer::DrawBulbLightBuffer()
{
   RenderPass* const initial_rt = m_renderDevice->GetCurrentPass();
   static int id = 0; id++;

   // switch to 'bloom' output buffer to collect all bulb lights
   m_renderDevice->SetRenderTarget("Transmitted Light " + std::to_string(id) + " Clear", GetBloomBufferTexture(), false);
   m_renderDevice->ResetRenderState();
   m_renderDevice->Clear(clearType::TARGET, 0, 1.0f, 0L);

   // Draw bulb lights
   m_render_mask |= Renderer::LIGHT_BUFFER;
   m_renderDevice->SetRenderTarget("Transmitted Light " + std::to_string(id), GetBloomBufferTexture(), true, true);
   m_renderDevice->SetRenderState(RenderState::ZENABLE, RenderState::RS_FALSE); // disable all z-tests as zbuffer is in different resolution
   for (Hitable *hitable : g_pplayer->m_vhitables)
      if (hitable->HitableGetItemType() == eItemLight)
         hitable->Render(m_render_mask);
   m_render_mask &= ~Renderer::LIGHT_BUFFER;

   bool hasLight = m_renderDevice->GetCurrentPass()->GetCommandCount() > 0;
   if (hasLight)
   { // Only apply blur if we have actually rendered some lights
      RenderPass* renderPass = m_renderDevice->GetCurrentPass();
      m_renderDevice->DrawGaussianBlur(
         GetBloomBufferTexture(), 
         GetBloomTmpBufferTexture(), 
         GetBloomBufferTexture(), 19.f); // FIXME kernel size should depend on buffer resolution
      RenderPass *blurPass2 = m_renderDevice->GetCurrentPass();
      RenderPass *blurPass1 = blurPass2->m_dependencies[0];
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
   m_render_mask |= Renderer::STATIC_ONLY;
   for (Hitable* hitable : g_pplayer->m_vhitables)
      hitable->Render(m_render_mask);
   m_render_mask = mask;
}

void Renderer::DrawDynamics(bool onlyBalls)
{
   const unsigned int mask = m_render_mask;
   m_render_mask |= Renderer::DYNAMIC_ONLY;
   if (onlyBalls)
   {
      for (HitBall* ball : g_pplayer->m_vball)
         ball->m_pBall->Render(m_render_mask);
   }
   else
   {
      DrawBulbLightBuffer();
      for (Hitable* hitable : g_pplayer->m_vhitables)
         hitable->Render(m_render_mask);
   }
   m_render_mask = mask;
}

void Renderer::DrawSprite(const float posx, const float posy, const float width, const float height, const COLORREF color, Texture* const tex, const float intensity, const bool backdrop)
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
      m_renderDevice->m_DMDShader->SetTexture(SHADER_tex_sprite, tex, SF_NONE, SA_REPEAT, SA_REPEAT);
   m_renderDevice->SetRenderState(RenderState::ZENABLE, RenderState::RS_FALSE);
   m_renderDevice->DrawTexturedQuad(m_renderDevice->m_DMDShader, vertices);
   m_renderDevice->GetCurrentPass()->m_commands.back()->SetTransparent(true);
   m_renderDevice->GetCurrentPass()->m_commands.back()->SetDepth(-10000.f);
}

void Renderer::DrawSprite(const float posx, const float posy, const float width, const float height, const COLORREF color, Sampler* const tex, const float intensity, const bool backdrop)
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
      m_renderDevice->m_DMDShader->SetTexture(SHADER_tex_sprite, tex);
   m_renderDevice->SetRenderState(RenderState::ZENABLE, RenderState::RS_FALSE);
   m_renderDevice->DrawTexturedQuad(m_renderDevice->m_DMDShader, vertices);
   m_renderDevice->GetCurrentPass()->m_commands.back()->SetTransparent(true);
   m_renderDevice->GetCurrentPass()->m_commands.back()->SetDepth(-10000.f);
}

// MSVC Concurrency Viewer support
// This requires _WIN32_WINNT >= 0x0600 and to add the MSVC Concurrency SDK to the project
#ifdef MSVC_CONCURRENCY_VIEWER
#include <cvmarkersobj.h>
using namespace Concurrency::diagnostic;
extern marker_series series;
#endif

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

   // The code will fail if the static render target is MSAA (the copy operation we are performing is not allowed)
   delete m_staticPrepassRT;
   m_staticPrepassRT = GetBackBufferTexture()->Duplicate("StaticPreRender"s);
   assert(!m_staticPrepassRT->IsMSAA());

   RenderTarget *accumulationSurface = IsUsingStaticPrepass() ? m_staticPrepassRT->Duplicate("Accumulation"s) : nullptr;

   RenderTarget* renderRT = GetAOMode() == 1 ? GetBackBufferTexture() : m_staticPrepassRT;

   // if rendering static/with heavy oversampling, disable the aniso/trilinear filter to get a sharper/more precise result overall!
   if (IsUsingStaticPrepass())
   {
      PLOGI << "Performing prerendering of static parts."; // For profiling
      m_renderDevice->SetMainTextureDefaultFiltering(SF_BILINEAR);
   }

   //#define STATIC_PRERENDER_ITERATIONS_KOROBOV 7.0 // for the (commented out) lattice-based QMC oversampling, 'magic factor', depending on the the number of iterations!
   // loop for X times and accumulate/average these renderings
   // NOTE: iter == 0 MUST ALWAYS PRODUCE an offset of 0,0!
   int n_iter = IsUsingStaticPrepass() ? (STATIC_PRERENDER_ITERATIONS - 1) : 0;
   for (int iter = n_iter; iter >= 0; --iter) // just do one iteration if in dynamic camera/light/material tweaking mode
   {
      #ifdef MSVC_CONCURRENCY_VIEWER
      span* tagSpan = new span(series, 1, _T("PreRender"));
      #endif

      g_pplayer->m_progressDialog.SetProgress("Prerendering Static Parts..."s, 70 + (((30 * (n_iter + 1 - iter)) / (n_iter + 1))));
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
         for (Hitable *hitable : g_pplayer->m_vhitables)
            hitable->Render(m_render_mask);

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

   // if rendering static/with heavy oversampling, re-enable the aniso/trilinear filter now for the normal rendering
   const bool forceAniso = m_table->m_settings.LoadValueWithDefault(Settings::Player, "ForceAnisotropicFiltering"s, true);
   m_renderDevice->SetMainTextureDefaultFiltering(forceAniso ? SF_ANISOTROPIC : SF_TRILINEAR);

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
      m_renderDevice->m_FBShader->SetTexture(SHADER_tex_ao_dither, &m_aoDitherTexture, SF_NONE, SA_REPEAT, SA_REPEAT, true); // FIXME the force linear RGB is not honored in VR
      m_renderDevice->m_FBShader->SetVector(SHADER_AO_scale_timeblur, m_table->m_AOScale, 0.1f, 0.f, 0.f);
      m_renderDevice->m_FBShader->SetTechnique(SHADER_TECHNIQUE_AO);

      for (unsigned int i = 0; i < 50; ++i) // 50 iterations to get AO smooth
      {
         m_renderDevice->SetRenderTarget("PreRender AO"s, GetAORenderTarget(0));
         m_renderDevice->AddRenderTargetDependency(renderRT);
         m_renderDevice->AddRenderTargetDependency(GetAORenderTarget(1));
         if (i == 0)
            m_renderDevice->Clear(clearType::TARGET, 0, 1.0f, 0L);

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
         for (Hitable* hitable : g_pplayer->m_vhitables)
            hitable->Render(m_render_mask);
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
   PROFILE_FUNCTION(FrameProfiler::PROFILE_GPU_COLLECT);
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
   #define MAX_BALL_SHADOW 8
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

   // Render the default backglass without depth write before the table so that it will be visible for tables without a VR backglass but overwriten otherwise
   if (m_backGlass != nullptr)
      m_backGlass->Render();

   m_render_mask = IsUsingStaticPrepass() ? Renderer::DYNAMIC_ONLY : Renderer::DEFAULT;
   DrawBulbLightBuffer();
   for (Hitable* hitable : g_pplayer->m_vhitables)
      hitable->Render(m_render_mask);
   m_render_mask = Renderer::DEFAULT;
   
   m_renderDevice->m_basicShader->SetTextureNull(SHADER_tex_base_transmission); // need to reset the bulb light texture, as its used as render target for bloom again

   for (size_t i = 0; i < m_table->m_vrenderprobe.size(); ++i)
      m_table->m_vrenderprobe[i]->ApplyAreaOfInterest();

   if (!g_pplayer->m_liveUI->IsTweakMode())
      mixer_draw(); // Draw the mixer volume
}



#pragma region PostProcess

void Renderer::SetScreenOffset(const float x, const float y)
{
   const float rotation = ANGTORAD(m_table->mViewSetups[m_table->m_BG_current_set].GetRotation(m_renderDevice->GetOutputBackBuffer()->GetWidth(), m_renderDevice->GetOutputBackBuffer()->GetHeight()));
   const float c = cosf(-rotation), s = sinf(-rotation);
   m_ScreenOffset.x = x * c - y * s;
   m_ScreenOffset.y = x * s + y * c;
}

void Renderer::SSRefl()
{
   m_renderDevice->SetRenderTarget("ScreenSpace Reflection"s, GetReflectionBufferTexture(), false);
   m_renderDevice->AddRenderTargetDependency(GetBackBufferTexture(), true);

   m_renderDevice->m_FBShader->SetTexture(SHADER_tex_fb_filtered, GetBackBufferTexture()->GetColorSampler());
   m_renderDevice->m_FBShader->SetTexture(SHADER_tex_fb_unfiltered, GetBackBufferTexture()->GetColorSampler());
   m_renderDevice->m_FBShader->SetTexture(SHADER_tex_ao_dither, &m_aoDitherTexture, SF_NONE, SA_REPEAT, SA_REPEAT, true); // FIXME the force linear RGB is not honored in VR

   // FIXME check if size should not be taken from renderdevice to account for VR (double width) or supersampling
   m_renderDevice->m_FBShader->SetVector(SHADER_w_h_height,
      (float)(1.0 / GetBackBufferTexture()->GetWidth()), (float)(1.0 / GetBackBufferTexture()->GetHeight()), 1.0f /*radical_inverse(m_overall_frames%2048)*/, 1.0f);

   const float rotation = m_table->mViewSetups[m_table->m_BG_current_set].GetRotation(m_renderDevice->GetOutputBackBuffer()->GetWidth(), m_renderDevice->GetOutputBackBuffer()->GetHeight());
   const vec4 SSR_bumpHeight_fresnelRefl_scale_FS(0.3f, 0.3f, m_table->m_SSRScale, rotation);
   m_renderDevice->m_FBShader->SetVector(SHADER_SSR_bumpHeight_fresnelRefl_scale_FS, &SSR_bumpHeight_fresnelRefl_scale_FS);

   m_renderDevice->m_FBShader->SetTechnique(SHADER_TECHNIQUE_SSReflection);

   m_renderDevice->DrawFullscreenTexturedQuad(m_renderDevice->m_FBShader);
}

void Renderer::Bloom()
{
   if (m_table->m_bloom_strength <= 0.0f || m_bloomOff || g_pplayer->GetInfoMode() == IF_LIGHT_BUFFER_ONLY)
      return;

   const double w = (double)GetBackBufferTexture()->GetWidth();
   const double h = (double)GetBackBufferTexture()->GetHeight();
   Vertex3D_TexelOnly shiftedVerts[4] =
   {
      {  1.0f,  1.0f, 0.0f, 1.0f + (float)(2.25 / w), 0.0f + (float)(2.25 / h) },
      { -1.0f,  1.0f, 0.0f, 0.0f + (float)(2.25 / w), 0.0f + (float)(2.25 / h) },
      {  1.0f, -1.0f, 0.0f, 1.0f + (float)(2.25 / w), 1.0f + (float)(2.25 / h) },
      { -1.0f, -1.0f, 0.0f, 0.0f + (float)(2.25 / w), 1.0f + (float)(2.25 / h) }
   };
   #if defined(ENABLE_BGFX)
   if (bgfx::getCaps()->originBottomLeft)
   {
      shiftedVerts[0].tv = shiftedVerts[1].tv = 1.0f + (float)(2.25 / h);
      shiftedVerts[2].tv = shiftedVerts[3].tv = 0.0f + (float)(2.25 / h);
   }
   #endif
   {
      m_renderDevice->m_FBShader->SetTextureNull(SHADER_tex_fb_filtered);

      // switch to 'bloom' output buffer to collect clipped framebuffer values
      m_renderDevice->SetRenderTarget("Bloom Cut Off"s, GetBloomBufferTexture(), false);
      m_renderDevice->AddRenderTargetDependency(GetBackBufferTexture());

      m_renderDevice->m_FBShader->SetTexture(SHADER_tex_fb_filtered, GetBackBufferTexture()->GetColorSampler());
      m_renderDevice->m_FBShader->SetVector(SHADER_w_h_height, (float) (1.0 / w), (float) (1.0 / h), m_table->m_bloom_strength, 1.0f);
      m_renderDevice->m_FBShader->SetTechnique(SHADER_TECHNIQUE_fb_bloom);

      m_renderDevice->DrawTexturedQuad(m_renderDevice->m_FBShader, shiftedVerts);
   }

   m_renderDevice->DrawGaussianBlur(
      GetBloomBufferTexture(), 
      GetBloomTmpBufferTexture(),
      GetBloomBufferTexture(), 39.f); // FIXME kernel size should depend on buffer resolution
}

void Renderer::PrepareVideoBuffers()
{
   const bool useAA = m_renderWidth > GetBackBufferTexture()->GetWidth();
   const bool stereo = m_stereo3D == STEREO_VR || ((m_stereo3D != STEREO_OFF) && m_stereo3Denabled && (!m_stereo3DfakeStereo || m_renderDevice->DepthBufferReadBackAvailable()));
   // Since stereo is applied as a postprocess step for fake stereo, it disables AA and sharpening except for top/bottom & side by side modes
   const bool PostProcAA = !m_stereo3DfakeStereo || (!stereo || (m_stereo3D == STEREO_TB) || (m_stereo3D == STEREO_SBS));
#ifndef __OPENGLES__
   const bool SMAA  = PostProcAA && m_FXAA == Quality_SMAA;
#else
   const bool SMAA = false;
#endif
   const bool DLAA  = PostProcAA && m_FXAA == Standard_DLAA;
   const bool NFAA  = PostProcAA && m_FXAA == Fast_NFAA;
   const bool FXAA1 = PostProcAA && m_FXAA == Fast_FXAA;
   const bool FXAA2 = PostProcAA && m_FXAA == Standard_FXAA;
   const bool FXAA3 = PostProcAA && m_FXAA == Quality_FXAA;
   const bool ss_refl = m_ss_refl && m_table->m_enableSSR && m_renderDevice->DepthBufferReadBackAvailable() && m_table->m_SSRScale > 0.f;
   const unsigned int sharpen = PostProcAA ? m_sharpen : 0;
   const bool useAO = GetAOMode() == 2;
   const bool useUpscaler = (m_renderWidth < GetBackBufferTexture()->GetWidth()) && !stereo && (SMAA || DLAA || NFAA || FXAA1 || FXAA2 || FXAA3 || sharpen);

   RenderTarget *renderedRT = GetBackBufferTexture();
   RenderTarget *outputRT = nullptr;
   m_renderDevice->ResetRenderState();
   m_renderDevice->SetRenderState(RenderState::ALPHABLENDENABLE, RenderState::RS_FALSE);
   m_renderDevice->SetRenderState(RenderState::CULLMODE, RenderState::CULL_NONE);
   m_renderDevice->SetRenderState(RenderState::ZWRITEENABLE, RenderState::RS_FALSE);
   m_renderDevice->SetRenderState(RenderState::ZENABLE, RenderState::RS_FALSE);

   // All postprocess that uses depth sample it from the MSAA resolved rendered backbuffer
   m_renderDevice->m_FBShader->SetTexture(SHADER_tex_depth, renderedRT->GetDepthSampler());

   // Compute bloom (to be applied later)
   Bloom();
   if (g_pplayer->GetProfilingMode() == PF_ENABLED)
      m_gpu_profiler.Timestamp(GTS_Bloom);

   // Add screen space reflections
   if (ss_refl)
   {
      SSRefl();
      renderedRT = GetReflectionBufferTexture();
   }
   if (g_pplayer->GetProfilingMode() == PF_ENABLED)
      m_gpu_profiler.Timestamp(GTS_SSR);

   // Compute AO contribution (to be applied later)
   if (useAO)
   {
      // separate normal generation pass, currently roughly same perf or even much worse
      /* m_renderDevice->SetRenderTarget(m_pd3dDevice->GetPostProcessRenderTarget1()); //!! expects stereo or FXAA enabled
      m_renderDevice->m_FBShader->SetTexture(SHADER_tex_depth, m_pdds3DZBuffer, true);
      m_renderDevice->m_FBShader->SetVector(SHADER_w_h_height, (float)(1.0 / m_width), (float)(1.0 / m_height),
         radical_inverse(m_overall_frames%2048)*(float)(1. / 8.0), sobol(m_overall_frames%2048)*(float)(5. / 8.0));// jitter within lattice cell //!! ?
      m_renderDevice->m_FBShader->SetTechnique("normals");
      m_renderDevice->DrawFullscreenTexturedQuad(m_renderDevice->m_FBShader);*/

      m_renderDevice->SetRenderTarget("ScreenSpace AO"s, GetAORenderTarget(0), false);
      m_renderDevice->AddRenderTargetDependency(GetAORenderTarget(1));
      m_renderDevice->AddRenderTargetDependency(GetBackBufferTexture());
      m_renderDevice->m_FBShader->SetTexture(SHADER_tex_fb_filtered, GetAORenderTarget(1)->GetColorSampler());
      //m_renderDevice->m_FBShader->SetTexture(SHADER_Texture1, m_pd3dDevice->GetPostProcessRenderTarget1()); // temporary normals
      m_renderDevice->m_FBShader->SetTexture(SHADER_tex_ao_dither, &m_aoDitherTexture, SF_NONE, SA_REPEAT, SA_REPEAT, true); // FIXME the force linear RGB is not honored
      m_renderDevice->m_FBShader->SetVector(SHADER_w_h_height, 
         (float)(1.0 / GetAORenderTarget(1)->GetWidth()),
         (float)(1.0 / GetAORenderTarget(1)->GetHeight()), 
         radical_inverse(g_pplayer->m_overall_frames % 2048) * (float)(1. / 8.0),
         /*sobol*/ radical_inverse<3>(g_pplayer->m_overall_frames % 2048) * (float)(1. / 8.0)); // jitter within (64/8)x(64/8) neighborhood of 64x64 tex, good compromise between blotches and noise
      m_renderDevice->m_FBShader->SetVector(SHADER_AO_scale_timeblur, m_table->m_AOScale, 0.4f, 0.f, 0.f); //!! 0.4f: fake global option in video pref? or time dependent? //!! commonly used is 0.1, but would require to clear history for moving stuff
      m_renderDevice->m_FBShader->SetTechnique(SHADER_TECHNIQUE_AO);
      m_renderDevice->DrawFullscreenTexturedQuad(m_renderDevice->m_FBShader);

      // flip AO buffers (avoids copy)
      SwapAORenderTargets();
   }
   if (g_pplayer->GetProfilingMode() == PF_ENABLED)
      m_gpu_profiler.Timestamp(GTS_AO);

   // Perform color grade LUT / dither / tonemapping, also applying bloom and AO
   {
      // switch to output buffer (main output frame buffer, or a temporary one for postprocessing)
      if (SMAA || DLAA || NFAA || FXAA1 || FXAA2 || FXAA3 || sharpen || stereo || useUpscaler)
         outputRT = GetPostProcessRenderTarget1();
      else
         outputRT = m_renderDevice->GetOutputBackBuffer();
      m_renderDevice->SetRenderTarget("Tonemap/Dither/ColorGrade"s, outputRT, false);
      m_renderDevice->AddRenderTargetDependency(renderedRT, useAO);

      int render_w = renderedRT->GetWidth(), render_h = renderedRT->GetHeight();
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

      Texture *const pin = m_table->GetImage(m_table->m_imageColorGrade);
      if (pin)
         // FIXME ensure that we always honor the linear RGB. Here it can be defeated if texture is used for something else (which is very unlikely)
         m_renderDevice->m_FBShader->SetTexture(SHADER_tex_color_lut, pin, SF_BILINEAR, SA_CLAMP, SA_CLAMP, true);
      m_renderDevice->m_FBShader->SetBool(SHADER_color_grade, pin != nullptr);
      m_renderDevice->m_FBShader->SetBool(SHADER_do_dither, m_renderDevice->GetOutputBackBuffer()->GetColorFormat() != colorFormat::RGBA10);
      m_renderDevice->m_FBShader->SetBool(SHADER_do_bloom, (m_table->m_bloom_strength > 0.0f && !m_bloomOff && infoMode <= IF_DYNAMIC_ONLY));

      //const unsigned int jittertime = (unsigned int)((U64)msec()*90/1000);
      const float jitter = (float)((msec()&2047)/1000.0);
      m_renderDevice->m_FBShader->SetVector(SHADER_w_h_height,
         (float)(1.0 / (double)render_w), (float)(1.0 / (double)render_h), //1.0f, 1.0f);
         jitter, // radical_inverse(jittertime) * 11.0f,
         jitter); // sobol(jittertime) * 13.0f); // jitter for dither pattern

      if (infoMode == IF_AO_ONLY)
         m_renderDevice->m_FBShader->SetTechnique(SHADER_TECHNIQUE_fb_AO);
      else if (infoMode == IF_RENDER_PROBES)
         m_renderDevice->m_FBShader->SetTechnique(m_toneMapper == TM_REINHARD ? SHADER_TECHNIQUE_fb_rhtonemap
                                                : m_toneMapper == TM_FILMIC   ? SHADER_TECHNIQUE_fb_fmtonemap
                                                : m_toneMapper == TM_NEUTRAL  ? SHADER_TECHNIQUE_fb_nttonemap
                                                : m_toneMapper == TM_AGX      ? SHADER_TECHNIQUE_fb_agxtonemap
                                                : /* TM_TONY_MC_MAPFACE */      SHADER_TECHNIQUE_fb_tmtonemap);
      else if (m_BWrendering != 0)
         m_renderDevice->m_FBShader->SetTechnique(m_BWrendering == 1 ? SHADER_TECHNIQUE_fb_rhtonemap_no_filterRG : SHADER_TECHNIQUE_fb_rhtonemap_no_filterR);
      else if (m_toneMapper == TM_REINHARD)
         m_renderDevice->m_FBShader->SetTechnique(useAO ?
              useAA ? SHADER_TECHNIQUE_fb_rhtonemap_AO : SHADER_TECHNIQUE_fb_rhtonemap_AO_no_filter
            : useAA ? SHADER_TECHNIQUE_fb_rhtonemap    : SHADER_TECHNIQUE_fb_rhtonemap_no_filter);
      else if (m_toneMapper == TM_FILMIC)
         m_renderDevice->m_FBShader->SetTechnique(useAO ?
              useAA ? SHADER_TECHNIQUE_fb_fmtonemap_AO : SHADER_TECHNIQUE_fb_fmtonemap_AO_no_filter
            : useAA ? SHADER_TECHNIQUE_fb_fmtonemap    : SHADER_TECHNIQUE_fb_fmtonemap_no_filter);
      else if (m_toneMapper == TM_NEUTRAL)
         m_renderDevice->m_FBShader->SetTechnique(useAO ?
              useAA ? SHADER_TECHNIQUE_fb_nttonemap_AO : SHADER_TECHNIQUE_fb_nttonemap_AO_no_filter
            : useAA ? SHADER_TECHNIQUE_fb_nttonemap    : SHADER_TECHNIQUE_fb_nttonemap_no_filter);
      else if (m_toneMapper == TM_AGX)
         m_renderDevice->m_FBShader->SetTechnique(useAO ?
              useAA ? SHADER_TECHNIQUE_fb_agxtonemap_AO : SHADER_TECHNIQUE_fb_agxtonemap_AO_no_filter
            : useAA ? SHADER_TECHNIQUE_fb_agxtonemap    : SHADER_TECHNIQUE_fb_agxtonemap_no_filter);
      else // TM_TONY_MC_MAPFACE
         m_renderDevice->m_FBShader->SetTechnique(useAO ?
              useAA ? SHADER_TECHNIQUE_fb_tmtonemap_AO : SHADER_TECHNIQUE_fb_tmtonemap_AO_no_filter
            : useAA ? SHADER_TECHNIQUE_fb_tmtonemap    : SHADER_TECHNIQUE_fb_tmtonemap_no_filter);

      Vertex3D_TexelOnly shiftedVerts[4] =
      {
         {  1.0f + m_ScreenOffset.x,  1.0f + m_ScreenOffset.y, 0.0f, 1.0f, 0.0f },
         { -1.0f + m_ScreenOffset.x,  1.0f + m_ScreenOffset.y, 0.0f, 0.0f, 0.0f },
         {  1.0f + m_ScreenOffset.x, -1.0f + m_ScreenOffset.y, 0.0f, 1.0f, 1.0f },
         { -1.0f + m_ScreenOffset.x, -1.0f + m_ScreenOffset.y, 0.0f, 0.0f, 1.0f }
      };
      #if defined(ENABLE_BGFX)
      if (bgfx::getCaps()->originBottomLeft)
      {
         shiftedVerts[0].tv = shiftedVerts[1].tv = 1.0f;
         shiftedVerts[2].tv = shiftedVerts[3].tv = 0.0f;
      }
      #endif
      m_renderDevice->DrawTexturedQuad(m_renderDevice->m_FBShader, shiftedVerts);
      renderedRT = outputRT;
   }

   // This code allows to check that the FB shader does perform pixel perfect processing (1 to 1 match between renderedRT and outputRT)
   // This needs a modification of the shader to used the filtered texture (tex_fb_filtered) instead of unfiltered
   if (false)
   {
      BaseTexture *tex = new BaseTexture(renderedRT->GetWidth(), renderedRT->GetHeight(), BaseTexture::RGB);
      BYTE *const __restrict pdest = tex->data();
      for (size_t i = 0; i < (size_t)renderedRT->GetWidth() * renderedRT->GetHeight(); ++i)
      {
         size_t y = i / renderedRT->GetWidth();
         #if defined(ENABLE_OPENGL) || defined(ENABLE_BGFX)
         y = renderedRT->GetHeight() - 1 - y;
         #endif
         pdest[i * 3 + 0] = ((i & 1) == 0 && (y & 1) == 0) ? 0x00 : 0xFF;
         pdest[i * 3 + 1] = ((i >> 2) & 1) == 0 ? 0x00 : ((i & 1) == 0 && (y & 1) == 0) ? 0x00 : 0xFF;
         pdest[i * 3 + 2] = ((y >> 2) & 1) == 0 ? 0x00 : ((i & 1) == 0 && (y & 1) == 0) ? 0x00 : 0xFF;
      }
      Sampler *checker = new Sampler(m_renderDevice, tex, true, SA_CLAMP, SA_CLAMP, SF_NONE);
      m_renderDevice->m_FBShader->SetVector(SHADER_w_h_height, (float)(1.0 / renderedRT->GetWidth()), (float)(1.0 / renderedRT->GetHeight()), 1.f, 1.f);
      m_renderDevice->m_FBShader->SetTexture(SHADER_tex_fb_filtered, checker);
      m_renderDevice->m_FBShader->SetTechnique(SHADER_TECHNIQUE_fb_mirror);
      m_renderDevice->DrawFullscreenTexturedQuad(m_renderDevice->m_FBShader);
      renderedRT = outputRT;
      delete checker;
      delete tex;
   }

   // Stereo and AA are performed on LDR render buffer after tonemapping (RGB8 or RGB10, but nof RGBF).
   // We ping pong between BackBufferTmpTexture and BackBufferTmpTexture2 for the different postprocess
   // SMAA is a special case since it needs 3 passes, so it uses GetBackBufferTexture also (which is somewhat overkill since it is RGB16F)
   assert(renderedRT == m_renderDevice->GetOutputBackBuffer() || renderedRT == GetPostProcessRenderTarget1());

   // Perform post processed anti aliasing
   if (NFAA || FXAA1 || FXAA2 || FXAA3)
   {
      assert(renderedRT == GetPostProcessRenderTarget1());
      outputRT = sharpen || stereo || useUpscaler ? GetPostProcessRenderTarget(renderedRT) : m_renderDevice->GetOutputBackBuffer();
      m_renderDevice->SetRenderTarget(SMAA ? "SMAA Color/Edge Detection"s : "Post Process AA Pass 1"s, outputRT, false);
      m_renderDevice->AddRenderTargetDependency(renderedRT);
      m_renderDevice->m_FBShader->SetTexture(SHADER_tex_fb_filtered, renderedRT->GetColorSampler());
      m_renderDevice->m_FBShader->SetTexture(SHADER_tex_fb_unfiltered, renderedRT->GetColorSampler());
      m_renderDevice->AddRenderTargetDependency(GetBackBufferTexture(), true); // Depth is always taken from the MSAA resolved render buffer
      m_renderDevice->m_FBShader->SetVector(SHADER_w_h_height, (float)(1.0 / renderedRT->GetWidth()), (float)(1.0 / renderedRT->GetHeight()), (float)renderedRT->GetWidth(), 1.f);
      m_renderDevice->m_FBShader->SetTechnique(NFAA  ? SHADER_TECHNIQUE_NFAA : FXAA3 ? SHADER_TECHNIQUE_FXAA3 : FXAA2 ? SHADER_TECHNIQUE_FXAA2 : SHADER_TECHNIQUE_FXAA1);
      m_renderDevice->DrawFullscreenTexturedQuad(m_renderDevice->m_FBShader);
      renderedRT = outputRT;
   }
   else if (DLAA)
   {
      assert(renderedRT == GetPostProcessRenderTarget1());
      // First pass detect edges and write it to alpha channel (keeping RGB)
      outputRT = GetPostProcessRenderTarget(renderedRT);
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
      outputRT = sharpen || stereo || useUpscaler ? GetPostProcessRenderTarget(renderedRT) : m_renderDevice->GetOutputBackBuffer();
      m_renderDevice->SetRenderTarget("DLAA Neigborhood blending"s, outputRT, false);
      m_renderDevice->AddRenderTargetDependency(renderedRT);
      m_renderDevice->m_FBShader->SetTexture(SHADER_tex_fb_filtered, renderedRT->GetColorSampler());
      m_renderDevice->m_FBShader->SetTexture(SHADER_tex_fb_unfiltered, renderedRT->GetColorSampler());
      m_renderDevice->m_FBShader->SetTechnique(SHADER_TECHNIQUE_DLAA);
      m_renderDevice->DrawFullscreenTexturedQuad(m_renderDevice->m_FBShader);
      renderedRT = outputRT;
   }
   else if (SMAA)
   {
#ifndef __OPENGLES__
      assert(renderedRT == GetPostProcessRenderTarget1());
      // SMAA use 3 passes, all of them using the initial render, so since tonemap use postprocess RT 1, we use the back buffer and post process RT 2
      RenderTarget *sourceRT = renderedRT;
      m_renderDevice->m_FBShader->SetTexture(SHADER_tex_fb_filtered, sourceRT->GetColorSampler());
      m_renderDevice->m_FBShader->SetTexture(SHADER_tex_fb_unfiltered, sourceRT->GetColorSampler());
      m_renderDevice->m_FBShader->SetVector(SHADER_w_h_height, (float)(1.0 / sourceRT->GetWidth()), (float)(1.0 / sourceRT->GetHeight()), (float)sourceRT->GetWidth(), (float)sourceRT->GetHeight());

      outputRT = GetPreviousBackBufferTexture(); // We don't need it anymore, so use it as a third postprocess buffer
      m_renderDevice->SetRenderTarget("SMAA Color/Edge Detection"s, outputRT, false);
      m_renderDevice->AddRenderTargetDependency(sourceRT); // PostProcess RT 1
      m_renderDevice->Clear(clearType::TARGET, 0, 1.0f, 0L); // Needed since shader uses discard
      m_renderDevice->m_FBShader->SetTechnique(SHADER_TECHNIQUE_SMAA_ColorEdgeDetection);
      m_renderDevice->DrawFullscreenTexturedQuad(m_renderDevice->m_FBShader);
      renderedRT = outputRT;

      outputRT = GetPostProcessRenderTarget(sourceRT);
      m_renderDevice->SetRenderTarget("SMAA Blend weight calculation"s, outputRT, false);
      m_renderDevice->AddRenderTargetDependency(sourceRT); // PostProcess RT 1
      m_renderDevice->AddRenderTargetDependency(renderedRT); // BackBuffer RT
      m_renderDevice->m_FBShader->SetTechnique(SHADER_TECHNIQUE_SMAA_BlendWeightCalculation);
      m_renderDevice->m_FBShader->SetTexture(SHADER_edgesTex, renderedRT->GetColorSampler());
      m_renderDevice->DrawFullscreenTexturedQuad(m_renderDevice->m_FBShader);
      renderedRT = outputRT;

      outputRT = sharpen || stereo || useUpscaler ? GetPreviousBackBufferTexture() : m_renderDevice->GetOutputBackBuffer();
      m_renderDevice->SetRenderTarget("SMAA Neigborhood blending"s, outputRT, false);
      m_renderDevice->AddRenderTargetDependency(sourceRT); // PostProcess RT 1
      m_renderDevice->AddRenderTargetDependency(renderedRT); // PostProcess RT 2
      m_renderDevice->m_FBShader->SetTechnique(SHADER_TECHNIQUE_SMAA_NeighborhoodBlending);
      m_renderDevice->m_FBShader->SetTexture(SHADER_blendTex, renderedRT->GetColorSampler());
      m_renderDevice->DrawFullscreenTexturedQuad(m_renderDevice->m_FBShader);
      renderedRT = outputRT;
#endif
   }

   // Performs sharpening
   if (sharpen)
   {
      assert(renderedRT != m_renderDevice->GetOutputBackBuffer()); // At this point, renderedRT may be PP1, PP2 or backbuffer
      outputRT = stereo || useUpscaler ? GetPostProcessRenderTarget(renderedRT) : m_renderDevice->GetOutputBackBuffer();
      m_renderDevice->SetRenderTarget("Sharpen"s, outputRT, false);
      m_renderDevice->AddRenderTargetDependency(renderedRT);
      m_renderDevice->AddRenderTargetDependency(GetBackBufferTexture(), true); // Depth is always taken from the MSAA resolved render buffer
      m_renderDevice->m_FBShader->SetTexture(SHADER_tex_fb_filtered, renderedRT->GetColorSampler());
      m_renderDevice->m_FBShader->SetTexture(SHADER_tex_fb_unfiltered, renderedRT->GetColorSampler());
      m_renderDevice->m_FBShader->SetVector(SHADER_w_h_height, (float)(1.0 / renderedRT->GetWidth()), (float)(1.0 / renderedRT->GetHeight()), (float)renderedRT->GetWidth(), 1.f);
      m_renderDevice->m_FBShader->SetTechnique((sharpen == 1) ? SHADER_TECHNIQUE_CAS : SHADER_TECHNIQUE_BilateralSharp_CAS);
      m_renderDevice->DrawFullscreenTexturedQuad(m_renderDevice->m_FBShader);
      renderedRT = outputRT;
   }

   // Apply stereo
   if (stereo)
   {
      #if defined(ENABLE_VR)
      // For STEREO_OFF, STEREO_TB, STEREO_SBS, this won't do anything. The previous postprocess steps should already have written to OutputBackBuffer
      // For VR, copy each eye to the HMD texture and render the wanted preview if activated
      if (m_stereo3D == STEREO_VR)
      {
         assert(renderedRT != m_renderDevice->GetOutputBackBuffer());
         int w = renderedRT->GetWidth(), h = renderedRT->GetHeight();

         RenderTarget *leftTexture = GetOffscreenVR(0);
         m_renderDevice->SetRenderTarget("Left Eye"s, leftTexture, false);
         m_renderDevice->AddRenderTargetDependency(renderedRT);
         m_renderDevice->BlitRenderTarget(renderedRT, leftTexture, true, false, 0, 0, w, h, 0, 0, w, h, 0, 0);
         if (g_pplayer->m_liveUI->IsTweakMode()) // Render as an overlay in VR (not in preview) at the eye resolution, without depth
            m_renderDevice->RenderLiveUI();

         RenderTarget *rightTexture = GetOffscreenVR(1);
         m_renderDevice->SetRenderTarget("Right Eye"s, rightTexture, false);
         m_renderDevice->AddRenderTargetDependency(renderedRT);
         m_renderDevice->BlitRenderTarget(renderedRT, rightTexture, true, false, 0, 0, w, h, 0, 0, w, h, 1, 0);
         if (g_pplayer->m_liveUI->IsTweakMode()) // Render as an overlay in VR (not in preview) at the eye resolution, without depth
            m_renderDevice->RenderLiveUI();

         RenderTarget *outRT = m_renderDevice->GetOutputBackBuffer();
         m_renderDevice->SetRenderTarget("VR Preview"s, outRT, false);
         m_renderDevice->AddRenderTargetDependency(leftTexture); // To ensure blit is made
         m_renderDevice->AddRenderTargetDependency(rightTexture); // To ensure blit is made
         m_renderDevice->AddRenderTargetDependency(renderedRT);
         const int outW = m_vrPreview == VRPREVIEW_BOTH ? outRT->GetWidth() / 2 : outRT->GetWidth(), outH = outRT->GetHeight();
         float ar = (float)w / (float)h, outAr = (float)outW / (float)outH;
         int x = 0, y = 0;
         int fw = w, fh = h;
         if ((m_vrPreviewShrink && ar < outAr) || (!m_vrPreviewShrink && ar > outAr))
         { // Fit on Y
            const int scaledW = (int) (h * outAr);
            x = (w - scaledW) / 2;
            fw = scaledW;
         }
         else
         { // Fit on X
            const int scaledH = (int)(w / outAr);
            y = (h - scaledH) / 2;
            fh = scaledH;
         }
         if (m_vrPreviewShrink || m_vrPreview == VRPREVIEW_DISABLED)
            m_renderDevice->Clear(clearType::TARGET | clearType::ZBUFFER, 0, 1.0f, 0L);
         if (m_vrPreview == VRPREVIEW_LEFT || m_vrPreview == VRPREVIEW_RIGHT)
         {
            m_renderDevice->BlitRenderTarget(renderedRT, outRT, true, false, x, y, fw, fh, 0, 0, outW, outH, m_vrPreview == VRPREVIEW_LEFT ? 0 : 1, 0);
         }
         else if (m_vrPreview == VRPREVIEW_BOTH)
         {
            m_renderDevice->BlitRenderTarget(renderedRT, outRT, true, false, x, y, fw, fh, 0, 0, outW, outH, 0, 0);
            m_renderDevice->BlitRenderTarget(renderedRT, outRT, true, false, x, y, fw, fh, outW, 0, outW, outH, 1, 0);
         }

         m_renderDevice->SubmitVR(renderedRT);
      }
      else 
      #endif
      if (IsAnaglyphStereoMode(m_stereo3D) || Is3DTVStereoMode(m_stereo3D))
      {
         // Anaglyph and 3DTV
         assert(renderedRT != m_renderDevice->GetOutputBackBuffer());
         // For anaglyph, defocus the "lesser" eye (the one with a darker color, which should be the non dominant eye of the player)
         if (m_stereo3DDefocus != 0.f)
         {
            RenderTarget *tmpRT = GetPostProcessRenderTarget(renderedRT);
            outputRT = GetPostProcessRenderTarget(tmpRT);
            m_renderDevice->DrawGaussianBlur(renderedRT, tmpRT, outputRT, abs(m_stereo3DDefocus) * 39.f, m_stereo3DDefocus > 0.f ? 0 : 1);
            renderedRT = outputRT;
         }
         // Stereo composition
         m_renderDevice->SetRenderTarget("Stereo Anaglyph"s, m_renderDevice->GetOutputBackBuffer(), false);
         m_renderDevice->AddRenderTargetDependency(renderedRT);
         if (m_stereo3DfakeStereo)
         {
            m_renderDevice->AddRenderTargetDependency(GetBackBufferTexture(), true);
            m_renderDevice->m_stereoShader->SetTexture(SHADER_tex_stereo_depth, GetBackBufferTexture()->GetDepthSampler());
         }
         m_renderDevice->m_stereoShader->SetTexture(SHADER_tex_stereo_fb, renderedRT->GetColorSampler());
         m_renderDevice->DrawFullscreenTexturedQuad(m_renderDevice->m_stereoShader);
      }
      else
      {
         // STEREO_OFF: nothing to do
         assert(renderedRT == m_renderDevice->GetOutputBackBuffer());
      }
   }
   // Upscale: When using downscaled backbuffer (for performance reason), upscaling is done after postprocessing
   else if (useUpscaler)
   {
      assert(renderedRT != m_renderDevice->GetOutputBackBuffer()); // At this point, renderedRT may be PP1, PP2 or backbuffer
      outputRT = m_renderDevice->GetOutputBackBuffer();
      m_renderDevice->SetRenderTarget("Upscale"s, outputRT, false);
      m_renderDevice->AddRenderTargetDependency(renderedRT);
      m_renderDevice->m_FBShader->SetTexture(SHADER_tex_fb_filtered, renderedRT->GetColorSampler());
      m_renderDevice->m_FBShader->SetTechnique(SHADER_TECHNIQUE_fb_copy);
      m_renderDevice->DrawFullscreenTexturedQuad(m_renderDevice->m_FBShader);
      renderedRT = outputRT;
   }

   if (m_stereo3D != STEREO_VR)
   {
      // Except for VR, render LiveUI after tonemapping and stereo (otherwise it would break the calibration process for stereo anaglyph)
      g_frameProfiler.EnterProfileSection(FrameProfiler::PROFILE_MISC);
      m_renderDevice->SetRenderTarget("ImGui"s, m_renderDevice->GetOutputBackBuffer());
      m_renderDevice->RenderLiveUI();
      g_frameProfiler.ExitProfileSection();
   }

   if (g_pplayer->GetProfilingMode() == PF_ENABLED)
      m_gpu_profiler.Timestamp(GTS_PostProcess);
}

#pragma endregion
