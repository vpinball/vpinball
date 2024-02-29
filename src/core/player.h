#pragma once

#include "renderer/typedefs3D.h"
#include "renderer/Renderer.h"
#include "physics/PhysicsEngine.h"
#include "ui/Debugger.h"
#include "ui/LiveUI.h"
#include "pininput.h"

#ifdef __STANDALONE__
#include "standalone/inc/common/WindowManager.h"
#endif

#define DEFAULT_PLAYER_WIDTH 1024
#define DEFAULT_PLAYER_FS_WIDTH 1920
#define DEFAULT_PLAYER_FS_REFRESHRATE 60

constexpr int DBG_SPRITE_SIZE = 1024;

class VRDevice;

// NOTE that the following four definitions need to be in sync in their order!
enum EnumAssignKeys
{
   eLeftFlipperKey,
   eRightFlipperKey,
   eStagedLeftFlipperKey,
   eStagedRightFlipperKey,
   eLeftTiltKey,
   eRightTiltKey,
   eCenterTiltKey,
   ePlungerKey,
   eFrameCount,
   eDBGBalls,
   eDebugger,
   eAddCreditKey,
   eAddCreditKey2,
   eStartGameKey,
   eMechanicalTilt,
   eRightMagnaSave,
   eLeftMagnaSave,
   eExitGame,
   eVolumeUp,
   eVolumeDown,
   eLockbarKey,
   eEnable3D,
   eTableRecenter,
   eTableUp,
   eTableDown,
   eEscape,
   ePause,
   eTweak,
   eCKeys
};

static const string regkey_string[eCKeys] = {
   "LFlipKey"s,
   "RFlipKey"s,
   "StagedLFlipKey"s,
   "StagedRFlipKey"s,
   "LTiltKey"s,
   "RTiltKey"s,
   "CTiltKey"s,
   "PlungerKey"s,
   "FrameCount"s,
   "DebugBalls"s,
   "Debugger"s,
   "AddCreditKey"s,
   "AddCreditKey2"s,
   "StartGameKey"s,
   "MechTilt"s,
   "RMagnaSave"s,
   "LMagnaSave"s,
   "ExitGameKey"s,
   "VolumeUp"s,
   "VolumeDown"s,
   "LockbarKey"s,
   "Enable3DKey"s,
   "TableRecenterKey"s,
   "TableUpKey"s,
   "TableDownKey"s,
   "EscapeKey"s,
   "PauseKey"s,
   "TweakKey"s
};

static constexpr int regkey_defdik[eCKeys] = {
   DIK_LSHIFT,
   DIK_RSHIFT,
   DIK_LWIN,
   DIK_RALT,
   DIK_Z,
   DIK_SLASH,
   DIK_SPACE,
   DIK_RETURN,
#if !defined(__APPLE__) && !defined(__ANDROID__)
   DIK_F11,
#else
   DIK_F1,
#endif
   DIK_O,
   DIK_D,
   DIK_5,
   DIK_4,
   DIK_1,
   DIK_T,
   DIK_RCONTROL,
   DIK_LCONTROL,
   DIK_Q,
   DIK_EQUALS,
   DIK_MINUS,
   DIK_LALT,
   DIK_F10,
   DIK_NUMPAD5,
   DIK_NUMPAD8,
   DIK_NUMPAD2,
   DIK_ESCAPE,
   DIK_P,
   DIK_F12
};

static constexpr int regkey_idc[eCKeys] = {
   IDC_LEFTFLIPPER,
   IDC_RIGHTFLIPPER,
   IDC_STAGEDLEFTFLIPPER,
   IDC_STAGEDRIGHTFLIPPER,
   IDC_LEFTTILT,
   IDC_RIGHTTILT,
   IDC_CENTERTILT,
   IDC_PLUNGER_TEXT,
   IDC_FRAMECOUNT,
   IDC_DEBUGBALL,
   IDC_DEBUGGER,
   IDC_ADDCREDIT,
   IDC_ADDCREDITKEY2,
   IDC_STARTGAME,
   IDC_MECHTILT,
   IDC_RMAGSAVE,
   IDC_LMAGSAVE,
   IDC_EXITGAME,
   IDC_VOLUMEUP,
   IDC_VOLUMEDN,
   IDC_LOCKBAR,
   -1, //!! missing in key dialog! (Enable/disable 3D stereo)
   IDC_TABLEREC_TEXT,
   IDC_TABLEUP_TEXT,
   IDC_TABLEDOWN_TEXT,
   -1, // Escape
   IDC_PAUSE,
   IDC_TWEAK
};

#define MAX_TOUCHREGION 11

static constexpr RECT touchregion[MAX_TOUCHREGION] = { //left,top,right,bottom (in % of screen)
   { 0, 0, 50, 10 },      // Extra Ball
   { 50, 0, 100, 10 },    // Escape
   { 0, 10, 50, 30 },     // 2nd Left Button
   { 50, 10, 100, 30 },   // 2nd Right Button
   { 0, 30, 50, 60 },     // Left Nudge Button
   { 50, 30, 100, 60 },   // Right Nudge Button
   { 0, 60, 30, 90 },     // 1st Left Button (Flipper)
   { 30, 60, 70, 100 },   // Center Nudge Button
   { 70, 60, 100, 90 },   // 1st Right Button (Flipper)
   { 0, 90, 30, 100 },    // Start
   { 70, 90, 100, 100 },  // Plunger
};

static constexpr EnumAssignKeys touchkeymap[MAX_TOUCHREGION] = {
   eAddCreditKey, //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
   eEscape,
   eLeftMagnaSave,
   eRightMagnaSave,
   eLeftTiltKey,
   eRightTiltKey,
   eLeftFlipperKey,
   eCenterTiltKey,
   eRightFlipperKey,
   eStartGameKey,
   ePlungerKey
};

enum InfoMode
{
   IF_NONE,
   IF_FPS,
   IF_PROFILING,
   IF_STATIC_ONLY,
   IF_DYNAMIC_ONLY,
   IF_AO_ONLY,
   IF_LIGHT_BUFFER_ONLY,
   IF_RENDER_PROBES,
   IF_BAM_MENU,
   IF_INVALID
};

enum ProfilingMode
{
   PF_DISABLED,
   PF_ENABLED,
};


////////////////////////////////////////////////////////////////////////////////

struct TimerOnOff
{
   HitTimer* m_timer;
   bool m_enabled;
};

class Player : public CWnd
{
public:
   Player(PinTable *const editor_table, PinTable *const live_table, const int playMode);
   virtual ~Player();

   void CreateWnd(HWND parent = 0);
   virtual void PreRegisterClass(WNDCLASS& wc) override;
   virtual void PreCreate(CREATESTRUCT& cs) override;
   virtual void OnInitialUpdate() override;
   void OnClose() override;
   virtual LRESULT WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
   void LockForegroundWindow(const bool enable);

   string GetPerfInfo();

   void RecomputePauseState();
   void RecomputePseudoPauseState();

private:
   HRESULT Init(); // Called from OnInitialUpdate callback (after native window creation)

   const int m_playMode;

public:
   PinTable *const m_pEditorTable; // The untouched version of the table, as it is in the editor (The Player needs it to interact with the UI)
   PinTable *const m_ptable; // The played table, which can be modified by the script

   U32 m_time_msec; // current physics time
   U32 m_last_frame_time_msec; // used for non-physics controlled animations to update once per-frame only, aligned with m_time_msec

   Ball *m_pactiveball; // ball the script user can get with ActiveBall
   Ball *m_pactiveballDebug; // ball the debugger will use as Activeball when firing events
   Ball *m_pactiveballBC; // ball that the ball control UI will use
   Vertex3Ds *m_pBCTarget; // if non-null, the target location for the ball to roll towards

   void FireSyncController();


#pragma region Main Loop
public:
   void OnIdle();

   VideoSyncMode m_videoSyncMode = VideoSyncMode::VSM_FRAME_PACING;
   int m_maxFramerate = 0; // targeted refresh rate in Hz, if larger refresh rate it will limit FPS by uSleep() //!! currently does not work adaptively as it would require IDirect3DDevice9Ex which is not supported on WinXP
   int m_mainLoopPhase = 0;
   U64 m_lastPresentFrameTick = 0;
   bool m_lastFrameSyncOnVBlank;
   bool m_lastFrameSyncOnFPS;
   bool m_curFrameSyncOnVBlank = false;
   bool m_curFrameSyncOnFPS = false;
   U64 m_startFrameTick; // System time in us when render frame was started (beginning of frame animation then collect,...)
#pragma endregion


#pragma region MechPlunger
public:
   void MechPlungerIn(const int z);

   U32 m_movedPlunger; // has plunger moved, must have moved at least three times
   U32 m_LastPlungerHit; // The last time the plunger was in contact (at least the vicinity) of the ball.
   float m_curMechPlungerPos;

   void MechPlungerUpdate();

private:
   int m_curPlunger;
#pragma endregion


#pragma region Nudge
public:
   void ReadAccelerometerCalibration();
   void SetNudgeX(const int x, const int joyidx);
   void SetNudgeY(const int y, const int joyidx);
   const Vertex2D& GetRawAccelerometer() const;
   
   #ifdef UNUSED_TILT
   int NudgeGetTilt(); // returns non-zero when appropriate to set the tilt switch
   #endif

   float m_NudgeShake; // whether to shake the screen during nudges and how much
   int2 m_curAccel[PININ_JOYMXCNT];

private:
   mutable Vertex2D m_accelerometer; // lazily evaluated sum of joystick mapped accelerometers, applying clamping then gain
   mutable bool m_accelerometerDirty = true;
   bool m_accelerometerEnabled; // true if electronic accelerometer enabled
   bool m_accelerometerFaceUp; // true is Normal Mounting (Left Hand Coordinates)
   float m_accelerometerAngle; // 0 degrees rotated counterclockwise (GUI is lefthand coordinates)
   float m_accelerometerSensitivity;
   Vertex2D m_accelerometerGain; // Accelerometer gain X/Y axis
   int2 m_accelerometerMax; // Accelerometer max value X/Y axis
#pragma endregion


#pragma region Physics
public:
   Ball *CreateBall(const float x, const float y, const float z, const float vx, const float vy, const float vz, const float radius = 25.0f, const float mass = 1.0f);
   void DestroyBall(Ball *pball);

   PhysicsEngine* m_physics = nullptr;

   vector<Ball *> m_vball;
   vector<Hitable *> m_vhitables; // all Hitables obtained from the table's list of Editables

   int m_minphyslooptime; // minimum physics loop processing time in usec (0-1000), effort to reduce input latency (mainly useful if vsync is enabled, too)

private:
   vector<Ball *> m_vballDelete; // Balls to free at the end of the frame
#pragma endregion


#pragma region Timers
public:
   void FireTimers(const unsigned int simulationTime);
   void DeferTimerStateChange(HitTimer * const hittimer, bool enabled);
   void ApplyDeferredTimerChanges();

private:
   vector<HitTimer *> m_vht;
   vector<TimerOnOff> m_changed_vht; // stores all en/disable changes to the m_vht timer list, to avoid problems with timers dis/enabling themselves
#pragma endregion


#pragma region UI
public:
   void InitFPS();
   bool ShowFPSonly() const;
   bool ShowStats() const;
   InfoMode GetInfoMode() const;
   ProfilingMode GetProfilingMode() const;

   LiveUI *m_liveUI = nullptr;
   InfoMode m_infoMode = IF_NONE;
   unsigned int m_infoProbeIndex = 0;
#pragma endregion


#pragma region Rendering
public:
   Renderer *m_renderer = nullptr;
   VRDevice *m_vrDevice = nullptr;
   StereoMode m_stereo3D = STEREO_OFF;
   bool m_headTracking = false;
   #ifdef ENABLE_SDL_VIDEO
   SDL_Window  *m_sdl_playfieldHwnd = nullptr;
   #endif
   bool m_scaleFX_DMD = false;

private:
   void PrepareFrame();
   void SubmitFrame();
   bool FinishFrame();
#pragma endregion


#pragma region Input
public:
   PinInput m_pininput;
   EnumAssignKeys m_rgKeys[eCKeys]; // Player's key assignments
   bool m_supportsTouch = false; // Display is a touchscreen?
   bool m_touchregion_pressed[MAX_TOUCHREGION]; // status for each touch region to avoid multitouch double triggers (true = finger on, false = finger off)

private:
   int m_lastcursorx, m_lastcursory; // used for the dumb task of seeing if the mouse has really moved when we get a WM_MOUSEMOVE message
#pragma endregion


#pragma region Audio
public:
   void PauseMusic();
   void UnpauseMusic();

   bool m_PlayMusic;
   bool m_PlaySound;
   int m_MusicVolume;
   int m_SoundVolume;
   AudioPlayer *m_audio;
#pragma endregion

   vector<CLSID*> m_controlclsidsafe; // ActiveX control types which have already been okayed as being safe

   enum CloseState
   {
      CS_PLAYING = 0,   // Normal state
      CS_USER_INPUT = 1,// Open UI to let user choose between debugger, quit,...
      CS_STOP_PLAY = 2, // Stop play and get back to editor, if started without user input (minimized) then close the application
      CS_CLOSE_APP = 3, // Close the application and get back to operating system
      CS_FORCE_STOP = 4 // Force close the application and get back to operating system
   };
   CloseState m_closing = CS_PLAYING;

   bool m_userDebugPaused;
   bool m_debugWindowActive;
#ifndef __STANDALONE__
   DebuggerDialog m_debuggerDialog;
#endif
#ifdef ENABLE_SDL_INPUT
   float m_wnd_scale_x;
   float m_wnd_scale_y;
#endif
   bool m_debugMode;
   HWND m_hwndDebugOutput;
   bool m_showDebugger;

   bool m_throwBalls;
   bool m_ballControl;
   int  m_debugBallSize;
   float m_debugBallMass;
   bool m_debugBalls;                   // Draw balls in the foreground via 'O' key

   bool m_noTimeCorrect;                // Used so the frame after debugging does not do normal time correction

   // Used to detect script hangs (modal is used by script to tell VPX that it is in a modal state, so disabling watch dog)
   bool m_detectScriptHang;
   int m_LastKnownGoodCounter;
   int m_ModalRefCount;

   int m_wnd_width, m_wnd_height; // Window height (requested size before creation, effective size after) which is not directly linked to the render size

   int m_screenwidth, m_screenheight, m_refreshrate;
   bool m_fullScreen;

   bool m_drawCursor;
   bool m_gameWindowActive;

   Primitive *m_implicitPlayfieldMesh = nullptr;

   bool m_capExtDMD;
   int2 m_dmd;
   BaseTexture* m_texdmd;

   bool m_capPUP;
   BaseTexture *m_texPUP = nullptr;

   unsigned int m_overall_frames; // amount of rendered frames since start

private:
   int m_pauseRefCount;
   bool m_pseudoPause;   // Nothing is moving, but we're still redrawing

public:
   // all kinds of stats tracking, incl. FPS measurement
   int m_lastMaxChangeTime; // Used to update counters every seconds
   float m_fps;             // Average number of frames per second, updated once per second
   U32 m_script_max;

#ifdef __STANDALONE__
   VP::WindowManager* m_pWindowManager;
#endif

#ifdef STEPPING
public:
   U32 m_pauseTimeTarget;
   volatile bool m_pause;
   bool m_step;
#endif

#ifdef PLAYBACK
public:
   float ParseLog(LARGE_INTEGER *pli1, LARGE_INTEGER *pli2);

private:
   bool m_playback;
   FILE *m_fplaylog;
#endif
};
