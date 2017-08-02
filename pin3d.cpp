#include "StdAfx.h"
#include "RenderDevice.h"

int NumVideoBytes = 0;

Pin3D::Pin3D()
{
   m_pddsBackBuffer = NULL;
   m_pddsAOBackBuffer = NULL;
   m_pddsAOBackTmpBuffer = NULL;
   m_pddsZBuffer = NULL;
   m_pdds3DZBuffer = NULL;
   m_pd3dDevice = NULL;
   m_pddsStatic = NULL;
   m_pddsStaticZ = NULL;
   m_envRadianceTexture = NULL;
   tableVBuffer = NULL;
   tableIBuffer = NULL;

   m_camx = 0.f;
   m_camy = 0.f;
   m_camz = 0.f;
   m_inc  = 0.f;
}

Pin3D::~Pin3D()
{
#ifdef FPS
   m_gpu_profiler.Shutdown();
#endif
   m_pd3dDevice->SetZBuffer(NULL);
   m_pd3dDevice->FreeShader();

   pinballEnvTexture.FreeStuff();

   envTexture.FreeStuff();

   aoDitherTexture.FreeStuff();

   if (m_envRadianceTexture)
   {
      m_pd3dDevice->m_texMan.UnloadTexture(m_envRadianceTexture);
      delete m_envRadianceTexture;
      m_envRadianceTexture = NULL;
   }

   if (tableVBuffer)
      tableVBuffer->release();
   if (tableIBuffer)
      tableIBuffer->release();

   SAFE_RELEASE(m_pddsAOBackBuffer);
   SAFE_RELEASE(m_pddsAOBackTmpBuffer);
   if (!m_pd3dDevice->m_useNvidiaApi && m_pd3dDevice->m_INTZ_support)
   {
      SAFE_RELEASE_NO_SET((D3DTexture*)m_pddsStaticZ);
      SAFE_RELEASE_NO_SET((D3DTexture*)m_pddsZBuffer);
   }
   else
   {
      SAFE_RELEASE_NO_SET((RenderTarget*)m_pddsStaticZ);
      SAFE_RELEASE_NO_SET((RenderTarget*)m_pddsZBuffer);
   }
   m_pddsStaticZ = NULL;
   m_pddsZBuffer = NULL;
   SAFE_RELEASE(m_pdds3DZBuffer);
   SAFE_RELEASE(m_pddsStatic);
   SAFE_RELEASE_NO_RCC(m_pddsBackBuffer);

   delete m_pd3dDevice;
}

void Pin3D::TransformVertices(const Vertex3D_NoTex2 * rgv, const WORD * rgi, int count, Vertex2D * rgvout) const
{
   // Get the width and height of the viewport. This is needed to scale the
   // transformed vertices to fit the render window.
   const float rClipWidth = (float)vp.Width*0.5f;
   const float rClipHeight = (float)vp.Height*0.5f;
   const int xoffset = vp.X;
   const int yoffset = vp.Y;

   // Transform each vertex through the current matrix set
   for (int i = 0; i < count; ++i)
   {
      const int l = rgi ? rgi[i] : i;

      // Get the untransformed vertex position
      const float x = rgv[l].x;
      const float y = rgv[l].y;
      const float z = rgv[l].z;

      // Transform it through the current matrix set
      const float xp = m_proj.m_matrixTotal._11*x + m_proj.m_matrixTotal._21*y + m_proj.m_matrixTotal._31*z + m_proj.m_matrixTotal._41;
      const float yp = m_proj.m_matrixTotal._12*x + m_proj.m_matrixTotal._22*y + m_proj.m_matrixTotal._32*z + m_proj.m_matrixTotal._42;
      const float wp = m_proj.m_matrixTotal._14*x + m_proj.m_matrixTotal._24*y + m_proj.m_matrixTotal._34*z + m_proj.m_matrixTotal._44;

      // Finally, scale the vertices to screen coords. This step first
      // "flattens" the coordinates from 3D space to 2D device coordinates,
      // by dividing each coordinate by the wp value. Then, the x- and
      // y-components are transformed from device coords to screen coords.
      // Note 1: device coords range from -1 to +1 in the viewport.
      const float inv_wp = 1.0f / wp;
      const float vTx = (1.0f + xp*inv_wp) * rClipWidth + xoffset;
      const float vTy = (1.0f - yp*inv_wp) * rClipHeight + yoffset;

      rgvout[l].x = vTx;
      rgvout[l].y = vTy;
   }
}

void EnvmapPrecalc(const void* const __restrict envmap, const DWORD env_xres, const DWORD env_yres, void* const __restrict rad_envmap, const DWORD rad_env_xres, const DWORD rad_env_yres, const bool isHDR)
{
   // brute force sampling over hemisphere for each normal direction of the to-be-(ir)radiance-baked environment
   // not the fastest solution, could do a "cosine convolution" over the picture instead (where also just 1024 or x samples could be used per pixel)
   // but with this implementation one can also have custom maps/LUTs for glossy, etc. later-on
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
            Vertex3Ds l = sphere_sample((float)s*(float)(1.0/num_samples), radical_inverse(s)); // QMC hammersley point set
            float NdotL = l.Dot(n);
            if(NdotL < 0.0f) // flip if on backside of hemisphere
            {
               NdotL = -NdotL;
               l = -l;
            }
#else
            //Vertex3Ds cos_hemisphere_sample(const Vertex3Ds &normal, Vertex2D uv) { float theta = (float)(2.*M_PI) * uv.x; uv.y = 2.f * uv.y - 1.f; Vertex3Ds spherePoint(sqrt(1.f - uv.y * uv.y) * Vertex2D(cosf(theta), sinf(theta)), uv.y); return normalize(normal + spherePoint); }
            const Vertex3Ds l = rotate_to_vector_upper(cos_hemisphere_sample((float)s*(float)(1.0 / num_samples), radical_inverse(s)), n); // QMC hammersley point set
#endif
            // trafo from light direction to envmap
            const float u = atan2f(l.y, l.x) * (float)(0.5 / M_PI) + 0.5f;
            const float v = acosf(l.z) * (float)(1.0 / M_PI);

	    float r,g,b;
	    if(isHDR)
	    {
		unsigned int offs = ((int)(u*(float)env_xres) + (int)(v*(float)env_yres)*env_xres)*3;
		if(offs >= env_yres*env_xres*3)
		    offs = 0;
		r = ((float*)envmap)[offs];
		g = ((float*)envmap)[offs+1];
		b = ((float*)envmap)[offs+2];
	    }
	    else
	    {
               unsigned int offs = (int)(u*(float)env_xres) + (int)(v*(float)env_yres)*env_xres;
	       if(offs >= env_yres*env_xres)
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
         sum[0] *= (float)(2.0/num_samples); // pre-divides by PI for final radiance/color lookup in shader
         sum[1] *= (float)(2.0/num_samples);
         sum[2] *= (float)(2.0/num_samples);
#else
         sum[0] *= (float)(1.0 / num_samples); // pre-divides by PI for final radiance/color lookup in shader
         sum[1] *= (float)(1.0 / num_samples);
         sum[2] *= (float)(1.0 / num_samples);
#endif
	 if(isHDR)
	 {
            const unsigned int offs = (y*rad_env_xres + x)*3;
            ((float*)rad_envmap)[offs  ] = sum[0];
	    ((float*)rad_envmap)[offs+1] = sum[1];
	    ((float*)rad_envmap)[offs+2] = sum[2];
	 }
	 else
	 {
            sum[0] = gammaApprox(sum[0]);
            sum[1] = gammaApprox(sum[1]);
            sum[2] = gammaApprox(sum[2]);
	    ((DWORD*)rad_envmap)[y*rad_env_xres + x] = ((int)(sum[0] * 255.0f)) | (((int)(sum[1] * 255.0f)) << 8) | (((int)(sum[2] * 255.0f)) << 16);
	 }
      }
}

HRESULT Pin3D::InitPin3D(const HWND hwnd, const bool fullScreen, const int width, const int height, const int colordepth, int &refreshrate, const int VSync, const bool useAA, const bool stereo3D, const unsigned int FXAA, const bool useAO)
{
   m_hwnd = hwnd;

   m_useAA = useAA;

   try {
      m_pd3dDevice = new RenderDevice(m_hwnd, width, height, fullScreen, colordepth, refreshrate, VSync, useAA, stereo3D, FXAA, g_pplayer->m_useNvidiaApi, g_pplayer->m_disableDWM);
   }
   catch (...) {
      return E_FAIL;
   }

   const int forceAniso = GetRegIntWithDefault("Player", "ForceAnisotropicFiltering", 1);
   m_pd3dDevice->ForceAnisotropicFiltering(!!forceAniso);

   const int compressTextures = GetRegIntWithDefault("Player", "CompressTextures", 0);
   m_pd3dDevice->CompressTextures(!!compressTextures);

   // set the viewport for the newly created device
   vp.X = 0;
   vp.Y = 0;
   vp.Width = width;
   vp.Height = height;
   vp.MinZ = 0.0f;
   vp.MaxZ = 1.0f;
   m_pd3dDevice->SetViewport(&vp);

   m_pd3dDevice->GetBackBufferTexture()->GetSurfaceLevel(0, &m_pddsBackBuffer);

   // Create the "static" color buffer.  
   // This will hold a pre-rendered image of the table and any non-changing elements (ie ramps, decals, etc).
   m_pddsStatic = m_pd3dDevice->DuplicateRenderTarget(m_pddsBackBuffer);
   if (!m_pddsStatic)
       return E_FAIL;

   m_pddsZBuffer = m_pd3dDevice->AttachZBufferTo(m_pddsBackBuffer);
   m_pddsStaticZ = m_pd3dDevice->AttachZBufferTo(m_pddsStatic);
   if (!m_pddsZBuffer || !m_pddsStaticZ)
	   return E_FAIL;

   pinballEnvTexture.CreateFromResource(IDB_BALL);

   //

   aoDitherTexture.CreateFromResource(IDB_AO_DITHER);

   m_envTexture = g_pplayer->m_ptable->GetImage(g_pplayer->m_ptable->m_szEnvImage);
   envTexture.CreateFromResource(IDB_ENV);

   Texture * const envTex = m_envTexture ? m_envTexture : &envTexture;

   const unsigned int envTexHeight = min(envTex->m_pdsBuffer->height(),256) / 8;
   const unsigned int envTexWidth = envTexHeight*2;
   
   m_envRadianceTexture = new BaseTexture(envTexWidth, envTexHeight, envTex->m_pdsBuffer->m_format);

   EnvmapPrecalc(envTex->m_pdsBuffer->data(), envTex->m_pdsBuffer->width(), envTex->m_pdsBuffer->height(),
                 m_envRadianceTexture->data(), envTexWidth, envTexHeight, envTex->IsHDR());

   m_pd3dDevice->m_texMan.SetDirty(m_envRadianceTexture);

   //

   if(m_pd3dDevice->DepthBufferReadBackAvailable() && (stereo3D || useAO)) {
      m_pdds3DZBuffer = !m_pd3dDevice->m_useNvidiaApi ? (D3DTexture*)m_pd3dDevice->AttachZBufferTo(m_pddsBackBuffer) : m_pd3dDevice->DuplicateDepthTexture((RenderTarget*)m_pddsZBuffer);

      if (!m_pdds3DZBuffer)
      {
         ShowError("Unable to create depth texture!\r\nTry to (un)set \"Alternative Depth Buffer processing\" in the video options!\r\nOr disable Ambient Occlusion and/or 3D stereo!");
         return E_FAIL;
      }
   }

   if(m_pd3dDevice->DepthBufferReadBackAvailable() && useAO) 
   {
       HRESULT hr;
       hr = m_pd3dDevice->GetCoreDevice()->CreateTexture(width, height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_L8, D3DPOOL_DEFAULT, &m_pddsAOBackTmpBuffer, NULL);
       if (FAILED(hr))
       {
           ShowError("Unable to create AO buffers!\r\nPlease disable Ambient Occlusion.\r\nOr try to (un)set \"Alternative Depth Buffer processing\" in the video options!");
           return E_FAIL;
       }
       hr = m_pd3dDevice->GetCoreDevice()->CreateTexture(width, height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_L8, D3DPOOL_DEFAULT, &m_pddsAOBackBuffer, NULL);
       if (FAILED(hr))
       {
           ShowError("Unable to create AO buffers!\r\nPlease disable Ambient Occlusion.\r\nOr try to (un)set \"Alternative Depth Buffer processing\" in the video options!");
           return E_FAIL;
       }
       if (!m_pddsAOBackBuffer || !m_pddsAOBackTmpBuffer)
          return E_FAIL;
   }

   InitRenderState();

   // Direct all renders to the "static" buffer.
   SetRenderTarget(m_pddsStatic, m_pddsStaticZ);

//#ifdef FPS
   //m_gpu_profiler.Init(m_pd3dDevice->GetCoreDevice()); // done by first BeginFrame() call lazily
//#endif
   return S_OK;
}


// Sets the texture filtering state.
void Pin3D::SetTextureFilter(const int TextureNum, const int Mode) const
{
   m_pd3dDevice->SetTextureFilter(TextureNum, Mode);
}

void Pin3D::SetRenderTarget(RenderTarget* pddsSurface, RenderTarget* pddsZ) const
{
   m_pd3dDevice->SetRenderTarget(pddsSurface);
   m_pd3dDevice->SetZBuffer(pddsZ);
}

void Pin3D::SetRenderTarget(RenderTarget* pddsSurface, void* pddsZ) const
{   
   if (!m_pd3dDevice->m_useNvidiaApi && m_pd3dDevice->m_INTZ_support)
      SetRenderTarget(pddsSurface, (D3DTexture*)pddsZ);
   else
      SetRenderTarget(pddsSurface, (RenderTarget*)pddsZ);
}

void Pin3D::SetRenderTarget(RenderTarget* pddsSurface, D3DTexture* pddsZ) const
{
   m_pd3dDevice->SetRenderTarget(pddsSurface);
   IDirect3DSurface9 *textureSurface;
   CHECKD3D(pddsZ->GetSurfaceLevel(0, &textureSurface));
   m_pd3dDevice->SetZBuffer(textureSurface);
   SAFE_RELEASE_NO_RCC(textureSurface);
}

void Pin3D::InitRenderState()
{
   g_pplayer->m_pin3d.DisableAlphaBlend();

   m_pd3dDevice->SetRenderState(RenderDevice::LIGHTING, FALSE);

   m_pd3dDevice->SetRenderState(RenderDevice::ZENABLE, TRUE);
   m_pd3dDevice->SetRenderState(RenderDevice::ZWRITEENABLE, TRUE);
   m_pd3dDevice->SetRenderState(RenderDevice::CULLMODE, D3DCULL_CCW);

   m_pd3dDevice->SetRenderState(RenderDevice::CLIPPING, FALSE);
   m_pd3dDevice->SetRenderState(RenderDevice::CLIPPLANEENABLE, 0);

   // initialize first texture stage
   m_pd3dDevice->SetTextureAddressMode(0, RenderDevice::TEX_CLAMP/*WRAP*/);
   m_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
   m_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
   SetTextureFilter(0, TEXTURE_MODE_TRILINEAR);
   m_pd3dDevice->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
   m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
   m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
   m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TFACTOR); // default tfactor: 1,1,1,1

   m_pd3dDevice->SetTextureAddressMode(4, RenderDevice::TEX_CLAMP/*WRAP*/); // normal maps
   SetTextureFilter(4, TEXTURE_MODE_TRILINEAR);
}

void Pin3D::DrawBackground()
{
   SetTextureFilter(0, TEXTURE_MODE_TRILINEAR);

   PinTable * const ptable = g_pplayer->m_ptable;
   Texture * const pin = ptable->GetDecalsEnabled()
      ? ptable->GetImage(ptable->m_BG_szImage[ptable->m_BG_current_set])
      : NULL;
   if (pin)
   {
      m_pd3dDevice->Clear(0, NULL, D3DCLEAR_ZBUFFER, 0, 1.0f, 0L);

      m_pd3dDevice->SetRenderState(RenderDevice::ZWRITEENABLE, FALSE);
      m_pd3dDevice->SetRenderState(RenderDevice::ZENABLE, FALSE);

      if (g_pplayer->m_ptable->m_tblMirrorEnabled^g_pplayer->m_ptable->m_fReflectionEnabled)
         m_pd3dDevice->SetRenderState(RenderDevice::CULLMODE, D3DCULL_NONE);

      g_pplayer->m_pin3d.DisableAlphaBlend();

      g_pplayer->Spritedraw(0.f, 0.f, 1.f, 1.f, 0xFFFFFFFF, pin, ptable->m_ImageBackdropNightDay ? sqrtf(g_pplayer->m_globalEmissionScale) : 1.0f);

      if (g_pplayer->m_ptable->m_tblMirrorEnabled^g_pplayer->m_ptable->m_fReflectionEnabled)
         m_pd3dDevice->SetRenderState(RenderDevice::CULLMODE, D3DCULL_CCW);

      m_pd3dDevice->SetRenderState(RenderDevice::ZENABLE, TRUE);
      m_pd3dDevice->SetRenderState(RenderDevice::ZWRITEENABLE, TRUE);
   }
   else
   {
      const D3DCOLOR d3dcolor = COLORREF_to_D3DCOLOR(ptable->m_colorbackdrop);
      m_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, d3dcolor, 1.0f, 0L);
   }
}

void Pin3D::InitLights()
{
   //m_pd3dDevice->basicShader->SetInt("iLightPointNum",MAX_LIGHT_SOURCES);
#ifdef SEPARATE_CLASSICLIGHTSHADER
   //m_pd3dDevice->classicLightShader->SetInt("iLightPointNum",MAX_LIGHT_SOURCES);
#endif

   g_pplayer->m_ptable->m_Light[0].pos.x = g_pplayer->m_ptable->m_right*0.5f;
   g_pplayer->m_ptable->m_Light[1].pos.x = g_pplayer->m_ptable->m_right*0.5f;
   g_pplayer->m_ptable->m_Light[0].pos.y = g_pplayer->m_ptable->m_bottom*(float)(1.0 / 3.0);
   g_pplayer->m_ptable->m_Light[1].pos.y = g_pplayer->m_ptable->m_bottom*(float)(2.0 / 3.0);
   g_pplayer->m_ptable->m_Light[0].pos.z = g_pplayer->m_ptable->m_lightHeight;
   g_pplayer->m_ptable->m_Light[1].pos.z = g_pplayer->m_ptable->m_lightHeight;

   D3DXVECTOR4 emission = convertColor(g_pplayer->m_ptable->m_Light[0].emission);
   emission.x *= g_pplayer->m_ptable->m_lightEmissionScale*g_pplayer->m_globalEmissionScale;
   emission.y *= g_pplayer->m_ptable->m_lightEmissionScale*g_pplayer->m_globalEmissionScale;
   emission.z *= g_pplayer->m_ptable->m_lightEmissionScale*g_pplayer->m_globalEmissionScale;

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
   m_pd3dDevice->basicShader->SetValue("packedLights", l, sizeof(CLight)*MAX_LIGHT_SOURCES);
#ifdef SEPARATE_CLASSICLIGHTSHADER
   m_pd3dDevice->classicLightShader->SetValue("packedLights", l, sizeof(CLight)*MAX_LIGHT_SOURCES);
#endif

   D3DXVECTOR4 amb_lr = convertColor(g_pplayer->m_ptable->m_lightAmbient, g_pplayer->m_ptable->m_lightRange);
   amb_lr.x *= g_pplayer->m_globalEmissionScale;
   amb_lr.y *= g_pplayer->m_globalEmissionScale;
   amb_lr.z *= g_pplayer->m_globalEmissionScale;
   m_pd3dDevice->basicShader->SetVector("cAmbient_LightRange", &amb_lr);
#ifdef SEPARATE_CLASSICLIGHTSHADER
   m_pd3dDevice->classicLightShader->SetVector("cAmbient_LightRange", &amb_lr);
#endif

}

// currently unused
//void LookAt( Matrix3D &mat, D3DVECTOR eye, D3DVECTOR target, D3DVECTOR up )
//{
//   D3DVECTOR zaxis = Normalize(eye - target);
//   D3DVECTOR xaxis = Normalize(CrossProduct(up,zaxis));
//   D3DVECTOR yaxis = CrossProduct(zaxis,xaxis);
//   mat._11 = xaxis.x; mat._12 = yaxis.x; mat._13 = zaxis.x; mat._14=0.0f;
//   mat._21 = xaxis.y; mat._22 = yaxis.y; mat._23 = zaxis.y; mat._24=0.0f;
//   mat._31 = xaxis.z; mat._32 = yaxis.z; mat._33 = zaxis.z; mat._34=0.0f;
//   mat._41 = 0.0f;    mat._42 = 0.0f;    mat._43 = zaxis.x; mat._44=0.0f;
//   Matrix3D trans;
//   trans.SetIdentity();
//   trans._41 = eye.x; trans._42 = eye.y; trans._43=eye.z;
//   mat.Multiply( trans, mat );
//}

Matrix3D ComputeLaybackTransform(float layback)
{
   // skew the coordinate system from kartesian to non kartesian.
   Matrix3D matTrans;
   matTrans.SetIdentity();
   matTrans._32 = -tanf(0.5f * ANGTORAD(layback));
   return matTrans;
}

void Pin3D::InitLayoutFS()
{
   TRACE_FUNCTION();

   const float rotation = ANGTORAD(g_pplayer->m_ptable->m_BG_rotation[g_pplayer->m_ptable->m_BG_current_set]);
   const float inclination = 0.0f;// ANGTORAD(g_pplayer->m_ptable->m_BG_inclination[g_pplayer->m_ptable->m_BG_current_set]);
   //const float FOV = (g_pplayer->m_ptable->m_BG_FOV[g_pplayer->m_ptable->m_BG_current_set] < 1.0f) ? 1.0f : g_pplayer->m_ptable->m_BG_FOV[g_pplayer->m_ptable->m_BG_current_set];

   std::vector<Vertex3Ds> vvertex3D;
   for (int i = 0; i < g_pplayer->m_ptable->m_vedit.Size(); ++i)
      g_pplayer->m_ptable->m_vedit.ElementAt(i)->GetBoundingVertices(vvertex3D);

   m_proj.m_rcviewport.left = 0;
   m_proj.m_rcviewport.top = 0;
   m_proj.m_rcviewport.right = vp.Width;
   m_proj.m_rcviewport.bottom = vp.Height;

   const float aspect = (float)vp.Width / (float)vp.Height; //(float)(4.0/3.0);

   //m_proj.FitCameraToVerticesFS(vvertex3D, aspect, rotation, inclination, FOV, g_pplayer->m_ptable->m_BG_xlatez[g_pplayer->m_ptable->m_BG_current_set], g_pplayer->m_ptable->m_BG_layback[g_pplayer->m_ptable->m_BG_current_set]);
   const float yof = g_pplayer->m_ptable->m_bottom*0.5f + g_pplayer->m_ptable->m_BG_xlatey[g_pplayer->m_ptable->m_BG_current_set];
   const float camx = 0.0f;
   const float camy = g_pplayer->m_ptable->m_bottom*0.5f + g_pplayer->m_ptable->m_BG_xlatex[g_pplayer->m_ptable->m_BG_current_set];
   const float camz = g_pplayer->m_ptable->m_bottom + g_pplayer->m_ptable->m_BG_xlatez[g_pplayer->m_ptable->m_BG_current_set];
   m_proj.m_matWorld.SetIdentity();
   D3DXVECTOR3 eye(camx, camy, camz);
   D3DXVECTOR3 at(0.0f, yof, 1.0f);
   const D3DXVECTOR3 up(0.0f, -1.0f, 0.0f);

   D3DXMATRIX rotationMat;
   D3DXMatrixRotationYawPitchRoll(&rotationMat, inclination, 0, rotation);
   D3DXVec3TransformCoord(&eye, &eye, &rotationMat);
   D3DXVec3TransformCoord(&at, &at, &rotationMat);
   //D3DXVec3TransformCoord(&up, &up, &rotationMat);
   //at=eye+at;

   D3DXMATRIX mView;
   D3DXMatrixLookAtLH(&mView, &eye, &at, &up);
   memcpy(m_proj.m_matView.m, mView.m, sizeof(float) * 4 * 4);
   m_proj.ScaleView(g_pplayer->m_ptable->m_BG_scalex[g_pplayer->m_ptable->m_BG_current_set], g_pplayer->m_ptable->m_BG_scaley[g_pplayer->m_ptable->m_BG_current_set], 1.0f);
   m_proj.RotateView(0, 0, rotation);
   m_proj.m_matWorld._41 = -g_pplayer->m_ptable->m_right*0.5f;//-m_proj.m_vertexcamera.x;
   m_proj.m_matWorld._42 = -g_pplayer->m_ptable->m_bottom*0.5f;//-m_proj.m_vertexcamera.y*1.0f;
   m_proj.m_matWorld._43 = -g_pplayer->m_ptable->m_glassheight;
   // recompute near and far plane (workaround for VP9 FitCameraToVertices bugs)
   m_proj.ComputeNearFarPlane(vvertex3D);
   D3DXMATRIX proj;
   //D3DXMatrixPerspectiveFovLH(&proj, ANGTORAD(FOV), aspect, m_proj.m_rznear, m_proj.m_rzfar);
   //D3DXMatrixPerspectiveFovLH(&proj, (float)(M_PI / 4.0), aspect, m_proj.m_rznear, m_proj.m_rzfar);

   D3DXMatrixIdentity(&proj);
   const float monitorPixel = 1.0f;// 25.4f * 23.3f / sqrt(1920.0f*1920.0f + 1080.0f*1080.0f);
   const float viewRight = monitorPixel*(float)vp.Width *0.5f;
   const float viewTop = monitorPixel*(float)vp.Height *0.5f;
   //eye.x = g_pplayer->m_ptable->m_bottom*0.4f;
   //eye.z += g_pplayer->m_ptable->m_glassheight;
   eye.z = g_pplayer->m_ptable->m_bottom;

   float right = viewRight - eye.x;
   float left = -viewRight - eye.x;
   float top = viewTop - eye.y;
   float bottom = -viewTop - eye.y;
   const float z_screen = eye.z >= m_proj.m_rznear ? eye.z : m_proj.m_rznear;

   // move edges of frustum from z_screen to z_near
   const float z_near_to_z_screen = m_proj.m_rznear / z_screen; // <=1.0
   right *= z_near_to_z_screen;
   left *= z_near_to_z_screen;
   top *= z_near_to_z_screen;
   bottom *= z_near_to_z_screen;

   D3DXMatrixPerspectiveOffCenterLH(&proj, left, right, bottom, top, m_proj.m_rznear, m_proj.m_rzfar);

   memcpy(m_proj.m_matProj.m, proj.m, sizeof(float) * 4 * 4);

   //m_proj.m_cameraLength = sqrtf(m_proj.m_vertexcamera.x*m_proj.m_vertexcamera.x + m_proj.m_vertexcamera.y*m_proj.m_vertexcamera.y + m_proj.m_vertexcamera.z*m_proj.m_vertexcamera.z);

   m_pd3dDevice->SetTransform(TRANSFORMSTATE_PROJECTION, &m_proj.m_matProj);
   m_pd3dDevice->SetTransform(TRANSFORMSTATE_VIEW, &m_proj.m_matView);
   m_pd3dDevice->SetTransform(TRANSFORMSTATE_WORLD, &m_proj.m_matWorld);

   m_proj.CacheTransform();

   // Compute view vector
   /*Matrix3D temp, viewRot;
   temp = m_proj.m_matView;
   temp.Invert();
   temp.GetRotationPart( viewRot );
   viewRot.MultiplyVector(Vertex3Ds(0, 0, 1), m_viewVec);
   m_viewVec.Normalize();*/

   InitLights();
}

// here is where the tables camera / rotation / scale is setup
// flashers are ignored in the calculation of boundaries to center the
// table in the view
void Pin3D::InitLayout(const bool FSS_mode)
{
   TRACE_FUNCTION();

   const float rotation = ANGTORAD(g_pplayer->m_ptable->m_BG_rotation[g_pplayer->m_ptable->m_BG_current_set]);
   float inclination = ANGTORAD(g_pplayer->m_ptable->m_BG_inclination[g_pplayer->m_ptable->m_BG_current_set]);
   const float FOV = (g_pplayer->m_ptable->m_BG_FOV[g_pplayer->m_ptable->m_BG_current_set] < 1.0f) ? 1.0f : g_pplayer->m_ptable->m_BG_FOV[g_pplayer->m_ptable->m_BG_current_set];

   std::vector<Vertex3Ds> vvertex3D;
   for (int i = 0; i < g_pplayer->m_ptable->m_vedit.Size(); ++i)
      g_pplayer->m_ptable->m_vedit.ElementAt(i)->GetBoundingVertices(vvertex3D);

   m_proj.m_rcviewport.left = 0;
   m_proj.m_rcviewport.top = 0;
   m_proj.m_rcviewport.right = vp.Width;
   m_proj.m_rcviewport.bottom = vp.Height;

   const float aspect = ((float)vp.Width) / ((float)vp.Height); //(float)(4.0/3.0);

   // next 4 def values for layout portrait(game vert) in landscape(screen horz)
   // for FSS, force an offset to camy which drops the table down 1/3 of the way.
   // some values to camy have been commented out because I found the default value 
   // better and just modify the camz and keep the table design inclination 
   // within 50-60 deg and 40-50 FOV in editor.
   // these values were tested against all known video modes upto 1920x1080 
   // in landscape and portrait on the display
   const float camx = m_camx;
   const float camy = m_camy + (FSS_mode ? 500.0f : 0.f);
         float camz = m_camz;
   const float inc  = m_inc  + (FSS_mode ? 0.2f : 0.f);

   if(FSS_mode)
   {
   //m_proj.m_rcviewport.right = vp.Height;
   //m_proj.m_rcviewport.bottom = vp.Width;
   const int width = GetSystemMetrics(SM_CXSCREEN);
   const int height = GetSystemMetrics(SM_CYSCREEN);

   // layout landscape(game horz) in lanscape(LCD\LED horz)
   if ((vp.Width > vp.Height) && (height < width))
   {
      //inc += 0.1f;       // 0.05-best, 0.1-good, 0.2-bad > (0.2 terrible original)
      //camy -= 30.0f;     // 70.0f original // 100
      if (aspect > 1.6f)
          camz -= 1170.0f; // 700
      else if (aspect > 1.5f)
          camz -= 1070.0f; // 650
      else if (aspect > 1.4f)
          camz -= 900.0f;  // 580
      else if (aspect > 1.3f)
          camz -= 820.0f;  // 500 // 600
      else
          camz -= 800.0f;  // 480
   }
   else {
      // layout potrait(game vert) in portrait(LCD\LED vert)
      if (height > width)
      {
         if (aspect > 0.6f) {
            camz += 10.0f;
            //camy += 50.0f;
         }
         else if (aspect > 0.5f) {
            camz += 300.0f;
            //camy += 100.0f;
         }
         else {
            camz += 300.0f;
            //camy += 200.0f;
         }
      }
      // layout landscape(game horz) in portrait(LCD\LED vert), who would but the UI allows for it!
      else {
      }
   }
   }

   inclination += inc; // added this to inclination in radians

   m_proj.FitCameraToVertices(vvertex3D, aspect, rotation, inclination, FOV, g_pplayer->m_ptable->m_BG_xlatez[g_pplayer->m_ptable->m_BG_current_set], g_pplayer->m_ptable->m_BG_layback[g_pplayer->m_ptable->m_BG_current_set]);

   m_proj.m_matWorld.SetIdentity();

   m_proj.m_matView.RotateXMatrix((float)M_PI);  // convert Z=out to Z=in (D3D coordinate system)
   m_proj.ScaleView(g_pplayer->m_ptable->m_BG_scalex[g_pplayer->m_ptable->m_BG_current_set], g_pplayer->m_ptable->m_BG_scaley[g_pplayer->m_ptable->m_BG_current_set], 1.0f);

   //!! FSS: added 500.0f to next line on camera y 
   //!! FSS: m_proj.m_vertexcamera.y += camy;
   //!! FSS: g_pplayer->m_ptable->m_BG_xlatey[g_pplayer->m_ptable->m_BG_current_set] += camy;
   //!! FSS: camy = 0.0f;

   m_proj.TranslateView(g_pplayer->m_ptable->m_BG_xlatex[g_pplayer->m_ptable->m_BG_current_set] - m_proj.m_vertexcamera.x + camx, g_pplayer->m_ptable->m_BG_xlatey[g_pplayer->m_ptable->m_BG_current_set] - m_proj.m_vertexcamera.y + camy, -m_proj.m_vertexcamera.z + camz);
   if (g_pplayer->cameraMode && (g_pplayer->m_ptable->m_BG_current_set == 0 || g_pplayer->m_ptable->m_BG_current_set == 2)) // DT & FSS
      m_proj.RotateView(inclination, 0, rotation);
   else
   {
      m_proj.RotateView(0, 0, rotation);
      m_proj.RotateView(inclination, 0, 0);
   }
   m_proj.MultiplyView(ComputeLaybackTransform(g_pplayer->m_ptable->m_BG_layback[g_pplayer->m_ptable->m_BG_current_set]));

   // recompute near and far plane (workaround for VP9 FitCameraToVertices bugs)
   m_proj.ComputeNearFarPlane(vvertex3D);
   D3DXMATRIX proj;
   D3DXMatrixPerspectiveFovLH(&proj, ANGTORAD(FOV), aspect, m_proj.m_rznear, m_proj.m_rzfar);
   memcpy(m_proj.m_matProj.m, proj.m, sizeof(float) * 4 * 4);

   //m_proj.m_cameraLength = sqrtf(m_proj.m_vertexcamera.x*m_proj.m_vertexcamera.x + m_proj.m_vertexcamera.y*m_proj.m_vertexcamera.y + m_proj.m_vertexcamera.z*m_proj.m_vertexcamera.z);

   m_pd3dDevice->SetTransform(TRANSFORMSTATE_PROJECTION, &m_proj.m_matProj);
   m_pd3dDevice->SetTransform(TRANSFORMSTATE_VIEW, &m_proj.m_matView);
   m_pd3dDevice->SetTransform(TRANSFORMSTATE_WORLD, &m_proj.m_matWorld);

   m_proj.CacheTransform();

   // Compute view vector
   /*Matrix3D temp, viewRot;
   temp = m_proj.m_matView;
   temp.Invert();
   temp.GetRotationPart( viewRot );
   viewRot.MultiplyVector(Vertex3Ds(0, 0, 1), m_viewVec);
   m_viewVec.Normalize();*/

   InitLights();
}

void Pin3D::InitPlayfieldGraphics()
{
   Vertex3D_NoTex2 rgv[7];
   rgv[0].x = g_pplayer->m_ptable->m_left;     rgv[0].y = g_pplayer->m_ptable->m_top;      rgv[0].z = g_pplayer->m_ptable->m_tableheight;
   rgv[1].x = g_pplayer->m_ptable->m_right;    rgv[1].y = g_pplayer->m_ptable->m_top;      rgv[1].z = g_pplayer->m_ptable->m_tableheight;
   rgv[2].x = g_pplayer->m_ptable->m_right;    rgv[2].y = g_pplayer->m_ptable->m_bottom;   rgv[2].z = g_pplayer->m_ptable->m_tableheight;
   rgv[3].x = g_pplayer->m_ptable->m_left;     rgv[3].y = g_pplayer->m_ptable->m_bottom;   rgv[3].z = g_pplayer->m_ptable->m_tableheight;

   // These next 4 vertices are used just to set the extents
   rgv[4].x = g_pplayer->m_ptable->m_left;     rgv[4].y = g_pplayer->m_ptable->m_top;      rgv[4].z = g_pplayer->m_ptable->m_tableheight + 50.0f;
   rgv[5].x = g_pplayer->m_ptable->m_left;     rgv[5].y = g_pplayer->m_ptable->m_bottom;   rgv[5].z = g_pplayer->m_ptable->m_tableheight + 50.0f;
   rgv[6].x = g_pplayer->m_ptable->m_right;    rgv[6].y = g_pplayer->m_ptable->m_bottom;   rgv[6].z = g_pplayer->m_ptable->m_tableheight + 50.0f;
   //rgv[7].x=g_pplayer->m_ptable->m_right;    rgv[7].y=g_pplayer->m_ptable->m_top;      rgv[7].z=50.0f;

   for (int i = 0; i < 4; ++i)
   {
      rgv[i].nx = 0;
      rgv[i].ny = 0;
      rgv[i].nz = 1.0f;

      rgv[i].tv = (i & 2) ? 1.0f : 0.f;
      rgv[i].tu = (i == 1 || i == 2) ? 1.0f : 0.f;
   }

   const WORD playfieldPolyIndices[10] = { 0, 1, 3, 0, 3, 2, 2, 3, 5, 6 };
   tableIBuffer = m_pd3dDevice->CreateAndFillIndexBuffer(10, playfieldPolyIndices);

   assert(tableVBuffer == NULL);
   m_pd3dDevice->CreateVertexBuffer(4 + 7, 0, MY_D3DFVF_NOTEX2_VERTEX, &tableVBuffer); //+7 verts for second rendering step

   Vertex3D_NoTex2 *buffer;
   tableVBuffer->lock(0, 0, (void**)&buffer, VertexBuffer::WRITEONLY);

   unsigned int offs = 0;
   for (unsigned int y = 0; y <= 1; ++y)
      for (unsigned int x = 0; x <= 1; ++x, ++offs)
      {
         buffer[offs].x = (x & 1) ? rgv[1].x : rgv[0].x;
         buffer[offs].y = (y & 1) ? rgv[2].y : rgv[0].y;
         buffer[offs].z = rgv[0].z;

         buffer[offs].tu = (x & 1) ? rgv[1].tu : rgv[0].tu;
         buffer[offs].tv = (y & 1) ? rgv[2].tv : rgv[0].tv;

         buffer[offs].nx = rgv[0].nx;
         buffer[offs].ny = rgv[0].ny;
         buffer[offs].nz = rgv[0].nz;
      }

   SetNormal(rgv, playfieldPolyIndices + 6, 4);

   memcpy(buffer + 4, rgv, 7 * sizeof(Vertex3D_NoTex2));

   tableVBuffer->unlock();
}

void Pin3D::RenderPlayfieldGraphics(const bool depth_only)
{
   TRACE_FUNCTION();

   const Material * const mat = g_pplayer->m_ptable->GetMaterial(g_pplayer->m_ptable->m_szPlayfieldMaterial);
   Texture * const pin = (depth_only && (!mat || !mat->m_bOpacityActive)) ? NULL : g_pplayer->m_ptable->GetImage((char *)g_pplayer->m_ptable->m_szImage);

   if(depth_only)
   {
       m_pd3dDevice->SetRenderState(RenderDevice::COLORWRITEENABLE, 0); //m_pin3d.m_pd3dDevice->SetRenderTarget(NULL); // disable color writes
	   // even with depth-only rendering we have to take care of alpha textures (stencil playfield to see underlying objects)
	   if (pin)
	   {
		   SetTextureFilter(0, TEXTURE_MODE_ANISOTROPIC);
		   m_pd3dDevice->basicShader->SetTechnique("basic_depth_only_with_texture");
		   m_pd3dDevice->basicShader->SetTexture("Texture0", pin);
		   m_pd3dDevice->basicShader->SetAlphaTestValue(pin->m_alphaTestValue * (float)(1.0 / 255.0));
	   }
	   else // No image by that name
		   m_pd3dDevice->basicShader->SetTechnique("basic_depth_only_without_texture");
   }
   else
   {
       m_pd3dDevice->basicShader->SetMaterial(mat);

       if (pin)
       {
           SetTextureFilter(0, TEXTURE_MODE_ANISOTROPIC);
           m_pd3dDevice->basicShader->SetTechnique(mat->m_bIsMetal ? "basic_with_texture_isMetal" : "basic_with_texture_isNotMetal");
           m_pd3dDevice->basicShader->SetTexture("Texture0", pin);
           m_pd3dDevice->basicShader->SetAlphaTestValue(pin->m_alphaTestValue * (float)(1.0 / 255.0));
       }
       else // No image by that name
           m_pd3dDevice->basicShader->SetTechnique(mat->m_bIsMetal ? "basic_without_texture_isMetal" : "basic_without_texture_isNotMetal");
   }

   assert(tableVBuffer != NULL);
   assert(tableIBuffer != NULL);
   m_pd3dDevice->basicShader->Begin(0);
   m_pd3dDevice->DrawIndexedPrimitiveVB(D3DPT_TRIANGLELIST, MY_D3DFVF_NOTEX2_VERTEX, tableVBuffer, 0, 4, tableIBuffer, 0, 6);
   m_pd3dDevice->basicShader->End();

   if(depth_only && pin)
   {
	   SetTextureFilter(0, TEXTURE_MODE_TRILINEAR);
	   m_pd3dDevice->basicShader->SetTechnique("basic_depth_only_without_texture");
   }
   else if(!depth_only && pin)
   {
      //m_pd3dDevice->basicShader->SetTexture("Texture0",(D3DTexture*)NULL);
      //m_pd3dDevice->m_texMan.UnloadTexture(pin->m_pdsBuffer); //!! is used by ball reflection later-on
      SetTextureFilter(0, TEXTURE_MODE_TRILINEAR);
      m_pd3dDevice->basicShader->SetTechnique(mat->m_bIsMetal ? "basic_without_texture_isMetal" : "basic_without_texture_isNotMetal");
   }

   m_pd3dDevice->basicShader->Begin(0);
   m_pd3dDevice->DrawIndexedPrimitiveVB(D3DPT_TRIANGLEFAN, MY_D3DFVF_NOTEX2_VERTEX, tableVBuffer, 4, 7, tableIBuffer, 6, 4);
   m_pd3dDevice->basicShader->End();

   if(depth_only)
       m_pd3dDevice->SetRenderState(RenderDevice::COLORWRITEENABLE, 0x0000000Fu); // reenable color writes with default value

   // Apparently, releasing the vertex buffer here immediately can cause rendering glitches in
   // later rendering steps, so we keep it around for now.
}

void Pin3D::EnableAlphaTestReference(const DWORD alphaRefValue) const
{
   m_pd3dDevice->SetRenderState(RenderDevice::ALPHAREF, alphaRefValue);
   m_pd3dDevice->SetRenderState(RenderDevice::ALPHATESTENABLE, TRUE);
   m_pd3dDevice->SetRenderState(RenderDevice::ALPHAFUNC, D3DCMP_GREATEREQUAL);
}

void Pin3D::EnableAlphaBlend(const bool additiveBlending, const bool set_dest_blend, const bool set_blend_op) const
{
   m_pd3dDevice->SetRenderState(RenderDevice::ALPHABLENDENABLE, TRUE);
   m_pd3dDevice->SetRenderState(RenderDevice::SRCBLEND, D3DBLEND_SRCALPHA);
   if (set_dest_blend)
      m_pd3dDevice->SetRenderState(RenderDevice::DESTBLEND, additiveBlending ? D3DBLEND_ONE : D3DBLEND_INVSRCALPHA);
   if (set_blend_op)
      m_pd3dDevice->SetRenderState(RenderDevice::BLENDOP, D3DBLENDOP_ADD);
}

void Pin3D::DisableAlphaBlend() const
{
   m_pd3dDevice->SetRenderState(RenderDevice::ALPHABLENDENABLE, FALSE);
}

void Pin3D::Flip(bool vsync)
{
   m_pd3dDevice->Flip(vsync);
}

Vertex3Ds Pin3D::Unproject(const Vertex3Ds& point)
{
   m_proj.CacheTransform(); // compute m_matrixTotal

   Matrix3D m2 = m_proj.m_matrixTotal; // = world * view * proj
   m2.Invert();
   Vertex3Ds p, p3;

   p.x = 2.0f * (point.x - (float)g_pplayer->m_pin3d.vp.X) / (float)g_pplayer->m_pin3d.vp.Width - 1.0f;
   p.y = 1.0f - 2.0f * (point.y - (float)g_pplayer->m_pin3d.vp.Y) / (float)g_pplayer->m_pin3d.vp.Height;
   p.z = (point.z - g_pplayer->m_pin3d.vp.MinZ) / (g_pplayer->m_pin3d.vp.MaxZ - g_pplayer->m_pin3d.vp.MinZ);
   p3 = m2.MultiplyVector(p);
   return p3;
}

Vertex3Ds Pin3D::Get3DPointFrom2D(const POINT& p)
{
   Vertex3Ds p1, p2, pNear, pFar;
   pNear.x = (float)p.x; pNear.y = (float)p.y; pNear.z = vp.MinZ;
   pFar.x = (float)p.x; pFar.y = (float)p.y; pFar.z = vp.MaxZ;
   p1 = Unproject(pNear);
   p2 = Unproject(pFar);
   float wz = g_pplayer->m_ptable->m_tableheight;
   float wx = ((wz - p1.z)*(p2.x - p1.x)) / (p2.z - p1.z) + p1.x;
   float wy = ((wz - p1.z)*(p2.y - p1.y)) / (p2.z - p1.z) + p1.y;
   Vertex3Ds vertex(wx, wy, wz);
   return vertex;
}

void PinProjection::RotateView(float x, float y, float z)
{
   Matrix3D matRotateX, matRotateY, matRotateZ;

   matRotateX.RotateXMatrix(x);
   m_matView.Multiply(matRotateX, m_matView);
   matRotateY.RotateYMatrix(y);
   m_matView.Multiply(matRotateY, m_matView);
   matRotateZ.RotateZMatrix(z);
   m_matView.Multiply(matRotateZ, m_matView);        // matView = rotZ * rotY * rotX * origMatView
}

void PinProjection::TranslateView(const float x, const float y, const float z)
{
   Matrix3D matTrans;
   matTrans.SetTranslation(x, y, z);
   m_matView.Multiply(matTrans, m_matView);
}

void PinProjection::ScaleView(const float x, const float y, const float z)
{
   m_matView.Scale(x, y, z);
}

void PinProjection::MultiplyView(const Matrix3D& mat)
{
   m_matView.Multiply(mat, m_matView);
}

void PinProjection::FitCameraToVerticesFS(std::vector<Vertex3Ds>& pvvertex3D, float aspect, float rotation, float inclination, float FOV, float xlatez, float layback)
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

   Matrix3D laybackTrans = ComputeLaybackTransform(layback);

   for (size_t i = 0; i < pvvertex3D.size(); ++i)
   {
      Vertex3Ds v = pvvertex3D[i];
      float temp;

      //v = laybackTrans.MultiplyVector(v);

      // Rotate vertex about x axis according to incoming inclination
      temp = v.y;
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
   m_vertexcamera.z = (float)(max(ydist, xdist)) + xlatez;
   m_vertexcamera.y = (float)((maxyintercept + minyintercept) * 0.5f);
   m_vertexcamera.x = (float)((maxxintercept + minxintercept) * 0.5f);
}

void PinProjection::FitCameraToVertices(std::vector<Vertex3Ds>& pvvertex3D, float aspect, float rotation, float inclination, float FOV, float xlatez, float layback)
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

   Matrix3D laybackTrans = ComputeLaybackTransform(layback);

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

void PinProjection::ComputeNearFarPlane(std::vector<Vertex3Ds>& verts)
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

   // beware the div-0 problem
   if (m_rznear < 0.001f)
      m_rznear = 0.001f;
   //m_rznear *= 0.89f; //!! magic, influences also stereo3D code
   m_rzfar *= 1.01f;
}

void PinProjection::CacheTransform()
{
   Matrix3D matT;
   m_matProj.Multiply(m_matView, matT);        // matT = matView * matProj
   matT.Multiply(m_matWorld, m_matrixTotal);   // total = matWorld * matView * matProj
}

// transforms the backdrop
void PinProjection::TransformVertices(const Vertex3Ds * const rgv, const WORD * const rgi, const int count, Vertex2D * const rgvout) const
{
   const float rClipWidth = (m_rcviewport.right - m_rcviewport.left)*0.5f;
   const float rClipHeight = (m_rcviewport.bottom - m_rcviewport.top)*0.5f;
   const int xoffset = m_rcviewport.left;
   const int yoffset = m_rcviewport.top;

   // Transform each vertex through the current matrix set
   for (int i = 0; i < count; ++i)
   {
      const int l = rgi ? rgi[i] : i;

      // Get the untransformed vertex position
      const float x = rgv[l].x;
      const float y = rgv[l].y;
      const float z = rgv[l].z;

      // Transform it through the current matrix set
      const float xp = m_matrixTotal._11*x + m_matrixTotal._21*y + m_matrixTotal._31*z + m_matrixTotal._41;
      const float yp = m_matrixTotal._12*x + m_matrixTotal._22*y + m_matrixTotal._32*z + m_matrixTotal._42;
      const float wp = m_matrixTotal._14*x + m_matrixTotal._24*y + m_matrixTotal._34*z + m_matrixTotal._44;

      // Finally, scale the vertices to screen coords. This step first
      // "flattens" the coordinates from 3D space to 2D device coordinates,
      // by dividing each coordinate by the wp value. Then, the x- and
      // y-components are transformed from device coords to screen coords.
      // Note 1: device coords range from -1 to +1 in the viewport.
      const float inv_wp = 1.0f / wp;
      const float vTx = (1.0f + xp*inv_wp) * rClipWidth + xoffset;
      const float vTy = (1.0f - yp*inv_wp) * rClipHeight + yoffset;

      rgvout[l].x = vTx;
      rgvout[l].y = vTy;
   }
}
