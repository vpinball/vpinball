#include "stdafx.h"
#include "objloader.h"
#include "meshes/bumperBaseMesh.h"
#include "meshes/bumperRingMesh.h"
#include "meshes/bumperCapMesh.h"
#include "meshes/bumperSocketMesh.h"

Bumper::Bumper()
{
   m_pbumperhitcircle = NULL;
   baseVertexBuffer = NULL;
   baseIndexBuffer = NULL;
   ringVertexBuffer = NULL;
   ringIndexBuffer = NULL;
   capVertexBuffer = NULL;
   capIndexBuffer = NULL;
   socketIndexBuffer = NULL;
   socketVertexBuffer = NULL;
   ringMaterial.m_cBase = 0xFFFFFFFF;
   ringMaterial.m_cGlossy = 0;
   ringAnimate = false;
   m_propVisual = NULL;
   memset(m_d.m_szBaseMaterial, 0, 32);
   memset(m_d.m_szCapMaterial, 0, 32);
   memset(m_d.m_szSkirtMaterial, 0, 32);
   memset(m_d.m_szSurface, 0, MAXTOKEN);
}

Bumper::~Bumper()
{
   if (baseVertexBuffer)
   {
      baseVertexBuffer->release();
      baseVertexBuffer = 0;
   }
   if (baseIndexBuffer)
   {
      baseIndexBuffer->release();
      baseIndexBuffer = 0;
      baseTexture.FreeStuff();
   }
   if (ringVertexBuffer)
   {
      ringVertexBuffer->release();
      ringVertexBuffer = 0;
   }
   if (ringIndexBuffer)
   {
      ringIndexBuffer->release();
      ringIndexBuffer = 0;
      ringTexture.FreeStuff();
      delete[] ringVertices;
      ringVertices = 0;
   }
   if (capIndexBuffer)
   {
      capIndexBuffer->release();
      capIndexBuffer = 0;
      capTexture.FreeStuff();
   }
   if (capVertexBuffer)
   {
      capVertexBuffer->release();
      capVertexBuffer = 0;
   }
   if (socketIndexBuffer)
   {
      socketIndexBuffer->release();
      socketIndexBuffer = 0;
      socketTexture.FreeStuff();
   }
   if (socketVertexBuffer)
   {
      socketVertexBuffer->release();
      socketVertexBuffer = 0;
   }
}

HRESULT Bumper::Init(PinTable *ptable, float x, float y, bool fromMouseClick)
{
   m_ptable = ptable;

   SetDefaults(fromMouseClick);

   m_d.m_vCenter.x = x;
   m_d.m_vCenter.y = y;

   ringAnimate = false;

   return InitVBA(fTrue, 0, NULL);
}

void Bumper::SetDefaults(bool fromMouseClick)
{
   HRESULT hr;
   float fTmp;
   int iTmp;

   hr = GetRegStringAsFloat("DefaultProps\\Bumper", "Radius", &fTmp);
   m_d.m_radius = (hr == S_OK) && fromMouseClick ? fTmp : 45.f;

   SetDefaultPhysics(fromMouseClick);

   hr = GetRegStringAsFloat("DefaultProps\\Bumper", "HeightScale", &fTmp);
   m_d.m_heightScale = (hr == S_OK) && fromMouseClick ? fTmp : 90.0f;

   hr = GetRegStringAsFloat("DefaultProps\\Bumper", "RingSpeed", &fTmp);
   m_d.m_ringSpeed = (hr == S_OK) && fromMouseClick ? fTmp : 0.5f;

   hr = GetRegStringAsFloat("DefaultProps\\Bumper", "Orientation", &fTmp);
   m_d.m_orientation = (hr == S_OK) && fromMouseClick ? fTmp : 0.0f;

   hr = GetRegStringAsFloat("DefaultProps\\Bumper", "Threshold", &fTmp);
   m_d.m_threshold = (hr == S_OK) && fromMouseClick ? fTmp : 1.f;

   hr = GetRegString("DefaultProps\\Bumper", "Surface", m_d.m_szSurface, MAXTOKEN);
   if (hr != S_OK || !fromMouseClick)
      m_d.m_szSurface[0] = 0;

   hr = GetRegInt("DefaultProps\\Bumper", "TimerEnabled", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_tdr.m_fTimerEnabled = iTmp == 0 ? false : true;
   else
      m_d.m_tdr.m_fTimerEnabled = false;

   hr = GetRegInt("DefaultProps\\Bumper", "TimerInterval", &iTmp);
   m_d.m_tdr.m_TimerInterval = (hr == S_OK) && fromMouseClick ? iTmp : 100;

   hr = GetRegInt("DefaultProps\\Bumper", "CapVisible", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_fCapVisible = iTmp == 0 ? false : true;
   else
      m_d.m_fCapVisible = true;

   hr = GetRegInt("DefaultProps\\Bumper", "BaseVisible", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_fBaseVisible = iTmp == 0 ? false : true;
   else
      m_d.m_fBaseVisible = true;

   hr = GetRegInt("DefaultProps\\Bumper", "ReflectionEnabled", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_fReflectionEnabled = iTmp == 0 ? false : true;
   else
      m_d.m_fReflectionEnabled = true;

   hr = GetRegInt("DefaultProps\\Bumper", "HasHitEvent", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
       m_d.m_fHitEvent= iTmp == 0 ? false : true;
   else
       m_d.m_fHitEvent = true;

   hr = GetRegInt("DefaultProps\\Bumper", "Collidable", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
       m_d.m_fCollidable = iTmp == 0 ? false : true;
   else
       m_d.m_fCollidable = true;
}

void Bumper::WriteRegDefaults()
{
   SetRegValueFloat("DefaultProps\\Bumper", "Radius", m_d.m_radius);
   SetRegValueFloat("DefaultProps\\Bumper", "Force", m_d.m_force);
   SetRegValueFloat("DefaultProps\\Bumper", "HeightScale", m_d.m_heightScale);
   SetRegValueFloat("DefaultProps\\Bumper", "RingSpeed", m_d.m_ringSpeed);
   SetRegValueFloat("DefaultProps\\Bumper", "Orientation", m_d.m_orientation);
   SetRegValueFloat("DefaultProps\\Bumper", "Threshold", m_d.m_threshold);
   SetRegValueBool("DefaultProps\\Bumper", "TimerEnabled", m_d.m_tdr.m_fTimerEnabled);
   SetRegValueInt("DefaultProps\\Bumper", "TimerInterval", m_d.m_tdr.m_TimerInterval);
   SetRegValueInt("DefaultProps\\Bumper", "CapVisible", m_d.m_fCapVisible);
   SetRegValueInt("DefaultProps\\Bumper", "BaseVisible", m_d.m_fBaseVisible);
   SetRegValueInt("DefaultProps\\Bumper", "HasHitEvent", m_d.m_fHitEvent);
   SetRegValueInt("DefaultProps\\Bumper", "Collidable", m_d.m_fCollidable);
   SetRegValueInt("DefaultProps\\Bumper", "ReflectionEnabled", m_d.m_fReflectionEnabled);
   SetRegValue("DefaultProps\\Bumper", "Surface", REG_SZ, &m_d.m_szSurface, lstrlen(m_d.m_szSurface));
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

void Bumper::PreRender(Sur * const psur)
{
   psur->SetBorderColor(-1, false, 0);

   psur->SetObject(this);
   const float radangle = ANGTORAD(m_d.m_orientation);
   const float sn = sinf(radangle);
   const float cs = cosf(radangle);

   float x1 = m_d.m_vCenter.x - cs*(m_d.m_radius + 10.f);
   float y1 = m_d.m_vCenter.y - sn*(m_d.m_radius + 10.f);
   float x2 = m_d.m_vCenter.x + cs*(m_d.m_radius + 10.f);
   float y2 = m_d.m_vCenter.y + sn*(m_d.m_radius + 10.f);
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

void Bumper::Render(Sur * const psur)
{
   psur->SetBorderColor(RGB(0, 0, 0), false, 0);
   psur->SetFillColor(-1);
   psur->SetObject(this);
   psur->SetObject(NULL);
   const float radangle = ANGTORAD(m_d.m_orientation - 90.f);
   const float sn = sinf(radangle);
   const float cs = cosf(radangle);

   float x1 = m_d.m_vCenter.x - cs*(m_d.m_radius + 10.f);
   float y1 = m_d.m_vCenter.y - sn*(m_d.m_radius + 10.f);
   float x2 = m_d.m_vCenter.x + cs*(m_d.m_radius + 10.f);
   float y2 = m_d.m_vCenter.y + sn*(m_d.m_radius + 10.f);
   psur->Ellipse(x1, y1, 10.0f);
   psur->Ellipse(x2, y2, 10.0f);
   psur->Ellipse(m_d.m_vCenter.x, m_d.m_vCenter.y, m_d.m_radius*1.5f);
   psur->Ellipse(m_d.m_vCenter.x, m_d.m_vCenter.y, m_d.m_radius);

   if (g_pvp->m_fAlwaysDrawLightCenters)
   {
      psur->Line(m_d.m_vCenter.x - 10.0f, m_d.m_vCenter.y, m_d.m_vCenter.x + 10.0f, m_d.m_vCenter.y);
      psur->Line(m_d.m_vCenter.x, m_d.m_vCenter.y - 10.0f, m_d.m_vCenter.x, m_d.m_vCenter.y + 10.0f);
   }
}

void Bumper::GetTimers(Vector<HitTimer> * const pvht)
{
   IEditable::BeginPlay();

   HitTimer * const pht = new HitTimer();
   pht->m_interval = max(m_d.m_tdr.m_TimerInterval,MAX_TIMER_MSEC_INTERVAL);
   pht->m_nextfire = pht->m_interval;
   pht->m_pfe = (IFireEvents *)this;

   m_phittimer = pht;

   if (m_d.m_tdr.m_fTimerEnabled)
      pvht->AddElement(pht);
}

void Bumper::GetHitShapes(Vector<HitObject> * const pvho)
{
   const float height = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y);

   BumperHitCircle * const phitcircle = new BumperHitCircle();

   phitcircle->m_pfe = NULL;
   phitcircle->m_fEnabled = m_d.m_fCollidable;

   phitcircle->center.x = m_d.m_vCenter.x;
   phitcircle->center.y = m_d.m_vCenter.y;
   phitcircle->radius = m_d.m_radius;
   phitcircle->zlow = height;
   phitcircle->zhigh = height + m_d.m_heightScale;

   phitcircle->m_pbumper = this;

   pvho->AddElement(phitcircle);

   m_pbumperhitcircle = phitcircle;

   phitcircle->m_bumperanim.m_fVisible = m_d.m_fBaseVisible;
}

void Bumper::GetHitShapesDebug(Vector<HitObject> * const pvho)
{
   const float height = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y);

   HitObject * const pho = CreateCircularHitPoly(m_d.m_vCenter.x, m_d.m_vCenter.y, height + m_d.m_heightScale, m_d.m_radius, 32);
   pvho->AddElement(pho);
}

void Bumper::EndPlay()
{
   IEditable::EndPlay();

   m_pbumperhitcircle = NULL;

   if (baseVertexBuffer)
   {
      baseVertexBuffer->release();
      baseVertexBuffer = 0;
   }
   if (baseIndexBuffer)
   {
      baseIndexBuffer->release();
      baseIndexBuffer = 0;
      baseTexture.FreeStuff();
   }
   if (ringVertexBuffer)
   {
      ringVertexBuffer->release();
      ringVertexBuffer = 0;
   }
   if (ringIndexBuffer)
   {
      ringIndexBuffer->release();
      ringIndexBuffer = 0;
      ringTexture.FreeStuff();
      delete[] ringVertices;
      ringVertices = 0;
   }
   if (capIndexBuffer)
   {
      capIndexBuffer->release();
      capIndexBuffer = 0;
      capTexture.FreeStuff();
   }
   if (capVertexBuffer)
   {
      capVertexBuffer->release();
      capVertexBuffer = 0;
   }
   if (socketIndexBuffer)
   {
      socketIndexBuffer->release();
      socketIndexBuffer = 0;
      socketTexture.FreeStuff();
   }
   if (socketVertexBuffer)
   {
      socketVertexBuffer->release();
      socketVertexBuffer = 0;
   }
}

void Bumper::UpdateRing(RenderDevice *pd3dDevice)
{
   Vertex3D_NoTex2 *buf;
   ringVertexBuffer->lock(0, 0, (void**)&buf, VertexBuffer::DISCARDCONTENTS);
   for (int i = 0; i < bumperRingNumVertices; i++)
   {
      buf[i].x = ringVertices[i].x;
      buf[i].y = ringVertices[i].y;
      buf[i].z = ringVertices[i].z + m_pbumperhitcircle->m_bumperanim.m_ringAnimOffset;
      buf[i].nx = ringVertices[i].nx;
      buf[i].ny = ringVertices[i].ny;
      buf[i].nz = ringVertices[i].nz;
      buf[i].tu = ringVertices[i].tu;
      buf[i].tv = ringVertices[i].tv;
   }
   ringVertexBuffer->unlock();
}

void Bumper::RenderBase(RenderDevice *pd3dDevice, const Material * const baseMaterial)
{
   pd3dDevice->basicShader->SetMaterial(baseMaterial);
   pd3dDevice->basicShader->SetTexture("Texture0", &baseTexture);
   g_pplayer->m_pin3d.EnableAlphaBlend(false);
   pd3dDevice->basicShader->SetAlphaTestValue((float)(1.0 / 255.0));

   pd3dDevice->basicShader->Begin(0);
   pd3dDevice->DrawIndexedPrimitiveVB(D3DPT_TRIANGLELIST, MY_D3DFVF_NOTEX2_VERTEX, baseVertexBuffer, 0, bumperBaseNumVertices, baseIndexBuffer, 0, bumperBaseNumFaces);
   pd3dDevice->basicShader->End();

   //g_pplayer->m_pin3d.DisableAlphaBlend(); //!! not necessary anymore
}

void Bumper::RenderSocket(RenderDevice *pd3dDevice, const Material * const socketMaterial)
{
   pd3dDevice->basicShader->SetMaterial(socketMaterial);
   pd3dDevice->basicShader->SetTexture("Texture0", &socketTexture);
   g_pplayer->m_pin3d.EnableAlphaBlend(false);
   pd3dDevice->basicShader->SetAlphaTestValue((float)(1.0 / 255.0));

   pd3dDevice->basicShader->Begin(0);
   pd3dDevice->DrawIndexedPrimitiveVB(D3DPT_TRIANGLELIST, MY_D3DFVF_NOTEX2_VERTEX, socketVertexBuffer, 0, bumperSocketNumVertices, socketIndexBuffer, 0, bumperSocketNumFaces);
   pd3dDevice->basicShader->End();

   //g_pplayer->m_pin3d.DisableAlphaBlend(); //!! not necessary anymore
}

void Bumper::RenderCap(RenderDevice *pd3dDevice, const Material * const capMaterial)
{
   pd3dDevice->basicShader->SetMaterial(capMaterial);
   pd3dDevice->basicShader->SetTexture("Texture0", &capTexture);
   g_pplayer->m_pin3d.EnableAlphaBlend(false);
   pd3dDevice->basicShader->SetAlphaTestValue((float)(1.0 / 255.0));

   pd3dDevice->basicShader->Begin(0);
   pd3dDevice->DrawIndexedPrimitiveVB(D3DPT_TRIANGLELIST, MY_D3DFVF_NOTEX2_VERTEX, capVertexBuffer, 0, bumperCapNumVertices, capIndexBuffer, 0, bumperCapNumFaces);
   pd3dDevice->basicShader->End();

   //g_pplayer->m_pin3d.DisableAlphaBlend(); //!! not necessary anymore
}

void Bumper::PostRenderStatic(RenderDevice* pd3dDevice)
{
   TRACE_FUNCTION();

   if (ringVertexBuffer == NULL && (m_d.m_fBaseVisible || m_d.m_fCapVisible))
      return;

   if (m_ptable->m_fReflectionEnabled && !m_d.m_fReflectionEnabled)
      return;

   const U32 old_time_msec = (m_d.m_time_msec < g_pplayer->m_time_msec) ? m_d.m_time_msec : g_pplayer->m_time_msec;
   m_d.m_time_msec = g_pplayer->m_time_msec;
   const float diff_time_msec = (float)(g_pplayer->m_time_msec - old_time_msec);

   pd3dDevice->SetRenderState(RenderDevice::DEPTHBIAS, 0);
   pd3dDevice->SetRenderState(RenderDevice::ZWRITEENABLE, TRUE);

   if (m_d.m_fBaseVisible)
   {
      const int state = m_pbumperhitcircle->m_bumperanim.m_fHitEvent ? 1 : 0;    // 0 = not hit, 1 = hit

      if (state == 1)
      {
         ringAnimate = true;
         ringDown = true;
         m_pbumperhitcircle->m_bumperanim.m_fHitEvent = false;
      }

      if (ringAnimate)
      {
         float step = m_d.m_ringSpeed*m_ptable->m_BG_scalez[m_ptable->m_BG_current_set];
         const float limit = 45.f*m_ptable->m_BG_scalez[m_ptable->m_BG_current_set];
         if (ringDown)
            step = -step;
         m_pbumperhitcircle->m_bumperanim.m_ringAnimOffset += step*diff_time_msec;
         if (ringDown)
         {
            if (m_pbumperhitcircle->m_bumperanim.m_ringAnimOffset <= -limit)
            {
               m_pbumperhitcircle->m_bumperanim.m_ringAnimOffset = -limit;
               ringDown = false;
            }
         }
         else
         {
            if (m_pbumperhitcircle->m_bumperanim.m_ringAnimOffset >= 0.0f)
            {
               m_pbumperhitcircle->m_bumperanim.m_ringAnimOffset = 0.0f;
               ringAnimate = false;
            }
         }
         UpdateRing(pd3dDevice);
      }

      pd3dDevice->SetRenderState(RenderDevice::CULLMODE, D3DCULL_CCW);

      pd3dDevice->basicShader->SetTechnique("basic_with_texture");
      pd3dDevice->basicShader->SetMaterial(&ringMaterial);
      pd3dDevice->basicShader->SetTexture("Texture0", &ringTexture);
      pd3dDevice->basicShader->SetAlphaTestValue(-1.0f);
      // render ring
      pd3dDevice->basicShader->Begin(0);
      pd3dDevice->DrawIndexedPrimitiveVB(D3DPT_TRIANGLELIST, MY_D3DFVF_NOTEX2_VERTEX, ringVertexBuffer, 0, bumperRingNumVertices, ringIndexBuffer, 0, bumperRingNumFaces);
      pd3dDevice->basicShader->End();

      const Material *mat = m_ptable->GetMaterial(m_d.m_szSkirtMaterial);
      if (mat->m_bOpacityActive)
      {
         pd3dDevice->SetRenderState(RenderDevice::CULLMODE, D3DCULL_NONE);
         RenderSocket(pd3dDevice, mat);
      }

      mat = m_ptable->GetMaterial(m_d.m_szBaseMaterial);
      if (mat->m_bOpacityActive)
      {
         pd3dDevice->SetRenderState(RenderDevice::CULLMODE, D3DCULL_NONE);
         RenderBase(pd3dDevice, mat);
      }
   }

   if (m_d.m_fCapVisible)
   {
      const Material * const mat = m_ptable->GetMaterial(m_d.m_szCapMaterial);
      if (mat->m_bOpacityActive)
      {
         pd3dDevice->basicShader->SetTechnique("basic_with_texture");
         pd3dDevice->SetRenderState(RenderDevice::CULLMODE, D3DCULL_NONE);
         RenderCap(pd3dDevice, mat);
      }
   }
}

void Bumper::ExportMesh(FILE *f)
{
   char name[MAX_PATH];
   char subObjName[MAX_PATH];
   WideCharToMultiByte(CP_ACP, 0, m_wzName, -1, name, MAX_PATH, NULL, NULL);


   baseHeight = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y) * m_ptable->m_BG_scalez[m_ptable->m_BG_current_set];
   fullMatrix.RotateZMatrix(ANGTORAD(m_d.m_orientation));

   if (m_d.m_fBaseVisible)
   {
      Vertex3D_NoTex2 *base = new Vertex3D_NoTex2[bumperBaseNumVertices];
      strcpy_s(subObjName, name);
      strcat_s(subObjName, "Base");
      WaveFrontObj_WriteObjectName(f, subObjName);

      GenerateBaseMesh(base);
      WaveFrontObj_WriteVertexInfo(f, base, bumperBaseNumVertices);
      const Material * mat = m_ptable->GetMaterial(m_d.m_szBaseMaterial);
      WaveFrontObj_WriteMaterial(m_d.m_szBaseMaterial, NULL, mat);
      WaveFrontObj_UseTexture(f, m_d.m_szBaseMaterial);
      WaveFrontObj_WriteFaceInfoList(f, bumperBaseIndices, bumperBaseNumFaces);
      WaveFrontObj_UpdateFaceOffset(bumperBaseNumVertices);
      delete[] base;

      Vertex3D_NoTex2 *ring = new Vertex3D_NoTex2[bumperRingNumVertices];
      strcpy_s(subObjName, name);
      strcat_s(subObjName, "Ring");
      WaveFrontObj_WriteObjectName(f, subObjName);

      GenerateRingMesh(ring);
      WaveFrontObj_WriteVertexInfo(f, ring, bumperRingNumVertices);
      WaveFrontObj_WriteFaceInfoList(f, bumperRingIndices, bumperRingNumFaces);
      WaveFrontObj_UpdateFaceOffset(bumperRingNumVertices);
      delete[] ring;

      Vertex3D_NoTex2 *socket = new Vertex3D_NoTex2[bumperSocketNumVertices];
      strcpy_s(subObjName, name);
      strcat_s(subObjName, "Socket");
      WaveFrontObj_WriteObjectName(f, subObjName);

      GenerateSocketMesh(socket);
      WaveFrontObj_WriteVertexInfo(f, socket, bumperSocketNumVertices);
      mat = m_ptable->GetMaterial(m_d.m_szSkirtMaterial);
      WaveFrontObj_WriteMaterial(m_d.m_szSkirtMaterial, NULL, mat);
      WaveFrontObj_UseTexture(f, m_d.m_szSkirtMaterial);
      WaveFrontObj_WriteFaceInfoList(f, bumperSocketIndices, bumperSocketNumFaces);
      WaveFrontObj_UpdateFaceOffset(bumperSocketNumVertices);
      delete[] socket;
   }
   if (m_d.m_fCapVisible)
   {
      Vertex3D_NoTex2 *cap = new Vertex3D_NoTex2[bumperCapNumVertices];
      strcpy_s(subObjName, name);
      strcat_s(subObjName, "Cap");
      WaveFrontObj_WriteObjectName(f, subObjName);

      GenerateCapMesh(cap);
      WaveFrontObj_WriteVertexInfo(f, cap, bumperCapNumVertices);
      const Material * const mat = m_ptable->GetMaterial(m_d.m_szCapMaterial);
      WaveFrontObj_WriteMaterial(m_d.m_szCapMaterial, NULL, mat);
      WaveFrontObj_UseTexture(f, m_d.m_szCapMaterial);
      WaveFrontObj_WriteFaceInfoList(f, bumperCapIndices, bumperCapNumFaces);
      WaveFrontObj_UpdateFaceOffset(bumperCapNumVertices);
      delete[] cap;
   }
}

void Bumper::GenerateBaseMesh(Vertex3D_NoTex2 *buf)
{
   const float scalexy = m_d.m_radius*2.0f;
   for (int i = 0; i < bumperBaseNumVertices; i++)
   {
      Vertex3Ds vert(bumperBase[i].x, bumperBase[i].y, bumperBase[i].z);
      vert = fullMatrix.MultiplyVector(vert);
      buf[i].x = vert.x*scalexy + m_d.m_vCenter.x;
      buf[i].y = vert.y*scalexy + m_d.m_vCenter.y;
      buf[i].z = (vert.z * m_d.m_heightScale)*m_ptable->m_BG_scalez[m_ptable->m_BG_current_set] + baseHeight;

      vert = Vertex3Ds(bumperBase[i].nx, bumperBase[i].ny, bumperBase[i].nz);
      vert = fullMatrix.MultiplyVectorNoTranslate(vert);
      buf[i].nx = vert.x;
      buf[i].ny = vert.y;
      buf[i].nz = vert.z;
      buf[i].tu = bumperBase[i].tu;
      buf[i].tv = bumperBase[i].tv;
   }
}

void Bumper::GenerateSocketMesh(Vertex3D_NoTex2 *buf)
{
   const float scalexy = m_d.m_radius*2.0f;

   for (int i = 0; i < bumperSocketNumVertices; i++)
   {
      Vertex3Ds vert(bumperSocket[i].x, bumperSocket[i].y, bumperSocket[i].z);
      vert = fullMatrix.MultiplyVector(vert);
      buf[i].x = vert.x*scalexy + m_d.m_vCenter.x;
      buf[i].y = vert.y*scalexy + m_d.m_vCenter.y;
      buf[i].z = (vert.z*m_d.m_heightScale)*m_ptable->m_BG_scalez[m_ptable->m_BG_current_set] + baseHeight;

      vert = Vertex3Ds(bumperSocket[i].nx, bumperSocket[i].ny, bumperSocket[i].nz);
      vert = fullMatrix.MultiplyVectorNoTranslate(vert);
      buf[i].nx = vert.x;
      buf[i].ny = vert.y;
      buf[i].nz = vert.z;
      buf[i].tu = bumperSocket[i].tu;
      buf[i].tv = bumperSocket[i].tv;
   }
}

void Bumper::GenerateRingMesh(Vertex3D_NoTex2 *buf)
{
   const float scalexy = m_d.m_radius*2.0f;

   for (int i = 0; i < bumperRingNumVertices; i++)
   {
      Vertex3Ds vert(bumperRing[i].x, bumperRing[i].y, bumperRing[i].z);
      vert = fullMatrix.MultiplyVector(vert);
      buf[i].x = vert.x*scalexy + m_d.m_vCenter.x;
      buf[i].y = vert.y*scalexy + m_d.m_vCenter.y;
      buf[i].z = (vert.z * m_d.m_heightScale)*m_ptable->m_BG_scalez[m_ptable->m_BG_current_set] + baseHeight;

      vert = Vertex3Ds(bumperRing[i].nx, bumperRing[i].ny, bumperRing[i].nz);
      vert = fullMatrix.MultiplyVectorNoTranslate(vert);
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

   for (int i = 0; i < bumperCapNumVertices; i++)
   {
      Vertex3Ds vert(bumperCap[i].x, bumperCap[i].y, bumperCap[i].z);
      vert = fullMatrix.MultiplyVector(vert);
      buf[i].x = vert.x*scalexy + m_d.m_vCenter.x;
      buf[i].y = vert.y*scalexy + m_d.m_vCenter.y;
      buf[i].z = (vert.z *m_d.m_heightScale + m_d.m_heightScale)*m_ptable->m_BG_scalez[m_ptable->m_BG_current_set] + baseHeight;

      vert = Vertex3Ds(bumperCap[i].nx, bumperCap[i].ny, bumperCap[i].nz);
      vert = fullMatrix.MultiplyVectorNoTranslate(vert);
      buf[i].nx = vert.x;
      buf[i].ny = vert.y;
      buf[i].nz = vert.z;
      buf[i].tu = bumperCap[i].tu;
      buf[i].tv = bumperCap[i].tv;
   }
}

void Bumper::RenderSetup(RenderDevice* pd3dDevice)
{
   m_d.m_time_msec = g_pplayer->m_time_msec;

   baseHeight = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y) * m_ptable->m_BG_scalez[m_ptable->m_BG_current_set];

   const float scalez = m_d.m_radius*m_d.m_heightScale*2.0f*m_ptable->m_BG_scalez[m_ptable->m_BG_current_set];

   if (m_d.m_fBaseVisible)
   {
      baseTexture.CreateFromResource(IDB_BUMPERBASE);
      socketTexture.CreateFromResource(IDB_BUMPERSOCKET);
      ringTexture.CreateFromResource(IDB_RINGENVMAP);

      if (baseIndexBuffer)
         baseIndexBuffer->release();
      baseIndexBuffer = pd3dDevice->CreateAndFillIndexBuffer(bumperBaseNumFaces, bumperBaseIndices);

      if (baseVertexBuffer)
         baseVertexBuffer->release();
      pd3dDevice->CreateVertexBuffer(bumperBaseNumVertices, 0, MY_D3DFVF_NOTEX2_VERTEX, &baseVertexBuffer);

      fullMatrix.RotateZMatrix(ANGTORAD(m_d.m_orientation));

      Vertex3D_NoTex2 *buf;
      baseVertexBuffer->lock(0, 0, (void**)&buf, 0);
      GenerateBaseMesh(buf);
      baseVertexBuffer->unlock();

      if (socketIndexBuffer)
         socketIndexBuffer->release();
      socketIndexBuffer = pd3dDevice->CreateAndFillIndexBuffer(bumperSocketNumFaces, bumperSocketIndices);

      if (socketVertexBuffer)
         socketVertexBuffer->release();
      pd3dDevice->CreateVertexBuffer(bumperSocketNumVertices, 0, MY_D3DFVF_NOTEX2_VERTEX, &socketVertexBuffer);

      socketVertexBuffer->lock(0, 0, (void**)&buf, 0);
      GenerateSocketMesh(buf);
      socketVertexBuffer->unlock();

      if (ringIndexBuffer)
         ringIndexBuffer->release();
      ringIndexBuffer = pd3dDevice->CreateAndFillIndexBuffer(bumperRingNumFaces, bumperRingIndices);

      if (ringVertexBuffer)
         ringVertexBuffer->release();
      pd3dDevice->CreateVertexBuffer(bumperRingNumVertices, USAGE_DYNAMIC, MY_D3DFVF_NOTEX2_VERTEX, &ringVertexBuffer);

      ringVertices = new Vertex3D_NoTex2[bumperRingNumVertices];
      GenerateRingMesh(ringVertices);

      ringVertexBuffer->lock(0, 0, (void**)&buf, 0);
      memcpy(buf, ringVertices, bumperRingNumVertices*sizeof(Vertex3D_NoTex2));
      ringVertexBuffer->unlock();
   }

   if (m_d.m_fCapVisible)
   {
      capTexture.CreateFromResource(IDB_BUMPERCAP);

      if (capIndexBuffer)
         capIndexBuffer->release();
      capIndexBuffer = pd3dDevice->CreateAndFillIndexBuffer(bumperCapNumFaces, bumperCapIndices);

      if (capVertexBuffer)
         capVertexBuffer->release();
      pd3dDevice->CreateVertexBuffer(bumperCapNumVertices, 0, MY_D3DFVF_NOTEX2_VERTEX, &capVertexBuffer);

      Vertex3D_NoTex2 *buf;
      capVertexBuffer->lock(0, 0, (void**)&buf, 0);
      GenerateCapMesh(buf);
      capVertexBuffer->unlock();
   }

}

void Bumper::RenderStatic(RenderDevice* pd3dDevice)
{
   if (m_ptable->m_fReflectionEnabled && !m_d.m_fReflectionEnabled)
      return;

   if (m_d.m_fBaseVisible)
   {
      pd3dDevice->basicShader->SetTechnique("basic_with_texture");

      const Material *mat = m_ptable->GetMaterial(m_d.m_szSkirtMaterial);
      if (!mat->m_bOpacityActive)
      {
         RenderSocket(pd3dDevice, mat);
      }

      mat = m_ptable->GetMaterial(m_d.m_szBaseMaterial);
      if (!mat->m_bOpacityActive)
      {
         RenderBase(pd3dDevice, mat);
      }
   }
   if (m_d.m_fCapVisible)
   {
      const Material * const mat = m_ptable->GetMaterial(m_d.m_szCapMaterial);
      if (!mat->m_bOpacityActive)
      {
         pd3dDevice->basicShader->SetTechnique("basic_with_texture");
         RenderCap(pd3dDevice, mat);
      }
   }
}

void Bumper::SetObjectPos()
{
   g_pvp->SetObjectPosCur(m_d.m_vCenter.x, m_d.m_vCenter.y);
}

void Bumper::MoveOffset(const float dx, const float dy)
{
   m_d.m_vCenter.x += dx;
   m_d.m_vCenter.y += dy;

   m_ptable->SetDirtyDraw();
}

void Bumper::GetCenter(Vertex2D * const pv) const
{
   *pv = m_d.m_vCenter;
}

void Bumper::PutCenter(const Vertex2D * const pv)
{
   m_d.m_vCenter = *pv;

   m_ptable->SetDirtyDraw();
}

HRESULT Bumper::SaveData(IStream *pstm, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
{
   BiffWriter bw(pstm, hcrypthash, hcryptkey);

   bw.WriteStruct(FID(VCEN), &m_d.m_vCenter, sizeof(Vertex2D));
   bw.WriteFloat(FID(RADI), m_d.m_radius);
   bw.WriteBool(FID(TMON), m_d.m_tdr.m_fTimerEnabled);
   bw.WriteInt(FID(TMIN), m_d.m_tdr.m_TimerInterval);
   bw.WriteFloat(FID(THRS), m_d.m_threshold);
   bw.WriteFloat(FID(FORC), m_d.m_force);
   bw.WriteFloat(FID(HISC), m_d.m_heightScale);
   bw.WriteFloat(FID(RISP), m_d.m_ringSpeed);
   bw.WriteFloat(FID(ORIN), m_d.m_orientation);
   bw.WriteString(FID(MATR), m_d.m_szCapMaterial);
   bw.WriteString(FID(BAMA), m_d.m_szBaseMaterial);
   bw.WriteString(FID(SKMA), m_d.m_szSkirtMaterial);
   bw.WriteString(FID(SURF), m_d.m_szSurface);
   bw.WriteWideString(FID(NAME), (WCHAR *)m_wzName);

   bw.WriteBool(FID(CAVI), m_d.m_fCapVisible);
   bw.WriteBool(FID(BSVS), m_d.m_fBaseVisible);
   bw.WriteBool(FID(HAHE), m_d.m_fHitEvent);
   bw.WriteBool(FID(COLI), m_d.m_fCollidable);
   bw.WriteBool(FID(REEN), m_d.m_fReflectionEnabled);

   ISelect::SaveData(pstm, hcrypthash, hcryptkey);

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

BOOL Bumper::LoadToken(int id, BiffReader *pbr)
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
   else if (id == FID(MATR))
   {
      pbr->GetString(m_d.m_szCapMaterial);
   }
   else if (id == FID(BAMA))
   {
      pbr->GetString(m_d.m_szBaseMaterial);
   }
   else if (id == FID(SKMA))
   {
      pbr->GetString(m_d.m_szSkirtMaterial);
   }
   else if (id == FID(TMON))
   {
      pbr->GetBool(&m_d.m_tdr.m_fTimerEnabled);
   }
   else if (id == FID(TMIN))
   {
      pbr->GetInt(&m_d.m_tdr.m_TimerInterval);
   }
   else if (id == FID(THRS))
   {
      pbr->GetFloat(&m_d.m_threshold);
   }
   else if (id == FID(FORC))
   {
      pbr->GetFloat(&m_d.m_force);
   }
   else if (id == FID(HISC))
   {
      pbr->GetFloat(&m_d.m_heightScale);
   }
   else if (id == FID(RISP))
   {
      pbr->GetFloat(&m_d.m_ringSpeed);
   }
   else if (id == FID(ORIN))
   {
      pbr->GetFloat(&m_d.m_orientation);
   }
   else if (id == FID(SURF))
   {
      pbr->GetString(m_d.m_szSurface);
   }
   else if (id == FID(NAME))
   {
      pbr->GetWideString((WCHAR *)m_wzName);
   }
   else if (id == FID(BVIS))
   {
      //backwards compatibility when loading old VP9 tables
      bool value;
      pbr->GetBool(&value);
      m_d.m_fCapVisible = value;
      m_d.m_fBaseVisible = value;
   }
   else if (id == FID(CAVI))
   {
      pbr->GetBool(&m_d.m_fCapVisible);
   }
   else if (id == FID(HAHE))
   {
       pbr->GetBool(&m_d.m_fHitEvent);
   }
   else if (id == FID(COLI))
   {
       pbr->GetBool(&m_d.m_fCollidable);
   }
   else if (id == FID(BSVS))
   {
      pbr->GetBool(&m_d.m_fBaseVisible);
   }
   else if (id == FID(REEN))
   {
      pbr->GetBool(&m_d.m_fReflectionEnabled);
   }
   else
   {
      ISelect::LoadToken(id, pbr);
   }
   return fTrue;
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
   STARTUNDO

      m_d.m_radius = newVal;

   STOPUNDO

      return S_OK;
}

STDMETHODIMP Bumper::get_Force(float *pVal)
{
   *pVal = m_d.m_force;

   return S_OK;
}

STDMETHODIMP Bumper::put_Force(float newVal)
{
   STARTUNDO

      m_d.m_force = newVal;

   STOPUNDO

      return S_OK;
}

STDMETHODIMP Bumper::get_HeightScale(float *pVal)
{
   *pVal = m_d.m_heightScale;

   return S_OK;
}

STDMETHODIMP Bumper::put_HeightScale(float newVal)
{
   STARTUNDO

      m_d.m_heightScale = newVal;

   STOPUNDO

      return S_OK;
}

STDMETHODIMP Bumper::get_RingSpeed(float *pVal)
{
   *pVal = m_d.m_ringSpeed;

   return S_OK;
}

STDMETHODIMP Bumper::put_RingSpeed(float newVal)
{
   STARTUNDO

      m_d.m_ringSpeed = newVal;

   STOPUNDO

      return S_OK;
}

STDMETHODIMP Bumper::get_Orientation(float *pVal)
{
   *pVal = m_d.m_orientation;

   return S_OK;
}

STDMETHODIMP Bumper::put_Orientation(float newVal)
{
   STARTUNDO

      m_d.m_orientation = newVal;

   STOPUNDO

      return S_OK;
}

STDMETHODIMP Bumper::get_Threshold(float *pVal)
{
   *pVal = m_d.m_threshold;

   return S_OK;
}

STDMETHODIMP Bumper::put_Threshold(float newVal)
{
   STARTUNDO

      m_d.m_threshold = newVal;

   STOPUNDO

      return S_OK;
}

STDMETHODIMP Bumper::get_CapMaterial(BSTR *pVal)
{
   WCHAR wz[512];

   MultiByteToWideChar(CP_ACP, 0, m_d.m_szCapMaterial, -1, wz, 32);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP Bumper::put_CapMaterial(BSTR newVal)
{
   STARTUNDO

      WideCharToMultiByte(CP_ACP, 0, newVal, -1, m_d.m_szCapMaterial, 32, NULL, NULL);

   STOPUNDO

      return S_OK;
}

STDMETHODIMP Bumper::get_BaseMaterial(BSTR *pVal)
{
   WCHAR wz[512];

   MultiByteToWideChar(CP_ACP, 0, m_d.m_szBaseMaterial, -1, wz, 32);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP Bumper::put_BaseMaterial(BSTR newVal)
{
   STARTUNDO

      WideCharToMultiByte(CP_ACP, 0, newVal, -1, m_d.m_szBaseMaterial, 32, NULL, NULL);

   STOPUNDO

      return S_OK;
}

STDMETHODIMP Bumper::get_SkirtMaterial(BSTR *pVal)
{
   WCHAR wz[512];

   MultiByteToWideChar(CP_ACP, 0, m_d.m_szSkirtMaterial, -1, wz, 32);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP Bumper::put_SkirtMaterial(BSTR newVal)
{
   STARTUNDO

      WideCharToMultiByte(CP_ACP, 0, newVal, -1, m_d.m_szSkirtMaterial, 32, NULL, NULL);

   STOPUNDO

      return S_OK;
}

STDMETHODIMP Bumper::get_X(float *pVal)
{
   *pVal = m_d.m_vCenter.x;

   return S_OK;
}

STDMETHODIMP Bumper::put_X(float newVal)
{
   STARTUNDO

      m_d.m_vCenter.x = newVal;

   STOPUNDO

      return S_OK;
}

STDMETHODIMP Bumper::get_Y(float *pVal)
{
   *pVal = m_d.m_vCenter.y;

   return S_OK;
}

STDMETHODIMP Bumper::put_Y(float newVal)
{
   STARTUNDO

      m_d.m_vCenter.y = newVal;

   STOPUNDO

      return S_OK;
}

STDMETHODIMP Bumper::get_Surface(BSTR *pVal)
{
   WCHAR wz[512];

   MultiByteToWideChar(CP_ACP, 0, m_d.m_szSurface, -1, wz, 32);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP Bumper::put_Surface(BSTR newVal)
{
   STARTUNDO

      WideCharToMultiByte(CP_ACP, 0, newVal, -1, m_d.m_szSurface, 32, NULL, NULL);

   STOPUNDO

      return S_OK;
}

void Bumper::GetDialogPanes(Vector<PropertyPane> *pvproppane)
{
   PropertyPane *pproppane;

   pproppane = new PropertyPane(IDD_PROP_NAME, NULL);
   pvproppane->AddElement(pproppane);

   m_propVisual = new PropertyPane(IDD_PROPBUMPER_VISUALS, IDS_VISUALS);
   pvproppane->AddElement(m_propVisual);

   pproppane = new PropertyPane(IDD_PROPLIGHT_POSITION, IDS_POSITION);
   pvproppane->AddElement(pproppane);

   pproppane = new PropertyPane(IDD_PROPBUMPER_PHYSICS, IDS_PHYSICS);
   pvproppane->AddElement(pproppane);

   pproppane = new PropertyPane(IDD_PROP_TIMER, IDS_MISC);
   pvproppane->AddElement(pproppane);
}

STDMETHODIMP Bumper::get_HasHitEvent(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_d.m_fHitEvent);

   return S_OK;
}

STDMETHODIMP Bumper::put_HasHitEvent(VARIANT_BOOL newVal)
{
   STARTUNDO
      m_d.m_fHitEvent = VBTOF(newVal);
   STOPUNDO

      return S_OK;
}

STDMETHODIMP Bumper::get_Collidable(VARIANT_BOOL *pVal)
{
    *pVal = (VARIANT_BOOL)FTOVB(m_d.m_fCollidable);

    return S_OK;
}

STDMETHODIMP Bumper::put_Collidable(VARIANT_BOOL newVal)
{
    STARTUNDO
        m_d.m_fCollidable = VBTOF(newVal);

    if (m_pbumperhitcircle)
        m_pbumperhitcircle->m_fEnabled = m_d.m_fCollidable;

    STOPUNDO

        return S_OK;
}


STDMETHODIMP Bumper::get_CapVisible(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_d.m_fCapVisible);

   return S_OK;
}

STDMETHODIMP Bumper::put_CapVisible(VARIANT_BOOL newVal)
{
   STARTUNDO
      m_d.m_fCapVisible = VBTOF(newVal);
   STOPUNDO

      return S_OK;
}

STDMETHODIMP Bumper::get_BaseVisible(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_d.m_fBaseVisible);

   return S_OK;
}

STDMETHODIMP Bumper::put_BaseVisible(VARIANT_BOOL newVal)
{
   STARTUNDO
      m_d.m_fBaseVisible = VBTOF(newVal);
   STOPUNDO

      return S_OK;
}

STDMETHODIMP Bumper::get_ReflectionEnabled(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_d.m_fReflectionEnabled);

   return S_OK;
}

STDMETHODIMP Bumper::put_ReflectionEnabled(VARIANT_BOOL newVal)
{
   STARTUNDO
      m_d.m_fReflectionEnabled = VBTOF(newVal);
   STOPUNDO

      return S_OK;
}

STDMETHODIMP Bumper::PlayHit()
{
    if ( m_pbumperhitcircle )
        m_pbumperhitcircle->m_bumperanim.m_fHitEvent=true;
    return S_OK;
}

void Bumper::UpdatePropertyPanes()
{
   if (m_propVisual == NULL)
      return;

   if (!m_d.m_fCapVisible)
      EnableWindow(GetDlgItem(m_propVisual->dialogHwnd, IDC_MATERIAL_COMBO), FALSE);
   else
      EnableWindow(GetDlgItem(m_propVisual->dialogHwnd, IDC_MATERIAL_COMBO), TRUE);

   if (!m_d.m_fBaseVisible)
   {
      EnableWindow(GetDlgItem(m_propVisual->dialogHwnd, IDC_MATERIAL_COMBO2), FALSE);
      EnableWindow(GetDlgItem(m_propVisual->dialogHwnd, IDC_MATERIAL_COMBO3), FALSE);
   }
   else
   {
      EnableWindow(GetDlgItem(m_propVisual->dialogHwnd, IDC_MATERIAL_COMBO2), TRUE);
      EnableWindow(GetDlgItem(m_propVisual->dialogHwnd, IDC_MATERIAL_COMBO3), TRUE);
   }
}

void Bumper::SetDefaultPhysics(bool fromMouseClick)
{
   HRESULT hr;
   float fTmp;

   hr = GetRegStringAsFloat("DefaultProps\\Bumper", "Force", &fTmp);
   m_d.m_force = (hr == S_OK) && fromMouseClick ? fTmp : 15;

}
