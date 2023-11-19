#include "stdafx.h"
#include "vpinball.h"
#include "meshes/ballMesh.h"

BallEx::BallEx()
{
   m_pball = nullptr;
}

BallEx::~BallEx()
{
   assert(m_rd == nullptr);
}

void BallEx::GetDebugCommands(vector<int> & pvids, vector<int> & pvcommandid)
{
   pvids.push_back(IDS_MAKEACTIVEBALL);
   pvcommandid.push_back(0);
}

void BallEx::RunDebugCommand(int id)
{
   switch (id)
   {
   case 0:
      g_pplayer->m_pactiveballDebug = m_pball;
      break;
   }
}


#pragma region Rendering

void BallEx::RenderSetup(RenderDevice *device)
{
   assert(m_rd == nullptr);
   m_rd = device;
   CalcBallAspectRatio();
}

void BallEx::RenderRelease()
{
   assert(m_rd != nullptr);
   m_rd = nullptr;
}

void BallEx::UpdateAnimation(const float diff_time_msec)
{
   assert(m_rd != nullptr);
   // Animation is updated by physics engine through a MoverObject. No additional visual animation here
}

inline float map_bulblight_to_emission(const Light* const l) // magic mapping of bulblight parameters to "real" lightsource emission
{
   return l->m_currentIntensity * clamp(powf(l->m_d.m_falloff*0.6f, l->m_d.m_falloff_power*0.6f), 0.f, 23000.f); //!! 0.6f,0.6f = magic, also clamp 23000
}

void search_for_nearest(const Ball * const pball, const vector<Light*> &lights, Light* light_nearest[MAX_BALL_LIGHT_SOURCES])
{
   for (unsigned int l = 0; l < MAX_BALL_LIGHT_SOURCES; ++l)
   {
      float min_dist = FLT_MAX;
      light_nearest[l] = nullptr;
      for (size_t i = 0; i < lights.size(); ++i)
      {
         bool already_processed = false;
         for (unsigned int i2 = 0; i2 < MAX_BALL_LIGHT_SOURCES - 1; ++i2)
            if (l > i2 && light_nearest[i2] == lights[i]) {
               already_processed = true;
               break;
            }
         if (already_processed)
            continue;

         const float dist = Vertex3Ds(lights[i]->m_d.m_vCenter.x - pball->m_d.m_pos.x, lights[i]->m_d.m_vCenter.y - pball->m_d.m_pos.y, lights[i]->m_d.m_meshRadius + lights[i]->m_surfaceHeight - pball->m_d.m_pos.z).LengthSquared(); //!! z pos
         //const float contribution = map_bulblight_to_emission(lights[i]) / dist; // could also weight in light color if necessary //!! JF didn't like that, seems like only distance is a measure better suited for the human eye
         if (dist < min_dist)
         {
            min_dist = dist;
            light_nearest[l] = lights[i];
         }
      }
   }
}

// FIXME remove completely and use exact projection (conic intersection leading to ellipsoid based on real MVP matrix)
void BallEx::CalcBallAspectRatio()
{
   const int ballStretchMode = g_pplayer->m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "BallStretchMode"s, 0);
   if (ballStretchMode == 0)
   {
      m_BallStretch = Vertex2D(1.0f, 1.0f);
      return;
   }

   const float ballAspecRatioOffsetX = g_pplayer->m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "BallCorrectionX"s, 0.f);
   const float ballAspecRatioOffsetY = g_pplayer->m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "BallCorrectionY"s, 0.f);

   const ViewSetup &viewSetup = g_pplayer->m_ptable->mViewSetups[g_pplayer->m_ptable->m_BG_current_set];
   const float scalebackX = (viewSetup.mSceneScaleX != 0.0f) ? ((viewSetup.mSceneScaleX + viewSetup.mSceneScaleY) * 0.5f) / viewSetup.mSceneScaleX : 1.0f;
   const float scalebackY = (viewSetup.mSceneScaleY != 0.0f) ? ((viewSetup.mSceneScaleX + viewSetup.mSceneScaleY) * 0.5f) / viewSetup.mSceneScaleY : 1.0f;

   const float rotation = viewSetup.GetRotation(g_pplayer->m_wnd_width, g_pplayer->m_wnd_height); // FIXME We use the window size since this is called before creating the device (which is a bit wrong)
      // m_pin3d.m_pd3dPrimaryDevice->m_width, m_pin3d.m_pd3dPrimaryDevice->m_height);
   const float c = sinf(ANGTORAD(fmodf(rotation + 90.0f, 180.0f)));
   const float s = sinf(ANGTORAD(fmodf(rotation, 180.0f)));
   m_BallStretch = Vertex2D(scalebackX * c + scalebackY * s, scalebackY * c + scalebackX * s);
   m_antiStretchBall = false;

   // Cabinet anti stretch mode is fairly weird: it will only apply when using rotated fullscreen, and apply a stretching based on monitor aspect ratio
   if (ballStretchMode == 2 && (g_pplayer->m_fullScreen || (g_pplayer->m_wnd_width == g_pplayer->m_screenwidth && g_pplayer->m_wnd_height == g_pplayer->m_screenheight)))
   {
      m_antiStretchBall = true;
      double xMonitor, yMonitor;
      const double aspect = (double)g_pplayer->m_screenwidth / (double)g_pplayer->m_screenheight;
      double factor = aspect * 3.0;
      if (factor > 4.0)
      {
         factor = aspect * 9.0;
         if ((int)(factor + 0.5) == 16)
         {
            //16:9
            xMonitor = (16.0 + ballAspecRatioOffsetX) / 4.0;
            yMonitor = (9.0 + ballAspecRatioOffsetY) / 3.0;
         }
         else if ((int)(factor + 0.5) == 21)
         {
            //21:9
            xMonitor = (21.0 + ballAspecRatioOffsetX) / 4.0;
            yMonitor = (9.0 + ballAspecRatioOffsetY) / 3.0;
         }
         else
         {
            factor = aspect * 10.0;
            if ((int)(factor + 0.5) == 16)
            {
               //16:10
               xMonitor = (16.0 + ballAspecRatioOffsetX) / 4.0;
               yMonitor = (10.0 + ballAspecRatioOffsetY) / 3.0;
            }
            else
            {
               //21:10
               xMonitor = (factor + ballAspecRatioOffsetX) / 4.0;
               yMonitor = (10.0 + ballAspecRatioOffsetY) / 3.0;
            }
         }
      }
      else
      {
         //4:3
         xMonitor = (factor + ballAspecRatioOffsetX) / 4.0;
         yMonitor = (3.0 + ballAspecRatioOffsetY) / 3.0;
      }
      const double scalebackMonitorX = (xMonitor + yMonitor) * 0.5 / xMonitor;
      const double scalebackMonitorY = (xMonitor + yMonitor) * 0.5 / yMonitor;
      m_BallStretch.x *= (float)(scalebackMonitorX * c + scalebackMonitorY * s);
      m_BallStretch.y *= (float)(scalebackMonitorY * c + scalebackMonitorX * s);
   }
}

// FIXME remove completely and use exact projection (conic intersection leading to ellipsoid based on real MVP matrix)
void BallEx::GetBallAspectRatio(const Ball * const pball, Vertex2D &stretch, const float zHeight)
{
   // always use lowest detail level for fastest update
   Vertex3Ds rgvIn[(basicBallLoNumVertices+1) / 2];
   Vertex2D rgvOut[(basicBallLoNumVertices+1) / 2];

   //     rgvIn[0].x = pball->m_pos.x;                    rgvIn[0].y = pball->m_pos.y+pball->m_radius;    rgvIn[0].z = zHeight;
   //     rgvIn[1].x = pball->m_pos.x + pball->m_radius;  rgvIn[1].y = pball->m_pos.y;                    rgvIn[1].z = zHeight;
   //     rgvIn[2].x = pball->m_pos.x;                    rgvIn[2].y = pball->m_pos.y - pball->m_radius;  rgvIn[2].z = zHeight;
   //     rgvIn[3].x = pball->m_pos.x - pball->m_radius;  rgvIn[3].y = pball->m_pos.y;                    rgvIn[3].z = zHeight;
   //     rgvIn[4].x = pball->m_pos.x;                    rgvIn[4].y = pball->m_pos.y;                    rgvIn[4].z = zHeight + pball->m_radius;
   //     rgvIn[5].x = pball->m_pos.x;                    rgvIn[5].y = pball->m_pos.y;                    rgvIn[5].z = zHeight - pball->m_radius;
   
   for (unsigned int i = 0, t = 0; i < basicBallLoNumVertices; i += 2, t++)
   {
      rgvIn[t].x = basicBallLo[i].x*pball->m_d.m_radius + pball->m_d.m_pos.x;
      rgvIn[t].y = basicBallLo[i].y*pball->m_d.m_radius + pball->m_d.m_pos.y;
      rgvIn[t].z = basicBallLo[i].z*pball->m_d.m_radius + zHeight;
   }
   
   RECT viewport { 0, 0, (LONG)g_pplayer->m_pin3d.m_viewPort.Width, (LONG)g_pplayer->m_pin3d.m_viewPort.Height };
   g_pplayer->m_pin3d.TransformVertices(rgvIn, nullptr, basicBallLoNumVertices / 2, rgvOut);
   
   float maxX = -FLT_MAX;
   float minX = FLT_MAX;
   float maxY = -FLT_MAX;
   float minY = FLT_MAX;
   for (unsigned int i = 0; i < basicBallLoNumVertices / 2; i++)
   {
      if (maxX < rgvOut[i].x) maxX = rgvOut[i].x;
      if (minX > rgvOut[i].x) minX = rgvOut[i].x;
      if (maxY < rgvOut[i].y) maxY = rgvOut[i].y;
      if (minY > rgvOut[i].y) minY = rgvOut[i].y;
   }

   const float midX = maxX - minX;
   const float midY = maxY - minY;
   stretch.y = midY/midX;
   stretch.x = 1.0f; // midX/midY;
}

void BallEx::Render(const unsigned int renderMask)
{
   assert(m_rd != nullptr);
   const bool isStaticOnly = renderMask & Player::STATIC_ONLY;
   const bool isDynamicOnly = renderMask & Player::DYNAMIC_ONLY;
   const bool isReflectionPass = renderMask & Player::REFLECTION_PASS;
   TRACE_FUNCTION();
   
   if (!m_pball->m_visible
    || isStaticOnly
    || (isReflectionPass && !m_pball->m_reflectionEnabled))
      return;

   // Adapt z position of ball
   float zheight = !m_pball->m_d.m_lockedInKicker ? m_pball->m_d.m_pos.z : (m_pball->m_d.m_pos.z - m_pball->m_d.m_radius);

   // Don't draw reflection if the ball is not on the playfield (e.g. on a ramp/kicker), except if explicitely asked too
   if (isReflectionPass && !m_pball->m_forceReflection 
      && ((zheight > m_pball->m_d.m_radius + 3.0f) || m_pball->m_d.m_lockedInKicker || (m_pball->m_d.m_pos.z < m_pball->m_d.m_radius - 0.1f)))
      return;

   m_rd->ResetRenderState();
   
   // Set the render state to something that will always display for debug mode
   m_rd->SetRenderState(RenderState::ZENABLE, g_pplayer->m_debugBalls ? RenderState::RS_FALSE : RenderState::RS_TRUE);

   m_rd->m_ballShader->SetVector(SHADER_invTableRes_reflection, 
      1.0f / (g_pplayer->m_ptable->m_right - g_pplayer->m_ptable->m_left),
      1.0f / (g_pplayer->m_ptable->m_bottom - g_pplayer->m_ptable->m_top), 
      g_pplayer->m_ptable->m_ballPlayfieldReflectionStrength * m_pball->m_playfieldReflectionStrength, 0.f);

   // collect the x nearest lights that can reflect on balls
   Light* light_nearest[MAX_BALL_LIGHT_SOURCES];
   search_for_nearest(m_pball, g_pplayer->m_ballReflectedLights, light_nearest);
   #ifdef ENABLE_SDL
   float lightPos[MAX_LIGHT_SOURCES + MAX_BALL_LIGHT_SOURCES][4] = { 0.0f, 0.0f, 0.0f, 0.0f };
   float lightEmission[MAX_LIGHT_SOURCES + MAX_BALL_LIGHT_SOURCES][4] = { 0.0f, 0.0f, 0.0f, 0.0f };
   float *pLightPos = (float *)lightPos, *pLightEm = (float *)lightEmission;
   const int lightStride = 4, lightOfs = 0;
   #else
   struct CLight
   {
      float vPos[3];
      float vEmission[3];
   };
   CLight l[MAX_LIGHT_SOURCES + MAX_BALL_LIGHT_SOURCES];
   float *pLightPos = (float *)l, *pLightEm = (float *)l;
   const int lightStride = 6, lightOfs = 3;
   #endif
   vec4 emission = convertColor(g_pplayer->m_ptable->m_Light[0].emission);
   emission.x *= g_pplayer->m_ptable->m_lightEmissionScale * g_pplayer->m_globalEmissionScale;
   emission.y *= g_pplayer->m_ptable->m_lightEmissionScale * g_pplayer->m_globalEmissionScale;
   emission.z *= g_pplayer->m_ptable->m_lightEmissionScale * g_pplayer->m_globalEmissionScale;
   for (unsigned int i2 = 0; i2 < MAX_LIGHT_SOURCES; ++i2)
   {
      const int pPos = i2 * lightStride, pEm = pPos + lightOfs;
      memcpy(&pLightPos[pPos], &g_pplayer->m_ptable->m_Light[i2].pos, sizeof(float) * 3);
      memcpy(&pLightEm[pEm], &emission, sizeof(float) * 3);
   }
   for (unsigned int light_i = 0; light_i < MAX_BALL_LIGHT_SOURCES; ++light_i)
   {
      const int pPos = (light_i + MAX_LIGHT_SOURCES) * lightStride, pEm = pPos + lightOfs;
      if (light_nearest[light_i] != nullptr)
      {
         pLightPos[pPos + 0] = light_nearest[light_i]->m_d.m_vCenter.x;
         pLightPos[pPos + 1] = light_nearest[light_i]->m_d.m_vCenter.y;
         pLightPos[pPos + 2] = light_nearest[light_i]->GetCurrentHeight();
         const float c = map_bulblight_to_emission(light_nearest[light_i]) * m_pball->m_bulb_intensity_scale;
         const vec4 color = convertColor(light_nearest[light_i]->m_d.m_color);
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
   #ifdef ENABLE_SDL
   m_rd->m_ballShader->SetFloat4v(SHADER_ballLightPos, (vec4 *)lightPos, MAX_LIGHT_SOURCES + MAX_BALL_LIGHT_SOURCES);
   m_rd->m_ballShader->SetFloat4v(SHADER_ballLightEmission, (vec4 *)lightEmission, MAX_LIGHT_SOURCES + MAX_BALL_LIGHT_SOURCES);
   #else
   m_rd->m_ballShader->SetFloat4v(SHADER_ballPackedLights, (vec4 *)l, sizeof(CLight) * (MAX_LIGHT_SOURCES + MAX_BALL_LIGHT_SOURCES) / (4 * sizeof(float)));
   #endif

   // now for a weird hack: make material more rough, depending on how near the nearest lightsource is, to 'emulate' the area of the bulbs (as VP only features point lights so far)
   float Roughness = 0.8f;
   if (light_nearest[0] != nullptr)
   {
       const float dist = Vertex3Ds(light_nearest[0]->m_d.m_vCenter.x - m_pball->m_d.m_pos.x, light_nearest[0]->m_d.m_vCenter.y - m_pball->m_d.m_pos.y, light_nearest[0]->m_d.m_meshRadius + light_nearest[0]->m_surfaceHeight - m_pball->m_d.m_pos.z).Length(); //!! z pos
       Roughness = min(max(dist*0.006f, 0.4f), Roughness);
   }
   const vec4 rwem(exp2f(10.0f * Roughness + 1.0f), 0.f, 1.f, 0.05f);
   m_rd->m_ballShader->SetVector(SHADER_Roughness_WrapL_Edge_Thickness, &rwem);

   // ************************* draw the ball itself ****************************
   Vertex2D stretch;
   if (m_antiStretchBall && g_pplayer->m_ptable->mViewSetups[g_pplayer->m_ptable->m_BG_current_set].GetRotation(m_rd->m_width, m_rd->m_height) != 0.0f)
      GetBallAspectRatio(m_pball, stretch, zheight);
   else
      stretch = m_BallStretch;

   const vec4 diffuse = convertColor(m_pball->m_color, 1.0f);
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

   Matrix3D scale, trans, m3D_full;
   Matrix3D rot(m_pball->m_orientation.m_d[0][0], m_pball->m_orientation.m_d[1][0], m_pball->m_orientation.m_d[2][0], 0.0f,
      m_pball->m_orientation.m_d[0][1], m_pball->m_orientation.m_d[1][1], m_pball->m_orientation.m_d[2][1], 0.0f,
      m_pball->m_orientation.m_d[0][2], m_pball->m_orientation.m_d[1][2], m_pball->m_orientation.m_d[2][2], 0.0f,
      0.f, 0.f, 0.f, 1.f);
   scale.SetScaling(m_pball->m_d.m_radius * stretch.x, m_pball->m_d.m_radius * stretch.y, m_pball->m_d.m_radius);
   trans.SetTranslation(m_pball->m_d.m_pos.x, m_pball->m_d.m_pos.y, zheight);
   m3D_full = rot * scale * trans;
   m_rd->m_ballShader->SetMatrix(SHADER_orientation, &m3D_full);

   m_rd->m_ballShader->SetVector(SHADER_w_h_disableLighting, 
      1.5f / m_rd->GetPreviousBackBufferTexture()->GetWidth(), // UV Offset for sampling reflections
      1.5f / m_rd->GetPreviousBackBufferTexture()->GetHeight(),
      g_pplayer->m_disableLightingForBalls ? 1.f : 0.f, 0.f);

   m_rd->SetRenderState(RenderState::ZWRITEENABLE, RenderState::RS_TRUE);
   bool sphericalMapping;
   if (!m_pball->m_pinballEnv)
   {
      sphericalMapping = false; // Environment texture is an equirectangular map
      m_rd->m_ballShader->SetTexture(SHADER_tex_ball_color, &g_pplayer->m_pin3d.m_pinballEnvTexture);
   }
   else
   {
      sphericalMapping = m_pball->m_pinballEnvSphericalMapping;
      m_rd->m_ballShader->SetTexture(SHADER_tex_ball_color, m_pball->m_pinballEnv);
   }
   if (m_pball->m_pinballDecal)
      m_rd->m_ballShader->SetTexture(SHADER_tex_ball_decal, m_pball->m_pinballDecal);
   else
      m_rd->m_ballShader->SetTextureNull(SHADER_tex_ball_decal);
   m_rd->m_ballShader->SetTechnique(sphericalMapping ? m_pball->m_decalMode ? SHADER_TECHNIQUE_RenderBall_SphericalMap_DecalMode : SHADER_TECHNIQUE_RenderBall_SphericalMap
                                                     : m_pball->m_decalMode ? SHADER_TECHNIQUE_RenderBall_DecalMode : SHADER_TECHNIQUE_RenderBall);
   Vertex3Ds pos(m_pball->m_d.m_pos.x, m_pball->m_d.m_pos.y, zheight);
   m_rd->DrawMesh(m_rd->m_ballShader, false, pos, 0.f, g_pplayer->m_ballMeshBuffer, RenderDevice::TRIANGLELIST, 0, g_pplayer->m_ballMeshBuffer->m_ib->m_count);

   // draw debug points for visualizing ball rotation (this uses point rendering which is a deprecated feature, not available in OpenGL ES)
   #if defined(DEBUG_BALL_SPIN) && !defined(__OPENGLES__)
   if (g_pplayer->m_liveUI->IsShowingFPSDetails())
   {
      float pointSize = 5.f * m_rd->GetCurrentRenderTarget()->GetWidth() / 1920.0f;
      // this is buggy as we set the point size directly while the render comma,nd is used later on, but this is the only place where point rendering is used so it's ok for now
      #if defined(ENABLE_SDL)
      glPointSize(pointSize);
      #else
      CHECKD3D(m_rd->GetCoreDevice()->SetRenderState(D3DRS_POINTSIZE, float_as_uint(pointSize)));
      #endif
      m_rd->ResetRenderState();
      m_rd->m_ballShader->SetTechnique(SHADER_TECHNIQUE_RenderBall_Debug);
      m_rd->DrawMesh(m_rd->m_ballShader, false, pos, 0.f, g_pplayer->m_ballDebugPoints, RenderDevice::POINTLIST, 0, g_pplayer->m_ballDebugPoints->m_vb->m_count);
   }
   #endif

   // ball trails (except in reflection passes)
   if (g_pplayer->m_trailForBalls && g_pplayer->m_ballTrailStrength > 0.f && !isReflectionPass)
   {
      Vertex3D_NoTex2 vertices[MAX_BALL_TRAIL_POS * 2];
      unsigned int nVertices = 0;
      for (int i2 = 0; i2 < MAX_BALL_TRAIL_POS - 1; ++i2)
      {
         int i3 = m_pball->m_ringcounter_oldpos / (10000 / PHYSICS_STEPTIME) - 1 - i2;
         if (i3 < 0)
            i3 += MAX_BALL_TRAIL_POS;
         int io = i3 - 1;
         if (io < 0)
            io += MAX_BALL_TRAIL_POS;
         if ((m_pball->m_oldpos[i3].x == FLT_MAX) && (m_pball->m_oldpos[io].x == FLT_MAX))
            continue; // No position data => discard

         Vertex3Ds vec(m_pball->m_oldpos[io].x - m_pball->m_oldpos[i3].x, m_pball->m_oldpos[io].y - m_pball->m_oldpos[i3].y, m_pball->m_oldpos[io].z - m_pball->m_oldpos[i3].z);
         const float ls = vec.LengthSquared();
         if (ls <= 1e-3f)
            continue; // Too small => discard

         const float length = sqrtf(ls);
         const float bc = g_pplayer->m_ballTrailStrength * powf(1.f - 1.f / max(length, 1.0f), 64.0f); //!! 64=magic alpha falloff
         const float r = min(m_pball->m_d.m_radius*0.9f, 2.0f*m_pball->m_d.m_radius / powf((float)(i2 + 2), 0.6f)); //!! consts are for magic radius falloff
         if (bc <= 0.f && r <= 1e-3f)
            continue; // Fully faded out or radius too small => discard

         vec *= 1.0f / sqrtf(ls);
         const Vertex3Ds up(0.f, 0.f, 1.f); // TODO Should be camera axis instead of fixed vertical
         const Vertex3Ds n = CrossProduct(vec, up) * r;

         Vertex3D_NoTex2 quadVertices[4];
         quadVertices[0].x = m_pball->m_oldpos[i3].x - n.x;
         quadVertices[0].y = m_pball->m_oldpos[i3].y - n.y;
         quadVertices[0].z = m_pball->m_oldpos[i3].z - n.z;
         quadVertices[1].x = m_pball->m_oldpos[i3].x + n.x;
         quadVertices[1].y = m_pball->m_oldpos[i3].y + n.y;
         quadVertices[1].z = m_pball->m_oldpos[i3].z + n.z;
         quadVertices[2].x = m_pball->m_oldpos[io].x + n.x;
         quadVertices[2].y = m_pball->m_oldpos[io].y + n.y;
         quadVertices[2].z = m_pball->m_oldpos[io].z + n.z;
         quadVertices[3].x = m_pball->m_oldpos[io].x - n.x;
         quadVertices[3].y = m_pball->m_oldpos[io].y - n.y;
         quadVertices[3].z = m_pball->m_oldpos[io].z - n.z;

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
      if (nVertices > 0 && g_pplayer->m_ballTrailMeshBufferPos + nVertices <= g_pplayer->m_ballTrailMeshBuffer->m_vb->m_count)
      {
         Vertex3D_NoTex2 *bufvb;
         g_pplayer->m_ballTrailMeshBuffer->m_vb->lock(
            g_pplayer->m_ballTrailMeshBufferPos * sizeof(Vertex3D_NoTex2), nVertices * sizeof(Vertex3D_NoTex2), (void **)&bufvb, VertexBuffer::DISCARDCONTENTS);
         memcpy(bufvb, vertices, nVertices * sizeof(Vertex3D_NoTex2));
         g_pplayer->m_ballTrailMeshBuffer->m_vb->unlock();
         m_rd->ResetRenderState();
         m_rd->SetRenderState(RenderState::CULLMODE, RenderState::CULL_NONE);
         m_rd->SetRenderState(RenderState::ZFUNC, RenderState::Z_ALWAYS);
         m_rd->SetRenderState(RenderState::ZWRITEENABLE, RenderState::RS_FALSE);
         m_rd->SetRenderState(RenderState::ALPHABLENDENABLE, RenderState::RS_TRUE);
         m_rd->SetRenderState(RenderState::SRCBLEND, RenderState::SRC_ALPHA);
         m_rd->SetRenderState(RenderState::DESTBLEND, RenderState::INVSRC_ALPHA);
         m_rd->SetRenderState(RenderState::BLENDOP, RenderState::BLENDOP_ADD);
         m_rd->m_ballShader->SetTechnique(SHADER_TECHNIQUE_RenderBallTrail);
         m_rd->DrawMesh(m_rd->m_ballShader, true, pos, 0.f, g_pplayer->m_ballTrailMeshBuffer, RenderDevice::TRIANGLESTRIP, g_pplayer->m_ballTrailMeshBufferPos, nVertices);
         g_pplayer->m_ballTrailMeshBufferPos += nVertices;
      }
   }
}

#pragma endregion


#pragma region ScriptProxy

#define CHECKSTALEBALL if (!m_pball) return E_POINTER;

STDMETHODIMP BallEx::get_X(float *pVal)
{
   CHECKSTALEBALL

   *pVal = m_pball->m_d.m_pos.x;

   return S_OK;
}

STDMETHODIMP BallEx::put_X(float newVal)
{
   CHECKSTALEBALL

   m_pball->m_d.m_pos.x = newVal;

   return S_OK;
}

STDMETHODIMP BallEx::get_Y(float *pVal)
{
   CHECKSTALEBALL

   *pVal = m_pball->m_d.m_pos.y;

   return S_OK;
}

STDMETHODIMP BallEx::put_Y(float newVal)
{
   CHECKSTALEBALL

   m_pball->m_d.m_pos.y = newVal;

   return S_OK;
}

STDMETHODIMP BallEx::get_VelX(float *pVal)
{
   CHECKSTALEBALL

   *pVal = m_pball->m_d.m_vel.x;

   return S_OK;
}

STDMETHODIMP BallEx::put_VelX(float newVal)
{
   CHECKSTALEBALL

   m_pball->m_d.m_vel.x = newVal;

   return S_OK;
}

STDMETHODIMP BallEx::get_VelY(float *pVal)
{
   CHECKSTALEBALL

   *pVal = m_pball->m_d.m_vel.y;

   return S_OK;
}

STDMETHODIMP BallEx::put_VelY(float newVal)
{
   CHECKSTALEBALL

   m_pball->m_d.m_vel.y = newVal;

   return S_OK;
}

STDMETHODIMP BallEx::get_Z(float *pVal)
{
   CHECKSTALEBALL

   *pVal = m_pball->m_d.m_pos.z;

   return S_OK;
}

STDMETHODIMP BallEx::put_Z(float newVal)
{
   CHECKSTALEBALL

   m_pball->m_d.m_pos.z = newVal;

   return S_OK;
}

STDMETHODIMP BallEx::get_VelZ(float *pVal)
{
   CHECKSTALEBALL

   *pVal = m_pball->m_d.m_vel.z;

   return S_OK;
}

STDMETHODIMP BallEx::put_VelZ(float newVal)
{
   CHECKSTALEBALL

   m_pball->m_d.m_vel.z = newVal;

   return S_OK;
}

STDMETHODIMP BallEx::get_AngVelX(float *pVal)
{
   CHECKSTALEBALL

   *pVal = m_pball->m_angularmomentum.x / m_pball->Inertia();

   return S_OK;
}

STDMETHODIMP BallEx::get_AngVelY(float *pVal)
{
   CHECKSTALEBALL

   *pVal = m_pball->m_angularmomentum.y / m_pball->Inertia();

   return S_OK;
}

STDMETHODIMP BallEx::get_AngVelZ(float *pVal)
{
   CHECKSTALEBALL

   *pVal = m_pball->m_angularmomentum.z / m_pball->Inertia();

   return S_OK;
}

STDMETHODIMP BallEx::get_AngMomX(float *pVal)
{
   CHECKSTALEBALL

   *pVal = m_pball->m_angularmomentum.x;

   return S_OK;
}

STDMETHODIMP BallEx::put_AngMomX(float newVal)
{
   CHECKSTALEBALL

   m_pball->m_angularmomentum.x = newVal;

   return S_OK;
}

STDMETHODIMP BallEx::get_AngMomY(float *pVal)
{
   CHECKSTALEBALL

   *pVal = m_pball->m_angularmomentum.y;

   return S_OK;
}

STDMETHODIMP BallEx::put_AngMomY(float newVal)
{
   CHECKSTALEBALL

   m_pball->m_angularmomentum.y = newVal;

   return S_OK;
}

STDMETHODIMP BallEx::get_AngMomZ(float *pVal)
{
   CHECKSTALEBALL

   *pVal = m_pball->m_angularmomentum.z;

   return S_OK;
}

STDMETHODIMP BallEx::put_AngMomZ(float newVal)
{
   CHECKSTALEBALL

   m_pball->m_angularmomentum.z = newVal;

   return S_OK;
}

STDMETHODIMP BallEx::get_Color(OLE_COLOR *pVal)
{
   CHECKSTALEBALL

   *pVal = m_pball->m_color;

   return S_OK;
}

STDMETHODIMP BallEx::put_Color(OLE_COLOR newVal)
{
   CHECKSTALEBALL

   m_pball->m_color = newVal;

   return S_OK;
}

STDMETHODIMP BallEx::get_Image(BSTR *pVal)
{
   WCHAR wz[MAXTOKEN];
   MultiByteToWideCharNull(CP_ACP, 0, m_pball->m_image.c_str(), -1, wz, MAXTOKEN);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP BallEx::put_Image(BSTR newVal)
{
   char buf[MAXTOKEN];
   WideCharToMultiByteNull(CP_ACP, 0, newVal, -1, buf, MAXTOKEN, nullptr, nullptr);
   m_pball->m_image = buf;
   m_pball->m_pinballEnv = g_pplayer->m_ptable->GetImage(m_pball->m_image);
   return S_OK;
}

HRESULT BallEx::get_UserValue(VARIANT *pVal)
{
   VariantClear(pVal);
   return VariantCopy(pVal, &m_uservalue);
}

HRESULT BallEx::put_UserValue(VARIANT *newVal)
{
   VariantInit(&m_uservalue);
   VariantClear(&m_uservalue);
   return VariantCopy(&m_uservalue, newVal);
}

STDMETHODIMP BallEx::get_FrontDecal(BSTR *pVal)
{
   WCHAR wz[MAXTOKEN];
   MultiByteToWideCharNull(CP_ACP, 0, m_pball->m_imageDecal.c_str(), -1, wz, MAXTOKEN);
   *pVal = SysAllocString(wz);

   return S_OK;
}

STDMETHODIMP BallEx::put_FrontDecal(BSTR newVal)
{
   char szImage[MAXTOKEN];
   WideCharToMultiByteNull(CP_ACP, 0, newVal, -1, szImage, MAXTOKEN, nullptr, nullptr);
   Texture * const tex = g_pplayer->m_ptable->GetImage(szImage);
   if (tex && tex->IsHDR())
   {
       ShowError("Cannot use a HDR image (.exr/.hdr) here");
       return E_FAIL;
   }

   m_pball->m_imageDecal = szImage;
   m_pball->m_pinballDecal = tex;

   return S_OK;
}

STDMETHODIMP BallEx::get_DecalMode(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_pball->m_decalMode);

   return S_OK;
}

STDMETHODIMP BallEx::put_DecalMode(VARIANT_BOOL newVal)
{
   m_pball->m_decalMode = VBTOb(newVal);

   return S_OK;
}

STDMETHODIMP BallEx::get_Name(BSTR *pVal)
{
   *pVal = SysAllocString(L"Ball");
   return S_OK;
}

STDMETHODIMP BallEx::put_Name(BSTR newVal)
{
   return S_OK;
}

STDMETHODIMP BallEx::get_Mass(float *pVal)
{
   CHECKSTALEBALL

   *pVal = m_pball->m_d.m_mass;

   return S_OK;
}

STDMETHODIMP BallEx::put_Mass(float newVal)
{
   CHECKSTALEBALL

   m_pball->m_d.m_mass = newVal;

   return S_OK;
}

STDMETHODIMP BallEx::get_ID(int *pVal)
{
   CHECKSTALEBALL

   *pVal = m_pball->m_id;

   return S_OK;
}

STDMETHODIMP BallEx::put_ID(int newVal)
{
   CHECKSTALEBALL

   m_pball->m_id = newVal;

   return S_OK;
}

STDMETHODIMP BallEx::get_Radius(float *pVal)
{
   CHECKSTALEBALL

   *pVal = m_pball->m_d.m_radius;

   return S_OK;
}

STDMETHODIMP BallEx::put_Radius(float newVal)
{
   CHECKSTALEBALL

   m_pball->m_d.m_radius = newVal;

   return S_OK;
}

STDMETHODIMP BallEx::get_BulbIntensityScale(float *pVal)
{
   CHECKSTALEBALL

   *pVal = m_pball->m_bulb_intensity_scale;

   return S_OK;
}

STDMETHODIMP BallEx::put_BulbIntensityScale(float newVal)
{
   CHECKSTALEBALL

   m_pball->m_bulb_intensity_scale = newVal;

   return S_OK;
}

STDMETHODIMP BallEx::get_ReflectionEnabled(VARIANT_BOOL *pVal)
{
   *pVal = FTOVB(m_pball->m_reflectionEnabled);

   return S_OK;
}

STDMETHODIMP BallEx::put_ReflectionEnabled(VARIANT_BOOL newVal)
{
   m_pball->m_reflectionEnabled = VBTOb(newVal);

   return S_OK;
}

STDMETHODIMP BallEx::get_PlayfieldReflectionScale(float *pVal)
{
   CHECKSTALEBALL

   *pVal = m_pball->m_playfieldReflectionStrength;

   return S_OK;
}

STDMETHODIMP BallEx::put_PlayfieldReflectionScale(float newVal)
{
   CHECKSTALEBALL

   m_pball->m_playfieldReflectionStrength = newVal;

   return S_OK;
}

STDMETHODIMP BallEx::get_ForceReflection(VARIANT_BOOL *pVal)
{
    *pVal = FTOVB(m_pball->m_forceReflection);

    return S_OK;
}

STDMETHODIMP BallEx::put_ForceReflection(VARIANT_BOOL newVal)
{
    m_pball->m_forceReflection = VBTOb(newVal);

    return S_OK;
}

STDMETHODIMP BallEx::get_Visible(VARIANT_BOOL *pVal)
{
    *pVal = FTOVB(m_pball->m_visible);

    return S_OK;
}

STDMETHODIMP BallEx::put_Visible(VARIANT_BOOL newVal)
{
    m_pball->m_visible = VBTOb(newVal);

    return S_OK;
}

STDMETHODIMP BallEx::DestroyBall(int *pVal)
{
   int cnt = 0;
   if (g_pplayer)
   {
      ++cnt;
      Ball * const b = g_pplayer->m_pactiveball;
      g_pplayer->m_pactiveball = nullptr;
      g_pplayer->DestroyBall(b); // clear ActiveBall
   }

   if (pVal) *pVal = cnt;

   return S_OK;
}

#pragma endregion