#include "StdAfx.h"
#include "forsyth.h"

const int numIndices = 294;

Flipper::Flipper()
{
   m_phitflipper = NULL;
   vertexBuffer = NULL;
   indexBuffer = NULL;
}

Flipper::~Flipper()
{
    if (vertexBuffer)
        vertexBuffer->release();
    if (indexBuffer)
        indexBuffer->release();
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
   m_d.m_FlipperRadiusMax = (hr == S_OK) && fromMouseClick ? fTmp : 130.f;	// 80

   hr = GetRegStringAsFloat("DefaultProps\\Flipper","MaxDifLength", &fTmp);
   m_d.m_FlipperRadiusMin = (hr == S_OK) && fromMouseClick ? fTmp : 0;

   m_d.m_FlipperRadius = m_d.m_FlipperRadiusMax;

   //m_d.m_angleEOS = 0; //!! reenable?

   hr = GetRegStringAsFloat("DefaultProps\\Flipper","ReturnStrength", &fTmp);
   m_d.m_return = (hr == S_OK) && fromMouseClick ? fTmp : 0.07f;

   hr = GetRegStringAsFloat("DefaultProps\\Flipper","Speed", &fTmp);
   m_d.m_mass = (hr == S_OK) && fromMouseClick ? fTmp : 1.08f;

   hr = GetRegStringAsFloat("DefaultProps\\Flipper","Elasticity", &fTmp);
   m_d.m_elasticity = (hr == S_OK) && fromMouseClick ? fTmp : 0.85f;

   m_d.m_elasticityFalloff = GetRegStringAsFloatWithDefault("DefaultProps\\Flipper","ElasticityFalloff", 0.4f);

   m_d.m_OverridePhysics = 0;

   m_d.m_friction = GetRegStringAsFloatWithDefault("DefaultProps\\Flipper","Friction", 0.85f);
   m_d.m_rampUp = GetRegStringAsFloatWithDefault("DefaultProps\\Flipper","RampUp", 0.0f);

   m_d.m_scatter = 0.0;

   hr = GetRegInt("DefaultProps\\Flipper","TimerEnabled", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_tdr.m_fTimerEnabled = iTmp == 0 ? fFalse : fTrue;
   else
      m_d.m_tdr.m_fTimerEnabled = fFalse;

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
   m_d.m_strength = (hr == S_OK) && fromMouseClick ? fTmp : 2000.0f;

   hr = GetRegStringAsFloat("DefaultProps\\Flipper","Height", &fTmp);
   m_d.m_height = (hr == S_OK) && fromMouseClick ? fTmp : 50;

   hr = GetRegInt("DefaultProps\\Flipper","RubberThickness", &iTmp);
   m_d.m_rubberthickness = (hr == S_OK) && fromMouseClick ? iTmp : 7;

   hr = GetRegInt("DefaultProps\\Flipper","RubberHeight", &iTmp);
   m_d.m_rubberheight = (hr == S_OK) && fromMouseClick ? iTmp : 19;

   hr = GetRegInt("DefaultProps\\Flipper","RubberWidth", &iTmp);
   m_d.m_rubberwidth = (hr == S_OK) && fromMouseClick ? iTmp : 24;

   hr = GetRegInt("DefaultProps\\Flipper","Visible", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_fVisible = iTmp == 0 ? false : true;
   else
      m_d.m_fVisible = true;

   hr = GetRegInt("DefaultProps\\Flipper","Enabled", &iTmp);
   if ((hr == S_OK) && fromMouseClick)
      m_d.m_fEnabled = iTmp == 0 ? false : true;
   else
      m_d.m_fEnabled = true;
}

void Flipper::WriteRegDefaults()
{
   static const char regKey[] = "DefaultProps\\Flipper";

   SetRegValueFloat(regKey,"BaseRadius", m_d.m_BaseRadius);
   SetRegValueFloat(regKey,"StartAngle", m_d.m_StartAngle);
   SetRegValueFloat(regKey,"EndAngle", m_d.m_EndAngle);
   SetRegValueFloat(regKey,"BaseRadius", m_d.m_BaseRadius);
   SetRegValueFloat(regKey,"MaxDifLength", m_d.m_FlipperRadiusMin);
   SetRegValueFloat(regKey,"EndRadius", m_d.m_EndRadius);
   SetRegValueFloat(regKey,"ReturnStrength", m_d.m_return);
   SetRegValueFloat(regKey,"StartAngle", m_d.m_StartAngle);
   SetRegValueFloat(regKey,"EndAngle", m_d.m_EndAngle);
   SetRegValueFloat(regKey,"Length", m_d.m_FlipperRadiusMax);
   SetRegValueFloat(regKey,"MaxDifLength", m_d.m_FlipperRadiusMin);
   SetRegValueFloat(regKey,"ReturnStrength", m_d.m_return);
   SetRegValueFloat(regKey,"Speed", m_d.m_mass);
   SetRegValueFloat(regKey,"Elasticity", m_d.m_elasticity);
   SetRegValueFloat(regKey,"ElasticityFalloff", m_d.m_elasticityFalloff);
   SetRegValueFloat(regKey, "Friction", m_d.m_friction);
   SetRegValueFloat(regKey, "RampUp", m_d.m_rampUp);
   SetRegValueInt(regKey,"TimerEnabled", m_d.m_tdr.m_fTimerEnabled);
   SetRegValueInt(regKey,"TimerInterval", m_d.m_tdr.m_TimerInterval);
   SetRegValueInt(regKey,"Color", m_d.m_color);
   SetRegValueInt(regKey,"RubberColor", m_d.m_rubbercolor);
   SetRegValue(regKey,"Surface", REG_SZ, &m_d.m_szSurface,strlen(m_d.m_szSurface));
   SetRegValueFloat(regKey,"Strength", m_d.m_strength);
   SetRegValueFloat(regKey,"Height", m_d.m_height);
   SetRegValueInt(regKey,"RubberThickness", m_d.m_rubberthickness);
   SetRegValueInt(regKey,"RubberHeight", m_d.m_rubberheight);
   SetRegValueInt(regKey,"RubberWidth", m_d.m_rubberwidth);
   SetRegValueBool(regKey,"Visible", m_d.m_fVisible);
   SetRegValueBool(regKey,"Enabled", m_d.m_fEnabled);
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
 		 m_d.m_OverrideMass = 0.15f;
	     sprintf_s(tmp,256,"FlipperPhysicsMass%d",m_d.m_OverridePhysics-1);
         HRESULT hr = GetRegStringAsFloat("Player", tmp, &m_d.m_OverrideMass);
         if (hr != S_OK)
            m_d.m_OverrideMass = 0.15f;

		 m_d.m_OverrideStrength = 3.f;
	     sprintf_s(tmp,256,"FlipperPhysicsStrength%d",m_d.m_OverridePhysics-1);
         hr = GetRegStringAsFloat("Player", tmp, &m_d.m_OverrideStrength);
         if (hr != S_OK)
            m_d.m_OverrideStrength = 3.f;

 		 m_d.m_OverrideElasticity = 0.55f;
	     sprintf_s(tmp,256,"FlipperPhysicsElasticity%d",m_d.m_OverridePhysics-1);
         hr = GetRegStringAsFloat("Player", tmp, &m_d.m_OverrideElasticity);
         if (hr != S_OK)
            m_d.m_OverrideElasticity = 0.55f;
		 
		 m_d.m_OverrideReturnStrength = 0.09f;
	     sprintf_s(tmp,256,"FlipperPhysicsReturnStrength%d",m_d.m_OverridePhysics-1);
         hr = GetRegStringAsFloat("Player", tmp, &m_d.m_OverrideReturnStrength);
         if (hr != S_OK)
            m_d.m_OverrideReturnStrength = 0.09f;

		 m_d.m_OverrideElasticityFalloff = 0.43f;
	     sprintf_s(tmp,256,"FlipperPhysicsElasticityFalloff%d",m_d.m_OverridePhysics-1);
         hr = GetRegStringAsFloat("Player", tmp, &m_d.m_OverrideElasticityFalloff);
         if (hr != S_OK)
            m_d.m_OverrideElasticityFalloff = 0.43f;

  		 m_d.m_OverrideFriction = 0.8f;
	     sprintf_s(tmp,256,"FlipperPhysicsFriction%d",m_d.m_OverridePhysics-1);
         hr = GetRegStringAsFloat("Player", tmp, &m_d.m_OverrideFriction);
         if (hr != S_OK)
            m_d.m_OverrideFriction = 0.8f;

		 m_d.m_OverrideCoilRampUp = 0.f;
	     sprintf_s(tmp,256,"FlipperPhysicsCoilRampUp%d",m_d.m_OverridePhysics-1);
         hr = GetRegStringAsFloat("Player", tmp, &m_d.m_OverrideCoilRampUp);
         if (hr != S_OK)
            m_d.m_OverrideCoilRampUp = 0.f;
   }

   //

   const float height = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_Center.x, m_d.m_Center.y);

   if (m_d.m_FlipperRadiusMin > 0.f && m_d.m_FlipperRadiusMax > m_d.m_FlipperRadiusMin)
   {
      m_d.m_FlipperRadius = m_d.m_FlipperRadiusMax - (m_d.m_FlipperRadiusMax - m_d.m_FlipperRadiusMin) * m_ptable->m_globalDifficulty;
      m_d.m_FlipperRadius = max(m_d.m_FlipperRadius, m_d.m_BaseRadius - m_d.m_EndRadius +0.05f);
   }
   else m_d.m_FlipperRadius = m_d.m_FlipperRadiusMax;

   const float strength = m_d.m_OverridePhysics ? m_d.m_OverrideStrength : m_d.m_strength;
   const float return_ratio = m_d.m_OverridePhysics ? m_d.m_OverrideReturnStrength : m_d.m_return;
   const float mass = m_d.m_OverridePhysics ? m_d.m_OverrideMass : m_d.m_mass;

   HitFlipper * const phf = new HitFlipper(m_d.m_Center, m_d.m_BaseRadius, m_d.m_EndRadius,
	   m_d.m_FlipperRadius, ANGTORAD(m_d.m_StartAngle), ANGTORAD(m_d.m_EndAngle), height, height + m_d.m_height,
       strength, mass, return_ratio);

   phf->m_elasticity = m_d.m_OverridePhysics ? m_d.m_OverrideElasticity : m_d.m_elasticity;
   phf->SetFriction(m_d.m_OverridePhysics ? m_d.m_OverrideFriction : m_d.m_friction);
   phf->m_scatter = m_d.m_scatter;

   const float coil_ramp_up = m_d.m_OverridePhysics ? m_d.m_OverrideCoilRampUp : m_d.m_rampUp;

   if (coil_ramp_up <= 0.f)
       phf->m_flipperanim.m_torqueRampupSpeed = 1e6f; // set very high for instant coil response
   else
       phf->m_flipperanim.m_torqueRampupSpeed = strength / coil_ramp_up;

   phf->m_flipperanim.m_EnableRotateEvent = 0;
   phf->m_pfe = NULL;

   phf->m_flipperanim.m_fEnabled = m_d.m_fEnabled;
   phf->m_flipperanim.m_fVisible = m_d.m_fVisible;

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
        m_phitflipper = NULL;
    }
    if (vertexBuffer)
    {
        vertexBuffer->release();
        vertexBuffer = NULL;
    }
    if (indexBuffer)
    {
        indexBuffer->release();
        indexBuffer = NULL;
    }

    IEditable::EndPlay();
}

void Flipper::SetVertices(float basex, float basey, const float angle, Vertex2D * const pvEndCenter, Vertex2D * const rgvTangents, const float baseradius, const float endradius) const
{
   const float fradius = m_d.m_FlipperRadius;
   const float fa = asinf((baseradius-endradius)/fradius); //face to centerline angle (center to center)

   const float faceNormOffset = (float)(M_PI/2.0) - fa; //angle of normal when flipper center line at angle zero	

   const float endx = basex + fradius*sinf(angle); //place end radius center
   pvEndCenter->x = endx;
   const float endy = basey - fradius*cosf(angle);
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

   psur->SetFillColor(m_ptable->RenderSolid() ? RGB(192,192,192) : -1);
   psur->SetBorderColor(-1,false,0);
   psur->SetLineColor(RGB(0,0,0), false, 0);

   Vertex2D vendcenter;
   Vertex2D rgv[4];
   SetVertices(m_d.m_Center.x, m_d.m_Center.y, anglerad, &vendcenter, rgv, m_d.m_BaseRadius, m_d.m_EndRadius);

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
   SetVertices(m_d.m_Center.x, m_d.m_Center.y, anglerad, &vendcenter, rgv, m_d.m_BaseRadius, m_d.m_EndRadius);

   psur->SetFillColor(m_ptable->RenderSolid() ? RGB(192,192,192) : -1);
   psur->SetBorderColor(-1,false,0);
   psur->SetLineColor(RGB(0,0,0), false, 0);

   psur->SetObject(this);

   psur->Line(rgv[0].x, rgv[0].y, rgv[1].x, rgv[1].y);
   psur->Line(rgv[2].x, rgv[2].y, rgv[3].x, rgv[3].y);

   psur->Arc(m_d.m_Center.x, m_d.m_Center.y, m_d.m_BaseRadius, rgv[0].x, rgv[0].y, rgv[3].x, rgv[3].y);
   psur->Arc(vendcenter.x, vendcenter.y, m_d.m_EndRadius, rgv[2].x, rgv[2].y, rgv[1].x, rgv[1].y);

   SetVertices(m_d.m_Center.x, m_d.m_Center.y, anglerad2, &vendcenter, rgv, m_d.m_BaseRadius, m_d.m_EndRadius);

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
      SetVertices(m_d.m_Center.x, m_d.m_Center.y, anglerad, &vendcenter, rgv, m_d.m_BaseRadius, m_d.m_EndRadius);

      psur->SetObject(this);

      psur->Line(rgv[0].x, rgv[0].y, rgv[1].x, rgv[1].y);
      psur->Line(rgv[2].x, rgv[2].y, rgv[3].x, rgv[3].y);

      psur->Arc(m_d.m_Center.x, m_d.m_Center.y, m_d.m_BaseRadius, rgv[0].x, rgv[0].y, rgv[3].x, rgv[3].y);
      psur->Arc(vendcenter.x, vendcenter.y, m_d.m_EndRadius, rgv[2].x, rgv[2].y, rgv[1].x, rgv[1].y);

      SetVertices(m_d.m_Center.x, m_d.m_Center.y, anglerad2, &vendcenter, rgv, m_d.m_BaseRadius, m_d.m_EndRadius);

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

      SetVertices(m_d.m_Center.x, m_d.m_Center.y, anglerad, &vendcenter, rgv, m_d.m_BaseRadius, m_d.m_EndRadius);

      psur->SetObject(this);

      psur->Line(rgv[0].x, rgv[0].y, rgv[1].x, rgv[1].y);
      psur->Line(rgv[2].x, rgv[2].y, rgv[3].x, rgv[3].y);

      psur->Arc(m_d.m_Center.x, m_d.m_Center.y, m_d.m_BaseRadius, rgv[0].x, rgv[0].y, rgv[3].x, rgv[3].y);
      psur->Arc(vendcenter.x, vendcenter.y, m_d.m_EndRadius, rgv[2].x, rgv[2].y, rgv[1].x, rgv[1].y);

      SetVertices(m_d.m_Center.x, m_d.m_Center.y, anglerad2, &vendcenter, rgv, m_d.m_BaseRadius, m_d.m_EndRadius);

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
      m_phitflipper->m_flipperanim.SetSolenoidState(true);
   }

   return S_OK;
}

STDMETHODIMP Flipper::RotateToStart() // return to park 
{
   if (m_phitflipper)
   {
      const float startAng =  ANGTORAD(m_d.m_StartAngle);		
      m_phitflipper->m_flipperanim.m_EnableRotateEvent = -1;
      m_phitflipper->m_flipperanim.SetSolenoidState(false);
   }

   return S_OK;
}

void Flipper::PostRenderStatic(RenderDevice* pd3dDevice)
{
    TRACE_FUNCTION();

    if (m_phitflipper && !m_phitflipper->m_flipperanim.m_fVisible)
        return;
    if (m_phitflipper == NULL && !m_d.m_fVisible)
        return;
    
    Pin3D * const ppin3d = &g_pplayer->m_pin3d;

    Material *mat = m_ptable->GetMaterial( m_d.m_szMaterial);
    pd3dDevice->basicShader->SetMaterial(mat);
    pd3dDevice->basicShader->SetTechnique("basic_without_texture");

    pd3dDevice->SetRenderState(RenderDevice::DEPTHBIAS, 0);
    pd3dDevice->SetRenderState(RenderDevice::ZWRITEENABLE, TRUE);

    Matrix3D matTrafo, matTemp;
    matTrafo.SetIdentity();
    matTrafo._41 = m_d.m_Center.x;
    matTrafo._42 = m_d.m_Center.y;
    if ( m_phitflipper )
      matTemp.RotateZMatrix(m_phitflipper->m_flipperanim.m_angleCur);
    matTrafo.Multiply(matTemp, matTrafo);
    g_pplayer->UpdateBasicShaderMatrix(matTrafo);
    
    // render flipper
    pd3dDevice->basicShader->Begin(0);
    pd3dDevice->DrawIndexedPrimitiveVB( D3DPT_TRIANGLELIST, MY_D3DFVF_NOTEX2_VERTEX, vertexBuffer, 0, 108, indexBuffer, 0, numIndices );
    pd3dDevice->basicShader->End();  

    // render rubber
    if (m_d.m_rubberthickness > 0)
    {
       mat =  m_ptable->GetMaterial( m_d.m_szRubberMaterial );
       pd3dDevice->basicShader->SetMaterial(mat);

       pd3dDevice->basicShader->Begin(0);
       pd3dDevice->DrawIndexedPrimitiveVB( D3DPT_TRIANGLELIST, MY_D3DFVF_NOTEX2_VERTEX, vertexBuffer, 108, 108, indexBuffer, 0, numIndices );
       pd3dDevice->basicShader->End();  
    }

    g_pplayer->UpdateBasicShaderMatrix();
}


void Flipper::RenderSetup(RenderDevice* pd3dDevice)
{
    const unsigned long numVertices= 108*2; //*2 for flipper plus rubber
    // 12 + 16 + 32 + 16 + 32
    _ASSERTE(m_phitflipper);
    Pin3D * const ppin3d = &g_pplayer->m_pin3d;

    const float height = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_Center.x, m_d.m_Center.y);

    const float anglerad = ANGTORAD(m_d.m_StartAngle);
    const float anglerad2 = ANGTORAD(m_d.m_EndAngle);

    if (vertexBuffer)
		vertexBuffer->release();
    pd3dDevice->CreateVertexBuffer(numVertices, 0, MY_D3DFVF_NOTEX2_VERTEX, &vertexBuffer);

    // 18 + 3*14 + 3*14 + 6*16*2 = 294
    // first 6 quads: top/front/back sides (no bottom)
    // next 14 tris:  base top cap
    // next 14 tris:  end top cap
    // next 16 quads: base mantle
    // next 16 quads: end mantle
    static WORD idx[numIndices]={0,1,2,0,2,3, 4,5,6,4,6,7, 8,9,10,8,10,11 };
    for( int i=0;i<14;i++ )
    {
        idx[18 + i*3  ] = 12;
        idx[18 + i*3+1] = 12+i+1;
        idx[18 + i*3+2] = 12+i+2;

        idx[60 + i*3  ] = 60;
        idx[60 + i*3+1] = 60+i+1;
        idx[60 + i*3+2] = 60+i+2;
    }

    for(int i=0;i<16;i++ )
    {
        const WORD o = 2 * i;

        idx[102 + i*6  ] = 28 + o;
        idx[102 + i*6+1] = 28 + (o+1)%32;
        idx[102 + i*6+2] = 28 + (o+2)%32;
        idx[102 + i*6+3] = 28 + (o+1)%32;
        idx[102 + i*6+4] = 28 + (o+3)%32;
        idx[102 + i*6+5] = 28 + (o+2)%32;

        idx[102 + 96 + i*6  ] = 76 + o;
        idx[102 + 96 + i*6+1] = 76 + (o+1)%32;
        idx[102 + 96 + i*6+2] = 76 + (o+2)%32;
        idx[102 + 96 + i*6+3] = 76 + (o+1)%32;
        idx[102 + 96 + i*6+4] = 76 + (o+3)%32;
        idx[102 + 96 + i*6+5] = 76 + (o+2)%32;
    }

    if (indexBuffer)
        indexBuffer->release();

	WORD* tmp = reorderForsyth(idx,numIndices/3,numVertices);
    if(tmp != NULL)
    {
       memcpy(idx,tmp,numIndices*sizeof(WORD));
       delete [] tmp;
    }
    indexBuffer = pd3dDevice->CreateAndFillIndexBuffer(numIndices, idx);

    Vertex3D_NoTex2 *buf;
    vertexBuffer->lock(0, 0, (void**)&buf, VertexBuffer::WRITEONLY);

    // Render just the flipper (in standard position, angle=0)
    RenderAtThickness(pd3dDevice, 0.0f, height, m_d.m_BaseRadius - (float)m_d.m_rubberthickness, m_d.m_EndRadius - (float)m_d.m_rubberthickness, m_d.m_height, buf);

    // Render just the rubber.
    if (m_d.m_rubberthickness > 0)
        RenderAtThickness(pd3dDevice, 0.0f, height + (float)m_d.m_rubberheight, m_d.m_BaseRadius, m_d.m_EndRadius, (float)m_d.m_rubberwidth, buf+108);

    vertexBuffer->unlock();
}

void Flipper::RenderStatic(RenderDevice* pd3dDevice)
{
}

static const WORD rgiFlipper1[4] = {0,4,5,1};
static const WORD rgiFlipper2[4] = {2,6,7,3};
static const WORD rgi0123[4] = {0,1,2,3};

void Flipper::RenderAtThickness(RenderDevice* pd3dDevice, float angle, float height,
                                float baseradius, float endradius, float flipperheight, Vertex3D_NoTex2* buf)
{
    Pin3D * const ppin3d = &g_pplayer->m_pin3d;

    Vertex2D vendcenter;
    Vertex2D rgv[4];
    SetVertices(0.0f, 0.0f, angle, &vendcenter, rgv, baseradius, endradius);

    Vertex3D_NoTex2 rgv3D[32];
    for (int l=0;l<8;l++)
    {
        rgv3D[l].x = rgv[l&3].x;
        rgv3D[l].y = rgv[l&3].y;
        rgv3D[l].z = (l<4) ? height + flipperheight + 0.1f : height; // Make flippers a bit taller so they draw above walls
        rgv3D[l].z *= m_ptable->m_BG_scalez[m_ptable->m_BG_current_set];
    }

    unsigned long offset = 0;

    SetNormal<Vertex3D_NoTex2,WORD>(rgv3D, rgi0123, 3, NULL, NULL, 4);
    // Draw top.
    buf[offset    ] = rgv3D[0];
    buf[offset + 1] = rgv3D[1];
    buf[offset + 2] = rgv3D[2];
    buf[offset + 3] = rgv3D[3];
    offset+=4;
    SetNormal<Vertex3D_NoTex2,WORD>(rgv3D, rgiFlipper1, 3, NULL, NULL, 4);
    // Draw front side wall.
    buf[offset    ] = rgv3D[0];
    buf[offset + 1] = rgv3D[4];
    buf[offset + 2] = rgv3D[5];
    buf[offset + 3] = rgv3D[1];
    offset+=4;
    SetNormal<Vertex3D_NoTex2,WORD>(rgv3D, rgiFlipper2, 3, NULL, NULL, 4);
    // Draw back side wall.
    buf[offset    ] = rgv3D[2];
    buf[offset + 1] = rgv3D[6];
    buf[offset + 2] = rgv3D[7];
    buf[offset + 3] = rgv3D[3];
    offset+=4;

    // offset = 12

    // Base circle
    for (int l=0;l<16;l++)
    {
        const float anglel = (float)(M_PI*2.0/16.0)*(float)l;
        rgv3D[l].x = /*m_d.m_Center.x*/ + sinf(anglel)*baseradius;
        rgv3D[l].y = /*m_d.m_Center.y*/ - cosf(anglel)*baseradius;
        rgv3D[l].z = height + flipperheight + 0.1f;
        rgv3D[l].z *= m_ptable->m_BG_scalez[m_ptable->m_BG_current_set];
        rgv3D[l+16].x = rgv3D[l].x;
        rgv3D[l+16].y = rgv3D[l].y;
        rgv3D[l+16].z = height;
        rgv3D[l+16].z *= m_ptable->m_BG_scalez[m_ptable->m_BG_current_set];
    }

    // Draw end caps of cylinders of large ends.
    WORD endCapsIndex[3*14];
    for (int l=0;l<14;l++)
    {
        endCapsIndex[l*3  ] = 0;
        endCapsIndex[l*3+1] = l+1;
        endCapsIndex[l*3+2] = l+2;
        SetNormal(rgv3D, endCapsIndex+l*3, 3);
    }
    memcpy( &buf[offset], rgv3D, sizeof(Vertex3D_NoTex2)*16 );
    offset += 16;

    // offset = 28

    // Draw vertical cylinders at large end of flipper.
    for (int l=0; l<16; l++)
    {
        // set normal according to cylinder surface
        const float anglel = (float)(M_PI*2.0/16.0)*(float)l;
        rgv3D[l].nx = rgv3D[l+16].nx = +sinf(anglel);
        rgv3D[l].ny = rgv3D[l+16].ny = -cosf(anglel);
        rgv3D[l].nz = rgv3D[l+16].nz = 0.0f;

        buf[offset] = rgv3D[l];
        buf[offset+1] = rgv3D[l+16];
        offset += 2;
    }

    // offset = 60

    // End circle.
    for (int l=0;l<16;l++)
    {
        const float anglel = (float)(M_PI*2.0/16.0)*(float)l;
        rgv3D[l].x = vendcenter.x + sinf(anglel)*endradius;
        rgv3D[l].y = vendcenter.y - cosf(anglel)*endradius;
        rgv3D[l].z = height + flipperheight + 0.1f;
        rgv3D[l].z *= m_ptable->m_BG_scalez[m_ptable->m_BG_current_set];
        rgv3D[l+16].x = rgv3D[l].x;
        rgv3D[l+16].y = rgv3D[l].y;
        rgv3D[l+16].z = height;
        rgv3D[l+16].z *= m_ptable->m_BG_scalez[m_ptable->m_BG_current_set];
    }

    // Draw end caps to vertical cylinder at small end.
    for (int l=0;l<14;l++)
        SetNormal(rgv3D, endCapsIndex+l*3, 3);

	memcpy( &buf[offset], rgv3D, sizeof(Vertex3D_NoTex2)*16 );
    offset += 16;

    // offset = 76

    // Draw vertical cylinders at small end.
    for (int l=0;l<16;l++)
    {
        const float anglel = (float)(M_PI*2.0/16.0)*(float)l;
        rgv3D[l].nx = rgv3D[l+16].nx = +sinf(anglel);
        rgv3D[l].ny = rgv3D[l+16].ny = -cosf(anglel);
        rgv3D[l].nz = rgv3D[l+16].nz = 0.0f;

        buf[offset] = rgv3D[l];
        buf[offset+1] = rgv3D[l+16];
        offset += 2;
    }

    // offset = 108
}


HRESULT Flipper::SaveData(IStream *pstm, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
{
   BiffWriter bw(pstm, hcrypthash, hcryptkey);

   bw.WriteStruct(FID(VCEN), &m_d.m_Center, sizeof(Vertex2D));
   bw.WriteFloat(FID(BASR), m_d.m_BaseRadius);
   bw.WriteFloat(FID(ENDR), m_d.m_EndRadius);
   bw.WriteFloat(FID(FLPR), m_d.m_FlipperRadiusMax);
   //bw.WriteFloat(FID(FAEOS), m_d.m_angleEOS);
   bw.WriteFloat(FID(FRTN), m_d.m_return);
   bw.WriteFloat(FID(ANGS), m_d.m_StartAngle);
   bw.WriteFloat(FID(ANGE), m_d.m_EndAngle);
   bw.WriteInt(FID(OVRP), m_d.m_OverridePhysics);
   bw.WriteFloat(FID(FORC), m_d.m_mass);
   bw.WriteBool(FID(TMON), m_d.m_tdr.m_fTimerEnabled);
   bw.WriteInt(FID(TMIN), m_d.m_tdr.m_TimerInterval);
   bw.WriteString(FID(SURF), m_d.m_szSurface);
   bw.WriteString(FID(MATR), m_d.m_szMaterial);
   bw.WriteWideString(FID(NAME), (WCHAR *)m_wzName);
   bw.WriteString(FID(RUMA), m_d.m_szRubberMaterial);
   bw.WriteInt(FID(RTHK), m_d.m_rubberthickness);
   bw.WriteInt(FID(RHGT), m_d.m_rubberheight);
   bw.WriteInt(FID(RWDT), m_d.m_rubberwidth);
   bw.WriteFloat(FID(STRG), m_d.m_strength);
   bw.WriteFloat(FID(ELAS), m_d.m_elasticity);
   bw.WriteFloat(FID(ELFO), m_d.m_elasticityFalloff);
   bw.WriteFloat(FID(FRIC), m_d.m_friction);
   bw.WriteFloat(FID(RPUP), m_d.m_rampUp);
   bw.WriteBool(FID(VSBL), m_d.m_fVisible);
   bw.WriteBool(FID(ENBL), m_d.m_fEnabled);
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
   /*else if (id == FID(FAEOS))
   {
      pbr->GetFloat(&m_d.m_angleEOS);
   }*/
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
      pbr->GetFloat(&m_d.m_mass);
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
   else if (id == FID(MATR))
   {
      pbr->GetString(m_d.m_szMaterial);
   }
   else if (id == FID(RUMA))
   {
      pbr->GetString(m_d.m_szRubberMaterial);
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
   else if (id == FID(ELFO))
   {
      pbr->GetFloat(&m_d.m_elasticityFalloff);
   }
   else if (id == FID(FRIC))
   {
      pbr->GetFloat(&m_d.m_friction);
   }
   else if (id == FID(RPUP))
   {
      pbr->GetFloat(&m_d.m_rampUp);
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

STDMETHODIMP Flipper::get_Material(BSTR *pVal)
{
    WCHAR wz[512];

    MultiByteToWideChar(CP_ACP, 0, m_d.m_szMaterial, -1, wz, 32);
    *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP Flipper::put_Material(BSTR newVal)
{
   STARTUNDO

   WideCharToMultiByte(CP_ACP, 0, newVal, -1, m_d.m_szMaterial, 32, NULL, NULL);

   STOPUNDO

   return S_OK;
}

/*STDMETHODIMP Flipper::get_AngleEOS(float *pVal)
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
}*/

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

STDMETHODIMP Flipper::get_RubberMaterial(BSTR *pVal)
{
    WCHAR wz[512];

    MultiByteToWideChar(CP_ACP, 0, m_d.m_szRubberMaterial, -1, wz, 32);
    *pVal = SysAllocString(wz);

    return S_OK;
}

STDMETHODIMP Flipper::put_RubberMaterial(BSTR newVal)
{
   STARTUNDO

   WideCharToMultiByte(CP_ACP, 0, newVal, -1, m_d.m_szRubberMaterial, 32, NULL, NULL);

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
      //m_phitflipper->m_flipperanim.m_fEnabled = m_d.m_fVisible;
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
      //m_phitflipper->m_flipperanim.m_fEnabled = m_d.m_fVisible;
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

STDMETHODIMP Flipper::get_ElasticityFalloff(float *pVal)
{
    *pVal = m_d.m_elasticityFalloff;
    return S_OK;
}

STDMETHODIMP Flipper::put_ElasticityFalloff(float newVal)
{
    STARTUNDO
    m_d.m_elasticityFalloff = newVal;
    STOPUNDO
    return S_OK;
}

STDMETHODIMP Flipper::get_Friction(float *pVal)
{
   *pVal = (m_phitflipper) ? m_phitflipper->m_friction : m_d.m_friction;

   return S_OK;
}

STDMETHODIMP Flipper::put_Friction(float newVal)
{
   if (m_phitflipper)
   {
      m_phitflipper->m_friction = newVal;
   }
   else
   {
      STARTUNDO
      m_d.m_friction = newVal;
      STOPUNDO
   }

   return S_OK;
}

STDMETHODIMP Flipper::get_RampUp(float *pVal)
{
   *pVal = m_d.m_rampUp;
   return S_OK;
}

STDMETHODIMP Flipper::put_RampUp(float newVal)
{
   STARTUNDO
   m_d.m_rampUp = newVal;
   STOPUNDO

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

STDMETHODIMP Flipper::get_Return(float *pVal)
{
   *pVal = m_d.m_return;

   return S_OK;
}

STDMETHODIMP Flipper::put_Return(float newVal)
{
   STARTUNDO

   m_d.m_return = clamp(newVal, 0.0f, 1.0f);

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
