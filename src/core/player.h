#pragma once

#include "physics/kdtree.h"
#include "physics/quadtree.h"
#include "Debugger.h"
#include "typedefs3D.h"
#include "pininput.h"
#include "LiveUI.h"

#define DEFAULT_PLAYER_WIDTH 1024
#define DEFAULT_PLAYER_FS_WIDTH 1920
#define DEFAULT_PLAYER_FS_REFRESHRATE 60

constexpr int DBG_SPRITE_SIZE = 1024;

enum VRPreviewMode
{
   VRPREVIEW_DISABLED,
   VRPREVIEW_LEFT,
   VRPREVIEW_RIGHT,
   VRPREVIEW_BOTH
};

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
   DIK_F11,
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

#define MAX_TOUCHREGION 8

static constexpr RECT touchregion[MAX_TOUCHREGION] = { //left,top,right,bottom (in % of screen)
   { 0, 0, 50, 10 },      // ExtraBall
   { 0, 10, 50, 50 },     // 2nd Left Button
   { 0, 50, 50, 90 },     // 1st Left Button (Flipper)
   { 0, 90, 50, 100 },    // Start
   { 50, 0, 100, 10 },    // Exit
   { 50, 10, 100, 50 },   // 2nd Right Button
   { 50, 50, 100, 90 },   // 1st Right Button (Flipper)
   { 50, 90, 100, 100 }   // Plunger
};

static constexpr EnumAssignKeys touchkeymap[MAX_TOUCHREGION] = {
   eAddCreditKey, //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
   eLeftMagnaSave,
   eLeftFlipperKey,
   eStartGameKey,
   eExitGame,
   eRightMagnaSave,
   eRightFlipperKey,
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

#ifndef ENABLE_SDL
// Note: Nowadays the original code seems to be counter-productive, so we use the official
// pre-rendered frame mechanism instead where possible
// (e.g. all windows versions except for XP and no "EnableLegacyMaximumPreRenderedFrames" set in the registry)
/*
 * Class to limit the length of the GPU command buffer queue to at most 'numFrames' frames.
 * Excessive buffering of GPU commands creates high latency and can create stuttery overlong
 * frames when the CPU stalls due to a full command buffer ring.
 *
 * Calling Execute() within a BeginScene() / EndScene() pair creates an artificial pipeline
 * stall by locking a vertex buffer which was rendered from (numFrames-1) frames ago. This
 * forces the CPU to wait and let the GPU catch up so that rendering doesn't lag more than
 * numFrames behind the CPU. It does *NOT* limit the framerate itself, only the drawahead.
 * Note that VP is currently usually GPU-bound.
 *
 * This is similar to Flush() in later DX versions, but doesn't flush the entire command
 * buffer, only up to a certain previous frame.
 *
 * Use of this class has been observed to effectively reduce stutter at least on an NVidia/
 * Win7 64 bit setup. The queue limiting effect can be clearly seen in GPUView.
 *
 * The initial cause for the stutter may be that our command buffers are too big (two
 * packets per frame on typical tables, instead of one), so with more optimizations to
 * draw calls/state changes, none of this may be needed anymore.
 */
class FrameQueueLimiter
{
public:
   void Init(RenderDevice * const pd3dDevice, const int numFrames)
   {
      const int EnableLegacyMaximumPreRenderedFrames = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "EnableLegacyMaximumPreRenderedFrames"s, 0);

      // if available, use the official RenderDevice mechanism
      if (!EnableLegacyMaximumPreRenderedFrames && pd3dDevice->SetMaximumPreRenderedFrames(numFrames))
      {
          m_buffers.resize(0);
          return;
      }

      // if not, fallback to cheating the driver
      m_buffers.resize(numFrames, nullptr);
      m_curIdx = 0;
   }

   void Shutdown()
   {
      for (size_t i = 0; i < m_buffers.size(); ++i)
         delete m_buffers[i];
   }

   void Execute(RenderDevice * const pd3dDevice)
   {
      if (m_buffers.empty())
         return;

      if (m_buffers[m_curIdx])
      {
         Vertex3Ds pos(0.f, 0.f, 0.f);
         pd3dDevice->DrawMesh(pd3dDevice->basicShader, false, pos, 0.f, m_buffers[m_curIdx], RenderDevice::TRIANGLESTRIP, 0, 3);
      }

      m_curIdx = (m_curIdx + 1) % m_buffers.size();

      if (!m_buffers[m_curIdx])
      {
         VertexBuffer *vb = new VertexBuffer(pd3dDevice, 1024);
         m_buffers[m_curIdx] = new MeshBuffer(L"FrameLimiter"s,  vb);
      }

      // idea: locking a static vertex buffer stalls the pipeline if that VB is still
      // in the GPU render queue. In effect, this lets the GPU catch up.
      Vertex3D_NoTex2* buf;
      m_buffers[m_curIdx]->m_vb->lock(0, 0, (void**)&buf, VertexBuffer::WRITEONLY);
      memset(buf, 0, 3 * sizeof(buf[0]));
      buf[0].z = buf[1].z = buf[2].z = 1e5f;      // single triangle, degenerates to point far off screen
      m_buffers[m_curIdx]->m_vb->unlock();
   }

   FrameQueueLimiter()
   {
      m_curIdx = 0;
   }

private:
   vector<MeshBuffer*> m_buffers;
   size_t m_curIdx;
};
#else
class FrameQueueLimiter
{
public:
   void Init(RenderDevice * const pd3dDevice, const int numFrames)
   {
      pd3dDevice->SetMaximumPreRenderedFrames(numFrames);
   }
   void Execute(RenderDevice *const pd3dDevice) { }
   void Shutdown() { }
};
#endif

////////////////////////////////////////////////////////////////////////////////

#ifdef DEBUG_NUDGE
# define IF_DEBUG_NUDGE(code) code
#else
# define IF_DEBUG_NUDGE(code)
#endif

class NudgeFilter
{
public:
   NudgeFilter();

   // adjust an acceleration sample (m_Nudge.x or m_Nudge.y)
   void sample(float &a, const U64 frameTime);

private:
   // debug output
   IF_DEBUG_NUDGE(void dbg(const char *fmt, ...);)
   IF_DEBUG_NUDGE(virtual const char *axis() const = 0;)

   // running total of samples
   float m_sum;

   // previous sample
   float m_prv;

   // timestamp of last zero crossing in the raw acceleration data
   U64 m_tzc;

   // timestamp of last correction inserted into the data
   U64 m_tCorr;

   // timestamp of last motion == start of rest
   U64 m_tMotion;
};

class NudgeFilterX : public NudgeFilter
{
   const char *axis() const { return "x"; }
};
class NudgeFilterY : public NudgeFilter
{
   const char *axis() const { return "y"; }
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
   virtual LRESULT WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

private:
   void InitKeys();

   const int m_playMode;

public:
   void LockForegroundWindow(const bool enable);

   string GetPerfInfo();

   Ball *CreateBall(const float x, const float y, const float z, const float vx, const float vy, const float vz, const float radius = 25.0f, const float mass = 1.0f);
   void DestroyBall(Ball *pball);

   void AddCabinetBoundingHitShapes();

   void InitDebugHitStructure();
   void DoDebugObjectMenu(const int x, const int y);

   void PauseMusic();
   void UnpauseMusic();

   void RecomputePauseState();
   void RecomputePseudoPauseState();

#pragma region Main Loop
   void OnIdle();

   VideoSyncMode m_videoSyncMode = VideoSyncMode::VSM_FRAME_PACING;
   int m_maxFramerate = 0; // targeted refresh rate in Hz, if larger refresh rate it will limit FPS by uSleep() //!! currently does not work adaptively as it would require IDirect3DDevice9Ex which is not supported on WinXP
   int m_mainLoopPhase = 0;
   U64 m_lastPresentFrameTick = 0;
   bool m_lastFrameSyncOnVBlank;
   bool m_lastFrameSyncOnFPS;
   bool m_curFrameSyncOnVBlank = false;
   bool m_curFrameSyncOnFPS = false;

#pragma endregion

#pragma region Physics
private:
   void UpdatePhysics();
   void PhysicsSimulateCycle(float dtime);
   void NudgeUpdate();
   void FilterNudge();
   #ifdef UNUSED_TILT
   int NudgeGetTilt(); // returns non-zero when appropriate to set the tilt switch
   #endif
   void MechPlungerUpdate();

public:
   void NudgeX(const int x, const int joyidx);
   void NudgeY(const int y, const int joyidx);
   void MechPlungerIn(const int z, const int joyidx);
   void MechPlungerSpeedIn(const int z, const int joyidx);
   int GetMechPlungerSpeed() const;
   void SetGravity(float slopeDeg, float strength);

   Vertex3Ds m_gravity;

   Vertex2D m_Nudge;

   NudgeFilterX m_NudgeFilterX;
   NudgeFilterY m_NudgeFilterY;

   // new nudging
   Vertex3Ds m_tableVel;
   Vertex3Ds m_tableDisplacement;
   Vertex3Ds m_tableVelOld;
   Vertex3Ds m_tableVelDelta;
   float m_nudgeSpring;
   float m_nudgeDamping;

   // External accelerometer velocity input.  This is for newer 
   // pinball I/O controllers that integrate the acceleration samples
   // in the device to compute the instantaneous cabinet velocity.
   // This works just like the "new nudging" scheme above, except
   // that the velocity reading comes from the external, physical 
   // pin cab, as measured on an accelerometer.
   Vertex3Ds m_accelVel;
   Vertex3Ds m_accelVelOld;
   bool m_accelInputIsVelocity;

   // legacy/VP9 style keyboard nudging
   bool m_legacyNudge;
   float m_legacyNudgeStrength;
   Vertex2D m_legacyNudgeBack;
   int m_legacyNudgeTime;

   bool m_swap_ball_collision_handling; // Swaps the order of ball-ball collision handling around each physics cycle (in regard to the RLC comment block in quadtree.cpp (hopefully ;)))

#ifdef DEBUGPHYSICS
   U32 c_hitcnts;
   U32 c_collisioncnt;
   U32 c_contactcnt;
#ifdef C_DYNAMIC
   U32 c_staticcnt;
#endif
   U32 c_embedcnts;
   U32 c_timesearch;

   U32 c_kDObjects;
   U32 c_kDNextlevels;
   U32 c_quadObjects;
   U32 c_quadNextlevels;

   U32 c_traversed;
   U32 c_tested;
   U32 c_deepTested;
#endif

   U32 m_movedPlunger; // has plunger moved, must have moved at least three times
   U32 m_LastPlungerHit; // The last time the plunger was in contact (at least the vicinity) of the ball.
   float m_curMechPlungerPos;  // position from joystick axis input, if a position axis is assigned
   int m_curMechPlungerSpeed;  // plunger speed from joystick axis input, if a speed axis is assigned
   float m_plungerSpeedScale;  // scaling factor for plunger speed input, to convert from joystick to internal units
   bool m_fExtPlungerSpeed;    // flag: plunger speed was received via joystick input

   // Physics stats
   U32 m_phys_iterations;
   U64 m_phys_total_iterations;
   U32 m_phys_max_iterations;
   U32 m_phys_max;
   U64 m_count; // Number of frames included in the total variant of the counters

   bool m_recordContacts; // flag for DoHitTest()
   vector<CollisionEvent> m_contacts;

#ifndef LOG
private:
#endif
   vector<MoverObject *> m_vmover; // moving objects for physics simulation
#ifdef LOG
private:
#endif
   vector<HitObject *> m_vho;

   vector<Ball *> m_vballDelete; // Balls to free at the end of the frame

   /*HitKD*/ HitQuadtree m_hitoctree;

   vector<HitObject *> m_vdebugho;
   HitQuadtree m_debugoctree;

   vector<HitObject *> m_vho_dynamic;
#ifdef USE_EMBREE
   HitQuadtree m_hitoctree_dynamic; // should be generated from scratch each time something changes
#else
   HitKD m_hitoctree_dynamic; // should be generated from scratch each time something changes
#endif

   float m_NudgeShake; // whether to shake the screen during nudges and how much

   HitPlane m_hitPlayfield; // HitPlanes cannot be part of octree (infinite size)
   HitPlane m_hitTopGlass;

   U64 m_StartTime_usec;
   U64 m_curPhysicsFrameTime; // Time when the last frame was drawn
   U64 m_nextPhysicsFrameTime; // Time at which the next physics update should be
   U64 m_lastFlipTime;

   // all Hitables obtained from the table's list of Editables
   vector<Hitable *> m_vhitables;

   int2 m_curAccel[PININ_JOYMXCNT];

   int m_curPlunger[PININ_JOYMXCNT];
   int m_curPlungerSpeed[PININ_JOYMXCNT];
#pragma endregion


#ifdef PLAYBACK
public:
   float ParseLog(LARGE_INTEGER *pli1, LARGE_INTEGER *pli2);

private:
   bool m_playback;
   FILE *m_fplaylog;
#endif


#pragma region Rendering
public:
   void SetupShaders();

private:
   void PrepareFrame();
   void SubmitFrame();
   bool FinishFrame();

   void RenderStaticPrepass();
   void DrawBulbLightBuffer();
   void RenderDynamics();
   void PrepareVideoBuffers();
   void Bloom();
   void SSRefl();

   FrameQueueLimiter m_limiter;

   void SetScreenOffset(const float x, const float y); // set render offset in screen coordinates, e.g., for the nudge shake

   Vertex2D m_ScreenOffset; // for screen shake effect during nudge

public:
   vector<Light*> m_ballReflectedLights;
   MeshBuffer *m_ballMeshBuffer = nullptr;
   MeshBuffer *m_ballTrailMeshBuffer = nullptr;
   #ifdef DEBUG_BALL_SPIN
   MeshBuffer *m_ballDebugPoints = nullptr;
   #endif
   int m_ballTrailMeshBufferPos = 0;
   bool m_trailForBalls;
   float m_ballTrailStrength;
   bool m_disableLightingForBalls;
   bool m_overwriteBallImages = false;
   Texture *m_ballImage = nullptr;
   Texture *m_decalImage = nullptr;

private:
   int m_maxPrerenderedFrames;

   U64 m_startFrameTick; // System time in us when render frame was started (beginning of frame animation then collect,...)

public:
   void DrawStatics();
   void DrawDynamics(bool onlyBalls);
   void Spritedraw(const float posx, const float posy, const float width, const float height, const COLORREF color, Texture* const tex, const float intensity, const bool backdrop=false);
   void Spritedraw(const float posx, const float posy, const float width, const float height, const COLORREF color, Sampler* const tex, const float intensity, const bool backdrop=false);

   void UpdateBasicShaderMatrix(const Matrix3D &objectTrafo = Matrix3D::MatrixIdentity());
   void UpdateBallShaderMatrix();

   #ifdef ENABLE_SDL
   SDL_Window  *m_sdl_playfieldHwnd;
   #endif

   void DisableStaticPrePass(const bool disable) { if (m_disableStaticPrepass != disable) { m_disableStaticPrepass = disable; m_isStaticPrepassDirty = true; } }
   bool IsUsingStaticPrepass() const { return !m_disableStaticPrepass && m_stereo3D != STEREO_VR && !m_headTracking; }

private:
   bool m_isStaticPrepassDirty = true;
   bool m_disableStaticPrepass = false;
   RenderTarget *m_staticPrepassRT = nullptr;

public:

   float m_globalEmissionScale;

   int m_FXAA;    // =FXAASettings
   int m_sharpen; // 0=off, 1=CAS, 2=bilateral CAS
   int m_MSAASamples;
   float m_AAfactor;

   bool m_dynamicAO;
   bool m_disableAO;
   int GetAOMode(); // 0=Off, 1=Static, 2=Dynamic

   RenderProbe::ReflectionMode m_maxReflectionMode;
   bool m_ss_refl;

   bool m_useNvidiaApi;
   bool m_disableDWM;

   StereoMode m_stereo3D;
   bool m_stereo3DfakeStereo;
   bool m_stereo3Denabled;
   float m_stereo3DDefocus = 0.f;
   void UpdateStereoShaderState();

   VRPreviewMode m_vrPreview;
   bool m_vrPreviewShrink = false;

   bool m_headTracking;

   int m_BWrendering; // 0=off, 1=Black&White from RedGreen, 2=B&W from Red only
   bool m_bloomOff;
   ToneMapper m_toneMapper = TM_TONY_MC_MAPFACE;

   Pin3D m_pin3d;

   bool m_scaleFX_DMD;
   enum RenderMask : unsigned int
   {
      DEFAULT = 0, // Render everything
      STATIC_ONLY = 1 << 0, // Disable non static part rendering (for static prerendering)
      DYNAMIC_ONLY = 1 << 1, // Disable static part rendering
      LIGHT_BUFFER = 1 << 2, // Transmitted light rendering
      REFLECTION_PASS = 1 << 3,
      DISABLE_LIGHTMAPS = 1 << 4
   };
   unsigned int m_render_mask = 0; // Active pass render bit mask
   inline bool IsRenderPass(const RenderMask pass_mask) const { return (m_render_mask & pass_mask) != 0; }

   Texture *m_tonemapLUT = nullptr;
#pragma endregion


#pragma region UI
public:
   InfoMode m_infoMode = IF_NONE;
   unsigned int m_infoProbeIndex = 0;

   void InitFPS();
   bool ShowFPSonly() const;
   bool ShowStats() const;
   InfoMode GetInfoMode() const;
   ProfilingMode GetProfilingMode() const;

   LiveUI *m_liveUI = nullptr;

#pragma endregion


   PinTable * const m_pEditorTable; // The untouched version of the table, as it is in the editor (The Player needs it to interact with the UI)
   PinTable * const m_ptable; // The played table, which can be modified by the script

   U32 m_time_msec;          // current physics time
   U32 m_last_frame_time_msec; // used for non-physics controlled animations to update once per-frame only, aligned with m_time_msec

   Ball *m_pactiveball;      // ball the script user can get with ActiveBall
   Ball *m_pactiveballDebug; // ball the debugger will use as Activeball when firing events
   Ball *m_pactiveballBC;    // ball that the ball control UI will use
   Vertex3Ds *m_pBCTarget;   // if non-null, the target location for the ball to roll towards

   vector<Ball*> m_vball;
   vector<HitFlipper*> m_vFlippers;

   vector<HitTimer*> m_vht;
   vector<TimerOnOff> m_changed_vht; // stores all en/disable changes to the m_vht timer list, to avoid problems with timers dis/enabling themselves

#pragma region Input
public:
   PinInput m_pininput;
   EnumAssignKeys m_rgKeys[eCKeys]; //Player's key assignments
   int m_lastcursorx, m_lastcursory; // used for the dumb task of seeing if the mouse has really moved when we get a WM_MOUSEMOVE message
#pragma endregion


#pragma region Audio
public:
   bool m_PlayMusic;
   bool m_PlaySound;
   int m_MusicVolume;
   int m_SoundVolume;
   AudioPlayer *m_audio;
#pragma endregion

   vector<CLSID*> m_controlclsidsafe; // ActiveX control types which have already been okayed as being safe

   int m_minphyslooptime;    // minimum physics loop processing time in usec (0-1000), effort to reduce input latency (mainly useful if vsync is enabled, too)

   int m_LastKnownGoodCounter;
   int m_ModalRefCount;

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
   CloseState m_closing = CS_PLAYING;

public:
   HWND m_hwndDebugOutput;
   bool m_showDebugger;

   bool m_showWindowedCaption;

   bool m_throwBalls;
   bool m_ballControl;
   int  m_debugBallSize;
   float m_debugBallMass;

   bool m_detectScriptHang;
   bool m_noTimeCorrect;                // Used so the frame after debugging does not do normal time correction

   bool m_debugMode;

   bool m_debugBalls;                   // Draw balls in the foreground via 'O' key

   int m_wnd_width, m_wnd_height; // Window height (requested size before creation, effective size after) which is not directly linked to the render size

   int m_screenwidth, m_screenheight, m_refreshrate;
   bool m_fullScreen;

   bool m_touchregion_pressed[MAX_TOUCHREGION]; // status for each touch region to avoid multitouch double triggers (true = finger on, false = finger off)

   bool m_drawCursor;
   bool m_gameWindowActive;
   bool m_userDebugPaused;
   bool m_debugWindowActive;
   Primitive *m_implicitPlayfieldMesh = nullptr;

   bool m_capExtDMD;
   int2 m_dmd;
   BaseTexture* m_texdmd;

   bool m_capPUP;
   BaseTexture *m_texPUP = nullptr;

   unsigned int m_overall_frames; // amount of rendered frames since start

private:
   //HANDLE m_hSongCompletionEvent;

   int m_pauseRefCount;

   bool m_pseudoPause;   // Nothing is moving, but we're still redrawing

public:
   bool m_supportsTouch; // Display is a touchscreen?
   bool m_showTouchMessage;

   // all kinds of stats tracking, incl. FPS measurement
   int m_lastMaxChangeTime; // Used to update counters every seconds
   float m_fps;             // Average number of frames per second, updated once per second
   U32 m_script_max;

private:
   // only called from ctor
   HRESULT Init();

public:
   void OnClose() override;

#ifdef STEPPING
   U32 m_pauseTimeTarget;
   volatile bool m_pause;
   bool m_step;
#endif

   DebuggerDialog m_debuggerDialog;
};
