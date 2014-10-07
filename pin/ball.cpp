#include "stdafx.h"

int Ball::ballsInUse=0;

int Ball::GetBallsInUse()
{
    return ballsInUse;
}

Ball::Ball()
{
   ballsInUse++;

   m_coll.ball = this;      // TODO: this needs to move somewhere else
   m_coll.obj = NULL;
   m_pballex = NULL;
   m_vpVolObjs = NULL; // should be NULL ... only real balls have this value
   m_pin=NULL;
   m_pinFront=NULL;
   defaultZ = 25.0f;
   m_Event_Pos.x = m_Event_Pos.y = m_Event_Pos.z = -1.0f;
   fFrozen = false;
   m_color = RGB(0,0,0);

   ringcounter_oldpos = 0;
   for(int i = 0; i < BALL_TRAIL_NUM_POS; ++i)
	   oldpos[i].x = FLT_MAX;
}

Ball::~Ball()
{
	ballsInUse--; //Added by JEP.  Need to keep track of number of balls on table for autostart to work.
}

void Ball::RenderSetup()
{
}

void Ball::Init()
{
   // Only called by real balls, not temporary objects created for physics/rendering
   m_mass = 1.0f;
   m_invMass = 1.0f / m_mass;

   m_orientation.Identity();
   m_inertia = (float)(2.0/5.0) * radius*radius * m_mass;
   m_angularvelocity.Set(0,0,0);
   m_angularmomentum.Set(0,0,0);

   m_ballanim.m_pball = this;

   fFrozen = false;

   m_coll.obj = NULL;
   m_fDynamic = C_DYNAMIC; // assume dynamic

   m_pballex = NULL;

   m_vpVolObjs = new VectorVoid;

   m_color = RGB(0,0,0);

   if (g_pplayer->m_ptable->m_szBallImage[0] == '\0')
   {
      m_szImage[0] = '\0';
      m_pin = NULL;
   }
   else
   {
      lstrcpy(m_szImage, g_pplayer->m_ptable->m_szBallImage);
      m_pin = g_pplayer->m_ptable->GetImage(m_szImage);
   }

   if (g_pplayer->m_ptable->m_szBallImageFront[0] == '\0')
   {
      m_szImageFront[0] = '\0';
      m_pinFront = NULL;
   }
   else
   {
      lstrcpy(m_szImageFront, g_pplayer->m_ptable->m_szBallImageFront);
      m_pinFront = g_pplayer->m_ptable->GetImage(m_szImageFront);
   }

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
 
void Ball::Collide3DWall(const Vertex3Ds& hitNormal, const float elasticity, float friction, float scatter_angle)
{
    //speed normal to wall
    float dot = vel.Dot(hitNormal);

	if (dot >= -C_LOWNORMVEL )							// nearly receding ... make sure of conditions
	{													// otherwise if clearly approaching .. process the collision
		if (dot > C_LOWNORMVEL) return;					//is this velocity clearly receding (i.e must > a minimum)

#ifdef C_EMBEDDED
		if (m_coll.distance < -C_EMBEDDED)
			dot = -C_EMBEDSHOT;							// has ball become embedded???, give it a kick
		else return;
#endif
	} 
		
#ifdef C_DISP_GAIN 
	// correct displacements, mostly from low velocity, alternative to acceleration processing
	float hdist = -C_DISP_GAIN * m_coll.distance;	// limit delta noise crossing ramps,
	if (hdist > 1.0e-4f)					// when hit detection checked it what was the displacement
	{
		if (hdist > C_DISP_LIMIT) 
			hdist = C_DISP_LIMIT;	// crossing ramps, delta noise
		pos += hdist * hitNormal;	// push along norm, back to free area
        // use the norm, but this is not correct, reverse time is correct
	}
#endif

    // magnitude of the impulse which is just sufficient to keep the ball from
    // penetrating the wall (needed for friction computations)
    const float reactionImpulse = m_mass * fabsf(dot);

    dot *= -(1.0f + elasticity);
    vel += dot * hitNormal;     // apply collision impulse (along normal, so no torque)

    // compute friction impulse

    const Vertex3Ds surfP = -radius * hitNormal;    // surface contact point relative to center of mass

    Vertex3Ds surfVel = SurfaceVelocity(surfP);            // velocity at impact point

    Vertex3Ds tangent = surfVel - surfVel.Dot(hitNormal) * hitNormal;       // calc the tangential velocity

    const float tangentSpSq = tangent.LengthSquared();
    if (tangent.LengthSquared() > 1e-6f)
    {
        tangent /= sqrtf(tangentSpSq);           // normalize to get tangent direction
        const float vt = surfVel.Dot(tangent);   // get speed in tangential direction

        // compute friction impulse
        const Vertex3Ds cross = CrossProduct(surfP, tangent);
        const float kt = m_invMass + tangent.Dot( CrossProduct(cross / m_inertia, surfP));

        // friction impulse can't be greather than coefficient of friction times collision impulse (Coulomb friction cone)
        const float maxFric = friction * reactionImpulse;
        const float jt = clamp(-vt / kt, -maxFric, maxFric);

        ApplySurfaceImpulse(surfP, jt * tangent);
    }

    //!! TODO: reenable scatter if needed
	//if (scatter_angle <= 0.0f) scatter_angle = c_hardScatter;			// if <= 0 use global value
	//scatter_angle *= g_pplayer->m_ptable->m_globalDifficulty;			// apply difficulty weighting

	//if (dot > 1.0f && scatter_angle > 1.0e-5f) //no scatter at low velocity
	//{
	//	float scatter = rand_mt_m11();									// -1.0f..1.0f
	//	scatter *= (1.0f - scatter*scatter)*2.59808f * scatter_angle;	// shape quadratic distribution and scale
	//	const float radsin = sinf(scatter); // Green's transform matrix... rotate angle delta
	//	const float radcos = cosf(scatter); // rotational transform from current position to position at time t
	//	const float vxt = vel.x;
	//	const float vyt = vel.y;
	//	vel.x = vxt *radcos - vyt *radsin;  // rotate to random scatter angle
	//	vel.y = vyt *radcos + vxt *radsin;
	//}
}

float Ball::HitTest(const Ball * pball_, float dtime, CollisionEvent& coll)
{	
    Ball * pball = const_cast<Ball*>(pball_);   // HACK; needed below

    Vertex3Ds d = pos - pball->pos;          // delta position

    Vertex3Ds dv = vel - pball->vel;        // delta velocity

	float bcddsq = d.LengthSquared();       // square of ball center's delta distance
	float bcdd = sqrtf(bcddsq);				// length of delta

	if (bcdd < 1.0e-8f)						// two balls center-over-center embedded
	{ //return -1;
		d.z = -1.0f;						// patch up
		pball->pos.z -= d.z;				// lift up
		
		bcdd = 1.0f;						// patch up
		bcddsq = 1.0f;						// patch up
		dv.z = 0.1f;						// small speed difference
		pball->vel.z -= dv.z;
	}

	const float b = dv.Dot(d);              // inner product
	const float bnv = b/bcdd;				// normal speed of balls toward each other

	if ( bnv > C_LOWNORMVEL) return -1.0f;	// dot of delta velocity and delta displacement, postive if receding no collison

	const float totalradius = pball->radius + radius;
	const float bnd = bcdd - totalradius;   // distance between ball surfaces

	float hittime;
	if (bnd < (float)PHYS_TOUCH)			// in contact??? 
	{
		if (bnd <= (float)(-PHYS_SKIN*2.0))
			return -1.0f;					// embedded too deep

		if ((fabsf(bnv) > C_CONTACTVEL)			// >fast velocity, return zero time
												//zero time for rigid fast bodies
		|| (bnd <= (float)(-PHYS_TOUCH)))
			hittime = 0;						// slow moving but embedded
		else
			hittime = bnd/(float)(2.0*PHYS_TOUCH) + 0.5f;	// don't compete for fast zero time events
	}
	else
	{
        // find collision time as solution of quadratic equation
        //   at^2 + bt + c = 0
		const float a = dv.LengthSquared();         // square of differential velocity

		if (a < 1.0e-8f)
            return -1.0f;				// ball moving really slow, then wait for contact

        float time1, time2;
        if (!SolveQuadraticEq(a, 2.0f*b, bcddsq - totalradius*totalradius, time1, time2))
            return -1.0f;

        // choose smallest non-negative solution
		hittime = std::min(time1, time2);
        if (hittime < 0)
            hittime = std::max(time1, time2);

		if (infNaN(hittime) || hittime < 0 || hittime > dtime)
            return -1.0f; // .. was some time previous || beyond the next physics tick
	}

    const Vertex3Ds hitPos = pball->pos + hittime * dv; // new ball position

    //calc unit normal of collision
	coll.normal[0] = hitPos - pos;
    coll.normal[0].Normalize();

	coll.distance = bnd;					//actual contact distance
	coll.hitRigid = true;					//rigid collision type

	return hittime;	
}

void Ball::Collide(CollisionEvent *coll)
{
    Ball *pball = coll->ball;

    // make sure we process each ball/ball collision only once
    // (but if we are frozen, there won't be a second collision event, so deal with it now!)
    if (pball <= this && !this->fFrozen)
        return;

    // target ball to object ball delta velocity
    const Vertex3Ds vrel = pball->vel - vel;
	const Vertex3Ds vnormal = coll->normal[0];
	float dot = vrel.Dot(vnormal);

	// correct displacements, mostly from low velocity, alternative to true acceleration processing
	if (dot >= -C_LOWNORMVEL )								// nearly receding ... make sure of conditions
	{														// otherwise if clearly approaching .. process the collision
		if (dot > C_LOWNORMVEL) return;						//is this velocity clearly receding (i.e must > a minimum)		
#ifdef C_EMBEDDED
		if (coll->distance < -C_EMBEDDED)
			dot = -C_EMBEDSHOT;		// has ball become embedded???, give it a kick
		else return;
#endif
	}

#ifdef C_DISP_GAIN
	float edist = -C_DISP_GAIN * coll->distance;
	if (edist > 1.0e-4f)
	{										
		if (edist > C_DISP_LIMIT) 
			edist = C_DISP_LIMIT;		// crossing ramps, delta noise
		if (!fFrozen) edist *= 0.5f;	// if the hitten ball is not frozen
        pball->pos += edist * vnormal;// push along norm, back to free area
        // use the norm, but is not correct, but cheaply handled
	}

	edist = -C_DISP_GAIN * m_coll.distance;	// noisy value .... needs investigation
	if (!fFrozen && edist > 1.0e-4f)
	{ 
		if (edist > C_DISP_LIMIT) 
			edist = C_DISP_LIMIT;		// crossing ramps, delta noise
		edist *= 0.5f;		
        pos -= edist * vnormal;         // pull along norm, back to free area
	}
#endif

    const float myInvMass = fFrozen ? 0.0f : m_invMass; // frozen ball has infinite mass
    const float impulse = -(float)(1.0 + 0.8) * dot / (myInvMass + pball->m_invMass);    // resitution = 0.8

    if (!fFrozen)
    {
        vel -= (impulse * myInvMass) * vnormal;
        m_fDynamic = C_DYNAMIC;
    }
    pball->vel += (impulse * pball->m_invMass) * vnormal;
    pball->m_fDynamic = C_DYNAMIC;
}

void Ball::HandleStaticContact(const Vertex3Ds& normal, float origNormVel, float friction, float dtime)
{
    const float normVel = vel.Dot(normal);   // this should be zero, but only up to +/- C_CONTACTVEL

    // If some collision has changed the ball's velocity, we may not have to do anything.
    if (normVel <= C_CONTACTVEL)
    {
        const Vertex3Ds fe = m_mass * g_pplayer->m_gravity;      // external forces (only gravity for now)
        const float dot = fe.Dot(normal);
        const float normalForce = std::max( 0.0f, -(dot*dtime + origNormVel) ); // normal force is always nonnegative

        // Add just enough to kill original normal velocity and counteract the external forces.
        vel += normalForce * normal;

        ApplyFriction(normal, dtime, friction);
    }
}

void Ball::ApplyFriction(const Vertex3Ds& hitnormal, float dtime, float fricCoeff)
{
    const Vertex3Ds surfP = -radius * hitnormal;    // surface contact point relative to center of mass

    Vertex3Ds surfVel = SurfaceVelocity(surfP);
    const Vertex3Ds slip = surfVel - surfVel.Dot(hitnormal) * hitnormal;       // calc the tangential slip velocity

    const float maxFric = fricCoeff * m_mass * -g_pplayer->m_gravity.Dot(hitnormal);

    const float slipspeed = slip.Length();
    //slintf("Velocity: %.2f Angular velocity: %.2f Surface velocity: %.2f Slippage: %.2f\n", vel.Length(), m_angularvelocity.Length(), surfVel.Length(), slipspeed);
    //if (slipspeed > 1e-6f)
    if (slipspeed < C_PRECISION)
    {
        // slip speed zero - static friction case

        Vertex3Ds surfAcc = SurfaceAcceleration(surfP);
        const Vertex3Ds slipAcc = surfAcc - surfAcc.Dot(hitnormal) * hitnormal;       // calc the tangential slip acceleration

        // neither slip velocity nor slip acceleration? nothing to do here
        if (slipAcc.LengthSquared() < 1e-6f)
            return;

        Vertex3Ds slipDir = slipAcc;
        slipDir.Normalize();

        const float numer = - slipDir.Dot( surfAcc );
        const float denom = m_invMass + slipDir.Dot( CrossProduct( CrossProduct(surfP, slipDir) / m_inertia, surfP ) );
        const float fric = clamp(numer / denom, -maxFric, maxFric);

        ApplySurfaceImpulse(surfP, (dtime * fric) * slipDir);
    }
    else
    {
        // nonzero slip speed - dynamic friction case

        Vertex3Ds slipDir = slip / slipspeed;

        const float numer = - slipDir.Dot( surfVel );
        const float denom = m_invMass + slipDir.Dot( CrossProduct( CrossProduct(surfP, slipDir) / m_inertia, surfP ) );
        const float fric = clamp(numer / denom, -maxFric, maxFric);

        ApplySurfaceImpulse(surfP, (dtime * fric) * slipDir);
    }
}

Vertex3Ds Ball::SurfaceVelocity(const Vertex3Ds& surfP) const
{
    return vel + CrossProduct(m_angularvelocity, surfP);      // linear velocity plus tangential velocity due to rotation
}

Vertex3Ds Ball::SurfaceAcceleration(const Vertex3Ds& surfP) const
{
    // if we had any external torque, we would have to add "(deriv. of ang.vel.) x surfP" here
    return m_invMass * g_pplayer->m_gravity     // linear acceleration
        + CrossProduct( m_angularvelocity, CrossProduct(m_angularvelocity, surfP) ); // centripetal acceleration
}

void Ball::ApplySurfaceImpulse(const Vertex3Ds& surfP, const Vertex3Ds& impulse)
{
    vel += m_invMass * impulse;

    const Vertex3Ds rotI = CrossProduct(surfP, impulse);
    m_angularmomentum += rotI;
    m_angularvelocity = m_angularmomentum / m_inertia;
}

void Ball::CalcHitRect()
{
	const float dx = fabsf(vel.x);
	const float dy = fabsf(vel.y);

	m_rcHitRect.left   = pos.x - (radius + 0.1f + dx); //!! make more accurate ????
	m_rcHitRect.right  = pos.x + (radius + 0.1f + dx);
	m_rcHitRect.top    = pos.y - (radius + 0.1f + dy);
	m_rcHitRect.bottom = pos.y + (radius + 0.1f + dy);

	m_rcHitRect.zlow  = min(pos.z, pos.z+vel.z) - radius;
	m_rcHitRect.zhigh = max(pos.z, pos.z+vel.z) + (radius + 0.1f);
   // update defaultZ for ball reflection
   // if the ball was created by a kicker which is higher than the playfield 
   // the defaultZ must be updated if the ball falls onto the playfield that means the Z value is equal to the radius
   if ( pos.z==radius )
      defaultZ = pos.z;
}

void BallAnimObject::UpdateDisplacements(const float dtime)
{
	m_pball->UpdateDisplacements(dtime);
}

void Ball::UpdateDisplacements(const float dtime)
{    	
	if (!fFrozen)
	{
        const Vertex3Ds ds = dtime * vel;
        pos += ds;

		drsq = ds.LengthSquared();                      // used to determine if static ball

		CalcHitRect();
		
		Matrix3 mat3;
		mat3.CreateSkewSymmetric(m_angularvelocity);
		
		Matrix3 addedorientation;
		addedorientation.MultiplyMatrix(&mat3, &m_orientation);
		addedorientation.MultiplyScalar(dtime);

		m_orientation.AddMatrix(&addedorientation, &m_orientation);
		m_orientation.OrthoNormalize();

        m_angularvelocity = m_angularmomentum / m_inertia;
	}
}

void BallAnimObject::UpdateVelocities()
{
	m_pball->UpdateVelocities();
}

void Ball::UpdateVelocities()
{
	const float g = g_pplayer->m_gravity.z;
	const float nx = g_pplayer->m_NudgeX;     // TODO: depends on STEPTIME
	const float ny = g_pplayer->m_NudgeY;
	
	if (!fFrozen)  // Gravity	
	{
		vel += (float)PHYS_FACTOR * g_pplayer->m_gravity;

		vel.x += nx;
		vel.y += ny;

        vel -= g_pplayer->m_tableVelDelta;
	}

	m_fDynamic = C_DYNAMIC; // always set .. after adding velocity

	CalcHitRect();
}
