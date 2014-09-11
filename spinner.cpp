#include "StdAfx.h"

Spinner::Spinner()
{
   m_phitspinner = NULL;
   staticMaterial.setDiffuse( 0.0f, 0.6f, 0.6f, 0.6f );
   staticMaterial.setAmbient( 0.0f, 0.6f, 0.6f, 0.6f );
   staticMaterial.setSpecular( 0.0f, 0.0f, 0.0f, 0.0f );
   staticMaterial.setEmissive( 0.0f, 0.0f, 0.0f, 0.0f );
   staticMaterial.setPower( 0.0f );

   vtxBuf = 0;
   idxBuf = 0;
}

Spinner::~Spinner()
{
   if (vtxBuf)
   {
       vtxBuf->release();
       vtxBuf = 0;
   }
   if (idxBuf)
   {
       idxBuf->release();
       idxBuf = 0;
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
   SetRegValue("DefaultProps\\Spinner","Supports",REG_DWORD,&m_d.m_fSupports,4);
   SetRegValueFloat("DefaultProps\\Spinner","Height", m_d.m_height);
   SetRegValueFloat("DefaultProps\\Spinner","Overhang", m_d.m_overhang);
   SetRegValue("DefaultProps\\Spinner","Color", REG_DWORD, &m_d.m_color, 4);
   SetRegValue("DefaultProps\\Spinner","CastsShadow",REG_DWORD,&m_d.m_fCastsShadow,4);
   SetRegValueFloat("DefaultProps\\Spinner","AngleMax", m_d.m_angleMax);
   SetRegValueFloat("DefaultProps\\Spinner","AngleMin", m_d.m_angleMin);
   SetRegValueFloat("DefaultProps\\Spinner","Elasticity", m_d.m_elasticity);
   //SetRegValueFloat("DefaultProps\\Spinner","Friction", m_d.m_friction);
   SetRegValueFloat("DefaultProps\\Spinner","Scatter", m_d.m_scatter);
   SetRegValue("DefaultProps\\Spinner","Visible",REG_DWORD,&m_d.m_fVisible,4);
   SetRegValue("DefaultProps\\Spinner","TimerEnabled",REG_DWORD,&m_d.m_tdr.m_fTimerEnabled,4);
   SetRegValue("DefaultProps\\Spinner","TimerInterval", REG_DWORD, &m_d.m_tdr.m_TimerInterval, 4);
   SetRegValue("DefaultProps\\Spinner","ImageFront", REG_SZ, &m_d.m_szImageFront,lstrlen(m_d.m_szImageFront));
   SetRegValue("DefaultProps\\Spinner","ImageBack", REG_SZ, &m_d.m_szImageBack,lstrlen(m_d.m_szImageBack));
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

   hr = GetRegInt("DefaultProps\\Spinner","Supports", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_fSupports = iTmp == 0 ? false : true;
   else
      m_d.m_fSupports = fTrue;

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

   hr = GetRegInt("DefaultProps\\Spinner","Color", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_color = iTmp;
   else
      m_d.m_color = RGB(50,200,50);

   hr = GetRegInt("DefaultProps\\Spinner","CastsShadow", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_fCastsShadow = iTmp == 0 ? false : true;
   else
      m_d.m_fCastsShadow = fTrue;			//<<< added by Chris

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

   //hr = GetRegStringAsFloat("DefaultProps\\Spinner","Friction", &fTmp);
   //if ((hr == S_OK) && fromMouseClick)
   //   m_d.m_friction = fTmp;
   //else
   //   m_d.m_friction = 0;	//zero uses global value

   hr = GetRegInt("DefaultProps\\Spinner","Visible", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_fVisible = iTmp == 0 ? false : true;
   else
      m_d.m_fVisible = fTrue;

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

   hr = GetRegString("DefaultProps\\Spinner","ImageFront", m_d.m_szImageFront, MAXTOKEN);
   if ((hr != S_OK) || !fromMouseClick)
      m_d.m_szImageFront[0] = 0;

   hr = GetRegString("DefaultProps\\Spinner","ImageBack", m_d.m_szImageBack, MAXTOKEN);
   if ((hr != S_OK) || !fromMouseClick)
      m_d.m_szImageBack[0] = 0;

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

   if(m_d.m_fSupports)
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

   if (vtxBuf)
   {
       vtxBuf->release();
       vtxBuf = 0;
   }
   if (idxBuf)
   {
       idxBuf->release();
       idxBuf = 0;
   }
}

static const WORD rgiSpinner0[8] = {0,1,2,3,6,7,4,5};
static const WORD rgiSpinner1[8] = {4,5,6,7,2,3,0,1};
static const WORD rgiSpinnerNormal[3] = {0,1,3};

static const WORD rgiSpinner2[4] = {0,1,5,4};      // back
static const WORD rgiSpinner3[4] = {2,6,7,3};      // front
static const WORD rgiSpinner4[4] = {0,2,3,1};      // bottom
static const WORD rgiSpinner5[4] = {4,5,7,6};      // top
static const WORD rgiSpinner6[4] = {0,4,6,2};      // left
static const WORD rgiSpinner7[4] = {1,3,7,5};      // right

void Spinner::PostRenderStatic(RenderDevice* pd3dDevice)
{
    TRACE_FUNCTION();
    if (!m_phitspinner->m_spinneranim.m_fVisible)
        return;

    Pin3D * const ppin3d = &g_pplayer->m_pin3d;

    pd3dDevice->SetVertexDeclaration( pd3dDevice->m_pVertexNormalTexelTexelDeclaration );

    const float r = (float)(m_d.m_color & 255) * (float)(1.0/255.0);
    const float g = (float)(m_d.m_color & 65280) * (float)(1.0/65280.0);
    const float b = (float)(m_d.m_color & 16711680) * (float)(1.0/16711680.0);
    D3DXVECTOR4 matColor(r,g,b,1.0f);   
    pd3dDevice->basicShader->Core()->SetFloat("fGlossyPower",0.0f);
    pd3dDevice->basicShader->Core()->SetVector("vDiffuseColor",&matColor);

    COLORREF rgbTransparent = RGB(255,0,255); //RGB(0,0,0);

    Texture * const pinback = m_ptable->GetImage(m_d.m_szImageBack);
    Texture * const pinfront = m_ptable->GetImage(m_d.m_szImageFront);

    g_pplayer->m_pin3d.EnableAlphaTestReference(0x80);

    // Set texture to mirror, so the alpha state of the texture blends correctly to the outside
    pd3dDevice->SetTextureAddressMode(ePictureTexture, RenderDevice::TEX_MIRROR);

    // set world transform
    Matrix3D matOrig, matNew, matTemp;
    pd3dDevice->GetTransform(TRANSFORMSTATE_WORLD, &matOrig);

    matTemp.SetTranslation(m_d.m_vCenter.x, m_d.m_vCenter.y, m_posZ);
    matOrig.Multiply(matTemp, matNew);

    matTemp.RotateZMatrix(ANGTORAD(m_d.m_rotation));
    matNew.Multiply(matTemp, matNew);

    matTemp.RotateXMatrix(-m_phitspinner->m_spinneranim.m_angle);
    matNew.Multiply(matTemp, matNew);

    pd3dDevice->SetTransform(TRANSFORMSTATE_WORLD, &matNew);
    g_pplayer->UpdateBasicShaderMatrix();

    // Draw Backside
    if (pinback)
    {
        pinback->Set( ePictureTexture );
        if (pinback->m_fTransparent)
        {
            pd3dDevice->SetRenderState(RenderDevice::ALPHABLENDENABLE, FALSE);
            if (m_d.m_color != rgbTransparent) rgbTransparent = pinback->m_rgbTransparent;
        }
        else 
        {
            g_pplayer->m_pin3d.EnableAlphaBlend( 1, fFalse );
        } 

        if (m_d.m_color == rgbTransparent || m_d.m_color == NOTRANSCOLOR) 
            pd3dDevice->SetRenderState(RenderDevice::CULLMODE, D3DCULL_CCW);
        else
            pd3dDevice->SetRenderState(RenderDevice::CULLMODE, D3DCULL_NONE);

        g_pplayer->m_pin3d.SetTextureFilter ( ePictureTexture, TEXTURE_MODE_TRILINEAR );
//        pd3dDevice->SetMaterial(textureMaterial);
        pd3dDevice->basicShader->SetTexture("Texture0",pinback);
        pd3dDevice->basicShader->Core()->SetTechnique("basic_with_texture");
    }
    else // No image by that name
    {
        //ppin3d->SetTexture(NULL);
        //pd3dDevice->SetMaterial(solidMaterial);
       pd3dDevice->basicShader->Core()->SetTechnique("basic_without_texture");
    }

    pd3dDevice->basicShader->Begin(0);
    pd3dDevice->DrawPrimitiveVB(D3DPT_TRIANGLEFAN, vtxBuf, 0, 4);
    pd3dDevice->basicShader->End();

    // Draw Frontside
    if (pinfront)
    {
        //pinfront->Set( ePictureTexture );
        if (pinfront->m_fTransparent)
        {
            pd3dDevice->SetRenderState(RenderDevice::ALPHABLENDENABLE, FALSE);	
            if (m_d.m_color != rgbTransparent) rgbTransparent = pinfront->m_rgbTransparent;
        }
        else 
        {
            g_pplayer->m_pin3d.EnableAlphaBlend( 1, fFalse );
        }

        if (m_d.m_color == rgbTransparent || m_d.m_color == NOTRANSCOLOR) 
            pd3dDevice->SetRenderState(RenderDevice::CULLMODE, D3DCULL_CCW);
        else
            pd3dDevice->SetRenderState(RenderDevice::CULLMODE, D3DCULL_NONE);

        g_pplayer->m_pin3d.SetTextureFilter ( ePictureTexture, TEXTURE_MODE_TRILINEAR );
        pd3dDevice->basicShader->SetTexture("Texture0",pinfront);
        pd3dDevice->basicShader->Core()->SetTechnique("basic_with_texture");
    }
    else // No image by that name
    {
//         ppin3d->SetTexture(NULL);
//         pd3dDevice->SetMaterial(solidMaterial);
        pd3dDevice->basicShader->Core()->SetTechnique("basic_without_texture");
    }

    pd3dDevice->basicShader->Begin(0);
    pd3dDevice->DrawPrimitiveVB(D3DPT_TRIANGLEFAN, vtxBuf, 4, 4);
    pd3dDevice->basicShader->End();

    if (m_d.m_color != rgbTransparent && m_d.m_color != NOTRANSCOLOR)
    {
//        ppin3d->SetTexture(NULL);
        pd3dDevice->basicShader->Core()->SetTechnique("basic_without_texture");
        pd3dDevice->basicShader->Begin(0);
        pd3dDevice->DrawIndexedPrimitiveVB(D3DPT_TRIANGLELIST, vtxBuf, 8, 16, idxBuf, 0, 24);
        pd3dDevice->basicShader->End();
    }

    pd3dDevice->SetTransform(TRANSFORMSTATE_WORLD, &matOrig);
    g_pplayer->UpdateBasicShaderMatrix();

    pd3dDevice->SetRenderState(RenderDevice::CULLMODE, D3DCULL_CCW);
    pd3dDevice->SetRenderState(RenderDevice::ALPHATESTENABLE, FALSE);
    pd3dDevice->SetTextureAddressMode(ePictureTexture, RenderDevice::TEX_WRAP);
}

void Spinner::PrepareStatic( RenderDevice* pd3dDevice )
{
   if(!m_d.m_fSupports) return;

   const float height = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y);

   Pin3D * const ppin3d = &g_pplayer->m_pin3d;

   const float halflength = m_d.m_length * 0.5f + m_d.m_overhang;
   const float halfthick = 2.0f;
   const float h = m_d.m_height*0.5f + 30.0f;

   staticVertices[0].x = -halflength + halfthick;
   staticVertices[0].y = 0;
   staticVertices[0].z = 0;

   staticVertices[1].x = -halflength - halfthick;
   staticVertices[1].y = 0;
   staticVertices[1].z = 0;

   staticVertices[2].x = -halflength + halfthick;
   staticVertices[2].y = 0;
   //rgv3D[2].z = 60.0f - halfthick;
   staticVertices[2].z = h - halfthick;

   staticVertices[3].x = -halflength - halfthick;
   staticVertices[3].y = 0;
   //rgv3D[3].z = 60.0f + halfthick;
   staticVertices[3].z = h + halfthick;

   staticVertices[4].x = halflength - halfthick;
   staticVertices[4].y = 0;
   staticVertices[4].z = 0;

   staticVertices[5].x = halflength + halfthick;
   staticVertices[5].y = 0;
   staticVertices[5].z = 0;

   staticVertices[6].x = halflength - halfthick;
   staticVertices[6].y = 0;
   //rgv3D[6].z = 60.0f - halfthick;
   staticVertices[6].z = h - halfthick;

   staticVertices[7].x = halflength + halfthick;
   staticVertices[7].y = 0;
   //rgv3D[7].z = 60.0f + halfthick;
   staticVertices[7].z = h + halfthick;

   const float radangle = ANGTORAD(m_d.m_rotation);
   const float snY = sinf(radangle);
   const float csY = cosf(radangle);

   for (int l=0;l<8;l++)
   {
      const float temp = staticVertices[l].x;
      staticVertices[l].x = csY*temp - snY*staticVertices[l].y;
      staticVertices[l].y = csY*staticVertices[l].y + snY*temp;

      staticVertices[l].x += m_d.m_vCenter.x;
      staticVertices[l].y += m_d.m_vCenter.y;
      staticVertices[l].z += height;
      staticVertices[l].z *= m_ptable->m_zScale;
   }
   ppin3d->CalcShadowCoordinates(staticVertices,8);
}

void Spinner::PrepareMovers( RenderDevice* pd3dDevice )
{
   Pin3D * const ppin3d = &g_pplayer->m_pin3d;

   COLORREF rgbTransparent = RGB(255,0,255); //RGB(0,0,0);

   const float height = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y);
   const float h = (m_d.m_height*0.5f + 30.0f);
   m_posZ = (h + height) * m_ptable->m_zScale;

   const float halflength = m_d.m_length * 0.5f;
   const float halfwidth = m_d.m_height * 0.5f;

   const float r = (float)(m_d.m_color & 255) * (float)(1.0/255.0);
   const float g = (float)(m_d.m_color & 65280) * (float)(1.0/65280.0);
   const float b = (float)(m_d.m_color & 16711680) * (float)(1.0/16711680.0);

   const float minx = -halflength;
   const float maxx = halflength;
   const float miny = -3.0f;
   const float maxy = 3.0f;
   const float minz = -halfwidth;
   const float maxz = halfwidth;

   Vertex3D moverVertices[8];

   for (int l=0;l<8;l++)
   {
       moverVertices[l].x = (l & 1) ? maxx : minx;
       moverVertices[l].y = (l & 2) ? maxy : miny;
       moverVertices[l].z = (l & 4) ? maxz : minz;

       if (l & 2)
       {
           moverVertices[l].tu = (l & 1) ? 1.0f : 0.f;
           moverVertices[l].tv = (l & 4) ? 0.f : 1.0f;
       }
       else
       {
           moverVertices[l].tu = (l & 1) ? 1.0f : 0.f;
           moverVertices[l].tv = (l & 4) ? 1.0f : 0.f;
       }
   }

   ppin3d->CalcShadowCoordinates(moverVertices, 8);

   std::vector< Vertex3D > vbVerts;
   vbVerts.reserve(6*4);

   SetNormal(moverVertices, rgiSpinner2, 4);        // back
   for (int i = 0; i < 4; ++i)
       vbVerts.push_back( moverVertices[ rgiSpinner2[i] ] );

   SetNormal(moverVertices, rgiSpinner3, 4);        // front
   for (int i = 0; i < 4; ++i)
       vbVerts.push_back( moverVertices[ rgiSpinner3[i] ] );

   SetNormal(moverVertices, rgiSpinner4, 4);        // bottom
   for (int i = 0; i < 4; ++i)
       vbVerts.push_back( moverVertices[ rgiSpinner4[i] ] );

   SetNormal(moverVertices, rgiSpinner5, 4);        // top
   for (int i = 0; i < 4; ++i)
       vbVerts.push_back( moverVertices[ rgiSpinner5[i] ] );

   SetNormal(moverVertices, rgiSpinner6, 4);        // left
   for (int i = 0; i < 4; ++i)
       vbVerts.push_back( moverVertices[ rgiSpinner6[i] ] );

   SetNormal(moverVertices, rgiSpinner7, 4);        // right
   for (int i = 0; i < 4; ++i)
       vbVerts.push_back( moverVertices[ rgiSpinner7[i] ] );

   if (vtxBuf)
       vtxBuf->release();
   pd3dDevice->CreateVertexBuffer(vbVerts.size(), 0, MY_D3DFVF_VERTEX, &vtxBuf);
   void *buf;
   vtxBuf->lock(0, 0, &buf, 0);
   memcpy(buf, &vbVerts[0], vbVerts.size() * sizeof(vbVerts[0]));
   vtxBuf->unlock();

   if (idxBuf)
       idxBuf->release();
   static const WORD idx[24] = {0,1,2,0,2,3, 4,5,6,4,6,7, 8,9,10,8,10,11, 12,13,14,12,14,15 };
   idxBuf = pd3dDevice->CreateAndFillIndexBuffer(24, idx);
}

void Spinner::RenderSetup(RenderDevice* pd3dDevice)
{
   PrepareStatic( pd3dDevice );
   Texture* const pinback = m_ptable->GetImage(m_d.m_szImageBack);
   Texture* const pinfront = m_ptable->GetImage(m_d.m_szImageFront);
   solidMaterial.setColor( 1.0f, m_d.m_color );

   if ( pinback )
   {
      pinback->CreateAlphaChannel();
   }
   if( pinfront )
   {
      pinfront->CreateAlphaChannel();
   }
   PrepareMovers( pd3dDevice );
}

void Spinner::RenderStatic(RenderDevice* pd3dDevice)
{
   if(!m_d.m_fSupports) return;

   Pin3D * const ppin3d = &g_pplayer->m_pin3d;
   pd3dDevice->SetVertexDeclaration( pd3dDevice->m_pVertexNormalTexelTexelDeclaration );

   //pd3dDevice->SetMaterial(staticMaterial);
   D3DXVECTOR4 matColor(0.6f,0.6f,0.6f,1.0f);   
   pd3dDevice->basicShader->Core()->SetFloat("fGlossyPower",0.0f);
   pd3dDevice->basicShader->Core()->SetVector("vDiffuseColor",&matColor);
   pd3dDevice->basicShader->Core()->SetTechnique("basic_without_texture");

   Vertex3D rgv3D[8];
   memcpy( rgv3D, staticVertices, sizeof(Vertex3D)*8);
   
   pd3dDevice->basicShader->Begin(0);
   SetNormal(rgv3D, rgiSpinnerNormal, 3, rgv3D, rgiSpinner0, 8);
   pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, MY_D3DFVF_VERTEX, rgv3D, 8, rgiSpinner0, 8);

   SetNormal(rgv3D, rgiSpinnerNormal, 3, rgv3D, rgiSpinner1, 8);
   pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, MY_D3DFVF_VERTEX, rgv3D, 8, rgiSpinner1, 8);
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
   bw.WriteInt(FID(SVIS), m_d.m_fVisible);
   bw.WriteBool(FID(SSUPT), m_d.m_fSupports);
   bw.WriteFloat(FID(OVRH), m_d.m_overhang);
   bw.WriteInt(FID(COLR), m_d.m_color);
   bw.WriteBool(FID(CSHD), m_d.m_fCastsShadow);	//<<< added by Chris
   bw.WriteString(FID(IMGF), m_d.m_szImageFront);
   bw.WriteString(FID(IMGB), m_d.m_szImageBack);
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
   else if (id == FID(COLR))
   {
      pbr->GetInt(&m_d.m_color);
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
      pbr->GetBool(&m_d.m_fSupports); 
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
      pbr->GetInt(&m_d.m_fVisible);
   }
   else if (id == FID(OVRH))
   {
      pbr->GetFloat(&m_d.m_overhang);
   }
   else if (id == FID(IMGF))
   {
      pbr->GetString(m_d.m_szImageFront);
   }
   else if (id == FID(IMGB))
   {
      pbr->GetString(m_d.m_szImageBack);
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

STDMETHODIMP Spinner::get_Color(OLE_COLOR *pVal)
{
   *pVal = m_d.m_color;

   return S_OK;
}

STDMETHODIMP Spinner::put_Color(OLE_COLOR newVal)
{
   STARTUNDO

      m_d.m_color = newVal;

   STOPUNDO

      return S_OK;
}

STDMETHODIMP Spinner::get_ImageFront(BSTR *pVal)
{
   WCHAR wz[512];

   MultiByteToWideChar(CP_ACP, 0, m_d.m_szImageFront, -1, wz, 32);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP Spinner::put_ImageFront(BSTR newVal)
{
   STARTUNDO

      WideCharToMultiByte(CP_ACP, 0, newVal, -1, m_d.m_szImageFront, 32, NULL, NULL);

   STOPUNDO

      return S_OK;
}

STDMETHODIMP Spinner::get_ImageBack(BSTR *pVal)
{
   WCHAR wz[512];

   MultiByteToWideChar(CP_ACP, 0, m_d.m_szImageBack, -1, wz, 32);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP Spinner::put_ImageBack(BSTR newVal)
{
   STARTUNDO

      WideCharToMultiByte(CP_ACP, 0, newVal, -1, m_d.m_szImageBack, 32, NULL, NULL);

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

STDMETHODIMP Spinner::get_Supports(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_d.m_fSupports);

   return S_OK;
}

STDMETHODIMP Spinner::put_Supports(VARIANT_BOOL newVal)
{	
   STARTUNDO

      m_d.m_fSupports = newVal;

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
      m_phitspinner->m_spinneranim.m_fVisible = newVal;// && m_d.m_fVisible;
   }
   else
   {
      STARTUNDO

         m_d.m_fVisible = newVal;

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
