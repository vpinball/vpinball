// license:GPLv3+

#pragma once

#include "renderer/typedefs3D.h"
#include "renderer/Renderer.h"
#include "renderer/Window.h"
#include "physics/PhysicsEngine.h"
#include "ui/Debugger.h"
#include "ui/live/LiveUI.h"
#include "input/InputManager.h"
#include "plugins/ControllerPlugin.h"
#include "plugins/VPXPlugin.h"
#include "plugins/ResURIResolver.h"
#include "audio/AudioPlayer.h"

class VRDevice;

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

class ProgressDialog final : public CDialog
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
   BOOL OnInitDialog() override
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
   Player(PinTable *const live_table, const int playMode);
   ~Player();

   void LockForegroundWindow(const bool enable);

   string GetPerfInfo();

   void SetPlayState(const bool isPlaying, const uint32_t delayBeforePauseMs = 0); // Allow to play/pause during UI interaction or to perform timed simulation steps (still needs the player window to be focused).
   bool IsPlaying(const bool applyWndFocus = true) const { return (g_pvp->m_captureAttract != 0) || (m_playing && (applyWndFocus ? m_playfieldWnd->IsFocused() : true) && !IsEditorMode()); }
   void OnFocusChanged(); // On focus lost, pause player and show mouse cursor

   uint32_t m_pauseTimeTarget = 0;
   bool m_step = false; // If set to true, the physics engine will do a single physic step and stop simulation (turning this flag to false)

   PinTable *const m_ptable; // The played table (which can eventually be a shallow copy of a table to allow being modified by the script without changing the original table)
   bool IsEditorMode() const { return m_ptable->m_liveBaseTable == nullptr; }

   ProgressDialog m_progressDialog;

   double m_time_sec; // current physics time
   uint32_t m_time_msec; // current physics time
   uint32_t m_last_frame_time_msec; // used for non-physics controlled animations to update once per-frame only, aligned with m_time_msec

   Ball *m_pactiveball = nullptr; // ball the script user can get with ActiveBall
   Ball *m_pactiveballDebug = nullptr; // ball the debugger will use as ActiveBall when firing events

   void FireSyncTimer(int timerValue);

private:
   bool m_playing = true;
   void ApplyPlayingState(const bool play);

#pragma region Main Loop
public:
   void GameLoop();

   void UpdateGameLogic();

   VideoSyncMode GetVideoSyncMode() const { return m_videoSyncMode; }
   void SetVideoSyncMode(VideoSyncMode mode) { m_videoSyncMode = mode; }
   float GetTargetRefreshRate() const { return m_maxFramerate; }
   void SetTargetRefreshRate(float v) { m_maxFramerate = v < 0.f ? m_playfieldWnd->GetRefreshRate() : v == 0.f ? 10000.f : v < 24.f ? 24.f : v; }
   bool m_curFrameSyncOnFPS = false;
   bool m_curFrameSyncOnVBlank = false;
   bool m_lastFrameSyncOnVBlank = false;
   bool m_lastFrameSyncOnFPS = false;
   
   FrameProfiler m_logicProfiler; // Frame timing profiler to be used when measuring timings from the game logic thread
   FrameProfiler* m_renderProfiler = nullptr; // Frame timing profiler to be used when measuring timings from the render thread (same as game logic profiler for single threaded mode)

private:
   VideoSyncMode m_videoSyncMode = VideoSyncMode::VSM_FRAME_PACING;
   float m_maxFramerate = 0.f; // targeted refresh rate in Hz, if larger refresh rate it will limit FPS by uSleep() //!! currently does not work adaptively as it would require IDirect3DDevice9Ex which is not supported on WinXP
   uint64_t m_startFrameTick;  // System time in us when render frame was started (beginning of frame animation then collect,...)
   unsigned int m_onPrepareFrameMsgId;

   void ProcessOSMessages();
   void MultithreadedGameLoop();
   void FramePacingGameLoop();
   void GPUQueueStuffingGameLoop();
#pragma endregion


#pragma region MechPlunger
public:
   uint32_t m_LastPlungerHit = 0; // the last time the plunger was in contact (at least the vicinity) of the ball
#pragma endregion


#pragma region Nudge
public:
   float m_NudgeShake; // whether to shake the screen during nudges and how much
#pragma endregion


#pragma region Physics
public:
   Ball *CreateBall(const float x, const float y, const float z, const float vx, const float vy, const float vz, const float radius, const float mass);
   void DestroyBall(Ball *pBall);

   PhysicsEngine* m_physics = nullptr;

   vector<Ball *> m_vball;
   vector<IEditable *> m_vhitables; // all Renderable parts obtained from the table's list of Editables

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
   VPX::RenderOutput& GetOutput(VPXWindowId window);
   VPX::Window* m_playfieldWnd;
   VPX::RenderOutput m_backglassOutput;
   VPX::RenderOutput m_scoreViewOutput;
   VPX::RenderOutput m_topperOutput;
   Renderer *m_renderer = nullptr;
   VRDevice *m_vrDevice = nullptr;
   bool m_headTracking = false;
   vector<AncillaryRendererDef> m_ancillaryWndRenderers[VPXWindowId::VPXWINDOW_Topper + 1];

   int GetCabinetAutoFitMode() const { return m_cabinetAutoFitMode; }
   void SetCabinetAutoFitMode(int mode);
   float GetCabinetAutoFitPos() const { return m_cabinetAutoFitPos; }
   void SetCabinetAutoFitPos(float pos);

private:
   void PrepareFrame();
   void SubmitFrame();
   void FinishFrame();

   static void OnAuxRendererChanged(const unsigned int msgId, void *userData, void *msgData);
   unsigned int m_getAuxRendererId = 0, m_onAuxRendererChgId = 0;

   int m_cabinetAutoFitMode = 0;
   float m_cabinetAutoFitPos = 0.05f;
#pragma endregion


#pragma region Input
public:
   InputManager m_pininput;
   void ShowMouseCursor(const bool show) { m_drawCursor = show; UpdateCursorState(); }

private:
   bool m_drawCursor = false;
   void UpdateCursorState() const;
#pragma endregion


#pragma region Audio
public:
   void PauseMusic();
   void UnpauseMusic();
   void UpdateVolume();

   bool m_PlayMusic;
   bool m_PlaySound;
   int m_MusicVolume; // -100..100
   int m_SoundVolume; // -100..100
   bool m_musicPlaying = false;

   std::unique_ptr<VPX::AudioPlayer> m_audioPlayer;

private:
   int m_pauseMusicRefCount = 0;

   // External audio sources with priority override chain
   static void OnAudioUpdated(const unsigned int msgId, void *userData, void *msgData);
   static void OnAudioSrcChanged(const unsigned int msgId, void *userData, void *msgData);
   void UpdateActiveAudioSource();
   unsigned int m_onAudioUpdatedMsgId;
   unsigned int m_onAudioSrcChangedMsgId;
   unsigned int m_getAudioSrcMsgId;
   uint64_t m_activeAudioSourceId = 0;
   ankerl::unordered_dense::map<uint64_t, VPX::AudioPlayer::AudioStreamID> m_audioStreams;
#pragma endregion

public:
   vector<CLSID*> m_controlclsidsafe; // ActiveX control types which have already been okayed as being safe

   enum CloseState
   {
      CS_PLAYING = 0,    // Normal state
      CS_USER_INPUT = 1, // Open UI to let user choose between debugger, quit,...
      CS_STOP_PLAY = 2,  // Stop play and get back to editor, if started without user input (minimized) then close the application
      CS_CLOSE_APP = 3,  // Close the application and get back to operating system
      CS_FORCE_STOP = 4, // Force close the application and get back to operating system
      CS_CLOSED = 5,     // Closing (or closed is called from another thread, but g_pplayer is null when closed)
#ifdef __LIBVPINBALL__
      CS_CLOSE_CAPTURE_SCREENSHOT = 6 // Close and capture screenshot for table image
#endif
   };
   void SetCloseState(CloseState state) { if (m_closing != CS_CLOSED) m_closing = state; }
   CloseState GetCloseState() const { return m_closing; }
private:
   volatile CloseState m_closing = CS_PLAYING;

public:
   bool m_debugWindowActive = false;
   bool m_debugMode = false;
   bool m_showDebugger = false;
   HWND m_hwndDebugOutput = nullptr;
#ifndef __STANDALONE__
   DebuggerDialog m_debuggerDialog;
#endif

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
   std::shared_ptr<BaseTexture> m_dmdFrame = nullptr;
   unsigned int m_dmdFrameId = 0;

   ResURIResolver m_resURIResolver;


public:
   bool m_capPUP = false;
   std::shared_ptr<BaseTexture> m_texPUP = nullptr;

   unsigned int m_overall_frames = 0; // amount of rendered frames since start

   // all kinds of stats tracking, incl. FPS measurement
   int m_lastMaxChangeTime; // Used to update counters every seconds
   float m_fps;             // Average number of frames per second, updated once per second
   uint32_t m_script_max;

#ifdef PLAYBACK
   float ParseLog(LARGE_INTEGER *pli1, LARGE_INTEGER *pli2);

private:
   bool m_playback = false;
   FILE *m_fplaylog = nullptr;
#endif
};
