#include "core/stdafx.h"
#include "PhysicsEngine.h"

PhysicsEngine::PhysicsEngine(PinTable *const table) : m_nudgeFilterX("x"), m_nudgeFilterY("y")
{
   m_physicsMaxLoops = table->m_PhysicsMaxLoops == 0xFFFFFFFFu ? 0 : table->m_PhysicsMaxLoops * (10000 / PHYSICS_STEPTIME) /*2*/;
   m_contacts.reserve(8);

   m_plumb = Vertex2D(0.f, 0.f);
   m_plumbVel = Vertex2D(0.f, 0.f);
   m_plumbTiltThreshold = (float)table->m_settings.LoadValueWithDefault(Settings::Player, "TiltSensitivity"s, 400) * (float)(1.0 / 1000.0);
   m_enablePlumbTilt = table->m_settings.LoadValueWithDefault(Settings::Player, "TiltSensCB"s, false);

   m_enableNudgeFilter = table->m_settings.LoadValueWithDefault(Settings::Player, "EnableNudgeFilter"s, false);

   // Initialize legacy nudging.
   m_legacyNudge = table->m_settings.LoadValueWithDefault(Settings::Player, "EnableLegacyNudge"s, false);
   m_legacyNudgeStrength = table->m_settings.LoadValueWithDefault(Settings::Player, "LegacyNudgeStrength"s, 1.f);
   m_legacyNudgeBack = Vertex2D(0.f, 0.f);

   // Initialize new nudging.
   m_tableVel.SetZero();
   m_tableDisplacement.SetZero();
   m_tableVelOld.SetZero();
   m_tableVelDelta.SetZero();

   // Table movement (displacement u) is modeled as a mass-spring-damper system
   //   u'' = -k u - c u'
   // with a spring constant k and a damping coefficient c.
   // See http://en.wikipedia.org/wiki/Damping#Linear_damping

   const float nudgeTime = table->m_nudgeTime; // T
   constexpr float dampingRatio = 0.5f; // zeta

   // time for one half period (one swing and swing back):
   //   T = pi / omega_d,
   // where
   //   omega_d = omega_0 * sqrt(1 - zeta^2)       (damped frequency)
   //   omega_0 = sqrt(k)                          (undamped frequency)
   // Solving for the spring constant k, we get
   m_nudgeSpring = (float)(M_PI * M_PI) / (nudgeTime * nudgeTime * (1.0f - dampingRatio * dampingRatio));

   // The formula for the damping ratio is
   //   zeta = c / (2 sqrt(k)).
   // Solving for the damping coefficient c, we get
   m_nudgeDamping = dampingRatio * 2.0f * sqrtf(m_nudgeSpring);

   for (IEditable *const pe : table->m_vedit)
   {
      Hitable * const ph = pe->GetIHitable();
      if (ph)
      {
#ifdef DEBUGPHYSICS
         if(pe->GetScriptable())
         {
            CComBSTR bstr;
            pe->GetScriptable()->get_Name(&bstr);
            char * bstr2 = MakeChar(bstr);
            CHAR wzDst[256];
            sprintf_s(wzDst, sizeof(wzDst), "Initializing Object-Physics %s...", bstr2);
            delete [] bstr2;
            g_pplayer->m_pEditorTable->m_progressDialog.SetName(wzDst);
         }
#endif
         const size_t currentsize = m_vho.size();
         ph->GetHitShapes(m_vho);
         const size_t newsize = m_vho.size();
         // Save the objects the trouble of having to set the idispatch pointer themselves
         for (size_t hitloop = currentsize; hitloop < newsize; hitloop++)
            m_vho[hitloop]->m_editable = pe;
      }
   }

   PLOGI << "Initializing octree"; // For profiling

   AddCabinetBoundingHitShapes(table);

   for (HitObject *const pho : m_vho)
   {
      pho->CalcHitBBox(); // maybe needed to update here, as only done lazily for some objects (i.e. balls!)
      m_hitoctree.AddElement(pho);

      if (pho->GetType() == eFlipper)
         m_vFlippers.push_back((HitFlipper*)pho);

      MoverObject * const pmo = pho->GetMoverObject();
      if (pmo && pmo->AddToList()) // Spinner, Gate, Flipper, Plunger (ball is added separately on each create ball)
         m_vmover.push_back(pmo);
   }

   const FRect3D tableBounds = table->GetBoundingBox();
   m_hitoctree.Initialize(FRect(tableBounds.left,tableBounds.right,tableBounds.top,tableBounds.bottom));
#if !defined(NDEBUG) && defined(PRINT_DEBUG_COLLISION_TREE)
   m_hitoctree.DumpTree(0);
#endif

   // initialize hit structure for dynamic objects
   m_hitoctree_dynamic.FillFromVector(m_vho_dynamic);


#ifdef DEBUGPHYSICS
   c_hitcnts = 0;
   c_collisioncnt = 0;
   c_contactcnt = 0;
   #ifdef C_DYNAMIC
   c_staticcnt = 0;
   #endif
   c_embedcnts = 0;
   c_timesearch = 0;

   c_traversed = 0;
   c_tested = 0;
   c_deepTested = 0;
#endif
}

PhysicsEngine::~PhysicsEngine()
{
   for (size_t i = 0; i < m_vho.size(); i++)
      delete m_vho[i];
   m_vho.clear();

   for (size_t i = 0; i < m_vUIHitObjects.size(); i++)
      delete m_vUIHitObjects[i];
   m_vUIHitObjects.clear();
}

void PhysicsEngine::SetGravity(float slopeDeg, float strength)
{
   m_gravity.x = 0;
   m_gravity.y = sinf(ANGTORAD(slopeDeg)) * strength;
   m_gravity.z = -cosf(ANGTORAD(slopeDeg)) * strength;
}

//
// license:GPLv3+
// Ported at: VisualPinball.Engine/VPT/Table/TableHitGenerator.cs
//

void PhysicsEngine::AddCabinetBoundingHitShapes(PinTable *const table)
{
   // simple outer borders:
   m_vho.push_back(new LineSeg(Vertex2D(table->m_right, table->m_top),    Vertex2D(table->m_right, table->m_bottom), 0.f, table->m_glassTopHeight));
   m_vho.push_back(new LineSeg(Vertex2D(table->m_left,  table->m_bottom), Vertex2D(table->m_left,  table->m_top),    0.f, table->m_glassBottomHeight));
   m_vho.push_back(new LineSeg(Vertex2D(table->m_right, table->m_bottom), Vertex2D(table->m_left,  table->m_bottom), 0.f, table->m_glassBottomHeight));
   m_vho.push_back(new LineSeg(Vertex2D(table->m_left,  table->m_top),    Vertex2D(table->m_right, table->m_top),    0.f, table->m_glassTopHeight));

   // glass:
   Vertex3Ds * const rgv3D = new Vertex3Ds[4];
   rgv3D[0] = Vertex3Ds(table->m_left, table->m_top, table->m_glassTopHeight);
   rgv3D[1] = Vertex3Ds(table->m_right, table->m_top, table->m_glassTopHeight);
   rgv3D[2] = Vertex3Ds(table->m_right, table->m_bottom, table->m_glassBottomHeight);
   rgv3D[3] = Vertex3Ds(table->m_left, table->m_bottom, table->m_glassBottomHeight);
   m_vho.push_back(new Hit3DPoly(rgv3D, 4)); //!!

   /*
   // playfield:
   Vertex3Ds * const rgv3D = new Vertex3Ds[4];
   rgv3D[3] = Vertex3Ds(table->m_left, table->m_top, 0.f);
   rgv3D[2] = Vertex3Ds(table->m_right, table->m_top, 0.f);
   rgv3D[1] = Vertex3Ds(table->m_right, table->m_bottom, 0.f);
   rgv3D[0] = Vertex3Ds(table->m_left, table->m_bottom, 0.f);
   Hit3DPoly * const ph3dpoly = new Hit3DPoly(rgv3D, 4); //!!
   ph3dpoly->SetFriction(table->m_overridePhysics ? table->m_fOverrideContactFriction : table->m_friction);
   ph3dpoly->m_elasticity = table->m_overridePhysics ? table->m_fOverrideElasticity : table->m_elasticity;
   ph3dpoly->m_elasticityFalloff = table->m_overridePhysics ? table->m_fOverrideElasticityFalloff : table->m_elasticityFalloff;
   ph3dpoly->m_scatter = ANGTORAD(table->m_overridePhysics ? table->m_fOverrideScatterAngle : table->m_scatter);
   m_vho.push_back(ph3dpoly);
   */

   // playfield:
   m_hitPlayfield = HitPlane(Vertex3Ds(0, 0, 1), 0.f);
   m_hitPlayfield.SetFriction(table->m_overridePhysics ? table->m_fOverrideContactFriction : table->m_friction);
   m_hitPlayfield.m_elasticity = table->m_overridePhysics ? table->m_fOverrideElasticity : table->m_elasticity;
   m_hitPlayfield.m_elasticityFalloff = table->m_overridePhysics ? table->m_fOverrideElasticityFalloff : table->m_elasticityFalloff;
   m_hitPlayfield.m_scatter = ANGTORAD(table->m_overridePhysics ? table->m_fOverrideScatterAngle : table->m_scatter);

   // glass:
   Vertex3Ds glassNormal(0, table->m_bottom - table->m_top, table->m_glassBottomHeight - table->m_glassTopHeight);
   glassNormal.Normalize();
   m_hitTopGlass = HitPlane(Vertex3Ds(0, glassNormal.z, -glassNormal.y), -table->m_glassTopHeight);
   m_hitTopGlass.m_elasticity = 0.2f;
}

//
// end of license:GPLv3+, back to 'old MAME'-like
//

void PhysicsEngine::AddBall(Ball *const ball)
{
   m_vmover.push_back(&ball->m_mover); // balls are always added separately to this list!
   m_vho_dynamic.push_back(ball);
   m_hitoctree_dynamic.FillFromVector(m_vho_dynamic);
}

void PhysicsEngine::RemoveBall(Ball *const ball)
{
   RemoveFromVectorSingle<MoverObject *>(m_vmover, &ball->m_mover);
   RemoveFromVectorSingle<HitObject *>(m_vho_dynamic, ball);
   m_hitoctree_dynamic.FillFromVector(m_vho_dynamic);
}

bool PhysicsEngine::RecordContact(CollisionEvent& newColl)
{
   if (m_recordContacts) // remember all contacts?
   {
      #ifdef USE_EMBREE
      const std::lock_guard<std::mutex> lg(mtx); // multiple threads may end up here and call push_back
      #endif
      m_contacts.push_back(newColl);
      return true;
   }
   return false;
}

void PhysicsEngine::Nudge(float angle, float force)
{
   const float a = ANGTORAD(angle);
   const float sn = sinf(a) * force;
   const float cs = cosf(a) * force;
   if (!m_legacyNudge)
   {
      m_tableVel.x += sn;
      m_tableVel.y += -cs;
   }
   else if (m_legacyNudgeTime == 0)
   {
      m_legacyNudgeBack.x = sn * m_legacyNudgeStrength;
      m_legacyNudgeBack.y = -cs * m_legacyNudgeStrength;
      m_legacyNudgeTime = 100;
   }
}

void PhysicsEngine::UpdateNudge(float dtime)
{
   // Nudge acceleration is computed either from hardware accelerometer(s) or from nudge commands called from script.
  
   if (!m_legacyNudge)
   {
      // Perform keyboard nudge by simulating table movement is modeled as a mass-spring-damper system
      //   u'' = -k u - c u'
      // with a spring constant k and a damping coefficient c
      const Vertex3Ds force = -m_nudgeSpring * m_tableDisplacement - m_nudgeDamping * m_tableVel;
      m_tableVel += (float)PHYS_FACTOR * force;
      m_tableDisplacement += (float)PHYS_FACTOR * m_tableVel;

      m_tableVelDelta = m_tableVel - m_tableVelOld;
      m_tableVelOld = m_tableVel;

      // Perform hardware nudge by getting the accelerometer and applying it directly to the ball
      m_nudge = g_pplayer->GetRawAccelerometer();
   }

   // legacy/VP9 style keyboard nudging
   if (m_legacyNudgeTime != 0)
   {
      m_legacyNudgeTime--;
      if (m_legacyNudgeTime == 95)
      {
         m_nudge.x = -m_legacyNudgeBack.x * 2.0f;
         m_nudge.y = m_legacyNudgeBack.y * 2.0f;
      }
      else if (m_legacyNudgeTime == 90)
      {
         m_nudge.x = m_legacyNudgeBack.x;
         m_nudge.y = -m_legacyNudgeBack.y;
      }
   }

   // Apply our filter to the nudge data
   if (m_enableNudgeFilter)
   {
      m_nudgeFilterX.sample(m_nudge.x, m_curPhysicsFrameTime);
      m_nudgeFilterY.sample(m_nudge.y, m_curPhysicsFrameTime);
   }

   if (m_enablePlumbTilt && m_plumbTiltThreshold > 0.0f && dtime > 0.f && dtime <= 0.1f) // Ignore large time slices... forces will get crazy!
   {
      #if 1
      // If you modify this function... make sure you update the same function in the front-end!
      //
      // The physics on the plumb are not very accurate... but they seem to do good enough to convince players.
      // In the real world, the plumb is a pendulum.  With force of gravity, the force of the string, the friction
      // force of the hook, and the dampening force of the air.  Here, force is exerted only by the table tilting
      // (the string) and dampening and friction forces are lumped together with a constant, and gravity is not
      // present.
      const Vertex2D &nudge = m_nudge;
      const float ax = sinf((nudge.x - m_plumb.x) * (float)(M_PI / 5.0));
      const float ay = sinf((nudge.x - m_plumb.y) * (float)(M_PI / 5.0));

      // Add force to the plumb.
      m_plumbVel.x += (float)(825.0 * 0.25) * ax * dtime;
      m_plumbVel.y += (float)(825.0 * 0.25) * ay * dtime;

      // Check if we hit the edge.
      const float len2 = m_plumb.LengthSquared();
      const float TiltPerc = (len2 * 100.0f) / ((1.0f - m_plumbTiltThreshold) * (1.0f - m_plumbTiltThreshold));
      bool tilted = false;
      if (TiltPerc > 100.0f)
      {
         // Bounce the plumb and scrub velocity.
         const float oolen = ((1.0f - m_plumbTiltThreshold) / sqrtf(len2)) * 0.90f;
         m_plumb *= oolen;
         m_plumbVel *= -0.025f;
         tilted = true;
      }

      // Dampen the velocity.
      m_plumbVel.x -= 2.50f * (m_plumbVel.x * dtime);
      m_plumbVel.y -= 2.50f * (m_plumbVel.y * dtime);

      // Check if velocity is near zero and we near center.
      if (((m_plumbVel.x + m_plumbVel.y) > -VELOCITY_EPSILON) && ((m_plumbVel.x + m_plumbVel.y) < VELOCITY_EPSILON) 
         && (m_plumb.x > -0.10f) && (m_plumb.x < 0.10f) && (m_plumb.y > -0.10f) && (m_plumb.y < 0.10f))
      {
         // Set the velocity to zero. This reduces annoying jittering when at rest.
         m_plumbVel.SetZero();
         m_plumb.SetZero();
      }

      // Update position.
      m_plumb += m_plumbVel * dtime;

      // Fire event (same as keyboard tilt)
      if (m_plumbTiltHigh != tilted)
      {
         m_plumbTiltHigh = tilted;
         g_pplayer->m_pininput.FireKeyEvent(m_plumbTiltHigh ? DISPID_GameEvents_KeyDown : DISPID_GameEvents_KeyUp, g_pplayer->m_rgKeys[eCenterTiltKey]);
      }

      #else
      // Alternative implementation (work in progress for VPX 10.8):
      // - use full VPX 10 nudge (include table velocity) instead of simplified one (only nudge acceleration without table)
      // - simulate a simplified pendulum with 3 (simplified) forces: gravity, nudge and string
      // - not used to send fake keyboard nudge, but to send mechanical tilt (like the plumb on real machine, triggering rom)
      // Therefore likely not backward compatible at all, so disabled...
      Vertex2D nudge = Vertex2D(GetNudge().x, GetNudge().y);

      // Dampen the velocity.
      m_plumbVel *= max(0.f, 1.f - 0.01f * dtime);

      // Add force to the plumb (largely simplified model where nudge push away while gravity takes back, the string nullifying force along its axis)
      const float len = m_plumb.Length();
      const float alpha = atan2f(len, 100.f); // the string length is a magic number since the model is not correct
      const float nudgeCoef = cosf(alpha) * 5.f; // again a magic number for the nudge strength (this factor should be computed from the mass of the plumb)
      const float gravCoef = sinf(alpha) * 0.97f * GRAVITYCONST;
      m_plumbVel += dtime * (nudgeCoef * nudge - gravCoef * m_plumb);

      // Check if we hit the edge.
      const float TiltPerc = 100.f * len / (1.0f - m_plumbTiltThreshold);
      bool tilted = false;
      if (TiltPerc > 100.0f)
      {
         // Bounce the plumb and scrub velocity.
         const float oolen = ((1.0f - m_plumbTiltThreshold) / len) * 0.90f;
         m_plumb *= oolen;
         m_plumbVel *= -0.925f;
         tilted = true;
      }

      // Update position.
      m_plumb += m_plumbVel * dtime;

      // Fire event (same as keyboard tilt)
      if (m_plumbTiltHigh != tilted)
      {
         m_plumbTiltHigh = tilted;
         g_pplayer->m_pininput.FireKeyEvent(m_plumbTiltHigh ? DISPID_GameEvents_KeyDown : DISPID_GameEvents_KeyUp, g_pplayer->m_rgKeys[eMechanicalTilt]);
      }
      #endif

      // Update player for diagnostic/table script visibility.  Only update if input value is larger than what's there.
      // When the table script reads the values, they will reset to 0.
      if (TiltPerc > g_pplayer->m_ptable->m_tblNudgeReadTilt)
         g_pplayer->m_ptable->m_tblNudgeReadTilt = TiltPerc;
      if (fabsf(nudge.x) > fabsf(g_pplayer->m_ptable->m_tblNudgeRead.x))
         g_pplayer->m_ptable->m_tblNudgeRead.x = nudge.x;
      if (fabsf(nudge.y) > fabsf(g_pplayer->m_ptable->m_tblNudgeRead.y))
         g_pplayer->m_ptable->m_tblNudgeRead.y = nudge.y;
      if (fabsf(m_plumb.x) > fabsf(g_pplayer->m_ptable->m_tblNudgePlumb.x))
         g_pplayer->m_ptable->m_tblNudgePlumb.x = m_plumb.x;
      if (fabsf(m_plumb.y) > fabsf(g_pplayer->m_ptable->m_tblNudgePlumb.y))
         g_pplayer->m_ptable->m_tblNudgePlumb.y = m_plumb.y;
   }
}

Vertex3Ds PhysicsEngine::GetNudge() const
{
   return Vertex3Ds(m_nudge.x - m_tableVelDelta.x, m_nudge.y - m_tableVelDelta.y, 0.f);
}

Vertex2D PhysicsEngine::GetScreenNudge() const
{
   // NB: in table coordinates, +Y points down, but in screen coordinates, it points up, so we have to flip the y component
   if (m_legacyNudge)
      return Vertex2D(m_legacyNudgeBack.x * sqrf((float)m_legacyNudgeTime * 0.01f), -m_legacyNudgeBack.y * sqrf((float)m_legacyNudgeTime * 0.01f));
   else
      return Vertex2D(m_tableDisplacement.x, -m_tableDisplacement.y);
}

void PhysicsEngine::RayCast(const Vertex3Ds &source, const Vertex3Ds &target, const bool uiCast, vector<HitObject *> &vhoHit)
{
   // First time the debug hit-testing has been used
   if (uiCast && m_vUIHitObjects.empty())
   {
      for (IEditable *const pe : g_pplayer->m_ptable->m_vedit)
      {
         if (pe->GetIHitable())
         {
            const size_t currentsize = m_vUIHitObjects.size();
            pe->GetIHitable()->GetHitShapesDebug(m_vUIHitObjects);
            // Save the objects the trouble of having the set the idispatch pointer themselves
            for (size_t hitloop = currentsize, newsize = m_vUIHitObjects.size(); hitloop < newsize; hitloop++)
            {
               m_vUIHitObjects[hitloop]->m_editable = pe;
               m_vUIHitObjects[hitloop]->CalcHitBBox(); // maybe needed to update here, as only done lazily for some objects (i.e. balls!)
               m_UIOctree.AddElement(m_vUIHitObjects[hitloop]);
            }
         }
      }
      const FRect3D bbox = g_pplayer->m_ptable->GetBoundingBox();
      m_UIOctree.Initialize(FRect(bbox.left, bbox.right, bbox.top, bbox.bottom));
   }

   // Create a ray (ball) that travels in 3D space along the given ray, and find what it intersects with.
   Ball ballT;
   ballT.m_d.m_pos = source;
   ballT.m_d.m_vel = target - source;
   ballT.m_d.m_radius = 0.f;
   ballT.m_coll.m_hittime = 1.0f;
   ballT.CalcHitBBox(); // need to update here, as only done lazily

   m_hitoctree_dynamic.HitTestXRay(&ballT, vhoHit, ballT.m_coll);
   m_hitoctree.HitTestXRay(&ballT, vhoHit, ballT.m_coll);
   if (uiCast)
      m_UIOctree.HitTestXRay(&ballT, vhoHit, ballT.m_coll);
}


void PhysicsEngine::OnPrepareFrame()
{
   // Reset per frame debug counters
   #ifdef DEBUGPHYSICS
   c_hitcnts = 0;
   c_collisioncnt = 0;
   c_contactcnt = 0;
   #ifdef C_DYNAMIC
   c_staticcnt = 0;
   #endif
   c_embedcnts = 0;
   c_timesearch = 0;

   c_traversed = 0;
   c_tested = 0;
   c_deepTested = 0;
   #endif
}

void PhysicsEngine::OnFinishFrame()
{
   m_lastFlipTime = usec();
}

void PhysicsEngine::StartPhysics()
{
   m_StartTime_usec = usec();
   m_curPhysicsFrameTime = m_StartTime_usec;
   m_nextPhysicsFrameTime = m_curPhysicsFrameTime + PHYSICS_STEPTIME;

#ifdef PLAYBACK
   if (m_playback)
   {
      float physicsStepTime;
      ParseLog((LARGE_INTEGER*)&physicsStepTime, (LARGE_INTEGER*)&m_StartTime_usec);
   }
#endif

#ifdef LOG
   PLOGD.printf("Step Time %llu", m_StartTime_usec);
   PLOGD.printf("End Frame");
#endif
}

void PhysicsEngine::UpdatePhysics()
{
   if (!g_pplayer) //!! meh, we have a race condition somewhere where we delete g_pplayer while still in use (e.g. if we have a script compile error and cancel the table start)
      return;

   g_frameProfiler.EnterProfileSection(FrameProfiler::PROFILE_PHYSICS);
   U64 initial_time_usec = usec();

   // DJRobX's crazy latency-reduction code
   U64 delta_frame = 0;
   if (g_pplayer->m_minphyslooptime > 0 && m_lastFlipTime > 0)
   {
      // We want the physics loops to sync up to the frames, not
      // the post-render period, as that can cause some judder.
      delta_frame = initial_time_usec - m_lastFlipTime;
      initial_time_usec -= delta_frame;
   }

   if (g_pplayer->m_noTimeCorrect) // After debugging script
   {
      // Shift whole game forward in time
      m_StartTime_usec       += initial_time_usec - m_curPhysicsFrameTime;
      m_nextPhysicsFrameTime += initial_time_usec - m_curPhysicsFrameTime;
      m_curPhysicsFrameTime   = initial_time_usec; // 0 time frame
      g_pplayer->m_noTimeCorrect = false;
   }

#ifdef STEPPING
#ifndef EVENPHYSICSTIME
   if (g_pplayer->m_debugWindowActive || g_pplayer->m_userDebugPaused)
   {
      // Shift whole game forward in time
      m_StartTime_usec       += initial_time_usec - m_curPhysicsFrameTime;
      m_nextPhysicsFrameTime += initial_time_usec - m_curPhysicsFrameTime;
      if (g_pplayer->m_step)
      {
         // Walk one physics step forward
         m_curPhysicsFrameTime = initial_time_usec - PHYSICS_STEPTIME;
         g_pplayer->m_step = false;
      }
      else
         m_curPhysicsFrameTime = initial_time_usec; // 0 time frame
   }
#endif
#endif

#ifdef EVENPHYSICSTIME
   if (!m_pause || m_step)
   {
      initial_time_usec = m_curPhysicsFrameTime - 3547811060 + 3547825450;
      m_step = false;
   }
   else
      initial_time_usec = m_curPhysicsFrameTime;
#endif

#ifdef LOG
   const double timepassed = (double)(initial_time_usec - m_curPhysicsFrameTime) / 1000000.0;
   const float frametime =
   #ifdef PLAYBACK
      (!m_playback) ? (float)(timepassed * 100.0) : ParseLog((LARGE_INTEGER*)&initial_time_usec, (LARGE_INTEGER*)&m_nextPhysicsFrameTime);
   #else
      #define TIMECORRECT 1
      #ifdef TIMECORRECT
         (float)(timepassed * 100.0);
         // 1.456927f;
      #else
         0.45f;
      #endif
   #endif //PLAYBACK
   PLOGD.printf("Frame Time %.20f %u %u %u %u", frametime, initial_time_usec >> 32, initial_time_usec, m_nextPhysicsFrameTime >> 32, m_nextPhysicsFrameTime);
   PLOGD.printf("End Frame");
#endif

   m_phys_iterations = 0;

   while (m_curPhysicsFrameTime < initial_time_usec) // loop here until current (real) time matches the physics (simulated) time
   {
      // Get time in milliseconds for timers
      g_pplayer->m_time_msec = (U32)((m_curPhysicsFrameTime - m_StartTime_usec) / 1000);

      m_phys_iterations++;

      // Get the time until the next physics tick is done, and get the time
      // until the next frame is done
      // If the frame is the next thing to happen, update physics to that
      // point next update acceleration, and continue loop

      const float physics_diff_time = (float)((double)(m_nextPhysicsFrameTime - m_curPhysicsFrameTime)*(1.0 / DEFAULT_STEPTIME));
      //const float physics_to_graphic_diff_time = (float)((double)(initial_time_usec - m_curPhysicsFrameTime)*(1.0 / DEFAULT_STEPTIME));

      //if (physics_to_graphic_diff_time < physics_diff_time)          // is graphic frame time next???
      //{
      //      PhysicsSimulateCycle(physics_to_graphic_diff_time);      // advance physics to this time
      //      m_curPhysicsFrameTime = initial_time_usec;               // now current to the wall clock
      //      break;  //this is the common exit from the loop          // exit skipping accelerate
      //}                     // some rare cases will exit from while()


      // DJRobX's crazy latency-reduction code: Artificially lengthen the execution of the physics loop by X usecs, to give more opportunities to read changes from input(s) (try values in the multiple 100s up to maximum 1000 range, in general: the more, the faster the CPU is)
      //                                        Intended mainly to be used if vsync is enabled (e.g. most idle time is shifted from vsync-waiting to here)
      if (g_pplayer->m_minphyslooptime > 0)
      {
         const U64 basetime = usec();
         const U64 targettime = ((U64)g_pplayer->m_minphyslooptime * m_phys_iterations) + m_lastFlipTime;
         // If we're 3/4 of the way through the loop, fire a "controller sync" timer (timers with an interval set to -2) event so VPM can react to input.
         if (m_phys_iterations == 750 / ((int)g_pplayer->m_fps + 1))
            g_pplayer->FireSyncController();
         if (basetime < targettime)
         {
            g_frameProfiler.EnterProfileSection(FrameProfiler::PROFILE_SLEEP);
            uSleep(targettime - basetime);
            g_frameProfiler.ExitProfileSection();
         }
      }
      // end DJRobX's crazy code
      
      const U64 cur_time_usec = usec()-delta_frame; //!! one could also do this directly in the while loop condition instead (so that the while loop will really match with the current time), but that leads to some stuttering on some heavy frames

      // hung in the physics loop over 200 milliseconds or the number of physics iterations to catch up on is high (i.e. very low/unplayable FPS)
      if ((cur_time_usec - initial_time_usec > 200000) || (m_physicsMaxLoops != 0 && m_phys_iterations > m_physicsMaxLoops))
      {                                                             // can not keep up to real time
         m_curPhysicsFrameTime  = initial_time_usec;                // skip physics forward ... slip-cycles -> 'slowed' down physics
         m_nextPhysicsFrameTime = initial_time_usec + PHYSICS_STEPTIME;
         break;                                                     // go draw frame
      }

      //update keys, hid, plumb, nudge, timers, etc
      //const U32 sim_msec = (U32)(m_curPhysicsFrameTime / 1000);
      const U32 cur_time_msec = (U32)(cur_time_usec / 1000);

      g_pplayer->m_pininput.ProcessKeys(/*sim_msec,*/ cur_time_msec);

      mixer_update();
      hid_update(/*sim_msec*/cur_time_msec);

      #ifdef ACCURATETIMERS
      g_pplayer->ApplyDeferredTimerChanges();
      if (g_pplayer->m_videoSyncMode == VideoSyncMode::VSM_FRAME_PACING || g_frameProfiler.Get(FrameProfiler::PROFILE_SCRIPT) <= 1000 * MAX_TIMERS_MSEC_OVERALL) // if overall script time per frame exceeded, skip
         g_pplayer->FireTimers(g_pplayer->m_time_msec);
      #endif

      g_pplayer->MechPlungerUpdate(); // integral physics frame. So the previous graphics frame was (1.0 - physics_diff_time) before 
      // this integral physics frame. Accelerations and inputs are always physics frame aligned

      UpdateNudge(physics_diff_time);

      for (size_t i = 0; i < m_vmover.size(); i++)
         m_vmover[i]->UpdateVelocities();      // always on integral physics frame boundary (spinner, gate, flipper, plunger, ball)

      //primary physics loop
      PhysicsSimulateCycle(physics_diff_time); // main simulator call

      //ball trail, keep old pos of balls
      for (size_t i = 0; i < g_pplayer->m_vball.size(); i++)
      {
         Ball *const pball = g_pplayer->m_vball[i];
         pball->m_oldpos[pball->m_ringcounter_oldpos / (10000 / PHYSICS_STEPTIME)] = pball->m_d.m_pos;

         pball->m_ringcounter_oldpos++;
         if (pball->m_ringcounter_oldpos == MAX_BALL_TRAIL_POS*(10000 / PHYSICS_STEPTIME))
            pball->m_ringcounter_oldpos = 0;
      }

      //PLOGD << "PT: " << physics_diff_time << " " << physics_to_graphic_diff_time << " " << (U32)(m_curPhysicsFrameTime/1000) << " " << (U32)(initial_time_usec/1000) << " " << cur_time_msec;

      m_curPhysicsFrameTime = m_nextPhysicsFrameTime; // new cycle, on physics frame boundary
      m_nextPhysicsFrameTime += PHYSICS_STEPTIME;     // advance physics position
   } // end while (m_curPhysicsFrameTime < initial_time_usec)

   g_frameProfiler.ExitProfileSection();
}

void PhysicsEngine::PhysicsSimulateCycle(float dtime) // move physics forward to this time
{
   // PLOGD << "Cycle " << dtime;

   int StaticCnts = STATICCNTS; // maximum number of static counts
   // it's okay to have this code outside of the inner loop, as the ball hitrects already include the maximum distance they can travel in that timespan
   m_hitoctree_dynamic.Update();

   while (dtime > 0.f)
   {
      // first find hits, if any +++++++++++++++++++++ 
#ifdef DEBUGPHYSICS
      c_timesearch++;
#endif
      float hittime = dtime;       // begin time search from now ...  until delta ends

      // find earliest time where a flipper collides with its stop
      for (size_t i = 0; i < m_vFlippers.size(); ++i)
      {
         const float fliphit = m_vFlippers[i]->GetHitTime();
         //if ((fliphit >= 0.f) && !sign(fliphit) && (fliphit <= hittime))
         if ((fliphit > 0.f) && (fliphit <= hittime)) //!! >= 0.f causes infinite loop
            hittime = fliphit;
      }

      m_recordContacts = true;
      m_contacts.clear();

#ifdef USE_EMBREE
      for (size_t i = 0; i < m_vball.size(); i++)
         if (!m_vball[i]->m_d.m_lockedInKicker
#ifdef C_DYNAMIC
             && m_vball[i]->m_dynamic > 0
#endif
            ) // don't play with frozen balls
         {
            m_vball[i]->m_coll.m_hittime = hittime; // search upto current hittime
            m_vball[i]->m_coll.m_obj = nullptr;
         }

      if (!m_vball.empty())
      {
         m_hitoctree.HitTestBall(m_vball);         // find the hit objects hit times
         m_hitoctree_dynamic.HitTestBall(m_vball); // dynamic objects !! should reuse the same embree scene created already in m_hitoctree.HitTestBall!
      }
#endif

      for (size_t i = 0; i < g_pplayer->m_vball.size(); i++)
      {
         Ball *const pball = g_pplayer->m_vball[i];

         if (!pball->m_d.m_lockedInKicker
#ifdef C_DYNAMIC
             && pball->m_dynamic > 0
#endif
            ) // don't play with frozen balls
         {
#ifndef USE_EMBREE
            pball->m_coll.m_hittime = hittime;          // search upto current hittime
            pball->m_coll.m_obj = nullptr;
#endif
            // always check for playfield and top glass
            if (g_pplayer->m_implicitPlayfieldMesh)
               DoHitTest(pball, &m_hitPlayfield, pball->m_coll);

            DoHitTest(pball, &m_hitTopGlass, pball->m_coll);

#ifndef USE_EMBREE
            if (rand_mt_01() < 0.5f) // swap order of dynamic and static obj checks randomly
            {
               m_hitoctree_dynamic.HitTestBall(pball, pball->m_coll); // dynamic objects
               m_hitoctree.HitTestBall(pball, pball->m_coll);         // find the static hit objects hit times
            }
            else
            {
               m_hitoctree.HitTestBall(pball, pball->m_coll);         // find the static hit objects hit times
               m_hitoctree_dynamic.HitTestBall(pball, pball->m_coll); // dynamic objects
            }
#endif
            const float htz = pball->m_coll.m_hittime; // this ball's hit time
            if (htz < 0.f) pball->m_coll.m_obj = nullptr; // no negative time allowed

            if (pball->m_coll.m_obj)                   // hit object
            {
#ifdef DEBUGPHYSICS
               ++c_hitcnts;                            // stats for display

               if (/*pball->m_coll.m_hitRigid &&*/ pball->m_coll.m_hitdistance < -0.0875f) //rigid and embedded
                  ++c_embedcnts;
#endif
               ///////////////////////////////////////////////////////////////////////////

               if (htz <= hittime)                     // smaller hit time??
               {
                  hittime = htz;                       // record actual event time

                  if (hittime < STATICTIME)            // less than static time interval
                  {
                     /*if (!pball->m_coll.m_hitRigid) hittime = STATICTIME; // non-rigid ... set Static time
                     else*/ if (--StaticCnts < 0)
                     {
                        StaticCnts = 0;                // keep from wrapping
                        hittime = STATICTIME;
                     }
                  }
               }
            }
         }
      } // end loop over all balls

      m_recordContacts = false;

      // hittime now set ... or full frame if no hit 
      // now update displacements to collide-contact or end of physics frame
      // !!!!! 2) move objects to hittime

      if (hittime > STATICTIME) StaticCnts = STATICCNTS; // allow more zeros next round

      for (size_t i = 0; i < m_vmover.size(); i++)
         m_vmover[i]->UpdateDisplacements(hittime); // step 2: move the objects about according to velocities (spinner, gate, flipper, plunger, ball)

      // find balls that need to be collided and script'ed (generally there will be one, but more are possible)

      for (size_t i = 0; i < g_pplayer->m_vball.size(); i++) // use m_vball.size(), in case script deletes a ball
      {
         Ball *const pball = g_pplayer->m_vball[i];

         if (
#ifdef C_DYNAMIC
             pball->m_dynamic > 0 &&
#endif
             pball->m_coll.m_obj && pball->m_coll.m_hittime <= hittime) // find balls with hit objects and minimum time
         {
            // now collision, contact and script reactions on active ball (object)+++++++++
            HitObject * const pho = pball->m_coll.m_obj; // object that ball hit in trials
            g_pplayer->m_pactiveball = pball; // For script that wants the ball doing the collision
#ifdef DEBUGPHYSICS
            c_collisioncnt++;
#endif
            pho->Collide(pball->m_coll);                 //!!!!! 3) collision on active ball
            pball->m_coll.m_obj = nullptr;                  // remove trial hit object pointer

            // Collide may have changed the velocity of the ball, 
            // and therefore the bounding box for the next hit cycle
            if (g_pplayer->m_vball[i] != pball) // Ball still exists? may have been deleted from list
            {
               // collision script deleted the ball, back up one count
               --i;
               continue;
            }
            else
            {
#ifdef C_DYNAMIC
               // is this ball static? .. set static and quench        
               if (/*pball->m_coll.m_hitRigid &&*/ (pball->m_coll.m_hitdistance < (float)PHYS_TOUCH)) //rigid and close distance contacts //!! rather test isContact??
               {
                  const float mag = pball->m_vel.x*pball->m_vel.x + pball->m_vel.y*pball->m_vel.y; // values below are taken from simulation
                  if (pball->m_drsq < 8.0e-5f && mag < 1.0e-3f*m_gravity*m_gravity / GRAVITYCONST / GRAVITYCONST && fabsf(pball->m_vel.z) < 0.2f*m_gravity / GRAVITYCONST)
                  {
                     if (--pball->m_dynamic <= 0)             //... ball static, cancels next gravity increment
                     {                                       // m_dynamic is cleared in ball gravity section
                        pball->m_dynamic = 0;
#ifdef DEBUGPHYSICS
                        c_staticcnt++;
#endif
                        pball->m_vel.x = pball->m_vel.y = pball->m_vel.z = 0.f; //quench the remaining velocity and set ...
                     }
                  }
               }
#endif
            }
         }
      }

#ifdef DEBUGPHYSICS
      c_contactcnt = (U32)m_contacts.size();
#endif
      /*
       * Now handle contacts.
       *
       * At this point UpdateDisplacements() was already called, so the state is different
       * from that at HitTest(). However, contacts have zero relative velocity, so
       * hopefully nothing catastrophic has happened in the meanwhile.
       *
       * Maybe a two-phase setup where we first process only contacts, then only collisions
       * could also work.
       */
      if (rand_mt_01() < 0.5f) // swap order of contact handling randomly
         for (size_t i = 0; i < m_contacts.size(); ++i)
            //if (m_contacts[i].m_hittime <= hittime) // does not happen often, and values then look sane, so do this check //!! why does this break some collisions (MM NZ&TT Reloaded Skitso, also CCC (Saloon))? maybe due to ball colliding with multiple things and then some sideeffect?
               m_contacts[i].m_obj->Contact(m_contacts[i], hittime);
      else
         for (size_t i = m_contacts.size() - 1; i != -1; --i)
            //if (m_contacts[i].m_hittime <= hittime) // does not happen often, and values then look sane, so do this check //!! why does this break some collisions (MM NZ&TT Reloaded Skitso, also CCC (Saloon))? maybe due to ball colliding with multiple things and then some sideeffect?
               m_contacts[i].m_obj->Contact(m_contacts[i], hittime);

      m_contacts.clear();

#ifdef C_BALL_SPIN_HACK
      // hacky killing of ball spin on resting balls (very low and very high spinning)
      for (size_t i = 0; i < g_pplayer->m_vball.size(); i++)
      {
         Ball *const pball = g_pplayer->m_vball[i];

         const unsigned int p0 = (pball->m_ringcounter_oldpos / (10000 / PHYSICS_STEPTIME) + 1) % MAX_BALL_TRAIL_POS;
         const unsigned int p1 = (pball->m_ringcounter_oldpos / (10000 / PHYSICS_STEPTIME) + 2) % MAX_BALL_TRAIL_POS;

         if (/*pball->m_coll.m_hitRigid &&*/ (pball->m_coll.m_hitdistance < (float)PHYS_TOUCH) && (pball->m_oldpos[p0].x != FLT_MAX) && (pball->m_oldpos[p1].x != FLT_MAX)) // only if already initialized
         {
            /*const float mag = pball->m_vel.x*pball->m_vel.x + pball->m_vel.y*pball->m_vel.y; // values below are copy pasted from above
            if (pball->m_drsq < 8.0e-5f && mag < 1.0e-3f*m_gravity*m_gravity / GRAVITYCONST / GRAVITYCONST && fabsf(pball->m_vel.z) < 0.2f*m_gravity / GRAVITYCONST
            && pball->m_angularmomentum.Length() < 0.9f*m_gravity / GRAVITYCONST
            ) //&& rand_mt_01() < 0.95f)
            {
            pball->m_angularmomentum *= 0.05f; // do not kill spin completely, otherwise stuck balls will happen during regular gameplay
            }*/

            const Vertex3Ds diff_pos = pball->m_oldpos[p0] - pball->m_d.m_pos;
            const float mag = diff_pos.x*diff_pos.x + diff_pos.y*diff_pos.y;
            const Vertex3Ds diff_pos2 = pball->m_oldpos[p1] - pball->m_d.m_pos;
            const float mag2 = diff_pos2.x*diff_pos2.x + diff_pos2.y*diff_pos2.y;

            const float threshold = (pball->m_angularmomentum.x*pball->m_angularmomentum.x + pball->m_angularmomentum.y*pball->m_angularmomentum.y) / max(mag, mag2);

            if (!infNaN(threshold) && threshold > 666.f)
            {
               const float damp = clamp(1.0f - (threshold - 666.f) / 10000.f, 0.23f, 1.f); // do not kill spin completely, otherwise stuck balls will happen during regular gameplay
               pball->m_angularmomentum *= damp;
            }
         }
      }
#endif

      dtime -= hittime;       //new delta .. i.e. time remaining

      m_swap_ball_collision_handling = !m_swap_ball_collision_handling; // swap order of ball-ball collisions

   } // end physics loop
}

string PhysicsEngine::GetPerfInfo(bool resetMax)
{
   if (resetMax || g_frameProfiler.GetPrev(FrameProfiler::PROFILE_PHYSICS) > m_phys_max)
      m_phys_max = g_frameProfiler.GetPrev(FrameProfiler::PROFILE_PHYSICS);

   if (resetMax || m_phys_iterations > m_phys_max_iterations)
      m_phys_max_iterations = m_phys_iterations;

   if (m_count == 0)
   {
      m_phys_total_iterations = m_phys_iterations;
      m_count = 1;
   }
   else
   {
      m_phys_total_iterations += m_phys_iterations;
      m_count++;
   }

   std::ostringstream info;
   info << std::fixed << std::setprecision(1);

   info << "Physics: " << m_phys_iterations << " iterations per frame (" << ((U32)(m_phys_total_iterations / m_count)) << " avg " << m_phys_max_iterations
        << " max)\n";
#ifdef DEBUGPHYSICS
   info << std::setprecision(5);
   info << "Hits:" << c_hitcnts << " Collide:" << c_collisioncnt << " Ctacs:" << c_contactcnt;
#ifdef C_DYNAMIC
   info << " Static:" << c_staticcnt;
#endif
   info << " Embed:" << c_embedcnts << " TimeSearch:" << c_timesearch << "\n";
   info << "kDObjects:" << m_hitoctree_dynamic.GetObjectCount() << " kD:" << m_hitoctree_dynamic.GetNLevels()
        << " QuadObjects:" << m_hitoctree.GetObjectCount() << " Quadtree:" << m_hitoctree.GetNLevels()
        << " Traversed:" << c_traversed << " Tested:" << c_tested << " DeepTested:" << c_deepTested << "\n";
   info << std::setprecision(1);
#endif

   return info.str();
}
