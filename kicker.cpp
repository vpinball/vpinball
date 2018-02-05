#include "StdAfx.h"
#include "objloader.h"

#include "meshes/kickerCupMesh.h"
#include "meshes/kickerHoleMesh.h"
#include "meshes/kickerHitMesh.h"
#include "meshes/kickerSimpleHoleMesh.h"
#include "meshes/kickerWilliams.h"
#include "meshes/kickerGottlieb.h"
#include "meshes/kickerT1Mesh.h"

Kicker::Kicker()
{
   m_phitkickercircle = NULL;
   vertexBuffer = NULL;
   indexBuffer = NULL;
   plateVertexBuffer = NULL;
   plateIndexBuffer = NULL;
   memset(m_d.m_szMaterial, 0, 32);
   memset(m_d.m_szSurface, 0, MAXTOKEN);
   m_ptable = NULL;
   m_numVertices = 0;
   m_numIndices = 0;
   m_baseHeight = 0.0f;
}

Kicker::~Kicker()
{
   if (vertexBuffer)
   {
      vertexBuffer->release();
      vertexBuffer = 0;
   }
   if (indexBuffer)
   {
      indexBuffer->release();
      indexBuffer = 0;
   }

   if (plateVertexBuffer)
   {
      plateVertexBuffer->release();
      plateVertexBuffer = 0;
   }
   if (plateIndexBuffer)
   {
      plateIndexBuffer->release();
      plateIndexBuffer = 0;
   }
}

void Kicker::UpdateUnitsInfo()
{
   char tbuf[128];
   sprintf_s(tbuf, "Radius: %.3f", g_pvp->ConvertToUnit(m_d.m_radius));
   g_pvp->SetStatusBarUnitInfo(tbuf);
}

HRESULT Kicker::Init(PinTable *ptable, float x, float y, bool fromMouseClick)
{
   m_ptable = ptable;

   m_d.m_vCenter.x = x;
   m_d.m_vCenter.y = y;

   SetDefaults(fromMouseClick);

   return InitVBA(fTrue, 0, NULL);
}

void Kicker::SetDefaults(bool fromMouseClick)
{
   HRESULT hr;
   float fTmp;
   int iTmp;

   hr = GetRegStringAsFloat("DefaultProps\\Kicker", "Radius", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_radius = fTmp;
   else
      m_d.m_radius = 25;

   hr = GetRegInt("DefaultProps\\Kicker", "TimerEnabled", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_tdr.m_fTimerEnabled = iTmp == 0 ? false : true;
   else
      m_d.m_tdr.m_fTimerEnabled = false;

   hr = GetRegInt("DefaultProps\\Kicker", "TimerInterval", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_tdr.m_TimerInterval = iTmp;
   else
      m_d.m_tdr.m_TimerInterval = 100;

   hr = GetRegInt("DefaultProps\\Kicker", "Enabled", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_fEnabled = iTmp == 0 ? false : true;
   else
      m_d.m_fEnabled = true;


   hr = GetRegStringAsFloat("DefaultProps\\Kicker", "HitAccuracy", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_hitAccuracy = fTmp;
   else
      m_d.m_hitAccuracy = 0.7f;

   hr = GetRegStringAsFloat("DefaultProps\\Kicker", "HitHeight", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_hit_height = fTmp;
   else
      m_d.m_hit_height = 40.0f;

   hr = GetRegStringAsFloat("DefaultProps\\Kicker", "Orientation", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_orientation = fTmp;
   else
      m_d.m_orientation = 0.0f;

   SetDefaultPhysics(fromMouseClick);

   hr = GetRegString("DefaultProps\\Kicker", "Surface", &m_d.m_szSurface, MAXTOKEN);
   if ((hr != S_OK) || !fromMouseClick)
      m_d.m_szSurface[0] = 0;

   hr = GetRegInt("DefaultProps\\Kicker", "KickerType", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_kickertype = (KickerType)iTmp;
   else
      m_d.m_kickertype = KickerHole;

   //legacy handling:
   if (m_d.m_kickertype > KickerCup2)
	   m_d.m_kickertype = KickerInvisible;

   hr = GetRegInt("DefaultProps\\Kicker", "FallThrough", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_fFallThrough = iTmp == 0 ? false : true;
   else
      m_d.m_fFallThrough = false;

   hr = GetRegInt("DefaultProps\\Kicker", "Legacy", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_legacyMode = iTmp == 0 ? false : true;
   else
      m_d.m_legacyMode = true;
}

void Kicker::WriteRegDefaults()
{
   SetRegValueBool("DefaultProps\\Kicker", "TimerEnabled", m_d.m_tdr.m_fTimerEnabled);
   SetRegValue("DefaultProps\\Kicker", "TimerInterval", REG_DWORD, &m_d.m_tdr.m_TimerInterval, 4);
   SetRegValueBool("DefaultProps\\Kicker", "Enabled", m_d.m_fEnabled);
   SetRegValueFloat("DefaultProps\\Kicker", "HitAccuracy", m_d.m_hitAccuracy);
   SetRegValueFloat("DefaultProps\\Kicker", "HitHeight", m_d.m_hit_height);
   SetRegValueFloat("DefaultProps\\Kicker", "Orientation", m_d.m_orientation);
   SetRegValueFloat("DefaultProps\\Kicker", "Radius", m_d.m_radius);
   SetRegValueFloat("DefaultProps\\Kicker", "Scatter", m_d.m_scatter);
   SetRegValue("DefaultProps\\Kicker", "KickerType", REG_DWORD, &m_d.m_kickertype, 4);
   SetRegValue("DefaultProps\\Kicker", "Surface", REG_SZ, &m_d.m_szSurface, lstrlen(m_d.m_szSurface));
   SetRegValueBool("DefaultProps\\Kicker", "FallThrough", m_d.m_fFallThrough);
   SetRegValueBool("DefaultProps\\Kicker", "Legacy", m_d.m_legacyMode);
}

void Kicker::PreRender(Sur * const psur)
{
}

void Kicker::Render(Sur * const psur)
{
   psur->SetBorderColor(RGB(0, 0, 0), false, 0);
   psur->SetFillColor(-1);
   psur->SetObject(this);

   psur->Ellipse(m_d.m_vCenter.x, m_d.m_vCenter.y, m_d.m_radius);
   psur->Ellipse(m_d.m_vCenter.x, m_d.m_vCenter.y, m_d.m_radius*0.75f);
   psur->Ellipse(m_d.m_vCenter.x, m_d.m_vCenter.y, m_d.m_radius*0.5f);
   psur->Ellipse(m_d.m_vCenter.x, m_d.m_vCenter.y, m_d.m_radius*0.25f);
}

void Kicker::GetTimers(Vector<HitTimer> * const pvht)
{
   IEditable::BeginPlay();

   HitTimer * const pht = new HitTimer();
   pht->m_interval = m_d.m_tdr.m_TimerInterval >= 0 ? max(m_d.m_tdr.m_TimerInterval, MAX_TIMER_MSEC_INTERVAL) : -1;
   pht->m_nextfire = pht->m_interval;
   pht->m_pfe = (IFireEvents *)this;

   m_phittimer = pht;

   if (m_d.m_tdr.m_fTimerEnabled)
   {
      pvht->AddElement(pht);
   }
}

void Kicker::GetHitShapes(Vector<HitObject> * const pvho)
{
   const float height = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y) * m_ptable->m_BG_scalez[m_ptable->m_BG_current_set];

   KickerHitCircle * const phitcircle = new KickerHitCircle(m_d.m_vCenter, m_d.m_radius *
      (m_d.m_legacyMode ? (m_d.m_fFallThrough ? 0.75f :
                                                0.6f) // reduce the hit circle radius because only the inner circle of the kicker should start a hit event
                                              : 1.f),
      height, height+m_d.m_hit_height); // height of kicker hit cylinder

   if (!m_d.m_legacyMode)
   {
      const float rad = phitcircle->radius * 0.8f;
      hitMesh.resize(kickerHitNumVertices);
      for (unsigned int t = 0; t < kickerHitNumVertices; t++)
      {
         // find the right normal by calculating the distance from current ball position to vertex of the kicker mesh               
         Vertex3Ds vpos = Vertex3Ds(kickerHitMesh[t].x, kickerHitMesh[t].y, kickerHitMesh[t].z);
         vpos.x = vpos.x*rad + m_d.m_vCenter.x;
         vpos.y = vpos.y*rad + m_d.m_vCenter.y;
         vpos.z = vpos.z*rad * m_ptable->m_BG_scalez[m_ptable->m_BG_current_set] + height;
         hitMesh[t] = vpos;
      }
   }

   phitcircle->m_fEnabled = m_d.m_fEnabled;

   phitcircle->m_ObjType = eKicker;
   phitcircle->m_obj = (IFireEvents*) this;

   phitcircle->m_pkicker = this;

   pvho->AddElement(phitcircle);

   m_phitkickercircle = phitcircle;
}

void Kicker::GetHitShapesDebug(Vector<HitObject> * const pvho)
{
}

void Kicker::EndPlay()
{
   m_phitkickercircle = NULL;
   if (vertexBuffer)
   {
      vertexBuffer->release();
      vertexBuffer = 0;
   }
   if (indexBuffer)
   {
      indexBuffer->release();
      indexBuffer = 0;
   }

   if (plateVertexBuffer)
   {
      plateVertexBuffer->release();
      plateVertexBuffer = 0;
   }
   if (plateIndexBuffer)
   {
      plateIndexBuffer->release();
      plateIndexBuffer = 0;
   }

   hitMesh.clear();

   IEditable::EndPlay();
}

void Kicker::RenderStatic(RenderDevice* pd3dDevice)
{
}

void Kicker::ExportMesh(FILE *f)
{
   if(m_d.m_kickertype == KickerInvisible)
       return;

   char name[MAX_PATH];
   WideCharToMultiByte(CP_ACP, 0, m_wzName, -1, name, MAX_PATH, NULL, NULL);
   m_baseHeight = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y) * m_ptable->m_BG_scalez[m_ptable->m_BG_current_set];

   int num_vertices;
   int num_indices;
   const WORD *indices;
   switch (m_d.m_kickertype)
   {
       case KickerInvisible:
           assert(false);
       break;
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
   WaveFrontObj_WriteObjectName(f, name);
   WaveFrontObj_WriteVertexInfo(f, vertices, num_vertices);
   const Material * const mat = m_ptable->GetMaterial(m_d.m_szMaterial);
   WaveFrontObj_WriteMaterial(m_d.m_szMaterial, NULL, mat);
   WaveFrontObj_UseTexture(f, m_d.m_szMaterial);
   WaveFrontObj_WriteFaceInfoList(f, indices, num_indices);
   WaveFrontObj_UpdateFaceOffset(num_vertices);
   delete[] vertices;
}

void Kicker::GenerateMesh(Vertex3D_NoTex2 *const buf)
{
   Matrix3D fullMatrix;
   fullMatrix.RotateZMatrix(ANGTORAD(m_d.m_orientation));

   int num_vertices;
   const Vertex3D_NoTex2 *vertices;
   switch (m_d.m_kickertype)
   {
       case KickerInvisible:
           assert(false);
       break;
       case KickerCup:
       {
           num_vertices = kickerCupNumVertices;
           vertices = kickerCupMesh;
       }
       break;
       case KickerWilliams:
       {
           num_vertices = kickerWilliamsNumVertices;
           vertices = kickerWilliamsMesh;
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
       }
       break;
       case KickerHoleSimple:
       {
           num_vertices = kickerSimpleHoleNumVertices;
           vertices = kickerSimpleHoleMesh;
       }
       break;
   }

   for (int i = 0; i < num_vertices; i++)
   {
      Vertex3Ds vert(vertices[i].x, vertices[i].y, vertices[i].z - 0.18f);
      vert = fullMatrix.MultiplyVector(vert);

      buf[i].x = vert.x*m_d.m_radius + m_d.m_vCenter.x;
      buf[i].y = vert.y*m_d.m_radius + m_d.m_vCenter.y;
      buf[i].z = vert.z*m_d.m_radius*m_ptable->m_BG_scalez[m_ptable->m_BG_current_set] + m_baseHeight;
      vert = Vertex3Ds(vertices[i].nx, vertices[i].ny, vertices[i].nz);
      vert = fullMatrix.MultiplyVectorNoTranslate(vert);
      buf[i].nx = vert.x;
      buf[i].ny = vert.y;
      buf[i].nz = vert.z;
      buf[i].tu = vertices[i].tu;
      buf[i].tv = vertices[i].tv;
   }
}

void Kicker::RenderSetup(RenderDevice* pd3dDevice)
{
   if (m_d.m_kickertype == KickerInvisible)
      return;

   Pin3D * const ppin3d = &g_pplayer->m_pin3d;
   m_baseHeight = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y) * m_ptable->m_BG_scalez[m_ptable->m_BG_current_set];

   if (m_d.m_kickertype == KickerCup || m_d.m_kickertype == KickerHole || m_d.m_kickertype == KickerHoleSimple || m_d.m_kickertype==KickerWilliams || m_d.m_kickertype==KickerGottlieb || m_d.m_kickertype==KickerCup2)
   {
      Vertex3D_NoTex2 *buf = new Vertex3D_NoTex2[kickerPlateNumVertices];
      float rad = m_d.m_radius; 

      if(m_d.m_kickertype == KickerWilliams || m_d.m_kickertype == KickerGottlieb)
          rad = m_d.m_radius * 0.88f;
      else if(m_d.m_kickertype == KickerCup2)
          rad = m_d.m_radius * 0.87f;
      else if(m_d.m_kickertype != KickerCup && m_d.m_kickertype != KickerWilliams)
          rad = m_d.m_radius * 0.82f;

      for (unsigned int i = 0; i < kickerPlateNumVertices; i++)
      {
         buf[i].x = kickerPlate[i].x*rad + m_d.m_vCenter.x;
         buf[i].y = kickerPlate[i].y*rad + m_d.m_vCenter.y;
         buf[i].z = kickerPlate[i].z*rad*m_ptable->m_BG_scalez[m_ptable->m_BG_current_set] + m_baseHeight;
         buf[i].nx = kickerPlate[i].nx;
         buf[i].ny = kickerPlate[i].ny;
         buf[i].nz = kickerPlate[i].nz;
         buf[i].tu = 0.0f;
         buf[i].tv = 0.0f;
      }

      if (plateIndexBuffer)
         plateIndexBuffer->release();
      plateIndexBuffer = pd3dDevice->CreateAndFillIndexBuffer(kickerPlateNumIndices, kickerPlateIndices);

      if (plateVertexBuffer)
         plateVertexBuffer->release();
      pd3dDevice->CreateVertexBuffer(kickerPlateNumVertices, 0, MY_D3DFVF_NOTEX2_VERTEX, &plateVertexBuffer);

      Vertex3D_NoTex2 *bufvb;
      plateVertexBuffer->lock(0, 0, (void**)&bufvb, VertexBuffer::WRITEONLY);
      memcpy(bufvb,buf,kickerPlateNumVertices*sizeof(Vertex3D_NoTex2));
      plateVertexBuffer->unlock();

      delete[] buf;
   }

   //

   const WORD * indices;
   switch(m_d.m_kickertype)
   {
       case KickerInvisible:
          assert(false);
       break;
       case KickerCup:
       {
          texture.CreateFromResource(IDB_KICKER_CUP);

          m_numIndices = kickerCupNumIndices;
          m_numVertices = kickerCupNumVertices;

          indices = kickerCupIndices;
       }
       break;
       case KickerWilliams:
       {
           texture.CreateFromResource(IDB_KICKER_WILLIAMS);

           m_numIndices = kickerWilliamsNumIndices;
           m_numVertices = kickerWilliamsNumVertices;

           indices = kickerWilliamsIndices;
       }
       break;
       case KickerGottlieb:
       {
           texture.CreateFromResource(IDB_KICKER_GOTTLIEB);

           m_numIndices = kickerGottliebNumIndices;
           m_numVertices = kickerGottliebNumVertices;

           indices = kickerGottliebIndices;
       }
       break;
       case KickerCup2:
       {
           texture.CreateFromResource(IDB_KICKER_T1);

           m_numIndices = kickerT1NumIndices;
           m_numVertices = kickerT1NumVertices;

           indices = kickerT1Indices;
       }
       break;
       case KickerHole:
       {
          texture.CreateFromResource(IDB_KICKER_HOLE_WOOD);

          m_numIndices = kickerHoleNumIndices;
          m_numVertices = kickerHoleNumVertices;

          indices = kickerHoleIndices;
       }
       break;
       default:
       case KickerHoleSimple:
       {
          texture.CreateFromResource(IDB_KICKER_HOLE_WOOD);

          m_numIndices = kickerSimpleHoleNumIndices;
          m_numVertices = kickerSimpleHoleNumVertices;

          indices = kickerSimpleHoleIndices;
       }
       break;
   }

   //

   if (indexBuffer)
       indexBuffer->release();
   indexBuffer = pd3dDevice->CreateAndFillIndexBuffer(m_numIndices, indices);

   if (vertexBuffer)
       vertexBuffer->release();
   pd3dDevice->CreateVertexBuffer(m_numVertices, 0, MY_D3DFVF_NOTEX2_VERTEX, &vertexBuffer);

   Vertex3D_NoTex2 *buf;
   vertexBuffer->lock(0, 0, (void**)&buf, VertexBuffer::WRITEONLY);
   GenerateMesh(buf);
   vertexBuffer->unlock();
}


void Kicker::PreRenderStatic(RenderDevice* pd3dDevice)
{
}

void Kicker::SetDefaultPhysics(bool fromMouseClick)
{
   HRESULT hr;
   float fTmp;

   hr = GetRegStringAsFloat("DefaultProps\\Kicker", "Scatter", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_scatter = fTmp;
   else
      m_d.m_scatter = 0;
}

void Kicker::PostRenderStatic(RenderDevice* pd3dDevice)
{
   if (m_ptable->m_fReflectionEnabled)
      return;

   if(m_d.m_kickertype == KickerCup || m_d.m_kickertype == KickerHole || m_d.m_kickertype == KickerHoleSimple || m_d.m_kickertype == KickerWilliams || m_d.m_kickertype == KickerGottlieb || m_d.m_kickertype == KickerCup2)
   {
      pd3dDevice->SetRenderState(RenderDevice::DEPTHBIAS, 0);
      pd3dDevice->SetRenderState(RenderDevice::ZWRITEENABLE, TRUE);
      if ( m_d.m_kickertype!=KickerHoleSimple)
         pd3dDevice->SetRenderState(RenderDevice::CULLMODE, D3DCULL_CCW);
      else
         pd3dDevice->SetRenderState(RenderDevice::CULLMODE, D3DCULL_NONE);


      const Material * const mat = m_ptable->GetMaterial(m_d.m_szMaterial);
      pd3dDevice->basicShader->SetMaterial(mat);

      pd3dDevice->basicShader->SetTechnique(mat->m_bIsMetal ? "kickerBoolean_isMetal" : "kickerBoolean_isNotMetal");
      pd3dDevice->basicShader->Core()->SetFloat("fKickerScale", m_ptable->m_BG_scalez[m_ptable->m_BG_current_set]);
      pd3dDevice->SetRenderState(RenderDevice::ZFUNC, D3DCMP_ALWAYS);

      pd3dDevice->basicShader->Begin(0);
      pd3dDevice->DrawIndexedPrimitiveVB(D3DPT_TRIANGLELIST, MY_D3DFVF_NOTEX2_VERTEX, plateVertexBuffer, 0, kickerPlateNumVertices, plateIndexBuffer, 0, kickerPlateNumIndices);
      pd3dDevice->basicShader->End();

      pd3dDevice->SetRenderState(RenderDevice::ZFUNC, D3DCMP_LESSEQUAL);

      if (m_d.m_kickertype != KickerHoleSimple)
      {
         pd3dDevice->basicShader->SetTechnique(mat->m_bIsMetal ? "basic_with_texture_isMetal" : "basic_with_texture_isNotMetal");
         pd3dDevice->basicShader->SetTexture("Texture0", &texture);
      }
      else
         pd3dDevice->basicShader->SetTechnique(mat->m_bIsMetal ? "basic_without_texture_isMetal" : "basic_without_texture_isNotMetal");

      g_pplayer->m_pin3d.EnableAlphaBlend(false);
      pd3dDevice->basicShader->SetAlphaTestValue(-1.0f);

      pd3dDevice->basicShader->Begin(0);
      pd3dDevice->DrawIndexedPrimitiveVB(D3DPT_TRIANGLELIST, MY_D3DFVF_NOTEX2_VERTEX, vertexBuffer, 0, m_numVertices, indexBuffer, 0, m_numIndices);
      pd3dDevice->basicShader->End();

   }
}

void Kicker::SetObjectPos()
{
   g_pvp->SetObjectPosCur(m_d.m_vCenter.x, m_d.m_vCenter.y);
}

void Kicker::MoveOffset(const float dx, const float dy)
{
   m_d.m_vCenter.x += dx;
   m_d.m_vCenter.y += dy;

   m_ptable->SetDirtyDraw();
}

void Kicker::GetCenter(Vertex2D * const pv) const
{
   *pv = m_d.m_vCenter;
}

void Kicker::PutCenter(const Vertex2D * const pv)
{
   m_d.m_vCenter = *pv;

   SetDirtyDraw();
}


HRESULT Kicker::SaveData(IStream *pstm, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
{
   BiffWriter bw(pstm, hcrypthash, hcryptkey);

   bw.WriteStruct(FID(VCEN), &m_d.m_vCenter, sizeof(Vertex2D));
   bw.WriteFloat(FID(RADI), m_d.m_radius);
   bw.WriteBool(FID(TMON), m_d.m_tdr.m_fTimerEnabled);
   bw.WriteInt(FID(TMIN), m_d.m_tdr.m_TimerInterval);
   bw.WriteString(FID(MATR), m_d.m_szMaterial);
   bw.WriteString(FID(SURF), m_d.m_szSurface);
   bw.WriteBool(FID(EBLD), m_d.m_fEnabled);
   bw.WriteWideString(FID(NAME), (WCHAR *)m_wzName);
   bw.WriteInt(FID(TYPE), m_d.m_kickertype);
   bw.WriteFloat(FID(KSCT), m_d.m_scatter);
   bw.WriteFloat(FID(KHAC), m_d.m_hitAccuracy);
   bw.WriteFloat(FID(KHHI), m_d.m_hit_height);
   bw.WriteFloat(FID(KORI), m_d.m_orientation);
   bw.WriteBool(FID(FATH), m_d.m_fFallThrough);
   bw.WriteBool(FID(LEMO), m_d.m_legacyMode);

   ISelect::SaveData(pstm, hcrypthash, hcryptkey);

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

BOOL Kicker::LoadToken(int id, BiffReader *pbr)
{
   if (id == FID(PIID))
   {
      pbr->GetInt((int *)pbr->m_pdata);
   }
   else if (id == FID(VCEN))
   {
      pbr->GetStruct(&m_d.m_vCenter, sizeof(Vertex2D));
   }
   else if (id == FID(RADI))
   {
      pbr->GetFloat(&m_d.m_radius);
   }
   else if (id == FID(KSCT))
   {
      pbr->GetFloat(&m_d.m_scatter);
   }
   else if (id == FID(KHAC))
   {
      pbr->GetFloat(&m_d.m_hitAccuracy);
   }
   else if (id == FID(KHHI))
   {
      pbr->GetFloat(&m_d.m_hit_height);
   }
   else if (id == FID(KORI))
   {
      pbr->GetFloat(&m_d.m_orientation);
   }
   else if (id == FID(MATR))
   {
      pbr->GetString(m_d.m_szMaterial);
   }
   else if (id == FID(TMON))
   {
      pbr->GetBool(&m_d.m_tdr.m_fTimerEnabled);
   }
   else if (id == FID(EBLD))
   {
      pbr->GetBool(&m_d.m_fEnabled);
   }
   else if (id == FID(TMIN))
   {
      pbr->GetInt(&m_d.m_tdr.m_TimerInterval);
   }
   else if (id == FID(TYPE))
   {
      pbr->GetInt(&m_d.m_kickertype);
	  //legacy handling:
	  if (m_d.m_kickertype > KickerCup2)
		  m_d.m_kickertype = KickerInvisible;
   }
   else if (id == FID(SURF))
   {
      pbr->GetString(m_d.m_szSurface);
   }
   else if (id == FID(NAME))
   {
      pbr->GetWideString((WCHAR *)m_wzName);
   }
   else if (id == FID(FATH))
   {
      pbr->GetBool(&m_d.m_fFallThrough);
   }
   else if (id == FID(LEMO))
   {
      pbr->GetBool(&m_d.m_legacyMode);
   }
   else
   {
      ISelect::LoadToken(id, pbr);
   }

   return fTrue;
}

HRESULT Kicker::InitPostLoad()
{
   m_phitkickercircle = NULL;
   return S_OK;
}


STDMETHODIMP Kicker::InterfaceSupportsErrorInfo(REFIID riid)
{
   static const IID* arr[] =
   {
      &IID_IKicker,
   };

   for (size_t i = 0; i < sizeof(arr) / sizeof(arr[0]); i++)
   {
      if (InlineIsEqualGUID(*arr[i], riid))
         return S_OK;
   }
   return S_FALSE;
}

STDMETHODIMP Kicker::CreateSizedBallWithMass(/*[in]*/float radius, /*[in]*/float mass, /*out, retval]*/ IBall **pBallEx)
{
   if (m_phitkickercircle)
   {
      const float height = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y);

      Ball * const pball = g_pplayer->CreateBall(m_phitkickercircle->center.x,
         m_phitkickercircle->center.y, height, 0.1f, 0, 0, radius, mass);

      *pBallEx = pball->m_pballex;
      pball->m_pballex->AddRef();

      pball->m_coll.m_hitflag = true;           // HACK: avoid capture leaving kicker
      Vertex3Ds hitnormal(FLT_MAX, FLT_MAX, FLT_MAX); // unused due to newBall being true
      m_phitkickercircle->DoCollide(pball, hitnormal, false, true);
   }

   return S_OK;
}

STDMETHODIMP Kicker::CreateSizedBall(/*[in]*/float radius, /*out, retval]*/ IBall **pBallEx)
{
   if (m_phitkickercircle)
   {
      const float height = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y);

      Ball * const pball = g_pplayer->CreateBall(m_phitkickercircle->center.x,
         m_phitkickercircle->center.y, height, 0.1f, 0, 0, radius);

      *pBallEx = pball->m_pballex;
      pball->m_pballex->AddRef();

      pball->m_coll.m_hitflag = true;           // HACK: avoid capture leaving kicker
      Vertex3Ds hitnormal(FLT_MAX, FLT_MAX, FLT_MAX); // unused due to newBall being true
      m_phitkickercircle->DoCollide(pball, hitnormal, false, true);
   }

   return S_OK;
}

STDMETHODIMP Kicker::CreateBall(IBall **pBallEx)
{
   if (m_phitkickercircle)
   {
      const float height = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y);

      Ball * const pball = g_pplayer->CreateBall(m_phitkickercircle->center.x, m_phitkickercircle->center.y, height, 0.1f, 0, 0);

      *pBallEx = pball->m_pballex;
      pball->m_pballex->AddRef();

      pball->m_coll.m_hitflag = true;           // HACK: avoid capture leaving kicker
      Vertex3Ds hitnormal(FLT_MAX, FLT_MAX, FLT_MAX); // unused due to newBall being true
      m_phitkickercircle->DoCollide(pball, hitnormal, false, true);
   }

   return S_OK;
}


STDMETHODIMP Kicker::DestroyBall(int *pVal)
{
   int cnt = 0;

   if (m_phitkickercircle && m_phitkickercircle->m_pball)
   {
      _ASSERTE(g_pplayer);
      ++cnt;
      Ball * const b = m_phitkickercircle->m_pball;
      m_phitkickercircle->m_pball = NULL;
      g_pplayer->DestroyBall(b);
   }

   if (pVal) *pVal = cnt;

   return S_OK;
}

STDMETHODIMP Kicker::KickXYZ(float angle, float speed, float inclination, float x, float y, float z)
{
   if (g_pplayer && m_phitkickercircle && m_phitkickercircle->m_pball)
   {
      float anglerad = ANGTORAD(angle);				// yaw angle, zero is along -Y axis

      if (fabsf(inclination) > (float)(M_PI / 2.0))		// radians or degrees?  if greater PI/2 assume degrees
         inclination *= (float)(M_PI / 180.0);			// convert to radians

      float scatterAngle = (m_d.m_scatter < 0.0f) ? c_hardScatter : ANGTORAD(m_d.m_scatter); // if < 0 use global value
      scatterAngle *= g_pplayer->m_ptable->m_globalDifficulty;		// apply dificulty weighting

      if (scatterAngle > 1.0e-5f)										// ignore near zero angles
      {
         float scatter = rand_mt_m11();								// -1.0f..1.0f
         scatter *= (1.0f - scatter*scatter)*2.59808f * scatterAngle;// shape quadratic distribution and scale
         anglerad += scatter;
      }

      const float speedz = sinf(inclination) * speed;
      if (speedz > 0.0f)
         speed *= cosf(inclination);

      m_phitkickercircle->m_pball->m_angularvelocity.SetZero();
      m_phitkickercircle->m_pball->m_angularmomentum.SetZero();
      m_phitkickercircle->m_pball->m_coll.m_hitdistance = 0.0f;
      m_phitkickercircle->m_pball->m_coll.m_hittime = -1.0f;
      m_phitkickercircle->m_pball->m_coll.m_hitnormal.SetZero();
      m_phitkickercircle->m_pball->m_coll.m_hitvel.SetZero();
      m_phitkickercircle->m_pball->m_coll.m_hitflag = false;
      m_phitkickercircle->m_pball->m_coll.m_isContact = false;
      m_phitkickercircle->m_pball->m_coll.m_hitmoment_bit = true;
      m_phitkickercircle->m_pball->m_pos.x += x; // brian's suggestion
      m_phitkickercircle->m_pball->m_pos.y += y;
      m_phitkickercircle->m_pball->m_pos.z += z;
      m_phitkickercircle->m_pball->m_vel.x = sinf(anglerad) * speed;
      m_phitkickercircle->m_pball->m_vel.y = -cosf(anglerad) * speed;
      m_phitkickercircle->m_pball->m_vel.z = speedz;
      m_phitkickercircle->m_pball->m_frozen = false;
#ifdef C_DYNAMIC
      m_phitkickercircle->m_pball->m_dynamic = C_DYNAMIC;
#endif
      m_phitkickercircle->m_pball = NULL;
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
   WCHAR wz[512];

   MultiByteToWideChar(CP_ACP, 0, m_d.m_szSurface, -1, wz, 32);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP Kicker::put_Surface(BSTR newVal)
{
   STARTUNDO

   WideCharToMultiByte(CP_ACP, 0, newVal, -1, m_d.m_szSurface, 32, NULL, NULL);

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Kicker::get_Enabled(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_d.m_fEnabled);

   return S_OK;
}

STDMETHODIMP Kicker::put_Enabled(VARIANT_BOOL newVal)
{
   STARTUNDO

   m_d.m_fEnabled = VBTOF(newVal);

   if (m_phitkickercircle)
      m_phitkickercircle->m_fEnabled = m_d.m_fEnabled;

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
   UpdateUnitsInfo();
   return S_OK;
}

STDMETHODIMP Kicker::put_Radius(float newVal)
{
   STARTUNDO

      m_d.m_radius = newVal;
   UpdateUnitsInfo();

   STOPUNDO

      return S_OK;
}

STDMETHODIMP Kicker::get_FallThrough(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_d.m_fFallThrough);

   return S_OK;
}

STDMETHODIMP Kicker::put_FallThrough(VARIANT_BOOL newVal)
{
   STARTUNDO

      m_d.m_fFallThrough = VBTOF(newVal);

   STOPUNDO

      return S_OK;
}

STDMETHODIMP Kicker::get_Legacy(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_d.m_legacyMode);

   return S_OK;
}

STDMETHODIMP Kicker::put_Legacy(VARIANT_BOOL newVal)
{
   STARTUNDO

      m_d.m_legacyMode = VBTOF(newVal);

   STOPUNDO

      return S_OK;
}

void Kicker::GetDialogPanes(Vector<PropertyPane> *pvproppane)
{
   PropertyPane *pproppane;

   pproppane = new PropertyPane(IDD_PROP_NAME, NULL);
   pvproppane->AddElement(pproppane);

   pproppane = new PropertyPane(IDD_PROPKICKER_VISUALS, IDS_VISUALS);
   pvproppane->AddElement(pproppane);

   pproppane = new PropertyPane(IDD_PROPLIGHT_POSITION, IDS_POSITION);
   pvproppane->AddElement(pproppane);

   pproppane = new PropertyPane(IDD_PROPKICKER_STATE, IDS_STATE);
   pvproppane->AddElement(pproppane);

   pproppane = new PropertyPane(IDD_PROP_TIMER, IDS_MISC);
   pvproppane->AddElement(pproppane);
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
   //legacy handling:
   if (m_d.m_kickertype > KickerCup2)
	   m_d.m_kickertype = KickerInvisible;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Kicker::get_Material(BSTR *pVal)
{
   WCHAR wz[512];

   MultiByteToWideChar(CP_ACP, 0, m_d.m_szMaterial, -1, wz, 32);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP Kicker::put_Material(BSTR newVal)
{
   STARTUNDO

      WideCharToMultiByte(CP_ACP, 0, newVal, -1, m_d.m_szMaterial, 32, NULL, NULL);

   STOPUNDO

      return S_OK;
}

STDMETHODIMP Kicker::BallCntOver(int *pVal)
{
   int cnt = 0;

   if (g_pplayer)
   {
      for (unsigned i = 0; i < g_pplayer->m_vball.size(); i++)
      {
         Ball * const pball = g_pplayer->m_vball[i];

         if (pball->m_vpVolObjs && pball->m_vpVolObjs->IndexOf((IFireEvents*)this) >= 0) // cast to IFireEvents necessary, as it is stored like this in HitObject.m_obj
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
    for (unsigned int i = 0; i < g_pplayer->m_vball.size(); i++)
    {
        if (g_pplayer->m_vball[i] == m_phitkickercircle->m_lastCapturedBall)
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

    BallEx * const pballex = m_phitkickercircle->m_lastCapturedBall->m_pballex;

    if (!pballex)
    {
        return E_POINTER;
    }
    pballex->QueryInterface(IID_IBall, (void **)pVal);

    return S_OK;
}

float KickerHitCircle::HitTest(const Ball * const pball, const float dtime, CollisionEvent& coll) const
{
   return HitTestBasicRadius(pball, dtime, coll, false, false, false); //any face, not-lateral, non-rigid
}

void KickerHitCircle::DoChangeBallVelocity(Ball * const pball, const Vertex3Ds& hitnormal) const
{
    float minDist_sqr = FLT_MAX;
    unsigned int idx = ~0u;
    for (size_t t = 0; t < m_pkicker->hitMesh.size(); t++)
    {
        // find the right normal by calculating the distance from current ball position to vertex of the kicker mesh               
        const float length_sqr = (pball->m_pos - m_pkicker->hitMesh[t]).LengthSquared();
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
        const float dot = -pball->m_vel.Dot(hitnorm);
        const float reactionImpulse = pball->m_mass * fabsf(dot);
        /*
        if (pball->m_pos.z > pball->m_defaultZ)
        {
        // ball is on a surface(e.g. upper playfield) use a high friction and a different calculation to compensate surface collision
        friction = 1.0f;
        surfP = -pball->m_radius * hitnorm;      // surface contact point relative to center of mass

        surfVel = pball->SurfaceVelocity(surfP); // velocity at impact point

        tangent = surfVel - surfVel.Dot(hitnorm) * hitnorm; // calc the tangential velocity
        }
        else
        */
        surfP = -pball->m_radius * hitnormal;    // surface contact point relative to center of mass

        surfVel = pball->SurfaceVelocity(surfP);         // velocity at impact point

        tangent = surfVel - surfVel.Dot(hitnormal) * hitnorm; // calc the tangential velocity

        pball->m_vel += dot * hitnorm; // apply collision impulse (along normal, so no torque)
#ifdef C_DYNAMIC
        pball->m_dynamic = C_DYNAMIC;
#endif

        const float friction = 0.3f;
        const float tangentSpSq = tangent.LengthSquared();

        if (tangentSpSq > 1e-6f)
        {
            tangent /= sqrtf(tangentSpSq);           // normalize to get tangent direction
            const float vt = surfVel.Dot(tangent);   // get speed in tangential direction

            // compute friction impulse
            const Vertex3Ds cross = CrossProduct(surfP, tangent);
            const float kt = pball->m_invMass + tangent.Dot(CrossProduct(cross / pball->m_inertia, surfP));

            // friction impulse can't be greater than coefficient of friction times collision impulse (Coulomb friction cone)
            const float maxFric = friction * reactionImpulse;
            const float jt = clamp(-vt / kt, -maxFric, maxFric);

            pball->ApplySurfaceImpulse(jt * cross, jt * tangent);
        }
    }
}

void KickerHitCircle::DoCollide(Ball * const pball, const Vertex3Ds& hitnormal, const bool hitbit, const bool newBall)
{
   if (m_pball) return;								 // a previous ball already in kicker

   const int i = pball->m_vpVolObjs->IndexOf(m_obj); // check if kicker in ball's volume set

   if (newBall || ((!hitbit) == (i < 0)))            // New or (Hit && !Vol || UnHit && Vol)
   {
      if (m_pkicker->m_d.m_legacyMode || newBall)
         pball->m_pos += STATICTIME * pball->m_vel;  // move ball slightly forward

      if (i < 0) // entering Kickers volume
      {
         bool hitEvent;
         const float grabHeight = (m_hitBBox.zlow + pball->m_radius) * m_pkicker->m_d.m_hitAccuracy;

         if (pball->m_pos.z < grabHeight || m_pkicker->m_d.m_legacyMode || newBall)
         {
            // early out here if the ball is slow and we are near the kicker center
            hitEvent = true;
         }
         else
         {
            hitEvent = false;
            DoChangeBallVelocity(pball, hitnormal);

            // this is an ugly hack to prevent the ball stopping rapidly at the kicker bevel 
            // something with the friction calculation is wrong in the physics engine
            // so we monitor the ball velocity if it drop under a length value of 0.2
            // if so we take the last "good" velocity to help the ball moving over the critical spot at the kicker bevel
            // this hack seems to work only if the kicker is on the playfield, a kicker attached to a wall has still problems
            // because the friction calculation for a wall is also different
            const float length = pball->m_vel.Length();
            if (length < 0.2f)
            {
                pball->m_vel = pball->m_oldVel;
            }
            pball->m_oldVel = pball->m_vel;
         }

         if (hitEvent)
         {
            if (m_pkicker->m_d.m_fFallThrough)
               pball->m_frozen = false;
            else
            {
               pball->m_frozen = true;
               pball->m_vpVolObjs->AddElement(m_obj);		// add kicker to ball's volume set
               m_pball = pball;
               m_lastCapturedBall = pball;
            }
            // Don't fire the hit event if the ball was just created
            // Fire the event before changing ball attributes, so scripters can get a useful ball state
            if (!newBall)
               m_pkicker->FireGroupEvent(DISPID_HitEvents_Hit);

            if (pball->m_frozen || m_pkicker->m_d.m_fFallThrough)	// script may have unfrozen the ball
            {
               // if ball falls through hole, we fake the collision algo by changing the ball height
               // in HitTestBasicRadius() the z-position of the ball is checked if it is >= to the hit cylinder
               // if we don't change the height of the ball we get a lot of hit events while the ball is falling!!

               // Only mess with variables if ball was not kicked during event
               pball->m_vel.SetZero();
               pball->m_angularmomentum.SetZero();
               pball->m_angularvelocity.SetZero();
               pball->m_pos.x = center.x;
               pball->m_pos.y = center.y;
#ifdef C_DYNAMIC
               pball->m_dynamic = 0;
#endif
               if (m_pkicker->m_d.m_fFallThrough)
                  pball->m_pos.z = m_hitBBox.zlow - pball->m_radius - 5.0f;
               else
                  pball->m_pos.z = m_hitBBox.zlow + pball->m_radius/**pball->m_radius/radius*/;

            }
            else m_pball = NULL; // make sure
         }
      }
      else // exiting kickers volume
      {
         pball->m_vpVolObjs->RemoveElementAt(i); // remove kicker to ball's volume set
         m_pkicker->FireGroupEvent(DISPID_HitEvents_Unhit);
      }
   }
}
