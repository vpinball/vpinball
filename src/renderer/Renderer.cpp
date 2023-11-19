#include "stdafx.h"
#include "renderer/RenderDevice.h"
#include "renderer/Shader.h"
#include "math/math.h"
#include "ThreadPool.h"
#include "BAM/BAMView.h"
#include "meshes/ballMesh.h"

Pin3D::Pin3D(PinTable* const table, const bool fullScreen, const int width, const int height, const int colordepth, int& refreshrate, VideoSyncMode& syncMode, const float AAfactor, const StereoMode stereo3D)
   : m_table(table)
{
   m_vrPreview = (VRPreviewMode)m_table->m_settings.LoadValueWithDefault(Settings::PlayerVR, "VRPreview"s, (int)VRPREVIEW_LEFT);
   m_vrPreviewShrink = m_table->m_settings.LoadValueWithDefault(Settings::PlayerVR, "ShrinkPreview"s, false);
   m_FXAA = m_table->m_settings.LoadValueWithDefault(Settings::Player, "FXAA"s, (int)Disabled);
   m_sharpen = m_table->m_settings.LoadValueWithDefault(Settings::Player, "Sharpen"s, 0);
   m_ss_refl = m_table->m_settings.LoadValueWithDefault(Settings::Player, "SSRefl"s, false);
   const bool ss_refl = m_ss_refl && m_table->m_enableSSR;
   m_bloomOff = m_table->m_settings.LoadValueWithDefault(Settings::Player, "ForceBloomOff"s, false);
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

   m_stereo3D = stereo3D;
   m_mvp = new ModelViewProj(m_stereo3D == STEREO_OFF ? 1 : 2);
   m_AAfactor = AAfactor;

   // set the expected viewport for the newly created device (it may be modified upon creation)
   m_viewPort.X = 0;
   m_viewPort.Y = 0;
   m_viewPort.Width = width;
   m_viewPort.Height = height;
   m_viewPort.MinZ = 0.0f;
   m_viewPort.MaxZ = 1.0f;

   const int display = g_pvp->m_primaryDisplay ? 0 : g_pplayer->m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "Display"s, 0);
   vector<DisplayConfig> displays;
   getDisplayList(displays);
   int adapter = 0;
   for (vector<DisplayConfig>::iterator dispConf = displays.begin(); dispConf != displays.end(); ++dispConf)
      if (display == dispConf->display)
         adapter = dispConf->adapter;

   m_pd3dPrimaryDevice = new RenderDevice(g_pplayer->GetHwnd(), m_viewPort.Width, m_viewPort.Height, fullScreen, colordepth, AAfactor, stereo3D, m_FXAA, m_sharpen, ss_refl,
      g_pplayer->m_useNvidiaApi, g_pplayer->m_disableDWM, g_pplayer->m_BWrendering);
   try {
      m_pd3dPrimaryDevice->CreateDevice(refreshrate, syncMode, adapter);
   }
   catch (...) {
      // TODO better error handling => just let the exception up
      throw(E_FAIL);
   }

   #ifndef ENABLE_SDL
   if (m_stereo3D == STEREO_SBS)
   {
      // Side by side needs to fit the 2 views along the width, so each view is half the total width
      m_viewPort.Width = m_pd3dPrimaryDevice->m_width / 2;
      m_viewPort.Height = m_pd3dPrimaryDevice->m_height;
   }
   else if (m_stereo3D == STEREO_TB || m_stereo3D == STEREO_INT || m_stereo3D == STEREO_FLIPPED_INT)
   {
      // Top/Bottom (and interlaced) needs to fit the 2 views along the height, so each view is half the total height
      m_viewPort.Width = m_pd3dPrimaryDevice->m_width;
      m_viewPort.Height = m_pd3dPrimaryDevice->m_height / 2;
   }
   else
   #endif
   { // Use the effective size of the created device's window (should be the same as the requested)
      m_viewPort.Width = m_pd3dPrimaryDevice->m_width;
      m_viewPort.Height = m_pd3dPrimaryDevice->m_height;
   }

   // TODO better error handling
   if (!m_pd3dPrimaryDevice->LoadShaders())
      throw(E_FAIL);

   BAMView::init();

   const bool compressTextures = g_pplayer->m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "CompressTextures"s, false);
   m_pd3dPrimaryDevice->CompressTextures(compressTextures);

   m_pd3dPrimaryDevice->SetViewport(&m_viewPort);

   //

   if (m_stereo3D == STEREO_VR)
      m_backGlass = new BackGlass(
         m_pd3dPrimaryDevice, g_pplayer->m_ptable->GetDecalsEnabled() ? g_pplayer->m_ptable->GetImage(g_pplayer->m_ptable->m_BG_image[g_pplayer->m_ptable->m_BG_current_set]) : nullptr);
   else
      m_backGlass = nullptr;

   // This used to be a spheremap BMP, upgraded in 10.8 for an equirectangular HDR env map
   m_pinballEnvTexture.LoadFromFile(g_pvp->m_szMyPath + "assets" + PATH_SEPARATOR_CHAR + "BallEnv.exr");
   m_aoDitherTexture.LoadFromFile(g_pvp->m_szMyPath + "assets" + PATH_SEPARATOR_CHAR + "AODither.webp");
   m_builtinEnvTexture.LoadFromFile(g_pvp->m_szMyPath + "assets" + PATH_SEPARATOR_CHAR + "EnvMap.webp");
   m_envTexture = g_pplayer->m_ptable->GetImage(g_pplayer->m_ptable->m_envImage);
   PLOGI << "Computing environment map radiance"; // For profiling
   #ifdef ENABLE_SDL // OpenGL
   Texture* const envTex = m_envTexture ? m_envTexture : &m_builtinEnvTexture;
   const int envTexHeight = min(envTex->m_pdsBuffer->height(), 256u) / 8;
   const int envTexWidth = envTexHeight * 2;
   const colorFormat rad_format = envTex->m_pdsBuffer->m_format == BaseTexture::RGB_FP32 ? colorFormat::RGBA32F : colorFormat::RGBA16F;
   m_envRadianceTexture = new RenderTarget(m_pd3dPrimaryDevice, SurfaceType::RT_DEFAULT, "Irradiance"s, envTexWidth, envTexHeight, rad_format, false, 1, "Failed to create irradiance render target");
   m_pd3dPrimaryDevice->FBShader->SetTechnique(SHADER_TECHNIQUE_irradiance);
   m_pd3dPrimaryDevice->FBShader->SetTexture(SHADER_tex_env, envTex);
   m_pd3dPrimaryDevice->SetRenderTarget("Env Irradiance PreCalc"s, m_envRadianceTexture);
   m_pd3dPrimaryDevice->DrawFullscreenTexturedQuad(m_pd3dPrimaryDevice->FBShader);
   m_pd3dPrimaryDevice->FlushRenderFrame();
   m_pd3dPrimaryDevice->basicShader->SetTexture(SHADER_tex_diffuse_env, m_envRadianceTexture->GetColorSampler());
   m_pd3dPrimaryDevice->m_ballShader->SetTexture(SHADER_tex_diffuse_env, m_envRadianceTexture->GetColorSampler());
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
#endif
}

Pin3D::~Pin3D()
{
   delete m_mvp;
   m_gpu_profiler.Shutdown();
   m_pd3dPrimaryDevice->FreeShader();
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
}

void Pin3D::TransformVertices(const Vertex3D_NoTex2 * const __restrict rgv, const WORD * const __restrict rgi, const int count, Vertex2D * const __restrict rgvout) const
{
   RECT viewport { 0, 0, (LONG)m_viewPort.Width, (LONG)m_viewPort.Height };
   m_mvp->GetModelViewProj(0).TransformVertices(rgv, rgi, count, rgvout, viewport);
}

void Pin3D::TransformVertices(const Vertex3Ds* const __restrict rgv, const WORD* const __restrict rgi, const int count, Vertex2D* const __restrict rgvout) const
{
   RECT viewport { 0, 0, (LONG)m_viewPort.Width, (LONG)m_viewPort.Height };
   m_mvp->GetModelViewProj(0).TransformVertices(rgv, rgi, count, rgvout, viewport);
}

BaseTexture* Pin3D::EnvmapPrecalc(const Texture* envTex, const unsigned int rad_env_xres, const unsigned int rad_env_yres)
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
                  ((unsigned short*)rad_envmap)[offs  ] = float2half(sum_r);
                  ((unsigned short*)rad_envmap)[offs+1] = float2half(sum_g);
                  ((unsigned short*)rad_envmap)[offs+2] = float2half(sum_b);
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

void Pin3D::DrawBackground()
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
      g_pplayer->Spritedraw(0.f, 0.f, 1.f, 1.f, 0xFFFFFFFF, pin, ptable->m_ImageBackdropNightDay ? sqrtf(g_pplayer->m_globalEmissionScale) : 1.0f, true);
   }
   else
   {
      const D3DCOLOR d3dcolor = COLORREF_to_D3DCOLOR(ptable->m_colorbackdrop);
      m_pd3dPrimaryDevice->Clear(clearType::TARGET | clearType::ZBUFFER, d3dcolor, 1.0f, 0L);
   }
}

Matrix3D ComputeLaybackTransform(const float layback)
{
   // skew the coordinate system from kartesian to non kartesian.
   Matrix3D matTrans;
   matTrans.SetIdentity();
   matTrans._32 = -tanf(0.5f * ANGTORAD(layback));
   return matTrans;
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
void Pin3D::InitLayout(const float xpixoff, const float ypixoff)
{
   TRACE_FUNCTION();
   ViewSetup& viewSetup = g_pplayer->m_ptable->mViewSetups[g_pplayer->m_ptable->m_BG_current_set];
   #ifdef ENABLE_SDL
   bool stereo = m_stereo3D != STEREO_OFF && m_stereo3D != STEREO_VR && g_pplayer->m_stereo3Denabled;
   #else
   bool stereo = false;
   #endif
   if (viewSetup.mMode == VLM_WINDOW)
      viewSetup.SetWindowModeFromSettings(g_pplayer->m_ptable);
   viewSetup.ComputeMVP(g_pplayer->m_ptable, m_viewPort.Width, m_viewPort.Height, stereo, *m_mvp, vec3(m_cam.x, m_cam.y, m_cam.z), m_inc, xpixoff, ypixoff);
   SetupShaders();
}

Vertex3Ds Pin3D::Unproject(const Vertex3Ds& point)
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

Vertex3Ds Pin3D::Get3DPointFrom2D(const POINT& p)
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

void Pin3D::UpdateBAMHeadTracking()
{
   Matrix3D m_matView;
   Matrix3D m_matProj[2];
   BAMView::createProjectionAndViewMatrix(&m_matProj[0]._11, &m_matView._11);
   m_mvp->SetView(m_matView);
   for (unsigned int eye = 0; eye < m_mvp->m_nEyes; eye++)
      m_mvp->SetProj(eye, m_matProj[eye]);
}





void Pin3D::SetupShaders()
{
   const vec4 envEmissionScale_TexWidth(m_table->m_envEmissionScale * g_pplayer->m_globalEmissionScale,
      (float) (m_envTexture ? *m_envTexture : m_builtinEnvTexture).m_height /*+m_builtinEnvTexture.m_width)*0.5f*/, 0.f, 0.f); //!! dto.

   UpdateBasicShaderMatrix();
   m_pd3dPrimaryDevice->basicShader->SetTexture(SHADER_tex_env, m_envTexture ? m_envTexture : &m_builtinEnvTexture);
   m_pd3dPrimaryDevice->basicShader->SetVector(SHADER_fenvEmissionScale_TexWidth, &envEmissionScale_TexWidth);

   UpdateBallShaderMatrix();
   const vec4 st(m_table->m_envEmissionScale * g_pplayer->m_globalEmissionScale, m_envTexture ? (float)m_envTexture->m_height/*+m_envTexture->m_width)*0.5f*/ : (float)m_builtinEnvTexture.m_height/*+m_builtinEnvTexture.m_width)*0.5f*/, 0.f, 0.f);
   m_pd3dPrimaryDevice->m_ballShader->SetVector(SHADER_fenvEmissionScale_TexWidth, &st);
   m_pd3dPrimaryDevice->m_ballShader->SetVector(SHADER_fenvEmissionScale_TexWidth, &envEmissionScale_TexWidth);
   //m_pd3dPrimaryDevice->m_ballShader->SetInt("iLightPointNum",MAX_LIGHT_SOURCES);

   constexpr float Roughness = 0.8f;
   m_pd3dPrimaryDevice->m_ballShader->SetVector(SHADER_Roughness_WrapL_Edge_Thickness, exp2f(10.0f * Roughness + 1.0f), 0.f, 1.f, 0.05f);
   vec4 amb_lr = convertColor(m_table->m_lightAmbient, m_table->m_lightRange);
   m_pd3dPrimaryDevice->m_ballShader->SetVector(SHADER_cAmbient_LightRange, 
      amb_lr.x * g_pplayer->m_globalEmissionScale, amb_lr.y * g_pplayer->m_globalEmissionScale, amb_lr.z * g_pplayer->m_globalEmissionScale, m_table->m_lightRange);

   //m_pd3dPrimaryDevice->basicShader->SetInt("iLightPointNum",MAX_LIGHT_SOURCES);

   g_pplayer->m_ptable->m_Light[0].pos.x = g_pplayer->m_ptable->m_right * 0.5f;
   g_pplayer->m_ptable->m_Light[1].pos.x = g_pplayer->m_ptable->m_right * 0.5f;
   g_pplayer->m_ptable->m_Light[0].pos.y = g_pplayer->m_ptable->m_bottom * (float)(1.0 / 3.0);
   g_pplayer->m_ptable->m_Light[1].pos.y = g_pplayer->m_ptable->m_bottom * (float)(2.0 / 3.0);
   g_pplayer->m_ptable->m_Light[0].pos.z = g_pplayer->m_ptable->m_lightHeight;
   g_pplayer->m_ptable->m_Light[1].pos.z = g_pplayer->m_ptable->m_lightHeight;

   vec4 emission = convertColor(g_pplayer->m_ptable->m_Light[0].emission);
   emission.x *= g_pplayer->m_ptable->m_lightEmissionScale * g_pplayer->m_globalEmissionScale;
   emission.y *= g_pplayer->m_ptable->m_lightEmissionScale * g_pplayer->m_globalEmissionScale;
   emission.z *= g_pplayer->m_ptable->m_lightEmissionScale * g_pplayer->m_globalEmissionScale;

#ifdef ENABLE_SDL
   float lightPos[MAX_LIGHT_SOURCES][4] = { 0.f };
   float lightEmission[MAX_LIGHT_SOURCES][4] = { 0.f };

   for (unsigned int i = 0; i < MAX_LIGHT_SOURCES; ++i)
   {
      memcpy(&lightPos[i], &g_pplayer->m_ptable->m_Light[i].pos, sizeof(float) * 3);
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
      memcpy(&l[i].vPos, &g_pplayer->m_ptable->m_Light[i].pos, sizeof(float) * 3);
      memcpy(&l[i].vEmission, &emission, sizeof(float) * 3);
   }

   m_pd3dPrimaryDevice->basicShader->SetFloat4v(SHADER_basicPackedLights, (vec4*)l, sizeof(CLight) * MAX_LIGHT_SOURCES / (4 * sizeof(float)));
#endif
}

void Pin3D::UpdateBasicShaderMatrix(const Matrix3D& objectTrafo)
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

void Pin3D::UpdateBallShaderMatrix()
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

void Pin3D::PrepareFrame()
{
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
   // m_renderer->m_pd3dPrimaryDevice->AddRenderTargetDependency(m_renderer->m_pd3dPrimaryDevice->GetPreviousBackBufferTexture());
   m_pd3dPrimaryDevice->m_ballShader->SetTexture(SHADER_tex_ball_playfield, m_pd3dPrimaryDevice->GetPreviousBackBufferTexture()->GetColorSampler());
}

void Pin3D::DrawBulbLightBuffer()
{
   RenderDevice* p3dDevice = m_pd3dPrimaryDevice;
   const RenderPass *initial_rt = p3dDevice->GetCurrentPass();
   static int id = 0; id++;

   // switch to 'bloom' output buffer to collect all bulb lights
   p3dDevice->SetRenderTarget("Transmitted Light " + std::to_string(id) + " Clear", p3dDevice->GetBloomBufferTexture(), false);
   p3dDevice->ResetRenderState();
   p3dDevice->Clear(clearType::TARGET, 0, 1.0f, 0L);

   // Draw bulb lights
   m_render_mask |= Pin3D::LIGHT_BUFFER;
   p3dDevice->SetRenderTarget("Transmitted Light " + std::to_string(id), p3dDevice->GetBloomBufferTexture(), true, true);
   p3dDevice->SetRenderState(RenderState::ZENABLE, RenderState::RS_FALSE); // disable all z-tests as zbuffer is in different resolution
   for (Hitable *hitable : g_pplayer->m_vhitables)
      if (hitable->HitableGetItemType() == eItemLight)
         hitable->Render(m_render_mask);
   m_render_mask &= ~Pin3D::LIGHT_BUFFER;

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

void Pin3D::DrawStatics()
{
   const unsigned int mask = m_render_mask;
   m_render_mask |= Pin3D::STATIC_ONLY;
   for (Hitable* hitable : g_pplayer->m_vhitables)
      hitable->Render(m_render_mask);
   m_render_mask = mask;
}

void Pin3D::DrawDynamics(bool onlyBalls)
{
   if (!onlyBalls)
   {
      // Update Bulb light buffer and set up render pass dependencies
      DrawBulbLightBuffer();

      // Draw all parts
      const unsigned int mask = m_render_mask;
      m_render_mask |= Pin3D::DYNAMIC_ONLY;
      for (Hitable* hitable : g_pplayer->m_vhitables)
         hitable->Render(m_render_mask);
      m_render_mask = mask;
   }

   for (Ball* ball : g_pplayer->m_vball)
      ball->m_pballex->Render(m_render_mask);
}



#pragma region PostProcess

void Pin3D::SetScreenOffset(const float x, const float y)
{
   const float rotation = ANGTORAD(m_table->mViewSetups[m_table->m_BG_current_set].GetRotation(m_pd3dPrimaryDevice->m_width, m_pd3dPrimaryDevice->m_height));
   const float c = cosf(-rotation), s = sinf(-rotation);
   m_ScreenOffset.x = x * c - y * s;
   m_ScreenOffset.y = x * s + y * c;
}

void Pin3D::SSRefl()
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

void Pin3D::Bloom()
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

void Pin3D::PrepareVideoBuffers()
{
   const bool useAA = m_AAfactor > 1.0f;
   const bool stereo = m_stereo3D == STEREO_VR || ((m_stereo3D != STEREO_OFF) && g_pplayer->m_stereo3Denabled && (!g_pplayer->m_stereo3DfakeStereo || m_pd3dPrimaryDevice->DepthBufferReadBackAvailable()));
   // Since stereo is applied as a postprocess step for fake stereo, it disables AA and sharpening except for top/bottom & side by side modes
   const bool PostProcAA = !g_pplayer->m_stereo3DfakeStereo || (!stereo || (m_stereo3D == STEREO_TB) || (m_stereo3D == STEREO_SBS));
   const bool SMAA  = PostProcAA && m_FXAA == Quality_SMAA;
   const bool DLAA  = PostProcAA && m_FXAA == Standard_DLAA;
   const bool NFAA  = PostProcAA && m_FXAA == Fast_NFAA;
   const bool FXAA1 = PostProcAA && m_FXAA == Fast_FXAA;
   const bool FXAA2 = PostProcAA && m_FXAA == Standard_FXAA;
   const bool FXAA3 = PostProcAA && m_FXAA == Quality_FXAA;
   const bool ss_refl = m_ss_refl && m_table->m_enableSSR && m_pd3dPrimaryDevice->DepthBufferReadBackAvailable() && m_table->m_SSRScale > 0.f;
   const unsigned int sharpen = PostProcAA ? m_sharpen : 0;
   const bool useAO = g_pplayer->GetAOMode() == 2;
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
         m_pd3dPrimaryDevice->FBShader->SetTechnique(g_pplayer->m_toneMapper == TM_REINHARD ? SHADER_TECHNIQUE_fb_rhtonemap
                                                   : g_pplayer->m_toneMapper == TM_FILMIC   ? SHADER_TECHNIQUE_fb_fmtonemap
                                                                                            : SHADER_TECHNIQUE_fb_tmtonemap);
      else if (g_pplayer->m_BWrendering != 0)
         m_pd3dPrimaryDevice->FBShader->SetTechnique(g_pplayer->m_BWrendering == 1 ? SHADER_TECHNIQUE_fb_rhtonemap_no_filterRG : SHADER_TECHNIQUE_fb_rhtonemap_no_filterR);
      else if (g_pplayer->m_toneMapper == TM_REINHARD)
         m_pd3dPrimaryDevice->FBShader->SetTechnique(useAO ?
              useAA ? SHADER_TECHNIQUE_fb_rhtonemap_AO : SHADER_TECHNIQUE_fb_rhtonemap_AO_no_filter
            : useAA ? SHADER_TECHNIQUE_fb_rhtonemap    : SHADER_TECHNIQUE_fb_rhtonemap_no_filter);
      else if (g_pplayer->m_toneMapper == TM_FILMIC)
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
         if (g_pplayer->m_stereo3DDefocus != 0.f)
         {
            RenderTarget *tmpRT = m_pd3dPrimaryDevice->GetPostProcessRenderTarget(renderedRT);
            outputRT = m_pd3dPrimaryDevice->GetPostProcessRenderTarget(tmpRT);
            m_pd3dPrimaryDevice->DrawGaussianBlur(renderedRT, tmpRT, outputRT, abs(g_pplayer->m_stereo3DDefocus) * 39.f, g_pplayer->m_stereo3DDefocus > 0.f ? 0 : 1);
            renderedRT = outputRT;
         }
         // Stereo composition
         m_pd3dPrimaryDevice->SetRenderTarget("Stereo Anaglyph"s, m_pd3dPrimaryDevice->GetOutputBackBuffer(), false);
         m_pd3dPrimaryDevice->AddRenderTargetDependency(renderedRT);
         if (g_pplayer->m_stereo3DfakeStereo)
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
