#include "stdafx.h"
#include "RenderProbe.h"

RenderProbe::RenderProbe()
{ 
}

RenderProbe::~RenderProbe()
{
   assert((m_staticRT == nullptr) && (m_dynamicRT == nullptr));
}

bool RenderProbe::LoadToken(const int id, BiffReader * const pbr)
{
   switch (id)
   {
   //case FID(PIID): pbr->GetInt((int *)pbr->m_pdata); break;
   }
   return true;
}

string RenderProbe::GetName() const
{
   return m_name;
}

void RenderProbe::SetName(const string name)
{
   m_name = name;
}

void RenderProbe::SetReflectionPlane(vec4& plane)
{
   m_reflection_plane = plane;
}

void RenderProbe::GetReflectionPlaneNormal(vec4& normal) const
{
   normal.x = m_reflection_plane.x;
   normal.y = m_reflection_plane.y;
   normal.z = m_reflection_plane.z;
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
}

void RenderProbe::MarkDirty()
{ 
   m_dirty = true;
}

Sampler *RenderProbe::GetProbe(const bool is_static)
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
   return rt ? rt->GetColorSampler() : nullptr;
}

void RenderProbe::RenderScreenSpaceTransparency(const bool is_static)
{
   RenderDevice* p3dDevice = g_pplayer->m_pin3d.m_pd3dPrimaryDevice;
   if (m_dynamicRT == nullptr)
      m_dynamicRT = p3dDevice->GetBackBufferTexture()->Duplicate();
   p3dDevice->GetMSAABackBufferTexture()->CopyTo(m_dynamicRT);
}

void RenderProbe::RenderReflectionProbe(const bool is_static)
{ 
   if ((is_static && (m_reflection_mode == REFL_NONE || m_reflection_mode == REFL_BALLS || m_reflection_mode == REFL_DYNAMIC))
      || (!is_static && (m_reflection_mode == REFL_NONE || m_reflection_mode == REFL_STATIC)))
         return;

   RenderTarget* previousRT = RenderTarget::GetCurrentRenderTarget();
   RenderDevice* p3dDevice = g_pplayer->m_pin3d.m_pd3dPrimaryDevice;
   RenderDevice::RenderStateCache initial_state; p3dDevice->CopyRenderStates(true, initial_state);

   // Prepare to render into the reflection back buffer
   if (is_static)
   {
      if (m_staticRT == nullptr)
      {
         //m_staticRT = p3dDevice->GetBackBufferTexture()->Duplicate();
         const int w = p3dDevice->GetBackBufferTexture()->GetWidth(), h = p3dDevice->GetBackBufferTexture()->GetHeight();
         m_staticRT = new RenderTarget(p3dDevice, w, h, p3dDevice->GetBackBufferTexture()->GetColorFormat(), true, 1, p3dDevice->GetBackBufferTexture()->GetStereo(), "Failed to create plane reflection static render target", nullptr);
      }
      m_staticRT->Activate();
      p3dDevice->Clear(clearType::TARGET | clearType::ZBUFFER, 0, 1.0f, 0L);
   }
   else
   {
      if (m_dynamicRT == nullptr)
      {
         // m_dynamicRT = p3dDevice->GetBackBufferTexture()->Duplicate();
         const int w = p3dDevice->GetBackBufferTexture()->GetWidth(), h = p3dDevice->GetBackBufferTexture()->GetHeight();
         m_dynamicRT = new RenderTarget(p3dDevice, w, h, p3dDevice->GetBackBufferTexture()->GetColorFormat(), true, 1, p3dDevice->GetBackBufferTexture()->GetStereo(), "Failed to create plane reflection dynamic render target", nullptr);
      }
      if (m_reflection_mode == REFL_SYNCED_DYNAMIC && m_staticRT != nullptr)
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

   const bool render_static = is_static || (m_reflection_mode == REFL_DYNAMIC);
   const bool render_balls = !is_static && (m_reflection_mode != REFL_NONE && m_reflection_mode != REFL_STATIC);
   const bool render_dynamic = !is_static && (m_reflection_mode >= REFL_UNSYNCED_DYNAMIC);

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
   previousRT->Activate();
}


