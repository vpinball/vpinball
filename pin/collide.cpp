#include "stdafx.h"
#include "..\Main.h"

/*******************
Begin New Physics constants
*******************/

PINFLOAT c_maxBallSpeedSqed = C_SPEEDLIMIT*C_SPEEDLIMIT; 
PINFLOAT c_dampingFriction = 0.95f;

PINFLOAT c_hardScatter = 0;
PINFLOAT c_hardFriction = 1.0f - RC_FRICTIONCONST;

#define TANX  normal.y
#define TANY -normal.x

PINFLOAT c_plungerNormalize = 1/13.0f;  //match button plunger physics
bool c_plungerFilter = true;

/*******************
End New Physics constants
*******************/

HitObject *CreateCircularHitPoly(PINFLOAT x, PINFLOAT y, PINFLOAT z, PINFLOAT r, int sections)
	{
	int i;
	Vertex3D *rgv3d = new Vertex3D[sections];
	
	for (i=0;i<sections;i++)
		{
		PINFLOAT angle = (PI*2) / sections * i;

		rgv3d[i].x = (float)(x + sin(angle) * r);
		rgv3d[i].y = (float)(y + cos(angle) * r);
		rgv3d[i].z = (float)z;
		}

	Hit3DPoly *ph3p = new Hit3DPoly(rgv3d, sections);

	delete rgv3d;

	return ph3p;
	}

HitObject::HitObject() : m_fEnabled(fTrue), m_fSlipFactor(fTrue),
	m_elasticity(0.3f), m_pfedebug(NULL), m_ObjType(eItemPad), m_pObj(NULL)
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
	
PINFLOAT LineSeg::HitTestBasic(Ball *pball, PINFLOAT dtime, Vertex3D *phitnormal ,bool direction, bool lateral, bool rigid)
{
	if (!m_fEnabled || pball->fFrozen) return -1;

	PINFLOAT hittime;
	PINFLOAT ballvx = pball->vx;					 // ball velocity
	PINFLOAT ballvy = pball->vy;

	PINFLOAT bnv = ballvx*normal.x + ballvy*normal.y;//ball velocity normal to segment, positive if receding, zero=parallel

	if (direction &&  bnv > C_LOWNORMVEL)		//direction true and clearly receding from normal face
	{
#ifndef SHOWNORMAL
		check1 = bnv < 0.0						// true is approaching to normal face: !UnHit signal
#endif
			return -1;
	}

	PINFLOAT ballx = pball->x;	//ball position
	PINFLOAT bally = pball->y;

	// ball normal distance: contact distance normal to segment. lateral contact subtract the ball radius 

	PINFLOAT rollingRadius = (lateral ? pball->radius : C_TOL_RADIUS);	//lateral or rolling point
	PINFLOAT bcpd = (ballx - v1.x)*normal.x + (bally - v1.y)*normal.y ;	// ball center to plane distance
	PINFLOAT bnd = bcpd - rollingRadius;
	PINFLOAT btv = ballvx*TANX + ballvy*TANY;		//ball velocity tangent to segment with respect to direction from V1 to V2
	PINFLOAT btd = (ballx - v1.x)*TANX + (bally - v1.y)* TANY ;	// ball tangent distance 


	bool bUnHit = (bnv > C_LOWNORMVEL);
	bool inside = bnd <= 0;							// in ball inside object volume

	//HitTestBasic
	if (rigid)
	{
		if (bnv > C_LOWNORMVEL || bnd < -PHYS_SKIN || (lateral && bcpd < 0)) return -1;	// (ball normal distance) excessive pentratration of object skin ... no collision HACK

		if (lateral && bnd >= -PHYS_SKIN && bnd <= PHYS_TOUCH)
		{
			if (inside || fabsf((float)bnv) > C_CONTACTVEL)						// fast velocity, return zero time
				{
				hittime = 0;												//zero time for rigid fast bodies				
				}
			else if(bnd <= -PHYS_TOUCH) hittime = 0;						// slow moving but embedded
			else hittime = (bnd + PHYS_TOUCH) * (1.0f/PHYS_TOUCH/2);		// don't compete for fast zero time events
		}
		else if (fabsf((float)bnv) > C_LOWNORMVEL )					// not velocity low ????
			hittime = bnd/(-bnv);								// rate ok for safe divide 
		else return -1;											// wait for touching
	}
	else //non-rigid ... target hits
	{
		if (bnv * bnd >= 0)										// outside-receding || inside-approaching
		{
			if (m_ObjType != eTrigger)return -1;				// no a trigger

			if (!pball->m_vpVolObjs) return -1;

			if (fabs(bnd) >= PHYS_SKIN/2)			// not to close ... nor to far away
			{return -1;}

			bool hit = pball->m_vpVolObjs->IndexOf(m_pObj) >= 0;	// hit already???

			if (inside == !hit) // ...ball outside and hit set or  ball inside and no hit set
			{
				hittime = 0;
				bUnHit = !inside;	// ball on outside is UnHit, otherwise it's a Hit
			}	
			else return -1;	
		}
		else hittime = bnd/(-bnv);
	}

	if (hittime < 0 || hittime > dtime) return -1;	// time is outside this frame ... no collision

	btd += btv * hittime;	// ball tangent distance (projection) (initial position + velocity * hitime)

	if (btd < -C_TOL_ENDPNTS|| btd > CalcLength() + C_TOL_ENDPNTS) // is the contact off the line segment??? 
	{return -1;}

	if (!rigid)												// non rigid body collision? return direction
	{phitnormal[1].x = (float)bUnHit;}	// UnHit signal	is receding from outside target


	PINFLOAT ballr = pball->radius;
	PINFLOAT hitz = pball->z - ballr + pball->vz*hittime;	// check too high or low relative to ball rolling point at hittime

	if ((hitz + (ballr * 1.5)) < m_rcHitRect.zlow			//check limits of object's height and depth  
		|| (hitz + (ballr * 0.5)) > m_rcHitRect.zhigh)
	{return -1;}

	phitnormal->x = normal.x;				// hit normal is same as line segment normal
	phitnormal->y = normal.y;

	pball->m_HitDist = bnd;					//actual contact distance ... 
	pball->m_HitNormVel = bnv;
	pball->m_HitRigid = rigid;				// collision type

	return hittime;
}	

PINFLOAT LineSeg::HitTest(Ball *pball, PINFLOAT dtime, Vertex3D *phitnormal)
	{
	if (!fOldPhys)
	{
		return HitTestBasic(pball, dtime, phitnormal, true, true, true);
	}
	//PINFLOAT a,b,c; // convert y=mx+b to ax+by+c=0
	//PINFLOAT slope,intersect;
	PINFLOAT endx, endy;

	PINFLOAT hitx, hity;
	//PINFLOAT angle;

	PINFLOAT x1,y1,x2,y2;

	PINFLOAT ballvx = pball->vx;
	PINFLOAT ballvy = pball->vy;
	PINFLOAT ballx;
	PINFLOAT bally;

	if ((ballvx*normal.x + ballvy*normal.y) > 0 || !m_fEnabled)
		{
#ifdef SHOWNORMAL
		check1 = fFalse;
#endif
		return -1;
		}
	else
		{
#ifdef SHOWNORMAL
		check1 = fTrue;
#endif
		}

	x1 = v1.x;
	y1 = v1.y;
	x2 = v2.x;
	y2 = v2.y;

	ballx = pball->x;
	bally = pball->y;

	endx = ballx + ballvx*dtime;
	endy = bally + ballvy*dtime;

	// This is the point that will hit the line if it hits in a normal way (non-endpoint hit)
	// This will catch it if the ball hits straight on.
	// If the ball does not hit straight on, the joints will catch it

	PINFLOAT len = sqrt(ballvx*ballvx + ballvy*ballvy);

	hitx = (PINFLOAT)(-normal.x) * pball->radius + ballx;
	hity = (PINFLOAT)(-normal.y) * pball->radius + bally;

	// Catch the round-off error by seeing if the ball will hit in a slightly
	// negative time, meaning the ball is interseting the wall due to
	// round-off error
	PINFLOAT startx = hitx - (ballvx/len);
	PINFLOAT starty = hity - (ballvy/len);

	if (!FQuickLineIntersect(startx, starty, hitx+(ballvx*dtime), hity+(ballvy*dtime),
		x1, y1, x2, y2))
		{
		return -1;
		}

	PINFLOAT intersectx, intersecty;

	PINFLOAT A,B,C,D,E,F;

	A = -pball->vy*dtime;
	B = pball->vx*dtime;
	C = -(A*hitx + B*hity);

	D = -(y2-y1);
	E = (x2-x1);
	F = -(D*x1 + E*y1);

	PINFLOAT det = (A*E) - (B*D);

	PINFLOAT hittime;

	// return time from the more precise component.  Not so bad because actually hitting is a relatively rare event
	if (fabs(ballvx) > fabs(ballvy))
		{
		intersectx=(B*F-E*C)/det;
		hittime = ((intersectx-hitx)/(ballvx)); // real equation (intersectx-hitx)/(ballvx*dtime), return value multipled by dtime because result is in real time, not percentage
		}
	else
		{
		intersecty=(C*D-A*F)/det;
		hittime = ((intersecty-hity)/(ballvy));
		}

	hittime = max(hittime, 0);

	//float hitz = pball->GetZ() + pball->vz*(float)hittime;
	PINFLOAT hitz = pball->z - pball->radius + pball->vz*hittime;

	if ((hitz + (pball->radius*1.5)) < m_rcHitRect.zlow || (hitz + (pball->radius*0.5)) > m_rcHitRect.zhigh)
		{
		return -1;
		}

	phitnormal->x = normal.x;
	phitnormal->y = normal.y;
	phitnormal->z = 0;
	
	//pball->m_HitDist = bnd;					//actual contact distance ... 
	//pball->m_HitNormVel = bnv;
	//pball->m_HitRigid = rigid;				// collision type

	return hittime;
	}

void LineSeg::Draw(HDC hdc)
	{
	SelectObject(hdc, GetStockObject(BLACK_PEN));

	DrawLine(hdc, v1.x, v1.y, v2.x, v2.y);
	//MoveToEx(hdc, (int)(plineseg->v1.x*zoom), (int)(plineseg->v1.y*zoom), NULL);
	//LineTo(hdc, (int)(plineseg->v2.x*zoom), (int)(plineseg->v2.y*zoom));

	float x,y;
	x = (v1.x + v2.x) / 2;
	y = (v1.y + v2.y) / 2;

#ifdef SHOWNORMAL
	if (check1)
		{
		SelectObject(hdc, GetStockObject(BLACK_PEN));
		}
	else
		{
		SelectObject(hdc, GetStockObject(WHITE_PEN));
		}

	SelectObject(hdc, GetStockObject(BLACK_PEN));

	DrawLine(hdc, x, y, x+normal.x*20, y+normal.y*20);
#endif
	}

void LineSeg::Collide(Ball *pball, Vertex3D *phitnormal)
	{
	phitnormal->z = 0;
	pball->CollideWall(phitnormal, m_elasticity, 0, 0);

	if (m_pfe)
		{
		if ((pball->vx * pball->vx + pball->vy * pball->vy) >=
			(m_threshold*m_threshold))
			{
			m_pfe->FireGroupEvent(DISPID_HitEvents_Hit);
			//m_pep->FireGroupEvent(DISPID_HitEvents_Hit);
			}
		}
	}

void LineSeg::CalcNormal()
	{
	Calc2DNormal(&v1, &v2, &normal);
	/*Vertex vT;
	float length;

	vT.x = v1.x - v2.x;
	vT.y = v1.y - v2.y;

	// Set up line normal
	length = (float)sqrt((vT.x * vT.x) + (vT.y * vT.y));
	normal.x = vT.y / length;
	normal.y = -vT.x / length;*/
	}
	
double LineSeg::CalcLength()
	{
	Vertex vT;
	float length;

	vT.x = v1.x - v2.x;
	vT.y = v1.y - v2.y;
	
	length = (float)sqrt((vT.x * vT.x) + (vT.y * vT.y));
	
	return length;
	}

Joint::Joint()
	{
	radius = 0;
	}

void Joint::CalcHitRect()
	{
	// Allow roundoff
	m_rcHitRect.left = center.x;
	m_rcHitRect.right = center.x;
	m_rcHitRect.top = center.y;
	m_rcHitRect.bottom = center.y;
	//m_rcHitRect.zlow = 0;
	//m_rcHitRect.zhigh = 50;
	zlow = m_rcHitRect.zlow;
	zhigh = m_rcHitRect.zhigh;
	}

PINFLOAT Joint::HitTest(Ball *pball, PINFLOAT dtime, Vertex3D *phitnormal)
	{
	/*PINFLOAT m, q;
	PINFLOAT u,w; // w should be v ( v is taken by member v)
	PINFLOAT a,b,c;
	PINFLOAT result;
	PINFLOAT hitx1, hitx2;
	PINFLOAT time1, time2;
	PINFLOAT hittime;*/
	PINFLOAT ballvx = pball->vx;
	PINFLOAT ballvy = pball->vy;
	//PINFLOAT ballx;
	//PINFLOAT bally;

	// Trivial normal rejection
	/*if ((ballvx*normal.x + ballvy*normal.y) > 0)
		{
#ifdef SHOWNORMAL
		//check1 = fFalse;
#endif
		return -1;
		}
	else
		{
#ifdef SHOWNORMAL
		//check1 = fTrue;
#endif
		}*/

	if (!m_fEnabled)
		{
		return -1;
		}

	return HitTestRadius(pball, dtime, phitnormal, pball->radius);

	//if (fabs(ballvx) > fabs(ballvy))
		/*{
		ballx = pball->x;
		bally = pball->y;

		// To see if the ball hits the joint, pretend there is a ball around
		// the joint, and see if the center of the real ball passes through
		// that ball.
		// Use these equations for finding intersection of a line and circle

		m = ballvy/ballvx;
		q = bally - m*ballx;

		u = v.x;
		w = v.y;

		a = 1+(m*m);
		b = 2*( (m*q) - (m*w) - u );
		c = (u*u) + (q-w)*(q-w) - (pball->radius*pball->radius);

		result = (b*b) - (4*a*c);

		if (result >= 0)
			{
			hitx1 = (-b + (PINFLOAT)(sqrt(result))) / (2*a);
			hitx2 = (-b - (PINFLOAT)(sqrt(result))) / (2*a);

			time1 = (hitx1-ballx)/(ballvx);
			time2 = (hitx2-ballx)/(ballvx);

			PINFLOAT realhitx;

			if (time1 > time2)
				{
				hittime = time2;
				realhitx = hitx2;
				}
			else
				{
				hittime = time1;
				realhitx = hitx1;
				}

			// check hittime against zero, because it could be a joint way behind the ball
			// that is close to being clipped but not quite

			if ((hittime >= 0) && (hittime <= dtime))
				{

				float hitz = pball->z + pball->vz*(float)hittime + pball->radius;

				if (hitz < m_rcHitRect.zlow || hitz > m_rcHitRect.zhigh)
					{
					return -1;
					}

				phitnormal->x = ((float)realhitx - v.x) / pball->radius;

				PINFLOAT hity = pball->y + pball->vy * hittime;

				phitnormal->y = ((float)hity - v.y)/pball->radius;

				//phitnormal->x = normal.x;
				//phitnormal->y = normal.y;
				return (float)hittime;
				}
			}
		}*/
	/*else
		{

		ballvx = pball->vy;
		ballvy = pball->vx;

		ballx = pball->y;
		bally = pball->x;

		// To see if the ball hits the joint, pretend there is a ball around
		// the joint, and see if the center of the real ball passes through
		// that ball.
		// Use these equations for finding intersection of a line and circle

		m = ballvy/ballvx;
		q = bally - m*ballx;

		u = v.y;
		w = v.x;

		a = 1+(m*m);
		b = 2*( (m*q) - (m*w) - u );
		c = (u*u) + (q-w)*(q-w) - (pball->radius*pball->radius);

		result = (b*b) - (4*a*c);

		if (result >= 0)
			{
			hitx1 = (-b + (float)(sqrt(result))) / (2*a);
			hitx2 = (-b - (float)(sqrt(result))) / (2*a);

			time1 = (hitx1-ballx)/(ballvx);
			time2 = (hitx2-ballx)/(ballvx);

			hittime = min(time1, time2);

			if ((hittime >= 0) && (hittime <= dtime))
				{
				phitnormal->x = normal.x;
				phitnormal->y = normal.y;
				return (float)hittime;
				}
			}
		}*/

	//check1 = 10;
	//check2 = 10;
	return -1;
	}

void Joint::Draw(HDC hdc)
	{
	SelectObject(hdc, GetStockObject(BLACK_BRUSH));
	SelectObject(hdc, GetStockObject(BLACK_PEN));
	DrawCircleAbsolute(hdc, center.x, center.y, 3);
	//Ellipse(hdc, pjoint->v.x, int(pjoint->v.y*zoom)-3, (int)(pjoint->v.x*zoom)+4, (int)(pjoint->v.y*zoom)+4);

	float x,y;
	x = center.x;
	y = center.y;

#ifdef SHOWNORMAL
	/*if (check1)
		{
		SelectObject(hdc, GetStockObject(BLACK_PEN));
		}
	else
		{
		SelectObject(hdc, GetStockObject(WHITE_PEN));
		}

	SelectObject(hdc, GetStockObject(BLACK_PEN));

	DrawLine(hdc, x, y, x+normal.x*20, y+normal.y*20);*/
#endif
	}

void Joint::Collide(Ball *pball, Vertex3D *phitnormal)
	{
	pball->CollideWall(phitnormal, m_elasticity, 0, 0);

	if (m_pfe)
		{
		if ((pball->vx * pball->vx + pball->vy * pball->vy) >=
			(m_threshold*m_threshold))
			{
			m_pfe->FireGroupEvent(DISPID_HitEvents_Hit);
			//m_pep->FireGroupEvent(DISPID_HitEvents_Hit);
			//g_pplayer->m_pin3d.PlaySound();
			}
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

PINFLOAT HitCircle::HitTest(Ball *pball, PINFLOAT dtime, Vertex3D *phitnormal)
	{
	return HitTestRadius(pball, dtime, phitnormal, pball->radius);
	}
	
PINFLOAT HitCircle::HitTestBasicRadius(Ball *pball, PINFLOAT dtime, Vertex3D *phitnormal, bool direction,bool lateral, bool rigid)
{
	if (!m_fEnabled || pball->fFrozen) return -1;	

	PINFLOAT hittime; 
	BOOL fUnhit;

	bool capsule3D = false;
	PINFLOAT targetRadius;

	PINFLOAT x = center.x;
	PINFLOAT y = center.y;	

	PINFLOAT dx = pball->x - x;		// form delta components (i.e. translate coordinate frame)
	PINFLOAT dy = pball->y - y;

	PINFLOAT dvx = pball->vx;		// delta velocity from ball's coordinate frame
	PINFLOAT dvy = pball->vy;

	PINFLOAT z,dz,dvz; z = dz = dvz = 0;

	targetRadius = lateral ? radius + pball->radius : radius;	// 	

	if (!lateral && pball->z > zhigh)
	{
		float hcap =  radius/5;									// cap height to hit-circle radius ratio
		capsule3D = true;										// handle ball over target? 
		dvz = pball->vz;										// differential velocity
		targetRadius = (radius*radius + hcap*hcap)/(hcap*2);	// c = (r^2+h^2)/(2*h)
		z = zhigh - (targetRadius - hcap);						// b = c - h
		dz = pball->z  - z;										//ball rolling point - capsule center height 			
	}

	PINFLOAT bcddsq = dx*dx + dy*dy + dz*dz;		// ball center to circle center distance ... squared

	PINFLOAT bcdd = sqrt(bcddsq);					//distance center to center

	PINFLOAT bnv;

	PINFLOAT b = dx*dvx + dy*dvy + dz*dvz;			//inner product

	if (bcdd > 1.0e-6) bnv = b/bcdd;				//ball normal velocity, 
	else return -1;									// no hit on exact center 	

	if (direction && bnv > C_LOWNORMVEL) return -1;				// clearly receding from radius

	PINFLOAT a = dvx*dvx + dvy*dvy +dvz*dvz;		// square of the delta velocity (outer product)

	PINFLOAT bnd = bcdd - targetRadius;				// ball normal distance to 

	// Kicker is special.. handle ball stalled on kicker, commonly hit while receding, knocking back into kicker pocket
	if (m_ObjType == eItemKicker && bnd <= 0 && bnd >= -radius &&  a < C_CONTACTVEL*C_CONTACTVEL )	
	{
		if (pball->m_vpVolObjs) pball->m_vpVolObjs->RemoveElement(m_pObj);	// cause capture
	}

	if (rigid && bnd < PHYS_TOUCH)				// postive: contact possible in future ... Negative: objects in contact now
	{
		if (bnd < -PHYS_SKIN) {return -1;} 	

		if (fabsf((float)bnv) > C_CONTACTVEL)										// >fast velocity, return zero time
			hittime = 0;													//zero time for rigid fast bodies
		else if(bnd <= -PHYS_TOUCH) hittime = 0;							// slow moving but embedded
		else hittime = (bnd + PHYS_TOUCH) * (1.0f/PHYS_TOUCH/2);			// don't compete for fast zero time events
	}
	else if (m_ObjType >= eTrigger && pball->m_vpVolObjs && (bnd < 0 == pball->m_vpVolObjs->IndexOf(m_pObj) < 0))
	{// here if ... ball inside and no hit set .... or ... ball outside and hit set

		if (fabs(bnd -radius) <0.05f)	 // if ball appears in center of trigger, then assumed it was gen'ed there
		{
			if (pball->m_vpVolObjs) pball->m_vpVolObjs->AddElement(m_pObj);	//special case for trigger overlaying a kicker
		}										// this will add the ball to the trigger space without a Hit
		else
		{
			hittime = 0;
			fUnhit = bnd > 0;	// ball on outside is UnHit, otherwise it's a Hit
		}
	}
	else
	{	
		if(!rigid && bnd * bnv > 0) return -1;	// (outside and receding) or (inside and approaching)

		PINFLOAT c = bcddsq - targetRadius*targetRadius; // contact distance ... square delta distance (outer product)

		if (a < 1.0e-8) return -1;;					//no hit ... ball not moving relative to object

		b *= 2;										// twice the (inner products)

		PINFLOAT result = (b*b)-(4*a*c);			// inner products minus the outer products

		if (result < 0) return -1;					// contact impossible 

		result = sqrt(result); a *= 2;				//optmize sqrt calc, sqrt is expensive cpu time

		PINFLOAT time1 = (-b + result)/ a;
		PINFLOAT time2 = (-b - result)/ a;

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

		if (hittime < 0 || hittime > dtime) return -1;			// contact out of physics frame
	}

	PINFLOAT hitz = pball->z - pball->radius + pball->vz * hittime; //rolling point

	if ((hitz + pball->radius *1.50) < zlow)return -1;

	if (!capsule3D && (hitz + pball->radius/2) > zhigh) return-1;

	if (capsule3D && (pball->z + pball->vz * hittime) < zhigh) return -1;

	PINFLOAT hitx = pball->x + pball->vx*hittime;
	PINFLOAT hity = pball->y + pball->vy*hittime;

	PINFLOAT len = sqrt((hitx - x)*(hitx - x)+(hity - y)*(hity - y));

	if (len > 1.0e-8)											// over center???
	{//no
		phitnormal->x = (float)((hitx - x)/len);
		phitnormal->y = (float)((hity - y)/len);	
	}
	else 
	{//yes over center
		phitnormal->x = 0;	// make up a value, any direction is ok
		phitnormal->y = 1;	
	}

	if (!rigid)											// non rigid body collision? return direction
		phitnormal[1].x = (float)fUnhit;				// UnHit signal	is receding from target

	pball->m_HitDist = bnd;					//actual contact distance ... 
	pball->m_HitNormVel = bnv;
	pball->m_HitRigid = rigid;				// collision type

	return hittime;	
}	

PINFLOAT HitCircle::HitTestRadius(Ball *pball, PINFLOAT dtime, Vertex3D *phitnormal, float ballradius)
	{
	if (!fOldPhys)
	{
		return HitTestBasicRadius(pball, dtime, phitnormal, true, true, true);
	}
	/*if (!m_fEnabled)
		{
		return -1;
		}*/

	//float radius = 0;
	float vx = 0;
	float vy = 0;
	float x = center.x;
	float y = center.y;

	PINFLOAT result;
	PINFLOAT dx,dy;
	//PINFLOAT totalradius = pball->radius + radius;
	PINFLOAT totalradius = radius + ballradius;
	PINFLOAT hittime;

	PINFLOAT a,b,c;

	dx = vx-pball->vx;
	dy = vy-pball->vy;

	a = dx*dx+dy*dy;
	b = dx*(x-pball->x) + dy*(y-pball->y);
	c = (x-pball->x)*(x-pball->x) + (y-pball->y)*(y-pball->y) - totalradius * totalradius;

	b=b*2;

	result = (b*b) - (4*a*c);

	if (result >= 0)
		{
		PINFLOAT time1, time2;
		//BOOL fUnhit;

		time1 = (-b + (PINFLOAT)(sqrt(result))) / (2*a);
		time2 = (-b - (PINFLOAT)(sqrt(result))) / (2*a);

		/*if (time1*time2 < 0) // ball is inside the trigger
			{
			fUnhit = fTrue;

			hittime = max(time1,time2);
			}
		else
			{
			fUnhit = fFalse;
			hittime = min(time1,time2);
			}*/

		if (time1 < time2)
			{
			hittime = time1;
			}
		else
			{
			hittime = time2;
			}

		if (hittime < 0 || hittime > dtime)
			{
			return -1;
			}

		//float hitz = pball->GetZ() - pball->vz*(float)hittime;
		PINFLOAT hitz = pball->z - pball->radius + pball->vz*hittime;

		if ((hitz + (pball->radius*1.5)) < zlow || (hitz + (pball->radius*0.5)) > zhigh)
			{
			return -1;
			}

		PINFLOAT hitx, hity;
		hitx = pball->x-dx*hittime;
		hity = pball->y-dy*hittime;

		phitnormal->x = (float)((hitx - x)/totalradius);
		phitnormal->y = (float)((hity - y)/totalradius);
		phitnormal->z = 0;
		
		//pball->m_HitDist = bnd;					//actual contact distance ... 
		//pball->m_HitNormVel = bnv;
		//pball->m_HitRigid = rigid;				// collision type


		//phitnormal[1].x = (float)fUnhit;

		return hittime;
		}

	return -1;
	/*PINFLOAT m, q;
	PINFLOAT u,w; // w should be v
	PINFLOAT a,b,c;
	PINFLOAT result;
	PINFLOAT hitx1, hitx2;
	PINFLOAT time1, time2;
	PINFLOAT hittime;
	PINFLOAT ballvx = pball->vx;
	PINFLOAT ballvy = pball->vy;
	PINFLOAT ballx;
	PINFLOAT bally;
	PINFLOAT newradius;

	newradius = radius + ballradius;

	if (fabs(pball->vx) > fabs(pball->vy))
		{
		ballx = pball->x;
		bally = pball->y;

		// To see if the ball hits the joint, pretend there is a ball around
		// the joint, and see if the center of the real ball passes through
		// that ball.
		// Use these equations for finding intersection of a line and circle

		m = ballvy/ballvx;
		q = bally - m*ballx;

		u = center.x;
		w = center.y;

		a = 1+(m*m);
		b = 2*( (m*q) - (m*w) - u );
		c = (u*u) + (q-w)*(q-w) - (newradius*newradius);

		result = (b*b) - (4*a*c);

		if (result >= 0)
			{
			PINFLOAT vx,vy;

			hitx1 = (-b + (PINFLOAT)(sqrt(result))) / (2*a);
			hitx2 = (-b - (PINFLOAT)(sqrt(result))) / (2*a);

			time1 = (hitx1-ballx)/(ballvx);
			time2 = (hitx2-ballx)/(ballvx);

			if (time1 < time2)
				{
				hittime = time1;
				vx = hitx1 - center.x;
				vy = m*hitx1 + q;
				}
			else
				{
				hittime = time2;
				vx = hitx2 - center.x;
				vy = m*hitx2 + q;
				}
			//hittime = min(time1, time2);

			if ((hittime > 0) && (hittime <= dtime))
				{

				float hitz = pball->GetZ() + pball->vz*(float)hittime;

				if ((hitz + (pball->radius*1.5)) < zlow || (hitz + (pball->radius*0.5)) > zhigh)
					{
					return -1;
					}

				vy = vy - center.y;
				phitnormal->x = (float)(vx/newradius);
				phitnormal->y = (float)(vy/newradius);
				//phitnormal->x = normal.x;
				//phitnormal->y = normal.y;
				return (float)hittime;
				}
			}
		}
	else
		{
		ballvx = pball->vy;
		ballvy = pball->vx;

		ballx = pball->y;
		bally = pball->x;

		// To see if the ball hits the joint, pretend there is a ball around
		// the joint, and see if the center of the real ball passes through
		// that ball.
		// Use these equations for finding intersection of a line and circle

		m = ballvy/ballvx;
		q = bally - m*ballx;

		u = center.y;
		w = center.x;

		a = 1+(m*m);
		b = 2*( (m*q) - (m*w) - u );
		c = (u*u) + (q-w)*(q-w) - (newradius*newradius);

		result = (b*b) - (4*a*c);

		if (result >= 0)
			{
			PINFLOAT vx,vy;

			hitx1 = (-b + (float)(sqrt(result))) / (2*a);
			hitx2 = (-b - (float)(sqrt(result))) / (2*a);

			time1 = (hitx1-ballx)/(ballvx);
			time2 = (hitx2-ballx)/(ballvx);

			if (time1 < time2)
				{
				hittime = time1;
				vx = hitx1 - center.y;
				vy = m*hitx1 + q;
				}
			else
				{
				hittime = time2;
				vx = hitx2 - center.y;
				vy = m*hitx2 + q;
				}
			//hittime = min(time1, time2);

			if ((hittime > 0) && (hittime <= dtime))
				{

				float hitz = pball->GetZ() + pball->vz*(float)hittime;

				if ((hitz + (pball->radius*1.5)) < zlow || (hitz + (pball->radius*0.5)) > zhigh)
					{
					return -1;
					}

				vy = vy - center.x;
				phitnormal->y = (float)(vx/newradius);
				phitnormal->x = (float)(vy/newradius);
				//phitnormal->x = normal.x;
				//phitnormal->y = normal.y;
				return (float)hittime;
				}
			}
		}

	//check1 = 10;
	//check2 = 10;
	return -1;*/
	}

void HitCircle::Draw(HDC hdc)
	{
	SelectObject(hdc, GetStockObject(NULL_BRUSH));
	SelectObject(hdc, GetStockObject(BLACK_PEN));
	DrawCircle(hdc, center.x, center.y, radius);
	}

void HitCircle::Collide(Ball *pball, Vertex3D *phitnormal)
	{
	pball->CollideWall(phitnormal, m_elasticity, 0, 0);
	}

HitOctree::~HitOctree()
	{
	int i;

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
		for (i=0;i<8;i++)
			{
			delete m_phitoct[i];
			}
		}
	}

void HitOctree::CreateNextLevel()
	{
	int i;

	m_fLeaf = fFalse;

	Vector<HitObject> vRemain; // hit objects which did not go to an octant

	for (i=0;i<8;i++)
		{
		m_phitoct[i] = new HitOctree();

		m_phitoct[i]->m_rectbounds.left = (i&1) ? m_vcenter.x : m_rectbounds.left;
		m_phitoct[i]->m_rectbounds.top = (i&2) ?  m_vcenter.y : m_rectbounds.top;
		m_phitoct[i]->m_rectbounds.zlow = (i&4) ? m_vcenter.z : m_rectbounds.zlow;

		m_phitoct[i]->m_rectbounds.right = (i&1) ?  m_rectbounds.right : m_vcenter.x;
		m_phitoct[i]->m_rectbounds.bottom = (i&2) ? m_rectbounds.bottom : m_vcenter.y;
		m_phitoct[i]->m_rectbounds.zhigh = (i&4) ? m_rectbounds.zhigh : m_vcenter.z;

		m_phitoct[i]->m_vcenter.x = (m_phitoct[i]->m_rectbounds.left + m_phitoct[i]->m_rectbounds.right)/2;
		m_phitoct[i]->m_vcenter.y = (m_phitoct[i]->m_rectbounds.top + m_phitoct[i]->m_rectbounds.bottom)/2;
		m_phitoct[i]->m_vcenter.z = (m_phitoct[i]->m_rectbounds.zlow + m_phitoct[i]->m_rectbounds.zhigh)/2;

		m_phitoct[i]->m_fLeaf = fTrue;
		}

	int ccross = 0;
	int ccrossx = 0, ccrossy = 0, ccrossz = 0;

	for (i=0;i<m_vho.Size();i++)
		{
		int oct = 0;
		HitObject *pho = m_vho.ElementAt(i);

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
			oct |= 0;
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

		/*if (pho->m_rcHitRect.zhigh < m_vcenter.z)
			{
			oct |= 0;
			}
		else if (pho->m_rcHitRect.zlow > m_vcenter.z)
			{
			oct |= 4;
			}
		else
			{
			oct |= 128;
			ccrossz++;
			}*/

		if (!(oct & 128))
			{
			m_phitoct[oct]->m_vho.AddElement(pho);
			}
		else
			{
			vRemain.AddElement(pho);
			ccross++;
			}

		/*if (pho->m_rcHitRect.left <= m_vcenter.x)
			{
			if (pho->m_rcHitRect.top <= m_vcenter.y)
				{
				if (pho->m_rcHitRect.zlow <= m_vcenter.z)
					{
					m_phitoct[0]->m_vho.AddElement(pho);
					}
				if (pho->m_rcHitRect.zhigh >= m_vcenter.z)
					{
					m_phitoct[4]->m_vho.AddElement(pho);
					}
				}
			if (pho->m_rcHitRect.bottom >= m_vcenter.y)
				{
				if (pho->m_rcHitRect.zlow <= m_vcenter.z)
					{
					m_phitoct[2]->m_vho.AddElement(pho);
					}
				if (pho->m_rcHitRect.zhigh >= m_vcenter.z)
					{
					m_phitoct[6]->m_vho.AddElement(pho);
					}
				}
			}
		if (pho->m_rcHitRect.right >= m_vcenter.x)
			{
			if (pho->m_rcHitRect.top <= m_vcenter.y)
				{
				if (pho->m_rcHitRect.zlow <= m_vcenter.z)
					{
					m_phitoct[1]->m_vho.AddElement(pho);
					}
				if (pho->m_rcHitRect.zhigh >= m_vcenter.z)
					{
					m_phitoct[5]->m_vho.AddElement(pho);
					}
				}
			if (pho->m_rcHitRect.bottom >= m_vcenter.y)
				{
				if (pho->m_rcHitRect.zlow <= m_vcenter.z)
					{
					m_phitoct[3]->m_vho.AddElement(pho);
					}
				if (pho->m_rcHitRect.zhigh >= m_vcenter.z)
					{
					m_phitoct[7]->m_vho.AddElement(pho);
					}
				}
			}*/
		}

	/*for (i=0;i<8;i++)
		{
		int l;
		for (l=0;l<m_phitoct[i]->m_vho.Size();l++)
			{
			m_vho.RemoveElement(m_phitoct[i]->m_vho.ElementAt(l));
			}
		}*/

	m_vho.RemoveAllElements();
	for (i=0;i<vRemain.Size();i++)
		{
		m_vho.AddElement(vRemain.ElementAt(i));
		}

	if ((m_vcenter.x - m_rectbounds.left) > 125)
		{
		for (i=0;i<8;i++)
			{
			m_phitoct[i]->CreateNextLevel();
			}
		}
	}

#ifdef LOG
extern int cTested;
extern int cDeepTested;
#endif

void HitOctree::HitTestBall(Ball *pball)
	{
	int i;

	PINFLOAT newtime;

	for (i=0; i<m_vho.Size(); i++)
		{
#ifdef LOG
		cTested++;
#endif
		if ((pball != m_vho.ElementAt(i)) && (pball->phoHitLast != m_vho.ElementAt(i)) && fRectIntersect3D(pball->m_rcHitRect, m_vho.ElementAt(i)->m_rcHitRect))
			{
#ifdef LOG
		cDeepTested++;
#endif
			newtime = m_vho.ElementAt(i)->HitTest(pball, pball->m_hittime, pball->m_hitnormal);
			if (newtime != -1)
				{
				//pho = m_vho.ElementAt(i);
				//hittime = newtime;
				//phitball = pball;
				pball->m_pho = m_vho.ElementAt(i);
				pball->m_hittime = newtime;
				pball->m_hitx = (pball->x + pball->vx*newtime);
				pball->m_hity = (pball->y + pball->vy*newtime);
				}
			}
		}

	if (!m_fLeaf)
		{
		BOOL fLeft, fRight, fTop, fBottom;

		fLeft = (pball->m_rcHitRect.left <= m_vcenter.x);
		fRight = (pball->m_rcHitRect.right >= m_vcenter.x);
		fTop = (pball->m_rcHitRect.top <= m_vcenter.y);
		fBottom = (pball->m_rcHitRect.bottom >= m_vcenter.y);

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
	int i;

	PINFLOAT newtime;

	for (i=0; i<m_vho.Size(); i++)
		{
#ifdef LOG
		cTested++;
#endif
		if ((pball != m_vho.ElementAt(i)) && fRectIntersect3D(pball->m_rcHitRect, m_vho.ElementAt(i)->m_rcHitRect))
			{
#ifdef LOG
		cDeepTested++;
#endif
			newtime = m_vho.ElementAt(i)->HitTest(pball, pball->m_hittime, pball->m_hitnormal);
			if (newtime != -1)
				{
				pvhoHit->AddElement(m_vho.ElementAt(i));
				}
			}
		}

	if (!m_fLeaf)
		{
		BOOL fLeft, fRight, fTop, fBottom;

		fLeft = (pball->m_rcHitRect.left <= m_vcenter.x);
		fRight = (pball->m_rcHitRect.right >= m_vcenter.x);
		fTop = (pball->m_rcHitRect.top <= m_vcenter.y);
		fBottom = (pball->m_rcHitRect.bottom >= m_vcenter.y);

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