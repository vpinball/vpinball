#include "stdafx.h"
#include "RenderDevice.h"
#include "Shader.h"
#include "math/math.h"
#include "inc/ThreadPool.h"
#include "BAM/BAMView.h"

Pin3D::Pin3D()
{
   m_pd3dPrimaryDevice = nullptr;
   m_pd3dSecondaryDevice = nullptr;
   m_pddsStatic = nullptr;
   m_envRadianceTexture = nullptr;
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

   if (m_envRadianceTexture)
   {
      m_pd3dPrimaryDevice->m_texMan.UnloadTexture(m_envRadianceTexture);
      delete m_envRadianceTexture;
      m_envRadianceTexture = nullptr;
   }

   delete m_pddsStatic;

   if(m_pd3dPrimaryDevice != m_pd3dSecondaryDevice)
      delete m_pd3dSecondaryDevice;
   delete m_pd3dPrimaryDevice;
   m_pd3dPrimaryDevice = nullptr;
   m_pd3dSecondaryDevice = nullptr;

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
   g_pvp->ProfileLog("EnvmapPrecalc Start"s);
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

   g_pvp->ProfileLog("EnvmapPrecalc End"s);

#ifdef __OPENGLES__
   if (radTex->m_format == BaseTexture::SRGB || radTex->m_format == BaseTexture::RGB_FP16) {
      radTex->AddAlpha();
   }
#endif

   return radTex;
}

HRESULT Pin3D::InitPrimary(const bool fullScreen, const int colordepth, int &refreshrate, const int VSync, const float AAfactor, const StereoMode stereo3D, const unsigned int FXAA, const bool sharpen, const bool ss_refl)
{
   const int display = g_pvp->m_primaryDisplay ? 0 : LoadValueIntWithDefault(regKey[RegName::Player], "Display"s, 0);
   vector<DisplayConfig> displays;
   getDisplayList(displays);
   int adapter = 0;
   for (vector<DisplayConfig>::iterator dispConf = displays.begin(); dispConf != displays.end(); ++dispConf)
      if (display == dispConf->display)
         adapter = dispConf->adapter;

   m_pd3dPrimaryDevice = new RenderDevice(g_pplayer->GetHwnd(), m_viewPort.Width, m_viewPort.Height, fullScreen, colordepth, VSync, AAfactor, stereo3D, FXAA, sharpen, ss_refl, g_pplayer->m_useNvidiaApi, g_pplayer->m_disableDWM, g_pplayer->m_BWrendering);
   try {
      m_pd3dPrimaryDevice->CreateDevice(refreshrate, adapter);
   }
   catch (...) {
      return E_FAIL;
   }

   if (m_stereo3D == STEREO_VR)
   { // VR mode renders to a double widthed render target => pin3d viewport is the halh of the render buffer
      m_viewPort.Width = m_pd3dPrimaryDevice->m_width / 2;
      m_viewPort.Height = m_pd3dPrimaryDevice->m_height;
   }
#ifdef ENABLE_SDL
   else if (m_stereo3D >= STEREO_ANAGLYPH_RC && m_stereo3D <= STEREO_ANAGLYPH_AB)
   { // Anaglyph mode renders to a double widthed render target => pin3d viewport is the halh of the render buffer
      m_viewPort.Width = m_pd3dPrimaryDevice->m_width / 2;
      m_viewPort.Height = m_pd3dPrimaryDevice->m_height;
   }
#endif
   else
   { // Use the effective size of the created device's window (should be the same as the requested)
      m_viewPort.Width = m_pd3dPrimaryDevice->m_width;
      m_viewPort.Height = m_pd3dPrimaryDevice->m_height;
   }

   if (!m_pd3dPrimaryDevice->LoadShaders())
      return E_FAIL;

   BAMView::init();

   const bool compressTextures = LoadValueBoolWithDefault(regKey[RegName::Player], "CompressTextures"s, false);
   m_pd3dPrimaryDevice->CompressTextures(compressTextures);

   m_pd3dPrimaryDevice->SetViewport(&m_viewPort);

   // Static render target is a copy of the main back buffer (without MSAA since static prerender is done with custom antialiasing)
   if (m_stereo3D != STEREO_VR)
      m_pddsStatic = m_pd3dPrimaryDevice->GetBackBufferTexture()->Duplicate("StaticPreRender"s);
   else
      m_pddsStatic = nullptr;

   return S_OK;
}

HRESULT Pin3D::InitPin3D(const bool fullScreen, const int width, const int height, const int colordepth, int& refreshrate, const int VSync, const float AAfactor, const StereoMode stereo3D, const unsigned int FXAA, const bool sharpen, const bool ss_refl)
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

   if (FAILED(InitPrimary(fullScreen, colordepth, refreshrate, VSync, AAfactor, stereo3D, FXAA, sharpen, ss_refl)))
      return E_FAIL;

   m_pd3dSecondaryDevice = m_pd3dPrimaryDevice; //!! for now, there is no secondary device :/

   //

   if (m_stereo3D == STEREO_VR)
      m_backGlass = new BackGlass(m_pd3dSecondaryDevice, g_pplayer->m_ptable->GetDecalsEnabled() ? g_pplayer->m_ptable->GetImage(g_pplayer->m_ptable->m_BG_image[g_pplayer->m_ptable->m_BG_current_set]) : nullptr);
   else
      m_backGlass = nullptr;

   // Create the "static" color buffer.
   // This will hold a pre-rendered image of the table and any non-changing elements (ie ramps, decals, etc).

   m_pinballEnvTexture.CreateFromResource(IDB_BALL);
   m_aoDitherTexture.CreateFromResource(IDB_AO_DITHER);

   m_envTexture = g_pplayer->m_ptable->GetImage(g_pplayer->m_ptable->m_envImage);
   m_builtinEnvTexture.CreateFromResource(IDB_ENV);

   const Texture * const envTex = m_envTexture ? m_envTexture : &m_builtinEnvTexture;

   const unsigned int envTexHeight = min(envTex->m_pdsBuffer->height(),256u) / 8;
   const unsigned int envTexWidth = envTexHeight*2;

   m_envRadianceTexture = EnvmapPrecalc(envTex, envTexWidth, envTexHeight);

   m_pd3dPrimaryDevice->m_texMan.SetDirty(m_envRadianceTexture);

   //

   InitPrimaryRenderState();

   //m_gpu_profiler.Init(m_pd3dDevice->GetCoreDevice()); // done by first BeginFrame() call lazily

   return S_OK;
}

void Pin3D::InitRenderState(RenderDevice * const pd3dDevice)
{
   pd3dDevice->SetRenderState(RenderState::ALPHABLENDENABLE, RenderState::RS_FALSE);

   pd3dDevice->SetRenderState(RenderState::ZENABLE, RenderState::RS_TRUE);
   pd3dDevice->SetRenderState(RenderState::ZWRITEENABLE, RenderState::RS_TRUE);
   pd3dDevice->SetRenderState(RenderState::ZFUNC, RenderState::Z_LESSEQUAL);

   pd3dDevice->SetRenderStateCulling(RenderState::CULL_CCW);

   pd3dDevice->SetRenderState(RenderState::CLIPPLANEENABLE, RenderState::RS_FALSE);

   // initialize first texture stage
#ifndef ENABLE_SDL
   CHECKD3D(pd3dDevice->GetCoreDevice()->SetRenderState(D3DRS_LIGHTING, FALSE));
   CHECKD3D(pd3dDevice->GetCoreDevice()->SetRenderState(D3DRS_CLIPPING, FALSE));
   CHECKD3D(pd3dDevice->GetCoreDevice()->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1));
   CHECKD3D(pd3dDevice->GetCoreDevice()->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE));
   CHECKD3D(pd3dDevice->GetCoreDevice()->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0));
   CHECKD3D(pd3dDevice->GetCoreDevice()->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE));
   CHECKD3D(pd3dDevice->GetCoreDevice()->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE));
   CHECKD3D(pd3dDevice->GetCoreDevice()->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TFACTOR)); // default tfactor: 1,1,1,1
#endif
}

void Pin3D::InitPrimaryRenderState()
{
   InitRenderState(m_pd3dPrimaryDevice);
}

void Pin3D::InitSecondaryRenderState()
{
   InitRenderState(m_pd3dSecondaryDevice);
}

void Pin3D::DrawBackground()
{
   const PinTable * const ptable = g_pplayer->m_ptable;
   Texture * const pin = ptable->GetDecalsEnabled()
      ? ptable->GetImage(ptable->m_BG_image[ptable->m_BG_current_set])
      : nullptr;
   if (pin)
   {
      m_pd3dPrimaryDevice->Clear(clearType::ZBUFFER, 0, 1.0f, 0L);

      m_pd3dPrimaryDevice->SetRenderState(RenderState::ZWRITEENABLE, RenderState::RS_FALSE);
      m_pd3dPrimaryDevice->SetRenderState(RenderState::ZENABLE, RenderState::RS_FALSE);

      if (g_pplayer->m_ptable->m_tblMirrorEnabled ^ g_pplayer->IsRenderPass(Player::REFLECTION_PASS))
         m_pd3dPrimaryDevice->SetRenderStateCulling(RenderState::CULL_NONE);

      m_pd3dPrimaryDevice->SetRenderState(RenderState::ALPHABLENDENABLE, RenderState::RS_FALSE);

      // FIXME this should be called with a trilinear/anisotropy filtering override
      g_pplayer->Spritedraw(0.f, 0.f, 1.f, 1.f, 0xFFFFFFFF, pin, ptable->m_ImageBackdropNightDay ? sqrtf(g_pplayer->m_globalEmissionScale) : 1.0f, true);

      if (g_pplayer->m_ptable->m_tblMirrorEnabled ^ g_pplayer->IsRenderPass(Player::REFLECTION_PASS))
         m_pd3dPrimaryDevice->SetRenderStateCulling(RenderState::CULL_CCW);

      m_pd3dPrimaryDevice->SetRenderState(RenderState::ZENABLE, RenderState::RS_TRUE);
      m_pd3dPrimaryDevice->SetRenderState(RenderState::ZWRITEENABLE, RenderState::RS_TRUE);
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
   PinProjection proj;
   proj.Setup(g_pplayer->m_ptable, m_viewPort, g_pplayer->m_cameraMode, m_stereo3D, m_cam, m_inc, xpixoff, ypixoff);
   m_mvp->SetModel(proj.m_matWorld);
   m_mvp->SetView(proj.m_matView);
   for (unsigned int eye = 0; eye < m_mvp->m_nEyes; eye++)
      m_mvp->SetProj(eye, proj.m_matProj[eye]);
   InitLights();
}

void Pin3D::EnableAlphaBlend(const bool additiveBlending, const bool set_dest_blend, const bool set_blend_op) const
{
   m_pd3dPrimaryDevice->SetRenderState(RenderState::ALPHABLENDENABLE, RenderState::RS_TRUE);
   m_pd3dPrimaryDevice->SetRenderState(RenderState::SRCBLEND, RenderState::SRC_ALPHA);
   if (set_dest_blend)
      m_pd3dPrimaryDevice->SetRenderState(RenderState::DESTBLEND, additiveBlending ? RenderState::ONE : RenderState::INVSRC_ALPHA);
   if (set_blend_op)
      m_pd3dPrimaryDevice->SetRenderState(RenderState::BLENDOP, RenderState::BLENDOP_ADD);
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
   const float wz = g_pplayer->m_ptable->m_tableheight;
   const float wx = ((wz - p1.z)*(p2.x - p1.x)) / (p2.z - p1.z) + p1.x;
   const float wy = ((wz - p1.z)*(p2.y - p1.y)) / (p2.z - p1.z) + p1.y;
   const Vertex3Ds vertex(wx, wy, wz);
   return vertex;
}

void PinProjection::Setup(const PinTable* table, const ViewPort& viewPort, const bool cameraMode, const StereoMode stereo3D, 
   const Vertex3Ds& cam, const float cam_inc, const float xpixoff, const float ypixoff)
{

   const float rotation = ANGTORAD(table->m_BG_rotation[table->m_BG_current_set]);
   const float inclination = ANGTORAD(table->m_BG_inclination[table->m_BG_current_set]);
   const float FOV = (table->m_BG_FOV[table->m_BG_current_set] < 1.0f) ? 1.0f : table->m_BG_FOV[table->m_BG_current_set]; // Can't have a real zero FOV, but this will look almost the same
   const CameraLayoutMode layoutMode = table->m_cameraLayoutMode;
   const bool FSS_mode = table->m_BG_enable_FSS;
   const bool isLegacy = layoutMode == CLM_RELATIVE;

   m_rcviewport.left = 0;
   m_rcviewport.top = 0;
   m_rcviewport.right = viewPort.Width;
   m_rcviewport.bottom = viewPort.Height;

   const float aspect = ((float)viewPort.Width) / ((float)viewPort.Height); //(float)(4.0/3.0);

   vector<Vertex3Ds> vvertex3D;
   for (size_t i = 0; i < table->m_vedit.size(); ++i)
      table->m_vedit[i]->GetBoundingVertices(vvertex3D);

   // next 4 def values for layout portrait(game vert) in landscape(screen horz)
   // for FSS, force an offset to camy which drops the table down 1/3 of the way.
   // some values to camy have been commented out because I found the default value
   // better and just modify the camz and keep the table design inclination
   // within 50-60 deg and 40-50 FOV in editor.
   // these values were tested against all known video modes upto 1920x1080
   // in landscape and portrait on the display
   const float camx = cam.x;
   const float camy = cam.y + (isLegacy && FSS_mode ? 500.0f : 0.f);
   float camz = cam.z;
   const float inc = inclination + cam_inc + (isLegacy && FSS_mode ? 0.2f : 0.f);

   if (isLegacy && FSS_mode)
   {
      //m_proj.m_rcviewport.right = m_viewPort.Height;
      //m_proj.m_rcviewport.bottom = m_viewPort.Width;
      const int width = GetSystemMetrics(SM_CXSCREEN);
      const int height = GetSystemMetrics(SM_CYSCREEN);

      // layout landscape(game horz) in lanscape(LCD\LED horz)
      if ((viewPort.Width > viewPort.Height) && (height < width))
      {
         //inc += 0.1f;       // 0.05-best, 0.1-good, 0.2-bad > (0.2 terrible original)
         //camy -= 30.0f;     // 70.0f original // 100
         if (aspect > 1.6f)
            camz -= 1170.0f; // 700
         else if (aspect > 1.5f)
            camz -= 1070.0f; // 650
         else if (aspect > 1.4f)
            camz -= 900.0f; // 580
         else if (aspect > 1.3f)
            camz -= 820.0f; // 500 // 600
         else
            camz -= 800.0f; // 480
      }
      else
      {
         // layout potrait(game vert) in portrait(LCD\LED vert)
         if (height > width)
         {
            if (aspect > 0.6f)
            {
               camz += 10.0f;
               //camy += 50.0f;
            }
            else if (aspect > 0.5f)
            {
               camz += 300.0f;
               //camy += 100.0f;
            }
            else
            {
               camz += 300.0f;
               //camy += 200.0f;
            }
         }
         // layout landscape(game horz) in portrait(LCD\LED vert), who would but the UI allows for it!
         else
         {
         }
      }
   }

   // Original matrix stack was: [Lb.Rx.Rz.T.S.Rpi] . [P]   ([first] part is view matrix, [second] is projection matrix)
   // This leads to a non orthonormal view matrix since layback and scaling are part of it. This slightly breaks lighting, reflection and stereo.
   // 
   // To improve this situation, a new 'absolute' camera mode was added. The matrix stack was also modified to move scaling (S) and DirectX 
   // coordinate system (Rpi) from view to projection matrix. The final matrix stacks are the following:
   // 'relative' mode: [Lb.Rx.Rz.T] . [Rpi.S.P]
   // 'absolute' mode:       [T.Rx] . [Ry.Rpi.S.P]
   // 
   // This seems ok for 'absolute' mode since the view is orthonormal (it transforms normals and points without changing their length or relative angle).

   m_matWorld.SetIdentity();

   Matrix3D coords, rotx, trans, rotz, proj, projTrans;
   projTrans.SetTranslation((float)((double)xpixoff / (double)viewPort.Width), (float)((double)ypixoff / (double)viewPort.Height), 0.f); // in-pixel offset for manual oversampling
   coords.SetScaling(table->m_BG_scalex[table->m_BG_current_set], isLegacy ? -table->m_BG_scaley[table->m_BG_current_set] : -1.f, -1.f); // Stretch viewport, also revert Y and Z axis to convert to D3D coordinate system
   rotx.SetRotateX(inc); // Player head inclination
   rotz.SetRotateZ(rotation); // Viewport rotation
   if (isLegacy)
   {
      FitCameraToVertices(vvertex3D, aspect, rotation, inc, FOV, table->m_BG_xlatez[table->m_BG_current_set], table->m_BG_layback[table->m_BG_current_set]);
      trans.SetTranslation(
         table->m_BG_xlatex[table->m_BG_current_set] - m_vertexcamera.x + camx,
         table->m_BG_xlatey[table->m_BG_current_set] - m_vertexcamera.y + camy,
         -m_vertexcamera.z + camz);
      // Recompute near and far plane (workaround for VP9 FitCameraToVertices bugs), needs a complete matView with DirectX coordinate change
      Matrix3D layback = ComputeLaybackTransform(table->m_BG_layback[table->m_BG_current_set]); // Layback to skew the view backward (bug: this breaks orthonormal property of the view matrix)
      if (cameraMode && (table->m_BG_current_set == BG_DESKTOP || table->m_BG_current_set == BG_FSS))
         m_matView = layback * rotz * rotx * trans * projTrans * coords;
      else
         m_matView = layback * rotx * rotz * trans * projTrans * coords;
      ComputeNearFarPlane(vvertex3D);
      if (fabsf(inc) < 0.0075f) //!! magic threshold, otherwise kicker holes are missing for inclination ~0
         m_rzfar += 10.f;
   }
   else
   {
      trans.SetTranslation(
         -table->m_BG_xlatex[table->m_BG_current_set] + cam.x - 0.5f * table->m_right,
         -table->m_BG_xlatey[table->m_BG_current_set] + cam.y - table->m_bottom,
         -table->m_BG_xlatez[table->m_BG_current_set] + cam.z);
      /* m_matView = trans * rotx * rotz * projTrans * coords;
      ComputeNearFarPlane(vvertex3D);
      if (fabsf(inc) < 0.0075f) //!! magic threshold, otherwise kicker holes are missing for inclination ~0
         m_rzfar += 10.f;*/
      m_rznear = CMTOVPU(5.f); //500.f;
      m_rzfar = CMTOVPU(300.f); // 5000.0f;
   }

   const float sx = table->m_BG_scalex[table->m_BG_current_set];
   const float sy = table->m_BG_scaley[table->m_BG_current_set];
   const float ymax = m_rznear * tanf(0.5f * ANGTORAD(FOV));
   const float xmax = ymax * aspect;
   const float ofs = isLegacy ? 0.f : 0.01f * table->m_BG_layback[table->m_BG_current_set];
   const float xofs = m_rznear * ofs * sinf(rotation);
   const float yofs = m_rznear * ofs * cosf(rotation);
   proj.SetPerspectiveOffCenterLH(-xmax + xofs, xmax + xofs, -ymax + yofs, ymax + yofs, m_rznear, m_rzfar);
   if (isLegacy)
   {
      Matrix3D layback = ComputeLaybackTransform(table->m_BG_layback[table->m_BG_current_set]); // Layback to skew the view backward (bug: this breaks orthonormal property of the view matrix)
      if (cameraMode && (table->m_BG_current_set == BG_DESKTOP || table->m_BG_current_set == BG_FSS))
         m_matView = layback * rotz * rotx * trans;
      else
         m_matView = layback * rotx * rotz * trans;
      m_matProj[0] = projTrans * coords * proj;
   }
   else
   {
      m_matView = trans * rotx;
      m_matProj[0] = rotz * projTrans * coords * proj;
   }

#ifdef ENABLE_SDL
   if (stereo3D != STEREO_OFF)
   {
      // Create eye projection matrices for real stereo (not VR but anaglyph,...)
      if (isLegacy)
         m_matProj[1] = m_matProj[0];
      else
      {
         // 63mm is the average distance between eyes (varies from 54 to 74mm between adults, 43 to 58mm for children), 50 VPUnit is 1.25 inches
         const float stereo3DMS = LoadValueFloatWithDefault(regKey[RegName::Player], "Stereo3DEyeSeparation"s, 63.0f);
         const float halfEyeDist = 0.5f * MMTOVPU(stereo3DMS);
         Matrix3D invView(m_matView);
         invView.Invert();
         // Compute the view orthonormal basis
         Matrix3D baseView, coords;
         coords.SetScaling(1.f, -1.f, -1.f);
         baseView = coords * invView;
         const vec3 right = baseView.GetOrthoNormalRight();
         const vec3 up = baseView.GetOrthoNormalUp();
         const vec3 dir = baseView.GetOrthoNormalDir();
         const vec3 pos = baseView.GetOrthoNormalPos();
         // Default is to look at the ball (playfield level = 0 + ball radius = 50)
         float camDistance = (50.f - pos.z) / dir.z;
         // Clamp it to a reasonable range, a normal viewing distance being around 80cm between view focus (table) and viewer (depends a lot on the player size & position)
         constexpr float minCamDistance = CMTOVPU(30.f);
         constexpr float maxCamDistance = CMTOVPU(200.f);
         const vec3 at = pos + dir * clamp(camDistance, minCamDistance, maxCamDistance);
         Matrix3D lookat = Matrix3D::MatrixLookAtLH(pos + (halfEyeDist * right), at, up); // Apply offset & rotation to the right eye projection
         m_matProj[1] = invView * lookat * coords * m_matProj[0];
         lookat = Matrix3D::MatrixLookAtLH(pos + (-halfEyeDist * right), at, up); // Apply offset & rotation to the left eye projection
         m_matProj[0] = invView * lookat * coords * m_matProj[0];
      }
   }
#endif
}

void PinProjection::FitCameraToVertices(const vector<Vertex3Ds>& pvvertex3D, float aspect, float rotation, float inclination, float FOV, float xlatez, float layback)
{
   // Determine camera distance
   const float rrotsin = sinf(rotation);
   const float rrotcos = cosf(rotation);
   const float rincsin = sinf(inclination);
   const float rinccos = cosf(inclination);

   const float slopey = tanf(0.5f*ANGTORAD(FOV)); // *0.5 because slope is half of FOV - FOV includes top and bottom

   // Field of view along the axis = atan(tan(yFOV)*width/height)
   // So the slope of x simply equals slopey*width/height

   const float slopex = slopey*aspect;

   float maxyintercept = -FLT_MAX;
   float minyintercept = FLT_MAX;
   float maxxintercept = -FLT_MAX;
   float minxintercept = FLT_MAX;

   const Matrix3D laybackTrans = ComputeLaybackTransform(layback);

   for (size_t i = 0; i < pvvertex3D.size(); ++i)
   {
      Vertex3Ds v = laybackTrans.MultiplyVector(pvvertex3D[i]);

      // Rotate vertex about x axis according to incoming inclination
      float temp = v.y;
      v.y = rinccos*temp - rincsin*v.z;
      v.z = rincsin*temp + rinccos*v.z;

      // Rotate vertex about z axis according to incoming rotation
      temp = v.x;
      v.x = rrotcos*temp - rrotsin*v.y;
      v.y = rrotsin*temp + rrotcos*v.y;

      // Extend slope lines from point to find camera intersection
      maxyintercept = max(maxyintercept, v.y + slopey*v.z);
      minyintercept = min(minyintercept, v.y - slopey*v.z);
      maxxintercept = max(maxxintercept, v.x + slopex*v.z);
      minxintercept = min(minxintercept, v.x - slopex*v.z);
   }

   slintf("maxy: %f\n", maxyintercept);
   slintf("miny: %f\n", minyintercept);
   slintf("maxx: %f\n", maxxintercept);
   slintf("minx: %f\n", minxintercept);

   // Find camera center in xy plane

   const float ydist = (maxyintercept - minyintercept) / (slopey*2.0f);
   const float xdist = (maxxintercept - minxintercept) / (slopex*2.0f);
   m_vertexcamera.z = max(ydist, xdist) + xlatez;
   m_vertexcamera.y = (maxyintercept + minyintercept) * 0.5f;
   m_vertexcamera.x = (maxxintercept + minxintercept) * 0.5f;
}

void PinProjection::ComputeNearFarPlane(const vector<Vertex3Ds>& verts)
{
   m_rznear = FLT_MAX;
   m_rzfar = -FLT_MAX;

   Matrix3D matWorldView;
   m_matView.Multiply(m_matWorld, matWorldView);

   for (size_t i = 0; i < verts.size(); ++i)
   {
      const float tempz = matWorldView.MultiplyVector(verts[i]).z;

      // Extend z-range if necessary
      m_rznear = min(m_rznear, tempz);
      m_rzfar = max(m_rzfar, tempz);
   }

   slintf("m_rznear: %f\n", m_rznear);
   slintf("m_rzfar : %f\n", m_rzfar);

   //m_rznear *= 0.89f; //!! magic, influences also stereo3D code
   // Avoid near plane below 1 which result in loss of precision and z rendering artefacts
   if (m_rznear < 1.0f)
      m_rznear = 1.0f;

   m_rzfar *= 1.01f;
   // Avoid div-0 problem (div by far - near)
   if (m_rzfar <= m_rznear)
      m_rzfar = m_rznear + 1.0f;

   // FIXME for the time being, the result is not that good since neither primitives vertices are taken in account, nor reflected geometry, so just add a margin
   m_rzfar += 1000.0f;
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
