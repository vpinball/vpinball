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
   m_bracketIndexBuffer = NULL;
   m_bracketVertexBuffer = NULL;
   m_wireIndexBuffer = NULL;
   m_wireVertexBuffer = NULL;
   m_vertexbuffer_angle = FLT_MAX;
   memset(m_d.m_szSurface, 0, sizeof(m_d.m_szSurface));
   m_d.m_type = GateWireW;
   m_vertices = 0;
   m_indices = 0;
   m_numIndices = 0;
   m_numVertices = 0;
}

void Gate::SetGateType(GateType type)
{
    switch (m_d.m_type)
    {
    case GateWireW:
    {
        m_vertices = gateWire;
        m_indices = gateWireIndices;
        m_numIndices = gateWireNumIndices;
        m_numVertices = gateWireNumVertices;
        break;
    }
    case GateWireRectangle:
    {
        m_vertices = gateWireRectangleMesh;
        m_indices = gateWireRectangleIndices;
        m_numIndices = gateWireRectangleNumIndices;
        m_numVertices = gateWireRectangleNumVertices;
        break;
    }
    case GateLongPlate:
    {
        m_vertices = gateLongPlateMesh;
        m_indices = gateLongPlateIndices;
        m_numIndices = gateLongPlateNumIndices;
        m_numVertices = gateLongPlateVertices;
        break;
    }
    case GatePlate:
    {
        m_vertices = gatePlateMesh;
        m_indices = gatePlateIndices;
        m_numIndices = gatePlateNumIndices;
        m_numVertices = gatePlateNumVertices;
        break;
    }
    default:
        ShowError("Unknown Gate type");
        break;
    }
}

Gate::~Gate()
{
   if (m_bracketVertexBuffer)
   {
      m_bracketVertexBuffer->release();
      m_bracketVertexBuffer = NULL;
   }
   if (m_bracketIndexBuffer)
   {
      m_bracketIndexBuffer->release();
      m_bracketIndexBuffer = NULL;
   }
   if (m_wireIndexBuffer)
   {
      m_wireIndexBuffer->release();
      m_wireIndexBuffer = NULL;
   }
   if (m_wireVertexBuffer)
   {
      m_wireVertexBuffer->release();
      m_wireVertexBuffer = NULL;
   }
}

void Gate::UpdateStatusBarInfo()
{
   char tbuf[128];
   sprintf_s(tbuf, "Length: %.3f | Height: %.3f", m_vpinball->ConvertToUnit(m_d.m_length), m_vpinball->ConvertToUnit(m_d.m_height));
   m_vpinball->SetStatusBarUnitInfo(tbuf, true);
}

HRESULT Gate::Init(PinTable *ptable, float x, float y, bool fromMouseClick)
{
   m_ptable = ptable;

   m_d.m_vCenter.x = x;
   m_d.m_vCenter.y = y;

   SetDefaults(fromMouseClick);

   InitVBA(fTrue, 0, NULL);

   return S_OK;
}

void Gate::SetDefaults(bool fromMouseClick)
{
   m_d.m_length = fromMouseClick ? LoadValueFloatWithDefault("DefaultProps\\Gate", "Length", 100.f) : 100.f;
   m_d.m_height = fromMouseClick ? LoadValueFloatWithDefault("DefaultProps\\Gate", "Height", 50.f) : 50.f;
   m_d.m_rotation = fromMouseClick ? LoadValueFloatWithDefault("DefaultProps\\Gate", "Rotation", -90.f) : -90.f;
   m_d.m_showBracket = fromMouseClick ? LoadValueBoolWithDefault("DefaultProps\\Gate", "ShowBracket", true) : true;
   m_d.m_type = fromMouseClick ? (GateType)LoadValueIntWithDefault("DefaultProps\\Gate", "GateType", GateWireW) : GateWireW;
   m_d.m_collidable = fromMouseClick ? LoadValueBoolWithDefault("DefaultProps\\Gate", "Collidable", true) : true;
   m_d.m_angleMin = fromMouseClick ? LoadValueFloatWithDefault("DefaultProps\\Gate", "AngleMin", 0.f) : 0.f;
   m_d.m_angleMax = fromMouseClick ? LoadValueFloatWithDefault("DefaultProps\\Gate", "AngleMax", (float)(M_PI / 2.0)) : (float)(M_PI / 2.0);
   m_d.m_visible = fromMouseClick ? LoadValueBoolWithDefault("DefaultProps\\Gate", "Visible", true) : true;
   m_d.m_tdr.m_TimerEnabled = fromMouseClick ? LoadValueBoolWithDefault("DefaultProps\\Gate", "TimerEnabled", false) : false;
   m_d.m_tdr.m_TimerInterval = fromMouseClick ? LoadValueIntWithDefault("DefaultProps\\Gate", "TimerInterval", 100) : 100;

   const HRESULT hr = LoadValueString("DefaultProps\\Gate", "Surface", m_d.m_szSurface, MAXTOKEN);
   if ((hr != S_OK) || !fromMouseClick)
      m_d.m_szSurface[0] = 0;

   SetDefaultPhysics(fromMouseClick);

   m_d.m_twoWay = fromMouseClick ? LoadValueBoolWithDefault("DefaultProps\\Gate", "TwoWay", true) : true;
   m_d.m_reflectionEnabled = fromMouseClick ? LoadValueBoolWithDefault("DefaultProps\\Gate", "ReflectionEnabled", true) : true;
}


void Gate::WriteRegDefaults()
{
   SaveValueFloat("DefaultProps\\Gate", "Length", m_d.m_length);
   SaveValueFloat("DefaultProps\\Gate", "Height", m_d.m_height);
   SaveValueFloat("DefaultProps\\Gate", "Rotation", m_d.m_rotation);
   SaveValueBool("DefaultProps\\Gate", "ShowBracket", m_d.m_showBracket);
   SaveValueBool("DefaultProps\\Gate", "Collidable", m_d.m_collidable);
   SaveValueFloat("DefaultProps\\Gate", "AngleMin", m_d.m_angleMin);
   SaveValueFloat("DefaultProps\\Gate", "AngleMax", m_d.m_angleMax);
   SaveValueBool("DefaultProps\\Gate", "Visible", m_d.m_visible);
   SaveValueBool("DefaultProps\\Gate", "TimerEnabled", m_d.m_tdr.m_TimerEnabled);
   SaveValueInt("DefaultProps\\Gate", "TimerInterval", m_d.m_tdr.m_TimerInterval);
   SaveValueString("DefaultProps\\Gate", "Surface", m_d.m_szSurface);
   SaveValueFloat("DefaultProps\\Gate", "Elasticity", m_d.m_elasticity);
   SaveValueFloat("DefaultProps\\Gate", "Friction", m_d.m_friction);
   SaveValueFloat("DefaultProps\\Gate", "Scatter", m_d.m_scatter);
   SaveValueFloat("DefaultProps\\Gate", "GravityFactor", m_d.m_gravityfactor);
   SaveValueBool("DefaultProps\\Gate", "TwoWay", m_d.m_twoWay);
   SaveValueBool("DefaultProps\\Gate", "ReflectionEnabled", m_d.m_reflectionEnabled);
   SaveValueInt("DefaultProps\\Gate", "GateType", m_d.m_type);
}

float Gate::GetOpenAngle() const
{
    return RADTOANG(g_pplayer ? m_phitgate->m_gateMover.m_angleMax : m_d.m_angleMax);	// player active value
}

void Gate::SetOpenAngle(const float angle)
{
    float newVal = ANGTORAD(angle);
    if (g_pplayer)
    {
        if (newVal > m_d.m_angleMax) newVal = m_d.m_angleMax;
        else if (newVal < m_d.m_angleMin) newVal = m_d.m_angleMin;

        if (m_phitgate->m_gateMover.m_angleMin < newVal)  // min is smaller
            m_phitgate->m_gateMover.m_angleMax = newVal;  // then set new maximum
        else m_phitgate->m_gateMover.m_angleMin = newVal; // else set new min
    }
    else
        m_d.m_angleMax = newVal;
}

float Gate::GetCloseAngle() const
{
    return RADTOANG(g_pplayer ? m_phitgate->m_gateMover.m_angleMin : m_d.m_angleMin);
}

void Gate::SetCloseAngle(const float angle)
{
    float newVal = ANGTORAD(angle);
    if (g_pplayer)
    {
        if (newVal > m_d.m_angleMax) newVal = m_d.m_angleMax;
        else if (newVal < m_d.m_angleMin) newVal = m_d.m_angleMin;

        if (m_phitgate->m_gateMover.m_angleMax > newVal)  // max is bigger
            m_phitgate->m_gateMover.m_angleMin = newVal;  // then set new minumum
        else m_phitgate->m_gateMover.m_angleMax = newVal; // else set new max
    }
    else
        m_d.m_angleMin = newVal;
}

void Gate::UIRenderPass1(Sur * const psur)
{
}

void Gate::UIRenderPass2(Sur * const psur)
{
   psur->SetBorderColor(RGB(0, 0, 0), false, 0);
   psur->SetLineColor(RGB(0, 0, 0), false, 2);
   psur->SetObject(this);

   const float halflength = m_d.m_length * 0.5f;
   const float len1 = halflength *0.5f;
   const float len2 = len1 * 0.5f;
   Vertex2D tmp;

   psur->Ellipse(m_d.m_vCenter.x, m_d.m_vCenter.y, halflength);

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
   m_d.m_elasticity = fromMouseClick ? LoadValueFloatWithDefault("DefaultProps\\Gate", "Elasticity", 0.3f) : 0.3f;
   m_d.m_friction = fromMouseClick ? LoadValueFloatWithDefault("DefaultProps\\Gate", "Friction", 0.02f) : 0.02f;
   m_d.m_damping = fromMouseClick ? LoadValueFloatWithDefault("DefaultProps\\Gate", "AntiFriction", 0.985f) : 0.985f;
   m_d.m_scatter = fromMouseClick ? LoadValueFloatWithDefault("DefaultProps\\Gate", "Scatter", 0.f) : 0.f;
   m_d.m_gravityfactor = fromMouseClick ? LoadValueFloatWithDefault("DefaultProps\\Gate", "GravityFactor", 0.25f) : 0.25f;
}

void Gate::RenderBlueprint(Sur *psur, const bool solid)
{
}

void Gate::GetTimers(vector<HitTimer*> &pvht)
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

//
// license:GPLv3+
// Ported at: VisualPinball.Engine/VPT/Gate/GateHitGenerator.cs
//

void Gate::GetHitShapes(vector<HitObject*> &pvho)
{
   const float height = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y);
   const float h = m_d.m_height;		// relative height of the gate 

   const float halflength = m_d.m_length * 0.5f;

   const float angleMin = min(m_d.m_angleMin, m_d.m_angleMax); // correct angle inversions
   const float angleMax = max(m_d.m_angleMin, m_d.m_angleMax);

   m_d.m_angleMin = angleMin;
   m_d.m_angleMax = angleMax;

   const float radangle = ANGTORAD(m_d.m_rotation);
   const Vertex2D tangent(cosf(radangle), sinf(radangle));

   const Vertex2D rgv[2] = { // oversize by the ball's radius to prevent the ball from clipping through
      m_d.m_vCenter + (halflength + (float)PHYS_SKIN) * tangent,
      m_d.m_vCenter - (halflength + (float)PHYS_SKIN) * tangent
   };

   if (!m_d.m_twoWay)
   {
      m_plineseg = new LineSeg(rgv[0], rgv[1], height, height + (float)(2.0*PHYS_SKIN)); //!! = ball diameter

      m_plineseg->m_elasticity = m_d.m_elasticity;
      m_plineseg->SetFriction(m_d.m_friction);
      m_plineseg->m_scatter = ANGTORAD(m_d.m_scatter);

      pvho.push_back(m_plineseg);
   }

   m_phitgate = new HitGate(this, height);
   m_phitgate->m_twoWay = m_d.m_twoWay;
   m_phitgate->m_obj = (IFireEvents*)this;
   m_phitgate->m_fe = true;
   m_phitgate->m_enabled = m_d.m_collidable;

   pvho.push_back(m_phitgate);

   if (m_d.m_showBracket)
   {
      HitCircle *phitcircle;
      phitcircle = new HitCircle(m_d.m_vCenter + halflength * tangent, 0.01f, height, height + h);
      pvho.push_back(phitcircle);

      phitcircle = new HitCircle(m_d.m_vCenter - halflength * tangent, 0.01f, height, height + h);
      pvho.push_back(phitcircle);
   }
}

//
// end of license:GPLv3+, back to 'old MAME'-like
//

void Gate::GetHitShapesDebug(vector<HitObject*> &pvho)
{
}

void Gate::EndPlay()
{
   IEditable::EndPlay();

   m_phitgate = NULL;
   m_plineseg = NULL;

   if (m_bracketVertexBuffer)
   {
      m_bracketVertexBuffer->release();
      m_bracketVertexBuffer = NULL;
   }
   if (m_bracketIndexBuffer)
   {
      m_bracketIndexBuffer->release();
      m_bracketIndexBuffer = NULL;
   }
   if (m_wireIndexBuffer)
   {
      m_wireIndexBuffer->release();
      m_wireIndexBuffer = NULL;
   }
   if (m_wireVertexBuffer)
   {
      m_wireVertexBuffer->release();
      m_wireVertexBuffer = NULL;
      m_vertexbuffer_angle = FLT_MAX;
   }
}

void Gate::UpdateWire()
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
   tempMat.SetTranslation(m_d.m_vCenter.x, m_d.m_vCenter.y, m_d.m_height*m_ptable->m_BG_scalez[m_ptable->m_BG_current_set] + m_baseHeight);
   tempMat.Multiply(vertMatrix, vertMatrix);

   Vertex3D_NoTex2 *buf;
   m_wireVertexBuffer->lock(0, 0, (void**)&buf, VertexBuffer::DISCARDCONTENTS);
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
   m_wireVertexBuffer->unlock();
}

void Gate::RenderObject()
{
   UpdateWire();

   RenderDevice * const pd3dDevice = g_pplayer->m_pin3d.m_pd3dPrimaryDevice;

   const Material * const mat = m_ptable->GetMaterial(m_d.m_szMaterial);
   pd3dDevice->basicShader->SetMaterial(mat);

   Pin3D * const ppin3d = &g_pplayer->m_pin3d;
   ppin3d->EnableAlphaBlend(false);

   pd3dDevice->SetRenderState(RenderDevice::DEPTHBIAS, 0);
   pd3dDevice->SetRenderState(RenderDevice::ZWRITEENABLE, RenderDevice::RS_TRUE);
   pd3dDevice->SetRenderState(RenderDevice::CULLMODE, RenderDevice::CULL_CCW);

   pd3dDevice->basicShader->SetTechnique(mat->m_bIsMetal ? "basic_without_texture_isMetal" : "basic_without_texture_isNotMetal");
   pd3dDevice->basicShader->Begin(0);

   // render bracket
   if (m_d.m_showBracket)
      pd3dDevice->DrawIndexedPrimitiveVB(RenderDevice::TRIANGLELIST, MY_D3DFVF_NOTEX2_VERTEX, m_bracketVertexBuffer, 0, gateBracketNumVertices, m_bracketIndexBuffer, 0, gateBracketNumIndices);
   // render wire
   pd3dDevice->DrawIndexedPrimitiveVB(RenderDevice::TRIANGLELIST, MY_D3DFVF_NOTEX2_VERTEX, m_wireVertexBuffer, 0, m_numVertices, m_wireIndexBuffer, 0, m_numIndices);

   pd3dDevice->basicShader->End();
}

void Gate::RenderDynamic()
{
   TRACE_FUNCTION();

   if (!m_phitgate->m_gateMover.m_visible)
      return;

   if (m_ptable->m_reflectionEnabled && !m_d.m_reflectionEnabled)
      return;

   RenderObject();
}

void Gate::ExportMesh(FILE *f)
{
   char name[sizeof(m_wzName)/sizeof(m_wzName[0])];
   WideCharToMultiByteNull(CP_ACP, 0, m_wzName, -1, name, sizeof(name), NULL, NULL);
   m_baseHeight = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y)*m_ptable->m_BG_scalez[m_ptable->m_BG_current_set];

   if (m_d.m_showBracket)
   {
      const string subName = name + string("Bracket");
      WaveFrontObj_WriteObjectName(f, subName);
      Vertex3D_NoTex2* const buf = new Vertex3D_NoTex2[gateBracketNumVertices];
      GenerateBracketMesh(buf);
      WaveFrontObj_WriteVertexInfo(f, buf, gateBracketNumVertices);
      const Material * const mat = m_ptable->GetMaterial(m_d.m_szMaterial);
      WaveFrontObj_WriteMaterial(m_d.m_szMaterial, string(), mat);
      WaveFrontObj_UseTexture(f, m_d.m_szMaterial);
      WaveFrontObj_WriteFaceInfoList(f, gateBracketIndices, gateBracketNumIndices);
      WaveFrontObj_UpdateFaceOffset(gateBracketNumVertices);
      delete[] buf;
   }

   SetGateType(m_d.m_type);

   const string subName = name + string("Wire");
   WaveFrontObj_WriteObjectName(f, subName);
   Vertex3D_NoTex2* const buf = new Vertex3D_NoTex2[m_numVertices];
   GenerateWireMesh(buf);
   WaveFrontObj_WriteVertexInfo(f, buf, m_numVertices);
   const Material * const mat = m_ptable->GetMaterial(m_d.m_szMaterial);
   WaveFrontObj_WriteMaterial(m_d.m_szMaterial, string(), mat);
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
      buf[i].z = vert.z*m_d.m_length*m_ptable->m_BG_scalez[m_ptable->m_BG_current_set] + (m_d.m_height*m_ptable->m_BG_scalez[m_ptable->m_BG_current_set] + m_baseHeight);

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
      buf[i].z = vert.z*m_d.m_length*m_ptable->m_BG_scalez[m_ptable->m_BG_current_set] + (m_d.m_height*m_ptable->m_BG_scalez[m_ptable->m_BG_current_set] + m_baseHeight);

      vert = Vertex3Ds(m_vertices[i].nx, m_vertices[i].ny, m_vertices[i].nz);
      vert = fullMatrix.MultiplyVectorNoTranslate(vert);
      buf[i].nx = vert.x;
      buf[i].ny = vert.y;
      buf[i].nz = vert.z;
      buf[i].tu = m_vertices[i].tu;
      buf[i].tv = m_vertices[i].tv;
   }
}

void Gate::RenderSetup()
{
   RenderDevice * const pd3dDevice = g_pplayer->m_pin3d.m_pd3dPrimaryDevice;

   if (m_bracketIndexBuffer)
      m_bracketIndexBuffer->release();
   m_bracketIndexBuffer = pd3dDevice->CreateAndFillIndexBuffer(gateBracketNumIndices, gateBracketIndices);

   if (m_bracketVertexBuffer)
      m_bracketVertexBuffer->release();
   pd3dDevice->CreateVertexBuffer(gateBracketNumVertices, 0, MY_D3DFVF_NOTEX2_VERTEX, &m_bracketVertexBuffer);

   SetGateType(m_d.m_type);

   m_baseHeight = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y)*m_ptable->m_BG_scalez[m_ptable->m_BG_current_set];

   Vertex3D_NoTex2 *buf;
   m_bracketVertexBuffer->lock(0, 0, (void**)&buf, VertexBuffer::WRITEONLY);
   GenerateBracketMesh(buf);
   m_bracketVertexBuffer->unlock();

   if (m_wireIndexBuffer)
      m_wireIndexBuffer->release();
   m_wireIndexBuffer = pd3dDevice->CreateAndFillIndexBuffer(m_numIndices, m_indices);

   if (m_wireVertexBuffer)
      m_wireVertexBuffer->release();
   pd3dDevice->CreateVertexBuffer(m_numVertices, USAGE_DYNAMIC, MY_D3DFVF_NOTEX2_VERTEX, &m_wireVertexBuffer);

   m_wireVertexBuffer->lock(0, 0, (void**)&buf, VertexBuffer::DISCARDCONTENTS);
   GenerateWireMesh(buf);
   m_wireVertexBuffer->unlock();
}

void Gate::RenderStatic()
{
}

void Gate::SetObjectPos()
{
   m_vpinball->SetObjectPosCur(m_d.m_vCenter.x, m_d.m_vCenter.y);
}

void Gate::MoveOffset(const float dx, const float dy)
{
   m_d.m_vCenter.x += dx;
   m_d.m_vCenter.y += dy;
}

Vertex2D Gate::GetCenter() const
{
   return m_d.m_vCenter;
}

void Gate::PutCenter(const Vertex2D& pv)
{
   m_d.m_vCenter = pv;
}

HRESULT Gate::SaveData(IStream *pstm, HCRYPTHASH hcrypthash, const bool backupForPlay)
{
   BiffWriter bw(pstm, hcrypthash);

   bw.WriteStruct(FID(VCEN), &m_d.m_vCenter, sizeof(Vertex2D));
   bw.WriteFloat(FID(LGTH), m_d.m_length);
   bw.WriteFloat(FID(HGTH), m_d.m_height);
   bw.WriteFloat(FID(ROTA), m_d.m_rotation);
   bw.WriteString(FID(MATR), m_d.m_szMaterial);
   bw.WriteBool(FID(TMON), m_d.m_tdr.m_TimerEnabled);
   bw.WriteBool(FID(GSUP), m_d.m_showBracket);
   bw.WriteBool(FID(GCOL), m_d.m_collidable);
   bw.WriteInt(FID(TMIN), m_d.m_tdr.m_TimerInterval);
   bw.WriteString(FID(SURF), m_d.m_szSurface);
   bw.WriteFloat(FID(ELAS), m_d.m_elasticity);
   bw.WriteFloat(FID(GAMA), m_d.m_angleMax);
   bw.WriteFloat(FID(GAMI), m_d.m_angleMin);
   bw.WriteFloat(FID(GFRC), m_d.m_friction);
   bw.WriteFloat(FID(AFRC), m_d.m_damping);
   bw.WriteFloat(FID(GGFC), m_d.m_gravityfactor);
   bw.WriteBool(FID(GVSB), m_d.m_visible);
   bw.WriteWideString(FID(NAME), m_wzName);
   bw.WriteBool(FID(TWWA), m_d.m_twoWay);
   bw.WriteBool(FID(REEN), m_d.m_reflectionEnabled);
   bw.WriteInt(FID(GATY), m_d.m_type);

   ISelect::SaveData(pstm, hcrypthash);

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

bool Gate::LoadToken(const int id, BiffReader * const pbr)
{
   switch(id)
   {
   case FID(PIID): pbr->GetInt((int *)pbr->m_pdata); break;
   case FID(GATY):
   {
      pbr->GetInt(&m_d.m_type);
      if (m_d.m_type < GateWireW || m_d.m_type > GateLongPlate) // for tables that were saved in the phase where m_type could've been undefined
         m_d.m_type = GateWireW;
      break;
   }
   case FID(VCEN): pbr->GetStruct(&m_d.m_vCenter, sizeof(Vertex2D)); break;
   case FID(LGTH): pbr->GetFloat(&m_d.m_length); break;
   case FID(HGTH): pbr->GetFloat(&m_d.m_height); break;
   case FID(ROTA): pbr->GetFloat(&m_d.m_rotation); break;
   case FID(MATR): pbr->GetString(m_d.m_szMaterial); break;
   case FID(TMON): pbr->GetBool(&m_d.m_tdr.m_TimerEnabled); break;
   case FID(GSUP): pbr->GetBool(&m_d.m_showBracket); break;
   case FID(GCOL): pbr->GetBool(&m_d.m_collidable); break;
   case FID(TWWA): pbr->GetBool(&m_d.m_twoWay); break;
   case FID(GVSB): pbr->GetBool(&m_d.m_visible); break;
   case FID(REEN): pbr->GetBool(&m_d.m_reflectionEnabled); break;
   case FID(TMIN): pbr->GetInt(&m_d.m_tdr.m_TimerInterval); break;
   case FID(SURF): pbr->GetString(m_d.m_szSurface); break;
   case FID(NAME): pbr->GetWideString(m_wzName); break;
   case FID(ELAS): pbr->GetFloat(&m_d.m_elasticity); break;
   case FID(GAMA): pbr->GetFloat(&m_d.m_angleMax); break;
   case FID(GAMI): pbr->GetFloat(&m_d.m_angleMin); break;
   case FID(GFRC): pbr->GetFloat(&m_d.m_friction); break;
   case FID(AFRC): pbr->GetFloat(&m_d.m_damping); break;
   case FID(GGFC): pbr->GetFloat(&m_d.m_gravityfactor); break;
   default: ISelect::LoadToken(id, pbr); break;
   }
   return true;
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
   return S_OK;
}

STDMETHODIMP Gate::put_Length(float newVal)
{
   m_d.m_length = newVal;
   return S_OK;
}

STDMETHODIMP Gate::get_Height(float *pVal)
{
   *pVal = m_d.m_height;
   return S_OK;
}

STDMETHODIMP Gate::put_Height(float newVal)
{
   m_d.m_height = newVal;
   return S_OK;
}

STDMETHODIMP Gate::get_Rotation(float *pVal)
{
   *pVal = m_d.m_rotation;

   return S_OK;
}

STDMETHODIMP Gate::put_Rotation(float newVal)
{
   m_d.m_rotation = newVal;

   return S_OK;
}

STDMETHODIMP Gate::get_X(float *pVal)
{
   *pVal = m_d.m_vCenter.x;

   return S_OK;
}

STDMETHODIMP Gate::put_X(float newVal)
{
   m_d.m_vCenter.x = newVal;

   return S_OK;
}

STDMETHODIMP Gate::get_Y(float *pVal)
{
   *pVal = m_d.m_vCenter.y;

   return S_OK;
}

STDMETHODIMP Gate::put_Y(float newVal)
{
   m_d.m_vCenter.y = newVal;

   return S_OK;
}

STDMETHODIMP Gate::get_Surface(BSTR *pVal)
{
   WCHAR wz[MAXTOKEN];
   MultiByteToWideCharNull(CP_ACP, 0, m_d.m_szSurface, -1, wz, MAXTOKEN);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP Gate::put_Surface(BSTR newVal)
{
   WideCharToMultiByteNull(CP_ACP, 0, newVal, -1, m_d.m_szSurface, MAXTOKEN, NULL, NULL);

   return S_OK;
}

STDMETHODIMP Gate::get_Material(BSTR *pVal)
{
   WCHAR wz[MAXNAMEBUFFER];
   MultiByteToWideCharNull(CP_ACP, 0, m_d.m_szMaterial.c_str(), -1, wz, MAXNAMEBUFFER);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP Gate::put_Material(BSTR newVal)
{
   char buf[MAXNAMEBUFFER];
   WideCharToMultiByteNull(CP_ACP, 0, newVal, -1, buf, MAXNAMEBUFFER, NULL, NULL);
   m_d.m_szMaterial = buf;

   return S_OK;
}

STDMETHODIMP Gate::get_Open(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB((m_phitgate) ? m_phitgate->m_gateMover.m_open : false);

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
         m_phitgate->m_gateMover.m_open = true;

         m_phitgate->m_enabled = false;
         if (!m_d.m_twoWay)
            m_plineseg->m_enabled = false;

         if (m_phitgate->m_gateMover.m_angle < m_phitgate->m_gateMover.m_angleMax)
            m_phitgate->m_gateMover.m_anglespeed = 0.2f;
      }
      else
      {
         m_phitgate->m_gateMover.m_open = false;

         m_phitgate->m_enabled = m_d.m_collidable;
         if (!m_d.m_twoWay)
            m_plineseg->m_enabled = m_d.m_collidable;

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
   m_d.m_elasticity = newVal;

   return S_OK;
}

STDMETHODIMP Gate::get_ShowBracket(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_d.m_showBracket);

   return S_OK;
}

STDMETHODIMP Gate::put_ShowBracket(VARIANT_BOOL newVal)
{
   m_d.m_showBracket = VBTOb(newVal);

   return S_OK;
}

STDMETHODIMP Gate::get_CloseAngle(float *pVal)
{
   *pVal = GetCloseAngle();

   return S_OK;
}

STDMETHODIMP Gate::put_CloseAngle(float newVal)
{
   if (m_d.m_collidable)
   {
      newVal = 0;
      ShowError("Gate is collidable! closing angles other than 0 aren't possible!");
   }

   SetCloseAngle(newVal);
   return S_OK;
}


STDMETHODIMP Gate::get_OpenAngle(float *pVal)
{
   *pVal = GetOpenAngle();

   return S_OK;
}

STDMETHODIMP Gate::put_OpenAngle(float newVal)
{
   if (m_d.m_collidable)
   {
       newVal = (float)(M_PI / 2.0);
       ShowError("Gate is collidable! open angles other than 90 aren't possible!");
   }
   SetOpenAngle(newVal);
   return S_OK;
}


STDMETHODIMP Gate::get_Collidable(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB((g_pplayer) ? m_phitgate->m_enabled : m_d.m_collidable);

   return S_OK;
}


STDMETHODIMP Gate::put_Collidable(VARIANT_BOOL newVal)
{
   if (g_pplayer)
   {
      m_phitgate->m_enabled = VBTOb(newVal);
      if (!m_d.m_twoWay)
         m_plineseg->m_enabled = VBTOb(newVal);
      m_phitgate->m_gateMover.m_angleMax = m_d.m_angleMax;
      m_phitgate->m_gateMover.m_angleMin = m_d.m_angleMin;

      if (m_phitgate->m_enabled) m_phitgate->m_gateMover.m_angleMin = 0;
   }
   else
   {
      m_d.m_collidable = VBTOb(newVal);
      if (m_d.m_collidable) m_d.m_angleMin = 0;
   }

   return S_OK;
}

STDMETHODIMP Gate::Move(int dir, float speed, float angle) //move non-collidable gate, for graphic effects only
{
   if (g_pplayer)
   {
      m_phitgate->m_gateMover.m_forcedMove = true;
      m_phitgate->m_gateMover.m_open = true; // move always turns off natural swing
      m_phitgate->m_enabled = false;         // and collidable off
      if (!m_d.m_twoWay)
         m_plineseg->m_enabled = false;

      if (speed <= 0.0f) speed = 0.2f;       //default gate angle speed
      else speed *= (float)(M_PI / 180.0);   // convert to radians

      if (!dir || angle != 0)                // if no direction or non-zero angle
      {
         angle *= (float)(M_PI / 180.0);     // convert to radians

         if (angle < m_d.m_angleMin) angle = m_d.m_angleMin;
         else if (angle > m_d.m_angleMax) angle = m_d.m_angleMax;

         const float da = angle - m_phitgate->m_gateMover.m_angle; //calc true direction

         if (da > 1.0e-5f) dir = +1;
         else if (da < -1.0e-5f) dir = -1;
         else
         {
            dir = 0;                                  // do nothing
            m_phitgate->m_gateMover.m_anglespeed = 0; //stop 
         }
      }
      else { angle = (dir < 0) ? m_d.m_angleMin : m_d.m_angleMax; } //dir selected and angle not specified			

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
      m_phitgate->m_gateMover.m_friction = newVal;
   else
      m_d.m_friction = newVal;

   return S_OK;
}

STDMETHODIMP Gate::get_Damping(float *pVal)
{
   *pVal = !g_pplayer ? m_d.m_damping : powf(m_phitgate->m_gateMover.m_damping, (float)(1.0/PHYS_FACTOR));

   return S_OK;
}

STDMETHODIMP Gate::put_Damping(float newVal)
{
   const float tmp = clamp(newVal, 0.0f, 1.0f);
   if (g_pplayer)
      m_phitgate->m_gateMover.m_damping = powf(tmp, (float)PHYS_FACTOR); //0.996f;
   else
      m_d.m_damping = tmp;

   return S_OK;
}

STDMETHODIMP Gate::get_GravityFactor(float *pVal)
{
   *pVal = !g_pplayer ? m_d.m_gravityfactor : m_phitgate->m_gateMover.m_gravityfactor;

   return S_OK;
}

STDMETHODIMP Gate::put_GravityFactor(float newVal)
{
   const float tmp = clamp(newVal, 0.0f, 100.0f);

   if (g_pplayer)
      m_phitgate->m_gateMover.m_gravityfactor = tmp;
   else
      m_d.m_gravityfactor = tmp;

   return S_OK;
}

STDMETHODIMP Gate::get_Visible(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB((g_pplayer) ? m_phitgate->m_gateMover.m_visible : m_d.m_visible);

   return S_OK;
}

STDMETHODIMP Gate::put_Visible(VARIANT_BOOL newVal)
{
   if (g_pplayer)
      m_phitgate->m_gateMover.m_visible = VBTOb(newVal);
   else
      m_d.m_visible = VBTOb(newVal);

   return S_OK;
}

STDMETHODIMP Gate::get_TwoWay(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_d.m_twoWay);

   return S_OK;
}

STDMETHODIMP Gate::put_TwoWay(VARIANT_BOOL newVal)
{
   if (g_pplayer)
      m_phitgate->m_twoWay = VBTOb(newVal);
   else
      m_d.m_twoWay = VBTOb(newVal);

   return S_OK;
}

STDMETHODIMP Gate::get_ReflectionEnabled(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_d.m_reflectionEnabled);

   return S_OK;
}

STDMETHODIMP Gate::put_ReflectionEnabled(VARIANT_BOOL newVal)
{
   m_d.m_reflectionEnabled = VBTOb(newVal);

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
   m_d.m_type = newVal;

   return S_OK;
}
