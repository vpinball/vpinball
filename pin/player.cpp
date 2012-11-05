#include "stdafx.h"

#ifdef VP3D
 #include "..\stereo3D.h"
#endif

#include "..\DongleAPI.h"

#define RECOMPUTEBUTTONCHECK WM_USER+100
#define RESIZE_FROM_EXPAND WM_USER+101

#if _MSC_VER <= 1310 // VC 2003 and before
inline bool fopen_s(FILE** f, const char *fname, const char *attr)
{
	*f = fopen(fname, attr);
	return (*f == NULL);
}
#endif

float curMechPlungerPos;

LRESULT CALLBACK PlayerWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK PlayerDMDHackWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int CALLBACK PauseProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

int CALLBACK DebuggerProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

U32 c_hitcnts = 0;
U32 c_collisioncnt = 0;
U32 c_contactcnt = 0;
U32 c_staticcnt = 0;
U32 c_embedcnts = 0;
static int movedPlunger = 0;						// has plunger moved, must have moved at least three times
U32 LastPlungerHit = 0;								// The last time the plunger was in contact (at least the vicinity) of the ball.
int Coins = 0;										// The number of coins queued to be inserted.  These were sent from the shell after the load.


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

	m_fAccelerometer = fTrue;	// true if electronic Accelerometer enabled 
	m_AccelNormalMount = fTrue;	// fTrue normal mounting (left hand coordinates)
	m_AccelAngle = 0;			// 0 degrees (GUI is lefthand coordinates)
	m_AccelAmp = 1.5f;			// Accelerometer gain 
	m_AccelAmpX = m_AccelAmp;	// Accelerometer gain X axis
	m_AccelAmpY = m_AccelAmp;   // Accelerometer gain Y axis
	m_AccelMAmp = 2.5f;			// manual input gain, generally from joysticks

#ifdef PLAYBACK
	m_fPlayback = fFalse;

	m_fplaylog = NULL;
#endif

	m_frotate = fFalse;

#ifdef LOG
	m_flog = NULL;
#endif

	m_sleeptime = 0;

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

	int vsync;
	hr = GetRegInt("Player", "AdaptiveVSync", &vsync);
	if (hr != S_OK)
		{
		vsync = fFalse; // The default
		}
	m_fVSync = (vsync == 1);

	int stereo3D;
	hr = GetRegInt("Player", "Stereo3D", &stereo3D);
	if (hr != S_OK)
		{
		stereo3D = fFalse; // The default
		}
	m_fStereo3D = (stereo3D == 1);

	int stereo3Denabled;
	hr = GetRegInt("Player", "Stereo3DEnabled", &stereo3Denabled);
	if (hr != S_OK)
		{
		stereo3Denabled = m_fStereo3D; // The default
		}
	m_fStereo3Denabled = (stereo3Denabled == 1);

	int stereo3DAA;
	hr = GetRegInt("Player", "Stereo3DAntialias", &stereo3DAA);
	if (hr != S_OK)
		{
		stereo3DAA = fFalse; // The default
		}
	m_fStereo3DAA = (stereo3DAA == 1);

	int stereo3DY;
	hr = GetRegInt("Player", "Stereo3DYAxis", &stereo3DY);
	if (hr != S_OK)
		{
		stereo3DY = fFalse; // The default
		}
	m_fStereo3DY = (stereo3DY == 1);
	
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
	if (!fopen_s(&foo, "c:\\debug1.txt","r"))
		{
		fclose(foo);
		m_fCheckBlt = fFalse;
		}
	if (!fopen_s(&foo, "c:\\debug2.txt","r"))
		{
		fclose(foo);
		m_fCheckBlt = fTrue;
		}
	if (!fopen_s(&foo, "c:\\debug3.txt","r"))
		{
		fclose(foo);
		m_fWasteTime = fTrue;
		}
	if (!fopen_s(&foo, "c:\\debug4.txt","r"))
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
	m_fCloseType = 0;

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
	m_NudgeManual = -1;
	}

Player::~Player()
	{
	for (int i=0;i<m_ptable->m_vedit.Size();i++)
		{
		Hitable * const ph = m_ptable->m_vedit.ElementAt(i)->GetIHitable();
		if (ph)
			{
			ph->EndPlay();
			}
		}

	for (int i=0;i<m_vho.Size();i++)
		{
		delete m_vho.ElementAt(i);
		}
	m_vho.RemoveAllElements();

	for (int i=0;i<m_vdebugho.Size();i++)
		{
		delete m_vdebugho.ElementAt(i);
		}
	m_vdebugho.RemoveAllElements();

	// balls get deleted by the hit object vector
	// not anymore - balls are added to the octree, but not the main list
	for (int i=0;i<m_vball.Size();i++)
		{
		Ball * const pball = m_vball.ElementAt(i);
		if (pball->m_pballex)
			{
			pball->m_pballex->m_pball = NULL;
			pball->m_pballex->Release();
			}

		delete pball->m_vpVolObjs;
		delete pball;
		}
		m_vball.RemoveAllElements();

#ifdef GDIDRAW
	DeleteObject(m_hbmOffScreen);
#endif

#ifdef LOG
	if (m_flog)
		{
		fclose(m_flog);
		}
#endif

#ifdef PLAYBACK
	if (m_fplaylog)
		{
		fclose(m_fplaylog);
		}
#endif

	CloseHandle(m_hSongCompletionEvent);

	if (m_pxap)
		{
		delete m_pxap;
		m_pxap = NULL;
		}

	for (int i=0;i<m_controlclsidsafe.Size();i++)
		{
		delete m_controlclsidsafe.ElementAt(i);
		}
	m_controlclsidsafe.RemoveAllElements();
	}

void Player::ToggleFPS()
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

void Player::EnableFPS()
{
	ToggleFPS();
	m_fShowFPS = 1;
}

void Player::DisableFPS()
{
	ToggleFPS();
	m_fShowFPS = 0;
}

void Player::RecomputePauseState()
	{
	const BOOL fOldPause = m_fPause;
	const BOOL fNewPause = !(m_fGameWindowActive || m_fDebugWindowActive);// || m_fUserDebugPaused;
	
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
	const BOOL fOldPseudoPause = m_fPseudoPause;
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

	Vertex3Ds rgv3D[4] = {
		Vertex3Ds(m_ptable->m_left,m_ptable->m_top,m_ptable->m_glassheight),
		Vertex3Ds(m_ptable->m_right,m_ptable->m_top,m_ptable->m_glassheight),
		Vertex3Ds(m_ptable->m_right,m_ptable->m_bottom,m_ptable->m_glassheight),
		Vertex3Ds(m_ptable->m_left,m_ptable->m_bottom,m_ptable->m_glassheight)};

	Hit3DPoly * const ph3dpoly = new Hit3DPoly(rgv3D,4,false);

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
		key = DIK_4;
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

	hr = GetRegInt("Player","FrameCount", &key);
	if (hr != S_OK || key > 0xdd)
		{
		key = DIK_F11;
		}
	m_rgKeys[eFrameCount] = (EnumAssignKeys)key;

	hr = GetRegInt("Player","VolumeUp", &key);
	if (hr != S_OK || key > 0xdd)
		{
		key = DIK_EQUALS;
		}
	m_rgKeys[eVolumeUp] = (EnumAssignKeys)key;

	hr = GetRegInt("Player","VolumeDown", &key);
	if (hr != S_OK || key > 0xdd)
		{
		key = DIK_MINUS;
		}
	m_rgKeys[eVolumeDown] = (EnumAssignKeys)key;

	hr = GetRegInt("Player","DebugBalls", &key);
	if (hr != S_OK || key > 0xdd)
		{
		key = DIK_O;
		}
	m_rgKeys[eDBGBalls] = (EnumAssignKeys)key;

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

	hr = GetRegInt("Player","Enable3DKey", &key);
	if (hr != S_OK || key > 0xdd)
		{
		key = DIK_F10;
		}
	m_rgKeys[eEnable3D] = (EnumAssignKeys)key;

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
	m_fPlayMusic = fmusic;

	hr = GetRegInt("Player", "PlaySound", &fmusic);
	if (hr != S_OK)
		{
		fmusic = 1; // default value
		}
	m_fPlaySound = fmusic;

	hr = GetRegInt("Player", "MusicVolume", &m_MusicVolume);
	if (hr != S_OK)
		{
		m_MusicVolume = 100; // default value
		}
	m_MusicVolume = (int)(logf((float)m_MusicVolume)*(float)(1000.0/log(10.0)) - 2000.0f); // 10 volume = -10Db

	hr = GetRegInt("Player", "SoundVolume", &m_SoundVolume);
	if (hr != S_OK)
		{
		m_SoundVolume = 100; // default value
		}
	}

void Player::InitDebugHitStructure()
	{
	for (int i=0;i<m_ptable->m_vedit.Size();i++)
		{
		Hitable * const ph = m_ptable->m_vedit.ElementAt(i)->GetIHitable();
		if (ph)
			{
			const int currentsize = m_vdebugho.Size();
			ph->GetHitShapesDebug(&m_vdebugho);
			const int newsize = m_vdebugho.Size();
			// Save the objects the trouble of having the set the idispatch pointer themselves
			for (int hitloop = currentsize;hitloop < newsize;hitloop++)
				{
				m_vdebugho.ElementAt(hitloop)->m_pfedebug = m_ptable->m_vedit.ElementAt(i)->GetIFireEvents();
				}
			}
		}

	for (int i=0;i<m_vdebugho.Size();i++)
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

	m_debugoctree.m_vcenter.x = (m_hitoctree.m_rectbounds.left + m_hitoctree.m_rectbounds.right)*0.5f;
	m_debugoctree.m_vcenter.y = (m_hitoctree.m_rectbounds.top + m_hitoctree.m_rectbounds.bottom)*0.5f;
	m_debugoctree.m_vcenter.z = (m_hitoctree.m_rectbounds.zlow + m_hitoctree.m_rectbounds.zhigh)*0.5f;

	m_debugoctree.CreateNextLevel();
	}

HRESULT Player::Init(PinTable *ptable, HWND hwndProgress, HWND hwndProgressName, BOOL fCheckForCache)
	{
	m_ptable = ptable;

	//accelerometer normal mounting is 90 degrees in left-hand coordinates (1/4 turn counterclockwise)
	m_fAccelerometer = m_ptable->m_tblAccelerometer;		// true if electronic Accelerometer enabled m_ptable->
	m_AccelNormalMount = m_ptable->m_tblAccelNormalMount;	// true is normal mounting (left hand coordinates)
	m_AccelAngle = m_ptable->m_tblAccelAngle * (float)(M_PI/180.0); // 0 rotated counterclockwise (GUI is lefthand coordinates)
	m_AccelAmp = m_ptable->m_tblAccelAmp;					// Accelerometer gain 
	m_AccelAmpX = m_ptable->m_tblAccelAmpX;
	m_AccelAmpY = m_ptable->m_tblAccelAmpY;
	m_AccelMAmp = m_ptable->m_tblAccelManualAmp;			// manual input gain, generally from joysticks

	m_jolt_amount = (U32)m_ptable->m_jolt_amount;
	m_tilt_amount = (U32)m_ptable->m_tilt_amount;
	m_jolt_trigger_time = (U32)m_ptable->m_jolt_trigger_time;
	m_tilt_trigger_time = (U32)m_ptable->m_tilt_trigger_time;

    m_hSongCompletionEvent = CreateEvent( NULL, TRUE, FALSE, NULL );

	SendMessage(hwndProgress, PBM_SETPOS, 40, 0);
	// TEXT
	SetWindowText(hwndProgressName, "Initalizing Visuals...");

	InitWindow();
#ifdef ULTRAPIN
	InitDMDHackWindow();
#endif

	InitKeys();

	InitRegValues();

	// width, height, and colordepth are only defined if fullscreen is true.
	HRESULT hr = m_pin3d.InitDD(m_hwnd, m_fFullScreen != 0, m_screenwidth, m_screenheight, m_screendepth, m_refreshrate, !!m_fStereo3D);

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
		sprintf_s(szfoo, "Error code: %x",hr);
		ShowError(szfoo);
		return hr;
		}

	m_pininput.Init(m_hwnd);

#ifdef ULTRAPIN
	// Initialize light hack states.
	for (int i=0; i<LIGHTHACK_MAX; i++)
	{
		m_LastUpdateTime[i] = 0;
		m_LightHackReadyForDrawLightHackFn[i] = FALSE;

		m_LightHackCurrentState[i] = FALSE;
		m_LightHackPreviousState[i] = FALSE;

		m_LightHackCurrentAnimState[i] = FALSE;
		m_LightHackPreviousAnimState[i] = FALSE;
	}

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
#endif

	// Initialize render and texture states for D3D blit support.
	Display_InitializeRenderStates();
	Display_InitializeTextureStates();

	hr = m_pin3d.m_pd3dDevice->BeginScene();

	float realFOV = ptable->m_FOV;

	if (realFOV < 0.01f)
		{
		realFOV = 0.01f; // Can't have a real zero FOV, but this will look the same
		}

	m_pin3d.InitLayout(ptable->m_left, ptable->m_top, ptable->m_right,
					   ptable->m_bottom, ptable->m_inclination, realFOV,
					   ptable->m_rotation, ptable->m_scalex, ptable->m_scaley,
					   ptable->m_xlatex, ptable->m_xlatey, ptable->m_layback,
					   ptable->m_maxSeparation, ptable->m_ZPD);

	m_mainlevel.m = 0;
	m_mainlevel.n = 0;
	m_mainlevel.b = 0;

	const float slope = ptable->m_angletiltMin 
					  + (ptable->m_angletiltMax - ptable->m_angletiltMin) 
					  * ptable->m_globalDifficulty;
	
	m_mainlevel.m_gravity.x = 0;
	m_mainlevel.m_gravity.y =  sinf(ANGTORAD(slope))*ptable->m_Gravity;
	m_mainlevel.m_gravity.z = -cosf(ANGTORAD(slope))*ptable->m_Gravity;

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

	for (int i=0;i<m_ptable->m_vedit.Size();i++)
		{
		Hitable * const ph = m_ptable->m_vedit.ElementAt(i)->GetIHitable();
		if (ph)
			{
			const int currentsize = m_vho.Size();
			ph->GetHitShapes(&m_vho);
			const int newsize = m_vho.Size();
			// Save the objects the trouble of having the set the idispatch pointer themselves
			for (int hitloop = currentsize;hitloop < newsize;hitloop++)
				{
				m_vho.ElementAt(hitloop)->m_pfedebug = m_ptable->m_vedit.ElementAt(i)->GetIFireEvents();
				}
			ph->GetTimers(&m_vht);

    		if (g_pvp->m_pdd.m_fHardwareAccel)
			{
  				if ((m_ptable->m_vedit.ElementAt(i)->GetItemType() == eItemRamp && ((Ramp*)m_ptable->m_vedit.ElementAt(i))->m_d.m_fAlpha) ||
	  				m_ptable->m_vedit.ElementAt(i)->GetItemType() == eItemPrimitive)
				{
				  m_vhitacrylic.AddElement(ph);
				}
			}
			else
				if (m_ptable->m_vedit.ElementAt(i)->GetItemType() == eItemPrimitive)
				{
					m_vhitacrylic.AddElement(ph);
				}
			}
		}

	CreateBoundingHitShapes(&m_vho);

	for (int i=0;i<m_vho.Size();i++)
		{
		m_vho.ElementAt(i)->CalcHitRect();

		m_hitoctree.m_vho.AddElement(m_vho.ElementAt(i));

		if ((m_vho.ElementAt(i)->GetType() == e3DPoly) && ((Hit3DPoly *)m_vho.ElementAt(i))->m_fVisible)
			{
			m_shadowoctree.m_vho.AddElement(m_vho.ElementAt(i));
			}

		if ((m_vho.ElementAt(i)->GetAnimObject() != NULL) && m_vho.ElementAt(i)->GetAnimObject()->FMover())
			{
			m_vmover.AddElement(m_vho.ElementAt(i)->GetAnimObject());
			}
		}

#ifdef HITLOG	
	if (g_fWriteHitDeleteLog)
		{
		FILE *file = fopen("c:\\log.txt", "w"); 
		fclose(file);
		}
#endif

	m_hitoctree.m_rectbounds.left = m_ptable->m_left;
	m_hitoctree.m_rectbounds.right = m_ptable->m_right;
	m_hitoctree.m_rectbounds.top = m_ptable->m_top;
	m_hitoctree.m_rectbounds.bottom = m_ptable->m_bottom;
	m_hitoctree.m_rectbounds.zlow = m_ptable->m_tableheight;
	m_hitoctree.m_rectbounds.zhigh = m_ptable->m_glassheight;

	m_hitoctree.m_vcenter.x = (m_hitoctree.m_rectbounds.left + m_hitoctree.m_rectbounds.right)*0.5f;
	m_hitoctree.m_vcenter.y = (m_hitoctree.m_rectbounds.top + m_hitoctree.m_rectbounds.bottom)*0.5f;
	m_hitoctree.m_vcenter.z = (m_hitoctree.m_rectbounds.zlow + m_hitoctree.m_rectbounds.zhigh)*0.5f;

	m_hitoctree.CreateNextLevel();

	m_shadowoctree.m_rectbounds.left = m_ptable->m_left;
	m_shadowoctree.m_rectbounds.right = m_ptable->m_right;
	m_shadowoctree.m_rectbounds.top = m_ptable->m_top;
	m_shadowoctree.m_rectbounds.bottom = m_ptable->m_bottom;
	m_shadowoctree.m_rectbounds.zlow = m_ptable->m_tableheight;
	m_shadowoctree.m_rectbounds.zhigh = m_ptable->m_glassheight;

	m_shadowoctree.m_vcenter.x = (m_hitoctree.m_rectbounds.left + m_hitoctree.m_rectbounds.right)*0.5f;
	m_shadowoctree.m_vcenter.y = (m_hitoctree.m_rectbounds.top + m_hitoctree.m_rectbounds.bottom)*0.5f;
	m_shadowoctree.m_vcenter.z = (m_hitoctree.m_rectbounds.zlow + m_hitoctree.m_rectbounds.zhigh)*0.5f;

	m_shadowoctree.CreateNextLevel();

	hr = m_pin3d.m_pd3dDevice->EndScene();
	
	// Start Cache
	m_pin3d.m_fWritingToCache = false;
	m_pin3d.m_fReadingFromCache = false;
	m_pin3d.m_hFileCache = NULL;

	if (fCheckForCache)
		{
		const BOOL fGotCache = m_pin3d.OpenCacheFileForRead();

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
	else if (m_pin3d.m_fWritingToCache)
		{
		SetWindowText(hwndProgressName, "Writing Table To Cache...");
		}
	else
		{
		SetWindowText(hwndProgressName, "Rendering Table...");
		}

    //----------------------------------------------------------------------------------
//#define IS_ATI(DDDEVICEID) (DDDEVICEID.dwVendorId==0x1002)  //BDS
    /*if (m_pin3d.m_pd3dDevice->dwVendorID==0x1002)	//ATI
		{
		ReOrder();
		}*/
		if (g_pvp->m_pdd.m_fAlternateRender)
			{
			ReOrder();
			}
    //----------------------------------------------------------------------------------
	// Pre-render all non-changing elements such as 
	// static walls, rails, backdrops, etc.
	InitStatic(hwndProgress);

	SendMessage(hwndProgress, PBM_SETPOS, 80, 0);
	if (m_pin3d.m_fReadingFromCache)
		{
		SetWindowText(hwndProgressName, "Reading Animations From Cache...");
		}
	else if (m_pin3d.m_fWritingToCache)
		{
		SetWindowText(hwndProgressName, "Writing Animations To Cache...");
		}
	else
		{
		SetWindowText(hwndProgressName, "Rendering Animations...");
		}

	// Pre-render all elements which have animations.
	// Add the pre-rendered animations to the display list. 
	InitAnimations(hwndProgress);
	
	// End Cache
	m_pin3d.CloseCacheFile();

	///////////////// Screen Update Vector
	///// (List of movers which can be blitted at any time)
	/////////////////////////

	// Cupid for Primitives: 
	// seems to me that this is the place, where Animobjects schould be added to the m_vscreenupdate vector.
	// But the prioblem is that primitives should have an animobject first.
	// Primitives should respond to GetAnimObject now.
	// No, they dont respond to GetAnimObject and all the other elements dont either...
	// So there must be another place, where m_vho is filled... m_vho seems to be a hitobject... what does
	// this have to do with animobjects.

	for (int i=0;i<m_vho.Size();i++)
		{
		if (m_vho.ElementAt(i)->GetAnimObject() != NULL)
			{
			// Put the screenupdate vector in sorted order back to
			// front so that invalidated objects draw over each-other
			// correctly

			AnimObject * const pao = m_vho.ElementAt(i)->GetAnimObject();

			const float myz = (pao->m_znear + pao->m_zfar)*0.5f;

			int l;
			for (l=0;l<m_vscreenupdate.Size();l++)
				{
				const bool fInBack = (myz > (m_vscreenupdate.ElementAt(l)->m_znear + m_vscreenupdate.ElementAt(l)->m_zfar)*0.5f);

				if (fInBack)
					{
					//m_vscreenupdate.InsertElementAt(m_vho.ElementAt(i), l);
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

			//m_vscreenupdate.AddElement(m_vho.ElementAt(i));
			pao->m_fInvalid = fFalse;
			}
		}

	// Render inital textbox text
	for (int i=0;i<m_ptable->m_vedit.Size();i++)
		{
		if (m_ptable->m_vedit.ElementAt(i)->GetItemType() == eItemTextbox)
            {
			Textbox * const ptb = (Textbox *)m_ptable->m_vedit.ElementAt(i);
			ptb->RenderText();
			}
		}

    // Render inital dispreel(s)
	for (int i=0;i<m_ptable->m_vedit.Size();i++)
		{
		if (m_ptable->m_vedit.ElementAt(i)->GetItemType() == eItemDispReel)
			{
			DispReel * const pdr = (DispReel *)m_ptable->m_vedit.ElementAt(i);
			pdr->RenderText();
			}
		}

	m_ptable->m_pcv->Start(); // Hook up to events and start cranking script

	SetWindowText(hwndProgressName, "Starting Game Scripts...");

	m_ptable->FireVoidEvent(DISPID_GameEvents_Init);

#ifdef LOG
	m_flog = fopen("c:\\log.txt","w");
	m_timestamp = 0;
#endif

#ifdef PLAYBACK
	if (m_fPlayback)
		{
		m_fplaylog = fopen("c:\\badlog.txt","r");
		}
#endif
	
	wintimer_init();

	m_liStartTime = usec();

	m_PhysicsStepTime = 10000;
	m_nextPhysicsFrameTime = m_liStartTime + m_PhysicsStepTime;
	m_liPhysicsCalced = m_liStartTime; // haven't calced any physics yet

#ifdef PLAYBACK
	if (m_fPlayback)
		{
		ParseLog((LARGE_INTEGER*)&m_PhysicsStepTime, (LARGE_INTEGER*)&m_liStartTime);
		}
#endif

	m_curPhysicsFrameTime = m_liStartTime;

#ifdef LOG
	fprintf(m_flog, "Step Time %u %u %u %u\n", m_PhysicsStepTime>>32, m_PhysicsStepTime, m_liStartTime>>32, m_liStartTime);
	fprintf(m_flog, "End Frame\n");
#endif

	SendMessage(hwndProgress, PBM_SETPOS, 100, 0);
	// TEXT
	SetWindowText(hwndProgressName, "Starting...");

	// Check if we should show the window.
	// Show if we don't have a front end, or autostart is not enabled.
	const HWND hFrontEndWnd = FindWindow( NULL, "Ultrapin (plfe)" );
	if ( (hFrontEndWnd == NULL) || 
		 (m_ptable->m_tblAutoStartEnabled == false) )
	{
		// Show the window.
		ShowWindow(m_hwnd, SW_SHOW);
		SetForegroundWindow(m_hwnd);                      
		SetFocus(m_hwnd);
	}

	// Call Init
	for (int i=0;i<m_ptable->m_vedit.Size();i++)
		{
		Hitable * const ph = m_ptable->m_vedit.ElementAt(i)->GetIHitable();
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

	// Check if we have a valid dongle.
//	if ( get_dongle_status() != DONGLE_STATUS_OK )
//	{
		// Exit the application.
//		ExitApp();
//	}
	Render();
	return S_OK;
	}

void Player::ReOrder() // Reorder playfield objects for ATI configurations
{
	for (int i=0;i<m_ptable->m_vedit.Size();i++) //fix render walls
		{
		if (m_ptable->m_vedit.ElementAt(i)->GetItemType() == eItemSurface)
			{
			const Surface * const psurface = ((Surface *)m_ptable->m_vedit.ElementAt(i));
			if (psurface->m_d.m_fDroppable)
				{
				IEditable * const piedit = m_ptable->m_vedit.ElementAt(i);
				m_ptable->m_vedit.RemoveElement(piedit);
				m_ptable->m_vedit.InsertElementAt(piedit,0);
				m_ptable->SetDirtyDraw();
				}
			}
		}

	for (int i=0;i<m_ptable->m_vedit.Size();i++) //fix render bumpers
		{
		if (m_ptable->m_vedit.ElementAt(i)->GetItemType() == eItemBumper)
			{
			IEditable * const piedit = m_ptable->m_vedit.ElementAt(i);
			m_ptable->m_vedit.RemoveElement(piedit);
			m_ptable->m_vedit.InsertElementAt(piedit,0);
			m_ptable->SetDirtyDraw();
			}
		}

	for (int i=0;i<m_ptable->m_vedit.Size();i++) //fix render gates
		{
		if (m_ptable->m_vedit.ElementAt(i)->GetItemType() == eItemGate)
			{
			IEditable * const piedit = m_ptable->m_vedit.ElementAt(i);
			m_ptable->m_vedit.RemoveElement(piedit);
			m_ptable->m_vedit.InsertElementAt(piedit,0);
			m_ptable->SetDirtyDraw();
			}
		}

	for (int i=0;i<m_ptable->m_vedit.Size();i++) //fix render spinners
		{
		if (m_ptable->m_vedit.ElementAt(i)->GetItemType() == eItemSpinner)
			{
			IEditable * const piedit = m_ptable->m_vedit.ElementAt(i);
			m_ptable->m_vedit.RemoveElement(piedit);
			m_ptable->m_vedit.InsertElementAt(piedit,0);
			m_ptable->SetDirtyDraw();
			}
		}

	for (int i=0;i<m_ptable->m_vedit.Size();i++) //fix render flippers
		{
		if (m_ptable->m_vedit.ElementAt(i)->GetItemType() == eItemFlipper)
			{
			IEditable * const piedit = m_ptable->m_vedit.ElementAt(i);
			m_ptable->m_vedit.RemoveElement(piedit);
			m_ptable->m_vedit.InsertElementAt(piedit,0);
			m_ptable->SetDirtyDraw();
			}
		}

	for (int i=0;i<m_ptable->m_vedit.Size();i++) //fix render lights
		{
		if (m_ptable->m_vedit.ElementAt(i)->GetItemType() == eItemLight)
			{
			IEditable * const piedit = m_ptable->m_vedit.ElementAt(i);
			m_ptable->m_vedit.RemoveElement(piedit);
			m_ptable->m_vedit.InsertElementAt(piedit,0);
			m_ptable->SetDirtyDraw();
			}
		}
}

void Player::InitStatic(HWND hwndProgress)
	{
	if (m_pin3d.m_fReadingFromCache)
		{
		m_pin3d.ReadSurfaceFromCacheFile(m_pin3d.m_pddsStatic);
		m_pin3d.ReadSurfaceFromCacheFile(m_pin3d.m_pddsStaticZ);
		return;
		}

	// Start the frame.
	HRESULT hr = m_pin3d.m_pd3dDevice->BeginScene();

	// Direct all renders to the "static" buffer.
	m_pin3d.SetRenderTarget(m_pin3d.m_pddsStatic, m_pin3d.m_pddsStaticZ);

	// Draw stuff
	for (int i=0;i<m_ptable->m_vedit.Size();i++)
		{
		if (m_ptable->m_vedit.ElementAt(i)->GetItemType() != eItemDecal && 
			m_ptable->m_vedit.ElementAt(i)->GetItemType() != eItemKicker)
			{
			Hitable * const ph = m_ptable->m_vedit.ElementAt(i)->GetIHitable();
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
	for (int i=0;i<m_ptable->m_vedit.Size();i++)
		{
		if (m_ptable->m_vedit.ElementAt(i)->GetItemType() == eItemDecal)
			{
			Hitable * const ph = m_ptable->m_vedit.ElementAt(i)->GetIHitable();
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
	for (int i=0;i<m_ptable->m_vedit.Size();i++)
		{
		if (m_ptable->m_vedit.ElementAt(i)->GetItemType() == eItemKicker)
			{
			Hitable * const ph = m_ptable->m_vedit.ElementAt(i)->GetIHitable();
			if (ph)
				{
				ph->RenderStatic(m_pin3d.m_pd3dDevice);
				//SendMessage(hwndProgress, PBM_SETPOS, 75 + ((5*i)/m_ptable->m_vedit.Size()), 0);
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
	HRESULT hr = m_pin3d.m_pd3dDevice->BeginScene();

	// Direct all renders to the back buffer.
	m_pin3d.SetRenderTarget(m_pin3d.m_pddsBackBuffer, m_pin3d.m_pddsZBuffer);

	// Set up z-buffer to the static one, so movers can clip to it
	m_pin3d.m_pddsZBuffer->Blt(NULL, m_pin3d.m_pddsStaticZ, NULL, DDBLT_WAIT, NULL);
	m_pin3d.m_pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET, 0x00000000, 1.0f, 0L );

	// Draw stuff
	for (int i=0;i<m_ptable->m_vedit.Size();i++)
		{
		Hitable * const ph = m_ptable->m_vedit.ElementAt(i)->GetIHitable();
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
	for (int i=0;i<m_ptable->m_vedit.Size();i++)
	{
		if (m_ptable->m_vedit.ElementAt(i)->GetItemType() == eItemLight)
		{
			Light * const plight = ((Light *)m_ptable->m_vedit.ElementAt(i));
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
			Bumper * const pbumper = ((Bumper *)m_ptable->m_vedit.ElementAt(i));
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
	m_pin3d.m_pddsBackBuffer->Blt(NULL, m_pin3d.m_pddsStatic, NULL, DDBLT_WAIT , NULL);
	m_pin3d.m_pddsZBuffer->Blt(NULL, m_pin3d.m_pddsStaticZ, NULL, DDBLT_WAIT , NULL);
}

Ball *Player::CreateBall(const float x, const float y, const float z, const float vx, const float vy, const float vz)
	{
	return CreateBall(x,y,z,vx,vy,vz,25.0f);
	}

Ball *Player::CreateBall(const float x, const float y, const float z, const float vx, const float vy, const float vz, const float radius)
	{
	Ball * const pball = new Ball();
	pball->radius = radius;
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
	// Flag the region as needing to be updated.
	if (m_fBallShadows)
	{
		if (!(fIntRectIntersect(pball->m_rcScreen, pball->m_rcScreenShadow)))
		{
			InvalidateRect(&pball->m_rcScreenShadow);
		}
	}
	InvalidateRect(&pball->m_rcScreen);
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
		m_pactiveballDebug = (m_vball.Size() > 0) ? m_vball.ElementAt(0) : NULL;
		}
	}

#ifdef ULTRAPIN
void Player::InitDMDHackWindow()
{
	// Define the window.
	WNDCLASSEX wcex;
	ZeroMemory( &wcex, sizeof ( WNDCLASSEX ) );
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
	const int x = 0;
	const int y = 0;

	// Set the width and height.
	const int width = 200;
	const int height = 100;

	// TEXT
	m_dmdhackhwnd = ::CreateWindowEx( (WS_EX_TOPMOST), "VPPlayerDMDHack", "Visual Pinball Player DMD Hack", (WS_POPUP | WS_CLIPCHILDREN), x, y, width, height, NULL, NULL, g_hinst, 0);
}
#endif

void Player::InitWindow()
	{
	WNDCLASSEX wcex;
	ZeroMemory(&wcex, sizeof(WNDCLASSEX));
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

	int x = (screenwidth - m_width) / 2;
	int y = (screenheight - m_height) / 2;

	int windowflags = WS_POPUP | WS_CLIPCHILDREN;
	int windowflagsex = 0;

	const int captionheight = GetSystemMetrics(SM_CYCAPTION);

	if (!m_fFullScreen && (screenheight - m_height >= (captionheight*2))) // We have enough room for a frame
	{
		// Check if we have a front end.
		if ( FindWindow( NULL, "Ultrapin (plfe)" ) != NULL )
		{
			// No window border, title, or control boxes.
			windowflags = WS_POPUP | WS_CLIPCHILDREN;
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

	int ballStretchMode;
	hr = GetRegInt("Player", "BallStretchMode", &ballStretchMode);
	if (hr != S_OK)
	{
		ballStretchMode = 0;
	}

	// Monitors: 4:3, 16:9, 16:10, 21:10
	int ballStretchMonitor;
	hr = GetRegInt("Player", "BallStretchMonitor", &ballStretchMonitor);
	if (hr != S_OK)
	{
		ballStretchMonitor = 1; // assume 16:9
	}

	float scalebackX = ((m_ptable->m_scalex + m_ptable->m_scaley)*0.5f)/m_ptable->m_scalex;
	float scalebackY = ((m_ptable->m_scalex + m_ptable->m_scaley)*0.5f)/m_ptable->m_scaley;
	if (abs(m_ptable->m_rotation) <= 0.001f)
	{
		scalebackX = 1.0f;
		scalebackY = 1.0f;
	}
	float xMonitor = 16.0f;
	float yMonitor = 9.0f;
	switch (ballStretchMonitor)
	{
	case 0: 
		xMonitor = 4.0f / 4.0f;
		yMonitor = 3.0f / 3.0f;
		break;
	case 1: 
		xMonitor = 16.0f / 4.0f;
		yMonitor = 9.0f / 3.0f;
		break;
	case 2: 
		xMonitor = 16.0f / 4.0f;
		yMonitor = 10.0f / 3.0f;
		break;
	case 3: 
		xMonitor = 21.0f / 4.0f;
		yMonitor = 10.0f / 3.0f;
		break;
	case 4: 
		xMonitor = 3.0f / 4.0f;
		yMonitor = 4.0f / 3.0f;
		break;
	case 5: 
		xMonitor = 9.0f / 4.0f;
		yMonitor = 16.0f / 3.0f;
		break;
	case 6: 
		xMonitor = 10.0f / 4.0f;
		yMonitor = 16.0f / 3.0f;
		break;
	case 7: 
		xMonitor = 10.0f / 4.0f;
		yMonitor = 21.0f / 3.0f;
		break;

	}
	float scalebackMonitorX = ((xMonitor + yMonitor)*0.5f)/xMonitor;
	float scalebackMonitorY = ((xMonitor + yMonitor)*0.5f)/yMonitor;

	float temprotation = m_ptable->m_rotation;
	while (temprotation < 0.f)
	{
		temprotation += 360.0f;
	}

	switch(ballStretchMode)
	{
		case 0:	m_BallStretchX = 1.0f;
				m_BallStretchY = 1.0f;
				break;
		case 1: /*
				m_width
				m_height
				m_ptable->m_scalex
				m_ptable->m_scaley
				*/
				m_BallStretchX = scalebackX*sinf(ANGTORAD(fmodf(temprotation + 90.0f,180.0f))) + scalebackY*sinf(ANGTORAD(fmodf(temprotation,180.0f)));
				m_BallStretchY = scalebackY*sinf(ANGTORAD(fmodf(temprotation + 90.0f,180.0f))) + scalebackX*sinf(ANGTORAD(fmodf(temprotation,180.0f)));

				break;
		case 2: m_BallStretchX = scalebackX*sinf(ANGTORAD(fmodf(temprotation + 90.0f,180.0f))) + scalebackY*sinf(ANGTORAD(fmodf(temprotation,180.0f)));
				m_BallStretchY = scalebackY*sinf(ANGTORAD(fmodf(temprotation + 90.0f,180.0f))) + scalebackX*sinf(ANGTORAD(fmodf(temprotation,180.0f)));
				if (m_fFullScreen)
				{
					m_BallStretchX = m_BallStretchX * (scalebackMonitorX*sinf(ANGTORAD(fmodf(temprotation + 90.0f,180.0f))) + scalebackMonitorY*sinf(ANGTORAD(fmodf(temprotation,180.0f))));
					m_BallStretchY = m_BallStretchY * (scalebackMonitorY*sinf(ANGTORAD(fmodf(temprotation + 90.0f,180.0f))) + scalebackMonitorX*sinf(ANGTORAD(fmodf(temprotation,180.0f))));
				}
				break;
	}

	// TEXT
	m_hwnd = ::CreateWindowEx(windowflagsex, "VPPlayer", "Visual Pinball Player", windowflags, x, y, m_width, m_height, NULL, NULL, g_hinst, 0);

    mixer_init( m_hwnd );
    hid_init();

	SetCursorPos( 400,999999); //rlc ... move to hide in lower left corner, one pixel shows

#ifdef GDIDRAW
	HDC hdc = GetDC(NULL);

	m_hbmOffScreen = CreateCompatibleBitmap(hdc, m_width, m_height);

	ReleaseDC(NULL, hdc);
#endif
	}

#ifdef ULTRACADE

extern int e_JoyCnt;
static int curAccel_x[PININ_JOYMXCNT]= {0,0,0,0};
static int curAccel_y[PININ_JOYMXCNT]= {0,0,0,0};

//static float curMechPlungerPos;

int curPlunger = JOYRANGEMN-1;	// assume


void Player::UltraNudgeX(int x, int j )
{
	curAccel_x[j] = x;
}


void Player::UltraNudgeY(int y, int j )
{
	curAccel_y[j] = y;
}


F32 GetX()
{
	return ((F32)curAccel_x[0]) * (F32)(2.0 / (JOYRANGEMX-JOYRANGEMN));
}


F32 GetY()
{
    return ((F32)curAccel_y[0]) * (F32)(2.0 / (JOYRANGEMX-JOYRANGEMN));
}


int Player::UltraNudgeGetTilt()
{
	static U32 last_tilt_time;
	static U32 last_jolt_time;

	if( !m_fAccelerometer || m_NudgeManual >= 0 ||	         //disabled or in joystick test mode
	     m_tilt_amount == 0 || m_jolt_amount == 0) return 0; //disabled

	const U32 ms = msec();

	U32 tilt_2 = 0;
	for(int j = 0; j < e_JoyCnt; ++j)	//find largest value
		{
		tilt_2 = max(tilt_2,(U32)(curAccel_x[j] * curAccel_x[j] + curAccel_y[j] * curAccel_y[j])); //always postive numbers
		}

	if( ( ms - last_jolt_time > m_jolt_trigger_time ) &&
		( ms - last_tilt_time > m_tilt_trigger_time ) &&
	   	tilt_2 > ( m_tilt_amount * m_tilt_amount ) )
	{
		last_tilt_time = ms;

		return 1;
	}

	if( ms - last_jolt_time > m_jolt_trigger_time && tilt_2 > ( m_jolt_amount * m_jolt_amount ) )
	{
		last_jolt_time = ms;
	}

	return 0;
}


void Player::UltraNudge()	// called on every intergral physics frame
{	
	static F32 cna=1,sna=0,na=0;	//rlc initialize for angle 0
	
	if (m_NudgeManual >= 0)			// Only one joystick controls in manual mode
	{		
		m_NudgeX = m_AccelMAmp * ((float)curAccel_x[m_NudgeManual])*(float)(1.0/JOYRANGE); // * Manual Gain
		m_NudgeY = m_AccelMAmp * ((float)curAccel_y[m_NudgeManual])*(float)(1.0/JOYRANGE);

		if (g_pplayer->m_ptable->m_tblMirrorEnabled)
			m_NudgeX = -m_NudgeX;

		return;
	}	

	m_NudgeX = 0;	// accumlate over joysticks, these acceleration values are used in update ball velocity calculations
	m_NudgeY = 0;	// and are required to be acceleration values (not velocity or displacement)

	if(!m_fAccelerometer) return;	// uShock is disabled 
	
	//rotate to match hardware mounting orentation, including left or right coordinates
	// Cache the sin and cos results whenever the angle changes
	if( na != m_AccelAngle )
	{
		na = m_AccelAngle;
		cna = cosf(na);
		sna = sinf(na);
	}

	for(int j = 0; j < e_JoyCnt; ++j)
	{		
		float dx = ((float)curAccel_x[j])*(float)(1.0/JOYRANGE);		// norm range -1 .. 1	
		const float dy = ((float)curAccel_y[j])*(float)(1.0/JOYRANGE);	
		if ( g_pplayer->m_ptable->m_tblMirrorEnabled )
			dx = -dx;
		m_NudgeX += m_AccelAmpX*(dx*cna + dy*sna) * (1.0f - nudge_get_sensitivity());  //calc Green's transform component for X
		const float nugY = m_AccelAmpY*(dy*cna - dx*sna) * (1.0f - nudge_get_sensitivity()); // calc Green transform component for Y...
		m_NudgeY = m_AccelNormalMount ? (m_NudgeY + nugY): (m_NudgeY - nugY);	// add as left or right hand coordinate system
	}
}

void Player::UltraPlunger()	// called on every intergral physics frame
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
		//int init = IIR_Order;
		curMechPlungerPos = 0;
		return;	// not until a real value is entered
		}

	if (!c_plungerFilter)
		{ 
		curMechPlungerPos = (float)curPlunger;
		return;
		}
	
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
float PlungerAnimObject::mechPlunger() const
{
	const float range = (float)JOYRANGEMX * (1.0f - m_parkPosition) - (float)JOYRANGEMN *m_parkPosition; // final range limit
	float tmp = (curMechPlungerPos < 0) ? curMechPlungerPos*m_parkPosition : curMechPlungerPos*(1.0f - m_parkPosition);
	tmp = tmp/range + m_parkPosition;		//scale and offset
	return tmp;
}


void Player::mechPlungerIn(int z)
	{
	curPlunger = -z; //axis reversal

	if (++movedPlunger == 0x7ffffff) movedPlunger = 3; //restart at 3
	}

#endif

//++++++++++++++++++++++++++++++++++++++++

#define STATICCNTS 10

void Player::PhysicsSimulateCycle(float dtime, const U64 startTime) // move physics forward to this time
	{
	float hittime;
	float staticTime = STATICTIME;
	const int limitTime = m_ptable->m_PhysicsLoopTime;
	int halfLimitTime = limitTime/2;

	const int vballsize = m_vball.Size();

	int StaticCnts = STATICCNTS;	// maximum number of static counts

	while (dtime > 0)
		{

		if (limitTime)//time in microseconds
			{
			const int time_elasped = (int)(usec()- startTime);

			if (time_elasped > limitTime) //time in microseconds
				return; // hung in the physics loop

			if (time_elasped > halfLimitTime)		//time in microseconds
				{
				staticTime += staticTime*0.5f;		//increase minimum time step by 50%
				halfLimitTime += halfLimitTime/2;	// set next half limit time step (logarithmic)			
				}
			}

		// first find hits, if any +++++++++++++++++++++ 

		hittime = dtime;	//begin time search from now ...  until delta ends

		for (int i = 0; i < vballsize; i++)
			{
			Ball * const pball = m_vball.ElementAt(i);

			if (!pball->fFrozen && pball->m_fDynamic > 0)// && !pball->fTempFrozen )	// don't play with frozen balls
				{
				pball->m_hittime = hittime;				// search upto current hittime
				pball->m_pho = NULL;

				m_hitoctree.HitTestBall(pball);			// find the hit objects and hit times

				const float htz = pball->m_hittime;		// this ball's hit time

				if(htz < 0) pball->m_pho = NULL;		// no negative time allowed

				if (pball->m_pho)						// hit object
					{
#ifdef _DEBUG
					++c_hitcnts;						// stats for display

					if (pball->m_HitRigid && pball->m_HitDist < -0.0875f) //rigid and embedded
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
	
		for (int i=0;i<m_vmover.Size();i++)
			{
			m_vmover.ElementAt(i)->UpdateDisplacements(hittime); //step 2:  move the objects about according to velocities
			} 

		//  ok find balls that need to be collided and script'ed (generally there will be one, but more are possible)

		for (int i=0; i < m_vball.Size(); i++)			// use m_vball.Size(), in case script deletes a ball
			{
			Ball * const pball = m_vball.ElementAt(i);			// local pointer

			if (pball->m_fDynamic > 0 && pball->m_pho && pball->m_hittime <= hittime) // find balls with hit objects and minimum time			
				{			
	
				// now collision, contact and script reactions on active ball (object)+++++++++
				HitObject * const pho = pball->m_pho;// object that ball hit in trials
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
					if (pball->m_HitRigid && pball->m_HitDist < (float)PHYS_TOUCH) //rigid and close distance
						{//contacts
						c_contactcnt++;
						const float mag = pball->vx*pball->vx + pball->vy*pball->vy; // values below are taken from simulation
						if (pball->drsq < 8.0e-5f && mag < 1.0e-3f && fabsf(pball->vz) < 0.2f)
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

void Player::Render()
	{
	// On Win95 when there are no balls, frame updates happen so fast the
	// blitter gets stuck
	const int cball = m_vball.Size();
	if ((cball == 0) && m_fWasteTime2)
		{
		Sleep(1);
		}

	if (m_sleeptime > 0)
		{
		Sleep(m_sleeptime - 1);
		}

	if (m_fCheckBlt) // Don't calculate the next frame if the last one isn't done blitting yet
		{
		const HRESULT hrdone = m_pin3d.m_pddsFrontBuffer->GetBltStatus(DDGBS_ISBLTDONE);

		if (hrdone != DD_OK)
			{
			if (m_fWasteTime)
				{
				Sleep(1);
				}
			return;
			}
		}

#ifdef ANTI_TEAR
	static U64 sync;

	if( sync ) // Spin the CPU to prevent from running graphics faster than necessary
	{
		while( usec() - sync < 16666 ) { ; } // ~60 fps
	}
	sync = usec();
#endif

	//

	for (int iball=0;iball<cball;iball++)
		{
		Ball * const pball = m_vball.ElementAt(iball);

		if (pball->m_fErase) // Need to clear the ball off the playfield
			{
			EraseBall(pball);
			}
		}

	// Erase the mixer volume.
	mixer_erase();
    plumb_erase();

	c_collisioncnt = 0; 
	c_hitcnts = 0;
	c_contactcnt = 0;
	c_staticcnt = 0;

	//

	const U64 m_RealTimeClock = usec();

	if (m_fNoTimeCorrect) // After debugging script
		{
		// Shift whole game foward in time
		m_liStartTime += m_RealTimeClock - m_curPhysicsFrameTime;
		m_nextPhysicsFrameTime += m_RealTimeClock - m_curPhysicsFrameTime;
		m_curPhysicsFrameTime = m_RealTimeClock; // 0 time frame
		m_fNoTimeCorrect = fFalse;
		}

#ifdef STEPPING
#ifndef EVENTIME
	if (m_fDebugWindowActive || m_fUserDebugPaused)
		{
		// Shift whole game foward in time
		m_liStartTime += m_RealTimeClock - m_curPhysicsFrameTime;
		m_nextPhysicsFrameTime += m_RealTimeClock - m_curPhysicsFrameTime;
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
#endif
#endif

#ifdef EVENTIME
	if (!m_fPause || m_fStep)
		{
		m_RealTimeClock = m_curPhysicsFrameTime - 3547811060 + 3547825450;//(m_PhysicsStepTime*3/4);
		m_fStep = fFalse;
		}
	else
		{
		m_RealTimeClock = m_curPhysicsFrameTime;
		}
#endif

	// Get time in milliseconds for timers
	m_timeCur = (int)((m_RealTimeClock - m_liStartTime)/1000);

#ifdef FPS
	//if (m_fShowFPS)
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

#ifdef LOG
	const double timepassed = (double)(m_RealTimeClock - m_curPhysicsFrameTime) * (1.0/1000000.0);
	float frametime;

#ifdef PLAYBACK
	if (!m_fPlayback)
		{
		frametime = (float)(timepassed * 100.0);
		}
	else
		{
		frametime = ParseLog((LARGE_INTEGER*)&m_RealTimeClock, (LARGE_INTEGER*)&m_nextPhysicsFrameTime);
		}
#else // PLAYBACK

#define TIMECORRECT 1
#ifdef TIMECORRECT
	frametime = (float)(timepassed * 100.0);
	//frametime = 1.456927f;
#else // TIMECORRECT
	frametime = 0.45f;
#endif // TIMECORRECT

#endif //PLACKBACK

		fprintf(m_flog, "Frame Time %.20f %u %u %u %u\n", frametime, m_RealTimeClock>>32, m_RealTimeClock, m_nextPhysicsFrameTime>>32, m_nextPhysicsFrameTime);
		fprintf(m_flog, "End Frame\n");
#endif

#ifdef FPS
	U32 phys_iterations = 0;
	U64 phys_period = m_RealTimeClock;	
#endif

	///+++++++++++++++++++++++++++++++++++++++++++++++++++++
	while (m_curPhysicsFrameTime < m_RealTimeClock)		//loop here until next frame time
		{
#ifdef FPS
		phys_iterations++;
#endif
		// Get the time until the next physics tick is done, and get the time
		// Until the next frame is done (newtime)
		// If the frame is the next thing to happen, update physics to that
		// point next update acceleration, and continue loop

		const float physics_dtime = (float)((double)(m_nextPhysicsFrameTime - m_curPhysicsFrameTime)*(1.0/10000.0));
		const float physics_to_graphic_dtime  = (float)((double)(m_RealTimeClock - m_curPhysicsFrameTime)*(1.0/10000.0));

		const U64 cur_time = usec();

		if (physics_to_graphic_dtime < physics_dtime )				 // is graphic frame time next???
			{		
			PhysicsSimulateCycle(physics_to_graphic_dtime, cur_time);// advance physics to this time
			m_curPhysicsFrameTime = m_RealTimeClock;				 // now current to the wall clock
			break;	//this is the common exit from the loop			 // exit skipping accelerate
			}		// some rare cases will exit from while()

		if (cur_time - m_RealTimeClock > 200000)					 // hung in the physics loop over 200 milliseconds
			{														 // can not keep up to real time
			m_curPhysicsFrameTime = m_RealTimeClock;				 // skip physics forward ... slip-cycles
			m_nextPhysicsFrameTime = m_RealTimeClock + m_PhysicsStepTime;
			break;	//this is the common exit from the loop			 // go draw frame
			}
		
		//primary physics loop
		PhysicsSimulateCycle(physics_dtime, cur_time); 				 // main simulator call physics_dtime

 		m_curPhysicsFrameTime = m_nextPhysicsFrameTime;				 // new cycle, on physics frame boundary
		m_nextPhysicsFrameTime += m_PhysicsStepTime;				 // advance physics position

		// now get and/or calculate integral cycle physics events, digital filters, external acceleration inputs, etc.

		const U32 sim_msec = m_curPhysicsFrameTime/1000;
		m_pininput.ProcessKeys(m_ptable, sim_msec);
        mixer_update();

        hid_update(sim_msec);
        plumb_update(sim_msec);

#ifdef ACCURATETIMERS
	m_pactiveball = NULL;  // No ball is the active ball for timers/key events

	const int p_timeCur = (int)((m_curPhysicsFrameTime - m_liStartTime)/1000); //rlc 10 milli-seconds

	for (int i=0;i<m_vht.Size();i++)
		{
		HitTimer * const pht = m_vht.ElementAt(i);
		if (pht->m_nextfire <= p_timeCur)
			{
			pht->m_pfe->FireGroupEvent(DISPID_TimerEvents_Timer);
			pht->m_nextfire += pht->m_interval;
			}
		}
#endif

		//slintf( "%u %u\n", m_RealTimeClock/1000, sim_msec );
		//slintf( "%f %f %d %d\n", physics_dtime, physics_to_graphic_dtime, sim_msec, msec() );	
		
#ifdef ULTRACADE
		UltraNudge();		//rlc physics_dtime is the balance of time to move from the graphic frame position to the next
		UltraPlunger();		// integral physics frame.  So the previous graphics frame was (1.0 - physics_dtime) before 
							// this integral physics frame. Accelerations and inputs are always physics frame aligned
#endif
		
	if (m_nudgetime)
	{
		m_nudgetime--;	                                                                                                                                                    

		if (m_nudgetime == 5)
		{
			m_NudgeX = -m_NudgeBackX * 2.0f;
			m_NudgeY = m_NudgeBackY * 2.0f;
		}
		else if (m_nudgetime == 0)
		{
			m_NudgeX = m_NudgeBackX;
			m_NudgeY = -m_NudgeBackY;
		}
	}

	for (int i=0;i<m_vmover.Size();i++)
	{
		m_vmover.ElementAt(i)->UpdateVelocities(1.0f);	// always on integral physics frame boundary
	}			

#ifndef ULTRACADE
		m_NudgeX = 0;
		m_NudgeY = 0;
#endif			
	} // end while (m_curPhysicsFrameTime < m_RealTimeClock)

#ifdef FPS
	phys_period = usec() - phys_period;
#endif

	m_LastKnownGoodCounter++;

	if(Pressed(PININ_ENABLE3D)) {
		m_fStereo3Denabled = !m_fStereo3Denabled;
		SetRegValue("Player", "Stereo3DEnabled", REG_DWORD, &m_fStereo3Denabled, 4);
		m_fCleanBlt = fFalse;
	}

#ifdef GDIDRAW
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(m_hwnd, &ps);

	HBITMAP hbmOld;

	HDC hdcT = CreateCompatibleDC(hdc);

	hbmOld = (HBITMAP)SelectObject(hdcT, m_hbmOffScreen);

	SelectObject(hdcT, GetStockObject(WHITE_BRUSH));

	PatBlt(hdcT, 0, 0, m_width, m_height, PATCOPY);

	for (int i=0;i<m_vho.Size();i++)
		{
		m_vho.ElementAt(i)->Draw(hdcT);
		}

	for (int i=0;i<m_vball.Size();i++)
		{
		SelectObject(hdcT, GetStockObject(NULL_BRUSH));
		SelectObject(hdcT, GetStockObject(BLACK_PEN));
		Ball *pball = m_vball.ElementAt(i);
		DrawCircle(hdcT, pball->x, pball->y, pball->radius);
		}

	BitBlt(hdc, 0, 0, m_width, m_height, hdcT, 0, 0, SRCCOPY);

	SelectObject(hdcT, hbmOld);

	DeleteDC(hdcT);

	EndPaint(m_hwnd, &ps);

#else // GDIDRAW

	// Check all elements that could possibly need updating.
	// cupid for primitives: OK, i need my primitives in here... And they need to understand m_fInvalid, Check3D and m_rcBounds
	for (int i=0;i<m_vscreenupdate.Size();i++)
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

	//rlc BUG -- moved this code before copy of static buffers being copied to back and z buffers
	//rlc  JEP placed code for copy of static buffers too soon 
	// Notice - the light can only update once per frame, so if the light
	// is blinking faster than the frame rate, the user will still see
	// the light blinking, it will just be slower than intended.
	for (int i=0;i<m_vblink.Size();i++)
	{
		IBlink * const pblink = m_vblink.ElementAt(i);
		if (pblink->m_timenextblink <= m_timeCur)
		{
			const char cold = pblink->m_rgblinkpattern[pblink->m_iblinkframe];
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

	//
	// dynamic ... copies Static buffers to back buffer and z buffer
	//

	// Initialize all invalid regions by resetting the region (basically clear) 
	// it with the contents of the static buffer.

	// cupid for primitives:
	// If i call InvalidateRect here, with a big enough rect, all is displayed correctly... 
	// If I call it from the render process in Primitives, that does not work, since it is 
	// called via drawacrylics, about 200 Lines down. So i have to get this done before.
	// that seems to have something to do with m_vscreenupdate ... So lets look where this gets filled.
	/*
	RECT * rect;
	rect = new RECT();
	rect->left = 100;
	rect->right = 1700;
	rect->top = 100;
	rect->bottom = 700;			
	InvalidateRect(rect);
	*/

	for (int i=0;i<m_vupdaterect.Size();i++)
		{
		UpdateRect * const pur = m_vupdaterect.ElementAt(i);
		if (pur->m_fSeeThrough)													
			{
			
			RECT * const prc = &pur->m_rcupdate;
			
			// Redraw the region from the static buffers to the back and z buffers.
			m_pin3d.m_pddsBackBuffer->Blt(prc, m_pin3d.m_pddsStatic, prc, 0, NULL);
			m_pin3d.m_pddsZBuffer->Blt(prc, m_pin3d.m_pddsStaticZ, prc, 0, NULL);
			}
		}	

	// Start rendering the next frame.
	HRESULT hr = m_pin3d.m_pd3dDevice->BeginScene();

	D3DMATRIX RestoreWorldMatrix;
	RenderStateType	RestoreRenderState;
	TextureStateType RestoreTextureState;
	HRESULT ReturnCode;
	// Check if we are blitting with D3D.
	if (g_pvp->m_pdd.m_fUseD3DBlit)
		{
		// Save the current transformation state.
		ReturnCode = g_pplayer->m_pin3d.m_pd3dDevice->GetTransform ( D3DTRANSFORMSTATE_WORLD, &RestoreWorldMatrix );

		// Save the current render state.
		Display_GetRenderState(g_pplayer->m_pin3d.m_pd3dDevice, &RestoreRenderState);

		// Save the current texture state.
		Display_GetTextureState (g_pplayer->m_pin3d.m_pd3dDevice, &RestoreTextureState);
		}

// why is this done twice? is there some stubborn hardware refusing to take commands?? (SnailGary)
/*
	// Check if we are blitting with D3D.
	if (g_pvp->m_pdd.m_fUseD3DBlit)
		{
		// Restore the render states.
		Display_SetRenderState(g_pplayer->m_pin3d.m_pd3dDevice, &RestoreRenderState);

		// Restore the texture state.
		Display_SetTextureState(g_pplayer->m_pin3d.m_pd3dDevice, &RestoreTextureState);

		// Restore the transformation state.
		ReturnCode = g_pplayer->m_pin3d.m_pd3dDevice->SetTransform ( D3DTRANSFORMSTATE_WORLD, &RestoreWorldMatrix ); 
		}

	// Check if we are blitting with D3D.								
	if (g_pvp->m_pdd.m_fUseD3DBlit)				
		{
		// Save the current transformation state.
		ReturnCode = g_pplayer->m_pin3d.m_pd3dDevice->GetTransform ( D3DTRANSFORMSTATE_WORLD, &RestoreWorldMatrix ); 

		// Save the current render state.
		Display_GetRenderState(g_pplayer->m_pin3d.m_pd3dDevice, &RestoreRenderState);

		// Save the current texture state.
		Display_GetTextureState (g_pplayer->m_pin3d.m_pd3dDevice, &RestoreTextureState);
		}
*/

	// Check if we are blitting with D3D.
	// And setup everything that will be needed to draw sprites in next loop
	if (g_pvp->m_pdd.m_fUseD3DBlit)					
	{
		const D3DMATRIX WorldMatrix(1.0f,0.0f,0.0f,0.0f,0.0f,1.0f,0.0f,0.0f,0.0f,0.0f,1.0f,0.0f,0.0f,0.0f,0.0f,1.0f);

		// Apply the transformation.
		/*const HRESULT ReturnCode =*/ g_pplayer->m_pin3d.m_pd3dDevice->SetTransform ( D3DTRANSFORMSTATE_WORLD, (LPD3DMATRIX)&WorldMatrix );

		// Set the texture state.
		Display_SetTextureState ( g_pplayer->m_pin3d.m_pd3dDevice, &(TextureStates[DISPLAY_TEXTURESTATE_NOFILTER]) );
	
		// Set the render state.
		Display_SetRenderState ( g_pplayer->m_pin3d.m_pd3dDevice, &(RenderStates[DISPLAY_RENDERSTATE_TRANSPARENT]) );
	}

	// Process all regions that need updating.  
	// The region will be drawn with the current frame.
	for (int i=0;i<m_vupdaterect.Size();i++)
	{
		const UpdateRect * const pur = m_vupdaterect.ElementAt(i);

		// Process all objects associated with this region.
		for (int l=0;l<pur->m_vobject.Size();l++)
		{
			// Get the object's frame to draw.
			const ObjFrame * const pobjframe = pur->m_vobject.ElementAt(l)->Draw3D(&pur->m_rcupdate);

			// Make sure we have a frame.
			if (pobjframe != NULL)
			{
				const LPDIRECTDRAWSURFACE7 pdds = g_pplayer->m_pin3d.m_pddsBackBuffer;
				const RECT * const prc = &pur->m_rcupdate;

				// NOTE: prc is the rectangle of the region needing to be updated.
				// NOTE: pobjframe->rc is the rectangle of the entire object that intersects the region needing to updated.
				// I think they are trying to define a rectangle that intersects... but why subtract pobjframe->rc?   -JEP

				const int bltleft = max(pobjframe->rc.left, prc->left);
				const int blttop = max(pobjframe->rc.top, prc->top);

				RECT rcUpdate;
				rcUpdate.left = bltleft - pobjframe->rc.left;
				rcUpdate.top = blttop - pobjframe->rc.top;
				rcUpdate.right = min(pobjframe->rc.right, prc->right) - pobjframe->rc.left;
				rcUpdate.bottom = min(pobjframe->rc.bottom, prc->bottom) - pobjframe->rc.top;

				// Make sure our rectangle dimensions aren't wacky.
				if ((rcUpdate.right > rcUpdate.left) && (rcUpdate.bottom > rcUpdate.top))
				{
					// Check if we are blitting with D3D.
					if (g_pvp->m_pdd.m_fUseD3DBlit)
					{
						// Blit to the backbuffer with D3D.
						// NOTE: Rather than drawing just a portion of the sprite... draw the whole thing.
						Display_DrawSprite_NoMatrix_NoStates(g_pplayer->m_pin3d.m_pd3dDevice, 
										(float) (pobjframe->rc.left), (float) (pobjframe->rc.top), 
										(float) (pobjframe->rc.right - pobjframe->rc.left), (float) (pobjframe->rc.bottom - pobjframe->rc.top), 
										0xFFFFFFFF, 
										0.0f, 
										pobjframe->pTexture, pobjframe->u, pobjframe->v);
					}
					else
					{
						// Make sure we have a source color surface.
						if (pobjframe->pdds != NULL)
						{
							// Blit to the backbuffer with DDraw.   
							/*const HRESULT hr =*/ pdds->BltFast(bltleft, blttop, pobjframe->pdds, &rcUpdate, DDBLTFAST_SRCCOLORKEY);
						}
					}
					
					// Make sure we have a source z surface.
					if (pobjframe->pddsZBuffer != NULL)
					{
						// Blit to the z buffer.	
						/*const HRESULT hr =*/ g_pplayer->m_pin3d.m_pddsZBuffer->BltFast(bltleft, blttop, pobjframe->pddsZBuffer, &rcUpdate, DDBLTFAST_NOCOLORKEY);
					}
				}
			}
		}
	}

	// Check if we are blitting with D3D.
	if (g_pvp->m_pdd.m_fUseD3DBlit)
	{
		// Restore the render states.
		Display_SetRenderState(g_pplayer->m_pin3d.m_pd3dDevice, &RestoreRenderState);

		// Restore the texture state.
		Display_SetTextureState(g_pplayer->m_pin3d.m_pd3dDevice, &RestoreTextureState);

		// Restore the transformation state.
		ReturnCode = g_pplayer->m_pin3d.m_pd3dDevice->SetTransform ( D3DTRANSFORMSTATE_WORLD, &RestoreWorldMatrix ); 
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

		g_pplayer->m_ToggleDebugBalls = fFalse;
	}

	if (m_fBallAntialias)
	{
		DrawBallShadows();
		DrawBalls();
	}
	else
	{
		// When not antialiasing, we can get a perf win by
		// drawing the ball first.  That way, the part of the
		// shadow that gets obscured doesn't need to do
		// alpha-blending
		DrawBalls();
		DrawBallShadows();
	}

	// Draw the acrylics (now alphas).
	//if (g_pvp->m_pdd.m_fHardwareAccel)
		DrawAcrylics();

	// Check if we should turn animate the plunger light.
	const U32 cur_time_msec = msec();
	hid_set_output ( HID_OUTPUT_PLUNGER, ((cur_time_msec - LastPlungerHit) < 512) && ((cur_time_msec & 512) > 0) );

	// Draw the mixer volume.
	mixer_draw();
    plumb_draw();

	// Finish rendering the next frame.
	hr = m_pin3d.m_pd3dDevice->EndScene();

	// Check if we are mirrored.
	if ( g_pplayer->m_ptable->m_tblMirrorEnabled )
	{
		// Mirroring only works if we mirror the entire backbuffer.
		// Flag to draw the entire backbuffer.
		m_fCleanBlt = fFalse;
	}


#ifdef VP3D
if(!m_fStereo3D || !m_fStereo3Denabled || (m_pin3d.m_maxSeparation <= 0.0f) || (m_pin3d.m_maxSeparation >= 1.0f) || (m_pin3d.m_ZPD <= 0.0f) || (m_pin3d.m_ZPD >= 1.0f) || !m_pin3d.m_pdds3Dbuffercopy || !m_pin3d.m_pdds3DBackBuffer)
{
#endif
	if ( m_nudgetime &&					// Table is being nudged.
		 g_pplayer->m_ptable->m_Shake )	// The "EarthShaker" effect is active.
	{
		// Draw with an offset to shake the display.
		m_pin3d.Flip((int)m_NudgeBackX, (int)m_NudgeBackY, (m_fps > m_refreshrate*ADAPT_VSYNC_FACTOR));
		m_fCleanBlt = fFalse;
	}
	else
	{
		unsigned int overall_area = 0;

		if (m_fCleanBlt) // detect overall area to blit
		{
			for (int i=0;i<m_vupdaterect.Size();++i)
			{
				const RECT& prc = m_vupdaterect.ElementAt(i)->m_rcupdate;
				overall_area += (prc.right-prc.left)*(prc.bottom-prc.top);
			}
		}

		if (m_fCleanBlt && (overall_area < (unsigned int)(g_pplayer->m_pin3d.m_dwRenderWidth*g_pplayer->m_pin3d.m_dwRenderHeight))) //!! other heuristic? // test if its worth to blit every element separately
		{
			// Smart Blit - only update the invalidated areas
			for (int i=0;i<m_vupdaterect.Size();i++)
			{
				UpdateRect * const pur = m_vupdaterect.ElementAt(i);
				RECT * const prc = &pur->m_rcupdate;

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
			m_pin3d.Flip(0, 0, (m_fps > m_refreshrate*ADAPT_VSYNC_FACTOR));

			// Flag that we only need to update regions from now on...
			m_fCleanBlt = fTrue;
		}
	}
#ifdef VP3D
}
else
{
	//!! add key to en/disable 3D at runtime
	//!! num_threads(max_threads-1 or -2) ? on my AMD omp is not really faster for the update path, a bit faster for full path
	//!! overall half resolution necessary only (Y3D profits from full res though (implicit filtering))

	static const unsigned int zmask = 0xFFFFFFu; //!! can this really be hardcoded? only has to be used for 32bit (because of 8bit stencil) so far
	static const __m128i zmask128 = _mm_set1_epi32(zmask);

	DDSURFACEDESC2 ddsd,ddsdz;
	ZeroMemory( &ddsd, sizeof(ddsd) );
	ZeroMemory( &ddsdz, sizeof(ddsdz) );
	ddsd.dwSize = sizeof(ddsd);
	ddsdz.dwSize = sizeof(ddsdz);

	hr = m_pin3d.m_pddsBackBuffer->Lock(NULL, &ddsd, DDLOCK_WAIT | DDLOCK_NOSYSLOCK | DDLOCK_SURFACEMEMORYPTR | DDLOCK_READONLY, NULL);
    if(!FAILED(hr) && (ddsd.lpSurface != NULL)) {
	hr = m_pin3d.m_pddsZBuffer->Lock(NULL, &ddsdz,   DDLOCK_WAIT | DDLOCK_NOSYSLOCK | DDLOCK_SURFACEMEMORYPTR | DDLOCK_READONLY, NULL); 
    if(!FAILED(hr) && (ddsdz.lpSurface != NULL)) {

	const unsigned int width  = min((unsigned int)GetSystemMetrics(SM_CXSCREEN), min(ddsd.dwWidth,ddsdz.dwWidth));   // just to make sure we don't screw with some weird configuration and also avoid unnecessary (offscreen) work
	const unsigned int height = min((unsigned int)GetSystemMetrics(SM_CYSCREEN), min(ddsd.dwHeight,ddsdz.dwHeight)); // just to make sure we don't screw with some weird configuration and also avoid unnecessary (offscreen) work
	const unsigned int shift = (ddsd.ddpfPixelFormat.dwRGBBitCount == 32) ? 2 : 1;

#ifdef ONLY3DUPD
			unsigned int overall_area = 0;

			if (m_fCleanBlt) // detect overall area to blit
			{
				for (int i=0;i<m_vupdaterect.Size();++i)
				{
					const RECT& prc = m_vupdaterect.ElementAt(i)->m_rcupdate;

					const int left   = max(prc.left + m_pin3d.m_rcUpdate.left,0);
					const int right  = min(prc.right + m_pin3d.m_rcUpdate.left,(int)width-1);
					const int top    = max(prc.top + m_pin3d.m_rcUpdate.top,0);
					const int bottom = min(prc.bottom + m_pin3d.m_rcUpdate.top,(int)height-1);

					overall_area += (right-left)*(bottom-top);
				}
			}

			if (m_fCleanBlt && (overall_area < width*height)) //!! other heuristic? // test if its worth to blit every element separately
			{
				// Smart Blit - only update the invalidated areas
#pragma omp parallel for schedule(dynamic)
				for (int i=0;i<m_vupdaterect.Size();++i)
				{
					const RECT& prc = m_vupdaterect.ElementAt(i)->m_rcupdate;

					const int left4     =  max(prc.left  + m_pin3d.m_rcUpdate.left,0           )<<shift;
					const int copywidth = (min(prc.right + m_pin3d.m_rcUpdate.left,(int)width-1)<<shift) - left4;
					if(copywidth <= 0)
						continue;

					const int top    = left4 + max(prc.top    + m_pin3d.m_rcUpdate.top,0            )*ddsd.lPitch;
					const int bottom = left4 + min(prc.bottom + m_pin3d.m_rcUpdate.top,(int)height-1)*ddsd.lPitch;

					// Copy the region from the back buffer to the front buffer.
					unsigned char* __restrict bc  = ((unsigned char*)m_pin3d.m_pdds3Dbuffercopy) +top;
					unsigned char* __restrict bcz = ((unsigned char*)m_pin3d.m_pdds3Dbufferzcopy)+top;
					unsigned char* __restrict sf  = ((unsigned char*)ddsd.lpSurface) +top;
					unsigned char* __restrict sfz = ((unsigned char*)ddsdz.lpSurface)+top;
					for(int offset = top; offset < bottom; offset+=ddsd.lPitch,bc+=ddsd.lPitch,bcz+=ddsd.lPitch,sf+=ddsd.lPitch,sfz+=ddsd.lPitch)
					{
						memcpy/*_sse2*/(bc,  sf,  copywidth); //!! sse2 version slower here (vc10)?! or is this some other bug comin into play?
						memcpy/*_sse2*/(bcz, sfz, copywidth);
					}
				}
			}
			else
			{
#endif
				if((m_fStereo3DAA) || (m_fStereo3DY))
				{
					memcpy_sse2((void*)m_pin3d.m_pdds3Dbuffercopy, ddsd.lpSurface, ddsd.lPitch*height);
					memcpy_sse2((void*)m_pin3d.m_pdds3Dbufferzcopy,ddsdz.lpSurface,ddsd.lPitch*height);
				}
				else
				{
#pragma omp parallel for schedule(dynamic)
					for(int y = 0; y < (int)height; y+=2) { //!! opt to copy larger blocks somehow? //!! opt. muls?
						memcpy_sse2(((unsigned char* const __restrict)m_pin3d.m_pdds3Dbuffercopy) +ddsd.lPitch*y,  ((const unsigned char* const __restrict)ddsd.lpSurface)+ddsd.lPitch*y, ddsd.lPitch);
						memcpy_sse2(((unsigned char* const __restrict)m_pin3d.m_pdds3Dbufferzcopy)+ddsd.lPitch*y, ((const unsigned char* const __restrict)ddsdz.lpSurface)+ddsd.lPitch*y, ddsd.lPitch);
					}
				}
#ifdef ONLY3DUPD
			}
#endif
	const unsigned int oPitch = ddsd.lPitch >> shift;

	m_pin3d.m_pddsBackBuffer->Unlock(NULL);
	m_pin3d.m_pddsZBuffer->Unlock(NULL);

	ZeroMemory( &ddsd, sizeof(ddsd) );
	ddsd.dwSize = sizeof(ddsd);
#ifdef ONLY3DUPD
	hr = m_pin3d.m_pdds3DBackBuffer->Lock(NULL, &ddsd, DDLOCK_WAIT | DDLOCK_NOSYSLOCK | DDLOCK_SURFACEMEMORYPTR | DDLOCK_WRITEONLY, NULL);
#else
	hr = m_pin3d.m_pdds3DBackBuffer->Lock(NULL, &ddsd, DDLOCK_WAIT | DDLOCK_NOSYSLOCK | DDLOCK_SURFACEMEMORYPTR | DDLOCK_WRITEONLY | DDLOCK_DISCARDCONTENTS, NULL);
#endif

	      unsigned int* const __restrict bufferfinal = (unsigned int*)ddsd.lpSurface;
    if(!FAILED(hr) && (bufferfinal != NULL)) {
	const unsigned int* const __restrict buffercopy = m_pin3d.m_pdds3Dbuffercopy;
	const unsigned int* const __restrict bufferzcopy = m_pin3d.m_pdds3Dbufferzcopy;
	     unsigned char* const __restrict mask = m_pin3d.m_pdds3Dbuffermask;

	const unsigned int nPitch = ddsd.lPitch >> shift;

#ifndef FXAA

	const unsigned int maxSeparationU = m_fStereo3DY ? (unsigned int)(height*m_pin3d.m_maxSeparation) :
													   (unsigned int)(width*m_pin3d.m_maxSeparation);
	const unsigned int ZPDU = m_fStereo3DY ? (unsigned int)(16u * zmask * (height*m_pin3d.m_maxSeparation)*m_pin3d.m_ZPD) :
											 (unsigned int)(16u * zmask * (width*m_pin3d.m_maxSeparation)*m_pin3d.m_ZPD); // 16 = fixed point math for filtering pixels
	const unsigned int samples[3] = { m_fStereo3DY ? ((unsigned int)(0.5 * (height*m_pin3d.m_maxSeparation)))*oPitch :
													  (unsigned int)(0.5 * (width*m_pin3d.m_maxSeparation)),
									  m_fStereo3DY ? ((unsigned int)(0.666 * (height*m_pin3d.m_maxSeparation)))*oPitch :
													  (unsigned int)(0.666 * (width*m_pin3d.m_maxSeparation)),
									  m_fStereo3DY ? maxSeparationU*oPitch :
													 maxSeparationU }; // filter depth values instead of trunc?? (not necessary, would blur depth values anyhow?)

	const __m128 ZPDU128 = _mm_set1_ps((float)ZPDU * ((shift == 1) ? (float)(1.0/256.0) : 1.0f)); // in 16bit scale z value by 256 (16bit zbuffer values scaled to 24bit (=32-8stencil), so its the same as when rendering in 32bit)
	const float maxSepShl4 = (float)(maxSeparationU<<4);
	const __m128 maxSepShl4128 = _mm_set1_ps(maxSepShl4);
	const __m128i nPitch128 = _mm_set1_epi32(nPitch);

	ZeroMemory(mask,(width*height)>>2); //!! not necessary when full update

#ifdef ONLY3DUPD
			if (m_fCleanBlt)
			{
				// Smart Blit - only update the invalidated areas
#pragma omp parallel for schedule(dynamic)
				for (int i=0;i<m_vupdaterect.Size();++i)
				{
					const RECT& prc = m_vupdaterect.ElementAt(i)->m_rcupdate;

					const int left   = prc.left + m_pin3d.m_rcUpdate.left;
					const int right  = prc.right + m_pin3d.m_rcUpdate.left;
					const int top    = prc.top + m_pin3d.m_rcUpdate.top;
					const int bottom = prc.bottom + m_pin3d.m_rcUpdate.top;
					if((left >= right) || (top >= bottom))
						continue;

					// Update the region (+ area around) from the back buffer to the front buffer.
					if(m_fStereo3DY) {
						if(shift == 1)
							stereo_repro_16bit_y(max(top-(int)maxSeparationU,(int)maxSeparationU+1+1)&0xFFFFFFFE, min((unsigned int)bottom+maxSeparationU+1,height-(maxSeparationU+1))&0xFFFFFFFE, max(left,0)&0xFFFFFFFE, min((unsigned int)right+1,width)&0xFFFFFFFE, width,oPitch,nPitch,height,maxSeparationU,(unsigned short*)buffercopy,(unsigned short*)bufferzcopy,(unsigned short*)bufferfinal,samples,         ZPDU128,maxSepShl4128,false,m_fStereo3DAA,mask); //!! x: +1,&0xFFFFFFFE due to SSE //!! y: opt. for AA3D only(?):&0xFFFFFFFE, also bottom+1 then
						else
							stereo_repro_32bit_y(max(top-(int)maxSeparationU,(int)maxSeparationU+1+1)&0xFFFFFFFE, min((unsigned int)bottom+maxSeparationU+1,height-(maxSeparationU+1))&0xFFFFFFFE, max(left,0)&0xFFFFFFFC, min((unsigned int)right+3,width)&0xFFFFFFFC, width,oPitch,nPitch,height,maxSeparationU,(unsigned int  *)buffercopy,(unsigned int  *)bufferzcopy,(unsigned int  *)bufferfinal,samples,zmask128,ZPDU128,maxSepShl4128,false,m_fStereo3DAA,mask); //!! x: +3,&0xFFFFFFFC due to SSE //!! y: opt. for AA3D only(?):&0xFFFFFFFE, also bottom+1 then
					} else {
						if(shift == 1)
							stereo_repro_16bit_x(max(top,0)&0xFFFFFFFE, min((unsigned int)bottom+1,height)&0xFFFFFFFE, max(left-(int)maxSeparationU,(int)maxSeparationU+1+1)&0xFFFFFFFE, min(right+maxSeparationU+1,width-(maxSeparationU+1))&0xFFFFFFFE, width,oPitch,nPitch,height,maxSeparationU,(unsigned short*)buffercopy,(unsigned short*)bufferzcopy,(unsigned short*)bufferfinal,samples,         ZPDU128,maxSepShl4128,false,m_fStereo3DAA,mask); //!! x: +1,&0xFFFFFFFE due to SSE //!! y: opt. for AA3D only:&0xFFFFFFFE, also bottom+1 then
						else
							stereo_repro_32bit_x(max(top,0)&0xFFFFFFFE, min((unsigned int)bottom+1,height)&0xFFFFFFFE, max(left-(int)maxSeparationU,(int)maxSeparationU+1+3)&0xFFFFFFFC, min(right+maxSeparationU+3,width-(maxSeparationU+1))&0xFFFFFFFC, width,oPitch,nPitch,height,maxSeparationU,(unsigned int  *)buffercopy,(unsigned int  *)bufferzcopy,(unsigned int  *)bufferfinal,samples,zmask128,ZPDU128,maxSepShl4128,false,m_fStereo3DAA,mask); //!! x: +3,&0xFFFFFFFC due to SSE //!! y: opt. for AA3D only:&0xFFFFFFFE, also bottom+1 then
					}
				}
			}
			else
#endif
				if(m_fStereo3DY) {
					if(shift == 1)
	 					stereo_repro_16bit_y((maxSeparationU+1+1)&0xFFFFFFFE, (height-(maxSeparationU+1))&0xFFFFFFFE, 0, width&0xFFFFFFFE, width,oPitch,nPitch,height,maxSeparationU,(unsigned short*)buffercopy,(unsigned short*)bufferzcopy,(unsigned short*)bufferfinal,samples,         ZPDU128,maxSepShl4128,true,m_fStereo3DAA,mask); //!! mask not necessary for full update
					else
						stereo_repro_32bit_y((maxSeparationU+1+1)&0xFFFFFFFE, (height-(maxSeparationU+1))&0xFFFFFFFE, 0, width&0xFFFFFFFC, width,oPitch,nPitch,height,maxSeparationU,(unsigned int  *)buffercopy,(unsigned int  *)bufferzcopy,(unsigned int  *)bufferfinal,samples,zmask128,ZPDU128,maxSepShl4128,true,m_fStereo3DAA,mask);
				} else {
					if(shift == 1)
						stereo_repro_16bit_x(0, height, (maxSeparationU+1+3)&0xFFFFFFFE, (width-(maxSeparationU+1))&0xFFFFFFFE, width,oPitch,nPitch,height,maxSeparationU,(unsigned short*)buffercopy,(unsigned short*)bufferzcopy,(unsigned short*)bufferfinal,samples,         ZPDU128,maxSepShl4128,true,m_fStereo3DAA,mask);
					else
						stereo_repro_32bit_x(0, height, (maxSeparationU+1+3)&0xFFFFFFFC, (width-(maxSeparationU+1))&0xFFFFFFFC, width,oPitch,nPitch,height,maxSeparationU,(unsigned int  *)buffercopy,(unsigned int  *)bufferzcopy,(unsigned int  *)bufferfinal,samples,zmask128,ZPDU128,maxSepShl4128,true,m_fStereo3DAA,mask);
				}

#else // continue with FXAA //!! misses SSE(2) opt. //!! add update only version

#define FXAA_SPAN_MAX 8
#define OFFS (((FXAA_SPAN_MAX*8)>>4) + 1)

#pragma omp parallel for schedule(dynamic)
	for(int y = OFFS; y < (int)height - (OFFS+1); ++y) //!! border handling
	{
	unsigned int offsm1 = (y-1)*nPitch + (OFFS-1);
	for(int x = OFFS; x < (int)width - (OFFS+1); ++x,++offsm1) //!! border handling
	{
		//!! sliding window instead? (on y-1,y,y+1), incl. the filtered values already?
		const unsigned int rNW = buffercopy[offsm1]  &0xFCFCFC;
		const unsigned int rN  = buffercopy[offsm1+1]&0xFCFCFC;
		const unsigned int rNE = buffercopy[offsm1+2]&0xFCFCFC;
		const unsigned int offs = offsm1+nPitch;
		const unsigned int rW = buffercopy[offs]  &0xFCFCFC;
		const unsigned int rM = buffercopy[offs+1]&0xFCFCFC;
		const unsigned int rE = buffercopy[offs+2]&0xFCFCFC;
		const unsigned int offsp1 = offs+nPitch;
		const unsigned int rSW = buffercopy[offsp1]  &0xFCFCFC;
		const unsigned int rS  = buffercopy[offsp1+1]&0xFCFCFC;
		const unsigned int rSE = buffercopy[offsp1+1]&0xFCFCFC;

		const unsigned int rMrN = rM+rN;
		const unsigned int lumaNW = lumas2(rMrN+rNW+rW);
		const unsigned int lumaNE = lumas2(rMrN+rNE+rE);
		const unsigned int rMrS = rM+rS;
		const unsigned int lumaSW = lumas2(rMrS+rSW+rW);
		const unsigned int lumaSE = lumas2(rMrS+rSE+rE);
		const unsigned int lumaM  = luma(rM);

		unsigned int tempMin,tempMin2,tempMax,tempMax2; //!! use cmovs?
		if(lumaSW > lumaSE) {
			tempMax = lumaSW;
			tempMin = lumaSE;
		} else {
			tempMax = lumaSE;
			tempMin = lumaSW;
		}
		if(lumaNW > lumaNE) {
			tempMax2 = lumaNW;
			tempMin2 = lumaNE;
		} else {
			tempMax2 = lumaNE;
			tempMin2 = lumaNW;
		}
		const unsigned int lumaMin = min(lumaM, min(tempMin, tempMin2));
		const unsigned int lumaMax = max(lumaM, max(tempMax, tempMax2));

		const unsigned int SWSE = lumaSW + lumaSE;
		const unsigned int NWNE = lumaNW + lumaNE;
		int dirx = SWSE - NWNE;
		int diry = (lumaNW + lumaSW) - (lumaNE + lumaSE);

		const int temp = min(abs(dirx), abs(diry)) + (int)max((NWNE + SWSE)>>5, 2u);
		dirx = (dirx<<3)/temp;
		diry = (diry<<3)/temp;
		dirx = min(FXAA_SPAN_MAX*8, max(-FXAA_SPAN_MAX*8, dirx));
		diry = min(FXAA_SPAN_MAX*8, max(-FXAA_SPAN_MAX*8, diry));

		unsigned int rgbB = bilerpFE(buffercopy,x,y,dirx,diry,nPitch,width,height);
		dirx >>= 2;
		diry >>= 2;
		const unsigned int rgbA = bilerpFE(buffercopy,x,y,dirx,diry,nPitch,width,height);
		rgbB = ((rgbA&0xFEFEFE) + (rgbB&0xFEFEFE))>>1;
		const unsigned int lumaB = luma(rgbB);
		bufferfinal[offs+1] = ((lumaB < lumaMin) || (lumaB > lumaMax)) ? rgbA : rgbB;
	}
	}
#endif
	m_pin3d.m_pdds3DBackBuffer->Unlock(NULL);
	} else m_fStereo3Denabled = false; } else m_fStereo3Denabled = false; } else m_fStereo3Denabled = false; // 'handle' fails to lock buffers

	// Copy the entire back buffer to the front buffer.
	m_pin3d.Flip(0, 0, (m_fps > m_refreshrate*ADAPT_VSYNC_FACTOR));

	// Flag that we only need to update regions from now on...
	m_fCleanBlt = fTrue;
	}
#endif

	// Remove the list of update regions.
	// Note:  The ball and the mixer update rects are removed here as well...
	//        so if we need a clear, we need to do it somewhere else.
	for (int i=0;i<m_vupdaterect.Size();i++)
	{
		UpdateRect * const pur = m_vupdaterect.ElementAt(i);
		delete pur;
	}
	m_vupdaterect.RemoveAllElements();

#endif //GDIDRAW

#ifndef ACCURATETIMERS
	m_pactiveball = NULL;  // No ball is the active ball for timers/key events

	for (int i=0;i<m_vht.Size();i++)
	{
		HitTimer * const pht = m_vht.ElementAt(i);
		if (pht->m_nextfire <= m_timeCur)
		{
			pht->m_pfe->FireGroupEvent(DISPID_TimerEvents_Timer);
			pht->m_nextfire += pht->m_interval;
		}
	}
#endif
	
#ifdef ULTRAPIN
	// Draw hack lights.
	DrawLightHack();
#endif

	if (m_pxap)
	{
		if (!m_pxap->Tick())
		{
			delete m_pxap;
			m_pxap = NULL;
			m_ptable->FireVoidEvent(DISPID_GameEvents_MusicDone);
		}
	}

	for (int i=0;i<m_vballDelete.Size();i++)
	{
		Ball * const pball = m_vballDelete.ElementAt(i);
		delete pball->m_vpVolObjs;
		delete pball;
	}

	m_vballDelete.RemoveAllElements();

#ifdef FPS
	if (m_fShowFPS)
	{
		static U32 stamp;
		static U32 period;
		HDC hdcNull = GetDC(NULL);
		char szFoo[128];

		// Draw the amount of video memory used.
		int len = sprintf_s(szFoo, "Total Video Memory = %d", NumVideoBytes);
		TextOut(hdcNull, 10, 75, szFoo, len);

		// Draw the framerate.
		len = sprintf_s(szFoo, "%d %llu", m_fps, m_PhysicsStepTime);
		TextOut(hdcNull, 10, 10, szFoo, len);
		
		period = msec()-stamp;
		if( period > m_max ) m_max = period;
		if( phys_period > m_phys_max ) m_phys_max = phys_period;
		if( phys_iterations > m_phys_max_iterations ) m_phys_max_iterations = phys_iterations;
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

#ifdef DEBUG_FRATE
		{
#define TSIZE 20
			static int period[TSIZE];
			static int speriod[TSIZE];
			static int idx;
			
			period[idx] = period;
			idx++;
			if( idx >= TSIZE ) idx = 0;

			for(int i=0; i<TSIZE; i++)
			{
				len = sprintf_s( szFoo, "%d", period[i] );
				TextOut( hdcNull,  20 + i * 20, 10 + period[i], szFoo, len );
			}
		}
#endif
	
#ifdef _DEBUGPHYSICS
		len = sprintf_s(szFoo, sizeof(szFoo), "period: %3d ms (%3d avg %10d max)      ",
		period, (U32)( m_total / m_count ), (U32) m_max );
		TextOut(hdcNull, 10, 120, szFoo, len);

		len = sprintf_s(szFoo, sizeof(szFoo), "physTimes %10d uS(%12d avg %12d max)    ",
			   	(U32)phys_period,
			   	(U32)(m_phys_total / m_count),
			   	m_phys_max );
		TextOut(hdcNull, 10, 140, szFoo, len);

		len = sprintf_s(szFoo, sizeof(szFoo), "phys:%5d iterations(%5d avg %5d max))   ",
			   	(U32)phys_iterations,
			   	(U32)( m_phys_total_iterations / m_count ),
				(U32)m_phys_max_iterations );
		TextOut(hdcNull, 10, 160, szFoo, len);

		len = sprintf_s(szFoo, sizeof(szFoo), "Hits:%5d Collide:%5d Ctacs:%5d Static:%5d Embed: %5d    ",
		c_hitcnts, c_collisioncnt, c_contactcnt, c_staticcnt, c_embedcnts);
		TextOut(hdcNull, 10, 180, szFoo, len);
#endif

		stamp = msec();
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

			if(m_fCloseType == 2) 
			{
				exit(-9999); // blast into space
			}
			else if( !VPinball::m_open_minimized && m_fCloseType == 0)
			{
				option = DialogBox(g_hinstres, MAKEINTRESOURCE(IDD_GAMEPAUSE), m_hwnd, PauseProc);
			}
			else //m_fCloseType == all others
			{
				option = ID_QUIT;
				SendMessage(g_pvp->m_hwnd, WM_COMMAND, ID_FILE_EXIT, NULL );
			}
		
			m_fCloseDown = fFalse;
			g_pplayer->m_fNoTimeCorrect = fTrue; // Skip the time we were in the dialog
			UnpauseMusic();
			if (option == ID_QUIT)
			{
				SendMessage(m_hwnd, WM_CLOSE, 0, 0); 				// This line returns to the editor after exiting a table

/*				//unload VPM - works first time, crashes at rendering animations next time vpm is loaded by script
				HMODULE hmod;
				hmod=GetModuleHandle("VPinMAME.dll");
				if(hmod !=NULL)
				{
					ShowError("handle obtained");
			
					FreeLibrary(hmod);
					{
						//ShowError("Failed!");
					}
				}
*/
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
	// Nobody likes the ball shadows.		- JEP
	//return;

	{
	D3DMATERIAL7 mtrl;
	mtrl.diffuse.r = mtrl.ambient.r =
	mtrl.diffuse.g = mtrl.ambient.g =
	mtrl.diffuse.b = mtrl.ambient.b =
	mtrl.diffuse.a = mtrl.ambient.a = 1.0f;
	mtrl.specular.r = mtrl.specular.g =	mtrl.specular.b = mtrl.specular.a =
	mtrl.emissive.r = mtrl.emissive.g =	mtrl.emissive.b = mtrl.emissive.a =
	mtrl.power = 0;	
	m_pin3d.m_pd3dDevice->SetMaterial(&mtrl);
	}

	m_pin3d.m_pd3dDevice->SetRenderState(D3DRENDERSTATE_TEXTUREPERSPECTIVE, FALSE );

	m_pin3d.m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ADDRESS, D3DTADDRESS_CLAMP);// WRAP

	//const float sn = sinf(m_pin3d.m_inclination);
	//const float cs = cosf(m_pin3d.m_inclination);

	for (int i=0;i<m_vball.Size();i++)
		{
		Ball * const pball = m_vball.ElementAt(i);

		if (m_fBallShadows)
			{
			m_pin3d.m_pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
			m_pin3d.m_pd3dDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE);

			m_pin3d.m_pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHAREF, (DWORD)0x0000001);
			m_pin3d.m_pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHAFUNC, D3DCMP_GREATEREQUAL);
			m_pin3d.m_pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, TRUE); 

			Ball ballT;
			ballT.x = pball->x;
			ballT.y = pball->y;
			ballT.z = pball->z;
			ballT.vx = 200.0f;
			ballT.vy = -200.0f;
			ballT.vz = -200.0f;
			ballT.radius = 0;
			
			ballT.m_hittime = 1.0f;
			
			ballT.CalcBoundingRect();

			m_shadowoctree.HitTestBall(&ballT);

			float offsetx;
			float offsety;
			float shadowz;

			if (ballT.m_hittime < 1.0f) // shadow falls on an object
				{
				offsetx = ballT.m_hittime * 200.0f - 12.5f;
				offsety = ballT.m_hittime * -200.0f + 12.5f;
				shadowz = pball->z + 0.1f - ballT.m_hittime * 200.0f;				
				}
			else // shadow is on the floor
				{
				offsetx = pball->z*0.5f;
				offsety = pball->z*-0.5f;
				shadowz = 0.1f; //pball->z - pball->radius + 0.1f;
				}

			const float shadowradius = pball->radius*1.2f;
			const float shadowradiusX = shadowradius * m_BallStretchX;
			const float shadowradiusY = shadowradius * m_BallStretchY;
			const float inv_shadowradius = 0.5f/shadowradius;

			Vertex3D * const rgv3DShadow = pball->m_rgv3DShadow;
			rgv3DShadow[0].x = pball->x - shadowradiusX + offsetx;
			rgv3DShadow[0].y = pball->y - shadowradiusY + offsety;
			rgv3DShadow[0].z = shadowz;
			rgv3DShadow[0].tu = 0;
			rgv3DShadow[0].tv = 0;
			rgv3DShadow[0].nx = 0;
			rgv3DShadow[0].ny = 0;
			rgv3DShadow[0].nz = -1.0f;

			rgv3DShadow[1].x = pball->x + shadowradiusX + offsetx;
			rgv3DShadow[1].y = pball->y - shadowradiusY + offsety;
			rgv3DShadow[1].z = shadowz;
			rgv3DShadow[1].tu = 1.0f;
			rgv3DShadow[1].tv = 0;
			rgv3DShadow[1].nx = 0;
			rgv3DShadow[1].ny = 0;
			rgv3DShadow[1].nz = -1.0f;

			rgv3DShadow[2].x = pball->x + shadowradiusX + offsetx;
			rgv3DShadow[2].y = pball->y + shadowradiusY + offsety;
			rgv3DShadow[2].z = shadowz;
			rgv3DShadow[2].tu = 1.0f;
			rgv3DShadow[2].tv = 1.0f;
			rgv3DShadow[2].nx = 0;
			rgv3DShadow[2].ny = 0;
			rgv3DShadow[2].nz = -1.0f;

			rgv3DShadow[3].x = pball->x - shadowradiusX + offsetx;
			rgv3DShadow[3].y = pball->y + shadowradiusY + offsety;
			rgv3DShadow[3].z = shadowz;
			rgv3DShadow[3].tu = 0;
			rgv3DShadow[3].tv = 1.0f;
			rgv3DShadow[3].nx = 0;
			rgv3DShadow[3].ny = 0;
			rgv3DShadow[3].nz = -1.0f;

			if (!pball->fFrozen && rgv3DShadow[0].x <= m_ptable->m_right && rgv3DShadow[2].y >= m_ptable->m_top)
				{
				if (rgv3DShadow[2].x > m_ptable->m_right)
					{
					const float newtu = (rgv3DShadow[2].x - m_ptable->m_right) * inv_shadowradius;
					rgv3DShadow[2].tu = 1.0f-newtu;
					rgv3DShadow[1].tu = 1.0f-newtu;
					rgv3DShadow[2].x = m_ptable->m_right;
					rgv3DShadow[1].x = m_ptable->m_right;
					}

				if (rgv3DShadow[1].y < m_ptable->m_top)
					{
					const float newtv = (m_ptable->m_top - rgv3DShadow[1].y) * inv_shadowradius;
					rgv3DShadow[1].tv = newtv;
					rgv3DShadow[0].tv = newtv;
					rgv3DShadow[1].tv = m_ptable->m_top;
					rgv3DShadow[0].tv = m_ptable->m_top;
					}

				m_pin3d.m_pd3dDevice->SetTexture(0, m_pin3d.m_pddsShadowTexture);

				m_pin3d.m_pd3dDevice->SetRenderState(D3DRENDERSTATE_COLORKEYENABLE, FALSE);

				//m_pin3d.m_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX, rgv3DShadow, 4,0);

				m_pin3d.m_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,
															  rgv3DShadow, 4,
															  (LPWORD)rgi0123, 4, NULL);
				}
			}
		}
	}


void Player::DrawBalls()
	{
	//m_pin3d.m_pd3dDevice->SetRenderState(D3DRENDERSTATE_LIGHTING, FALSE);

	m_pin3d.m_pd3dDevice->SetRenderState(D3DRENDERSTATE_TEXTUREPERSPECTIVE, FALSE );

	m_pin3d.m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ADDRESS, D3DTADDRESS_CLAMP/*WRAP*/);

	D3DMATERIAL7 mtrl;
	mtrl.specular.r = mtrl.specular.g =	mtrl.specular.b = mtrl.specular.a =
	mtrl.emissive.r = mtrl.emissive.g =	mtrl.emissive.b = mtrl.emissive.a =
	mtrl.power = 0;
	mtrl.diffuse.a = mtrl.ambient.a = 1.0f;

	const float sn = sinf(m_pin3d.m_inclination);
	const float cs = cosf(m_pin3d.m_inclination);

	for (int i=0;i<m_vball.Size();i++)
		{
		Ball * const pball = m_vball.ElementAt(i);
		float radiusX = pball->radius * m_BallStretchX;
		float radiusY = pball->radius * m_BallStretchY;

		const float r = (pball->m_color & 255) * (float)(1.0/255.0);
		const float g = (pball->m_color & 65280) * (float)(1.0/65280.0);
		const float b = (pball->m_color & 16711680) * (float)(1.0/16711680.0);
		mtrl.diffuse.r = mtrl.ambient.r = r;
		mtrl.diffuse.g = mtrl.ambient.g = g;
		mtrl.diffuse.b = mtrl.ambient.b = b;
		m_pin3d.m_pd3dDevice->SetMaterial(&mtrl);

		const float zheight = (!pball->fFrozen) ? pball->z : (pball->z - pball->radius);

		Vertex3D * const rgv3D = pball->m_rgv3D;
		rgv3D[0].x = pball->x - radiusX;
		rgv3D[0].y = pball->y - (radiusY * cs);
		rgv3D[0].z = zheight + (pball->radius * sn);
		rgv3D[0].tu = 0;
		rgv3D[0].tv = 0;
		rgv3D[0].nx = 0;
		rgv3D[0].ny = 0;
		rgv3D[0].nz = -1.0f;

		rgv3D[3].x = pball->x - radiusX;
		rgv3D[3].y = pball->y + (radiusY * cs);
		rgv3D[3].z = zheight - (pball->radius * sn);
		rgv3D[3].tu = 0;
		//rgv3D[3].tv = 1.0f; // decided by ball picture
		rgv3D[3].nx = 0;
		rgv3D[3].ny = 0;
		rgv3D[3].nz = -1.0f;

		rgv3D[2].x = pball->x + radiusX;
		rgv3D[2].y = pball->y + (radiusY * cs);
		rgv3D[2].z = zheight - (pball->radius * sn);
		//rgv3D[2].tu = 1.0f;  // decided by ball picture
		//rgv3D[2].tv = 1.0f;  // decided by ball picture
		rgv3D[2].nx = 0;
		rgv3D[2].ny = 0;
		rgv3D[2].nz = -1.0f;

		rgv3D[1].x = pball->x + radiusX;
		rgv3D[1].y = pball->y - (radiusY * cs);
		rgv3D[1].z = zheight + (pball->radius * sn);
		//rgv3D[1].tu = 1.0f;  // decided by ball picture
		rgv3D[1].tv = 0;
		rgv3D[1].nx = 0;
		rgv3D[1].ny = 0;
		rgv3D[1].nz = -1.0f;

		if (!pball->m_pin)
			{
			m_pin3d.m_pd3dDevice->SetTexture(0, m_pin3d.m_pddsBallTexture);
			rgv3D[3].tv = 1.0f;
			rgv3D[2].tu = 1.0f;
			rgv3D[2].tv = 1.0f;
			rgv3D[1].tu = 1.0f;
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

		m_pin3d.m_pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHAREF, (DWORD)0x0000001);
		m_pin3d.m_pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, TRUE); 
		m_pin3d.m_pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHAFUNC, D3DCMP_GREATEREQUAL);

		/*m_pin3d.m_lightproject.CalcCoordinates(&rgv3D[0]);
		m_pin3d.m_lightproject.CalcCoordinates(&rgv3D[1]);
		m_pin3d.m_lightproject.CalcCoordinates(&rgv3D[2]);
		m_pin3d.m_lightproject.CalcCoordinates(&rgv3D[3]);*/

		m_pin3d.m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE);
		//m_pin3d.m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
		m_pin3d.m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTFG_LINEAR);
		m_pin3d.m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTFN_LINEAR);

		m_pin3d.m_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,
												  rgv3D, 4,
												  (LPWORD)rgi0123, 4, NULL);
/*
		This does not work, i don't know why. - Cupid

		m_pin3d.m_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,
												  rgv3D, 4,
												  NULL);
*/

		// Draw the ball logo
		Vertex3D rgv3DArrowTransformed[4];
		Vertex3D rgv3DArrowTransformed2[4];
		if (m_fBallDecals && (pball->m_pinFront || pball->m_pinBack))
			{
			/*mtrl.diffuse.r = mtrl.ambient.r = 0.8f;
			mtrl.diffuse.g = mtrl.ambient.g = 0.4f;
			mtrl.diffuse.b = mtrl.ambient.b = 0.2f;*/
			mtrl.diffuse.a = mtrl.ambient.a = 0.8f;//0.7f;
			m_pin3d.m_pd3dDevice->SetMaterial(&mtrl);

				Vertex3D rgv3DArrow[4];

				rgv3DArrow[0].tu = 0;
				rgv3DArrow[0].tv = 0;
				rgv3DArrow[0].x = -0.333333333f;
				rgv3DArrow[0].y = -0.333333333f;
				rgv3DArrow[0].z = -0.881917103f;

				rgv3DArrow[1].tu = 1.0f;
				rgv3DArrow[1].tv = 0;
				rgv3DArrow[1].x = 0.333333333f;
				rgv3DArrow[1].y = -0.333333333f;
				rgv3DArrow[1].z = -0.881917103f;

				rgv3DArrow[2].tu = 1.0f;
				rgv3DArrow[2].tv = 1.0f;
				rgv3DArrow[2].x = 0.333333333f;
				rgv3DArrow[2].y = 0.333333333f;
				rgv3DArrow[2].z = -0.881917103f;

				rgv3DArrow[3].tu = 0;
				rgv3DArrow[3].tv = 1.0f;
				rgv3DArrow[3].x = -0.333333333f;
				rgv3DArrow[3].y = 0.333333333f;
				rgv3DArrow[3].z = -0.881917103f;

				m_pin3d.m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE);
				m_pin3d.m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTFG_LINEAR);
				m_pin3d.m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTFN_LINEAR);
				m_pin3d.m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MIPFILTER, D3DTFP_LINEAR);

				m_pin3d.m_pd3dDevice->SetRenderState(D3DRENDERSTATE_COLORKEYENABLE, FALSE);
				m_pin3d.m_pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);

				m_pin3d.m_pd3dDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE);
				
				// Scale the orientation for Ball stretching
				Matrix3 orientation;
				orientation.Identity();
				orientation.scaleX(m_BallStretchX);
				orientation.scaleY(m_BallStretchY);
				orientation.MultiplyMatrix(&orientation, &pball->m_orientation);
				if (pball->m_pinFront)
					{
					pball->m_pinFront->EnsureColorKey();
					m_pin3d.m_pd3dDevice->SetTexture(0, pball->m_pinFront->m_pdsBufferColorKey);

					for (int iPoint=0;iPoint<4;iPoint++)
						{
						const Vertex3Ds tmp = orientation.MultiplyVector(rgv3DArrow[iPoint]);
						rgv3DArrowTransformed[iPoint].nx = tmp.x;
						rgv3DArrowTransformed[iPoint].ny = tmp.y;
						rgv3DArrowTransformed[iPoint].nz = tmp.z;
						rgv3DArrowTransformed[iPoint].x = pball->x - tmp.x*pball->radius;
						rgv3DArrowTransformed[iPoint].y = pball->y - tmp.y*pball->radius;
						rgv3DArrowTransformed[iPoint].z = zheight  - tmp.z*pball->radius;
						rgv3DArrowTransformed[iPoint].tu = rgv3DArrow[iPoint].tu * pball->m_pinFront->m_maxtu;
						rgv3DArrowTransformed[iPoint].tv = rgv3DArrow[iPoint].tv * pball->m_pinFront->m_maxtv;
						}

					//m_pin3d.m_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,
															  //rgv3DArrowTransformed, 4,
															  //rgi0123, DECALPOINTS, NULL);

					m_pin3d.m_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,
															  rgv3DArrowTransformed, 4,
															  (LPWORD)rgi0123, 4, NULL);

					}
				orientation.Identity();
				orientation.scaleX(m_BallStretchX);
				orientation.scaleY(m_BallStretchY);
				orientation.MultiplyMatrix(&orientation, &pball->m_orientation);
				if (pball->m_pinBack)
					{
					// Other side of ball
					pball->m_pinBack->EnsureColorKey();
					m_pin3d.m_pd3dDevice->SetTexture(0, pball->m_pinBack->m_pdsBufferColorKey);

					for (int iPoint=0;iPoint<4;iPoint++)
						{
						rgv3DArrow[iPoint].x = -rgv3DArrow[iPoint].x;
						rgv3DArrow[iPoint].z = -rgv3DArrow[iPoint].z;
						const Vertex3Ds tmp = orientation.MultiplyVector(rgv3DArrow[iPoint]);
						rgv3DArrowTransformed2[iPoint].nx = tmp.x;
						rgv3DArrowTransformed2[iPoint].ny = tmp.y;
						rgv3DArrowTransformed2[iPoint].nz = tmp.z;
						rgv3DArrowTransformed2[iPoint].x = pball->x - tmp.x*pball->radius;
						rgv3DArrowTransformed2[iPoint].y = pball->y - tmp.y*pball->radius;
						rgv3DArrowTransformed2[iPoint].z = zheight  - tmp.z*pball->radius;
						rgv3DArrowTransformed2[iPoint].tu = rgv3DArrow[iPoint].tu * pball->m_pinBack->m_maxtu;
						rgv3DArrowTransformed2[iPoint].tv = rgv3DArrow[iPoint].tv * pball->m_pinBack->m_maxtv;
						}

					/*m_pin3d.m_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,
															  rgv3DArrowTransformed, 4,
															  rgi0123, DECALPOINTS, NULL);*/

					m_pin3d.m_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,
															  rgv3DArrowTransformed2, 4,
															  (LPWORD)rgi0123, 4, NULL);
					}
			}

			pball->m_fErase = true;

			// Mark ball rect as dirty for blitting to the screen
			m_pin3d.ClearExtents(&pball->m_rcScreen, NULL, NULL);
			m_pin3d.ExpandExtentsPlus(&pball->m_rcScreen, pball->m_rgv3D, NULL, NULL, 4, fFalse);

			if (m_fBallDecals && pball->m_pinFront && (m_ptable->m_layback > 0))
				m_pin3d.ExpandExtentsPlus(&pball->m_rcScreen, rgv3DArrowTransformed, NULL, NULL, 4, fFalse);
			if (m_fBallDecals && pball->m_pinBack && (m_ptable->m_layback > 0))
				m_pin3d.ExpandExtentsPlus(&pball->m_rcScreen, rgv3DArrowTransformed2, NULL, NULL, 4, fFalse);

			if (m_fBallShadows)
				{
				m_pin3d.ClearExtents(&pball->m_rcScreenShadow, NULL, NULL);
				m_pin3d.ExpandExtentsPlus(&pball->m_rcScreenShadow, pball->m_rgv3DShadow, NULL, NULL, 4, fFalse);

				if (fIntRectIntersect(pball->m_rcScreen, pball->m_rcScreenShadow))
					{
					pball->m_rcScreen.left = min(pball->m_rcScreen.left, pball->m_rcScreenShadow.left);
					pball->m_rcScreen.top = min(pball->m_rcScreen.top, pball->m_rcScreenShadow.top);
					pball->m_rcScreen.right = max(pball->m_rcScreen.right, pball->m_rcScreenShadow.right);
					pball->m_rcScreen.bottom = max(pball->m_rcScreen.bottom, pball->m_rcScreenShadow.bottom);
					}
				else
					{
					InvalidateRect(&pball->m_rcScreenShadow);
					}
				}
			InvalidateRect(&pball->m_rcScreen);
		}

	m_pin3d.m_pd3dDevice->SetTexture(0, NULL);
	m_pin3d.m_pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, FALSE);
}

void Player::InvalidateRect(RECT * const prc)
	{
	// This assumes the caller does not need *prc any more!!!
	// Either that, or we assume it can be permanently changed,
	// Because we never care about redrawing stuff off the screen.
	if (prc->top < 0)
		{
		prc->top = 0;
		}

	UpdateRect * const pur = new UpdateRect();
	pur->m_rcupdate = *prc;
	pur->m_fSeeThrough = fTrue;

	// Check all animated objects.
	for (int i=0;i<m_vscreenupdate.Size();++i)
		{
		// Get the bounds of this animated object.
		const RECT * const prc2 = &m_vscreenupdate.ElementAt(i)->m_rcBounds;

		// Check if the bounds of the animated object are within the bounds of our invalid rectangle.
		if ((prc->right >= prc2->left) && (prc->left <= prc2->right) && (prc->bottom >= prc2->top) && (prc->top <= prc2->bottom))
			{
			// Add to this rect's list of objects that need to be redrawn.
			pur->m_vobject.AddElement(m_vscreenupdate.ElementAt(i));
			}
		}

	// Add the rect.
	m_vupdaterect.AddElement(pur);

	}

#ifdef LOG
int cTested;
int cDeepTested;
int cTotalTested = 0;
int cNumUpdates = 0;
#endif


struct DebugMenuItem
	{
	int objectindex;
	VectorInt<int> *pvdispid;
	HMENU hmenu;
	};

void AddEventToDebugMenu(char *sz, int index, int dispid, LPARAM lparam)
{
	DebugMenuItem * const pdmi = (DebugMenuItem *)lparam;
	HMENU hmenu = pdmi->hmenu;
	const int menuid = ((pdmi->objectindex+1)<<16) | pdmi->pvdispid->Size();
	pdmi->pvdispid->AddElement(dispid);
	AppendMenu(hmenu, MF_STRING, menuid, sz);
	
}

void Player::DoDebugObjectMenu(int x, int y)
	{
	if (g_pplayer->m_vdebugho.Size() == 0)
		{
		// First time the debug hit-testing has been used
		g_pplayer->InitDebugHitStructure();
		}

	Matrix3D mat3D = g_pplayer->m_pin3d.m_matrixTotal;
	mat3D.Invert();

	D3DVIEWPORT7 vp;
	g_pplayer->m_pin3d.m_pd3dDevice->GetViewport( &vp );
	const float rClipWidth  = (float)vp.dwWidth*0.5f;
	const float rClipHeight = (float)vp.dwHeight*0.5f;

	const float xcoord = ((float)x-rClipWidth)/rClipWidth;
	const float ycoord = (-((float)y-rClipHeight))/rClipHeight;

	Vertex3D vT, vT2;
	g_pplayer->m_pin3d.m_matrixTotal.MultiplyVector(798,1465,89,&vT);
	mat3D.MultiplyVector(vT.x,vT.y,vT.z,&vT2);

	// Use the inverse of our 3D transform to determine where in 3D space the
	// screen pixel the user clicked on is at.  Get the point at the near
	// clipping plane (z=0) and the far clipping plane (z=1) to get the whole
	// range we need to hit test
	Vertex3Ds v3d, v3d2;
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
	ballT.m_hittime = 1.0f;
	ballT.CalcBoundingRect();

	//const float slope = (v3d2.y - v3d.y)/(v3d2.z - v3d.z);
	//const float yhit = v3d.y - (v3d.z*slope);

	//const float slopex = (v3d2.x - v3d.x)/(v3d2.z - v3d.z);
	//const float xhit = v3d.x - (v3d.z*slopex);

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

	for (int i=0;i<vhoHit.Size();i++)
		{
		HitObject * const pho = vhoHit.ElementAt(i);
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

			IDebugCommands * const pdc = pho->m_pfedebug->GetDebugCommands();
			if (pdc)
				{
				VectorInt<int> vids;
				VectorInt<int> vcommandid;

				pdc->GetDebugCommands(&vids, &vcommandid);
				for (int l=0;l<vids.Size();l++)
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

	const int icmd = TrackPopupMenuEx(hmenu, TPM_RETURNCMD | TPM_RIGHTBUTTON,
									  pt.x, pt.y, m_hwnd, NULL);

	if (icmd != 0 && vsubmenu.Size() > 0)
		{
		const int highword = HIWORD(icmd) - 1;
		const int lowword = icmd & 0xffff;
		IFireEvents * const pfe = vhoHit.ElementAt(highword)->m_pfedebug;
		if (lowword & 0x8000) // custom debug command
			{
			pfe->GetDebugCommands()->RunDebugCommand(lowword & 0x7fff);
			}
		else
			{
			const int dispid = vvdispid.ElementAt(highword)->ElementAt(lowword);
			g_pplayer->m_pactiveball = m_pactiveballDebug;
			pfe->FireGroupEvent(dispid);
			g_pplayer->m_pactiveball = NULL;
			}
		}

	DestroyMenu(hmenu);
	for (int i=0;i<vsubmenu.Size();i++)
		{
		DestroyMenu(vsubmenu.ElementAt(i));
		}

	for (int i=0;i<vvdispid.Size();i++)
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
//            mixer_display_volume(); only display volume when volume is changed by the user
            break;

		case WM_CLOSE:
#ifdef ULTRAPIN
			// Close the DMD hack window.
			SendMessage(g_pplayer->m_dmdhackhwnd, WM_CLOSE, 0, 0);
#endif
			break;

		case WM_DESTROY:
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
            hid_shutdown();
			
			break;

		case WM_PAINT:
			g_pplayer->m_pin3d.Flip(0,0,0);
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

#ifdef STEPPING
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
#endif

		case WM_RBUTTONUP:
			{
			if (g_pplayer->m_fDebugMode)
				{
				const int x = lParam & 0xffff;
				const int y = (lParam>>16) & 0xffff;
				g_pplayer->DoDebugObjectMenu(x, y);
				}
			return 0;
			}
			break;

			case WM_ACTIVATE:
#ifdef VBA
		
			g_pvp->ApcHost.WmActivate(wParam);
			break;

		case WM_ENABLE:
			g_pvp->ApcHost.WmEnable(wParam);
			
#else	
#ifdef  ULTRACADE
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
			
#else
			g_pplayer->m_fGameWindowActive = (wParam != WA_INACTIVE);
			g_pplayer->RecomputePauseState();			
#endif
#endif
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
								Coins += lParam;
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
				//int idCtrl = (int) wParam;
				NMHDR *pnmh = (LPNMHDR) lParam;
				SCNotification *pscnmh = (SCNotification *)lParam;
				//HWND hwndRE = pnmh->hwndFrom;
				const int code = pnmh->code;
				
				switch (code)
					{
					case SCN_CHARADDED:
						if (pscnmh->ch == '\n')
							{
							SendMessage(pnmh->hwndFrom, SCI_DELETEBACK, 0, 0);
							
							const int curpos = SendMessage(pnmh->hwndFrom, SCI_GETCURRENTPOS, 0, 0);
							const int line = SendMessage(pnmh->hwndFrom, SCI_LINEFROMPOSITION, curpos, 0);
							const int lineStart = SendMessage(pnmh->hwndFrom, SCI_POSITIONFROMLINE, line, 0);
							const int lineEnd = SendMessage(pnmh->hwndFrom, SCI_GETLINEENDPOSITION, line, 0);

							char * const szText = new char[lineEnd - lineStart + 1];
							TextRange tr;
							tr.chrg.cpMin = lineStart;
							tr.chrg.cpMax = lineEnd;
							tr.lpstrText = szText;
							SendMessage(pnmh->hwndFrom, SCI_GETTEXTRANGE, 0, (LPARAM)&tr);
							
							const int maxlines = SendMessage(pnmh->hwndFrom, SCI_GETLINECOUNT, 0, 0);

							if (maxlines == line+1)
								{
								// need to add a new line to the end
								SendMessage(pnmh->hwndFrom, SCI_DOCUMENTEND, 0, 0);
								SendMessage(pnmh->hwndFrom, SCI_ADDTEXT, lstrlen("\n"), (LPARAM)"\n");
								}
							else
								{
								const int pos = SendMessage(pnmh->hwndFrom, SCI_POSITIONFROMLINE, line+1, 0);
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
				g_pplayer->m_fDebugWindowActive = (wParam != WA_INACTIVE);
				g_pplayer->RecomputePauseState();
				g_pplayer->RecomputePseudoPauseState();
				break;

			case RESIZE_FROM_EXPAND:
				{
				const int state = SendDlgItemMessage(hwndDlg, IDC_EXPAND, BM_GETCHECK, 0, 0);
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

				const int diffx = rcSizer2.right - rcSizer1.right;
				const int diffy = rcSizer2.bottom - rcSizer1.bottom;

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
				{
				RECT rcDialog;
				RECT rcMain;
				GetWindowRect(GetParent(hwndDlg), &rcMain);
				GetWindowRect(hwndDlg, &rcDialog);

				SetWindowPos(hwndDlg, NULL,
				(rcMain.right + rcMain.left)/2 - (rcDialog.right - rcDialog.left)/2,
				(rcMain.bottom + rcMain.top)/2 - (rcDialog.bottom - rcDialog.top)/2,
				0, 0, SWP_NOOWNERZORDER | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE/* | SWP_NOMOVE*/);

				return TRUE;
				}
			case WM_COMMAND:
				switch (HIWORD(wParam))
					{
					case BN_CLICKED:
						switch (LOWORD(wParam))
							{
							case ID_RESUME:
								{
								EndDialog(hwndDlg, ID_RESUME);
								}
								break;
							case ID_DEBUGWINDOW:
								{
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
								}
								break;

							case ID_QUIT:
								{
								EndDialog(hwndDlg, ID_QUIT);
								}
								break;
							}
					}
				break;
			}

	return FALSE;
	}

#ifdef PLAYBACK

float Player::ParseLog(LARGE_INTEGER *pli1, LARGE_INTEGER *pli2)
	{
	char szLine[1024];
	const float dtime = 0.45f;

	while (1)
		{
		int c=0;

		while ((szLine[c] = getc(m_fplaylog)) != '\n')
			{
			if (szLine[c] == EOF)
				{
				fclose(m_fplaylog);
				m_fPlayback = fFalse;
				m_fplaylog = NULL;
				return dtime;
				}
			c++;
			}

		char szWord[64];
		char szSubWord[64];
		int index;
		sscanf(szLine, "%s",szWord);

		if (!strcmp(szWord,"Key"))
			{
			sscanf(szLine, "%s %s %d",szWord, szSubWord,&index);
			if (!strcmp(szSubWord, "Down"))
				{
				g_pplayer->m_ptable->FireKeyEvent(DISPID_GameEvents_KeyDown, index);
				}
			else // Release
				{
				g_pplayer->m_ptable->FireKeyEvent(DISPID_GameEvents_KeyUp, index);
				}
			}
		else if (!strcmp(szWord, "Physics"))
			{
			sscanf(szLine, "%s %s %f",szWord, szSubWord, &dtime);
			}
		else if (!strcmp(szWord, "Frame"))
			{
			int a,b,c,d;
			sscanf(szLine, "%s %s %f %u %u %u %u",szWord, szSubWord, &dtime, &a, &b, &c, &d);
			pli1->HighPart = a;
			pli1->LowPart = b;
			pli2->HighPart = c;
			pli2->LowPart = d;
			}
		else if (!strcmp(szWord, "Step"))
			{
			int a,b,c,d;
			sscanf(szLine, "%s %s %u %u %u %u",szWord, szSubWord, &a, &b, &c, &d);
			pli1->HighPart = a;
			pli1->LowPart = b;
			pli2->HighPart = c;
			pli2->LowPart = d;
			}
		else if (!strcmp(szWord,"End"))
			{
			return dtime;
			}
		}
	}

#endif


// Draws all transparent ramps that are in the vicinity of the ball.
void Player::DrawAcrylics ()
	{

	if (g_pvp->m_pdd.m_fHardwareAccel)
	{
		// Build a set of clipping planes which tightly bound the ball.
		// Turn off z writes for same values.  It fixes the problem of ramps rendering twice. 
		g_pplayer->m_pin3d.m_pd3dDevice->SetRenderState(D3DRENDERSTATE_ZFUNC,D3DCMP_LESS);
	}

	// the helper list of m_vhitacrylic only contains objects which evaluated to true in the old code.
	// it is created once on startup and never changed during play. (SnailGary)
	for (int i=0;i<m_vhitacrylic.Size();i++)
		m_vhitacrylic.ElementAt(i)->PostRenderStatic(m_pin3d.m_pd3dDevice);

// AMD profiler shows a lot of activity inside this block at runtime... so I decided to make a new list with
// hitable-only objects which saves a lot of dereferencing/checks at runtime (SnailGary)
/*
  // Check if we are hardware accelerated.
	if (g_pvp->m_pdd.m_fHardwareAccel)
		{
		// Build a set of clipping planes which tightly bound the ball.
		// Turn off z writes for same values.  It fixes the problem of ramps rendering twice. 
		g_pplayer->m_pin3d.m_pd3dDevice->SetRenderState(D3DRENDERSTATE_ZFUNC,D3DCMP_LESS);

		// Draw acrylic ramps (they have transparency, so they have to be drawn last).
		for (int i=0;i<m_ptable->m_vedit.Size();i++)
			{
				if (m_ptable->m_vedit.ElementAt(i)->GetItemType() == eItemRamp ||
					m_ptable->m_vedit.ElementAt(i)->GetItemType() == eItemPrimitive)
				{
				Hitable * const ph = m_ptable->m_vedit.ElementAt(i)->GetIHitable();
				if (ph)
					{
					ph->PostRenderStatic(m_pin3d.m_pd3dDevice);
					}
				}
			}
		}
	else
		{
		for (int i=0;i<m_ptable->m_vedit.Size();i++)
			{
				if (m_ptable->m_vedit.ElementAt(i)->GetItemType() == eItemPrimitive)
				{
				Hitable * const ph = m_ptable->m_vedit.ElementAt(i)->GetIHitable();
				if (ph)
					{
					ph->PostRenderStatic(m_pin3d.m_pd3dDevice);
					}
				}
			}
		}
*/ 
	}


int get_dongle_status()
{
	int		Status;
#ifndef ULTRA_FREE
#ifdef DONGLE_SUPPORT
	//Initialize.
	Status = DONGLE_STATUS_NOTFOUND;

	// Check for HASP dongle.
	if ( DongleAPI_IsValid() )
	{
		char GameName[1024];
		char Version[1024];
		char Region[1024];
		char RegionLabel[1024];
		char CabLabel[1024];
		char Cab[1024];
		long SerialNumber = 0;

		// Get attributes.
		strcpy ( GameName, DongleAPI_GetGameName() );
		strcpy ( Version, DongleAPI_GetVersion() );
		strcpy ( Region, DongleAPI_GetRegion() );
		strcpy ( RegionLabel, DongleAPI_GetRegionLabel() );
		strcpy ( CabLabel, DongleAPI_GetCabLabel() );
		strcpy ( Cab, DongleAPI_GetCab() );
		SerialNumber = DongleAPI_GetSerialNumber();

		// Check if this dongle unlocks this revision of UltraPin.
		if ( (strcmp ( GameName, "UPCOIN" ) == 0) &&
			 (strcmp ( Version, "1.0" ) == 0) &&
			 (strcmp ( RegionLabel, "CTRY" ) == 0) &&
			 (strcmp ( Region, "US" ) == 0) )
		{
			// Correct dongle.
			Status = DONGLE_STATUS_OK;
		}
		else
		{
			// Incorrect dongle.
			Status = DONGLE_STATUS_INCORRECT;
		}
	}
	else
	{
		// Dongle not found.
		Status = DONGLE_STATUS_NOTFOUND;
	}
	return ( Status );
#endif
#endif
	
	Status = DONGLE_STATUS_OK;
	return ( Status );
}


#ifdef ULTRAPIN
// Performs special draws... ok, hacks!
// These are to address shortcomings in not having
// a proper backglass display that can be animated.
void Player::DrawLightHack ()
{
	// Check the state of all lights.
	for ( int i=0; i<LIGHTHACK_MAX; i++ )
	{
		if ( g_pplayer->m_LightHackReadyForDrawLightHackFn[i] )
		{
			// Process based on the type of light.
			switch ( i )
			{
				case LIGHTHACK_FIREPOWER_P1:
				case LIGHTHACK_FIREPOWER_P2:
				case LIGHTHACK_FIREPOWER_P3:
				case LIGHTHACK_FIREPOWER_P4:
					// Check if the light is on.
					if ( m_LightHackCurrentState[i] )
					{
						// Update the blink animation.
						m_LightHackCurrentAnimState[i] = ( (msec() & 256) > 0 );

//						// Check if the state changed.
//						if ( m_LightHackPreviousState[i] != m_LightHackCurrentState[i] )
//						{
//							// Initialize the animation.
//							m_LightHackPreviousAnimState[i] = FALSE;
//							m_LightHackCurrentAnimState[i] = TRUE;
//						}

						// Check if the animation state changed.
						if ( m_LightHackPreviousAnimState[i] != m_LightHackCurrentAnimState[i] )
						{
							// Check if we are on.
							if ( m_LightHackCurrentAnimState[i] )
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
						if ( (m_LightHackCurrentAnimState[i]) ||
							 (m_LightHackPreviousAnimState[i]) )
						{
							// Hide the window.
							ShowWindow ( m_dmdhackhwnd, SW_HIDE );
							m_LightHackCurrentAnimState[i] = FALSE;
							m_LightHackPreviousAnimState[i] = FALSE;
						}
					}
					m_LightHackPreviousState[i] = m_LightHackCurrentState[i];
					break;

				case LIGHTHACK_BK2K_R:
				case LIGHTHACK_BK2K_A:
				case LIGHTHACK_BK2K_N:
				case LIGHTHACK_BK2K_S:
				case LIGHTHACK_BK2K_O:
				case LIGHTHACK_BK2K_M:
					break;
			}

			// Clear the light state.  If it's really on, it will be refreshed by Surface::put_IsDropped().
			m_LightHackCurrentState[i] = FALSE;
			m_LightHackReadyForDrawLightHackFn[i] = FALSE;
		}
		else
		{
			// Check if we got an update from IsDropped a while ago, but never updated the visual.
			// This happens when the light changes state from on to off.
			if ( (m_LightHackPreviousState[i]) &&
				 ((msec() - m_LastUpdateTime[i]) > 500) )
			{
				// Flag that it's safe to update.  We'll update on the next frame.
				m_LightHackReadyForDrawLightHackFn[i] = TRUE;
			}
		}
	}
}
#endif

