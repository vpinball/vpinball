#include "stdafx.h"

Bumper::Bumper()
{
   m_pbumperhitcircle = NULL;
}

Bumper::~Bumper()
{
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
      m_d.m_tdr.m_fTimerEnabled = iTmp == 0 ? false : true;
   else
      m_d.m_tdr.m_fTimerEnabled = false;

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
   char strTmp[40];

   sprintf_s(strTmp, 40, "%f", m_d.m_radius);
   SetRegValue("DefaultProps\\Bumper","Radius", REG_SZ, &strTmp,strlen(strTmp));	
   sprintf_s(strTmp, 40, "%f", m_d.m_force);
   SetRegValue("DefaultProps\\Bumper","Force", REG_SZ, &strTmp,strlen(strTmp));	
   sprintf_s(strTmp, 40, "%f", m_d.m_threshold);
   SetRegValue("DefaultProps\\Bumper","Threshold", REG_SZ, &strTmp,strlen(strTmp));	
   sprintf_s(strTmp, 40, "%f", m_d.m_overhang);
   SetRegValue("DefaultProps\\Bumper","Overhang", REG_SZ, &strTmp,strlen(strTmp));	
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

   for (int i=0;i<sizeof(arr)/sizeof(arr[0]);i++)
   {
      if (InlineIsEqualGUID(*arr[i],riid))
         return S_OK;
   }
   return S_FALSE;
}

void Bumper::PreRender(Sur * const psur)
{
   psur->SetBorderColor(-1,false,0);
   psur->SetFillColor(m_d.m_color);
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

   psur->EllipseSkew(m_d.m_vCenter.x, m_d.m_vCenter.y, m_d.m_radius, height, height+40.0f);

   psur->EllipseSkew(m_d.m_vCenter.x, m_d.m_vCenter.y, m_d.m_radius + m_d.m_overhang, height+40.0f, height+65.0f);
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
   {
      pvht->AddElement(pht);
   }
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
   phitcircle->zhigh = height+50.0f;

   phitcircle->m_pbumper = this;

   pvho->AddElement(phitcircle);

   m_pbumperhitcircle = phitcircle;

   phitcircle->m_bumperanim.m_fVisible = m_d.m_fVisible;

   // HACK - should pass pointer to vector in
   if (m_d.m_state == LightStateBlinking)
   {
      g_pplayer->m_vblink.AddElement((IBlink *)this);
      m_timenextblink = g_pplayer->m_timeCur + m_blinkinterval;
   }
   m_iblinkframe = 0;
}

void Bumper::GetHitShapesDebug(Vector<HitObject> * const pvho)
{
   const float height = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y);

   HitObject * const pho = CreateCircularHitPoly(m_d.m_vCenter.x, m_d.m_vCenter.y, height + 50.0f, m_d.m_radius + m_d.m_overhang, 32);
   pvho->AddElement(pho);
}

void Bumper::EndPlay()
{
   IEditable::EndPlay();

   // ensure not locked just incase the player exits during a LS sequence
   m_fLockedByLS = false;

   m_pbumperhitcircle = NULL;
}

void Bumper::PostRenderStatic(const RenderDevice* pd3dDevice)
{
}

static const WORD rgiBumperStatic[32] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31};
void Bumper::RenderSetup(const RenderDevice* _pd3dDevice )
{
   int l,t,i;
   const float outerradius = m_d.m_radius + m_d.m_overhang;
   const float height = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y);
   const float inv_width  = 1.0f/(g_pplayer->m_ptable->m_left + g_pplayer->m_ptable->m_right);
   const float inv_height = 1.0f/(g_pplayer->m_ptable->m_top  + g_pplayer->m_ptable->m_bottom);
   float maxtu, maxtv;

   Pin3D * const ppin3d = &g_pplayer->m_pin3d;
   Texture * const pin = m_ptable->GetImage(m_d.m_szImage);	

   float r = (m_d.m_color & 255) * (float) (1.0/255.0);
   float g = (m_d.m_color & 65280) * (float) (1.0/65280.0);
   float b = (m_d.m_color & 16711680) * (float) (1.0/16711680.0);
   topNonLitMaterial.setAmbient( 1.0f, r*0.5f, g*0.5f, b*0.5f );
   topNonLitMaterial.setDiffuse( 1.0f, r*0.5f, g*0.5f, b*0.5f );
   topLitMaterial.setAmbient(1.0f, 0.0f, 0.0f, 0.0f );
   topLitMaterial.setDiffuse(1.0f, 0.0f, 0.0f, 0.0f );
   topLitMaterial.setEmissive( 0.0f, r, g, b );

   r = (m_d.m_sidecolor & 255) * (float) (1.0/255.0);
   g = (m_d.m_sidecolor & 65280) * (float) (1.0/65280.0);
   b = (m_d.m_sidecolor & 16711680) * (float) (1.0/16711680.0);
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

   if ( pin )
   {
      m_ptable->GetTVTU(pin, &maxtu, &maxtv);
   }
   t=0;
   i=0;
   for (l=0;l<32;l++,t+=6,i+=4)
   {
      normalIndices[t  ] = (l==0) ? 31 : (l-1);
      normalIndices[t+1] = (l==0) ? 63 : (l+31);
      normalIndices[t+2] = (l==0) ? 33 : (l+1);
      normalIndices[t+3] = l;
      normalIndices[t+4] = l+32;
      normalIndices[t+5] = (l<30) ? (l+2) : (l-30);

      indices[i  ] = l;
      indices[i+1] = l+32;
      indices[i+2] = (l==31) ? 32 : (l+33);
      indices[i+3] = (l==31) ? 0  : (l+1);

      const float angle = (float)(M_PI*2.0/32.0)*(float)l;
      const float sinangle =  sinf(angle);
      const float cosangle = -cosf(angle);

      staticVertices[l].x    = sinangle*outerradius*0.5f + m_d.m_vCenter.x;
      staticVertices[l].y    = cosangle*outerradius*0.5f + m_d.m_vCenter.y;
      staticVertices[l].z    = height+60.0f;

      staticVertices[l+32].x = sinangle*outerradius*0.9f + m_d.m_vCenter.x;
      staticVertices[l+32].y = cosangle*outerradius*0.9f + m_d.m_vCenter.y;
      staticVertices[l+32].z = height+50.0f;

      staticVertices[l+64].x = sinangle*outerradius + m_d.m_vCenter.x;
      staticVertices[l+64].y = cosangle*outerradius + m_d.m_vCenter.y;
      staticVertices[l+64].z = height+40.0f;

      if (pin)
      {
         staticVertices[l].tu    = (0.5f+sinangle*0.25f)*maxtu;
         staticVertices[l].tv    = (0.5f+cosangle*0.25f)*maxtv;
         staticVertices[l+32].tu = (0.5f+sinangle*(float)(0.5*0.9))*maxtu;
         staticVertices[l+32].tv = (0.5f+cosangle*(float)(0.5*0.9))*maxtv;
         staticVertices[l+64].tu = (0.5f+sinangle*0.5f)*maxtu;
         staticVertices[l+64].tv = (0.5f+cosangle*0.5f)*maxtv;
      }

      moverVertices[0][l].x = sinangle*m_d.m_radius + m_d.m_vCenter.x;
      moverVertices[0][l].y = cosangle*m_d.m_radius + m_d.m_vCenter.y;
      moverVertices[0][l].z = height+40.0f;
      moverVertices[0][l+32].x = moverVertices[0][l].x;
      moverVertices[0][l+32].y = moverVertices[0][l].y;
      moverVertices[0][l+32].z = height;

      moverVertices[0][l+64].x = sinangle*outerradius*0.5f + m_d.m_vCenter.x;
      moverVertices[0][l+64].y = cosangle*outerradius*0.5f + m_d.m_vCenter.y;
      moverVertices[0][l+64].z = height+60.0f;

      moverVertices[0][l+96].x = sinangle*outerradius*0.9f + m_d.m_vCenter.x;
      moverVertices[0][l+96].y = cosangle*outerradius*0.9f + m_d.m_vCenter.y;
      moverVertices[0][l+96].z = height+50.0f;

      moverVertices[0][l+128].x = sinangle*outerradius + m_d.m_vCenter.x;
      moverVertices[0][l+128].y = cosangle*outerradius + m_d.m_vCenter.y;
      moverVertices[0][l+128].z = height+40.0f;

      moverVertices[0][l].tu = 0.5f+sinangle*0.5f;
      moverVertices[0][l].tv = 0.5f-cosangle*0.5f;
      moverVertices[0][l+32].tu = 0.5f+sinangle*0.5f;
      moverVertices[0][l+32].tv = 0.5f-cosangle*0.5f;
      moverVertices[0][l+64].tu = 0.5f+sinangle*0.25f;
      moverVertices[0][l+64].tv = 0.5f-cosangle*0.25f;
      moverVertices[0][l+96].tu = 0.5f+sinangle*(float)(0.5*0.9);
      moverVertices[0][l+96].tv = 0.5f-cosangle*(float)(0.5*0.9);
      moverVertices[0][l+128].tu = 0.5f+sinangle*0.5f;
      moverVertices[0][l+128].tv = 0.5f-cosangle*0.5f;

      if ( pin )
      {
         moverVertices[0][l+64].tu = (0.5f+sinangle*0.25f)*maxtu;
         moverVertices[0][l+64].tv = (0.5f+cosangle*0.25f)*maxtv;
         moverVertices[0][l+96].tu = (0.5f+sinangle*(float)(0.5*0.9))*maxtu;
         moverVertices[0][l+96].tv = (0.5f+cosangle*(float)(0.5*0.9))*maxtv;
         moverVertices[0][l+128].tu = (0.5f+sinangle*0.5f)*maxtu;
         moverVertices[0][l+128].tv = (0.5f+cosangle*0.5f)*maxtv;

         const float lightmaxtu = 0.8f;
         const float lightmaxtv = 0.8f;

         moverVertices[0][l].tu2 = moverVertices[0][l].tu;
         moverVertices[0][l+32].tu2 = moverVertices[0][l+32].tu;
         moverVertices[0][l].tv2 = moverVertices[0][l].tv;
         moverVertices[0][l+32].tv2 = moverVertices[0][l+32].tv;
         moverVertices[0][l+64].tu2 = (0.5f+sinangle*0.25f)*lightmaxtu;
         moverVertices[0][l+64].tv2 = (0.5f+cosangle*0.25f)*lightmaxtv;
         moverVertices[0][l+96].tu2 = (0.5f+sinangle*(float)(0.5*0.9))*lightmaxtu;
         moverVertices[0][l+96].tv2 = (0.5f+cosangle*(float)(0.5*0.9))*lightmaxtv;
         moverVertices[0][l+128].tu2 = (0.5f+sinangle*0.5f)*lightmaxtu;
         moverVertices[0][l+128].tv2 = (0.5f+cosangle*0.5f)*lightmaxtv;
      }

      ppin3d->m_lightproject.CalcCoordinates(&moverVertices[0][l],inv_width,inv_height);
      ppin3d->m_lightproject.CalcCoordinates(&moverVertices[0][l+32],inv_width,inv_height);
      ppin3d->m_lightproject.CalcCoordinates(&moverVertices[0][l+64],inv_width,inv_height);
      ppin3d->m_lightproject.CalcCoordinates(&moverVertices[0][l+96],inv_width,inv_height);
      ppin3d->m_lightproject.CalcCoordinates(&moverVertices[0][l+128],inv_width,inv_height);
      memcpy( moverVertices[1], moverVertices[0], sizeof(Vertex3D)*160 );
   }
   SetNormal(staticVertices, rgiBumperStatic, 32, NULL, NULL, 0);
}

void Bumper::RenderStatic(const RenderDevice* _pd3dDevice)
{
   RenderDevice* pd3dDevice=(RenderDevice*)_pd3dDevice;

   // ensure we are not disabled at game start
   m_fDisabled = fFalse;
   if(!m_d.m_fVisible)	return;
   
   int t,k;
   // All this function does is render the bumper image so the black shows through where it's missing in the animated form
   Texture * const pin = m_ptable->GetImage(m_d.m_szImage);	
   if (pin)
   {
      Pin3D *const ppin3d = &g_pplayer->m_pin3d;

      pin->CreateAlphaChannel();
      pin->Set(ePictureTexture);

      pd3dDevice->SetRenderState( RenderDevice::ALPHABLENDENABLE, TRUE);

      staticMaterial.set();

      pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX, staticVertices, 32, (LPWORD)rgiBumperStatic, 32, 0);
      t=0;
      k=0;
      for (int l=0;l<32;l++,t+=6,k+=4)
      {
         SetNormal(staticVertices, &normalIndices[t], 3, NULL, &indices[k], 2);
         SetNormal(&staticVertices[32], &normalIndices[t], 3, NULL, &indices[k], 2);
         SetNormal(staticVertices, &normalIndices[t+3], 3, NULL, &indices[k+2], 2);
         SetNormal(&staticVertices[32], &normalIndices[t+3], 3, NULL, &indices[k+2], 2);

         pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX, staticVertices, 64, (LPWORD)&indices[k], 4, 0);
         pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX, &staticVertices[32], 64, (LPWORD)&indices[k], 4, 0);
      }

      pd3dDevice->SetRenderState( RenderDevice::ALPHABLENDENABLE, FALSE);

      ppin3d->SetTexture(NULL);
   }
}

void Bumper::RenderMovers(const RenderDevice* _pd3dDevice)
{
   RenderDevice* pd3dDevice=(RenderDevice*)_pd3dDevice;
   if(!m_d.m_fVisible)	return;

   int k,t;
   Pin3D * const ppin3d = &g_pplayer->m_pin3d;

   ppin3d->ClearSpriteRectangle(&m_pbumperhitcircle->m_bumperanim, NULL );
   for (int i=0;i<2;i++)	//0 is unlite, while 1 is lite
   {
      ObjFrame * const pof = new ObjFrame();

      ppin3d->ClearSpriteRectangle( NULL, pof );

      ppin3d->ExpandExtents(&pof->rc, moverVertices[i], &m_pbumperhitcircle->m_bumperanim.m_znear, &m_pbumperhitcircle->m_bumperanim.m_zfar, 160, fFalse);

      Texture * const pin = m_ptable->GetImage(m_d.m_szImage);
      if (!pin) // Top solid color
      {
         switch (i)
         {
            case 0:
            {
               ppin3d->SetTexture(NULL);
               topNonLitMaterial.set();
               break;
            }
            case 1:
            {
               ppin3d->lightTexture.Set( ePictureTexture );
               ppin3d->EnableLightMap(fFalse, -1);
               topLitMaterial.set();
            }
            break;
         }

         SetNormal(&moverVertices[i][64], rgiBumperStatic, 32, NULL, NULL, 0);
         pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX, &moverVertices[i][64], 32, (LPWORD)rgiBumperStatic, 32, 0);

         t=0;
         k=0;
         for (int l=0;l<32;l++,t+=6,k+=4)
         {
            SetNormal(&moverVertices[i][64], &normalIndices[t], 3, NULL, &indices[k], 2);
            SetNormal(&moverVertices[i][96], &normalIndices[t], 3, NULL, &indices[k], 2);
            SetNormal(&moverVertices[i][64], &normalIndices[t+3], 3, NULL, &indices[k+2], 2);
            SetNormal(&moverVertices[i][96], &normalIndices[t+3], 3, NULL, &indices[k+2], 2);

            pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX, &moverVertices[i][64], 64, (LPWORD)&indices[k], 4, 0);
            pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX, &moverVertices[i][96], 64, (LPWORD)&indices[k], 4, 0);
         }
      }

      if (m_d.m_fSideVisible)
      {
         const float rside = (m_d.m_sidecolor & 255) * (float) (1.0/255.0);
         const float gside = (m_d.m_sidecolor & 65280) * (float) (1.0/65280.0);
         const float bside = (m_d.m_sidecolor & 16711680) * (float) (1.0/16711680.0);
         // Side color
         switch (i)
         {
            case 0:
            {
               ppin3d->SetTexture(NULL);
               sideNonLitMaterial.set();
               break;
            }
            case 1:
            {
               ppin3d->lightTexture.Set( ePictureTexture );
               ppin3d->EnableLightMap(fFalse, -1);
               sideLitMaterial.set();
               break;
            }
         }

         t=0;
         k=0;
         for (int l=0;l<32;l++,t+=6,k+=4)
         {
            SetNormal(moverVertices[i], &normalIndices[t], 3, NULL, &indices[k], 2);
            SetNormal(moverVertices[i], &normalIndices[t+3], 3, NULL, &indices[k+2], 2);
            pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,moverVertices[i],64,(LPWORD)&indices[k], 4, 0);
         }
      }

      if (pin)
      {
         float maxtu, maxtv;
         m_ptable->GetTVTU(pin, &maxtu, &maxtv);

         pin->EnsureBackdrop(m_d.m_color);
         pin->SetBackDrop( ePictureTexture );

         //pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);
         //pd3dDevice->SetRenderState(D3DRENDERSTATE_COLORKEYENABLE, TRUE);

         //pd3dDevice->SetRenderState(D3DRENDERSTATE_DITHERENABLE, FALSE);

         /*// HACK!!! I think this is the last place we ever use COLORKEY instead
         // of Alpha transparency.  We do this because the D3D software
         // rasterizer can only handle transparency on the first texture,
         // but we need to layer textures to light up the bitmap provided
         // by the user.  We could solve this by creating a bitmap
         // with the background color swapped out in a pre-processing step.*/

         switch (i)
         {
            case 0:
            {
               ppin3d->EnableLightMap(fFalse, -1);
               nonLitMaterial.set();
               break;
            }

            case 1:
            {
               ppin3d->lightTexture.Set( eLightProject1 );
               litMaterial.set();
               break;           
            }
         }

         SetNormal(&moverVertices[i][64], rgiBumperStatic, 32, NULL, NULL, 0);
         pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX, &moverVertices[i][64], 32, (LPWORD)rgiBumperStatic, 32, 0);

         t=0;
         k=0;
         for (int l=0;l<32;l++,t+=6,k+=4)
         {
            SetNormal(&moverVertices[i][64], &normalIndices[t], 3, NULL, &indices[k], 2);
            SetNormal(&moverVertices[i][96], &normalIndices[t], 3, NULL, &indices[k], 2);
            SetNormal(&moverVertices[i][64], &normalIndices[t+3], 3, NULL, &indices[k+2], 2);
            SetNormal(&moverVertices[i][96], &normalIndices[t+3], 3, NULL, &indices[k+2], 2);

            pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX, &moverVertices[i][64], 64, &indices[k], 4, 0);
            pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX, &moverVertices[i][96], 64, &indices[k], 4, 0);
         }

         // Reset all the texture coordinates
         if (i == 1)
         {
            ppin3d->EnableLightMap(fFalse, -1);
         }
      }

      ppin3d->CreateAndCopySpriteBuffers( &m_pbumperhitcircle->m_bumperanim, pof );
      m_pbumperhitcircle->m_bumperanim.m_pobjframe[i] = pof;
   }

   ppin3d->SetTexture(NULL);
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
   //HRESULT hr;

   BiffWriter bw(pstm, hcrypthash, hcryptkey);

#ifdef VBA
   bw.WriteInt(FID(PIID), ApcProjectItem.ID());
#endif
   bw.WriteStruct(FID(VCEN), &m_d.m_vCenter, sizeof(Vertex2D));
   bw.WriteFloat(FID(RADI), m_d.m_radius);
   bw.WriteBool(FID(TMON), m_d.m_tdr.m_fTimerEnabled);
   bw.WriteInt(FID(TMIN), m_d.m_tdr.m_TimerInterval);
   bw.WriteFloat(FID(THRS), m_d.m_threshold);
   bw.WriteFloat(FID(FORC), m_d.m_force);
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
   {
      m_d.m_sidecolor = m_d.m_color; // So tables look like they did before
   }

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
      //		if (!(m_d.m_color & MINBLACKMASK)) {m_d.m_color |= MINBLACK;}	// set minimum black
   }
   else if (id == FID(SCLR))
   {
      pbr->GetInt(&m_d.m_sidecolor);
      //		if (!(m_d.m_sidecolor & MINBLACKMASK)) {m_d.m_sidecolor |= MINBLACK;}	// set minimum black
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
      {
         setLightState(newVal);
      }
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
      // Restart the sequence
      // BUG - merge with code in player for light blinking someday
      const char cold = m_rgblinkpattern[m_iblinkframe];
      m_iblinkframe = 0;
      const char cnew = m_rgblinkpattern[m_iblinkframe];
      if (cold != cnew)
          DrawFrame(cnew == '1');
      m_timenextblink = g_pplayer->m_timeCur + m_blinkinterval;
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
      m_timenextblink = g_pplayer->m_timeCur + m_blinkinterval;

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
         if (lastState == LightStateBlinking)
         {
            // must not be blinking anymore
            g_pplayer->m_vblink.RemoveElement((IBlink *)this);
         }
         else if (m_realState == LightStateBlinking)
         {
            // must be blinking now
            g_pplayer->m_vblink.AddElement((IBlink *)this);
            m_timenextblink = g_pplayer->m_timeCur; // Start pattern right away // + m_d.m_blinkinterval;
         }

         switch (m_realState)
         {
         case LightStateOff:
            DrawFrame(fFalse);
            break;

         case LightStateOn:
            DrawFrame(fTrue);
            break;

         case LightStateBlinking:
            // We know we can't be on the list already because we make sure our state has changed
            m_iblinkframe = 0; // reset pattern
            break;
         }
      }
   }
}
