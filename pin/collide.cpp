#include "stdafx.h"

float c_maxBallSpeedSqr = C_SPEEDLIMIT*C_SPEEDLIMIT; 
float c_dampingFriction = C_DAMPFRICTION;
float c_plungerNormalize = C_PLUNGERNORMALIZE;
bool c_plungerFilter = false;

float c_hardScatter = 0.0f;
float c_hardFriction = 1.0f - C_FRICTIONCONST;

HitObject *CreateCircularHitPoly(const float x, const float y, const float z, const float r, const int sections)
	{
	Vertex3Ds * const rgv3d = new Vertex3Ds[sections];

	const float inv_sections = (float)(M_PI*2.0)/(float)sections;
	
	for (int i=0; i<sections; ++i)
		{
		const float angle = inv_sections * (float)i;

		rgv3d[i].x = x + sinf(angle) * r;
		rgv3d[i].y = y + cosf(angle) * r;
		rgv3d[i].z = z;
		}

	return new Hit3DPoly(rgv3d, sections);
	}

HitObject::HitObject() : m_fEnabled(fTrue), m_ObjType(eNull), m_pObj(NULL),
						 m_elasticity(0.3f), m_pfedebug(NULL)
	{
	}

void LineSeg::CalcHitRect()
	{
	// Allow roundoff
	m_rcHitRect.left = min(v1.x, v2.x);
	m_rcHitRect.right = max(v1.x, v2.x);
	m_rcHitRect.top = min(v1.y, v2.y);
	m_rcHitRect.bottom = max(v1.y, v2.y);

	//m_rcHitRect.zlow = 0; //!!?
	//m_rcHitRect.zhigh = 50;
	}

float LineSeg::HitTestBasic(const Ball * pball, const float dtime, CollisionEvent& coll, const bool direction, const bool lateral, const bool rigid)
	{
	if (!m_fEnabled || pball->fFrozen) return -1.0f;	

	const float ballvx = pball->vel.x;							// ball velocity
	const float ballvy = pball->vel.y;

	const float bnv = ballvx*normal.x + ballvy*normal.y;	// ball velocity normal to segment, positive if receding, zero=parallel
	bool bUnHit = (bnv > C_LOWNORMVEL);

	if (direction && bUnHit)								// direction true and clearly receding from normal face
		{
		return -1.0f;
		}

	const float ballx = pball->pos.x;							// ball position
	const float bally = pball->pos.y;

	// ball normal distance: contact distance normal to segment. lateral contact subtract the ball radius 

	const float rollingRadius = lateral ? pball->radius : C_TOL_RADIUS;	  //lateral or rolling point
	const float bcpd = (ballx - v1.x)*normal.x + (bally - v1.y)*normal.y; // ball center to plane distance
	const float bnd = bcpd - rollingRadius;

	const bool inside = (bnd <= 0.f);									  // in ball inside object volume
	
	//HitTestBasic
	float hittime;
	if (rigid)
		{
		if (bnv > C_LOWNORMVEL || bnd < (float)(-PHYS_SKIN) || (lateral && bcpd < 0)) return -1.0f;	// (ball normal distance) excessive pentratration of object skin ... no collision HACK
			
		if (lateral && (bnd <= (float)PHYS_TOUCH))
			{
			if (inside || (fabsf(bnv) > C_CONTACTVEL)				// fast velocity, return zero time
																	//zero time for rigid fast bodies				
			|| (bnd <= (float)(-PHYS_TOUCH)))
				hittime = 0;										// slow moving but embedded
			else
				hittime = bnd*(float)(1.0/(2.0*PHYS_TOUCH)) + 0.5f;	// don't compete for fast zero time events
            }
		else if (fabsf(bnv) > C_LOWNORMVEL) 					// not velocity low ????
			hittime = bnd/(-bnv);								// rate ok for safe divide 
		else return -1.0f;										// wait for touching
		}
	else //non-rigid ... target hits
		{
		if (bnv * bnd >= 0)										// outside-receding || inside-approaching
			{
			if ((m_ObjType != eTrigger) ||						// no a trigger
			    (!pball->m_vpVolObjs) ||
			    (fabsf(bnd) >= (float)(PHYS_SKIN/2.0)) ||		// not to close ... nor to far away
			    (inside != (pball->m_vpVolObjs->IndexOf(m_pObj) < 0))) // ...ball outside and hit set or  ball inside and no hit set
				return -1.0f;
			
			hittime = 0;
			bUnHit = !inside;	// ball on outside is UnHit, otherwise it's a Hit
			}
		else
			hittime = bnd/(-bnv);	
		}

	if (infNaN(hittime) || hittime < 0 || hittime > dtime) return -1.0f; // time is outside this frame ... no collision

	const float btv = ballvx*normal.y - ballvy*normal.x;				 //ball velocity tangent to segment with respect to direction from V1 to V2
	const float btd = (ballx - v1.x)*normal.y - (bally - v1.y)*normal.x  // ball tangent distance 
					+ btv * hittime;								     // ball tangent distance (projection) (initial position + velocity * hitime)

	if (btd < -C_TOL_ENDPNTS || btd > length + C_TOL_ENDPNTS) // is the contact off the line segment??? 
		return -1.0f;

	if (!rigid)												  // non rigid body collision? return direction
		coll.normal[1].x = bUnHit ? 1.0f : 0.0f;				  // UnHit signal is receding from outside target
	
	const float ballr = pball->radius;
	const float hitz = pball->pos.z - ballr + pball->vel.z*hittime;  // check too high or low relative to ball rolling point at hittime

	if (hitz + ballr * 1.5f < m_rcHitRect.zlow				  // check limits of object's height and depth  
		|| hitz + ballr * 0.5f > m_rcHitRect.zhigh)
		return -1.0f;

	coll.normal->x = normal.x;				// hit normal is same as line segment normal
	coll.normal->y = normal.y;
		
	coll.distance = bnd;					// actual contact distance ...
	coll.normVel = bnv;
	coll.hitRigid = rigid;				// collision type

	return hittime;
	}

float LineSeg::HitTest(const Ball * pball, float dtime, CollisionEvent& coll)
	{															// normal face, lateral, rigid
	return HitTestBasic(pball, dtime, coll, true, true, true);
	}

float HitCircle::HitTestBasicRadius(const Ball * pball, const float dtime, CollisionEvent& coll,
									const bool direction, const bool lateral, const bool rigid)
	{
	if (!m_fEnabled || pball->fFrozen) return -1.0f;	

	const float x = center.x;
	const float y = center.y;	
	
	const float dx = pball->pos.x - x;	// form delta components (i.e. translate coordinate frame)
	const float dy = pball->pos.y - y;

	const float dvx = pball->vel.x;	// delta velocity from ball's coordinate frame
	const float dvy = pball->vel.y;

	float targetRadius,z,dz,dvz;
	bool capsule3D;
	
	if (!lateral && pball->pos.z > zhigh)
		{
		capsule3D = true;										// handle ball over target? 
		//const float hcap = radius*(float)(1.0/5.0);			// cap height to hit-circle radius ratio
		//targetRadius = radius*radius/(hcap*2.0f) + hcap*0.5f;	// c = (r^2+h^2)/(2*h)
		targetRadius = radius*(float)(13.0/5.0);				// optimized version of above code
		//z = zhigh - (targetRadius - hcap);					// b = c - h
		z = zhigh - radius*(float)(12.0/5.0);					// optimized version of above code
		dz = pball->pos.z - z;										// ball rolling point - capsule center height 			
		dvz = pball->vel.z;										// differential velocity
		}
	else
		{
		capsule3D = false;
		targetRadius = radius;
		if(lateral)
			targetRadius += pball->radius;
		z = dz = dvz = 0.0f;
		}
	
	const float bcddsq = dx*dx + dy*dy + dz*dz;	// ball center to circle center distance ... squared

	const float bcdd = sqrtf(bcddsq);			//distance center to center
	if (bcdd <= 1.0e-6f) return -1.0f;			// no hit on exact center

	float b = dx*dvx + dy*dvy + dz*dvz;			//inner product

	const float bnv = b/bcdd;					//ball normal velocity

	if (direction && bnv > C_LOWNORMVEL) return -1.0f; // clearly receding from radius

	const float bnd = bcdd - targetRadius;		// ball normal distance to 

	const float a = dvx*dvx + dvy*dvy +dvz*dvz;	// square of the delta velocity (outer product)

	float hittime = 0;
	bool fUnhit = false;
	// Kicker is special.. handle ball stalled on kicker, commonly hit while receding, knocking back into kicker pocket
	if (m_ObjType == eKicker && bnd <= 0 && bnd >= -radius && a < C_CONTACTVEL*C_CONTACTVEL )	
		{
		if (pball->m_vpVolObjs) pball->m_vpVolObjs->RemoveElement(m_pObj);	// cause capture
		}

	if (rigid && bnd < (float)PHYS_TOUCH)		// positive: contact possible in future ... Negative: objects in contact now
		{
		if (bnd < (float)(-PHYS_SKIN)) return -1.0f;	

		if ((fabsf(bnv) > C_CONTACTVEL)			// >fast velocity, return zero time
												//zero time for rigid fast bodies
		|| (bnd <= (float)(-PHYS_TOUCH)))
			hittime = 0;						// slow moving but embedded
		else
			hittime = bnd*(float)(1.0/(2.0*PHYS_TOUCH)) + 0.5f;	// don't compete for fast zero time events
		}
	else if (m_ObjType >= eTrigger // triggers & kickers
		     && pball->m_vpVolObjs && (bnd < 0 == pball->m_vpVolObjs->IndexOf(m_pObj) < 0))
		{ // here if ... ball inside and no hit set .... or ... ball outside and hit set

		if (fabsf(bnd-radius) < 0.05f)	 // if ball appears in center of trigger, then assumed it was gen'ed there
			{
			if (pball->m_vpVolObjs) pball->m_vpVolObjs->AddElement(m_pObj);	//special case for trigger overlaying a kicker
			}										// this will add the ball to the trigger space without a Hit
		else
			{
			hittime = 0;
			fUnhit = (bnd > 0);	// ball on outside is UnHit, otherwise it's a Hit
			}
		}
	else
		{
		if((!rigid && bnd * bnv > 0) ||	// (outside and receding) or (inside and approaching)
		   (a < 1.0e-8f)) return -1.0f;	//no hit ... ball not moving relative to object

		const float c = bcddsq - targetRadius*targetRadius; // contact distance ... square delta distance (outer product)

		b += b;									// twice the (inner products)

		float result = b*b - 4.0f*a*c;			// inner products minus the outer products

		if (result < 0) return -1.0f;			// contact impossible 
			
		result = sqrtf(result);
		
		const float inv_a = (-0.5f)/a;
		const float time1 = (b - result)* inv_a;
		const float time2 = (b + result)* inv_a;
		
		fUnhit = (time1*time2 < 0);
		hittime = fUnhit ? max(time1,time2) : min(time1,time2); // ball is inside the circle

		if (infNaN(hittime) || hittime < 0 || hittime > dtime) return -1.0f; // contact out of physics frame
		}
	
	const float hitz = pball->pos.z - pball->radius + pball->vel.z * hittime; //rolling point

	if(((hitz + pball->radius *1.5f) < zlow) ||
	   (!capsule3D && (hitz + pball->radius*0.5f) > zhigh) ||
	   (capsule3D && (pball->pos.z + pball->vel.z * hittime) < zhigh)) return -1.0f;
		
	const float hitx = pball->pos.x + pball->vel.x*hittime;
	const float hity = pball->pos.y + pball->vel.y*hittime;

	const float sqrlen = (hitx - x)*(hitx - x)+(hity - y)*(hity - y);

	 if (sqrlen > 1.0e-8f) // over center???
		{//no
		const float inv_len = 1.0f/sqrtf(sqrlen);
		coll.normal->x = (hitx - x)*inv_len;
		coll.normal->y = (hity - y)*inv_len;
		}
	 else 
		{//yes over center
		coll.normal->x = 0; // make up a value, any direction is ok
		coll.normal->y = 1.0f;
		}
	
	if (!rigid)											// non rigid body collision? return direction
		coll.normal[1].x = fUnhit ? 1.0f : 0.0f;			// UnHit signal	is receding from target

	coll.distance = bnd;					//actual contact distance ... 
	coll.normVel = bnv;
	coll.hitRigid = rigid;				// collision type

	return hittime;
	}

float HitCircle::HitTestRadius(const Ball *pball, float dtime, CollisionEvent& coll)
	{	
													//normal face, lateral, rigid
	return HitTestBasicRadius(pball, dtime, coll, true, true, true);
	}	

void LineSeg::Collide(CollisionEvent *coll)
	{
    Ball *pball = coll->ball;
    Vertex3Ds *phitnormal = coll->normal;

	const float dot = phitnormal->x * pball->vel.x + phitnormal->y * pball->vel.y;

	pball->CollideWall(phitnormal, m_elasticity, m_antifriction, m_scatter);

	if (m_pfe)
		{			
		if (dot <= -m_threshold)
			{
			const float dx = pball->m_Event_Pos.x - pball->pos.x; // is this the same place as last event????
			const float dy = pball->m_Event_Pos.y - pball->pos.y; // if same then ignore it
			const float dz = pball->m_Event_Pos.z - pball->pos.z;

			if (dx*dx + dy*dy + dz*dz > 0.25f)// must be a new place if only by a little
				{
				m_pfe->FireGroupEvent(DISPID_HitEvents_Hit);
				}
			}
		
        pball->m_Event_Pos = pball->pos; //remember last collide position
		}
	}

void LineSeg::CalcNormal()
	{
	const Vertex2D vT(v1.x - v2.x, v1.y - v2.y);

	// Set up line normal
	length = sqrtf(vT.x*vT.x + vT.y*vT.y);
	const float inv_length = 1.0f/length;
	normal.x =  vT.y * inv_length;
	normal.y = -vT.x * inv_length;
	}

Joint::Joint()
	{
	radius = 0.0f;
	}

void Joint::CalcHitRect()
	{
	// Allow roundoff
	m_rcHitRect.left = center.x;
	m_rcHitRect.right = center.x;
	m_rcHitRect.top = center.y;
	m_rcHitRect.bottom = center.y;
	
	zlow = m_rcHitRect.zlow; //!!?
	zhigh = m_rcHitRect.zhigh;
	}

float Joint::HitTest(const Ball * pball, float dtime, CollisionEvent& coll)
	{
	if (!m_fEnabled)
		return -1.0f;

	return HitTestRadius(pball, dtime, coll);
	}

void Joint::Collide(CollisionEvent *coll)
	{
    Ball *pball = coll->ball;
    Vertex3Ds *phitnormal = coll->normal;

	const float dot = phitnormal->x * pball->vel.x + phitnormal->y * pball->vel.y;

	pball->CollideWall(phitnormal, m_elasticity, m_antifriction, m_scatter);

	if (m_pfe)
		{			
		if (dot <= -m_threshold)
			{
			const float dx = pball->m_Event_Pos.x - pball->pos.x; // is this the same place as last event????
			const float dy = pball->m_Event_Pos.y - pball->pos.y; // if same then ignore it
			const float dz = pball->m_Event_Pos.z - pball->pos.z;

			if (dx*dx + dy*dy + dz*dz > 0.25f) // must be a new place if only by a little
				{
				m_pfe->FireGroupEvent(DISPID_HitEvents_Hit);
				}
			}
		
        pball->m_Event_Pos = pball->pos;        //remember last collide position
		}
	}

void HitCircle::CalcHitRect()
	{
	// Allow roundoff
	m_rcHitRect.left = center.x - radius;
	m_rcHitRect.right = center.x + radius;
	m_rcHitRect.top = center.y - radius;
	m_rcHitRect.bottom = center.y + radius;
	m_rcHitRect.zlow = zlow;
	m_rcHitRect.zhigh = zhigh;
	}

float HitCircle::HitTest(const Ball * pball, float dtime, CollisionEvent& coll)
	{
	return HitTestRadius(pball, dtime, coll);
	}

void HitCircle::Collide(CollisionEvent *coll)
{
    coll->ball->CollideWall(coll->normal, m_elasticity, m_antifriction, m_scatter);
}


void DoHitTest(Ball *pball, HitObject *pho)
{
#ifdef _DEBUGPHYSICS
    g_pplayer->c_deepTested++;
#endif
    const float newtime = pho->HitTest(pball, pball->m_coll.hittime, pball->m_coll);
    if ((newtime >= 0) && (newtime <= pball->m_coll.hittime))
    {
        pball->m_coll.obj = pho;
        pball->m_coll.hittime = newtime;
        pball->m_coll.hitx = pball->pos.x + pball->vel.x*newtime;
        pball->m_coll.hity = pball->pos.y + pball->vel.y*newtime;
    }
}

