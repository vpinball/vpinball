#include "stdafx.h"
#include "objloader.h"
#include "meshes/bumperBaseMesh.h"
#include "meshes/bumperRingMesh.h"
#include "meshes/bumperCapMesh.h"
#include "meshes/bumperSocketMesh.h"
#include "renderer/Shader.h"
#include "renderer/IndexBuffer.h"
#include "renderer/VertexBuffer.h"

Bumper::Bumper()
{
   m_d.m_ringDropOffset = 0.0f;
}

Bumper::~Bumper()
{
   assert(m_rd == nullptr);
}

Bumper *Bumper::CopyForPlay(PinTable *live_table)
{
   STANDARD_EDITABLE_COPY_FOR_PLAY_IMPL(Bumper, live_table)
   return dst;
}

HRESULT Bumper::Init(PinTable * const ptable, const float x, const float y, const bool fromMouseClick, const bool forPlay)
{
   m_ptable = ptable;
   SetDefaults(fromMouseClick);
   m_d.m_vCenter.x = x;
   m_d.m_vCenter.y = y;
   return forPlay ? S_OK : InitVBA(fTrue, 0, nullptr);
}

void Bumper::SetDefaults(const bool fromMouseClick)
{
#define regKey Settings::DefaultPropsBumper

   m_d.m_radius = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "Radius"s, 45.f) : 45.f;

   SetDefaultPhysics(fromMouseClick);

   m_d.m_heightScale = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "HeightScale"s, 90.0f) : 90.0f;
   m_d.m_ringSpeed = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "RingSpeed"s, 0.5f) : 0.5f;
   m_d.m_orientation = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "Orientation"s, 0.0f) : 0.0f;
   m_d.m_threshold = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "Threshold"s, 1.f) : 1.f;

   const bool hr = g_pvp->m_settings.LoadValue(regKey, "Surface"s, m_d.m_szSurface);
   if (!hr || !fromMouseClick)
      m_d.m_szSurface.clear();

   m_d.m_tdr.m_TimerEnabled = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "TimerEnabled"s, false) : false;
   m_d.m_tdr.m_TimerInterval = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "TimerInterval"s, 100) : 100;
   m_d.m_capVisible = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "CapVisible"s, true) : true;
   m_d.m_baseVisible = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "BaseVisible"s, true) : true;
   m_d.m_ringVisible = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "RingVisible"s, true) : true;
   m_d.m_skirtVisible = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "SkirtVisible"s, true) : true;
   m_d.m_reflectionEnabled = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "ReflectionEnabled"s, true) : true;
   m_d.m_hitEvent = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "HasHitEvent"s, true) : true;
   m_d.m_collidable = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "Collidable"s, true) : true;

   m_ringAnimate = false;
   m_d.m_ringDropOffset = 0.0f;

#undef regKey
}

void Bumper::WriteRegDefaults()
{
#define regKey Settings::DefaultPropsBumper

   g_pvp->m_settings.SaveValue(regKey, "Radius"s, m_d.m_radius);
   g_pvp->m_settings.SaveValue(regKey, "Force"s, m_d.m_force);
   g_pvp->m_settings.SaveValue(regKey, "Scatter"s, m_d.m_scatter);
   g_pvp->m_settings.SaveValue(regKey, "HeightScale"s, m_d.m_heightScale);
   g_pvp->m_settings.SaveValue(regKey, "RingSpeed"s, m_d.m_ringSpeed);
   g_pvp->m_settings.SaveValue(regKey, "Orientation"s, m_d.m_orientation);
   g_pvp->m_settings.SaveValue(regKey, "Threshold"s, m_d.m_threshold);
   g_pvp->m_settings.SaveValue(regKey, "TimerEnabled"s, m_d.m_tdr.m_TimerEnabled);
   g_pvp->m_settings.SaveValue(regKey, "TimerInterval"s, m_d.m_tdr.m_TimerInterval);
   g_pvp->m_settings.SaveValue(regKey, "CapVisible"s, m_d.m_capVisible);
   g_pvp->m_settings.SaveValue(regKey, "BaseVisible"s, m_d.m_baseVisible);
   g_pvp->m_settings.SaveValue(regKey, "HasHitEvent"s, m_d.m_hitEvent);
   g_pvp->m_settings.SaveValue(regKey, "Collidable"s, m_d.m_collidable);
   g_pvp->m_settings.SaveValue(regKey, "ReflectionEnabled"s, m_d.m_reflectionEnabled);
   g_pvp->m_settings.SaveValue(regKey, "Surface"s, m_d.m_szSurface);

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
   m_phittimer = new HitTimer(GetName(), m_d.m_tdr.m_TimerInterval, this);
   if (m_d.m_tdr.m_TimerEnabled)
      pvht.push_back(m_phittimer);
}


#pragma region Physics

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
}

#pragma endregion


#pragma region Rendering

void Bumper::RenderSetup(RenderDevice *device)
{
   assert(m_rd == nullptr);
   m_rd = device;

   m_baseHeight = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y);

   m_fullMatrix.SetRotateZ(ANGTORAD(m_d.m_orientation));
   if (m_d.m_baseVisible)
   {
      m_baseTexture.LoadFromFile(g_pvp->m_szMyPath + "assets" + PATH_SEPARATOR_CHAR + "BumperBase.webp");
      m_baseTexture.m_alphaTestValue = (float)(-1.0 / 255.0);
      IndexBuffer* baseIndexBuffer = new IndexBuffer(m_rd, bumperBaseNumIndices, bumperBaseIndices);
      VertexBuffer* baseVertexBuffer = new VertexBuffer(m_rd, bumperBaseNumVertices);
      Vertex3D_NoTex2 *buf;
      baseVertexBuffer->lock(0, 0, (void**)&buf, VertexBuffer::WRITEONLY);
      GenerateBaseMesh(buf);
      baseVertexBuffer->unlock();
      delete m_baseMeshBuffer;
      m_baseMeshBuffer = new MeshBuffer(m_wzName + L".Base"s, baseVertexBuffer, baseIndexBuffer, true);
   }

   if (m_d.m_skirtVisible)
   {
      m_skirtTexture.LoadFromFile(g_pvp->m_szMyPath + "assets" + PATH_SEPARATOR_CHAR + "BumperSkirt.webp");
      m_skirtTexture.m_alphaTestValue = (float)(-1.0 / 255.0);
      IndexBuffer* socketIndexBuffer = new IndexBuffer(m_rd, bumperSocketNumIndices, bumperSocketIndices);
      VertexBuffer* socketVertexBuffer = new VertexBuffer(m_rd, bumperSocketNumVertices, nullptr, true);
      Vertex3D_NoTex2 *buf;
      socketVertexBuffer->lock(0, 0, (void**)&buf, VertexBuffer::WRITEONLY);
      GenerateSocketMesh(buf);
      socketVertexBuffer->unlock();
      delete m_socketMeshBuffer;
      m_socketMeshBuffer = new MeshBuffer(m_wzName + L".Socket"s, socketVertexBuffer, socketIndexBuffer, true);
   }

   if (m_d.m_ringVisible)
   {
      m_ringTexture.LoadFromFile(g_pvp->m_szMyPath + "assets" + PATH_SEPARATOR_CHAR + "BumperRing.webp");
      m_ringTexture.m_alphaTestValue = (float)(-1.0 / 255.0);
      IndexBuffer* ringIndexBuffer = new IndexBuffer(m_rd, bumperRingNumIndices, bumperRingIndices);
      VertexBuffer *ringVertexBuffer = new VertexBuffer(m_rd, bumperRingNumVertices, nullptr, true);
      m_ringVertices = new Vertex3D_NoTex2[bumperRingNumVertices];
      GenerateRingMesh(m_ringVertices);
      Vertex3D_NoTex2 *buf;
      ringVertexBuffer->lock(0, 0, (void**)&buf, VertexBuffer::DISCARDCONTENTS);
      memcpy(buf, m_ringVertices, bumperRingNumVertices*sizeof(Vertex3D_NoTex2));
      ringVertexBuffer->unlock();
      delete m_ringMeshBuffer;
      m_ringMeshBuffer = new MeshBuffer(m_wzName + L".Ring"s, ringVertexBuffer, ringIndexBuffer, true);
   }

   if (m_d.m_capVisible)
   {
      m_capTexture.LoadFromFile(g_pvp->m_szMyPath + "assets" + PATH_SEPARATOR_CHAR + "BumperCap.webp");
      m_capTexture.m_alphaTestValue = (float)(-1.0 / 255.0);
      IndexBuffer* capIndexBuffer = new IndexBuffer(m_rd, bumperCapNumIndices, bumperCapIndices);
      VertexBuffer* capVertexBuffer = new VertexBuffer(m_rd, bumperCapNumVertices);
      Vertex3D_NoTex2 *buf;
      capVertexBuffer->lock(0, 0, (void**)&buf, VertexBuffer::WRITEONLY);
      GenerateCapMesh(buf);
      capVertexBuffer->unlock();
      delete m_capMeshBuffer;
      m_capMeshBuffer = new MeshBuffer(m_wzName + L".Cap"s, capVertexBuffer, capIndexBuffer, true);
   }
}

void Bumper::RenderRelease()
{
   assert(m_rd != nullptr);
   delete m_baseMeshBuffer;
   delete m_ringMeshBuffer;
   delete m_capMeshBuffer;
   delete m_socketMeshBuffer;
   m_baseMeshBuffer = nullptr;
   m_ringMeshBuffer = nullptr;
   m_capMeshBuffer = nullptr;
   m_socketMeshBuffer = nullptr;
   m_baseTexture.FreeStuff();
   m_ringTexture.FreeStuff();

   delete[] m_ringVertices;
   m_ringVertices = nullptr;
   m_capTexture.FreeStuff();
   m_skirtTexture.FreeStuff();

   m_rd = nullptr;
}

void Bumper::Render(const unsigned int renderMask)
{
   assert(m_rd != nullptr);
   const bool isStaticOnly = renderMask & Player::STATIC_ONLY;
   const bool isDynamicOnly = renderMask & Player::DYNAMIC_ONLY;
   const bool isReflectionPass = renderMask & Player::REFLECTION_PASS;
   TRACE_FUNCTION();

   if (isReflectionPass && !m_d.m_reflectionEnabled)
      return;

   if (m_d.m_baseVisible)
   {
      const Material * const mat = m_ptable->GetMaterial(m_d.m_szBaseMaterial);
      if ((!mat->m_bOpacityActive && !isDynamicOnly) || (mat->m_bOpacityActive && !isStaticOnly))
      {
         m_rd->ResetRenderState();
         if (mat->m_bOpacityActive)
            m_rd->SetRenderState(RenderState::CULLMODE, RenderState::CULL_NONE);
         m_rd->basicShader->SetBasic(mat, &m_baseTexture);
         Vertex3Ds pos(m_d.m_vCenter.x, m_d.m_vCenter.y, m_baseHeight);
         m_rd->DrawMesh(m_rd->basicShader, false, pos, 0.f, m_baseMeshBuffer, RenderDevice::TRIANGLELIST, 0, bumperBaseNumIndices);
      }
   }

   if (m_d.m_capVisible)
   {
      const Material * const mat = m_ptable->GetMaterial(m_d.m_szCapMaterial);
      if ((!mat->m_bOpacityActive && !isDynamicOnly) || (mat->m_bOpacityActive && !isStaticOnly))
      {
         m_rd->ResetRenderState();
         if (mat->m_bOpacityActive)
            m_rd->SetRenderState(RenderState::CULLMODE, RenderState::CULL_NONE);
         m_rd->basicShader->SetBasic(mat, &m_capTexture);
         Vertex3Ds pos(m_d.m_vCenter.x, m_d.m_vCenter.y, m_baseHeight);
         m_rd->DrawMesh(m_rd->basicShader, false, pos, 0.f, m_capMeshBuffer, RenderDevice::TRIANGLELIST, 0, bumperCapNumIndices);
      }
   }

   if (m_d.m_ringVisible && !isStaticOnly)
   {
      Material ringMaterial;
      if (m_d.m_szRingMaterial[0] != '\0')
      {
         ringMaterial = *(m_ptable->GetMaterial(m_d.m_szRingMaterial));
      }
      else
      {
         ringMaterial.m_cBase = 0xFFFFFFFF; //!! set properly
         ringMaterial.m_cGlossy = 0;
         ringMaterial.m_type = Material::MaterialType::METAL;
      }
      m_rd->ResetRenderState();
      m_rd->basicShader->SetBasic(&ringMaterial, &m_ringTexture);
      Vertex3Ds pos(m_d.m_vCenter.x, m_d.m_vCenter.y, m_baseHeight + m_pbumperhitcircle->m_bumperanim_ringAnimOffset);
      m_rd->DrawMesh(m_rd->basicShader, false, pos, 0.f, m_ringMeshBuffer, RenderDevice::TRIANGLELIST, 0, bumperRingNumIndices);
   }

   if (m_d.m_skirtVisible && !isStaticOnly)
   {
      const Material * const mat = m_ptable->GetMaterial(m_d.m_szSkirtMaterial);
      m_rd->ResetRenderState();
      if (mat->m_bOpacityActive)
         m_rd->SetRenderState(RenderState::CULLMODE, RenderState::CULL_NONE);
      m_rd->basicShader->SetBasic(mat, &m_skirtTexture);
      Vertex3Ds pos(m_d.m_vCenter.x, m_d.m_vCenter.y, m_baseHeight + 5.0f);
      m_rd->DrawMesh(m_rd->basicShader, false, pos, 0.f, m_socketMeshBuffer, RenderDevice::TRIANGLELIST, 0, bumperSocketNumIndices);
   }
}

void Bumper::UpdateSkirt(const bool doCalculation)
{
   if (m_socketMeshBuffer == nullptr)
      return;

   constexpr float SKIRT_TILT = 5.0f;

   const float scalexy = m_d.m_radius;
   m_rotx = 0.0f, m_roty = 0.0f;

   if (doCalculation)
   {
      const float hitx = m_pbumperhitcircle->m_bumperanim_hitBallPosition.x;
      const float hity = m_pbumperhitcircle->m_bumperanim_hitBallPosition.y;
      float dy = fabsf(hity - m_d.m_vCenter.y);
      if (dy == 0.0f)
         dy = 0.000001f;
      const float dx = fabsf(hitx - m_d.m_vCenter.x);
      const float skirtA = atanf(dx / dy);
      m_rotx = cosf(skirtA)*SKIRT_TILT;
      m_roty = sinf(skirtA)*SKIRT_TILT;
      if (m_d.m_vCenter.y < hity)
         m_rotx = -m_rotx;
      if (m_d.m_vCenter.x > hitx)
         m_roty = -m_roty;
   }

   Matrix3D tempMatrix, rMatrix;
   rMatrix.SetIdentity();

   tempMatrix.SetRotateZ(ANGTORAD(m_d.m_orientation));
   tempMatrix.Multiply(rMatrix, rMatrix);

   tempMatrix.SetRotateY(ANGTORAD(m_roty));
   tempMatrix.Multiply(rMatrix, rMatrix);
   tempMatrix.SetRotateX(ANGTORAD(m_rotx));
   tempMatrix.Multiply(rMatrix, rMatrix);

   Vertex3D_NoTex2 *buf;
   m_socketMeshBuffer->m_vb->lock(0, 0, (void**)&buf, VertexBuffer::DISCARDCONTENTS);
   for (unsigned int i = 0; i < bumperSocketNumVertices; i++)
   {
      Vertex3Ds vert(bumperSocket[i].x, bumperSocket[i].y, bumperSocket[i].z);
      vert = rMatrix.MultiplyVector(vert);
      buf[i].x = vert.x*scalexy + m_d.m_vCenter.x;
      buf[i].y = vert.y*scalexy + m_d.m_vCenter.y;
      buf[i].z = vert.z*m_d.m_heightScale + (m_baseHeight+5.0f);

      vert = Vertex3Ds(bumperSocket[i].nx, bumperSocket[i].ny, bumperSocket[i].nz);
      vert = rMatrix.MultiplyVectorNoTranslate(vert);
      buf[i].nx = vert.x;
      buf[i].ny = vert.y;
      buf[i].nz = vert.z;
      buf[i].tu = bumperSocket[i].tu;
      buf[i].tv = bumperSocket[i].tv;
   }
   m_socketMeshBuffer->m_vb->unlock();
}

void Bumper::ExportMesh(ObjLoader& loader)
{
   char name[sizeof(m_wzName)/sizeof(m_wzName[0])];
   WideCharToMultiByteNull(CP_ACP, 0, m_wzName, -1, name, sizeof(name), nullptr, nullptr);

   m_baseHeight = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y);
   m_fullMatrix.SetRotateZ(ANGTORAD(m_d.m_orientation));

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
      buf[i].z = vert.z*m_d.m_heightScale + m_baseHeight;

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
      buf[i].z = vert.z*m_d.m_heightScale + (m_baseHeight+5.0f);

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
      buf[i].z = vert.z*m_d.m_heightScale + m_baseHeight;

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
      buf[i].z = vert.z*m_d.m_heightScale + (m_d.m_heightScale + m_baseHeight);

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

void Bumper::UpdateAnimation(const float diff_time_msec)
{
   if (m_pbumperhitcircle->m_bumperanim_hitEvent)
      g_pplayer->m_pininput.PlayRumble(0.1f, 0.05f, 100);

   const int state = m_pbumperhitcircle->m_bumperanim_hitEvent ? 1 : 0; // 0 = not hit, 1 = hit
   m_pbumperhitcircle->m_bumperanim_hitEvent = false;

   // Ring animation
   {
      const float limit = m_d.m_ringDropOffset + (m_d.m_heightScale * 0.5f);

      if (state == 1)
      {
         m_ringAnimate = true;
         m_ringDown = true;
      }

      if (m_ringAnimate)
      {
         float step = m_d.m_ringSpeed;
         if (m_ringDown)
              step = -step;
         const float old_bumperanim_ringAnimOffset = m_pbumperhitcircle->m_bumperanim_ringAnimOffset;
         m_pbumperhitcircle->m_bumperanim_ringAnimOffset += step * diff_time_msec;
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
         if (m_ringMeshBuffer && (old_bumperanim_ringAnimOffset != m_pbumperhitcircle->m_bumperanim_ringAnimOffset))
         {
            //TODO update Worldmatrix instead.
            Vertex3D_NoTex2 *buf;
            m_ringMeshBuffer->m_vb->lock(0, 0, (void **)&buf, VertexBuffer::DISCARDCONTENTS);
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
            m_ringMeshBuffer->m_vb->unlock();
         }

         FireGroupEvent(DISPID_AnimateEvents_Animate);
      }
   }

   // Skirt animation
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
              m_skirtCounter += /*1.0f**/ diff_time_msec;
              if (m_skirtCounter > 160.0f)
              {
                  m_doSkirtAnimation = false;
                  UpdateSkirt(false);
              }
              FireGroupEvent(DISPID_AnimateEvents_Animate);
         }
      }
      else if (m_updateSkirt) // do a single update if the animation was turned off via script
      {
         m_updateSkirt = false;
         UpdateSkirt(false);
         FireGroupEvent(DISPID_AnimateEvents_Animate);
      }
   }
}

#pragma endregion


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

HRESULT Bumper::SaveData(IStream *pstm, HCRYPTHASH hcrypthash, const bool saveForUndo)
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

STDMETHODIMP Bumper::get_CurrentRingOffset(float *pVal)
{
   *pVal = m_pbumperhitcircle->m_bumperanim_ringAnimOffset;
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

STDMETHODIMP Bumper::get_RotX(float *pVal)
{
   *pVal = m_rotx;
   return S_OK;
}

STDMETHODIMP Bumper::get_RotY(float *pVal)
{
   *pVal = m_roty;
   return S_OK;
}

void Bumper::SetDefaultPhysics(const bool fromMouseClick)
{
   m_d.m_force   = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(Settings::DefaultPropsBumper, "Force"s, 15.f) : 15.f;
   m_d.m_scatter = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(Settings::DefaultPropsBumper, "Scatter"s, 0.f) : 0.f;
}
