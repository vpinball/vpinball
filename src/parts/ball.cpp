// license:GPLv3+

#include "core/stdafx.h"
#ifndef __STANDALONE__
#include "vpinball.h"
#endif
#include "meshes/ballMesh.h"

const AntiStretchHelper Ball::m_ash;
unsigned int Ball::m_nextBallID = 0;
unsigned int Ball::GetNextBallID() { unsigned int id = Ball::m_nextBallID; Ball::m_nextBallID++; return id; }

Ball::Ball() : m_id(GetNextBallID())
{
   swprintf_s(m_wzName, std::size(m_wzName), L"LiveBall%d", m_id); // Default name
   m_d.m_useTableRenderSettings = true;
   m_d.m_color = RGB(255, 255, 255);
   m_d.m_bulb_intensity_scale = 1.0f;
   m_d.m_playfieldReflectionStrength = 1.0f;
   m_d.m_reflectionEnabled = true;
   m_d.m_forceReflection = false;
   m_d.m_visible = true;
   m_d.m_decalMode = false;
   m_hitBall.m_d.m_pos = Vertex3Ds(0.f, 0.f, 25.f);
   m_hitBall.m_d.m_radius = 25.f;
   m_hitBall.m_d.m_mass = 1.f;
   m_hitBall.m_pBall = this;
   m_hitBall.m_editable = this;
   m_hitBall.CalcHitBBox(); // need to update here, as only done lazily
}

Ball::~Ball()
{
   assert(m_rd == nullptr);
}


#pragma region Init

Ball *Ball::CopyForPlay(PinTable *live_table) const
{
   STANDARD_EDITABLE_COPY_FOR_PLAY_IMPL(Ball, live_table)
   dst->m_hitBall.m_d.m_pos = m_hitBall.m_d.m_pos;
   dst->m_hitBall.m_d.m_mass = m_hitBall.m_d.m_mass;
   dst->m_hitBall.m_d.m_radius = m_hitBall.m_d.m_radius;
   return dst;
}

HRESULT Ball::Init(PinTable *const ptable, const float x, const float y, const bool fromMouseClick, const bool forPlay)
{
   m_ptable = ptable;
   SetDefaults(fromMouseClick);
   m_hitBall.m_d.m_pos.x = x;
   m_hitBall.m_d.m_pos.y = y;
   m_hitBall.m_d.m_pos.z = m_hitBall.m_d.m_radius;
   return forPlay ? S_OK : InitVBA(fTrue, 0, nullptr);
}

void Ball::SetObjectPos()
{
    m_vpinball->SetObjectPosCur(m_hitBall.m_d.m_pos.x, m_hitBall.m_d.m_pos.y);
}

void Ball::MoveOffset(const float dx, const float dy)
{
   m_hitBall.m_d.m_pos.x += dx;
   m_hitBall.m_d.m_pos.y += dy;
}

Vertex2D Ball::GetCenter() const
{
   return Vertex2D(m_hitBall.m_d.m_pos.x, m_hitBall.m_d.m_pos.y);
}

void Ball::PutCenter(const Vertex2D& pv)
{
   m_hitBall.m_d.m_pos.x = pv.x;
   m_hitBall.m_d.m_pos.y = pv.y;
}

void Ball::SetDefaults(const bool fromMouseClick)
{
#define regKey Settings::DefaultPropsBall
   m_hitBall.m_d.m_mass = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "Mass"s, 1.f) : 1.f;
   m_hitBall.m_d.m_radius = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "Radius"s, 25.f) : 25.f;
   m_d.m_forceReflection = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "ForceReflection"s, false) : false;
   m_d.m_decalMode = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "DecalMode"s, false) : false;
   m_d.m_szImage = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "Image"s, ""s) : "";
   m_d.m_imageDecal = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "DecalImage"s, ""s) : "";
   m_d.m_bulb_intensity_scale = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "BulbIntensityScale"s, 1.f) : 1.f;
   m_d.m_playfieldReflectionStrength = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "PFReflStrength"s, 1.f) : 1.f;
   m_d.m_color = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "Color"s, (int) RGB(255, 255, 255)) : RGB(255, 255, 255);
   m_d.m_pinballEnvSphericalMapping = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "SphereMap"s, true) : true;
   m_d.m_reflectionEnabled = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "ReflectionEnabled"s, true) : true;
   m_d.m_tdr.m_TimerEnabled = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "TimerEnabled"s, false) : false;
   m_d.m_tdr.m_TimerInterval = fromMouseClick ? g_pvp->m_settings.LoadValueWithDefault(regKey, "TimerInterval"s, 100) : 100;
#undef regKey
}

void Ball::WriteRegDefaults()
{
#define regKey Settings::DefaultPropsBall
   g_pvp->m_settings.SaveValue(regKey, "Mass"s, m_hitBall.m_d.m_mass);
   g_pvp->m_settings.SaveValue(regKey, "Radius"s, m_hitBall.m_d.m_radius);
   g_pvp->m_settings.SaveValue(regKey, "ForceReflection"s, m_d.m_forceReflection);
   g_pvp->m_settings.SaveValue(regKey, "DecalMode"s, m_d.m_decalMode);
   g_pvp->m_settings.SaveValue(regKey, "Image"s, m_d.m_szImage);
   g_pvp->m_settings.SaveValue(regKey, "DecalImage"s, m_d.m_imageDecal);
   g_pvp->m_settings.SaveValue(regKey, "BulbIntensityScale"s, m_d.m_bulb_intensity_scale);
   g_pvp->m_settings.SaveValue(regKey, "PFReflStrength"s, m_d.m_playfieldReflectionStrength);
   g_pvp->m_settings.SaveValue(regKey, "Color"s, (int) m_d.m_color);
   g_pvp->m_settings.SaveValue(regKey, "SphereMap"s, m_d.m_pinballEnvSphericalMapping);
   g_pvp->m_settings.SaveValue(regKey, "ReflectionEnabled"s, m_d.m_reflectionEnabled);
   g_pvp->m_settings.SaveValue(regKey, "TimerEnabled"s, m_d.m_tdr.m_TimerEnabled);
   g_pvp->m_settings.SaveValue(regKey, "TimerInterval"s, m_d.m_tdr.m_TimerInterval);
#undef regKey
}

HRESULT Ball::SaveData(IStream *pstm, HCRYPTHASH hcrypthash, const bool saveForUndo)
{
   BiffWriter bw(pstm, hcrypthash);
   bw.WriteVector3(FID(VCEN), m_hitBall.m_d.m_pos);
   bw.WriteFloat(FID(RADI), m_hitBall.m_d.m_radius);
   bw.WriteFloat(FID(MASS), m_hitBall.m_d.m_mass);
   bw.WriteBool(FID(FREF), m_d.m_forceReflection);
   bw.WriteBool(FID(DCMD), m_d.m_decalMode);
   bw.WriteString(FID(IMAG), m_d.m_szImage);
   bw.WriteString(FID(DIMG), m_d.m_imageDecal);
   bw.WriteFloat(FID(BISC), m_d.m_bulb_intensity_scale);
   bw.WriteFloat(FID(PFRF), m_d.m_playfieldReflectionStrength);
   bw.WriteInt(FID(COLR), m_d.m_color);
   bw.WriteBool(FID(SPHR), m_d.m_pinballEnvSphericalMapping);
   bw.WriteBool(FID(REEN), m_d.m_reflectionEnabled);
   bw.WriteBool(FID(TMON), m_d.m_tdr.m_TimerEnabled);
   bw.WriteInt(FID(TMIN), m_d.m_tdr.m_TimerInterval);
   bw.WriteWideString(FID(NAME), m_wzName);
   ISelect::SaveData(pstm, hcrypthash);
   bw.WriteTag(FID(ENDB));
   return S_OK;
}

HRESULT Ball::InitLoad(IStream *pstm, PinTable *ptable, int *pid, int version, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
{
   SetDefaults(false);
   BiffReader br(pstm, this, pid, version, hcrypthash, hcryptkey);
   m_ptable = ptable;
   br.Load();
   return S_OK;
}

bool Ball::LoadToken(const int id, BiffReader *const pbr)
{
   switch(id)
   {
   case FID(PIID): pbr->GetInt((int *)pbr->m_pdata); break;
   case FID(VCEN): pbr->GetVector3(m_hitBall.m_d.m_pos); break;
   case FID(RADI): pbr->GetFloat(m_hitBall.m_d.m_radius); break;
   case FID(MASS): pbr->GetFloat(m_hitBall.m_d.m_mass); break;
   case FID(FREF): pbr->GetBool(m_d.m_forceReflection); break;
   case FID(DCMD): pbr->GetBool(m_d.m_decalMode); break;
   case FID(IMAG): pbr->GetString(m_d.m_szImage); break;
   case FID(DIMG): pbr->GetString(m_d.m_imageDecal); break;
   case FID(BISC): pbr->GetFloat(m_d.m_bulb_intensity_scale); break;
   case FID(PFRF): pbr->GetFloat(m_d.m_playfieldReflectionStrength); break;
   case FID(COLR): pbr->GetInt(m_d.m_color); break;
   case FID(SPHR): pbr->GetBool(m_d.m_pinballEnvSphericalMapping); break;
   case FID(TMON): pbr->GetBool(m_d.m_tdr.m_TimerEnabled); break;
   case FID(TMIN): pbr->GetInt(m_d.m_tdr.m_TimerInterval); break;
   case FID(NAME): pbr->GetWideString(m_wzName,std::size(m_wzName)); break;
   default: ISelect::LoadToken(id, pbr); break;
   }
   return true;
}

HRESULT Ball::InitPostLoad()
{
   return S_OK;
}

#pragma endregion


void Ball::UIRenderPass1(Sur *const psur)
{
}

void Ball::UIRenderPass2(Sur *const psur)
{
   psur->SetBorderColor(RGB(0, 0, 0), false, 0);
   psur->SetFillColor(-1);
   psur->SetObject(this);
   psur->Ellipse(m_hitBall.m_d.m_pos.x, m_hitBall.m_d.m_pos.y, m_hitBall.m_d.m_radius);
}

void Ball::RenderBlueprint(Sur *psur, const bool solid)
{
   psur->SetBorderColor(RGB(0, 0, 0), false, 0);
   psur->SetFillColor(solid ? BLUEPRINT_SOLID_COLOR : -1);
   psur->SetObject(this);
   psur->Ellipse(m_hitBall.m_d.m_pos.x, m_hitBall.m_d.m_pos.y, m_hitBall.m_d.m_radius);
}

void Ball::BeginPlay(vector<HitTimer *> &pvht) { IEditable::BeginPlay(pvht); }

void Ball::EndPlay() { IEditable::EndPlay(); }


#pragma region Physics

void Ball::PhysicSetup(PhysicsEngine* physics, const bool isUI)
{
   if (!isUI)
      physics->AddBall(&m_hitBall);
}

void Ball::PhysicRelease(PhysicsEngine* physics, const bool isUI)
{
   if (!isUI)
      physics->RemoveBall(&m_hitBall);
}

#pragma endregion



#pragma region Rendering

void Ball::RenderSetup(RenderDevice *device)
{
   assert(m_rd == nullptr);
   m_rd = device;

   m_antiStretch = m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "BallAntiStretch"s, false);

   if (m_d.m_useTableRenderSettings)
   {
      m_d.m_bulb_intensity_scale = m_ptable->m_defaultBulbIntensityScaleOnBall;
      m_d.m_decalMode = m_ptable->m_BallDecalMode;
      m_d.m_szImage = m_ptable->m_ballImage;
      m_d.m_pinballEnvSphericalMapping = m_ptable->m_ballSphericalMapping;
      m_d.m_imageDecal = m_ptable->m_ballImageDecal;
   }

   if (m_d.m_useTableRenderSettings && g_pplayer->m_renderer->m_overwriteBallImages && g_pplayer->m_renderer->m_ballImage)
   {
      m_pinballEnv = g_pplayer->m_renderer->m_ballImage;
      m_d.m_pinballEnvSphericalMapping = true;
   }
   else if (m_d.m_szImage.empty())
   {
      m_pinballEnv = nullptr;
      m_d.m_pinballEnvSphericalMapping = true;
   }
   else
      m_pinballEnv = m_ptable->GetImage(m_d.m_szImage);

   if (m_d.m_useTableRenderSettings && g_pplayer->m_renderer->m_overwriteBallImages && g_pplayer->m_renderer->m_decalImage)
      m_pinballDecal = g_pplayer->m_renderer->m_decalImage;
   else if (m_d.m_imageDecal.empty())
      m_pinballDecal = nullptr;
   else
      m_pinballDecal = m_ptable->GetImage(m_d.m_imageDecal);
}

void Ball::RenderRelease()
{
   assert(m_rd != nullptr);
   m_rd = nullptr;
}

void Ball::UpdateAnimation(const float diff_time_msec)
{
   assert(m_rd != nullptr);
   // Animation is updated by physics engine through a MoverObject. No additional visual animation here
}

inline float map_bulblight_to_emission(const Light* const l) // magic mapping of bulblight parameters to "real" lightsource emission
{
   return l->m_currentIntensity * clamp(powf(l->m_d.m_falloff*0.6f, l->m_d.m_falloff_power*0.6f), 0.f, 23000.f); //!! 0.6f,0.6f = magic, also clamp 23000
}

void Ball::Render(const unsigned int renderMask)
{
   assert(m_rd != nullptr);
   const bool isStaticOnly = renderMask & Renderer::STATIC_ONLY;
   const bool isDynamicOnly = renderMask & Renderer::DYNAMIC_ONLY;
   const bool isReflectionPass = renderMask & Renderer::REFLECTION_PASS;
   TRACE_FUNCTION();
   
   if (!m_d.m_visible
    || isStaticOnly
    || (isReflectionPass && !m_d.m_reflectionEnabled))
      return;

   // Adapt z position of ball
   float zheight = m_hitBall.m_d.m_lockedInKicker ? (m_hitBall.m_d.m_pos.z - m_hitBall.m_d.m_radius) : m_hitBall.m_d.m_pos.z;

   // Don't draw reflection if the ball is not on the playfield (e.g. on a ramp/kicker), except if explicitely asked too
   if (isReflectionPass && !m_d.m_forceReflection 
      && ((zheight > m_hitBall.m_d.m_radius + 3.0f) || m_hitBall.m_d.m_lockedInKicker || (m_hitBall.m_d.m_pos.z < m_hitBall.m_d.m_radius - 0.1f)))
      return;

   m_rd->ResetRenderState();
   
   // Set the render state to something that will always display for debug mode
   m_rd->SetRenderState(RenderState::ZENABLE, g_pplayer->m_debugBalls ? RenderState::RS_FALSE : RenderState::RS_TRUE);

   m_rd->m_ballShader->SetVector(SHADER_invTableRes_reflection, 
      1.0f / (g_pplayer->m_ptable->m_right - g_pplayer->m_ptable->m_left),
      1.0f / (g_pplayer->m_ptable->m_bottom - g_pplayer->m_ptable->m_top), 
      clamp(g_pplayer->m_ptable->m_ballPlayfieldReflectionStrength * m_d.m_playfieldReflectionStrength, 0.f, 1.f), 0.f);

   // collect the x nearest lights that can reflect on balls
   vector<Light*>& reflectedLights = g_pplayer->m_renderer->m_ballReflectedLights;
   std::sort(reflectedLights.begin(), reflectedLights.end(), [this](Light* const pLight1, Light* const pLight2) {
      const float dist1 = Vertex3Ds(pLight1->m_d.m_vCenter.x - m_hitBall.m_d.m_pos.x, pLight1->m_d.m_vCenter.y - m_hitBall.m_d.m_pos.y, pLight1->m_d.m_meshRadius + pLight1->m_surfaceHeight - m_hitBall.m_d.m_pos.z).LengthSquared(); //!! z pos
      const float dist2 = Vertex3Ds(pLight2->m_d.m_vCenter.x - m_hitBall.m_d.m_pos.x, pLight2->m_d.m_vCenter.y - m_hitBall.m_d.m_pos.y, pLight2->m_d.m_meshRadius + pLight2->m_surfaceHeight - m_hitBall.m_d.m_pos.z).LengthSquared(); //!! z pos
      return dist1 < dist2;
   });
   #if defined(ENABLE_OPENGL) || defined(ENABLE_BGFX)
   float lightPos[MAX_LIGHT_SOURCES + MAX_BALL_LIGHT_SOURCES][4] = { 0.0f, 0.0f, 0.0f, 0.0f };
   float lightEmission[MAX_LIGHT_SOURCES + MAX_BALL_LIGHT_SOURCES][4] = { 0.0f, 0.0f, 0.0f, 0.0f };
   float *pLightPos = (float *)lightPos, *pLightEm = (float *)lightEmission;
   constexpr int lightStride = 4, lightOfs = 0;
   #elif defined(ENABLE_DX9)
   struct CLight
   {
      float vPos[3];
      float vEmission[3];
   };
   CLight l[MAX_LIGHT_SOURCES + MAX_BALL_LIGHT_SOURCES];
   float *pLightPos = (float *)l, *pLightEm = (float *)l;
   constexpr int lightStride = 6, lightOfs = 3;
   #endif
   vec4 emission = convertColor(g_pplayer->m_ptable->m_Light[0].emission, 1.f);
   emission.x *= g_pplayer->m_ptable->m_lightEmissionScale * g_pplayer->m_renderer->m_globalEmissionScale;
   emission.y *= g_pplayer->m_ptable->m_lightEmissionScale * g_pplayer->m_renderer->m_globalEmissionScale;
   emission.z *= g_pplayer->m_ptable->m_lightEmissionScale * g_pplayer->m_renderer->m_globalEmissionScale;
   for (unsigned int i2 = 0; i2 < MAX_LIGHT_SOURCES; ++i2)
   {
      const int pPos = i2 * lightStride, pEm = pPos + lightOfs;
      memcpy(&pLightPos[pPos], &g_pplayer->m_ptable->m_Light[i2].pos, sizeof(float) * 3);
      memcpy(&pLightEm[pEm], &emission, sizeof(float) * 3);
   }
   for (unsigned int light_i = 0; light_i < MAX_BALL_LIGHT_SOURCES; ++light_i)
   {
      const int pPos = (light_i + MAX_LIGHT_SOURCES) * lightStride, pEm = pPos + lightOfs;
      if (light_i < reflectedLights.size())
      {
         pLightPos[pPos + 0] = reflectedLights[light_i]->m_d.m_vCenter.x;
         pLightPos[pPos + 1] = reflectedLights[light_i]->m_d.m_vCenter.y;
         pLightPos[pPos + 2] = reflectedLights[light_i]->GetCurrentHeight();
         const float c = map_bulblight_to_emission(reflectedLights[light_i]) * m_d.m_bulb_intensity_scale;
         const vec4 color = convertColor(reflectedLights[light_i]->m_d.m_color, 1.f);
         pLightEm[pEm + 0] = color.x * c;
         pLightEm[pEm + 1] = color.y * c;
         pLightEm[pEm + 2] = color.z * c;
      }
      else //!! rather just set the max number of ball lights!?
      {
         pLightPos[pPos + 0] = -100000.0f;
         pLightPos[pPos + 1] = -100000.0f;
         pLightPos[pPos + 2] = -100000.0f;
         pLightEm[pEm + 0] = 0.0f;
         pLightEm[pEm + 1] = 0.0f;
         pLightEm[pEm + 2] = 0.0f;
      }
   }
   #if defined(ENABLE_OPENGL) || defined(ENABLE_BGFX)
   m_rd->m_ballShader->SetFloat4v(SHADER_ballLightPos, (vec4 *)lightPos, MAX_LIGHT_SOURCES + MAX_BALL_LIGHT_SOURCES);
   m_rd->m_ballShader->SetFloat4v(SHADER_ballLightEmission, (vec4 *)lightEmission, MAX_LIGHT_SOURCES + MAX_BALL_LIGHT_SOURCES);
   #elif defined(ENABLE_DX9)
   m_rd->m_ballShader->SetFloat4v(SHADER_ballPackedLights, (vec4 *)l, sizeof(CLight) * (MAX_LIGHT_SOURCES + MAX_BALL_LIGHT_SOURCES) / (4 * sizeof(float)));
   #endif

   // now for a weird hack: make material more rough, depending on how near the nearest lightsource is, to 'emulate' the area of the bulbs (as VP only features point lights so far)
   float Roughness = 0.8f;
   if (!reflectedLights.empty())
   {
       const float dist = Vertex3Ds(reflectedLights[0]->m_d.m_vCenter.x - m_hitBall.m_d.m_pos.x, reflectedLights[0]->m_d.m_vCenter.y - m_hitBall.m_d.m_pos.y, reflectedLights[0]->m_d.m_meshRadius + reflectedLights[0]->m_surfaceHeight - m_hitBall.m_d.m_pos.z).Length(); //!! z pos
       Roughness = min(max(dist*0.006f, 0.4f), Roughness);
   }
   const vec4 rwem(exp2f(10.0f * Roughness + 1.0f), 0.f, 1.f, 0.05f);
   m_rd->m_ballShader->SetVector(SHADER_Roughness_WrapL_Edge_Thickness, &rwem);

   // ************************* draw the ball itself ****************************
   Vertex2D antiStretch(1.f, 1.f);
   if (m_antiStretch)
   {
      // To evaluate projection stretch, we project a few points and compute projected bounds then apply opposite stretching on YZ axis.
      // This is somewhat overkill but the maths to do it directly would be fairly complicated to accomodate for the 3 view setup projections
      // and tests did not show a real performance impact (likely because VPX is mainly GPU bound, not CPU)
      // Note that this will only work if view is screen aligned (x axis is left-right, yz is top-down). If view has some free rotation this will fail.
      const Matrix3D &mvp = g_pplayer->m_renderer->GetMVP().GetModelViewProj(0);
      bool invalid = false;
      float xMin = FLT_MAX, yMin = FLT_MAX, xMax = -FLT_MAX, yMax = -FLT_MAX;
      invalid = m_ash.computeProjBounds(mvp, m_hitBall.m_d.m_pos.x, m_hitBall.m_d.m_pos.y, zheight, m_hitBall.m_d.m_radius, xMin, xMax, yMin, yMax);
      if (!invalid)
      {
         // compute size of the rendered ball on viewport, then apply reversed viewport rotation, then compute stretch correction
         const int w = m_rd->GetCurrentRenderTarget()->GetWidth();
         const int h = m_rd->GetCurrentRenderTarget()->GetHeight();
         const float viewportRot = -ANGTORAD(g_pplayer->m_ptable->mViewSetups[g_pplayer->m_ptable->m_BG_current_set].GetRotation(w, h));
         const float c = cosf(viewportRot), s = sinf(viewportRot);
         const float rx = (xMax - xMin) * (float)w;
         const float ry = (yMax - yMin) * (float)h;
         const float sx = fabsf(c * rx - s * ry);
         const float sy = fabsf(s * rx + c * ry);
         // only shrink ball to avoid artifact of the ball being rendered over resting parts
         /* if (sy > sx) antiStretch.y = sx / sy; else antiStretch.x = sy / sx; */
         // balance stretching/enlarging to avoid having too much size difference between ball at the top / ball at the bottom
         antiStretch.x = 0.5f * (1.0f + sy / sx);
         antiStretch.y = 0.5f * (1.0f + sx / sy);
      }
   }

   const vec4 diffuse = convertColor(m_d.m_color, 1.0f);
   m_rd->m_ballShader->SetVector(SHADER_cBase_Alpha, &diffuse);
   if (diffuse.w < 1.0f)
   {
      m_rd->SetRenderState(RenderState::ALPHABLENDENABLE, RenderState::RS_TRUE);
      m_rd->SetRenderState(RenderState::SRCBLEND, RenderState::SRC_ALPHA);
      m_rd->SetRenderState(RenderState::DESTBLEND, RenderState::INVSRC_ALPHA);
      m_rd->SetRenderState(RenderState::BLENDOP, RenderState::BLENDOP_ADD);
   }
   else
   {
      m_rd->SetRenderState(RenderState::ALPHABLENDENABLE, RenderState::RS_FALSE);
   }

   Matrix3D rot(m_hitBall.m_orientation.m_d[0][0], m_hitBall.m_orientation.m_d[1][0], m_hitBall.m_orientation.m_d[2][0], 0.0f,
                m_hitBall.m_orientation.m_d[0][1], m_hitBall.m_orientation.m_d[1][1], m_hitBall.m_orientation.m_d[2][1], 0.0f,
                m_hitBall.m_orientation.m_d[0][2], m_hitBall.m_orientation.m_d[1][2], m_hitBall.m_orientation.m_d[2][2], 0.0f,
                0.f, 0.f, 0.f, 1.f);
   Matrix3D scale = Matrix3D::MatrixScale(m_hitBall.m_d.m_radius * antiStretch.x, m_hitBall.m_d.m_radius * antiStretch.y, m_hitBall.m_d.m_radius * antiStretch.y);
   Matrix3D trans = Matrix3D::MatrixTranslate(m_hitBall.m_d.m_pos.x, m_hitBall.m_d.m_pos.y, zheight);
   Matrix3D m3D_full = rot * scale * trans;
   m_rd->m_ballShader->SetMatrix(SHADER_orientation, &m3D_full);

   m_rd->SetRenderState(RenderState::ZWRITEENABLE, RenderState::RS_TRUE);
   bool sphericalMapping;
   if (!m_pinballEnv)
   {
      sphericalMapping = false; // Environment texture is an equirectangular map
      m_rd->m_ballShader->SetTexture(SHADER_tex_ball_color, &g_pplayer->m_renderer->m_pinballEnvTexture);
   }
   else
   {
      sphericalMapping = m_d.m_pinballEnvSphericalMapping;
      m_rd->m_ballShader->SetTexture(SHADER_tex_ball_color, m_pinballEnv);
   }
   if (m_pinballDecal)
      m_rd->m_ballShader->SetTexture(SHADER_tex_ball_decal, m_pinballDecal);
   else
      m_rd->m_ballShader->SetTextureNull(SHADER_tex_ball_decal);
   m_rd->m_ballShader->SetTechnique(sphericalMapping ? m_d.m_decalMode ? SHADER_TECHNIQUE_RenderBall_SphericalMap_DecalMode : SHADER_TECHNIQUE_RenderBall_SphericalMap
                                                     : m_d.m_decalMode ? SHADER_TECHNIQUE_RenderBall_DecalMode : SHADER_TECHNIQUE_RenderBall);
   Vertex3Ds pos(m_hitBall.m_d.m_pos.x, m_hitBall.m_d.m_pos.y, zheight);
   m_rd->DrawMesh(m_rd->m_ballShader, false, pos, 0.f, g_pplayer->m_renderer->m_ballMeshBuffer, RenderDevice::TRIANGLELIST, 0, g_pplayer->m_renderer->m_ballMeshBuffer->m_ib->m_count);

   // draw debug points for visualizing ball rotation (this uses point rendering which is a deprecated feature, not available in OpenGL ES)
   #if defined(DEBUG_BALL_SPIN) && !defined(__OPENGLES__)
   if (g_pplayer->m_liveUI->IsShowingFPSDetails())
   {
      const float pointSize = 5.f * (float)m_rd->GetCurrentRenderTarget()->GetWidth() / 1920.0f;
      // this is buggy as we set the point size directly while the render command is used later on, but this is the only place where point rendering is used so it's ok for now
      #if defined(ENABLE_BGFX)
      // FIXME BGFX implement point rendering for ball debug
      #elif defined(ENABLE_OPENGL)
      glPointSize(pointSize);
      #elif defined(ENABLE_DX9)
      CHECKD3D(m_rd->GetCoreDevice()->SetRenderState(D3DRS_POINTSIZE, float_as_uint(pointSize)));
      #endif
      m_rd->ResetRenderState();
      m_rd->m_ballShader->SetTechnique(SHADER_TECHNIQUE_RenderBall_Debug);
      m_rd->DrawMesh(m_rd->m_ballShader, false, pos, 0.f, g_pplayer->m_renderer->m_ballDebugPoints, RenderDevice::POINTLIST, 0, g_pplayer->m_renderer->m_ballDebugPoints->m_vb->m_count);
   }
   #endif

   // ball trails (except in reflection passes)
   if (g_pplayer->m_renderer->m_trailForBalls && g_pplayer->m_renderer->m_ballTrailStrength > 0.f && !isReflectionPass)
   {
      Vertex3D_NoTex2 vertices[MAX_BALL_TRAIL_POS * 2];
      unsigned int nVertices = 0;
      for (int i2 = 0; i2 < MAX_BALL_TRAIL_POS - 1; ++i2)
      {
         int i3 = m_hitBall.m_ringcounter_oldpos / (10000 / PHYSICS_STEPTIME) - i2;
         if (i3 < 0)
            i3 += MAX_BALL_TRAIL_POS;
         int io = i3 - 1;
         if (io < 0)
            io += MAX_BALL_TRAIL_POS;
         if ((m_hitBall.m_oldpos[i3].x == FLT_MAX) && (m_hitBall.m_oldpos[io].x == FLT_MAX))
            continue; // No position data => discard

         Vertex3Ds vec(m_hitBall.m_oldpos[io].x - m_hitBall.m_oldpos[i3].x, m_hitBall.m_oldpos[io].y - m_hitBall.m_oldpos[i3].y, m_hitBall.m_oldpos[io].z - m_hitBall.m_oldpos[i3].z);
         const float ls = vec.LengthSquared();
         if (ls <= 1e-3f)
            continue; // Too small => discard

         const float length = sqrtf(ls);
         const float bc = g_pplayer->m_renderer->m_ballTrailStrength * powf(1.f - 1.f / max(length, 1.0f), 64.0f); //!! 64=magic alpha falloff
         const float r = min(m_hitBall.m_d.m_radius*0.9f, 2.0f*m_hitBall.m_d.m_radius / powf((float)(i2 + 2), 0.6f)); //!! consts are for magic radius falloff
         if (bc <= 0.f && r <= 1e-3f)
            continue; // Fully faded out or radius too small => discard

         vec *= 1.0f / sqrtf(ls);
         const Vertex3Ds up(0.f, 0.f, 1.f); // TODO Should be camera axis instead of fixed vertical
         const Vertex3Ds n = CrossProduct(vec, up) * r;

         Vertex3D_NoTex2 quadVertices[4];
         quadVertices[0].x = m_hitBall.m_oldpos[i3].x - n.x;
         quadVertices[0].y = m_hitBall.m_oldpos[i3].y - n.y;
         quadVertices[0].z = m_hitBall.m_oldpos[i3].z - n.z;
         quadVertices[1].x = m_hitBall.m_oldpos[i3].x + n.x;
         quadVertices[1].y = m_hitBall.m_oldpos[i3].y + n.y;
         quadVertices[1].z = m_hitBall.m_oldpos[i3].z + n.z;
         quadVertices[2].x = m_hitBall.m_oldpos[io].x + n.x;
         quadVertices[2].y = m_hitBall.m_oldpos[io].y + n.y;
         quadVertices[2].z = m_hitBall.m_oldpos[io].z + n.z;
         quadVertices[3].x = m_hitBall.m_oldpos[io].x - n.x;
         quadVertices[3].y = m_hitBall.m_oldpos[io].y - n.y;
         quadVertices[3].z = m_hitBall.m_oldpos[io].z - n.z;

         quadVertices[0].nx = quadVertices[1].nx = quadVertices[2].nx = quadVertices[3].nx = bc; //!! abuses normal for now for the color/alpha

         quadVertices[0].tu = 0.5f + (float)(i2) * (float)(1.0 / (2.0 * (MAX_BALL_TRAIL_POS - 1)));
         quadVertices[0].tv = 0.f;
         quadVertices[1].tu = quadVertices[0].tu;
         quadVertices[1].tv = 1.f;
         quadVertices[2].tu = 0.5f + (float)(i2 + 1) * (float)(1.0 / (2.0 * (MAX_BALL_TRAIL_POS - 1)));
         quadVertices[2].tv = 1.f;
         quadVertices[3].tu = quadVertices[2].tu;
         quadVertices[3].tv = 0.f;

         if (nVertices == 0)
         { // First quad: just commit it
            vertices[0] = quadVertices[0];
            vertices[1] = quadVertices[1];
            vertices[2] = quadVertices[3];
            vertices[3] = quadVertices[2];
            nVertices += 4;
         }
         else
         { // Following quads: blend with the previous points
            vertices[nVertices - 2].x = (quadVertices[0].x + vertices[nVertices - 2].x) * 0.5f;
            vertices[nVertices - 2].y = (quadVertices[0].y + vertices[nVertices - 2].y) * 0.5f;
            vertices[nVertices - 2].z = (quadVertices[0].z + vertices[nVertices - 2].z) * 0.5f;
            vertices[nVertices - 1].x = (quadVertices[1].x + vertices[nVertices - 1].x) * 0.5f;
            vertices[nVertices - 1].y = (quadVertices[1].y + vertices[nVertices - 1].y) * 0.5f;
            vertices[nVertices - 1].z = (quadVertices[1].z + vertices[nVertices - 1].z) * 0.5f;
            vertices[nVertices    ] =    quadVertices[3];
            vertices[nVertices + 1] =    quadVertices[2];
            nVertices += 2;
         }
      }
      if (nVertices > 0 && g_pplayer->m_renderer->m_ballTrailMeshBufferPos + nVertices <= g_pplayer->m_renderer->m_ballTrailMeshBuffer->m_vb->m_count)
      {
         Vertex3D_NoTex2 *bufvb;
         g_pplayer->m_renderer->m_ballTrailMeshBuffer->m_vb->Lock(bufvb, g_pplayer->m_renderer->m_ballTrailMeshBufferPos * sizeof(Vertex3D_NoTex2), nVertices * sizeof(Vertex3D_NoTex2));
         memcpy(bufvb, vertices, nVertices * sizeof(Vertex3D_NoTex2));
         g_pplayer->m_renderer->m_ballTrailMeshBuffer->m_vb->Unlock();
         m_rd->ResetRenderState();
         m_rd->SetRenderState(RenderState::CULLMODE, RenderState::CULL_NONE);
         m_rd->SetRenderState(RenderState::ZWRITEENABLE, RenderState::RS_FALSE);
         m_rd->SetRenderState(RenderState::ALPHABLENDENABLE, RenderState::RS_TRUE);
         m_rd->SetRenderState(RenderState::SRCBLEND, RenderState::SRC_ALPHA);
         m_rd->SetRenderState(RenderState::DESTBLEND, RenderState::INVSRC_ALPHA);
         m_rd->SetRenderState(RenderState::BLENDOP, RenderState::BLENDOP_ADD);
         m_rd->m_ballShader->SetTechnique(SHADER_TECHNIQUE_RenderBallTrail);
         m_rd->DrawMesh(m_rd->m_ballShader, true, pos, 0.f, g_pplayer->m_renderer->m_ballTrailMeshBuffer, RenderDevice::TRIANGLESTRIP, g_pplayer->m_renderer->m_ballTrailMeshBufferPos, nVertices);
         g_pplayer->m_renderer->m_ballTrailMeshBufferPos += nVertices;
      }
   }
}

#pragma endregion


#pragma region ScriptProxy

STDMETHODIMP Ball::get_X(float *pVal)
{
   *pVal = m_hitBall.m_d.m_pos.x;
   return S_OK;
}

STDMETHODIMP Ball::put_X(float newVal)
{
   m_hitBall.m_d.m_pos.x = newVal;
   return S_OK;
}

STDMETHODIMP Ball::get_Y(float *pVal)
{
   *pVal = m_hitBall.m_d.m_pos.y;
   return S_OK;
}

STDMETHODIMP Ball::put_Y(float newVal)
{
   m_hitBall.m_d.m_pos.y = newVal;
   return S_OK;
}

STDMETHODIMP Ball::get_Z(float *pVal)
{
   *pVal = m_hitBall.m_d.m_pos.z;
   return S_OK;
}

STDMETHODIMP Ball::put_Z(float newVal)
{
   m_hitBall.m_d.m_pos.z = newVal;
   return S_OK;
}

STDMETHODIMP Ball::get_VelX(float *pVal)
{
   *pVal = m_hitBall.m_d.m_vel.x;
   return S_OK;
}

STDMETHODIMP Ball::put_VelX(float newVal)
{
   m_hitBall.m_d.m_vel.x = newVal;
   return S_OK;
}

STDMETHODIMP Ball::get_VelY(float *pVal)
{
   *pVal = m_hitBall.m_d.m_vel.y;
   return S_OK;
}

STDMETHODIMP Ball::put_VelY(float newVal)
{
   m_hitBall.m_d.m_vel.y = newVal;
   return S_OK;
}

STDMETHODIMP Ball::get_VelZ(float *pVal)
{
   *pVal = m_hitBall.m_d.m_vel.z;
   return S_OK;
}

STDMETHODIMP Ball::put_VelZ(float newVal)
{
   m_hitBall.m_d.m_vel.z = newVal;
   return S_OK;
}

STDMETHODIMP Ball::get_AngVelX(float *pVal)
{
   *pVal = m_hitBall.m_angularmomentum.x / m_hitBall.Inertia();
   return S_OK;
}

STDMETHODIMP Ball::get_AngVelY(float *pVal)
{
   *pVal = m_hitBall.m_angularmomentum.y / m_hitBall.Inertia();
   return S_OK;
}

STDMETHODIMP Ball::get_AngVelZ(float *pVal)
{
   *pVal = m_hitBall.m_angularmomentum.z / m_hitBall.Inertia();
   return S_OK;
}

STDMETHODIMP Ball::get_AngMomX(float *pVal)
{
   *pVal = m_hitBall.m_angularmomentum.x;
   return S_OK;
}

STDMETHODIMP Ball::put_AngMomX(float newVal)
{
   m_hitBall.m_angularmomentum.x = newVal;
   return S_OK;
}

STDMETHODIMP Ball::get_AngMomY(float *pVal)
{
   *pVal = m_hitBall.m_angularmomentum.y;
   return S_OK;
}

STDMETHODIMP Ball::put_AngMomY(float newVal)
{
   m_hitBall.m_angularmomentum.y = newVal;
   return S_OK;
}

STDMETHODIMP Ball::get_AngMomZ(float *pVal)
{
   *pVal = m_hitBall.m_angularmomentum.z;
   return S_OK;
}

STDMETHODIMP Ball::put_AngMomZ(float newVal)
{
   m_hitBall.m_angularmomentum.z = newVal;
   return S_OK;
}

STDMETHODIMP Ball::get_Color(OLE_COLOR *pVal)
{
   *pVal = m_d.m_color;
   return S_OK;
}

STDMETHODIMP Ball::put_Color(OLE_COLOR newVal)
{
   m_d.m_color = newVal;
   return S_OK;
}

STDMETHODIMP Ball::get_Image(BSTR *pVal)
{
   WCHAR wz[MAXTOKEN];
   MultiByteToWideCharNull(CP_ACP, 0, m_d.m_szImage.c_str(), -1, wz, MAXTOKEN);
   *pVal = SysAllocString(wz);
   return S_OK;
}

STDMETHODIMP Ball::put_Image(BSTR newVal)
{
   char buf[MAXTOKEN];
   WideCharToMultiByteNull(CP_ACP, 0, newVal, -1, buf, MAXTOKEN, nullptr, nullptr);
   m_d.m_szImage = buf;
   if (g_pplayer)
      m_pinballEnv = g_pplayer->m_ptable->GetImage(m_d.m_szImage);
   return S_OK;
}

STDMETHODIMP Ball::get_FrontDecal(BSTR *pVal)
{
   WCHAR wz[MAXTOKEN];
   MultiByteToWideCharNull(CP_ACP, 0, m_d.m_imageDecal.c_str(), -1, wz, MAXTOKEN);
   *pVal = SysAllocString(wz);
   return S_OK;
}

STDMETHODIMP Ball::put_FrontDecal(BSTR newVal)
{
   char szImage[MAXTOKEN];
   WideCharToMultiByteNull(CP_ACP, 0, newVal, -1, szImage, MAXTOKEN, nullptr, nullptr);
   m_d.m_imageDecal = szImage;
   if (g_pplayer)
   {
      Texture * const tex = g_pplayer->m_ptable->GetImage(szImage);
      if (tex && tex->IsHDR())
      {
          ShowError("Cannot use a HDR image (.exr/.hdr) here");
          return E_FAIL;
      }
      m_pinballDecal = tex;
   }
   return S_OK;
}

STDMETHODIMP Ball::get_DecalMode(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_d.m_decalMode);
   return S_OK;
}

STDMETHODIMP Ball::put_DecalMode(VARIANT_BOOL newVal)
{
   m_d.m_decalMode = VBTOb(newVal);
   return S_OK;
}

STDMETHODIMP Ball::get_Mass(float *pVal)
{
   *pVal = m_hitBall.m_d.m_mass;
   return S_OK;
}

STDMETHODIMP Ball::put_Mass(float newVal)
{
   m_hitBall.m_d.m_mass = newVal;
   return S_OK;
}

STDMETHODIMP Ball::get_ID(int *pVal)
{
   *pVal = m_id;
   return S_OK;
}

STDMETHODIMP Ball::put_ID(int newVal)
{
   // ID is not mutable (to guarantee a unique id)
   if (m_id != newVal)
      return E_FAIL;
   return S_OK;
}

STDMETHODIMP Ball::get_Radius(float *pVal)
{
   *pVal = m_hitBall.m_d.m_radius;
   return S_OK;
}

STDMETHODIMP Ball::put_Radius(float newVal)
{
   m_hitBall.m_d.m_radius = newVal;
   return S_OK;
}

STDMETHODIMP Ball::get_BulbIntensityScale(float *pVal)
{
   *pVal = m_d.m_bulb_intensity_scale;
   return S_OK;
}

STDMETHODIMP Ball::put_BulbIntensityScale(float newVal)
{
   m_d.m_bulb_intensity_scale = newVal;
   return S_OK;
}

STDMETHODIMP Ball::get_ReflectionEnabled(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_d.m_reflectionEnabled);
   return S_OK;
}

STDMETHODIMP Ball::put_ReflectionEnabled(VARIANT_BOOL newVal)
{
   m_d.m_reflectionEnabled = VBTOb(newVal);
   return S_OK;
}

STDMETHODIMP Ball::get_PlayfieldReflectionScale(float *pVal)
{
   *pVal = m_d.m_playfieldReflectionStrength;
   return S_OK;
}

STDMETHODIMP Ball::put_PlayfieldReflectionScale(float newVal)
{
   m_d.m_playfieldReflectionStrength = newVal;
   return S_OK;
}

STDMETHODIMP Ball::get_ForceReflection(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_d.m_forceReflection);
   return S_OK;
}

STDMETHODIMP Ball::put_ForceReflection(VARIANT_BOOL newVal)
{
   m_d.m_forceReflection = VBTOb(newVal);
   return S_OK;
}

STDMETHODIMP Ball::get_Visible(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_d.m_visible);
   return S_OK;
}

STDMETHODIMP Ball::put_Visible(VARIANT_BOOL newVal)
{
   m_d.m_visible = VBTOb(newVal);
   return S_OK;
}

STDMETHODIMP Ball::DestroyBall(int *pVal)
{
   int cnt = 0;
   if (g_pplayer)
   {
      ++cnt;
      HitBall *const b = g_pplayer->m_pactiveball;
      g_pplayer->m_pactiveball = nullptr;
      g_pplayer->DestroyBall(b);
   }
   if (pVal) *pVal = cnt;
   return S_OK;
}

#pragma endregion
