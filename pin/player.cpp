#include "stdafx.h"
#include <algorithm>
#include "../meshes/ballMesh.h"
#include "BallShader.h"

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
	POINTER_FLAG_NONE = 0x00000000,
	POINTER_FLAG_NEW = 0x00000001,
	POINTER_FLAG_INRANGE = 0x00000002,
	POINTER_FLAG_INCONTACT = 0x00000004,
	POINTER_FLAG_FIRSTBUTTON = 0x00000010,
	POINTER_FLAG_SECONDBUTTON = 0x00000020,
	POINTER_FLAG_THIRDBUTTON = 0x00000040,
	POINTER_FLAG_OTHERBUTTON = 0x00000080,
	POINTER_FLAG_PRIMARY = 0x00000100,
	POINTER_FLAG_CONFIDENCE = 0x00000200,
	POINTER_FLAG_CANCELLED = 0x00000400,
	POINTER_FLAG_DOWN = 0x00010000,
	POINTER_FLAG_UPDATE = 0x00020000,
	POINTER_FLAG_UP = 0x00040000,
	POINTER_FLAG_WHEEL = 0x00080000,
	POINTER_FLAG_HWHEEL = 0x00100000,
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

static const float quadVerts[4*5] =
{
  1.0f, 1.0f,0.0f,1.0f,0.0f,
 -1.0f, 1.0f,0.0f,0.0f,0.0f,
  1.0f,-1.0f,0.0f,1.0f,1.0f,
 -1.0f,-1.0f,0.0f,0.0f,1.0f
};

//

#define RECOMPUTEBUTTONCHECK WM_USER+100
#define RESIZE_FROM_EXPAND WM_USER+101


LRESULT CALLBACK PlayerWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

INT_PTR CALLBACK PauseProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DebuggerProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

Player::Player(bool _cameraMode) : cameraMode(_cameraMode)
{
	{
		int regs[4];
		__cpuid(regs, 1);
		// check for SSE and exit if not available, as some code relies on it by now
		if ((regs[3] & 0x002000000) == 0) { // NO SSE?
			ShowError("SSE is not supported on this processor");
			exit(0);
		}
		// disable denormalized floating point numbers, can be faster on some CPUs (and VP doesn't need to rely on denormals)
		if ((regs[3] & 0x004000000) != 0) // SSE2?
			_mm_setcsr(_mm_getcsr() | 0x8040); // flush denorms to zero and also treat incoming denorms as zeros
		else
			_MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON); // only flush denorms to zero
	}

	m_fPause = false;
	m_fStep = false;
	m_fPseudoPause = false;
	m_pauseRefCount = 0;
	m_fNoTimeCorrect = false;
	m_firstFrame = true;

	m_fThrowBalls = false;
#ifdef PLAYBACK
	m_fPlayback = fFalse;

	m_fplaylog = NULL;
#endif

#ifdef LOG
	m_flog = NULL;
#endif

	for (int i = 0; i < PININ_JOYMXCNT; ++i) {
		m_curAccel_x[i] = 0;
		m_curAccel_y[i] = 0;
	}

	m_sleeptime = 0;

	m_pxap = NULL;
	m_pactiveball = NULL;

	m_curPlunger = JOYRANGEMN - 1;

	m_current_renderstage = 0;

	HRESULT hr;

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

    float nudgeStrength;
    hr = GetRegStringAsFloat("Player", "NudgeStrength", &nudgeStrength);
    if (hr != S_OK)
        nudgeStrength = 2e-2f; // The default
    m_NudgeShake = nudgeStrength;

        hr = GetRegInt("Player", "FXAA", &m_fFXAA);
    if (hr != S_OK)
      m_fFXAA = 0; // The default = off

    int trailballs;
    hr = GetRegInt("Player", "BallTrail", &trailballs);
    if (hr != S_OK)
      m_fTrailForBalls = true; // The default = on
    else
        m_fTrailForBalls = (trailballs == 1);

    int reflballs;
        hr = GetRegInt("Player", "BallReflection", &reflballs);
    if (hr != S_OK)
      m_fReflectionForBalls = true; // The default = on
    else
        m_fReflectionForBalls = (reflballs == 1);

    int AA;
    hr = GetRegInt("Player", "USEAA", &AA);
    if (hr != S_OK)
      m_fAA = false; // The default = off
    else
        m_fAA = (AA == 1);

    int AO;
    hr = GetRegInt("Player", "USEAO", &AO);
    if (hr != S_OK)
		m_fAO = false; // The default = off
	else
		m_fAO = (AO == 1);

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

	m_fShowFPS = false;

	m_fCloseDown = false;
	m_fCloseType = 0;

	m_DebugBalls = false;
	m_ToggleDebugBalls = false;

	m_swap_ball_collision_handling = false;

	m_fDebugMode = false;
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

#ifdef _DEBUGPHYSICS
	c_hitcnts = 0;
	c_collisioncnt = 0;
	c_contactcnt = 0;
	c_staticcnt = 0;
	c_embedcnts = 0;
	c_timesearch = 0;

	c_octNextlevels = 0;
	c_quadNextlevels = 0;

	c_traversed = 0;
	c_tested = 0;
	c_deepTested = 0;
#endif

	m_movedPlunger = 0;
	m_LastPlungerHit = 0;

	for (unsigned int i = 0; i < 8; ++i)
		m_touchregion_pressed[i] = false;

#ifdef DEBUG_FPS
	ToggleFPS();
#endif

	m_fRecordContacts = false;
	m_contacts.reserve(8);

	m_overall_frames = 0;

	m_dmdx = 0;
	m_dmdy = 0;
	m_texdmd = NULL;
	m_device_texdmd = NULL;
	backdropSettingActive = 0;

	m_ScreenOffset = Vertex2D(0, 0);

	ballIndexBuffer = NULL;
	ballVertexBuffer = NULL;
	m_ballDebugPoints = NULL;
}

Player::~Player()
{
}

void Player::Shutdown()
{
    m_pininput.UnInit();

    SAFE_RELEASE(ballVertexBuffer);
    SAFE_RELEASE(ballIndexBuffer);
    if (ballShader)
    {
        ballShader->Core()->SetTexture("Texture0",NULL);
        ballShader->Core()->SetTexture("Texture1",NULL);
        ballShader->Core()->SetTexture("Texture2",NULL);
        delete ballShader;
        ballShader=0;
    }
#ifdef DEBUG_BALL_SPIN
    SAFE_RELEASE(m_ballDebugPoints);
#endif

    m_limiter.Shutdown();

	for (unsigned i = 0; i < m_vhitables.size(); ++i)
		m_vhitables[i]->EndPlay();

	for (int i = 0; i < m_vho.Size(); i++)
		delete m_vho.ElementAt(i);
	m_vho.RemoveAllElements();

	for (int i = 0; i < m_vdebugho.Size(); i++)
		delete m_vdebugho.ElementAt(i);
	m_vdebugho.RemoveAllElements();

	//!! cleanup the whole mem management for balls, this is a mess!

	// balls are added to the octree, but not the hit object vector
	for (unsigned i = 0; i < m_vball.size(); i++)
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
	//      delete m_vho_dynamic.ElementAt(i);
	//m_vho_dynamic.RemoveAllElements();

	m_vball.clear();

	m_dmdx = 0;
	m_dmdy = 0;
	if (m_texdmd)
	{
		m_pin3d.m_pd3dDevice->DMDShader->SetTexture("Texture0", (D3DTexture*)NULL);
		m_pin3d.m_pd3dDevice->m_texMan.UnloadTexture(m_texdmd);
		delete m_texdmd;
		m_texdmd = NULL;
	}
	m_device_texdmd = NULL;

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

	for (int i = 0; i < m_controlclsidsafe.Size(); i++)
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
		m_fNoTimeCorrect = true;
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

	plineseg = new LineSeg(Vertex2D(m_ptable->m_right,m_ptable->m_top),Vertex2D(m_ptable->m_right,m_ptable->m_bottom),m_ptable->m_tableheight,m_ptable->m_glassheight);
	plineseg->m_pfe = NULL;
	pvho->AddElement(plineseg);

	plineseg = new LineSeg(Vertex2D(m_ptable->m_left,m_ptable->m_bottom),Vertex2D(m_ptable->m_left,m_ptable->m_top),m_ptable->m_tableheight,m_ptable->m_glassheight);
	plineseg->m_pfe = NULL;
	pvho->AddElement(plineseg);

	plineseg = new LineSeg(Vertex2D(m_ptable->m_right,m_ptable->m_bottom),Vertex2D(m_ptable->m_left,m_ptable->m_bottom),m_ptable->m_tableheight,m_ptable->m_glassheight);
	plineseg->m_pfe = NULL;
	pvho->AddElement(plineseg);

	plineseg = new LineSeg(Vertex2D(m_ptable->m_left,m_ptable->m_top),Vertex2D(m_ptable->m_right,m_ptable->m_top),m_ptable->m_tableheight,m_ptable->m_glassheight);
	plineseg->m_pfe = NULL;
	pvho->AddElement(plineseg);

	Vertex3Ds * const rgv3D = new Vertex3Ds[4];
	rgv3D[0] = Vertex3Ds(m_ptable->m_left, m_ptable->m_top, m_ptable->m_glassheight);
	rgv3D[1] = Vertex3Ds(m_ptable->m_right, m_ptable->m_top, m_ptable->m_glassheight);
	rgv3D[2] = Vertex3Ds(m_ptable->m_right, m_ptable->m_bottom, m_ptable->m_glassheight);
	rgv3D[3] = Vertex3Ds(m_ptable->m_left, m_ptable->m_bottom, m_ptable->m_glassheight);

	Hit3DPoly * const ph3dpoly = new Hit3DPoly(rgv3D, 4); //!!

	pvho->AddElement(ph3dpoly);

	m_hitPlayfield = HitPlane(Vertex3Ds(0, 0, 1), m_ptable->m_tableheight);
	m_hitPlayfield.SetFriction(m_ptable->m_friction);
	m_hitPlayfield.m_elasticity = m_ptable->m_elasticity;
	m_hitPlayfield.m_elasticityFalloff = m_ptable->m_elasticityFalloff;
	m_hitPlayfield.m_scatter = m_ptable->m_scatter;

	m_hitTopGlass = HitPlane(Vertex3Ds(0, 0, -1), m_ptable->m_glassheight);
	m_hitTopGlass.SetFriction(0.3f);
	m_hitTopGlass.m_elasticity = 0.2f;
}

void Player::InitKeys()
{
	HRESULT hr;
	int key;
	hr = GetRegInt("Player", "LFlipKey", &key);
	if (hr != S_OK || key > 0xdd)
	{
		key = DIK_LSHIFT;
	}
	m_rgKeys[eLeftFlipperKey] = (EnumAssignKeys)key;

	hr = GetRegInt("Player", "RFlipKey", &key);
	if (hr != S_OK || key > 0xdd)
	{
		key = DIK_RSHIFT;
	}
	m_rgKeys[eRightFlipperKey] = (EnumAssignKeys)key;

	hr = GetRegInt("Player", "LTiltKey", &key);
	if (hr != S_OK || key > 0xdd)
	{
		key = DIK_Z;
	}
	m_rgKeys[eLeftTiltKey] = (EnumAssignKeys)key;

	hr = GetRegInt("Player", "RTiltKey", &key);
	if (hr != S_OK || key > 0xdd)
	{
		key = DIK_SLASH;
	}
	m_rgKeys[eRightTiltKey] = (EnumAssignKeys)key;

	hr = GetRegInt("Player", "CTiltKey", &key);
	if (hr != S_OK || key > 0xdd)
	{
		key = DIK_SPACE;
	}
	m_rgKeys[eCenterTiltKey] = (EnumAssignKeys)key;

	hr = GetRegInt("Player", "PlungerKey", &key);
	if (hr != S_OK || key > 0xdd)
	{
		key = DIK_RETURN;
	}
	m_rgKeys[ePlungerKey] = (EnumAssignKeys)key;

	hr = GetRegInt("Player", "AddCreditKey", &key);
	if (hr != S_OK || key > 0xdd)
	{
		key = DIK_5;
	}
	m_rgKeys[eAddCreditKey] = (EnumAssignKeys)key;

	hr = GetRegInt("Player", "AddCreditKey2", &key);
	if (hr != S_OK || key > 0xdd)
	{
		key = DIK_4;
	}
	m_rgKeys[eAddCreditKey2] = (EnumAssignKeys)key;

	hr = GetRegInt("Player", "StartGameKey", &key);
	if (hr != S_OK || key > 0xdd)
	{
		key = DIK_1;
	}
	m_rgKeys[eStartGameKey] = (EnumAssignKeys)key;

	hr = GetRegInt("Player", "ExitGameKey", &key);
	if (hr != S_OK || key > 0xdd)
	{
		key = DIK_Q;
	}
	m_rgKeys[eExitGame] = (EnumAssignKeys)key;

	hr = GetRegInt("Player", "FrameCount", &key);
	if (hr != S_OK || key > 0xdd)
	{
		key = DIK_F11;
	}
	m_rgKeys[eFrameCount] = (EnumAssignKeys)key;

	hr = GetRegInt("Player", "VolumeUp", &key);
	if (hr != S_OK || key > 0xdd)
	{
		key = DIK_EQUALS;
	}
	m_rgKeys[eVolumeUp] = (EnumAssignKeys)key;

	hr = GetRegInt("Player", "VolumeDown", &key);
	if (hr != S_OK || key > 0xdd)
	{
		key = DIK_MINUS;
	}
	m_rgKeys[eVolumeDown] = (EnumAssignKeys)key;

	hr = GetRegInt("Player", "DebugBalls", &key);
	if (hr != S_OK || key > 0xdd)
	{
		key = DIK_O;
	}
	m_rgKeys[eDBGBalls] = (EnumAssignKeys)key;

	hr = GetRegInt("Player", "MechTilt", &key);
	if (hr != S_OK || key > 0xdd)
	{
		key = DIK_T;
	}
	m_rgKeys[eMechanicalTilt] = (EnumAssignKeys)key;

	hr = GetRegInt("Player", "RMagnaSave", &key);
	if (hr != S_OK || key > 0xdd)
	{
		key = DIK_RCONTROL;             //157 (0x9D) DIK_RCONTROL        0x9D
	}
	m_rgKeys[eRightMagnaSave] = (EnumAssignKeys)key;

	hr = GetRegInt("Player", "LMagnaSave", &key);
	if (hr != S_OK || key > 0xdd)
	{
		key = DIK_LCONTROL; //29 (0x1D)
	}
	m_rgKeys[eLeftMagnaSave] = (EnumAssignKeys)key;

	hr = GetRegInt("Player", "Enable3DKey", &key);
	if (hr != S_OK || key > 0xdd)
	{
		key = DIK_F10;
	}
	m_rgKeys[eEnable3D] = (EnumAssignKeys)key;
}

void Player::InitRegValues()
{
	HRESULT hr;

	int playmusic;
	hr = GetRegInt("Player", "PlayMusic", &playmusic);
	if (hr != S_OK)
		m_fPlayMusic = true; // default value
	else
		m_fPlayMusic = (playmusic == 1);

	int playsound;
	hr = GetRegInt("Player", "PlaySound", &playsound);
	if (hr != S_OK)
		m_fPlaySound = true; // default value
	else
		m_fPlaySound = (playsound == 1);

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
    // GetDepth approximates direction in view distance to camera; sort ascending
    return h1->GetDepth(g_viewDir) >= h2->GetDepth(g_viewDir);
}

static bool CompareHitableDepthReverse(Hitable* h1, Hitable* h2)
{
    // GetDepth approximates direction in view distance to camera; sort descending
    return h1->GetDepth(g_viewDir) < h2->GetDepth(g_viewDir);
}

static bool CompareHitableMaterial(Hitable* h1, Hitable* h2)
{
    return h1->GetMaterialID() < h2->GetMaterialID();
}

void Player::UpdateBasicShaderMatrix(const Matrix3D& objectTrafo)
{
    D3DMATRIX worldMat;
    D3DMATRIX viewMat;
    D3DMATRIX projMat;
    m_pin3d.m_pd3dDevice->GetTransform(TRANSFORMSTATE_WORLD, &worldMat);
    m_pin3d.m_pd3dDevice->GetTransform(TRANSFORMSTATE_VIEW, &viewMat);
    m_pin3d.m_pd3dDevice->GetTransform(TRANSFORMSTATE_PROJECTION, &projMat);

    D3DXMATRIX matProj(projMat);
    D3DXMATRIX matView(viewMat);
    D3DXMATRIX matWorld(worldMat);
    D3DXMATRIX matObject(objectTrafo);

    if (m_ptable->m_fReflectionEnabled)
    {
        matObject._43 -= m_ptable->m_tableheight*2.0f; // why factor 2.0f?
    }

    D3DXMATRIX matWorldView = matObject * matWorld * matView;
    D3DXMATRIX matWorldViewProj = matWorldView * matProj;

	if (m_ptable->m_tblMirrorEnabled)
	{
		const D3DXMATRIX flipx(-1, 0, 0, 0,
								0, 1, 0, 0,
								0, 0, 1, 0,
								0, 0, 0, 1);
		const D3DXMATRIX flipy( 1, 0, 0, 0,
								0, -1, 0, 0,
								0, 0, 1, 0,
								0, 0, 0, 1);
		const float rotation = fmodf(m_ptable->m_BG_rotation[m_ptable->m_BG_current_set], 360.f);
		matWorldViewProj = matWorldViewProj * (rotation != 0.0f ? flipy : flipx);
	}
    Matrix3D temp;
    memcpy(temp.m,matWorldView.m,4*4*sizeof(float));
    temp.Invert();
    temp.Transpose();
    D3DXMATRIX matWorldViewInvTrans;
    memcpy(matWorldViewInvTrans.m,temp.m,4*4*sizeof(float));
    
    m_pin3d.m_pd3dDevice->basicShader->SetMatrix("matWorldViewProj", &matWorldViewProj);
    m_pin3d.m_pd3dDevice->flasherShader->SetMatrix("matWorldViewProj", &matWorldViewProj);
    m_pin3d.m_pd3dDevice->lightShader->SetMatrix("matWorldViewProj", &matWorldViewProj);
#ifdef SEPARATE_CLASSICLIGHTSHADER
    m_pin3d.m_pd3dDevice->classicLightShader->SetMatrix("matWorldViewProj", &matWorldViewProj);
#endif

    m_pin3d.m_pd3dDevice->basicShader->SetMatrix("matWorldView", &matWorldView);
    m_pin3d.m_pd3dDevice->basicShader->SetMatrix("matWorldViewInverseTranspose", &matWorldViewInvTrans);
    //m_pin3d.m_pd3dDevice->basicShader->SetMatrix("matWorld", &matWorld);
    m_pin3d.m_pd3dDevice->basicShader->SetMatrix("matView", &matView);
#ifdef SEPARATE_CLASSICLIGHTSHADER
    m_pin3d.m_pd3dDevice->classicLightShader->SetMatrix("matWorldView", &matWorldView);
    m_pin3d.m_pd3dDevice->classicLightShader->SetMatrix("matWorldViewInverseTranspose", &matWorldViewInvTrans);
    //m_pin3d.m_pd3dDevice->classicLightShader->SetMatrix("matWorld", &matWorld);
    m_pin3d.m_pd3dDevice->classicLightShader->SetMatrix("matView", &matView);
#endif

    memcpy(temp.m,matView.m,4*4*sizeof(float));
    temp.Invert();
    D3DXMATRIX matViewInv;
    memcpy(matViewInv.m,temp.m,4*4*sizeof(float));

    m_pin3d.m_pd3dDevice->basicShader->SetMatrix("matViewInverse", &matViewInv);
#ifdef SEPARATE_CLASSICLIGHTSHADER
    m_pin3d.m_pd3dDevice->classicLightShader->SetMatrix("matViewInverse", &matViewInv);
#endif
}

void Player::InitShader()
{
   /*D3DMATRIX worldMat;
   D3DMATRIX viewMat;
   D3DMATRIX projMat;
   m_pin3d.m_pd3dDevice->GetTransform(TRANSFORMSTATE_WORLD, &worldMat );
   m_pin3d.m_pd3dDevice->GetTransform(TRANSFORMSTATE_VIEW, &viewMat);
   m_pin3d.m_pd3dDevice->GetTransform(TRANSFORMSTATE_PROJECTION, &projMat);

   D3DXMATRIX matProj(projMat);
   D3DXMATRIX matView(viewMat);
   D3DXMATRIX matWorld(worldMat);
   D3DXMATRIX worldViewProj = matWorld * matView * matProj;*/

   UpdateBasicShaderMatrix();
   //D3DXVECTOR4 cam( worldViewProj._41, worldViewProj._42, worldViewProj._43, 1 );
   //m_pin3d.m_pd3dDevice->basicShader->SetVector("camera", &cam);
#ifdef SEPARATE_CLASSICLIGHTSHADER
   //m_pin3d.m_pd3dDevice->classicLightShader->SetVector("camera", &cam);
#endif

   m_pin3d.m_pd3dDevice->basicShader->SetTexture("Texture1", &m_pin3d.envTexture);
   m_pin3d.m_pd3dDevice->basicShader->SetTexture("Texture2", m_pin3d.m_device_envRadianceTexture);
#ifdef SEPARATE_CLASSICLIGHTSHADER
   m_pin3d.m_pd3dDevice->classicLightShader->SetTexture("Texture1", &m_pin3d.envTexture);
   m_pin3d.m_pd3dDevice->classicLightShader->SetTexture("Texture2", m_pin3d.m_device_envRadianceTexture);
#endif
   const D3DXVECTOR4 st(m_ptable->m_envEmissionScale*m_ptable->m_globalEmissionScale, (float)m_pin3d.envTexture.m_height/*+m_pin3d.envTexture.m_width)*0.5f*/, 0.f,0.f);
   m_pin3d.m_pd3dDevice->basicShader->SetVector("fenvEmissionScale_TexWidth", &st);
#ifdef SEPARATE_CLASSICLIGHTSHADER
   m_pin3d.m_pd3dDevice->classicLightShader->SetVector("fenvEmissionScale_TexWidth", &st);
#endif

   InitBallShader();
}

void Player::UpdateBallShaderMatrix()
{
    D3DMATRIX worldMat;
    D3DMATRIX viewMat;
    D3DMATRIX projMat;
    m_pin3d.m_pd3dDevice->GetTransform(TRANSFORMSTATE_WORLD, &worldMat);
    m_pin3d.m_pd3dDevice->GetTransform(TRANSFORMSTATE_VIEW, &viewMat);
    m_pin3d.m_pd3dDevice->GetTransform(TRANSFORMSTATE_PROJECTION, &projMat);

    D3DXMATRIX matProj(projMat);
    D3DXMATRIX matView(viewMat);
    D3DXMATRIX matWorld(worldMat);

    D3DXMATRIX matWorldView = matWorld * matView;
    D3DXMATRIX matWorldViewProj = matWorldView * matProj;

	if (m_ptable->m_tblMirrorEnabled)
	{
		const D3DXMATRIX flipx(-1, 0, 0, 0,
								0, 1, 0, 0,
								0, 0, 1, 0,
								0, 0, 0, 1);
		const D3DXMATRIX flipy( 1, 0, 0, 0,
								0, -1, 0, 0,
								0, 0, 1, 0,
								0, 0, 0, 1);
		const float rotation = fmodf(m_ptable->m_BG_rotation[m_ptable->m_BG_current_set], 360.f);
		matWorldViewProj = matWorldViewProj * (rotation != 0.f ? flipy : flipx);
	}

    Matrix3D temp;
    memcpy(temp.m,matWorldView.m,4*4*sizeof(float));
    temp.Invert();
    temp.Transpose();
    D3DXMATRIX matWorldViewInvTrans;
    memcpy(matWorldViewInvTrans.m,temp.m,4*4*sizeof(float));

    ballShader->SetMatrix("matWorldViewProj", &matWorldViewProj);
    ballShader->SetMatrix("matWorldView", &matWorldView);
    //ballShader->SetMatrix("matWorldViewInverseTranspose", &matWorldViewInvTrans);
    ballShader->SetMatrix("matView", &matView);

    memcpy(temp.m,matView.m,4*4*sizeof(float));
    temp.Invert();
    D3DXMATRIX matViewInv;
    memcpy(matViewInv.m,temp.m,4*4*sizeof(float));

    ballShader->SetMatrix("matViewInverse", &matViewInv);
}

void Player::InitBallShader()
{
   ballShader = new Shader(m_pin3d.m_pd3dDevice );
#if _MSC_VER >= 1700
   ballShader->Load(g_ballShaderCode, sizeof(g_ballShaderCode));
#else
   ballShader->Load(ballShaderCode, sizeof(ballShaderCode));
#endif

   UpdateBallShaderMatrix();

   const float inv_tablewidth = 1.0f/(m_ptable->m_right - m_ptable->m_left);
   const float inv_tableheight = 1.0f/(m_ptable->m_bottom - m_ptable->m_top);
   //const float inclination = ANGTORAD(m_ptable->m_inclination);

   const D3DXVECTOR4 bs(m_BallStretchX, m_BallStretchY, inv_tablewidth, inv_tableheight);
   ballShader->SetVector("ballStretch_invTableRes", &bs );
   ballShader->SetBool("decalMode", m_ptable->m_BallDecalMode );
   const float rotation = fmodf(m_ptable->m_BG_rotation[m_ptable->m_BG_current_set],360.f);
   ballShader->SetBool("cabMode", rotation!=0.f );

   //D3DXVECTOR4 cam( matView._41, matView._42, matView._43, 1 );
   //ballShader->SetVector("camera", &cam);
   const D3DXVECTOR4 st(m_ptable->m_envEmissionScale*m_ptable->m_globalEmissionScale, (float)m_pin3d.envTexture.m_height/*+m_pin3d.envTexture.m_width)*0.5f*/, 0.f,0.f);
   ballShader->SetVector("fenvEmissionScale_TexWidth",&st);
   //ballShader->SetInt("iLightPointNum",MAX_LIGHT_SOURCES);

   const float Roughness = 0.8f;
   const D3DXVECTOR4 rwem(exp2f(10.0f * Roughness + 1.0f), 0.f, 1.f, 0.0f); // no metal, as ball collects the diffuse playfield which uses this flag!
   ballShader->SetVector("Roughness_WrapL_Edge_IsMetal", &rwem);

   assert(ballIndexBuffer == NULL);
   const bool lowDetailBall = m_ptable->GetDetailLevel() < 10;
   ballIndexBuffer = m_pin3d.m_pd3dDevice->CreateAndFillIndexBuffer(lowDetailBall ? basicBallLoNumFaces : basicBallMidNumFaces, lowDetailBall ? basicBallLoIndices : basicBallMidIndices);

   // VB for normal ball
   assert(ballVertexBuffer == NULL);
   m_pin3d.m_pd3dDevice->CreateVertexBuffer(lowDetailBall ? basicBallLoNumVertices : basicBallMidNumVertices, 0, MY_D3DFVF_NOTEX2_VERTEX, &ballVertexBuffer);

   // load precomputed ball vertices into vertex buffer
   Vertex3D_NoTex2 *buf;
   ballVertexBuffer->lock(0, 0, (void**)&buf, VertexBuffer::WRITEONLY);
   memcpy(buf, lowDetailBall ? basicBallLo : basicBallMid, sizeof(Vertex3D_NoTex2)*(lowDetailBall ? basicBallLoNumVertices : basicBallMidNumVertices));
   ballVertexBuffer->unlock();

   D3DXVECTOR4 amb_lr = convertColor(m_ptable->m_lightAmbient, m_ptable->m_lightRange);
   amb_lr.x *= m_ptable->m_globalEmissionScale;
   amb_lr.y *= m_ptable->m_globalEmissionScale;
   amb_lr.z *= m_ptable->m_globalEmissionScale;
   ballShader->SetVector("cAmbient_LightRange", &amb_lr);
}


HRESULT Player::Init(PinTable * const ptable, const HWND hwndProgress, const HWND hwndProgressName)
{
    TRACE_FUNCTION();

	m_ptable = ptable;

	//m_hSongCompletionEvent = CreateEvent( NULL, TRUE, FALSE, NULL );

	SendMessage(hwndProgress, PBM_SETPOS, 10, 0);
	// TEXT
	SetWindowText(hwndProgressName, "Initializing Visuals...");

	InitWindow();
	InitKeys();
	InitRegValues();

	int vsync = (m_ptable->m_TableAdaptiveVSync == -1) ? m_fVSync : m_ptable->m_TableAdaptiveVSync;

    const bool useAO = (m_fAO && (m_ptable->m_useAO == -1)) || (m_ptable->m_useAO == 1);
    const bool useAA = (m_fAA && (m_ptable->m_useAA == -1)) || (m_ptable->m_useAA == 1);
    const bool stereo3D = (!!m_fStereo3D);
    const bool FXAA = ((m_fFXAA && (m_ptable->m_useFXAA == -1)) || (m_ptable->m_useFXAA > 0));

    int colordepth;
    if(GetRegInt("Player", "ColorDepth", &colordepth) != S_OK)
        colordepth = 32; // The default

    // colordepth & refreshrate are only defined if fullscreen is true.
    HRESULT hr = m_pin3d.InitPin3D(m_hwnd, m_fFullScreen, m_width, m_height, colordepth,
				                   m_refreshrate, vsync, useAA, stereo3D, FXAA, useAO);

	if (hr != S_OK)
	{
		char szfoo[64];
		sprintf_s(szfoo, "Error code: %x", hr);
		ShowError(szfoo);
		return hr;
	}

	if (m_fFullScreen)
		SetWindowPos(m_hwnd, NULL, 0, 0, m_width, m_height, SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE);

	m_pininput.Init(m_hwnd);

	//
	const unsigned int lflip = get_vk(m_rgKeys[eLeftFlipperKey]);
	const unsigned int rflip = get_vk(m_rgKeys[eRightFlipperKey]);
	if (((GetAsyncKeyState(VK_LSHIFT) & 0x8000) && (GetAsyncKeyState(VK_RSHIFT) & 0x8000))
		|| ((lflip != ~0u) && (rflip != ~0u) && (GetAsyncKeyState(lflip) & 0x8000) && (GetAsyncKeyState(rflip) & 0x8000)))
	{
		m_ptable->m_tblMirrorEnabled = true;
	}
	else
	{
		m_ptable->m_tblMirrorEnabled = false;

		int tmp;
		hr = GetRegInt("Player", "mirror", &tmp);
		if (hr == S_OK)
			m_ptable->m_tblMirrorEnabled = (tmp != 0);
	}
    m_pin3d.m_pd3dDevice->SetRenderState(RenderDevice::CULLMODE, D3DCULL_NONE); // re-init/thrash cache entry due to the hacky nature of the table mirroring
    m_pin3d.m_pd3dDevice->SetRenderState(RenderDevice::CULLMODE, D3DCULL_CCW);

    //

        m_pin3d.InitLayout();

	const float slope = ptable->m_angletiltMin
		+ (ptable->m_angletiltMax - ptable->m_angletiltMin)
		* ptable->m_globalDifficulty;

	m_gravity.x = 0;
	m_gravity.y = sinf(ANGTORAD(slope))*(ptable->m_fOverridePhysics ? ptable->m_fOverrideGravityConstant : ptable->m_Gravity);
	m_gravity.z = -cosf(ANGTORAD(slope))*(ptable->m_fOverridePhysics ? ptable->m_fOverrideGravityConstant : ptable->m_Gravity);

	m_NudgeX = 0;
	m_NudgeY = 0;
	m_movedPlunger = 0;

    Ball::ballID=0;

	SendMessage(hwndProgress, PBM_SETPOS, 30, 0);
	SetWindowText(hwndProgressName, "Initializing Physics...");

    {
        // Initialize new nudging.
        m_tableVel.SetZero();
        m_tableDisplacement.SetZero();
        m_tableVelOld.SetZero();
        m_tableVelDelta.SetZero();

        // Table movement (displacement u) is modeled as a mass-spring-damper system
        //   u'' = -k u - c u'
        // with a spring constant k and a damping coefficient c.
        // See http://en.wikipedia.org/wiki/Damping#Linear_damping

        const float nudgeTime = m_ptable->m_nudgeTime;      // T
        const float dampingRatio = 0.5f;                    // zeta

        // time for one half period (one swing and swing back):
        //   T = pi / omega_d,
        // where
        //   omega_d = omega_0 * sqrt(1 - zeta^2)       (damped frequency)
        //   omega_0 = sqrt(k)                          (undamped frequency)
        // Solving for the spring constant k, we get
        m_nudgeSpring = (float)(M_PI*M_PI) / (nudgeTime*nudgeTime * (1.0f - dampingRatio*dampingRatio));

        // The formula for the damping ratio is
        //   zeta = c / (2 sqrt(k)).
        // Solving for the damping coefficient c, we get
        m_nudgeDamping = dampingRatio * 2.0f * sqrtf(m_nudgeSpring);
    }

	// Need to set timecur here, for init functions that set timers
	m_time_msec = 0;

#ifdef FPS
    ToggleFPS();
    m_fShowFPS = false;
#endif

	for (int i = 0; i < m_ptable->m_vedit.Size(); i++)
	{
		IEditable * const pe = m_ptable->m_vedit.ElementAt(i);
		Hitable * const ph = pe->GetIHitable();
		if (ph)
		{
#ifdef _DEBUGPHYSICS
			if(pe->GetScriptable())
			{
				CComBSTR bstr;
				pe->GetScriptable()->get_Name(&bstr);
				char * bstr2 = MakeChar(bstr);
				CHAR wzDst[256];
				sprintf_s(wzDst, "Initializing Object-Physics %s...", bstr2);
				delete [] bstr2;
				SetWindowText(hwndProgressName, wzDst);
			}
#endif
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

	SendMessage(hwndProgress, PBM_SETPOS, 45, 0);
	SetWindowText(hwndProgressName, "Initializing Octree...");

	CreateBoundingHitShapes(&m_vho);

	for (int i = 0; i < m_vho.Size(); ++i)
    {
        HitObject *pho = m_vho.ElementAt(i);

        pho->CalcHitRect();

        m_hitoctree.AddElement(pho);

        if (pho->GetType() == eFlipper)
            m_vFlippers.push_back((HitFlipper*)pho);

        AnimObject *pao = pho->GetAnimObject();
        if (pao)
        {
			if (pho->GetType() == eLineSeg) // only slingshot still uses this
				m_vanimate.AddElement(pao);
            if (pao->FMover()) // spinner, gate, flipper, plunger
                m_vmover.push_back(pao);
        }
    }

    FRect3D tableBounds = m_ptable->GetBoundingBox();
    m_hitoctree.Initialize(tableBounds);
#ifndef NDEBUG
    m_hitoctree.DumpTree(0);
#endif

    // initialize hit structure for dynamic objects
    m_hitoctree_dynamic.FillFromVector( m_vho_dynamic );

	//----------------------------------------------------------------------------------

	SendMessage(hwndProgress, PBM_SETPOS, 60, 0);
	SetWindowText(hwndProgressName, "Rendering Table...");

    //g_viewDir = m_pin3d.m_viewVec;
    g_viewDir = Vertex3Ds(0, 0, -1.0f);

    InitShader();

    // search through all collection for elements which support group rendering
    for (int i = 0; i < m_ptable->m_vcollection.Size(); i++)
    {
        Collection *pcol = m_ptable->m_vcollection.ElementAt(i);
        for (int t = 0; t < pcol->m_visel.size(); t++)
        {
            // search for a primitive in the group, if found try to create a grouped render element
            ISelect *pisel = pcol->m_visel.ElementAt(t);
            if (pisel->GetItemType() == eItemPrimitive)
            {
                Primitive *prim = (Primitive*)pisel;
                prim->CreateRenderGroup(pcol, m_pin3d.m_pd3dDevice);
                break;
            }
        }
    }

	// Pre-render all non-changing elements such as 
	// static walls, rails, backdrops, etc.
	InitStatic(hwndProgress);

	for (int i = 0; i < m_ptable->m_vedit.Size(); ++i)
	{
		IEditable * const pe = m_ptable->m_vedit.ElementAt(i);
		Hitable * const ph = pe->GetIHitable();
		if (ph)
		{
			// sort into proper categories
			if (ph->IsTransparent())
				m_vHitTrans.push_back(ph);
			else
				m_vHitNonTrans.push_back(ph);
		}
	}

    std::stable_sort( m_vHitNonTrans.begin(), m_vHitNonTrans.end(), CompareHitableDepthReverse ); // stable, so that em reels (=same depth) will keep user defined order
    std::stable_sort( m_vHitNonTrans.begin(), m_vHitNonTrans.end(), CompareHitableMaterial ); // stable, so that objects with same materials will keep depth order

    std::stable_sort( m_vHitTrans.begin(), m_vHitTrans.end(), CompareHitableMaterial ); // see above
    std::stable_sort( m_vHitTrans.begin(), m_vHitTrans.end(), CompareHitableDepth );

    // Direct all renders to the back buffer.
    m_pin3d.SetRenderTarget(m_pin3d.m_pddsBackBuffer, m_pin3d.m_pddsZBuffer);

    SendMessage(hwndProgress, PBM_SETPOS, 90, 0);

#ifdef DEBUG_BALL_SPIN
    {
        std::vector< Vertex3D_TexelOnly > ballDbgVtx;
        for (int j = -1; j <= 1; ++j)
        {
            const int numPts = (j==0) ? 6 : 3;
            const float theta = (float)(j * (M_PI / 4.0));
            for (int i = 0; i < numPts; ++i)
            {
                const float phi = (float)(i * (2.0 * M_PI) / numPts);
                Vertex3D_TexelOnly vtx;
                vtx.x = 25.0f * cosf(theta) * cosf(phi);
                vtx.y = 25.0f * cosf(theta) * sinf(phi);
                vtx.z = 25.0f * sinf(theta);
                ballDbgVtx.push_back(vtx);
            }
        }

            assert(m_ballDebugPoints == NULL);
        m_pin3d.m_pd3dDevice->CreateVertexBuffer( ballDbgVtx.size(), 0, MY_D3DFVF_TEX, &m_ballDebugPoints );
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
		m_fplaylog = fopen("c:\\badlog.txt", "r");
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

	// Show the window.
	ShowWindow(m_hwnd, SW_SHOW);
	SetForegroundWindow(m_hwnd);
	SetFocus(m_hwnd);

	// Call Init -- TODO: what's the relation to ptable->FireVoidEvent() above?
	for (unsigned i = 0; i < m_vhitables.size(); ++i)
    {
        Hitable * const ph = m_vhitables[i];
        if (ph->GetEventProxyBase())
            ph->GetEventProxyBase()->FireVoidEvent(DISPID_GameEvents_Init);
    }

	if (m_fDetectScriptHang)
		g_pvp->PostWorkToWorkerThread(HANG_SNOOP_START, NULL);

    // 0 means disable limiting of draw-ahead queue
    m_limiter.Init(m_fMaxPrerenderedFrames);

	Render();

	return S_OK;
}

// reflection is split into two parts static and dynamic
// for the static objects:
//  1. switch to an extra mirror back buffer and mirror z-buffer
//  2. render the mirrored elements into these buffers (normal rendering)
//  3. switch back to normal camera mode and use the static back buffer but still use mirror z-buffer
//  4. render all static elements again to fill the mirror z-buffer with the correct depth information
//  5. copy the mirror back buffer into the mirror texture for blending the texture over the playfield in a later step
//
// for the dynamic objects:
//  1. copy the mirror z-buffer into the normal z-buffer
//  2. switch to a temporary mirror texture and render all dynamic elements into that buffer 
//  3. switch back to normal back buffer
//  4. render the dynamic mirror texture over the scene
//  5. render all dynamic objects as normal
void Player::RenderStaticMirror(const bool onlyBalls)
{
   D3DMATRIX viewMat;
   const float rotation = fmodf(m_ptable->m_BG_rotation[m_ptable->m_BG_current_set], 360.f);

   // Direct all renders to the "static" buffer.
   m_pin3d.SetRenderTarget(m_pin3d.m_mirrorBuffer, m_pin3d.m_mirrorZBuffer);
   RenderTarget *tmpMirrorSurface = NULL;
   m_pin3d.m_pd3dDevice->GetMirrorBufferTexture()->GetSurfaceLevel(0, &tmpMirrorSurface);

   m_pin3d.m_pd3dDevice->Clear(0, NULL, D3DCLEAR_ZBUFFER, 0, 1.0f, 0L);
   m_pin3d.m_pd3dDevice->FBShader->SetFloat("mirrorFactor", (float)m_ptable->m_playfieldReflectionStrength / 255.0f);

   if (!onlyBalls)
   {
      m_pin3d.m_pd3dDevice->GetTransform(TRANSFORMSTATE_VIEW, &viewMat);
      // flip camera
      viewMat._33 *= -1.0f;
      if (rotation != 0.0f)
         viewMat._31 *= -1.0f;
      else
      {
         viewMat._32 *= -1.0f;
      }
      m_pin3d.m_pd3dDevice->SetTransform(TRANSFORMSTATE_VIEW, &viewMat);

      m_pin3d.m_pd3dDevice->SetRenderState(RenderDevice::CULLMODE, D3DCULL_NONE);
      m_ptable->m_fReflectionEnabled = true;
      UpdateBasicShaderMatrix();

      // render mirrored static elements
      for (int i = 0; i < m_ptable->m_vedit.Size(); i++)
      {
         if (m_ptable->m_vedit.ElementAt(i)->GetItemType() != eItemDecal)
         {
            Hitable * const ph = m_ptable->m_vedit.ElementAt(i)->GetIHitable();
            if (ph)
            {
               ph->RenderStatic(m_pin3d.m_pd3dDevice);
            }
         }
      }

      m_ptable->m_fReflectionEnabled = false;
      m_pin3d.m_pd3dDevice->SetRenderState(RenderDevice::CULLMODE, D3DCULL_CCW);

      // and flip back camera
      viewMat._33 *= -1.0f;
      if (rotation != 0.0f)
         viewMat._31 *= -1.0f;
      else
      {
         viewMat._32 *= -1.0f;
      }
      m_pin3d.m_pd3dDevice->SetTransform(TRANSFORMSTATE_VIEW, &viewMat);
      UpdateBasicShaderMatrix();
   }
   m_pin3d.m_pd3dDevice->SetRenderTarget(m_pin3d.m_pddsStatic);


   // render normal static elements but into mirrored z-buffer
   for (int i = 0; i < m_ptable->m_vedit.Size(); i++)
   {
       if (m_ptable->m_vedit.ElementAt(i)->GetItemType() != eItemDecal)
       {
           Hitable * const ph = m_ptable->m_vedit.ElementAt(i)->GetIHitable();
           if (ph)
           {
               ph->RenderStatic(m_pin3d.m_pd3dDevice);
           }
       }
   }
   m_pin3d.SetRenderTarget(m_pin3d.m_pddsStatic, m_pin3d.m_pddsStaticZ);
   // copy mirror back buffer into mirror texture for rendering it over the playfield later on
   m_pin3d.m_pd3dDevice->CopySurface(tmpMirrorSurface, m_pin3d.m_mirrorBuffer);
   SAFE_RELEASE_NO_RCC(tmpMirrorSurface);
}

void Player::RenderDynamicMirror(const bool onlyBalls)
{
   D3DMATRIX viewMat;
   const float rotation = fmodf(m_ptable->m_BG_rotation[m_ptable->m_BG_current_set], 360.f);

   // copy the special mirror z-Buffer to the current z-buffer
   m_pin3d.m_pd3dDevice->CopySurface(m_pin3d.m_pddsZBuffer, m_pin3d.m_mirrorZBuffer);

   // render into temp back buffer 
   RenderTarget *tmpMirrorSurface = NULL;
   m_pin3d.m_pd3dDevice->GetMirrorTmpBufferTexture()->GetSurfaceLevel(0, &tmpMirrorSurface);
   m_pin3d.m_pd3dDevice->SetRenderTarget(tmpMirrorSurface);

   m_pin3d.m_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET, 0, 1.0f, 0L);

   m_pin3d.m_pd3dDevice->GetTransform(TRANSFORMSTATE_VIEW, &viewMat);
   // flip camera
   viewMat._33 *= -1.0f;
   if (rotation != 0.0f)
       viewMat._31 *= -1.0f;
   else
       viewMat._32 *= -1.0f;
   m_pin3d.m_pd3dDevice->SetTransform(TRANSFORMSTATE_VIEW, &viewMat);
   
   if ( !onlyBalls )
      UpdateBasicShaderMatrix();

   UpdateBallShaderMatrix();
   m_pin3d.m_pd3dDevice->BeginScene();

   m_pin3d.m_pd3dDevice->SetRenderState(RenderDevice::CULLMODE, D3DCULL_NONE);
   m_ptable->m_fReflectionEnabled = true;

   if (!onlyBalls)
   {
      // Draw transparent objects.
      for (unsigned int i = 0; i < m_vHitTrans.size(); ++i)
         m_vHitTrans[i]->PostRenderStatic(m_pin3d.m_pd3dDevice);
   }

   DrawBalls();

   if (!onlyBalls)
   {
      // Draw non-transparent objects.
      for (unsigned int i = 0; i < m_vHitNonTrans.size(); ++i)
         m_vHitNonTrans[i]->PostRenderStatic(m_pin3d.m_pd3dDevice);
   }


   m_ptable->m_fReflectionEnabled = false;
   m_pin3d.m_pd3dDevice->SetRenderState(RenderDevice::CULLMODE, D3DCULL_CCW);

   m_pin3d.m_pd3dDevice->EndScene();

   // and flip back camera
   viewMat._33 *= -1.0f;
   if (rotation != 0.0f)
       viewMat._31 *= -1.0f;
   else
       viewMat._32 *= -1.0f;
   m_pin3d.m_pd3dDevice->SetTransform(TRANSFORMSTATE_VIEW, &viewMat);
   if ( !onlyBalls)
      UpdateBasicShaderMatrix();

   UpdateBallShaderMatrix();

   m_pin3d.m_pd3dDevice->SetRenderTarget(m_pin3d.m_pddsBackBuffer);
   SAFE_RELEASE_NO_RCC(tmpMirrorSurface);
}

void Player::RenderMirrorOverlay(bool onlyStatic)
{
   // render the mirrored texture over the playfield
   if (onlyStatic)
      m_pin3d.m_pd3dDevice->FBShader->SetTexture("Texture0", m_pin3d.m_pd3dDevice->GetMirrorBufferTexture());
   else
      m_pin3d.m_pd3dDevice->FBShader->SetTexture("Texture0", m_pin3d.m_pd3dDevice->GetMirrorTmpBufferTexture());

   m_pin3d.m_pd3dDevice->FBShader->SetTechnique("fb_mirror");

   m_pin3d.EnableAlphaBlend(false, false);
   m_pin3d.m_pd3dDevice->SetRenderState(RenderDevice::DESTBLEND, D3DBLEND_DESTALPHA);
   // z-test must be enabled otherwise mirrored elements are drawn over blocking elements
   m_pin3d.m_pd3dDevice->SetRenderState(RenderDevice::CULLMODE, D3DCULL_NONE);
   m_pin3d.m_pd3dDevice->SetRenderState(RenderDevice::ZWRITEENABLE, FALSE);

   m_pin3d.m_pd3dDevice->FBShader->Begin(0);
   m_pin3d.m_pd3dDevice->DrawFullscreenQuad();
   m_pin3d.m_pd3dDevice->FBShader->End();

   m_pin3d.m_pd3dDevice->SetRenderState(RenderDevice::ZWRITEENABLE, TRUE);
   m_pin3d.m_pd3dDevice->SetRenderState(RenderDevice::CULLMODE, D3DCULL_CCW);
}

void Player::InitStatic(HWND hwndProgress)
{
    TRACE_FUNCTION();
	// Start the frame.
    for (unsigned i = 0; i < m_vhitables.size(); ++i)
    {
       Hitable * const ph = m_vhitables[i];
       ph->RenderSetup(m_pin3d.m_pd3dDevice);
    }


    m_pin3d.m_pd3dDevice->BeginScene();

	// Direct all renders to the "static" buffer.
	m_pin3d.SetRenderTarget(m_pin3d.m_pddsStatic, m_pin3d.m_pddsStaticZ);

    m_pin3d.DrawBackground();

    // perform render setup and give elements a chance to render before the playfield
    for (unsigned i=0; i < m_vhitables.size(); ++i)
    {
        Hitable * const ph = m_vhitables[i];
        ph->PreRenderStatic(m_pin3d.m_pd3dDevice);
    }

    m_pin3d.InitPlayfieldGraphics();
    if ( !cameraMode )
    {
       const bool drawBallReflection = ((m_fReflectionForBalls && (m_ptable->m_useReflectionForBalls == -1)) || (m_ptable->m_useReflectionForBalls == 1));
       if (!m_ptable->m_fReflectElementsOnPlayfield && drawBallReflection)
         RenderStaticMirror(true);
       else
          if (m_ptable->m_fReflectElementsOnPlayfield )
             RenderStaticMirror();

       m_pin3d.RenderPlayfieldGraphics();
       if (m_ptable->m_fReflectElementsOnPlayfield)
          RenderMirrorOverlay();

        // now render everything else
        for (int i=0;i<m_ptable->m_vedit.Size();i++)
        {
            if (m_ptable->m_vedit.ElementAt(i)->GetItemType() != eItemDecal)
            {
                Hitable * const ph = m_ptable->m_vedit.ElementAt(i)->GetIHitable();
                if (ph)
                {
                    ph->RenderStatic(m_pin3d.m_pd3dDevice);
                    if (hwndProgress && ((i%16)==0))
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
                    if (hwndProgress && ((i%16)==0))
                        SendMessage(hwndProgress, PBM_SETPOS, 75 + ((15*i)/m_ptable->m_vedit.Size()), 0);
                }
            }
        }
    }

	// Finish the frame.
	m_pin3d.m_pd3dDevice->EndScene();
}

Ball *Player::CreateBall(const float x, const float y, const float z, const float vx, const float vy, const float vz, const float radius, const float mass)
{
	Ball * const pball = new Ball();
	pball->m_radius = radius;
	pball->m_pos.x = x;
	pball->m_pos.y = y;
	pball->m_pos.z = z + pball->m_radius;

	//pball->z = z;
	pball->m_vel.x = vx;
	pball->m_vel.y = vy;
	pball->m_vel.z = vz;
	pball->Init(mass); // Call this after radius set to get proper inertial tensor set up

	pball->EnsureOMObject();

	pball->m_pfedebug = (IFireEvents *)pball->m_pballex;

	m_vball.push_back(pball);
	m_vmover.push_back(&pball->m_ballanim);

	pball->CalcHitRect();

	m_vho_dynamic.AddElement(pball);
    m_hitoctree_dynamic.FillFromVector(m_vho_dynamic);

	if (!m_pactiveballDebug)
		m_pactiveballDebug = pball;

	pball->m_defaultZ = pball->m_pos.z;

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

void Player::InitWindow()
{
	WNDCLASSEX wcex;
	ZeroMemory(&wcex, sizeof(WNDCLASSEX));
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = 0;
	wcex.lpfnWndProc = (WNDPROC)PlayerWndProc;
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
		m_height = m_width * 3 / 4;

	int fullscreen;
	hr = GetRegInt("Player", "FullScreen", &fullscreen);
	if (hr != S_OK)
		m_fFullScreen = false;
	else
		m_fFullScreen = (fullscreen == 1);

	if (m_fFullScreen)
	{
		m_screenwidth = m_width;
		m_screenheight = m_height;
		hr = GetRegInt("Player", "RefreshRate", &m_refreshrate);
		if (hr != S_OK)
			m_refreshrate = 0; // The default
	}
	else
	{
		m_screenwidth = GetSystemMetrics(SM_CXSCREEN);
		m_screenheight = GetSystemMetrics(SM_CYSCREEN);
		m_refreshrate = 0; // The default

		// constrain window to screen
		if (m_width > m_screenwidth)
		{
			m_width = m_screenwidth;
			m_height = m_width * 3 / 4;
		}

		if (m_height > m_screenheight)
		{
			m_height = m_screenheight;
			m_width = m_height * 4 / 3;
		}
	}

	const int x = (m_screenwidth - m_width) / 2;
	int y = (m_screenheight - m_height) / 2;

	// No window border, title, or control boxes.
	int windowflags = WS_POPUP;
	int windowflagsex = 0;

	const int captionheight = GetSystemMetrics(SM_CYCAPTION);

	if (!m_fFullScreen && ((m_screenheight - m_height) >= (captionheight * 2))) // We have enough room for a frame
	{
		// Add a pretty window border and standard control boxes.
		windowflags = WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_CLIPCHILDREN;
		windowflagsex = WS_EX_OVERLAPPEDWINDOW;

		y -= captionheight;
		m_height += captionheight;
	}
	CalcBallAspectRatio();
	m_hwnd = ::CreateWindowEx(windowflagsex, "VPPlayer", "Visual Pinball Player", windowflags, x, y, m_width, m_height, NULL, NULL, g_hinst, 0);

	mixer_init(m_hwnd);
	hid_init();
	SetCursorPos(400, 999999); // ShowCursor(false)?
}

void Player::CalcBallAspectRatio()
{
    HRESULT hr;

    int ballStretchMode;
    hr = GetRegInt("Player", "BallStretchMode", &ballStretchMode);
    if (hr != S_OK)
        ballStretchMode = 0;

    // Monitors: 4:3, 16:9, 16:10, 21:10
    int ballStretchMonitor;
    hr = GetRegInt("Player", "BallStretchMonitor", &ballStretchMonitor);
    if (hr != S_OK)
        ballStretchMonitor = 1; // assume 16:9

    float ballAspecRatioOffsetX;
    hr = GetRegStringAsFloat("Player", "BallCorrectionX", &ballAspecRatioOffsetX);
    if (hr != S_OK)
        ballAspecRatioOffsetX = 0.0f;

    float ballAspecRatioOffsetY;
    hr = GetRegStringAsFloat("Player", "BallCorrectionY", &ballAspecRatioOffsetY);
    if (hr != S_OK)
        ballAspecRatioOffsetY = 0.0f;

    const float scalebackX = (m_ptable->m_BG_scalex[m_ptable->m_BG_current_set] != 0.0f) ? ((m_ptable->m_BG_scalex[m_ptable->m_BG_current_set] + m_ptable->m_BG_scaley[m_ptable->m_BG_current_set])*0.5f)/m_ptable->m_BG_scalex[m_ptable->m_BG_current_set] : 1.0f;
    const float scalebackY = (m_ptable->m_BG_scaley[m_ptable->m_BG_current_set] != 0.0f) ? ((m_ptable->m_BG_scalex[m_ptable->m_BG_current_set] + m_ptable->m_BG_scaley[m_ptable->m_BG_current_set])*0.5f)/m_ptable->m_BG_scaley[m_ptable->m_BG_current_set] : 1.0f;
    float xMonitor=16.0f,yMonitor=9.0f;

    float aspect = (float)m_screenwidth/(float)m_screenheight;
    float factor = aspect*3.0f;
    if ( factor > 4.0f )
    {
        factor = aspect*9.0f;
        if ( factor == 16.0f )
        {
           //16:9
           xMonitor = (factor+ballAspecRatioOffsetX) / 4.0f;
           yMonitor = (9.0f+ballAspecRatioOffsetY) / 3.0f;
        }
        else
        {
            factor = aspect*10.f;
            if ( factor==16.0f )
            {
                //16:10
                xMonitor = (factor+ballAspecRatioOffsetX) /4.0f;
                yMonitor = (10.0f+ballAspecRatioOffsetY) / 3.0f;
            }
            else
            {
                //21:10
                xMonitor = (21.0f+ballAspecRatioOffsetX) / 4.0f;
                yMonitor = (10.0f+ballAspecRatioOffsetY) / 3.0f;
            }
        }
    }
    else
    {
        //4:3
        xMonitor = (factor+ballAspecRatioOffsetX) / 4.0f;
        yMonitor = (3.0f+ballAspecRatioOffsetY) / 3.0f;
    }
/*
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
    default:
        xMonitor = 16.0f;
        yMonitor = 9.0f;
        break;
    }
    xMonitor += -0.4f;
    yMonitor += 0.0f;
*/
    const float scalebackMonitorX = ((xMonitor + yMonitor)*0.5f)/xMonitor;
    const float scalebackMonitorY = (((xMonitor + yMonitor)*0.5f)/yMonitor);

    float temprotation = m_ptable->m_BG_rotation[m_ptable->m_BG_current_set];
    while (temprotation < 0.f)
        temprotation += 360.0f;

    const float c = sinf(ANGTORAD(fmodf(temprotation + 90.0f,180.0f)));
    const float s = sinf(ANGTORAD(fmodf(temprotation,180.0f)));
    m_antiStretchBall=false;

    switch(ballStretchMode)
    {
    case 0:
		m_BallStretchX = 1.0f;
        m_BallStretchY = 1.0f;
        break;
    case 1:
		m_BallStretchX = scalebackX*c + scalebackY*s;
        m_BallStretchY = scalebackY*c + scalebackX*s;
        break;
    case 2:
		m_BallStretchX = scalebackX*c + scalebackY*s;
        m_BallStretchY = scalebackY*c + scalebackX*s;
        if (m_fFullScreen || (m_width == m_screenwidth && m_height == m_screenheight)) // detect windowed fullscreen
        {
            m_antiStretchBall=true;
            m_BallStretchX *= scalebackMonitorX*c + scalebackMonitorY*s;
            m_BallStretchY *= scalebackMonitorY*c + scalebackMonitorX*s;
        }
        break;
    }

}

void Player::NudgeX(const int x, const int j)
{
   int v = x;
   if(x >  m_ptable->m_tblAccelMaxX) v =  m_ptable->m_tblAccelMaxX;
   if(x < -m_ptable->m_tblAccelMaxX) v = -m_ptable->m_tblAccelMaxX;
   m_curAccel_x[j] = v;
}

void Player::NudgeY(const int y, const int j)
{
   int v = y;
   if(y >  m_ptable->m_tblAccelMaxY) v =  m_ptable->m_tblAccelMaxY;
   if(y < -m_ptable->m_tblAccelMaxY) v = -m_ptable->m_tblAccelMaxY;
   m_curAccel_y[j] = v;
}

#define GetNudgeX() (((F32)m_curAccel_x[0]) * (F32)(2.0 / JOYRANGE)) // Get the -2 .. 2 values from joystick input tilt sensor / ushock //!! why 2?
#define GetNudgeY() (((F32)m_curAccel_y[0]) * (F32)(2.0 / JOYRANGE))

#ifdef UNUSED_TILT
int Player::NudgeGetTilt()
{
	static U32 last_tilt_time;
	static U32 last_jolt_time;

	if( !m_ptable->m_tblAccelerometer || m_NudgeManual >= 0 ||                   //disabled or in joystick test mode
		m_ptable->m_tilt_amount == 0 || m_ptable->m_jolt_amount == 0) return 0; //disabled

	const U32 ms = msec();

	U32 tilt_2 = 0;
	for(int j = 0; j < m_pininput.e_JoyCnt; ++j)    //find largest value
	{
		tilt_2 = max(tilt_2, (U32)(m_curAccel_x[j] * m_curAccel_x[j] + m_curAccel_y[j] * m_curAccel_y[j])); //always postive numbers
	}

	if( ( ms - last_jolt_time > m_ptable->m_jolt_trigger_time ) &&
		( ms - last_tilt_time > (U32)m_ptable->m_tilt_trigger_time ) &&
		tilt_2 > ( (U32)m_ptable->m_tilt_amount * (U32)m_ptable->m_tilt_amount ) )
	{
		last_tilt_time = ms;

		return 1;
	}

	if( ms - last_jolt_time > (U32)m_ptable->m_jolt_trigger_time && 
		tilt_2 > ( (U32)m_ptable->m_jolt_amount * (U32)m_ptable->m_jolt_amount ) )
	{
		last_jolt_time = ms;
	}

	return 0;
}
#endif

void Player::NudgeUpdate()      // called on every integral physics frame
{
	m_NudgeX = 0;   // accumulate over joysticks, these acceleration values are used in update ball velocity calculations
	m_NudgeY = 0;   // and are required to be acceleration values (not velocity or displacement)

	if (!m_ptable->m_tblAccelerometer) return;       // electronic accelerometer disabled 

	//rotate to match hardware mounting orentation, including left or right coordinates
	const float a = ANGTORAD(m_ptable->m_tblAccelAngle);
	const float cna = cosf(a);
	const float sna = sinf(a);

	for (int j = 0; j < m_pininput.e_JoyCnt; ++j)
	{
		float dx = ((float)m_curAccel_x[j])*(float)(1.0 / JOYRANGE);              // norm range -1 .. 1   
		const float dy = ((float)m_curAccel_y[j])*(float)(1.0 / JOYRANGE);
		if (m_ptable->m_tblMirrorEnabled)
			dx = -dx;
		m_NudgeX += m_ptable->m_tblAccelAmpX * (dx*cna + dy*sna) * (1.0f - nudge_get_sensitivity());  //calc Green's transform component for X
		const float nugY = m_ptable->m_tblAccelAmpY * (dy*cna - dx*sna) * (1.0f - nudge_get_sensitivity()); // calc Green transform component for Y...
		m_NudgeY = m_ptable->m_tblAccelNormalMount ? (m_NudgeY + nugY) : (m_NudgeY - nugY);     // add as left or right hand coordinate system
	}
}

#define IIR_Order 4

// coefficients for IIR_Order Butterworth filter set to 10 Hz passband
const float IIR_a [IIR_Order+1] = {
        0.0048243445f,
        0.019297378f,   
        0.028946068f,
        0.019297378f,
        0.0048243445f};

const float IIR_b[IIR_Order + 1] = {
        1.00000000f, //if not 1 add division below
        -2.369513f,
        2.3139884f,
        -1.0546654f,
        0.1873795f};

void Player::mechPlungerUpdate()        // called on every integral physics frame, only really triggered if before mechPlungerIn() was called, which again relies on USHOCKTYPE_GENERIC,USHOCKTYPE_ULTRACADE,USHOCKTYPE_PBWIZARD,USHOCKTYPE_VIRTUAPIN,USHOCKTYPE_SIDEWINDER being used
{       
	static int init = IIR_Order;    // first time call
	static float x[IIR_Order + 1] = { 0, 0, 0, 0, 0 };
	static float y[IIR_Order + 1] = { 0, 0, 0, 0, 0 };

	//http://www.dsptutor.freeuk.com/IIRFilterDesign/IIRFilterDesign.html  
	// (this applet is set to 8000Hz sample rate, therefore, multiply ...
	// our values by 80 to shift sample clock of 100hz to 8000hz)

	if (m_movedPlunger < 3)
	{
		init = IIR_Order;
		m_curMechPlungerPos = 0;
		return; // not until a real value is entered
	}

	if (!m_ptable->m_plungerFilter)
	{
		m_curMechPlungerPos = (float)m_curPlunger;
		return;
	}

	x[0] = (float)m_curPlunger; //initialize filter
	do
	{
		y[0] = IIR_a[0] * x[0];   // initial

		for (int i = IIR_Order; i > 0; --i) // all terms but the zero-th 
		{
			y[0] += (IIR_a[i] * x[i] - IIR_b[i] * y[i]);// /b[0]; always one     // sum terms from high to low
			x[i] = x[i - 1];          //shift 
			y[i] = y[i - 1];          //shift
		}
	} while (init-- > 0); //loop until all registers are initialized with the first input

	init = 0;

	m_curMechPlungerPos = y[0];
}

// mechPlunger NOTE: Normalized position is from 0.0 to +1.0f
// +1.0 is fully retracted, 0.0 is all the way forward.
//
// The traditional method requires calibration in control panel game controllers to work right.
// The calibrated zero value should match the rest position of the mechanical plunger.
// The method below uses a dual - piecewise linear function to map the mechanical pull and push 
// onto the virtual plunger position from 0..1, the pulunger properties has a ParkPosition setting 
// that matches the mechanical plunger zero position
//
// If the plunger device is a "linear plunger", we replace that calculation with a single linear
// scaling factor that applies on both sides of the park position.  This eliminates the need for
// separate calibration on each side of the park position, which seems to produce more consistent
// and linear behavior.  The Pinscape Controller plunger uses this method.
float PlungerAnimObject::mechPlunger() const
{
    if (g_pplayer->m_pininput.m_linearPlunger)
    {
        // Linear plunger device - the joystick must be calibrated such that the park
        // position reads as 0 and the fully retracted position reads as JOYRANGEMX.  The
        // scaling factor between physical units and joystick units must be the same on the
        // positive and negative sides.  (The maximum forward position is not calibrated.)
        const float m = (1.0f - m_restPos)*(float)(1.0/JOYRANGEMX), b = m_restPos;
        return m*g_pplayer->m_curMechPlungerPos + b;
    }
    else
    {
        // Standard plunger device - the joystick must be calibrated such that the park
        // position reads as 0, the fully retracted position reads as JOYRANGEMN, and the
        // full forward position reads as JOYRANGMN.
        const float range = (float)JOYRANGEMX * (1.0f - m_restPos) - (float)JOYRANGEMN *m_restPos; // final range limit
        const float tmp = (g_pplayer->m_curMechPlungerPos < 0) ? g_pplayer->m_curMechPlungerPos*m_restPos : g_pplayer->m_curMechPlungerPos*(1.0f - m_restPos);
        return tmp/range + m_restPos;              //scale and offset
    }
}

void Player::mechPlungerIn(const int z)
{
	m_curPlunger = -z; //axis reversal

	if (++m_movedPlunger == 0xffffffff) m_movedPlunger = 3; //restart at 3
}

// Accelerometer data filter.
//
// This is designed to process the raw acceleration data from a
// physical accelerometer installed in a cabinet to yield more
// realistic effects on the virtual ball.  With a physical
// accelerometer, there are inherent inaccuracies due to small
// measurement errors from the instrument and the finite sampling
// rate.  In addition, the VP simulation only approximates real
// time, so VP can only approximate the duration of each
// instantaneous acceleration - this can exaggerate some inputs
// and under-apply others.  Some of these sources of error are
// random and tend to cancel out over time, but others compound
// over many samples.  In practice there can be noticeable bias
// that causes unrealistic results in the simulation.
//
// This filter is designed to compensate for these various sources
// of error by applying some assumptions about how a real cabinet
// should behave, and adjusting the real input accelerations to
// more closely match how our ideal model cabinet would behave.
//
// The main constraint we use in this filter is net zero motion.
// When you nudge a real cabinet, you make it sway on its legs a
// little, but you don't usually move the cabinet across the
// floor - real cabinets are quite heavy so they tend to stay
// firmly rooted in place during normal play.  So once the swaying
// from a nudge dies out, which happens fairly quickly (in about
// one second, say), the cabinet is back where it started.  This
// means that the cabinet experienced a series of accelerations,
// back and forth, that ultimately canceled out and left the
// box at rest at its original position.  This is the central
// assumption of this filter: we should be able to add up
// (integrate) the series of instantaneous velocities imparted
// by the instantaneous accelerations over the course of a nudge,
// and we know that at the end, the sum should be zero, because
// the cabinet is back at rest at its starting location.  In
// practice, real accelerometer data for a real nudge event will
// come *close* to netting to zero, but won't quite get there;
// the discrepancy is from the various sources of error described
// above.  This filter tries to reconcile the imperfect measured
// data with our mathematically ideal model by making small
// adjustments to the measured data to get it to match the
// ideal model results.
//
// The point of the filter is to make the results *feel* more
// realistic by reducing visible artifacts from the measurement
// inaccuracies.  We thus have to take care that the filter's
// meedling hand doesn't itself become apparent as another
// visible artifact.  We try to keep the filter's effects subtle
// by trying to limit its intervention to small adjustments.
// For the most part, it kicks in at the point in a nudge where
// the real accelerometer data says things are coming back to rest
// naturally, and simply cleans up the tail end of the nudge
// response to get it to mathematical exactness.  The filter also
// notices a special situation where it shouldn't intervene,
// which is when there's a sustained acceleration in one
// direction.  Assuming that we're not operating under
// extraordinary conditions (e.g., on board an airplane
// accelerating down the runway), a sustained acceleration can
// only mean that someone picked up one end of the cabinet and
// is holding it at an angle.  In this case the ball *should*
// be accelerated in the direction of the tilt, so we don't
// attempt to zero out the net accelerations when we notice this
// type of condition.
//
// It's important to understand that this filter is only useful
// when the nudge inputs are coming from a physical, analog
// accelerometer installed in a standard, free-standing pinball
// cabinet.  The model and the parameters are tailored for this
// particular physical configuration, and it won't give good results
// for other setups.  In particular, don't use this filter with
// "digital" on/off nudge inputs, such as keyboard-based nudging or
// with cabinet sensors based on mercury switches or plumb bobs.
// The nudge accelerations for these sorts of digital nudge inputs
// are simulated, so they're already free of the analog measurement
// errors that this filter is designed to compensate for.
//
// This filter *might* work with real accelerometers that aren't
// in standard cabinets, such as in mini-cabs or desktop controllers.
// It's designed to mimic the physics of a standard cabinet, so
// using it in another physical setup probably wouldn't emulate
// that setup's natural physical behavior.  But the filter might
// give pleasing results anyway simply because every VP setup is
// ultimately meant to simulate the cabinet experience, so you
// probably want the simulation to behave like a cabinet even when
// it's not actually running in a cabinet physically.
//

NudgeFilter::NudgeFilter()
{
    m_sum = m_prv = 0.0f;
    m_tMotion = m_tCorr = m_tzc = 0;
}

// Process a sample.  Adds the sample to the running total, and checks
// to see if a correction should be applied.  Replaces 'a' with the
// corrected value if a correction is needed.
void NudgeFilter::sample(float &a, const U64 now)
{
    IF_DEBUG_NUDGE(char notes[128] = ""; float aIn = a;)
            
    // if we're not roughly at rest, reset the last motion timer
    if (fabsf(a) >= .02f)
        m_tMotion = now;
    
    // check for a sign change
    if (fabsf(a) > .01f && fabsf(m_prv) > .01f
        && ((a < 0.f && m_prv > 0.f) || (a > 0.f && m_prv < 0.f)))
    {
        // sign change/zero crossing - note the time
        m_tzc = now;
        IF_DEBUG_NUDGE(strcat(notes, "zc ");)
    }
    else if (fabsf(a) <= .01f)
    {
        // small value -> not a sustained one-way acceleration
        m_tzc = now;
    }
    else if (fabsf(a) > .05f && now - m_tzc > 500000)
    {
        // More than 500 ms in motion with same sign - we must be
        // experiencing a gravitational acceleration due to a tilt
        // of the playfield rather than a transient acceleration
        // from a nudge.  Don't attempt to correct these - clear
        // the sum and do no further processing.
        m_sum = 0;
        IF_DEBUG_NUDGE(dbg("%f >>>\n", a));
        return;
    }
    
    // if this sample is non-zero, remember it as the previous sample
    if (a != 0.f)
        m_prv = a;
    
    // add this sample to the running total
    m_sum += a;
    
    // If the running total is near zero, correct it to exactly zero.
    // 
    // Otherwise, if it's been too long since the last correction, or
    // we've been roughly at rest a while, add a damping correction to
    // bring the running total toward rest.
    if (fabsf(m_sum) < .02f)
    {
        // bring the residual acceleration exactly to rest
        IF_DEBUG_NUDGE(strcat(notes, "zero ");)
            a -= m_sum;
        m_sum = 0.f;
        
        // this counts as a zero crossing reset
        m_prv = 0;
        m_tzc = m_tCorr = now;
    }
    else if (now - m_tCorr > 50000 || now - m_tMotion > 50000)
    {
        // bring the running total toward neutral
        const float corr = expf(0.33f*logf(fabsf(m_sum*(float)(1.0/.02)))) * (m_sum < 0.0f ? -.02f : .02f);
        IF_DEBUG_NUDGE(strcat(notes, "damp ");)
        a -= corr;
        m_sum -= corr;
        
        // Advance the correction time slightly, but not all the
        // way to the present - we want to allow another forced
        // correction soon if necessary to get things back to
        // neutral quickly.
        m_tCorr = now - 40000;
    }
    
    IF_DEBUG_NUDGE(
        if (a != 0.f || aIn != 0.f)
        dbg(*axis() == 'x' ? "%f,%f, , ,%s\n" : " , ,%f,%f,%s\n",
            aIn, a, notes);)
}

// debug output
IF_DEBUG_NUDGE(void NudgeFilter::dbg(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    static FILE *fp = 0;
    if (fp == 0) fp = fopen("c:\\joystick.csv", "w");
    vfprintf(fp, fmt, args);
    va_end(args);
})
    
// apply nudge acceleration data filtering
void Player::FilterNudge()
{
    m_NudgeFilterX.sample(m_NudgeX, m_curPhysicsFrameTime);
    m_NudgeFilterY.sample(m_NudgeY, m_curPhysicsFrameTime);
}

//++++++++++++++++++++++++++++++++++++++++

void Player::SetGravity(float slopeDeg, float strength)
{
    m_gravity.x = 0;
    m_gravity.y =  sinf(ANGTORAD(slopeDeg)) * strength;
    m_gravity.z = -cosf(ANGTORAD(slopeDeg)) * strength;
}

#define STATICCNTS 10

void Player::PhysicsSimulateCycle(float dtime) // move physics forward to this time
{
	float hittime;
	int StaticCnts = STATICCNTS;    // maximum number of static counts

	// it's okay to have this code outside of the inner loop, as the ball hitrects already include the maximum distance they can travel in that timespan
	m_hitoctree_dynamic.Update();

	while (dtime > 0.f)
	{
		// first find hits, if any +++++++++++++++++++++ 
#ifdef _DEBUGPHYSICS
		c_timesearch++;
#endif
		hittime = dtime;        // begin time search from now ...  until delta ends

		// find earliest time where a flipper collides with its stop
		for (unsigned i = 0; i < m_vFlippers.size(); ++i)
		{
			const float fliphit = m_vFlippers[i]->GetHitTime();
			if (fliphit > 0.f && fliphit < hittime) //!! >= 0.f causes infinite loop
				hittime = fliphit;
		}

		m_fRecordContacts = true;
		m_contacts.clear();

		for (unsigned i = 0; i < m_vball.size(); i++)
		{
			Ball * const pball = m_vball[i];

			if (!pball->m_frozen && pball->m_dynamic > 0) // don't play with frozen balls
			{
				pball->m_coll.hittime = hittime;          // search upto current hittime
				pball->m_coll.obj = NULL;

				// always check for playfield and top glass
				DoHitTest(pball, &m_hitPlayfield, pball->m_coll);
				DoHitTest(pball, &m_hitTopGlass, pball->m_coll);

				if (rand_mt_01() < 0.5f) // swap order of dynamic and static obj checks randomly
				{
					m_hitoctree_dynamic.HitTestBall(pball, pball->m_coll);  // dynamic objects
					m_hitoctree.HitTestBall(pball, pball->m_coll);  // find the hit objects and hit times
				}
				else
				{
					m_hitoctree.HitTestBall(pball, pball->m_coll);  // find the hit objects and hit times
					m_hitoctree_dynamic.HitTestBall(pball, pball->m_coll);  // dynamic objects
				}

				const float htz = pball->m_coll.hittime; // this ball's hit time
				if (htz < 0.f) pball->m_coll.obj = NULL; // no negative time allowed

				if (pball->m_coll.obj)                                  // hit object
				{
#ifdef _DEBUGPHYSICS
					++c_hitcnts;                                        // stats for display

					if (/*pball->m_coll.hitRigid &&*/ pball->m_coll.hitdistance < -0.0875f) //rigid and embedded
						++c_embedcnts;
#endif
					///////////////////////////////////////////////////////////////////////////
					if (htz <= hittime)                         // smaller hit time??
					{
						hittime = htz;                          // record actual event time

						if (htz < STATICTIME)                   // less than static time interval
						{
							/*if (!pball->m_coll.hitRigid) hittime = STATICTIME; // non-rigid ... set Static time
							else*/ if (--StaticCnts < 0)
							{
								StaticCnts = 0;                 // keep from wrapping
								hittime = STATICTIME;
							}
						}
					}
				}
			}
		} // end loop over all balls

		m_fRecordContacts = false;

		// hittime now set ... or full frame if no hit 
		// now update displacements to collide-contact or end of physics frame
		// !!!!! 2) move objects to hittime

		if (hittime > STATICTIME) StaticCnts = STATICCNTS; // allow more zeros next round

		for (unsigned i = 0; i < m_vmover.size(); i++)
			m_vmover[i]->UpdateDisplacements(hittime); // step 2: move the objects about according to velocities (spinner, gate, flipper, plunger, ball)

		// find balls that need to be collided and script'ed (generally there will be one, but more are possible)

		for (unsigned i = 0; i < m_vball.size(); i++) // use m_vball.size(), in case script deletes a ball
		{
			Ball * const pball = m_vball[i];

			if (pball->m_dynamic > 0 && pball->m_coll.obj && pball->m_coll.hittime <= hittime) // find balls with hit objects and minimum time
			{
				// now collision, contact and script reactions on active ball (object)+++++++++
				HitObject * const pho = pball->m_coll.obj; // object that ball hit in trials
				m_pactiveball = pball;                     // For script that wants the ball doing the collision
#ifdef _DEBUGPHYSICS
				c_collisioncnt++;
#endif
				pho->Collide(&pball->m_coll);        //!!!!! 3) collision on active ball
				pball->m_coll.obj = NULL;            // remove trial hit object pointer

				// Collide may have changed the velocity of the ball, 
				// and therefore the bounding box for the next hit cycle
				if (m_vball[i] != pball) // Ball still exists? may have been deleted from list
				{
					// collision script deleted the ball, back up one count
					--i;
					continue;
				}
				else
				{
					pball->CalcHitRect(); // do new boundings 

					// is this ball static? .. set static and quench        
					if (/*pball->m_coll.hitRigid &&*/ (pball->m_coll.hitdistance < (float)PHYS_TOUCH)) //rigid and close distance contacts
					{
						const float mag = pball->m_vel.x*pball->m_vel.x + pball->m_vel.y*pball->m_vel.y; // values below are taken from simulation
						if (pball->m_drsq < 8.0e-5f && mag < 1.0e-3f*m_ptable->m_Gravity*m_ptable->m_Gravity / GRAVITYCONST / GRAVITYCONST && fabsf(pball->m_vel.z) < 0.2f*m_ptable->m_Gravity / GRAVITYCONST)
						{
							if(--pball->m_dynamic <= 0)             //... ball static, cancels next gravity increment
							{                                       // m_dynamic is cleared in ball gravity section
								pball->m_dynamic = 0;
#ifdef _DEBUGPHYSICS
								c_staticcnt++;
#endif
								pball->m_vel.x = pball->m_vel.y = pball->m_vel.z = 0.f; //quench the remaining velocity and set ...
						}
					}
				}
			}
			}
		}

#ifdef _DEBUGPHYSICS
		c_contactcnt = m_contacts.size();
#endif
		/*
		 * Now handle contacts.
		 *
		 * At this point UpdateDisplacements() was already called, so the state is different
		 * from that at HitTest(). However, contacts have zero relative velocity, so
		 * hopefully nothing catastrophic has happened in the meanwhile.
		 *
		 * Maybe a two-phase setup where we first process only contacts, then only collisions
		 * could also work.
		 */
		if (rand_mt_01() < 0.5f) // swap order of contact handling randomly
			for (unsigned i = 0; i < m_contacts.size(); ++i)
				m_contacts[i].obj->Contact(m_contacts[i], hittime);
		else
			for (unsigned i = m_contacts.size()-1; i != -1; --i)
				m_contacts[i].obj->Contact(m_contacts[i], hittime);

		m_contacts.clear();

		// hacky killing of ball spin on resting balls (very low and very high spinning)
		for (unsigned i = 0; i < m_vball.size(); i++)
		{
			Ball * const pball = m_vball[i];

			if (/*pball->m_coll.hitRigid &&*/ (pball->m_coll.hitdistance < (float)PHYS_TOUCH))
			{
				/*const float mag = pball->m_vel.x*pball->m_vel.x + pball->m_vel.y*pball->m_vel.y; // values below are copy pasted from above
				if (pball->m_drsq < 8.0e-5f && mag < 1.0e-3f*m_ptable->m_Gravity*m_ptable->m_Gravity / GRAVITYCONST / GRAVITYCONST && fabsf(pball->m_vel.z) < 0.2f*m_ptable->m_Gravity / GRAVITYCONST
					&& pball->m_angularmomentum.Length() < 0.9f*m_ptable->m_Gravity / GRAVITYCONST
					) //&& rand_mt_01() < 0.95f)
				{
					pball->m_angularmomentum *= 0.05f; // do not kill spin completely, otherwise stuck balls will happen during regular gameplay
					pball->m_angularvelocity *= 0.05f;
				}*/

				const Vertex3Ds diff_pos = pball->m_oldpos[(pball->m_ringcounter_oldpos / (10000 / PHYSICS_STEPTIME) + 1) % MAX_BALL_TRAIL_POS] - pball->m_pos;
				const float mag = diff_pos.x*diff_pos.x + diff_pos.y*diff_pos.y;
				const Vertex3Ds diff_pos2 = pball->m_oldpos[(pball->m_ringcounter_oldpos / (10000 / PHYSICS_STEPTIME) + 2) % MAX_BALL_TRAIL_POS] - pball->m_pos;
				const float mag2 = diff_pos2.x*diff_pos2.x + diff_pos2.y*diff_pos2.y;

				const float threshold = (pball->m_angularmomentum.x*pball->m_angularmomentum.x + pball->m_angularmomentum.y*pball->m_angularmomentum.y) / max(mag,mag2);

				if (!NaN(threshold) && threshold > 666.f)
				{
					const float damp = clamp(1.0f - (threshold - 666.f)/10000.f, 0.23f,1.f); // do not kill spin completely, otherwise stuck balls will happen during regular gameplay
					pball->m_angularmomentum *= damp;
					pball->m_angularvelocity *= damp;
				}
			}
		}

		dtime -= hittime;       //new delta .. i.e. time remaining

		m_swap_ball_collision_handling = !m_swap_ball_collision_handling; // swap order of ball-ball collisions

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
		m_fNoTimeCorrect = false;
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
	m_time_msec = (U32)((initial_time_usec - m_StartTime_usec) / 1000);

	phys_iterations = 0;

	m_overall_frames++;

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

		const float physics_diff_time = (float)((double)(m_nextPhysicsFrameTime - m_curPhysicsFrameTime)*(1.0 / DEFAULT_STEPTIME));
		const float physics_to_graphic_diff_time = (float)((double)(initial_time_usec - m_curPhysicsFrameTime)*(1.0 / DEFAULT_STEPTIME));

		//if (physics_to_graphic_diff_time < physics_diff_time)          // is graphic frame time next???
		//{
		//      PhysicsSimulateCycle(physics_to_graphic_diff_time);      // advance physics to this time
		//      m_curPhysicsFrameTime = initial_time_usec;                               // now current to the wall clock
		//      break;  //this is the common exit from the loop                  // exit skipping accelerate
		//}                     // some rare cases will exit from while()

		const U64 cur_time_usec = usec();
		if ((cur_time_usec - initial_time_usec > 200000) || (phys_iterations > ((m_ptable->m_PhysicsMaxLoops == 0) || (m_ptable->m_PhysicsMaxLoops == 0xFFFFFFFFu) ? 0xFFFFFFFFu : (m_ptable->m_PhysicsMaxLoops*(10000 / PHYSICS_STEPTIME))/*2*/))) // hung in the physics loop over 200 milliseconds or the number of physics iterations to catch up on is high (i.e. very low/unplayable FPS)
		{                                                                                                                        // can not keep up to real time
			m_curPhysicsFrameTime = initial_time_usec;                               // skip physics forward ... slip-cycles -> 'slowed' down physics
			m_nextPhysicsFrameTime = initial_time_usec + PHYSICS_STEPTIME;
			break;                                                                                                   // go draw frame
		}

		//update keys, hid, plumb, nudge, timers, etc
		//const U32 sim_msec = (U32)(m_curPhysicsFrameTime/1000);
		const U32 cur_time_msec = (U32)(cur_time_usec / 1000);

		m_pininput.ProcessKeys(m_ptable/*, sim_msec*/, cur_time_msec);

		mixer_update();
		hid_update(/*sim_msec*/cur_time_msec);
		plumb_update(/*sim_msec*/cur_time_msec, GetNudgeX(), GetNudgeY());

#ifdef ACCURATETIMERS
		m_pactiveball = NULL; // No ball is the active ball for timers/key events

		const int p_timeCur = (int)((m_curPhysicsFrameTime - m_StartTime_usec) / 1000); // milliseconds

		for (int i = 0; i < m_vht.Size(); i++)
		{
			HitTimer * const pht = m_vht.ElementAt(i);
			if (pht->m_nextfire <= p_timeCur)
			{
				pht->m_pfe->FireGroupEvent(DISPID_TimerEvents_Timer);
				pht->m_nextfire += pht->m_interval;
			}
		}
#endif
		NudgeUpdate();           // physics_diff_time is the balance of time to move from the graphic frame position to the next
		mechPlungerUpdate(); // integral physics frame. So the previous graphics frame was (1.0 - physics_diff_time) before 
		// this integral physics frame. Accelerations and inputs are always physics frame aligned
		{
			// table movement is modeled as a mass-spring-damper system
			//   u'' = -k u - c u'
			// with a spring constant k and a damping coefficient c
			const Vertex3Ds force = -m_nudgeSpring * m_tableDisplacement - m_nudgeDamping * m_tableVel;
			m_tableVel += (float)PHYS_FACTOR * force;
			m_tableDisplacement += (float)PHYS_FACTOR * m_tableVel;

			if (m_NudgeShake > 0.0f)
			{
				// NB: in table coordinates, +Y points down, but in screen coordinates, it points up,
				// so we have to flip the y component
				SetScreenOffset(m_NudgeShake * m_tableDisplacement.x, -m_NudgeShake * m_tableDisplacement.y);
			}

			m_tableVelDelta = m_tableVel - m_tableVelOld;
			m_tableVelOld = m_tableVel;
		}

		// Apply our filter to the nudge data
		if (m_pininput.m_enable_nudge_filter)
			FilterNudge();

		for (unsigned i = 0; i < m_vmover.size(); i++)
			m_vmover[i]->UpdateVelocities();        // always on integral physics frame boundary (spinner, gate, flipper, plunger, ball)

		//primary physics loop
		PhysicsSimulateCycle(physics_diff_time);    // main simulator call

		//ball trail, keep old pos of balls
		for (unsigned i = 0; i < m_vball.size(); i++)
		{
			Ball * const pball = m_vball[i];
			pball->m_oldpos[pball->m_ringcounter_oldpos / (10000 / PHYSICS_STEPTIME)] = pball->m_pos;

			pball->m_ringcounter_oldpos++;
			if (pball->m_ringcounter_oldpos == MAX_BALL_TRAIL_POS*(10000 / PHYSICS_STEPTIME))
				pball->m_ringcounter_oldpos = 0;
		}

		//slintf( "PT: %f %f %u %u %u\n", physics_diff_time, physics_to_graphic_diff_time, (U32)(m_curPhysicsFrameTime/1000), (U32)(initial_time_usec/1000), cur_time_msec );

		m_curPhysicsFrameTime = m_nextPhysicsFrameTime;                          // new cycle, on physics frame boundary
		m_nextPhysicsFrameTime += PHYSICS_STEPTIME;                                      // advance physics position
	} // end while (m_curPhysicsFrameTime < initial_time_usec)

#ifdef FPS
	phys_period = usec() - phys_period;
#endif
}

void Player::DMDdraw(const float DMDposx, const float DMDposy, const float DMDwidth, const float DMDheight, const COLORREF DMDcolor, const float intensity)
{
  if(m_device_texdmd)
  {
	  float DMDVerts[4 * 5] =
	  {
		  1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
		  0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		  1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
		  0.0f, 0.0f, 0.0f, 0.0f, 0.0f
	  };

	  for (unsigned int i = 0; i < 4; ++i)
	  {
		  DMDVerts[i * 5] = (DMDVerts[i * 5] * DMDwidth + DMDposx)*2.0f - 1.0f;
		  DMDVerts[i * 5 + 1] = 1.0f - (DMDVerts[i * 5 + 1] * DMDheight + DMDposy)*2.0f;
	  }

    const float width = g_pplayer->m_pin3d.m_useAA ? 2.0f*(float)m_width : (float)m_width;
    m_pin3d.m_pd3dDevice->DMDShader->SetTechnique(width*DMDwidth/(float)m_dmdx <= 3.74f ? "basic_DMD_tiny" : (width*DMDwidth/(float)m_dmdx <= 7.49f ? "basic_DMD" : "basic_DMD_big")); // use different smoothing functions for LED/Plasma emulation (rule of thumb here: up to quarter width of 1920HD = tiny, up to half width of 1920HD = normal, up to full width of 1920HD = big)

    const D3DXVECTOR4 c = convertColor(DMDcolor,intensity);
    m_pin3d.m_pd3dDevice->DMDShader->SetVector("vColor_Intensity",&c);
    const D3DXVECTOR4 r((float)m_dmdx,(float)m_dmdy, 0.f,0.f);
    m_pin3d.m_pd3dDevice->DMDShader->SetVector("vRes",&r);

    m_pin3d.m_pd3dDevice->DMDShader->SetTexture("Texture0", m_device_texdmd);

    m_pin3d.m_pd3dDevice->DMDShader->Begin(0);
    m_pin3d.m_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, MY_D3DFVF_TEX, (LPVOID)DMDVerts, 4);
    m_pin3d.m_pd3dDevice->DMDShader->End();
  }
}

void Player::Spritedraw(const float posx, const float posy, const float width, const float height, const COLORREF color, Texture * const tex, const float u0, const float v0, const float u1, const float v1, const float intensity)
{
	float Verts[4 * 5] =
	{
		1.0f, 1.0f, 0.0f, u1, v1,
		0.0f, 1.0f, 0.0f, u0, v1,
		1.0f, 0.0f, 0.0f, u1, v0,
		0.0f, 0.0f, 0.0f, u0, v0
	};

	for (unsigned int i = 0; i < 4; ++i)
	{
		Verts[i * 5] = (Verts[i * 5] * width + posx)*2.0f - 1.0f;
		Verts[i * 5 + 1] = 1.0f - (Verts[i * 5 + 1] * height + posy)*2.0f;
	}

	m_pin3d.m_pd3dDevice->DMDShader->SetTechnique(tex ? "basic_noDMD" : "basic_noDMD_notex");

	const D3DXVECTOR4 c = convertColor(color, intensity);
	m_pin3d.m_pd3dDevice->DMDShader->SetVector("vColor_Intensity", &c);

	if (tex)
		m_pin3d.m_pd3dDevice->DMDShader->SetTexture("Texture0", tex);

	m_pin3d.m_pd3dDevice->DMDShader->Begin(0);
	m_pin3d.m_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, MY_D3DFVF_TEX, (LPVOID)Verts, 4);
	m_pin3d.m_pd3dDevice->DMDShader->End();
}

void Player::Spritedraw(const float posx, const float posy, const float width, const float height, const COLORREF color, D3DTexture * const tex, const float intensity)
{
	float Verts[4 * 5] =
	{
		1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
		0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f, 0.0f
	};

	for (unsigned int i = 0; i < 4; ++i)
	{
		Verts[i * 5] = (Verts[i * 5] * width + posx)*2.0f - 1.0f;
		Verts[i * 5 + 1] = 1.0f - (Verts[i * 5 + 1] * height + posy)*2.0f;
	}

    m_pin3d.m_pd3dDevice->DMDShader->SetTechnique(tex ? "basic_noDMD" : "basic_noDMD_notex");

    const D3DXVECTOR4 c = convertColor(color,intensity);
    m_pin3d.m_pd3dDevice->DMDShader->SetVector("vColor_Intensity",&c);

	if (tex)
		m_pin3d.m_pd3dDevice->DMDShader->SetTexture("Texture0", tex);

    m_pin3d.m_pd3dDevice->DMDShader->Begin(0);
    m_pin3d.m_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, MY_D3DFVF_TEX, (LPVOID)Verts, 4);
    m_pin3d.m_pd3dDevice->DMDShader->End();
}

void Player::DrawBulbLightBuffer()
{
	// switch to 'bloom' output buffer to collect all bulb lights
	RenderTarget* tmpBloomSurface;
	m_pin3d.m_pd3dDevice->GetBloomBufferTexture()->GetSurfaceLevel(0, &tmpBloomSurface);
	m_pin3d.m_pd3dDevice->SetRenderTarget(tmpBloomSurface);

	m_pin3d.m_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET, 0, 1.0f, 0L);

	// check if any bulb specified at all
	bool do_renderstage = false;
	for (unsigned int i = 0; i < m_vHitTrans.size(); ++i)
		if (m_vHitTrans[i]->RenderToLightBuffer())
		{
			do_renderstage = true;
			break;
		}

	if (do_renderstage)
	{
		m_current_renderstage = 1; // for bulb lights so they know what they have to do

		m_pin3d.m_pd3dDevice->SetRenderState(RenderDevice::ZENABLE, FALSE); // disable all z-tests as zbuffer is in different resolution

		// Draw bulb lights with transmission scale only
		for (unsigned int i = 0; i < m_vHitTrans.size(); ++i)
			if (m_vHitTrans[i]->RenderToLightBuffer())
				m_vHitTrans[i]->PostRenderStatic(m_pin3d.m_pd3dDevice);

		m_pin3d.DisableAlphaBlend();
		m_pin3d.m_pd3dDevice->SetRenderState(RenderDevice::CULLMODE, D3DCULL_NONE);
		m_pin3d.m_pd3dDevice->SetRenderState(RenderDevice::ZWRITEENABLE, FALSE);
		m_pin3d.m_pd3dDevice->SetRenderState(RenderDevice::ZENABLE, FALSE);

		float verts[4 * 5] =
		{
			1.0f, 1.0f, 0.0f, 1.0f, 0.0f,
			-1.0f, 1.0f, 0.0f, 0.0f, 0.0f,
			1.0f, -1.0f, 0.0f, 1.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 1.0f
		};


		for (unsigned int blur = 0; blur < 2; ++blur) //!!! opt.: use larger blur kernel instead?!
		{
			RenderTarget* tmpBloomSurface2;
			{
				m_pin3d.m_pd3dDevice->FBShader->SetTexture("Texture0", (D3DTexture*)NULL);

				// switch to 'bloom' temporary output buffer for horizontal phase of gaussian blur
				m_pin3d.m_pd3dDevice->GetBloomTmpBufferTexture()->GetSurfaceLevel(0, &tmpBloomSurface2);
				m_pin3d.m_pd3dDevice->SetRenderTarget(tmpBloomSurface2);

				m_pin3d.m_pd3dDevice->FBShader->SetTexture("Texture0", m_pin3d.m_pd3dDevice->GetBloomBufferTexture());
				const D3DXVECTOR4 fb_inv_resolution_05((float)(3.0 / (double)m_width), (float)(3.0 / (double)m_height), 1.0f, 1.0f);
				m_pin3d.m_pd3dDevice->FBShader->SetVector("w_h_height", &fb_inv_resolution_05);
				m_pin3d.m_pd3dDevice->FBShader->SetTechnique("fb_bloom_horiz");

				m_pin3d.m_pd3dDevice->FBShader->Begin(0);
				m_pin3d.m_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, MY_D3DFVF_TEX, (LPVOID)verts, 4);
				m_pin3d.m_pd3dDevice->FBShader->End();
			}
			RenderTarget* tmpBloomSurface3;
			{
				m_pin3d.m_pd3dDevice->FBShader->SetTexture("Texture0", (D3DTexture*)NULL);

				// switch to 'bloom' output buffer for vertical phase of gaussian blur
				m_pin3d.m_pd3dDevice->GetBloomBufferTexture()->GetSurfaceLevel(0, &tmpBloomSurface3);
				m_pin3d.m_pd3dDevice->SetRenderTarget(tmpBloomSurface3);

				m_pin3d.m_pd3dDevice->FBShader->SetTexture("Texture0", m_pin3d.m_pd3dDevice->GetBloomTmpBufferTexture());
				const D3DXVECTOR4 fb_inv_resolution_05((float)(3.0 / (double)m_width), (float)(3.0 / (double)m_height), 1.0f, 1.0f);
				m_pin3d.m_pd3dDevice->FBShader->SetVector("w_h_height", &fb_inv_resolution_05);
				m_pin3d.m_pd3dDevice->FBShader->SetTechnique("fb_bloom_vert");

				m_pin3d.m_pd3dDevice->FBShader->Begin(0);
				m_pin3d.m_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, MY_D3DFVF_TEX, (LPVOID)verts, 4);
				m_pin3d.m_pd3dDevice->FBShader->End();
			}
			SAFE_RELEASE_NO_RCC(tmpBloomSurface2);
			SAFE_RELEASE_NO_RCC(tmpBloomSurface3);
		}

		m_pin3d.m_pd3dDevice->SetRenderState(RenderDevice::ZENABLE, TRUE);
		m_pin3d.m_pd3dDevice->SetRenderState(RenderDevice::ZWRITEENABLE, TRUE);
		m_pin3d.m_pd3dDevice->SetRenderState(RenderDevice::CULLMODE, D3DCULL_CCW);

		m_current_renderstage = 0;
	}

	// switch back to render buffer
	m_pin3d.m_pd3dDevice->SetRenderTarget(m_pin3d.m_pddsBackBuffer);
	SAFE_RELEASE_NO_RCC(tmpBloomSurface);

	m_pin3d.m_pd3dDevice->basicShader->SetTexture("Texture3", m_pin3d.m_pd3dDevice->GetBloomBufferTexture());
}

void Player::RenderDynamics()
{
   TRACE_FUNCTION();

   // Start rendering the next frame.
   m_pin3d.m_pd3dDevice->BeginScene();

   if( cameraMode )
   {
	   m_pin3d.InitLights();

	   const D3DXVECTOR4 st(m_ptable->m_envEmissionScale*m_ptable->m_globalEmissionScale, (float)m_pin3d.envTexture.m_height/*+m_pin3d.envTexture.m_width)*0.5f*/, 0.f, 0.f); //!! dto.
	   m_pin3d.m_pd3dDevice->basicShader->SetVector("fenvEmissionScale_TexWidth", &st);
#ifdef SEPARATE_CLASSICLIGHTSHADER
	   m_pin3d.m_pd3dDevice->classicLightShader->SetVector("fenvEmissionScale_TexWidth", &st);
#endif

	   UpdateBallShaderMatrix();
       m_pin3d.RenderPlayfieldGraphics();
       for (int i=0;i<m_ptable->m_vedit.Size();i++)
       {
           if (m_ptable->m_vedit.ElementAt(i)->GetItemType() != eItemDecal)
           {
               Hitable * const ph = m_ptable->m_vedit.ElementAt(i)->GetIHitable();
               if (ph)
               {
                   ph->RenderStatic(m_pin3d.m_pd3dDevice);
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
               }
           }
       }
   }

   // Draw non-transparent objects.
   for (unsigned int i=0; i < m_vHitNonTrans.size(); ++i)
       m_vHitNonTrans[i]->PostRenderStatic(m_pin3d.m_pd3dDevice);

   DrawBalls();

   m_limiter.Execute(m_pin3d.m_pd3dDevice); //!! move below other draw calls??

   DrawBulbLightBuffer();

   // Draw transparent objects.
   for (unsigned int i=0; i < m_vHitTrans.size(); ++i)
       m_vHitTrans[i]->PostRenderStatic(m_pin3d.m_pd3dDevice);

   m_pin3d.m_pd3dDevice->basicShader->SetTexture("Texture3", (D3DTexture*)NULL); // need to reset the bulb light texture, as its used as render target for bloom again

   m_pin3d.m_pd3dDevice->SetRenderState(RenderDevice::DEPTHBIAS, 0); //!! paranoia set of old state, remove as soon as sure that no other code still relies on that legacy set
   m_pin3d.m_pd3dDevice->SetRenderState(RenderDevice::ZWRITEENABLE, TRUE);
   m_pin3d.m_pd3dDevice->SetRenderState(RenderDevice::BLENDOP, D3DBLENDOP_ADD);
   m_pin3d.m_pd3dDevice->SetRenderState(RenderDevice::CULLMODE, D3DCULL_CCW);

   if( !cameraMode )
   {
       // Draw the mixer volume.
       mixer_draw();
       // Debug draw of plumb.
       plumb_draw();
   }

   // Finish rendering the next frame.
   m_pin3d.m_pd3dDevice->EndScene();
}


void Player::CheckAndUpdateRegions()
{
    //
    // copy static buffers to back buffer and z buffer
    //
    m_pin3d.m_pd3dDevice->CopySurface(m_pin3d.m_pddsBackBuffer, m_pin3d.m_pddsStatic);

    // Process all AnimObjects (currently only DispReel, LightSeq and Slingshot)
    for (int l = 0; l < m_vanimate.Size(); ++l)
       m_vanimate.ElementAt(l)->Animate();

    if (!cameraMode)
    {
        const bool drawBallReflection = ((m_fReflectionForBalls && (m_ptable->m_useReflectionForBalls == -1)) || (m_ptable->m_useReflectionForBalls == 1));

        if (!m_ptable->m_fReflectElementsOnPlayfield && drawBallReflection)
        {
            RenderDynamicMirror(true);
            RenderMirrorOverlay(false);
        }
        else if (m_ptable->m_fReflectElementsOnPlayfield)
        {
            RenderDynamicMirror();
            RenderMirrorOverlay(false);
        }
    }
    m_pin3d.m_pd3dDevice->CopySurface(m_pin3d.m_pddsZBuffer, m_pin3d.m_pddsStaticZ);
}

void Player::Bloom()
{
	if (m_ptable->m_bloom_strength <= 0.0f)
	{
		// need to reset content from (optional) bulb light abuse of the buffer
		RenderTarget* tmpBloomSurface;
		m_pin3d.m_pd3dDevice->GetBloomBufferTexture()->GetSurfaceLevel(0, &tmpBloomSurface);
		m_pin3d.m_pd3dDevice->SetRenderTarget(tmpBloomSurface);
		m_pin3d.m_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET, 0, 1.0f, 0L);
		SAFE_RELEASE_NO_RCC(tmpBloomSurface);

		return;
	}

	float shiftedVerts[4 * 5] =
	{
		1.0f, 1.0f, 0.0f, 1.0f + (float)(1.0 / (double)m_width), 0.0f + (float)(1.0 / (double)m_height),
		-1.0f, 1.0f, 0.0f, 0.0f + (float)(1.0 / (double)m_width), 0.0f + (float)(1.0 / (double)m_height),
		1.0f, -1.0f, 0.0f, 1.0f + (float)(1.0 / (double)m_width), 1.0f + (float)(1.0 / (double)m_height),
		-1.0f, -1.0f, 0.0f, 0.0f + (float)(1.0 / (double)m_width), 1.0f + (float)(1.0 / (double)m_height)
	};

	RenderTarget* tmpBloomSurface;
	{
		// switch to 'bloom' output buffer to collect clipped framebuffer values
		m_pin3d.m_pd3dDevice->GetBloomBufferTexture()->GetSurfaceLevel(0, &tmpBloomSurface);
		m_pin3d.m_pd3dDevice->SetRenderTarget(tmpBloomSurface);

		m_pin3d.m_pd3dDevice->FBShader->SetTexture("Texture0", m_pin3d.m_pd3dDevice->GetBackBufferTexture());

		const D3DXVECTOR4 fb_inv_resolution_05((float)(0.5 / (double)m_width), (float)(0.5 / (double)m_height), 1.0f, 1.0f);
		m_pin3d.m_pd3dDevice->FBShader->SetVector("w_h_height", &fb_inv_resolution_05);
		m_pin3d.m_pd3dDevice->FBShader->SetTechnique("fb_bloom");

		m_pin3d.m_pd3dDevice->FBShader->Begin(0);
		m_pin3d.m_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, MY_D3DFVF_TEX, (LPVOID)shiftedVerts, 4);
		m_pin3d.m_pd3dDevice->FBShader->End();
	}
	RenderTarget* tmpBloomSurface2;
	{
		m_pin3d.m_pd3dDevice->FBShader->SetTexture("Texture0", (D3DTexture*)NULL);

		// switch to 'bloom' temporary output buffer for horizontal phase of gaussian blur
		m_pin3d.m_pd3dDevice->GetBloomTmpBufferTexture()->GetSurfaceLevel(0, &tmpBloomSurface2);
		m_pin3d.m_pd3dDevice->SetRenderTarget(tmpBloomSurface2);

		m_pin3d.m_pd3dDevice->FBShader->SetTexture("Texture0", m_pin3d.m_pd3dDevice->GetBloomBufferTexture());
		const D3DXVECTOR4 fb_inv_resolution_05((float)(3.0 / (double)m_width), (float)(3.0 / (double)m_height), 1.0f, 1.0f);
		m_pin3d.m_pd3dDevice->FBShader->SetVector("w_h_height", &fb_inv_resolution_05);
		m_pin3d.m_pd3dDevice->FBShader->SetTechnique("fb_bloom_horiz");

		m_pin3d.m_pd3dDevice->FBShader->Begin(0);
      m_pin3d.m_pd3dDevice->DrawFullscreenQuad();
		m_pin3d.m_pd3dDevice->FBShader->End();
	}
	RenderTarget* tmpBloomSurface3;
	{
		m_pin3d.m_pd3dDevice->FBShader->SetTexture("Texture0", (D3DTexture*)NULL);

		// switch to 'bloom' output buffer for vertical phase of gaussian blur
		m_pin3d.m_pd3dDevice->GetBloomBufferTexture()->GetSurfaceLevel(0, &tmpBloomSurface3);
		m_pin3d.m_pd3dDevice->SetRenderTarget(tmpBloomSurface3);

		m_pin3d.m_pd3dDevice->FBShader->SetTexture("Texture0", m_pin3d.m_pd3dDevice->GetBloomTmpBufferTexture());
		const D3DXVECTOR4 fb_inv_resolution_05((float)(3.0 / (double)m_width), (float)(3.0 / (double)m_height), m_ptable->m_bloom_strength, 1.0f);
		m_pin3d.m_pd3dDevice->FBShader->SetVector("w_h_height", &fb_inv_resolution_05);
		m_pin3d.m_pd3dDevice->FBShader->SetTechnique("fb_bloom_vert");

		m_pin3d.m_pd3dDevice->FBShader->Begin(0);
      m_pin3d.m_pd3dDevice->DrawFullscreenQuad();
		m_pin3d.m_pd3dDevice->FBShader->End();
	}

	SAFE_RELEASE_NO_RCC(tmpBloomSurface);
	SAFE_RELEASE_NO_RCC(tmpBloomSurface2);
	SAFE_RELEASE_NO_RCC(tmpBloomSurface3);
}

void Player::StereoFXAA(const bool stereo, const bool FXAA1, const bool FXAA2, const bool FXAA3, const bool depth_available) //!! SMAA, luma sharpen, dither?
{
	if (stereo) // stereo implicitly disables FXAA
	{
		m_pin3d.m_pd3dDevice->SetRenderTarget(m_pin3d.m_pd3dDevice->GetOutputBackBuffer());

		m_pin3d.m_pd3dDevice->FBShader->SetTexture("Texture0", m_pin3d.m_pd3dDevice->GetBackBufferTmpTexture());
		m_pin3d.m_pd3dDevice->FBShader->SetTexture("Texture3", m_pin3d.m_pdds3DZBuffer);

		const D3DXVECTOR4 ms_zpd_ya_td(m_ptable->GetMaxSeparation(), m_ptable->GetZPD(), m_fStereo3DY ? 1.0f : 0.0f, (m_fStereo3D == 1) ? 1.0f : 0.0f);
		m_pin3d.m_pd3dDevice->FBShader->SetVector("ms_zpd_ya_td", &ms_zpd_ya_td);
		const D3DXVECTOR4 w_h_height((float)(1.0 / (double)m_width), (float)(1.0 / (double)m_height), (float)m_height, m_fStereo3DAA ? 1.0f : 0.0f);

		m_pin3d.m_pd3dDevice->FBShader->SetVector("w_h_height", &w_h_height);

		m_pin3d.m_pd3dDevice->FBShader->SetTechnique("stereo");

		m_pin3d.m_pd3dDevice->FBShader->Begin(0);
		m_pin3d.m_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, MY_D3DFVF_TEX, (LPVOID)quadVerts, 4);
		m_pin3d.m_pd3dDevice->FBShader->End();
	}
	else if (FXAA1 || FXAA2 || FXAA3)
	{
		m_pin3d.m_pd3dDevice->SetRenderTarget(m_pin3d.m_pd3dDevice->GetOutputBackBuffer());

		m_pin3d.m_pd3dDevice->FBShader->SetTexture("Texture0", m_pin3d.m_pd3dDevice->GetBackBufferTmpTexture());
		if (depth_available)
			m_pin3d.m_pd3dDevice->FBShader->SetTexture("Texture3", m_pin3d.m_pdds3DZBuffer);

		const D3DXVECTOR4 w_h_height((float)(1.0 / (double)m_width), (float)(1.0 / (double)m_height), 0.f, depth_available ? 1.f : 0.f);
		m_pin3d.m_pd3dDevice->FBShader->SetVector("w_h_height", &w_h_height);

		m_pin3d.m_pd3dDevice->FBShader->SetTechnique(FXAA3 ? "FXAA3" : (FXAA2 ? "FXAA2" : "FXAA1"));

		m_pin3d.m_pd3dDevice->FBShader->Begin(0);
		m_pin3d.m_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, MY_D3DFVF_TEX, (LPVOID)quadVerts, 4);
		m_pin3d.m_pd3dDevice->FBShader->End();
	}
}

void Player::FlipVideoBuffersNormal( const bool vsync )
{
	const bool useAA = (m_fAA && (m_ptable->m_useAA == -1)) || (m_ptable->m_useAA == 1);
    const bool stereo = ((m_fStereo3D != 0) && m_fStereo3Denabled);
	const bool FXAA1 = (((m_fFXAA == 1) && (m_ptable->m_useFXAA == -1)) || (m_ptable->m_useFXAA == 1));
	const bool FXAA2 = (((m_fFXAA == 2) && (m_ptable->m_useFXAA == -1)) || (m_ptable->m_useFXAA == 2));
	const bool FXAA3 = (((m_fFXAA == 3) && (m_ptable->m_useFXAA == -1)) || (m_ptable->m_useFXAA == 3));

	if (stereo)
		m_pin3d.m_pd3dDevice->CopyDepth(m_pin3d.m_pdds3DZBuffer, m_pin3d.m_pddsZBuffer);

	float shiftedVerts[4 * 5] =
	{
		1.0f + m_ScreenOffset.x, 1.0f + m_ScreenOffset.y, 0.0f, 1.0f + (float)(1.0 / (double)m_width), 0.0f + (float)(1.0 / (double)m_height),
		-1.0f + m_ScreenOffset.x, 1.0f + m_ScreenOffset.y, 0.0f, 0.0f + (float)(1.0 / (double)m_width), 0.0f + (float)(1.0 / (double)m_height),
		1.0f + m_ScreenOffset.x, -1.0f + m_ScreenOffset.y, 0.0f, 1.0f + (float)(1.0 / (double)m_width), 1.0f + (float)(1.0 / (double)m_height),
		-1.0f + m_ScreenOffset.x, -1.0f + m_ScreenOffset.y, 0.0f, 0.0f + (float)(1.0 / (double)m_width), 1.0f + (float)(1.0 / (double)m_height)
	};

	m_pin3d.m_pd3dDevice->BeginScene();

	m_pin3d.DisableAlphaBlend();
	m_pin3d.m_pd3dDevice->SetRenderState(RenderDevice::CULLMODE, D3DCULL_NONE);
	m_pin3d.m_pd3dDevice->SetRenderState(RenderDevice::ZWRITEENABLE, FALSE);
	m_pin3d.m_pd3dDevice->SetRenderState(RenderDevice::ZENABLE, FALSE);

	Bloom();

	// switch to output buffer
	if (!(stereo || FXAA1 || FXAA2 || FXAA3))
		m_pin3d.m_pd3dDevice->SetRenderTarget(m_pin3d.m_pd3dDevice->GetOutputBackBuffer());
	else
	{
		RenderTarget* tmpSurface;
		m_pin3d.m_pd3dDevice->GetBackBufferTmpTexture()->GetSurfaceLevel(0, &tmpSurface);
		m_pin3d.m_pd3dDevice->SetRenderTarget(tmpSurface);
		SAFE_RELEASE_NO_RCC(tmpSurface); //!!
	}

	// copy framebuffer over from texture and tonemap/gamma
	m_pin3d.m_pd3dDevice->FBShader->SetTexture("Texture0", m_pin3d.m_pd3dDevice->GetBackBufferTexture());
	m_pin3d.m_pd3dDevice->FBShader->SetTexture("Texture1", m_pin3d.m_pd3dDevice->GetBloomBufferTexture());

	Texture * const pin = m_ptable->GetImage((char *)m_ptable->m_szImageColorGrade);
	if (pin)
		m_pin3d.m_pd3dDevice->FBShader->SetTexture("Texture4", pin);
	m_pin3d.m_pd3dDevice->FBShader->SetBool("color_grade", pin != NULL);

        const D3DXVECTOR4 fb_inv_resolution_05((float)(0.5/(double)m_width),(float)(0.5/(double)m_height), 1.0f,1.0f);
	m_pin3d.m_pd3dDevice->FBShader->SetVector("w_h_height", &fb_inv_resolution_05);
	m_pin3d.m_pd3dDevice->FBShader->SetTechnique(useAA ? "fb_tonemap" : "fb_tonemap_no_filter");

	m_pin3d.m_pd3dDevice->FBShader->Begin(0);
	m_pin3d.m_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, MY_D3DFVF_TEX, (LPVOID)shiftedVerts, 4);
	m_pin3d.m_pd3dDevice->FBShader->End();

	StereoFXAA(stereo, FXAA1, FXAA2, FXAA3, false);

	m_pin3d.m_pd3dDevice->SetRenderState(RenderDevice::ZENABLE, TRUE);
	m_pin3d.m_pd3dDevice->SetRenderState(RenderDevice::ZWRITEENABLE, TRUE);
	m_pin3d.m_pd3dDevice->SetRenderState(RenderDevice::CULLMODE, D3DCULL_CCW);


//    m_pin3d.m_pd3dDevice->FBShader->SetTexture("Texture0", m_pin3d.m_pd3dDevice->GetMirrorTmpBufferTexture());
//    m_pin3d.m_pd3dDevice->FBShader->SetTechnique("fb_mirror");
// 
//    m_pin3d.m_pd3dDevice->SetRenderState(RenderDevice::CULLMODE, D3DCULL_NONE);
//    m_pin3d.m_pd3dDevice->SetRenderState(RenderDevice::ZWRITEENABLE, FALSE);
//    m_pin3d.m_pd3dDevice->SetRenderState(RenderDevice::ZENABLE, FALSE);
// 
//    m_pin3d.m_pd3dDevice->FBShader->Begin(0);
//    m_pin3d.m_pd3dDevice->DrawFullscreenQuad();
//    m_pin3d.m_pd3dDevice->FBShader->End();
// 
//    m_pin3d.m_pd3dDevice->SetRenderState(RenderDevice::ZENABLE, TRUE);
//    m_pin3d.m_pd3dDevice->SetRenderState(RenderDevice::ZWRITEENABLE, TRUE);
//    m_pin3d.m_pd3dDevice->SetRenderState(RenderDevice::CULLMODE, D3DCULL_CCW);
// 
   m_pin3d.m_pd3dDevice->EndScene();
   // display frame
	m_pin3d.Flip(vsync);

	// switch to texture output buffer again
	m_pin3d.m_pd3dDevice->FBShader->SetTexture("Texture0", (D3DTexture*)NULL);
	m_pin3d.m_pd3dDevice->SetRenderTarget(m_pin3d.m_pddsBackBuffer);
}

void Player::FlipVideoBuffersAO( const bool vsync )
{
	const bool useAO = (m_fAO && (m_ptable->m_useAO == -1)) || (m_ptable->m_useAO == 1);
	const bool useAA = (m_fAA && (m_ptable->m_useAA == -1)) || (m_ptable->m_useAA == 1);
	const bool stereo = ((m_fStereo3D != 0) && m_fStereo3Denabled);
	const bool FXAA1 = (((m_fFXAA == 1) && (m_ptable->m_useFXAA == -1)) || (m_ptable->m_useFXAA == 1));
	const bool FXAA2 = (((m_fFXAA == 2) && (m_ptable->m_useFXAA == -1)) || (m_ptable->m_useFXAA == 2));
	const bool FXAA3 = (((m_fFXAA == 3) && (m_ptable->m_useFXAA == -1)) || (m_ptable->m_useFXAA == 3));

	m_pin3d.m_pd3dDevice->CopyDepth(m_pin3d.m_pdds3DZBuffer, m_pin3d.m_pddsZBuffer);

	m_pin3d.m_pd3dDevice->BeginScene();

	m_pin3d.DisableAlphaBlend();
	m_pin3d.m_pd3dDevice->SetRenderState(RenderDevice::CULLMODE, D3DCULL_NONE);
	m_pin3d.m_pd3dDevice->SetRenderState(RenderDevice::ZWRITEENABLE, FALSE);
	m_pin3d.m_pd3dDevice->SetRenderState(RenderDevice::ZENABLE, FALSE);

	Bloom();

	RenderTarget* tmpAOSurface;
	m_pin3d.m_pddsAOBackTmpBuffer->GetSurfaceLevel(0, &tmpAOSurface);
	m_pin3d.m_pd3dDevice->SetRenderTarget(tmpAOSurface);
	SAFE_RELEASE_NO_RCC(tmpAOSurface); //!!

	m_pin3d.m_pd3dDevice->FBShader->SetTexture("Texture0", m_pin3d.m_pddsAOBackBuffer);
	m_pin3d.m_pd3dDevice->FBShader->SetTexture("Texture3", m_pin3d.m_pdds3DZBuffer);

	const D3DXVECTOR4 w_h_height((float)(1.0 / (double)m_width), (float)(1.0 / (double)m_height),
		radical_inverse(m_overall_frames)*(float)(1. / 9.0),
		sobol(m_overall_frames)*(float)(2. / 9.0)); // jitter within lattice cell
	m_pin3d.m_pd3dDevice->FBShader->SetVector("w_h_height", &w_h_height);
	m_pin3d.m_pd3dDevice->FBShader->SetFloat("AO_scale", m_ptable->m_AOScale);

	m_pin3d.m_pd3dDevice->FBShader->SetTechnique("AO");

	m_pin3d.m_pd3dDevice->FBShader->Begin(0);
	m_pin3d.m_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, MY_D3DFVF_TEX, (LPVOID)quadVerts, 4);
	m_pin3d.m_pd3dDevice->FBShader->End();

	// flip AO buffers (avoids copy)
	D3DTexture *tmpAO = m_pin3d.m_pddsAOBackBuffer;
	m_pin3d.m_pddsAOBackBuffer = m_pin3d.m_pddsAOBackTmpBuffer;
	m_pin3d.m_pddsAOBackTmpBuffer = tmpAO;

	// switch to output buffer
	if (!(stereo || FXAA1 || FXAA2 || FXAA3))
		m_pin3d.m_pd3dDevice->SetRenderTarget(m_pin3d.m_pd3dDevice->GetOutputBackBuffer());
	else
	{
		RenderTarget* tmpSurface;
		m_pin3d.m_pd3dDevice->GetBackBufferTmpTexture()->GetSurfaceLevel(0, &tmpSurface);
		m_pin3d.m_pd3dDevice->SetRenderTarget(tmpSurface);
		SAFE_RELEASE_NO_RCC(tmpSurface); //!!
	}

	float shiftedVerts[4 * 5] =
	{
		1.0f + m_ScreenOffset.x, 1.0f + m_ScreenOffset.y, 0.0f, 1.0f + (float)(1.0 / (double)m_width), 0.0f + (float)(1.0 / (double)m_height),
		-1.0f + m_ScreenOffset.x, 1.0f + m_ScreenOffset.y, 0.0f, 0.0f + (float)(1.0 / (double)m_width), 0.0f + (float)(1.0 / (double)m_height),
		1.0f + m_ScreenOffset.x, -1.0f + m_ScreenOffset.y, 0.0f, 1.0f + (float)(1.0 / (double)m_width), 1.0f + (float)(1.0 / (double)m_height),
		-1.0f + m_ScreenOffset.x, -1.0f + m_ScreenOffset.y, 0.0f, 0.0f + (float)(1.0 / (double)m_width), 1.0f + (float)(1.0 / (double)m_height)
	};

	m_pin3d.m_pd3dDevice->FBShader->SetTexture("Texture0", m_pin3d.m_pd3dDevice->GetBackBufferTexture());
	m_pin3d.m_pd3dDevice->FBShader->SetTexture("Texture1", m_pin3d.m_pd3dDevice->GetBloomBufferTexture());
	m_pin3d.m_pd3dDevice->FBShader->SetTexture("Texture3", m_pin3d.m_pddsAOBackBuffer);

	Texture * const pin = m_ptable->GetImage((char *)m_ptable->m_szImageColorGrade);
	if (pin)
		m_pin3d.m_pd3dDevice->FBShader->SetTexture("Texture4", pin);
	m_pin3d.m_pd3dDevice->FBShader->SetBool("color_grade", pin != NULL);

	const D3DXVECTOR4 fb_inv_resolution_05((float)(0.5 / (double)m_width), (float)(0.5 / (double)m_height), 1.0f, 1.0f);
	m_pin3d.m_pd3dDevice->FBShader->SetVector("w_h_height", &fb_inv_resolution_05);
	m_pin3d.m_pd3dDevice->FBShader->SetTechnique(useAA ? "fb_tonemap_AO" : "fb_tonemap_AO_no_filter");

	m_pin3d.m_pd3dDevice->FBShader->Begin(0);
	m_pin3d.m_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, MY_D3DFVF_TEX, (LPVOID)shiftedVerts, 4);
	m_pin3d.m_pd3dDevice->FBShader->End();

	StereoFXAA(stereo, FXAA1, FXAA2, FXAA3, true);

	//

	m_pin3d.m_pd3dDevice->SetRenderState(RenderDevice::ZENABLE, TRUE);
	m_pin3d.m_pd3dDevice->SetRenderState(RenderDevice::ZWRITEENABLE, TRUE);
	m_pin3d.m_pd3dDevice->SetRenderState(RenderDevice::CULLMODE, D3DCULL_CCW);

	m_pin3d.m_pd3dDevice->EndScene();

	// display frame
	m_pin3d.Flip(vsync);

	// switch to texture output buffer again
	m_pin3d.m_pd3dDevice->FBShader->SetTexture("Texture0", (D3DTexture*)NULL);
	m_pin3d.m_pd3dDevice->SetRenderTarget(m_pin3d.m_pddsBackBuffer);
}

void Player::SetScreenOffset(float x, float y)
{
    const float rotation = fmodf(m_ptable->m_BG_rotation[m_ptable->m_BG_current_set],360.f);
    m_ScreenOffset.x = (rotation!=0.0f ? -y : x);
    m_ScreenOffset.y = (rotation!=0.0f ? x : y);
}

void Player::UpdateBackdropSettings(const bool up)
{
	const float thesign = !up ? -1.0f : 1.0f;

	switch (backdropSettingActive)
	{
	case 0:
	{
		m_ptable->m_BG_inclination[m_ptable->m_BG_current_set] += thesign;
		break;
	}
	case 1:
	{
		m_ptable->m_BG_FOV[m_ptable->m_BG_current_set] += thesign;
		break;
	}
	case 2:
	{
		m_ptable->m_BG_layback[m_ptable->m_BG_current_set] += thesign;
		break;
	}
	case 3:
	{
		m_ptable->m_BG_scalex[m_ptable->m_BG_current_set] += 0.01f*thesign;
		break;
	}
	case 4:
	{
		m_ptable->m_BG_scaley[m_ptable->m_BG_current_set] += 0.01f*thesign;
		break;
	}
	case 5:
	{
		m_ptable->m_BG_scalez[m_ptable->m_BG_current_set] += 0.01f*thesign;
		break;
	}
	case 6:
	{
		m_ptable->m_BG_xlatex[m_ptable->m_BG_current_set] += thesign;
		break;
	}
	case 7:
	{
		m_ptable->m_BG_xlatey[m_ptable->m_BG_current_set] += thesign;
		break;
	}
	case 8:
	{
		m_ptable->m_BG_xlatez[m_ptable->m_BG_current_set] += thesign;
		break;
	}
	case 9:
	{
		m_ptable->m_lightEmissionScale += thesign*100000.f;
		if (m_ptable->m_lightEmissionScale < 0.f)
			m_ptable->m_lightEmissionScale = 0.f;
		break;
	}
	case 10:
	{
		m_ptable->m_lightRange += thesign*1000.f;
		if (m_ptable->m_lightRange < 0.f)
			m_ptable->m_lightRange = 0.f;
		break;
	}
	case 11:
	{
		m_ptable->m_lightHeight += thesign*100.f;
		if (m_ptable->m_lightHeight < 100.f)
			m_ptable->m_lightHeight = 100.f;
		break;
	}
	case 12:
	{
		m_ptable->m_envEmissionScale += thesign;
		if (m_ptable->m_envEmissionScale < 0.f)
			m_ptable->m_envEmissionScale = 0.f;
		break;
	}
	}
}
void Player::UpdateCameraModeDisplay()
{
    HDC hdcNull = GetDC(NULL);
    char szFoo[128];
    int len;
    len = sprintf_s(szFoo,"Camera & Light Mode                                 ");
    TextOut(hdcNull, 10, 30, szFoo, len);
    len = sprintf_s(szFoo,"Left / Right flipper key = decrease / increase value");
    TextOut(hdcNull, 10, 50, szFoo, len);
    len = sprintf_s(szFoo,"Left / Right magna save key = previous / next option");
    TextOut(hdcNull, 10, 70, szFoo, len);

	switch (backdropSettingActive)
	{
	case 0:
	{
		len = sprintf_s(szFoo, "Inclination: %f           ", m_ptable->m_BG_inclination[m_ptable->m_BG_current_set]);
		break;
	}
	case 1:
	{
		len = sprintf_s(szFoo, "Field Of View: %f         ", m_ptable->m_BG_FOV[m_ptable->m_BG_current_set]);
		break;
	}
	case 2:
	{
		len = sprintf_s(szFoo, "Layback: %f               ", m_ptable->m_BG_layback[m_ptable->m_BG_current_set]);
		break;
	}
	case 3:
	{
		len = sprintf_s(szFoo, "X Scale: %f               ", m_ptable->m_BG_scalex[m_ptable->m_BG_current_set]);
		break;
	}
	case 4:
	{
		len = sprintf_s(szFoo, "Y Scale: %f               ", m_ptable->m_BG_scaley[m_ptable->m_BG_current_set]);
		break;
	}
	case 5:
	{
		len = sprintf_s(szFoo, "Z Scale: %f               ", m_ptable->m_BG_scalez[m_ptable->m_BG_current_set]);
		break;
	}
	case 6:
	{
		len = sprintf_s(szFoo, "X Offset: %f              ", m_ptable->m_BG_xlatex[m_ptable->m_BG_current_set]);
		break;
	}
	case 7:
	{
		len = sprintf_s(szFoo, "Y Offset: %f              ", m_ptable->m_BG_xlatey[m_ptable->m_BG_current_set]);
		break;
	}
	case 8:
	{
		len = sprintf_s(szFoo, "Z Offset: %f              ", m_ptable->m_BG_xlatez[m_ptable->m_BG_current_set]);
		break;
	}
	case 9:
	{
		len = sprintf_s(szFoo, "Light Emission Scale: %f  ", m_ptable->m_lightEmissionScale);
		break;
	}
	case 10:
	{
		len = sprintf_s(szFoo, "Light Range: %f           ", m_ptable->m_lightRange);
		break;
	}
	case 11:
	{
		len = sprintf_s(szFoo, "Light Height: %f          ", m_ptable->m_lightHeight);
		break;
	}
	case 12:
	{
		len = sprintf_s(szFoo, "Environment Emission: %f  ", m_ptable->m_envEmissionScale);
		break;
	}
	default:
	{
		len = sprintf_s(szFoo, "unknown                ");
	}
	}
    TextOut(hdcNull, 10, 100, szFoo, len);
    ReleaseDC(NULL, hdcNull);
    m_pin3d.InitLayout();
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void Player::Render()
{
	U64 timeforframe = usec();

	if (m_firstFrame)
	{
		const HWND hVPMWnd = FindWindow("MAME", NULL);
		if (hVPMWnd != NULL)
		{
			if (IsWindowVisible(hVPMWnd))
				SetWindowPos(hVPMWnd, HWND_TOPMOST, 0, 0, 0, 0, (SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW | SWP_NOACTIVATE)); // in some strange cases the vpinmame window is not on top, so enforce it
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

	c_octNextlevels = 0;
	//c_quadNextlevels = 0; // not updated per frame so keep!

	c_traversed = 0;
    c_tested = 0;
    c_deepTested = 0;
#endif

    ///+++++++++++++++++++++++++++++++++++++++++++++++++++++
    //if ( !cameraMode )
    {
        UpdatePhysics();
    }

    m_LastKnownGoodCounter++;

    CheckAndUpdateRegions();
    RenderDynamics();

    // Check if we should turn animate the plunger light.
    hid_set_output ( HID_OUTPUT_PLUNGER, ((m_time_msec - m_LastPlungerHit) < 512) && ((m_time_msec & 512) > 0) );

    int localvsync = (m_ptable->m_TableAdaptiveVSync == -1) ? m_fVSync : m_ptable->m_TableAdaptiveVSync;
	if (localvsync > m_refreshrate) // cannot sync, just limit to selected framerate
		localvsync = 0;
	else if (localvsync > 1) // adaptive sync to refresh rate
		localvsync = m_refreshrate;

    bool vsync = false;
    if(localvsync > 0)
    {
        if(localvsync == 1) // legacy auto-detection
        {
            //if(m_fps > m_refreshrate*ADAPT_VSYNC_FACTOR) // do nothing, as already enforced during device set
            //    vsync = true;
        }
        else
            if(m_fps > localvsync*ADAPT_VSYNC_FACTOR)
                vsync = true;
    }

    if(!((m_fAO && (m_ptable->m_useAO == -1)) || (m_ptable->m_useAO == 1)) || !m_pin3d.m_pddsAOBackBuffer)
        FlipVideoBuffersNormal( vsync );
    else
        FlipVideoBuffersAO( vsync );

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

    if ( cameraMode )
        UpdateCameraModeDisplay();

#ifdef FPS
    if (m_fShowFPS && !cameraMode)
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
			float(1e-3f*period), float(1e-3f*(m_total / m_count)), float(1e-3f*m_max));
		TextOut(hdcNull, 10, 30, szFoo, len);

		// performance counters
		len = sprintf_s(szFoo, sizeof(szFoo), "Draw calls: %u      ", m_pin3d.m_pd3dDevice->Perf_GetNumDrawCalls());
		TextOut(hdcNull, 10, 65, szFoo, len);
		len = sprintf_s(szFoo, sizeof(szFoo), "State changes: %u      ", m_pin3d.m_pd3dDevice->Perf_GetNumStateChanges());
		TextOut(hdcNull, 10, 85, szFoo, len);
		len = sprintf_s(szFoo, sizeof(szFoo), "Texture changes: %u      ", m_pin3d.m_pd3dDevice->Perf_GetNumTextureChanges());
		TextOut(hdcNull, 10, 105, szFoo, len);
		len = sprintf_s(szFoo, sizeof(szFoo), "Parameter changes: %u      ", m_pin3d.m_pd3dDevice->Perf_GetNumParameterChanges());
		TextOut(hdcNull, 10, 125, szFoo, len);
		len = sprintf_s(szFoo, sizeof(szFoo), "Objects: %u Trans, %u Solid      ", m_vHitTrans.size(), m_vHitNonTrans.size());
		TextOut(hdcNull, 10, 145, szFoo, len);

#ifdef _DEBUGPHYSICS
		len = sprintf_s(szFoo, sizeof(szFoo), "physTimes %10u uS(%12u avg %12u max)    ",
			(U32)phys_period,
			(U32)(m_phys_total / m_count),
			(U32)m_phys_max );
		TextOut(hdcNull, 10, 180, szFoo, len);

		len = sprintf_s(szFoo, sizeof(szFoo), "phys:%5u iterations(%5u avg %5u max))   ",
			phys_iterations,
			(U32)( m_phys_total_iterations / m_count ),
			(U32)m_phys_max_iterations );
		TextOut(hdcNull, 10, 200, szFoo, len);

		len = sprintf_s(szFoo, sizeof(szFoo), "Hits:%5u Collide:%5u Ctacs:%5u Static:%5u Embed:%5u TimeSearch:%5u    ",
			c_hitcnts, c_collisioncnt, c_contactcnt, c_staticcnt, c_embedcnts, c_timesearch);
		TextOut(hdcNull, 10, 220, szFoo, len);

		len = sprintf_s(szFoo, sizeof(szFoo), "OctObjects: %5u Octree:%5u QuadObjects: %5u Quadtree:%5u Traversed:%5u Tested:%5u DeepTested:%5u  ",
			c_octObjects,c_octNextlevels,c_quadObjects,c_quadNextlevels,c_traversed,c_tested,c_deepTested);
		TextOut(hdcNull, 10, 240, szFoo, len);
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
	if (localvsync > m_refreshrate)
	{
		timeforframe = usec() - timeforframe;
		if (timeforframe < 1000000ull / localvsync)
			uSleep(1000000ull / localvsync - timeforframe);
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

            size_t option;

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

            m_fCloseDown = false;
            m_fNoTimeCorrect = true; // Skip the time we were in the dialog
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
			m_pxap->Pause();

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
			m_pxap->Unpause();

		// signal the script that the game is now running again
		m_ptable->FireVoidEvent(DISPID_GameEvents_UnPaused);
	}
	else if (m_pauseRefCount < 0)
		m_pauseRefCount = 0;
}

inline float map_bulblight_to_emission(const Light* const l) // magic mapping of bulblight parameters to "real" lightsource emission
{
    return l->m_d.m_currentIntensity * clamp(powf(l->m_d.m_falloff*0.6f, l->m_d.m_falloff_power*0.6f),0.f,23000.f); //!! 0.6f,0.6f = magic, also clamp 23000
}

void search_for_nearest(const Ball * const pball, const std::vector<Light*> &lights, Light* light_nearest[MAX_BALL_LIGHT_SOURCES])
{
	for (unsigned int l = 0; l < MAX_BALL_LIGHT_SOURCES; ++l)
	{
		float min_dist = FLT_MAX;
		light_nearest[l] = NULL;
		for (unsigned int i = 0; i < lights.size(); ++i)
		{
			bool already_processed = false;
			for (unsigned int i2 = 0; i2 < MAX_BALL_LIGHT_SOURCES - 1; ++i2)
				if (l > i2 && light_nearest[i2] == lights[i]) {
					already_processed = true;
					break;
				}
			if (already_processed)
				continue;

			const float dist = Vertex3Ds(lights[i]->m_d.m_vCenter.x - pball->m_pos.x, lights[i]->m_d.m_vCenter.y - pball->m_pos.y, lights[i]->m_d.m_meshRadius + lights[i]->m_surfaceHeight - pball->m_pos.z).LengthSquared(); //!! z pos
			//const float contribution = map_bulblight_to_emission(lights[i]) / dist; // could also weight in light color if necessary //!! JF didn't like that, seems like only distance is a measure better suited for the human eye
			if (dist < min_dist)
			{
				min_dist = dist;
				light_nearest[l] = lights[i];
			}
		}
	}
}

void Player::GetBallAspectRatio(const Ball * const pball, float &stretchX, float &stretchY, const float zHeight)
{
    Vertex3D_NoTex2 *rgvIn;
    Vertex2D *rgvOut;

    //     rgvIn[0].x = pball->m_pos.x;                    rgvIn[0].y=pball->m_pos.y+pball->m_radius;    rgvIn[0].z=zHeight;
//     rgvIn[1].x = pball->m_pos.x + pball->m_radius;    rgvIn[1].y = pball->m_pos.y;                    rgvIn[1].z = zHeight;
//     rgvIn[2].x = pball->m_pos.x;                    rgvIn[2].y = pball->m_pos.y - pball->m_radius;    rgvIn[2].z = zHeight;
//     rgvIn[3].x = pball->m_pos.x - pball->m_radius;    rgvIn[3].y = pball->m_pos.y;                    rgvIn[3].z = zHeight;
//     rgvIn[4].x = pball->m_pos.x;                    rgvIn[4].y = pball->m_pos.y;                    rgvIn[4].z = zHeight + pball->m_radius;
//     rgvIn[5].x = pball->m_pos.x;                    rgvIn[5].y = pball->m_pos.y;                    rgvIn[5].z = zHeight - pball->m_radius;
	const bool lowDetailBall = m_ptable->GetDetailLevel() < 10;
	const int numVerts = lowDetailBall ? basicBallLoNumVertices : basicBallMidNumVertices;
	const Vertex3D_NoTex2 * const ball = lowDetailBall ? basicBallLo : basicBallMid;
   rgvIn = new Vertex3D_NoTex2[(numVerts+1) / 2];
   rgvOut = new Vertex2D[(numVerts+1) / 2];
    for (int i = 0, t = 0; i < numVerts; i += 2,t++)
    {
       rgvIn[t].x = ball[i].x*pball->m_radius + pball->m_pos.x;
       rgvIn[t].y = ball[i].y*pball->m_radius + pball->m_pos.y;
       rgvIn[t].z = ball[i].z*pball->m_radius + zHeight;
    }
    m_pin3d.m_proj.TransformVertices(rgvIn, NULL, numVerts/2, rgvOut);
    float maxX = FLT_MIN;
    float minX = FLT_MAX;
    float maxY = FLT_MIN;
    float minY = FLT_MAX;
    for( int i=0;i<numVerts/2;i++ )
    {
        if(maxX<rgvOut[i].x) maxX = rgvOut[i].x;
        if(minX>rgvOut[i].x) minX = rgvOut[i].x;
        if(maxY<rgvOut[i].y) maxY = rgvOut[i].y;
        if(minY>rgvOut[i].y) minY = rgvOut[i].y;
    }
    const float midX = maxX - minX;
    const float midY = maxY - minY;
    stretchY = midY/midX;
    //stretchX = midX/midY;
    stretchX = 1.0f;
    delete[] rgvIn;
    delete[] rgvOut;
}

// not used anymore. Reflection of the ball is done in RenderDynamicMirror()!
void Player::DrawBallReflection(Ball *pball, const float zheight, const bool lowDetailBall)
{
   // this is the old ball reflection hack and can be removed if the new reflection works!
   const D3DXVECTOR4 pos_radRef(pball->m_pos.x, pball->m_pos.y, zheight + m_ptable->m_tableheight, pball->m_radius);
   ballShader->SetVector("position_radius", &pos_radRef);
   const D3DXVECTOR4 refl((float)m_ptable->m_ballReflectionStrength / 255.0f, (float)m_ptable->m_playfieldReflectionStrength / 255.0f, 0.f, 0.f);
   ballShader->SetVector("reflection_ball_playfield", &refl);
   m_pin3d.m_pd3dDevice->SetRenderState(RenderDevice::ZWRITEENABLE, FALSE);
   m_pin3d.EnableAlphaBlend(false, false);
   m_pin3d.m_pd3dDevice->SetRenderState(RenderDevice::DESTBLEND, D3DBLEND_DESTALPHA);
   ballShader->SetTechnique("RenderBallReflection");

   ballShader->Begin(0);
   m_pin3d.m_pd3dDevice->DrawIndexedPrimitiveVB(D3DPT_TRIANGLELIST, MY_D3DFVF_NOTEX2_VERTEX, ballVertexBuffer, 0, lowDetailBall ? basicBallLoNumVertices : basicBallMidNumVertices, ballIndexBuffer, 0, lowDetailBall ? basicBallLoNumFaces : basicBallMidNumFaces);
   ballShader->End();

   m_pin3d.DisableAlphaBlend();
}

void Player::DrawBalls()
{
	m_pin3d.m_pd3dDevice->SetRenderState(RenderDevice::DEPTHBIAS, 0);
    m_pin3d.m_pd3dDevice->SetRenderState(RenderDevice::BLENDOP, D3DBLENDOP_ADD);
    m_pin3d.m_pd3dDevice->SetRenderState(RenderDevice::CULLMODE, D3DCULL_CCW);

    if (m_ToggleDebugBalls && m_DebugBalls)
        // Set the render state to something that will always display.
        m_pin3d.m_pd3dDevice->SetRenderState ( RenderDevice::ZENABLE, FALSE );

	// collect all lights that can reflect on balls (currently only bulbs and if flag set to do so)
	std::vector<Light*> lights;
	for (int i = 0; i < m_ptable->m_vedit.Size(); i++)
	{
		IEditable *item = m_ptable->m_vedit.ElementAt(i);
		if (item->GetItemType() == eItemLight && ((Light *)item)->m_d.m_BulbLight && ((Light *)item)->m_d.m_showReflectionOnBall)
			lights.push_back((Light *)item);
	}

    bool drawReflection = ((m_fReflectionForBalls && (m_ptable->m_useReflectionForBalls == -1)) || (m_ptable->m_useReflectionForBalls == 1));
    
//     if (reflectionOnly && !drawReflection)
//        return;

    //m_pin3d.m_pd3dDevice->SetTextureAddressMode(0, RenderDevice::TEX_CLAMP);
    //m_pin3d.m_pd3dDevice->SetTextureFilter(0, TEXTURE_MODE_TRILINEAR);

    for (unsigned i=0; i<m_vball.size(); i++)
    {
        Ball * const pball = m_vball[i];

		// collect the x nearest lights that can reflect on balls
		Light* light_nearest[MAX_BALL_LIGHT_SOURCES];
		search_for_nearest(pball, lights, light_nearest);

		struct CLight
		{
			float vPos[3];
			float vEmission[3];
		};
		CLight l[MAX_LIGHT_SOURCES + MAX_BALL_LIGHT_SOURCES];
                
		D3DXVECTOR4 emission = convertColor(m_ptable->m_Light[0].emission);
		emission.x *= m_ptable->m_lightEmissionScale*m_ptable->m_globalEmissionScale;
		emission.y *= m_ptable->m_lightEmissionScale*m_ptable->m_globalEmissionScale;
		emission.z *= m_ptable->m_lightEmissionScale*m_ptable->m_globalEmissionScale;

		for (unsigned int i2 = 0; i2 < MAX_LIGHT_SOURCES; ++i2)
		{
			memcpy(&l[i2].vPos, &g_pplayer->m_ptable->m_Light[i2].pos, sizeof(float) * 3);
			memcpy(&l[i2].vEmission, &emission, sizeof(float) * 3);
		}

		for (unsigned int light_i = 0; light_i < MAX_BALL_LIGHT_SOURCES; ++light_i)
			if (light_nearest[light_i] != NULL)
			{
				l[light_i + MAX_LIGHT_SOURCES].vPos[0] = light_nearest[light_i]->m_d.m_vCenter.x;
				l[light_i + MAX_LIGHT_SOURCES].vPos[1] = light_nearest[light_i]->m_d.m_vCenter.y;
				l[light_i + MAX_LIGHT_SOURCES].vPos[2] = light_nearest[light_i]->m_d.m_meshRadius + light_nearest[light_i]->m_surfaceHeight; //!! z pos
				const float c = map_bulblight_to_emission(light_nearest[light_i]) * pball->m_bulb_intensity_scale;
				const D3DXVECTOR4 color = convertColor(light_nearest[light_i]->m_d.m_color);
				l[light_i + MAX_LIGHT_SOURCES].vEmission[0] = color.x*c;
				l[light_i + MAX_LIGHT_SOURCES].vEmission[1] = color.y*c;
				l[light_i + MAX_LIGHT_SOURCES].vEmission[2] = color.z*c;
			}
			else //!! rather just set the max number of ball lights!?
			{
				l[light_i + MAX_LIGHT_SOURCES].vPos[0] = -100000.0f;
				l[light_i + MAX_LIGHT_SOURCES].vPos[1] = -100000.0f;
				l[light_i + MAX_LIGHT_SOURCES].vPos[2] = -100000.0f;
				l[light_i + MAX_LIGHT_SOURCES].vEmission[0] = 0.0f;
				l[light_i + MAX_LIGHT_SOURCES].vEmission[1] = 0.0f;
				l[light_i + MAX_LIGHT_SOURCES].vEmission[2] = 0.0f;
			}

      ballShader->SetValue("packedLights", l, sizeof(CLight)*(MAX_LIGHT_SOURCES+MAX_BALL_LIGHT_SOURCES));

      // now for a weird hack: make material more rough, depending on how near the nearest lightsource is, to 'emulate' the area of the bulbs (as VP only features point lights so far)
      float Roughness = 0.8f;
	  if(light_nearest[0] != NULL)
	  {
		  const float dist = Vertex3Ds(light_nearest[0]->m_d.m_vCenter.x - pball->m_pos.x, light_nearest[0]->m_d.m_vCenter.y - pball->m_pos.y, light_nearest[0]->m_d.m_meshRadius + light_nearest[0]->m_surfaceHeight - pball->m_pos.z).Length(); //!! z pos
		  Roughness = min(max(dist*0.006f,0.3f),Roughness);
	  }
	  const D3DXVECTOR4 rwem(exp2f(10.0f * Roughness + 1.0f), 0.f, 1.f, 0.0f);
	  ballShader->SetVector("Roughness_WrapL_Edge_IsMetal", &rwem);

      // start drawing

      float zheight = (!pball->m_frozen) ? pball->m_pos.z : (pball->m_pos.z - pball->m_radius);
      
      if ( m_ptable->m_fReflectionEnabled)
         zheight -= m_ptable->m_tableheight*2.0f;

      const float maxz = pball->m_defaultZ+3.0f;
      const float minz = pball->m_defaultZ-0.1f;
      if((m_fReflectionForBalls && (m_ptable->m_useReflectionForBalls == -1)) || (m_ptable->m_useReflectionForBalls == 1))
         // don't draw reflection if the ball is not on the playfield (e.g. on a ramp/kicker)
         drawReflection = !( (zheight > maxz) || pball->m_frozen || (pball->m_pos.z < minz) );

      if( (zheight > maxz) || (pball->m_pos.z < minz) )
      {
         // scaling the ball height by the z scale value results in a flying ball over the playfield/ramp
         // by reducing it with 0.96f (a factor found by trial'n error) the ball is on the ramp again
         if ( m_ptable->m_BG_scalez[m_ptable->m_BG_current_set] != 1.0f )
            zheight *= (m_ptable->m_BG_scalez[m_ptable->m_BG_current_set]*0.96f); 
      }

      Texture * const playfield = m_ptable->GetImage((char *)m_ptable->m_szImage);
      if( playfield )
          ballShader->SetTexture("Texture1", playfield );

      // ************************* draw the ball itself ****************************
      if ( m_antiStretchBall && m_ptable->m_BG_rotation[m_ptable->m_BG_current_set]!=0.0f )
      {
          float sx,sy;
          GetBallAspectRatio(pball, sx, sy, zheight);

		  const float inv_tablewidth = 1.0f / (m_ptable->m_right - m_ptable->m_left);
		  const float inv_tableheight = 1.0f / (m_ptable->m_bottom - m_ptable->m_top);
          //const D3DXVECTOR4 bs(m_BallStretchX/* +sx*/, m_BallStretchY - sy, inv_tablewidth, inv_tableheight);
          const D3DXVECTOR4 bs(sx, sy, inv_tablewidth, inv_tableheight);
          ballShader->SetVector("ballStretch_invTableRes", &bs);
      }

	  const D3DXVECTOR4 diffuse = convertColor(pball->m_color, 1.0f);
	  ballShader->SetVector("cBase_Alpha", &diffuse);

	  const D3DXMATRIX m(pball->m_orientation.m_d[0][0], pball->m_orientation.m_d[1][0], pball->m_orientation.m_d[2][0], 0.0f,
		  pball->m_orientation.m_d[0][1], pball->m_orientation.m_d[1][1], pball->m_orientation.m_d[2][1], 0.0f,
		  pball->m_orientation.m_d[0][2], pball->m_orientation.m_d[1][2], pball->m_orientation.m_d[2][2], 0.0f,
		  0.f, 0.f, 0.f, 1.f);
      ballShader->SetMatrix("orientation",&m);

      const D3DXVECTOR4 pos_rad( pball->m_pos.x, pball->m_pos.y, zheight, pball->m_radius );
      if ( !pball->m_pinballEnv )
          ballShader->SetTexture("Texture0", &m_pin3d.pinballEnvTexture);
      else
          ballShader->SetTexture("Texture0", pball->m_pinballEnv);

      if( pball->m_pinballDecal )
          ballShader->SetTexture("Texture2", pball->m_pinballDecal);

	  const bool lowDetailBall = m_ptable->GetDetailLevel() < 10;

// 	  if (drawReflection)
// 	  {
//         DrawBallReflection(pball, zheight, lowDetailBall);
//      }

     if ( !drawReflection &&  m_ptable->m_fReflectionEnabled)
        continue;

      ballShader->SetVector("position_radius", &pos_rad);
      m_pin3d.m_pd3dDevice->SetRenderState(RenderDevice::ZWRITEENABLE, TRUE);
      ballShader->SetTechnique("RenderBall");
      
      ballShader->Begin(0);
	  m_pin3d.m_pd3dDevice->DrawIndexedPrimitiveVB(D3DPT_TRIANGLELIST, MY_D3DFVF_NOTEX2_VERTEX, ballVertexBuffer, 0, lowDetailBall ? basicBallLoNumVertices : basicBallMidNumVertices, ballIndexBuffer, 0, lowDetailBall ? basicBallLoNumFaces : basicBallMidNumFaces);
      ballShader->End();

      // ball trails
      if((m_fTrailForBalls && (m_ptable->m_useTrailForBalls == -1)) || (m_ptable->m_useTrailForBalls == 1))
	  {
		  Vertex3D_NoTex2 rgv3D_all[MAX_BALL_TRAIL_POS * 2];
		  unsigned int num_rgv3D = 0;

		  for (int i2 = 0; i2 < MAX_BALL_TRAIL_POS - 1; ++i2)
		  {
			  int i3 = pball->m_ringcounter_oldpos / (10000 / PHYSICS_STEPTIME) - 1 - i2;
			  if (i3 < 0)
				  i3 += MAX_BALL_TRAIL_POS;
			  int io = i3 - 1;
			  if (io < 0)
				  io += MAX_BALL_TRAIL_POS;

			  if ((pball->m_oldpos[i3].x != FLT_MAX) && (pball->m_oldpos[io].x != FLT_MAX)) // only if already initialized
			  {
				  Vertex3Ds vec;
				  vec.x = pball->m_oldpos[io].x - pball->m_oldpos[i3].x;
				  vec.y = pball->m_oldpos[io].y - pball->m_oldpos[i3].y;
				  vec.z = pball->m_oldpos[io].z - pball->m_oldpos[i3].z;
				  const float bc = (float)m_ptable->m_ballTrailStrength * powf(1.f - 1.f / max(vec.Length(), 1.0f), 16.0f); //!! 16=magic alpha falloff
				  const float r = min(pball->m_radius*0.9f, 2.0f*pball->m_radius / powf((float)(i2 + 2), 0.6f)); //!! consts are for magic radius falloff

				  if (bc > 0.f && r > FLT_MIN)
				  {
					  Vertex3Ds v = vec;
					  v.Normalize();
					  const Vertex3Ds up(0.f, 0.f, 1.f);
					  Vertex3Ds n = CrossProduct(v, up);
					  n.x *= r;
					  n.y *= r;
					  n.z *= r;

					  Vertex3D_NoTex2 rgv3D[4];
					  rgv3D[0].x = pball->m_oldpos[i3].x - n.x;
					  rgv3D[0].y = pball->m_oldpos[i3].y - n.y;
					  rgv3D[0].z = pball->m_oldpos[i3].z - n.z;
					  rgv3D[1].x = pball->m_oldpos[i3].x + n.x;
					  rgv3D[1].y = pball->m_oldpos[i3].y + n.y;
					  rgv3D[1].z = pball->m_oldpos[i3].z + n.z;
					  rgv3D[2].x = pball->m_oldpos[io].x + n.x;
					  rgv3D[2].y = pball->m_oldpos[io].y + n.y;
					  rgv3D[2].z = pball->m_oldpos[io].z + n.z;
					  rgv3D[3].x = pball->m_oldpos[io].x - n.x;
					  rgv3D[3].y = pball->m_oldpos[io].y - n.y;
					  rgv3D[3].z = pball->m_oldpos[io].z - n.z;

					  rgv3D[0].nx = rgv3D[1].nx = rgv3D[2].nx = rgv3D[3].nx = bc*(float)(1.0 / 255.0); //!! abuses normal for now for the color/alpha

					  rgv3D[0].tu = 0.5f + (float)(i2)*(float)(1.0 / (2.0*(MAX_BALL_TRAIL_POS - 1)));
					  rgv3D[0].tv = 0.f;
					  rgv3D[1].tu = rgv3D[0].tu;
					  rgv3D[1].tv = 1.f;
					  rgv3D[2].tu = 0.5f + (float)(i2 + 1)*(float)(1.0 / (2.0*(MAX_BALL_TRAIL_POS - 1)));
					  rgv3D[2].tv = 1.f;
					  rgv3D[3].tu = rgv3D[2].tu;
					  rgv3D[3].tv = 0.f;

					  if (num_rgv3D == 0)
					  {
						  rgv3D_all[0] = rgv3D[0];
						  rgv3D_all[1] = rgv3D[1];
						  rgv3D_all[2] = rgv3D[3];
						  rgv3D_all[3] = rgv3D[2];
					  }
					  else
					  {
						  rgv3D_all[num_rgv3D - 2].x = (rgv3D[0].x + rgv3D_all[num_rgv3D - 2].x)*0.5f;
						  rgv3D_all[num_rgv3D - 2].y = (rgv3D[0].y + rgv3D_all[num_rgv3D - 2].y)*0.5f;
						  rgv3D_all[num_rgv3D - 2].z = (rgv3D[0].z + rgv3D_all[num_rgv3D - 2].z)*0.5f;
						  rgv3D_all[num_rgv3D - 1].x = (rgv3D[1].x + rgv3D_all[num_rgv3D - 1].x)*0.5f;
						  rgv3D_all[num_rgv3D - 1].y = (rgv3D[1].y + rgv3D_all[num_rgv3D - 1].y)*0.5f;
						  rgv3D_all[num_rgv3D - 1].z = (rgv3D[1].z + rgv3D_all[num_rgv3D - 1].z)*0.5f;
						  rgv3D_all[num_rgv3D] = rgv3D[3];
						  rgv3D_all[num_rgv3D + 1] = rgv3D[2];
					  }

					  if (num_rgv3D == 0)
						  num_rgv3D += 4;
					  else
						  num_rgv3D += 2;
				  }
			  }
		  }

		  if (num_rgv3D > 0)
		  {
			  m_pin3d.m_pd3dDevice->SetRenderState(RenderDevice::ZWRITEENABLE, FALSE);
			  m_pin3d.EnableAlphaBlend(false);

			  ballShader->SetTechnique("RenderBallTrail");
			  ballShader->Begin(0);
			  m_pin3d.m_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, MY_D3DFVF_NOTEX2_VERTEX, rgv3D_all, num_rgv3D);
			  ballShader->End();
		  }
	  }

#ifdef DEBUG_BALL_SPIN        // draw debug points for visualizing ball rotation
        if (m_fShowFPS)
        {
            // set transform
            Matrix3D matOrig, matNew, matRot;
            matOrig = m_pin3d.GetWorldTransform();
            matNew.SetTranslation(pball->m_pos);
            matOrig.Multiply(matNew, matNew);
            matRot.SetIdentity();
            for (int j = 0; j < 3; ++j)
                for (int k = 0; k < 3; ++k)
                    matRot.m[j][k] = pball->m_orientation.m_d[k][j];
            matNew.Multiply(matRot, matNew);
            m_pin3d.m_pd3dDevice->SetTransform(TRANSFORMSTATE_WORLD, &matNew);
            m_pin3d.DisableAlphaBlend();

            // draw points
            const float ptsize = 5.0f;
            m_pin3d.m_pd3dDevice->SetRenderState((RenderDevice::RenderStates)D3DRS_POINTSIZE, *((DWORD*)&ptsize));
            m_pin3d.m_pd3dDevice->DrawPrimitiveVB( D3DPT_POINTLIST, MY_D3DFVF_TEX, m_ballDebugPoints, 0, 12 );

            // reset transform
            m_pin3d.m_pd3dDevice->SetTransform(TRANSFORMSTATE_WORLD, &matOrig);
        }
#endif

    }   // end loop over all balls

    //m_pin3d.DisableAlphaBlend(); //!! not necessary anymore

    // Set the render state to something that will always display.
	if (m_ToggleDebugBalls && m_DebugBalls)
        m_pin3d.m_pd3dDevice->SetRenderState ( RenderDevice::ZENABLE, TRUE );
    if (m_ToggleDebugBalls)
        m_ToggleDebugBalls = false;
}

struct DebugMenuItem
{
	int objectindex;
	std::vector<int> *pvdispid;
	HMENU hmenu;
};

void AddEventToDebugMenu(char *sz, int index, int dispid, LPARAM lparam)
{
	DebugMenuItem * const pdmi = (DebugMenuItem *)lparam;
	HMENU hmenu = pdmi->hmenu;
	const int menuid = ((pdmi->objectindex + 1) << 16) | pdmi->pvdispid->size();
	pdmi->pvdispid->push_back(dispid);
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
	m_pin3d.m_pd3dDevice->GetViewport(&vp);
	const float rClipWidth = (float)vp.Width*0.5f;
	const float rClipHeight = (float)vp.Height*0.5f;

	const float xcoord = ((float)x - rClipWidth) / rClipWidth;
	const float ycoord = (rClipHeight - (float)y) / rClipHeight;

	// Use the inverse of our 3D transform to determine where in 3D space the
	// screen pixel the user clicked on is at.  Get the point at the near
	// clipping plane (z=0) and the far clipping plane (z=1) to get the whole
	// range we need to hit test
	Vertex3Ds v3d, v3d2;
	mat3D.MultiplyVector(Vertex3Ds(xcoord, ycoord, 0), v3d);
	mat3D.MultiplyVector(Vertex3Ds(xcoord, ycoord, 1), v3d2);

	// Create a ray (ball) that travels in 3D space from the screen pixel at
	// the near clipping plane to the far clipping plane, and find what
	// it intersects with.
	Ball ballT;
	ballT.m_pos = v3d;
	ballT.m_vel = v3d2 - v3d;
	ballT.m_radius = 0;
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

	std::vector<HMENU> vsubmenu;
	HMENU hmenu = CreatePopupMenu();

	std::vector< std::vector<int>* > vvdispid;

	if (vhoHit.Size() == 0)
	{
		// Nothing was hit-tested
		return;
	}

	PauseMusic();

	for (int i = 0; i < vhoHit.Size(); i++)
	{
		HitObject * const pho = vhoHit.ElementAt(i);
		// Make sure we don't do the same object twice through 2 different Hitobjs.
		if (pho->m_pfedebug && (vpfe.IndexOf(pho->m_pfedebug) == -1))
		{
			vpfe.AddElement(pho->m_pfedebug);
			CComVariant var;
			DISPPARAMS dispparams = {
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
			vsubmenu.push_back(submenu);
			if (hr == S_OK)
			{
				WCHAR *wzT;
				wzT = V_BSTR(&var);
				AppendMenuW(hmenu, MF_STRING | MF_POPUP, (UINT_PTR)submenu, wzT);

				std::vector<int> *pvdispid = new std::vector<int>();
				vvdispid.push_back(pvdispid);

				DebugMenuItem dmi;
				dmi.objectindex = i;
				dmi.pvdispid = pvdispid;
				dmi.hmenu = submenu;
				EnumEventsFromDispatch(pho->m_pfedebug->GetDispatch(), AddEventToDebugMenu, (LPARAM)&dmi);
			}

			IDebugCommands * const pdc = pho->m_pfedebug->GetDebugCommands();
			if (pdc)
			{
				std::vector<int> vids;
				std::vector<int> vcommandid;

				pdc->GetDebugCommands(vids, vcommandid);
				for (unsigned l = 0; l < vids.size(); l++)
				{
					LocalString ls(vids[l]);
					AppendMenu(submenu, MF_STRING, ((i + 1) << 16) | vcommandid[l] | 0x8000, ls.m_szbuffer);
				}
			}
		}
		else
		{
			vvdispid.push_back(NULL); // Put a spacer in so we can keep track of indexes
		}
	}

	POINT pt;
	pt.x = x;
	pt.y = y;
	ClientToScreen(m_hwnd, &pt);

	const int icmd = TrackPopupMenuEx(hmenu, TPM_RETURNCMD | TPM_RIGHTBUTTON,
		pt.x, pt.y, m_hwnd, NULL);

	if (icmd != 0 && vsubmenu.size() > 0)
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
			const int dispid = (*vvdispid[highword])[lowword];
			m_pactiveball = m_pactiveballDebug;
			pfe->FireGroupEvent(dispid);
			m_pactiveball = NULL;
		}
	}

	DestroyMenu(hmenu);
	for (unsigned i = 0; i < vsubmenu.size(); i++)
	{
		DestroyMenu(vsubmenu[i]);
	}

	for (unsigned i = 0; i < vvdispid.size(); i++)
		delete vvdispid[i];

	UnpauseMusic();
}

LRESULT CALLBACK PlayerWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case MM_MIXM_CONTROL_CHANGE:
		mixer_get_volume();
		break;

	case WM_CLOSE:
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
		g_pplayer->Shutdown();

		delete g_pplayer; // needs to be deleted here, as code below relies on it being NULL
		g_pplayer = NULL;

		g_pvp->SetEnableToolbar();
		mixer_shutdown();
		hid_shutdown();
		// modification to m_vedit of each table after playing them must be done here, otherwise VP will crash (WTF?!)
		playedTable->RestoreLayers();
		SetForegroundWindow(g_pvp->m_hwnd);
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
			g_pplayer->m_fNoTimeCorrect = true;
		}
		break;
#endif
#endif
	case WM_RBUTTONUP:
	{
		if (g_pplayer->m_fDebugMode)
		{
			const int x = lParam & 0xffff;
			const int y = (lParam >> 16) & 0xffff;
			g_pplayer->DoDebugObjectMenu(x, y);
		}
		return 0;
	}
	break;

	case WM_POINTERDOWN:
	case WM_POINTERUP:
	{
#ifndef TEST_TOUCH_WITH_MOUSE
		if (!GetPointerInfo)
			GetPointerInfo = (pGPI)GetProcAddress(GetModuleHandle(TEXT("user32.dll")),
			"GetPointerInfo");
		if (GetPointerInfo)
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
					if ((g_pplayer->m_touchregion_pressed[i] != (uMsg == WM_POINTERDOWN)) && Intersect(touchregion[i], g_pplayer->m_width, g_pplayer->m_height, pointerInfo.ptPixelLocation, fmodf(g_pplayer->m_ptable->m_BG_rotation[g_pplayer->m_ptable->m_BG_current_set], 360.0f) != 0.f))
					{
						g_pplayer->m_touchregion_pressed[i] = (uMsg == WM_POINTERDOWN);

						DIDEVICEOBJECTDATA didod;
						didod.dwOfs = g_pplayer->m_rgKeys[touchkeymap[i]];
						didod.dwData = g_pplayer->m_touchregion_pressed[i] ? 0x80 : 0;
						g_pplayer->m_pininput.PushQueue(&didod, APP_KEYBOARD/*, curr_time_msec*/);
					}
			}
		}
	}
	break;

	case WM_ACTIVATE:
		if (wParam != WA_INACTIVE)
		{
			g_pplayer->m_fGameWindowActive = true;
			SetCursor(NULL);
			g_pplayer->m_fNoTimeCorrect = true;
			g_pplayer->m_fPause = false;
		}
		else
		{
			g_pplayer->m_fGameWindowActive = false;
			g_pplayer->m_fPause = true;
		}
		g_pplayer->RecomputePauseState();
		break;

	case WM_EXITMENULOOP:
		g_pplayer->m_fNoTimeCorrect = true;
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
#ifdef _WIN64
	{0, IDC_PLAY, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, 0, 0, 0, 0, IDS_PLAY, 0},
	{1, IDC_PAUSE, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, 0, 0, 0, 0, IDS_PAUSE, 1},
	{2, IDC_STEP, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, 0, 0, 0, 0, IDS_STEP, 2},
#else
	{ 0, IDC_PLAY, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, IDS_PLAY, 0 },
	{ 1, IDC_PAUSE, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, IDS_PAUSE, 1 },
	{ 2, IDC_STEP, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, IDS_STEP, 2 },
#endif
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
			(rcMain.right + rcMain.left) / 2 - (rcDialog.right - rcDialog.left) / 2,
			(rcMain.bottom + rcMain.top) / 2 - (rcDialog.bottom - rcDialog.top) / 2,
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
		NMHDR *pnmh = (LPNMHDR)lParam;
		SCNotification *pscnmh = (SCNotification *)lParam;
		//HWND hwndRE = pnmh->hwndFrom;
		const int code = pnmh->code;

		switch (code)
		{
		case SCN_CHARADDED:
			if (pscnmh->ch == '\n')
			{
				SendMessage(pnmh->hwndFrom, SCI_DELETEBACK, 0, 0);

				const size_t curpos = SendMessage(pnmh->hwndFrom, SCI_GETCURRENTPOS, 0, 0);
				const size_t line = SendMessage(pnmh->hwndFrom, SCI_LINEFROMPOSITION, curpos, 0);
				const size_t lineStart = SendMessage(pnmh->hwndFrom, SCI_POSITIONFROMLINE, line, 0);
				const size_t lineEnd = SendMessage(pnmh->hwndFrom, SCI_GETLINEENDPOSITION, line, 0);

				char * const szText = new char[lineEnd - lineStart + 1];
				TextRange tr;
				tr.chrg.cpMin = lineStart;
				tr.chrg.cpMax = lineEnd;
				tr.lpstrText = szText;
				SendMessage(pnmh->hwndFrom, SCI_GETTEXTRANGE, 0, (LPARAM)&tr);

				const size_t maxlines = SendMessage(pnmh->hwndFrom, SCI_GETLINECOUNT, 0, 0);

				if (maxlines == line + 1)
				{
					// need to add a new line to the end
					SendMessage(pnmh->hwndFrom, SCI_DOCUMENTEND, 0, 0);
					SendMessage(pnmh->hwndFrom, SCI_ADDTEXT, lstrlen("\n"), (LPARAM)"\n");
				}
				else
				{
					const size_t pos = SendMessage(pnmh->hwndFrom, SCI_POSITIONFROMLINE, line + 1, 0);
					SendMessage(pnmh->hwndFrom, SCI_SETCURRENTPOS, pos, 0);
				}

				g_pplayer->m_ptable->m_pcv->EvaluateScriptStatement(szText);
				delete[] szText;
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
		SendMessage(hwndToolbar, TB_CHECKBUTTON, IDC_PLAY, PlayDown);
		SendMessage(hwndToolbar, TB_CHECKBUTTON, IDC_PAUSE, PauseDown);
		SendMessage(hwndToolbar, TB_CHECKBUTTON, IDC_STEP, StepDown);
	}
	break;

	case WM_CLOSE:
		g_pplayer->m_PauseTimeTarget = 0;
		g_pplayer->m_fUserDebugPaused = false;
		g_pplayer->RecomputePseudoPauseState();
		g_pplayer->m_fDebugMode = false;
		ShowWindow(hwndDlg, SW_HIDE);
		break;

	case WM_ACTIVATE:
		g_pplayer->m_fDebugWindowActive = (wParam != WA_INACTIVE);
		g_pplayer->RecomputePauseState();
		g_pplayer->RecomputePseudoPauseState();
		break;

	case RESIZE_FROM_EXPAND:
	{
		const size_t state = SendDlgItemMessage(hwndDlg, IDC_EXPAND, BM_GETCHECK, 0, 0);
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
				HWND hwndControl = GetDlgItem(hwndDlg, IDC_BALL_THROWING);
				size_t checked = SendMessage(hwndControl, BM_GETCHECK, 0, 0);
				g_pplayer->m_fThrowBalls = !!checked;
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
			(rcMain.right + rcMain.left) / 2 - (rcDialog.right - rcDialog.left) / 2,
			(rcMain.bottom + rcMain.top) / 2 - (rcDialog.bottom - rcDialog.top) / 2,
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
					g_pplayer->m_fDebugMode = true;
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
