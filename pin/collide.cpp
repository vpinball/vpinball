#include "stdafx.h"

//spare


PINFLOAT c_maxBallSpeedSqed = C_SPEEDLIMIT*C_SPEEDLIMIT; 
PINFLOAT c_dampingFriction = 0.95f;
PINFLOAT c_plungerNormalize = (PINFLOAT)(1.0/13.0);  //match button plunger physics
bool c_plungerFilter = false;

PINFLOAT c_hardScatter = 0;
PINFLOAT c_hardFriction = 1.0f - RC_FRICTIONCONST;
PINFLOAT c_Gravity = GRAVITYCONST;

U32 c_PostCheck = 0;

HitObject *CreateCircularHitPoly(const PINFLOAT x, const PINFLOAT y, const PINFLOAT z, const PINFLOAT r, const int sections)
	{
	Vertex3D * const rgv3d = new Vertex3D[sections];

	const float inv_sections = (float)(M_PI*2.0)/(float)sections;
	
	for (int i=0;i<sections;i++)
		{
		const float angle = inv_sections * (float)i;

		rgv3d[i].x = x + sinf(angle) * r;
		rgv3d[i].y = y + cosf(angle) * r;
		rgv3d[i].z = z;
		}

	Hit3DPoly * const ph3p = new Hit3DPoly(rgv3d, sections);

	delete rgv3d;

	return ph3p;
	}

HitObject::HitObject() : m_fEnabled(fTrue), m_ObjType(eNull), m_pObj(NULL)
						, m_elasticity(0.3f), m_pfedebug(NULL)
	{
	}

void LineSeg::CalcHitRect()
	{
	// Allow roundoff
	m_rcHitRect.left = min(v1.x, v2.x);
	m_rcHitRect.right = max(v1.x, v2.x);
	m_rcHitRect.top = min(v1.y, v2.y);
	m_rcHitRect.bottom = max(v1.y, v2.y);
	//m_rcHitRect.zlow = 0;
	//m_rcHitRect.zhigh = 50;
	}
//rlc begin change >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

#define TANX  normal.y
#define TANY -normal.x

PINFLOAT LineSeg::HitTestBasic(Ball *pball, PINFLOAT dtime, Vertex3Ds *phitnormal,
							   bool direction, bool lateral, bool rigid)
	{
	if (!m_fEnabled || pball->fFrozen) return -1.0f;	

	PINFLOAT hittime;
	PINFLOAT ballvx = pball->vx;						// ball velocity
	PINFLOAT ballvy = pball->vy;

	PINFLOAT bnv = ballvx*normal.x + ballvy*normal.y;	//ball velocity normal to segment, positive if receding, zero=parallel

	if (direction &&  bnv > C_LOWNORMVEL)				//direction true and clearly receding from normal face
		{
#ifndef SHOWNORMAL
		check1 = (bnv < 0.0f);							// true is approaching to normal face: !UnHit signal
#endif
		return -1.0f;
		}

	const PINFLOAT ballx = pball->x;	//ball position
	const PINFLOAT bally = pball->y;

	// ball normal distance: contact distance normal to segment. lateral contact subtract the ball radius 

	const PINFLOAT rollingRadius = (lateral ? pball->radius : C_TOL_RADIUS);	//lateral or rolling point
	const PINFLOAT bcpd = (ballx - v1.x)*normal.x + (bally - v1.y)*normal.y ;	// ball center to plane distance
	const PINFLOAT bnd = bcpd - rollingRadius;
	const PINFLOAT btv = ballvx*TANX + ballvy*TANY;		//ball velocity tangent to segment with respect to direction from V1 to V2
	PINFLOAT btd = (ballx - v1.x)*TANX + (bally - v1.y)* TANY ;	// ball tangent distance 


	bool bUnHit = (bnv > C_LOWNORMVEL);
	const bool inside = (bnd <= 0);							// in ball inside object volume
	
	//HitTestBasic
	if (rigid)
		{
		if (bnv > C_LOWNORMVEL || bnd < (float)(-PHYS_SKIN) || (lateral && bcpd < 0)) return -1.0f;	// (ball normal distance) excessive pentratration of object skin ... no collision HACK
			
		if (lateral && bnd >= (float)(-PHYS_SKIN) && bnd <= (float)PHYS_TOUCH)
			{
			if (inside || fabsf(bnv) > C_CONTACTVEL)						// fast velocity, return zero time
				hittime = 0;												//zero time for rigid fast bodies				
			else if(bnd <= (float)(-PHYS_TOUCH)) hittime = 0;				// slow moving but embedded
			else hittime = bnd*(float)(1.0/(2.0*PHYS_TOUCH)) + 0.5f;	// don't compete for fast zero time events
            }
		else if (fabsf(bnv) > C_LOWNORMVEL )					// not velocity low ????
			hittime = bnd/(-bnv);								// rate ok for safe divide 
		else return -1.0f;										// wait for touching
		}
	else //non-rigid ... target hits
		{
		if (bnv * bnd >= 0)										// outside-receding || inside-approaching
			{
			if (m_ObjType != eTrigger) return -1.0f;				// no a trigger

			if (!pball->m_vpVolObjs) return -1.0f;

			if (fabsf(bnd) >= (float)(PHYS_SKIN/2.0))			// not to close ... nor to far away
				return -1.0f;
			
			const bool hit = pball->m_vpVolObjs->IndexOf(m_pObj) >= 0;	// hit already???

			if (inside == !hit) // ...ball outside and hit set or  ball inside and no hit set
				{
				hittime = 0;
				bUnHit = !inside;	// ball on outside is UnHit, otherwise it's a Hit
				}	
			else return -1.0f;	
			}
		else hittime = bnd/(-bnv);	
		}

	if (hittime < 0 || hittime > dtime) return -1.0f;	// time is outside this frame ... no collision

	btd += btv * hittime;	// ball tangent distance (projection) (initial position + velocity * hitime)

	if (btd < -C_TOL_ENDPNTS|| btd > length + C_TOL_ENDPNTS) // is the contact off the line segment??? 
		return -1.0f;

	if (!rigid)												// non rigid body collision? return direction
		phitnormal[1].x = (float)bUnHit;	// UnHit signal	is receding from outside target
	
	
	const PINFLOAT ballr = pball->radius;
	const PINFLOAT hitz = pball->z - ballr + pball->vz*hittime;	// check too high or low relative to ball rolling point at hittime

	if ((hitz + (ballr * 1.5f)) < m_rcHitRect.zlow			//check limits of object's height and depth  
		|| (hitz + (ballr * 0.5f)) > m_rcHitRect.zhigh)
		return -1.0f;

	phitnormal->x = normal.x;				// hit normal is same as line segment normal
	phitnormal->y = normal.y;
		
	pball->m_HitDist = bnd;					//actual contact distance ... 
	pball->m_HitNormVel = bnv;
	pball->m_HitRigid = rigid;				// collision type

	return hittime;
	}	

PINFLOAT LineSeg::HitTest(Ball *pball, PINFLOAT dtime, Vertex3Ds *phitnormal) 
	{															// normal face, lateral, rigid
	return HitTestBasic(pball, dtime, phitnormal, true, true, true);
	}


PINFLOAT HitCircle::HitTestBasicRadius(Ball *pball, PINFLOAT dtime, Vertex3Ds *phitnormal, bool direction,bool lateral, bool rigid)
	{
	if (!m_fEnabled || pball->fFrozen) return -1.0f;	

	PINFLOAT hittime; 
	BOOL fUnhit;
	
	bool capsule3D = false;

	const PINFLOAT x = center.x;
	const PINFLOAT y = center.y;	
	
	const PINFLOAT dx = pball->x - x;	// form delta components (i.e. translate coordinate frame)
	const PINFLOAT dy = pball->y - y;

	const PINFLOAT dvx = pball->vx;		// delta velocity from ball's coordinate frame
	const PINFLOAT dvy = pball->vy;

	PINFLOAT z,dz,dvz;
	z = dz = dvz = 0.0f;

	PINFLOAT targetRadius = lateral ? radius + pball->radius : radius;	// 	
	
	if (!lateral && pball->z > zhigh)
		{
		const float hcap =  radius*(float)(1.0/5.0);			// cap height to hit-circle radius ratio
		capsule3D = true;										// handle ball over target? 
		dvz = pball->vz;										// differential velocity
		targetRadius = (radius*radius + hcap*hcap)/(hcap*2.0f);	// c = (r^2+h^2)/(2*h)
		z = zhigh - (targetRadius - hcap);						// b = c - h
		dz = pball->z  - z;										// ball rolling point - capsule center height 			
		}
	
	const PINFLOAT bcddsq = dx*dx + dy*dy + dz*dz;	// ball center to circle center distance ... squared

	const PINFLOAT bcdd = sqrtf(bcddsq);			//distance center to center

	PINFLOAT b = dx*dvx + dy*dvy + dz*dvz;			//inner product

	PINFLOAT bnv;
	if (bcdd > 1.0e-6f) bnv = b/bcdd;				//ball normal velocity, 
	else return -1.0f;								// no hit on exact center

	if (direction && bnv > C_LOWNORMVEL) return -1.0f; // clearly receding from radius

	PINFLOAT a = dvx*dvx + dvy*dvy +dvz*dvz;	// square of the delta velocity (outer product)
 
	const PINFLOAT bnd = bcdd - targetRadius;		// ball normal distance to 

// Kicker is special.. handle ball stalled on kicker, commonly hit while receding, knocking back into kicker pocket
	if (m_ObjType == eKicker && bnd <= 0 && bnd >= -radius &&  a < C_CONTACTVEL*C_CONTACTVEL )	
		{
		if (pball->m_vpVolObjs) pball->m_vpVolObjs->RemoveElement(m_pObj);	// cause capture
		}

	if (rigid && bnd < (float)PHYS_TOUCH)			// postive: contact possible in future ... Negative: objects in contact now
		{
		if (bnd < (float)(-PHYS_SKIN)) return -1.0f;	

		if (fabsf(bnv) > C_CONTACTVEL)				// >fast velocity, return zero time
			hittime = 0;							//zero time for rigid fast bodies
		else if(bnd <= (float)(-PHYS_TOUCH)) hittime = 0;						// slow moving but embedded
		else hittime = bnd*(float)(1.0/(2.0*PHYS_TOUCH)) + 0.5f;	// don't compete for fast zero time events
		}
	else if (m_ObjType >= eTrigger && pball->m_vpVolObjs && (bnd < 0 == pball->m_vpVolObjs->IndexOf(m_pObj) < 0))
		{ // here if ... ball inside and no hit set .... or ... ball outside and hit set

		if (fabsf(bnd -radius) < 0.05f)	 // if ball appears in center of trigger, then assumed it was gen'ed there
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
		if(!rigid && bnd * bnv > 0) return -1.0f;	// (outside and receding) or (inside and approaching)

		const PINFLOAT c = bcddsq - targetRadius*targetRadius; // contact distance ... square delta distance (outer product)

		if (a < 1.0e-8f) return -1.0f;				//no hit ... ball not moving relative to object

		b += b;										// twice the (inner products)

		PINFLOAT result = b*b - 4.0f*a*c;			// inner products minus the outer products

		if (result < 0) return -1.0f;				// contact impossible 
			
		result = sqrtf(result); a += a;

		const float inv_a = 1.0f/a;
		const PINFLOAT time1 = (-b + result)* inv_a;
		const PINFLOAT time2 = (-b - result)* inv_a;
		
		if (time1*time2 < 0)						// ball is inside the circle
			{
			fUnhit = fTrue;
			hittime = max(time1,time2);  			
			}
		else
			{
			fUnhit = fFalse;
			hittime = min(time1,time2);
			}

		if (hittime < 0 || hittime > dtime) return -1.0f;	// contact out of physics frame
		}
	
	const PINFLOAT hitz = pball->z - pball->radius + pball->vz * hittime; //rolling point

	if ((hitz + pball->radius *1.50f) < zlow)return -1.0f;

	if (!capsule3D && (hitz + pball->radius*0.5f) > zhigh) return -1.0f;

	if (capsule3D && (pball->z + pball->vz * hittime) < zhigh) return -1.0f;
		
	const PINFLOAT hitx = pball->x + pball->vx*hittime;
	const PINFLOAT hity = pball->y + pball->vy*hittime;

	const PINFLOAT len = sqrtf((hitx - x)*(hitx - x)+(hity - y)*(hity - y));

	 if (len > 1.0e-8f)										// over center???
		{//no
		const float inv_len = 1.0f/len;
		phitnormal->x = (hitx - x)*inv_len;
		phitnormal->y = (hity - y)*inv_len;
		}
	 else 
		{//yes over center
		phitnormal->x = 0;	// make up a value, any direction is ok
		phitnormal->y = 1.0f;	
		}
	
	if (!rigid)											// non rigid body collision? return direction
		phitnormal[1].x = (float)fUnhit;				// UnHit signal	is receding from target

	pball->m_HitDist = bnd;					//actual contact distance ... 
	pball->m_HitNormVel = bnv;
	pball->m_HitRigid = rigid;				// collision type

	return hittime;	
	}	


PINFLOAT HitCircle::HitTestRadius(Ball *pball, PINFLOAT dtime, Vertex3Ds *phitnormal)
	{	
													//normal face, lateral, rigid
	return HitTestBasicRadius(pball, dtime, phitnormal, true, true, true);		
	}	

//rlc end  <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

void LineSeg::Draw(HDC hdc)
	{
	SelectObject(hdc, GetStockObject(BLACK_PEN));

	DrawLine(hdc, v1.x, v1.y, v2.x, v2.y);
	//MoveToEx(hdc, (int)(plineseg->v1.x*zoom), (int)(plineseg->v1.y*zoom), NULL);
	//LineTo(hdc, (int)(plineseg->v2.x*zoom), (int)(plineseg->v2.y*zoom));

#ifdef SHOWNORMAL
	const float x = (v1.x + v2.x) *0.5f;
	const float y = (v1.y + v2.y) *0.5f;

	if (!check1)
		{
		SelectObject(hdc, GetStockObject(BLACK_PEN));
		}
	else
		{
		SelectObject(hdc, GetStockObject(WHITE_PEN));
		}

	SelectObject(hdc, GetStockObject(BLACK_PEN));

	DrawLine(hdc, x, y, x+normal.x*20.0f, y+normal.y*20.0f);
#endif
	}

void LineSeg::Collide(Ball *pball, Vertex3Ds *phitnormal)
	{
	const float dot = phitnormal->x * pball->vx + phitnormal->y * pball->vy;

	pball->CollideWall(phitnormal, m_elasticity, m_antifriction, m_scatter);

	if (m_pfe)
		{			
		if (dot <= -m_threshold)
			{
			const float dx = pball->m_Event_Pos.x - pball->x; // is this the same place as last event????
			const float dy = pball->m_Event_Pos.y - pball->y; // if same then ignore it
			const float dz = pball->m_Event_Pos.z - pball->z;

			if (dx*dx + dy*dy + dz*dz > 0.25f)// must be a new place if only by a little
				{
				m_pfe->FireGroupEvent(DISPID_HitEvents_Hit);
				}
			}
		
		pball->m_Event_Pos.x = pball->x; 
		pball->m_Event_Pos.y = pball->y; 
		pball->m_Event_Pos.z = pball->z; //remember last collide position
		}
	}

void LineSeg::CalcNormal()
	{
	Vertex2D vT;
	vT.x = v1.x - v2.x;
	vT.y = v1.y - v2.y;

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
	
	zlow = m_rcHitRect.zlow;
	zhigh = m_rcHitRect.zhigh;
	}

PINFLOAT Joint::HitTest(Ball *pball, PINFLOAT dtime, Vertex3Ds *phitnormal)
	{
	if (!m_fEnabled)
		{
		return -1.0f;
		}

	return HitTestRadius(pball, dtime, phitnormal);	
	}

void Joint::Draw(HDC hdc)
	{
	SelectObject(hdc, GetStockObject(BLACK_BRUSH));
	SelectObject(hdc, GetStockObject(BLACK_PEN));
	DrawCircleAbsolute(hdc, center.x, center.y, 3.0f);
	}

void Joint::Collide(Ball *pball, Vertex3Ds *phitnormal)
	{
	const float dot = phitnormal->x * pball->vx + phitnormal->y * pball->vy;

	pball->CollideWall(phitnormal, m_elasticity, m_antifriction, m_scatter);

	if (m_pfe)
		{			
		if (dot <= -m_threshold)
			{
			const float dx = pball->m_Event_Pos.x - pball->x; // is this the same place as last event????
			const float dy = pball->m_Event_Pos.y - pball->y; // if same then ignore it
			const float dz = pball->m_Event_Pos.z - pball->z;

			if (dx*dx + dy*dy + dz*dz > 0.25f) // must be a new place if only by a little
				{
				m_pfe->FireGroupEvent(DISPID_HitEvents_Hit);
				}
			}
		
		pball->m_Event_Pos.x = pball->x; 
		pball->m_Event_Pos.y = pball->y; 
		pball->m_Event_Pos.z = pball->z; //remember last collide position
		}
	}

void HitCircle::CalcHitRect()
	{
	// Allow roundoff
	m_rcHitRect.left = center.x - radius;
	m_rcHitRect.right = center.x + radius;
	m_rcHitRect.top = center.y  - radius;
	m_rcHitRect.bottom = center.y + radius;
	m_rcHitRect.zlow = zlow;
	m_rcHitRect.zhigh = zhigh;
	}

PINFLOAT HitCircle::HitTest(Ball *pball, PINFLOAT dtime, Vertex3Ds *phitnormal)
	{
	return HitTestRadius(pball, dtime, phitnormal);
	}


void HitCircle::Draw(HDC hdc)
	{
	SelectObject(hdc, GetStockObject(NULL_BRUSH));
	SelectObject(hdc, GetStockObject(BLACK_PEN));
	DrawCircle(hdc, center.x, center.y, radius);
	}

void HitCircle::Collide(Ball *pball, Vertex3Ds *phitnormal)
	{
	pball->CollideWall(phitnormal, m_elasticity, m_antifriction, m_scatter);
	}

HitOctree::~HitOctree()
	{
#ifdef HITLOG
	if (g_fWriteHitDeleteLog)
		{
		FILE *file = fopen("c:\\log.txt", "a");
		fprintf(file,"Deleting %f %f %f %f %d\n", m_rectbounds.left, m_rectbounds.top, m_rectbounds.right, m_rectbounds.bottom, m_fLeaf); 
		fclose(file);
		}
#endif

	if (!m_fLeaf)
		{
		for (int i=0;i<8;i++)
			{
			delete m_phitoct[i];
			}
		}
	}

void HitOctree::CreateNextLevel()
	{
	m_fLeaf = fFalse;

	Vector<HitObject> vRemain; // hit objects which did not go to an octant

	for (int i=0;i<8;i++)
		{
		m_phitoct[i] = new HitOctree();

		m_phitoct[i]->m_rectbounds.left = (i&1) ? m_vcenter.x : m_rectbounds.left;
		m_phitoct[i]->m_rectbounds.top = (i&2) ?  m_vcenter.y : m_rectbounds.top;
		m_phitoct[i]->m_rectbounds.zlow = (i&4) ? m_vcenter.z : m_rectbounds.zlow;

		m_phitoct[i]->m_rectbounds.right = (i&1) ?  m_rectbounds.right : m_vcenter.x;
		m_phitoct[i]->m_rectbounds.bottom = (i&2) ? m_rectbounds.bottom : m_vcenter.y;
		m_phitoct[i]->m_rectbounds.zhigh = (i&4) ? m_rectbounds.zhigh : m_vcenter.z;

		m_phitoct[i]->m_vcenter.x = (m_phitoct[i]->m_rectbounds.left + m_phitoct[i]->m_rectbounds.right)*0.5f;
		m_phitoct[i]->m_vcenter.y = (m_phitoct[i]->m_rectbounds.top + m_phitoct[i]->m_rectbounds.bottom)*0.5f;
		m_phitoct[i]->m_vcenter.z = (m_phitoct[i]->m_rectbounds.zlow + m_phitoct[i]->m_rectbounds.zhigh)*0.5f;

		m_phitoct[i]->m_fLeaf = fTrue;
		}

	int ccross = 0;
	int ccrossx = 0, ccrossy = 0;

	for (int i=0;i<m_vho.Size();i++)
		{
		int oct;
		HitObject * const pho = m_vho.ElementAt(i);

		if (pho->m_rcHitRect.right < m_vcenter.x)
			{
			oct = 0;
			}
		else if (pho->m_rcHitRect.left > m_vcenter.x)
			{
			oct = 1;
			}
		else
			{
			oct = 128;
			ccrossx++;
			}

		if (pho->m_rcHitRect.bottom < m_vcenter.y)
			{
			//oct |= 0;
			}
		else if (pho->m_rcHitRect.top > m_vcenter.y)
			{
			oct |= 2;
			}
		else
			{
			oct |= 128;
			ccrossy++;
			}

		if ((oct & 128) == 0)
			{
			m_phitoct[oct]->m_vho.AddElement(pho);
			}
		else
			{
			vRemain.AddElement(pho);
			ccross++;
			}
		}

	m_vho.RemoveAllElements();
	for (int i=0;i<vRemain.Size();i++)
		{
		m_vho.AddElement(vRemain.ElementAt(i));
		}

	if (m_vcenter.x - m_rectbounds.left > 125.0f)
		{
		for (int i=0;i<8;i++)
			{
			m_phitoct[i]->CreateNextLevel();
			}
		}
	}

#ifdef LOG
extern int cTested;
extern int cDeepTested;
#endif


/*  RLC

Hit logic needs to be expanded, during static and psudo-static conditions, multiple hits (multi-face contacts)
are possible and should be handled, with embedding (pentrations) some contacts persist for long periods
and may cause others not to be seen (masked because of their position in the object list).

A short term solution might be to rotate the object list on each collision round. Currently, its a linear array.
and some subscript magic might be needed, where the actually collision counts are used to cycle the starting position
for the next search. This could become a Ball property ... i.e my last hit object index, start at the next
and cycle around until the last hit object is the last to be tested ... this could be made complex due to 
scripts removing objects .... i.e. balls ... better study well before I start

The most effective would be to sort the search results, always moving the last hit to the end of it's grouping

At this instance, I'm reporting static contacts as random hitimes during the specific physics frame; the zero time
slot is not in the random time generator algorithm, it is offset by STATICTIME so not to compete with the fast moving
collisions

*/

void HitOctree::HitTestBall(Ball *pball)
	{
	for (int i=0; i<m_vho.Size(); i++)
		{		

#ifdef LOG
		cTested++;
#endif
		if ((pball != m_vho.ElementAt(i)) // ball can not hit it's self
			       //&& (pball->phoHitLast != m_vho.ElementAt(i)) //rlc error: don't hit test last thing hit again, why not?
			       && fRectIntersect3D(pball->m_rcHitRect, m_vho.ElementAt(i)->m_rcHitRect))
			{
#ifdef LOG
			cDeepTested++;
#endif
			const PINFLOAT newtime = m_vho.ElementAt(i)->HitTest(pball, pball->m_hittime, pball->m_hitnormal); // test for hit
			if (newtime >= 0 && newtime <= pball->m_hittime)
				{
				pball->m_pho = m_vho.ElementAt(i);
				pball->m_hittime = newtime;
				pball->m_hitx = (pball->x + pball->vx*newtime);
				pball->m_hity = (pball->y + pball->vy*newtime);
				}
			}
		}//end for loop

	if (!m_fLeaf)
		{
		const BOOL fLeft = (pball->m_rcHitRect.left <= m_vcenter.x);
		const BOOL fRight = (pball->m_rcHitRect.right >= m_vcenter.x);
		const BOOL fTop = (pball->m_rcHitRect.top <= m_vcenter.y);
		const BOOL fBottom = (pball->m_rcHitRect.bottom >= m_vcenter.y);

#ifdef LOG
		cTested++;
#endif
		if (fLeft && fTop)
			{
			m_phitoct[0]->HitTestBall(pball);
			}
		if (fRight && fTop)
			{
			m_phitoct[1]->HitTestBall(pball);
			}
		if (fLeft && fBottom)
			{
			m_phitoct[2]->HitTestBall(pball);
			}
		if (fRight && fBottom)
			{
			m_phitoct[3]->HitTestBall(pball);
			}
		}
	}

void HitOctree::HitTestXRay(Ball *pball, Vector<HitObject> *pvhoHit)
	{
	for (int i=0; i<m_vho.Size(); i++)
		{
#ifdef LOG
		cTested++;
#endif
		if ((pball != m_vho.ElementAt(i)) && fRectIntersect3D(pball->m_rcHitRect, m_vho.ElementAt(i)->m_rcHitRect))
			{
#ifdef LOG
		cDeepTested++;
#endif
			const PINFLOAT newtime = m_vho.ElementAt(i)->HitTest(pball, pball->m_hittime, pball->m_hitnormal);
			if (newtime >= 0)
				{
				pvhoHit->AddElement(m_vho.ElementAt(i));
				}
			}
		}

	if (!m_fLeaf)
		{
		const BOOL fLeft = (pball->m_rcHitRect.left <= m_vcenter.x);
		const BOOL fRight = (pball->m_rcHitRect.right >= m_vcenter.x);
		const BOOL fTop = (pball->m_rcHitRect.top <= m_vcenter.y);
		const BOOL fBottom = (pball->m_rcHitRect.bottom >= m_vcenter.y);

#ifdef LOG
		cTested++;
#endif

		if (fLeft && fTop)
			{
			m_phitoct[0]->HitTestXRay(pball, pvhoHit);
			}
		if (fRight && fTop)
			{
			m_phitoct[1]->HitTestXRay(pball, pvhoHit);
			}
		if (fLeft && fBottom)
			{
			m_phitoct[2]->HitTestXRay(pball, pvhoHit);
			}
		if (fRight && fBottom)
			{
			m_phitoct[3]->HitTestXRay(pball, pvhoHit);
			}
		}
	}
