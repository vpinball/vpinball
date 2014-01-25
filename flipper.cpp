#include "StdAfx.h"

Flipper::Flipper()
{
   m_phitflipper = NULL;
   vBufferArray=0;
}

Flipper::~Flipper()
{
}

HRESULT Flipper::Init(PinTable *ptable, float x, float y, bool fromMouseClick)
{
   m_ptable = ptable;

   m_d.m_Center.x = x;
   m_d.m_Center.y = y;

   SetDefaults(fromMouseClick);

   m_phitflipper = NULL;

   return InitVBA(fTrue, 0, NULL);
}

void Flipper::SetDefaults(bool fromMouseClick)
{
   HRESULT hr;
   float fTmp;
   int iTmp;

   hr = GetRegStringAsFloat("DefaultProps\\Flipper","StartAngle", &fTmp);
   m_d.m_StartAngle = (hr == S_OK) && fromMouseClick ? fTmp : 121;

   hr = GetRegStringAsFloat("DefaultProps\\Flipper","EndAngle", &fTmp);
   m_d.m_EndAngle = (hr == S_OK) && fromMouseClick ? fTmp : 70;

   hr = GetRegStringAsFloat("DefaultProps\\Flipper","BaseRadius", &fTmp);
   m_d.m_BaseRadius = (hr == S_OK) && fromMouseClick ? fTmp : 21.5f;		// 15

   hr = GetRegStringAsFloat("DefaultProps\\Flipper","EndRadius", &fTmp);
   m_d.m_EndRadius = (hr == S_OK) && fromMouseClick ? fTmp : 13.f;		// 6

   hr = GetRegStringAsFloat("DefaultProps\\Flipper","Length", &fTmp);
   m_d.m_FlipperRadiusMax = (hr == S_OK) && fromMouseClick ? fTmp : 125.f;	// 80

   hr = GetRegStringAsFloat("DefaultProps\\Flipper","MaxDifLength", &fTmp);
   m_d.m_FlipperRadiusMin = (hr == S_OK) && fromMouseClick ? fTmp : 0;

   m_d.m_FlipperRadius = m_d.m_FlipperRadiusMax;

   hr = GetRegStringAsFloat("DefaultProps\\Flipper","RecoilVelocity", &fTmp);
   m_d.m_recoil = (hr == S_OK) && fromMouseClick ? fTmp : 2;		// disabled

   m_d.m_angleEOS = 0;		//disabled

   hr = GetRegStringAsFloat("DefaultProps\\Flipper","ReturnStrength", &fTmp);
   m_d.m_return = (hr == S_OK) && fromMouseClick ? fTmp : 0.09f;		// match existing physics, return equals stroke 

   hr = GetRegStringAsFloat("DefaultProps\\Flipper","Speed", &fTmp);
   m_d.m_force = (hr == S_OK) && fromMouseClick ? fTmp : 0.15f;

   hr = GetRegStringAsFloat("DefaultProps\\Flipper","Elasticity", &fTmp);
   m_d.m_elasticity = (hr == S_OK) && fromMouseClick ? fTmp : 0.55f;

   m_d.m_OverridePhysics = 0;

   //hr = GetRegStringAsFloat("DefaultProps\\Flipper","Friction", &fTmp);
   //if (hr == S_OK)
   //	m_d.m_friction = fTmp;
   //else
   m_d.m_friction = 0;	//zero uses global value

   m_d.m_scatter = 0.0;	//zero uses global value

   hr = GetRegInt("DefaultProps\\Flipper","TimerEnabled", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_tdr.m_fTimerEnabled = iTmp == 0 ? false : true;
   else
      m_d.m_tdr.m_fTimerEnabled = false;

   hr = GetRegInt("DefaultProps\\Flipper","TimerInterval", &iTmp);
   m_d.m_tdr.m_TimerInterval = (hr == S_OK) && fromMouseClick ? iTmp : 100;

   hr = GetRegInt("DefaultProps\\Flipper","Color", &iTmp);
   m_d.m_color = (hr == S_OK) && fromMouseClick ? iTmp : RGB(255,255,255);

   hr = GetRegInt("DefaultProps\\Flipper","RubberColor", &iTmp);
   m_d.m_rubbercolor = (hr == S_OK) && fromMouseClick ? iTmp : RGB(128,50,50);

   hr = GetRegString("DefaultProps\\Flipper", "Surface", &m_d.m_szSurface, MAXTOKEN);
   if ((hr != S_OK) || !fromMouseClick)
      m_d.m_szSurface[0] = 0;

   hr = GetRegStringAsFloat("DefaultProps\\Flipper","Strength", &fTmp);
   m_d.m_strength = (hr == S_OK) && fromMouseClick ? fTmp : 3.0f;

   hr = GetRegStringAsFloat("DefaultProps\\Flipper","PowerLaw", &fTmp);
   m_d.m_powerlaw = (hr == S_OK) && fromMouseClick ? fTmp : 1.0f;

   hr = GetRegStringAsFloat("DefaultProps\\Flipper","ObliqueCorrection", &fTmp);
   m_d.m_obliquecorrection = (hr == S_OK) && fromMouseClick ? fTmp : ANGTORAD(3.0f); //flipper face correction 

   hr = GetRegStringAsFloat("DefaultProps\\Flipper","ScatterAngle", &fTmp);
   m_d.m_scatterangle = ANGTORAD((hr == S_OK) && fromMouseClick ? fTmp : -11.0f); //flipper scatter angle

   hr = GetRegStringAsFloat("DefaultProps\\Flipper","Height", &fTmp);
   m_d.m_height = (hr == S_OK) && fromMouseClick ? fTmp : 50;

   hr = GetRegInt("DefaultProps\\Flipper","RubberThickness", &iTmp);
   m_d.m_rubberthickness = (hr == S_OK) && fromMouseClick ? iTmp : 7;

   hr = GetRegInt("DefaultProps\\Flipper","RubberHeight", &iTmp);
   m_d.m_rubberheight = (hr == S_OK) && fromMouseClick ? iTmp : 19;

   hr = GetRegInt("DefaultProps\\Flipper","RubberWidth", &iTmp);
   m_d.m_rubberwidth = (hr == S_OK) && fromMouseClick ? iTmp : 24;

   m_d.m_mass = 1;

   hr = GetRegInt("DefaultProps\\Flipper","Visible", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_fVisible = iTmp == 0 ? false : true;
   else
      m_d.m_fVisible = fTrue;

   hr = GetRegInt("DefaultProps\\Flipper","Enabled", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_fEnabled = iTmp == 0 ? false : true;
   else
      m_d.m_fEnabled = fTrue;
}

void Flipper::WriteRegDefaults()
{
   char strTmp[MAXTOKEN];

   sprintf_s(strTmp, 40, "%f", m_d.m_BaseRadius);
   SetRegValue("DefaultProps\\Flipper","BaseRadius", REG_SZ, &strTmp,strlen(strTmp));
   sprintf_s(strTmp, 40, "%f", m_d.m_EndRadius);
   SetRegValue("DefaultProps\\Flipper","EndRadius", REG_SZ, &strTmp,strlen(strTmp));
   sprintf_s(strTmp, 40, "%f", m_d.m_StartAngle);
   SetRegValue("DefaultProps\\Flipper","StartAngle", REG_SZ, &strTmp,strlen(strTmp));
   sprintf_s(strTmp, 40, "%f", m_d.m_EndAngle);
   SetRegValue("DefaultProps\\Flipper","EndAngle", REG_SZ, &strTmp,strlen(strTmp));
   sprintf_s(strTmp, 40, "%f", m_d.m_FlipperRadiusMax);
   SetRegValue("DefaultProps\\Flipper","Length", REG_SZ, &strTmp,strlen(strTmp));
   sprintf_s(strTmp, 40, "%f", m_d.m_FlipperRadiusMin);
   SetRegValue("DefaultProps\\Flipper","MaxDifLength", REG_SZ, &strTmp,strlen(strTmp));
   sprintf_s(strTmp, 40, "%f", m_d.m_recoil);
   SetRegValue("DefaultProps\\Flipper","RecoilVelocity", REG_SZ, &strTmp,strlen(strTmp));
   sprintf_s(strTmp, 40, "%f", m_d.m_return);
   SetRegValue("DefaultProps\\Flipper","ReturnStrength", REG_SZ, &strTmp,strlen(strTmp));
   sprintf_s(strTmp, 40, "%f", m_d.m_force);
   SetRegValue("DefaultProps\\Flipper","Speed", REG_SZ, &strTmp,strlen(strTmp));
   sprintf_s(strTmp, 40, "%f", m_d.m_elasticity);
   SetRegValue("DefaultProps\\Flipper","Elasticity", REG_SZ, &strTmp,strlen(strTmp));
   sprintf_s(strTmp, 40, "%f", RADTOANG(m_d.m_scatterangle));
   SetRegValue("DefaultProps\\Flipper","ScatterAngle", REG_SZ, &strTmp,strlen(strTmp));
   SetRegValue("DefaultProps\\Flipper","TimerEnabled",REG_DWORD,&m_d.m_tdr.m_fTimerEnabled,4);
   SetRegValue("DefaultProps\\Flipper","TimerInterval", REG_DWORD, &m_d.m_tdr.m_TimerInterval, 4);
   SetRegValue("DefaultProps\\Flipper","Color", REG_DWORD, &m_d.m_color, 4);
   SetRegValue("DefaultProps\\Flipper","RubberColor", REG_DWORD, &m_d.m_rubbercolor, 4);
   SetRegValue("DefaultProps\\Flipper","Surface", REG_SZ, &m_d.m_szSurface,strlen(m_d.m_szSurface));
   sprintf_s(strTmp, 40, "%f", m_d.m_strength);
   SetRegValue("DefaultProps\\Flipper","Strength", REG_SZ, &strTmp,strlen(strTmp));
   sprintf_s(strTmp, 40, "%f", m_d.m_powerlaw);
   SetRegValue("DefaultProps\\Flipper","PowerLaw", REG_SZ, &strTmp,strlen(strTmp));
   sprintf_s(strTmp, 40, "%f", m_d.m_obliquecorrection);
   SetRegValue("DefaultProps\\Flipper","ObliqueCorrection", REG_SZ, &strTmp,strlen(strTmp));
   sprintf_s(strTmp, 40, "%f", m_d.m_height);
   SetRegValue("DefaultProps\\Flipper","Height", REG_SZ, &strTmp,strlen(strTmp));
   SetRegValue("DefaultProps\\Flipper","RubberThickness", REG_DWORD, &m_d.m_rubberthickness, 4);
   SetRegValue("DefaultProps\\Flipper","RubberHeight", REG_DWORD, &m_d.m_rubberheight, 4);
   SetRegValue("DefaultProps\\Flipper","RubberWidth", REG_DWORD, &m_d.m_rubberwidth, 4);
   SetRegValue("DefaultProps\\Flipper","Visible",REG_DWORD,&m_d.m_fVisible,4);
   SetRegValue("DefaultProps\\Flipper","Enabled",REG_DWORD,&m_d.m_fEnabled,4);
}


void Flipper::GetTimers(Vector<HitTimer> * const pvht)
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

void Flipper::GetHitShapes(Vector<HitObject> * const pvho)
{
   if(m_d.m_OverridePhysics)
   {
	     char tmp[256];
		 m_d.m_OverrideSpeed = 0.15f;
	     sprintf_s(tmp,256,"FlipperPhysicsSpeed%u",m_d.m_OverridePhysics-1);
         HRESULT hr = GetRegStringAsFloat("Player", tmp, &m_d.m_OverrideSpeed);
         if (hr != S_OK)
            m_d.m_OverrideSpeed = 0.15f;

		 m_d.m_OverrideStrength = 3.f;
	     sprintf_s(tmp,256,"FlipperPhysicsStrength%u",m_d.m_OverridePhysics-1);
         hr = GetRegStringAsFloat("Player", tmp, &m_d.m_OverrideStrength);
         if (hr != S_OK)
            m_d.m_OverrideStrength = 3.f;

 		 m_d.m_OverrideElasticity = 0.55f;
	     sprintf_s(tmp,256,"FlipperPhysicsElasticity%u",m_d.m_OverridePhysics-1);
         hr = GetRegStringAsFloat("Player", tmp, &m_d.m_OverrideElasticity);
         if (hr != S_OK)
            m_d.m_OverrideElasticity = 0.55f;

  		 m_d.m_OverrideScatter = -11.f;
	     sprintf_s(tmp,256,"FlipperPhysicsScatter%u",m_d.m_OverridePhysics-1);
         hr = GetRegStringAsFloat("Player", tmp, &m_d.m_OverrideScatter);
         if (hr != S_OK)
            m_d.m_OverrideScatter = -11.f;
		 m_d.m_OverrideScatter = ANGTORAD(m_d.m_OverrideScatter);

  		 m_d.m_OverrideReturnStrength = 0.09f;
	     sprintf_s(tmp,256,"FlipperPhysicsReturnStrength%u",m_d.m_OverridePhysics-1);
         hr = GetRegStringAsFloat("Player", tmp, &m_d.m_OverrideReturnStrength);
         if (hr != S_OK)
            m_d.m_OverrideReturnStrength = 0.09f;

		 m_d.m_OverrideRecoil = 2.f;
	     sprintf_s(tmp,256,"FlipperPhysicsRecoil%u",m_d.m_OverridePhysics-1);
         hr = GetRegStringAsFloat("Player", tmp, &m_d.m_OverrideRecoil);
         if (hr != S_OK)
            m_d.m_OverrideRecoil = 2.f;

  		 m_d.m_OverridePowerLaw = 1.f;
	     sprintf_s(tmp,256,"FlipperPhysicsPowerLaw%u",m_d.m_OverridePhysics-1);
         hr = GetRegStringAsFloat("Player", tmp, &m_d.m_OverridePowerLaw);
         if (hr != S_OK)
            m_d.m_OverridePowerLaw = 1.f;

		 m_d.m_OverrideOblique = 3.f;
	     sprintf_s(tmp,256,"FlipperPhysicsOblique%u",m_d.m_OverridePhysics-1);
         hr = GetRegStringAsFloat("Player", tmp, &m_d.m_OverrideOblique);
         if (hr != S_OK)
            m_d.m_OverrideOblique = 3.f;
		 m_d.m_OverrideOblique = ANGTORAD(m_d.m_OverrideOblique);
   }

   //

   const float height = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_Center.x, m_d.m_Center.y);

   if (m_d.m_FlipperRadiusMin > 0.f && m_d.m_FlipperRadiusMax > m_d.m_FlipperRadiusMin)
   {
      m_d.m_FlipperRadius = m_d.m_FlipperRadiusMax - (m_d.m_FlipperRadiusMax - m_d.m_FlipperRadiusMin) * m_ptable->m_globalDifficulty;
      m_d.m_FlipperRadius = max(m_d.m_FlipperRadius, m_d.m_BaseRadius - m_d.m_EndRadius +0.05f);
   }
   else m_d.m_FlipperRadius = m_d.m_FlipperRadiusMax;

   HitFlipper * const phf = new HitFlipper(m_d.m_Center.x, m_d.m_Center.y, m_d.m_BaseRadius, m_d.m_EndRadius,
	   m_d.m_FlipperRadius, ANGTORAD(m_d.m_StartAngle), height, height + m_d.m_height, (m_d.m_OverridePhysics ? m_d.m_OverrideStrength : m_d.m_strength), m_d.m_mass);

   phf->m_elasticity = (m_d.m_OverridePhysics ? m_d.m_OverrideElasticity : m_d.m_elasticity);
   phf->m_antifriction = 1.0f - m_d.m_friction;	//antifriction
   phf->m_scatter = m_d.m_scatter;

   phf->m_flipperanim.m_EnableRotateEvent = 0;

   phf->m_pfe = NULL;

   phf->m_flipperanim.m_frameStart = ANGTORAD(m_d.m_StartAngle);
   phf->m_flipperanim.m_frameEnd = ANGTORAD(m_d.m_EndAngle);
   phf->m_flipperanim.m_angleMin = min(phf->m_flipperanim.m_frameStart, phf->m_flipperanim.m_frameEnd);
   phf->m_flipperanim.m_angleMax = max(phf->m_flipperanim.m_frameStart, phf->m_flipperanim.m_frameEnd);

   phf->m_flipperanim.m_fEnabled = (m_d.m_fEnabled != 0);
   phf->m_flipperanim.m_fVisible = (m_d.m_fVisible != 0);

   pvho->AddElement(phf);
   phf->m_pflipper = this;
   phf->m_flipperanim.m_pflipper = this;
   m_phitflipper = phf;	
}

void Flipper::GetHitShapesDebug(Vector<HitObject> * const pvho)
{
   const float height = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_Center.x, m_d.m_Center.y);

   if (m_d.m_FlipperRadiusMin > 0.f && m_d.m_FlipperRadiusMax > m_d.m_FlipperRadiusMin)
   {
      m_d.m_FlipperRadius = m_d.m_FlipperRadiusMax - ( m_d.m_FlipperRadiusMax - m_d.m_FlipperRadiusMin) * m_ptable->m_globalDifficulty;
      m_d.m_FlipperRadius = max(m_d.m_FlipperRadius, m_d.m_BaseRadius - m_d.m_EndRadius +0.05f);
   }
   else m_d.m_FlipperRadius = m_d.m_FlipperRadiusMax;

   HitObject * const pho = CreateCircularHitPoly(m_d.m_Center.x, m_d.m_Center.y, height + m_d.m_height, m_d.m_FlipperRadius + m_d.m_EndRadius, 32);
   pvho->AddElement(pho);
}

void Flipper::EndPlay()
{
   if (m_phitflipper) // Failed player case
   {
      for (int i=0;i<m_phitflipper->m_flipperanim.m_vddsFrame.Size();i++)
      {
         delete m_phitflipper->m_flipperanim.m_vddsFrame.ElementAt(i);
      }

      m_phitflipper = NULL;
   }
   if ( vBufferArray )
   {
      for( int i=0;i<maxFrames; i++ )
      {
         vBufferArray[i].vbuffer->release();
         vBufferArray[i].vbuffer=0;
      }
      delete[] vBufferArray;
      vBufferArray=0;
   }
   IEditable::EndPlay();
}

void Flipper::SetVertices(const float angle, Vertex2D * const pvEndCenter, Vertex2D * const rgvTangents, const float baseradius, const float endradius) const
{
   const float fradius = m_d.m_FlipperRadius;
   const float fa = asinf((baseradius-endradius)/fradius); //face to centerline angle (center to center)

   const float faceNormOffset = (float)(M_PI/2.0) - fa; //angle of normal when flipper center line at angle zero	

   const float basex = m_d.m_Center.x;
   const float endx = m_d.m_Center.x + fradius*sinf(angle); //place end radius center
   pvEndCenter->x = endx;
   const float basey = m_d.m_Center.y;
   const float endy = m_d.m_Center.y - fradius*cosf(angle);	
   pvEndCenter->y = endy;

   const float faceNormx1 =  sinf(angle - faceNormOffset); // normals to new face positions
   const float faceNormy1 = -cosf(angle - faceNormOffset);
   const float faceNormx2 =  sinf(angle + faceNormOffset); // both faces
   const float faceNormy2 = -cosf(angle + faceNormOffset);

   rgvTangents[0].x = basex + baseradius*faceNormx1;	// endpoints of faces
   rgvTangents[0].y = basey + baseradius*faceNormy1;

   rgvTangents[1].x = endx + endradius*faceNormx1; 
   rgvTangents[1].y = endy + endradius*faceNormy1;

   rgvTangents[3].x = basex + baseradius*faceNormx2; 
   rgvTangents[3].y = basey + baseradius*faceNormy2; 

   rgvTangents[2].x = endx + endradius*faceNormx2;
   rgvTangents[2].y = endy + endradius*faceNormy2;
}

void Flipper::PreRender(Sur * const psur)
{
   const float anglerad = ANGTORAD(m_d.m_StartAngle);
   //const float anglerad2 = ANGTORAD(m_d.m_EndAngle);

   m_d.m_FlipperRadius = m_d.m_FlipperRadiusMax;

   psur->SetFillColor(RGB(192,192,192));
   psur->SetBorderColor(-1,false,0);
   psur->SetLineColor(RGB(0,0,0), false, 0);

   Vertex2D vendcenter;
   Vertex2D rgv[4];
   SetVertices(anglerad, &vendcenter, rgv, m_d.m_BaseRadius, m_d.m_EndRadius);	

   psur->SetObject(this);

   psur->Polygon(rgv, 4);
   psur->Ellipse(m_d.m_Center.x, m_d.m_Center.y, m_d.m_BaseRadius);
   psur->Ellipse(vendcenter.x, vendcenter.y, m_d.m_EndRadius);
}

void Flipper::Render(Sur * const psur)
{
   const float anglerad = ANGTORAD(m_d.m_StartAngle);
   const float anglerad2 = ANGTORAD(m_d.m_EndAngle);

   Vertex2D vendcenter;
   Vertex2D rgv[4];
   SetVertices(anglerad, &vendcenter, rgv, m_d.m_BaseRadius, m_d.m_EndRadius);

   psur->SetFillColor(RGB(192,192,192));
   psur->SetBorderColor(-1,false,0);
   psur->SetLineColor(RGB(0,0,0), false, 0);

   psur->SetObject(this);

   psur->Line(rgv[0].x, rgv[0].y, rgv[1].x, rgv[1].y);
   psur->Line(rgv[2].x, rgv[2].y, rgv[3].x, rgv[3].y);

   psur->Arc(m_d.m_Center.x, m_d.m_Center.y, m_d.m_BaseRadius, rgv[0].x, rgv[0].y, rgv[3].x, rgv[3].y);
   psur->Arc(vendcenter.x, vendcenter.y, m_d.m_EndRadius, rgv[2].x, rgv[2].y, rgv[1].x, rgv[1].y);

   SetVertices(anglerad2, &vendcenter, rgv, m_d.m_BaseRadius, m_d.m_EndRadius);

   psur->SetLineColor(RGB(128,128,128), true, 0);

   psur->Line(rgv[0].x, rgv[0].y, rgv[1].x, rgv[1].y);
   psur->Line(rgv[2].x, rgv[2].y, rgv[3].x, rgv[3].y);

   psur->Arc(m_d.m_Center.x, m_d.m_Center.y, m_d.m_BaseRadius, rgv[0].x, rgv[0].y, rgv[3].x, rgv[3].y);
   psur->Arc(vendcenter.x, vendcenter.y, m_d.m_EndRadius, rgv[2].x, rgv[2].y, rgv[1].x, rgv[1].y);

   rgv[0].x = m_d.m_Center.x + sinf(anglerad) * (m_d.m_FlipperRadius+m_d.m_EndRadius);
   rgv[0].y = m_d.m_Center.y - cosf(anglerad) * (m_d.m_FlipperRadius+m_d.m_EndRadius);

   rgv[1].x = m_d.m_Center.x + sinf(anglerad2) * (m_d.m_FlipperRadius+m_d.m_EndRadius);
   rgv[1].y = m_d.m_Center.y - cosf(anglerad2) * (m_d.m_FlipperRadius+m_d.m_EndRadius);

   if (m_d.m_EndAngle < m_d.m_StartAngle)
      psur->Arc(m_d.m_Center.x, m_d.m_Center.y, m_d.m_FlipperRadius+m_d.m_EndRadius
      , rgv[0].x, rgv[0].y, rgv[1].x, rgv[1].y);
   else psur->Arc(m_d.m_Center.x, m_d.m_Center.y, m_d.m_FlipperRadius+m_d.m_EndRadius
      , rgv[1].x, rgv[1].y, rgv[0].x, rgv[0].y);

   if (m_d.m_FlipperRadiusMin > 0.f && m_d.m_FlipperRadiusMax > m_d.m_FlipperRadiusMin)
   {
      m_d.m_FlipperRadius = (m_ptable->m_globalDifficulty > 0.f) ? m_d.m_FlipperRadiusMin : m_d.m_FlipperRadiusMax;
      m_d.m_FlipperRadius = max(m_d.m_FlipperRadius, m_d.m_BaseRadius - m_d.m_EndRadius +0.05f);
   }
   else return;

   if (m_d.m_FlipperRadius != m_d.m_FlipperRadiusMax)
   {
      SetVertices(anglerad, &vendcenter, rgv, m_d.m_BaseRadius, m_d.m_EndRadius);

      psur->SetObject(this);

      psur->Line(rgv[0].x, rgv[0].y, rgv[1].x, rgv[1].y);
      psur->Line(rgv[2].x, rgv[2].y, rgv[3].x, rgv[3].y);

      psur->Arc(m_d.m_Center.x, m_d.m_Center.y, m_d.m_BaseRadius, rgv[0].x, rgv[0].y, rgv[3].x, rgv[3].y);
      psur->Arc(vendcenter.x, vendcenter.y, m_d.m_EndRadius, rgv[2].x, rgv[2].y, rgv[1].x, rgv[1].y);

      SetVertices(anglerad2, &vendcenter, rgv, m_d.m_BaseRadius, m_d.m_EndRadius);

      psur->SetLineColor(RGB(128,128,128), true, 0);

      psur->Line(rgv[0].x, rgv[0].y, rgv[1].x, rgv[1].y);
      psur->Line(rgv[2].x, rgv[2].y, rgv[3].x, rgv[3].y);

      psur->Arc(m_d.m_Center.x, m_d.m_Center.y, m_d.m_BaseRadius, rgv[0].x, rgv[0].y, rgv[3].x, rgv[3].y);
      psur->Arc(vendcenter.x, vendcenter.y, m_d.m_EndRadius, rgv[2].x, rgv[2].y, rgv[1].x, rgv[1].y);

      rgv[0].x = m_d.m_Center.x + sinf(anglerad) * (m_d.m_FlipperRadius+m_d.m_EndRadius);
      rgv[0].y = m_d.m_Center.y - cosf(anglerad) * (m_d.m_FlipperRadius+m_d.m_EndRadius);

      rgv[1].x = m_d.m_Center.x + sinf(anglerad2) * (m_d.m_FlipperRadius+m_d.m_EndRadius);
      rgv[1].y = m_d.m_Center.y - cosf(anglerad2) * (m_d.m_FlipperRadius+m_d.m_EndRadius);

      if (m_d.m_EndAngle < m_d.m_StartAngle)
         psur->Arc(m_d.m_Center.x, m_d.m_Center.y, m_d.m_FlipperRadius+m_d.m_EndRadius
         , rgv[0].x, rgv[0].y, rgv[1].x, rgv[1].y);
      else psur->Arc(m_d.m_Center.x, m_d.m_Center.y, m_d.m_FlipperRadius+m_d.m_EndRadius
         , rgv[1].x, rgv[1].y, rgv[0].x, rgv[0].y);

      m_d.m_FlipperRadius = m_d.m_FlipperRadiusMax - (m_d.m_FlipperRadiusMax - m_d.m_FlipperRadiusMin) * m_ptable->m_globalDifficulty;
      m_d.m_FlipperRadius = max(m_d.m_FlipperRadius, m_d.m_BaseRadius - m_d.m_EndRadius +0.05f);

      SetVertices(anglerad, &vendcenter, rgv, m_d.m_BaseRadius, m_d.m_EndRadius);

      psur->SetObject(this);

      psur->Line(rgv[0].x, rgv[0].y, rgv[1].x, rgv[1].y);
      psur->Line(rgv[2].x, rgv[2].y, rgv[3].x, rgv[3].y);

      psur->Arc(m_d.m_Center.x, m_d.m_Center.y, m_d.m_BaseRadius, rgv[0].x, rgv[0].y, rgv[3].x, rgv[3].y);
      psur->Arc(vendcenter.x, vendcenter.y, m_d.m_EndRadius, rgv[2].x, rgv[2].y, rgv[1].x, rgv[1].y);

      SetVertices(anglerad2, &vendcenter, rgv, m_d.m_BaseRadius, m_d.m_EndRadius);

      psur->SetLineColor(RGB(128,128,128), true, 0);

      psur->Line(rgv[0].x, rgv[0].y, rgv[1].x, rgv[1].y);
      psur->Line(rgv[2].x, rgv[2].y, rgv[3].x, rgv[3].y);

      psur->Arc(m_d.m_Center.x, m_d.m_Center.y, m_d.m_BaseRadius, rgv[0].x, rgv[0].y, rgv[3].x, rgv[3].y);
      psur->Arc(vendcenter.x, vendcenter.y, m_d.m_EndRadius, rgv[2].x, rgv[2].y, rgv[1].x, rgv[1].y);

      rgv[0].x = m_d.m_Center.x + sinf(anglerad) * (m_d.m_FlipperRadius+m_d.m_EndRadius);
      rgv[0].y = m_d.m_Center.y - cosf(anglerad) * (m_d.m_FlipperRadius+m_d.m_EndRadius);

      rgv[1].x = m_d.m_Center.x + sinf(anglerad2) * (m_d.m_FlipperRadius+m_d.m_EndRadius);
      rgv[1].y = m_d.m_Center.y - cosf(anglerad2) * (m_d.m_FlipperRadius+m_d.m_EndRadius);

      if (m_d.m_EndAngle < m_d.m_StartAngle)
         psur->Arc(m_d.m_Center.x, m_d.m_Center.y, m_d.m_FlipperRadius+m_d.m_EndRadius
         , rgv[0].x, rgv[0].y, rgv[1].x, rgv[1].y);
      else psur->Arc(m_d.m_Center.x, m_d.m_Center.y, m_d.m_FlipperRadius+m_d.m_EndRadius
         , rgv[1].x, rgv[1].y, rgv[0].x, rgv[0].y);
   }

   m_d.m_FlipperRadius = m_d.m_FlipperRadiusMax;
}

void Flipper::RenderShadow(ShadowSur * const psur, const float height)
{
}

void Flipper::SetObjectPos()
{
   g_pvp->SetObjectPosCur(m_d.m_Center.x, m_d.m_Center.y);
}

void Flipper::MoveOffset(const float dx, const float dy)
{
   m_d.m_Center.x += dx;
   m_d.m_Center.y += dy;

   m_ptable->SetDirtyDraw();
}

void Flipper::GetCenter(Vertex2D * const pv) const
{
   *pv = m_d.m_Center;
}

void Flipper::PutCenter(const Vertex2D * const pv)
{
   m_d.m_Center = *pv;

   m_ptable->SetDirtyDraw();
}

STDMETHODIMP Flipper::InterfaceSupportsErrorInfo(REFIID riid)
{
   static const IID* arr[] =
   {
      &IID_IFlipper,
   };

   for (int i=0;i<sizeof(arr)/sizeof(arr[0]);i++)
   {
      if (InlineIsEqualGUID(*arr[i],riid))
         return S_OK;
   }
   return S_FALSE;
}

STDMETHODIMP Flipper::RotateToEnd() //power stroke to hit ball
{
   if (m_phitflipper)
   {
      const float endAng = ANGTORAD(m_d.m_EndAngle);
      m_phitflipper->m_flipperanim.m_EnableRotateEvent = 1;
      m_phitflipper->m_flipperanim.m_angleEnd = endAng;

      if (fabsf(endAng - m_phitflipper->m_flipperanim.m_angleCur) < 1.0e-5f)   //already there?
      {
         m_phitflipper->m_flipperanim.m_fAcc = 0;
         m_phitflipper->m_flipperanim.m_anglespeed = 0;
      }
      else m_phitflipper->m_flipperanim.m_fAcc = (endAng > m_phitflipper->m_flipperanim.m_angleCur) ? +1 : -1;

	  m_phitflipper->m_flipperanim.m_maxvelocity = (m_d.m_OverridePhysics ? m_d.m_OverrideSpeed : m_d.m_force) * 4.5f;
      m_phitflipper->m_flipperanim.m_force = (m_d.m_OverridePhysics ? m_d.m_OverrideSpeed : m_d.m_force);
      m_phitflipper->m_forcemass = (m_d.m_OverridePhysics ? m_d.m_OverrideStrength : m_d.m_strength);
   }
   return S_OK;
}

STDMETHODIMP Flipper::RotateToStart() // return to park 
{
   if (m_phitflipper)
   {
      const float startAng =  ANGTORAD(m_d.m_StartAngle);		
      m_phitflipper->m_flipperanim.m_EnableRotateEvent = -1;
      m_phitflipper->m_flipperanim.m_angleEnd = startAng;

      if (fabsf(startAng - m_phitflipper->m_flipperanim.m_angleCur) < 1.0e-5f)//already there?
      {
         m_phitflipper->m_flipperanim.m_fAcc = 0;
         m_phitflipper->m_flipperanim.m_anglespeed = 0;
      }
      else m_phitflipper->m_flipperanim.m_fAcc = (startAng > m_phitflipper->m_flipperanim.m_angleCur) ? +1 : -1;

      m_phitflipper->m_flipperanim.m_maxvelocity = (m_d.m_OverridePhysics ? m_d.m_OverrideSpeed : m_d.m_force) * 4.5f;

      float rtn = (m_d.m_OverridePhysics ? m_d.m_OverrideReturnStrength : m_d.m_return);
      if (rtn <= 0.f) rtn = 1.0f;

      m_phitflipper->m_flipperanim.m_force = (m_d.m_OverridePhysics ? m_d.m_OverrideSpeed : m_d.m_force) * rtn;
      m_phitflipper->m_forcemass = (m_d.m_OverridePhysics ? m_d.m_OverrideStrength : m_d.m_strength) * rtn;		
   }
   return S_OK;
}

void Flipper::PostRenderStatic(const RenderDevice* _pd3dDevice)
{
   RenderDevice* pd3dDevice=(RenderDevice*)_pd3dDevice;
   if ( !m_d.m_fVisible )
      return;
   const int frame = m_phitflipper->m_flipperanim.m_iframe;
   if (frame == -1 || frame >= maxFrames)
      return;

   pd3dDevice->SetRenderState(RenderDevice::ALPHATESTENABLE, TRUE); 
   pd3dDevice->SetRenderState(RenderDevice::ALPHABLENDENABLE, FALSE); 
   pd3dDevice->SetRenderState(RenderDevice::ZWRITEENABLE, TRUE); 

   Pin3D * const ppin3d = &g_pplayer->m_pin3d;
   Material mat;
   mat.setColor( 1.0f, m_d.m_color);
   mat.set();

   //draw flipper
   pd3dDevice->renderPrimitive( D3DPT_TRIANGLELIST, vBufferArray[frame].vbuffer, 0, 162, (LPWORD)indexBuffer, 270, 0 );

   // render the rubber
   mat.setColor( 1.0f, m_d.m_rubbercolor);
   mat.set();

   //draw rubber
   pd3dDevice->renderPrimitive( D3DPT_TRIANGLELIST, vBufferArray[frame].vbuffer, 162, 162, (LPWORD)indexBuffer, 270, 0 );
}

void Flipper::RenderSetup(const RenderDevice* _pd3dDevice)
{
  const unsigned long numVertices= 162*2; //*2 for flipper plus rubber
   RenderDevice* pd3dDevice=(RenderDevice*)_pd3dDevice;
   _ASSERTE(m_phitflipper);
   Pin3D * const ppin3d = &g_pplayer->m_pin3d;

   const float height = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_Center.x, m_d.m_Center.y);

   ppin3d->ClearSpriteRectangle( &m_phitflipper->m_flipperanim, NULL );

   const float anglerad = ANGTORAD(m_d.m_StartAngle);
   const float anglerad2 = ANGTORAD(m_d.m_EndAngle);

   ppin3d->SetTexture(NULL);

   const int cframes = max(abs(((int)(m_d.m_EndAngle - m_d.m_StartAngle))/2),2);//10),2);

   if( !vBufferArray )
   {      
      vBufferArray = new VertexBufferArray[cframes];
      for( int i=0; i<cframes; i++ )
      {
         pd3dDevice->createVertexBuffer( numVertices, 0, MY_D3DFVF_VERTEX, &vBufferArray[i].vbuffer );
         NumVideoBytes += numVertices*sizeof(Vertex3D); //!! never cleared up again here
      }
      maxFrames = cframes;
   }

   // Direct all renders to the back buffer.
   //ppin3d->SetRenderTarget(g_pplayer->m_pin3d.m_pddsBackBuffer);

   const float inv_cframes = (anglerad2 - anglerad)/(float)(cframes-1);
   // Pre-render each of the frames.
   unsigned long ofs=0;

   static WORD idx[18+3*14+3*14+6*14*2]={0,1,2,0,2,3, 4,5,6,4,6,7, 8,9,10,8,10,11 };
   for( int i=0;i<14;i++ )
   {
      idx[18+i*3  ] = 12;
      idx[18+i*3+1] = 12+i+1;
      idx[18+i*3+2] = 12+i+2;
      idx[60+i*3  ] = 84;
      idx[60+i*3+1] = 84+i+1;
      idx[60+i*3+2] = 84+i+2;
   }

   WORD iidx[6*14*2];
   WORD o=28;
   for(int i=0;i<14;i++ )
   {
      iidx[i*6  ] = o;
      iidx[i*6+1] = o+1;
      iidx[i*6+2] = o+2;
      iidx[i*6+3] = o;
      iidx[i*6+4] = o+2;
      iidx[i*6+5] = o+3;

      iidx[84+i*6  ] = 72+o;
      iidx[84+i*6+1] = 72+o+1;
      iidx[84+i*6+2] = 72+o+2;
      iidx[84+i*6+3] = 72+o;
      iidx[84+i*6+4] = 72+o+2;
      iidx[84+i*6+5] = 72+o+3;
      o+=4;
   }
   memcpy( &idx[102], iidx, sizeof(WORD)*6*14*2);
   memcpy( indexBuffer, idx, sizeof(WORD)*270);

   for (int i=0;i<cframes;i++)
   {
      const float angle = anglerad + inv_cframes*(float)i;

      ObjFrame * const pof = new ObjFrame();

      ppin3d->ClearSpriteRectangle( NULL, pof );

      // Render just the flipper.
      RenderAtThickness(pd3dDevice, pof, angle,  height, m_d.m_BaseRadius - (float)m_d.m_rubberthickness, m_d.m_EndRadius - (float)m_d.m_rubberthickness, m_d.m_height, false, i );

      // Render just the rubber.
      if (m_d.m_rubberthickness > 0)
      {
         RenderAtThickness(pd3dDevice, pof, angle, height + (float)m_d.m_rubberheight, m_d.m_BaseRadius, m_d.m_EndRadius, (float)m_d.m_rubberwidth, true, i );// 34);
      }
      //ppin3d->CreateAndCopySpriteBuffers( &m_phitflipper->m_flipperanim, pof );
      ppin3d->ExpandRectByRect(&m_phitflipper->m_flipperanim.m_rcBounds, &pof->rc);

      m_phitflipper->m_flipperanim.m_vddsFrame.AddElement(pof);
   }

}

void Flipper::RenderStatic(const RenderDevice* pd3dDevice)
{
}

static const WORD rgiFlipper1[4] = {0,4,5,1};
static const WORD rgiFlipper2[4] = {2,6,7,3};
void Flipper::RenderAtThickness(RenderDevice* _pd3dDevice, ObjFrame * const pof, const float angle, const float height, 
                                const float baseradius, const float endradius, const float flipperheight, const bool isRubber, const int vBufferIndex)
{
   RenderDevice* pd3dDevice=(RenderDevice*)_pd3dDevice;
   Pin3D * const ppin3d = &g_pplayer->m_pin3d;
   unsigned long offset=0;

   Vertex2D vendcenter;
   Vertex2D rgv[4];
   SetVertices(angle, &vendcenter, rgv, baseradius, endradius);

   const float inv_width  = 1.0f/(g_pplayer->m_ptable->m_left + g_pplayer->m_ptable->m_right);
   const float inv_height = 1.0f/(g_pplayer->m_ptable->m_top  + g_pplayer->m_ptable->m_bottom);

   Vertex3D rgv3D[32];
   for (int l=0;l<8;l++)
   {
      rgv3D[l].x = rgv[l&3].x;
      rgv3D[l].y = rgv[l&3].y;
      rgv3D[l].z = (l<4) ? height + flipperheight + 0.1f : height; // Make flippers a bit taller so they draw above walls
      rgv3D[l].z *= m_ptable->m_zScale;
      ppin3d->m_lightproject.CalcCoordinates(&rgv3D[l],inv_width,inv_height);		
   }

   ppin3d->ExpandExtents(&pof->rc, rgv3D, &m_phitflipper->m_flipperanim.m_znear, &m_phitflipper->m_flipperanim.m_zfar, 8, fFalse);

   if( isRubber )
      offset=162;

   Vertex3D *buf;
   vBufferArray[vBufferIndex].vbuffer->lock(0,0,(void**)&buf, VertexBuffer::WRITEONLY | VertexBuffer::NOOVERWRITE );

   SetNormal(rgv3D, rgi0123, 4, NULL, NULL, 0);
   // Draw top.
   buf[offset    ] = rgv3D[0];
   buf[offset + 1] = rgv3D[1];
   buf[offset + 2] = rgv3D[2];
   buf[offset + 3] = rgv3D[3];
   offset+=4;
   SetNormal(rgv3D, rgiFlipper1, 4, NULL, NULL, 0);
   // Draw front side wall of flipper (flipper and rubber).
   buf[offset    ] = rgv3D[0];
   buf[offset + 1] = rgv3D[4];
   buf[offset + 2] = rgv3D[5];
   buf[offset + 3] = rgv3D[1];
   offset+=4;
   SetNormal(rgv3D, rgiFlipper2, 4, NULL, NULL, 0);
   // Draw back side wall.
   buf[offset    ] = rgv3D[2];
   buf[offset + 1] = rgv3D[6];
   buf[offset + 2] = rgv3D[7];
   buf[offset + 3] = rgv3D[3];
   offset+=4;

   // Base circle
   for (int l=0;l<16;l++)
   {
      const float anglel = (float)(M_PI*2.0/16.0)*(float)l;
      rgv3D[l].x = m_d.m_Center.x + sinf(anglel)*baseradius;
      rgv3D[l].y = m_d.m_Center.y - cosf(anglel)*baseradius;
      rgv3D[l].z = height + flipperheight + 0.1f;
	  rgv3D[l].z *= m_ptable->m_zScale;
      rgv3D[l+16].x = rgv3D[l].x;
      rgv3D[l+16].y = rgv3D[l].y;
      rgv3D[l+16].z = height;
      rgv3D[l+16].z *= m_ptable->m_zScale;

      ppin3d->m_lightproject.CalcCoordinates(&rgv3D[l],inv_width,inv_height);
      ppin3d->m_lightproject.CalcCoordinates(&rgv3D[l+16],inv_width,inv_height);
   }

   ppin3d->ExpandExtents(&pof->rc, rgv3D,&m_phitflipper->m_flipperanim.m_znear,
      &m_phitflipper->m_flipperanim.m_zfar, 32, fFalse);

   // Draw end caps of cylinders of large ends.
   WORD endCapsIndex[3*14];
   for (int l=0;l<14;l++)
   {
      endCapsIndex[l*3  ] = 0;
      endCapsIndex[l*3+1] = l+1;
      endCapsIndex[l*3+2] = l+2;
      SetNormal(rgv3D, endCapsIndex+l*3, 3, NULL, NULL, 0);
   }	
   memcpy( &buf[offset], rgv3D, sizeof(Vertex3D)*16 );
   offset+=16;

   for (int l=0;l<14;l++)
   {
      const WORD rgiNormal[6] = 
      {
         (l+15) % 16,
         (l+15) % 16 + 16,
         l+1,
         l,
         l+16,
         (l+2) % 16
      };

      const WORD rgi[4] = {l,l+16,l+1+16,l+1};
      SetNormal(rgv3D, rgiNormal, 3, NULL, rgi, 2);
      SetNormal(rgv3D, &rgiNormal[3], 3, NULL, &rgi[2], 2);
      // Draw vertical cylinders at large end of flipper.
      memcpy( &buf[offset],   &rgv3D[l], sizeof(Vertex3D));
      memcpy( &buf[offset+1], &rgv3D[l+16], sizeof(Vertex3D));
      memcpy( &buf[offset+2], &rgv3D[l+1+16], sizeof(Vertex3D));
      memcpy( &buf[offset+3], &rgv3D[l+1], sizeof(Vertex3D));
      offset+=4;
   }

   // End circle.
   for (int l=0;l<16;l++)
   {
      const float anglel = (float)(M_PI*2.0/16.0)*(float)l;
      rgv3D[l].x = vendcenter.x + sinf(anglel)*endradius;
      rgv3D[l].y = vendcenter.y - cosf(anglel)*endradius;
      rgv3D[l].z = height + flipperheight + 0.1f;
	  rgv3D[l].z *= m_ptable->m_zScale;
	  rgv3D[l+16].x = rgv3D[l].x;
      rgv3D[l+16].y = rgv3D[l].y;
      rgv3D[l+16].z = height;
      rgv3D[l+16].z *= m_ptable->m_zScale;

      ppin3d->m_lightproject.CalcCoordinates(&rgv3D[l],inv_width,inv_height);
      ppin3d->m_lightproject.CalcCoordinates(&rgv3D[l+16],inv_width,inv_height);
   }

   ppin3d->ExpandExtents(&pof->rc, rgv3D, &m_phitflipper->m_flipperanim.m_znear, &m_phitflipper->m_flipperanim.m_zfar, 32, fFalse);

   // Draw end caps to vertical cylinder at small end.
   for (int l=0;l<14;l++)
   {
      SetNormal(rgv3D, endCapsIndex+l*3, 3, NULL, NULL, 0);
   }
   memcpy( &buf[offset], rgv3D, sizeof(Vertex3D)*16 );
   offset+=16;

   for (int l=0;l<14;l++)
   {
      const WORD rgiNormal[6] = {
         (l+15) % 16,
         (l+15) % 16 + 16,
         l+1,
         l,
         l+16,
         (l+2) % 16};

         const WORD rgi[4] = {l,l+16,l+1+16,l+1};
         SetNormal(rgv3D, rgiNormal, 3, NULL, rgi, 2);
         SetNormal(rgv3D, &rgiNormal[3], 3, NULL, &rgi[2], 2);
         // Draw vertical cylinders at small end.
         memcpy( &buf[offset],   &rgv3D[l], sizeof(Vertex3D));
         memcpy( &buf[offset+1], &rgv3D[l+16], sizeof(Vertex3D));
         memcpy( &buf[offset+2], &rgv3D[l+1+16], sizeof(Vertex3D));
         memcpy( &buf[offset+3], &rgv3D[l+1], sizeof(Vertex3D));
         offset+=4;
   }
   vBufferArray[vBufferIndex].vbuffer->unlock();
}

void Flipper::RenderMovers(const RenderDevice* _pd3dDevice)
{
}


HRESULT Flipper::SaveData(IStream *pstm, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
{
   BiffWriter bw(pstm, hcrypthash, hcryptkey);

#ifdef VBA
   bw.WriteInt(FID(PIID), ApcProjectItem.ID());
#endif
   bw.WriteStruct(FID(VCEN), &m_d.m_Center, sizeof(Vertex2D));
   bw.WriteFloat(FID(BASR), m_d.m_BaseRadius);
   bw.WriteFloat(FID(ENDR), m_d.m_EndRadius);
   bw.WriteFloat(FID(FLPR), m_d.m_FlipperRadiusMax);
   bw.WriteFloat(FID(FCOIL), m_d.m_recoil);
   bw.WriteFloat(FID(FAEOS), m_d.m_angleEOS);
   bw.WriteFloat(FID(FRTN), m_d.m_return);
   bw.WriteFloat(FID(ANGS), m_d.m_StartAngle);
   bw.WriteFloat(FID(ANGE), m_d.m_EndAngle);
   bw.WriteInt(FID(OVRP), m_d.m_OverridePhysics);
   bw.WriteFloat(FID(FORC), m_d.m_force);
   bw.WriteBool(FID(TMON), m_d.m_tdr.m_fTimerEnabled);
   bw.WriteInt(FID(TMIN), m_d.m_tdr.m_TimerInterval);
   bw.WriteString(FID(SURF), m_d.m_szSurface);
   bw.WriteInt(FID(COLR), m_d.m_color);
   bw.WriteWideString(FID(NAME), (WCHAR *)m_wzName);
   bw.WriteInt(FID(FCLR), m_d.m_rubbercolor);
   bw.WriteInt(FID(RTHK), m_d.m_rubberthickness);
   bw.WriteInt(FID(RHGT), m_d.m_rubberheight);
   bw.WriteInt(FID(RWDT), m_d.m_rubberwidth);
   bw.WriteFloat(FID(STRG), m_d.m_strength);
   bw.WriteFloat(FID(ELAS), m_d.m_elasticity);
   bw.WriteBool(FID(VSBL), m_d.m_fVisible);
   bw.WriteBool(FID(ENBL), m_d.m_fEnabled);
   bw.WriteFloat(FID(FPWL), m_d.m_powerlaw);	
   bw.WriteFloat(FID(FOCR), m_d.m_obliquecorrection);	
   bw.WriteFloat(FID(FSCT), m_d.m_scatterangle);	
   bw.WriteFloat(FID(FRMN), m_d.m_FlipperRadiusMin);		
   bw.WriteFloat(FID(FHGT), m_d.m_height);

   ISelect::SaveData(pstm, hcrypthash, hcryptkey);

   bw.WriteTag(FID(ENDB));

   return S_OK;
}

HRESULT Flipper::InitLoad(IStream *pstm, PinTable *ptable, int *pid, int version, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
{
   SetDefaults(false);

   BiffReader br(pstm, this, pid, version, hcrypthash, hcryptkey);

   m_ptable = ptable;

   br.Load();
   return S_OK;
}

BOOL Flipper::LoadToken(int id, BiffReader *pbr)
{
   if (id == FID(PIID))
   {
      pbr->GetInt((int *)pbr->m_pdata);
   }
   else if (id == FID(VCEN))
   {
      pbr->GetStruct(&m_d.m_Center, sizeof(Vertex2D));
   }
   else if (id == FID(BASR))
   {
      pbr->GetFloat(&m_d.m_BaseRadius);
   }
   else if (id == FID(ENDR))
   {
      pbr->GetFloat(&m_d.m_EndRadius);
   }
   else if (id == FID(FLPR))
   {
      pbr->GetFloat(&m_d.m_FlipperRadiusMax);
   }
   else if (id == FID(FCOIL))
   {
      pbr->GetFloat(&m_d.m_recoil);
   }
   else if (id == FID(FAEOS))
   {
      pbr->GetFloat(&m_d.m_angleEOS);
   }
   else if (id == FID(FRTN))
   {
      pbr->GetFloat(&m_d.m_return);
   }
   else if (id == FID(ANGS))
   {
      pbr->GetFloat(&m_d.m_StartAngle);
   }
   else if (id == FID(ANGE))
   {
      pbr->GetFloat(&m_d.m_EndAngle);
   }
   else if (id == FID(OVRP))
   {
      pbr->GetInt(&m_d.m_OverridePhysics);
   }
   else if (id == FID(FORC))
   {
      pbr->GetFloat(&m_d.m_force);
   }
   else if (id == FID(TMON))
   {
      pbr->GetBool(&m_d.m_tdr.m_fTimerEnabled);
   }
   else if (id == FID(TMIN))
   {
      pbr->GetInt(&m_d.m_tdr.m_TimerInterval);
      //m_d.m_tdr.m_TimerInterval = INT(m_d.m_tdr.m_TimerInterval);
      if (m_d.m_tdr.m_TimerInterval<1) {m_d.m_tdr.m_TimerInterval = 100;}
   }
   else if (id == FID(SURF))
   {
      pbr->GetString(m_d.m_szSurface);
   }
   else if (id == FID(COLR))
   {
      pbr->GetInt(&m_d.m_color);
      //if (!(m_d.m_color & MINBLACKMASK)) {m_d.m_color |= MINBLACK;}	// set minimum black
   }
   else if (id == FID(FCLR))
   {
      pbr->GetInt(&m_d.m_rubbercolor);
      //if (!(m_d.m_rubbercolor & MINBLACKMASK)) {m_d.m_rubbercolor |= MINBLACK;}	// set minimum black
   }
   else if (id == FID(NAME))
   {
      pbr->GetWideString((WCHAR *)m_wzName);
   }
   else if (id == FID(RTHK))
   {
      pbr->GetInt(&m_d.m_rubberthickness);
   }
   else if (id == FID(RHGT))
   {
      pbr->GetInt(&m_d.m_rubberheight);
   }
   else if (id == FID(RWDT))
   {
      pbr->GetInt(&m_d.m_rubberwidth);
   }
   else if (id == FID(FHGT))
   {
      pbr->GetFloat(&m_d.m_height);
   }
   else if (id == FID(STRG))
   {
      pbr->GetFloat(&m_d.m_strength);
   }
   else if (id == FID(ELAS))
   {
      pbr->GetFloat(&m_d.m_elasticity);
   }
   else if (id == FID(FPWL))
   {
      pbr->GetFloat(&m_d.m_powerlaw);
   }
   else if (id == FID(FOCR))
   {
      pbr->GetFloat(&m_d.m_obliquecorrection); 
   }
   else if (id == FID(FSCT))
   {
      pbr->GetFloat(&m_d.m_scatterangle);
   }
   else if (id == FID(FRMN))
   {
      pbr->GetFloat(&m_d.m_FlipperRadiusMin);	
   }
   else if (id == FID(VSBL))
   {
      pbr->GetBool(&m_d.m_fVisible);
   }
   else if (id == FID(ENBL))
   {
      pbr->GetBool(&m_d.m_fEnabled);
   }
   else
   {
      ISelect::LoadToken(id, pbr);
   }

   return fTrue;
}

HRESULT Flipper::InitPostLoad()
{
   if(m_d.m_height > 1000.0f) m_d.m_height = 50.0f;
   if(m_d.m_rubberheight > 1000) m_d.m_rubberheight = 8;
   if(m_d.m_rubberthickness > 0 && m_d.m_height > 16.0f && m_d.m_rubberwidth == 0)
      m_d.m_rubberwidth = (int)(m_d.m_height-16.0f);
   if(m_d.m_rubberwidth > 1000) m_d.m_rubberwidth = (int)(m_d.m_height-16.0f);

   return S_OK;
}

STDMETHODIMP Flipper::get_BaseRadius(float *pVal)
{
   *pVal = m_d.m_BaseRadius;

   return S_OK;
}

STDMETHODIMP Flipper::put_BaseRadius(float newVal)
{
   STARTUNDO

   m_d.m_BaseRadius = newVal;

   STOPUNDO;

   return S_OK;
}

STDMETHODIMP Flipper::get_EndRadius(float *pVal)
{
   *pVal = m_d.m_EndRadius;

   return S_OK;
}

STDMETHODIMP Flipper::put_EndRadius(float newVal)
{
   STARTUNDO

   m_d.m_EndRadius = newVal;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Flipper::get_Length(float *pVal)
{
   *pVal = m_d.m_FlipperRadiusMax;

   return S_OK;
}

STDMETHODIMP Flipper::put_Length(float newVal)
{
   STARTUNDO

   m_d.m_FlipperRadiusMax = newVal;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Flipper::get_StartAngle(float *pVal)
{
   *pVal = m_d.m_StartAngle;

   return S_OK;
}

STDMETHODIMP Flipper::put_StartAngle(float newVal)
{
   STARTUNDO

   m_d.m_StartAngle = newVal;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Flipper::get_EndAngle(float *pVal)
{
   *pVal = m_d.m_EndAngle;

   return S_OK;
}

STDMETHODIMP Flipper::put_EndAngle(float newVal)
{
   STARTUNDO

   m_d.m_EndAngle = newVal;

   STOPUNDO;

   return S_OK;
}

STDMETHODIMP Flipper::get_CurrentAngle(float *pVal)
{
   if (m_phitflipper)
   {
      *pVal = RADTOANG(m_phitflipper->m_flipperanim.m_angleCur);
      return S_OK;
   }
   else
      return E_FAIL;
}


STDMETHODIMP Flipper::get_X(float *pVal)
{
   *pVal = m_d.m_Center.x;

   return S_OK;
}

STDMETHODIMP Flipper::put_X(float newVal)
{
   STARTUNDO

   m_d.m_Center.x = newVal;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Flipper::get_Y(float *pVal)
{
   *pVal = m_d.m_Center.y;

   return S_OK;
}

STDMETHODIMP Flipper::put_Y(float newVal)
{
   STARTUNDO

   m_d.m_Center.y = newVal;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Flipper::get_Surface(BSTR *pVal)
{
   WCHAR wz[512];

   MultiByteToWideChar(CP_ACP, 0, m_d.m_szSurface, -1, wz, 32);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP Flipper::put_Surface(BSTR newVal)
{
   STARTUNDO

   WideCharToMultiByte(CP_ACP, 0, newVal, -1, m_d.m_szSurface, 32, NULL, NULL);

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Flipper::get_Color(OLE_COLOR *pVal)
{
   *pVal = m_d.m_color;

   return S_OK;
}

STDMETHODIMP Flipper::put_Color(OLE_COLOR newVal)
{
   STARTUNDO

   m_d.m_color = newVal;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Flipper::get_Recoil(float *pVal)
{
   *pVal = m_d.m_recoil;

   return S_OK;
}

STDMETHODIMP Flipper::put_Recoil(float newVal)
{
   STARTUNDO

   m_d.m_recoil = newVal;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Flipper::get_AngleEOS(float *pVal)
{
   *pVal = m_d.m_angleEOS;

   return S_OK;
}

STDMETHODIMP Flipper::put_AngleEOS(float newVal)
{
   STARTUNDO

   m_d.m_angleEOS = newVal;

   STOPUNDO

   return S_OK;
}

void Flipper::GetDialogPanes(Vector<PropertyPane> *pvproppane)
{
   PropertyPane *pproppane;

   pproppane = new PropertyPane(IDD_PROP_NAME, NULL);
   pvproppane->AddElement(pproppane);

   pproppane = new PropertyPane(IDD_PROPFLIPPER_VISUALS, IDS_VISUALS);
   pvproppane->AddElement(pproppane);

   pproppane = new PropertyPane(IDD_PROPFLIPPER_POSITION, IDS_POSITION);
   pvproppane->AddElement(pproppane);

   pproppane = new PropertyPane(IDD_PROPFLIPPER_PHYSICS, IDS_PHYSICS);
   pvproppane->AddElement(pproppane);

   pproppane = new PropertyPane(IDD_PROP_TIMER, IDS_MISC);
   pvproppane->AddElement(pproppane);
}

STDMETHODIMP Flipper::get_Speed(float *pVal)
{
   /*	if (m_phitflipper)
   {
   *pVal = m_phitflipper->m_flipperanim.m_anglespeed * 180.0f /PI;
   }
   else*/ *pVal = m_d.m_force;

   return S_OK;
}

STDMETHODIMP Flipper::put_Speed(float newVal)
{
   /*	if (m_phitflipper)
   {
	  m_phitflipper->m_flipperanim.m_anglespeed = newVal *PI/180.0f;
   }
   else*/
   {
      STARTUNDO
      m_d.m_force = newVal;
      STOPUNDO
   }

   return S_OK;
}

STDMETHODIMP Flipper::get_OverridePhysics(long *pVal)
{
   *pVal = m_d.m_OverridePhysics;

   return S_OK;
}

STDMETHODIMP Flipper::put_OverridePhysics(long newVal)
{
   STARTUNDO
   m_d.m_OverridePhysics = newVal;
   STOPUNDO

   return S_OK;
}

STDMETHODIMP Flipper::get_RubberColor(OLE_COLOR *pVal)
{
   *pVal = m_d.m_rubbercolor;

   return S_OK;
}

STDMETHODIMP Flipper::put_RubberColor(OLE_COLOR newVal)
{
   STARTUNDO

   m_d.m_rubbercolor = newVal;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Flipper::get_RubberThickness(long *pVal)
{
   *pVal = m_d.m_rubberthickness;

   return S_OK;
}

STDMETHODIMP Flipper::get_RubberHeight(long *pVal)
{
   *pVal = m_d.m_rubberheight;

   return S_OK;
}

STDMETHODIMP Flipper::get_RubberWidth(long *pVal)
{
   *pVal = m_d.m_rubberwidth;

   return S_OK;
}

STDMETHODIMP Flipper::put_RubberThickness(long newVal)
{
   STARTUNDO

   m_d.m_rubberthickness = newVal;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Flipper::put_RubberHeight(long newVal)
{
   STARTUNDO

   if (newVal < 0) newVal = 0;
      else if (newVal > 1000) newVal = 50;

   m_d.m_rubberheight = newVal;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Flipper::put_RubberWidth(long newVal)
{
   STARTUNDO

   m_d.m_rubberwidth = newVal;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Flipper::get_Strength(float *pVal)
{
   *pVal = m_d.m_strength;

   return S_OK;
}

STDMETHODIMP Flipper::put_Strength(float newVal)
{
   STARTUNDO

   m_d.m_strength = newVal;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Flipper::get_Visible(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_phitflipper ? m_phitflipper->m_flipperanim.m_fVisible : m_d.m_fVisible);

   return S_OK;
}

STDMETHODIMP Flipper::put_Visible(VARIANT_BOOL newVal)
{
   if (m_phitflipper)
   {
      //m_phitflipper->m_flipperanim.m_fEnabled = m_d.m_fVisible; //rlc error 
      m_phitflipper->m_flipperanim.m_fVisible = VBTOF(newVal);
   }
   else
   {
      STARTUNDO
      m_d.m_fVisible = VBTOF(newVal);
      STOPUNDO
   }
   return S_OK;
}

STDMETHODIMP Flipper::get_Enabled(VARIANT_BOOL *pVal)
{
   *pVal = (VARIANT_BOOL)FTOVB(m_phitflipper ? m_phitflipper->m_flipperanim.m_fEnabled : m_d.m_fEnabled);

   return S_OK;
}

STDMETHODIMP Flipper::put_Enabled(VARIANT_BOOL newVal)
{
   if (m_phitflipper)
   {
      //m_phitflipper->m_flipperanim.m_fEnabled = m_d.m_fVisible; //rlc error 
      m_phitflipper->m_flipperanim.m_fEnabled = VBTOF(newVal);
   }
   else
   {
      STARTUNDO
         m_d.m_fEnabled = VBTOF(newVal);
      STOPUNDO
   }
   return S_OK;
}

STDMETHODIMP Flipper::get_Elasticity(float *pVal)
{
   *pVal = (m_phitflipper) ? m_phitflipper->m_elasticity : m_d.m_elasticity;

   return S_OK;
}

STDMETHODIMP Flipper::put_Elasticity(float newVal)
{
   if (m_phitflipper)
   {
      m_phitflipper->m_elasticity = (m_d.m_OverridePhysics ? m_d.m_OverrideElasticity : m_d.m_elasticity);
   }
   else
   {
      STARTUNDO
      m_d.m_elasticity = newVal;
      STOPUNDO
   }

   return S_OK;
}

STDMETHODIMP Flipper::get_Height(float *pVal)
{
   *pVal = m_d.m_height;

   return S_OK;
}

STDMETHODIMP Flipper::put_Height(float newVal)
{
   STARTUNDO

   m_d.m_height = newVal;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Flipper::get_Mass(float *pVal)
{
   *pVal = m_d.m_mass;

   return S_OK;
}

STDMETHODIMP Flipper::put_Mass(float newVal)
{
   STARTUNDO

   m_d.m_mass = newVal;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Flipper::get_Return(float *pVal)
{
   *pVal = m_d.m_return;

   return S_OK;
}

STDMETHODIMP Flipper::put_Return(float newVal)
{
   STARTUNDO

   if (newVal < 0.f) newVal = 0.f;
      else if (newVal > 1.0f) newVal = 1.0f;

   m_d.m_return = newVal;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Flipper::get_PowerLaw(float *pVal)
{
   *pVal = m_d.m_powerlaw;

   return S_OK;
}

STDMETHODIMP Flipper::put_PowerLaw(float newVal)
{
   STARTUNDO

   if (newVal < 0.0f) newVal = 0.0f;
      else if (newVal > 4.0f) newVal = 4.0f;

   m_d.m_powerlaw = newVal;

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Flipper::get_ObliqueCorrection(float *pVal)
{
   *pVal = RADTOANG(m_d.m_obliquecorrection);

   return S_OK;
}

STDMETHODIMP Flipper::put_ScatterAngle(float newVal)
{
   STARTUNDO

   m_d.m_scatterangle = ANGTORAD(newVal);

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Flipper::get_ScatterAngle(float *pVal)
{
   *pVal = RADTOANG(m_d.m_scatterangle);

   return S_OK;
}

STDMETHODIMP Flipper::put_ObliqueCorrection(float newVal)
{
   STARTUNDO

   m_d.m_obliquecorrection = ANGTORAD(newVal);

   STOPUNDO

   return S_OK;
}

STDMETHODIMP Flipper::get_FlipperRadiusMin(float *pVal)
{
   *pVal = m_d.m_FlipperRadiusMin;

   return S_OK;
}

STDMETHODIMP Flipper::put_FlipperRadiusMin(float newVal)
{
   STARTUNDO

   if (newVal < 0.0f) newVal = 0.0f;

   m_d.m_FlipperRadiusMin = newVal;

   STOPUNDO

   return S_OK;
}
