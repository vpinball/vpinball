#pragma once

#ifdef _DEBUG_
#define MOUSEPAUSE 1
#endif

#define DEFAULT_PLAYER_WIDTH 912

class IBlink;

enum
{
	// Halt application.
	DONGLE_STATUS_NOTFOUND			= 0,
	DONGLE_STATUS_INCORRECT,

	// Run application.
	DONGLE_STATUS_OK
};


#ifdef ULTRAPIN
enum 
{
	// Firepower
    LIGHTHACK_FIREPOWER_P1			= 0,
    LIGHTHACK_FIREPOWER_P2,
    LIGHTHACK_FIREPOWER_P3,
    LIGHTHACK_FIREPOWER_P4,

	// Black Knight 2000
    LIGHTHACK_BK2K_R,
    LIGHTHACK_BK2K_A,
    LIGHTHACK_BK2K_N,
    LIGHTHACK_BK2K_S,
    LIGHTHACK_BK2K_O,
    LIGHTHACK_BK2K_M,

	LIGHTHACK_MAX
};
#endif

enum EnumAssignKeys
	{
	eLeftFlipperKey,
	eRightFlipperKey,
	eLeftTiltKey,
	eRightTiltKey,
	eCenterTiltKey,
	ePlungerKey,
	eFrameCount,
	eDBGBalls,
	eAddCreditKey,
	eAddCreditKey2,
	eStartGameKey,
	eMechanicalTilt,
	eRightMagnaSave,
	eLeftMagnaSave,
	eExitGame,
	eVolumeUp,
	eVolumeDown,
	eEnable3D,
	eCKeys
	};

class Player
	{
public:
	Player();
	~Player();

	HRESULT Init(PinTable *ptable, HWND hwndProgress, HWND hwndProgressName, BOOL fCheckForCache);
	void InitWindow();
#ifdef ULTRAPIN
	void InitDMDHackWindow();
#endif
	void InitKeys();
	void InitRegValues();

	virtual IEditable *GetIEditable() {return (IEditable*)this;} 

	void ReOrder();
	void InitStatic(HWND hwndProgress);
	void InitAnimations(HWND hwndProgress);

	void Render();

	void DrawBallShadows();
	void DrawBalls();

	void DrawAcrylics();

	void PhysicsSimulateCycle(float dtime, const U64 startTime);

	void InvalidateRect(RECT * const prc);
#ifdef ULTRAPIN
	void DrawLightHack ();
#endif
	void EraseBall(Ball *pball);

	Ball *CreateBall(const float x, const float y, const float z, const float vx, const float vy, const float vz, const float radius);	
	Ball *CreateBall(const float x, const float y, const float z, const float vx, const float vy, const float vz);
	void DestroyBall(Ball *pball);

	void CreateBoundingHitShapes(Vector<HitObject> *pvho);

	void InitDebugHitStructure();
	void DoDebugObjectMenu(int x, int y);

	void PauseMusic();
	void UnpauseMusic();

	void RecomputePauseState();
	void RecomputePseudoPauseState();

	void UltraNudge();
	void UltraNudgeX( int x, int j );
	void UltraNudgeY( int y, int j );
	int  UltraNudgeGetTilt(); // returns non-zero when appropriate to set the tilt switch

#ifdef ULTRACADE
	void UltraPlunger();
	void mechPlungerIn( int z);		
#endif

#ifdef PLAYBACK
	float ParseLog(LARGE_INTEGER *pli1, LARGE_INTEGER *pli2);
#endif

	Vector<Ball> m_vball;
	Vector<HitObject> m_vho;
	Vector<AnimObject> m_vmover;
	Vector<AnimObject> m_vscreenupdate;
	//Vector<HitObject> m_vmover;
	//Vector<HitObject> m_vscreenupdate;
	Vector<HitTimer> m_vht;
	Vector<IBlink> m_vblink; // Lights which are set to blink

	Vector<Ball> m_vballDelete; // Balls to free at the end of the frame

	Ball *m_pactiveball; // ball the script user can get with ActiveBall

	Vector<UpdateRect> m_vupdaterect;

	HitOctree m_hitoctree;

	HitOctree m_shadowoctree;

	Vector<HitObject> m_vdebugho;
	HitOctree m_debugoctree;
	Ball *m_pactiveballDebug;	// ball the debugger will use as Activeball when firing events

	BOOL m_fAccelerometer;		//true if electronic Accelerometer enabled
	BOOL m_AccelNormalMount;	//true if normal mounting (left hand coordinates)
	float m_AccelAngle;			// 0 Radians rotated counterclockwise (GUI is lefthand coordinates)
	float m_AccelAmp;			// Accelerometer gain 
	float m_AccelAmpX;			// Accelerometer gain X axis 
	float m_AccelAmpY;			// Accelerometer gain Y axis
	float m_AccelMAmp;			// manual input gain, generally from joysticks

	U32 m_jolt_amount;
	U32 m_tilt_amount;
	int DeadZ;
	U32 m_jolt_trigger_time;
	U32 m_tilt_trigger_time;

	int m_width, m_height;

	int m_screenwidth, m_screenheight, m_screendepth, m_refreshrate, m_frotate;
	BOOL m_fFullScreen;

	PinTable *m_ptable;
	HWND m_hwnd;
#ifdef ULTRAPIN
	HWND m_dmdhackhwnd;
#endif
	int m_timerid;
	HBITMAP m_hbmOffScreen;

	int m_timeCur;

	Pin3D m_pin3d;

private:
	U64 m_liStartTime;
	U64 m_curPhysicsFrameTime; // Time when the last frame was drawn

	BOOL m_fLShiftDown; // For our internal bookkeeping since windows won't do it for us
	BOOL m_fRShiftDown;

	U64 m_PhysicsStepTime; // ticks to go between each gravity update
	U64 m_nextPhysicsFrameTime; // time at which the next physics update should be
	U64 m_liPhysicsCalced;

    // caching hitable ramps & primitives to speed up DrawAcrylics()
	Vector< Hitable > m_vhitacrylic;

public:
	BOOL m_fDetectScriptHang;
	BOOL m_fNoTimeCorrect; // Used so the frame after debugging does not do normal time correction
	PinInput m_pininput;
	Level m_mainlevel; // level object for main table level;

private:
	BOOL m_fPlayback;
	char m_szPlaybackFile;

	BOOL m_fCheckBlt;
	BOOL m_fWasteTime;
	BOOL m_fWasteTime2;

	BOOL m_fBallShadows;
	BOOL m_fBallDecals;
	BOOL m_fBallAntialias;

	BOOL m_fStereo3DAA;
	BOOL m_fStereo3DY;

public:
	BOOL m_fVSync;

	BOOL m_fStereo3D;
	BOOL m_fStereo3Denabled;

	float m_BallStretchX;
	float m_BallStretchY;

	float m_NudgeX;
	float m_NudgeY;
	int m_NudgeManual;		//index of joystick that has manual control
	int m_nudgetime;

	float m_NudgePosX;
	float m_NudgePosY;

	float m_NudgeVelX;
	float m_NudgeVelY;

	float m_NudgeAccX;
	float m_NudgeAccY;

	float m_NudgeBackX;
	float m_NudgeBackY;

	BOOL m_fCleanBlt; // We can do smart blitting next frame;

	EnumAssignKeys m_rgKeys[eCKeys]; //Player's key assignments;

	HANDLE m_hSongCompletionEvent;

	XAudPlayer *m_pxap;

	BOOL m_fPlayMusic;
	BOOL m_fPlaySound;
	int m_MusicVolume;
	int m_SoundVolume;

	BOOL m_fCloseDown; // Whether to shut down the player at the end of this frame
	int m_fCloseType; // if 0 exit player and close application if started minimized, if 1 close application always

	BOOL m_DebugBalls;			// Draw balls in the foreground.
	BOOL m_ToggleDebugBalls;

	int m_sleeptime; // time to sleep during each frame - can helps side threads like vpinmame

	Vector<CLSID> m_controlclsidsafe; // ActiveX control types which have already been okayed as being safe

	GPINFLOAT m_pixelaspectratio;

	BOOL m_fDebugMode;
	HWND m_hwndDebugger;
	HWND m_hwndDebugOutput;

#ifdef ULTRAPIN
	U32 m_LastUpdateTime[LIGHTHACK_MAX];
	BOOL m_LightHackReadyForDrawLightHackFn[LIGHTHACK_MAX];
	BOOL m_LightHackCurrentState[LIGHTHACK_MAX];
	BOOL m_LightHackPreviousState[LIGHTHACK_MAX];
	BOOL m_LightHackCurrentAnimState[LIGHTHACK_MAX];
	BOOL m_LightHackPreviousAnimState[LIGHTHACK_MAX];
	int m_LightHackX[LIGHTHACK_MAX];
	int m_LightHackY[LIGHTHACK_MAX];
	int m_LightHackWidth[LIGHTHACK_MAX];
	int m_LightHackHeight[LIGHTHACK_MAX];
#endif

#ifdef FPS
private:
	int m_lastfpstime;
	int m_cframes;
	int m_fps;
	U64 m_count;
	U64 m_total;
	U32 m_max;
	U64 m_phys_total;
	U64 m_phys_max;
	U64 m_phys_total_iterations;
	U64 m_phys_max_iterations;
	BOOL m_fShowFPS;

public:
	void ToggleFPS();
	void DisableFPS();
	void EnableFPS();
#endif

#ifdef LOG
	FILE *m_flog;
	int m_timestamp;
#endif

#ifdef PLAYBACK
	FILE *m_fplaylog;
#endif

#ifdef STEPPING
	BOOL m_fPause;
	int m_PauseTimeTarget;
	BOOL m_fStep;
#endif

	BOOL m_fPseudoPause; // Nothing is moving, but we're still redrawing

	BOOL m_fGameWindowActive;
	BOOL m_fDebugWindowActive;
	BOOL m_fUserDebugPaused;

	int m_LastKnownGoodCounter;
	int m_ModalRefCount;

	BOOL m_fDrawCursor;
	int m_lastcursorx, m_lastcursory; // used for the dumb task of seeing if the mouse has really moved when we get a WM_MOUSEMOVE message
	
	int m_pauseRefCount;
	};

F32 GetX(); // Get the -1.0f to +1.0f values from joystick input tilt sensor / ushock
F32 GetY();

int get_dongle_status();
