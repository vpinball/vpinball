#include "stdafx.h"

unsigned int Ball::ballID = 0;

Ball::Ball()
{
   m_id = ballID;
   ballID++;

   m_coll.m_ball = this;   // TODO: this needs to move somewhere else
   m_coll.m_obj = NULL;
   m_pballex = NULL;
   m_d.m_vpVolObjs = NULL; // should be NULL ... only real balls have this value
   m_pinballEnv = NULL;
   m_pinballDecal = NULL;
   m_lastEventPos.x = m_lastEventPos.y = m_lastEventPos.z = -10000.0f; // last pos is far far away
   m_d.m_frozen = false;
   m_color = RGB(255, 255, 255);
#ifdef C_DYNAMIC
   m_dynamic = C_DYNAMIC; // assume dynamic
   m_drsq = 0.0f;
#endif
   m_d.m_vel.SetZero();
   m_angularmomentum.SetZero();
   m_d.m_mass = 1.0f;
   m_d.m_radius = 25.0f;
   m_orientation.Identity();
   m_bulb_intensity_scale = 1.0f;
   m_playfieldReflectionStrength = 1.0f;
   m_reflectionEnabled = true;
   m_forceReflection = false;
   m_visible = true;
   m_decalMode = g_pplayer ? g_pplayer->m_ptable->m_BallDecalMode : false;

   memset(m_szImage, 0, sizeof(m_szImage));
   memset(m_szImageDecal, 0, sizeof(m_szImageDecal));

   m_ringcounter_oldpos = 0;
   for (int i = 0; i < MAX_BALL_TRAIL_POS; ++i)
      m_oldpos[i].x = FLT_MAX;
}

void Ball::RenderSetup()
{
}

void Ball::Init(const float mass)
{
   // Only called by real balls, not temporary objects created for physics/rendering
   m_d.m_mass = mass;

   m_orientation.Identity();
   m_angularmomentum.SetZero();

   m_mover.m_pball = this;

   m_d.m_frozen = false;

   m_playfieldReflectionStrength = 1.0f;
   m_reflectionEnabled = true;
   m_forceReflection = false;
   m_visible = true;
   m_decalMode = g_pplayer->m_ptable->m_BallDecalMode;

   m_coll.m_obj = NULL;
#ifdef C_DYNAMIC
   m_dynamic = C_DYNAMIC; // assume dynamic
#endif

   m_pballex = NULL;

   m_d.m_vpVolObjs = new vector<IFireEvents*>;

   m_color = RGB(255, 255, 255);

   // override table ball image with global ball image?
   if (g_pplayer->m_overwriteBallImages && g_pplayer->m_ballImage)
       m_pinballEnv = g_pplayer->m_ballImage;
   else
   {
       if (g_pplayer->m_ptable->m_szBallImage[0] == '\0')
       {
           m_szImage[0] = '\0';
           m_pinballEnv = NULL;
       }
       else
       {
           lstrcpy(m_szImage, g_pplayer->m_ptable->m_szBallImage);
           m_pinballEnv = g_pplayer->m_ptable->GetImage(m_szImage);
       }
   }

   // override table ball logo/decal image with global ball logo/decal image?
   if (g_pplayer->m_overwriteBallImages && g_pplayer->m_decalImage)
       m_pinballDecal = g_pplayer->m_decalImage;
   else
   {
       if (g_pplayer->m_ptable->m_szBallImageDecal[0] == '\0')
       {
           m_szImageDecal[0] = '\0';
           m_pinballDecal = NULL;
       }
       else
       {
           lstrcpy(m_szImageDecal, g_pplayer->m_ptable->m_szBallImageDecal);
           m_pinballDecal = g_pplayer->m_ptable->GetImage(m_szImageDecal);
       }
   }

   m_bulb_intensity_scale = g_pplayer->m_ptable->m_defaultBulbIntensityScaleOnBall;

   RenderSetup();
}


void Ball::EnsureOMObject()
{
   if (m_pballex)
      return;

   CComObject<BallEx>::CreateInstance(&m_pballex);
   m_pballex->AddRef();

   m_pballex->m_pball = this;
}

//
// license:GPLv3+
// Ported at: VisualPinball.Unity/VisualPinball.Unity/VPT/Ball/BallCollider.cs
//

void Ball::Collide3DWall(const Vertex3Ds& hitNormal, float elasticity, const float elastFalloff, const float friction, float scatter_angle)
{
   //speed normal to wall
   float dot = m_d.m_vel.Dot(hitNormal);

   if (dot >= -C_LOWNORMVEL)							// nearly receding ... make sure of conditions
   {													// otherwise if clearly approaching .. process the collision
      if (dot > C_LOWNORMVEL) return;					// is this velocity clearly receding (i.e must > a minimum)
#ifdef C_EMBEDDED
      if (m_coll.m_hitdistance < -C_EMBEDDED)
         dot = -C_EMBEDSHOT;							// has ball become embedded???, give it a kick
      else return;
#endif
   }

#ifdef C_DISP_GAIN 
   // correct displacements, mostly from low velocity, alternative to acceleration processing
   float hdist = -C_DISP_GAIN * m_coll.m_hitdistance; // limit delta noise crossing ramps,
   if (hdist > 1.0e-4f)                               // when hit detection checked it what was the displacement
   {
      if (hdist > C_DISP_LIMIT)
         hdist = C_DISP_LIMIT; // crossing ramps, delta noise
      m_d.m_pos += hdist * hitNormal; // push along norm, back to free area
      // use the norm, but this is not correct, reverse time is correct
   }
#endif

   // magnitude of the impulse which is just sufficient to keep the ball from
   // penetrating the wall (needed for friction computations)
   const float reactionImpulse = m_d.m_mass * fabsf(dot);

   elasticity = ElasticityWithFalloff(elasticity, elastFalloff, dot);
   dot *= -(1.0f + elasticity);
   m_d.m_vel += dot * hitNormal;     // apply collision impulse (along normal, so no torque)

   // compute friction impulse

   const Vertex3Ds surfP = -m_d.m_radius * hitNormal;    // surface contact point relative to center of mass

   const Vertex3Ds surfVel = SurfaceVelocity(surfP);     // velocity at impact point

   Vertex3Ds tangent = surfVel - surfVel.Dot(hitNormal) * hitNormal; // calc the tangential velocity

   const float tangentSpSq = tangent.LengthSquared();
   if (tangentSpSq > 1e-6f)
   {
      tangent /= sqrtf(tangentSpSq);           // normalize to get tangent direction
      const float vt = surfVel.Dot(tangent);   // get speed in tangential direction

      // compute friction impulse
      const Vertex3Ds cross = CrossProduct(surfP, tangent);
      const float kt = 1.0f/m_d.m_mass + tangent.Dot(CrossProduct(cross / Inertia(), surfP));

      // friction impulse can't be greater than coefficient of friction times collision impulse (Coulomb friction cone)
      const float maxFric = friction * reactionImpulse;
      const float jt = clamp(-vt / kt, -maxFric, maxFric);

      if (!infNaN(jt))
         ApplySurfaceImpulse(jt * cross, jt * tangent);
   }

   if (scatter_angle < 0.0f) scatter_angle = c_hardScatter;  // if < 0 use global value
   scatter_angle *= g_pplayer->m_ptable->m_globalDifficulty; // apply difficulty weighting

   if (dot > 1.0f && scatter_angle > 1.0e-5f) //no scatter at low velocity
   {
      float scatter = rand_mt_m11();      // -1.0f..1.0f
      scatter *= (1.0f - scatter*scatter)*2.59808f * scatter_angle;	// shape quadratic distribution and scale
      const float radsin = sinf(scatter); // Green's transform matrix... rotate angle delta
      const float radcos = cosf(scatter); // rotational transform from current position to position at time t
      const float vxt = m_d.m_vel.x;
      const float vyt = m_d.m_vel.y;
      m_d.m_vel.x = vxt *radcos - vyt *radsin;// rotate to random scatter angle
      m_d.m_vel.y = vyt *radcos + vxt *radsin;
   }
}

float Ball::HitTest(const BallS& ball, const float dtime, CollisionEvent& coll) const
{
   Vertex3Ds d = m_d.m_pos - ball.m_pos;  // delta position

   Vertex3Ds dv = m_d.m_vel - ball.m_vel; // delta velocity

   float bcddsq = d.LengthSquared();    // square of ball center's delta distance
   float bcdd = sqrtf(bcddsq);          // length of delta

#if 0 //!! TODO was this really necessary, must this be re-activated somehow??
   if (bcdd < 1.0e-8f)                  // two balls center-over-center embedded
   { //return -1;
      d.z = -1.0f;                      // patch up
      ball.m_pos.z -= d.z;              // lift up

      bcdd = 1.0f;                      // patch up
      bcddsq = 1.0f;                    // patch up
      dv.z = 0.1f;                      // small speed difference
      ball.m_vel.z -= dv.z;
   }
#endif

   const float b = dv.Dot(d);              // inner product
   const float bnv = b / bcdd;             // normal speed of balls toward each other

   if (bnv > C_LOWNORMVEL) return -1.0f;   // dot of delta velocity and delta displacement, positive if receding no collison

   const float totalradius = ball.m_radius + m_d.m_radius;
   const float bnd = bcdd - totalradius;   // distance between ball surfaces

   float hittime;
#ifdef BALL_CONTACTS //!! leads to trouble currently, might be due to missing contact handling for -both- balls?!
   bool isContact = false;
#endif
   if (bnd <= (float)PHYS_TOUCH)           // in contact??? 
   {
      if (bnd < ball.m_radius*-2.0f)
         return -1.0f;                     // embedded too deep?

      if ((fabsf(bnv) > C_CONTACTVEL)      // >fast velocity, return zero time
         //zero time for rigid fast bodies
         || (bnd <= (float)(-PHYS_TOUCH)))
         hittime = 0;                      // slow moving but embedded
      else
         hittime = bnd
#ifdef NEW_PHYSICS
          / -bnv;
#else
          *(float)(1.0/(2.0*PHYS_TOUCH)) + 0.5f; // don't compete for fast zero time events
#endif

#ifdef BALL_CONTACTS
      if (fabsf(bnv) <= C_CONTACTVEL)
         isContact = true;
#endif
   }
   else
   {
      // find collision time as solution of quadratic equation
      //   at^2 + bt + c = 0
      //	(length(m_vel - ball.m_vel)*t) ^ 2 + ((m_vel - ball.m_vel).(m_pos - ball.m_pos)) * 2 * t = totalradius*totalradius - length(m_pos - ball.m_pos)^2

      const float a = dv.LengthSquared(); // square of differential velocity

      if (a < 1.0e-8f)
         return -1.0f; // ball moving really slow, then wait for contact

      float time1, time2;
      if (!SolveQuadraticEq(a, 2.0f*b, bcddsq - totalradius*totalradius, time1, time2))
         return -1.0f;

      hittime = (time1*time2 < 0.f) ? std::max(time1, time2) : std::min(time1, time2); // find smallest nonnegative solution
   }

   if (infNaN(hittime) || hittime < 0.f || hittime > dtime)
	   return -1.0f; // .. was some time previous || beyond the next physics tick

   const Vertex3Ds hitPos = ball.m_pos + hittime * dv; // new ball position

   //calc unit normal of collision
   const Vertex3Ds hitnormal = hitPos - m_d.m_pos;
   if (fabsf(hitnormal.x) <= FLT_MIN && fabsf(hitnormal.y) <= FLT_MIN && fabsf(hitnormal.z) <= FLT_MIN)
      return -1.f;

   coll.m_hitnormal = hitnormal;
   coll.m_hitnormal.Normalize();

   coll.m_hitdistance = bnd;			// actual contact distance
   //coll.m_hitRigid = true;			// rigid collision type

#ifdef BALL_CONTACTS
   coll.m_isContact = isContact;
   if (isContact)
      coll.m_hit_org_normalvelocity = bnv;
#endif

   return hittime;
}

void Ball::Collide(const CollisionEvent& coll)
{
   Ball * const pball = coll.m_ball;

   // make sure we process each ball/ball collision only once
   // (but if we are frozen, there won't be a second collision event, so deal with it now!)
   if (((g_pplayer->m_swap_ball_collision_handling && pball >= this) ||
      (!g_pplayer->m_swap_ball_collision_handling && pball <= this)) &&
      !m_d.m_frozen)
      return;

   // target ball to object ball delta velocity
   const Vertex3Ds vrel = pball->m_d.m_vel - m_d.m_vel;
   const Vertex3Ds vnormal = coll.m_hitnormal;
   float dot = vrel.Dot(vnormal);

   // correct displacements, mostly from low velocity, alternative to true acceleration processing
   if (dot >= -C_LOWNORMVEL)								// nearly receding ... make sure of conditions
   {														// otherwise if clearly approaching .. process the collision
      if (dot > C_LOWNORMVEL) return;						// is this velocity clearly receding (i.e must > a minimum)		
#ifdef C_EMBEDDED
      if (coll.m_hitdistance < -C_EMBEDDED)
         dot = -C_EMBEDSHOT;		// has ball become embedded???, give it a kick
      else return;
#endif
   }

   // send ball/ball collision event to script function
   if (dot < -0.25f)    // only collisions with at least some small true impact velocity (no contacts)
      g_pplayer->m_ptable->InvokeBallBallCollisionCallback(this, pball, -dot);

#ifdef C_DISP_GAIN
   float edist = -C_DISP_GAIN * coll.m_hitdistance;
   if (edist > 1.0e-4f)
   {
      if (edist > C_DISP_LIMIT)
         edist = C_DISP_LIMIT;		// crossing ramps, delta noise
      if (!m_d.m_frozen) edist *= 0.5f;	// if the hitten ball is not frozen
      pball->m_d.m_pos += edist * vnormal;// push along norm, back to free area
      // use the norm, but is not correct, but cheaply handled
   }

   edist = -C_DISP_GAIN * m_coll.m_hitdistance;	// noisy value .... needs investigation
   if (!m_d.m_frozen && edist > 1.0e-4f)
   {
      if (edist > C_DISP_LIMIT)
         edist = C_DISP_LIMIT; // crossing ramps, delta noise
      edist *= 0.5f;
      m_d.m_pos -= edist * vnormal; // pull along norm, back to free area
   }
#endif

   const float myInvMass = m_d.m_frozen ? 0.0f : 1.0f/m_d.m_mass; // frozen ball has infinite mass
   const float pballInvMass = 1.0f/pball->m_d.m_mass; //!! do same frozen mass thing for that one?
   const float impulse = -(float)(1.0 + 0.8) * dot / (myInvMass + pballInvMass); // resitution = 0.8

   if (!m_d.m_frozen)
   {
      m_d.m_vel -= (impulse * myInvMass) * vnormal;
#ifdef C_DYNAMIC
      m_dynamic = C_DYNAMIC;
#endif
   }
   pball->m_d.m_vel += (impulse * pballInvMass) * vnormal;
#ifdef C_DYNAMIC
   pball->m_dynamic = C_DYNAMIC;
#endif
}

void Ball::HandleStaticContact(const CollisionEvent& coll, const float friction, const float dtime)
{
   const float normVel = m_d.m_vel.Dot(coll.m_hitnormal);   // this should be zero, but only up to +/- C_CONTACTVEL

   // If some collision has changed the ball's velocity, we may not have to do anything.
   if (normVel <= C_CONTACTVEL)
   {
      const Vertex3Ds fe = m_d.m_mass * g_pplayer->m_gravity;      // external forces (only gravity for now)
      const float dot = fe.Dot(coll.m_hitnormal);
      const float normalForce = std::max(0.0f, -(dot*dtime + coll.m_hit_org_normalvelocity)); // normal force is always nonnegative

      // Add just enough to kill original normal velocity and counteract the external forces.
      m_d.m_vel += normalForce * coll.m_hitnormal;

#ifdef C_EMBEDVELLIMIT
      if (coll.m_hitdistance <= (float)PHYS_TOUCH)
          m_d.m_vel += coll.m_hitnormal*max(min(C_EMBEDVELLIMIT,-coll.m_hitdistance),(float)PHYS_TOUCH);
#endif

#ifdef C_BALL_SPIN_HACK2 // hacky killing of ball spin
      float vell = m_vel.Length();
      if (m_vel.Length() < 1.f) //!! 1.f=magic, also see below
      {
         vell = (1.f-vell)*(float)C_BALL_SPIN_HACK2;
         const float damp = (1.0f - friction * clamp(-coll.m_hit_org_normalvelocity / C_CONTACTVEL, 0.0f,1.0f)) * vell + (1.0f-vell); // do not kill spin completely, otherwise stuck balls will happen during regular gameplay
         m_angularmomentum *= damp;
      }
#endif

      ApplyFriction(coll.m_hitnormal, dtime, friction);
   }
}

void Ball::ApplyFriction(const Vertex3Ds& hitnormal, const float dtime, const float fricCoeff)
{
   const Vertex3Ds surfP = -m_d.m_radius * hitnormal;    // surface contact point relative to center of mass

   const Vertex3Ds surfVel = SurfaceVelocity(surfP);
   const Vertex3Ds slip = surfVel - surfVel.Dot(hitnormal) * hitnormal;       // calc the tangential slip velocity

   const float maxFric = fricCoeff * m_d.m_mass * -g_pplayer->m_gravity.Dot(hitnormal);

   const float slipspeed = slip.Length();
   Vertex3Ds slipDir;
   float numer;
   //slintf("Velocity: %.2f Angular velocity: %.2f Surface velocity: %.2f Slippage: %.2f\n", m_vel.Length(), (m_angularmomentum / Inertia()).Length(), surfVel.Length(), slipspeed);
   //if (slipspeed > 1e-6f)

#ifdef C_BALL_SPIN_HACK
   const float normVel = m_d.m_vel.Dot(hitnormal);
   if ((normVel <= 0.025f) || // check for <=0.025 originated from ball<->rubber collisions pushing the ball upwards, but this is still not enough, some could even use <=0.2
#else
   if (
#endif
       (slipspeed < C_PRECISION))
   {
      // slip speed zero - static friction case

      const Vertex3Ds surfAcc = SurfaceAcceleration(surfP);
      const Vertex3Ds slipAcc = surfAcc - surfAcc.Dot(hitnormal) * hitnormal; // calc the tangential slip acceleration

      // neither slip velocity nor slip acceleration? nothing to do here
      if (slipAcc.LengthSquared() < 1e-6f)
         return;

      slipDir = slipAcc;
      slipDir.Normalize();

      numer = -slipDir.Dot(surfAcc);
   }
   else
   {
      // nonzero slip speed - dynamic friction case
      slipDir = slip / slipspeed;

      numer = -slipDir.Dot(surfVel);
   }

   const Vertex3Ds cp = CrossProduct(surfP, slipDir);
   const float denom = 1.0f/m_d.m_mass + slipDir.Dot(CrossProduct(cp / Inertia(), surfP));
   const float fric = clamp(numer / denom, -maxFric, maxFric);

   if (!infNaN(fric))
      ApplySurfaceImpulse((dtime * fric) * cp, (dtime * fric) * slipDir);
}

Vertex3Ds Ball::SurfaceVelocity(const Vertex3Ds& surfP) const
{
   return m_d.m_vel + CrossProduct(m_angularmomentum / Inertia(), surfP); // linear velocity plus tangential velocity due to rotation
}

Vertex3Ds Ball::SurfaceAcceleration(const Vertex3Ds& surfP) const
{
   const Vertex3Ds angularvelocity = m_angularmomentum / Inertia();
   // if we had any external torque, we would have to add "(deriv. of ang.vel.) x surfP" here
   return g_pplayer->m_gravity/m_d.m_mass    // linear acceleration
      + CrossProduct(angularvelocity, CrossProduct(angularvelocity, surfP)); // centripetal acceleration
}

void Ball::ApplySurfaceImpulse(const Vertex3Ds& rotI, const Vertex3Ds& impulse)
{
   m_d.m_vel += impulse/m_d.m_mass;

   m_angularmomentum += rotI;
   //const float aml = m_angularmomentum.Length();
   //if (aml > Inertia()*135.0f) //!! hack to limit ball spin
   //   m_angularmomentum *= (Inertia()*135.0f) / aml;
}

void Ball::CalcHitBBox()
{
   /* this would be okay if travelling only in vel direction, but the ball could also be reflected by something
   m_hitBBox.left   = min(m_pos.x, m_pos.x + m_vel.x) - (m_radius + 0.1f);
   m_hitBBox.right  = max(m_pos.x, m_pos.x + m_vel.x) + (m_radius + 0.1f);
   m_hitBBox.top    = min(m_pos.y, m_pos.y + m_vel.y) - (m_radius + 0.1f);
   m_hitBBox.bottom = max(m_pos.y, m_pos.y + m_vel.y) + (m_radius + 0.1f);
   m_hitBBox.zlow   = min(m_pos.z, m_pos.z + m_vel.z) - (m_radius + 0.1f);
   m_hitBBox.zhigh  = max(m_pos.z, m_pos.z + m_vel.z) + (m_radius + 0.1f);
   */

   const float vl = m_d.m_vel.Length() + m_d.m_radius + 0.05f; //!! 0.05f = paranoia
   m_hitBBox.left   = m_d.m_pos.x - vl;
   m_hitBBox.right  = m_d.m_pos.x + vl;
   m_hitBBox.top    = m_d.m_pos.y - vl;
   m_hitBBox.bottom = m_d.m_pos.y + vl;
   m_hitBBox.zlow   = m_d.m_pos.z - vl;
   m_hitBBox.zhigh  = m_d.m_pos.z + vl;
}

void BallMoverObject::UpdateDisplacements(const float dtime)
{
   m_pball->UpdateDisplacements(dtime);
}

void Ball::UpdateDisplacements(const float dtime)
{
   if (!m_d.m_frozen)
   {
      const Vertex3Ds ds = dtime * m_d.m_vel;
      m_d.m_pos += ds;

#ifdef C_DYNAMIC
      m_drsq = ds.LengthSquared(); // used to determine if static ball
#endif

      Matrix3 mat3;
      mat3.SkewSymmetric(m_angularmomentum / Inertia());

      Matrix3 addedorientation;
      addedorientation.MulMatricesAndMulScalar(mat3, m_orientation, dtime);

      m_orientation.AddMatrix(addedorientation);
      m_orientation.OrthoNormalize();
   }
}

void BallMoverObject::UpdateVelocities()
{
   m_pball->UpdateVelocities();
}

void Ball::UpdateVelocities()
{
   if (!m_d.m_frozen)  // Gravity
   {
      if (g_pplayer->m_ballControl && this == g_pplayer->m_pactiveballBC && g_pplayer->m_pBCTarget != NULL)
      {
         m_d.m_vel.x *= 0.5f;  // Null out most of the X/Y velocity, want a little bit so the ball can sort of find its way out of obstacles.
         m_d.m_vel.y *= 0.5f;

         m_d.m_vel += Vertex3Ds(max(-10.0f, min(10.0f, (g_pplayer->m_pBCTarget->x - m_d.m_pos.x) * (float)(1./10.))),
                                max(-10.0f, min(10.0f, (g_pplayer->m_pBCTarget->y - m_d.m_pos.y) * (float)(1./10.))),
                                -2.0f);
      }
      else
         m_d.m_vel += (float)PHYS_FACTOR * g_pplayer->m_gravity;

      m_d.m_vel.x += g_pplayer->m_Nudge.x; // TODO: depends on STEPTIME
      m_d.m_vel.y += g_pplayer->m_Nudge.y;

      m_d.m_vel -= g_pplayer->m_tableVelDelta;
   }

#ifdef C_DYNAMIC
   m_dynamic = C_DYNAMIC; // always set .. after adding velocity
#endif
}
