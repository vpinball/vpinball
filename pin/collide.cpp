#include "stdafx.h"
#include <xmmintrin.h>

//spare

float c_maxBallSpeedSqed = C_SPEEDLIMIT*C_SPEEDLIMIT; 
float c_dampingFriction = 0.95f;
float c_plungerNormalize = (float)(1.0/13.0);  //match button plunger physics
bool c_plungerFilter = false;

float c_hardScatter = 0;
float c_hardFriction = 1.0f - RC_FRICTIONCONST;
float c_Gravity = GRAVITYCONST;

//U32 c_PostCheck = 0;

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

	return new Hit3DPoly(rgv3d, sections, true);
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

float LineSeg::HitTestBasic(Ball * const pball, const float dtime, Vertex3Ds * const phitnormal, const bool direction, const bool lateral, const bool rigid)
	{
	if (!m_fEnabled || pball->fFrozen) return -1.0f;	

	float ballvx = pball->vx;						// ball velocity
	float ballvy = pball->vy;

	const float bnv = ballvx*normal.x + ballvy*normal.y;	//ball velocity normal to segment, positive if receding, zero=parallel
	bool bUnHit = (bnv > C_LOWNORMVEL);

	if (direction && bUnHit)						//direction true and clearly receding from normal face
		{
#ifndef SHOWNORMAL
		check1 = (bnv < 0.0f);						// true is approaching to normal face: !UnHit signal
#endif
		return -1.0f;
		}

	const float ballx = pball->x;	//ball position
	const float bally = pball->y;

	// ball normal distance: contact distance normal to segment. lateral contact subtract the ball radius 

	const float rollingRadius = lateral ? pball->radius : C_TOL_RADIUS;	//lateral or rolling point
	const float bcpd = (ballx - v1.x)*normal.x + (bally - v1.y)*normal.y ;	// ball center to plane distance
	const float bnd = bcpd - rollingRadius;

	const bool inside = (bnd <= 0);							// in ball inside object volume
	
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
		phitnormal[1].x = bUnHit ? 1.0f : 0.0f;				  // UnHit signal is receding from outside target
	
	const float ballr = pball->radius;
	const float hitz = pball->z - ballr + pball->vz*hittime;  // check too high or low relative to ball rolling point at hittime

	if (hitz + ballr * 1.5f < m_rcHitRect.zlow			  // check limits of object's height and depth  
		|| hitz + ballr * 0.5f > m_rcHitRect.zhigh)
		return -1.0f;

	phitnormal->x = normal.x;				// hit normal is same as line segment normal
	phitnormal->y = normal.y;
		
	pball->m_HitDist = bnd;					// actual contact distance ... 
	pball->m_HitNormVel = bnv;
	pball->m_HitRigid = rigid;				// collision type

	return hittime;
	}	

float LineSeg::HitTest(Ball * const pball, const float dtime, Vertex3Ds * const phitnormal) 
	{															// normal face, lateral, rigid
	return HitTestBasic(pball, dtime, phitnormal, true, true, true);
	}

float HitCircle::HitTestBasicRadius(Ball * const pball, const float dtime, Vertex3Ds * const phitnormal,
									const bool direction, const bool lateral, const bool rigid)
	{
	if (!m_fEnabled || pball->fFrozen) return -1.0f;	

	const float x = center.x;
	const float y = center.y;	
	
	const float dx = pball->x - x;	// form delta components (i.e. translate coordinate frame)
	const float dy = pball->y - y;

	const float dvx = pball->vx;	// delta velocity from ball's coordinate frame
	const float dvy = pball->vy;

	float targetRadius,z,dz,dvz;
	bool capsule3D;
	
	if (!lateral && pball->z > zhigh)
		{
		capsule3D = true;										// handle ball over target? 
		//const float hcap = radius*(float)(1.0/5.0);			// cap height to hit-circle radius ratio
		//targetRadius = radius*radius/(hcap*2.0f) + hcap*0.5f;	// c = (r^2+h^2)/(2*h)
		targetRadius = radius*(float)(13.0/5.0);				// optimized version of above code
		//z = zhigh - (targetRadius - hcap);					// b = c - h
		z = zhigh - radius*(float)(12.0/5.0);					// optimized version of above code
		dz = pball->z - z;										// ball rolling point - capsule center height 			
		dvz = pball->vz;										// differential velocity
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
	else if (m_ObjType >= eTrigger && pball->m_vpVolObjs && (bnd < 0 == pball->m_vpVolObjs->IndexOf(m_pObj) < 0))
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

		if (infNaN(hittime) || hittime < 0 || hittime > dtime) return -1.0f;	// contact out of physics frame
		}
	
	const float hitz = pball->z - pball->radius + pball->vz * hittime; //rolling point

	if(((hitz + pball->radius *1.5f) < zlow) ||
	   (!capsule3D && (hitz + pball->radius*0.5f) > zhigh) ||
	   (capsule3D && (pball->z + pball->vz * hittime) < zhigh)) return -1.0f;
		
	const float hitx = pball->x + pball->vx*hittime;
	const float hity = pball->y + pball->vy*hittime;

	const float sqrlen = (hitx - x)*(hitx - x)+(hity - y)*(hity - y);

	 if (sqrlen > 1.0e-8f)										// over center???
		{//no
		const float inv_len = 1.0f/sqrtf(sqrlen);
		phitnormal->x = (hitx - x)*inv_len;
		phitnormal->y = (hity - y)*inv_len;
		}
	 else 
		{//yes over center
		phitnormal->x = 0;	// make up a value, any direction is ok
		phitnormal->y = 1.0f;	
		}
	
	if (!rigid)											// non rigid body collision? return direction
		phitnormal[1].x = fUnhit ? 1.0f : 0.0f;			// UnHit signal	is receding from target

	pball->m_HitDist = bnd;					//actual contact distance ... 
	pball->m_HitNormVel = bnv;
	pball->m_HitRigid = rigid;				// collision type

	return hittime;
	}

float HitCircle::HitTestRadius(Ball *pball, float dtime, Vertex3Ds *phitnormal)
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

	SelectObject(hdc, GetStockObject(!check1 ? BLACK_PEN : WHITE_PEN));
	SelectObject(hdc, GetStockObject(BLACK_PEN));

	DrawLine(hdc, x, y, x+normal.x*20.0f, y+normal.y*20.0f);
#endif
	}

void LineSeg::Collide(Ball * const pball, Vertex3Ds * const phitnormal)
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
	
	zlow = m_rcHitRect.zlow;
	zhigh = m_rcHitRect.zhigh;
	}

float Joint::HitTest(Ball * const pball, const float dtime, Vertex3Ds * const phitnormal)
	{
	if (!m_fEnabled)
		return -1.0f;

	return HitTestRadius(pball, dtime, phitnormal);	
	}

void Joint::Draw(HDC hdc)
	{
	SelectObject(hdc, GetStockObject(BLACK_BRUSH));
	SelectObject(hdc, GetStockObject(BLACK_PEN));
	DrawCircleAbsolute(hdc, center.x, center.y, 3.0f);
	}

void Joint::Collide(Ball * const pball, Vertex3Ds * const phitnormal)
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

float HitCircle::HitTest(Ball * const pball, const float dtime, Vertex3Ds * const phitnormal)
	{
	return HitTestRadius(pball, dtime, phitnormal);
	}


void HitCircle::Draw(HDC hdc)
	{
	SelectObject(hdc, GetStockObject(NULL_BRUSH));
	SelectObject(hdc, GetStockObject(BLACK_PEN));
	DrawCircle(hdc, center.x, center.y, radius);
	}

void HitCircle::Collide(Ball * const pball, Vertex3Ds * const phitnormal)
	{
	pball->CollideWall(phitnormal, m_elasticity, m_antifriction, m_scatter);
	}

HitOctree::~HitOctree()
	{
#ifdef HITLOG
	if (g_fWriteHitDeleteLog)
		{
		FILE * const file = fopen("c:\\log.txt", "a");
		fprintf(file,"Deleting %f %f %f %f %d\n", m_rectbounds.left, m_rectbounds.top, m_rectbounds.right, m_rectbounds.bottom, m_fLeaf); 
		fclose(file);
		}
#endif

	if(lefts != 0)
	{
	_aligned_free(lefts);
	_aligned_free(rights);
	_aligned_free(tops);
	_aligned_free(bottoms);
	_aligned_free(zlows);
	_aligned_free(zhighs);
	}

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
	m_fLeaf = false;

	Vector<HitObject> vRemain; // hit objects which did not go to an octant

	for (int i=0;i<8;i++)
		{
		m_phitoct[i] = new HitOctree();

		m_phitoct[i]->m_rectbounds.left = (i&1) ? m_vcenter.x : m_rectbounds.left;
		m_phitoct[i]->m_rectbounds.top  = (i&2) ? m_vcenter.y : m_rectbounds.top;
		m_phitoct[i]->m_rectbounds.zlow = (i&4) ? m_vcenter.z : m_rectbounds.zlow;

		m_phitoct[i]->m_rectbounds.right  = (i&1) ? m_rectbounds.right  : m_vcenter.x;
		m_phitoct[i]->m_rectbounds.bottom = (i&2) ? m_rectbounds.bottom : m_vcenter.y;
		m_phitoct[i]->m_rectbounds.zhigh  = (i&4) ? m_rectbounds.zhigh  : m_vcenter.z;

		m_phitoct[i]->m_vcenter.x = (m_phitoct[i]->m_rectbounds.left + m_phitoct[i]->m_rectbounds.right )*0.5f;
		m_phitoct[i]->m_vcenter.y = (m_phitoct[i]->m_rectbounds.top  + m_phitoct[i]->m_rectbounds.bottom)*0.5f;
		m_phitoct[i]->m_vcenter.z = (m_phitoct[i]->m_rectbounds.zlow + m_phitoct[i]->m_rectbounds.zhigh )*0.5f;

		m_phitoct[i]->m_fLeaf = true;
		}

	//int ccross = 0;
	//int ccrossx = 0, ccrossy = 0;

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
			//ccrossx++;
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
			//ccrossy++;
			}

		if ((oct & 128) == 0)
			{
			m_phitoct[oct]->m_vho.AddElement(pho);
			}
		else
			{
			vRemain.AddElement(pho);
			//ccross++;
			}
		}

	m_vho.RemoveAllElements();
	for (int i=0;i<vRemain.Size();i++)
		{
		m_vho.AddElement(vRemain.ElementAt(i));
		}

	if (m_vcenter.x - m_rectbounds.left > 125.0f)
		{
		for (int i=0; i<8; ++i)
			{
			m_phitoct[i]->CreateNextLevel();
      }
		}

  InitSseArrays();
  for (int i=0; i<8; ++i)
    m_phitoct[i]->InitSseArrays();

	}

void HitOctree::InitSseArrays()
{
  // build SSE boundary arrays of the local hit-object list
  // (don't init twice)
  const int ssebytes = sizeof(float) * ((m_vho.Size()+3)/4)*4;
  if (ssebytes > 0 && lefts == 0)
  {
    lefts = (float*)_aligned_malloc(ssebytes, 16);
    rights = (float*)_aligned_malloc(ssebytes, 16);
    tops = (float*)_aligned_malloc(ssebytes, 16);
    bottoms = (float*)_aligned_malloc(ssebytes, 16);
    zlows = (float*)_aligned_malloc(ssebytes, 16);
    zhighs = (float*)_aligned_malloc(ssebytes, 16);

    //memset(lefts, 0, ssebytes);
    //memset(rights, 0, ssebytes);
    //memset(tops, 0, ssebytes);
    //memset(bottoms, 0, ssebytes);
    //memset(zlows, 0, ssebytes);
    //memset(zhighs, 0, ssebytes);

    for (int j=0;j<m_vho.Size();j++)
    {
      lefts[j] = m_vho.ElementAt(j)->m_rcHitRect.left;
      rights[j] = m_vho.ElementAt(j)->m_rcHitRect.right;
      tops[j] = m_vho.ElementAt(j)->m_rcHitRect.top;
      bottoms[j] = m_vho.ElementAt(j)->m_rcHitRect.bottom;
      zlows[j] = m_vho.ElementAt(j)->m_rcHitRect.zlow;
      zhighs[j] = m_vho.ElementAt(j)->m_rcHitRect.zhigh;
    }

	for (int j=m_vho.Size();j<((m_vho.Size()+3)/4)*4;j++)
	{
	  lefts[j] = FLT_MAX;
	  rights[j] = -FLT_MAX;
	  tops[j] = FLT_MAX;
	  bottoms[j] = -FLT_MAX;
	  zlows[j] = FLT_MAX;
	  zhighs[j] = -FLT_MAX;
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

void HitOctree::HitTestBall(Ball * const pball) const
	{
	for (int i=0; i<m_vho.Size(); i++)
		{		

#ifdef LOG
		cTested++;
#endif
		if ((pball != m_vho.ElementAt(i)) // ball can not hit itself
			       //&& (pball->phoHitLast != m_vho.ElementAt(i)) //rlc error: don't hit test last thing hit again, why not? -> numerical precison aka self intersection problem maybe?
			       && fRectIntersect3D(pball->m_rcHitRect, m_vho.ElementAt(i)->m_rcHitRect))
			{
#ifdef LOG
			cDeepTested++;
#endif
			const float newtime = m_vho.ElementAt(i)->HitTest(pball, pball->m_hittime, pball->m_hitnormal); // test for hit
			if ((newtime >= 0) && (newtime <= pball->m_hittime))
				{
				pball->m_pho = m_vho.ElementAt(i);
				pball->m_hittime = newtime;
				pball->m_hitx = pball->x + pball->vx*newtime;
				pball->m_hity = pball->y + pball->vy*newtime;
				}
			}
		}//end for loop

	if (!m_fLeaf)
		{
		const bool fLeft = (pball->m_rcHitRect.left <= m_vcenter.x);
		const bool fRight = (pball->m_rcHitRect.right >= m_vcenter.x);

#ifdef LOG
		cTested++;
#endif
		if (pball->m_rcHitRect.top <= m_vcenter.y) // Top
		{
			if (fLeft)
				m_phitoct[0]->HitTestBallSse(pball);
			if (fRight)
				m_phitoct[1]->HitTestBallSse(pball);
		}
		if (pball->m_rcHitRect.bottom >= m_vcenter.y) // Bottom
		{
			if (fLeft)
				m_phitoct[2]->HitTestBallSse(pball);
			if (fRight)
				m_phitoct[3]->HitTestBallSse(pball);
		}
		}
	}


void HitOctree::HitTestBallSseInner(Ball * const pball, const int i) const
{
  // ball can not hit itself
  if (pball == m_vho.ElementAt(i))
    return;

  const float newtime = m_vho.ElementAt(i)->HitTest(pball, pball->m_hittime, pball->m_hitnormal); // test for hit
  if ((newtime >= 0) && (newtime <= pball->m_hittime))
	{
  	pball->m_pho = m_vho.ElementAt(i);
  	pball->m_hittime = newtime;
  	pball->m_hitx = pball->x + pball->vx*newtime;
  	pball->m_hity = pball->y + pball->vy*newtime;
	}
}

void HitOctree::HitTestBallSse(Ball * const pball) const
{
  if (lefts != 0)
  {
    // init SSE registers with ball bbox
    const __m128 bleft = _mm_set1_ps(pball->m_rcHitRect.left);
    const __m128 bright = _mm_set1_ps(pball->m_rcHitRect.right);
    const __m128 btop = _mm_set1_ps(pball->m_rcHitRect.top);
    const __m128 bbottom = _mm_set1_ps(pball->m_rcHitRect.bottom);
    const __m128 bzlow = _mm_set1_ps(pball->m_rcHitRect.zlow);
    const __m128 bzhigh = _mm_set1_ps(pball->m_rcHitRect.zhigh);

    const __m128* const pL = (__m128*)lefts;
    const __m128* const pR = (__m128*)rights;
    const __m128* const pT = (__m128*)tops;
    const __m128* const pB = (__m128*)bottoms;
    const __m128* const pZl = (__m128*)zlows;
    const __m128* const pZh = (__m128*)zhighs;

    // loop implements 4 collision checks at once
    // (rc1.right >= rc2.left && rc1.bottom >= rc2.top && rc1.left <= rc2.right && rc1.top <= rc2.bottom && rc1.zlow <= rc2.zhigh && rc1.zhigh >= rc2.zlow)
    const int size = (m_vho.Size()+3)/4;
    for (int i=0; i<size; ++i)
    {
      // comparisons set bits if bounds miss. if all bits are set, there is no collision. otherwise continue comparisons
      // bits set, there is a bounding box collision
      __m128 cmp = _mm_cmpge_ps(bright, pL[i]);
      int mask = _mm_movemask_ps(cmp);
      if (mask == 0) continue;

	  cmp = _mm_cmple_ps(bleft, pR[i]);
      mask &= _mm_movemask_ps(cmp);
      if (mask == 0) continue;

      cmp = _mm_cmpge_ps(bbottom, pT[i]);
      mask &= _mm_movemask_ps(cmp);
      if (mask == 0) continue;

      cmp = _mm_cmple_ps(btop, pB[i]);
      mask &= _mm_movemask_ps(cmp);
      if (mask == 0) continue;

      cmp = _mm_cmpge_ps(bzhigh, pZl[i]);
      mask &= _mm_movemask_ps(cmp);
      if (mask == 0) continue;

	  cmp = _mm_cmple_ps(bzlow, pZh[i]);
      mask &= _mm_movemask_ps(cmp);
      if (mask == 0) continue;

      // now there is at least one bbox collision
      if ((mask & 1) != 0) HitTestBallSseInner(pball, i*4);
      if ((mask & 2) != 0 /*&& (i*4+1)<m_vho.Size()*/) HitTestBallSseInner(pball, i*4+1); // boundary checks not necessary
      if ((mask & 4) != 0 /*&& (i*4+2)<m_vho.Size()*/) HitTestBallSseInner(pball, i*4+2); //  anymore as non-valid entries were
      if ((mask & 8) != 0 /*&& (i*4+3)<m_vho.Size()*/) HitTestBallSseInner(pball, i*4+3); //  initialized to keep these maskbits 0
    }
  }

	if (!m_fLeaf)
		{
		const bool fLeft = (pball->m_rcHitRect.left <= m_vcenter.x);
		const bool fRight = (pball->m_rcHitRect.right >= m_vcenter.x);

		if (pball->m_rcHitRect.top <= m_vcenter.y) // Top
		{
			if (fLeft)
				m_phitoct[0]->HitTestBallSse(pball);
			if (fRight)
				m_phitoct[1]->HitTestBallSse(pball);
		}
		if (pball->m_rcHitRect.bottom >= m_vcenter.y) // Bottom
		{
			if (fLeft)
				m_phitoct[2]->HitTestBallSse(pball);
			if (fRight)
				m_phitoct[3]->HitTestBallSse(pball);
		}
		}
}

void HitOctree::HitTestXRay(Ball * const pball, Vector<HitObject> * const pvhoHit) const
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
			const float newtime = m_vho.ElementAt(i)->HitTest(pball, pball->m_hittime, pball->m_hitnormal);
			if (newtime >= 0)
				{
				pvhoHit->AddElement(m_vho.ElementAt(i));
				}
			}
		}

	if (!m_fLeaf)
		{
		const bool fLeft = (pball->m_rcHitRect.left <= m_vcenter.x);
		const bool fRight = (pball->m_rcHitRect.right >= m_vcenter.x);

#ifdef LOG
		cTested++;
#endif

		if (pball->m_rcHitRect.top <= m_vcenter.y) // Top
		{
			if (fLeft)
				m_phitoct[0]->HitTestXRay(pball, pvhoHit);
			if (fRight)
				m_phitoct[1]->HitTestXRay(pball, pvhoHit);
		}
		if (pball->m_rcHitRect.bottom >= m_vcenter.y) // Bottom
		{
			if (fLeft)
				m_phitoct[2]->HitTestXRay(pball, pvhoHit);
			if (fRight)
				m_phitoct[3]->HitTestXRay(pball, pvhoHit);
		}
		}
	}
