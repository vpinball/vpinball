#include "stdafx.h"


float c_hardScatter = 0.0f;

HitObject *CreateCircularHitPoly(const float x, const float y, const float z, const float r, const int sections)
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

   return new Hit3DPoly(rgv3d, sections);
}
void HitObject::FireHitEvent(Ball * const pball)
{
   if (m_pfe && m_fEnabled)
   {
      float normalDist = 0.25f;
      // is this the same place as last event? if same then ignore it
      const float dist_ls = (pball->m_Event_Pos - pball->m_pos).LengthSquared();
      pball->m_Event_Pos = pball->m_pos;    //remember last collide position
      
      if (m_ObjType == eHitTarget) // hit targets when used with a captured ball have always a too small distance 
         normalDist = 0.0f;
      
      if (dist_ls > normalDist) // must be a new place if only by a little //!! magic distance
         m_pfe->FireGroupEvent(DISPID_HitEvents_Hit);
   }
}


void LineSeg::CalcHitRect()
{
   // Allow roundoff
   m_rcHitRect.left = min(v1.x, v2.x);
   m_rcHitRect.right = max(v1.x, v2.x);
   m_rcHitRect.top = min(v1.y, v2.y);
   m_rcHitRect.bottom = max(v1.y, v2.y);

   // zlow and zhigh were already set in ctor
}

float LineSeg::HitTestBasic(const Ball * const pball, const float dtime, CollisionEvent& coll, const bool direction, const bool lateral, const bool rigid)
{
   if (!m_fEnabled || pball->m_frozen) return -1.0f;

   const float ballvx = pball->m_vel.x;					// ball velocity
   const float ballvy = pball->m_vel.y;

   const float bnv = ballvx*normal.x + ballvy*normal.y;	// ball velocity normal to segment, positive if receding, zero=parallel
   bool bUnHit = (bnv > C_LOWNORMVEL);

   if (direction && (bnv > C_LOWNORMVEL))					// direction true and clearly receding from normal face
      return -1.0f;

   const float ballx = pball->m_pos.x;						// ball position
   const float bally = pball->m_pos.y;

   // ball normal distance: contact distance normal to segment. lateral contact subtract the ball radius 

   const float rollingRadius = lateral ? pball->m_radius : C_TOL_RADIUS; //lateral or rolling point
   const float bcpd = (ballx - v1.x)*normal.x + (bally - v1.y)*normal.y; // ball center to plane distance
   float bnd = bcpd - rollingRadius;

   // for a spinner add the ball radius otherwise the ball goes half through the spinner until it moves
   if (m_ObjType == eSpinner || m_ObjType==eGate)
       bnd = bcpd + rollingRadius;

   const bool inside = (bnd <= 0.f);									  // in ball inside object volume

   float hittime;
   if (rigid)
   {
      if ((bnd < -pball->m_radius/**2.0f*/) || (lateral && bcpd < 0.f))
         return -1.0f;	// (ball normal distance) excessive pentratration of object skin ... no collision HACK //!! *2 necessary?

      if (lateral && (bnd <= (float)PHYS_TOUCH))
      {
         if (inside || (fabsf(bnv) > C_CONTACTVEL)				// fast velocity, return zero time
            // zero time for rigid fast bodies				
            || (bnd <= (float)(-PHYS_TOUCH)))
            hittime = 0;										// slow moving but embedded
         else
            hittime = bnd
#ifdef NEW_PHYSICS
             / -bnv;
#else
             *(float)(1.0/(2.0*PHYS_TOUCH)) + 0.5f;	// don't compete for fast zero time events
#endif
      }
      else if (fabsf(bnv) > C_LOWNORMVEL) 					// not velocity low ????
         hittime = bnd / -bnv;								// rate ok for safe divide 
      else
         return -1.0f;										// wait for touching
   }
   else //non-rigid ... target hits
   {
      if (bnv * bnd >= 0.f)									// outside-receding || inside-approaching
      {
          if ((m_ObjType != eTrigger) ||						// not a trigger
              (!pball->m_vpVolObjs) ||
              (fabsf(bnd) >= pball->m_radius*0.5f) ||		    // not too close ... nor too far away
              (inside != (pball->m_vpVolObjs->IndexOf(m_pObj) < 0))) // ...ball outside and hit set or ball inside and no hit set
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
   const float btv = ballvx*normal.y - ballvy*normal.x;				 // ball velocity tangent to segment with respect to direction from V1 to V2
   const float btd = (ballx - v1.x)*normal.y - (bally - v1.y)*normal.x  // ball tangent distance 
      + btv * hittime;								     // ball tangent distance (projection) (initial position + velocity * hitime)

   if (btd < -C_TOL_ENDPNTS || btd > length + C_TOL_ENDPNTS) // is the contact off the line segment??? 
   {
       return -1.0f;
   }
   if (!rigid)												  // non rigid body collision? return direction
      coll.hitflag = bUnHit;			  // UnHit signal is receding from outside target

   const float ballr = pball->m_radius;
   const float hitz = pball->m_pos.z + pball->m_vel.z*hittime;  // check too high or low relative to ball rolling point at hittime

   if (hitz + ballr * 0.5f < m_rcHitRect.zlow				  // check limits of object's height and depth  
       || hitz - ballr * 0.5f > m_rcHitRect.zhigh)
   {
       return -1.0f;
   }
   coll.hitnormal.x = normal.x;		// hit normal is same as line segment normal
   coll.hitnormal.y = normal.y;
   coll.hitnormal.z = 0.0f;

   coll.hitdistance = bnd;				// actual contact distance ...
   //coll.hitRigid = rigid;				// collision type

   // check for contact
   if (fabsf(bnv) <= C_CONTACTVEL && fabsf(bnd) <= (float)PHYS_TOUCH)
   {
      coll.isContact = true;
      coll.hit_org_normalvelocity = bnv;
   }

   return hittime;
}

float LineSeg::HitTest(const Ball * const pball, const float dtime, CollisionEvent& coll)
{
   return HitTestBasic(pball, dtime, coll, true, true, true); // normal face, lateral, rigid
}

void LineSeg::Collide(CollisionEvent& coll)
{
   const float dot = coll.hitnormal.Dot(coll.ball->m_vel);
   coll.ball->Collide3DWall(coll.hitnormal, m_elasticity, m_elasticityFalloff, m_friction, m_scatter);

   if (dot <= -m_threshold)
      FireHitEvent(coll.ball);
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

void LineSeg::Contact(CollisionEvent& coll, const float dtime)
{
   coll.ball->HandleStaticContact(coll, m_friction, dtime);
}

////////////////////////////////////////////////////////////////////////////////

float HitCircle::HitTestBasicRadius(const Ball * const pball, const float dtime, CollisionEvent& coll,
   const bool direction, const bool lateral, const bool rigid)
{
   if (!m_fEnabled || pball->m_frozen) return -1.0f;

   Vertex3Ds c(center.x, center.y, 0.0f);
   Vertex3Ds dist = pball->m_pos - c;    // relative ball position
   Vertex3Ds dv = pball->m_vel;

   const bool capsule3D = (!lateral && pball->m_pos.z > m_rcHitRect.zhigh);

   float targetRadius;
   if (capsule3D)
   {
      // handle ball over target?
      //const float hcap = radius*(float)(1.0/5.0);           // cap height to hit-circle radius ratio
      //targetRadius = radius*radius/(hcap*2.0f) + hcap*0.5f; // c = (r^2+h^2)/(2*h)
      targetRadius = radius*(float)(13.0 / 5.0);              // optimized version of above code
      //c.z = m_rcHitRect.zhigh - (targetRadius - hcap);      // b = c - h
      c.z = m_rcHitRect.zhigh - radius*(float)(12.0 / 5.0);   // optimized version of above code
      dist.z = pball->m_pos.z - c.z;                          // ball rolling point - capsule center height 			
   }
   else
   {
      targetRadius = radius;
      if (lateral)
         targetRadius += pball->m_radius;
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
   if (m_ObjType == eKicker && bnd <= 0 && bnd >= -radius && a < C_CONTACTVEL*C_CONTACTVEL)
   {
      if (pball->m_vpVolObjs) pball->m_vpVolObjs->RemoveElement(m_pObj);	// cause capture
   }

   if (rigid && bnd < (float)PHYS_TOUCH)		// positive: contact possible in future ... Negative: objects in contact now
   {
      if (bnd < -pball->m_radius/**2.0f*/) //!! *2 necessary?
         return -1.0f;
      else if (fabsf(bnv) <= C_CONTACTVEL)
      {
         isContact = true;
      }
      else
         hittime = /*std::max(0.0f,*/ -bnd / bnv /*)*/;   // estimate based on distance and speed along distance
   }
   else if (m_ObjType >= eTrigger // triggers & kickers
      && pball->m_vpVolObjs && ((bnd < 0.f) == (pball->m_vpVolObjs->IndexOf(m_pObj) < 0)))
   { // here if ... ball inside and no hit set .... or ... ball outside and hit set

      if (fabsf(bnd - radius) < 0.05f)	 // if ball appears in center of trigger, then assumed it was gen'ed there
      {
         if (pball->m_vpVolObjs)
            pball->m_vpVolObjs->AddElement(m_pObj);	//special case for trigger overlaying a kicker
      }												// this will add the ball to the trigger space without a Hit
      else
      {
         bUnhit = (bnd > 0.f);	// ball on outside is UnHit, otherwise it's a Hit
      }
   }
   else
   {
      if ((!rigid && bnd * bnv > 0.f) ||	// (outside and receding) or (inside and approaching)
         (a < 1.0e-8f)) return -1.0f;	    // no hit ... ball not moving relative to object

      float time1, time2;
      if (!SolveQuadraticEq(a, 2.0f*b, bcddsq - targetRadius*targetRadius, time1, time2))
         return -1.0f;

      bUnhit = (time1*time2 < 0.f);
      hittime = bUnhit ? max(time1, time2) : min(time1, time2); // ball is inside the circle
   }

   if (infNaN(hittime) || hittime < 0.f || hittime > dtime)
      return -1.0f; // contact out of physics frame

   const float hitz = pball->m_pos.z + pball->m_vel.z * hittime; // rolling point

   if (((hitz + pball->m_radius*0.5f) < m_rcHitRect.zlow) ||
      (!capsule3D && (hitz - pball->m_radius*0.5f) > m_rcHitRect.zhigh) ||
      (capsule3D && hitz < m_rcHitRect.zhigh)) return -1.0f;

   const float hitx = pball->m_pos.x + pball->m_vel.x*hittime;
   const float hity = pball->m_pos.y + pball->m_vel.y*hittime;

   const float sqrlen = (hitx - c.x)*(hitx - c.x) + (hity - c.y)*(hity - c.y);

   if (sqrlen > 1.0e-8f) // over center???
   { // no
      const float inv_len = 1.0f / sqrtf(sqrlen);
      coll.hitnormal.x = (hitx - c.x)*inv_len;
      coll.hitnormal.y = (hity - c.y)*inv_len;
      coll.hitnormal.z = 0.0f;
   }
   else
   { // yes, over center
      coll.hitnormal.x = 0.0f; // make up a value, any direction is ok
      coll.hitnormal.y = 1.0f;
      coll.hitnormal.z = 0.0f;
   }

   if (!rigid)                 // non rigid body collision? return direction
      coll.hitflag = bUnhit;   // UnHit signal	is receding from target

   coll.isContact = isContact;
   if (isContact)
      coll.hit_org_normalvelocity = bnv;

   coll.hitdistance = bnd;     //actual contact distance ... 
   //coll.hitRigid = rigid;    // collision type

   return hittime;
}

void HitCircle::CalcHitRect()
{
   // Allow roundoff
   m_rcHitRect.left = center.x - radius;
   m_rcHitRect.right = center.x + radius;
   m_rcHitRect.top = center.y - radius;
   m_rcHitRect.bottom = center.y + radius;

   // zlow & zhigh already set in ctor
}

float HitCircle::HitTest(const Ball * const pball, const float dtime, CollisionEvent& coll)
{
	//normal face, lateral, rigid
	return HitTestBasicRadius(pball, dtime, coll, true, true, true);
}

void HitCircle::Collide(CollisionEvent& coll)
{
   coll.ball->Collide3DWall(coll.hitnormal, m_elasticity, m_elasticityFalloff, m_friction, m_scatter);
}

void HitCircle::Contact(CollisionEvent& coll, const float dtime)
{
   coll.ball->HandleStaticContact(coll, m_friction, dtime);
}


///////////////////////////////////////////////////////////////////////////////


float HitLineZ::HitTest(const Ball * const pball, const float dtime, CollisionEvent& coll)
{
   if (!m_fEnabled)
      return -1.0f;

   const Vertex2D bp2d(pball->m_pos.x, pball->m_pos.y);
   const Vertex2D dist = bp2d - m_xy;    // relative ball position
   const Vertex2D dv(pball->m_vel.x, pball->m_vel.y);

   const float bcddsq = dist.LengthSquared();  // ball center to line distance squared
   const float bcdd = sqrtf(bcddsq);           // distance ball to line
   if (bcdd <= 1.0e-6f)
      return -1.0f;                           // no hit on exact center

   const float b = dist.Dot(dv);
   const float bnv = b / bcdd;                   // ball normal velocity

   if (bnv > C_CONTACTVEL)
      return -1.0f;                           // clearly receding from radius

   const float bnd = bcdd - pball->m_radius;   // ball distance to line

   const float a = dv.LengthSquared();

   float hittime = 0;
   bool isContact = false;

   if (bnd < (float)PHYS_TOUCH)       // already in collision distance?
   {
      if (fabsf(bnv) <= C_CONTACTVEL)
      {
         isContact = true;
         hittime = 0;
      }
      else
         hittime = /*std::max(0.0f,*/ -bnd / bnv /*)*/;   // estimate based on distance and speed along distance
   }
   else
   {
      if (a < 1.0e-8f)
         return -1.0f;    // no hit - ball not moving relative to object

      float time1, time2;
      if (!SolveQuadraticEq(a, 2.0f*b, bcddsq - pball->m_radius*pball->m_radius, time1, time2))
         return -1.0f;

      hittime = (time1*time2 < 0.f) ? max(time1, time2) : min(time1, time2); // find smallest nonnegative solution
   }

   if (infNaN(hittime) || hittime < 0 || hittime > dtime)
      return -1.0f; // contact out of physics frame

   const float hitz = pball->m_pos.z + hittime * pball->m_vel.z;   // ball z position at hit time

   if (hitz < m_rcHitRect.zlow || hitz > m_rcHitRect.zhigh)    // check z coordinate
      return -1.0f;

   const float hitx = pball->m_pos.x + hittime * pball->m_vel.x;   // ball x position at hit time
   const float hity = pball->m_pos.y + hittime * pball->m_vel.y;   // ball y position at hit time

   Vertex2D norm(hitx - m_xy.x, hity - m_xy.y);
   norm.Normalize();
   coll.hitnormal.Set(norm.x, norm.y, 0.0f);

   coll.isContact = isContact;
   if (isContact)
      coll.hit_org_normalvelocity = bnv;

   coll.hitdistance = bnd;                    // actual contact distance
   //coll.hitRigid = true;

   return hittime;
}

void HitLineZ::CalcHitRect()
{
   m_rcHitRect.left = m_xy.x;
   m_rcHitRect.right = m_xy.x;
   m_rcHitRect.top = m_xy.y;
   m_rcHitRect.bottom = m_xy.y;

   // zlow and zhigh set in ctor
}

void HitLineZ::Collide(CollisionEvent& coll)
{
   const float dot = coll.hitnormal.Dot(coll.ball->m_vel);
   coll.ball->Collide3DWall(coll.hitnormal, m_elasticity, m_elasticityFalloff, m_friction, m_scatter);

   if (dot <= -m_threshold)
      FireHitEvent(coll.ball);
}

void HitLineZ::Contact(CollisionEvent& coll, const float dtime)
{
   coll.ball->HandleStaticContact(coll, m_friction, dtime);
}


///////////////////////////////////////////////////////////////////////////////


float HitPoint::HitTest(const Ball * const pball, const float dtime, CollisionEvent& coll)
{
   if (!m_fEnabled)
      return -1.0f;

   const Vertex3Ds dist = pball->m_pos - m_p;  // relative ball position

   const float bcddsq = dist.LengthSquared();  // ball center to line distance squared
   const float bcdd = sqrtf(bcddsq);           // distance ball to line
   if (bcdd <= 1.0e-6f)
      return -1.0f;                           // no hit on exact center

   const float b = dist.Dot(pball->m_vel);
   const float bnv = b / bcdd;                   // ball normal velocity

   if (bnv > C_CONTACTVEL)
      return -1.0f;                           // clearly receding from radius

   const float bnd = bcdd - pball->m_radius;   // ball distance to line

   const float a = pball->m_vel.LengthSquared();

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
         hittime = std::max(0.0f, -bnd / bnv );
#endif
   }
   else
   {
      if (a < 1.0e-8f)
         return -1.0f;    // no hit - ball not moving relative to object

      float time1, time2;
      if (!SolveQuadraticEq(a, 2.0f*b, bcddsq - pball->m_radius*pball->m_radius, time1, time2))
         return -1.0f;

      hittime = (time1*time2 < 0.f) ? max(time1, time2) : min(time1, time2); // find smallest nonnegative solution
   }

   if (infNaN(hittime) || hittime < 0 || hittime > dtime)
      return -1.0f; // contact out of physics frame

   const Vertex3Ds hitPos = pball->m_pos + hittime * pball->m_vel;
   coll.hitnormal = hitPos - m_p;
   coll.hitnormal.Normalize();

   coll.isContact = isContact;
   if (isContact)
      coll.hit_org_normalvelocity = bnv;

   coll.hitdistance = bnd;                    // actual contact distance
   //coll.hitRigid = true;

   return hittime;
}

void HitPoint::CalcHitRect()
{
   m_rcHitRect = FRect3D(m_p.x, m_p.x, m_p.y, m_p.y, m_p.z, m_p.z);
}

void HitPoint::Collide(CollisionEvent& coll)
{
   const float dot = coll.hitnormal.Dot(coll.ball->m_vel);
   coll.ball->Collide3DWall(coll.hitnormal, m_elasticity, m_elasticityFalloff, m_friction, m_scatter);

   if (dot <= -m_threshold)
      FireHitEvent(coll.ball);
}

void HitPoint::Contact(CollisionEvent& coll, const float dtime)
{
   coll.ball->HandleStaticContact(coll, m_friction, dtime);
}

void DoHitTest(Ball *const pball, HitObject *const pho, CollisionEvent& coll)
{
#ifdef _DEBUGPHYSICS
   g_pplayer->c_deepTested++;
#endif
   if (pho == NULL || pball==NULL)
      return;

   if (pho->m_ObjType == eHitTarget)
   {
      if ( pho->m_objHitEvent && (((HitTarget*)pho->m_objHitEvent)->m_d.m_isDropped == true) )
         return;
   }

   CollisionEvent newColl;
   const float newtime = pho->HitTest(pball, coll.hittime, !g_pplayer->m_fRecordContacts ? coll : newColl);
   const bool validhit = ((newtime >= 0.f) && !sign(newtime) && (newtime <= coll.hittime));

   if (!g_pplayer->m_fRecordContacts) // simply find first event
   {
      if (validhit)
      {
         coll.ball = pball;
         coll.obj = pho;
         coll.hittime = newtime;
      }
   }
   else // find first collision, but also remember all contacts
   {
      if (newColl.isContact || validhit)
      {
         newColl.ball = pball;
         newColl.obj = pho;
		
		 if (newColl.isContact)
			 g_pplayer->m_contacts.push_back(newColl);
		 else //if (validhit)
		 {
			 coll = newColl;
			 coll.hittime = newtime;
		 }
	  }
   }
}
