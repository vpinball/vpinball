#include "stdafx.h"
#include "..\Main.h"

//#define MECH_PLUNGER 1

HitPlunger::HitPlunger(float x, float y, float x2, float pos, float zheight, float parkPosition, float scatterVelocity, float breakOverVelocity, float mechStrength,BOOL mechPlunger)
		{
		m_plungeranim.m_mechPlunger = mechPlunger;
		m_plungeranim.m_phitplunger = this;
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

		m_plungeranim.m_force = 0;
		//m_acc = 0;
		m_plungeranim.m_mass = 30;
		m_plungeranim.m_fAcc = fFalse;
		m_plungeranim.m_speed = 0;

		m_plungeranim.m_linesegBase.m_pfe = NULL;
		m_plungeranim.m_jointBase[0].m_pfe = NULL;
		m_plungeranim.m_jointBase[1].m_pfe = NULL;
		m_plungeranim.m_linesegSide[0].m_pfe = NULL;
		m_plungeranim.m_linesegSide[1].m_pfe = NULL;
		m_plungeranim.m_linesegEnd.m_pfe = NULL;
		m_plungeranim.m_jointEnd[0].m_pfe = NULL;
		m_plungeranim.m_jointEnd[1].m_pfe = NULL;

		m_rcHitRect.zlow = zheight;
		m_rcHitRect.zhigh = zheight+50;

		m_plungeranim.m_linesegBase.m_rcHitRect.zlow = zheight;
		m_plungeranim.m_linesegBase.m_rcHitRect.zhigh = zheight+50;
		m_plungeranim.m_linesegSide[0].m_rcHitRect.zlow = zheight;
		m_plungeranim.m_linesegSide[0].m_rcHitRect.zhigh = zheight+50;
		m_plungeranim.m_linesegSide[1].m_rcHitRect.zlow = zheight;
		m_plungeranim.m_linesegSide[1].m_rcHitRect.zhigh = zheight+50;
		m_plungeranim.m_linesegEnd.m_rcHitRect.zlow = zheight;
		m_plungeranim.m_linesegEnd.m_rcHitRect.zhigh = zheight+50;

		m_plungeranim.m_jointBase[0].m_rcHitRect.zlow = zheight;
		m_plungeranim.m_jointBase[0].m_rcHitRect.zhigh = zheight+50;
		m_plungeranim.m_jointBase[1].m_rcHitRect.zlow = zheight;
		m_plungeranim.m_jointBase[1].m_rcHitRect.zhigh = zheight+50;
		m_plungeranim.m_jointEnd[0].m_rcHitRect.zlow = zheight;
		m_plungeranim.m_jointEnd[0].m_rcHitRect.zhigh = zheight+50;
		m_plungeranim.m_jointEnd[1].m_rcHitRect.zlow = zheight;
		m_plungeranim.m_jointEnd[1].m_rcHitRect.zhigh = zheight+50;
		m_plungeranim.m_jointBase[0].zlow = zheight;
		m_plungeranim.m_jointBase[0].zhigh = zheight+50;
		m_plungeranim.m_jointBase[1].zlow = zheight;
		m_plungeranim.m_jointBase[1].zhigh = zheight+50;
		m_plungeranim.m_jointEnd[0].zlow = zheight;
		m_plungeranim.m_jointEnd[0].zhigh = zheight+50;
		m_plungeranim.m_jointEnd[1].zlow = zheight;
		m_plungeranim.m_jointEnd[1].zhigh = zheight+50;

		m_plungeranim.SetObjects(m_plungeranim.m_pos);

		m_plungeranim.m_iframe = -1;
		
		// Start New Physics
		
		m_plungeranim.m_mechPlunger = mechPlunger;
		m_plungeranim.m_parkPosition = parkPosition;
		m_plungeranim.m_scatterVelocity = scatterVelocity;
		m_plungeranim.m_breakOverVelocity = breakOverVelocity;
		m_plungeranim.m_mechStrength = mechStrength;

		// End New Physics
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

	float deg45 = (float)sin(PI/4);

	m_jointBase[0].normal.x = -deg45;
	m_jointBase[0].normal.y = deg45;
	m_jointBase[1].normal.x = deg45;
	m_jointBase[1].normal.y = deg45;

	m_jointEnd[0].normal.x = -deg45;
	m_jointEnd[0].normal.y = -deg45;
	m_jointEnd[1].normal.x = deg45;
	m_jointEnd[1].normal.y = -deg45;
	}

void PlungerAnimObject::UpdateTimeTemp(PINFLOAT dtime)
	{
	m_pos += (float)dtime*m_speed;
	
#if 0
	if (fOldPhys)
		{
		if ((m_speed < 0) && (m_pos < m_posdesired))
			{
			m_pos = m_posdesired;
			}
		else if ((m_speed > 0) && (m_pos > m_posdesired))
			{
			m_pos = m_posdesired;
			
			if (m_phitplunger->m_pplunger->m_d.m_autoPlunger == 1)
				{
				m_phitplunger->m_pplunger->PullBack();
				}
			}
		}
	else // new phys
#endif
		{
		if (m_pos < m_frameEnd)	
			{		
			//m_plunger->FireVoidEventParm(DISPID_LimitEvents_EOS, fabs(m_speed));	// send EOS event
			m_speed = 0;
			m_pos = m_frameEnd;						// hard limit plunger position range
			m_mechTimeOut = 0;						// stroke complete ...then clear timer
			
			if (m_phitplunger->m_pplunger->m_d.m_autoPlunger == 1)
				{
				m_phitplunger->m_pplunger->PullBack();
				}
			}
		else if (m_pos > m_frameStart)
			{
			//m_plunger->FireVoidEventParm(DISPID_LimitEvents_BOS, fabs(m_speed));	// send Park event		
			m_speed = 0;
			m_pos = m_frameStart;		// using either control method
			}
		}

	SetObjects(m_pos);

	// Used to be in updatetimepermanent

	m_posFrame = m_pos;
	if (m_fAcc)
		{
		if (m_pos == m_posdesired)
			{
#ifdef LOG
		fprintf(g_pplayer->m_flog, "//Plunger Back In Place %d\n", g_pplayer->m_timestamp);
#endif
			m_fAcc = fFalse;
			m_speed = 0;
			}
		//else
			//{
			//m_speed += (m_force/m_mass)*dtime;
			//}
		}
	}

void PlungerAnimObject::UpdateAcceleration(PINFLOAT dtime)
	{
	if (m_fAcc)
		{
			m_speed += (m_force/m_mass) *(float)dtime;		
		}
		else if (!m_mechPlunger) m_mechTimeOut = 0;// disallow mechanical plunger control
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
					m_speed *= m_mechStrength/m_mass *(float)c_plungerNormalize;	// match button physics

					if (m_speed <= -m_breakOverVelocity)						//mechinical speed is too fast, windows will
					{														// will alias at high speed and EOS 
						m_force = -m_mechStrength;							// set to mechnical plunger force
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
						m_force = -m_mechStrength;								// set tomechanical plunger force
						m_posdesired = m_frameEnd;								// set to button controller 
						m_fAcc = fTrue;											// enable animation
						m_mechTimeOut = 15;										// stay any other actions until completed
						if (g_pplayer)
						{
							g_pplayer->m_ptable->FireKeyEvent(DISPID_GameEvents_KeyDown, g_pplayer->m_rgKeys[ePlungerKey]);
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

void PlungerAnimObject::ResetFrameTime()
	{
	m_pos = m_posFrame;
	SetObjects(m_pos);
	}

void PlungerAnimObject::UpdateTimePermanent()
	{
	m_posFrame = m_pos;
	if (m_fAcc)
		{
		if (m_pos == m_posdesired)
			{
#ifdef LOG
		fprintf(g_pplayer->m_flog, "//Plunger Back In Place %d\n", g_pplayer->m_timestamp);
#endif
			m_fAcc = fFalse;
			m_speed = 0;
			}
		else
			{
			m_speed += m_force/m_mass;
			}
		}
	//SetObjects(m_pos);
	}

PINFLOAT HitPlunger::HitTest(Ball *pball, PINFLOAT dtime, Vertex3D *phitnormal)
	{
	if (fOldPhys)
		{
		int i;

		float newtime;
		float hittime = (float)dtime;
		BOOL fHit = fFalse;
		Ball BallT;
		Vertex3D normalT;

		BallT = *pball;

		float newpos;

		/*if (m_pos < m_posdesired)
			{
			newpos = m_pos + dtime*m_speed;
			if (newpos > m_posdesired)
				{
				newpos = m_posdesired;
				}
			}
		else if (m_pos > m_posdesired)
			{
			newpos = m_pos - dtime*m_speed;
			if (newpos < m_posdesired)
				{
				newpos = m_posdesired;
				}
			}*/
		if (m_plungeranim.m_pos != m_plungeranim.m_posdesired)
			{
			newpos = m_plungeranim.m_pos + (float)dtime*m_plungeranim.m_speed;

			if ((m_plungeranim.m_speed < 0) && (m_plungeranim.m_pos < m_plungeranim.m_posdesired))
				{
				newpos = m_plungeranim.m_posdesired;
				}
			else if ((m_plungeranim.m_speed > 0) && (m_plungeranim.m_pos > m_plungeranim.m_posdesired))
				{
				newpos = m_plungeranim.m_posdesired;
				}
			}
		else
			{
			newpos = m_plungeranim.m_pos;
			}

		newtime = (float)m_plungeranim.m_linesegBase.HitTest(&BallT, dtime, &normalT);
		if (newtime != -1 && newtime < hittime)
			{
			fHit = fTrue;
			hittime = newtime;
			*phitnormal = normalT;

			phitnormal[1].x = 0;
			phitnormal[1].y = 0;
			}

		for (i=0;i<2;i++)
			{
			newtime = (float)m_plungeranim.m_linesegSide[i].HitTest(&BallT, dtime, &normalT);
				if (newtime != -1 && newtime < hittime)
				{
				fHit = fTrue;
				hittime = newtime;
				*phitnormal = normalT;

				phitnormal[1].x = 0;
				phitnormal[1].y = 0;
				}

			newtime = (float)m_plungeranim.m_jointBase[i].HitTest(&BallT, dtime, &normalT);
			if (newtime != -1 && newtime < hittime)
				{
				fHit = fTrue;
				hittime = newtime;
				*phitnormal = normalT;

				phitnormal[1].x = 0;
				phitnormal[1].y = 0;
				}
			}

		//float deltay = newpos - m_pos;

		float deltay = m_plungeranim.m_speed;
		
		BallT.vy -= deltay;

		newtime = (float)m_plungeranim.m_linesegEnd.HitTest(&BallT, dtime, &normalT);
		if (newtime != -1 && newtime < hittime)
			{
			fHit = fTrue;
			hittime = newtime;

			*phitnormal = normalT;

			phitnormal[1].x = 0;
			phitnormal[1].y = deltay;	 //m_speed;		//>>> changed by chris
			}

		for (i=0;i<2;i++)
			{
			newtime = (float)m_plungeranim.m_jointEnd[i].HitTest(&BallT, dtime, &normalT);
			if (newtime != -1 && newtime < hittime)
				{
				fHit = fTrue;
				hittime = newtime;

				*phitnormal = normalT;

				phitnormal[1].x = 0;
				phitnormal[1].y = deltay;	 //m_speed;		//>>> changed by chris
				}
			}

		return fHit ? hittime : -1;
		}
	else // new phys
		{
		int i;

		float newtime;
		float hittime = (float)dtime; //start time
		BOOL fHit = fFalse;
		Ball BallT;
		Vertex3D hitnormal[5];

		BallT = *pball;
		BallT.m_vpVolObjs = NULL; // HACK HACK HACK - this should be dealt with some other way so that this ball doesn't delete this member variable when it is destructed.

		// If we got here, then the ball is close enough to the plunger
		// to where we should animate the button's light.
		// Save the time so we can tell the button when to turn on/off.  
		//LastPlungerHit = msec();

		// We are close enthe plunger light.
		newtime = (float)m_plungeranim.m_linesegBase.HitTest(&BallT, dtime, &hitnormal[0]);
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

		for (i=0;i<2;i++)
		{
			newtime = (float)m_plungeranim.m_linesegSide[i].HitTest(&BallT, hittime, &hitnormal[0]);
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

			newtime = (float)m_plungeranim.m_jointBase[i].HitTest(&BallT, hittime, &hitnormal[0]);
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

		float deltay = m_plungeranim.m_speed;

		BallT.vy -= deltay;

		newtime = (float)m_plungeranim.m_linesegEnd.HitTest(&BallT, hittime, &hitnormal[0]);
		if (newtime >= 0 && newtime <= hittime)
		{
			fHit = fTrue;																	// auto plunger button
			hittime = newtime;

			phitnormal[0] = hitnormal[0];
			pball->m_HitDist = BallT.m_HitDist;
			pball->m_HitNormVel = BallT.m_HitNormVel;
			pball->m_HitRigid = true;
			phitnormal[1].x = 0;
			phitnormal[1].y = deltay;	 //m_speed;		//>>> changed by chris

			// Terribly sorry for this hack.  
			// I'm trying to get the mechanical plunger to behave like a button when it is an autoplunger (ie on AFM, MM).
			// If I set the strength and forces, it still does a weak shot every now and then.
			// It happens in the HitTest, and I don't have a clue why... and I'm running out of time.
			// So here is my hack... button pulls and autofires of the plunger give fairly consistent values.
			// So I check here for the table name and if it's an autoplunger... and hard set the value.  Again, sorry.  :(

			/*if ( this->m_pplunger->m_d.m_autoPlunger == 1 )
			{
				if ( strcmp ( g_pplayer->m_ptable->m_szTableName, "Medieval Madness" ) == 0 )
				{
					pball->m_HitDist = 1.5100f;
					pball->m_HitNormVel = -40.6810f;
					phitnormal[1].y = -40.7295f;
				}

				if ( strcmp ( g_pplayer->m_ptable->m_szTableName, "Attack From Mars" ) == 0 )
				{
					pball->m_HitDist = 5.8555f;
					pball->m_HitNormVel = -37.3279f;
					phitnormal[1].y = -37.1875f;
				}
			}*/

			//		HDC hdcNull = GetDC(NULL);
			//		char szFoo[128];
			//		int len;
			//
			//		len = sprintf(szFoo, "Plunger: m_HitDist = %f", pball->m_HitDist);
			//		TextOut(hdcNull, 10, 50, szFoo, len);
			//		len = sprintf(szFoo, "Plunger: m_HitNormVel = %f", pball->m_HitNormVel);
			//		TextOut(hdcNull, 10, 75, szFoo, len);
			//		len = sprintf(szFoo, "Plunger: phitnormal[1].y = %f", phitnormal[1].y);
			//		TextOut(hdcNull, 10, 100, szFoo, len);
			//
			//		ReleaseDC(NULL, hdcNull);
		}

		for (i=0;i<2;i++)
		{
			newtime = (float)m_plungeranim.m_jointEnd[i].HitTest(&BallT, hittime, &hitnormal[0]);
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

		return fHit ? hittime : -1;
		}
	}

void HitPlunger::Draw(HDC hdc)
	{
	m_plungeranim.m_linesegBase.Draw(hdc);
	m_plungeranim.m_linesegEnd.Draw(hdc);
	
	int i;

	for (i=0;i<2;i++)
		{
		m_plungeranim.m_linesegSide[i].Draw(hdc);
		m_plungeranim.m_jointBase[i].Draw(hdc);
		m_plungeranim.m_jointEnd[i].Draw(hdc);
		}
	}

void HitPlunger::Collide(Ball *pball, Vertex3D *phitnormal)
	{
	if (fOldPhys)
		{
		float e = 0.3f;

		Vertex vel;

		vel.x = (float)pball->vx;
		vel.y = (float)pball->vy;
		vel.x -= (float)phitnormal[1].x;
		vel.y -= (float)phitnormal[1].y;

		float dot = vel.x * phitnormal->x + vel.y * phitnormal->y;

		float ImpulseNumerator = -(1+e) * dot;
		float ImpulseDenominator = (1/1 + 0/*1/m_mass*/);

		float impulse = ImpulseNumerator / ImpulseDenominator;

		pball->vx += impulse/1 * phitnormal->x;
		pball->vy += impulse/1 * phitnormal->y;

		// Friction

		Vertex ThruVec;
		Vertex CrossVec;

		CrossVec.x = dot * phitnormal->x;
		CrossVec.y = dot * phitnormal->y;

		ThruVec.x = (float)(pball->vx - CrossVec.x);
		ThruVec.y = (float)(pball->vy - CrossVec.y);

		//ThruVec.x *= 0.99f; // Rolling friction is low
		//ThruVec.y *= 0.99f;

		ThruVec.x *= 0.999f; // Rolling friction is low
		ThruVec.y *= 0.999f;

		pball->vx = ThruVec.x + CrossVec.x;
		pball->vy = ThruVec.y + CrossVec.y;
		}
	else
		{
		float dot = (float)((pball->vx - phitnormal[1].x)* phitnormal->x + (pball->vy - phitnormal[1].y) * phitnormal->y);

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
		// correct displacements, mostly from low velocity blindness, an alternative to true acceleration processing	
		float hdist = (float)(-C_DISP_GAIN * pball->m_HitDist);				// distance found in hit detection
		if (hdist > 1.0e-4)
		{													// magnitude of jump
			if (hdist > C_DISP_LIMIT) 
			{hdist = C_DISP_LIMIT;}		// crossing ramps, delta noise
			pball->x += hdist * phitnormal->x;					// push along norm, back to free area
			pball->y += hdist * phitnormal->y;					// use the norm, but is not correct
		}
#endif


		float impulse = -(dot * ((float)1.45)/(1+1/m_plungeranim.m_mass));

		pball->vx += impulse *phitnormal->x;  
		pball->vy += impulse *phitnormal->y; 

		pball->vx *= c_hardFriction; pball->vy *= c_hardFriction; pball->vz *= c_hardFriction; //friction all axiz

		float scatter_vel = m_plungeranim.m_scatterVelocity * 1;//g_pplayer->m_ptable->m_globalDifficulty;			// apply dificulty weighting

		if ( scatter_vel > 0  && fabs(pball->vy) > scatter_vel) //skip if low velocity 
		{
			float scatter = 2.0f* ((float)rand()/((float)RAND_MAX) - 0.5f);  // -1.0f..1.0f
			scatter *=  (1.0f - scatter*scatter)*2.59808f * scatter_vel;	// shape quadratic distribution and scale
			pball->vy += scatter;
		}

		pball->m_fDynamic = C_DYNAMIC;
		}
		
	Vertex3D vnormal;
	vnormal.Set(phitnormal->x, phitnormal->y, 0);
	pball->AngularAcceleration(&vnormal);
	}
	
// mechPlunger NOTE: Normalized position is from 0.0 to +1.0f
// +1.0 is fully retracted, 0.0 is fully compressed
// method requires calibration in control panel game controllers to work right
// calibrated zero value should match the rest position of the mechanical plunger
// the method below uses a dual - piecewise linear function to map the mechanical pull and push 
// onto the virtual plunger position from 0..1, the pulunger properties has a ParkPosition setting 
// that matches the mechanical plunger zero position
/*#define JOYRANGEMN (-1000)
#define JOYRANGEMX (+1000)
float PlungerAnimObject::mechPlunger()
{
	float range = (float)JOYRANGEMX * (1.0f - m_parkPosition) - (float)JOYRANGEMN *m_parkPosition; // final range limit
	float tmp = 0;//(curMechPlungerPos < 0) ? curMechPlungerPos*m_parkPosition : curMechPlungerPos*(1.0f - m_parkPosition);
	tmp = tmp/range + m_parkPosition;		//scale and offset
	return tmp;
}*/

void PlungerAnimObject::Check3D()
	{
	LPDIRECTDRAWSURFACE7 pdds;

	pdds = g_pplayer->m_pin3d.m_pddsBackBuffer;

	//Vertex3D v;

	//v.x = m_hitcircleBase.center.x;
	//v.y = m_hitcircleBase.center.y;
	//v.z = 25;

	//int frame = (int)((m_angleCur-m_frameStart)/(m_frameEnd-m_frameStart) * m_vddsFrame.Size());
	int frame;
	
	if (fOldPhys)
		{
		frame = (int)((m_pos-m_frameStart)/(m_frameEnd-m_frameStart) * (m_vddsFrame.Size()-1));
		}
	else
		{
		frame = (int)((m_pos - m_frameStart + (float)1.0)/(m_frameEnd-m_frameStart) * (m_vddsFrame.Size()-1)+(float)0.5);
		}
		
	/*if (frame > m_vddsFrame.Size()-1)
		{
		frame = m_vddsFrame.Size()-1;
		}*/

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

	//ObjFrame *pobjframe = m_vddsFrame.ElementAt(frame);

	//pdds->Blt(&pobjframe->rc, pobjframe->pdds, NULL, 0, NULL);
	}

ObjFrame *PlungerAnimObject::Draw3D(RECT *prc)
	{
	LPDIRECTDRAWSURFACE7 pdds;
	
	if (!fOldPhys && m_iframe == -1) return NULL;

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