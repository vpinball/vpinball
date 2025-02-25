// license:GPLv3+

#include "core/stdafx.h"
#include "utils/objloader.h"

#include "meshes/kickerCupMesh.h"
#include "meshes/kickerHoleMesh.h"
#include "meshes/kickerHitMesh.h"
#include "meshes/kickerSimpleHoleMesh.h"
#include "meshes/kickerWilliams.h"
#include "meshes/kickerGottlieb.h"
#include "meshes/kickerT1Mesh.h"
#include "renderer/Shader.h"
#include "renderer/IndexBuffer.h"
#include "renderer/VertexBuffer.h"

Kicker::Kicker()
{
}

Kicker::~Kicker()
{
   assert(m_rd == nullptr);
}

Kicker *Kicker::CopyForPlay(PinTable *live_table) const
{
   STANDARD_EDITABLE_COPY_FOR_PLAY_IMPL(Kicker, live_table)
   return dst;
}

void Kicker::UpdateStatusBarInfo()
{
   char tbuf[128];
   sprintf_s(tbuf, sizeof(tbuf), "Radius: %.3f", m_vpinball->ConvertToUnit(m_d.m_radius));
   m_vpinball->SetStatusBarUnitInfo(tbuf, true);
}

HRESULT Kicker::Init(PinTable *const ptable, const float x, const float y, const bool fromMouseClick, const bool forPlay)
{
   m_ptable = ptable;
   SetDefaults(fromMouseClick);
   m_d.m_vCenter.x = x;
   m_d.m_vCenter.y = y;
   return forPlay ? S_OK : InitVBA(fTrue, 0, nullptr);
}

void Kicker::SetDefaults(const bool fromMouseClick)
{
#define regKey Settings::DefaultPropsKicker

   m_d.m_radius = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "Radius"s, 25.f) : 25.f;
   m_d.m_tdr.m_TimerEnabled = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "TimerEnabled"s, false) : false;
   m_d.m_tdr.m_TimerInterval = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "TimerInterval"s, 100) : 100;
   m_d.m_enabled = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "Enabled"s, true) : true;
   m_d.m_hitAccuracy = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "HitAccuracy"s, 0.5f) : 0.5f;
   m_d.m_hit_height = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "HitHeight"s, 35.0f) : 35.0f;
   m_d.m_orientation = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "Orientation"s, 0.f) : 0.f;

   SetDefaultPhysics(fromMouseClick);

   const bool hr = g_pvp->m_settings.LoadValue(regKey, "Surface"s, m_d.m_szSurface);
   if (!hr || !fromMouseClick)
      m_d.m_szSurface.clear();

   m_d.m_kickertype = fromMouseClick ? (KickerType)g_pvp->m_settings.LoadValueWithDefault(regKey, "KickerType"s, (int)KickerHole) : KickerHole;
   //legacy handling:
   if (m_d.m_kickertype > KickerCup2)
      m_d.m_kickertype = KickerInvisible;

   m_d.m_fallThrough = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "FallThrough"s, false) : false;
   m_d.m_legacyMode = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "Legacy"s, true) : true;

#undef regKey
}

void Kicker::WriteRegDefaults()
{
   g_pvp->m_settings.SaveValue(Settings::DefaultPropsKicker, "TimerEnabled"s, m_d.m_tdr.m_TimerEnabled);
   g_pvp->m_settings.SaveValue(Settings::DefaultPropsKicker, "TimerInterval"s, m_d.m_tdr.m_TimerInterval);
   g_pvp->m_settings.SaveValue(Settings::DefaultPropsKicker, "Enabled"s, m_d.m_enabled);
   g_pvp->m_settings.SaveValue(Settings::DefaultPropsKicker, "HitAccuracy"s, m_d.m_hitAccuracy);
   g_pvp->m_settings.SaveValue(Settings::DefaultPropsKicker, "HitHeight"s, m_d.m_hit_height);
   g_pvp->m_settings.SaveValue(Settings::DefaultPropsKicker, "Orientation"s, m_d.m_orientation);
   g_pvp->m_settings.SaveValue(Settings::DefaultPropsKicker, "Radius"s, m_d.m_radius);
   g_pvp->m_settings.SaveValue(Settings::DefaultPropsKicker, "Scatter"s, m_d.m_scatter);
   g_pvp->m_settings.SaveValue(Settings::DefaultPropsKicker, "KickerType"s, m_d.m_kickertype);
   g_pvp->m_settings.SaveValue(Settings::DefaultPropsKicker, "Surface"s, m_d.m_szSurface);
   g_pvp->m_settings.SaveValue(Settings::DefaultPropsKicker, "FallThrough"s, m_d.m_fallThrough);
   g_pvp->m_settings.SaveValue(Settings::DefaultPropsKicker, "Legacy"s, m_d.m_legacyMode);
}

void Kicker::UIRenderPass1(Sur * const psur)
{
}

void Kicker::UIRenderPass2(Sur * const psur)
{
   psur->SetBorderColor(RGB(0, 0, 0), false, 0);
   psur->SetFillColor(-1);
   psur->SetObject(this);

   // Draw Arrow to display (default) orientation
   {
       const float radangle = ANGTORAD(m_d.m_orientation);
       constexpr float halflength = 50.0f;

       psur->SetLineColor(RGB(255, 0, 0), false, 1);

       Vertex2D tmp;
       {
           const float sn = sinf(radangle);
           const float cs = cosf(radangle);

           constexpr float len1 = halflength * 0.5f;
           tmp.x = m_d.m_vCenter.x + sn * len1;
           tmp.y = m_d.m_vCenter.y - cs * len1;
       }

       psur->Line(tmp.x, tmp.y, m_d.m_vCenter.x, m_d.m_vCenter.y);
       constexpr float len2 = halflength * 0.25f;
       {
           const float arrowang = radangle + 0.6f;
           const float sn = sinf(arrowang);
           const float cs = cosf(arrowang);

           psur->Line(tmp.x, tmp.y, m_d.m_vCenter.x + sn * len2, m_d.m_vCenter.y - cs * len2);
       }
       {
           const float arrowang = radangle - 0.6f;
           const float sn = sinf(arrowang);
           const float cs = cosf(arrowang);

           psur->Line(tmp.x, tmp.y, m_d.m_vCenter.x + sn * len2, m_d.m_vCenter.y - cs * len2);
       }
   }

   psur->Ellipse(m_d.m_vCenter.x, m_d.m_vCenter.y, m_d.m_radius);
   psur->Ellipse(m_d.m_vCenter.x, m_d.m_vCenter.y, m_d.m_radius*0.75f);
   psur->Ellipse(m_d.m_vCenter.x, m_d.m_vCenter.y, m_d.m_radius*0.5f);
   psur->Ellipse(m_d.m_vCenter.x, m_d.m_vCenter.y, m_d.m_radius*0.25f);
}

void Kicker::BeginPlay(vector<HitTimer*> &pvht) { IEditable::BeginPlay(pvht, &m_d.m_tdr, this); }

void Kicker::EndPlay() { IEditable::EndPlay(); }

#pragma region Physics

// Ported at: VisualPinball.Engine/VPT/Kicker/KickerHit.cs

void Kicker::PhysicSetup(PhysicsEngine* physics, const bool isUI)
{
   const float height = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y);
   const float radius = m_d.m_radius * (m_d.m_legacyMode ? (m_d.m_fallThrough ? 0.75f : 0.6f) // reduce the hit circle radius because only the inner circle of the kicker should start a hit event
                                                         : 1.f);
   if (isUI)
   {
      HitCircle *const phitcircle = new HitCircle(m_d.m_vCenter, radius, height, height + m_d.m_hit_height); // height of kicker hit cylinder
      phitcircle->m_ObjType = eKicker;
      phitcircle->m_obj = (IFireEvents *)this;
      physics->AddCollider(phitcircle, this, isUI);
   }
   else
   {
      KickerHitCircle *const phitcircle = new KickerHitCircle(m_d.m_vCenter, radius, height, height + m_d.m_hit_height); // height of kicker hit cylinder
      if (!m_d.m_legacyMode)
      {
         const float rad = phitcircle->radius * 0.8f;
         m_hitMesh.resize(kickerHitNumVertices);
         for (unsigned int t = 0; t < kickerHitNumVertices; t++)
         {
            // find the right normal by calculating the distance from current ball position to vertex of the kicker mesh
            Vertex3Ds vpos = Vertex3Ds(kickerHitMesh[t].x, kickerHitMesh[t].y, kickerHitMesh[t].z);
            vpos.x = vpos.x * rad + m_d.m_vCenter.x;
            vpos.y = vpos.y * rad + m_d.m_vCenter.y;
            vpos.z = vpos.z * rad + height;
            m_hitMesh[t] = vpos;
         }
      }
      phitcircle->m_enabled = m_d.m_enabled;
      phitcircle->m_ObjType = eKicker;
      phitcircle->m_obj = (IFireEvents *)this;
      phitcircle->m_pkicker = this;
      m_phitkickercircle = phitcircle;
      physics->AddCollider(phitcircle, this, isUI);
   }
}

void Kicker::PhysicRelease(PhysicsEngine* physics, const bool isUI)
{
   if (!isUI)
   {
      m_phitkickercircle = nullptr;
      m_hitMesh.clear();
   }
}

#pragma endregion



#pragma region Rendering

void Kicker::RenderSetup(RenderDevice *device)
{
   assert(m_rd == nullptr);
   m_rd = device;

   if (m_d.m_kickertype == KickerInvisible)
      return;

   m_baseHeight = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y);

   // Ported at: VisualPinball.Engine/VPT/Kicker/KickerMeshGenerator.cs

   if (m_d.m_kickertype == KickerCup || m_d.m_kickertype == KickerHole || m_d.m_kickertype == KickerHoleSimple || m_d.m_kickertype == KickerWilliams || m_d.m_kickertype == KickerGottlieb || m_d.m_kickertype == KickerCup2)
   {
      Vertex3D_NoTex2 *buf = new Vertex3D_NoTex2[kickerPlateNumVertices];
      float rad = m_d.m_radius; 

      if (m_d.m_kickertype == KickerWilliams || m_d.m_kickertype == KickerGottlieb)
          rad = m_d.m_radius * 0.88f;
      else if (m_d.m_kickertype == KickerCup2)
          rad = m_d.m_radius * 0.87f;
      else if (m_d.m_kickertype != KickerCup && m_d.m_kickertype != KickerWilliams)
          rad = m_d.m_radius * 0.82f;

      for (unsigned int i = 0; i < kickerPlateNumVertices; i++)
      {
         buf[i].x = kickerPlate[i].x*rad + m_d.m_vCenter.x;
         buf[i].y = kickerPlate[i].y*rad + m_d.m_vCenter.y;
         buf[i].z = kickerPlate[i].z*rad + m_baseHeight;
         buf[i].nx = kickerPlate[i].nx;
         buf[i].ny = kickerPlate[i].ny;
         buf[i].nz = kickerPlate[i].nz;
         buf[i].tu = 0.0f;
         buf[i].tv = 0.0f;
      }

      VertexBuffer *plateVertexBuffer = new VertexBuffer(m_rd, kickerPlateNumVertices, (float*)buf);
      IndexBuffer *plateIndexBuffer = new IndexBuffer(m_rd, kickerPlateNumIndices, kickerPlateIndices);
      m_plateMeshBuffer = new MeshBuffer(m_wzName + L".Plate"s, plateVertexBuffer, plateIndexBuffer, true);

      delete[] buf;
   }

   const WORD * indices;
   switch (m_d.m_kickertype)
   {
       case KickerInvisible:
          assert(false);
       break;
       case KickerCup:
       {
          m_texture.LoadFromFile(g_pvp->m_szMyPath + "assets" + PATH_SEPARATOR_CHAR + "KickerCup.webp");
          m_texture.m_alphaTestValue = (float)(-1.0 / 255.0);
          m_numIndices = kickerCupNumIndices;
          m_numVertices = kickerCupNumVertices;
          indices = kickerCupIndices;
       }
       break;
       case KickerWilliams:
       {
          m_texture.LoadFromFile(g_pvp->m_szMyPath + "assets" + PATH_SEPARATOR_CHAR + "KickerWilliams.webp");
          m_texture.m_alphaTestValue = (float)(-1.0 / 255.0);
          m_numIndices = kickerWilliamsNumIndices;
          m_numVertices = kickerWilliamsNumVertices;
          indices = kickerWilliamsIndices;
       }
       break;
       case KickerGottlieb:
       {
          m_texture.LoadFromFile(g_pvp->m_szMyPath + "assets" + PATH_SEPARATOR_CHAR + "KickerGottlieb.webp");
          m_texture.m_alphaTestValue = (float)(-1.0 / 255.0);
          m_numIndices = kickerGottliebNumIndices;
          m_numVertices = kickerGottliebNumVertices;
          indices = kickerGottliebIndices;
       }
       break;
       case KickerCup2:
       {
          m_texture.LoadFromFile(g_pvp->m_szMyPath + "assets" + PATH_SEPARATOR_CHAR + "KickerT1.webp");
          m_texture.m_alphaTestValue = (float)(-1.0 / 255.0);
          m_numIndices = kickerT1NumIndices;
          m_numVertices = kickerT1NumVertices;
          indices = kickerT1Indices;
       }
       break;
       case KickerHole:
       {
          m_texture.LoadFromFile(g_pvp->m_szMyPath + "assets" + PATH_SEPARATOR_CHAR + "KickerHoleWood.webp");
          m_texture.m_alphaTestValue = (float)(-1.0 / 255.0);
          m_numIndices = kickerHoleNumIndices;
          m_numVertices = kickerHoleNumVertices;
          indices = kickerHoleIndices;
       }
       break;
       default:
       case KickerHoleSimple:
       {
          m_texture.LoadFromFile(g_pvp->m_szMyPath + "assets" + PATH_SEPARATOR_CHAR + "KickerHoleWood.webp");
          m_texture.m_alphaTestValue = (float)(-1.0 / 255.0);
          m_numIndices = kickerSimpleHoleNumIndices;
          m_numVertices = kickerSimpleHoleNumVertices;
          indices = kickerSimpleHoleIndices;
       }
       break;
   }

   //

   VertexBuffer *vertexBuffer = new VertexBuffer(m_rd, m_numVertices);
   Vertex3D_NoTex2 *buf;
   vertexBuffer->Lock(buf);
   GenerateMesh(buf);
   vertexBuffer->Unlock();
   IndexBuffer *indexBuffer = new IndexBuffer(m_rd, m_numIndices, indices);
   m_meshBuffer = new MeshBuffer(m_wzName + L".Kicker"s, vertexBuffer, indexBuffer, true);
}

void Kicker::RenderRelease()
{
   assert(m_rd != nullptr);
   delete m_meshBuffer;
   delete m_plateMeshBuffer;
   m_meshBuffer = nullptr;
   m_plateMeshBuffer = nullptr;
   m_texture.FreeStuff();
   m_rd = nullptr;
}

void Kicker::UpdateAnimation(const float diff_time_msec)
{
   assert(m_rd != nullptr);
}

void Kicker::Render(const unsigned int renderMask)
{
   assert(m_rd != nullptr);
   const bool isStaticOnly = renderMask & Renderer::STATIC_ONLY;
   const bool isDynamicOnly = renderMask & Renderer::DYNAMIC_ONLY;
   const bool isReflectionPass = renderMask & Renderer::REFLECTION_PASS;
   TRACE_FUNCTION();

   if (isStaticOnly 
   || isReflectionPass 
   || !(m_d.m_kickertype == KickerCup || m_d.m_kickertype == KickerHole || m_d.m_kickertype == KickerHoleSimple || m_d.m_kickertype == KickerWilliams || m_d.m_kickertype == KickerGottlieb || m_d.m_kickertype == KickerCup2))
      return;

   m_rd->ResetRenderState();
   if (m_d.m_kickertype == KickerHoleSimple)
      m_rd->SetRenderState(RenderState::CULLMODE, RenderState::CULL_NONE);

   const Material * const mat = m_ptable->GetMaterial(m_d.m_szMaterial);
   m_rd->m_basicShader->SetMaterial(mat);
   m_rd->m_basicShader->SetTechniqueMaterial(SHADER_TECHNIQUE_kickerBoolean, *mat);
   m_rd->SetRenderState(RenderState::ZFUNC, RenderState::Z_ALWAYS);
   Vertex3Ds pos(m_d.m_vCenter.x, m_d.m_vCenter.y, m_baseHeight);
   m_rd->DrawMesh(m_rd->m_basicShader, false, pos, 0.f, m_plateMeshBuffer, RenderDevice::TRIANGLELIST, 0, kickerPlateNumIndices);

   m_rd->SetRenderState(RenderState::ZFUNC, RenderState::Z_LESSEQUAL);
   m_rd->m_basicShader->SetBasic(mat, m_d.m_kickertype == KickerHoleSimple ? nullptr : &m_texture);
   m_rd->EnableAlphaBlend(false);
   m_rd->DrawMesh(m_rd->m_basicShader, false, pos, 0.f, m_meshBuffer, RenderDevice::TRIANGLELIST, 0, m_numIndices);
}

#pragma endregion

void Kicker::ExportMesh(ObjLoader& loader)
{
   if (m_d.m_kickertype == KickerInvisible)
      return;

   char name[sizeof(m_wzName)/sizeof(m_wzName[0])];
   WideCharToMultiByteNull(CP_ACP, 0, m_wzName, -1, name, sizeof(name), nullptr, nullptr);
   m_baseHeight = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y);

   int num_vertices;
   int num_indices;
   const WORD *indices;
   switch (m_d.m_kickertype)
   {
       case KickerCup:
       {
           num_vertices = kickerCupNumVertices;
           num_indices = kickerCupNumIndices;
           indices = kickerCupIndices;
       }
       break;
       case KickerWilliams:
       {
           num_vertices = kickerWilliamsNumVertices;
           num_indices = kickerWilliamsNumIndices;
           indices = kickerWilliamsIndices;
       }
       break;
       case KickerGottlieb:
       {
           num_vertices = kickerGottliebNumVertices;
           num_indices = kickerGottliebNumIndices;
           indices = kickerGottliebIndices;
       }
       break;
       case KickerCup2:
       {
           num_vertices = kickerT1NumVertices;
           num_indices = kickerT1NumIndices;
           indices = kickerT1Indices;
       }
       break;
       case KickerHole:
       {
           num_vertices = kickerHoleNumVertices;
           num_indices = kickerHoleNumIndices;
           indices = kickerHoleIndices;
       }
       break;
       case KickerInvisible:
           assert(false);
       default:
       case KickerHoleSimple:
       {
           num_vertices = kickerSimpleHoleNumVertices;
           num_indices = kickerSimpleHoleNumIndices;
           indices = kickerSimpleHoleIndices;
       }
       break;
   }

   Vertex3D_NoTex2 *vertices = new Vertex3D_NoTex2[num_vertices];
   GenerateMesh(vertices);
   loader.WriteObjectName(name);
   loader.WriteVertexInfo(vertices, num_vertices);
   const Material * const mat = m_ptable->GetMaterial(m_d.m_szMaterial);
   loader.WriteMaterial(m_d.m_szMaterial, string(), mat);
   loader.UseTexture(m_d.m_szMaterial);
   loader.WriteFaceInfoList(indices, num_indices);
   loader.UpdateFaceOffset(num_vertices);
   delete[] vertices;
}

void Kicker::GenerateMesh(Vertex3D_NoTex2 *const buf)
{
   int num_vertices;
   const Vertex3D_NoTex2 *vertices;
   float zoffset = 0.f;
   float zrot = m_d.m_orientation;
   switch (m_d.m_kickertype)
   {
       case KickerCup:
       {
           num_vertices = kickerCupNumVertices;
           vertices = kickerCupMesh;
           zoffset = -0.18f;
       }
       break;
       case KickerWilliams:
       {
           num_vertices = kickerWilliamsNumVertices;
           vertices = kickerWilliamsMesh;
           zrot = m_d.m_orientation + 90.f;
       }
       break;
       case KickerGottlieb:
       {
           num_vertices = kickerGottliebNumVertices;
           vertices = kickerGottliebMesh;
       }
       break;
       case KickerCup2:
       {
           num_vertices = kickerT1NumVertices;
           vertices = kickerT1Mesh;
       }
       break;
       case KickerHole:
       {
           num_vertices = kickerHoleNumVertices;
           vertices = kickerHoleMesh;
           zrot = 0.f;
       }
       break;
       case KickerInvisible:
           assert(false);
       default:
       case KickerHoleSimple:
       {
           num_vertices = kickerSimpleHoleNumVertices;
           vertices = kickerSimpleHoleMesh;
           zrot = 0.f;
       }
       break;
   }

   const Matrix3D fullMatrix = Matrix3D::MatrixRotateZ(ANGTORAD(zrot));

   for (int i = 0; i < num_vertices; i++)
   {
      Vertex3Ds vert = fullMatrix * Vertex3Ds{vertices[i].x, vertices[i].y, vertices[i].z + zoffset};

      buf[i].x = vert.x*m_d.m_radius + m_d.m_vCenter.x;
      buf[i].y = vert.y*m_d.m_radius + m_d.m_vCenter.y;
      buf[i].z = vert.z*m_d.m_radius + m_baseHeight;

      vert = fullMatrix.MultiplyVectorNoTranslate(Vertex3Ds{vertices[i].nx, vertices[i].ny, vertices[i].nz});
      buf[i].nx = vert.x;
      buf[i].ny = vert.y;
      buf[i].nz = vert.z;
      buf[i].tu = vertices[i].tu;
      buf[i].tv = vertices[i].tv;
   }
}

void Kicker::SetDefaultPhysics(const bool fromMouseClick)
{
   m_d.m_scatter = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(Settings::DefaultPropsKicker, "Scatter"s, 0.f) : 0.f;
}

void Kicker::SetObjectPos()
{
    m_vpinball->SetObjectPosCur(m_d.m_vCenter.x, m_d.m_vCenter.y);
}

void Kicker::MoveOffset(const float dx, const float dy)
{
   m_d.m_vCenter.x += dx;
   m_d.m_vCenter.y += dy;
}

Vertex2D Kicker::GetCenter() const
{
   return m_d.m_vCenter;
}

void Kicker::PutCenter(const Vertex2D& pv)
{
   m_d.m_vCenter = pv;
}


HRESULT Kicker::SaveData(IStream *pstm, HCRYPTHASH hcrypthash, const bool saveForUndo)
{
   BiffWriter bw(pstm, hcrypthash);

   bw.WriteVector2(FID(VCEN), m_d.m_vCenter);
   bw.WriteFloat(FID(RADI), m_d.m_radius);
   bw.WriteBool(FID(TMON), m_d.m_tdr.m_TimerEnabled);
   bw.WriteInt(FID(TMIN), m_d.m_tdr.m_TimerInterval);
   bw.WriteString(FID(MATR), m_d.m_szMaterial);
   bw.WriteString(FID(SURF), m_d.m_szSurface);
   bw.WriteBool(FID(EBLD), m_d.m_enabled);
   bw.WriteWideString(FID(NAME), m_wzName);
   bw.WriteInt(FID(TYPE), m_d.m_kickertype);
   bw.WriteFloat(FID(KSCT), m_d.m_scatter);
   bw.WriteFloat(FID(KHAC), m_d.m_hitAccuracy);
   bw.WriteFloat(FID(KHHI), m_d.m_hit_height);
   bw.WriteFloat(FID(KORI), m_d.m_orientation);
   bw.WriteBool(FID(FATH), m_d.m_fallThrough);
   bw.WriteBool(FID(LEMO), m_d.m_legacyMode);

   ISelect::SaveData(pstm, hcrypthash);

   bw.WriteTag(FID(ENDB));

   return S_OK;
}

HRESULT Kicker::InitLoad(IStream *pstm, PinTable *ptable, int *pid, int version, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
{
   SetDefaults(false);

   BiffReader br(pstm, this, pid, version, hcrypthash, hcryptkey);

   m_ptable = ptable;

   br.Load();
   return S_OK;
}

bool Kicker::LoadToken(const int id, BiffReader * const pbr)
{
   switch(id)
   {
   case FID(PIID): pbr->GetInt((int *)pbr->m_pdata); break;
   case FID(VCEN): pbr->GetStruct(&m_d.m_vCenter, sizeof(Vertex2D)); break;
   case FID(RADI): pbr->GetFloat(m_d.m_radius); break;
   case FID(KSCT): pbr->GetFloat(m_d.m_scatter); break;
   case FID(KHAC): pbr->GetFloat(m_d.m_hitAccuracy); break;
   case FID(KHHI): pbr->GetFloat(m_d.m_hit_height); break;
   case FID(KORI): pbr->GetFloat(m_d.m_orientation); break;
   case FID(MATR): pbr->GetString(m_d.m_szMaterial); break;
   case FID(TMON): pbr->GetBool(m_d.m_tdr.m_TimerEnabled); break;
   case FID(EBLD): pbr->GetBool(m_d.m_enabled); break;
   case FID(TMIN): pbr->GetInt(m_d.m_tdr.m_TimerInterval); break;
   case FID(TYPE):
   {
      pbr->GetInt(&m_d.m_kickertype);
      // legacy handling:
      if (m_d.m_kickertype > KickerCup2)
          m_d.m_kickertype = KickerInvisible;
      break;
   }
   case FID(SURF): pbr->GetString(m_d.m_szSurface); break;
   case FID(NAME): pbr->GetWideString(m_wzName, std::size(m_wzName)); break;
   case FID(FATH): pbr->GetBool(m_d.m_fallThrough); break;
   case FID(LEMO): pbr->GetBool(m_d.m_legacyMode); break;
   default: ISelect::LoadToken(id, pbr); break;
   }
   return true;
}

HRESULT Kicker::InitPostLoad()
{
   m_phitkickercircle = nullptr;
   return S_OK;
}


STDMETHODIMP Kicker::InterfaceSupportsErrorInfo(REFIID riid)
{
   static const IID* arr[] =
   {
      &IID_IKicker,
   };

   for (size_t i = 0; i < std::size(arr); i++)
      if (InlineIsEqualGUID(*arr[i], riid))
         return S_OK;

   return S_FALSE;
}

STDMETHODIMP Kicker::CreateSizedBallWithMass(/*[in]*/ float radius, /*[in]*/ float mass, /*out, retval]*/ IBall **pResult)
{
   if (m_phitkickercircle)
   {
      const float height = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y);

      HitBall *const pball = g_pplayer->CreateBall(m_phitkickercircle->center.x,
         m_phitkickercircle->center.y, height, 0.1f, 0, 0, radius, mass);

      *pResult = pball->m_pBall;
      pball->m_pBall->AddRef();

      pball->m_coll.m_hitflag = true;           // HACK: avoid capture leaving kicker
      const Vertex3Ds hitnormal(FLT_MAX, FLT_MAX, FLT_MAX); // unused due to newBall being true
      m_phitkickercircle->DoCollide(pball, hitnormal, false, true);
   }

   return S_OK;
}

STDMETHODIMP Kicker::CreateSizedBall(/*[in]*/ float radius, /*out, retval]*/ IBall **pResult)
{
   if (m_phitkickercircle)
   {
      const float height = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y);

      HitBall *const pball = g_pplayer->CreateBall(m_phitkickercircle->center.x,
         m_phitkickercircle->center.y, height, 0.1f, 0, 0, radius);

      *pResult = pball->m_pBall;
      pball->m_pBall->AddRef();

      pball->m_coll.m_hitflag = true;           // HACK: avoid capture leaving kicker
      const Vertex3Ds hitnormal(FLT_MAX, FLT_MAX, FLT_MAX); // unused due to newBall being true
      m_phitkickercircle->DoCollide(pball, hitnormal, false, true);
   }

   return S_OK;
}

STDMETHODIMP Kicker::CreateBall(IBall **pResult)
{
   if (m_phitkickercircle)
   {
      const float height = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y);

      HitBall *const pball = g_pplayer->CreateBall(m_phitkickercircle->center.x, m_phitkickercircle->center.y, height, 0.1f, 0, 0);

      *pResult = pball->m_pBall;
      pball->m_pBall->AddRef();

      pball->m_coll.m_hitflag = true;           // HACK: avoid capture leaving kicker
      const Vertex3Ds hitnormal(FLT_MAX, FLT_MAX, FLT_MAX); // unused due to newBall being true
      m_phitkickercircle->DoCollide(pball, hitnormal, false, true);
   }

   return S_OK;
}


STDMETHODIMP Kicker::DestroyBall(int *pVal)
{
   int cnt = 0;

   if (m_phitkickercircle && m_phitkickercircle->m_pHitBall)
   {
      _ASSERTE(g_pplayer);
      ++cnt;
      HitBall *const b = m_phitkickercircle->m_pHitBall;
      m_phitkickercircle->m_pHitBall = nullptr;
      g_pplayer->DestroyBall(b);
   }

   if (pVal) *pVal = cnt;
   return S_OK;
}

// Ported at: VisualPinball.Unity/VisualPinball.Unity/VPT/Ball/BallManager.cs

STDMETHODIMP Kicker::KickXYZ(float angle, float speed, float inclination, float x, float y, float z)
{
   if (g_pplayer && m_phitkickercircle && m_phitkickercircle->m_pHitBall)
   {
	   if (g_pplayer->m_pactiveballBC == nullptr)
	   {
		   // Ball control most recently kicked if none currently.  
		   g_pplayer->m_pactiveballBC = m_phitkickercircle->m_pHitBall;
	   }
	   if (g_pplayer->m_pactiveballBC == m_phitkickercircle->m_pHitBall)
	   {
		  // Clear any existing ball control target to allow kickout to work correctly.
		   delete g_pplayer->m_pBCTarget;
		   g_pplayer->m_pBCTarget = nullptr;
	   }
      float anglerad = ANGTORAD(angle);					// yaw angle, zero is along -Y axis

      if (fabsf(inclination) > (float)(M_PI / 2.0))		// radians or degrees?  if greater PI/2 assume degrees
         inclination *= (float)(M_PI / 180.0);			// convert to radians

      float scatterAngle = (m_d.m_scatter < 0.0f) ? c_hardScatter : ANGTORAD(m_d.m_scatter); // if < 0 use global value
      scatterAngle *= m_ptable->m_globalDifficulty;		// apply dificulty weighting

      if (scatterAngle > 1.0e-5f)						// ignore near zero angles
      {
         float scatter = rand_mt_m11();					// -1.0f..1.0f
         scatter *= (1.0f - scatter*scatter)*2.59808f * scatterAngle;// shape quadratic distribution and scale
         anglerad += scatter;
      }

      const float speedz = sinf(inclination) * speed;
      if (speedz > 0.0f)
         speed *= cosf(inclination);

      m_phitkickercircle->m_pHitBall->m_angularmomentum.SetZero();
      m_phitkickercircle->m_pHitBall->m_coll.m_hitdistance = 0.0f;
      m_phitkickercircle->m_pHitBall->m_coll.m_hittime = -1.0f;
      m_phitkickercircle->m_pHitBall->m_coll.m_hitnormal.SetZero();
      m_phitkickercircle->m_pHitBall->m_coll.m_hitvel.SetZero();
      m_phitkickercircle->m_pHitBall->m_coll.m_hitflag = false;
      m_phitkickercircle->m_pHitBall->m_coll.m_isContact = false;
      m_phitkickercircle->m_pHitBall->m_coll.m_hitmoment_bit = true;
      m_phitkickercircle->m_pHitBall->m_d.m_pos.x += x; // brian's suggestion
      m_phitkickercircle->m_pHitBall->m_d.m_pos.y += y;
      m_phitkickercircle->m_pHitBall->m_d.m_pos.z += z;
      m_phitkickercircle->m_pHitBall->m_d.m_vel.x =  sinf(anglerad) * speed;
      m_phitkickercircle->m_pHitBall->m_d.m_vel.y = -cosf(anglerad) * speed;
      m_phitkickercircle->m_pHitBall->m_d.m_vel.z = speedz;
      m_phitkickercircle->m_pHitBall->m_d.m_lockedInKicker = false;
#ifdef C_DYNAMIC
      m_phitkickercircle->m_pHitBall->m_dynamic = C_DYNAMIC;
#endif
      m_phitkickercircle->m_pHitBall = nullptr;
   }

   return S_OK;
}

STDMETHODIMP Kicker::KickZ(float angle, float speed, float inclination, float heightz)
{
   KickXYZ(angle, speed, inclination, 0, 0, heightz);
   return S_OK;
}

STDMETHODIMP Kicker::Kick(float angle, float speed, float inclination)
{
   KickXYZ(angle, speed, inclination, 0, 0, 0);
   return S_OK;
}

STDMETHODIMP Kicker::get_X(float *pVal)
{
   *pVal = m_d.m_vCenter.x;
   return S_OK;
}

STDMETHODIMP Kicker::put_X(float newVal)
{
   STARTUNDO
   m_d.m_vCenter.x = newVal;
   STOPUNDO

   return S_OK;
}

STDMETHODIMP Kicker::get_Y(float *pVal)
{
   *pVal = m_d.m_vCenter.y;
   return S_OK;
}

STDMETHODIMP Kicker::put_Y(float newVal)
{
   STARTUNDO
   m_d.m_vCenter.y = newVal;
   STOPUNDO

   return S_OK;
}

STDMETHODIMP Kicker::get_Surface(BSTR *pVal)
{
   WCHAR wz[MAXTOKEN];
   MultiByteToWideCharNull(CP_ACP, 0, m_d.m_szSurface.c_str(), -1, wz, MAXTOKEN);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP Kicker::put_Surface(BSTR newVal)
{
   char buf[MAXTOKEN];
   WideCharToMultiByteNull(CP_ACP, 0, newVal, -1, buf, MAXTOKEN, nullptr, nullptr);
   STARTUNDO
   m_d.m_szSurface = buf;
   STOPUNDO

   return S_OK;
}

STDMETHODIMP Kicker::get_Enabled(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_d.m_enabled);
   return S_OK;
}

STDMETHODIMP Kicker::put_Enabled(VARIANT_BOOL newVal)
{
   STARTUNDO
   m_d.m_enabled = VBTOb(newVal);
   if (m_phitkickercircle)
      m_phitkickercircle->m_enabled = m_d.m_enabled;
   STOPUNDO

   return S_OK;
}

STDMETHODIMP Kicker::get_Scatter(float *pVal)
{
   *pVal = m_d.m_scatter;
   return S_OK;
}

STDMETHODIMP Kicker::put_Scatter(float newVal)
{
   STARTUNDO
   m_d.m_scatter = newVal;
   STOPUNDO

   return S_OK;
}

STDMETHODIMP Kicker::get_HitAccuracy(float *pVal)
{
   *pVal = m_d.m_hitAccuracy;
   return S_OK;
}

STDMETHODIMP Kicker::put_HitAccuracy(float newVal)
{
   STARTUNDO
   m_d.m_hitAccuracy = clamp(newVal, 0.f, 1.f);
   STOPUNDO

   return S_OK;
}

STDMETHODIMP Kicker::get_HitHeight(float *pVal)
{
   *pVal = m_d.m_hit_height;
   return S_OK;
}

STDMETHODIMP Kicker::put_HitHeight(float newVal)
{
   STARTUNDO
   m_d.m_hit_height = newVal;
   STOPUNDO

   return S_OK;
}

STDMETHODIMP Kicker::get_Orientation(float *pVal)
{
   *pVal = m_d.m_orientation;
   return S_OK;
}

STDMETHODIMP Kicker::put_Orientation(float newVal)
{
   STARTUNDO
   m_d.m_orientation = newVal;
   STOPUNDO

   return S_OK;
}

STDMETHODIMP Kicker::get_Radius(float *pVal)
{
   *pVal = m_d.m_radius;
   return S_OK;
}

STDMETHODIMP Kicker::put_Radius(float newVal)
{
   STARTUNDO
   m_d.m_radius = newVal;
   STOPUNDO

   return S_OK;
}

STDMETHODIMP Kicker::get_FallThrough(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_d.m_fallThrough);
   return S_OK;
}

STDMETHODIMP Kicker::put_FallThrough(VARIANT_BOOL newVal)
{
   STARTUNDO
   m_d.m_fallThrough = VBTOb(newVal);
   STOPUNDO

   return S_OK;
}

STDMETHODIMP Kicker::get_Legacy(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_d.m_legacyMode);
   return S_OK;
}

STDMETHODIMP Kicker::put_Legacy(VARIANT_BOOL newVal)
{
   STARTUNDO
   m_d.m_legacyMode = VBTOb(newVal);
   STOPUNDO

   return S_OK;
}


STDMETHODIMP Kicker::get_DrawStyle(KickerType *pVal)
{
   *pVal = m_d.m_kickertype;
   return S_OK;
}

STDMETHODIMP Kicker::put_DrawStyle(KickerType newVal)
{
   STARTUNDO
   m_d.m_kickertype = newVal;
   // legacy handling:
   if (m_d.m_kickertype > KickerCup2)
	   m_d.m_kickertype = KickerInvisible;
   STOPUNDO

   return S_OK;
}

STDMETHODIMP Kicker::get_Material(BSTR *pVal)
{
   WCHAR wz[MAXNAMEBUFFER];
   MultiByteToWideCharNull(CP_ACP, 0, m_d.m_szMaterial.c_str(), -1, wz, MAXNAMEBUFFER);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP Kicker::put_Material(BSTR newVal)
{
   char buf[MAXNAMEBUFFER];
   WideCharToMultiByteNull(CP_ACP, 0, newVal, -1, buf, MAXNAMEBUFFER, nullptr, nullptr);

   STARTUNDO
   m_d.m_szMaterial = buf;
   STOPUNDO

   return S_OK;
}

STDMETHODIMP Kicker::BallCntOver(int *pVal)
{
   int cnt = 0;

   if (g_pplayer)
   {
      for (auto pball : g_pplayer->m_vball)
      {
         if (pball->m_d.m_vpVolObjs && FindIndexOf(*(pball->m_d.m_vpVolObjs), (IFireEvents*)this) >= 0) // cast to IFireEvents necessary, as it is stored like this in HitObject.m_obj
         {
            ++cnt;
            g_pplayer->m_pactiveball = pball; // set active ball for scriptor
         }
      }
   }

   *pVal = cnt;
   return S_OK;
}

STDMETHODIMP Kicker::get_LastCapturedBall(IBall **pVal)
{
    if (!pVal || !g_pplayer || !m_phitkickercircle)
    {
        return E_POINTER;
    }

    if (!m_phitkickercircle->m_lastCapturedBall)
    {
        ShowError("LastCapturedBall was called but no ball was captured!");
        return E_POINTER;
    }

    bool ballFound = false;
    for (auto ball : g_pplayer->m_vball)
    {
        if (ball == m_phitkickercircle->m_lastCapturedBall)
        {
            ballFound = true;
            break;
        }
    }

    if (!ballFound)
    {
        ShowError("LastCapturedBall was called but ball is already destroyed!");
        return E_POINTER;
    }

    Ball * const pBall = m_phitkickercircle->m_lastCapturedBall->m_pBall;

    if (!pBall)
    {
        return E_POINTER;
    }
    pBall->QueryInterface(IID_IBall, (void **)pVal);

    return S_OK;
}

float KickerHitCircle::HitTest(const BallS& ball, const float dtime, CollisionEvent& coll) const
{
   return HitTestBasicRadius(ball, dtime, coll, false, false, false); //any face, not-lateral, non-rigid
}

// Ported at: VisualPinball.Unity/VisualPinball.Unity/VPT/Kicker/KickerCollider.cs

void KickerHitCircle::DoChangeBallVelocity(HitBall *const pball, const Vertex3Ds &hitnormal) const
{
    float minDist_sqr = FLT_MAX;
    unsigned int idx = ~0u;
    for (size_t t = 0; t < m_pkicker->m_hitMesh.size(); t++)
    {
        // find the right normal by calculating the distance from current ball position to vertex of the kicker mesh               
        const float length_sqr = (pball->m_d.m_pos - m_pkicker->m_hitMesh[t]).LengthSquared();
        if (length_sqr < minDist_sqr)
        {
            minDist_sqr = length_sqr;
            idx = (unsigned int)t;
        }
    }
    //minDist_sqr = sqrtf(minDist_sqr);

    if (idx != ~0u)
    {
        // we have the nearest vertex now use the normal and damp it so it doesn't speed up the ball velocity too much
        const Vertex3Ds hitnorm(kickerHitMesh[idx].nx, kickerHitMesh[idx].ny, kickerHitMesh[idx].nz);
        Vertex3Ds surfVel, tangent, surfP;
        const float dot = -pball->m_d.m_vel.Dot(hitnorm);
        const float reactionImpulse = pball->m_d.m_mass * fabsf(dot);
        /*
        if (pball->m_pos.z > pball->m_radius)
        {
        // ball is on a surface(e.g. upper playfield) use a high friction and a different calculation to compensate surface collision
        friction = 1.0f;
        surfP = -pball->m_radius * hitnorm;      // surface contact point relative to center of mass

        surfVel = pball->SurfaceVelocity(surfP); // velocity at impact point

        tangent = surfVel - surfVel.Dot(hitnorm) * hitnorm; // calc the tangential velocity
        }
        else
        */
        surfP = -pball->m_d.m_radius * hitnormal;    // surface contact point relative to center of mass

        surfVel = pball->SurfaceVelocity(surfP);         // velocity at impact point

        tangent = surfVel - surfVel.Dot(hitnormal) * hitnorm; // calc the tangential velocity

        pball->m_d.m_vel += dot * hitnorm; // apply collision impulse (along normal, so no torque)
#ifdef C_DYNAMIC
        pball->m_dynamic = C_DYNAMIC;
#endif

        constexpr float friction = 0.3f;
        const float tangentSpSq = tangent.LengthSquared();

        if (tangentSpSq > 1e-6f)
        {
            tangent /= sqrtf(tangentSpSq);           // normalize to get tangent direction
            const float vt = surfVel.Dot(tangent);   // get speed in tangential direction

            // compute friction impulse
            const Vertex3Ds cross = CrossProduct(surfP, tangent);
            const float kt = 1.0f/pball->m_d.m_mass + tangent.Dot(CrossProduct(cross / pball->Inertia(), surfP));

            // friction impulse can't be greater than coefficient of friction times collision impulse (Coulomb friction cone)
            const float maxFric = friction * reactionImpulse;
            const float jt = clamp(-vt / kt, -maxFric, maxFric);

            pball->ApplySurfaceImpulse(jt * cross, jt * tangent);
        }
    }
}

void KickerHitCircle::DoCollide(HitBall *const pball, const Vertex3Ds &hitnormal, const bool hitbit, const bool newBall)
{
   if (m_pHitBall) return;                              // a previous ball already in kicker

   const int i = FindIndexOf(*(pball->m_d.m_vpVolObjs), m_obj); // check if kicker in ball's volume set

   if (newBall || ((!hitbit) == (i < 0)))            // New or (Hit && !Vol || UnHit && Vol)
   {
      if (m_pkicker->m_d.m_legacyMode || newBall)
      {
         pball->m_d.m_pos += STATICTIME * pball->m_d.m_vel; // move ball slightly forward
      }

      if (i < 0) // entering Kickers volume
      {
         const float grabHeight = (m_hitBBox.zlow + pball->m_d.m_radius) * m_pkicker->m_d.m_hitAccuracy;
         // early out here if the ball is slow and we are near the kicker center
         const bool hitEvent = (pball->m_d.m_pos.z < grabHeight || m_pkicker->m_d.m_legacyMode || newBall);

         if(!hitEvent)
         {
            DoChangeBallVelocity(pball, hitnormal);

            // this is an ugly hack to prevent the ball stopping rapidly at the kicker bevel
            // something with the friction calculation is wrong in the physics engine
            // so we monitor the ball velocity if it drop under a length value of 0.2
            // if so we take the last "good" velocity to help the ball moving over the critical spot at the kicker bevel
            // this hack seems to work only if the kicker is on the playfield, a kicker attached to a wall has still problems
            // because the friction calculation for a wall is also different
            if (pball->m_d.m_vel.LengthSquared() < (float)(0.2*0.2))
               pball->m_d.m_vel = pball->m_oldVel;

            pball->m_oldVel = pball->m_d.m_vel;
         }

         if (hitEvent)
         {
            pball->m_d.m_lockedInKicker = !m_pkicker->m_d.m_fallThrough;
            if (pball->m_d.m_lockedInKicker)
            {
               pball->m_d.m_vpVolObjs->push_back(m_obj);		// add kicker to ball's volume set
               m_pHitBall = pball;
               m_lastCapturedBall = pball;
               if (pball == g_pplayer->m_pactiveballBC)
                  g_pplayer->m_pactiveballBC = nullptr;
            }

            // Don't fire the hit event if the ball was just created
            // Fire the event before changing ball attributes, so scripters can get a useful ball state
            if (!newBall)
               m_pkicker->FireGroupEvent(DISPID_HitEvents_Hit);

            if (pball->m_d.m_lockedInKicker || m_pkicker->m_d.m_fallThrough)	// script may have unfrozen the ball
            {
               // if ball falls through hole, we fake the collision algo by changing the ball height
               // in HitTestBasicRadius() the z-position of the ball is checked if it is >= to the hit cylinder
               // if we don't change the height of the ball we get a lot of hit events while the ball is falling!!

               // Only mess with variables if ball was not kicked during event
               pball->m_d.m_vel.SetZero();
               pball->m_angularmomentum.SetZero();
               pball->m_d.m_pos.x = center.x;
               pball->m_d.m_pos.y = center.y;
#ifdef C_DYNAMIC
               pball->m_dynamic = 0;
#endif
               if (m_pkicker->m_d.m_fallThrough)
                  pball->m_d.m_pos.z = m_hitBBox.zlow - pball->m_d.m_radius - 5.0f;
               else
                  pball->m_d.m_pos.z = m_hitBBox.zlow + pball->m_d.m_radius/**pball->m_radius/radius*/;
            }
            else
               m_pHitBall = nullptr; // make sure
         }
      }
      else // exiting kickers volume
      {
         pball->m_d.m_vpVolObjs->erase(pball->m_d.m_vpVolObjs->begin() + i); // remove kicker to ball's volume set
         m_pkicker->FireGroupEvent(DISPID_HitEvents_Unhit);
      }
   }
}
