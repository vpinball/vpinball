#include "stdafx.h"
#include "renderer/RenderDevice.h"
#include "renderer/Shader.h"
#include "math/math.h"
#include "ThreadPool.h"
#ifndef __STANDALONE__
#include "BAM/BAMView.h"
#endif

Pin3D::Pin3D()
{
   m_pd3dPrimaryDevice = nullptr;
   m_backGlass = nullptr;

   m_cam.x = 0.f;
   m_cam.y = 0.f;
   m_cam.z = 0.f;
   m_inc  = 0.f;
}

Pin3D::~Pin3D()
{
   delete m_mvp;

   m_gpu_profiler.Shutdown();

   m_pd3dPrimaryDevice->FreeShader();

   m_pinballEnvTexture.FreeStuff();

   m_builtinEnvTexture.FreeStuff();

   m_aoDitherTexture.FreeStuff();

   delete m_envRadianceTexture;

   delete m_pd3dPrimaryDevice;
   m_pd3dPrimaryDevice = nullptr;

   delete m_backGlass;
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

BaseTexture* EnvmapPrecalc(const Texture* envTex, const unsigned int rad_env_xres, const unsigned int rad_env_yres)
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

HRESULT Pin3D::InitPrimary(const bool fullScreen, const int colordepth, int& refreshrate, VideoSyncMode& syncMode, const float AAfactor,
   const StereoMode stereo3D, const unsigned int FXAA, const bool sharpen, const bool ss_refl)
{
   const int display = g_pvp->m_primaryDisplay ? 0 : g_pplayer->m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "Display"s, 0);
   vector<DisplayConfig> displays;
   getDisplayList(displays);
   int adapter = 0;
   for (vector<DisplayConfig>::iterator dispConf = displays.begin(); dispConf != displays.end(); ++dispConf)
      if (display == dispConf->display)
         adapter = dispConf->adapter;

   m_pd3dPrimaryDevice = new RenderDevice(g_pplayer->GetHwnd(), m_viewPort.Width, m_viewPort.Height, fullScreen, colordepth, AAfactor, stereo3D, FXAA, sharpen, ss_refl,
      g_pplayer->m_useNvidiaApi, g_pplayer->m_disableDWM, g_pplayer->m_BWrendering);
   try {
      m_pd3dPrimaryDevice->CreateDevice(refreshrate, syncMode, adapter);
   }
   catch (...) {
      return E_FAIL;
   }

   m_viewPort.Width = m_pd3dPrimaryDevice->m_width;
   m_viewPort.Height = m_pd3dPrimaryDevice->m_height;

   if (!m_pd3dPrimaryDevice->LoadShaders())
      return E_FAIL;

#ifndef __STANDALONE__
   BAMView::init();
#endif

   const bool compressTextures = g_pplayer->m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "CompressTextures"s, false);
   m_pd3dPrimaryDevice->CompressTextures(compressTextures);

   m_pd3dPrimaryDevice->SetViewport(&m_viewPort);

   return S_OK;
}

HRESULT Pin3D::InitPin3D(const bool fullScreen, const int width, const int height, const int colordepth, int& refreshrate, VideoSyncMode& syncMode,
   const float AAfactor, const StereoMode stereo3D, const unsigned int FXAA, const bool sharpen, const bool ss_refl)
{
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

   if (FAILED(InitPrimary(fullScreen, colordepth, refreshrate, syncMode, AAfactor, stereo3D, FXAA, sharpen, ss_refl)))
      return E_FAIL;

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

   return S_OK;
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

void Pin3D::InitLights()
{
   //m_pd3dPrimaryDevice->basicShader->SetInt("iLightPointNum",MAX_LIGHT_SOURCES);

   g_pplayer->m_ptable->m_Light[0].pos.x = g_pplayer->m_ptable->m_right*0.5f;
   g_pplayer->m_ptable->m_Light[1].pos.x = g_pplayer->m_ptable->m_right*0.5f;
   g_pplayer->m_ptable->m_Light[0].pos.y = g_pplayer->m_ptable->m_bottom*(float)(1.0 / 3.0);
   g_pplayer->m_ptable->m_Light[1].pos.y = g_pplayer->m_ptable->m_bottom*(float)(2.0 / 3.0);
   g_pplayer->m_ptable->m_Light[0].pos.z = g_pplayer->m_ptable->m_lightHeight;
   g_pplayer->m_ptable->m_Light[1].pos.z = g_pplayer->m_ptable->m_lightHeight;

   vec4 emission = convertColor(g_pplayer->m_ptable->m_Light[0].emission);
   emission.x *= g_pplayer->m_ptable->m_lightEmissionScale*g_pplayer->m_globalEmissionScale;
   emission.y *= g_pplayer->m_ptable->m_lightEmissionScale*g_pplayer->m_globalEmissionScale;
   emission.z *= g_pplayer->m_ptable->m_lightEmissionScale*g_pplayer->m_globalEmissionScale;

#ifdef ENABLE_SDL
   float lightPos[MAX_LIGHT_SOURCES][4] = { 0.f };
   float lightEmission[MAX_LIGHT_SOURCES][4] = { 0.f };

   for (unsigned int i = 0; i < MAX_LIGHT_SOURCES; ++i)
   {
      memcpy(&lightPos[i], &g_pplayer->m_ptable->m_Light[i].pos, sizeof(float) * 3);
      memcpy(&lightEmission[i], &emission, sizeof(float) * 3);
   }

   m_pd3dPrimaryDevice->basicShader->SetFloat4v(SHADER_basicLightPos, (vec4*) lightPos, MAX_LIGHT_SOURCES);
   m_pd3dPrimaryDevice->basicShader->SetFloat4v(SHADER_basicLightEmission, (vec4*) lightEmission, MAX_LIGHT_SOURCES);
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

   m_pd3dPrimaryDevice->basicShader->SetFloat4v(SHADER_basicPackedLights, (vec4*) l, sizeof(CLight) * MAX_LIGHT_SOURCES / (4 * sizeof(float)));
#endif

   vec4 amb_lr = convertColor(g_pplayer->m_ptable->m_lightAmbient, g_pplayer->m_ptable->m_lightRange);
   amb_lr.x *= g_pplayer->m_globalEmissionScale;
   amb_lr.y *= g_pplayer->m_globalEmissionScale;
   amb_lr.z *= g_pplayer->m_globalEmissionScale;
   m_pd3dPrimaryDevice->basicShader->SetVector(SHADER_cAmbient_LightRange, &amb_lr);
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
   viewSetup.ComputeMVP(g_pplayer->m_ptable, 
      (float)((double)m_pd3dPrimaryDevice->GetBackBufferTexture()->GetWidth() / (double)m_pd3dPrimaryDevice->GetBackBufferTexture()->GetHeight()),
      stereo, *m_mvp, vec3(m_cam.x, m_cam.y, m_cam.z), m_inc,
      xpixoff / (float)m_pd3dPrimaryDevice->GetBackBufferTexture()->GetWidth(), ypixoff / (float)m_pd3dPrimaryDevice->GetBackBufferTexture()->GetHeight());
   InitLights();
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
   constexpr float wz = 0.f;
   const float wx = ((wz - p1.z)*(p2.x - p1.x)) / (p2.z - p1.z) + p1.x;
   const float wy = ((wz - p1.z)*(p2.y - p1.y)) / (p2.z - p1.z) + p1.y;
   const Vertex3Ds vertex(wx, wy, wz);
   return vertex;
}

void Pin3D::UpdateBAMHeadTracking()
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
