// license:GPLv3+

#include "core/stdafx.h"
#include "utils/objloader.h"
#include "meshes/spinnerBracketMesh.h"
#include "meshes/spinnerPlateMesh.h"
#include "renderer/Shader.h"
#include "renderer/IndexBuffer.h"
#include "renderer/VertexBuffer.h"

Spinner::Spinner()
{
   m_phitspinner = nullptr;
   m_vertexBuffer_spinneranimangle = -FLT_MAX;
}

Spinner::~Spinner()
{
   assert(m_rd == nullptr);
}

Spinner *Spinner::CopyForPlay(PinTable *live_table) const
{
   STANDARD_EDITABLE_COPY_FOR_PLAY_IMPL(Spinner, live_table)
   return dst;
}

void Spinner::UpdateStatusBarInfo()
{
   char tbuf[128];
   sprintf_s(tbuf, sizeof(tbuf), "Length: %.3f | Height: %.3f", m_vpinball->ConvertToUnit(m_d.m_length), m_vpinball->ConvertToUnit(m_d.m_height));
   m_vpinball->SetStatusBarUnitInfo(tbuf, true);
}

float Spinner::GetAngleMax() const
{
   return (g_pplayer) ? RADTOANG(m_phitspinner->m_spinnerMover.m_angleMax) : // player active value
                        m_d.m_angleMax;
}

void Spinner::SetAngleMax(const float angle)
{
    float newVal = angle;

    if (g_pplayer)
    {
        if (m_d.m_angleMin != m_d.m_angleMax)	// allow only if in limited angle mode
        {
            if (newVal > m_d.m_angleMax) newVal = m_d.m_angleMax;
            else if (newVal < m_d.m_angleMin) newVal = m_d.m_angleMin;

            newVal = ANGTORAD(newVal);

            if (m_phitspinner->m_spinnerMover.m_angleMin < newVal)  // Min is smaller???
                m_phitspinner->m_spinnerMover.m_angleMax = newVal;  // yes set new max
            else m_phitspinner->m_spinnerMover.m_angleMin = newVal; // no set new minumum
        }
    }
    else
        m_d.m_angleMax = newVal;
}

float Spinner::GetAngleMin() const
{
    return (g_pplayer) ? RADTOANG(m_phitspinner->m_spinnerMover.m_angleMin) : // player active value
                        m_d.m_angleMin;
}

void Spinner::SetAngleMin(const float angle)
{
    float newVal = angle;
    if (g_pplayer)
    {
        if (m_d.m_angleMin != m_d.m_angleMax)	// allow only if in limited angle mode
        {
            if (newVal > m_d.m_angleMax) newVal = m_d.m_angleMax;
            else if (newVal < m_d.m_angleMin) newVal = m_d.m_angleMin;

            newVal = ANGTORAD(newVal);

            if (m_phitspinner->m_spinnerMover.m_angleMax > newVal)  // max is bigger
                m_phitspinner->m_spinnerMover.m_angleMin = newVal;  // then set new minumum
            else m_phitspinner->m_spinnerMover.m_angleMax = newVal; // else set new max
        }
    }
    else
        m_d.m_angleMin = newVal;
}

HRESULT Spinner::Init(PinTable *const ptable, const float x, const float y, const bool fromMouseClick, const bool forPlay)
{
   m_ptable = ptable;
   SetDefaults(fromMouseClick);
   m_d.m_vCenter.x = x;
   m_d.m_vCenter.y = y;
   return forPlay ? S_OK : InitVBA(fTrue, 0, nullptr);
}


void Spinner::WriteRegDefaults()
{
#define regKey Settings::DefaultPropsSpinner

   g_pvp->m_settings.SaveValue(regKey, "Length"s, m_d.m_length);
   g_pvp->m_settings.SaveValue(regKey, "Rotation"s, m_d.m_rotation);
   g_pvp->m_settings.SaveValue(regKey, "ShowBracket"s, m_d.m_showBracket);
   g_pvp->m_settings.SaveValue(regKey, "Height"s, m_d.m_height);
   g_pvp->m_settings.SaveValue(regKey, "AngleMax"s, m_d.m_angleMax);
   g_pvp->m_settings.SaveValue(regKey, "AngleMin"s, m_d.m_angleMin);
   g_pvp->m_settings.SaveValue(regKey, "Elasticity"s, m_d.m_elasticity);
   g_pvp->m_settings.SaveValue(regKey, "AntiFriction"s, m_d.m_damping);
   g_pvp->m_settings.SaveValue(regKey, "Scatter"s, m_d.m_scatter);
   g_pvp->m_settings.SaveValue(regKey, "Visible"s, m_d.m_visible);
   g_pvp->m_settings.SaveValue(regKey, "TimerEnabled"s, m_d.m_tdr.m_TimerEnabled);
   g_pvp->m_settings.SaveValue(regKey, "TimerInterval"s, m_d.m_tdr.m_TimerInterval);
   g_pvp->m_settings.SaveValue(regKey, "Image"s, m_d.m_szImage);
   g_pvp->m_settings.SaveValue(regKey, "Surface"s, m_d.m_szSurface);
   g_pvp->m_settings.SaveValue(regKey, "ReflectionEnabled"s, m_d.m_reflectionEnabled);

#undef regKey
}

void Spinner::SetDefaults(const bool fromMouseClick)
{
#define regKey Settings::DefaultPropsSpinner

   m_d.m_length = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "Length"s, 80.f) : 80.f;
   m_d.m_rotation = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "Rotation"s, 0.f) : 0.f;
   m_d.m_showBracket = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "ShowBracket"s, true) : true;
   m_d.m_height = (float)(fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "Height"s, 60000) : 60000) / 1000.0f;

   SetDefaultPhysics(fromMouseClick);

   m_d.m_angleMax = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "AngleMax"s, 0.f) : 0.f;
   m_d.m_angleMin = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "AngleMin"s, 0.f) : 0.f;
   m_d.m_visible = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "Visible"s, true) : true;
   m_d.m_reflectionEnabled = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "ReflectionEnabled"s, true) : true;
   m_d.m_tdr.m_TimerEnabled = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "TimerEnabled"s, false) : false;
   m_d.m_tdr.m_TimerInterval = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "TimerInterval"s, 100) : 100;

   bool hr = g_pvp->m_settings.LoadValue(regKey, "Image"s, m_d.m_szImage);
   if (!hr || !fromMouseClick)
      m_d.m_szImage.clear();

   hr = g_pvp->m_settings.LoadValue(regKey, "Surface"s, m_d.m_szSurface);
   if (!hr || !fromMouseClick)
      m_d.m_szSurface.clear();

#undef regKey
}

void Spinner::UIRenderPass1(Sur * const psur)
{
}

void Spinner::UIRenderPass2(Sur * const psur)
{
   psur->SetBorderColor(RGB(0, 0, 0), false, 0);
   psur->SetLineColor(RGB(0, 0, 0), false, 3);
   psur->SetObject(this);

   const float halflength = m_d.m_length * 0.5f;

   const float radangle = ANGTORAD(m_d.m_rotation);
   float sn = sinf(radangle);
   float cs = cosf(radangle);

   psur->Line(m_d.m_vCenter.x + cs*halflength, m_d.m_vCenter.y + sn*halflength,
      m_d.m_vCenter.x - cs*halflength, m_d.m_vCenter.y - sn*halflength);

   psur->SetLineColor(RGB(0, 0, 0), false, 1);
   psur->SetObject(this);

   psur->Line(m_d.m_vCenter.x + cs*halflength, m_d.m_vCenter.y + sn*halflength,
      m_d.m_vCenter.x - cs*halflength, m_d.m_vCenter.y - sn*halflength);

   if (sn == 0.0f) sn = 1.0f;
   if (cs == 0.0f) cs = 1.0f;
   psur->Rectangle(m_d.m_vCenter.x - cs * halflength * 0.65f, m_d.m_vCenter.y - sn * halflength * 0.65f,
                   m_d.m_vCenter.x + cs * halflength * 0.65f, m_d.m_vCenter.y + sn * halflength * 0.65f);
}

void Spinner::BeginPlay(vector<HitTimer*> &pvht) { IEditable::BeginPlay(pvht, &m_d.m_tdr, this); }

void Spinner::EndPlay() { IEditable::EndPlay(); }

#pragma region Physics

// Ported at: VisualPinball.Engine/VPT/Spinner/SpinnerHitGenerator.cs

void Spinner::PhysicSetup(PhysicsEngine* physics, const bool isUI)
{
   if (isUI)
   {
      // FIXME implement UI picking
   }
   else
   {
      const float height = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y);
      const float h = m_d.m_height + 30.0f;

      const float angleMin = min(m_d.m_angleMin, m_d.m_angleMax); // correct angle inversions
      const float angleMax = max(m_d.m_angleMin, m_d.m_angleMax);

      m_d.m_angleMin = angleMin;
      m_d.m_angleMax = angleMax;

      HitSpinner *const phitspinner = new HitSpinner(this, height);
      m_phitspinner = phitspinner;

      physics->AddCollider(phitspinner, this, isUI);

      if (m_d.m_showBracket)
      {
            /*add a hit shape for the bracket if shown, just in case if the bracket spinner height is low enough so the ball can hit it*/
            const float halflength = m_d.m_length * 0.5f + (m_d.m_length * 0.1875f);
            const float radangle = ANGTORAD(m_d.m_rotation);
            const float sn = sinf(radangle);
            const float cs = cosf(radangle);

            HitCircle *phitcircle;
            phitcircle = new HitCircle(Vertex2D(m_d.m_vCenter.x + cs * halflength, m_d.m_vCenter.y + sn * halflength), m_d.m_length * 0.075f, height + m_d.m_height, height + h);
            physics->AddCollider(phitcircle, this, isUI);

            phitcircle = new HitCircle(Vertex2D(m_d.m_vCenter.x - cs * halflength, m_d.m_vCenter.y - sn * halflength), m_d.m_length * 0.075f, height + m_d.m_height, height + h);
            physics->AddCollider(phitcircle, this, isUI);
      }
   }
}

void Spinner::PhysicRelease(PhysicsEngine* physics, const bool isUI)
{
   if (!isUI)
   {
      m_phitspinner = nullptr;
   }
}

#pragma endregion


void Spinner::ExportMesh(ObjLoader& loader)
{
   char name[sizeof(m_wzName)/sizeof(m_wzName[0])];
   WideCharToMultiByteNull(CP_ACP, 0, m_wzName, -1, name, sizeof(name), nullptr, nullptr);
   vector<Vertex3D_NoTex2> transformedVertices;
   vector<HitObject*> dummyHitObj;

   const float height = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y);
   m_posZ = height + m_d.m_height;

   // FIXME is this really needed
   // PhysicSetup(dummyHitObj, false);

   if (m_d.m_showBracket)
   {
      const string subObjName = name + "Bracket"s;
      loader.WriteObjectName(subObjName);

      m_fullMatrix = Matrix3D::MatrixRotateZ(ANGTORAD(m_d.m_rotation));

      transformedVertices.resize(spinnerBracketNumVertices);

      for (unsigned int i = 0; i < spinnerBracketNumVertices; i++)
      {
         Vertex3Ds vert = m_fullMatrix * Vertex3Ds{spinnerBracket[i].x, spinnerBracket[i].y, spinnerBracket[i].z};
         transformedVertices[i].x = vert.x*m_d.m_length + m_d.m_vCenter.x;
         transformedVertices[i].y = vert.y*m_d.m_length + m_d.m_vCenter.y;
         transformedVertices[i].z = vert.z*m_d.m_length + m_posZ;

         vert = Vertex3Ds(spinnerBracket[i].nx, spinnerBracket[i].ny, spinnerBracket[i].nz);
         vert = m_fullMatrix.MultiplyVectorNoTranslate(vert);
         transformedVertices[i].nx = vert.x;
         transformedVertices[i].ny = vert.y;
         transformedVertices[i].nz = vert.z;

         transformedVertices[i].tu = spinnerBracket[i].tu;
         transformedVertices[i].tv = spinnerBracket[i].tv;
      }
      loader.WriteVertexInfo(transformedVertices.data(), spinnerBracketNumVertices);

      const Material * const mat = m_ptable->GetMaterial(m_d.m_szMaterial);
      loader.WriteMaterial(m_d.m_szMaterial, string(), mat);
      loader.UseTexture(m_d.m_szMaterial);
      loader.WriteFaceInfoList(spinnerBracketIndices, spinnerBracketNumFaces);
      loader.UpdateFaceOffset(spinnerBracketNumVertices);

      transformedVertices.clear();
   }

   transformedVertices.resize(spinnerPlateNumVertices);
   m_vertexBuffer_spinneranimangle = -FLT_MAX;
   UpdatePlate(transformedVertices.data());

   const string subObjName = name + "Plate"s;
   loader.WriteObjectName(subObjName);
   loader.WriteVertexInfo(transformedVertices.data(), spinnerPlateNumVertices);
   loader.WriteFaceInfoList(spinnerPlateIndices, spinnerPlateNumFaces);
   loader.UpdateFaceOffset(spinnerPlateNumVertices);
   transformedVertices.clear();
}


#pragma region Rendering

void Spinner::RenderSetup(RenderDevice *device)
{
   assert(m_rd == nullptr);
   m_rd = device;

   const float height = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y);
   m_posZ = height + m_d.m_height;

   IndexBuffer *bracketIndexBuffer = new IndexBuffer(m_rd, spinnerBracketNumFaces, spinnerBracketIndices);
   VertexBuffer *bracketVertexBuffer = new VertexBuffer(m_rd, spinnerBracketNumVertices);
   m_bracketMeshBuffer = new MeshBuffer(m_wzName + L".Bracket"s, bracketVertexBuffer, bracketIndexBuffer, true);

   m_fullMatrix = Matrix3D::MatrixRotateZ(ANGTORAD(m_d.m_rotation));

   Vertex3D_NoTex2 *buf;
   bracketVertexBuffer->Lock(buf);
   for (unsigned int i = 0; i < spinnerBracketNumVertices; i++)
   {
      Vertex3Ds vert = m_fullMatrix * Vertex3Ds{spinnerBracket[i].x, spinnerBracket[i].y, spinnerBracket[i].z};
      buf[i].x = vert.x*m_d.m_length + m_d.m_vCenter.x;
      buf[i].y = vert.y*m_d.m_length + m_d.m_vCenter.y;
      buf[i].z = vert.z*m_d.m_length + m_posZ;

      vert = m_fullMatrix.MultiplyVectorNoTranslate(Vertex3Ds{spinnerBracket[i].nx, spinnerBracket[i].ny, spinnerBracket[i].nz});
      buf[i].nx = vert.x;
      buf[i].ny = vert.y;
      buf[i].nz = vert.z;

      buf[i].tu = spinnerBracket[i].tu;
      buf[i].tv = spinnerBracket[i].tv;
   }
   bracketVertexBuffer->Unlock();

   IndexBuffer* plateIndexBuffer = new IndexBuffer(m_rd, spinnerPlateNumFaces, spinnerPlateIndices);
   VertexBuffer* plateVertexBuffer = new VertexBuffer(m_rd, spinnerPlateNumVertices, nullptr, true);
   m_plateMeshBuffer = new MeshBuffer(m_wzName + L".Plate"s, plateVertexBuffer, plateIndexBuffer, true);

   m_vertexBuffer_spinneranimangle = -FLT_MAX;
   UpdatePlate(nullptr);
}

void Spinner::RenderRelease()
{
   assert(m_rd != nullptr);
   m_rd = nullptr;
   delete m_bracketMeshBuffer;
   delete m_plateMeshBuffer;
   m_bracketMeshBuffer = nullptr;
   m_plateMeshBuffer = nullptr;
}

void Spinner::UpdateAnimation(const float diff_time_msec)
{
   // Animation is updated by physics engine through a MoverObject. No additional visual animation here
   // Still monitor angle updates in order to fire animate event at most once per frame (physics engine perform far more cycle per frame)
   if (m_phitspinner && m_lastAngle != m_phitspinner->m_spinnerMover.m_angle)
   {
      m_lastAngle = m_phitspinner->m_spinnerMover.m_angle;
      FireGroupEvent(DISPID_AnimateEvents_Animate);
   }
}

void Spinner::Render(const unsigned int renderMask)
{
   assert(m_rd != nullptr);
   const bool isStaticOnly = renderMask & Renderer::STATIC_ONLY;
   const bool isDynamicOnly = renderMask & Renderer::DYNAMIC_ONLY;
   const bool isReflectionPass = renderMask & Renderer::REFLECTION_PASS;
   TRACE_FUNCTION();

   if (!m_d.m_visible
   || (isReflectionPass && !m_d.m_reflectionEnabled))
      return;

   m_rd->ResetRenderState();

   if (m_d.m_showBracket && !isDynamicOnly)
   {
      Material mat;
      mat.m_type = Material::MaterialType::METAL;
      mat.m_cBase = 0x20202020;
      mat.m_fWrapLighting = 0.9f;
      mat.m_cGlossy = 0x60606060;
      mat.m_fRoughness = 0.4f;
      mat.m_fGlossyImageLerp = 1.0f;
      mat.m_fThickness = 0.05f;
      mat.m_cClearcoat = 0x20202020;
      mat.m_fEdge = 1.0f;
      mat.m_fEdgeAlpha = 1.0f;
      m_rd->m_basicShader->SetBasic(&mat, nullptr);
      Vertex3Ds pos(m_d.m_vCenter.x, m_d.m_vCenter.y, m_posZ);
      m_rd->DrawMesh(m_rd->m_basicShader, false, pos, 0.f, m_bracketMeshBuffer, RenderDevice::TRIANGLELIST, 0, spinnerBracketNumFaces);
   }

   if (m_phitspinner->m_spinnerMover.m_visible && !isStaticOnly)
   {
      UpdatePlate(nullptr);
      Vertex3Ds pos(m_d.m_vCenter.x, m_d.m_vCenter.y, m_posZ);
      m_rd->m_basicShader->SetBasic(m_ptable->GetMaterial(m_d.m_szMaterial), m_ptable->GetImage(m_d.m_szImage));
      m_rd->DrawMesh(m_rd->m_basicShader, false, pos, 0.f, m_plateMeshBuffer, RenderDevice::TRIANGLELIST, 0, spinnerPlateNumFaces);
   }
}

void Spinner::UpdatePlate(Vertex3D_NoTex2 * const vertBuffer)
{
   // early out in case still same rotation
   if (m_phitspinner->m_spinnerMover.m_angle == m_vertexBuffer_spinneranimangle)
       return;

   m_vertexBuffer_spinneranimangle = m_phitspinner->m_spinnerMover.m_angle;

   const Matrix3D fullMatrix = Matrix3D::MatrixRotateX(-m_phitspinner->m_spinnerMover.m_angle)
                             * Matrix3D::MatrixRotateZ(ANGTORAD(m_d.m_rotation));

   Vertex3D_NoTex2 *buf;
   if (vertBuffer == nullptr)
      m_plateMeshBuffer->m_vb->Lock(buf);
   else
      buf = vertBuffer;

   for (unsigned int i = 0; i < spinnerPlateNumVertices; i++)
   {
      Vertex3Ds vert = fullMatrix * Vertex3Ds{spinnerPlate[i].x, spinnerPlate[i].y, spinnerPlate[i].z};
      buf[i].x = vert.x*m_d.m_length + m_d.m_vCenter.x;
      buf[i].y = vert.y*m_d.m_length + m_d.m_vCenter.y;
      buf[i].z = vert.z*m_d.m_length + m_posZ;

      vert = fullMatrix.MultiplyVectorNoTranslate(Vertex3Ds{spinnerPlate[i].nx, spinnerPlate[i].ny, spinnerPlate[i].nz});
      buf[i].nx = vert.x;
      buf[i].ny = vert.y;
      buf[i].nz = vert.z;

      buf[i].tu = spinnerPlate[i].tu;
      buf[i].tv = spinnerPlate[i].tv;
   }
   if (vertBuffer == nullptr)
      m_plateMeshBuffer->m_vb->Unlock();
}

#pragma endregion


void Spinner::SetObjectPos()
{
   m_vpinball->SetObjectPosCur(m_d.m_vCenter.x, m_d.m_vCenter.y);
}

void Spinner::MoveOffset(const float dx, const float dy)
{
   m_d.m_vCenter.x += dx;
   m_d.m_vCenter.y += dy;
}

Vertex2D Spinner::GetCenter() const
{
   return m_d.m_vCenter;
}

void Spinner::PutCenter(const Vertex2D& pv)
{
   m_d.m_vCenter = pv;
}

void Spinner::SetDefaultPhysics(const bool fromMouseClick)
{
   m_d.m_elasticity = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(Settings::DefaultPropsSpinner, "Elasticity"s, 0.3f) : 0.3f;
   m_d.m_damping = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(Settings::DefaultPropsSpinner, "AntiFriction"s, 0.9879f) : 0.9879f;
}

HRESULT Spinner::SaveData(IStream *pstm, HCRYPTHASH hcrypthash, const bool saveForUndo)
{
   BiffWriter bw(pstm, hcrypthash);

   bw.WriteVector2(FID(VCEN), m_d.m_vCenter);
   bw.WriteFloat(FID(ROTA), m_d.m_rotation);
   bw.WriteBool(FID(TMON), m_d.m_tdr.m_TimerEnabled);
   bw.WriteInt(FID(TMIN), m_d.m_tdr.m_TimerInterval);
   bw.WriteFloat(FID(HIGH), m_d.m_height);
   bw.WriteFloat(FID(LGTH), m_d.m_length);
   bw.WriteFloat(FID(AFRC), m_d.m_damping);

   bw.WriteFloat(FID(SMAX), m_d.m_angleMax);
   bw.WriteFloat(FID(SMIN), m_d.m_angleMin);
   bw.WriteFloat(FID(SELA), m_d.m_elasticity);
   bw.WriteBool(FID(SVIS), m_d.m_visible);
   bw.WriteBool(FID(SSUP), m_d.m_showBracket);
   bw.WriteString(FID(MATR), m_d.m_szMaterial);
   bw.WriteString(FID(IMGF), m_d.m_szImage);
   bw.WriteString(FID(SURF), m_d.m_szSurface);
   bw.WriteWideString(FID(NAME), m_wzName);
   bw.WriteBool(FID(REEN), m_d.m_reflectionEnabled);

   ISelect::SaveData(pstm, hcrypthash);

   bw.WriteTag(FID(ENDB));

   return S_OK;
}

HRESULT Spinner::InitLoad(IStream *pstm, PinTable *ptable, int *pid, int version, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
{
   SetDefaults(false);

   BiffReader br(pstm, this, pid, version, hcrypthash, hcryptkey);

   m_ptable = ptable;

   br.Load();
   return S_OK;
}

bool Spinner::LoadToken(const int id, BiffReader * const pbr)
{
   switch(id)
   {
   case FID(PIID): pbr->GetInt((int *)pbr->m_pdata); break;
   case FID(VCEN): pbr->GetVector2(m_d.m_vCenter); break;
   case FID(ROTA): pbr->GetFloat(m_d.m_rotation); break;
   case FID(MATR): pbr->GetString(m_d.m_szMaterial); break;
   case FID(TMON): pbr->GetBool(m_d.m_tdr.m_TimerEnabled); break;
   case FID(TMIN): pbr->GetInt(m_d.m_tdr.m_TimerInterval); break;
   case FID(SSUP): pbr->GetBool(m_d.m_showBracket); break;
   case FID(HIGH): pbr->GetFloat(m_d.m_height); break;
   case FID(LGTH): pbr->GetFloat(m_d.m_length); break;
   case FID(AFRC): pbr->GetFloat(m_d.m_damping); break;
   case FID(SMAX): pbr->GetFloat(m_d.m_angleMax); break;
   case FID(SMIN): pbr->GetFloat(m_d.m_angleMin); break;
   case FID(SELA): pbr->GetFloat(m_d.m_elasticity); break;
   case FID(SVIS): pbr->GetBool(m_d.m_visible); break;
   case FID(IMGF): pbr->GetString(m_d.m_szImage); break;
   case FID(SURF): pbr->GetString(m_d.m_szSurface); break;
   case FID(NAME): pbr->GetWideString(m_wzName, std::size(m_wzName)); break;
   case FID(REEN): pbr->GetBool(m_d.m_reflectionEnabled); break;
   default: ISelect::LoadToken(id, pbr); break;
   }
   return true;
}

HRESULT Spinner::InitPostLoad()
{
   return S_OK;
}

STDMETHODIMP Spinner::InterfaceSupportsErrorInfo(REFIID riid)
{
   static const IID* arr[] =
   {
      &IID_ISpinner,
   };

   for (size_t i = 0; i < std::size(arr); i++)
      if (InlineIsEqualGUID(*arr[i], riid))
         return S_OK;

   return S_FALSE;
}

STDMETHODIMP Spinner::get_Length(float *pVal)
{
   *pVal = m_d.m_length;
   return S_OK;
}

STDMETHODIMP Spinner::put_Length(float newVal)
{
   m_d.m_length = newVal;
   return S_OK;
}

STDMETHODIMP Spinner::get_Rotation(float *pVal)
{
   *pVal = m_d.m_rotation;
   return S_OK;
}

STDMETHODIMP Spinner::put_Rotation(float newVal)
{
   m_d.m_rotation = newVal;
   return S_OK;
}

STDMETHODIMP Spinner::get_Height(float *pVal)
{
   *pVal = m_d.m_height;
   return S_OK;
}

STDMETHODIMP Spinner::put_Height(float newVal)
{
   m_d.m_height = newVal;
   return S_OK;
}

STDMETHODIMP Spinner::get_Damping(float *pVal)
{
   *pVal = !g_pplayer ? m_d.m_damping : powf(m_phitspinner->m_spinnerMover.m_damping,(float)(1.0/PHYS_FACTOR));
   return S_OK;
}

STDMETHODIMP Spinner::put_Damping(float newVal)
{
   const float tmp = clamp(newVal, 0.0f, 1.0f);
   if (g_pplayer)
      m_phitspinner->m_spinnerMover.m_damping = powf(tmp, (float)PHYS_FACTOR);
   else
      m_d.m_damping = tmp;

   return S_OK;
}

STDMETHODIMP Spinner::get_Material(BSTR *pVal)
{
   WCHAR wz[MAXNAMEBUFFER];
   MultiByteToWideCharNull(CP_ACP, 0, m_d.m_szMaterial.c_str(), -1, wz, MAXNAMEBUFFER);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP Spinner::put_Material(BSTR newVal)
{
   char buf[MAXNAMEBUFFER];
   WideCharToMultiByteNull(CP_ACP, 0, newVal, -1, buf, MAXNAMEBUFFER, nullptr, nullptr);
   m_d.m_szMaterial = buf;

   return S_OK;
}

STDMETHODIMP Spinner::get_Image(BSTR *pVal)
{
   WCHAR wz[MAXTOKEN];
   MultiByteToWideCharNull(CP_ACP, 0, m_d.m_szImage.c_str(), -1, wz, MAXTOKEN);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP Spinner::put_Image(BSTR newVal)
{
   char szImage[MAXTOKEN];
   WideCharToMultiByteNull(CP_ACP, 0, newVal, -1, szImage, MAXTOKEN, nullptr, nullptr);
   const Texture * const tex = m_ptable->GetImage(szImage);
   if (tex && tex->IsHDR())
   {
       ShowError("Cannot use a HDR image (.exr/.hdr) here");
       return E_FAIL;
   }
   m_d.m_szImage = szImage;

   return S_OK;
}

STDMETHODIMP Spinner::get_X(float *pVal)
{
   *pVal = m_d.m_vCenter.x;
   return S_OK;
}

STDMETHODIMP Spinner::put_X(float newVal)
{
   m_d.m_vCenter.x = newVal;
   return S_OK;
}

STDMETHODIMP Spinner::get_Y(float *pVal)
{
   *pVal = m_d.m_vCenter.y;
   return S_OK;
}

STDMETHODIMP Spinner::put_Y(float newVal)
{
   m_d.m_vCenter.y = newVal;
   return S_OK;
}

STDMETHODIMP Spinner::get_Surface(BSTR *pVal)
{
   WCHAR wz[MAXTOKEN];
   MultiByteToWideCharNull(CP_ACP, 0, m_d.m_szSurface.c_str(), -1, wz, MAXTOKEN);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP Spinner::put_Surface(BSTR newVal)
{
   char buf[MAXTOKEN];
   WideCharToMultiByteNull(CP_ACP, 0, newVal, -1, buf, MAXTOKEN, nullptr, nullptr);
   m_d.m_szSurface = buf;

   return S_OK;
}

STDMETHODIMP Spinner::get_ShowBracket(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_d.m_showBracket);
   return S_OK;
}

STDMETHODIMP Spinner::put_ShowBracket(VARIANT_BOOL newVal)
{
   m_d.m_showBracket = VBTOb(newVal);
   return S_OK;
}

STDMETHODIMP Spinner::get_AngleMax(float *pVal)
{
   *pVal = GetAngleMax();
   return S_OK;
}

STDMETHODIMP Spinner::put_AngleMax(float newVal)
{
   if (g_pplayer && (m_d.m_angleMin == m_d.m_angleMax)) // allow only if in limited angle mode
      return S_FAIL;

   SetAngleMax(newVal);
   return S_OK;
}

STDMETHODIMP Spinner::get_AngleMin(float *pVal)
{
   *pVal = (g_pplayer) ? RADTOANG(m_phitspinner->m_spinnerMover.m_angleMin) :	//player active value
                         m_d.m_angleMin;
   return S_OK;
}

STDMETHODIMP Spinner::put_AngleMin(float newVal)
{
   if (g_pplayer && (m_d.m_angleMin != m_d.m_angleMax))	// allow only if in limited angle mode
      return S_FAIL;

   SetAngleMin(newVal);
   return S_OK;
}

STDMETHODIMP Spinner::get_Elasticity(float *pVal)
{
   *pVal = (g_pplayer) ? m_phitspinner->m_spinnerMover.m_elasticity :	//player active value
                         m_d.m_elasticity;
   return S_OK;
}

STDMETHODIMP Spinner::put_Elasticity(float newVal)
{
   if (g_pplayer)
      m_phitspinner->m_spinnerMover.m_elasticity = newVal;	//player active value
   else
      m_d.m_elasticity = newVal;

   return S_OK;
}

STDMETHODIMP Spinner::get_Visible(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB((g_pplayer) ? m_phitspinner->m_spinnerMover.m_visible : m_d.m_visible);
   return S_OK;
}

STDMETHODIMP Spinner::put_Visible(VARIANT_BOOL newVal)
{
   if (g_pplayer)
      m_phitspinner->m_spinnerMover.m_visible = VBTOb(newVal);// && m_d.m_visible;
   else
      m_d.m_visible = VBTOb(newVal);

   return S_OK;
}

STDMETHODIMP Spinner::get_ReflectionEnabled(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_d.m_reflectionEnabled);
   return S_OK;
}

STDMETHODIMP Spinner::put_ReflectionEnabled(VARIANT_BOOL newVal)
{
   m_d.m_reflectionEnabled = VBTOb(newVal);
   return S_OK;
}

STDMETHODIMP Spinner::get_CurrentAngle(float *pVal)
{
   if (m_phitspinner)
   {
      *pVal = RADTOANG(m_phitspinner->m_spinnerMover.m_angle);
      return S_OK;
   }
   else
      return E_FAIL;
}
