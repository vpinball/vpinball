#include "stdafx.h"
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

#ifndef ENABLE_SDL
// Note: Nowadays the original code seems to be counter-productive, so we use the official
// pre-rendered frame mechanism instead where possible
// (e.g. all windows versions except for XP and no "EnableLegacyMaximumPreRenderedFrames" set in the registry)
/*
 * Class to limit the length of the GPU command buffer queue to at most 'numFrames' frames.
 * Excessive buffering of GPU commands creates high latency and can create stuttery overlong
 * frames when the CPU stalls due to a full command buffer ring.
 *
 * Calling Execute() within a BeginScene() / EndScene() pair creates an artificial pipeline
 * stall by locking a vertex buffer which was rendered from (numFrames-1) frames ago. This
 * forces the CPU to wait and let the GPU catch up so that rendering doesn't lag more than
 * numFrames behind the CPU. It does *NOT* limit the framerate itself, only the drawahead.
 * Note that VP is currently usually GPU-bound.
 *
 * This is similar to Flush() in later DX versions, but doesn't flush the entire command
 * buffer, only up to a certain previous frame.
 *
 * Use of this class has been observed to effectively reduce stutter at least on an NVidia/
 * Win7 64 bit setup. The queue limiting effect can be clearly seen in GPUView.
 *
 * The initial cause for the stutter may be that our command buffers are too big (two
 * packets per frame on typical tables, instead of one), so with more optimizations to
 * draw calls/state changes, none of this may be needed anymore.
 */
class FrameQueueLimiter
{
public:
   FrameQueueLimiter(RenderDevice* const pd3dDevice, const int numFrames)
      : m_pd3dDevice(pd3dDevice)
   {
      m_curIdx = 0;
      const int EnableLegacyMaximumPreRenderedFrames = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "EnableLegacyMaximumPreRenderedFrames"s, 0);

      // if available, use the official RenderDevice mechanism
      if (!EnableLegacyMaximumPreRenderedFrames && pd3dDevice->SetMaximumPreRenderedFrames(numFrames))
      {
          m_buffers.resize(0);
          return;
      }

      // if not, fallback to cheating the driver
      m_buffers.resize(numFrames, nullptr);
      m_curIdx = 0;
   }

   ~FrameQueueLimiter()
   {
      for (size_t i = 0; i < m_buffers.size(); ++i)
         delete m_buffers[i];
   }

   void Execute()
   {
      if (m_buffers.empty())
         return;

      if (m_buffers[m_curIdx])
      {
         Vertex3Ds pos(0.f, 0.f, 0.f);
         m_pd3dDevice->DrawMesh(m_pd3dDevice->basicShader, false, pos, 0.f, m_buffers[m_curIdx], RenderDevice::TRIANGLESTRIP, 0, 3);
      }

      m_curIdx = (m_curIdx + 1) % m_buffers.size();

      if (!m_buffers[m_curIdx])
      {
         VertexBuffer* vb = new VertexBuffer(m_pd3dDevice, 1024);
         m_buffers[m_curIdx] = new MeshBuffer(L"FrameLimiter"s,  vb);
      }

      // idea: locking a static vertex buffer stalls the pipeline if that VB is still
      // in the GPU render queue. In effect, this lets the GPU catch up.
      Vertex3D_NoTex2* buf;
      m_buffers[m_curIdx]->m_vb->lock(0, 0, (void**)&buf, VertexBuffer::WRITEONLY);
      memset(buf, 0, 3 * sizeof(buf[0]));
      buf[0].z = buf[1].z = buf[2].z = 1e5f;      // single triangle, degenerates to point far off screen
      m_buffers[m_curIdx]->m_vb->unlock();
   }

private:
   RenderDevice* const m_pd3dDevice;
   vector<MeshBuffer*> m_buffers;
   size_t m_curIdx;
};
#else
class FrameQueueLimiter
{
public:
   FrameQueueLimiter(RenderDevice* const pd3dDevice, const int numFrames) { }
   void Execute() { }
};
#endif

////////////////////////////////////////////////////////////////////////////////

Renderer::Renderer(PinTable* const table, const bool fullScreen, const int width, const int height, const int colordepth, int& refreshrate, VideoSyncMode& syncMode, const StereoMode stereo3D)
   : m_table(table)
{
   m_stereo3Denabled = m_table->m_settings.LoadValueWithDefault(Settings::Player, "Stereo3DEnabled"s, (m_stereo3D != STEREO_OFF));
   m_stereo3DfakeStereo = m_table->m_settings.LoadValueWithDefault(Settings::Player, "Stereo3DFake"s, false);
   #ifndef ENABLE_SDL // DirectX does not support stereo rendering
   m_stereo3DfakeStereo = true;
   #endif
   m_BWrendering = m_table->m_settings.LoadValueWithDefault(Settings::Player, "BWRendering"s, 0);
   m_toneMapper = (ToneMapper)m_table->m_settings.LoadValueWithDefault(Settings::TableOverride, "ToneMapper"s, m_table->GetToneMapper());
   m_dynamicAO = m_table->m_settings.LoadValueWithDefault(Settings::Player, "DynamicAO"s, true);
   m_disableAO = m_table->m_settings.LoadValueWithDefault(Settings::Player, "DisableAO"s, false);
   m_AAfactor = m_table->m_settings.LoadValueWithDefault(Settings::Player, "AAFactor"s, m_table->m_settings.LoadValueWithDefault(Settings::Player, "USEAA"s, false) ? 2.0f : 1.0f);
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
   m_disableLightingForBalls = m_table->m_settings.LoadValueWithDefault(Settings::Player, "DisableLightingForBalls"s, false);
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

   m_stereo3D = m_stereo3DfakeStereo ? STEREO_OFF : stereo3D;
   m_mvp = new ModelViewProj(m_stereo3D == STEREO_OFF ? 1 : 2);

   const int display = g_pvp->m_primaryDisplay ? 0 : m_table->m_settings.LoadValueWithDefault(Settings::Player, "Display"s, 0);
   vector<DisplayConfig> displays;
   getDisplayList(displays);
   int adapter = 0;
   for (vector<DisplayConfig>::iterator dispConf = displays.begin(); dispConf != displays.end(); ++dispConf)
      if (display == dispConf->display)
         adapter = dispConf->adapter;

   #ifdef ENABLE_SDL
   const int nMSAASamples = m_table->m_settings.LoadValueWithDefault(Settings::Player, "MSAASamples"s, 1);
   #else
   // Sadly DX9 does not support resolving an MSAA depth buffer, making MSAA implementation complex for it. So just disable for now
   constexpr int nMSAASamples = 1;
   #endif
   const bool useNvidiaApi = m_table->m_settings.LoadValueWithDefault(Settings::Player, "UseNVidiaAPI"s, false);
   const bool disableDWM = m_table->m_settings.LoadValueWithDefault(Settings::Player, "DisableDWM"s, false);
   int renderWidth, renderHeight;
   if (m_stereo3D == STEREO_VR)
   {
      // For VR the render view is defined by the HMD
      renderWidth = g_pplayer->m_vrDevice->GetEyeWidth();
      renderHeight = g_pplayer->m_vrDevice->GetEyeHeight();
   }
   if (m_stereo3D == STEREO_SBS)
   {
      // Side by side needs to fit the 2 views along the width, so each view is half the total width
      renderWidth = width / 2;
      renderHeight = height;
   }
   else if (m_stereo3D == STEREO_TB || m_stereo3D == STEREO_INT || m_stereo3D == STEREO_FLIPPED_INT)
   {
      // Top/Bottom (and interlaced) needs to fit the 2 views along the height, so each view is half the total height
      renderWidth = width;
      renderHeight = height / 2;
   }
   else
   {
      // Use the effective size of the created device's window (should be the same as the requested)
      renderWidth = width;
      renderHeight = height;
   }
   
   // set the expected viewport for the newly created device
   m_viewPort.X = 0;
   m_viewPort.Y = 0;
   m_viewPort.Width = renderWidth;
   m_viewPort.Height = renderHeight;
   m_viewPort.MinZ = 0.0f;
   m_viewPort.MaxZ = 1.0f;

   try {
      m_pd3dPrimaryDevice = new RenderDevice(g_pplayer->GetHwnd(), renderWidth, renderHeight, fullScreen, colordepth, 
         m_AAfactor, stereo3D, useNvidiaApi, disableDWM, m_BWrendering, nMSAASamples, refreshrate, syncMode, adapter);
   }
   catch (...) {
      // TODO better error handling => just let the exception up ?
      throw(E_FAIL);
   }
   
   if ((m_pd3dPrimaryDevice->GetOutputBackBuffer()->GetColorFormat() == colorFormat::RGBA10) && (m_FXAA == Quality_SMAA || m_FXAA == Standard_DLAA))
      ShowError("SMAA or DLAA post-processing AA should not be combined with 10bit-output rendering (will result in visible artifacts)!");

#ifndef __STANDALONE__
   BAMView::init();
#endif

   const bool compressTextures = m_table->m_settings.LoadValueWithDefault(Settings::Player, "CompressTextures"s, false);
   m_pd3dPrimaryDevice->CompressTextures(compressTextures);

   m_pd3dPrimaryDevice->SetViewport(&m_viewPort);

   //

   if (m_stereo3D == STEREO_VR)
      m_backGlass = new BackGlass(
         m_pd3dPrimaryDevice, m_table->GetDecalsEnabled() ? m_table->GetImage(m_table->m_BG_image[m_table->m_BG_current_set]) : nullptr);
   else
      m_backGlass = nullptr;

   // This used to be a spheremap BMP, upgraded in 10.8 for an equirectangular HDR env map
   m_pinballEnvTexture.LoadFromFile(g_pvp->m_szMyPath + "assets" + PATH_SEPARATOR_CHAR + "BallEnv.exr");
   m_aoDitherTexture.LoadFromFile(g_pvp->m_szMyPath + "assets" + PATH_SEPARATOR_CHAR + "AODither.webp");
   m_builtinEnvTexture.LoadFromFile(g_pvp->m_szMyPath + "assets" + PATH_SEPARATOR_CHAR + "EnvMap.webp");
   m_envTexture = m_table->GetImage(m_table->m_envImage);
   PLOGI << "Computing environment map radiance"; // For profiling
   #ifdef ENABLE_SDL // OpenGL
   Texture* const envTex = m_envTexture ? m_envTexture : &m_builtinEnvTexture;
   const int envTexHeight = min(envTex->m_pdsBuffer->height(), 256u) / 8;
   const int envTexWidth = envTexHeight * 2;
#ifndef __OPENGLES__
   const colorFormat rad_format = envTex->m_pdsBuffer->m_format == BaseTexture::RGB_FP32 ? colorFormat::RGBA32F : colorFormat::RGBA16F;
   m_envRadianceTexture = new RenderTarget(m_pd3dPrimaryDevice, SurfaceType::RT_DEFAULT, "Irradiance"s, envTexWidth, envTexHeight, rad_format, false, 1, "Failed to create irradiance render target");
   m_pd3dPrimaryDevice->FBShader->SetTechnique(SHADER_TECHNIQUE_irradiance);
   m_pd3dPrimaryDevice->FBShader->SetTexture(SHADER_tex_env, envTex);
   m_pd3dPrimaryDevice->SetRenderTarget("Env Irradiance PreCalc"s, m_envRadianceTexture);
   m_pd3dPrimaryDevice->DrawFullscreenTexturedQuad(m_pd3dPrimaryDevice->FBShader);
   m_pd3dPrimaryDevice->FlushRenderFrame();
   m_pd3dPrimaryDevice->basicShader->SetTexture(SHADER_tex_diffuse_env, m_envRadianceTexture->GetColorSampler());
   m_pd3dPrimaryDevice->m_ballShader->SetTexture(SHADER_tex_diffuse_env, m_envRadianceTexture->GetColorSampler());
#else
   m_envRadianceTexture = EnvmapPrecalc(envTex, envTexWidth, envTexHeight);
   m_pd3dPrimaryDevice->m_texMan.SetDirty(m_envRadianceTexture);
   m_pd3dPrimaryDevice->basicShader->SetTexture(SHADER_tex_diffuse_env, m_envRadianceTexture);
   m_pd3dPrimaryDevice->m_ballShader->SetTexture(SHADER_tex_diffuse_env, m_envRadianceTexture);
#endif
   #else // DirectX 9
   // DirectX 9 does not support bitwise operation in shader, so radical_inverse is not implemented and therefore we use the slow CPU path instead of GPU
   const Texture* const envTex = m_envTexture ? m_envTexture : &m_builtinEnvTexture;
   const unsigned int envTexHeight = min(envTex->m_pdsBuffer->height(), 256u) / 8;
   const unsigned int envTexWidth = envTexHeight * 2;
   m_envRadianceTexture = EnvmapPrecalc(envTex, envTexWidth, envTexHeight);
   m_pd3dPrimaryDevice->m_texMan.SetDirty(m_envRadianceTexture);
   m_pd3dPrimaryDevice->basicShader->SetTexture(SHADER_tex_diffuse_env, m_envRadianceTexture);
   m_pd3dPrimaryDevice->m_ballShader->SetTexture(SHADER_tex_diffuse_env, m_envRadianceTexture);
   #endif
   PLOGI << "Environment map radiance computed"; // For profiling

   const bool lowDetailBall = (m_table->GetDetailLevel() < 10);
   IndexBuffer* ballIndexBuffer
      = new IndexBuffer(m_pd3dPrimaryDevice, lowDetailBall ? basicBallLoNumFaces : basicBallMidNumFaces, lowDetailBall ? basicBallLoIndices : basicBallMidIndices);
   VertexBuffer* ballVertexBuffer
      = new VertexBuffer(m_pd3dPrimaryDevice, lowDetailBall ? basicBallLoNumVertices : basicBallMidNumVertices, (float*)(lowDetailBall ? basicBallLo : basicBallMid));
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

      VertexBuffer* ballDebugPoints = new VertexBuffer(m_pd3dPrimaryDevice, (unsigned int)ballDbgVtx.size(), (float*)ballDbgVtx.data(), false);
      m_ballDebugPoints = new MeshBuffer(L"Ball.Debug"s, ballDebugPoints);
   }
#endif
   // Support up to 64 balls, that should be sufficient
   VertexBuffer* ballTrailVertexBuffer = new VertexBuffer(m_pd3dPrimaryDevice, 64 * (MAX_BALL_TRAIL_POS - 2) * 2 + 4, nullptr, true);
   m_ballTrailMeshBuffer = new MeshBuffer(L"Ball.Trail"s, ballTrailVertexBuffer);

   // FIXME we always loads the LUT since this can be changed in the LiveUI. Would be better to do this lazily
   //if (m_toneMapper == TM_TONY_MC_MAPFACE)
   {
      m_tonemapLUT = new Texture();
      m_tonemapLUT->LoadFromFile(g_pvp->m_szMyPath + "assets" + PATH_SEPARATOR_CHAR + "tony_mc_mapface_unrolled.exr");
      m_pd3dPrimaryDevice->FBShader->SetTexture(SHADER_tex_tonemap_lut, m_tonemapLUT, SF_BILINEAR, SA_CLAMP, SA_CLAMP, true);
   }

   m_pd3dPrimaryDevice->ResetRenderState();
   #ifndef ENABLE_SDL
   CHECKD3D(m_pd3dPrimaryDevice->GetCoreDevice()->SetRenderState(D3DRS_LIGHTING, FALSE));
   CHECKD3D(m_pd3dPrimaryDevice->GetCoreDevice()->SetRenderState(D3DRS_CLIPPING, FALSE));
   CHECKD3D(m_pd3dPrimaryDevice->GetCoreDevice()->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1));
   CHECKD3D(m_pd3dPrimaryDevice->GetCoreDevice()->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE));
   CHECKD3D(m_pd3dPrimaryDevice->GetCoreDevice()->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0));
   CHECKD3D(m_pd3dPrimaryDevice->GetCoreDevice()->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE));
   CHECKD3D(m_pd3dPrimaryDevice->GetCoreDevice()->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE));
   CHECKD3D(m_pd3dPrimaryDevice->GetCoreDevice()->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TFACTOR)); // default tfactor: 1,1,1,1
   // 0 means disable limiting of draw-ahead queue
   int maxPrerenderedFrames = m_stereo3D == STEREO_VR ? 0 : m_table->m_settings.LoadValueWithDefault(Settings::Player, "MaxPrerenderedFrames"s, 0);
   m_limiter = new FrameQueueLimiter(m_pd3dPrimaryDevice, maxPrerenderedFrames);
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
   delete m_pd3dPrimaryDevice;
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
   delete m_limiter;
}

void Renderer::TransformVertices(const Vertex3D_NoTex2 * const __restrict rgv, const WORD * const __restrict rgi, const int count, Vertex2D * const __restrict rgvout) const
{
   RECT viewport { 0, 0, (LONG)m_viewPort.Width, (LONG)m_viewPort.Height };
   m_mvp->GetModelViewProj(0).TransformVertices(rgv, rgi, count, rgvout, viewport);
}

void Renderer::TransformVertices(const Vertex3Ds* const __restrict rgv, const WORD* const __restrict rgi, const int count, Vertex2D* const __restrict rgvout) const
{
   RECT viewport { 0, 0, (LONG)m_viewPort.Width, (LONG)m_viewPort.Height };
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
   m_pd3dPrimaryDevice->ResetRenderState();
   m_pd3dPrimaryDevice->SetRenderState(RenderState::CULLMODE, RenderState::CULL_CCW);
   if (pin)
   {
      m_pd3dPrimaryDevice->Clear(clearType::ZBUFFER, 0, 1.0f, 0L);
      m_pd3dPrimaryDevice->SetRenderState(RenderState::ZWRITEENABLE, RenderState::RS_FALSE);
      m_pd3dPrimaryDevice->SetRenderState(RenderState::ZENABLE, RenderState::RS_FALSE);
      m_pd3dPrimaryDevice->SetRenderState(RenderState::ALPHABLENDENABLE, RenderState::RS_FALSE);
      // FIXME this should be called with a trilinear/anisotropy filtering override
      g_pplayer->m_renderer->DrawSprite(0.f, 0.f, 1.f, 1.f, 0xFFFFFFFF, pin, ptable->m_ImageBackdropNightDay ? sqrtf(m_globalEmissionScale) : 1.0f, true);
   }
   else
   {
      const D3DCOLOR d3dcolor = COLORREF_to_D3DCOLOR(ptable->m_colorbackdrop);
      m_pd3dPrimaryDevice->Clear(clearType::TARGET | clearType::ZBUFFER, d3dcolor, 1.0f, 0L);
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
   #ifdef ENABLE_SDL
   bool stereo = m_stereo3D != STEREO_OFF && m_stereo3D != STEREO_VR && m_stereo3Denabled;
   #else
   bool stereo = false;
   #endif
   if (viewSetup.mMode == VLM_WINDOW)
      viewSetup.SetWindowModeFromSettings(m_table);
   viewSetup.ComputeMVP(m_table, m_viewPort.Width, m_viewPort.Height, stereo, *m_mvp, vec3(m_cam.x, m_cam.y, m_cam.z), m_inc, xpixoff, ypixoff);
   SetupShaders();
}

Vertex3Ds Renderer::Unproject(const Vertex3Ds& point)
{
   Matrix3D m2 = m_mvp->GetModelViewProj(0);
   m2.Invert();
   const Vertex3Ds p(
       2.0f * (point.x - (float)m_viewPort.X) / (float)m_viewPort.Width - 1.0f,
       1.0f - 2.0f * (point.y - (float)m_viewPort.Y) / (float)m_viewPort.Height,
       (point.z - m_viewPort.MinZ) / (m_viewPort.MaxZ - m_viewPort.MinZ));
   const Vertex3Ds p3 = m2.MultiplyVector(p);
   return p3;
}

Vertex3Ds Renderer::Get3DPointFrom2D(const POINT& p)
{
   const Vertex3Ds pNear((float)p.x,(float)p.y,m_viewPort.MinZ);
   const Vertex3Ds pFar ((float)p.x,(float)p.y,m_viewPort.MaxZ);
   const Vertex3Ds p1 = Unproject(pNear);
   const Vertex3Ds p2 = Unproject(pFar);
   const float wz = 0.f;
   const float wx = ((wz - p1.z)*(p2.x - p1.x)) / (p2.z - p1.z) + p1.x;
   const float wy = ((wz - p1.z)*(p2.y - p1.y)) / (p2.z - p1.z) + p1.y;
   const Vertex3Ds vertex(wx, wy, wz);
   return vertex;
}

void Renderer::SetupShaders()
{
   const vec4 envEmissionScale_TexWidth(m_table->m_envEmissionScale * m_globalEmissionScale,
      (float) (m_envTexture ? *m_envTexture : m_builtinEnvTexture).m_height /*+m_builtinEnvTexture.m_width)*0.5f*/, 0.f, 0.f); //!! dto.

   UpdateBasicShaderMatrix();
   m_pd3dPrimaryDevice->basicShader->SetTexture(SHADER_tex_env, m_envTexture ? m_envTexture : &m_builtinEnvTexture);
   m_pd3dPrimaryDevice->basicShader->SetVector(SHADER_fenvEmissionScale_TexWidth, &envEmissionScale_TexWidth);

   UpdateBallShaderMatrix();
   const vec4 st(m_table->m_envEmissionScale * m_globalEmissionScale, m_envTexture ? (float)m_envTexture->m_height/*+m_envTexture->m_width)*0.5f*/ : (float)m_builtinEnvTexture.m_height/*+m_builtinEnvTexture.m_width)*0.5f*/, 0.f, 0.f);
   m_pd3dPrimaryDevice->m_ballShader->SetVector(SHADER_fenvEmissionScale_TexWidth, &st);
   m_pd3dPrimaryDevice->m_ballShader->SetVector(SHADER_fenvEmissionScale_TexWidth, &envEmissionScale_TexWidth);
   //m_pd3dPrimaryDevice->m_ballShader->SetInt("iLightPointNum",MAX_LIGHT_SOURCES);

   constexpr float Roughness = 0.8f;
   m_pd3dPrimaryDevice->m_ballShader->SetVector(SHADER_Roughness_WrapL_Edge_Thickness, exp2f(10.0f * Roughness + 1.0f), 0.f, 1.f, 0.05f);
   vec4 amb_lr = convertColor(m_table->m_lightAmbient, m_table->m_lightRange);
   m_pd3dPrimaryDevice->m_ballShader->SetVector(SHADER_cAmbient_LightRange, 
      amb_lr.x * m_globalEmissionScale, amb_lr.y * m_globalEmissionScale, amb_lr.z * m_globalEmissionScale, m_table->m_lightRange);

   //m_pd3dPrimaryDevice->basicShader->SetInt("iLightPointNum",MAX_LIGHT_SOURCES);

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

#ifdef ENABLE_SDL
   float lightPos[MAX_LIGHT_SOURCES][4] = { 0.f };
   float lightEmission[MAX_LIGHT_SOURCES][4] = { 0.f };

   for (unsigned int i = 0; i < MAX_LIGHT_SOURCES; ++i)
   {
      memcpy(&lightPos[i], &m_table->m_Light[i].pos, sizeof(float) * 3);
      memcpy(&lightEmission[i], &emission, sizeof(float) * 3);
   }

   m_pd3dPrimaryDevice->basicShader->SetFloat4v(SHADER_basicLightPos, (vec4*)lightPos, MAX_LIGHT_SOURCES);
   m_pd3dPrimaryDevice->basicShader->SetFloat4v(SHADER_basicLightEmission, (vec4*)lightEmission, MAX_LIGHT_SOURCES);
#else
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

   m_pd3dPrimaryDevice->basicShader->SetFloat4v(SHADER_basicPackedLights, (vec4*)l, sizeof(CLight) * MAX_LIGHT_SOURCES / (4 * sizeof(float)));
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

#ifdef ENABLE_SDL // OpenGL
   const int nEyes = m_pd3dPrimaryDevice->m_stereo3D != STEREO_OFF ? 2 : 1;
   for (int eye = 0; eye < nEyes; eye++)
      matrices.matWorldViewProj[eye] = GetMVP().GetModelViewProj(eye);
   m_pd3dPrimaryDevice->flasherShader->SetMatrix(SHADER_matWorldViewProj, &matrices.matWorldViewProj[0].m[0][0], nEyes);
   m_pd3dPrimaryDevice->lightShader->SetMatrix(SHADER_matWorldViewProj, &matrices.matWorldViewProj[0].m[0][0], nEyes);
   m_pd3dPrimaryDevice->DMDShader->SetMatrix(SHADER_matWorldViewProj, &matrices.matWorldViewProj[0].m[0][0], nEyes);
   m_pd3dPrimaryDevice->basicShader->SetUniformBlock(SHADER_basicMatrixBlock, &matrices.matWorld.m[0][0]);

#else // DirectX 9
   matrices.matWorldViewProj[0] = GetMVP().GetModelViewProj(0);
   m_pd3dPrimaryDevice->basicShader->SetMatrix(SHADER_matWorld, &matrices.matWorld);
   m_pd3dPrimaryDevice->basicShader->SetMatrix(SHADER_matView, &matrices.matView);
   m_pd3dPrimaryDevice->basicShader->SetMatrix(SHADER_matWorldView, &matrices.matWorldView);
   m_pd3dPrimaryDevice->basicShader->SetMatrix(SHADER_matWorldViewInverseTranspose, &matrices.matWorldViewInverseTranspose);
   m_pd3dPrimaryDevice->basicShader->SetMatrix(SHADER_matWorldViewProj, &matrices.matWorldViewProj[0]);
   m_pd3dPrimaryDevice->flasherShader->SetMatrix(SHADER_matWorldViewProj, &matrices.matWorldViewProj[0]);
   m_pd3dPrimaryDevice->lightShader->SetMatrix(SHADER_matWorldViewProj, &matrices.matWorldViewProj[0]);
   m_pd3dPrimaryDevice->DMDShader->SetMatrix(SHADER_matWorldViewProj, &matrices.matWorldViewProj[0]);
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
#ifdef ENABLE_SDL
   const int nEyes = m_pd3dPrimaryDevice->m_stereo3D != STEREO_OFF ? 2 : 1;
   for (int eye = 0; eye < nEyes; eye++)
      matrices.matWorldViewProj[eye] = GetMVP().GetModelViewProj(eye);
   m_pd3dPrimaryDevice->m_ballShader->SetUniformBlock(SHADER_ballMatrixBlock, &matrices.matView.m[0][0]);
#else
   matrices.matWorldViewProj[0] = GetMVP().GetModelViewProj(0);
   m_pd3dPrimaryDevice->m_ballShader->SetMatrix(SHADER_matWorldViewProj, &matrices.matWorldViewProj[0]);
   m_pd3dPrimaryDevice->m_ballShader->SetMatrix(SHADER_matWorldView, &matrices.matWorldView);
   m_pd3dPrimaryDevice->m_ballShader->SetMatrix(SHADER_matWorldViewInverse, &matrices.matWorldViewInverse);
   m_pd3dPrimaryDevice->m_ballShader->SetMatrix(SHADER_matView, &matrices.matView);
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
      m_table->mViewSetups[m_table->m_BG_current_set].ComputeMVP(m_ptable, m_viewPort.Width, m_viewPort.Height, true, stereoMVP);
      RECT viewport { 0, 0, (LONG)m_viewPort.Width, (LONG)m_viewPort.Height };
      vec3 deepPt(0.f, 0.f, 0.f); // = 5000.f * stereoMVP.GetModelViewInverse().GetOrthoNormalDir();
      Vertex2D projLeft, projRight;
      stereoMVP.GetModelViewProj(0).TransformVertices(&deepPt, nullptr, 1, &projLeft, viewport);
      stereoMVP.GetModelViewProj(1).TransformVertices(&deepPt, nullptr, 1, &projRight, viewport);*/
      const float eyeSeparation = m_table->GetMaxSeparation();
      const float zpd = m_table->GetZPD();
      const bool swapAxis = m_table->m_settings.LoadValueWithDefault(Settings::Player, "Stereo3DYAxis"s, false); // Swap X/Y axis
      m_pd3dPrimaryDevice->StereoShader->SetVector(SHADER_Stereo_MS_ZPD_YAxis, eyeSeparation, zpd, swapAxis ? 1.0f : 0.0f, 0.0f);
   }
   
   RenderTarget *renderedRT = m_pd3dPrimaryDevice->GetPostProcessRenderTarget1();
   #ifdef ENABLE_SDL
   if (m_stereo3DfakeStereo) // OpenGL strip down this uniform which is only needed for interlaced mode on DirectX 9
   #endif
   m_pd3dPrimaryDevice->StereoShader->SetVector(SHADER_w_h_height, (float)(1.0 / renderedRT->GetWidth()), (float)(1.0 / renderedRT->GetHeight()), (float)renderedRT->GetHeight(), m_table->Get3DOffset());

   m_stereo3DDefocus = 0.f;
   if (IsAnaglyphStereoMode(m_stereo3D))
   {
      Anaglyph anaglyph;
      anaglyph.LoadSetupFromRegistry(clamp(m_stereo3D - STEREO_ANAGLYPH_1, 0, 9));
      anaglyph.SetupShader(m_pd3dPrimaryDevice->StereoShader);
      // The defocus kernel size should depend on the render resolution but since this is a user tweak, this doesn't matter that much
      m_stereo3DDefocus = m_table->m_settings.LoadValueWithDefault(Settings::Player, "Stereo3DDefocus"s, 0.f);
      // TODO I'm not 100% sure about this. I think the right way would be to select based on the transmitted luminance of the filter, the defocus 
      // being done on the lowest of the 2. Here we do on the single color channel, which is the same most of the time but not always (f.e. green/magenta)
      if (anaglyph.IsReversedColorPair())
         m_stereo3DDefocus = -m_stereo3DDefocus;
   }
   else
   {
      m_pd3dPrimaryDevice->StereoShader->SetTechnique(m_stereo3D == STEREO_SBS ? SHADER_TECHNIQUE_stereo_SBS 
                                                    : m_stereo3D == STEREO_TB  ? SHADER_TECHNIQUE_stereo_TB
                                                    : m_stereo3D == STEREO_INT ? SHADER_TECHNIQUE_stereo_Int 
                                                    :                            SHADER_TECHNIQUE_stereo_Flipped_Int);
   }
}

void Renderer::PrepareFrame()
{
   // Update camera point of view
   #ifdef ENABLE_VR
   if (m_stereo3D == STEREO_VR)
      g_pplayer->m_vrDevice->UpdateVRPosition(GetMVP());
   else 
   #endif
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
   else if (g_pplayer->m_liveUI->IsTweakMode())
      InitLayout();

   // Update staticly prerendered parts if needed
   RenderStaticPrepass();

   // Start from the prerendered parts/background or a clear background for VR
   m_pd3dPrimaryDevice->SetRenderTarget("Render Scene"s, m_pd3dPrimaryDevice->GetMSAABackBufferTexture());
   if (m_stereo3D == STEREO_VR || g_pplayer->GetInfoMode() == IF_DYNAMIC_ONLY)
      m_pd3dPrimaryDevice->Clear(clearType::TARGET | clearType::ZBUFFER, 0, 1.0f, 0L);
   else
   {
      m_pd3dPrimaryDevice->AddRenderTargetDependency(m_staticPrepassRT);
      m_pd3dPrimaryDevice->BlitRenderTarget(m_staticPrepassRT, m_pd3dPrimaryDevice->GetMSAABackBufferTexture());
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
   // m_pd3dPrimaryDevice->AddRenderTargetDependency(m_pd3dPrimaryDevice->GetPreviousBackBufferTexture());
   m_pd3dPrimaryDevice->m_ballShader->SetTexture(SHADER_tex_ball_playfield, m_pd3dPrimaryDevice->GetPreviousBackBufferTexture()->GetColorSampler());

   if (g_pplayer->GetInfoMode() != IF_STATIC_ONLY)
      RenderDynamics();

   // Resolve MSAA buffer to a normal one (noop if not using MSAA), allowing sampling it for postprocessing
   m_pd3dPrimaryDevice->ResolveMSAA();
}

void Renderer::SubmitFrame()
{
   // Submit to GPU render queue
   g_frameProfiler.EnterProfileSection(FrameProfiler::PROFILE_GPU_SUBMIT);
   m_pd3dPrimaryDevice->FlushRenderFrame();
   m_pd3dPrimaryDevice->SwapBackBufferRenderTargets(); // Keep previous render as a reflection probe for ball reflection and for hires motion blur
   // (Optionally) force queue flushing of the driver. Can be used to artifically limit latency on DX9 (depends on OS/GFXboard/driver if still useful nowadays). This must be done after submiting render commands
   if (m_limiter)
      m_limiter->Execute();
   g_frameProfiler.ExitProfileSection();
}

void Renderer::DrawBulbLightBuffer()
{
   RenderDevice* p3dDevice = m_pd3dPrimaryDevice;
   const RenderPass *initial_rt = p3dDevice->GetCurrentPass();
   static int id = 0; id++;

   // switch to 'bloom' output buffer to collect all bulb lights
   p3dDevice->SetRenderTarget("Transmitted Light " + std::to_string(id) + " Clear", p3dDevice->GetBloomBufferTexture(), false);
   p3dDevice->ResetRenderState();
   p3dDevice->Clear(clearType::TARGET, 0, 1.0f, 0L);

   // Draw bulb lights
   m_render_mask |= Renderer::LIGHT_BUFFER;
   p3dDevice->SetRenderTarget("Transmitted Light " + std::to_string(id), p3dDevice->GetBloomBufferTexture(), true, true);
   p3dDevice->SetRenderState(RenderState::ZENABLE, RenderState::RS_FALSE); // disable all z-tests as zbuffer is in different resolution
   for (Hitable *hitable : g_pplayer->m_vhitables)
      if (hitable->HitableGetItemType() == eItemLight)
         hitable->Render(m_render_mask);
   m_render_mask &= ~Renderer::LIGHT_BUFFER;

   bool hasLight = p3dDevice->GetCurrentPass()->GetCommandCount() > 1;
   if (hasLight)
   { // Only apply blur if we have actually rendered some lights
      RenderPass* renderPass = p3dDevice->GetCurrentPass();
      p3dDevice->DrawGaussianBlur(
         p3dDevice->GetBloomBufferTexture(), 
         p3dDevice->GetBloomTmpBufferTexture(), 
         p3dDevice->GetBloomBufferTexture(), 19.f); // FIXME kernel size should depend on buffer resolution
      RenderPass *blurPass2 = p3dDevice->GetCurrentPass();
      RenderPass *blurPass1 = blurPass2->m_dependencies[0];
      constexpr float margin = 0.05f; // margin for the blur
      blurPass1->m_areaOfInterest.x = renderPass->m_areaOfInterest.x - margin;
      blurPass1->m_areaOfInterest.y = renderPass->m_areaOfInterest.y - margin;
      blurPass1->m_areaOfInterest.z = renderPass->m_areaOfInterest.z + margin;
      blurPass1->m_areaOfInterest.w = renderPass->m_areaOfInterest.w + margin;
      blurPass2->m_areaOfInterest = blurPass1->m_areaOfInterest;
   }

   // Restore state and render target
   p3dDevice->SetRenderTarget(initial_rt->m_name + '+', initial_rt->m_rt);

   if (hasLight)
   {
      // Declare dependency on Bulb Light buffer (actually rendered to the bloom buffer texture)
      m_pd3dPrimaryDevice->AddRenderTargetDependency(m_pd3dPrimaryDevice->GetBloomBufferTexture());
      p3dDevice->basicShader->SetTexture(SHADER_tex_base_transmission, p3dDevice->GetBloomBufferTexture()->GetColorSampler());
   } 
   else
   {
      p3dDevice->basicShader->SetTextureNull(SHADER_tex_base_transmission);
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
   if (!onlyBalls)
   {
      // Update Bulb light buffer and set up render pass dependencies
      DrawBulbLightBuffer();

      // Draw all parts
      const unsigned int mask = m_render_mask;
      m_render_mask |= Renderer::DYNAMIC_ONLY;
      for (Hitable* hitable : g_pplayer->m_vhitables)
         hitable->Render(m_render_mask);
      m_render_mask = mask;
   }

   for (Ball* ball : g_pplayer->m_vball)
      ball->m_pballex->Render(m_render_mask);
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
   m_pd3dPrimaryDevice->DMDShader->SetVector(SHADER_vColor_Intensity, &c);
   m_pd3dPrimaryDevice->DMDShader->SetTechnique(tex ? SHADER_TECHNIQUE_basic_noDMD : SHADER_TECHNIQUE_basic_noDMD_notex);
   if (tex)
      m_pd3dPrimaryDevice->DMDShader->SetTexture(SHADER_tex_sprite, tex, SF_NONE, SA_REPEAT, SA_REPEAT);
   m_pd3dPrimaryDevice->SetRenderState(RenderState::ZENABLE, RenderState::RS_FALSE);
   m_pd3dPrimaryDevice->DrawTexturedQuad(m_pd3dPrimaryDevice->DMDShader, vertices);
   m_pd3dPrimaryDevice->GetCurrentPass()->m_commands.back()->SetTransparent(true);
   m_pd3dPrimaryDevice->GetCurrentPass()->m_commands.back()->SetDepth(-10000.f);
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
   m_pd3dPrimaryDevice->DMDShader->SetVector(SHADER_vColor_Intensity, &c);
   m_pd3dPrimaryDevice->DMDShader->SetTechnique(tex ? SHADER_TECHNIQUE_basic_noDMD : SHADER_TECHNIQUE_basic_noDMD_notex);
   if (tex)
      m_pd3dPrimaryDevice->DMDShader->SetTexture(SHADER_tex_sprite, tex);
   m_pd3dPrimaryDevice->SetRenderState(RenderState::ZENABLE, RenderState::RS_FALSE);
   m_pd3dPrimaryDevice->DrawTexturedQuad(m_pd3dPrimaryDevice->DMDShader, vertices);
   m_pd3dPrimaryDevice->GetCurrentPass()->m_commands.back()->SetTransparent(true);
   m_pd3dPrimaryDevice->GetCurrentPass()->m_commands.back()->SetDepth(-10000.f);
}

bool Renderer::IsUsingStaticPrepass() const
{
   return !m_disableStaticPrepass && m_stereo3D != STEREO_VR && !g_pplayer->m_headTracking;
}

void Renderer::RenderStaticPrepass()
{
   // For VR, we don't use any static pre-rendering
   if (m_stereo3D == STEREO_VR)
      return;

   if (!m_isStaticPrepassDirty)
      return;

   m_isStaticPrepassDirty = false;

   TRACE_FUNCTION();

   m_pd3dPrimaryDevice->FlushRenderFrame();
   m_render_mask |= Renderer::STATIC_ONLY;

   // The code will fail if the static render target is MSAA (the copy operation we are performing is not allowed)
   delete m_staticPrepassRT;
   m_staticPrepassRT = m_pd3dPrimaryDevice->GetBackBufferTexture()->Duplicate("StaticPreRender"s);
   assert(!m_staticPrepassRT->IsMSAA());
   
   RenderTarget *accumulationSurface = IsUsingStaticPrepass() ? m_staticPrepassRT->Duplicate("Accumulation"s) : nullptr;

   RenderTarget* renderRT = GetAOMode() == 1 ? m_pd3dPrimaryDevice->GetBackBufferTexture() : m_staticPrepassRT;

   // if rendering static/with heavy oversampling, disable the aniso/trilinear filter to get a sharper/more precise result overall!
   if (IsUsingStaticPrepass())
   {
      PLOGI << "Performing prerendering of static parts."; // For profiling
      m_pd3dPrimaryDevice->SetMainTextureDefaultFiltering(SF_BILINEAR);
   }

   //#define STATIC_PRERENDER_ITERATIONS_KOROBOV 7.0 // for the (commented out) lattice-based QMC oversampling, 'magic factor', depending on the the number of iterations!
   // loop for X times and accumulate/average these renderings
   // NOTE: iter == 0 MUST ALWAYS PRODUCE an offset of 0,0!
   int n_iter = IsUsingStaticPrepass() ? (STATIC_PRERENDER_ITERATIONS - 1) : 0;
   for (int iter = n_iter; iter >= 0; --iter) // just do one iteration if in dynamic camera/light/material tweaking mode
   {
      m_pd3dPrimaryDevice->m_curDrawnTriangles = 0;

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
      m_pd3dPrimaryDevice->SetRenderTarget("PreRender Background"s, renderRT, false);
      DrawBackground();
      m_pd3dPrimaryDevice->FlushRenderFrame();

      m_pd3dPrimaryDevice->SetRenderTarget("PreRender Draw"s, renderRT);

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
         m_pd3dPrimaryDevice->SetRenderTarget("PreRender Accumulate"s, accumulationSurface);
         m_pd3dPrimaryDevice->AddRenderTargetDependency(renderRT);
         m_pd3dPrimaryDevice->ResetRenderState();
         m_pd3dPrimaryDevice->SetRenderState(RenderState::ALPHABLENDENABLE, iter == STATIC_PRERENDER_ITERATIONS - 1 ? RenderState::RS_FALSE : RenderState::RS_TRUE);
         m_pd3dPrimaryDevice->SetRenderState(RenderState::SRCBLEND, RenderState::ONE);
         m_pd3dPrimaryDevice->SetRenderState(RenderState::DESTBLEND, RenderState::ONE);
         m_pd3dPrimaryDevice->SetRenderState(RenderState::BLENDOP, RenderState::BLENDOP_ADD);
         m_pd3dPrimaryDevice->SetRenderState(RenderState::ZENABLE, RenderState::RS_FALSE);
         m_pd3dPrimaryDevice->SetRenderState(RenderState::ZWRITEENABLE, RenderState::RS_FALSE);
         m_pd3dPrimaryDevice->SetRenderState(RenderState::CULLMODE, RenderState::CULL_NONE);
         m_pd3dPrimaryDevice->FBShader->SetTechnique(SHADER_TECHNIQUE_fb_mirror);
         m_pd3dPrimaryDevice->FBShader->SetVector(SHADER_w_h_height, 
            (float)(1.0 / (double)renderRT->GetWidth()), (float)(1.0 / (double)renderRT->GetHeight()),
            (float)((double)STATIC_PRERENDER_ITERATIONS), 1.0f);
         m_pd3dPrimaryDevice->FBShader->SetTexture(SHADER_tex_fb_unfiltered, renderRT->GetColorSampler());
         m_pd3dPrimaryDevice->DrawFullscreenTexturedQuad(m_pd3dPrimaryDevice->FBShader);
         m_pd3dPrimaryDevice->FBShader->SetTextureNull(SHADER_tex_fb_unfiltered);
      }

      // Finish the frame.
      m_pd3dPrimaryDevice->FlushRenderFrame();
#ifndef __STANDALONE__
      if (g_pplayer->m_pEditorTable->m_progressDialog.IsWindow())
         g_pplayer->m_pEditorTable->m_progressDialog.SetProgress(70 + (((30 * (n_iter + 1 - iter)) / (n_iter + 1))));
#endif
   }

   if (accumulationSurface)
   {
      // copy back weighted antialiased color result to the static render target, keeping depth untouched
      m_pd3dPrimaryDevice->SetRenderTarget("PreRender Store"s, renderRT);
      m_pd3dPrimaryDevice->BlitRenderTarget(accumulationSurface, renderRT, true, false);
      m_pd3dPrimaryDevice->FlushRenderFrame(); // Execute before destroying the render target
      delete accumulationSurface;
   }

   // if rendering static/with heavy oversampling, re-enable the aniso/trilinear filter now for the normal rendering
   const bool forceAniso = m_table->m_settings.LoadValueWithDefault(Settings::Player, "ForceAnisotropicFiltering"s, true);
   m_pd3dPrimaryDevice->SetMainTextureDefaultFiltering(forceAniso ? SF_ANISOTROPIC : SF_TRILINEAR);

   // Now finalize static buffer with static AO
   if (GetAOMode() == 1)
   {
      PLOGI << "Starting static AO prerendering"; // For profiling

      const bool useAA = m_AAfactor != 1.0f;

      m_pd3dPrimaryDevice->SetRenderTarget("PreRender AO Save Depth"s, m_staticPrepassRT);
      m_pd3dPrimaryDevice->ResetRenderState();
      m_pd3dPrimaryDevice->BlitRenderTarget(renderRT, m_staticPrepassRT, false, true);

      m_pd3dPrimaryDevice->SetRenderState(RenderState::ALPHABLENDENABLE, RenderState::RS_FALSE);
      m_pd3dPrimaryDevice->SetRenderState(RenderState::CULLMODE ,RenderState::CULL_NONE);
      m_pd3dPrimaryDevice->SetRenderState(RenderState::ZWRITEENABLE, RenderState::RS_FALSE);
      m_pd3dPrimaryDevice->SetRenderState(RenderState::ZENABLE, RenderState::RS_FALSE);

      m_pd3dPrimaryDevice->FBShader->SetTexture(SHADER_tex_depth, renderRT->GetDepthSampler());
      m_pd3dPrimaryDevice->FBShader->SetTexture(SHADER_tex_ao_dither, &m_aoDitherTexture, SF_NONE, SA_REPEAT, SA_REPEAT, true); // FIXME the force linear RGB is not honored in VR
      m_pd3dPrimaryDevice->FBShader->SetVector(SHADER_AO_scale_timeblur, m_table->m_AOScale, 0.1f, 0.f, 0.f);
      m_pd3dPrimaryDevice->FBShader->SetTechnique(SHADER_TECHNIQUE_AO);

      for (unsigned int i = 0; i < 50; ++i) // 50 iterations to get AO smooth
      {
         m_pd3dPrimaryDevice->SetRenderTarget("PreRender AO"s, m_pd3dPrimaryDevice->GetAORenderTarget(0));
         m_pd3dPrimaryDevice->AddRenderTargetDependency(renderRT);
         m_pd3dPrimaryDevice->AddRenderTargetDependency(m_pd3dPrimaryDevice->GetAORenderTarget(1));
         if (i == 0)
            m_pd3dPrimaryDevice->Clear(clearType::TARGET, 0, 1.0f, 0L);

         m_pd3dPrimaryDevice->FBShader->SetTexture(SHADER_tex_fb_filtered, m_pd3dPrimaryDevice->GetAORenderTarget(1)->GetColorSampler()); //!! ?
         m_pd3dPrimaryDevice->FBShader->SetTexture(SHADER_tex_fb_unfiltered, m_pd3dPrimaryDevice->GetAORenderTarget(1)->GetColorSampler()); //!! ?
         m_pd3dPrimaryDevice->FBShader->SetVector(SHADER_w_h_height, 
            (float)(1.0 / m_pd3dPrimaryDevice->GetAORenderTarget(1)->GetWidth()), (float)(1.0 / m_pd3dPrimaryDevice->GetAORenderTarget(1)->GetHeight()),
            radical_inverse(i) * (float)(1. / 8.0), /*sobol*/ radical_inverse<3>(i) * (float)(1. / 8.0)); // jitter within (64/8)x(64/8) neighborhood of 64x64 tex, good compromise between blotches and noise
         m_pd3dPrimaryDevice->DrawFullscreenTexturedQuad(m_pd3dPrimaryDevice->FBShader);

         // flip AO buffers (avoids copy)
         m_pd3dPrimaryDevice->SwapAORenderTargets();
      }

      m_pd3dPrimaryDevice->FBShader->SetTextureNull(SHADER_tex_depth);

      m_pd3dPrimaryDevice->SetRenderTarget("PreRender Apply AO"s, m_staticPrepassRT);
      m_pd3dPrimaryDevice->AddRenderTargetDependency(renderRT);
      m_pd3dPrimaryDevice->AddRenderTargetDependency(m_pd3dPrimaryDevice->GetAORenderTarget(1));

      m_pd3dPrimaryDevice->FBShader->SetTexture(SHADER_tex_fb_filtered, renderRT->GetColorSampler());
      m_pd3dPrimaryDevice->FBShader->SetTexture(SHADER_tex_fb_unfiltered, renderRT->GetColorSampler());
      m_pd3dPrimaryDevice->FBShader->SetTexture(SHADER_tex_ao, m_pd3dPrimaryDevice->GetAORenderTarget(1)->GetColorSampler());

      m_pd3dPrimaryDevice->FBShader->SetVector(SHADER_w_h_height, (float)(1.0 / renderRT->GetWidth()), (float)(1.0 / renderRT->GetHeight()), 1.0f, 1.0f);
      m_pd3dPrimaryDevice->FBShader->SetTechnique(useAA ? SHADER_TECHNIQUE_fb_AO_static : SHADER_TECHNIQUE_fb_AO_no_filter_static);

      m_pd3dPrimaryDevice->DrawFullscreenTexturedQuad(m_pd3dPrimaryDevice->FBShader);

      m_pd3dPrimaryDevice->FlushRenderFrame(); // Execute before destroying the render targets

      // Delete buffers: we won't need them anymore since dynamic AO is disabled
      m_pd3dPrimaryDevice->FBShader->SetTextureNull(SHADER_tex_ao);
      m_pd3dPrimaryDevice->ReleaseAORenderTargets();
   }

   if (m_pd3dPrimaryDevice->GetMSAABackBufferTexture()->IsMSAA())
   {
      // Render one frame with MSAA to keep MSAA depth (this adds MSAA to the overlapping parts between statics & dynamics)
      RenderTarget* const renderRTmsaa = m_pd3dPrimaryDevice->GetMSAABackBufferTexture()->Duplicate("MSAAPreRender"s);
      InitLayout();
      m_pd3dPrimaryDevice->SetRenderTarget("PreRender MSAA Background"s, renderRTmsaa, false);
      DrawBackground();
      m_pd3dPrimaryDevice->FlushRenderFrame();
      if (IsUsingStaticPrepass())
      {
         m_pd3dPrimaryDevice->SetRenderTarget("PreRender MSAA Scene"s, renderRTmsaa);
         m_pd3dPrimaryDevice->ResetRenderState();
         for (size_t i = 0; i < m_table->m_vrenderprobe.size(); ++i)
            m_table->m_vrenderprobe[i]->MarkDirty();
         UpdateBasicShaderMatrix();
         for (Hitable* hitable : g_pplayer->m_vhitables)
            hitable->Render(m_render_mask);
         m_pd3dPrimaryDevice->FlushRenderFrame();
      }
      // Copy supersampled color buffer
      m_pd3dPrimaryDevice->SetRenderTarget("PreRender Combine Color"s, renderRTmsaa);
      m_pd3dPrimaryDevice->BlitRenderTarget(m_staticPrepassRT, renderRTmsaa, true, false);
      m_pd3dPrimaryDevice->FlushRenderFrame();
      // Replace with this new MSAA pre render
      RenderTarget *initialPreRender = m_staticPrepassRT;
      m_staticPrepassRT = renderRTmsaa;
      delete initialPreRender;
   }

   if (IsUsingStaticPrepass())
   {
      PLOGI << "Starting Reflection Probe prerendering"; // For profiling
      for (RenderProbe *probe : m_table->m_vrenderprobe)
         probe->PreRenderStatic();
   }

   // Store the total number of triangles prerendered (including ones done for render probes)
   m_statsDrawnStaticTriangles = m_pd3dPrimaryDevice->m_curDrawnTriangles;

   PLOGI << "Static PreRender done"; // For profiling
   
   m_render_mask &= ~Renderer::STATIC_ONLY;
   m_pd3dPrimaryDevice->FlushRenderFrame();
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
   #ifdef ENABLE_SDL
   const int nEyes = m_pd3dPrimaryDevice->m_stereo3D != STEREO_OFF ? 2 : 1;
   for (int eye = 0; eye < nEyes; eye++)
      matProj[eye] = GetMVP().GetProj(eye);
   m_pd3dPrimaryDevice->basicShader->SetMatrix(SHADER_matProj, &matProj[0], nEyes);
   m_pd3dPrimaryDevice->m_ballShader->SetMatrix(SHADER_matProj, &matProj[0], nEyes);
   #else
   matProj[0] = GetMVP().GetProj(0);
   m_pd3dPrimaryDevice->basicShader->SetMatrix(SHADER_matProj, &matProj[0]);
   m_pd3dPrimaryDevice->m_ballShader->SetMatrix(SHADER_matProj, &matProj[0]);
   #endif

   // Update ball pos uniforms
   #define MAX_BALL_SHADOW 8
   vec4 balls[MAX_BALL_SHADOW];
   int p = 0;
   for (size_t i = 0; i < g_pplayer->m_vball.size() && p < MAX_BALL_SHADOW; i++)
   {
      Ball* const pball = g_pplayer->m_vball[i];
      if (!pball->m_visible)
         continue;
      balls[p] = vec4(pball->m_d.m_pos.x, pball->m_d.m_pos.y, pball->m_d.m_pos.z, pball->m_d.m_radius);
      p++;
   }
   for (; p < MAX_BALL_SHADOW; p++)
      balls[p] = vec4(-1000.f, -1000.f, -1000.f, 0.0f);
   m_pd3dPrimaryDevice->lightShader->SetFloat4v(SHADER_balls, balls, MAX_BALL_SHADOW);
   m_pd3dPrimaryDevice->basicShader->SetFloat4v(SHADER_balls, balls, MAX_BALL_SHADOW);
   m_pd3dPrimaryDevice->flasherShader->SetFloat4v(SHADER_balls, balls, MAX_BALL_SHADOW);

   UpdateBasicShaderMatrix();
   UpdateBallShaderMatrix();

   // Render the default backglass without depth write before the table so that it will be visible for tables without a VR backglass but overwriten otherwise
   if (m_backGlass != nullptr)
      m_backGlass->Render();

   m_render_mask = IsUsingStaticPrepass() ? Renderer::DYNAMIC_ONLY : Renderer::DEFAULT;
   DrawBulbLightBuffer();
   for (Hitable* hitable : g_pplayer->m_vhitables)
      hitable->Render(m_render_mask);
   for (Ball* ball : g_pplayer->m_vball)
      ball->m_pballex->Render(m_render_mask);
   m_render_mask = Renderer::DEFAULT;
   
   m_pd3dPrimaryDevice->basicShader->SetTextureNull(SHADER_tex_base_transmission); // need to reset the bulb light texture, as its used as render target for bloom again

   for (size_t i = 0; i < m_table->m_vrenderprobe.size(); ++i)
      m_table->m_vrenderprobe[i]->ApplyAreaOfInterest();

   if (!g_pplayer->m_liveUI->IsTweakMode())
      mixer_draw(); // Draw the mixer volume
}



#pragma region PostProcess

void Renderer::SetScreenOffset(const float x, const float y)
{
   const float rotation = ANGTORAD(m_table->mViewSetups[m_table->m_BG_current_set].GetRotation(m_pd3dPrimaryDevice->m_width, m_pd3dPrimaryDevice->m_height));
   const float c = cosf(-rotation), s = sinf(-rotation);
   m_ScreenOffset.x = x * c - y * s;
   m_ScreenOffset.y = x * s + y * c;
}

void Renderer::SSRefl()
{
   m_pd3dPrimaryDevice->SetRenderTarget("ScreenSpace Reflection"s, m_pd3dPrimaryDevice->GetReflectionBufferTexture(), false);
   m_pd3dPrimaryDevice->AddRenderTargetDependency(m_pd3dPrimaryDevice->GetBackBufferTexture(), true);

   m_pd3dPrimaryDevice->FBShader->SetTexture(SHADER_tex_fb_filtered, m_pd3dPrimaryDevice->GetBackBufferTexture()->GetColorSampler());
   m_pd3dPrimaryDevice->FBShader->SetTexture(SHADER_tex_fb_unfiltered, m_pd3dPrimaryDevice->GetBackBufferTexture()->GetColorSampler());
   m_pd3dPrimaryDevice->FBShader->SetTexture(SHADER_tex_ao_dither, &m_aoDitherTexture, SF_NONE, SA_REPEAT, SA_REPEAT, true); // FIXME the force linear RGB is not honored in VR

   // FIXME check if size should not be taken from renderdevice to account for VR (double width) or supersampling
   m_pd3dPrimaryDevice->FBShader->SetVector(SHADER_w_h_height,
      (float)(1.0 / m_pd3dPrimaryDevice->GetBackBufferTexture()->GetWidth()), (float)(1.0 / m_pd3dPrimaryDevice->GetBackBufferTexture()->GetHeight()), 1.0f /*radical_inverse(m_overall_frames%2048)*/, 1.0f);

   const float rotation = m_table->mViewSetups[m_table->m_BG_current_set].GetRotation(m_pd3dPrimaryDevice->m_width, m_pd3dPrimaryDevice->m_height);
   const vec4 SSR_bumpHeight_fresnelRefl_scale_FS(0.3f, 0.3f, m_table->m_SSRScale, rotation);
   m_pd3dPrimaryDevice->FBShader->SetVector(SHADER_SSR_bumpHeight_fresnelRefl_scale_FS, &SSR_bumpHeight_fresnelRefl_scale_FS);

   m_pd3dPrimaryDevice->FBShader->SetTechnique(SHADER_TECHNIQUE_SSReflection);

   m_pd3dPrimaryDevice->DrawFullscreenTexturedQuad(m_pd3dPrimaryDevice->FBShader);
}

void Renderer::Bloom()
{
   if (m_table->m_bloom_strength <= 0.0f || m_bloomOff || g_pplayer->GetInfoMode() == IF_LIGHT_BUFFER_ONLY)
      return;

   const double w = (double)m_pd3dPrimaryDevice->GetBackBufferTexture()->GetWidth();
   const double h = (double)m_pd3dPrimaryDevice->GetBackBufferTexture()->GetHeight();
   const Vertex3D_TexelOnly shiftedVerts[4] =
   {
      {  1.0f,  1.0f, 0.0f, 1.0f + (float)(2.25 / w), 0.0f + (float)(2.25 / h) },
      { -1.0f,  1.0f, 0.0f, 0.0f + (float)(2.25 / w), 0.0f + (float)(2.25 / h) },
      {  1.0f, -1.0f, 0.0f, 1.0f + (float)(2.25 / w), 1.0f + (float)(2.25 / h) },
      { -1.0f, -1.0f, 0.0f, 0.0f + (float)(2.25 / w), 1.0f + (float)(2.25 / h) }
   };
   {
      m_pd3dPrimaryDevice->FBShader->SetTextureNull(SHADER_tex_fb_filtered);

      // switch to 'bloom' output buffer to collect clipped framebuffer values
      m_pd3dPrimaryDevice->SetRenderTarget("Bloom Cut Off"s, m_pd3dPrimaryDevice->GetBloomBufferTexture(), false);
      m_pd3dPrimaryDevice->AddRenderTargetDependency(m_pd3dPrimaryDevice->GetBackBufferTexture());

      m_pd3dPrimaryDevice->FBShader->SetTexture(SHADER_tex_fb_filtered, m_pd3dPrimaryDevice->GetBackBufferTexture()->GetColorSampler());
      m_pd3dPrimaryDevice->FBShader->SetVector(SHADER_w_h_height, (float) (1.0 / w), (float) (1.0 / h), m_table->m_bloom_strength, 1.0f);
      m_pd3dPrimaryDevice->FBShader->SetTechnique(SHADER_TECHNIQUE_fb_bloom);

      m_pd3dPrimaryDevice->DrawTexturedQuad(m_pd3dPrimaryDevice->FBShader, shiftedVerts);
   }

   m_pd3dPrimaryDevice->DrawGaussianBlur(
      m_pd3dPrimaryDevice->GetBloomBufferTexture(), 
      m_pd3dPrimaryDevice->GetBloomTmpBufferTexture(),
      m_pd3dPrimaryDevice->GetBloomBufferTexture(), 39.f); // FIXME kernel size should depend on buffer resolution
}

void Renderer::PrepareVideoBuffers()
{
   const bool useAA = m_AAfactor > 1.0f;
   const bool stereo = m_stereo3D == STEREO_VR || ((m_stereo3D != STEREO_OFF) && m_stereo3Denabled && (!m_stereo3DfakeStereo || m_pd3dPrimaryDevice->DepthBufferReadBackAvailable()));
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
   const bool ss_refl = m_ss_refl && m_table->m_enableSSR && m_pd3dPrimaryDevice->DepthBufferReadBackAvailable() && m_table->m_SSRScale > 0.f;
   const unsigned int sharpen = PostProcAA ? m_sharpen : 0;
   const bool useAO = GetAOMode() == 2;
   const bool useUpscaler = (m_AAfactor < 1.0f) && !stereo && (SMAA || DLAA || NFAA || FXAA1 || FXAA2 || FXAA3 || sharpen);

   RenderTarget *renderedRT = m_pd3dPrimaryDevice->GetBackBufferTexture();
   RenderTarget *outputRT = nullptr;
   m_pd3dPrimaryDevice->ResetRenderState();
   m_pd3dPrimaryDevice->SetRenderState(RenderState::ALPHABLENDENABLE, RenderState::RS_FALSE);
   m_pd3dPrimaryDevice->SetRenderState(RenderState::CULLMODE, RenderState::CULL_NONE);
   m_pd3dPrimaryDevice->SetRenderState(RenderState::ZWRITEENABLE, RenderState::RS_FALSE);
   m_pd3dPrimaryDevice->SetRenderState(RenderState::ZENABLE, RenderState::RS_FALSE);

   // All postprocess that uses depth sample it from the MSAA resolved rendered backbuffer
   m_pd3dPrimaryDevice->FBShader->SetTexture(SHADER_tex_depth, m_pd3dPrimaryDevice->GetBackBufferTexture()->GetDepthSampler());

   // Compute bloom (to be applied later)
   Bloom();
   if (g_pplayer->GetProfilingMode() == PF_ENABLED)
      m_gpu_profiler.Timestamp(GTS_Bloom);

   // Add screen space reflections
   if (ss_refl)
   {
      SSRefl();
      renderedRT = m_pd3dPrimaryDevice->GetReflectionBufferTexture();
   }
   if (g_pplayer->GetProfilingMode() == PF_ENABLED)
      m_gpu_profiler.Timestamp(GTS_SSR);

   // Compute AO contribution (to be applied later)
   if (useAO)
   {
      // separate normal generation pass, currently roughly same perf or even much worse
      /* m_pd3dPrimaryDevice->SetRenderTarget(m_pd3dDevice->GetPostProcessRenderTarget1()); //!! expects stereo or FXAA enabled
      m_pd3dDevice->FBShader->SetTexture(SHADER_tex_depth, m_pdds3DZBuffer, true);
      m_pd3dDevice->FBShader->SetVector(SHADER_w_h_height, (float)(1.0 / m_width), (float)(1.0 / m_height),
         radical_inverse(m_overall_frames%2048)*(float)(1. / 8.0), sobol(m_overall_frames%2048)*(float)(5. / 8.0));// jitter within lattice cell //!! ?
      m_pd3dDevice->FBShader->SetTechnique("normals");
      m_pd3dDevice->DrawFullscreenTexturedQuad(m_pd3dPrimaryDevice->FBShader);*/

      m_pd3dPrimaryDevice->SetRenderTarget("ScreenSpace AO"s, m_pd3dPrimaryDevice->GetAORenderTarget(0), false);
      m_pd3dPrimaryDevice->AddRenderTargetDependency(m_pd3dPrimaryDevice->GetAORenderTarget(1));
      m_pd3dPrimaryDevice->AddRenderTargetDependency(m_pd3dPrimaryDevice->GetBackBufferTexture());
      m_pd3dPrimaryDevice->FBShader->SetTexture(SHADER_tex_fb_filtered, m_pd3dPrimaryDevice->GetAORenderTarget(1)->GetColorSampler());
      //m_pd3dDevice->FBShader->SetTexture(SHADER_Texture1, m_pd3dDevice->GetPostProcessRenderTarget1()); // temporary normals
      m_pd3dPrimaryDevice->FBShader->SetTexture(SHADER_tex_ao_dither, &m_aoDitherTexture, SF_NONE, SA_REPEAT, SA_REPEAT, true); // FIXME the force linear RGB is not honored
      m_pd3dPrimaryDevice->FBShader->SetVector(SHADER_w_h_height, 
         (float)(1.0 / m_pd3dPrimaryDevice->GetAORenderTarget(1)->GetWidth()),
         (float)(1.0 / m_pd3dPrimaryDevice->GetAORenderTarget(1)->GetHeight()), 
         radical_inverse(g_pplayer->m_overall_frames % 2048) * (float)(1. / 8.0),
         /*sobol*/ radical_inverse<3>(g_pplayer->m_overall_frames % 2048) * (float)(1. / 8.0)); // jitter within (64/8)x(64/8) neighborhood of 64x64 tex, good compromise between blotches and noise
      m_pd3dPrimaryDevice->FBShader->SetVector(SHADER_AO_scale_timeblur, m_table->m_AOScale, 0.4f, 0.f, 0.f); //!! 0.4f: fake global option in video pref? or time dependent? //!! commonly used is 0.1, but would require to clear history for moving stuff
      m_pd3dPrimaryDevice->FBShader->SetTechnique(SHADER_TECHNIQUE_AO);
      m_pd3dPrimaryDevice->DrawFullscreenTexturedQuad(m_pd3dPrimaryDevice->FBShader);

      // flip AO buffers (avoids copy)
      m_pd3dPrimaryDevice->SwapAORenderTargets();
   }
   if (g_pplayer->GetProfilingMode() == PF_ENABLED)
      m_gpu_profiler.Timestamp(GTS_AO);

   // Perform color grade LUT / dither / tonemapping, also applying bloom and AO
   {
      // switch to output buffer (main output frame buffer, or a temporary one for postprocessing)
      if (SMAA || DLAA || NFAA || FXAA1 || FXAA2 || FXAA3 || sharpen || stereo || useUpscaler)
         outputRT = m_pd3dPrimaryDevice->GetPostProcessRenderTarget1();
      else
         outputRT = m_pd3dPrimaryDevice->GetOutputBackBuffer();
      m_pd3dPrimaryDevice->SetRenderTarget("Tonemap/Dither/ColorGrade"s, outputRT, false);
      m_pd3dPrimaryDevice->AddRenderTargetDependency(renderedRT, useAO);

      int render_w = renderedRT->GetWidth(), render_h = renderedRT->GetHeight();
      m_pd3dPrimaryDevice->FBShader->SetTexture(SHADER_tex_fb_unfiltered, renderedRT->GetColorSampler());
      m_pd3dPrimaryDevice->FBShader->SetTexture(SHADER_tex_fb_filtered, renderedRT->GetColorSampler());
      if (m_table->m_bloom_strength > 0.0f && !m_bloomOff)
      {
         m_pd3dPrimaryDevice->AddRenderTargetDependency(m_pd3dPrimaryDevice->GetBloomBufferTexture());
         m_pd3dPrimaryDevice->FBShader->SetTexture(SHADER_tex_bloom, m_pd3dPrimaryDevice->GetBloomBufferTexture()->GetColorSampler());
      }
      if (useAO)
      {
         m_pd3dPrimaryDevice->FBShader->SetTexture(SHADER_tex_ao, m_pd3dPrimaryDevice->GetAORenderTarget(1)->GetColorSampler());
         m_pd3dPrimaryDevice->AddRenderTargetDependency(m_pd3dPrimaryDevice->GetAORenderTarget(1));
      }

      // For information mode, override with the wanted render target instead of the render buffer
      const InfoMode infoMode = g_pplayer->GetInfoMode();
      if (infoMode == IF_RENDER_PROBES)
      {
         RenderProbe* render_probe = m_table->m_vrenderprobe[g_pplayer->m_infoProbeIndex];
         RenderTarget *probe = render_probe->Render(0);
         if (probe)
         {
            m_pd3dPrimaryDevice->AddRenderTargetDependency(probe);
            m_pd3dPrimaryDevice->FBShader->SetTexture(SHADER_tex_fb_unfiltered, probe->GetColorSampler());
            m_pd3dPrimaryDevice->FBShader->SetTexture(SHADER_tex_fb_filtered, probe->GetColorSampler());
            render_w = probe->GetWidth();
            render_h = probe->GetHeight();
         }
      }
      else if (infoMode == IF_LIGHT_BUFFER_ONLY)
      {
         renderedRT = m_pd3dPrimaryDevice->GetBloomBufferTexture();
         m_pd3dPrimaryDevice->AddRenderTargetDependency(renderedRT);
         m_pd3dPrimaryDevice->FBShader->SetTexture(SHADER_tex_fb_unfiltered, renderedRT->GetColorSampler());
         m_pd3dPrimaryDevice->FBShader->SetTexture(SHADER_tex_fb_filtered, renderedRT->GetColorSampler());
         render_w = renderedRT->GetWidth();
         render_h = renderedRT->GetHeight();
      }

      // Texture used for LUT color grading must be treated as if they were linear
      Texture *const pin = m_table->GetImage(m_table->m_imageColorGrade);
      if (pin)
         m_pd3dPrimaryDevice->FBShader->SetTexture(SHADER_tex_color_lut, pin, SF_BILINEAR, SA_CLAMP, SA_CLAMP, true); // FIXME always honor the linear RGB
      m_pd3dPrimaryDevice->FBShader->SetBool(SHADER_color_grade, pin != nullptr);
      m_pd3dPrimaryDevice->FBShader->SetBool(SHADER_do_dither, m_pd3dPrimaryDevice->GetOutputBackBuffer()->GetColorFormat() != colorFormat::RGBA10);
      m_pd3dPrimaryDevice->FBShader->SetBool(SHADER_do_bloom, (m_table->m_bloom_strength > 0.0f && !m_bloomOff && infoMode <= IF_DYNAMIC_ONLY));

      //const unsigned int jittertime = (unsigned int)((U64)msec()*90/1000);
      const float jitter = (float)((msec()&2047)/1000.0);
      m_pd3dPrimaryDevice->FBShader->SetVector(SHADER_w_h_height, 
         (float)(1.0 / (double)render_w), (float)(1.0 / (double)render_h), //1.0f, 1.0f);
         jitter, // radical_inverse(jittertime) * 11.0f,
         jitter); // sobol(jittertime) * 13.0f); // jitter for dither pattern

      if (infoMode == IF_AO_ONLY)
         m_pd3dPrimaryDevice->FBShader->SetTechnique(SHADER_TECHNIQUE_fb_AO);
      else if (infoMode == IF_RENDER_PROBES)
         m_pd3dPrimaryDevice->FBShader->SetTechnique(m_toneMapper == TM_REINHARD ? SHADER_TECHNIQUE_fb_rhtonemap
                                                   : m_toneMapper == TM_FILMIC   ? SHADER_TECHNIQUE_fb_fmtonemap
                                                                                            : SHADER_TECHNIQUE_fb_tmtonemap);
      else if (m_BWrendering != 0)
         m_pd3dPrimaryDevice->FBShader->SetTechnique(m_BWrendering == 1 ? SHADER_TECHNIQUE_fb_rhtonemap_no_filterRG : SHADER_TECHNIQUE_fb_rhtonemap_no_filterR);
      else if (m_toneMapper == TM_REINHARD)
         m_pd3dPrimaryDevice->FBShader->SetTechnique(useAO ?
              useAA ? SHADER_TECHNIQUE_fb_rhtonemap_AO : SHADER_TECHNIQUE_fb_rhtonemap_AO_no_filter
            : useAA ? SHADER_TECHNIQUE_fb_rhtonemap    : SHADER_TECHNIQUE_fb_rhtonemap_no_filter);
      else if (m_toneMapper == TM_FILMIC)
         m_pd3dPrimaryDevice->FBShader->SetTechnique(useAO ?
              useAA ? SHADER_TECHNIQUE_fb_fmtonemap_AO : SHADER_TECHNIQUE_fb_fmtonemap_AO_no_filter
            : useAA ? SHADER_TECHNIQUE_fb_fmtonemap    : SHADER_TECHNIQUE_fb_fmtonemap_no_filter);
      else // TM_TONY_MC_MAPFACE
         m_pd3dPrimaryDevice->FBShader->SetTechnique(useAO ?
              useAA ? SHADER_TECHNIQUE_fb_tmtonemap_AO : SHADER_TECHNIQUE_fb_tmtonemap_AO_no_filter
            : useAA ? SHADER_TECHNIQUE_fb_tmtonemap    : SHADER_TECHNIQUE_fb_tmtonemap_no_filter);

      const Vertex3D_TexelOnly shiftedVerts[4] =
      {
         {  1.0f + m_ScreenOffset.x,  1.0f + m_ScreenOffset.y, 0.0f, 1.0f, 0.0f },
         { -1.0f + m_ScreenOffset.x,  1.0f + m_ScreenOffset.y, 0.0f, 0.0f, 0.0f },
         {  1.0f + m_ScreenOffset.x, -1.0f + m_ScreenOffset.y, 0.0f, 1.0f, 1.0f },
         { -1.0f + m_ScreenOffset.x, -1.0f + m_ScreenOffset.y, 0.0f, 0.0f, 1.0f }
      };
      m_pd3dPrimaryDevice->DrawTexturedQuad(m_pd3dPrimaryDevice->FBShader, shiftedVerts);
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
         #ifdef ENABLE_SDL
         y = renderedRT->GetHeight() - 1 - y;
         #endif
         pdest[i * 3 + 0] = ((i & 1) == 0 && (y & 1) == 0) ? 0x00 : 0xFF;
         pdest[i * 3 + 1] = ((i >> 2) & 1) == 0 ? 0x00 : ((i & 1) == 0 && (y & 1) == 0) ? 0x00 : 0xFF;
         pdest[i * 3 + 2] = ((y >> 2) & 1) == 0 ? 0x00 : ((i & 1) == 0 && (y & 1) == 0) ? 0x00 : 0xFF;
      }
      Sampler *checker = new Sampler(m_pd3dPrimaryDevice, tex, true, SA_CLAMP, SA_CLAMP, SF_NONE);
      m_pd3dPrimaryDevice->FBShader->SetVector(SHADER_w_h_height, (float)(1.0 / renderedRT->GetWidth()), (float)(1.0 / renderedRT->GetHeight()), 1.f, 1.f);
      m_pd3dPrimaryDevice->FBShader->SetTexture(SHADER_tex_fb_filtered, checker);
      m_pd3dPrimaryDevice->FBShader->SetTechnique(SHADER_TECHNIQUE_fb_mirror);
      m_pd3dPrimaryDevice->DrawFullscreenTexturedQuad(m_pd3dPrimaryDevice->FBShader);
      renderedRT = outputRT;
      delete checker;
      delete tex;
   }

   // Stereo and AA are performed on LDR render buffer after tonemapping (RGB8 or RGB10, but nof RGBF).
   // We ping pong between BackBufferTmpTexture and BackBufferTmpTexture2 for the different postprocess
   // SMAA is a special case since it needs 3 passes, so it uses GetBackBufferTexture also (which is somewhat overkill since it is RGB16F)
   assert(renderedRT == m_pd3dPrimaryDevice->GetOutputBackBuffer() || renderedRT == m_pd3dPrimaryDevice->GetPostProcessRenderTarget1());

   // Perform post processed anti aliasing
   if (NFAA || FXAA1 || FXAA2 || FXAA3)
   {
      assert(renderedRT == m_pd3dPrimaryDevice->GetPostProcessRenderTarget1());
      outputRT = sharpen || stereo || useUpscaler ? m_pd3dPrimaryDevice->GetPostProcessRenderTarget(renderedRT) : m_pd3dPrimaryDevice->GetOutputBackBuffer();
      m_pd3dPrimaryDevice->SetRenderTarget(SMAA ? "SMAA Color/Edge Detection"s : "Post Process AA Pass 1"s, outputRT, false);
      m_pd3dPrimaryDevice->AddRenderTargetDependency(renderedRT);
      m_pd3dPrimaryDevice->FBShader->SetTexture(SHADER_tex_fb_filtered, renderedRT->GetColorSampler());
      m_pd3dPrimaryDevice->FBShader->SetTexture(SHADER_tex_fb_unfiltered, renderedRT->GetColorSampler());
      m_pd3dPrimaryDevice->AddRenderTargetDependency(m_pd3dPrimaryDevice->GetBackBufferTexture(), true); // Depth is always taken from the MSAA resolved render buffer
      m_pd3dPrimaryDevice->FBShader->SetVector(SHADER_w_h_height, (float)(1.0 / renderedRT->GetWidth()), (float)(1.0 / renderedRT->GetHeight()), (float)renderedRT->GetWidth(), 1.f);
      m_pd3dPrimaryDevice->FBShader->SetTechnique(NFAA  ? SHADER_TECHNIQUE_NFAA : FXAA3 ? SHADER_TECHNIQUE_FXAA3 : FXAA2 ? SHADER_TECHNIQUE_FXAA2 : SHADER_TECHNIQUE_FXAA1);
      m_pd3dPrimaryDevice->DrawFullscreenTexturedQuad(m_pd3dPrimaryDevice->FBShader);
      renderedRT = outputRT;
   }
   else if (DLAA)
   {
      assert(renderedRT == m_pd3dPrimaryDevice->GetPostProcessRenderTarget1());
      // First pass detect edges and write it to alpha channel (keeping RGB)
      outputRT = m_pd3dPrimaryDevice->GetPostProcessRenderTarget(renderedRT);
      m_pd3dPrimaryDevice->SetRenderTarget("DLAA Edge Detection"s, outputRT, false);
      m_pd3dPrimaryDevice->AddRenderTargetDependency(renderedRT);
      m_pd3dPrimaryDevice->FBShader->SetTexture(SHADER_tex_fb_filtered, renderedRT->GetColorSampler());
      m_pd3dPrimaryDevice->FBShader->SetTexture(SHADER_tex_fb_unfiltered, renderedRT->GetColorSampler());
      m_pd3dPrimaryDevice->AddRenderTargetDependency(m_pd3dPrimaryDevice->GetBackBufferTexture(), true); // Depth is always taken from the MSAA resolved render buffer
      m_pd3dPrimaryDevice->FBShader->SetVector(SHADER_w_h_height, (float)(1.0 / renderedRT->GetWidth()), (float)(1.0 / renderedRT->GetHeight()), (float)renderedRT->GetWidth(), 1.f);
      m_pd3dPrimaryDevice->FBShader->SetTechnique(SHADER_TECHNIQUE_DLAA_edge);
      m_pd3dPrimaryDevice->DrawFullscreenTexturedQuad(m_pd3dPrimaryDevice->FBShader);
      renderedRT = outputRT;

      // Second pass: use edge detection from first pass (alpha channel) and RGB colors for actual filtering
      outputRT = sharpen || stereo || useUpscaler ? m_pd3dPrimaryDevice->GetPostProcessRenderTarget(renderedRT) : m_pd3dPrimaryDevice->GetOutputBackBuffer();
      m_pd3dPrimaryDevice->SetRenderTarget("DLAA Neigborhood blending"s, outputRT, false);
      m_pd3dPrimaryDevice->AddRenderTargetDependency(renderedRT);
      m_pd3dPrimaryDevice->FBShader->SetTexture(SHADER_tex_fb_filtered, renderedRT->GetColorSampler());
      m_pd3dPrimaryDevice->FBShader->SetTexture(SHADER_tex_fb_unfiltered, renderedRT->GetColorSampler());
      m_pd3dPrimaryDevice->FBShader->SetTechnique(SHADER_TECHNIQUE_DLAA);
      m_pd3dPrimaryDevice->DrawFullscreenTexturedQuad(m_pd3dPrimaryDevice->FBShader);
      renderedRT = outputRT;
   }
   else if (SMAA)
   {
#ifndef __OPENGLES__
      assert(renderedRT == m_pd3dPrimaryDevice->GetPostProcessRenderTarget1());
      // SMAA use 3 passes, all of them using the initial render, so since tonemap use postprocess RT 1, we use the back buffer and post process RT 2
      RenderTarget *sourceRT = renderedRT;
      m_pd3dPrimaryDevice->FBShader->SetTexture(SHADER_tex_fb_filtered, sourceRT->GetColorSampler());
      m_pd3dPrimaryDevice->FBShader->SetTexture(SHADER_tex_fb_unfiltered, sourceRT->GetColorSampler());
      m_pd3dPrimaryDevice->FBShader->SetTexture(SHADER_areaTex, m_pd3dPrimaryDevice->m_SMAAareaTexture);
      m_pd3dPrimaryDevice->FBShader->SetTexture(SHADER_searchTex, m_pd3dPrimaryDevice->m_SMAAsearchTexture);
      m_pd3dPrimaryDevice->FBShader->SetVector(SHADER_w_h_height, (float)(1.0 / sourceRT->GetWidth()), (float)(1.0 / sourceRT->GetHeight()), (float)sourceRT->GetWidth(), (float)sourceRT->GetHeight());

      outputRT = m_pd3dPrimaryDevice->GetPreviousBackBufferTexture(); // We don't need it anymore, so use it as a third postprocess buffer
      m_pd3dPrimaryDevice->SetRenderTarget("SMAA Color/Edge Detection"s, outputRT, false);
      m_pd3dPrimaryDevice->AddRenderTargetDependency(sourceRT); // PostProcess RT 1
      m_pd3dPrimaryDevice->FBShader->SetTechnique(SHADER_TECHNIQUE_SMAA_ColorEdgeDetection);
      m_pd3dPrimaryDevice->DrawFullscreenTexturedQuad(m_pd3dPrimaryDevice->FBShader);
      renderedRT = outputRT;

      outputRT = m_pd3dPrimaryDevice->GetPostProcessRenderTarget(sourceRT);
      m_pd3dPrimaryDevice->SetRenderTarget("SMAA Blend weight calculation"s, outputRT, false);
      m_pd3dPrimaryDevice->AddRenderTargetDependency(sourceRT); // PostProcess RT 1
      m_pd3dPrimaryDevice->AddRenderTargetDependency(renderedRT); // BackBuffer RT
      m_pd3dPrimaryDevice->FBShader->SetTexture(SHADER_edgesTex, renderedRT->GetColorSampler());
      m_pd3dPrimaryDevice->FBShader->SetTechnique(SHADER_TECHNIQUE_SMAA_BlendWeightCalculation);
      m_pd3dPrimaryDevice->DrawFullscreenTexturedQuad(m_pd3dPrimaryDevice->FBShader);
      renderedRT = outputRT;

      outputRT = sharpen || stereo || useUpscaler ? m_pd3dPrimaryDevice->GetPreviousBackBufferTexture() : m_pd3dPrimaryDevice->GetOutputBackBuffer();
      m_pd3dPrimaryDevice->SetRenderTarget("SMAA Neigborhood blending"s, outputRT, false);
      m_pd3dPrimaryDevice->AddRenderTargetDependency(sourceRT); // PostProcess RT 1
      m_pd3dPrimaryDevice->AddRenderTargetDependency(renderedRT); // PostProcess RT 2
      m_pd3dPrimaryDevice->FBShader->SetTexture(SHADER_blendTex, renderedRT->GetColorSampler());
      m_pd3dPrimaryDevice->FBShader->SetTechnique(SHADER_TECHNIQUE_SMAA_NeighborhoodBlending);
      m_pd3dPrimaryDevice->DrawFullscreenTexturedQuad(m_pd3dPrimaryDevice->FBShader);
      renderedRT = outputRT;
#endif
   }

   // Performs sharpening
   if (sharpen)
   {
      assert(renderedRT != m_pd3dPrimaryDevice->GetOutputBackBuffer()); // At this point, renderedRT may be PP1, PP2 or backbuffer
      outputRT = stereo || useUpscaler ? m_pd3dPrimaryDevice->GetPostProcessRenderTarget(renderedRT) : m_pd3dPrimaryDevice->GetOutputBackBuffer();
      m_pd3dPrimaryDevice->SetRenderTarget("Sharpen"s, outputRT, false);
      m_pd3dPrimaryDevice->AddRenderTargetDependency(renderedRT);
      m_pd3dPrimaryDevice->AddRenderTargetDependency(m_pd3dPrimaryDevice->GetBackBufferTexture(), true); // Depth is always taken from the MSAA resolved render buffer
      m_pd3dPrimaryDevice->FBShader->SetTexture(SHADER_tex_fb_filtered, renderedRT->GetColorSampler());
      m_pd3dPrimaryDevice->FBShader->SetTexture(SHADER_tex_fb_unfiltered, renderedRT->GetColorSampler());
      m_pd3dPrimaryDevice->FBShader->SetVector(SHADER_w_h_height, (float)(1.0 / renderedRT->GetWidth()), (float)(1.0 / renderedRT->GetHeight()), (float)renderedRT->GetWidth(), 1.f);
      m_pd3dPrimaryDevice->FBShader->SetTechnique((sharpen == 1) ? SHADER_TECHNIQUE_CAS : SHADER_TECHNIQUE_BilateralSharp_CAS);
      m_pd3dPrimaryDevice->DrawFullscreenTexturedQuad(m_pd3dPrimaryDevice->FBShader);
      renderedRT = outputRT;
   }

   // Apply stereo
   if (stereo)
   {
      #if defined(ENABLE_SDL) && defined(ENABLE_VR)
      // For STEREO_OFF, STEREO_TB, STEREO_SBS, this won't do anything. The previous postprocess steps should already have written to OutputBackBuffer
      // For VR, copy each eye to the HMD texture and render the wanted preview if activated
      if (m_stereo3D == STEREO_VR)
      {
         // Render LiveUI in headset for VR
         g_frameProfiler.EnterProfileSection(FrameProfiler::PROFILE_MISC);
         m_pd3dPrimaryDevice->SetRenderTarget("ImGui"s, renderedRT);
         m_pd3dPrimaryDevice->RenderLiveUI();
         g_frameProfiler.ExitProfileSection();

         assert(renderedRT != m_pd3dPrimaryDevice->GetOutputBackBuffer());
         int w = renderedRT->GetWidth(), h = renderedRT->GetHeight();
         
         RenderTarget *leftTexture = m_pd3dPrimaryDevice->GetOffscreenVR(0);
         m_pd3dPrimaryDevice->SetRenderTarget("Left Eye"s, leftTexture, false);
         m_pd3dPrimaryDevice->AddRenderTargetDependency(renderedRT);
         m_pd3dPrimaryDevice->BlitRenderTarget(renderedRT, leftTexture, true, false, 0, 0, w, h, 0, 0, w, h, 0, 0);

         RenderTarget *rightTexture = m_pd3dPrimaryDevice->GetOffscreenVR(1);
         m_pd3dPrimaryDevice->SetRenderTarget("Right Eye"s, rightTexture, false);
         m_pd3dPrimaryDevice->AddRenderTargetDependency(renderedRT);
         m_pd3dPrimaryDevice->BlitRenderTarget(renderedRT, rightTexture, true, false, 0, 0, w, h, 0, 0, w, h, 1, 0);

         RenderTarget *outRT = m_pd3dPrimaryDevice->GetOutputBackBuffer();
         m_pd3dPrimaryDevice->SetRenderTarget("VR Preview"s, outRT, false);
         m_pd3dPrimaryDevice->AddRenderTargetDependency(leftTexture); // To ensure blit is made
         m_pd3dPrimaryDevice->AddRenderTargetDependency(rightTexture); // To ensure blit is made
         m_pd3dPrimaryDevice->AddRenderTargetDependency(renderedRT);
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
            m_pd3dPrimaryDevice->Clear(clearType::TARGET | clearType::ZBUFFER, 0, 1.0f, 0L);
         if (m_vrPreview == VRPREVIEW_LEFT || m_vrPreview == VRPREVIEW_RIGHT)
         {
            m_pd3dPrimaryDevice->BlitRenderTarget(renderedRT, outRT, true, false, x, y, fw, fh, 0, 0, outW, outH, m_vrPreview == VRPREVIEW_LEFT ? 0 : 1, 0);
         }
         else if (m_vrPreview == VRPREVIEW_BOTH)
         {
            m_pd3dPrimaryDevice->BlitRenderTarget(renderedRT, outRT, true, false, x, y, fw, fh, 0, 0, outW, outH, 0, 0);
            m_pd3dPrimaryDevice->BlitRenderTarget(renderedRT, outRT, true, false, x, y, fw, fh, outW, 0, outW, outH, 1, 0);
         }

         m_pd3dPrimaryDevice->SubmitVR(renderedRT);
      }
      else 
      #endif
      if (IsAnaglyphStereoMode(m_stereo3D) || Is3DTVStereoMode(m_stereo3D))
      {
         // Anaglyph and 3DTV
         assert(renderedRT != m_pd3dPrimaryDevice->GetOutputBackBuffer());
         // For anaglyph, defocus the "lesser" eye (the one with a darker color, which should be the non dominant eye of the player)
         if (m_stereo3DDefocus != 0.f)
         {
            RenderTarget *tmpRT = m_pd3dPrimaryDevice->GetPostProcessRenderTarget(renderedRT);
            outputRT = m_pd3dPrimaryDevice->GetPostProcessRenderTarget(tmpRT);
            m_pd3dPrimaryDevice->DrawGaussianBlur(renderedRT, tmpRT, outputRT, abs(m_stereo3DDefocus) * 39.f, m_stereo3DDefocus > 0.f ? 0 : 1);
            renderedRT = outputRT;
         }
         // Stereo composition
         m_pd3dPrimaryDevice->SetRenderTarget("Stereo Anaglyph"s, m_pd3dPrimaryDevice->GetOutputBackBuffer(), false);
         m_pd3dPrimaryDevice->AddRenderTargetDependency(renderedRT);
         if (m_stereo3DfakeStereo)
         {
            m_pd3dPrimaryDevice->AddRenderTargetDependency(m_pd3dPrimaryDevice->GetBackBufferTexture(), true);
            m_pd3dPrimaryDevice->StereoShader->SetTexture(SHADER_tex_stereo_depth, m_pd3dPrimaryDevice->GetBackBufferTexture()->GetDepthSampler());
         }
         m_pd3dPrimaryDevice->StereoShader->SetTexture(SHADER_tex_stereo_fb, renderedRT->GetColorSampler());
         m_pd3dPrimaryDevice->DrawFullscreenTexturedQuad(m_pd3dPrimaryDevice->StereoShader);
      }
      else
      {
         // STEREO_OFF: nothing to do
         assert(renderedRT == m_pd3dPrimaryDevice->GetOutputBackBuffer());
      }
   }
   // Upscale: When using downscaled backbuffer (for performance reason), upscaling is done after postprocessing
   else if (useUpscaler)
   {
      assert(renderedRT != m_pd3dPrimaryDevice->GetOutputBackBuffer()); // At this point, renderedRT may be PP1, PP2 or backbuffer
      outputRT = m_pd3dPrimaryDevice->GetOutputBackBuffer();
      m_pd3dPrimaryDevice->SetRenderTarget("Upscale"s, outputRT, false);
      m_pd3dPrimaryDevice->AddRenderTargetDependency(renderedRT);
      m_pd3dPrimaryDevice->FBShader->SetTexture(SHADER_tex_fb_filtered, renderedRT->GetColorSampler());
      m_pd3dPrimaryDevice->FBShader->SetTechnique(SHADER_TECHNIQUE_fb_copy);
      m_pd3dPrimaryDevice->DrawFullscreenTexturedQuad(m_pd3dPrimaryDevice->FBShader);
      renderedRT = outputRT;
   }

   if (!stereo || m_stereo3D != STEREO_VR)
   {
      // Except for VR, render LiveUI after tonemapping and stereo (otherwise it would break the calibration process for stereo anaglyph)
      g_frameProfiler.EnterProfileSection(FrameProfiler::PROFILE_MISC);
      m_pd3dPrimaryDevice->SetRenderTarget("ImGui"s, m_pd3dPrimaryDevice->GetOutputBackBuffer());
      m_pd3dPrimaryDevice->RenderLiveUI();
      g_frameProfiler.ExitProfileSection();
   }

   if (g_pplayer->GetProfilingMode() == PF_ENABLED)
      m_gpu_profiler.Timestamp(GTS_PostProcess);
}

#pragma endregion
