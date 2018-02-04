#include "StdAfx.h"
#include "objloader.h"
#include "meshes/gateBracketMesh.h"
#include "meshes/gateWireMesh.h"
#include "meshes/gateLongPlateMesh.h"
#include "meshes/gatePlateMesh.h"
#include "meshes/gateWireRectangleMesh.h"

Gate::Gate()
{
   m_phitgate = NULL;
   m_plineseg = NULL;
   bracketIndexBuffer = NULL;
   bracketVertexBuffer = NULL;
   wireIndexBuffer = NULL;
   wireVertexBuffer = NULL;
   m_vertexbuffer_angle = FLT_MAX;
   memset(m_d.m_szMaterial, 0, 32);
   memset(m_d.m_szSurface, 0, MAXTOKEN);
   m_vertices = 0;
   m_indices = 0;
   m_numIndices = 0;
   m_numVertices = 0;
}

void Gate::SetGateType(GateType type)
{
    if (m_d.m_type == GateWireW)
    {
        m_vertices = gateWire;
        m_indices = gateWireIndices;
        m_numIndices = gateWireNumIndices;
        m_numVertices = gateWireNumVertices;
    }
    else if (m_d.m_type == GateWireRectangle)
    {
        m_vertices = gateWireRectangleMesh;
        m_indices = gateWireRectangleIndices;
        m_numIndices = gateWireRectangleNumIndices;
        m_numVertices = gateWireRectangleNumVertices;
    }
    else if (m_d.m_type == GateLongPlate)
    {
        m_vertices = gateLongPlateMesh;
        m_indices = gateLongPlateIndices;
        m_numIndices = gateLongPlateNumIndices;
        m_numVertices = gateLongPlateVertices;
    }
    else if (m_d.m_type == GatePlate)
    {
        m_vertices = gatePlateMesh;
        m_indices = gatePlateIndices;
        m_numIndices = gatePlateNumIndices;
        m_numVertices = gatePlateNumVertices;
    }
    else
    {
       m_vertices = gateWire;
       m_indices = gateWireIndices;
       m_numIndices = gateWireNumIndices;
       m_numVertices = gateWireNumVertices;
       m_d.m_type = GateWireW;
    }
}

Gate::~Gate()
{
   if (bracketVertexBuffer)
   {
      bracketVertexBuffer->release();
      bracketVertexBuffer = NULL;
   }
   if (bracketIndexBuffer)
   {
      bracketIndexBuffer->release();
      bracketIndexBuffer = NULL;
   }
   if (wireIndexBuffer)
   {
      wireIndexBuffer->release();
      wireIndexBuffer = NULL;
   }
   if (wireVertexBuffer)
   {
      wireVertexBuffer->release();
      wireVertexBuffer = NULL;
   }
}

void Gate::UpdateUnitsInfo()
{
   char tbuf[128];
   sprintf_s(tbuf, "Length: %.3f | Height: %.3f", g_pvp->ConvertToUnit(m_d.m_length), g_pvp->ConvertToUnit(m_d.m_height));
   g_pvp->SetStatusBarUnitInfo(tbuf);
}

HRESULT Gate::Init(PinTable *ptable, float x, float y, bool fromMouseClick)
{
   HRESULT hr = S_OK;

   m_ptable = ptable;

   m_d.m_vCenter.x = x;
   m_d.m_vCenter.y = y;

   SetDefaults(fromMouseClick);

   InitVBA(fTrue, 0, NULL);

   return hr;
}

void Gate::SetDefaults(bool fromMouseClick)
{
   HRESULT hr;
   float fTmp;
   int iTmp;

   hr = GetRegStringAsFloat("DefaultProps\\Gate", "Length", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_length = fTmp;
   else
      m_d.m_length = 100.f;

   hr = GetRegStringAsFloat("DefaultProps\\Gate", "Height", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_height = fTmp;
   else
      m_d.m_height = 50.f;

   hr = GetRegStringAsFloat("DefaultProps\\Gate", "Rotation", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_rotation = fTmp;
   else
      m_d.m_rotation = -90.f;

   hr = GetRegInt("DefaultProps\\Gate", "ShowBracket", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
       m_d.m_fShowBracket = iTmp == 0 ? false : true;
   else
       m_d.m_fShowBracket = true;

   hr = GetRegInt("DefaultProps\\Gate", "GateType", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
       m_d.m_type= (enum GateType)iTmp;
   else
       m_d.m_type = GateWireW;

   hr = GetRegInt("DefaultProps\\Gate", "Collidable", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_fCollidable = iTmp == 0 ? false : true;
   else
      m_d.m_fCollidable = true;

   hr = GetRegStringAsFloat("DefaultProps\\Gate", "AngleMin", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_angleMin = fTmp;
   else
      m_d.m_angleMin = 0;

   hr = GetRegStringAsFloat("DefaultProps\\Gate", "AngleMax", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_angleMax = fTmp;
   else
      m_d.m_angleMax = (float)(M_PI / 2.0);

   hr = GetRegInt("DefaultProps\\Gate", "Visible", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_fVisible = iTmp == 0 ? false : true;
   else
      m_d.m_fVisible = true;

   hr = GetRegInt("DefaultProps\\Gate", "TimerEnabled", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_tdr.m_fTimerEnabled = iTmp == 0 ? false : true;
   else
      m_d.m_tdr.m_fTimerEnabled = false;

   hr = GetRegInt("DefaultProps\\Gate", "TimerInterval", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_tdr.m_TimerInterval = iTmp;
   else
      m_d.m_tdr.m_TimerInterval = 100;

   hr = GetRegString("DefaultProps\\Gate", "Surface", &m_d.m_szSurface, MAXTOKEN);
   if ((hr != S_OK) || !fromMouseClick)
      m_d.m_szSurface[0] = 0;

   SetDefaultPhysics(fromMouseClick);

   hr = GetRegInt("DefaultProps\\Gate", "TwoWay", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_twoWay = iTmp == 0 ? false : true;
   else
      m_d.m_twoWay = true;

   hr = GetRegInt("DefaultProps\\Gate", "ReflectionEnabled", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_fReflectionEnabled = iTmp == 0 ? false : true;
   else
      m_d.m_fReflectionEnabled = true;
}


void Gate::WriteRegDefaults()
{
   SetRegValueFloat("DefaultProps\\Gate", "Length", m_d.m_length);
   SetRegValueFloat("DefaultProps\\Gate", "Height", m_d.m_height);
   SetRegValueFloat("DefaultProps\\Gate", "Rotation", m_d.m_rotation);
   SetRegValueBool("DefaultProps\\Gate", "ShowBracket", m_d.m_fShowBracket);
   SetRegValueBool("DefaultProps\\Gate", "Collidable", m_d.m_fCollidable);
   SetRegValueFloat("DefaultProps\\Gate", "AngleMin", m_d.m_angleMin);
   SetRegValueFloat("DefaultProps\\Gate", "AngleMax", m_d.m_angleMax);
   SetRegValueBool("DefaultProps\\Gate", "Visible", m_d.m_fVisible);
   SetRegValueBool("DefaultProps\\Gate", "TimerEnabled", m_d.m_tdr.m_fTimerEnabled);
   SetRegValue("DefaultProps\\Gate", "TimerInterval", REG_DWORD, &m_d.m_tdr.m_TimerInterval, 4);
   SetRegValue("DefaultProps\\Gate", "Surface", REG_SZ, &m_d.m_szSurface, lstrlen(m_d.m_szSurface));
   SetRegValueFloat("DefaultProps\\Gate", "Elasticity", m_d.m_elasticity);
   SetRegValueFloat("DefaultProps\\Gate", "Friction", m_d.m_friction);
   SetRegValueFloat("DefaultProps\\Gate", "Scatter", m_d.m_scatter);
   SetRegValueBool("DefaultProps\\Gate", "TwoWay", m_d.m_twoWay);
   SetRegValueBool("DefaultProps\\Gate", "ReflectionEnabled", m_d.m_fReflectionEnabled);
   SetRegValue("DefaultProps\\Gate", "GateType", REG_DWORD, &m_d.m_type, 4);
}

void Gate::PreRender(Sur * const psur)
{
}

void Gate::Render(Sur * const psur)
{
   psur->SetLineColor(RGB(0, 0, 0), false, 2);
   psur->SetObject(this);

   const float halflength = m_d.m_length * 0.5f;
   const float len1 = halflength *0.5f;
   const float len2 = len1 * 0.5f;
   Vertex2D tmp;

   {
      const float radangle = ANGTORAD(m_d.m_rotation);
      {
         const float sn = sinf(radangle);
         const float cs = cosf(radangle);

         psur->Line(m_d.m_vCenter.x + cs*halflength, m_d.m_vCenter.y + sn*halflength,
            m_d.m_vCenter.x - cs*halflength, m_d.m_vCenter.y - sn*halflength);

         // Draw Arrow
         psur->SetLineColor(RGB(0, 0, 0), false, 1);

         tmp.x = m_d.m_vCenter.x + sn*len1;
         tmp.y = m_d.m_vCenter.y - cs*len1;

         psur->Line(tmp.x, tmp.y,
            m_d.m_vCenter.x, m_d.m_vCenter.y);
      }

   {
      const float arrowang = radangle + 0.6f;
      const float sn = sinf(arrowang);
      const float cs = cosf(arrowang);

      psur->Line(tmp.x, tmp.y,
         m_d.m_vCenter.x + sn*len2, m_d.m_vCenter.y - cs*len2);
   }
   }

   {
      const float arrowang = ANGTORAD(m_d.m_rotation) - 0.6f;
      const float sn = sinf(arrowang);
      const float cs = cosf(arrowang);

      psur->Line(tmp.x, tmp.y,
         m_d.m_vCenter.x + sn*len2, m_d.m_vCenter.y - cs*len2);
   }

   if (m_d.m_twoWay)
   {
      const float radangle = ANGTORAD(m_d.m_rotation - 180.f);
      {
         const float sn = sinf(radangle);
         const float cs = cosf(radangle);

         // Draw Arrow
         psur->SetLineColor(RGB(0, 0, 0), false, 1);

         tmp.x = m_d.m_vCenter.x + sn*len1;
         tmp.y = m_d.m_vCenter.y - cs*len1;

         psur->Line(tmp.x, tmp.y,
            m_d.m_vCenter.x, m_d.m_vCenter.y);
      }

       {
          const float arrowang = radangle + 0.6f;
          const float sn = sinf(arrowang);
          const float cs = cosf(arrowang);

          psur->Line(tmp.x, tmp.y,
             m_d.m_vCenter.x + sn*len2, m_d.m_vCenter.y - cs*len2);
       }

       const float arrowang = radangle - 0.6f;
       const float sn = sinf(arrowang);
       const float cs = cosf(arrowang);

       psur->Line(tmp.x, tmp.y,
          m_d.m_vCenter.x + sn*len2, m_d.m_vCenter.y - cs*len2);
   }
}

void Gate::SetDefaultPhysics(bool fromMouseClick)
{
   HRESULT hr;
   float fTmp;

   hr = GetRegStringAsFloat("DefaultProps\\Gate", "Elasticity", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_elasticity = fTmp;
   else
      m_d.m_elasticity = 0.3f;

   hr = GetRegStringAsFloat("DefaultProps\\Gate", "Friction", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_friction = fTmp;
   else
      m_d.m_friction = 0.02f;

   hr = GetRegStringAsFloat("DefaultProps\\Gate", "AntiFriction", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_damping = fTmp;
   else
      m_d.m_damping = 0.985f;

   hr = GetRegStringAsFloat("DefaultProps\\Gate", "Scatter", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_scatter = fTmp;
   else
      m_d.m_scatter = 0;
}

void Gate::RenderBlueprint(Sur *psur, const bool solid)
{
}

void Gate::GetTimers(Vector<HitTimer> * const pvht)
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

void Gate::GetHitShapes(Vector<HitObject> * const pvho)
{
   const float height = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y);
   const float h = m_d.m_height;		//relative height of the gate 

   const float halflength = m_d.m_length * 0.5f;

   const float angleMin = min(m_d.m_angleMin, m_d.m_angleMax); // correct angle inversions
   const float angleMax = max(m_d.m_angleMin, m_d.m_angleMax);

   m_d.m_angleMin = angleMin;
   m_d.m_angleMax = angleMax;

   const float radangle = ANGTORAD(m_d.m_rotation);
   const Vertex2D tangent(cosf(radangle), sinf(radangle));

   const Vertex2D rgv[2] = { //oversize by the ball's radius to prevent the ball from clipping through
      m_d.m_vCenter + (halflength + (float)PHYS_SKIN) * tangent,
      m_d.m_vCenter - (halflength + (float)PHYS_SKIN) * tangent
   };

   if (!m_d.m_twoWay)
   {
      m_plineseg = new LineSeg(rgv[0], rgv[1], height, height + (float)(2.0*PHYS_SKIN)); //!! = ball diameter

      m_plineseg->m_elasticity = m_d.m_elasticity;
      m_plineseg->SetFriction(m_d.m_friction);
      m_plineseg->m_scatter = ANGTORAD(m_d.m_scatter);

      pvho->AddElement(m_plineseg);
   }

   m_phitgate = new HitGate(this, height);
   m_phitgate->m_twoWay = m_d.m_twoWay;
   m_phitgate->m_obj = (IFireEvents*) this;
   m_phitgate->m_fe = true;
   pvho->AddElement(m_phitgate);

   m_phitgate->m_fEnabled = m_d.m_fCollidable;

   if (m_d.m_fShowBracket)
   {
      HitCircle *phitcircle;
      phitcircle = new HitCircle(m_d.m_vCenter + halflength * tangent, 0.01f, height, height + h);
      pvho->AddElement(phitcircle);

      phitcircle = new HitCircle(m_d.m_vCenter - halflength * tangent, 0.01f, height, height + h);
      pvho->AddElement(phitcircle);
   }
}

void Gate::GetHitShapesDebug(Vector<HitObject> * const pvho)
{
}

void Gate::EndPlay()
{
   IEditable::EndPlay();

   m_phitgate = NULL;
   m_plineseg = NULL;

   if (bracketVertexBuffer)
   {
      bracketVertexBuffer->release();
      bracketVertexBuffer = NULL;
   }
   if (bracketIndexBuffer)
   {
      bracketIndexBuffer->release();
      bracketIndexBuffer = NULL;
   }
   if (wireIndexBuffer)
   {
      wireIndexBuffer->release();
      wireIndexBuffer = NULL;
   }
   if (wireVertexBuffer)
   {
      wireVertexBuffer->release();
      wireVertexBuffer = NULL;
      m_vertexbuffer_angle = FLT_MAX;
   }
}

void Gate::UpdateWire(RenderDevice *pd3dDevice)
{
   if (m_phitgate->m_gateMover.m_angle == m_vertexbuffer_angle)
      return;

   m_vertexbuffer_angle = m_phitgate->m_gateMover.m_angle;

   Matrix3D fullMatrix, tempMat;

   fullMatrix.RotateXMatrix(m_d.m_twoWay ? m_phitgate->m_gateMover.m_angle : -m_phitgate->m_gateMover.m_angle);
   tempMat.RotateZMatrix(ANGTORAD(m_d.m_rotation));
   tempMat.Multiply(fullMatrix, fullMatrix);

   Matrix3D vertMatrix;
   tempMat.SetScaling(m_d.m_length, m_d.m_length, m_d.m_length*m_ptable->m_BG_scalez[m_ptable->m_BG_current_set]);
   tempMat.Multiply(fullMatrix, vertMatrix);
   tempMat.SetTranslation(m_d.m_vCenter.x, m_d.m_vCenter.y, m_d.m_height*m_ptable->m_BG_scalez[m_ptable->m_BG_current_set] + baseHeight);
   tempMat.Multiply(vertMatrix, vertMatrix);

   Vertex3D_NoTex2 *buf;
   wireVertexBuffer->lock(0, 0, (void**)&buf, VertexBuffer::DISCARDCONTENTS);
   for (unsigned int i = 0; i < m_numVertices; i++)
   {
      Vertex3Ds vert(m_vertices[i].x, m_vertices[i].y, m_vertices[i].z);
      vert = vertMatrix.MultiplyVector(vert);
      buf[i].x = vert.x;
      buf[i].y = vert.y;
      buf[i].z = vert.z;

      vert = Vertex3Ds(m_vertices[i].nx, m_vertices[i].ny, m_vertices[i].nz);
      vert = fullMatrix.MultiplyVectorNoTranslate(vert);
      buf[i].nx = vert.x;
      buf[i].ny = vert.y;
      buf[i].nz = vert.z;
      buf[i].tu = m_vertices[i].tu;
      buf[i].tv = m_vertices[i].tv;
   }
   wireVertexBuffer->unlock();
}

void Gate::RenderObject(RenderDevice* pd3dDevice)
{
   UpdateWire(pd3dDevice);

   const Material * const mat = m_ptable->GetMaterial(m_d.m_szMaterial);
   pd3dDevice->basicShader->SetMaterial(mat);

   Pin3D * const ppin3d = &g_pplayer->m_pin3d;
   ppin3d->EnableAlphaBlend(false);

   pd3dDevice->SetRenderState(RenderDevice::DEPTHBIAS, 0);
   pd3dDevice->SetRenderState(RenderDevice::ZWRITEENABLE, TRUE);
   pd3dDevice->SetRenderState(RenderDevice::CULLMODE, D3DCULL_CCW);

   pd3dDevice->basicShader->SetTechnique(mat->m_bIsMetal ? "basic_without_texture_isMetal" : "basic_without_texture_isNotMetal");
   pd3dDevice->basicShader->Begin(0);

   // render bracket      
   if (m_d.m_fShowBracket)
      pd3dDevice->DrawIndexedPrimitiveVB(D3DPT_TRIANGLELIST, MY_D3DFVF_NOTEX2_VERTEX, bracketVertexBuffer, 0, gateBracketNumVertices, bracketIndexBuffer, 0, gateBracketNumIndices);
   // render wire
   pd3dDevice->DrawIndexedPrimitiveVB(D3DPT_TRIANGLELIST, MY_D3DFVF_NOTEX2_VERTEX, wireVertexBuffer, 0, m_numVertices, wireIndexBuffer, 0, m_numIndices);

   pd3dDevice->basicShader->End();
}

void Gate::PostRenderStatic(RenderDevice* pd3dDevice)
{
   TRACE_FUNCTION();

   if (!m_phitgate->m_gateMover.m_fVisible)
      return;

   if (m_ptable->m_fReflectionEnabled && !m_d.m_fReflectionEnabled)
      return;

   RenderObject(pd3dDevice);
}

void Gate::ExportMesh(FILE *f)
{
   char name[MAX_PATH];
   char subName[MAX_PATH];
   Vertex3D_NoTex2 *buf;

   WideCharToMultiByte(CP_ACP, 0, m_wzName, -1, name, MAX_PATH, NULL, NULL);
   baseHeight = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y)*m_ptable->m_BG_scalez[m_ptable->m_BG_current_set];

   if (m_d.m_fShowBracket)
   {
      buf = new Vertex3D_NoTex2[gateBracketNumVertices];
      strcpy_s(subName, name);
      strcat_s(subName, "Bracket");
      WaveFrontObj_WriteObjectName(f, subName);
      GenerateBracketMesh(buf);
      WaveFrontObj_WriteVertexInfo(f, buf, gateBracketNumVertices);
      const Material * const mat = m_ptable->GetMaterial(m_d.m_szMaterial);
      WaveFrontObj_WriteMaterial(m_d.m_szMaterial, NULL, mat);
      WaveFrontObj_UseTexture(f, m_d.m_szMaterial);
      WaveFrontObj_WriteFaceInfoList(f, gateBracketIndices, gateBracketNumIndices);
      WaveFrontObj_UpdateFaceOffset(gateBracketNumVertices);
      delete[] buf;
   }

   SetGateType(m_d.m_type);

   buf = new Vertex3D_NoTex2[m_numVertices];
   strcpy_s(subName, name);
   strcat_s(subName, "Wire");
   WaveFrontObj_WriteObjectName(f, subName);
   GenerateWireMesh(buf);
   WaveFrontObj_WriteVertexInfo(f, buf, m_numVertices);
   const Material * const mat = m_ptable->GetMaterial(m_d.m_szMaterial);
   WaveFrontObj_WriteMaterial(m_d.m_szMaterial, NULL, mat);
   WaveFrontObj_UseTexture(f, m_d.m_szMaterial);
   WaveFrontObj_WriteFaceInfoList(f, m_indices, m_numIndices);
   WaveFrontObj_UpdateFaceOffset(m_numVertices);
   delete[] buf;
}

void Gate::GenerateBracketMesh(Vertex3D_NoTex2 *buf)
{
   Matrix3D fullMatrix;
   fullMatrix.RotateZMatrix(ANGTORAD(m_d.m_rotation));
   for (int i = 0; i < gateBracketNumVertices; i++)
   {
      Vertex3Ds vert(gateBracket[i].x, gateBracket[i].y, gateBracket[i].z);
      vert = fullMatrix.MultiplyVector(vert);
      buf[i].x = vert.x*m_d.m_length + m_d.m_vCenter.x;
      buf[i].y = vert.y*m_d.m_length + m_d.m_vCenter.y;
      buf[i].z = vert.z*m_d.m_length*m_ptable->m_BG_scalez[m_ptable->m_BG_current_set] + (m_d.m_height*m_ptable->m_BG_scalez[m_ptable->m_BG_current_set] + baseHeight);

      vert = Vertex3Ds(gateBracket[i].nx, gateBracket[i].ny, gateBracket[i].nz);
      vert = fullMatrix.MultiplyVectorNoTranslate(vert);
      buf[i].nx = vert.x;
      buf[i].ny = vert.y;
      buf[i].nz = vert.z;
      buf[i].tu = gateBracket[i].tu;
      buf[i].tv = gateBracket[i].tv;
   }
}

void Gate::GenerateWireMesh(Vertex3D_NoTex2 *buf)
{
   Matrix3D fullMatrix;
   fullMatrix.RotateZMatrix(ANGTORAD(m_d.m_rotation));

   for (unsigned int i = 0; i < m_numVertices; i++)
   {
      Vertex3Ds vert(m_vertices[i].x, m_vertices[i].y, m_vertices[i].z);
      vert = fullMatrix.MultiplyVector(vert);
      buf[i].x = vert.x*m_d.m_length + m_d.m_vCenter.x;
      buf[i].y = vert.y*m_d.m_length + m_d.m_vCenter.y;
      buf[i].z = vert.z*m_d.m_length*m_ptable->m_BG_scalez[m_ptable->m_BG_current_set] + (m_d.m_height*m_ptable->m_BG_scalez[m_ptable->m_BG_current_set] + baseHeight);

      vert = Vertex3Ds(m_vertices[i].nx, m_vertices[i].ny, m_vertices[i].nz);
      vert = fullMatrix.MultiplyVectorNoTranslate(vert);
      buf[i].nx = vert.x;
      buf[i].ny = vert.y;
      buf[i].nz = vert.z;
      buf[i].tu = m_vertices[i].tu;
      buf[i].tv = m_vertices[i].tv;
   }
}

void Gate::RenderSetup(RenderDevice* pd3dDevice)
{
   if (bracketIndexBuffer)
      bracketIndexBuffer->release();
   bracketIndexBuffer = pd3dDevice->CreateAndFillIndexBuffer(gateBracketNumIndices, gateBracketIndices);

   if (bracketVertexBuffer)
      bracketVertexBuffer->release();
   pd3dDevice->CreateVertexBuffer(gateBracketNumVertices, 0, MY_D3DFVF_NOTEX2_VERTEX, &bracketVertexBuffer);

   SetGateType(m_d.m_type);

   baseHeight = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y)*m_ptable->m_BG_scalez[m_ptable->m_BG_current_set];

   Vertex3D_NoTex2 *buf;
   bracketVertexBuffer->lock(0, 0, (void**)&buf, VertexBuffer::WRITEONLY);
   GenerateBracketMesh(buf);
   bracketVertexBuffer->unlock();

   if (wireIndexBuffer)
      wireIndexBuffer->release();
   wireIndexBuffer = pd3dDevice->CreateAndFillIndexBuffer(m_numIndices, m_indices);

   if (wireVertexBuffer)
      wireVertexBuffer->release();
   pd3dDevice->CreateVertexBuffer(m_numVertices, USAGE_DYNAMIC, MY_D3DFVF_NOTEX2_VERTEX, &wireVertexBuffer);

   wireVertexBuffer->lock(0, 0, (void**)&buf, VertexBuffer::DISCARDCONTENTS);
   GenerateWireMesh(buf);
   wireVertexBuffer->unlock();
}

void Gate::RenderStatic(RenderDevice* pd3dDevice) // only the support structures are rendered here
{
}

void Gate::SetObjectPos()
{
   g_pvp->SetObjectPosCur(m_d.m_vCenter.x, m_d.m_vCenter.y);
}

void Gate::MoveOffset(const float dx, const float dy)
{
   m_d.m_vCenter.x += dx;
   m_d.m_vCenter.y += dy;

   m_ptable->SetDirtyDraw();
}

void Gate::GetCenter(Vertex2D * const pv) const
{
   *pv = m_d.m_vCenter;
}

void Gate::PutCenter(const Vertex2D * const pv)
{
   m_d.m_vCenter = *pv;

   m_ptable->SetDirtyDraw();
}

HRESULT Gate::SaveData(IStream *pstm, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
{
   BiffWriter bw(pstm, hcrypthash, hcryptkey);

   bw.WriteStruct(FID(VCEN), &m_d.m_vCenter, sizeof(Vertex2D));
   bw.WriteFloat(FID(LGTH), m_d.m_length);
   bw.WriteFloat(FID(HGTH), m_d.m_height);
   bw.WriteFloat(FID(ROTA), m_d.m_rotation);
   bw.WriteString(FID(MATR), m_d.m_szMaterial);
   bw.WriteBool(FID(TMON), m_d.m_tdr.m_fTimerEnabled);
   bw.WriteBool(FID(GSUPT), m_d.m_fShowBracket);
   bw.WriteBool(FID(GCOLD), m_d.m_fCollidable);
   bw.WriteInt(FID(TMIN), m_d.m_tdr.m_TimerInterval);
   bw.WriteString(FID(SURF), m_d.m_szSurface);
   bw.WriteFloat(FID(ELAS), m_d.m_elasticity);
   bw.WriteFloat(FID(GAMAX), m_d.m_angleMax);
   bw.WriteFloat(FID(GAMIN), m_d.m_angleMin);
   bw.WriteFloat(FID(GFRCT), m_d.m_friction);
   bw.WriteFloat(FID(AFRC), m_d.m_damping);
   bw.WriteBool(FID(GVSBL), m_d.m_fVisible);
   bw.WriteWideString(FID(NAME), (WCHAR *)m_wzName);
   bw.WriteBool(FID(TWWA), m_d.m_twoWay);
   bw.WriteBool(FID(REEN), m_d.m_fReflectionEnabled);
   bw.WriteInt(FID(GATY), m_d.m_type);

   ISelect::SaveData(pstm, hcrypthash, hcryptkey);

   bw.WriteTag(FID(ENDB));

   return S_OK;
}

HRESULT Gate::InitLoad(IStream *pstm, PinTable *ptable, int *pid, int version, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
{
   SetDefaults(false);

   BiffReader br(pstm, this, pid, version, hcrypthash, hcryptkey);

   m_ptable = ptable;

   br.Load();
   return S_OK;
}

BOOL Gate::LoadToken(int id, BiffReader *pbr)
{
   if (id == FID(PIID))
   {
      pbr->GetInt((int *)pbr->m_pdata);
   }
   else if (id == FID(GATY))
   {
       pbr->GetInt(&m_d.m_type);
   }
   else if (id == FID(VCEN))
   {
      pbr->GetStruct(&m_d.m_vCenter, sizeof(Vertex2D));
   }
   else if (id == FID(LGTH))
   {
      pbr->GetFloat(&m_d.m_length);
   }
   else if (id == FID(HGTH))
   {
      pbr->GetFloat(&m_d.m_height);
   }
   else if (id == FID(ROTA))
   {
      pbr->GetFloat(&m_d.m_rotation);
   }
   else if (id == FID(MATR))
   {
      pbr->GetString(m_d.m_szMaterial);
   }
   else if (id == FID(TMON))
   {
      pbr->GetBool(&m_d.m_tdr.m_fTimerEnabled);
   }
   else if (id == FID(GSUPT))
   {
      pbr->GetBool(&m_d.m_fShowBracket);
   }
   else if (id == FID(GCOLD))
   {
      pbr->GetBool(&m_d.m_fCollidable);
   }
   else if (id == FID(TWWA))
   {
      pbr->GetBool(&m_d.m_twoWay);
   }
   else if (id == FID(GVSBL))
   {
      pbr->GetBool(&m_d.m_fVisible);
   }
   else if (id == FID(REEN))
   {
      pbr->GetBool(&m_d.m_fReflectionEnabled);
   }
   else if (id == FID(TMIN))
   {
      pbr->GetInt(&m_d.m_tdr.m_TimerInterval);
   }
   else if (id == FID(SURF))
   {
      pbr->GetString(m_d.m_szSurface);
   }
   else if (id == FID(NAME))
   {
      pbr->GetWideString((WCHAR *)m_wzName);
   }
   else if (id == FID(ELAS))
   {
      pbr->GetFloat(&m_d.m_elasticity);
   }
   else if (id == FID(GAMAX))
   {
      pbr->GetFloat(&m_d.m_angleMax);
   }
   else if (id == FID(GAMIN))
   {
      pbr->GetFloat(&m_d.m_angleMin);
   }
   else if (id == FID(GFRCT))
   {
      pbr->GetFloat(&m_d.m_friction);
   }
   else if (id == FID(AFRC))
   {
      pbr->GetFloat(&m_d.m_damping);
   }
   else
   {
      ISelect::LoadToken(id, pbr);
   }

   return fTrue;
}

HRESULT Gate::InitPostLoad()
{
   return S_OK;
}

STDMETHODIMP Gate::InterfaceSupportsErrorInfo(REFIID riid)
{
   static const IID* arr[] =
   {
      &IID_IGate,
   };

   for (size_t i = 0; i < sizeof(arr) / sizeof(arr[0]); i++)
   {
      if (InlineIsEqualGUID(*arr[i], riid))
         return S_OK;
   }
   return S_FALSE;
}

STDMETHODIMP Gate::get_Length(float *pVal)
{
   *pVal = m_d.m_length;
   UpdateUnitsInfo();

   return S_OK;
}

STDMETHODIMP Gate::put_Length(float newVal)
{
   STARTUNDO

      m_d.m_length = newVal;
   UpdateUnitsInfo();

   STOPUNDO

      return S_OK;
}

STDMETHODIMP Gate::get_Height(float *pVal)
{
   *pVal = m_d.m_height;
   UpdateUnitsInfo();
   return S_OK;
}

STDMETHODIMP Gate::put_Height(float newVal)
{
   STARTUNDO

      m_d.m_height = newVal;
   UpdateUnitsInfo();

   STOPUNDO

      return S_OK;
}

STDMETHODIMP Gate::get_Rotation(float *pVal)
{
   *pVal = m_d.m_rotation;

   return S_OK;
}

STDMETHODIMP Gate::put_Rotation(float newVal)
{
   STARTUNDO

      m_d.m_rotation = newVal;

   STOPUNDO;

   return S_OK;
}

STDMETHODIMP Gate::get_X(float *pVal)
{
   *pVal = m_d.m_vCenter.x;

   return S_OK;
}

STDMETHODIMP Gate::put_X(float newVal)
{
   STARTUNDO

      m_d.m_vCenter.x = newVal;

   STOPUNDO

      return S_OK;
}

STDMETHODIMP Gate::get_Y(float *pVal)
{
   *pVal = m_d.m_vCenter.y;

   return S_OK;
}

STDMETHODIMP Gate::put_Y(float newVal)
{
   STARTUNDO

      m_d.m_vCenter.y = newVal;

   STOPUNDO

      return S_OK;
}

STDMETHODIMP Gate::get_Surface(BSTR *pVal)
{
   WCHAR wz[512];
   MultiByteToWideChar(CP_ACP, 0, m_d.m_szSurface, -1, wz, 32);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP Gate::put_Surface(BSTR newVal)
{
   STARTUNDO

      WideCharToMultiByte(CP_ACP, 0, newVal, -1, m_d.m_szSurface, 32, NULL, NULL);

   STOPUNDO

      return S_OK;
}

STDMETHODIMP Gate::get_Material(BSTR *pVal)
{
   WCHAR wz[512];

   MultiByteToWideChar(CP_ACP, 0, m_d.m_szMaterial, -1, wz, 32);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP Gate::put_Material(BSTR newVal)
{
   STARTUNDO

      WideCharToMultiByte(CP_ACP, 0, newVal, -1, m_d.m_szMaterial, 32, NULL, NULL);

   STOPUNDO

      return S_OK;
}

STDMETHODIMP Gate::get_Open(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB((m_phitgate) ? m_phitgate->m_gateMover.m_fOpen : false);

   return S_OK;
}

STDMETHODIMP Gate::put_Open(VARIANT_BOOL newVal)
{
   if (m_phitgate)
   {
      m_phitgate->m_gateMover.m_angleMax = m_d.m_angleMax;
      m_phitgate->m_gateMover.m_angleMin = m_d.m_angleMin;
      m_phitgate->m_gateMover.m_forcedMove = true;

      if (newVal)
      {
         m_phitgate->m_gateMover.m_fOpen = true;
         m_phitgate->m_fEnabled = false;
         if (!m_d.m_twoWay)
            m_plineseg->m_fEnabled = false;

         if (m_phitgate->m_gateMover.m_angle < m_phitgate->m_gateMover.m_angleMax)
            m_phitgate->m_gateMover.m_anglespeed = 0.2f;
      }
      else
      {
         m_phitgate->m_gateMover.m_fOpen = false;

         m_phitgate->m_fEnabled = m_d.m_fCollidable;
         if (!m_d.m_twoWay)
            m_plineseg->m_fEnabled = m_d.m_fCollidable;

         if (m_phitgate->m_gateMover.m_angle > m_phitgate->m_gateMover.m_angleMin)
            m_phitgate->m_gateMover.m_anglespeed = -0.2f;
      }
   }

   return S_OK;
}

STDMETHODIMP Gate::get_Elasticity(float *pVal)
{
   *pVal = m_d.m_elasticity;

   return S_OK;
}

STDMETHODIMP Gate::put_Elasticity(float newVal)
{
   STARTUNDO

      m_d.m_elasticity = newVal;

   STOPUNDO

      return S_OK;
}

STDMETHODIMP Gate::get_ShowBracket(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_d.m_fShowBracket);

   return S_OK;
}

STDMETHODIMP Gate::put_ShowBracket(VARIANT_BOOL newVal)
{
   STARTUNDO

      m_d.m_fShowBracket = VBTOF(newVal);

   STOPUNDO

      return S_OK;
}

STDMETHODIMP Gate::get_CloseAngle(float *pVal)
{
   *pVal = RADTOANG(g_pplayer ? m_phitgate->m_gateMover.m_angleMin : m_d.m_angleMin);

   return S_OK;
}

STDMETHODIMP Gate::put_CloseAngle(float newVal)
{
   if (m_d.m_fCollidable)
   {
      newVal = 0;
      ShowError("Gate is collidable! closing angles other than 0 aren't possible!");
   }
   else newVal = ANGTORAD(newVal);

   if (g_pplayer)
   {
      if (newVal > m_d.m_angleMax) newVal = m_d.m_angleMax;
      else if (newVal < m_d.m_angleMin) newVal = m_d.m_angleMin;

      if (m_phitgate->m_gateMover.m_angleMax > newVal)	// max is bigger
         m_phitgate->m_gateMover.m_angleMin = newVal;	//then set new minumum
      else m_phitgate->m_gateMover.m_angleMax = newVal;//else set new max
   }
   else
   {
      STARTUNDO
         m_d.m_angleMin = newVal;
      STOPUNDO
   }

   return S_OK;
}


STDMETHODIMP Gate::get_OpenAngle(float *pVal)
{
   *pVal = RADTOANG((g_pplayer) ? m_phitgate->m_gateMover.m_angleMax : m_d.m_angleMax);	//player active value

   return S_OK;
}

STDMETHODIMP Gate::put_OpenAngle(float newVal)
{
   if (m_d.m_fCollidable)
   {
      newVal = (float)(M_PI / 2.0);
      ShowError("Gate is collidable! open angles other than 90 aren't possible!");
   }
   else newVal = ANGTORAD(newVal);
   if (g_pplayer)
   {
      if (newVal > m_d.m_angleMax) newVal = m_d.m_angleMax;
      else if (newVal < m_d.m_angleMin) newVal = m_d.m_angleMin;

      if (m_phitgate->m_gateMover.m_angleMin < newVal)	// min is smaller
         m_phitgate->m_gateMover.m_angleMax = newVal;	//then set new maximum
      else m_phitgate->m_gateMover.m_angleMin = newVal;  //else set new min
   }
   else
   {
      STARTUNDO
         m_d.m_angleMax = newVal;
      STOPUNDO
   }
   return S_OK;
}


STDMETHODIMP Gate::get_Collidable(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB((g_pplayer) ? m_phitgate->m_fEnabled : m_d.m_fCollidable);

   return S_OK;
}


STDMETHODIMP Gate::put_Collidable(VARIANT_BOOL newVal)
{
   if (g_pplayer)
   {
      m_phitgate->m_fEnabled = VBTOF(newVal);
      if (!m_d.m_twoWay)
         m_plineseg->m_fEnabled = VBTOF(newVal);
      m_phitgate->m_gateMover.m_angleMax = m_d.m_angleMax;
      m_phitgate->m_gateMover.m_angleMin = m_d.m_angleMin;

      if (newVal) m_phitgate->m_gateMover.m_angleMin = 0;
   }
   else
   {
      STARTUNDO

         m_d.m_fCollidable = VBTOF(newVal);

      if (newVal) m_d.m_angleMin = 0;

      STOPUNDO
   }

   return S_OK;
}

STDMETHODIMP Gate::Move(int dir, float speed, float angle) //move non-collidable gate, for graphic effects only
{
   if (g_pplayer)
   {
      m_phitgate->m_gateMover.m_forcedMove = true;
      m_phitgate->m_gateMover.m_fOpen = true;// move always turns off natural swing
      m_phitgate->m_fEnabled = false;		// and collidable off
      if (!m_d.m_twoWay)
         m_plineseg->m_fEnabled = false;

      if (speed <= 0.0f) speed = 0.2f;		//default gate angle speed
      else speed *= (float)(M_PI / 180.0);	// convert to radians

      if (!dir || angle != 0)				// if no direction or non-zero angle
      {
         angle *= (float)(M_PI / 180.0);		// convert to radians

         if (angle < m_d.m_angleMin) angle = m_d.m_angleMin;
         else if (angle > m_d.m_angleMax) angle = m_d.m_angleMax;

         const float da = angle - m_phitgate->m_gateMover.m_angle; //calc true direction

         if (da > 1.0e-5f) dir = +1;
         else if (da < -1.0e-5f) dir = -1;
         else
         {
            dir = 0;									// do nothing
            m_phitgate->m_gateMover.m_anglespeed = 0;	//stop 
         }
      }
      else { angle = (dir < 0) ? m_d.m_angleMin : m_d.m_angleMax; }	//dir selected and angle not specified			

      if (dir > 0)
      {
         m_phitgate->m_gateMover.m_angleMax = angle;

         if (m_phitgate->m_gateMover.m_angle < m_phitgate->m_gateMover.m_angleMax)
            m_phitgate->m_gateMover.m_anglespeed = speed;
      }
      else if (dir < 0)
      {
         m_phitgate->m_gateMover.m_angleMin = angle;

         if (m_phitgate->m_gateMover.m_angle > m_phitgate->m_gateMover.m_angleMin)
            m_phitgate->m_gateMover.m_anglespeed = -speed;
      }
   }

   return S_OK;
}

STDMETHODIMP Gate::get_Friction(float *pVal)
{
   *pVal = (g_pplayer) ? m_phitgate->m_gateMover.m_friction : m_d.m_friction;

   return S_OK;
}

STDMETHODIMP Gate::put_Friction(float newVal)
{
   newVal = clamp(newVal, 0.f,1.f);

   if (g_pplayer)
   {
      m_phitgate->m_gateMover.m_friction = newVal;
   }
   else
   {
      STARTUNDO

      m_d.m_friction = newVal;

      STOPUNDO
   }

   return S_OK;
}

STDMETHODIMP Gate::get_Damping(float *pVal)
{
   *pVal = !g_pplayer ? m_d.m_damping : powf(m_phitgate->m_gateMover.m_damping,(float)(1.0/PHYS_FACTOR));

   return S_OK;
}

STDMETHODIMP Gate::put_Damping(float newVal)
{
   const float tmp = clamp(newVal, 0.0f, 1.0f);
   if (g_pplayer)
      m_phitgate->m_gateMover.m_damping = powf(tmp, (float)PHYS_FACTOR); //0.996f;
   else
   {
      STARTUNDO

      m_d.m_damping = tmp;

      STOPUNDO
   }

   return S_OK;
}

STDMETHODIMP Gate::get_Visible(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB((g_pplayer) ? m_phitgate->m_gateMover.m_fVisible : m_d.m_fVisible);

   return S_OK;
}

STDMETHODIMP Gate::put_Visible(VARIANT_BOOL newVal)
{
   if (g_pplayer)
   {
      m_phitgate->m_gateMover.m_fVisible = VBTOF(newVal);
   }
   else
   {
      STARTUNDO

         m_d.m_fVisible = VBTOF(newVal);

      STOPUNDO
   }

   return S_OK;
}

STDMETHODIMP Gate::get_TwoWay(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_d.m_twoWay);

   return S_OK;
}

STDMETHODIMP Gate::put_TwoWay(VARIANT_BOOL newVal)
{
   if (g_pplayer)
   {
      m_phitgate->m_twoWay = VBTOF(newVal);
   }
   else
   {
      STARTUNDO

         m_d.m_twoWay = VBTOF(newVal);

      STOPUNDO
   }

   return S_OK;
}

STDMETHODIMP Gate::get_ReflectionEnabled(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_d.m_fReflectionEnabled);

   return S_OK;
}

STDMETHODIMP Gate::put_ReflectionEnabled(VARIANT_BOOL newVal)
{
   STARTUNDO

      m_d.m_fReflectionEnabled = VBTOF(newVal);

   STOPUNDO

      return S_OK;
}

STDMETHODIMP Gate::get_CurrentAngle(float *pVal)
{
   if (g_pplayer)
   {
      *pVal = RADTOANG(m_phitgate->m_gateMover.m_angle);
      return S_OK;
   }
   else
      return E_FAIL;
}

STDMETHODIMP Gate::get_DrawStyle(GateType *pVal)
{
    *pVal = m_d.m_type;

    return S_OK;
}

STDMETHODIMP Gate::put_DrawStyle(GateType newVal)
{
    STARTUNDO

        m_d.m_type = newVal;

    STOPUNDO

        return S_OK;
}

void Gate::GetDialogPanes(Vector<PropertyPane> *pvproppane)
{
   PropertyPane *pproppane;

   pproppane = new PropertyPane(IDD_PROP_NAME, NULL);
   pvproppane->AddElement(pproppane);

   pproppane = new PropertyPane(IDD_PROPGATE_VISUALS, IDS_VISUALS);
   pvproppane->AddElement(pproppane);

   pproppane = new PropertyPane(IDD_PROPGATE_POSITION, IDS_POSITION);
   pvproppane->AddElement(pproppane);

   pproppane = new PropertyPane(IDD_PROPGATE_PHYSICS, IDS_PHYSICS);
   pvproppane->AddElement(pproppane);

   pproppane = new PropertyPane(IDD_PROP_TIMER, IDS_MISC);
   pvproppane->AddElement(pproppane);
}
