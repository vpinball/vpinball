#include "StdAfx.h"

Spinner::Spinner()
{
   m_phitspinner = NULL;
   staticMaterial.setDiffuse( 0.0f, 0.6f, 0.6f, 0.6f );
   staticMaterial.setAmbient( 0.0f, 0.6f, 0.6f, 0.6f );
   staticMaterial.setSpecular( 0.0f, 0.0f, 0.0f, 0.0f );
   staticMaterial.setEmissive( 0.0f, 0.0f, 0.0f, 0.0f );
   staticMaterial.setPower( 0.0f );
   moverVertices = 0;
}

Spinner::~Spinner()
{
	if(moverVertices)
		delete [] moverVertices;
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
   char strTmp[40];

   sprintf_s(strTmp, 40, "%f", m_d.m_length);
   SetRegValue("DefaultProps\\Spinner","Length", REG_SZ, &strTmp,strlen(strTmp));
   sprintf_s(strTmp, 40, "%f", m_d.m_rotation);
   SetRegValue("DefaultProps\\Spinner","Rotation", REG_SZ, &strTmp,strlen(strTmp));
   SetRegValue("DefaultProps\\Spinner","Supports",REG_DWORD,&m_d.m_fSupports,4);
   sprintf_s(strTmp, 40, "%f", m_d.m_height);
   SetRegValue("DefaultProps\\Spinner","Height", REG_SZ, &strTmp,strlen(strTmp));
   sprintf_s(strTmp, 40, "%f", m_d.m_overhang);
   SetRegValue("DefaultProps\\Spinner","Overhang", REG_SZ, &strTmp,strlen(strTmp));
   SetRegValue("DefaultProps\\Spinner","Color", REG_DWORD, &m_d.m_color, 4);
   SetRegValue("DefaultProps\\Spinner","CastsShadow",REG_DWORD,&m_d.m_fCastsShadow,4);
   sprintf_s(strTmp, 40, "%f", m_d.m_angleMax);
   SetRegValue("DefaultProps\\Spinner","AngleMax", REG_SZ, &strTmp,strlen(strTmp));
   sprintf_s(strTmp, 40, "%f", m_d.m_angleMin);
   SetRegValue("DefaultProps\\Spinner","AngleMin", REG_SZ, &strTmp,strlen(strTmp));
   sprintf_s(strTmp, 40, "%f", m_d.m_elasticity);
   SetRegValue("DefaultProps\\Spinner","Elasticity", REG_SZ, &strTmp,strlen(strTmp));
   sprintf_s(strTmp, 40, "%f", m_d.m_friction);
   SetRegValue("DefaultProps\\Spinner","Friction", REG_SZ, &strTmp,strlen(strTmp));
   sprintf_s(strTmp, 40, "%f", m_d.m_scatter);
   SetRegValue("DefaultProps\\Spinner","Scatter", REG_SZ, &strTmp,strlen(strTmp));
   SetRegValue("DefaultProps\\Spinner","Animations", REG_DWORD, &m_d.m_animations, 4);
   SetRegValue("DefaultProps\\Spinner","Visible",REG_DWORD,&m_d.m_fVisible,4);
   SetRegValue("DefaultProps\\Spinner","TimerEnabled",REG_DWORD,&m_d.m_tdr.m_fTimerEnabled,4);
   SetRegValue("DefaultProps\\Spinner","TimerInterval", REG_DWORD, &m_d.m_tdr.m_TimerInterval, 4);
   SetRegValue("DefaultProps\\Spinner","ImageFront", REG_SZ, &m_d.m_szImageFront,strlen(m_d.m_szImageFront));
   SetRegValue("DefaultProps\\Spinner","ImageBack", REG_SZ, &m_d.m_szImageBack,strlen(m_d.m_szImageBack));
   SetRegValue("DefaultProps\\Spinner","Surface", REG_SZ, &m_d.m_szSurface,strlen(m_d.m_szSurface));
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

   hr = GetRegStringAsFloat("DefaultProps\\Spinner","Friction", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_friction = fTmp;
   else
      m_d.m_friction = 0;	//zero uses global value

   hr = GetRegStringAsFloat("DefaultProps\\Spinner","Scatter", &fTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_scatter = fTmp;
   else
      m_d.m_scatter = 0;	//zero uses global value

   hr = GetRegInt("DefaultProps\\Spinner","Animations", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_animations = iTmp;
   else
      m_d.m_animations = 0;	// manual selection of the animations frame count

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

   if (m_phitspinner) // Failed Player case
   {
      for (int i=0;i<m_phitspinner->m_spinneranim.m_vddsFrame.Size();i++)
      {
         delete m_phitspinner->m_spinneranim.m_vddsFrame.ElementAt(i);
      }

      m_phitspinner = NULL;
   }
}

void Spinner::PostRenderStatic(const RenderDevice* pd3dDevice)
{
}

static const WORD rgiSpinner0[8] = {0,1,2,3,6,7,4,5};
static const WORD rgiSpinner1[8] = {4,5,6,7,2,3,0,1};
static const WORD rgiSpinnerNormal[3] = {0,1,3};

static const WORD rgiSpinner2[4] = {0,1,5,4};
static const WORD rgiSpinner3[4] = {2,6,7,3};
static const WORD rgiSpinner4[4] = {0,2,3,1};
static const WORD rgiSpinner5[4] = {4,5,7,6};
static const WORD rgiSpinner6[4] = {0,4,6,2};
static const WORD rgiSpinner7[4] = {1,3,7,5};

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
      ppin3d->m_lightproject.CalcCoordinates(&staticVertices[l]);
   }

}

void Spinner::PrepareMovers( RenderDevice* pd3dDevice )
{
   Pin3D * const ppin3d = &g_pplayer->m_pin3d;

   COLORREF rgbTransparent = RGB(255,0,255); //RGB(0,0,0);

   const float height = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_vCenter.x, m_d.m_vCenter.y);
   const float h = (m_d.m_height*0.5f + 30.0f);

   Texture* const pinback = m_ptable->GetImage(m_d.m_szImageBack);
   Texture* const pinfront = m_ptable->GetImage(m_d.m_szImageFront);

   float maxtuback, maxtvback;
   float maxtufront, maxtvfront;
   if (pinback)
   {
      m_ptable->GetTVTU(pinback, &maxtuback, &maxtvback);
   }
   else
   {
      maxtuback = maxtvback = 1.0f;
   }

   if (pinfront)
   {
      m_ptable->GetTVTU(pinfront, &maxtufront, &maxtvfront);
   }
   else
   {
      maxtufront = maxtvfront = 1.0f;
   }

   int cframes;
   if (m_d.m_animations > 0) cframes = m_d.m_animations;
   else if (m_d.m_angleMax != m_d.m_angleMin)
   {
      cframes = (int)((m_d.m_angleMax - m_d.m_angleMin)*(float)((20-1)/90.0) + 1.5f); // 15 frames per 90 degrees
   }
   else cframes = 80;

   const float halflength = m_d.m_length * 0.5f;
   const float halfwidth = m_d.m_height * 0.5f;

   ppin3d->ClearExtents(&m_phitspinner->m_spinneranim.m_rcBounds, &m_phitspinner->m_spinneranim.m_znear, &m_phitspinner->m_spinneranim.m_zfar);

   const float r = (float)(m_d.m_color & 255) * (float)(1.0/255.0);
   const float g = (float)(m_d.m_color & 65280) * (float)(1.0/65280.0);
   const float b = (float)(m_d.m_color & 16711680) * (float)(1.0/16711680.0);

   const float inv_cframes = 1.0f/(float)cframes;

   frameCount=cframes;
   if(moverVertices)
	   delete [] moverVertices;
   moverVertices = new Vertex3D[8*frameCount];
   int ofs=0;
   for ( int i=0;i<cframes;i++,ofs+=8 )
   {
      float angle;
      if (m_d.m_angleMax != m_d.m_angleMin)
         angle = ANGTORAD(m_d.m_angleMin + (m_d.m_angleMax - m_d.m_angleMin)*inv_cframes*(float)i);
      else angle = (float)(2.0*M_PI)*inv_cframes*(float)i;

      const float radangle = ANGTORAD(m_d.m_rotation);
      const float snY = sinf(radangle);
      const float csY = cosf(radangle);

      const float snTurn = sinf(angle);
      const float csTurn = cosf(angle);

      const float minx = -halflength;
      const float maxx = halflength;
      const float miny = -3.0f;
      const float maxy = 3.0f;
      const float minz = -halfwidth;
      const float maxz = halfwidth;

      Vertex3D *rgv3D=&moverVertices[ofs];
      for (int l=0;l<8;l++)
      {
         rgv3D[l].x = (l & 1) ? maxx : minx;
         rgv3D[l].y = (l & 2) ? maxy : miny;
         rgv3D[l].z = (l & 4) ? maxz : minz;

         if (l & 2)
         {
            rgv3D[l].tu = (l & 1) ? maxtufront : 0;
            rgv3D[l].tv = (l & 4) ? 0 : maxtvfront;
         }
         else
         {
            rgv3D[l].tu = (l & 1) ? maxtuback : 0;
            rgv3D[l].tv = (l & 4) ? maxtvback : 0;
         }
      }

      for (int l=0;l<8;l++)
      {
         {
            const float temp = rgv3D[l].y;
            rgv3D[l].y = csTurn*temp + snTurn*rgv3D[l].z;
            rgv3D[l].z = csTurn*rgv3D[l].z - snTurn*temp;
         }

         {
            const float temp = rgv3D[l].x;
            rgv3D[l].x = csY*temp - snY*rgv3D[l].y;
            rgv3D[l].y = csY*rgv3D[l].y + snY*temp;
         }

         rgv3D[l].x += m_d.m_vCenter.x;
         rgv3D[l].y += m_d.m_vCenter.y;
         //rgv3D[l].z += 60.0f + height;
         rgv3D[l].z += h + height;
         rgv3D[l].z *= m_ptable->m_zScale;

         ppin3d->m_lightproject.CalcCoordinates(&rgv3D[l]);
      }

      if (pinback)
      {			
         pinback->CreateAlphaChannel();
      }

      if (pinfront)
      {
         pinfront->CreateAlphaChannel();
      }
   }

}
void Spinner::RenderSetup(const RenderDevice* _pd3dDevice)
{
   PrepareStatic( (RenderDevice*)_pd3dDevice );
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
   PrepareMovers( (RenderDevice*)_pd3dDevice );
}

void Spinner::RenderStatic(const RenderDevice* _pd3dDevice)
{
   if(!m_d.m_fSupports) return;

   RenderDevice* pd3dDevice = (RenderDevice*)_pd3dDevice;

   Pin3D * const ppin3d = &g_pplayer->m_pin3d;

   staticMaterial.set();
   Vertex3D rgv3D[8];
   memcpy( rgv3D, staticVertices, sizeof(Vertex3D)*8);

   SetNormal(rgv3D, rgiSpinnerNormal, 3, rgv3D, rgiSpinner0, 8);
   pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, MY_D3DFVF_VERTEX, rgv3D, 8, (LPWORD)rgiSpinner0, 8, 0);

   SetNormal(rgv3D, rgiSpinnerNormal, 3, rgv3D, rgiSpinner1, 8);
   pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, MY_D3DFVF_VERTEX, rgv3D, 8, (LPWORD)rgiSpinner1, 8, 0);
}

void Spinner::RenderMovers(const RenderDevice* _pd3dDevice)
{
   Pin3D * const ppin3d = &g_pplayer->m_pin3d;
   RenderDevice *pd3dDevice = (RenderDevice*)_pd3dDevice;

   COLORREF rgbTransparent = RGB(255,0,255); //RGB(0,0,0);

   Texture * const pinback = m_ptable->GetImage(m_d.m_szImageBack);
   Texture * const pinfront = m_ptable->GetImage(m_d.m_szImageFront);

   if (g_pvp->m_pdd.m_fHardwareAccel)
   {
      pd3dDevice->SetRenderState(RenderDevice::ALPHAREF, 0x80);
      pd3dDevice->SetRenderState(RenderDevice::ALPHAFUNC, D3DCMP_GREATER);
      pd3dDevice->SetRenderState(RenderDevice::ALPHATESTENABLE, TRUE);
   }

   // Set texture to mirror, so the alpha state of the texture blends correctly to the outside
   pd3dDevice->SetTextureStageState( ePictureTexture, D3DTSS_ADDRESS, D3DTADDRESS_MIRROR);

   ppin3d->ClearSpriteRectangle( &m_phitspinner->m_spinneranim, NULL );

   int ofs=0;
   for (int i=0;i<frameCount;i++,ofs+=8)
   {
      ObjFrame * const pof = new ObjFrame();

      Vertex3D rgv3D[8];
      memcpy( rgv3D, &moverVertices[ofs], sizeof(Vertex3D)*8);

      ppin3d->ClearSpriteRectangle( NULL, pof );
      ppin3d->ExpandExtents(&pof->rc, rgv3D, &m_phitspinner->m_spinneranim.m_znear, &m_phitspinner->m_spinneranim.m_zfar, 8, fFalse);

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
         else pd3dDevice->SetRenderState(RenderDevice::CULLMODE, D3DCULL_NONE);

         g_pplayer->m_pin3d.SetColorKeyEnabled(TRUE);
         pd3dDevice->SetRenderState(RenderDevice::ZWRITEENABLE, TRUE);
         g_pplayer->m_pin3d.SetTextureFilter ( ePictureTexture, TEXTURE_MODE_TRILINEAR );
         textureMaterial.set();
      }
      else // No image by that name
      {
         ppin3d->SetTexture(NULL);
         solidMaterial.set();
      }

      SetNormal(rgv3D, rgiSpinner2, 4, NULL, NULL, 0);
      pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,rgv3D, 6,(LPWORD)rgiSpinner2,4, 0);

      // Draw Frontside
      if (pinfront)
      {
         pinfront->Set( ePictureTexture );
         if (pinfront->m_fTransparent)
         {				
            pd3dDevice->SetRenderState(RenderDevice::ALPHABLENDENABLE, FALSE);	
            if (m_d.m_color != rgbTransparent)rgbTransparent = pinfront->m_rgbTransparent;
         }
         else 
         {	
            g_pplayer->m_pin3d.EnableAlphaBlend( 1, fFalse );
         }

         if (m_d.m_color == rgbTransparent || m_d.m_color == NOTRANSCOLOR) 
            pd3dDevice->SetRenderState(RenderDevice::CULLMODE, D3DCULL_CCW);
         else pd3dDevice->SetRenderState(RenderDevice::CULLMODE, D3DCULL_NONE);

         g_pplayer->m_pin3d.SetColorKeyEnabled(TRUE);
         pd3dDevice->SetRenderState(RenderDevice::ZWRITEENABLE, TRUE);
         g_pplayer->m_pin3d.SetTextureFilter ( ePictureTexture, TEXTURE_MODE_TRILINEAR );
         textureMaterial.set();
      }
      else // No image by that name
      {
         ppin3d->SetTexture(NULL);
         solidMaterial.set();
      }
      SetNormal(rgv3D, rgiSpinner3, 4, NULL, NULL, 0);
      pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,rgv3D, 8,(LPWORD)rgiSpinner3, 4, 0);

      solidMaterial.set();
      ppin3d->SetTexture(NULL);

      if (m_d.m_color != rgbTransparent && m_d.m_color != NOTRANSCOLOR)
      {
         Vertex3D verts[16];
         static const WORD idx[24] = {0,1,2,0,1,2, 4,5,6,4,6,7, 8,9,10,8,10,11, 12,13,14,12,14,15 };
         // Top & Bottom
         SetNormal(rgv3D, rgiSpinner4, 4, NULL, NULL, 0);
         memcpy( &verts[0 ], &rgv3D[ rgiSpinner4[0]], sizeof(Vertex3D));
         memcpy( &verts[1 ], &rgv3D[ rgiSpinner4[1]], sizeof(Vertex3D));
         memcpy( &verts[2 ], &rgv3D[ rgiSpinner4[2]], sizeof(Vertex3D));
         memcpy( &verts[3 ], &rgv3D[ rgiSpinner4[3]], sizeof(Vertex3D));

         SetNormal(rgv3D, rgiSpinner5, 4, NULL, NULL, 0);
         memcpy( &verts[4 ], &rgv3D[ rgiSpinner5[0]], sizeof(Vertex3D));
         memcpy( &verts[5 ], &rgv3D[ rgiSpinner5[1]], sizeof(Vertex3D));
         memcpy( &verts[6 ], &rgv3D[ rgiSpinner5[2]], sizeof(Vertex3D));
         memcpy( &verts[7 ], &rgv3D[ rgiSpinner5[3]], sizeof(Vertex3D));
         // Sides
         SetNormal(rgv3D, rgiSpinner6, 4, NULL, NULL, 0);
         memcpy( &verts[8 ], &rgv3D[ rgiSpinner6[0]], sizeof(Vertex3D));
         memcpy( &verts[9 ], &rgv3D[ rgiSpinner6[1]], sizeof(Vertex3D));
         memcpy( &verts[10], &rgv3D[ rgiSpinner6[2]], sizeof(Vertex3D));
         memcpy( &verts[11], &rgv3D[ rgiSpinner6[3]], sizeof(Vertex3D));

         SetNormal(rgv3D, rgiSpinner7, 4, NULL, NULL, 0);
         memcpy( &verts[12], &rgv3D[ rgiSpinner7[0]], sizeof(Vertex3D));
         memcpy( &verts[13], &rgv3D[ rgiSpinner7[1]], sizeof(Vertex3D));
         memcpy( &verts[14], &rgv3D[ rgiSpinner7[2]], sizeof(Vertex3D));
         memcpy( &verts[15], &rgv3D[ rgiSpinner7[3]], sizeof(Vertex3D));
         pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, MY_D3DFVF_VERTEX,verts, 16,(LPWORD)idx, 24, 0);
      }

      ppin3d->CreateAndCopySpriteBuffers( &m_phitspinner->m_spinneranim, pof );
      m_phitspinner->m_spinneranim.m_vddsFrame.AddElement(pof);
   }

   g_pplayer->m_pin3d.SetColorKeyEnabled(FALSE);
   pd3dDevice->SetRenderState(RenderDevice::ALPHATESTENABLE, FALSE);
   pd3dDevice->SetTextureStageState( ePictureTexture, D3DTSS_ADDRESS, D3DTADDRESS_WRAP);
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

#ifdef VBA
   bw.WriteInt(FID(PIID), ApcProjectItem.ID());
#endif
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
   bw.WriteInt(FID(SANM), m_d.m_animations);
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
      //		if (!(m_d.m_color & MINBLACKMASK)) {m_d.m_color |= MINBLACK;}	// set minimum black
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
   else if (id == FID(SANM))
   {
      pbr->GetInt(&m_d.m_animations);
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

      m_d.m_antifriction = 1.0f - newVal*(float)(1.0/100.0);

   if (m_d.m_antifriction < 0)
   {
      m_d.m_antifriction = 0;
   }
   else if (m_d.m_antifriction > 1.0f)
   {
      m_d.m_antifriction = 1.0f;
   }

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

STDMETHODIMP Spinner::get_Animations(int *pVal)
{
   if (!g_pplayer)
   {
      *pVal = m_d.m_animations;
   }

   return S_OK;
}

STDMETHODIMP Spinner::put_Animations(int newVal)
{	
   if (!g_pplayer)
   {
      STARTUNDO
         m_d.m_animations = newVal;
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
