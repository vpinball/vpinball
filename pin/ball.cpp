#include "stdafx.h"

#ifndef ALTSTARTUPBALLCNT
static int _balls_created;

int Ball::NumInitted()  //
{
    return ( _balls_created );
}
#endif

Ball::Ball()
	{
#ifndef ALTSTARTUPBALLCNT
	_balls_created++; //
#endif
	m_pho = NULL;
	m_pballex = NULL;
	m_vpVolObjs = NULL; // should be NULL ... only real balls have this value
	m_Event_Pos.x = m_Event_Pos.y = m_Event_Pos.z = -1.0f;
	}

Ball::~Ball()	
{
#ifndef ALTSTARTUPBALLCNT
	_balls_created--; //Added by JEP.  Need to keep track of number of balls on table for autostart to work.
#endif
}

void Ball::Init()
	{
	const float tmp = (float)(5.0/2.0)/(radius*radius);
	// Only called by real balls, not temporary objects created for physics/rendering
	for (int i=0;i<3;i++)
		{
		for (int l=0;l<3;l++)
			{
			if (i==l)
				{
				m_orientation.m_d[i][l] = 1.0f;
				m_inversebodyinertiatensor.m_d[i][l] = tmp;
				}
			else
				{
				m_orientation.m_d[i][l] = 0;
				m_inversebodyinertiatensor.m_d[i][l] = 0;
				}
			}
		
		m_angularvelocity.m_d[i] = 0;
		m_angularmomentum.m_d[i] = 0;
		}

	m_ballanim.m_pball = this;

	fFrozen = false;

	// world limits on ball displacements
//	x_min = g_pplayer->m_ptable->m_left + radius;
//	x_max = g_pplayer->m_ptable->m_right - radius;
//	y_min = g_pplayer->m_ptable->m_top + radius;
//	y_max = g_pplayer->m_ptable->m_bottom - radius;
	z_min = g_pplayer->m_ptable->m_tableheight + radius;
	z_max = g_pplayer->m_ptable->m_glassheight - radius;

	m_fErase = false;

	m_pho = NULL;
	m_fDynamic = C_DYNAMIC; //rlc assume dynamic

	m_pballex = NULL;

	m_vpVolObjs = new VectorVoid;

	m_color = RGB(255,255,255);
	
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

	if (g_pplayer->m_ptable->m_szBallImageBack[0] == '\0')
		{
		m_szImageBack[0] = '\0';
		m_pinBack = NULL;
		}
	else
		{
		lstrcpy(m_szImageBack, g_pplayer->m_ptable->m_szBallImageBack);
		m_pinBack = g_pplayer->m_ptable->GetImage(m_szImageBack);
		}
	}


void Ball::EnsureOMObject()
	{
	if (m_pballex)
		{
		return;
		}

	CComObject<BallEx>::CreateInstance(&m_pballex);
	m_pballex->AddRef();

	m_pballex->m_pball = this;
	}
 
void Ball::CalcBoundingRect()
	{
	const float dx = fabsf(vx);
	const float dy = fabsf(vy);

	brc.left = x - (radius + 0.1f + dx); //rlc make more accurate ????
	brc.top = y - (radius + 0.1f + dy);
	brc.right = x + (radius + 0.1f + dx);
	brc.bottom = y + (radius + 0.1f + dy);

	m_rcHitRect.left = brc.left;
	m_rcHitRect.top = brc.top;
	m_rcHitRect.right = brc.right;
	m_rcHitRect.bottom = brc.bottom;

	m_rcHitRect.zlow = min(z, z+vz);
	m_rcHitRect.zhigh = max(z, z+vz);

	m_rcHitRect.zhigh += radius + 0.1f;
	m_rcHitRect.zlow -= radius /*+ 0.f*/;
	}

void Ball::CollideWall(const Vertex3Ds * const phitnormal, const float m_elasticity, float antifriction, float scatter_angle)
	{
	float dot = vx * phitnormal->x + vy * phitnormal->y; //speed normal to wall

	if (dot >= -C_LOWNORMVEL )							// nearly receding ... make sure of conditions
		{												// otherwise if clearly approaching .. process the collision
		if (dot > C_LOWNORMVEL) return;					//is this velocity clearly receding (i.e must > a minimum)		
#ifdef C_EMBEDDED
		if (m_HitDist < -C_EMBEDDED)
			dot = -C_EMBEDSHOT;							// has ball become embedded???, give it a kick
		else return;
#endif
		} 
		
#ifdef C_DISP_GAIN 
		float hdist = -C_DISP_GAIN * m_HitDist;			// limit delta noise crossing ramps,
		if (hdist > 1.0e-4f)
			{
			if (hdist > C_DISP_LIMIT) 
				hdist = C_DISP_LIMIT;	// crossing ramps, delta noise
			x += hdist * phitnormal->x;	// push along norm, back to free area
			y += hdist * phitnormal->y;	// use the norm, but this is not correct, reverse time is correct
			}
#endif		

	dot *= -1.005f - m_elasticity; //rlc some small minimum
	vx += dot * phitnormal->x;	
	vy += dot * phitnormal->y;

	if (antifriction >= 1.0f || antifriction <= 0) 
		antifriction = c_hardFriction; // use global

	vx *= antifriction; vy *= antifriction; vz *= antifriction; //friction all axiz

	if (scatter_angle <= 0) scatter_angle = c_hardScatter;				// if <= 0 use global value
	scatter_angle *= g_pplayer->m_ptable->m_globalDifficulty;			// apply dificulty weighting

	if (dot > 1.0f && scatter_angle > 1.0e-5f) //no scatter at low velocity 
		{
		float scatter = 2.0f* ((float)rand()*(float)(1.0/RAND_MAX) - 0.5f);  // -1.0f..1.0f
		scatter *= (1.0f - scatter*scatter)*2.59808f * scatter_angle;	// shape quadratic distribution and scale
		const float radsin = sinf(scatter);//  Green's transform matrix... rotate angle delta 
		const float radcos = cosf(scatter);//  rotational transform from current position to position at time t
		const float vxt = vx; 
		const float vyt = vy;

		vx = vxt *radcos - vyt *radsin;  // rotate to random scatter angle
		vy = vyt *radcos + vxt *radsin;  // 
		}

	Vertex3Ds vnormal;
	vnormal.Set(phitnormal->x, phitnormal->y, 0); //??? contact point 
	AngularAcceleration(&vnormal);	//calc new rolling dynamics
	}


void Ball::Collide3DWall(const Vertex3Ds * const phitnormal, const float m_elasticity, float antifriction, float scatter_angle)
	{
	float dot = vx * phitnormal->x + vy * phitnormal->y + vz * phitnormal->z; //speed normal to wall

	if (dot >= -C_LOWNORMVEL )								// nearly receding ... make sure of conditions
		{													// otherwise if clearly approaching .. process the collision
		if (dot > C_LOWNORMVEL) return;						//is this velocity clearly receding (i.e must > a minimum)	

#ifdef C_EMBEDDED
		if (m_HitDist < -C_EMBEDDED)
			dot = -C_EMBEDSHOT;		// has ball become embedded???, give it a kick
		else return;
#endif
		}
		
#ifdef C_DISP_GAIN 		
	// correct displacements, mostly from low velocity, alternative to acceleration processing
	float hdist = -C_DISP_GAIN * m_HitDist;			// limit delta noise crossing ramps, 
	if (hdist > 1.0e-4f)					// when hit detection checked it what was the displacement
		{			
		if (hdist > C_DISP_LIMIT) 
			hdist = C_DISP_LIMIT;	// crossing ramps, delta noise			
		x += hdist * phitnormal->x;	// push along norm, back to free area
		y += hdist * phitnormal->y;	// use the norm, but his is not correct
		z += hdist * phitnormal->z;	// 
		}	
#endif					

	if (antifriction >= 1.0f || antifriction <= 0) 
		antifriction = c_hardFriction; // use global

	dot *= -1.005f - m_elasticity;	
	vx += dot * phitnormal->x; vx *= antifriction;
	vy += dot * phitnormal->y; vy *= antifriction;
	vz += dot * phitnormal->z; vz *= antifriction;

	
	if (scatter_angle <= 0) scatter_angle = c_hardScatter;						// if <= zero use global value
	scatter_angle *= g_pplayer->m_ptable->m_globalDifficulty;	// apply dificulty weighting
	
	if (dot > 1.0f && scatter_angle > 1.0e-5f) //no scatter at low velocity 
		{
		float scatter = 2.0f* ((float)rand()*(float)(1.0/RAND_MAX) - 0.5f);  // -1.0f..1.0f
		scatter *= (1.0f - scatter*scatter)*2.59808f * scatter_angle;	// shape quadratic distribution and scale
		const float radsin = sinf(scatter);//  Green's transform matrix... rotate angle delta 
		const float radcos = cosf(scatter);//  rotational transform from current position to position at time t
		const float vxt = vx; 
		const float vyt = vy;

		vx = vxt *radcos - vyt *radsin;  // rotate to random scatter angle
		vy = vyt *radcos + vxt *radsin;  //
		}

	AngularAcceleration(phitnormal);	// calc new rolling dynmaics
	}


float Ball::HitTest(Ball *pball, float dtime, Vertex3Ds *phitnormal) //rlc change begin >>>>>>>>>>>>>>>>>>>>>>
	{	
	float dvx = vx - pball->vx;		 // delta velocity 
	float dvy = vy - pball->vy;
	float dvz = vz - pball->vz;

	float dx = x - pball->x;				// delta position
	float dy = y - pball->y;
	float dz = z - pball->z;

	float bcddsq = dx*dx + dy*dy + dz*dz;	//square of ball center's delta distance

	float bcdd = sqrtf(bcddsq);				// length of delta
	if (bcdd < 1.0e-8f)						// two balls center-over-center embedded
		{ //return -1;
		dz = -1.0f;							// patch up			
		pball->z -= dz;						//lift up
		
		bcdd = 1.0f;						//patch up
		bcddsq = 1.0f;						//patch up
		dvz = 0.1f;							// small speed difference
		pball->vz -= dvz;
		}

	float b = dvx*dx + dvy*dy + dvz*dz;		// inner product
	const float bnv = b/bcdd;				// normal speed of balls toward each other

	if ( bnv >  C_LOWNORMVEL) return -1.0f;	// dot of delta velocity and delta displacement, postive if receding no collison

	const float totalradius = pball->radius + radius;
	const float bnd = bcdd - totalradius;

	float hittime;
	if (bnd < (float)PHYS_TOUCH)			// in contact??? 
		{
		if (bnd <= (float)(-PHYS_SKIN*2.0))
			return -1.0f;					// embedded too deep

		if (fabsf(bnv) > C_CONTACTVEL)			// >fast velocity, return zero time
			hittime = 0;						//zero time for rigid fast bodies
		else if (bnd <= (float)(-PHYS_TOUCH))
			hittime = 0;						// slow moving but embedded
		else
			hittime = bnd*(float)(1.0/(2.0*PHYS_TOUCH)) + 0.5f;		// don't compete for fast zero time events
		}
	else
		{
		float a = dvx*dvx + dvy*dvy + dvz*dvz;				//square of differential velocity 
		const float c = bcddsq - totalradius*totalradius;	//first contact test: square delta position - square of radii

		if (a < 1.0e-12f) return -1.0f;				// ball moving really slow, then wait for contact

		b += b;										// two inner products
		float result = b*b - 4.0f*a*c;				// squareroot term in Quadratic equation

		if (result < 0.0f) return -1.0f;			// no collision path exist	

		result = sqrtf(result);
		a += a;										// optimize calculation

		const float inv_a = 1.0f/a;
		float time1 = (-b + result)*inv_a;
		float time2 = (-b - result)*inv_a;

		if (time1 < 0) time1 = time2;				// if time1 negative, assume time2 postive

		hittime = (time1 < time2) ? time1 : time2;	// select lessor
													// if time2 is negative ... 

		if (hittime < 0 || hittime > dtime) return -1.0f; // .. was some time previous || beyond the next physics tick
		}

	const float hitx = pball->x + dvx * hittime;  // new ball position
	const float hity = pball->y + dvy * hittime;
	const float hitz = pball->z + dvz * hittime;
	
	const float inv_len = 1.0f/sqrtf((hitx - x)*(hitx - x)+(hity - y)*(hity - y)+(hitz - z)*(hitz - z));

	phitnormal->x = (hitx - x)*inv_len;	//calc unit normal of collision
	phitnormal->y = (hity - y)*inv_len;
	phitnormal->z = (hitz - z)*inv_len;

	m_HitDist = bnd;					//actual contact distance 
	m_HitNormVel = bnv;
	m_HitRigid = true;					//rigid collision type

	return hittime;	
	}


void Ball::Collide(Ball *pball, Vertex3Ds *phitnormal)
	{
	if (pball->fFrozen) 
		return;

	const Vertex3Ds vnormal = *phitnormal;
	
	// correct displacements, mostly from low velocity, alternative to true acceleration processing

	Vertex3Ds vel;
	vel.x = pball->vx -vx;  //target ball to object ball
	vel.y = pball->vy -vy;  //delta velocity
	vel.z = pball->vz -vz;
	
	float dot = vel.x * vnormal.x + vel.y * vnormal.y + vel.z * vnormal.z;

	if (dot >= -C_LOWNORMVEL )								// nearly receding ... make sure of conditions
		{													// otherwise if clearly approaching .. process the collision
		if (dot > C_LOWNORMVEL) return;						//is this velocity clearly receding (i.e must > a minimum)		
#ifdef C_EMBEDDED
		if (pball->m_HitDist < -C_EMBEDDED)
			dot = -C_EMBEDSHOT;		// has ball become embedded???, give it a kick
		else return;
#endif
		}
			
#ifdef C_DISP_GAIN 		
		float edist = -C_DISP_GAIN * pball->m_HitDist; // 
		if (edist > 1.0e-4f)
			{										
			if (edist > C_DISP_LIMIT) 
				edist = C_DISP_LIMIT;		// crossing ramps, delta noise
			if (!fFrozen) edist *= 0.5f;	// if the hitten ball is not frozen
			pball->x += edist * vnormal.x;	// push along norm, back to free area
			pball->y += edist * vnormal.y;	// use the norm, but is not correct, but cheaply handled
			pball->z += edist * vnormal.z;	// 
			}

		edist = -C_DISP_GAIN * m_HitDist;	// noisy value .... needs investigation
		if (!fFrozen && edist > 1.0e-4f)
			{ 
			if (edist > C_DISP_LIMIT) 
				edist = C_DISP_LIMIT;		// crossing ramps, delta noise
			edist *= 0.5f;		
			x -= edist * vnormal.x;			// pull along norm, back to free area
			y -= edist * vnormal.y;			// use the norm
			z -= edist * vnormal.z;			//
			}
#endif				

	float impulse = (float)(-1.8 * 0.5) * dot;

	if (!fFrozen)
		{
		vx -= impulse * vnormal.x;
		vy -= impulse * vnormal.y;
		vz -= impulse * vnormal.z;
		m_fDynamic = C_DYNAMIC;		
		}
	else impulse += impulse;
		
	pball->vx += impulse * vnormal.x;
	pball->vy += impulse * vnormal.y;
	pball->vz += impulse * vnormal.z;
	pball->m_fDynamic = C_DYNAMIC;
	}

void Ball::AngularAcceleration(const Vertex3Ds * const phitnormal)
	{
	Vertex3Ds bccpd; // vector ball center to contact point displacement
	bccpd.Set(-radius * phitnormal->x, -radius * phitnormal->y, -radius * phitnormal->z); //from ball center to contact point

	const float bnv = vx * phitnormal->x + vy * phitnormal->y + vz * phitnormal->z; //ball normal velocity to hit face

	Vertex3Ds bvn;
	bvn.x = bnv * phitnormal->x;						//project the normal velocity along normal
	bvn.y = bnv * phitnormal->y;
	bvn.z = bnv * phitnormal->z;

	Vertex3Ds bvt;
	bvt.x = vx - bvn.x;									// calc the tangent velocity
	bvt.y = vy - bvn.y;
	bvt.z = vz - bvn.z;

	Vertex3Ds bvT;										// ball tangent velocity Unit Tangent
	bvT.Set(bvt.x, bvt.y, bvt.z);						//copy ball tangent velocity
	bvT.Normalize();	

	Vertex3Ds bstv;										// ball surface tangential velocity

	CrossProduct(&m_angularvelocity, &bccpd, &bstv);	// velocity of ball surface at contact point

	Vertex3Ds cpvt;						// contact point velocity tangential to hit face

	const float dot = bstv.Dot(&bvT);	// speed ball surface contact point tangential to contact surface point
	cpvt.x = bvT.x * dot;				//contact point velocity tangent to hit face
	cpvt.y = bvT.y * dot;
	cpvt.z = bvT.z * dot;

	Vertex3Ds slideVel;	// contact point slide velocity with ball center velocity
	slideVel.x = bstv.x - cpvt.x;  // slide velocity
	slideVel.y = bstv.y - cpvt.y;
	slideVel.z = bstv.z - cpvt.z;

	m_angularmomentum.MultiplyScalar(0.99f);

	// If the point and the ball are travelling in opposite directions,
	// and the point's velocity is at least the magnitude of the balls,
	// then we have a natural rool
	
	Vertex3Ds cpctrv;
	cpctrv.x = -slideVel.x;	//contact point co-tangential reverse velocity
	cpctrv.y = -slideVel.y;
	cpctrv.z = -slideVel.z;

	// Calculate the maximum amount the point velocity can change this
	// time segment due to friction
	Vertex3Ds FrictionForce;
	FrictionForce.x = cpvt.x + bvt.x;
	FrictionForce.y = cpvt.y + bvt.y;
	FrictionForce.z = cpvt.z + bvt.z;

#define ANGULARFORCE 1   
					// Number I pulled out of my butt - this number indicates the maximum angular change 
					//per time unit, or the speed at which angluar momentum changes over time, 
					//depending on how you interpret it.

	// If the point can change fast enough to go directly to a natural roll, then do it.

	if (FrictionForce.LengthSquared() > (float)(ANGULARFORCE*ANGULARFORCE))
		{
		FrictionForce.Normalize();
		FrictionForce.MultiplyScalar(ANGULARFORCE);
		}

	if (vx*vx + vy*vy + vz*vz > (float)(0.7*0.7))
		{
		cpctrv.x -= FrictionForce.x;
		cpctrv.y -= FrictionForce.y;
		cpctrv.z -= FrictionForce.z;
		}

	// Divide by the inertial tensor for a sphere in order to change
	// linear force into angular momentum
	cpctrv.x *= (float)(1.0/2.5); // Inertial tensor for a sphere
	cpctrv.y *= (float)(1.0/2.5);
	cpctrv.z *= (float)(1.0/2.5);

	Vertex3Ds vResult;
	CrossProduct(&bccpd, &cpctrv, &vResult);//ball center contact point displacement X reverse contact point co-tan vel

	m_angularmomentum.Add(&vResult);	// add delta 

	m_inverseworldinertiatensor.MultiplyVector(&m_angularmomentum, &m_angularvelocity);
	}

void Ball::CalcHitRect()
	{
	}

void BallAnimObject::UpdateDisplacements(float dtime)
	{
	m_pball->UpdateDisplacements(dtime);
	}

void Ball::UpdateDisplacements(float dtime)
	{
    	
	if (!fFrozen)
		{
#ifdef _DEBUGXXX
		if (_isnan(x) || _isnan(y) || _isnan(z)|| _isnan(vx) || _isnan(vy) || _isnan(vz))
			{
				static int devx = -3;
				static int devy = -3;

				if (++devx > 3) devx = -3;
				if (++devy > 3) devy = -3;

				x = x_max/2 + devx*50; vx = 0; 
				y = y_max/2 + devy*50; vy = 0;

				z = z_max/2; vz = 0;
			}
#endif
		const float dsx = vx * dtime;
		const float dsy = vy * dtime;
		const float dsz = vz * dtime;
		x += dsx;
		y += dsy;
		z += dsz;

		drsq = dsx*dsx + dsy*dsy + dsz*dsz;				// used to determine if static ball

		if (vz < 0 && z <= z_min)						//rolling point below the table and velocity driving deeper
			{
			z = z_min;									// set rolling point to table surface
			vz *= -0.2f;							    // reflect velocity  ...  dull bounce

			vx *= c_hardFriction; vy *= c_hardFriction;  //friction other axiz
			
			Vertex3Ds vnormal;
			vnormal.Set(0.0f,0.0f,1.0f);
			AngularAcceleration(&vnormal);
			}
		else if (vz > 0 && z >= z_max)						//top glass ...contact and going higher
			{
			z = z_max;									// set diametric rolling point to top glass
			vz *= -0.2f;								// reflect velocity  ...  dull bounce
			}

/*		if (vx < 0 && x <= x_min)						//left wall
			{
			x = x_min;									
			vx *= -0.2f;
			}
		else if (vx > 0 && x >= x_max)				//right wall
			{
			x = x_max;							
			vx *= -0.2f;
			}

		if (vy < 0 && y <= y_min)					//top wall
			{
			y = y_min;									
			vy *= -0.2f;
			}
		else if (vy > 0 && y >= y_max)				//bottom wall
			{
			y = y_max;							
			vy *= -0.2f;
			}
*/	
		CalcBoundingRect();
		
		Matrix3 mat3;
		mat3.CreateSkewSymmetric(&m_angularvelocity);
		
		Matrix3 addedorientation;
		addedorientation.MultiplyMatrix(&mat3, &m_orientation);

		addedorientation.MultiplyScalar(dtime);

		m_orientation.AddMatrix(&addedorientation, &m_orientation);

		m_orientation.OrthoNormalize();

		Matrix3 matTransposeOrientation;
		m_orientation.Transpose(&matTransposeOrientation);
		m_inverseworldinertiatensor.MultiplyMatrix(&m_orientation,&m_inversebodyinertiatensor);
		m_inverseworldinertiatensor.MultiplyMatrix(&m_inverseworldinertiatensor,&matTransposeOrientation);

        m_inverseworldinertiatensor.MultiplyVector(&m_angularmomentum, &m_angularvelocity);
		}
	}

void BallAnimObject::UpdateVelocities(float dtime)
	{
	m_pball->UpdateVelocities(dtime);
	}

void Ball::UpdateVelocities(float dtime)
	{
	///  dtime is always 1.0	
	const float g = g_pplayer->m_mainlevel.m_gravity.z;
	float nx = g_pplayer->m_NudgeX;
	float ny = g_pplayer->m_NudgeY;
	
	if( g_pplayer && g_pplayer->m_NudgeManual >= 0) //joystick control of ball roll
		{
		vx *= 0.92f;//*dtime;	//rolling losses high for easy manual control
		vy *= 0.92f;//*dtime;
		vz *= 0.92f;//*dtime;	

#define JOY_DEADBAND  5.0e-2f

		const float mag = nx*nx + ny*ny;// + nz*nz;
		if (mag > (JOY_DEADBAND * JOY_DEADBAND))			//joystick dead band, allows hold and very slow motion
			{
			const float inv = JOY_DEADBAND/sqrtf(mag);
			nx -= nx*inv;	// remove deadband offsets
			ny -= ny*inv; 
			//nz -= nz*inv;

			vx += nx;// *dtime;
			vy += ny;// *dtime;
			vz += g;// *dtime;//-c_Gravity;
			}
		}//manual joystick control
	else if (!fFrozen)  // Gravity	
		{
		vx += g_pplayer->m_mainlevel.m_gravity.x;// *dtime;	
		vy += g_pplayer->m_mainlevel.m_gravity.y;// *dtime;	

		if (z > z_min + 0.05f || g > 0)// off the deck??? or gravity postive Z direction	
			vz += g;
		else vz += g *0.001f;			// don't add so much energy if already on the world floor

		vx += nx;// *dtime;
		vy += ny;// *dtime;
		}

		const float mag = vx*vx + vy*vy + vz*vz; //speed check 
		const float antifrict = (mag > c_maxBallSpeedSqed) ? c_dampingFriction : 0.99875f;
		
		vx *= antifrict;//*dtime;	// speed damping
		vy *= antifrict;//*dtime;
		vz *= antifrict;//*dtime;	
				
	m_fDynamic = C_DYNAMIC;		// always set .. after adding velocity

	CalcBoundingRect();
	}
