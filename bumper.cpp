#include "stdafx.h"
#include "objloader.h"
#include "meshes/bumperBaseMesh.h"
#include "meshes/bumperRingMesh.h"
#include "meshes/bumperCapMesh.h"
#include "meshes/bumperSocketMesh.h"
#include "Shader.h"
#include "IndexBuffer.h"
#include "VertexBuffer.h"

Bumper::Bumper()
{
   m_pbumperhitcircle = nullptr;
   m_baseVertexBuffer = nullptr;
   m_baseIndexBuffer = nullptr;
   m_ringVertexBuffer = nullptr;
   m_ringIndexBuffer = nullptr;
   m_capVertexBuffer = nullptr;
   m_capIndexBuffer = nullptr;
   m_socketIndexBuffer = nullptr;
   m_socketVertexBuffer = nullptr;
   m_ringAnimate = false;
   m_propVisual = nullptr;
   m_d.m_ringDropOffset = 0.0f;
   m_ringDown = false;
   m_updateSkirt = false;
   m_doSkirtAnimation = false;
   m_enableSkirtAnimation = true;
   m_skirtCounter = 0.0f;
}

Bumper::~Bumper()
{
   EndPlay();
}

HRESULT Bumper::Init(PinTable *ptable, float x, float y, bool fromMouseClick)
{
   m_ptable = ptable;

   SetDefaults(fromMouseClick);

   m_d.m_vCenter.x = x;
   m_d.m_vCenter.y = y;

   return InitVBA(fTrue, 0, nullptr);
}

void Bumper::SetDefaults(bool fromMouseClick)
{
#define regKey regKey[RegName::DefaultPropsBumper]

   m_d.m_radius = fromMouseClick ? LoadValueFloatWithDefault(regKey, "Radius"s, 45.f) : 45.f;

   SetDefaultPhysics(fromMouseClick);

   m_d.m_heightScale = fromMouseClick ? LoadValueFloatWithDefault(regKey, "HeightScale"s, 90.0f) : 90.0f;
   m_d.m_ringSpeed = fromMouseClick ? LoadValueFloatWithDefault(regKey, "RingSpeed"s, 0.5f) : 0.5f;
   m_d.m_orientation = fromMouseClick ? LoadValueFloatWithDefault(regKey, "Orientation"s, 0.0f) : 0.0f;
   m_d.m_threshold = fromMouseClick ? LoadValueFloatWithDefault(regKey, "Threshold"s, 1.f) : 1.f;

   const HRESULT hr = LoadValue(regKey, "Surface"s, m_d.m_szSurface);
   if (hr != S_OK || !fromMouseClick)
      m_d.m_szSurface.clear();

   m_d.m_tdr.m_TimerEnabled = fromMouseClick ? LoadValueBoolWithDefault(regKey, "TimerEnabled"s, false) : false;
   m_d.m_tdr.m_TimerInterval = fromMouseClick ? LoadValueIntWithDefault(regKey, "TimerInterval"s, 100) : 100;
   m_d.m_capVisible = fromMouseClick ? LoadValueBoolWithDefault(regKey, "CapVisible"s, true) : true;
   m_d.m_baseVisible = fromMouseClick ? LoadValueBoolWithDefault(regKey, "BaseVisible"s, true) : true;
   m_d.m_ringVisible = fromMouseClick ? LoadValueBoolWithDefault(regKey, "RingVisible"s, true) : true;
   m_d.m_skirtVisible = fromMouseClick ? LoadValueBoolWithDefault(regKey, "SkirtVisible"s, true) : true;
   m_d.m_reflectionEnabled = fromMouseClick ? LoadValueBoolWithDefault(regKey, "ReflectionEnabled"s, true) : true;
   m_d.m_hitEvent = fromMouseClick ? LoadValueBoolWithDefault(regKey, "HasHitEvent"s, true) : true;
   m_d.m_collidable = fromMouseClick ? LoadValueBoolWithDefault(regKey, "Collidable"s, true) : true;

   m_ringAnimate = false;
   m_d.m_ringDropOffset = 0.0f;

#undef regKey
}

void Bumper::WriteRegDefaults()
{
#define regKey regKey[RegName::DefaultPropsBumper]

   SaveValueFloat(regKey, "Radius"s, m_d.m_radius);
   SaveValueFloat(regKey, "Force"s, m_d.m_force);
   SaveValueFloat(regKey, "Scatter"s, m_d.m_scatter);
   SaveValueFloat(regKey, "HeightScale"s, m_d.m_heightScale);
   SaveValueFloat(regKey, "RingSpeed"s, m_d.m_ringSpeed);
   SaveValueFloat(regKey, "Orientation"s, m_d.m_orientation);
   SaveValueFloat(regKey, "Threshold"s, m_d.m_threshold);
   SaveValueBool(regKey, "TimerEnabled"s, m_d.m_tdr.m_TimerEnabled);
   SaveValueInt(regKey, "TimerInterval"s, m_d.m_tdr.m_TimerInterval);
   SaveValueBool(regKey, "CapVisible"s, m_d.m_capVisible);
   SaveValueBool(regKey, "BaseVisible"s, m_d.m_baseVisible);
   SaveValueBool(regKey, "HasHitEvent"s, m_d.m_hitEvent);
   SaveValueBool(regKey, "Collidable"s, m_d.m_collidable);
   SaveValueBool(regKey, "ReflectionEnabled"s, m_d.m_reflectionEnabled);
   SaveValue(regKey, "Surface"s, m_d.m_szSurface);

#undef regKey
}

STDMETHODIMP Bumper::InterfaceSupportsErrorInfo(REFIID riid)
{
   static const IID* arr[] =
   {
      &IID_IBumper,
   };

   for (size_t i = 0; i < sizeof(arr) / sizeof(arr[0]); i++)
   {
      if (InlineIsEqualGUID(*arr[i], riid))
         return S_OK;
   }
   return S_FALSE;
}

void Bumper::UIRenderPass1(Sur * const psur)
{
   psur->SetBorderColor(-1, false, 0);

   psur->SetObject(this);
   const float radangle = ANGTORAD(m_d.m_orientation);
   const float sn = sinf(radangle);
   const float cs = cosf(radangle);

   const float x1 = m_d.m_vCenter.x - cs*(m_d.m_radius + 10.f);
   const float y1 = m_d.m_vCenter.y - sn*(m_d.m_radius + 10.f);
   const float x2 = m_d.m_vCenter.x + cs*(m_d.m_radius + 10.f);
   const float y2 = m_d.m_vCenter.y + sn*(m_d.m_radius + 10.f);
   psur->Ellipse(x1, y1, 10.0f);
   psur->Ellipse(x2, y2, 10.0f);

   if (m_ptable->m_renderSolid)
   {
      const Material * const mat = m_ptable->GetMaterial(m_d.m_szCapMaterial);
      psur->SetFillColor(mat->m_cBase);
   }
   else
      psur->SetFillColor(-1);

   psur->Ellipse(m_d.m_vCenter.x, m_d.m_vCenter.y, m_d.m_radius*1.5f);
   if (m_ptable->m_renderSolid)
   {
      const Material * const mat = m_ptable->GetMaterial(m_d.m_szBaseMaterial);
      psur->SetFillColor(mat->m_cBase);
   }
   else
      psur->SetFillColor(-1);

   psur->Ellipse(m_d.m_vCenter.x, m_d.m_vCenter.y, m_d.m_radius);
}

void Bumper::UIRenderPass2(Sur * const psur)
{
   psur->SetBorderColor(RGB(0, 0, 0), false, 0);
   psur->SetFillColor(-1);
   psur->SetObject(this);
   psur->SetObject(nullptr);
   const float radangle = ANGTORAD(m_d.m_orientation - 90.f);
   const float sn = sinf(radangle);
   const float cs = cosf(radangle);

   const float x1 = m_d.m_vCenter.x - cs*(m_d.m_radius + 10.f);
   const float y1 = m_d.m_vCenter.y - sn*(m_d.m_radius + 10.f);
   const float x2 = m_d.m_vCenter.x + cs*(m_d.m_radius + 10.f);
   const float y2 = m_d.m_vCenter.y + sn*(m_d.m_radius + 10.f);
   psur->Ellipse(x1, y1, 10.0f);
   psur->Ellipse(x2, y2, 10.0f);
   psur->Ellipse(m_d.m_vCenter.x, m_d.m_vCenter.y, m_d.m_radius*1.5f);
   psur->Ellipse(m_d.m_vCenter.x, m_d.m_vCenter.y, m_d.m_radius);

   if (m_vpinball->m_alwaysDrawLightCenters)
   {
      psur->Line(m_d.m_vCenter.x - 10.0f, m_d.m_vCenter.y, m_d.m_vCenter.x + 10.0f, m_d.m_vCenter.y);
      psur->Line(m_d.m_vCenter.x, m_d.m_vCenter.y - 10.0f, m_d.m_vCenter.x, m_d.m_vCenter.y + 10.0f);
   }
}

void Bumper::RenderBlueprint(Sur *psur, const bool solid)
{
   psur->SetFillColor(solid ? BLUEPRINT_SOLID_COLOR : -1);
   psur->SetBorderColor(RGB(0, 0, 0), false, 0);
   psur->SetObject(this);
   psur->SetObject(nullptr);
   const float radangle = ANGTORAD(m_d.m_orientation - 90.f);
   const float sn = sinf(radangle);
   const float cs = cosf(radangle);

   const float x1 = m_d.m_vCenter.x - cs*(m_d.m_radius + 10.f);
   const float y1 = m_d.m_vCenter.y - sn*(m_d.m_radius + 10.f);
   const float x2 = m_d.m_vCenter.x + cs*(m_d.m_radius + 10.f);
   const float y2 = m_d.m_vCenter.y + sn*(m_d.m_radius + 10.f);
   psur->Ellipse(x1, y1, 10.0f);
   psur->Ellipse(x2, y2, 10.0f);
   psur->Ellipse(m_d.m_vCenter.x, m_d.m_vCenter.y, m_d.m_radius*1.5f);
   psur->Ellipse(m_d.m_vCenter.x, m_d.m_vCenter.y, m_d.m_radius);
}

void Bumper::GetTimers(vector<HitTimer*> &pvht)
{
   IEditable::BeginPlay();

   HitTimer * const pht = new HitTimer();
   pht->m_interval = m_d.m_tdr.m_TimerInterval >= 0 ? max(m_d.m_tdr.m_TimerInterval, MAX_TIMER_MSEC_INTERVAL) : -1;
   pht->m_nextfire = pht->m_interval;
   pht->m_pfe = (IFireEvents *)this;

   m_phittimer = pht;

   if (m_d.m_tdr.m_TimerEnabled)
      pvht.push_back(pht);
}

void Bumper::GetHitShapes(vector<HitObject*> &pvho)
{
   const float height = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y);

   BumperHitCircle * const phitcircle = new BumperHitCircle(m_d.m_vCenter,m_d.m_radius,height,height+m_d.m_heightScale);

   phitcircle->m_bumperanim_hitEvent = m_d.m_hitEvent;
   phitcircle->m_enabled = m_d.m_collidable;
   phitcircle->m_scatter = ANGTORAD(m_d.m_scatter);

   phitcircle->m_pbumper = this;

   pvho.push_back(phitcircle);

   m_pbumperhitcircle = phitcircle;
}

void Bumper::GetHitShapesDebug(vector<HitObject*> &pvho)
{
   const float height = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y);

   Hit3DPoly * const pcircle = new Hit3DPoly(m_d.m_vCenter.x, m_d.m_vCenter.y, height + m_d.m_heightScale, m_d.m_radius, 32);
   pvho.push_back(pcircle);
}

void Bumper::EndPlay()
{
   IEditable::EndPlay();

   m_pbumperhitcircle = nullptr;

   if (m_baseIndexBuffer)
   {
       SAFE_BUFFER_RELEASE(m_baseIndexBuffer);
       SAFE_BUFFER_RELEASE(m_baseVertexBuffer);
       m_baseTexture.FreeStuff();
   }
   if (m_ringIndexBuffer)
   {
      SAFE_BUFFER_RELEASE(m_ringIndexBuffer);
      SAFE_BUFFER_RELEASE(m_ringVertexBuffer);
      m_ringTexture.FreeStuff();
      delete[] m_ringVertices;
      m_ringVertices = nullptr;
   }
   if (m_capIndexBuffer)
   {
      SAFE_BUFFER_RELEASE(m_capIndexBuffer);
      SAFE_BUFFER_RELEASE(m_capVertexBuffer);
      m_capTexture.FreeStuff();
   }
   if (m_socketIndexBuffer)
   {
       SAFE_BUFFER_RELEASE(m_socketIndexBuffer);
       SAFE_BUFFER_RELEASE(m_socketVertexBuffer);
       m_skirtTexture.FreeStuff();
   }
}

void Bumper::UpdateRing()
{
   //TODO update Worldmatrix instead.
   Vertex3D_NoTex2 *buf;
   m_ringVertexBuffer->lock(0, 0, (void**)&buf, VertexBuffer::DISCARDCONTENTS);
   for (unsigned int i = 0; i < bumperRingNumVertices; i++)
   {
      buf[i].x = m_ringVertices[i].x;
      buf[i].y = m_ringVertices[i].y;
      buf[i].z = m_ringVertices[i].z + m_pbumperhitcircle->m_bumperanim_ringAnimOffset;
      buf[i].nx = m_ringVertices[i].nx;
      buf[i].ny = m_ringVertices[i].ny;
      buf[i].nz = m_ringVertices[i].nz;
      buf[i].tu = m_ringVertices[i].tu;
      buf[i].tv = m_ringVertices[i].tv;
   }
   m_ringVertexBuffer->unlock();
}

void Bumper::RenderBase(const Material * const baseMaterial)
{
   RenderDevice * const pd3dDevice = g_pplayer->m_pin3d.m_pd3dPrimaryDevice;

   pd3dDevice->basicShader->SetMaterial(baseMaterial, false);
   pd3dDevice->basicShader->SetTexture(SHADER_tex_base_color, &m_baseTexture);
   pd3dDevice->basicShader->SetAlphaTestValue((float)(1.0 / 255.0));

   pd3dDevice->basicShader->Begin();
   pd3dDevice->DrawIndexedPrimitiveVB(RenderDevice::TRIANGLELIST, MY_D3DFVF_NOTEX2_VERTEX, m_baseVertexBuffer, 0, bumperBaseNumVertices, m_baseIndexBuffer, 0, bumperBaseNumIndices);
   pd3dDevice->basicShader->End();
}

void Bumper::RenderSocket(const Material * const socketMaterial)
{
   RenderDevice * const pd3dDevice = g_pplayer->m_pin3d.m_pd3dPrimaryDevice;

   pd3dDevice->basicShader->SetMaterial(socketMaterial, false);
   pd3dDevice->basicShader->SetTexture(SHADER_tex_base_color, &m_skirtTexture);
   pd3dDevice->basicShader->SetAlphaTestValue((float)(1.0 / 255.0));

   pd3dDevice->basicShader->Begin();
   pd3dDevice->DrawIndexedPrimitiveVB(RenderDevice::TRIANGLELIST, MY_D3DFVF_NOTEX2_VERTEX, m_socketVertexBuffer, 0, bumperSocketNumVertices, m_socketIndexBuffer, 0, bumperSocketNumIndices);
   pd3dDevice->basicShader->End();
}

void Bumper::RenderCap(const Material * const capMaterial)
{
   RenderDevice * const pd3dDevice = g_pplayer->m_pin3d.m_pd3dPrimaryDevice;

   pd3dDevice->basicShader->SetMaterial(capMaterial, false);
   pd3dDevice->basicShader->SetTexture(SHADER_tex_base_color, &m_capTexture);
   pd3dDevice->basicShader->SetAlphaTestValue((float)(1.0 / 255.0));

   pd3dDevice->basicShader->Begin();
   pd3dDevice->DrawIndexedPrimitiveVB(RenderDevice::TRIANGLELIST, MY_D3DFVF_NOTEX2_VERTEX, m_capVertexBuffer, 0, bumperCapNumVertices, m_capIndexBuffer, 0, bumperCapNumIndices);
   pd3dDevice->basicShader->End();
}

void Bumper::UpdateSkirt(const bool doCalculation)
{
   constexpr float SKIRT_TILT = 5.0f;

   const float scalexy = m_d.m_radius;
   float rotx = 0.0f, roty = 0.0f;

   if (doCalculation)
   {
      const float hitx = m_pbumperhitcircle->m_bumperanim_hitBallPosition.x;
      const float hity = m_pbumperhitcircle->m_bumperanim_hitBallPosition.y;
      float dy = fabsf(hity - m_d.m_vCenter.y);
      if (dy == 0.0f)
         dy = 0.000001f;
      const float dx = fabsf(hitx - m_d.m_vCenter.x);
      const float skirtA = atanf(dx / dy);
      rotx = cosf(skirtA)*SKIRT_TILT;
      roty = sinf(skirtA)*SKIRT_TILT;
      if (m_d.m_vCenter.y < hity)
         rotx = -rotx;
      if (m_d.m_vCenter.x > hitx)
         roty = -roty;
   }

   Matrix3D tempMatrix, rMatrix;
   rMatrix.SetIdentity();

   tempMatrix.RotateZMatrix(ANGTORAD(m_d.m_orientation));
   tempMatrix.Multiply(rMatrix, rMatrix);

   tempMatrix.RotateYMatrix(ANGTORAD(roty));
   tempMatrix.Multiply(rMatrix, rMatrix);
   tempMatrix.RotateXMatrix(ANGTORAD(rotx));
   tempMatrix.Multiply(rMatrix, rMatrix);

   Vertex3D_NoTex2 *buf;
   m_socketVertexBuffer->lock(0, 0, (void**)&buf, VertexBuffer::DISCARDCONTENTS);
   for (unsigned int i = 0; i < bumperSocketNumVertices; i++)
   {
      Vertex3Ds vert(bumperSocket[i].x, bumperSocket[i].y, bumperSocket[i].z);
      vert = rMatrix.MultiplyVector(vert);
      buf[i].x = vert.x*scalexy + m_d.m_vCenter.x;
      buf[i].y = vert.y*scalexy + m_d.m_vCenter.y;
      buf[i].z = vert.z*(m_d.m_heightScale*m_ptable->m_BG_scalez[m_ptable->m_BG_current_set]) + (m_baseHeight+5.0f);

      vert = Vertex3Ds(bumperSocket[i].nx, bumperSocket[i].ny, bumperSocket[i].nz);
      vert = rMatrix.MultiplyVectorNoTranslate(vert);
      buf[i].nx = vert.x;
      buf[i].ny = vert.y;
      buf[i].nz = vert.z;
      buf[i].tu = bumperSocket[i].tu;
      buf[i].tv = bumperSocket[i].tv;
   }
   m_socketVertexBuffer->unlock();
}

void Bumper::RenderDynamic()
{
   RenderDevice * const pd3dDevice = g_pplayer->m_pin3d.m_pd3dPrimaryDevice;
   TRACE_FUNCTION();

   if (m_ptable->m_reflectionEnabled && !m_d.m_reflectionEnabled)
      return;

   const U32 old_time_msec = (m_d.m_time_msec < g_pplayer->m_time_msec) ? m_d.m_time_msec : g_pplayer->m_time_msec;
   m_d.m_time_msec = g_pplayer->m_time_msec;
   const float diff_time_msec = (float)(g_pplayer->m_time_msec - old_time_msec);

   pd3dDevice->SetRenderStateDepthBias(0.0f);
   pd3dDevice->SetRenderState(RenderDevice::ZWRITEENABLE, RenderDevice::RS_TRUE);
   pd3dDevice->SetRenderStateCulling(RenderDevice::CULL_CCW);

   if (m_pbumperhitcircle->m_bumperanim_hitEvent)
      g_pplayer->m_pininput.PlayRumble(0.1f, 0.05f, 100);

   const int state = m_pbumperhitcircle->m_bumperanim_hitEvent ? 1 : 0;    // 0 = not hit, 1 = hit
   m_pbumperhitcircle->m_bumperanim_hitEvent = false;

   if (m_d.m_ringVisible)
   {
      const float limit = m_d.m_ringDropOffset + (m_d.m_heightScale*0.5f)*m_ptable->m_BG_scalez[m_ptable->m_BG_current_set];

      if (state == 1)
      {
         m_ringAnimate = true;
         m_ringDown = true;
      }

      if (m_ringAnimate)
      {
         float step = m_d.m_ringSpeed*m_ptable->m_BG_scalez[m_ptable->m_BG_current_set];
         if (m_ringDown)
            step = -step;
         const float old_bumperanim_ringAnimOffset = m_pbumperhitcircle->m_bumperanim_ringAnimOffset;
         m_pbumperhitcircle->m_bumperanim_ringAnimOffset += step*diff_time_msec;
         if (m_ringDown)
         {
            if (m_pbumperhitcircle->m_bumperanim_ringAnimOffset <= -limit)
            {
               m_pbumperhitcircle->m_bumperanim_ringAnimOffset = -limit;
               m_ringDown = false;
            }
         }
         else
         {
            if (m_pbumperhitcircle->m_bumperanim_ringAnimOffset >= 0.0f)
            {
               m_pbumperhitcircle->m_bumperanim_ringAnimOffset = 0.0f;
               m_ringAnimate = false;
            }
         }
         if (m_ringVertexBuffer && (old_bumperanim_ringAnimOffset != m_pbumperhitcircle->m_bumperanim_ringAnimOffset))
            UpdateRing();
      }

      Material ringMaterial;
      if (m_d.m_szRingMaterial[0] != '\0')
      {
         ringMaterial = *(m_ptable->GetMaterial(m_d.m_szRingMaterial));
      }
      else
      {
         ringMaterial.m_cBase = 0xFFFFFFFF; //!! set properly
         ringMaterial.m_cGlossy = 0;
         ringMaterial.m_bIsMetal = true;
      }

      pd3dDevice->basicShader->SetTechniqueMetal(SHADER_TECHNIQUE_basic_with_texture, ringMaterial.m_bIsMetal);
      pd3dDevice->basicShader->SetTexture(SHADER_tex_base_color, &m_ringTexture);
      pd3dDevice->basicShader->SetMaterial(&ringMaterial, false);
      pd3dDevice->basicShader->SetAlphaTestValue(-1.0f);

      // render ring
      pd3dDevice->basicShader->Begin();
      pd3dDevice->DrawIndexedPrimitiveVB(RenderDevice::TRIANGLELIST, MY_D3DFVF_NOTEX2_VERTEX, m_ringVertexBuffer, 0, bumperRingNumVertices, m_ringIndexBuffer, 0, bumperRingNumIndices);
      pd3dDevice->basicShader->End();
   }

   if (m_d.m_skirtVisible)
   {
      if (m_enableSkirtAnimation)
      {
          if (state == 1)
          {
              m_doSkirtAnimation = true;
              UpdateSkirt(true);
              m_skirtCounter = 0.0f;
          }
          if (m_doSkirtAnimation)
          {
              m_skirtCounter += /*1.0f**/diff_time_msec;
              if (m_skirtCounter > 160.0f)
              {
                  m_doSkirtAnimation = false;
                  UpdateSkirt(false);
              }
          }
      }
      else if(m_updateSkirt) // do a single update if the animation was turned off via script
      {
         m_updateSkirt = false;
         UpdateSkirt(false);
      }

      const Material * const mat = m_ptable->GetMaterial(m_d.m_szSkirtMaterial);
      pd3dDevice->basicShader->SetTexture(SHADER_tex_base_color, &m_skirtTexture);
      pd3dDevice->basicShader->SetTechniqueMetal(SHADER_TECHNIQUE_basic_with_texture, mat->m_bIsMetal);
      pd3dDevice->SetRenderStateCulling(RenderDevice::CULL_NONE);
      RenderSocket(mat);
   }

   if (m_d.m_baseVisible)
   {
      const Material * const mat = m_ptable->GetMaterial(m_d.m_szBaseMaterial);
      if (mat->m_bOpacityActive)
      {
         pd3dDevice->basicShader->SetTechniqueMetal(SHADER_TECHNIQUE_basic_with_texture, mat->m_bIsMetal);
         pd3dDevice->SetRenderStateCulling(RenderDevice::CULL_NONE);
         RenderBase(mat);
      }
   }

   if (m_d.m_capVisible)
   {
      const Material * const mat = m_ptable->GetMaterial(m_d.m_szCapMaterial);
      if (mat->m_bOpacityActive)
      {
         pd3dDevice->basicShader->SetTechniqueMetal(SHADER_TECHNIQUE_basic_with_texture, mat->m_bIsMetal);
         pd3dDevice->SetRenderStateCulling(RenderDevice::CULL_NONE);
         RenderCap(mat);
      }
   }
}

void Bumper::ExportMesh(ObjLoader& loader)
{
   char name[sizeof(m_wzName)/sizeof(m_wzName[0])];
   WideCharToMultiByteNull(CP_ACP, 0, m_wzName, -1, name, sizeof(name), nullptr, nullptr);

   m_baseHeight = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y) * m_ptable->m_BG_scalez[m_ptable->m_BG_current_set];
   m_fullMatrix.RotateZMatrix(ANGTORAD(m_d.m_orientation));

   if (m_d.m_baseVisible)
   {
      const string subObjName = name + "Base"s;
      loader.WriteObjectName(subObjName);

      Vertex3D_NoTex2* base = new Vertex3D_NoTex2[bumperBaseNumVertices];
      GenerateBaseMesh(base);
      loader.WriteVertexInfo(base, bumperBaseNumVertices);
      const Material * const mat = m_ptable->GetMaterial(m_d.m_szBaseMaterial);
      loader.WriteMaterial(m_d.m_szBaseMaterial, string(), mat);
      loader.UseTexture(m_d.m_szBaseMaterial);
      loader.WriteFaceInfoList(bumperBaseIndices, bumperBaseNumIndices);
      loader.UpdateFaceOffset(bumperBaseNumVertices);
      delete[] base;
   }
   if (m_d.m_ringVisible)
   {
      const string subObjName = name + "Ring"s;
      loader.WriteObjectName(subObjName);

      Vertex3D_NoTex2* const ring = new Vertex3D_NoTex2[bumperRingNumVertices];
      GenerateRingMesh(ring);
      loader.WriteVertexInfo(ring, bumperRingNumVertices);
      loader.WriteFaceInfoList(bumperRingIndices, bumperRingNumIndices);
      loader.UpdateFaceOffset(bumperRingNumVertices);
      delete[] ring;
   }
   if (m_d.m_skirtVisible)
   {
      const string subObjName = name + "Skirt"s;
      loader.WriteObjectName(subObjName);

      Vertex3D_NoTex2* const socket = new Vertex3D_NoTex2[bumperSocketNumVertices];
      GenerateSocketMesh(socket);
      loader.WriteVertexInfo(socket, bumperSocketNumVertices);
      const Material * const mat = m_ptable->GetMaterial(m_d.m_szSkirtMaterial);
      loader.WriteMaterial(m_d.m_szSkirtMaterial, string(), mat);
      loader.UseTexture(m_d.m_szSkirtMaterial);
      loader.WriteFaceInfoList(bumperSocketIndices, bumperSocketNumIndices);
      loader.UpdateFaceOffset(bumperSocketNumVertices);
      delete[] socket;
   }
   if (m_d.m_capVisible)
   {
      const string subObjName = name + "Cap"s;
      loader.WriteObjectName(subObjName);

      Vertex3D_NoTex2* const cap = new Vertex3D_NoTex2[bumperCapNumVertices];
      GenerateCapMesh(cap);
      loader.WriteVertexInfo(cap, bumperCapNumVertices);
      const Material * const mat = m_ptable->GetMaterial(m_d.m_szCapMaterial);
      loader.WriteMaterial(m_d.m_szCapMaterial, string(), mat);
      loader.UseTexture(m_d.m_szCapMaterial);
      loader.WriteFaceInfoList(bumperCapIndices, bumperCapNumIndices);
      loader.UpdateFaceOffset(bumperCapNumVertices);
      delete[] cap;
   }
}

//
// license:GPLv3+
// Ported at: VisualPinball.Engine/VPT/Bumper/BumperMeshGenerator.cs
//

void Bumper::GenerateBaseMesh(Vertex3D_NoTex2 *buf)
{
   const float scalexy = m_d.m_radius;
   for (unsigned int i = 0; i < bumperBaseNumVertices; i++)
   {
      Vertex3Ds vert(bumperBase[i].x, bumperBase[i].y, bumperBase[i].z);
      vert = m_fullMatrix.MultiplyVector(vert);
      buf[i].x = vert.x*scalexy + m_d.m_vCenter.x;
      buf[i].y = vert.y*scalexy + m_d.m_vCenter.y;
      buf[i].z = vert.z*(m_d.m_heightScale*m_ptable->m_BG_scalez[m_ptable->m_BG_current_set]) + m_baseHeight;

      vert = Vertex3Ds(bumperBase[i].nx, bumperBase[i].ny, bumperBase[i].nz);
      vert = m_fullMatrix.MultiplyVectorNoTranslate(vert);
      buf[i].nx = vert.x;
      buf[i].ny = vert.y;
      buf[i].nz = vert.z;
      buf[i].tu = bumperBase[i].tu;
      buf[i].tv = bumperBase[i].tv;
   }
}

void Bumper::GenerateSocketMesh(Vertex3D_NoTex2 *buf)
{
   const float scalexy = m_d.m_radius;

   for (unsigned int i = 0; i < bumperSocketNumVertices; i++)
   {
      Vertex3Ds vert(bumperSocket[i].x, bumperSocket[i].y, bumperSocket[i].z);
      vert = m_fullMatrix.MultiplyVector(vert);
      buf[i].x = vert.x*scalexy + m_d.m_vCenter.x;
      buf[i].y = vert.y*scalexy + m_d.m_vCenter.y;
      buf[i].z = vert.z*(m_d.m_heightScale*m_ptable->m_BG_scalez[m_ptable->m_BG_current_set]) + (m_baseHeight+5.0f);

      vert = Vertex3Ds(bumperSocket[i].nx, bumperSocket[i].ny, bumperSocket[i].nz);
      vert = m_fullMatrix.MultiplyVectorNoTranslate(vert);
      buf[i].nx = vert.x;
      buf[i].ny = vert.y;
      buf[i].nz = vert.z;
      buf[i].tu = bumperSocket[i].tu;
      buf[i].tv = bumperSocket[i].tv;
   }
}

void Bumper::GenerateRingMesh(Vertex3D_NoTex2 *buf)
{
   const float scalexy = m_d.m_radius;

   for (unsigned int i = 0; i < bumperRingNumVertices; i++)
   {
      Vertex3Ds vert(bumperRing[i].x, bumperRing[i].y, bumperRing[i].z);
      vert = m_fullMatrix.MultiplyVector(vert);
      buf[i].x = vert.x*scalexy + m_d.m_vCenter.x;
      buf[i].y = vert.y*scalexy + m_d.m_vCenter.y;
      buf[i].z = vert.z*(m_d.m_heightScale*m_ptable->m_BG_scalez[m_ptable->m_BG_current_set]) + m_baseHeight;

      vert = Vertex3Ds(bumperRing[i].nx, bumperRing[i].ny, bumperRing[i].nz);
      vert = m_fullMatrix.MultiplyVectorNoTranslate(vert);
      buf[i].nx = vert.x;
      buf[i].ny = vert.y;
      buf[i].nz = vert.z;
      buf[i].tu = bumperRing[i].tu;
      buf[i].tv = bumperRing[i].tv;
   }
}

void Bumper::GenerateCapMesh(Vertex3D_NoTex2 *buf)
{
   const float scalexy = m_d.m_radius*2.0f;

   for (unsigned int i = 0; i < bumperCapNumVertices; i++)
   {
      Vertex3Ds vert(bumperCap[i].x, bumperCap[i].y, bumperCap[i].z);
      vert = m_fullMatrix.MultiplyVector(vert);
      buf[i].x = vert.x*scalexy + m_d.m_vCenter.x;
      buf[i].y = vert.y*scalexy + m_d.m_vCenter.y;
      buf[i].z = (vert.z*m_d.m_heightScale + m_d.m_heightScale)*m_ptable->m_BG_scalez[m_ptable->m_BG_current_set] + m_baseHeight;

      vert = Vertex3Ds(bumperCap[i].nx, bumperCap[i].ny, bumperCap[i].nz);
      vert = m_fullMatrix.MultiplyVectorNoTranslate(vert);
      buf[i].nx = vert.x;
      buf[i].ny = vert.y;
      buf[i].nz = vert.z;
      buf[i].tu = bumperCap[i].tu;
      buf[i].tv = bumperCap[i].tv;
   }
}

//
// end of license:GPLv3+, back to 'old MAME'-like
//

void Bumper::RenderSetup()
{
   m_d.m_time_msec = g_pplayer->m_time_msec;

   m_baseHeight = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y) * m_ptable->m_BG_scalez[m_ptable->m_BG_current_set];

   m_fullMatrix.RotateZMatrix(ANGTORAD(m_d.m_orientation));
   if (m_d.m_baseVisible)
   {
      m_baseTexture.CreateFromResource(IDB_BUMPER_BASE);
      SAFE_BUFFER_RELEASE(m_baseIndexBuffer);
      m_baseIndexBuffer = IndexBuffer::CreateAndFillIndexBuffer(bumperBaseNumIndices, bumperBaseIndices, PRIMARY_DEVICE);

      SAFE_BUFFER_RELEASE(m_baseVertexBuffer);
      m_baseVertexBuffer = new VertexBuffer(g_pplayer->m_pin3d.m_pd3dPrimaryDevice, bumperBaseNumVertices, 0, MY_D3DFVF_NOTEX2_VERTEX);

      Vertex3D_NoTex2 *buf;
      m_baseVertexBuffer->lock(0, 0, (void**)&buf, VertexBuffer::WRITEONLY);
      GenerateBaseMesh(buf);
      m_baseVertexBuffer->unlock();
   }

   if (m_d.m_skirtVisible)
   {
      m_skirtTexture.CreateFromResource(IDB_BUMPER_SKIRT);

      SAFE_BUFFER_RELEASE(m_socketIndexBuffer);
      m_socketIndexBuffer = IndexBuffer::CreateAndFillIndexBuffer(bumperSocketNumIndices, bumperSocketIndices, PRIMARY_DEVICE);

      SAFE_BUFFER_RELEASE(m_socketVertexBuffer);
      m_socketVertexBuffer = new VertexBuffer(g_pplayer->m_pin3d.m_pd3dPrimaryDevice, bumperSocketNumVertices, 0, MY_D3DFVF_NOTEX2_VERTEX);

      Vertex3D_NoTex2 *buf;
      m_socketVertexBuffer->lock(0, 0, (void**)&buf, VertexBuffer::WRITEONLY);
      GenerateSocketMesh(buf);
      m_socketVertexBuffer->unlock();
   }

   if (m_d.m_ringVisible)
   {
      m_ringTexture.CreateFromResource(IDB_BUMPER_RING);

      SAFE_BUFFER_RELEASE(m_ringIndexBuffer);
      m_ringIndexBuffer = IndexBuffer::CreateAndFillIndexBuffer(bumperRingNumIndices, bumperRingIndices, PRIMARY_DEVICE);

      SAFE_BUFFER_RELEASE(m_ringVertexBuffer);
      m_ringVertexBuffer = new VertexBuffer(g_pplayer->m_pin3d.m_pd3dPrimaryDevice, bumperRingNumVertices, USAGE_DYNAMIC, MY_D3DFVF_NOTEX2_VERTEX);

      m_ringVertices = new Vertex3D_NoTex2[bumperRingNumVertices];
      GenerateRingMesh(m_ringVertices);

      Vertex3D_NoTex2 *buf;
      m_ringVertexBuffer->lock(0, 0, (void**)&buf, VertexBuffer::DISCARDCONTENTS);
      memcpy(buf, m_ringVertices, bumperRingNumVertices*sizeof(Vertex3D_NoTex2));
      m_ringVertexBuffer->unlock();
   }

   if (m_d.m_capVisible)
   {
      m_capTexture.CreateFromResource(IDB_BUMPERCAP);

      SAFE_BUFFER_RELEASE(m_capIndexBuffer);
      m_capIndexBuffer = IndexBuffer::CreateAndFillIndexBuffer(bumperCapNumIndices, bumperCapIndices, PRIMARY_DEVICE);

      SAFE_BUFFER_RELEASE(m_capVertexBuffer);
      m_capVertexBuffer = new VertexBuffer(g_pplayer->m_pin3d.m_pd3dPrimaryDevice, bumperCapNumVertices, 0, MY_D3DFVF_NOTEX2_VERTEX);

      Vertex3D_NoTex2 *buf;
      m_capVertexBuffer->lock(0, 0, (void**)&buf, VertexBuffer::WRITEONLY);
      GenerateCapMesh(buf);
      m_capVertexBuffer->unlock();
   }

}

void Bumper::RenderStatic()
{
   const RenderDevice * const pd3dDevice = g_pplayer->m_pin3d.m_pd3dPrimaryDevice;

   if (m_ptable->m_reflectionEnabled && !m_d.m_reflectionEnabled)
      return;

   if (m_d.m_baseVisible)
   {
      const Material * const mat = m_ptable->GetMaterial(m_d.m_szBaseMaterial);
      if (!mat->m_bOpacityActive)
      {
         pd3dDevice->basicShader->SetTechniqueMetal(SHADER_TECHNIQUE_basic_with_texture, mat->m_bIsMetal);
         RenderBase(mat);
      }
   }

   if (m_d.m_capVisible)
   {
      const Material * const mat = m_ptable->GetMaterial(m_d.m_szCapMaterial);
      if (!mat->m_bOpacityActive)
      {
         pd3dDevice->basicShader->SetTechniqueMetal(SHADER_TECHNIQUE_basic_with_texture, mat->m_bIsMetal);
         RenderCap(mat);
      }
   }
}

void Bumper::SetObjectPos()
{
    m_vpinball->SetObjectPosCur(m_d.m_vCenter.x, m_d.m_vCenter.y);
}

void Bumper::MoveOffset(const float dx, const float dy)
{
   m_d.m_vCenter.x += dx;
   m_d.m_vCenter.y += dy;
}

Vertex2D Bumper::GetCenter() const
{
   return m_d.m_vCenter;
}

void Bumper::PutCenter(const Vertex2D& pv)
{
   m_d.m_vCenter = pv;
}

HRESULT Bumper::SaveData(IStream *pstm, HCRYPTHASH hcrypthash, const bool backupForPlay)
{
   BiffWriter bw(pstm, hcrypthash);

   bw.WriteVector2(FID(VCEN), m_d.m_vCenter);
   bw.WriteFloat(FID(RADI), m_d.m_radius);
   bw.WriteBool(FID(TMON), m_d.m_tdr.m_TimerEnabled);
   bw.WriteInt(FID(TMIN), m_d.m_tdr.m_TimerInterval);
   bw.WriteFloat(FID(THRS), m_d.m_threshold);
   bw.WriteFloat(FID(FORC), m_d.m_force);
   bw.WriteFloat(FID(BSCT), m_d.m_scatter);
   bw.WriteFloat(FID(HISC), m_d.m_heightScale);
   bw.WriteFloat(FID(RISP), m_d.m_ringSpeed);
   bw.WriteFloat(FID(ORIN), m_d.m_orientation);
   bw.WriteFloat(FID(RDLI), m_d.m_ringDropOffset);
   bw.WriteString(FID(MATR), m_d.m_szCapMaterial);
   bw.WriteString(FID(BAMA), m_d.m_szBaseMaterial);
   bw.WriteString(FID(SKMA), m_d.m_szSkirtMaterial);
   bw.WriteString(FID(RIMA), m_d.m_szRingMaterial);
   bw.WriteString(FID(SURF), m_d.m_szSurface);
   bw.WriteWideString(FID(NAME), m_wzName);

   bw.WriteBool(FID(CAVI), m_d.m_capVisible);
   bw.WriteBool(FID(BSVS), m_d.m_baseVisible);
   bw.WriteBool(FID(RIVS), m_d.m_ringVisible);
   bw.WriteBool(FID(SKVS), m_d.m_skirtVisible);
   bw.WriteBool(FID(HAHE), m_d.m_hitEvent);
   bw.WriteBool(FID(COLI), m_d.m_collidable);
   bw.WriteBool(FID(REEN), m_d.m_reflectionEnabled);

   ISelect::SaveData(pstm, hcrypthash);

   bw.WriteTag(FID(ENDB));

   return S_OK;
}


HRESULT Bumper::InitLoad(IStream *pstm, PinTable *ptable, int *pid, int version, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
{
   SetDefaults(false);
   BiffReader br(pstm, this, pid, version, hcrypthash, hcryptkey);

   m_ptable = ptable;

   br.Load();

   return S_OK;
}

bool Bumper::LoadToken(const int id, BiffReader * const pbr)
{
   switch(id)
   {
   case FID(PIID): pbr->GetInt((int *)pbr->m_pdata); break;
   case FID(VCEN): pbr->GetVector2(m_d.m_vCenter); break;
   case FID(RADI): pbr->GetFloat(m_d.m_radius); break;
   case FID(MATR): pbr->GetString(m_d.m_szCapMaterial); break;
   case FID(RIMA): pbr->GetString(m_d.m_szRingMaterial); break;
   case FID(BAMA): pbr->GetString(m_d.m_szBaseMaterial); break;
   case FID(SKMA): pbr->GetString(m_d.m_szSkirtMaterial); break;
   case FID(TMON): pbr->GetBool(m_d.m_tdr.m_TimerEnabled); break;
   case FID(TMIN): pbr->GetInt(m_d.m_tdr.m_TimerInterval); break;
   case FID(THRS): pbr->GetFloat(m_d.m_threshold); break;
   case FID(FORC): pbr->GetFloat(m_d.m_force); break;
   case FID(BSCT): pbr->GetFloat(m_d.m_scatter); break;
   case FID(HISC): pbr->GetFloat(m_d.m_heightScale); break;
   case FID(RISP): pbr->GetFloat(m_d.m_ringSpeed); break;
   case FID(ORIN): pbr->GetFloat(m_d.m_orientation); break;
   case FID(RDLI): pbr->GetFloat(m_d.m_ringDropOffset); break;
   case FID(SURF): pbr->GetString(m_d.m_szSurface); break;
   case FID(NAME): pbr->GetWideString(m_wzName,sizeof(m_wzName)/sizeof(m_wzName[0])); break;
   case FID(BVIS):
   {
      // backwards compatibility when loading old VP9 tables
      bool value;
      pbr->GetBool(value);
      m_d.m_capVisible = value;
      m_d.m_baseVisible = value;
      m_d.m_ringVisible = value;
      m_d.m_skirtVisible = value;
      break;
   }
   case FID(CAVI): pbr->GetBool(m_d.m_capVisible); break;
   case FID(HAHE): pbr->GetBool(m_d.m_hitEvent); break;
   case FID(COLI): pbr->GetBool(m_d.m_collidable); break;
   case FID(BSVS):
   {
      pbr->GetBool(m_d.m_baseVisible);
      // backwards compatibilty with pre 10.2 tables
      m_d.m_ringVisible = m_d.m_baseVisible;
      m_d.m_skirtVisible = m_d.m_baseVisible;
      break;
   }
   case FID(RIVS): pbr->GetBool(m_d.m_ringVisible); break;
   case FID(SKVS): pbr->GetBool(m_d.m_skirtVisible); break;
   case FID(REEN): pbr->GetBool(m_d.m_reflectionEnabled); break;
   default: ISelect::LoadToken(id, pbr); break;
   }
   return true;
}

HRESULT Bumper::InitPostLoad()
{
   return S_OK;
}

STDMETHODIMP Bumper::get_Radius(float *pVal)
{
   *pVal = m_d.m_radius;

   return S_OK;
}

STDMETHODIMP Bumper::put_Radius(float newVal)
{
   m_d.m_radius = newVal;

   return S_OK;
}

STDMETHODIMP Bumper::get_Force(float *pVal)
{
   *pVal = m_d.m_force;
   return S_OK;
}

STDMETHODIMP Bumper::put_Force(float newVal)
{
   m_d.m_force = newVal;

   return S_OK;
}

STDMETHODIMP Bumper::get_Scatter(float *pVal)
{
   *pVal = m_d.m_scatter;

   return S_OK;
}

STDMETHODIMP Bumper::put_Scatter(float newVal)
{
   /*if (m_pbumperhitcircle)
     m_pbumperhitcircle->m_scatter = ANGTORAD(m_d.m_scatter);
   else*/
     m_d.m_scatter = newVal;

   return S_OK;
}

STDMETHODIMP Bumper::get_HeightScale(float *pVal)
{
   *pVal = m_d.m_heightScale;

   return S_OK;
}

STDMETHODIMP Bumper::put_HeightScale(float newVal)
{
   m_d.m_heightScale = newVal;

   return S_OK;
}

STDMETHODIMP Bumper::get_RingSpeed(float *pVal)
{
   *pVal = m_d.m_ringSpeed;

   return S_OK;
}

STDMETHODIMP Bumper::put_RingSpeed(float newVal)
{
   m_d.m_ringSpeed = newVal;

   return S_OK;
}

STDMETHODIMP Bumper::get_RingDropOffset(float *pVal)
{
   *pVal = m_d.m_ringDropOffset;

   return S_OK;
}

STDMETHODIMP Bumper::put_RingDropOffset(float newVal)
{
   m_d.m_ringDropOffset = newVal;

   return S_OK;
}

STDMETHODIMP Bumper::get_Orientation(float *pVal)
{
   *pVal = m_d.m_orientation;

   return S_OK;
}

STDMETHODIMP Bumper::put_Orientation(float newVal)
{
   m_d.m_orientation = newVal;

   return S_OK;
}

STDMETHODIMP Bumper::get_Threshold(float *pVal)
{
   *pVal = m_d.m_threshold;

   return S_OK;
}

STDMETHODIMP Bumper::put_Threshold(float newVal)
{
   m_d.m_threshold = newVal;

   return S_OK;
}

STDMETHODIMP Bumper::get_CapMaterial(BSTR *pVal)
{
   WCHAR wz[MAXNAMEBUFFER];
   MultiByteToWideCharNull(CP_ACP, 0, m_d.m_szCapMaterial.c_str(), -1, wz, MAXNAMEBUFFER);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP Bumper::put_CapMaterial(BSTR newVal)
{
   char buf[MAXNAMEBUFFER];
   WideCharToMultiByteNull(CP_ACP, 0, newVal, -1, buf, MAXNAMEBUFFER, nullptr, nullptr);
   m_d.m_szCapMaterial = buf;

   return S_OK;
}

STDMETHODIMP Bumper::get_RingMaterial(BSTR *pVal)
{
   WCHAR wz[MAXNAMEBUFFER];
   MultiByteToWideCharNull(CP_ACP, 0, m_d.m_szRingMaterial.c_str(), -1, wz, MAXNAMEBUFFER);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP Bumper::put_RingMaterial(BSTR newVal)
{
   char buf[MAXNAMEBUFFER];
   WideCharToMultiByteNull(CP_ACP, 0, newVal, -1, buf, MAXNAMEBUFFER, nullptr, nullptr);
   m_d.m_szRingMaterial = buf;

   return S_OK;
}

STDMETHODIMP Bumper::get_BaseMaterial(BSTR *pVal)
{
   WCHAR wz[MAXNAMEBUFFER];
   MultiByteToWideCharNull(CP_ACP, 0, m_d.m_szBaseMaterial.c_str(), -1, wz, MAXNAMEBUFFER);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP Bumper::put_BaseMaterial(BSTR newVal)
{
   char buf[MAXNAMEBUFFER];
   WideCharToMultiByteNull(CP_ACP, 0, newVal, -1, buf, MAXNAMEBUFFER, nullptr, nullptr);
   m_d.m_szBaseMaterial = buf;

   return S_OK;
}

STDMETHODIMP Bumper::get_SkirtMaterial(BSTR *pVal)
{
   WCHAR wz[MAXNAMEBUFFER];
   MultiByteToWideCharNull(CP_ACP, 0, m_d.m_szSkirtMaterial.c_str(), -1, wz, MAXNAMEBUFFER);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP Bumper::put_SkirtMaterial(BSTR newVal)
{
   char buf[MAXNAMEBUFFER];
   WideCharToMultiByteNull(CP_ACP, 0, newVal, -1, buf, MAXNAMEBUFFER, nullptr, nullptr);
   m_d.m_szSkirtMaterial = buf;

   return S_OK;
}

STDMETHODIMP Bumper::get_X(float *pVal)
{
   *pVal = m_d.m_vCenter.x;
   m_vpinball->SetStatusBarUnitInfo(string(), true);

   return S_OK;
}

STDMETHODIMP Bumper::put_X(float newVal)
{
   m_d.m_vCenter.x = newVal;

   return S_OK;
}

STDMETHODIMP Bumper::get_Y(float *pVal)
{
   *pVal = m_d.m_vCenter.y;

   return S_OK;
}

STDMETHODIMP Bumper::put_Y(float newVal)
{
   m_d.m_vCenter.y = newVal;

   return S_OK;
}

STDMETHODIMP Bumper::get_Surface(BSTR *pVal)
{
   WCHAR wz[MAXTOKEN];
   MultiByteToWideCharNull(CP_ACP, 0, m_d.m_szSurface.c_str(), -1, wz, MAXTOKEN);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP Bumper::put_Surface(BSTR newVal)
{
   char buf[MAXTOKEN];
   WideCharToMultiByteNull(CP_ACP, 0, newVal, -1, buf, MAXTOKEN, nullptr, nullptr);
   m_d.m_szSurface = buf;

   return S_OK;
}


STDMETHODIMP Bumper::get_HasHitEvent(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_d.m_hitEvent);

   return S_OK;
}

STDMETHODIMP Bumper::put_HasHitEvent(VARIANT_BOOL newVal)
{
   m_d.m_hitEvent = VBTOb(newVal);

   return S_OK;
}

STDMETHODIMP Bumper::get_Collidable(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_d.m_collidable);

   return S_OK;
}

STDMETHODIMP Bumper::put_Collidable(VARIANT_BOOL newVal)
{
   m_d.m_collidable = VBTOb(newVal);
   if (m_pbumperhitcircle)
      m_pbumperhitcircle->m_enabled = m_d.m_collidable;

   return S_OK;
}


STDMETHODIMP Bumper::get_CapVisible(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_d.m_capVisible);

   return S_OK;
}

STDMETHODIMP Bumper::put_CapVisible(VARIANT_BOOL newVal)
{
   m_d.m_capVisible = VBTOb(newVal);

   return S_OK;
}

STDMETHODIMP Bumper::get_BaseVisible(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_d.m_baseVisible);

   return S_OK;
}

STDMETHODIMP Bumper::put_BaseVisible(VARIANT_BOOL newVal)
{
   m_d.m_baseVisible = VBTOb(newVal);

   return S_OK;
}

STDMETHODIMP Bumper::get_RingVisible(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_d.m_ringVisible);

   return S_OK;
}

STDMETHODIMP Bumper::put_RingVisible(VARIANT_BOOL newVal)
{
   m_d.m_ringVisible = VBTOb(newVal);

   return S_OK;
}

STDMETHODIMP Bumper::get_SkirtVisible(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_d.m_skirtVisible);

   return S_OK;
}

STDMETHODIMP Bumper::put_SkirtVisible(VARIANT_BOOL newVal)
{
   m_d.m_skirtVisible = VBTOb(newVal);

   return S_OK;
}

STDMETHODIMP Bumper::get_ReflectionEnabled(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_d.m_reflectionEnabled);

   return S_OK;
}

STDMETHODIMP Bumper::put_ReflectionEnabled(VARIANT_BOOL newVal)
{
   m_d.m_reflectionEnabled = VBTOb(newVal);

   return S_OK;
}

STDMETHODIMP Bumper::get_EnableSkirtAnimation(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_enableSkirtAnimation);

   return S_OK;
}

STDMETHODIMP Bumper::put_EnableSkirtAnimation(VARIANT_BOOL newVal)
{
   const bool val = VBTOb(newVal);
   if(m_enableSkirtAnimation != val)
   {
      if (!val)
         m_updateSkirt = true;
      m_enableSkirtAnimation = val;
   }

   return S_OK;
}

STDMETHODIMP Bumper::PlayHit()
{
   if ( m_pbumperhitcircle )
      m_pbumperhitcircle->m_bumperanim_hitEvent = true;

   return S_OK;
}

void Bumper::SetDefaultPhysics(bool fromMouseClick)
{
   m_d.m_force   = fromMouseClick ? LoadValueFloatWithDefault(regKey[RegName::DefaultPropsBumper], "Force"s, 15) : 15;
   m_d.m_scatter = fromMouseClick ? LoadValueFloatWithDefault(regKey[RegName::DefaultPropsBumper], "Scatter"s, 0) : 0;
}
