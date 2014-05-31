#include "stdafx.h"

Bumper::Bumper()
{
   m_pbumperhitcircle = NULL;
   vtxBuf = NULL;
   idxBuf = NULL;
}

Bumper::~Bumper()
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

HRESULT Bumper::Init(PinTable *ptable, float x, float y, bool fromMouseClick)
{
   m_ptable = ptable;

   SetDefaults(fromMouseClick);

   m_d.m_vCenter.x = x;
   m_d.m_vCenter.y = y;

   m_fLockedByLS = false;			//>>> added by chris
   m_realState	= m_d.m_state;		//>>> added by chris

   return InitVBA(fTrue, 0, NULL);
}

void Bumper::SetDefaults(bool fromMouseClick)
{
   HRESULT hr;
   float fTmp;
   int iTmp;

   hr = GetRegStringAsFloat("DefaultProps\\Bumper","Radius", &fTmp);
   m_d.m_radius = (hr == S_OK) && fromMouseClick ? fTmp : 45;

   hr = GetRegStringAsFloat("DefaultProps\\Bumper","Force", &fTmp);
   m_d.m_force = (hr == S_OK) && fromMouseClick ? fTmp : 15;

   hr = GetRegStringAsFloat("DefaultProps\\Bumper","HeightOffset", &fTmp);
   m_d.m_heightoffset = (hr == S_OK) && fromMouseClick ? fTmp : 0;

   hr = GetRegStringAsFloat("DefaultProps\\Bumper","Threshold", &fTmp);
   m_d.m_threshold = (hr == S_OK) && fromMouseClick ? fTmp : 1;

   hr = GetRegStringAsFloat("DefaultProps\\Bumper","Overhang", &fTmp);
   m_d.m_overhang = (hr == S_OK) && fromMouseClick ? fTmp : 25;

   hr = GetRegInt("DefaultProps\\Bumper","Color", &iTmp);
   m_d.m_color = (hr == S_OK) && fromMouseClick ? iTmp : RGB(255,0,0);

   hr = GetRegInt("DefaultProps\\Bumper","SideColor", &iTmp);
   m_d.m_sidecolor = (hr == S_OK) && fromMouseClick ? iTmp : RGB(255,255,255);

   hr = GetRegString("DefaultProps\\Bumper","Image", m_d.m_szImage, MAXTOKEN);
   if ((hr != S_OK) || !fromMouseClick)
      m_d.m_szImage[0] = 0;

   hr = GetRegString("DefaultProps\\Bumper","Surface", m_d.m_szSurface, MAXTOKEN);
   if (hr != S_OK || !fromMouseClick )	
      m_d.m_szSurface[0] = 0;

   hr = GetRegInt("DefaultProps\\Bumper","TimerEnabled", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_tdr.m_fTimerEnabled = iTmp == 0 ? fFalse : fTrue;
   else
      m_d.m_tdr.m_fTimerEnabled = fFalse;

   hr = GetRegInt("DefaultProps\\Bumper","TimerInterval", &iTmp);
   m_d.m_tdr.m_TimerInterval = (hr == S_OK) && fromMouseClick ? iTmp : 100;

   hr = GetRegInt("DefaultProps\\Bumper","LightState", &iTmp);
   m_d.m_state = (hr == S_OK) && fromMouseClick ? (enum LightState)iTmp : LightStateOff;

   hr = GetRegString("DefaultProps\\Bumper","BlinkPattern", m_rgblinkpattern, MAXTOKEN);
   if ((hr != S_OK) || !fromMouseClick)
      strcpy_s(m_rgblinkpattern, sizeof(m_rgblinkpattern), "10");

   hr = GetRegInt("DefaultProps\\Bumper","BlinkInterval", &iTmp);
   m_blinkinterval = (hr == S_OK) && fromMouseClick ? iTmp : 125;

   hr = GetRegInt("DefaultProps\\Bumper","FlashWhenHit", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_fFlashWhenHit = iTmp == 0 ? false : true;
   else
      m_d.m_fFlashWhenHit = fTrue;

   hr = GetRegInt("DefaultProps\\Bumper","CastsShadow", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_fCastsShadow = iTmp == 0 ? false : true;
   else
      m_d.m_fCastsShadow = fTrue;

   hr = GetRegInt("DefaultProps\\Bumper","Visible", &iTmp);
   if ((hr == S_OK)&& fromMouseClick)
      m_d.m_fVisible = iTmp == 0 ? false : true;
   else
      m_d.m_fVisible = fTrue;

   hr = GetRegInt("DefaultProps\\Bumper","SideVisible", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_fSideVisible = iTmp == 0 ? false : true;
   else
      m_d.m_fSideVisible = fTrue;
}

void Bumper::WriteRegDefaults()
{
   SetRegValueFloat("DefaultProps\\Bumper","Radius", m_d.m_radius);
   SetRegValueFloat("DefaultProps\\Bumper","Force", m_d.m_force);
   SetRegValueFloat("DefaultProps\\Bumper","HeightOffset", m_d.m_heightoffset);
   SetRegValueFloat("DefaultProps\\Bumper","Threshold", m_d.m_threshold);
   SetRegValueFloat("DefaultProps\\Bumper","Overhang", m_d.m_overhang);
   SetRegValue("DefaultProps\\Bumper","Color", REG_DWORD, &m_d.m_color,4);	
   SetRegValue("DefaultProps\\Bumper","SideColor", REG_DWORD, &m_d.m_sidecolor,4);	
   SetRegValue("DefaultProps\\Bumper","Image", REG_SZ, &m_d.m_szImage,lstrlen(m_d.m_szImage));	
   SetRegValue("DefaultProps\\Bumper","TimerEnabled", REG_DWORD, &m_d.m_tdr.m_fTimerEnabled,4);	
   SetRegValue("DefaultProps\\Bumper","TimerInterval", REG_DWORD, &m_d.m_tdr.m_TimerInterval,4);	
   SetRegValue("DefaultProps\\Bumper","LightState", REG_DWORD, (int *)&m_d.m_state,4);	
   SetRegValue("DefaultProps\\Bumper","BlinkPattern", REG_SZ, &m_rgblinkpattern,strlen(m_rgblinkpattern));	
   SetRegValue("DefaultProps\\Bumper","BlinkInterval", REG_DWORD, &m_blinkinterval,4);	
   SetRegValue("DefaultProps\\Bumper","FlashWhenHit", REG_DWORD, &m_d.m_fFlashWhenHit,4);	
   SetRegValue("DefaultProps\\Bumper","CastsShadow", REG_DWORD, &m_d.m_fCastsShadow,4);	
   SetRegValue("DefaultProps\\Bumper","Visible", REG_DWORD, &m_d.m_fVisible,4);	
   SetRegValue("DefaultProps\\Bumper","SideVisible", REG_DWORD, &m_d.m_fSideVisible,4);	
   SetRegValue("DefaultProps\\Bumper","Surface", REG_SZ, &m_d.m_szSurface,strlen(m_d.m_szSurface));	
}

STDMETHODIMP Bumper::InterfaceSupportsErrorInfo(REFIID riid)
{
   static const IID* arr[] =
   {
      &IID_IBumper,
   };

   for (size_t i=0;i<sizeof(arr)/sizeof(arr[0]);i++)
   {
      if (InlineIsEqualGUID(*arr[i],riid))
         return S_OK;
   }
   return S_FALSE;
}

void Bumper::PreRender(Sur * const psur)
{
   psur->SetBorderColor(-1,false,0);
   psur->SetFillColor(m_ptable->RenderSolid() ? m_d.m_color : -1);
   psur->SetObject(this);

   psur->Ellipse(m_d.m_vCenter.x, m_d.m_vCenter.y, m_d.m_radius);
}

void Bumper::Render(Sur * const psur)
{
   psur->SetBorderColor(RGB(0,0,0),false,0);
   psur->SetFillColor(-1);
   psur->SetObject(this);
   psur->SetObject(NULL);

   psur->Ellipse(m_d.m_vCenter.x, m_d.m_vCenter.y, m_d.m_radius);

   psur->Ellipse(m_d.m_vCenter.x, m_d.m_vCenter.y, m_d.m_radius + m_d.m_overhang);

   if (g_pvp->m_fAlwaysDrawLightCenters)
   {
      psur->Line(m_d.m_vCenter.x - 10.0f, m_d.m_vCenter.y, m_d.m_vCenter.x + 10.0f, m_d.m_vCenter.y);
      psur->Line(m_d.m_vCenter.x, m_d.m_vCenter.y - 10.0f, m_d.m_vCenter.x, m_d.m_vCenter.y + 10.0f);
   }
}

void Bumper::RenderShadow(ShadowSur * const psur, const float z)
{
   if ( (!m_d.m_fCastsShadow) || (!m_ptable->m_fRenderShadows) )
      return;

   psur->SetBorderColor(-1,false,0);
   psur->SetFillColor(RGB(0,0,0));

   const float height = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y);

   psur->EllipseSkew(m_d.m_vCenter.x, m_d.m_vCenter.y, m_d.m_radius, height, height+40.0f+m_d.m_heightoffset);

   psur->EllipseSkew(m_d.m_vCenter.x, m_d.m_vCenter.y, m_d.m_radius + m_d.m_overhang, height+40.0f+m_d.m_heightoffset, height+65.0f+m_d.m_heightoffset);
}

void Bumper::GetTimers(Vector<HitTimer> * const pvht)
{
   IEditable::BeginPlay();

   HitTimer * const pht = new HitTimer();
   pht->m_interval = m_d.m_tdr.m_TimerInterval;
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

   phitcircle->center.x = m_d.m_vCenter.x;
   phitcircle->center.y = m_d.m_vCenter.y;
   phitcircle->radius = m_d.m_radius;
   phitcircle->zlow = height;
   phitcircle->zhigh = height+50.0f+m_d.m_heightoffset;

   phitcircle->m_pbumper = this;

   pvho->AddElement(phitcircle);

   m_pbumperhitcircle = phitcircle;

   phitcircle->m_bumperanim.m_fVisible = m_d.m_fVisible;
   DrawFrame( m_d.m_state == LightStateOn );        // make sure bumper light gets turned on

   if (m_d.m_state == LightStateBlinking)
   {
      m_timenextblink = g_pplayer->m_time_msec + m_blinkinterval;
   }
   m_iblinkframe = 0;
}

void Bumper::GetHitShapesDebug(Vector<HitObject> * const pvho)
{
   const float height = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y);

   HitObject * const pho = CreateCircularHitPoly(m_d.m_vCenter.x, m_d.m_vCenter.y, height + 50.0f + m_d.m_heightoffset, m_d.m_radius + m_d.m_overhang, 32);
   pvho->AddElement(pho);
}

void Bumper::EndPlay()
{
    IEditable::EndPlay();

    // ensure not locked just incase the player exits during a LS sequence
    m_fLockedByLS = false;

    m_pbumperhitcircle = NULL;

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


void Bumper::PostRenderStatic(RenderDevice* pd3dDevice)
{
    TRACE_FUNCTION();

    /* HACK / VP9COMPAT:
     * Make pure black bumpers invisible for compatibility (e.g., Medusa by PacDude).
     */
    if (m_d.m_color == 0 && m_d.m_sidecolor == 0)
        return;

    if(!m_d.m_fVisible)	return;

    Pin3D * const ppin3d = &g_pplayer->m_pin3d;

    if (m_d.m_state == LightStateBlinking)
        UpdateBlinker(g_pplayer->m_time_msec);
    m_pbumperhitcircle->m_bumperanim.UpdateAnimation();
    const int state = m_pbumperhitcircle->m_bumperanim.m_iframe ? 1 : 0;    // 0 = off, 1 = lit

    Texture * const pin = m_ptable->GetImage(m_d.m_szImage);
    if (!pin) // Top solid color
    {
        if (state == 0) // off
        {
            ppin3d->SetTexture(NULL);
            pd3dDevice->SetMaterial(topNonLitMaterial);
        }
        else            // on
        {
            ppin3d->lightTexture[0].Set( ePictureTexture );
            pd3dDevice->SetMaterial(topLitMaterial);
        }

        // render the top circle
        pd3dDevice->DrawPrimitiveVB(D3DPT_TRIANGLEFAN, vtxBuf, 0, 32);
        // render the "mushroom"
        pd3dDevice->DrawIndexedPrimitiveVB(D3DPT_TRIANGLELIST, vtxBuf, 32, 8*32, idxBuf, 6*32, 12*32);
    }

    if (m_d.m_fSideVisible && /* VP9COMPAT */ m_d.m_sidecolor != 0)
    {
        if (state == 0)
        {
            ppin3d->SetTexture(NULL);
            pd3dDevice->SetMaterial(sideNonLitMaterial);
        }
        else
        {
            ppin3d->lightTexture[0].Set( ePictureTexture );
            pd3dDevice->SetMaterial(sideLitMaterial);
        }

        // render the side walls
        pd3dDevice->DrawIndexedPrimitiveVB(D3DPT_TRIANGLELIST, vtxBuf, 32+8*32, 2*32, idxBuf, 0, 6*32);
    }

    if (pin)
    {
        pin->EnsureBackdrop(m_d.m_color);
        pin->SetBackDrop( ePictureTexture );

        if (state == 0)
        {
            pd3dDevice->SetMaterial(nonLitMaterial);
        }
        else
        {
            ppin3d->lightTexture[0].Set( eLightProject1 );
            pd3dDevice->SetMaterial(litMaterial);
        }

        // render the top circle
        pd3dDevice->DrawPrimitiveVB(D3DPT_TRIANGLEFAN, vtxBuf, 0, 32);
        // render the "mushroom"
        pd3dDevice->DrawIndexedPrimitiveVB(D3DPT_TRIANGLELIST, vtxBuf, 32, 8*32, idxBuf, 6*32, 12*32);

        // Reset all the texture coordinates
        if (state == 1)
        {
            ppin3d->DisableLightMap();
        }
    }
}

void Bumper::RenderSetup(RenderDevice* pd3dDevice )
{
    if (m_d.m_state == LightStateBlinking)
        RestartBlinker(g_pplayer->m_time_msec);

   const float outerradius = m_d.m_radius + m_d.m_overhang;
   const float height = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y) * m_ptable->m_zScale;

   Pin3D * const ppin3d = &g_pplayer->m_pin3d;
   Texture * const pin = m_ptable->GetImage(m_d.m_szImage);	

   float r = (float)(m_d.m_color & 255) * (float) (1.0/255.0);
   float g = (float)(m_d.m_color & 65280) * (float) (1.0/65280.0);
   float b = (float)(m_d.m_color & 16711680) * (float) (1.0/16711680.0);
   topNonLitMaterial.setAmbient( 1.0f, r*0.5f, g*0.5f, b*0.5f );
   topNonLitMaterial.setDiffuse( 1.0f, r*0.5f, g*0.5f, b*0.5f );
   topLitMaterial.setAmbient(1.0f, 0.0f, 0.0f, 0.0f );
   topLitMaterial.setDiffuse(1.0f, 0.0f, 0.0f, 0.0f );
   topLitMaterial.setEmissive( 0.0f, r, g, b );

   r = (float)(m_d.m_sidecolor & 255) * (float) (1.0/255.0);
   g = (float)(m_d.m_sidecolor & 65280) * (float) (1.0/65280.0);
   b = (float)(m_d.m_sidecolor & 16711680) * (float) (1.0/16711680.0);
   sideNonLitMaterial.setAmbient( 1.0f, r*0.5f, g*0.5f, b*0.5f );
   sideNonLitMaterial.setDiffuse( 1.0f, r*0.5f, g*0.5f, b*0.5f );
   sideLitMaterial.setAmbient(1.0f, 0.0f, 0.0f, 0.0f );
   sideLitMaterial.setDiffuse(1.0f, 0.0f, 0.0f, 0.0f );
   sideLitMaterial.setEmissive( 0.0f, r, g, b );

   nonLitMaterial.setAmbient( 1.0f, 0.5f, 0.5f, 0.5f );
   nonLitMaterial.setDiffuse( 1.0f, 0.5f, 0.5f, 0.5f );
   nonLitMaterial.setEmissive(0.0f, 0.0f, 0.0f, 0.0f );

   litMaterial.setAmbient( 1.0f, 0.0f, 0.0f, 0.0f );
   litMaterial.setDiffuse( 1.0f, 0.0f, 0.0f, 0.0f );
   litMaterial.setEmissive(0.0f, 1.0f, 1.0f, 1.0f );

   Vertex3D moverVertices[5*32];
   WORD     indices[4*32];
   WORD     normalIndices[6*32];

   std::vector<WORD> allIndices;        // collect all indices which should go into an index buffer
   allIndices.reserve(6*32 + 12*32);

   for (int l=0,i=0,t=0; l<32; l++,t+=6,i+=4)
   {
      normalIndices[t  ] = (l==0) ? 31 : (l-1);
      normalIndices[t+1] = (l==0) ? 63 : (l+31);
      normalIndices[t+2] = (l==0) ? 33 : (l+1);
      normalIndices[t+3] = l;
      normalIndices[t+4] = l+32;
      normalIndices[t+5] = (l<30) ? (l+2) : (l-30);

      indices[i  ] = l;
      indices[i+1] = 32 + l;
      indices[i+2] = 32 + (l+1) % 32;
      indices[i+3] = (l+1) % 32;

      allIndices.push_back(l);
      allIndices.push_back(32 + l);
      allIndices.push_back(32 + (l+1) % 32);
      allIndices.push_back(l);
      allIndices.push_back(32 + (l+1) % 32);
      allIndices.push_back((l+1) % 32);

      const float angle = (float)(M_PI*2.0/32.0)*(float)l;
      const float sinangle =  sinf(angle);
      const float cosangle = -cosf(angle);

      // top circle of base cylinder
      moverVertices[l].x = sinangle*m_d.m_radius + m_d.m_vCenter.x;
      moverVertices[l].y = cosangle*m_d.m_radius + m_d.m_vCenter.y;
      moverVertices[l].z = height+(40.0f+m_d.m_heightoffset)*m_ptable->m_zScale;
      moverVertices[l].nx = sinangle;
      moverVertices[l].ny = cosangle;
      moverVertices[l].nz = 0.0f;

      // bottom circle of base cylinder
      moverVertices[l+32] = moverVertices[l];
      moverVertices[l+32].z = height;

      // top circle of cap
      moverVertices[l+64].x = sinangle*outerradius*0.5f + m_d.m_vCenter.x;
      moverVertices[l+64].y = cosangle*outerradius*0.5f + m_d.m_vCenter.y;
      moverVertices[l+64].z = height+(60.0f+m_d.m_heightoffset)*m_ptable->m_zScale;

      // middle ring of cap
      moverVertices[l+96].x = sinangle*outerradius*0.9f + m_d.m_vCenter.x;
      moverVertices[l+96].y = cosangle*outerradius*0.9f + m_d.m_vCenter.y;
      moverVertices[l+96].z = height+(50.0f+m_d.m_heightoffset)*m_ptable->m_zScale;

      // outer rim of cap
      moverVertices[l+128].x = sinangle*outerradius + m_d.m_vCenter.x;
      moverVertices[l+128].y = cosangle*outerradius + m_d.m_vCenter.y;
      moverVertices[l+128].z = height+(40.0f+m_d.m_heightoffset)*m_ptable->m_zScale;

      moverVertices[l].tu = 0.5f+sinangle*0.5f;
      moverVertices[l].tv = 0.5f-cosangle*0.5f;
      moverVertices[l+32].tu = 0.5f+sinangle*0.5f;
      moverVertices[l+32].tv = 0.5f-cosangle*0.5f;
      moverVertices[l+64].tu = 0.5f+sinangle*0.25f;
      moverVertices[l+64].tv = 0.5f-cosangle*0.25f;
      moverVertices[l+96].tu = 0.5f+sinangle*(float)(0.5*0.9);
      moverVertices[l+96].tv = 0.5f-cosangle*(float)(0.5*0.9);
      moverVertices[l+128].tu = 0.5f+sinangle*0.5f;
      moverVertices[l+128].tv = 0.5f-cosangle*0.5f;

      if ( pin )
      {
         moverVertices[l+64].tu = 0.5f+sinangle*0.25f;
         moverVertices[l+64].tv = 0.5f+cosangle*0.25f;
         moverVertices[l+96].tu = 0.5f+sinangle*(float)(0.5*0.9);
         moverVertices[l+96].tv = 0.5f+cosangle*(float)(0.5*0.9);
         moverVertices[l+128].tu = 0.5f+sinangle*0.5f;
         moverVertices[l+128].tv = 0.5f+cosangle*0.5f;

         const float lightmaxtu = 0.8f;
         const float lightmaxtv = 0.8f;

         moverVertices[l].tu2 = moverVertices[l].tu;
         moverVertices[l+32].tu2 = moverVertices[l+32].tu;
         moverVertices[l].tv2 = moverVertices[l].tv;
         moverVertices[l+32].tv2 = moverVertices[l+32].tv;
         moverVertices[l+64].tu2 = (0.5f+sinangle*0.25f)*lightmaxtu;
         moverVertices[l+64].tv2 = (0.5f+cosangle*0.25f)*lightmaxtv;
         moverVertices[l+96].tu2 = (0.5f+sinangle*(float)(0.5*0.9))*lightmaxtu;
         moverVertices[l+96].tv2 = (0.5f+cosangle*(float)(0.5*0.9))*lightmaxtv;
         moverVertices[l+128].tu2 = (0.5f+sinangle*0.5f)*lightmaxtu;
         moverVertices[l+128].tv2 = (0.5f+cosangle*0.5f)*lightmaxtv;
      }
   }
   ppin3d->CalcShadowCoordinates(moverVertices,32*5);

   for( int l=0,k=0; l<32*12; l+=6,k+=4 )
   {
      allIndices.push_back( k   );
      allIndices.push_back( k+1 );
      allIndices.push_back( k+2 );
      allIndices.push_back( k   );
      allIndices.push_back( k+2 );
      allIndices.push_back( k+3 );
   }


   if (idxBuf)
       idxBuf->release();
   idxBuf = pd3dDevice->CreateAndFillIndexBuffer( allIndices );

   Vertex3D dynVerts[8*32];
   for (int l=0,t=0,k=0,ofs=0; l<32; l++,t+=6,k+=4,ofs+=8)
   {
      SetNormal(&moverVertices[64], &normalIndices[t], 3, NULL, &indices[k], 2);
      SetNormal(&moverVertices[96], &normalIndices[t], 3, NULL, &indices[k], 2);
      SetNormal(&moverVertices[64], &normalIndices[t+3], 3, NULL, &indices[k+2], 2);
      SetNormal(&moverVertices[96], &normalIndices[t+3], 3, NULL, &indices[k+2], 2);
      dynVerts[ofs  ] = moverVertices[64+indices[k  ]];
      dynVerts[ofs+1] = moverVertices[64+indices[k+1]];
      dynVerts[ofs+2] = moverVertices[64+indices[k+2]];
      dynVerts[ofs+3] = moverVertices[64+indices[k+3]];
      dynVerts[ofs+4] = moverVertices[96+indices[k  ]];
      dynVerts[ofs+5] = moverVertices[96+indices[k+1]];
      dynVerts[ofs+6] = moverVertices[96+indices[k+2]];
      dynVerts[ofs+7] = moverVertices[96+indices[k+3]];
   }

   static const WORD rgiBumperStatic[32] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
   SetNormal(&moverVertices[64], rgiBumperStatic, 32, NULL, NULL, 0);

   if (!vtxBuf)
       pd3dDevice->CreateVertexBuffer(32+8*32+2*32, 0, MY_D3DFVF_VERTEX, &vtxBuf);

   Vertex3D *buf;
   vtxBuf->lock(0, 0, (void**)&buf, 0);
   memcpy(buf, &moverVertices[64], 32*sizeof(buf[0]));
   memcpy(buf+32, dynVerts, 8*32*sizeof(buf[0]));
   memcpy(buf+32+8*32, moverVertices, 2*32*sizeof(buf[0]));
   vtxBuf->unlock();

   // ensure we are not disabled at game start
   m_fDisabled = fFalse;
}

void Bumper::RenderStatic(RenderDevice* pd3dDevice)
{
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
   bw.WriteFloat(FID(HOFF), m_d.m_heightoffset);
   bw.WriteFloat(FID(OVRH), m_d.m_overhang);
   bw.WriteInt(FID(COLR), m_d.m_color);
   bw.WriteInt(FID(SCLR), m_d.m_sidecolor);
   bw.WriteString(FID(IMAG), m_d.m_szImage);
   bw.WriteString(FID(SURF), m_d.m_szSurface);
   bw.WriteWideString(FID(NAME), (WCHAR *)m_wzName);
   bw.WriteInt(FID(STAT), m_d.m_state);
   bw.WriteString(FID(BPAT), m_rgblinkpattern);
   bw.WriteInt(FID(BINT), m_blinkinterval);

   bw.WriteBool(FID(TRNS), m_d.m_fFlashWhenHit);

   bw.WriteBool(FID(CSHD), m_d.m_fCastsShadow);
   bw.WriteBool(FID(BVIS), m_d.m_fVisible);
   bw.WriteBool(FID(BSVS), m_d.m_fSideVisible);

   ISelect::SaveData(pstm, hcrypthash, hcryptkey);

   bw.WriteTag(FID(ENDB));

   return S_OK;
}


HRESULT Bumper::InitLoad(IStream *pstm, PinTable *ptable, int *pid, int version, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
{
   SetDefaults(false);
   BiffReader br(pstm, this, pid, version, hcrypthash, hcryptkey);

   m_ptable = ptable;

   m_fLockedByLS = false;			//>>> added by chris
   m_realState	= m_d.m_state;		//>>> added by chris

   br.Load();

   if (version < 41)
      m_d.m_sidecolor = m_d.m_color; // So tables look like they did before

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
   else if (id == FID(COLR))
   {
      pbr->GetInt(&m_d.m_color);
   }
   else if (id == FID(SCLR))
   {
      pbr->GetInt(&m_d.m_sidecolor);
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
   else if (id == FID(HOFF))
   {
      pbr->GetFloat(&m_d.m_heightoffset);
   }
   else if (id == FID(OVRH))
   {
      pbr->GetFloat(&m_d.m_overhang);
   }
   else if (id == FID(IMAG))
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
   else if (id == FID(STAT))
   {
      pbr->GetInt(&m_d.m_state);
      m_realState	= m_d.m_state;		//>>> added by chris
   }
   else if (id == FID(BPAT))
   {
      pbr->GetString(m_rgblinkpattern);
   }
   else if (id == FID(BINT))
   {
      pbr->GetInt(&m_blinkinterval);
   }
   else if (id == FID(TRNS))
   {
      pbr->GetBool(&m_d.m_fFlashWhenHit);
   }
   else if (id == FID(CSHD))
   {
      pbr->GetBool(&m_d.m_fCastsShadow);
   }
   else if (id == FID(BVIS))
   {
      pbr->GetBool(&m_d.m_fVisible);
   }
   else if (id == FID(BSVS))
   {
      pbr->GetBool(&m_d.m_fSideVisible);
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

void Bumper::DrawFrame(BOOL fOn)
{
   m_pbumperhitcircle->m_bumperanim.m_iframedesired = fOn;
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

STDMETHODIMP Bumper::get_HeightOffset(float *pVal)
{
   *pVal = m_d.m_heightoffset;

   return S_OK;
}

STDMETHODIMP Bumper::put_HeightOffset(float newVal)
{
   STARTUNDO

   m_d.m_heightoffset = newVal;

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

STDMETHODIMP Bumper::get_Overhang(float *pVal)
{
   *pVal = m_d.m_overhang;

   return S_OK;
}

STDMETHODIMP Bumper::put_Overhang(float newVal)
{
   STARTUNDO

   m_d.m_overhang = newVal;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Bumper::get_Color(OLE_COLOR *pVal)
{
   *pVal = m_d.m_color;

   return S_OK;
}

STDMETHODIMP Bumper::put_Color(OLE_COLOR newVal)
{
   STARTUNDO

   m_d.m_color = newVal;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Bumper::get_SideColor(OLE_COLOR *pVal)
{
   *pVal = m_d.m_sidecolor;

   return S_OK;
}

STDMETHODIMP Bumper::put_SideColor(OLE_COLOR newVal)
{
   STARTUNDO

   m_d.m_sidecolor = newVal;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Bumper::get_Image(BSTR *pVal)
{
   WCHAR wz[512];

   MultiByteToWideChar(CP_ACP, 0, m_d.m_szImage, -1, wz, 32);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP Bumper::put_Image(BSTR newVal)
{
   STARTUNDO

   WideCharToMultiByte(CP_ACP, 0, newVal, -1, m_d.m_szImage, 32, NULL, NULL);

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

   pproppane = new PropertyPane(IDD_PROPBUMPER_VISUALS, IDS_VISUALS);
   pvproppane->AddElement(pproppane);

   pproppane = new PropertyPane(IDD_PROPLIGHT_POSITION, IDS_POSITION);
   pvproppane->AddElement(pproppane);

   pproppane = new PropertyPane(IDD_PROPBUMPER_STATE, IDS_STATE);
   pvproppane->AddElement(pproppane);

   pproppane = new PropertyPane(IDD_PROPBUMPER_PHYSICS, IDS_PHYSICS);
   pvproppane->AddElement(pproppane);

   pproppane = new PropertyPane(IDD_PROP_TIMER, IDS_MISC);
   pvproppane->AddElement(pproppane);
}

STDMETHODIMP Bumper::get_State(LightState *pVal)
{
   *pVal = m_d.m_state;

   return S_OK;
}

STDMETHODIMP Bumper::put_State(LightState newVal)
{
   STARTUNDO

   // if the light is locked by the LS then just change the state and don't change the actual light
   if (!m_fLockedByLS)
      setLightState(newVal);
   m_d.m_state = newVal;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Bumper::get_BlinkPattern(BSTR *pVal)
{
   WCHAR wz[512];

   MultiByteToWideChar(CP_ACP, 0, m_rgblinkpattern, -1, wz, 32);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP Bumper::put_BlinkPattern(BSTR newVal)
{
   STARTUNDO

   WideCharToMultiByte(CP_ACP, 0, newVal, -1, m_rgblinkpattern, 32, NULL, NULL);

   if (m_rgblinkpattern[0] == '\0')
   {
      m_rgblinkpattern[0] = '0';
      m_rgblinkpattern[1] = '\0';
   }

   if (g_pplayer)
   {
       RestartBlinker(g_pplayer->m_time_msec);
   }

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Bumper::get_BlinkInterval(long *pVal)
{
   *pVal = m_blinkinterval;

   return S_OK;
}

STDMETHODIMP Bumper::put_BlinkInterval(long newVal)
{
   STARTUNDO

   m_blinkinterval = newVal;

   if (g_pplayer)
      m_timenextblink = g_pplayer->m_time_msec + m_blinkinterval;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Bumper::get_FlashWhenHit(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_d.m_fFlashWhenHit);

   return S_OK;
}

STDMETHODIMP Bumper::put_FlashWhenHit(VARIANT_BOOL newVal)
{
   m_d.m_fFlashWhenHit = VBTOF(newVal);

   return S_OK;
}

STDMETHODIMP Bumper::get_CastsShadow(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_d.m_fCastsShadow);

   return S_OK;
}

STDMETHODIMP Bumper::put_CastsShadow(VARIANT_BOOL newVal)
{
   STARTUNDO
   m_d.m_fCastsShadow = VBTOF(newVal);
   STOPUNDO

   return S_OK;
}

STDMETHODIMP Bumper::get_Disabled(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_fDisabled);

   return S_OK;
}

STDMETHODIMP Bumper::put_Disabled(VARIANT_BOOL newVal)
{
   STARTUNDO
   m_fDisabled = VBTOF(newVal);
   STOPUNDO

   return S_OK;
}

STDMETHODIMP Bumper::get_Visible(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_d.m_fVisible);

   return S_OK;
}

STDMETHODIMP Bumper::put_Visible(VARIANT_BOOL newVal)
{
   STARTUNDO
   m_d.m_fVisible = VBTOF(newVal);
   STOPUNDO

   return S_OK;
}

STDMETHODIMP Bumper::get_SideVisible(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_d.m_fSideVisible);

   return S_OK;
}

STDMETHODIMP Bumper::put_SideVisible(VARIANT_BOOL newVal)
{
   STARTUNDO
   m_d.m_fSideVisible = VBTOF(newVal);
   STOPUNDO

   return S_OK;
}

void Bumper::lockLight()
{
   m_fLockedByLS = true;
}

void Bumper::unLockLight()
{
   m_fLockedByLS = false;
}

void Bumper::setLightStateBypass(const LightState newVal)
{
   lockLight();
   setLightState(newVal);
}

void Bumper::setLightState(const LightState newVal)
{
   if (newVal != m_realState)
   {
      const LightState lastState = m_realState;
      m_realState = newVal;

      if (m_pbumperhitcircle)
      {
         switch (m_realState)
         {
         case LightStateOff:
            DrawFrame(fFalse);
            break;

         case LightStateOn:
            DrawFrame(fTrue);
            break;

         case LightStateBlinking:
            m_iblinkframe = 0; // reset pattern
            m_timenextblink = g_pplayer->m_time_msec; // Start pattern right away // + m_d.m_blinkinterval;
            break;
         }
      }
   }
}
