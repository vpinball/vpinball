#include "StdAfx.h"
#include "objloader.h"
#include "meshes/spinnerBracketMesh.h"
#include "meshes/spinnerPlateMesh.h"

Spinner::Spinner()
{
   m_phitspinner = NULL;
   m_bracketVertexBuffer = 0;
   m_bracketIndexBuffer = 0;
   m_plateVertexBuffer = 0;
   m_plateIndexBuffer = 0;
   m_vertexBuffer_spinneranimangle = -FLT_MAX;
   memset(m_d.m_szSurface, 0, sizeof(m_d.m_szSurface));
}

Spinner::~Spinner()
{
   if (m_bracketVertexBuffer)
   {
      m_bracketVertexBuffer->release();
      m_bracketVertexBuffer = 0;
   }
   if (m_bracketIndexBuffer)
   {
      m_bracketIndexBuffer->release();
      m_bracketIndexBuffer = 0;
   }
   if (m_plateVertexBuffer)
   {
      m_plateVertexBuffer->release();
      m_plateVertexBuffer = 0;
   }
   if (m_plateIndexBuffer)
   {
      m_plateIndexBuffer->release();
      m_plateIndexBuffer = 0;
   }
}

void Spinner::UpdateStatusBarInfo()
{
   char tbuf[128];
   sprintf_s(tbuf, "Length: %.3f | Height: %.3f", m_vpinball->ConvertToUnit(m_d.m_length), m_vpinball->ConvertToUnit(m_d.m_height));
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

HRESULT Spinner::Init(PinTable *ptable, float x, float y, bool fromMouseClick)
{
   m_ptable = ptable;

   m_d.m_vCenter.x = x;
   m_d.m_vCenter.y = y;

   SetDefaults(fromMouseClick);

   InitVBA(fTrue, 0, NULL);

   return S_OK;
}


void Spinner::WriteRegDefaults()
{
   SaveValueFloat("DefaultProps\\Spinner", "Length", m_d.m_length);
   SaveValueFloat("DefaultProps\\Spinner", "Rotation", m_d.m_rotation);
   SaveValueBool("DefaultProps\\Spinner", "ShowBracket", m_d.m_showBracket);
   SaveValueFloat("DefaultProps\\Spinner", "Height", m_d.m_height);
   SaveValueFloat("DefaultProps\\Spinner", "AngleMax", m_d.m_angleMax);
   SaveValueFloat("DefaultProps\\Spinner", "AngleMin", m_d.m_angleMin);
   SaveValueFloat("DefaultProps\\Spinner", "Elasticity", m_d.m_elasticity);
   SaveValueFloat("DefaultProps\\Spinner", "AntiFriction", m_d.m_damping);
   SaveValueFloat("DefaultProps\\Spinner", "Scatter", m_d.m_scatter);
   SaveValueBool("DefaultProps\\Spinner", "Visible", m_d.m_visible);
   SaveValueBool("DefaultProps\\Spinner", "TimerEnabled", m_d.m_tdr.m_TimerEnabled);
   SaveValueInt("DefaultProps\\Spinner", "TimerInterval", m_d.m_tdr.m_TimerInterval);
   SaveValueString("DefaultProps\\Spinner", "Image", m_d.m_szImage);
   SaveValueString("DefaultProps\\Spinner", "Surface", m_d.m_szSurface);
   SaveValueBool("DefaultProps\\Spinner", "ReflectionEnabled", m_d.m_reflectionEnabled);
}

void Spinner::SetDefaults(bool fromMouseClick)
{
   m_d.m_length = fromMouseClick ? LoadValueFloatWithDefault("DefaultProps\\Spinner", "Length", 80.f) : 80.f;
   m_d.m_rotation = fromMouseClick ? LoadValueFloatWithDefault("DefaultProps\\Spinner", "Rotation", 0.f) : 0.f;
   m_d.m_showBracket = fromMouseClick ? LoadValueBoolWithDefault("DefaultProps\\Spinner", "ShowBracket", true) : true;
   m_d.m_height = (float)(fromMouseClick ? LoadValueIntWithDefault("DefaultProps\\Spinner", "Height", 60000) : 60000) / 1000.0f;

   SetDefaultPhysics(fromMouseClick);

   m_d.m_angleMax = fromMouseClick ? LoadValueFloatWithDefault("DefaultProps\\Spinner", "AngleMax", 0.f) : 0.f;
   m_d.m_angleMin = fromMouseClick ? LoadValueFloatWithDefault("DefaultProps\\Spinner", "AngleMin", 0.f) : 0.f;
   m_d.m_visible = fromMouseClick ? LoadValueBoolWithDefault("DefaultProps\\Spinner", "Visible", true) : true;
   m_d.m_reflectionEnabled = fromMouseClick ? LoadValueBoolWithDefault("DefaultProps\\Spinner", "ReflectionEnabled", true) : true;
   m_d.m_tdr.m_TimerEnabled = fromMouseClick ? LoadValueBoolWithDefault("DefaultProps\\Spinner", "TimerEnabled", false) : false;
   m_d.m_tdr.m_TimerInterval = fromMouseClick ? LoadValueIntWithDefault("DefaultProps\\Spinner", "TimerInterval", 100) : 100;

   char buf[MAXTOKEN] = { 0 };
   HRESULT hr = LoadValueString("DefaultProps\\Spinner", "Image", buf, MAXTOKEN);
   if ((hr != S_OK) || !fromMouseClick)
      m_d.m_szImage.clear();
   else
      m_d.m_szImage = buf;

   hr = LoadValueString("DefaultProps\\Spinner", "Surface", m_d.m_szSurface, MAXTOKEN);
   if ((hr != S_OK) || !fromMouseClick)
      m_d.m_szSurface[0] = 0;
}

void Spinner::UIRenderPass1(Sur * const psur)
{
}

void Spinner::UIRenderPass2(Sur * const psur)
{
   psur->SetBorderColor(RGB(0, 0, 0), false, 0);
   psur->SetLineColor(RGB(0, 0, 0), false, 3);
   psur->SetObject(this);

   float halflength = m_d.m_length * 0.5f;

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

void Spinner::GetTimers(vector<HitTimer*> &pvht)
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
// Ported at: VisualPinball.Engine/VPT/Spinner/SpinnerHitGenerator.cs
//

void Spinner::GetHitShapes(vector<HitObject*> &pvho)
{
   const float height = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y);
   const float h = m_d.m_height + 30.0f;

   const float angleMin = min(m_d.m_angleMin, m_d.m_angleMax); // correct angle inversions
   const float angleMax = max(m_d.m_angleMin, m_d.m_angleMax);

   m_d.m_angleMin = angleMin;
   m_d.m_angleMax = angleMax;

   HitSpinner * const phitspinner = new HitSpinner(this, height);
   m_phitspinner = phitspinner;

   pvho.push_back(phitspinner);

   if (m_d.m_showBracket)
   {
      /*add a hit shape for the bracket if shown, just in case if the bracket spinner height is low enough so the ball can hit it*/
      const float halflength = m_d.m_length * 0.5f + (m_d.m_length*0.1875f);
      const float radangle = ANGTORAD(m_d.m_rotation);
      const float sn = sinf(radangle);
      const float cs = cosf(radangle);

      HitCircle *phitcircle;
      phitcircle = new HitCircle(Vertex2D(m_d.m_vCenter.x + cs*halflength, m_d.m_vCenter.y + sn*halflength), m_d.m_length*0.075f, height + m_d.m_height, height + h);
      pvho.push_back(phitcircle);

      phitcircle = new HitCircle(Vertex2D(m_d.m_vCenter.x - cs*halflength, m_d.m_vCenter.y - sn*halflength), m_d.m_length*0.075f, height + m_d.m_height, height + h);
      pvho.push_back(phitcircle);
   }
}

//
// end of license:GPLv3+, back to 'old MAME'-like
//

void Spinner::GetHitShapesDebug(vector<HitObject*> &pvho)
{
}

void Spinner::EndPlay()
{
   IEditable::EndPlay();
   m_phitspinner = NULL;

   if (m_bracketVertexBuffer)
   {
      m_bracketVertexBuffer->release();
      m_bracketVertexBuffer = 0;
   }
   if (m_bracketIndexBuffer)
   {
      m_bracketIndexBuffer->release();
      m_bracketIndexBuffer = 0;
   }
   if (m_plateVertexBuffer)
   {
      m_plateVertexBuffer->release();
      m_plateVertexBuffer = 0;
   }
   if (m_plateIndexBuffer)
   {
      m_plateIndexBuffer->release();
      m_plateIndexBuffer = 0;
   }
}

void Spinner::ExportMesh(FILE *f)
{
   char name[sizeof(m_wzName)/sizeof(m_wzName[0])];
   WideCharToMultiByte(CP_ACP, 0, m_wzName, -1, name, sizeof(name), NULL, NULL);
   std::vector<Vertex3D_NoTex2> transformedVertices;
   vector<HitObject*> dummyHitObj;

   const float height = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y)*m_ptable->m_BG_scalez[m_ptable->m_BG_current_set];
   m_posZ = height + m_d.m_height;

   GetHitShapes(dummyHitObj);

   if (m_d.m_showBracket)
   {
      const string subObjName = name + string("Bracket");
      WaveFrontObj_WriteObjectName(f, subObjName);

      m_fullMatrix.RotateZMatrix(ANGTORAD(m_d.m_rotation));

      transformedVertices.resize(spinnerBracketNumVertices);

      for (int i = 0; i < spinnerBracketNumVertices; i++)
      {
         Vertex3Ds vert(spinnerBracket[i].x, spinnerBracket[i].y, spinnerBracket[i].z);
         vert = m_fullMatrix.MultiplyVector(vert);
         transformedVertices[i].x = vert.x*m_d.m_length + m_d.m_vCenter.x;
         transformedVertices[i].y = vert.y*m_d.m_length + m_d.m_vCenter.y;
         transformedVertices[i].z = vert.z*m_d.m_length*m_ptable->m_BG_scalez[m_ptable->m_BG_current_set] + m_posZ;

         vert = Vertex3Ds(spinnerBracket[i].nx, spinnerBracket[i].ny, spinnerBracket[i].nz);
         vert = m_fullMatrix.MultiplyVectorNoTranslate(vert);
         transformedVertices[i].nx = vert.x;
         transformedVertices[i].ny = vert.y;
         transformedVertices[i].nz = vert.z;

         transformedVertices[i].tu = spinnerBracket[i].tu;
         transformedVertices[i].tv = spinnerBracket[i].tv;
      }
      WaveFrontObj_WriteVertexInfo(f, transformedVertices.data(), spinnerBracketNumVertices);

      const Material * const mat = m_ptable->GetMaterial(m_d.m_szMaterial);
      WaveFrontObj_WriteMaterial(m_d.m_szMaterial, string(), mat);
      WaveFrontObj_UseTexture(f, m_d.m_szMaterial);
      WaveFrontObj_WriteFaceInfoList(f, spinnerBracketIndices, spinnerBracketNumFaces);
      WaveFrontObj_UpdateFaceOffset(spinnerBracketNumVertices);

      transformedVertices.clear();
   }

   transformedVertices.resize(spinnerPlateNumVertices);
   m_vertexBuffer_spinneranimangle = -FLT_MAX;
   UpdatePlate(transformedVertices.data());

   const string subObjName = name + string("Plate");
   WaveFrontObj_WriteObjectName(f, subObjName);

   WaveFrontObj_WriteVertexInfo(f, transformedVertices.data(), spinnerPlateNumVertices);
   WaveFrontObj_WriteFaceInfoList(f, spinnerPlateIndices, spinnerPlateNumFaces);
   WaveFrontObj_UpdateFaceOffset(spinnerPlateNumVertices);
   transformedVertices.clear();
}

void Spinner::UpdatePlate(Vertex3D_NoTex2 * const vertBuffer)
{
   // early out in case still same rotation
   if (m_phitspinner->m_spinnerMover.m_angle == m_vertexBuffer_spinneranimangle)
       return;

   m_vertexBuffer_spinneranimangle = m_phitspinner->m_spinnerMover.m_angle;

   Matrix3D fullMatrix;
   Matrix3D rotzMat, rotxMat;

   fullMatrix.SetIdentity();
   rotxMat.RotateXMatrix(-m_phitspinner->m_spinnerMover.m_angle);
   rotxMat.Multiply(fullMatrix, fullMatrix);
   rotzMat.RotateZMatrix(ANGTORAD(m_d.m_rotation));
   rotzMat.Multiply(fullMatrix, fullMatrix);

   Vertex3D_NoTex2 *buf;
   if (vertBuffer == NULL)
      m_plateVertexBuffer->lock(0, 0, (void**)&buf, VertexBuffer::DISCARDCONTENTS);
   else
      buf = vertBuffer;

   for (int i = 0; i < spinnerPlateNumVertices; i++)
   {
      Vertex3Ds vert(spinnerPlate[i].x, spinnerPlate[i].y, spinnerPlate[i].z);
      vert = fullMatrix.MultiplyVector(vert);
      buf[i].x = vert.x*m_d.m_length + m_d.m_vCenter.x;
      buf[i].y = vert.y*m_d.m_length + m_d.m_vCenter.y;
      buf[i].z = vert.z*m_d.m_length*m_ptable->m_BG_scalez[m_ptable->m_BG_current_set] + m_posZ;

      vert = Vertex3Ds(spinnerPlate[i].nx, spinnerPlate[i].ny, spinnerPlate[i].nz);
      vert = fullMatrix.MultiplyVectorNoTranslate(vert);
      buf[i].nx = vert.x;
      buf[i].ny = vert.y;
      buf[i].nz = vert.z;

      buf[i].tu = spinnerPlate[i].tu;
      buf[i].tv = spinnerPlate[i].tv;
   }
   if (vertBuffer == NULL)
      m_plateVertexBuffer->unlock();
}

void Spinner::RenderDynamic()
{
   RenderDevice * const pd3dDevice = g_pplayer->m_pin3d.m_pd3dPrimaryDevice;

   TRACE_FUNCTION();

   if (!m_phitspinner->m_spinnerMover.m_visible || !m_d.m_visible)
      return;

   if (m_ptable->m_reflectionEnabled && !m_d.m_reflectionEnabled)
      return;

   UpdatePlate(NULL);

   const Material * const mat = m_ptable->GetMaterial(m_d.m_szMaterial);
   pd3dDevice->basicShader->SetMaterial(mat);

   pd3dDevice->SetRenderState(RenderDevice::DEPTHBIAS, 0);
   pd3dDevice->SetRenderState(RenderDevice::ZWRITEENABLE, RenderDevice::RS_TRUE);
   pd3dDevice->SetRenderState(RenderDevice::CULLMODE, RenderDevice::CULL_CCW);

   Texture * const image = m_ptable->GetImage(m_d.m_szImage);
   if (image)
   {
      pd3dDevice->basicShader->SetTechnique(mat->m_bIsMetal ? "basic_with_texture_isMetal" : "basic_with_texture_isNotMetal");
      pd3dDevice->basicShader->SetTexture("Texture0", image, false);
      pd3dDevice->basicShader->SetAlphaTestValue(image->m_alphaTestValue * (float)(1.0 / 255.0));
   }
   else // No image by that name
      pd3dDevice->basicShader->SetTechnique(mat->m_bIsMetal ? "basic_without_texture_isMetal" : "basic_without_texture_isNotMetal");

   pd3dDevice->basicShader->Begin(0);
   pd3dDevice->DrawIndexedPrimitiveVB(RenderDevice::TRIANGLELIST, MY_D3DFVF_NOTEX2_VERTEX, m_plateVertexBuffer, 0, spinnerPlateNumVertices, m_plateIndexBuffer, 0, spinnerPlateNumFaces);
   pd3dDevice->basicShader->End();

   //    g_pplayer->UpdateBasicShaderMatrix();

   //    pd3dDevice->SetRenderState(RenderDevice::CULLMODE, RenderDevice::CULL_CCW);
}


void Spinner::RenderSetup()
{
   if (!m_d.m_visible)
      return;

   RenderDevice * const pd3dDevice = g_pplayer->m_pin3d.m_pd3dPrimaryDevice;

   const float height = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y)*m_ptable->m_BG_scalez[m_ptable->m_BG_current_set];
   m_posZ = height + m_d.m_height;

   if (m_bracketIndexBuffer)
      m_bracketIndexBuffer->release();
   m_bracketIndexBuffer = pd3dDevice->CreateAndFillIndexBuffer(spinnerBracketNumFaces, spinnerBracketIndices);

   if (m_bracketVertexBuffer)
      m_bracketVertexBuffer->release();
   pd3dDevice->CreateVertexBuffer(spinnerBracketNumVertices, 0, MY_D3DFVF_NOTEX2_VERTEX, &m_bracketVertexBuffer);

   m_fullMatrix.RotateZMatrix(ANGTORAD(m_d.m_rotation));

   Vertex3D_NoTex2 *buf;
   m_bracketVertexBuffer->lock(0, 0, (void**)&buf, VertexBuffer::WRITEONLY);
   for (int i = 0; i < spinnerBracketNumVertices; i++)
   {
      Vertex3Ds vert(spinnerBracket[i].x, spinnerBracket[i].y, spinnerBracket[i].z);
      vert = m_fullMatrix.MultiplyVector(vert);
      buf[i].x = vert.x*m_d.m_length + m_d.m_vCenter.x;
      buf[i].y = vert.y*m_d.m_length + m_d.m_vCenter.y;
      buf[i].z = vert.z*m_d.m_length*m_ptable->m_BG_scalez[m_ptable->m_BG_current_set] + m_posZ;

      vert = Vertex3Ds(spinnerBracket[i].nx, spinnerBracket[i].ny, spinnerBracket[i].nz);
      vert = m_fullMatrix.MultiplyVectorNoTranslate(vert);
      buf[i].nx = vert.x;
      buf[i].ny = vert.y;
      buf[i].nz = vert.z;

      buf[i].tu = spinnerBracket[i].tu;
      buf[i].tv = spinnerBracket[i].tv;
   }
   m_bracketVertexBuffer->unlock();

   if (m_plateIndexBuffer)
      m_plateIndexBuffer->release();
   m_plateIndexBuffer = pd3dDevice->CreateAndFillIndexBuffer(spinnerPlateNumFaces, spinnerPlateIndices);

   if (m_plateVertexBuffer)
      m_plateVertexBuffer->release();
   pd3dDevice->CreateVertexBuffer(spinnerPlateNumVertices, USAGE_DYNAMIC, MY_D3DFVF_NOTEX2_VERTEX, &m_plateVertexBuffer);

   m_vertexBuffer_spinneranimangle = -FLT_MAX;
   UpdatePlate(NULL);
}

void Spinner::RenderStatic()
{
   if (!m_d.m_showBracket || !m_d.m_visible)
      return;

   if (m_ptable->m_reflectionEnabled && !m_d.m_reflectionEnabled)
      return;

   RenderDevice * const pd3dDevice = g_pplayer->m_pin3d.m_pd3dPrimaryDevice;
   Pin3D * const ppin3d = &g_pplayer->m_pin3d;

   Material mat;
   mat.m_bIsMetal = true;
   mat.m_cBase = 0x20202020;
   mat.m_fWrapLighting = 0.9f;
   mat.m_cGlossy = 0x60606060;
   mat.m_fRoughness = 0.4f;
   mat.m_fGlossyImageLerp = 1.0f;
   mat.m_fThickness = 0.05f;
   mat.m_cClearcoat = 0x20202020;
   mat.m_fEdge = 1.0f;
   mat.m_fEdgeAlpha = 1.0f;
   pd3dDevice->basicShader->SetMaterial(&mat);
   pd3dDevice->basicShader->SetTechnique(mat.m_bIsMetal ? "basic_without_texture_isMetal" : "basic_without_texture_isNotMetal");
   ppin3d->EnableAlphaBlend(false);

   pd3dDevice->basicShader->Begin(0);
   pd3dDevice->DrawIndexedPrimitiveVB(RenderDevice::TRIANGLELIST, MY_D3DFVF_NOTEX2_VERTEX, m_bracketVertexBuffer, 0, spinnerBracketNumVertices, m_bracketIndexBuffer, 0, spinnerBracketNumFaces);
   pd3dDevice->basicShader->End();
}

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

void Spinner::SetDefaultPhysics(bool fromMouseClick)
{
   m_d.m_elasticity = fromMouseClick ? LoadValueFloatWithDefault("DefaultProps\\Spinner", "Elasticity", 0.3f) : 0.3f;
   m_d.m_damping = fromMouseClick ? LoadValueFloatWithDefault("DefaultProps\\Spinner", "AntiFriction", 0.9879f) : 0.9879f;
}

HRESULT Spinner::SaveData(IStream *pstm, HCRYPTHASH hcrypthash, const bool backupForPlay)
{
   BiffWriter bw(pstm, hcrypthash);

   bw.WriteStruct(FID(VCEN), &m_d.m_vCenter, sizeof(Vertex2D));
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
   case FID(VCEN): pbr->GetStruct(&m_d.m_vCenter, sizeof(Vertex2D)); break;
   case FID(ROTA): pbr->GetFloat(&m_d.m_rotation); break;
   case FID(MATR): pbr->GetString(m_d.m_szMaterial); break;
   case FID(TMON): pbr->GetBool(&m_d.m_tdr.m_TimerEnabled); break;
   case FID(TMIN): pbr->GetInt(&m_d.m_tdr.m_TimerInterval); break;
   case FID(SSUP): pbr->GetBool(&m_d.m_showBracket); break;
   case FID(HIGH): pbr->GetFloat(&m_d.m_height); break;
   case FID(LGTH): pbr->GetFloat(&m_d.m_length); break;
   case FID(AFRC): pbr->GetFloat(&m_d.m_damping); break;
   case FID(SMAX): pbr->GetFloat(&m_d.m_angleMax); break;
   case FID(SMIN): pbr->GetFloat(&m_d.m_angleMin); break;
   case FID(SELA): pbr->GetFloat(&m_d.m_elasticity); break;
   case FID(SVIS): pbr->GetBool(&m_d.m_visible); break;
   case FID(IMGF): pbr->GetString(m_d.m_szImage); break;
   case FID(SURF): pbr->GetString(m_d.m_szSurface); break;
   case FID(NAME): pbr->GetWideString(m_wzName); break;
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

   for (int i = 0; i < sizeof(arr) / sizeof(arr[0]); i++)
   {
      if (InlineIsEqualGUID(*arr[i], riid))
         return S_OK;
   }
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
   MultiByteToWideChar(CP_ACP, 0, m_d.m_szMaterial.c_str(), -1, wz, MAXNAMEBUFFER);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP Spinner::put_Material(BSTR newVal)
{
   char buf[MAXNAMEBUFFER];
   WideCharToMultiByte(CP_ACP, 0, newVal, -1, buf, MAXNAMEBUFFER, NULL, NULL);
   m_d.m_szMaterial = buf;

   return S_OK;
}

STDMETHODIMP Spinner::get_Image(BSTR *pVal)
{
   WCHAR wz[MAXTOKEN];
   MultiByteToWideChar(CP_ACP, 0, m_d.m_szImage.c_str(), -1, wz, MAXTOKEN);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP Spinner::put_Image(BSTR newVal)
{
   char szImage[MAXTOKEN];
   WideCharToMultiByte(CP_ACP, 0, newVal, -1, szImage, MAXTOKEN, NULL, NULL);
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
   MultiByteToWideChar(CP_ACP, 0, m_d.m_szSurface, -1, wz, MAXTOKEN);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP Spinner::put_Surface(BSTR newVal)
{
   WideCharToMultiByte(CP_ACP, 0, newVal, -1, m_d.m_szSurface, MAXTOKEN, NULL, NULL);

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
