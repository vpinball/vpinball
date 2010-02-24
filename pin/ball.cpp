#include "stdafx.h"
#include "..\Main.h"

Ball::Ball()
	{
	phoHitLast = NULL;
	
	if (!fOldPhys)
		{
		m_vpVolObjs = NULL; // should be NULL ... only real balls have this value
		m_Event_Pos.x = m_Event_Pos.y = m_Event_Pos.z = -1;
		}
	}
	
Ball::~Ball()
	{
		if (!fOldPhys && m_vpVolObjs)
		{
			delete m_vpVolObjs;
		}
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

	//m_angularmomentum.m_d[0] = 4;
	//m_angularvelocity.m_d[1] = 0.02f;

	fFrozen = fFalse;
	fTempFrozen = fFalse;

	m_fErase = fFalse;

	//m_pho = NULL;
	m_fCalced = fFalse;

	m_pballex = NULL;
	
	if (!fOldPhys)
		{
		m_vpVolObjs = new VectorVoid();
		}
	m_color = RGB(255,255,255);

	// m_szImage[0] = '\0';
	
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

/*Ball::~Ball()
	{
	if (m_pballex)
		{
		m_pballex->Release();
		m_pballex->m_pball = NULL;
		}
	}*/

/*Ball::Ball(Ball &pball)
	{

	}*/

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
	PINFLOAT dx = fabs(vx);
	PINFLOAT dy = fabs(vy);

	brc.left = (float)(x - radius - dx - 0.1);
	brc.top = (float)(y - radius - dy - 0.1);
	brc.right = (float)(x + radius + dx + 0.1);
	brc.bottom = (float)(y + radius + dy + 0.1);

	m_rcHitRect.left = brc.left;
	m_rcHitRect.top = brc.top;
	m_rcHitRect.right = brc.right;
	m_rcHitRect.bottom = brc.bottom;

	/*if (m_plevel)
		{
		PINFLOAT znew = m_plevel->m*(x+vx) + m_plevel->n*(y+vy) + m_plevel->b + radius;
		
		m_rcHitRect.zlow = (float)min(z, znew);
		m_rcHitRect.zhigh = (float)max(z, znew);
		}
	else*/
		{
		m_rcHitRect.zlow = (float)min(z, z+vz);
		m_rcHitRect.zhigh = (float)max(z, z+vz);
		}

	m_rcHitRect.zhigh += (float)(radius + 0.1);
	m_rcHitRect.zlow -= (float)(radius + 0.f);

	//m_rcHitRect.zhigh += radius*2 + 0.1f;
	//m_rcHitRect.zlow -= 0.1f;

	//m_rcHitRect.zlow = -1000;
	//m_rcHitRect.zhigh = 1000;

	//m_rcHitRect.zlow = z;
	//m_rcHitRect.zhigh = z+radius*2;
	}

void Ball::CollideWall(Vertex3D *phitnormal, float elasticity, float antifriction, float scatter_angle)
	{
	if (fOldPhys)
		{
		PINFLOAT dot = vx * phitnormal->x + vy * phitnormal->y;
		
		VertexD ThruVec;
		VertexD CrossVec;

		CrossVec.x = dot * phitnormal->x;
		CrossVec.y = dot * phitnormal->y;

		ThruVec.x = vx - CrossVec.x;
		ThruVec.y = vy - CrossVec.y;

		//ThruVec.x *= 0.99f; // Rolling friction is low
		//ThruVec.y *= 0.99f;

		ThruVec.x *= 0.999f; // Rolling friction is low
		ThruVec.y *= 0.999f;

		CrossVec.x *= -elasticity; // Friction in general is higher (negative due to bounce)
		CrossVec.y *= -elasticity;

		vx = ThruVec.x + CrossVec.x;
		vy = ThruVec.y + CrossVec.y;
		}
	else // new phys
		{
		PINFLOAT dot = vx * phitnormal->x + vy * phitnormal->y + vz * phitnormal->z; //speed normal to wall

		if (dot >= -C_LOWNORMVEL )								// nearly receding ... make sure of conditions
			{													// otherwise if clearly approaching .. process the collision
			if (dot > C_LOWNORMVEL) return;						//is this velocity clearly receding (i.e must > a minimum)	

	#ifdef C_EMBEDDED
			if (m_HitDist < -C_EMBEDDED)
				dot = -C_EMBEDSHOT;		// has ball become embedded???, give it a kick
				
			/*else if (m_hittime > 0) // only go into this special 'nearly receding' state if we're going to get out of it with some movement 
				{
				return;
				}*/
				
			return;
	#endif
			}
			
	#ifdef C_DISP_GAIN 		
		// correct displacements, mostly from low velocity, alternative to acceleration processing
		float hdist = -C_DISP_GAIN * (float)m_HitDist;			// limit delta noise crossing ramps, 
		if (hdist > 1.0e-4f)					// when hit detection checked it what was the displacement
			{			
			if (hdist > C_DISP_LIMIT) 
				{hdist = C_DISP_LIMIT;}	// crossing ramps, delta noise			
			x += hdist * phitnormal->x;	// push along norm, back to free area
			y += hdist * phitnormal->y;	// use the norm, but his is not correct
			z += hdist * phitnormal->z;	// 
			}	
	#endif					

		if (antifriction >= 1.0f || antifriction <= 0) 
			{antifriction = (float)c_hardFriction;} // use global

		dot *= -(1.005f + elasticity);	
		vx += dot * phitnormal->x; vx *= antifriction;
		vy += dot * phitnormal->y; vy *= antifriction;
		vz += dot * phitnormal->z; vz *= antifriction;

		
		if (scatter_angle <= 0) scatter_angle = (float)c_hardScatter;						// if <= zero use global value
		//scatter_angle *= g_pplayer->m_ptable->m_globalDifficulty;	// apply dificulty weighting
		
		if (dot > 1.0f && scatter_angle > 1.0e-5) //no scatter at low velocity 
			{
			float scatter = 2.0f* ((float)rand()/((float)RAND_MAX) - 0.5f);  // -1.0f..1.0f
			scatter *=  (1.0f - scatter*scatter)*2.59808f  * scatter_angle;	// shape quadratic distribution and scale
			float radsin = sin(scatter);//  Green's transform matrix... rotate angle delta 
			float radcos = cos(scatter);//  rotational transform from current position to position at time t
			PINFLOAT vxt = vx; 
			PINFLOAT vyt = vy;

			vx = vxt *radcos - vyt *radsin;  // rotate to random scatter angle
			vy = vyt *radcos + vxt *radsin;  //
			}

		}
	
	Vertex3D vnormal;
	vnormal.Set(phitnormal->x, phitnormal->y, phitnormal->z);
	
	AngularAcceleration(&vnormal);
	}

void Ball::Collide3DWall(Vertex3D *phitnormal, float m_elasticity, float antifriction, float scatter_angle)
	{
	PINFLOAT dot = vx * phitnormal->x + vy * phitnormal->y + vz * phitnormal->z;
	
	if (fOldPhys)
		{		
		Vertex3D ThruVec;
		Vertex3D CrossVec;

		CrossVec.x = (float)(dot * phitnormal->x);
		CrossVec.y = (float)(dot * phitnormal->y);
		CrossVec.z = (float)(dot * phitnormal->z);

		ThruVec.x = (float)(vx - CrossVec.x);
		ThruVec.y = (float)(vy - CrossVec.y);
		ThruVec.z = (float)(vz - CrossVec.z);

		ThruVec.x *= 0.999f; // Rolling friction is low
		ThruVec.y *= 0.999f;
		ThruVec.z *= 0.999f;

		CrossVec.x *= -m_elasticity;//0.2f;//-0.3f; // Friction in general is higher (negative due to bounce)
		CrossVec.y *= -m_elasticity;//0.2f;//-0.3f;
		CrossVec.z *= -m_elasticity;//0.2f;//-0.3f;

		vx = ThruVec.x + CrossVec.x;
		vy = ThruVec.y + CrossVec.y;
		vz = ThruVec.z + CrossVec.z;
		}
	else
		{
		PINFLOAT dot = vx * phitnormal->x + vy * phitnormal->y + vz * phitnormal->z; //speed normal to wall

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
		float hdist = -C_DISP_GAIN * (float)m_HitDist;			// limit delta noise crossing ramps, 
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
			{antifriction = (float)c_hardFriction;} // use global

		dot *= -(1.005f + m_elasticity);	
		vx += dot * phitnormal->x; vx *= antifriction;
		vy += dot * phitnormal->y; vy *= antifriction;
		vz += dot * phitnormal->z; vz *= antifriction;

		
		if (scatter_angle <= 0) scatter_angle = (float)c_hardScatter;						// if <= zero use global value
		scatter_angle *= 1;//g_pplayer->m_ptable->m_globalDifficulty;	// apply dificulty weighting
		
		if (dot > 1.0f && scatter_angle > 1.0e-5) //no scatter at low velocity 
			{
			float scatter = 2.0f* ((float)rand()/((float)RAND_MAX) - 0.5f);  // -1.0f..1.0f
			scatter *=  (1.0f - scatter*scatter)*2.59808f  * scatter_angle;	// shape quadratic distribution and scale
			float radsin = (float)sin(scatter);//  Green's transform matrix... rotate angle delta 
			float radcos = (float)cos(scatter);//  rotational transform from current position to position at time t
			float vxt = (float)vx; 
			float vyt = (float)vy;

			vx = vxt *radcos - vyt *radsin;  // rotate to random scatter angle
			vy = vyt *radcos + vxt *radsin;  //
			}

		}

	AngularAcceleration(phitnormal);
	}

PINFLOAT Ball::HitTest(Ball *pball, PINFLOAT dtime, Vertex3D *phitnormal)
	{
	if (fOldPhys)
		{
		PINFLOAT result;
		PINFLOAT dx,dy,dz;
		PINFLOAT totalradius = pball->radius + radius;
		PINFLOAT hittime;

		PINFLOAT a,b,c;

		PINFLOAT x1 = x;
		PINFLOAT y1 = y;
		PINFLOAT z1 = z;
		PINFLOAT x2 = pball->x;
		PINFLOAT y2 = pball->y;
		PINFLOAT z2 = pball->z;

		dx = vx-pball->vx;
		dy = vy-pball->vy;
		dz = vz-pball->vz;

		a = dx*dx+dy*dy+dz*dz;
		b = dx*(x1-x2) + dy*(y1-y2) + dz*(z1-z2);
		c = (x1-x2)*(x1-x2) + (y1-y2)*(y1-y2) + (z1-z2)*(z1-z2) - totalradius * totalradius;

		b=b*2;

		result = (b*b) - (4*a*c);

		if (result >= 0)
			{
			PINFLOAT time1, time2;

			time1 = (-b + (PINFLOAT)(sqrt(result))) / (2*a);
			time2 = (-b - (PINFLOAT)(sqrt(result))) / (2*a);

			hittime = min(time1,time2);

			if (hittime < 0 || hittime > dtime)
				{
				return -1;
				}

			PINFLOAT hitx, hity, hitz;
			hitx = x1+dx*hittime;
			hity = y1+dy*hittime;
			hitz = z1+dz*hittime;

			((Vertex3D *)phitnormal)->x = (float)((x2 - hitx)/totalradius);
			((Vertex3D *)phitnormal)->y = (float)((y2 - hity)/totalradius);
			((Vertex3D *)phitnormal)->z = (float)((z2 - hitz)/totalradius);

			return (float)hittime;
			}
		}
	else
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

		if ( bnv >  C_LOWNORMVEL) return -1;		// dot of delta velocity and delta displacement, postive if receding no collison

		PINFLOAT totalradius = pball->radius + radius;
		PINFLOAT bnd = bcdd - totalradius;

		if (bnd < PHYS_TOUCH)								// in contact??? 
			{
			if (bnd <= -PHYS_SKIN *2) 	
				{return -1;}				// embedded too deep

			if (fabsf((float)bnv) > C_CONTACTVEL)		// >fast velocity, return zero time
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

		((Vertex3D *)phitnormal)->x = (float)((hitx - x)/len);	//calc unit normal of collision
		((Vertex3D *)phitnormal)->y = (float)((hity - y)/len);
		((Vertex3D *)phitnormal)->z = (float)((hitz - z)/len);

		m_HitDist = bnd;								//actual contact distance 
		m_HitNormVel = bnv;
		m_HitRigid = true;								//rigid collision type

		return hittime;	
		}
		

	return -1;
	}	

void Ball::Collide(Ball *pball, Vertex3D *phitnormal)
	{
	if (fOldPhys)
		{
		float e = 0.8f;

		Vertex3D vel;
		Vertex3D vnormal = *(Vertex3D *)phitnormal;

		vel.x = (float)pball->vx;
		vel.y = (float)pball->vy;
		vel.z = (float)pball->vz;
		vel.x -= (float)vx;
		vel.y -= (float)vy;
		vel.z -= (float)vz;

		float dot = vel.x * vnormal.x + vel.y * vnormal.y + vel.z * vnormal.z;

		float ImpulseNumerator = -(1+e) * dot;
		float ImpulseDenominator = (1/1 + 1/1); // Set both masses to 1, since they are equal balls

		float impulse = ImpulseNumerator / ImpulseDenominator;

		if (!fFrozen)
			{
			pball->vx += impulse/1 * vnormal.x;
			pball->vy += impulse/1 * vnormal.y;
			pball->vz += impulse/1 * vnormal.z;
			
			vx -= impulse/1 * vnormal.x;
			vy -= impulse/1 * vnormal.y;
			vz -= impulse/1 * vnormal.z;
			}
		else
			{
			// HACK!!!! - real way is to set pball mass to infinite, and use (1-(impulse/1)) instead of (impulse*2/1)
			pball->vx += impulse*2/1 * vnormal.x;
			pball->vy += impulse*2/1 * vnormal.y;
			pball->vz += impulse*2/1 * vnormal.z;
			}
		}
	else // new phys
		{
		if (pball->fFrozen) 
			{return;}
			
#ifdef GLOBALLOG
	fprintf(logfile, "Collide Wall %d\n", g_pplayer->m_timeCur);
#endif

		Vertex3D vel;
		Vertex3D vnormal = *(Vertex3D *)phitnormal;
		
		// correct displacements, mostly from low velocity, alternative to true acceleration processing

		vel.x = (float)(pball->vx -vx);  //target ball to object ball
		vel.y = (float)(pball->vy -vy);  //delta velocity
		vel.z = (float)(pball->vz -vz);
		
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
			float edist = -C_DISP_GAIN * (float)pball->m_HitDist; // 
			if (edist > 1.0e-4)
				{										
				if (edist > C_DISP_LIMIT) 
					{edist = C_DISP_LIMIT;}		// crossing ramps, delta noise
				if (!fFrozen) edist /= 2;		// if the hitten ball is not frozen
				pball->x += edist * vnormal.x;	// push along norm, back to free area
				pball->y += edist * vnormal.y;	// use the norm, but is not correct, but cheaply handled
				pball->z += edist * vnormal.z;	// 
				}

			edist = -C_DISP_GAIN * (float)m_HitDist;	// noisy value .... needs investigation
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
	}

void Ball::AngularAcceleration(Vertex3D *phitnormal)
	{
	if (fOldPhys)
		{
		Vertex3D vR; // vector of point of contact
		Vertex3D vFriction;
		Vertex3D vResult;
		Vertex3D vVelBall; // Velocity of the ball

		vR.Set(-radius * phitnormal->x, -radius * phitnormal->y, -radius * phitnormal->z);

		PINFLOAT balldot = vx * phitnormal->x + vy * phitnormal->y + vz * phitnormal->z;

		//PINFLOAT balldot = vx * 0 + vy * 0 + vz * 1;
		
		Vertex3D ThruVec;
		Vertex3D CrossVec;

		//CrossVec.x = (float)(balldot * 0);
		//CrossVec.y = (float)(balldot * 0);
		//CrossVec.z = (float)(balldot * 1);

		CrossVec.x = (float)(balldot * phitnormal->x);
		CrossVec.y = (float)(balldot * phitnormal->y);
		CrossVec.z = (float)(balldot * phitnormal->z);

		ThruVec.x = (float)(vx - CrossVec.x);
		ThruVec.y = (float)(vy - CrossVec.y);
		ThruVec.z = (float)(vz - CrossVec.z);

		vVelBall.Set(ThruVec.x, ThruVec.y, ThruVec.z);

		float velballsquared = vVelBall.LengthSquared();

		Vertex3D vVelPoint; // Velocity of the contact point

		CrossProduct(&m_angularvelocity, &vR, &vVelPoint);

		Vertex3D vVelPointThrough; // Velocity of the roll which is along the line of the ball velocity
		Vertex3D vVelPointCross; // Velocity of the roll going across the line of the ball's velocity

		vVelBall.Normalize();
		float dot = vVelPoint.Dot(&vVelBall);

		vVelPointThrough.x = vVelBall.x * dot;
		vVelPointThrough.y = vVelBall.y * dot;
		vVelPointThrough.z = vVelBall.z * dot;

		vVelPointCross.x = vVelPoint.x - vVelPointThrough.x;
		vVelPointCross.y = vVelPoint.y - vVelPointThrough.y;
		vVelPointCross.z = vVelPoint.z - vVelPointThrough.z;

		float velpointsquared = vVelPointThrough.LengthSquared();

		float signball = vVelBall.x * vVelBall.y * vVelBall.z;
		float signpoint = vVelPointThrough.x * vVelPointThrough.y * vVelPointThrough.z;

		m_angularmomentum.MultiplyScalar(0.99f);

		// If the point and the ball are travelling in opposite directions,
		// and the point's velocity is at least the magnitude of the balls,
		// then we have a natural rool
		/*if (velballsquared > velpointsquared && (signball*signpoint <= 0))
			{
			float coef = 0.08f;
			float friction = (float)(coef * 1 * GRAVITY);
			//float inertialtensor = 5.0f/(2.0f * 1 * (radius*radius));

			vFriction.Set((float)-vx, (float)-vy, (float)-vz);
			vFriction.Normalize();
			vFriction.MultiplyScalar(friction);

			CrossProduct(&vR, &vFriction, &vResult);

			m_angularmomentum.Add(&vResult);

			m_inverseworldinertiatensor.MultiplyVector(&m_angularmomentum, &m_angularvelocity);
			
			vFriction.Set(-vVelPointCross.x, -vVelPointCross.y, -vVelPointCross.z);
			}
		else
			{
			vFriction.x = -vVelPointCross.x + (-vVelPointThrough.x - ThruVec.x);
			vFriction.y = -vVelPointCross.y + (-vVelPointThrough.y - ThruVec.y);
			vFriction.z = -vVelPointCross.z + (-vVelPointThrough.z - ThruVec.z);
			}*/

			//CrossProduct(&m_angularvelocity, &vR, &vVelPoint);

			vFriction.x = -vVelPointCross.x;// + (-vVelPointThrough.x - ThruVec.x);
			vFriction.y = -vVelPointCross.y;// + (-vVelPointThrough.y - ThruVec.y);
			vFriction.z = -vVelPointCross.z;// + (-vVelPointThrough.z - ThruVec.z);

			// Get velocity of the point contacting the surface
			Vertex3D PointDelta;
			PointDelta.x = vVelPointThrough.x;
			PointDelta.y = vVelPointThrough.y;
			PointDelta.z = vVelPointThrough.z;

			// Get what the point's velocity would be in a natural roll
			Vertex3D NaturalRoll;
			NaturalRoll.x = -ThruVec.x;
			NaturalRoll.y = -ThruVec.y;
			NaturalRoll.z = -ThruVec.z;

			// Calculate the maximum amount the point velocity can change this
			// time segment due to friction
			Vertex3D FrictionForce;
			FrictionForce.x = PointDelta.x - NaturalRoll.x;
			FrictionForce.y = PointDelta.y - NaturalRoll.y;
			FrictionForce.z = PointDelta.z - NaturalRoll.z;
			float naturaldist = FrictionForce.LengthSquared();

	#define ANGULARFORCE 1   // Number I pulled out of my butt - this number indicates the maximum angular change per time unit, or the speed at which angluar momentum changes over time, depending on how you interpret it.

			// If the point can change fast enough to go directly to a natural roll, then do it.
			if (naturaldist > ANGULARFORCE*ANGULARFORCE)
				{
				FrictionForce.Normalize();
				FrictionForce.MultiplyScalar(ANGULARFORCE);
				}

			if ((vx*vx + vy*vy + vz*vz) > (0.7*0.7))
				{
				vFriction.x -= FrictionForce.x;
				vFriction.y -= FrictionForce.y;
				vFriction.z -= FrictionForce.z;
				}
			else
				{
				// TOTAL HACK!!!!
				// If the ball is moving less than a certain speed, do not add to
				// natural roll - let it die off through friction.
				// This fixes the ball rolling slowly even when resting problem

				/*vFriction.x -= PointDelta.x;
				vFriction.y -= PointDelta.y;
				vFriction.z -= PointDelta.z;*/
				}

			/*vFriction.x = -vVelPointCross.x + (-vVelPointThrough.x - ThruVec.x);
			vFriction.y = -vVelPointCross.y + (-vVelPointThrough.y - ThruVec.y);
			vFriction.z = -vVelPointCross.z + (-vVelPointThrough.z - ThruVec.z);*/

			// Divide by the inertial tensor for a sphere in order to change
			// linear force into angular momentum
			vFriction.x /= 2.5; // Inertial tensor for a sphere
			vFriction.y /= 2.5;
			vFriction.z /= 2.5;

			/*vVelPointThrough.x = vVelBall.x;
			vVelPointThrough.y = vVelBall.y;
			vVelPointThrough.z = vVelBall.z;

			vVelPointCross.x = 0;
			vVelPointCross.y = 0;
			vVelPointCross.z = 0;*/

			/*vVelPoint.x = vVelPointThrough.x + vVelPointCross.x;

			vVelPoint.y = vVelPointThrough.y + vVelPointCross.y;
			vVelPoint.z = vVelPointThrough.z + vVelPointCross.z;*/

			//vFriction.Set(-vVelPointCross.x, -vVelPointCross.y, -vVelPointCross.z);

			CrossProduct(&vR, &vFriction, &vResult);

			m_angularmomentum.Add(&vResult);

			m_inverseworldinertiatensor.MultiplyVector(&m_angularmomentum, &m_angularvelocity);

		//vResult.MultiplyScalar(inertialtensor);
		}
	else
		{
		Vertex3D bccpd; // vector ball center to contact point displacement
		bccpd.Set(-radius * phitnormal->x, -radius * phitnormal->y, -radius * phitnormal->z); //from ball center to contact point

		PINFLOAT bnv = vx * phitnormal->x + vy * phitnormal->y + vz * phitnormal->z; //ball normal velocity to hit face

		Vertex3D bvn;
		bvn.x = (float)(bnv * phitnormal->x);						//project the normal velocity along normal
		bvn.y = (float)(bnv * phitnormal->y);
		bvn.z = (float)(bnv * phitnormal->z);

		Vertex3D bvt;
		bvt.x = (float)(vx - bvn.x);									// calc the tangent velocity
		bvt.y = (float)(vy - bvn.y);
		bvt.z = (float)(vz - bvn.z);

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
	}

void Ball::CalcHitRect()
	{
	}

void BallAnimObject::UpdateTimeTemp(PINFLOAT dtime)
	{
	m_pball->UpdateTimeTemp(dtime);
	}

void Ball::UpdateTimeTemp(PINFLOAT dtime)
	{
	if (!fFrozen)
		{
		// hittime is in 100th's of a second, so vy=vy*100
		x += vx * dtime;
		y += vy * dtime;
			{
			
			z += vz * dtime;
			
			
			if ((z-radius) <= 0 && (vz < 0))
				{
				
				vz *= -0.2;
				vx *= 0.999;
				vy *= 0.999;

				z = radius;

				Vertex3D vnormal;
				vnormal.Set(0,0,1);

				AngularAcceleration(&vnormal);
				}
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
		
		if (dtime < 0.000001 && fabs(vx) < 0.00000001 && fabs(vy) < 0.000000001)
		//if (dtime < 0.001 && fabs(vx) < 0.001 && fabs(vy) < 0.001)
			{
			// HACK - if the ball is hardly moving and hits two walls at roughly the same time, then we know that it's probably stuck in a cavern and we should just not hit-test it anymore
			fTempFrozen = fTrue;
			}
		}
	}

void BallAnimObject::UpdateAcceleration(PINFLOAT dtime)
	{
	m_pball->UpdateAcceleration(dtime);
	}

void Ball::UpdateAcceleration(PINFLOAT dtime)
	{
	fTempFrozen = fFalse;
	///  dtime is always 1.0	
	float g = (float)(-g_pplayer->m_gravityz/*g_pplayer->m_mainlevel.m_gravity.z*/ * dtime);
	float nx = g_pplayer->m_NudgeX; // * dtime;
	float ny = g_pplayer->m_NudgeY; // * dtime;
	float mag;

	if( g_pplayer && g_pplayer->m_NudgeManual >= 0) //joystick control of ball roll
	{
		vx *=  0.92f;//*dtime;	//rolling losses high for easy manual control
		vy *=  0.92f;//*dtime;
		vz *=  0.92f;//*dtime;	

#define JOY_DEADBAND  5.0e-2f

		mag = nx*nx + ny*ny;// + nz*nz;
		if (mag > (JOY_DEADBAND * JOY_DEADBAND))			//joystick dead band, allows hold and very slow motion
		{
			mag = sqrt(mag);
			nx -= nx/mag * JOY_DEADBAND;	// remove deadband offsets
			ny -= ny/mag * JOY_DEADBAND; 
			//nz -= nz/mag*JOY_DEADBAND;

			vx +=  nx;// *dtime;
			vy +=  ny;// *dtime;
			vz +=  g;// *dtime;//-c_Gravity;
		}
	}//manual joystick control
	else if (!fFrozen)  // Gravity	
	{
		vx += g_pplayer->m_mainlevel.m_gravity.x * dtime;	
		vy += g_pplayer->m_mainlevel.m_gravity.y * dtime;	

		vz +=  g;	

		vx += nx * dtime;
		vy += ny * dtime;
	}

	mag = (float)(vx*vx + vy*vy + vz*vz);				//speed check 
	float antifrict = (float)((mag > c_maxBallSpeedSqed) ? c_dampingFriction : 0.99875f);
	
	vx *= antifrict;							// speed damping
	vy *= antifrict; 
	vz *= antifrict; 
			
	m_fDynamic = C_DYNAMIC;						// always set .. after adding velocity

	CalcBoundingRect();
}
