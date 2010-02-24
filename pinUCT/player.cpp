#include "stdafx.h"
#include <time.h>

#include "..\roms.h"


//#define _DEBUGPHYSICSx
//#define DEBUG_FRATE


#define RECOMPUTEBUTTONCHECK WM_USER+100
#define RESIZE_FROM_EXPAND WM_USER+101

#define PHYSICS_OLDTIMESTEP		10000				// The old physics time step (10 ms).
#define PHYSICS_NEWTIMESTEP		10000				// The physics time step (5 ms).


LRESULT CALLBACK PlayerWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK PlayerDMDHackWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int CALLBACK PauseProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

int CALLBACK DebuggerProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

U32 sim_msec;
U32 prev_sim_msec;

U32 c_hitcnts = 0;
U32 c_collisioncnt = 0;
U32 c_contactcnt = 0;
U32 c_staticcnt = 0;
U32 c_embedcnts = 0;
static int movedPlunger = 0;						// has plunger moved, must have moved at least three times
U32 LastPlungerHit = 0;								// The last time the plunger was in contact (at least the vicinity) of the ball.
int Coins = 0;										// The number of coins queued to be inserted.  These were sent from the shell after the load.

extern U32 activity_stamp;
extern int pressed_start;
extern U32 startbutton_stamp;
extern U32 appstarttime;


Player::Player()
{
	c_embedcnts = 0;
	m_fLShiftDown = fFalse;
	m_fRShiftDown = fFalse;
	m_fPause = fFalse;
	m_fStep = fFalse;
	m_fPseudoPause = fFalse;
	m_pauseRefCount = 0;
	m_fNoTimeCorrect = fFalse;

	m_frotate = fFalse;

	m_sleeptime = 0;

	m_pcsimpleplayer = NULL;
	m_pxap = NULL;
	m_pactiveball = NULL;

	int checkblit;
	HRESULT hr = GetRegInt("Player", "CheckBlit", &checkblit);
	if (hr != S_OK)
	{
		checkblit = g_pvp->FDefaultCheckBlit(); // The default
	}
	m_fCheckBlt = (checkblit == 1);

	int shadow;
	hr = GetRegInt("Player", "BallShadows", &shadow);
	if (hr != S_OK)
	{
		shadow = fTrue; // The default
	}
	m_fBallShadows = (shadow == 1);

	int decal;
	hr = GetRegInt("Player", "BallDecals", &decal);
	if (hr != S_OK)
	{
		decal = fTrue; // The default
	}
	m_fBallDecals = (decal == 1);

	int antialias;
	hr = GetRegInt("Player", "BallAntialias", &antialias);
	if (hr != S_OK)
	{
		antialias = fTrue; // The default
	}
	m_fBallAntialias = (antialias == 1);

	int detecthang;
	hr = GetRegInt("Player", "DetectHang", &detecthang);
	if (hr != S_OK)
	{
		detecthang = fFalse; // The default
	}
	m_fDetectScriptHang = (detecthang == 1);

	int wastetime;
	hr = GetRegInt("Player", "WasteTime", &wastetime);
	if (hr != S_OK)
	{
		wastetime = fTrue; // The default
	}

	m_fWasteTime2 = fTrue;//(wastetime == 1);

	m_fWasteTime = fFalse;

	FILE *foo;
	foo = fopen("c:\\debug1.txt","r");
	if (foo)
	{
		fclose(foo);
		m_fCheckBlt = fFalse;
	}
	foo = fopen("c:\\debug2.txt","r");
	if (foo)
	{
		fclose(foo);
		m_fCheckBlt = fTrue;
	}

	foo = fopen("c:\\debug3.txt","r");
	if (foo)
	{
		fclose(foo);
		m_fWasteTime = fTrue;
	}

	foo = fopen("c:\\debug4.txt","r");
	if (foo)
	{
		fclose(foo);
		m_fWasteTime2 = fTrue;
	}

#if defined( _DEBUGPHYSICSx ) || defined( DEBUG_FRATE )
	m_fShowFPS = fTrue;
#else
	m_fShowFPS = fFalse;
#endif

	m_fCloseDown = fFalse;

	m_DebugBalls = fFalse;
	m_ToggleDebugBalls = fFalse;

	m_fCleanBlt = fFalse;

	m_fDebugMode = fFalse;
	m_hwndDebugger = NULL;
	m_PauseTimeTarget = 0;
	m_pactiveballDebug = NULL;

	m_fGameWindowActive = fFalse;
	m_fDebugWindowActive = fFalse;
	m_fUserDebugPaused = fFalse;
	m_hwndDebugOutput = NULL;

	m_LastKnownGoodCounter = 0;
	m_ModalRefCount = 0;

	m_fDrawCursor = fFalse;
	m_lastcursorx = 0xfffffff;
	m_lastcursory = 0xfffffff;
}

Player::~Player()
{
	int i;

	for (i=0;i<m_ptable->m_vedit.Size();i++)
	{
		Hitable *ph = m_ptable->m_vedit.ElementAt(i)->GetIHitable();
		if (ph)
		{
			ph->EndPlay();
		}
	}

	for (i=0;i<m_vho.Size();i++)
	{
		delete m_vho.ElementAt(i);
	}
	m_vho.RemoveAllElements();

	for (i=0;i<m_vdebugho.Size();i++)
	{
		delete m_vdebugho.ElementAt(i);
	}
	m_vdebugho.RemoveAllElements();

	// balls get deleted by the hit object vector
	// not anymore - balls are added to the octree, but not the main list
	for (i=0;i<m_vball.Size();i++)
	{
		Ball *pball = m_vball.ElementAt(i);
		if (pball->m_pballex)
		{
			pball->m_pballex->m_pball = NULL;
			pball->m_pballex->Release();
		}

		delete pball->m_vpVolObjs;
		delete pball;
	}
	m_vball.RemoveAllElements();

	if (m_pcsimpleplayer)
	{
		m_pcsimpleplayer->Stop();
		m_pcsimpleplayer->Release();
	}

	CloseHandle(m_hSongCompletionEvent);

	if (m_pxap)
	{
		delete m_pxap;
		m_pxap = NULL;
	}

	for (i=0;i<m_controlclsidsafe.Size();i++)
	{
		delete m_controlclsidsafe.ElementAt(i);
	}
	m_controlclsidsafe.RemoveAllElements();
}

void Player::ToggleFPS( void )
{
	m_fShowFPS = !m_fShowFPS;
	m_lastfpstime = m_timeCur;
	m_cframes = 0;
	m_fps = 0;
	m_total = 0;
	m_count = 0;
	m_max = 0;

	m_phys_total = 0;
	m_phys_max = 0;
	m_phys_max_iterations = 0;
	m_phys_total_iterations = 0;
}

void Player::EnableFPS( void )
{
	ToggleFPS();
	m_fShowFPS = 1;
}

void Player::DisableFPS( void )
{
	ToggleFPS();
	m_fShowFPS = 0;
}

void Player::RecomputePauseState()
{
	BOOL fOldPause = m_fPause;
	BOOL fNewPause = !(m_fGameWindowActive || m_fDebugWindowActive);// || m_fUserDebugPaused;

	if (fOldPause && fNewPause)
	{
		g_pplayer->m_LastKnownGoodCounter++; // So our catcher doesn't catch on the last value
		g_pplayer->m_fNoTimeCorrect = fTrue;
		g_pplayer->m_fCleanBlt = fFalse;
	}

	m_fPause = fNewPause;
}

void Player::RecomputePseudoPauseState()
{
	BOOL fOldPseudoPause = m_fPseudoPause;
	m_fPseudoPause = m_fUserDebugPaused || m_fDebugWindowActive;
	if (fOldPseudoPause != m_fPseudoPause)
	{
		if (m_fPseudoPause)
		{
			PauseMusic();
		}
		else
		{
			UnpauseMusic();
		}
	}
}

void Player::CreateBoundingHitShapes(Vector<HitObject> *pvho)
{
	LineSeg *plineseg;

	plineseg = new LineSeg();
	plineseg->m_pfe = NULL;
	plineseg->m_rcHitRect.zlow = m_ptable->m_tableheight;
	plineseg->m_rcHitRect.zhigh = m_ptable->m_glassheight;
	plineseg->v1.x = m_ptable->m_right;
	plineseg->v1.y = m_ptable->m_top;
	plineseg->v2.x = m_ptable->m_right;
	plineseg->v2.y = m_ptable->m_bottom;
	plineseg->CalcNormal();
	pvho->AddElement(plineseg);

	plineseg = new LineSeg();
	plineseg->m_pfe = NULL;
	plineseg->m_rcHitRect.zlow = m_ptable->m_tableheight;
	plineseg->m_rcHitRect.zhigh = m_ptable->m_glassheight;
	plineseg->v1.x = m_ptable->m_left;
	plineseg->v1.y = m_ptable->m_bottom;
	plineseg->v2.x = m_ptable->m_left;
	plineseg->v2.y = m_ptable->m_top;
	plineseg->CalcNormal();
	pvho->AddElement(plineseg);

	plineseg = new LineSeg();
	plineseg->m_pfe = NULL;
	plineseg->m_rcHitRect.zlow = m_ptable->m_tableheight;
	plineseg->m_rcHitRect.zhigh = m_ptable->m_glassheight;
	plineseg->v1.x = m_ptable->m_right;
	plineseg->v1.y = m_ptable->m_bottom;
	plineseg->v2.x = m_ptable->m_left;
	plineseg->v2.y = m_ptable->m_bottom;
	plineseg->CalcNormal();
	pvho->AddElement(plineseg);

	plineseg = new LineSeg();
	plineseg->m_pfe = NULL;
	plineseg->m_rcHitRect.zlow = m_ptable->m_tableheight;
	plineseg->m_rcHitRect.zhigh = m_ptable->m_glassheight;
	plineseg->v1.x = m_ptable->m_left;
	plineseg->v1.y = m_ptable->m_top;
	plineseg->v2.x = m_ptable->m_right;
	plineseg->v2.y = m_ptable->m_top;
	plineseg->CalcNormal();
	pvho->AddElement(plineseg);

	Vertex3D rgv3D[4];

	Hit3DPoly *ph3dpoly;

	rgv3D[0].x = m_ptable->m_left;
	rgv3D[0].y = m_ptable->m_top;
	rgv3D[0].z = m_ptable->m_glassheight;

	rgv3D[1].x = m_ptable->m_right;
	rgv3D[1].y = m_ptable->m_top;
	rgv3D[1].z = m_ptable->m_glassheight;

	rgv3D[2].x = m_ptable->m_right;
	rgv3D[2].y = m_ptable->m_bottom;
	rgv3D[2].z = m_ptable->m_glassheight;

	rgv3D[3].x = m_ptable->m_left;
	rgv3D[3].y = m_ptable->m_bottom;
	rgv3D[3].z = m_ptable->m_glassheight;

	ph3dpoly = new Hit3DPoly(rgv3D,4);

	pvho->AddElement(ph3dpoly);
}

void Player::InitKeys()
{
	HRESULT hr;
	int key;
	hr = GetRegInt("Player","LFlipKey", &key);
	if (hr != S_OK || key > 0xdd)
	{
		key = DIK_LSHIFT;
	}
	m_rgKeys[eLeftFlipperKey] = (EnumAssignKeys)key;

	hr = GetRegInt("Player","RFlipKey", &key);
	if (hr != S_OK || key > 0xdd)
	{
		key = DIK_RSHIFT;
	}
	m_rgKeys[eRightFlipperKey] = (EnumAssignKeys)key;

	hr = GetRegInt("Player","LTiltKey", &key);
	if (hr != S_OK || key > 0xdd)
	{
		key = DIK_Z;
	}
	m_rgKeys[eLeftTiltKey] = (EnumAssignKeys)key;

	hr = GetRegInt("Player","RTiltKey", &key);
	if (hr != S_OK || key > 0xdd)
	{
		key = DIK_SLASH;
	}
	m_rgKeys[eRightTiltKey] = (EnumAssignKeys)key;

	hr = GetRegInt("Player","CTiltKey", &key);
	if (hr != S_OK || key > 0xdd)
	{
		key = DIK_SPACE;
	}
	m_rgKeys[eCenterTiltKey] = (EnumAssignKeys)key;

	hr = GetRegInt("Player","PlungerKey", &key);
	if (hr != S_OK || key > 0xdd)
	{
		key = DIK_RETURN;
	}
	m_rgKeys[ePlungerKey] = (EnumAssignKeys)key;

	hr = GetRegInt("Player","AddCreditKey", &key);
	if (hr != S_OK || key > 0xdd)
	{
		key = DIK_5;
	}
	m_rgKeys[eAddCreditKey] = (EnumAssignKeys)key;

	hr = GetRegInt("Player","AddCreditKey2", &key);
	if (hr != S_OK || key > 0xdd)
	{
		key = DIK_3;
	}
	m_rgKeys[eAddCreditKey2] = (EnumAssignKeys)key;

	hr = GetRegInt("Player","StartGameKey", &key);
	if (hr != S_OK || key > 0xdd)
	{
		key = DIK_1;
	}
	m_rgKeys[eStartGameKey] = (EnumAssignKeys)key;

	hr = GetRegInt("Player","ExitGameKey", &key);
	if (hr != S_OK || key > 0xdd)
	{
		key = DIK_Q;
	}
	m_rgKeys[eExitGame] = (EnumAssignKeys)key;

	//rlc begin added keys

	hr = GetRegInt("Player","MechTilt", &key);
	if (hr != S_OK || key > 0xdd)
	{
		key = DIK_T;			//assigned by Aaron
	}
	m_rgKeys[eMechanicalTilt] = (EnumAssignKeys)key;

	hr = GetRegInt("Player","RMagnaSave", &key);
	if (hr != S_OK || key > 0xdd)
	{
		//key = DIK_BACKSPACE;	//14 (0x0E) taken from Black Knight  DIK_RCONTROL        0x9D
		key = DIK_RCONTROL;		//157 (0x9D) DIK_RCONTROL        0x9D
	}
	m_rgKeys[eRightMagnaSave] = (EnumAssignKeys)key;

	hr = GetRegInt("Player","LMagnaSave", &key);
	if (hr != S_OK || key > 0xdd)
	{
		//key = DIK_APOSTROPHE; //40 (0x28) taken from Black Knight
		key = DIK_LCONTROL; //29 (0x1D)
	}
	m_rgKeys[eLeftMagnaSave] = (EnumAssignKeys)key;

	m_rgKeys[eVolumeUp] = (EnumAssignKeys) DIK_ADD;
	m_rgKeys[eVolumeDown] = (EnumAssignKeys) DIK_MINUS;

	//rlc end add keys
}

void Player::InitRegValues()
{
	int fmusic = 0;
	HRESULT hr;

	hr = GetRegInt("Player", "PlayMusic", &fmusic);
	if (hr != S_OK)
	{
		fmusic = 1; // default value
	}
	m_fPlayMusic = fmusic ? fTrue:fFalse;

	hr = GetRegInt("Player", "PlaySound", &fmusic);
	if (hr != S_OK)
	{
		fmusic = 1; // default value
	}
	m_fPlaySound = fmusic ? fTrue:fFalse;

	hr = GetRegInt("Player", "MusicVolume", &m_MusicVolume);
	if (hr != S_OK)
	{
		m_MusicVolume = 100; // default value
	}
	m_MusicVolume = (int)(((log((float)m_MusicVolume)/log(10.0f))*1000) - 2000); // 10 volume = -10Db

	hr = GetRegInt("Player", "SoundVolume", &m_SoundVolume);
	if (hr != S_OK)
	{
		m_SoundVolume = 100; // default value
	}
}

void Player::InitDebugHitStructure()
{
	int i;
	for (i=0;i<m_ptable->m_vedit.Size();i++)
	{
		Hitable *ph = m_ptable->m_vedit.ElementAt(i)->GetIHitable();
		if (ph)
		{
			int currentsize = m_vdebugho.Size();
			ph->GetHitShapesDebug(&m_vdebugho);
			int newsize = m_vdebugho.Size();
			int hitloop;
			// Save the objects the trouble of having the set the idispatch pointer themselves
			for (hitloop = currentsize;hitloop < newsize;hitloop++)
			{
				m_vdebugho.ElementAt(hitloop)->m_pfedebug = m_ptable->m_vedit.ElementAt(i)->GetIFireEvents();
			}
		}
	}

	for (i=0;i<m_vdebugho.Size();i++)
	{
		m_vdebugho.ElementAt(i)->CalcHitRect();
		m_debugoctree.m_vho.AddElement(m_vdebugho.ElementAt(i));
	}

	m_debugoctree.m_rectbounds.left = m_ptable->m_left;
	m_debugoctree.m_rectbounds.right = m_ptable->m_right;
	m_debugoctree.m_rectbounds.top = m_ptable->m_top;
	m_debugoctree.m_rectbounds.bottom = m_ptable->m_bottom;
	m_debugoctree.m_rectbounds.zlow = m_ptable->m_tableheight;
	m_debugoctree.m_rectbounds.zhigh = m_ptable->m_glassheight;

	m_debugoctree.m_vcenter.x = (m_hitoctree.m_rectbounds.left + m_hitoctree.m_rectbounds.right)/2;
	m_debugoctree.m_vcenter.y = (m_hitoctree.m_rectbounds.top + m_hitoctree.m_rectbounds.bottom)/2;
	m_debugoctree.m_vcenter.z = (m_hitoctree.m_rectbounds.zlow + m_hitoctree.m_rectbounds.zhigh)/2;

	m_debugoctree.CreateNextLevel();
}

HRESULT Player::Init(PinTable *ptable, HWND hwndProgress, HWND hwndProgressName, BOOL fCheckForCache)
{
	HRESULT hr;
	int		i;

	m_ptable = ptable;

	m_jolt_amount = (U32)m_ptable->m_jolt_amount;
	m_tilt_amount = (U32)m_ptable->m_tilt_amount;	
	m_jolt_trigger_time = (U32)m_ptable->m_jolt_trigger_time;
	m_tilt_trigger_time = (U32)m_ptable->m_tilt_trigger_time;

	m_hSongCompletionEvent = CreateEvent( NULL, TRUE, FALSE, NULL );

	SendMessage(hwndProgress, PBM_SETPOS, 40, 0);
	// TEXT
	SetWindowText(hwndProgressName, "Initalizing Visuals...");

	InitWindow();
	InitDMDHackWindow();

	InitKeys();

	InitRegValues();

	// width, height, and colordepth are only defined if fullscreen is true.
	hr = m_pin3d.InitDD(m_hwnd, m_fFullScreen, m_screenwidth, m_screenheight, m_screendepth, m_refreshrate);

	if (m_fFullScreen)
	{
		SetWindowPos(m_hwnd, NULL, 0, 0, m_screenwidth, m_screenheight, SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE);
	}

	if (m_fFullScreen)
	{
		m_pixelaspectratio = ((GPINFLOAT)m_screenwidth / (GPINFLOAT)m_screenheight) / (4.0/3.0);
	}
	else
	{
		m_pixelaspectratio = ((GPINFLOAT)m_width / (GPINFLOAT)m_height) / (4.0/3.0);
	}

	if (hr != S_OK)
	{
		char szfoo[64];
		sprintf(szfoo, "Error code: %x",hr);
		ShowError(szfoo);
		return hr;
	}

	m_pininput.Init(m_hwnd);

	// Initialize light hack states.
	for ( i=0; i<LIGHTHACK_MAX; i++ )
	{
		m_LastUpdateTime[i] = 0;
		m_LightHackReadyForDrawLightHackFn[i] = FALSE;

		m_LightHackCurrentState[i] = FALSE;
		m_LightHackPreviousState[i] = FALSE;

		m_LightHackCurrentAnimState[i] = FALSE;
		m_LightHackPreviousAnimState[i] = FALSE;

		m_LightHackImage[i] = NULL;
		strcpy ( &(m_LightHackImageName[i][0]), "" );
	}

	// Firepower.
	m_LightHackX[LIGHTHACK_FIREPOWER_P1] = 2372;
	m_LightHackY[LIGHTHACK_FIREPOWER_P1] = 512;
	m_LightHackWidth[LIGHTHACK_FIREPOWER_P1] = 86;
	m_LightHackHeight[LIGHTHACK_FIREPOWER_P1] = 512;

	m_LightHackX[LIGHTHACK_FIREPOWER_P2] = 2372;
	m_LightHackY[LIGHTHACK_FIREPOWER_P2] = 0;
	m_LightHackWidth[LIGHTHACK_FIREPOWER_P2] = 86;
	m_LightHackHeight[LIGHTHACK_FIREPOWER_P2] = 512;

	m_LightHackX[LIGHTHACK_FIREPOWER_P3] = 2458;
	m_LightHackY[LIGHTHACK_FIREPOWER_P3] = 512;
	m_LightHackWidth[LIGHTHACK_FIREPOWER_P3] = 86;
	m_LightHackHeight[LIGHTHACK_FIREPOWER_P3] = 512;

	m_LightHackX[LIGHTHACK_FIREPOWER_P4] = 2458;
	m_LightHackY[LIGHTHACK_FIREPOWER_P4] = 0;
	m_LightHackWidth[LIGHTHACK_FIREPOWER_P4] = 86;
	m_LightHackHeight[LIGHTHACK_FIREPOWER_P4] = 512;

	// Strikes & Spares.
	m_LightHackX[LIGHTHACK_STRIKESANDSPARES_P1] = 2372;
	m_LightHackY[LIGHTHACK_STRIKESANDSPARES_P1] = 512;
	m_LightHackWidth[LIGHTHACK_STRIKESANDSPARES_P1] = 86;
	m_LightHackHeight[LIGHTHACK_STRIKESANDSPARES_P1] = 512;

	m_LightHackX[LIGHTHACK_STRIKESANDSPARES_P2] = 2372;
	m_LightHackY[LIGHTHACK_STRIKESANDSPARES_P2] = 0;
	m_LightHackWidth[LIGHTHACK_STRIKESANDSPARES_P2] = 86;
	m_LightHackHeight[LIGHTHACK_STRIKESANDSPARES_P2] = 512;

	m_LightHackX[LIGHTHACK_STRIKESANDSPARES_P3] = 2458;
	m_LightHackY[LIGHTHACK_STRIKESANDSPARES_P3] = 512;
	m_LightHackWidth[LIGHTHACK_STRIKESANDSPARES_P3] = 86;
	m_LightHackHeight[LIGHTHACK_STRIKESANDSPARES_P3] = 512;

	m_LightHackX[LIGHTHACK_STRIKESANDSPARES_P4] = 2458;
	m_LightHackY[LIGHTHACK_STRIKESANDSPARES_P4] = 0;
	m_LightHackWidth[LIGHTHACK_STRIKESANDSPARES_P4] = 86;
	m_LightHackHeight[LIGHTHACK_STRIKESANDSPARES_P4] = 512;

	// Create the real-time renderer.
	Display_Create ( );

	hr = m_pin3d.m_pd3dDevice->BeginScene();

	float realFOV = ptable->m_FOV;

	if (realFOV <= 0)
	{
		realFOV = 0.01f; // Can't have a real zero FOV, but this will look the same
	}

	m_pin3d.InitLayout ( ptable->m_left, ptable->m_top, ptable->m_right, ptable->m_bottom, ptable->m_inclination, 
		realFOV, 0.0f, ptable->m_scalex, ptable->m_scaley, ptable->m_xlatex, ptable->m_xlatey );

	m_mainlevel.m = 0;
	m_mainlevel.n = 0;
	m_mainlevel.b = 0;

	float slope = ptable->m_angletiltMin 
		+ (ptable->m_angletiltMax - ptable->m_angletiltMin) 
		* ptable->m_slopeDifficulty;

	m_mainlevel.m_gravity.x = (D3DVALUE)0;
	m_mainlevel.m_gravity.y = (D3DVALUE)(sin(ANGTORAD(slope))*ptable->m_Gravity); 
	m_mainlevel.m_gravity.z = (D3DVALUE)(-cos(ANGTORAD(slope))*ptable->m_Gravity);

	m_NudgeX = 0;
	m_NudgeY = 0;
	m_nudgetime = 0;
	movedPlunger = 0;	// has plunger moved, must have moved at least three times

	SendMessage(hwndProgress, PBM_SETPOS, 50, 0);
	// TEXT
	SetWindowText(hwndProgressName, "Initalizing Physics...");

	// Need to set timecur here, for init functions that set timers
	m_timeCur = 0;

#ifdef FPS
	m_lastfpstime = m_timeCur;
	m_cframes = 0;
	m_fps = 0;
	m_max = 0;
	m_count = 0;
	m_total = 0;
	m_phys_total = 0;
	m_phys_max = 0;
	m_phys_max_iterations = 0;
	m_phys_total_iterations = 0;
#endif

	// Initialize all objects that have collision.
	for ( i=0; i<m_ptable->m_vedit.Size(); i++ )
	{
		Hitable *ph = m_ptable->m_vedit.ElementAt(i)->GetIHitable();
		if (ph)
		{
			int currentsize = m_vho.Size();
			ph->GetHitShapes ( &m_vho );
			int newsize = m_vho.Size();
			int hitloop;
			
			// Save the objects the trouble of having the set the idispatch pointer themselves
			for (hitloop = currentsize; hitloop < newsize; hitloop++ )
			{
				m_vho.ElementAt(hitloop)->m_pfedebug = m_ptable->m_vedit.ElementAt(i)->GetIFireEvents();
			}
			
			ph->GetTimers ( &m_vht );
		}
	}

	CreateBoundingHitShapes ( &m_vho );

	for ( i=0; i<m_vho.Size(); i++ )
	{
		m_vho.ElementAt(i)->CalcHitRect();

		m_hitoctree.m_vho.AddElement(m_vho.ElementAt(i));

		if (m_vho.ElementAt(i)->GetType() == e3DPoly && ((Hit3DPoly *)m_vho.ElementAt(i))->m_fVisible)
		{
			m_shadowoctree.m_vho.AddElement ( m_vho.ElementAt(i) );
		}

		if ( (m_vho.ElementAt(i)->GetAnimObject() != NULL) && m_vho.ElementAt(i)->GetAnimObject()->FMover() )
		{
			m_vmover.AddElement ( m_vho.ElementAt(i)->GetAnimObject() );
		}
	}

	m_hitoctree.m_rectbounds.left = m_ptable->m_left;
	m_hitoctree.m_rectbounds.right = m_ptable->m_right;
	m_hitoctree.m_rectbounds.top = m_ptable->m_top;
	m_hitoctree.m_rectbounds.bottom = m_ptable->m_bottom;
	m_hitoctree.m_rectbounds.zlow = m_ptable->m_tableheight;
	m_hitoctree.m_rectbounds.zhigh = m_ptable->m_glassheight;

	m_hitoctree.m_vcenter.x = (m_hitoctree.m_rectbounds.left + m_hitoctree.m_rectbounds.right)/2;
	m_hitoctree.m_vcenter.y = (m_hitoctree.m_rectbounds.top + m_hitoctree.m_rectbounds.bottom)/2;
	m_hitoctree.m_vcenter.z = (m_hitoctree.m_rectbounds.zlow + m_hitoctree.m_rectbounds.zhigh)/2;

	m_hitoctree.CreateNextLevel();

	m_shadowoctree.m_rectbounds.left = m_ptable->m_left;
	m_shadowoctree.m_rectbounds.right = m_ptable->m_right;
	m_shadowoctree.m_rectbounds.top = m_ptable->m_top;
	m_shadowoctree.m_rectbounds.bottom = m_ptable->m_bottom;
	m_shadowoctree.m_rectbounds.zlow = m_ptable->m_tableheight;
	m_shadowoctree.m_rectbounds.zhigh = m_ptable->m_glassheight;

	m_shadowoctree.m_vcenter.x = (m_hitoctree.m_rectbounds.left + m_hitoctree.m_rectbounds.right)/2;
	m_shadowoctree.m_vcenter.y = (m_hitoctree.m_rectbounds.top + m_hitoctree.m_rectbounds.bottom)/2;
	m_shadowoctree.m_vcenter.z = (m_hitoctree.m_rectbounds.zlow + m_hitoctree.m_rectbounds.zhigh)/2;

	m_shadowoctree.CreateNextLevel();

	hr = m_pin3d.m_pd3dDevice->EndScene();

	// Start Cache
	m_pin3d.m_fWritingToCache = fFalse;
	m_pin3d.m_fReadingFromCache = fFalse;
	m_pin3d.m_hFileCache = NULL;

	if (fCheckForCache)
	{
		BOOL fGotCache = m_pin3d.OpenCacheFileForRead();

		if (!fGotCache)
		{
			m_pin3d.OpenCacheFileForWrite();
		}
	}

	SendMessage(hwndProgress, PBM_SETPOS, 60, 0);
	if (m_pin3d.m_fReadingFromCache)
	{
		SetWindowText(hwndProgressName, "Reading Table From Cache...");
	}
	else 
	{
		if (m_pin3d.m_fWritingToCache)
		{
			SetWindowText(hwndProgressName, "Writing Table To Cache...");
		}
		else
		{
			SetWindowText(hwndProgressName, "Rendering Table...");
		}
	}

	// Pre-render all non-changing elements such as 
	// static walls, rails, backdrops, etc.
	InitStatic(hwndProgress);

	SendMessage(hwndProgress, PBM_SETPOS, 80, 0);
	if (m_pin3d.m_fReadingFromCache)
	{
		SetWindowText(hwndProgressName, "Reading Animations From Cache...");
	}
	else 
	{
		if (m_pin3d.m_fWritingToCache)
		{
			SetWindowText(hwndProgressName, "Writing Animations To Cache...");
		}
		else
		{
			SetWindowText(hwndProgressName, "Rendering Animations...");
		}
	}

	// Pre-render all elements which have animations.
	// Add the pre-rendered animations to the display list. 
	InitAnimations(hwndProgress);

	// End Cache
	m_pin3d.CloseCacheFile();

	///////////////// Screen Update Vector
	///// (List of movers which can be blitted at any time)
	/////////////////////////

	for ( i=0; i<m_vho.Size(); i++ )
	{
		if (m_vho.ElementAt(i)->GetAnimObject() != NULL)
		{
			int l;

			// Put the screenupdate vector in sorted order back to
			// front so that invalidated objects draw over each-other
			// correctly

			AnimObject *pao = m_vho.ElementAt(i)->GetAnimObject();

			float myz = (pao->m_znear + pao->m_zfar) / 2;

			for (l=0;l<m_vscreenupdate.Size();l++)
			{
				BOOL fInBack = fFalse;

				{
					float comparez = (m_vscreenupdate.ElementAt(l)->m_znear + m_vscreenupdate.ElementAt(l)->m_zfar)/2;

					if (myz > comparez)
					{
						fInBack = fTrue;
					}
				}

				if (fInBack)
				{
					break;
				}
			}

			if (l == m_vscreenupdate.Size())
			{
				m_vscreenupdate.AddElement(pao);
			}
			else
			{
				m_vscreenupdate.InsertElementAt(pao, l);
			}

			pao->m_fInvalid = fFalse;
		}
	}

	// Render inital textbox text
	for (i=0;i<m_ptable->m_vedit.Size();i++)
	{
		if (m_ptable->m_vedit.ElementAt(i)->GetItemType() == eItemTextbox)
		{
			Textbox *ptb = (Textbox *)m_ptable->m_vedit.ElementAt(i);
			ptb->RenderText();
		}
	}

	// Render inital dispreel(s)
	for (i=0;i<m_ptable->m_vedit.Size();i++)
	{
		if (m_ptable->m_vedit.ElementAt(i)->GetItemType() == eItemDispReel)
		{
			DispReel *pdr = (DispReel *)m_ptable->m_vedit.ElementAt(i);
			pdr->RenderText();
		}
	}

	m_ptable->m_pcv->Start(); // Hook up to events and start cranking script

	SetWindowText(hwndProgressName, "Starting Game Scripts...");

	m_ptable->FireVoidEvent(DISPID_GameEvents_Init);

	wintimer_init();

	m_liStartTime = usec();

	m_PhysicsStepTime = PHYSICS_NEWTIMESTEP;	
	m_nextPhysicsFrameTime = m_liStartTime + m_PhysicsStepTime;
	m_liPhysicsCalced = m_liStartTime; // haven't calced any physics yet

	m_curPhysicsFrameTime = m_liStartTime;

	m_physicsdtime = 1;

	SendMessage(hwndProgress, PBM_SETPOS, 100, 0);
	// TEXT
	SetWindowText(hwndProgressName, "Starting...");

	// Check if we should show the window.
	// Show if we don't have a front end, or autostart is not enabled.
	HWND hFrontEndWnd = FindWindow( NULL, "UltraPin" );
	if ( (hFrontEndWnd == NULL) ||		
		 (m_ptable->m_tblAutoStartEnabled == false) )
	{
		// Show the window.
		ShowWindow(m_hwnd, SW_SHOW);
		SetForegroundWindow(m_hwnd);                      
		SetFocus(m_hwnd);
	}

	// Call Init
	for ( i=0; i<m_ptable->m_vedit.Size(); i++ )
	{
		Hitable *ph = m_ptable->m_vedit.ElementAt(i)->GetIHitable();
		if (ph)
		{
			if (ph->GetEventProxyBase())
			{
				ph->GetEventProxyBase()->FireVoidEvent(DISPID_GameEvents_Init);
			}
		}
	}

	if (m_fDetectScriptHang)
	{
		g_pvp->PostWorkToWorkerThread(HANG_SNOOP_START, NULL);
	}

	return S_OK;
}

void Player::InitStatic(HWND hwndProgress)
{
	HRESULT hr;
	int i;

	if (m_pin3d.m_fReadingFromCache)
	{
		m_pin3d.ReadSurfaceFromCacheFile(m_pin3d.m_pddsStatic);
		m_pin3d.ReadSurfaceFromCacheFile(m_pin3d.m_pddsStaticZ);
		return;
	}

	// Start the frame.
	hr = m_pin3d.m_pd3dDevice->BeginScene();

	// Direct all renders to the "static" buffer.
	m_pin3d.SetRenderTarget(m_pin3d.m_pddsStatic);

	// Draw stuff
	for (i=0;i<m_ptable->m_vedit.Size();i++)
	{
		if (m_ptable->m_vedit.ElementAt(i)->GetItemType() != eItemDecal && m_ptable->m_vedit.ElementAt(i)->GetItemType() != eItemKicker)
		{
			Hitable *ph = m_ptable->m_vedit.ElementAt(i)->GetIHitable();
			if (ph)
			{
				ph->RenderStatic(m_pin3d.m_pd3dDevice);
				if (hwndProgress)
				{
					SendMessage(hwndProgress, PBM_SETPOS, 60 + ((15*i)/m_ptable->m_vedit.Size()), 0);
				}
			}
		}
	}

	// Draw decals (they have transparency, so they have to be drawn after the wall they are on)
	for (i=0;i<m_ptable->m_vedit.Size();i++)
	{
		if (m_ptable->m_vedit.ElementAt(i)->GetItemType() == eItemDecal)
		{
			Hitable *ph = m_ptable->m_vedit.ElementAt(i)->GetIHitable();
			if (ph)
			{
				ph->RenderStatic(m_pin3d.m_pd3dDevice);
				if (hwndProgress)
				{
					SendMessage(hwndProgress, PBM_SETPOS, 75 + ((5*i)/m_ptable->m_vedit.Size()), 0);
				}
			}
		}
	}

	// Draw kickers (they change z-buffer, so they have to be drawn after the wall they are on)
	for (i=0;i<m_ptable->m_vedit.Size();i++)
	{
		if (m_ptable->m_vedit.ElementAt(i)->GetItemType() == eItemKicker)
		{
			Hitable *ph = m_ptable->m_vedit.ElementAt(i)->GetIHitable();
			if (ph)
			{
				ph->RenderStatic(m_pin3d.m_pd3dDevice);
			}
		}
	}

	// Finish the frame.
	hr = m_pin3d.m_pd3dDevice->EndScene();

	m_pin3d.WriteSurfaceToCacheFile(m_pin3d.m_pddsStatic);
	m_pin3d.WriteSurfaceToCacheFile(m_pin3d.m_pddsStaticZ);
}

void Player::InitAnimations(HWND hwndProgress)
{
	HRESULT hr;
	int i;

	hr = m_pin3d.m_pd3dDevice->BeginScene();

	// Direct all renders to the back buffer.
	m_pin3d.SetRenderTarget(m_pin3d.m_pddsBackBuffer);

	// Set up z-buffer to the static one, so movers can clip to it
	m_pin3d.m_pddsZBuffer->Blt(NULL, m_pin3d.m_pddsStaticZ, NULL, 0, NULL);
	m_pin3d.m_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET, 0x00000000, 1.0f, 0L );

	// Draw stuff
	for (i=0;i<m_ptable->m_vedit.Size();i++)
	{
		Hitable *ph = m_ptable->m_vedit.ElementAt(i)->GetIHitable();
		if (ph)
		{
			if (m_pin3d.m_fReadingFromCache)
			{
				ph->RenderMoversFromCache(&m_pin3d);
			}
			else
			{
				ph->RenderMovers(m_pin3d.m_pd3dDevice);
			}
			if (hwndProgress)
			{
				SendMessage(hwndProgress, PBM_SETPOS, 80 + ((20*i)/m_ptable->m_vedit.Size()), 0);
			}
		}
	}

	// Init lights to initial state
	for (i=0;i<m_ptable->m_vedit.Size();i++)
	{
		if (m_ptable->m_vedit.ElementAt(i)->GetItemType() == eItemLight)
		{
			Light *plight = ((Light *)m_ptable->m_vedit.ElementAt(i));
			if (plight->m_d.m_state == LightStateBlinking)
			{
				plight->DrawFrame(plight->m_rgblinkpattern[0] == '1');
			}
			else
			{
				plight->DrawFrame(plight->m_d.m_state != LightStateOff);
			}
		}
		else if (m_ptable->m_vedit.ElementAt(i)->GetItemType() == eItemBumper)
		{
			Bumper *pbumper = ((Bumper *)m_ptable->m_vedit.ElementAt(i));
			if (pbumper->m_d.m_state == LightStateBlinking)
			{
				pbumper->DrawFrame(pbumper->m_rgblinkpattern[0] == '1');
			}
			else
			{
				pbumper->DrawFrame(pbumper->m_d.m_state != LightStateOff);
			}
		}
	}

	m_pin3d.EnableLightMap(fFalse, -1);

	hr = m_pin3d.m_pd3dDevice->EndScene();

	// Copy the "static" buffer to the back buffer.
	m_pin3d.m_pddsBackBuffer->Blt(NULL, m_pin3d.m_pddsStatic, NULL, 0 , NULL);
	m_pin3d.m_pddsZBuffer->Blt(NULL, m_pin3d.m_pddsStaticZ, NULL, 0 , NULL);
}

Ball *Player::CreateBall(float x, float y, float z, float vx, float vy, float vz)
{
	Ball *pball = new Ball();
	pball->radius = 25;
	pball->x = x;
	pball->y = y;
	pball->z = z+pball->radius;
	//pball->z = z;
	pball->vx = vx;
	pball->vy = vy;
	pball->vz = vz;
	pball->Init(); // Call this after radius set to get proper inertial tensor set up

	pball->EnsureOMObject();

	pball->m_pfedebug = (IFireEvents *)pball->m_pballex;

	m_vball.AddElement(pball);
	m_vmover.AddElement(&pball->m_ballanim);

	// Add to list of global exception hit-tests for now
	m_hitoctree.m_vho.AddElement(pball);

	pball->CalcBoundingRect();

	if (!m_pactiveballDebug)
	{
		m_pactiveballDebug = pball;
	}

	return pball;
}

void Player::EraseBall(Ball *pball)
{

	int blur;

	// Erase each ball blur.
	for ( blur=0; blur<BALL_NUMBLURS; blur++ )
	{
		// Flag the region as needing to be updated.
		if (m_fBallShadows)
		{
			if (fIntRectIntersect(pball->m_rcScreen[blur], pball->m_rcScreenShadow[blur]))
			{
				// Rect already merged
				InvalidateRect(&pball->m_rcScreen[blur]);
			}
			else
			{
				InvalidateRect(&pball->m_rcScreen[blur]);
				InvalidateRect(&pball->m_rcScreenShadow[blur]);
			}
		}
		else
		{
			InvalidateRect(&pball->m_rcScreen[blur]);
		}
	}

}


void Player::DestroyBall(Ball *pball)
{
	if (!pball) return;

	if (pball->m_fErase) // Need to clear the ball off the playfield
	{
		EraseBall(pball);
	}

	if (pball->m_pballex)
	{
		pball->m_pballex->m_pball = NULL;
		pball->m_pballex->Release();
	}

	m_vball.RemoveElement(pball);
	m_vmover.RemoveElement(&pball->m_ballanim);

	m_hitoctree.m_vho.RemoveElement(pball);

	m_vballDelete.AddElement(pball);

	if (m_pactiveballDebug == pball)
	{
		if (m_vball.Size() > 0)
		{
			m_pactiveballDebug = m_vball.ElementAt(0);
		}
		else
		{
			m_pactiveballDebug = NULL;
		}
	}
}

void Player::InitDMDHackWindow()
{

	WNDCLASSEX	wcex;
	int			x, y;
	int			width, height;

	// Define the window.
	memset ( &wcex, 0, sizeof ( WNDCLASSEX ) );
	wcex.cbSize = sizeof ( WNDCLASSEX );
	wcex.style = 0;
	wcex.lpfnWndProc = (WNDPROC) PlayerDMDHackWndProc;
	wcex.hInstance = g_hinst;
	wcex.lpszClassName = "VPPlayerDMDHack";
	wcex.hIcon = LoadIcon ( g_hinst, MAKEINTRESOURCE(IDI_TABLEICON) );
	wcex.hCursor = LoadCursor ( NULL, IDC_ARROW );
	wcex.lpszMenuName = NULL;
	RegisterClassEx ( &wcex );

	// Place window in top-left corner.
	x = 0;
	y = 0;

	// Set the width and height.
	width = 200;
	height = 100;

	// TEXT
	m_dmdhackhwnd = ::CreateWindowEx( (WS_EX_TOPMOST), "VPPlayerDMDHack", "Visual Pinball Player DMD Hack", (WS_POPUP | WS_CLIPCHILDREN), x, y, width, height, NULL, NULL, g_hinst, 0);

}


void Player::InitWindow()
{

	WNDCLASSEX wcex;

	memset(&wcex, 0, sizeof(WNDCLASSEX));

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = 0;
	wcex.lpfnWndProc = (WNDPROC) PlayerWndProc;
	wcex.hInstance = g_hinst;
	wcex.lpszClassName = "VPPlayer";
	wcex.hIcon = LoadIcon(g_hinst, MAKEINTRESOURCE(IDI_TABLEICON));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.lpszMenuName = NULL;
	RegisterClassEx(&wcex);

	HRESULT hr;

	hr = GetRegInt("Player", "Width", &m_width);
	if (hr != S_OK)
	{
		m_width = DEFAULT_PLAYER_WIDTH; // The default
	}

	hr = GetRegInt("Player", "Height", &m_height);
	if (hr != S_OK)
	{
		m_height = m_width *3/4;
	}

	hr = GetRegInt("Player", "FullScreen", &m_fFullScreen);
	if (hr != S_OK)
	{
		m_fFullScreen = fFalse;
	}

	// Set dimensions of playfield to vertical orientation.
	if ( m_width != DEFAULT_PLAYER_WIDTH )
	{
		int	temp;

		temp = m_width;
		m_width = m_height;
		m_height = temp;
	}

	int screenwidth;
	int screenheight;

	if (m_fFullScreen)
	{
		m_screenwidth = m_width;
		m_screenheight = m_height;
		screenwidth = m_width;
		screenheight = m_height;
		hr = GetRegInt("Player", "ColorDepth", &m_screendepth);
		if (hr != S_OK)
		{
			m_screendepth = 32; // The default
		}
		hr = GetRegInt("Player", "RefreshRate", &m_refreshrate);
		if (hr != S_OK)
		{
			m_refreshrate = 0; // The default
		}
	}
	else
	{
		screenwidth = GetSystemMetrics(SM_CXSCREEN);
		screenheight = GetSystemMetrics(SM_CYSCREEN);
	}

	int x,y;

	// constrain window to screen
	if (!m_fFullScreen & (m_width > screenwidth))
	{
		m_width = screenwidth;
		m_height = m_width * 3 / 4;
	}

	if (!m_fFullScreen & (m_height > screenheight))
	{
		m_height = screenheight;
		m_width = m_height * 4 / 3;
	}

	x = (screenwidth - m_width) / 2;
	y = (screenheight - m_height) / 2;

	int windowflags = WS_POPUP | WS_CLIPCHILDREN;
	int windowflagsex = 0;

	int captionheight = GetSystemMetrics(SM_CYCAPTION);

	if (!m_fFullScreen & (screenheight - m_height >= (captionheight*2)))	// We have enough room for a frame
	{
		// Check if we have a front end.
		if ( FindWindow( NULL, "UltraPin" ) != NULL )								
		{
			// No window border, title, or control boxes.
			windowflags = WS_POPUP; 
			windowflagsex = WS_EX_TOPMOST;

			// Place window in top-left corner.
			x = 0;
			y = 0;
		}
		else
		{
			// Add a pretty window border and standard control boxes.
			windowflags = WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_CLIPCHILDREN;
			windowflagsex = WS_EX_OVERLAPPEDWINDOW;
			y -= captionheight;
			m_height += captionheight;
		}
	}

	// TEXT
	m_hwnd = ::CreateWindowEx(windowflagsex, "VPPlayer", "Visual Pinball Player", windowflags, x, y, m_width, m_height, NULL, NULL, g_hinst, 0);

	mixer_init( m_hwnd );

	SetCursorPos( 400,999999); //rlc ... move to hide in lower let corner, one pixel shows
}


extern int e_JoyCnt;
static int curAccel_x[PININ_JOYMXCNT]= {0,0,0,0};
static int curAccel_y[PININ_JOYMXCNT]= {0,0,0,0};

static float curMechPlungerPos;

int curPlunger = JOYRANGEMN-1;	// assume


F32 GetX( void )
{

	return ((F32)curAccel_x[0]) * (2.0f / ((F32)(JOYRANGEMX-JOYRANGEMN)));

}


F32 GetY( void )
{

	return ((F32)curAccel_y[0]) * (2.0f / ((F32)(JOYRANGEMX-JOYRANGEMN)));

}


void Player::UltraNudge(void)	// called on every intergral physics frame
{	

	m_NudgeX = 0;
	m_NudgeY = 0;

}


void Player::UltraPlunger(void)	// called on every intergral physics frame
{	
#define IIR_Order 4

	static int init = IIR_Order;	// first time call
	static float x [IIR_Order+1] = {0,0,0,0,0};
	static float y [IIR_Order+1] = {0,0,0,0,0};	

	// coefficients for IIR_Order Butterworth filter set to 10 Hz passband
	static const float a [IIR_Order+1] = {	0.0048243445f,
		0.019297378f,	
		0.028946068f,
		0.019297378f,
		0.0048243445f};

	static const float b [IIR_Order+1] = {	1.00000000f, //if not 1 add division below
		-2.369513f,
		2.3139884f,
		-1.0546654f,
		0.1873795f};

	//http://www.dsptutor.freeuk.com/IIRFilterDesign/IIRFilterDesign.html  
	// (this applet is set to 8000Hz sample rate, therefore, multiply ...
	// our values by 80 to shift sample clock of 100hz to 8000hz)

	if (movedPlunger < 3) 
	{
		int init = IIR_Order;
		curMechPlungerPos = 0;
		return;	// not until a real value is entered
	}

	if (!c_plungerFilter)
	{ 
		curMechPlungerPos = (float)curPlunger;
		return;
	} 

	// Should never get here!
	// Plunger filtering sucks!

	x[0] = (float)curPlunger; //initialize filter
	do	{
		y[0] = a[0]*x[0];		  // initial

		for (int i = IIR_Order; i > 0 ;--i) // all terms but the zero-th 
		{ 
			y[0] += (a[i]*x[i] - b[i]*y[i]);// /b[0]; always one     // sum terms from high to low
			x[i] = x[i-1];		//shift 
			y[i] = y[i-1];		//shift
		}			
	} while (init--); //loop until all registers are initialized with the first input

	init = 0;

	curMechPlungerPos = y[0];
}


// mechPlunger NOTE: Normalized position is from 0.0 to +1.0f
// +1.0 is fully retracted, 0.0 is fully compressed
// method requires calibration in control panel game controllers to work right
// calibrated zero value should match the rest position of the mechanical plunger
// the method below uses a dual - piecewise linear function to map the mechanical pull and push 
// onto the virtual plunger position from 0..1, the pulunger properties has a ParkPosition setting 
// that matches the mechanical plunger zero position
float PlungerAnimObject::mechPlunger()
{
	float range = (float)JOYRANGEMX * (1.0f - m_parkPosition) - (float)JOYRANGEMN *m_parkPosition; // final range limit
	float tmp = (curMechPlungerPos < 0) ? curMechPlungerPos*m_parkPosition : curMechPlungerPos*(1.0f - m_parkPosition);
	tmp = tmp/range + m_parkPosition;		//scale and offset
	return tmp;
}


void Player::mechPlungerIn(int z)
{
	curPlunger = -z; //axis reversal

	if (++movedPlunger == 0x7ffffff) movedPlunger = 3; //restart at 3
}


//++++++++++++++++++++++++++++++++++++++++

#define EMBEDED_LIMIT  0.0875f
#define STATICCNTS 10

void Player::PhysicsSimulateCycle( PINFLOAT dtime,U64 startTime) // move physics forward to this time
{
	int i = 0;
	PINFLOAT hittime,htz;
	PINFLOAT staticTime = STATICTIME;
	int limitTime = m_ptable->m_PhysicsLoopTime;
	int halfLimitTime = limitTime/2;

	Ball *pball;
	int vballsize = m_vball.Size();

	int StaticCnts = STATICCNTS;			// maximum number of static counts

	while (dtime > 0) 	
	{

		if (limitTime)//time in microseconds
		{
			int time_elasped = (int)(usec()- startTime);

			if (time_elasped > limitTime)//time in microseconds
			{return;} // hung in the physics loop

			if (time_elasped > halfLimitTime)//time in microseconds
			{
				staticTime += staticTime/2;			//increase minimum time step by 50%
				halfLimitTime += halfLimitTime/2;	// set next half limit time step (logrithmic)			
			}
		}

		// first find hits, if any +++++++++++++++++++++ 

		hittime = dtime;	//begin time search from now ...  until delta ends

		for (i = 0; i < vballsize; i++)
		{
			pball = m_vball.ElementAt(i);

			if (!pball->fFrozen && pball->m_fDynamic > 0)// && !pball->fTempFrozen )	// don't play with frozen balls
			{
				pball->m_hittime = hittime;				// search upto current hittime
				pball->m_pho = NULL;

				m_hitoctree.HitTestBall(pball);			 // find the hit objects and hit times

				htz = pball->m_hittime;					// this ball's hit time

				if( htz < 0) pball->m_pho = NULL;		// no negative time allowed

				if (pball->m_pho)						// hit object
				{
#ifdef _DEBUG
					++c_hitcnts;						// stats for display

					if (pball->m_HitRigid && pball->m_HitDist < -EMBEDED_LIMIT) //rigid and embedded
						++c_embedcnts;
#endif
					///////////////////////////////////////////////////////////////////////////
					if (htz <= hittime)						//smaller hit time??
					{
						hittime = htz;						// record actual event time

						if (htz < staticTime)				// less than static time interval
						{ 
							if(!pball->m_HitRigid) hittime = staticTime; // non-rigid ... set Static time
							else if (--StaticCnts < 0)		
							{
								StaticCnts = 0;			// keep from wrapping
								hittime = staticTime;		
							}
						}
					}
				}
			}				
		}

		// hittime now set ... or full frame if no hit 
		// now update displacements to collide-contact or end of physics frame
		// !!!!! 2) move objects to hittime

		if (hittime > staticTime) StaticCnts = STATICCNTS;	// allow more zeros next round

		for (i=0;i<m_vmover.Size();i++)
		{
			m_vmover.ElementAt(i)->UpdateDisplacements(hittime); //step 2:  move the objects about according to velocities
		} 

		//  ok find balls that need to be collided and script'ed (generally there will be one, but more are possible)

		for (i=0; i < m_vball.Size(); i++)			// use m_vball.Size(), in case script deletes a ball
		{
			pball = m_vball.ElementAt(i);			// local pointer

			if (pball->m_fDynamic > 0 && pball->m_pho && pball->m_hittime <= hittime) // find balls with hit objects and minimum time			
			{			

				// now collision, contact and script reactions on active ball (object)+++++++++
				HitObject *pho =  pball->m_pho;		// object that ball hit in trials
				pball->m_pho = NULL;				// remove trial hit object pointer
				m_pactiveball = pball;				// For script that wants the ball doing the collision

				c_collisioncnt++;

				pho->Collide(pball, pball->m_hitnormal);	//!!!!! 3) collision on active ball

				// Collide may have changed the velocity of the ball, 
				// and therefore the bounding box for the next hit cycle
				if ( m_vball.ElementAt(i) != pball) // Ball still exists? may have been deleted from list
				{ if(i) --i;}  // collision script deleted the ball, back up one count, if not zero
				else
				{
					pball->CalcBoundingRect();		// do new boundings 

					// is this ball static? .. set static and quench	
					if (pball->m_HitRigid && pball->m_HitDist < PHYS_TOUCH) //rigid and close distance
					{//contacts
						c_contactcnt++;
						PINFLOAT mag = pball->vx*pball->vx + pball->vy*pball->vy; // values below are taken from simulation
						if (pball->drsq < (PINFLOAT)8.0e-5 && mag < (PINFLOAT)1.0e-3 && fabsf(pball->vz)< (PINFLOAT)0.2)
						{
							if(--pball->m_fDynamic <= 0)						//... ball static, cancels next gravity increment
							{												// m_fDynamic is cleared in ball gravity section
								pball->m_fDynamic = 0;
								c_staticcnt++;
								pball->vx =	pball->vy = pball->vz = 0;			//quench the remaing velocity and set ...
							}
						}
					}
				}
			}
		}

		dtime -= hittime;	//new delta .. i.e. time remaining

	}// physics loop
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


int fpieee_handler( _FPIEEE_RECORD *pieee )
{
	// user-defined ieee trap handler routine:
	// there is one handler for all 
	// IEEE exceptions

	// Assume the user wants all invalid 
	// operations to return 0.

	if ((pieee->Cause.InvalidOperation) && (pieee->Result.Format == _FpFormatFp32)) 
	{
		pieee->Result.Value.Fp32Value = 0.0F;

		return EXCEPTION_CONTINUE_EXECUTION;
	}
	else
		return EXCEPTION_EXECUTE_HANDLER;
}



#define _EXC_MASK  _EM_UNDERFLOW  + _EM_OVERFLOW   + _EM_ZERODIVIDE \
	+ _EM_INEXACT + _EM_DENORMAL +_EM_INVALID


void Player::Render()
{

	int iball;
	int i;
	Ball *pball;
	int cball = m_vball.Size();
	HRESULT				ReturnCode;
	HRESULT hr;

	__try {
		// unmask invalid operation exception
		_controlfp(_EXC_MASK, _MCW_EM); 

		// code that may generate 
		// fp exceptions goes here
	}
	__except ( _fpieee_flt( GetExceptionCode(),
		GetExceptionInformation(),
		fpieee_handler ) ){

			// code that gets control 

			// if fpieee_handler returns
			// EXCEPTION_EXECUTE_HANDLER goes here

		}

		// Check if there are no balls in play.
		if ( (Ball::NumInitted() - PinTable::NumStartBalls()) == 0 )
		{
			// Prevent tilts from one ball from affecting the next ball.
			plumb_reset();
		}

		// Don't calculate the next frame if the last one isn't done blitting yet
		// On Win95 when there are no balls, frame updates happen so fast the
		// blitter gets stuck
		if ( (cball == 0) && m_fWasteTime2 )
		{
			Sleep ( 1 );
		}

		if ( m_sleeptime > 0 )
		{
			Sleep ( m_sleeptime - 1 );
		}

		if ( m_fCheckBlt )
		{
			HRESULT hrdone = m_pin3d.m_pddsFrontBuffer->GetBltStatus(DDGBS_ISBLTDONE);

			if ( hrdone != DD_OK )
			{
				if ( m_fWasteTime )
				{
					Sleep ( 1 );
				}
				return;
			}
		}

		U64 m_RealTimeClock = usec();
		U32 new_msec = msec();

		if ( m_fNoTimeCorrect ) // After debugging script
		{
			// Shift whole game foward in time
			m_liStartTime += (m_RealTimeClock - m_curPhysicsFrameTime);
			m_nextPhysicsFrameTime += (m_RealTimeClock - m_curPhysicsFrameTime);
			m_curPhysicsFrameTime = m_RealTimeClock; // 0 time frame
			m_fNoTimeCorrect = fFalse;
		}

		if ( m_fDebugWindowActive || m_fUserDebugPaused )
		{
			// Shift whole game foward in time
			m_liStartTime += (m_RealTimeClock - m_curPhysicsFrameTime);
			m_nextPhysicsFrameTime += (m_RealTimeClock - m_curPhysicsFrameTime);
			if (m_fStep)
			{
				// Walk one physics step foward
				m_curPhysicsFrameTime = m_RealTimeClock - m_PhysicsStepTime;
				m_fStep = fFalse;
			}
			else
			{
				m_curPhysicsFrameTime = m_RealTimeClock; // 0 time frame
			}
		}

		float timepassed = (float)(m_RealTimeClock - m_curPhysicsFrameTime) / (float)1000000.0f;

		// Get time in milliseconds for timers
		m_timeCur = (int)((m_RealTimeClock - m_liStartTime)/1000);

#ifdef FPS
		if (m_fShowFPS)
		{
			m_cframes++;
			if ((m_timeCur - m_lastfpstime) > 1000)
			{
				m_fps = m_cframes * 1000 / (m_timeCur - m_lastfpstime);
				m_lastfpstime = m_timeCur;
				m_cframes = 0;
			}
		}
#endif

		float frametime;

		frametime = timepassed * 100;

		for (iball=0;iball<cball;iball++)
		{
			pball = m_vball.ElementAt(iball);

			if (pball->m_fErase) // Need to clear the ball off the playfield
			{
				EraseBall(pball);
			}
		}

		// Erase the mixer volume.
		mixer_erase();

		c_collisioncnt = 0; 
		c_hitcnts = 0;
		c_contactcnt = 0;
		c_staticcnt = 0;

		U32 phys_iterations = 0;
		U64 phys_period = usec(); // NOTE: variable is mis-named until the end of this while loop	

		///+++++++++++++++++++++++++++++++++++++++++++++++++++++
		while (m_curPhysicsFrameTime < m_RealTimeClock)		//loop here until next frame time
		{
			phys_iterations++;
			// Get the time until the next physics tick is done, and get the time
			// Unitl the next frame is done (newtime)
			// If the frame is the next thing to happen, update physics to that
			// point next update acceleration, and continue loop

			PINFLOAT physics_dtime	= (float)(m_nextPhysicsFrameTime - m_curPhysicsFrameTime)/10000.0f;
			PINFLOAT physics_to_graphic_dtime  = (float)(m_RealTimeClock - m_curPhysicsFrameTime)/10000.0f;

			if (physics_to_graphic_dtime < physics_dtime )				// is graphic frame time next???
			{		
				PhysicsSimulateCycle(physics_to_graphic_dtime, usec());	// advance physics to this time	
				m_curPhysicsFrameTime = m_RealTimeClock;				// now current to the wall clock
				break;	//this is the common exit from the loop			// exit skipping accelerate
			}		// some rare cases will exit from while()

			if ((int)(usec()- m_RealTimeClock) > 200000)				// hung in the physics loop over 200 milliseconds
			{															// can not keep up to real time
				m_curPhysicsFrameTime = m_RealTimeClock;				// skip physics forward ... slip-cycles
				m_nextPhysicsFrameTime = m_RealTimeClock + m_PhysicsStepTime;
				break;	//this is the common exit from the loop			// go draw frame
			}

			// Process input.
			prev_sim_msec = sim_msec;
			sim_msec = new_msec - (U32) ( ( m_RealTimeClock - m_curPhysicsFrameTime ) / 1000 );
			m_pininput.ProcessKeys(m_ptable, sim_msec );

			//primary physics loop
			PhysicsSimulateCycle(physics_dtime, usec());				// main simulator call physics_dtime

			m_curPhysicsFrameTime = m_nextPhysicsFrameTime;				// new cycle, on physics frame boundary
			m_nextPhysicsFrameTime += m_PhysicsStepTime;				// advance physics position

			// now get and/or calculate integral cycle physics events, digital filters, external acceleration inputs, etc.

			mixer_update();
			plumb_update();

			m_pactiveball = NULL;  // No ball is the active ball for timers/key events

			int p_timeCur = (int)((m_curPhysicsFrameTime - m_liStartTime)/1000); //rlc 10 milli-seconds

			for (i=0;i<m_vht.Size();i++)
			{
				HitTimer *pht = m_vht.ElementAt(i);
				if (pht->m_nextfire <= p_timeCur)
				{
					pht->m_pfe->FireGroupEvent(DISPID_TimerEvents_Timer);
					pht->m_nextfire += pht->m_interval;
				}
			}

			slintf( "%u %u\n", new_msec, sim_msec );
			slintf( "%f %f %d %d\n", physics_dtime, physics_to_graphic_dtime, sim_msec, msec() );	

			//rlc physics_dtime is the balance of time to move from the graphic frame position to the next
			// integral physics frame.  So the previous graphics frame was (1.0 - physics_dtime) before 
			// this integral physics frame. Accelerations and inputs are always physics frame aligned
			UltraNudge();		
			UltraPlunger();		

			if ( m_nudgetime )
			{
				m_nudgetime--;	                                                                                                                                                    

				if (m_nudgetime == 5)
				{
					m_NudgeX = m_NudgeBackX * 2;
					m_NudgeY = m_NudgeBackY * 2;
				}
				else if (m_nudgetime == 0)
				{
					m_NudgeX = -m_NudgeBackX;
					m_NudgeY = -m_NudgeBackY;
				}
			}

			for (i=0;i<m_vmover.Size();i++)
			{
				// always on integral physics frame boundary
				m_vmover.ElementAt(i)->UpdateVelocities( ((float) PHYSICS_NEWTIMESTEP) / ((float) PHYSICS_OLDTIMESTEP) );	
			}			

		}	// end while (m_curPhysicsFrameTime < m_RealTimeClock)
		//end marker <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

		phys_period = usec() - phys_period;

		m_LastKnownGoodCounter++;

		// Check all elements that could possibly need updating.
		for (i=0;i<m_vscreenupdate.Size();i++)
		{
			// Check if the element is invalid (its frame changed).
			m_vscreenupdate.ElementAt(i)->m_fInvalid = fFalse;
			m_vscreenupdate.ElementAt(i)->Check3D();
			if (m_vscreenupdate.ElementAt(i)->m_fInvalid)					
			{
				// Flag the element's region as needing a redraw.
				InvalidateRect(&m_vscreenupdate.ElementAt(i)->m_rcBounds);
			}
		}	

		// Initialize all invalid regions by resetting the region (basically clear) 
		// it with the contents of the static buffer.
		for (i=0;i<m_vupdaterect.Size();i++)
		{
			UpdateRect *pur = m_vupdaterect.ElementAt(i);
			if (pur->m_fSeeThrough)													
			{
				RECT *prc = &pur->m_rcupdate;

				// Redraw the region from the static buffers to the back and z buffers.
				m_pin3d.m_pddsBackBuffer->Blt(prc, m_pin3d.m_pddsStatic, prc, 0, NULL);
				m_pin3d.m_pddsZBuffer->Blt(prc, m_pin3d.m_pddsStaticZ, prc, 0, NULL);
			}
		}

		// Start rendering the next frame.
		hr = m_pin3d.m_pd3dDevice->BeginScene();

		// Draw the lights.
		for (i=0;i<m_vblink.Size();i++)
		{
			IBlink *pblink = m_vblink.ElementAt(i);
			if (pblink->m_timenextblink <= m_timeCur)
			{
				char cold = pblink->m_rgblinkpattern[pblink->m_iblinkframe];
				pblink->m_iblinkframe++;
				char cnew = pblink->m_rgblinkpattern[pblink->m_iblinkframe];
				if (cnew == 0)
				{
					pblink->m_iblinkframe = 0;
					cnew = pblink->m_rgblinkpattern[0];
				}
				if (cold != cnew)
				{
					pblink->DrawFrame(cnew == '1');
				}
				pblink->m_timenextblink += pblink->m_blinkinterval;
			}
		}

		// Process all regions that need updating.  
		// The region will be drawn with the current frame.
		for ( i=0; i<m_vupdaterect.Size(); i++ )
		{
			UpdateRect *pur = m_vupdaterect.ElementAt(i);
			int l;

			// Process all objects associated with this region.
			for ( l=0; l<pur->m_vobject.Size(); l++ )
			{
				// Get the object's frame to draw.
				ObjFrame *pobjframe = pur->m_vobject.ElementAt(l)->Draw3D ( &pur->m_rcupdate );

				// Make sure we have a frame.
				if (pobjframe != NULL)
				{
					LPDIRECTDRAWSURFACE7 pdds = g_pplayer->m_pin3d.m_pddsBackBuffer;
					RECT rcUpdate;
					RECT *prc = &pur->m_rcupdate;

					// NOTE: prc is the rectangle of the region needing to be updated.
					// NOTE: pobjframe->rc is the rectangle of the entire object that intersects the region needing to updated.
					// I think they are trying to define a rectangle that intersects... but why subtract pobjframe->rc?   -JEP
					rcUpdate.left = max(pobjframe->rc.left, prc->left) - pobjframe->rc.left;
					rcUpdate.top = max(pobjframe->rc.top, prc->top) - pobjframe->rc.top;
					rcUpdate.right = min(pobjframe->rc.right, prc->right) - pobjframe->rc.left;
					rcUpdate.bottom = min(pobjframe->rc.bottom, prc->bottom) - pobjframe->rc.top;

					int bltleft, blttop;
					bltleft = max(pobjframe->rc.left, prc->left);
					blttop = max(pobjframe->rc.top, prc->top);

					// Make sure our rectangle dimensions aren't wacky.
					if ((rcUpdate.right > rcUpdate.left) && (rcUpdate.bottom > rcUpdate.top))
					{
						// Make sure we have a source color surface.
						if (pobjframe->pdds != NULL)
						{
							// Blit to the backbuffer with DDraw.   
							HRESULT hr = pdds->BltFast ( bltleft, blttop, pobjframe->pdds, &rcUpdate, DDBLTFAST_SRCCOLORKEY );
						}

						// Make sure we have a source z surface.
						if (pobjframe->pddsZBuffer != NULL)
						{
							// Blit to the z buffer.	
							HRESULT hr = g_pplayer->m_pin3d.m_pddsZBuffer->BltFast ( bltleft, blttop, pobjframe->pddsZBuffer, &rcUpdate, DDBLTFAST_NOCOLORKEY );
						}
					}
				}
			}
		}

		// Check if we are debugging balls
		if (g_pplayer->m_ToggleDebugBalls)
		{
			// Check if we are debugging balls
			if (g_pplayer->m_DebugBalls)
			{
				// Set the render state to something that will always display.
				ReturnCode = g_pplayer->m_pin3d.m_pd3dDevice->SetRenderState ( D3DRENDERSTATE_ZENABLE, D3DZB_FALSE );
				ReturnCode = g_pplayer->m_pin3d.m_pd3dDevice->SetRenderState ( D3DRENDERSTATE_ALPHABLENDENABLE, FALSE );
			}
			else
			{
				// Restore the render state.
				ReturnCode = g_pplayer->m_pin3d.m_pd3dDevice->SetRenderState ( D3DRENDERSTATE_ZENABLE, D3DZB_TRUE );
				ReturnCode = g_pplayer->m_pin3d.m_pd3dDevice->SetRenderState ( D3DRENDERSTATE_ALPHABLENDENABLE, TRUE );
			}

			g_pplayer->m_ToggleDebugBalls = False;
		}

		if (m_fBallAntialias)
		{
			DrawBallShadows();
			DrawBalls();
		}
		else
		{
			// When not antilaiasing, we can get a perf win by
			// drawing the ball first.  That way, the part of the
			// shadow that gets obscured doesn't need to do
			// alpha-blending
			DrawBalls();
			DrawBallShadows();
		}

		// Process all objects.
		for ( i=0; i<m_ptable->m_vedit.Size(); i++ )
		{
			// Get the object's hitable interface.
			// This interface includes objects that are animated and static (as well as non-display types).
			Hitable *pHitable = m_ptable->m_vedit.ElementAt(i)->GetIHitable();
			if ( pHitable )
			{
				// Update the object's animation.
				pHitable->UpdateRealTime();

				// Add the object to the display list.
				pHitable->RenderRealTime();
			}
		}

		// Draw the mixer volume.
		mixer_draw();

		// Finish rendering the next frame.
		hr = m_pin3d.m_pd3dDevice->EndScene();

		// Check if we are mirrored.
		if ( g_pplayer->m_ptable->m_tblMirrorEnabled == TRUE )
		{
			// Mirroring only works if we mirror the entire backbuffer.
			// Flag to draw the entire backbuffer.
			m_fCleanBlt = fFalse;
		}

		// Check if we drew any real-time 3D objects.
		if ( g_pplayer->m_ptable->m_RealTime3D == fTrue )
		{
			// Dynamic lighting alone can affect a large area.
			// This negates the benefit of drawing only regions.
			// We need to draw the entire buffer.
			m_fCleanBlt = fFalse;
		}

		if ( m_nudgetime ) 						
		{
			// Draw with an offset to shake the display.
			m_pin3d.Flip((int)m_NudgeBackX, (int)m_NudgeBackY);
			m_fCleanBlt = fFalse;
		}
		else
		{
			if (m_fCleanBlt)
			{
				// Smart Blit - only update the invalidated areas
				for (i=0;i<m_vupdaterect.Size();i++)
				{
					UpdateRect *pur = m_vupdaterect.ElementAt(i);
					RECT *prc = &pur->m_rcupdate;

					RECT rcNew;
					rcNew.left = prc->left + m_pin3d.m_rcUpdate.left;
					rcNew.right = prc->right + m_pin3d.m_rcUpdate.left;
					rcNew.top = prc->top + m_pin3d.m_rcUpdate.top;
					rcNew.bottom = prc->bottom + m_pin3d.m_rcUpdate.top;

					// Copy the region from the back buffer to the front buffer.
					m_pin3d.m_pddsFrontBuffer->Blt(&rcNew, m_pin3d.m_pddsBackBuffer, prc, 0, NULL);
				}
			}
			else
			{
				// Copy the entire back buffer to the front buffer.
				m_pin3d.Flip(0, 0);

				// Flag that we only need to update regions from now on...
				m_fCleanBlt = fTrue;
				g_pplayer->m_ptable->m_RealTime3D = fFalse;
			}
		}

		// Remove the list of update regions.
		// Note:  The ball and the mixer update rects are removed here as well...
		//        so if we need a clear, we need to do it somewhere else.
		for (i=0;i<m_vupdaterect.Size();i++)
		{
			UpdateRect *pur = m_vupdaterect.ElementAt(i);
			delete pur;
		}
		m_vupdaterect.RemoveAllElements();

		// Draw hack lights.
		DrawLightHack();

		if (m_pxap)
		{
			if (!m_pxap->Tick())
			{
				delete m_pxap;
				m_pxap = NULL;
				m_ptable->FireVoidEvent(DISPID_GameEvents_MusicDone);
			}
		}

		for (i=0;i<m_vballDelete.Size();i++)
		{
			Ball* pball = m_vballDelete.ElementAt(i);
			delete pball->m_vpVolObjs;
			delete pball;
		}

		m_vballDelete.RemoveAllElements();

#ifdef FPS
		if ( m_fShowFPS )
		{
			static U32 stamp;
			static U32 period;
			char szFoo[128];
			HDC hdcNull;

			// Get the device context.
			hdcNull = GetDC ( NULL );

			// Draw the amount of video memory used.
			int len = sprintf(szFoo, "Total Video Memory = %d", NumVideoBytes);
			TextOut(hdcNull, 10, 75, szFoo, len);

			// Draw the current time and activity timer.
			len = sprintf(szFoo, "Current Time = %d", msec() );
			TextOut(hdcNull, 10, 400, szFoo, len);
			len = sprintf(szFoo, "Last Activity = %d", activity_stamp );
			TextOut(hdcNull, 10, 425, szFoo, len);

			// Draw the number of balls created.
			len = sprintf(szFoo, "Num Balls Created= %d", Ball::NumInitted() );
			TextOut(hdcNull, 10, 450, szFoo, len);

			// Draw the framerate.
			len = sprintf(szFoo, "FPS = %d, FloatingPointControlWord = %x, PhysicsTimeStep = %d ms ", m_fps, _controlfp(_PC_53, 0), (m_PhysicsStepTime / 1000) );
			TextOut(hdcNull, 10, 10, szFoo, len);
			len = sprintf(szFoo, "LastInputProcess = %d ms ", (sim_msec - prev_sim_msec) );
			TextOut(hdcNull, 10, 35, szFoo, len);
			period = msec()-stamp;
			if( period > m_max ) 
			{
				m_max = period;
			}
			if( phys_period > m_phys_max ) 
			{
				m_phys_max = phys_period;
			}
			if( phys_iterations > m_phys_max_iterations ) 
			{
				m_phys_max_iterations = phys_iterations;
			}
			if( m_count == 0 )
			{
				m_total = period;
				m_phys_total = phys_period;
				m_phys_total_iterations = phys_iterations;
				m_count = 1;
			}
			else
			{
				m_total += period;
				m_phys_total += phys_period;
				m_phys_total_iterations += phys_iterations;
				m_count++;
			}

			// Draw start button info.
			len = sprintf(szFoo, "appstarttime = %d, pressed_start = %d, startbutton_stamp = %d", appstarttime, pressed_start, startbutton_stamp );
			TextOut(hdcNull, 10, 500, szFoo, len);

			// Physics debugging.
			len = sprintf(szFoo, "period: %3d ms (%3d avg %10d max)      ", period, (U32)( m_total / m_count ), (U32) m_max );
			stamp = msec();
			TextOut(hdcNull, 10, 120, szFoo, len);

			len = sprintf(szFoo, "physTimes %10d uS(%12d avg %12d max)    ", (U32)phys_period, (U32)(m_phys_total / m_count), m_phys_max );
			stamp = msec();
			TextOut(hdcNull, 10, 140, szFoo, len);

			len = sprintf(szFoo, "phys:%5d iterations(%5d avg %5d max))   ", (U32)phys_iterations, (U32)( m_phys_total_iterations / m_count ), (U32)m_phys_max_iterations );
			stamp = msec();
			TextOut(hdcNull, 10, 160, szFoo, len);

			len = sprintf(szFoo, "Hits:%5d Collide:%5d Ctacs:%5d Static:%5d Embed: %5d    ",
				c_hitcnts, c_collisioncnt, c_contactcnt, c_staticcnt, c_embedcnts);
			stamp = msec();
			TextOut(hdcNull, 10, 180, szFoo, len);

			// Release the device context.
			ReleaseDC(NULL, hdcNull);
		}
#endif

		if ((m_PauseTimeTarget > 0) && (m_PauseTimeTarget <= m_timeCur))
		{
			m_PauseTimeTarget = 0;
			m_fUserDebugPaused = fTrue;
			RecomputePseudoPauseState();
			SendMessage(m_hwndDebugger, RECOMPUTEBUTTONCHECK, 0, 0);
		}

		if (m_ptable->m_pcv->m_fScriptError)
		{
			// Crash back to the editor
			SendMessage(m_hwnd, WM_CLOSE, 0, 0);
		}
		else 
		{
			if (m_fCloseDown)
			{
				PauseMusic();

				int option;

				if( !VPinball::m_open_minimized )
				{
					option = DialogBox(g_hinstres, MAKEINTRESOURCE(IDD_GAMEPAUSE), m_hwnd, PauseProc);
				}
				else
				{
					option = ID_QUIT;
					SendMessage(g_pvp->m_hwnd, WM_COMMAND, ID_FILE_EXIT, NULL );
				}

				m_fCloseDown = fFalse;
				g_pplayer->m_fNoTimeCorrect = fTrue; // Skip the time we were in the dialog
				if (option == ID_QUIT)
				{
					// This line causes the crash when exiting a table.
					SendMessage(m_hwnd, WM_CLOSE, 0, 0);
				}
				else
				{
					UnpauseMusic();
				}
			}
		}

		///// Don't put anything here - the ID_QUIT check must be the last thing done
		///// in this function
}


void Player::PauseMusic()
{
	if (m_pauseRefCount == 0)
	{
		if (m_pcsimpleplayer)
		{
			m_pcsimpleplayer->Pause();
		}
		if (m_pxap)
		{
			m_pxap->Pause();
		}

		// signal the script that the game is now paused
		m_ptable->FireVoidEvent(DISPID_GameEvents_Paused);
	}
	m_pauseRefCount++;
}

void Player::UnpauseMusic()
{
	m_pauseRefCount--;
	if (m_pauseRefCount == 0)
	{
		if (m_pcsimpleplayer)
		{
			m_pcsimpleplayer->Unpause();
		}
		if (m_pxap)
		{
			m_pxap->Unpause();
		}

		// signal the script that the game is now running again
		m_ptable->FireVoidEvent(DISPID_GameEvents_UnPaused);
	}
	else if (m_pauseRefCount<0)
	{
		m_pauseRefCount = 0;
	}
}


void Player::DrawBallShadows()
{

	int i;

	D3DMATERIAL7 mtrl;
	ZeroMemory( &mtrl, sizeof(mtrl) );
	mtrl.diffuse.a = mtrl.ambient.a = 1;

	m_pin3d.m_pd3dDevice->SetMaterial(&mtrl);

	WORD rgi[4];

	for (i=0;i<4;i++)
	{
		rgi[i] = i;
	}

	m_pin3d.m_pd3dDevice->SetRenderState(D3DRENDERSTATE_TEXTUREPERSPECTIVE, FALSE );

	m_pin3d.m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ADDRESS, D3DTADDRESS_CLAMP);// WRAP

	float sn = (float)sin(m_pin3d.m_inclination);
	float cs = (float)cos(m_pin3d.m_inclination);

	for (i=0;i<m_vball.Size();i++)
	{
		Ball *pball = m_vball.ElementAt(i);

		if (m_fBallShadows)
		{
			Vertex3D *rgv3DShadow = pball->m_rgv3DShadow;

			mtrl.diffuse.r = mtrl.ambient.r = 1;
			mtrl.diffuse.g = mtrl.ambient.g = 1;
			mtrl.diffuse.b = mtrl.ambient.b = 1;
			mtrl.diffuse.a = mtrl.ambient.a = 1;

			m_pin3d.m_pd3dDevice->SetMaterial(&mtrl);

			m_pin3d.m_pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
			m_pin3d.m_pd3dDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE);

			if (g_pvp->m_pdd.m_fHardwareAccel)
			{
				m_pin3d.m_pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHAREF, (DWORD)0x0000001);
				m_pin3d.m_pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHAFUNC, D3DCMP_GREATEREQUAL);
				m_pin3d.m_pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, TRUE); 
			}
			else
			{
				m_pin3d.m_pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHAREF, (DWORD)0x0000001);
				m_pin3d.m_pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHAFUNC, D3DCMP_GREATEREQUAL);
				m_pin3d.m_pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, TRUE); 
			}

			Ball ballT;
			ballT.x = pball->x;
			ballT.y = pball->y;
			ballT.z = pball->z;
			ballT.vx = 200;
			ballT.vy = -200;
			ballT.vz = -200;
			ballT.radius = 0;

			ballT.m_hittime = 1;

			ballT.CalcBoundingRect();

			m_shadowoctree.HitTestBall(&ballT);

			float offsetx;
			float offsety;
			float shadowz;

			if (ballT.m_hittime < 1) // shadow falls on an object
			{
				shadowz = (float)(pball->z + 0.1f - ballT.m_hittime * 200);

				offsetx = (float)(ballT.m_hittime * 200 - 12.5);
				offsety = (float)(-ballT.m_hittime * 200 + 12.5);
			}
			else // shadow is on the floor
			{
				offsetx = (float)(pball->z/2);
				offsety = (float)(-pball->z/2);
				shadowz = 0.1f;//(float)(pball->z - pball->radius + 0.1f);
			}

			float shadowradius = pball->radius*1.2f;

			rgv3DShadow[0].x = (float)pball->x - shadowradius + offsetx;
			rgv3DShadow[0].y = (float)pball->y - shadowradius + offsety;
			rgv3DShadow[0].z = shadowz;
			rgv3DShadow[0].tu = 0;
			rgv3DShadow[0].tv = 0;
			rgv3DShadow[0].nx = 0;
			rgv3DShadow[0].ny = 0;
			rgv3DShadow[0].nz = -1;

			rgv3DShadow[1].x = (float)pball->x + shadowradius + offsetx;
			rgv3DShadow[1].y = (float)pball->y - shadowradius + offsety;
			rgv3DShadow[1].z = shadowz;
			rgv3DShadow[1].tu = 1;
			rgv3DShadow[1].tv = 0;
			rgv3DShadow[1].nx = 0;
			rgv3DShadow[1].ny = 0;
			rgv3DShadow[1].nz = -1;

			rgv3DShadow[2].x = (float)pball->x + shadowradius + offsetx;
			rgv3DShadow[2].y = (float)pball->y + shadowradius + offsety;
			rgv3DShadow[2].z = shadowz;
			rgv3DShadow[2].tu = 1;
			rgv3DShadow[2].tv = 1;
			rgv3DShadow[2].nx = 0;
			rgv3DShadow[2].ny = 0;
			rgv3DShadow[2].nz = -1;

			rgv3DShadow[3].x = (float)pball->x - shadowradius + offsetx;
			rgv3DShadow[3].y = (float)pball->y + shadowradius + offsety;
			rgv3DShadow[3].z = shadowz;
			rgv3DShadow[3].tu = 0;
			rgv3DShadow[3].tv = 1;
			rgv3DShadow[3].nx = 0;
			rgv3DShadow[3].ny = 0;
			rgv3DShadow[3].nz = -1;

			if (!pball->fFrozen && rgv3DShadow[0].x <= m_ptable->m_right && rgv3DShadow[2].y >= m_ptable->m_top)
			{
				if (rgv3DShadow[2].x > m_ptable->m_right)
				{
					float newtu = (rgv3DShadow[2].x - m_ptable->m_right) / (2*shadowradius);
					rgv3DShadow[2].tu = 1-newtu;
					rgv3DShadow[1].tu = 1-newtu;
					rgv3DShadow[2].x = m_ptable->m_right;
					rgv3DShadow[1].x = m_ptable->m_right;
				}

				if (rgv3DShadow[1].y < m_ptable->m_top)
				{
					float newtv = (m_ptable->m_top - rgv3DShadow[1].y) / (2*shadowradius);
					rgv3DShadow[1].tv = newtv;
					rgv3DShadow[0].tv = newtv;
					rgv3DShadow[1].tv = m_ptable->m_top;
					rgv3DShadow[0].tv = m_ptable->m_top;
				}

				m_pin3d.m_pd3dDevice->SetTexture(0, m_pin3d.m_pddsShadowTexture);

				m_pin3d.m_pd3dDevice->SetRenderState(D3DRENDERSTATE_COLORKEYENABLE, FALSE);

				m_pin3d.m_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,rgv3DShadow, 4,0);
			}
		}
	}
}


void Player::DrawBalls()
{

	int i;
	int	blur;

	D3DMATERIAL7 mtrl;
	ZeroMemory( &mtrl, sizeof(mtrl) );
	mtrl.diffuse.a = mtrl.ambient.a = 1;

	m_pin3d.m_pd3dDevice->SetMaterial(&mtrl);

	WORD rgi[4];

	for (i=0;i<4;i++)
	{
		rgi[i] = i;
	}

	m_pin3d.m_pd3dDevice->SetRenderState(D3DRENDERSTATE_TEXTUREPERSPECTIVE, FALSE );

	m_pin3d.m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ADDRESS, D3DTADDRESS_CLAMP); //WRAP

	float sn = (float)sin(m_pin3d.m_inclination);
	float cs = (float)cos(m_pin3d.m_inclination);

	for (i=0;i<m_vball.Size();i++)
	{
		Ball *pball = m_vball.ElementAt(i);

		float r = (pball->m_color & 255) / 255.0f;
		float g = (pball->m_color & 65280) / 65280.0f;
		float b = (pball->m_color & 16711680) / 16711680.0f;
		mtrl.diffuse.r = mtrl.ambient.r = r;
		mtrl.diffuse.g = mtrl.ambient.g = g;
		mtrl.diffuse.b = mtrl.ambient.b = b;
		m_pin3d.m_pd3dDevice->SetMaterial(&mtrl);

		Vertex3D *rgv3D = pball->m_rgv3D;

		PINFLOAT zheight;

		zheight = pball->z - pball->radius;

		if (!pball->fFrozen)
		{
			zheight += pball->radius;
		}

		// Check if we are debugging.
		if (m_DebugBalls)
		{
			// Draw the position of the ball.
			HDC hdcNull = GetDC(NULL);
			char szFoo[128];
			int len = sprintf(szFoo, "Ball_%02d (%.2f, %.2f, %.2f)", i, (float) pball->x, (float) pball->y, (float) zheight);
			TextOut(hdcNull, 10, (100 + (i*20)), szFoo, len);
			ReleaseDC(NULL, hdcNull);
		}

		// Draw each ball blur. 
		for ( blur=0; blur<BALL_NUMBLURS; blur++ ) 
		{
			float		x_blur;
			float		y_blur; 
			float		z_blur; 

			// Interpolate between the previous position and the current position.
			x_blur = pball->prev_x + ((pball->x - pball->prev_x) * (((float)(blur + 1)) / ((float) BALL_NUMBLURS)));
			y_blur = pball->prev_y + ((pball->y - pball->prev_y) * (((float)(blur + 1)) / ((float) BALL_NUMBLURS)));
			z_blur = pball->prev_z + ((pball->z - pball->prev_z) * (((float)(blur + 1)) / ((float) BALL_NUMBLURS)));

			// Only draw the .
			if ( (pball->IsBlurReady == true) ||		// Previous position is correct (thus blur is valid).
				(blur == (BALL_NUMBLURS - 1)) )		// Actual position of the ball.
			{
				if (pball->fFrozen)
				{
					z_blur -= pball->radius;
				}

				rgv3D[0].x = (float)(x_blur - pball->radius);
				rgv3D[0].y = (float)(y_blur - (pball->radius * cs));
				rgv3D[0].z = (float)(z_blur + (pball->radius * sn));
				rgv3D[0].tu = 0;
				rgv3D[0].tv = 0;
				rgv3D[0].nx = 0;
				rgv3D[0].ny = 0;
				rgv3D[0].nz = -1;

				rgv3D[3].x = (float)(x_blur - pball->radius);
				rgv3D[3].y = (float)(y_blur + (pball->radius * cs));
				rgv3D[3].z = (float)(z_blur - (pball->radius * sn));
				rgv3D[3].tu = 0;

				rgv3D[3].nx = 0;
				rgv3D[3].ny = 0;
				rgv3D[3].nz = -1;

				rgv3D[2].x = (float)(x_blur + pball->radius);
				rgv3D[2].y = (float)(y_blur + (pball->radius * cs));
				rgv3D[2].z = (float)(z_blur - (pball->radius * sn));

				rgv3D[2].nx = 0;
				rgv3D[2].ny = 0;
				rgv3D[2].nz = -1;

				rgv3D[1].x = (float)(x_blur + pball->radius);
				rgv3D[1].y = (float)(y_blur - (pball->radius * cs));
				rgv3D[1].z = (float)(z_blur + (pball->radius * sn));

				rgv3D[1].tv = 0;
				rgv3D[1].nx = 0;
				rgv3D[1].ny = 0;
				rgv3D[1].nz = -1;

				if (!pball->m_pin)
				{
					m_pin3d.m_pd3dDevice->SetTexture(0, m_pin3d.m_pddsBallTexture);
					rgv3D[3].tv = 1;
					rgv3D[2].tu = 1;
					rgv3D[2].tv = 1;
					rgv3D[1].tu = 1;
				}
				else
				{
					pball->m_pin->EnsureColorKey();
					m_pin3d.m_pd3dDevice->SetTexture(0, pball->m_pin->m_pdsBufferColorKey);
					rgv3D[3].tv = pball->m_pin->m_maxtv;
					rgv3D[2].tu = pball->m_pin->m_maxtu;
					rgv3D[2].tv = pball->m_pin->m_maxtv;
					rgv3D[1].tu = pball->m_pin->m_maxtu;
				}

				if (m_fBallAntialias)
				{
					m_pin3d.m_pd3dDevice->SetRenderState(D3DRENDERSTATE_COLORKEYENABLE, FALSE);
					m_pin3d.m_pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
					m_pin3d.m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MIPFILTER, D3DTFP_LINEAR);
				}
				else
				{
					m_pin3d.m_pd3dDevice->SetRenderState(D3DRENDERSTATE_COLORKEYENABLE, TRUE);
					m_pin3d.m_pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);
					m_pin3d.m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MIPFILTER, D3DTFP_NONE);
				}
				m_pin3d.m_pd3dDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE);

				if (g_pvp->m_pdd.m_fHardwareAccel)
				{
					m_pin3d.m_pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHAREF, (DWORD)0x0000001);
					m_pin3d.m_pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHAFUNC, D3DCMP_GREATEREQUAL);
					m_pin3d.m_pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, TRUE); 
				}
				else
				{
					m_pin3d.m_pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHAREF, (DWORD)0x0000001);
					m_pin3d.m_pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHAFUNC, D3DCMP_GREATEREQUAL);
					m_pin3d.m_pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, TRUE); 
				}

				m_pin3d.m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE);

				g_pplayer->m_pin3d.SetTextureFilter ( ePictureTexture, TEXTURE_MODE_TRILINEAR );

				m_pin3d.m_pd3dDevice->SetRenderState ( D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA );		// RGBA, Good but too dark with alpha at 128
				m_pin3d.m_pd3dDevice->SetRenderState ( D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA );

				m_pin3d.m_pd3dDevice->SetRenderState ( D3DRENDERSTATE_ZWRITEENABLE, FALSE );
				m_pin3d.m_pd3dDevice->SetRenderState ( D3DRENDERSTATE_ALPHATESTENABLE, FALSE );

				m_pin3d.m_pd3dDevice->SetRenderState ( D3DRENDERSTATE_ALPHABLENDENABLE, TRUE );

				m_pin3d.m_pd3dDevice->SetTextureStageState ( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
				m_pin3d.m_pd3dDevice->SetTextureStageState ( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
				m_pin3d.m_pd3dDevice->SetTextureStageState ( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1 );
				m_pin3d.m_pd3dDevice->SetTextureStageState ( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
				m_pin3d.m_pd3dDevice->SetTextureStageState ( 0, D3DTSS_ALPHAARG2, D3DTA_CURRENT );
				m_pin3d.m_pd3dDevice->SetTextureStageState ( 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );

				m_pin3d.m_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,rgv3D, 4,0);

				// Draw the ball logo

				if (m_fBallDecals && (pball->m_pinFront || pball->m_pinBack))
				{
					/*mtrl.diffuse.r = mtrl.ambient.r = 0.8f;
					mtrl.diffuse.g = mtrl.ambient.g = 0.4f;
					mtrl.diffuse.b = mtrl.ambient.b = 0.2f;*/
					mtrl.diffuse.a = mtrl.ambient.a = 0.8f;
					m_pin3d.m_pd3dDevice->SetMaterial(&mtrl);

#define DECALPOINTS 4

					Vertex3D rgv3DArrow[DECALPOINTS];
					{
						rgv3DArrow[0].tu = 0;
						rgv3DArrow[0].tv = 0;
						rgv3DArrow[0].x = -0.333333333f;
						rgv3DArrow[0].y = -0.333333333f;
						rgv3DArrow[0].z = -0.881917103f;

						rgv3DArrow[1].tu = 1;
						rgv3DArrow[1].tv = 0;
						rgv3DArrow[1].x = 0.333333333f;
						rgv3DArrow[1].y = -0.333333333f;
						rgv3DArrow[1].z = -0.881917103f;

						rgv3DArrow[2].tu = 1;
						rgv3DArrow[2].tv = 1;
						rgv3DArrow[2].x = 0.333333333f;
						rgv3DArrow[2].y = 0.333333333f;
						rgv3DArrow[2].z = -0.881917103f;

						rgv3DArrow[3].tu = 0;
						rgv3DArrow[3].tv = 1;
						rgv3DArrow[3].x = -0.333333333f;
						rgv3DArrow[3].y = 0.333333333f;
						rgv3DArrow[3].z = -0.881917103f;

						m_pin3d.m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE);

						g_pplayer->m_pin3d.SetTextureFilter ( ePictureTexture, TEXTURE_MODE_TRILINEAR );

						m_pin3d.m_pd3dDevice->SetRenderState(D3DRENDERSTATE_COLORKEYENABLE, FALSE);
						m_pin3d.m_pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);

						m_pin3d.m_pd3dDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE);
					}
				}

				pball->m_fErase = fTrue;

				// Mark ball rect as dirty for blitting to the screen
				m_pin3d.ClearExtents(&pball->m_rcScreen[blur], NULL, NULL);
				m_pin3d.ExpandExtents(&pball->m_rcScreen[blur], pball->m_rgv3D, NULL, NULL, 4, fFalse);

				if (m_fBallShadows)
				{
					m_pin3d.ClearExtents(&pball->m_rcScreenShadow[blur], NULL, NULL);
					m_pin3d.ExpandExtents(&pball->m_rcScreenShadow[blur], pball->m_rgv3DShadow, NULL, NULL, 4, fFalse);

					if (fIntRectIntersect(pball->m_rcScreen[blur], pball->m_rcScreenShadow[blur]))
					{
						pball->m_rcScreen[blur].left = min(pball->m_rcScreen[blur].left, pball->m_rcScreenShadow[blur].left);
						pball->m_rcScreen[blur].top = min(pball->m_rcScreen[blur].top, pball->m_rcScreenShadow[blur].top);
						pball->m_rcScreen[blur].right = max(pball->m_rcScreen[blur].right, pball->m_rcScreenShadow[blur].right);
						pball->m_rcScreen[blur].bottom = max(pball->m_rcScreen[blur].bottom, pball->m_rcScreenShadow[blur].bottom);
						InvalidateRect(&pball->m_rcScreen[blur]);
					}
					else
					{
						InvalidateRect(&pball->m_rcScreen[blur]);
						InvalidateRect(&pball->m_rcScreenShadow[blur]);
					}
				}
				else
				{
					InvalidateRect(&pball->m_rcScreen[blur]);
				}
			}
		}

		// Save the position of the ball.
		pball->prev_x = pball->x;
		pball->prev_y = pball->y;
		pball->prev_z = pball->z;
		pball->IsBlurReady = true;
	}

	m_pin3d.m_pd3dDevice->SetTexture(0, NULL);
	m_pin3d.m_pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, FALSE);

}


int totalrects = 0;

void Player::InvalidateRect(RECT *prc)
{
	int i;

	totalrects++;

	// This assumes the caller does not need *prc any more!!!
	// Either that, or we assume it can be permantnently changed,
	// Because we never care about redrawing stuff off the screen.
	if (prc->top < 0)
	{
		prc->top = 0;
	}

	UpdateRect *pur;
	pur = new UpdateRect();
	pur->m_rcupdate = *prc;
	pur->m_fSeeThrough = fTrue;

	// Check all animated objects.
	for (i=0;i<m_vscreenupdate.Size();i++)
	{
		// Get the bounds of this animated object.
		RECT *prc2 = &m_vscreenupdate.ElementAt(i)->m_rcBounds;

		// Check if the bounds of the animated object are within the bounds of our invalid rectangle.
		if (!((prc->right < prc2->left) || (prc->left > prc2->right) || (prc->bottom < prc2->top) || (prc->top > prc2->bottom)))
		{
			// Add to this rect's list of objects that need to be redrawn.
			pur->m_vobject.AddElement(m_vscreenupdate.ElementAt(i));
		}
	}

	// Add the rect.
	m_vupdaterect.AddElement(pur);

}


struct DebugMenuItem
{

	int objectindex;
	VectorInt<int> *pvdispid;
	HMENU hmenu;

};


void AddEventToDebugMenu(char *sz, int index, int dispid, LPARAM lparam)
{

	DebugMenuItem *pdmi = (DebugMenuItem *)lparam;
	HMENU hmenu = pdmi->hmenu;
	int menuid = ((pdmi->objectindex+1)<<16) | pdmi->pvdispid->Size();
	pdmi->pvdispid->AddElement(dispid);
	AppendMenu(hmenu, MF_STRING, menuid, sz);

}

void Player::DoDebugObjectMenu(int x, int y)
{

	int i;

	if (g_pplayer->m_vdebugho.Size() == 0)
	{
		// First time the debug hit-testing has been used
		g_pplayer->InitDebugHitStructure();
	}

	Matrix3D mat3D = g_pplayer->m_pin3d.m_matrixTotal;

	mat3D.Invert();

	Vertex3D v3d, v3d2;

	D3DVIEWPORT7 vp;
	g_pplayer->m_pin3d.m_pd3dDevice->GetViewport( &vp );
	float rClipWidth  = vp.dwWidth/2.0f;
	float rClipHeight = vp.dwHeight/2.0f;

	float xcoord = (x-rClipWidth)/rClipWidth;
	float ycoord = (-(y-rClipHeight))/rClipHeight;

	Vertex3D vT, vT2;
	g_pplayer->m_pin3d.m_matrixTotal.MultiplyVector(798,1465,89,&vT);
	mat3D.MultiplyVector(vT.x,vT.y,vT.z,&vT2);

	// Use the inverse of our 3D transform to determine where in 3D space the
	// screen pixel the user clicked on is at.  Get the point at the near
	// clipping plane (z=0) and the far clipping plane (z=1) to get the whole
	// range we need to hit test
	mat3D.MultiplyVector(xcoord,ycoord,0,&v3d);
	mat3D.MultiplyVector(xcoord,ycoord,1,&v3d2);

	// Create a ray (ball) that travels in 3D space from the screen pixel at
	// the near clipping plane to the far clipping plane, and find what
	// it intersects with.
	Ball ballT;
	ballT.x = v3d.x;
	ballT.y = v3d.y;
	ballT.z = v3d.z;
	ballT.vx = v3d2.x - v3d.x;
	ballT.vy = v3d2.y - v3d.y;
	ballT.vz = v3d2.z - v3d.z;
	ballT.radius = 0;
	ballT.m_hittime = 1;
	ballT.CalcBoundingRect();

	float slope = (v3d2.y - v3d.y)/(v3d2.z - v3d.z);
	float yhit = v3d.y - (v3d.z*slope);

	float slopex = (v3d2.x - v3d.x)/(v3d2.z - v3d.z);
	float xhit = v3d.x - (v3d.z*slopex);

	Vector<HitObject> vhoHit;
	Vector<IFireEvents> vpfe;

	g_pplayer->m_hitoctree.HitTestXRay(&ballT, &vhoHit);
	g_pplayer->m_debugoctree.HitTestXRay(&ballT, &vhoHit);

	VectorInt<HMENU> vsubmenu;
	HMENU hmenu = CreatePopupMenu();

	Vector<VectorInt<int> > vvdispid;

	if (vhoHit.Size() == 0)
	{
		// Nothing was hit-tested
		return;
	}

	PauseMusic();

	for (i=0;i<vhoHit.Size();i++)
	{
		HitObject *pho = vhoHit.ElementAt(i);
		// Make sure we don't do the same object twice through 2 different Hitobjs.
		if (pho->m_pfedebug && (vpfe.IndexOf(pho->m_pfedebug) == -1))
		{
			vpfe.AddElement(pho->m_pfedebug);
			CComVariant var;
			DISPPARAMS dispparams  = {
				NULL,
					NULL,
					0,
					0
			};
			HRESULT hr = pho->m_pfedebug->GetDispatch()->Invoke(
				0x80010000, IID_NULL,
				LOCALE_USER_DEFAULT,
				DISPATCH_PROPERTYGET,
				&dispparams, &var, NULL, NULL);

			HMENU submenu = CreatePopupMenu();
			vsubmenu.AddElement(submenu);
			if (hr == S_OK)
			{
				WCHAR *wzT;
				wzT = V_BSTR(&var);
				AppendMenuW(hmenu, MF_STRING | MF_POPUP, (UINT_PTR)submenu, wzT);

				VectorInt<int> *pvdispid = new VectorInt<int>();
				vvdispid.AddElement(pvdispid);

				DebugMenuItem dmi;
				dmi.objectindex = i;
				dmi.pvdispid = pvdispid;
				dmi.hmenu = submenu;
				EnumEventsFromDispatch(pho->m_pfedebug->GetDispatch(), AddEventToDebugMenu, (LPARAM)&dmi);
			}

			IDebugCommands *pdc = pho->m_pfedebug->GetDebugCommands();
			if (pdc)
			{
				VectorInt<int> vids;
				VectorInt<int> vcommandid;

				pdc->GetDebugCommands(&vids, &vcommandid);
				int l;
				for (l=0;l<vids.Size();l++)
				{
					LocalString ls(vids.ElementAt(l));
					AppendMenu(submenu, MF_STRING, ((i+1)<<16) | vcommandid.ElementAt(l) | 0x8000, ls.m_szbuffer);
				}
			}
		}
		else
		{
			vvdispid.AddElement(NULL); // Put a spacer in so we can keep track of indexes
		}
	}

	POINT pt;

	pt.x = x;
	pt.y = y;
	ClientToScreen(m_hwnd, &pt);

	int icmd = TrackPopupMenuEx(hmenu, TPM_RETURNCMD | TPM_RIGHTBUTTON,
		pt.x, pt.y, m_hwnd, NULL);

	if (icmd != 0 && vsubmenu.Size() > 0)
	{
		int highword = HIWORD(icmd) - 1;
		int lowword = icmd & 0xffff;
		IFireEvents *pfe = vhoHit.ElementAt(highword)->m_pfedebug;
		if (lowword & 0x8000) // custom debug command
		{
			pfe->GetDebugCommands()->RunDebugCommand(lowword & 0x7fff);
		}
		else
		{
			int dispid = vvdispid.ElementAt(highword)->ElementAt(lowword);
			g_pplayer->m_pactiveball = m_pactiveballDebug;
			pfe->FireGroupEvent(dispid);
			g_pplayer->m_pactiveball = NULL;
		}
	}

	DestroyMenu(hmenu);
	for (i=0;i<vsubmenu.Size();i++)
	{
		DestroyMenu(vsubmenu.ElementAt(i));
	}

	for (i=0;i<vvdispid.Size();i++)
	{
		if (vvdispid.ElementAt(i))
		{
			delete vvdispid.ElementAt(i);
		}
	}

	UnpauseMusic();
}


LRESULT CALLBACK PlayerDMDHackWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

	switch ( uMsg )
	{
	case MM_MIXM_CONTROL_CHANGE:
		break;

	case WM_CLOSE:
		break;

	case WM_DESTROY:
		break;

	case WM_PAINT:
		PAINTSTRUCT Paint;

		// Fill the window with black.
		BeginPaint ( hwnd, &Paint );
		PatBlt ( Paint.hdc,0, 0, 86, 512, BLACKNESS ); 
		EndPaint ( hwnd, &Paint );
		break;

	case WM_KEYDOWN:
		break;

	case WM_MOUSEMOVE:
		break;

	case WM_MOVE:
		break;

	case WM_LBUTTONDOWN:
		break;

	case WM_RBUTTONDOWN:
		break;

	case WM_RBUTTONUP:
		break;

	case WM_ACTIVATE:
		break;

	case WM_ENABLE:
		break;

	case WM_EXITMENULOOP:
		break;

	case WM_SETCURSOR:
		break;

	case WM_USER:
		break;
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);

}



LRESULT CALLBACK PlayerWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

	switch (uMsg)
	{
	case MM_MIXM_CONTROL_CHANGE:
		mixer_get_volume();
		mixer_display_volume();
		break;

	case WM_CLOSE:
		// Close the DMD hack window.
		SendMessage(g_pplayer->m_dmdhackhwnd, WM_CLOSE, 0, 0);
		break;

	case WM_DESTROY:
		if (g_pplayer->m_pcsimpleplayer)
		{
			g_pplayer->m_pcsimpleplayer->Pause();
		}
		if (g_pplayer->m_pxap)
		{
			g_pplayer->m_pxap->Pause();
		}
		// signal the script that the game is now exit to allow any cleanup
		g_pplayer->m_ptable->FireVoidEvent(DISPID_GameEvents_Exit);
		if (g_pplayer->m_fDetectScriptHang)
		{
			g_pvp->PostWorkToWorkerThread(HANG_SNOOP_STOP, NULL);
		}
		g_pplayer->m_ptable->StopPlaying();
		g_pplayer->m_pininput.UnInit();
		delete g_pplayer;
		g_pplayer = NULL;

		g_pvp->SetEnableToolbar();
		g_pvp->SetEnableMenuItems();
		mixer_shutdown();
		break;

	case WM_PAINT:
		g_pplayer->m_pin3d.Flip(0,0);
		break;

	case WM_KEYDOWN:
		g_pplayer->m_fDrawCursor = fFalse;
		SetCursor(NULL);
		break;

	case WM_MOUSEMOVE:
		if (g_pplayer->m_lastcursorx != LOWORD(lParam) || g_pplayer->m_lastcursory != HIWORD(lParam))
		{
			g_pplayer->m_fDrawCursor = fTrue;
			g_pplayer->m_lastcursorx = LOWORD(lParam);
			g_pplayer->m_lastcursory = HIWORD(lParam);
		}
		break;

	case WM_MOVE:
		g_pplayer->m_pin3d.SetUpdatePos(LOWORD(lParam), HIWORD(lParam));
		break;

#ifdef MOUSEPAUSE
	case WM_LBUTTONDOWN:
		if (g_pplayer->m_fPause)
		{
			g_pplayer->m_fStep = fTrue;
		}
		break;

	case WM_RBUTTONDOWN:
		if (!g_pplayer->m_fPause)
		{
			g_pplayer->m_fPause = fTrue;

			g_pplayer->m_fGameWindowActive = fFalse;	//rlc  added
			g_pplayer->RecomputePauseState();			//rlc added
			g_pplayer->RecomputePseudoPauseState();		//rlc added
		}
		else
		{
			g_pplayer->m_fPause = fFalse;

			g_pplayer->m_fGameWindowActive = fTrue;	//rlc  added
			SetCursor(NULL);						//rlc  added
			g_pplayer->m_fNoTimeCorrect = fTrue;	//rlc  added
			g_pplayer->m_fCleanBlt = fFalse;		//rlc  added
		}
		break;
#endif

	case WM_RBUTTONUP:
		{
			if (g_pplayer->m_fDebugMode)
			{
				int x = lParam & 0xffff;
				int y = (lParam>>16) & 0xffff;
				g_pplayer->DoDebugObjectMenu(x, y);
			}
			return 0;
		}
		break;

	case WM_ACTIVATE:
		if (wParam != WA_INACTIVE)
		{
			g_pplayer->m_fGameWindowActive = fTrue;
			SetCursor(NULL);
			g_pplayer->m_fNoTimeCorrect = fTrue;
			g_pplayer->m_fPause = fFalse;
			g_pplayer->m_fCleanBlt = fFalse;
		}
		else
		{
			g_pplayer->m_fGameWindowActive = fFalse;
			g_pplayer->m_fPause = fTrue;
		}
		g_pplayer->RecomputePauseState();
		break;

	case WM_EXITMENULOOP:
		g_pplayer->m_fNoTimeCorrect = fTrue;
		break;

	case WM_SETCURSOR:
		if (LOWORD(lParam) == HTCLIENT && !g_pplayer->m_fDrawCursor)
		{
			SetCursor(NULL);
		}
		else
		{
			SetCursor(LoadCursor(NULL, IDC_ARROW));
		}
		return TRUE;
		break;

	case WM_USER:
		// Custom user message.  
		LRESULT ReturnCode = FALSE;

		// Process based on type.
		switch ( wParam )
		{
		case WINDOWMESSAGE_ADDUNITS:
			// Coin message from the front end (aka "credit manager").
			if ( g_pplayer != NULL )
			{
				if ( g_pplayer->m_ptable != NULL )
				{
					// Check parameter.
					if ( (lParam > 0) && (lParam < 10) )
					{
						// Add the coins.
						Coins = Coins + lParam;
						ReturnCode = TRUE;
					}
					else
					{
						// Print an error.
						OutputDebugString ( "Autocoin: Invalid parameter." );
					}
				}
			}
			break;
		}

		return ReturnCode;
		break;
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);

}


void AssignIconToButton(HWND hwnd, int controlid, int resourceid)
{

	HWND hwndButton = GetDlgItem(hwnd, controlid);
	HICON hicon = (HICON)LoadImage(g_hinst, MAKEINTRESOURCE(resourceid), IMAGE_ICON, 16, 16, 0);
	SendMessage(hwndButton, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hicon);

}


TBBUTTON const g_tbbuttonDebug[] = {
	{0, IDC_PLAY, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, IDS_PLAY, 0},
	{1, IDC_PAUSE, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, IDS_PAUSE, 1},
	{2, IDC_STEP, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, IDS_STEP, 2},
};


int CALLBACK DebuggerProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

	switch (uMsg)
	{
	case WM_INITDIALOG:
		{
			RECT rcDialog;
			RECT rcMain;
			GetWindowRect(GetParent(hwndDlg), &rcMain);
			GetWindowRect(hwndDlg, &rcDialog);

			SetWindowPos(hwndDlg, NULL,
				(rcMain.right + rcMain.left)/2 - (rcDialog.right - rcDialog.left)/2,
				(rcMain.bottom + rcMain.top)/2 - (rcDialog.bottom - rcDialog.top)/2,
				0, 0, SWP_NOOWNERZORDER | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE/* | SWP_NOMOVE*/);

			AssignIconToButton(hwndDlg, IDC_PLAY, IDI_PLAY);
			AssignIconToButton(hwndDlg, IDC_PAUSE, IDI_PAUSE);
			AssignIconToButton(hwndDlg, IDC_STEP, IDI_STEP);

			SendMessage(hwndDlg, RECOMPUTEBUTTONCHECK, 0, 0);

			if (!g_pplayer->m_ptable->CheckPermissions(DISABLE_SCRIPT_EDITING))
			{
				RECT rcEditSize;
				HWND hwndEditSize = GetDlgItem(hwndDlg, IDC_EDITSIZE);
				GetWindowRect(hwndEditSize, &rcEditSize);

				ScreenToClient(hwndDlg, (POINT *)&rcEditSize);
				ScreenToClient(hwndDlg, &((POINT *)&rcEditSize)[1]);

				g_pplayer->m_hwndDebugOutput = CreateWindowEx(0, "Scintilla", "",
					WS_CHILD | ES_NOHIDESEL | WS_VISIBLE | ES_SUNKEN | WS_HSCROLL | WS_VSCROLL | ES_MULTILINE | ES_WANTRETURN | WS_BORDER,
					rcEditSize.left, rcEditSize.top, rcEditSize.right - rcEditSize.left, rcEditSize.bottom - rcEditSize.top, hwndDlg, NULL, g_hinst, 0);

				SendMessage(g_pplayer->m_hwndDebugOutput, SCI_STYLESETSIZE, 32, 10);
				SendMessage(g_pplayer->m_hwndDebugOutput, SCI_STYLESETFONT, 32, (LPARAM)"Courier");

				SendMessage(g_pplayer->m_hwndDebugOutput, SCI_SETMARGINWIDTHN, 1, 0);

				SendMessage(g_pplayer->m_hwndDebugOutput, SCI_SETTABWIDTH, 4, 0);
			}
			else
			{
				HWND hwndExpand = GetDlgItem(hwndDlg, IDC_EXPAND);
				ShowWindow(hwndExpand, SW_HIDE);
			}

			SendMessage(hwndDlg, RESIZE_FROM_EXPAND, 0, 0);

			return TRUE;
		}
		break;

	case WM_NOTIFY:
		{
			int idCtrl = (int) wParam;
			NMHDR *pnmh = (LPNMHDR) lParam;
			SCNotification *pscnmh = (SCNotification *)lParam;
			HWND hwndRE = pnmh->hwndFrom;
			int code = pnmh->code;

			switch (code)
			{
			case SCN_CHARADDED:
				if (pscnmh->ch == '\n')
				{
					SendMessage(pnmh->hwndFrom, SCI_DELETEBACK, 0, 0);

					int curpos = SendMessage(pnmh->hwndFrom, SCI_GETCURRENTPOS, 0, 0);
					int line = SendMessage(pnmh->hwndFrom, SCI_LINEFROMPOSITION, curpos, 0);
					int lineStart = SendMessage(pnmh->hwndFrom, SCI_POSITIONFROMLINE, line, 0);
					int lineEnd = SendMessage(pnmh->hwndFrom, SCI_GETLINEENDPOSITION, line, 0);

					char *szText = new char[lineEnd - lineStart + 1];
					TextRange tr;
					tr.chrg.cpMin = lineStart;
					tr.chrg.cpMax = lineEnd;
					tr.lpstrText = szText;
					SendMessage(pnmh->hwndFrom, SCI_GETTEXTRANGE, 0, (LPARAM)&tr);

					int maxlines = SendMessage(pnmh->hwndFrom, SCI_GETLINECOUNT, 0, 0);

					if (maxlines == line+1)
					{
						// need to add a new line to the end
						SendMessage(pnmh->hwndFrom, SCI_DOCUMENTEND, 0, 0);
						SendMessage(pnmh->hwndFrom, SCI_ADDTEXT, lstrlen("\n"), (LPARAM)"\n");
					}
					else
					{
						int pos = SendMessage(pnmh->hwndFrom, SCI_POSITIONFROMLINE, line+1, 0);
						SendMessage(pnmh->hwndFrom, SCI_SETCURRENTPOS, pos, 0);	
					}

					g_pplayer->m_ptable->m_pcv->EvaluateScriptStatement(szText);
					delete szText;
				}
				break;
			}
		}
		break;

	case RECOMPUTEBUTTONCHECK:
		{
			int PlayDown = BST_UNCHECKED;
			int PauseDown = BST_UNCHECKED;
			int StepDown = BST_UNCHECKED;

			if (g_pplayer->m_fUserDebugPaused)
			{
				PauseDown = BST_CHECKED;
			}
			else if (g_pplayer->m_PauseTimeTarget > 0)
			{
				StepDown = BST_CHECKED;
			}
			else
			{
				PlayDown = BST_CHECKED;
			}

			HWND hwndTBParent = GetDlgItem(hwndDlg, IDC_TOOLBARSIZE);
			HWND hwndToolbar = GetWindow(hwndTBParent, GW_CHILD);

			SendDlgItemMessage(hwndDlg, IDC_PLAY, BM_SETCHECK, PlayDown, 0);
			SendDlgItemMessage(hwndDlg, IDC_PAUSE, BM_SETCHECK, PauseDown, 0);
			SendDlgItemMessage(hwndDlg, IDC_STEP, BM_SETCHECK, StepDown, 0);
			SendMessage(hwndToolbar,TB_CHECKBUTTON,IDC_PLAY,PlayDown);
			SendMessage(hwndToolbar,TB_CHECKBUTTON,IDC_PAUSE,PauseDown);
			SendMessage(hwndToolbar,TB_CHECKBUTTON,IDC_STEP,StepDown);
		}
		break;

	case WM_CLOSE:
		g_pplayer->m_PauseTimeTarget = 0;
		g_pplayer->m_fUserDebugPaused = fFalse;
		g_pplayer->RecomputePseudoPauseState();
		g_pplayer->m_fDebugMode = fFalse;
		ShowWindow(hwndDlg, SW_HIDE);
		break;

	case WM_ACTIVATE:
		if (wParam != WA_INACTIVE)
		{
			g_pplayer->m_fDebugWindowActive = fTrue;
		}
		else
		{
			g_pplayer->m_fDebugWindowActive = fFalse;
		}
		g_pplayer->RecomputePauseState();
		g_pplayer->RecomputePseudoPauseState();
		break;

	case RESIZE_FROM_EXPAND:
		{
			int state = SendDlgItemMessage(hwndDlg, IDC_EXPAND, BM_GETCHECK, 0, 0);
			HWND hwndSizer1 = GetDlgItem(hwndDlg, IDC_GUIDE1);
			HWND hwndSizer2 = GetDlgItem(hwndDlg, IDC_GUIDE2);
			int mult;

			if (state == BST_CHECKED)
			{
				mult = 1;
				SetWindowText(GetDlgItem(hwndDlg, IDC_EXPAND), "<");
			}
			else
			{
				mult = -1;
				SetWindowText(GetDlgItem(hwndDlg, IDC_EXPAND), ">");
			}

			RECT rcSizer1;
			RECT rcSizer2;
			GetWindowRect(hwndSizer1, &rcSizer1);
			GetWindowRect(hwndSizer2, &rcSizer2);

			int diffx = rcSizer2.right - rcSizer1.right;
			int diffy = rcSizer2.bottom - rcSizer1.bottom;

			RECT rcDialog;
			GetWindowRect(hwndDlg, &rcDialog);

			SetWindowPos(hwndDlg, NULL,
				rcDialog.left,
				rcDialog.top,
				rcDialog.right - rcDialog.left + diffx*mult, rcDialog.bottom - rcDialog.top + diffy*mult, SWP_NOOWNERZORDER | SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
		}
		break;

	case WM_COMMAND:
		switch (HIWORD(wParam))
		{
		case BN_CLICKED:
			switch (LOWORD(wParam))
			{
			case IDC_PLAY:
				g_pplayer->m_PauseTimeTarget = 0;
				g_pplayer->m_fUserDebugPaused = fFalse;
				g_pplayer->RecomputePseudoPauseState();
				SendMessage(hwndDlg, RECOMPUTEBUTTONCHECK, 0, 0);
				break;

			case IDC_PAUSE:
				g_pplayer->m_PauseTimeTarget = 0;
				g_pplayer->m_fUserDebugPaused = fTrue;
				g_pplayer->RecomputePseudoPauseState();
				SendMessage(hwndDlg, RECOMPUTEBUTTONCHECK, 0, 0);
				break;

			case IDC_STEP:
				{
					int ms = GetDlgItemInt(hwndDlg, IDC_STEPAMOUNT, NULL, FALSE);
					g_pplayer->m_PauseTimeTarget = g_pplayer->m_timeCur + ms;
					g_pplayer->m_fUserDebugPaused = fFalse;
					g_pplayer->RecomputePseudoPauseState();
					SendMessage(hwndDlg, RECOMPUTEBUTTONCHECK, 0, 0);
				}
				break;

			case IDC_EXPAND:
				SendMessage(hwndDlg, RESIZE_FROM_EXPAND, 0, 0);
				break;
			}
		}
		break;
	}

	return FALSE;
}

int CALLBACK PauseProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

	switch (uMsg)
	{
	case WM_INITDIALOG:

		RECT rcDialog;
		RECT rcMain;
		GetWindowRect(GetParent(hwndDlg), &rcMain);
		GetWindowRect(hwndDlg, &rcDialog);

		SetWindowPos(hwndDlg, NULL,
			(rcMain.right + rcMain.left)/2 - (rcDialog.right - rcDialog.left)/2,
			(rcMain.bottom + rcMain.top)/2 - (rcDialog.bottom - rcDialog.top)/2,
			0, 0, SWP_NOOWNERZORDER | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE/* | SWP_NOMOVE*/);

		return TRUE;
		break;

	case WM_COMMAND:
		switch (HIWORD(wParam))
		{
		case BN_CLICKED:
			switch (LOWORD(wParam))
			{
			case ID_RESUME:
				EndDialog(hwndDlg, ID_RESUME);
				break;

			case ID_DEBUGWINDOW:
				if (g_pplayer->m_ptable->CheckPermissions(DISABLE_DEBUGGER))
				{
					EndDialog(hwndDlg, ID_RESUME);
				}
				else
				{
					g_pplayer->m_fDebugMode = fTrue;
					if (g_pplayer->m_hwndDebugger)
					{
						ShowWindow(g_pplayer->m_hwndDebugger, SW_SHOW);
						SetActiveWindow(g_pplayer->m_hwndDebugger);
					}
					else
					{
						g_pplayer->m_hwndDebugger = CreateDialogParam(g_hinst, MAKEINTRESOURCE(IDD_DEBUGGER), g_pplayer->m_hwnd, DebuggerProc, NULL);
					}
					EndDialog(hwndDlg, ID_DEBUGWINDOW);
				}
				break;

			case ID_QUIT:
				EndDialog(hwndDlg, ID_QUIT);
				break;
			}
		}
		break;
	}

	return FALSE;

}


// Draws all transparent ramps that are in the vicinity of the ball.
void Player::DrawAcrylics ( void )
{

	// Check if we are hardware accelerated.
	if (g_pvp->m_pdd.m_fHardwareAccel == fTrue)
	{
		// Build a set of clipping planes which tightly bound the ball.
		int			i;
		HRESULT		ReturnCode;

		// Turn off z writes for same values.  It fixes the problem of ramps rendering twice. 
		ReturnCode = g_pplayer->m_pin3d.m_pd3dDevice->SetRenderState(D3DRENDERSTATE_ZFUNC,D3DCMP_LESS);

		// Draw acrylic ramps (they have transparency, so they have to be drawn last).
		for (i=0;i<m_ptable->m_vedit.Size();i++)
		{
			if (m_ptable->m_vedit.ElementAt(i)->GetItemType() == eItemRamp)
			{
				Hitable *ph = m_ptable->m_vedit.ElementAt(i)->GetIHitable();
				if (ph)
				{
					ph->PostRenderStatic(m_pin3d.m_pd3dDevice);
				}
			}
		}
	}
}


// Performs special draws... ok, hacks!
// These are to address shortcomings in not having
// a proper backglass display that can be animated.
void Player::DrawLightHack ( void )
{

	int		i;

	// Check all lights.
	for ( i=LIGHTHACK_FIREPOWER_P1; i<=LIGHTHACK_FIREPOWER_P4; i++ )
	{
		// Check if there are no (or extra) balls in play.
		if ( (Ball::NumInitted() - PinTable::NumStartBalls()) != 1 )
		{
			// Turn off blinking.
			m_LightHackCurrentState[i] = FALSE;
		}

		// Check if we are ready for an update.
		if ( g_pplayer->m_LightHackReadyForDrawLightHackFn[i] == TRUE )
		{
			// Check if the light is on.
			if ( m_LightHackCurrentState[i] == TRUE )
			{
				// Update the blink animation.
				if ( (msec() & 256) > 0 )
				{
					m_LightHackCurrentAnimState[i] = TRUE;
				}
				else
				{
					m_LightHackCurrentAnimState[i] = FALSE;
				}

				// Check if the animation state changed.
				if ( m_LightHackPreviousAnimState[i] != m_LightHackCurrentAnimState[i] )
				{
					// Check if we are on.
					if ( m_LightHackCurrentAnimState[i] == TRUE )
					{
						// Show the window.
						SetWindowPos ( m_dmdhackhwnd, HWND_TOPMOST, m_LightHackX[i], m_LightHackY[i], m_LightHackWidth[i], m_LightHackHeight[i], (SWP_SHOWWINDOW | SWP_NOACTIVATE) );
					}
					else
					{
						// Hide the window.
						ShowWindow ( m_dmdhackhwnd, SW_HIDE );
					}
				}
				m_LightHackPreviousAnimState[i] = m_LightHackCurrentAnimState[i];
			}
			else
			{
				// Check if the state changed.
				if ( m_LightHackPreviousState[i] != m_LightHackCurrentState[i] )
				{
					// Hide the window.
					ShowWindow ( m_dmdhackhwnd, SW_HIDE );
				}

				// Check if the state changed.
				if ( (m_LightHackCurrentAnimState[i] == TRUE) ||
					(m_LightHackPreviousAnimState[i] == TRUE) )
				{
					// Hide the window.
					ShowWindow ( m_dmdhackhwnd, SW_HIDE );
					m_LightHackCurrentAnimState[i] = FALSE;
					m_LightHackPreviousAnimState[i] = FALSE;
				}
			}

			// Save the current state.
			m_LightHackPreviousState[i] = m_LightHackCurrentState[i];

			// Clear the light state.  If it's really on, it will be refreshed by Surface::put_IsDropped().
			m_LightHackCurrentState[i] = FALSE;
			m_LightHackReadyForDrawLightHackFn[i] = FALSE;
		}
		else
		{
			// Check if we got an update from IsDropped a while ago, but never updated the visual.
			// This happens when the light changes state from on to off.
			if ( (m_LightHackPreviousState[i] == TRUE) &&
				((msec() - m_LastUpdateTime[i]) > 500) )
			{
				// Flag that it's safe to update.  We'll update on the next frame.
				m_LightHackReadyForDrawLightHackFn[i] = TRUE;
			}
		}
	}

	// Check all dispreels.
	// These behave differently, in that their state remains persistent until
	// the reel is updated to the next interval.  So we need some extra hack
	// logic to turn off the blinking after a few seconds.
	for ( i=LIGHTHACK_STRIKESANDSPARES_P1; i<=LIGHTHACK_STRIKESANDSPARES_P4; i++ )
	{
		// Check if there are no (or extra) balls in play.
		if ( (Ball::NumInitted() - PinTable::NumStartBalls()) != 1 )
		{
			// Turn off blinking.
			m_LightHackCurrentState[i] = FALSE;
		}

		// Check if the light is on.
		if ( m_LightHackCurrentState[i] == TRUE )
		{
			// Update the blink animation.
			if ( (msec() & 256) > 0 )
			{
				m_LightHackCurrentAnimState[i] = TRUE;
			}
			else
			{
				m_LightHackCurrentAnimState[i] = FALSE;
			}

			// Check if the animation state changed.
			if ( m_LightHackPreviousAnimState[i] != m_LightHackCurrentAnimState[i] )
			{
				// Check if we are on.
				if ( m_LightHackCurrentAnimState[i] == TRUE )
				{
					// Show the window.
					SetWindowPos ( m_dmdhackhwnd, HWND_TOPMOST, m_LightHackX[i], m_LightHackY[i], m_LightHackWidth[i], m_LightHackHeight[i], (SWP_SHOWWINDOW | SWP_NOACTIVATE) );
				}
				else
				{
					// Hide the window.
					ShowWindow ( m_dmdhackhwnd, SW_HIDE );
				}
			}
			m_LightHackPreviousAnimState[i] = m_LightHackCurrentAnimState[i];
		}
		else
		{
			// Check if the state changed.
			if ( m_LightHackPreviousState[i] != m_LightHackCurrentState[i] )
			{
				// Hide the window.
				ShowWindow ( m_dmdhackhwnd, SW_HIDE );
			}
		}

		// Save the current state.
		m_LightHackPreviousState[i] = m_LightHackCurrentState[i];
	}

}




