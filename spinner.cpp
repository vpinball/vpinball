#include "StdAfx.h"
#include "meshes/spinnerBracketMesh.h"
#include "meshes/spinnerPlateMesh.h"

Spinner::Spinner()
{
   m_phitspinner = NULL;
   bracketVertexBuffer = 0;
   bracketIndexBuffer = 0;
   plateVertexBuffer = 0;
   plateIndexBuffer = 0;
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
   SetRegValueFloat("DefaultProps\\Spinner","Length", m_d.m_length);
   SetRegValueFloat("DefaultProps\\Spinner","Rotation", m_d.m_rotation);
   SetRegValueBool("DefaultProps\\Spinner","ShowBracket", m_d.m_fShowBracket);
   SetRegValueFloat("DefaultProps\\Spinner","Height", m_d.m_height);
   SetRegValueFloat("DefaultProps\\Spinner","Overhang", m_d.m_overhang);
   SetRegValue("DefaultProps\\Spinner","CastsShadow",REG_DWORD,&m_d.m_fCastsShadow,4);
   SetRegValueFloat("DefaultProps\\Spinner","AngleMax", m_d.m_angleMax);
   SetRegValueFloat("DefaultProps\\Spinner","AngleMin", m_d.m_angleMin);
   SetRegValueFloat("DefaultProps\\Spinner","Elasticity", m_d.m_elasticity);
   //SetRegValueFloat("DefaultProps\\Spinner","Friction", m_d.m_friction);
   SetRegValueFloat("DefaultProps\\Spinner","Scatter", m_d.m_scatter);
   SetRegValue("DefaultProps\\Spinner","Visible",REG_DWORD,&m_d.m_fVisible,4);
   SetRegValue("DefaultProps\\Spinner","TimerEnabled",REG_DWORD,&m_d.m_tdr.m_fTimerEnabled,4);
   SetRegValue("DefaultProps\\Spinner","TimerInterval", REG_DWORD, &m_d.m_tdr.m_TimerInterval, 4);
   SetRegValue("DefaultProps\\Spinner","Image", REG_SZ, &m_d.m_szImage,lstrlen(m_d.m_szImage));
   SetRegValue("DefaultProps\\Spinner","Surface", REG_SZ, &m_d.m_szSurface,lstrlen(m_d.m_szSurface));
}

void Spinner::SetDefaults(bool fromMouseClick)
{
   HRESULT hr;
   int iTmp;
   float fTmp;

   hr = GetRegStringAsFloat("DefaultProps\\Spinner","Length", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_length = fTmp;
   else
      m_d.m_length = 80;

   hr = GetRegStringAsFloat("DefaultProps\\Spinner","Rotation", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_rotation = fTmp;
   else
      m_d.m_rotation = 0;

   hr = GetRegInt("DefaultProps\\Spinner","ShowBracket", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_fShowBracket = iTmp == 0 ? false : true;
   else
      m_d.m_fShowBracket = true;

   hr = GetRegInt("DefaultProps\\Spinner","Height", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_height = (float)iTmp/1000.0f;
   else
      m_d.m_height = 60;

   hr = GetRegStringAsFloat("DefaultProps\\Spinner","Overhang", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_overhang = fTmp;
   else
      m_d.m_overhang = 10;

   hr = GetRegInt("DefaultProps\\Spinner","CastsShadow", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_fCastsShadow = iTmp == 0 ? false : true;
   else
      m_d.m_fCastsShadow = true;			//<<< added by Chris

   // Anti-friction is 1-friction (throughput)
   m_d.m_antifriction = 0.99f;

   hr = GetRegStringAsFloat("DefaultProps\\Spinner","AngleMax", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_angleMax = fTmp;
   else
      m_d.m_angleMax = 0;

   hr = GetRegStringAsFloat("DefaultProps\\Spinner","AngleMin", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_angleMin = fTmp;
   else
      m_d.m_angleMin = 0;

   hr = GetRegStringAsFloat("DefaultProps\\Spinner","Elasticity", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_elasticity = fTmp;
   else
      m_d.m_elasticity = 0.3f;

   hr = GetRegInt("DefaultProps\\Spinner","Visible", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_fVisible = iTmp == 0 ? false : true;
   else
      m_d.m_fVisible = true;

   hr = GetRegInt("DefaultProps\\Spinner","TimerEnabled", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_tdr.m_fTimerEnabled = iTmp == 0 ? false : true;
   else
      m_d.m_tdr.m_fTimerEnabled = fFalse;

   hr = GetRegInt("DefaultProps\\Spinner","TimerInterval", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_tdr.m_TimerInterval = iTmp;
   else
      m_d.m_tdr.m_TimerInterval = 100;

   hr = GetRegString("DefaultProps\\Spinner","Image", m_d.m_szImage, MAXTOKEN);
   if ((hr != S_OK) || !fromMouseClick)
      m_d.m_szImage[0] = 0;

   hr = GetRegString("DefaultProps\\Spinner","Surface", &m_d.m_szSurface, MAXTOKEN);
   if ((hr != S_OK) || !fromMouseClick)
      m_d.m_szSurface[0] = 0;
}

void Spinner::PreRender(Sur * const psur)
{	
}

void Spinner::Render(Sur * const psur)
{
   psur->SetLineColor(RGB(0,0,0),false,3);
   psur->SetObject(this);

   float halflength = m_d.m_length * 0.5f;

   const float radangle = ANGTORAD(m_d.m_rotation);
   const float sn = sinf(radangle);
   const float cs = cosf(radangle);

   psur->Line(m_d.m_vCenter.x + cs*halflength, m_d.m_vCenter.y + sn*halflength,
      m_d.m_vCenter.x - cs*halflength, m_d.m_vCenter.y - sn*halflength);

   psur->SetLineColor(RGB(0,0,0),false,1);
   psur->SetObject(this);

   halflength += m_d.m_overhang;

   psur->Line(m_d.m_vCenter.x + cs*halflength, m_d.m_vCenter.y + sn*halflength,
      m_d.m_vCenter.x - cs*halflength, m_d.m_vCenter.y - sn*halflength);
}

void Spinner::RenderShadow(ShadowSur * const psur, const float height)
{
   if ( (!m_d.m_fCastsShadow) || (!m_ptable->m_fRenderShadows) )
      return;

   psur->SetLineColor(RGB(0,0,0),false,4);
   psur->SetObject(this);

   float halflength = m_d.m_length * 0.5f;

   const float radangle = m_d.m_rotation * (float)(M_PI*2.0/360.0);
   const float sn = sinf(radangle);
   const float cs = cosf(radangle);

   psur->Line(m_d.m_vCenter.x + cs*halflength, m_d.m_vCenter.y + sn*halflength,
      m_d.m_vCenter.x - cs*halflength, m_d.m_vCenter.y - sn*halflength);

   psur->SetLineColor(RGB(0,0,0),false,1);
   psur->SetObject(this);

   halflength += m_d.m_overhang;

   psur->Line(m_d.m_vCenter.x + cs*halflength, m_d.m_vCenter.y + sn*halflength,
      m_d.m_vCenter.x - cs*halflength, m_d.m_vCenter.y - sn*halflength);
}

void Spinner::GetTimers(Vector<HitTimer> * const pvht)
{
   IEditable::BeginPlay();

   HitTimer * const pht = new HitTimer();
   pht->m_interval = m_d.m_tdr.m_TimerInterval;
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
   const float h = m_d.m_height*0.5f + 30.0f;

   const float angleMin = min(m_d.m_angleMin, m_d.m_angleMax); // correct angle inversions
   const float angleMax = max(m_d.m_angleMin, m_d.m_angleMax);

   m_d.m_angleMin = angleMin;	
   m_d.m_angleMax = angleMax;

   HitSpinner * const phitspinner = new HitSpinner(this, height);
   m_phitspinner = phitspinner;

   pvho->AddElement(phitspinner);

   if(m_d.m_fShowBracket)
   {
      float halflength = m_d.m_length * 0.5f;
      const float radangle = ANGTORAD(m_d.m_rotation);
      const float sn = sinf(radangle);
      const float cs = cosf(radangle);

      halflength += m_d.m_overhang;

      HitCircle *phitcircle;
      phitcircle = new HitCircle();
      phitcircle->m_pfe = NULL;
      phitcircle->center.x = m_d.m_vCenter.x + cs*halflength;
      phitcircle->center.y = m_d.m_vCenter.y + sn*halflength;
      phitcircle->radius = 0.01f;
      phitcircle->zlow = height;
      phitcircle->zhigh = height+h;//+50.0f;
      pvho->AddElement(phitcircle);

      phitcircle = new HitCircle();
      phitcircle->m_pfe = NULL;
      phitcircle->center.x = m_d.m_vCenter.x - cs*halflength;
      phitcircle->center.y = m_d.m_vCenter.y - sn*halflength;
      phitcircle->radius = 0.01f;
      phitcircle->zlow = height;
      phitcircle->zhigh = height+h; //+50.0f;
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

void Spinner::UpdatePlate( RenderDevice *pd3dDevice )
{
    Matrix3D fullMatrix;
    Matrix3D rotzMat,rotxMat;
    Vertex3D_NoTex2 *buf;

    fullMatrix.SetIdentity();
    rotxMat.RotateXMatrix(-m_phitspinner->m_spinneranim.m_angle);
    rotxMat.Multiply(fullMatrix, fullMatrix);
    rotzMat.RotateZMatrix(ANGTORAD(m_d.m_rotation));
    rotzMat.Multiply(fullMatrix, fullMatrix);

    plateVertexBuffer->lock(0, 0, (void**)&buf, 0);
    for( int i=0;i<spinnerPlateNumVertices;i++ )
    {
        Vertex3Ds vert(spinnerPlate[i].x,spinnerPlate[i].y,spinnerPlate[i].z);
        vert = fullMatrix.MultiplyVector(vert);

        buf[i].x = (vert.x*m_d.m_length)+m_d.m_vCenter.x;
        buf[i].y = (vert.y*m_d.m_length)+m_d.m_vCenter.y;
        buf[i].z = (vert.z*m_d.m_length*m_ptable->m_zScale);
        buf[i].z += m_posZ;
        vert = Vertex3Ds( spinnerPlate[i].nx, spinnerPlate[i].ny, spinnerPlate[i].nz );
        vert = fullMatrix.MultiplyVectorNoTranslate(vert);
        buf[i].nx = vert.x;
        buf[i].ny = vert.y;
        buf[i].nz = vert.z;
        buf[i].tu = spinnerPlate[i].tu;
        buf[i].tv = spinnerPlate[i].tv;
    }
    plateVertexBuffer->unlock();

}

void Spinner::PostRenderStatic(RenderDevice* pd3dDevice)
{

    TRACE_FUNCTION();
    if (!m_phitspinner->m_spinneranim.m_fVisible || !m_d.m_fVisible)
        return;

    Pin3D * const ppin3d = &g_pplayer->m_pin3d;
    pd3dDevice->SetVertexDeclaration( pd3dDevice->m_pVertexNormalTexelDeclaration );

    Material *mat = m_ptable->GetMaterial( m_d.m_szMaterial);
    pd3dDevice->basicShader->SetMaterial(mat);

    Texture * const image = m_ptable->GetImage(m_d.m_szImage);

    pd3dDevice->basicShader->Core()->SetBool("bPerformAlphaTest", true);
    pd3dDevice->basicShader->Core()->SetFloat("fAlphaTestValue", 128.0f/255.0f);

    UpdatePlate(pd3dDevice);
    if ( image )
    {
        image->CreateAlphaChannel();
        g_pplayer->m_pin3d.EnableAlphaBlend(1, false);
        pd3dDevice->basicShader->SetTexture("Texture0",image);
        pd3dDevice->basicShader->Core()->SetTechnique("basic_with_texture");
    }
    else // No image by that name
    {
        pd3dDevice->basicShader->Core()->SetTechnique("basic_without_texture");
    }
    pd3dDevice->basicShader->Begin(0);
    pd3dDevice->DrawIndexedPrimitiveVB( D3DPT_TRIANGLELIST, plateVertexBuffer, 0, spinnerPlateNumVertices, plateIndexBuffer, 0, spinnerPlateNumFaces);
    pd3dDevice->basicShader->End();

//    g_pplayer->UpdateBasicShaderMatrix();

    pd3dDevice->SetRenderState(RenderDevice::CULLMODE, D3DCULL_CCW);
    pd3dDevice->SetRenderState(RenderDevice::ALPHATESTENABLE, FALSE);
    pd3dDevice->basicShader->Core()->SetBool("bPerformAlphaTest", false);

}


void Spinner::RenderSetup(RenderDevice* pd3dDevice)
{
   if ( !m_d.m_fVisible )
    return;

   Texture* const image = m_ptable->GetImage(m_d.m_szImage);

   if( image )
   {
      image->CreateAlphaChannel();
   }
   const float height = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y);
   std::vector<WORD> indices(spinnerBracketNumFaces);
   Vertex3D_NoTex2 *buf;
   for( int i=0;i<spinnerBracketNumFaces;i++ ) indices[i] = spinnerBracketIndices[i];

   if (bracketIndexBuffer)
       bracketIndexBuffer->release();
   bracketIndexBuffer = pd3dDevice->CreateAndFillIndexBuffer( indices );

   if (!bracketVertexBuffer)
       pd3dDevice->CreateVertexBuffer(spinnerBracketNumVertices, 0, MY_D3DFVF_NOTEX2_VERTEX, &bracketVertexBuffer);

   fullMatrix.RotateZMatrix(ANGTORAD(m_d.m_rotation));

   bracketVertexBuffer->lock(0, 0, (void**)&buf, 0);
   for( int i=0;i<spinnerBracketNumVertices;i++ )
   {
       Vertex3Ds vert(spinnerBracket[i].x,spinnerBracket[i].y,spinnerBracket[i].z);
       vert = fullMatrix.MultiplyVector(vert);

       buf[i].x = (vert.x*m_d.m_length)+m_d.m_vCenter.x;
       buf[i].y = (vert.y*m_d.m_length)+m_d.m_vCenter.y;
       buf[i].z = (vert.z*m_d.m_length*m_ptable->m_zScale);
       buf[i].z += (height+m_d.m_height);
       vert = Vertex3Ds( spinnerBracket[i].nx, spinnerBracket[i].ny, spinnerBracket[i].nz );
       vert = fullMatrix.MultiplyVectorNoTranslate(vert);
       buf[i].nx = vert.x;
       buf[i].ny = vert.y;
       buf[i].nz = vert.z;
       buf[i].tu = spinnerBracket[i].tu;
       buf[i].tv = spinnerBracket[i].tv;
   }
   bracketVertexBuffer->unlock();

   indices.clear();
   indices.resize(spinnerPlateNumFaces);
   for( int i=0;i<spinnerPlateNumFaces;i++ ) indices[i] = spinnerPlateIndices[i];

   if (plateIndexBuffer)
       plateIndexBuffer->release();
   plateIndexBuffer = pd3dDevice->CreateAndFillIndexBuffer( indices );

   if (!plateVertexBuffer)
       pd3dDevice->CreateVertexBuffer(spinnerBracketNumVertices, 0, MY_D3DFVF_NOTEX2_VERTEX, &plateVertexBuffer);

   plateVertexBuffer->lock(0, 0, (void**)&buf, 0);
   for( int i=0;i<spinnerPlateNumVertices;i++ )
   {
       Vertex3Ds vert(spinnerPlate[i].x, spinnerPlate[i].y, spinnerPlate[i].z);
       vert = fullMatrix.MultiplyVector(vert);

       buf[i].x = (vert.x*m_d.m_length);//+m_d.m_vCenter.x;
       buf[i].y = (vert.y*m_d.m_length);//+m_d.m_vCenter.y;
       buf[i].z = (vert.z*m_d.m_length*m_ptable->m_zScale);
       //buf[i].z += (height+m_d.m_height);
       m_posZ = height+m_d.m_height;

       vert = Vertex3Ds( spinnerPlate[i].nx, spinnerPlate[i].ny, spinnerPlate[i].nz );
       vert = fullMatrix.MultiplyVectorNoTranslate(vert);
       buf[i].nx = vert.x;
       buf[i].ny = vert.y;
       buf[i].nz = vert.z;
       buf[i].tu = spinnerPlate[i].tu;
       buf[i].tv = spinnerPlate[i].tv;
   }
   plateVertexBuffer->unlock();
}

void Spinner::RenderStatic(RenderDevice* pd3dDevice)
{
   if(!m_d.m_fShowBracket || !m_d.m_fVisible) return;

   Pin3D * const ppin3d = &g_pplayer->m_pin3d;
   pd3dDevice->SetVertexDeclaration( pd3dDevice->m_pVertexNormalTexelDeclaration );

   Material mat;
   mat.m_bIsMetal = true;
   mat.m_cBase = 0x20202020;
   mat.m_fWrapLighting = 0.9f;
   mat.m_cGlossy = 0x60606060;
   mat.m_fRoughness = 0.4f;
   mat.m_cClearcoat = 0x20202020;
   mat.m_fEdge=1.0f;
   pd3dDevice->basicShader->SetMaterial(&mat);
   pd3dDevice->basicShader->Core()->SetTechnique("basic_without_texture");
   ppin3d->EnableAlphaBlend(1,false);

   pd3dDevice->basicShader->Begin(0);
   pd3dDevice->DrawIndexedPrimitiveVB( D3DPT_TRIANGLELIST, bracketVertexBuffer, 0, spinnerBracketNumVertices, bracketIndexBuffer, 0, spinnerBracketNumFaces );
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

HRESULT Spinner::SaveData(IStream *pstm, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
{
   BiffWriter bw(pstm, hcrypthash, hcryptkey);

   bw.WriteStruct(FID(VCEN), &m_d.m_vCenter, sizeof(Vertex2D));
   bw.WriteFloat(FID(ROTA), m_d.m_rotation);
   bw.WriteBool(FID(TMON), m_d.m_tdr.m_fTimerEnabled);
   bw.WriteInt(FID(TMIN), m_d.m_tdr.m_TimerInterval);
   bw.WriteFloat(FID(HIGH), m_d.m_height);
   bw.WriteFloat(FID(LGTH), m_d.m_length);
   bw.WriteFloat(FID(AFRC), m_d.m_antifriction);

   bw.WriteFloat(FID(SMAX), m_d.m_angleMax);
   bw.WriteFloat(FID(SMIN), m_d.m_angleMin);
   bw.WriteFloat(FID(SELA), m_d.m_elasticity);
   bw.WriteBool(FID(SVIS), m_d.m_fVisible);
   bw.WriteBool(FID(SSUPT), m_d.m_fShowBracket);
   bw.WriteFloat(FID(OVRH), m_d.m_overhang);
   bw.WriteString(FID(MATR), m_d.m_szMaterial);
   bw.WriteBool(FID(CSHD), m_d.m_fCastsShadow);	//<<< added by Chris
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
      pbr->GetFloat(&m_d.m_antifriction);
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
   else if (id == FID(OVRH))
   {
      pbr->GetFloat(&m_d.m_overhang);
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
   else if (id == FID(CSHD))
   {
      pbr->GetBool(&m_d.m_fCastsShadow);
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

   for (int i=0;i<sizeof(arr)/sizeof(arr[0]);i++)
   {
      if (InlineIsEqualGUID(*arr[i],riid))
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

STDMETHODIMP Spinner::get_Overhang(float *pVal)
{
   *pVal = m_d.m_overhang;

   return S_OK;
}

STDMETHODIMP Spinner::put_Overhang(float newVal)
{
   STARTUNDO

      m_d.m_overhang = newVal;

   STOPUNDO

      return S_OK;
}

STDMETHODIMP Spinner::get_Friction(float *pVal)
{
   *pVal = (1.0f-m_d.m_antifriction)*100.0f;

   return S_OK;
}

STDMETHODIMP Spinner::put_Friction(float newVal)
{
   STARTUNDO

      m_d.m_antifriction = clamp(1.0f - newVal*(float)(1.0/100.0), 0.0f, 1.0f);

   STOPUNDO

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
   STARTUNDO

      WideCharToMultiByte(CP_ACP, 0, newVal, -1, m_d.m_szImage, 32, NULL, NULL);

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

//>>> added by Chris
STDMETHODIMP Spinner::get_CastsShadow(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_d.m_fCastsShadow);

   return S_OK;
}

STDMETHODIMP Spinner::put_CastsShadow(VARIANT_BOOL newVal)
{
   STARTUNDO

      m_d.m_fCastsShadow = VBTOF(newVal);

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
