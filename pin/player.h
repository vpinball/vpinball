#pragma once

#include "kdtree.h"
#include "quadtree.h"
#include <unordered_set>

#define DEFAULT_PLAYER_WIDTH 1024

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
    void Init(int numFrames)
    {
        m_buffers.resize(numFrames, NULL);
        m_curIdx = 0;
    }

    void Shutdown()
    {
        for (unsigned i = 0; i < m_buffers.size(); ++i)
        {
            if (m_buffers[i])
                m_buffers[i]->release();
        }
    }

    void Execute(RenderDevice *pd3dDevice)
    {
        if (m_buffers.empty())
            return;

        if (m_buffers[m_curIdx])
            pd3dDevice->DrawPrimitiveVB(D3DPT_TRIANGLEFAN, m_buffers[m_curIdx], 0, 3);

        m_curIdx = (m_curIdx + 1) % m_buffers.size();

        if (!m_buffers[m_curIdx])
            pd3dDevice->CreateVertexBuffer(1024, 0, MY_D3DFVF_NOTEX2_VERTEX, &m_buffers[m_curIdx]);

        // idea: locking a static vertex buffer stalls the pipeline if that VB is still
        // in the GPU render queue. In effect, this lets the GPU catch up.
        Vertex3D_NoTex2* buf;
        m_buffers[m_curIdx]->lock(0, 0, (void**)&buf, 0);
        memset(buf, 0, 3*sizeof(buf[0]));
        buf[0].z = buf[1].z = buf[2].z = 1e5f;      // single triangle, degenerates to point far off screen
        m_buffers[m_curIdx]->unlock();
    }

private:
    std::vector<VertexBuffer*> m_buffers;
    unsigned m_curIdx;
};


////////////////////////////////////////////////////////////////////////////////
// begin mjr

//#define DEBUG_NUDGE
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
        void sample(float &a, U64 frameTime);

private:
    // debug output
        IF_DEBUG_NUDGE(void dbg(const char *fmt, ...);)
                IF_DEBUG_NUDGE(virtual const char *axis() const = 0;)

    // running total of samples
                float sum_;

    // previous sample
        float prv_;

    // timestamp of last zero crossing in the raw acceleration data
        U64 tzc_;

    // timestamp of last correction inserted into the data
        U64 tCorr_;

    // timestamp of last motion == start of rest
        U64 tMotion_;
};

class NudgeFilterX: public NudgeFilter
   { const char *axis() const { return "x"; } };
class NudgeFilterY: public NudgeFilter
   { const char *axis() const { return "y"; } };


// end mjr


////////////////////////////////////////////////////////////////////////////////


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

    void Shutdown();

        virtual IEditable *GetIEditable() { return (IEditable*)this; }

        void InitStatic(HWND hwndProgress);

    void UpdatePhysics();
        void Render();
    void RenderDynamics();

    void DrawBallShadow(Ball * const pball);
    void CalcBallShadow(Ball * const pball, Vertex3D_NoTex2 *vBuffer);
    void DrawBalls();
    void DrawBallLogo(Ball * const pball );
    void CalcBallLogo(Ball * const pball, Vertex3D_NoTex2 *vBuffer);
    void CheckAndUpdateRegions();
    void FlipVideoBuffersNormal( const bool vsync );
    void FlipVideoBuffers3DFXAA( const bool vsync );

        void PhysicsSimulateCycle(float dtime);
#ifdef ULTRAPIN
        void DrawLightHack();
#endif

        Ball *CreateBall(const float x, const float y, const float z, const float vx, const float vy, const float vz, const float radius = 25.0f);
        void DestroyBall(Ball *pball);

        void CreateBoundingHitShapes(Vector<HitObject> *pvho);

        void InitDebugHitStructure();
        void DoDebugObjectMenu(int x, int y);

        void PauseMusic();
        void UnpauseMusic();

        void RecomputePauseState();
        void RecomputePseudoPauseState();

        void UltraNudge_update();
        void UltraNudgeX( const int x, const int j );
        void UltraNudgeY( const int y, const int j );
        void FilterNudge();  // MJR
#if 0
        int  UltraNudgeGetTilt(); // returns non-zero when appropriate to set the tilt switch
#endif

        void UltraPlunger_update();
        void mechPlungerIn( const int z );

    void SetGravity(float slopeDeg, float strength);

#ifdef PLAYBACK
        float ParseLog(LARGE_INTEGER *pli1, LARGE_INTEGER *pli2);
#endif

        HWND m_hwnd;
#ifdef ULTRAPIN
        HWND m_dmdhackhwnd;
#endif

        PinTable *m_ptable;

        Pin3D m_pin3d;

        U32 m_time_msec;

        int m_DeadZ;

        Ball *m_pactiveball;            // ball the script user can get with ActiveBall
        Ball *m_pactiveballDebug;       // ball the debugger will use as Activeball when firing events

    std::vector<Ball*> m_vball;
    std::vector<HitFlipper*> m_vFlippers;

        Vector<AnimObject> m_vscreenupdate;
        Vector<HitTimer> m_vht;

    BOOL m_fThrowBalls;
        BOOL m_fAccelerometer;          //true if electronic Accelerometer enabled
        BOOL m_AccelNormalMount;        //true if normal mounting (left hand coordinates)
        float m_AccelAngle;                     // 0 Radians rotated counterclockwise (GUI is lefthand coordinates)
        float m_AccelAmp;                       // Accelerometer gain 
        float m_AccelAmpX;                      // Accelerometer gain X axis 
        float m_AccelAmpY;                      // Accelerometer gain Y axis
        float m_AccelMAmp;                      // manual input gain, generally from joysticks

        U32 m_jolt_amount;
        U32 m_tilt_amount;
        U32 m_jolt_trigger_time;
        U32 m_tilt_trigger_time;

        Vertex3Ds m_gravity;

        BOOL m_fDetectScriptHang;
        BOOL m_fNoTimeCorrect;          // Used so the frame after debugging does not do normal time correction
        PinInput m_pininput;

        float m_NudgeX;
        float m_NudgeY;
        int m_NudgeManual;                      //index of joystick that has manual control

        NudgeFilterX m_NudgeFilterX;  // MJR
        NudgeFilterY m_NudgeFilterY;  // MJR

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

        Vector<CLSID> m_controlclsidsafe; // ActiveX control types which have already been okayed as being safe

        BOOL m_fCloseDown;                      // Whether to shut down the player at the end of this frame
        int m_fCloseType;                       // if 0 exit player and close application if started minimized, if 1 close application always, 2 is brute force exit

        int m_sleeptime;                        // time to sleep during each frame - can helps side threads like vpinmame

        GPINFLOAT m_pixelaspectratio;

        int m_fVSync; // targeted refresh rate in Hz, if larger refresh rate it will limit FPS by uSleep() //!! currently does not work adaptively as it would require IDirect3DDevice9Ex which is not supported on WinXP
    int m_fMaxPrerenderedFrames;
        int m_fFXAA;
    BOOL m_fAA;

        BOOL m_fReflectionForBalls;
        BOOL m_fTrailForBalls;

        int m_fStereo3D; // 0=off, 1=top/down, 2=interlaced/LG //!! support sidebyside, too?
        BOOL m_fStereo3Denabled;

        XAudPlayer *m_pxap;

        BOOL m_fDebugMode;

        BOOL m_DebugBalls;                       // Draw balls in the foreground.
        BOOL m_ToggleDebugBalls;

        BOOL m_fPlayMusic;
        BOOL m_fPlaySound;
        int m_MusicVolume;
        int m_SoundVolume;

        int m_lastcursorx, m_lastcursory; // used for the dumb task of seeing if the mouse has really moved when we get a WM_MOUSEMOVE message

        int m_LastKnownGoodCounter;
        int m_ModalRefCount;

#ifdef _DEBUGPHYSICS
        U32 c_hitcnts;
        U32 c_collisioncnt;
        U32 c_contactcnt;
        U32 c_staticcnt;
        U32 c_embedcnts;
        U32 c_timesearch;

        U32 c_octNextlevels;
        U32 c_traversed;
        U32 c_tested;
        U32 c_deepTested;
#endif

#ifdef DEBUG_BALL_SPIN
    VertexBuffer * m_ballDebugPoints;
#endif
        int m_movedPlunger;                     // has plunger moved, must have moved at least three times
        U32 m_LastPlungerHit;           // The last time the plunger was in contact (at least the vicinity) of the ball.
        int m_Coins;                            // The number of coins queued to be inserted.  These were sent from the shell after the load.
        float m_curMechPlungerPos;

    int m_screenwidth, m_screenheight, m_screendepth, m_refreshrate;
    BOOL m_fFullScreen;

        int m_width, m_height;

        bool m_touchregion_pressed[8]; // status for each touch region to avoid multitouch double triggers (true = finger on, false = finger off)

    bool m_fDrawCursor;
        bool m_fGameWindowActive;
        bool m_fUserDebugPaused;
        bool m_fDebugWindowActive;
        
    std::vector< Hitable* > m_triggeredLights;  // lights whose state changed this frame (VP9COMPAT)

    bool m_fRecordContacts;             // flag for DoHitTest()
    std::vector< CollisionEvent > m_contacts;

private:
        Vector<HitObject> m_vho;
    std::vector< AnimObject* > m_vmover;    // moving objects for physics simulation

    std::vector<Ball*> m_vballDelete;   // Balls to free at the end of the frame

        HitQuadtree m_hitoctree;
        HitQuadtree m_shadowoctree;

        Vector<HitObject> m_vdebugho;
        HitQuadtree m_debugoctree;

        Vector<HitObject> m_vho_dynamic;
    HitKD m_hitoctree_dynamic; // should be generated from scratch each time something changes

    HitPlane m_hitPlayfield;
    HitPlane m_hitTopGlass;

        U64 m_StartTime_usec;
        U64 m_curPhysicsFrameTime;      // Time when the last frame was drawn
        U64 m_nextPhysicsFrameTime;     // time at which the next physics update should be

    // all Hitables obtained from the table's list of Editables
    std::vector< Hitable* > m_vhitables;
    std::vector< Hitable* > m_vHitNonTrans; // non-transparent hitables
    std::vector< Hitable* > m_vHitTrans;    // transparent hitables
    std::vector< Hitable* > m_vHitBackglass; // backglass objects (VP9COMPAT)
    std::vector< Hitable* > m_vLights;      // lights objects (VP9COMPAT)

    // VP9COMPAT: these are only used during the lights update routine,
    // but we keep them around to minimize dynamic allocations
    std::vector< Hitable* > m_sortedTriggeredLights;
    std::tr1::unordered_set< Hitable* > m_alreadyAddedLights;

        int m_curAccel_x[PININ_JOYMXCNT];
        int m_curAccel_y[PININ_JOYMXCNT];

#ifdef PLAYBACK
        BOOL m_fPlayback;
        FILE *m_fplaylog;
#endif

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

        int m_curPlunger;

        //HANDLE m_hSongCompletionEvent;

        int m_pauseRefCount;

        bool m_fPseudoPause;            // Nothing is moving, but we're still redrawing

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

#ifdef FPS
        int m_lastfpstime;
        int m_cframes;
        float m_fps;
    float m_fpsAvg;
    int m_fpsCount;
    U64 m_lastTime_usec;
    U64 m_lastFrameDuration;
        U64 m_count;
        U64 m_total;
        U64 m_max;
    int m_lastMaxChangeTime;
        U64 m_phys_total;
        U64 m_phys_max;
        U64 m_phys_total_iterations;
        U64 m_phys_max_iterations;
        BOOL m_fShowFPS;

    FrameQueueLimiter m_limiter;

public:
        void ToggleFPS();
#endif

#ifdef STEPPING
public:
        U32 m_PauseTimeTarget;
        bool m_fPause;
        bool m_fStep;
#endif

        bool m_firstFrame;
};

#ifdef ULTRAPIN
enum 
{
        // Firepower
    LIGHTHACK_FIREPOWER_P1                      = 0,
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
        DONGLE_STATUS_NOTFOUND                  = 0,
        DONGLE_STATUS_INCORRECT,

        // Run application.
        DONGLE_STATUS_OK
};

int get_dongle_status();
#endif
