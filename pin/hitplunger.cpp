#include "stdafx.h"

#define PLUNGERHEIGHT 50


extern U32 LastPlungerHit;


HitPlunger::HitPlunger(float x, float y, float x2, float pos, float zheight,Plunger* pPlunger)
		{
		m_plungeranim.m_plunger = pPlunger;
		m_plungeranim.m_x = x;
		m_plungeranim.m_x2 = x2;
		m_plungeranim.m_y = y;

		m_plungeranim.m_pos = pos;
		m_plungeranim.m_posdesired = pos;
		m_plungeranim.m_posFrame =  pos;

		
		m_plungeranim.recock = false;
		m_plungeranim.err_fil = 0;	// integrate error over multiple update periods 

		m_plungeranim.m_force = 0;
		m_plungeranim.m_mechTimeOut = 0;	//must be zero at game start, scripts will cock button plungers
		
		m_plungeranim.m_mass = 30;
		m_plungeranim.m_fAcc = fFalse;
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

		m_plungeranim.m_jointBase[0].m_rcHitRect.zlow = zheight;
		m_plungeranim.m_jointBase[0].m_rcHitRect.zhigh = zheight+PLUNGERHEIGHT;
		m_plungeranim.m_jointBase[1].m_rcHitRect.zlow = zheight;
		m_plungeranim.m_jointBase[1].m_rcHitRect.zhigh = zheight+PLUNGERHEIGHT;
		m_plungeranim.m_jointEnd[0].m_rcHitRect.zlow = zheight;
		m_plungeranim.m_jointEnd[0].m_rcHitRect.zhigh = zheight+PLUNGERHEIGHT;
		m_plungeranim.m_jointEnd[1].m_rcHitRect.zlow = zheight;
		m_plungeranim.m_jointEnd[1].m_rcHitRect.zhigh = zheight+PLUNGERHEIGHT;
		m_plungeranim.m_jointBase[0].zlow = zheight;
		m_plungeranim.m_jointBase[0].zhigh = zheight+PLUNGERHEIGHT;
		m_plungeranim.m_jointBase[1].zlow = zheight;
		m_plungeranim.m_jointBase[1].zhigh = zheight+PLUNGERHEIGHT;
		m_plungeranim.m_jointEnd[0].zlow = zheight;
		m_plungeranim.m_jointEnd[0].zhigh = zheight+PLUNGERHEIGHT;
		m_plungeranim.m_jointEnd[1].zlow = zheight;
		m_plungeranim.m_jointEnd[1].zhigh = zheight+PLUNGERHEIGHT;

		m_plungeranim.SetObjects(m_plungeranim.m_pos);

		m_plungeranim.m_iframe = -1;
		}

void HitPlunger::CalcHitRect()
	{
	// Allow roundoff
	m_rcHitRect.left = m_plungeranim.m_x - 0.1f;
	m_rcHitRect.right = m_plungeranim.m_x2 + 0.1f;
	m_rcHitRect.top = m_plungeranim.m_frameEnd - 0.1f;
	m_rcHitRect.bottom = m_plungeranim.m_y + 0.1f;
	// z stuff gets set in constructor
	//m_rcHitRect.zlow = 0;
	//m_rcHitRect.zhigh = 50;
	}

void PlungerAnimObject::SetObjects(float len)
	{
	m_linesegBase.v1.x = m_x;
	m_linesegBase.v1.y = m_y;
	m_linesegBase.v2.x = m_x2;
	m_linesegBase.v2.y = m_y;// + 0.0001f;

	m_jointBase[0].center.x = m_x;
	m_jointBase[0].center.y = m_y;
	m_jointBase[1].center.x = m_x2;
	m_jointBase[1].center.y = m_y;// + 0.0001f;
	
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
	
	m_jointEnd[0].center.x = m_x;
	m_jointEnd[0].center.y = len;
	m_jointEnd[1].center.x = m_x2;
	m_jointEnd[1].center.y = len;// + 0.0001f;

	m_linesegBase.CalcNormal();
	m_linesegEnd.CalcNormal();

	m_linesegSide[0].CalcNormal();
	m_linesegSide[1].CalcNormal();

	float deg45 = (float)sin(M_PI/4.0);

	m_jointBase[0].normal.x = -deg45;
	m_jointBase[0].normal.y = deg45;
	m_jointBase[1].normal.x = deg45;
	m_jointBase[1].normal.y = deg45;

	m_jointEnd[0].normal.x = -deg45;
	m_jointEnd[0].normal.y = -deg45;
	m_jointEnd[1].normal.x = deg45;
	m_jointEnd[1].normal.y = -deg45;
	}

void PlungerAnimObject::UpdateDisplacements(PINFLOAT dtime)
	{
	m_pos += (float)dtime*m_speed;

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
		m_pos = m_frameStart;		// using either control method		
		}

	SetObjects(m_pos);

	// Used to be in updatetimepermanent

	m_posFrame = m_pos;

	if (m_fAcc)
		{
		if (m_pos == m_posdesired)
			{
			m_fAcc = fFalse;
			m_speed = 0;
			}		
		}
	}

void PlungerAnimObject::UpdateVelocities(PINFLOAT dtime)//dtime always 1.0f
	{	
	if (m_fAcc)
		{
		m_speed += (m_force/m_mass);//*(float)dtime;		
		}
	else if (!m_plunger->m_d.m_mechPlunger)	m_mechTimeOut = 0;// disallow mechanical plunger control
	else {	
		if(m_posdesired == m_frameEnd) // mechanical plunger position ...make sure button control is idle
			{	
			float mech_pos  = mechPlunger();										// mechanical position
			float cur_pos = (m_frameEnd - m_pos)/(m_frameEnd - m_frameStart);		// VP plunger position
			float error = mech_pos - cur_pos;										// error

			err_fil = error + err_fil * 0.60f;

			if (!recock && m_mechTimeOut <= 0)
				{
				m_speed = -err_fil*(m_frameEnd - m_frameStart);// /dtime ;				//new velocity based on filtered position error
				m_speed *= m_plunger->m_d.m_mechStrength/m_mass *c_plungerNormalize;	// match button physics

				if (m_speed <= -m_breakOverVelocity)						//mechinical speed is too fast, windows will
					{														// will alias at high speed and EOS 
					m_force = -m_plunger->m_d.m_mechStrength;				// set to mechnical plunger force
					m_posdesired = m_frameEnd;								// set to button controller 
					m_fAcc = fTrue;											// enable animation
					m_mechTimeOut = 5;										// stay any other actions until completed
					}
				}															// no interaction with mech plunger
			}																// oscillations, bounce, etc.
		else if(m_posdesired == m_frameStart)								//button plunger is fully cocked when parked
			{	
			static float last_pos = 0;										// assume initial uncocked

			float mech_pos  = mechPlunger();								// mechanical position
			float diff = mech_pos - last_pos;								// change
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
					m_fAcc = fTrue;											// enable animation
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
				m_fAcc = fTrue;					// enable animation
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


PINFLOAT HitPlunger::HitTest(Ball *pball, PINFLOAT dtime, Vertex3Ds *phitnormal)
	{
	float newtime;
	float hittime = dtime; //start time
	BOOL fHit = fFalse;
	Ball BallT = *pball;	

	// If we got here, then the ball is close enough to the plunger
	// to where we should animate the button's light.
	// Save the time so we can tell the button when to turn on/off.  
	LastPlungerHit = msec();

	// We are close enable the plunger light.
	Vertex3Ds hitnormal[5];
	newtime = m_plungeranim.m_linesegBase.HitTest(&BallT, dtime, &hitnormal[0]);
	if (newtime >= 0 && newtime <= hittime)
		{
		fHit = fTrue;
		hittime = newtime;
		phitnormal[0] = hitnormal[0];	
		pball->m_HitDist = BallT.m_HitDist;
		pball->m_HitNormVel = BallT.m_HitNormVel;
		pball->m_HitRigid = true;
		phitnormal[1].x = 0;
		phitnormal[1].y = 0;
		}

	for (int i=0;i<2;i++)
		{
		newtime = m_plungeranim.m_linesegSide[i].HitTest(&BallT, hittime, &hitnormal[0]);
		if (newtime >= 0 && newtime <= hittime)
			{
			fHit = fTrue;
			hittime = newtime;
			phitnormal[0] = hitnormal[0];
			pball->m_HitDist = BallT.m_HitDist;
			pball->m_HitNormVel = BallT.m_HitNormVel;
			pball->m_HitRigid = true;

			phitnormal[1].x = 0;
			phitnormal[1].y = 0;
			}

		newtime = m_plungeranim.m_jointBase[i].HitTest(&BallT, hittime, &hitnormal[0]);
		if (newtime >= 0 && newtime <= hittime)
			{
			fHit = fTrue;
			hittime = newtime;
			phitnormal[0] = hitnormal[0];
			pball->m_HitDist = BallT.m_HitDist;
			pball->m_HitNormVel = BallT.m_HitNormVel;
			pball->m_HitRigid = true;
			phitnormal[1].x = 0;
			phitnormal[1].y = 0;
			}
		}

	const float deltay = m_plungeranim.m_speed;
	
	BallT.vy -= deltay;

	newtime = m_plungeranim.m_linesegEnd.HitTest(&BallT, hittime, &hitnormal[0]);
	if (newtime >= 0 && newtime <= hittime)
		{
		fHit = fTrue;
		hittime = newtime;

		phitnormal[0] = hitnormal[0];
		pball->m_HitDist = BallT.m_HitDist;
		pball->m_HitNormVel = BallT.m_HitNormVel;
		pball->m_HitRigid = true;
		phitnormal[1].x = 0;
		phitnormal[1].y = deltay;	 //m_speed;		//>>> changed by chris
		}

	for (int i=0;i<2;i++)
		{
		newtime = m_plungeranim.m_jointEnd[i].HitTest(&BallT, hittime, &hitnormal[0]);
		if (newtime >= 0 && newtime <= hittime)
			{
			fHit = fTrue;
			hittime = newtime;

			phitnormal[0] = hitnormal[0];
			pball->m_HitDist = BallT.m_HitDist;
			pball->m_HitNormVel = BallT.m_HitNormVel;
			pball->m_HitRigid = true;
			phitnormal[1].x = 0;
			phitnormal[1].y = deltay;	 //m_speed;		//>>> changed by chris
			}
		}

	return fHit ? hittime : -1.0f;
	}

void HitPlunger::Draw(HDC hdc)
	{
	m_plungeranim.m_linesegBase.Draw(hdc);
	m_plungeranim.m_linesegEnd.Draw(hdc);
	
	for (int i=0;i<2;i++)
		{
		m_plungeranim.m_linesegSide[i].Draw(hdc);
		m_plungeranim.m_jointBase[i].Draw(hdc);
		m_plungeranim.m_jointEnd[i].Draw(hdc);
		}
	}

void HitPlunger::Collide(Ball *pball, Vertex3Ds *phitnormal)
	{
	float dot = (pball->vx - phitnormal[1].x)* phitnormal->x + (pball->vy - phitnormal[1].y) * phitnormal->y;

	if (dot >= -C_LOWNORMVEL )								// nearly receding ... make sure of conditions
		{													// otherwise if clearly approaching .. process the collision
		if (dot > C_LOWNORMVEL) return;						//is this velocity clearly receding (i.e must > a minimum)		
#ifdef C_EMBEDDED
		if (pball->m_HitDist < -C_EMBEDDED)
			dot = -C_EMBEDSHOT;		// has ball become embedded???, give it a kick
		else return;
#endif
		}
		
#ifdef C_DISP_GAIN 
		// correct displacements, mostly from low velocity blidness, an alternative to true acceleration processing	
		float hdist = -C_DISP_GAIN * pball->m_HitDist;				// distance found in hit detection
		if (hdist > 1.0e-4f)
			{													// maginitude of jump
			if (hdist > C_DISP_LIMIT) 
				{hdist = C_DISP_LIMIT;}		// crossing ramps, delta noise
			pball->x += hdist * phitnormal->x;					// push along norm, back to free area
			pball->y += hdist * phitnormal->y;					// use the norm, but is not correct
			}
#endif
			

	const float impulse = -(dot * 1.45f/(1.0f+1.0f/m_plungeranim.m_mass));

	pball->vx += impulse *phitnormal->x;  
	pball->vy += impulse *phitnormal->y; 

	pball->vx *= c_hardFriction; pball->vy *= c_hardFriction; pball->vz *= c_hardFriction; //friction all axiz
		
	const float scatter_vel = m_plungeranim.m_scatterVelocity * g_pplayer->m_ptable->m_globalDifficulty;// apply dificulty weighting

	if ( scatter_vel > 0  && fabsf(pball->vy) > scatter_vel) //skip if low velocity 
		{
		float scatter = 2.0f* ((float)rand()*(float)(1.0/RAND_MAX) - 0.5f);  // -1.0f..1.0f
		scatter *= (1.0f - scatter*scatter)*2.59808f * scatter_vel;	// shape quadratic distribution and scale
		pball->vy += scatter;
		}

	pball->m_fDynamic = C_DYNAMIC;

	Vertex3Ds vnormal;
	vnormal.Set(phitnormal->x, phitnormal->y, 0);
	pball->AngularAcceleration(&vnormal);	
	}

void PlungerAnimObject::Check3D()
	{
																		//rlc fixed frame jitter by rounding up 0.5
	const int frame = (int)((m_pos - m_frameStart + 1.0f)/(m_frameEnd-m_frameStart) * (m_vddsFrame.Size()-1)+0.5f);

	if (frame != m_iframe)
		{
		m_iframe = frame;
		m_fInvalid = fTrue;
		}
	}

ObjFrame *PlungerAnimObject::Draw3D(RECT *prc)
	{
	if (m_iframe == -1) return NULL;

	ObjFrame * const pobjframe = m_vddsFrame.ElementAt(m_iframe);

	return pobjframe;
	}
