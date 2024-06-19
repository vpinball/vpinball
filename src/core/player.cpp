#include "core/stdafx.h"

#ifdef ENABLE_SDL_VIDEO
  #include <SDL2/SDL_syswm.h>
  #include "imgui/imgui_impl_sdl2.h"
#else
  #include "imgui/imgui_impl_win32.h"
#endif

#ifndef __STANDALONE__
#include "BAM/BAMView.h"
#endif

#ifdef __STANDALONE__
#include "standalone/Standalone.h"
#endif

#include <ctime>
#include <fstream>
#include <sstream>
#include <filesystem>
#include "renderer/Shader.h"
#include "renderer/Anaglyph.h"
#include "renderer/VRDevice.h"
#include "renderer/typedefs3D.h"
#ifndef __STANDALONE__
#include "renderer/captureExt.h"
#endif
#ifdef _MSC_VER
#include "winsdk/legacy_touch.h"
#endif
#include "tinyxml2/tinyxml2.h"

#if __cplusplus >= 202002L && !defined(__clang__)
#define stable_sort std::ranges::stable_sort
#define sort std::ranges::sort
#else
#define stable_sort std::stable_sort
#define sort std::sort
#endif

#ifdef __STANDALONE__
#include <map>
#endif

#ifdef _MSC_VER
// Used to log which program steals the focus from VPX
#include "psapi.h"
#pragma comment(lib, "Psapi")
#endif


#if !(_WIN32_WINNT >= 0x0500)
 #define KEYEVENTF_SCANCODE    0x0008
#endif /* _WIN32_WINNT >= 0x0500 */


//

#define RECOMPUTEBUTTONCHECK WM_USER+100

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

#ifdef __ANDROID__
#define APPNAME "PLAYER"
#define LOG_TAG "player"

#include "jni.h"
#include "../utils/AndroidLog.h"
#include "../standalone/inc/webserver/WebServer.h"

static WebServer android_webServer;

extern "C" JNIEXPORT void JNICALL Java_org_vpinball_app_VPXViewModel_initwebserver(JNIEnv* env, jobject obj, 
                                 jstring addr, jint port, jboolean debug, jint fd) 
{
   jboolean isCopy;
   auto strAddr = env->GetStringUTFChars(addr, &isCopy);
   auto path = SAFtoPath(fd) + "/";

   android_webServer.Init(strAddr, port, debug, path, path);

   env->ReleaseStringUTFChars(addr, strAddr);
}

extern "C" JNIEXPORT void JNICALL Java_org_vpinball_app_VPXViewModel_webserver(JNIEnv* env, jobject obj, jboolean state) 
{
   if (state) 
      android_webServer.Start();
   else
      android_webServer.Stop();
}

#endif


// leave as-is as e.g. VPM relies on this
#define WIN32_PLAYER_WND_CLASSNAME _T("VPPlayer")
#define WIN32_WND_TITLE _T("Visual Pinball Player")

#if !defined(ENABLE_SDL_VIDEO) // Win32 Windowing
LRESULT CALLBACK PlayerWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   if (g_pplayer == nullptr || g_pplayer->m_playfieldWnd == nullptr || g_pplayer->m_playfieldWnd->GetCore() != hwnd)
      return DefWindowProc(hwnd, uMsg, wParam, lParam);

   #ifndef ENABLE_SDL_VIDEO
   if (ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam))
      return true;
   #endif

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

   case WM_MOUSEMOVE: // Show cursor if paused ot if user move the mouse
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

   // FIXME the following events are not handled by the SDL implementation (in pininput.cpp)
   case MM_MIXM_CONTROL_CHANGE: // not implemented for SDL, still the mixer API is fairly buggy (no mapping to the right mixer, fails on some conf)
      mixer_get_volume();
      break;

   case WM_POINTERDOWN: // not implemented for SDL (SDL2 does not support touch devices under windows)
   case WM_POINTERUP:
   {
#ifndef TEST_TOUCH_WITH_MOUSE
      if (!GetPointerInfo)
         GetPointerInfo = (pGPI)GetProcAddress(GetModuleHandle(TEXT("user32.dll")), "GetPointerInfo");
      if (GetPointerInfo)
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

                  DIDEVICEOBJECTDATA didod;
                  didod.dwOfs = g_pplayer->m_rgKeys[touchkeymap[i]];
                  didod.dwData = g_pplayer->m_touchregion_pressed[i] ? 0x80 : 0;
                  g_pplayer->m_pininput.PushQueue(&didod, APP_TOUCH /*, curr_time_msec*/);
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
{
   // For the time being, lots of access are made through the global singleton, so ensure we are unique, and define it as soon as needed
   assert(g_pplayer == nullptr);
   g_pplayer = this; 

   m_progressDialog.Create(g_pvp->GetHwnd());
   m_progressDialog.ShowWindow(g_pvp->m_open_minimized ? SW_HIDE : SW_SHOWNORMAL);
   m_progressDialog.SetProgress("Creating Player..."s, 1);

#if !(defined(_M_IX86) || defined(_M_X64) || defined(_M_AMD64) || defined(__i386__) || defined(__i386) || defined(__i486__) || defined(__i486) || defined(i386) || defined(__ia64__) || defined(__x86_64__))
 #pragma message ( "Warning: No CPU float ignore denorm implemented" )
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

   for (int i = 0; i < PININ_JOYMXCNT; ++i)
      m_curAccel[i] = int2(0,0);

   m_curPlunger = JOYRANGEMN - 1;

#ifdef ENABLE_VR
   const int vrDetectionMode = m_ptable->m_settings.LoadValueWithDefault(Settings::PlayerVR, "AskToTurnOn"s, 0);
   bool useVR = vrDetectionMode == 2 /* VR Disabled */  ? false : VRDevice::IsVRinstalled();
   if (useVR && (vrDetectionMode == 1 /* VR Autodetect => ask to turn on and adapt accordingly */) && !VRDevice::IsVRturnedOn())
      useVR = g_pvp->MessageBox("VR headset detected but SteamVR is not running.\n\nTurn VR on?", "VR Headset Detected", MB_YESNO) == IDYES;
   m_capExtDMD = useVR && m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "CaptureExternalDMD"s, false);
   m_capPUP = useVR && m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "CapturePUP"s, false);
#else
   bool useVR = false;
#endif
   m_vrDevice = useVR ? new VRDevice() : nullptr;
   StereoMode stereo3D = useVR ? STEREO_VR : (StereoMode)m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "Stereo3D"s, (int)STEREO_OFF);
   m_headTracking = useVR ? false : m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "BAMHeadTracking"s, false);
   m_detectScriptHang = m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "DetectHang"s, false);

   m_NudgeShake = m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "NudgeStrength"s, 2e-2f);
   m_scaleFX_DMD = m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "ScaleFXDMD"s, false);

   m_minphyslooptime = min(m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "MinPhysLoopTime"s, 0), 1000);

   m_throwBalls = m_ptable->m_settings.LoadValueWithDefault(Settings::Editor, "ThrowBallsAlwaysOn"s, false);
   m_ballControl = m_ptable->m_settings.LoadValueWithDefault(Settings::Editor, "BallControlAlwaysOn"s, false);
   m_debugBallSize = m_ptable->m_settings.LoadValueWithDefault(Settings::Editor, "ThrowBallSize"s, 50);
   m_debugBallMass = m_ptable->m_settings.LoadValueWithDefault(Settings::Editor, "ThrowBallMass"s, 1.0f);

   for (unsigned int i = 0; i < MAX_TOUCHREGION; ++i)
      m_touchregion_pressed[i] = false;

   m_dmd = int2(0,0);

   PLOGI << "Creating main window"; // For profiling
   {
      #if defined(_MSC_VER) && !defined(__STANDALONE__)
         WNDCLASS wc;
         ZeroMemory(&wc, sizeof(wc));
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
      
      int wnd_width, wnd_height;
      if (stereo3D == STEREO_VR)
      {
         m_fullScreen = false;
         wnd_width = m_ptable->m_settings.LoadValueWithDefault(Settings::PlayerVR, "PreviewWidth"s, 640);
         wnd_height = m_ptable->m_settings.LoadValueWithDefault(Settings::PlayerVR, "PreviewHeight"s, 480);
      }
      else
      {
         m_fullScreen = m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "FullScreen"s, IsWindows10_1803orAbove());
         // command line override
         if (g_pvp->m_disEnableTrueFullscreen == 0)
            m_fullScreen = false;
         else if (g_pvp->m_disEnableTrueFullscreen == 1)
            m_fullScreen = true;
         wnd_width = m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "Width"s, m_fullScreen ? -1 : DEFAULT_PLAYER_WIDTH);
         wnd_height = m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "Height"s, wnd_width * 9 / 16);
      }
      const int display = g_pvp->m_primaryDisplay ? -1 : m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "Display"s, -1);
      const int requestedRefreshRate = m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "RefreshRate"s, 0);
      const bool video10bit = m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "Render10Bit"s, false);
      const int colordepth = (video10bit && m_fullScreen && stereo3D != STEREO_VR) ? 30 : 32;
      if (!m_fullScreen && video10bit)
         ShowError("10Bit-Monitor support requires 'Force exclusive Fullscreen Mode' to be also enabled!");

      m_playfieldWnd = new VPX::Window(WIN32_WND_TITLE, "Playfield", display, wnd_width, wnd_height, m_fullScreen, colordepth, requestedRefreshRate);

      int pfRefreshRate = m_playfieldWnd->GetRefreshRate();
      m_maxFramerate = m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "MaxFramerate"s, -1);
      if(m_maxFramerate > 0 && m_maxFramerate < 24) // at least 24 fps
         m_maxFramerate = 24;
      m_videoSyncMode = (VideoSyncMode)m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "SyncMode"s, VSM_INVALID);
      if (m_maxFramerate < 0 && m_videoSyncMode == VideoSyncMode::VSM_INVALID)
      {
         const int vsync = m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "AdaptiveVSync"s, -1);
         switch (vsync)
         {
         case -1: m_maxFramerate = pfRefreshRate; m_videoSyncMode = VideoSyncMode::VSM_FRAME_PACING; break;
         case 0: m_maxFramerate = pfRefreshRate; m_videoSyncMode = VideoSyncMode::VSM_NONE; break;
         case 1: m_maxFramerate = pfRefreshRate; m_videoSyncMode = VideoSyncMode::VSM_VSYNC; break;
         case 2: m_maxFramerate = pfRefreshRate; m_videoSyncMode = VideoSyncMode::VSM_ADAPTIVE_VSYNC; break;
         default: m_maxFramerate = pfRefreshRate; m_videoSyncMode = VideoSyncMode::VSM_ADAPTIVE_VSYNC; break;
         }
      }
      if (m_videoSyncMode == VideoSyncMode::VSM_INVALID)
         m_videoSyncMode = VideoSyncMode::VSM_FRAME_PACING;
      if (m_maxFramerate < 0) // Negative is display refresh rate
         m_maxFramerate = pfRefreshRate;
      if (m_maxFramerate == 0) // 0 is unbound refresh rate
         m_maxFramerate = 10000;
      if (m_videoSyncMode != VideoSyncMode::VSM_NONE && m_maxFramerate > pfRefreshRate)
         m_maxFramerate = pfRefreshRate;
      if (useVR)
      {
         // Disable VSync for VR (sync is performed by the OpenVR runtime)
         m_videoSyncMode = VideoSyncMode::VSM_NONE;
         m_maxFramerate = 10000;
      }
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
       if (!UnregisterTouchWindow)
           UnregisterTouchWindow = (pUnregisterTouchWindow)GetProcAddress(GetModuleHandle(TEXT("user32.dll")), "UnregisterTouchWindow");
       if (UnregisterTouchWindow)
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

    // Disable visual feedback for touch, this saves one frame of latency on touchdisplays
    if (!SetWindowFeedbackSetting)
        SetWindowFeedbackSetting = (pSWFS)GetProcAddress(GetModuleHandle(TEXT("user32.dll")), "SetWindowFeedbackSetting");
    if (SetWindowFeedbackSetting)
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

   #ifdef ENABLE_SDL_VIDEO // SDL Windowing
   mixer_init(nullptr); // FIXME
   #else // Win32 Windowing
   mixer_init(m_playfieldWnd->GetCore());
   #endif
   hid_init();

   // General player initialization

   TRACE_FUNCTION();

   PLOGI << "Initializing player"; // For profiling

   set_lowest_possible_win_timer_resolution();

   m_progressDialog.SetProgress("Initializing Visuals..."s, 10);

   for(unsigned int i = 0; i < eCKeys; ++i)
   {
      int key;
      const bool hr = m_ptable->m_settings.LoadValue(Settings::Player, regkey_string[i], key);
      if (!hr || key > 0xdd)
          key = regkey_defdik[i];
      m_rgKeys[i] = (EnumAssignKeys)key;
   }

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

   // Disable static prerendering for VR and legacy headtracking (this won't be reenabled)
   if (m_headTracking || (stereo3D == STEREO_VR))
      m_renderer->DisableStaticPrePass(true);

   m_renderer->m_pd3dPrimaryDevice->m_vsyncCount = 1;

   PLOGI << "Initializing inputs & implicit objects"; // For profiling

   m_pininput.LoadSettings(m_ptable->m_settings);
   #ifdef _WIN32
      #ifdef ENABLE_SDL_VIDEO // SDL Windowing
      SDL_SysWMinfo wmInfo;
      SDL_VERSION(&wmInfo.version);
      SDL_GetWindowWMInfo(m_playfieldWnd->GetCore(), &wmInfo);
      HWND hwnd = wmInfo.info.win.window;
      #else // Win32 Windowing
      HWND hwnd = m_playfieldWnd->GetCore();
      #endif
      m_pininput.Init(hwnd);
   #else
      m_pininput.Init();
   #endif

#ifndef __STANDALONE__
   //
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
   m_renderer->m_pd3dPrimaryDevice->CopyRenderStates(false, state);
   m_renderer->m_pd3dPrimaryDevice->SetDefaultRenderState();
   m_renderer->InitLayout();

   m_accelerometer = Vertex2D(0.f, 0.f);

   Ball::ballID = 0;

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
#ifdef __STANDALONE__
            PLOGI.printf("Texture cache found at %s", path.c_str());
#endif
            std::stringstream buffer;
            std::ifstream myFile(path);
            buffer << myFile.rdbuf();
            myFile.close();
            auto xml = buffer.str();
            tinyxml2::XMLDocument xmlDoc;
            if (xmlDoc.Parse(xml.c_str()) == tinyxml2::XML_SUCCESS)
            {
               auto root = xmlDoc.FirstChildElement("textures");
               for (auto node = root->FirstChildElement("texture"); node != nullptr; node = node->NextSiblingElement())
               {
                  int filter = 0, clampU = 0, clampV = 0;
                  bool linearRGB = false, preRenderOnly = false;
                  const char *name = node->GetText();
                  Texture *tex = m_ptable->GetImage(name);
                  if (tex == nullptr 
                     || node->QueryBoolAttribute("linear", &linearRGB) != tinyxml2::XML_SUCCESS
                     || node->QueryIntAttribute("clampu", &clampU) != tinyxml2::XML_SUCCESS
                     || node->QueryIntAttribute("clampv", &clampV) != tinyxml2::XML_SUCCESS 
                     || node->QueryIntAttribute("filter", &filter) != tinyxml2::XML_SUCCESS
                     || node->QueryBoolAttribute("prerender", &preRenderOnly) != tinyxml2::XML_SUCCESS)
                  {
                     PLOGE << "Texture preloading failed for '" << name << "'. Preloading aborted";
                     break; // Stop preloading on first error
                  }
                  // For dynamic modes (VR, head tracking,...) mark all preloaded textures as static only
                  // This will make the cache wrong for the next non static run but it will rebuild, while the opposite would not (all preloads would stay as not prerender only)
                  m_renderer->m_render_mask = (!m_renderer->IsUsingStaticPrepass() || preRenderOnly) ? Renderer::STATIC_ONLY : Renderer::DEFAULT;
                  m_renderer->m_pd3dPrimaryDevice->m_texMan.LoadTexture(tex->m_pdsBuffer, (SamplerFilter)filter, (SamplerAddressMode)clampU, (SamplerAddressMode)clampV, linearRGB);
                  PLOGI << "Texture preloading: '" << name << '\'';
               }
            }
         }
      }
      catch (...) // something failed while trying to preload images
      {
         PLOGE << "Texture preloading failed";
      }
      m_renderer->m_render_mask = Renderer::DEFAULT;
   }

   //----------------------------------------------------------------------------------

   PLOGI << "Initializing renderer"; // For profiling
   m_progressDialog.SetProgress("Initializing Renderer..."s, 60);

   m_renderer->SetupShaders();

   // search through all collection for elements which support group rendering
   for (int i = 0; i < m_ptable->m_vcollection.size(); i++)
   {
      Collection * const pcol = m_ptable->m_vcollection.ElementAt(i);
      for (int t = 0; t < pcol->m_visel.size(); t++)
      {
         // search for a primitive in the group, if found try to create a grouped render element
         ISelect * const pisel = pcol->m_visel.ElementAt(t);
         if (pisel != nullptr && pisel->GetItemType() == eItemPrimitive)
         {
            Primitive * const prim = (Primitive*)pisel;
            prim->CreateRenderGroup(pcol);
            break;
         }
      }
   }

   // Start the frame.
   for (auto editable : m_ptable->m_vedit)
      if (editable->GetIHitable())
      {
         editable->GetIHitable()->BeginPlay(m_vht);
         m_vhitables.push_back(editable->GetIHitable());
      }
   for (RenderProbe* probe : m_ptable->m_vrenderprobe)
      probe->RenderSetup(m_renderer->m_pd3dPrimaryDevice);
   for (Hitable* hitable : m_vhitables)
      hitable->RenderSetup(m_renderer->m_pd3dPrimaryDevice);

   // Setup anisotropic filtering
   const bool forceAniso = m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "ForceAnisotropicFiltering"s, true);
   m_renderer->m_pd3dPrimaryDevice->SetMainTextureDefaultFiltering(forceAniso ? SF_ANISOTROPIC : SF_TRILINEAR);

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

   PLOGI << "Starting script"; // For profiling
   m_progressDialog.SetProgress("Starting Game Scripts..."s);

   m_ptable->m_pcv->Start(); // Hook up to events and start cranking script

   // Fire Init event for table object and all 'hitable' parts, also fire Animate event of parts having it since initial setup is considered as the initial animation event
   m_ptable->FireVoidEvent(DISPID_GameEvents_Init);
   for (Hitable *const ph : m_vhitables)
   {
      if (ph->GetEventProxyBase())
      {
         ph->GetEventProxyBase()->FireVoidEvent(DISPID_GameEvents_Init);
         ItemTypeEnum type = ph->HitableGetItemType();
         if (type == ItemTypeEnum::eItemBumper || type == ItemTypeEnum::eItemDispReel || type == ItemTypeEnum::eItemFlipper || type == ItemTypeEnum::eItemGate
            || type == ItemTypeEnum::eItemHitTarget || type == ItemTypeEnum::eItemLight || type == ItemTypeEnum::eItemSpinner || type == ItemTypeEnum::eItemTrigger)
            ph->GetEventProxyBase()->FireVoidEvent(DISPID_AnimateEvents_Animate);
      }
   }
   m_ptable->FireKeyEvent(DISPID_GameEvents_OptionEvent, 0 /* custom option init event */); 
   m_ptable->FireVoidEvent(DISPID_GameEvents_Paused);

   // Initialize stereo rendering
   m_renderer->UpdateStereoShaderState();

   ReadAccelerometerCalibration();

#ifdef PLAYBACK
   if (m_playback)
      m_fplaylog = fopen("c:\\badlog.txt", "r");
#endif

   // We need to initialize the perf counter before creating the UI which uses it
   wintimer_init();
   m_liveUI = new LiveUI(m_renderer->m_pd3dPrimaryDevice);

   // Signal plugins before performing static prerendering. The only thing not fully initialized is the physics (is this ok ?)
   m_onPrepareFrameEventId = PluginManager::GetEventID(VPX_EVT_ON_PREPARE_FRAME);
   PluginManager::BroadcastEvent(PluginManager::GetEventID(VPX_EVT_ON_GAME_START), nullptr);

   // Open UI if requested (this also disables static prerendering, so must be done before performing it)
   if (playMode == 1)
      m_liveUI->OpenTweakMode();
   else if (playMode == 2 && m_renderer->m_stereo3D != STEREO_VR)
      m_liveUI->OpenLiveUI();

   // Pre-render all non-changing elements such as static walls, rails, backdrops, etc. and also static playfield reflections
   // This is done after starting the script and firing the Init event to allow script to adjust static parts on startup
   PLOGI << "Prerendering static parts"; // For profiling
   m_renderer->RenderStaticPrepass();

   // Reset the perf counter to start time when physics starts
   wintimer_init();
   m_physics->StartPhysics();

   m_progressDialog.SetProgress("Starting..."s, 100);
   m_ptable->FireVoidEvent(DISPID_GameEvents_UnPaused);

#ifdef __STANDALONE__
   if (g_pvp->m_settings.LoadValueWithDefault(Settings::Standalone, "WebServer"s, false))
      g_pvp->m_webServer.Start();
#endif

   PLOGI << "Startup done"; // For profiling

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
   m_playfieldWnd->ShowAndFocus();

   // Popup notification on startup
   if (m_renderer->m_stereo3D != STEREO_OFF && m_renderer->m_stereo3D != STEREO_VR && !m_renderer->m_stereo3Denabled)
      m_liveUI->PushNotification("3D Stereo is enabled but currently toggled off, press F10 to toggle 3D Stereo on"s, 4000);
   const int numberOfTimesToShowTouchMessage = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "NumberOfTimesToShowTouchMessage"s, 10);
   if (m_supportsTouch && numberOfTimesToShowTouchMessage != 0) //!! visualize with real buttons or at least the areas?? Add extra buttons?
   {
      g_pvp->m_settings.SaveValue(Settings::Player, "NumberOfTimesToShowTouchMessage"s, max(numberOfTimesToShowTouchMessage - 1, 0));
      m_liveUI->PushNotification("You can use Touch controls on this display: bottom left area to Start Game, bottom right area to use the Plunger\n"
                                 "lower left/right for Flippers, upper left/right for Magna buttons, top left for Credits and (hold) top right to Exit"s, 12000);
   }
}

Player::~Player()
{
   assert(g_pplayer == this && g_pplayer->m_closing != CS_CLOSED);
   if (g_pplayer == nullptr || g_pplayer->m_closing == CS_CLOSED)
   {
      PLOGE << "Player::OnClose discarded since player is already closing (destructor called from 2 different places...)";
      return;
   }
   m_closing = CS_CLOSED;
   PLOGI << "Closing player...";

   // Signal plugins early since most fields will become invalid
   PluginManager::BroadcastEvent(PluginManager::GetEventID(VPX_EVT_ON_GAME_END), nullptr);

   // signal the script that the game is now exited to allow any cleanup
   m_ptable->FireVoidEvent(DISPID_GameEvents_Exit);
   if (m_detectScriptHang)
      g_pvp->PostWorkToWorkerThread(HANG_SNOOP_STOP, NULL);

   // Stop script engine before destroying objects
   m_ptable->m_pcv->CleanUpScriptEngine();

   g_frameProfiler.LogWorstFrame();

   // Save list of used textures to avoid stuttering in next play
   if ((m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "CacheMode"s, 1) > 0) && FileExists(m_ptable->m_szFileName))
   {
      string dir = g_pvp->m_szMyPrefPath + "Cache" + PATH_SEPARATOR_CHAR + m_ptable->m_szTitle + PATH_SEPARATOR_CHAR;
      std::filesystem::create_directories(std::filesystem::path(dir));

      std::map<string, bool> prevPreRenderOnly;
      if (!m_renderer->IsUsingStaticPrepass() && FileExists(dir + "used_textures.xml"))
      {
         std::ifstream myFile(dir + "used_textures.xml");
         std::stringstream buffer;
         buffer << myFile.rdbuf();
         myFile.close();
         auto xml = buffer.str();
         tinyxml2::XMLDocument xmlDoc;
         if (xmlDoc.Parse(xml.c_str()) == tinyxml2::XML_SUCCESS)
         {
            auto root = xmlDoc.FirstChildElement("textures");
            for (auto node = root->FirstChildElement("texture"); node != nullptr; node = node->NextSiblingElement())
            {
               bool preRenderOnly = false;
               const char *name = node->GetText();
               if (node->QueryBoolAttribute("prerender", &preRenderOnly) == tinyxml2::XML_SUCCESS)
                  prevPreRenderOnly[name] = preRenderOnly;
            }
         }
      }

      tinyxml2::XMLDocument xmlDoc;
      tinyxml2::XMLElement *root = xmlDoc.NewElement("textures");
      xmlDoc.InsertEndChild(xmlDoc.NewDeclaration());
      xmlDoc.InsertEndChild(root);
      vector<BaseTexture *> textures = m_renderer->m_pd3dPrimaryDevice->m_texMan.GetLoadedTextures();
      for (BaseTexture *memtex : textures)
      {
         for (Texture *image : m_ptable->m_vimage)
         {
            if (image->m_pdsBuffer == memtex)
            {
               tinyxml2::XMLElement *node = xmlDoc.NewElement("texture");
               node->SetText(image->m_szName.c_str());
               node->SetAttribute("filter", (int)m_renderer->m_pd3dPrimaryDevice->m_texMan.GetFilter(memtex));
               node->SetAttribute("clampu", (int)m_renderer->m_pd3dPrimaryDevice->m_texMan.GetClampU(memtex));
               node->SetAttribute("clampv", (int)m_renderer->m_pd3dPrimaryDevice->m_texMan.GetClampV(memtex));
               node->SetAttribute("linear", m_renderer->m_pd3dPrimaryDevice->m_texMan.IsLinearRGB(memtex));
               bool preRenderOnly = !m_renderer->IsUsingStaticPrepass() ? (prevPreRenderOnly.find(image->m_szName) != prevPreRenderOnly.end() ? prevPreRenderOnly[image->m_szName] : true)
                                                                        : m_renderer->m_pd3dPrimaryDevice->m_texMan.IsPreRenderOnly(memtex);
               node->SetAttribute("prerender", preRenderOnly);
               root->InsertEndChild(node);
               break;
            }
         }
      }
      tinyxml2::XMLPrinter prn;
      xmlDoc.Print(&prn);

      std::ofstream myfile(dir + "used_textures.xml");
      myfile << prn.CStr();
      myfile.close();
   }

   // Save adjusted VR settings
   if (m_renderer->m_stereo3D == STEREO_VR)
      m_vrDevice->SaveVRSettings(g_pvp->m_settings);

   if (m_audio)
      m_audio->MusicPause();
   delete m_audio;

   mixer_shutdown();
   hid_shutdown();

#ifdef EXT_CAPTURE
   StopCaptures();
   g_DXGIRegistry.ReleaseAll();
#endif

   delete m_liveUI;
   m_pininput.UnInit();
   delete m_physics;

   for (auto probe : m_ptable->m_vrenderprobe)
      probe->RenderRelease();
   for (auto renderable : m_vhitables)
      renderable->RenderRelease();
   for (auto ball : m_vball)
      ball->m_pballex->RenderRelease();
   for (auto hitable : m_vhitables)
      hitable->EndPlay();

   if (m_implicitPlayfieldMesh)
   {
      RemoveFromVectorSingle(m_ptable->m_vedit, (IEditable *)m_implicitPlayfieldMesh);
      m_ptable->m_pcv->RemoveItem(m_implicitPlayfieldMesh->GetScriptable());
      delete m_implicitPlayfieldMesh;
      m_implicitPlayfieldMesh = nullptr;
   }

   if (m_texdmd)
   {
      m_renderer->m_pd3dPrimaryDevice->m_DMDShader->SetTextureNull(SHADER_tex_dmd);
      m_renderer->m_pd3dPrimaryDevice->m_texMan.UnloadTexture(m_texdmd);
      delete m_texdmd;
      m_texdmd = nullptr;
   }

#ifdef PLAYBACK
   if (m_fplaylog)
      fclose(m_fplaylog);
#endif

   for (size_t i = 0; i < m_controlclsidsafe.size(); i++)
      delete m_controlclsidsafe[i];
   m_controlclsidsafe.clear();

   m_changed_vht.clear();

   //!! cleanup the whole mem management for balls, this is a mess!
   // balls are added to the octree, but not the hit object vector
   for (size_t i = 0; i < m_vball.size(); i++)
   {
      Ball *const pball = m_vball[i];
      if (pball->m_pballex)
      {
         pball->m_pballex->m_pball = nullptr;
         pball->m_pballex->Release();
      }
      delete pball->m_d.m_vpVolObjs;
      delete pball;
   }
   //!! see above
   //for (size_t i=0;i<m_vho_dynamic.size();i++)
   //      delete m_vho_dynamic[i];
   //m_vho_dynamic.clear();
   m_vball.clear();

   delete m_pBCTarget;
   delete m_ptable;
   delete m_renderer;
   delete m_playfieldWnd;

   g_pplayer = nullptr;

   restore_win_timer_resolution();

   LockForegroundWindow(false);
   while (ShowCursor(FALSE) >= 0);
   while (ShowCursor(TRUE) < 0);

#ifndef __STANDALONE__
   if (m_progressDialog.IsWindow())
      m_progressDialog.Destroy();

   // Reactivate edited table or close application if requested
   if (m_closing == CS_CLOSE_APP)
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

   PLOGI << "Player closed.";
}

void Player::InitFPS()
{
   m_fps = 0.f;
   m_lastMaxChangeTime = 0;
   m_script_max = 0;
   m_physics->ResetStats();
   g_frameProfiler.Reset();
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
      string newFocusedWnd = "undefined"s;
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
                  newFocusedWnd = szFileName;
            }
         }
      }
      PLOGI << "Focus lost (new focused window: " << newFocusedWnd << ')';
      #else
      PLOGI << "Focus lost";
      #endif
   }
   const bool wasPlaying = IsPlaying();
   const bool willPlay = m_playing && isGameFocused;
   if (wasPlaying != willPlay)
   {
      ApplyPlayingState(willPlay);
      m_focused = isGameFocused;
   }

   #ifdef _MSC_VER
   // FIXME Hacky handling of auxiliary windows (B2S, DMD, Pup,...) stealing focus under Windows: keep focused during first 5 seconds
   if (!isGameFocused && m_time_msec < 5000)
   {
      #ifdef ENABLE_SDL_VIDEO
      SDL_RaiseWindow(m_playfieldWnd->GetCore());
      #elif defined(_MSC_VER)
      SetForegroundWindow(m_playfieldWnd->GetCore());
      #endif
   }
   #endif
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
      m_ptable->FireVoidEvent(DISPID_GameEvents_UnPaused); // signal the script that the game is now running again
   }
   else
   {
      PauseMusic();
      PLOGI << "Pausing Game";
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

Ball *Player::CreateBall(const float x, const float y, const float z, const float vx, const float vy, const float vz, const float radius, const float mass)
{
   Ball * const pball = new Ball();
   pball->m_d.m_radius = radius;
   pball->m_d.m_pos.x = x;
   pball->m_d.m_pos.y = y;
   pball->m_d.m_pos.z = z + pball->m_d.m_radius;
   pball->m_d.m_vel.x = vx;
   pball->m_d.m_vel.y = vy;
   pball->m_d.m_vel.z = vz;
   pball->m_bulb_intensity_scale = m_ptable->m_defaultBulbIntensityScaleOnBall;

   pball->Init(mass); // Call this after radius set to get proper inertial tensor set up

   CComObject<BallEx>::CreateInstance(&pball->m_pballex);
   pball->m_pballex->AddRef();
   pball->m_pballex->m_pball = pball;
   pball->m_pballex->RenderSetup(m_renderer->m_pd3dPrimaryDevice);

   // FIXME set m_editable when balls will be editable parts
   //pball->m_editable = pball->m_pballex;

   pball->CalcHitBBox(); // need to update here, as only done lazily

   if (!m_pactiveballDebug)
      m_pactiveballDebug = pball;

   m_vball.push_back(pball);

   m_physics->AddBall(pball);

   return pball;
}

void Player::DestroyBall(Ball *pball)
{
   if (!pball) return;

   RemoveFromVectorSingle(m_vball, pball);
   m_vballDelete.push_back(pball);
   m_physics->RemoveBall(pball);

   if (m_pactiveball == pball)
      m_pactiveball = m_vball.empty() ? nullptr : m_vball.front();
   if (m_pactiveballDebug == pball)
      m_pactiveballDebug = m_vball.empty() ? nullptr : m_vball.front();
   if (m_pactiveballBC == pball)
      m_pactiveballBC = nullptr;
}


void Player::FireSyncController()
{
   // Legacy implementation: timers with magic interval value have special behaviors: -2 for controller sync event
   for (HitTimer *const pht : m_vht)
      if (pht->m_interval == -2)
      {
         g_frameProfiler.EnterScriptSection(DISPID_TimerEvents_Timer, pht->m_name); 
         pht->m_pfe->FireGroupEvent(DISPID_TimerEvents_Timer);
         g_frameProfiler.ExitScriptSection(pht->m_name);
      }
}

void Player::FireTimers(const unsigned int simulationTime)
{
   Ball *const old_pactiveball = g_pplayer->m_pactiveball;
   g_pplayer->m_pactiveball = nullptr; // No ball is the active ball for timers/key events
   for (HitTimer *const pht : m_vht)
   {
      if (pht->m_interval >= 0 && pht->m_nextfire <= simulationTime)
      {
         const unsigned int curnextfire = pht->m_nextfire;
         g_frameProfiler.EnterScriptSection(DISPID_TimerEvents_Timer, pht->m_name);
         pht->m_pfe->FireGroupEvent(DISPID_TimerEvents_Timer);
         g_frameProfiler.ExitScriptSection(pht->m_name);
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

void Player::ReadAccelerometerCalibration()
{
   m_accelerometerEnabled = m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "PBWEnabled"s, true); // true if electronic accelerometer enabled
   m_accelerometerFaceUp = m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "PBWNormalMount"s, true); // true is normal mounting (left hand coordinates)

   m_accelerometerAngle = 0.0f; // 0 degrees rotated counterclockwise (GUI is lefthand coordinates)
   const bool accel = m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "PBWRotationCB"s, false);
   if (accel)
      m_accelerometerAngle = (float)m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "PBWRotationValue"s, 0);

   m_accelerometerSensitivity = clamp((float)m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "NudgeSensitivity"s, 500) * (float)(1.0/1000.0), 0.f, 1.f);

   m_accelerometerMax.x = m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "PBWAccelMaxX"s, 100) * JOYRANGEMX / 100;
   m_accelerometerMax.y = m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "PBWAccelMaxY"s, 100) * JOYRANGEMX / 100;
   m_accelerometerGain.x = dequantizeUnsignedPercentNoClamp(m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "PBWAccelGainX"s, 150));
   m_accelerometerGain.y = dequantizeUnsignedPercentNoClamp(m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "PBWAccelGainY"s, 150));

   m_pininput.LoadSettings(m_ptable->m_settings);
}

void Player::SetNudgeX(const int x, const int joyidx)
{
   m_accelerometerDirty |= m_accelerometerEnabled;
   m_curAccel[joyidx].x = clamp(x, -m_accelerometerMax.x, m_accelerometerMax.x);
}

void Player::SetNudgeY(const int y, const int joyidx)
{
   m_accelerometerDirty |= m_accelerometerEnabled;
   m_curAccel[joyidx].y = clamp(y, -m_accelerometerMax.y, m_accelerometerMax.y);
}

const Vertex2D& Player::GetRawAccelerometer() const
{
   if (m_accelerometerDirty)
   {
      m_accelerometerDirty = false;
      m_accelerometer = Vertex2D(0.f, 0.f); 
      if (m_accelerometerEnabled)
      {
         // accumulate over joysticks, these acceleration values are used in update ball velocity calculations
         // and are required to be acceleration values (not velocity or displacement)

         // rotate to match hardware mounting orentation, including left or right coordinates
         const float a = ANGTORAD(m_accelerometerAngle);
         const float cna = cosf(a);
         const float sna = sinf(a);

         for (int j = 0; j < m_pininput.m_num_joy; ++j)
         {
                  float dx = ((float)m_curAccel[j].x)*(float)(1.0 / JOYRANGE); // norm range -1 .. 1
            const float dy = ((float)m_curAccel[j].y)*(float)(1.0 / JOYRANGE);
            if (m_ptable->m_tblMirrorEnabled)
               dx = -dx;
            m_accelerometer.x += m_accelerometerGain.x * (dx * cna + dy * sna) * (1.0f - m_accelerometerSensitivity); // calc Green's transform component for X
            const float nugY   = m_accelerometerGain.y * (dy * cna - dx * sna) * (1.0f - m_accelerometerSensitivity); // calc Green's transform component for Y
            m_accelerometer.y += m_accelerometerFaceUp ? nugY : -nugY; // add as left or right hand coordinate system
         }
      }
   }
   return m_accelerometer;
}

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

void Player::MechPlungerUpdate()   // called on every integral physics frame, only really triggered if before MechPlungerIn() was called, which again relies on USHOCKTYPE_GENERIC,USHOCKTYPE_ULTRACADE,USHOCKTYPE_PBWIZARD,USHOCKTYPE_VIRTUAPIN,USHOCKTYPE_SIDEWINDER being used
{
   static int init = IIR_Order;    // first time call
   static float x[IIR_Order + 1] = { 0, 0, 0, 0, 0 };
   static float y[IIR_Order + 1] = { 0, 0, 0, 0, 0 };

   //http://www.dsptutor.freeuk.com/IIRFilterDesign/IIRFilterDesign.html  
   // (this applet is set to 8000Hz sample rate, therefore, multiply ...
   // our values by 80 to shift sample clock of 100hz to 8000hz)

   if (m_movedPlunger < 3)
   {
      init = IIR_Order;
      m_curMechPlungerPos = 0;
      return; // not until a real value is entered
   }

   if (!m_ptable->m_plungerFilter)
   {
      m_curMechPlungerPos = (float)m_curPlunger;
      return;
   }

   x[0] = (float)m_curPlunger; //initialize filter
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

// MechPlunger NOTE: Normalized position is from 0.0 to +1.0f
// +1.0 is fully retracted, 0.0 is all the way forward.
//
// The traditional method requires calibration in control panel game controllers to work right.
// The calibrated zero value should match the rest position of the mechanical plunger.
// The method below uses a dual - piecewise linear function to map the mechanical pull and push 
// onto the virtual plunger position from 0..1, the pulunger properties has a ParkPosition setting 
// that matches the mechanical plunger zero position
//
// If the plunger device is a "linear plunger", we replace that calculation with a single linear
// scaling factor that applies on both sides of the park position.  This eliminates the need for
// separate calibration on each side of the park position, which seems to produce more consistent
// and linear behavior.  The Pinscape Controller plunger uses this method.
float PlungerMoverObject::MechPlunger() const
{
   if (g_pplayer->m_pininput.m_linearPlunger)
   {
      // Linear plunger device - the joystick must be calibrated such that the park
      // position reads as 0 and the fully retracted position reads as JOYRANGEMX.  The
      // scaling factor between physical units and joystick units must be the same on the
      // positive and negative sides.  (The maximum forward position is not calibrated.)
      const float m = (1.0f - m_restPos)*(float)(1.0 / JOYRANGEMX), b = m_restPos;
      return m * g_pplayer->m_curMechPlungerPos + b;
   }
   else
   {
      // Standard plunger device - the joystick must be calibrated such that the park
      // position reads as 0, the fully retracted position reads as JOYRANGEMN, and the
      // full forward position reads as JOYRANGMN.
      const float range = (float)JOYRANGEMX * (1.0f - m_restPos) - (float)JOYRANGEMN *m_restPos; // final range limit
      const float tmp = (g_pplayer->m_curMechPlungerPos < 0) ? g_pplayer->m_curMechPlungerPos * m_restPos : g_pplayer->m_curMechPlungerPos * (1.0f - m_restPos);
      return tmp / range + m_restPos;              //scale and offset
   }
}

void Player::MechPlungerIn(const int z)
{
   m_curPlunger = -z; //axis reversal

   if (++m_movedPlunger == 0xffffffff) m_movedPlunger = 3; //restart at 3
}

//++++++++++++++++++++++++++++++++++++++++

#pragma endregion


string Player::GetPerfInfo()
{
   // Make it more or less readable by updating only once per second
   static string txt;
   static U32 lastUpdate = -1;
   U32 now = msec();
   if (lastUpdate != -1 && now - lastUpdate < 1000)
      return txt;

   lastUpdate = now;
   std::ostringstream info;
   info << std::fixed << std::setprecision(1);
   
   const bool resetMax = m_time_msec - m_lastMaxChangeTime > 1000;

   if (g_frameProfiler.GetPrev(FrameProfiler::PROFILE_SCRIPT) > m_script_max || resetMax)
      m_script_max = g_frameProfiler.GetPrev(FrameProfiler::PROFILE_SCRIPT);

   if (resetMax)
      m_lastMaxChangeTime = m_time_msec;

   // Renderer additional information
   info << "Triangles: " << ((m_renderer->m_pd3dPrimaryDevice->m_frameDrawnTriangles + 999) / 1000) << "k per frame, "
        << ((m_renderer->GetNPrerenderTris() + m_renderer->m_pd3dPrimaryDevice->m_frameDrawnTriangles + 999) / 1000) << "k overall. DayNight " << quantizeUnsignedPercent(m_renderer->m_globalEmissionScale)
        << "%%\n";
   info << "Draw calls: " << m_renderer->m_pd3dPrimaryDevice->Perf_GetNumDrawCalls() << "  (" << m_renderer->m_pd3dPrimaryDevice->Perf_GetNumLockCalls() << " Locks)\n";
   info << "State changes: " << m_renderer->m_pd3dPrimaryDevice->Perf_GetNumStateChanges() << "\n";
   info << "Texture changes: " << m_renderer->m_pd3dPrimaryDevice->Perf_GetNumTextureChanges() << " (" << m_renderer->m_pd3dPrimaryDevice->Perf_GetNumTextureUploads() << " Uploads)\n";
   info << "Shader/Parameter changes: " << m_renderer->m_pd3dPrimaryDevice->Perf_GetNumTechniqueChanges() << " / " << m_renderer->m_pd3dPrimaryDevice->Perf_GetNumParameterChanges() << "\n";
   info << "Objects: " << (unsigned int)m_vhitables.size() << "\n";
   info << "\n";

   // Physics additional information
   info << m_physics->GetPerfInfo(resetMax);
   info << "Ball Velocity / Ang.Vel.: " << (m_pactiveball ? (m_pactiveball->m_d.m_vel + (float)PHYS_FACTOR * m_physics->GetGravity()).Length() : -1.f) << " "
        << (m_pactiveball ? (m_pactiveball->m_angularmomentum / m_pactiveball->Inertia()).Length() : -1.f) << "\n";

   info << "Left Flipper keypress to rotate: "
      << ((INT64)(m_pininput.m_leftkey_down_usec_rotate_to_end - m_pininput.m_leftkey_down_usec) < 0 ? int_as_float(0x7FC00000) : (double)(m_pininput.m_leftkey_down_usec_rotate_to_end - m_pininput.m_leftkey_down_usec) / 1000.) << " ms ("
      << ((int)(m_pininput.m_leftkey_down_frame_rotate_to_end - m_pininput.m_leftkey_down_frame) < 0 ? -1 : (int)(m_pininput.m_leftkey_down_frame_rotate_to_end - m_pininput.m_leftkey_down_frame)) << " f) to eos: "
      << ((INT64)(m_pininput.m_leftkey_down_usec_EOS - m_pininput.m_leftkey_down_usec) < 0 ? int_as_float(0x7FC00000) : (double)(m_pininput.m_leftkey_down_usec_EOS - m_pininput.m_leftkey_down_usec) / 1000.) << " ms ("
      << ((int)(m_pininput.m_leftkey_down_frame_EOS - m_pininput.m_leftkey_down_frame) < 0 ? -1 : (int)(m_pininput.m_leftkey_down_frame_EOS - m_pininput.m_leftkey_down_frame)) << " f)\n";

   // Draw performance readout - at end of CPU frame, so hopefully the previous frame
   //  (whose data we're getting) will have finished on the GPU by now.
   #if defined(ENABLE_DX9) // No GPU profiler for OpenGL / BGFX
   if (GetProfilingMode() != PF_DISABLED && m_closing == CS_PLAYING)
   {
      info << "\n";
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
            info << " " << GTS_name_item[gts] << ": " << float(1000.0 * m_renderer->m_gpu_profiler.DtAvg(gts)) << " ms (" << float(100. * m_renderer->m_gpu_profiler.DtAvg(gts) / dTDrawTotal)
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
#if(_WIN32_WINNT >= 0x0500)
    if (m_fullScreen) // revert special tweaks of exclusive fullscreen app
       ::LockSetForegroundWindow(enable ? LSFW_LOCK : LSFW_UNLOCK);
#else
#pragma message ( "Warning: Missing LockSetForegroundWindow()" )
#endif
#endif
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// MSVC Concurrency Viewer support
// This require to WINVER >= 0x0600 and to add the MSVC Concurrendy SDK to the project
//#define MSVC_CONCURRENCY_VIEWER
#ifdef MSVC_CONCURRENCY_VIEWER
#include <cvmarkersobj.h>
using namespace Concurrency::diagnostic;
marker_series series;
#endif

#ifdef _MSC_VER
// Sadly Windows does not offer a microsecond precise sleep function like unix does
// using uSleep or Sleep results in bad precision and/or high CPU use.
// Taken from https://www.c-plusplus.net/forum/topic/109539/usleep-unter-windows
void usleep(unsigned int usec)
{
   HANDLE timer = CreateWaitableTimer(NULL, TRUE, NULL);
   if (timer)
   {
      LARGE_INTEGER ft;
      ft.QuadPart = -(10 * (__int64)usec); // microseconds to 100 nanoseconds intervals
      SetWaitableTimer(timer, &ft, 0, NULL, NULL, 0);
      WaitForSingleObject(timer, INFINITE);
      CloseHandle(timer);
   }
   else Sleep(0);
}
#else
#include <unistd.h>
#endif

void Player::GameLoop(std::function<void()> ProcessOSMessages)
{
   assert(m_renderer->m_stereo3D != STEREO_VR || (m_videoSyncMode == VideoSyncMode::VSM_NONE && m_maxFramerate > 1000)); // Stereo must be run unthrottled to let OpenVR set the frame pace according to the head set

   int syncLengthPos = 0;
   memset(m_syncLengths, 0, sizeof(m_syncLengths));
   auto sync = [this, ProcessOSMessages, &syncLengthPos](bool updateTimings)
   {
      // Controller sync
      #ifdef MSVC_CONCURRENCY_VIEWER
      //series.write_flag(_T("Sync"));
      span *tagSpan = new span(series, 1, _T("Sync"));
      #endif
      unsigned long long startTick = usec();
      ProcessOSMessages();
      m_pininput.ProcessKeys(/*sim_msec,*/ -(int)(m_startFrameTick / 1000)); // Trigger key events to sync with controller
      m_physics->UpdatePhysics(); // Update physics (also trigerring events, syncing with controller)
      FireSyncController(); // Trigger script sync event (to sync solenoids back)
      m_syncLengths[syncLengthPos] = (unsigned int)(usec() - startTick);
      syncLengthPos = (syncLengthPos + 1) % (sizeof(m_syncLengths) / sizeof(m_syncLengths[0]));
      #ifdef MSVC_CONCURRENCY_VIEWER
      delete tagSpan;
      #endif
   };

   #ifdef ENABLE_BGFX
   MultithreadedGameLoop(sync);
   
   #else
   if (m_videoSyncMode == VideoSyncMode::VSM_FRAME_PACING)
      FramePacingGameLoop(sync);
   else
      GPUQueueStuffingGameLoop(sync);
   #endif
}

void Player::MultithreadedGameLoop(std::function<void(bool)> sync)
{
   while (GetCloseState() == CS_PLAYING || GetCloseState() == CS_USER_INPUT)
   {
      #ifdef MSVC_CONCURRENCY_VIEWER
      series.write_flag(_T("Frame"));
      #endif
      unsigned long long syncStopTimestamp = usec();
      g_frameProfiler.NewFrame(m_time_msec);
      m_overall_frames++; // This causes the next VPinMame <-> VPX sync to update light status which can be heavy since it needs to perform PWM integration of all lights

      // Process OS messages, input, physics and timers including synchronization with game controller like PinMame
      sync(false);

      // Prepare render frame
      PrepareFrame();

      sync(true);

      // Submit render frame to backend
      SubmitFrame();

      sync(true);

      // performs some syncs to limit IO latency and improve syncing between emulation/input/physics
      unsigned int maxSyncLength = 0;
      for (int i = 0; i < 512; i++)
         maxSyncLength = maxSyncLength < m_syncLengths[i] ? m_syncLengths[i] : maxSyncLength;
      int delta = (1000000 / m_maxFramerate) - 600 - maxSyncLength;
      m_lastFrameSyncOnVBlank = delta > 0;
      if (m_lastFrameSyncOnVBlank)
      {
         syncStopTimestamp += delta;
         while (usec() < syncStopTimestamp)
         {
            usleep(100);
            sync(true);
         }
      }

      // Schedule present
      #ifdef MSVC_CONCURRENCY_VIEWER
      span* tagSpan = new span(series, 1, _T("Flip"));
      #endif
      m_renderer->m_pd3dPrimaryDevice->Flip();
      FinishFrame();
      #ifdef MSVC_CONCURRENCY_VIEWER
      delete tagSpan;
      #endif
   }
}

void Player::GPUQueueStuffingGameLoop(std::function<void(bool)> sync)
{
   // Legacy main loop performs the frame as a single block. This leads to having the input <-> physics stall between frames increasing
   // the latency and causing syncing problems with PinMAME (which runs in realtime and expects realtime inputs, especially for video modes
   // with repeated button presses like Black Rose's "Walk the Plank Video Mode" or Lethal Weapon 3's "Battle Video Mode")
   // This also leads to filling up the GPU render queue leading to a few frame latency, depending on driver setup
   while (GetCloseState() == CS_PLAYING || GetCloseState() == CS_USER_INPUT)
   {
      #ifdef MSVC_CONCURRENCY_VIEWER
      series.write_flag(_T("Frame"));
      #endif

      // Collect stats from previous frame and starts profiling a new frame
      g_frameProfiler.NewFrame(m_time_msec);
      m_overall_frames++;

      sync(false);

      PrepareFrame();

      sync(false);

      SubmitFrame();

      sync(false);

      // Present & VSync
      #ifdef MSVC_CONCURRENCY_VIEWER
      span* tagSpan = new span(series, 1, _T("Flip"));
      #endif
      g_frameProfiler.EnterProfileSection(FrameProfiler::PROFILE_GPU_FLIP);
      m_renderer->m_pd3dPrimaryDevice->Flip();
      #if defined(ENABLE_DX9) // DirectX 9 does not support native adaptive sync, so we must emulate it at the application level
      if (m_videoSyncMode == VideoSyncMode::VSM_ADAPTIVE_VSYNC && m_fps > m_maxFramerate * ADAPT_VSYNC_FACTOR)
         m_renderer->m_pd3dPrimaryDevice->WaitForVSync(false);
      #endif
      g_frameProfiler.ExitProfileSection();

      FinishFrame();

      // Adjust framerate if requested by user (i.e. not using a synchronization mode that will lead to blocking calls aligned to the display refresh rate)
      if (m_videoSyncMode == VideoSyncMode::VSM_NONE || m_maxFramerate < m_playfieldWnd->GetRefreshRate()) // The synchronization is not already performed by VSYNC
      {
         const int timeForFrame = (int)(usec() - m_startFrameTick);
         const int targetTime = 1000000 / m_maxFramerate;
         if (timeForFrame < targetTime)
         {
            g_frameProfiler.EnterProfileSection(FrameProfiler::PROFILE_SLEEP);
            uSleep(targetTime - timeForFrame);
            g_frameProfiler.ExitProfileSection();
         }
      }

      #ifdef MSVC_CONCURRENCY_VIEWER
      delete tagSpan;
      #endif
   }
}

void Player::FramePacingGameLoop(std::function<void(bool)> sync)
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

      g_frameProfiler.NewFrame(m_time_msec);
      m_overall_frames++;

      sync(false);

      PLOGI_IF(debugLog) << "Frame Collect [Last frame length: " << ((double)g_frameProfiler.GetPrev(FrameProfiler::PROFILE_FRAME) / 1000.0) << "ms] at " << usec();
      PrepareFrame();

      sync(false);

      PLOGI_IF(debugLog) << "Frame Submit at " << usec();
      SubmitFrame();
      g_frameProfiler.EnterProfileSection(FrameProfiler::PROFILE_SLEEP);

      // Wait for at least one VBlank after last frame submission (adaptive sync)
      while (m_renderer->m_pd3dPrimaryDevice->m_vsyncCount == 0)
      {
         m_curFrameSyncOnVBlank = true;
         usleep(100);
         sync(false);
      }

      // If the user asked to sync on a lower frame rate than the refresh rate, then wait for it
      if (m_maxFramerate != m_playfieldWnd->GetRefreshRate())
      {
         const U64 now = usec();
         const int refreshLength = (int)(1000000ul / m_playfieldWnd->GetRefreshRate());
         const int minimumFrameLength = 1000000ull / m_maxFramerate;
         const int maximumFrameLength = 5 * refreshLength;
         const int targetFrameLength = clamp(refreshLength - 2000, min(minimumFrameLength, maximumFrameLength), maximumFrameLength);
         while (now < m_lastPresentFrameTick + targetFrameLength)
         {
            m_curFrameSyncOnFPS = true;
            usleep(100);
            sync(false);
         }
         m_lastPresentFrameTick = now;
      }

      // Schedule frame presentation, ask for an asynchronous VBlank, start preparing next frame
      #ifdef MSVC_CONCURRENCY_VIEWER
      span* tagSpan = new span(series, 1, _T("Flip"));
      #endif
      m_lastFrameSyncOnVBlank = m_curFrameSyncOnVBlank;
      m_lastFrameSyncOnFPS = m_curFrameSyncOnFPS;
      PLOGI_IF(debugLog) << "Frame Scheduled at " << usec() << ", Waited for VBlank: " << m_curFrameSyncOnVBlank << ", Waited for FPS: " << m_curFrameSyncOnFPS;
      m_renderer->m_pd3dPrimaryDevice->m_vsyncCount = 0;
      g_frameProfiler.ExitProfileSection(); // Out of Sleep section
      g_frameProfiler.EnterProfileSection(FrameProfiler::PROFILE_GPU_FLIP);
      m_renderer->m_pd3dPrimaryDevice->Flip();
      m_renderer->m_pd3dPrimaryDevice->WaitForVSync(true);
      g_frameProfiler.ExitProfileSection();
      FinishFrame();
      m_curFrameSyncOnVBlank = m_curFrameSyncOnFPS = false;
      #ifdef MSVC_CONCURRENCY_VIEWER
      delete tagSpan;
      #endif
   }
}

void Player::PrepareFrame()
{
   // Rendering outputs to m_pd3dPrimaryDevice->GetBackBufferTexture(). If MSAA is used, it is resolved as part of the rendering (i.e. this surface is NOT the MSAA rneder surface but its resolved copy)
   // Then it is tonemapped/bloom/dither/... to m_pd3dPrimaryDevice->GetPostProcessRenderTarget1() if needed for postprocessing (sharpen, FXAA,...), or directly to the main output framebuffer otherwise
   // The optional postprocessing is done from m_pd3dPrimaryDevice->GetPostProcessRenderTarget1() to the main output framebuffer
   #ifdef MSVC_CONCURRENCY_VIEWER
   span* tagSpan = new span(series, 1, _T("Prepare"));
   #endif

   m_LastKnownGoodCounter++;
   m_startFrameTick = usec();
   g_frameProfiler.OnPrepare();

   PluginManager::BroadcastEvent(m_onPrepareFrameEventId, nullptr);

   m_physics->OnPrepareFrame();

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
         g_frameProfiler.EnterScriptSection(DISPID_TimerEvents_Timer, pht->m_name); 
         pht->m_pfe->FireGroupEvent(DISPID_TimerEvents_Timer);
         g_frameProfiler.ExitScriptSection(pht->m_name);
      }

   #if defined(ENABLE_DX9)
   // Kill the profiler so that it does not affect performance
   if (m_infoMode != IF_PROFILING)
      m_renderer->m_gpu_profiler.Shutdown();
   if (GetProfilingMode() == PF_ENABLED)
      m_renderer->m_gpu_profiler.BeginFrame(m_renderer->m_pd3dPrimaryDevice->GetCoreDevice());
   #endif

   g_frameProfiler.EnterProfileSection(FrameProfiler::PROFILE_GPU_COLLECT);
   
   m_renderer->PrepareFrame();

   // Check if we should turn animate the plunger light.
   hid_set_output(HID_OUTPUT_PLUNGER, ((m_time_msec - m_LastPlungerHit) < 512) && ((m_time_msec & 512) > 0));

   g_frameProfiler.EnterProfileSection(FrameProfiler::PROFILE_MISC);
   if (m_renderer->m_stereo3D != STEREO_VR)
      m_liveUI->Update(m_playfieldWnd->GetBackBuffer());
   else if (m_liveUI->IsTweakMode())
      m_liveUI->Update(m_renderer->GetOffscreenVR(0));
   g_frameProfiler.ExitProfileSection();

   // Shake screne when nudging
   if (m_NudgeShake > 0.0f)
   {
      Vertex2D offset = m_physics->GetScreenNudge();
      m_renderer->SetScreenOffset(m_NudgeShake * offset.x, m_NudgeShake * offset.y);
   }
   else
      m_renderer->SetScreenOffset(0.f, 0.f);

   m_renderer->PrepareVideoBuffers();

   g_frameProfiler.ExitProfileSection();
   #ifdef MSVC_CONCURRENCY_VIEWER
   delete tagSpan;
   #endif
}

void Player::SubmitFrame()
{
   #ifdef MSVC_CONCURRENCY_VIEWER
   span* tagSpan = new span(series, 1, _T("Submit"));
   #endif
   m_renderer->SubmitFrame();

   #ifdef EXT_CAPTURE
   // Trigger captures
   if (m_renderer->m_stereo3D == STEREO_VR)
      UpdateExtCaptures();
   #endif
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
   m_fps = (float) (1e6 / g_frameProfiler.GetSlidingAvg(FrameProfiler::PROFILE_FRAME));

#ifndef ACCURATETIMERS
   ApplyDeferredTimerChanges();
   FireTimers(m_time_msec);
#else
   if (m_videoSyncMode != VideoSyncMode::VSM_FRAME_PACING)
      m_pininput.ProcessKeys(/*sim_msec,*/ -(int)(m_startFrameTick / 1000)); // trigger key events mainly for VPM<->VP rountrip
#endif

   // Detect & fire end of music events
   if (IsPlaying() && m_audio && !m_audio->MusicActive())
   {
      delete m_audio;
      m_audio = nullptr;
      m_ptable->FireVoidEvent(DISPID_GameEvents_MusicDone);
   }

   // Pause after performing a simulation step
   if ((m_pauseTimeTarget > 0) && (m_pauseTimeTarget <= m_time_msec))
      SetPlayState(false);

   // Memory clean up for balls that may have been destroyed from scripts
   for (const Ball *const pball : m_vballDelete)
   {
      pball->m_pballex->RenderRelease();
      pball->m_pballex->m_pball = nullptr;
      pball->m_pballex->Release();
      delete pball->m_d.m_vpVolObjs;
      delete pball;
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
   #if (defined(__APPLE__) && ((defined(TARGET_OS_IOS) && TARGET_OS_IOS) || (defined(TARGET_OS_TV) && TARGET_OS_TV))) || defined(__ANDROID__)
      PLOGE.printf("Runtime error detected. Resetting LaunchTable to default.");
      g_pvp->m_settings.SaveValue(Settings::Standalone, "LaunchTable"s, "assets/exampleTable.vpx");
   #endif
#endif
   }

   // Close requested with user input
   if (m_closing == CS_USER_INPUT)
   {
      m_closing = CS_PLAYING;
      if (g_pvp->m_disable_pause_menu || m_renderer->m_stereo3D == STEREO_VR)
         m_closing = CS_STOP_PLAY;
      else
         m_liveUI->OpenMainSplash();
   }

#ifdef __STANDALONE__
   g_pStandalone->Render();
#endif

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
         #ifdef ENABLE_SDL_VIDEO // SDL Windowing
         SDL_SysWMinfo wmInfo;
         SDL_VERSION(&wmInfo.version);
         SDL_GetWindowWMInfo(m_playfieldWnd->GetCore(), &wmInfo);
         HWND hwnd = wmInfo.info.win.window;
         #else // Win32 Windowing
         HWND hwnd = m_playfieldWnd->GetCore();
         #endif
         m_debuggerDialog.Create(hwnd);
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
      const vector<string> overlaylist = { "MAME"s, "Virtual DMD"s, "pygame"s, "PUPSCREEN1"s, "formDMD"s, "PUPSCREEN5"s, "PUPSCREEN2"s, "Form1"s /* Old B2S */, "B2S Backglass Server"s, "B2S Background"s, "B2S DMD"s };
      for (const string &windowtext : overlaylist)
      {
         HWND hVPMWnd = FindWindow(nullptr, windowtext.c_str());
         if (hVPMWnd == nullptr)
            hVPMWnd = FindWindow(windowtext.c_str(), nullptr);
         if (hVPMWnd != nullptr && ::IsWindowVisible(hVPMWnd))
            ::SetWindowPos(hVPMWnd, HWND_TOPMOST, 0, 0, 0, 0, (SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW | SWP_NOACTIVATE)); // in some strange cases the VPinMAME window is not on top, so enforce it
      }
   }
#endif
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
            m_ptable->FireKeyEvent(DISPID_GameEvents_KeyDown, index);
         }
         else // Release
         {
            m_ptable->FireKeyEvent(DISPID_GameEvents_KeyUp, index);
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
