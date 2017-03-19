#include "stdafx.h"

#define PLUNGERHEIGHT 50.0f

HitPlunger::HitPlunger(const float x, const float y, const float x2, const float pos, const float zheight, Plunger * const pPlunger)
		{
		m_plungeranim.m_plunger = pPlunger;
		m_plungeranim.m_x = x;
		m_plungeranim.m_x2 = x2;
		m_plungeranim.m_y = y;

		m_plungeranim.m_pos = pos;
		m_plungeranim.m_posdesired = pos;

		
		m_plungeranim.recock = false;
		m_plungeranim.err_fil = 0;	// integrate error over multiple update periods 

		m_plungeranim.m_force = 0;
		m_plungeranim.m_mechTimeOut = 0;	//must be zero at game start, scripts will cock button plungers
		
		m_plungeranim.m_mass = 30.0f;
		m_plungeranim.m_fAcc = false;
		m_plungeranim.m_speed = 0;
		m_plungeranim.m_parkPosition = pPlunger->m_d.m_parkPosition;
		m_plungeranim.m_scatterVelocity = pPlunger->m_d.m_scatterVelocity;
		m_plungeranim.m_breakOverVelocity = pPlunger->m_d.m_breakOverVelocity;

		m_plungeranim.m_linesegBase.m_pfe = NULL;
		m_plungeranim.m_jointBase[0].m_pfe = NULL;
		m_plungeranim.m_jointBase[1].m_pfe = NULL;
		m_plungeranim.m_linesegSide[0].m_pfe = NULL;
		m_plungeranim.m_linesegSide[1].m_pfe = NULL;
		m_plungeranim.m_linesegEnd.m_pfe = NULL;
		m_plungeranim.m_jointEnd[0].m_pfe = NULL;
		m_plungeranim.m_jointEnd[1].m_pfe = NULL;

		m_rcHitRect.zlow = zheight;
		m_rcHitRect.zhigh = zheight+PLUNGERHEIGHT;

		m_plungeranim.m_linesegBase.m_rcHitRect.zlow = zheight;
		m_plungeranim.m_linesegBase.m_rcHitRect.zhigh = zheight+PLUNGERHEIGHT;
		m_plungeranim.m_linesegSide[0].m_rcHitRect.zlow = zheight;
		m_plungeranim.m_linesegSide[0].m_rcHitRect.zhigh = zheight+PLUNGERHEIGHT;
		m_plungeranim.m_linesegSide[1].m_rcHitRect.zlow = zheight;
		m_plungeranim.m_linesegSide[1].m_rcHitRect.zhigh = zheight+PLUNGERHEIGHT;
		m_plungeranim.m_linesegEnd.m_rcHitRect.zlow = zheight;
		m_plungeranim.m_linesegEnd.m_rcHitRect.zhigh = zheight+PLUNGERHEIGHT;

		m_plungeranim.m_jointBase[0].m_zlow = zheight;
		m_plungeranim.m_jointBase[0].m_zhigh = zheight+PLUNGERHEIGHT;
		m_plungeranim.m_jointBase[1].m_zlow = zheight;
		m_plungeranim.m_jointBase[1].m_zhigh = zheight+PLUNGERHEIGHT;
		m_plungeranim.m_jointEnd[0].m_zlow = zheight;
		m_plungeranim.m_jointEnd[0].m_zhigh = zheight+PLUNGERHEIGHT;
		m_plungeranim.m_jointEnd[1].m_zlow = zheight;
		m_plungeranim.m_jointEnd[1].m_zhigh = zheight+PLUNGERHEIGHT;

		m_plungeranim.SetObjects(m_plungeranim.m_pos);
		}

void HitPlunger::CalcHitRect()
	{
	// Allow roundoff
	m_rcHitRect.left   = m_plungeranim.m_x - 0.1f;
	m_rcHitRect.right  = m_plungeranim.m_x2 + 0.1f;
	m_rcHitRect.top    = m_plungeranim.m_frameEnd - 0.1f;
	m_rcHitRect.bottom = m_plungeranim.m_y + 0.1f;
	// z stuff gets set in constructor
	//m_rcHitRect.zlow = 0;
	//m_rcHitRect.zhigh = 50;
	}

void PlungerAnimObject::SetObjects(const float len)
	{
	m_linesegBase.v1.x = m_x;
	m_linesegBase.v1.y = m_y;
	m_linesegBase.v2.x = m_x2;
	m_linesegBase.v2.y = m_y;// + 0.0001f;

	m_jointBase[0].m_xy.x = m_x;
	m_jointBase[0].m_xy.y = m_y;
	m_jointBase[1].m_xy.x = m_x2;
	m_jointBase[1].m_xy.y = m_y;// + 0.0001f;
	
	m_linesegSide[0].v2.x = m_x;
	m_linesegSide[0].v2.y = m_y;
	m_linesegSide[0].v1.x = m_x + 0.0001f;
	m_linesegSide[0].v1.y = len;

	m_linesegSide[1].v1.x = m_x2;
	m_linesegSide[1].v1.y = m_y;
	m_linesegSide[1].v2.x = m_x2 + 0.0001f;
	m_linesegSide[1].v2.y = len;

	m_linesegEnd.v2.x = m_x;
	m_linesegEnd.v2.y = len;
	m_linesegEnd.v1.x = m_x2;
	m_linesegEnd.v1.y = len;// + 0.0001f;
	
	m_jointEnd[0].m_xy.x = m_x;
	m_jointEnd[0].m_xy.y = len;
	m_jointEnd[1].m_xy.x = m_x2;
	m_jointEnd[1].m_xy.y = len;// + 0.0001f;

	m_linesegBase.CalcNormal();
	m_linesegEnd.CalcNormal();

	m_linesegSide[0].CalcNormal();
	m_linesegSide[1].CalcNormal();
	}

void PlungerAnimObject::UpdateDisplacements(const float dtime)
{
	m_pos += dtime*m_speed;

	if (m_pos < m_frameEnd)	
		{		
		m_plunger->FireVoidEventParm(DISPID_LimitEvents_EOS, fabsf(m_speed));	// send EOS event
		m_speed = 0;
		m_pos = m_frameEnd;						// hard limit plunger position range
		m_mechTimeOut = 0;						// stroke complete ...then clear timer
		}
	else if (m_pos > m_frameStart)
		{
		m_plunger->FireVoidEventParm(DISPID_LimitEvents_BOS, fabsf(m_speed));	// send Park event		
		m_speed = 0;
		m_pos = m_frameStart;					// using either control method		
		}

	SetObjects(m_pos);

	if (m_fAcc && (m_pos == m_posdesired))
		{
		m_fAcc = false;
		m_speed = 0;
		}
}

void PlungerAnimObject::UpdateVelocities()
	{	
	if (m_fAcc)
		{
		m_speed += PHYS_FACTOR * (m_force/m_mass);
		}
	else if (!m_plunger->m_d.m_mechPlunger)
        m_mechTimeOut = 0;// disallow mechanical plunger control
	else {	
		if(m_posdesired == m_frameEnd) // mechanical plunger position ...make sure button control is idle
			{	
			const float mech_pos = mechPlunger();										// mechanical position
			const float cur_pos = (m_frameEnd - m_pos)/(m_frameEnd - m_frameStart);		// VP plunger position
			const float error = mech_pos - cur_pos;										// error

			err_fil = error + err_fil * 0.60f;

			if (!recock && m_mechTimeOut <= 0)
				{
				m_speed = -err_fil*(m_frameEnd - m_frameStart);				//new velocity based on filtered position error
				m_speed *= m_plunger->m_d.m_mechStrength/m_mass *c_plungerNormalize;	// match button physics

				if (m_speed <= -m_breakOverVelocity)						//mechanical speed is too fast, windows will
					{														// alias at high speed and EOS 
					m_force = -m_plunger->m_d.m_mechStrength;				// set to mechnical plunger force
					m_posdesired = m_frameEnd;								// set to button controller 
					m_fAcc = true;											// enable animation
					m_mechTimeOut = 5;										// stay any other actions until completed
					}
				}															// no interaction with mech plunger
			}																// oscillations, bounce, etc.
		else if(m_posdesired == m_frameStart)								//button plunger is fully cocked when parked
			{	
			static float last_pos = 0;										// assume initial uncocked

			const float mech_pos = mechPlunger();							// mechanical position
			const float diff = mech_pos - last_pos;							// change
			last_pos = mech_pos;											//remember last position			
			err_fil = 0;
			if (!recock && m_mechTimeOut <= 0)
				{
				if (diff <= -0.20f)											// modest release velocity
					{	
					recock = true;											// need to recock later
					m_speed = 0;											// starting from fully cocked
					m_force = -m_plunger->m_d.m_mechStrength;				// set to mechnical plunger force
					m_posdesired = m_frameEnd;								// set to button controller 
					m_fAcc = true;											// enable animation
					m_mechTimeOut = 15;										// stay any other actions until completed
					if (g_pplayer)
						{
						g_pplayer->m_ptable->FireKeyEvent(DISPID_GameEvents_KeyDown
														, g_pplayer->m_rgKeys[ePlungerKey]);
						}
					}
				}
			}
		else err_fil = 0;

		if (m_mechTimeOut > 0) --m_mechTimeOut;
		else {
			if (recock || (!m_fAcc && m_posdesired == m_frameStart && m_pos != m_frameStart))
				{
				m_speed = 0;					// quench any motion
				m_force = 10;					// retract plunger slowly .... v about 1
				m_posdesired = m_frameStart;	// return to the cocked position
				m_fAcc = true;					// enable animation
				m_mechTimeOut = 15;				// stay any other actions until completed					
				if (recock && g_pplayer)
					{
					g_pplayer->m_ptable->FireKeyEvent(DISPID_GameEvents_KeyUp, g_pplayer->m_rgKeys[ePlungerKey]); 
					}
				recock = false;					// clear recock	
				}
			}
		}
	}	


float HitPlunger::HitTest(const Ball * pball, float dtime, CollisionEvent& coll)
	{
	float hittime = dtime; //start time
	bool fHit = false;
	Ball BallT = *pball;	

	// If we got here, then the ball is close enough to the plunger
	// to where we should animate the button's light.
	// Save the time so we can tell the button when to turn on/off.  
	g_pplayer->m_LastPlungerHit = g_pplayer->m_time_msec;

	// We are close enable the plunger light.
	CollisionEvent hit;
    float newtime;

	newtime = m_plungeranim.m_linesegBase.HitTest(&BallT, dtime, hit);
	if (newtime >= 0 && newtime <= hittime)
		{
		fHit = true;
		hittime = newtime;
        coll = hit;
		coll.normal[1].x = 0;
		coll.normal[1].y = 0;
		}

	for (int i=0;i<2;i++)
		{
		newtime = m_plungeranim.m_linesegSide[i].HitTest(&BallT, hittime, hit);
		if (newtime >= 0 && newtime <= hittime)
			{
			fHit = true;
			hittime = newtime;
            coll = hit;
			coll.normal[1].x = 0;
			coll.normal[1].y = 0;
			}

		newtime = m_plungeranim.m_jointBase[i].HitTest(&BallT, hittime, hit);
		if (newtime >= 0 && newtime <= hittime)
			{
			fHit = true;
			hittime = newtime;
            coll = hit;
			coll.normal[1].x = 0;
			coll.normal[1].y = 0;
			}
		}

	const float deltay = m_plungeranim.m_speed;
	
	BallT.vel.y -= deltay;

	newtime = m_plungeranim.m_linesegEnd.HitTest(&BallT, hittime, hit);
	if (newtime >= 0 && newtime <= hittime)
		{
		fHit = true;
		hittime = newtime;
        coll = hit;
		coll.normal[1].x = 0;
		coll.normal[1].y = deltay;	 //m_speed;		//>>> changed by chris
		}

	for (int i=0;i<2;i++)
		{
		newtime = m_plungeranim.m_jointEnd[i].HitTest(&BallT, hittime, hit);
		if (newtime >= 0 && newtime <= hittime)
			{
			fHit = true;
			hittime = newtime;
            coll = hit;
			coll.normal[1].x = 0;
			coll.normal[1].y = deltay;	 //m_speed;		//>>> changed by chris
			}
		}

    //coll.isContact = false;     // HACK: we get contacts from the LineSegs, but ignore them
	return fHit ? hittime : -1.0f;
	}

void HitPlunger::Collide(CollisionEvent *coll)
	{
    Ball *pball = coll->ball;
    Vertex3Ds *phitnormal = coll->normal;

	float dot = (pball->vel.x - phitnormal[1].x)* phitnormal->x + (pball->vel.y - phitnormal[1].y) * phitnormal->y;

	if (dot >= -C_LOWNORMVEL )								// nearly receding ... make sure of conditions
		{													// otherwise if clearly approaching .. process the collision
		if (dot > C_LOWNORMVEL) return;						// is this velocity clearly receding (i.e must > a minimum)		
#ifdef C_EMBEDDED
		if (coll->distance < -C_EMBEDDED)
			dot = -C_EMBEDSHOT;		// has ball become embedded???, give it a kick
		else return;
#endif
		}
		
#ifdef C_DISP_GAIN 
		// correct displacements, mostly from low velocity blidness, an alternative to true acceleration processing	
		float hdist = -C_DISP_GAIN * coll->distance;				// distance found in hit detection
		if (hdist > 1.0e-4f)
			{													// magnitude of jump
			if (hdist > C_DISP_LIMIT) 
				{hdist = C_DISP_LIMIT;}		// crossing ramps, delta noise
			pball->pos.x += hdist * phitnormal->x;					// push along norm, back to free area
			pball->pos.y += hdist * phitnormal->y;					// use the norm, but is not correct
			}
#endif
			

	const float impulse = dot * -1.45f/(1.0f+1.0f/m_plungeranim.m_mass);

	pball->vel.x += impulse *phitnormal->x;  
	pball->vel.y += impulse *phitnormal->y; 

	pball->vel *= 0.999f;           //friction all axiz     // TODO: fix this

	const float scatter_vel = m_plungeranim.m_scatterVelocity * g_pplayer->m_ptable->m_globalDifficulty;// apply dificulty weighting

	if (scatter_vel > 0 && fabsf(pball->vel.y) > scatter_vel) //skip if low velocity 
		{
		float scatter = rand_mt_m11();								// -1.0f..1.0f
		scatter *= (1.0f - scatter*scatter)*2.59808f * scatter_vel;	// shape quadratic distribution and scale
		pball->vel.y += scatter;
		}

	pball->m_fDynamic = C_DYNAMIC;

	const Vertex3Ds vnormal(phitnormal->x, phitnormal->y, 0.0f);
	}

void HitPlunger::Contact(CollisionEvent& coll, float dtime)
{
}
