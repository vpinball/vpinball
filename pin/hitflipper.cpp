#include "stdafx.h"

FlipperAnimObject::FlipperAnimObject(const Vertex2D& center, float baser, float endr, float flipr, float angleStart, float angleEnd,
                      float zlow, float zhigh, float strength, float mass, float returnRatio)
{
   m_height = zhigh - zlow;

   m_hitcircleBase.m_pfe = NULL;

   m_hitcircleBase.zlow = zlow;
   m_hitcircleBase.zhigh = zhigh;

   m_hitcircleBase.center = center;

   if (baser < 0.01f) baser = 0.01f; // must not be zero 
   m_hitcircleBase.radius = baser;   //radius of base section

   if (endr < 0.01f) endr = 0.01f; // must not be zero 
   m_endradius = endr;             // radius of flipper end

   if (flipr < 0.01f) flipr = 0.01f; // must not be zero 
   m_flipperradius = flipr; //radius of flipper arc, center-to-center radius

   m_dir = (angleEnd >= angleStart) ? 1 : -1;
   m_solState = false;
   m_isInContact = false;
   m_curTorque = 0.0f;
   m_torqueRampupSpeed = 1e6f;

   m_angleStart = angleStart;
   m_angleEnd   = angleEnd;
   m_angleMin   = min(angleStart, angleEnd);
   m_angleMax   = max(angleStart, angleEnd);
   m_angleCur   = angleStart;

   m_angularMomentum = 0;
   m_angularAcceleration = 0;
   m_anglespeed = 0;

   const float fa = asinf((baser-endr)/flipr); //face to centerline angle (center to center)

   const float faceNormOffset = (float)(M_PI/2.0) - fa; //angle of normal when flipper center line at angle zero

   m_force = strength;
   m_returnRatio = returnRatio;

   // model inertia of flipper as that of rod of length flipr around its end
   m_inertia = (float)(1.0/3.0) * mass * (flipr*flipr);

   m_lastHitFace = false; // used to optimize hit face search order

   m_faceLength = m_flipperradius*cosf(fa); //Cosine of face angle X hypotenuse

   zeroAngNorm.x =  sinf(faceNormOffset);// F2 Norm, used in Green's transform, in FPM time search
   zeroAngNorm.y = -cosf(faceNormOffset);// F1 norm, change sign of x component, i.e -zeroAngNorm.x

#if 0 // needs wiring of moment of inertia
   // now calculate moment of inertia using isoceles trapizoid and two circular sections
   // ISOSCELES TRAPEZOID, Area Moment of Inertia
   // I(area)FF = h/(144*(a+b)*(16*h^2*a*b+4*h^2*b^2+4*h^2*a^2+3*a^4+6*a^2*b^2+6*a^3*b+6*a*b^3+3*b^4)) (centroidial axis)
   // circular sections, Area Moment of Inertia
   // I(area)FB = rb^4/4*(theta - sin(theta)+2/3*sin(theta)*sin(theta/2)^2), where rb is flipper base radius
   // I(area)FE = re^4/4*(theta - sin(theta)+2/3*sin(theta)*sin(theta/2)^2),requires translation to centroidial axis
   // then translate these using the parallel axis theorem to the flipper rotational axis

   const float etheta = (float)M_PI - (fa+fa); // end radius section angle
   const float btheta = (float)M_PI + (fa+fa);	// base radius section angle
   const float tmp1 = sinf(btheta*0.5f);
   const float tmp2 = sinf(etheta*0.5f);
   const float a = 2.0f*endr*tmp2; 
   const float b = 2.0f*baser*tmp1; // face thickness at end and base radii

   const float baseh = baser*cosf(btheta*0.5f);
   const float endh = endr*cosf(etheta*0.5f);
   const float h = flipr + baseh + endh;

   float Irb_inertia = (baser*baser)*(baser*baser)*0.25f*(btheta - sinf(btheta) + (float)(2.0/3.0)*sinf(btheta)*tmp1*tmp1);//base radius
   Irb_inertia /= baser*baser*(btheta - sinf(btheta)); // divide by area to obtain simple Inertia

   float Ire_inertia = (endr*endr)*(endr*endr)*0.25f*(etheta - sinf(etheta) + (float)(2.0/3.0)*sinf(etheta)*(tmp2*tmp2));//end radius
   Ire_inertia /= endr*endr*(etheta - sinf(etheta)); // divide by area

   // translate to centroidal and then flipper axis.. subtract section radius squared then add (flipper radius + section radius) squared
   const float tmp3 = (float)(4.0/3.0)*endr*(tmp2*tmp2)*tmp2/(etheta-sinf(etheta));
   Ire_inertia = Ire_inertia + ((flipr+tmp3)*(flipr+tmp3)
      -       tmp3 *       tmp3); // double parallel axis

   //flipper body trapizoidal section
   float Ifb_inertia = h/(144.0f*(a+b))*(16.0f*(h*h)*a*b + 4.0f*(h*h)*((b*b)+(a*a)) + 3.0f*(a*a)*(a*a)
      + 6.0f*(a*a)*(b*b) + 6.0f*(a*b)*((b*b)+(a*a)) + 3.0f*(b*b)*(b*b));
   Ifb_inertia /= h*0.5f*(a+b); // divide by area

   const float tmp4 = h*(float)(1.0/3.0)*(a+(a+b))/(a+b);
   Ifb_inertia = Ifb_inertia + tmp4*tmp4; //flipper body translated to flipper axis ...parallel axis

   const float Iff = Irb_inertia + Ifb_inertia + Ire_inertia; //scalar moment of inertia ... multiply by weight next

   m_inertia = Iff * mass;  //mass of flipper body

   //m_inertia = mass;  //stubbed to mass of flipper body
#endif
}

HitFlipper::HitFlipper(const Vertex2D& center, float baser, float endr, float flipr, float angleStart, float angleEnd,
		       float zlow, float zhigh, float strength, float mass, float returnRatio)
    : m_flipperanim(center, baser, endr, flipr, angleStart, angleEnd, zlow, zhigh, strength, mass, returnRatio)
{
    m_last_hittime = 0;
}

HitFlipper::~HitFlipper()
{
   //m_pflipper->m_phitflipper = NULL;
}

void HitFlipper::CalcHitRect()
{
   // Allow roundoff
   m_rcHitRect.left = m_flipperanim.m_hitcircleBase.center.x - m_flipperanim.m_flipperradius - m_flipperanim.m_endradius - 0.1f;
   m_rcHitRect.right = m_flipperanim.m_hitcircleBase.center.x + m_flipperanim.m_flipperradius + m_flipperanim.m_endradius + 0.1f;
   m_rcHitRect.top = m_flipperanim.m_hitcircleBase.center.y - m_flipperanim.m_flipperradius - m_flipperanim.m_endradius - 0.1f;
   m_rcHitRect.bottom = m_flipperanim.m_hitcircleBase.center.y + m_flipperanim.m_flipperradius + m_flipperanim.m_endradius + 0.1f;
   m_rcHitRect.zlow = m_flipperanim.m_hitcircleBase.zlow;
   m_rcHitRect.zhigh = m_flipperanim.m_hitcircleBase.zhigh;
}

void FlipperAnimObject::SetReturnRatio(const float r)
{
    m_returnRatio = r;
}

void FlipperAnimObject::SetMass(const float m)
{
    m_inertia = (float)(1.0/3.0) * m * (m_flipperradius*m_flipperradius); //!! also change if wiring of moment of inertia happens (see ctor)
}

void FlipperAnimObject::SetStrength(const float s)
{
    m_force = s;
}

void FlipperAnimObject::UpdateDisplacements(const float dtime)
{
   m_angleCur += m_anglespeed*dtime;	// move flipper angle

   //if (m_anglespeed)
   //    slintf("Ang.speed: %f\n", m_anglespeed);

   if (m_angleCur >= m_angleMax)        // hit stop?
   {
      //m_angleCur = m_angleMax;

      if (m_anglespeed > 0.f) 
      {
#ifdef DEBUG_FLIPPERS
         if (m_startTime)
         {
             U32 dur = g_pplayer->m_time_msec - m_startTime;
             m_startTime = 0;
             slintf("Stroke duration: %u ms\nAng. velocity: %f\n", dur, m_anglespeed);
             slintf("Ball velocity: %f\n", g_pplayer->m_vball[0]->vel.Length());
         }
#endif

         const float anglespd = fabsf(RADTOANG(m_anglespeed));
         m_angularMomentum *= -0.3f;
         m_anglespeed = m_angularMomentum / m_inertia;

         if (m_EnableRotateEvent > 0) m_pflipper->FireVoidEventParm(DISPID_LimitEvents_EOS,anglespd); // send EOS event
         else if (m_EnableRotateEvent < 0) m_pflipper->FireVoidEventParm(DISPID_LimitEvents_BOS, anglespd);	// send Beginning of Stroke event
         m_EnableRotateEvent = 0;
      }
   }
   else if (m_angleCur <= m_angleMin)
   {
      //m_angleCur = m_angleMin;

      if (m_anglespeed < 0.f)
      {
         const float anglespd = fabsf(RADTOANG(m_anglespeed));
         m_angularMomentum *= -0.3f;
         m_anglespeed = m_angularMomentum / m_inertia;

         if (m_EnableRotateEvent > 0) m_pflipper->FireVoidEventParm(DISPID_LimitEvents_EOS,anglespd); // send EOS event
         else if (m_EnableRotateEvent < 0) m_pflipper->FireVoidEventParm(DISPID_LimitEvents_BOS, anglespd);	// send Park event
         m_EnableRotateEvent = 0;
      }
   }
}

void FlipperAnimObject::UpdateVelocities()
{
    //const float springDispl = GetStrokeRatio() * 0.5f + 0.5f; // range: [0.5 .. 1]
    //const float springForce = -0.6f * springDispl * m_force;
    //const float solForce = m_solState ? m_force : 0.0f;
    //float force = m_dir * (solForce + springForce);

    float desiredTorque = 0;
    if (m_solState)
    {
        desiredTorque = m_force;
        if (fabsf(m_angleCur - m_angleEnd) <= (float)(3.0*M_PI/180.0))
            desiredTorque *= 0.75f;     // hold coil is weaker
    }
    else
        desiredTorque = -m_returnRatio * m_force;

    desiredTorque *= m_dir;

    // update current torque linearly towards desired torque
    // (simple model for coil hysteresis)
    if (desiredTorque >= m_curTorque)
        m_curTorque = std::min(m_curTorque + m_torqueRampupSpeed * (float)PHYS_FACTOR, desiredTorque);
    else
        m_curTorque = std::max(m_curTorque - m_torqueRampupSpeed * (float)PHYS_FACTOR, desiredTorque);

    // resolve contacts with stoppers
    float torque = m_curTorque;
    m_isInContact = false;
    if (fabsf(m_anglespeed) <= 1e-2f)
    {
        if (m_angleCur >= m_angleMax - 1e-2f && torque > 0)
        {
            m_isInContact = true;
            m_contactTorque = torque;
            m_angularMomentum = 0;
            torque = 0;
        }
        else if (m_angleCur <= m_angleMin + 1e-2f && torque < 0)
        {
            m_isInContact = true;
            m_contactTorque = torque;
            m_angularMomentum = 0;
            torque = 0;
        }
    }

    m_angularMomentum += (float)PHYS_FACTOR * torque;
    m_anglespeed = m_angularMomentum / m_inertia;
    m_angularAcceleration = torque / m_inertia;
}

void FlipperAnimObject::ApplyImpulse(const Vertex3Ds& surfP, const Vertex3Ds& impulse)
{
    const Vertex3Ds rotI = CrossProduct(surfP, impulse);
    m_angularMomentum += rotI.z;            // only rotation about z axis
    m_anglespeed = m_angularMomentum / m_inertia;    // TODO: figure out moment of inertia
}


void FlipperAnimObject::SetSolenoidState(bool s)
{
    m_solState = s;
#ifdef DEBUG_FLIPPERS
    if (m_angleCur == m_angleStart)
        m_startTime = g_pplayer->m_time_msec;
#endif
}

float FlipperAnimObject::GetStrokeRatio() const
{
    return (m_angleCur - m_angleStart) / (m_angleEnd - m_angleStart);
}

// compute the cross product (0,0,rz) x v
static inline Vertex3Ds CrossZ(float rz, const Vertex3Ds& v)
{
    return Vertex3Ds( -rz * v.y, rz * v.x, 0 );
}

Vertex3Ds FlipperAnimObject::SurfaceVelocity(const Vertex3Ds& surfP) const
{
    //const Vertex3Ds angularVelocity(0, 0, m_anglespeed);
    //return CrossProduct( angularVelocity, surfP );
    // equivalent:
    return CrossZ(m_anglespeed, surfP);
}

Vertex3Ds FlipperAnimObject::SurfaceAcceleration(const Vertex3Ds& surfP) const
{
    // tangential acceleration = (0, 0, omega) x surfP
    const Vertex3Ds tangAcc = CrossZ(m_angularAcceleration, surfP);

    // centripetal acceleration = (0,0,omega) x ( (0,0,omega) x surfP )
    const float av2 = m_anglespeed * m_anglespeed;
    const Vertex3Ds centrAcc( -av2 * surfP.x, -av2 * surfP.y, 0 );

    return tangAcc + centrAcc;
}

float FlipperAnimObject::GetHitTime() const
{
    if (m_anglespeed == 0)
        return -1.0f;

    const float dist = (m_anglespeed > 0)
        ? m_angleMax - m_angleCur       // >= 0
        : m_angleMin - m_angleCur;      // <= 0

    const float hittime = dist / m_anglespeed;

    if (infNaN(hittime) || hittime < 0)
        return -1.0f;
    else
        return hittime;
}

float HitFlipper::GetHitTime() const
{
    return m_flipperanim.GetHitTime();
}

#define LeftFace 1
#define RightFace 0

float HitFlipper::HitTest(const Ball * pball, float dtime, CollisionEvent& coll)
{
   if (!m_flipperanim.m_fEnabled) return -1;

   const bool lastface = m_flipperanim.m_lastHitFace;

   // for effective computing, adding a last face hit value to speed calculations 
   //  a ball can only hit one face never two
   // also if a ball hits a face then it can not hit either radius
   // so only check these if a face is not hit
   // endRadius is more likely than baseRadius ... so check it first

   float hittime = HitTestFlipperFace(pball, dtime, coll, lastface); // first face
   if (hittime >= 0) return hittime;		

   hittime = HitTestFlipperFace(pball, dtime, coll, !lastface); //second face
   if (hittime >= 0)
   {
      m_flipperanim.m_lastHitFace = !lastface;	// change this face to check first
      return hittime;
   }

   hittime = HitTestFlipperEnd(pball, dtime, coll); // end radius
   if (hittime >= 0)
      return hittime;

   hittime = m_flipperanim.m_hitcircleBase.HitTest(pball, dtime, coll);
   if (hittime >= 0)
   {		
      coll.hitvelocity.x = 0;		//Tangent velocity of contact point (rotate Normal right)
      coll.hitvelocity.y = 0;		//units: rad*d/t (Radians*diameter/time

      coll.hitmoment = 0;			//moment is zero ... only friction
       //!! unused coll.hitangularrate = 0;		//radians/time at collison

      // Flipper::Contact() expects origNormVel,
      // but HitCircle puts it in normal[1].z
      if (coll.isContact)
          origNormVel = coll.hitvelocity.z;

      return hittime;
   }

   return -1.0f;	// no hits
}

float HitFlipper::HitTestFlipperEnd(const Ball * pball, const float dtime, CollisionEvent& coll) // replacement
{ 	 
   const float angleCur = m_flipperanim.m_angleCur;
   float anglespeed = m_flipperanim.m_anglespeed;		// rotation rate

   const Vertex2D flipperbase = m_flipperanim.m_hitcircleBase.center;

   const float angleMin = m_flipperanim.m_angleMin;
   const float angleMax = m_flipperanim.m_angleMax;

   const float ballr = pball->m_radius;
   const float feRadius = m_flipperanim.m_endradius;

   const float ballrEndr = feRadius + ballr;			// magnititude of (ball - flipperEnd)

   const float ballx = pball->m_pos.x;
   const float bally = pball->m_pos.y;

   const float ballvx = pball->m_vel.x;
   const float ballvy = pball->m_vel.y;

   const Vertex2D vp(0.0f,                            //m_flipperradius*sin(0));
      -m_flipperanim.m_flipperradius); //m_flipperradius*(-cos(0));

   float ballvtx, ballvty;	// new ball position at time t in flipper face coordinate
   float contactAng;
   float bfend, cbcedist;
   float t0,t1, d0,d1,dp; // Modified False Position control

   float t = 0; //start first interval ++++++++++++++++++++++++++
   int k;
   for (k=1;k <= C_INTERATIONS;++k)
   {
      // determine flipper rotation direction, limits and parking 

      contactAng = angleCur + anglespeed * t;					// angle at time t

      if (contactAng >= angleMax) contactAng = angleMax;		// stop here			
      else if (contactAng <= angleMin) contactAng = angleMin;	// stop here 

      const float radsin = sinf(contactAng);// Green's transform matrix... rotate angle delta 
      const float radcos = cosf(contactAng);// rotational transform from zero position to position at time t

      //rotate angle delta unit vector, rotates system according to flipper face angle
      const Vertex2D vt(
         vp.x *radcos - vp.y *radsin + flipperbase.x,		//rotate and translate to world position
         vp.y *radcos + vp.x *radsin + flipperbase.y);

      ballvtx = ballx + ballvx*t - vt.x;						// new ball position relative to flipper end radius
      ballvty = bally + ballvy*t - vt.y;

      cbcedist = sqrtf(ballvtx*ballvtx + ballvty*ballvty);	// center ball to center end radius distance

      bfend = cbcedist - ballrEndr;							// ball face-to-radius surface distance

      if (fabsf(bfend) <= C_PRECISION) break; 

      if (k == 1)   // end of pass one ... set full interval pass, t = dtime
      { // test for extreme conditions
         if (bfend < -(pball->m_radius + feRadius)) return -1.0f; // too deeply embedded, ambigious position
         if (bfend <= (float)PHYS_TOUCH) 
            break; // inside the clearance limits

         t0 = t1 = dtime; d0 = 0; d1 = bfend; // set for second pass, force t=dtime
      }
      else if (k == 2) // end pass two, check if zero crossing on initial interval, exit if none
      {
         if (dp*bfend > 0.0f) return -1.0f;	// no solution ... no obvious zero crossing

         t0 = 0; t1 = dtime; d0 = dp; d1 = bfend; // set initial boundaries
      }
      else // (k >= 3) // MFP root search +++++++++++++++++++++++++++++++++++++++++
      {
         if (bfend*d0 <= 0.0f)										// zero crossing
         { t1 = t; d1 = bfend; if (dp*bfend > 0.0) d0 *= 0.5f; } // 	move right interval limit			
         else 
         { t0 = t; d0 = bfend; if (dp*bfend > 0.0) d1 *= 0.5f; }	// 	move left interval limit		
      }		

      t = t0 - d0*(t1 - t0)/(d1 - d0);			// estimate next t
      dp = bfend;									// remember 

   }//for loop
   //+++ End time interation loop found time t soultion ++++++

   if (t < 0 || t > dtime							// time is outside this frame ... no collision
      ||
      ((k > C_INTERATIONS) && (fabsf(bfend) > pball->m_radius*0.25f))) // last ditch effort to accept a solution
      return -1.0f; // no solution

   // here ball and flipper end are in contact .. well in most cases, near and embedded solutions need calculations	

   const float hitz = pball->m_pos.z - ballr + pball->m_vel.z*t; // check for a hole, relative to ball rolling point at hittime

   if ((hitz + (ballr * 1.5f)) < m_rcHitRect.zlow		//check limits of object's height and depth
      || (hitz + (ballr * 0.5f)) > m_rcHitRect.zhigh)
      return -1.0f;

   // ok we have a confirmed contact, calc the stats, remember there are "near" solution, so all
   // parameters need to be calculated from the actual configuration, i.e. contact radius must be calc'ed

   const float inv_cbcedist = 1.0f/cbcedist;
   coll.hitnormal.x = ballvtx*inv_cbcedist;				// normal vector from flipper end to ball
   coll.hitnormal.y = ballvty*inv_cbcedist;
   coll.hitnormal.z = 0.0f;

   const Vertex2D dist(
      (pball->m_pos.x + ballvx*t - ballr*coll.hitnormal.x - m_flipperanim.m_hitcircleBase.center.x), // vector from base to flipperEnd plus the projected End radius
      (pball->m_pos.y + ballvy*t - ballr*coll.hitnormal.y - m_flipperanim.m_hitcircleBase.center.y));

   const float distance = sqrtf(dist.x*dist.x + dist.y*dist.y);	// distance from base center to contact point

   if ((contactAng >= angleMax && anglespeed > 0) || (contactAng <= angleMin && anglespeed < 0))	// hit limits ??? 
      anglespeed = 0;							// rotation stopped

   const float inv_distance = 1.0f/distance;
   coll.hitvelocity.x = -dist.y*inv_distance; //Unit Tangent vector velocity of contact point(rotate normal right)
   coll.hitvelocity.y =  dist.x*inv_distance;

   coll.hitmoment = distance;				//moment arm diameter
    //!! unused coll.hitangularrate = anglespeed;		//radians/time at collison

   //recheck using actual contact angle of velocity direction
   const Vertex2D dv(
      (ballvx - coll.hitvelocity.x *anglespeed*distance), 
      (ballvy - coll.hitvelocity.y *anglespeed*distance)); //delta velocity ball to face

   const float bnv = dv.x*coll.hitnormal.x + dv.y*coll.hitnormal.y;  //dot Normal to delta v

   if (fabsf(bnv) <= C_CONTACTVEL && bfend <= (float)PHYS_TOUCH)
   {
       coll.isContact = true;
       origNormVel = bnv;
   }
   if (bnv >= 0) 
      return -1.0f; // not hit ... ball is receding from face already, must have been embedded or shallow angled

   coll.hitdistance = bfend;			//actual contact distance ..
   //coll.hitRigid = true;				// collision type

   return t;
}


float HitFlipper::HitTestFlipperFace(const Ball * pball, const float dtime, CollisionEvent& coll, const bool face)
{ 
   const float angleCur = m_flipperanim.m_angleCur;
   float anglespeed = m_flipperanim.m_anglespeed;				// rotation rate

   const Vertex2D flipperbase = m_flipperanim.m_hitcircleBase.center;
   const float feRadius = m_flipperanim.m_endradius;

   const float angleMin = m_flipperanim.m_angleMin;
   const float angleMax = m_flipperanim.m_angleMax;	

   const float ballr = pball->m_radius;	
   const float ballvx = pball->m_vel.x;
   const float ballvy = pball->m_vel.y;	

   // flipper positions at zero degrees rotation

   float ffnx = m_flipperanim.zeroAngNorm.x; // flipper face normal vector //Face2 
   if (face == LeftFace) ffnx = -ffnx;		  // negative for face1

   const float ffny = m_flipperanim.zeroAngNorm.y;	  // norm y component same for either face
   const Vertex2D vp(									  // face segment V1 point
      m_flipperanim.m_hitcircleBase.radius*ffnx, // face endpoint of line segment on base radius
      m_flipperanim.m_hitcircleBase.radius*ffny);		

   Vertex2D F;			// flipper face normal

   float bffnd;		// ball flipper face normal distance (negative for normal side)
   float ballvtx, ballvty;		// new ball position at time t in flipper face coordinate
   float contactAng;

   float t,t0,t1, d0,d1,dp; // Modified False Position control

   t = 0; //start first interval ++++++++++++++++++++++++++
   int k;
   for (k=1; k<=C_INTERATIONS; ++k)
   {
      // determine flipper rotation direction, limits and parking 	

      contactAng = angleCur + anglespeed * t;					// angle at time t

      if (contactAng >= angleMax) contactAng = angleMax;			// stop here			
      else if (contactAng <= angleMin) contactAng = angleMin;		// stop here 

      const float radsin = sinf(contactAng);//  Green's transform matrix... rotate angle delta 
      const float radcos = cosf(contactAng);//  rotational transform from current position to position at time t

      F.x = ffnx *radcos - ffny *radsin;  // rotate to time t, norm and face offset point
      F.y = ffny *radcos + ffnx *radsin;  // 

      const Vertex2D vt(
         vp.x *radcos - vp.y *radsin + flipperbase.x, //rotate and translate to world position
         vp.y *radcos + vp.x *radsin + flipperbase.y);

      ballvtx = pball->m_pos.x + ballvx*t - vt.x;	// new ball position relative to rotated line segment endpoint
      ballvty = pball->m_pos.y + ballvy*t - vt.y;	

      bffnd = ballvtx *F.x +  ballvty *F.y - ballr; // normal distance to segment 

      if (fabsf(bffnd) <= C_PRECISION) break;

      // loop control, boundary checks, next estimate, etc.

      if (k == 1)   // end of pass one ... set full interval pass, t = dtime
      {    // test for already inside flipper plane, either embedded or beyond the face endpoints
         if (bffnd < -(pball->m_radius + feRadius)) return -1.0f; // wrong side of face, or too deeply embedded			
         if (bffnd <= (float)PHYS_TOUCH) 
            break; // inside the clearance limits, go check face endpoints

         t0 = t1 = dtime; d0 = 0; d1 = bffnd; // set for second pass, so t=dtime
      }
      else if (k == 2)// end pass two, check if zero crossing on initial interval, exit
      {	
         if (dp*bffnd > 0.0) return -1.0f;	// no solution ... no obvious zero crossing
         t0 = 0; t1 = dtime; d0 = dp; d1 = bffnd; // testing MFP estimates			
      }
      else // (k >= 3) // MFP root search +++++++++++++++++++++++++++++++++++++++++
      {
         if (bffnd*d0 <= 0.0)									// zero crossing
         { t1 = t; d1 = bffnd; if (dp*bffnd > 0.0) d0 *= 0.5f; } // 	move right limits
         else 
         { t0 = t; d0 = bffnd; if (dp*bffnd > 0.0) d1 *= 0.5f; } // move left limits
      }		

      t = t0 - d0*(t1-t0)/(d1-d0);					// next estimate
      dp = bffnd;	// remember 
   }//for loop

   //+++ End time interation loop found time t soultion ++++++

   if (t < 0 || t > dtime								// time is outside this frame ... no collision
      ||
      ((k > C_INTERATIONS) && (fabsf(bffnd) > pball->m_radius*0.25f))) // last ditch effort to accept a near solution
      return -1.0f; // no solution

   // here ball and flipper face are in contact... past the endpoints, also, don't forget embedded and near soultion

   Vertex2D T;			// flipper face tangent
   if (face == LeftFace) 
   { T.x = -F.y; T.y = F.x; }	// rotate to form Tangent vector				
   else
   { T.x = F.y; T.y = -F.x; }	// rotate to form Tangent vector

   const float bfftd = ballvtx * T.x + ballvty * T.y;			// ball to flipper face tanget distance	

   const float len = m_flipperanim.m_faceLength; // face segment length ... i.g same on either face									
   if (bfftd < -C_TOL_ENDPNTS || bfftd > len + C_TOL_ENDPNTS) return -1.0f;	// not in range of touching

   const float hitz = pball->m_pos.z - ballr + pball->m_vel.z*t;	// check for a hole, relative to ball rolling point at hittime

   if ((hitz + (ballr * 1.5f)) < m_rcHitRect.zlow			//check limits of object's height and depth 
      || (hitz + (ballr * 0.5f)) > m_rcHitRect.zhigh)
      return -1.0f;

   // ok we have a confirmed contact, calc the stats, remember there are "near" solution, so all
   // parameters need to be calculated from the actual configuration, i.e contact radius must be calc'ed

   coll.hitnormal.x = F.x;	// hit normal is same as line segment normal
   coll.hitnormal.y = F.y;
   coll.hitnormal.z = 0.0f;

   const Vertex2D dist( // calculate moment from flipper base center
      (pball->m_pos.x + ballvx*t - ballr*F.x - m_flipperanim.m_hitcircleBase.center.x),  //center of ball + projected radius to contact point
      (pball->m_pos.y + ballvy*t - ballr*F.y - m_flipperanim.m_hitcircleBase.center.y)); // all at time t

   const float distance = sqrtf(dist.x*dist.x + dist.y*dist.y);	// distance from base center to contact point

   const float inv_dist = 1.0f/distance;
   coll.hitvelocity.x = -dist.y*inv_dist;		//Unit Tangent velocity of contact point(rotate Normal clockwise)
   coll.hitvelocity.y =  dist.x*inv_dist;
   coll.hitvelocity.z = 0.0f;

   if (contactAng >= angleMax && anglespeed > 0 || contactAng <= angleMin && anglespeed < 0)	// hit limits ??? 
      anglespeed = 0.0f;							// rotation stopped

   coll.hitmoment = distance;				//moment arm diameter
   //!! unused coll.hitangularrate = anglespeed;		//radians/time at collison

   const Vertex2D dv(
      (ballvx - coll.hitvelocity.x *anglespeed*distance), 
      (ballvy - coll.hitvelocity.y *anglespeed*distance)); //delta velocity ball to face

   const float bnv = dv.x*coll.hitnormal.x + dv.y*coll.hitnormal.y;  //dot Normal to delta v

   if (fabsf(bnv) <= C_CONTACTVEL && bffnd <= (float)PHYS_TOUCH)
   {
       coll.isContact = true;
       origNormVel = bnv;
   }
   else if (bnv > C_LOWNORMVEL)
      return -1.0f; // not hit ... ball is receding from endradius already, must have been embedded

   coll.hitdistance = bffnd;			//normal ...actual contact distance ... 
   //coll.hitRigid = true;				// collision type

   return t;
}


void HitFlipper::Collide(CollisionEvent *coll)
{
    Ball *pball = coll->ball;
    const Vertex3Ds normal = coll->hitnormal;

    const Vertex3Ds rB = -pball->m_radius * normal;
    const Vertex3Ds hitPos = pball->m_pos + rB;

    const Vertex3Ds cF(
            m_flipperanim.m_hitcircleBase.center.x,
            m_flipperanim.m_hitcircleBase.center.y,
            pball->m_pos.z );     // make sure collision happens in same z plane where ball is

    const Vertex3Ds rF = hitPos - cF;       // displacement relative to flipper center

    const Vertex3Ds vB = pball->SurfaceVelocity(rB);
    const Vertex3Ds vF = m_flipperanim.SurfaceVelocity(rF);
    const Vertex3Ds vrel = vB - vF;
    float bnv = normal.Dot(vrel);       // relative normal velocity

#ifdef DEBUG_FLIPPERS
    slintf("Collision\n  normal: %.2f %.2f %.2f\n  rel.vel.: %.2f %.2f %.2f\n", normal.x, normal.y, normal.z, vrel.x, vrel.y, vrel.z);
    slintf("  ball vel. %.2f %.2f %.2f\n", pball->m_vel.x, pball->m_vel.y, pball->m_vel.z);
    slintf("  norm.vel.: %.2f\n", bnv);
    slintf("  flipper: %.2f %.2f\n", m_flipperanim.m_angleCur, m_flipperanim.m_anglespeed);
#endif

   if (bnv >= -C_LOWNORMVEL )							// nearly receding ... make sure of conditions
   {													// otherwise if clearly approaching .. process the collision
      if (bnv > C_LOWNORMVEL) return;					//is this velocity clearly receding (i.e must > a minimum)		
#ifdef C_EMBEDDED
      if (coll->hitdistance < -C_EMBEDDED)
         bnv = -C_EMBEDSHOT;							// has ball become embedded???, give it a kick
      else return;
#endif
   }

#ifdef C_DISP_GAIN 
   // correct displacements, mostly from low velocity blindness, an alternative to true acceleration processing
   float hdist = -C_DISP_GAIN * coll->hitdistance;		// distance found in hit detection
   if (hdist > 1.0e-4f)
   {
      if (hdist > C_DISP_LIMIT) 
         hdist = C_DISP_LIMIT;	// crossing ramps, delta noise
      pball->m_pos.x += hdist * coll->hitnormal.x;	// push along norm, back to free area
      pball->m_pos.y += hdist * coll->hitnormal.y;	// use the norm, but is not correct
   }
#endif

   // angular response to impulse in normal direction
   Vertex3Ds angResp = CrossProduct( rF, normal );

   /*
    * Check if flipper is in contact with its stopper and the collision impulse
    * would push it beyond the stopper. In that case, don't allow any transfer
    * of kinetic energy from ball to flipper. This avoids overly dead bounces
    * in that case.
    */
   const float angImp = -angResp.z;     // minus because impulse will apply in -normal direction
   float flipperResponseScaling = 1.0f;
   if (m_flipperanim.m_isInContact && m_flipperanim.m_contactTorque * angImp >= 0)
   {
       // if impulse pushes against stopper, allow no loss of kinetic energy to flipper
       // (still allow flipper recoil, but a diminished amount)
       angResp.SetZero();
       flipperResponseScaling = 0.5f;
   }

   /*
    * Rubber has a coefficient of restitution which decreases with the impact velocity.
    * We use a heuristic model which decreases the COR according to a falloff parameter:
    * 0 = no falloff, 1 = half the COR at 1 m/s (18.53 speed units)
    */
   const float falloff = m_pflipper->m_d.m_OverridePhysics ? m_pflipper->m_d.m_OverrideElasticityFalloff : m_pflipper->m_d.m_elasticityFalloff;
   const float epsilon = ElasticityWithFalloff(m_elasticity, falloff, bnv);

   float impulse = -(1.0f + epsilon) * bnv
       / (pball->m_invMass + normal.Dot(CrossProduct(angResp / m_flipperanim.m_inertia, rF)));
   Vertex3Ds flipperImp = -(impulse * flipperResponseScaling) * normal;

#ifdef DEBUG_FLIPPERS
   slintf("  epsilon: %.2f\n  angular response: %.3f\n", epsilon, normal.Dot(CrossProduct(angResp / m_flipperanim.m_inertia, rF)));
#endif

   if (m_flipperanim.m_isInContact)
   {
       const Vertex3Ds rotI = CrossProduct(rF, flipperImp);
       if (rotI.z * m_flipperanim.m_contactTorque < 0)     // pushing against the solenoid?
       {
           // Get a bound on the time the flipper needs to return to static conditions.
           // If it's too short, we treat the flipper as static during the whole collision.
           const float recoilTime = -rotI.z / m_flipperanim.m_contactTorque; // time flipper needs to eliminate this impulse, in 10ms

           // Check ball normal velocity after collision. If the ball rebounded
           // off the flipper, we need to make sure it does so with full
           // reflection, i.e., treat the flipper as static, otherwise
           // we get overly dead bounces.
           const float bnv_after = bnv + impulse * pball->m_invMass;

#ifdef DEBUG_FLIPPERS
           slintf("  recoil time: %f  norm.vel after: %.2f\n", recoilTime, bnv_after);
#endif
           if (recoilTime <= 0.5f || bnv_after > 0)
           {
               // treat flipper as static for this impact
               impulse = -(1.0f + epsilon) * bnv / pball->m_invMass;
               flipperImp.SetZero();
           }
       }
   }

   pball->m_vel += (impulse * pball->m_invMass) * normal;      // new velocity for ball after impact
   m_flipperanim.ApplyImpulse(rF, flipperImp);

   // apply friction

   Vertex3Ds tangent = vrel - vrel.Dot(normal) * normal;       // calc the tangential velocity

   const float tangentSpSq = tangent.LengthSquared();
   if (tangent.LengthSquared() > 1e-6f)
   {
       tangent /= sqrtf(tangentSpSq);            // normalize to get tangent direction
       const float vt = vrel.Dot(tangent);   // get speed in tangential direction

       // compute friction impulse
       Vertex3Ds cross = CrossProduct(rB, tangent);
       float kt = pball->m_invMass + tangent.Dot(CrossProduct(cross / pball->m_inertia, rB));

       cross = CrossProduct(rF, tangent);
       kt += tangent.Dot(CrossProduct(cross / m_flipperanim.m_inertia, rF));    // flipper only has angular response

       // friction impulse can't be greater than coefficient of friction times collision impulse (Coulomb friction cone)
       const float maxFric = m_friction * impulse;
       const float jt = clamp(-vt / kt, -maxFric, maxFric);

       pball->ApplySurfaceImpulse(rB, jt * tangent);
       m_flipperanim.ApplyImpulse(rF, -jt * tangent);
   }

   pball->m_dynamic = C_DYNAMIC;           // reactive ball if quenched

   if ((bnv < -0.25f) && (g_pplayer->m_time_msec - m_last_hittime) > 250) // limit rate to 250 milliseconds per event
   {
       const float distance = coll->hitmoment;                     // moment .... and the flipper response
       const float flipperHit = (distance == 0.0f) ? -1.0f : -bnv; // move event processing to end of collision handler...
       if (flipperHit < 0.f)
           m_pflipper->FireGroupEvent(DISPID_HitEvents_Hit);        // simple hit event
       else
           m_pflipper->FireVoidEventParm(DISPID_FlipperEvents_Collide, flipperHit); // collision velocity (normal to face)
   }

   m_last_hittime = g_pplayer->m_time_msec; // keep resetting until idle for 250 milliseconds

#ifdef DEBUG_FLIPPERS
    slintf("   ---- after collision ----\n");
    slintf("  ball vel. %.2f %.2f %.2f\n", pball->m_vel.x, pball->m_vel.y, pball->m_vel.z);
    slintf("  flipper: %.2f %.2f\n", m_flipperanim.m_angleCur, m_flipperanim.m_anglespeed);
#endif
}

void HitFlipper::Contact(CollisionEvent& coll, float dtime)
{
    Ball * pball = coll.ball;
    const Vertex3Ds normal = coll.hitnormal;

    const Vertex3Ds rB = -pball->m_radius * normal;
    const Vertex3Ds hitPos = pball->m_pos + rB;

    const Vertex3Ds cF(
            m_flipperanim.m_hitcircleBase.center.x,
            m_flipperanim.m_hitcircleBase.center.y,
            pball->m_pos.z );     // make sure collision happens in same z plane where ball is

    const Vertex3Ds rF = hitPos - cF;       // displacement relative to flipper center

    const Vertex3Ds vB = pball->SurfaceVelocity(rB);
    const Vertex3Ds vF = m_flipperanim.SurfaceVelocity(rF);
    const Vertex3Ds vrel = vB - vF;

    const float normVel = vrel.Dot(normal);   // this should be zero, but only up to +/- C_CONTACTVEL

#ifdef DEBUG_FLIPPERS
    slintf("Flipper contact - rel.vel. %f\n", normVel);
#endif

    // If some collision has changed the ball's velocity, we may not have to do anything.
    if (normVel <= C_CONTACTVEL)
    {
        // compute accelerations of point on ball and flipper
        const Vertex3Ds aB = pball->SurfaceAcceleration(rB);
        const Vertex3Ds aF = m_flipperanim.SurfaceAcceleration(rF);
        const Vertex3Ds arel = aB - aF;

        // time derivative of the normal vector
        const Vertex3Ds normalDeriv = CrossZ(m_flipperanim.m_anglespeed, normal);

        // relative acceleration in the normal direction
        const float normAcc = arel.Dot(normal) + 2.0f * normalDeriv.Dot(vrel);

        if (normAcc >= 0)
            return;     // objects accelerating away from each other, nothing to do

        // hypothetical accelerations arising from a unit contact force in normal direction
        const Vertex3Ds aBc = pball->m_invMass * normal;
        const Vertex3Ds aFc = CrossProduct( CrossProduct(rF, -normal) / m_flipperanim.m_inertia, rF );
        const float contactForceAcc = normal.Dot( aBc - aFc );

        assert( contactForceAcc > 0.f );

        // find j >= 0 such that normAcc + j * contactForceAcc >= 0  (bodies should not accelerate towards each other)

        const float j = -normAcc / contactForceAcc;

        pball->m_vel += (j * pball->m_invMass * dtime - origNormVel) * normal; // kill any existing normal velocity
        m_flipperanim.ApplyImpulse(rF, (-j * dtime) * normal);

        // apply friction

        // first check for slippage
        const Vertex3Ds slip = vrel - normVel * normal;       // calc the tangential slip velocity

        const float maxFric = j * m_friction;

        const float slipspeed = slip.Length();
        if (slipspeed < C_PRECISION)
        {
            // slip speed zero - static friction case

            const Vertex3Ds slipAcc = arel - arel.Dot(normal) * normal;       // calc the tangential slip acceleration

            // neither slip velocity nor slip acceleration? nothing to do here
            if (slipAcc.LengthSquared() < 1e-6f)
                return;

            Vertex3Ds slipDir = slipAcc;
            slipDir.Normalize();

            const float numer = - slipDir.Dot( arel );
            const float denomB = pball->m_invMass + slipDir.Dot( CrossProduct( CrossProduct(rB, slipDir) / pball->m_inertia, rB ) );
            const float denomF = slipDir.Dot( CrossProduct( CrossProduct(rF, -slipDir) / m_flipperanim.m_inertia, rF ) );
            const float fric = clamp(numer / (denomB + denomF), -maxFric, maxFric);

            pball->ApplySurfaceImpulse(rB, (dtime * fric) * slipDir);
            m_flipperanim.ApplyImpulse(rF, (-dtime * fric) * slipDir);
        }
        else
        {
            // nonzero slip speed - dynamic friction case

            Vertex3Ds slipDir = slip / slipspeed;

            const float numer = - slipDir.Dot( vrel );
            const float denomB = pball->m_invMass + slipDir.Dot( CrossProduct( CrossProduct(rB, slipDir) / pball->m_inertia, rB ) );
            const float denomF = slipDir.Dot( CrossProduct( CrossProduct(rF, slipDir) / m_flipperanim.m_inertia, rF ) );
            const float fric = clamp(numer / (denomB + denomF), -maxFric, maxFric);

            pball->ApplySurfaceImpulse(rB, (dtime * fric) * slipDir);
            m_flipperanim.ApplyImpulse(rF, (-dtime * fric) * slipDir);
        }
    }
}
