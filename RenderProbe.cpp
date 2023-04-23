#include "stdafx.h"
#include "RenderProbe.h"
#include "Shader.h"
#include "math/bluenoise.h"

const string PLAYFIELD_REFLECTION_RENDERPROBE_NAME = "Playfield Reflections"s;

RenderProbe::RenderProbe()
{ 
}

RenderProbe::~RenderProbe()
{
   assert(m_prerenderRT == nullptr && m_dynamicRT == nullptr); // EndPlay must be call before destructor
}

int RenderProbe::GetSaveSize() const
{
   int size = 0;
   size += 2 * sizeof(int) + sizeof(int); // TYPE
   size += 2 * sizeof(int) + sizeof(int) + (int)m_name.length(); // NAME
   size += 2 * sizeof(int) + sizeof(int); // RBAS
   size += 2 * sizeof(int) + sizeof(int); // RCLE
   size += 2 * sizeof(int) + sizeof(vec4); // RPLA
   size += 2 * sizeof(int) + sizeof(int); // RMOD
   return size;
}

HRESULT RenderProbe::SaveData(IStream* pstm, HCRYPTHASH hcrypthash, const bool saveForUndo)
{
   BiffWriter bw(pstm, hcrypthash);
   bw.WriteInt(FID(TYPE), m_type);
   bw.WriteString(FID(NAME), m_name);
   bw.WriteInt(FID(RBAS), m_roughness_base);
   bw.WriteInt(FID(RCLE), m_roughness_clear);
   bw.WriteStruct(FID(RPLA), (void*)&m_reflection_plane, sizeof(vec4));
   bw.WriteInt(FID(RMOD), m_reflection_mode);
   bw.WriteTag(FID(ENDB));
   return S_OK;
}

HRESULT RenderProbe::LoadData(IStream* pstm, PinTable* ppt, int version, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
{
   BiffReader br(pstm, this, ppt, version, hcrypthash, hcryptkey);
   br.Load();
   return S_OK;
}

bool RenderProbe::LoadToken(const int id, BiffReader* const pbr)
{
   switch (id)
   {
   case FID(TYPE): pbr->GetInt(&m_type); break;
   case FID(NAME): pbr->GetString(m_name); break;
   case FID(RBAS): pbr->GetInt(&m_roughness_base); break;
   case FID(RCLE): pbr->GetInt(&m_roughness_clear); break;
   case FID(RPLA): pbr->GetStruct(&m_reflection_plane, sizeof(vec4)); break;
   case FID(RMOD): pbr->GetInt(&m_reflection_mode); break;
   }
   return true;
}

string& RenderProbe::GetName()
{
   return m_name;
}

void RenderProbe::SetName(const string& name)
{
   m_name = name;
}

bool RenderProbe::IsRendering() const
{
   return m_rendering;
}

void RenderProbe::RenderSetup()
{
   MarkDirty();
}

void RenderProbe::EndPlay()
{ 
   delete m_prerenderRT;
   m_prerenderRT = nullptr;
   delete m_dynamicRT;
   m_dynamicRT = nullptr;
   delete m_blurRT;
   m_blurRT = nullptr;
}

void RenderProbe::MarkDirty()
{ 
   m_dirty = true;
}

void RenderProbe::PreRenderStatic()
{
   if (m_type == PLANE_REFLECTION)
   {
      PreRenderStaticReflectionProbe();
   }
}

RenderTarget *RenderProbe::GetProbe(const bool is_static)
{
   // Rendering is not reentrant. If a probe is requested while probe is being updated 
   // (for example and object with reflection, rendering itself in its reflection probe),
   // then the last render probe (may be null) will be returned
   if (m_dirty && !m_rendering && !g_pplayer->IsRenderPass(Player::REFLECTION_PASS))
   {
      m_rendering = true;
      switch (m_type)
      {
      case PLANE_REFLECTION: RenderReflectionProbe(is_static); break;
      case SCREEN_SPACE_TRANSPARENCY: RenderScreenSpaceTransparency(is_static); break;
      }
      m_rendering = false;
      m_dirty = false;
   }
   if (m_type == PLANE_REFLECTION)
   {
      ReflectionMode mode = min(m_reflection_mode, g_pplayer->m_pfReflectionMode);
      if (is_static)
      {
         if (mode == REFL_STATIC || mode == REFL_STATIC_N_BALLS || mode == REFL_STATIC_N_DYNAMIC)
            return m_dynamicRT;
      }
      else
      {
         if (mode != REFL_NONE && mode != REFL_STATIC)
            return m_dynamicRT;
      }
      return nullptr;
   }
   else // SCREEN_SPACE_TRANSPARENCY
   {
      return is_static ? nullptr : m_dynamicRT;
   }
}


//////////////////////////////////////////////////////////////////////////////////////////////////
// Roughness implementation, using downscaling and bluring

int RenderProbe::GetRoughnessDownscale(const int roughness)
{
   if (m_type == SCREEN_SPACE_TRANSPARENCY)
      return 1; // No downscaling since this is not supported by hardware for depth buffer
   return roughness < 7 ? 1 : 2;
}

void RenderProbe::ApplyRoughness(RenderTarget* probe, const int roughness)
{
   assert(0 <= roughness && roughness <= 12);
   if (roughness > 0)
   {
      RenderDevice* p3dDevice = g_pplayer->m_pin3d.m_pd3dPrimaryDevice;
      if (m_blurRT == nullptr)
         m_blurRT = probe->Duplicate("BlurProbe"s);
      // The kernel sizes were chosen by reverse engineering the blur shader. So there's a part of guess here.
      // The blur shader seems to mix binomial & gaussian distributions, with a kernel size which does not directly match the pascal triangle size.
      // Ideally this should be a gaussian distribution's sigma, scaled by the render height against a reference height.
      if (m_type == SCREEN_SPACE_TRANSPARENCY)
      {
         // FIXME adjust the kernels to have as many as there are roughness levels
         constexpr float kernel[] = { 0.f, 7.f, 9.f, 11.f, 13.f, 15.f, 19.f, 23.f, 27.f, 39.f, 39.f, 39.f, 39.f };
         p3dDevice->DrawGaussianBlur(probe, m_blurRT, probe, kernel[roughness]);
      }
      else
      {
         constexpr float kernel[] = { 0.f, 7.f, 9.f, 13.f, 15.f, 19.f, 23.f, 13.f, 15.f, 19.f, 23.f, 27.f, 39.f };
         p3dDevice->DrawGaussianBlur(probe, m_blurRT, probe, kernel[roughness]);
      }
   }
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Screen copy

void RenderProbe::RenderScreenSpaceTransparency(const bool is_static)
{
   RenderDevice* p3dDevice = g_pplayer->m_pin3d.m_pd3dPrimaryDevice;
   RenderTarget* previousRT = p3dDevice->GetCurrentRenderTarget();
   if (m_dynamicRT == nullptr)
   {
      const int downscale = GetRoughnessDownscale(m_roughness_base);
      const int w = p3dDevice->GetBackBufferTexture()->GetWidth() / downscale, h = p3dDevice->GetBackBufferTexture()->GetHeight() / downscale;
      m_dynamicRT = new RenderTarget(p3dDevice, "RefractionProbe"s,  w, h, p3dDevice->GetBackBufferTexture()->GetColorFormat(), true, 1, StereoMode::STEREO_OFF, "Failed to create refraction render target", nullptr);
   }
   p3dDevice->SetRenderTarget("Refraction"s, m_dynamicRT);
   p3dDevice->BlitRenderTarget(p3dDevice->GetMSAABackBufferTexture(), m_dynamicRT, true, true);
   ApplyRoughness(m_dynamicRT, m_roughness_base);
   p3dDevice->SetRenderTarget(""s, previousRT);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Reflection plane

void RenderProbe::GetReflectionPlane(vec4& plane) const
{
   plane.x = m_reflection_plane.x;
   plane.y = m_reflection_plane.y;
   plane.z = m_reflection_plane.z;
   plane.w = m_reflection_plane.w;
}

void RenderProbe::SetReflectionPlane(const vec4& plane) {
   m_reflection_plane = plane;
}

void RenderProbe::GetReflectionPlaneNormal(vec3& normal) const
{
   normal.x = m_reflection_plane.x;
   normal.y = m_reflection_plane.y;
   normal.z = m_reflection_plane.z;
}

void RenderProbe::SetReflectionMode(ReflectionMode mode) 
{
   assert(m_reflection_mode == mode || m_dynamicRT == nullptr); // Reflection mode may not be changed between RenderSetup/EndPlay
   m_reflection_mode = mode;
}

void RenderProbe::PreRenderStaticReflectionProbe()
{
   // For dynamic reflection mode, in static camera mode, we prerender static elements (like for main view) to get better antialiasing and overall performance
   if (g_pplayer->m_dynamicMode || min(m_reflection_mode, g_pplayer->m_pfReflectionMode) != REFL_DYNAMIC)
      return;

   RenderDevice* p3dDevice = g_pplayer->m_pin3d.m_pd3dPrimaryDevice;
   RenderTarget* previousRT = p3dDevice->GetCurrentRenderTarget();

   if (m_prerenderRT == nullptr)
   {
      const int downscale = GetRoughnessDownscale(m_roughness_base);
      const int w = p3dDevice->GetBackBufferTexture()->GetWidth() / downscale, h = p3dDevice->GetBackBufferTexture()->GetHeight() / downscale;
      m_prerenderRT = new RenderTarget(p3dDevice, "StaticReflProbe"s, w, h, p3dDevice->GetBackBufferTexture()->GetColorFormat(), true, 1, p3dDevice->GetBackBufferTexture()->GetStereo(),
         "Failed to create plane reflection static render target", nullptr);
   }

   RenderTarget* accumulationSurface = m_prerenderRT->Duplicate("Accumulation"s);

   // if rendering static/with heavy oversampling, disable the aniso/trilinear filter to get a sharper/more precise result overall!
   p3dDevice->SetMainTextureDefaultFiltering(SF_BILINEAR);

   //#define STATIC_PRERENDER_ITERATIONS_KOROBOV 7.0 // for the (commented out) lattice-based QMC oversampling, 'magic factor', depending on the the number of iterations!
   // loop for X times and accumulate/average these renderings
   // NOTE: iter == 0 MUST ALWAYS PRODUCE an offset of 0,0!
   unsigned int nTris = p3dDevice->m_curDrawnTriangles;
   int n_iter = STATIC_PRERENDER_ITERATIONS - 1;
   for (int iter = n_iter; iter >= 0; --iter) // just do one iteration if in dynamic camera/light/material tweaking mode
   {
      p3dDevice->m_curDrawnTriangles = 0;

      float u1 = xyLDBNbnot[iter * 2]; //      (float)iter*(float)(1.0                                /STATIC_PRERENDER_ITERATIONS);
      float u2 = xyLDBNbnot[iter * 2 + 1]; //fmodf((float)iter*(float)(STATIC_PRERENDER_ITERATIONS_KOROBOV/STATIC_PRERENDER_ITERATIONS), 1.f);
      // the following line implements filter importance sampling for a small gauss (i.e. less jaggies as it also samples neighboring pixels) -> but also potentially more artifacts in compositing!
      gaussianDistribution(u1, u2, 0.5f, 0.0f); //!! first 0.5 could be increased for more blur, but is pretty much what is recommended
      // sanity check to be sure to limit filter area to 3x3 in practice, as the gauss transformation is unbound (which is correct, but for our use-case/limited amount of samples very bad)
      assert(u1 > -1.5f && u1 < 1.5f);
      assert(u2 > -1.5f && u2 < 1.5f);
      // Last iteration MUST set a sample offset of 0,0 so that final depth buffer features 'correctly' centered pixel sample
      assert(iter != 0 || (u1 == 0.f && u2 == 0.f));

      // Setup Camera,etc matrices for each iteration, applying antialiasing offset
      g_pplayer->m_pin3d.InitLayout(g_pplayer->m_ptable->m_BG_enable_FSS, g_pplayer->m_ptable->GetMaxSeparation(), u1, u2);

      RenderState initial_state;
      p3dDevice->CopyRenderStates(true, initial_state);
      p3dDevice->SetRenderTarget("PreRender Reflection"s, m_prerenderRT);
      p3dDevice->AddRenderTargetDependency(accumulationSurface);
      p3dDevice->Clear(clearType::TARGET | clearType::ZBUFFER, 0, 1.0f, 0L);
      DoRenderReflectionProbe(true, false, false);
      p3dDevice->CopyRenderStates(false, initial_state);

      // Rendering is done to the static render target then accumulated to accumulationSurface
      // We use the framebuffer mirror shader which copies a weighted version of the bound texture
      p3dDevice->SetRenderTarget("PreRender Accumulate Reflection"s, accumulationSurface, true);
      p3dDevice->AddRenderTargetDependency(m_prerenderRT);
      p3dDevice->SetRenderState(RenderState::ALPHABLENDENABLE, RenderState::RS_TRUE);
      p3dDevice->SetRenderState(RenderState::SRCBLEND, RenderState::ONE);
      p3dDevice->SetRenderState(RenderState::DESTBLEND, RenderState::ONE);
      p3dDevice->SetRenderState(RenderState::BLENDOP, RenderState::BLENDOP_ADD);
      p3dDevice->SetRenderState(RenderState::ZENABLE, RenderState::RS_FALSE);
      p3dDevice->SetRenderState(RenderState::ZWRITEENABLE, RenderState::RS_FALSE);
      p3dDevice->SetRenderStateCulling(RenderState::CULL_NONE);
      if (iter == STATIC_PRERENDER_ITERATIONS - 1)
         p3dDevice->Clear(clearType::TARGET, 0, 1.0f, 0L);
      p3dDevice->FBShader->SetTechnique(SHADER_TECHNIQUE_fb_mirror);
      p3dDevice->FBShader->SetVector(
         SHADER_w_h_height, (float)(1.0 / (double)m_prerenderRT->GetWidth()), (float)(1.0 / (double)m_prerenderRT->GetHeight()), (float)((double)STATIC_PRERENDER_ITERATIONS), 1.0f);
      p3dDevice->FBShader->SetTexture(SHADER_tex_fb_unfiltered, m_prerenderRT->GetColorSampler());
      p3dDevice->DrawFullscreenTexturedQuad(p3dDevice->FBShader);
      p3dDevice->FBShader->SetTextureNull(SHADER_tex_fb_unfiltered);
      p3dDevice->CopyRenderStates(false, initial_state);
   }
   p3dDevice->m_curDrawnTriangles += nTris;

   // copy back weighted antialiased color result to the static render target, keeping depth untouched
   p3dDevice->SetRenderTarget("PreRender Store Reflection"s, m_prerenderRT);
   p3dDevice->BlitRenderTarget(accumulationSurface, m_prerenderRT, true, false);
   p3dDevice->FlushRenderFrame();
   delete accumulationSurface;
   p3dDevice->SetRenderTarget(""s, previousRT);

   // if rendering static/with heavy oversampling, re-enable the aniso/trilinear filter now for the normal rendering
   const bool forceAniso = LoadValueBoolWithDefault(regKey[RegName::Player], "ForceAnisotropicFiltering"s, true);
   p3dDevice->SetMainTextureDefaultFiltering(forceAniso ? SF_ANISOTROPIC : SF_TRILINEAR);
}

void RenderProbe::RenderReflectionProbe(const bool is_static)
{
   ReflectionMode mode = min(m_reflection_mode, g_pplayer->m_pfReflectionMode);
   if ((is_static && (mode == REFL_NONE || mode == REFL_BALLS || m_reflection_mode == REFL_DYNAMIC)) 
      || (!is_static && (mode == REFL_NONE || mode == REFL_STATIC)))
      return;

   // Rendering reflection is not reentrant and would fail (clip plane and view matrices are not cached)
   assert(!g_pplayer->IsRenderPass(Player::REFLECTION_PASS));

   RenderDevice* p3dDevice = g_pplayer->m_pin3d.m_pd3dPrimaryDevice;
   RenderTarget* previousRT = p3dDevice->GetCurrentRenderTarget();

   // Prepare to render into the reflection back buffer
   if (m_dynamicRT == nullptr)
   {
      const int downscale = GetRoughnessDownscale(m_roughness_base);
      const int w = p3dDevice->GetBackBufferTexture()->GetWidth() / downscale, h = p3dDevice->GetBackBufferTexture()->GetHeight() / downscale;
      m_dynamicRT = new RenderTarget(p3dDevice, "DynamicReflProbe"s, w, h, p3dDevice->GetBackBufferTexture()->GetColorFormat(), true, 1, p3dDevice->GetBackBufferTexture()->GetStereo(),
         "Failed to create plane reflection dynamic render target", nullptr);
   }
   p3dDevice->SetRenderTarget("Reflection"s, m_dynamicRT);
   if (mode == REFL_DYNAMIC && m_prerenderRT != nullptr && !g_pplayer->m_dynamicMode)
      p3dDevice->BlitRenderTarget(m_prerenderRT, m_dynamicRT, true, true);
   else
      p3dDevice->Clear(clearType::TARGET | clearType::ZBUFFER, 0, 1.0f, 0L);

   const bool render_static = is_static || (mode == REFL_DYNAMIC && (m_prerenderRT == nullptr || g_pplayer->m_dynamicMode));
   const bool render_balls = !is_static && (mode != REFL_NONE && mode != REFL_STATIC);
   const bool render_dynamic = !is_static && (mode >= REFL_STATIC_N_DYNAMIC);
   DoRenderReflectionProbe(render_static, render_balls, render_dynamic);

   ApplyRoughness(p3dDevice->GetCurrentRenderTarget(), m_roughness_base);

   p3dDevice->SetRenderTarget(""s, previousRT);
}

void RenderProbe::DoRenderReflectionProbe(const bool render_static, const bool render_balls, const bool render_dynamic)
{
   RenderDevice* p3dDevice = g_pplayer->m_pin3d.m_pd3dPrimaryDevice;
   RenderState initial_state;
   p3dDevice->CopyRenderStates(true, initial_state);

   g_pplayer->m_render_mask |= Player::REFLECTION_PASS;

   // Set the clip plane to only render objects above the reflection plane (do not reflect what is under or the plane itself)
   vec4 clip_plane = vec4(-m_reflection_plane.x, -m_reflection_plane.y, -m_reflection_plane.z, m_reflection_plane.w);
   p3dDevice->SetClipPlane(clip_plane);
   p3dDevice->SetRenderState(RenderState::CLIPPLANEENABLE, RenderState::RS_TRUE);

   p3dDevice->SetRenderStateCulling(RenderState::CULL_CCW); // re-init/thrash cache entry due to the hacky nature of the table mirroring

   // Flip camera
   Matrix3D viewMat, initialViewMat;
   p3dDevice->GetTransform(RenderDevice::TRANSFORMSTATE_VIEW, &viewMat, 1);
   memcpy(initialViewMat.m, viewMat.m, 4 * 4 * sizeof(float));
   // Reflect against reflection plane given by its normal (formula from https://en.wikipedia.org/wiki/Transformation_matrix#Reflection_2)
   Matrix3D reflect;
   reflect._11 = 1.0f - 2.0f * m_reflection_plane.x * m_reflection_plane.x;
   reflect._12 = -2.0f * m_reflection_plane.x * m_reflection_plane.y;
   reflect._13 = -2.0f * m_reflection_plane.x * m_reflection_plane.z;
   reflect._14 = -2.0f * m_reflection_plane.x * m_reflection_plane.w;
   reflect._21 = -2.0f * m_reflection_plane.y * m_reflection_plane.x;
   reflect._22 = 1.0f - 2.0f * m_reflection_plane.y * m_reflection_plane.y;
   reflect._23 = -2.0f * m_reflection_plane.y * m_reflection_plane.z;
   reflect._24 = -2.0f * m_reflection_plane.y * m_reflection_plane.w;
   reflect._31 = -2.0f * m_reflection_plane.z * m_reflection_plane.x;
   reflect._32 = -2.0f * m_reflection_plane.z * m_reflection_plane.y;
   reflect._33 = 1.0f - 2.0f * m_reflection_plane.z * m_reflection_plane.z;
   reflect._34 = -2.0f * m_reflection_plane.z * m_reflection_plane.w;
   reflect._41 = 0.0f;
   reflect._42 = 0.0f;
   reflect._43 = 0.0f;
   reflect._44 = 1.0f;
   viewMat = reflect * viewMat;
   // Translate the camera on the other side of the plane (move by twice the distance along its normal)
   // reflect.SetTranslation(-m_reflection_plane.w * m_reflection_plane.x * 2.0f, -m_reflection_plane.w * m_reflection_plane.y * 2.0f, -m_reflection_plane.w * m_reflection_plane.z * 2.0f);
   //viewMat = reflect * viewMat;
   p3dDevice->SetTransform(RenderDevice::TRANSFORMSTATE_VIEW, &viewMat);

   if (render_static || render_dynamic)
      g_pplayer->UpdateBasicShaderMatrix();
   if (render_balls)
      g_pplayer->UpdateBallShaderMatrix();

   if (render_static)
      g_pplayer->DrawStatics();
   if (render_dynamic)
      g_pplayer->DrawDynamics(false);
   else if (render_balls)
      g_pplayer->DrawDynamics(true);

   // Restore initial render states and camera
   g_pplayer->m_render_mask &= ~Player::REFLECTION_PASS;
   p3dDevice->CopyRenderStates(false, initial_state);
   p3dDevice->SetTransform(RenderDevice::TRANSFORMSTATE_VIEW, &initialViewMat);
   if (render_static || render_dynamic)
      g_pplayer->UpdateBasicShaderMatrix();
   if (render_balls)
      g_pplayer->UpdateBallShaderMatrix();
}