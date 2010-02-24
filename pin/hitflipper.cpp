#include "stdafx.h"
#include "..\Main.h"

HitFlipper::HitFlipper(float x, float y, float baser, float endr, float flipr, float angle, float zlow, float zhigh, float strength, float mass)
	{
	//int i;

	if (fOldPhys)
		{
		m_flipperanim.m_hitcircleBase.m_pfe = NULL;
		m_flipperanim.m_hitcircleEnd[0].m_pfe = NULL;
		m_flipperanim.m_hitcircleEnd[1].m_pfe = NULL;
		m_flipperanim.m_lineseg1[0].m_pfe = NULL;
		m_flipperanim.m_lineseg2[0].m_pfe = NULL;
		m_flipperanim.m_lineseg1[1].m_pfe = NULL;
		m_flipperanim.m_lineseg2[1].m_pfe = NULL;

		m_flipperanim.m_lineseg1[0].m_rcHitRect.zlow = zlow;
		m_flipperanim.m_lineseg1[0].m_rcHitRect.zhigh = zhigh;
		m_flipperanim.m_lineseg1[1].m_rcHitRect.zlow = zlow;
		m_flipperanim.m_lineseg1[1].m_rcHitRect.zhigh = zhigh;
		m_flipperanim.m_lineseg2[0].m_rcHitRect.zlow = zlow;
		m_flipperanim.m_lineseg2[0].m_rcHitRect.zhigh = zhigh;
		m_flipperanim.m_lineseg2[1].m_rcHitRect.zlow = zlow;
		m_flipperanim.m_lineseg2[1].m_rcHitRect.zhigh = zhigh;

		m_flipperanim.m_hitcircleEnd[0].zlow = zlow;
		m_flipperanim.m_hitcircleEnd[0].zhigh = zhigh;
		m_flipperanim.m_hitcircleEnd[1].zlow = zlow;
		m_flipperanim.m_hitcircleEnd[1].zhigh = zhigh;
		m_flipperanim.m_hitcircleBase.zlow = zlow;
		m_flipperanim.m_hitcircleBase.zhigh = zhigh;

		m_flipperanim.m_hitcircleBase.center.x = x;
		m_flipperanim.m_hitcircleBase.center.y = y;
		m_flipperanim.m_hitcircleBase.radius = baser;

		m_flipperanim.m_endradius = endr;
		m_flipperanim.m_flipperradius = flipr;

		m_flipperanim.m_angleCur = angle;
		m_flipperanim.m_angleEnd = angle;
		m_flipperanim.m_angleFrame = angle;
		//m_angleEnd = 100000000;
		m_flipperanim.m_angleDelta = 0.05f; // angle per time-slice
		m_flipperanim.m_angleSpan = 0.5f; // time-slice

		m_flipperanim.m_anglespeed = 0.1f;
		
		m_flipperanim.SetObjects(0, angle);
		m_flipperanim.SetObjects(1, angle);

		m_flipperanim.m_force = 2;
		m_flipperanim.m_mass = 1;
		m_flipperanim.m_fAcc = fFalse;

		m_flipperanim.m_iframe = -1;

		m_forcemass = strength;
		}
	else
		{
		m_flipperanim.m_height = zhigh - zlow;

		m_flipperanim.m_hitcircleBase.m_pfe = NULL;
		m_flipperanim.m_hitcircleEnd[0].m_pfe = NULL;

		m_flipperanim.m_lineseg1[0].m_pfe = NULL;
		m_flipperanim.m_lineseg2[0].m_pfe = NULL;

		m_flipperanim.m_lineseg1[0].m_rcHitRect.zlow = zlow;
		m_flipperanim.m_lineseg1[0].m_rcHitRect.zhigh = zhigh;

		m_flipperanim.m_lineseg2[0].m_rcHitRect.zlow = zlow;
		m_flipperanim.m_lineseg2[0].m_rcHitRect.zhigh = zhigh;


		m_flipperanim.m_hitcircleEnd[0].zlow = zlow;
		m_flipperanim.m_hitcircleEnd[0].zhigh = zhigh;

		m_flipperanim.m_hitcircleBase.zlow = zlow;
		m_flipperanim.m_hitcircleBase.zhigh = zhigh;

		m_flipperanim.m_hitcircleBase.center.x = x;
		m_flipperanim.m_hitcircleBase.center.y = y;

		if (baser < 0.01)
			{
			baser = (float)0.01; // must not be zero 
			}
		m_flipperanim.m_hitcircleBase.radius = baser; //radius of base section

		if (endr < 0.01)
			{
			endr = (float)0.01; // must not be zero 
			}
		m_flipperanim.m_endradius = endr;		// radius of flipper end

		if (flipr < 0.01)
			{
			flipr = (float)0.01; // must not be zero 
			}
		m_flipperanim.m_flipperradius = flipr;	//radius of flipper arc, center-to-center radius

		m_flipperanim.m_angleCur = angle;
		m_flipperanim.m_angleEnd = angle;
		m_flipperanim.m_angleFrame = angle;

		m_flipperanim.m_anglespeed = 0;
		m_flipperanim.m_lastAngspd = 0;

		m_flipperanim.SetObjectsNewPhys(angle);	

		m_flipperanim.m_fAcc = 0;
		m_flipperanim.m_mass = mass;

		m_flipperanim.m_iframe = -1;

		m_last_hittime = 0;

		m_flipperanim.m_force = 2;
		if (strength < 0.01)strength = 0.01f;
		m_forcemass = strength;

		m_flipperanim.m_maxvelocity = m_flipperanim.m_force * 4.5f;

		m_flipperanim.m_lastHitFace = false; //rlc used to optimize hit face search order

		float fa = asin ((baser-endr)/flipr); //face to centerline angle (center to center)

		m_flipperanim.faceNormOffset = PI/2- fa; //angle of normal when flipper center line at angle zero

		float len = (float)m_flipperanim.m_flipperradius*cos(fa); //Cosine of face angle X hypotenuse
		//m_flipperanim.m_lineseg1[0].length = len;
		//m_flipperanim.m_lineseg2[0].length = len;


		m_flipperanim.zeroAngNorm.x = sin(PI/2 - fa); // F2 Norm, used in Green's transform, in FPM time search
		m_flipperanim.zeroAngNorm.y = -cos(PI/2 - fa);// F1 norm, change sign of x component, i.e -zeroAngNorm.x

		m_flipperanim.m_inertia = mass;  //stubbed to mass of flipper body
		}
	}

HitFlipper::~HitFlipper()
	{
	//m_pflipper->m_phitflipper = NULL;
	}

void HitFlipper::CalcHitRect()
	{
	// Allow roundoff
	m_rcHitRect.left = (float)(m_flipperanim.m_hitcircleBase.center.x - m_flipperanim.m_flipperradius - m_flipperanim.m_endradius - 0.1f);
	m_rcHitRect.right = (float)(m_flipperanim.m_hitcircleBase.center.x + m_flipperanim.m_flipperradius + m_flipperanim.m_endradius + 0.1f);
	m_rcHitRect.top = (float)(m_flipperanim.m_hitcircleBase.center.y - m_flipperanim.m_flipperradius - m_flipperanim.m_endradius - 0.1f);
	m_rcHitRect.bottom = (float)(m_flipperanim.m_hitcircleBase.center.y + m_flipperanim.m_flipperradius + m_flipperanim.m_endradius + 0.1f);
	m_rcHitRect.zlow = m_flipperanim.m_hitcircleBase.zlow;
	m_rcHitRect.zhigh = m_flipperanim.m_hitcircleBase.zhigh;
	}

void FlipperAnimObject::SetObjects(int i, PINFLOAT angle)
	{
	PINFLOAT a,b,c;
	PINFLOAT r,r2;
	PINFLOAT x,y,x2,y2;

	m_hitcircleEnd[i].center.x = (float)(m_hitcircleBase.center.x + m_flipperradius*sin(angle));
	m_hitcircleEnd[i].center.y = (float)(m_hitcircleBase.center.y - m_flipperradius*cos(angle));
	m_hitcircleEnd[i].radius = (float)m_endradius;

	r = m_hitcircleBase.radius;
	r2 = m_hitcircleEnd[i].radius;

	x = m_hitcircleBase.center.x;
	y = m_hitcircleBase.center.y;
	x2 = m_hitcircleEnd[i].center.x;
	y2 = m_hitcircleEnd[i].center.y;

	// Use quadratic equation to calculate slope of tangent lines.
	
	a = (r*r  - 2*r2*r + r2*r2  - x2*x2  + 2*x*x2 - x*x);
	b = 2*( y - y2 )*( x - x2 );
	c = (r*r  - 2*r2*r + r2*r2  - y2*y2  + 2*y*y2 - y*y );

	PINFLOAT result, ans1, ans2;
	result = (b*b) - (4*a*c);
	PINFLOAT newangle, na2;

	if (result < 0)
		{
		result = 0; // This can happen due to floating point truncation, and base/end radii that are almost the same
		// Not the right answer, but fake it so something really wrong doesn't happen
		}

	//if (result >= 0)
		{
		ans1 = (-b + (sqrt(result))) / (2*a);
		ans2 = (-b - (sqrt(result))) / (2*a);

		newangle = atan(ans1);
		na2 = atan(ans2);

		if (y2 > y)
			{
			// Test left edge of outer circle to inner circle, to get pos/nef of tangent
			// Have to do this because atan doesn't diffierentiate between quandrants
			if (x2-r2 < x-r)
				{
				newangle = PI+newangle;
				}
			if (x2+r2 < x+r)
				{
				na2 = PI+na2;
				}
			}
		else
			{
			if (x2+r2 < x+r)
				{
				newangle = PI+newangle;
				}
			if (x2-r2 < x-r)
				{
				na2 = PI+na2;
				}
			}
		}

	m_lineseg1[i].v2.x = (float)(m_hitcircleBase.center.x + m_hitcircleBase.radius*sin(na2));
	m_lineseg1[i].v2.y = (float)(m_hitcircleBase.center.y - m_hitcircleBase.radius*cos(na2));
	m_lineseg1[i].v1.x = (float)(m_hitcircleEnd[i].center.x + m_hitcircleEnd[i].radius*sin(na2));
	m_lineseg1[i].v1.y = (float)(m_hitcircleEnd[i].center.y - m_hitcircleEnd[i].radius*cos(na2));

	m_lineseg2[i].v1.x = (float)(m_hitcircleBase.center.x - m_hitcircleBase.radius*sin(newangle));
	m_lineseg2[i].v1.y = (float)(m_hitcircleBase.center.y + m_hitcircleBase.radius*cos(newangle));
	m_lineseg2[i].v2.x = (float)(m_hitcircleEnd[i].center.x - m_hitcircleEnd[i].radius*sin(newangle));
	m_lineseg2[i].v2.y = (float)(m_hitcircleEnd[i].center.y + m_hitcircleEnd[i].radius*cos(newangle));
	
	m_lineseg1[i].CalcNormal();
	m_lineseg2[i].CalcNormal();

	if (m_lineseg1[i].v1.y == m_lineseg1[i].v2.y)
		{
		m_lineseg1[i].v2.y += 0.0001f;
		}

	if (m_lineseg2[i].v1.y == m_lineseg2[i].v2.y)
		{
		m_lineseg2[i].v2.y += 0.0001f;
		}
	}
	
void FlipperAnimObject::SetObjectsNewPhys(PINFLOAT angle)
{	
	m_angleCur = angle;
	m_hitcircleEnd[0].center.x = (m_hitcircleBase.center.x + (float)(m_flipperradius*sin(angle))); //place end radius center
	m_hitcircleEnd[0].center.y = (m_hitcircleBase.center.y + (float)(m_flipperradius*(-cos(angle))));
	m_hitcircleEnd[0].radius = (float)m_endradius;

	PINFLOAT faceNormx1 = m_lineseg1[0].normal.x =  (float)sin(angle - faceNormOffset); // normals to new face positions
	PINFLOAT faceNormy1 = m_lineseg1[0].normal.y = (float)-cos(angle - faceNormOffset);
	PINFLOAT faceNormx2 = m_lineseg2[0].normal.x =  (float)sin(angle + faceNormOffset);
	PINFLOAT faceNormy2 = m_lineseg2[0].normal.y = (float)-cos(angle + faceNormOffset);

	m_lineseg1[0].v1.x = m_hitcircleEnd[0].center.x + m_hitcircleEnd[0].radius*(float)faceNormx1; //new endpoints
	m_lineseg1[0].v1.y = m_hitcircleEnd[0].center.y + m_hitcircleEnd[0].radius*(float)faceNormy1;

	m_lineseg1[0].v2.x = m_hitcircleBase.center.x + m_hitcircleBase.radius*(float)faceNormx1;
	m_lineseg1[0].v2.y = m_hitcircleBase.center.y + m_hitcircleBase.radius*(float)faceNormy1;

	m_lineseg2[0].v1.x = m_hitcircleBase.center.x + m_hitcircleBase.radius*(float)faceNormx2; // remember v1 to v2 direction
	m_lineseg2[0].v1.y = m_hitcircleBase.center.y + m_hitcircleBase.radius*(float)faceNormy2; // to make sure norm is correct

	m_lineseg2[0].v2.x = m_hitcircleEnd[0].center.x + m_hitcircleEnd[0].radius*(float)faceNormx2;
	m_lineseg2[0].v2.y = m_hitcircleEnd[0].center.y + m_hitcircleEnd[0].radius*(float)faceNormy2;
}


void FlipperAnimObject::UpdateTimeTemp(PINFLOAT dtime)
	{
	if (fOldPhys)
		{
		PINFLOAT endangle;
		//float deltatime;

		m_angleSpan = m_angleDelta / m_anglespeed;

		endangle = m_angleCur + m_angleDelta*(dtime/m_angleSpan);

		if (endangle/*m_angleEnd*/ > m_angleCur)
			{
			if (endangle > m_angleEnd)
				{
				endangle = m_angleEnd;
				}
			}
		else if (endangle/*m_angleEnd*/ < m_angleCur)
			{
			if (endangle < m_angleEnd)
				{
				endangle = m_angleEnd;
				}
			}
		else
			{
			return;
			}

		m_angleCur = (float)endangle;

		/*if (m_angleCur > PI*2)
			{
			m_angleCur -= PI*2;
			}*/

		// Used to be in updatetimepermanent
		m_angleFrame = (float)m_angleCur;

		if (m_fAcc)
			{
			if (m_angleCur == m_angleEnd)
				{
				m_fAcc = fFalse;
				}
			}
		}
	else
		{
		m_angleCur += m_anglespeed*dtime;	// move flipper angle

		if (m_angleCur > m_angleMax)		// too far???
			{
			m_angleCur = m_angleMax; 

			if (m_anglespeed > 0) 
				{
				if(m_fAcc > 0) m_fAcc = 0;

				float anglespd = (float)fabs(m_anglespeed*180/PI);
				m_anglespeed = 0; 

				// TODO
				//if (m_EnableRotateEvent > 0) m_pflipper->FireVoidEventParm(DISPID_LimitEvents_EOS,anglespd); // send EOS event
				//else if (m_EnableRotateEvent < 0) m_pflipper->FireVoidEventParm(DISPID_LimitEvents_BOS, anglespd);	// send Beginning of Stroke event
				//m_EnableRotateEvent = 0;
				}
			}	
		else if (m_angleCur < m_angleMin)
			{
			m_angleCur = m_angleMin; 

			if (m_anglespeed < 0)
				{
				if(m_fAcc < 0) m_fAcc = 0;

				float anglespd = (float)fabs(m_anglespeed*180/PI);
				m_anglespeed = 0;			

				//if (m_EnableRotateEvent > 0) m_pflipper->FireVoidEventParm(DISPID_LimitEvents_EOS,anglespd); // send EOS event
				//else if (m_EnableRotateEvent < 0) m_pflipper->FireVoidEventParm(DISPID_LimitEvents_BOS, anglespd);	// send Park event
				//m_EnableRotateEvent = 0;
				}
			}

		}
	}
	
#define EPSILON 0.00001

void FlipperAnimObject::UpdateAcceleration(PINFLOAT dtime)
	{
	if (fOldPhys)
		{
		if (m_angleEnd > (m_angleCur+EPSILON))
			{
			m_anglespeed += (m_force/m_mass)*(float)dtime;
			if (m_anglespeed > m_maxvelocity)
				{
				m_anglespeed = m_maxvelocity;
				}
			}
		else if (m_angleEnd < (m_angleCur-EPSILON))
			{
			m_anglespeed -= (m_force/m_mass)*(float)dtime;
			if (m_anglespeed < -m_maxvelocity)
				{
				m_anglespeed = -m_maxvelocity;
				}
			}
		else
			{
			m_anglespeed = 0;
			}
		}
	else
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
	}

/*void HitFlipper::ResetFrameTime()
	{
	m_angleCur = m_angleFrame;
	}

void HitFlipper::UpdateTimePermanent()
	{
	m_angleFrame = m_angleCur;

	if (m_fAcc)
		{
		if (m_angleCur == m_angleEnd)
			{
			m_fAcc = fFalse;
			}
		m_anglespeed += m_force/m_mass;
		}
	}*/

PINFLOAT myangle;
PINFLOAT myangle2;
int fHitFlipperCount = 0;

PINFLOAT HitFlipper::HitTest(Ball *pball, PINFLOAT dtime, Vertex3D *phitnormal)
	{
	PINFLOAT val;
	PINFLOAT best = dtime;
	BOOL fHit = fFalse;
	Vertex3D normalT;
	//int i;
	Ball ballT;
	PINFLOAT AngleCur = m_flipperanim.m_angleCur;
	BOOL fClockwise;

	if (!m_flipperanim.m_fEnabled)
		{
		return -1;
		}
		
	if (fOldPhys)
		{
		ballT = *pball;

		//m_angleCur = 0;

		myangle = -1;

		PINFLOAT time;

		best = dtime;

		m_flipperanim.m_angleSpan = m_flipperanim.m_angleDelta / fabs(m_flipperanim.m_anglespeed);

		for (time = 0;time < dtime; time += m_flipperanim.m_angleSpan)
			{
			PINFLOAT endangle;
			//Vertex DeltaV;
			PINFLOAT deltatime;

			//deltatime = m_angleSpan;
			deltatime = min(m_flipperanim.m_angleSpan, dtime-time);

			PINFLOAT speedmult = deltatime/m_flipperanim.m_angleSpan;

			PINFLOAT angspeed;

			angspeed = m_flipperanim.m_anglespeed;

			if (m_flipperanim.m_angleEnd > AngleCur)
				{
				endangle = AngleCur + m_flipperanim.m_angleDelta*speedmult;
				if (endangle > m_flipperanim.m_angleEnd)
					{
					endangle = m_flipperanim.m_angleEnd;
					}
				fClockwise = fTrue;
				}
			else if (m_flipperanim.m_angleEnd < AngleCur)
				{
				endangle = AngleCur - m_flipperanim.m_angleDelta*speedmult;
				if (endangle < m_flipperanim.m_angleEnd)
					{
					endangle = m_flipperanim.m_angleEnd;
					}
				fClockwise = fFalse;
				}
			else
				{
				//SetObjects(0, AngleCur);
				//break;
				endangle = AngleCur;
				angspeed = 0;
				fClockwise = false; // Just to make it equal to something
				}
		
			m_flipperanim.SetObjects(0, AngleCur);
			m_flipperanim.SetObjects(1, endangle);

			ballT.vx = pball->vx;
			ballT.vy = pball->vy;

			ballT.x = pball->x + pball->vx*time;
			ballT.y = pball->y + pball->vy*time;
			
			Vertex DeltaPos;

			//DeltaPos.x = m_lineseg2[1].v2.x - m_lineseg2[0].v2.x;
			//DeltaPos.y = m_lineseg2[1].v2.y - m_lineseg2[0].v2.y;

			DeltaPos.x = m_flipperanim.m_hitcircleEnd[1].center.x - m_flipperanim.m_hitcircleEnd[0].center.x;
			DeltaPos.y = m_flipperanim.m_hitcircleEnd[1].center.y - m_flipperanim.m_hitcircleEnd[0].center.y;

			PINFLOAT flipspeedx = DeltaPos.x / deltatime;
			PINFLOAT flipspeedy = DeltaPos.y / deltatime;

			// Calculate distance for torque

			Vertex V;
			V.x = (float)sin(AngleCur);
			V.y = (float)-cos(AngleCur);
			Vertex W;
			W.x = (float)ballT.x - m_flipperanim.m_hitcircleBase.center.x;
			W.y = (float)ballT.y - m_flipperanim.m_hitcircleBase.center.y;
			PINFLOAT distance = V.x * W.x + V.y*W.y;

			//distance = distance * 0.1f;

			// Hit test against static flipper

			/*val = m_hitcircleEnd[0].HitTest(&ballT, deltatime, &normalT);
			if (val != -1 && (val+time < best))// && val > time)
				{
				myangle = AngleCur;
				myangle2 = endangle;
				best = val+time;
				fHit = fTrue;
				*phitnormal = normalT;

				phitnormal[1].x = (float)cos(endangle) * angspeed * distance;
				phitnormal[1].y = (float)sin(endangle) * angspeed * distance;
				phitnormal[2].x = distance;
				}*/

			if (fClockwise)
				{
				val = m_flipperanim.m_lineseg2[0].HitTest(&ballT, deltatime, &normalT);
				if ((val != -1) && (val+time < best))
					{
					myangle = AngleCur;
					myangle2 = endangle;
					best = val+time;
					fHit = fTrue;
					*phitnormal = normalT;

					phitnormal[1].x = (float)(cos(endangle) * angspeed * distance);
					phitnormal[1].y = (float)(sin(endangle) * angspeed * distance);
					phitnormal[2].x = (float)distance;
					
					//phitnormal[1] = DeltaPos;
					
					//if (!fClockwise)
						//{
						//phitnormal[1].x = -phitnormal[1].x;
						//phitnormal[1].y = -phitnormal[1].y;
						//}
					}
				}
			else
				{
				val = m_flipperanim.m_lineseg1[0].HitTest(&ballT, deltatime, &normalT);
				if ((val != -1) && (val+time < best))
					{
					myangle = AngleCur;
					myangle2 = endangle;
					best = val+time;
					fHit = fTrue;
					*phitnormal = normalT;
					
					//float distance;
					//float dx,dy;
					//dx = ballT.x - m_hitcircleBase.center.x;
					//dy = ballT.y - m_hitcircleBase.center.y;
					//distance = (float)sqrt((dx*dx)+(dy*dy)) / (m_flipperradius / 2);
					phitnormal[1].x = (float)(cos(endangle) * angspeed * distance);
					phitnormal[1].y = (float)(sin(endangle) * angspeed * distance);
					phitnormal[2].x = (float)distance;
					
					//phitnormal[1] = DeltaPos;
					/*if (fClockwise)
						{
						phitnormal[1].x = -phitnormal[1].x;
						phitnormal[1].y = -phitnormal[1].y;
						}*/
					}
				}

			//DeltaPos.x = m_hitcircleEnd[1].center.x - m_hitcircleEnd[0].center.x;
			//DeltaPos.y = m_hitcircleEnd[1].center.y - m_hitcircleEnd[0].center.y;

			ballT.vx -= flipspeedx;
			ballT.vy -= flipspeedy;

			m_flipperanim.m_lineseg2[1].v1.x -= DeltaPos.x;
			m_flipperanim.m_lineseg2[1].v1.y -= DeltaPos.y;
			m_flipperanim.m_lineseg2[1].v2.x -= DeltaPos.x;
			m_flipperanim.m_lineseg2[1].v2.y -= DeltaPos.y;

			m_flipperanim.m_lineseg1[1].v1.x -= DeltaPos.x;
			m_flipperanim.m_lineseg1[1].v1.y -= DeltaPos.y;
			m_flipperanim.m_lineseg1[1].v2.x -= DeltaPos.x;
			m_flipperanim.m_lineseg1[1].v2.y -= DeltaPos.y;

			m_flipperanim.m_hitcircleEnd[1].center.x -= DeltaPos.x;
			m_flipperanim.m_hitcircleEnd[1].center.y -= DeltaPos.y;

			val = m_flipperanim.m_hitcircleEnd[1].HitTest(&ballT, deltatime, &normalT);
			if ((val != -1) && (val+time < best))
				{
				myangle = AngleCur;
				myangle2 = endangle;
				best = val+time;
				fHit = fTrue;
				*phitnormal = normalT;
				
				//float distance;
				//float dx,dy;
				//dx = ballT.x - m_hitcircleBase.center.x;
				//dy = ballT.y - m_hitcircleBase.center.y;
				//distance = (float)sqrt((dx*dx)+(dy*dy)) / (m_flipperradius / 2);
				phitnormal[1].x = (float)(cos(endangle) * angspeed * distance);
				phitnormal[1].y = (float)(sin(endangle) * angspeed * distance);
				phitnormal[2].x = (float)distance;
				
				//phitnormal[1] = DeltaPos;
				}

			// Only test the forward wavefront
			//if (fClockwise)
				{
				val = m_flipperanim.m_lineseg2[1].HitTest(&ballT, deltatime, &normalT);
				if ((val != -1) && (val+time < best))
					{
					myangle = AngleCur;
					myangle2 = endangle;
					best = val+time;
					fHit = fTrue;
					*phitnormal = normalT;
					
					//float distance;
					//float dx,dy;
					//dx = ballT.x - m_hitcircleBase.center.x;
					//dy = ballT.y - m_hitcircleBase.center.y;
					//distance = (float)sqrt((dx*dx)+(dy*dy)) / (m_flipperradius / 2);
					phitnormal[1].x = (float)(cos(endangle) * angspeed * distance);
					phitnormal[1].y = (float)(sin(endangle) * angspeed * distance);
					phitnormal[2].x = (float)distance;
					
					//phitnormal[1] = DeltaPos;
					}
				}
			//else // Counterclockwise
				{
				val = m_flipperanim.m_lineseg1[1].HitTest(&ballT, deltatime, &normalT);
				if ((val != -1) && (val+time < best))
					{
					myangle = AngleCur;
					myangle2 = endangle;
					best = val+time;
					fHit = fTrue;
					*phitnormal = normalT;
					
					//float distance;
					//float dx,dy;
					//dx = ballT.x - m_hitcircleBase.center.x;
					//dy = ballT.y - m_hitcircleBase.center.y;
					//distance = (float)sqrt((dx*dx)+(dy*dy)) / (m_flipperradius / 2);
					phitnormal[1].x = (float)(cos(endangle) * angspeed * distance);
					phitnormal[1].y = (float)(sin(endangle) * angspeed * distance);
					phitnormal[2].x = (float)distance;
					
					//phitnormal[1] = DeltaPos;
					}
				}

			AngleCur = endangle;

			//goto foo;
			}

	/*	if (fHit)
			{
			SetObjects(0, myangle);
			SetObjects(1, myangle2);
			}*/
		//foo:

		// Only have to test base once - it doesn't move
		val = m_flipperanim.m_hitcircleBase.HitTest(pball, dtime, &normalT);
		if (val != -1 && val < best)
			{
			best = val;
			fHit = fTrue;
			*phitnormal = normalT;
			phitnormal[1].x = 0;
			phitnormal[1].y = 0;
			phitnormal[2].x = 1;
			}

		if (fHit)
			{
			if (phitnormal->x < -1000)
				{
				int i = 9;
				}

			fHitFlipperCount = 40;
			
				/*{
				CComVariant rgvar[2] = {  CComVariant(1.0f), CComVariant(1.0f)};
		
				DISPPARAMS dispparams  = {
					rgvar,
					NULL,
					1,
					0
					};
			
				m_pflipper->FireDispID(DISPID_FlipperEvents_Collide, &dispparams);
				}*/
			}

		return fHit ? best : -1;
		}
	else // newphys
		{
		PINFLOAT hittime;
		bool lastface = m_flipperanim.m_lastHitFace;

		//m_flipperanim.SetObjects(m_flipperanim.m_angleCur);	// set current positions ... not needed

		// for effective computing, adding a last face hit value to speed calculations 
		//  a ball can only hit one face never two
		// also if a ball hits a face then it can not hit either radius
		// so only check these if a face is not hit
		// endRadius is more likely than baseRadius ... so check it first

#ifdef GLOBALLOG
		fprintf(logfile, "AngleCur %f %f\n", m_flipperanim.m_angleCur, m_flipperanim.m_angleCur * 360 / (2*3.141));
#endif

		hittime = HitTestFlipperFace(pball, dtime, phitnormal, lastface); // first face
		if (hittime >= 0)
			{
#ifdef GLOBALLOG
		fprintf(logfile, "Hit Face 1 %d %f %f\n", g_pplayer->m_timeCur, pball->vx, pball->vy);
#endif
			return hittime;
			}

		hittime = HitTestFlipperFace(pball, dtime, phitnormal, !lastface); //second face
		if (hittime >= 0)
			{
			m_flipperanim.m_lastHitFace = !lastface;	// change this face to check first
#ifdef GLOBALLOG
		fprintf(logfile, "Hit Face 2 %d\n", g_pplayer->m_timeCur);
#endif
			return hittime;
			}

		hittime = HitTestFlipperEnd(pball, dtime, phitnormal); // end radius
		if (hittime >= 0)
			{
#ifdef GLOBALLOG
		fprintf(logfile, "Hit End %d\n", g_pplayer->m_timeCur);
#endif
			return hittime;
			}

		hittime = m_flipperanim.m_hitcircleBase.HitTest(pball, dtime, phitnormal);
		if (hittime >= 0)
		{
#ifdef GLOBALLOG
		fprintf(logfile, "Hit Base %d\n", g_pplayer->m_timeCur);
#endif
			phitnormal[1].x = 0;			//Tangent velocity of contact point (rotate Normal right)
			phitnormal[1].y = 0;			//units: rad*d/t (Radians*diameter/time

			phitnormal[2].x = 0;			//moment is zero ... only friction
			phitnormal[2].y = 0;			//radians/time at collison

			return hittime;
		}
		
	#ifdef GLOBALLOG
		fprintf(logfile, "No Hit %d %f %f\n", g_pplayer->m_timeCur, pball->vx, pball->vy);
	#endif

		return -1;	// no hits
		}
	}
	
PINFLOAT HitFlipper::HitTestFlipperEnd(Ball *pball, PINFLOAT dtime, Vertex3D *phitnormal) // replacement
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
	PINFLOAT feRadius = m_flipperanim.m_hitcircleEnd[0].radius;

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
	vp.y = (float)-m_flipperanim.m_flipperradius;  //m_flipperradius*(-cos(0));

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
		vt.x = (float)(vp.x *radcos - vp.y *radsin + flipperbase.x);	//rotate and translate to world position
		vt.y = (float)(vp.y *radcos + vp.x *radsin + flipperbase.y);	

		ballvtx = ballx + ballvx*t - vt.x;	// new ball position relative to flipper end radius
		ballvty = bally + ballvy*t - vt.y;

		cbcedist = sqrt(ballvtx*ballvtx + ballvty*ballvty); // center ball to center end radius distance

		bfend = cbcedist - ballrEndr;	// ball face-to-radius surface distance

		if (fabsf((float)bfend) <= C_PRECISION) break; 

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

	if ((k > C_INTERATIONS)&& (fabsf((float)bfend) >  PHYS_SKIN/4)) // last ditch effort to accept a solution
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
	phitnormal[0].x = N.x = (float)(ballvtx/cbcedist);				// normal vector from flipper end to ball
	phitnormal[0].y = N.y = (float)(ballvty/cbcedist);

	Vertex dist = m_flipperanim.m_hitcircleBase.center;  // flipper base center

	dist.x =  (float)((pball->x + ballvx*t + ballr*(-N.x) - dist.x)); // vector from base to flipperEnd plus the projected End radius
	dist.y =  (float)((pball->y + ballvy*t + ballr*(-N.y) - dist.y));		

	PINFLOAT distance = sqrt(dist.x*dist.x + dist.y*dist.y);	// distance from base center to contact point

	if (contactAng >= angleMax && anglespeed > 0 || contactAng <= angleMin && anglespeed < 0)	// hit limits ??? 
		anglespeed = 0;							// rotation stopped

	phitnormal[1].x = (float)(-dist.y/distance); //Unit Tangent vector velocity of contact point(rotate normal right)
	phitnormal[1].y = (float)(dist.x/distance); // 

	phitnormal[2].x = (float)distance;				//moment arm diameter
	phitnormal[2].y = (float)anglespeed;			//radians/time at collison


	//recheck using actual contact angle of velocity direction
	Vertex dv;	
	dv.x = (float)((ballvx - phitnormal[1].x *anglespeed*distance)); 
	dv.y = (float)((ballvy - phitnormal[1].y *anglespeed*distance)); //delta velocity ball to face

	PINFLOAT bnv = dv.x*phitnormal->x + dv.y*phitnormal->y;  //dot Normal to delta v

	if (bnv >= 0) 
	{return -1;} // not hit ... ball is receding from face already, must have been embedded or shallow angled

	pball->m_HitDist = bfend;				//actual contact distance ..
	pball->m_HitNormVel = bnv;
	pball->m_HitRigid = true;				// collision type

	return t;
}


PINFLOAT HitFlipper::HitTestFlipperFace(Ball *pball, PINFLOAT dtime, Vertex3D *phitnormal, bool face) // replacement
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
	PINFLOAT feRadius = m_flipperanim.m_hitcircleEnd[0].radius;

	PINFLOAT angleMin = m_flipperanim.m_angleMin;
	PINFLOAT angleMax = m_flipperanim.m_angleMax;	

	PINFLOAT ballr = pball->radius;	
	PINFLOAT ballvx = pball->vx;
	PINFLOAT ballvy = pball->vy;	
	PINFLOAT ballrEndr = m_flipperanim.m_hitcircleEnd[0].radius + ballr;// magnititude of (ball - flipperEnd)

	PINFLOAT t,t0,t1, d0,d1,dp; // Modified False Position control
	int k;
	PINFLOAT ffnx, ffny, len; // flipper face normal vector
	Vertex vp,vt;				// face segment V1 point

	// flipper positions at zero degrees rotation

	ffnx = m_flipperanim.zeroAngNorm.x; //Face2 
	if (face == LeftFace) ffnx = -ffnx;	// negative for face1

	ffny = m_flipperanim.zeroAngNorm.y;// norm y component same for either face
	vp.x = (float)(m_flipperanim.m_hitcircleBase.radius*ffnx); // face endpoint of line segment on base radius
	vp.y = (float)(m_flipperanim.m_hitcircleBase.radius*ffny);		
	len  = m_flipperanim.m_lineseg1[0].CalcLength();	// face segment length ... i.g same on either face									

	t = 0; //start first interval ++++++++++++++++++++++++++
	for (k=1; k<=C_INTERATIONS; ++k)
	{
		// determine flipper rotation direction, limits and parking 	

		contactAng = angleCur + anglespeed * t;					// angle at time t

		if (contactAng >= angleMax) contactAng = angleMax;			// stop here			
		else if (contactAng <= angleMin) contactAng = angleMin;		// stop here 

		radsin = sin(contactAng);//  Green's transform matrix... rotate angle delta 
		radcos = cos(contactAng);//  rotational transform from current position to position at time t

		F.x = (float)(ffnx *radcos - ffny *radsin);  // rotate to time t, norm and face offset point
		F.y = (float)(ffny *radcos + ffnx *radsin);  // 

		vt.x = (float)(vp.x *radcos - vp.y *radsin + flipperbase.x);//rotate and translate to world position
		vt.y = (float)(vp.y *radcos + vp.x *radsin + flipperbase.y);

		ballvtx = pball->x + ballvx*t - vt.x;	// new ball position relative to rotated line segment endpoint
		ballvty = pball->y + ballvy*t - vt.y;	

		bffnd = ballvtx *F.x +  ballvty *F.y - ballr; // normal distance to segment 

		if (fabsf((float)bffnd) <= C_PRECISION) break;

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

	if ((k > C_INTERATIONS)&& (fabsf((float)bffnd) > PHYS_SKIN/4)) // last ditch effort to accept a near solution
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

	dist.x =  (float)((pball->x + ballvx*t + ballr*(-F.x) - dist.x)); //center of ball + projected radius to contact point
	dist.y =  (float)((pball->y + ballvy*t + ballr*(-F.y) - dist.y)); // all at time t

	PINFLOAT distance = sqrt(dist.x*dist.x + dist.y*dist.y);	// distance from base center to contact point

	if (contactAng >= angleMax && anglespeed > 0 || contactAng <= angleMin && anglespeed < 0)	// hit limits ??? 
		anglespeed = (PINFLOAT)0.0;							// rotation stopped

	phitnormal[1].x = (float)(-dist.y/distance);		//Unit Tangent velocity of contact point(rotate Normal clockwise)
	phitnormal[1].y = (float)(dist.x/distance);		//

	phitnormal[2].x = (float)distance;				//moment arm diameter
	phitnormal[2].y = (float)anglespeed;			//radians/time at collison

	Vertex dv;	
	dv.x = (float)((ballvx - phitnormal[1].x *anglespeed*distance)); 
	dv.y = (float)((ballvy - phitnormal[1].y *anglespeed*distance)); //delta velocity ball to face

	PINFLOAT bnv = dv.x*phitnormal->x + dv.y*phitnormal->y;  //dot Normal to delta v

	if (bnv >= C_LOWNORMVEL) 
		{return -1;} // not hit ... ball is receding from endradius already, must have been embedded

	pball->m_HitDist = bffnd;				//normal ...actual contact distance ... 
	pball->m_HitNormVel = bnv;
	pball->m_HitRigid = true;				// collision type

	return t;
}

void HitFlipper::Draw(HDC hdc)
	{

	/*if (fHitFlipperCount)
		{
		char szT[100];
		int len;
		len = sprintf(szT, "In");
		ExtTextOut(hdc, 160, 220, 0, NULL, szT, len, NULL);
		fHitFlipperCount--;
		}*/

	/*if (myangle != -1)
		{
		SetObjects(0, myangle);
		SetObjects(1, myangle2);
		}*/

	m_flipperanim.m_lineseg1[0].Draw(hdc);
	m_flipperanim.m_lineseg2[0].Draw(hdc);

	SelectObject(hdc, GetStockObject(BLACK_PEN));

	DrawArc(hdc, m_flipperanim.m_hitcircleBase.center.x, m_flipperanim.m_hitcircleBase.center.y, m_flipperanim.m_hitcircleBase.radius,
			m_flipperanim.m_lineseg1[0].v2.x, m_flipperanim.m_lineseg1[0].v2.y, m_flipperanim.m_lineseg2[0].v1.x, m_flipperanim.m_lineseg2[0].v1.y);

	DrawArc(hdc, m_flipperanim.m_hitcircleEnd[0].center.x, m_flipperanim.m_hitcircleEnd[0].center.y, m_flipperanim.m_hitcircleEnd[0].radius,
			m_flipperanim.m_lineseg2[0].v2.x, m_flipperanim.m_lineseg2[0].v2.y, m_flipperanim.m_lineseg1[0].v1.x, m_flipperanim.m_lineseg1[0].v1.y);
	}

void HitFlipper::Collide(Ball *pball, Vertex3D *phitnormal)
	{
	if (fOldPhys)
		{
		PINFLOAT e = m_elasticity;
		BOOL fStillHit;
		PINFLOAT ImpulseDenominator;

	#ifdef LOG
		fprintf(g_pplayer->m_flog, "Flipper Collide %f %f\n", pball->x, pball->y);
	#endif

		fStillHit = (m_flipperanim.m_angleCur == m_flipperanim.m_angleEnd || m_flipperanim.m_anglespeed == 0);

		Vertex vel;

		vel.x = (float)pball->vx;
		vel.y = (float)pball->vy;
		vel.x -= phitnormal[1].x;
		vel.y -= phitnormal[1].y;

		PINFLOAT dot = vel.x * phitnormal->x + vel.y * phitnormal->y;

		PINFLOAT ImpulseNumerator = -(1+e) * dot;

		if (fStillHit)
			{
			ImpulseDenominator = ((1/1) + 0);
			}
		else
			{
			ImpulseDenominator = ((1.0f/1.0f) + (1/m_forcemass));
			}

		PINFLOAT impulse = ImpulseNumerator / ImpulseDenominator;

		//impulse *= 0.9f;

		pball->vx += (float)(impulse/1 * phitnormal->x);
		pball->vy += (float)(impulse/1 * phitnormal->y);

		/*Vertex V;
		V.x = (float)sin(m_angleCur);
		V.y = (float)-cos(m_angleCur);
		Vertex W;
		W.x = pball->x - m_hitcircleBase.center.x;
		W.y = pball->y - m_hitcircleBase.center.y;
		float distance = V.x * W.x + V.y*W.y;*/
		PINFLOAT distance = phitnormal[2].x;

		//m_anglespeed *= 0.9;

		BOOL fPos = (m_flipperanim.m_anglespeed > 0);

		if (!fStillHit)
			{
			if (fPos)
				{
				m_flipperanim.m_anglespeed -= (float)((impulse/distance)/m_forcemass);
				}
			else
				{
				m_flipperanim.m_anglespeed += (float)((impulse/distance)/m_forcemass);
				}
			}

		if ((fPos && m_flipperanim.m_anglespeed < 0) || (!fPos && m_flipperanim.m_anglespeed > 0))
			{
			m_flipperanim.m_anglespeed = 0;
			}

		// Friction

		/*Vertex ThruVec;
		Vertex CrossVec;

		CrossVec.x = dot * phitnormal->x;
		CrossVec.y = dot * phitnormal->y;

		ThruVec.x = pball->vx - CrossVec.x;
		ThruVec.y = pball->vy - CrossVec.y;

		//ThruVec.x *= 0.99f; // Rolling friction is low
		//ThruVec.y *= 0.99f;

		ThruVec.x *= 0.999f; // Rolling friction is low
		ThruVec.y *= 0.999f;

		pball->vx = ThruVec.x + CrossVec.x;
		pball->vy = ThruVec.y + CrossVec.y;*/
		}
	else // new phys
		{
	#ifdef GLOBALLOG
		fprintf(logfile, "Collide Flipper %d\n", g_pplayer->m_timeCur);
	#endif
		PINFLOAT vx = pball->vx;
		PINFLOAT vy = pball->vy;
		PINFLOAT distance = phitnormal[2].x;												// moment .... and the flipper response
		PINFLOAT angsp = m_flipperanim.m_anglespeed;										// angular rate of flipper at impact moment
		PINFLOAT tanspd = distance * angsp;													// distance * anglespeed
		PINFLOAT flipperHit = 0;

		Vertex dv;	
		dv.x = (float)((vx - phitnormal[1].x*tanspd)); 
		dv.y = (float)((vy - phitnormal[1].y*tanspd));												// delta velocity ball to face

		PINFLOAT dot = dv.x*phitnormal->x + dv.y*phitnormal->y;								// dot Normal to delta v

		if (dot >= -C_LOWNORMVEL )															// nearly receding ... make sure of conditions
		{																					// otherwise if clearly approaching .. process the collision
			if (dot > C_LOWNORMVEL) return;													// is this velocity clearly receding (i.e must > a minimum)		
	#ifdef C_EMBEDDED
			if (pball->m_HitDist < -C_EMBEDDED)
				dot = -C_EMBEDSHOT;															// has ball become embedded???, give it a kick
			else return;
	#endif		
		}

	#ifdef C_DISP_GAIN 
		// correct displacements, mostly from low velocity blindness, an alternative to true acceleration processing
		float hdist = (float)(-C_DISP_GAIN * pball->m_HitDist);										// distance found in hit detection
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
			float tfdr = (float)(distance/maxradius); 		
			float tfr = powf(tfdr,m_pflipper->m_d.m_powerlaw);								// apply powerlaw weighting
			float dvt = dv.x * phitnormal[1].x + dv.y  * phitnormal[1].y;					// velocity transvere to flipper
			float j = (float)(tfr * impulse/(1 + (tfr/m_forcemass))*(1/m_forcemass));		
			float anglespeed = (float)(m_flipperanim.m_anglespeed + dvt/distance * j);		

			if (m_flipperanim.m_fAcc != 0)													// currently in rotation
			{
				// Check if the ball hit on the base half of the flipper.
				float clamped_distance = (float)distance;
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
					/*if (fabs(anglespeed) > 0.05f) 
						{
						m_flipperanim.m_EnableRotateEvent = 1;
						}*/								// make EOS event
				}
				else if (anglespeed > 0 && m_flipperanim.m_angleCur <= m_flipperanim.m_angleMin)// at min angle now?
				{// rotation toward maximum angle
					m_flipperanim.m_force = max(fabs(anglespeed * 2),0.005f);				// restoreing force
					impulse = (1.005f + m_elasticity)/(1 + (tfr/m_forcemass));				// impulse for pinball
					m_flipperanim.m_anglespeed = anglespeed;								// angle speed, less linkage losses, etc.
					m_flipperanim.m_fAcc = -1;												// set acceleration to opposite direction
					/*if (fabs(anglespeed) > 0.05f) 
						{
						m_flipperanim.m_EnableRotateEvent = 1;
						}*/								// make EOS event
				}
			}	
		}
		pball->vx -= impulse*dot * phitnormal->x; 											// new velocity for ball after impact
		pball->vy -= impulse*dot * phitnormal->y; 											// 


		float scatter_angle = m_pflipper->m_d.m_scatterangle;								//
		if (scatter_angle <= 0)
			{
			scatter_angle = (float)c_hardScatter;								// object specific roughness
			}
			
		scatter_angle *= 1;//g_pplayer->m_ptable->m_globalDifficulty;							// apply dificulty weighting

		if (dot > -1.0) scatter_angle = 0;													// not for low velocities

		if (obliquecorr != 0 || scatter_angle  > 1.0e-5)									// trajectory correction to reduce the obliqueness 
		{
			float scatter = 2.0f* ((float)rand()/((float)RAND_MAX) - 0.5f);					// -1.0f..1.0f
			scatter *=  (1.0f - scatter*scatter)* 2.59808f * scatter_angle;					// shape quadratic distribution and scale
			scatter_angle  = obliquecorr + scatter;
			float radsin = sin(scatter_angle);												// Green's transform matrix... rotate angle delta 
			float radcos = cos(scatter_angle);												// rotational transform from current position to position at time t
			float vx = (float)(pball->vx);
			float vy = (float)(pball->vy);
			pball->vx = vx *radcos - vy *radsin;											// rotate trajectory more acutely
			pball->vy = vy *radcos + vx *radsin;											// 
		}

		pball->vx *= 0.985f; pball->vy *= 0.985f; pball->vz *= 0.96f;						// friction

		pball->m_fDynamic = C_DYNAMIC;														// reactive ball if quenched

		tanspd = m_flipperanim.m_anglespeed *distance;										// new tangential speed
		dv.x = (float)((pball->vx - phitnormal[1].x * tanspd));										// project along unit transverse vector
		dv.y = (float)((pball->vy - phitnormal[1].y * tanspd));										// delta velocity

		dot = dv.x*phitnormal->x + dv.y*phitnormal->y;										//dot face Normal to delta v

		if (dot < 0)
			{	//opps .... impulse calculations were off a bit, add a little boost
			dot *= -1.2f;																	// small bounce
			pball->vx += dot * phitnormal->x;												// new velocity for ball after corrected impact
			pball->vy += dot * phitnormal->y;												//
			//++c_PostCheck;																// don't count this one
			}

		// move hit event to end of collision routine, pinball may be deleted
		/*if (flipperHit != 0)
			{
			if (flipperHit < 0) m_pflipper->FireGroupEvent(DISPID_HitEvents_Hit);			// simple hit event	
			else m_pflipper->FireVoidEventParm(DISPID_FlipperEvents_Collide,flipperHit);	// collision velocity (normal to face)	
			}*/
		}

	Vertex3D vnormal;
	vnormal.Set(phitnormal->x, phitnormal->y, 0);
	pball->AngularAcceleration(&vnormal);
	}

void FlipperAnimObject::Check3D()
	{
	int frame;
	LPDIRECTDRAWSURFACE7 pdds;

	pdds = g_pplayer->m_pin3d.m_pddsBackBuffer;

	if (m_fEnabled)
		{
		frame = (int)((m_angleCur-m_frameStart)/(m_frameEnd-m_frameStart) * (m_vddsFrame.Size()/*-1*/));
		}
	else
		{
		frame = -1;
		}

	// Since the flipper frames are spread over the arc evenly, there is no frame for the very end; just use the 'last' one.
	if (frame > m_vddsFrame.Size()-1)
		{
		frame = m_vddsFrame.Size()-1;
		}

	if (frame != m_iframe)
		{
		/*if (m_iframe != -1) // erase old frame
			{
			ObjFrame *pobjframe = m_vddsFrame.ElementAt(m_iframe);

			pdds->BltFast(pobjframe->rc.left, pobjframe->rc.top, g_pplayer->m_pin3d.m_pddsStatic,
				&pobjframe->rc, DDBLTFAST_NOCOLORKEY);
			g_pplayer->m_pin3d.m_pddsZBuffer->BltFast(pobjframe->rc.left, pobjframe->rc.top,
				g_pplayer->m_pin3d.m_pddsStaticZ, &pobjframe->rc, DDBLTFAST_NOCOLORKEY);
			}*/

		m_iframe = frame;

		m_fInvalid = fTrue;
		}
	}

ObjFrame *FlipperAnimObject::Draw3D(RECT *prc)
	{
	LPDIRECTDRAWSURFACE7 pdds;

	if (m_fEnabled)
		{
		pdds = g_pplayer->m_pin3d.m_pddsBackBuffer;

		ObjFrame *pobjframe = m_vddsFrame.ElementAt(m_iframe);

		return pobjframe;
		//RECT rcUpdate;

		//rcUpdate.left = max(pobjframe->rc.left, prc->left) - pobjframe->rc.left;
		//rcUpdate.top = max(pobjframe->rc.top, prc->top) - pobjframe->rc.top;
		//rcUpdate.right = min(pobjframe->rc.right, prc->right) - pobjframe->rc.left;
		//rcUpdate.bottom = min(pobjframe->rc.bottom, prc->bottom) - pobjframe->rc.top;

		//int bltleft, blttop;
		//bltleft = max(pobjframe->rc.left, prc->left);
		//blttop = max(pobjframe->rc.top, prc->top);

		//HRESULT hr = pdds->BltFast(bltleft/*pobjframe->rc.left*/, blttop/*pobjframe->rc.top*/, pobjframe->pdds,
			//&rcUpdate/*NULL*/, DDBLTFAST_SRCCOLORKEY);
		//g_pplayer->m_pin3d.m_pddsZBuffer->BltFast(bltleft/*pobjframe->rc.left*/, blttop/*pobjframe->rc.top*/,
			//pobjframe->pddsZBuffer, &rcUpdate/*NULL*/, DDBLTFAST_NOCOLORKEY);
		}

	return NULL;

	//pdds->Blt(&pobjframe->rc, pobjframe->pdds, NULL, DDBLT_KEYSRC, NULL);
	//HRESULT hr = g_pplayer->m_pin3d.m_pddsZBuffer->Blt(&pobjframe->rc, pobjframe->pddsZBuffer, NULL, 0, NULL);
	}