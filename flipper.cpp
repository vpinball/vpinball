#include "stdafx.h"
#include "meshes/flipperBase.h"
#include "objloader.h"
#include "Shader.h"
#include "IndexBuffer.h"
#include "VertexBuffer.h"

static constexpr float vertsTipBottomf[13 * 3] =
{
   -0.101425f, 0.786319f, 0.003753f,
   -0.097969f, 0.812569f, 0.003753f,
   -0.087837f, 0.837031f, 0.003753f,
   -0.071718f, 0.858037f, 0.003753f,
   -0.050713f, 0.874155f, 0.003753f,
   -0.026251f, 0.884288f, 0.003753f,
   -0.000000f, 0.887744f, 0.003753f,
   0.026251f, 0.884288f, 0.003753f,
   0.050713f, 0.874155f, 0.003753f,
   0.071718f, 0.858037f, 0.003753f,
   0.087837f, 0.837031f, 0.003753f,
   0.097969f, 0.812569f, 0.003753f,
   0.101425f, 0.786319f, 0.003753f
};

static const Vertex3Ds* const vertsTipBottom = (Vertex3Ds*)vertsTipBottomf;

static constexpr float vertsTipTopf[13 * 3] =
{
   -0.101425f, 0.786319f, 1.004253f,
   -0.097969f, 0.812569f, 1.004253f,
   -0.087837f, 0.837031f, 1.004253f,
   -0.071718f, 0.858037f, 1.004253f,
   -0.050713f, 0.874155f, 1.004253f,
   -0.026251f, 0.884288f, 1.004253f,
   -0.000000f, 0.887744f, 1.004253f,
   0.026251f, 0.884288f, 1.004253f,
   0.050713f, 0.874155f, 1.004253f,
   0.071718f, 0.858037f, 1.004253f,
   0.087837f, 0.837031f, 1.004253f,
   0.097969f, 0.812569f, 1.004253f,
   0.101425f, 0.786319f, 1.004253f
};

static const Vertex3Ds* const vertsTipTop = (Vertex3Ds*)vertsTipTopf;

static constexpr float vertsBaseBottomf[13 * 3] =
{
   -0.100762f, -0.000000f, 0.003753f,
   -0.097329f, -0.026079f, 0.003753f,
   -0.087263f, -0.050381f, 0.003753f,
   -0.071250f, -0.071250f, 0.003753f,
   -0.050381f, -0.087263f, 0.003753f,
   -0.026079f, -0.097329f, 0.003753f,
   -0.000000f, -0.100762f, 0.003753f,
   0.026079f, -0.097329f, 0.003753f,
   0.050381f, -0.087263f, 0.003753f,
   0.071250f, -0.071250f, 0.003753f,
   0.087263f, -0.050381f, 0.003753f,
   0.097329f, -0.026079f, 0.003753f,
   0.100762f, -0.000000f, 0.003753f
};

static const Vertex3Ds* const vertsBaseBottom = (Vertex3Ds*)vertsBaseBottomf;

static constexpr float vertsBaseTopf[13 * 3] =
{
   -0.100762f, 0.000000f, 1.004253f,
   -0.097329f, -0.026079f, 1.004253f,
   -0.087263f, -0.050381f, 1.004253f,
   -0.071250f, -0.071250f, 1.004253f,
   -0.050381f, -0.087263f, 1.004253f,
   -0.026079f, -0.097329f, 1.004253f,
   -0.000000f, -0.100762f, 1.004253f,
   0.026079f, -0.097329f, 1.004253f,
   0.050381f, -0.087263f, 1.004253f,
   0.071250f, -0.071250f, 1.004253f,
   0.087263f, -0.050381f, 1.004253f,
   0.097329f, -0.026079f, 1.004253f,
   0.100762f, -0.000000f, 1.004253f
};

static const Vertex3Ds* const vertsBaseTop = (Vertex3Ds*)vertsBaseTopf;

Flipper::Flipper()
{
   m_phitflipper = nullptr;
   m_vertexBuffer = nullptr;
   m_indexBuffer = nullptr;
   m_ptable = nullptr;
}

Flipper::~Flipper()
{
   SAFE_BUFFER_RELEASE(m_vertexBuffer);
   SAFE_BUFFER_RELEASE(m_indexBuffer);
}

HRESULT Flipper::Init(PinTable *ptable, float x, float y, bool fromMouseClick)
{
   m_ptable = ptable;

   m_d.m_Center.x = x;
   m_d.m_Center.y = y;

   SetDefaults(fromMouseClick);

   m_phitflipper = nullptr;

   return InitVBA(fTrue, 0, nullptr);
}

void Flipper::SetDefaults(bool fromMouseClick)
{
#define regKey regKey[RegName::DefaultPropsFlipper]

   SetDefaultPhysics(fromMouseClick);

   m_d.m_StartAngle = fromMouseClick ? LoadValueFloatWithDefault(regKey, "StartAngle"s, 121.f) : 121.f;
   m_d.m_EndAngle = fromMouseClick ? LoadValueFloatWithDefault(regKey, "EndAngle"s, 70.f) : 70.f;
   m_d.m_BaseRadius = fromMouseClick ? LoadValueFloatWithDefault(regKey, "BaseRadius"s, 21.5f) : 21.5f; // 15
   m_d.m_EndRadius = fromMouseClick ? LoadValueFloatWithDefault(regKey, "EndRadius"s, 13.f) : 13.f; // 6
   m_d.m_FlipperRadiusMax = fromMouseClick ? LoadValueFloatWithDefault(regKey, "Length"s, 130.f) : 130.f; // 80
   m_d.m_FlipperRadiusMin = fromMouseClick ? LoadValueFloatWithDefault(regKey, "MaxDifLength"s, 0.f) : 0.f;

   m_d.m_FlipperRadius = m_d.m_FlipperRadiusMax;

   m_d.m_tdr.m_TimerEnabled = fromMouseClick ? LoadValueBoolWithDefault(regKey, "TimerEnabled"s, false) : false;
   m_d.m_tdr.m_TimerInterval = fromMouseClick ? LoadValueIntWithDefault(regKey, "TimerInterval"s, 100) : 100;
   m_d.m_color = fromMouseClick ? LoadValueIntWithDefault(regKey, "Color"s, RGB(255,255,255)) : RGB(255,255,255);
   m_d.m_rubbercolor = fromMouseClick ? LoadValueIntWithDefault(regKey, "RubberColor"s, RGB(128,50,50)) : RGB(128,50,50);

   const HRESULT hr = LoadValue(regKey, "Surface"s, m_d.m_szSurface);
   if ((hr != S_OK) || !fromMouseClick)
      m_d.m_szSurface.clear();

   m_d.m_height = fromMouseClick ? LoadValueFloatWithDefault(regKey, "Height"s, 50.f) : 50.f;
   m_d.m_rubberthickness = fromMouseClick ? LoadValueFloatWithDefault(regKey, "RubberThickness"s, 7.f) : 7.f;
   m_d.m_rubberheight = fromMouseClick ? LoadValueFloatWithDefault(regKey, "RubberHeight"s, 19.f) : 19.f;
   m_d.m_rubberwidth = fromMouseClick ? LoadValueFloatWithDefault(regKey, "RubberWidth"s, 24.f) : 24.f;
   m_d.m_visible = fromMouseClick ? LoadValueBoolWithDefault(regKey, "Visible"s, true) : true;
   m_d.m_enabled = fromMouseClick ? LoadValueBoolWithDefault(regKey, "Enabled"s, true) : true;
   m_d.m_reflectionEnabled = fromMouseClick ? LoadValueBoolWithDefault(regKey, "ReflectionEnabled"s, true) : true;

#undef regKey
}

void Flipper::WriteRegDefaults()
{
#define regKey regKey[RegName::DefaultPropsFlipper]

   SaveValueFloat(regKey, "Scatter"s, m_d.m_scatter);
   SaveValueFloat(regKey, "Strength"s, m_d.m_strength);
   SaveValueFloat(regKey, "EOSTorque"s, m_d.m_torqueDamping);
   SaveValueFloat(regKey, "EOSTorqueAngle"s, m_d.m_torqueDampingAngle);
   SaveValueFloat(regKey, "StartAngle"s, m_d.m_StartAngle);
   SaveValueFloat(regKey, "EndAngle"s, m_d.m_EndAngle);
   SaveValueFloat(regKey, "BaseRadius"s, m_d.m_BaseRadius);
   SaveValueFloat(regKey, "EndRadius"s, m_d.m_EndRadius);
   SaveValueFloat(regKey, "MaxDifLength"s, m_d.m_FlipperRadiusMin);
   SaveValueFloat(regKey, "ReturnStrength"s, m_d.m_return);
   SaveValueFloat(regKey, "Length"s, m_d.m_FlipperRadiusMax);
   SaveValueFloat(regKey, "Mass"s, m_d.m_mass);
   SaveValueFloat(regKey, "Elasticity"s, m_d.m_elasticity);
   SaveValueFloat(regKey, "ElasticityFalloff"s, m_d.m_elasticityFalloff);
   SaveValueFloat(regKey, "Friction"s, m_d.m_friction);
   SaveValueFloat(regKey, "RampUp"s, m_d.m_rampUp);
   SaveValueBool(regKey, "TimerEnabled"s, m_d.m_tdr.m_TimerEnabled);
   SaveValueInt(regKey, "TimerInterval"s, m_d.m_tdr.m_TimerInterval);
   SaveValueInt(regKey, "Color"s, m_d.m_color);
   SaveValueInt(regKey, "RubberColor"s, m_d.m_rubbercolor);
   SaveValue(regKey, "Surface"s, m_d.m_szSurface);
   SaveValueFloat(regKey, "Height"s, m_d.m_height);
   SaveValueFloat(regKey, "RubberThickness"s, m_d.m_rubberthickness);
   SaveValueFloat(regKey, "RubberHeight"s, m_d.m_rubberheight);
   SaveValueFloat(regKey, "RubberWidth"s, m_d.m_rubberwidth);
   SaveValueBool(regKey, "Visible"s, m_d.m_visible);
   SaveValueBool(regKey, "Enabled"s, m_d.m_enabled);
   SaveValueBool(regKey, "ReflectionEnabled"s, m_d.m_reflectionEnabled);

#undef regKey
}


void Flipper::GetTimers(vector<HitTimer*> &pvht)
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

void Flipper::UpdatePhysicsSettings()
{
   if (m_d.m_OverridePhysics || (m_ptable->m_overridePhysicsFlipper && m_ptable->m_overridePhysics))
   {
      const int idx = m_d.m_OverridePhysics ? (m_d.m_OverridePhysics-1) : (m_ptable->m_overridePhysics-1);

      m_d.m_OverrideMass = LoadValueFloatWithDefault(regKey[RegName::Player], "FlipperPhysicsMass" + std::to_string(idx), 1.f);
      if (m_d.m_OverrideMass < 0.0f)
         m_d.m_OverrideMass = m_d.m_mass;

      m_d.m_OverrideStrength = LoadValueFloatWithDefault(regKey[RegName::Player], "FlipperPhysicsStrength" + std::to_string(idx), 2200.f);
      if (m_d.m_OverrideStrength < 0.0f)
         m_d.m_OverrideStrength = m_d.m_strength;

      m_d.m_OverrideElasticity = LoadValueFloatWithDefault(regKey[RegName::Player], "FlipperPhysicsElasticity" + std::to_string(idx), 0.8f);
      if (m_d.m_OverrideElasticity < 0.0f)
         m_d.m_OverrideElasticity = m_d.m_elasticity;

      m_d.m_OverrideScatterAngle = LoadValueFloatWithDefault(regKey[RegName::Player], "FlipperPhysicsScatter" + std::to_string(idx), 0.f);
      if (m_d.m_OverrideScatterAngle < 0.0f)
         m_d.m_OverrideScatterAngle = m_d.m_scatter;

      m_d.m_OverrideReturnStrength = LoadValueFloatWithDefault(regKey[RegName::Player], "FlipperPhysicsReturnStrength" + std::to_string(idx), 0.058f);
      if (m_d.m_OverrideReturnStrength < 0.0f)
         m_d.m_OverrideReturnStrength = m_d.m_return;

      m_d.m_OverrideElasticityFalloff = LoadValueFloatWithDefault(regKey[RegName::Player], "FlipperPhysicsElasticityFalloff" + std::to_string(idx), 0.43f);
      if (m_d.m_OverrideElasticityFalloff < 0.0f)
         m_d.m_OverrideElasticityFalloff = m_d.m_elasticityFalloff;

      m_d.m_OverrideFriction = LoadValueFloatWithDefault(regKey[RegName::Player], "FlipperPhysicsFriction" + std::to_string(idx), 0.6f);
      if (m_d.m_OverrideFriction < 0.0f)
         m_d.m_OverrideFriction = m_d.m_friction;

      m_d.m_OverrideCoilRampUp = LoadValueFloatWithDefault(regKey[RegName::Player], "FlipperPhysicsCoilRampUp" + std::to_string(idx), 3.f);
      if (m_d.m_OverrideCoilRampUp < 0.0f)
         m_d.m_OverrideCoilRampUp = m_d.m_rampUp;

      m_d.m_OverrideTorqueDamping = LoadValueFloatWithDefault(regKey[RegName::Player], "FlipperPhysicsEOSTorque" + std::to_string(idx), 0.75f);
      if (m_d.m_OverrideTorqueDamping < 0.0f)
         m_d.m_OverrideTorqueDamping = m_d.m_torqueDamping;

      m_d.m_OverrideTorqueDampingAngle = LoadValueFloatWithDefault(regKey[RegName::Player], "FlipperPhysicsEOSTorqueAngle" + std::to_string(idx), 6.f);
      if (m_d.m_OverrideTorqueDampingAngle < 0.0f)
         m_d.m_OverrideTorqueDampingAngle = m_d.m_torqueDampingAngle;
   }
}

void Flipper::GetHitShapes(vector<HitObject*> &pvho)
{
   UpdatePhysicsSettings();

   //

   const float height = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_Center.x, m_d.m_Center.y);

   if (m_d.m_FlipperRadiusMin > 0.f && m_d.m_FlipperRadiusMax > m_d.m_FlipperRadiusMin)
   {
      m_d.m_FlipperRadius = m_d.m_FlipperRadiusMax - (m_d.m_FlipperRadiusMax - m_d.m_FlipperRadiusMin) * m_ptable->m_globalDifficulty;
      m_d.m_FlipperRadius = max(m_d.m_FlipperRadius, m_d.m_BaseRadius - m_d.m_EndRadius + 0.05f);
   }
   else m_d.m_FlipperRadius = m_d.m_FlipperRadiusMax;

   HitFlipper * const phf = new HitFlipper(m_d.m_Center, max(m_d.m_BaseRadius, 0.01f), max(m_d.m_EndRadius, 0.01f),
      max(m_d.m_FlipperRadius, 0.01f), ANGTORAD(m_d.m_StartAngle), ANGTORAD(m_d.m_EndAngle), height, height + m_d.m_height, this);

   phf->m_flipperMover.m_enabled = m_d.m_enabled;
   phf->m_flipperMover.m_visible = m_d.m_visible;

   pvho.push_back(phf);

   m_phitflipper = phf;
}

void Flipper::GetHitShapesDebug(vector<HitObject*> &pvho)
{
   const float height = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_Center.x, m_d.m_Center.y);

   if (m_d.m_FlipperRadiusMin > 0.f && m_d.m_FlipperRadiusMax > m_d.m_FlipperRadiusMin)
   {
      m_d.m_FlipperRadius = m_d.m_FlipperRadiusMax - (m_d.m_FlipperRadiusMax - m_d.m_FlipperRadiusMin) * m_ptable->m_globalDifficulty;
      m_d.m_FlipperRadius = max(m_d.m_FlipperRadius, m_d.m_BaseRadius - m_d.m_EndRadius + 0.05f);
   }
   else m_d.m_FlipperRadius = m_d.m_FlipperRadiusMax;

   Hit3DPoly * const pcircle = new Hit3DPoly(m_d.m_Center.x, m_d.m_Center.y, height + m_d.m_height, m_d.m_FlipperRadius + m_d.m_EndRadius, 32);
   pvho.push_back(pcircle);
}

void Flipper::EndPlay()
{
   if (m_phitflipper) // Failed player case
      m_phitflipper = nullptr;
   SAFE_BUFFER_RELEASE(m_vertexBuffer);
   SAFE_BUFFER_RELEASE(m_indexBuffer);

   IEditable::EndPlay();
}

void Flipper::SetVertices(const float basex, const float basey, const float angle, Vertex2D * const pvEndCenter, Vertex2D * const rgvTangents, const float baseradius, const float endradius) const
{
   const float fradius = m_d.m_FlipperRadius;
   const float fa = asinf((baseradius - endradius) / fradius); //face to centerline angle (center to center)

   const float faceNormOffset = (float)(M_PI / 2.0) - fa; //angle of normal when flipper center line at angle zero	

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

void Flipper::UIRenderPass1(Sur * const psur)
{
   const float rubBaseRadius = m_d.m_BaseRadius - m_d.m_rubberthickness;
   const float rubEndRadius = m_d.m_EndRadius - m_d.m_rubberthickness;
   const float anglerad = ANGTORAD(m_d.m_StartAngle);
   //const float anglerad2 = ANGTORAD(m_d.m_EndAngle);

   m_d.m_FlipperRadius = m_d.m_FlipperRadiusMax;

   psur->SetFillColor(m_ptable->RenderSolid() ? m_vpinball->m_fillColor : -1);
   psur->SetBorderColor(-1, false, 0);
   psur->SetLineColor(RGB(0, 0, 0), false, 0);

   Vertex2D vendcenter;
   Vertex2D rgv[4];
   SetVertices(m_d.m_Center.x, m_d.m_Center.y, anglerad, &vendcenter, rgv, m_d.m_BaseRadius, m_d.m_EndRadius);

   psur->SetObject(this);

   psur->Polygon(rgv, 4);
   psur->Ellipse(m_d.m_Center.x, m_d.m_Center.y, m_d.m_BaseRadius);
   psur->Ellipse(vendcenter.x, vendcenter.y, m_d.m_EndRadius);

   // rubber
   SetVertices(m_d.m_Center.x, m_d.m_Center.y, anglerad, &vendcenter, rgv, rubBaseRadius, rubEndRadius);

   psur->SetObject(this);
   psur->SetLineColor(RGB(128, 0, 0), false, 0);

   psur->Polygon(rgv, 4);
   psur->Ellipse(m_d.m_Center.x, m_d.m_Center.y, rubBaseRadius);
   psur->Ellipse(vendcenter.x, vendcenter.y, rubEndRadius);
}

void Flipper::UIRenderPass2(Sur * const psur)
{
   const float anglerad = ANGTORAD(m_d.m_StartAngle);
   const float anglerad2 = ANGTORAD(m_d.m_EndAngle);
   const float rubBaseRadius = m_d.m_BaseRadius - m_d.m_rubberthickness;
   const float rubEndRadius = m_d.m_EndRadius - m_d.m_rubberthickness;

   Vertex2D vendcenter;
   Vertex2D rgv[4];
   SetVertices(m_d.m_Center.x, m_d.m_Center.y, anglerad, &vendcenter, rgv, m_d.m_BaseRadius, m_d.m_EndRadius);

   psur->SetFillColor(m_ptable->RenderSolid() ? m_vpinball->m_fillColor : -1);
   psur->SetBorderColor(-1, false, 0);
   psur->SetLineColor(RGB(0, 0, 0), false, 0);

   psur->SetObject(this);

   psur->Line(rgv[0].x, rgv[0].y, rgv[1].x, rgv[1].y);
   psur->Line(rgv[2].x, rgv[2].y, rgv[3].x, rgv[3].y);

   psur->Arc(m_d.m_Center.x, m_d.m_Center.y, m_d.m_BaseRadius, rgv[0].x, rgv[0].y, rgv[3].x, rgv[3].y);
   psur->Arc(vendcenter.x, vendcenter.y, m_d.m_EndRadius, rgv[2].x, rgv[2].y, rgv[1].x, rgv[1].y);

   //rubber
   SetVertices(m_d.m_Center.x, m_d.m_Center.y, anglerad, &vendcenter, rgv, rubBaseRadius, rubEndRadius);

   psur->SetFillColor(m_ptable->RenderSolid() ? m_vpinball->m_fillColor : -1);
   psur->SetBorderColor(-1, false, 0);
   psur->SetLineColor(RGB(0, 0, 0), false, 0);

   psur->SetObject(this);

   psur->Line(rgv[0].x, rgv[0].y, rgv[1].x, rgv[1].y);
   psur->Line(rgv[2].x, rgv[2].y, rgv[3].x, rgv[3].y);

   psur->Arc(m_d.m_Center.x, m_d.m_Center.y, rubBaseRadius, rgv[0].x, rgv[0].y, rgv[3].x, rgv[3].y);
   psur->Arc(vendcenter.x, vendcenter.y, rubEndRadius, rgv[2].x, rgv[2].y, rgv[1].x, rgv[1].y);


   //draw the flipper up position
   SetVertices(m_d.m_Center.x, m_d.m_Center.y, anglerad2, &vendcenter, rgv, m_d.m_BaseRadius, m_d.m_EndRadius);

   psur->SetLineColor(RGB(128, 128, 128), true, 0);

   psur->Line(rgv[0].x, rgv[0].y, rgv[1].x, rgv[1].y);
   psur->Line(rgv[2].x, rgv[2].y, rgv[3].x, rgv[3].y);

   psur->Arc(m_d.m_Center.x, m_d.m_Center.y, m_d.m_BaseRadius, rgv[0].x, rgv[0].y, rgv[3].x, rgv[3].y);
   psur->Arc(vendcenter.x, vendcenter.y, m_d.m_EndRadius, rgv[2].x, rgv[2].y, rgv[1].x, rgv[1].y);

   rgv[0].x = m_d.m_Center.x + sinf(anglerad) * (m_d.m_FlipperRadius + m_d.m_EndRadius);
   rgv[0].y = m_d.m_Center.y - cosf(anglerad) * (m_d.m_FlipperRadius + m_d.m_EndRadius);

   rgv[1].x = m_d.m_Center.x + sinf(anglerad2) * (m_d.m_FlipperRadius + m_d.m_EndRadius);
   rgv[1].y = m_d.m_Center.y - cosf(anglerad2) * (m_d.m_FlipperRadius + m_d.m_EndRadius);

   if (m_d.m_EndAngle < m_d.m_StartAngle)
      psur->Arc(m_d.m_Center.x, m_d.m_Center.y, m_d.m_FlipperRadius + m_d.m_EndRadius
      , rgv[0].x, rgv[0].y, rgv[1].x, rgv[1].y);
   else psur->Arc(m_d.m_Center.x, m_d.m_Center.y, m_d.m_FlipperRadius + m_d.m_EndRadius
      , rgv[1].x, rgv[1].y, rgv[0].x, rgv[0].y);

   if (m_d.m_FlipperRadiusMin > 0.f && m_d.m_FlipperRadiusMax > m_d.m_FlipperRadiusMin)
   {
      m_d.m_FlipperRadius = (m_ptable->m_globalDifficulty > 0.f) ? m_d.m_FlipperRadiusMin : m_d.m_FlipperRadiusMax;
      m_d.m_FlipperRadius = max(m_d.m_FlipperRadius, m_d.m_BaseRadius - m_d.m_EndRadius + 0.05f);
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

      psur->SetLineColor(RGB(128, 128, 128), true, 0);

      psur->Line(rgv[0].x, rgv[0].y, rgv[1].x, rgv[1].y);
      psur->Line(rgv[2].x, rgv[2].y, rgv[3].x, rgv[3].y);

      psur->Arc(m_d.m_Center.x, m_d.m_Center.y, m_d.m_BaseRadius, rgv[0].x, rgv[0].y, rgv[3].x, rgv[3].y);
      psur->Arc(vendcenter.x, vendcenter.y, m_d.m_EndRadius, rgv[2].x, rgv[2].y, rgv[1].x, rgv[1].y);

      rgv[0].x = m_d.m_Center.x + sinf(anglerad) * (m_d.m_FlipperRadius + m_d.m_EndRadius);
      rgv[0].y = m_d.m_Center.y - cosf(anglerad) * (m_d.m_FlipperRadius + m_d.m_EndRadius);

      rgv[1].x = m_d.m_Center.x + sinf(anglerad2) * (m_d.m_FlipperRadius + m_d.m_EndRadius);
      rgv[1].y = m_d.m_Center.y - cosf(anglerad2) * (m_d.m_FlipperRadius + m_d.m_EndRadius);

      if (m_d.m_EndAngle < m_d.m_StartAngle)
         psur->Arc(m_d.m_Center.x, m_d.m_Center.y, m_d.m_FlipperRadius + m_d.m_EndRadius
         , rgv[0].x, rgv[0].y, rgv[1].x, rgv[1].y);
      else psur->Arc(m_d.m_Center.x, m_d.m_Center.y, m_d.m_FlipperRadius + m_d.m_EndRadius
         , rgv[1].x, rgv[1].y, rgv[0].x, rgv[0].y);

      m_d.m_FlipperRadius = m_d.m_FlipperRadiusMax - (m_d.m_FlipperRadiusMax - m_d.m_FlipperRadiusMin) * m_ptable->m_globalDifficulty;
      m_d.m_FlipperRadius = max(m_d.m_FlipperRadius, m_d.m_BaseRadius - m_d.m_EndRadius + 0.05f);

      SetVertices(m_d.m_Center.x, m_d.m_Center.y, anglerad, &vendcenter, rgv, m_d.m_BaseRadius, m_d.m_EndRadius);

      psur->SetObject(this);

      psur->Line(rgv[0].x, rgv[0].y, rgv[1].x, rgv[1].y);
      psur->Line(rgv[2].x, rgv[2].y, rgv[3].x, rgv[3].y);

      psur->Arc(m_d.m_Center.x, m_d.m_Center.y, m_d.m_BaseRadius, rgv[0].x, rgv[0].y, rgv[3].x, rgv[3].y);
      psur->Arc(vendcenter.x, vendcenter.y, m_d.m_EndRadius, rgv[2].x, rgv[2].y, rgv[1].x, rgv[1].y);

      SetVertices(m_d.m_Center.x, m_d.m_Center.y, anglerad2, &vendcenter, rgv, m_d.m_BaseRadius, m_d.m_EndRadius);

      psur->SetLineColor(RGB(128, 128, 128), true, 0);

      psur->Line(rgv[0].x, rgv[0].y, rgv[1].x, rgv[1].y);
      psur->Line(rgv[2].x, rgv[2].y, rgv[3].x, rgv[3].y);

      psur->Arc(m_d.m_Center.x, m_d.m_Center.y, m_d.m_BaseRadius, rgv[0].x, rgv[0].y, rgv[3].x, rgv[3].y);
      psur->Arc(vendcenter.x, vendcenter.y, m_d.m_EndRadius, rgv[2].x, rgv[2].y, rgv[1].x, rgv[1].y);

      rgv[0].x = m_d.m_Center.x + sinf(anglerad) * (m_d.m_FlipperRadius + m_d.m_EndRadius);
      rgv[0].y = m_d.m_Center.y - cosf(anglerad) * (m_d.m_FlipperRadius + m_d.m_EndRadius);

      rgv[1].x = m_d.m_Center.x + sinf(anglerad2) * (m_d.m_FlipperRadius + m_d.m_EndRadius);
      rgv[1].y = m_d.m_Center.y - cosf(anglerad2) * (m_d.m_FlipperRadius + m_d.m_EndRadius);

      if (m_d.m_EndAngle < m_d.m_StartAngle)
         psur->Arc(m_d.m_Center.x, m_d.m_Center.y, m_d.m_FlipperRadius + m_d.m_EndRadius
         , rgv[0].x, rgv[0].y, rgv[1].x, rgv[1].y);
      else psur->Arc(m_d.m_Center.x, m_d.m_Center.y, m_d.m_FlipperRadius + m_d.m_EndRadius
         , rgv[1].x, rgv[1].y, rgv[0].x, rgv[0].y);
   }

   m_d.m_FlipperRadius = m_d.m_FlipperRadiusMax;
}

void Flipper::SetObjectPos()
{
    m_vpinball->SetObjectPosCur(m_d.m_Center.x, m_d.m_Center.y);
}

void Flipper::MoveOffset(const float dx, const float dy)
{
   m_d.m_Center.x += dx;
   m_d.m_Center.y += dy;
}

Vertex2D Flipper::GetCenter() const
{
   return m_d.m_Center;
}

void Flipper::PutCenter(const Vertex2D& pv)
{
   m_d.m_Center = pv;
}

void Flipper::SetDefaultPhysics(bool fromMouseClick)
{
#define regKey regKey[RegName::DefaultPropsFlipper]

   m_d.m_scatter = fromMouseClick ? LoadValueFloatWithDefault(regKey, "Scatter"s, 0.f) : 0.f;
   m_d.m_strength = fromMouseClick ? LoadValueFloatWithDefault(regKey, "Strength"s, 2200.f) : 2200.f;
   m_d.m_torqueDamping = fromMouseClick ? LoadValueFloatWithDefault(regKey, "EOSTorque"s, 0.75f) : 0.75f;
   m_d.m_torqueDampingAngle = fromMouseClick ? LoadValueFloatWithDefault(regKey, "EOSTorqueAngle"s, 6.f) : 6.f;

   //m_d.m_angleEOS = 0;

   m_d.m_return = fromMouseClick ? LoadValueFloatWithDefault(regKey, "ReturnStrength"s, 0.058f) : 0.058f;

   float fTmp;
   HRESULT hr = LoadValue(regKey, "Mass", fTmp);
   if (hr != S_OK)
      hr = LoadValue(regKey, "Speed", fTmp); // previously Mass was called Speed, deprecated!
   m_d.m_mass = (hr == S_OK) && fromMouseClick ? fTmp : 1.0f;

   m_d.m_elasticity = fromMouseClick ? LoadValueFloatWithDefault(regKey, "Elasticity"s, 0.8f) : 0.8f;
   m_d.m_elasticityFalloff = fromMouseClick ? LoadValueFloatWithDefault(regKey, "ElasticityFalloff"s, 0.43f) : 0.43f;
   m_d.m_friction = fromMouseClick ? LoadValueFloatWithDefault(regKey, "Friction"s, 0.6f) : 0.6f;
   m_d.m_rampUp = fromMouseClick ? LoadValueFloatWithDefault(regKey, "RampUp"s, 3.0f) : 3.0f;

   m_d.m_OverridePhysics = 0;

#undef regKey
}

STDMETHODIMP Flipper::InterfaceSupportsErrorInfo(REFIID riid)
{
   static const IID* arr[] =
   {
      &IID_IFlipper,
   };

   for (size_t i = 0; i < sizeof(arr) / sizeof(arr[0]); i++)
   {
      if (InlineIsEqualGUID(*arr[i], riid))
         return S_OK;
   }
   return S_FALSE;
}

STDMETHODIMP Flipper::RotateToEnd() // power stroke to hit ball, key/button down/pressed
{
   if (m_phitflipper)
   {
      g_pplayer->m_pininput.m_leftkey_down_usec_rotate_to_end = usec(); // debug only
      g_pplayer->m_pininput.m_leftkey_down_frame_rotate_to_end = g_pplayer->m_overall_frames;

      m_phitflipper->m_flipperMover.m_enableRotateEvent = 1;
      m_phitflipper->m_flipperMover.SetSolenoidState(true);
   }

   return S_OK;
}

STDMETHODIMP Flipper::RotateToStart() // return to park, key/button up/released
{
   if (m_phitflipper)
   {
      m_phitflipper->m_flipperMover.m_enableRotateEvent = -1;
      m_phitflipper->m_flipperMover.SetSolenoidState(false);
   }

   return S_OK;
}
void Flipper::RenderDynamic()
{
   RenderDevice * const pd3dDevice = g_pplayer->m_pin3d.m_pd3dPrimaryDevice;
   TRACE_FUNCTION();

   if (m_phitflipper && !m_phitflipper->m_flipperMover.m_visible)
      return;
   if (m_phitflipper == nullptr && !m_d.m_visible)
      return;

   if (m_ptable->m_reflectionEnabled && !m_d.m_reflectionEnabled)
      return;

   const Material * mat = m_ptable->GetMaterial(m_d.m_szMaterial);

   Texture * const pin = m_ptable->GetImage(m_d.m_szImage);
   if (pin)
   {
      pd3dDevice->basicShader->SetTechniqueMetal(SHADER_TECHNIQUE_basic_with_texture, mat->m_bIsMetal);
      // accomodate models with UV coords outside of [0,1]
      pd3dDevice->basicShader->SetTexture(SHADER_tex_base_color, pin, SF_UNDEFINED, SA_REPEAT, SA_REPEAT);
      pd3dDevice->basicShader->SetAlphaTestValue(pin->m_alphaTestValue * (float)(1.0 / 255.0));
      pd3dDevice->basicShader->SetMaterial(mat, pin->m_pdsBuffer->has_alpha());
   }
   else
   {
      pd3dDevice->basicShader->SetTechniqueMetal(SHADER_TECHNIQUE_basic_without_texture, mat->m_bIsMetal);
      pd3dDevice->basicShader->SetMaterial(mat, false);
   }

   pd3dDevice->SetRenderStateDepthBias(0.0f);
   pd3dDevice->SetRenderState(RenderDevice::ZWRITEENABLE, RenderDevice::RS_TRUE);
   pd3dDevice->SetRenderStateCulling(RenderDevice::CULL_CCW);

   Matrix3D matTrafo;
   matTrafo.SetIdentity();
   matTrafo._41 = m_d.m_Center.x;
   matTrafo._42 = m_d.m_Center.y;
   if (m_phitflipper)
   {
      Matrix3D matTemp;
      matTemp.RotateZMatrix(m_phitflipper->m_flipperMover.m_angleCur);
      matTrafo.Multiply(matTemp, matTrafo);
   }
   g_pplayer->UpdateBasicShaderMatrix(matTrafo);
   pd3dDevice->basicShader->Begin();
   pd3dDevice->DrawIndexedPrimitiveVB(RenderDevice::TRIANGLELIST, MY_D3DFVF_NOTEX2_VERTEX, m_vertexBuffer, 0, flipperBaseVertices, m_indexBuffer, 0, flipperBaseNumIndices);
   pd3dDevice->basicShader->End();

   //render rubber
   if (m_d.m_rubberthickness > 0.f)
   {
      mat = m_ptable->GetMaterial(m_d.m_szRubberMaterial);
      if (pin)
      {
         pd3dDevice->basicShader->SetTechniqueMetal(SHADER_TECHNIQUE_basic_with_texture, mat->m_bIsMetal);
         pd3dDevice->basicShader->SetMaterial(mat, pin->m_pdsBuffer->has_alpha());
      }
      else
      {
         pd3dDevice->basicShader->SetTechniqueMetal(SHADER_TECHNIQUE_basic_without_texture, mat->m_bIsMetal);
         pd3dDevice->basicShader->SetMaterial(mat, false);
      }

      pd3dDevice->basicShader->Begin();
      pd3dDevice->DrawIndexedPrimitiveVB(RenderDevice::TRIANGLELIST, MY_D3DFVF_NOTEX2_VERTEX, m_vertexBuffer, flipperBaseVertices, flipperBaseVertices, m_indexBuffer, 0, flipperBaseNumIndices);
      pd3dDevice->basicShader->End();
   }
   g_pplayer->UpdateBasicShaderMatrix();
}

void Flipper::ExportMesh(ObjLoader& loader)
{
   char name[sizeof(m_wzName)/sizeof(m_wzName[0])];
   WideCharToMultiByteNull(CP_ACP, 0, m_wzName, -1, name, sizeof(name), nullptr, nullptr);

   Matrix3D matTrafo, matTemp;
   matTrafo.SetIdentity();
   matTemp.SetIdentity();
   matTrafo._41 = m_d.m_Center.x;
   matTrafo._42 = m_d.m_Center.y;
   matTemp.RotateZMatrix(ANGTORAD(m_d.m_StartAngle));
   matTrafo.Multiply(matTemp, matTrafo);

   Vertex3D_NoTex2 *const flipper = new Vertex3D_NoTex2[flipperBaseVertices * 2];
   GenerateBaseMesh(flipper);

   {
   Vertex3D_NoTex2 *const buf = flipper;
   for (unsigned int i = 0; i < flipperBaseVertices; i++)
   {
      Vertex3Ds vert(buf[i].x, buf[i].y, buf[i].z);
      vert = matTrafo.MultiplyVector(vert);
      buf[i].x = vert.x;
      buf[i].y = vert.y;
      buf[i].z = vert.z;

      vert = Vertex3Ds(buf[i].nx, buf[i].ny, buf[i].nz);
      vert = matTrafo.MultiplyVectorNoTranslate(vert);
      buf[i].nx = vert.x;
      buf[i].ny = vert.y;
      buf[i].nz = vert.z;
   }
   }

   string subObjName = name + "Base"s;
   loader.WriteObjectName(subObjName);
   loader.WriteVertexInfo(flipper, flipperBaseVertices);
   const Material * mat = m_ptable->GetMaterial(m_d.m_szMaterial);
   loader.WriteMaterial(m_d.m_szMaterial, string(), mat);
   loader.UseTexture(m_d.m_szMaterial);
   loader.WriteFaceInfoList(flipperBaseIndices, flipperBaseNumIndices);
   loader.UpdateFaceOffset(flipperBaseVertices);
   if (m_d.m_rubberthickness > 0.f)
   {
      Vertex3D_NoTex2 *buf = &flipper[flipperBaseVertices];
      for (unsigned int i = 0; i < flipperBaseVertices; i++)
      {
         Vertex3Ds vert(buf[i].x, buf[i].y, buf[i].z);
         vert = matTrafo.MultiplyVector(vert);
         buf[i].x = vert.x;
         buf[i].y = vert.y;
         buf[i].z = vert.z;

         vert = Vertex3Ds(buf[i].nx, buf[i].ny, buf[i].nz);
         vert = matTrafo.MultiplyVectorNoTranslate(vert);
         buf[i].nx = vert.x;
         buf[i].ny = vert.y;
         buf[i].nz = vert.z;
      }

      subObjName = name + "Rubber"s;
      loader.WriteObjectName(subObjName);
      loader.WriteVertexInfo(&flipper[flipperBaseVertices], flipperBaseVertices);
      mat = m_ptable->GetMaterial(m_d.m_szRubberMaterial);
      loader.WriteMaterial(m_d.m_szRubberMaterial, string(), mat);
      loader.UseTexture(m_d.m_szRubberMaterial);
      loader.WriteFaceInfoList(flipperBaseIndices, flipperBaseNumIndices);
      loader.UpdateFaceOffset(flipperBaseVertices);
   }

   delete [] flipper;
}

//
// license:GPLv3+
// Ported at: VisualPinball.Engine/VPT/Bumper/BumperMeshGenerator.cs
//

static void ApplyFix(Vertex3D_NoTex2& vert, const Vertex2D& center, const float midAngle, const float radius, const Vertex2D& newCenter, const float fixAngleScale)
{
   float vAngle = atan2f(vert.y - center.y, vert.x - center.x);
   float nAngle = atan2f(vert.ny, vert.nx);
   
   // we want to have angles with same sign as midAngle, fix it:
   if (midAngle < 0.0f)
   {
   	  if (vAngle > 0.0f)
         vAngle -= (float)(M_PI * 2.0);
   	  if (nAngle > 0.0f)
   	     nAngle -= (float)(M_PI * 2.0);
   }
   else
   {
   	  if (vAngle < 0.0f)
         vAngle += (float)(M_PI * 2.0);
   	  if (nAngle < 0.0f)
   	     nAngle += (float)(M_PI * 2.0);
   }
   
   nAngle -= (vAngle - midAngle) * fixAngleScale * sgn(midAngle);
   vAngle -= (vAngle - midAngle) * fixAngleScale * sgn(midAngle);
   const float nL = Vertex2D(vert.nx, vert.ny).Length();
   
   vert.x  = cosf(vAngle) * radius + newCenter.x;
   vert.y  = sinf(vAngle) * radius + newCenter.y;
   vert.nx = cosf(nAngle) * nL;
   vert.ny = sinf(nAngle) * nL;
}

void Flipper::GenerateBaseMesh(Vertex3D_NoTex2 *buf)
{
   Matrix3D fullMatrix;

   fullMatrix.RotateZMatrix(ANGTORAD(180.0f));

   const float height = m_ptable->GetSurfaceHeight(m_d.m_szSurface, m_d.m_Center.x, m_d.m_Center.y);

   // calc angle needed to fix P0 location
   const float sinAngle = clamp((m_d.m_BaseRadius - m_d.m_EndRadius) / m_d.m_FlipperRadius,-1.f,1.f);
   const float fixAngle = asinf(sinAngle);
   const float fixAngleScale = fixAngle * (float)(1./(M_PI * 0.5)); // scale (in relation to 90 deg.)
            // fixAngleScale = 0.0; // note: if you force fixAngleScale = 0.0 then all will look as old/buggy version
   const float baseRadius = m_d.m_BaseRadius - m_d.m_rubberthickness;
   const float endRadius = m_d.m_EndRadius - m_d.m_rubberthickness;
   vector<Vertex3D_NoTex2> temp(flipperBaseVertices);

   // scale the base and tip
   memcpy(temp.data(), flipperBaseMesh, sizeof(Vertex3D_NoTex2)*flipperBaseVertices);
   for (int t = 0; t < 13; t++)
   {
      for (unsigned int i = 0; i < flipperBaseVertices; i++)
      {
         if (temp[i].x == vertsBaseBottom[t].x && temp[i].y == vertsBaseBottom[t].y && temp[i].z == vertsBaseBottom[t].z)
            ApplyFix(temp[i], Vertex2D(vertsBaseBottom[6].x, vertsBaseBottom[0].y), (float)(-M_PI * 0.5), baseRadius, Vertex2D(0.f, 0.f), fixAngleScale);
         if (temp[i].x == vertsTipBottom[t].x && temp[i].y == vertsTipBottom[t].y && temp[i].z == vertsTipBottom[t].z)
            ApplyFix(temp[i], Vertex2D(vertsTipBottom[6].x, vertsTipBottom[0].y), (float)(M_PI * 0.5), endRadius, Vertex2D(0.f, m_d.m_FlipperRadius), fixAngleScale);
         if (temp[i].x == vertsBaseTop[t].x && temp[i].y == vertsBaseTop[t].y && temp[i].z == vertsBaseTop[t].z)
            ApplyFix(temp[i], Vertex2D(vertsBaseBottom[6].x, vertsBaseBottom[0].y), (float)(-M_PI * 0.5), baseRadius, Vertex2D(0.f, 0.f), fixAngleScale);
         if (temp[i].x == vertsTipTop[t].x && temp[i].y == vertsTipTop[t].y && temp[i].z == vertsTipTop[t].z)
            ApplyFix(temp[i], Vertex2D(vertsTipBottom[6].x, vertsTipBottom[0].y), (float)(M_PI * 0.5), endRadius, Vertex2D(0.f, m_d.m_FlipperRadius), fixAngleScale);
      }
   }
   for (unsigned int i = 0; i < flipperBaseVertices; i++)
   {
      Vertex3Ds vert = fullMatrix.MultiplyVector(Vertex3Ds(temp[i].x, temp[i].y, temp[i].z));
      buf[i].x = vert.x;
      buf[i].y = vert.y;
      buf[i].z = vert.z*m_d.m_height*m_ptable->m_BG_scalez[m_ptable->m_BG_current_set] + height;

      vert = fullMatrix.MultiplyVectorNoTranslate(Vertex3Ds(flipperBaseMesh[i].nx, flipperBaseMesh[i].ny, flipperBaseMesh[i].nz));
      buf[i].nx = vert.x;
      buf[i].ny = vert.y;
      buf[i].nz = vert.z;
      buf[i].tu = flipperBaseMesh[i].tu;
      buf[i].tv = flipperBaseMesh[i].tv;
   }

   //rubber
   if (m_d.m_rubberthickness > 0.f)
   {
      memcpy(temp.data(), flipperBaseMesh, sizeof(Vertex3D_NoTex2)*flipperBaseVertices);
      for (int t = 0; t < 13; t++)
      {
         for (unsigned int i = 0; i < flipperBaseVertices; i++)
         {
            if (temp[i].x == vertsBaseBottom[t].x && temp[i].y == vertsBaseBottom[t].y && temp[i].z == vertsBaseBottom[t].z)
               ApplyFix(temp[i], Vertex2D(vertsBaseBottom[6].x, vertsBaseBottom[0].y), (float)(-M_PI * 0.5), baseRadius + m_d.m_rubberthickness, Vertex2D(0.f, 0.f), fixAngleScale);
            if (temp[i].x == vertsTipBottom[t].x && temp[i].y == vertsTipBottom[t].y && temp[i].z == vertsTipBottom[t].z)
               ApplyFix(temp[i], Vertex2D(vertsTipBottom[6].x, vertsTipBottom[0].y), (float)(M_PI * 0.5), endRadius + m_d.m_rubberthickness, Vertex2D(0.f, m_d.m_FlipperRadius), fixAngleScale);
            if (temp[i].x == vertsBaseTop[t].x && temp[i].y == vertsBaseTop[t].y && temp[i].z == vertsBaseTop[t].z)
               ApplyFix(temp[i], Vertex2D(vertsBaseBottom[6].x, vertsBaseBottom[0].y), (float)(-M_PI * 0.5), baseRadius + m_d.m_rubberthickness, Vertex2D(0.f, 0.f), fixAngleScale);
            if (temp[i].x == vertsTipTop[t].x && temp[i].y == vertsTipTop[t].y && temp[i].z == vertsTipTop[t].z)
               ApplyFix(temp[i], Vertex2D(vertsTipBottom[6].x, vertsTipBottom[0].y), (float)(M_PI * 0.5), endRadius + m_d.m_rubberthickness, Vertex2D(0.f, m_d.m_FlipperRadius), fixAngleScale);
         }
      }

      for (unsigned int i = 0; i < flipperBaseVertices; i++)
      {
         Vertex3Ds vert = fullMatrix.MultiplyVector(Vertex3Ds(temp[i].x, temp[i].y, temp[i].z));
         buf[i + flipperBaseVertices].x = vert.x;
         buf[i + flipperBaseVertices].y = vert.y;
         buf[i + flipperBaseVertices].z = vert.z*m_d.m_rubberwidth*m_ptable->m_BG_scalez[m_ptable->m_BG_current_set] + (height + m_d.m_rubberheight);

         vert = fullMatrix.MultiplyVectorNoTranslate(Vertex3Ds(flipperBaseMesh[i].nx, flipperBaseMesh[i].ny, flipperBaseMesh[i].nz));
         buf[i + flipperBaseVertices].nx = vert.x;
         buf[i + flipperBaseVertices].ny = vert.y;
         buf[i + flipperBaseVertices].nz = vert.z;
         buf[i + flipperBaseVertices].tu = flipperBaseMesh[i].tu;
         buf[i + flipperBaseVertices].tv = flipperBaseMesh[i].tv + 0.5f;
      }
   }
}

//
// end of license:GPLv3+, back to 'old MAME'-like
//

void Flipper::RenderSetup()
{
   SAFE_BUFFER_RELEASE(m_indexBuffer);
   m_indexBuffer = IndexBuffer::CreateAndFillIndexBuffer(flipperBaseNumIndices, flipperBaseIndices, PRIMARY_DEVICE);

   SAFE_BUFFER_RELEASE(m_vertexBuffer);
   VertexBuffer::CreateVertexBuffer(flipperBaseVertices * 2, 0, MY_D3DFVF_NOTEX2_VERTEX, &m_vertexBuffer, PRIMARY_DEVICE);

   Vertex3D_NoTex2 *buf;
   m_vertexBuffer->lock(0, 0, (void**)&buf, VertexBuffer::WRITEONLY);
   GenerateBaseMesh(buf);
   m_vertexBuffer->unlock();
}

void Flipper::RenderStatic()
{
}

HRESULT Flipper::SaveData(IStream *pstm, HCRYPTHASH hcrypthash, const bool backupForPlay)
{
   BiffWriter bw(pstm, hcrypthash);

   bw.WriteVector2(FID(VCEN), m_d.m_Center);
   bw.WriteFloat(FID(BASR), m_d.m_BaseRadius);
   bw.WriteFloat(FID(ENDR), m_d.m_EndRadius);
   bw.WriteFloat(FID(FLPR), m_d.m_FlipperRadiusMax);
   //bw.WriteFloat(FID(FAEO), m_d.m_angleEOS);
   bw.WriteFloat(FID(FRTN), m_d.m_return);
   bw.WriteFloat(FID(ANGS), m_d.m_StartAngle);
   bw.WriteFloat(FID(ANGE), m_d.m_EndAngle);
   bw.WriteInt(FID(OVRP), m_d.m_OverridePhysics);
   bw.WriteFloat(FID(FORC), m_d.m_mass);
   bw.WriteBool(FID(TMON), m_d.m_tdr.m_TimerEnabled);
   bw.WriteInt(FID(TMIN), m_d.m_tdr.m_TimerInterval);
   bw.WriteString(FID(SURF), m_d.m_szSurface);
   bw.WriteString(FID(MATR), m_d.m_szMaterial);
   bw.WriteWideString(FID(NAME), m_wzName);
   bw.WriteString(FID(RUMA), m_d.m_szRubberMaterial);
   bw.WriteInt(FID(RTHK), (int)m_d.m_rubberthickness); //!! deprecated, remove
   bw.WriteFloat(FID(RTHF), m_d.m_rubberthickness);
   bw.WriteInt(FID(RHGT), (int)m_d.m_rubberheight); //!! deprecated, remove
   bw.WriteFloat(FID(RHGF), m_d.m_rubberheight);
   bw.WriteInt(FID(RWDT), (int)m_d.m_rubberwidth); //!! deprecated, remove
   bw.WriteFloat(FID(RWDF), m_d.m_rubberwidth);
   bw.WriteFloat(FID(STRG), m_d.m_strength);
   bw.WriteFloat(FID(ELAS), m_d.m_elasticity);
   bw.WriteFloat(FID(ELFO), m_d.m_elasticityFalloff);
   bw.WriteFloat(FID(FRIC), m_d.m_friction);
   bw.WriteFloat(FID(RPUP), m_d.m_rampUp);
   bw.WriteFloat(FID(SCTR), m_d.m_scatter);
   bw.WriteFloat(FID(TODA), m_d.m_torqueDamping);
   bw.WriteFloat(FID(TDAA), m_d.m_torqueDampingAngle);
   bw.WriteBool(FID(VSBL), m_d.m_visible);
   bw.WriteBool(FID(ENBL), m_d.m_enabled);
   bw.WriteFloat(FID(FRMN), m_d.m_FlipperRadiusMin);
   bw.WriteFloat(FID(FHGT), m_d.m_height);
   bw.WriteString(FID(IMAG), m_d.m_szImage);
   bw.WriteBool(FID(REEN), m_d.m_reflectionEnabled);

   ISelect::SaveData(pstm, hcrypthash);

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

bool Flipper::LoadToken(const int id, BiffReader * const pbr)
{
   switch(id)
   {
   case FID(PIID): pbr->GetInt((int *)pbr->m_pdata); break;
   case FID(VCEN): pbr->GetVector2(m_d.m_Center); break;
   case FID(BASR): pbr->GetFloat(m_d.m_BaseRadius); break;
   case FID(ENDR): pbr->GetFloat(m_d.m_EndRadius); break;
   case FID(FLPR): pbr->GetFloat(m_d.m_FlipperRadiusMax); break;
   //case FID(FAEO): pbr->GetFloat(m_d.m_angleEOS); break;
   case FID(FRTN): pbr->GetFloat(m_d.m_return); break;
   case FID(ANGS): pbr->GetFloat(m_d.m_StartAngle); break;
   case FID(ANGE): pbr->GetFloat(m_d.m_EndAngle); break;
   case FID(OVRP): pbr->GetInt(m_d.m_OverridePhysics); break;
   case FID(FORC): pbr->GetFloat(m_d.m_mass); break;
   case FID(TMON): pbr->GetBool(m_d.m_tdr.m_TimerEnabled); break;
   case FID(TMIN):
   {
      pbr->GetInt(m_d.m_tdr.m_TimerInterval);
      //m_d.m_tdr.m_TimerInterval = INT(m_d.m_tdr.m_TimerInterval);
      if (m_d.m_tdr.m_TimerInterval < 1)
         m_d.m_tdr.m_TimerInterval = 100;
      break;
   }
   case FID(SURF): pbr->GetString(m_d.m_szSurface); break;
   case FID(MATR): pbr->GetString(m_d.m_szMaterial); break;
   case FID(RUMA): pbr->GetString(m_d.m_szRubberMaterial); break;
   case FID(NAME): pbr->GetWideString(m_wzName,sizeof(m_wzName)/sizeof(m_wzName[0])); break;
   case FID(RTHK): //!! deprecated, remove
   {
      int rt;
      pbr->GetInt(rt);
      m_d.m_rubberthickness = (float)rt;
      break;
   }
   case FID(RTHF): pbr->GetFloat(m_d.m_rubberthickness); break;
   case FID(RHGT): //!! deprecated, remove
   {
      int rh;
      pbr->GetInt(rh);
      m_d.m_rubberheight = (float)rh;
      break;
   }
   case FID(RHGF): pbr->GetFloat(m_d.m_rubberheight); break;
   case FID(RWDT): //!! deprecated, remove
   {
      int rw;
      pbr->GetInt(rw);
      m_d.m_rubberwidth = (float)rw;
      break;
   }
   case FID(RWDF): pbr->GetFloat(m_d.m_rubberwidth); break;
   case FID(FHGT): pbr->GetFloat(m_d.m_height); break;
   case FID(STRG): pbr->GetFloat(m_d.m_strength); break;
   case FID(ELAS): pbr->GetFloat(m_d.m_elasticity); break;
   case FID(ELFO): pbr->GetFloat(m_d.m_elasticityFalloff); break;
   case FID(FRIC): pbr->GetFloat(m_d.m_friction); break;
   case FID(RPUP): pbr->GetFloat(m_d.m_rampUp); break;
   case FID(SCTR): pbr->GetFloat(m_d.m_scatter); break;
   case FID(TODA): pbr->GetFloat(m_d.m_torqueDamping); break;
   case FID(TDAA): pbr->GetFloat(m_d.m_torqueDampingAngle); break;
   case FID(FRMN): pbr->GetFloat(m_d.m_FlipperRadiusMin); break;
   case FID(VSBL): pbr->GetBool(m_d.m_visible); break;
   case FID(ENBL): pbr->GetBool(m_d.m_enabled); break;
   case FID(REEN): pbr->GetBool(m_d.m_reflectionEnabled); break;
   case FID(IMAG): pbr->GetString(m_d.m_szImage); break;
   default: ISelect::LoadToken(id, pbr); break;
   }
   return true;
}

HRESULT Flipper::InitPostLoad()
{
   if (m_d.m_height > 1000.0f)
      m_d.m_height = 50.0f;
   if (m_d.m_rubberheight > 1000.f)
      m_d.m_rubberheight = 8.f;
   if (m_d.m_rubberthickness > 0.f && m_d.m_height > 16.0f && m_d.m_rubberwidth == 0.f)
      m_d.m_rubberwidth = m_d.m_height - 16.0f;
   if (m_d.m_rubberwidth > 1000.f)
      m_d.m_rubberwidth = m_d.m_height - 16.0f;

   return S_OK;
}

STDMETHODIMP Flipper::get_BaseRadius(float *pVal)
{
   *pVal = m_d.m_BaseRadius;

   return S_OK;
}

STDMETHODIMP Flipper::put_BaseRadius(float newVal)
{
   m_d.m_BaseRadius = newVal;

   return S_OK;
}

STDMETHODIMP Flipper::get_EndRadius(float *pVal)
{
   *pVal = m_d.m_EndRadius;

   return S_OK;
}

STDMETHODIMP Flipper::put_EndRadius(float newVal)
{
   m_d.m_EndRadius = newVal;

   return S_OK;
}

STDMETHODIMP Flipper::get_Length(float *pVal)
{
   *pVal = m_d.m_FlipperRadiusMax;
   return S_OK;
}

STDMETHODIMP Flipper::put_Length(float newVal)
{
   m_d.m_FlipperRadiusMax = newVal;
   return S_OK;
}

STDMETHODIMP Flipper::get_EOSTorque(float *pVal)
{
   *pVal = (m_d.m_OverridePhysics || (m_ptable->m_overridePhysicsFlipper && m_ptable->m_overridePhysics)) ? m_d.m_OverrideTorqueDamping : m_d.m_torqueDamping;

   return S_OK;
}

STDMETHODIMP Flipper::put_EOSTorque(float newVal)
{
    if (m_phitflipper)
    {
        if (!(m_d.m_OverridePhysics || (m_ptable->m_overridePhysicsFlipper && m_ptable->m_overridePhysics)))
           m_d.m_torqueDamping = newVal;
    }
    else
        m_d.m_torqueDamping = newVal;

    return S_OK;
}

STDMETHODIMP Flipper::get_EOSTorqueAngle(float *pVal)
{
   *pVal = (m_d.m_OverridePhysics || (m_ptable->m_overridePhysicsFlipper && m_ptable->m_overridePhysics)) ? m_d.m_OverrideTorqueDampingAngle : m_d.m_torqueDampingAngle;

   return S_OK;
}

STDMETHODIMP Flipper::put_EOSTorqueAngle(float newVal)
{
    if (m_phitflipper)
    {
        if (!(m_d.m_OverridePhysics || (m_ptable->m_overridePhysicsFlipper && m_ptable->m_overridePhysics)))
           m_d.m_torqueDampingAngle = newVal;
    }
    else
        m_d.m_torqueDampingAngle = newVal;

    return S_OK;
}

STDMETHODIMP Flipper::get_StartAngle(float *pVal)
{
   if (m_phitflipper)
      *pVal = RADTOANG(m_phitflipper->m_flipperMover.m_angleStart);
   else
      *pVal = m_d.m_StartAngle;

   return S_OK;
}

STDMETHODIMP Flipper::put_StartAngle(float newVal)
{
   if (m_phitflipper)
      m_phitflipper->m_flipperMover.SetStartAngle(ANGTORAD(newVal));
   else
      m_d.m_StartAngle = newVal;

   return S_OK;
}

STDMETHODIMP Flipper::get_EndAngle(float *pVal)
{
   if (m_phitflipper)
      *pVal = RADTOANG(m_phitflipper->m_flipperMover.m_angleEnd);
   else
      *pVal = m_d.m_EndAngle;

   return S_OK;
}

STDMETHODIMP Flipper::put_EndAngle(float newVal)
{
   if (m_phitflipper)
      m_phitflipper->m_flipperMover.SetEndAngle(ANGTORAD(newVal));
   else
      m_d.m_EndAngle = newVal;

   return S_OK;
}

STDMETHODIMP Flipper::get_CurrentAngle(float *pVal)
{
   if (m_phitflipper)
   {
      *pVal = RADTOANG(m_phitflipper->m_flipperMover.m_angleCur);
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
   m_d.m_Center.x = newVal;

   return S_OK;
}

STDMETHODIMP Flipper::get_Y(float *pVal)
{
   *pVal = m_d.m_Center.y;

   return S_OK;
}

STDMETHODIMP Flipper::put_Y(float newVal)
{
   m_d.m_Center.y = newVal;

   return S_OK;
}

STDMETHODIMP Flipper::get_Surface(BSTR *pVal)
{
   WCHAR wz[MAXTOKEN];
   MultiByteToWideCharNull(CP_ACP, 0, m_d.m_szSurface.c_str(), -1, wz, MAXTOKEN);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP Flipper::put_Surface(BSTR newVal)
{
   char buf[MAXTOKEN];
   WideCharToMultiByteNull(CP_ACP, 0, newVal, -1, buf, MAXTOKEN, nullptr, nullptr);
   m_d.m_szSurface = buf;

   return S_OK;
}

STDMETHODIMP Flipper::get_Material(BSTR *pVal)
{
   WCHAR wz[MAXNAMEBUFFER];
   MultiByteToWideCharNull(CP_ACP, 0, m_d.m_szMaterial.c_str(), -1, wz, MAXNAMEBUFFER);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP Flipper::put_Material(BSTR newVal)
{
   char buf[MAXNAMEBUFFER];
   WideCharToMultiByteNull(CP_ACP, 0, newVal, -1, buf, MAXNAMEBUFFER, nullptr, nullptr);
   m_d.m_szMaterial = buf;

   return S_OK;
}

STDMETHODIMP Flipper::get_Mass(float *pVal)
{
    if (m_phitflipper)
        *pVal = m_phitflipper->m_flipperMover.GetMass();
    else
        *pVal = m_d.m_mass;

   return S_OK;
}

STDMETHODIMP Flipper::put_Mass(float newVal)
{
   if (m_phitflipper)
   {
      if (!(m_d.m_OverridePhysics || (m_ptable->m_overridePhysicsFlipper && m_ptable->m_overridePhysics)))
         m_phitflipper->m_flipperMover.SetMass(newVal);
   }
   else
      m_d.m_mass = newVal;

   return S_OK;
}

STDMETHODIMP Flipper::get_OverridePhysics(PhysicsSet *pVal)
{
   *pVal = (PhysicsSet)m_d.m_OverridePhysics;

   return S_OK;
}

STDMETHODIMP Flipper::put_OverridePhysics(PhysicsSet newVal)
{
   m_d.m_OverridePhysics = (int)newVal;

   if (m_phitflipper)
   {
      UpdatePhysicsSettings();
      m_phitflipper->UpdatePhysicsFromFlipper();
   }

   return S_OK;
}

STDMETHODIMP Flipper::get_RubberMaterial(BSTR *pVal)
{
   WCHAR wz[MAXNAMEBUFFER];
   MultiByteToWideCharNull(CP_ACP, 0, m_d.m_szRubberMaterial.c_str(), -1, wz, MAXNAMEBUFFER);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP Flipper::put_RubberMaterial(BSTR newVal)
{
   char buf[MAXNAMEBUFFER];
   WideCharToMultiByteNull(CP_ACP, 0, newVal, -1, buf, MAXNAMEBUFFER, nullptr, nullptr);
   m_d.m_szRubberMaterial = buf;

   return S_OK;
}

STDMETHODIMP Flipper::get_RubberThickness(float *pVal)
{
   *pVal = m_d.m_rubberthickness;

   return S_OK;
}

STDMETHODIMP Flipper::get_RubberHeight(float *pVal)
{
   *pVal = m_d.m_rubberheight;
   return S_OK;
}

STDMETHODIMP Flipper::get_RubberWidth(float *pVal)
{
   *pVal = m_d.m_rubberwidth;
   return S_OK;
}

STDMETHODIMP Flipper::put_RubberThickness(float newVal)
{
   m_d.m_rubberthickness = newVal;

   return S_OK;
}

STDMETHODIMP Flipper::put_RubberHeight(float newVal)
{
   if (newVal < 0.f) newVal = 0.f;
   else if (newVal > 1000.f) newVal = 50.f; //!! legacy, deprecated
   
   m_d.m_rubberheight = newVal;
   return S_OK;
}

STDMETHODIMP Flipper::put_RubberWidth(float newVal)
{
   m_d.m_rubberwidth = newVal;
   return S_OK;
}

STDMETHODIMP Flipper::get_Strength(float *pVal)
{
   if (m_phitflipper)
       *pVal = m_phitflipper->m_flipperMover.GetStrength();
   else
       *pVal = m_d.m_strength;

   return S_OK;
}

STDMETHODIMP Flipper::put_Strength(float newVal)
{
   if (m_phitflipper)
   {
      if (!(m_d.m_OverridePhysics || (m_ptable->m_overridePhysicsFlipper && m_ptable->m_overridePhysics)))
         m_d.m_strength = newVal;
   }
   else
      m_d.m_strength = newVal;

   return S_OK;
}

STDMETHODIMP Flipper::get_Visible(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_phitflipper ? m_phitflipper->m_flipperMover.m_visible : m_d.m_visible);

   return S_OK;
}

STDMETHODIMP Flipper::put_Visible(VARIANT_BOOL newVal)
{
   if (m_phitflipper)
   {
      m_phitflipper->m_flipperMover.m_visible = VBTOb(newVal); //m_d.m_visible
   }
   else
      m_d.m_visible = VBTOb(newVal);

   return S_OK;
}

STDMETHODIMP Flipper::get_Enabled(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_phitflipper ? m_phitflipper->m_flipperMover.m_enabled : m_d.m_enabled);

   return S_OK;
}

STDMETHODIMP Flipper::put_Enabled(VARIANT_BOOL newVal)
{
   if (m_phitflipper)
   {
      m_phitflipper->m_flipperMover.m_enabled = VBTOb(newVal); //m_d.m_visible
   }
   else
      m_d.m_enabled = VBTOb(newVal);

   return S_OK;
}

STDMETHODIMP Flipper::get_Elasticity(float *pVal)
{
   *pVal = m_phitflipper ? m_phitflipper->m_elasticity : m_d.m_elasticity;

   return S_OK;
}

STDMETHODIMP Flipper::put_Elasticity(float newVal)
{
   if (m_phitflipper)
   {
      if (!(m_d.m_OverridePhysics || (m_ptable->m_overridePhysicsFlipper && m_ptable->m_overridePhysics)))
         m_phitflipper->m_elasticity = newVal;
   }
   else
      m_d.m_elasticity = newVal;

   return S_OK;
}

STDMETHODIMP Flipper::get_Scatter(float *pVal)
{
   *pVal = m_phitflipper ? RADTOANG(m_phitflipper->m_scatter) : m_d.m_scatter;

   return S_OK;
}

STDMETHODIMP Flipper::put_Scatter(float newVal)
{
   if (m_phitflipper)
   {
      if (!(m_d.m_OverridePhysics || (m_ptable->m_overridePhysicsFlipper && m_ptable->m_overridePhysics)))
         m_phitflipper->m_scatter = ANGTORAD(newVal);
   }
   else
      m_d.m_scatter = newVal;

   return S_OK;
}

STDMETHODIMP Flipper::get_ElasticityFalloff(float *pVal)
{
   *pVal = m_phitflipper ? m_phitflipper->m_elasticityFalloff : m_d.m_elasticityFalloff;

   return S_OK;
}

STDMETHODIMP Flipper::put_ElasticityFalloff(float newVal)
{
   SetElasticityFalloff(newVal);

   return S_OK;
}

STDMETHODIMP Flipper::get_Friction(float *pVal)
{
   *pVal = m_phitflipper ? m_phitflipper->m_friction : m_d.m_friction;

   return S_OK;
}

STDMETHODIMP Flipper::put_Friction(float newVal)
{
   if (m_phitflipper)
      m_phitflipper->SetFriction(newVal);
   else
      m_d.m_friction = newVal;

   return S_OK;
}

STDMETHODIMP Flipper::get_RampUp(float *pVal)
{
   *pVal = GetRampUp();

   return S_OK;
}

STDMETHODIMP Flipper::put_RampUp(float newVal)
{
   SetRampUp(newVal);

   return S_OK;
}

STDMETHODIMP Flipper::get_Height(float *pVal)
{
   *pVal = m_d.m_height;
   return S_OK;
}

STDMETHODIMP Flipper::put_Height(float newVal)
{
   m_d.m_height = newVal;
   return S_OK;
}

STDMETHODIMP Flipper::get_Return(float *pVal)
{
   if (m_phitflipper)
       *pVal = m_phitflipper->m_flipperMover.GetReturnRatio();
   else
       *pVal = m_d.m_return;

   return S_OK;
}

STDMETHODIMP Flipper::put_Return(float newVal)
{
   SetReturn(newVal);

   return S_OK;
}

STDMETHODIMP Flipper::get_FlipperRadiusMin(float *pVal)
{
   *pVal = GetFlipperRadiusMin();

   return S_OK;
}

STDMETHODIMP Flipper::put_FlipperRadiusMin(float newVal)
{
   SetFlipperRadiusMin(newVal);

   return S_OK;
}

STDMETHODIMP Flipper::get_Image(BSTR *pVal)
{
   WCHAR wz[MAXTOKEN];
   MultiByteToWideCharNull(CP_ACP, 0, m_d.m_szImage.c_str(), -1, wz, MAXTOKEN);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP Flipper::put_Image(BSTR newVal)
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

STDMETHODIMP Flipper::get_ReflectionEnabled(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_d.m_reflectionEnabled);

   return S_OK;
}

STDMETHODIMP Flipper::put_ReflectionEnabled(VARIANT_BOOL newVal)
{
   m_d.m_reflectionEnabled = VBTOb(newVal);

   return S_OK;
}
