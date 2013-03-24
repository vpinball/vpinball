#pragma once

#ifdef _DEBUG_
#define MOUSEPAUSE 1
#endif

#define DEFAULT_PLAYER_WIDTH 912

class IBlink;

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
	virtual ~Player();

	HRESULT Init(PinTable * const ptable, const HWND hwndProgress, const HWND hwndProgressName);
	void InitWindow();
#ifdef ULTRAPIN
	void InitDMDHackWindow();
#endif
	void InitKeys();
	void InitRegValues();

	virtual IEditable *GetIEditable() { return (IEditable*)this; }

	void ReOrder();
	void InitStatic(HWND hwndProgress);
	void InitAnimations(HWND hwndProgress);

	void Render();

	void DrawBallShadows();
	void DrawBalls();

	void DrawAlphas();

	void PhysicsSimulateCycle(float dtime, const U64 startTime);

	void InvalidateRect(const RECT * const prc);
#ifdef ULTRAPIN
	void DrawLightHack ();
#endif
	void EraseBall(Ball *pball);

	Ball *CreateBall(const float x, const float y, const float z, const float vx, const float vy, const float vz, const float radius = 25.0f);
	void DestroyBall(Ball *pball);

	void CreateBoundingHitShapes(Vector<HitObject> *pvho);

	void InitDebugHitStructure();
	void DoDebugObjectMenu(int x, int y);

	void PauseMusic();
	void UnpauseMusic();

	void RecomputePauseState();
	void RecomputePseudoPauseState();

	void UltraNudge();
	void UltraNudgeX( const int x, const int j );
	void UltraNudgeY( const int y, const int j );
	int  UltraNudgeGetTilt(); // returns non-zero when appropriate to set the tilt switch

	void UltraPlunger();
	void mechPlungerIn( const int z );		

#ifdef PLAYBACK
	float ParseLog(LARGE_INTEGER *pli1, LARGE_INTEGER *pli2);
#endif

	HWND m_hwnd;
#ifdef ULTRAPIN
	HWND m_dmdhackhwnd;
#endif

	PinTable *m_ptable;

	Pin3D m_pin3d;

	int m_timeCur;

	int m_frotate;

	int DeadZ;

	Ball *m_pactiveball;		// ball the script user can get with ActiveBall
	Ball *m_pactiveballDebug;	// ball the debugger will use as Activeball when firing events

	Vector<Ball> m_vball;
	Vector<UpdateRect> m_vupdaterect;
	Vector<AnimObject> m_vscreenupdate;
	Vector<HitTimer> m_vht;
	Vector<IBlink> m_vblink;	// Lights which are set to blink

	BOOL m_fAccelerometer;		//true if electronic Accelerometer enabled
	BOOL m_AccelNormalMount;	//true if normal mounting (left hand coordinates)
	float m_AccelAngle;			// 0 Radians rotated counterclockwise (GUI is lefthand coordinates)
	float m_AccelAmp;			// Accelerometer gain 
	float m_AccelAmpX;			// Accelerometer gain X axis 
	float m_AccelAmpY;			// Accelerometer gain Y axis
	float m_AccelMAmp;			// manual input gain, generally from joysticks

	U32 m_jolt_amount;
	U32 m_tilt_amount;
	U32 m_jolt_trigger_time;
	U32 m_tilt_trigger_time;

	Vertex3Ds m_gravity;

	BOOL m_fDetectScriptHang;
	BOOL m_fNoTimeCorrect;		// Used so the frame after debugging does not do normal time correction
	PinInput m_pininput;

	float m_NudgeX;
	float m_NudgeY;
	float m_NudgeBackX;
	float m_NudgeBackY;
	int m_NudgeManual;			//index of joystick that has manual control

	EnumAssignKeys m_rgKeys[eCKeys]; //Player's key assignments

	HWND m_hwndDebugOutput;
	HWND m_hwndDebugger;

	Vector<CLSID> m_controlclsidsafe; // ActiveX control types which have already been okayed as being safe

	BOOL m_fCloseDown;			// Whether to shut down the player at the end of this frame
	int m_fCloseType;			// if 0 exit player and close application if started minimized, if 1 close application always

	int m_sleeptime;			// time to sleep during each frame - can helps side threads like vpinmame

	int m_nudgetime;

	GPINFLOAT m_pixelaspectratio;

	BOOL m_fVSync;

	BOOL m_fFXAA;

	int m_fStereo3D; // 0=off, 1=top/down, 2=interlaced/LG //!! support sidebyside, too?
	BOOL m_fStereo3Denabled;

	XAudPlayer *m_pxap;

	BOOL m_fDebugMode;

	BOOL m_DebugBalls;			 // Draw balls in the foreground.
	BOOL m_ToggleDebugBalls;

	BOOL m_fEnableRegionUpdates; // Use the CleanBlt flag below or not, nowadays some setups are slower when using it (especially NVIDIA Optimus setups)
	BOOL m_fCleanBlt;			 // We can do smart blitting next frame

	BOOL m_fPlayMusic;
	BOOL m_fPlaySound;
	int m_MusicVolume;
	int m_SoundVolume;

	int m_lastcursorx, m_lastcursory; // used for the dumb task of seeing if the mouse has really moved when we get a WM_MOUSEMOVE message

	int m_LastKnownGoodCounter;
	int m_ModalRefCount;

	U32 c_hitcnts;
	U32 c_collisioncnt;
	U32 c_contactcnt;
	U32 c_staticcnt;
	U32 c_embedcnts;
	int movedPlunger;			// has plunger moved, must have moved at least three times
	U32 LastPlungerHit;			// The last time the plunger was in contact (at least the vicinity) of the ball.
	int Coins;					// The number of coins queued to be inserted.  These were sent from the shell after the load.
	float curMechPlungerPos;

	bool m_fDrawCursor;
	bool m_fGameWindowActive;
	bool m_fUserDebugPaused;
	bool m_fDebugWindowActive;
	
private:
	Vector<HitObject> m_vho;
	Vector<AnimObject> m_vmover;

	Vector<Ball> m_vballDelete;	// Balls to free at the end of the frame

	HitOctree m_hitoctree;
	HitOctree m_shadowoctree;

	Vector<HitObject> m_vdebugho;
	HitOctree m_debugoctree;

	int m_width, m_height;

	int m_screenwidth, m_screenheight, m_screendepth, m_refreshrate;
	BOOL m_fFullScreen;

	U64 m_liStartTime;
	U64 m_curPhysicsFrameTime;	// Time when the last frame was drawn
	U64 m_nextPhysicsFrameTime;	// time at which the next physics update should be

    // caching hitable alpha ramps & primitives to speed up DrawAlphas()
	Vector< Hitable > m_vhitalpha;

	int curAccel_x[PININ_JOYMXCNT];
	int curAccel_y[PININ_JOYMXCNT];

	BOOL m_fPlayback;
	char m_szPlaybackFile;

	BOOL m_fCheckBlt;

	BOOL m_fBallShadows;
	BOOL m_fBallDecals;
	BOOL m_fBallAntialias;

	BOOL m_fStereo3DAA;
	BOOL m_fStereo3DY;

	float m_BallStretchX;
	float m_BallStretchY;

	float m_NudgePosX;
	float m_NudgePosY;

	float m_NudgeVelX;
	float m_NudgeVelY;

	float m_NudgeAccX;
	float m_NudgeAccY;

	int curPlunger;				// assume

	HANDLE m_hSongCompletionEvent;

	int m_pauseRefCount;

	bool m_fPseudoPause;		// Nothing is moving, but we're still redrawing

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

#ifdef LOG
	FILE *m_flog;
	int m_timestamp;
#endif

#ifdef PLAYBACK
	FILE *m_fplaylog;
#endif

#ifdef FPS
	int m_lastfpstime;
	int m_cframes;
	int m_fps;
   int m_fpsAvg;
   int m_fpsCount;
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

#ifdef STEPPING
public:
	int m_PauseTimeTarget;
	bool m_fPause;
	bool m_fStep;
#endif
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

#ifdef DONGLE_SUPPORT
enum
{
	// Halt application.
	DONGLE_STATUS_NOTFOUND			= 0,
	DONGLE_STATUS_INCORRECT,

	// Run application.
	DONGLE_STATUS_OK
};

int get_dongle_status();
#endif
