#include "stdafx.h"

BumperHitCircle::BumperHitCircle()
{
	m_bumperanim.m_iframe = 0;
	m_bumperanim.m_TimeReset = 0;
	m_bumperanim.m_iframedesired = 0;
	m_bumperanim.m_fAutoTurnedOff = fFalse;
	m_elasticity = 0.3f;
    SetFriction(0.0f);
	m_scatter = 0;
	m_bumperanim.m_fVisible = fTrue;
}

void BumperHitCircle::Collide(CollisionEvent* coll)
{
    Ball *pball = coll->ball;
    const Vertex3Ds& hitnormal = coll->normal[0];

	const float dot = hitnormal.x * pball->vel.x + hitnormal.y * pball->vel.y;

	pball->CollideWall(hitnormal, m_elasticity, /*m_friction*/ 0.3f, m_scatter);	//reflect ball from wall

	// if the bumper is disabled then don't activate the bumper
	if (m_pbumper->m_fDisabled)
		return;

	if (dot <= -m_pbumper->m_d.m_threshold) // if velocity greater than threshold level
	{
		pball->vel.x += hitnormal.x * m_pbumper->m_d.m_force; // add a chunk of velocity to drive ball away
		pball->vel.y += hitnormal.y * m_pbumper->m_d.m_force;

		if (m_pbumper->m_d.m_fFlashWhenHit)
		{
			m_bumperanim.m_TimeReset = g_pplayer->m_time_msec + 100;
			m_bumperanim.m_fAutoTurnedOff = fTrue;
		}

		m_pbumper->FireGroupEvent(DISPID_HitEvents_Hit);
	}
}

void BumperAnimObject::UpdateAnimation()
{
	if(!m_fVisible)
		return;
	
	if ((m_iframe == 0) && (m_TimeReset != 0))
	{
		m_iframe = 1;
	}
	else if (m_fAutoTurnedOff && (m_iframe == 1) && (m_TimeReset < g_pplayer->m_time_msec) && (m_iframe != m_iframedesired))
	{
		m_iframe = m_iframedesired;
		m_TimeReset = 0;
		m_fAutoTurnedOff = fFalse;
	}
	else if (!m_fAutoTurnedOff && (m_iframe != m_iframedesired))
	{
		m_iframe = m_iframedesired;
	}
}

void BumperAnimObject::Reset()
{
	m_iframe = 0;
}

LineSegSlingshot::LineSegSlingshot()
{
	m_slingshotanim.m_iframe = 0;
	m_slingshotanim.m_TimeReset = 0; // zero means the slingshot is currently reset
	m_elasticity = 0.3f;
    SetFriction(0.0f);
	m_scatter = 0;
}

float LineSegSlingshot::HitTest(const Ball * pball, float dtime, CollisionEvent& coll)
{
	if (!m_fEnabled) return -1.0f;	

	return this->HitTestBasic(pball, dtime, coll, true, true, true);
}

void LineSegSlingshot::Collide(CollisionEvent* coll)
{
    Ball *pball = coll->ball;
    const Vertex3Ds& hitnormal = coll->normal[0];

	const float dot = pball->vel.x * hitnormal.x  + pball->vel.y * hitnormal.y; // normal velocity to slingshot

	const bool threshold = (dot <= -m_psurface->m_d.m_slingshot_threshold);  // normal greater than threshold?

	if (!m_psurface->m_fDisabled && threshold) // enabled and if velocity greater than threshold level		
	{
		const float len = (v2.x - v1.x)*hitnormal.y - (v2.y - v1.y)*hitnormal.x; // length of segment, Unit TAN points from V1 to V2

		const Vertex2D vhitpoint(pball->pos.x - hitnormal.x * pball->radius, //project ball radius along norm
								 pball->pos.y - hitnormal.y * pball->radius);

		// vhitpoint will now be the point where the ball hits the line
		// Calculate this distance from the center of the slingshot to get force

		const float btd = (vhitpoint.x - v1.x)*hitnormal.y - (vhitpoint.y - v1.y)*hitnormal.x; // distance to vhit from V1
		float force = (len != 0.0f) ? ((btd+btd)/len - 1.0f) : -1.0f;	// -1..+1
		force = 0.5f *(1.0f-force*force);	//!! maximum value 0.5 ...I think this should have been 1.0...oh well
											// will match the previous physics
		force *= m_force;//-80;

		pball->vel.x -= hitnormal.x * force;	// boost velocity, drive into slingshot (counter normal)
		pball->vel.y -= hitnormal.y * force;	// allow CollideWall to handle the remainder
	}

	pball->CollideWall(hitnormal, m_elasticity, /*m_friction*/ 0.3f, m_scatter);

    if (m_pfe && !m_psurface->m_fDisabled && threshold)
    {
        // is this the same place as last event? if same then ignore it
        const Vertex3Ds dist = pball->m_Event_Pos - pball->pos;
        pball->m_Event_Pos = pball->pos; //remember last collide position

        if (dist.LengthSquared() > 0.25f) // must be a new place if only by a little
        {
            m_pfe->FireGroupEvent(DISPID_SurfaceEvents_Slingshot);
            m_slingshotanim.m_TimeReset = g_pplayer->m_time_msec + 100;
        }
    }
}

void SlingshotAnimObject::Check3D()
{
	if ((m_iframe == 0) && (m_TimeReset != 0) && m_fAnimations)
	{
		m_iframe = 1;
	}
	else if ((m_iframe == 1) && (m_TimeReset < g_pplayer->m_time_msec))
	{
		m_iframe = 0;
		m_TimeReset = 0;
	}
}

void SlingshotAnimObject::Reset()
{
	// Do nothing right now - just let it draw as not hit
}

HitGate::HitGate(Gate * const pgate)
{
	m_pgate = pgate;

	m_gateanim.m_angleMin = pgate->m_d.m_angleMin;
	m_gateanim.m_angleMax = pgate->m_d.m_angleMax;

	m_gateanim.m_friction = pgate->m_d.m_friction;
	m_gateanim.m_fVisible = pgate->m_d.m_fVisible;	

	m_gateanim.m_angle = m_gateanim.m_angleMin;
	m_gateanim.m_anglespeed = 0;

	m_gateanim.m_pgate = pgate;
	m_gateanim.m_fOpen = fFalse;
}

float HitGate::HitTest(const Ball * pball, float dtime, CollisionEvent& coll)
{	
	if (!m_fEnabled) return -1.0f;		

	return HitTestBasic(pball, dtime, coll, true, true, false); // normal face, lateral, non-rigid
}

void HitGate::Collide(CollisionEvent* coll)
{
    Ball *pball = coll->ball;
    const Vertex3Ds& hitnormal = coll->normal[0];

	const float dot = pball->vel.x * hitnormal.x + pball->vel.y * hitnormal.y;
	if (dot > 0.0f) return;	//hit from back doesn't count

	const float h = m_pgate->m_d.m_height;

	//linear speed = ball speed
	//angular speed = linear/radius (height of hit)

	// h is the height of the gate axis.
	m_gateanim.m_anglespeed = fabsf(dot); // use this until a better value comes along

	if (fabsf(h - pball->radius) > 1.0f)				// avoid divide by zero
		m_gateanim.m_anglespeed /= h - pball->radius;

    FireHitEvent(pball);
}

void GateAnimObject::UpdateDisplacements(const float dtime)
{
	m_angle += m_anglespeed * dtime;

	if (m_angle > m_angleMax)
	{
		m_angle = m_angleMax;
		m_pgate->FireVoidEventParm(DISPID_LimitEvents_EOS, fabsf(RADTOANG(m_anglespeed)));	// send EOS event
		if (m_anglespeed > 0.0f)
			m_anglespeed = 0.0f;
	}
	if (m_angle < m_angleMin)
	{
		m_angle = m_angleMin;
		m_pgate->FireVoidEventParm(DISPID_LimitEvents_BOS, fabsf(RADTOANG(m_anglespeed)));	// send Park event
		if (m_anglespeed < 0.0f)
			m_anglespeed = 0.0f;
	}
}

void GateAnimObject::UpdateVelocities()
{
	if (!m_fOpen)
	{
		if (m_angle == m_angleMin)
			m_anglespeed = 0.0f;
		else
			m_anglespeed = (m_anglespeed - sinf(m_angle) * 0.0025f) * (1.0f - m_friction); // Center of gravity towards bottom of object, makes it stop vertical
		//else m_anglespeed = (m_anglespeed-sinf((m_angle - m_angleMin)*0.5f) * 0.02f)*(1.0f - m_friction); // Center of gravity towards bottom of object, makes it stop vertical
	}
}

void GateAnimObject::Reset()
{
}

HitSpinner::HitSpinner(Spinner * const pspinner, const float height)
{
	m_spinneranim.m_pspinner = pspinner;

	const float halflength = pspinner->m_d.m_length * 0.5f;

	const float radangle = ANGTORAD(pspinner->m_d.m_rotation);
	const float sn = sinf(radangle);
	const float cs = cosf(radangle);

	m_lineseg[0].m_rcHitRect.zlow = height;
	m_lineseg[0].m_rcHitRect.zhigh = height + 50.0f;
	m_lineseg[1].m_rcHitRect.zlow = height;
	m_lineseg[1].m_rcHitRect.zhigh = height + 50.0f;

	m_lineseg[0].m_pfe = NULL;
	m_lineseg[1].m_pfe = NULL;

	m_lineseg[0].v2.x = pspinner->m_d.m_vCenter.x + cs*(halflength + (float)PHYS_SKIN); //oversize  by the ball radius
	m_lineseg[0].v2.y = pspinner->m_d.m_vCenter.y + sn*(halflength + (float)PHYS_SKIN); //this will prevent clipping
	m_lineseg[0].v1.x = pspinner->m_d.m_vCenter.x - cs*(halflength + (float)PHYS_SKIN); //through the edge of the
	m_lineseg[0].v1.y = pspinner->m_d.m_vCenter.y - sn*(halflength + (float)PHYS_SKIN); //spinner

	m_lineseg[1].v1.x = m_lineseg[0].v2.x;
	m_lineseg[1].v1.y = m_lineseg[0].v2.y;
	m_lineseg[1].v2.x = m_lineseg[0].v1.x;
	m_lineseg[1].v2.y = m_lineseg[0].v1.y;

	m_lineseg[0].CalcNormal();
	m_lineseg[1].CalcNormal();

	m_spinneranim.m_angleMax = ANGTORAD(pspinner->m_d.m_angleMax);
	m_spinneranim.m_angleMin = ANGTORAD(pspinner->m_d.m_angleMin);

	m_spinneranim.m_angle = clamp(0.0f, m_spinneranim.m_angleMin, m_spinneranim.m_angleMax);
	m_spinneranim.m_anglespeed = 0;
    // compute proper damping factor for physics framerate
    m_spinneranim.m_damping = powf(pspinner->m_d.m_antifriction, (float)PHYS_FACTOR);

	m_spinneranim.m_elasticity = pspinner->m_d.m_elasticity;
	m_spinneranim.m_fVisible = pspinner->m_d.m_fVisible;	
}

float HitSpinner::HitTest(const Ball * pball, float dtime, CollisionEvent& coll)
{
	if (!m_fEnabled) return -1.0f;	

	{
	const float hittime = m_lineseg[0].HitTestBasic(pball, dtime, coll, false, true, false);// any face, lateral, non-rigid
	if (hittime >= 0)
	{
		coll.normal[1].x = 1.0f;
		coll.normal[1].y = 0;

		return hittime;
	}
	}

	const float hittime = m_lineseg[1].HitTestBasic(pball, dtime, coll, false, true, false);// any face, lateral, non-rigid
	if (hittime >= 0)
		coll.normal[1].x = 0;

	return hittime;
}

void HitSpinner::Collide(CollisionEvent* coll)
{
    Ball *pball = coll->ball;
    const Vertex3Ds& hitnormal = coll->normal[0];

	const float dot = pball->vel.x * hitnormal.x + pball->vel.y * hitnormal.y;
	if (dot < 0) return;	//hit from back doesn't count
	
	const float h = m_spinneranim.m_pspinner->m_d.m_height*0.5f  + 30.0f;

	//linear speed = ball speed
	//angular speed = linear/radius (height of hit)

	// h is the height of the spinner axis;
	// Since the spinner has no mass in our equation, the spot
	// h -pball->radius will be moving a at linear rate of
	// 'speed'.  We can calculate the angular speed from that.

	m_spinneranim.m_anglespeed = fabsf(dot);		// use this until a better value comes along

	if (fabsf(h - pball->radius) > 1.0f)				// avoid divide by zero
		m_spinneranim.m_anglespeed /= h - pball->radius;

	// We encoded which side of the spinner the ball hit
	if (coll->normal[1].x > 0.0f)
		m_spinneranim.m_anglespeed = -m_spinneranim.m_anglespeed; 
}

void SpinnerAnimObject::UpdateDisplacements(const float dtime)
{
	if (m_pspinner->m_d.m_angleMin != m_pspinner->m_d.m_angleMax)	//blocked spinner, limited motion spinner
	{
		m_angle += m_anglespeed * dtime;

		if (m_angle > m_angleMax)
		{
			m_angle = m_angleMax;
			m_pspinner->FireVoidEventParm(DISPID_LimitEvents_EOS, fabsf(RADTOANG(m_anglespeed)));	// send EOS event

			if (m_anglespeed > 0)
				m_anglespeed *= -0.005f - m_elasticity;
		}
		if (m_angle < m_angleMin)
		{
			m_angle = m_angleMin;

			m_pspinner->FireVoidEventParm(DISPID_LimitEvents_BOS, fabsf(RADTOANG(m_anglespeed)));	// send Park event

			if (m_anglespeed < 0)
				m_anglespeed *= -0.005f - m_elasticity;
		}
	}
	else
	{
		const float target = (m_anglespeed > 0) ?
			((m_angle < (float)M_PI) ? (float)M_PI : (float)(3.0*M_PI))
			:
			((m_angle < (float)M_PI) ? (float)(-M_PI) : (float)M_PI);

		m_angle += m_anglespeed * dtime;

		if (m_anglespeed > 0)
		{
			if (m_angle > target) 
				m_pspinner->FireGroupEvent(DISPID_SpinnerEvents_Spin);
		}
		else
		{
			if (m_angle < target) 
				m_pspinner->FireGroupEvent(DISPID_SpinnerEvents_Spin);
		}

		while (m_angle > (float)(2.0*M_PI))
			m_angle -= (float)(2.0*M_PI);
		while (m_angle < 0.0f)
			m_angle += (float)(2.0*M_PI);
	}
}

void SpinnerAnimObject::UpdateVelocities()
{
	m_anglespeed -= sinf(m_angle) * (float)(0.0025 * PHYS_FACTOR); // Center of gravity towards bottom of object, makes it stop vertical

	m_anglespeed *= m_damping;
}

void SpinnerAnimObject::Reset()
{
}

void HitSpinner::CalcHitRect()
{
	// Bounding rect for both lines will be the same
	m_lineseg[0].CalcHitRect();
	m_rcHitRect = m_lineseg[0].m_rcHitRect;
}

Hit3DPoly::Hit3DPoly(Vertex3Ds * const rgv, const int count) : m_rgv(rgv),m_cvertex(count)
{
	normal.x = 0.f;
	normal.y = 0.f;
	normal.z = 0.f;

    // Newell's method for normal computation
	for (int i=0; i<m_cvertex; ++i)
	{
		const int m = (i < m_cvertex-1) ? (i+1) : 0;

		normal.x += (m_rgv[i].y - m_rgv[m].y) * (m_rgv[i].z + m_rgv[m].z);
		normal.y += (m_rgv[i].z - m_rgv[m].z) * (m_rgv[i].x + m_rgv[m].x);
		normal.z += (m_rgv[i].x - m_rgv[m].x) * (m_rgv[i].y + m_rgv[m].y);		
	}

	const float sqr_len = normal.x * normal.x + normal.y * normal.y + normal.z * normal.z;
	const float inv_len = (sqr_len > 0.0f) ? -1.0f/sqrtf(sqr_len) : 0.0f;   // NOTE: normal is flipped!
	normal.x *= inv_len;
	normal.y *= inv_len;
	normal.z *= inv_len;

	m_fVisible = fFalse;
	m_elasticity = 0.3f;
    SetFriction(0.0f);
	m_scatter = 0;
}

Hit3DPoly::~Hit3DPoly()
{
	delete [] m_rgv;
}

float Hit3DPoly::HitTest(const Ball * pball, float dtime, CollisionEvent& coll)
{
	if (!m_fEnabled) return -1.0f;

	const float bnv = normal.Dot(pball->vel);           //speed in Normal-vector direction

	if ((m_ObjType != eTrigger) && (bnv >= 0.f))								// return if clearly ball is receding from object
		return -1.0f;

	// Point on the ball that will hit the polygon, if it hits at all
	const float bRadius = pball->radius;
    Vertex3Ds hitPos = pball->pos - bRadius * normal; // nearest point on ball ... projected radius along norm

    const float bnd = normal.Dot( hitPos - m_rgv[0] ); // distance from plane to ball

	bool bUnHit = (bnv > C_LOWNORMVEL);
	const bool inside = (bnd <= 0);									// in ball inside object volume

	const bool rigid = (m_ObjType != eTrigger);
	float hittime;
	if (rigid) //rigid polygon
	{
		if (bnd < (float)(-PHYS_SKIN)) return -1.0f;	// (ball normal distance) excessive pentratration of object skin ... no collision HACK
			
		if (bnd <= (float)PHYS_TOUCH)
		{
			if (inside || (fabsf(bnv) > C_CONTACTVEL)		// fast velocity, return zero time
															//zero time for rigid fast bodies
			|| (bnd <= (float)(-PHYS_TOUCH)))				// slow moving but embedded
				hittime = 0;
			else
				hittime = bnd*(float)(1.0/(2.0*PHYS_TOUCH)) + 0.5f;	// don't compete for fast zero time events
		}
		else if (fabsf(bnv) > C_LOWNORMVEL )					// not velocity low ????
			hittime = bnd/(-bnv);								// rate ok for safe divide 
		else
			return -1.0f;										// wait for touching
	}
	else //non-rigid polygon
	{
		if (bnv * bnd >= 0)										// outside-receding || inside-approaching
		{
			if((m_ObjType != eTrigger) ||				// not a trigger
			   (!pball->m_vpVolObjs) ||					// temporary ball
			   (fabsf(bnd) >= (float)(PHYS_SKIN/2.0)) ||		// not to close ... nor to far away
			   (inside != (pball->m_vpVolObjs->IndexOf(m_pObj) < 0)))// ...ball outside and hit set or  ball inside and no hit set
				return -1.0f;

			hittime = 0;
			bUnHit = !inside;	// ball on outside is UnHit, otherwise it's a Hit
		}
		else
			hittime = bnd/(-bnv);	
	}

	if (infNaN(hittime) || hittime < 0 || hittime > dtime) return -1.0f;	// time is outside this frame ... no collision

    hitPos += hittime * pball->vel;     // advance hit point to contact

	// Do a point in poly test, using the xy plane, to see if the hit point is inside the polygon
	//this need to be changed to a point in polygon on 3D plane

	float x2 = m_rgv[0].x;
	float y2 = m_rgv[0].y;
	bool hx2 = (hitPos.x >= x2);
	bool hy2 = (hitPos.y <= y2);
	int crosscount=0;	// count of lines which the hit point is to the left of
	for (int i=0;i<m_cvertex;i++)
	{
		const float x1 = x2;
		const float y1 = y2;
		const bool hx1 = hx2;
		const bool hy1 = hy2;
		
		const int j = (i < m_cvertex-1) ? (i+1) : 0;
		x2 = m_rgv[j].x;
		y2 = m_rgv[j].y;
		hx2 = (hitPos.x >= x2);
		hy2 = (hitPos.y <= y2);

		if ((y1==y2) ||
		    (hy1 && hy2) || (!hy1 && !hy2) || // if out of y range, forget about this segment
		    (hx1 && hx2)) // Hit point is on the right of the line
			continue;

		if (!hx1 && !hx2)
		{
			crosscount^=1;
			continue;
		}

		if (x2 == x1)
		{
			if (!hx2)
				crosscount^=1;
			continue;
		}

		// Now the hard part - the hit point is in the line bounding box

		if (x2 - (y2 - hitPos.y)*(x1 - x2)/(y1 - y2) > hitPos.x)
			crosscount^=1;
	}

	if (crosscount & 1)
	{
		coll.normal[0].x = m_rgv[0].z;
		coll.normal[0].y = m_rgv[2].z;

		if (!rigid)								// non rigid body collision? return direction
			coll.normal[1].x = bUnHit ? 1.0f : 0.0f;	// UnHit signal	is receding from outside target
			
		coll.distance = bnd;					// 3dhit actual contact distance ... 
		coll.hitRigid = rigid;				// collision type

		return hittime;
	}

	return -1.0f;
}

void Hit3DPoly::Collide(CollisionEvent *coll)
{
   Ball *pball = coll->ball;
   const Vertex3Ds& hitnormal = coll->normal[0];

   if (m_ObjType != eTrigger)
   {
      const float dot = hitnormal.x * pball->vel.x + hitnormal.y * pball->vel.y;

      pball->Collide3DWall(normal, m_elasticity, /*m_friction*/ 0.3f, m_scatter);

      if (m_ObjType == ePrimitive && dot <= -m_threshold)
          FireHitEvent(pball);
   }
	else
	{
		if (!pball->m_vpVolObjs) return;

		const int i = pball->m_vpVolObjs->IndexOf(m_pObj); // if -1 then not in objects volume set (i.e not already hit)

		if ((coll->normal[1].x < 1.0f) == (i < 0))	// Hit == NotAlreadyHit
		{			
            pball->pos += STATICTIME * pball->vel; //move ball slightly forward
				
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

void Hit3DPoly::CalcHitRect()
{
	m_rcHitRect.left = m_rgv[0].x;
	m_rcHitRect.right = m_rgv[0].x;
	m_rcHitRect.top = m_rgv[0].y;
	m_rcHitRect.bottom = m_rgv[0].y;
	m_rcHitRect.zlow = m_rgv[0].z;
	m_rcHitRect.zhigh = m_rgv[0].z;

	for (int i=1;i<m_cvertex;i++)
	{
		m_rcHitRect.left = min(m_rgv[i].x, m_rcHitRect.left);
		m_rcHitRect.right = max(m_rgv[i].x, m_rcHitRect.right);
		m_rcHitRect.top = min(m_rgv[i].y, m_rcHitRect.top);
		m_rcHitRect.bottom = max(m_rgv[i].y, m_rcHitRect.bottom);
		m_rcHitRect.zlow = min(m_rgv[i].z, m_rcHitRect.zlow);
		m_rcHitRect.zhigh = max(m_rgv[i].z, m_rcHitRect.zhigh);
	}
}

HitTriangle::HitTriangle(const Vertex3Ds rgv[3]) //!! triangle soup instead
{
	m_rgv[0] = rgv[0];
	m_rgv[1] = rgv[1];
	m_rgv[2] = rgv[2];

	const Vertex3Ds e0 = m_rgv[2] - m_rgv[0];
	const Vertex3Ds e1 = m_rgv[1] - m_rgv[0];
	normal = CrossProduct(e0,e1);
    normal.NormalizeSafe();

	m_fVisible = fFalse;
	m_elasticity = 0.3f;
    SetFriction(0.0f);
	m_scatter = 0;
}

float HitTriangle::HitTest(const Ball * pball, float dtime, CollisionEvent& coll)
{
	if (!m_fEnabled) return -1.0f;

    const float bnv = normal.Dot(pball->vel);       //speed in Normal-vector direction

	if (bnv >= C_CONTACTVEL)						// return if clearly ball is receding from object
		return -1.0f;

	// Point on the ball that will hit the polygon, if it hits at all
	const float bRadius = pball->radius;
    Vertex3Ds hitPos = pball->pos - bRadius * normal; // nearest point on ball ... projected radius along norm

    const float bnd = normal.Dot( hitPos - m_rgv[0] ); // distance from plane to ball

	float hittime;

    if (bnd < (float)(-PHYS_SKIN))
        return -1.0f;	// (ball normal distance) excessive pentratration of object skin ... no collision HACK

    bool isContact = false;

    if (bnd <= (float)PHYS_TOUCH)
    {
        if (fabsf(bnv) <= C_CONTACTVEL)
        {
            hittime = 0;
            isContact = true;
        }
        else if (bnd <= 0)
            hittime = 0;                            // zero time for rigid fast bodies
        else
            hittime = bnd / -bnv;
    }
    else if (fabsf(bnv) > C_LOWNORMVEL )					// not velocity low?
        hittime = bnd / -bnv;								// rate ok for safe divide 
    else
        return -1.0f;										// wait for touching

	if (infNaN(hittime) || hittime < 0 || hittime > dtime)
        return -1.0f;	// time is outside this frame ... no collision

    hitPos += hittime * pball->vel;	// advance hit point to contact

    // check if hitPos is within the triangle

    // Compute vectors
    const Vertex3Ds v0 = m_rgv[2] - m_rgv[0];
    const Vertex3Ds v1 = m_rgv[1] - m_rgv[0];
    const Vertex3Ds v2 = hitPos   - m_rgv[0];

    // Compute dot products
    const float dot00 = v0.Dot(v0);
    const float dot01 = v0.Dot(v1);
    const float dot02 = v0.Dot(v2);
    const float dot11 = v1.Dot(v1);
    const float dot12 = v1.Dot(v2);

    // Compute barycentric coordinates
    const float invDenom = 1.0f / (dot00 * dot11 - dot01 * dot01);
    const float u = (dot11 * dot02 - dot01 * dot12) * invDenom;
    const float v = (dot00 * dot12 - dot01 * dot02) * invDenom;

    // Check if point is in triangle
    const bool pointInTri = (u >= 0.f) && (v >= 0.f) && (u + v <= 1.f);

	if (pointInTri)
	{
		coll.normal[0] = normal;

		coll.distance = bnd;				// 3dhit actual contact distance ... 
		coll.hitRigid = true;				// collision type

        if (isContact)
        {
            coll.isContact = true;
            coll.normal[1].z = bnv;
        }

		return hittime;
	}
    else
        return -1.0f;
}

void HitTriangle::Collide(CollisionEvent* coll)
{
    Ball *pball = coll->ball;
    const Vertex3Ds& hitnormal = coll->normal[0];

    const float dot = hitnormal.Dot(pball->vel);

    pball->Collide3DWall(normal, m_elasticity, /*m_friction*/ 0.3f, m_scatter);

    if (m_ObjType == ePrimitive && dot <= -m_threshold)
        FireHitEvent(pball);
}

void HitTriangle::Contact(CollisionEvent& coll, float dtime)
{
    coll.ball->HandleStaticContact(coll.normal[0], coll.normal[1].z, /*m_friction*/ 0.3f, dtime);
}

void HitTriangle::CalcHitRect()
{
	m_rcHitRect.left = min(m_rgv[0].x, min(m_rgv[1].x,m_rgv[2].x));
	m_rcHitRect.right = max(m_rgv[0].x, max(m_rgv[1].x,m_rgv[2].x));
	m_rcHitRect.top = min(m_rgv[0].y, min(m_rgv[1].y,m_rgv[2].y));
	m_rcHitRect.bottom = max(m_rgv[0].y, max(m_rgv[1].y,m_rgv[2].y));
	m_rcHitRect.zlow = min(m_rgv[0].z, min(m_rgv[1].z,m_rgv[2].z));
	m_rcHitRect.zhigh = max(m_rgv[0].z, max(m_rgv[1].z,m_rgv[2].z));
}


////////////////////////////////////////////////////////////////////////////////


HitPlane::HitPlane(const Vertex3Ds& normal_, float d_)
    : normal(normal_), d(d_)
{
    m_elasticity = 0.2f;
}

float HitPlane::HitTest(const Ball * pball, float dtime, CollisionEvent& coll)
{
    if (!m_fEnabled) return -1.0f;

    //slintf("HitPlane test - %f %f\n", pball->pos.z, pball->vel.z);

    const float bnv = normal.Dot(pball->vel);       // speed in normal direction

    if (bnv >= (float)C_CONTACTVEL)                 // return if clearly ball is receding from object
        return -1.0f;

    const float bnd = normal.Dot( pball->pos ) - pball->radius - d; // distance from plane to ball surface

    if (bnd < (float)(-PHYS_SKIN))
        return -1.0f;   // excessive penetration of plane ... no collision HACK

    // slow moving ball? then either contact or no collision at all
    if (fabsf(bnv) <= (float)C_CONTACTVEL)
    {
        if (fabsf(bnd) <= (float)PHYS_TOUCH)
        {
            coll.isContact = true;
            coll.normal[0] = normal;
            coll.normal[1].z = bnv;     // remember original normal velocity
            coll.distance = bnd;
            coll.hitRigid = true;
            return 0.0f;    // hittime is ignored for contacts
        }
        else
            return -1.0f;   // large distance, small velocity -> no hit
    }

    float hittime = bnd / (-bnv);                   // rate ok for safe divide
    if (hittime < 0)
        hittime = 0.0f;     // already penetrating? then collide immediately

    if (infNaN(hittime) || hittime > dtime)
        return -1.0f;       // time is outside this frame ... no collision

    coll.normal[0] = normal;
    coll.distance = bnd;                // actual contact distance
    coll.hitRigid = true;               // collision type

    return hittime;
}

void HitPlane::Collide(CollisionEvent* coll)
{
    //slintf("Playfield COLLISION - (%f %f %f) - (%f %f %f)\n",
    //        coll->ball->pos.x, coll->ball->pos.y, coll->ball->pos.z,
    //        coll->ball->vel.x, coll->ball->vel.y, coll->ball->vel.z);
    coll->ball->Collide3DWall(coll->normal[0], m_elasticity, m_friction, /*m_scatter*/ 0.0f);

    // if ball has penetrated, push it out of the plane
    const float bnd = normal.Dot( coll->ball->pos ) - coll->ball->radius - d; // distance from plane to ball surface
    if (bnd < 0.f)
        coll->ball->pos -= bnd * normal;
}

void HitPlane::Contact(CollisionEvent& coll, float dtime)
{
    coll.ball->HandleStaticContact(coll.normal[0], coll.normal[1].z, m_friction, dtime);
}

////////////////////////////////////////////////////////////////////////////////


HitLine3D::HitLine3D(const Vertex3Ds& v1, const Vertex3Ds& v2)
{
    CacheHitTransform(v1, v2);

    m_rcHitRect.left = min(v1.x, v2.x);
    m_rcHitRect.right = max(v1.x, v2.x);
    m_rcHitRect.top = min(v1.y, v2.y);
    m_rcHitRect.bottom = max(v1.y, v2.y);
    m_rcHitRect.zlow = min(v1.z, v2.z);
    m_rcHitRect.zhigh = max(v1.z, v2.z);
}

void HitLine3D::CacheHitTransform(const Vertex3Ds& v1, const Vertex3Ds& v2)
{
	Vertex3Ds vLine = v2 - v1;
	vLine.Normalize();

	// Axis of rotation to make 3D cylinder a cylinder along the z-axis
	Vertex3Ds transaxis;
	/*const Vertex3Ds vup(0,0,1.0f);
	CrossProduct(vLine, vup, &transaxis);*/
	transaxis.x =  vLine.y;
	transaxis.y = -vLine.x;
	transaxis.z = 0.0f;

    if (transaxis.LengthSquared() <= 1e-6f)     // line already points in z axis?
        transaxis.Set(1, 0, 0);                 // choose arbitrary rotation vector
    else
        transaxis.Normalize();

	// Angle to rotate the line into the z-axis
	const float dot = vLine.z; //vLine.Dot(&vup);

	const float transangle = acosf(dot);

    matTrans.RotationAroundAxis(transaxis, -transangle);

    const Vertex3Ds vtrans1 = matTrans * v1;
    const Vertex3Ds vtrans2 = matTrans * v2;

    // set up HitLineZ parameters
    m_xy.x = vtrans1.x;
    m_xy.y = vtrans1.y;
    m_zlow = min(vtrans1.z, vtrans2.z);
    m_zhigh = max(vtrans1.z, vtrans2.z);
}

float HitLine3D::HitTest(const Ball * pball, float dtime, CollisionEvent& coll)
{
	if (!m_fEnabled)
		return -1.0f;

    // transform ball to cylinder coordinate system
    Ball ballT = *pball;
    ballT.pos = matTrans * ballT.pos;
    ballT.vel = matTrans * ballT.vel;

    const float hittime = HitLineZ::HitTest(&ballT, dtime, coll);

	if (hittime >= 0)       // transform hit normal back to world coordinate system
		coll.normal[0] = matTrans.MultiplyVectorT(coll.normal[0]);

	return hittime;
}

void HitLine3D::Collide(CollisionEvent* coll)
{
    Ball *pball = coll->ball;
    const Vertex3Ds& hitnormal = coll->normal[0];

    const float dot = hitnormal.Dot(pball->vel);
    pball->Collide3DWall(hitnormal, m_elasticity, /*m_friction*/ 0.3f, m_scatter);

    if (m_ObjType == ePrimitive && dot <= -m_threshold)
        FireHitEvent(pball);
}

void HitLine3D::CalcHitRect()
{
    // already done in constructor
}





// this function is called every frame to see if the object needs updating
//
void DispReelAnimObject::Check3D()
{
    // update the reels animation (returns saying weither to redraw the frame object or not)
    /*m_fInvalid =*/ m_pDispReel->RenderAnimation();
}

void DispReelAnimObject::Reset()
{
}

// this function is called every frame to see if the object needs updating
//
void LightSeqAnimObject::Check3D()
{
    // update the reels animation (returns saying weither to redraw the frame object or not)
    /*m_fInvalid =*/ m_pLightSeq->RenderAnimation();
}


float TriggerLineSeg::HitTest(const Ball * pball, float dtime, CollisionEvent& coll) //rlc problem-5
{
	if (!m_ptrigger->m_hitEnabled) return -1.0f;				//rlc custom shape trigger
	
    // approach either face, not lateral-rolling point (assume center), not a rigid body contact
	return this->HitTestBasic(pball, dtime, coll, false,false, false);
}

void TriggerLineSeg::Collide(CollisionEvent* coll)
{
    Ball *pball = coll->ball;

	if((m_ObjType != eTrigger) ||
	   (!pball->m_vpVolObjs)) return;

	const int i = pball->m_vpVolObjs->IndexOf(m_pObj); // if -1 then not in objects volume set (i.e not already hit)

	if ((coll->normal[1].x < 1.0f) == (i < 0))	// Hit == NotAlreadyHit
	{		
        pball->pos += STATICTIME * pball->vel;  //move ball slightly forward
		
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


float TriggerHitCircle::HitTest(const Ball * pball, float dtime, CollisionEvent& coll)
{
	return HitTestBasicRadius(pball, dtime, coll, false, false, false); //any face, not-lateral, non-rigid
}

void TriggerHitCircle::Collide(CollisionEvent* coll)
{
    Ball *pball = coll->ball;

	if((m_ObjType < eTrigger) || // triggers and kickers
	   (!pball->m_vpVolObjs)) return;

	const int i = pball->m_vpVolObjs->IndexOf(m_pObj); // if -1 then not in objects volume set (i.e not already hit)

	if ((coll->normal[1].x < 1.0f) == (i < 0))		   // Hit == NotAlreadyHit
	{
        pball->pos += STATICTIME * pball->vel;		   //move ball slightly forward

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
