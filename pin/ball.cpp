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
   m_pinBack=NULL;
   m_pinFront=NULL;
   defaultZ = 25.0f;
   m_Event_Pos.x = m_Event_Pos.y = m_Event_Pos.z = -1.0f;
   fFrozen = false;
   m_color = RGB(255,255,255);
   material.setColor( 1.0f, m_color );

   m_disableLighting = false;

   ringcounter_oldpos = 0;
   for(int i = 0; i < 10; ++i)
	   oldpos[i].x = FLT_MAX;
}

Ball::~Ball()
{
	ballsInUse--; //Added by JEP.  Need to keep track of number of balls on table for autostart to work.
}

void Ball::RenderSetup()
{
   if (m_pin)
      m_pin->CreateAlphaChannel();
   if ( m_pinFront )
      m_pinFront->CreateAlphaChannel();
}

void Ball::Init()
{
   // Only called by real balls, not temporary objects created for physics/rendering
   collisionMass = 1.0f;
   m_orientation.Identity();
   m_inversebodyinertiatensor.Identity((float)(5.0/2.0)/(radius*radius));
   m_angularvelocity.Set(0,0,0);
   m_angularmomentum.Set(0,0,0);

   m_ballanim.m_pball = this;

   fFrozen = false;

   // world limits on ball displacements
   //	x_min = g_pplayer->m_ptable->m_left + radius;
   //	x_max = g_pplayer->m_ptable->m_right - radius;
   //	y_min = g_pplayer->m_ptable->m_top + radius;
   //	y_max = g_pplayer->m_ptable->m_bottom - radius;
   z_min = g_pplayer->m_ptable->m_tableheight + radius;
   z_max = (g_pplayer->m_ptable->m_glassheight - radius);

   m_coll.obj = NULL;
   m_fDynamic = C_DYNAMIC; // assume dynamic

   m_pballex = NULL;

   m_vpVolObjs = new VectorVoid;

   m_color = RGB(255,255,255);

   m_disableLighting = false;

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
 
void Ball::Collide3DWall(const Vertex3Ds& hitNormal, const float elasticity, float antifriction, float scatter_angle)
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

	dot *= -1.005f - elasticity; //!! some small minimum
	vel += dot * hitNormal;

	if (antifriction >= 1.0f || antifriction <= 0.0f) 
		antifriction = c_hardFriction; // use global

	//friction all axes
	vel *= antifriction;

	if (scatter_angle <= 0.0f) scatter_angle = c_hardScatter;			// if <= 0 use global value
	scatter_angle *= g_pplayer->m_ptable->m_globalDifficulty;			// apply difficulty weighting

	if (dot > 1.0f && scatter_angle > 1.0e-5f) //no scatter at low velocity 
	{
		float scatter = rand_mt_m11();									// -1.0f..1.0f
		scatter *= (1.0f - scatter*scatter)*2.59808f * scatter_angle;	// shape quadratic distribution and scale
		const float radsin = sinf(scatter); // Green's transform matrix... rotate angle delta
		const float radcos = cosf(scatter); // rotational transform from current position to position at time t
		const float vxt = vel.x; 
		const float vyt = vel.y;
		vel.x = vxt *radcos - vyt *radsin;  // rotate to random scatter angle
		vel.y = vyt *radcos + vxt *radsin; 
	}

    //calc new rolling dynamics
    AngularAcceleration(hitNormal);
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

	float b = dv.Dot(d);                    // inner product
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

		if (a < 1.0e-8f) return -1.0f;				// ball moving really slow, then wait for contact

		const float c = bcddsq - totalradius*totalradius;	//first contact test: square delta position - square of radii
		b *= 2.0f;									// two inner products
		float result = b*b - 4.0f*a*c;				// squareroot term (discriminant) in quadratic equation

		if (result < 0.0f) return -1.0f;			// no collision path exist	

		result = sqrtf(result);

        // compute the two solutions to the quadratic equation
		      float time1 = (-b + result)/(2.0f * a);
		const float time2 = (-b - result)/(2.0f * a);

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
	const Vertex3Ds vnormal = coll->normal[0];

	if (pball->fFrozen) 
		return;

	// correct displacements, mostly from low velocity, alternative to true acceleration processing

    // target ball to object ball delta velocity
	const Vertex3Ds impulse = pball->collisionMass * pball->vel - collisionMass * vel;

	float dot = impulse.Dot(vnormal);

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

	const float averageMass = (collisionMass + pball->collisionMass)*0.5f;
	const float impulse1 = ((float)(-1.8 * 0.5) * dot) * pball->collisionMass / (averageMass * collisionMass);
	float impulse2 = ((float)(-1.8 * 0.5) * dot) * collisionMass / (averageMass * pball->collisionMass);

	if (!fFrozen)
	{
        vel -= impulse1 * vnormal;
		m_fDynamic = C_DYNAMIC;		
	}
	else impulse2 += impulse1;
		
    pball->vel += impulse2 * vnormal;
	pball->m_fDynamic = C_DYNAMIC;
}

void Ball::AngularAcceleration(const Vertex3Ds& hitnormal)
{
	const Vertex3Ds bccpd = -radius * hitnormal;    // vector ball center to contact point displacement

	const float bnv = vel.Dot(hitnormal);       // ball normal velocity to hit face

	const Vertex3Ds bvn = bnv * hitnormal;      // project the normal velocity along normal

	const Vertex3Ds bvt = vel - bvn;            // calc the tangent velocity

	Vertex3Ds bvT = bvt;                        // ball tangent velocity Unit Tangent
	bvT.Normalize();	

	const Vertex3Ds bstv =						// ball surface tangential velocity
	CrossProduct(m_angularvelocity, bccpd);		// velocity of ball surface at contact point

	const float dot = bstv.Dot(bvT);			// speed ball surface contact point tangential to contact surface point
	const Vertex3Ds cpvt = dot * bvT;           // contact point velocity tangential to hit face

	const Vertex3Ds slideVel = bstv - cpvt;     // contact point slide velocity with ball center velocity -- slide velocity

	// If the point and the ball are travelling in opposite directions,
	// and the point's velocity is at least the magnitude of the balls,
	// then we have a natural roll
	
	Vertex3Ds cpctrv = -slideVel;				//contact point co-tangential reverse velocity

    if (vel.LengthSquared() > (float)(0.7*0.7))
    {
        // Calculate the maximum amount the point velocity can change this
        // time segment due to friction
        Vertex3Ds FrictionForce = cpvt + bvt;

        // If the point can change fast enough to go directly to a natural roll, then do it.

        if (FrictionForce.LengthSquared() > (float)(ANGULARFORCE*ANGULARFORCE))
            FrictionForce.Normalize(ANGULARFORCE);

        cpctrv -= FrictionForce;
    }

	// Divide by the inertial tensor for a sphere in order to change
	// linear force into angular momentum
	cpctrv *= (float)(2.0/5.0); // Inertial tensor for a sphere

	const Vertex3Ds vResult = CrossProduct(bccpd, cpctrv); // ball center contact point displacement X reverse contact point co-tan vel

	m_angularmomentum *= 0.99f;
	m_angularmomentum += vResult; // add delta
	m_angularvelocity = m_inverseworldinertiatensor.MultiplyVector(m_angularmomentum);
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

		if (vel.z < 0.f && pos.z <= z_min)              //rolling point below the table and velocity driving deeper
		{
			pos.z = z_min;								// set rolling point to table surface
			vel.z *= -0.2f;							    // reflect velocity  ...  dull bounce

			vel.x *= c_hardFriction;
			vel.y *= c_hardFriction;					//friction other axiz
			
			const Vertex3Ds vnormal(0.0f,0.0f,1.0f);
			AngularAcceleration(vnormal);
		}
		else if (vel.z > 0.f && pos.z >= z_max)			//top glass ...contact and going higher
		{
			pos.z = z_max;								// set diametric rolling point to top glass
			vel.z *= -0.2f;								// reflect velocity  ...  dull bounce
		}

        // side walls are handled via actual collision objects set up in Player::CreateBoundingHitShapes

		CalcHitRect();
		
		Matrix3 mat3;
		mat3.CreateSkewSymmetric(m_angularvelocity);
		
		Matrix3 addedorientation;
		addedorientation.MultiplyMatrix(&mat3, &m_orientation);

		addedorientation.MultiplyScalar(dtime);

		m_orientation.AddMatrix(&addedorientation, &m_orientation);

		m_orientation.OrthoNormalize();

		Matrix3 matTransposeOrientation;
		m_orientation.Transpose(&matTransposeOrientation);
		m_inverseworldinertiatensor.MultiplyMatrix(&m_orientation,&m_inversebodyinertiatensor);
		m_inverseworldinertiatensor.MultiplyMatrix(&m_inverseworldinertiatensor,&matTransposeOrientation);

        m_angularvelocity = m_inverseworldinertiatensor.MultiplyVector(m_angularmomentum);
	}
}

void BallAnimObject::UpdateVelocities()
{
	m_pball->UpdateVelocities();
}

void Ball::UpdateVelocities()
{
	const float g = g_pplayer->m_gravity.z;
	float nx = g_pplayer->m_NudgeX;     // TODO: depends on STEPTIME
	float ny = g_pplayer->m_NudgeY;
	
	if(g_pplayer->m_NudgeManual >= 0) //joystick control of ball roll
	{
        vel *= NUDGE_MANUAL_FRICTION;   //rolling losses high for easy manual control

#define JOY_DEADBAND  5.0e-2f

		const float mag = nx*nx + ny*ny;// + nz*nz;
		if (mag > (JOY_DEADBAND * JOY_DEADBAND)) // joystick dead band, allows hold and very slow motion
		{
			const float inv = (mag > 0.0f) ? JOY_DEADBAND/sqrtf(mag) : 0.0f;
			nx -= nx*inv;	// remove deadband offsets
			ny -= ny*inv; 
			//nz -= nz*inv;

			vel.x += nx;
			vel.y += ny;
			vel.z += g;
		}
	} // manual joystick control
	else if (!fFrozen)  // Gravity	
	{
		vel.x += g_pplayer->m_gravity.x;
		vel.y += g_pplayer->m_gravity.y;

		if (pos.z > z_min + 0.05f || g > 0.f) // off the deck??? or gravity postive Z direction	
			vel.z += g;
		else
			vel.z += g * 0.001f;			  // don't add so much energy if already on the world floor

		vel.x += nx;
		vel.y += ny;
	}

	const float mag = vel.LengthSquared(); //speed check 
	const float antifrict = (mag > c_maxBallSpeedSqr) ? c_dampingFriction : 0.99875f;
		
	vel *= antifrict;      // speed damping

	m_fDynamic = C_DYNAMIC; // always set .. after adding velocity

	CalcHitRect();
}
