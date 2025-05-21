// license:GPLv3+

#include "core/stdafx.h"

#ifdef ENABLE_SDL_VIDEO
  #include <SDL3/SDL_main.h>
  #include "imgui/imgui_impl_sdl3.h"
#else
  #include "imgui/imgui_impl_win32.h"
#endif

#ifndef __STANDALONE__
#include "BAM/BAMView.h"
#endif

#ifdef __STANDALONE__
#include "standalone/Standalone.h"
#include "unordered_dense.h"
#endif

#ifdef __LIBVPINBALL__
#include "standalone/VPinballLib.h"
#endif

#include <iomanip>
#include <ctime>
#include <fstream>
#include <sstream>
#include <array>
#include <filesystem>
#include "renderer/Shader.h"
#include "renderer/Anaglyph.h"
#include "renderer/VRDevice.h"
#include "renderer/typedefs3D.h"
#ifndef __STANDALONE__
#include "renderer/captureExt.h"
#endif
#ifdef _MSC_VER
#if !defined(ENABLE_SDL_VIDEO)
#include "winsdk/legacy_touch.h"
#endif
// Used to log which program steals the focus from VPX
#include "psapi.h"
#pragma comment(lib, "Psapi")
#endif
#include "tinyxml2/tinyxml2.h"

#include "plugins/MsgPlugin.h"
#include "plugins/VPXPlugin.h"
#include "core/VPXPluginAPIImpl.h"

// MSVC Concurrency Viewer support
// This requires to add the MSVC Concurrency SDK to the project
//#define MSVC_CONCURRENCY_VIEWER
#ifdef MSVC_CONCURRENCY_VIEWER
#include <cvmarkersobj.h>
using namespace Concurrency::diagnostic;
extern marker_series series;
#endif

//

#define RECOMPUTEBUTTONCHECK (WM_USER+100)

#if (defined(_M_IX86) || defined(_M_X64) || defined(_M_AMD64) || defined(__i386__) || defined(__i386) || defined(__i486__) || defined(__i486) || defined(i386) || defined(__ia64__) || defined(__x86_64__))
#ifdef _MSC_VER
 #define init_cpu_detection int regs[4]; __cpuid(regs, 1);
 #define detect_no_sse (regs[3] & 0x002000000) == 0
 #define detect_sse2 (regs[3] & 0x004000000) != 0
#else
 #define init_cpu_detection __builtin_cpu_init();
 #define detect_no_sse !__builtin_cpu_supports("sse")
 #define detect_sse2 __builtin_cpu_supports("sse2")
#endif
#endif

// leave as-is as e.g. VPM relies on this
#define WIN32_PLAYER_WND_CLASSNAME _T("VPPlayer")
#define WIN32_WND_TITLE _T("Visual Pinball Player")

#if !defined(ENABLE_SDL_VIDEO) // Win32 Windowing
LRESULT CALLBACK PlayerWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   if (g_pplayer == nullptr || g_pplayer->m_playfieldWnd == nullptr || g_pplayer->m_playfieldWnd->GetCore() != hwnd)
      return DefWindowProc(hwnd, uMsg, wParam, lParam);

   if (ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam))
      return true;

   switch (uMsg)
   {
   case WM_CLOSE:
      g_pvp->QuitPlayer(Player::CloseState::CS_STOP_PLAY);
      return 0;

   case WM_ACTIVATE: // Toggle pause state based on window focus
      g_pplayer->OnFocusChanged(wParam != WA_INACTIVE);
      break;

   case WM_KEYDOWN: // Hide cursor when playing
      g_pplayer->ShowMouseCursor(false);
      break;

   case WM_MOUSEMOVE: // Show cursor if paused or if user move the mouse
      {
         static int m_lastcursorx = 0xfffffff, m_lastcursory = 0xfffffff; // used to detect user moving the mouse, therefore requesting the cursor to be shown
         if (m_lastcursorx != LOWORD(lParam) || m_lastcursory != HIWORD(lParam))
         {
            m_lastcursorx = LOWORD(lParam);
            m_lastcursory = HIWORD(lParam);
            g_pplayer->ShowMouseCursor(true);
         }
      }
      break;

   // FIXME the following events are not handled by the SDL implementation
   case WM_POINTERDOWN: // not implemented for SDL (SDL2 does not support touch devices under windows)
   case WM_POINTERUP:
   {
#ifndef TEST_TOUCH_WITH_MOUSE
#if (WINVER < 0x0602)
      if (!GetPointerInfo)
         GetPointerInfo = (pGPI)GetProcAddress(GetModuleHandle(TEXT("user32.dll")), "GetPointerInfo");
      if (GetPointerInfo)
#endif
#endif
      {
         POINTER_INFO pointerInfo;
#ifdef TEST_TOUCH_WITH_MOUSE
         GetCursorPos(&pointerInfo.ptPixelLocation);
#else
         if (GetPointerInfo(GET_POINTERID_WPARAM(wParam), &pointerInfo))
#endif
         {
            ScreenToClient(hwnd, &pointerInfo.ptPixelLocation);
            for (unsigned int i = 0; i < MAX_TOUCHREGION; ++i)
               if ((g_pplayer->m_touchregion_pressed[i] != (uMsg == WM_POINTERDOWN))
                  && Intersect(touchregion[i], g_pplayer->m_playfieldWnd->GetWidth(), g_pplayer->m_playfieldWnd->GetHeight(), pointerInfo.ptPixelLocation,
                     g_pplayer->m_ptable->mViewSetups[g_pplayer->m_ptable->m_BG_current_set].GetRotation(g_pplayer->m_playfieldWnd->GetWidth(), g_pplayer->m_playfieldWnd->GetHeight()) != 0.f))
               {
                  g_pplayer->m_touchregion_pressed[i] = (uMsg == WM_POINTERDOWN);
                  g_pplayer->m_pininput.PushActionEvent(touchActionMap[i], g_pplayer->m_touchregion_pressed[i]);
               }
         }
      }
      break;
   }
   }
   return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
#endif



Player::Player(PinTable *const editor_table, PinTable *const live_table, const int playMode)
   : m_pEditorTable(editor_table)
   , m_ptable(live_table)
   , m_scoreviewOutput("Visual Pinball - Score"s, live_table->m_settings, Settings::ScoreView, "ScoreView"s)
   , m_backglassOutput("Visual Pinball - Backglass"s, live_table->m_settings, Settings::Backglass, "Backglass"s)
   , m_topperOutput("Visual Pinball - Topper"s, live_table->m_settings, Settings::Topper, "Topper"s)
   , m_resURIResolver(MsgPluginManager::GetInstance().GetMsgAPI(), VPXPluginAPIImpl::GetInstance().GetVPXEndPointId(), true, true, true, true)
{
   // For the time being, lots of access are made through the global singleton, so ensure we are unique, and define it as soon as needed
   assert(g_pplayer == nullptr);
   g_pplayer = this;

   m_logicProfiler.NewFrame(0);
   m_renderProfiler = new FrameProfiler();
   m_renderProfiler->NewFrame(0);
   g_frameProfiler = &m_logicProfiler;

   m_progressDialog.Create(g_pvp->GetHwnd());
   m_progressDialog.ShowWindow(g_pvp->m_open_minimized ? SW_HIDE : SW_SHOWNORMAL);
   m_progressDialog.SetProgress("Creating Player..."s, 1);

#if !(defined(_M_IX86) || defined(_M_X64) || defined(_M_AMD64) || defined(__i386__) || defined(__i386) || defined(__i486__) || defined(__i486) || defined(i386) || defined(__ia64__) || defined(__x86_64__))
   constexpr int denormalBitMask = 1 << 24;
   int status_word;
#if defined(__aarch64__)
   asm volatile("mrs %x[status_word], FPCR" : [status_word] "=r"(status_word));
   status_word |= denormalBitMask;
   asm volatile("msr FPCR, %x[src]" : : [src] "r"(status_word));
#elif defined(__arm__)
   asm volatile("vmrs %[status_word], FPSCR" : [status_word] "=r"(status_word));
   status_word |= denormalBitMask;
   asm volatile("vmsr FPSCR, %[src]" : : [src] "r"(status_word));
#else
   #pragma message ( "Warning: No CPU float ignore denorm implemented" )
#endif
#else
   {
      init_cpu_detection
      // check for SSE and exit if not available, as some code relies on it by now
      if (detect_no_sse) { // No SSE?
         ShowError("SSE is not supported on this processor");
         exit(0);
      }
      // disable denormalized floating point numbers, can be faster on some CPUs (and VP doesn't need to rely on denormals)
      if (detect_sse2) // SSE2?
         _mm_setcsr(_mm_getcsr() | 0x8040); // flush denorms to zero and also treat incoming denorms as zeros
      else
         _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON); // only flush denorms to zero
   }
#endif

   m_plungerSpeedScale = 1.0f;
   m_curMechPlungerPos = 0;

   m_plungerSpeedScale = m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "PlungerSpeedScale"s, 100.0f) / 100.0f;
   if (m_plungerSpeedScale <= 0.0f)
      m_plungerSpeedScale = 1.0f;

   // Accelerometer inputs are accelerations (not velocities) by default
   m_accelInputIsVelocity = m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "AccelVelocityInput"s, false);

   bool useVR = false;
   #if defined(ENABLE_VR) || defined(ENABLE_XR)
      const int vrDetectionMode = m_ptable->m_settings.LoadValueWithDefault(Settings::PlayerVR, "AskToTurnOn"s, 1);
      #if defined(ENABLE_XR)
         if (vrDetectionMode != 2) // 2 is VR off (0 is VR on, 1 is autodetect)
         {
            m_vrDevice = new VRDevice();
            if (m_vrDevice->IsOpenXRReady())
            {
               m_vrDevice->SetupHMD();
               if (m_vrDevice->IsOpenXRHMDReady())
                  useVR = true;
               else if (vrDetectionMode == 0) // 0 is VR on
               {
                  while (!m_vrDevice->IsOpenXRHMDReady() && (g_pvp->MessageBox("Retry connection ?", "Connection to VR headset failed", MB_YESNO) == IDYES))
                     m_vrDevice->SetupHMD();
                  useVR = m_vrDevice->IsOpenXRHMDReady();
               }
            }
            else if (vrDetectionMode == 0) // 0 is VR on, tell the user that the choice will not be fullfilled
               ShowError("VR mode activated but OpenXR initialization failed.");
            if (!useVR)
            {
               delete m_vrDevice;
               m_vrDevice = nullptr;
            }
         }
      #elif defined(ENABLE_VR)
         useVR = vrDetectionMode == 2 /* VR Disabled */  ? false : VRDevice::IsVRinstalled();
         if (useVR && (vrDetectionMode == 1 /* VR Autodetect => ask to turn on and adapt accordingly */) && !VRDevice::IsVRturnedOn())
            useVR = g_pvp->MessageBox("VR headset detected but SteamVR is not running.\n\nTurn VR on?", "VR Headset Detected", MB_YESNO) == IDYES;
         m_vrDevice = useVR ? new VRDevice() : nullptr;
      #endif
   #endif
   const StereoMode stereo3D = useVR ? STEREO_VR : (StereoMode)m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "Stereo3D"s, (int)STEREO_OFF);
   assert(useVR == (stereo3D == STEREO_VR));

   m_capExtDMD = (stereo3D == STEREO_VR) && m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "CaptureExternalDMD"s, false);
   m_capPUP = (stereo3D == STEREO_VR) && m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "CapturePUP"s, false);
   m_headTracking = (stereo3D == STEREO_VR) ? false : m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "BAMHeadTracking"s, false);
   m_detectScriptHang = m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "DetectHang"s, false);

   m_NudgeShake = m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "NudgeStrength"s, 2e-2f);

   //!! TODO for now parse all dmd settings and assign scaleFX setting to internal dmd rendering
   m_scaleFX_DMD = m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "ScaleFXDMD"s, false);
   for (unsigned int n = 0; n < 10; ++n)
   {
      const string prefix = "User." + std::to_string(n + 1) + '.';
      m_scaleFX_DMD |= m_ptable->m_settings.LoadValueWithDefault(Settings::DMD, prefix + "ScaleFX", false);
   }

   m_minphyslooptime = min(m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "MinPhysLoopTime"s, 0), 1000);

   m_throwBalls = m_ptable->m_settings.LoadValueWithDefault(Settings::Editor, "ThrowBallsAlwaysOn"s, false);
   m_ballControl = m_ptable->m_settings.LoadValueWithDefault(Settings::Editor, "BallControlAlwaysOn"s, false);
   m_debugBallSize = m_ptable->m_settings.LoadValueWithDefault(Settings::Editor, "ThrowBallSize"s, 50);
   m_debugBallMass = m_ptable->m_settings.LoadValueWithDefault(Settings::Editor, "ThrowBallMass"s, 1.0f);

   for (unsigned int i = 0; i < MAX_TOUCHREGION; ++i)
      m_touchregion_pressed[i] = false;

#ifdef __LIBVPINBALL__
   m_liveUIOverride = g_pvp->m_settings.LoadValueWithDefault(Settings::Standalone, "LiveUIOverride"s, true);
#endif

   PLOGI << "Creating main window"; // For profiling
   {
      #if defined(_MSC_VER) && !defined(__STANDALONE__)
         WNDCLASS wc = {};
         wc.hInstance = g_pvp->theInstance;
         #ifndef ENABLE_SDL_VIDEO
         wc.lpfnWndProc = PlayerWindowProc;
         #elif defined(UNICODE)
         wc.lpfnWndProc = ::DefWindowProcW;
         #else
         wc.lpfnWndProc = ::DefWindowProcA;
         #endif
         wc.lpszClassName = WIN32_PLAYER_WND_CLASSNAME;
         wc.hIcon = LoadIcon(g_pvp->theInstance, MAKEINTRESOURCE(IDI_TABLE));
         wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
         ::RegisterClass(&wc);
         #ifdef ENABLE_SDL_VIDEO
         SDL_RegisterApp(WIN32_PLAYER_WND_CLASSNAME, 0, g_pvp->theInstance);
         #endif
      #endif
      
      const Settings* settings = &(g_pvp->m_settings); // Always use main application settings (not overridable per table)
      m_playfieldWnd = new VPX::Window(WIN32_WND_TITLE, settings, stereo3D == STEREO_VR ? Settings::PlayerVR : Settings::Player, stereo3D == STEREO_VR ? "Preview" : "Playfield");

      float pfRefreshRate = m_playfieldWnd->GetRefreshRate(); 
      m_maxFramerate = m_ptable->m_settings.LoadValueFloat(Settings::Player, "MaxFramerate"s);
      if(m_maxFramerate > 0.f && m_maxFramerate < 24.f) // at least 24 fps
         m_maxFramerate = 24.f;
      if (m_maxFramerate < 0.f) // Negative is display refresh rate
         m_maxFramerate = pfRefreshRate;
      if (m_maxFramerate == 0.f) // 0 is unbound refresh rate
         m_maxFramerate = 10000.f;
      m_videoSyncMode = static_cast<VideoSyncMode>(m_ptable->m_settings.LoadValueUInt(Settings::Player, "SyncMode"s));
      if (m_videoSyncMode != VideoSyncMode::VSM_NONE)
      {
         if (m_maxFramerate > pfRefreshRate)
            // User requested a max framerate above display rate but using VSync => limit to display refresh rate
            m_maxFramerate = pfRefreshRate;
         else if (m_maxFramerate < pfRefreshRate)
         {
            // User requested a max framerate below display rate but using VSync => limit to an integral division of the display refresh rate (keeping the FPS above 24FPS)
            float divider = 1.f;
            while ((m_maxFramerate * divider > pfRefreshRate) && (24.f * divider <= pfRefreshRate))
               divider += 1.f;
            m_maxFramerate = pfRefreshRate / divider;
         }
      }
      if (stereo3D == STEREO_VR)
      {
         // Disable VSync for VR (sync is performed by the OpenVR runtime)
         m_videoSyncMode = VideoSyncMode::VSM_NONE;
         m_maxFramerate = 10000.f;
      }
      assert(24.f <= m_maxFramerate && m_maxFramerate <= 10000.f); // We guarantee a target framerate from 24 FPS to unbound, expressed as 10000 FPS
      PLOGI << "Synchronization mode: " << m_videoSyncMode << " with maximum FPS: " << m_maxFramerate << ", display FPS: " << pfRefreshRate;
   }


   // Touch screen support

#if defined(ENABLE_SDL_VIDEO) // SDL windowing
   #if (defined(__APPLE__) && TARGET_OS_IOS) || defined(__ANDROID__)
       m_supportsTouch = true;
   #endif
#else // Win32 Windowing
    // Check for Touch support
    m_supportsTouch = ((GetSystemMetrics(SM_DIGITIZER) & NID_READY) != 0) && ((GetSystemMetrics(SM_DIGITIZER) & NID_MULTI_INPUT) != 0)
        && (GetSystemMetrics(SM_MAXIMUMTOUCHES) != 0);

   #if 1 // we do not want to handle WM_TOUCH
   #if (WINVER < 0x0602)
       if (!UnregisterTouchWindow)
           UnregisterTouchWindow = (pUnregisterTouchWindow)GetProcAddress(GetModuleHandle(TEXT("user32.dll")), "UnregisterTouchWindow");
       if (UnregisterTouchWindow)
   #endif
           UnregisterTouchWindow(m_playfieldWnd->GetCore());
   #else // would be useful if handling WM_TOUCH instead of WM_POINTERDOWN
       // Disable palm detection
       if (!RegisterTouchWindow)
           RegisterTouchWindow = (pRegisterTouchWindow)GetProcAddress(GetModuleHandle(TEXT("user32.dll")), "RegisterTouchWindow");
       if (RegisterTouchWindow)
           RegisterTouchWindow(m_playfieldWnd->GetCore(), 0);

       if (!IsTouchWindow)
           IsTouchWindow = (pIsTouchWindow)GetProcAddress(GetModuleHandle(TEXT("user32.dll")), "IsTouchWindow");

       // Disable Gesture Detection
       if (!SetGestureConfig)
           SetGestureConfig = (pSetGestureConfig)GetProcAddress(GetModuleHandle(TEXT("user32.dll")), "SetGestureConfig");
       if (SetGestureConfig)
       {
           // http://msdn.microsoft.com/en-us/library/ms812373.aspx
           const DWORD dwHwndTabletProperty =
               TABLET_DISABLE_PRESSANDHOLD |      // disables press and hold (right-click) gesture
               TABLET_DISABLE_PENTAPFEEDBACK |    // disables UI feedback on pen up (waves)
               TABLET_DISABLE_PENBARRELFEEDBACK | // disables UI feedback on pen button down
               TABLET_DISABLE_FLICKS;             // disables pen flicks (back, forward, drag down, drag up)
           LPCTSTR tabletAtom = MICROSOFT_TABLETPENSERVICE_PROPERTY;

           // Get the Tablet PC atom ID
           const ATOM atomID = GlobalAddAtom(tabletAtom);
           if (atomID)
           {
               // Try to disable press and hold gesture 
               SetProp(m_playfieldWnd->GetCore(), tabletAtom, (HANDLE)dwHwndTabletProperty);
           }
           // Gesture configuration
           GESTURECONFIG gc[] = { 0, 0, GC_ALLGESTURES };
           UINT uiGcs = 1;
           const BOOL bResult = SetGestureConfig(m_playfieldWnd->GetCore(), 0, uiGcs, gc, sizeof(GESTURECONFIG));
       }
   #endif

    // Disable visual feedback for touch, this saves one frame of latency on touch displays
    #if (WINVER < 0x0602)
    if (!SetWindowFeedbackSetting)
        SetWindowFeedbackSetting = (pSWFS)GetProcAddress(GetModuleHandle(TEXT("user32.dll")), "SetWindowFeedbackSetting");
    if (SetWindowFeedbackSetting)
    #endif
    {
        constexpr BOOL enabled = FALSE;

        SetWindowFeedbackSetting(m_playfieldWnd->GetCore(), FEEDBACK_TOUCH_CONTACTVISUALIZATION, 0, sizeof(enabled), &enabled);
        SetWindowFeedbackSetting(m_playfieldWnd->GetCore(), FEEDBACK_TOUCH_TAP, 0, sizeof(enabled), &enabled);
        SetWindowFeedbackSetting(m_playfieldWnd->GetCore(), FEEDBACK_TOUCH_DOUBLETAP, 0, sizeof(enabled), &enabled);
        SetWindowFeedbackSetting(m_playfieldWnd->GetCore(), FEEDBACK_TOUCH_PRESSANDHOLD, 0, sizeof(enabled), &enabled);
        SetWindowFeedbackSetting(m_playfieldWnd->GetCore(), FEEDBACK_TOUCH_RIGHTTAP, 0, sizeof(enabled), &enabled);

        SetWindowFeedbackSetting(m_playfieldWnd->GetCore(), FEEDBACK_PEN_BARRELVISUALIZATION, 0, sizeof(enabled), &enabled);
        SetWindowFeedbackSetting(m_playfieldWnd->GetCore(), FEEDBACK_PEN_TAP, 0, sizeof(enabled), &enabled);
        SetWindowFeedbackSetting(m_playfieldWnd->GetCore(), FEEDBACK_PEN_DOUBLETAP, 0, sizeof(enabled), &enabled);
        SetWindowFeedbackSetting(m_playfieldWnd->GetCore(), FEEDBACK_PEN_PRESSANDHOLD, 0, sizeof(enabled), &enabled);
        SetWindowFeedbackSetting(m_playfieldWnd->GetCore(), FEEDBACK_PEN_RIGHTTAP, 0, sizeof(enabled), &enabled);

        SetWindowFeedbackSetting(m_playfieldWnd->GetCore(), FEEDBACK_GESTURE_PRESSANDTAP, 0, sizeof(enabled), &enabled);
    }
#endif

   // FIXME remove or at least move legacy ushock to a plugin
   ushock_output_init();

   // General player initialization

   TRACE_FUNCTION();

   PLOGI << "Initializing player"; // For profiling

   set_lowest_possible_win_timer_resolution();

   m_progressDialog.SetProgress("Initializing Visuals..."s, 10);

   for(unsigned int i = 0; i < eCKeys; ++i)
      m_rgKeys[i] = m_ptable->m_settings.LoadValueInt(Settings::Player, regkey_string[i]);

   m_PlayMusic = m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "PlayMusic"s, true);
   m_PlaySound = m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "PlaySound"s, true);
   m_MusicVolume = m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "MusicVolume"s, 100);
   m_SoundVolume = m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "SoundVolume"s, 100);

   //

   PLOGI << "Initializing renderer (global states & resources)"; // For profiling

   ViewSetup &viewSetup = m_ptable->mViewSetups[m_ptable->m_BG_current_set];
   if (viewSetup.mMode == VLM_WINDOW)
      viewSetup.SetWindowModeFromSettings(m_ptable);

   try
   {
      m_renderer = new Renderer(m_ptable, m_playfieldWnd, m_videoSyncMode, stereo3D);
   }
   catch (HRESULT hr)
   {
      char szFoo[64];
      sprintf_s(szFoo, sizeof(szFoo), "Renderer initialization error code: %x", hr);
      ShowError(szFoo);
      throw hr;
   }

#ifdef __LIBVPINBALL__
   VPinballLib::WindowCreatedData windowCreatedData = {};
#if (defined(__APPLE__) && (defined(TARGET_OS_IOS) && TARGET_OS_IOS))
   SDL_PropertiesID props = SDL_GetWindowProperties(m_playfieldWnd->GetCore());
   windowCreatedData.pWindow = (void*)SDL_GetPointerProperty(props, SDL_PROP_WINDOW_UIKIT_WINDOW_POINTER, NULL);
#endif
   windowCreatedData.pTitle = SDL_GetWindowTitle(m_playfieldWnd->GetCore());
   VPinballLib::VPinball::SendEvent(VPinballLib::Event::WindowCreated, &windowCreatedData);
#endif

   #if defined(ENABLE_BGFX)
   if (m_vrDevice == nullptr) // Anciliary windows are not yet supported while in VR mode
   {
      if (m_scoreviewOutput.GetMode() == VPX::RenderOutput::OM_WINDOW)
         m_renderer->m_renderDevice->AddWindow(m_scoreviewOutput.GetWindow());
      if (m_backglassOutput.GetMode() == VPX::RenderOutput::OM_WINDOW)
         m_renderer->m_renderDevice->AddWindow(m_backglassOutput.GetWindow());
      if (m_topperOutput.GetMode() == VPX::RenderOutput::OM_WINDOW)
         m_renderer->m_renderDevice->AddWindow(m_topperOutput.GetWindow());

      if (m_scoreviewOutput.GetMode() != VPX::RenderOutput::OM_DISABLED)
      {
         m_scoreView.Load(PathFromFilename(m_ptable->m_szFileName));
         if (!m_scoreView.HasLayouts())
            m_scoreView.Load(g_pvp->m_szMyPath + "assets" + PATH_SEPARATOR_CHAR);
      }
   }
   #endif

   // Disable static prerendering for VR and legacy headtracking (this won't be reenabled)
   if (m_headTracking || (stereo3D == STEREO_VR))
      m_renderer->DisableStaticPrePass(true);

   m_renderer->m_renderDevice->m_vsyncCount = 1;

   PLOGI << "Initializing inputs & implicit objects"; // For profiling

   #ifdef _WIN32
      m_pininput.SetFocusWindow(m_playfieldWnd->GetNativeHWND());
   #endif
   m_pininput.Init();

#ifndef __STANDALONE__
   const unsigned int lflip = get_vk(m_rgKeys[eLeftFlipperKey]);
   const unsigned int rflip = get_vk(m_rgKeys[eRightFlipperKey]);

   if (((GetAsyncKeyState(VK_LSHIFT) & 0x8000) && (GetAsyncKeyState(VK_RSHIFT) & 0x8000))
      || ((lflip != ~0u) && (rflip != ~0u) && (GetAsyncKeyState(lflip) & 0x8000) && (GetAsyncKeyState(rflip) & 0x8000)))
   {
      m_ptable->m_tblMirrorEnabled = true;
      int rotation = (int)(m_ptable->mViewSetups[m_ptable->m_BG_current_set].GetRotation(m_playfieldWnd->GetWidth(), m_playfieldWnd->GetHeight())) / 90;
      m_renderer->GetMVP().SetFlip(rotation == 0 || rotation == 2 ? ModelViewProj::FLIPX : ModelViewProj::FLIPY);
   }
   else
#endif
      m_ptable->m_tblMirrorEnabled = m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "mirror"s, false);

#ifndef __STANDALONE__
   // if left flipper or shift hold during load, then swap DT/FS view (for quick testing)
   if (m_ptable->m_BG_current_set != BG_FSS &&
       !m_ptable->m_tblMirrorEnabled &&
       ((GetAsyncKeyState(VK_LSHIFT) & 0x8000)
       || ((lflip != ~0u) && (GetAsyncKeyState(lflip) & 0x8000))))
   {
      switch (m_ptable->m_BG_current_set)
      {
      case BG_DESKTOP: m_ptable->m_BG_override = BG_FSS; break;
      case BG_FSS: m_ptable->m_BG_override = BG_DESKTOP; break;
      default: break;
      }
      m_ptable->UpdateCurrentBGSet();
   }
#endif

   // Initialize default state
   RenderState state;
   state.SetRenderState(RenderState::CULLMODE, m_ptable->m_tblMirrorEnabled ? RenderState::CULL_CW : RenderState::CULL_CCW);
   m_renderer->m_renderDevice->CopyRenderStates(false, state);
   m_renderer->m_renderDevice->SetDefaultRenderState();
   m_renderer->InitLayout();

   Ball::ResetBallIDCounter();

   // Add a playfield primitive if it is missing
   bool hasExplicitPlayfield = false;
   for (size_t i = 0; i < m_ptable->m_vedit.size(); i++)
   {
      IEditable *const pedit = m_ptable->m_vedit[i];
      if (pedit->GetItemType() == ItemTypeEnum::eItemPrimitive && ((Primitive *)pedit)->IsPlayfield())
      {
         hasExplicitPlayfield = true;
         break;
      }
   }
   if (!hasExplicitPlayfield)
   {
      m_implicitPlayfieldMesh = (Primitive *)EditableRegistry::CreateAndInit(ItemTypeEnum::eItemPrimitive, m_ptable, 0, 0);
      if (m_implicitPlayfieldMesh)
      {
         m_implicitPlayfieldMesh->SetName("playfield_mesh"s);
         m_implicitPlayfieldMesh->m_backglass = false;
         m_implicitPlayfieldMesh->m_d.m_staticRendering = true;
         m_implicitPlayfieldMesh->m_d.m_reflectionEnabled = true;
         m_implicitPlayfieldMesh->m_d.m_collidable = false;
         m_implicitPlayfieldMesh->m_d.m_toy = true;
         m_implicitPlayfieldMesh->m_d.m_use3DMesh = true;
         m_implicitPlayfieldMesh->m_d.m_vSize.Set(1.0f, 1.0f, 1.0f);
         m_implicitPlayfieldMesh->m_d.m_depthBias = 100000.0f; // Draw before the other objects
         m_implicitPlayfieldMesh->m_mesh.m_vertices.resize(4);
         m_implicitPlayfieldMesh->m_d.m_disableLightingBelow = 1.0f;
         for (unsigned int y = 0; y <= 1; ++y)
            for (unsigned int x = 0; x <= 1; ++x)
            {
               const unsigned int offs = x + y * 2;
               m_implicitPlayfieldMesh->m_mesh.m_vertices[offs].x = (x & 1) ? m_ptable->m_right : m_ptable->m_left;
               m_implicitPlayfieldMesh->m_mesh.m_vertices[offs].y = (y & 1) ? m_ptable->m_bottom : m_ptable->m_top;
               m_implicitPlayfieldMesh->m_mesh.m_vertices[offs].z = 0.0f;
               m_implicitPlayfieldMesh->m_mesh.m_vertices[offs].tu = (x & 1) ? 1.f : 0.f;
               m_implicitPlayfieldMesh->m_mesh.m_vertices[offs].tv = (y & 1) ? 1.f : 0.f;
               m_implicitPlayfieldMesh->m_mesh.m_vertices[offs].nx = 0.f;
               m_implicitPlayfieldMesh->m_mesh.m_vertices[offs].ny = 0.f;
               m_implicitPlayfieldMesh->m_mesh.m_vertices[offs].nz = 1.f;
            }
         m_implicitPlayfieldMesh->m_mesh.m_indices.resize(6);
         m_implicitPlayfieldMesh->m_mesh.m_indices[0] = 0;
         m_implicitPlayfieldMesh->m_mesh.m_indices[1] = 1;
         m_implicitPlayfieldMesh->m_mesh.m_indices[2] = 2;
         m_implicitPlayfieldMesh->m_mesh.m_indices[3] = 2;
         m_implicitPlayfieldMesh->m_mesh.m_indices[4] = 1;
         m_implicitPlayfieldMesh->m_mesh.m_indices[5] = 3;
         m_implicitPlayfieldMesh->m_mesh.m_validBounds = false;
         m_ptable->m_vedit.push_back(m_implicitPlayfieldMesh);
      }
   }

   // Adjust the implicit playfield reflection probe
   RenderProbe *pf_reflection_probe = m_ptable->GetRenderProbe(PLAYFIELD_REFLECTION_RENDERPROBE_NAME);
   if (pf_reflection_probe)
   {
      vec4 plane = vec4(0.f, 0.f, 1.f, 0.f);
      pf_reflection_probe->SetReflectionPlane(plane);
   }

   PLOGI << "Initializing physics"; // For profiling
   m_progressDialog.SetProgress("Initializing Physics..."s, 30);
   // Need to set timecur here, for init functions that set timers
   m_time_sec = 0.0;
   m_time_msec = m_last_frame_time_msec = 0;
   m_physics = new PhysicsEngine(m_ptable);
   const float minSlope = (m_ptable->m_overridePhysics ? m_ptable->m_fOverrideMinSlope : m_ptable->m_angletiltMin);
   const float maxSlope = (m_ptable->m_overridePhysics ? m_ptable->m_fOverrideMaxSlope : m_ptable->m_angletiltMax);
   const float slope = minSlope + (maxSlope - minSlope) * m_ptable->m_globalDifficulty;
   m_physics->SetGravity(slope, m_ptable->m_overridePhysics ? m_ptable->m_fOverrideGravityConstant : m_ptable->m_Gravity);

   InitFPS();

   //----------------------------------------------------------------------------------

   m_progressDialog.SetProgress("Loading Textures..."s, 50);

   if ((m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "CacheMode"s, 1) > 0) && FileExists(m_ptable->m_szFileName))
   {
      try {
         string dir = g_pvp->m_szMyPrefPath + "Cache" + PATH_SEPARATOR_CHAR + m_ptable->m_szTitle + PATH_SEPARATOR_CHAR;
         std::filesystem::create_directories(std::filesystem::path(dir));
         string path = dir + "used_textures.xml";
         if (FileExists(path))
         {
            PLOGI.printf("Texture cache found at %s", path.c_str());
            std::stringstream buffer;
            std::ifstream myFile(path);
            buffer << myFile.rdbuf();
            myFile.close();
            const string xml = buffer.str();
            tinyxml2::XMLDocument xmlDoc;
            if (xmlDoc.Parse(xml.c_str()) == tinyxml2::XML_SUCCESS)
            {
               auto root = xmlDoc.FirstChildElement("textures");
               for (auto node = root->FirstChildElement("texture"); node != nullptr; node = node->NextSiblingElement())
               {
                  bool linearRGB = false;
                  const char *name = node->GetText();
                  if (name == nullptr)
                     continue;
                  Texture *tex = m_ptable->GetImage(name);
                  if (tex != nullptr && node->QueryBoolAttribute("linear", &linearRGB) == tinyxml2::XML_SUCCESS)
                  {
                     PLOGI << "Texture preloading: '" << name << '\'';
                     m_renderer->m_renderDevice->UploadTexture(tex->m_pdsBuffer, linearRGB);
                  }
               }
            }
         }
      }
      catch (...) // something failed while trying to preload images
      {
         PLOGE << "Texture preloading failed";
      }
   }

   //----------------------------------------------------------------------------------

   PLOGI << "Initializing renderer"; // For profiling
   m_progressDialog.SetProgress("Initializing Renderer..."s, 60);

   // Setup rendering and timers
   for (RenderProbe *probe : m_ptable->m_vrenderprobe)
      probe->RenderSetup(m_renderer);
   for (auto editable : m_ptable->m_vedit)
      if (editable->GetIHitable())
         m_vhitables.push_back(editable);
   for (IEditable *hitable : m_vhitables)
   {
      hitable->GetIHitable()->TimerSetup(m_vht);
      hitable->GetIHitable()->RenderSetup(m_renderer->m_renderDevice);
      if (hitable->GetItemType() == ItemTypeEnum::eItemBall)
         m_vball.push_back(&static_cast<Ball *>(hitable)->m_hitBall);
   }

   // Setup anisotropic filtering
   const bool forceAniso = m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "ForceAnisotropicFiltering"s, true);
   RenderDevice::SetMainTextureDefaultFiltering(forceAniso ? SF_ANISOTROPIC : SF_TRILINEAR);

   #if defined(EXT_CAPTURE)
   if (m_renderer->m_stereo3D == STEREO_VR)
   {
      if (m_capExtDMD)
         StartDMDCapture();
      if (m_capPUP)
         StartPUPCapture();
   }
   #endif

#ifdef __STANDALONE__
   g_pStandalone = Standalone::GetInstance();
   g_pStandalone->PreStartup();
#endif

   if (!IsEditorMode())
   {
      PLOGI << "Starting script"; // For profiling
      m_progressDialog.SetProgress("Starting Game Scripts..."s);

      m_ptable->m_pcv->Start(); // Hook up to events and start cranking script

      // Fire Init event for table object and all 'hitable' parts, also fire Animate event of parts having it since initial setup is considered as the initial animation event
      m_ptable->FireVoidEvent(DISPID_GameEvents_Init);
      for (IEditable *const ph : m_vhitables)
      {
         if (ph->GetIHitable()->GetEventProxyBase())
         {
            ph->GetIHitable()->GetEventProxyBase()->FireVoidEvent(DISPID_GameEvents_Init);
            ItemTypeEnum type = ph->GetItemType();
            if (type == ItemTypeEnum::eItemBumper || type == ItemTypeEnum::eItemDispReel || type == ItemTypeEnum::eItemFlipper || type == ItemTypeEnum::eItemGate
               || type == ItemTypeEnum::eItemHitTarget || type == ItemTypeEnum::eItemLight || type == ItemTypeEnum::eItemSpinner || type == ItemTypeEnum::eItemTrigger)
               ph->GetIHitable()->GetEventProxyBase()->FireVoidEvent(DISPID_AnimateEvents_Animate);
         }
      }
      m_ptable->FireOptionEvent(0); // Custom option init event
      m_ptable->FireVoidEvent(DISPID_GameEvents_Paused);
   }

   // Initialize stereo rendering
   m_renderer->UpdateStereoShaderState();

#ifdef PLAYBACK
   if (m_playback)
      m_fplaylog = fopen("c:\\badlog.txt", "r");
#endif

   // We need to initialize the perf counter before creating the UI which uses it
   wintimer_init();
   m_liveUI = new LiveUI(m_renderer->m_renderDevice);

   m_onGameStartMsgId = VPXPluginAPIImpl::GetMsgID(VPXPI_NAMESPACE, VPXPI_EVT_ON_GAME_START);
   m_onPrepareFrameMsgId = VPXPluginAPIImpl::GetMsgID(VPXPI_NAMESPACE, VPXPI_EVT_ON_PREPARE_FRAME);
   m_onAudioUpdatedMsgId = VPXPluginAPIImpl::GetMsgID(CTLPI_NAMESPACE, CTLPI_AUDIO_ON_UPDATE_MSG);
   MsgPluginManager::GetInstance().GetMsgAPI().SubscribeMsg(VPXPluginAPIImpl::GetInstance().GetVPXEndPointId(), m_onAudioUpdatedMsgId, OnAudioUpdated, this);

   // Signal plugins before performing static prerendering. The only thing not fully initialized is the physics (is this ok ?)
   VPXPluginAPIImpl::GetInstance().BroadcastVPXMsg(m_onGameStartMsgId, nullptr);

   m_scoreView.Select(m_scoreviewOutput);

   // Open UI if requested (this also disables static prerendering, so must be done before performing it)
   if (playMode == 1)
      m_liveUI->OpenTweakMode();
   else if (playMode == 2 && m_renderer->m_stereo3D != STEREO_VR)
      m_liveUI->OpenLiveUI();

   // Pre-render all non-changing elements such as static walls, rails, backdrops, etc. and also static playfield reflections
   // This is done after starting the script and firing the Init event to allow script to adjust static parts on startup
   PLOGI << "Prerendering static parts"; // For profiling
   #if defined(ENABLE_BGFX)
   m_renderer->m_renderDevice->m_frameMutex.lock();
   #endif
   m_renderer->RenderFrame();
   #if defined(ENABLE_BGFX)
   m_renderer->m_renderDevice->m_frameMutex.unlock();
   #endif

   // Reset the perf counter to start time when physics starts
   wintimer_init();
   m_physics->StartPhysics();

   m_progressDialog.SetProgress("Starting..."s, 100);
   if (!IsEditorMode())
      m_ptable->FireVoidEvent(DISPID_GameEvents_UnPaused);

#ifdef __STANDALONE__
#ifndef __LIBVPINBALL__
   if (g_pvp->m_settings.LoadValueWithDefault(Settings::Standalone, "WebServer"s, false))
      g_pvp->m_webServer.Start();
#endif
#endif

   PLOGI << "Startup done"; // For profiling

#ifdef __LIBVPINBALL__
   VPinballLib::VPinball::SendEvent(VPinballLib::Event::PlayerStarted, nullptr);
#endif

#ifdef __STANDALONE__
   g_pStandalone->PostStartup();
#endif

#ifndef __STANDALONE__
   // Disable editor (Note that now that the played table use a copy, we could allow editing while playing but problem may arise with shared parts like images and mesh data)
   g_pvp->GetPropertiesDocker()->EnableWindow(FALSE);
   g_pvp->GetLayersDocker()->EnableWindow(FALSE);
   g_pvp->GetToolbarDocker()->EnableWindow(FALSE);
   if(g_pvp->GetNotesDocker()!=nullptr)
      g_pvp->GetNotesDocker()->EnableWindow(FALSE);
   m_pEditorTable->EnableWindow(FALSE);
   m_progressDialog.Destroy();
   LockForegroundWindow(true);
   if (m_detectScriptHang)
      g_pvp->PostWorkToWorkerThread(HANG_SNOOP_START, NULL);
#endif

   // Broadcast a message to notify front-ends that it is 
   // time to reveal the playfield. 
#ifdef _MSC_VER
   UINT nMsgID = RegisterWindowMessage(_T("VPTableStart"));
   ::PostMessage(HWND_BROADCAST, nMsgID, NULL, NULL);
#endif

   // Show the window (for VR, even without preview, we need to create a window).
   m_focused = true; // For some reason, we do not always receive the 'on focus' event after creation event on SDL. Just take for granted that focus is given upon showing
   m_playfieldWnd->Show();
   m_playfieldWnd->RaiseAndFocus();

   // Popup notification on startup
   if (m_renderer->m_stereo3D != STEREO_OFF && m_renderer->m_stereo3D != STEREO_VR && !m_renderer->m_stereo3Denabled)
      m_liveUI->PushNotification("3D Stereo is enabled but currently toggled off, press F10 to toggle 3D Stereo on"s, 4000);
#ifdef __LIBVPINBALL__
   if (!m_liveUIOverride) {
#endif
      const int numberOfTimesToShowTouchMessage = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "NumberOfTimesToShowTouchMessage"s, 10);
      if (m_supportsTouch && numberOfTimesToShowTouchMessage != 0) //!! visualize with real buttons or at least the areas?? Add extra buttons?
      {
         g_pvp->m_settings.SaveValue(Settings::Player, "NumberOfTimesToShowTouchMessage"s, max(numberOfTimesToShowTouchMessage - 1, 0));
         m_liveUI->PushNotification("You can use Touch controls on this display: bottom left area to Start Game, bottom right area to use the Plunger\n"
                                    "lower left/right for Flippers, upper left/right for Magna buttons, top left for Credits and (hold) top right to Exit"s, 12000);
      }
#ifdef __LIBVPINBALL__
   }
#endif
}

Player::~Player()
{
   assert(g_pplayer == this && g_pplayer->m_closing != CS_CLOSED);

   // note if application exit was requested, and set the new closing state to CLOSED
   bool appExitRequested = (m_closing == CS_CLOSE_APP);
   m_closing = CS_CLOSED;
   PLOGI << "Closing player...";

#ifdef __LIBVPINBALL__
   VPinballLib::VPinball::SendEvent(VPinballLib::Event::PlayerClosing, nullptr);
#endif

   // Signal plugins early since most fields will become invalid
   const unsigned int onGameEndMsgId = VPXPluginAPIImpl::GetMsgID(VPXPI_NAMESPACE, VPXPI_EVT_ON_GAME_END);
   VPXPluginAPIImpl::GetInstance().BroadcastVPXMsg(onGameEndMsgId, nullptr);

   // signal the script that the game is now exited to allow any cleanup
   if (!IsEditorMode())
   {
      m_ptable->FireVoidEvent(DISPID_GameEvents_Exit);
      if (m_detectScriptHang)
         g_pvp->PostWorkToWorkerThread(HANG_SNOOP_STOP, NULL);

      // Stop script engine before destroying objects
      m_ptable->m_pcv->CleanUpScriptEngine();
   }

   // Release plugin message Ids
   MsgPluginManager::GetInstance().GetMsgAPI().UnsubscribeMsg(m_onAudioUpdatedMsgId, OnAudioUpdated);
   VPXPluginAPIImpl::ReleaseMsgID(m_onAudioUpdatedMsgId);
   VPXPluginAPIImpl::ReleaseMsgID(m_onGameStartMsgId);
   VPXPluginAPIImpl::ReleaseMsgID(onGameEndMsgId);
   VPXPluginAPIImpl::ReleaseMsgID(m_onPrepareFrameMsgId);

   // Save list of used textures to avoid stuttering in next play
   if ((m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "CacheMode"s, 1) > 0) && FileExists(m_ptable->m_szFileName))
   {
      string dir = g_pvp->m_szMyPrefPath + "Cache" + PATH_SEPARATOR_CHAR + m_ptable->m_szTitle + PATH_SEPARATOR_CHAR;
      std::filesystem::create_directories(std::filesystem::path(dir));

      tinyxml2::XMLDocument xmlDoc;
      tinyxml2::XMLElement* root;
      ankerl::unordered_dense::map<string, tinyxml2::XMLElement*> textureAge;
      const string path = dir + "used_textures.xml";
      if (FileExists(path))
      {
         std::ifstream myFile(path);
         std::stringstream buffer;
         buffer << myFile.rdbuf();
         myFile.close();
         const string xml = buffer.str();
         if (xmlDoc.Parse(xml.c_str()) == tinyxml2::XML_SUCCESS)
         {
            vector<tinyxml2::XMLElement *> toRemove;
            int age;
            root = xmlDoc.FirstChildElement("textures");
            for (tinyxml2::XMLElement* node = root->FirstChildElement("texture"); node != nullptr; node = node->NextSiblingElement())
            {
               const char *name = node->GetText();
               if (name)
               {
                  const string name_s = name;
                  if (textureAge.count(name_s) == 1)
                     toRemove.push_back(textureAge[name_s]);
                  textureAge[name_s] = node;
                  if (node->QueryIntAttribute("age", &age) == tinyxml2::XML_SUCCESS)
                     node->SetAttribute("age", age + 1);
                  else
                     node->SetAttribute("age", 0);
               }
               else
               {
                  toRemove.push_back(node);
               }
            }
            // Remove old entries (texture that were not used during a high number of play count)
            for (auto it = textureAge.cbegin(); it != textureAge.cend();)
            {
               if (it->second->QueryIntAttribute("age", &age) == tinyxml2::XML_SUCCESS && age >= 100)
               {
                  toRemove.push_back(it->second);
                  it = textureAge.erase(it);
               }
               else
               {
                  ++it;
               }
            }
            // Delete too old, duplicates and invalid nodes
            for (tinyxml2::XMLElement* node : toRemove)
               root->DeleteChild(node);
         }
      }
      else
      {
         root = xmlDoc.NewElement("textures");
         xmlDoc.InsertEndChild(xmlDoc.NewDeclaration());
         xmlDoc.InsertEndChild(root);
      }

      vector<BaseTexture *> textures = m_renderer->m_renderDevice->m_texMan.GetLoadedTextures();
      for (BaseTexture *memtex : textures)
      {
         auto tex = std::ranges::find_if(m_ptable->m_vimage.begin(), m_ptable->m_vimage.end(), [&memtex](Texture *&x) { return (!x->m_szName.empty()) && (x->m_pdsBuffer == memtex); });
         if (tex != m_ptable->m_vimage.end())
         {
            tinyxml2::XMLElement *node = textureAge[(*tex)->m_szName];
            if (node == nullptr)
            {
               node = xmlDoc.NewElement("texture");
               node->SetText((*tex)->m_szName.c_str());
               root->InsertEndChild(node);
            }
            node->DeleteAttribute("clampu");
            node->DeleteAttribute("clampv");
            node->DeleteAttribute("filter");
            node->DeleteAttribute("prerender");
            node->SetAttribute("linear", m_renderer->m_renderDevice->m_texMan.IsLinearRGB(memtex));
            node->SetAttribute("age", 0);
         }
      }

      std::ofstream myfile(path);
      tinyxml2::XMLPrinter prn;
      xmlDoc.Print(&prn);
      myfile << prn.CStr();
      myfile.close();
   }

   // Save adjusted VR settings
   if (m_renderer->m_stereo3D == STEREO_VR)
      m_vrDevice->SaveVRSettings(g_pvp->m_settings);

   for (auto &sound : m_ptable->m_vsound)
   {
      sound->Stop();
      sound->UnInitialize();
   }

   // Stop all played musics, including ones streamed from plugins
   if (m_audio)
      m_audio->MusicPause();
   delete m_audio;
   m_audio = nullptr;
   for (const auto& entry : m_externalAudioPlayers)
   {
      entry.second->MusicPause();
      delete entry.second;
   }
   m_externalAudioPlayers.clear();

   // FIXME remove or at least move legacy ushock to a plugin
   ushock_output_shutdown();

#ifdef EXT_CAPTURE
   StopCaptures();
   g_DXGIRegistry.ReleaseAll();
#endif

   delete m_liveUI;
   m_liveUI = nullptr;
   m_pininput.UnInit();
   delete m_physics;
   m_physics = nullptr;

   for (auto probe : m_ptable->m_vrenderprobe)
      probe->RenderRelease();
   for (auto renderable : m_vhitables)
      renderable->GetIHitable()->RenderRelease();
   for (auto hitable : m_vhitables)
      hitable->GetIHitable()->TimerRelease();
   for (int window = 0; window <= VPXAnciliaryWindow::VPXWINDOW_Topper; window++)
      delete m_anciliaryWndHdrRT[window];
   assert(m_vballDelete.empty());
   m_vball.clear();

   if (m_implicitPlayfieldMesh)
   {
      RemoveFromVectorSingle(m_ptable->m_vedit, (IEditable *)m_implicitPlayfieldMesh);
      m_ptable->m_pcv->RemoveItem(m_implicitPlayfieldMesh->GetScriptable());
      delete m_implicitPlayfieldMesh;
      m_implicitPlayfieldMesh = nullptr;
   }

   m_renderer->m_renderDevice->m_DMDShader->SetTextureNull(SHADER_tex_dmd);
   if (m_dmdFrame)
   {
      m_renderer->m_renderDevice->m_texMan.UnloadTexture(m_dmdFrame);
      delete m_dmdFrame;
      m_dmdFrame = nullptr;
   }

#ifdef PLAYBACK
   if (m_fplaylog)
      fclose(m_fplaylog);
#endif

   for (size_t i = 0; i < m_controlclsidsafe.size(); i++)
      delete m_controlclsidsafe[i];
   m_controlclsidsafe.clear();

   m_changed_vht.clear();

   delete m_pBCTarget;
   m_pBCTarget = nullptr;
   if (m_ptable->m_isLiveInstance)
      delete m_ptable;
   //m_ptable = nullptr;
   #ifdef ENABLE_XR
   if (m_vrDevice)
      m_vrDevice->DiscardVisibilityMask();
   #endif
   delete m_renderer;
   m_renderer = nullptr;
   LockForegroundWindow(false);
   delete m_playfieldWnd;
   m_playfieldWnd = nullptr;

   delete m_vrDevice;
   m_vrDevice = nullptr;

   m_logicProfiler.LogWorstFrame();
   if (&m_logicProfiler != m_renderProfiler)
   {
      m_renderProfiler->LogWorstFrame();
      delete m_renderProfiler;
      m_renderProfiler = nullptr;
   }
   g_frameProfiler = nullptr;

   g_pplayer = nullptr;

   restore_win_timer_resolution();

   while (ShowCursor(FALSE) >= 0);
   while (ShowCursor(TRUE) < 0);

#ifndef __STANDALONE__
   if (m_progressDialog.IsWindow())
      m_progressDialog.Destroy();

   // Reactivate edited table or close application if requested
   if (appExitRequested)
   {
      g_pvp->PostMessage(WM_CLOSE, 0, 0);
   }
   else
   {
      g_pvp->GetPropertiesDocker()->EnableWindow();
      g_pvp->GetLayersDocker()->EnableWindow();
      g_pvp->GetToolbarDocker()->EnableWindow();
      if (g_pvp->GetNotesDocker() != nullptr)
         g_pvp->GetNotesDocker()->EnableWindow();
      g_pvp->ToggleToolbar();
      g_pvp->ShowWindow(SW_SHOW);
      g_pvp->SetForegroundWindow();
      m_pEditorTable->EnableWindow();
      m_pEditorTable->SetFocus();
      m_pEditorTable->SetActiveWindow();
      m_pEditorTable->SetDirtyDraw();
      m_pEditorTable->RefreshProperties();
      m_pEditorTable->BeginAutoSaveCounter();
   }

   ::UnregisterClass(WIN32_PLAYER_WND_CLASSNAME, g_pvp->theInstance);
   #ifdef ENABLE_SDL_VIDEO
   SDL_UnregisterApp();
   #endif
#endif

#ifdef __STANDALONE__
   g_pStandalone->Shutdown();
#endif

   PLOGI << "Player closed.";

#ifdef __LIBVPINBALL__
   VPinballLib::VPinball::SendEvent(VPinballLib::Event::PlayerClosed, nullptr);
#endif
}

void Player::InitFPS()
{
   m_fps = 0.f;
   m_lastMaxChangeTime = 0;
   m_script_max = 0;
   m_physics->ResetStats();
   m_logicProfiler.Reset();
   if (&m_logicProfiler != m_renderProfiler)
      m_renderProfiler->Reset();
}

InfoMode Player::GetInfoMode() const {
   return m_infoMode;
}

ProfilingMode Player::GetProfilingMode() const
{
   const InfoMode mode = GetInfoMode();
   if (mode == IF_PROFILING)
      return ProfilingMode::PF_ENABLED;
   else
      return ProfilingMode::PF_DISABLED;
}

bool Player::ShowFPSonly() const
{
   const InfoMode mode = GetInfoMode();
   return mode == IF_FPS || mode == IF_STATIC_ONLY || mode == IF_AO_ONLY;
}

bool Player::ShowStats() const
{
   const InfoMode mode = GetInfoMode();
   return mode == IF_FPS || mode == IF_PROFILING;
}

void Player::SetPlayState(const bool isPlaying, const U32 delayBeforePauseMs)
{
   bool wasPlaying = IsPlaying();
   if (isPlaying || delayBeforePauseMs == 0)
   {
      m_pauseTimeTarget = 0;
      bool willPlay = isPlaying && m_focused;
      if (wasPlaying != willPlay)
      {
         ApplyPlayingState(willPlay);
         m_playing = isPlaying;
      }
   }
   else if (wasPlaying)
      m_pauseTimeTarget = m_time_msec + delayBeforePauseMs;
}

void Player::OnFocusChanged(const bool isGameFocused)
{
   // A lost focus event happens during player destruction when the main window is destroyed
   if (m_closing == CS_CLOSED)
      return;
   if (isGameFocused)
   {
      PLOGI << "Focus gained";
   }
   else
   {
      #ifdef _MSC_VER
         string focusedWnd = "undefined"s;
         HWND foregroundWnd = GetForegroundWindow();
         if (foregroundWnd)
         {
            DWORD foregroundProcessId;
            DWORD foregroundThreadId = GetWindowThreadProcessId(foregroundWnd, &foregroundProcessId);
            if (foregroundProcessId)
            {
               HANDLE foregroundProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION /* PROCESS_QUERY_INFORMATION | PROCESS_VM_READ */, FALSE, foregroundProcessId);
               if (foregroundProcess)
               {
                  char szFileName[MAXSTRING];
                  if (GetProcessImageFileName(foregroundProcess, szFileName, MAXSTRING))
                     focusedWnd = szFileName;
               }
            }
            char title[1000];
            GetWindowText(foregroundWnd, title, 1000);
            PLOGI << "Focus lost. Current focused window: " << focusedWnd << ", with title: '" << title << '\'';
         }
         else
      #endif
      {
         PLOGI << "Focus lost.";
      }

      #if defined(_MSC_VER) && !defined(DEBUG)
         // FIXME Hacky handling of auxiliary windows (B2S, DMD, Pup,...) stealing focus under Windows: keep focused during first 5 seconds
         // Note that m_liveUI might be null, such as when a message box pops up before the UI finishes initializing
         if (m_time_msec < 5000 && m_liveUI != nullptr && !m_liveUI->IsOpened() && !m_debuggerDialog.IsWindow())
            m_playfieldWnd->RaiseAndFocus();
      #endif
   }
   const bool wasPlaying = IsPlaying();
   const bool willPlay = m_playing && isGameFocused;
   if (wasPlaying != willPlay)
   {
      ApplyPlayingState(willPlay);
      m_focused = isGameFocused;
   }
}

void Player::ApplyPlayingState(const bool play)
{
   #ifndef __STANDALONE__
   if(m_debuggerDialog.IsWindow())
      m_debuggerDialog.SendMessage(RECOMPUTEBUTTONCHECK, 0, 0);
   #endif
   if (play)
   {
      m_LastKnownGoodCounter++; // Reset hang script detection
      m_noTimeCorrect = true;   // Disable physics engine time correction on next physic update
      UnpauseMusic();
      PLOGI << "Unpausing Game";
      if (!IsEditorMode())
         m_ptable->FireVoidEvent(DISPID_GameEvents_UnPaused); // signal the script that the game is now running again
   }
   else
   {
      PauseMusic();
      PLOGI << "Pausing Game";
      if (!IsEditorMode())
         m_ptable->FireVoidEvent(DISPID_GameEvents_Paused); // signal the script that the game is now paused
   }
   UpdateCursorState();
}

void Player::UpdateCursorState() const
{
   if (m_drawCursor || !IsPlaying())
   {
      while (ShowCursor(TRUE) < 0); // FIXME on a system without a mouse, it looks like this may result in an infinite loop
   }
   else
   {
      while (ShowCursor(FALSE) >= 0);
   }
}

HitBall *Player::CreateBall(const float x, const float y, const float z, const float vx, const float vy, const float vz, const float radius, const float mass)
{
   CComObject<Ball>* m_pBall;
   CComObject<Ball>::CreateInstance(&m_pBall);
   m_pBall->AddRef();
   m_pBall->Init(m_ptable, x, y, false, true);
   m_pBall->m_hitBall.m_d.m_pos.z = z + radius;
   m_pBall->m_hitBall.m_d.m_mass = mass;
   m_pBall->m_hitBall.m_d.m_radius = radius;
   m_pBall->m_hitBall.m_d.m_vel.x = vx;
   m_pBall->m_hitBall.m_d.m_vel.y = vy;
   m_pBall->m_hitBall.m_d.m_vel.z = vz;
   m_pBall->m_d.m_useTableRenderSettings = true;
   m_ptable->m_vedit.push_back(m_pBall);
   m_vhitables.push_back(m_pBall);
   m_pBall->TimerSetup(m_vht);
   m_pBall->RenderSetup(m_renderer->m_renderDevice);
   m_pBall->PhysicSetup(m_physics, false);
   if (!m_pactiveballDebug)
      m_pactiveballDebug = &m_pBall->m_hitBall;
   m_vball.push_back(&m_pBall->m_hitBall);
   return &m_pBall->m_hitBall;
}

void Player::DestroyBall(HitBall *pHitBall)
{
   assert(pHitBall);
   if (!pHitBall) return;

   RemoveFromVectorSingle(m_vball, pHitBall);
   m_vballDelete.push_back(pHitBall->m_pBall);
   pHitBall->m_pBall->PhysicRelease(m_physics, false);

   if (m_pactiveball == pHitBall)
      m_pactiveball = m_vball.empty() ? nullptr : m_vball.front();
   if (m_pactiveballDebug == pHitBall)
      m_pactiveballDebug = m_vball.empty() ? nullptr : m_vball.front();
   if (m_pactiveballBC == pHitBall)
      m_pactiveballBC = nullptr;
}


void Player::FireSyncController()
{
   // Legacy implementation: timers with magic interval value have special behaviors: -2 for controller sync event
   for (HitTimer *const pht : m_vht)
      if (pht->m_interval == -2)
      {
         m_logicProfiler.EnterScriptSection(DISPID_TimerEvents_Timer, pht->m_name); 
         pht->m_pfe->FireGroupEvent(DISPID_TimerEvents_Timer);
         m_logicProfiler.ExitScriptSection(pht->m_name);
      }
}

void Player::FireTimers(const unsigned int simulationTime)
{
   HitBall *const old_pactiveball = g_pplayer->m_pactiveball;
   g_pplayer->m_pactiveball = nullptr; // No ball is the active ball for timers/key events
   for (HitTimer *const pht : m_vht)
   {
      if (pht->m_interval >= 0 && pht->m_nextfire <= simulationTime)
      {
         const unsigned int curnextfire = pht->m_nextfire;
         m_logicProfiler.EnterScriptSection(DISPID_TimerEvents_Timer, pht->m_name);
         pht->m_pfe->FireGroupEvent(DISPID_TimerEvents_Timer);
         m_logicProfiler.ExitScriptSection(pht->m_name);
         // Only add interval if the next fire time hasn't changed since the event was run. 
         // Handles corner case:
         //Timer1.Enabled = False
         //Timer1.Interval = 1000
         //Timer1.Enabled = True
         if (curnextfire == pht->m_nextfire && pht->m_interval > 0)
            while (pht->m_nextfire <= simulationTime)
               pht->m_nextfire += pht->m_interval;
      }
   }
   g_pplayer->m_pactiveball = old_pactiveball;
}

void Player::DeferTimerStateChange(HitTimer * const hittimer, bool enabled)
{
   // fakes the disabling of the timer, until it will be catched by the cleanup via m_changed_vht
   hittimer->m_nextfire = enabled ? m_time_msec + hittimer->m_interval : 0xFFFFFFFF;
   // to avoid problems with timers dis/enabling themselves, store all the changes in a list
   for (auto& changed_ht : m_changed_vht)
      if (changed_ht.m_timer == hittimer)
      {
         changed_ht.m_enabled = enabled;
         return;
      }
   TimerOnOff too;
   too.m_enabled = enabled;
   too.m_timer = hittimer;
   m_changed_vht.push_back(too);
}

void Player::ApplyDeferredTimerChanges()
{
   // do the en/disable changes for the timers that piled up
   for (size_t i = 0; i < m_changed_vht.size(); ++i)
      if (m_changed_vht[i].m_enabled) // add the timer?
      {
         if (FindIndexOf(m_vht, m_changed_vht[i].m_timer) < 0)
            m_vht.push_back(m_changed_vht[i].m_timer);
      }
      else // delete the timer?
      {
         const int idx = FindIndexOf(m_vht, m_changed_vht[i].m_timer);
         if (idx >= 0)
            m_vht.erase(m_vht.begin() + idx);
      }
   m_changed_vht.clear();
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// 
// Physics engine
// 
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma region Physics

#ifdef UNUSED_TILT
int Player::NudgeGetTilt()
{
   static U32 last_tilt_time;
   static U32 last_jolt_time;

   if(!m_ptable->m_accelerometerEnabled || m_NudgeManual >= 0 ||               //disabled or in joystick test mode
       m_ptable->m_tilt_amount == 0 || m_ptable->m_jolt_amount == 0) return 0; //disabled

   const U32 ms = msec();

   U32 tilt_2 = 0;
   for (int j = 0; j < m_pininput.m_num_joy; ++j) //find largest value
   {
      tilt_2 = max(tilt_2, (U32)(m_curAccel[j].x * m_curAccel[j].x + m_curAccel[j].y * m_curAccel[j].y)); //always postive numbers
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

#define IIR_Order 4

// coefficients for IIR_Order Butterworth filter set to 10 Hz passband
static constexpr float IIR_a[IIR_Order + 1] = {
   0.0048243445f,
   0.019297378f,
   0.028946068f,
   0.019297378f,
   0.0048243445f };

static constexpr float IIR_b[IIR_Order + 1] = {
   1.00000000f, //if not 1 add division below
   -2.369513f,
   2.3139884f,
   -1.0546654f,
   0.1873795f };

// called on every integral physics frame, only really triggered if before MechPlungerIn() was called, which again relies on an hardware device being used
void Player::MechPlungerUpdate()
{
   static int init = IIR_Order; // first time call
   static float x[IIR_Order + 1] = { 0, 0, 0, 0, 0 };
   static float y[IIR_Order + 1] = { 0, 0, 0, 0, 0 };

   //http://www.dsptutor.freeuk.com/IIRFilterDesign/IIRFilterDesign.html  
   // (this applet is set to 8000Hz sample rate, therefore, multiply ...
   // our values by 80 to shift sample clock of 100hz to 8000hz)

   m_plungerUpdateCount++;
   if (m_plungerUpdateCount <= 3)
   {
      init = IIR_Order;
      m_curMechPlungerPos = 0;
      return; // not until a real value is entered
   }

   // get the sum of current plunger inputs across joysticks
   float curPos = m_pininput.GetPlungerPos();

   if (!m_ptable->m_plungerFilter)
   {
      m_curMechPlungerPos = curPos;
      return;
   }

   x[0] = curPos; //initialize filter
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

float Player::GetMechPlungerSpeed() const 
{ 
    return m_pininput.GetPlungerSpeed();
}

//++++++++++++++++++++++++++++++++++++++++

#pragma endregion


string Player::GetPerfInfo()
{
   // Make it more or less readable by updating only once per second
   static string txt;
   static U32 lastUpdate = 0;
   U32 now = msec();
   if (lastUpdate != 0 && now - lastUpdate < 1000)
      return txt;

   lastUpdate = now;
   std::ostringstream info;
   info << std::fixed << std::setprecision(1);
   
   const bool resetMax = m_time_msec - m_lastMaxChangeTime > 1000;

   if (m_logicProfiler.GetPrev(FrameProfiler::PROFILE_SCRIPT) > m_script_max || resetMax)
      m_script_max = m_logicProfiler.GetPrev(FrameProfiler::PROFILE_SCRIPT);

   if (resetMax)
      m_lastMaxChangeTime = m_time_msec;

   // Renderer additional information
   info << "Triangles: " << ((m_renderer->m_renderDevice->m_frameDrawnTriangles + 999) / 1000) << "k per frame, "
        << ((m_renderer->GetNPrerenderTris() + m_renderer->m_renderDevice->m_frameDrawnTriangles + 999) / 1000) << "k overall. DayNight " << quantizeUnsignedPercent(m_renderer->m_globalEmissionScale)
        << "%\n";
   info << "Draw calls: " << m_renderer->m_renderDevice->Perf_GetNumDrawCalls() << "  (" << m_renderer->m_renderDevice->Perf_GetNumLockCalls() << " Locks)\n";
   info << "State changes: " << m_renderer->m_renderDevice->Perf_GetNumStateChanges() << '\n';
   info << "Texture changes: " << m_renderer->m_renderDevice->Perf_GetNumTextureChanges() << " (" << m_renderer->m_renderDevice->Perf_GetNumTextureUploads() << " Uploads)\n";
   info << "Shader/Parameter changes: " << m_renderer->m_renderDevice->Perf_GetNumTechniqueChanges() << " / " << m_renderer->m_renderDevice->Perf_GetNumParameterChanges() << '\n';
   info << "Objects: " << static_cast<unsigned int>(m_vhitables.size()) << '\n';
   info << '\n';

   // Physics additional information
   info << m_physics->GetPerfInfo(resetMax);
   info << "Ball Velocity / Ang.Vel.: " << (m_pactiveball ? (m_pactiveball->m_d.m_vel + (float)PHYS_FACTOR * m_physics->GetGravity()).Length() : -1.f) << ' '
        << (m_pactiveball ? (m_pactiveball->m_angularmomentum / m_pactiveball->Inertia()).Length() : -1.f) << '\n';

   info << "Flipper keypress to rotate: "
      << ((INT64)(m_pininput.m_leftkey_down_usec_rotate_to_end - m_pininput.m_leftkey_down_usec) < 0 ? int_as_float(0x7FC00000) : (double)(m_pininput.m_leftkey_down_usec_rotate_to_end - m_pininput.m_leftkey_down_usec) / 1000.) << " ms ("
      << ((int)(m_pininput.m_leftkey_down_frame_rotate_to_end - m_pininput.m_leftkey_down_frame) < 0 ? -1 : (int)(m_pininput.m_leftkey_down_frame_rotate_to_end - m_pininput.m_leftkey_down_frame)) << " f) to eos: "
      << ((INT64)(m_pininput.m_leftkey_down_usec_EOS - m_pininput.m_leftkey_down_usec) < 0 ? int_as_float(0x7FC00000) : (double)(m_pininput.m_leftkey_down_usec_EOS - m_pininput.m_leftkey_down_usec) / 1000.) << " ms ("
      << ((int)(m_pininput.m_leftkey_down_frame_EOS - m_pininput.m_leftkey_down_frame) < 0 ? -1 : (int)(m_pininput.m_leftkey_down_frame_EOS - m_pininput.m_leftkey_down_frame)) << " f)\n";

   // Draw performance readout - at end of CPU frame, so hopefully the previous frame
   //  (whose data we're getting) will have finished on the GPU by now.
   #if defined(ENABLE_DX9) // No GPU profiler for OpenGL / BGFX
   if (GetProfilingMode() != PF_DISABLED && m_closing == CS_PLAYING)
   {
      info << '\n';
      info << "Detailed (approximate) GPU profiling:\n";

      m_renderer->m_gpu_profiler.WaitForDataAndUpdate();

      double dTDrawTotal = 0.0;
      for (GTS gts = GTS_BeginFrame; gts < GTS_EndFrame; gts = GTS(gts + 1))
         dTDrawTotal += m_renderer->m_gpu_profiler.DtAvg(gts);

      info << std::setw(4) << std::setprecision(2);
      if (GetProfilingMode() == PF_ENABLED)
      {
         info << " Draw time: " << float(1000.0 * dTDrawTotal) << " ms\n";
         for (GTS gts = GTS(GTS_BeginFrame + 1); gts < GTS_EndFrame; gts = GTS(gts + 1))
         {
            info << "   " << GTS_name[gts] << ": " << float(1000.0 * m_renderer->m_gpu_profiler.DtAvg(gts)) << " ms (" << float(100. * m_renderer->m_gpu_profiler.DtAvg(gts) / dTDrawTotal) << "%%)\n";
         }
         info << " Frame time: " << float(1000.0 * (dTDrawTotal + m_renderer->m_gpu_profiler.DtAvg(GTS_EndFrame))) << " ms\n";
      }
      else
      {
         for (GTS gts = GTS(GTS_BeginFrame + 1); gts < GTS_EndFrame; gts = GTS(gts + 1))
         {
            info << ' ' << GTS_name_item[gts] << ": " << float(1000.0 * m_renderer->m_gpu_profiler.DtAvg(gts)) << " ms (" << float(100. * m_renderer->m_gpu_profiler.DtAvg(gts) / dTDrawTotal)
                 << "%%)\n";
         }
      }
   }
   #endif

   txt = info.str();

   return txt;
}

void Player::LockForegroundWindow(const bool enable)
{
#ifdef _MSC_VER
   // TODO how do we handle this situation with multiple windows, some being full-screen, other not ?
   if (m_playfieldWnd->IsFullScreen()) // revert special tweaks of exclusive full-screen app
      ::LockSetForegroundWindow(enable ? LSFW_LOCK : LSFW_UNLOCK);
#endif
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void Player::GameLoop(std::function<void()> ProcessOSMessages)
{
   // Stereo must be run unthrottled to let OpenVR set the frame pace according to the head set
   assert(!(m_renderer->m_stereo3D == STEREO_VR && (m_videoSyncMode != VideoSyncMode::VSM_NONE || m_maxFramerate < 1000.f)));

   auto sync = [this, ProcessOSMessages]()
   {
      // Controller sync
      #ifdef MSVC_CONCURRENCY_VIEWER
      //series.write_flag(_T("Sync"));
      span *tagSpan = new span(series, 1, _T("Sync"));
      #endif
      ProcessOSMessages();
      if (!IsEditorMode())
      {
         m_pininput.ProcessInput(); // Trigger key events to sync with controller
         m_physics->UpdatePhysics(); // Update physics (also triggering events, syncing with controller)
         // TODO These updates should also be done directly in the physics engine after collision events
         FireSyncController(); // Trigger script sync event (to sync solenoids back)
      }
      MsgPluginManager::GetInstance().ProcessAsyncCallbacks();
      #ifdef MSVC_CONCURRENCY_VIEWER
      delete tagSpan;
      #endif
   };

   #ifdef ENABLE_BGFX
      // Flush any pending frame
      m_renderer->m_renderDevice->m_frameReadySem.post();

      #ifdef __ANDROID__
         MultithreadedGameLoop(sync);
      #else
         #ifdef __LIBVPINBALL__
            auto gameLoop = [this, sync]() {
               MultithreadedGameLoop(sync);
            };
            VPinballLib::VPinball::GetInstance().SetGameLoop(gameLoop);
         #else
            MultithreadedGameLoop(sync);
         #endif
      #endif
   #else
      delete m_renderProfiler;
      m_renderProfiler = &m_logicProfiler;
      if (m_videoSyncMode == VideoSyncMode::VSM_FRAME_PACING)
         FramePacingGameLoop(sync);
      else
         GPUQueueStuffingGameLoop(sync);
   #endif
}

void Player::MultithreadedGameLoop(const std::function<void()>& sync)
{
#ifdef ENABLE_BGFX
   m_logicProfiler.SetThreadLock();
   while (GetCloseState() == CS_PLAYING || GetCloseState() == CS_USER_INPUT)
   {
      // Continuously process input, synchronize with emulation and step physics to keep latency low
      sync();

      // If rendering thread is ready, push a new frame as soon as possible
      if (!m_renderer->m_renderDevice->m_framePending && m_renderer->m_renderDevice->m_frameMutex.try_lock())
      {
         FinishFrame();
         m_lastFrameSyncOnFPS = (m_videoSyncMode != VideoSyncMode::VSM_NONE) && ((m_logicProfiler.GetSlidingAvg(FrameProfiler::PROFILE_FRAME) - 100) * m_playfieldWnd->GetRefreshRate() < 1000000);
         PrepareFrame(sync);
         m_renderer->m_renderDevice->m_framePending = true;
         m_renderer->m_renderDevice->m_frameReadySem.post();
         m_renderer->m_renderDevice->m_frameMutex.unlock();
      }
      else
      {
         m_logicProfiler.EnterProfileSection(FrameProfiler::PROFILE_SLEEP);
         // Sadly waiting is very imprecise (at least on Windows) and we suffer a bit from it.
         // On Windows 10/11 experiments show a minimum delay around 300-500us (half a ms) leading to an overall latency around 2ms
         uOverSleep(100000);
         // The other option would be to use spin wait to achieve sub ms overall latency but this is too CPU intensive:
         // YieldProcessor();
         m_logicProfiler.ExitProfileSection();
      }
#if (defined(__APPLE__) && (defined(TARGET_OS_IOS) && TARGET_OS_IOS))
      // iOS has its own game loop so we need to break here
      break;
#endif
   }
#endif
}

void Player::GPUQueueStuffingGameLoop(const std::function<void()>& sync)
{
   // Legacy main loop performs the frame as a single block. This leads to having the input <-> physics stall between frames increasing
   // the latency and causing syncing problems with PinMAME (which runs in real-time and expects real-time inputs, especially for video modes
   // with repeated button presses like Black Rose's "Walk the Plank Video Mode" or Lethal Weapon 3's "Battle Video Mode")
   // This also leads to filling up the GPU render queue leading to a few frame latency, depending on driver setup
   while (GetCloseState() == CS_PLAYING || GetCloseState() == CS_USER_INPUT)
   {
      #ifdef MSVC_CONCURRENCY_VIEWER
      series.write_flag(_T("Frame"));
      #endif

      sync();

      PrepareFrame(sync);

      sync();

      SubmitFrame();

      sync();

      // Present & VSync
      #ifdef MSVC_CONCURRENCY_VIEWER
      span* tagSpan = new span(series, 1, _T("Flip"));
      #endif
      m_renderProfiler->EnterProfileSection(FrameProfiler::PROFILE_RENDER_FLIP);
      m_renderer->m_renderDevice->Flip();
      #if defined(ENABLE_DX9) // DirectX 9 does not support native adaptive sync, so we must emulate it at the application level
      if (m_videoSyncMode == VideoSyncMode::VSM_ADAPTIVE_VSYNC && m_fps > m_maxFramerate * ADAPT_VSYNC_FACTOR)
         m_renderer->m_renderDevice->WaitForVSync(false);
      #endif
      m_renderProfiler->ExitProfileSection();

      FinishFrame();

      // Adjust framerate if requested by user (i.e. not using a synchronization mode that will lead to blocking calls aligned to the display refresh rate)
      if (m_videoSyncMode == VideoSyncMode::VSM_NONE || m_maxFramerate < m_playfieldWnd->GetRefreshRate()) // The synchronization is not already performed by VSYNC
      {
         const int timeForFrame = static_cast<int>(usec() - m_startFrameTick);
         const int targetTime = static_cast<int>(1000000. / (double)m_maxFramerate);
         if (timeForFrame < targetTime)
         {
            m_renderProfiler->EnterProfileSection(FrameProfiler::PROFILE_RENDER_SLEEP);
            uSleep(targetTime - timeForFrame);
            m_renderProfiler->ExitProfileSection();
         }
      }

      #ifdef MSVC_CONCURRENCY_VIEWER
      delete tagSpan;
      #endif
   }
}

void Player::FramePacingGameLoop(const std::function<void()>& sync)
{
   // The main loop tries to perform a constant input/physics cycle at a 1ms pace while feeding the GPU command queue at a stable rate, without multithreading.
   // These 2 tasks are designed as follows:
   // - Input/Physics: acquire, then process input (executing script events that will trigger the PinMAME controller), then allow
   //   physics to catch up to the real machine time. The aim is to run at real time speed since the PinMAME controller does so and requires
   //   its input to be done the same way, and some flipper tricks depend a lot on precise timings.
   // - Rendering: it is performed in 3 steps:
   //   . Collect (C): update table (animation, per frame timers), build a render (R) command sequence (ideally without any GPU interaction, not yet implemented as such)
   //   . Submit  (S): Feed all commands to the GPU command queue
   //   . Finish  (F): Schedule frame presentation at the right time, perform per frame tasks
   //
   // The overall sequence looks like this (input/physics is not shown and is done as frequently as possible on the CPU, frames alternate upper/lower case):
   // Display ......v.......V.......v.......V......
   // CPU     CSS....Fcss....fCSS....Fcss....fCSS..
   // GPU     .RRRRRRR.rrrrrrr.RRRRRRR.rrrrrrr.RRRR
   // It shows that we aim at always having one frame prepared before the next VBlank. This implies a 1 frame latency but allows
   // to keep the GPU mostly always busy with lower stutter risk (missing a frame, not rendering a point in time at the right time).
   //
   // If the system is high end and the table not too demanding, it may look like this:
   // Display ......v.......V.......v.......V......
   // CPU     CS.....Fcs.....fCS.....Fcs.....fCS...
   // GPU     .RRR.....rrr.....RRR.....rrr.....RRR.
   // In these situations, to lower input-render latency, we could delay the frame start instead of starting directly after submitting 
   // the previous frame. This is not implemented for the time being.
   //
   // On the opposite, if the table is too demanding, the VBlank will be ignored and the rendering would try to catch up:
   // Display ......V...X...V....x..V.....X.V.....xV.......VX.....Vx......V.X....
   // CPU     CCSSS..Fccssss.fCCSSSSSFccssssssFCCSSSSSFccssssssFCCSSSSSFccssssssf
   // GPU     ..RRRRRRRRRrrrrrrrrrRRRRRRRRRrrrrrrrrRRRRRRRRRrrrrrrrrRRRRRRRRRrrrr
   // It shows that after the first few frames, the CPU will hit a blocking call when submitting to the GPU render queue (longer submit phase).
   // This would defeat the design since during the blocking call, the CPU is stalled and VPX's input/physics will lag behind PinMAME.
   // It also shows that since frames arrive late, they are pushed to the display out of sync. Wether they will wait for the next VBlank or 
   // not (causing tearing) depends on the user setup (DWM, fullscreen,...).
   //
   // What we do is adjust the target frame length based on averaged previous frame length (sliding average searching to get back to 
   // refresh rate). On the following diagram, it is shown as some 'W' for additional wait during which input/physics is still processed.
   // Display ......V...X...V.....x.V.......VX.....V..x....V....X.V.......Vx.....
   // CPU     CCSSS..WWFccsss..wwfCCSSS..WWFccsss..wwfCCSSS..WWFccsss..wwfCCSSSS.
   // GPU     ..RRRRRRRRR.rrrrrrrrr.RRRRRRRRR.rrrrrrrrr.RRRRRRRRR.rrrrrrrrr.RRRRR
   // This also allows, if selected (not shown), to only use multiples of the refresh rate to enforce that frames are in sync with VBlank.
   constexpr bool debugLog = false;

   while (GetCloseState() == CS_PLAYING || GetCloseState() == CS_USER_INPUT)
   {
      #ifdef MSVC_CONCURRENCY_VIEWER
      series.write_flag(_T("Frame"));
      #endif

      sync();

      PLOGI_IF(debugLog) << "Frame Collect [Last frame length: " << ((double)m_logicProfiler.GetPrev(FrameProfiler::PROFILE_FRAME) / 1000.0) << "ms] at " << usec();
      PrepareFrame(sync);

      sync();

      PLOGI_IF(debugLog) << "Frame Submit at " << usec();
      SubmitFrame();
      m_renderProfiler->EnterProfileSection(FrameProfiler::PROFILE_RENDER_SLEEP);

      // Wait for at least one VBlank after last frame submission (adaptive sync)
      while (m_renderer->m_renderDevice->m_vsyncCount == 0)
      {
         m_curFrameSyncOnVBlank = true;
         YieldProcessor();
         sync();
      }

      // If the user asked to sync on a lower frame rate than the refresh rate, then wait for it
      if (m_maxFramerate != m_playfieldWnd->GetRefreshRate())
      {
         const U64 now = usec();
         const unsigned int refreshLength = static_cast<unsigned int>(1000000. / (double)m_playfieldWnd->GetRefreshRate());
         const unsigned int minimumFrameLength = static_cast<unsigned int>(1000000. / (double)m_maxFramerate);
         const unsigned int maximumFrameLength = 5 * refreshLength;
         const unsigned int targetFrameLength = clamp(refreshLength - 2000, min(minimumFrameLength, maximumFrameLength), maximumFrameLength);
         while (now - m_renderer->m_renderDevice->m_lastPresentFrameTick < targetFrameLength)
         {
            m_curFrameSyncOnFPS = true;
            YieldProcessor();
            sync();
         }
      }

      // Schedule frame presentation, ask for an asynchronous VBlank, start preparing next frame
      #ifdef MSVC_CONCURRENCY_VIEWER
      span* tagSpan = new span(series, 1, _T("Flip"));
      #endif
      m_lastFrameSyncOnVBlank = m_curFrameSyncOnVBlank;
      m_lastFrameSyncOnFPS = m_curFrameSyncOnFPS;
      PLOGI_IF(debugLog) << "Frame Scheduled at " << usec() << ", Waited for VBlank: " << m_curFrameSyncOnVBlank << ", Waited for FPS: " << m_curFrameSyncOnFPS;
      m_renderer->m_renderDevice->m_vsyncCount = 0;
      m_renderProfiler->ExitProfileSection(); // Out of Sleep section
      m_renderProfiler->EnterProfileSection(FrameProfiler::PROFILE_RENDER_FLIP);
      m_renderer->m_renderDevice->Flip();
      m_renderer->m_renderDevice->WaitForVSync(true);
      m_renderProfiler->ExitProfileSection();
      FinishFrame();
      m_curFrameSyncOnVBlank = m_curFrameSyncOnFPS = false;
      #ifdef MSVC_CONCURRENCY_VIEWER
      delete tagSpan;
      #endif
   }
}

extern void PrecompSplineTonemap(const float displayMaxLum, float out[6]);

void Player::PrepareFrame(const std::function<void()>& sync)
{
   // Rendering outputs to m_renderDevice->GetBackBufferTexture(). If MSAA is used, it is resolved as part of the rendering (i.e. this surface is NOT the MSAA rneder surface but its resolved copy)
   // Then it is tonemapped/bloom/dither/... to m_renderDevice->GetPostProcessRenderTarget1() if needed for postprocessing (sharpen, FXAA,...), or directly to the main output framebuffer otherwise
   // The optional postprocessing is done from m_renderDevice->GetPostProcessRenderTarget1() to the main output framebuffer
   #ifdef MSVC_CONCURRENCY_VIEWER
   span* tagSpan = new span(series, 1, _T("Prepare"));
   #endif

   m_logicProfiler.NewFrame(m_time_msec);
   m_logicProfiler.EnterProfileSection(FrameProfiler::PROFILE_PREPARE_FRAME);

   m_overall_frames++; // This causes the next VPinMame <-> VPX sync to update light status which can be heavy since it needs to perform PWM integration of all lights
   m_LastKnownGoodCounter++;
   m_startFrameTick = usec();
   
   VPXPluginAPIImpl::GetInstance().BroadcastVPXMsg(m_onPrepareFrameMsgId, nullptr);
   
   #ifdef EXT_CAPTURE
   // Trigger captures
   if (m_renderer->m_stereo3D == STEREO_VR)
      UpdateExtCaptures();
   #endif

   // Update visually animated parts (e.g. primitives, reels, gates, lights, bumper-skirts, hittargets, etc)
   if (IsPlaying())
   {
      const float diff_time_msec = (float)(m_time_msec - m_last_frame_time_msec);
      m_last_frame_time_msec = m_time_msec;
      if (diff_time_msec > 0.f)
         for (size_t i = 0; i < m_ptable->m_vedit.size(); ++i)
         {
            Hitable *const ph = m_ptable->m_vedit[i]->GetIHitable();
            if (ph)
               ph->UpdateAnimation(diff_time_msec);
         }
   }

   // New Frame event: Legacy implementation with timers with magic interval value have special behaviors, here -1 for onNewFrame event
   for (HitTimer *const pht : m_vht)
      if (pht->m_interval == -1) {
         m_logicProfiler.EnterScriptSection(DISPID_TimerEvents_Timer, pht->m_name); 
         pht->m_pfe->FireGroupEvent(DISPID_TimerEvents_Timer);
         m_logicProfiler.ExitScriptSection(pht->m_name);
      }

   // Check if we should turn animate the plunger light.
   ushock_output_set(HID_OUTPUT_PLUNGER, ((m_time_msec - m_LastPlungerHit) < 512) && ((m_time_msec & 512) > 0));

   // Update Live UI (must be rendered using a display resolution matching the final composition)
   {
      int w, h;
      m_renderer->GetRenderSize(w, h); // LiveUI is rendered after up/downscaling, so don't use AA render size;
      if (m_renderer->IsStereo())
      {
         if (m_vrDevice)
         {
            w = m_vrDevice->GetEyeWidth();
            h = m_vrDevice->GetEyeHeight();
         }
         else if (m_renderer->m_stereo3Denabled)
         {
            // LiveUI is rendered before stereo and upscaling, but after downscaling
            w = min(w, m_renderer->GetBackBufferTexture()->GetWidth());
            h = min(h, m_renderer->GetBackBufferTexture()->GetHeight());
         }
         else
         {
            // FIXME disabled stereo is not really supported since the change to layered rendering, it will fail if AA is not 100%. It has never been properly supported beside fake stereo since render buffer do not have the right resolution
            //assert(false);
            w = m_playfieldWnd->GetWidth();
            h = m_playfieldWnd->GetHeight();
         }
      }
      m_liveUI->Update(w, h);
      #ifdef __LIBVPINBALL__
         if (m_liveUIOverride)
            VPinballLib::VPinball::SendEvent(VPinballLib::Event::LiveUIUpdate, nullptr);
      #endif
      m_physics->ResetPerFrameStats();
   }

   // Shake screen when nudging
   if (m_NudgeShake > 0.0f)
   {
      Vertex2D offset = m_physics->GetScreenNudge();
      m_renderer->SetScreenOffset(m_NudgeShake * offset.x, m_NudgeShake * offset.y);
   }
   else
      m_renderer->SetScreenOffset(0.f, 0.f);

   #if defined(ENABLE_DX9)
   // Kill the profiler so that it does not affect performance
   if (m_infoMode != IF_PROFILING)
      m_renderer->m_gpu_profiler.Shutdown();
   if (GetProfilingMode() == PF_ENABLED)
      m_renderer->m_gpu_profiler.BeginFrame(m_renderer->m_renderDevice->GetCoreDevice());
   #endif
   
   #if defined(ENABLE_BGFX)
   // Since the script can be somewhat lengthy, we do an additional sync here
   sync();
   #endif

   #ifdef MSVC_CONCURRENCY_VIEWER
   delete tagSpan;
   #endif

   #ifdef MSVC_CONCURRENCY_VIEWER
   tagSpan = new span(series, 1, _T("Build.RF"));
   #endif

   RenderDevice *const rd = m_renderer->m_renderDevice;

   // Prepare main 3D scene frame
   m_renderer->RenderFrame();
   RenderTarget *playfieldRT = rd->GetCurrentRenderTarget();

   // Prepare anciliary windows (for the time being always embedded in playfield anciliary windows)
   RenderTarget *scoreViewRT = RenderAnciliaryWindow(VPXAnciliaryWindow::VPXWINDOW_ScoreView, playfieldRT);
   RenderTarget *backglassRT = RenderAnciliaryWindow(VPXAnciliaryWindow::VPXWINDOW_Backglass, playfieldRT);
   RenderTarget *topperRT = RenderAnciliaryWindow(VPXAnciliaryWindow::VPXWINDOW_Topper, playfieldRT);
   
   // Apply screenspace transforms (MSAA, AO, AA, stereo, ball motion blur, tonemapping, dithering, bloom,...)
   m_renderer->PrepareVideoBuffers(m_renderer->m_renderDevice->GetOutputBackBuffer());

   m_logicProfiler.ExitProfileSection();

   #ifdef MSVC_CONCURRENCY_VIEWER
   delete tagSpan;
   #endif
}

void Player::SubmitFrame()
{
   #ifdef MSVC_CONCURRENCY_VIEWER
   span* tagSpan = new span(series, 1, _T("Submit"));
   #endif
   m_renderProfiler->EnterProfileSection(FrameProfiler::PROFILE_RENDER_SUBMIT);
   m_renderer->m_renderDevice->SubmitRenderFrame();
   m_renderProfiler->ExitProfileSection();

   #ifdef MSVC_CONCURRENCY_VIEWER
   delete tagSpan;
   #endif
}

void Player::FinishFrame()
{
   m_physics->OnFinishFrame();

   if (GetProfilingMode() != PF_DISABLED)
      m_renderer->m_gpu_profiler.EndFrame();

   // Update FPS counter
   m_fps = (float) (1e6 / m_logicProfiler.GetSlidingAvg(FrameProfiler::PROFILE_FRAME));

   #ifndef ACCURATETIMERS
      ApplyDeferredTimerChanges();
      FireTimers(m_time_msec);
   #elif !defined(ENABLE_BGFX)
      // Not applied for BGFX as physics & input sync is managed more cleanly in the main (multithreaded) loop
      if (m_videoSyncMode != VideoSyncMode::VSM_FRAME_PACING)
         m_pininput.ProcessInput(); // trigger input events mainly for VPM<->VP roundtrip
   #endif

   // Detect & fire end of music events
   if (IsPlaying() && m_audio && !m_audio->MusicActive())
   {
      delete m_audio;
      m_audio = nullptr;
      if (!IsEditorMode())
         m_ptable->FireVoidEvent(DISPID_GameEvents_MusicDone);
   }

   // Pause after performing a simulation step
   if ((m_pauseTimeTarget > 0) && (m_pauseTimeTarget <= m_time_msec))
      SetPlayState(false);

   // Memory clean up for balls that may have been destroyed from scripts
   for (Ball *const pBall : m_vballDelete)
   {
      pBall->RenderRelease();
      pBall->TimerRelease();
      pBall->Release();
      RemoveFromVectorSingle(m_ptable->m_vedit, static_cast<IEditable *>(pBall));
      RemoveFromVectorSingle(m_vhitables, static_cast<IEditable *>(pBall));
   }
   m_vballDelete.clear();

   // Crash back to the editor
   if (m_ptable->m_pcv->m_scriptError)
   {
      // Stop playing (send close window message)
      m_pEditorTable->m_pcv->m_scriptError = true;
#ifndef __STANDALONE__
      m_closing = CS_STOP_PLAY;
#else
      m_closing = CS_CLOSE_APP;
#endif
   }

   // Close requested with user input
   if (m_closing == CS_USER_INPUT)
   {
      m_closing = CS_PLAYING;
      if (g_pvp->m_disable_pause_menu || m_renderer->m_stereo3D == STEREO_VR)
         m_closing = CS_STOP_PLAY;
      else {
#ifdef __LIBVPINBALL__
         if (m_liveUIOverride)
            VPinballLib::VPinball::SendEvent(VPinballLib::Event::LiveUIToggle, nullptr);
         else
            m_liveUI->OpenMainSplash();
#else
         m_liveUI->OpenMainSplash();
#endif
      }
   }

   // Brute force stop: blast into space
   if (m_closing == CS_FORCE_STOP)
      exit(-9999); 

   // Open debugger window
   if (m_showDebugger && !g_pvp->m_disable_pause_menu && !m_ptable->IsLocked())
   {
      m_debugMode = true;
      m_showDebugger = false;

#ifndef __STANDALONE__
      if (!m_debuggerDialog.IsWindow())
      {
         m_debuggerDialog.Create(m_playfieldWnd->GetNativeHWND());
         m_debuggerDialog.ShowWindow();
      }
      else
         m_debuggerDialog.SetForegroundWindow();

      EndDialog( g_pvp->GetHwnd(), ID_DEBUGWINDOW );
#endif
   }

#ifdef _MSC_VER
   // TODO hacky Win32 management: try to bring PinMAME, B2S, Freezy's DMD, Pup window back on top (to be removed when these extensions will be cleanly handled by cleaned up plugins)
   if (m_overall_frames < 10)
   {
      static const std::array<string,11> overlaylist{ "MAME"s, "Virtual DMD"s, "pygame"s, "PUPSCREEN1"s, "formDMD"s, "PUPSCREEN5"s, "PUPSCREEN2"s, "Form1"s /* Old B2S */, "B2S Backglass Server"s, "B2S Background"s, "B2S DMD"s };
      for (const string &windowtext : overlaylist)
      {
         HWND hVPMWnd = FindWindow(nullptr, windowtext.c_str());
         if (hVPMWnd == nullptr)
            hVPMWnd = FindWindow(windowtext.c_str(), nullptr);
         if (hVPMWnd != nullptr && ::IsWindowVisible(hVPMWnd))
         {
            ::SetWindowPos(hVPMWnd, HWND_TOPMOST, 0, 0, 0, 0, (SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW | SWP_NOACTIVATE)); // in some strange cases the VPinMAME window is not on top, so enforce it
            ::BringWindowToTop(hVPMWnd);
         }
      }
   }
#endif
}

RenderTarget *Player::RenderAnciliaryWindow(VPXAnciliaryWindow window, RenderTarget *embedRT)
{
   RenderDevice *const rd = m_renderer->m_renderDevice;
   int m_outputX, m_outputY, m_outputW, m_outputH;

   VPX::RenderOutput &output = window == VPXAnciliaryWindow::VPXWINDOW_Backglass ? m_backglassOutput :
                               window == VPXAnciliaryWindow::VPXWINDOW_ScoreView ? m_scoreviewOutput :
                               /*window == VPXAnciliaryWindow::VPXWINDOW_Topper ? */ m_topperOutput;
   const string renderPassName = window == VPXAnciliaryWindow::VPXWINDOW_Backglass ? "Backglass Render"s :
                                 window == VPXAnciliaryWindow::VPXWINDOW_ScoreView ? "ScoreView Render"s :
                                /*window == VPXAnciliaryWindow::VPXWINDOW_Topper ? */ "Topper Render"s;
   const string tonemapPassName = window == VPXAnciliaryWindow::VPXWINDOW_Backglass ? "Backglass Tonemap"s :
                                  window == VPXAnciliaryWindow::VPXWINDOW_ScoreView ? "ScoreView Tonemap"s :
                                /*window == VPXAnciliaryWindow::VPXWINDOW_Topper ? */ "Topper Tonemap"s;
   const string hdrRTName = window == VPXAnciliaryWindow::VPXWINDOW_Backglass ? "BackglassBackBuffer"s :
                            window == VPXAnciliaryWindow::VPXWINDOW_ScoreView ? "ScoreViewBackBuffer"s :
                          /*window == VPXAnciliaryWindow::VPXWINDOW_Topper ? */ "TopperBackBuffer"s;

   // TODO implement rendering for VR (on a flasher)
   if (m_vrDevice != nullptr)
      return nullptr;

   RenderTarget *outputRT;
   if (output.GetMode() == VPX::RenderOutput::OM_EMBEDDED)
   {
      outputRT = embedRT;
      m_outputW = output.GetEmbeddedWindow()->GetWidth();
      m_outputH = output.GetEmbeddedWindow()->GetHeight();
      output.GetEmbeddedWindow()->GetPos(m_outputX, m_outputY);
      m_outputY = outputRT->GetHeight() - m_outputY - m_outputH;
   }
   #ifdef ENABLE_BGFX
   else if (output.GetMode() == VPX::RenderOutput::OM_WINDOW)
   {
      outputRT = output.GetWindow()->GetBackBuffer();
      m_outputW = outputRT->GetWidth();
      m_outputH = outputRT->GetHeight();
      m_outputX = m_outputY = 0;
   }
   #endif
   else
   {
      return nullptr;
   }
      
   Matrix3D matWorldViewProj[2];
   matWorldViewProj[0] = Matrix3D::MatrixIdentity();
   matWorldViewProj[0]._11 = 2.f * static_cast<float>(m_outputW) / static_cast<float>(outputRT->GetWidth());
   matWorldViewProj[0]._41 = -1.f + 2.f * m_outputX / static_cast<float>(outputRT->GetWidth());
   matWorldViewProj[0]._22 = -2.f * static_cast<float>(m_outputH) / static_cast<float>(outputRT->GetHeight());
   matWorldViewProj[0]._42 = 1.f - 2.f * m_outputY / static_cast<float>(outputRT->GetHeight());
   const int eyes = rd->GetCurrentRenderTarget()->m_nLayers;
   if (eyes > 1)
      matWorldViewProj[1] = matWorldViewProj[0];
   rd->m_basicShader->SetMatrix(SHADER_matWorldViewProj, &matWorldViewProj[0], eyes);
   rd->m_basicShader->SetFloat(SHADER_alphaTestValue, 0.0f);
   rd->m_basicShader->SetTechnique(SHADER_TECHNIQUE_bg_decal_with_texture);
   rd->ResetRenderState();
   rd->SetRenderState(RenderState::ZWRITEENABLE, RenderState::RS_FALSE);
   rd->SetRenderState(RenderState::ZENABLE, RenderState::RS_FALSE);
   rd->SetRenderState(RenderState::CULLMODE, RenderState::CULL_NONE);
   rd->SetRenderState(RenderState::SRCBLEND, RenderState::SRC_ALPHA);
   rd->SetRenderState(RenderState::DESTBLEND, RenderState::INVSRC_ALPHA);
   rd->SetRenderState(RenderState::BLENDOP, RenderState::BLENDOP_ADD);

   // Performing linear rendering + tonemapping is overkill when used for LDR rendering (Pup pack, B2S,...)
   constexpr bool enableHDR = true; // TODO still needs to implement sRGB image drawing when render target is non linear

   if (output.GetMode() == VPX::RenderOutput::OM_WINDOW)
   {
      if (enableHDR)
      {
         if (m_anciliaryWndHdrRT[window] == nullptr)
            m_anciliaryWndHdrRT[window] = new RenderTarget(rd, SurfaceType::RT_DEFAULT, hdrRTName, m_outputW, m_outputH, colorFormat::RGBA16F, false, 1, "Fatal Error: unable to create anciliary window back buffer");
         rd->SetRenderTarget(renderPassName, m_anciliaryWndHdrRT[window], false, true);
      }
      else
      {
         rd->SetRenderTarget(renderPassName, outputRT, false, true);
      }
   }
   else
   {
      rd->SetRenderTarget(renderPassName, outputRT, true, true);
   }

   VPXRenderContext2D context
   {
      window,
      static_cast<float>(m_outputW), static_cast<float>(m_outputH),
      static_cast<float>(m_outputW), static_cast<float>(m_outputH),
      [](VPXRenderContext2D *ctx, VPXTexture texture,
         const float tintR, const float tintG, const float tintB, const float alpha,
         const float srcX, const float srcY, const float srcW, const float srcH,
         const float dstX, const float dstY, const float dstW, const float dstH)
         {
            BaseTexture *const tex = static_cast<BaseTexture *>(texture);
            RenderDevice * const rd = g_pplayer->m_renderer->m_renderDevice;
            rd->SetRenderState(RenderState::ALPHABLENDENABLE, (alpha != 1.f || !tex->IsOpaque()) ? RenderState::RS_TRUE : RenderState::RS_FALSE);
            rd->m_basicShader->SetVector(SHADER_cBase_Alpha, tintR, tintG, tintB, alpha);
            rd->m_basicShader->SetTexture(SHADER_tex_base_color, tex);
            const float vx1 = dstX / ctx->srcWidth;
            const float vy1 = dstY / ctx->srcHeight;
            const float vx2 = vx1 + dstW / ctx->srcWidth;
            const float vy2 = vy1 + dstH / ctx->srcHeight;
            const float tx1 = srcX / tex->width();
            const float ty1 = 1.f - srcY / tex->height();
            const float tx2 = (srcX + srcW) / tex->width();
            const float ty2 = 1.f - (srcY + srcH) / tex->height();
            const Vertex3D_NoTex2 vertices[4] = {
               { vx2, vy1, 0.f, 0.f, 0.f, 1.f, tx2, ty2 },
               { vx1, vy1, 0.f, 0.f, 0.f, 1.f, tx1, ty2 },
               { vx2, vy2, 0.f, 0.f, 0.f, 1.f, tx2, ty1 },
               { vx1, vy2, 0.f, 0.f, 0.f, 1.f, tx1, ty1 } };
            rd->DrawTexturedQuad(rd->m_basicShader, vertices, true, 0.f);
         }
   };

   // TODO evaluate if clear is needed and perform it

   // TODO cache all this, and priorize renderers according to user settings
   const MsgPluginAPI *m_msgApi = &MsgPluginManager::GetInstance().GetMsgAPI();
   unsigned int m_getAuxRendererId = 0, m_onAuxRendererChgId = 0;
   m_getAuxRendererId = m_msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_MSG_GET_AUX_RENDERER);
   m_onAuxRendererChgId = m_msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_EVT_AUX_RENDERER_CHG);
   GetAnciliaryRendererMsg getAuxRendererMsg { window };
   getAuxRendererMsg.maxEntryCount = 256;
   getAuxRendererMsg.entries = new AnciliaryRendererDef[getAuxRendererMsg.maxEntryCount];
   m_msgApi->BroadcastMsg(VPXPluginAPIImpl::GetInstance().GetVPXEndPointId(), m_getAuxRendererId, &getAuxRendererMsg);
   bool rendered = false;
   for (unsigned int i = 0; i < getAuxRendererMsg.count; i++)
   {
      rendered = getAuxRendererMsg.entries[i].Render(&context, getAuxRendererMsg.entries[i].context);
      if (rendered)
         break;
   }
   delete[] getAuxRendererMsg.entries;
   m_msgApi->ReleaseMsgID(m_getAuxRendererId);
   m_msgApi->ReleaseMsgID(m_onAuxRendererChgId);

   m_renderer->UpdateBasicShaderMatrix();

   if (!rendered)
   {
      // FIXME integrate score view in the global rendering scheme
      if (window == VPXAnciliaryWindow::VPXWINDOW_ScoreView && m_scoreView.IsMatched())
      {
         m_scoreView.Render(m_scoreviewOutput);
         #ifdef ENABLE_BGFX
         if (output.GetMode() == VPX::RenderOutput::OM_WINDOW)
            output.GetWindow()->Show();
         #endif
         return rd->GetCurrentRenderTarget();
      }
      else
      {
         rd->SetRenderTarget("PostProcess"s, m_renderer->GetBackBufferTexture(), true);
         return nullptr;
      }
   }

   if (enableHDR && (output.GetMode() == VPX::RenderOutput::OM_WINDOW))
   {
      const float jitter = (float)((msec() & 2047) / 1000.0);
      rd->SetRenderTarget(tonemapPassName, outputRT, true, true);
      rd->AddRenderTargetDependency(m_anciliaryWndHdrRT[window], false);
      rd->m_FBShader->SetTextureNull(SHADER_tex_depth);
      rd->m_FBShader->SetTexture(SHADER_tex_fb_unfiltered, m_anciliaryWndHdrRT[window]->GetColorSampler());
      rd->m_FBShader->SetTexture(SHADER_tex_fb_filtered, m_anciliaryWndHdrRT[window]->GetColorSampler());
      rd->m_FBShader->SetVector(SHADER_bloom_dither_colorgrade,
         0.f, // Bloom
         output.GetWindow()->IsWCGBackBuffer() ? 0.f : 1.f, // Dither
         0.f, // LUT colorgrade
         0.f);
      rd->m_FBShader->SetVector(SHADER_w_h_height, 
         static_cast<float>(1.0 / static_cast<double>(m_outputW)),
         static_cast<float>(1.0 / static_cast<double>(m_outputH)),
         jitter, // radical_inverse(jittertime) * 11.0f,
         jitter); // sobol(jittertime) * 13.0f); // jitter for dither pattern}
      ShaderTechniques tonemapTechnique; // FIXME use a tonemapping corresponding to the output, handling situations where playfield is on a HDR display but backglass is not
      switch (m_renderer->m_toneMapper)
      {
      case TM_REINHARD: tonemapTechnique = SHADER_TECHNIQUE_fb_rhtonemap_no_filter; break;
      case TM_FILMIC: tonemapTechnique = SHADER_TECHNIQUE_fb_fmtonemap_no_filter; break;
      case TM_NEUTRAL: tonemapTechnique = SHADER_TECHNIQUE_fb_nttonemap_no_filter; break;
      case TM_AGX: tonemapTechnique = SHADER_TECHNIQUE_fb_agxtonemap_no_filter; break;
      case TM_AGX_PUNCHY: tonemapTechnique = SHADER_TECHNIQUE_fb_agxptonemap_no_filter; break;
      default: assert(!"unknown tonemapper"); break;
      }
      rd->m_FBShader->SetTechnique(tonemapTechnique);
      const float m_exposure = 1.f; // TODO implement exposure
      if (output.GetWindow()->IsWCGBackBuffer())
      {
         const float maxDisplayLuminance = output.GetWindow()->GetHDRHeadRoom() * (output.GetWindow()->GetSDRWhitePoint() * 80.f); // Maximum luminance of display in nits, note that GetSDRWhitePoint()*80 should usually be in the 200 nits range
         rd->m_FBShader->SetVector(SHADER_exposure_wcg,
            m_exposure,
            (output.GetWindow()->GetSDRWhitePoint() * 80.f) / maxDisplayLuminance, // Apply SDR whitepoint (1.0 -> white point in nits), then scale down by maximum luminance (in nits) of display to get a relative value before before tonemapping, equal to 1/GetHDRHeadRoom()
            maxDisplayLuminance / 10000.f, // Apply back maximum luminance in nits of display after tonemapping, scaled down to PQ limits (1.0 is 10000 nits)
            1.f);
         float spline_params[6];
         PrecompSplineTonemap(maxDisplayLuminance, spline_params);
         rd->m_FBShader->SetVector(SHADER_spline1, spline_params[0],spline_params[1],spline_params[2],spline_params[3]);
         rd->m_FBShader->SetVector(SHADER_spline2, spline_params[4],spline_params[5], 0.f,0.f);
      }
      else
      {
         rd->m_FBShader->SetVector(SHADER_exposure_wcg, 
            m_exposure,
            1.f, 
            0.f,  // Unused for SDR
            0.f); // Tonemapping mode: 0 = SDR
      }
      rd->DrawFullscreenTexturedQuad(rd->m_FBShader);
   }

   #ifdef ENABLE_BGFX
   if (output.GetMode() == VPX::RenderOutput::OM_WINDOW)
      output.GetWindow()->Show();
   #endif

   return rd->GetCurrentRenderTarget();
}

void Player::UpdateVolume()
{
   if (m_audio)
      m_audio->UpdateVolume();
   for (auto sound : m_ptable->m_vsound)
      sound->UpdateVolume();
   for (const auto& players : m_externalAudioPlayers)
      players.second->UpdateVolume();
}

void Player::OnAudioUpdated(const unsigned int msgId, void* userData, void* msgData)
{
   Player *me = static_cast<Player *>(userData);
   AudioUpdateMsg &msg = *static_cast<AudioUpdateMsg *>(msgData);
   const auto &entry = me->m_externalAudioPlayers.find(msg.id.id);
   if (entry == me->m_externalAudioPlayers.end())
   {
      if (msg.buffer != nullptr)
      {
         const int nChannels = (msg.type == CTLPI_AUDIO_SRC_BACKGLASS_MONO) ? 1 : 2;
         PinSound* const pPinSound = new PinSound(me->m_ptable->m_settings); //!!??
         pPinSound->StreamInit(static_cast<DWORD>(msg.sampleRate), nChannels, 1.f);
         pPinSound->StreamUpdate(msg.buffer, msg.bufferSize);
         me->m_externalAudioPlayers[msg.id.id] = pPinSound;
      }
   }
   else
   {
      if (msg.buffer != nullptr)
      {
         int pending = SDL_GetAudioStreamQueued(entry->second->m_pstream);
         if (pending >= 0 && static_cast<unsigned int>(pending) >= msg.bufferSize)
         {
            double delay = 1000.0 * msg.bufferSize / (msg.sampleRate * ((msg.type == CTLPI_AUDIO_SRC_BACKGLASS_MONO) ? 1 : 2));
            PLOGI << "Dropping 1 sound frame (" << delay << "ms) as lag is exceeding 1 frame (" << msg.bufferSize << "bytes of " << ((msg.type == CTLPI_AUDIO_SRC_BACKGLASS_MONO)
               ? "mono" : "stereo") << " data at " << msg.sampleRate << "Hz)";
         }
         else
         {
            // PLOGD << "Pending: " << pending << " queuing: " << msg.bufferSize;
            entry->second->StreamUpdate(msg.buffer, msg.bufferSize);
         }
      }
      else
      {
         entry->second->MusicPause();
         delete entry->second;
         me->m_externalAudioPlayers.erase(entry);
      }
   }
}

void Player::PauseMusic()
{
   if (m_pauseMusicRefCount == 0)
   {
      if (m_audio)
         m_audio->MusicPause();
   }
   m_pauseMusicRefCount++;
}

void Player::UnpauseMusic()
{
   m_pauseMusicRefCount--;
   if (m_pauseMusicRefCount == 0)
   {
      if (m_audio)
         m_audio->MusicUnpause();
   }
   else if (m_pauseMusicRefCount < 0)
      m_pauseMusicRefCount = 0;
}

#ifdef PLAYBACK
float Player::ParseLog(LARGE_INTEGER *pli1, LARGE_INTEGER *pli2)
{
   char szLine[MAXSTRING];
   float dtime = 0.45f;

   while (1)
   {
      int c=0;

      while ((szLine[c] = getc(m_fplaylog)) != '\n')
      {
         if (szLine[c] == EOF)
         {
            fclose(m_fplaylog);
            m_playback = false;
            m_fplaylog = nullptr;
            return dtime;
         }
         c++;
      }

      char szWord[64];
      char szSubWord[64];
      int index;
      sscanf_s(szLine, "%s",szWord, (unsigned)_countof(szWord));

      if (!strcmp(szWord,"Key"))
      {
         sscanf_s(szLine, "%s %s %d",szWord, (unsigned)_countof(szWord), szSubWord, (unsigned)_countof(szSubWord), &index);
         if (!strcmp(szSubWord, "Down"))
         {
            m_ptable->FireGenericKeyEvent(DISPID_GameEvents_KeyDown, index);
         }
         else // Release
         {
            m_ptable->FireGenericKeyEvent(DISPID_GameEvents_KeyUp, index);
         }
      }
      else if (!strcmp(szWord, "Physics"))
      {
         sscanf_s(szLine, "%s %s %f",szWord, (unsigned)_countof(szWord), szSubWord, (unsigned)_countof(szSubWord), &dtime);
      }
      else if (!strcmp(szWord, "Frame"))
      {
         int a,b,c,d;
         sscanf_s(szLine, "%s %s %f %u %u %u %u",szWord, (unsigned)_countof(szWord), szSubWord, (unsigned)_countof(szSubWord), &dtime, &a, &b, &c, &d);
         pli1->HighPart = a;
         pli1->LowPart = b;
         pli2->HighPart = c;
         pli2->LowPart = d;
      }
      else if (!strcmp(szWord, "Step"))
      {
         int a,b,c,d;
         sscanf_s(szLine, "%s %s %u %u %u %u",szWord, (unsigned)_countof(szWord), szSubWord, (unsigned)_countof(szSubWord), &a, &b, &c, &d);
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
