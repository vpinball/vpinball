#include "stdafx.h"
#include "..\main.h"
#include <time.h>

//#define GDIDRAW 1

//#define EVENTIME 1

#define RECOMPUTEBUTTONCHECK WM_USER+100
#define RESIZE_FROM_EXPAND WM_USER+101

LRESULT CALLBACK PlayerWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int CALLBACK PauseProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

int CALLBACK DebuggerProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

static bool initPlungerFilter = true;	// initalize filter taps
unsigned int LastPlungerHit = 0;		

Player::Player()
	{
	m_fLShiftDown = fFalse;
	m_fRShiftDown = fFalse;
//#ifdef STEPPING
	m_fPause = fFalse;
	m_fStep = fFalse;
//#endif
	m_fPseudoPause = fFalse;
	m_pauseRefCount = 0;
	m_fNoTimeCorrect = fFalse;

	m_fAccelerometer = fTrue;	// true if electronic Accelerometer enabled 
	m_AccelNormalMount = fTrue;	// fTrue normal mounting (left hand coordinates)
	m_AccelAngle = 0;			// 0 degrees (GUI is lefthand coordinates)
	m_AccelAmp = 1.5f;			// Accelerometer gain 
	m_AccelMAmp = 2.5f;			// manual input gain, generally from joysticks
	m_NudgeManual = -1;

#ifdef PLAYBACK
	m_fPlayback = fFalse;

	m_fplaylog = NULL;
#endif

	m_frotate = fFalse;

#ifdef LOG
	m_flog = NULL;
#endif

	//DWORD ver = GetVersion();

	m_sleeptime = 0;

	m_pcsimpleplayer = NULL;
	m_pxap = NULL;
	m_pactiveball = NULL;

	//m_fCheckBlt = (ver & 0x80000000) != 0; // check blt status on Win9x

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

	m_fShowFPS = fFalse;

	m_fCloseDown = fFalse;

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
	plineseg->m_rcHitRect.zlow = 0;
	plineseg->m_rcHitRect.zhigh = 100000;
	plineseg->v1.x = m_ptable->m_right;
	plineseg->v1.y = m_ptable->m_top;
	plineseg->v2.x = m_ptable->m_right;
	plineseg->v2.y = m_ptable->m_bottom;
	plineseg->CalcNormal();
	pvho->AddElement(plineseg);

	plineseg = new LineSeg();
	plineseg->m_pfe = NULL;
	plineseg->m_rcHitRect.zlow = 0;
	plineseg->m_rcHitRect.zhigh = 100000;
	plineseg->v1.x = m_ptable->m_left;
	plineseg->v1.y = m_ptable->m_bottom;
	plineseg->v2.x = m_ptable->m_left;
	plineseg->v2.y = m_ptable->m_top;
	plineseg->CalcNormal();
	pvho->AddElement(plineseg);

	plineseg = new LineSeg();
	plineseg->m_pfe = NULL;
	plineseg->m_rcHitRect.zlow = 0;
	plineseg->m_rcHitRect.zhigh = 100000;
	plineseg->v1.x = m_ptable->m_right;
	plineseg->v1.y = m_ptable->m_bottom;
	plineseg->v2.x = m_ptable->m_left;
	plineseg->v2.y = m_ptable->m_bottom;
	plineseg->CalcNormal();
	pvho->AddElement(plineseg);

	plineseg = new LineSeg();
	plineseg->m_pfe = NULL;
	plineseg->m_rcHitRect.zlow = 0;
	plineseg->m_rcHitRect.zhigh = 100000;
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

	hr = GetRegInt("Player","StartGameKey", &key);
	if (hr != S_OK || key > 0xdd)
		{
		key = DIK_1;
		}
	m_rgKeys[eStartGameKey] = (EnumAssignKeys)key;
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
	//m_SoundVolume = (int)(((log((float)m_SoundVolume)/log(10.0f))*1000) - 2000); // 10 volume = -10Db
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
	m_debugoctree.m_rectbounds.zlow = 0;
	m_debugoctree.m_rectbounds.zhigh = m_ptable->m_glassheight;

	m_debugoctree.m_vcenter.x = (m_hitoctree.m_rectbounds.left + m_hitoctree.m_rectbounds.right)/2;
	m_debugoctree.m_vcenter.y = (m_hitoctree.m_rectbounds.top + m_hitoctree.m_rectbounds.bottom)/2;
	m_debugoctree.m_vcenter.z = (m_hitoctree.m_rectbounds.zlow + m_hitoctree.m_rectbounds.zhigh)/2;

	m_debugoctree.CreateNextLevel();
	}

HRESULT Player::Init(PinTable *ptable, HWND hwndProgress, HWND hwndProgressName, BOOL fCheckForCache)
	{
	HRESULT hr;

	m_ptable = ptable;

	
	//accelerometer normal mounting is 90 degrees in left-hand coordinates (1/4 turn counterclockwise)
	m_fAccelerometer = m_ptable->m_tblAccelerometer;		// true if electronic Accelerometer enabled m_ptable->
	m_AccelNormalMount = m_ptable->m_tblAccelNormalMount;	// true is normal mounting (left hand coordinates)

	//rlc PibballWizard
	m_AccelAngle = (m_ptable->m_tblAccelAngle +90.0f) * PI/180.0f; // 0 rotated counterclockwise (GUI is lefthand coordinates)

	//rlc uShock
	//m_AccelAngle = (m_ptable->m_tblAccelAngle) * PI/180.0f; // 0 rotated counterclockwise (GUI is lefthand coordinates)
	m_AccelAmp = m_ptable->m_tblAccelAmp;					// Accelerometer gain 
	m_AccelMAmp = m_ptable->m_tblAccelManualAmp;			// manual input gain, generally from joysticks
	
	if (m_ptable->m_PhysicsType == PhysicsUltracade)
		{
		fOldPhys = false;
		}
	else
		{
		fOldPhys = true;
		}

    m_hSongCompletionEvent = CreateEvent( NULL, TRUE, FALSE, NULL );

    /*if( SUCCEEDED( hrPlay ) )
    {
        WaitForSingleObject( hCompletionEvent, INFINITE );
    }*/

	SendMessage(hwndProgress, PBM_SETPOS, 40, 0);
	// TEXT
	SetWindowText(hwndProgressName, "Initalizing Visuals...");

	InitWindow();

	InitKeys();

	InitRegValues();

	// width, height, and colordepth are only defined if fullscreen is true.
	hr = m_pin3d.InitDD(m_hwnd, m_fFullScreen, m_screenwidth, m_screenheight, m_screendepth, m_refreshrate);

	if (m_fFullScreen)
		{
		SetWindowPos(m_hwnd, NULL, 0, 0, m_screenwidth, m_screenheight, SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE);
		/*ShowWindow(m_hwnd, SW_SHOW);
		SetParent(GetParent(hwndProgress), m_hwnd);
		//::InvalidateRect(GetParent(hwndProgress), NULL, fTrue);
		RECT rcProgress;
		RECT rcMain;
		GetWindowRect(m_hwnd, &rcMain);
		GetWindowRect(GetParent(hwndProgress), &rcProgress);

		SetWindowPos(GetParent(hwndProgress), NULL,
			(rcMain.right + rcMain.left)/2 - (rcProgress.right - rcProgress.left)/2,
			(rcMain.bottom + rcMain.top)/2 - (rcProgress.bottom - rcProgress.top)/2,
			0, 0, SWP_NOOWNERZORDER | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);

		RedrawWindow(GetParent(hwndProgress), NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_ALLCHILDREN);
		*/
		}
		
	/*HDC hdcT = GetDC(m_hwnd);
	HDC hdcCompat = CreateCompatibleDC(hdcT);
	
	//SetMapMode(hdcCompat, MM_LOENGLISH);
	POINT pt;
	pt.x = 100;
	pt.y = 100;
	LPtoDP(hdcCompat, &pt, 1);
	
	int foox = GetDeviceCaps(hdcT, ASPECTX);
	int fooy = GetDeviceCaps(hdcT, ASPECTY);
	int foogoo = GetDeviceCaps(hdcT, ASPECTXY);
	
	DeleteDC(hdcCompat);
	ReleaseDC(m_hwnd, hdcT);
	
	// invert y because LOENGLISH is bottom to top, not top to bottom
	m_pixelaspectratio = ((double)pt.x)/((double)-pt.y);*/
	
	if (m_fFullScreen)
		{
		m_pixelaspectratio = ((double)m_screenwidth / (double)m_screenheight) / (4.0/3.0);
		}
	else
		{
		int screenwidth = GetSystemMetrics(SM_CXSCREEN);
		int screenheight = GetSystemMetrics(SM_CYSCREEN);
		m_pixelaspectratio = ((double)screenwidth / (double)screenheight) / (4.0/3.0);
		}
	
	if (hr != S_OK)
		{
		char szfoo[64];
		sprintf(szfoo, "Error code: %x",hr);
		ShowError(szfoo);
		return hr;
		}

	m_pininput.Init(m_hwnd);

	hr = m_pin3d.m_pd3dDevice->BeginScene();

	float realFOV = ptable->m_FOV;

	if (realFOV <= 0)
		{
		realFOV = 0.01f; // Can't have a real zero FOV, but this will look the same
		}

	m_pin3d.InitLayout(ptable->m_left, ptable->m_top, ptable->m_right,
		ptable->m_bottom, ptable->m_inclination, realFOV);

	m_mainlevel.m = 0;
	m_mainlevel.n = 0;
	m_mainlevel.b = 0;
	m_mainlevel.m_gravity.x = 0.00f;
	// Table tilted at 6 degrees
	m_mainlevel.m_gravity.y = (float)(sin(ANGTORAD(ptable->m_angletilt))*GRAVITY); //0.06f;
	m_gravityz = (float)(sin(ANGTORAD(90 - (ptable->m_angletilt)))*GRAVITY);
	//m_gravityy = (float)(cos(ANGTORAD(90 - (ptable->m_angletilt)))*GRAVITY);

	m_NudgeX = 0;
	m_NudgeY = 0;
	m_nudgetime = 0;
	initPlungerFilter = true;	// initial plunger filter


	SendMessage(hwndProgress, PBM_SETPOS, 50, 0);
	// TEXT
	SetWindowText(hwndProgressName, "Initalizing Physics...");

	// Need to set timecur here, for init functions that set timers
	m_timeCur = 0;

#ifdef FPS
	m_lastfpstime = m_timeCur;
	m_cframes = 0;
	m_fps = 0;
#endif

	int i;
	for (i=0;i<m_ptable->m_vedit.Size();i++)
		{
		Hitable *ph = m_ptable->m_vedit.ElementAt(i)->GetIHitable();
		if (ph)
			{
			int currentsize = m_vho.Size();
			ph->GetHitShapes(&m_vho);
			int newsize = m_vho.Size();
			int hitloop;
			// Save the objects the trouble of having the set the idispatch pointer themselves
			for (hitloop = currentsize;hitloop < newsize;hitloop++)
				{
				m_vho.ElementAt(hitloop)->m_pfedebug = m_ptable->m_vedit.ElementAt(i)->GetIFireEvents();
				}
			ph->GetTimers(&m_vht);
			}
		}

	CreateBoundingHitShapes(&m_vho);

	for (i=0;i<m_vho.Size();i++)
		{
		m_vho.ElementAt(i)->CalcHitRect();

		m_hitoctree.m_vho.AddElement(m_vho.ElementAt(i));

		if (m_vho.ElementAt(i)->GetType() == e3DPoly && ((Hit3DPoly *)m_vho.ElementAt(i))->m_fVisible)
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
	m_hitoctree.m_rectbounds.zlow = 0;
	m_hitoctree.m_rectbounds.zhigh = m_ptable->m_glassheight;

	m_hitoctree.m_vcenter.x = (m_hitoctree.m_rectbounds.left + m_hitoctree.m_rectbounds.right)/2;
	m_hitoctree.m_vcenter.y = (m_hitoctree.m_rectbounds.top + m_hitoctree.m_rectbounds.bottom)/2;
	m_hitoctree.m_vcenter.z = (m_hitoctree.m_rectbounds.zlow + m_hitoctree.m_rectbounds.zhigh)/2;

	m_hitoctree.CreateNextLevel();

	m_shadowoctree.m_rectbounds.left = m_ptable->m_left;
	m_shadowoctree.m_rectbounds.right = m_ptable->m_right;
	m_shadowoctree.m_rectbounds.top = m_ptable->m_top;
	m_shadowoctree.m_rectbounds.bottom = m_ptable->m_bottom;
	m_shadowoctree.m_rectbounds.zlow = 0;
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
	else if (m_pin3d.m_fWritingToCache)
		{
		SetWindowText(hwndProgressName, "Writing Table To Cache...");
		}
	else
		{
		SetWindowText(hwndProgressName, "Rendering Table...");
		}

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

	InitAnimations(hwndProgress);
	
	// End Cache
	m_pin3d.CloseCacheFile();

	///////////////// Screen Update Vector
	///// (List of movers which can be blitted at any time)
	/////////////////////////

	for (i=0;i<m_vho.Size();i++)
		{
		if (m_vho.ElementAt(i)->GetAnimObject() != NULL)
			{
			int l;

			// Put the screenupdate vector in sorted order back to
			// front so that invalidated objects draw over each-other
			// correctly

			AnimObject *pao = m_vho.ElementAt(i)->GetAnimObject();

			float myz = (pao->m_znear + pao->m_zfar) / 2;

			/*Vertex3D camerapos;
			camerapos.x = m_pin3d.m_vertexcamera.x;
			camerapos.y = m_pin3d.m_vertexcamera.z;
			camerapos.z = m_pin3d.m_vertexcamera.y;

			Vertex3D myv;
			myv.x = (m_vho.ElementAt(i)->m_rcHitRect.left + m_vho.ElementAt(i)->m_rcHitRect.right)/2;
			myv.y = (m_vho.ElementAt(i)->m_rcHitRect.top + m_vho.ElementAt(i)->m_rcHitRect.bottom)/2;
			myv.z = (m_vho.ElementAt(i)->m_rcHitRect.zlow + m_vho.ElementAt(i)->m_rcHitRect.zhigh)/2;

			float mydis = camerapos.DistanceSquared(&myv);*/

			for (l=0;l<m_vscreenupdate.Size();l++)
				{
				BOOL fInBack = fFalse;

				/*if (m_vho.ElementAt(i)->m_rcHitRect.zhigh < m_vscreenupdate.ElementAt(l)->m_rcHitRect.zlow)
					{
					fInBack = fTrue;
					}
				else if (m_vho.ElementAt(i)->m_rcHitRect.bottom < m_vscreenupdate.ElementAt(l)->m_rcHitRect.top)
					{
					fInBack = fTrue;
					}
				else if (m_vscreenupdate.ElementAt(l)->m_rcHitRect.zhigh < m_vho.ElementAt(i)->m_rcHitRect.zlow)
					{
					fInBack = fFalse;
					}
				else if (m_vscreenupdate.ElementAt(l)->m_rcHitRect.bottom < m_vho.ElementAt(i)->m_rcHitRect.top)
					{
					fInBack = fFalse;
					}
				else*/
					{
					/*Vertex3D comparev;
					comparev.x = (m_vscreenupdate.ElementAt(l)->m_rcHitRect.left + m_vscreenupdate.ElementAt(l)->m_rcHitRect.right)/2;
					comparev.y = (m_vscreenupdate.ElementAt(l)->m_rcHitRect.top + m_vscreenupdate.ElementAt(l)->m_rcHitRect.bottom)/2;
					comparev.z = (m_vscreenupdate.ElementAt(l)->m_rcHitRect.zlow + m_vscreenupdate.ElementAt(l)->m_rcHitRect.zhigh)/2;

					float comparedis = camerapos.DistanceSquared(&comparev);*/

					float comparez = (m_vscreenupdate.ElementAt(l)->m_znear + m_vscreenupdate.ElementAt(l)->m_zfar)/2;

					if (myz > comparez)
						{
						fInBack = fTrue;
						}
					}

				/*float comparez = (m_vscreenupdate.ElementAt(l)->m_znear + m_vscreenupdate.ElementAt(l)->m_zfar)/2;
				if (myz > comparez)
					{
					//m_vscreenupdate.InsertElementAt(m_vho.ElementAt(i), l);
					break;
					}*/
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

	QueryPerformanceFrequency (&m_liCounterFrequency);

	if (m_liCounterFrequency.QuadPart == 0)
		{
		// TEXT
		ShowError("High Performance Counter not installed.");
		}

	QueryPerformanceCounter(&m_liStartTime);

	m_liPhysicsStep.QuadPart = m_liCounterFrequency.QuadPart / 100; // update gravity 100 times a second
	m_liPhysicsNext.QuadPart = m_liStartTime.QuadPart + m_liPhysicsStep.QuadPart; // time at which the next physics update should be
	m_liPhysicsCalced = m_liStartTime; // haven't calced any physics yet

#ifdef PLAYBACK
	if (m_fPlayback)
		{
		ParseLog(&m_liPhysicsStep, &m_liStartTime);
		}
#endif

	m_liOldTime = m_liStartTime;

	//m_physicsdtime = /*(float)m_liPhysicsStep.LowPart / (float)m_liCounterFrequency.LowPart*/;
	m_physicsdtime = 1;

#ifdef LOG
		fprintf(m_flog, "Step Time %u %u %u %u\n", m_liPhysicsStep.HighPart, m_liPhysicsStep.LowPart, m_liStartTime.HighPart, m_liStartTime.LowPart);
		fprintf(m_flog, "End Frame\n");
#endif

	SendMessage(hwndProgress, PBM_SETPOS, 100, 0);
	// TEXT
	SetWindowText(hwndProgressName, "Starting...");

	ShowWindow(m_hwnd, SW_SHOW);

	// Call Init
	for (i=0;i<m_ptable->m_vedit.Size();i++)
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

	//Ball *pball = CreateBall(31.287897f, 656.972542f, 0, -19.638394f, -7.920756f, 0);

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

	hr = m_pin3d.m_pd3dDevice->BeginScene();

	m_pin3d.SetRenderTarget(m_pin3d.m_pddsStatic, m_pin3d.m_pddsStaticZ);

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

	// Draw kickers (they chnage z-buffer, so they have to be drawn after the wall they are on)
	for (i=0;i<m_ptable->m_vedit.Size();i++)
		{
		if (m_ptable->m_vedit.ElementAt(i)->GetItemType() == eItemKicker)
			{
			Hitable *ph = m_ptable->m_vedit.ElementAt(i)->GetIHitable();
			if (ph)
				{
				ph->RenderStatic(m_pin3d.m_pd3dDevice);
				//SendMessage(hwndProgress, PBM_SETPOS, 75 + ((5*i)/m_ptable->m_vedit.Size()), 0);
				}
			}
		}

	hr = m_pin3d.m_pd3dDevice->EndScene();
	
	m_pin3d.WriteSurfaceToCacheFile(m_pin3d.m_pddsStatic);
	m_pin3d.WriteSurfaceToCacheFile(m_pin3d.m_pddsStaticZ);
	}

void Player::InitAnimations(HWND hwndProgress)
	{
	HRESULT hr;
	int i;

	hr = m_pin3d.m_pd3dDevice->BeginScene();

	//DrawTest();

	m_pin3d.SetRenderTarget(m_pin3d.m_pddsBackBuffer, m_pin3d.m_pddsZBuffer);

	// Set up z-buffer to the static one, so movers can clip to it
	m_pin3d.m_pddsZBuffer->Blt(NULL, m_pin3d.m_pddsStaticZ, NULL, DDBLTFAST_WAIT, NULL);
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

	//m_pin3d.m_pd3dDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE);

	hr = m_pin3d.m_pd3dDevice->EndScene();

	// Set up backbuffer with static board
	m_pin3d.m_pddsBackBuffer->Blt(NULL, m_pin3d.m_pddsStatic, NULL, DDBLTFAST_WAIT , NULL);
	m_pin3d.m_pddsZBuffer->Blt(NULL, m_pin3d.m_pddsStaticZ, NULL, DDBLTFAST_WAIT , NULL);
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

	/*if (z == 0)
		{
		pball->m_plevel = &m_mainlevel;
		}
	else
		{
		pball->m_plevel = NULL;
		}*/

	m_vball.AddElement(pball);
	//m_vho.AddElement(pball);
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
	//RECT rc;
	//RECT rcShadow;

	/*m_pin3d.ClearExtents(&rc, NULL, NULL);
	m_pin3d.ExpandExtents(&rc, pball->m_rgv3D, NULL, NULL, 4, fFalse);*/

	if (m_fBallShadows)
		{
		/*m_pin3d.ClearExtents(&rcShadow, NULL, NULL);
		m_pin3d.ExpandExtents(&rcShadow, pball->m_rgv3DShadow, NULL, NULL, 4, fFalse);*/

		if (fIntRectIntersect(pball->m_rcScreen, pball->m_rcScreenShadow))
			{
			// Rect already merged
			/*rc.left = min(rc.left, rcShadow.left);
			rc.top = min(rc.top, rcShadow.top);
			rc.right = max(rc.right, rcShadow.right);
			rc.bottom = max(rc.bottom, rcShadow.bottom);*/
			InvalidateRect(&pball->m_rcScreen);
			}
		else
			{
			InvalidateRect(&pball->m_rcScreen);
			InvalidateRect(&pball->m_rcScreenShadow);
			}
		}
	else
		{
		InvalidateRect(&pball->m_rcScreen);
		}
	}

void Player::DestroyBall(Ball *pball)
	{
	if (pball->m_fErase) // Need to clear the ball off the playfield
		{
		EraseBall(pball);
		/*RECT rc;

		m_pin3d.ClearExtents(&rc, NULL, NULL);
		m_pin3d.ExpandExtents(&rc, pball->m_rgv3D, NULL, NULL, 4, fFalse);

		InvalidateRect(&rc);*/
		}

	if (pball->m_pballex)
		{
		pball->m_pballex->m_pball = NULL;
		pball->m_pballex->Release();
		}

	m_vball.RemoveElement(pball);
	//m_vho.RemoveElement(pball);
	m_vmover.RemoveElement(&pball->m_ballanim);

	m_hitoctree.m_vho.RemoveElement(pball);
	//delete pball;
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
			m_screendepth = 16; // The default
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

	//m_width = 900;
	//m_height= 675;

	//RECT rcWorkArea;

	int x,y;

	//SystemParametersInfo(SPI_GETWORKAREA, 0, &rcWorkArea, 0);

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

	x = (screenwidth/*(rcWorkArea.right - rcWorkArea.left)*/ - m_width) / 2;
	y = (screenheight/*(rcWorkArea.bottom - rcWorkArea.top)*/ - m_height) / 2;

	int windowflags = WS_POPUP;
	int windowflagsex = 0;

	int captionheight = GetSystemMetrics(SM_CYCAPTION);

	if (!m_fFullScreen & (screenheight - m_height >= (captionheight*2))) // We have enough room for a frame
		{
		windowflags = WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
		windowflagsex = WS_EX_OVERLAPPEDWINDOW;
		y -= captionheight;
		m_height += captionheight;
		}

	windowflags |= WS_CLIPCHILDREN;

	// if window is larger than screen
	//x = max(x,0);
	//y = max(y,0);
	//m_width = min(m_width, rcWorkArea.right - rcWorkArea.left);
	//m_height = min(m_height, rcWorkArea.bottom - rcWorkArea.top);

	// TEXT
	m_hwnd = ::CreateWindowEx(windowflagsex,"VPPlayer","Visual Pinball Player",
		windowflags,
		x,y,m_width,m_height,NULL,NULL,g_hinst,0);

#ifdef GDIDRAW
	HDC hdc = GetDC(NULL);

	m_hbmOffScreen = CreateCompatibleBitmap(hdc, m_width, m_height);

	ReleaseDC(NULL, hdc);
#endif

	//m_timerid = ::SetTimer(m_hwnd, 11, 1, NULL); // Set up redraw
	//PostMessage(m_hwnd, WM_USER+1000, 0, 0);
}

/////////////////////////////////////////////////////////////////////////////////////////

extern int e_JoyCnt;
extern int m_PinBallDevice;						// only one accelerometer device
static int curAccel_x[PININ_JOYMXCNT]= {0,0,0,0};
static int curAccel_y[PININ_JOYMXCNT]= {0,0,0,0};

static float curMechPlungerPos;

int curPlunger = JOYRANGEMN-1;	// assume


void Player::m_PinBallNudgeX(int x, int j )
{
	curAccel_x[j] = x;
}


void Player::m_PinBallNudgeY(int y, int j )
{
	curAccel_y[j] = y;
}


int Player::m_PinBallNudgeGetTilt( void )
{
	return 0;//TBI
}

void Player::m_PinBallNudge(void)	// called on every integral physics frame
{
	static float cna=1,sna=0,na=0;	//rlc initialize for angle 0
	float dx, dy;

	if (m_NudgeManual >= 0)			// Only one joystick controls in manual mode
	{
		m_NudgeX =  m_AccelMAmp * ((float)curAccel_x[m_NudgeManual])/((float)JOYRANGE); // * Manual Gain
		m_NudgeY =  m_AccelMAmp * ((float)curAccel_y[m_NudgeManual])/((float)JOYRANGE);
		return;
	}	

	m_NudgeX = 0;	// accumlate over joysticks, these acceleration values are used in update ball velocity calculations
	m_NudgeY = 0;	// and are required to be acceleration values (not velocity or displacement)

	if(!m_fAccelerometer) return;	// accelerometer is disabled 

	//rotate to match hardware mounting orientation, including left or right coordinates
	// Cache the sin and cos results whenever the angle changes
	if( na != m_AccelAngle )
	{
		na = m_AccelAngle ;
		cna = cos(na);
		sna = sin(na);
	}

	int j = m_PinBallDevice;

	dx =  ((float)curAccel_x[j])/((float)JOYRANGE);							// norm range -1 .. 1	
	dy =  ((float)curAccel_y[j])/((float)JOYRANGE);	

	m_NudgeX += m_AccelAmp*(dx*cna + dy*sna);	//calc Green's transform component for X
	float nugY = m_AccelAmp*(dy*cna - dx*sna); // calc Green transform component for Y...
	m_NudgeY = m_AccelNormalMount ? (m_NudgeY + nugY): (m_NudgeY - nugY);	// add as left or right hand coordinate system

#if 0
	for(int j = 0; j < e_JoyCnt; ++j)
	{
		dx =  ((float)curAccel_x[j])/((float)JOYRANGE);							// norm range -1 .. 1	
		dy =  ((float)curAccel_y[j])/((float)JOYRANGE);	

		m_NudgeX += m_AccelAmp*(dx*cna + dy*sna);	//calc Green's transform component for X
		float nugY = m_AccelAmp*(dy*cna - dx*sna); // calc Green transform component for Y...
		m_NudgeY = m_AccelNormalMount ? (m_NudgeY + nugY): (m_NudgeY - nugY);	// add as left or right hand coordinate system
	}
#endif
}

void Player::m_PinBallPlunger(void)	// called on every integral physics frame
{	
#define IIR_Order 4
	static float x [IIR_Order+1] = {0,0,0,0,0};
	static float y [IIR_Order+1] = {0,0,0,0,0};	

	// coefficients for IIR_Order Butterworth filter set to 10 Hz passband
	static const float a [IIR_Order+1] = {	
		0.0048243445f,
		0.019297378f,	
		0.028946068f,
		0.019297378f,
		0.0048243445f};

	static const float b [IIR_Order+1] = {	
		1.00000000f, //if not 1 add division below
		-2.369513f,
		2.3139884f,
		-1.0546654f,
		0.1873795f};

	//http://www.dsptutor.freeuk.com/IIRFilterDesign/IIRFilterDesign.html  
	// (this applet is set to 8000Hz sample rate, therefore, multiply ...
	// our values by 80 to shift sample clock of 100hz to 8000hz)

	curMechPlungerPos = (float)curPlunger;

	//rlc@@@@@@@ disable for now 
	//if (!c_plungerFilter) return;

	if (initPlungerFilter) 
	{
		initPlungerFilter = false;
		for (int i = IIR_Order; i >= 0 ;--i) // all terms but the zero-th 
		{ 
			x[i] = y[i] = curMechPlungerPos;
		}
	}	

	x[0] = curMechPlungerPos; //initialize filter

	y[0] = a[0]*x[0];		  // initial

	for (int i = IIR_Order; i > 0 ;--i) // all terms but the zero-th 
	{ 
		y[0] += (a[i]*x[i] - b[i]*y[i]);// /b[0]; always one     // sum terms from high to low
		x[i] = x[i-1];		//shift 
		y[i] = y[i-1];		//shift
	}			
	curMechPlungerPos = y[0];
}


// mechPlunger NOTE: Normalized position is from 0.0 to +1.0f
// +1.0 is fully retracted, 0.0 is fully compressed
// method requires calibration in control panel game controllers to work right
// calibrated zero value should match the rest position of the mechanical plunger
// the method below uses a dual - piecewise linear function to map the mechanical pull and push 
// onto the virtual plunger position from 0..1, the plunger properties has a ParkPosition setting 
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
	curPlunger = z; //axis reversal
}

////////////////////////////////////////////////////////////////////////

void Player::Render()
{
	int iball;
	int i;
	Ball *pball;
	int cball = m_vball.Size();

	//_controlfp(_PC_24, MCW_PC);

	// Don't calculate the next frame if the last one isn't done blitting yet
	// On Win95 when there are no balls, frame updates happen so fast the
	// blitter gets stuck

	if ((cball == 0) && m_fWasteTime2)
		{
		Sleep(1);
		}

	if (m_sleeptime > 0)
		{
		Sleep(m_sleeptime - 1);
		}

	//Sleep(50);

	if (m_fCheckBlt)
		{
		HRESULT hrdone = m_pin3d.m_pddsFrontBuffer->GetBltStatus(DDGBS_ISBLTDONE);

		if (hrdone != DD_OK)
			{
			if (m_fWasteTime)
				{
				Sleep(1);
				}
			return;
			}
		}

	//LARGE_INTEGER liFreq;
	//QueryPerformanceFrequency (&liFreq);

	LARGE_INTEGER liNewTime;
	QueryPerformanceCounter(&liNewTime);

	if (m_fNoTimeCorrect) // After debugging script
		{
		// Shift whole game foward in time
		m_liStartTime.QuadPart += (liNewTime.QuadPart - m_liOldTime.QuadPart);
		m_liPhysicsNext.QuadPart += (liNewTime.QuadPart - m_liOldTime.QuadPart);
		m_liOldTime = liNewTime; // 0 time frame
		m_fNoTimeCorrect = fFalse;
		}

#ifdef STEPPING
#ifndef EVENTIME
	if (m_fDebugWindowActive || m_fUserDebugPaused)
		{
		// Shift whole game foward in time
		m_liStartTime.QuadPart += (liNewTime.QuadPart - m_liOldTime.QuadPart);
		m_liPhysicsNext.QuadPart += (liNewTime.QuadPart - m_liOldTime.QuadPart);
		if (m_fStep)
			{
			// Walk one physics step foward
			m_liOldTime.QuadPart = liNewTime.QuadPart - m_liPhysicsStep.QuadPart;
			m_fStep = fFalse;
			}
		else
			{
			m_liOldTime = liNewTime; // 0 time frame
			}
		}
#endif
#endif

#ifdef EVENTIME
	if (!m_fPause || m_fStep)
		{
		liNewTime.QuadPart = m_liOldTime.QuadPart - 3547811060 + 3551825450;//(m_liPhysicsStep.QuadPart*3/4);
		m_fStep = fFalse;
		}
	else
		{
		liNewTime.QuadPart = m_liOldTime.QuadPart;
		}
#endif

	float timepassed = (float)(liNewTime.LowPart - m_liOldTime.LowPart) / (float)m_liCounterFrequency.LowPart;

	// Get time in milliseconds for timers
	m_timeCur = (int)(((liNewTime.QuadPart - m_liStartTime.QuadPart)*1000)/m_liCounterFrequency.LowPart);

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

#define TIMECORRECT 1

#ifdef PLAYBACK
	if (!m_fPlayback)
		{
		frametime = timepassed * 100;
		}
	else
		{
		float temp = ParseLog(&liNewTime, &m_liPhysicsNext);
		if (m_fPlayback)
			{
			frametime = temp;
			}
		}
#else // PLAYBACK

#ifdef TIMECORRECT
	frametime = timepassed * 100;
	//frametime = 1.456927f;
#else // TIMECORRECT
	frametime = 0.45f;
#endif // TIMECORRECT

#endif //PLACKBACK

#ifdef LOG
		fprintf(m_flog, "Frame Time %.20f %u %u %u %u\n", frametime, liNewTime.HighPart, liNewTime.LowPart, m_liPhysicsNext.HighPart, m_liPhysicsNext.LowPart);
		fprintf(m_flog, "End Frame\n");
#endif

	for (iball=0;iball<cball;iball++)
		{
		pball = m_vball.ElementAt(iball);
		//pball->CalcBoundingRect();

		if (pball->m_fErase) // Need to clear the ball off the playfield
			{
			/*RECT rc;

			m_pin3d.ClearExtents(&rc, NULL, NULL);
			m_pin3d.ExpandExtents(&rc, pball->m_rgv3D, NULL, NULL, 4, fFalse);

			InvalidateRect(&rc);*/

			EraseBall(pball);
			}
		}

	//pball = rgpsynclist[0];
	//float remainingtime;
	PINFLOAT hittime;

	// Calculate physics in gravity segments

	Ball *phitball;

	while (m_liOldTime.QuadPart < liNewTime.QuadPart)
		{
		// Get the time until the next physics tick is done, and get the time
		// Unitl the next frame is done (newtime)
		// If the frame is the next thing to happen, update physics to that
		// point or beyond, don't update acceleration, and exit loop
		// If the physics tick is next, update physics to that point,
		// update acceleration, and continue loop
		PINFLOAT remainingtime = (PINFLOAT)(m_liPhysicsNext.QuadPart - m_liOldTime.QuadPart)/(PINFLOAT)m_liCounterFrequency.LowPart;
		PINFLOAT endframetime = (PINFLOAT)(liNewTime.QuadPart - m_liOldTime.QuadPart) / (PINFLOAT)m_liCounterFrequency.LowPart;
		BOOL fTick = fFalse; // This pass will end at a physics tick instead of a frame
		remainingtime *= 100;
		endframetime *= 100;

		if (endframetime > remainingtime)
			{
			// Go until next physics step
			endframetime = remainingtime;
			fTick = fTrue;
			}
			
		int zeroCount = 0; // count of how many times a ball has collided without advancing the clock

		while (zeroCount < 100 && ((hittime = UpdatePhysics(remainingtime, &phitball)) < endframetime))
			{
			if (hittime == 0)
				{
				zeroCount++;
				}

			_ASSERTE(phitball);

			for (i=0;i<m_vmover.Size();i++)
				{
				m_vmover.ElementAt(i)->UpdateTimeTemp(hittime);
				}

			// Perform ball hit
			//phitball->x = phitball->m_hitx;
			//phitball->y = phitball->m_hity;
			// Don't call anything on this ball after Collide - script could delete it
			HitObject *pho = phitball->m_pho;
			phitball->phoHitLast = phitball->m_pho;
			phitball->m_fCalced = fFalse;
			m_pactiveball = phitball; // For script that wants the ball during the collision
			pho->Collide(phitball, phitball->m_hitnormal);

			// Move each ball's hit time forward by the time elapsed
			for (i=0;i<m_vball.Size();i++)
				{
				if (m_vball.ElementAt(i)->m_fCalced)
					{
					m_vball.ElementAt(i)->m_hittime -= hittime;
					}
				// BUG - once we stale out the ball instead of deleting it (so it can be used in script) then we can just check the stale flag on the hit ball
				if (m_vball.ElementAt(i) == phitball) // Ball still exists
					{
					phitball->CalcBoundingRect(); // Collide may have changed the velocity of the ball, and therefore the bounding box for the next hit frame
					}

				/*if (m_vball.ElementAt(i) != phitball)
					{
					m_vball.ElementAt(i)->phoHitLast = NULL;
					}*/
				}

			remainingtime -= hittime;
			endframetime -= hittime;
			}

		if (fTick) // Completed a physics tick
			{
			for (i=0;i<m_vmover.Size();i++)
				{
				m_vmover.ElementAt(i)->UpdateTimeTemp(hittime);
				}

			///////////////////////////////////////////////////

			m_PinBallNudge();   // update accelerometer effects

			m_PinBallPlunger();

			///////////////////////////////////////////////////
			if (m_nudgetime)
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
				m_vmover.ElementAt(i)->UpdateAcceleration(m_physicsdtime);
				}

			m_NudgeX = 0;
			m_NudgeY = 0;

			for (i=0;i<m_vball.Size();i++)
				{
				m_vball.ElementAt(i)->m_fCalced = NULL;
				}
			}
		else
			{
			for (i=0;i<m_vmover.Size();i++)
				{
				m_vmover.ElementAt(i)->UpdateTimeTemp(endframetime);
				}

			for (i=0;i<m_vball.Size();i++)
				{
				if (m_vball.ElementAt(i)->m_fCalced)
					{
					// Move each ball's hit time forward by the time elapsed
					// at the end of the frame
					m_vball.ElementAt(i)->m_hittime -= endframetime;
					}
				}
			}

		for (i=0;i<m_vball.Size();i++)
			{
			m_vball.ElementAt(i)->phoHitLast = NULL;
			}

		//frametime -= m_physicsdtime;

		if (fTick)
			{
			m_liOldTime = m_liPhysicsNext;
			m_liPhysicsNext.QuadPart += m_liPhysicsStep.QuadPart;
			}
		else
			{
			m_liOldTime = liNewTime;
			}
		}

	m_LastKnownGoodCounter++;

#ifdef GDIDRAW

	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(m_hwnd, &ps);

	HBITMAP hbmOld;

	HDC hdcT = CreateCompatibleDC(hdc);

	hbmOld = (HBITMAP)SelectObject(hdcT, m_hbmOffScreen);

	SelectObject(hdcT, GetStockObject(WHITE_BRUSH));

	PatBlt(hdcT, 0, 0, m_width, m_height, PATCOPY);

	for (i=0;i<m_vho.Size();i++)
		{
		m_vho.ElementAt(i)->Draw(hdcT);
		}

	for (i=0;i<m_vball.Size();i++)
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

	//m_pin3d.m_pddsBackBuffer->Blt(NULL, m_pin3d.m_pddsStatic, NULL, 0, NULL);

	for (i=0;i<m_vscreenupdate.Size();i++)
		{
		m_vscreenupdate.ElementAt(i)->m_fInvalid = fFalse;
		m_vscreenupdate.ElementAt(i)->Check3D();
		if (m_vscreenupdate.ElementAt(i)->m_fInvalid)
			{
			InvalidateRect(&m_vscreenupdate.ElementAt(i)->m_rcBounds);
			}
		}

	for (i=0;i<m_vupdaterect.Size();i++)
		{
		UpdateRect *pur = m_vupdaterect.ElementAt(i);
		if (pur->m_fSeeThrough)
			{
			RECT *prc = &pur->m_rcupdate;
			m_pin3d.m_pddsBackBuffer->Blt(prc, m_pin3d.m_pddsStatic, prc, DDBLTFAST_WAIT, NULL);
			m_pin3d.m_pddsZBuffer->Blt(prc, m_pin3d.m_pddsStaticZ, prc, DDBLTFAST_WAIT, NULL);
			}
		}

	for (i=0;i<m_vupdaterect.Size();i++)
		{
		UpdateRect *pur = m_vupdaterect.ElementAt(i);
		int l;
		for (l=0;l<pur->m_vobject.Size();l++)
			{
			ObjFrame *pobjframe = pur->m_vobject.ElementAt(l)->Draw3D(&pur->m_rcupdate);

			if (pobjframe)
				{
				LPDIRECTDRAWSURFACE7 pdds = g_pplayer->m_pin3d.m_pddsBackBuffer;
				RECT rcUpdate;
				RECT *prc = &pur->m_rcupdate;

				rcUpdate.left = max(pobjframe->rc.left, prc->left) - pobjframe->rc.left;
				rcUpdate.top = max(pobjframe->rc.top, prc->top) - pobjframe->rc.top;
				rcUpdate.right = min(pobjframe->rc.right, prc->right) - pobjframe->rc.left;
				rcUpdate.bottom = min(pobjframe->rc.bottom, prc->bottom) - pobjframe->rc.top;

				int bltleft, blttop;
				bltleft = max(pobjframe->rc.left, prc->left);
				blttop = max(pobjframe->rc.top, prc->top);

				if (rcUpdate.right > rcUpdate.left && rcUpdate.bottom > rcUpdate.top)
					{
					HRESULT hr = pdds->BltFast(bltleft/*pobjframe->rc.left*/, blttop/*pobjframe->rc.top*/, pobjframe->pdds,
						&rcUpdate/*NULL*/, DDBLTFAST_SRCCOLORKEY);
					g_pplayer->m_pin3d.m_pddsZBuffer->BltFast(bltleft/*pobjframe->rc.left*/, blttop/*pobjframe->rc.top*/,
						pobjframe->pddsZBuffer, &rcUpdate/*NULL*/, DDBLTFAST_NOCOLORKEY);
					}
				}
			}
		//delete pur;
		}
	//m_vupdaterect.RemoveAllElements();

	/*for (i=0;i<m_vscreenupdate.Size();i++)
		{
		if (m_vscreenupdate.ElementAt(i)->m_fInvalid)
			{
			m_vscreenupdate.ElementAt(i)->Draw3D();
			int l;
			for (l=0;l<m_vscreenupdate.ElementAt(i)->m_vrcupdate.Size();l++)
				{
				delete m_vscreenupdate.ElementAt(i)->m_vrcupdate.ElementAt(l);
				}
			m_vscreenupdate.ElementAt(i)->m_vrcupdate.RemoveAllElements();
			m_vscreenupdate.ElementAt(i)->m_fInvalid = fFalse;
			}
		}*/

	HRESULT hr = m_pin3d.m_pd3dDevice->BeginScene();

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
		
	if (m_fDebugMode)
		{
		DrawBallsDebug();
		}

	hr = m_pin3d.m_pd3dDevice->EndScene();

	if (m_nudgetime) // Nudging - can't do a smart blit of the updated areas
		{
		m_pin3d.Flip((int)m_NudgeBackX, (int)m_NudgeBackY);
		m_fCleanBlt = fFalse;
		}
	else if (m_fCleanBlt)
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

			m_pin3d.m_pddsFrontBuffer->Blt(&rcNew, m_pin3d.m_pddsBackBuffer, prc, DDBLTFAST_WAIT, NULL);
			}
		//m_fCleanBlt = fTrue;
		}
	else
		{
		m_pin3d.Flip(0, 0);
		m_fCleanBlt = fTrue;
		}

	for (i=0;i<m_vupdaterect.Size();i++)
		{
		UpdateRect *pur = m_vupdaterect.ElementAt(i);
		delete pur;
		}
	m_vupdaterect.RemoveAllElements();

#endif //GDIDRAW

	m_pactiveball = NULL;  // No ball is the active ball for timers/key events

	for (i=0;i<m_vht.Size();i++)
		{
		HitTimer *pht = m_vht.ElementAt(i);
		if (pht->m_nextfire <= m_timeCur)
			{
			pht->m_pfe->FireGroupEvent(DISPID_TimerEvents_Timer);
			//pht->m_pep->FireGroupEvent(DISPID_TimerEvents_Timer);
			pht->m_nextfire += pht->m_interval;
			}
		}

	// Notice - the light can only update once per frame, so if the light
	// is blinking faster than the frame rate, the user will still see
	// the light blinking, it will just be slower than intended.
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

	m_pininput.ProcessKeys(m_ptable);

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
		delete m_vballDelete.ElementAt(i);
		}
	m_vballDelete.RemoveAllElements();

#ifdef FPS
	if (m_fShowFPS)
		{
		HDC hdcNull = GetDC(NULL);
		char szFoo[64];
		int len = sprintf(szFoo, "%d %x %d", m_fps, _controlfp(_PC_53, 0), !fOldPhys);
		TextOut(hdcNull, 10, 10, szFoo, len);
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
	else if (m_fCloseDown)
		{
		PauseMusic();
		
		int option;

		if (g_pvp->m_fPlayOnly)
			{
			option = DialogBox(g_hinstres, MAKEINTRESOURCE(IDD_GAMEPAUSENOEDIT), m_hwnd, PauseProc);
			}
		else
			{
			option = DialogBox(g_hinstres, MAKEINTRESOURCE(IDD_GAMEPAUSE), m_hwnd, PauseProc);
			}

		m_fCloseDown = fFalse;
		g_pplayer->m_fNoTimeCorrect = fTrue; // Skip the time we were in the dialog
		if (option == ID_QUIT)
			{
			SendMessage(m_hwnd, WM_CLOSE, 0, 0);
			}
		else
			{
			UnpauseMusic();
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

#ifdef DRAWTEST

void Player::DrawTest()
	{
	int i;
	Vertex3D rgv3D[4];

	D3DMATERIAL7 mtrl;
	ZeroMemory( &mtrl, sizeof(mtrl) );
	mtrl.diffuse.r = mtrl.ambient.r = 1;
	mtrl.diffuse.g = mtrl.ambient.g = 1;
	mtrl.diffuse.b = mtrl.ambient.b = (m_timeCur & 1000) / 1000.0f;

	m_pin3d.m_pd3dDevice->SetMaterial(&mtrl);

	WORD rgi[4];

	for (i=0;i<4;i++)
		{
		rgi[i] = (i+2) & 3;
		}

	m_pin3d.m_pd3dDevice->SetTexture(0, m_pin3d.m_pddsBallTexture);

	m_pin3d.m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ADDRESS, D3DTADDRESS_CLAMP);
	m_pin3d.m_pd3dDevice->SetRenderState(D3DRENDERSTATE_COLORKEYENABLE, TRUE);

	float sn = (float)sin(m_pin3d.m_inclination);
	float cs = (float)cos(m_pin3d.m_inclination);

	float x = 770;
	float y = 1430;
	float radius = 25;
	float zheight = radius;

	rgv3D[0].x = x - radius;
	rgv3D[0].y = y - radius; //y - (radius * cs);
	rgv3D[0].z = 0.1f;//zheight + (radius * sn);
	rgv3D[0].tu = 0;
	rgv3D[0].tv = 0;
	rgv3D[0].nx = 0;
	rgv3D[0].ny = 0;
	rgv3D[0].nz = 1;

	rgv3D[3].x = x - radius;
	rgv3D[3].y = y + radius; //y + (radius * cs);
	rgv3D[3].z = 0.1f;//zheight - (radius * sn);
	rgv3D[3].tu = 0;
	rgv3D[3].tv = 1;
	rgv3D[3].nx = 0;
	rgv3D[3].ny = 0;
	rgv3D[3].nz = 1;

	rgv3D[2].x = x + radius;
	rgv3D[2].y = y + radius; //y + (radius * cs);
	rgv3D[2].z = 0.1f;//zheight - (radius * sn);
	rgv3D[2].tu = 1;
	rgv3D[2].tv = 1;
	rgv3D[2].nx = 0;
	rgv3D[2].ny = 0;
	rgv3D[2].nz = 1;

	rgv3D[1].x = x + radius;
	rgv3D[1].y = y - radius; //y - (radius * cs);
	rgv3D[1].z = 0.1f;//zheight + (radius * sn);
	rgv3D[1].tu = 1;
	rgv3D[1].tv = 0;
	rgv3D[1].nx = 0;
	rgv3D[1].ny = 0;
	rgv3D[1].nz = 1;

	/*m_pin3d.m_lightproject.CalcCoordinates(&rgv3D[0]);
	m_pin3d.m_lightproject.CalcCoordinates(&rgv3D[1]);
	m_pin3d.m_lightproject.CalcCoordinates(&rgv3D[2]);
	m_pin3d.m_lightproject.CalcCoordinates(&rgv3D[3]);*/

	m_pin3d.m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	//m_pin3d.m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
	m_pin3d.m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTFG_LINEAR);
	m_pin3d.m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTFG_LINEAR);
	m_pin3d.m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MIPFILTER, D3DTFP_NONE);

	m_pin3d.m_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,
											  rgv3D, 4,
											  rgi, 4, NULL);

	for (i=0;i<4;i++)
		{
		rgi[i] = i & 3;
		}

	m_pin3d.m_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,
											  rgv3D, 4,
											  rgi, 3, NULL);

	/*rgi[1] = 3;
	rgi[3] = 1;

	m_pin3d.m_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,
											  rgv3D, 4,
											  rgi, 4, NULL);*/
	}

#endif // DRAWTEST

void Player::DrawBallShadows()
	{
	/*int i;

	D3DMATERIAL7 mtrl;
	ZeroMemory( &mtrl, sizeof(mtrl) );
	mtrl.diffuse.a = mtrl.ambient.a = 1;

	m_pin3d.m_pd3dDevice->SetMaterial(&mtrl);

	m_pin3d.m_pd3dDevice->SetRenderState(D3DRENDERSTATE_TEXTUREPERSPECTIVE, FALSE );

	m_pin3d.m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ADDRESS, D3DTADDRESS_CLAMP);

	//float sn = (float)sin(m_pin3d.m_inclination);
	//float cs = (float)cos(m_pin3d.m_inclination);

	for (i=0;i<m_vball.Size();i++)
		{
		Ball *pball = m_vball.ElementAt(i);

		Vertex3D *rgv3DShadow = pball->m_rgv3DShadow;

		if (m_fBallShadows)
			{
			mtrl.diffuse.r = mtrl.ambient.r = 1;
			mtrl.diffuse.g = mtrl.ambient.g = 1;
			mtrl.diffuse.b = mtrl.ambient.b = 1;
			mtrl.diffuse.a = mtrl.ambient.a = 1;

			m_pin3d.m_pd3dDevice->SetMaterial(&mtrl);

			m_pin3d.m_pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
			m_pin3d.m_pd3dDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE);
			
			m_pin3d.m_pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHAREF, (DWORD)0x0000001);
			m_pin3d.m_pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, TRUE); 
			m_pin3d.m_pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHAFUNC, D3DCMP_GREATEREQUAL);

			Ball ballT;
			ballT.x = pball->x;
			ballT.y = pball->y;
			ballT.z = pball->z;
			ballT.vx = 200;
			ballT.vy = -200;
			ballT.vz = -200;
			ballT.radius = 0;
			//ballT.m_plevel = NULL;
			ballT.m_hittime = 1;
			ballT.phoHitLast = NULL;
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

				//m_pin3d.m_pd3dDevice->SetTextureStageState(ePictureTexture, D3DTSS_MAGFILTER, D3DTFG_LINEAR);
				//m_pin3d.m_pd3dDevice->SetTextureStageState(ePictureTexture, D3DTSS_MINFILTER, D3DTFN_LINEAR);
				//m_pin3d.m_pd3dDevice->SetTextureStageState(ePictureTexture, D3DTSS_MIPFILTER, D3DTFP_NONE);

				//m_pin3d.m_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,
														  //rgv3DShadow, 4,
														  //rgi, 4, NULL);

				m_pin3d.m_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,
														  rgv3DShadow, 4,
														  NULL);
				}
			}

		pball->m_fErase = fTrue;

			// Mark ball rect as dirty for blitting to the screen

		if (m_fBallShadows)
			{
			m_pin3d.ClearExtents(&pball->m_rcScreenShadow, NULL, NULL);
			m_pin3d.ExpandExtents(&pball->m_rcScreenShadow, pball->m_rgv3DShadow, NULL, NULL, 4, fFalse);
			}
		}

	m_pin3d.m_pd3dDevice->SetTexture(0, NULL);
	m_pin3d.m_pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, FALSE);*/

	int i;

	D3DMATERIAL7 mtrl;
	ZeroMemory( &mtrl, sizeof(mtrl) );
	/*mtrl.diffuse.r = mtrl.ambient.r = 1;
	mtrl.diffuse.g = mtrl.ambient.g = 1;
	mtrl.diffuse.b = mtrl.ambient.b = 1;*/
	mtrl.diffuse.a = mtrl.ambient.a = 1;

	m_pin3d.m_pd3dDevice->SetMaterial(&mtrl);

	WORD rgi[4];

	for (i=0;i<4;i++)
		{
		rgi[i] = i;

		//rgv3D[i].nx = 0;
		//rgv3D[i].ny = 0;
		//rgv3D[i].nz = 1;
		}

	//m_pin3d.m_pd3dDevice->SetRenderState(D3DRENDERSTATE_LIGHTING, FALSE);

	m_pin3d.m_pd3dDevice->SetRenderState(D3DRENDERSTATE_TEXTUREPERSPECTIVE, FALSE );

	m_pin3d.m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ADDRESS, D3DTADDRESS_CLAMP/*WRAP*/);

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

			m_pin3d.m_pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHAREF, (DWORD)0x0000001);
			m_pin3d.m_pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, TRUE); 
			m_pin3d.m_pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHAFUNC, D3DCMP_GREATEREQUAL);

			Ball ballT;
			ballT.x = pball->x;
			ballT.y = pball->y;
			ballT.z = pball->z;
			ballT.vx = 200;
			ballT.vy = -200;
			ballT.vz = -200;
			ballT.radius = 0;
			//ballT.m_plevel = NULL;
			ballT.m_hittime = 1;
			ballT.phoHitLast = NULL;
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

				//m_pin3d.m_pd3dDevice->SetTextureStageState(ePictureTexture, D3DTSS_MAGFILTER, D3DTFG_LINEAR);
				//m_pin3d.m_pd3dDevice->SetTextureStageState(ePictureTexture, D3DTSS_MINFILTER, D3DTFN_LINEAR);
				//m_pin3d.m_pd3dDevice->SetTextureStageState(ePictureTexture, D3DTSS_MIPFILTER, D3DTFP_NONE);

				//m_pin3d.m_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,
														  //rgv3DShadow, 4,
														  //rgi, 4, NULL);

				m_pin3d.m_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,
														  rgv3DShadow, 4,
														  NULL);
				}
			}
		}
	}

void Player::DrawBalls()
	{
	int i;

	D3DMATERIAL7 mtrl;
	ZeroMemory( &mtrl, sizeof(mtrl) );
	/*mtrl.diffuse.r = mtrl.ambient.r = 1;
	mtrl.diffuse.g = mtrl.ambient.g = 1;
	mtrl.diffuse.b = mtrl.ambient.b = 1;*/
	mtrl.diffuse.a = mtrl.ambient.a = 1;

	m_pin3d.m_pd3dDevice->SetMaterial(&mtrl);

	WORD rgi[4];

	for (i=0;i<4;i++)
		{
		rgi[i] = i;

		//rgv3D[i].nx = 0;
		//rgv3D[i].ny = 0;
		//rgv3D[i].nz = 1;
		}

	//m_pin3d.m_pd3dDevice->SetRenderState(D3DRENDERSTATE_LIGHTING, FALSE);

	m_pin3d.m_pd3dDevice->SetRenderState(D3DRENDERSTATE_TEXTUREPERSPECTIVE, FALSE );

	m_pin3d.m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ADDRESS, D3DTADDRESS_CLAMP/*WRAP*/);

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

		rgv3D[0].x = (float)(pball->x - pball->radius);
		rgv3D[0].y = (float)(pball->y - (pball->radius * cs));
		rgv3D[0].z = (float)(zheight + (pball->radius * sn));
		rgv3D[0].tu = 0;
		rgv3D[0].tv = 0;
		rgv3D[0].nx = 0;
		rgv3D[0].ny = 0;
		rgv3D[0].nz = -1;

		rgv3D[3].x = (float)(pball->x - pball->radius);
		rgv3D[3].y = (float)(pball->y + (pball->radius * cs));
		rgv3D[3].z = (float)(zheight - (pball->radius * sn));
		rgv3D[3].tu = 0;
		//rgv3D[3].tv = 1; // decided by ball picture
		rgv3D[3].nx = 0;
		rgv3D[3].ny = 0;
		rgv3D[3].nz = -1;

		rgv3D[2].x = (float)(pball->x + pball->radius);
		rgv3D[2].y = (float)(pball->y + (pball->radius * cs));
		rgv3D[2].z = (float)(zheight - (pball->radius * sn));
		//rgv3D[2].tu = 1;  // decided by ball picture
		//rgv3D[2].tv = 1;  // decided by ball picture
		rgv3D[2].nx = 0;
		rgv3D[2].ny = 0;
		rgv3D[2].nz = -1;

		rgv3D[1].x = (float)(pball->x + pball->radius);
		rgv3D[1].y = (float)(pball->y - (pball->radius * cs));
		rgv3D[1].z = (float)(zheight + (pball->radius * sn));
		//rgv3D[1].tu = 1;  // decided by ball picture
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

		//m_pin3d.m_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,
												  //rgv3D, 4,
												  //rgi, 4, NULL);

		m_pin3d.m_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,
												  rgv3D, 4,
												  NULL);

		// Draw the ball logo

		if (m_fBallDecals && (pball->m_pinFront || pball->m_pinBack))
			{
			/*mtrl.diffuse.r = mtrl.ambient.r = 0.8f;
			mtrl.diffuse.g = mtrl.ambient.g = 0.4f;
			mtrl.diffuse.b = mtrl.ambient.b = 0.2f;*/
			mtrl.diffuse.a = mtrl.ambient.a = 0.8f;//0.7f;
			m_pin3d.m_pd3dDevice->SetMaterial(&mtrl);

#define DECALPOINTS 4

			Vertex3D rgv3DArrow[DECALPOINTS];
			Vertex3D rgv3DArrowTransformed[DECALPOINTS];
			WORD rgiDecal[DECALPOINTS];
				{
				/*rgv3DArrow[0].x = (float)(-pball->radius/3);
				rgv3DArrow[0].y = (float)(-pball->radius/3);
				rgv3DArrow[0].z = (float)pball->radius * 0.881917103f;
				rgv3DArrow[0].tu = 0;
				rgv3DArrow[0].tv = 0;
				rgv3DArrowTransformed[0].nx = 0;
				rgv3DArrowTransformed[0].ny = 0;
				rgv3DArrowTransformed[0].nz = -1;

				rgv3DArrow[3].x = (float)(-pball->radius/3);
				rgv3DArrow[3].y = (float)(pball->radius/3);
				rgv3DArrow[3].z = (float)pball->radius * 0.881917103f;
				rgv3DArrow[3].tu = 0;
				rgv3DArrow[3].tv = 1;
				rgv3DArrowTransformed[3].nx = 0;
				rgv3DArrowTransformed[3].ny = 0;
				rgv3DArrowTransformed[3].nz = -1;

				rgv3DArrow[2].x = (float)(pball->radius/3);
				rgv3DArrow[2].y = (float)(pball->radius/3);
				rgv3DArrow[2].z = (float)pball->radius * 0.881917103f;
				rgv3DArrow[2].tu = 1;
				rgv3DArrow[2].tv = 1;
				rgv3DArrowTransformed[2].nx = 0;
				rgv3DArrowTransformed[2].ny = 0;
				rgv3DArrowTransformed[2].nz = -1;

				rgv3DArrow[1].x = (float)(pball->radius/3);
				rgv3DArrow[1].y = (float)(-pball->radius/3);
				rgv3DArrow[1].z = (float)pball->radius * 0.881917103f;
				rgv3DArrow[1].tu = 1;
				rgv3DArrow[1].tv = 0;
				rgv3DArrowTransformed[1].nx = 0;
				rgv3DArrowTransformed[1].ny = 0;
				rgv3DArrowTransformed[1].nz = -1;*/

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

				/*rgv3DArrow[0].tu = 0.5f;
				rgv3DArrow[0].tv = 0.5f;
				rgv3DArrow[0].x = 0;
				rgv3DArrow[0].y = 0;
				rgv3DArrow[0].z = -1;

				rgv3DArrow[1].tu = 0;
				rgv3DArrow[1].tv = 0;
				rgv3DArrow[1].x = -0.577350269f;
				rgv3DArrow[1].y = -0.577350269f;
				rgv3DArrow[1].z = -0.577350269f;

				rgv3DArrow[2].tu = 1;
				rgv3DArrow[2].tv = 0;
				rgv3DArrow[2].x = 0.577350269f;
				rgv3DArrow[2].y = -0.577350269f;
				rgv3DArrow[2].z = -0.577350269f;

				rgv3DArrow[3].tu = 1;
				rgv3DArrow[3].tv = 1;
				rgv3DArrow[3].x = 0.577350269f;
				rgv3DArrow[3].y = 0.577350269f;
				rgv3DArrow[3].z = -0.577350269f;

				rgv3DArrow[4].tu = 0;
				rgv3DArrow[4].tv = 1;
				rgv3DArrow[4].x = -0.577350269f;
				rgv3DArrow[4].y = 0.577350269f;
				rgv3DArrow[4].z = -0.577350269f;

				rgv3DArrow[5].tu = 0;
				rgv3DArrow[5].tv = 0;
				rgv3DArrow[5].x = -0.577350269f;
				rgv3DArrow[5].y = -0.577350269f;
				rgv3DArrow[5].z = -0.577350269f;*/

				m_pin3d.m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE);
				//m_pin3d.m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
				m_pin3d.m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTFG_LINEAR);
				m_pin3d.m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTFN_LINEAR);
				m_pin3d.m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MIPFILTER, D3DTFP_LINEAR);

				m_pin3d.m_pd3dDevice->SetRenderState(D3DRENDERSTATE_COLORKEYENABLE, FALSE);
				m_pin3d.m_pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);

				m_pin3d.m_pd3dDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE);

				if (pball->m_pinFront)
					{
					pball->m_pinFront->EnsureColorKey();
					m_pin3d.m_pd3dDevice->SetTexture(0, pball->m_pinFront->m_pdsBufferColorKey);

					int iPoint;
					for (iPoint=0;iPoint<DECALPOINTS;iPoint++)
						{
						rgiDecal[iPoint] = iPoint;
						pball->m_orientation.MultiplyVector(&rgv3DArrow[iPoint], &rgv3DArrowTransformed[iPoint]);
						rgv3DArrowTransformed[iPoint].nx = rgv3DArrowTransformed[iPoint].x;
						rgv3DArrowTransformed[iPoint].ny = rgv3DArrowTransformed[iPoint].y;
						rgv3DArrowTransformed[iPoint].nz = rgv3DArrowTransformed[iPoint].z;
						rgv3DArrowTransformed[iPoint].x *= -pball->radius;
						rgv3DArrowTransformed[iPoint].y *= -pball->radius;
						rgv3DArrowTransformed[iPoint].z *= -pball->radius;
						rgv3DArrowTransformed[iPoint].tu = rgv3DArrow[iPoint].tu * pball->m_pinFront->m_maxtu;
						rgv3DArrowTransformed[iPoint].tv = rgv3DArrow[iPoint].tv * pball->m_pinFront->m_maxtv;
						rgv3DArrowTransformed[iPoint].x += (float)pball->x;
						rgv3DArrowTransformed[iPoint].y += (float)pball->y;
						rgv3DArrowTransformed[iPoint].z += (float)zheight;
						}

					//m_pin3d.m_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,
															  //rgv3DArrowTransformed, DECALPOINTS,
															  //rgiDecal, DECALPOINTS, NULL);
					m_pin3d.m_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,
															  rgv3DArrowTransformed, DECALPOINTS,
															  NULL);
					}

				if (pball->m_pinBack)
					{
					// Other side of ball
					pball->m_pinBack->EnsureColorKey();
					m_pin3d.m_pd3dDevice->SetTexture(0, pball->m_pinBack->m_pdsBufferColorKey);
							
					int iPoint;
					for (iPoint=0;iPoint<DECALPOINTS;iPoint++)
						{
						rgiDecal[iPoint] = iPoint;
						rgv3DArrow[iPoint].x = -rgv3DArrow[iPoint].x;
						rgv3DArrow[iPoint].z = -rgv3DArrow[iPoint].z;
						pball->m_orientation.MultiplyVector(&rgv3DArrow[iPoint], &rgv3DArrowTransformed[iPoint]);
						rgv3DArrowTransformed[iPoint].nx = rgv3DArrowTransformed[iPoint].x;
						rgv3DArrowTransformed[iPoint].ny = rgv3DArrowTransformed[iPoint].y;
						rgv3DArrowTransformed[iPoint].nz = rgv3DArrowTransformed[iPoint].z;
						rgv3DArrowTransformed[iPoint].x *= -pball->radius;
						rgv3DArrowTransformed[iPoint].y *= -pball->radius;
						rgv3DArrowTransformed[iPoint].z *= -pball->radius;
						rgv3DArrowTransformed[iPoint].tu = rgv3DArrow[iPoint].tu * pball->m_pinBack->m_maxtu;
						rgv3DArrowTransformed[iPoint].tv = rgv3DArrow[iPoint].tv * pball->m_pinBack->m_maxtv;
						rgv3DArrowTransformed[iPoint].x += (float)pball->x;
						rgv3DArrowTransformed[iPoint].y += (float)pball->y;
						rgv3DArrowTransformed[iPoint].z += (float)zheight;
						}

					/*m_pin3d.m_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,
															  rgv3DArrowTransformed, DECALPOINTS,
															  rgiDecal, DECALPOINTS, NULL);*/

					m_pin3d.m_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,
															  rgv3DArrowTransformed, DECALPOINTS,
															  NULL);
					}
				}
			}

		pball->m_fErase = fTrue;

			// Mark ball rect as dirty for blitting to the screen
			{
			m_pin3d.ClearExtents(&pball->m_rcScreen, NULL, NULL);
			m_pin3d.ExpandExtents(&pball->m_rcScreen, pball->m_rgv3D, NULL, NULL, 4, fFalse);

			//m_pin3d.ExpandExtents(&pball->m_rcScreen, rgv3DArrow, NULL, NULL, 4, fFalse);

			if (m_fBallShadows)
				{
				m_pin3d.ClearExtents(&pball->m_rcScreenShadow, NULL, NULL);
				m_pin3d.ExpandExtents(&pball->m_rcScreenShadow, pball->m_rgv3DShadow, NULL, NULL, 4, fFalse);

				if (fIntRectIntersect(pball->m_rcScreen, pball->m_rcScreenShadow))
					{
					pball->m_rcScreen.left = min(pball->m_rcScreen.left, pball->m_rcScreenShadow.left);
					pball->m_rcScreen.top = min(pball->m_rcScreen.top, pball->m_rcScreenShadow.top);
					pball->m_rcScreen.right = max(pball->m_rcScreen.right, pball->m_rcScreenShadow.right);
					pball->m_rcScreen.bottom = max(pball->m_rcScreen.bottom, pball->m_rcScreenShadow.bottom);
					InvalidateRect(&pball->m_rcScreen);
					}
				else
					{
					InvalidateRect(&pball->m_rcScreen);
					InvalidateRect(&pball->m_rcScreenShadow);
					}
				}
			else
				{
				InvalidateRect(&pball->m_rcScreen);
				}
			}
		}

	m_pin3d.m_pd3dDevice->SetTexture(0, NULL);
	m_pin3d.m_pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, FALSE);
	}

void Player::DrawBallsDebug()
	{
	int i;

	D3DMATERIAL7 mtrl;
	ZeroMemory( &mtrl, sizeof(mtrl) );
	mtrl.diffuse.a = mtrl.ambient.a = 1;

	m_pin3d.m_pd3dDevice->SetMaterial(&mtrl);

	m_pin3d.EnsureDebugTextures();

	WORD rgi[4];

	for (i=0;i<4;i++)
		{
		rgi[i] = i;
		}

	m_pin3d.m_pd3dDevice->SetRenderState(D3DRENDERSTATE_TEXTUREPERSPECTIVE, FALSE );

	m_pin3d.m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ADDRESS, D3DTADDRESS_CLAMP/*WRAP*/);

	float sn = (float)sin(m_pin3d.m_inclination);
	float cs = (float)cos(m_pin3d.m_inclination);

	for (i=0;i<m_vball.Size();i++)
		{
		Ball *pball = m_vball.ElementAt(i);
		if (pball == m_pactiveballDebug)
			{
			mtrl.diffuse.r = mtrl.ambient.r = 1;
			mtrl.diffuse.g = mtrl.ambient.g = 0.2f;
			mtrl.diffuse.b = mtrl.ambient.b = 0.2f;
			}
		else
			{
			mtrl.diffuse.r = mtrl.ambient.r = 0;
			mtrl.diffuse.g = mtrl.ambient.g = 0;
			mtrl.diffuse.b = mtrl.ambient.b = 0;
			}

		m_pin3d.m_pd3dDevice->SetMaterial(&mtrl);

		Vertex3D *rgv3D = pball->m_rgv3D;

		PINFLOAT zheight;

		zheight = pball->z - pball->radius;

		if (!pball->fFrozen)
			{
			zheight += pball->radius;
			}

		rgv3D[0].x = (float)(pball->x - pball->radius);
		rgv3D[0].y = (float)(pball->y - (pball->radius * cs));
		rgv3D[0].z = (float)(zheight + (pball->radius * sn));
		rgv3D[0].tu = 0;
		rgv3D[0].tv = 0;
		rgv3D[0].nx = 0;
		rgv3D[0].ny = 0;
		rgv3D[0].nz = -1;

		rgv3D[3].x = (float)(pball->x - pball->radius);
		rgv3D[3].y = (float)(pball->y + (pball->radius * cs));
		rgv3D[3].z = (float)(zheight - (pball->radius * sn));
		rgv3D[3].tu = 0;
		//rgv3D[3].tv = 1; // decided by ball picture
		rgv3D[3].nx = 0;
		rgv3D[3].ny = 0;
		rgv3D[3].nz = -1;

		rgv3D[2].x = (float)(pball->x + pball->radius);
		rgv3D[2].y = (float)(pball->y + (pball->radius * cs));
		rgv3D[2].z = (float)(zheight - (pball->radius * sn));
		//rgv3D[2].tu = 1;  // decided by ball picture
		//rgv3D[2].tv = 1;  // decided by ball picture
		rgv3D[2].nx = 0;
		rgv3D[2].ny = 0;
		rgv3D[2].nz = -1;

		rgv3D[1].x = (float)(pball->x + pball->radius);
		rgv3D[1].y = (float)(pball->y - (pball->radius * cs));
		rgv3D[1].z = (float)(zheight + (pball->radius * sn));
		//rgv3D[1].tu = 1;  // decided by ball picture
		rgv3D[1].tv = 0;
		rgv3D[1].nx = 0;
		rgv3D[1].ny = 0;
		rgv3D[1].nz = -1;

		m_pin3d.m_pd3dDevice->SetTexture(0, m_pin3d.m_pddsTargetTexture);
		rgv3D[3].tv = 1;
		rgv3D[2].tu = 1;
		rgv3D[2].tv = 1;
		rgv3D[1].tu = 1;

		m_pin3d.m_pd3dDevice->SetRenderState(D3DRENDERSTATE_COLORKEYENABLE, FALSE);
		m_pin3d.m_pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
		m_pin3d.m_pd3dDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE);
		m_pin3d.m_pd3dDevice->SetRenderState(D3DRENDERSTATE_ZENABLE, FALSE);

		//m_pin3d.m_pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHAREF, (DWORD)0x0000001);
		m_pin3d.m_pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, FALSE); 
		//m_pin3d.m_pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHAFUNC, D3DCMP_GREATEREQUAL);

		/*m_pin3d.m_lightproject.CalcCoordinates(&rgv3D[0]);
		m_pin3d.m_lightproject.CalcCoordinates(&rgv3D[1]);
		m_pin3d.m_lightproject.CalcCoordinates(&rgv3D[2]);
		m_pin3d.m_lightproject.CalcCoordinates(&rgv3D[3]);*/

		m_pin3d.m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE);
		//m_pin3d.m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
		m_pin3d.m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTFG_LINEAR);
		m_pin3d.m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTFN_LINEAR);
		m_pin3d.m_pd3dDevice->SetTextureStageState( 0, D3DTSS_MIPFILTER, D3DTFP_LINEAR);

		//m_pin3d.m_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,
												  //rgv3D, 4,
												  //rgi, 4, NULL);

		m_pin3d.m_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,
												  rgv3D, 4,
												  NULL);

		pball->m_fErase = fTrue;

			// Mark ball rect as dirty for blitting to the screen
			{
			m_pin3d.ClearExtents(&pball->m_rcScreen, NULL, NULL);
			m_pin3d.ExpandExtents(&pball->m_rcScreen, pball->m_rgv3D, NULL, NULL, 4, fFalse);

			//m_pin3d.ExpandExtents(&pball->m_rcScreen, rgv3DArrow, NULL, NULL, 4, fFalse);

			if (m_fBallShadows)
				{
				m_pin3d.ClearExtents(&pball->m_rcScreenShadow, NULL, NULL);
				m_pin3d.ExpandExtents(&pball->m_rcScreenShadow, pball->m_rgv3DShadow, NULL, NULL, 4, fFalse);

				if (fIntRectIntersect(pball->m_rcScreen, pball->m_rcScreenShadow))
					{
					pball->m_rcScreen.left = min(pball->m_rcScreen.left, pball->m_rcScreenShadow.left);
					pball->m_rcScreen.top = min(pball->m_rcScreen.top, pball->m_rcScreenShadow.top);
					pball->m_rcScreen.right = max(pball->m_rcScreen.right, pball->m_rcScreenShadow.right);
					pball->m_rcScreen.bottom = max(pball->m_rcScreen.bottom, pball->m_rcScreenShadow.bottom);
					InvalidateRect(&pball->m_rcScreen);
					}
				else
					{
					InvalidateRect(&pball->m_rcScreen);
					InvalidateRect(&pball->m_rcScreenShadow);
					}
				}
			else
				{
				InvalidateRect(&pball->m_rcScreen);
				}
			}
		}

	m_pin3d.m_pd3dDevice->SetTexture(0, NULL);
	m_pin3d.m_pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, FALSE);
	m_pin3d.m_pd3dDevice->SetRenderState(D3DRENDERSTATE_ZENABLE, TRUE);
	}

int totaloverlap = 0;
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

	//m_pin3d.m_pddsBackBuffer->Blt(prc, m_pin3d.m_pddsStatic, prc, DDBLTFAST_WAIT, NULL);
	//m_pin3d.m_pddsZBuffer->Blt(prc, m_pin3d.m_pddsStaticZ, prc, DDBLTFAST_WAIT, NULL);

	UpdateRect *pur;
	pur = new UpdateRect();
	pur->m_rcupdate = *prc;
	pur->m_fSeeThrough = fTrue;

	for (i=0;i<m_vscreenupdate.Size();i++)
		{
		RECT *prc2 = &m_vscreenupdate.ElementAt(i)->m_rcBounds;
		if (!(prc->right < prc2->left || prc->left > prc2->right || prc->bottom < prc2->top || prc->top > prc2->bottom))
			{
			pur->m_vobject.AddElement(m_vscreenupdate.ElementAt(i));

			/*m_vscreenupdate.ElementAt(i)->m_fInvalid = fTrue;
			RECT *prc = new RECT();

			prc->left = max(prc->left, prc2->left);
			prc->top = max(prc->top, prc2->top);
			prc->right = min(prc->right, prc2->right);
			prc->bottom = min(prc->bottom, prc2->bottom);

			m_vscreenupdate.ElementAt(i)->m_vrcupdate.AddElement(prc);*/
			}
		}

	//if (pur)
		//{

			/*if (!pur)
				{
				pur = new UpdateRect();
				pur->m_rcupdate = *prc;

				int l;
				for (l=0;l<m_vupdaterect.Size();l++)
					{
					RECT *prcOld = &m_vupdaterect.ElementAt(l)->m_rcupdate;
					if (!(prc->right < prcOld->left || prc->left > prcOld->right || prc->bottom < prcOld->top || prc->top > prcOld->bottom))
						{
						totaloverlap++;
						}
					}
				}*/

	m_vupdaterect.AddElement(pur);
		//}
	}

/*struct HitBall
	{
	Ball *pball;
	Vertex hitnormal[2];
	};*/

#ifdef LOG
int cTested;
int cDeepTested;
int cTotalTested = 0;
int cNumUpdates = 0;
#endif

PINFLOAT Player::UpdatePhysics(PINFLOAT dtime, Ball **phitball)
	{
	//float dtime = 1;
	//int b;
	//int i;
	//Vertex hitnormal;
	//Vertex hitnormal[2];

	if (m_vball.Size() < 1)
		{
		return dtime; // inefficient to do this check here, but it makes the calling function simpler
		}

#ifdef PLAYBACK
	if (m_fPlayback)
		{
		float temp = ParseLog(NULL, NULL);
		if (m_fPlayback)
			{
			dtime = temp;
			}
		}
#endif

#ifdef LOG
		fprintf(m_flog, "Time %d ms: %d\n", m_timestamp, m_timeCur);
#endif

	//hitnormal[1].x = 0;
	//hitnormal[1].y = 0;

	Ball *pball;
	PINFLOAT hittime = 0;
	//float newtime;
	//BOOL fHit = fFalse;
	//HitObject *pho = NULL;
	//Ball *phitball;

	int iball;

/*#ifdef DEBUG
		Ball ballOld2;
		ballOld2 = *m_vball.ElementAt(0);
		float vxT, vyT;
		vxT = ballOld2.vx;
		vyT = ballOld2.vy;
#endif*/

#ifdef LOG
	cTested = 0;
	cDeepTested = 0;
	int hitballindex = 0;
#endif

	for (iball = 0; iball<m_vball.Size();iball++)
		{
		pball = m_vball.ElementAt(iball);

		// If the ball has zero velocity, don't bother checking it - the math can get screwed up
		BOOL fspeedcheck = (pball->vx != 0 || pball->vy != 0);

		if (!fspeedcheck)
			{
			pball->m_pho = NULL;
			pball->m_hittime = hittime;
			}

		// If ball already has hit object, don't bother finding it again
		if (!pball->m_fCalced /*&& !pball->fFrozen /*&& !pball->fTempFrozen*/)
			{
			if (!pball->fFrozen && !pball->fTempFrozen && fspeedcheck)
				{
				hittime = dtime;

				//pball->m_fCalced = fTrue;
				pball->m_pho = NULL;
				pball->m_hittime = hittime;

				m_hitoctree.HitTestBall(pball);
				}
			else
				{
				pball->m_pho = NULL;
				}
			}
		}

	// Find ball hit this time around, if any
	hittime = dtime;
	*phitball = NULL;

	for (iball = 0; iball<m_vball.Size();iball++)
		{
		pball = m_vball.ElementAt(iball);
		if (/*pball->m_fCalced &&*/ /*!pball->fFrozen && !pball->fTempFrozen &&*/ pball->m_pho && pball->m_hittime < hittime)
			{
			*phitball = pball;
			hittime = pball->m_hittime;
#ifdef LOG
			hitballindex = iball;
#endif
			/*if (pball->m_pho->GetType() == eBall)
				{
				// Interrupt based ball-to-ball collisions
				((Ball *)pball->m_pho)->m_fCalced = fFalse;
				}*/
			}
		}

#ifdef LOG

	fprintf(m_flog, "Physics Time %.20f\n", dtime);

	cTotalTested += cTested;
	cNumUpdates++;

	fprintf(m_flog, "Tested %d %d %d %f\n", m_vho.Size(), cTested, cDeepTested, (float)cTotalTested/(float)cNumUpdates);

	if (*phitball)
		{
		pball = *phitball;
		fprintf(m_flog, "Ball %d\n", hitballindex);
		fprintf(m_flog, "End Hit x:%f y:%f z:%f vx:%f vy:%f vz:%f hittime:%f hittype:%d nx:%f ny:%f fx:%f fy:%f\n",
			pball->x, pball->y, pball->z, pball->vx, pball->vy, pball->vz, pball->m_hittime, pball->m_pho->GetType(), pball->m_hitnormal[0].x, pball->m_hitnormal[0].y, pball->m_hitnormal[1].x, pball->m_hitnormal[1].y);
		}
	else
		{
		fprintf(m_flog, "End Miss x:%f y:%f z:%f vx:%f vy:%f vz:%f\n",
			pball->x, pball->y, pball->z, pball->vx, pball->vy, pball->vz);
		}
	m_timestamp++;
#endif

	return hittime;
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
	//int index = SendMessage(hwnd, CB_ADDSTRING, 0, (int)sz);
	//SendMessage(hwnd, CB_SETITEMDATA, index, dispid);
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
	//ballT.m_plevel = NULL;
	ballT.m_hittime = 1;
	ballT.phoHitLast = NULL;
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
	//AppendMenu(hmenu, MF_STRING, ID_DRAWINFRONT, "MyString");

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

LRESULT CALLBACK PlayerWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
	switch (uMsg)
		{
		case WM_CLOSE:
			//PostMessage(hwnd, WM_DESTROY, 0, 0);
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
			// TODO - shouldn't all this cleanup live in Table->StopPlaying()?  Well, maybe not if the player is in charge of disabling the editor.
			g_pplayer->m_pininput.UnInit();
			//KillTimer(hwnd, g_pplayer->m_timerid);
			delete g_pplayer;
			g_pplayer = NULL;
			g_pvp->SetEnableToolbar();
			g_pvp->SetEnableMenuItems();
			
			if (g_pvp->m_fPlayOnly)
				{
				SendMessage(g_pvp->m_hwnd, WM_CLOSE, 0, 0);
				}
			break;

		/*case WM_TIMER:
			InvalidateRect(hwnd, NULL, fFalse);
			break;*/

		case WM_PAINT:
			g_pplayer->m_pin3d.Flip(0,0);
			break;

		//case WM_USER+1000:
			//g_pplayer->Render();
			//PostMessage(hwnd, WM_USER+1000, 0, 0);
			//break;

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

		/*case WM_KEYDOWN:
			{
			BOOL fRepeat = fFalse;

			int code = wParam;

			if (code != VK_SHIFT)
				{
			// Extended keyboard support (L/R ALT and L/R CTRL)
				if (lParam & 1<<24)
					{
					code |= (1<<16);
					}

				// Have to use this method to distinguish between left and right shift
				// MapVirtualKey only does left/right difference under NT
					{
					// Don't send repeats
					if ((lParam & 1<<30) != 0)
						{
						fRepeat = fTrue;
						}
					}

				if (!fRepeat)
					{
					g_pplayer->m_ptable->FireKeyEvent(DISPID_GameEvents_KeyDown, code);
					}
				}
			}
			break;

		case WM_KEYUP:
			{
			int code = wParam;

			if (code != VK_SHIFT)
				{
				if (lParam & 1<<24)
					{
					code |= (1<<16);
					}
				g_pplayer->m_ptable->FireKeyEvent(DISPID_GameEvents_KeyUp, code);
				}
			}
			break;*/

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
				//KillTimer(hwnd, g_pplayer->m_timerid);
				}
			else
				{
				g_pplayer->m_fPause = fFalse;
				//g_pplayer->m_timerid = ::SetTimer(hwnd, 11, 1, NULL);
				}
			break;
#endif
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

#ifdef VBA
		case WM_ACTIVATE:
			g_pvp->ApcHost.WmActivate(wParam);
			break;

		case WM_ENABLE:
			g_pvp->ApcHost.WmEnable(wParam);
			break;
#else
		case WM_ACTIVATE:
			if (wParam != WA_INACTIVE)
				{
				g_pplayer->m_fGameWindowActive = fTrue;
				//SetCursor(NULL);
				/*g_pplayer->m_fNoTimeCorrect = fTrue;
				g_pplayer->m_fPause = fFalse;
				g_pplayer->m_fCleanBlt = fFalse;*/
				}
			else
				{
				g_pplayer->m_fGameWindowActive = fFalse;
				//g_pplayer->m_fPause = fTrue;
				}
			g_pplayer->RecomputePauseState();
			break;
#endif

		//case WM_ENTERMENULOOP:
			//g_pplayer->m_fPause = fTrue;
			//break;

		case WM_EXITMENULOOP:
			g_pplayer->m_fNoTimeCorrect = fTrue;
			//g_pplayer->m_fPause = fFalse;
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

				/*HWND hwndTBParent = GetDlgItem(hwndDlg, IDC_TOOLBARSIZE);

				HWND hwndToolbar = CreateToolbarEx(hwndDlg,
					WS_CHILD | WS_VISIBLE | TBSTYLE_TOOLTIPS,
					1, 3, g_hinst, IDB_TB_DEBUG, g_tbbuttonDebug, 3, 16, 24, 16, 24,
					sizeof(TBBUTTON));

				//SetParent(hwndToolbar, hwndTBParent);

				SendMessage(hwndToolbar, TB_SETMAXTEXTROWS, 0, 0);

				char szBuf[1024];
				int i;
				for (i=0;i<3;i++)
					{
					LoadString(g_hinstres, g_tbbuttonDebug[i].dwData, szBuf, 1024-1);
					szBuf[lstrlen(szBuf) + 1] = 0;  //Double-null terminate.
					SendMessage(hwndToolbar, TB_ADDSTRING, 0, (LPARAM) szBuf);
					}

				for (i=0;i<3;i++)
					{
					TBBUTTONINFO tbbi;
					tbbi.cbSize = sizeof(tbbi);
					tbbi.dwMask = TBIF_SIZE | TBIF_COMMAND | TBIF_STATE | TBIF_STYLE;
					SendMessage(hwndToolbar, TB_GETBUTTONINFO, g_tbbuttonDebug[i].idCommand, (LPARAM)&tbbi);
					tbbi.cx = 32;
					SendMessage(hwndToolbar, TB_SETBUTTONINFO, g_tbbuttonDebug[i].idCommand, (LPARAM)&tbbi);
					}

				SendMessage(hwndToolbar, TB_AUTOSIZE, 0, 0);*/

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
					//EnableWindow(hwndExpand, FALSE);
					ShowWindow(hwndExpand, SW_HIDE);
					}

				SendMessage(hwndDlg, RESIZE_FROM_EXPAND, 0, 0);
				//SendDlgItemMessage(hwndDlg, IDC_EXPAND, BM_SETCHECK, BST_CHECKED, 0);

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
							//SendMessage(pnmh->hwndFrom, SCI_LINEDOWN, 0, 0);
							
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
							
							/*SendMessage(pnmh->hwndFrom, SCI_ADDTEXT, lstrlen("\t"), (LPARAM)"\t");
							SendMessage(pnmh->hwndFrom, SCI_ADDTEXT, lstrlen(szResult), (LPARAM)szResult);
							SendMessage(pnmh->hwndFrom, SCI_ADDTEXT, lstrlen("\n"), (LPARAM)"\n");*/
							delete szText;
							//delete szResult;
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
				//EndDialog(hwndDlg, FALSE);
				break;

			case WM_ACTIVATE:
				if (wParam != WA_INACTIVE)
					{
					g_pplayer->m_fDebugWindowActive = fTrue;
					//g_pplayer->m_pininput.m_pKeyboard->SetCooperativeLevel(hwndDlg, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
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
								{
								if (g_pplayer->m_ptable->CheckPermissions(DISABLE_DEBUGGER))
									{
									EndDialog(hwndDlg, ID_RESUME);
									}
								else
									{
									g_pplayer->m_fDebugMode = fTrue;
									/*if (g_pplayer->m_vball.Size() > 0)
										{
										g_pplayer->m_pactiveballDebug = g_pplayer->m_vball.ElementAt(0);
										}*/
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
								EndDialog(hwndDlg, ID_QUIT);
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
	int c;
	float dtime = 0.45f;

	while (1)
		{
		c=0;

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
		/*else if (!strcmp(szWord,"Plunger"))
			{
			sscanf(szLine, "%s %s %d",szWord, szSubWord,&index);
			if (!strcmp(szSubWord, "Pull"))
				{
				((HitPlunger *)m_vmover.ElementAt(index))->m_pplunger->PullBack();
				}
			else // Release
				{
				((HitPlunger *)m_vmover.ElementAt(index))->m_pplunger->Fire();
				}
			}
		else if (!strcmp(szWord,"Flipper"))
			{
			sscanf(szLine, "%s %s %d",szWord, szSubWord,&index);
			if (!strcmp(szSubWord, "End"))
				{
				((HitFlipper *)m_vmover.ElementAt(index))->m_pflipper->RotateToEnd();
				}
			else // Release
				{
				((HitFlipper *)m_vmover.ElementAt(index))->m_pflipper->RotateToStart();
				}
			}*/
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
		//else if (!strcmp(szWord,"Hit"))
			//{
			//return;
			//}
		}
	}

#endif