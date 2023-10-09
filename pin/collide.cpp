#include "stdafx.h"

#ifdef USE_EMBREE
#include <mutex>
static std::mutex mtx;
#endif


float c_hardScatter = 0.0f;


void HitObject::Contact(CollisionEvent& coll, const float dtime) { coll.m_ball->HandleStaticContact(coll, m_friction, dtime); }

void HitObject::FireHitEvent(Ball * const pball)
{
   if (m_obj && m_fe && m_enabled)
   {
      // is this the same place as last event? if same then ignore it
      const float dist_ls = (pball->m_lastEventPos - pball->m_d.m_pos).LengthSquared();
      pball->m_lastEventPos = pball->m_d.m_pos;    // remember last collision position

      const float normalDist = (m_ObjType == eHitTarget) ? 0.0f   // hit targets when used with a captured ball have always a too small distance
                                                         : 0.25f; //!! magic distance

      if (dist_ls > normalDist) // must be a new place if only by a little
         ((IFireEvents *)m_obj)->FireGroupEvent(DISPID_HitEvents_Hit);
   }
}


void LineSeg::CalcHitBBox()
{
   // Allow roundoff
   m_hitBBox.left = min(v1.x, v2.x);
   m_hitBBox.right = max(v1.x, v2.x);
   m_hitBBox.top = min(v1.y, v2.y);
   m_hitBBox.bottom = max(v1.y, v2.y);

   // zlow and zhigh were already set in ctor
}

//
// license:GPLv3+
// Ported at: VisualPinball.Unity/VisualPinball.Unity/Physics/Collider/LineCollider.cs
//

float LineSeg::HitTestBasic(const BallS& ball, const float dtime, CollisionEvent& coll, const bool direction, const bool lateral, const bool rigid) const
{
   if (!m_enabled || ball.m_lockedInKicker) return -1.0f;

   const float ballvx = ball.m_vel.x;						// ball velocity
   const float ballvy = ball.m_vel.y;

   const float bnv = ballvx*normal.x + ballvy*normal.y;		// ball velocity normal to segment, positive if receding, zero=parallel
   bool bUnHit = (bnv > C_LOWNORMVEL);

   if (direction && bUnHit)					// direction true and clearly receding from normal face
      return -1.0f;

   const float ballx = ball.m_pos.x;						// ball position
   const float bally = ball.m_pos.y;

   // ball normal distance: contact distance normal to segment. lateral contact subtract the ball radius 

   const float rollingRadius = lateral ? ball.m_radius : C_TOL_RADIUS; //lateral or rolling point
   const float bcpd = (ballx - v1.x)*normal.x + (bally - v1.y)*normal.y; // ball center to plane distance
   float bnd = bcpd - rollingRadius;

   // for a spinner add the ball radius otherwise the ball goes half through the spinner until it moves
   if (m_ObjType == eSpinner || m_ObjType == eGate)
       bnd = bcpd + rollingRadius;

   const bool inside = (bnd <= 0.f);						// in ball inside object volume

   float hittime;
   if (rigid)
   {
      if ((bnd < -ball.m_radius/**2.0f*/) || (lateral && bcpd < 0.f))
         return -1.0f;	// (ball normal distance) excessive pentratration of object skin ... no collision HACK //!! *2 necessary?

      if (lateral && (bnd <= (float)PHYS_TOUCH))
      {
         if (inside || (fabsf(bnv) > C_CONTACTVEL)			// fast velocity, return zero time
            // zero time for rigid fast bodies
            || (bnd <= (float)(-PHYS_TOUCH)))
            hittime = 0;									// slow moving but embedded
         else {
#ifdef NEW_PHYSICS
            hittime = bnd / -bnv;
#else
            hittime = bnd * (float)(1.0/(2.0*PHYS_TOUCH)) + 0.5f;	        // don't compete for fast zero time events
#endif
         }
      }
      else if (fabsf(bnv) > C_LOWNORMVEL)                   // not velocity low ????
         hittime = bnd / -bnv;                              // rate ok for safe divide 
      else
         return -1.0f;                                      // wait for touching
   }
   else //non-rigid ... target hits
   {
      if (bnv * bnd >= 0.f)                                 // outside-receding || inside-approaching
      {
         if ((m_ObjType != eTrigger) ||                     // not a trigger
             (!ball.m_vpVolObjs) ||
             // is a trigger, so test:
             (fabsf(bnd) >= ball.m_radius*0.5f) ||          // not too close ... nor too far away
             (inside != (FindIndexOf(*(ball.m_vpVolObjs), m_obj) < 0))) // ...ball outside and hit set or ball inside and no hit set
         {
              return -1.0f;
         }
         hittime = 0;
         bUnHit = !inside;	// ball on outside is UnHit, otherwise it's a Hit
      }
      else
         hittime = bnd / -bnv;
   }

   if (infNaN(hittime) || hittime < 0.f || hittime > dtime)
   {
       return -1.0f; // time is outside this frame ... no collision
   }
   const float btv = ballvx*normal.y - ballvy*normal.x;      // ball velocity tangent to segment with respect to direction from V1 to V2
   const float btd = (ballx - v1.x)*normal.y - (bally - v1.y)*normal.x // ball tangent distance
      + btv * hittime;                                       // ball tangent distance (projection) (initial position + velocity * hitime)

   if (btd < -C_TOL_ENDPNTS || btd > length + C_TOL_ENDPNTS) // is the contact off the line segment??? 
   {
       return -1.0f;
   }
   if (!rigid)                                               // non rigid body collision? return direction
      coll.m_hitflag = bUnHit;                               // UnHit signal is receding from outside target

   const float hitz = ball.m_pos.z + ball.m_vel.z*hittime;   // check too high or low relative to ball rolling point at hittime

   if (hitz + ball.m_radius*0.5f < m_hitBBox.zlow            // check limits of object's height and depth  
    || hitz - ball.m_radius*0.5f > m_hitBBox.zhigh)
   {
       return -1.0f;
   }
   coll.m_hitnormal.x = normal.x; // hit normal is same as line segment normal
   coll.m_hitnormal.y = normal.y;
   coll.m_hitnormal.z = 0.0f;

   coll.m_hitdistance = bnd;      // actual contact distance ...
   //coll.m_hitRigid = rigid;     // collision type

   // check for contact
   coll.m_isContact = (fabsf(bnv) <= C_CONTACTVEL && fabsf(bnd) <= (float)PHYS_TOUCH);
   if(coll.m_isContact)
      coll.m_hit_org_normalvelocity = bnv;

   return hittime;
}

float LineSeg::HitTest(const BallS& ball, const float dtime, CollisionEvent& coll) const
{
   return HitTestBasic(ball, dtime, coll, true, true, true); // normal face, lateral, rigid
}

void LineSeg::Collide(const CollisionEvent& coll)
{
   const float dot = coll.m_hitnormal.Dot(coll.m_ball->m_d.m_vel);
   coll.m_ball->Collide3DWall(coll.m_hitnormal, m_elasticity, m_elasticityFalloff, m_friction, m_scatter);

   if (dot <= -m_threshold)
      FireHitEvent(coll.m_ball);
}

void LineSeg::CalcNormal()
{
   const Vertex2D vT(v1.x - v2.x, v1.y - v2.y);

   // Set up line normal
   length = vT.Length();
   const float inv_length = 1.0f / length;
   normal.x = vT.y * inv_length;
   normal.y = -vT.x * inv_length;
}

////////////////////////////////////////////////////////////////////////////////
// Ported at: VisualPinball.Unity/VisualPinball.Unity/Physics/Collider/CircleCollider.cs

float HitCircle::HitTestBasicRadius(const BallS& ball, const float dtime, CollisionEvent& coll,
                                    const bool direction, const bool lateral, const bool rigid) const // all of these true = bumper/flipperbase/gate/spinner, all false = kicker/trigger
{
   if (!m_enabled || ball.m_lockedInKicker) return -1.0f;

   Vertex3Ds c(center.x, center.y, 0.0f);
   Vertex3Ds dist = ball.m_pos - c;    // relative ball position
   Vertex3Ds dv = ball.m_vel;

   const bool capsule3D = (!lateral && ball.m_pos.z > m_hitBBox.zhigh);

   float targetRadius;
   if (capsule3D)
   {
      // handle ball over target?
      //const float hcap = radius*(float)(1.0/5.0);           // cap height to hit-circle radius ratio
      //targetRadius = radius*radius/(hcap*2.0f) + hcap*0.5f; // c = (r^2+h^2)/(2*h)
      targetRadius = radius*(float)(13.0 / 5.0);              // optimized version of above code
      //c.z = m_hitBBox.zhigh - (targetRadius - hcap);        // b = c - h
      c.z = m_hitBBox.zhigh - radius*(float)(12.0 / 5.0);     // optimized version of above code
      dist.z = ball.m_pos.z - c.z;                            // ball rolling point - capsule center height 			
   }
   else
   {
      targetRadius = radius;
      if (lateral)
         targetRadius += ball.m_radius;
      dist.z = dv.z = 0.0f;
   }

   const float bcddsq = dist.LengthSquared();	// ball center to circle center distance ... squared
   const float bcdd = sqrtf(bcddsq);			// distance center to center
   if (bcdd <= 1.0e-6f)
      return -1.0f;                           // no hit on exact center

   const float b = dist.Dot(dv);
   const float bnv = b / bcdd;					// ball normal velocity

   if (direction && bnv > C_LOWNORMVEL)
      return -1.0f;                           // clearly receding from radius

   const float bnd = bcdd - targetRadius;		// ball normal distance to 

   const float a = dv.LengthSquared();

   float hittime = 0;
   bool bUnhit = false;
   bool isContact = false;
   // Kicker is special.. handle ball stalled on kicker, commonly hit while receding, knocking back into kicker pocket
   if (m_ObjType == eKicker && bnd <= 0.f && bnd >= -radius && a < C_CONTACTVEL*C_CONTACTVEL && ball.m_vpVolObjs)
   {
      RemoveFromVectorSingle(*(ball.m_vpVolObjs), m_obj); // cause capture
   }

   if (rigid && bnd < (float)PHYS_TOUCH)        // positive: contact possible in future ... Negative: objects in contact now
   {
      if (bnd < -ball.m_radius/**2.0f*/) //!! *2 necessary?
         return -1.0f;
      else if (fabsf(bnv) <= C_CONTACTVEL)
      {
         isContact = true;
      }
      else
         // estimate based on distance and speed along distance
         // the ball can be that fast that in the next hit cycle the ball will be inside the hit shape of a bumper or other element.
         // if that happens bnd is negative and greater than the negative bnv value: that results in a negative hittime
         // i.e. below the "if (infNan(hittime) || hittime <0.f...)" will then be true and the hit function would return -1.0f = no hit
         hittime = std::max(0.0f, -bnd / bnv);
   }
   else if (m_ObjType >= eTrigger // triggers & kickers
      && ball.m_vpVolObjs && ((bnd < 0.f) == (FindIndexOf(*(ball.m_vpVolObjs), m_obj) < 0)))
   { // here if ... ball inside and no hit set .... or ... ball outside and hit set

      if (fabsf(bnd - radius) < 0.05f) // if ball appears in center of trigger, then assumed it was gen'ed there
      {
         ball.m_vpVolObjs->push_back(m_obj); // special case for trigger overlaying a kicker
      }                                        // this will add the ball to the trigger space without a Hit
      else
      {
         bUnhit = (bnd > 0.f);	// ball on outside is UnHit, otherwise it's a Hit
      }
   }
   else
   {
      if ((!rigid && bnd * bnv > 0.f) || // (outside and receding) or (inside and approaching)
         (a < 1.0e-8f)) return -1.0f; // no hit ... ball not moving relative to object

      float time1, time2;
      if (!SolveQuadraticEq(a, 2.0f*b, bcddsq - targetRadius*targetRadius, time1, time2))
         return -1.0f;

      bUnhit = (time1*time2 < 0.f);
      hittime = bUnhit ? max(time1, time2) : min(time1, time2); // ball is inside the circle
   }

   if (infNaN(hittime) || hittime < 0.f || hittime > dtime)
      return -1.0f; // contact out of physics frame

   const float hitz = ball.m_pos.z + ball.m_vel.z * hittime; // rolling point

   if (((hitz + ball.m_radius*0.5f) < m_hitBBox.zlow) ||
      (!capsule3D && (hitz - ball.m_radius*0.5f) > m_hitBBox.zhigh) ||
      (capsule3D && hitz < m_hitBBox.zhigh)) return -1.0f;

   const float hitx = ball.m_pos.x + ball.m_vel.x*hittime;
   const float hity = ball.m_pos.y + ball.m_vel.y*hittime;

   const float sqrlen = (hitx - c.x)*(hitx - c.x) + (hity - c.y)*(hity - c.y);

   if (sqrlen > 1.0e-8f) // over center???
   { // no
      const float inv_len = 1.0f / sqrtf(sqrlen);
      coll.m_hitnormal.x = (hitx - c.x)*inv_len;
      coll.m_hitnormal.y = (hity - c.y)*inv_len;
      coll.m_hitnormal.z = 0.0f;
   }
   else
   { // yes, over center
      coll.m_hitnormal.x = 0.0f; // make up a value, any direction is ok
      coll.m_hitnormal.y = 1.0f;
      coll.m_hitnormal.z = 0.0f;
   }

   if (!rigid)                 // non rigid body collision? return direction
      coll.m_hitflag = bUnhit; // UnHit signal	is receding from target

   coll.m_isContact = isContact;
   if (isContact)
      coll.m_hit_org_normalvelocity = bnv;

   coll.m_hitdistance = bnd;   //actual contact distance ... 
   //coll.m_hitRigid = rigid;  // collision type

   return hittime;
}

void HitCircle::CalcHitBBox()
{
   // Allow roundoff
   m_hitBBox.left = center.x - radius;
   m_hitBBox.right = center.x + radius;
   m_hitBBox.top = center.y - radius;
   m_hitBBox.bottom = center.y + radius;

   // zlow & zhigh already set in ctor
}

float HitCircle::HitTest(const BallS& ball, const float dtime, CollisionEvent& coll) const
{
   //normal face, lateral, rigid
   return HitTestBasicRadius(ball, dtime, coll, true, true, true);
}

void HitCircle::Collide(const CollisionEvent& coll)
{
   coll.m_ball->Collide3DWall(coll.m_hitnormal, m_elasticity, m_elasticityFalloff, m_friction, m_scatter);
}

///////////////////////////////////////////////////////////////////////////////
// Ported at: VisualPinball.Unity/VisualPinball.Unity/Physics/Collider/LineZCollider.cs

float HitLineZ::HitTest(const BallS &ball, const float dtime, CollisionEvent& coll) const
{
   if (!m_enabled)
      return -1.0f;

   const Vertex2D bp2d(ball.m_pos.x, ball.m_pos.y);
   const Vertex2D dist = bp2d - m_xy;    // relative ball position
   const Vertex2D dv(ball.m_vel.x, ball.m_vel.y);

   const float bcddsq = dist.LengthSquared(); // ball center to line distance squared
   const float bcdd = sqrtf(bcddsq);          // distance ball to line
   if (bcdd <= 1.0e-6f)
      return -1.0f;                           // no hit on exact center

   const float b = dist.Dot(dv);
   const float bnv = b / bcdd;                // ball normal velocity

   if (bnv > C_CONTACTVEL)
      return -1.0f;                           // clearly receding from radius

   const float bnd = bcdd - ball.m_radius;    // ball distance to line

   const float a = dv.LengthSquared();

   float hittime = 0.f;
   bool isContact = false;

   if (bnd < (float)PHYS_TOUCH)       // already in collision distance?
   {
      if (fabsf(bnv) <= C_CONTACTVEL)
      {
         isContact = true;
         hittime = 0.f;
      }
      else
         hittime = /*std::max(0.0f,*/ -bnd / bnv /*)*/;   // estimate based on distance and speed along distance
   }
   else
   {
      if (a < 1.0e-8f)
         return -1.0f;    // no hit - ball not moving relative to object

      float time1, time2;
      if (!SolveQuadraticEq(a, 2.0f*b, bcddsq - ball.m_radius*ball.m_radius, time1, time2))
         return -1.0f;

      hittime = (time1*time2 < 0.f) ? max(time1, time2) : min(time1, time2); // find smallest nonnegative solution
   }

   if (infNaN(hittime) || hittime < 0 || hittime > dtime)
      return -1.0f; // contact out of physics frame

   const float hitz = ball.m_pos.z + hittime * ball.m_vel.z;   // ball z position at hit time

   if (hitz < m_zlow || hitz > m_zhigh)    // check z coordinate
      return -1.0f;

   const float hitx = ball.m_pos.x + hittime * ball.m_vel.x;   // ball x position at hit time
   const float hity = ball.m_pos.y + hittime * ball.m_vel.y;   // ball y position at hit time

   Vertex2D norm(hitx - m_xy.x, hity - m_xy.y);
   norm.Normalize();
   coll.m_hitnormal.Set(norm.x, norm.y, 0.0f);

   coll.m_isContact = isContact;
   if (isContact)
      coll.m_hit_org_normalvelocity = bnv;

   coll.m_hitdistance = bnd; // actual contact distance
   //coll.m_hitRigid = true;

   return hittime;
}

void HitLineZ::CalcHitBBox()
{
   m_hitBBox.left = m_xy.x;
   m_hitBBox.right = m_xy.x;
   m_hitBBox.top = m_xy.y;
   m_hitBBox.bottom = m_xy.y;
   m_hitBBox.zlow = m_zlow;
   m_hitBBox.zhigh = m_zhigh;
}

void HitLineZ::Collide(const CollisionEvent& coll)
{
   const float dot = coll.m_hitnormal.Dot(coll.m_ball->m_d.m_vel);
   coll.m_ball->Collide3DWall(coll.m_hitnormal, m_elasticity, m_elasticityFalloff, m_friction, m_scatter);

   if (dot <= -m_threshold)
      FireHitEvent(coll.m_ball);
}

///////////////////////////////////////////////////////////////////////////////
// Ported at: VisualPinball.Unity/VisualPinball.Unity/Physics/Collider/PointCollider.cs

float HitPoint::HitTest(const BallS &ball, const float dtime, CollisionEvent& coll) const
{
   if (!m_enabled)
      return -1.0f;

   const Vertex3Ds dist = ball.m_pos - m_p;   // relative ball position

   const float bcddsq = dist.LengthSquared(); // ball center to line distance squared
   const float bcdd = sqrtf(bcddsq);          // distance ball to line
   if (bcdd <= 1.0e-6f)
      return -1.0f;                           // no hit on exact center

   const float b = dist.Dot(ball.m_vel);
   const float bnv = b / bcdd;                // ball normal velocity

   if (bnv > C_CONTACTVEL)
      return -1.0f;                           // clearly receding from radius

   const float bnd = bcdd - ball.m_radius;    // ball distance to line

   const float a = ball.m_vel.LengthSquared();

   float hittime = 0.f;
   bool isContact = false;

   if (bnd < (float)PHYS_TOUCH)       // already in collision distance?
   {
      if (fabsf(bnv) <= C_CONTACTVEL)
      {
         isContact = true;
         hittime = 0.f;
      }
      else   // estimate based on distance and speed along distance
#ifdef NEW_PHYSICS
         hittime = -bnd / bnv;
#else
         hittime = std::max(0.0f, -bnd / bnv);
#endif
   }
   else
   {
      if (a < 1.0e-8f)
         return -1.0f;    // no hit - ball not moving relative to object

      float time1, time2;
      if (!SolveQuadraticEq(a, 2.0f*b, bcddsq - ball.m_radius*ball.m_radius, time1, time2))
         return -1.0f;

      hittime = (time1*time2 < 0.f) ? max(time1, time2) : min(time1, time2); // find smallest nonnegative solution
   }

   if (infNaN(hittime) || hittime < 0.f || hittime > dtime)
      return -1.0f; // contact out of physics frame

   const Vertex3Ds hitPos = ball.m_pos + hittime * ball.m_vel;
   coll.m_hitnormal = hitPos - m_p;
   coll.m_hitnormal.Normalize();

   coll.m_isContact = isContact;
   if (isContact)
      coll.m_hit_org_normalvelocity = bnv;

   coll.m_hitdistance = bnd;                    // actual contact distance
   //coll.m_hitRigid = true;

   return hittime;
}

void HitPoint::CalcHitBBox()
{
   m_hitBBox = FRect3D(m_p.x, m_p.x, m_p.y, m_p.y, m_p.z, m_p.z);
}

void HitPoint::Collide(const CollisionEvent& coll)
{
   const float dot = coll.m_hitnormal.Dot(coll.m_ball->m_d.m_vel);
   coll.m_ball->Collide3DWall(coll.m_hitnormal, m_elasticity, m_elasticityFalloff, m_friction, m_scatter);

   if (dot <= -m_threshold)
      FireHitEvent(coll.m_ball);
}

//
// end of license:GPLv3+, back to 'old MAME'-like
//

void DoHitTest(const Ball *const pball, const HitObject *const pho, CollisionEvent& coll)
{
   if (pho == nullptr || pball == nullptr
      || (pho->m_ObjType == eHitTarget && ((HitTarget*)pho->m_obj)->m_d.m_isDropped)) //!! why is this done here and not in corresponding HitTest()?
      return;

#ifdef DEBUGPHYSICS
   g_pplayer->c_deepTested++; //!! atomic needed if USE_EMBREE
#endif

   CollisionEvent newColl;
   const float newtime = pho->HitTest(pball->m_d, coll.m_hittime, newColl);
   const bool validhit = ((newtime >= 0.f) && !sign(newtime) && (newtime <= coll.m_hittime));

   if (validhit)
   {
      newColl.m_ball = const_cast<Ball*>(pball); //!! meh, but will not be changed in here
      newColl.m_obj = const_cast<HitObject*>(pho); //!! meh, but will not be changed in here
      newColl.m_hittime = newtime;

      if (g_pplayer->m_recordContacts && newColl.m_isContact) // remember all contacts?
      {
#ifdef USE_EMBREE
         const std::lock_guard<std::mutex> lg(mtx); // multiple threads may end up here and call push_back
#endif
         g_pplayer->m_contacts.push_back(newColl);
      }
      else // record first collision event
         coll = newColl;
   }
}
