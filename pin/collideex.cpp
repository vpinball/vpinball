#include "stdafx.h"
#include "..\main.h"

BumperHitCircle::BumperHitCircle()
	{
	m_bumperanim.m_iframe = -1;
	m_bumperanim.m_TimeReset = 0;
	m_bumperanim.m_iframedesired = 0;
	m_bumperanim.m_fAutoTurnedOff = fFalse;
	}

BumperHitCircle::~BumperHitCircle()
	{
	int i;

	for (i=0;i<2;i++)
		{
		delete m_bumperanim.m_pobjframe[i];
		}
	}

void BumperHitCircle::Collide(Ball *pball, Vertex3D *phitnormal)
	{
	pball->CollideWall(phitnormal, m_elasticity, 0, 0);

	// if the bumper is disabled then don't activate the bumper
	if (m_pbumper->m_fDisabled == fTrue)
	{
		return;
	}

	if ((pball->vx * pball->vx + pball->vy * pball->vy) >=
		(m_pbumper->m_d.m_threshold*m_pbumper->m_d.m_threshold))
		{
		pball->vx += phitnormal[0].x * m_pbumper->m_d.m_force;
		pball->vy += phitnormal[0].y * m_pbumper->m_d.m_force;

		//m_pbumper->FireVoidEvent(DISPID_HitEvents_Hit);
		m_pbumper->FireGroupEvent(DISPID_HitEvents_Hit);

		if (m_pbumper->m_d.m_fFlashWhenHit)
			{
			m_bumperanim.m_TimeReset = g_pplayer->m_timeCur + 100;
			m_bumperanim.m_fAutoTurnedOff = fTrue;
			}
		}
	}

void BumperAnimObject::Check3D()
	{
	if (m_iframe == 0 && m_TimeReset != 0)
		{
		m_iframe = 1;
		m_fInvalid = fTrue;
		}
	else if (m_fAutoTurnedOff && m_iframe == 1 && (m_TimeReset < g_pplayer->m_timeCur) && (m_iframe != m_iframedesired))
		{
		m_iframe = m_iframedesired;
		m_fInvalid = fTrue;
		m_TimeReset = 0;
		m_fAutoTurnedOff = fFalse;
		}
	else if (m_iframe == -1)
		{
		m_iframe = 0;
		m_fInvalid = fTrue;
		}
	else if (!m_fAutoTurnedOff && (m_iframe != m_iframedesired))
		{
		m_iframe = m_iframedesired;
		m_fInvalid = fTrue;
		}

	/*if (m_fInvalid)
		{
		ObjFrame *pobjframe = m_pobjframe;

		g_pplayer->m_pin3d.m_pddsBackBuffer->BltFast(pobjframe->rc.left, pobjframe->rc.top, g_pplayer->m_pin3d.m_pddsStatic,
			&pobjframe->rc, DDBLTFAST_NOCOLORKEY);
		g_pplayer->m_pin3d.m_pddsZBuffer->BltFast(pobjframe->rc.left, pobjframe->rc.top,
			g_pplayer->m_pin3d.m_pddsStaticZ, &pobjframe->rc, DDBLTFAST_NOCOLORKEY);
		}*/
	}

ObjFrame *BumperAnimObject::Draw3D(RECT *prc)
	{
	ObjFrame *pobjframe = m_pobjframe[m_iframe];

	/*g_pplayer->m_pin3d.m_pddsBackBuffer->BltFast(pobjframe->rc.left, pobjframe->rc.top, pobjframe->pdds,
		NULL, DDBLTFAST_SRCCOLORKEY);
	g_pplayer->m_pin3d.m_pddsZBuffer->BltFast(pobjframe->rc.left, pobjframe->rc.top,
		pobjframe->pddsZBuffer, NULL, DDBLTFAST_NOCOLORKEY);*/

	return pobjframe;
	}

void BumperAnimObject::Reset()
	{
	m_iframe = -2;
	}

LineSegSlingshot::LineSegSlingshot()
	{
	m_slingshotanim.m_iframe = 0;
	m_slingshotanim.m_TimeReset = 0; // zero means the slingshot is currently reset
	}

LineSegSlingshot::~LineSegSlingshot()
	{
	delete m_slingshotanim.m_pobjframe;
	}

PINFLOAT LineSegSlingshot::HitTest(Ball *pball, PINFLOAT dtime, Vertex3D *phitnormal)
	{
	if (!fOldPhys)
	{
		if (!m_fEnabled) return -1;	
		{
			return this->HitTestBasic(pball, dtime, phitnormal, true, true, true);	
		}
	}
	
	if (!m_fEnabled) return -1;	

	PINFLOAT hittime = LineSeg::HitTest(pball, dtime, phitnormal);

	if (hittime != -1)
		{
		float len;
		float balldis;
		float force;
		float dx,dy;
		dx = v1.x - v2.x;
		dy = v1.y - v2.y;

		len = (float)sqrt(dx*dx + dy*dy);

		Vertex vhitpoint;

		vhitpoint.x = (float)(pball->x + pball->vx * hittime);
		vhitpoint.y = (float)(pball->y + pball->vy * hittime);
		vhitpoint.x -= normal.x * pball->radius;
		vhitpoint.y -= normal.y * pball->radius;

		// vhitpoint will now be the point where the ball hits the line
		// Calculate this distance from the center of the slingshot to get force
		dx = ((v1.x + v2.x) / 2) - vhitpoint.x;
		dy = ((v1.y + v2.y) / 2) - vhitpoint.y;

		balldis = (float)sqrt(dx*dx + dy*dy);

		force = ((len/2) - balldis) / len;

		force *= -m_force;//-80;

		phitnormal[1].x = normal.x * force;
		phitnormal[1].y = normal.y * force;
		}

	return hittime;
	}

void LineSegSlingshot::Collide(Ball *pball, Vertex3D *phitnormal)
	{
	if (fOldPhys)
		{
		if (!m_psurface->m_fDisabled)
			{
			pball->vx += phitnormal[1].x;
			pball->vy += phitnormal[1].y;
			}

		pball->CollideWall(phitnormal, m_elasticity, 0, 0);

		if (m_pfe && !m_psurface->m_fDisabled)
			{
			if ((pball->vx * pball->vx + pball->vy * pball->vy) >=
				(m_threshold*m_threshold))
				{
				m_pfe->FireGroupEvent(DISPID_SurfaceEvents_Slingshot);
				//m_pep->FireGroupEvent(DISPID_SurfaceEvents_Slingshot);
				m_slingshotanim.m_TimeReset = g_pplayer->m_timeCur + 100;
				}
			}
		}
	else // newphys
		{
			float dot = (float)(pball->vx*phitnormal->x  + pball->vy*phitnormal->y); // normal velocity to slingshot

			bool threshold = dot <= m_threshold;//-m_psurface->m_d.m_slingshot_threshold;  // normal greater than threshold?

			if (!m_psurface->m_fDisabled && threshold) // enabled and if velocity greater than threshold level		
			{

		#define TANX  phitnormal->y
		#define TANY -phitnormal->x

				float len = (v2.x - v1.x)*TANX + (v2.y - v1.y)*TANY; //rlc length of segment, Unit TAN points from V1 to V2

				Vertex vhitpoint;

				vhitpoint.x = (float)pball->x - phitnormal->x * pball->radius; //project ball radius along norm
				vhitpoint.y = (float)pball->y - phitnormal->y * pball->radius;

				// vhitpoint will now be the point where the ball hits the line
				// Calculate this distance from the center of the slingshot to get force

				float btd = (vhitpoint.x - v1.x)*TANX + (vhitpoint.y - v1.y)*TANY; //rlc distance to vhit from V1
				float force = 2*btd/len - (float)1;		// -1..+1
				force = (float)0.5 *(1-force*force);	//rlc maximum value 0.5 ...I think this should have been 1.0...oh well
				// will match the previous physics
				force *= m_force;//-80;

				pball->vx -= phitnormal->x * force;	// boost velocity, drive into slingshot (counter normal)
				pball->vy -= phitnormal->y * force;	// allow CollideWall to handle the remainder
			}

			pball->CollideWall(phitnormal, m_elasticity, 0, 0);

			if (m_pfe && !m_psurface->m_fDisabled)
			{			
				if (dot <= -m_threshold)
				{
					float dx = pball->m_Event_Pos.x - (float)pball->x; // is this the same place as last event????
					float dy = pball->m_Event_Pos.y - (float)pball->y; // if same then ignore it
					float dz = pball->m_Event_Pos.z - (float)pball->z;

					if ((dx*dx + dy*dy + dz*dz) > (float)0.25)// must be a new place if only by a little
					{
						if (m_threshold/*m_psurface->m_d.m_slingshot_threshold*/ != 0.0f)// if new slingshot threshold is set use it
						{			
							if (threshold)
							{
								m_pfe->FireGroupEvent(DISPID_SurfaceEvents_Slingshot);
								m_slingshotanim.m_TimeReset = g_pplayer->m_timeCur + 100;
							}
						}	
						else if (dot <= -m_threshold)//legacy wall threshold
						{
							m_pfe->FireGroupEvent(DISPID_SurfaceEvents_Slingshot);
							m_slingshotanim.m_TimeReset = g_pplayer->m_timeCur + 100;
						}
					}
				}
				pball->m_Event_Pos.x = (float)pball->x; 
				pball->m_Event_Pos.y = (float)pball->y; 
				pball->m_Event_Pos.z = (float)pball->z; //remember last collide position
			}
		}
	}

PINFLOAT LineSegLevelEdge::HitTest(Ball *pball, PINFLOAT dtime, Vertex3D *phitnormal)
	{
	if (!fOldPhys)
	{
		if (!m_fEnabled) return -1;	

		// approach from either face, lateral contact, not rigid
		return this->HitTestBasic(pball, dtime, phitnormal, false, true, false);
	}
	
	PINFLOAT endx, endy;

	PINFLOAT hitx, hity;

	PINFLOAT x1,y1,x2,y2;

	PINFLOAT ballvx = pball->vx;
	PINFLOAT ballvy = pball->vy;
	PINFLOAT ballx;
	PINFLOAT bally;

	x1 = v1.x;
	y1 = v1.y;
	x2 = v2.x;
	y2 = v2.y;

	ballx = pball->x;
	bally = pball->y;

	endx = ballx + ballvx*dtime;
	endy = bally + ballvy*dtime;

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

	phitnormal->x = normal.x;
	phitnormal->y = normal.y;

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

	// Find out which side the ball hit from - this tells us which level to switch to

	if ((ballvx*normal.x + ballvy*normal.y) > 0)
		{
		phitnormal[1].x = 0;
		}
	else
		{
		phitnormal[1].x = 1;
		}

	return hittime;
	}

void LineSegLevelEdge::Collide(Ball *pball, Vertex3D *phitnormal)
	{
	/*if (phitnormal[1].x == 0)
		{
		pball->m_plevel = m_plevel1;
		}
	else
		{
		pball->m_plevel = m_plevel2;
		}*/
	}

void SlingshotAnimObject::Check3D()
	{
	if (m_iframe == 0 && m_TimeReset != 0)
		{
		m_iframe = 1;
		m_fInvalid = fTrue;
		}
	else if (m_iframe == 1 && (m_TimeReset < g_pplayer->m_timeCur))
		{
		m_iframe = 0;
		m_fInvalid = fTrue;
		m_TimeReset = 0;
		}

	/*if (m_fInvalid)
		{
		ObjFrame *pobjframe = m_pobjframe;

		g_pplayer->m_pin3d.m_pddsBackBuffer->BltFast(pobjframe->rc.left, pobjframe->rc.top, g_pplayer->m_pin3d.m_pddsStatic,
			&pobjframe->rc, DDBLTFAST_NOCOLORKEY);
		g_pplayer->m_pin3d.m_pddsZBuffer->BltFast(pobjframe->rc.left, pobjframe->rc.top,
			g_pplayer->m_pin3d.m_pddsStaticZ, &pobjframe->rc, DDBLTFAST_NOCOLORKEY);
		}*/
	}

ObjFrame *SlingshotAnimObject::Draw3D(RECT *prc)
	{
	if (m_iframe == 1)
		{
		ObjFrame *pobjframe = m_pobjframe;

		/*g_pplayer->m_pin3d.m_pddsBackBuffer->BltFast(pobjframe->rc.left, pobjframe->rc.top, pobjframe->pdds,
			NULL, DDBLTFAST_SRCCOLORKEY);
		g_pplayer->m_pin3d.m_pddsZBuffer->BltFast(pobjframe->rc.left, pobjframe->rc.top,
			pobjframe->pddsZBuffer, NULL, DDBLTFAST_NOCOLORKEY);*/

		return pobjframe;
		}

	return NULL;
	}

void SlingshotAnimObject::Reset()
	{
	// Do nothing right now - just let it draw as not hit
	}

HitGate::HitGate(Gate *pgate)
	{
	m_pgate = pgate;

	m_gateanim.m_iframe = -1;
	m_gateanim.m_angle = 0;
	m_gateanim.m_anglespeed = 0;

	m_gateanim.m_fOpen = fFalse;
	}

PINFLOAT HitGate::HitTest(Ball *pball, PINFLOAT dtime, Vertex3D *phitnormal)
	{
	if (!fOldPhys)
	{
		if (!m_fEnabled) return -1;		
		return HitTestBasic(pball, dtime, phitnormal, true, true, false);// normal face, lateral, non-rigid
	}

	// BUG!  Merge with LineSeg::HitTest!  Need to figure out fudge-factor

	PINFLOAT endx, endy;

	PINFLOAT hitx, hity;

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
	//PINFLOAT startx = hitx - (ballvx/len);
	//PINFLOAT starty = hity - (ballvy/len);

	if (!FQuickLineIntersect(hitx, hity, hitx+(ballvx*dtime), hity+(ballvy*dtime),
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

	return hittime;
	}

void HitGate::Collide(Ball *pball, Vertex3D *phitnormal)
	{
	if (fOldPhys)
		{
		float speed;

		PINFLOAT dot = pball->vx * phitnormal->x + pball->vy * phitnormal->y;

		Vertex CrossVec;

		CrossVec.x = (float)(dot * phitnormal->x);
		CrossVec.y = (float)(dot * phitnormal->y);

		speed = (float)sqrt((CrossVec.x * CrossVec.x) + (CrossVec.y * CrossVec.y));

		//linear speed = ball speed
		//angular speed = linear/radius (height of hit)

		// 50 is the height of the gate axis.
		m_gateanim.m_anglespeed = speed / (50 - pball->radius);

		if (m_pfe)
			{
			m_pfe->FireGroupEvent(DISPID_HitEvents_Hit);
			//m_pep->FireGroupEvent(DISPID_HitEvents_Hit);
			}
		}
	else // new phys
		{
			PINFLOAT dot = pball->vx * phitnormal->x + pball->vy * phitnormal->y;
			if (dot > 0) return;								//hit from back doesn't count

			float h = 50;//m_pgate->m_d.m_height;

			// h is the height of the gate axis.
			if (fabsf(h - pball->radius) > 1)					// avoid divide by zero
				m_gateanim.m_anglespeed = fabsf((float)dot)/(h - pball->radius);
			else 
				m_gateanim.m_anglespeed = fabsf((float)dot);			// use this until a better value comes along

			if (m_pfe)
			{			
				float dx = pball->m_Event_Pos.x - (float)pball->x;		// is this the same place as last event????
				float dy = pball->m_Event_Pos.y - (float)pball->y;		// if same then ignore it
				float dz = pball->m_Event_Pos.z - (float)pball->z;

				pball->m_Event_Pos.x = (float)pball->x; 
				pball->m_Event_Pos.y = (float)pball->y; 
				pball->m_Event_Pos.z = (float)pball->z;				//remember last collide position

				if ((dx*dx + dy*dy + dz*dz) > 0.25f)			// must be a new place if only by a little
				{
					m_pfe->FireGroupEvent(DISPID_HitEvents_Hit);
				}		
			}
		}
	}

void GateAnimObject::UpdateTimeTemp(PINFLOAT dtime)
	{
	/*float target;

	if (m_anglespeed > 0)
		{
		if (m_angle < PI)
			{
			target = PI;
			}
		else
			{
			target = 3*PI;
			}
		}
	else
		{
		if (m_angle < PI)
			{
			target = -PI;
			}
		else
			{
			target = PI;
			}
		}*/

	m_angle += m_anglespeed * (float)dtime;

	if (m_angle > (PI/2))
		{
		m_angle = (PI/2);
		m_anglespeed = 0;
		}
	if (m_angle < 0)
		{
		m_angle = 0;
		m_anglespeed = 0;
		}
	}

void GateAnimObject::UpdateAcceleration(PINFLOAT dtime)
	{
	if (!m_fOpen)
		{
		//m_anglespeed -= (float)(sin(m_angle)*0.005); // Center of gravity towards bottom of object, makes it stop vertical
		m_anglespeed -= (float)(sin(m_angle)*0.01); // Center of gravity towards bottom of object, makes it stop vertical
		}
	}

void GateAnimObject::Check3D()
	{
	int frame = (int)(((m_angle / (PI/2)) * m_vddsFrame.Size()) + 0.5);

	if (frame == m_vddsFrame.Size()) // Happens when the angle exactly equals PI/2
		{
		frame = m_vddsFrame.Size()-1;
		}

	if (frame != m_iframe)
		{
		LPDIRECTDRAWSURFACE7 pdds;

		pdds = g_pplayer->m_pin3d.m_pddsBackBuffer;

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

ObjFrame *GateAnimObject::Draw3D(RECT *prc)
	{
	LPDIRECTDRAWSURFACE7 pdds;

	pdds = g_pplayer->m_pin3d.m_pddsBackBuffer;

	ObjFrame *pobjframe = m_vddsFrame.ElementAt(m_iframe);

	/*HRESULT hr = pdds->BltFast(pobjframe->rc.left, pobjframe->rc.top, pobjframe->pdds,
		NULL, DDBLTFAST_SRCCOLORKEY);
	g_pplayer->m_pin3d.m_pddsZBuffer->BltFast(pobjframe->rc.left, pobjframe->rc.top,
		pobjframe->pddsZBuffer, NULL, DDBLTFAST_NOCOLORKEY);*/

	return pobjframe;
	}

void GateAnimObject::Reset()
	{
	m_iframe = -1;
	}

HitSpinner::HitSpinner(Spinner *pspinner, float height)
	{
	m_spinneranim.m_pspinner = pspinner;

	float halflength = pspinner->m_d.m_length * 0.5f;

	//Vertex rgv[2];

	float radangle = pspinner->m_d.m_rotation / 360 * PI * 2;
	float sn = (float)sin(radangle);
	float cs = (float)cos(radangle);

	/*rgv[0].x = m_d.m_vCenter.x + cs*halflength;
	rgv[0].y = m_d.m_vCenter.y + sn*halflength;

	rgv[1].x = m_d.m_vCenter.x - cs*halflength;
	rgv[1].y = m_d.m_vCenter.y - sn*halflength;*/

	m_lineseg[0].m_rcHitRect.zlow = height;
	m_lineseg[0].m_rcHitRect.zhigh = height + 50;
	m_lineseg[1].m_rcHitRect.zlow = height;
	m_lineseg[1].m_rcHitRect.zhigh = height + 50;

	m_lineseg[0].m_pfe = NULL;
	m_lineseg[1].m_pfe = NULL;

	if (fOldPhys)
		{
		m_lineseg[0].v2.x = pspinner->m_d.m_vCenter.x + cs*halflength;
		m_lineseg[0].v2.y = pspinner->m_d.m_vCenter.y + sn*halflength;
		m_lineseg[0].v1.x = pspinner->m_d.m_vCenter.x - cs*halflength;
		m_lineseg[0].v1.y = pspinner->m_d.m_vCenter.y - sn*halflength;
		}
	else
		{
		m_lineseg[0].v2.x = pspinner->m_d.m_vCenter.x + cs*(halflength + PHYS_SKIN); //oversize  by the ball radius
		m_lineseg[0].v2.y = pspinner->m_d.m_vCenter.y + sn*(halflength + PHYS_SKIN); //this will prevent clipping
		m_lineseg[0].v1.x = pspinner->m_d.m_vCenter.x - cs*(halflength + PHYS_SKIN); // throught the edge of the
		m_lineseg[0].v1.y = pspinner->m_d.m_vCenter.y - sn*(halflength + PHYS_SKIN); //spinner
		}
		
	m_lineseg[1].v1.x = m_lineseg[0].v2.x;
	m_lineseg[1].v1.y = m_lineseg[0].v2.y;
	m_lineseg[1].v2.x = m_lineseg[0].v1.x;
	m_lineseg[1].v2.y = m_lineseg[0].v1.y;

	m_lineseg[0].CalcNormal();
	m_lineseg[1].CalcNormal();
	
	m_spinneranim.m_angleMax = ANGTORAD(pspinner->m_d.m_angleMax);
	m_spinneranim.m_angleMin = ANGTORAD(pspinner->m_d.m_angleMin);

	m_spinneranim.m_iframe = -1;
	m_spinneranim.m_angle = 0;
	m_spinneranim.m_anglespeed = 0;
	
	m_spinneranim.m_elasticity = pspinner->m_d.m_elasticity;
	m_spinneranim.m_friction = 1.0f - pspinner->m_d.m_friction;	//antifriction
	m_spinneranim.m_scatter = pspinner->m_d.m_scatter;
	m_spinneranim.m_fVisible = pspinner->m_d.m_fVisible;	
	}

PINFLOAT HitSpinner::HitTest(Ball *pball, PINFLOAT dtime, Vertex3D *phitnormal)
	{
	if (fOldPhys)
		{
		PINFLOAT newtime;
		//float hittime = dtime;
		Vertex3D normalT;

		newtime = m_lineseg[0].HitTest(pball, dtime, &normalT);
		if (newtime != -1)
			{
			//hittime = newtime;
			*phitnormal = normalT;

			phitnormal[1].x = 1;
			phitnormal[1].y = 0;

			return newtime;
			}

		newtime = m_lineseg[1].HitTest(pball, dtime, phitnormal);
		if (newtime != -1)
			{
			phitnormal[1].x = 0;
			}

		return newtime;
		}
	else // newphys
		{
			PINFLOAT hittime;

			if (!m_fEnabled) return -1;	

			hittime = m_lineseg[0].HitTestBasic(pball, dtime, phitnormal, false, true, false);// any face, lateral, non-rigid
			if (hittime >= 0)
			{
				phitnormal[1].x = 1;
				phitnormal[1].y = 0;

				return hittime;
			}

			hittime = m_lineseg[1].HitTestBasic(pball, dtime, phitnormal, false, true, false);// any face, lateral, non-rigid
			if (hittime >= 0)
			{
				phitnormal[1].x = 0;
			}

			return hittime;
		}
	}

void HitSpinner::Collide(Ball *pball, Vertex3D *phitnormal)
	{
	if (fOldPhys)
		{
		float speed;

		PINFLOAT dot = pball->vx * phitnormal->x + pball->vy * phitnormal->y;

		Vertex CrossVec;

		CrossVec.x = (float)(dot * phitnormal->x);
		CrossVec.y = (float)(dot * phitnormal->y);

		speed = (float)sqrt((CrossVec.x * CrossVec.x) + (CrossVec.y * CrossVec.y));

		//linear speed = ball speed
		//angular speed = linear/radius (height of hit)

		// 60 is the height of the spinner axis;
		// Since the spinner has no mass in our equation, the spot
		// 60-pball->radius will be moving a at linear rate of
		// 'speed'.  We can calculate the angular speed from that.
		m_spinneranim.m_anglespeed = speed / (60 - pball->radius);

		// We encoded which side of the spinner the ball hit
		if (phitnormal[1].x < 0.5)
			{
			m_spinneranim.m_anglespeed *= -1;
			}
		}
	else // new phys
		{
		PINFLOAT dot = pball->vx * phitnormal->x + pball->vy * phitnormal->y;
		if (dot < 0) return;	//hit from back doesn't count

		PINFLOAT h = m_spinneranim.m_pspinner->m_d.m_height/2  + 30.0f;

		//linear speed = ball speed
		//angular speed = linear/radius (height of hit)

		// h is the height of the spinner axis;
		// Since the spinner has no mass in our equation, the spot
		// h -pball->radius will be moving a at linear rate of
		// 'speed'.  We can calculate the angular speed from that.

		if (fabsf((float)h - pball->radius) > 1)				// avoid divide by zero
			m_spinneranim.m_anglespeed = fabsf((float)dot)/((float)h - pball->radius);
		else m_spinneranim.m_anglespeed = fabsf((float)dot);		// use this until a better value comes along

		// We encoded which side of the spinner the ball hit
		if (phitnormal[1].x > 0)
			{
			m_spinneranim.m_anglespeed = -m_spinneranim.m_anglespeed; 
			}

		}
	}

void SpinnerAnimObject::UpdateTimeTemp(PINFLOAT dtime)
	{
	if (fOldPhys)
		{
		float target;

		if (m_anglespeed > 0)
			{
			if (m_angle < PI)
				{
				target = PI;
				}
			else
				{
				target = 3*PI;
				}
			}
		else
			{
			if (m_angle < PI)
				{
				target = -PI;
				}
			else
				{
				target = PI;
				}
			}

		m_angle += m_anglespeed * (float)dtime;

		if (m_anglespeed > 0)
			{
			if (m_angle > target)
				{
				m_pspinner->FireGroupEvent(DISPID_SpinnerEvents_Spin);
				}
			}
		else
			{
			if (m_angle < target)
				{
				m_pspinner->FireGroupEvent(DISPID_SpinnerEvents_Spin);
				}
			}

		if (m_angle > (2*PI))
			{
			m_angle -= (2*PI);
			}
		if (m_angle < 0)
			{
			m_angle += 2*PI;
			}
		}
	else // new phys
		{
		/*if (m_pspinner->m_d.m_angleMin != m_pspinner->m_d.m_angleMax)	//blocked spinner, limited motion spinner
			{
				m_angle += m_anglespeed * dtime;

				if (m_angle > m_angleMax)
				{
					m_angle = m_angleMax;
					m_pspinner->FireVoidEventParm(DISPID_LimitEvents_EOS, fabs(m_anglespeed*180/PI));	// send EOS event

					if (m_anglespeed > 0) m_anglespeed *= -(0.005f + m_elasticity);
				}
				if (m_angle < m_angleMin)
				{
					m_angle = m_angleMin;

					m_pspinner->FireVoidEventParm(DISPID_LimitEvents_BOS, fabs(m_anglespeed*180/PI));	// send Park event

					if (m_anglespeed < 0) m_anglespeed *= -(0.005f + m_elasticity);
				}
			}
		else*/
			{
				float target;

				if (m_anglespeed > 0)
				{
					if (m_angle < PI)target = PI;
					else target = 3*PI;
				}
				else
				{
					if (m_angle < PI) target = -PI;
					else target = PI;
				}

				m_angle += m_anglespeed * (float)dtime;

				if (m_anglespeed > 0)
				{
					if (m_angle > target) 
					{m_pspinner->FireGroupEvent(DISPID_SpinnerEvents_Spin);}
				}
				else
				{
					if (m_angle < target) 
					{m_pspinner->FireGroupEvent(DISPID_SpinnerEvents_Spin);}
				}

				while (m_angle > (2*PI)) m_angle -= (2*PI);
				while (m_angle < 0)m_angle += 2*PI;	

			}

		}
	}

void SpinnerAnimObject::UpdateAcceleration(PINFLOAT dtime)
	{
	if (fOldPhys)
		{
		m_anglespeed -= (float)(sin(m_angle)*0.005); // Center of gravity towards bottom of object, makes it stop vertical
		m_anglespeed *= m_pspinner->m_d.m_antifriction;
		}
	else
		{
		m_anglespeed -= sin(m_angle) *(float)dtime *0.0025f; // Center of gravity towards bottom of object, makes it stop vertical
		m_anglespeed *= 1.0f - (1.0f - m_pspinner->m_d.m_antifriction)*(float)dtime;
		}
	}

void SpinnerAnimObject::Check3D()
	{
	int frame = (int)(((m_angle / (2*PI)) * m_vddsFrame.Size()) + 0.5);

	if (frame == m_vddsFrame.Size())
		{
		frame = 0;
		}

	if (frame != m_iframe)
		{
		LPDIRECTDRAWSURFACE7 pdds;

		pdds = g_pplayer->m_pin3d.m_pddsBackBuffer;

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

ObjFrame *SpinnerAnimObject::Draw3D(RECT *prc)
	{
	LPDIRECTDRAWSURFACE7 pdds;

	pdds = g_pplayer->m_pin3d.m_pddsBackBuffer;

	ObjFrame *pobjframe = m_vddsFrame.ElementAt(m_iframe);

	/*HRESULT hr = pdds->BltFast(pobjframe->rc.left, pobjframe->rc.top, pobjframe->pdds,
		NULL, DDBLTFAST_SRCCOLORKEY);
	g_pplayer->m_pin3d.m_pddsZBuffer->BltFast(pobjframe->rc.left, pobjframe->rc.top,
		pobjframe->pddsZBuffer, NULL, DDBLTFAST_NOCOLORKEY);*/

	return pobjframe;
	}

void SpinnerAnimObject::Reset()
	{
	m_iframe = -1;
	}

void HitSpinner::Draw(HDC hdc)
	{
	m_lineseg[0].Draw(hdc);
	m_lineseg[1].Draw(hdc);
	}

void HitSpinner::CalcHitRect()
	{
	// Bounding rect for both lines will be the same
	m_lineseg[0].CalcHitRect();
	m_rcHitRect = m_lineseg[0].m_rcHitRect;
	}

TriggerHitCircle::TriggerHitCircle()
	{
	m_fEnabled = fTrue;
	}

PINFLOAT TriggerHitCircle::HitTest(Ball *pball, PINFLOAT dtime, Vertex3D *phitnormal)
	{
	if (!fOldPhys)
	{
		return HitTestBasicRadius(pball, dtime, phitnormal, false, false, false);//any face, not-lateral, non-rigid
	}
	
	if (!m_fEnabled)
		{
		return -1;
		}

	//float radius = 0;
	PINFLOAT vx = 0;
	PINFLOAT vy = 0;
	PINFLOAT x = center.x;
	PINFLOAT y = center.y;

	PINFLOAT result;
	PINFLOAT dx,dy;
	//PINFLOAT totalradius = pball->radius + radius;
	PINFLOAT totalradius = 0 + radius;
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
		BOOL fUnhit;

		time1 = (-b + (PINFLOAT)(sqrt(result))) / (2*a);
		time2 = (-b - (PINFLOAT)(sqrt(result))) / (2*a);

		if (time1*time2 < 0) // ball is inside the trigger
			{
			fUnhit = fTrue;

			hittime = max(time1,time2);
			}
		else
			{
			fUnhit = fFalse;
			hittime = min(time1,time2);
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

		(phitnormal)->x = (float)((pball->x - hitx)/totalradius);
		(phitnormal)->y = (float)((pball->y - hity)/totalradius);
		phitnormal->z = 0;

		phitnormal[1].x = (float)fUnhit;

#ifdef LOG

	fprintf(g_pplayer->m_flog, "Trigger Hit\n");

	fprintf(g_pplayer->m_flog, "dTime: %.20f hittime: %.20f\n", dtime, hittime);

#endif

		return hittime;
		}

	return -1;

	//return HitTestRadius(pball, dtime, phitnormal, 0);
	}

void TriggerHitCircle::Collide(Ball *pball, Vertex3D *phitnormal)
	{
	if (fOldPhys)
		{
		if (phitnormal[1].x < 1)
			{
			m_ptrigger->FireGroupEvent(DISPID_HitEvents_Hit);
			//m_ptrigger->FireBallEvent(DISPID_HitEvents_Hit, pball);
			}
		else
			{
			m_ptrigger->FireGroupEvent(DISPID_HitEvents_Unhit);
			//m_ptrigger->FireBallEvent(DISPID_HitEvents_Unhit, pball);
			}
		}
	else // new phys
		{
			if (m_ObjType < eTrigger)return;// triggers and kickers

			if (!pball->m_vpVolObjs) return;

			int i = pball->m_vpVolObjs->IndexOf(m_pObj); // if -1 then not in objects volume set (i.e not already hit)

			if ((phitnormal[1].x < 1) == (i < 0))	// Hit == NotAlreadyHit
			{			
				pball->x += pball->vx * STATICTIME; //move ball slightly forward
				pball->y += pball->vy * STATICTIME;		
				pball->z += pball->vz * STATICTIME; 		

				if (i < 0)
				{	
					pball->m_vpVolObjs->AddElement(m_pObj);
					((Trigger*)m_pObj)->FireGroupEvent(DISPID_HitEvents_Hit);
				}		
				else			
				{
					pball->m_vpVolObjs->RemoveElementAt(i);
					((Trigger*)m_pObj)->FireGroupEvent(DISPID_HitEvents_Unhit);
				}
			}	

		}
	}

Hit3DPoly::Hit3DPoly(Vertex3D *rgv, int count)
	{
	m_cvertex = count;

	m_rgv = new Vertex3D[count];

	memcpy(m_rgv, rgv, count * sizeof(Vertex3D));

	CalcNormal();

	m_fVisible = fFalse;
	}

Hit3DPoly::~Hit3DPoly()
	{
	delete m_rgv;
	}
	
PINFLOAT Hit3DPoly::HitTestBasicPolygon(Ball *pball, PINFLOAT dtime, Vertex3D *phitnormal,bool direction, bool rigid)
{
	if (!m_fEnabled) return -1;

	PINFLOAT hittime,bnd;
	PINFLOAT bnv; 
	PINFLOAT hitx, hity, hitz; // Point on the ball that will hit the polygon, if it hits at all

	bnv = (normal.x*pball->vx) + (normal.y*pball->vy) + (normal.z*pball->vz);  //speed in Normal-vector direction

	if (direction && bnv >= 0)								//rlc ... return if clearly ball is receding from object
	{return -1;}

	PINFLOAT bRadius = pball->radius;
	hitx = (-normal.x) * bRadius + pball->x; //rlc nearest point on ball ... projected radius along norm
	hity = (-normal.y) * bRadius + pball->y;
	hitz = (-normal.z) * bRadius + pball->z;

	bnd = normal.x * hitx + normal.y * hity + normal.z * hitz + D; // distance from plane to ball

	bool bUnHit = bnv > C_LOWNORMVEL;
	bool inside = bnd <= 0;									// in ball inside object volume

	if (rigid) //rigid polygon
	{
		if (bnd < -PHYS_SKIN) return -1;	// (ball normal distance) excessive pentratration of object skin ... no collision HACK

		if (bnd >= -PHYS_SKIN && bnd <= PHYS_TOUCH)
		{
			if (inside || fabsf((float)bnv) > C_CONTACTVEL)		// fast velocity, return zero time
				hittime = 0;													//zero time for rigid fast bodies
			else if((float)bnd <= -PHYS_TOUCH) hittime = 0;							// slow moving but embedded
			else hittime = ((float)bnd + PHYS_TOUCH) * (1.0f/PHYS_TOUCH/2);			// don't compete for fast zero time events															
		}
		else if (fabsf((float)bnv) > C_LOWNORMVEL )					// not velocity low ????
			hittime = bnd/(-bnv);								// rate ok for safe divide 
		else return -1;											// wait for touching
	}
	else //non-rigid polygon
	{
		if (bnv * bnd >= 0)										// outside-receding || inside-approaching
		{
			if (m_ObjType != eTrigger)return -1;				// not a trigger
			if (!pball->m_vpVolObjs) return -1;					// temporary ball

			if (fabs(bnd) >= PHYS_SKIN/2)		// not to close ... nor to far away
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

	hitx += pball->vx*hittime;	// advance hit point to contact
	hity += pball->vy*hittime;
	hitz += pball->vz*hittime;

	// Do a point in poly test, using the xy plane, to see if the hit point is inside the polygon
	//this need to be changed to a point in polygon on 3D plane

	int i,j;
	PINFLOAT x1,y1,x2,y2;
	int crosscount=0;	// count of lines which the hit point is to the left of
	for (i=0;i<m_cvertex;i++)
	{
		j = ((i+1) % m_cvertex);

		y1 = m_rgv[i].y;
		y2 = m_rgv[j].y;

		if (y1==y2)
			goto done;

		if ((hity <= y1 && hity <= y2) || (hity > y1 && hity > y2)) // if out of y range, forget about this segment
			goto done;

		x1 = m_rgv[i].x;
		x2 = m_rgv[j].x;

		if (hitx >= x1 && hitx >= x2) // Hit point is on the right of the line
			goto done;

		if (hitx < x1 && hitx < x2)
		{
			crosscount++;
			goto done;
		}

		if (x2 == x1)
		{
			if (hitx < x2)
				crosscount++;
			goto done;
		}

		// Now the hard part - the hit point is in the line bounding box

		if ( (x2 - ( (y2 - hity) * (x1 - x2) / (y1 - y2) ) ) > hitx)
			crosscount++;
done:
		;
	}

	if (crosscount & 1)
	{
		phitnormal->x = m_rgv[0].z;
		phitnormal->y = m_rgv[2].z;

		if (!rigid)								// non rigid body collision? return direction
		{ phitnormal[1].x = (float)bUnHit;}	// UnHit signal	is receding from outside target

		pball->m_HitDist = bnd;					//rlc-3dhit actual contact distance ... 
		pball->m_HitNormVel = bnv;
		pball->m_HitRigid = rigid;				// collision type

		return hittime;
	}

	return -1;
}

PINFLOAT Hit3DPoly::HitTest(Ball *pball, PINFLOAT dtime, Vertex3D *phitnormal)
	{
	if (!fOldPhys)
		{
		if (m_ObjType == eTrigger)
			{
			return HitTestBasicPolygon(pball, dtime, phitnormal,false, false);
			}

		return HitTestBasicPolygon(pball, dtime, phitnormal,true, true); 
		}

	PINFLOAT t;
	PINFLOAT denom;
	PINFLOAT hitx, hity, hitz; // Point on the ball that will hit the polygon, if it hits at all

	denom = normal.x * pball->vx + normal.y * pball->vy + normal.z * pball->vz;

	//if ((pball->vx*normal.x + pball->vy*normal.y + pball->vz*normal.z) > 0)
	if (denom >= 0)// ball is travelling exactly parallel to the polygon, or away from it
		{
		return -1;
		}

	hitx = (PINFLOAT)(-normal.x) * pball->radius + pball->x;
	hity = (PINFLOAT)(-normal.y) * pball->radius + pball->y;
	hitz = (PINFLOAT)(-normal.z) * pball->radius + pball->z;

	//denom = normal.x * pball->vx + normal.y * pball->vy + normal.z * pball->vz;

	//if (denom == 0) // ball if travelling exactly parallel to the polygon
		//{
		//return -1;
		//}

	t = normal.x * hitx + normal.y * hity + normal.z * hitz + D;
	t = -t/denom;

	PINFLOAT slipfactor;
	PINFLOAT len = sqrt(pball->vx*pball->vx + pball->vy*pball->vy + pball->vz*pball->vz);
	slipfactor = -(1/len);

	if (t > dtime || t < slipfactor)
		{
		return -1;
		}

	hitx += pball->vx*t;
	hity += pball->vy*t;
	hitz += pball->vz*t;

	t = max(0,t); // for slip factor

	// Do a point in poly test, using the xy plane, to see if the hit point is inside the polygon

	int i,j;
	PINFLOAT x1,y1,x2,y2;
	int crosscount=0;	// count of lines which the hit point is to the left of
	for (i=0;i<m_cvertex;i++)
		{
		j = ((i+1) % m_cvertex);

		y1 = m_rgv[i].y;
		y2 = m_rgv[j].y;

		if (y1==y2)
			goto done;

		if ((hity <= y1 && hity <= y2) || (hity > y1 && hity > y2)) // if out of y range, forget about this segment
			goto done;

		x1 = m_rgv[i].x;
		x2 = m_rgv[j].x;

		if (hitx >= x1 && hitx >= x2) // Hit point is on the right of the line
			goto done;

		if (hitx < x1 && hitx < x2)
			{
			crosscount++;
			goto done;
			}

		if (x2 == x1)
			{
			if (hitx < x2)
				crosscount++;
			goto done;
			}

		// Now the hard part - the hit point is in the line bounding box

		if ( (x2 - ( (y2 - hity) * (x1 - x2) / (y1 - y2) ) ) > hitx)
			crosscount++;
done:
;
		}

	if (crosscount & 1)
		{
		int i = 1;
		}

	if (t > dtime || t < 0)
		{
		return -1;
		}

	if (crosscount & 1)
		{
		phitnormal->x = m_rgv[0].z;
		phitnormal->y = m_rgv[2].z;
		
		//pball->m_HitDist = bnd;					//rlc-3dhit actual contact distance ... 
		//pball->m_HitNormVel = bnv;
		//pball->m_HitRigid = rigid;				// collision type

		return t;
		}

	return -1;
	}

void Hit3DPoly::Draw(HDC hdc)
	{
	}

void Hit3DPoly::Collide(Ball *pball, Vertex3D *phitnormal)
	{
	if (fOldPhys)
		{
		pball->Collide3DWall(&normal, m_elasticity, 0, 0);
		}
	else // new phys
		{
			if (m_ObjType != eTrigger) pball->Collide3DWall(&normal, m_elasticity, 0, 0); 
			else		
			{
				if (!pball->m_vpVolObjs) return;

				int i = pball->m_vpVolObjs->IndexOf(m_pObj); // if -1 then not in objects volume set (i.e not already hit)

				if ((phitnormal[1].x < 1) == (i < 0))	// Hit == NotAlreadyHit
				{			
					pball->x += pball->vx * STATICTIME; //move ball slightly forward
					pball->y += pball->vy * STATICTIME;		
					pball->z += pball->vz * STATICTIME; 	

					if (i < 0)
					{	
						pball->m_vpVolObjs->AddElement(m_pObj);
						((Trigger*)m_pObj)->FireGroupEvent(DISPID_HitEvents_Hit);				
					}		
					else			
					{
						pball->m_vpVolObjs->RemoveElementAt(i);
						((Trigger*)m_pObj)->FireGroupEvent(DISPID_HitEvents_Unhit);				
					}
				}	
			}
		}
	}

void Hit3DPoly::CalcNormal()
	{
	WORD *rgi;
	int i;

	rgi = new WORD[m_cvertex];

	for (i=0;i<m_cvertex;i++)
		{
		rgi[i]= i;
		}

	//SetNormal(m_rgv, rgi, 3, NULL, NULL, NULL);

	SetNormal(m_rgv, rgi, m_cvertex, NULL, NULL, NULL);
	normal.x = m_rgv[0].nx;
	normal.y = m_rgv[0].ny;
	normal.z = m_rgv[0].nz;
	D = -(normal.x * m_rgv[0].x + normal.y * m_rgv[0].y + normal.z * m_rgv[0].z);

	delete rgi;
	}

void Hit3DPoly::CalcHitRect()
	{
	int i;

	m_rcHitRect.left = FLT_MAX;
	m_rcHitRect.right = -FLT_MAX;
	m_rcHitRect.top = FLT_MAX;
	m_rcHitRect.bottom = -FLT_MAX;
	m_rcHitRect.zlow = FLT_MAX;
	m_rcHitRect.zhigh = -FLT_MAX;

	for (i=0;i<m_cvertex;i++)
		{
		m_rcHitRect.left = min(m_rgv[i].x, m_rcHitRect.left);
		m_rcHitRect.right = max(m_rgv[i].x, m_rcHitRect.right);
		m_rcHitRect.top = min(m_rgv[i].y, m_rcHitRect.top);
		m_rcHitRect.bottom = max(m_rgv[i].y, m_rcHitRect.bottom);
		m_rcHitRect.zlow = min(m_rgv[i].z, m_rcHitRect.zlow);
		m_rcHitRect.zhigh = max(m_rgv[i].z, m_rcHitRect.zhigh);
		}
	}

Hit3DCylinder::Hit3DCylinder(Vertex3D *pv1, Vertex3D *pv2, Vertex3D *pvnormal)
	{
	v1 = *pv1;
	v2 = *pv2;
	normal = *pvnormal;
	radius = 0;
	CacheHitTransform();
	}

void Hit3DCylinder::CacheHitTransform()
	{
	Vertex3D vLine;

	vLine.x = v2.x - v1.x;
	vLine.y = v2.y - v1.y;
	vLine.z = v2.z - v1.z;

	vLine.Normalize();

	Vertex3D vup;

	vup.x = 0;
	vup.y = 0;
	vup.z = 1;

	// Axis of rotation to make 3D cynlinder a cylinder along the z-axis
	CrossProduct(&vLine, &vup, &transaxis);

	// Angle to rotate the cylinder into the z-axis
	PINFLOAT dot;
	dot = vLine.Dot(&vup);

	transangle = acos(-dot);

	vtrans[0] = v1;
	vtrans[1] = v2;

	RotateAround(&transaxis, vtrans, 2, (float)transangle);
	}

PINFLOAT Hit3DCylinder::HitTest(Ball *pball, PINFLOAT dtime, Vertex3D *phitnormal)
	{
	Ball ballT = *pball;
	ballT.m_vpVolObjs = NULL; // HACK HACK HACK - this should be dealt with some other way so that this ball doesn't delete this member variable when it is destructed.

	Vertex3D vball;
	vball.x = (float)(ballT.x);
	vball.y = (float)(ballT.y);
	vball.z = (float)(ballT.z);

	RotateAround(&transaxis, &vball, 1, (float)transangle);

	Vertex3D vvelocity;
	vvelocity.x = (float)(ballT.vx);
	vvelocity.y = (float)(ballT.vy);
	vvelocity.z = (float)(ballT.vz);

	RotateAround(&transaxis, &vvelocity, 1, (float)transangle);

	ballT.x = vball.x;
	ballT.y = vball.y;
	// HACK!!! After tech beta, need to make the ball z actually the real value
	//ballT.PutZ(vball.z - pball->radius);
	ballT.z = vball.z + pball->radius;
	ballT.vx = vvelocity.x;
	ballT.vy = vvelocity.y;
	ballT.vz = vvelocity.z;
	ballT.radius = pball->radius;

	center.x = vtrans[0].x;
	center.y = vtrans[0].y;
	zlow = min(vtrans[0].z, vtrans[1].z);
	zhigh = max(vtrans[0].z, vtrans[1].z);

	PINFLOAT hittime = HitTestRadius(&ballT, dtime, phitnormal, ballT.radius);

	if (hittime != -1)
		{
		/*float hitz = ballT.z + ballT.vz * hittime + pball->radius;
		// Check the z-value in our transformed coordinate system to
		// see if the the ball hit the cylinder between the two
		// line vertices
		if (hitz < min(nv1.z, nv2.z) || hitz > max(nv1.z, nv2.z))
			{
			return -1;
			}*/
		Vertex3D mynormal;
		mynormal.x = phitnormal->x;
		mynormal.y = phitnormal->y;
		mynormal.z = 0;
		RotateAround(&transaxis, &mynormal, 1, (float)-transangle);
		*(Vertex3D *)phitnormal = mynormal;
		}

	return hittime;
	}

void Hit3DCylinder::Collide(Ball *pball, Vertex3D *phitnormal)
	{
	//pball->m_plevel = NULL;
	pball->Collide3DWall((Vertex3D *)phitnormal, m_elasticity, 0, 0);
	}

void Hit3DCylinder::CalcHitRect()
	{
	m_rcHitRect.left = min(v1.x, v2.x);
	m_rcHitRect.right = max(v1.x, v2.x);
	m_rcHitRect.top = min(v1.y, v2.y);
	m_rcHitRect.bottom = max(v1.y, v2.y);
	m_rcHitRect.zlow = min(v1.z, v2.z);
	m_rcHitRect.zhigh = max(v1.z, v2.z);
	}

Hit3DPolyDrop::Hit3DPolyDrop(Vertex3D *rgv, int count) : Hit3DPoly(rgv, count)
	{
	m_polydropanim.m_iframe = -1;
	m_fVisible = fFalse;
	}

void PolyDropAnimObject::Check3D()
	{
	if (m_iframe != m_iframedesire)
		{
		m_iframe = m_iframedesire;
		m_fInvalid = fTrue;
		}

	/*if (m_fInvalid)
		{
		ObjFrame *pobjframe = m_pobjframe[m_iframe];

		g_pplayer->m_pin3d.m_pddsBackBuffer->BltFast(pobjframe->rc.left, pobjframe->rc.top, g_pplayer->m_pin3d.m_pddsStatic,
			&pobjframe->rc, DDBLTFAST_NOCOLORKEY);
		g_pplayer->m_pin3d.m_pddsZBuffer->BltFast(pobjframe->rc.left, pobjframe->rc.top,
			g_pplayer->m_pin3d.m_pddsStaticZ, &pobjframe->rc, DDBLTFAST_NOCOLORKEY);
		}*/
	}

ObjFrame *PolyDropAnimObject::Draw3D(RECT *prc)
	{
	ObjFrame *pobjframe = m_pobjframe[m_iframe];

	/*g_pplayer->m_pin3d.m_pddsBackBuffer->BltFast(pobjframe->rc.left, pobjframe->rc.top, pobjframe->pdds,
		NULL, DDBLTFAST_SRCCOLORKEY);
	g_pplayer->m_pin3d.m_pddsZBuffer->BltFast(pobjframe->rc.left, pobjframe->rc.top,
		pobjframe->pddsZBuffer, NULL, DDBLTFAST_NOCOLORKEY);*/

	return pobjframe;
	}

void PolyDropAnimObject::Reset()
	{
	m_iframe = -1;
	}

ObjFrame *TextboxAnimObject::Draw3D(RECT *prc)
	{
	g_pplayer->m_pin3d.m_pddsBackBuffer->Blt(&m_ptextbox->m_pobjframe->rc, m_ptextbox->m_pobjframe->pdds, NULL, DDBLTFAST_WAIT, NULL);

	return NULL;
	}

void TextboxAnimObject::Reset()
	{
	}

//>>> added by Chris

// this function is called every frame to see if the object needs updating
//
void DispReelAnimObject::Check3D()
{
    // update the reels animation (returns saying weither to redraw the frame object or not)
    m_fInvalid = m_pDispReel->RenderAnimation();
}

// this function is called when it is time to be drawn (Z-Ordered)
// and only if Check3D() tells it to. (m_fInvalid == true)
//
ObjFrame *DispReelAnimObject::Draw3D(RECT *prc)
{
	// blit the current reel frame object onto the back buffer
//	g_pplayer->m_pin3d.m_pddsBackBuffer->Blt(&m_pDispReel->m_pobjframe->rc,
//			                                 m_pDispReel->m_pobjframe->pdds,
//				                             NULL,
//					                         DDBLTFAST_WAIT,
//						                     NULL);
	RECT	rc;
	rc.left = 0;
	rc.right = m_pDispReel->m_pobjframe->rc.right - m_pDispReel->m_pobjframe->rc.left;
	rc.top = 0;
	rc.bottom = m_pDispReel->m_pobjframe->rc.bottom - m_pDispReel->m_pobjframe->rc.top;

	g_pplayer->m_pin3d.m_pddsBackBuffer->BltFast(m_pDispReel->m_pobjframe->rc.left,
												 m_pDispReel->m_pobjframe->rc.top,
												 m_pDispReel->m_pobjframe->pdds,
												 &rc,
												 DDBLTFAST_SRCCOLORKEY/*DDBLTFAST_WAIT*/);
												 
	/*m_pDispReel->UpdateObjFrame();*/
	
	// return with no object
	return (ObjFrame *)NULL;
}
//<<<

void DispReelAnimObject::Reset()
	{
	}

// this function is called every frame to see if the object needs updating
//
void LightSeqAnimObject::Check3D()
{
    // update the reels animation (returns saying weither to redraw the frame object or not)
    m_fInvalid = m_pLightSeq->RenderAnimation();
}

// this function is called when it is time to be drawn (Z-Ordered)
// and only if Check3D() tells it to. (m_fInvalid == true)
//
ObjFrame *LightSeqAnimObject::Draw3D(RECT *prc)
{
	// return with no object
	return (ObjFrame *)NULL;
}

TriggerLineSeg::TriggerLineSeg()
	{
	m_fEnabled = fTrue;
	}

PINFLOAT TriggerLineSeg::HitTest(Ball *pball, PINFLOAT dtime, Vertex3D *phitnormal)
	{
	if (!fOldPhys)
	{
		// approach either face, not lateral-rolling point (assume center),  not a rigid body contact
		return this->HitTestBasic(pball, dtime, phitnormal, false,false, false); 	
	}
	
	PINFLOAT endx, endy;

	PINFLOAT hitx, hity;

	PINFLOAT x1,y1,x2,y2;

	PINFLOAT ballvx = pball->vx;
	PINFLOAT ballvy = pball->vy;
	PINFLOAT ballx;
	PINFLOAT bally;

	BOOL fUnhit;

	if (!m_fEnabled)
		{
		return -1;
		}

	if ((ballvx*normal.x + ballvy*normal.y) > 0 || !m_fEnabled)
		{
		fUnhit = fTrue;
		}
	else
		{
		fUnhit = fFalse;
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

	// Do not take radius into account - the ball hits only at the center.
	hitx = ballx;
	hity = bally;

	// Catch the round-off error by seeing if the ball will hit in a slightly
	// negative time, meaning the ball is interseting the wall due to
	// round-off error
	PINFLOAT startx = hitx;// - (ballvx/len);
	PINFLOAT starty = hity;// - (ballvy/len);

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

	phitnormal[1].x = (float)fUnhit;

	return hittime;
	}

void TriggerLineSeg::Collide(Ball *pball, Vertex3D *phitnormal)
	{
		if (fOldPhys)
		{
			if (phitnormal[1].x < 1)
				{
				m_ptrigger->FireGroupEvent(DISPID_HitEvents_Hit);
				}
			else
				{
				m_ptrigger->FireGroupEvent(DISPID_HitEvents_Unhit);
				}
		}
		else // new phys
		{
			if (m_ObjType != eTrigger)return;

			if (!pball->m_vpVolObjs) return;

			int i = pball->m_vpVolObjs->IndexOf(m_pObj); // if -1 then not in objects volume set (i.e not already hit)

			if ((phitnormal[1].x < 1) == (i < 0))	// Hit == NotAlreadyHit
			{		
				pball->x += pball->vx * STATICTIME; //move ball slightly forward
				pball->y += pball->vy * STATICTIME;		
				pball->z += pball->vz * STATICTIME; 	

				if (i < 0)
				{	
					pball->m_vpVolObjs->AddElement(m_pObj);
					((Trigger*)m_pObj)->FireGroupEvent(DISPID_HitEvents_Hit);			
				}		
				else			
				{
					pball->m_vpVolObjs->RemoveElementAt(i);
					((Trigger*)m_pObj)->FireGroupEvent(DISPID_HitEvents_Unhit);			
				}
			}	

		}
	}

