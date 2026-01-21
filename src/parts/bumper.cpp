// license:GPLv3+

#include "core/stdafx.h"
#include "utils/objloader.h"
#include "meshes/bumperBaseMesh.h"
#include "meshes/bumperRingMesh.h"
#include "meshes/bumperCapMesh.h"
#include "meshes/bumperSocketMesh.h"
#include "renderer/Shader.h"
#include "renderer/IndexBuffer.h"
#include "renderer/VertexBuffer.h"

Bumper::~Bumper()
{
   assert(m_rd == nullptr);
}

Bumper *Bumper::CopyForPlay(PinTable *live_table) const
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
   return forPlay ? S_OK : InitVBA(true, nullptr);
}

#define LinkProp(field, prop) field = fromMouseClick ? g_pvp->m_settings.GetDefaultPropsBumper_##prop() : Settings::GetDefaultPropsBumper_##prop##_Default()
void Bumper::SetDefaults(const bool fromMouseClick)
{
   LinkProp(m_d.m_radius, Radius);
   LinkProp(m_d.m_heightScale, HeightScale);
   LinkProp(m_d.m_ringSpeed, RingSpeed);
   LinkProp(m_d.m_orientation, Orientation);
   LinkProp(m_d.m_threshold, Threshold);
   LinkProp(m_d.m_capVisible, CapVisible);
   LinkProp(m_d.m_baseVisible, BaseVisible);
   LinkProp(m_d.m_hitEvent, HasHitEvent);
   LinkProp(m_d.m_collidable, Collidable);
   LinkProp(m_d.m_szSurface, Surface);
   LinkProp(m_d.m_reflectionEnabled, ReflectionEnabled);
   LinkProp(m_d.m_tdr.m_TimerEnabled, TimerEnabled);
   LinkProp(m_d.m_tdr.m_TimerInterval, TimerInterval);
   m_ringAnimate = false;
   m_d.m_ringDropOffset = 0.0f;
   SetDefaultPhysics(fromMouseClick);
}

void Bumper::SetDefaultPhysics(const bool fromMouseClick)
{
   LinkProp(m_d.m_force, Force);
   LinkProp(m_d.m_scatter, Scatter);
}
#undef LinkProp

void Bumper::WriteRegDefaults()
{
#define LinkProp(field, prop) g_pvp->m_settings.SetDefaultPropsBumper_##prop(field, false)
   LinkProp(m_d.m_radius, Radius);
   LinkProp(m_d.m_heightScale, HeightScale);
   LinkProp(m_d.m_ringSpeed, RingSpeed);
   LinkProp(m_d.m_orientation, Orientation);
   LinkProp(m_d.m_threshold, Threshold);
   LinkProp(m_d.m_capVisible, CapVisible);
   LinkProp(m_d.m_baseVisible, BaseVisible);
   LinkProp(m_d.m_hitEvent, HasHitEvent);
   LinkProp(m_d.m_collidable, Collidable);
   LinkProp(m_d.m_szSurface, Surface);
   LinkProp(m_d.m_force, Force);
   LinkProp(m_d.m_scatter, Scatter);
   LinkProp(m_d.m_reflectionEnabled, ReflectionEnabled);
   LinkProp(m_d.m_tdr.m_TimerEnabled, TimerEnabled);
   LinkProp(m_d.m_tdr.m_TimerInterval, TimerInterval);
#undef LinkProp
}

STDMETHODIMP Bumper::InterfaceSupportsErrorInfo(REFIID riid)
{
   static const IID* arr[] =
   {
      &IID_IBumper,
   };

   for (size_t i = 0; i < std::size(arr); i++)
      if (InlineIsEqualGUID(*arr[i], riid))
         return S_OK;

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


#pragma region Physics

void Bumper::PhysicSetup(PhysicsEngine* physics, const bool isUI)
{
   if (!isUI && GetPartGroup() != nullptr && GetPartGroup()->GetReferenceSpace() != PartGroupData::SpaceReference::SR_PLAYFIELD)
      return;

   const float height = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y);
   if (isUI)
   {
      HitCircle *const pcircle = new HitCircle(this, m_d.m_vCenter, m_d.m_radius, height, height + m_d.m_heightScale);
      physics->AddCollider(pcircle, isUI);
   }
   else
   {
      BumperHitCircle *const phitcircle = new BumperHitCircle(this, m_d.m_vCenter, m_d.m_radius, height, height + m_d.m_heightScale);
      phitcircle->m_bumperanim_hitEvent = m_d.m_hitEvent;
      phitcircle->m_enabled = m_d.m_collidable;
      phitcircle->m_scatter = ANGTORAD(m_d.m_scatter);
      m_pbumperhitcircle = phitcircle;
      physics->AddCollider(phitcircle, isUI);
   }
}

void Bumper::PhysicRelease(PhysicsEngine* physics, const bool isUI)
{
   if (!isUI)
      m_pbumperhitcircle = nullptr;
}

#pragma endregion


#pragma region Rendering

void Bumper::RenderSetup(RenderDevice *device)
{
   assert(m_rd == nullptr);
   m_rd = device;

   m_baseHeight = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y);

   m_fullMatrix = Matrix3D::MatrixRotateZ(ANGTORAD(m_d.m_orientation));
   // We always create all render data to support live editor
   //if (m_d.m_baseVisible)
   {
      m_baseTexture.reset(Texture::CreateFromFile(g_pvp->GetAppPath(VPinball::AppSubFolder::Assets, "BumperBase.webp").string()));
      std::shared_ptr<IndexBuffer> baseIndexBuffer = std::make_shared<IndexBuffer>(m_rd, bumperBaseNumIndices, bumperBaseIndices);
      std::shared_ptr<VertexBuffer> baseVertexBuffer = std::make_shared<VertexBuffer>(m_rd, bumperBaseNumVertices);
      Vertex3D_NoTex2 *buf;
      baseVertexBuffer->Lock(buf);
      GenerateBaseMesh(buf);
      baseVertexBuffer->Unlock();
      m_baseMeshBuffer = std::make_shared<MeshBuffer>(GetName() + ".Base", baseVertexBuffer, baseIndexBuffer, true);
   }

   //if (m_d.m_skirtVisible)
   {
      m_skirtTexture.reset(Texture::CreateFromFile(g_pvp->GetAppPath(VPinball::AppSubFolder::Assets, "BumperSkirt.webp").string()));
      std::shared_ptr<IndexBuffer> socketIndexBuffer = std::make_shared<IndexBuffer>(m_rd, bumperSocketNumIndices, bumperSocketIndices);
      std::shared_ptr<VertexBuffer> socketVertexBuffer = std::make_shared<VertexBuffer>(m_rd, bumperSocketNumVertices, nullptr, true);
      Vertex3D_NoTex2 *buf;
      socketVertexBuffer->Lock(buf);
      GenerateSocketMesh(buf);
      socketVertexBuffer->Unlock();
      m_socketMeshBuffer = std::make_shared<MeshBuffer>(GetName() + ".Socket"s, socketVertexBuffer, socketIndexBuffer, true);
   }

   //if (m_d.m_ringVisible)
   {
      m_ringTexture.reset(Texture::CreateFromFile(g_pvp->GetAppPath(VPinball::AppSubFolder::Assets, "BumperRing.webp").string()));
      std::shared_ptr<IndexBuffer> ringIndexBuffer = std::make_shared<IndexBuffer>(m_rd, bumperRingNumIndices, bumperRingIndices);
      std::shared_ptr<VertexBuffer> ringVertexBuffer = std::make_shared<VertexBuffer>(m_rd, bumperRingNumVertices, nullptr, true);
      m_ringVertices = new Vertex3D_NoTex2[bumperRingNumVertices];
      GenerateRingMesh(m_ringVertices);
      Vertex3D_NoTex2 *buf;
      ringVertexBuffer->Lock(buf);
      memcpy(buf, m_ringVertices, bumperRingNumVertices*sizeof(Vertex3D_NoTex2));
      ringVertexBuffer->Unlock();
      m_ringMeshBuffer = std::make_shared<MeshBuffer>(GetName() + ".Ring"s, ringVertexBuffer, ringIndexBuffer, true);
   }

   //if (m_d.m_capVisible)
   {
      m_capTexture.reset(Texture::CreateFromFile(g_pvp->GetAppPath(VPinball::AppSubFolder::Assets, "BumperCap.webp").string()));
      std::shared_ptr<IndexBuffer> capIndexBuffer = std::make_shared<IndexBuffer>(m_rd, bumperCapNumIndices, bumperCapIndices);
      std::shared_ptr<VertexBuffer> capVertexBuffer = std::make_shared<VertexBuffer>(m_rd, bumperCapNumVertices);
      Vertex3D_NoTex2 *buf;
      capVertexBuffer->Lock(buf);
      GenerateCapMesh(buf);
      capVertexBuffer->Unlock();
      m_capMeshBuffer = std::make_shared<MeshBuffer>(GetName() + ".Cap"s, capVertexBuffer, capIndexBuffer, true);
   }
}

void Bumper::RenderRelease()
{
   assert(m_rd != nullptr);
   m_baseMeshBuffer = nullptr;
   m_ringMeshBuffer = nullptr;
   m_capMeshBuffer = nullptr;
   m_socketMeshBuffer = nullptr;
   delete[] m_ringVertices;
   m_ringVertices = nullptr;
   if (m_baseTexture)
      m_rd->m_texMan.UnloadTexture(m_baseTexture.get());
   m_baseTexture = nullptr;
   if (m_ringTexture)
      m_rd->m_texMan.UnloadTexture(m_ringTexture.get());
   m_ringTexture = nullptr;
   if (m_capTexture)
      m_rd->m_texMan.UnloadTexture(m_capTexture.get());
   m_capTexture = nullptr;
   if (m_skirtTexture)
      m_rd->m_texMan.UnloadTexture(m_skirtTexture.get());
   m_skirtTexture = nullptr;

   m_rd = nullptr;
}

void Bumper::Render(const unsigned int renderMask)
{
   assert(m_rd != nullptr);
   assert(!m_backglass);
   const bool isStaticOnly = renderMask & Renderer::STATIC_ONLY;
   const bool isDynamicOnly = renderMask & Renderer::DYNAMIC_ONLY;
   const bool isReflectionPass = renderMask & Renderer::REFLECTION_PASS;
   const bool isUIPass = renderMask & Renderer::UI_EDGES || renderMask & Renderer::UI_FILL;
   TRACE_FUNCTION();

   if (isReflectionPass && !m_d.m_reflectionEnabled)
      return;

   if (m_d.m_baseVisible)
   {
      Vertex3Ds pos(m_d.m_vCenter.x, m_d.m_vCenter.y, m_baseHeight);
      if (isUIPass)
      {
         if (renderMask & Renderer::UI_FILL)
            m_rd->DrawMesh(m_rd->m_basicShader, true, pos, 0.f, m_baseMeshBuffer, RenderDevice::TRIANGLELIST, 0, bumperBaseNumIndices);
         // FIXME render wireframe
      }
      else
      {
         const Material *const mat = m_ptable->GetMaterial(m_d.m_szBaseMaterial);
         if ((!mat->m_bOpacityActive && !isDynamicOnly) || (mat->m_bOpacityActive && !isStaticOnly))
         {
            m_rd->ResetRenderState();
            if (mat->m_bOpacityActive)
               m_rd->SetRenderState(RenderState::CULLMODE, RenderState::CULL_NONE);
            m_rd->m_basicShader->SetBasic(mat, m_baseTexture.get());
            m_rd->DrawMesh(m_rd->m_basicShader, false, pos, 0.f, m_baseMeshBuffer, RenderDevice::TRIANGLELIST, 0, bumperBaseNumIndices);
         }
      }
   }

   if (m_d.m_capVisible)
   {
      Vertex3Ds pos(m_d.m_vCenter.x, m_d.m_vCenter.y, m_baseHeight);
      if (isUIPass)
      {
         if (renderMask & Renderer::UI_FILL)
            m_rd->DrawMesh(m_rd->m_basicShader, true, pos, 0.f, m_capMeshBuffer, RenderDevice::TRIANGLELIST, 0, bumperCapNumIndices);
         // FIXME render wireframe
      }
      else
      {
         const Material *const mat = m_ptable->GetMaterial(m_d.m_szCapMaterial);
         if ((!mat->m_bOpacityActive && !isDynamicOnly) || (mat->m_bOpacityActive && !isStaticOnly))
         {
            m_rd->ResetRenderState();
            if (mat->m_bOpacityActive)
               m_rd->SetRenderState(RenderState::CULLMODE, RenderState::CULL_NONE);
            m_rd->m_basicShader->SetBasic(mat, m_capTexture.get());
            m_rd->DrawMesh(m_rd->m_basicShader, false, pos, 0.f, m_capMeshBuffer, RenderDevice::TRIANGLELIST, 0, bumperCapNumIndices);
         }
      }
   }

   if (m_d.m_ringVisible && !isStaticOnly)
   {
      Vertex3Ds pos(m_d.m_vCenter.x, m_d.m_vCenter.y, m_baseHeight + m_pbumperhitcircle->m_bumperanim_ringAnimOffset);
      if (isUIPass)
      {
         if (renderMask & Renderer::UI_FILL)
            m_rd->DrawMesh(m_rd->m_basicShader, true, pos, 0.f, m_ringMeshBuffer, RenderDevice::TRIANGLELIST, 0, bumperRingNumIndices);
         // FIXME render wireframe
      }
      else
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
         m_rd->m_basicShader->SetBasic(&ringMaterial, m_ringTexture.get());
         m_rd->DrawMesh(m_rd->m_basicShader, false, pos, 0.f, m_ringMeshBuffer, RenderDevice::TRIANGLELIST, 0, bumperRingNumIndices);
      }
   }

   if (m_d.m_skirtVisible && !isStaticOnly)
   {
      Vertex3Ds pos(m_d.m_vCenter.x, m_d.m_vCenter.y, m_baseHeight + 5.0f);
      if (isUIPass)
      {
         if (renderMask & Renderer::UI_FILL)
            m_rd->DrawMesh(m_rd->m_basicShader, true, pos, 0.f, m_socketMeshBuffer, RenderDevice::TRIANGLELIST, 0, bumperSocketNumIndices);
         // FIXME render wireframe
      }
      else
      {
         const Material *const mat = m_ptable->GetMaterial(m_d.m_szSkirtMaterial);
         m_rd->ResetRenderState();
         if (mat->m_bOpacityActive)
            m_rd->SetRenderState(RenderState::CULLMODE, RenderState::CULL_NONE);
         m_rd->m_basicShader->SetBasic(mat, m_skirtTexture.get());
         m_rd->DrawMesh(m_rd->m_basicShader, false, pos, 0.f, m_socketMeshBuffer, RenderDevice::TRIANGLELIST, 0, bumperSocketNumIndices);
      }
   }
}

void Bumper::UpdateSkirt(const bool doCalculation)
{
   if (m_socketMeshBuffer == nullptr)
      return;

   constexpr float SKIRT_TILT = 5.0f;

   const float scalexy = m_d.m_radius;
   m_rotx = 0.0f;
   m_roty = 0.0f;

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

   const Matrix3D rMatrix = (Matrix3D::MatrixRotateZ(ANGTORAD(m_d.m_orientation))
                           * Matrix3D::MatrixRotateY(ANGTORAD(m_roty)))
                           * Matrix3D::MatrixRotateX(ANGTORAD(m_rotx));

   Vertex3D_NoTex2 *buf;
   m_socketMeshBuffer->m_vb->Lock(buf);
   for (unsigned int i = 0; i < bumperSocketNumVertices; i++)
   {
      Vertex3Ds vert = rMatrix * Vertex3Ds{bumperSocket[i].x, bumperSocket[i].y, bumperSocket[i].z};
      buf[i].x = vert.x*scalexy + m_d.m_vCenter.x;
      buf[i].y = vert.y*scalexy + m_d.m_vCenter.y;
      buf[i].z = vert.z*m_d.m_heightScale + (m_baseHeight+5.0f);

      vert = rMatrix.MultiplyVectorNoTranslate(Vertex3Ds{bumperSocket[i].nx, bumperSocket[i].ny, bumperSocket[i].nz});
      buf[i].nx = vert.x;
      buf[i].ny = vert.y;
      buf[i].nz = vert.z;
      buf[i].tu = bumperSocket[i].tu;
      buf[i].tv = bumperSocket[i].tv;
   }
   m_socketMeshBuffer->m_vb->Unlock();
}

void Bumper::ExportMesh(ObjLoader& loader)
{
   const string name = MakeString(m_wzName);

   m_baseHeight = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y);
   m_fullMatrix = Matrix3D::MatrixRotateZ(ANGTORAD(m_d.m_orientation));

   if (m_d.m_baseVisible)
   {
      const string subObjName = name + "Base";
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
      const string subObjName = name + "Ring";
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
      const string subObjName = name + "Skirt";
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
      const string subObjName = name + "Cap";
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

// Ported at: VisualPinball.Engine/VPT/Bumper/BumperMeshGenerator.cs

void Bumper::GenerateBaseMesh(Vertex3D_NoTex2 *buf) const
{
   const float scalexy = m_d.m_radius;
   for (unsigned int i = 0; i < bumperBaseNumVertices; i++)
   {
      Vertex3Ds vert = m_fullMatrix * Vertex3Ds{bumperBase[i].x, bumperBase[i].y, bumperBase[i].z};
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

void Bumper::GenerateSocketMesh(Vertex3D_NoTex2 *buf) const
{
   const float scalexy = m_d.m_radius;

   for (unsigned int i = 0; i < bumperSocketNumVertices; i++)
   {
      Vertex3Ds vert = m_fullMatrix * Vertex3Ds{bumperSocket[i].x, bumperSocket[i].y, bumperSocket[i].z};
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

void Bumper::GenerateRingMesh(Vertex3D_NoTex2 *buf) const
{
   const float scalexy = m_d.m_radius;

   for (unsigned int i = 0; i < bumperRingNumVertices; i++)
   {
      Vertex3Ds vert = m_fullMatrix * Vertex3Ds{bumperRing[i].x, bumperRing[i].y, bumperRing[i].z};
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

void Bumper::GenerateCapMesh(Vertex3D_NoTex2 *buf) const
{
   const float scalexy = m_d.m_radius*2.0f;

   for (unsigned int i = 0; i < bumperCapNumVertices; i++)
   {
      Vertex3Ds vert = m_fullMatrix * Vertex3Ds{bumperCap[i].x, bumperCap[i].y, bumperCap[i].z};
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
            m_ringMeshBuffer->m_vb->Lock(buf);
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
            m_ringMeshBuffer->m_vb->Unlock();
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


HRESULT Bumper::InitLoad(IStream *pstm, PinTable *ptable, int version, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
{
   SetDefaults(false);

   BiffReader br(pstm, this, version, hcrypthash, hcryptkey);

   m_ptable = ptable;

   br.Load();
   return S_OK;
}

bool Bumper::LoadToken(const int id, BiffReader * const pbr)
{
   switch(id)
   {
   case FID(PIID): { int pid; pbr->GetInt(&pid); } break;
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
   case FID(NAME): pbr->GetWideString(m_wzName, std::size(m_wzName)); break;
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
   *pVal = MakeWideBSTR(m_d.m_szCapMaterial);
   return S_OK;
}

STDMETHODIMP Bumper::put_CapMaterial(BSTR newVal)
{
   m_d.m_szCapMaterial = MakeString(newVal);
   return S_OK;
}

STDMETHODIMP Bumper::get_RingMaterial(BSTR *pVal)
{
   *pVal = MakeWideBSTR(m_d.m_szRingMaterial);
   return S_OK;
}

STDMETHODIMP Bumper::put_RingMaterial(BSTR newVal)
{
   m_d.m_szRingMaterial = MakeString(newVal);
   return S_OK;
}

STDMETHODIMP Bumper::get_BaseMaterial(BSTR *pVal)
{
   *pVal = MakeWideBSTR(m_d.m_szBaseMaterial);
   return S_OK;
}

STDMETHODIMP Bumper::put_BaseMaterial(BSTR newVal)
{
   m_d.m_szBaseMaterial = MakeString(newVal);
   return S_OK;
}

STDMETHODIMP Bumper::get_SkirtMaterial(BSTR *pVal)
{
   *pVal = MakeWideBSTR(m_d.m_szSkirtMaterial);
   return S_OK;
}

STDMETHODIMP Bumper::put_SkirtMaterial(BSTR newVal)
{
   m_d.m_szSkirtMaterial = MakeString(newVal);
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
   *pVal = MakeWideBSTR(m_d.m_szSurface);
   return S_OK;
}

STDMETHODIMP Bumper::put_Surface(BSTR newVal)
{
   m_d.m_szSurface = MakeString(newVal);
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
