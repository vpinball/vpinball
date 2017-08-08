#include "StdAfx.h"
#include "objloader.h"
#include "meshes/spinnerBracketMesh.h"
#include "meshes/spinnerPlateMesh.h"

Spinner::Spinner()
{
   m_phitspinner = NULL;
   bracketVertexBuffer = 0;
   bracketIndexBuffer = 0;
   plateVertexBuffer = 0;
   plateIndexBuffer = 0;
   vertexBuffer_spinneranimangle = -FLT_MAX;
   memset(m_d.m_szImage, 0, MAXTOKEN);
   memset(m_d.m_szMaterial, 0, 32);
   memset(m_d.m_szSurface, 0, MAXTOKEN);
}

Spinner::~Spinner()
{
   if (bracketVertexBuffer)
   {
      bracketVertexBuffer->release();
      bracketVertexBuffer = 0;
   }
   if (bracketIndexBuffer)
   {
      bracketIndexBuffer->release();
      bracketIndexBuffer = 0;
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
   SetRegValueFloat("DefaultProps\\Spinner", "Length", m_d.m_length);
   SetRegValueFloat("DefaultProps\\Spinner", "Rotation", m_d.m_rotation);
   SetRegValueBool("DefaultProps\\Spinner", "ShowBracket", m_d.m_fShowBracket);
   SetRegValueFloat("DefaultProps\\Spinner", "Height", m_d.m_height);
   SetRegValueFloat("DefaultProps\\Spinner", "AngleMax", m_d.m_angleMax);
   SetRegValueFloat("DefaultProps\\Spinner", "AngleMin", m_d.m_angleMin);
   SetRegValueFloat("DefaultProps\\Spinner", "Elasticity", m_d.m_elasticity);
   SetRegValueFloat("DefaultProps\\Spinner","AntiFriction", m_d.m_damping);
   SetRegValueFloat("DefaultProps\\Spinner", "Scatter", m_d.m_scatter);
   SetRegValue("DefaultProps\\Spinner", "Visible", REG_DWORD, &m_d.m_fVisible, 4);
   SetRegValueBool("DefaultProps\\Spinner", "TimerEnabled", m_d.m_tdr.m_fTimerEnabled);
   SetRegValue("DefaultProps\\Spinner", "TimerInterval", REG_DWORD, &m_d.m_tdr.m_TimerInterval, 4);
   SetRegValue("DefaultProps\\Spinner", "Image", REG_SZ, &m_d.m_szImage, lstrlen(m_d.m_szImage));
   SetRegValue("DefaultProps\\Spinner", "Surface", REG_SZ, &m_d.m_szSurface, lstrlen(m_d.m_szSurface));
   SetRegValue("DefaultProps\\Spinner", "ReflectionEnabled", REG_DWORD, &m_d.m_fReflectionEnabled, 4);
}

void Spinner::SetDefaults(bool fromMouseClick)
{
   HRESULT hr;
   int iTmp;
   float fTmp;

   hr = GetRegStringAsFloat("DefaultProps\\Spinner", "Length", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_length = fTmp;
   else
      m_d.m_length = 80;

   hr = GetRegStringAsFloat("DefaultProps\\Spinner", "Rotation", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_rotation = fTmp;
   else
      m_d.m_rotation = 0;

   hr = GetRegInt("DefaultProps\\Spinner", "ShowBracket", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_fShowBracket = iTmp == 0 ? false : true;
   else
      m_d.m_fShowBracket = true;

   hr = GetRegInt("DefaultProps\\Spinner", "Height", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_height = (float)iTmp / 1000.0f;
   else
      m_d.m_height = 60;

   SetDefaultPhysics(fromMouseClick);

   hr = GetRegStringAsFloat("DefaultProps\\Spinner", "AngleMax", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_angleMax = fTmp;
   else
      m_d.m_angleMax = 0;

   hr = GetRegStringAsFloat("DefaultProps\\Spinner", "AngleMin", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_angleMin = fTmp;
   else
      m_d.m_angleMin = 0;

   hr = GetRegInt("DefaultProps\\Spinner", "Visible", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_fVisible = iTmp == 0 ? false : true;
   else
      m_d.m_fVisible = true;

   hr = GetRegInt("DefaultProps\\Spinner", "ReflectionEnabled", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_fReflectionEnabled = iTmp == 0 ? false : true;
   else
      m_d.m_fReflectionEnabled = true;

   hr = GetRegInt("DefaultProps\\Spinner", "TimerEnabled", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_tdr.m_fTimerEnabled = iTmp == 0 ? false : true;
   else
      m_d.m_tdr.m_fTimerEnabled = false;

   hr = GetRegInt("DefaultProps\\Spinner", "TimerInterval", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_tdr.m_TimerInterval = iTmp;
   else
      m_d.m_tdr.m_TimerInterval = 100;

   hr = GetRegString("DefaultProps\\Spinner", "Image", m_d.m_szImage, MAXTOKEN);
   if ((hr != S_OK) || !fromMouseClick)
      m_d.m_szImage[0] = 0;

   hr = GetRegString("DefaultProps\\Spinner", "Surface", &m_d.m_szSurface, MAXTOKEN);
   if ((hr != S_OK) || !fromMouseClick)
      m_d.m_szSurface[0] = 0;
}

void Spinner::PreRender(Sur * const psur)
{
}

void Spinner::Render(Sur * const psur)
{
   psur->SetLineColor(RGB(0, 0, 0), false, 3);
   psur->SetObject(this);

   float halflength = m_d.m_length * 0.5f;

   const float radangle = ANGTORAD(m_d.m_rotation);
   const float sn = sinf(radangle);
   const float cs = cosf(radangle);

   psur->Line(m_d.m_vCenter.x + cs*halflength, m_d.m_vCenter.y + sn*halflength,
      m_d.m_vCenter.x - cs*halflength, m_d.m_vCenter.y - sn*halflength);

   psur->SetLineColor(RGB(0, 0, 0), false, 1);
   psur->SetObject(this);

   psur->Line(m_d.m_vCenter.x + cs*halflength, m_d.m_vCenter.y + sn*halflength,
      m_d.m_vCenter.x - cs*halflength, m_d.m_vCenter.y - sn*halflength);
}

void Spinner::GetTimers(Vector<HitTimer> * const pvht)
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

void Spinner::GetHitShapes(Vector<HitObject> * const pvho)
{
   const float height = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y);
   const float h = m_d.m_height + 30.0f;

   const float angleMin = min(m_d.m_angleMin, m_d.m_angleMax); // correct angle inversions
   const float angleMax = max(m_d.m_angleMin, m_d.m_angleMax);

   m_d.m_angleMin = angleMin;
   m_d.m_angleMax = angleMax;

   HitSpinner * const phitspinner = new HitSpinner(this, height);
   m_phitspinner = phitspinner;

   pvho->AddElement(phitspinner);

   if (m_d.m_fShowBracket)
   {
      /*add a hit shape for the bracket if shown, just in case if the bracket spinner height is low enough so the ball can hit it*/
      const float halflength = m_d.m_length * 0.5f + (m_d.m_length*0.1875f);
      const float radangle = ANGTORAD(m_d.m_rotation);
      const float sn = sinf(radangle);
      const float cs = cosf(radangle);

      HitCircle *phitcircle;
      phitcircle = new HitCircle(Vertex2D(m_d.m_vCenter.x + cs*halflength, m_d.m_vCenter.y + sn*halflength), m_d.m_length*0.075f, height + m_d.m_height, height + h);
      phitcircle->m_pfe = NULL;
      pvho->AddElement(phitcircle);

      phitcircle = new HitCircle(Vertex2D(m_d.m_vCenter.x - cs*halflength, m_d.m_vCenter.y - sn*halflength), m_d.m_length*0.075f, height + m_d.m_height, height + h);
      phitcircle->m_pfe = NULL;
      pvho->AddElement(phitcircle);
   }
}

void Spinner::GetHitShapesDebug(Vector<HitObject> * const pvho)
{
}

void Spinner::EndPlay()
{
   IEditable::EndPlay();
   m_phitspinner = NULL;

   if (bracketVertexBuffer)
   {
      bracketVertexBuffer->release();
      bracketVertexBuffer = 0;
   }
   if (bracketIndexBuffer)
   {
      bracketIndexBuffer->release();
      bracketIndexBuffer = 0;
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

void Spinner::ExportMesh(FILE *f)
{
   char name[MAX_PATH];
   char subObjName[MAX_PATH];
   WideCharToMultiByte(CP_ACP, 0, m_wzName, -1, name, MAX_PATH, NULL, NULL);
   std::vector<Vertex3D_NoTex2> transformedVertices;
   Vector<HitObject> dummyHitObj;

   const float height = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y)*m_ptable->m_BG_scalez[m_ptable->m_BG_current_set];
   m_posZ = height + m_d.m_height;

   GetHitShapes(&dummyHitObj);

   if (m_d.m_fShowBracket)
   {
      strcpy_s(subObjName, name);
      strcat_s(subObjName, "Bracket");
      WaveFrontObj_WriteObjectName(f, subObjName);

      fullMatrix.RotateZMatrix(ANGTORAD(m_d.m_rotation));

      transformedVertices.resize(spinnerBracketNumVertices);

      for (int i = 0; i < spinnerBracketNumVertices; i++)
      {
         Vertex3Ds vert(spinnerBracket[i].x, spinnerBracket[i].y, spinnerBracket[i].z);
         vert = fullMatrix.MultiplyVector(vert);
         transformedVertices[i].x = vert.x*m_d.m_length + m_d.m_vCenter.x;
         transformedVertices[i].y = vert.y*m_d.m_length + m_d.m_vCenter.y;
         transformedVertices[i].z = vert.z*m_d.m_length*m_ptable->m_BG_scalez[m_ptable->m_BG_current_set] + m_posZ;

         vert = Vertex3Ds(spinnerBracket[i].nx, spinnerBracket[i].ny, spinnerBracket[i].nz);
         vert = fullMatrix.MultiplyVectorNoTranslate(vert);
         transformedVertices[i].nx = vert.x;
         transformedVertices[i].ny = vert.y;
         transformedVertices[i].nz = vert.z;

         transformedVertices[i].tu = spinnerBracket[i].tu;
         transformedVertices[i].tv = spinnerBracket[i].tv;
      }
      WaveFrontObj_WriteVertexInfo(f, transformedVertices.data(), spinnerBracketNumVertices);

      const Material * mat = m_ptable->GetMaterial(m_d.m_szMaterial);
      WaveFrontObj_WriteMaterial(m_d.m_szMaterial, NULL, mat);
      WaveFrontObj_UseTexture(f, m_d.m_szMaterial);
      WaveFrontObj_WriteFaceInfoList(f, spinnerBracketIndices, spinnerBracketNumFaces);
      WaveFrontObj_UpdateFaceOffset(spinnerBracketNumVertices);

      transformedVertices.clear();
   }

   transformedVertices.resize(spinnerPlateNumVertices);
   vertexBuffer_spinneranimangle = -FLT_MAX;
   UpdatePlate(NULL, transformedVertices.data());

   strcpy_s(subObjName, name);
   strcat_s(subObjName, "Plate");
   WaveFrontObj_WriteObjectName(f, subObjName);

   WaveFrontObj_WriteVertexInfo(f, transformedVertices.data(), spinnerPlateNumVertices);
   WaveFrontObj_WriteFaceInfoList(f, spinnerPlateIndices, spinnerPlateNumFaces);
   WaveFrontObj_UpdateFaceOffset(spinnerPlateNumVertices);
   transformedVertices.clear();
}

void Spinner::UpdatePlate(RenderDevice *pd3dDevice, Vertex3D_NoTex2 *vertBuffer)
{
   // early out in case still same rotation
   if( m_phitspinner->m_spinneranim.m_angle == vertexBuffer_spinneranimangle)
       return;

   vertexBuffer_spinneranimangle = m_phitspinner->m_spinneranim.m_angle;

   Matrix3D _fullMatrix;
   Matrix3D rotzMat, rotxMat;

   _fullMatrix.SetIdentity();
   rotxMat.RotateXMatrix(-m_phitspinner->m_spinneranim.m_angle);
   rotxMat.Multiply(_fullMatrix, _fullMatrix);
   rotzMat.RotateZMatrix(ANGTORAD(m_d.m_rotation));
   rotzMat.Multiply(_fullMatrix, _fullMatrix);

   Vertex3D_NoTex2 *buf;
   if (pd3dDevice != NULL && vertBuffer == NULL)
      plateVertexBuffer->lock(0, 0, (void**)&buf, VertexBuffer::DISCARDCONTENTS);
   else
      buf = vertBuffer;

   for (int i = 0; i < spinnerPlateNumVertices; i++)
   {
      Vertex3Ds vert(spinnerPlate[i].x, spinnerPlate[i].y, spinnerPlate[i].z);
      vert = _fullMatrix.MultiplyVector(vert);
      buf[i].x = vert.x*m_d.m_length + m_d.m_vCenter.x;
      buf[i].y = vert.y*m_d.m_length + m_d.m_vCenter.y;
      buf[i].z = vert.z*m_d.m_length*m_ptable->m_BG_scalez[m_ptable->m_BG_current_set] + m_posZ;

      vert = Vertex3Ds(spinnerPlate[i].nx, spinnerPlate[i].ny, spinnerPlate[i].nz);
      vert = _fullMatrix.MultiplyVectorNoTranslate(vert);
      buf[i].nx = vert.x;
      buf[i].ny = vert.y;
      buf[i].nz = vert.z;

      buf[i].tu = spinnerPlate[i].tu;
      buf[i].tv = spinnerPlate[i].tv;
   }
   if ( pd3dDevice!=NULL && vertBuffer==NULL)
      plateVertexBuffer->unlock();
}

void Spinner::PostRenderStatic(RenderDevice* pd3dDevice)
{
   TRACE_FUNCTION();

   if (!m_phitspinner->m_spinneranim.m_fVisible || !m_d.m_fVisible)
      return;

   if (m_ptable->m_fReflectionEnabled && !m_d.m_fReflectionEnabled)
      return;

   UpdatePlate(pd3dDevice);

   const Material * const mat = m_ptable->GetMaterial(m_d.m_szMaterial);
   pd3dDevice->basicShader->SetMaterial(mat);

   pd3dDevice->SetRenderState(RenderDevice::DEPTHBIAS, 0);
   pd3dDevice->SetRenderState(RenderDevice::ZWRITEENABLE, TRUE);
   pd3dDevice->SetRenderState(RenderDevice::CULLMODE, D3DCULL_CCW);

   Texture * const image = m_ptable->GetImage(m_d.m_szImage);
   if (image)
   {
      pd3dDevice->basicShader->SetTechnique(mat->m_bIsMetal ? "basic_with_texture_isMetal" : "basic_with_texture_isNotMetal");
      pd3dDevice->basicShader->SetTexture("Texture0", image);
      pd3dDevice->basicShader->SetAlphaTestValue(image->m_alphaTestValue * (float)(1.0 / 255.0));
   }
   else // No image by that name
      pd3dDevice->basicShader->SetTechnique(mat->m_bIsMetal ? "basic_without_texture_isMetal" : "basic_without_texture_isNotMetal");

   pd3dDevice->basicShader->Begin(0);
   pd3dDevice->DrawIndexedPrimitiveVB(D3DPT_TRIANGLELIST, MY_D3DFVF_NOTEX2_VERTEX, plateVertexBuffer, 0, spinnerPlateNumVertices, plateIndexBuffer, 0, spinnerPlateNumFaces);
   pd3dDevice->basicShader->End();

   //    g_pplayer->UpdateBasicShaderMatrix();

   //    pd3dDevice->SetRenderState(RenderDevice::CULLMODE, D3DCULL_CCW);
}


void Spinner::RenderSetup(RenderDevice* pd3dDevice)
{
   if (!m_d.m_fVisible)
      return;

   const float height = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y)*m_ptable->m_BG_scalez[m_ptable->m_BG_current_set];
   m_posZ = height + m_d.m_height;

   if (bracketIndexBuffer)
      bracketIndexBuffer->release();
   bracketIndexBuffer = pd3dDevice->CreateAndFillIndexBuffer(spinnerBracketNumFaces, spinnerBracketIndices);

   if (bracketVertexBuffer)
      bracketVertexBuffer->release();
   pd3dDevice->CreateVertexBuffer(spinnerBracketNumVertices, 0, MY_D3DFVF_NOTEX2_VERTEX, &bracketVertexBuffer);

   fullMatrix.RotateZMatrix(ANGTORAD(m_d.m_rotation));

   Vertex3D_NoTex2 *buf;
   bracketVertexBuffer->lock(0, 0, (void**)&buf, VertexBuffer::WRITEONLY);
   for (int i = 0; i < spinnerBracketNumVertices; i++)
   {
      Vertex3Ds vert(spinnerBracket[i].x, spinnerBracket[i].y, spinnerBracket[i].z);
      vert = fullMatrix.MultiplyVector(vert);
      buf[i].x = vert.x*m_d.m_length + m_d.m_vCenter.x;
      buf[i].y = vert.y*m_d.m_length + m_d.m_vCenter.y;
      buf[i].z = vert.z*m_d.m_length*m_ptable->m_BG_scalez[m_ptable->m_BG_current_set] + m_posZ;

      vert = Vertex3Ds(spinnerBracket[i].nx, spinnerBracket[i].ny, spinnerBracket[i].nz);
      vert = fullMatrix.MultiplyVectorNoTranslate(vert);
      buf[i].nx = vert.x;
      buf[i].ny = vert.y;
      buf[i].nz = vert.z;

      buf[i].tu = spinnerBracket[i].tu;
      buf[i].tv = spinnerBracket[i].tv;
   }
   bracketVertexBuffer->unlock();

   if (plateIndexBuffer)
      plateIndexBuffer->release();
   plateIndexBuffer = pd3dDevice->CreateAndFillIndexBuffer(spinnerPlateNumFaces, spinnerPlateIndices);

   if (plateVertexBuffer)
      plateVertexBuffer->release();
   pd3dDevice->CreateVertexBuffer(spinnerPlateNumVertices, USAGE_DYNAMIC, MY_D3DFVF_NOTEX2_VERTEX, &plateVertexBuffer);

   vertexBuffer_spinneranimangle = -FLT_MAX;
   UpdatePlate(pd3dDevice);
}

void Spinner::RenderStatic(RenderDevice* pd3dDevice)
{
   if (!m_d.m_fShowBracket || !m_d.m_fVisible)
      return;

   if (m_ptable->m_fReflectionEnabled && !m_d.m_fReflectionEnabled)
      return;

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
   pd3dDevice->DrawIndexedPrimitiveVB(D3DPT_TRIANGLELIST, MY_D3DFVF_NOTEX2_VERTEX, bracketVertexBuffer, 0, spinnerBracketNumVertices, bracketIndexBuffer, 0, spinnerBracketNumFaces);
   pd3dDevice->basicShader->End();
}

void Spinner::SetObjectPos()
{
   g_pvp->SetObjectPosCur(m_d.m_vCenter.x, m_d.m_vCenter.y);
}

void Spinner::MoveOffset(const float dx, const float dy)
{
   m_d.m_vCenter.x += dx;
   m_d.m_vCenter.y += dy;

   m_ptable->SetDirtyDraw();
}

void Spinner::GetCenter(Vertex2D * const pv) const
{
   *pv = m_d.m_vCenter;
}

void Spinner::PutCenter(const Vertex2D * const pv)
{
   m_d.m_vCenter = *pv;

   m_ptable->SetDirtyDraw();
}

void Spinner::SetDefaultPhysics(bool fromMouseClick)
{
   HRESULT hr;
   float fTmp;
   hr = GetRegStringAsFloat("DefaultProps\\Spinner", "Elasticity", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_elasticity = fTmp;
   else
      m_d.m_elasticity = 0.3f;
   hr = GetRegStringAsFloat("DefaultProps\\Spinner", "AntiFriction", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_damping = fTmp;
   else
      m_d.m_damping = 0.9879f;
}

HRESULT Spinner::SaveData(IStream *pstm, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
{
   BiffWriter bw(pstm, hcrypthash, hcryptkey);

   bw.WriteStruct(FID(VCEN), &m_d.m_vCenter, sizeof(Vertex2D));
   bw.WriteFloat(FID(ROTA), m_d.m_rotation);
   bw.WriteBool(FID(TMON), m_d.m_tdr.m_fTimerEnabled);
   bw.WriteInt(FID(TMIN), m_d.m_tdr.m_TimerInterval);
   bw.WriteFloat(FID(HIGH), m_d.m_height);
   bw.WriteFloat(FID(LGTH), m_d.m_length);
   bw.WriteFloat(FID(AFRC), m_d.m_damping);

   bw.WriteFloat(FID(SMAX), m_d.m_angleMax);
   bw.WriteFloat(FID(SMIN), m_d.m_angleMin);
   bw.WriteFloat(FID(SELA), m_d.m_elasticity);
   bw.WriteBool(FID(SVIS), m_d.m_fVisible);
   bw.WriteBool(FID(SSUPT), m_d.m_fShowBracket);
   bw.WriteString(FID(MATR), m_d.m_szMaterial);
   bw.WriteString(FID(IMGF), m_d.m_szImage);
   bw.WriteString(FID(SURF), m_d.m_szSurface);
   bw.WriteWideString(FID(NAME), (WCHAR *)m_wzName);

   ISelect::SaveData(pstm, hcrypthash, hcryptkey);

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

BOOL Spinner::LoadToken(int id, BiffReader *pbr)
{
   if (id == FID(PIID))
   {
      pbr->GetInt((int *)pbr->m_pdata);
   }
   else if (id == FID(VCEN))
   {
      pbr->GetStruct(&m_d.m_vCenter, sizeof(Vertex2D));
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
   else if (id == FID(TMIN))
   {
      pbr->GetInt(&m_d.m_tdr.m_TimerInterval);
   }
   else if (id == FID(SSUPT))
   {
      pbr->GetBool(&m_d.m_fShowBracket);
   }
   else if (id == FID(HIGH))
   {
      pbr->GetFloat(&m_d.m_height);
   }
   else if (id == FID(LGTH))
   {
      pbr->GetFloat(&m_d.m_length);
   }
   else if (id == FID(AFRC))
   {
      pbr->GetFloat(&m_d.m_damping);
   }
   else if (id == FID(SMAX))
   {
      pbr->GetFloat(&m_d.m_angleMax);
   }
   else if (id == FID(SMIN))
   {
      pbr->GetFloat(&m_d.m_angleMin);
   }
   else if (id == FID(SELA))
   {
      pbr->GetFloat(&m_d.m_elasticity);
   }
   else if (id == FID(SVIS))
   {
      pbr->GetBool(&m_d.m_fVisible);
   }
   else if (id == FID(IMGF))
   {
      pbr->GetString(m_d.m_szImage);
   }
   else if (id == FID(SURF))
   {
      pbr->GetString(m_d.m_szSurface);
   }
   else if (id == FID(NAME))
   {
      pbr->GetWideString((WCHAR *)m_wzName);
   }
   else
   {
      ISelect::LoadToken(id, pbr);
   }

   return fTrue;
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
   STARTUNDO

      m_d.m_length = newVal;

   STOPUNDO

      return S_OK;
}

STDMETHODIMP Spinner::get_Rotation(float *pVal)
{
   *pVal = m_d.m_rotation;

   return S_OK;
}

STDMETHODIMP Spinner::put_Rotation(float newVal)
{
   STARTUNDO

      m_d.m_rotation = newVal;

   STOPUNDO

      return S_OK;
}

STDMETHODIMP Spinner::get_Height(float *pVal)
{
   *pVal = m_d.m_height;

   return S_OK;
}

STDMETHODIMP Spinner::put_Height(float newVal)
{
   STARTUNDO

      m_d.m_height = newVal;

   STOPUNDO

      return S_OK;
}

STDMETHODIMP Spinner::get_Damping(float *pVal)
{
   *pVal = !g_pplayer ? m_d.m_damping : powf(m_phitspinner->m_spinneranim.m_damping,(float)(1.0/PHYS_FACTOR));

   return S_OK;
}

STDMETHODIMP Spinner::put_Damping(float newVal)
{
   const float tmp = clamp(newVal, 0.0f, 1.0f);
   if (g_pplayer)
      m_phitspinner->m_spinneranim.m_damping = powf(tmp, (float)PHYS_FACTOR);
   else
   {
      STARTUNDO

      m_d.m_damping = tmp;

      STOPUNDO
   }

   return S_OK;
}

STDMETHODIMP Spinner::get_Material(BSTR *pVal)
{
   WCHAR wz[512];

   MultiByteToWideChar(CP_ACP, 0, m_d.m_szMaterial, -1, wz, 32);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP Spinner::put_Material(BSTR newVal)
{
   STARTUNDO

      WideCharToMultiByte(CP_ACP, 0, newVal, -1, m_d.m_szMaterial, 32, NULL, NULL);

   STOPUNDO

      return S_OK;
}

STDMETHODIMP Spinner::get_Image(BSTR *pVal)
{
   WCHAR wz[512];

   MultiByteToWideChar(CP_ACP, 0, m_d.m_szImage, -1, wz, 32);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP Spinner::put_Image(BSTR newVal)
{
   char szImage[MAXTOKEN];
   WideCharToMultiByte(CP_ACP, 0, newVal, -1, szImage, 32, NULL, NULL);
   const Texture * const tex = m_ptable->GetImage(szImage);
   if(tex && tex->IsHDR())
   {
       ShowError("Cannot use a HDR image (.exr/.hdr) here");
       return E_FAIL;
   }

   STARTUNDO

   strcpy_s(m_d.m_szImage,szImage);

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Spinner::get_X(float *pVal)
{
   *pVal = m_d.m_vCenter.x;

   return S_OK;
}

STDMETHODIMP Spinner::put_X(float newVal)
{
   STARTUNDO

      m_d.m_vCenter.x = newVal;

   STOPUNDO

      return S_OK;
}

STDMETHODIMP Spinner::get_Y(float *pVal)
{
   *pVal = m_d.m_vCenter.y;

   return S_OK;
}

STDMETHODIMP Spinner::put_Y(float newVal)
{
   STARTUNDO

      m_d.m_vCenter.y = newVal;

   STOPUNDO

      return S_OK;
}

STDMETHODIMP Spinner::get_Surface(BSTR *pVal)
{
   WCHAR wz[512];

   MultiByteToWideChar(CP_ACP, 0, m_d.m_szSurface, -1, wz, 32);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP Spinner::put_Surface(BSTR newVal)
{
   STARTUNDO

      WideCharToMultiByte(CP_ACP, 0, newVal, -1, m_d.m_szSurface, 32, NULL, NULL);

   STOPUNDO

      return S_OK;
}

STDMETHODIMP Spinner::get_ShowBracket(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_d.m_fShowBracket);

   return S_OK;
}

STDMETHODIMP Spinner::put_ShowBracket(VARIANT_BOOL newVal)
{
   STARTUNDO

      m_d.m_fShowBracket = VBTOF(newVal);

   STOPUNDO

      return S_OK;
}


STDMETHODIMP Spinner::get_AngleMax(float *pVal)
{
   *pVal = (g_pplayer) ? RADTOANG(m_phitspinner->m_spinneranim.m_angleMax) :	//player active value
      m_d.m_angleMax;

   return S_OK;
}

STDMETHODIMP Spinner::put_AngleMax(float newVal)
{
   if (g_pplayer)
   {
      if (m_d.m_angleMin != m_d.m_angleMax)	// allow only if in limited angle mode
      {
         if (newVal > m_d.m_angleMax) newVal = m_d.m_angleMax;
         else if (newVal < m_d.m_angleMin) newVal = m_d.m_angleMin;

         newVal = ANGTORAD(newVal);

         if (m_phitspinner->m_spinneranim.m_angleMin < newVal)	// Min is smaller???
            m_phitspinner->m_spinneranim.m_angleMax = newVal;	//yes set new max
         else m_phitspinner->m_spinneranim.m_angleMin = newVal;	//no set new minumum
      }
      else return S_FAIL;
   }
   else
   {
      STARTUNDO
         m_d.m_angleMax = newVal;
      STOPUNDO
   }

   return S_OK;
}


STDMETHODIMP Spinner::get_AngleMin(float *pVal)
{
   *pVal = (g_pplayer) ? RADTOANG(m_phitspinner->m_spinneranim.m_angleMin) :	//player active value
      m_d.m_angleMin;

   return S_OK;
}

STDMETHODIMP Spinner::put_AngleMin(float newVal)
{
   if (g_pplayer)
   {
      if (m_d.m_angleMin != m_d.m_angleMax)	// allow only if in limited angle mode
      {
         if (newVal > m_d.m_angleMax) newVal = m_d.m_angleMax;
         else if (newVal < m_d.m_angleMin) newVal = m_d.m_angleMin;

         newVal = ANGTORAD(newVal);

         if (m_phitspinner->m_spinneranim.m_angleMax > newVal)	// max is bigger
            m_phitspinner->m_spinneranim.m_angleMin = newVal;	//then set new minumum
         else m_phitspinner->m_spinneranim.m_angleMax = newVal;	//else set new max
      }
      else return S_FAIL;
   }
   else
   {
      STARTUNDO
         m_d.m_angleMin = newVal;
      STOPUNDO
   }
   return S_OK;
}


STDMETHODIMP Spinner::get_Elasticity(float *pVal)
{
   *pVal = (g_pplayer) ? m_phitspinner->m_spinneranim.m_elasticity :	//player active value
      m_d.m_elasticity;

   return S_OK;
}

STDMETHODIMP Spinner::put_Elasticity(float newVal)
{
   if (g_pplayer)
   {
      m_phitspinner->m_spinneranim.m_elasticity = newVal;	//player active value
   }
   else
   {
      STARTUNDO
         m_d.m_elasticity = newVal;
      STOPUNDO
   }

   return S_OK;
}

STDMETHODIMP Spinner::get_Visible(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB((g_pplayer) ? m_phitspinner->m_spinneranim.m_fVisible : m_d.m_fVisible);

   return S_OK;
}


STDMETHODIMP Spinner::put_Visible(VARIANT_BOOL newVal)
{
   if (g_pplayer)
   {
      m_phitspinner->m_spinneranim.m_fVisible = VBTOF(newVal);// && m_d.m_fVisible;
   }
   else
   {
      STARTUNDO

         m_d.m_fVisible = VBTOF(newVal);

      STOPUNDO
   }

   return S_OK;
}

STDMETHODIMP Spinner::get_ReflectionEnabled(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_d.m_fReflectionEnabled);

   return S_OK;
}


STDMETHODIMP Spinner::put_ReflectionEnabled(VARIANT_BOOL newVal)
{
   STARTUNDO
      m_d.m_fReflectionEnabled = VBTOF(newVal);
   STOPUNDO

      return S_OK;
}

STDMETHODIMP Spinner::get_CurrentAngle(float *pVal)
{
   if (m_phitspinner)
   {
      *pVal = RADTOANG(m_phitspinner->m_spinneranim.m_angle);
      return S_OK;
   }
   else
      return E_FAIL;
}

void Spinner::GetDialogPanes(Vector<PropertyPane> *pvproppane)
{
   PropertyPane *pproppane;

   pproppane = new PropertyPane(IDD_PROP_NAME, NULL);
   pvproppane->AddElement(pproppane);

   pproppane = new PropertyPane(IDD_PROPSPINNER_VISUALS, IDS_VISUALS);
   pvproppane->AddElement(pproppane);

   pproppane = new PropertyPane(IDD_PROPSPINNER_POSITION, IDS_POSITION);
   pvproppane->AddElement(pproppane);

   pproppane = new PropertyPane(IDD_PROPSPINNER_PHYSICS, IDS_PHYSICS);
   pvproppane->AddElement(pproppane);

   pproppane = new PropertyPane(IDD_PROP_TIMER, IDS_MISC);
   pvproppane->AddElement(pproppane);
}
