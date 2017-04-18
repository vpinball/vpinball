#pragma once

#include "kdtree.h"
#include "quadtree.h"
#include <unordered_set>

#define DEFAULT_PLAYER_WIDTH 1024

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
   eEscape,
   eCKeys
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
	"EscapeKey"
};
static const int regkey_defdik[eCKeys] = {
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
	DIK_ESCAPE
};
static const int regkey_idc[eCKeys] = {
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
	-1
};

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
      int EnableLegacyMaximumPreRenderedFrames = 0;
      const HRESULT hr = GetRegInt("Player", "EnableLegacyMaximumPreRenderedFrames", &EnableLegacyMaximumPreRenderedFrames);
      if (hr != S_OK)
          EnableLegacyMaximumPreRenderedFrames = 0;

      // if available, use the official RenderDevice mechanism
      if (!EnableLegacyMaximumPreRenderedFrames && pd3dDevice->SetMaximumPreRenderedFrames(numFrames))
      {
          m_buffers.resize(0);
          return;
      }

      // if not, fallback to cheating the driver
      m_buffers.resize(numFrames, NULL);
      m_curIdx = 0;
   }

   void Shutdown()
   {
      for (size_t i = 0; i < m_buffers.size(); ++i)
      {
         if (m_buffers[i])
            m_buffers[i]->release();
      }
   }

   void Execute(RenderDevice * const pd3dDevice)
   {
      if (m_buffers.empty())
         return;

      if (m_buffers[m_curIdx])
         pd3dDevice->DrawPrimitiveVB(D3DPT_TRIANGLEFAN, MY_D3DFVF_NOTEX2_VERTEX, m_buffers[m_curIdx], 0, 3);

      m_curIdx = (m_curIdx + 1) % m_buffers.size();

      if (!m_buffers[m_curIdx])
         pd3dDevice->CreateVertexBuffer(1024, 0, MY_D3DFVF_NOTEX2_VERTEX, &m_buffers[m_curIdx]);

      // idea: locking a static vertex buffer stalls the pipeline if that VB is still
      // in the GPU render queue. In effect, this lets the GPU catch up.
      Vertex3D_NoTex2* buf;
      m_buffers[m_curIdx]->lock(0, 0, (void**)&buf, 0);
      memset(buf, 0, 3 * sizeof(buf[0]));
      buf[0].z = buf[1].z = buf[2].z = 1e5f;      // single triangle, degenerates to point far off screen
      m_buffers[m_curIdx]->unlock();
   }

private:
   std::vector<VertexBuffer*> m_buffers;
   size_t m_curIdx;
};

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

   // adjust an acceleration sample (m_NudgeX or m_NudgeY)
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
   bool enabled;
};

class Player
{
public:
   Player(bool _cameraMode);
   virtual ~Player();

   HRESULT Init(PinTable * const ptable, const HWND hwndProgress, const HWND hwndProgressName);
   void RenderStaticMirror(const bool onlyBalls);
   void RenderDynamicMirror(const bool onlyBalls);
   void RenderMirrorOverlay();
   void InitBallShader();
   void CreateDebugFont();
   void DebugPrint(int x, int y, LPCSTR text, int stringLen, bool shadow = false);
   void InitWindow();
   void InitKeys();
   void InitRegValues();

   void Shutdown();

   virtual IEditable *GetIEditable() { return (IEditable*)this; }

   void InitStatic(HWND hwndProgress);

   void UpdatePerFrame();

   void UpdatePhysics();
   void Render();
   void RenderDynamics();

   void DrawBalls();

   void SetClipPlanePlayfield(const bool clip_orientation);

   void DrawBulbLightBuffer();
   void Bloom();
   void StereoFXAA(const bool stereo, const bool SMAA, const bool DLAA, const bool NFAA, const bool FXAA1, const bool FXAA2, const bool FXAA3, const bool depth_available);

   void UpdateHUD();

   void FlipVideoBuffersNormal(const bool vsync);
   void FlipVideoBuffersAO(const bool vsync);

   void SetScreenOffset(float x, float y);     // set render offset in screen coordinates, e.g., for the nudge shake

   void PhysicsSimulateCycle(float dtime);

   Ball *CreateBall(const float x, const float y, const float z, const float vx, const float vy, const float vz, const float radius = 25.0f, const float mass = 1.0f);
   void DestroyBall(Ball *pball);

   void CreateBoundingHitShapes(Vector<HitObject> *pvho);

   void InitDebugHitStructure();
   void DoDebugObjectMenu(int x, int y);

   void PauseMusic();
   void UnpauseMusic();

   void RecomputePauseState();
   void RecomputePseudoPauseState();

   void NudgeUpdate();
   void FilterNudge();
   void NudgeX(const int x, const int j);
   void NudgeY(const int y, const int j);
#ifdef UNUSED_TILT
   int  NudgeGetTilt(); // returns non-zero when appropriate to set the tilt switch
#endif

   void mechPlungerUpdate();
   void mechPlungerIn(const int z);

   void SetGravity(float slopeDeg, float strength);

#ifdef PLAYBACK
   float ParseLog(LARGE_INTEGER *pli1, LARGE_INTEGER *pli2);
#endif

   void DMDdraw(const float DMDposx, const float DMDposy, const float DMDwidth, const float DMDheight, const COLORREF DMDcolor, const float intensity);
   void Spritedraw(const float posx, const float posy, const float width, const float height, const COLORREF color, Texture* const tex, const float intensity);
   void Spritedraw(const float posx, const float posy, const float width, const float height, const COLORREF color, D3DTexture* const tex, const float intensity);

   HWND m_hwnd;

   Shader      *ballShader;
   IndexBuffer *ballIndexBuffer;
   VertexBuffer *ballVertexBuffer;
   VertexBuffer *m_ballTrailVertexBuffer;
   bool m_antiStretchBall;

   bool cameraMode;
   int backdropSettingActive;
   PinTable *m_ptable;

   Pin3D m_pin3d;

   U32 m_time_msec;

   Ball *m_pactiveball;		// ball the script user can get with ActiveBall
   Ball *m_pactiveballDebug;	// ball the debugger will use as Activeball when firing events

   std::vector<Ball*> m_vball;
   std::vector<HitFlipper*> m_vFlippers;

   Vector<AnimObject> m_vanimate;

   Vector<HitTimer> m_vht;
   std::vector<TimerOnOff> m_changed_vht; // stores all en/disable changes to the m_vht timer list, to avoid problems with timers dis/enabling themselves

   Vertex3Ds m_gravity;

   PinInput m_pininput;

   float m_NudgeX;
   float m_NudgeY;

   NudgeFilterX m_NudgeFilterX;
   NudgeFilterY m_NudgeFilterY;

   // new nudging
   Vertex3Ds m_tableVel;
   Vertex3Ds m_tableDisplacement;
   Vertex3Ds m_tableVelOld;
   Vertex3Ds m_tableVelDelta;
   float m_nudgeSpring;
   float m_nudgeDamping;

   EnumAssignKeys m_rgKeys[eCKeys]; //Player's key assignments

   HWND m_hwndDebugOutput;
   HWND m_hwndDebugger;
   HWND m_hwndLightDebugger;
   HWND m_hwndMaterialDebugger;

   Vector<CLSID> m_controlclsidsafe; // ActiveX control types which have already been okayed as being safe

   int m_sleeptime;			// time to sleep during each frame - can helps side threads like vpinmame

   float m_globalEmissionScale;

   int m_fVSync; // targeted refresh rate in Hz, if larger refresh rate it will limit FPS by uSleep() //!! currently does not work adaptively as it would require IDirect3DDevice9Ex which is not supported on WinXP
   int m_fMaxPrerenderedFrames;
   int m_fFXAA;
   bool m_fAA;

   bool m_dynamicAO;
   bool m_disableAO;

   bool m_useNvidiaApi;
   bool m_disableDWM;

   bool m_fStereo3Denabled;
   int m_fStereo3D; // 0=off, 1=top/down, 2=interlaced/LG, 3=sidebyside

   bool m_fPlayMusic;
   bool m_fPlaySound;
   int m_MusicVolume;
   int m_SoundVolume;

   XAudPlayer *m_pxap;

   int m_lastcursorx, m_lastcursory; // used for the dumb task of seeing if the mouse has really moved when we get a WM_MOUSEMOVE message

   int m_LastKnownGoodCounter;
   int m_ModalRefCount;

   int m_fCloseType;			// if 0 exit player and close application if started minimized, if 1 close application always, 2 is brute force exit
   bool m_fCloseDown;			// Whether to shut down the player at the end of this frame
   bool m_fCloseDownDelay;
   bool m_fShowDebugger;

   bool m_fReflectionForBalls;
   bool m_fTrailForBalls;

   bool m_fThrowBalls;
   int  m_DebugBallSize;

   bool m_fDetectScriptHang;
   bool m_fNoTimeCorrect;		// Used so the frame after debugging does not do normal time correction

   bool m_fDebugMode;

   bool m_DebugBalls;			 // Draw balls in the foreground.
   bool m_ToggleDebugBalls;

   bool m_swap_ball_collision_handling; // Swaps the order of ball-ball collision handling around each physics cycle (in regard to the RLC comment block in quadtree.cpp (hopefully ;)))

   U32 m_script_period;
   U64 m_script_total;
   U32 m_script_max;
   U32 m_script_max_total;

#ifdef _DEBUGPHYSICS
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
   VertexBuffer * m_ballDebugPoints;
#endif
   U32 m_movedPlunger;			// has plunger moved, must have moved at least three times
   U32 m_LastPlungerHit;		// The last time the plunger was in contact (at least the vicinity) of the ball.
   float m_curMechPlungerPos;

   int m_width, m_height;

   int m_screenwidth, m_screenheight, m_refreshrate;
   bool m_fFullScreen;

   bool m_touchregion_pressed[8]; // status for each touch region to avoid multitouch double triggers (true = finger on, false = finger off)

   bool m_fDrawCursor;
   bool m_fGameWindowActive;
   bool m_fUserDebugPaused;
   bool m_fDebugWindowActive;
   bool m_fCabinetMode;
   bool m_fRecordContacts;             // flag for DoHitTest()
   std::vector< CollisionEvent > m_contacts;
   char m_ballShaderTechnique[MAX_PATH];

   int m_dmdx;
   int m_dmdy;
   BaseTexture* m_texdmd;

   unsigned int m_current_renderstage; // currently only used for bulb lights
   unsigned int m_dmdstate; // used to distinguish different flasher/DMD rendering mode states

   int m_overall_frames; // amount of rendered frames since start

private:
   Vector<HitObject> m_vho;
   std::vector< AnimObject* > m_vmover;    // moving objects for physics simulation

   std::vector<Ball*> m_vballDelete;	// Balls to free at the end of the frame

   /*HitKD*/HitQuadtree m_hitoctree;

   Vector<HitObject> m_vdebugho;
   HitQuadtree m_debugoctree;

   Vector<HitObject> m_vho_dynamic;
   HitKD m_hitoctree_dynamic; // should be generated from scratch each time something changes

   HitPlane m_hitPlayfield; // HitPlanes cannot be part of octree (infinite size)
   HitPlane m_hitTopGlass;

   U64 m_StartTime_usec;
   U64 m_curPhysicsFrameTime;	// Time when the last frame was drawn
   U64 m_nextPhysicsFrameTime;	// time at which the next physics update should be

   // all Hitables obtained from the table's list of Editables
   std::vector< Hitable* > m_vhitables;
   std::vector< Hitable* > m_vHitNonTrans; // non-transparent hitables
   std::vector< Hitable* > m_vHitTrans;    // transparent hitables

   int m_curAccel_x[PININ_JOYMXCNT];
   int m_curAccel_y[PININ_JOYMXCNT];

#ifdef PLAYBACK
   BOOL m_fPlayback;
   FILE *m_fplaylog;
#endif

   float m_BallStretchX;
   float m_BallStretchY;

   float m_NudgeShake;         // whether to shake the screen during digital nudges and how much
   Vertex2D m_ScreenOffset;    // for screen shake effect during nudge

   int m_curPlunger;

   //HANDLE m_hSongCompletionEvent;

   int m_pauseRefCount;

   bool m_fPseudoPause;		// Nothing is moving, but we're still redrawing

#ifdef LOG
   FILE *m_flog;
   int m_timestamp;
#endif

   U32 m_phys_iterations;

#ifdef FPS
   U32 m_lastfpstime;
   U32 m_cframes;
   float m_fps;
   float m_fpsAvg;
   U32 m_fpsCount;
   U64 m_lastTime_usec;
   U32 m_lastFrameDuration;
   U32 m_max;
   U32 m_max_total;
   U64 m_count;
   U64 m_total;
   int m_lastMaxChangeTime;
   U64 m_phys_total;
   U64 m_phys_total_iterations;
   U32 m_phys_max_iterations;
   U32 m_phys_period;
   U32 m_phys_max;
   U32 m_phys_max_total;

   FrameQueueLimiter m_limiter;

public:
   void ToggleFPS();
   void InitFPS();
   void UpdateBasicShaderMatrix( const Matrix3D& objectTrafo = Matrix3D( 1.0f ) );
   void InitShader();
   void UpdateCameraModeDisplay();
   void UpdateBackdropSettings(const bool up);
   void UpdateBallShaderMatrix();
   void CalcBallAspectRatio();
   void GetBallAspectRatio(const Ball * const pball, float &stretchX, float &stretchY, const float zHeight);
   //void DrawBallReflection(Ball *pball, const float zheight, const bool lowDetailBall);
#endif

#ifdef STEPPING
public:
   U32 m_PauseTimeTarget;
   bool m_fPause;
   bool m_fStep;
#endif

   bool m_fShowFPS;
   unsigned int m_staticOnly;

   bool m_scaleFX_DMD;

   bool m_toogle_DTFS;

   bool m_fStereo3DY;
   bool m_fOverwriteBallImages;
   Texture *m_ballImage;
   Texture *m_decalImage;

   ID3DXFont *m_pFont;
};
