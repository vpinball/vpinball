// license:GPLv3+

#pragma once

#include "renderer/typedefs3D.h"
#include "renderer/Renderer.h"
#include "renderer/Window.h"
#include "physics/PhysicsEngine.h"
#include "ui/Debugger.h"
#include "ui/LiveUI.h"
#include "pininput.h"
#include "plugins/CorePlugin.h"
#include "ResURIResolver.h"
#include "ScoreView.h"

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
// Startup progress dialog

class ProgressDialog : public CDialog
{
public:
   ProgressDialog() : CDialog(IDD_PROGRESS) { }

   void SetProgress(const string &text, const int value = -1) {
      #ifndef __STANDALONE__
      if (IsWindow()) {
         m_progressName.SetWindowText(text.c_str());
         if (value != -1)
            m_progressBar.SetPos(value);
      }
      #else
      if (value != -1 && m_progress != value) {
         PLOGI.printf("%s %d%%", text.c_str(), value);
         m_progress = value;
      }
      #endif
   }

protected:
   BOOL OnInitDialog() final
   {
      #ifndef __STANDALONE__
      AttachItem(IDC_PROGRESS2, m_progressBar);
      AttachItem(IDC_STATUSNAME, m_progressName);
      #endif
      return TRUE;
   }

private:
   #ifdef __STANDALONE__
   int m_progress = -1;
   #else
   CProgressBar m_progressBar;
   CStatic m_progressName;
   #endif
};

////////////////////////////////////////////////////////////////////////////////

struct TimerOnOff
{
   HitTimer* m_timer;
   bool m_enabled;
};


class Player final
{
public:
   Player(PinTable *const editor_table, PinTable *const live_table, const int playMode);
   ~Player();

   void LockForegroundWindow(const bool enable);

   string GetPerfInfo();

   void SetPlayState(const bool isPlaying, const U32 delayBeforePauseMs = 0); // Allow to play/pause during UI interaction or to perform timed simulation steps (still needs the player window to be focused).
   inline bool IsPlaying(const bool applyWndFocus = true) const { return applyWndFocus ? (m_playing && m_focused) : m_focused; }
   void OnFocusChanged(const bool isGameFocused); // On focus lost, pause player and show mouse cursor

   U32 m_pauseTimeTarget = 0;
   bool m_step = false; // If set to true, the physics engine will do a single physic step and stop simulation (turning this flag to false)

   PinTable *const m_pEditorTable; // The untouched version of the table, as it is in the editor (The Player needs it to interact with the UI)
   PinTable *const m_ptable; // The played table, which can be modified by the script

   ProgressDialog m_progressDialog;

   double m_time_sec; // current physics time
   U32 m_time_msec; // current physics time
   U32 m_last_frame_time_msec; // used for non-physics controlled animations to update once per-frame only, aligned with m_time_msec

   HitBall *m_pactiveball = nullptr; // ball the script user can get with ActiveBall
   HitBall *m_pactiveballDebug = nullptr; // ball the debugger will use as ActiveBall when firing events
   HitBall *m_pactiveballBC = nullptr; // ball that the ball control UI will use
   Vertex3Ds *m_pBCTarget = nullptr; // if non-null, the target location for the ball to roll towards

   void FireSyncController();

   // Temporary API used to communicate between VPinMame and VPinballX
   PinMame::core_tGlobalOutputState *m_pStateMappedMem = nullptr; // mapped shared memory used to share output states

#ifdef __LIBVPINBALL__
   bool m_liveUIOverride = false;
#endif

private:
   bool m_playing = true;
   bool m_focused = false;
   void ApplyPlayingState(const bool play);

#pragma region Main Loop
public:
   void GameLoop(std::function<void()> ProcessOSMessages);

   VideoSyncMode m_videoSyncMode = VideoSyncMode::VSM_FRAME_PACING;
   bool m_lastFrameSyncOnVBlank = false;
   bool m_lastFrameSyncOnFPS = false;

   float GetTargetRefreshRate() const { return m_maxFramerate < 10000.f ? m_maxFramerate : m_playfieldWnd->GetRefreshRate(); }
   bool m_curFrameSyncOnFPS = false;
   
   FrameProfiler m_logicProfiler; // Frame timing profiler to be used when measuring timings from the game logic thread
   FrameProfiler* m_renderProfiler = nullptr; // Frame timing profiler to be used when measuring timings from the render thread (same as game logic profiler for single threaded mode)

private:
   float m_maxFramerate = 0.f; // targeted refresh rate in Hz, if larger refresh rate it will limit FPS by uSleep() //!! currently does not work adaptively as it would require IDirect3DDevice9Ex which is not supported on WinXP
   bool m_curFrameSyncOnVBlank = false;
   U64 m_startFrameTick; // System time in us when render frame was started (beginning of frame animation then collect,...)
   unsigned int m_onGameStartMsgId;
   unsigned int m_onPrepareFrameMsgId;

   void MultithreadedGameLoop(const std::function<void()>& sync);
   void FramePacingGameLoop(const std::function<void()>& sync);
   void GPUQueueStuffingGameLoop(const std::function<void()>& sync);
#pragma endregion


#pragma region MechPlunger
public:
   void MechPlungerIn(const int z, const int joyidx);
   void MechPlungerSpeedIn(const int z, const int joyidx);
   int GetMechPlungerSpeed() const;

   U32 m_movedPlunger = 0;    // has plunger moved, must have moved at least three times
   U32 m_LastPlungerHit = 0;  // the last time the plunger was in contact (at least the vicinity) of the ball
   float m_curMechPlungerPos; // position from joystick axis input, if a position axis is assigned
   int m_curMechPlungerSpeed; // plunger speed from joystick axis input, if a speed axis is assigned
   float m_plungerSpeedScale; // scaling factor for plunger speed input, to convert from joystick to internal units
   bool m_fExtPlungerSpeed;   // flag: plunger speed was received via joystick input

   void MechPlungerUpdate();

private:
   int m_curPlunger[PININ_JOYMXCNT];      // mechanical plunger position input, one reading per joystick device
   int m_curPlungerSpeed[PININ_JOYMXCNT]; // mechanical plunger speed input, per joystick device
#pragma endregion


#pragma region Nudge
public:
   void ReadAccelerometerCalibration();
   void SetNudgeX(const int x, const int joyidx);
   void SetNudgeY(const int y, const int joyidx);
   const Vertex2D& GetRawAccelerometer() const;
   bool IsAccelInputAsVelocity() const { return m_accelInputIsVelocity; }
   
   #ifdef UNUSED_TILT
   int NudgeGetTilt(); // returns non-zero when appropriate to set the tilt switch
   #endif

   float m_NudgeShake; // whether to shake the screen during nudges and how much

private:
   int2 m_accelerometerMax; // Accelerometer max value X/Y axis (in -JOYRANGEMX..JOYRANGEMX range)
   int2 m_curAccel[PININ_JOYMXCNT]; // Live value acquired from joystick, clamped to max values (in -m_accelerometerMax..m_accelerometerMax)
   mutable bool m_accelerometerDirty = true;
   mutable Vertex2D m_accelerometer; // lazily evaluated sum of joystick mapped accelerometers, applying clamping then gain, normalized to -1..1 range
   bool m_accelerometerEnabled; // true if electronic accelerometer enabled
   bool m_accelerometerFaceUp; // true is Normal Mounting (Left Hand Coordinates)
   float m_accelerometerAngle; // 0 degrees rotated counterclockwise (GUI is lefthand coordinates)
   float m_accelerometerSensitivity;
   Vertex2D m_accelerometerGain; // Accelerometer gain X/Y axis
   bool m_accelInputIsVelocity;

#pragma endregion


#pragma region Physics
public:
   HitBall *CreateBall(const float x, const float y, const float z, const float vx, const float vy, const float vz, const float radius = 25.0f, const float mass = 1.0f);
   void DestroyBall(HitBall *pHitBall);

   PhysicsEngine* m_physics = nullptr;

   vector<HitBall *> m_vball;
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
   VPX::Window *m_playfieldWnd = nullptr;
   int m_lastDmdFrameId = -1;
   VPX::RenderOutput m_scoreviewOutput;
   VPX::RenderOutput m_backglassOutput;
   Renderer *m_renderer = nullptr;
   VRDevice *m_vrDevice = nullptr;
   bool m_headTracking = false;
   bool m_scaleFX_DMD = false;

private:
   void PrepareFrame(const std::function<void()>& sync);
   void SubmitFrame();
   void FinishFrame();
#pragma endregion


#pragma region Input
public:
   PinInput m_pininput;
   EnumAssignKeys m_rgKeys[eCKeys]; // Player's key assignments
   bool m_supportsTouch = false; // Display is a touchscreen?
   bool m_touchregion_pressed[MAX_TOUCHREGION]; // status for each touch region to avoid multitouch double triggers (true = finger on, false = finger off)
   void ShowMouseCursor(const bool show) { m_drawCursor = show; UpdateCursorState(); }

private:
   bool m_drawCursor = false;
   void UpdateCursorState() const;
#pragma endregion


#pragma region Audio
private:
   int m_pauseMusicRefCount = 0;

public:
   void PauseMusic();
   void UnpauseMusic();

   bool m_PlayMusic;
   bool m_PlaySound;
   int m_MusicVolume;
   int m_SoundVolume;
   AudioPlayer *m_audio = nullptr;
#pragma endregion

   vector<CLSID*> m_controlclsidsafe; // ActiveX control types which have already been okayed as being safe

   enum CloseState
   {
      CS_PLAYING = 0,    // Normal state
      CS_USER_INPUT = 1, // Open UI to let user choose between debugger, quit,...
      CS_STOP_PLAY = 2,  // Stop play and get back to editor, if started without user input (minimized) then close the application
      CS_CLOSE_APP = 3,  // Close the application and get back to operating system
      CS_FORCE_STOP = 4, // Force close the application and get back to operating system
      CS_CLOSED = 5      // Closing (or closed is called from another thread, but g_pplayer is null when closed)
   };
   void SetCloseState(CloseState state) { if (m_closing != CS_CLOSED) m_closing = state; }
   CloseState GetCloseState() const { return m_closing; }
private:
   volatile CloseState m_closing = CS_PLAYING;

public:
   bool m_debugWindowActive = false;
#ifndef __STANDALONE__
   DebuggerDialog m_debuggerDialog;
#endif
   bool m_debugMode = false;
   HWND m_hwndDebugOutput = nullptr;
   bool m_showDebugger = false;

   bool m_throwBalls = false;
   bool m_ballControl = false;
   int  m_debugBallSize;
   float m_debugBallMass;
   bool m_debugBalls = false;           // Draw balls in the foreground via 'O' key

   bool m_noTimeCorrect = false;        // Used so the frame after debugging does not do normal time correction

   // Used to detect script hangs (modal is used by script to tell VPX that it is in a modal state, so disabling watch dog)
   bool m_detectScriptHang;
   int m_LastKnownGoodCounter = 0;
   int m_ModalRefCount = 0;

   Primitive *m_implicitPlayfieldMesh = nullptr;

   // External DMD and displays, defined from script or captured
   bool m_capExtDMD = false; // frame capturing (hack for VR)
   int2 m_dmdSize = int2(0, 0); // DMD defined through VPX API DMDWidth/DMDHeight/DMDPixels/DMDColoredPixels
   BaseTexture* m_dmdFrame = nullptr;
   int m_dmdFrameId = 0;

   // DMDs and video displays gathered through plugin API
   struct ControllerDisplay
   {
      DmdSrcId dmdId;
      int frameId = -1;
      BaseTexture *frame = nullptr;
   };
   ControllerDisplay GetControllerDisplay(CtlResId id);

   // Segment displays gathered through plugin API
   struct ControllerSegDisplay
   {
      CtlResId segId;
      unsigned int nElements;
      float *frame = nullptr;
      vector<vector<SegElementType>> displays;
   };
   ControllerSegDisplay GetControllerSegDisplay(CtlResId id);

   ResURIResolver m_resURIResolver;

private:
   static void OnDmdChanged(const unsigned int msgId, void* userData, void* msgData);
   unsigned int m_getDmdMsgId, m_getDmdSrcMsgId, m_onDmdChangedMsgId;
   vector<ControllerDisplay> m_controllerDisplays;
   bool m_defaultDmdSelected = false;
   DmdSrcId m_defaultDmdId;

   static void OnSegChanged(const unsigned int msgId, void *userData, void *msgData);
   unsigned int m_getSegMsgId, m_getSegSrcMsgId, m_onSegChangedMsgId;
   vector<ControllerSegDisplay> m_controllerSegDisplays;
   bool m_defaultSegSelected = false;
   CtlResId m_defaultSegId; 
   
   ScoreView m_scoreView;


   // External audio sources
private:
   static void OnAudioUpdated(const unsigned int msgId, void *userData, void *msgData);
   unsigned int m_onAudioUpdatedMsgId;
   robin_hood::unordered_flat_map<uint64_t, AudioPlayer*> m_externalAudioPlayers;


public:
   bool m_capPUP = false;
   BaseTexture *m_texPUP = nullptr;

   unsigned int m_overall_frames = 0; // amount of rendered frames since start

   // all kinds of stats tracking, incl. FPS measurement
   int m_lastMaxChangeTime; // Used to update counters every seconds
   float m_fps;             // Average number of frames per second, updated once per second
   U32 m_script_max;

#ifdef PLAYBACK
   float ParseLog(LARGE_INTEGER *pli1, LARGE_INTEGER *pli2);

private:
   bool m_playback = false;
   FILE *m_fplaylog = nullptr;
#endif
};
