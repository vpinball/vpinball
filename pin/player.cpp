#include "stdafx.h"
#include <algorithm>
#include "ballMesh.h"

// touch defines, delete as soon as we can get rid of old compilers and use new ones that have these natively

//#define TEST_TOUCH_WITH_MOUSE
#ifdef TEST_TOUCH_WITH_MOUSE
 #define WM_POINTERDOWN WM_LBUTTONDOWN
 #define WM_POINTERUP WM_LBUTTONUP
#else
 #define WM_POINTERDOWN 0x0246
 #define WM_POINTERUP 0x0247
#endif

typedef enum tagPOINTER_INPUT_TYPE { 
  PT_POINTER  = 0x00000001,
  PT_TOUCH    = 0x00000002,
  PT_PEN      = 0x00000003,
  PT_MOUSE    = 0x00000004
} POINTER_INPUT_TYPE;

typedef enum tagPOINTER_FLAGS
{
	POINTER_FLAG_NONE           = 0x00000000,
	POINTER_FLAG_NEW            = 0x00000001,
	POINTER_FLAG_INRANGE        = 0x00000002,
	POINTER_FLAG_INCONTACT      = 0x00000004,
	POINTER_FLAG_FIRSTBUTTON    = 0x00000010,
	POINTER_FLAG_SECONDBUTTON   = 0x00000020,
	POINTER_FLAG_THIRDBUTTON    = 0x00000040,
	POINTER_FLAG_OTHERBUTTON    = 0x00000080,
	POINTER_FLAG_PRIMARY        = 0x00000100,
	POINTER_FLAG_CONFIDENCE     = 0x00000200,
	POINTER_FLAG_CANCELLED      = 0x00000400,
	POINTER_FLAG_DOWN           = 0x00010000,
	POINTER_FLAG_UPDATE         = 0x00020000,
	POINTER_FLAG_UP             = 0x00040000,
	POINTER_FLAG_WHEEL          = 0x00080000,
	POINTER_FLAG_HWHEEL         = 0x00100000, 
	POINTER_FLAG_CAPTURECHANGED = 0x00200000
} POINTER_FLAGS;

typedef enum _POINTER_BUTTON_CHANGE_TYPE { 
  POINTER_CHANGE_NONE               ,
  POINTER_CHANGE_FIRSTBUTTON_DOWN   ,
  POINTER_CHANGE_FIRSTBUTTON_UP     ,
  POINTER_CHANGE_SECONDBUTTON_DOWN  ,
  POINTER_CHANGE_SECONDBUTTON_UP    ,
  POINTER_CHANGE_THIRDBUTTON_DOWN   ,
  POINTER_CHANGE_THIRDBUTTON_UP     ,
  POINTER_CHANGE_FOURTHBUTTON_DOWN  ,
  POINTER_CHANGE_FOURTHBUTTON_UP    ,
  POINTER_CHANGE_FIFTHBUTTON_DOWN   ,
  POINTER_CHANGE_FIFTHBUTTON_UP 
} POINTER_BUTTON_CHANGE_TYPE;

typedef struct tagPOINTER_INFO {
  POINTER_INPUT_TYPE         pointerType;
  UINT32                     pointerId;
  UINT32                     frameId;
  POINTER_FLAGS              pointerFlags;
  HANDLE                     sourceDevice;
  HWND                       hwndTarget;
  POINT                      ptPixelLocation;
  POINT                      ptHimetricLocation;
  POINT                      ptPixelLocationRaw;
  POINT                      ptHimetricLocationRaw;
  DWORD                      dwTime;
  UINT32                     historyCount;
  INT32                      inputData;
  DWORD                      dwKeyStates;
  UINT64                     PerformanceCount;
  POINTER_BUTTON_CHANGE_TYPE ButtonChangeType;
} POINTER_INFO;

typedef BOOL (WINAPI *pGPI)(UINT32 pointerId, POINTER_INFO *pointerInfo);

static pGPI GetPointerInfo = NULL;

#define GET_POINTERID_WPARAM(wParam) (LOWORD (wParam))

const RECT touchregion[8] = { //left,top,right,bottom (in % of screen)
{0,0,50,10},      // ExtraBall
{0,10,50,50},     // 2nd Left Button
{0,50,50,90},     // 1st Left Button (Flipper)
{0,90,50,100},    // Start
{50,0,100,10},    // Exit
{50,10,100,50},   // 2nd Right Button
{50,50,100,90},   // 1st Right Button (Flipper)
{50,90,100,100}}; // Plunger

EnumAssignKeys touchkeymap[8] = {
eAddCreditKey, //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
eLeftMagnaSave,
eLeftFlipperKey,
eStartGameKey,
eExitGame,
eRightMagnaSave,
eRightFlipperKey,
ePlungerKey};

//

#include "..\stereo3D.h"
#ifdef DONGLE_SUPPORT
 #include "..\DongleAPI.h"
#endif

#define RECOMPUTEBUTTONCHECK WM_USER+100
#define RESIZE_FROM_EXPAND WM_USER+101

#if _MSC_VER <= 1310 // VC 2003 and before
inline bool fopen_s(FILE** f, const char *fname, const char *attr)
{
	*f = fopen(fname, attr);
	return (*f == NULL);
}
#endif


LRESULT CALLBACK PlayerWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
#ifdef ULTRAPIN
LRESULT CALLBACK PlayerDMDHackWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
#endif

INT_PTR CALLBACK PauseProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DebuggerProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

Player::Player()
{
	bool SSE2_supported;
	{
	int regs[4];
	__cpuid(regs,1);
	// check for SSE and exit if not available, as some code relies on it by now
	if((regs[3] & 0x002000000) == 0) { // NO SSE?
		ShowError("SSE is not supported on this processor");
		exit(0);
	}
	// disable denormalized floating point numbers, can be faster on some CPUs (and VP doesn't need to rely on denormals)
	SSE2_supported = ((regs[3] & 0x004000000) != 0);
	if(SSE2_supported) // SSE2?
		_mm_setcsr(_mm_getcsr() | 0x8040); // flush denorms to zero and also treat incoming denorms as zeros
	else
		_MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON); // only flush denorms to zero
	}

	m_fPause = false;
	m_fStep = false;
	m_fPseudoPause = false;
	m_pauseRefCount = 0;
	m_fNoTimeCorrect = fFalse;
	m_firstFrame = true;

    m_fThrowBalls = fFalse;
	m_fAccelerometer = fTrue;	// true if electronic Accelerometer enabled 
	m_AccelNormalMount = fTrue;	// normal mounting (left hand coordinates)
	m_AccelAngle = 0;			// 0 degrees (GUI is lefthand coordinates)
	m_AccelAmp = 1.5f;			// Accelerometer gain 
	m_AccelAmpX = m_AccelAmp;	// Accelerometer gain X axis
	m_AccelAmpY = m_AccelAmp;   // Accelerometer gain Y axis
	m_AccelMAmp = 2.5f;			// manual input gain, generally from joysticks
#ifdef PLAYBACK
	m_fPlayback = fFalse;

	m_fplaylog = NULL;
#endif

#ifdef LOG
	m_flog = NULL;
#endif

	for(int i = 0; i < PININ_JOYMXCNT; ++i) {
		m_curAccel_x[i] = 0;
		m_curAccel_y[i] = 0;
	}

	m_sleeptime = 0;

	m_pxap = NULL;
	m_pactiveball = NULL;

	m_curPlunger = JOYRANGEMN-1;

	int shadow;
	HRESULT hr = GetRegInt("Player", "BallShadows", &shadow);
	if (hr != S_OK)
		shadow = fTrue; // The default
	m_fBallShadows = (shadow == 1);

	int decal;
	hr = GetRegInt("Player", "BallDecals", &decal);
	if (hr != S_OK)
		decal = fTrue; // The default
	m_fBallDecals = (decal == 1);

	int antialias;
	hr = GetRegInt("Player", "BallAntialias", &antialias);
	if (hr != S_OK)
		antialias = fTrue; // The default
	m_fBallAntialias = (antialias == 1);

	int vsync;
	hr = GetRegInt("Player", "AdaptiveVSync", &vsync);
	if (hr != S_OK)
		vsync = 0; // The default
	m_fVSync = vsync;

    int maxPrerenderedFrames;
    hr = GetRegInt("Player", "MaxPrerenderedFrames", &maxPrerenderedFrames);
    if (hr != S_OK)
        maxPrerenderedFrames = 2; // The default
    m_fMaxPrerenderedFrames = maxPrerenderedFrames;


    hr = GetRegInt("Player", "FXAA", &m_fFXAA);
    if (hr != S_OK)
      m_fFXAA = 0; // The default = off

    hr = GetRegInt("Player", "BallTrail", &m_fTrailForBalls);
    if (hr != S_OK)
      m_fTrailForBalls = fTrue; // The default = on

	hr = GetRegInt("Player", "BallReflection", &m_fReflectionForBalls);
    if (hr != S_OK)
      m_fReflectionForBalls = fTrue; // The default = on

    hr = GetRegInt("Player", "USEAA", &m_fAA);
    if (hr != S_OK)
      m_fAA = fFalse; // The default = off

	hr = GetRegInt("Player", "Stereo3D", &m_fStereo3D);
	if (hr != S_OK)
		m_fStereo3D = 0; // The default = off

	int stereo3Denabled;
	hr = GetRegInt("Player", "Stereo3DEnabled", &stereo3Denabled);
	if (hr != S_OK)
		stereo3Denabled = (m_fStereo3D != 0); // The default
	m_fStereo3Denabled = (stereo3Denabled == 1);

	int stereo3DAA;
	hr = GetRegInt("Player", "Stereo3DAntialias", &stereo3DAA);
	if (hr != S_OK)
		stereo3DAA = fTrue; // The default
	m_fStereo3DAA = (stereo3DAA == 1);

	int stereo3DY;
	hr = GetRegInt("Player", "Stereo3DYAxis", &stereo3DY);
	if (hr != S_OK)
		stereo3DY = fFalse; // The default
	m_fStereo3DY = (stereo3DY == 1);

    int detecthang;
	hr = GetRegInt("Player", "DetectHang", &detecthang);
	if (hr != S_OK)
		detecthang = fFalse; // The default
	m_fDetectScriptHang = (detecthang == 1);

	m_fShowFPS = fFalse;

	m_fCloseDown = fFalse;
	m_fCloseType = 0;

	m_DebugBalls = fFalse;
	m_ToggleDebugBalls = fFalse;

	m_fDebugMode = fFalse;
	m_hwndDebugger = NULL;
	m_PauseTimeTarget = 0;
	m_pactiveballDebug = NULL;

	m_fGameWindowActive = false;
	m_fDebugWindowActive = false;
	m_fUserDebugPaused = false;
	m_hwndDebugOutput = NULL;

	m_LastKnownGoodCounter = 0;
	m_ModalRefCount = 0;

	m_fDrawCursor = false;
	m_lastcursorx = 0xfffffff;
	m_lastcursory = 0xfffffff;
	m_NudgeManual = -1;

#ifdef _DEBUGPHYSICS
	c_hitcnts = 0;
	c_collisioncnt = 0;
	c_contactcnt = 0;
	c_staticcnt = 0;
	c_embedcnts = 0;
	c_timesearch = 0;

	c_octNextlevels = 0;
	c_traversed = 0;
	c_tested = 0;
    c_deepTested = 0;
#endif

	m_movedPlunger = 0;
	m_LastPlungerHit = 0;
	m_Coins = 0;

	for(unsigned int i = 0; i < 8; ++i)
		m_touchregion_pressed[i] = false;

#ifdef DEBUG_FPS
	ToggleFPS();
#endif
}

Player::~Player()
{
	for (unsigned i=0; i < m_vhitables.size(); ++i)
	{
        m_vhitables[i]->EndPlay();
	}

	for (int i=0;i<m_vho.Size();i++)
		delete m_vho.ElementAt(i);
	m_vho.RemoveAllElements();

	for (int i=0;i<m_vdebugho.Size();i++)
		delete m_vdebugho.ElementAt(i);
	m_vdebugho.RemoveAllElements();

	//!! cleanup the whole mem management for balls, this is a mess!

	// balls are added to the octree, but not the hit object vector
	for (unsigned i=0; i<m_vball.size(); i++)
	{
		Ball * const pball = m_vball[i];
		if (pball->m_pballex)
		{
			pball->m_pballex->m_pball = NULL;
			pball->m_pballex->Release();
		}

		delete pball->m_vpVolObjs;
		delete pball;
	}

	//!! see above
	//for (int i=0;i<m_vho_dynamic.Size();i++)
	//	delete m_vho_dynamic.ElementAt(i);
	//m_vho_dynamic.RemoveAllElements();

	m_vball.clear();

	if ( Ball::vertexBuffer!=0 )
    {
        Ball::vertexBuffer->release();
        Ball::vertexBuffer=0;
    }
    if ( ballIndexBuffer )
    {
        ballIndexBuffer->release();
        ballIndexBuffer=0;
    }
    if( ballShader )
    {
        delete ballShader;
        ballShader=0;
    }
#ifdef LOG
	if (m_flog)
		fclose(m_flog);
#endif
#ifdef PLAYBACK
	if (m_fplaylog)
		fclose(m_fplaylog);
#endif

	//CloseHandle(m_hSongCompletionEvent);

	if (m_pxap)
	{
		delete m_pxap;
		m_pxap = NULL;
	}

	for (int i=0;i<m_controlclsidsafe.Size();i++)
		delete m_controlclsidsafe.ElementAt(i);
	m_controlclsidsafe.RemoveAllElements();
}

void Player::ToggleFPS()
{
	m_fShowFPS = !m_fShowFPS;
	m_lastfpstime = m_time_msec;
	m_cframes = 0;
	m_fps = 0.0f;
    m_fpsAvg = 0.0f;
    m_fpsCount = 0;
	m_total = 0;
	m_count = 0;
	m_max = 0;
    m_lastMaxChangeTime = 0;
    m_lastTime_usec = 0;

	m_phys_total = 0;
	m_phys_max = 0;
	m_phys_max_iterations = 0;
	m_phys_total_iterations = 0;
}

void Player::RecomputePauseState()
{
	const bool fOldPause = m_fPause;
	const bool fNewPause = !(m_fGameWindowActive || m_fDebugWindowActive);// || m_fUserDebugPaused;

	if (fOldPause && fNewPause)
		{
		m_LastKnownGoodCounter++; // So our catcher doesn't catch on the last value
		m_fNoTimeCorrect = fTrue;
		}

	m_fPause = fNewPause;
}

void Player::RecomputePseudoPauseState()
{
	const bool fOldPseudoPause = m_fPseudoPause;
	m_fPseudoPause = m_fUserDebugPaused || m_fDebugWindowActive;
	if (fOldPseudoPause != m_fPseudoPause)
		{
		if (m_fPseudoPause)
			PauseMusic();
		else
			UnpauseMusic();
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

	Vertex3Ds * const rgv3D = new Vertex3Ds[4];
	rgv3D[0] = Vertex3Ds(m_ptable->m_left,m_ptable->m_top,m_ptable->m_glassheight);
	rgv3D[1] = Vertex3Ds(m_ptable->m_right,m_ptable->m_top,m_ptable->m_glassheight);
	rgv3D[2] = Vertex3Ds(m_ptable->m_right,m_ptable->m_bottom,m_ptable->m_glassheight);
	rgv3D[3] = Vertex3Ds(m_ptable->m_left,m_ptable->m_bottom,m_ptable->m_glassheight);

	Hit3DPoly * const ph3dpoly = new Hit3DPoly(rgv3D,4); //!!

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
		key = DIK_T;
		}
	m_rgKeys[eMechanicalTilt] = (EnumAssignKeys)key;

	hr = GetRegInt("Player","RMagnaSave", &key);
	if (hr != S_OK || key > 0xdd)
		{
		key = DIK_RCONTROL;		//157 (0x9D) DIK_RCONTROL        0x9D
		}
	m_rgKeys[eRightMagnaSave] = (EnumAssignKeys)key;

	hr = GetRegInt("Player","LMagnaSave", &key);
	if (hr != S_OK || key > 0xdd)
		{
		key = DIK_LCONTROL; //29 (0x1D)
		}
	m_rgKeys[eLeftMagnaSave] = (EnumAssignKeys)key;

	hr = GetRegInt("Player","Enable3DKey", &key);
	if (hr != S_OK || key > 0xdd)
		{
      key = DIK_F10;
		}
	m_rgKeys[eEnable3D] = (EnumAssignKeys)key;
}

void Player::InitRegValues()
{
	HRESULT hr;

	hr = GetRegInt("Player", "PlayMusic", &m_fPlayMusic);
	if (hr != S_OK)
		m_fPlayMusic = 1; // default value

	hr = GetRegInt("Player", "PlaySound", &m_fPlaySound);
	if (hr != S_OK)
		m_fPlaySound = 1; // default value

	hr = GetRegInt("Player", "MusicVolume", &m_MusicVolume);
	if (hr != S_OK)
		m_MusicVolume = 100; // default value

	hr = GetRegInt("Player", "SoundVolume", &m_SoundVolume);
	if (hr != S_OK)
		m_SoundVolume = 100; // default value
}

void Player::InitDebugHitStructure()
{
    for (unsigned i=0; i < m_vhitables.size(); ++i)
    {
        Hitable * const ph = m_vhitables[i];
        const int currentsize = m_vdebugho.Size();
        ph->GetHitShapesDebug(&m_vdebugho);
        const int newsize = m_vdebugho.Size();
        // Save the objects the trouble of having the set the idispatch pointer themselves
        for (int hitloop = currentsize;hitloop < newsize;hitloop++)
            m_vdebugho.ElementAt(hitloop)->m_pfedebug = m_ptable->m_vedit.ElementAt(i)->GetIFireEvents();
    }

    for(int i = 0; i < m_vdebugho.Size(); ++i)
    {
        m_vdebugho.ElementAt(i)->CalcHitRect();
        m_debugoctree.AddElement( m_vdebugho.ElementAt(i) );
    }

    m_debugoctree.Initialize(m_ptable->GetBoundingBox());
}

Vertex3Ds g_viewDir;

static bool CompareHitableDepth(Hitable* h1, Hitable* h2)
{
    //  GetDepth approximates direction in view distance to camera; sort ascending
    return h1->GetDepth(g_viewDir) >= h2->GetDepth(g_viewDir);
}

const VertexElement ballDeclaration[] = 
{
   { 0, 0  * sizeof(float),D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },  // pos
   { 0, 3  * sizeof(float),D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,   0 },  // normal
   { 0, 6  * sizeof(float),D3DDECLTYPE_FLOAT2,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },  // tex0
   D3DDECL_END()
};
static VertexDeclaration* g_pBallVertexDeclaration	= NULL;

void Player::InitBallShader()
{
   ballShader = new Shader(m_pin3d.m_pd3dDevice );

   //ballShader->Load("c:\\shader\\BallShader.fx", true );

   ballShader->Load("BallShader.fx", false );

   m_pin3d.m_pd3dDevice->CreateVertexDeclaration( ballDeclaration, &g_pBallVertexDeclaration );

   D3DMATRIX worldMat;
   D3DMATRIX viewMat;
   D3DMATRIX projMat;
   m_pin3d.m_pd3dDevice->GetTransform(TRANSFORMSTATE_WORLD, &worldMat );
   m_pin3d.m_pd3dDevice->GetTransform(TRANSFORMSTATE_VIEW, &viewMat);
   m_pin3d.m_pd3dDevice->GetTransform(TRANSFORMSTATE_PROJECTION, &projMat);

   D3DXMATRIX matProj(projMat);
   D3DXMATRIX matView(viewMat);
   D3DXMATRIX matWorld(worldMat);
   D3DXMATRIX worldViewProj = matWorld * matView * matProj;
   ballShader->Core()->SetMatrix("matWorldViewProj", &worldViewProj);
   ballShader->Core()->SetMatrix("matProj", &matProj);
   ballShader->Core()->SetMatrix("matView", &matView);
   ballShader->Core()->SetMatrix("matWorld",  &matWorld);
   //ballShader->SetMatrix("matWorldI", &matWorldI);
   vector<WORD> indexList;
   indexList.resize(basicBallNumFaces);
   memcpy(&indexList[0],basicBallIndices, sizeof(WORD)*basicBallNumFaces);
   ballIndexBuffer = m_pin3d.m_pd3dDevice->CreateAndFillIndexBuffer( indexList );

}



HRESULT Player::Init(PinTable * const ptable, const HWND hwndProgress, const HWND hwndProgressName)
{
    TRACE_FUNCTION();

	m_ptable = ptable;

	//accelerometer normal mounting is 90 degrees in left-hand coordinates (1/4 turn counterclockwise)
	m_fAccelerometer = m_ptable->m_tblAccelerometer;		// true if electronic Accelerometer enabled
	m_AccelNormalMount = m_ptable->m_tblAccelNormalMount;	// true is normal mounting (left hand coordinates)
	m_AccelAngle = ANGTORAD(m_ptable->m_tblAccelAngle);     // 0 rotated counterclockwise (GUI is lefthand coordinates)
	m_AccelAmp = m_ptable->m_tblAccelAmp;					// Accelerometer gain 
	m_AccelAmpX = m_ptable->m_tblAccelAmpX;
	m_AccelAmpY = m_ptable->m_tblAccelAmpY;
	m_AccelMAmp = m_ptable->m_tblAccelManualAmp;			// manual input gain, generally from joysticks

	m_jolt_amount = (U32)m_ptable->m_jolt_amount;
	m_tilt_amount = (U32)m_ptable->m_tilt_amount;
	m_jolt_trigger_time = (U32)m_ptable->m_jolt_trigger_time;
	m_tilt_trigger_time = (U32)m_ptable->m_tilt_trigger_time;

    ShadowSur::m_shadowDirX = ptable->m_shadowDirX;
    ShadowSur::m_shadowDirY = ptable->m_shadowDirY;

    //m_hSongCompletionEvent = CreateEvent( NULL, TRUE, FALSE, NULL );

	SendMessage(hwndProgress, PBM_SETPOS, 40, 0);
	// TEXT
	SetWindowText(hwndProgressName, "Initalizing Visuals...");

	InitWindow();
#ifdef ULTRAPIN
	InitDMDHackWindow();
#endif

	InitKeys();

	InitRegValues();

	int vsync = (m_ptable->m_TableAdaptiveVSync == -1) ? m_fVSync : m_ptable->m_TableAdaptiveVSync;

    const bool useAA = false; /* TODO: disabled until flickering fixed */ // (m_fAA && (m_ptable->m_useAA == -1)) || (m_ptable->m_useAA == 1);
    const bool stereo3DFXAA = (!!m_fStereo3D) || ((m_fFXAA && (m_ptable->m_useFXAA == -1)) || (m_ptable->m_useFXAA > 0));

	// width, height, and colordepth are only defined if fullscreen is true.
    HRESULT hr = m_pin3d.InitPin3D(m_hwnd, m_fFullScreen != 0, m_screenwidth, m_screenheight, m_screendepth,
                   m_refreshrate, vsync, useAA, stereo3DFXAA);

	if (hr != S_OK)
	{
		char szfoo[64];
		sprintf_s(szfoo, "Error code: %x",hr);
		ShowError(szfoo);
		return hr;
	}

	if (m_fFullScreen)
		{
		SetWindowPos(m_hwnd, NULL, 0, 0, m_screenwidth, m_screenheight, SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE);
		m_pixelaspectratio = ((GPINFLOAT)m_screenwidth / (GPINFLOAT)m_screenheight) / (4.0/3.0);
		}
	else
		{
		m_pixelaspectratio = ((GPINFLOAT)m_width / (GPINFLOAT)m_height) / (4.0/3.0);
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

	m_pin3d.InitLayout();

	const float slope = ptable->m_angletiltMin 
					  + (ptable->m_angletiltMax - ptable->m_angletiltMin) 
					  * ptable->m_globalDifficulty;

	m_gravity.x = 0;
	m_gravity.y =  sinf(ANGTORAD(slope))*(ptable->m_fOverridePhysics ? ptable->m_fOverrideGravityConstant : ptable->m_Gravity);
	m_gravity.z = -cosf(ANGTORAD(slope))*(ptable->m_fOverridePhysics ? ptable->m_fOverrideGravityConstant : ptable->m_Gravity);

	m_NudgeX = 0;
	m_NudgeY = 0;
	m_nudgetime = 0;
	m_movedPlunger = 0;	// has plunger moved, must have moved at least three times

	SendMessage(hwndProgress, PBM_SETPOS, 50, 0);
	SetWindowText(hwndProgressName, "Initalizing Physics...");

	// Need to set timecur here, for init functions that set timers
	m_time_msec = 0;

#ifdef FPS
    ToggleFPS();
    m_fShowFPS = FALSE;
#endif

	for (int i=0;i<m_ptable->m_vedit.Size();i++)
	{
		IEditable * const pe = m_ptable->m_vedit.ElementAt(i);
		Hitable * const ph = pe->GetIHitable();
		if (ph)
		{
			const int currentsize = m_vho.Size();
			ph->GetHitShapes(&m_vho);
			const int newsize = m_vho.Size();
			// Save the objects the trouble of having to set the idispatch pointer themselves
			for (int hitloop = currentsize; hitloop < newsize; hitloop++)
				m_vho.ElementAt(hitloop)->m_pfedebug = pe->GetIFireEvents();

			ph->GetTimers(&m_vht);

            // build list of hitables
            m_vhitables.push_back(ph);
		}
	}

	CreateBoundingHitShapes(&m_vho);

	for (int i = 0; i < m_vho.Size(); ++i)
    {
        HitObject *pho = m_vho.ElementAt(i);

        pho->CalcHitRect();

        m_hitoctree.AddElement(pho);

        if (((pho->GetType() == e3DPoly) && ((Hit3DPoly *)pho)->m_fVisible) ||
            ((pho->GetType() == eTriangle) && ((HitTriangle *)pho)->m_fVisible) )
            m_shadowoctree.AddElement(pho);

        AnimObject *pao = pho->GetAnimObject();
        if (pao)
        {
            m_vscreenupdate.AddElement(pao);
            if (pao->FMover())
                m_vmover.push_back(pao);
        }
    }

    FRect3D tableBounds = m_ptable->GetBoundingBox();
    m_hitoctree.Initialize(tableBounds);
#ifndef NDEBUG
    m_hitoctree.DumpTree(0);
#endif

    m_shadowoctree.Initialize(tableBounds);

    // initialize hit structure for dynamic objects
    m_hitoctree_dynamic.FillFromVector( m_vho_dynamic );

    Ball::ballsInUse=0;

	//----------------------------------------------------------------------------------

	SendMessage(hwndProgress, PBM_SETPOS, 60, 0);
	SetWindowText(hwndProgressName, "Rendering Table...");

	// Pre-render all non-changing elements such as 
	// static walls, rails, backdrops, etc.
	InitStatic(hwndProgress);

	for (int i=0; i < m_ptable->m_vedit.Size(); ++i)
	{
		IEditable * const pe = m_ptable->m_vedit.ElementAt(i);
		Hitable * const ph = pe->GetIHitable();
		if (ph)
		{
            // sort into proper categories
            if (pe->GetItemType() == eItemLight)
                m_vLights.push_back(ph);            // VP9COMPAT: special treatment for lights
            else if (pe->m_fBackglass)
                m_vHitBackglass.push_back(ph);      // VP9COMPAT: fixes Homer head on TSPP, remove in VP10
            else if (ph->IsTransparent())
                m_vHitTrans.push_back(ph);
            else
                m_vHitNonTrans.push_back(ph);
		}
	}

    //g_viewDir = m_pin3d.m_viewVec;
    g_viewDir = Vertex3Ds(0, 0, -1.0f);
    std::sort( m_vHitTrans.begin(), m_vHitTrans.end(), CompareHitableDepth );

    InitBallShader();

	// Direct all renders to the back buffer.
    m_pin3d.SetRenderTarget(m_pin3d.m_pddsBackBuffer, m_pin3d.m_pddsZBuffer);

	SendMessage(hwndProgress, PBM_SETPOS, 90, 0);

#ifdef _DEBUGPHYSICS
    {
        std::vector< Vertex3D_NoLighting > ballDbgVtx;
        for (int j = -1; j <= 1; ++j)
        {
            const int numPts = (j==0) ? 6 : 3;
            const float theta = (float)(j * M_PI / 4.0f);
            for (int i = 0; i < numPts; ++i)
            {
                const float phi = (float)(i * 2 * M_PI / numPts);
                Vertex3D_NoLighting vtx;
                vtx.x = 25.0f * cosf(theta) * cosf(phi);
                vtx.y = 25.0f * cosf(theta) * sinf(phi);
                vtx.z = 25.0f * sinf(theta);
                vtx.color = 0xFF00FF00;
                ballDbgVtx.push_back(vtx);
            }
        }

        m_pin3d.m_pd3dDevice->CreateVertexBuffer( ballDbgVtx.size(), 0, MY_D3DFVF_NOLIGHTING_VERTEX, &m_ballDebugPoints );
        void *buf;
        m_ballDebugPoints->lock(0, 0, &buf, 0);
        memcpy(buf, &ballDbgVtx[0], ballDbgVtx.size() * sizeof(ballDbgVtx[0]));
        m_ballDebugPoints->unlock();
    }
#endif

	m_ptable->m_pcv->Start(); // Hook up to events and start cranking script

	SetWindowText(hwndProgressName, "Starting Game Scripts...");

	m_ptable->FireVoidEvent(DISPID_GameEvents_Init);

#ifdef LOG
	m_flog = fopen("c:\\log.txt","w");
	m_timestamp = 0;
#endif

#ifdef PLAYBACK
	if (m_fPlayback)
		m_fplaylog = fopen("c:\\badlog.txt","r");
#endif

	wintimer_init();

	m_StartTime_usec = usec();

	m_curPhysicsFrameTime = m_StartTime_usec;
	m_nextPhysicsFrameTime = m_curPhysicsFrameTime + PHYSICS_STEPTIME;

#ifdef PLAYBACK
	if (m_fPlayback)
		ParseLog((LARGE_INTEGER*)&m_PhysicsStepTime, (LARGE_INTEGER*)&m_StartTime_usec);
#endif

#ifdef LOG
	fprintf(m_flog, "Step Time %llu\n", m_StartTime_usec);
	fprintf(m_flog, "End Frame\n");
#endif

	SendMessage(hwndProgress, PBM_SETPOS, 100, 0);

	SetWindowText(hwndProgressName, "Starting...");

	// Check if we should show the window.
	// Show if we don't have a front end, or autostart is not enabled.
	const HWND hFrontEndWnd = FindWindow( NULL, "Ultrapin (plfe)" );
	if ( (hFrontEndWnd == NULL) || 
		 (!m_ptable->m_tblAutoStartEnabled) )
	{
		// Show the window.
		ShowWindow(m_hwnd, SW_SHOW);
		SetForegroundWindow(m_hwnd);
		SetFocus(m_hwnd);
	}

	// Call Init -- TODO: what's the relation to ptable->FireVoidEvent() above?
	for (unsigned i=0; i < m_vhitables.size(); ++i)
    {
        Hitable * const ph = m_vhitables[i];
        if (ph->GetEventProxyBase())
            ph->GetEventProxyBase()->FireVoidEvent(DISPID_GameEvents_Init);
    }

	if (m_fDetectScriptHang)
		g_pvp->PostWorkToWorkerThread(HANG_SNOOP_START, NULL);

#ifdef DONGLE_SUPPORT
	if ( get_dongle_status() != DONGLE_STATUS_OK )
	{
		// Exit the application.
		ExitApp();
	}
#endif

    // TODO: the limit should be a GUI option; 0 means disable limiting of drawahead queue
    m_limiter.Init(m_fMaxPrerenderedFrames);

	Render();

	return S_OK;
}


void Player::InitStatic(HWND hwndProgress)
{
    TRACE_FUNCTION();
	// Start the frame.
	m_pin3d.m_pd3dDevice->BeginScene();

	// Direct all renders to the "static" buffer.
	m_pin3d.SetRenderTarget(m_pin3d.m_pddsStatic, m_pin3d.m_pddsStaticZ);

    m_pin3d.DrawBackground();

    // HACK: make sure m_maxtu and m_maxtv are set before calling RenderSetup() on the Hitables
    m_pin3d.EnableLightMap(0.0f);
    m_pin3d.DisableLightMap();

    // perform render setup and give elements a chance to render before the playfield
	for (unsigned i=0; i < m_vhitables.size(); ++i)
    {
        Hitable * const ph = m_vhitables[i];
        ph->RenderSetup(m_pin3d.m_pd3dDevice);
        ph->PreRenderStatic(m_pin3d.m_pd3dDevice);
    }

    m_pin3d.RenderPlayfieldGraphics();

	// Draw stuff
	for (int i=0;i<m_ptable->m_vedit.Size();i++)
		{
		if (m_ptable->m_vedit.ElementAt(i)->GetItemType() != eItemDecal)
			{
			Hitable * const ph = m_ptable->m_vedit.ElementAt(i)->GetIHitable();
			if (ph)
				{
				ph->RenderStatic(m_pin3d.m_pd3dDevice);
				if (hwndProgress)
					SendMessage(hwndProgress, PBM_SETPOS, 60 + ((15*i)/m_ptable->m_vedit.Size()), 0);
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
					SendMessage(hwndProgress, PBM_SETPOS, 75 + ((15*i)/m_ptable->m_vedit.Size()), 0);
				}
			}
		}

	// Finish the frame.
	m_pin3d.m_pd3dDevice->EndScene();
}

Ball *Player::CreateBall(const float x, const float y, const float z, const float vx, const float vy, const float vz, const float radius)
{
	Ball * const pball = new Ball();
	pball->radius = radius;
	pball->pos.x = x;
	pball->pos.y = y;
	pball->pos.z = z+pball->radius;

	//pball->z = z;
	pball->vel.x = vx;
	pball->vel.y = vy;
	pball->vel.z = vz;
	pball->Init(); // Call this after radius set to get proper inertial tensor set up

	pball->EnsureOMObject();

	pball->m_pfedebug = (IFireEvents *)pball->m_pballex;

	m_vball.push_back(pball);
	m_vmover.push_back(&pball->m_ballanim);

	pball->CalcHitRect();

	m_vho_dynamic.AddElement(pball);
    m_hitoctree_dynamic.FillFromVector(m_vho_dynamic);

	if (!m_pactiveballDebug)
		m_pactiveballDebug = pball;

	pball->defaultZ = pball->pos.z;
	
	return pball;
}

void Player::DestroyBall(Ball *pball)
{
	if (!pball) return;

	if (pball->m_pballex)
		{
		pball->m_pballex->m_pball = NULL;
		pball->m_pballex->Release();
		}

    RemoveFromVector( m_vball, pball );
    RemoveFromVector<AnimObject*>( m_vmover, &pball->m_ballanim );

	m_vho_dynamic.RemoveElement(pball);
    m_hitoctree_dynamic.FillFromVector(m_vho_dynamic);

	m_vballDelete.push_back(pball);

	if (m_pactiveballDebug == pball)
		m_pactiveballDebug = (!m_vball.empty()) ? m_vball.front() : NULL;
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
	wcex.hIcon = LoadIcon ( g_hinst, MAKEINTRESOURCE(IDI_TABLE) );
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
	wcex.hIcon = LoadIcon(g_hinst, MAKEINTRESOURCE(IDI_TABLE));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.lpszMenuName = NULL;
	RegisterClassEx(&wcex);

	HRESULT hr;

	hr = GetRegInt("Player", "Width", &m_width);
	if (hr != S_OK)
		m_width = DEFAULT_PLAYER_WIDTH; // The default

	hr = GetRegInt("Player", "Height", &m_height);
	if (hr != S_OK)
		m_height = m_width *3/4;

	hr = GetRegInt("Player", "FullScreen", &m_fFullScreen);
	if (hr != S_OK)
		m_fFullScreen = fFalse;

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
			m_screendepth = 32; // The default
		hr = GetRegInt("Player", "RefreshRate", &m_refreshrate);
		if (hr != S_OK)
			m_refreshrate = 0; // The default
	}
	else
	{
		screenwidth = GetSystemMetrics(SM_CXSCREEN);
		screenheight = GetSystemMetrics(SM_CYSCREEN);
	}

	// constrain window to screen
	if (!m_fFullScreen && (m_width > screenwidth))
	{
		m_width = screenwidth;
		m_height = m_width * 3 / 4;
	}

	if (!m_fFullScreen && (m_height > screenheight))
	{
		m_height = screenheight;
		m_width = m_height * 4 / 3;
	}

	int x = (screenwidth - m_width) / 2;
	int y = (screenheight - m_height) / 2;

	// No window border, title, or control boxes.
	int windowflags = WS_POPUP;
	int windowflagsex = 0;

	const int captionheight = GetSystemMetrics(SM_CYCAPTION);

	if (!m_fFullScreen && (screenheight - m_height >= (captionheight*2))) // We have enough room for a frame
	{
#ifdef ULTRAPIN
		// Check if we have a front end.
		if ( FindWindow( NULL, "Ultrapin (plfe)" ) != NULL )
		{
			windowflagsex = WS_EX_TOPMOST;

			// Place window in top-left corner.
			x = 0;
			y = 0;
		}
		else
#endif
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
		ballStretchMode = 0;

	// Monitors: 4:3, 16:9, 16:10, 21:10
	int ballStretchMonitor;
	hr = GetRegInt("Player", "BallStretchMonitor", &ballStretchMonitor);
	if (hr != S_OK)
		ballStretchMonitor = 1; // assume 16:9

	const float scalebackX = (m_ptable->m_scalex != 0.0f) ? ((m_ptable->m_scalex + m_ptable->m_scaley)*0.5f)/m_ptable->m_scalex : 1.0f;
	const float scalebackY = (m_ptable->m_scaley != 0.0f) ? ((m_ptable->m_scalex + m_ptable->m_scaley)*0.5f)/m_ptable->m_scaley : 1.0f;

	float xMonitor = 16.0f;
	float yMonitor = 9.0f;
	switch (ballStretchMonitor)
	{
	case 0: 
		xMonitor = (float)(4.0 / 4.0);
		yMonitor = (float)(3.0 / 3.0);
		break;
	case 1: 
		xMonitor = (float)(16.0 / 4.0);
		yMonitor = (float)(9.0 / 3.0);
		break;
	case 2: 
		xMonitor = (float)(16.0 / 4.0);
		yMonitor = (float)(10.0 / 3.0);
		break;
	case 3: 
		xMonitor = (float)(21.0 / 4.0);
		yMonitor = (float)(10.0 / 3.0);
		break;
	case 4: 
		xMonitor = (float)(3.0 / 4.0);
		yMonitor = (float)(4.0 / 3.0);
		break;
	case 5: 
		xMonitor = (float)(9.0 / 4.0);
		yMonitor = (float)(16.0 / 3.0);
		break;
	case 6: 
		xMonitor = (float)(10.0 / 4.0);
		yMonitor = (float)(16.0 / 3.0);
		break;
	case 7: 
		xMonitor = (float)(10.0 / 4.0);
		yMonitor = (float)(21.0 / 3.0);
		break;

	}
	const float scalebackMonitorX = ((xMonitor + yMonitor)*0.5f)/xMonitor;
	const float scalebackMonitorY = ((xMonitor + yMonitor)*0.5f)/yMonitor;

	float temprotation = m_ptable->m_rotation;
	while (temprotation < 0.f)
	{
		temprotation += 360.0f;
	}

	const float c = sinf(ANGTORAD(fmodf(temprotation + 90.0f,180.0f)));
	const float s = sinf(ANGTORAD(fmodf(temprotation,180.0f)));
	switch(ballStretchMode)
	{
		case 0:	m_BallStretchX = 1.0f;
				m_BallStretchY = 1.0f;
				break;
		case 1: m_BallStretchX = scalebackX*c + scalebackY*s;
				m_BallStretchY = scalebackY*c + scalebackX*s;
				break;
		case 2: m_BallStretchX = scalebackX*c + scalebackY*s;
				m_BallStretchY = scalebackY*c + scalebackX*s;
				if (m_fFullScreen || (m_width == screenwidth && m_height == screenheight))      // detect windowed fullscreen
				{
					m_BallStretchX *= scalebackMonitorX*c + scalebackMonitorY*s;
					m_BallStretchY *= scalebackMonitorY*c + scalebackMonitorX*s;
				}
				break;
	}

	m_hwnd = ::CreateWindowEx(windowflagsex, "VPPlayer", "Visual Pinball Player", windowflags, x, y, m_width, m_height, NULL, NULL, g_hinst, 0);

    mixer_init( m_hwnd );
    hid_init();

	SetCursorPos( 400, 999999 ); // ShowCursor(false)?
}

void Player::UltraNudgeX(const int x, const int j )
{
	m_curAccel_x[j] = x;
}

void Player::UltraNudgeY(const int y, const int j )
{
	m_curAccel_y[j] = y;
}

#define GetUltraNudgeX() (((F32)m_curAccel_x[0]) * (F32)(2.0 / (JOYRANGEMX-JOYRANGEMN))) // Get the -1.0f to +1.0f values from joystick input tilt sensor / ushock
#define GetUltraNudgeY() (((F32)m_curAccel_y[0]) * (F32)(2.0 / (JOYRANGEMX-JOYRANGEMN)))

#if 0
int Player::UltraNudgeGetTilt()
{
	static U32 last_tilt_time;
	static U32 last_jolt_time;

	if( !m_fAccelerometer || m_NudgeManual >= 0 ||	         //disabled or in joystick test mode
	     m_tilt_amount == 0 || m_jolt_amount == 0) return 0; //disabled

	const U32 ms = msec();

	U32 tilt_2 = 0;
	for(int j = 0; j < m_pininput.e_JoyCnt; ++j)	//find largest value
		{
		tilt_2 = max(tilt_2,(U32)(m_curAccel_x[j] * m_curAccel_x[j] + m_curAccel_y[j] * m_curAccel_y[j])); //always postive numbers
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
#endif

void Player::UltraNudge_update()	// called on every integral physics frame
{
	if (m_NudgeManual >= 0)			// Only one joystick controls in manual mode
	{		
		m_NudgeX = m_AccelMAmp * ((float)m_curAccel_x[m_NudgeManual])*(float)(1.0/JOYRANGE); // * Manual Gain
		m_NudgeY = m_AccelMAmp * ((float)m_curAccel_y[m_NudgeManual])*(float)(1.0/JOYRANGE);

		if (m_ptable->m_tblMirrorEnabled)
			m_NudgeX = -m_NudgeX;

		return;
	}	

	m_NudgeX = 0;	// accumlate over joysticks, these acceleration values are used in update ball velocity calculations
	m_NudgeY = 0;	// and are required to be acceleration values (not velocity or displacement)

	if(!m_fAccelerometer) return;	// uShock is disabled 

	//rotate to match hardware mounting orentation, including left or right coordinates
	const float cna = cosf(m_AccelAngle);
	const float sna = sinf(m_AccelAngle);

	for(int j = 0; j < m_pininput.e_JoyCnt; ++j)
	{		
		float dx = ((float)m_curAccel_x[j])*(float)(1.0/JOYRANGE);		// norm range -1 .. 1	
		const float dy = ((float)m_curAccel_y[j])*(float)(1.0/JOYRANGE);	
		if ( m_ptable->m_tblMirrorEnabled )
			dx = -dx;
		m_NudgeX += m_AccelAmpX*(dx*cna + dy*sna) * (1.0f - nudge_get_sensitivity());  //calc Green's transform component for X
		const float nugY = m_AccelAmpY*(dy*cna - dx*sna) * (1.0f - nudge_get_sensitivity()); // calc Green transform component for Y...
		m_NudgeY = m_AccelNormalMount ? (m_NudgeY + nugY) : (m_NudgeY - nugY);	// add as left or right hand coordinate system
	}
}

#define IIR_Order 4

// coefficients for IIR_Order Butterworth filter set to 10 Hz passband
const float a [IIR_Order+1] = {
	0.0048243445f,
	0.019297378f,	
	0.028946068f,
	0.019297378f,
	0.0048243445f};

const float b [IIR_Order+1] = {
	1.00000000f, //if not 1 add division below
	-2.369513f,
	2.3139884f,
	-1.0546654f,
	0.1873795f};

void Player::UltraPlunger_update()	// called on every integral physics frame
{	
	static int init = IIR_Order;	// first time call
	static float x [IIR_Order+1] = {0,0,0,0,0};
	static float y [IIR_Order+1] = {0,0,0,0,0};	

	//http://www.dsptutor.freeuk.com/IIRFilterDesign/IIRFilterDesign.html  
	// (this applet is set to 8000Hz sample rate, therefore, multiply ...
	// our values by 80 to shift sample clock of 100hz to 8000hz)

	if (m_movedPlunger < 3) 
	{
		//int init = IIR_Order;
		m_curMechPlungerPos = 0;
		return;	// not until a real value is entered
	}

	if (!c_plungerFilter)
	{ 
		m_curMechPlungerPos = (float)m_curPlunger;
		return;
	}

	x[0] = (float)m_curPlunger; //initialize filter
	do
	{
		y[0] = a[0]*x[0];	  // initial

		for (int i = IIR_Order; i > 0; --i) // all terms but the zero-th 
		{
			y[0] += (a[i]*x[i] - b[i]*y[i]);// /b[0]; always one     // sum terms from high to low
			x[i] = x[i-1];		//shift 
			y[i] = y[i-1];		//shift
		}
	} while (init--); //loop until all registers are initialized with the first input

	init = 0;

	m_curMechPlungerPos = y[0];
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
	const float tmp = (g_pplayer->m_curMechPlungerPos < 0) ? g_pplayer->m_curMechPlungerPos*m_parkPosition : g_pplayer->m_curMechPlungerPos*(1.0f - m_parkPosition);
	return tmp/range + m_parkPosition;		//scale and offset
}

void Player::mechPlungerIn(const int z)
{
	m_curPlunger = -z; //axis reversal

	if (++m_movedPlunger == 0x7ffffff) m_movedPlunger = 3; //restart at 3
}

void Player::SetGravity(float slopeDeg, float strength)
{
    m_gravity.x = 0;
    m_gravity.y =  sinf(ANGTORAD(slopeDeg)) * strength;
    m_gravity.z = -cosf(ANGTORAD(slopeDeg)) * strength;
}

//++++++++++++++++++++++++++++++++++++++++

#define STATICCNTS 10

void Player::PhysicsSimulateCycle(float dtime) // move physics forward to this time
{
	float hittime;
	int StaticCnts = STATICCNTS;	// maximum number of static counts

	// it's okay to have this code outside of the inner loop, as the ball hitrects already include the maximum distance they can travel in that timespan
    m_hitoctree_dynamic.Update();

	while (dtime > 0.f)
	{
		// first find hits, if any +++++++++++++++++++++ 
#ifdef _DEBUGPHYSICS
		c_timesearch++;
#endif
		hittime = dtime;	//begin time search from now ...  until delta ends

		for (unsigned i = 0; i < m_vball.size(); i++)
		{
			Ball * const pball = m_vball[i];

			if (!pball->fFrozen && pball->m_fDynamic > 0) // don't play with frozen balls
			{
				pball->m_coll.hittime = hittime;		// search upto current hittime
				pball->m_coll.obj = NULL;

                m_hitoctree_dynamic.HitTestBall(pball, pball->m_coll);  // dynamic objects
				m_hitoctree.HitTestBall(pball, pball->m_coll);  // find the hit objects and hit times

				const float htz = pball->m_coll.hittime;// this ball's hit time
				if(htz < 0.f) pball->m_coll.obj = NULL;	// no negative time allowed

				if (pball->m_coll.obj)					// hit object
				{
#ifdef _DEBUGPHYSICS
					++c_hitcnts;						// stats for display

					if (pball->m_coll.hitRigid && pball->m_coll.distance < -0.0875f) //rigid and embedded
						++c_embedcnts;
#endif
					///////////////////////////////////////////////////////////////////////////
					if (htz <= hittime)					//smaller hit time??
					{
						hittime = htz;					// record actual event time

						if (htz < STATICTIME)			// less than static time interval
						{ 
							if(!pball->m_coll.hitRigid) hittime = STATICTIME; // non-rigid ... set Static time
							else if (--StaticCnts < 0)		
							{
								StaticCnts = 0;			// keep from wrapping
								hittime = STATICTIME;		
							}
						}
					}
				}
			}				
		}

		// hittime now set ... or full frame if no hit 
		// now update displacements to collide-contact or end of physics frame
		// !!!!! 2) move objects to hittime

		if (hittime > STATICTIME) StaticCnts = STATICCNTS;		 // allow more zeros next round

		for (unsigned i=0; i<m_vmover.size(); i++)
			m_vmover[i]->UpdateDisplacements(hittime); //step 2:  move the objects about according to velocities

		//  find balls that need to be collided and script'ed (generally there will be one, but more are possible)

		for (unsigned i=0; i < m_vball.size(); i++)					 // use m_vball.size(), in case script deletes a ball
		{
			Ball * const pball = m_vball[i];

			if (pball->m_fDynamic > 0 && pball->m_coll.obj && pball->m_coll.hittime <= hittime) // find balls with hit objects and minimum time			
			{
				// now collision, contact and script reactions on active ball (object)+++++++++
				HitObject * const pho = pball->m_coll.obj;// object that ball hit in trials
				m_pactiveball = pball;				 // For script that wants the ball doing the collision
#ifdef _DEBUGPHYSICS
				c_collisioncnt++;
#endif
				pho->Collide(&pball->m_coll);        //!!!!! 3) collision on active ball
				pball->m_coll.obj = NULL;			 // remove trial hit object pointer

				// Collide may have changed the velocity of the ball, 
				// and therefore the bounding box for the next hit cycle
				if ( m_vball[i] != pball) // Ball still exists? may have been deleted from list
				{
					// collision script deleted the ball, back up one count
                    --i;
                    continue;
				}
				else
				{
					pball->CalcHitRect();		// do new boundings 

					// is this ball static? .. set static and quench	
					if (pball->m_coll.hitRigid && pball->m_coll.distance < (float)PHYS_TOUCH) //rigid and close distance contacts
					{
#ifdef _DEBUGPHYSICS
						c_contactcnt++;
#endif
						const float mag = pball->vel.x*pball->vel.x + pball->vel.y*pball->vel.y; // values below are taken from simulation
						if (pball->drsq < 8.0e-5f && mag < 1.0e-3f && fabsf(pball->vel.z) < 0.2f)
						{
							if(--pball->m_fDynamic <= 0)						//... ball static, cancels next gravity increment
							{													// m_fDynamic is cleared in ball gravity section
								pball->m_fDynamic = 0;
#ifdef _DEBUGPHYSICS
								c_staticcnt++;
#endif
								pball->vel.x = pball->vel.y = pball->vel.z = 0.f;		//quench the remaing velocity and set ...
							}
						}
					}
				}
			}
		}

		dtime -= hittime;	//new delta .. i.e. time remaining

	} // end physics loop
}

U32 phys_iterations;
#ifdef FPS
U64 phys_period = 0;	
#endif

void Player::UpdatePhysics()
{
	const U64 initial_time_usec = usec();

	if (m_fNoTimeCorrect) // After debugging script
	{
		// Shift whole game foward in time
		m_StartTime_usec += initial_time_usec - m_curPhysicsFrameTime;
		m_nextPhysicsFrameTime += initial_time_usec - m_curPhysicsFrameTime;
		m_curPhysicsFrameTime = initial_time_usec; // 0 time frame
		m_fNoTimeCorrect = fFalse;
	}

#ifdef STEPPING
#ifndef EVENTIME
	if (m_fDebugWindowActive || m_fUserDebugPaused)
	{
		// Shift whole game foward in time
		m_StartTime_usec += initial_time_usec - m_curPhysicsFrameTime;
		m_nextPhysicsFrameTime += initial_time_usec - m_curPhysicsFrameTime;
		if (m_fStep)
		{
			// Walk one physics step foward
			m_curPhysicsFrameTime = initial_time_usec - PHYSICS_STEPTIME;
			m_fStep = false;
		}
		else
		{
			m_curPhysicsFrameTime = initial_time_usec; // 0 time frame
		}
	}
#endif
#endif

#ifdef EVENTIME
	if (!m_fPause || m_fStep)
	{
		initial_time_usec = m_curPhysicsFrameTime - 3547811060 + 3547825450;
		m_fStep = false;
	}
	else
	{
		initial_time_usec = m_curPhysicsFrameTime;
	}
#endif

	// Get time in milliseconds for timers
	m_time_msec = (U32)((initial_time_usec - m_StartTime_usec)/1000);

	phys_iterations = 0;
#ifdef FPS
	//if (m_fShowFPS)
	{
        m_lastFrameDuration = initial_time_usec - m_lastTime_usec;
        if (m_lastFrameDuration > 1000000)
            m_lastFrameDuration = 0;
        m_lastTime_usec = initial_time_usec;

		m_cframes++;
		if ((m_time_msec - m_lastfpstime) > 1000)
		{
			m_fps = m_cframes * 1000.0f / (m_time_msec - m_lastfpstime);
            m_lastfpstime = m_time_msec;
            m_fpsAvg += m_fps;
            m_fpsCount++;
			m_cframes = 0;
		}
	}

    phys_period = initial_time_usec;	
#endif

#ifdef LOG
	const double timepassed = (double)(initial_time_usec - m_curPhysicsFrameTime) / 1000000.0;

	const float frametime =
#ifdef PLAYBACK
		(!m_fPlayback) ? (float)(timepassed * 100.0) : ParseLog((LARGE_INTEGER*)&initial_time_usec, (LARGE_INTEGER*)&m_nextPhysicsFrameTime);
#else
 #define TIMECORRECT 1
 #ifdef TIMECORRECT
		(float)(timepassed * 100.0);
		// 1.456927f;
 #else
		0.45f;
 #endif
#endif //PLAYBACK

	fprintf(m_flog, "Frame Time %.20f %u %u %u %u\n", frametime, initial_time_usec>>32, initial_time_usec, m_nextPhysicsFrameTime>>32, m_nextPhysicsFrameTime);
	fprintf(m_flog, "End Frame\n");
#endif

	while (m_curPhysicsFrameTime < initial_time_usec) // loop here until current (real) time matches the physics (simulated) time
	{
		phys_iterations++;

		// Get the time until the next physics tick is done, and get the time
		// until the next frame is done
		// If the frame is the next thing to happen, update physics to that
		// point next update acceleration, and continue loop

		const float physics_diff_time =       (float)((double)(m_nextPhysicsFrameTime - m_curPhysicsFrameTime)*(1.0/PHYSICS_STEPTIME));
		const float physics_to_graphic_diff_time = (float)((double)(initial_time_usec - m_curPhysicsFrameTime)*(1.0/PHYSICS_STEPTIME));

		if (physics_to_graphic_diff_time < physics_diff_time)		 // is graphic frame time next???
		{
			PhysicsSimulateCycle(physics_to_graphic_diff_time);      // advance physics to this time
			m_curPhysicsFrameTime = initial_time_usec;				 // now current to the wall clock
			break;	//this is the common exit from the loop			 // exit skipping accelerate
		}			// some rare cases will exit from while()

		const U64 cur_time_usec = usec();
		if ((cur_time_usec - initial_time_usec > 200000) || (phys_iterations > ((m_ptable->m_PhysicsMaxLoops == 0) ? 0xFFFFFFFFu : m_ptable->m_PhysicsMaxLoops/*2*/))) // hung in the physics loop over 200 milliseconds or the number of physics iterations to catch up on is high (i.e. very low/unplayable FPS)
		{															 // can not keep up to real time
			m_curPhysicsFrameTime = initial_time_usec;				 // skip physics forward ... slip-cycles -> 'slowed' down physics
			m_nextPhysicsFrameTime = initial_time_usec + PHYSICS_STEPTIME;
			break;													 // go draw frame
		}

		//update keys, hid, plumb, nudge, timers, etc
        //const U32 sim_msec = (U32)(m_curPhysicsFrameTime/1000);
		const U32 cur_time_msec = (U32)(cur_time_usec/1000);

		m_pininput.ProcessKeys(m_ptable/*, sim_msec*/, cur_time_msec);

		mixer_update(m_pininput);
		hid_update(/*sim_msec*/cur_time_msec);
        plumb_update(/*sim_msec*/cur_time_msec, GetUltraNudgeX(), GetUltraNudgeY());

#ifdef ACCURATETIMERS
		m_pactiveball = NULL; // No ball is the active ball for timers/key events

		const int p_timeCur = (int)((m_curPhysicsFrameTime - m_StartTime_usec)/1000); // milliseconds

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
		UltraNudge_update();		// physics_diff_time is the balance of time to move from the graphic frame position to the next
		UltraPlunger_update();		// integral physics frame. So the previous graphics frame was (1.0 - physics_diff_time) before 
									// this integral physics frame. Accelerations and inputs are always physics frame aligned
		if (m_nudgetime)
		{
			m_nudgetime--;

			if (m_nudgetime == 5)
			{
				m_NudgeX = -m_NudgeBackX * 2.0f;
				m_NudgeY =  m_NudgeBackY * 2.0f;
			}
			else if (m_nudgetime == 0)
			{
				m_NudgeX =  m_NudgeBackX;
				m_NudgeY = -m_NudgeBackY;
			}
		}

		for (unsigned i=0; i<m_vmover.size(); i++)
			m_vmover[i]->UpdateVelocities();	// always on integral physics frame boundary

		//primary physics loop
		PhysicsSimulateCycle(physics_diff_time);	    // main simulator call

		//ball trail, keep old pos of balls
		for (unsigned i=0; i < m_vball.size(); i++)
		{
			Ball * const pball = m_vball[i];
            pball->oldpos[pball->ringcounter_oldpos] = pball->pos;

			pball->ringcounter_oldpos++;
			if(pball->ringcounter_oldpos == 10)
				pball->ringcounter_oldpos = 0;
		}

		//slintf( "PT: %f %f %u %u %u\n", physics_diff_time, physics_to_graphic_diff_time, (U32)(m_curPhysicsFrameTime/1000), (U32)(initial_time_usec/1000), cur_time_msec );

		m_curPhysicsFrameTime = m_nextPhysicsFrameTime;				 // new cycle, on physics frame boundary
		m_nextPhysicsFrameTime += PHYSICS_STEPTIME;					 // advance physics position
	} // end while (m_curPhysicsFrameTime < initial_time_usec)

#ifdef FPS
	phys_period = usec() - phys_period;
#endif
}

template <typename T>
struct IsMemberOf
{
    IsMemberOf(const std::vector<T>& _v) : v(_v) { }

    bool operator()(const T& val) const
    { return std::binary_search(v.begin(), v.end(), val); }

    const std::vector<T>& v;
};

void Player::RenderDynamics()
{
    TRACE_FUNCTION();

   // Start rendering the next frame.
   m_pin3d.m_pd3dDevice->BeginScene();

   m_pin3d.m_pd3dDevice->SetRenderState( RenderDevice::NORMALIZENORMALS, m_ptable->m_NormalizeNormals );

   // Check if we are debugging balls
   if (m_ToggleDebugBalls)
   {
      // Check if we are debugging balls
      if (m_DebugBalls)
      {
         //HRESULT ReturnCode;
         // Set the render state to something that will always display.
         m_pin3d.m_pd3dDevice->SetRenderState ( RenderDevice::ZENABLE, D3DZB_FALSE );
         m_pin3d.m_pd3dDevice->SetRenderState ( RenderDevice::ALPHABLENDENABLE, FALSE );
      }
      else
      {
         // Restore the render state.
         m_pin3d.m_pd3dDevice->SetRenderState ( RenderDevice::ZENABLE, D3DZB_TRUE );
         m_pin3d.m_pd3dDevice->SetRenderState ( RenderDevice::ALPHABLENDENABLE, TRUE );
      }

      m_ToggleDebugBalls = fFalse;
   }

   /* VP9COMPAT:
    * Most VP9 tables use several lights layered over each other in order to simulate
    * fading lights and GI. They rely on the fact that when a light in VP9 changes
    * state, it is blitted to the backbuffer. We emulate this by putting lights which
    * change state to the end of the draw order for lights.
    */
   {
       const unsigned numLights = m_vLights.size();

       // sort the list of triggered lights so that we can use binary search
       m_sortedTriggeredLights = m_triggeredLights;
       std::sort( m_sortedTriggeredLights.begin(), m_sortedTriggeredLights.end() );

       // remove triggered lights from the m_vLights vector
       m_vLights.erase(
         std::remove_if( m_vLights.begin(), m_vLights.end(), IsMemberOf<Hitable*>(m_sortedTriggeredLights) ),
         m_vLights.end() );

       // now re-add the triggered lights at the end in the proper order,
       // but make sure to add each only once
       m_alreadyAddedLights.clear();
       for (unsigned i = 0; i < m_triggeredLights.size(); ++i)
       {
           Hitable * cur = m_triggeredLights[i];
           if (m_alreadyAddedLights.find(cur) == m_alreadyAddedLights.end())       // not yet added?
           {
               m_vLights.push_back(cur);
               m_alreadyAddedLights.insert(cur);
           }
       }

       // reset list of triggered lights
       m_triggeredLights.clear();

       assert( numLights == m_vLights.size() );     // make sure we didn't mess up
   }

   // Draw non-transparent objects.
   for (unsigned i=0; i < m_vHitNonTrans.size(); ++i)
       m_vHitNonTrans[i]->PostRenderStatic(m_pin3d.m_pd3dDevice);

   // Draw Light objects (VP9COMPAT)
   for (unsigned i=0; i < m_vLights.size(); ++i)
       m_vLights[i]->PostRenderStatic(m_pin3d.m_pd3dDevice);

   // Draw backglass objects (VP9COMPAT)
   for (unsigned i=0; i < m_vHitBackglass.size(); ++i)
       m_vHitBackglass[i]->PostRenderStatic(m_pin3d.m_pd3dDevice);

   DrawBalls();

   m_limiter.Execute(m_pin3d.m_pd3dDevice);

   // Draw transparent objects.
   for (unsigned i=0; i < m_vHitTrans.size(); ++i)
       m_vHitTrans[i]->PostRenderStatic(m_pin3d.m_pd3dDevice);

   // Draw the mixer volume.
   mixer_draw();
   // Debug draw of plumb
   plumb_draw();

   m_pin3d.m_pd3dDevice->SetRenderState( RenderDevice::NORMALIZENORMALS, TRUE );

   // Finish rendering the next frame.
   m_pin3d.m_pd3dDevice->EndScene();
}


void Player::CheckAndUpdateRegions()
{
    //
    // copy static buffers to back buffer and z buffer
    //

    RenderTarget* backBuffer = m_pin3d.m_pddsBackBuffer;

    RenderTarget* const backBufferZ = m_pin3d.m_pddsZBuffer;

    // blit static background
    m_pin3d.m_pd3dDevice->CopySurface(backBuffer,  m_pin3d.m_pddsStatic );
    m_pin3d.m_pd3dDevice->CopySurface(backBufferZ, m_pin3d.m_pddsStaticZ);

    // Process all AnimObjects
    for (int l=0; l < m_vscreenupdate.Size(); ++l)
    {
        m_vscreenupdate.ElementAt(l)->Check3D();
    }
}


void Player::FlipVideoBuffersNormal( const bool vsync )
{
	if ( m_nudgetime &&			// Table is being nudged.
		 m_ptable->m_Shake )	// The "EarthShaker" effect is active. //!! make configurable (cab vs desktop)
	{
		// Draw with an offset to shake the display.
        // TODO: this doesn't work in DX9, have to handle shake some other way.
		m_pin3d.Flip( /*(int)m_NudgeBackX, (int)m_NudgeBackY,*/ vsync);
	}
    else
        m_pin3d.Flip(vsync);
}

static const float quadVerts[4*5] =
{
  1.0f, 1.0f,0.0f,1.0f,0.0f,
 -1.0f, 1.0f,0.0f,0.0f,0.0f,
  1.0f,-1.0f,0.0f,1.0f,1.0f,
 -1.0f,-1.0f,0.0f,0.0f,1.0f
};

void Player::FlipVideoBuffers3DFXAA( const bool vsync ) //!! SMAA, luma sharpen, dither
{
	const bool stereo = ((m_fStereo3D != 0) && m_fStereo3Denabled);
	const bool FXAA1 = (((m_fFXAA == 1) && (m_ptable->m_useFXAA == -1)) || (m_ptable->m_useFXAA == 1));

	m_pin3d.m_pd3dDevice->CreatePixelShader( stereo ? stereo3Dshader : (FXAA1 ? FXAAshader1 : FXAAshader2) );

	m_pin3d.m_pd3dDevice->CopySurface(m_pin3d.m_pdds3DBackBuffer, m_pin3d.m_pddsBackBuffer);
	if(stereo)
		m_pin3d.m_pd3dDevice->CopyDepth(m_pin3d.m_pdds3DZBuffer, m_pin3d.m_pddsZBuffer);

    m_pin3d.m_pd3dDevice->BeginScene();

	m_pin3d.m_pd3dDevice->SetRenderState(RenderDevice::CULLMODE, D3DCULL_NONE);
    m_pin3d.m_pd3dDevice->SetRenderState(RenderDevice::ZWRITEENABLE, FALSE);
    m_pin3d.m_pd3dDevice->SetRenderState(RenderDevice::LIGHTING, FALSE);
    m_pin3d.m_pd3dDevice->SetRenderState(RenderDevice::ZENABLE, FALSE);

	m_pin3d.m_pd3dDevice->SetTexture(0,m_pin3d.m_pdds3DBackBuffer);
	m_pin3d.m_pd3dDevice->SetTextureFilter(0, TEXTURE_MODE_BILINEAR);
    m_pin3d.m_pd3dDevice->SetTextureAddressMode(0, RenderDevice::TEX_CLAMP);
	if(stereo)
	{
		m_pin3d.m_pd3dDevice->SetTexture(1,m_pin3d.m_pdds3DZBuffer);
		m_pin3d.m_pd3dDevice->SetTextureFilter(1, TEXTURE_MODE_POINT); //!! TEXTURE_MODE_BILINEAR?
	    m_pin3d.m_pd3dDevice->SetTextureAddressMode(1, RenderDevice::TEX_CLAMP);
	}

	Matrix3D matWorld, matView, matProj;
	m_pin3d.m_pd3dDevice->GetTransform( TRANSFORMSTATE_WORLD,      &matWorld );
	m_pin3d.m_pd3dDevice->GetTransform( TRANSFORMSTATE_VIEW,       &matView );
	m_pin3d.m_pd3dDevice->GetTransform( TRANSFORMSTATE_PROJECTION, &matProj );

	Matrix3D ident;
	ident.SetIdentity();
	m_pin3d.m_pd3dDevice->SetTransform( TRANSFORMSTATE_WORLD,      &ident );
	m_pin3d.m_pd3dDevice->SetTransform( TRANSFORMSTATE_VIEW,       &ident );
	m_pin3d.m_pd3dDevice->SetTransform( TRANSFORMSTATE_PROJECTION, &ident );

	//m_pin3d.m_pd3dDevice->SetPixelShader();
	if(!stereo)
	{
		const float temp[4] = {(float)(1.0/(double)m_width), (float)(1.0/(double)m_height), 0.f, 0.f};
		m_pin3d.m_pd3dDevice->SetPixelShaderConstants(temp,1);
	}
	else
	{
		const float temp[8] = {m_ptable->GetMaxSeparation(), m_ptable->GetZPD(), m_fStereo3DY ? 1.0f : 0.0f, (m_fStereo3D == 1) ? 1.0f : 0.0f,
						       (float)(1.0/(double)m_width), (float)(1.0/(double)m_height), (float)m_height, m_fStereo3DAA ? 1.0f : 0.0f /*1.0f+(float)(usec()&0x1FF)*(float)(1.0/0x1FF)*/}; //!!
		m_pin3d.m_pd3dDevice->SetPixelShaderConstants(temp,2);
	}
    m_pin3d.m_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, MY_D3DFVF_TEX, (LPVOID)quadVerts, 4);
	
	m_pin3d.m_pd3dDevice->RevertPixelShaderToFixedFunction();

	m_pin3d.m_pd3dDevice->SetTransform( TRANSFORMSTATE_WORLD,      &matWorld );
	m_pin3d.m_pd3dDevice->SetTransform( TRANSFORMSTATE_VIEW,       &matView );
	m_pin3d.m_pd3dDevice->SetTransform( TRANSFORMSTATE_PROJECTION, &matProj );

	m_pin3d.m_pd3dDevice->SetTexture(0,NULL);
	m_pin3d.m_pd3dDevice->SetTextureFilter(0, TEXTURE_MODE_TRILINEAR );
	if(stereo)
	{
		m_pin3d.m_pd3dDevice->SetTexture(1,NULL);
		m_pin3d.m_pd3dDevice->SetTextureFilter(1, TEXTURE_MODE_TRILINEAR );
	}

	m_pin3d.m_pd3dDevice->SetRenderState(RenderDevice::ZENABLE, TRUE);
    m_pin3d.m_pd3dDevice->SetRenderState(RenderDevice::LIGHTING, TRUE);
    m_pin3d.m_pd3dDevice->SetRenderState(RenderDevice::CULLMODE, D3DCULL_CCW);
	m_pin3d.m_pd3dDevice->SetRenderState(RenderDevice::ZWRITEENABLE, TRUE);

	m_pin3d.m_pd3dDevice->EndScene();

	m_pin3d.m_pd3dDevice->Flip(vsync);
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void Player::Render()
{
	U64 timeforframe = usec();

	if(m_firstFrame)
	{
		const HWND hVPMWnd = FindWindow( "MAME", NULL );
		if (hVPMWnd != NULL)
		{
			if(IsWindowVisible( hVPMWnd ))
				SetWindowPos ( hVPMWnd, HWND_TOPMOST, 0, 0, 0, 0, (SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW | SWP_NOACTIVATE) ); // in some strange cases the vpinmame window is not on top, so enforce it
		}
	}

    if (m_sleeptime > 0)
    {
        Sleep(m_sleeptime - 1);
    }

#ifdef _DEBUGPHYSICS
	c_hitcnts = 0;
	c_collisioncnt = 0;
	c_contactcnt = 0;
	c_staticcnt = 0;
	c_embedcnts = 0;
	c_timesearch = 0;

	c_traversed = 0;
    c_tested = 0;
    c_deepTested = 0;
#endif

    ///+++++++++++++++++++++++++++++++++++++++++++++++++++++

    UpdatePhysics();

    m_LastKnownGoodCounter++;

    CheckAndUpdateRegions();
    RenderDynamics();

    // Check if we should turn animate the plunger light.
    hid_set_output ( HID_OUTPUT_PLUNGER, ((m_time_msec - m_LastPlungerHit) < 512) && ((m_time_msec & 512) > 0) );

    int localvsync = (m_ptable->m_TableAdaptiveVSync == -1) ? m_fVSync : m_ptable->m_TableAdaptiveVSync;
	if(localvsync > m_refreshrate)
		localvsync = 0;

    bool vsync = false;
    if(localvsync > 0)
    {
        if(localvsync == 1) // legacy auto-detection
        {
            if(m_fps > m_refreshrate*ADAPT_VSYNC_FACTOR)
                vsync = true;
        }
        else
            if(m_fps > localvsync*ADAPT_VSYNC_FACTOR)
                vsync = true;
    }


    if((((m_fStereo3D == 0) || !m_fStereo3Denabled) && (!((m_fFXAA && (m_ptable->m_useFXAA == -1)) || (m_ptable->m_useFXAA > 0)))) || (m_ptable->GetMaxSeparation() <= 0.0f) || (m_ptable->GetMaxSeparation() >= 1.0f) || (m_ptable->GetZPD() <= 0.0f) || (m_ptable->GetZPD() >= 1.0f) || !m_pin3d.m_pdds3DBackBuffer || !m_pin3d.m_pdds3DZBuffer)
    {
        FlipVideoBuffersNormal( vsync );
    }
    else
    {
        FlipVideoBuffers3DFXAA( vsync );
    }

#ifndef ACCURATETIMERS
    m_pactiveball = NULL;  // No ball is the active ball for timers/key events

    for (int i=0;i<m_vht.Size();i++)
    {
        HitTimer * const pht = m_vht.ElementAt(i);
        if (pht->m_nextfire <= m_time_msec)
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

    // Update music stream
    if (m_pxap)
    {
        if (!m_pxap->Tick())
        {
            delete m_pxap;
            m_pxap = NULL;
            m_ptable->FireVoidEvent(DISPID_GameEvents_MusicDone);
        }
    }

    for (unsigned i=0; i<m_vballDelete.size(); i++)
    {
        Ball * const pball = m_vballDelete[i];
        delete pball->m_vpVolObjs;
        delete pball;
    }

    m_vballDelete.clear();

	m_firstFrame = false;

#ifdef FPS
    if (m_fShowFPS)
    {
        HDC hdcNull = GetDC(NULL);
        char szFoo[128];

        // Draw the amount of video memory used.
        // Disabled in DX9 until we can compute this correctly.
        int len = sprintf_s(szFoo, " Used Graphics Memory: %.2f MB ", (float)NumVideoBytes/(float)(1024*1024));
        // TextOut(hdcNull, 10, 30, szFoo, len);

        // Draw the framerate.
        const float fpsAvg = (m_fpsCount == 0) ? 0.0f : m_fpsAvg/m_fpsCount;
        int len2 = sprintf_s(szFoo, " FPS: %.1f FPS(avg): %.1f", m_fps, fpsAvg);
        if( len2>=0 )
        {
            for(int l = len2; l < len+1; ++l)
                szFoo[l] = ' ';
            TextOut(hdcNull, 10, 10, szFoo, len);
        }

        const U64 period = m_lastFrameDuration;
        if( period > m_max || m_time_msec - m_lastMaxChangeTime > 1000)
        {
            m_max = period;
            m_lastMaxChangeTime = m_time_msec;
        }

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

#ifdef DEBUG_FPS
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
                len = sprintf_s( szFoo, " %d ", period[i] );
                TextOut( hdcNull,  20 + i * 20, 10 + period[i], szFoo, len );
            }
        }
#endif

		len = sprintf_s(szFoo, sizeof(szFoo), "period: %.1f ms (%.1f avg %.1f max)      ",
		  float(1e-3f*period), float(1e-3f*(m_total/m_count)), float(1e-3f*m_max) );
		TextOut(hdcNull, 10, 30, szFoo, len);

        // performance counters
		len = sprintf_s(szFoo, sizeof(szFoo), "Draw calls: %u      ", m_pin3d.m_pd3dDevice->Perf_GetNumDrawCalls());
		TextOut(hdcNull, 10, 65, szFoo, len);
		len = sprintf_s(szFoo, sizeof(szFoo), "State changes: %u      ", m_pin3d.m_pd3dDevice->Perf_GetNumStateChanges());
		TextOut(hdcNull, 10, 85, szFoo, len);
		len = sprintf_s(szFoo, sizeof(szFoo), "Texture changes: %u      ", m_pin3d.m_pd3dDevice->Perf_GetNumTextureChanges());
		TextOut(hdcNull, 10, 105, szFoo, len);

#ifdef _DEBUGPHYSICS
		len = sprintf_s(szFoo, sizeof(szFoo), "physTimes %10u uS(%12u avg %12u max)    ",
			   	(U32)phys_period,
			   	(U32)(m_phys_total / m_count),
			   	(U32)m_phys_max );
		TextOut(hdcNull, 10, 140, szFoo, len);

		len = sprintf_s(szFoo, sizeof(szFoo), "phys:%5u iterations(%5u avg %5u max))   ",
			   	phys_iterations,
			   	(U32)( m_phys_total_iterations / m_count ),
				(U32)m_phys_max_iterations );
		TextOut(hdcNull, 10, 160, szFoo, len);

		len = sprintf_s(szFoo, sizeof(szFoo), "Hits:%5u Collide:%5u Ctacs:%5u Static:%5u Embed:%5u TimeSearch:%5u    ",
		c_hitcnts, c_collisioncnt, c_contactcnt, c_staticcnt, c_embedcnts, c_timesearch);
		TextOut(hdcNull, 10, 180, szFoo, len);

		len = sprintf_s(szFoo, sizeof(szFoo), "Octree:%5u Traversed:%5u Tested:%5u DeepTested:%5u  ",
		c_octNextlevels,c_traversed,c_tested,c_deepTested);
		TextOut(hdcNull, 10, 200, szFoo, len);
#endif
        ReleaseDC(NULL, hdcNull);
    }
#endif

    if ((m_PauseTimeTarget > 0) && (m_PauseTimeTarget <= m_time_msec))
    {
        m_PauseTimeTarget = 0;
        m_fUserDebugPaused = true;
        RecomputePseudoPauseState();
        SendMessage(m_hwndDebugger, RECOMPUTEBUTTONCHECK, 0, 0);
    }

	// limit framerate if requested by user (vsync Hz higher than refreshrate of gfxcard/monitor)
	localvsync = (m_ptable->m_TableAdaptiveVSync == -1) ? m_fVSync : m_ptable->m_TableAdaptiveVSync;
	if(localvsync > m_refreshrate)
	{
		timeforframe = usec() - timeforframe;
		if(timeforframe < 1000000ull/localvsync)
			uSleep(1000000ull/localvsync-timeforframe);
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
                option = DialogBox(g_hinst, MAKEINTRESOURCE(IDD_GAMEPAUSE), m_hwnd, PauseProc);
            }
            else //m_fCloseType == all others
            {
                option = ID_QUIT;
                SendMessage(g_pvp->m_hwnd, WM_COMMAND, ID_FILE_EXIT, NULL );
            }

            m_fCloseDown = fFalse;
            m_fNoTimeCorrect = fTrue; // Skip the time we were in the dialog
            UnpauseMusic();
            if (option == ID_QUIT)
            {
                SendMessage(m_hwnd, WM_CLOSE, 0, 0); // This line returns to the editor after exiting a table

                //unload VPM - works first time, crashes after rendering animations next time vpm is loaded by script
                /*HMODULE hmod;
                  do {
                  hmod=GetModuleHandle("VPinMAME.dll");
                  } while(hmod != NULL && FreeLibrary(hmod));*/
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


void Player::DrawBalls()
{
   bool drawReflection = ((m_fReflectionForBalls && (m_ptable->m_useReflectionForBalls == -1)) || (m_ptable->m_useReflectionForBalls == 1));

   // m_pin3d.m_pd3dDevice->SetRenderState(RenderDevice::TEXTUREPERSPECTIVE, FALSE ); // this is always on in DX9
   m_pin3d.m_pd3dDevice->SetTextureAddressMode(0, RenderDevice::TEX_CLAMP);
   m_pin3d.m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE);
   m_pin3d.m_pd3dDevice->SetTextureFilter(0, TEXTURE_MODE_TRILINEAR);
   Matrix3D matWorld = g_pplayer->m_pin3d.GetWorldTransform();

   const float inclination = ANGTORAD(g_pplayer->m_ptable->m_inclination);

   const float sn = sinf(inclination);
   const float cs = cosf(inclination);
   const float inv_tablewidth = 1.0f/(m_ptable->m_right - m_ptable->m_left);
   const float inv_tableheight = 1.0f/(m_ptable->m_bottom - m_ptable->m_top);

   m_pin3d.m_pd3dDevice->SetVertexDeclaration( g_pBallVertexDeclaration );

   for (unsigned i=0; i<m_vball.size(); i++)
   {
      Ball * const pball = m_vball[i];
      // just calculate the vertices once!
      float zheight = (!pball->fFrozen) ? pball->pos.z : (pball->pos.z - pball->radius);

      float maxz = pball->defaultZ+3.0f;
      float minz = pball->defaultZ;
      if((m_fReflectionForBalls && (m_ptable->m_useReflectionForBalls == -1)) || (m_ptable->m_useReflectionForBalls == 1))
      {
         // don't draw reflection if the ball is not on the playfield (e.g. on a ramp/kicker)
         if( (zheight > maxz) || (pball->fFrozen) || pball->pos.z<minz)
            drawReflection=false;
         else
            drawReflection=true;
      }
      if( (zheight > maxz) || (pball->pos.z < minz) )
      {
         // scaling the ball height by the z scale value results in a flying ball over the playfield/ramp
         // by reducing it with 0.96f (a factor found by trial'n error) the ball is on the ramp again
         if ( m_ptable->m_zScale!=1.0f )
            zheight *= (m_ptable->m_zScale*0.96f); 
      }

      const float radiusX = (pball->radius*0.9f) * m_BallStretchX;
      const float radiusY = (pball->radius*0.9f) * m_BallStretchY;

      // this has to be moved to the shader as well
      Vertex3D_NoTex2 newBall[basicBallNumVertices];
      float minY=40000000.0f;
      float maxY=-40000000.0f;
      for( unsigned int i=0; i<basicBallNumVertices; i++ )
      {
         Vertex3Ds vec(basicBall[i].x,basicBall[i].y,basicBall[i].z);
         Vertex3Ds nvec(basicBall[i].nx,basicBall[i].ny,basicBall[i].nz);
         const Vertex3Ds tmp = pball->m_orientation.MultiplyVector(vec);
         nvec=pball->m_orientation.MultiplyVector(nvec);
         newBall[i].x = pball->pos.x + tmp.x*pball->radius*0.9f;
         newBall[i].y = pball->pos.y + tmp.y*pball->radius*0.9f;
         newBall[i].z = zheight + tmp.z*(pball->radius*0.9f);
         newBall[i].nx = nvec.x;
         newBall[i].ny = nvec.y;
         newBall[i].nz = nvec.z;
         newBall[i].tu = basicBall[i].tu;
         newBall[i].tv = basicBall[i].tv;
         if( newBall[i].y>maxY) maxY=newBall[i].y;
         if( newBall[i].y<minY) minY=newBall[i].y;
      }

      // prepare the vertex buffer for all possible options (ball,logo,shadow)
       Vertex3D_NoTex2 *buf;
       Ball::vertexBuffer->lock(0, 0, (void**)&buf, VertexBuffer::WRITEONLY | VertexBuffer::DISCARDCONTENTS);
       memcpy( buf, newBall, sizeof(Vertex3D_NoTex2)*basicBallNumVertices );
       Ball::vertexBuffer->unlock();

      pball->logoMaterial.setDiffuse( 0.8f, pball->m_color );
      pball->logoMaterial.setAmbient( 0.8f, pball->m_color );
      pball->material.setColor( 1.0f, pball->m_color );
      m_pin3d.m_pd3dDevice->SetMaterial(pball->material);

      Texture * const playfield = m_ptable->GetImage((char *)m_ptable->m_szImage);
      if( playfield )
      {
          ballShader->Core()->SetTexture("Texture1",m_pin3d.m_pd3dDevice->m_texMan.LoadTexture(playfield->m_pdsBuffer));
      }

      if(pball->m_disableLighting)
      {
         m_pin3d.m_pd3dDevice->SetRenderState(RenderDevice::TEXTUREFACTOR, COLORREF_to_D3DCOLOR(pball->m_color));
         m_pin3d.m_pd3dDevice->SetTextureStageState(ePictureTexture, D3DTSS_COLORARG2, D3DTA_TFACTOR); // do not modify tex by diffuse lighting
      }

      // ************************* draw the ball itself ****************************
      m_pin3d.EnableAlphaBlend(1);
      ballShader->Core()->SetFloat("posX", pball->pos.x );
      ballShader->Core()->SetFloat("posY", pball->pos.y );
      ballShader->Core()->SetFloat("sizeY", maxY-minY );
      ballShader->Core()->SetFloat("invTableWidth", inv_tablewidth );
      ballShader->Core()->SetFloat("invTableHeight", inv_tableheight );
      if ( !pball->m_pin )
          ballShader->Core()->SetTexture("Texture0",m_pin3d.m_pd3dDevice->m_texMan.LoadTexture(m_pin3d.ballTexture.m_pdsBufferColorKey));
      else
          ballShader->Core()->SetTexture("Texture0",m_pin3d.m_pd3dDevice->m_texMan.LoadTexture(pball->m_pin->m_pdsBufferColorKey));

      if( pball->m_pinFront )
      {
          pball->m_pinFront->CreateAlphaChannel();          
          ballShader->Core()->SetTexture("Texture2",m_pin3d.m_pd3dDevice->m_texMan.LoadTexture(pball->m_pinFront->m_pdsBufferColorKey));
      }
      UINT cPasses=0;
      if ( drawReflection )
      {
          ballShader->Core()->SetFloat("reflectionStrength", (float)m_ptable->m_ballReflectionStrength/255.0f );
          m_pin3d.m_pd3dDevice->SetRenderState(RenderDevice::ZWRITEENABLE, FALSE);
          m_pin3d.m_pd3dDevice->SetRenderState(RenderDevice::SRCBLEND,  D3DBLEND_SRCALPHA);
          m_pin3d.m_pd3dDevice->SetRenderState(RenderDevice::DESTBLEND, D3DBLEND_DESTALPHA);
          ballShader->Core()->SetTechnique("RenderBallReflection");
          ballShader->Core()->Begin(&cPasses,0);
          ballShader->Core()->BeginPass(0);
          m_pin3d.m_pd3dDevice->DrawIndexedPrimitiveVB( D3DPT_TRIANGLELIST, pball->vertexBuffer, 0, basicBallNumVertices, ballIndexBuffer, 0, basicBallNumFaces );
          ballShader->Core()->EndPass();
          ballShader->Core()->End();

          m_pin3d.m_pd3dDevice->SetRenderState(RenderDevice::ZWRITEENABLE, TRUE);
          m_pin3d.m_pd3dDevice->SetRenderState(RenderDevice::DESTBLEND, D3DBLEND_INVSRCALPHA);
      }

      if ( m_ptable->m_rotation!=270.0f )
      {
          ballShader->Core()->SetTechnique("RenderBall_Desktop");
      }
      else
      {
          ballShader->Core()->SetTechnique("RenderBall_Cabinet");
      }
      cPasses=0;
      ballShader->Core()->Begin(&cPasses,0);
      ballShader->Core()->BeginPass(0);
      m_pin3d.m_pd3dDevice->DrawIndexedPrimitiveVB( D3DPT_TRIANGLELIST, pball->vertexBuffer, 0, basicBallNumVertices, ballIndexBuffer, 0, basicBallNumFaces );
      ballShader->Core()->EndPass();
      ballShader->Core()->End();

      if(pball->m_disableLighting)
      {
         m_pin3d.m_pd3dDevice->SetRenderState(RenderDevice::TEXTUREFACTOR, 0xffffffff);
         m_pin3d.m_pd3dDevice->SetTextureStageState(ePictureTexture, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
      }

      // ball trails //!! misses lighting disabled part!
      if( ((m_fTrailForBalls && (m_ptable->m_useTrailForBalls == -1)) || (m_ptable->m_useTrailForBalls == 1)) && m_fBallAntialias )
      {
         Vertex3D_NoLighting rgv3D_all[10*2];
         unsigned int num_rgv3D = 0;

         for(int i2 = 0; i2 < 10-1; ++i2)
         {
            int i = pball->ringcounter_oldpos-1-i2;
            if(i<0)
               i += 10;
            int io = i-1;
            if(io<0)
               io += 10;

            if((pball->oldpos[i].x != FLT_MAX) && (pball->oldpos[io].x != FLT_MAX)) // only if already initialized
            {
               Vertex3Ds vec;
               vec.x = pball->oldpos[io].x-pball->oldpos[i].x;
               vec.y = pball->oldpos[io].y-pball->oldpos[i].y;
               vec.z = pball->oldpos[io].z-pball->oldpos[i].z;
               const unsigned int bc = (unsigned int)((float)m_ptable->m_ballTrailStrength * powf(1.f-1.f/max(vec.Length(), 1.0f), 16.0f)); //!! 16=magic alpha falloff
               const float r = min(pball->radius*0.9f, 2.0f*pball->radius/powf((float)(i2+2), 0.6f)); //!! consts are for magic radius falloff

               if(bc > 0 && r > FLT_MIN)
               {
                  Vertex3Ds v = vec;
                  v.Normalize();
                  Vertex3Ds up;
                  up.x = 0.f;
                  up.y = 0.f;
                  up.z = 1.f;
                  Vertex3Ds n = CrossProduct(v,up);
                  n.x *= r;
                  n.y *= r;
                  n.z *= r;

                  Vertex3D_NoLighting rgv3D[4];
                  rgv3D[0].x = pball->oldpos[i].x - n.x;
                  rgv3D[0].y = pball->oldpos[i].y - n.y;
                  rgv3D[0].z = pball->oldpos[i].z - n.z;
                  rgv3D[1].x = pball->oldpos[i].x + n.x;
                  rgv3D[1].y = pball->oldpos[i].y + n.y;
                  rgv3D[1].z = pball->oldpos[i].z + n.z;
                  rgv3D[2].x = pball->oldpos[io].x + n.x;
                  rgv3D[2].y = pball->oldpos[io].y + n.y;
                  rgv3D[2].z = pball->oldpos[io].z + n.z;
                  rgv3D[3].x = pball->oldpos[io].x - n.x;
                  rgv3D[3].y = pball->oldpos[io].y - n.y;
                  rgv3D[3].z = pball->oldpos[io].z - n.z;

                  rgv3D[0].color = rgv3D[1].color = rgv3D[2].color = rgv3D[3].color = bc | (bc<<8) | (bc<<16) | (bc<<24);

                  rgv3D[0].tu = 0.5f+(float)(i2)*(float)(1.0/(2.0*(10-1)));
                  rgv3D[0].tv = 0.f;
                  rgv3D[1].tu = rgv3D[0].tu;
                  rgv3D[1].tv = 1.f;
                  rgv3D[2].tu = 0.5f+(float)(i2+1)*(float)(1.0/(2.0*(10-1)));
                  rgv3D[2].tv = 1.f;
                  rgv3D[3].tu = rgv3D[2].tu;
                  rgv3D[3].tv = 0.f;

                  if(num_rgv3D == 0)
                  {
                     rgv3D_all[0] = rgv3D[0];
                     rgv3D_all[1] = rgv3D[1];
                     rgv3D_all[2] = rgv3D[3];
                     rgv3D_all[3] = rgv3D[2];
                  }
                  else
                  {
                     rgv3D_all[num_rgv3D-2].x = (rgv3D[0].x+rgv3D_all[num_rgv3D-2].x)*0.5f;
                     rgv3D_all[num_rgv3D-2].y = (rgv3D[0].y+rgv3D_all[num_rgv3D-2].y)*0.5f;
                     rgv3D_all[num_rgv3D-2].z = (rgv3D[0].z+rgv3D_all[num_rgv3D-2].z)*0.5f;
                     rgv3D_all[num_rgv3D-1].x = (rgv3D[1].x+rgv3D_all[num_rgv3D-1].x)*0.5f;
                     rgv3D_all[num_rgv3D-1].y = (rgv3D[1].y+rgv3D_all[num_rgv3D-1].y)*0.5f;
                     rgv3D_all[num_rgv3D-1].z = (rgv3D[1].z+rgv3D_all[num_rgv3D-1].z)*0.5f;
                     rgv3D_all[num_rgv3D] = rgv3D[3];
                     rgv3D_all[num_rgv3D+1] = rgv3D[2];
                  }

                  if(num_rgv3D == 0)
                     num_rgv3D += 4;
                  else
                     num_rgv3D += 2;
               }
            }
         }

         static const WORD rgi_all[10*2] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19};

         if(num_rgv3D > 0)
         {
            m_pin3d.m_pd3dDevice->SetRenderState(RenderDevice::ZWRITEENABLE, FALSE);
            m_pin3d.m_pd3dDevice->SetRenderState(RenderDevice::SRCBLEND,  D3DBLEND_SRCALPHA);
            m_pin3d.m_pd3dDevice->SetRenderState(RenderDevice::DESTBLEND, D3DBLEND_DESTALPHA);
            m_pin3d.m_pd3dDevice->SetRenderState(RenderDevice::LIGHTING, FALSE);

            m_pin3d.m_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, MY_D3DFVF_NOLIGHTING_VERTEX, rgv3D_all, num_rgv3D, (LPWORD)rgi_all, num_rgv3D);

            m_pin3d.m_pd3dDevice->SetRenderState(RenderDevice::LIGHTING, TRUE);
            m_pin3d.m_pd3dDevice->SetRenderState(RenderDevice::ZWRITEENABLE, TRUE);
            m_pin3d.m_pd3dDevice->SetRenderState(RenderDevice::DESTBLEND, D3DBLEND_INVSRCALPHA);
         }
      }


#ifdef _DEBUGPHYSICS        // draw debug points for visualizing ball rotation
      if (m_fShowFPS)
      {
         // set transform
         Matrix3D matOrig, matNew, matRot;
         matOrig = m_pin3d.GetWorldTransform();
         matNew.SetTranslation(pball->pos);
         matOrig.Multiply(matNew, matNew);
         matRot.SetIdentity();
         for (int j = 0; j < 3; ++j)
            for (int k = 0; k < 3; ++k)
               matRot.m[j][k] = pball->m_orientation.m_d[k][j];
         matNew.Multiply(matRot, matNew);
         m_pin3d.m_pd3dDevice->SetTransform(TRANSFORMSTATE_WORLD, &matNew);

         // draw points
         m_pin3d.SetTexture(NULL);
         float ptsize = 5.0f;
         m_pin3d.m_pd3dDevice->SetRenderState((RenderDevice::RenderStates)D3DRS_POINTSIZE, *((DWORD*)&ptsize));
         m_pin3d.m_pd3dDevice->SetRenderState(RenderDevice::LIGHTING, FALSE);
         m_pin3d.m_pd3dDevice->DrawPrimitiveVB( D3DPT_POINTLIST, m_ballDebugPoints, 0, 12 );
         m_pin3d.m_pd3dDevice->SetRenderState(RenderDevice::LIGHTING, TRUE);

         // reset transform
         m_pin3d.m_pd3dDevice->SetTransform(TRANSFORMSTATE_WORLD, &matOrig);
      }
#endif

   }   // end loop over all balls

   m_pin3d.m_pd3dDevice->SetTexture(0, NULL);
   m_pin3d.DisableAlphaBlend();
}

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
	if (m_vdebugho.Size() == 0)
		{
		// First time the debug hit-testing has been used
		InitDebugHitStructure();
		}

	Matrix3D mat3D = m_pin3d.m_proj.m_matrixTotal;
	mat3D.Invert();

	ViewPort vp;
	m_pin3d.m_pd3dDevice->GetViewport( &vp );
	const float rClipWidth  = (float)vp.Width*0.5f;
	const float rClipHeight = (float)vp.Height*0.5f;

	const float xcoord = ((float)x-rClipWidth)/rClipWidth;
	const float ycoord = (-((float)y-rClipHeight))/rClipHeight;

	// Use the inverse of our 3D transform to determine where in 3D space the
	// screen pixel the user clicked on is at.  Get the point at the near
	// clipping plane (z=0) and the far clipping plane (z=1) to get the whole
	// range we need to hit test
	Vertex3Ds v3d, v3d2;
	mat3D.MultiplyVector(Vertex3Ds(xcoord,ycoord,0), v3d);
	mat3D.MultiplyVector(Vertex3Ds(xcoord,ycoord,1), v3d2);

	// Create a ray (ball) that travels in 3D space from the screen pixel at
	// the near clipping plane to the far clipping plane, and find what
	// it intersects with.
	Ball ballT;
    ballT.pos = v3d;
	ballT.vel = v3d2 - v3d;
	ballT.radius = 0;
	ballT.m_coll.hittime = 1.0f;
	ballT.CalcHitRect();

	//const float slope = (v3d2.y - v3d.y)/(v3d2.z - v3d.z);
	//const float yhit = v3d.y - (v3d.z*slope);

	//const float slopex = (v3d2.x - v3d.x)/(v3d2.z - v3d.z);
	//const float xhit = v3d.x - (v3d.z*slopex);

	Vector<HitObject> vhoHit;
	Vector<IFireEvents> vpfe;

    m_hitoctree_dynamic.HitTestXRay(&ballT, &vhoHit, ballT.m_coll);
	m_hitoctree.HitTestXRay(&ballT, &vhoHit, ballT.m_coll);
	m_debugoctree.HitTestXRay(&ballT, &vhoHit, ballT.m_coll);

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
			m_pactiveball = m_pactiveballDebug;
			pfe->FireGroupEvent(dispid);
			m_pactiveball = NULL;
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

#ifdef ULTRAPIN
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
#endif

LRESULT CALLBACK PlayerWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
		{
        case MM_MIXM_CONTROL_CHANGE:
            mixer_get_volume();
            break;

		case WM_CLOSE:
#ifdef ULTRAPIN
			// Close the DMD hack window.
			SendMessage(g_pplayer->m_dmdhackhwnd, WM_CLOSE, 0, 0);
#endif
			break;

		case WM_DESTROY:
      {
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
            PinTable * const playedTable = g_pplayer->m_ptable;
			g_pplayer->m_ptable->StopPlaying();
			g_pplayer->m_pininput.UnInit();

			delete g_pplayer; // needs to be deleted here, as code below relies on it being NULL
			g_pplayer = NULL;

			g_pvp->SetEnableToolbar();
			g_pvp->SetEnableMenuItems();
            mixer_shutdown();
            hid_shutdown();
            // modification to m_vedit of each table after playing them must be done here, otherwise VP will crash (WTF?!)
            playedTable->RestoreLayers();

			break;
      }

		case WM_KEYDOWN:
			g_pplayer->m_fDrawCursor = false;
			SetCursor(NULL);
			break;

		case WM_MOUSEMOVE:
			if (g_pplayer->m_lastcursorx != LOWORD(lParam) || g_pplayer->m_lastcursory != HIWORD(lParam))
				{
				g_pplayer->m_fDrawCursor = true;
				g_pplayer->m_lastcursorx = LOWORD(lParam);
				g_pplayer->m_lastcursory = HIWORD(lParam);
				}
			break;

#ifdef STEPPING
#ifdef MOUSEPAUSE
		case WM_LBUTTONDOWN:
			if (g_pplayer->m_fPause)
				{
				g_pplayer->m_fStep = true;
				}
			break;

		case WM_RBUTTONDOWN:
			if (!g_pplayer->m_fPause)
				{
				g_pplayer->m_fPause = true;

				g_pplayer->m_fGameWindowActive = false;
				g_pplayer->RecomputePauseState();
				g_pplayer->RecomputePseudoPauseState();
				}
			else
				{
				g_pplayer->m_fPause = false;

				g_pplayer->m_fGameWindowActive = true;
				SetCursor(NULL);
				g_pplayer->m_fNoTimeCorrect = fTrue;
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

		case WM_POINTERDOWN:
		case WM_POINTERUP:
			{
#ifndef TEST_TOUCH_WITH_MOUSE
				if(!GetPointerInfo)
					GetPointerInfo = (pGPI) GetProcAddress(GetModuleHandle(TEXT("user32.dll")),
						                                   "GetPointerInfo");
				if(GetPointerInfo)
#endif
				{
					POINTER_INFO pointerInfo;
#ifdef TEST_TOUCH_WITH_MOUSE
                    GetCursorPos(&pointerInfo.ptPixelLocation);
#else
					if (GetPointerInfo(GET_POINTERID_WPARAM(wParam), &pointerInfo))
#endif
					{
						ScreenToClient(g_pplayer->m_hwnd, &pointerInfo.ptPixelLocation);
						for (unsigned int i = 0; i < 8; ++i)
							if((g_pplayer->m_touchregion_pressed[i] != (uMsg == WM_POINTERDOWN)) && Intersect(touchregion[i], g_pplayer->m_width, g_pplayer->m_height, pointerInfo.ptPixelLocation, fmodf(g_pplayer->m_ptable->m_rotation,360.0f) != 0.f))
							{
								g_pplayer->m_touchregion_pressed[i] = (uMsg == WM_POINTERDOWN);

								DIDEVICEOBJECTDATA didod;
								didod.dwOfs = g_pplayer->m_rgKeys[touchkeymap[i]];
								didod.dwData = g_pplayer->m_touchregion_pressed[i] ? 0x80 : 0;
								g_pplayer->m_pininput.PushQueue( &didod, APP_KEYBOARD/*, curr_time_msec*/ );
							}
					}
				}
			}
			break;

		case WM_ACTIVATE:
#ifdef VBA
			g_pvp->ApcHost.WmActivate(wParam);
			break;

		case WM_ENABLE:
			g_pvp->ApcHost.WmEnable(wParam);
#else	
			if (wParam != WA_INACTIVE)
				{
				g_pplayer->m_fGameWindowActive = true;
				SetCursor(NULL);
				g_pplayer->m_fNoTimeCorrect = fTrue;
				g_pplayer->m_fPause = false;
				}
			else
				{
				g_pplayer->m_fGameWindowActive = false;
				g_pplayer->m_fPause = true;
				}
			g_pplayer->RecomputePauseState();
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
								g_pplayer->m_Coins += lParam;
								ReturnCode = TRUE;
							}
							else
							{
								// Print an error.
#ifdef _DEBUG
								OutputDebugString ( "Autocoin: Invalid parameter." );
#endif
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

INT_PTR CALLBACK DebuggerProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
							delete [] szText;
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
				g_pplayer->m_fUserDebugPaused = false;
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
								g_pplayer->m_fUserDebugPaused = false;
								g_pplayer->RecomputePseudoPauseState();
								SendMessage(hwndDlg, RECOMPUTEBUTTONCHECK, 0, 0);
								break;

							case IDC_PAUSE:
								g_pplayer->m_PauseTimeTarget = 0;
								g_pplayer->m_fUserDebugPaused = true;
								g_pplayer->RecomputePseudoPauseState();
								SendMessage(hwndDlg, RECOMPUTEBUTTONCHECK, 0, 0);
								break;

							case IDC_STEP:
								{
								int ms = GetDlgItemInt(hwndDlg, IDC_STEPAMOUNT, NULL, FALSE);
								g_pplayer->m_PauseTimeTarget = g_pplayer->m_time_msec + ms;
								g_pplayer->m_fUserDebugPaused = false;
								g_pplayer->RecomputePseudoPauseState();
								SendMessage(hwndDlg, RECOMPUTEBUTTONCHECK, 0, 0);
								}
								break;

							case IDC_EXPAND:
								SendMessage(hwndDlg, RESIZE_FROM_EXPAND, 0, 0);
								break;
                     case IDC_BALL_THROWING:
                        {
                           HWND hwndControl;
                           int checked;
                           hwndControl = GetDlgItem(hwndDlg, IDC_BALL_THROWING);
                           checked = SendMessage(hwndControl, BM_GETCHECK, 0, 0);
                           g_pplayer->m_fThrowBalls = checked;
                           break;
                        }
							}
					}
				break;
			}

	return FALSE;
}

INT_PTR CALLBACK PauseProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
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

#ifdef DONGLE_SUPPORT
int get_dongle_status()
{
	//Initialize.
	int Status = DONGLE_STATUS_NOTFOUND;

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
}
#endif

#ifdef ULTRAPIN
// Performs special draws... ok, hacks!
// These are to address shortcomings in not having
// a proper backglass display that can be animated.
void Player::DrawLightHack ()
{
	// Check the state of all lights.
	for ( int i=0; i<LIGHTHACK_MAX; i++ )
	{
		if ( m_LightHackReadyForDrawLightHackFn[i] )
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
