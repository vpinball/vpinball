#include "stdafx.h"

//
// license:GPLv3+
// Ported at: VisualPinball.Unity/VisualPinball.Unity/VPT/Bumper/BumperCollider.cs
//

void BumperHitCircle::Collide(const CollisionEvent& coll)
{
   if (!m_enabled) return;

   const float dot = coll.m_hitnormal.Dot(coll.m_ball->m_d.m_vel); // needs to be computed before Collide3DWall()!

   coll.m_ball->Collide3DWall(coll.m_hitnormal, m_elasticity, m_elasticityFalloff, m_friction, m_scatter); // reflect ball from wall

   if (m_pbumper->m_d.m_hitEvent && (dot <= -m_pbumper->m_d.m_threshold)) // if velocity greater than threshold level
   {
      coll.m_ball->m_d.m_vel += coll.m_hitnormal * m_pbumper->m_d.m_force; // add a chunk of velocity to drive ball away

      m_bumperanim_hitEvent = true;
      m_bumperanim_hitBallPosition = coll.m_ball->m_d.m_pos;
      m_pbumper->FireGroupEvent(DISPID_HitEvents_Hit);
   }
}

//
// end of license:GPLv3+, back to 'old MAME'-like
//

////////////////////////////////////////////////////////////////////////////////
//
// license:GPLv3+
// Ported at: VisualPinball.Unity/VisualPinball.Unity/Physics/Collider/LineCollider.cs
//

float LineSegSlingshot::HitTest(const BallS& ball, const float dtime, CollisionEvent& coll) const
{
   return HitTestBasic(ball, dtime, coll, true, true, true);
}

void LineSegSlingshot::Collide(const CollisionEvent& coll)
{
   Ball * const pball = coll.m_ball;
   const Vertex3Ds& hitnormal = coll.m_hitnormal;

   const float dot = coll.m_hitnormal.Dot(coll.m_ball->m_d.m_vel); // normal velocity to slingshot

   const bool threshold = (dot <= -m_psurface->m_d.m_slingshot_threshold);  // normal greater than threshold?

   if (!m_psurface->m_disabled && threshold) // enabled and if velocity greater than threshold level		
   {
      const float len = (v2.x - v1.x)*hitnormal.y - (v2.y - v1.y)*hitnormal.x; // length of segment, Unit TAN points from V1 to V2

      const Vertex2D vhitpoint(pball->m_d.m_pos.x - hitnormal.x * pball->m_d.m_radius, //project ball radius along norm
         pball->m_d.m_pos.y - hitnormal.y * pball->m_d.m_radius);

      // vhitpoint will now be the point where the ball hits the line
      // Calculate this distance from the center of the slingshot to get force

      const float btd = (vhitpoint.x - v1.x)*hitnormal.y - (vhitpoint.y - v1.y)*hitnormal.x; // distance to vhit from V1
      float force = (fabsf(len) > 1.0e-6f) ? ((btd + btd) / len - 1.0f) : -1.0f;	// -1..+1
      force = 0.5f *(1.0f - force*force); //!! maximum value 0.5 ...I think this should have been 1.0...oh well
      // will match the previous physics
      force *= m_force;//-80;

      pball->m_d.m_vel -= hitnormal * force; // boost velocity, drive into slingshot (counter normal), allow CollideWall to handle the remainder
   }

   pball->Collide3DWall(hitnormal, m_elasticity, m_elasticityFalloff, m_friction, m_scatter);

   if (m_obj && m_fe && !m_psurface->m_disabled && threshold)
   {
      // is this the same place as last event? if same then ignore it
       const float dist_ls = (pball->m_lastEventPos - pball->m_d.m_pos).LengthSquared();
       pball->m_lastEventPos = pball->m_d.m_pos; //remember last collide position

       if (dist_ls > 0.25f) //!! magic distance, must be a new place if only by a little
       {
           ((IFireEvents *)m_obj)->FireGroupEvent(DISPID_SurfaceEvents_Slingshot);
           m_slingshotanim.m_TimeReset = g_pplayer->m_time_msec + 100;
       }
   }
}

void SlingshotAnimObject::Animate()
{
   if (!m_iframe && (m_TimeReset != 0) && m_animations)
   {
      m_iframe = true;
   }
   else if (m_iframe && (m_TimeReset < g_pplayer->m_time_msec))
   {
      m_iframe = false;
      m_TimeReset = 0;
   }
}

// Ported at: VisualPinball.Engine/VPT/Gate/GateHit.cs
//            VisualPinball.Engine/VPT/Gate/GateHitGenerator.cs
//            VisualPinball.Engine/VPT/Spinner/SpinnerHit.cs
//            VisualPinball.Unity/VisualPinball.Unity/VPT/Gate/GateCollider.cs
//            VisualPinball.Unity/VisualPinball.Unity/VPT/Gate/GateDisplacementSystem.cs

HitGate::HitGate(Gate * const pgate, const float height)
{
    m_pgate = pgate;
    const float halflength = pgate->m_d.m_length * 0.5f;

    const float radangle = ANGTORAD(pgate->m_d.m_rotation);
    const float sn = sinf(radangle);
    const float cs = cosf(radangle);

    m_lineseg[0].m_hitBBox.zlow = height;
    m_lineseg[0].m_hitBBox.zhigh = height + (float)(2.0*PHYS_SKIN);
    m_lineseg[1].m_hitBBox.zlow = height;
    m_lineseg[1].m_hitBBox.zhigh = height + (float)(2.0*PHYS_SKIN);

    m_lineseg[0].m_ObjType = eGate;
    m_lineseg[1].m_ObjType = eGate;

    m_lineseg[0].v2.x = pgate->m_d.m_vCenter.x + cs*(halflength + (float)PHYS_SKIN); //oversize by the ball radius
    m_lineseg[0].v2.y = pgate->m_d.m_vCenter.y + sn*(halflength + (float)PHYS_SKIN); //this will prevent clipping
    m_lineseg[0].v1.x = pgate->m_d.m_vCenter.x - cs*(halflength + (float)PHYS_SKIN); //through the edge of the
    m_lineseg[0].v1.y = pgate->m_d.m_vCenter.y - sn*(halflength + (float)PHYS_SKIN); //spinner

    m_lineseg[1].v1.x = m_lineseg[0].v2.x;
    m_lineseg[1].v1.y = m_lineseg[0].v2.y;
    m_lineseg[1].v2.x = m_lineseg[0].v1.x;
    m_lineseg[1].v2.y = m_lineseg[0].v1.y;

    m_lineseg[0].CalcNormal();
    m_lineseg[1].CalcNormal();

    m_gateMover.m_angleMin = pgate->m_d.m_angleMin;
    m_gateMover.m_angleMax = pgate->m_d.m_angleMax;

    m_gateMover.m_friction = pgate->m_d.m_friction;
    m_gateMover.m_visible = pgate->m_d.m_visible;

    m_gateMover.m_angle = m_gateMover.m_angleMin;
    m_gateMover.m_anglespeed = 0.0f;
    m_gateMover.m_damping = powf(pgate->m_d.m_damping, (float)PHYS_FACTOR); //0.996f;
    m_gateMover.m_gravityfactor = pgate->m_d.m_gravityfactor;

    m_gateMover.m_pgate = pgate;
    m_gateMover.m_open = false;
    m_gateMover.m_forcedMove = false;
    m_twoWay = false;
}

float HitGate::HitTest(const BallS& ball, const float dtime, CollisionEvent& coll) const
{
   if (!m_enabled) return -1.0f;

   for (unsigned int i = 0; i < 2; ++i)
   {
      const float hittime = m_lineseg[i].HitTestBasic(ball, dtime, coll, false, true, false); // any face, lateral, non-rigid
      if (hittime >= 0.f)
      {
         // signal the Collide() function that the hit is on the front or back side
         coll.m_hitflag = !!i;

         return hittime;
      }
   }

   return -1.0f;
}

void HitGate::Collide(const CollisionEvent& coll)
{
   const float dot = coll.m_hitnormal.Dot(coll.m_ball->m_d.m_vel);
   const float h = m_pgate->m_d.m_height*0.5f;

   //linear speed = ball speed
   //angular speed = linear/radius (height of hit)
   float speed = fabsf(dot);
   // h is the height of the gate axis.
   if (fabsf(h) > 1.0f) // avoid divide by zero
      speed /= h;

   m_gateMover.m_anglespeed = speed;
   if (!coll.m_hitflag && !m_twoWay)
   {
      m_gateMover.m_hitDirection = (dot > 0.f);
      m_gateMover.m_anglespeed *= (float)(1.0/8.0); // Give a little bounce-back.
      return;                                       // hit from back doesn't count if not two-way
   }

   m_gateMover.m_hitDirection = false;

   // We encoded which side of the gate the ball hit
   if (coll.m_hitflag && m_twoWay)
       m_gateMover.m_anglespeed = -m_gateMover.m_anglespeed;

   FireHitEvent(coll.m_ball);
}

void HitGate::CalcHitBBox()
{
   // Bounding rect for both lines will be the same
   m_lineseg[0].CalcHitBBox();
   m_hitBBox = m_lineseg[0].m_hitBBox;
}

void GateMoverObject::UpdateDisplacements(const float dtime)
{
   if (m_pgate->m_d.m_twoWay)
   {
      if (fabsf(m_angle) > m_angleMax)
      {
         if (m_angle < 0.0f)
            m_angle = -m_angleMax;
         else
            m_angle = m_angleMax;
         m_pgate->FireVoidEventParm(DISPID_LimitEvents_EOS, fabsf(RADTOANG(m_anglespeed)));	// send EOS event
         if (!m_forcedMove)
         {
            m_anglespeed = -m_anglespeed;
            m_anglespeed *= m_damping * 0.8f; // just some extra damping to reduce the anglespeed a bit faster
         }
         else if (m_anglespeed > 0.0f)
               m_anglespeed = 0.0f;
      }
      if (fabsf(m_angle) < m_angleMin)
      {
         if (m_angle < 0.0f)
            m_angle = -m_angleMin;
         else
            m_angle = m_angleMin;
         if (!m_forcedMove)
         {
            m_anglespeed = -m_anglespeed;
            m_anglespeed *= m_damping * 0.8f; // just some extra damping to reduce the anglespeed a bit faster
         }
         else if (m_anglespeed < 0.0f)
               m_anglespeed = 0.0f;
      }
   }
   else
   {
      const float direction = m_hitDirection ? -1.f : 1.f;
      if (direction * m_angle > m_angleMax)
      {
         m_angle = direction * m_angleMax;
         m_pgate->FireVoidEventParm(DISPID_LimitEvents_EOS, fabsf(RADTOANG(m_anglespeed)));	// send EOS event
         if (!m_forcedMove)
         {
            m_anglespeed = -m_anglespeed;
            m_anglespeed *= m_damping * 0.8f; // just some extra damping to reduce the anglespeed a bit faster
         }
         else if (m_anglespeed > 0.0f)
               m_anglespeed = 0.0f;
      }
      if (direction * m_angle < m_angleMin)
      {
         m_angle = direction * m_angleMin;
         m_pgate->FireVoidEventParm(DISPID_LimitEvents_BOS, fabsf(RADTOANG(m_anglespeed)));	// send Park event
         if (!m_forcedMove)
         {
            m_anglespeed = -m_anglespeed;
            m_anglespeed *= m_damping * 0.8f; // just some extra damping to reduce the anglespeed a bit faster
         }
         else if (m_anglespeed < 0.0f)
               m_anglespeed = 0.0f;
      }
   }
   if(m_anglespeed==0.0f)
      m_forcedMove = false;

   m_angle += m_anglespeed * dtime;
}

void GateMoverObject::UpdateVelocities()
{
   if (!m_open)
   {
      if (fabsf(m_angle) < (m_angleMin+0.01f) && fabsf(m_anglespeed) < 0.01f)
      {
         // stop a bit earlier to prevent a nearly endless animation (especially for slow balls)
         m_angle = m_angleMin;
         m_anglespeed = 0.0f;
      }
      if (fabsf(m_anglespeed) != 0.0f && m_angle != m_angleMin)
      {
         m_anglespeed -= sinf(m_angle) * m_gravityfactor * (float)(PHYS_FACTOR / 100.0); // Center of gravity towards bottom of object, makes it stop vertical
         m_anglespeed *= m_damping;
      }
   }
}

// Ported at: VisualPinball.Engine/Physics/Hit3DPoly.cs
//            VisualPinball.Engine/VPT/Spinner/SpinnerHit.cs
//            VisualPinball.Engine/VPT/Spinner/SpinnerHitGenerator.cs
//            VisualPinball.Unity/VisualPinball.Unity/VPT/Spinner/SpinnerCollider.cs
//            VisualPinball.Unity/VisualPinball.Unity/VPT/Spinner/SpinnerDisplacementSystem.cs

HitSpinner::HitSpinner(Spinner * const pspinner, const float height)
{
   m_spinnerMover.m_pspinner = pspinner;

   const float halflength = pspinner->m_d.m_length * 0.5f;

   const float radangle = ANGTORAD(pspinner->m_d.m_rotation);
   const float sn = sinf(radangle);
   const float cs = cosf(radangle);

   m_lineseg[0].m_hitBBox.zlow = height;
   m_lineseg[0].m_hitBBox.zhigh = height + (float)(2.0*PHYS_SKIN);
   m_lineseg[1].m_hitBBox.zlow = height;
   m_lineseg[1].m_hitBBox.zhigh = height + (float)(2.0*PHYS_SKIN);

   m_lineseg[0].m_ObjType = eSpinner;
   m_lineseg[1].m_ObjType = eSpinner;

   m_lineseg[0].v2.x = pspinner->m_d.m_vCenter.x + cs*(halflength + (float)PHYS_SKIN); //oversize by the ball radius
   m_lineseg[0].v2.y = pspinner->m_d.m_vCenter.y + sn*(halflength + (float)PHYS_SKIN); //this will prevent clipping
   m_lineseg[0].v1.x = pspinner->m_d.m_vCenter.x - cs*(halflength + (float)PHYS_SKIN); //through the edge of the
   m_lineseg[0].v1.y = pspinner->m_d.m_vCenter.y - sn*(halflength + (float)PHYS_SKIN); //spinner
   m_lineseg[1].v1.x = m_lineseg[0].v2.x;
   m_lineseg[1].v1.y = m_lineseg[0].v2.y;
   m_lineseg[1].v2.x = m_lineseg[0].v1.x;
   m_lineseg[1].v2.y = m_lineseg[0].v1.y;

   m_lineseg[0].CalcNormal();
   m_lineseg[1].CalcNormal();

   m_spinnerMover.m_angleMax = ANGTORAD(pspinner->m_d.m_angleMax);
   m_spinnerMover.m_angleMin = ANGTORAD(pspinner->m_d.m_angleMin);

   m_spinnerMover.m_angle = clamp(0.0f, m_spinnerMover.m_angleMin, m_spinnerMover.m_angleMax);
   m_spinnerMover.m_anglespeed = 0;
   // compute proper damping factor for physics framerate
   m_spinnerMover.m_damping = powf(pspinner->m_d.m_damping, (float)PHYS_FACTOR);

   m_spinnerMover.m_elasticity = pspinner->m_d.m_elasticity;
   m_spinnerMover.m_visible = pspinner->m_d.m_visible;
}

float HitSpinner::HitTest(const BallS& ball, const float dtime, CollisionEvent& coll) const
{
   if (!m_enabled) return -1.0f;

   for (unsigned int i = 0; i < 2; ++i)
   {
      const float hittime = m_lineseg[i].HitTestBasic(ball, dtime, coll, false, true, false); // any face, lateral, non-rigid
      if (hittime >= 0.f)
      {
         // signal the Collide() function that the hit is on the front or back side
         coll.m_hitflag = !i;

         return hittime;
      }
   }

   return -1.0f;
}

void HitSpinner::Collide(const CollisionEvent& coll)
{
   const float dot = coll.m_hitnormal.Dot(coll.m_ball->m_d.m_vel);

   if (dot < 0.f) return; //hit from back doesn't count

   const float h = m_spinnerMover.m_pspinner->m_d.m_height*0.5f;
   //linear speed = ball speed
   //angular speed = linear/radius (height of hit)

   // h is the height of the spinner axis;
   // Since the spinner has no mass in our equation, the spot
   // h -coll.m_radius will be moving a at linear rate of
   // 'speed'.  We can calculate the angular speed from that.

   m_spinnerMover.m_anglespeed = fabsf(dot); // use this until a better value comes along

   if (fabsf(h) > 1.0f)			// avoid divide by zero
       m_spinnerMover.m_anglespeed /= h;
   m_spinnerMover.m_anglespeed *= m_spinnerMover.m_damping;

   // We encoded which side of the spinner the ball hit
   if (coll.m_hitflag)
       m_spinnerMover.m_anglespeed = -m_spinnerMover.m_anglespeed;
}

void SpinnerMoverObject::UpdateDisplacements(const float dtime)
{
   if (m_pspinner->m_d.m_angleMin != m_pspinner->m_d.m_angleMax)	//blocked spinner, limited motion spinner
   {
      m_angle += m_anglespeed * dtime;

      if (m_angle > m_angleMax)
      {
         m_angle = m_angleMax;
         m_pspinner->FireVoidEventParm(DISPID_LimitEvents_EOS, fabsf(RADTOANG(m_anglespeed)));	// send EOS event

         if (m_anglespeed > 0.f)
            m_anglespeed *= -0.005f - m_elasticity;
      }
      if (m_angle < m_angleMin)
      {
         m_angle = m_angleMin;

         m_pspinner->FireVoidEventParm(DISPID_LimitEvents_BOS, fabsf(RADTOANG(m_anglespeed)));	// send Park event

         if (m_anglespeed < 0.f)
            m_anglespeed *= -0.005f - m_elasticity;
      }
   }
   else
   {
      const float target = (m_anglespeed > 0.f) ?
         ((m_angle < (float)M_PI) ? (float)M_PI : (float)(3.0*M_PI))
         :
         ((m_angle < (float)M_PI) ? (float)(-M_PI) : (float)M_PI);

      m_angle += m_anglespeed * dtime;

      if (m_anglespeed > 0.f)
      {
         if (m_angle > target)
            m_pspinner->FireGroupEvent(DISPID_SpinnerEvents_Spin);
      }
      else
      {
         if (m_angle < target)
            m_pspinner->FireGroupEvent(DISPID_SpinnerEvents_Spin);
      }

      while (m_angle > (float)(2.0*M_PI))
         m_angle -= (float)(2.0*M_PI);
      while (m_angle < 0.0f)
         m_angle += (float)(2.0*M_PI);
   }
}

void SpinnerMoverObject::UpdateVelocities()
{
   m_anglespeed -= sinf(m_angle) * (float)(0.0025 * PHYS_FACTOR); // Center of gravity towards bottom of object, makes it stop vertical

   m_anglespeed *= m_damping;
}

void HitSpinner::CalcHitBBox()
{
   // Bounding rect for both lines will be the same
   m_lineseg[0].CalcHitBBox();
   m_hitBBox = m_lineseg[0].m_hitBBox;
}

void Hit3DPoly::Init(Vertex3Ds * const rgv, const int count)
{
   m_rgv = rgv;
   m_cvertex = count;

   m_normal.x = 0.f;
   m_normal.y = 0.f;
   m_normal.z = 0.f;

   // Newell's method for normal computation
   for (int i = 0; i < m_cvertex; ++i)
   {
      const int m = (i < m_cvertex - 1) ? (i + 1) : 0;

      m_normal.x += (m_rgv[i].y - m_rgv[m].y) * (m_rgv[i].z + m_rgv[m].z);
      m_normal.y += (m_rgv[i].z - m_rgv[m].z) * (m_rgv[i].x + m_rgv[m].x);
      m_normal.z += (m_rgv[i].x - m_rgv[m].x) * (m_rgv[i].y + m_rgv[m].y);
   }

   const float sqr_len = m_normal.x * m_normal.x + m_normal.y * m_normal.y + m_normal.z * m_normal.z;
   const float inv_len = (sqr_len > 0.0f) ? -1.0f / sqrtf(sqr_len) : 0.0f;   // NOTE: normal is flipped! Thus we need vertices in CCW order
   m_normal.x *= inv_len;
   m_normal.y *= inv_len;
   m_normal.z *= inv_len;

   m_elasticity = 0.3f;
   SetFriction(0.3f);
   m_scatter = 0.f;
}

//
// end of license:GPLv3+, back to 'old MAME'-like
//

Hit3DPoly::Hit3DPoly(Vertex3Ds * const rgv, const int count)
{
    Init(rgv, count);
}

Hit3DPoly::Hit3DPoly(const float x, const float y, const float z, const float r, const int sections) // creates a circular hit poly
{
   Vertex3Ds * const rgv3d = new Vertex3Ds[sections];

   const float inv_sections = (float)(M_PI*2.0) / (float)sections;

   for (int i = 0; i < sections; ++i)
   {
      const float angle = inv_sections * (float)i;

      rgv3d[i].x = x + sinf(angle) * r;
      rgv3d[i].y = y + cosf(angle) * r;
      rgv3d[i].z = z;
   }

   Init(rgv3d, sections);
}

Hit3DPoly::~Hit3DPoly()
{
   delete[] m_rgv;
}

float Hit3DPoly::HitTest(const BallS& ball, const float dtime, CollisionEvent& coll) const
{
   if (!m_enabled) return -1.0f;

   const float bnv = m_normal.Dot(ball.m_vel); //speed in Normal-vector direction

   if ((m_ObjType != eTrigger) && (bnv > C_LOWNORMVEL)) // return if clearly ball is receding from object
      return -1.0f;

   // Point on the ball that will hit the polygon, if it hits at all
   Vertex3Ds hitPos = ball.m_pos - ball.m_radius * m_normal; // nearest point on ball ... projected radius along norm

   const float bnd = m_normal.Dot(hitPos - m_rgv[0]); // distance from plane to ball

   bool bUnHit = (bnv > C_LOWNORMVEL);
   const bool inside = (bnd <= 0.f);                  // in ball inside object volume

   const bool rigid = (m_ObjType != eTrigger);
   float hittime;
#ifdef NEW_PHYSICS
   bool isContact = false;
#endif
   if (rigid) //rigid polygon
   {
      if (bnd < -ball.m_radius/**2.0f*/) return -1.0f; // (ball normal distance) excessive penetration of object skin ... no collision HACK //!! *2 necessary?

      if (bnd <= (float)PHYS_TOUCH)
      {
#ifdef NEW_PHYSICS
          if (fabsf(bnv) <= C_CONTACTVEL)
          {
              hittime = 0;
              isContact = true;
          }
          else if (inside)
              hittime = 0;                          // zero time for rigid fast bodies
          else
              hittime = bnd / -bnv;
#else
          if (inside || (fabsf(bnv) > C_CONTACTVEL) // fast velocity, return zero time
                                                    //zero time for rigid fast bodies
              || (bnd <= (float)(-PHYS_TOUCH)))     // slow moving but embedded
              hittime = 0;
          else
              hittime = bnd*(float)(1.0/(2.0*PHYS_TOUCH)) + 0.5f; // don't compete for fast zero time events
#endif
      }
      else if (fabsf(bnv) > C_LOWNORMVEL)           // not velocity low?
          hittime = bnd / -bnv;                     // rate ok for safe divide 
      else
          return -1.0f;                             // wait for touching
   }
   else //non-rigid polygon
   {
      if (bnv * bnd >= 0.f)                         // outside-receding || inside-approaching
      {
         if (//(m_ObjType != eTrigger) ||           // not a trigger? // always false due to rigid test
            (!ball.m_vpVolObjs) ||                  // temporary ball
            // if trigger, then check:
            (fabsf(bnd) >= ball.m_radius*0.5f) ||   // not too close ... nor too far away
            (inside != (FindIndexOf(*(ball.m_vpVolObjs), m_obj) < 0))) // ...ball outside and hit set or ball inside and no hit set
            return -1.0f;

         hittime = 0;
         bUnHit = !inside; // ball on outside is UnHit, otherwise it's a Hit
      }
      else
         hittime = bnd / -bnv;
   }

   if (infNaN(hittime) || hittime < 0.f || hittime > dtime) return -1.0f; // time is outside this frame ... no collision

   hitPos += hittime * ball.m_vel;     // advance hit point to contact

   // Do a point in poly test, using the xy plane, to see if the hit point is inside the polygon
   //this need to be changed to a point in polygon on 3D plane

   float x2 = m_rgv[0].x;
   float y2 = m_rgv[0].y;
   bool hx2 = (hitPos.x >= x2);
   bool hy2 = (hitPos.y <= y2);
   int crosscount = 0;	// count of lines which the hit point is to the left of
   for (int i = 0; i < m_cvertex; i++)
   {
      const float x1 = x2;
      const float y1 = y2;
      const bool hx1 = hx2;
      const bool hy1 = hy2;

      const int j = (i < m_cvertex - 1) ? (i + 1) : 0;
      x2 = m_rgv[j].x;
      y2 = m_rgv[j].y;
      hx2 = (hitPos.x >= x2);
      hy2 = (hitPos.y <= y2);

      if ((y1 == y2) ||
         (hy1 && hy2) || (!hy1 && !hy2) || // if out of y range, forget about this segment
         (hx1 && hx2)) // Hit point is on the right of the line
         continue;

      if (!hx1 && !hx2)
      {
         crosscount ^= 1;
         continue;
      }

      if (x2 == x1)
      {
         if (!hx2)
            crosscount ^= 1;
         continue;
      }

      // Now the hard part - the hit point is in the line bounding box

      if (x2 - (y2 - hitPos.y)*(x1 - x2) / (y1 - y2) > hitPos.x)
         crosscount ^= 1;
   }

   if (crosscount & 1)
   {
      coll.m_hitnormal = m_normal;

      if (!rigid)                 // non rigid body collision? return direction
         coll.m_hitflag = bUnHit; // UnHit signal	is receding from outside target

      coll.m_hitdistance = bnd;   // 3dhit actual contact distance ... 
      //coll.m_hitRigid = rigid;  // collision type

#ifdef NEW_PHYSICS
      coll.m_isContact = isContact;
      if (isContact)
         coll.m_hit_org_normalvelocity = bnv;
#endif

      return hittime;
   }

   return -1.0f;
}

void Hit3DPoly::Collide(const CollisionEvent& coll)
{
   Ball * const pball = coll.m_ball;
   const Vertex3Ds& hitnormal = coll.m_hitnormal;

   if (m_ObjType != eTrigger)
   {
      const float dot = -(hitnormal.Dot(pball->m_d.m_vel));

      pball->Collide3DWall(m_normal, m_elasticity, m_elasticityFalloff, m_friction, m_scatter);

      if (m_obj && m_fe && dot >= m_threshold)
      {
          if (m_ObjType == ePrimitive)
          {
             m_obj->m_currentHitThreshold = dot;
             FireHitEvent(pball);
          }
          else if (m_ObjType == eHitTarget && ((HitTarget*)m_obj)->m_d.m_isDropped == false)
          {
             ((HitTarget*)m_obj)->m_hitEvent = true;
             m_obj->m_currentHitThreshold = dot;
             FireHitEvent(pball);
          }
      }
   }
   else // trigger:
   {
      if (!pball->m_d.m_vpVolObjs) return;

      const int i = FindIndexOf(*(pball->m_d.m_vpVolObjs), m_obj); // if -1 then not in objects volume set (i.e not already hit)

      if ((!coll.m_hitflag) == (i < 0)) // Hit == NotAlreadyHit
      {
         pball->m_d.m_pos += STATICTIME * pball->m_d.m_vel;      //move ball slightly forward

         if (i < 0)
         {
            pball->m_d.m_vpVolObjs->push_back(m_obj);
            ((Trigger*)m_obj)->FireGroupEvent(DISPID_HitEvents_Hit);
         }
         else
         {
            pball->m_d.m_vpVolObjs->erase(pball->m_d.m_vpVolObjs->begin() + i);
            ((Trigger*)m_obj)->FireGroupEvent(DISPID_HitEvents_Unhit);
         }
      }
   }
}

void Hit3DPoly::CalcHitBBox()
{
   m_hitBBox.left = m_rgv[0].x;
   m_hitBBox.right = m_rgv[0].x;
   m_hitBBox.top = m_rgv[0].y;
   m_hitBBox.bottom = m_rgv[0].y;
   m_hitBBox.zlow = m_rgv[0].z;
   m_hitBBox.zhigh = m_rgv[0].z;

   for (int i = 1; i < m_cvertex; i++)
   {
      m_hitBBox.left = min(m_rgv[i].x, m_hitBBox.left);
      m_hitBBox.right = max(m_rgv[i].x, m_hitBBox.right);
      m_hitBBox.top = min(m_rgv[i].y, m_hitBBox.top);
      m_hitBBox.bottom = max(m_rgv[i].y, m_hitBBox.bottom);
      m_hitBBox.zlow = min(m_rgv[i].z, m_hitBBox.zlow);
      m_hitBBox.zhigh = max(m_rgv[i].z, m_hitBBox.zhigh);
   }
}

//
// license:GPLv3+
// Ported at: VisualPinball.Engine/Physics/HitTriangle.cs
//            VisualPinball.Unity/VisualPinball.Unity/Physics/Collider/TriangleCollider.cs
//

HitTriangle::HitTriangle(const Vertex3Ds rgv[3])
{
   m_rgv[0] = rgv[0];
   m_rgv[1] = rgv[1];
   m_rgv[2] = rgv[2];

   /* NB: due to the swapping of the order of e0 and e1,
    * the vertices must be passed in counterclockwise order
    * (but rendering uses clockwise order!)
    */
   const Vertex3Ds e0 = m_rgv[2] - m_rgv[0];
   const Vertex3Ds e1 = m_rgv[1] - m_rgv[0];
   m_normal = CrossProduct(e0, e1);
   m_normal.NormalizeSafe();

   m_elasticity = 0.3f;
   SetFriction(0.3f);
   m_scatter = 0.f;
}

float HitTriangle::HitTest(const BallS& ball, const float dtime, CollisionEvent& coll) const
{
   if (!m_enabled) return -1.0f;

   const float bnv = m_normal.Dot(ball.m_vel); // speed in Normal-vector direction

   if (bnv > C_CONTACTVEL)                     // return if clearly ball is receding from object
      return -1.0f;

   // Point on the ball that will hit the polygon, if it hits at all
   Vertex3Ds hitPos = ball.m_pos - ball.m_radius * m_normal; // nearest point on ball ... projected radius along norm

   const float bnd = m_normal.Dot(hitPos - m_rgv[0]);  // distance from plane to ball

   if (bnd < -ball.m_radius/**2.0f*/) //!! *2 necessary?
      return -1.0f;	// (ball normal distance) excessive penetration of object skin ... no collision HACK

   bool isContact = false;
   float hittime;

   if (bnd <= (float)PHYS_TOUCH)
   {
      if (fabsf(bnv) <= C_CONTACTVEL)
      {
         hittime = 0;
         isContact = true;
      }
      else if (bnd <= 0.f)
         hittime = 0;                            // zero time for rigid fast bodies
      else
         hittime = bnd / -bnv;
   }
   else if (fabsf(bnv) > C_LOWNORMVEL)           // not velocity low?
      hittime = bnd / -bnv;                      // rate ok for safe divide 
   else
      return -1.0f;                              // wait for touching

   if (infNaN(hittime) || hittime < 0.f || hittime > dtime)
      return -1.0f;	// time is outside this frame ... no collision

   hitPos += hittime * ball.m_vel; // advance hit point to contact

   // check if hitPos is within the triangle

   // Compute vectors
   const Vertex3Ds v0 = m_rgv[2] - m_rgv[0];
   const Vertex3Ds v1 = m_rgv[1] - m_rgv[0];
   const Vertex3Ds v2 = hitPos - m_rgv[0];

   // Compute dot products
   const float dot00 = v0.Dot(v0);
   const float dot01 = v0.Dot(v1);
   const float dot02 = v0.Dot(v2);
   const float dot11 = v1.Dot(v1);
   const float dot12 = v1.Dot(v2);

   // Compute barycentric coordinates
   const float invDenom = 1.0f / (dot00 * dot11 - dot01 * dot01);
   const float u = (dot11 * dot02 - dot01 * dot12) * invDenom;
   const float v = (dot00 * dot12 - dot01 * dot02) * invDenom;

   // Check if point is in triangle
   if ((u >= 0.f) && (v >= 0.f) && (u + v <= 1.f))
   {
      coll.m_hitnormal = m_normal;

      coll.m_hitdistance = bnd;				// 3dhit actual contact distance ... 
      //coll.m_hitRigid = true;				// collision type

      coll.m_isContact = isContact;
      if(isContact)
         coll.m_hit_org_normalvelocity = bnv;

      return hittime;
   }
   else
      return -1.0f;
}

void HitTriangle::Collide(const CollisionEvent& coll)
{
   Ball * const pball = coll.m_ball;
   const Vertex3Ds& hitnormal = coll.m_hitnormal;

   const float dot = -(hitnormal.Dot(pball->m_d.m_vel));

   pball->Collide3DWall(m_normal, m_elasticity, m_elasticityFalloff, m_friction, m_scatter);

   if (m_obj && m_fe && dot >= m_threshold)
   {
      if (m_ObjType == ePrimitive)
      {
         m_obj->m_currentHitThreshold = dot;
         FireHitEvent(pball);
      }
      else if (m_ObjType == eHitTarget && ((HitTarget*)m_obj)->m_d.m_isDropped == false)
      {
          ((HitTarget*)m_obj)->m_hitEvent = true;
          m_obj->m_currentHitThreshold = dot;
          FireHitEvent(pball);
      }
   }
}

void HitTriangle::CalcHitBBox()
{
   m_hitBBox.left   = min(m_rgv[0].x, min(m_rgv[1].x, m_rgv[2].x));
   m_hitBBox.right  = max(m_rgv[0].x, max(m_rgv[1].x, m_rgv[2].x));
   m_hitBBox.top    = min(m_rgv[0].y, min(m_rgv[1].y, m_rgv[2].y));
   m_hitBBox.bottom = max(m_rgv[0].y, max(m_rgv[1].y, m_rgv[2].y));
   m_hitBBox.zlow   = min(m_rgv[0].z, min(m_rgv[1].z, m_rgv[2].z));
   m_hitBBox.zhigh  = max(m_rgv[0].z, max(m_rgv[1].z, m_rgv[2].z));
}

////////////////////////////////////////////////////////////////////////////////
// Ported at: VisualPinball.Engine/Physics/HitPlane.cs
//            VisualPinball.Unity/VisualPinball.Unity/Physics/Collider/PlaneCollider.cs

float HitPlane::HitTest(const BallS& ball, const float dtime, CollisionEvent& coll) const
{
   if (!m_enabled) return -1.0f;

   //slintf("HitPlane test - %f %f\n", pball->m_pos.z, pball->m_vel.z);

   const float bnv = m_normal.Dot(ball.m_vel); // speed in normal direction

   if (bnv > C_CONTACTVEL)                     // return if clearly ball is receding from object
      return -1.0f;

   const float bnd = m_normal.Dot(ball.m_pos) - ball.m_radius - m_d; // distance from plane to ball surface

   if (bnd < ball.m_radius*-2.0f) //!! solely responsible for ball through playfield?? check other places, too (radius*2??)
      return -1.0f;   // excessive penetration of plane ... no collision HACK

   float hittime;
#ifdef NEW_PHYSICS
   bool isContact = false;
   // slow moving ball? then either contact or no collision at all
   if (bnd <= (float)PHYS_TOUCH)
   {
       if (fabsf(bnv) <= C_CONTACTVEL)
       {
           hittime = 0;
           isContact = true;
       }
       else if (bnd <= 0.f)
           hittime = 0;                // zero time for rigid fast bodies
       else
           hittime = bnd / -bnv;
   }
   else if (fabsf(bnv) > C_LOWNORMVEL) // not velocity low?
       hittime = bnd / -bnv;           // rate ok for safe divide 
   else
       return -1.0f;                   // wait for touching
#else
   if (fabsf(bnv) <= C_CONTACTVEL)
   {
       if (fabsf(bnd) <= (float)PHYS_TOUCH)
       {
           coll.m_isContact = true;
           coll.m_hitnormal = m_normal;
           coll.m_hit_org_normalvelocity = bnv; // remember original normal velocity
           coll.m_hitdistance = bnd;
           //coll.m_hitRigid = true;
           return 0.0f;    // hittime is ignored for contacts
       }
       else
           return -1.0f;   // large distance, small velocity -> no hit
   }

   hittime = bnd / -bnv;   // rate ok for safe divide
   if (hittime < 0.f)
       hittime = 0.0f;     // already penetrating? then collide immediately
#endif

   if (infNaN(hittime) || hittime < 0.f || hittime > dtime)
      return -1.0f;       // time is outside this frame ... no collision

   coll.m_hitnormal = m_normal;
   coll.m_hitdistance = bnd;               // actual contact distance
   //coll.m_hitRigid = true;               // collision type

#ifdef NEW_PHYSICS
   coll.m_isContact = isContact;
   if(isContact)
      coll.m_hit_org_normalvelocity = bnv; // remember original normal velocity
#endif

   return hittime;
}

void HitPlane::Collide(const CollisionEvent& coll)
{
   //slintf("Playfield COLLISION - (%f %f %f) - (%f %f %f)\n",
   //        coll.m_ball->m_d.m_pos.x, coll.m_ball->m_d.m_pos.y, coll.m_ball->m_d.m_pos.z,
   //        coll.m_ball->m_d.m_vel.x, coll.m_ball->m_d.m_vel.y, coll.m_ball->m_d.m_vel.z);
   coll.m_ball->Collide3DWall(coll.m_hitnormal, m_elasticity, m_elasticityFalloff, m_friction, m_scatter);

#ifdef C_EMBEDSHOT_PLANE
   // if ball has penetrated, push it out of the plane
   const float bnd = m_normal.Dot(coll.m_ball->m_d.m_pos) - coll.m_ball->m_d.m_radius - m_d; // distance from plane to ball surface
   if (bnd < 0.f)
      coll.m_ball->m_d.m_pos -= bnd * m_normal;
#endif
}

////////////////////////////////////////////////////////////////////////////////
// Ported at: VisualPinball.Engine/Physics/HitLine3D.cs
//            VisualPinball.Unity/VisualPinball.Unity/Physics/Collider/Line3DCollider.cs
//

HitLine3D::HitLine3D(const Vertex3Ds& v1, const Vertex3Ds& v2)
{
   Vertex3Ds vLine = v2 - v1;
   vLine.Normalize();

   // Axis of rotation to make 3D cylinder a cylinder along the z-axis
   Vertex3Ds transaxis;
   /*const Vertex3Ds vup(0,0,1.0f);
   CrossProduct(vLine, vup, &transaxis);*/
   transaxis.x = vLine.y;
   transaxis.y = -vLine.x;
   transaxis.z = 0.0f;

   const float l = transaxis.LengthSquared();
   if (l <= 1e-6f)     // line already points in z axis?
      transaxis.Set(1.f, 0.f, 0.f);            // choose arbitrary rotation vector
   else
      transaxis /= sqrtf(l);

   // Angle to rotate the line into the z-axis
   const float dot = vLine.z; //vLine.Dot(&vup);

   //const float transangle = acosf(dot);
   //matTrans.RotationAroundAxis(transaxis, -transangle);
   m_matrix.RotationAroundAxis(transaxis,-sqrtf(1.f-dot*dot),dot);

   const Vertex3Ds vtrans1 = m_matrix * v1;
   const float vtrans2z = (m_matrix * v2).z;

   // set up HitLineZ parameters
   m_xy.x = vtrans1.x;
   m_xy.y = vtrans1.y;
   m_zlow = min(vtrans1.z, vtrans2z);
   m_zhigh = max(vtrans1.z, vtrans2z);

   m_hitBBox.left = min(v1.x, v2.x);
   m_hitBBox.right = max(v1.x, v2.x);
   m_hitBBox.top = min(v1.y, v2.y);
   m_hitBBox.bottom = max(v1.y, v2.y);
   m_hitBBox.zlow = min(v1.z, v2.z);
   m_hitBBox.zhigh = max(v1.z, v2.z);
}

float HitLine3D::HitTest(const BallS& ball, const float dtime, CollisionEvent& coll) const
{
   if (!m_enabled)
      return -1.0f;

   BallS ball_tmp = ball;
   // transform ball to cylinder coordinate system
   ball_tmp.m_pos = m_matrix * ball.m_pos;
   ball_tmp.m_vel = m_matrix * ball.m_vel;

   const float hittime = HitLineZ::HitTest(ball_tmp, dtime, coll);

   if (hittime >= 0.f)       // transform hit normal back to world coordinate system
      coll.m_hitnormal = m_matrix.MulVectorT(coll.m_hitnormal);

   return hittime;
}

void HitLine3D::Collide(const CollisionEvent& coll)
{
   Ball *const pball = coll.m_ball;
   const Vertex3Ds& hitnormal = coll.m_hitnormal;

   const float dot = -(hitnormal.Dot(pball->m_d.m_vel));
   pball->Collide3DWall(hitnormal, m_elasticity, m_elasticityFalloff, m_friction, m_scatter);

   if (m_obj && m_fe && dot >= m_threshold)
   {
       if (m_ObjType == ePrimitive)
       {
          m_obj->m_currentHitThreshold = dot;
          FireHitEvent(pball);
       }
       else if (m_ObjType == eHitTarget && ((HitTarget*)m_obj)->m_d.m_isDropped == false)
       {
           ((HitTarget*)m_obj)->m_hitEvent = true;
           m_obj->m_currentHitThreshold = dot;
           FireHitEvent(pball);
       }
   }
}

// Ported at: VisualPinball.Engine/VPT/Trigger/TriggerHitLineSeg.cs
//            VisualPinball.Unity/VisualPinball.Unity/VPT/Trigger/TriggerCollider.cs

float TriggerLineSeg::HitTest(const BallS& ball, const float dtime, CollisionEvent& coll) const
{
   if (!m_ptrigger->m_hitEnabled) return -1.0f;

   // approach either face, not lateral-rolling point (assume center), not a rigid body contact
   return HitTestBasic(ball, dtime, coll, false, false, false);
}

void TriggerLineSeg::Collide(const CollisionEvent& coll)
{
   Ball * const pball = coll.m_ball;

   if ((m_ObjType != eTrigger) ||
      (!pball->m_d.m_vpVolObjs)) return;

   const int i = FindIndexOf(*(pball->m_d.m_vpVolObjs), m_obj); // if -1 then not in objects volume set (i.e not already hit)

   if ((!coll.m_hitflag) == (i < 0))                 // Hit == NotAlreadyHit
   {
      pball->m_d.m_pos += STATICTIME * pball->m_d.m_vel;     // move ball slightly forward

      if (i < 0)
      {
         pball->m_d.m_vpVolObjs->push_back(m_obj);
         ((Trigger*)m_obj)->TriggerAnimationHit();
         ((Trigger*)m_obj)->FireGroupEvent(DISPID_HitEvents_Hit);
      }
      else
      {
         pball->m_d.m_vpVolObjs->erase(pball->m_d.m_vpVolObjs->begin() + i);
         ((Trigger*)m_obj)->TriggerAnimationUnhit();
         ((Trigger*)m_obj)->FireGroupEvent(DISPID_HitEvents_Unhit);
      }
   }
}


float TriggerHitCircle::HitTest(const BallS& ball, const float dtime, CollisionEvent& coll) const
{
   return HitTestBasicRadius(ball, dtime, coll, false, false, false); //any face, not-lateral, non-rigid
}

void TriggerHitCircle::Collide(const CollisionEvent& coll)
{
   Ball * const pball = coll.m_ball;

   if ((m_ObjType < eTrigger) || // triggers and kickers
      (!pball->m_d.m_vpVolObjs)) return;

   const int i = FindIndexOf(*(pball->m_d.m_vpVolObjs), m_obj); // if -1 then not in objects volume set (i.e not already hit)

   if ((!coll.m_hitflag) == (i < 0))                 // Hit == NotAlreadyHit
   {
      pball->m_d.m_pos += STATICTIME * pball->m_d.m_vel;     // move ball slightly forward

      if (i < 0)
      {
         pball->m_d.m_vpVolObjs->push_back(m_obj);
         ((Trigger*)m_obj)->TriggerAnimationHit();
         ((Trigger*)m_obj)->FireGroupEvent(DISPID_HitEvents_Hit);
      }
      else
      {
         pball->m_d.m_vpVolObjs->erase(pball->m_d.m_vpVolObjs->begin() + i);
         ((Trigger*)m_obj)->TriggerAnimationUnhit();
         ((Trigger*)m_obj)->FireGroupEvent(DISPID_HitEvents_Unhit);
      }
   }
}
