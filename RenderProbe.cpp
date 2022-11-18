#include "stdafx.h"
#include "RenderProbe.h"

const string PLAYFIELD_REFLECTION_RENDERPROBE_NAME = "Playfield Reflections"s;

RenderProbe::RenderProbe()
{ 
}

RenderProbe::~RenderProbe()
{
   assert((m_staticRT == nullptr) && (m_dynamicRT == nullptr));
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

HRESULT RenderProbe::SaveData(IStream* pstm, HCRYPTHASH hcrypthash, const bool backupForPlay)
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
   delete m_staticRT;
   m_staticRT = nullptr;
   delete m_dynamicRT;
   m_dynamicRT = nullptr;
   delete m_blurRT;
   m_blurRT = nullptr;
}

void RenderProbe::MarkDirty()
{ 
   m_dirty = true;
}

RenderTarget *RenderProbe::GetProbe(const bool is_static)
{
   if (m_dirty)
   {
      assert(!m_rendering); // Rendering is not reentrant (this could be allowed but has little benefit)
      m_rendering = true;
      switch (m_type)
      {
      case PLANE_REFLECTION: RenderReflectionProbe(is_static); break;
      case SCREEN_SPACE_TRANSPARENCY: RenderScreenSpaceTransparency(is_static); break;
      }
      m_rendering = false;
      m_dirty = false;
   }
   RenderTarget* rt = is_static ? m_staticRT : m_dynamicRT;
   return rt;
}


//////////////////////////////////////////////////////////////////////////////////////////////////
// Roughness implementation, using downscaling and bluring

int RenderProbe::GetRoughnessDownscale(const int roughness)
{
   return roughness < 7 ? 1 : 2;
}

void RenderProbe::ApplyRoughness(RenderTarget* probe, const int roughness)
{
   assert(0 <= roughness && roughness <= 12);
   if (roughness > 0)
   {
      RenderDevice* p3dDevice = g_pplayer->m_pin3d.m_pd3dPrimaryDevice;
      if (m_blurRT == nullptr)
         m_blurRT = probe->Duplicate();
      // The kernel sizes were chosen by reverse engiennering the blur shader. So there's a part of guess here.
      // The blur shader seems to mix binomial & gaussian distributions, with a kernel size which does not directly matches the pascal triangle size.
      // Ideally this should be a gaussian distribution's sigma, scaled by the render height against a reference height.
      const float kernel[] = { 0.f, 7.f, 9.f, 13.f, 15.f, 19.f, 23.f, 13.f, 15.f, 19.f, 23.f, 27.f, 39.f};
      p3dDevice->DrawGaussianBlur(probe->GetColorSampler(), m_blurRT, probe, kernel[roughness]);
   }
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Screen copy

void RenderProbe::RenderScreenSpaceTransparency(const bool is_static)
{
   RenderDevice* p3dDevice = g_pplayer->m_pin3d.m_pd3dPrimaryDevice;
   if (m_dynamicRT == nullptr)
   {
      const int downscale = GetRoughnessDownscale(m_roughness_base);
      const int w = p3dDevice->GetBackBufferTexture()->GetWidth() / downscale, h = p3dDevice->GetBackBufferTexture()->GetHeight() / downscale;
      m_dynamicRT = new RenderTarget(p3dDevice, w, h, p3dDevice->GetBackBufferTexture()->GetColorFormat(), true, 1, StereoMode::STEREO_OFF, "Failed to create refraction render target", nullptr);
   }
   p3dDevice->GetMSAABackBufferTexture()->CopyTo(m_dynamicRT);
   ApplyRoughness(m_dynamicRT, m_roughness_base);
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
   assert(m_reflection_mode == mode || (m_staticRT == nullptr && m_dynamicRT == nullptr)); // Reflection mode may not be changed between RenderSetup/EndPlay
   m_reflection_mode = mode;
}

void RenderProbe::RenderReflectionProbe(const bool is_static)
{
   ReflectionMode mode = min(m_reflection_mode, g_pplayer->m_pfReflectionMode);
   if ((is_static && (mode == REFL_NONE || mode == REFL_BALLS || m_reflection_mode == REFL_DYNAMIC)) || (!is_static && (mode == REFL_NONE || mode == REFL_STATIC)))
         return;

   RenderTarget* previousRT = RenderTarget::GetCurrentRenderTarget();
   RenderDevice* p3dDevice = g_pplayer->m_pin3d.m_pd3dPrimaryDevice;
   RenderDevice::RenderStateCache initial_state; p3dDevice->CopyRenderStates(true, initial_state);

   // Prepare to render into the reflection back buffer
   if (is_static)
   {
      if (m_staticRT == nullptr)
      {
         const int downscale = GetRoughnessDownscale(m_roughness_base);
         const int w = p3dDevice->GetBackBufferTexture()->GetWidth() / downscale, h = p3dDevice->GetBackBufferTexture()->GetHeight() / downscale;
         m_staticRT = new RenderTarget(p3dDevice, w, h, p3dDevice->GetBackBufferTexture()->GetColorFormat(), true, 1, p3dDevice->GetBackBufferTexture()->GetStereo(), "Failed to create plane reflection static render target", nullptr);
      }
      m_staticRT->Activate();
      p3dDevice->Clear(clearType::TARGET | clearType::ZBUFFER, 0, 1.0f, 0L);
   }
   else
   {
      if (m_dynamicRT == nullptr)
      {
         const int downscale = GetRoughnessDownscale(m_roughness_base);
         const int w = p3dDevice->GetBackBufferTexture()->GetWidth() / downscale, h = p3dDevice->GetBackBufferTexture()->GetHeight() / downscale;
         m_dynamicRT = new RenderTarget(p3dDevice, w, h, p3dDevice->GetBackBufferTexture()->GetColorFormat(), true, 1, p3dDevice->GetBackBufferTexture()->GetStereo(), "Failed to create plane reflection dynamic render target", nullptr);
      }
      if (mode == REFL_SYNCED_DYNAMIC && m_staticRT != nullptr)
      {
         // Intialize dynamic depth buffer from static one to avoid incorrect overlaps of staticly rendered parts by dynamic ones (this does not prevent overlaps the other way around though)
         m_staticRT->CopyTo(m_dynamicRT, false, true);
         m_dynamicRT->Activate();
         p3dDevice->Clear(clearType::TARGET, 0, 1.0f, 0L);
      }
      else
      {
         m_dynamicRT->Activate();
         p3dDevice->Clear(clearType::TARGET | clearType::ZBUFFER, 0, 1.0f, 0L);
      }
   }

   // Set the clip plane to only render objects above the reflection plane (do not reflect what is under or the plane itself)
   vec4 clip_plane = vec4(-m_reflection_plane.x, -m_reflection_plane.y, -m_reflection_plane.z, m_reflection_plane.w);
   p3dDevice->SetClipPlane0(clip_plane);
   p3dDevice->SetRenderStateClipPlane0(true);

   g_pplayer->m_ptable->m_reflectionEnabled = true; // set to let matrices and postrenderstatics know that we need to handle reflections now
   p3dDevice->SetRenderStateCulling(RenderDevice::CULL_CCW); // re-init/thrash cache entry due to the hacky nature of the table mirroring

   // Flip camera
   Matrix3D viewMat, initialViewMat;
   p3dDevice->GetTransform(TRANSFORMSTATE_VIEW, &viewMat, 1);
   memcpy(initialViewMat.m, viewMat.m, 4 * 4 * sizeof(float));
   // Reflect against reflection plane given by its normal (formula from https://en.wikipedia.org/wiki/Transformation_matrix#Reflection_2)
   Matrix3D reflect;
   reflect.SetIdentity();
   reflect._11 = 1.0f - 2.0f * m_reflection_plane.x * m_reflection_plane.x;
   reflect._12 = -2.0f * m_reflection_plane.x * m_reflection_plane.y;
   reflect._13 = -2.0f * m_reflection_plane.x * m_reflection_plane.z;
   reflect._21 = -2.0f * m_reflection_plane.x * m_reflection_plane.y;
   reflect._22 = 1.0f - 2.0f * m_reflection_plane.y * m_reflection_plane.y;
   reflect._23 = -2.0f * m_reflection_plane.y * m_reflection_plane.z;
   reflect._31 = -2.0f * m_reflection_plane.x * m_reflection_plane.z;
   reflect._32 = -2.0f * m_reflection_plane.y * m_reflection_plane.z;
   reflect._33 = 1.0f - 2.0f * m_reflection_plane.z * m_reflection_plane.z;
   reflect._33 = -1.0f;
   viewMat = reflect * viewMat;
   // Translate the camera on the other side of the plane (move by twice the distance along its normal)
   reflect.SetTranslation(-m_reflection_plane.w * m_reflection_plane.x * 2.0f, -m_reflection_plane.w * m_reflection_plane.y * 2.0f, -m_reflection_plane.w * m_reflection_plane.z * 2.0f);
   viewMat = reflect * viewMat;
   p3dDevice->SetTransform(TRANSFORMSTATE_VIEW, &viewMat);

   const bool render_static = is_static || (mode == REFL_DYNAMIC);
   const bool render_balls = !is_static && (mode != REFL_NONE && mode != REFL_STATIC);
   const bool render_dynamic = !is_static && (mode >= REFL_UNSYNCED_DYNAMIC);

   if (render_static || render_dynamic)
      g_pplayer->UpdateBasicShaderMatrix();
   if (render_balls)
      g_pplayer->UpdateBallShaderMatrix();

   if (render_static)
      g_pplayer->DrawStatics();
   
   if (render_balls)
      g_pplayer->DrawBalls();

   if (render_dynamic)
   {
      g_pplayer->SetViewVector(Vertex3Ds(0.f, 0.f, 1.f));
      g_pplayer->DrawDynamics();
      g_pplayer->SetViewVector(Vertex3Ds(0.f, 0.f, -1.f));
   }
   
   // Restore initial render states and camera
   g_pplayer->m_ptable->m_reflectionEnabled = false;
   p3dDevice->CopyRenderStates(false, initial_state);
   p3dDevice->SetTransform(TRANSFORMSTATE_VIEW, &initialViewMat);
   if (render_static || render_dynamic)
      g_pplayer->UpdateBasicShaderMatrix();
   if (render_balls)
      g_pplayer->UpdateBallShaderMatrix();

   ApplyRoughness(RenderTarget::GetCurrentRenderTarget(), m_roughness_base);

   previousRT->Activate();
}
