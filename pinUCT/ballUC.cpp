#include "stdafx.h"

#ifndef ALTSTARTUPBALLCNT

int _balls_created = 0;

int Ball::NumInitted( void )  //
{
    return ( _balls_created );
}
#endif

Ball::Ball()	//many Balls are temporary objects, init Real ball below in Init()
	{
#ifndef ALTSTARTUPBALLCNT
	_balls_created++; //rlc moved below ... this should count only real balls
#endif
	m_pho = NULL;
	m_pballex = NULL;
	m_vpVolObjs = NULL; // should be NULL ... only real balls have this value
	m_contacts = 0;
	m_Event_Pos.x = m_Event_Pos.y = m_Event_Pos.z = -1;
	IsBlurReady = false;
	}

Ball::~Ball()	
{
#ifndef ALTSTARTUPBALLCNT
	if (this)
	{
		_balls_created--; // Update number of balls for autostart to work.
	}
#endif
}

void Ball::Init()
	{
	// Only called by real balls, not temporary objects created for physics/rendering
	int i;
	int l;

	for (i=0;i<3;i++)
		{
		for (l=0;l<3;l++)
			{
			if (i==l)
				{
				m_orientation.m_d[i][l] = 1;
				m_inversebodyinertiatensor.m_d[i][l] = 1/(2*1*radius*radius/5.0f);
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

	IsBlurReady = false;
	prev_x = 0.0f;
	prev_y = 0.0f;
	prev_z = 0.0f;

	fFrozen = fFalse;
	m_contacts = 0;

	// world limits on ball displacements
	x_min = g_pplayer->m_ptable->m_left + radius;		
	x_max = g_pplayer->m_ptable->m_right - radius;	
	y_min = g_pplayer->m_ptable->m_top + radius;		
	y_max = g_pplayer->m_ptable->m_bottom - radius;	
	z_min = g_pplayer->m_ptable->m_tableheight + radius;
	z_max = g_pplayer->m_ptable->m_glassheight - radius;

	m_fErase = fFalse;

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
	PINFLOAT dx = fabsf(vx);
	PINFLOAT dy = fabsf(vy);

	brc.left = (float)(x - radius - dx - 0.1); //rlc make more accurate ????
	brc.top = (float)(y - radius - dy - 0.1);
	brc.right = (float)(x + radius + dx + 0.1);
	brc.bottom = (float)(y + radius + dy + 0.1);

	m_rcHitRect.left = brc.left;
	m_rcHitRect.top = brc.top;
	m_rcHitRect.right = brc.right;
	m_rcHitRect.bottom = brc.bottom;

	m_rcHitRect.zlow = (float)min(z, z+vz);
	m_rcHitRect.zhigh = (float)max(z, z+vz);


	m_rcHitRect.zhigh += (float)(radius + 0.1);
	m_rcHitRect.zlow -= (float)(radius + 0.f);
	}

void Ball::CollideWall(Vertex3D *phitnormal, float m_elasticity, float antifriction, float scatter_angle)
	{
	PINFLOAT dot = vx * phitnormal->x + vy * phitnormal->y; //speed normal to wall

	if (dot >= -C_LOWNORMVEL )							// nearly receeding ... make sure of conditions
		{												// otherwise if clearly approaching .. process the collision
		if (dot > C_LOWNORMVEL) return;					//is this velocity clearly receeding (i.e must > a minimum)		
#ifdef C_EMBEDDED
		if (m_HitDist < -C_EMBEDDED)
			dot = -C_EMBEDSHOT;							// has ball become embedded???, give it a kick
		else return;
#endif
		} 
		
#ifdef C_DISP_GAIN 
		float hdist = -C_DISP_GAIN * m_HitDist;			// limit delta noise crossing ramps,
		if (hdist > 1.0e-4)
			{
			if (hdist > C_DISP_LIMIT) 
				{hdist = C_DISP_LIMIT;}	// crossing ramps, delta noise
			x += hdist * phitnormal->x;	// push along norm, back to free area
			y += hdist * phitnormal->y;	// use the norm, but this is not correct, reverse time is correct
			}
#endif		

	dot *= -(1.005f + m_elasticity);//rlc some small minimum
	vx += dot * phitnormal->x;	
	vy += dot * phitnormal->y; 

	if (antifriction >= 1.0f || antifriction <= 0) 
		{antifriction = c_hardFriction;} // use global

	vx *= antifriction; vy *= antifriction; vz *= antifriction; //friction all axiz

	if (scatter_angle <= 0) scatter_angle = c_hardScatter;				// if <= 0 use global value
	scatter_angle *= g_pplayer->m_ptable->m_globalDifficulty;			// apply dificulty weighting

	if (dot > 1.0f && scatter_angle > 1.0e-5) //no scatter at low velocity 
		{
		float scatter = 2.0f* ((float)rand()/((float)RAND_MAX) - 0.5f);  // -1.0f..1.0f
		scatter *=  (1.0f - scatter*scatter)*2.59808f * scatter_angle;	// shape quadratic distribution and scale
		float radsin = sin(scatter);//  Green's transform matrix... rotate angle delta 
		float radcos = cos(scatter);//  rotational transform from current position to position at time t
		float vxt = vx; 
		float vyt = vy;

		vx = vxt *radcos - vyt *radsin;  // rotate to random scatter angle
		vy = vyt *radcos + vxt *radsin;  // 
		}

	Vertex3D vnormal;
	vnormal.Set(phitnormal->x, phitnormal->y, 0); //??? contact point 

	AngularAcceleration(&vnormal);	//calc new rolling dynamics
	}


void Ball::Collide3DWall(Vertex3D *phitnormal, float m_elasticity, float antifriction, float scatter_angle)
	{
	PINFLOAT dot = vx * phitnormal->x + vy * phitnormal->y + vz * phitnormal->z; //speed normal to wall

	if (dot >= -C_LOWNORMVEL )								// nearly receeding ... make sure of conditions
		{													// otherwise if clearly approaching .. process the collision
		if (dot > C_LOWNORMVEL) return;						//is this velocity clearly receeding (i.e must > a minimum)	

#ifdef C_EMBEDDED
		if (m_HitDist < -C_EMBEDDED)
			dot = -C_EMBEDSHOT;		// has ball become embedded???, give it a kick
		else return;
#endif
		}
		
#ifdef C_DISP_GAIN 		
	// correct displacements, mostly from low velocity, alternative to acceleration processing
	float hdist = -C_DISP_GAIN * m_HitDist;			// limit delta noise crossing ramps, 
	if (hdist > 1.0e-4)					// when hit detection checked it what was the displacement
		{			
		if (hdist > C_DISP_LIMIT) 
			{hdist = C_DISP_LIMIT;}	// crossing ramps, delta noise			
		x += hdist * phitnormal->x;	// push along norm, back to free area
		y += hdist * phitnormal->y;	// use the norm, but his is not correct
		z += hdist * phitnormal->z;	// 
		}	
#endif					

	if (antifriction >= 1.0f || antifriction <= 0) 
		{antifriction = c_hardFriction;} // use global

	dot *= -(1.005f + m_elasticity);	
	vx += dot * phitnormal->x; vx *= antifriction;
	vy += dot * phitnormal->y; vy *= antifriction;
	vz += dot * phitnormal->z; vz *= antifriction;

	
	if (scatter_angle <= 0) scatter_angle = c_hardScatter;						// if <= zero use global value
	scatter_angle *= g_pplayer->m_ptable->m_globalDifficulty;	// apply dificulty weighting
	
	if (dot > 1.0f && scatter_angle > 1.0e-5) //no scatter at low velocity 
		{
		float scatter = 2.0f* ((float)rand()/((float)RAND_MAX) - 0.5f);  // -1.0f..1.0f
		scatter *=  (1.0f - scatter*scatter)*2.59808f  * scatter_angle;	// shape quadratic distribution and scale
		float radsin = sin(scatter);//  Green's transform matrix... rotate angle delta 
		float radcos = cos(scatter);//  rotational transform from current position to position at time t
		float vxt = vx; 
		float vyt = vy;

		vx = vxt *radcos - vyt *radsin;  // rotate to random scatter angle
		vy = vyt *radcos + vxt *radsin;  //
		}

	AngularAcceleration(phitnormal);	// calc new rolling dynmaics
	}


PINFLOAT Ball::HitTest(Ball *pball, PINFLOAT dtime, Vertex3D *phitnormal) //rlc change begin >>>>>>>>>>>>>>>>>>>>>>
	{	
	PINFLOAT hittime;
	PINFLOAT a,b,c,bcdd,bnv;
	PINFLOAT result;

	PINFLOAT dvx = vx - pball->vx;		 // delta velocity 
	PINFLOAT dvy = vy - pball->vy;
	PINFLOAT dvz = vz - pball->vz;

	PINFLOAT dx = x - pball->x;					// delta position
	PINFLOAT dy = y - pball->y;
	PINFLOAT dz = z - pball->z;

	PINFLOAT bcddsq = dx*dx + dy*dy + dz*dz;	//square of ball center's delta distance

	bcdd = sqrt(bcddsq);						// length of delta
	if (bcdd < (float)1.0e-8)					// two balls center-over-center embedded
		{ //return -1;
		dz = -1.0f;								// patch up			
		pball->z -= dz;							//lift up
		
		bcdd = 1.0f;							//patch up
		bcddsq = 1.0f;							//patch up
		dvz = 0.1f;								// small speed difference
		pball->vz -= dvz;
		}

	b = dvx*dx + dvy*dy + dvz*dz;				// inner product
	bnv = b/bcdd;								// normal speed of balls toward each other

	if ( bnv >  C_LOWNORMVEL) return -1;		// dot of delta velocity and delta displacement, postive if receeding no collison

	PINFLOAT totalradius = pball->radius + radius;
	PINFLOAT bnd = bcdd - totalradius;

	if (bnd < PHYS_TOUCH)								// in contact??? 
		{
		if (bnd <= -PHYS_SKIN *2) 	
			{return -1;}				// embedded too deep

		if (fabsf(bnv) > C_CONTACTVEL)		// >fast velocity, return zero time
			hittime = 0;								//zero time for rigid fast bodies
		else if(bnd <= -PHYS_TOUCH) hittime = 0;						// slow moving but embedded
		else hittime = (bnd + PHYS_TOUCH) * (1.0f/PHYS_TOUCH/2);		// don't compete for fast zero time events
		}
	else
		{	
		a = dvx*dvx + dvy*dvy + dvz*dvz;				//square of differential velocity 
		c = bcddsq - totalradius*totalradius;			//first contact test: square delta position - square of radii

		if (a < (float)1.0e-12) return -1;				// ball moving really slow, then wait for contact

		b *= 2;;										// two inner products
		result = (b*b) - (4*a*c);						// squareroot term in Quadratic equation

		if (result < 0)	return -1;						// no collision path exist	

		result = sqrt(result); a = 2*a;					// optimize calculation

		PINFLOAT time1 = (-b + result)/a;
		PINFLOAT time2 = (-b - result)/a;

		if (time1 < 0) time1 = time2;				// if time1 negative, assume time2 postive

		if (time1 < time2) hittime = time1;			// select lessor
		else hittime = time2;						// if time2 is negative ... 

		if (hittime < 0 || hittime > dtime) return -1; // .. was some time previous || beyond the next physics tick
		}

	PINFLOAT hitx = pball->x + dvx * hittime;  // new ball position
	PINFLOAT hity = pball->y + dvy * hittime;
	PINFLOAT hitz = pball->z + dvz * hittime;
	
	PINFLOAT len = sqrt((hitx - x)*(hitx - x)+(hity - y)*(hity - y)+(hitz - z)*(hitz - z));

	((Vertex3D *)phitnormal)->x = (hitx - x)/len;	//calc unit normal of collision
	((Vertex3D *)phitnormal)->y = (hity - y)/len;
	((Vertex3D *)phitnormal)->z = (hitz - z)/len;

	m_HitDist = bnd;								//actual contact distance 
	m_HitNormVel = bnv;
	m_HitRigid = true;								//rigid collision type

	return hittime;	
	}


void Ball::Collide(Ball *pball, Vertex3D *phitnormal)
	{
	if (pball->fFrozen) 
		{return;}

	Vertex3D vel;
	Vertex3D vnormal = *(Vertex3D *)phitnormal;
	
	// correct displacements, mostly from low velocity, alternative to true acceleration processing

	vel.x = pball->vx -vx;  //target ball to object ball
	vel.y = pball->vy -vy;  //delta velocity
	vel.z = pball->vz -vz;
	
	float dot = vel.x * vnormal.x + vel.y * vnormal.y + vel.z * vnormal.z;

	if (dot >= -C_LOWNORMVEL )								// nearly receeding ... make sure of conditions
		{													// otherwise if clearly approaching .. process the collision
		if (dot > C_LOWNORMVEL) return;						//is this velocity clearly receeding (i.e must > a minimum)		
#ifdef C_EMBEDDED
		if (pball->m_HitDist < -C_EMBEDDED)
			dot = -C_EMBEDSHOT;		// has ball become embedded???, give it a kick
		else return;
#endif
		}
			
#ifdef C_DISP_GAIN 		
		float edist = -C_DISP_GAIN * pball->m_HitDist; // 
		if (edist > 1.0e-4)
			{										
			if (edist > C_DISP_LIMIT) 
				{edist = C_DISP_LIMIT;}		// crossing ramps, delta noise
			if (!fFrozen) edist /= 2;		// if the hitten ball is not frozen
			pball->x += edist * vnormal.x;	// push along norm, back to free area
			pball->y += edist * vnormal.y;	// use the norm, but is not correct, but cheaply handled
			pball->z += edist * vnormal.z;	// 
			}

		edist = -C_DISP_GAIN * m_HitDist;	// noisy value .... needs investigation
		if (!fFrozen && edist > 1.0e-4)
			{ 
			if (edist > C_DISP_LIMIT) 
				{edist = C_DISP_LIMIT;}		// crossing ramps, delta noise
			edist /= 2;		
			x -= edist * vnormal.x;			// pull along norm, back to free area
			y -= edist * vnormal.y;			// use the norm
			z -= edist * vnormal.z;			//
			}
#endif				

	float impulse = -(float)1.8f * dot / 2;

	if (!fFrozen)
		{
		vx -= impulse * vnormal.x;
		vy -= impulse * vnormal.y;
		vz -= impulse * vnormal.z;
		m_fDynamic = C_DYNAMIC;		
		}
	else impulse *= 2;	
		
	pball->vx += impulse * vnormal.x;
	pball->vy += impulse * vnormal.y;
	pball->vz += impulse * vnormal.z;
	pball->m_fDynamic = C_DYNAMIC;

	}

void Ball::AngularAcceleration(Vertex3D *phitnormal)
	{

	Vertex3D bccpd; // vector ball center to contact point displacement
	bccpd.Set(-radius * phitnormal->x, -radius * phitnormal->y, -radius * phitnormal->z); //from ball center to contact point

	PINFLOAT bnv = vx * phitnormal->x + vy * phitnormal->y + vz * phitnormal->z; //ball normal velocity to hit face

	Vertex3D bvn;
	bvn.x = bnv * phitnormal->x;						//project the normal velocity along normal
	bvn.y = bnv * phitnormal->y;
	bvn.z = bnv * phitnormal->z;

	Vertex3D bvt;
	bvt.x = vx - bvn.x;									// calc the tangent velocity
	bvt.y = vy - bvn.y;
	bvt.z = vz - bvn.z;

	Vertex3D bvT;										// ball tangent velocity Unit Tangent
	bvT.Set(bvt.x, bvt.y, bvt.z);						//copy ball tangent velocity
	bvT.Normalize();	

	Vertex3D bstv;										// ball surface tangential velocity

	CrossProduct(&m_angularvelocity, &bccpd, &bstv);	// velocity of ball surface at contact point

	Vertex3D cpvt;						// contact point velocity tangential to hit face

	float dot = bstv.Dot(&bvT);			// speed ball surface contact point tangential to contact surface point
	cpvt.x = bvT.x * dot;				//contact point velocity tangent to hit face
	cpvt.y = bvT.y * dot;
	cpvt.z = bvT.z * dot;

	Vertex3D slideVel;	// contact point slide velocity with ball center velocity
	slideVel.x = bstv.x - cpvt.x;  // slide velocity
	slideVel.y = bstv.y - cpvt.y;
	slideVel.z = bstv.z - cpvt.z;

	m_angularmomentum.MultiplyScalar(0.99f);

	// If the point and the ball are travelling in opposite directions,
	// and the point's velocity is at least the magnitude of the balls,
	// then we have a natural rool
	
	Vertex3D cpctrv;

	cpctrv.x = -slideVel.x;	//contact point co-tangential reverse velocity
	cpctrv.y = -slideVel.y;
	cpctrv.z = -slideVel.z;

	// Calculate the maximum amount the point velocity can change this
	// time segment due to friction
	Vertex3D FrictionForce;

	FrictionForce.x = cpvt.x - (-bvt.x);
	FrictionForce.y = cpvt.y - (-bvt.y);
	FrictionForce.z = cpvt.z - (-bvt.z);	

#define ANGULARFORCE 1   
					// Number I pulled out of my butt - this number indicates the maximum angular change 
					//per time unit, or the speed at which angluar momentum changes over time, 
					//depending on how you interpret it.

	// If the point can change fast enough to go directly to a natural roll, then do it.

	if (FrictionForce.LengthSquared() > ANGULARFORCE*ANGULARFORCE)
		{
		FrictionForce.Normalize();
		FrictionForce.MultiplyScalar(ANGULARFORCE);
		}

	if ((vx*vx + vy*vy + vz*vz) > (0.7*0.7))
		{
		cpctrv.x -= FrictionForce.x;
		cpctrv.y -= FrictionForce.y;
		cpctrv.z -= FrictionForce.z;
		}

	// Divide by the inertial tensor for a sphere in order to change
	// linear force into angular momentum
	cpctrv.x /= 2.5; // Inertial tensor for a sphere
	cpctrv.y /= 2.5;
	cpctrv.z /= 2.5;

	Vertex3D vResult;

	CrossProduct(&bccpd, &cpctrv, &vResult);//ball center contact point displacement X reverse contact point co-tan vel

	m_angularmomentum.Add(&vResult);	// add delta 

	m_inverseworldinertiatensor.MultiplyVector(&m_angularmomentum, &m_angularvelocity);
	}

void Ball::CalcHitRect()
	{
	}

void BallAnimObject::UpdateDisplacements(PINFLOAT dtime)
	{
	m_pball->UpdateDisplacements(dtime);
	}

void Ball::UpdateDisplacements(PINFLOAT dtime)
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
		x += (dsx = vx * dtime);
		y += (dsy = vy * dtime);		
		z += (dsz = vz * dtime);

		drsq = dsx*dsx + dsy*dsy + dsz*dsz;				// used to determine if static ball

		if (vz < 0 && z <= z_min)						//rolling point below the table and velocity driving deeper
			{
			z = z_min;									// set rolling point to table surface
			vz *= (PINFLOAT)-0.2;						// reflect velocity  ...  dull bounce

			vx *= c_hardFriction; vy *= c_hardFriction;  //friction other axiz
			
			Vertex3D vnormal;
			vnormal.Set(0,0,1);
			AngularAcceleration(&vnormal);
			}
		else if (vz > 0 && z >= z_max)						//top glass ...contact and going higher
			{
			z = z_max;									// set diametric rolling point to top glass
			vz *= (PINFLOAT)-0.2;						// reflect velocity  ...  dull bounce
			}

		if (vx < 0 && x <= x_min)						//left wall
			{
			x = x_min;									
			vx *= (PINFLOAT)-0.2;					
			}
		else if (vx > 0 && x >= x_max)				//right wall
			{
			x = x_max;							
			vx *= (PINFLOAT)-0.2;		
			}

		if (vy < 0 && y <= y_min)					//top wall
			{
			y = y_min;									
			vy *= (PINFLOAT)-0.2;					
			}
		else if (vy > 0 && y >= y_max)				//bottom wall
			{
			y = y_max;							
			vy *= (PINFLOAT)-0.2;		
			}
	
		CalcBoundingRect();
		
		Matrix3 mat3;
		Matrix3 addedorientation;
		mat3.CreateSkewSymmetric(&m_angularvelocity);
		
		addedorientation.MultiplyMatrix(&mat3, &m_orientation);

		addedorientation.MultiplyScalar((float)dtime);

		m_orientation.AddMatrix(&addedorientation, &m_orientation);

		m_orientation.OrthoNormalize();

		Matrix3 matTransposeOrientation;
		m_orientation.Transpose(&matTransposeOrientation);
		m_inverseworldinertiatensor.MultiplyMatrix(&m_orientation,&m_inversebodyinertiatensor);
		m_inverseworldinertiatensor.MultiplyMatrix(&m_inverseworldinertiatensor,&matTransposeOrientation);

        m_inverseworldinertiatensor.MultiplyVector(&m_angularmomentum, &m_angularvelocity);
		}
	}

void BallAnimObject::UpdateVelocities(PINFLOAT dtime)
{
	m_pball->UpdateVelocities(dtime);
}


void Ball::UpdateVelocities(PINFLOAT dtime)
{

	///  dtime is always 1.0	

	float g = g_pplayer->m_mainlevel.m_gravity.z * dtime;
	float nx = g_pplayer->m_NudgeX; // * dtime;
	float ny = g_pplayer->m_NudgeY; // * dtime;
	float mag;

	if (!fFrozen)  // Gravity	
	{
		vx += g_pplayer->m_mainlevel.m_gravity.x * dtime;	
		vy += g_pplayer->m_mainlevel.m_gravity.y * dtime;	

		if (z > z_min + 0.05f || g > 0)			// off the deck??? or gravity postive Z direction	
		{
			vz +=  g;
		}
		else 
		{
			vz += g *0.001f;					// don't add so much energy if already on the world floor
		}

		vx += nx;
		vy += ny;
	}

	mag = vx*vx + vy*vy + vz*vz;				//speed check 
	float antifrict = (mag > c_maxBallSpeedSqed) ? c_dampingFriction : 0.99875f;
	
	vx *= antifrict;							// speed damping
	vy *= antifrict; 
	vz *= antifrict; 
			
	m_fDynamic = C_DYNAMIC;						// always set .. after adding velocity

	CalcBoundingRect();
}
