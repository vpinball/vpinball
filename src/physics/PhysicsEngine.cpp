// license:GPLv3+

#include "core/stdafx.h"

#include <iomanip>

#include "PhysicsEngine.h"

#include "plugins/MsgPlugin.h"
#include "plugins/VPXPlugin.h"
#include "core/VPXPluginAPIImpl.h"

#include "utils/ushock_output.h"

PhysicsEngine::PhysicsEngine(PinTable *const table)
   : m_hitPlayfield(table)
   , m_hitTopGlass(table)
   , m_nudgeFilterX("x")
   , m_nudgeFilterY("y")
{
   m_physicsMaxLoops = table->m_PhysicsMaxLoops == 0xFFFFFFFFu ? 0 : table->m_PhysicsMaxLoops * (10000 / PHYSICS_STEPTIME) /*2*/;
   m_contacts.reserve(8);

   m_plumbMassFactor = 10.f;
   m_plumbPoleLength = 0.020f;
   m_plumbPos.Set(0.f, 0.f, -m_plumbPoleLength);
   m_plumbVel.Set(0.f, 0.f, 0.f);

   // Initialize legacy nudging.
   m_legacyNudgeBack = Vertex2D(0.f, 0.f); 
   m_nudgeAcceleration.SetZero();

   // Initialize new nudging.
   m_tableVel.SetZero();
   m_tableDisplacement.SetZero();
   m_tableVelOld.SetZero();
   m_tableAcceleration.SetZero();

   // Initialize velocity-based accelerometer sensor input.
   m_prevSensorTableVelocity.SetZero();

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

   ReadNudgeSettings(table->m_settings);

   // Collect all hit objects
   const FRect3D tableBounds = g_pplayer->m_ptable->GetBoundingBox();
   m_hitoctree.SetBounds(FRect(tableBounds.left, tableBounds.right, tableBounds.top, tableBounds.bottom)); // Limit to table bounds as we don't expect to play outside of it
   m_pendingHitObjects = &m_hitoctree.BeginReset();
   m_pendingHitObjects->clear();
   for (IEditable *const pe : table->m_vedit)
   {
      Hitable * const ph = pe->GetIHitable();
      if (ph)
      {
         #ifdef DEBUGPHYSICS
         g_pplayer->m_progressDialog.SetProgress("Initializing Object-Physics " + pe->GetName() + "...");
         #endif
         ph->PhysicSetup(this, false);
      }
   }
   AddCabinetBoundingHitShapes(table);
   for (HitObject *const pho : *m_pendingHitObjects)
   {
      if (pho->GetType() == eFlipper)
         m_vFlippers.push_back(static_cast<HitFlipper*>(pho));
      MoverObject * const pmo = pho->GetMoverObject();
      if (pmo && pmo->AddToList()) // Spinner, Gate, Flipper, Plunger (ball is added separately on each create ball)
         m_vmover.push_back(pmo);
   }

   PLOGI << "Initializing octree"; // For profiling
   m_hitoctree.EndReset();
   m_hitoctree.Finalize();
   m_pendingHitObjects = nullptr;
   #if !defined(NDEBUG) && defined(PRINT_DEBUG_COLLISION_TREE)
      m_hitoctree.DumpTree(0);
   #endif

   m_onUpdatePhysicsMsgId = VPXPluginAPIImpl::GetMsgID(VPXPI_NAMESPACE, VPXPI_EVT_ON_UPDATE_PHYSICS);

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
   delete m_UIQuadTtree;

   if (m_pendingHitObjects)
      ReleaseVHO(*m_pendingHitObjects, false);
   ReleaseVHO(m_hitoctree.GetHitObjects(), false);
   
   VPXPluginAPIImpl::ReleaseMsgID(m_onUpdatePhysicsMsgId);
   
   // We should release objects from the dynamic tree except HitBall (but there are only HitBall...)
}

void PhysicsEngine::ReleaseVHO(const vector<HitObject *> &vho, bool isUI)
{
   vector<IEditable *> editables;
   for (size_t i = 0; i < vho.size(); i++)
   {
      if (FindIndexOf(editables, vho[i]->m_editable) == -1)
      {
         editables.push_back(vho[i]->m_editable);
         if (vho[i]->m_editable->GetIHitable())
            vho[i]->m_editable->GetIHitable()->PhysicRelease(this, isUI);
      }
      if (vho[i]->GetType() != eBall) // As balls own their HitBall hit object
         delete vho[i];
   }
}

void PhysicsEngine::SetGravity(float slopeDeg, float strength)
{
   m_gravity.x = 0;
   m_gravity.y = sinf(ANGTORAD(slopeDeg)) * strength;
   m_gravity.z = -cosf(ANGTORAD(slopeDeg)) * strength;
}

void PhysicsEngine::AddCollider(HitObject *collider, const bool isUI)
{
   assert(collider->m_editable != nullptr);
   collider->CalcHitBBox();
   if (!isUI && (collider->GetType() == eBall))
   {
      m_vmover.push_back(&static_cast<HitBall*>(collider)->m_mover); // balls are always added separately to this list!
      m_hitoctree_dynamic.Insert(collider);
   }
   else
   {
      assert(m_pendingHitObjects != nullptr);
      m_pendingHitObjects->push_back(collider);
      //assert(collider->m_hitBBox.left >= -100000); // Invalid bound
   }
}

void PhysicsEngine::RemoveCollider(HitObject * collider, const bool isUI)
{
   assert(collider->GetType() == eBall); // Ball are the only object owning its HitObject, therefore the only one allowed to call here
   if (!isUI)
   {
      RemoveFromVectorSingle<MoverObject *>(m_vmover, &static_cast<HitBall *>(collider)->m_mover);
      m_hitoctree_dynamic.Remove(collider);
   }
}

void PhysicsEngine::CollectColliders(IEditable *editable, vector<HitObject *> *hitObjects, bool isUI)
{
   assert(m_pendingHitObjects == nullptr);
   m_pendingHitObjects = hitObjects;
   if (editable->GetIHitable())
      editable->GetIHitable()->PhysicSetup(this, isUI);
   m_pendingHitObjects = nullptr;
}

void PhysicsEngine::AddCabinetBoundingHitShapes(PinTable *const table)
{
   // TODO these are the only colliders that are added without an associated editable. We could enforce one here to guarantee m_editable != nullptr and simplify the code
   
   // simple outer borders:
   AddCollider(new LineSeg(table, Vertex2D(table->m_right, table->m_top), Vertex2D(table->m_right, table->m_bottom), 0.f, table->m_glassTopHeight), false);
   AddCollider(new LineSeg(table, Vertex2D(table->m_left, table->m_bottom), Vertex2D(table->m_left, table->m_top), 0.f, table->m_glassBottomHeight), false);
   AddCollider(new LineSeg(table, Vertex2D(table->m_right, table->m_bottom), Vertex2D(table->m_left, table->m_bottom), 0.f, table->m_glassBottomHeight),  false);
   AddCollider(new LineSeg(table, Vertex2D(table->m_left, table->m_top), Vertex2D(table->m_right, table->m_top), 0.f, table->m_glassTopHeight), false);

   // glass:
   Vertex3Ds * const rgv3D = new Vertex3Ds[4];
   rgv3D[0] = Vertex3Ds(table->m_left, table->m_top, table->m_glassTopHeight);
   rgv3D[1] = Vertex3Ds(table->m_right, table->m_top, table->m_glassTopHeight);
   rgv3D[2] = Vertex3Ds(table->m_right, table->m_bottom, table->m_glassBottomHeight);
   rgv3D[3] = Vertex3Ds(table->m_left, table->m_bottom, table->m_glassBottomHeight);
   AddCollider(new Hit3DPoly(table, rgv3D, 4), false);

   /*
   // playfield:
   Vertex3Ds * const rgv3D = new Vertex3Ds[4];
   rgv3D[3] = Vertex3Ds(table->m_left, table->m_top, 0.f);
   rgv3D[2] = Vertex3Ds(table->m_right, table->m_top, 0.f);
   rgv3D[1] = Vertex3Ds(table->m_right, table->m_bottom, 0.f);
   rgv3D[0] = Vertex3Ds(table->m_left, table->m_bottom, 0.f);
   Hit3DPoly * const ph3dpoly = new Hit3DPoly(table, rgv3D, 4); //!!
   ph3dpoly->SetFriction(table->m_overridePhysics ? table->m_fOverrideContactFriction : table->m_friction);
   ph3dpoly->m_elasticity = table->m_overridePhysics ? table->m_fOverrideElasticity : table->m_elasticity;
   ph3dpoly->m_elasticityFalloff = table->m_overridePhysics ? table->m_fOverrideElasticityFalloff : table->m_elasticityFalloff;
   ph3dpoly->m_scatter = ANGTORAD(table->m_overridePhysics ? table->m_fOverrideScatterAngle : table->m_scatter);
   AddCollider(ph3dpoly, false);
   */

   // playfield:
   m_hitPlayfield = HitPlane(table, Vertex3Ds(0, 0, 1), 0.f);
   m_hitPlayfield.SetFriction(table->m_overridePhysics ? table->m_fOverrideContactFriction : table->m_friction);
   m_hitPlayfield.m_elasticity = table->m_overridePhysics ? table->m_fOverrideElasticity : table->m_elasticity;
   m_hitPlayfield.m_elasticityFalloff = table->m_overridePhysics ? table->m_fOverrideElasticityFalloff : table->m_elasticityFalloff;
   m_hitPlayfield.m_scatter = ANGTORAD(table->m_overridePhysics ? table->m_fOverrideScatterAngle : table->m_scatter);

   // glass:
   Vertex3Ds glassNormal(0, table->m_bottom - table->m_top, table->m_glassBottomHeight - table->m_glassTopHeight);
   glassNormal.Normalize();
   m_hitTopGlass = HitPlane(table, Vertex3Ds(0, glassNormal.z, -glassNormal.y), -table->m_glassTopHeight);
   m_hitTopGlass.m_elasticity = 0.2f;
}

bool PhysicsEngine::RecordContact(const CollisionEvent& newColl)
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

// For the time being, there are 3 models available for nudge simulation (from keyboard or cabinet sensor).
// 1. Legacy nudge:
//    - Perform keyboard nudge by applying a force directly to the balls, first in the forward direction, 
//      then, after a little while, in the opposite direction
//    - No hardware nudging support
// 2. Acceleration based nudge:
//    - Perform keyboard nudge by applying an impulse to a physic model of the cabinet (spring-mass model, see below)
//    - Acquire cabinet acceleration from sensor and apply it directly to balls
// 3. Velocity based nudge:
//    - Perform keyboard nudge by applying an impulse to a physic model of the cabinet (spring-mass model, see below)
//    - Acquire cabinet velocity from sensor and apply it to the physic model of the cabinet (same spring-mass model as for keyboard nudge)
void PhysicsEngine::UpdateNudge(float dtime)
{
   // Since we are deriving forces/accelerations from velocities by doing a simple substract without scaling by delta time, we need dtime to be constant
   assert(fabs(dtime - ((double)PHYSICS_STEPTIME / (double)DEFAULT_STEPTIME)) < 1e-5);

   // Nudge acceleration is computed either from hardware accelerometer(s) or from nudge commands called from script.

   if (!m_legacyNudge)
   {
      // Perform keyboard nudge by simulating table movement modeled as a mass-spring-damper system
      //   u'' = -k u - c u'
      // with a spring constant k and a damping coefficient c
      const Vertex3Ds force = -m_nudgeSpring * m_tableDisplacement - m_nudgeDamping * m_tableVel;
      m_tableVel          += (float)PHYS_FACTOR * force;
      m_tableDisplacement += (float)PHYS_FACTOR * m_tableVel;

      m_tableAcceleration = (m_tableVel - m_tableVelOld) * (float)(1.0/PHYS_FACTOR);
      m_tableVelOld = m_tableVel;

      // Acquire from sensor input
      Vertex2D sensor = g_pplayer->m_pininput.GetNudge();

      // Simulate hardware nudge by getting the cabinet velocity and applying it to the table spring model
      if (g_pplayer->IsAccelInputAsVelocity())
      {
         // Compute acceleration from acquired table velocity and apply it as a force to the balls.
         // Apply the external accelerometer-based nudge velocity input (which is
         // a separate input from the traditional acceleration input)
         Vertex3Ds sensorTableVelocity(sensor.x, sensor.y, 0.f);
         m_tableAcceleration += (sensorTableVelocity - m_prevSensorTableVelocity) * (float)(1.0/PHYS_FACTOR);
         m_prevSensorTableVelocity = sensorTableVelocity;

         // No ball 'nudge' force directly applied to the ball, only force resulting from table acceleration
         m_nudgeAcceleration.SetZero();
      }
      else
      {
         // Simulate hardware nudge by getting the cabinet acceleration and applying it directly to the ball
         m_nudgeAcceleration.Set(sensor.x, sensor.y, 0.f);
      }
   }
   // legacy/VP9 style keyboard nudging, by directly applying a force to the balls
   else if (m_legacyNudgeTime != 0)
   {
      m_legacyNudgeTime--;
      if (m_legacyNudgeTime == 95)
      {
         m_nudgeAcceleration.x = -m_legacyNudgeBack.x * 2.0f;
         m_nudgeAcceleration.y = m_legacyNudgeBack.y * 2.0f;
      }
      else if (m_legacyNudgeTime == 90)
      {
         m_nudgeAcceleration.x = m_legacyNudgeBack.x;
         m_nudgeAcceleration.y = -m_legacyNudgeBack.y;
      }
      else
      {
         m_nudgeAcceleration.SetZero();
      }
   }

   // Apply our filter to the nudge data (meaningless for legacy and velocity based nudging ?)
   if (m_enableNudgeFilter)
   {
      m_nudgeFilterX.sample(m_nudgeAcceleration.x, m_curPhysicsFrameTime);
      m_nudgeFilterY.sample(m_nudgeAcceleration.y, m_curPhysicsFrameTime);
   }

   // Convert to force
   m_nudgeAcceleration.x *= (float)(1.0/PHYS_FACTOR);
   m_nudgeAcceleration.y *= (float)(1.0/PHYS_FACTOR);

   if (m_enablePlumbTilt && m_plumbTiltThreshold > 0.0f && dtime > 0.f && dtime <= 0.1f) // Ignore large time slices... forces would get crazy!
   {
      // Up to VPX 10.8, mechanical tilt was handled in cvpmNudge defined either in core.vbs or in a NudgePlugIn_xxx defined by the user (so in VBS, with a low and unstable integration period, using very crude model)
      // This new implementation replaces it by a physics based model, with the following differences:
      // - use full VPX 10 nudge (include table velocity) instead of partial one (would say that this is a bug of previous implementation)
      // - simulate a simplified pendulum with 3 (simplified) forces: gravity, nudge and pole, and some velocity dampening
      // - send mechanical tilt like the plumb on real machine, triggering rom tile (instead of a fake keyboard central nudge like in previous implementation. not sure why previous implementation did that: isn't the point of a plumb to actually tilt ?)
      // Consequently NudgePlugIn_xxx scripts were also removed, support VBS script were also adapted to handle nudging and tilting separately.
      // 
      // Simple Newton model:
      // . solid pole enforced by nullifying acceleration and velocity along pole axis, while keeping pole length constant
      // . gravity force, simply applied at pole tail
      // . table acceleration scaled and applied at pole tail (which is physically wrong but okish regarding gameplay result)
      // . magic velocity dampening
      const Vertex3Ds nudge = GetNudgeAcceleration();
      Vertex3Ds poleAxis(m_plumbPos);
      poleAxis.Normalize();
      Vertex3Ds gravity(0.f, 0.f, -9.81f);
      Vertex3Ds plumbAcc = gravity;
      plumbAcc += m_plumbMassFactor * nudge; // This is absolutely not physically correct
      plumbAcc -= plumbAcc.Dot(poleAxis) * poleAxis; // Keep acceleration orthogonal to pole
      m_plumbVel *= 0.999f;
      m_plumbVel += plumbAcc * (float)PHYSICS_STEPTIME_S;
      m_plumbVel -= m_plumbVel.Dot(poleAxis) * poleAxis; // Keep velocity orthogonal to pole
      m_plumbPos += m_plumbVel * (float)PHYSICS_STEPTIME_S;
      m_plumbPos *= m_plumbPoleLength / m_plumbPos.Length(); // Keep plumb at end of pole

      // Check if we hit the edge, using the pole angle (tilt threshold is 0..1000/1000 corresponding to 0..PI/4)
      const float psi = atan2f(sqrtf(m_plumbPos.x * m_plumbPos.x + m_plumbPos.y * m_plumbPos.y), -m_plumbPos.z);
      const float tiltAngle = (float)(M_PI * 0.25) * m_plumbTiltThreshold;
      const float tiltPerc = 100.0f * psi / tiltAngle;
      bool tilted = false;
      if (tiltPerc > 100.0f)
      {
         tilted = true;
         // Keep plumb inside tile limits
         const float limitAngle = tiltAngle - 1e-3f;
         m_plumbPos.z = -m_plumbPoleLength * cosf(limitAngle);
         const float xy = m_plumbPoleLength * sinf(limitAngle);
         const float theta = atan2f(m_plumbPos.x, m_plumbPos.y);
         const Vertex3Ds axis(sinf(theta), cosf(theta), 0.f);
         m_plumbPos.x = xy * axis.x;
         m_plumbPos.y = xy * axis.y;
         // Bounce the plumb (reflect velocity against circle normal, dampen it by a magic factor)
         m_plumbVel = 0.8f * (m_plumbVel - 2.f * m_plumbVel.Dot(axis) * axis);
      }

      // Fire event (same as keyboard tilt)
      if (m_plumbTiltHigh != tilted)
      {
         m_plumbTiltHigh = tilted;
         if (tilted)
            m_plumbTiltIndex++; 
         g_pplayer->m_pininput.FireActionEvent(eMechanicalTilt, m_plumbTiltHigh);
      }

      // Update player for diagnostic/table script visibility. Only update if input value is larger than what's there.
      // When the table script reads the values, they will reset to 0.
      if (tiltPerc > g_pplayer->m_ptable->m_tblNudgeReadTilt)
         g_pplayer->m_ptable->m_tblNudgeReadTilt = tiltPerc;
      if (fabsf(nudge.x) > fabsf(g_pplayer->m_ptable->m_tblNudgeRead.x))
         g_pplayer->m_ptable->m_tblNudgeRead.x = nudge.x;
      if (fabsf(nudge.y) > fabsf(g_pplayer->m_ptable->m_tblNudgeRead.y))
         g_pplayer->m_ptable->m_tblNudgeRead.y = nudge.y;
      if (fabsf(m_plumbPos.x) > fabsf(g_pplayer->m_ptable->m_tblNudgePlumb.x))
         g_pplayer->m_ptable->m_tblNudgePlumb.x = m_plumbPos.x;
      if (fabsf(m_plumbPos.y) > fabsf(g_pplayer->m_ptable->m_tblNudgePlumb.y))
         g_pplayer->m_ptable->m_tblNudgePlumb.y = m_plumbPos.y;
   }
}

void PhysicsEngine::ReadNudgeSettings(const Settings& settings)
{
   m_enablePlumbTilt = settings.LoadValueWithDefault(Settings::Player, "TiltSensCB"s, false);
   m_plumbMassFactor = settings.LoadValueWithDefault(Settings::Player, "TiltInertia"s, 100.f) * 0.05f;
   m_plumbTiltThreshold = (float)settings.LoadValueWithDefault(Settings::Player, "TiltSensitivity"s, 400) * (float)(1.0 / 1000.0);

   m_enableNudgeFilter = settings.LoadValueWithDefault(Settings::Player, "EnableNudgeFilter"s, false);

   m_legacyNudge = settings.LoadValueWithDefault(Settings::Player, "EnableLegacyNudge"s, false);
   m_legacyNudgeStrength = settings.LoadValueWithDefault(Settings::Player, "LegacyNudgeStrength"s, 1.f);
}

Vertex2D PhysicsEngine::GetScreenNudge() const
{
   // in table coordinates, +Y points down, but in screen coordinates, it points up, so we have to flip the y component
   if (m_legacyNudge)
      return {m_legacyNudgeBack.x * sqrf((float)m_legacyNudgeTime * 0.01f), -m_legacyNudgeBack.y * sqrf((float)m_legacyNudgeTime * 0.01f)};
   else
      return {m_tableDisplacement.x, -m_tableDisplacement.y};
}

AsyncDynamicQuadTree *PhysicsEngine::GetUIQuadTree()
{
   if (m_UIQuadTtree == nullptr)
      m_UIQuadTtree = new AsyncDynamicQuadTree(this, g_pplayer->m_ptable, true);
   return m_UIQuadTtree;
}

vector<HitObject *> PhysicsEngine::GetUIHitObjects(IEditable *editable)
{
   return GetUIQuadTree()->GetHitObjects(editable);
}

void PhysicsEngine::RayCast(const Vertex3Ds &source, const Vertex3Ds &target, const bool uiCast, vector<HitTestResult> &vhoHit)
{
   // Create a ray (ball) that travels in 3D space along the given ray, and find what it intersects with.
   HitBall ballT;
   ballT.m_d.m_pos = source;
   ballT.m_d.m_vel = target - source;
   ballT.m_d.m_radius = 0.01f;
   ballT.m_coll.m_hittime = 1.0f;
   ballT.CalcHitBBox(); // need to update here, as only done lazily

   if (uiCast)
   {
      GetUIQuadTree()->HitTestXRay(&ballT, vhoHit, ballT.m_coll);
   }
   else
   {
      m_hitoctree_dynamic.HitTestXRay(&ballT, vhoHit, ballT.m_coll);
      m_hitoctree.HitTestXRay(&ballT, vhoHit, ballT.m_coll);
   }

   // Sort result by distance from viewer
   std::ranges::sort(vhoHit.begin(), vhoHit.end(), [](const HitTestResult& a, const HitTestResult& b) { return a.m_time < b.m_time; });
}


void PhysicsEngine::ResetPerFrameStats()
{
   // Reset per frame debug counters
   m_phys_iterations = 0;

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
   m_startTime_usec = usec();
   m_curPhysicsFrameTime = m_startTime_usec;
   m_nextPhysicsFrameTime = m_curPhysicsFrameTime + PHYSICS_STEPTIME;

#ifdef PLAYBACK
   if (m_playback)
   {
      float physicsStepTime;
      ParseLog((LARGE_INTEGER*)&physicsStepTime, (LARGE_INTEGER*)&m_startTime_usec);
   }
#endif

#ifdef LOG
   PLOGD.printf("Step Time %llu", m_startTime_usec);
   PLOGD.printf("End Frame");
#endif
}

void PhysicsEngine::UpdatePhysics()
{
   if (!g_pplayer) //!! meh, we have a race condition somewhere where we delete g_pplayer while still in use (e.g. if we have a script compile error and cancel the table start)
      return;

   g_pplayer->m_logicProfiler.EnterProfileSection(FrameProfiler::PROFILE_PHYSICS);
   uint64_t initial_time_usec = usec();

   // DJRobX's crazy latency-reduction code
   uint64_t delta_frame = 0;
   if (g_pplayer->m_minphyslooptime > 0 && m_lastFlipTime > 0)
   {
      // We want the physics loops to sync up to the frames, not
      // the post-render period, as that can cause some judder.
      delta_frame = initial_time_usec - m_lastFlipTime;
      initial_time_usec -= delta_frame;
   }

   // When paused or after debugging, shift whole game forward in time
   // TODO not sure why we would need noTimeCorrect, as pause should already have shifted the timings
   if (!g_pplayer->IsPlaying() || g_pplayer->m_noTimeCorrect)
   {
      const uint64_t curPhysicsFrameTime = m_startTime_usec + (uint64_t)(g_pplayer->m_time_sec * 1000000.0);
      const uint64_t timeShift = initial_time_usec - curPhysicsFrameTime;
      m_startTime_usec += timeShift;
      m_nextPhysicsFrameTime += timeShift;
      m_curPhysicsFrameTime += timeShift;
      g_pplayer->m_noTimeCorrect = false;
   }

   // Walk a single physics step forward
   if (g_pplayer->m_step)
   {
      m_curPhysicsFrameTime -= PHYSICS_STEPTIME;
      g_pplayer->m_step = false;
   }

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

   if (m_nextPhysicsFrameTime < initial_time_usec)
      VPXPluginAPIImpl::GetInstance().BroadcastVPXMsg(m_onUpdatePhysicsMsgId, nullptr);

   while (m_nextPhysicsFrameTime < initial_time_usec) // loop here until physics (=simulated) time catches up to current real time, still staying behind real time by up to one physics emulation step
   {
      g_pplayer->m_time_sec = max(g_pplayer->m_time_sec, (double)(m_curPhysicsFrameTime - m_startTime_usec) / 1000000.0); // First iteration is done before precise time
      g_pplayer->m_time_msec = (uint32_t)((m_curPhysicsFrameTime - m_startTime_usec) / 1000); // Get time in milliseconds for timers

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
      // FIXME the initial idea of this implementation is somewhat defeated by the fact that in single threaded mode, the main thread is mostly stalled waiting for GPU (solved in multithreaded mode) => remove ?
      #if !defined(ENABLE_BGFX)
      if (g_pplayer->m_minphyslooptime > 0)
      {
         const uint64_t basetime = usec();
         const uint64_t targettime = ((uint64_t)g_pplayer->m_minphyslooptime * m_phys_iterations) + m_lastFlipTime;
         // If we're 3/4 of the way through the loop, fire a "controller sync" timer (timers with an interval set to -2) event so VPM can react to input.
         if (m_phys_iterations == 750 / ((int)g_pplayer->m_fps + 1))
            g_pplayer->FireSyncController();
         if (basetime < targettime)
         {
            g_pplayer->m_renderProfiler->EnterProfileSection(FrameProfiler::PROFILE_SLEEP);
            uSleep(targettime - basetime);
            g_pplayer->m_renderProfiler->ExitProfileSection();
         }
      }
      #endif
      // end DJRobX's crazy code

      const uint64_t cur_time_usec = usec()-delta_frame; //!! one could also do this directly in the while loop condition instead (so that the while loop will really match with the current time), but that leads to some stuttering on some heavy frames

      // hung in the physics loop over 200 milliseconds or the number of physics iterations to catch up on is high (i.e. very low/unplayable FPS)
      if ((cur_time_usec - initial_time_usec > 200000) || (m_physicsMaxLoops != 0 && m_phys_iterations > m_physicsMaxLoops))
      {                                                             // can not keep up to real time
         m_curPhysicsFrameTime  = initial_time_usec;                // skip physics forward ... slip-cycles -> 'slowed' down physics
         m_nextPhysicsFrameTime = initial_time_usec + PHYSICS_STEPTIME;
         break;                                                     // go draw frame
      }

      //update keys, hid, plumb, nudge, timers, etc
      //const uint32_t sim_msec = (uint32_t)(m_curPhysicsFrameTime / 1000);
      const uint32_t cur_time_msec = (uint32_t)(cur_time_usec / 1000);

      #if !defined(ENABLE_BGFX)
      // FIXME remove ? To be done correctly, we should process OS messages and sync back controller
      g_pplayer->m_pininput.ProcessInput();
      #endif

      // FIXME remove or at least move legacy ushock to a plugin
      ushock_output_update(/*sim_msec*/cur_time_msec);

      #ifdef ACCURATETIMERS
      g_pplayer->ApplyDeferredTimerChanges();
      #if !defined(ENABLE_BGFX)
      if (g_pplayer->m_videoSyncMode == VideoSyncMode::VSM_FRAME_PACING || g_pplayer->m_logicProfiler.Get(FrameProfiler::PROFILE_SCRIPT) <= 1000 * MAX_TIMERS_MSEC_OVERALL) // if overall script time per frame exceeded, skip
      #endif
         g_pplayer->FireTimers(g_pplayer->m_time_msec);
      #endif

      g_pplayer->MechPlungerUpdate(); // integral physics frame. So the previous graphics frame was (1.0 - physics_diff_time) before 
      // this integral physics frame. Accelerations and inputs are always physics frame aligned

      UpdateNudge(physics_diff_time);

      for (size_t i = 0; i < m_vmover.size(); i++)
         m_vmover[i]->UpdateVelocities();      // always on integral physics frame boundary (spinner, gate, flipper, plunger, ball)

      //primary physics loop
      PhysicsSimulateCycle(physics_diff_time); // main simulator call

      // Store new position of balls for ball trails
      for (const auto hitBall : g_pplayer->m_vball)
         hitBall->OnPhysicStepProcessed(g_pplayer->m_time_msec);

      //PLOGD << "PT: " << physics_diff_time << ' ' << physics_to_graphic_diff_time << ' ' << (uint32_t)(m_curPhysicsFrameTime/1000) << ' ' << (uint32_t)(initial_time_usec/1000) << ' ' << cur_time_msec;

      m_curPhysicsFrameTime = m_nextPhysicsFrameTime; // new cycle, on physics frame boundary
      m_nextPhysicsFrameTime += PHYSICS_STEPTIME;     // advance physics position
   } // end while (m_curPhysicsFrameTime < initial_time_usec)

   assert(m_curPhysicsFrameTime < m_nextPhysicsFrameTime);
   assert(m_curPhysicsFrameTime <= initial_time_usec);
   assert(initial_time_usec <= m_nextPhysicsFrameTime);
   assert(g_pplayer->m_time_sec <= (double)(initial_time_usec - m_startTime_usec) / 1000000.0);

   // The physics is emulated by PHYSICS_STEPTIME, but the overall emulation time is more precise
   g_pplayer->m_time_sec = (double)(initial_time_usec - m_startTime_usec) / 1000000.0;
   //g_pplayer->m_time_sec = (double)(max(initial_time_usec, m_curPhysicsFrameTime) - m_startTime_usec) / 1000000.0;
   // g_pplayer->m_time_msec = (uint32_t)((max(initial_time_usec, m_curPhysicsFrameTime) - m_startTime_usec) / 1000); // Not needed since PHYSICS_STEPTIME happens to be 1ms

   g_pplayer->m_logicProfiler.ExitProfileSection();
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
      float hittime = dtime; // begin time search from now ...  until delta ends

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
         HitBall *const pball = g_pplayer->m_vball[i];

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
         HitBall *const pball = g_pplayer->m_vball[i];

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
            pball->m_coll.m_obj = nullptr;               // remove trial hit object pointer

            // Collide may have changed the velocity of the ball, 
            // and therefore the bounding box for the next hit cycle
            if (i >= g_pplayer->m_vball.size() || g_pplayer->m_vball[i] != pball) // Ball still exists? may have been deleted from list
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
                        if (--pball->m_dynamic <= 0)            //... ball static, cancels next gravity increment
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
         c_contactcnt = (uint32_t)m_contacts.size();
      #endif

      // Now handle contacts.

      // At this point UpdateDisplacements() was already called, so the state is different
      // from that at HitTest(). However, contacts have zero relative velocity, so
      // hopefully nothing catastrophic has happened in the meanwhile.

      // Maybe a two-phase setup where we first process only contacts, then only collisions
      // could also work.
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
         for (HitBall *const pball : g_pplayer->m_vball)
         {
            if (pball->m_coll.m_hitdistance >= (float)PHYS_TOUCH)
               continue;

            Vertex3Ds oldPos0 = pball->GetOldPosition(g_pplayer->m_time_msec - 90); // Position 90ms ago
            if (oldPos0.x == FLT_MAX)
               continue;

            Vertex3Ds oldPos1 = pball->GetOldPosition(g_pplayer->m_time_msec - 80); // Position 80ms ago
            if (oldPos1.x == FLT_MAX)
               continue;

            /*
            const float mag = pball->m_vel.x*pball->m_vel.x + pball->m_vel.y*pball->m_vel.y; // values below are copy pasted from above
            if (pball->m_drsq < 8.0e-5f
                  && mag < 1.0e-3f*m_gravity*m_gravity / GRAVITYCONST / GRAVITYCONST
                  && fabsf(pball->m_vel.z) < 0.2f*m_gravity / GRAVITYCONST
                  && pball->m_angularmomentum.Length() < 0.9f*m_gravity / GRAVITYCONST)
                  //&& rand_mt_01() < 0.95f)
            {
               pball->m_angularmomentum *= 0.05f; // do not kill spin completely, otherwise stuck balls will happen during regular gameplay
            }
            */

            const Vertex3Ds diff_pos = oldPos0 - pball->m_d.m_pos;
            const float mag = diff_pos.x * diff_pos.x + diff_pos.y * diff_pos.y; // squared distance in the playfield plane, since 90ms
            const Vertex3Ds diff_pos2 = oldPos1 - pball->m_d.m_pos;
            const float mag2 = diff_pos2.x * diff_pos2.x + diff_pos2.y * diff_pos2.y; // squared distance in the playfield plane, since 80ms

            const float threshold = (pball->m_angularmomentum.x*pball->m_angularmomentum.x + pball->m_angularmomentum.y*pball->m_angularmomentum.y) / max(mag, mag2);

            if (!infNaN(threshold) && threshold > 666.f)
            {
               const float damp = clamp(1.0f - (threshold - 666.f) / 10000.f, 0.23f, 1.f); // do not kill spin completely, otherwise stuck balls will happen during regular gameplay
               pball->m_angularmomentum *= damp;
            }
         }
      #endif

      dtime -= hittime; //new delta .. i.e. time remaining

      m_swap_ball_collision_handling = !m_swap_ball_collision_handling; // swap order of ball-ball collisions

   } // end physics loop
}

string PhysicsEngine::GetPerfInfo(bool resetMax)
{
   if (resetMax || g_pplayer->m_logicProfiler.GetPrev(FrameProfiler::PROFILE_PHYSICS) > m_phys_max)
      m_phys_max = g_pplayer->m_logicProfiler.GetPrev(FrameProfiler::PROFILE_PHYSICS);

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

   info << "Physics: " << m_phys_iterations << " iterations per frame (" << ((uint32_t)(m_phys_total_iterations / m_count)) << " avg " << m_phys_max_iterations
        << " max)\n";
#ifdef DEBUGPHYSICS
   info << std::setprecision(5);
   info << "Hits:" << c_hitcnts << " Collide:" << c_collisioncnt << " Ctacs:" << c_contactcnt;
#ifdef C_DYNAMIC
   info << " Static:" << c_staticcnt;
#endif
   info << " Embed:" << c_embedcnts << " TimeSearch:" << c_timesearch << '\n';
   info << " kDObjects:" << m_hitoctree_dynamic.GetObjectCount() << " kD:" << m_hitoctree_dynamic.GetNLevels() << '\n'
        << " QuadObjects:" << m_hitoctree.GetObjectCount() << " Quadtree:" << m_hitoctree.GetNLevels() << '\n'
        << " Traversed:" << c_traversed << " Tested:" << c_tested << " DeepTested:" << c_deepTested << '\n';
   info << std::setprecision(1);
#endif

   return info.str();
}
