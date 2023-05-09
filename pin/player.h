#pragma once

#include "kdtree.h"
#include "quadtree.h"
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
   eCKeys
};

static constexpr RECT touchregion[8] = { //left,top,right,bottom (in % of screen)
   { 0, 0, 50, 10 },      // ExtraBall
   { 0, 10, 50, 50 },     // 2nd Left Button
   { 0, 50, 50, 90 },     // 1st Left Button (Flipper)
   { 0, 90, 50, 100 },    // Start
   { 50, 0, 100, 10 },    // Exit
   { 50, 10, 100, 50 },   // 2nd Right Button
   { 50, 50, 100, 90 },   // 1st Right Button (Flipper)
   { 50, 90, 100, 100 }   // Plunger
};

static EnumAssignKeys touchkeymap[8] = {
   eAddCreditKey, //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
   eLeftMagnaSave,
   eLeftFlipperKey,
   eStartGameKey,
   eExitGame,
   eRightMagnaSave,
   eRightFlipperKey,
   ePlungerKey
};

static const char* regkey_string[eCKeys] = {
   "LFlipKey",
   "RFlipKey",
   "LTiltKey",
   "RTiltKey",
   "CTiltKey",
   "PlungerKey",
   "FrameCount",
   "DebugBalls",
   "Debugger",
   "AddCreditKey",
   "AddCreditKey2",
   "StartGameKey",
   "MechTilt",
   "RMagnaSave",
   "LMagnaSave",
   "ExitGameKey",
   "VolumeUp",
   "VolumeDown",
   "LockbarKey",
   "Enable3DKey",
   "TableRecenterKey",
   "TableUpKey",
   "TableDownKey",
   "EscapeKey"
};
static constexpr int regkey_defdik[eCKeys] = {
   DIK_LSHIFT,
   DIK_RSHIFT,
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
   DIK_ESCAPE
};
static constexpr int regkey_idc[eCKeys] = {
   IDC_LEFTFLIPPER,
   IDC_RIGHTFLIPPER,
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

   -1, //!! missing in key dialog!
   IDC_TABLEREC_TEXT,
   IDC_TABLEUP_TEXT,
   IDC_TABLEDOWN_TEXT,
   -1
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
      const int EnableLegacyMaximumPreRenderedFrames = LoadValueIntWithDefault(regKey[RegName::Player], "EnableLegacyMaximumPreRenderedFrames"s, 0);

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
   Player(const bool cameraMode, PinTable *const editor_table, PinTable *const live_table);
   virtual ~Player();

   void CreateWnd(HWND parent = 0);
   virtual void PreRegisterClass(WNDCLASS& wc) override;
   virtual void PreCreate(CREATESTRUCT& cs) override;
   virtual void OnInitialUpdate() override;
   virtual LRESULT WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;

private:
   void InitBallShader();
   void InitKeys();

   void InitStatic();

   void UpdatePhysics();

   void DrawBulbLightBuffer();
   void Bloom();
   void SSRefl();
   void StereoFXAA(RenderTarget* renderedRT, const bool stereo, const bool SMAA, const bool DLAA, const bool NFAA, const bool FXAA1, const bool FXAA2, const bool FXAA3, const unsigned int sharpen, const bool depth_available);

   void PrepareVideoBuffersNormal();
   void PrepareVideoBuffersAO();
   void FlipVideoBuffers(const bool vsync);

   void PhysicsSimulateCycle(float dtime);

   void DrawBalls();

public:
   void LockForegroundWindow(const bool enable);
   void Render();
   void RenderDynamics();

   string GetPerfInfo();

   void DrawStatics();
   void DrawDynamics(bool onlyBalls);

   Ball *CreateBall(const float x, const float y, const float z, const float vx, const float vy, const float vz, const float radius = 25.0f, const float mass = 1.0f);
   void DestroyBall(Ball *pball);

   void AddCabinetBoundingHitShapes();

   void InitDebugHitStructure();
   void DoDebugObjectMenu(const int x, const int y);

   void PauseMusic();
   void UnpauseMusic();

   void RecomputePauseState();
   void RecomputePseudoPauseState();

   void NudgeUpdate();
   void FilterNudge();
   void NudgeX(const int x, const int joyidx);
   void NudgeY(const int y, const int joyidx);
#ifdef UNUSED_TILT
   int  NudgeGetTilt(); // returns non-zero when appropriate to set the tilt switch
#endif

   void MechPlungerUpdate();
   void MechPlungerIn(const int z);

   void SetGravity(float slopeDeg, float strength);

#ifdef PLAYBACK
   float ParseLog(LARGE_INTEGER *pli1, LARGE_INTEGER *pli2);
#endif

   void DMDdraw(const float DMDposx, const float DMDposy, const float DMDwidth, const float DMDheight, const COLORREF DMDcolor, const float intensity);
   void Spritedraw(const float posx, const float posy, const float width, const float height, const COLORREF color, Texture* const tex, const float intensity, const bool backdrop=false);
   void Spritedraw(const float posx, const float posy, const float width, const float height, const COLORREF color, Sampler* const tex, const float intensity, const bool backdrop=false);

#ifdef ENABLE_SDL
   SDL_Window  *m_sdl_playfieldHwnd;
#endif
   Shader      *m_ballShader;

   MeshBuffer *m_ballMeshBuffer = nullptr;
   MeshBuffer *m_ballTrailMeshBuffer = nullptr;
   bool m_antiStretchBall;

   void EnableStaticPrePass(const bool use_prepass) { if (m_dynamicMode == use_prepass) { m_dynamicMode = !use_prepass; InitStatic(); } }

   bool m_dynamicMode;
   bool m_cameraMode;
   int m_backdropSettingActive;
   PinTable * const m_pEditorTable; // The untouched version of the table, as it is in the editor (The Player needs it to interact with the UI)
   PinTable * const m_ptable; // The played table, which can be modified by the script

   Pin3D m_pin3d;

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

   Vertex3Ds m_gravity;

   PinInput m_pininput;

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

   // legacy/VP9 style keyboard nudging
   bool m_legacyNudge;
   float m_legacyNudgeStrength;
   Vertex2D m_legacyNudgeBack;
   int m_legacyNudgeTime;

   EnumAssignKeys m_rgKeys[eCKeys]; //Player's key assignments

   HWND m_hwndDebugOutput;

   vector<CLSID*> m_controlclsidsafe; // ActiveX control types which have already been okayed as being safe

   int m_sleeptime;          // time to sleep during each frame - can help side threads like vpinmame, but most likely outdated
   int m_minphyslooptime;    // minimum physics loop processing time in usec (0-1000), effort to reduce input latency (mainly useful if vsync is enabled, too)

   float m_globalEmissionScale;

   int m_VSync; // targeted refresh rate in Hz, if larger refresh rate it will limit FPS by uSleep() //!! currently does not work adaptively as it would require IDirect3DDevice9Ex which is not supported on WinXP
   int m_maxPrerenderedFrames;
   int m_FXAA;    // =FXAASettings
   int m_sharpen; // 0=off, 1=CAS, 2=bilateral CAS
   int m_MSAASamples;
   float m_AAfactor;

   bool m_dynamicAO;
   bool m_disableAO;
   int GetAOMode(); // 0=Off, 1=Static, 2=Dynamic

   RenderProbe::ReflectionMode m_pfReflectionMode;
   bool m_ss_refl;

   bool m_useNvidiaApi;
   bool m_disableDWM;

   bool m_stereo3Denabled;
   bool m_stereo3DY;
   StereoMode m_stereo3D;
   VRPreviewMode m_vrPreview;

   bool m_headTracking;
   float m_global3DContrast;
   float m_global3DDesaturation;

   int m_BWrendering; // 0=off, 1=Black&White from RedGreen, 2=B&W from Red only

   bool m_bloomOff;
   bool m_ditherOff;

   bool m_PlayMusic;
   bool m_PlaySound;
   int m_MusicVolume;
   int m_SoundVolume;

   AudioPlayer *m_audio;

   int m_lastcursorx, m_lastcursory; // used for the dumb task of seeing if the mouse has really moved when we get a WM_MOUSEMOVE message

   int m_LastKnownGoodCounter;
   int m_ModalRefCount;

   enum CloseState
   {
      CS_PLAYING = 0, // Normal state
      CS_USER_INPUT = 1, // Open UI to let user choose between debugger, quit,...
      CS_STOP_PLAY = 2, // Stop play and get back to editor, if started without user input (minimized) then close the application
      CS_CLOSE_APP = 3, // Close the application and get back to operating system
      CS_FORCE_STOP = 4 // Force close the application and get back to operating system
   };
   CloseState m_closing = CS_PLAYING;
   bool m_showDebugger;

   bool m_showWindowedCaption;

   bool m_trailForBalls;
   bool m_disableLightingForBalls;

   bool m_throwBalls;
   bool m_ballControl;
   int  m_debugBallSize;
   float m_debugBallMass;

   bool m_detectScriptHang;
   bool m_noTimeCorrect;                // Used so the frame after debugging does not do normal time correction

   bool m_debugMode;

   bool m_debugBalls;                   // Draw balls in the foreground via 'O' key

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

#ifdef DEBUG_BALL_SPIN
   MeshBuffer *m_ballDebugPoints = nullptr;
#endif
   U32 m_movedPlunger;            // has plunger moved, must have moved at least three times
   U32 m_LastPlungerHit;          // The last time the plunger was in contact (at least the vicinity) of the ball.
   float m_curMechPlungerPos;

   int m_wnd_width, m_wnd_height; // Window height (requested size before creation, effective size after) which is not directly linked to the render size

   int m_screenwidth, m_screenheight, m_refreshrate;
   bool m_fullScreen;

   bool m_touchregion_pressed[8]; // status for each touch region to avoid multitouch double triggers (true = finger on, false = finger off)

   bool m_drawCursor;
   bool m_gameWindowActive;
   bool m_userDebugPaused;
   bool m_debugWindowActive;
   bool m_cabinetMode;
   Primitive *m_implicitPlayfieldMesh = nullptr;
   bool m_recordContacts;         // flag for DoHitTest()
   vector< CollisionEvent > m_contacts;

   int2 m_dmd;
   BaseTexture* m_texdmd;
   BaseTexture* m_texPUP = nullptr;

   unsigned int m_overall_frames; // amount of rendered frames since start

#ifndef LOG
private:
#endif
   vector<MoverObject*> m_vmover; // moving objects for physics simulation
#ifdef LOG
private:
#endif
   vector<HitObject*> m_vho;

   vector<Ball*> m_vballDelete;   // Balls to free at the end of the frame

   /*HitKD*/HitQuadtree m_hitoctree;

   vector<HitObject*> m_vdebugho;
   HitQuadtree m_debugoctree;

   vector<HitObject*> m_vho_dynamic;
#ifdef USE_EMBREE
   HitQuadtree m_hitoctree_dynamic; // should be generated from scratch each time something changes
#else
   HitKD m_hitoctree_dynamic;     // should be generated from scratch each time something changes
#endif

   HitPlane m_hitPlayfield;       // HitPlanes cannot be part of octree (infinite size)
   HitPlane m_hitTopGlass;

   U64 m_StartTime_usec;
   U64 m_curPhysicsFrameTime;     // Time when the last frame was drawn
   U64 m_nextPhysicsFrameTime;    // time at which the next physics update should be
   U64 m_lastFlipTime;

   // all Hitables obtained from the table's list of Editables
   vector< Hitable* > m_vhitables;
   vector< Hitable* > m_vHitNonTrans; // non-transparent hitables
   vector< Hitable* > m_vHitTrans;    // transparent hitables

   int2 m_curAccel[PININ_JOYMXCNT];

#ifdef PLAYBACK
   bool m_playback;
   FILE *m_fplaylog;
#endif

   Vertex2D m_BallStretch;

   float m_NudgeShake;         // whether to shake the screen during nudges and how much
   Vertex2D m_ScreenOffset;    // for screen shake effect during nudge

   int m_curPlunger;

   //HANDLE m_hSongCompletionEvent;

   int m_pauseRefCount;

   bool m_pseudoPause;      // Nothing is moving, but we're still redrawing

public:
   LiveUI *m_liveUI = nullptr;

   bool m_supportsTouch; // Display is a touchscreen?
   bool m_showTouchMessage;

   U32 m_phys_iterations;

   // all kinds of stats tracking, incl. FPS measurement
   U64 m_count; // Number of frames included in the total variant of the counters
   U64 m_total; // Total time corresponding to the m_count frames
   int m_lastMaxChangeTime; // Used to update counters every seconds
   U64 m_lastTime_usec; // Timestamped marked when updating frmae counters
   U32 m_lastFrameDuration; // Last frame duration
   U32 m_avgFrameDuration; // Summed duration of summed frame count (reset on user request)
   U32 m_avgFrameCount; // Number of frame duration summed up (reset on user request)
   U32 m_secFrameDuration; // Summed duration of summed frame count (reset once per second)
   U32 m_secFrameCount; // Number of frame duration summed up (reset once per second)
   float m_fps; // Average number of frames per second, updated once per second
   U64 m_frame_collect; // Time spent (us) to collect all render commands
   U64 m_frame_collect_total;
   U64 m_frame_submit; // Time spent (us) to submit all render commands
   U64 m_frame_submit_total;
   U64 m_frame_flip; // Time spent (us) to flip framebuffer
   U64 m_frame_flip_total;
   U32 m_max;
   U32 m_max_total;
   U32 m_phys_period; // Time spent in physic simulation
   U64 m_phys_total;
   U64 m_phys_total_iterations;
   U32 m_phys_max_iterations;
   U32 m_phys_max;
   U32 m_phys_max_total;
   U32 m_script_period; // Time spent in script timers. TODO for the time being the time spent in events is not taken in account, making this counter misleading
   U64 m_script_total;
   U32 m_script_max;
   U32 m_script_max_total;

private:
   FrameQueueLimiter m_limiter;

   // only called from ctor
   HRESULT Init();
   // only called from dtor
   void Shutdown();

   void SetScreenOffset(const float x, const float y);     // set render offset in screen coordinates, e.g., for the nudge shake

public:
   InfoMode m_infoMode = IF_NONE;
   unsigned int m_infoProbeIndex = 0;

   void InitFPS();
   bool ShowFPSonly() const;
   bool ShowStats() const;
   InfoMode GetInfoMode() const;
   ProfilingMode GetProfilingMode() const;

private:
   void InitShader();
   void CalcBallAspectRatio();
   void GetBallAspectRatio(const Ball * const pball, Vertex2D &stretch, const float zHeight);
   //void DrawBallReflection(Ball *pball, const float zheight, const bool lowDetailBall);

public:
   void StopPlayer();

   void UpdateBasicShaderMatrix(const Matrix3D& objectTrafo = Matrix3D::MatrixIdentity());
   void UpdateBackdropSettings(const bool up);
   void UpdateBallShaderMatrix();

#ifdef STEPPING
   U32 m_pauseTimeTarget;
   volatile bool m_pause;
   bool m_step;
#endif

   bool m_scaleFX_DMD;
   bool m_capExtDMD;
   bool m_capPUP;

   bool m_toogle_DTFS;

   enum RenderMask : unsigned int
   {
      DEFAULT = 0,
      STATIC_PREPASS = 1 << 0,
      LIGHT_BUFFER = 1 << 1,
      TRANSPARENT_DMD_PASS = 1 << 2,
      OPAQUE_DMD_PASS = 1 << 3,
      REFLECTION_PASS = 1 << 4
   };
   unsigned int m_render_mask = 0; // Active pass render bit mask
   inline bool IsRenderPass(const RenderMask pass_mask) const { return (m_render_mask & pass_mask) != 0; }

   bool m_overwriteBallImages;
   Texture *m_ballImage;
   Texture *m_decalImage;
   DebuggerDialog m_debuggerDialog;
};
