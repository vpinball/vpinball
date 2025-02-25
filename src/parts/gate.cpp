// license:GPLv3+

#include "core/stdafx.h"
#include "utils/objloader.h"
#include "meshes/gateBracketMesh.h"
#include "meshes/gateWireMesh.h"
#include "meshes/gateLongPlateMesh.h"
#include "meshes/gatePlateMesh.h"
#include "meshes/gateWireRectangleMesh.h"
#include "renderer/Shader.h"
#include "renderer/IndexBuffer.h"
#include "renderer/VertexBuffer.h"

Gate::Gate()
{
   m_d.m_type = GateWireW;
}

Gate::~Gate()
{
   assert(m_rd == nullptr);
}

Gate *Gate::CopyForPlay(PinTable *live_table) const
{
   STANDARD_EDITABLE_COPY_FOR_PLAY_IMPL(Gate, live_table)
   return dst;
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

void Gate::UpdateStatusBarInfo()
{
   char tbuf[128];
   sprintf_s(tbuf, sizeof(tbuf), "Length: %.3f | Height: %.3f", m_vpinball->ConvertToUnit(m_d.m_length), m_vpinball->ConvertToUnit(m_d.m_height));
   m_vpinball->SetStatusBarUnitInfo(tbuf, true);
}

HRESULT Gate::Init(PinTable *const ptable, const float x, const float y, const bool fromMouseClick, const bool forPlay)
{
   m_ptable = ptable;
   SetDefaults(fromMouseClick);
   m_d.m_vCenter.x = x;
   m_d.m_vCenter.y = y;
   return forPlay ? S_OK : InitVBA(fTrue, 0, nullptr);
}

void Gate::SetDefaults(const bool fromMouseClick)
{
#define regKey Settings::DefaultPropsGate

   m_d.m_length = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "Length"s, 100.f) : 100.f;
   m_d.m_height = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "Height"s, 50.f) : 50.f;
   m_d.m_rotation = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "Rotation"s, -90.f) : -90.f;
   m_d.m_showBracket = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "ShowBracket"s, true) : true;
   m_d.m_type = fromMouseClick ? (GateType)g_pvp->m_settings.LoadValueWithDefault(regKey, "GateType"s, (int)GateWireW) : GateWireW;
   m_d.m_collidable = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "Collidable"s, true) : true;
   m_d.m_angleMin = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "AngleMin"s, 0.f) : 0.f;
   m_d.m_angleMax = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "AngleMax"s, (float)(M_PI / 2.0)) : (float)(M_PI / 2.0);
   m_d.m_visible = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "Visible"s, true) : true;
   m_d.m_tdr.m_TimerEnabled = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "TimerEnabled"s, false) : false;
   m_d.m_tdr.m_TimerInterval = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "TimerInterval"s, 100) : 100;

   const bool hr = g_pvp->m_settings.LoadValue(regKey, "Surface"s, m_d.m_szSurface);
   if (!hr || !fromMouseClick)
      m_d.m_szSurface.clear();

   SetDefaultPhysics(fromMouseClick);

   m_d.m_twoWay = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "TwoWay"s, true) : true;
   m_d.m_reflectionEnabled = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "ReflectionEnabled"s, true) : true;

#undef regKey
}


void Gate::WriteRegDefaults()
{
#define regKey Settings::DefaultPropsGate

   g_pvp->m_settings.SaveValue(regKey, "Length"s, m_d.m_length);
   g_pvp->m_settings.SaveValue(regKey, "Height"s, m_d.m_height);
   g_pvp->m_settings.SaveValue(regKey, "Rotation"s, m_d.m_rotation);
   g_pvp->m_settings.SaveValue(regKey, "ShowBracket"s, m_d.m_showBracket);
   g_pvp->m_settings.SaveValue(regKey, "Collidable"s, m_d.m_collidable);
   g_pvp->m_settings.SaveValue(regKey, "AngleMin"s, m_d.m_angleMin);
   g_pvp->m_settings.SaveValue(regKey, "AngleMax"s, m_d.m_angleMax);
   g_pvp->m_settings.SaveValue(regKey, "Visible"s, m_d.m_visible);
   g_pvp->m_settings.SaveValue(regKey, "TimerEnabled"s, m_d.m_tdr.m_TimerEnabled);
   g_pvp->m_settings.SaveValue(regKey, "TimerInterval"s, m_d.m_tdr.m_TimerInterval);
   g_pvp->m_settings.SaveValue(regKey, "Surface"s, m_d.m_szSurface);
   g_pvp->m_settings.SaveValue(regKey, "Elasticity"s, m_d.m_elasticity);
   g_pvp->m_settings.SaveValue(regKey, "Friction"s, m_d.m_friction);
   g_pvp->m_settings.SaveValue(regKey, "Scatter"s, m_d.m_scatter);
   g_pvp->m_settings.SaveValue(regKey, "GravityFactor"s, m_d.m_gravityfactor);
   g_pvp->m_settings.SaveValue(regKey, "TwoWay"s, m_d.m_twoWay);
   g_pvp->m_settings.SaveValue(regKey, "ReflectionEnabled"s, m_d.m_reflectionEnabled);
   g_pvp->m_settings.SaveValue(regKey, "GateType"s, m_d.m_type);

#undef regKey
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

void Gate::SetDefaultPhysics(const bool fromMouseClick)
{
#define regKey Settings::DefaultPropsGate

   m_d.m_elasticity = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "Elasticity"s, 0.3f) : 0.3f;
   m_d.m_friction = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "Friction"s, 0.02f) : 0.02f;
   m_d.m_damping = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "AntiFriction"s, 0.985f) : 0.985f;
   m_d.m_scatter = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "Scatter"s, 0.f) : 0.f;
   m_d.m_gravityfactor = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "GravityFactor"s, 0.25f) : 0.25f;

#undef regKey
}

void Gate::RenderBlueprint(Sur *psur, const bool solid)
{
}

void Gate::BeginPlay(vector<HitTimer*> &pvht) { IEditable::BeginPlay(pvht, &m_d.m_tdr, this); }

void Gate::EndPlay() { IEditable::EndPlay(); }


#pragma region Physics

// Ported at: VisualPinball.Engine/VPT/Gate/GateHitGenerator.cs

void Gate::PhysicSetup(PhysicsEngine* physics, const bool isUI)
{
   if (isUI)
   {
      // FIXME implement UI picking
   }
   else
   {
      const float height = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y);
      const float h = m_d.m_height; // relative height of the gate

      const float halflength = m_d.m_length * 0.5f;

      const float angleMin = min(m_d.m_angleMin, m_d.m_angleMax); // correct angle inversions
      const float angleMax = max(m_d.m_angleMin, m_d.m_angleMax);

      m_d.m_angleMin = angleMin;
      m_d.m_angleMax = angleMax;

      const float radangle = ANGTORAD(m_d.m_rotation);
      const Vertex2D tangent(cosf(radangle), sinf(radangle));

      const Vertex2D rgv[2] = { // oversize by the ball's radius to prevent the ball from clipping through
         m_d.m_vCenter + (halflength + (float)PHYS_SKIN) * tangent, m_d.m_vCenter - (halflength + (float)PHYS_SKIN) * tangent
      };

      if (!m_d.m_twoWay)
      {
          m_plineseg = new LineSeg(rgv[0], rgv[1], height, height + (float)(2.0 * PHYS_SKIN)); //!! = ball diameter
          m_plineseg->m_elasticity = m_d.m_elasticity;
          m_plineseg->SetFriction(m_d.m_friction);
          m_plineseg->m_scatter = ANGTORAD(m_d.m_scatter);
          physics->AddCollider(m_plineseg, this, isUI);
      }

      m_phitgate = new HitGate(this, height);
      m_phitgate->m_twoWay = m_d.m_twoWay;
      m_phitgate->m_obj = (IFireEvents *)this;
      m_phitgate->m_fe = true;
      m_phitgate->m_enabled = m_d.m_collidable;
      physics->AddCollider(m_phitgate, this, isUI);

      if (m_d.m_showBracket)
      {
          HitCircle *phitcircle;
          phitcircle = new HitCircle(m_d.m_vCenter + halflength * tangent, 0.01f, height, height + h);
          physics->AddCollider(phitcircle, this, isUI);

          phitcircle = new HitCircle(m_d.m_vCenter - halflength * tangent, 0.01f, height, height + h);
          physics->AddCollider(phitcircle, this, isUI);
      }
   }
}

void Gate::PhysicRelease(PhysicsEngine* physics, const bool isUI)
{
   if (!isUI)
   {
      m_phitgate = nullptr;
      m_plineseg = nullptr;
   }
}

#pragma endregion


#pragma region Rendering

void Gate::RenderSetup(RenderDevice *device)
{
   assert(m_rd == nullptr);
   m_rd = device;

   SetGateType(m_d.m_type);
   m_baseHeight = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y);

   IndexBuffer *bracketIndexBuffer = new IndexBuffer(m_rd, gateBracketNumIndices, gateBracketIndices);
   VertexBuffer *bracketVertexBuffer = new VertexBuffer(m_rd, gateBracketNumVertices);
   Vertex3D_NoTex2 *buf;
   bracketVertexBuffer->Lock(buf);
   GenerateBracketMesh(buf);
   bracketVertexBuffer->Unlock();
   m_bracketMeshBuffer = new MeshBuffer(m_wzName + L".Bracket"s, bracketVertexBuffer, bracketIndexBuffer, true);

   IndexBuffer *wireIndexBuffer = new IndexBuffer(m_rd, m_numIndices, m_indices);
   VertexBuffer *wireVertexBuffer = new VertexBuffer(m_rd, m_numVertices, nullptr, true);
   wireVertexBuffer->Lock(buf);
   GenerateWireMesh(buf);
   wireVertexBuffer->Unlock();
   m_wireMeshBuffer = new MeshBuffer(m_wzName + L".Wire"s, wireVertexBuffer, wireIndexBuffer, true);
}

void Gate::RenderRelease()
{
   assert(m_rd != nullptr);
   delete m_wireMeshBuffer;
   m_wireMeshBuffer = nullptr;
   delete m_bracketMeshBuffer;
   m_bracketMeshBuffer = nullptr;
   m_vertexbuffer_angle = FLT_MAX;
   m_rd = nullptr;
}

void Gate::UpdateAnimation(const float diff_time_msec)
{
   assert(m_rd != nullptr);
   // Animation is updated by physics engine through a MoverObject. No additional visual animation here
   // Still monitor angle updates in order to fire animate event at most once per frame (physics engine perform far more cycle per frame)
   if (m_phitgate && m_lastAngle != m_phitgate->m_gateMover.m_angle)
   {
      m_lastAngle = m_phitgate->m_gateMover.m_angle;
      FireGroupEvent(DISPID_AnimateEvents_Animate);
   }
}

void Gate::Render(const unsigned int renderMask)
{
   assert(m_rd != nullptr);
   const bool isStaticOnly = renderMask & Renderer::STATIC_ONLY;
   const bool isDynamicOnly = renderMask & Renderer::DYNAMIC_ONLY;
   const bool isReflectionPass = renderMask & Renderer::REFLECTION_PASS;
   TRACE_FUNCTION();

   if (isStaticOnly 
   || !m_phitgate->m_gateMover.m_visible 
   || (isReflectionPass && !m_d.m_reflectionEnabled))
      return;

   if (m_phitgate->m_gateMover.m_angle != m_vertexbuffer_angle)
   {
      m_vertexbuffer_angle = m_phitgate->m_gateMover.m_angle;

      const Matrix3D fullMatrix = Matrix3D::MatrixRotateX(m_d.m_twoWay ? m_phitgate->m_gateMover.m_angle : -m_phitgate->m_gateMover.m_angle)
                                * Matrix3D::MatrixRotateZ(ANGTORAD(m_d.m_rotation));
      const Matrix3D vertMatrix = (fullMatrix
                                 * Matrix3D::MatrixScale(m_d.m_length, m_d.m_length, m_d.m_length))
                                 * Matrix3D::MatrixTranslate(m_d.m_vCenter.x, m_d.m_vCenter.y, m_d.m_height + m_baseHeight);

      Vertex3D_NoTex2 *buf;
      m_wireMeshBuffer->m_vb->Lock(buf);
      vertMatrix.TransformPositions(m_vertices, buf, m_numVertices);
      fullMatrix.TransformNormals(m_vertices, buf, m_numVertices);
      m_wireMeshBuffer->m_vb->Unlock();
   }

   m_rd->ResetRenderState();
   m_rd->m_basicShader->SetBasic(m_ptable->GetMaterial(m_d.m_szMaterial), nullptr);
   Vertex3Ds pos(m_d.m_vCenter.x, m_d.m_vCenter.y, m_baseHeight);
   if (m_d.m_showBracket)
      m_rd->DrawMesh(m_rd->m_basicShader, false, pos, 0.f, m_bracketMeshBuffer, RenderDevice::TRIANGLELIST, 0, gateBracketNumIndices);
   m_rd->DrawMesh(m_rd->m_basicShader, false, pos, 0.f, m_wireMeshBuffer, RenderDevice::TRIANGLELIST, 0, m_numIndices);
}

#pragma endregion

void Gate::ExportMesh(ObjLoader& loader)
{
   char name[sizeof(m_wzName)/sizeof(m_wzName[0])];
   WideCharToMultiByteNull(CP_ACP, 0, m_wzName, -1, name, sizeof(name), nullptr, nullptr);
   m_baseHeight = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y);

   if (m_d.m_showBracket)
   {
      const string subName = name + "Bracket"s;
      loader.WriteObjectName(subName);
      Vertex3D_NoTex2* const buf = new Vertex3D_NoTex2[gateBracketNumVertices];
      GenerateBracketMesh(buf);
      loader.WriteVertexInfo(buf, gateBracketNumVertices);
      const Material * const mat = m_ptable->GetMaterial(m_d.m_szMaterial);
      loader.WriteMaterial(m_d.m_szMaterial, string(), mat);
      loader.UseTexture(m_d.m_szMaterial);
      loader.WriteFaceInfoList(gateBracketIndices, gateBracketNumIndices);
      loader.UpdateFaceOffset(gateBracketNumVertices);
      delete[] buf;
   }

   SetGateType(m_d.m_type);

   const string subName = name + "Wire"s;
   loader.WriteObjectName(subName);
   Vertex3D_NoTex2* const buf = new Vertex3D_NoTex2[m_numVertices];
   GenerateWireMesh(buf);
   loader.WriteVertexInfo(buf, m_numVertices);
   const Material * const mat = m_ptable->GetMaterial(m_d.m_szMaterial);
   loader.WriteMaterial(m_d.m_szMaterial, string(), mat);
   loader.UseTexture(m_d.m_szMaterial);
   loader.WriteFaceInfoList(m_indices, m_numIndices);
   loader.UpdateFaceOffset(m_numVertices);
   delete[] buf;
}

void Gate::GenerateBracketMesh(Vertex3D_NoTex2 *buf)
{
   Matrix3D rotMatrix = Matrix3D::MatrixRotateZ(ANGTORAD(m_d.m_rotation));
   Matrix3D vertMatrix = rotMatrix * Matrix3D::MatrixScale(m_d.m_length) * Matrix3D::MatrixTranslate(m_d.m_vCenter.x, m_d.m_vCenter.y, m_d.m_height + m_baseHeight);
   vertMatrix.TransformPositions(gateBracket, buf, gateBracketNumVertices);
   rotMatrix.TransformNormals(gateBracket, buf, gateBracketNumVertices);
}

void Gate::GenerateWireMesh(Vertex3D_NoTex2 *buf)
{
   const Matrix3D world = Matrix3D::MatrixRotateZ(ANGTORAD(m_d.m_rotation)) * Matrix3D::MatrixTranslate(m_d.m_vCenter.x, m_d.m_vCenter.y, m_d.m_height + m_baseHeight);
   world.TransformVertices(m_vertices, buf, m_numVertices);
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

HRESULT Gate::SaveData(IStream *pstm, HCRYPTHASH hcrypthash, const bool saveForUndo)
{
   BiffWriter bw(pstm, hcrypthash);

   bw.WriteVector2(FID(VCEN), m_d.m_vCenter);
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
   m_d.m_twoWay = false; // to keep old VP8/9 behavior when loading .vpt files

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
   case FID(VCEN): pbr->GetVector2(m_d.m_vCenter); break;
   case FID(LGTH): pbr->GetFloat(m_d.m_length); break;
   case FID(HGTH): pbr->GetFloat(m_d.m_height); break;
   case FID(ROTA): pbr->GetFloat(m_d.m_rotation); break;
   case FID(MATR): pbr->GetString(m_d.m_szMaterial); break;
   case FID(TMON): pbr->GetBool(m_d.m_tdr.m_TimerEnabled); break;
   case FID(GSUP): pbr->GetBool(m_d.m_showBracket); break;
   case FID(GCOL): pbr->GetBool(m_d.m_collidable); break;
   case FID(TWWA): pbr->GetBool(m_d.m_twoWay); break;
   case FID(GVSB): pbr->GetBool(m_d.m_visible); break;
   case FID(REEN): pbr->GetBool(m_d.m_reflectionEnabled); break;
   case FID(TMIN): pbr->GetInt(m_d.m_tdr.m_TimerInterval); break;
   case FID(SURF): pbr->GetString(m_d.m_szSurface); break;
   case FID(NAME): pbr->GetWideString(m_wzName, std::size(m_wzName)); break;
   case FID(ELAS): pbr->GetFloat(m_d.m_elasticity); break;
   case FID(GAMA): pbr->GetFloat(m_d.m_angleMax); break;
   case FID(GAMI): pbr->GetFloat(m_d.m_angleMin); break;
   case FID(GFRC): pbr->GetFloat(m_d.m_friction); break;
   case FID(AFRC): pbr->GetFloat(m_d.m_damping); break;
   case FID(GGFC): pbr->GetFloat(m_d.m_gravityfactor); break;
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

   for (size_t i = 0; i < std::size(arr); i++)
      if (InlineIsEqualGUID(*arr[i], riid))
         return S_OK;

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
   MultiByteToWideCharNull(CP_ACP, 0, m_d.m_szSurface.c_str(), -1, wz, MAXTOKEN);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP Gate::put_Surface(BSTR newVal)
{
   char buf[MAXTOKEN];
   WideCharToMultiByteNull(CP_ACP, 0, newVal, -1, buf, MAXTOKEN, nullptr, nullptr);
   m_d.m_szSurface = buf;

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
   WideCharToMultiByteNull(CP_ACP, 0, newVal, -1, buf, MAXNAMEBUFFER, nullptr, nullptr);
   m_d.m_szMaterial = buf;

   return S_OK;
}

STDMETHODIMP Gate::get_Open(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_phitgate ? m_phitgate->m_gateMover.m_open : false);
   return S_OK;
}

STDMETHODIMP Gate::put_Open(VARIANT_BOOL newVal)
{
   if (m_phitgate)
   {
      m_phitgate->m_gateMover.m_hitDirection = false;

      m_phitgate->m_gateMover.m_angleMax = m_d.m_angleMax;
      m_phitgate->m_gateMover.m_angleMin = m_d.m_angleMin;
      m_phitgate->m_gateMover.m_forcedMove = true;

      m_phitgate->m_gateMover.m_open = VBTOb(newVal);

      if (m_phitgate->m_gateMover.m_open)
      {
         m_phitgate->m_enabled = false;
         if (!m_d.m_twoWay)
            m_plineseg->m_enabled = false;

         if (m_phitgate->m_gateMover.m_angle < m_phitgate->m_gateMover.m_angleMax)
            m_phitgate->m_gateMover.m_anglespeed = 0.2f;
      }
      else
      {
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
      newVal = 0.f;
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
      m_phitgate->m_gateMover.m_hitDirection = false;

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
