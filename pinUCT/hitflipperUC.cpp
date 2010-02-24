#include "stdafx.h"
#include "..\Main.h"

#define C_FLIPPERACCEL  (PINFLOAT)1.25f
#define C_FLIPPERIMPULSE (PINFLOAT)1.0f

HitFlipperUC::HitFlipperUC(float x, float y, float baser, float endr, float flipr, float angle
					   , float zlow, float zhigh, float strength, float mass) : HitFlipper(x, y, baser, endr, flipr, angle, zlow, zhigh, strength, mass)
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

	if (baser < 0.01) baser = (PINFLOAT)0.01; // must not be zero 
	m_flipperanim.m_hitcircleBase.radius = baser; //radius of base section

	if (endr < 0.01) endr = (PINFLOAT)0.01; // must not be zero 
	m_flipperanim.m_endradius = endr;		// radius of flipper end

	if (flipr < 0.01) flipr = (PINFLOAT)0.01; // must not be zero 
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
	if (strength < 0.01)strength = (PINFLOAT)0.01;
	m_forcemass = strength;

	m_flipperanim.m_maxvelocity = m_flipperanim.m_force * 4.5f;

	m_flipperanim.m_lastHitFace = false; //rlc used to optimize hit face search order

	float fa = asin ((baser-endr)/flipr); //face to centerline angle (center to center)

	m_flipperanim.faceNormOffset = PI/2- fa; //angle of normal when flipper center line at angle zero

	float len = m_flipperanim.m_flipperradius*cos(fa); //Cosine of face angle X hypotenuse
	//m_flipperanim.m_lineseg1.length = len;
	//m_flipperanim.m_lineseg2.length = len;


	m_flipperanim.zeroAngNorm.x = sin(PI/2 - fa); // F2 Norm, used in Green's transform, in FPM time search
	m_flipperanim.zeroAngNorm.y = -cos(PI/2 - fa);// F1 norm, change sign of x component, i.e -zeroAngNorm.x

	m_flipperanim.m_inertia = mass;  //stubbed to mass of flipper body
}

HitFlipperUC::~HitFlipperUC()
{
	//m_pflipper->m_phitflipper = NULL;
}

void HitFlipperUC::CalcHitRect()
{
	// Allow roundoff
	m_rcHitRect.left = (float)(m_flipperanim.m_hitcircleBase.center.x - m_flipperanim.m_flipperradius - m_flipperanim.m_endradius - 0.1f);
	m_rcHitRect.right = (float)(m_flipperanim.m_hitcircleBase.center.x + m_flipperanim.m_flipperradius + m_flipperanim.m_endradius + 0.1f);
	m_rcHitRect.top = (float)(m_flipperanim.m_hitcircleBase.center.y - m_flipperanim.m_flipperradius - m_flipperanim.m_endradius - 0.1f);
	m_rcHitRect.bottom = (float)(m_flipperanim.m_hitcircleBase.center.y + m_flipperanim.m_flipperradius + m_flipperanim.m_endradius + 0.1f);
	m_rcHitRect.zlow = m_flipperanim.m_hitcircleBase.zlow;
	m_rcHitRect.zhigh = m_flipperanim.m_hitcircleBase.zhigh;
}


void FlipperAnimObjectUC::SetObjects(PINFLOAT angle)
{	
	m_angleCur = angle;
	m_hitcircleEnd.center.x = (m_hitcircleBase.center.x + m_flipperradius*sin(angle)); //place end radius center
	m_hitcircleEnd.center.y = (m_hitcircleBase.center.y + m_flipperradius*(-cos(angle)));
	m_hitcircleEnd.radius = m_endradius;

	PINFLOAT faceNormx1 = m_lineseg1.normal.x =  sin(angle - faceNormOffset); // normals to new face positions
	PINFLOAT faceNormy1 = m_lineseg1.normal.y = -cos(angle - faceNormOffset);
	PINFLOAT faceNormx2 = m_lineseg2.normal.x =  sin(angle + faceNormOffset);
	PINFLOAT faceNormy2 = m_lineseg2.normal.y = -cos(angle + faceNormOffset);

	m_lineseg1.v1.x = m_hitcircleEnd.center.x + m_hitcircleEnd.radius*faceNormx1; //new endpoints
	m_lineseg1.v1.y = m_hitcircleEnd.center.y + m_hitcircleEnd.radius*faceNormy1;

	m_lineseg1.v2.x = m_hitcircleBase.center.x + m_hitcircleBase.radius*faceNormx1;
	m_lineseg1.v2.y = m_hitcircleBase.center.y + m_hitcircleBase.radius*faceNormy1;

	m_lineseg2.v1.x = m_hitcircleBase.center.x + m_hitcircleBase.radius*faceNormx2; // remember v1 to v2 direction
	m_lineseg2.v1.y = m_hitcircleBase.center.y + m_hitcircleBase.radius*faceNormy2; // to make sure norm is correct

	m_lineseg2.v2.x = m_hitcircleEnd.center.x + m_hitcircleEnd.radius*faceNormx2;
	m_lineseg2.v2.y = m_hitcircleEnd.center.y + m_hitcircleEnd.radius*faceNormy2;
}


void FlipperAnimObjectUC::UpdateDisplacements(PINFLOAT dtime)
{
	m_angleCur += m_anglespeed*dtime;	// move flipper angle

	if (m_angleCur > m_angleMax)		// too far???
	{
		m_angleCur = m_angleMax; 

		if (m_anglespeed > 0) 
		{
			if(m_fAcc > 0) m_fAcc = 0;

			float anglespd = fabs(m_anglespeed*180/PI);
			m_anglespeed = 0; 

			//if (m_EnableRotateEvent > 0) m_pflipper->FireVoidEventParm(DISPID_LimitEvents_EOS,anglespd); // send EOS event
			//else if (m_EnableRotateEvent < 0) m_pflipper->FireVoidEventParm(DISPID_LimitEvents_BOS, anglespd);	// send Beginning of Stroke event
			m_EnableRotateEvent = 0;
		}
	}	
	else if (m_angleCur < m_angleMin)
	{
		m_angleCur = m_angleMin; 

		if (m_anglespeed < 0)
		{
			if(m_fAcc < 0) m_fAcc = 0;

			float anglespd = fabs(m_anglespeed*180/PI);
			m_anglespeed = 0;			

			//if (m_EnableRotateEvent > 0) m_pflipper->FireVoidEventParm(DISPID_LimitEvents_EOS,anglespd); // send EOS event
			//else if (m_EnableRotateEvent < 0) m_pflipper->FireVoidEventParm(DISPID_LimitEvents_BOS, anglespd);	// send Park event
			m_EnableRotateEvent = 0;
		}
	}
}

void FlipperAnimObjectUC::UpdateVelocities(PINFLOAT dtime)
{

	if (m_fAcc == 0) ;//m_anglespeed = 0; //idle
	else if (m_fAcc > 0) // postive ... increasing angle
	{			
		m_anglespeed += (m_force/m_mass)*dtime * C_FLIPPERACCEL; //new angular rate

		if (m_anglespeed > m_maxvelocity) 
		{m_anglespeed = m_maxvelocity;} //limit			
	}
	else // negative ... decreasing angle
	{		
		m_anglespeed -= (m_force/m_mass)*dtime * C_FLIPPERACCEL;	//new angular rate

		if (m_anglespeed < -m_maxvelocity) 
		{m_anglespeed = -m_maxvelocity;} //limit			
	}
}

//rlc  new hitflipper ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define LeftFace 1
#define RightFace 0

PINFLOAT HitFlipperUC::HitTest(Ball *pball, PINFLOAT dtime, Vertex3D *phitnormal)
{
	if ( (m_flipperanim.m_fEnabled == false) &&
		(m_flipperanim.m_fInvisible == false) )
	{
		return -1;
	}

	PINFLOAT hittime;
	bool lastface = m_flipperanim.m_lastHitFace;

	//m_flipperanim.SetObjects(m_flipperanim.m_angleCur);	// set current positions ... not needed

	// for effective computing, adding a last face hit value to speed calculations 
	//  a ball can only hit one face never two
	// also if a ball hits a face then it can not hit either radius
	// so only check these if a face is not hit
	// endRadius is more likely than baseRadius ... so check it first

	hittime = HitTestFlipperFace(pball, dtime, phitnormal, lastface); // first face
	if (hittime >= 0) return hittime;		

	hittime = HitTestFlipperFace(pball, dtime, phitnormal, !lastface); //second face
	if (hittime >= 0)
	{
		m_flipperanim.m_lastHitFace = !lastface;	// change this face to check first
		return hittime;
	}

	hittime = HitTestFlipperEnd(pball, dtime, phitnormal); // end radius
	if (hittime >= 0) return hittime;

	hittime = m_flipperanim.m_hitcircleBase.HitTest(pball, dtime, phitnormal);
	if (hittime >= 0)
	{		
		phitnormal[1].x = 0;			//Tangent velocity of contact point (rotate Normal right)
		phitnormal[1].y = 0;			//units: rad*d/t (Radians*diameter/time

		phitnormal[2].x = 0;			//moment is zero ... only friction
		phitnormal[2].y = 0;			//radians/time at collison

		return hittime;
	}

	return -1;	// no hits
}

// Precision level and cycles for interative calculations 

#define C_INTERATIONS 20
// acceptable contact time ... near zero time


PINFLOAT HitFlipperUC::HitTestFlipperEnd(Ball *pball, PINFLOAT dtime, Vertex3D *phitnormal) // replacement
{ 	 
	PINFLOAT ballvtx, ballvty;	// new ball position at time t in flipper face coordinate
	PINFLOAT contactAng;
	PINFLOAT radsin,radcos;	

	PINFLOAT angleCur = m_flipperanim.m_angleCur;
	PINFLOAT anglespeed = m_flipperanim.m_anglespeed;				// rotation rate

	Vertex flipperbase;
	flipperbase.x =	m_flipperanim.m_hitcircleBase.center.x;
	flipperbase.y =	m_flipperanim.m_hitcircleBase.center.y;

	PINFLOAT angleMin = m_flipperanim.m_angleMin;
	PINFLOAT angleMax = m_flipperanim.m_angleMax;

	PINFLOAT ballr = pball->radius;
	PINFLOAT feRadius = m_flipperanim.m_hitcircleEnd.radius;

	PINFLOAT ballrEndr = feRadius + ballr;			// magnititude of (ball - flipperEnd)

	PINFLOAT ballx = pball->x;
	PINFLOAT bally = pball->y;

	PINFLOAT ballvx = pball->vx;
	PINFLOAT ballvy = pball->vy;

	PINFLOAT bfend, cbcedist;
	PINFLOAT t,t0,t1, d0,d1,dp; // Modified False Position control
	int k;
	Vertex vp,vt;

	vp.x = (PINFLOAT) 0.0;					//m_flipperradius*sin(0));   
	vp.y = -m_flipperanim.m_flipperradius;  //m_flipperradius*(-cos(0));

	t = 0; //start first interval ++++++++++++++++++++++++++
	for (k=1;k <= C_INTERATIONS;++k)
	{
		// determine flipper rotation direction, limits and parking 

		contactAng = angleCur + anglespeed * t;					// angle at time t

		if (contactAng >= angleMax) contactAng = angleMax;			// stop here			
		else if (contactAng <= angleMin) contactAng = angleMin;		// stop here 

		radsin = sin(contactAng);// Green's transform matrix... rotate angle delta 
		radcos = cos(contactAng);// rotational transform from zero position to position at time t

		//rotate angle delta unit vector, rotates system according to flipper face angle		
		vt.x = vp.x *radcos - vp.y *radsin + flipperbase.x ;	//rotate and translate to world position
		vt.y = vp.y *radcos + vp.x *radsin + flipperbase.y;	

		ballvtx = ballx + ballvx*t - vt.x;	// new ball position relative to flipper end radius
		ballvty = bally + ballvy*t - vt.y;

		cbcedist = sqrt(ballvtx*ballvtx + ballvty*ballvty); // center ball to center end radius distance

		bfend = cbcedist - ballrEndr;	// ball face-to-radius surface distance

		if (fabsf(bfend) <= C_PRECISION) break; 

		if (k >= 3)// MFP root search +++++++++++++++++++++++++++++++++++++++++
		{
			if (bfend*d0 <= 0.0)									// zero crossing
			{ t1 = t; d1 = bfend; if (dp* bfend > 0.0) d0 /=2;} // 	move right interval limit			
			else 
			{ t0 = t; d0 = bfend; if (dp*bfend > 0.0) d1 /=2;}	// 	move left interval limit		
		}		
		else if (k == 2)// end pass two, check if zero crossing on initial interval, exit if none
		{	
			if (dp*bfend > 0.0) return -1;	// no solution ... no obvious zero crossing

			t0 = 0; t1 = dtime; d0 = dp; d1 = bfend; // set initial boundaries		
		}
		else // (k == 1) end of pass one ... set full interval pass, t = dtime
		{// test for extreme conditions
			if (bfend < -(PHYS_SKIN + feRadius)) return -1;	//  too deeply embedded, ambigious position					
			if (bfend <= PHYS_TOUCH) 
			{break;} // inside the clearance limits

			t0 = t1 = dtime; d0 = 0; d1 = bfend; // set for second pass, force t=dtime
		}

		t = t0 - d0*(t1 - t0)/(d1 - d0);			// estimate next t
		dp = bfend;									// remember 

	}//for loop
	//+++ End time interation loop found time t soultion ++++++

	if (t < 0 || t > dtime) return -1;	// time is outside this frame ... no collision

	if ((k > C_INTERATIONS)&& (fabsf(bfend) >  PHYS_SKIN/4)) // last ditch effort to accept a solution
	{
		return-1;// no solution	
	} 	
	// here ball and flipper end are in contact .. well in most cases, near and embedded solutions need calculations	

	PINFLOAT hitz = pball->z - ballr + pball->vz*t;	// check for a hole, relative to ball rolling point at hittime

	if ((hitz + (ballr * 1.5)) < m_rcHitRect.zlow			//check limits of object's height and depth
		|| (hitz + (ballr * 0.5)) > m_rcHitRect.zhigh)
	{return -1;}

	// ok we have a confirmed contact, calc the stats, remember there are "near" solution, so all
	// parameters need to be calculated from the actual configuration, i.e contact radius must be calc'ed

	Vertex N;
	phitnormal[0].x = N.x = ballvtx/cbcedist;				// normal vector from flipper end to ball
	phitnormal[0].y = N.y = ballvty/cbcedist;

	Vertex dist = m_flipperanim.m_hitcircleBase.center;  // flipper base center

	dist.x =  (pball->x + ballvx*t + ballr*(-N.x) - dist.x); // vector from base to flipperEnd plus the projected End radius
	dist.y =  (pball->y + ballvy*t + ballr*(-N.y) - dist.y);		

	PINFLOAT distance = sqrt(dist.x*dist.x + dist.y*dist.y);	// distance from base center to contact point

	if (contactAng >= angleMax && anglespeed > 0 || contactAng <= angleMin && anglespeed < 0)	// hit limits ??? 
		anglespeed = 0;							// rotation stopped

	phitnormal[1].x = -dist.y/distance; //Unit Tangent vector velocity of contact point(rotate normal right)
	phitnormal[1].y = dist.x/distance; // 

	phitnormal[2].x = distance;				//moment arm diameter
	phitnormal[2].y = anglespeed;			//radians/time at collison


	//recheck using actual contact angle of velocity direction
	Vertex dv;	
	dv.x = (ballvx - phitnormal[1].x *anglespeed*distance); 
	dv.y = (ballvy - phitnormal[1].y *anglespeed*distance); //delta velocity ball to face

	PINFLOAT bnv = dv.x*phitnormal->x + dv.y*phitnormal->y;  //dot Normal to delta v

	if (bnv >= 0) 
	{return -1;} // not hit ... ball is receeding from face already, must have been embedded or shallow angled

	pball->m_HitDist = bfend;				//actual contact distance ..
	pball->m_HitNormVel = bnv;
	pball->m_HitRigid = true;				// collision type

	return t;
}


PINFLOAT HitFlipperUC::HitTestFlipperFace(Ball *pball, PINFLOAT dtime, Vertex3D *phitnormal, bool face) // replacement
{ 
	Vertex F;			// flipper face normal
	Vertex T;			// flipper face tangent

	PINFLOAT bffnd;		//ball flipper face normal distance (negative for normal side)
	PINFLOAT bfftd;		// ball to flipper face tanget distance 
	PINFLOAT ballvtx;	// new ball position at time t in flipper face coordinate
	PINFLOAT ballvty;	//
	PINFLOAT contactAng;
	PINFLOAT radsin,radcos;	// rotate angle delta unit vector, rotates system according to flipper face angle

	PINFLOAT angleCur = m_flipperanim.m_angleCur;
	PINFLOAT anglespeed = m_flipperanim.m_anglespeed;				// rotation rate

	Vertex flipperbase;
	flipperbase.x =	m_flipperanim.m_hitcircleBase.center.x;
	flipperbase.y =	m_flipperanim.m_hitcircleBase.center.y;
	PINFLOAT feRadius = m_flipperanim.m_hitcircleEnd.radius;

	PINFLOAT angleMin = m_flipperanim.m_angleMin;
	PINFLOAT angleMax = m_flipperanim.m_angleMax;	

	PINFLOAT ballr = pball->radius;	
	PINFLOAT ballvx = pball->vx;
	PINFLOAT ballvy = pball->vy;	
	PINFLOAT ballrEndr = m_flipperanim.m_hitcircleEnd.radius + ballr;// magnititude of (ball - flipperEnd)

	PINFLOAT t,t0,t1, d0,d1,dp; // Modified False Position control
	int k;
	PINFLOAT ffnx, ffny, len; // flipper face normal vector
	Vertex vp,vt;				// face segment V1 point

	// flipper positions at zero degrees rotation

	ffnx = m_flipperanim.zeroAngNorm.x; //Face2 
	if (face == LeftFace) ffnx = -ffnx;	// negative for face1

	ffny = m_flipperanim.zeroAngNorm.y;// norm y component same for either face
	vp.x = m_flipperanim.m_hitcircleBase.radius*ffnx; // face endpoint of line segment on base radius
	vp.y = m_flipperanim.m_hitcircleBase.radius*ffny;		
	len  = m_flipperanim.m_lineseg1.CalcLength();//.length;	// face segment length ... i.g same on either face									

	t = 0; //start first interval ++++++++++++++++++++++++++
	for (k=1; k<=C_INTERATIONS; ++k)
	{
		// determine flipper rotation direction, limits and parking 	

		contactAng = angleCur + anglespeed * t;					// angle at time t

		if (contactAng >= angleMax) contactAng = angleMax;			// stop here			
		else if (contactAng <= angleMin) contactAng = angleMin;		// stop here 

		radsin = sin(contactAng);//  Green's transform matrix... rotate angle delta 
		radcos = cos(contactAng);//  rotational transform from current position to position at time t

		F.x = ffnx *radcos - ffny *radsin;  // rotate to time t, norm and face offset point
		F.y = ffny *radcos + ffnx *radsin;  // 

		vt.x = vp.x *radcos - vp.y *radsin + flipperbase.x;//rotate and translate to world position
		vt.y = vp.y *radcos + vp.x *radsin + flipperbase.y;

		ballvtx = pball->x + ballvx*t - vt.x;	// new ball position relative to rotated line segment endpoint
		ballvty = pball->y + ballvy*t - vt.y;	

		bffnd = ballvtx *F.x +  ballvty *F.y - ballr; // normal distance to segment 

		if (fabsf(bffnd) <= C_PRECISION) break;

		// loop control, boundary checks, next estimate, etc.

		if (k >= 3)// MFP root search +++++++++++++++++++++++++++++++++++++++++
		{
			if (bffnd*d0 <= 0.0)									// zero crossing
			{ t1 = t; d1 = bffnd; if (dp* bffnd > 0.0) d0 /=2;} // 	move right limits			
			else 
			{ t0 = t; d0 = bffnd; if (dp*bffnd > 0.0) d1 /=2;}	// move left limits							
		}		
		else if (k == 2)// end pass two, check if zero crossing on initial interval, exit
		{	
			if (dp*bffnd > 0.0) return -1;	// no solution ... no obvious zero crossing
			t0 = 0; t1 = dtime; d0 = dp; d1 = bffnd; // testing MFP estimates			
		}
		else // (k == 1) end of pass one ... set full interval pass, t = dtime
		{// test for already inside flipper plane, either embedded or beyond the face endpoints
			if (bffnd < -(PHYS_SKIN + feRadius)) return -1;		// wrong side of face, or too deeply embedded			
			if (bffnd <= PHYS_TOUCH) 
			{break;} // inside the clearance limits, go check face endpoints

			t0 = t1 = dtime; d0 = 0; d1 = bffnd; // set for second pass, so t=dtime
		}

		t = t0 - d0*(t1-t0)/(d1-d0);					// next estimate
		dp = bffnd;	// remember 
	}//for loop

	//+++ End time interation loop found time t soultion ++++++

	if (t < 0 || t > dtime) return -1;	// time is outside this frame ... no collision

	if ((k > C_INTERATIONS)&& (fabsf(bffnd) > PHYS_SKIN/4)) // last ditch effort to accept a near solution
	{
		return-1; // no solution
	} 	

	// here ball and flipper face are in contact... past the endpoints, also, don't forget embedded and near soultion

	if (face == LeftFace) 
	{ T.x = -F.y; T.y = F.x;}	// rotate to form Tangent vector				
	else { T.x = F.y; T.y = -F.x;}	// rotate to form Tangent vector

	bfftd = ballvtx * T.x + ballvty * T.y;			// ball to flipper face tanget distance	

	if (bfftd < -C_TOL_ENDPNTS || bfftd > len + C_TOL_ENDPNTS) return -1;	// not in range of touching

	PINFLOAT hitz = pball->z - ballr + pball->vz*t;	// check for a hole, relative to ball rolling point at hittime

	if ((hitz + (ballr * 1.5)) < m_rcHitRect.zlow			//check limits of object's height and depth 
		|| (hitz + (ballr * 0.5)) > m_rcHitRect.zhigh)
	{return -1;}	

	// ok we have a confirmed contact, calc the stats, remember there are "near" solution, so all
	// parameters need to be calculated from the actual configuration, i.e contact radius must be calc'ed

	phitnormal[0].x = F.x;	// hit normal is same as line segment normal
	phitnormal[0].y = F.y;	

	Vertex dist = m_flipperanim.m_hitcircleBase.center;  // calculate moment from flipper base center

	dist.x =  (pball->x + ballvx*t + ballr*(-F.x) - dist.x); //center of ball + projected radius to contact point
	dist.y =  (pball->y + ballvy*t + ballr*(-F.y) - dist.y); // all at time t

	PINFLOAT distance = sqrt(dist.x*dist.x + dist.y*dist.y);	// distance from base center to contact point

	if (contactAng >= angleMax && anglespeed > 0 || contactAng <= angleMin && anglespeed < 0)	// hit limits ??? 
		anglespeed = (PINFLOAT)0.0;							// rotation stopped

	phitnormal[1].x = -dist.y/distance;		//Unit Tangent velocity of contact point(rotate Normal clockwise)
	phitnormal[1].y =  dist.x/distance;		//

	phitnormal[2].x = distance;				//moment arm diameter
	phitnormal[2].y = anglespeed;			//radians/time at collison

	Vertex dv;	
	dv.x = (ballvx - phitnormal[1].x *anglespeed*distance); 
	dv.y = (ballvy - phitnormal[1].y *anglespeed*distance); //delta velocity ball to face

	PINFLOAT bnv = dv.x*phitnormal->x + dv.y*phitnormal->y;  //dot Normal to delta v

	if (bnv >= C_LOWNORMVEL) 
	{return -1;} // not hit ... ball is receeding from endradius already, must have been embedded

	pball->m_HitDist = bffnd;				//normal ...actual contact distance ... 
	pball->m_HitNormVel = bnv;
	pball->m_HitRigid = true;				// collision type

	return t;
}


void HitFlipperUC::Collide(Ball *pball, Vertex3D *phitnormal)
{

	PINFLOAT vx = pball->vx;
	PINFLOAT vy = pball->vy;
	PINFLOAT distance = phitnormal[2].x;												// moment .... and the flipper response
	PINFLOAT angsp = m_flipperanim.m_anglespeed;										// angular rate of flipper at impact moment
	PINFLOAT tanspd = distance * angsp;													// distance * anglespeed
	PINFLOAT flipperHit = 0;

	Vertex dv;	
	dv.x = (vx - phitnormal[1].x*tanspd); 
	dv.y = (vy - phitnormal[1].y*tanspd);												// delta velocity ball to face

	PINFLOAT dot = dv.x*phitnormal->x + dv.y*phitnormal->y;								// dot Normal to delta v

	if (dot >= -C_LOWNORMVEL )															// nearly receeding ... make sure of conditions
	{																					// otherwise if clearly approaching .. process the collision
		if (dot > C_LOWNORMVEL) return;													// is this velocity clearly receeding (i.e must > a minimum)		
#ifdef C_EMBEDDED
		if (pball->m_HitDist < -C_EMBEDDED)
			dot = -C_EMBEDSHOT;															// has ball become embedded???, give it a kick
		else return;
#endif		
	}

#ifdef C_DISP_GAIN 
	// correct displacements, mostly from low velocity blindness, an alternative to true acceleration processing
	float hdist = -C_DISP_GAIN * pball->m_HitDist;										// distance found in hit detection
	if (hdist > 1.0e-4)
	{
		if (hdist > C_DISP_LIMIT) 
		{hdist = C_DISP_LIMIT;}															// crossing ramps, delta noise
		pball->x += hdist * phitnormal->x;												// push along norm, back to free area
		pball->y += hdist * phitnormal->y;												// use the norm, but is not correct
	}
#endif		

	PINFLOAT impulse = 1.005f + m_elasticity;											// hit on static, immovable flipper ... i.e on the stops
	float obliquecorr = 0.0f;

	if ((dot < -0.25f) && (g_pplayer->m_timeCur - m_last_hittime) > 250)				// limit rate to 333 milliseconds per event 
	{
		if (!distance)flipperHit = -1;													// move event processing to end of collision handler...
		else flipperHit = -dot;															// script may delete the pinball	
	}

	m_last_hittime = g_pplayer->m_timeCur;												// keep resetting until idle for 250 milliseconds

	if (distance > 0)																	// recoil possible 
	{			
		float maxradius = m_pflipper->m_d.m_FlipperRadius + m_pflipper->m_d.m_EndRadius; 		
		float recoil = m_pflipper->m_d.m_recoil/maxradius;								// convert to Radians/time
		float tfdr = distance/maxradius; 		
		float tfr = powf(tfdr,m_pflipper->m_d.m_powerlaw);								// apply powerlaw weighting
		float dvt = dv.x * phitnormal[1].x + dv.y  * phitnormal[1].y;					// velocity transvere to flipper
		float j = tfr * impulse/(1 + (tfr/m_forcemass))*(1/m_forcemass);		
		float anglespeed = m_flipperanim.m_anglespeed + dvt/distance * j;		

		if (m_flipperanim.m_fAcc != 0)													// currently in rotation
		{
			// Check if the ball hit on the base half of the flipper.
			float clamped_distance = distance;
			if ( clamped_distance < 0.0f ) clamped_distance = 0.0f;
			if ( clamped_distance > m_pflipper->m_d.m_FlipperRadius ) clamped_distance = m_pflipper->m_d.m_FlipperRadius;
			float percentage = clamped_distance / m_pflipper->m_d.m_FlipperRadius;
			if ( percentage <= 0.50f )
			{
				// Check if the delta between base and center is non-zero.
				if ( fabs(m_pflipper->m_d.m_obliquecorrection - m_pflipper->m_d.m_baseobliquecorrection) > 0.000001f )
				{
					// Ball hit near base side of flipper.
					// Use base-weighted oblique correction.
					obliquecorr = m_flipperanim.m_fAcc * (m_pflipper->m_d.m_baseobliquecorrection + (((percentage - 0.00f) * 2.0f) * (m_pflipper->m_d.m_obliquecorrection - m_pflipper->m_d.m_baseobliquecorrection)));	// flipper trajectory correction
				}
				else
				{
					// Use center oblique correction.
					obliquecorr = m_flipperanim.m_fAcc * m_pflipper->m_d.m_obliquecorrection;	// flipper trajectory correction
				}
			}
			else
			{
				// Check if the delta between center and tip is non-zero.
				if ( fabs(m_pflipper->m_d.m_obliquecorrection - m_pflipper->m_d.m_tipobliquecorrection) > 0.000001f )
				{
					// Ball hit near tip side of flipper.
					// Use tip-weighted oblique correction.
					obliquecorr = m_flipperanim.m_fAcc * (m_pflipper->m_d.m_obliquecorrection + (((percentage - 0.50f) * 2.0f) * (m_pflipper->m_d.m_tipobliquecorrection - m_pflipper->m_d.m_obliquecorrection)));		// flipper trajectory correction
				}
				else
				{
					// Use center oblique correction.
					obliquecorr = m_flipperanim.m_fAcc * m_pflipper->m_d.m_obliquecorrection;	// flipper trajectory correction
				}
			}

			impulse = (1.005f + m_elasticity)/(1 + (tfr/m_forcemass));					// impulse for pinball
			m_flipperanim.m_anglespeed = anglespeed;									// new angle speed for flipper	
		}
		else if (recoil > 0 && fabs(anglespeed) > recoil)								// discard small static impact motions
		{ // these effects are for the flipper at EOS (End of Stroke)
			if (anglespeed < 0 && m_flipperanim.m_angleCur >= m_flipperanim.m_angleMax)	// at max angle now?
			{ // rotation toward minimum angle					
				m_flipperanim.m_force = max(fabs(anglespeed *2),0.005f);				// restoreing force
				impulse = (1.005f + m_elasticity)/(1 + (tfr/m_forcemass));				// impulse for pinball
				m_flipperanim.m_anglespeed = anglespeed;								// angle speed, less linkage losses, etc.
				m_flipperanim.m_fAcc = +1;												// set acceleration to opposite direction
				if (fabs(anglespeed) > 0.05f) 
				{m_flipperanim.m_EnableRotateEvent = 1;}								// make EOS event
			}
			else if (anglespeed > 0 && m_flipperanim.m_angleCur <= m_flipperanim.m_angleMin)// at min angle now?
			{// rotation toward maximum angle
				m_flipperanim.m_force = max(fabs(anglespeed * 2),0.005f);				// restoreing force
				impulse = (1.005f + m_elasticity)/(1 + (tfr/m_forcemass));				// impulse for pinball
				m_flipperanim.m_anglespeed = anglespeed;								// angle speed, less linkage losses, etc.
				m_flipperanim.m_fAcc = -1;												// set acceleration to opposite direction
				if (fabs(anglespeed) > 0.05f) 
				{m_flipperanim.m_EnableRotateEvent = 1;}								// make EOS event
			}
		}	
	}
	pball->vx -= impulse*dot * phitnormal->x; 											// new velocity for ball after impact
	pball->vy -= impulse*dot * phitnormal->y; 											// 


	float scatter_angle = m_pflipper->m_d.m_scatterangle;								//
	if (scatter_angle <= 0) scatter_angle = c_hardScatter;								// object specific roughness
	scatter_angle *= 1;//g_pplayer->m_ptable->m_globalDifficulty;							// apply dificulty weighting

	if (dot > -1.0) scatter_angle = 0;													// not for low velocities

	if (obliquecorr != 0 || scatter_angle  > 1.0e-5)									// trajectory correction to reduce the obliqueness 
	{
		float scatter = 2.0f* ((float)rand()/((float)RAND_MAX) - 0.5f);					// -1.0f..1.0f
		scatter *=  (1.0f - scatter*scatter)* 2.59808f * scatter_angle;					// shape quadratic distribution and scale
		scatter_angle  = obliquecorr + scatter;
		float radsin = sin(scatter_angle);												// Green's transform matrix... rotate angle delta 
		float radcos = cos(scatter_angle);												// rotational transform from current position to position at time t
		float vx = pball->vx;
		float vy = pball->vy;
		pball->vx = vx *radcos - vy *radsin;											// rotate trajectory more acutely
		pball->vy = vy *radcos + vx *radsin;											// 
	}

	pball->vx *= 0.985f; pball->vy *= 0.985f; pball->vz *= 0.96f;						// friction

	pball->m_fDynamic = C_DYNAMIC;														// reactive ball if quenched

	tanspd = m_flipperanim.m_anglespeed *distance;										// new tangential speed
	dv.x = (pball->vx - phitnormal[1].x * tanspd);										// project along unit transverse vector
	dv.y = (pball->vy - phitnormal[1].y * tanspd);										// delta velocity

	dot = dv.x*phitnormal->x + dv.y*phitnormal->y;										//dot face Normal to delta v

	if (dot < 0)
	{	//opps .... impulse calculations were off a bit, add a little boost
		dot *= -1.2f;																	// small bounce
		pball->vx += dot * phitnormal->x;												// new velocity for ball after corrected impact
		pball->vy += dot * phitnormal->y;												//
		//++c_PostCheck;																// don't count this one
	}

	// move hit event to end of collision routine, pinball may be deleted
	if (flipperHit != 0)
	{
		//if (flipperHit < 0) m_pflipper->FireGroupEvent(DISPID_HitEvents_Hit);			// simple hit event	
		//else m_pflipper->FireVoidEventParm(DISPID_FlipperEvents_Collide,flipperHit);	// collision velocity (normal to face)	
	}

	Vertex3D vnormal;
	vnormal.Set(phitnormal->x, phitnormal->y, 0);
	pball->AngularAcceleration(&vnormal);

}
//rlc end change  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


void HitFlipperUC::Draw(HDC hdc)
{

	m_flipperanim.m_lineseg1.Draw(hdc);
	m_flipperanim.m_lineseg2.Draw(hdc);

	SelectObject(hdc, GetStockObject(BLACK_PEN));

	DrawArc(hdc, m_flipperanim.m_hitcircleBase.center.x
		, m_flipperanim.m_hitcircleBase.center.y
		, m_flipperanim.m_hitcircleBase.radius
		, m_flipperanim.m_lineseg1.v2.x
		, m_flipperanim.m_lineseg1.v2.y
		, m_flipperanim.m_lineseg2.v1.x
		, m_flipperanim.m_lineseg2.v1.y);

	DrawArc(hdc, m_flipperanim.m_hitcircleEnd.center.x
		, m_flipperanim.m_hitcircleEnd.center.y
		, m_flipperanim.m_hitcircleEnd.radius
		, m_flipperanim.m_lineseg2.v2.x
		, m_flipperanim.m_lineseg2.v2.y
		, m_flipperanim.m_lineseg1.v1.x
		, m_flipperanim.m_lineseg1.v1.y);
}


void FlipperAnimObjectUC::Check3D()
{
	int frame;
	LPDIRECTDRAWSURFACE7 pdds;

	pdds = g_pplayer->m_pin3d.m_pddsBackBuffer;

	if (m_fEnabled)
	{
		frame = (int)((m_angleCur-m_frameStart)/(m_frameEnd-m_frameStart) * (m_vddsFrame.Size()-1));
	}
	else
	{
		frame = -1;
	}

	if (frame != m_iframe)
	{
		m_iframe = frame;

		m_fInvalid = fTrue;
	}
}

/*ObjFrame *FlipperAnimObject::Draw3D(RECT *prc)
{
	LPDIRECTDRAWSURFACE7 pdds;

	if (m_iframe == -1) return NULL;

	if (m_fEnabled)
	{
		pdds = g_pplayer->m_pin3d.m_pddsBackBuffer;

		ObjFrame *pobjframe = m_vddsFrame.ElementAt(m_iframe);

		return pobjframe;
	}

	return NULL;
}*/
