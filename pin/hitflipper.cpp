#include "stdafx.h"

HitFlipper::HitFlipper(const float x, const float y, float baser, float endr, float flipr, const float angle,
					   const float zlow, const float zhigh, float strength, const float mass)
	{	
	m_flipperanim.m_height = zhigh - zlow;

	m_flipperanim.m_hitcircleBase.m_pfe = NULL;
	m_flipperanim.m_hitcircleEnd.m_pfe = NULL;

	m_flipperanim.m_lineseg1.m_pfe = NULL;
	m_flipperanim.m_lineseg2.m_pfe = NULL;

	m_flipperanim.m_lineseg1.m_rcHitRect.zlow = zlow;
	m_flipperanim.m_lineseg1.m_rcHitRect.zhigh = zhigh;

	m_flipperanim.m_lineseg2.m_rcHitRect.zlow = zlow;
	m_flipperanim.m_lineseg2.m_rcHitRect.zhigh = zhigh;

	m_flipperanim.m_hitcircleEnd.zlow = zlow;
	m_flipperanim.m_hitcircleEnd.zhigh = zhigh;
	
	m_flipperanim.m_hitcircleBase.zlow = zlow;
	m_flipperanim.m_hitcircleBase.zhigh = zhigh;

	m_flipperanim.m_hitcircleBase.center.x = x;
	m_flipperanim.m_hitcircleBase.center.y = y;

	if (baser < 0.01f) baser = 0.01f; // must not be zero 
	m_flipperanim.m_hitcircleBase.radius = baser; //radius of base section

	if (endr < 0.01f) endr = 0.01f; // must not be zero 
	m_flipperanim.m_endradius = endr;		// radius of flipper end

	if (flipr < 0.01f) flipr = 0.01f; // must not be zero 
	m_flipperanim.m_flipperradius = flipr;	//radius of flipper arc, center-to-center radius

	m_flipperanim.m_angleCur = angle;
	m_flipperanim.m_angleEnd = angle;
	m_flipperanim.m_angleFrame = angle;

	m_flipperanim.m_anglespeed = 0;
	m_flipperanim.m_lastAngspd = 0;
	
	m_flipperanim.SetObjects(angle);	

	m_flipperanim.m_fAcc = 0;
	m_flipperanim.m_mass = mass;
	
	m_flipperanim.m_iframe = -1;

	m_last_hittime = 0;

	m_flipperanim.m_force = 2;
	if (strength < 0.01f) strength = 0.01f;
	m_forcemass = strength;

	m_flipperanim.m_maxvelocity = m_flipperanim.m_force * 4.5f;

    m_flipperanim.m_lastHitFace = false; //rlc used to optimize hit face search order

	const float fa = asinf((baser-endr)/flipr); //face to centerline angle (center to center)

	m_flipperanim.faceNormOffset = (float)(M_PI/2.0) - fa; //angle of normal when flipper center line at angle zero

	const float len = m_flipperanim.m_flipperradius*cosf(fa); //Cosine of face angle X hypotenuse
	m_flipperanim.m_lineseg1.length = len;
	m_flipperanim.m_lineseg2.length = len;

	
	m_flipperanim.zeroAngNorm.x =  sinf(m_flipperanim.faceNormOffset);// F2 Norm, used in Green's transform, in FPM time search
	m_flipperanim.zeroAngNorm.y = -cosf(m_flipperanim.faceNormOffset);// F1 norm, change sign of x component, i.e -zeroAngNorm.x

#if 0  //rlc removed until next major physics revision
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

	m_flipperanim.m_inertia = Iff * mass;  //mass of flipper body
#else
	m_flipperanim.m_inertia = mass;  //stubbed to mass of flipper body
#endif
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


void FlipperAnimObject::SetObjects(const float angle)
	{	
	m_angleCur = angle;
	m_hitcircleEnd.center.x = m_hitcircleBase.center.x + m_flipperradius*sinf(angle); //place end radius center
	m_hitcircleEnd.center.y = m_hitcircleBase.center.y - m_flipperradius*cosf(angle);
	m_hitcircleEnd.radius = m_endradius;

	m_lineseg1.normal.x =  sinf(angle - faceNormOffset); // normals to new face positions
	m_lineseg1.normal.y = -cosf(angle - faceNormOffset);
	m_lineseg2.normal.x =  sinf(angle + faceNormOffset);
	m_lineseg2.normal.y = -cosf(angle + faceNormOffset);

	m_lineseg1.v1.x = m_hitcircleEnd.center.x + m_hitcircleEnd.radius*m_lineseg1.normal.x; //new endpoints
	m_lineseg1.v1.y = m_hitcircleEnd.center.y + m_hitcircleEnd.radius*m_lineseg1.normal.y;

	m_lineseg1.v2.x = m_hitcircleBase.center.x + m_hitcircleBase.radius*m_lineseg1.normal.x;
	m_lineseg1.v2.y = m_hitcircleBase.center.y + m_hitcircleBase.radius*m_lineseg1.normal.y;

	m_lineseg2.v1.x = m_hitcircleBase.center.x + m_hitcircleBase.radius*m_lineseg2.normal.x; // remember v1 to v2 direction
	m_lineseg2.v1.y = m_hitcircleBase.center.y + m_hitcircleBase.radius*m_lineseg2.normal.y; // to make sure norm is correct

	m_lineseg2.v2.x = m_hitcircleEnd.center.x + m_hitcircleEnd.radius*m_lineseg2.normal.x;
	m_lineseg2.v2.y = m_hitcircleEnd.center.y + m_hitcircleEnd.radius*m_lineseg2.normal.y;
	}


void FlipperAnimObject::UpdateDisplacements(const float dtime)
	{
	 m_angleCur += m_anglespeed*dtime;	// move flipper angle
	
	if (m_angleCur > m_angleMax)		// too far???
		{
		m_angleCur = m_angleMax; 

		if (m_anglespeed > 0) 
			{
			if(m_fAcc > 0) m_fAcc = 0;

			const float anglespd = fabsf(m_anglespeed*(float)(180.0/M_PI));
			m_anglespeed = 0; 
			
			if (m_EnableRotateEvent > 0) m_pflipper->FireVoidEventParm(DISPID_LimitEvents_EOS,anglespd); // send EOS event
			else if (m_EnableRotateEvent < 0) m_pflipper->FireVoidEventParm(DISPID_LimitEvents_BOS, anglespd);	// send Beginning of Stroke event
			m_EnableRotateEvent = 0;
			}
		}	
	else if (m_angleCur < m_angleMin)
		{
		m_angleCur = m_angleMin; 

		if (m_anglespeed < 0)
			{
			if(m_fAcc < 0) m_fAcc = 0;

			const float anglespd = fabsf(m_anglespeed*(float)(180.0/M_PI));
			m_anglespeed = 0;			

			if (m_EnableRotateEvent > 0) m_pflipper->FireVoidEventParm(DISPID_LimitEvents_EOS,anglespd); // send EOS event
			else if (m_EnableRotateEvent < 0) m_pflipper->FireVoidEventParm(DISPID_LimitEvents_BOS, anglespd);	// send Park event
			m_EnableRotateEvent = 0;
			}
		}
	}

void FlipperAnimObject::UpdateVelocities()
	{
	if (m_fAcc == 0) ;//m_anglespeed = 0; //idle
	else if (m_fAcc > 0) // postive ... increasing angle
		{			
		m_anglespeed += (m_force/m_mass) * C_FLIPPERACCEL; //new angular rate

		if (m_anglespeed > m_maxvelocity) 
			m_anglespeed = m_maxvelocity; //limit			
		}
	else // negative ... decreasing angle
		{		
		m_anglespeed -= (m_force/m_mass) * C_FLIPPERACCEL;	//new angular rate

		if (m_anglespeed < -m_maxvelocity) 
			m_anglespeed = -m_maxvelocity; //limit			
		}
	}

//rlc  new hitflipper ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define LeftFace 1
#define RightFace 0

float HitFlipper::HitTest(Ball * const pball, const float dtime, Vertex3Ds * const phitnormal)
	{
	if (!m_flipperanim.m_fEnabled) return -1;

	const bool lastface = m_flipperanim.m_lastHitFace;

	//m_flipperanim.SetObjects(m_flipperanim.m_angleCur);	// set current positions ... not needed

	// for effective computing, adding a last face hit value to speed calculations 
	//  a ball can only hit one face never two
	// also if a ball hits a face then it can not hit either radius
	// so only check these if a face is not hit
	// endRadius is more likely than baseRadius ... so check it first

	float hittime = HitTestFlipperFace(pball, dtime, phitnormal, lastface); // first face
	if (hittime >= 0) return hittime;		

	hittime = HitTestFlipperFace(pball, dtime, phitnormal, !lastface); //second face
	if (hittime >= 0)
		{
		m_flipperanim.m_lastHitFace = !lastface;	// change this face to check first
		return hittime;
		}

	hittime = HitTestFlipperEnd(pball, dtime, phitnormal); // end radius
	if (hittime >= 0)
		return hittime;

	hittime = m_flipperanim.m_hitcircleBase.HitTest(pball, dtime, phitnormal);
	if (hittime >= 0)
		{		
		phitnormal[1].x = 0;			//Tangent velocity of contact point (rotate Normal right)
		phitnormal[1].y = 0;			//units: rad*d/t (Radians*diameter/time

		phitnormal[2].x = 0;			//moment is zero ... only friction
		phitnormal[2].y = 0;			//radians/time at collison

		return hittime;
		}

	return -1.0f;	// no hits
	}

float HitFlipper::HitTestFlipperEnd(Ball * const pball, const float dtime, Vertex3Ds * const phitnormal) // replacement
	{ 	 
	const float angleCur = m_flipperanim.m_angleCur;
	float anglespeed = m_flipperanim.m_anglespeed;		// rotation rate
	
	const Vertex2D flipperbase = m_flipperanim.m_hitcircleBase.center;
			
	const float angleMin = m_flipperanim.m_angleMin;
	const float angleMax = m_flipperanim.m_angleMax;

	const float ballr = pball->radius;
	const float feRadius = m_flipperanim.m_hitcircleEnd.radius;

	const float ballrEndr = feRadius + ballr;			// magnititude of (ball - flipperEnd)

	const float ballx = pball->x;
	const float bally = pball->y;

	const float ballvx = pball->vx;
	const float ballvy = pball->vy;

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
		
		if (k >= 3)// MFP root search +++++++++++++++++++++++++++++++++++++++++
			{
			if (bfend*d0 <= 0.0f)										// zero crossing
				{ t1 = t; d1 = bfend; if (dp*bfend > 0.0) d0 *= 0.5f; } // 	move right interval limit			
			else 
				{ t0 = t; d0 = bfend; if (dp*bfend > 0.0) d1 *= 0.5f; }	// 	move left interval limit		
			}		
		else if (k == 2) // end pass two, check if zero crossing on initial interval, exit if none
			{
			if (dp*bfend > 0.0f) return -1.0f;	// no solution ... no obvious zero crossing

			t0 = 0; t1 = dtime; d0 = dp; d1 = bfend; // set initial boundaries
			}
		else // (k == 1) end of pass one ... set full interval pass, t = dtime
			{ // test for extreme conditions
			if (bfend < -((float)PHYS_SKIN + feRadius)) return -1.0f;	// too deeply embedded, ambigious position
			if (bfend <= (float)PHYS_TOUCH) 
				break; // inside the clearance limits
				
			t0 = t1 = dtime; d0 = 0; d1 = bfend; // set for second pass, force t=dtime
			}

		t = t0 - d0*(t1 - t0)/(d1 - d0);			// estimate next t
		dp = bfend;									// remember 

		}//for loop
//+++ End time interation loop found time t soultion ++++++

	if (t < 0 || t > dtime							// time is outside this frame ... no collision
		||
	   ((k > C_INTERATIONS) && (fabsf(bfend) > (float)(PHYS_SKIN/4.0)))) // last ditch effort to accept a solution
		return -1.0f; // no solution

	// here ball and flipper end are in contact .. well in most cases, near and embedded solutions need calculations	
		
	const float hitz = pball->z - ballr + pball->vz*t;	// check for a hole, relative to ball rolling point at hittime

	if ((hitz + (ballr * 1.5f)) < m_rcHitRect.zlow		//check limits of object's height and depth
		|| (hitz + (ballr * 0.5f)) > m_rcHitRect.zhigh)
		return -1.0f;
	
	// ok we have a confirmed contact, calc the stats, remember there are "near" solution, so all
	// parameters need to be calculated from the actual configuration, i.e contact radius must be calc'ed

	const float inv_cbcedist = 1.0f/cbcedist;
	phitnormal[0].x = ballvtx*inv_cbcedist;				// normal vector from flipper end to ball
	phitnormal[0].y = ballvty*inv_cbcedist;
	
	const Vertex2D dist(
		(pball->x + ballvx*t - ballr*phitnormal[0].x - m_flipperanim.m_hitcircleBase.center.x), // vector from base to flipperEnd plus the projected End radius
		(pball->y + ballvy*t - ballr*phitnormal[0].y - m_flipperanim.m_hitcircleBase.center.y));

	const float distance = sqrtf(dist.x*dist.x + dist.y*dist.y);	// distance from base center to contact point

	if ((contactAng >= angleMax && anglespeed > 0) || (contactAng <= angleMin && anglespeed < 0))	// hit limits ??? 
		anglespeed = 0;							// rotation stopped

	const float inv_distance = 1.0f/distance;
	phitnormal[1].x = -dist.y*inv_distance; //Unit Tangent vector velocity of contact point(rotate normal right)
	phitnormal[1].y =  dist.x*inv_distance;
	
	phitnormal[2].x = distance;				//moment arm diameter
	phitnormal[2].y = anglespeed;			//radians/time at collison
	
	//recheck using actual contact angle of velocity direction
	const Vertex2D dv(
		(ballvx - phitnormal[1].x *anglespeed*distance), 
		(ballvy - phitnormal[1].y *anglespeed*distance)); //delta velocity ball to face

	const float bnv = dv.x*phitnormal->x + dv.y*phitnormal->y;  //dot Normal to delta v

	if (bnv >= 0) 
		return -1.0f; // not hit ... ball is receding from face already, must have been embedded or shallow angled

	pball->m_HitDist = bfend;				//actual contact distance ..
	pball->m_HitNormVel = bnv;
	pball->m_HitRigid = true;				// collision type

	return t;
	}


float HitFlipper::HitTestFlipperFace(Ball * const pball, const float dtime, Vertex3Ds * const phitnormal, const bool face) // replacement
	{ 
	const float angleCur = m_flipperanim.m_angleCur;
	float anglespeed = m_flipperanim.m_anglespeed;				// rotation rate
	
	const Vertex2D flipperbase = m_flipperanim.m_hitcircleBase.center;
	const float feRadius = m_flipperanim.m_hitcircleEnd.radius;
	
	const float angleMin = m_flipperanim.m_angleMin;
	const float angleMax = m_flipperanim.m_angleMax;	

	const float ballr = pball->radius;	
	const float ballvx = pball->vx;
	const float ballvy = pball->vy;	
	//float ballrEndr = m_flipperanim.m_hitcircleEnd.radius + ballr;// magnititude of (ball - flipperEnd)
	
	// flipper positions at zero degrees rotation

	float ffnx = m_flipperanim.zeroAngNorm.x; // flipper face normal vector //Face2 
	if (face == LeftFace) ffnx = -ffnx;		  // negative for face1

	const float ffny = m_flipperanim.zeroAngNorm.y;	  // norm y component same for either face
	const Vertex2D vp(									  // face segment V1 point
		m_flipperanim.m_hitcircleBase.radius*ffnx, // face endpoint of line segment on base radius
		m_flipperanim.m_hitcircleBase.radius*ffny);		
	
	Vertex2D F;			// flipper face normal
	
	float bffnd;		// ball flipper face normal distance (negative for normal side)
	float ballvtx;		// new ball position at time t in flipper face coordinate
	float ballvty;
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
			
		ballvtx = pball->x + ballvx*t - vt.x;	// new ball position relative to rotated line segment endpoint
		ballvty = pball->y + ballvy*t - vt.y;	

		bffnd = ballvtx *F.x +  ballvty *F.y - ballr; // normal distance to segment 
		
		if (fabsf(bffnd) <= C_PRECISION) break;

	// loop control, boundary checks, next estimate, etc.
		
		if (k >= 3)// MFP root search +++++++++++++++++++++++++++++++++++++++++
			{
			if (bffnd*d0 <= 0.0)									// zero crossing
				{ t1 = t; d1 = bffnd; if (dp*bffnd > 0.0) d0 *= 0.5f; } // 	move right limits
			else 
				{ t0 = t; d0 = bffnd; if (dp*bffnd > 0.0) d1 *= 0.5f; } // move left limits
			}		
		else if (k == 2)// end pass two, check if zero crossing on initial interval, exit
			{	
			if (dp*bffnd > 0.0) return -1.0f;	// no solution ... no obvious zero crossing
			t0 = 0; t1 = dtime; d0 = dp; d1 = bffnd; // testing MFP estimates			
			}
		else // (k == 1) end of pass one ... set full interval pass, t = dtime
			{// test for already inside flipper plane, either embedded or beyond the face endpoints
			if (bffnd < -((float)PHYS_SKIN + feRadius)) return -1.0f;		// wrong side of face, or too deeply embedded			
			if (bffnd <= (float)PHYS_TOUCH) 
				break; // inside the clearance limits, go check face endpoints

			t0 = t1 = dtime; d0 = 0; d1 = bffnd; // set for second pass, so t=dtime
			}

		t = t0 - d0*(t1-t0)/(d1-d0);					// next estimate
		dp = bffnd;	// remember 
		}//for loop

	//+++ End time interation loop found time t soultion ++++++

	if (t < 0 || t > dtime								// time is outside this frame ... no collision
		||
	   ((k > C_INTERATIONS) && (fabsf(bffnd) > (float)(PHYS_SKIN/4.0)))) // last ditch effort to accept a near solution
		return -1.0f; // no solution

	// here ball and flipper face are in contact... past the endpoints, also, don't forget embedded and near soultion

	Vertex2D T;			// flipper face tangent
	if (face == LeftFace) 
		{ T.x = -F.y; T.y = F.x; }	// rotate to form Tangent vector				
	else
	    { T.x = F.y; T.y = -F.x; }	// rotate to form Tangent vector
		
	const float bfftd = ballvtx * T.x + ballvty * T.y;			// ball to flipper face tanget distance	

	const float len = m_flipperanim.m_lineseg1.length;// face segment length ... i.g same on either face									
	if (bfftd < -C_TOL_ENDPNTS || bfftd > len + C_TOL_ENDPNTS) return -1.0f;	// not in range of touching
	
	const float hitz = pball->z - ballr + pball->vz*t;	// check for a hole, relative to ball rolling point at hittime

	if ((hitz + (ballr * 1.5f)) < m_rcHitRect.zlow			//check limits of object's height and depth 
		|| (hitz + (ballr * 0.5f)) > m_rcHitRect.zhigh)
		return -1.0f;

	// ok we have a confirmed contact, calc the stats, remember there are "near" solution, so all
	// parameters need to be calculated from the actual configuration, i.e contact radius must be calc'ed

	phitnormal[0].x = F.x;	// hit normal is same as line segment normal
	phitnormal[0].y = F.y;

	const Vertex2D dist( // calculate moment from flipper base center
		(pball->x + ballvx*t - ballr*F.x - m_flipperanim.m_hitcircleBase.center.x),  //center of ball + projected radius to contact point
		(pball->y + ballvy*t - ballr*F.y - m_flipperanim.m_hitcircleBase.center.y)); // all at time t

	const float distance = sqrtf(dist.x*dist.x + dist.y*dist.y);	// distance from base center to contact point

	const float inv_dist = 1.0f/distance;
	phitnormal[1].x = -dist.y*inv_dist;		//Unit Tangent velocity of contact point(rotate Normal clockwise)
	phitnormal[1].y =  dist.x*inv_dist;
	
	if (contactAng >= angleMax && anglespeed > 0 || contactAng <= angleMin && anglespeed < 0)	// hit limits ??? 
		anglespeed = 0.0f;							// rotation stopped

	phitnormal[2].x = distance;				//moment arm diameter
	phitnormal[2].y = anglespeed;			//radians/time at collison
	
	const Vertex2D dv(
		(ballvx - phitnormal[1].x *anglespeed*distance), 
		(ballvy - phitnormal[1].y *anglespeed*distance)); //delta velocity ball to face

	const float bnv = dv.x*phitnormal->x + dv.y*phitnormal->y;  //dot Normal to delta v

	if (bnv >= C_LOWNORMVEL) 
		return -1.0f; // not hit ... ball is receding from endradius already, must have been embedded

	pball->m_HitDist = bffnd;				//normal ...actual contact distance ... 
	pball->m_HitNormVel = bnv;
	pball->m_HitRigid = true;				// collision type

	return t;
	}


void HitFlipper::Collide(Ball * const pball, Vertex3Ds * const phitnormal)
	{
	const float vx = pball->vx;
	const float vy = pball->vy;
	const float distance = phitnormal[2].x;				// moment .... and the flipper response
	const float angsp = m_flipperanim.m_anglespeed;		// angular rate of flipper at impact moment
	float tanspd = distance * angsp;					// distance * anglespeed
	float flipperHit = 0;

	Vertex2D dv(
		vx - phitnormal[1].x*tanspd, 
		vy - phitnormal[1].y*tanspd);					 //delta velocity ball to face

	float dot = dv.x*phitnormal->x + dv.y*phitnormal->y; //dot Normal to delta v

	if (dot >= -C_LOWNORMVEL )							 // nearly receding ... make sure of conditions
		{												 // otherwise if clearly approaching .. process the collision
		if (dot > C_LOWNORMVEL) return;					 //is this velocity clearly receding (i.e must > a minimum)		
#ifdef C_EMBEDDED
		if (pball->m_HitDist < -C_EMBEDDED)
			dot = -C_EMBEDSHOT;							 // has ball become embedded???, give it a kick
		else return;
#endif		
		}

#ifdef C_DISP_GAIN 
		// correct displacements, mostly from low velocity blindness, an alternative to true acceleration processing
		float hdist = -C_DISP_GAIN * pball->m_HitDist;				// distance found in hit detection
		if (hdist > 1.0e-4f)
			{
			if (hdist > C_DISP_LIMIT) 
				hdist = C_DISP_LIMIT;	// crossing ramps, delta noise
			pball->x += hdist * phitnormal->x;	// push along norm, back to free area
			pball->y += hdist * phitnormal->y;	// use the norm, but is not correct
			}
#endif		
		
	float impulse = 1.005f + m_elasticity;		// hit on static, immovable flipper ... i.e on the stops
	float obliquecorr = 0.0f;

	if ((dot < -0.25f) && (g_pplayer->m_timeCur - m_last_hittime) > 250) //limit rate to 333 milliseconds per event 
		{
			flipperHit = (distance == 0.0f) ? -1.0f : -dot; // move event processing to end of collision handler...
		}

	m_last_hittime = g_pplayer->m_timeCur; // keep resetting until idle for 250 milliseconds

	if (distance > 0)	// recoil possible 
		{			
		const float maxradius = m_pflipper->m_d.m_FlipperRadius + m_pflipper->m_d.m_EndRadius; 		
		const float recoil = m_pflipper->m_d.m_recoil/maxradius;				// convert to Radians/time
		const float tfdr = distance/maxradius; 		
		const float tfr = powf(tfdr,m_pflipper->m_d.m_powerlaw);				// apply powerlaw weighting
		const float dvt = dv.x * phitnormal[1].x + dv.y  * phitnormal[1].y;		// velocity transvere to flipper
		const float anglespeed = m_flipperanim.m_anglespeed + dvt * tfr * impulse/(distance*(m_forcemass + tfr));		

		if (m_flipperanim.m_fAcc != 0)											// currently in rotation
			{	
			obliquecorr = m_flipperanim.m_fAcc * m_pflipper->m_d.m_obliquecorrection;	//flipper trajectory correction
			impulse = (1.005f + m_elasticity)/(1.0f + (tfr/m_forcemass));		// impulse for pinball
			m_flipperanim.m_anglespeed = anglespeed;							// new angle speed for flipper	
			}
		else if (recoil > 0 && fabsf(anglespeed) > recoil)						// discard small static impact motions
			{ // these effects are for the flipper at EOS (End of Stroke)
			if (anglespeed < 0 && m_flipperanim.m_angleCur >= m_flipperanim.m_angleMax)	// at max angle now?
				{ // rotation toward minimum angle					
				m_flipperanim.m_force = max(fabsf(anglespeed+anglespeed),0.005f); // restoring force
				impulse = (1.005f + m_elasticity)/(1.0f + (tfr/m_forcemass));	// impulse for pinball
				m_flipperanim.m_anglespeed = anglespeed;						// angle speed, less linkage losses, etc.
				m_flipperanim.m_fAcc = 1;	//set acceleration to opposite direction
				if (fabsf(anglespeed) > 0.05f) 
					m_flipperanim.m_EnableRotateEvent = 1; //make EOS event
				}
			else if (anglespeed > 0 && m_flipperanim.m_angleCur <= m_flipperanim.m_angleMin)// at min angle now?
				{// rotation toward maximum angle
				m_flipperanim.m_force = max(fabsf(anglespeed * 2.0f),0.005f);	// restoreing force
				impulse = (1.005f + m_elasticity)/(1.0f + (tfr/m_forcemass));		// impulse for pinball
				m_flipperanim.m_anglespeed = anglespeed;					// angle speed, less linkage losses, etc.
				m_flipperanim.m_fAcc = -1;//set acceleration to opposite direction
				if (fabsf(anglespeed) > 0.05f) 
					m_flipperanim.m_EnableRotateEvent = 1; //make EOS event
				}
			}	
		}
	pball->vx -= impulse*dot * phitnormal->x; 						// new velocity for ball after impact
	pball->vy -= impulse*dot * phitnormal->y; 						// 
	

	float scatter_angle = m_pflipper->m_d.m_scatterangle;				//
	if (scatter_angle <= 0.0f) scatter_angle = c_hardScatter;				// object specific roughness
	scatter_angle *= g_pplayer->m_ptable->m_globalDifficulty;			// apply dificulty weighting

	if (dot > -1.0f) scatter_angle = 0;									// not for low velocities

	if (obliquecorr != 0 || scatter_angle > 1.0e-5f)					// trajectory correction to reduce the obliqueness 
		{
		float scatter = (float)rand()*(float)(2.0/RAND_MAX) - 1.0f;     // -1.0f..1.0f
		scatter *= (1.0f - scatter*scatter)* 2.59808f * scatter_angle;	// shape quadratic distribution and scale
		scatter_angle = obliquecorr + scatter;
		const float radsin = sinf(scatter_angle);	//  Green's transform matrix... rotate angle delta 
		const float radcos = cosf(scatter_angle);	//  rotational transform from current position to position at time t
		const float vx2 = pball->vx;
		const float vy2 = pball->vy;
		pball->vx = vx2 *radcos - vy2 *radsin;  // rotate trajectory more accurately
		pball->vy = vy2 *radcos + vx2 *radsin;
		}

	pball->vx *= 0.985f; pball->vy *= 0.985f; pball->vz *= 0.96f;	// friction

	pball->m_fDynamic = C_DYNAMIC;			// reactive ball if quenched

	tanspd = m_flipperanim.m_anglespeed *distance; // new tangential speed
	dv.x = (pball->vx - phitnormal[1].x * tanspd); // project along unit transverse vector
	dv.y = (pball->vy - phitnormal[1].y * tanspd); // delta velocity

	dot = dv.x*phitnormal->x + dv.y*phitnormal->y;	// dot face Normal to delta v

	if (dot < 0)
		{	// opps .... impulse calculations were off a bit, add a little boost
		dot *= -1.2f;						// small bounce
		pball->vx += dot * phitnormal->x;	// new velocity for ball after corrected impact
		pball->vy += dot * phitnormal->y;	//
		}

	// move hit event to end of collision routine, pinball may be deleted
	if (flipperHit != 0)
		{
		if (flipperHit < 0) m_pflipper->FireGroupEvent(DISPID_HitEvents_Hit);	   //simple hit event	
		else m_pflipper->FireVoidEventParm(DISPID_FlipperEvents_Collide,flipperHit); // collision velocity (normal to face)	
		}

	const Vertex3Ds vnormal(phitnormal->x, phitnormal->y, 0.0f);
	pball->AngularAcceleration(&vnormal);
	}

//rlc end change  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void FlipperAnimObject::Check3D()
	{
	const int frame = (m_fEnabled) ? (int)((m_angleCur-m_frameStart)/(m_frameEnd-m_frameStart) * (m_vddsFrame.Size()-1)) : -1;

	if (frame != m_iframe)
		{
		m_iframe = frame;

		m_fInvalid = fTrue;
		}
	}

ObjFrame *FlipperAnimObject::Draw3D(const RECT * const prc)
	{
	if (m_iframe == -1) return NULL;

	return (m_fEnabled) ? m_vddsFrame.ElementAt(m_iframe) : NULL;
	}
