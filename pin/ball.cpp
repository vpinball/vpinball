#include "stdafx.h"

unsigned int Ball::ballID=0;

Ball::Ball()
{
   m_id = ballID;
   ballID++;

   m_coll.ball = this;      // TODO: this needs to move somewhere else
   m_coll.obj = NULL;
   m_pballex = NULL;
   m_vpVolObjs = NULL; // should be NULL ... only real balls have this value
   m_pinballEnv=NULL;
   m_pinballDecal=NULL;
   m_defaultZ = 25.0f; //!! assumes ball radius 25
   m_Event_Pos.x = m_Event_Pos.y = m_Event_Pos.z = -1.0f;
   m_frozen = false;
   m_color = RGB(20,20,20);

   m_ringcounter_oldpos = 0;
   for(int i = 0; i < MAX_BALL_TRAIL_POS; ++i)
	   m_oldpos[i].x = FLT_MAX;
}

void Ball::RenderSetup()
{
}

void Ball::Init(const float mass)
{
   // Only called by real balls, not temporary objects created for physics/rendering
   m_mass = mass;
   m_invMass = 1.0f / m_mass;

   m_orientation.Identity();
   m_inertia = (float)(2.0/5.0) * m_radius*m_radius * m_mass;
   m_angularvelocity.Set(0,0,0);
   m_angularmomentum.Set(0,0,0);

   m_ballanim.m_pball = this;

   m_frozen = false;

   m_coll.obj = NULL;
   m_dynamic = C_DYNAMIC; // assume dynamic

   m_pballex = NULL;

   m_vpVolObjs = new VectorVoid;

   m_color = RGB(20,20,20);

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

   if (g_pplayer->m_ptable->m_szBallImageFront[0] == '\0')
   {
      m_szImageFront[0] = '\0';
      m_pinballDecal = NULL;
   }
   else
   {
      lstrcpy(m_szImageFront, g_pplayer->m_ptable->m_szBallImageFront);
      m_pinballDecal = g_pplayer->m_ptable->GetImage(m_szImageFront);
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
 
void Ball::Collide3DWall(const Vertex3Ds& hitNormal, float elasticity, float elastFalloff, float friction, float scatter_angle)
{
    //speed normal to wall
    float dot = m_vel.Dot(hitNormal);

	if (dot >= -C_LOWNORMVEL )							// nearly receding ... make sure of conditions
	{													// otherwise if clearly approaching .. process the collision
		if (dot > C_LOWNORMVEL) return;					//is this velocity clearly receding (i.e must > a minimum)

#ifdef C_EMBEDDED
		if (m_coll.hitdistance < -C_EMBEDDED)
			dot = -C_EMBEDSHOT;							// has ball become embedded???, give it a kick
		else return;
#endif
	} 
		
#ifdef C_DISP_GAIN 
	// correct displacements, mostly from low velocity, alternative to acceleration processing
	float hdist = -C_DISP_GAIN * m_coll.hitdistance;	// limit delta noise crossing ramps,
	if (hdist > 1.0e-4f)					// when hit detection checked it what was the displacement
	{
		if (hdist > C_DISP_LIMIT) 
			hdist = C_DISP_LIMIT;	// crossing ramps, delta noise
		m_pos += hdist * hitNormal;	// push along norm, back to free area
        // use the norm, but this is not correct, reverse time is correct
	}
#endif

    // magnitude of the impulse which is just sufficient to keep the ball from
    // penetrating the wall (needed for friction computations)
    const float reactionImpulse = m_mass * fabsf(dot);

    elasticity = ElasticityWithFalloff(elasticity, elastFalloff, dot);
    dot *= -(1.0f + elasticity);
    m_vel += dot * hitNormal;     // apply collision impulse (along normal, so no torque)

    // compute friction impulse

    const Vertex3Ds surfP = -m_radius * hitNormal;    // surface contact point relative to center of mass

    Vertex3Ds surfVel = SurfaceVelocity(surfP);       // velocity at impact point

    Vertex3Ds tangent = surfVel - surfVel.Dot(hitNormal) * hitNormal; // calc the tangential velocity

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

    if (scatter_angle < 0.0f) scatter_angle = c_hardScatter;			// if < 0 use global value
	scatter_angle *= g_pplayer->m_ptable->m_globalDifficulty;			// apply difficulty weighting

	if (dot > 1.0f && scatter_angle > 1.0e-5f) //no scatter at low velocity
	{
		float scatter = rand_mt_m11();									// -1.0f..1.0f
		scatter *= (1.0f - scatter*scatter)*2.59808f * scatter_angle;	// shape quadratic distribution and scale
		const float radsin = sinf(scatter); // Green's transform matrix... rotate angle delta
		const float radcos = cosf(scatter); // rotational transform from current position to position at time t
		const float vxt = m_vel.x;
		const float vyt = m_vel.y;
		m_vel.x = vxt *radcos - vyt *radsin;  // rotate to random scatter angle
		m_vel.y = vyt *radcos + vxt *radsin;
	}
}

float Ball::HitTest(const Ball * pball_, float dtime, CollisionEvent& coll)
{	
    Ball * pball = const_cast<Ball*>(pball_);   // HACK; needed below

    Vertex3Ds d = m_pos - pball->m_pos;     // delta position

    Vertex3Ds dv = m_vel - pball->m_vel;    // delta velocity

	float bcddsq = d.LengthSquared();       // square of ball center's delta distance
	float bcdd = sqrtf(bcddsq);				// length of delta

	if (bcdd < 1.0e-8f)						// two balls center-over-center embedded
	{ //return -1;
		d.z = -1.0f;						// patch up
		pball->m_pos.z -= d.z;				// lift up
		
		bcdd = 1.0f;						// patch up
		bcddsq = 1.0f;						// patch up
		dv.z = 0.1f;						// small speed difference
		pball->m_vel.z -= dv.z;
	}

	const float b = dv.Dot(d);              // inner product
	const float bnv = b/bcdd;				// normal speed of balls toward each other

	if ( bnv > C_LOWNORMVEL) return -1.0f;	// dot of delta velocity and delta displacement, postive if receding no collison

	const float totalradius = pball->m_radius + m_radius;
	const float bnd = bcdd - totalradius;   // distance between ball surfaces

	float hittime;
	if (bnd < (float)PHYS_TOUCH)			// in contact??? 
	{
		if (bnd <= pball->m_radius*-2.0f)
			return -1.0f;					// embedded too deep?

		if ((fabsf(bnv) > C_CONTACTVEL)		// >fast velocity, return zero time
											//zero time for rigid fast bodies
		|| (bnd <= (float)(-PHYS_TOUCH)))
			hittime = 0;					// slow moving but embedded
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

    const Vertex3Ds hitPos = pball->m_pos + hittime * dv; // new ball position

    //calc unit normal of collision
	coll.hitnormal = hitPos - m_pos;
    coll.hitnormal.Normalize();

	coll.hitdistance = bnd;			// actual contact distance
	coll.hitRigid = true;			// rigid collision type

	return hittime;	
}

void Ball::Collide(CollisionEvent *coll)
{
    Ball * const pball = coll->ball;

    // make sure we process each ball/ball collision only once
    // (but if we are frozen, there won't be a second collision event, so deal with it now!)
    if (pball <= this && !this->m_frozen)
        return;

    // target ball to object ball delta velocity
    const Vertex3Ds vrel = pball->m_vel - m_vel;
	const Vertex3Ds vnormal = coll->hitnormal;
	float dot = vrel.Dot(vnormal);

	// correct displacements, mostly from low velocity, alternative to true acceleration processing
	if (dot >= -C_LOWNORMVEL )								// nearly receding ... make sure of conditions
	{														// otherwise if clearly approaching .. process the collision
		if (dot > C_LOWNORMVEL) return;						//is this velocity clearly receding (i.e must > a minimum)		
#ifdef C_EMBEDDED
		if (coll->hitdistance < -C_EMBEDDED)
			dot = -C_EMBEDSHOT;		// has ball become embedded???, give it a kick
		else return;
#endif
	}

    // send ball/ball collision event to script function
    if (dot < -0.25f)    // only collisions with at least some small true impact velocity (no contacts)
    {
        g_pplayer->m_ptable->InvokeBallBallCollisionCallback(this, pball, -dot);
    }

#ifdef C_DISP_GAIN
	float edist = -C_DISP_GAIN * coll->hitdistance;
	if (edist > 1.0e-4f)
	{										
		if (edist > C_DISP_LIMIT) 
			edist = C_DISP_LIMIT;		// crossing ramps, delta noise
		if (!m_frozen) edist *= 0.5f;	// if the hitten ball is not frozen
        pball->m_pos += edist * vnormal;// push along norm, back to free area
        // use the norm, but is not correct, but cheaply handled
	}

	edist = -C_DISP_GAIN * m_coll.hitdistance;	// noisy value .... needs investigation
	if (!m_frozen && edist > 1.0e-4f)
	{ 
		if (edist > C_DISP_LIMIT) 
			edist = C_DISP_LIMIT;		// crossing ramps, delta noise
		edist *= 0.5f;		
        m_pos -= edist * vnormal;       // pull along norm, back to free area
	}
#endif

    const float myInvMass = m_frozen ? 0.0f : m_invMass; // frozen ball has infinite mass
    const float impulse = -(float)(1.0 + 0.8) * dot / (myInvMass + pball->m_invMass);    // resitution = 0.8

    if (!m_frozen)
    {
        m_vel -= (impulse * myInvMass) * vnormal;
        m_dynamic = C_DYNAMIC;
    }
    pball->m_vel += (impulse * pball->m_invMass) * vnormal;
    pball->m_dynamic = C_DYNAMIC;
}

void Ball::HandleStaticContact(const Vertex3Ds& normal, const float origNormVel, const float friction, const float dtime)
{
    const float normVel = m_vel.Dot(normal);   // this should be zero, but only up to +/- C_CONTACTVEL

    // If some collision has changed the ball's velocity, we may not have to do anything.
    if (normVel <= C_CONTACTVEL)
    {
        const Vertex3Ds fe = m_mass * g_pplayer->m_gravity;      // external forces (only gravity for now)
        const float dot = fe.Dot(normal);
        const float normalForce = std::max( 0.0f, -(dot*dtime + origNormVel) ); // normal force is always nonnegative

        // Add just enough to kill original normal velocity and counteract the external forces.
        m_vel += normalForce * normal;

        ApplyFriction(normal, dtime, friction);
    }
}

void Ball::ApplyFriction(const Vertex3Ds& hitnormal, const float dtime, const float fricCoeff)
{
    const Vertex3Ds surfP = -m_radius * hitnormal;    // surface contact point relative to center of mass

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
    return m_vel + CrossProduct(m_angularvelocity, surfP); // linear velocity plus tangential velocity due to rotation
}

Vertex3Ds Ball::SurfaceAcceleration(const Vertex3Ds& surfP) const
{
    // if we had any external torque, we would have to add "(deriv. of ang.vel.) x surfP" here
    return m_invMass * g_pplayer->m_gravity     // linear acceleration
        + CrossProduct( m_angularvelocity, CrossProduct(m_angularvelocity, surfP) ); // centripetal acceleration
}

void Ball::ApplySurfaceImpulse(const Vertex3Ds& surfP, const Vertex3Ds& impulse)
{
    m_vel += m_invMass * impulse;

    const Vertex3Ds rotI = CrossProduct(surfP, impulse);
    m_angularmomentum += rotI;
    m_angularvelocity = m_angularmomentum / m_inertia;
}

void Ball::CalcHitRect()
{
	const float dx = fabsf(m_vel.x);
	const float dy = fabsf(m_vel.y);

	m_rcHitRect.left   = m_pos.x - (m_radius + 0.1f + dx); //!! make more accurate ????
	m_rcHitRect.right  = m_pos.x + (m_radius + 0.1f + dx);
	m_rcHitRect.top    = m_pos.y - (m_radius + 0.1f + dy);
	m_rcHitRect.bottom = m_pos.y + (m_radius + 0.1f + dy);

	m_rcHitRect.zlow  = min(m_pos.z, m_pos.z+m_vel.z) - m_radius;
	m_rcHitRect.zhigh = max(m_pos.z, m_pos.z+m_vel.z) + (m_radius + 0.1f);
   // update defaultZ for ball reflection
   // if the ball was created by a kicker which is higher than the playfield 
   // the defaultZ must be updated if the ball falls onto the playfield that means the Z value is equal to the radius
   if ( m_pos.z == m_radius )
      m_defaultZ = m_pos.z;
}

void BallAnimObject::UpdateDisplacements(const float dtime)
{
	m_pball->UpdateDisplacements(dtime);
}

void Ball::UpdateDisplacements(const float dtime)
{    	
	if (!m_frozen)
	{
        const Vertex3Ds ds = dtime * m_vel;
        m_pos += ds;

		m_drsq = ds.LengthSquared(); // used to determine if static ball

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
	const float nx = g_pplayer->m_NudgeX;     // TODO: depends on STEPTIME
	const float ny = g_pplayer->m_NudgeY;
	
	if (!m_frozen)  // Gravity	
	{
		m_vel += (float)PHYS_FACTOR * g_pplayer->m_gravity;

		m_vel.x += nx;
		m_vel.y += ny;

        m_vel -= g_pplayer->m_tableVelDelta;
	}

	m_dynamic = C_DYNAMIC; // always set .. after adding velocity

	CalcHitRect();
}
