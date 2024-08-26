#include "stdafx.h"

#ifdef ENABLE_SDL
#include <SDL2/SDL_syswm.h>
#endif

#include "BAM/BAMView.h"

#ifdef _MSC_VER
#include "imgui/imgui_impl_win32.h"
#endif

#include <algorithm>
#include <ctime>
#include <fstream>
#include <sstream>
#include <filesystem>
#include "../meshes/ballMesh.h"
#include "renderer/Shader.h"
#include "renderer/Anaglyph.h"
#include "renderer/RenderCommand.h"
#include "typedefs3D.h"
#include "captureExt.h"
#include "../math/bluenoise.h"
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

#if !(_WIN32_WINNT >= 0x0500)
 #define KEYEVENTF_SCANCODE    0x0008
#endif /* _WIN32_WINNT >= 0x0500 */

//
static unsigned int stats_drawn_static_triangles = 0;

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

Player::Player(PinTable *const editor_table, PinTable *const live_table, const int playMode)
   : m_pEditorTable(editor_table)
   , m_ptable(live_table)
   , m_playMode(playMode)
{
   m_pininput.LoadSettings(m_ptable->m_settings);
   m_disableStaticPrepass = playMode != 0;

#if !(defined(_M_IX86) || defined(_M_X64) || defined(_M_AMD64) || defined(__i386__) || defined(__i386) || defined(__i486__) || defined(__i486) || defined(i386) || defined(__ia64__) || defined(__x86_64__))
 #pragma message("Warning: No CPU float ignore denorm implemented")
#else
   {
      init_cpu_detection
      // check for SSE and exit if not available, as some code relies on it by now
      if (detect_no_sse)
      { // No SSE?
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

#ifdef STEPPING
   m_pause = false;
   m_step = false;
#endif

   m_supportsTouch = false;

   m_pseudoPause = false;
   m_pauseRefCount = 0;
   m_noTimeCorrect = false;

   m_render_mask = DEFAULT;

   m_throwBalls = false;
   m_ballControl = false;
   m_pactiveballBC = nullptr;
   m_pBCTarget = nullptr;

#ifdef PLAYBACK
   m_playback = false;
   m_fplaylog = nullptr;
#endif

   for (int i = 0; i < PININ_JOYMXCNT; ++i) 
   {
      m_curAccel[i] = int2(0, 0);
      m_curPlunger[i] = 0;
      m_curPlungerSpeed[i] = 0;
   }

   m_audio = nullptr;
   m_pactiveball = nullptr;

   m_plungerSpeedScale = 1.0f;
   m_curMechPlungerPos = 0;
   m_curMechPlungerSpeed = 0;
   m_fExtPlungerSpeed = false;

#ifdef ENABLE_VR
   const int vrDetectionMode = m_ptable->m_settings.LoadValueWithDefault(Settings::PlayerVR, "AskToTurnOn"s, 0);
   bool useVR = vrDetectionMode == 2 /* VR Disabled */  ? false : RenderDevice::isVRinstalled();
   if (useVR && (vrDetectionMode == 1 /* VR Autodetect => ask to turn on and adapt accordingly */) && !RenderDevice::isVRturnedOn())
      useVR = g_pvp->MessageBox("VR headset detected but SteamVR is not running.\n\nTurn VR on?", "VR Headset Detected", MB_YESNO) == IDYES;
   m_capExtDMD = useVR && m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "CaptureExternalDMD"s, false);
   m_capPUP = useVR && m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "CapturePUP"s, false);
#else
   bool useVR = false;
   m_capExtDMD = false;
   m_capPUP = false;
#endif
   m_vrPreview = (VRPreviewMode)m_ptable->m_settings.LoadValueWithDefault(Settings::PlayerVR, "VRPreview"s, (int)VRPREVIEW_LEFT);
   m_vrPreviewShrink = m_ptable->m_settings.LoadValueWithDefault(Settings::PlayerVR, "ShrinkPreview"s, false);
   m_trailForBalls = m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "BallTrail"s, true);
   m_ballTrailStrength = m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "BallTrailStrength"s, 0.5f);
   m_disableLightingForBalls = m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "DisableLightingForBalls"s, false);
   m_stereo3D = useVR ? STEREO_VR : (StereoMode)m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "Stereo3D"s, (int)STEREO_OFF);
   m_stereo3Denabled = m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "Stereo3DEnabled"s, (m_stereo3D != STEREO_OFF));
   m_disableDWM = m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "DisableDWM"s, false);
   m_useNvidiaApi = m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "UseNVidiaAPI"s, false);
   m_stereo3DfakeStereo = m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "Stereo3DFake"s, false);
   #ifndef ENABLE_SDL // DirectX does not support stereo rendering
   m_stereo3DfakeStereo = true;
   #endif
   m_headTracking = useVR ? false : m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "BAMHeadTracking"s, false);
   m_BWrendering = m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "BWRendering"s, 0);
   m_detectScriptHang = m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "DetectHang"s, false);
   const int maxReflection = m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "PFReflection"s, -1);
   if (maxReflection != -1)
      m_maxReflectionMode = (RenderProbe::ReflectionMode)maxReflection;
   else
   {
      m_maxReflectionMode = RenderProbe::REFL_STATIC;
      if (m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "BallReflection"s, true))
         m_maxReflectionMode = RenderProbe::REFL_STATIC_N_BALLS;
      if (m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "PFRefl"s, true))
         m_maxReflectionMode = RenderProbe::REFL_STATIC_N_DYNAMIC;
   }
   // For dynamic mode, static reflections are not available so adapt the mode
   if (!IsUsingStaticPrepass() && m_maxReflectionMode >= RenderProbe::REFL_STATIC)
      m_maxReflectionMode = RenderProbe::REFL_DYNAMIC;

   // Apply table specific overrides
   m_toneMapper = (ToneMapper)m_ptable->m_settings.LoadValueWithDefault(Settings::TableOverride, "ToneMapper"s, m_ptable->GetToneMapper());

   m_maxPrerenderedFrames = useVR ? 0 : m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "MaxPrerenderedFrames"s, 0);

   m_NudgeShake = m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "NudgeStrength"s, 2e-2f);
   m_sharpen = m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "Sharpen"s, 0);
   m_FXAA = m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "FXAA"s, (int)Disabled);
#ifdef ENABLE_SDL
   m_MSAASamples = m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "MSAASamples"s, 1);
#else
   // Sadly DX9 does not support resolving an MSAA depth buffer, making MSAA implementation complex for it. So just disable for now
   m_MSAASamples = 1;
#endif
   m_AAfactor = m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "AAFactor"s, m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "USEAA"s, false) ? 2.0f : 1.0f);
   m_dynamicAO = m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "DynamicAO"s, true);
   m_disableAO = m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "DisableAO"s, false);
   m_ss_refl = m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "SSRefl"s, false);
   m_scaleFX_DMD = m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "ScaleFXDMD"s, false);
   m_bloomOff = m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "ForceBloomOff"s, false);
   m_maxFramerate = m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "MaxFramerate"s, -1);
   if(m_maxFramerate > 0 && m_maxFramerate < 24) // at least 24 fps
      m_maxFramerate = 24;
   m_videoSyncMode = (VideoSyncMode)m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "SyncMode"s, VSM_INVALID);
   if (m_maxFramerate < 0 && m_videoSyncMode == VideoSyncMode::VSM_INVALID)
   {
      const int vsync = m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "AdaptiveVSync"s, -1);
      switch (vsync)
      {
      case -1: m_maxFramerate = 0; m_videoSyncMode = VideoSyncMode::VSM_FRAME_PACING; break;
      case 0: m_maxFramerate = 0; m_videoSyncMode = VideoSyncMode::VSM_NONE; break;
      case 1: m_maxFramerate = 0; m_videoSyncMode = VideoSyncMode::VSM_VSYNC; break;
      case 2: m_maxFramerate = 0; m_videoSyncMode = VideoSyncMode::VSM_ADAPTIVE_VSYNC; break;
      default: m_maxFramerate = vsync; m_videoSyncMode = VideoSyncMode::VSM_ADAPTIVE_VSYNC; break;
      }
   }
   if (m_maxFramerate < 0)
      m_maxFramerate = 0;
   if (m_videoSyncMode == VideoSyncMode::VSM_INVALID)
      m_videoSyncMode = VideoSyncMode::VSM_FRAME_PACING;
   if (useVR)
   {
      // Disable VSync for VR (sync is performed by the OpenVR runtime)
      m_videoSyncMode = VideoSyncMode::VSM_NONE;
      m_stereo3DfakeStereo = false;
      m_maxFramerate = 0;
   }

   m_ballImage = nullptr;
   m_decalImage = nullptr;

   m_overwriteBallImages = m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "OverwriteBallImage"s, false);
   m_minphyslooptime = min(m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "MinPhysLoopTime"s, 0), 1000);

   if (m_overwriteBallImages)
   {
       string imageName;
       bool hr = m_ptable->m_settings.LoadValue(Settings::Player, "BallImage"s, imageName);
       if (hr)
       {
         BaseTexture *const tex = BaseTexture::CreateFromFile(imageName, m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "MaxTexDimension"s, 0));

           if (tex != nullptr)
               m_ballImage = new Texture(tex);
       }
       hr = m_ptable->m_settings.LoadValue(Settings::Player, "DecalImage"s, imageName);
       if (hr)
       {
           BaseTexture *const tex = BaseTexture::CreateFromFile(imageName, m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "MaxTexDimension"s, 0));

           if (tex != nullptr)
               m_decalImage = new Texture(tex);
       }
   }

   m_throwBalls = m_ptable->m_settings.LoadValueWithDefault(Settings::Editor, "ThrowBallsAlwaysOn"s, false);
   m_ballControl = m_ptable->m_settings.LoadValueWithDefault(Settings::Editor, "BallControlAlwaysOn"s, false);
   m_debugBallSize = m_ptable->m_settings.LoadValueWithDefault(Settings::Editor, "ThrowBallSize"s, 50);
   m_debugBallMass = m_ptable->m_settings.LoadValueWithDefault(Settings::Editor, "ThrowBallMass"s, 1.0f);

   const int numberOfTimesToShowTouchMessage = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "NumberOfTimesToShowTouchMessage"s, 10);
   g_pvp->m_settings.SaveValue(Settings::Player, "NumberOfTimesToShowTouchMessage"s, max(numberOfTimesToShowTouchMessage - 1, 0));
   m_showTouchMessage = (numberOfTimesToShowTouchMessage != 0);

   m_showWindowedCaption = false;
   m_showDebugger = false;

   m_debugBalls = false;

   m_swap_ball_collision_handling = false;

   m_debugMode = false;

#ifdef STEPPING
   m_pauseTimeTarget = 0;
#endif
   m_pactiveballDebug = nullptr;

   m_gameWindowActive = false;
   m_debugWindowActive = false;
   m_userDebugPaused = false;
   m_hwndDebugOutput = nullptr;

   m_LastKnownGoodCounter = 0;
   m_ModalRefCount = 0;

   m_drawCursor = false;
   m_lastcursorx = 0xfffffff;
   m_lastcursory = 0xfffffff;

#ifdef DEBUGPHYSICS
   c_hitcnts = 0;
   c_collisioncnt = 0;
   c_contactcnt = 0;
#ifdef C_DYNAMIC
   c_staticcnt = 0;
#endif
   c_embedcnts = 0;
   c_timesearch = 0;

   c_kDNextlevels = 0;
   c_quadNextlevels = 0;

   c_traversed = 0;
   c_tested = 0;
   c_deepTested = 0;
#endif

   m_movedPlunger = 0;
   m_fExtPlungerSpeed = false;
   m_LastPlungerHit = 0;
   m_lastFlipTime = 0;

   for (unsigned int i = 0; i < MAX_TOUCHREGION; ++i)
      m_touchregion_pressed[i] = false;

   m_recordContacts = false;
   m_contacts.reserve(8);

   m_overall_frames = 0;

   m_dmd = int2(0,0);
   m_texdmd = nullptr;

   m_ScreenOffset = Vertex2D(0.f, 0.f);

   m_implicitPlayfieldMesh = nullptr;
}

Player::~Player()
{
   assert(g_pplayer == nullptr);
   m_closing = CS_CLOSED;
   m_ptable->StopPlaying();
   delete m_staticPrepassRT;
   delete m_ballImage;
   delete m_decalImage;
   delete m_pBCTarget;
   delete m_ptable;
   delete m_tonemapLUT;
}

void Player::PreRegisterClass(WNDCLASS& wc)
{
    wc.style = 0;
    wc.hInstance = g_pvp->theInstance;
    wc.lpszClassName = "VPPlayer"; // leave as-is as e.g. VPM relies on this
    wc.hIcon = LoadIcon(g_pvp->theInstance, MAKEINTRESOURCE(IDI_TABLE));
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.lpszMenuName = nullptr;
}

void Player::PreCreate(CREATESTRUCT& cs)
{
    int display = m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "Display"s, -1);
    if (display >= getNumberOfDisplays() || g_pvp->m_primaryDisplay)
        display = -1; // force primary monitor
    int x, y;
    getDisplaySetupByID(display, x, y, m_screenwidth, m_screenheight);

   if (m_stereo3D == STEREO_VR)
   {
      m_fullScreen = false;
      m_wnd_width = m_ptable->m_settings.LoadValueWithDefault(Settings::PlayerVR, "PreviewWidth"s, 640);
      m_wnd_height = m_ptable->m_settings.LoadValueWithDefault(Settings::PlayerVR, "PreviewHeight"s, 480);
   }
   else
   {
      m_fullScreen = m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "FullScreen"s, IsWindows10_1803orAbove());
      // command line override
      if (g_pvp->m_disEnableTrueFullscreen == 0)
         m_fullScreen = false;
      else if (g_pvp->m_disEnableTrueFullscreen == 1)
         m_fullScreen = true;
      m_wnd_width = m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "Width"s, m_fullScreen ? -1 : DEFAULT_PLAYER_WIDTH);
      m_wnd_height = m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "Height"s, m_wnd_width * 9 / 16);
   }
   if (m_wnd_width <= 0)
   {
      m_wnd_width = m_screenwidth;
      m_wnd_height = m_screenheight;
   }

   if (m_fullScreen)
   {
      x = 0;
      y = 0;
      m_screenwidth = m_wnd_width;
      m_screenheight = m_wnd_height;
      m_refreshrate = m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "RefreshRate"s, 0);
   }
   else
   {
      m_refreshrate = 0; // The default

      // constrain window to screen
      if (m_wnd_width > m_screenwidth)
      {
         m_wnd_width = m_screenwidth;
         m_wnd_height = m_wnd_width * 9 / 16;
      }
      if (m_wnd_height > m_screenheight)
      {
         m_wnd_height = m_screenheight;
         m_wnd_width = m_wnd_height * 16 / 9;
      }
      x += (m_screenwidth - m_wnd_width) / 2;
      y += (m_screenheight - m_wnd_height) / 2;

#ifdef _MSC_VER
      // is this a non-fullscreen window? -> get previously saved window position
      if ((m_wnd_height != m_screenheight) || (m_wnd_width != m_screenwidth))
      {
         const int xn = g_pvp->m_settings.LoadValueWithDefault(m_stereo3D == STEREO_VR ? Settings::PlayerVR : Settings::Player, "WindowPosX"s, x); //!! does this handle multi-display correctly like this?
         const int yn = g_pvp->m_settings.LoadValueWithDefault(m_stereo3D == STEREO_VR ? Settings::PlayerVR : Settings::Player, "WindowPosY"s, y);
         RECT r;
         r.left = xn;
         r.top = yn;
         r.right = xn + m_wnd_width;
         r.bottom = yn + m_wnd_height;
         if (MonitorFromRect(&r, MONITOR_DEFAULTTONULL) != nullptr) // window is visible somewhere, so use the coords from the registry
         {
            x = xn;
            y = yn;
         }
#endif
      }
   }

    int windowflags;
    int windowflagsex;

    const int captionheight = GetSystemMetrics(SM_CYCAPTION);

    if (false) // only do this nowadays if ESC menu is brought up //(!m_fullScreen && ((m_screenheight - m_height) >= (captionheight * 2))) // We have enough room for a frame?
    {
        // Add a pretty window border and standard control boxes.

        windowflags = WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_CLIPCHILDREN;
        windowflagsex = WS_EX_OVERLAPPEDWINDOW;

        //!! does not respect borders so far!!! -> change width/height accordingly ??
        //!! like this the render window is scaled and thus implicitly blurred!
        y -= captionheight;
        m_wnd_height += captionheight;
    }
    else // No window border, title, or control boxes.
    {
        windowflags = WS_POPUP;
        windowflagsex = 0;
    }

    ZeroMemory(&cs, sizeof(cs));
    cs.x = x; 
    cs.y = y;
    cs.cx = m_wnd_width;
    cs.cy = m_wnd_height;
    cs.style = windowflags;
    cs.dwExStyle = windowflagsex;
    cs.hInstance = g_pvp->theInstance;
    cs.lpszName = "Visual Pinball Player"; // leave as-is as e.g. VPM relies on this
    cs.lpszClass = "VPPlayer"; // leave as-is as e.g. VPM relies on this
}

void Player::CreateWnd(HWND parent /* = 0 */)
{
#ifdef ENABLE_SDL
   // SDL needs to create the window (as of SDL 2.0.22, SDL_CreateWindowFrom does not support OpenGL contexts) so we create it through SDL and attach it to win32++
   WNDCLASS wc;
   ZeroMemory(&wc, sizeof(wc));

   CREATESTRUCT cs;
   ZeroMemory(&cs, sizeof(cs));

   // Set the WNDCLASS parameters
   PreRegisterClass(wc);
   if (wc.lpszClassName)
   {
      ::RegisterClass(&wc);
      cs.lpszClass = wc.lpszClassName;
   }
   else
      cs.lpszClass = _T("Win32++ Window");
   SDL_RegisterApp(wc.lpszClassName, 0, g_pvp->theInstance);

   // Set a reasonable default window style.
   DWORD dwOverlappedStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
   cs.style = WS_VISIBLE | ((parent) ? WS_CHILD : dwOverlappedStyle);

   // Set a reasonable default window position
   if (0 == parent)
   {
      cs.x = CW_USEDEFAULT;
      cs.cx = CW_USEDEFAULT;
      cs.y = CW_USEDEFAULT;
      cs.cy = CW_USEDEFAULT;
   }

   // Allow the CREATESTRUCT parameters to be modified.
   PreCreate(cs);

   const int colordepth = m_stereo3D == STEREO_VR ? 32 : m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "ColorDepth"s, 32);
   constexpr bool video10bit = false; //!! Unsupported   m_stereo3D == STEREO_VR ? false : LoadValueWithDefault(Settings::Player, "Render10Bit"s, false);
   int channelDepth = video10bit ? 10 : ((colordepth == 16) ? 5 : 8);
   // We only set bit depth for fullscreen desktop modes (otherwise, use the desktop bit depth)
   if (m_fullScreen)
   {
      SDL_GL_SetAttribute(SDL_GL_RED_SIZE, channelDepth);
      SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, channelDepth);
      SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, channelDepth);
      SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 0);
      SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);
   }

   // Multisampling is performed on the offscreen buffers, not the window framebuffer
   SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
   SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);

#ifdef __OPENGLES__
   SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
   SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
   SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(__APPLE__) && defined(TARGET_OS_MAC)
   SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
   SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
   SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
#else
   SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
   //This would enforce a 4.1 context, disabling all recent features (storage buffers, debug information,...)
   //SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
   //SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
#endif

   SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

   const int display = g_pvp->m_primaryDisplay ? 0 : m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "Display"s, 0);
   vector<DisplayConfig> displays;
   getDisplayList(displays);
   int adapter = 0;
   for (vector<DisplayConfig>::iterator dispConf = displays.begin(); dispConf != displays.end(); ++dispConf)
      if (display == dispConf->display)
         adapter = dispConf->adapter;
   int displayX, displayY, displayWidth, displayHeight;
   getDisplaySetupByID(display, displayX, displayY, displayWidth, displayHeight);

   // Create the window.
   Uint32 flags = SDL_WINDOW_OPENGL;
#ifdef _MSC_VER
   flags |= SDL_WINDOW_BORDERLESS | SDL_WINDOW_HIDDEN | SDL_WINDOW_ALLOW_HIGHDPI;
#elif defined(__APPLE__) && !TARGET_OS_TV
   if (LoadValueWithDefault(Settings::Player, "HighDPI"s, true))
      flags |= SDL_WINDOW_ALLOW_HIGHDPI;
#endif

#ifndef _MSC_VER
   cs.x = SDL_WINDOWPOS_CENTERED_DISPLAY(adapter);
   cs.y = SDL_WINDOWPOS_CENTERED_DISPLAY(adapter);
#endif

   // Prevent from being top most, to allow DMD, B2S,... to be over the VPX window
   SDL_SetHint(SDL_HINT_ALLOW_TOPMOST, "0");

   PLOGI << "Creating main window"; // For profiling (SDL create window is fairly slow, can be more than 1 second, but there doesn't seem to be any workaround)

   if (m_fullScreen)
   {
      //FIXME we have a bug somewhere that will prevent SDL from keeping the focus when in fullscreen mode, so we just run in scaled windowed mode...
      //m_sdl_playfieldHwnd = SDL_CreateWindow(cs.lpszName, displayX, displayY, m_wnd_width, m_wnd_height, flags | SDL_WINDOW_FULLSCREEN);
      m_sdl_playfieldHwnd = SDL_CreateWindow(cs.lpszName, displayX, displayY, displayWidth, displayHeight, flags | SDL_WINDOW_FULLSCREEN);
      // Adjust refresh rate
      SDL_DisplayMode mode;
      SDL_GetWindowDisplayMode(m_sdl_playfieldHwnd, &mode);
      Uint32 format = mode.format;
      bool found = false;
      for (int index = 0; index < SDL_GetNumDisplayModes(adapter); index++)
      {
         SDL_GetDisplayMode(adapter, index, &mode);
         // if (mode.w == m_wnd_width && mode.h == m_wnd_height && mode.refresh_rate == m_refreshrate && mode.format == format)
         if (mode.w == displayWidth && mode.h == displayHeight && mode.refresh_rate == m_refreshrate && mode.format == format)
         {
            SDL_SetWindowDisplayMode(m_sdl_playfieldHwnd, &mode);
            found = true;
            break;
         }
      }
      if (!found)
         PLOGE << "Failed to find a display mode matching the requested refresh rate [" << m_refreshrate << ']';
   }
   else
   {
      // Don't ask fullscreen desktop as it will likely cause issue with overlaying DMD, B2S, Pup,... above VPX
      //if (cs.cx == displayWidth && cs.cy == displayHeight)
      //   flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
      m_sdl_playfieldHwnd = SDL_CreateWindow(cs.lpszName, cs.x, cs.y, cs.cx, cs.cy, flags);
   }
   SDL_DisplayMode mode;
   SDL_GetWindowDisplayMode(m_sdl_playfieldHwnd, &mode);
   PLOGI << "SDL display mode: " << mode.w << 'x' << mode.h << ' ' << mode.refresh_rate << "Hz " << SDL_GetPixelFormatName(mode.format);

   SDL_SysWMinfo wmInfo;
   SDL_VERSION(&wmInfo.version);
   SDL_GetWindowWMInfo(m_sdl_playfieldHwnd, &wmInfo);

   // Attach it (raise a WM_CREATE which in turns call OnInitialUpdate)
   Attach(wmInfo.info.win.window);

   if (cs.style & WS_VISIBLE)
   {
      if (cs.style & WS_MAXIMIZE)
         SDL_MaximizeWindow(m_sdl_playfieldHwnd);
      else if (cs.style & WS_MINIMIZE)
         SDL_MinimizeWindow(m_sdl_playfieldHwnd);
      else
         SDL_ShowWindow(m_sdl_playfieldHwnd);
   }

#else
   PLOGI << "Creating main window"; // For profiling
   Create();
#endif // ENABLE_SDL
}

void Player::OnInitialUpdate()
{
#ifndef _MSC_VER
#if (defined(__APPLE__) && TARGET_OS_IOS) || defined(__ANDROID__)
    m_supportsTouch = true;
#endif
#else
    // Check for Touch support
    m_supportsTouch = ((GetSystemMetrics(SM_DIGITIZER) & NID_READY) != 0) && ((GetSystemMetrics(SM_DIGITIZER) & NID_MULTI_INPUT) != 0)
        && (GetSystemMetrics(SM_MAXIMUMTOUCHES) != 0);

#if 1 // we do not want to handle WM_TOUCH
    if (!UnregisterTouchWindow)
        UnregisterTouchWindow = (pUnregisterTouchWindow)GetProcAddress(GetModuleHandle(TEXT("user32.dll")), "UnregisterTouchWindow");
    if (UnregisterTouchWindow)
        UnregisterTouchWindow(GetHwnd());
#else // would be useful if handling WM_TOUCH instead of WM_POINTERDOWN
    // Disable palm detection
    if (!RegisterTouchWindow)
        RegisterTouchWindow = (pRegisterTouchWindow)GetProcAddress(GetModuleHandle(TEXT("user32.dll")), "RegisterTouchWindow");
    if (RegisterTouchWindow)
        RegisterTouchWindow(GetHwnd(), 0);

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
            SetProp(m_playfieldHwnd, tabletAtom, (HANDLE)dwHwndTabletProperty);
        }
        // Gesture configuration
        GESTURECONFIG gc[] = { 0, 0, GC_ALLGESTURES };
        UINT uiGcs = 1;
        const BOOL bResult = SetGestureConfig(m_playfieldHwnd, 0, uiGcs, gc, sizeof(GESTURECONFIG));
    }
#endif

    // Disable visual feedback for touch, this saves one frame of latency on touchdisplays
    if (!SetWindowFeedbackSetting)
        SetWindowFeedbackSetting = (pSWFS)GetProcAddress(GetModuleHandle(TEXT("user32.dll")), "SetWindowFeedbackSetting");
    if (SetWindowFeedbackSetting)
    {
        constexpr BOOL enabled = FALSE;

        SetWindowFeedbackSetting(GetHwnd(), FEEDBACK_TOUCH_CONTACTVISUALIZATION, 0, sizeof(enabled), &enabled);
        SetWindowFeedbackSetting(GetHwnd(), FEEDBACK_TOUCH_TAP, 0, sizeof(enabled), &enabled);
        SetWindowFeedbackSetting(GetHwnd(), FEEDBACK_TOUCH_DOUBLETAP, 0, sizeof(enabled), &enabled);
        SetWindowFeedbackSetting(GetHwnd(), FEEDBACK_TOUCH_PRESSANDHOLD, 0, sizeof(enabled), &enabled);
        SetWindowFeedbackSetting(GetHwnd(), FEEDBACK_TOUCH_RIGHTTAP, 0, sizeof(enabled), &enabled);

        SetWindowFeedbackSetting(GetHwnd(), FEEDBACK_PEN_BARRELVISUALIZATION, 0, sizeof(enabled), &enabled);
        SetWindowFeedbackSetting(GetHwnd(), FEEDBACK_PEN_TAP, 0, sizeof(enabled), &enabled);
        SetWindowFeedbackSetting(GetHwnd(), FEEDBACK_PEN_DOUBLETAP, 0, sizeof(enabled), &enabled);
        SetWindowFeedbackSetting(GetHwnd(), FEEDBACK_PEN_PRESSANDHOLD, 0, sizeof(enabled), &enabled);
        SetWindowFeedbackSetting(GetHwnd(), FEEDBACK_PEN_RIGHTTAP, 0, sizeof(enabled), &enabled);

        SetWindowFeedbackSetting(GetHwnd(), FEEDBACK_GESTURE_PRESSANDTAP, 0, sizeof(enabled), &enabled);
    }
#endif

    mixer_init(GetHwnd());
    hid_init();

    const HRESULT result = Init();
    if (result != S_OK)
        throw 0; //!! have a more specific code (that is catched in the VPinball PeekMessageA loop)?!
}

void Player::OnClose()
{
   if (g_pplayer == nullptr || g_pplayer->m_closing == CS_CLOSED)
   {
      PLOGI << "Player::OnClose discarded since player is already closing";
      return;
   }
   assert(g_pplayer == this);
   m_closing = CS_CLOSED;
   PLOGI << "Closing player... [Player's VBS intepreter is #" << m_ptable->m_pcv->m_pScript << "]";

    g_frameProfiler.LogWorstFrame();

    // In Windows 10 1803, there may be a significant lag waiting for WM_DESTROY (msg sent by the delete call below) if script is not closed first.
    // signal the script that the game is now exited to allow any cleanup
    m_ptable->FireVoidEvent(DISPID_GameEvents_Exit);
    if (m_detectScriptHang)
        g_pvp->PostWorkToWorkerThread(HANG_SNOOP_STOP, NULL);

   // Save list of used textures to avoid stuttering in next play
   if ((m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "CacheMode"s, 1) > 0) && FileExists(m_ptable->m_szFileName))
   {
      string dir = g_pvp->m_szMyPrefPath + "Cache" + PATH_SEPARATOR_CHAR + m_ptable->m_szTitle + PATH_SEPARATOR_CHAR;
      std::filesystem::create_directories(std::filesystem::path(dir));

      std::map<string, bool> prevPreRenderOnly;
      if (!IsUsingStaticPrepass() && FileExists(dir + "used_textures.xml"))
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
      vector<BaseTexture *> textures = m_pin3d.m_pd3dPrimaryDevice->m_texMan.GetLoadedTextures();
      for (BaseTexture *memtex : textures)
      {
         for (Texture *image : m_ptable->m_vimage)
         {
            if (image->m_szName.length() > 0 && image->m_pdsBuffer == memtex)
            {
               tinyxml2::XMLElement *node = xmlDoc.NewElement("texture");
               node->SetText(image->m_szName.c_str());
               node->SetAttribute("filter", (int)m_pin3d.m_pd3dPrimaryDevice->m_texMan.GetFilter(memtex));
               node->SetAttribute("clampu", (int)m_pin3d.m_pd3dPrimaryDevice->m_texMan.GetClampU(memtex));
               node->SetAttribute("clampv", (int)m_pin3d.m_pd3dPrimaryDevice->m_texMan.GetClampV(memtex));
               node->SetAttribute("linear", m_pin3d.m_pd3dPrimaryDevice->m_texMan.IsLinearRGB(memtex));
               bool preRenderOnly = !IsUsingStaticPrepass() ? (prevPreRenderOnly.find(image->m_szName) != prevPreRenderOnly.end() ? prevPreRenderOnly[image->m_szName] : true)
                                                            : m_pin3d.m_pd3dPrimaryDevice->m_texMan.IsPreRenderOnly(memtex);
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

    // Save adjusted VR settings to the edited table
    if (m_stereo3D == STEREO_VR)
       m_pin3d.m_pd3dPrimaryDevice->SaveVRSettings(g_pvp->m_settings);

    if (m_audio)
        m_audio->MusicPause();

    mixer_shutdown();
    hid_shutdown();

    StopCaptures();
#ifdef ENABLE_SDL
   g_DXGIRegistry.ReleaseAll();
#endif

   delete m_liveUI;
   m_liveUI = nullptr;

   while (ShowCursor(FALSE) >= 0) ;
   while(ShowCursor(TRUE) < 0) ;

   m_pininput.UnInit();

   delete m_ballMeshBuffer;
   m_ballMeshBuffer = nullptr;
   #ifdef DEBUG_BALL_SPIN
   delete m_ballDebugPoints;
   m_ballDebugPoints = nullptr;
   #endif
   delete m_ballTrailMeshBuffer;
   m_ballTrailMeshBuffer = nullptr;
   delete m_ballImage;
   m_ballImage = nullptr;
   delete m_decalImage;
   m_decalImage = nullptr;

   m_limiter.Shutdown();

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

   for (size_t i = 0; i < m_vho.size(); i++)
      delete m_vho[i];
   m_vho.clear();

   for (size_t i = 0; i < m_vdebugho.size(); i++)
      delete m_vdebugho[i];
   m_vdebugho.clear();

   //!! cleanup the whole mem management for balls, this is a mess!

   // balls are added to the octree, but not the hit object vector
   for (size_t i = 0; i < m_vball.size(); i++)
   {
      Ball * const pball = m_vball[i];
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

   m_dmd = int2(0,0);
   if (m_texdmd)
   {
      m_pin3d.m_pd3dPrimaryDevice->DMDShader->SetTextureNull(SHADER_tex_dmd);
      m_pin3d.m_pd3dPrimaryDevice->m_texMan.UnloadTexture(m_texdmd);
      delete m_texdmd;
      m_texdmd = nullptr;
   }

#ifdef PLAYBACK
   if (m_fplaylog)
      fclose(m_fplaylog);
#endif

   //CloseHandle(m_hSongCompletionEvent);

   delete m_audio;
   m_audio = nullptr;

   for (size_t i = 0; i < m_controlclsidsafe.size(); i++)
      delete m_controlclsidsafe[i];
   m_controlclsidsafe.clear();

   m_changed_vht.clear();

   g_pplayer = nullptr;

   restore_win_timer_resolution();

   LockForegroundWindow(false);

   // Reactivate edited table or close application if requested
#ifndef __STANDALONE__
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
#endif

   // Destroy this player
   delete this; // Win32xx call Window destroy for us from destructor, don't call it directly or it will crash due to dual destruction
   PLOGI << "Player closed.";
}

void Player::InitFPS()
{
   m_fps = 0.f;

   m_count = 0;
   m_lastMaxChangeTime = 0;

   m_phys_max = 0;
   m_phys_max_iterations = 0;
   m_phys_total_iterations = 0;

   m_script_max = 0;

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

void Player::RecomputePauseState()
{
   const bool oldPause = m_pause;
   const bool newPause = !(m_gameWindowActive || m_debugWindowActive);// || m_userDebugPaused;

   if (oldPause && newPause)
   {
      m_LastKnownGoodCounter++; // So our catcher doesn't catch on the last value
      m_noTimeCorrect = true;
   }

   m_pause = newPause;
}

void Player::RecomputePseudoPauseState()
{
   const bool oldPseudoPause = m_pseudoPause;
   m_pseudoPause = m_userDebugPaused || m_debugWindowActive;
   if (oldPseudoPause != m_pseudoPause)
   {
      if (m_pseudoPause)
         PauseMusic();
      else
         UnpauseMusic();
   }
}

//
// license:GPLv3+
// Ported at: VisualPinball.Engine/VPT/Table/TableHitGenerator.cs
//

void Player::AddCabinetBoundingHitShapes()
{
   // simple outer borders:
   m_vho.push_back(new LineSeg(Vertex2D(m_ptable->m_right, m_ptable->m_top),    Vertex2D(m_ptable->m_right, m_ptable->m_bottom), 0.f, m_ptable->m_glassTopHeight));
   m_vho.push_back(new LineSeg(Vertex2D(m_ptable->m_left,  m_ptable->m_bottom), Vertex2D(m_ptable->m_left,  m_ptable->m_top),    0.f, m_ptable->m_glassBottomHeight));
   m_vho.push_back(new LineSeg(Vertex2D(m_ptable->m_right, m_ptable->m_bottom), Vertex2D(m_ptable->m_left,  m_ptable->m_bottom), 0.f, m_ptable->m_glassBottomHeight));
   m_vho.push_back(new LineSeg(Vertex2D(m_ptable->m_left,  m_ptable->m_top),    Vertex2D(m_ptable->m_right, m_ptable->m_top),    0.f, m_ptable->m_glassTopHeight));

   // glass:
   Vertex3Ds * const rgv3D = new Vertex3Ds[4];
   rgv3D[0] = Vertex3Ds(m_ptable->m_left, m_ptable->m_top, m_ptable->m_glassTopHeight);
   rgv3D[1] = Vertex3Ds(m_ptable->m_right, m_ptable->m_top, m_ptable->m_glassTopHeight);
   rgv3D[2] = Vertex3Ds(m_ptable->m_right, m_ptable->m_bottom, m_ptable->m_glassBottomHeight);
   rgv3D[3] = Vertex3Ds(m_ptable->m_left, m_ptable->m_bottom, m_ptable->m_glassBottomHeight);
   m_vho.push_back(new Hit3DPoly(rgv3D, 4)); //!!

   /*
   // playfield:
   Vertex3Ds * const rgv3D = new Vertex3Ds[4];
   rgv3D[3] = Vertex3Ds(m_ptable->m_left, m_ptable->m_top, 0.f);
   rgv3D[2] = Vertex3Ds(m_ptable->m_right, m_ptable->m_top, 0.f);
   rgv3D[1] = Vertex3Ds(m_ptable->m_right, m_ptable->m_bottom, 0.f);
   rgv3D[0] = Vertex3Ds(m_ptable->m_left, m_ptable->m_bottom, 0.f);
   Hit3DPoly * const ph3dpoly = new Hit3DPoly(rgv3D, 4); //!!
   ph3dpoly->SetFriction(m_ptable->m_overridePhysics ? m_ptable->m_fOverrideContactFriction : m_ptable->m_friction);
   ph3dpoly->m_elasticity = m_ptable->m_overridePhysics ? m_ptable->m_fOverrideElasticity : m_ptable->m_elasticity;
   ph3dpoly->m_elasticityFalloff = m_ptable->m_overridePhysics ? m_ptable->m_fOverrideElasticityFalloff : m_ptable->m_elasticityFalloff;
   ph3dpoly->m_scatter = ANGTORAD(m_ptable->m_overridePhysics ? m_ptable->m_fOverrideScatterAngle : m_ptable->m_scatter);
   m_vho.push_back(ph3dpoly);
   */

   // playfield:
   m_hitPlayfield = HitPlane(Vertex3Ds(0, 0, 1), 0.f);
   m_hitPlayfield.SetFriction(m_ptable->m_overridePhysics ? m_ptable->m_fOverrideContactFriction : m_ptable->m_friction);
   m_hitPlayfield.m_elasticity = m_ptable->m_overridePhysics ? m_ptable->m_fOverrideElasticity : m_ptable->m_elasticity;
   m_hitPlayfield.m_elasticityFalloff = m_ptable->m_overridePhysics ? m_ptable->m_fOverrideElasticityFalloff : m_ptable->m_elasticityFalloff;
   m_hitPlayfield.m_scatter = ANGTORAD(m_ptable->m_overridePhysics ? m_ptable->m_fOverrideScatterAngle : m_ptable->m_scatter);

   // glass:
   Vertex3Ds glassNormal(0, m_ptable->m_bottom - m_ptable->m_top, m_ptable->m_glassBottomHeight - m_ptable->m_glassTopHeight);
   glassNormal.Normalize();
   m_hitTopGlass = HitPlane(Vertex3Ds(0, glassNormal.z, -glassNormal.y), -m_ptable->m_glassTopHeight);
   m_hitTopGlass.m_elasticity = 0.2f;
}

//
// end of license:GPLv3+, back to 'old MAME'-like
//

void Player::InitKeys()
{
   for(unsigned int i = 0; i < eCKeys; ++i)
   {
      int key;
      const bool hr = m_ptable->m_settings.LoadValue(Settings::Player, regkey_string[i], key);
      if (!hr || key > 0xdd)
          key = regkey_defdik[i];
      m_rgKeys[i] = (EnumAssignKeys)key;
   }
}

void Player::InitDebugHitStructure()
{
   for (size_t i = 0; i < m_vhitables.size(); ++i)
   {
      Hitable * const ph = m_vhitables[i];
      const size_t currentsize = m_vdebugho.size();
      ph->GetHitShapesDebug(m_vdebugho);
      const size_t newsize = m_vdebugho.size();
      // Save the objects the trouble of having the set the idispatch pointer themselves
      for (size_t hitloop = currentsize; hitloop < newsize; hitloop++)
         m_vdebugho[hitloop]->m_pfedebug = m_ptable->m_vedit[i]->GetIFireEvents();
   }

   for (size_t i = 0; i < m_vdebugho.size(); ++i)
   {
      m_vdebugho[i]->CalcHitBBox(); // maybe needed to update here, as only done lazily for some objects (i.e. balls!)
      m_debugoctree.AddElement(m_vdebugho[i]);
   }

   const FRect3D bbox = m_ptable->GetBoundingBox();
   m_debugoctree.Initialize(FRect(bbox.left,bbox.right,bbox.top,bbox.bottom));
}

void Player::UpdateBasicShaderMatrix(const Matrix3D& objectTrafo)
{
   struct {
      Matrix3D matWorld;
      Matrix3D matView;
      Matrix3D matWorldView;
      Matrix3D matWorldViewInverseTranspose;
      Matrix3D matWorldViewProj[2];
   } matrices;
   m_pin3d.GetMVP().SetModel(objectTrafo);
   matrices.matWorld = m_pin3d.GetMVP().GetModel();
   matrices.matView = m_pin3d.GetMVP().GetView();
   matrices.matWorldView = m_pin3d.GetMVP().GetModelView();
   matrices.matWorldViewInverseTranspose = m_pin3d.GetMVP().GetModelViewInverseTranspose();

#ifdef ENABLE_SDL // OpenGL
   const int nEyes = m_pin3d.m_pd3dPrimaryDevice->m_stereo3D != STEREO_OFF ? 2 : 1;
   for (int eye = 0; eye < nEyes; eye++)
      matrices.matWorldViewProj[eye] = m_pin3d.GetMVP().GetModelViewProj(eye);
   m_pin3d.m_pd3dPrimaryDevice->flasherShader->SetMatrix(SHADER_matWorldViewProj, &matrices.matWorldViewProj[0].m[0][0], nEyes);
   m_pin3d.m_pd3dPrimaryDevice->lightShader->SetMatrix(SHADER_matWorldViewProj, &matrices.matWorldViewProj[0].m[0][0], nEyes);
   m_pin3d.m_pd3dPrimaryDevice->DMDShader->SetMatrix(SHADER_matWorldViewProj, &matrices.matWorldViewProj[0].m[0][0], nEyes);
   m_pin3d.m_pd3dPrimaryDevice->basicShader->SetUniformBlock(SHADER_basicMatrixBlock, &matrices.matWorld.m[0][0]);

#else // DirectX 9
   matrices.matWorldViewProj[0] = m_pin3d.GetMVP().GetModelViewProj(0);
   m_pin3d.m_pd3dPrimaryDevice->basicShader->SetMatrix(SHADER_matWorld, &matrices.matWorld);
   m_pin3d.m_pd3dPrimaryDevice->basicShader->SetMatrix(SHADER_matView, &matrices.matView);
   m_pin3d.m_pd3dPrimaryDevice->basicShader->SetMatrix(SHADER_matWorldView, &matrices.matWorldView);
   m_pin3d.m_pd3dPrimaryDevice->basicShader->SetMatrix(SHADER_matWorldViewInverseTranspose, &matrices.matWorldViewInverseTranspose);
   m_pin3d.m_pd3dPrimaryDevice->basicShader->SetMatrix(SHADER_matWorldViewProj, &matrices.matWorldViewProj[0]);
   m_pin3d.m_pd3dPrimaryDevice->flasherShader->SetMatrix(SHADER_matWorldViewProj, &matrices.matWorldViewProj[0]);
   m_pin3d.m_pd3dPrimaryDevice->lightShader->SetMatrix(SHADER_matWorldViewProj, &matrices.matWorldViewProj[0]);
   m_pin3d.m_pd3dPrimaryDevice->DMDShader->SetMatrix(SHADER_matWorldViewProj, &matrices.matWorldViewProj[0]);
#endif
}

void Player::UpdateStereoShaderState()
{
   if (m_stereo3DfakeStereo)
   {
      // FIXME compute max separation and zero point depth for fake stereo corresponding to the ones of the real stereo, remove these from settings and table properties
      // The problem with the legacy parameter is that both depends on the camera and do not match any physicaly correct measure. Authors tweak it but it will break at the
      // next depth buffer change (due to difference between rendering API or for example if we switch to reversed or infinite buffer for better precision, ...)
      // The idea would be (to be checked against shader implementation and ViewSetup projection maths):
      // - Max separation is the separation of a point with a very high depth (compute it from eye separation which is physically measures, and near/far planes)
      // - ZPD is the depth at which separation is 0 (compute it from the zNullSeparation in ViewSetup)
      /*ModelViewProj stereoMVP;
      m_ptable->mViewSetups[m_ptable->m_BG_current_set].ComputeMVP(m_ptable, m_pin3d.m_viewPort.Width, m_pin3d.m_viewPort.Height, true, stereoMVP);
      RECT viewport { 0, 0, (LONG)m_pin3d.m_viewPort.Width, (LONG)m_pin3d.m_viewPort.Height };
      vec3 deepPt(0.f, 0.f, 0.f); // = 5000.f * stereoMVP.GetModelViewInverse().GetOrthoNormalDir();
      Vertex2D projLeft, projRight;
      stereoMVP.GetModelViewProj(0).TransformVertices(&deepPt, nullptr, 1, &projLeft, viewport);
      stereoMVP.GetModelViewProj(1).TransformVertices(&deepPt, nullptr, 1, &projRight, viewport);*/
      const float eyeSeparation = m_ptable->GetMaxSeparation();
      const float zpd = m_ptable->GetZPD();
      const bool swapAxis = m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "Stereo3DYAxis"s, false); // Swap X/Y axis
      m_pin3d.m_pd3dPrimaryDevice->StereoShader->SetVector(SHADER_Stereo_MS_ZPD_YAxis, eyeSeparation, zpd, swapAxis ? 1.0f : 0.0f, 0.0f);
   }
   
   RenderTarget *renderedRT = m_pin3d.m_pd3dPrimaryDevice->GetPostProcessRenderTarget1();
   #ifdef ENABLE_SDL
   if (m_stereo3DfakeStereo) // OpenGL strip down this uniform which is only needed for interlaced mode on DirectX 9
   #endif
   m_pin3d.m_pd3dPrimaryDevice->StereoShader->SetVector(SHADER_w_h_height, (float)(1.0 / renderedRT->GetWidth()), (float)(1.0 / renderedRT->GetHeight()), (float)renderedRT->GetHeight(), m_ptable->Get3DOffset());

   m_stereo3DDefocus = 0.f;
   if (IsAnaglyphStereoMode(m_stereo3D))
   {
      Anaglyph anaglyph;
      anaglyph.LoadSetupFromRegistry(clamp(m_stereo3D - STEREO_ANAGLYPH_1, 0, 9));
      anaglyph.SetupShader(m_pin3d.m_pd3dPrimaryDevice->StereoShader);
      // The defocus kernel size should depend on the render resolution but since this is a user tweak, this doesn't matter that much
      m_stereo3DDefocus = m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "Stereo3DDefocus"s, 0.f);
      // TODO I'm not 100% sure about this. I think the right way would be to select based on the transmitted luminance of the filter, the defocus 
      // being done on the lowest of the 2. Here we do on the single color channel, which is the same most of the time but not always (f.e. green/magenta)
      if (anaglyph.IsReversedColorPair())
         m_stereo3DDefocus = -m_stereo3DDefocus;
   }
   else
   {
      m_pin3d.m_pd3dPrimaryDevice->StereoShader->SetTechnique(m_stereo3D == STEREO_SBS ? SHADER_TECHNIQUE_stereo_SBS 
                                                            : m_stereo3D == STEREO_TB  ? SHADER_TECHNIQUE_stereo_TB
                                                            : m_stereo3D == STEREO_INT ? SHADER_TECHNIQUE_stereo_Int 
                                                            :                            SHADER_TECHNIQUE_stereo_Flipped_Int);
   }
}

void Player::SetupShaders()
{
   const vec4 envEmissionScale_TexWidth(m_ptable->m_envEmissionScale * m_globalEmissionScale,
      (float) (m_pin3d.m_envTexture ? *m_pin3d.m_envTexture : m_pin3d.m_builtinEnvTexture).m_height /*+m_pin3d.m_builtinEnvTexture.m_width)*0.5f*/, 0.f, 0.f); //!! dto.

   UpdateBasicShaderMatrix();
   m_pin3d.m_pd3dPrimaryDevice->basicShader->SetTexture(SHADER_tex_env, m_pin3d.m_envTexture ? m_pin3d.m_envTexture : &m_pin3d.m_builtinEnvTexture);
   m_pin3d.m_pd3dPrimaryDevice->basicShader->SetVector(SHADER_fenvEmissionScale_TexWidth, &envEmissionScale_TexWidth);

   UpdateBallShaderMatrix();
   const vec4 st(m_ptable->m_envEmissionScale*m_globalEmissionScale, m_pin3d.m_envTexture ? (float)m_pin3d.m_envTexture->m_height/*+m_pin3d.m_envTexture->m_width)*0.5f*/ : (float)m_pin3d.m_builtinEnvTexture.m_height/*+m_pin3d.m_builtinEnvTexture.m_width)*0.5f*/, 0.f, 0.f);
   m_pin3d.m_pd3dPrimaryDevice->m_ballShader->SetVector(SHADER_fenvEmissionScale_TexWidth, &st);
   m_pin3d.m_pd3dPrimaryDevice->m_ballShader->SetVector(SHADER_fenvEmissionScale_TexWidth, &envEmissionScale_TexWidth);
   //m_pin3d.m_pd3dPrimaryDevice->m_ballShader->SetInt("iLightPointNum",MAX_LIGHT_SOURCES);

   constexpr float Roughness = 0.8f;
   m_pin3d.m_pd3dPrimaryDevice->m_ballShader->SetVector(SHADER_Roughness_WrapL_Edge_Thickness, exp2f(10.0f * Roughness + 1.0f), 0.f, 1.f, 0.05f);
   vec4 amb_lr = convertColor(m_ptable->m_lightAmbient, m_ptable->m_lightRange);
   m_pin3d.m_pd3dPrimaryDevice->m_ballShader->SetVector(SHADER_cAmbient_LightRange, 
      amb_lr.x * m_globalEmissionScale, amb_lr.y * m_globalEmissionScale, amb_lr.z * m_globalEmissionScale, m_ptable->m_lightRange);

   m_pin3d.InitLights();
}

void Player::UpdateBallShaderMatrix()
{
   struct {
      Matrix3D matView;
      Matrix3D matWorldView;
      Matrix3D matWorldViewInverse;
      Matrix3D matWorldViewProj[2];
   } matrices;
   m_pin3d.GetMVP().SetModel(Matrix3D::MatrixIdentity());
   matrices.matView = m_pin3d.GetMVP().GetView();
   matrices.matWorldView = m_pin3d.GetMVP().GetModelView();
   matrices.matWorldViewInverse = m_pin3d.GetMVP().GetModelViewInverse();
#ifdef ENABLE_SDL
   const int nEyes = m_pin3d.m_pd3dPrimaryDevice->m_stereo3D != STEREO_OFF ? 2 : 1;
   for (int eye = 0; eye < nEyes; eye++)
      matrices.matWorldViewProj[eye] = m_pin3d.GetMVP().GetModelViewProj(eye);
   m_pin3d.m_pd3dPrimaryDevice->m_ballShader->SetUniformBlock(SHADER_ballMatrixBlock, &matrices.matView.m[0][0]);
#else
   matrices.matWorldViewProj[0] = m_pin3d.GetMVP().GetModelViewProj(0);
   m_pin3d.m_pd3dPrimaryDevice->m_ballShader->SetMatrix(SHADER_matWorldViewProj, &matrices.matWorldViewProj[0]);
   m_pin3d.m_pd3dPrimaryDevice->m_ballShader->SetMatrix(SHADER_matWorldView, &matrices.matWorldView);
   m_pin3d.m_pd3dPrimaryDevice->m_ballShader->SetMatrix(SHADER_matWorldViewInverse, &matrices.matWorldViewInverse);
   m_pin3d.m_pd3dPrimaryDevice->m_ballShader->SetMatrix(SHADER_matView, &matrices.matView);
#endif
}

HRESULT Player::Init()
{
   TRACE_FUNCTION();

   PLOGI << "Initializing player"; // For profiling

   set_lowest_possible_win_timer_resolution();

   //m_hSongCompletionEvent = CreateEvent( nullptr, TRUE, FALSE, nullptr );

   m_pEditorTable->m_progressDialog.SetProgress(10);
   m_pEditorTable->m_progressDialog.SetName("Initializing Visuals..."s);

   InitKeys();

   m_PlayMusic = m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "PlayMusic"s, true);
   m_PlaySound = m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "PlaySound"s, true);
   m_MusicVolume = m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "MusicVolume"s, 100);
   m_SoundVolume = m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "SoundVolume"s, 100);

   // Global emission scale
   m_globalEmissionScale = m_ptable->m_globalEmissionScale;
   if (m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "OverrideTableEmissionScale"s, false))
   { // Overriden from settings
      if (m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "DynamicDayNight"s, false))
      {
         time_t hour_machine;
         time(&hour_machine);
         tm local_hour;
         localtime_s(&local_hour, &hour_machine);

         const float lat = m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "Latitude"s, 52.52f);
         const float lon = m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "Longitude"s, 13.37f);

         const double rlat = lat * (M_PI / 180.);
         const double rlong = lon * (M_PI / 180.);

         const double tr = TheoreticRadiation(local_hour.tm_mday, local_hour.tm_mon + 1, local_hour.tm_year + 1900, rlat);
         const double max_tr = MaxTheoreticRadiation(local_hour.tm_year + 1900, rlat);
         const double sset = SunsetSunriseLocalTime(local_hour.tm_mday, local_hour.tm_mon + 1, local_hour.tm_year + 1900, rlong, rlat, false);
         const double srise = SunsetSunriseLocalTime(local_hour.tm_mday, local_hour.tm_mon + 1, local_hour.tm_year + 1900, rlong, rlat, true);

         const double cur = local_hour.tm_hour + local_hour.tm_min / 60.0;

         const float factor = (float)(sin(M_PI * clamp((cur - srise) / (sset - srise), 0., 1.)) //!! leave space before sunrise and after sunset?
            * sqrt(tr / max_tr)); //!! magic, "emulates" that shorter days are usually also "darker",cloudier,whatever in most regions

         m_globalEmissionScale = clamp(factor, 0.15f, 1.f); //!! configurable clamp?
      }
      else
      {
         m_globalEmissionScale = m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "EmissionScale"s, 0.5f);
      }
   }
   if (g_pvp->m_bgles)
   { // Overriden from command line
      m_globalEmissionScale = g_pvp->m_fgles;
   }

   //

   const bool ss_refl = m_ss_refl && m_ptable->m_enableSSR;

   const int colordepth = m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "ColorDepth"s, 32);

   PLOGI << "Initializing renderer (global states & resources)"; // For profiling

   // colordepth & refreshrate are only defined if fullscreen is true.
   // width and height may be modified during initialization (for example for VR, they are adapted to the headset resolution)
   int w = m_wnd_width, h = m_wnd_height;
   if (m_stereo3DfakeStereo)
   {
      if (m_stereo3D == STEREO_SBS) // Side by side needs to fit the 2 views along the width, so each view is half the total width
         w = w / 2;
      else if (m_stereo3D == STEREO_TB) // Top/Bottom needs to fit the 2 views along the height, so each view is half the total height
         h = h / 2;
   }
   const HRESULT hr = m_pin3d.InitPin3D(m_fullScreen, w, h, colordepth, m_refreshrate, m_videoSyncMode, m_AAfactor, m_stereo3DfakeStereo ? STEREO_OFF : m_stereo3D, m_FXAA, !!m_sharpen, ss_refl);
   if (hr != S_OK)
   {
      char szFoo[64];
      sprintf_s(szFoo, sizeof(szFoo), "InitPin3D Error code: %x", hr);
      ShowError(szFoo);
      return hr;
   }
   m_pin3d.m_pd3dPrimaryDevice->m_vsyncCount = 1;
   m_maxFramerate = (m_videoSyncMode != VideoSyncMode::VSM_NONE && m_maxFramerate == 0) ? m_refreshrate : min(m_maxFramerate, m_refreshrate);
   PLOGI << "Synchronization mode: " << m_videoSyncMode << " with maximum FPS: " << m_maxFramerate << ", display FPS: " << m_refreshrate;

#ifdef ENABLE_SDL
   SDL_GL_GetDrawableSize(m_sdl_playfieldHwnd, &m_wnd_width, &m_wnd_height);
#endif
   // Set the output frame buffer size to the size of the window output
   m_pin3d.m_pd3dPrimaryDevice->GetOutputBackBuffer()->SetSize(m_wnd_width, m_wnd_height);

#ifdef _MSC_VER
   if (m_fullScreen)
      SetWindowPos(nullptr, 0, 0, m_wnd_width, m_wnd_height, SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE);
#endif

   PLOGI << "Initializing inputs & implicit objects"; // For profiling

   m_pininput.Init(GetHwnd());

   //
   const unsigned int lflip = get_vk(m_rgKeys[eLeftFlipperKey]);
   const unsigned int rflip = get_vk(m_rgKeys[eRightFlipperKey]);

   if (((GetAsyncKeyState(VK_LSHIFT) & 0x8000) && (GetAsyncKeyState(VK_RSHIFT) & 0x8000))
      || ((lflip != ~0u) && (rflip != ~0u) && (GetAsyncKeyState(lflip) & 0x8000) && (GetAsyncKeyState(rflip) & 0x8000)))
   {
      m_ptable->m_tblMirrorEnabled = true;
      int rotation = (int)(m_ptable->mViewSetups[m_ptable->m_BG_current_set].GetRotation(m_pin3d.m_pd3dPrimaryDevice->m_width, m_pin3d.m_pd3dPrimaryDevice->m_height)) / 90;
      m_pin3d.GetMVP().SetFlip(rotation == 0 || rotation == 2 ? ModelViewProj::FLIPX : ModelViewProj::FLIPY);
   }
   else
      m_ptable->m_tblMirrorEnabled = m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "mirror"s, false);

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

   // Initialize default state
   RenderState state;
   state.SetRenderState(RenderState::CULLMODE, m_ptable->m_tblMirrorEnabled ? RenderState::CULL_CW : RenderState::CULL_CCW);
   m_pin3d.m_pd3dPrimaryDevice->CopyRenderStates(false, state);
   m_pin3d.m_pd3dPrimaryDevice->SetDefaultRenderState();
   m_pin3d.InitLayout();

   const float minSlope = (m_ptable->m_overridePhysics ? m_ptable->m_fOverrideMinSlope : m_ptable->m_angletiltMin);
   const float maxSlope = (m_ptable->m_overridePhysics ? m_ptable->m_fOverrideMaxSlope : m_ptable->m_angletiltMax);
   const float slope = minSlope + (maxSlope - minSlope) * m_ptable->m_globalDifficulty;

   m_gravity.x = 0.f;
   m_gravity.y =  sinf(ANGTORAD(slope))*(m_ptable->m_overridePhysics ? m_ptable->m_fOverrideGravityConstant : m_ptable->m_Gravity);
   m_gravity.z = -cosf(ANGTORAD(slope))*(m_ptable->m_overridePhysics ? m_ptable->m_fOverrideGravityConstant : m_ptable->m_Gravity);

   m_Nudge = Vertex2D(0.f,0.f);

   m_legacyNudgeTime = 0;

   m_legacyNudge = m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "EnableLegacyNudge"s, false);
   m_legacyNudgeStrength = m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "LegacyNudgeStrength"s, 1.f);

   m_legacyNudgeBack = Vertex2D(0.f,0.f);

   m_movedPlunger = 0;

   Ball::ballID = 0;

   // Add a playfield primitive if it is missing
   m_implicitPlayfieldMesh = nullptr;
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

   m_pEditorTable->m_progressDialog.SetProgress(30);
   m_pEditorTable->m_progressDialog.SetName("Initializing Physics..."s);
   PLOGI << "Initializing physics"; // For profiling

   // Initialize new nudging.
   m_tableVel.SetZero();
   m_tableDisplacement.SetZero();
   m_tableVelOld.SetZero();
   m_tableVelDelta.SetZero();

   // Initialize velocity-based accelerometer input
   m_accelVel.SetZero();
   m_accelVelOld.SetZero();

   // Accelerometer inputs are accelerations (not velocities) by default
   m_accelInputIsVelocity = m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "AccelVelocityInput"s, false);

   m_plungerSpeedScale = m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "PlungerSpeedScale"s, 100.0f) / 100.0f;
   if (m_plungerSpeedScale <= 0.0f)
      m_plungerSpeedScale = 1.0f;

   // Table movement (displacement u) is modeled as a mass-spring-damper system
   //   u'' = -k u - c u'
   // with a spring constant k and a damping coefficient c.
   // See http://en.wikipedia.org/wiki/Damping#Linear_damping

   const float nudgeTime = m_ptable->m_nudgeTime;      // T
   constexpr float dampingRatio = 0.5f;                // zeta

   // time for one half period (one swing and swing back):
   //   T = pi / omega_d,
   // where
   //   omega_d = omega_0 * sqrt(1 - zeta^2)       (damped frequency)
   //   omega_0 = sqrt(k)                          (undamped frequency)
   // Solving for the spring constant k, we get
   m_nudgeSpring = (float)(M_PI*M_PI) / (nudgeTime*nudgeTime * (1.0f - dampingRatio*dampingRatio));

   // The formula for the damping ratio is
   //   zeta = c / (2 sqrt(k)).
   // Solving for the damping coefficient c, we get
   m_nudgeDamping = dampingRatio * 2.0f * sqrtf(m_nudgeSpring);

   // Need to set timecur here, for init functions that set timers
   m_time_msec = 0;

   m_last_frame_time_msec = 0;

   InitFPS();
   m_infoMode = IF_NONE;
   m_infoProbeIndex = 0;

   PLOGI << "Initializing Hitables"; // For profiling

   for (size_t i = 0; i < m_ptable->m_vedit.size(); i++)
   {
      IEditable * const pe = m_ptable->m_vedit[i];
      Hitable * const ph = pe->GetIHitable();
      if (ph)
      {
#ifdef DEBUGPHYSICS
         if(pe->GetScriptable())
         {
            CComBSTR bstr;
            pe->GetScriptable()->get_Name(&bstr);
            char * bstr2 = MakeChar(bstr);
            CHAR wzDst[256];
            sprintf_s(wzDst, sizeof(wzDst), "Initializing Object-Physics %s...", bstr2);
            delete [] bstr2;
            m_pEditorTable->m_progressDialog.SetName(wzDst);
         }
#endif
         const size_t currentsize = m_vho.size();
         ph->GetHitShapes(m_vho);
         const size_t newsize = m_vho.size();
         // Save the objects the trouble of having to set the idispatch pointer themselves
         for (size_t hitloop = currentsize; hitloop < newsize; hitloop++)
            m_vho[hitloop]->m_pfedebug = pe->GetIFireEvents();

         ph->GetTimers(m_vht);

         // build list of hitables
         m_vhitables.push_back(ph);
      }
   }

   m_pEditorTable->m_progressDialog.SetProgress(45);
   PLOGI << "Initializing octree"; // For profiling

   AddCabinetBoundingHitShapes();

   for (size_t i = 0; i < m_vho.size(); ++i)
   {
      HitObject * const pho = m_vho[i];

      pho->CalcHitBBox(); // maybe needed to update here, as only done lazily for some objects (i.e. balls!)
      m_hitoctree.AddElement(pho);

      if (pho->GetType() == eFlipper)
         m_vFlippers.push_back((HitFlipper*)pho);

      MoverObject * const pmo = pho->GetMoverObject();
      if (pmo && pmo->AddToList()) // Spinner, Gate, Flipper, Plunger (ball is added separately on each create ball)
         m_vmover.push_back(pmo);
   }

   const FRect3D tableBounds = m_ptable->GetBoundingBox();
   m_hitoctree.Initialize(FRect(tableBounds.left,tableBounds.right,tableBounds.top,tableBounds.bottom));
#if !defined(NDEBUG) && defined(PRINT_DEBUG_COLLISION_TREE)
   m_hitoctree.DumpTree(0);
#endif

   // initialize hit structure for dynamic objects
   m_hitoctree_dynamic.FillFromVector(m_vho_dynamic);

   //----------------------------------------------------------------------------------

   m_pEditorTable->m_progressDialog.SetProgress(50);
   m_pEditorTable->m_progressDialog.SetName("Loading Textures..."s);

   if ((m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "CacheMode"s, 1) > 0) && FileExists(m_ptable->m_szFileName))
   {
      try {
         string dir = g_pvp->m_szMyPrefPath + "Cache" + PATH_SEPARATOR_CHAR + m_ptable->m_szTitle + PATH_SEPARATOR_CHAR;
         std::filesystem::create_directories(std::filesystem::path(dir));
         if (FileExists(dir + "used_textures.xml"))
         {
            std::stringstream buffer;
            std::ifstream myFile(dir + "used_textures.xml");
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
                  if (name == nullptr)
                     continue;
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
                  m_render_mask = (!IsUsingStaticPrepass() || preRenderOnly) ? STATIC_ONLY : DEFAULT;
                  m_pin3d.m_pd3dPrimaryDevice->m_texMan.LoadTexture(tex->m_pdsBuffer, (SamplerFilter)filter, (SamplerAddressMode)clampU, (SamplerAddressMode)clampV, linearRGB);
                  PLOGI << "Texture preloading: '" << name << '\'';
               }
            }
         }
      }
      catch (...) // something failed while trying to preload images
      {
         PLOGE << "Texture preloading failed";
      }
      m_render_mask = DEFAULT;
   }

   //----------------------------------------------------------------------------------

   m_pEditorTable->m_progressDialog.SetProgress(60);
   m_pEditorTable->m_progressDialog.SetName("Initializing Renderer..."s);
   PLOGI << "Initializing renderer"; // For profiling

   SetupShaders();

   // FIXME we always loads the LUT since this can be changed in the LiveUI. Would be better to do this lazily
   //if (m_toneMapper == TM_TONY_MC_MAPFACE)
   {
      m_tonemapLUT = new Texture();
      m_tonemapLUT->LoadFromFile(g_pvp->m_szMyPath + "assets" + PATH_SEPARATOR_CHAR + "tony_mc_mapface_unrolled.exr");
      m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTexture(SHADER_tex_tonemap_lut, m_tonemapLUT, SF_BILINEAR, SA_CLAMP, SA_CLAMP, true);
   }

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
   for (RenderProbe* probe : m_ptable->m_vrenderprobe)
      probe->RenderSetup(m_pin3d.m_pd3dPrimaryDevice);
   for (Hitable* hitable : m_vhitables)
      hitable->RenderSetup(m_pin3d.m_pd3dPrimaryDevice);

   // Setup anisotropic filtering
   const bool forceAniso = m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "ForceAnisotropicFiltering"s, true);
   m_pin3d.m_pd3dPrimaryDevice->SetMainTextureDefaultFiltering(forceAniso ? SF_ANISOTROPIC : SF_TRILINEAR);

   const bool lowDetailBall = (m_ptable->GetDetailLevel() < 10);
   IndexBuffer *ballIndexBuffer = new IndexBuffer(m_pin3d.m_pd3dPrimaryDevice, lowDetailBall ? basicBallLoNumFaces : basicBallMidNumFaces, lowDetailBall ? basicBallLoIndices : basicBallMidIndices);
   VertexBuffer *ballVertexBuffer = new VertexBuffer(m_pin3d.m_pd3dPrimaryDevice, lowDetailBall ? basicBallLoNumVertices : basicBallMidNumVertices, (float *)(lowDetailBall ? basicBallLo : basicBallMid));
   m_ballMeshBuffer = new MeshBuffer(L"Ball"s, ballVertexBuffer, ballIndexBuffer, true);
   #ifdef DEBUG_BALL_SPIN
   {
      vector<Vertex3D_NoTex2> ballDbgVtx;
      for (int j = -1; j <= 1; ++j)
      {
         const int numPts = (j == 0) ? 6 : 3;
         const float theta = (float)(j * (M_PI/4.0));
         for (int i = 0; i < numPts; ++i)
         {
            const float phi = (float)(i * (2.0*M_PI) / numPts);
            Vertex3D_NoTex2 vtx;
            vtx.nx = cosf(theta) * cosf(phi);
            vtx.ny = cosf(theta) * sinf(phi);
            vtx.nz = sinf(theta);
            vtx.x = vtx.nx;
            vtx.y = vtx.ny;
            vtx.z = vtx.nz;
            vtx.tu = 0.f;
            vtx.tv = 0.f;
            ballDbgVtx.push_back(vtx);
         }
      }

      VertexBuffer *ballDebugPoints = new VertexBuffer(m_pin3d.m_pd3dPrimaryDevice, (unsigned int)ballDbgVtx.size(), (float *)ballDbgVtx.data(), false);
      m_ballDebugPoints = new MeshBuffer(L"Ball.Debug"s, ballDebugPoints);
   }
   #endif
   // Support up to 64 balls, that should be sufficient
   VertexBuffer* ballTrailVertexBuffer = new VertexBuffer(m_pin3d.m_pd3dPrimaryDevice, 64 * (MAX_BALL_TRAIL_POS - 2) * 2 + 4, nullptr, true);
   m_ballTrailMeshBuffer = new MeshBuffer(L"Ball.Trail"s, ballTrailVertexBuffer);

   #ifdef ENABLE_SDL
   if (m_stereo3D == STEREO_VR)
   {
      if (m_capExtDMD)
         StartDMDCapture();
      if (m_capPUP)
         StartPUPCapture();
   }
   #endif

   m_pEditorTable->m_progressDialog.SetName("Starting Game Scripts..."s);
   PLOGI << "Starting script"; // For profiling

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

   // Pre-render all non-changing elements such as static walls, rails, backdrops, etc. and also static playfield reflections
   // This is done after starting the script and firing the Init event to allow script to adjust static parts on startup
   m_pEditorTable->m_progressDialog.SetName("Prerendering Static Parts..."s);
   m_pEditorTable->m_progressDialog.SetProgress(70);
   PLOGI << "Prerendering static parts"; // For profiling
   RenderStaticPrepass();

#ifdef PLAYBACK
   if (m_playback)
      m_fplaylog = fopen("c:\\badlog.txt", "r");
#endif

   // Initialize stereo rendering
   UpdateStereoShaderState();

   wintimer_init();
   m_StartTime_usec = usec();
   m_curPhysicsFrameTime = m_StartTime_usec;
   m_nextPhysicsFrameTime = m_curPhysicsFrameTime + PHYSICS_STEPTIME;

   m_liveUI = new LiveUI(m_pin3d.m_pd3dPrimaryDevice);

#ifdef PLAYBACK
   if (m_playback)
   {
      float physicsStepTime;
      ParseLog((LARGE_INTEGER*)&physicsStepTime, (LARGE_INTEGER*)&m_StartTime_usec);
   }
#endif

#ifdef LOG
   PLOGD.printf("Step Time %llu", m_StartTime_usec);
   PLOGD.printf("End Frame");
#endif

   m_pEditorTable->m_progressDialog.SetProgress(100);
   m_pEditorTable->m_progressDialog.SetName("Starting..."s);
   PLOGI << "Startup done"; // For profiling

   g_pvp->GetPropertiesDocker()->EnableWindow(FALSE);
   g_pvp->GetLayersDocker()->EnableWindow(FALSE);
   g_pvp->GetToolbarDocker()->EnableWindow(FALSE);

   if(g_pvp->GetNotesDocker()!=nullptr)
      g_pvp->GetNotesDocker()->EnableWindow(FALSE);

   m_pEditorTable->EnableWindow(FALSE);

   m_pEditorTable->m_progressDialog.Destroy();

   // Show the window (even without preview, we need to create a window).
   ShowWindow(SW_SHOW);
   SetForegroundWindow();
   SetFocus();

   LockForegroundWindow(true);

   if (m_detectScriptHang)
      g_pvp->PostWorkToWorkerThread(HANG_SNOOP_START, NULL);

   // 0 means disable limiting of draw-ahead queue
   m_limiter.Init(m_pin3d.m_pd3dPrimaryDevice, m_maxPrerenderedFrames);

   // Broadcast a message to notify front-ends that it is 
   // time to reveal the playfield. 
#ifdef _MSC_VER
   UINT nMsgID = RegisterWindowMessage(_T("VPTableStart"));
   ::PostMessage(HWND_BROADCAST, nMsgID, NULL, NULL);
#endif

   // Popup notification on startup
   if (m_stereo3D != STEREO_OFF && m_stereo3D != STEREO_VR && !m_stereo3Denabled)
      m_liveUI->PushNotification("3D Stereo is enabled but currently toggled off, press F10 to toggle 3D Stereo on"s, 4000);
   if (m_supportsTouch && m_showTouchMessage) //!! visualize with real buttons or at least the areas?? Add extra buttons?
      m_liveUI->PushNotification("You can use Touch controls on this display: bottom left area to Start Game, bottom right area to use the Plunger\n"
                                 "lower left/right for Flippers, upper left/right for Magna buttons, top left for Credits and (hold) top right to Exit"s, 12000);

   if (m_playMode == 1)
      m_liveUI->OpenTweakMode();
   else if (m_playMode == 2 && m_stereo3D != STEREO_VR)
      m_liveUI->OpenLiveUI();

   return S_OK;
}

int Player::GetAOMode()
{
   // We must evaluate this dynamically since AO scale and enabled/disable can be changed from script
   if (m_disableAO || !m_ptable->m_enableAO || !m_pin3d.m_pd3dPrimaryDevice->DepthBufferReadBackAvailable() || m_ptable->m_AOScale == 0.f)
      return 0;
   if (m_dynamicAO)
      return 2;
   return IsUsingStaticPrepass() ? 1 : 0; // If AO is static prepass only and we are running without it, disable AO
}

void Player::RenderStaticPrepass()
{
   // For VR, we don't use any static pre-rendering
   if (m_stereo3D == STEREO_VR)
      return;

   if (!m_isStaticPrepassDirty)
      return;

   m_isStaticPrepassDirty = false;

   TRACE_FUNCTION();

   m_pin3d.m_pd3dPrimaryDevice->FlushRenderFrame();
   m_render_mask |= STATIC_ONLY;

   // The code will fail if the static render target is MSAA (the copy operation we are performing is not allowed)
   delete m_staticPrepassRT;
   m_staticPrepassRT = m_pin3d.m_pd3dPrimaryDevice->GetBackBufferTexture()->Duplicate("StaticPreRender"s);
   assert(!m_staticPrepassRT->IsMSAA());
   
   RenderTarget *accumulationSurface = IsUsingStaticPrepass() ? m_staticPrepassRT->Duplicate("Accumulation"s) : nullptr;

   RenderTarget *renderRT = GetAOMode() == 1 ? m_pin3d.m_pd3dPrimaryDevice->GetBackBufferTexture() : m_staticPrepassRT;

   // if rendering static/with heavy oversampling, disable the aniso/trilinear filter to get a sharper/more precise result overall!
   if (IsUsingStaticPrepass())
   {
      PLOGI << "Performing prerendering of static parts."; // For profiling
      m_pin3d.m_pd3dPrimaryDevice->SetMainTextureDefaultFiltering(SF_BILINEAR);
   }

   //#define STATIC_PRERENDER_ITERATIONS_KOROBOV 7.0 // for the (commented out) lattice-based QMC oversampling, 'magic factor', depending on the the number of iterations!
   // loop for X times and accumulate/average these renderings
   // NOTE: iter == 0 MUST ALWAYS PRODUCE an offset of 0,0!
   int n_iter = IsUsingStaticPrepass() ? (STATIC_PRERENDER_ITERATIONS - 1) : 0;
   for (int iter = n_iter; iter >= 0; --iter) // just do one iteration if in dynamic camera/light/material tweaking mode
   {
      m_pin3d.m_pd3dPrimaryDevice->m_curDrawnTriangles = 0;

      float u1 = xyLDBNbnot[iter*2  ];  //      (float)iter*(float)(1.0                                /STATIC_PRERENDER_ITERATIONS);
      float u2 = xyLDBNbnot[iter*2+1];  //fmodf((float)iter*(float)(STATIC_PRERENDER_ITERATIONS_KOROBOV/STATIC_PRERENDER_ITERATIONS), 1.f);
      // the following line implements filter importance sampling for a small gauss (i.e. less jaggies as it also samples neighboring pixels) -> but also potentially more artifacts in compositing!
      gaussianDistribution(u1, u2, 0.5f, 0.0f); //!! first 0.5 could be increased for more blur, but is pretty much what is recommended
      // sanity check to be sure to limit filter area to 3x3 in practice, as the gauss transformation is unbound (which is correct, but for our use-case/limited amount of samples very bad)
      assert(u1 > -1.5f && u1 < 1.5f);
      assert(u2 > -1.5f && u2 < 1.5f);
      // Last iteration MUST set a sample offset of 0,0 so that final depth buffer features 'correctly' centered pixel sample
      assert(iter != 0 || (u1 == 0.f && u2 == 0.f));

      // Setup Camera,etc matrices for each iteration.
      m_pin3d.InitLayout(u1, u2);

      // Direct all renders to the "static" buffer
      m_pin3d.m_pd3dPrimaryDevice->SetRenderTarget("PreRender Background"s, renderRT, false);
      m_pin3d.DrawBackground();
      m_pin3d.m_pd3dPrimaryDevice->FlushRenderFrame();

      m_pin3d.m_pd3dPrimaryDevice->SetRenderTarget("PreRender Draw"s, renderRT);

      if (IsUsingStaticPrepass())
      {
         // Mark all probes to be re-rendered for this frame (only if needed, lazily rendered)
         for (size_t i = 0; i < m_ptable->m_vrenderprobe.size(); ++i)
            m_ptable->m_vrenderprobe[i]->MarkDirty();

         // Render static parts
         UpdateBasicShaderMatrix();
         for (Hitable *hitable : m_vhitables)
            hitable->Render(m_render_mask);

         // Rendering is done to the static render target then accumulated to accumulationSurface
         // We use the framebuffer mirror shader which copies a weighted version of the bound texture
         m_pin3d.m_pd3dPrimaryDevice->SetRenderTarget("PreRender Accumulate"s, accumulationSurface);
         m_pin3d.m_pd3dPrimaryDevice->AddRenderTargetDependency(renderRT);
         m_pin3d.m_pd3dPrimaryDevice->ResetRenderState();
         m_pin3d.m_pd3dPrimaryDevice->SetRenderState(RenderState::ALPHABLENDENABLE, iter == STATIC_PRERENDER_ITERATIONS - 1 ? RenderState::RS_FALSE : RenderState::RS_TRUE);
         m_pin3d.m_pd3dPrimaryDevice->SetRenderState(RenderState::SRCBLEND, RenderState::ONE);
         m_pin3d.m_pd3dPrimaryDevice->SetRenderState(RenderState::DESTBLEND, RenderState::ONE);
         m_pin3d.m_pd3dPrimaryDevice->SetRenderState(RenderState::BLENDOP, RenderState::BLENDOP_ADD);
         m_pin3d.m_pd3dPrimaryDevice->SetRenderState(RenderState::ZENABLE, RenderState::RS_FALSE);
         m_pin3d.m_pd3dPrimaryDevice->SetRenderState(RenderState::ZWRITEENABLE, RenderState::RS_FALSE);
         m_pin3d.m_pd3dPrimaryDevice->SetRenderState(RenderState::CULLMODE, RenderState::CULL_NONE);
         m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTechnique(SHADER_TECHNIQUE_fb_mirror);
         m_pin3d.m_pd3dPrimaryDevice->FBShader->SetVector(SHADER_w_h_height, 
            (float)(1.0 / (double)renderRT->GetWidth()), (float)(1.0 / (double)renderRT->GetHeight()),
            (float)((double)STATIC_PRERENDER_ITERATIONS), 1.0f);
         m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTexture(SHADER_tex_fb_unfiltered, renderRT->GetColorSampler());
         m_pin3d.m_pd3dPrimaryDevice->DrawFullscreenTexturedQuad(m_pin3d.m_pd3dPrimaryDevice->FBShader);
         m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTextureNull(SHADER_tex_fb_unfiltered);
      }

      // Finish the frame.
      m_pin3d.m_pd3dPrimaryDevice->FlushRenderFrame();
      if (m_pEditorTable->m_progressDialog.IsWindow())
         m_pEditorTable->m_progressDialog.SetProgress(70 +(((30 * (n_iter + 1 - iter)) / (n_iter + 1))));
   }

   if (accumulationSurface)
   {
      // copy back weighted antialiased color result to the static render target, keeping depth untouched
      m_pin3d.m_pd3dPrimaryDevice->SetRenderTarget("PreRender Store"s, renderRT);
      m_pin3d.m_pd3dPrimaryDevice->BlitRenderTarget(accumulationSurface, renderRT, true, false);
      m_pin3d.m_pd3dPrimaryDevice->FlushRenderFrame(); // Execute before destroying the render target
      delete accumulationSurface;
   }

   // if rendering static/with heavy oversampling, re-enable the aniso/trilinear filter now for the normal rendering
   const bool forceAniso = m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "ForceAnisotropicFiltering"s, true);
   m_pin3d.m_pd3dPrimaryDevice->SetMainTextureDefaultFiltering(forceAniso ? SF_ANISOTROPIC : SF_TRILINEAR);

   // Now finalize static buffer with static AO
   if (GetAOMode() == 1)
   {
      PLOGI << "Starting static AO prerendering"; // For profiling

      const bool useAA = m_AAfactor != 1.0f;

      m_pin3d.m_pd3dPrimaryDevice->SetRenderTarget("PreRender AO Save Depth"s, m_staticPrepassRT);
      m_pin3d.m_pd3dPrimaryDevice->ResetRenderState();
      m_pin3d.m_pd3dPrimaryDevice->BlitRenderTarget(renderRT, m_staticPrepassRT, false, true);

      m_pin3d.m_pd3dPrimaryDevice->SetRenderState(RenderState::ALPHABLENDENABLE, RenderState::RS_FALSE);
      m_pin3d.m_pd3dPrimaryDevice->SetRenderState(RenderState::CULLMODE ,RenderState::CULL_NONE);
      m_pin3d.m_pd3dPrimaryDevice->SetRenderState(RenderState::ZWRITEENABLE, RenderState::RS_FALSE);
      m_pin3d.m_pd3dPrimaryDevice->SetRenderState(RenderState::ZENABLE, RenderState::RS_FALSE);

      m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTexture(SHADER_tex_depth, renderRT->GetDepthSampler());
      m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTexture(SHADER_tex_ao_dither, &m_pin3d.m_aoDitherTexture, SF_NONE, SA_REPEAT, SA_REPEAT, true); // FIXME the force linear RGB is not honored in VR
      m_pin3d.m_pd3dPrimaryDevice->FBShader->SetVector(SHADER_AO_scale_timeblur, m_ptable->m_AOScale, 0.1f, 0.f, 0.f);
      m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTechnique(SHADER_TECHNIQUE_AO);

      for (unsigned int i = 0; i < 50; ++i) // 50 iterations to get AO smooth
      {
         m_pin3d.m_pd3dPrimaryDevice->SetRenderTarget("PreRender AO"s, m_pin3d.m_pd3dPrimaryDevice->GetAORenderTarget(0));
         m_pin3d.m_pd3dPrimaryDevice->AddRenderTargetDependency(renderRT);
         m_pin3d.m_pd3dPrimaryDevice->AddRenderTargetDependency(m_pin3d.m_pd3dPrimaryDevice->GetAORenderTarget(1));
         if (i == 0)
            m_pin3d.m_pd3dPrimaryDevice->Clear(clearType::TARGET, 0, 1.0f, 0L);

         m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTexture(SHADER_tex_fb_filtered, m_pin3d.m_pd3dPrimaryDevice->GetAORenderTarget(1)->GetColorSampler()); //!! ?
         m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTexture(SHADER_tex_fb_unfiltered, m_pin3d.m_pd3dPrimaryDevice->GetAORenderTarget(1)->GetColorSampler()); //!! ?
         m_pin3d.m_pd3dPrimaryDevice->FBShader->SetVector(SHADER_w_h_height, 
            (float)(1.0 / m_pin3d.m_pd3dPrimaryDevice->GetAORenderTarget(1)->GetWidth()), (float)(1.0 / m_pin3d.m_pd3dPrimaryDevice->GetAORenderTarget(1)->GetHeight()),
            radical_inverse(i) * (float)(1. / 8.0), /*sobol*/ radical_inverse<3>(i) * (float)(1. / 8.0)); // jitter within (64/8)x(64/8) neighborhood of 64x64 tex, good compromise between blotches and noise
         m_pin3d.m_pd3dPrimaryDevice->DrawFullscreenTexturedQuad(m_pin3d.m_pd3dPrimaryDevice->FBShader);

         // flip AO buffers (avoids copy)
         m_pin3d.m_pd3dPrimaryDevice->SwapAORenderTargets();
      }

      m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTextureNull(SHADER_tex_depth);

      m_pin3d.m_pd3dPrimaryDevice->SetRenderTarget("PreRender Apply AO"s, m_staticPrepassRT);
      m_pin3d.m_pd3dPrimaryDevice->AddRenderTargetDependency(renderRT);
      m_pin3d.m_pd3dPrimaryDevice->AddRenderTargetDependency(m_pin3d.m_pd3dPrimaryDevice->GetAORenderTarget(1));

      m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTexture(SHADER_tex_fb_filtered, renderRT->GetColorSampler());
      m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTexture(SHADER_tex_fb_unfiltered, renderRT->GetColorSampler());
      m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTexture(SHADER_tex_ao, m_pin3d.m_pd3dPrimaryDevice->GetAORenderTarget(1)->GetColorSampler());

      m_pin3d.m_pd3dPrimaryDevice->FBShader->SetVector(SHADER_w_h_height, (float)(1.0 / renderRT->GetWidth()), (float)(1.0 / renderRT->GetHeight()), 1.0f, 1.0f);
      m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTechnique(useAA ? SHADER_TECHNIQUE_fb_AO_static : SHADER_TECHNIQUE_fb_AO_no_filter_static);

      m_pin3d.m_pd3dPrimaryDevice->DrawFullscreenTexturedQuad(m_pin3d.m_pd3dPrimaryDevice->FBShader);

      m_pin3d.m_pd3dPrimaryDevice->FlushRenderFrame(); // Execute before destroying the render targets

      // Delete buffers: we won't need them anymore since dynamic AO is disabled
      m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTextureNull(SHADER_tex_ao);
      m_pin3d.m_pd3dPrimaryDevice->ReleaseAORenderTargets();
   }

   if (m_MSAASamples > 1)
   {
      // Render one frame with MSAA to keep MSAA depth (this adds MSAA to the overlapping parts between statics & dynamics)
      RenderTarget* renderRT = m_pin3d.m_pd3dPrimaryDevice->GetMSAABackBufferTexture()->Duplicate("MSAAPreRender"s);
      m_pin3d.InitLayout();
      m_pin3d.m_pd3dPrimaryDevice->SetRenderTarget("PreRender MSAA Background"s, renderRT, false);
      m_pin3d.DrawBackground();
      m_pin3d.m_pd3dPrimaryDevice->FlushRenderFrame();
      if (IsUsingStaticPrepass())
      {
         m_pin3d.m_pd3dPrimaryDevice->SetRenderTarget("PreRender MSAA Scene"s, renderRT);
         m_pin3d.m_pd3dPrimaryDevice->ResetRenderState();
         for (size_t i = 0; i < m_ptable->m_vrenderprobe.size(); ++i)
            m_ptable->m_vrenderprobe[i]->MarkDirty();
         UpdateBasicShaderMatrix();
         for (Hitable *hitable : m_vhitables)
            hitable->Render(m_render_mask);
         m_pin3d.m_pd3dPrimaryDevice->FlushRenderFrame();
      }
      // Copy supersampled color buffer
      m_pin3d.m_pd3dPrimaryDevice->SetRenderTarget("PreRender Combine Color"s, renderRT);
      m_pin3d.m_pd3dPrimaryDevice->BlitRenderTarget(m_staticPrepassRT, renderRT, true, false);
      m_pin3d.m_pd3dPrimaryDevice->FlushRenderFrame();
      // Replace with this new MSAA pre render
      RenderTarget *initialPreRender = m_staticPrepassRT;
      m_staticPrepassRT = renderRT;
      delete initialPreRender;
   }

   if (IsUsingStaticPrepass())
   {
      PLOGI << "Starting Reflection Probe prerendering"; // For profiling
      for (RenderProbe *probe : m_ptable->m_vrenderprobe)
         probe->PreRenderStatic();
   }

   // Store the total number of triangles prerendered (including ones done for render probes)
   stats_drawn_static_triangles = m_pin3d.m_pd3dPrimaryDevice->m_curDrawnTriangles;

   PLOGI << "Static PreRender done"; // For profiling
   
   m_render_mask &= ~STATIC_ONLY;
   m_pin3d.m_pd3dPrimaryDevice->FlushRenderFrame();
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
   pball->m_pballex->RenderSetup(m_pin3d.m_pd3dPrimaryDevice);

   pball->m_pfedebug = (IFireEvents *)pball->m_pballex;

   m_vball.push_back(pball);
   m_vmover.push_back(&pball->m_mover); // balls are always added separately to this list!

   pball->CalcHitBBox(); // need to update here, as only done lazily

   m_vho_dynamic.push_back(pball);
   m_hitoctree_dynamic.FillFromVector(m_vho_dynamic);

   if (!m_pactiveballDebug)
      m_pactiveballDebug = pball;

   return pball;
}

void Player::DestroyBall(Ball *pball)
{
   if (!pball) return;

   const bool activeball = (m_pactiveball == pball);
   if (activeball)
      m_pactiveball = nullptr;

   const bool debugball = (m_pactiveballDebug == pball);
   if (debugball)
      m_pactiveballDebug = nullptr;

   if (m_pactiveballBC == pball)
      m_pactiveballBC = nullptr;

   RemoveFromVectorSingle(m_vball, pball);
   RemoveFromVectorSingle<MoverObject*>(m_vmover, &pball->m_mover);
   RemoveFromVectorSingle<HitObject*>(m_vho_dynamic, pball);

   m_hitoctree_dynamic.FillFromVector(m_vho_dynamic);

   m_vballDelete.push_back(pball);

   if (debugball && !m_vball.empty())
      m_pactiveballDebug = m_vball.front();
   if (activeball && !m_vball.empty())
      m_pactiveball = m_vball.front();
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// 
// Physics engine
// 
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma region Physics

void Player::NudgeX(const int x, const int joyidx)
{
   int v = x;
   if (x >  m_ptable->m_tblAccelMax.x) v =  m_ptable->m_tblAccelMax.x;
   if (x < -m_ptable->m_tblAccelMax.x) v = -m_ptable->m_tblAccelMax.x;
   m_curAccel[joyidx].x = v;
}

void Player::NudgeY(const int y, const int joyidx)
{
   int v = y;
   if (y >  m_ptable->m_tblAccelMax.y) v =  m_ptable->m_tblAccelMax.y;
   if (y < -m_ptable->m_tblAccelMax.y) v = -m_ptable->m_tblAccelMax.y;
   m_curAccel[joyidx].y = v;
}

// Get the accelerometer input, normalized to a -1..+1 range. 
// 
// (Note that there was a mistaken comment here in earlier versions
// that this normalizes the range to -2..+2.  I suspect someone saw
// the 2.0 factor and misconstrued it as the desired normalization
// range.  It's not; the 2.0 is there because JOYRANGE is defined
// as the width of the whole axis in joystick units, from negative
// to positive extreme, which makes JOYRANGE equal to 2x the
// absolute value of each extreme.  That is, JOYRANGE == 2*max_value
// and JOYRANGE == 2*abs(min_value), so JOYRANGE/2 == max_value, and
// finally 2/JOYRANGE*max_value == 1.  To be really precise, the 
// definition of JOYRANGE assumes that the range is for a 2's 
// complement field of some bit width, so min_value == -max_value-1,
// per the usual 2's complement rules.  So max_value is actually 
// JOYRANGE/2 - 1, hence the final normalized range is actually
// [-1, +1) - i.e., the normalized result can never actually reach 
// +1 exactly, but must always be fractionally less, because of the 
// limited range on the positive side of a 2's-complement field. 
// It's also worth noting that the 2's-complement assumption is NOT
// a device dependency; it's an assumption about how VP sets things 
// up with the OS HID API, so it's under VP's control and not 
// subject to breakage if the device uses some other axis 
// representation.  It would only break if VP's own HID API setup
// code chooses some other representation when initializing the 
// axis usages.)
#define GetNudgeX() (((F32)m_curAccel[0].x) * (F32)(2.0 / JOYRANGE))
#define GetNudgeY() (((F32)m_curAccel[0].y) * (F32)(2.0 / JOYRANGE)) 

#ifdef UNUSED_TILT
int Player::NudgeGetTilt()
{
   static U32 last_tilt_time;
   static U32 last_jolt_time;

   if(!m_ptable->m_tblAccelerometer || m_NudgeManual >= 0 ||                 //disabled or in joystick test mode
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

void Player::NudgeUpdate()      // called on every integral physics frame
{
   m_Nudge = Vertex2D(0.f,0.f); // accumulate over joysticks, these acceleration values are used in update ball velocity calculations
                                // and are required to be acceleration values (not velocity or displacement)

   if (!m_ptable->m_tblAccelerometer) return; // electronic accelerometer disabled 

   //rotate to match hardware mounting orientation, including left or right coordinates
   const float a = ANGTORAD(m_ptable->m_tblAccelAngle);
   const float cna = cosf(a);
   const float sna = sinf(a);

   for (int j = 0; j < m_pininput.m_num_joy; ++j)
   {
            float dx = ((float)m_curAccel[j].x)*(float)(1.0 / JOYRANGE); // norm range -1 .. 1   
      const float dy = ((float)m_curAccel[j].y)*(float)(1.0 / JOYRANGE);
      if (m_ptable->m_tblMirrorEnabled)
         dx = -dx;
            m_Nudge.x += m_ptable->m_tblAccelAmp.x * (dx*cna + dy*sna) * (1.0f - nudge_get_sensitivity()); // calc Green's transform component for X
      const float nugY = m_ptable->m_tblAccelAmp.y * (dy*cna - dx*sna) * (1.0f - nudge_get_sensitivity()); // calc Green's transform component for Y
      m_Nudge.y = m_ptable->m_tblAccelNormalMount ? (m_Nudge.y + nugY) : (m_Nudge.y - nugY);               // add as left or right hand coordinate system
   }

   // interpret the joystick input as a velocity if it's configured as such
   if (m_accelInputIsVelocity)
   {
      // move the input over to the velocity
      m_accelVel.x = m_Nudge.x;
      m_accelVel.y = m_Nudge.y;

      // clear the acceleration
      m_Nudge.SetZero();
   }
}

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
   // if we're receiving speed inputs, take a current snapshot
   if (m_fExtPlungerSpeed)
   {
      // compute the sum over joysticks
      m_curMechPlungerSpeed = 0;
      for (int i = 0; i < PININ_JOYMXCNT; ++i)
         m_curMechPlungerSpeed += m_curPlungerSpeed[i];
   }

   static int init = IIR_Order; // first time call
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

   // get the sum of current plunger inputs across joysticks
   float cur = 0;
   for (int i = 0; i < PININ_JOYMXCNT; ++i)
      cur += (float)m_curPlunger[i];

   if (!m_ptable->m_plungerFilter)
   {
      m_curMechPlungerPos = cur;
      return;
   }

   x[0] = cur; //initialize filter
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

int Player::GetMechPlungerSpeed() const 
{ 
    return m_curMechPlungerSpeed; 
}

// MechPlunger NOTE: Normalized position is from 0.0 to +1.0f
// +1.0 is fully retracted, 0.0 is all the way forward.
//
// The traditional method requires calibration in control panel game controllers to work right.
// The calibrated zero value should match the rest position of the mechanical plunger.
// The method below uses a dual - piecewise linear function to map the mechanical pull and push 
// onto the virtual plunger position from 0..1, the plunger properties has a ParkPosition setting 
// that matches the mechanical plunger zero position.
//
// If the plunger device is a "linear plunger", we replace the split axis with a single linear
// scaling that applies uniformly across the whole axis.  This eliminates the need for separate
// calibration on each side of the park position, and yields on-screen motion that tracks the
// mechanical plunger position more accurately.  The Pinscape Controller plunger uses the uniform
// linear method.
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

// Mechanical plunger speed, from I/O controller speed input, if configured.
// This takes input from the Plunger Speed axis, separate from the Plunger
// Position axis, allowing the controller to report instantaneous speeds
// along with position.  I/O controllers can usually measure the physical
// plunger's speed accurately thanks to their high-speed access to the raw
// sensor data.  It's impossible for the host to accurately compute the
// speed from position reports alone (via a first derivative of sequential
// position reports), because USB HID reports don't provide sufficient time
// resolution - physical plungers simply move too fast, so taking the first
// derivative results in pretty much random garbage a lot of the time.  The
// I/O controller can typically take readings at a high enough sampling
// rate to accurate track the speed, so we use its speed reports if they're
// available in preference to our internal speed calculations, which are
// unreliable at best.
float PlungerMoverObject::MechPlungerSpeed() const
{
   // Get the current speed reading
   float v = (float)g_pplayer->GetMechPlungerSpeed();

   // normalized the joystick input to -1..+1
   v *= (1.0f / (JOYRANGEMX - JOYRANGEMN));

   // The joystick report is device-defined speed units.  We
   // need to convert these to local speed units.  Since the
   // report units are device-specific, the conversion factor
   // is also device-specific, so the most general way to
   // handle it is as a user-adjustable setting.  This also
   // has the benefit that it allows the user to fine-tune the
   // feel to their liking.
   //
   // For reference, Pinscape Pico uses units where 1.0 (after
   // normalization) is the plunger travel length per
   // centisecond (10ms).  After scaling to the simulated
   // plunger length, that happens to equal VP9's native speed
   // units, so the scaling factor should be set to about 100%
   // when a Pinscape Pico is in use.
   v *= g_pplayer->m_plungerSpeedScale;

   // Scale to the virtual plunger we're operating.  The device
   // units are inherently relative to the length of the actual
   // mechanical plunger, so after conversion to simulation
   // units, they should maintain that proportionality to the
   // simulated plunger length.
   v *= m_frameLen;

   // Now apply the "mechanical strength" scaling.  This lets
   // the game set the relative strength of the plunger to be
   // higher or lower than "standard" (which is an arbitrary
   // reference point).  The strength is relative to the mass.
   // (The mass is actually a fixed constant, so including it
   // doesn't have any practical effect other than changing
   // the scale of the user-adjustable unit conversion factor
   // above, but we'll include it for consistency with other
   // places in the code where the mech strength is used.)
   v *= m_plunger->m_d.m_mechStrength / m_mass;

   // Return the result
   return v;
}

void Player::MechPlungerIn(const int z, const int joyidx)
{
   m_curPlunger[joyidx] = -z; //axis reversal

   if (++m_movedPlunger == 0xffffffff) m_movedPlunger = 3; //restart at 3
}

void Player::MechPlungerSpeedIn(const int z, const int joyidx)
{
   // record it
   m_curPlungerSpeed[joyidx] = -z;

   // flag that an external speed setting has been applied
   m_fExtPlungerSpeed = fTrue;
}

// Accelerometer data filter.
//
// This is designed to process the raw acceleration data from a
// physical accelerometer installed in a cabinet to yield more
// realistic effects on the virtual ball.  With a physical
// accelerometer, there are inherent inaccuracies due to small
// measurement errors from the instrument and the finite sampling
// rate.  In addition, the VP simulation only approximates real
// time, so VP can only approximate the duration of each
// instantaneous acceleration - this can exaggerate some inputs
// and under-apply others.  Some of these sources of error are
// random and tend to cancel out over time, but others compound
// over many samples.  In practice there can be noticeable bias
// that causes unrealistic results in the simulation.
//
// This filter is designed to compensate for these various sources
// of error by applying some assumptions about how a real cabinet
// should behave, and adjusting the real input accelerations to
// more closely match how our ideal model cabinet would behave.
//
// The main constraint we use in this filter is net zero motion.
// When you nudge a real cabinet, you make it sway on its legs a
// little, but you don't usually move the cabinet across the
// floor - real cabinets are quite heavy so they tend to stay
// firmly rooted in place during normal play.  So once the swaying
// from a nudge dies out, which happens fairly quickly (in about
// one second, say), the cabinet is back where it started.  This
// means that the cabinet experienced a series of accelerations,
// back and forth, that ultimately canceled out and left the
// box at rest at its original position.  This is the central
// assumption of this filter: we should be able to add up
// (integrate) the series of instantaneous velocities imparted
// by the instantaneous accelerations over the course of a nudge,
// and we know that at the end, the sum should be zero, because
// the cabinet is back at rest at its starting location.  In
// practice, real accelerometer data for a real nudge event will
// come *close* to netting to zero, but won't quite get there;
// the discrepancy is from the various sources of error described
// above.  This filter tries to reconcile the imperfect measured
// data with our mathematically ideal model by making small
// adjustments to the measured data to get it to match the
// ideal model results.
//
// The point of the filter is to make the results *feel* more
// realistic by reducing visible artifacts from the measurement
// inaccuracies.  We thus have to take care that the filter's
// meddling hand doesn't itself become apparent as another
// visible artifact.  We try to keep the filter's effects subtle
// by trying to limit its intervention to small adjustments.
// For the most part, it kicks in at the point in a nudge where
// the real accelerometer data says things are coming back to rest
// naturally, and simply cleans up the tail end of the nudge
// response to get it to mathematical exactness.  The filter also
// notices a special situation where it shouldn't intervene,
// which is when there's a sustained acceleration in one
// direction.  Assuming that we're not operating under
// extraordinary conditions (e.g., on board an airplane
// accelerating down the runway), a sustained acceleration can
// only mean that someone picked up one end of the cabinet and
// is holding it at an angle.  In this case the ball *should*
// be accelerated in the direction of the tilt, so we don't
// attempt to zero out the net accelerations when we notice this
// type of condition.
//
// It's important to understand that this filter is only useful
// when the nudge inputs are coming from a physical, analog
// accelerometer installed in a standard, free-standing pinball
// cabinet.  The model and the parameters are tailored for this
// particular physical configuration, and it won't give good results
// for other setups.  In particular, don't use this filter with
// "digital" on/off nudge inputs, such as keyboard-based nudging or
// with cabinet sensors based on mercury switches or plumb bobs.
// The nudge accelerations for these sorts of digital nudge inputs
// are simulated, so they're already free of the analog measurement
// errors that this filter is designed to compensate for.
//
// This filter *might* work with real accelerometers that aren't
// in standard cabinets, such as in mini-cabs or desktop controllers.
// It's designed to mimic the physics of a standard cabinet, so
// using it in another physical setup probably wouldn't emulate
// that setup's natural physical behavior.  But the filter might
// give pleasing results anyway simply because every VP setup is
// ultimately meant to simulate the cabinet experience, so you
// probably want the simulation to behave like a cabinet even when
// it's not actually running in a cabinet physically.
//

NudgeFilter::NudgeFilter()
{
   m_sum = m_prv = 0.0f;
   m_tMotion = m_tCorr = m_tzc = 0;
}

// Process a sample.  Adds the sample to the running total, and checks
// to see if a correction should be applied.  Replaces 'a' with the
// corrected value if a correction is needed.
void NudgeFilter::sample(float &a, const U64 now)
{
   IF_DEBUG_NUDGE(char notes[128] = ""; float aIn = a;)

   // if we're not roughly at rest, reset the last motion timer
   if (fabsf(a) >= .02f)
      m_tMotion = now;

   // check for a sign change
   if (fabsf(a) > .01f && fabsf(m_prv) > .01f
      && ((a < 0.f && m_prv > 0.f) || (a > 0.f && m_prv < 0.f)))
   {
      // sign change/zero crossing - note the time
      m_tzc = now;
      IF_DEBUG_NUDGE(strncat_s(notes, "zc ", sizeof(notes)-strnlen_s(notes, sizeof(notes))-1);)
   }
   else if (fabsf(a) <= .01f)
   {
      // small value -> not a sustained one-way acceleration
      m_tzc = now;
   }
   /*else if (fabsf(a) > .05f && now - m_tzc > 500000) // disabling this fixes an issue with Mot-Ion / Pinball Wizard controllers that suffer from calibration drift as they warm up
   {
      // More than 500 ms in motion with same sign - we must be
      // experiencing a gravitational acceleration due to a tilt
      // of the playfield rather than a transient acceleration
      // from a nudge.  Don't attempt to correct these - clear
      // the sum and do no further processing.
      m_sum = 0;
      IF_DEBUG_NUDGE(dbg("%f >>>\n", a));
      return;
   }*/

   // if this sample is non-zero, remember it as the previous sample
   if (a != 0.f)
      m_prv = a;

   // add this sample to the running total
   m_sum += a;

   // If the running total is near zero, correct it to exactly zero.
   // 
   // Otherwise, if it's been too long since the last correction, or
   // we've been roughly at rest a while, add a damping correction to
   // bring the running total toward rest.
   if (fabsf(m_sum) < .02f)
   {
      // bring the residual acceleration exactly to rest
      IF_DEBUG_NUDGE(strncat_s(notes, "zero ", sizeof(notes)-strnlen_s(notes, sizeof(notes))-1);)
         a -= m_sum;
      m_sum = 0.f;

      // this counts as a zero crossing reset
      m_prv = 0;
      m_tzc = m_tCorr = now;
   }
   else if (now - m_tCorr > 50000 || now - m_tMotion > 50000)
   {
      // bring the running total toward neutral
      const float corr = expf(0.33f*logf(fabsf(m_sum*(float)(1.0 / .02)))) * (m_sum < 0.0f ? -.02f : .02f);
      IF_DEBUG_NUDGE(strncat_s(notes, "damp ", sizeof(notes)-strnlen_s(notes, sizeof(notes))-1);)
         a -= corr;
      m_sum -= corr;

      // Advance the correction time slightly, but not all the
      // way to the present - we want to allow another forced
      // correction soon if necessary to get things back to
      // neutral quickly.
      m_tCorr = now - 40000;
   }

   IF_DEBUG_NUDGE(
      if (a != 0.f || aIn != 0.f)
         dbg(*axis() == 'x' ? "%f,%f, , ,%s\n" : " , ,%f,%f,%s\n",
         aIn, a, notes);)
}

// debug output
IF_DEBUG_NUDGE(void NudgeFilter::dbg(const char *fmt, ...) {
   va_list args;
   va_start(args, fmt);
   static FILE *fp = 0;
   if (fp == 0) fp = fopen("c:\\joystick.csv", "w");
   vfprintf(fp, fmt, args);
   va_end(args);
})

// apply nudge acceleration data filtering
void Player::FilterNudge()
{
   m_NudgeFilterX.sample(m_Nudge.x, m_curPhysicsFrameTime);
   m_NudgeFilterY.sample(m_Nudge.y, m_curPhysicsFrameTime);
}

//++++++++++++++++++++++++++++++++++++++++

void Player::SetGravity(float slopeDeg, float strength)
{
   m_gravity.x = 0;
   m_gravity.y = sinf(ANGTORAD(slopeDeg)) * strength;
   m_gravity.z = -cosf(ANGTORAD(slopeDeg)) * strength;
}

void Player::PhysicsSimulateCycle(float dtime) // move physics forward to this time
{
   // PLOGD << "Cycle " << dtime;

   int StaticCnts = STATICCNTS; // maximum number of static counts
   // it's okay to have this code outside of the inner loop, as the ball hitrects already include the maximum distance they can travel in that timespan
   m_hitoctree_dynamic.Update();

   while (dtime > 0.f)
   {
      // first find hits, if any +++++++++++++++++++++ 
#ifdef DEBUGPHYSICS
      c_timesearch++;
#endif
      float hittime = dtime;       // begin time search from now ...  until delta ends

      // find earliest time where a flipper collides with its stop
      for (size_t i = 0; i < m_vFlippers.size(); ++i)
      {
         const float fliphit = m_vFlippers[i]->GetHitTime();
         //if ((fliphit >= 0.f) && !sign(fliphit) && (fliphit <= hittime))
         if ((fliphit > 0.f) && (fliphit <= hittime)) //!! >= 0.f causes infinite loop
            hittime = fliphit;
      }

      m_recordContacts = true;
      m_contacts.clear();

#ifdef USE_EMBREE
      for (size_t i = 0; i < m_vball.size(); i++)
         if (!m_vball[i]->m_d.m_lockedInKicker
#ifdef C_DYNAMIC
             && m_vball[i]->m_dynamic > 0
#endif
            ) // don't play with frozen balls
         {
            m_vball[i]->m_coll.m_hittime = hittime; // search upto current hittime
            m_vball[i]->m_coll.m_obj = nullptr;
         }

      if (!m_vball.empty())
      {
         m_hitoctree.HitTestBall(m_vball);         // find the hit objects hit times
         m_hitoctree_dynamic.HitTestBall(m_vball); // dynamic objects !! should reuse the same embree scene created already in m_hitoctree.HitTestBall!
      }
#endif

      for (size_t i = 0; i < m_vball.size(); i++)
      {
         Ball * const pball = m_vball[i];

         if (!pball->m_d.m_lockedInKicker
#ifdef C_DYNAMIC
             && pball->m_dynamic > 0
#endif
            ) // don't play with frozen balls
         {
#ifndef USE_EMBREE
            pball->m_coll.m_hittime = hittime;          // search upto current hittime
            pball->m_coll.m_obj = nullptr;
#endif
            // always check for playfield and top glass
            if (m_implicitPlayfieldMesh)
               DoHitTest(pball, &m_hitPlayfield, pball->m_coll);

            DoHitTest(pball, &m_hitTopGlass, pball->m_coll);

#ifndef USE_EMBREE
            if (rand_mt_01() < 0.5f) // swap order of dynamic and static obj checks randomly
            {
               m_hitoctree_dynamic.HitTestBall(pball, pball->m_coll); // dynamic objects
               m_hitoctree.HitTestBall(pball, pball->m_coll);         // find the static hit objects hit times
            }
            else
            {
               m_hitoctree.HitTestBall(pball, pball->m_coll);         // find the static hit objects hit times
               m_hitoctree_dynamic.HitTestBall(pball, pball->m_coll); // dynamic objects
            }
#endif
            const float htz = pball->m_coll.m_hittime; // this ball's hit time
            if (htz < 0.f) pball->m_coll.m_obj = nullptr; // no negative time allowed

            if (pball->m_coll.m_obj)                   // hit object
            {
#ifdef DEBUGPHYSICS
               ++c_hitcnts;                            // stats for display

               if (/*pball->m_coll.m_hitRigid &&*/ pball->m_coll.m_hitdistance < -0.0875f) //rigid and embedded
                  ++c_embedcnts;
#endif
               ///////////////////////////////////////////////////////////////////////////

               if (htz <= hittime)                     // smaller hit time??
               {
                  hittime = htz;                       // record actual event time

                  if (hittime < STATICTIME)            // less than static time interval
                  {
                     /*if (!pball->m_coll.m_hitRigid) hittime = STATICTIME; // non-rigid ... set Static time
                     else*/ if (--StaticCnts < 0)
                     {
                        StaticCnts = 0;                // keep from wrapping
                        hittime = STATICTIME;
                     }
                  }
               }
            }
         }
      } // end loop over all balls

      m_recordContacts = false;

      // hittime now set ... or full frame if no hit 
      // now update displacements to collide-contact or end of physics frame
      // !!!!! 2) move objects to hittime

      if (hittime > STATICTIME) StaticCnts = STATICCNTS; // allow more zeros next round

      for (size_t i = 0; i < m_vmover.size(); i++)
         m_vmover[i]->UpdateDisplacements(hittime); // step 2: move the objects about according to velocities (spinner, gate, flipper, plunger, ball)

      // find balls that need to be collided and script'ed (generally there will be one, but more are possible)

      for (size_t i = 0; i < m_vball.size(); i++) // use m_vball.size(), in case script deletes a ball
      {
         Ball * const pball = m_vball[i];

         if (
#ifdef C_DYNAMIC
             pball->m_dynamic > 0 &&
#endif
             pball->m_coll.m_obj && pball->m_coll.m_hittime <= hittime) // find balls with hit objects and minimum time
         {
            // now collision, contact and script reactions on active ball (object)+++++++++
            HitObject * const pho = pball->m_coll.m_obj; // object that ball hit in trials
            m_pactiveball = pball;                       // For script that wants the ball doing the collision
#ifdef DEBUGPHYSICS
            c_collisioncnt++;
#endif
            pho->Collide(pball->m_coll);                 //!!!!! 3) collision on active ball
            pball->m_coll.m_obj = nullptr;                  // remove trial hit object pointer

            // Collide may have changed the velocity of the ball, 
            // and therefore the bounding box for the next hit cycle
            if (m_vball[i] != pball) // Ball still exists? may have been deleted from list
            {
               // collision script deleted the ball, back up one count
               --i;
               continue;
            }
            else
            {
#ifdef C_DYNAMIC
               // is this ball static? .. set static and quench        
               if (/*pball->m_coll.m_hitRigid &&*/ (pball->m_coll.m_hitdistance < (float)PHYS_TOUCH)) //rigid and close distance contacts //!! rather test isContact??
               {
                  const float mag = pball->m_vel.x*pball->m_vel.x + pball->m_vel.y*pball->m_vel.y; // values below are taken from simulation
                  if (pball->m_drsq < 8.0e-5f && mag < 1.0e-3f*m_ptable->m_Gravity*m_ptable->m_Gravity / GRAVITYCONST / GRAVITYCONST && fabsf(pball->m_vel.z) < 0.2f*m_ptable->m_Gravity / GRAVITYCONST)
                  {
                     if (--pball->m_dynamic <= 0)             //... ball static, cancels next gravity increment
                     {                                       // m_dynamic is cleared in ball gravity section
                        pball->m_dynamic = 0;
#ifdef DEBUGPHYSICS
                        c_staticcnt++;
#endif
                        pball->m_vel.x = pball->m_vel.y = pball->m_vel.z = 0.f; //quench the remaining velocity and set ...
                     }
                  }
               }
#endif
            }
         }
      }

#ifdef DEBUGPHYSICS
      c_contactcnt = (U32)m_contacts.size();
#endif
      /*
       * Now handle contacts.
       *
       * At this point UpdateDisplacements() was already called, so the state is different
       * from that at HitTest(). However, contacts have zero relative velocity, so
       * hopefully nothing catastrophic has happened in the meanwhile.
       *
       * Maybe a two-phase setup where we first process only contacts, then only collisions
       * could also work.
       */
      if (rand_mt_01() < 0.5f) // swap order of contact handling randomly
         for (size_t i = 0; i < m_contacts.size(); ++i)
            //if (m_contacts[i].m_hittime <= hittime) // does not happen often, and values then look sane, so do this check //!! why does this break some collisions (MM NZ&TT Reloaded Skitso, also CCC (Saloon))? maybe due to ball colliding with multiple things and then some sideeffect?
               m_contacts[i].m_obj->Contact(m_contacts[i], hittime);
      else
         for (size_t i = m_contacts.size() - 1; i != -1; --i)
            //if (m_contacts[i].m_hittime <= hittime) // does not happen often, and values then look sane, so do this check //!! why does this break some collisions (MM NZ&TT Reloaded Skitso, also CCC (Saloon))? maybe due to ball colliding with multiple things and then some sideeffect?
               m_contacts[i].m_obj->Contact(m_contacts[i], hittime);

      m_contacts.clear();

#ifdef C_BALL_SPIN_HACK
      // hacky killing of ball spin on resting balls (very low and very high spinning)
      for (size_t i = 0; i < m_vball.size(); i++)
      {
         Ball * const pball = m_vball[i];

         const unsigned int p0 = (pball->m_ringcounter_oldpos / (10000 / PHYSICS_STEPTIME) + 1) % MAX_BALL_TRAIL_POS;
         const unsigned int p1 = (pball->m_ringcounter_oldpos / (10000 / PHYSICS_STEPTIME) + 2) % MAX_BALL_TRAIL_POS;

         if (/*pball->m_coll.m_hitRigid &&*/ (pball->m_coll.m_hitdistance < (float)PHYS_TOUCH) && (pball->m_oldpos[p0].x != FLT_MAX) && (pball->m_oldpos[p1].x != FLT_MAX)) // only if already initialized
         {
            /*const float mag = pball->m_vel.x*pball->m_vel.x + pball->m_vel.y*pball->m_vel.y; // values below are copy pasted from above
            if (pball->m_drsq < 8.0e-5f && mag < 1.0e-3f*m_ptable->m_Gravity*m_ptable->m_Gravity / GRAVITYCONST / GRAVITYCONST && fabsf(pball->m_vel.z) < 0.2f*m_ptable->m_Gravity / GRAVITYCONST
            && pball->m_angularmomentum.Length() < 0.9f*m_ptable->m_Gravity / GRAVITYCONST
            ) //&& rand_mt_01() < 0.95f)
            {
            pball->m_angularmomentum *= 0.05f; // do not kill spin completely, otherwise stuck balls will happen during regular gameplay
            }*/

            const Vertex3Ds diff_pos = pball->m_oldpos[p0] - pball->m_d.m_pos;
            const float mag = diff_pos.x*diff_pos.x + diff_pos.y*diff_pos.y;
            const Vertex3Ds diff_pos2 = pball->m_oldpos[p1] - pball->m_d.m_pos;
            const float mag2 = diff_pos2.x*diff_pos2.x + diff_pos2.y*diff_pos2.y;

            const float threshold = (pball->m_angularmomentum.x*pball->m_angularmomentum.x + pball->m_angularmomentum.y*pball->m_angularmomentum.y) / max(mag, mag2);

            if (!infNaN(threshold) && threshold > 666.f)
            {
               const float damp = clamp(1.0f - (threshold - 666.f) / 10000.f, 0.23f, 1.f); // do not kill spin completely, otherwise stuck balls will happen during regular gameplay
               pball->m_angularmomentum *= damp;
            }
         }
      }
#endif

      dtime -= hittime;       //new delta .. i.e. time remaining

      m_swap_ball_collision_handling = !m_swap_ball_collision_handling; // swap order of ball-ball collisions

   } // end physics loop
}

void Player::UpdatePhysics()
{
   if (!g_pplayer) //!! meh, we have a race condition somewhere where we delete g_pplayer while still in use (e.g. if we have a script compile error and cancel the table start)
      return;

   g_frameProfiler.EnterProfileSection(FrameProfiler::PROFILE_PHYSICS);
   U64 initial_time_usec = usec();

   // DJRobX's crazy latency-reduction code
   U64 delta_frame = 0;
   if (m_minphyslooptime > 0 && m_lastFlipTime > 0)
   {
      // We want the physics loops to sync up to the frames, not
      // the post-render period, as that can cause some judder.
      delta_frame = initial_time_usec - m_lastFlipTime;
      initial_time_usec -= delta_frame;
   }

   if (m_noTimeCorrect) // After debugging script
   {
      // Shift whole game forward in time
      m_StartTime_usec       += initial_time_usec - m_curPhysicsFrameTime;
      m_nextPhysicsFrameTime += initial_time_usec - m_curPhysicsFrameTime;
      m_curPhysicsFrameTime   = initial_time_usec; // 0 time frame
      m_noTimeCorrect = false;
   }

#ifdef STEPPING
#ifndef EVENPHYSICSTIME
   if (m_debugWindowActive || m_userDebugPaused)
   {
      // Shift whole game forward in time
      m_StartTime_usec       += initial_time_usec - m_curPhysicsFrameTime;
      m_nextPhysicsFrameTime += initial_time_usec - m_curPhysicsFrameTime;
      if (m_step)
      {
         // Walk one physics step forward
         m_curPhysicsFrameTime = initial_time_usec - PHYSICS_STEPTIME;
         m_step = false;
      }
      else
         m_curPhysicsFrameTime = initial_time_usec; // 0 time frame
   }
#endif
#endif

#ifdef EVENPHYSICSTIME
   if (!m_pause || m_step)
   {
      initial_time_usec = m_curPhysicsFrameTime - 3547811060 + 3547825450;
      m_step = false;
   }
   else
      initial_time_usec = m_curPhysicsFrameTime;
#endif

#ifdef LOG
   const double timepassed = (double)(initial_time_usec - m_curPhysicsFrameTime) / 1000000.0;

   const float frametime =
#ifdef PLAYBACK
      (!m_playback) ? (float)(timepassed * 100.0) : ParseLog((LARGE_INTEGER*)&initial_time_usec, (LARGE_INTEGER*)&m_nextPhysicsFrameTime);
#else
#define TIMECORRECT 1
#ifdef TIMECORRECT
      (float)(timepassed * 100.0);
   // 1.456927f;
#else
      0.45f;
#endif
#endif //PLAYBACK

   PLOGD.printf("Frame Time %.20f %u %u %u %u", frametime, initial_time_usec >> 32, initial_time_usec, m_nextPhysicsFrameTime >> 32, m_nextPhysicsFrameTime);
   PLOGD.printf("End Frame");
#endif

   m_phys_iterations = 0;

   while (m_curPhysicsFrameTime < initial_time_usec) // loop here until current (real) time matches the physics (simulated) time
   {
      // Get time in milliseconds for timers
      m_time_msec = (U32)((m_curPhysicsFrameTime - m_StartTime_usec) / 1000);

      m_phys_iterations++;

      // Get the time until the next physics tick is done, and get the time
      // until the next frame is done
      // If the frame is the next thing to happen, update physics to that
      // point next update acceleration, and continue loop

      const float physics_diff_time = (float)((double)(m_nextPhysicsFrameTime - m_curPhysicsFrameTime)*(1.0 / DEFAULT_STEPTIME));
      //const float physics_to_graphic_diff_time = (float)((double)(initial_time_usec - m_curPhysicsFrameTime)*(1.0 / DEFAULT_STEPTIME));

      //if (physics_to_graphic_diff_time < physics_diff_time)          // is graphic frame time next???
      //{
      //      PhysicsSimulateCycle(physics_to_graphic_diff_time);      // advance physics to this time
      //      m_curPhysicsFrameTime = initial_time_usec;               // now current to the wall clock
      //      break;  //this is the common exit from the loop          // exit skipping accelerate
      //}                     // some rare cases will exit from while()


      // DJRobX's crazy latency-reduction code: Artificially lengthen the execution of the physics loop by X usecs, to give more opportunities to read changes from input(s) (try values in the multiple 100s up to maximum 1000 range, in general: the more, the faster the CPU is)
      //                                        Intended mainly to be used if vsync is enabled (e.g. most idle time is shifted from vsync-waiting to here)
      if (m_minphyslooptime > 0)
      {
         const U64 basetime = usec();
         const U64 targettime = ((U64)m_minphyslooptime * m_phys_iterations) + m_lastFlipTime;
         // If we're 3/4 of the way through the loop, fire a "controller sync" timer (timers with an interval set to -2) event so VPM can react to input.
         if (m_phys_iterations == 750 / ((int)m_fps + 1))
         {
            for (HitTimer *const pht : m_vht)
               if (pht->m_interval == -2)
               {
                  g_frameProfiler.EnterScriptSection(DISPID_TimerEvents_Timer, pht->m_name);
                  pht->m_pfe->FireGroupEvent(DISPID_TimerEvents_Timer);
                  g_frameProfiler.ExitScriptSection(pht->m_name);
               }
         }
         if (basetime < targettime)
         {
            g_frameProfiler.EnterProfileSection(FrameProfiler::PROFILE_SLEEP);
            uSleep(targettime - basetime);
            g_frameProfiler.ExitProfileSection();
         }
      }
      // end DJRobX's crazy code
      const U64 cur_time_usec = usec()-delta_frame; //!! one could also do this directly in the while loop condition instead (so that the while loop will really match with the current time), but that leads to some stuttering on some heavy frames

      // hung in the physics loop over 200 milliseconds or the number of physics iterations to catch up on is high (i.e. very low/unplayable FPS)
      if ((cur_time_usec - initial_time_usec > 200000) || (m_phys_iterations > ((m_ptable->m_PhysicsMaxLoops == 0) || (m_ptable->m_PhysicsMaxLoops == 0xFFFFFFFFu) ? 0xFFFFFFFFu : (m_ptable->m_PhysicsMaxLoops*(10000 / PHYSICS_STEPTIME))/*2*/)))
      {                                                             // can not keep up to real time
         m_curPhysicsFrameTime  = initial_time_usec;                // skip physics forward ... slip-cycles -> 'slowed' down physics
         m_nextPhysicsFrameTime = initial_time_usec + PHYSICS_STEPTIME;
         break;                                                     // go draw frame
      }

      //update keys, hid, plumb, nudge, timers, etc
      //const U32 sim_msec = (U32)(m_curPhysicsFrameTime / 1000);
      const U32 cur_time_msec = (U32)(cur_time_usec / 1000);

      m_pininput.ProcessKeys(/*sim_msec,*/ cur_time_msec);

      mixer_update();
      hid_update(/*sim_msec*/cur_time_msec);
      plumb_update(/*sim_msec*/cur_time_msec, GetNudgeX(), GetNudgeY());

#ifdef ACCURATETIMERS
      // do the en/disable changes for the timers that piled up
      for(size_t i = 0; i < m_changed_vht.size(); ++i)
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

      Ball * const old_pactiveball = m_pactiveball;
      m_pactiveball = nullptr; // No ball is the active ball for timers/key events

      if (m_videoSyncMode == VideoSyncMode::VSM_FRAME_PACING || g_frameProfiler.Get(FrameProfiler::PROFILE_SCRIPT) <= 1000 * MAX_TIMERS_MSEC_OVERALL) // if overall script time per frame exceeded, skip
      {
         const unsigned int p_timeCur = (unsigned int)((m_curPhysicsFrameTime - m_StartTime_usec) / 1000); // milliseconds

         for (size_t i = 0; i < m_vht.size(); i++)
         {
            HitTimer * const pht = m_vht[i];
            if (pht->m_interval >= 0 && pht->m_nextfire <= p_timeCur)
            {
               g_frameProfiler.EnterScriptSection(DISPID_TimerEvents_Timer, pht->m_name);
               const unsigned int curnextfire = pht->m_nextfire;
               pht->m_pfe->FireGroupEvent(DISPID_TimerEvents_Timer);
               // Only add interval if the next fire time hasn't changed since the event was run. 
               // Handles corner case:
               //Timer1.Enabled = False
               //Timer1.Interval = 1000
               //Timer1.Enabled = True
               if (curnextfire == pht->m_nextfire && pht->m_interval > 0)
                  while (pht->m_nextfire <= p_timeCur)
                     pht->m_nextfire += pht->m_interval;
               g_frameProfiler.ExitScriptSection(pht->m_name);
            }
         }
      }

      m_pactiveball = old_pactiveball;
#endif

      NudgeUpdate();       // physics_diff_time is the balance of time to move from the graphic frame position to the next
      MechPlungerUpdate(); // integral physics frame. So the previous graphics frame was (1.0 - physics_diff_time) before 
      // this integral physics frame. Accelerations and inputs are always physics frame aligned

      // table movement is modeled as a mass-spring-damper system
      //   u'' = -k u - c u'
      // with a spring constant k and a damping coefficient c
      const Vertex3Ds force = -m_nudgeSpring * m_tableDisplacement - m_nudgeDamping * m_tableVel;
      m_tableVel          += (float)PHYS_FACTOR * force;
      m_tableDisplacement += (float)PHYS_FACTOR * m_tableVel;

      m_tableVelDelta = m_tableVel - m_tableVelOld;
      m_tableVelOld = m_tableVel;

      // apply the external accelerometer-based nudge velocity input (which is
      // a separate input from the traditional acceleration input)
      m_tableVelDelta += m_accelVel - m_accelVelOld;
      m_accelVelOld = m_accelVel;

      // legacy/VP9 style keyboard nudging
      if (m_legacyNudge && m_legacyNudgeTime != 0)
      {
          --m_legacyNudgeTime;

          if (m_legacyNudgeTime == 95)
          {
              m_Nudge.x = -m_legacyNudgeBack.x * 2.0f;
              m_Nudge.y =  m_legacyNudgeBack.y * 2.0f;
          }
          else if (m_legacyNudgeTime == 90)
          {
              m_Nudge.x =  m_legacyNudgeBack.x;
              m_Nudge.y = -m_legacyNudgeBack.y;
          }

          if (m_NudgeShake > 0.0f)
              SetScreenOffset(m_NudgeShake * m_legacyNudgeBack.x * sqrf((float)m_legacyNudgeTime*0.01f), -m_NudgeShake * m_legacyNudgeBack.y * sqrf((float)m_legacyNudgeTime*0.01f));
      }
      else
          if (m_NudgeShake > 0.0f)
          {
              // NB: in table coordinates, +Y points down, but in screen coordinates, it points up,
              // so we have to flip the y component
              SetScreenOffset(m_NudgeShake * m_tableDisplacement.x, -m_NudgeShake * m_tableDisplacement.y);
          }

      // Apply our filter to the nudge data
      if (m_pininput.m_enable_nudge_filter)
         FilterNudge();

      for (size_t i = 0; i < m_vmover.size(); i++)
         m_vmover[i]->UpdateVelocities();      // always on integral physics frame boundary (spinner, gate, flipper, plunger, ball)

      //primary physics loop
      PhysicsSimulateCycle(physics_diff_time); // main simulator call

      //ball trail, keep old pos of balls
      for (size_t i = 0; i < m_vball.size(); i++)
      {
         Ball * const pball = m_vball[i];
         pball->m_oldpos[pball->m_ringcounter_oldpos / (10000 / PHYSICS_STEPTIME)] = pball->m_d.m_pos;

         pball->m_ringcounter_oldpos++;
         if (pball->m_ringcounter_oldpos == MAX_BALL_TRAIL_POS*(10000 / PHYSICS_STEPTIME))
            pball->m_ringcounter_oldpos = 0;
      }

      //slintf( "PT: %f %f %u %u %u\n", physics_diff_time, physics_to_graphic_diff_time, (U32)(m_curPhysicsFrameTime/1000), (U32)(initial_time_usec/1000), cur_time_msec );

      m_curPhysicsFrameTime = m_nextPhysicsFrameTime; // new cycle, on physics frame boundary
      m_nextPhysicsFrameTime += PHYSICS_STEPTIME;     // advance physics position
   } // end while (m_curPhysicsFrameTime < initial_time_usec)

   g_frameProfiler.ExitProfileSection();
}

#pragma endregion



///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Rendering engine
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma region Rendering

void Player::Spritedraw(const float posx, const float posy, const float width, const float height, const COLORREF color, Texture * const tex, const float intensity, const bool backdrop)
{
   RenderDevice * const pd3dDevice = m_pin3d.m_pd3dPrimaryDevice;

   Vertex3D_NoTex2 vertices[4] =
   {
      { 1.f, 1.f, 0.f, 0.f, 0.f, 1.f, 1.f, 1.f },
      { 0.f, 1.f, 0.f, 0.f, 0.f, 1.f, 0.f, 1.f },
      { 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 1.f, 0.f },
      { 0.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f }
   };

   for (unsigned int i = 0; i < 4; ++i)
   {
      vertices[i].x =        (vertices[i].x * width  + posx)*2.0f - 1.0f;
      vertices[i].y = 1.0f - (vertices[i].y * height + posy)*2.0f;
   }

   pd3dDevice->DMDShader->SetTechnique(tex ? SHADER_TECHNIQUE_basic_noDMD : SHADER_TECHNIQUE_basic_noDMD_notex);

   const vec4 c = convertColor(color, intensity);
   pd3dDevice->DMDShader->SetVector(SHADER_vColor_Intensity, &c);

   if (tex)
      pd3dDevice->DMDShader->SetTexture(SHADER_tex_sprite, tex, SF_NONE, SA_REPEAT, SA_REPEAT);

   pd3dDevice->SetRenderState(RenderState::ZENABLE, RenderState::RS_FALSE);
   pd3dDevice->DrawTexturedQuad(pd3dDevice->DMDShader, vertices);
   pd3dDevice->GetCurrentPass()->m_commands.back()->SetTransparent(true);
   pd3dDevice->GetCurrentPass()->m_commands.back()->SetDepth(-10000.f);
}

void Player::Spritedraw(const float posx, const float posy, const float width, const float height, const COLORREF color, Sampler * const tex, const float intensity, const bool backdrop)
{
   RenderDevice *const pd3dDevice = m_pin3d.m_pd3dPrimaryDevice;

   Vertex3D_NoTex2 vertices[4] =
   {
      { 1.f, 1.f, 0.f, 0.f, 0.f, 1.f, 1.f, 1.f },
      { 0.f, 1.f, 0.f, 0.f, 0.f, 1.f, 0.f, 1.f },
      { 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 1.f, 0.f },
      { 0.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f }
   };

   for (unsigned int i = 0; i < 4; ++i)
   {
      vertices[i].x =        (vertices[i].x * width  + posx)*2.0f - 1.0f;
      vertices[i].y = 1.0f - (vertices[i].y * height + posy)*2.0f;
   }

   pd3dDevice->DMDShader->SetTechnique(tex ? SHADER_TECHNIQUE_basic_noDMD : SHADER_TECHNIQUE_basic_noDMD_notex);

   const vec4 c = convertColor(color, intensity);
   pd3dDevice->DMDShader->SetVector(SHADER_vColor_Intensity, &c);

   if (tex)
      pd3dDevice->DMDShader->SetTexture(SHADER_tex_sprite, tex);

   pd3dDevice->SetRenderState(RenderState::ZENABLE, RenderState::RS_FALSE);
   pd3dDevice->DrawTexturedQuad(pd3dDevice->DMDShader, vertices);
   pd3dDevice->GetCurrentPass()->m_commands.back()->SetTransparent(true);
   pd3dDevice->GetCurrentPass()->m_commands.back()->SetDepth(-10000.f);
}

void Player::DrawBulbLightBuffer()
{
   RenderDevice* p3dDevice = m_pin3d.m_pd3dPrimaryDevice;
   const RenderPass *initial_rt = p3dDevice->GetCurrentPass();
   static int id = 0; id++;

   // switch to 'bloom' output buffer to collect all bulb lights
   p3dDevice->SetRenderTarget("Transmitted Light " + std::to_string(id) + " Clear", p3dDevice->GetBloomBufferTexture(), false);
   p3dDevice->ResetRenderState();
   p3dDevice->Clear(clearType::TARGET, 0, 1.0f, 0L);

   // Draw bulb lights
   m_render_mask |= LIGHT_BUFFER;
   p3dDevice->SetRenderTarget("Transmitted Light " + std::to_string(id), p3dDevice->GetBloomBufferTexture(), true, true);
   p3dDevice->SetRenderState(RenderState::ZENABLE, RenderState::RS_FALSE); // disable all z-tests as zbuffer is in different resolution
   for (Hitable *hitable : m_vhitables)
      if (hitable->HitableGetItemType() == eItemLight)
         hitable->Render(m_render_mask);
   m_render_mask &= ~LIGHT_BUFFER;
   bool hasLight = p3dDevice->GetCurrentPass()->GetCommandCount() > 0;

   if (hasLight)
   { // Only apply blur if we have actually rendered some lights
      RenderPass* renderPass = p3dDevice->GetCurrentPass();
      p3dDevice->DrawGaussianBlur(
         p3dDevice->GetBloomBufferTexture(), 
         p3dDevice->GetBloomTmpBufferTexture(), 
         p3dDevice->GetBloomBufferTexture(), 19.f); // FIXME kernel size should depend on buffer resolution
      RenderPass *blurPass2 = p3dDevice->GetCurrentPass();
      RenderPass *blurPass1 = blurPass2->m_dependencies[0];
      constexpr float margin = 0.05f; // margin for the blur
      blurPass1->m_areaOfInterest.x = renderPass->m_areaOfInterest.x - margin;
      blurPass1->m_areaOfInterest.y = renderPass->m_areaOfInterest.y - margin;
      blurPass1->m_areaOfInterest.z = renderPass->m_areaOfInterest.z + margin;
      blurPass1->m_areaOfInterest.w = renderPass->m_areaOfInterest.w + margin;
      blurPass2->m_areaOfInterest = blurPass1->m_areaOfInterest;
   }

   // Restore state and render target
   p3dDevice->SetRenderTarget(initial_rt->m_name + '+', initial_rt->m_rt);

   #ifndef ENABLE_SDL
   // For some reason, DirectX 9 will not handle correctly the null texture, so we just disable this optimization
   hasLight = true;
   #endif
   if (hasLight)
   {
      // Declare dependency on Bulb Light buffer (actually rendered to the bloom buffer texture)
      m_pin3d.m_pd3dPrimaryDevice->AddRenderTargetDependency(m_pin3d.m_pd3dPrimaryDevice->GetBloomBufferTexture());
      p3dDevice->basicShader->SetTexture(SHADER_tex_base_transmission, p3dDevice->GetBloomBufferTexture()->GetColorSampler());
   } 
   else
   {
      p3dDevice->basicShader->SetTextureNull(SHADER_tex_base_transmission);
   }
}

void Player::RenderDynamics()
{
   PROFILE_FUNCTION(FrameProfiler::PROFILE_GPU_COLLECT);
   TRACE_FUNCTION();

   // Mark all probes to be re-rendered for this frame (only if needed, lazily rendered)
   for (size_t i = 0; i < m_ptable->m_vrenderprobe.size(); ++i)
      m_ptable->m_vrenderprobe[i]->MarkDirty();

   // Setup the projection matrices used for refraction
   Matrix3D matProj[2];
   #ifdef ENABLE_SDL
   const int nEyes = m_pin3d.m_pd3dPrimaryDevice->m_stereo3D != STEREO_OFF ? 2 : 1;
   for (int eye = 0; eye < nEyes; eye++)
      matProj[eye] = m_pin3d.GetMVP().GetProj(eye);
   m_pin3d.m_pd3dPrimaryDevice->basicShader->SetMatrix(SHADER_matProj, &matProj[0], nEyes);
   m_pin3d.m_pd3dPrimaryDevice->m_ballShader->SetMatrix(SHADER_matProj, &matProj[0], nEyes);
   #else
   matProj[0] = m_pin3d.GetMVP().GetProj(0);
   m_pin3d.m_pd3dPrimaryDevice->basicShader->SetMatrix(SHADER_matProj, &matProj[0]);
   m_pin3d.m_pd3dPrimaryDevice->m_ballShader->SetMatrix(SHADER_matProj, &matProj[0]);
   #endif

   // Update ball pos uniforms
   #define MAX_BALL_SHADOW 8
   vec4 balls[MAX_BALL_SHADOW];
   int p = 0;
   for (size_t i = 0; i < m_vball.size() && p < MAX_BALL_SHADOW; i++)
   {
      Ball *const pball = m_vball[i];
      if (!pball->m_visible)
         continue;
      balls[p] = vec4(pball->m_d.m_pos.x, pball->m_d.m_pos.y, pball->m_d.m_pos.z, pball->m_d.m_radius);
      p++;
   }
   for (; p < MAX_BALL_SHADOW; p++)
      balls[p] = vec4(-1000.f, -1000.f, -1000.f, 0.0f);
   m_pin3d.m_pd3dPrimaryDevice->lightShader->SetFloat4v(SHADER_balls, balls, MAX_BALL_SHADOW);
   m_pin3d.m_pd3dPrimaryDevice->basicShader->SetFloat4v(SHADER_balls, balls, MAX_BALL_SHADOW);
   m_pin3d.m_pd3dPrimaryDevice->flasherShader->SetFloat4v(SHADER_balls, balls, MAX_BALL_SHADOW);

   UpdateBasicShaderMatrix();
   UpdateBallShaderMatrix();

   // Render the default backglass without depth write before the table so that it will be visible for tables without a VR backglass but overwriten otherwise
   if (m_pin3d.m_backGlass != nullptr)
      m_pin3d.m_backGlass->Render();

   m_render_mask = IsUsingStaticPrepass() ? DYNAMIC_ONLY : DEFAULT;
   DrawBulbLightBuffer();
   for (Hitable *hitable : m_vhitables)
      hitable->Render(m_render_mask);
   for (Ball* ball : m_vball)
      ball->m_pballex->Render(m_render_mask);
   m_render_mask = DEFAULT;
   
   m_pin3d.m_pd3dPrimaryDevice->basicShader->SetTextureNull(SHADER_tex_base_transmission); // need to reset the bulb light texture, as its used as render target for bloom again

   for (size_t i = 0; i < m_ptable->m_vrenderprobe.size(); ++i)
      m_ptable->m_vrenderprobe[i]->ApplyAreaOfInterest();

   if (!m_liveUI->IsTweakMode())
   {
      mixer_draw(); // Draw the mixer volume
      plumb_draw(); // Debug draw of plumb
   }
}

void Player::SSRefl()
{
   m_pin3d.m_pd3dPrimaryDevice->SetRenderTarget("ScreenSpace Reflection"s, m_pin3d.m_pd3dPrimaryDevice->GetReflectionBufferTexture(), false);
   m_pin3d.m_pd3dPrimaryDevice->AddRenderTargetDependency(m_pin3d.m_pd3dPrimaryDevice->GetBackBufferTexture(), true);

   m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTexture(SHADER_tex_fb_filtered, m_pin3d.m_pd3dPrimaryDevice->GetBackBufferTexture()->GetColorSampler());
   m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTexture(SHADER_tex_fb_unfiltered, m_pin3d.m_pd3dPrimaryDevice->GetBackBufferTexture()->GetColorSampler());
   m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTexture(SHADER_tex_ao_dither, &m_pin3d.m_aoDitherTexture, SF_NONE, SA_REPEAT, SA_REPEAT, true); // FIXME the force linear RGB is not honored in VR

   // FIXME check if size should not be taken from renderdevice to account for VR (double width) or supersampling
   m_pin3d.m_pd3dPrimaryDevice->FBShader->SetVector(SHADER_w_h_height,
      (float)(1.0 / m_pin3d.m_pd3dPrimaryDevice->GetBackBufferTexture()->GetWidth()), (float)(1.0 / m_pin3d.m_pd3dPrimaryDevice->GetBackBufferTexture()->GetHeight()), 1.0f /*radical_inverse(m_overall_frames%2048)*/, 1.0f);

   const float rotation = m_ptable->mViewSetups[m_ptable->m_BG_current_set].GetRotation(m_pin3d.m_pd3dPrimaryDevice->m_width, m_pin3d.m_pd3dPrimaryDevice->m_height);
   const vec4 SSR_bumpHeight_fresnelRefl_scale_FS(0.3f, 0.3f, m_ptable->m_SSRScale, rotation);
   m_pin3d.m_pd3dPrimaryDevice->FBShader->SetVector(SHADER_SSR_bumpHeight_fresnelRefl_scale_FS, &SSR_bumpHeight_fresnelRefl_scale_FS);

   m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTechnique(SHADER_TECHNIQUE_SSReflection);

   m_pin3d.m_pd3dPrimaryDevice->DrawFullscreenTexturedQuad(m_pin3d.m_pd3dPrimaryDevice->FBShader);
}

void Player::Bloom()
{
   if (m_ptable->m_bloom_strength <= 0.0f || m_bloomOff || GetInfoMode() == IF_LIGHT_BUFFER_ONLY)
      return;

   const double w = (double)m_pin3d.m_pd3dPrimaryDevice->GetBackBufferTexture()->GetWidth();
   const double h = (double)m_pin3d.m_pd3dPrimaryDevice->GetBackBufferTexture()->GetHeight();
   const Vertex3D_TexelOnly shiftedVerts[4] =
   {
      {  1.0f,  1.0f, 0.0f, 1.0f + (float)(2.25 / w), 0.0f + (float)(2.25 / h) },
      { -1.0f,  1.0f, 0.0f, 0.0f + (float)(2.25 / w), 0.0f + (float)(2.25 / h) },
      {  1.0f, -1.0f, 0.0f, 1.0f + (float)(2.25 / w), 1.0f + (float)(2.25 / h) },
      { -1.0f, -1.0f, 0.0f, 0.0f + (float)(2.25 / w), 1.0f + (float)(2.25 / h) }
   };
   {
      m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTextureNull(SHADER_tex_fb_filtered);

      // switch to 'bloom' output buffer to collect clipped framebuffer values
      m_pin3d.m_pd3dPrimaryDevice->SetRenderTarget("Bloom Cut Off"s, m_pin3d.m_pd3dPrimaryDevice->GetBloomBufferTexture(), false);
      m_pin3d.m_pd3dPrimaryDevice->AddRenderTargetDependency(m_pin3d.m_pd3dPrimaryDevice->GetBackBufferTexture());

      m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTexture(SHADER_tex_fb_filtered, m_pin3d.m_pd3dPrimaryDevice->GetBackBufferTexture()->GetColorSampler());
      m_pin3d.m_pd3dPrimaryDevice->FBShader->SetVector(SHADER_w_h_height, (float) (1.0 / w), (float) (1.0 / h), m_ptable->m_bloom_strength, 1.0f);
      m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTechnique(SHADER_TECHNIQUE_fb_bloom);

      m_pin3d.m_pd3dPrimaryDevice->DrawTexturedQuad(m_pin3d.m_pd3dPrimaryDevice->FBShader, shiftedVerts);
   }

   m_pin3d.m_pd3dPrimaryDevice->DrawGaussianBlur(
      m_pin3d.m_pd3dPrimaryDevice->GetBloomBufferTexture(), 
      m_pin3d.m_pd3dPrimaryDevice->GetBloomTmpBufferTexture(),
      m_pin3d.m_pd3dPrimaryDevice->GetBloomBufferTexture(), 39.f); // FIXME kernel size should depend on buffer resolution
}

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

   if (g_frameProfiler.GetPrev(FrameProfiler::PROFILE_PHYSICS) > m_phys_max || m_time_msec - m_lastMaxChangeTime > 1000)
      m_phys_max = g_frameProfiler.GetPrev(FrameProfiler::PROFILE_PHYSICS);

   if (m_phys_iterations > m_phys_max_iterations || m_time_msec - m_lastMaxChangeTime > 1000)
      m_phys_max_iterations = m_phys_iterations;

   if (g_frameProfiler.GetPrev(FrameProfiler::PROFILE_SCRIPT) > m_script_max || m_time_msec - m_lastMaxChangeTime > 1000)
      m_script_max = g_frameProfiler.GetPrev(FrameProfiler::PROFILE_SCRIPT);

   if (m_time_msec - m_lastMaxChangeTime > 1000)
      m_lastMaxChangeTime = m_time_msec;

   if (m_count == 0)
   {
      m_phys_total_iterations = m_phys_iterations;
      m_count = 1;
   }
   else
   {
      m_phys_total_iterations += m_phys_iterations;
      m_count++;
   }

   // Renderer additional information
   info << "Triangles: " << ((m_pin3d.m_pd3dPrimaryDevice->m_frameDrawnTriangles + 999) / 1000) << "k per frame, "
        << ((stats_drawn_static_triangles + m_pin3d.m_pd3dPrimaryDevice->m_frameDrawnTriangles + 999) / 1000) << "k overall. DayNight " << quantizeUnsignedPercent(m_globalEmissionScale)
        << "%%\n";
   info << "Draw calls: " << m_pin3d.m_pd3dPrimaryDevice->Perf_GetNumDrawCalls() << "  (" << m_pin3d.m_pd3dPrimaryDevice->Perf_GetNumLockCalls() << " Locks)\n";
   info << "State changes: " << m_pin3d.m_pd3dPrimaryDevice->Perf_GetNumStateChanges() << "\n";
   info << "Texture changes: " << m_pin3d.m_pd3dPrimaryDevice->Perf_GetNumTextureChanges() << " (" << m_pin3d.m_pd3dPrimaryDevice->Perf_GetNumTextureUploads() << " Uploads)\n";
   info << "Shader/Parameter changes: " << m_pin3d.m_pd3dPrimaryDevice->Perf_GetNumTechniqueChanges() << " / " << m_pin3d.m_pd3dPrimaryDevice->Perf_GetNumParameterChanges() << "\n";
   info << "Objects: " << (unsigned int)m_vhitables.size() << "\n";
   info << "\n";

   // Physics additional information
   info << "Physics: " << m_phys_iterations << " iterations per frame (" << ((U32)(m_phys_total_iterations / m_count)) << " avg " << m_phys_max_iterations
        << " max)    Ball Velocity / Ang.Vel.: " << (m_pactiveball ? (m_pactiveball->m_d.m_vel + (float)PHYS_FACTOR * m_gravity).Length() : -1.f) << " "
        << (m_pactiveball ? (m_pactiveball->m_angularmomentum / m_pactiveball->Inertia()).Length() : -1.f) << "\n";
#ifdef DEBUGPHYSICS
   info << std::setprecision(5);
   info << "Hits:" << c_hitcnts << " Collide:" << c_collisioncnt << " Ctacs:" << c_contactcnt;
#ifdef C_DYNAMIC
   info << " Static:" << c_staticcnt;
#endif
   info << " Embed:" << c_embedcnts << " TimeSearch:" << c_timesearch << "\n";
   info << "kDObjects:" << c_kDObjects << " kD:" << c_kDNextlevels << " QuadObjects:" << c_quadObjects << " Quadtree:" << c_quadNextlevels << " Traversed:" << c_traversed
        << " Tested:" << c_tested << " DeepTested:" << c_deepTested << "\n";
   info << std::setprecision(1);
#endif

   info << "Left Flipper keypress to rotate: "
      << ((INT64)(m_pininput.m_leftkey_down_usec_rotate_to_end - m_pininput.m_leftkey_down_usec) < 0 ? int_as_float(0x7FC00000) : (double)(m_pininput.m_leftkey_down_usec_rotate_to_end - m_pininput.m_leftkey_down_usec) / 1000.) << " ms ("
      << ((int)(m_pininput.m_leftkey_down_frame_rotate_to_end - m_pininput.m_leftkey_down_frame) < 0 ? -1 : (int)(m_pininput.m_leftkey_down_frame_rotate_to_end - m_pininput.m_leftkey_down_frame)) << " f) to eos: "
      << ((INT64)(m_pininput.m_leftkey_down_usec_EOS - m_pininput.m_leftkey_down_usec) < 0 ? int_as_float(0x7FC00000) : (double)(m_pininput.m_leftkey_down_usec_EOS - m_pininput.m_leftkey_down_usec) / 1000.) << " ms ("
      << ((int)(m_pininput.m_leftkey_down_frame_EOS - m_pininput.m_leftkey_down_frame) < 0 ? -1 : (int)(m_pininput.m_leftkey_down_frame_EOS - m_pininput.m_leftkey_down_frame)) << " f)\n";

   // Draw performance readout - at end of CPU frame, so hopefully the previous frame
   //  (whose data we're getting) will have finished on the GPU by now.
   #ifndef ENABLE_SDL // No GPU profiler for OpenGL
   if (GetProfilingMode() != PF_DISABLED && m_closing == CS_PLAYING)
   {
      info << "\n";
      info << "Detailed (approximate) GPU profiling:\n";

      m_pin3d.m_gpu_profiler.WaitForDataAndUpdate();

      double dTDrawTotal = 0.0;
      for (GTS gts = GTS_BeginFrame; gts < GTS_EndFrame; gts = GTS(gts + 1))
         dTDrawTotal += m_pin3d.m_gpu_profiler.DtAvg(gts);

      info << std::setw(4) << std::setprecision(2);
      if (GetProfilingMode() == PF_ENABLED)
      {
         info << " Draw time: " << float(1000.0 * dTDrawTotal) << " ms\n";
         for (GTS gts = GTS(GTS_BeginFrame + 1); gts < GTS_EndFrame; gts = GTS(gts + 1))
         {
            info << "   " << GTS_name[gts] << ": " << float(1000.0 * m_pin3d.m_gpu_profiler.DtAvg(gts)) << " ms (" << float(100. * m_pin3d.m_gpu_profiler.DtAvg(gts) / dTDrawTotal) << "%%)\n";
         }
         info << " Frame time: " << float(1000.0 * (dTDrawTotal + m_pin3d.m_gpu_profiler.DtAvg(GTS_EndFrame))) << " ms\n";
      }
      else
      {
         for (GTS gts = GTS(GTS_BeginFrame + 1); gts < GTS_EndFrame; gts = GTS(gts + 1))
         {
            info << " " << GTS_name_item[gts] << ": " << float(1000.0 * m_pin3d.m_gpu_profiler.DtAvg(gts)) << " ms (" << float(100. * m_pin3d.m_gpu_profiler.DtAvg(gts) / dTDrawTotal)
                 << "%%)\n";
         }
      }
   }
   #endif

   txt = info.str();

   return txt;
}

void Player::PrepareVideoBuffers()
{
   const bool useAA = m_AAfactor > 1.0f;
   const bool stereo = m_stereo3D == STEREO_VR || ((m_stereo3D != STEREO_OFF) && m_stereo3Denabled && (!m_stereo3DfakeStereo || m_pin3d.m_pd3dPrimaryDevice->DepthBufferReadBackAvailable()));
   // Since stereo is applied as a postprocess step for fake stereo, it disables AA and sharpening except for top/bottom & side by side modes
   const bool PostProcAA = !m_stereo3DfakeStereo || (!stereo || (m_stereo3D == STEREO_TB) || (m_stereo3D == STEREO_SBS));
   const bool SMAA  = PostProcAA && m_FXAA == Quality_SMAA;
   const bool DLAA  = PostProcAA && m_FXAA == Standard_DLAA;
   const bool NFAA  = PostProcAA && m_FXAA == Fast_NFAA;
   const bool FXAA1 = PostProcAA && m_FXAA == Fast_FXAA;
   const bool FXAA2 = PostProcAA && m_FXAA == Standard_FXAA;
   const bool FXAA3 = PostProcAA && m_FXAA == Quality_FXAA;
   const bool ss_refl = m_ss_refl && m_ptable->m_enableSSR && m_pin3d.m_pd3dPrimaryDevice->DepthBufferReadBackAvailable() && m_ptable->m_SSRScale > 0.f;
   const unsigned int sharpen = PostProcAA ? m_sharpen : 0;
   const bool useAO = GetAOMode() == 2;
   const bool useUpscaler = (m_AAfactor < 1.0f) && !stereo && (SMAA || DLAA || NFAA || FXAA1 || FXAA2 || FXAA3 || sharpen);

   RenderTarget *renderedRT = m_pin3d.m_pd3dPrimaryDevice->GetBackBufferTexture();
   RenderTarget *outputRT = nullptr;
   m_pin3d.m_pd3dPrimaryDevice->ResetRenderState();
   m_pin3d.m_pd3dPrimaryDevice->SetRenderState(RenderState::ALPHABLENDENABLE, RenderState::RS_FALSE);
   m_pin3d.m_pd3dPrimaryDevice->SetRenderState(RenderState::CULLMODE, RenderState::CULL_NONE);
   m_pin3d.m_pd3dPrimaryDevice->SetRenderState(RenderState::ZWRITEENABLE, RenderState::RS_FALSE);
   m_pin3d.m_pd3dPrimaryDevice->SetRenderState(RenderState::ZENABLE, RenderState::RS_FALSE);

   // All postprocess that uses depth sample it from the MSAA resolved rendered backbuffer
   m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTexture(SHADER_tex_depth, m_pin3d.m_pd3dPrimaryDevice->GetBackBufferTexture()->GetDepthSampler());

   // Compute bloom (to be applied later)
   Bloom();
   if (GetProfilingMode() == PF_ENABLED)
      m_pin3d.m_gpu_profiler.Timestamp(GTS_Bloom);

   // Add screen space reflections
   if (ss_refl)
   {
      SSRefl();
      renderedRT = m_pin3d.m_pd3dPrimaryDevice->GetReflectionBufferTexture();
   }
   if (GetProfilingMode() == PF_ENABLED)
      m_pin3d.m_gpu_profiler.Timestamp(GTS_SSR);

   // Compute AO contribution (to be applied later)
   if (useAO)
   {
      // separate normal generation pass, currently roughly same perf or even much worse
      /* m_pin3d.m_pd3dPrimaryDevice->SetRenderTarget(m_pin3d.m_pd3dDevice->GetPostProcessRenderTarget1()); //!! expects stereo or FXAA enabled
      m_pin3d.m_pd3dDevice->FBShader->SetTexture(SHADER_tex_depth, m_pin3d.m_pdds3DZBuffer, true);
      m_pin3d.m_pd3dDevice->FBShader->SetVector(SHADER_w_h_height, (float)(1.0 / m_width), (float)(1.0 / m_height),
         radical_inverse(m_overall_frames%2048)*(float)(1. / 8.0), sobol(m_overall_frames%2048)*(float)(5. / 8.0));// jitter within lattice cell //!! ?
      m_pin3d.m_pd3dDevice->FBShader->SetTechnique("normals");
      m_pin3d.m_pd3dDevice->DrawFullscreenTexturedQuad(m_pin3d.m_pd3dPrimaryDevice->FBShader);*/

      m_pin3d.m_pd3dPrimaryDevice->SetRenderTarget("ScreenSpace AO"s, m_pin3d.m_pd3dPrimaryDevice->GetAORenderTarget(0), false);
      m_pin3d.m_pd3dPrimaryDevice->AddRenderTargetDependency(m_pin3d.m_pd3dPrimaryDevice->GetAORenderTarget(1));
      m_pin3d.m_pd3dPrimaryDevice->AddRenderTargetDependency(m_pin3d.m_pd3dPrimaryDevice->GetBackBufferTexture());
      m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTexture(SHADER_tex_fb_filtered, m_pin3d.m_pd3dPrimaryDevice->GetAORenderTarget(1)->GetColorSampler());
      //m_pin3d.m_pd3dDevice->FBShader->SetTexture(SHADER_Texture1, m_pin3d.m_pd3dDevice->GetPostProcessRenderTarget1()); // temporary normals
      m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTexture(SHADER_tex_ao_dither, &m_pin3d.m_aoDitherTexture, SF_NONE, SA_REPEAT, SA_REPEAT, true); // FIXME the force linear RGB is not honored
      m_pin3d.m_pd3dPrimaryDevice->FBShader->SetVector(SHADER_w_h_height, 
         (float)(1.0 / m_pin3d.m_pd3dPrimaryDevice->GetAORenderTarget(1)->GetWidth()),
         (float)(1.0 / m_pin3d.m_pd3dPrimaryDevice->GetAORenderTarget(1)->GetHeight()), 
         radical_inverse(m_overall_frames % 2048) * (float)(1. / 8.0),
         /*sobol*/ radical_inverse<3>(m_overall_frames % 2048) * (float)(1. / 8.0)); // jitter within (64/8)x(64/8) neighborhood of 64x64 tex, good compromise between blotches and noise
      m_pin3d.m_pd3dPrimaryDevice->FBShader->SetVector(SHADER_AO_scale_timeblur, m_ptable->m_AOScale, 0.4f, 0.f, 0.f); //!! 0.4f: fake global option in video pref? or time dependent? //!! commonly used is 0.1, but would require to clear history for moving stuff
      m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTechnique(SHADER_TECHNIQUE_AO);
      m_pin3d.m_pd3dPrimaryDevice->DrawFullscreenTexturedQuad(m_pin3d.m_pd3dPrimaryDevice->FBShader);

      // flip AO buffers (avoids copy)
      m_pin3d.m_pd3dPrimaryDevice->SwapAORenderTargets();
   }
   if (GetProfilingMode() == PF_ENABLED)
      m_pin3d.m_gpu_profiler.Timestamp(GTS_AO);

   // Perform color grade LUT / dither / tonemapping, also applying bloom and AO
   {
      // switch to output buffer (main output frame buffer, or a temporary one for postprocessing)
      if (SMAA || DLAA || NFAA || FXAA1 || FXAA2 || FXAA3 || sharpen || stereo || useUpscaler)
         outputRT = m_pin3d.m_pd3dPrimaryDevice->GetPostProcessRenderTarget1();
      else
         outputRT = m_pin3d.m_pd3dPrimaryDevice->GetOutputBackBuffer();
      m_pin3d.m_pd3dPrimaryDevice->SetRenderTarget("Tonemap/Dither/ColorGrade"s, outputRT, false);
      m_pin3d.m_pd3dPrimaryDevice->AddRenderTargetDependency(renderedRT, useAO);

      int render_w = renderedRT->GetWidth(), render_h = renderedRT->GetHeight();
      m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTexture(SHADER_tex_fb_unfiltered, renderedRT->GetColorSampler());
      m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTexture(SHADER_tex_fb_filtered, renderedRT->GetColorSampler());
      if (m_ptable->m_bloom_strength > 0.0f && !m_bloomOff)
      {
         m_pin3d.m_pd3dPrimaryDevice->AddRenderTargetDependency(m_pin3d.m_pd3dPrimaryDevice->GetBloomBufferTexture());
         m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTexture(SHADER_tex_bloom, m_pin3d.m_pd3dPrimaryDevice->GetBloomBufferTexture()->GetColorSampler());
      }
      if (useAO)
      {
         m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTexture(SHADER_tex_ao, m_pin3d.m_pd3dPrimaryDevice->GetAORenderTarget(1)->GetColorSampler());
         m_pin3d.m_pd3dPrimaryDevice->AddRenderTargetDependency(m_pin3d.m_pd3dPrimaryDevice->GetAORenderTarget(1));
      }

      // For information mode, override with the wanted render target instead of the render buffer
      const InfoMode infoMode = GetInfoMode();
      if (infoMode == IF_RENDER_PROBES)
      {
         RenderProbe *render_probe = m_ptable->m_vrenderprobe[m_infoProbeIndex];
         RenderTarget *probe = render_probe->Render(0);
         if (probe)
         {
            m_pin3d.m_pd3dPrimaryDevice->AddRenderTargetDependency(probe);
            m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTexture(SHADER_tex_fb_unfiltered, probe->GetColorSampler());
            m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTexture(SHADER_tex_fb_filtered, probe->GetColorSampler());
            render_w = probe->GetWidth();
            render_h = probe->GetHeight();
         }
      }
      else if (infoMode == IF_LIGHT_BUFFER_ONLY)
      {
         renderedRT = m_pin3d.m_pd3dPrimaryDevice->GetBloomBufferTexture();
         m_pin3d.m_pd3dPrimaryDevice->AddRenderTargetDependency(renderedRT);
         m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTexture(SHADER_tex_fb_unfiltered, renderedRT->GetColorSampler());
         m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTexture(SHADER_tex_fb_filtered, renderedRT->GetColorSampler());
         render_w = renderedRT->GetWidth();
         render_h = renderedRT->GetHeight();
      }

      // Texture used for LUT color grading must be treated as if they were linear
      Texture *const pin = m_ptable->GetImage(m_ptable->m_imageColorGrade);
      if (pin)
         m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTexture(SHADER_tex_color_lut, pin, SF_BILINEAR, SA_CLAMP, SA_CLAMP, true); // FIXME always honor the linear RGB
      m_pin3d.m_pd3dPrimaryDevice->FBShader->SetBool(SHADER_color_grade, pin != nullptr);
      m_pin3d.m_pd3dPrimaryDevice->FBShader->SetBool(SHADER_do_dither, m_pin3d.m_pd3dPrimaryDevice->GetOutputBackBuffer()->GetColorFormat() != colorFormat::RGBA10);
      m_pin3d.m_pd3dPrimaryDevice->FBShader->SetBool(SHADER_do_bloom, (m_ptable->m_bloom_strength > 0.0f && !m_bloomOff && infoMode <= IF_DYNAMIC_ONLY));

      //const unsigned int jittertime = (unsigned int)((U64)msec()*90/1000);
      const float jitter = (float)((msec()&2047)/1000.0);
      m_pin3d.m_pd3dPrimaryDevice->FBShader->SetVector(SHADER_w_h_height, 
         (float)(1.0 / (double)render_w), (float)(1.0 / (double)render_h), //1.0f, 1.0f);
         jitter, // radical_inverse(jittertime) * 11.0f,
         jitter); // sobol(jittertime) * 13.0f); // jitter for dither pattern

      if (infoMode == IF_AO_ONLY)
         m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTechnique(SHADER_TECHNIQUE_fb_AO);
      else if (infoMode == IF_RENDER_PROBES)
         m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTechnique(m_toneMapper == TM_REINHARD ? SHADER_TECHNIQUE_fb_rhtonemap
                                                           : m_toneMapper == TM_FILMIC   ? SHADER_TECHNIQUE_fb_fmtonemap
                                                                                         : SHADER_TECHNIQUE_fb_tmtonemap);
      else if (m_BWrendering != 0)
         m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTechnique(m_BWrendering == 1 ? SHADER_TECHNIQUE_fb_rhtonemap_no_filterRG : SHADER_TECHNIQUE_fb_rhtonemap_no_filterR);
      else if (m_toneMapper == TM_REINHARD)
         m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTechnique(useAO ?
              useAA ? SHADER_TECHNIQUE_fb_rhtonemap_AO : SHADER_TECHNIQUE_fb_rhtonemap_AO_no_filter
            : useAA ? SHADER_TECHNIQUE_fb_rhtonemap    : SHADER_TECHNIQUE_fb_rhtonemap_no_filter);
      else if (m_toneMapper == TM_FILMIC)
         m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTechnique(useAO ?
              useAA ? SHADER_TECHNIQUE_fb_fmtonemap_AO : SHADER_TECHNIQUE_fb_fmtonemap_AO_no_filter
            : useAA ? SHADER_TECHNIQUE_fb_fmtonemap    : SHADER_TECHNIQUE_fb_fmtonemap_no_filter);
      else // TM_TONY_MC_MAPFACE
         m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTechnique(useAO ?
              useAA ? SHADER_TECHNIQUE_fb_tmtonemap_AO : SHADER_TECHNIQUE_fb_tmtonemap_AO_no_filter
            : useAA ? SHADER_TECHNIQUE_fb_tmtonemap    : SHADER_TECHNIQUE_fb_tmtonemap_no_filter);

      const Vertex3D_TexelOnly shiftedVerts[4] =
      {
         {  1.0f + m_ScreenOffset.x,  1.0f + m_ScreenOffset.y, 0.0f, 1.0f, 0.0f },
         { -1.0f + m_ScreenOffset.x,  1.0f + m_ScreenOffset.y, 0.0f, 0.0f, 0.0f },
         {  1.0f + m_ScreenOffset.x, -1.0f + m_ScreenOffset.y, 0.0f, 1.0f, 1.0f },
         { -1.0f + m_ScreenOffset.x, -1.0f + m_ScreenOffset.y, 0.0f, 0.0f, 1.0f }
      };
      m_pin3d.m_pd3dPrimaryDevice->DrawTexturedQuad(m_pin3d.m_pd3dPrimaryDevice->FBShader, shiftedVerts);
      renderedRT = outputRT;
   }

   // This code allows to check that the FB shader does perform pixel perfect processing (1 to 1 match between renderedRT and outputRT)
   // This needs a modification of the shader to used the filtered texture (tex_fb_filtered) instead of unfiltered
   if (false)
   {
      BaseTexture *tex = new BaseTexture(renderedRT->GetWidth(), renderedRT->GetHeight(), BaseTexture::RGB);
      BYTE *const __restrict pdest = tex->data();
      for (size_t i = 0; i < (size_t)renderedRT->GetWidth() * renderedRT->GetHeight(); ++i)
      {
         size_t y = i / renderedRT->GetWidth();
         #ifdef ENABLE_SDL
         y = renderedRT->GetHeight() - 1 - y;
         #endif
         pdest[i * 3 + 0] = ((i & 1) == 0 && (y & 1) == 0) ? 0x00 : 0xFF;
         pdest[i * 3 + 1] = ((i >> 2) & 1) == 0 ? 0x00 : ((i & 1) == 0 && (y & 1) == 0) ? 0x00 : 0xFF;
         pdest[i * 3 + 2] = ((y >> 2) & 1) == 0 ? 0x00 : ((i & 1) == 0 && (y & 1) == 0) ? 0x00 : 0xFF;
      }
      Sampler *checker = new Sampler(m_pin3d.m_pd3dPrimaryDevice, tex, true, SA_CLAMP, SA_CLAMP, SF_NONE);
      m_pin3d.m_pd3dPrimaryDevice->FBShader->SetVector(SHADER_w_h_height, (float)(1.0 / renderedRT->GetWidth()), (float)(1.0 / renderedRT->GetHeight()), 1.f, 1.f);
      m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTexture(SHADER_tex_fb_filtered, checker);
      m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTechnique(SHADER_TECHNIQUE_fb_mirror);
      m_pin3d.m_pd3dPrimaryDevice->DrawFullscreenTexturedQuad(m_pin3d.m_pd3dPrimaryDevice->FBShader);
      renderedRT = outputRT;
      delete checker;
      delete tex;
   }

   // Stereo and AA are performed on LDR render buffer after tonemapping (RGB8 or RGB10, but nof RGBF).
   // We ping pong between BackBufferTmpTexture and BackBufferTmpTexture2 for the different postprocess
   // SMAA is a special case since it needs 3 passes, so it uses GetBackBufferTexture also (which is somewhat overkill since it is RGB16F)
   assert(renderedRT == m_pin3d.m_pd3dPrimaryDevice->GetOutputBackBuffer() || renderedRT == m_pin3d.m_pd3dPrimaryDevice->GetPostProcessRenderTarget1());

   // Perform post processed anti aliasing
   if (NFAA || FXAA1 || FXAA2 || FXAA3)
   {
      assert(renderedRT == m_pin3d.m_pd3dPrimaryDevice->GetPostProcessRenderTarget1());
      outputRT = sharpen || stereo || useUpscaler ? m_pin3d.m_pd3dPrimaryDevice->GetPostProcessRenderTarget(renderedRT) : m_pin3d.m_pd3dPrimaryDevice->GetOutputBackBuffer();
      m_pin3d.m_pd3dPrimaryDevice->SetRenderTarget(SMAA ? "SMAA Color/Edge Detection"s : "Post Process AA Pass 1"s, outputRT, false);
      m_pin3d.m_pd3dPrimaryDevice->AddRenderTargetDependency(renderedRT);
      m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTexture(SHADER_tex_fb_filtered, renderedRT->GetColorSampler());
      m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTexture(SHADER_tex_fb_unfiltered, renderedRT->GetColorSampler());
      m_pin3d.m_pd3dPrimaryDevice->AddRenderTargetDependency(m_pin3d.m_pd3dPrimaryDevice->GetBackBufferTexture(), true); // Depth is always taken from the MSAA resolved render buffer
      m_pin3d.m_pd3dPrimaryDevice->FBShader->SetVector(SHADER_w_h_height, (float)(1.0 / renderedRT->GetWidth()), (float)(1.0 / renderedRT->GetHeight()), (float)renderedRT->GetWidth(), 1.f);
      m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTechnique(NFAA  ? SHADER_TECHNIQUE_NFAA : FXAA3 ? SHADER_TECHNIQUE_FXAA3 : FXAA2 ? SHADER_TECHNIQUE_FXAA2 : SHADER_TECHNIQUE_FXAA1);
      m_pin3d.m_pd3dPrimaryDevice->DrawFullscreenTexturedQuad(m_pin3d.m_pd3dPrimaryDevice->FBShader);
      renderedRT = outputRT;
   }
   else if (DLAA)
   {
      assert(renderedRT == m_pin3d.m_pd3dPrimaryDevice->GetPostProcessRenderTarget1());
      // First pass detect edges and write it to alpha channel (keeping RGB)
      outputRT = m_pin3d.m_pd3dPrimaryDevice->GetPostProcessRenderTarget(renderedRT);
      m_pin3d.m_pd3dPrimaryDevice->SetRenderTarget("DLAA Edge Detection"s, outputRT, false);
      m_pin3d.m_pd3dPrimaryDevice->AddRenderTargetDependency(renderedRT);
      m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTexture(SHADER_tex_fb_filtered, renderedRT->GetColorSampler());
      m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTexture(SHADER_tex_fb_unfiltered, renderedRT->GetColorSampler());
      m_pin3d.m_pd3dPrimaryDevice->AddRenderTargetDependency(m_pin3d.m_pd3dPrimaryDevice->GetBackBufferTexture(), true); // Depth is always taken from the MSAA resolved render buffer
      m_pin3d.m_pd3dPrimaryDevice->FBShader->SetVector(SHADER_w_h_height, (float)(1.0 / renderedRT->GetWidth()), (float)(1.0 / renderedRT->GetHeight()), (float)renderedRT->GetWidth(), 1.f);
      m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTechnique(SHADER_TECHNIQUE_DLAA_edge);
      m_pin3d.m_pd3dPrimaryDevice->DrawFullscreenTexturedQuad(m_pin3d.m_pd3dPrimaryDevice->FBShader);
      renderedRT = outputRT;

      // Second pass: use edge detection from first pass (alpha channel) and RGB colors for actual filtering
      outputRT = sharpen || stereo || useUpscaler ? m_pin3d.m_pd3dPrimaryDevice->GetPostProcessRenderTarget(renderedRT) : m_pin3d.m_pd3dPrimaryDevice->GetOutputBackBuffer();
      m_pin3d.m_pd3dPrimaryDevice->SetRenderTarget("DLAA Neigborhood blending"s, outputRT, false);
      m_pin3d.m_pd3dPrimaryDevice->AddRenderTargetDependency(renderedRT);
      m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTexture(SHADER_tex_fb_filtered, renderedRT->GetColorSampler());
      m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTexture(SHADER_tex_fb_unfiltered, renderedRT->GetColorSampler());
      m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTechnique(SHADER_TECHNIQUE_DLAA);
      m_pin3d.m_pd3dPrimaryDevice->DrawFullscreenTexturedQuad(m_pin3d.m_pd3dPrimaryDevice->FBShader);
      renderedRT = outputRT;
   }
   else if (SMAA)
   {
      assert(renderedRT == m_pin3d.m_pd3dPrimaryDevice->GetPostProcessRenderTarget1());
      // SMAA use 3 passes, all of them using the initial render, so since tonemap use postprocess RT 1, we use the back buffer and post process RT 2
      RenderTarget *sourceRT = renderedRT;
      m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTexture(SHADER_tex_fb_filtered, sourceRT->GetColorSampler());
      m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTexture(SHADER_tex_fb_unfiltered, sourceRT->GetColorSampler());
      m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTexture(SHADER_areaTex, m_pin3d.m_pd3dPrimaryDevice->m_SMAAareaTexture);
      m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTexture(SHADER_searchTex, m_pin3d.m_pd3dPrimaryDevice->m_SMAAsearchTexture);
      m_pin3d.m_pd3dPrimaryDevice->FBShader->SetVector(SHADER_w_h_height, (float)(1.0 / sourceRT->GetWidth()), (float)(1.0 / sourceRT->GetHeight()), (float)sourceRT->GetWidth(), (float)sourceRT->GetHeight());

      outputRT = m_pin3d.m_pd3dPrimaryDevice->GetPreviousBackBufferTexture(); // We don't need it anymore, so use it as a third postprocess buffer
      m_pin3d.m_pd3dPrimaryDevice->SetRenderTarget("SMAA Color/Edge Detection"s, outputRT, false);
      m_pin3d.m_pd3dPrimaryDevice->AddRenderTargetDependency(sourceRT); // PostProcess RT 1
      m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTechnique(SHADER_TECHNIQUE_SMAA_ColorEdgeDetection);
      m_pin3d.m_pd3dPrimaryDevice->DrawFullscreenTexturedQuad(m_pin3d.m_pd3dPrimaryDevice->FBShader);
      renderedRT = outputRT;

      outputRT = m_pin3d.m_pd3dPrimaryDevice->GetPostProcessRenderTarget(sourceRT);
      m_pin3d.m_pd3dPrimaryDevice->SetRenderTarget("SMAA Blend weight calculation"s, outputRT, false);
      m_pin3d.m_pd3dPrimaryDevice->AddRenderTargetDependency(sourceRT); // PostProcess RT 1
      m_pin3d.m_pd3dPrimaryDevice->AddRenderTargetDependency(renderedRT); // BackBuffer RT
      m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTexture(SHADER_edgesTex, renderedRT->GetColorSampler());
      m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTechnique(SHADER_TECHNIQUE_SMAA_BlendWeightCalculation);
      m_pin3d.m_pd3dPrimaryDevice->DrawFullscreenTexturedQuad(m_pin3d.m_pd3dPrimaryDevice->FBShader);
      renderedRT = outputRT;

      outputRT = sharpen || stereo || useUpscaler ? m_pin3d.m_pd3dPrimaryDevice->GetPreviousBackBufferTexture() : m_pin3d.m_pd3dPrimaryDevice->GetOutputBackBuffer();
      m_pin3d.m_pd3dPrimaryDevice->SetRenderTarget("SMAA Neigborhood blending"s, outputRT, false);
      m_pin3d.m_pd3dPrimaryDevice->AddRenderTargetDependency(sourceRT); // PostProcess RT 1
      m_pin3d.m_pd3dPrimaryDevice->AddRenderTargetDependency(renderedRT); // PostProcess RT 2
      m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTexture(SHADER_blendTex, renderedRT->GetColorSampler());
      m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTechnique(SHADER_TECHNIQUE_SMAA_NeighborhoodBlending);
      m_pin3d.m_pd3dPrimaryDevice->DrawFullscreenTexturedQuad(m_pin3d.m_pd3dPrimaryDevice->FBShader);
      renderedRT = outputRT;
   }

   // Performs sharpening
   if (sharpen)
   {
      assert(renderedRT != m_pin3d.m_pd3dPrimaryDevice->GetOutputBackBuffer()); // At this point, renderedRT may be PP1, PP2 or backbuffer
      outputRT = stereo || useUpscaler ? m_pin3d.m_pd3dPrimaryDevice->GetPostProcessRenderTarget(renderedRT) : m_pin3d.m_pd3dPrimaryDevice->GetOutputBackBuffer();
      m_pin3d.m_pd3dPrimaryDevice->SetRenderTarget("Sharpen"s, outputRT, false);
      m_pin3d.m_pd3dPrimaryDevice->AddRenderTargetDependency(renderedRT);
      m_pin3d.m_pd3dPrimaryDevice->AddRenderTargetDependency(m_pin3d.m_pd3dPrimaryDevice->GetBackBufferTexture(), true); // Depth is always taken from the MSAA resolved render buffer
      m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTexture(SHADER_tex_fb_filtered, renderedRT->GetColorSampler());
      m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTexture(SHADER_tex_fb_unfiltered, renderedRT->GetColorSampler());
      m_pin3d.m_pd3dPrimaryDevice->FBShader->SetVector(SHADER_w_h_height, (float)(1.0 / renderedRT->GetWidth()), (float)(1.0 / renderedRT->GetHeight()), (float)renderedRT->GetWidth(), 1.f);
      m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTechnique((sharpen == 1) ? SHADER_TECHNIQUE_CAS : SHADER_TECHNIQUE_BilateralSharp_CAS);
      m_pin3d.m_pd3dPrimaryDevice->DrawFullscreenTexturedQuad(m_pin3d.m_pd3dPrimaryDevice->FBShader);
      renderedRT = outputRT;
   }

   // Apply stereo
   if (stereo)
   {
      #if defined(ENABLE_SDL) && defined(ENABLE_VR)
      // For STEREO_OFF, STEREO_TB, STEREO_SBS, this won't do anything. The previous postprocess steps should already have written to OutputBackBuffer
      // For VR, copy each eye to the HMD texture and render the wanted preview if activated
      if (m_stereo3D == STEREO_VR)
      {
         assert(renderedRT != m_pin3d.m_pd3dPrimaryDevice->GetOutputBackBuffer());
         int w = renderedRT->GetWidth(), h = renderedRT->GetHeight();
         
         RenderTarget *leftTexture = m_pin3d.m_pd3dPrimaryDevice->GetOffscreenVR(0);
         m_pin3d.m_pd3dPrimaryDevice->SetRenderTarget("Left Eye"s, leftTexture, false);
         m_pin3d.m_pd3dPrimaryDevice->AddRenderTargetDependency(renderedRT);
         m_pin3d.m_pd3dPrimaryDevice->BlitRenderTarget(renderedRT, leftTexture, true, false, 0, 0, w, h, 0, 0, w, h, 0, 0);
         if (m_liveUI->IsTweakMode()) // Render as an overlay in VR (not in preview) at the eye resolution, without depth
            m_pin3d.m_pd3dPrimaryDevice->RenderLiveUI();

         RenderTarget *rightTexture = m_pin3d.m_pd3dPrimaryDevice->GetOffscreenVR(1);
         m_pin3d.m_pd3dPrimaryDevice->SetRenderTarget("Right Eye"s, rightTexture, false);
         m_pin3d.m_pd3dPrimaryDevice->AddRenderTargetDependency(renderedRT);
         m_pin3d.m_pd3dPrimaryDevice->BlitRenderTarget(renderedRT, rightTexture, true, false, 0, 0, w, h, 0, 0, w, h, 1, 0);
         if (m_liveUI->IsTweakMode()) // Render as an overlay in VR (not in preview) at the eye resolution, without depth
            m_pin3d.m_pd3dPrimaryDevice->RenderLiveUI();

         RenderTarget *outRT = m_pin3d.m_pd3dPrimaryDevice->GetOutputBackBuffer();
         m_pin3d.m_pd3dPrimaryDevice->SetRenderTarget("VR Preview"s, outRT, false);
         m_pin3d.m_pd3dPrimaryDevice->AddRenderTargetDependency(leftTexture); // To ensure blit is made
         m_pin3d.m_pd3dPrimaryDevice->AddRenderTargetDependency(rightTexture); // To ensure blit is made
         m_pin3d.m_pd3dPrimaryDevice->AddRenderTargetDependency(renderedRT);
         const int outW = m_vrPreview == VRPREVIEW_BOTH ? outRT->GetWidth() / 2 : outRT->GetWidth(), outH = outRT->GetHeight();
         float ar = (float)w / (float)h, outAr = (float)outW / (float)outH;
         int x = 0, y = 0;
         int fw = w, fh = h;
         if ((m_vrPreviewShrink && ar < outAr) || (!m_vrPreviewShrink && ar > outAr))
         { // Fit on Y
            const int scaledW = (int) (h * outAr);
            x = (w - scaledW) / 2;
            fw = scaledW;
         }
         else
         { // Fit on X
            const int scaledH = (int)(w / outAr);
            y = (h - scaledH) / 2;
            fh = scaledH;
         }
         if (m_vrPreviewShrink || m_vrPreview == VRPREVIEW_DISABLED)
            m_pin3d.m_pd3dPrimaryDevice->Clear(clearType::TARGET | clearType::ZBUFFER, 0, 1.0f, 0L);
         if (m_vrPreview == VRPREVIEW_LEFT || m_vrPreview == VRPREVIEW_RIGHT)
         {
            m_pin3d.m_pd3dPrimaryDevice->BlitRenderTarget(renderedRT, outRT, true, false, x, y, fw, fh, 0, 0, outW, outH, m_vrPreview == VRPREVIEW_LEFT ? 0 : 1, 0);
         }
         else if (m_vrPreview == VRPREVIEW_BOTH)
         {
            m_pin3d.m_pd3dPrimaryDevice->BlitRenderTarget(renderedRT, outRT, true, false, x, y, fw, fh, 0, 0, outW, outH, 0, 0);
            m_pin3d.m_pd3dPrimaryDevice->BlitRenderTarget(renderedRT, outRT, true, false, x, y, fw, fh, outW, 0, outW, outH, 1, 0);
         }

         m_pin3d.m_pd3dPrimaryDevice->SubmitVR(renderedRT);
      }
      else 
      #endif
      if (IsAnaglyphStereoMode(m_stereo3D) || Is3DTVStereoMode(m_stereo3D))
      {
         // Anaglyph and 3DTV
         assert(renderedRT != m_pin3d.m_pd3dPrimaryDevice->GetOutputBackBuffer());
         // For anaglyph, defocus the "lesser" eye (the one with a darker color, which should be the non dominant eye of the player)
         if (m_stereo3DDefocus != 0.f)
         {
            RenderTarget *tmpRT = m_pin3d.m_pd3dPrimaryDevice->GetPostProcessRenderTarget(renderedRT);
            outputRT = m_pin3d.m_pd3dPrimaryDevice->GetPostProcessRenderTarget(tmpRT);
            m_pin3d.m_pd3dPrimaryDevice->DrawGaussianBlur(renderedRT, tmpRT, outputRT, abs(m_stereo3DDefocus) * 39.f, m_stereo3DDefocus > 0.f ? 0 : 1);
            renderedRT = outputRT;
         }
         // Stereo composition
         m_pin3d.m_pd3dPrimaryDevice->SetRenderTarget("Stereo Anaglyph"s, m_pin3d.m_pd3dPrimaryDevice->GetOutputBackBuffer(), false);
         m_pin3d.m_pd3dPrimaryDevice->AddRenderTargetDependency(renderedRT);
         if (m_stereo3DfakeStereo)
         {
            m_pin3d.m_pd3dPrimaryDevice->AddRenderTargetDependency(m_pin3d.m_pd3dPrimaryDevice->GetBackBufferTexture(), true);
            m_pin3d.m_pd3dPrimaryDevice->StereoShader->SetTexture(SHADER_tex_stereo_depth, m_pin3d.m_pd3dPrimaryDevice->GetBackBufferTexture()->GetDepthSampler());
         }
         m_pin3d.m_pd3dPrimaryDevice->StereoShader->SetTexture(SHADER_tex_stereo_fb, renderedRT->GetColorSampler());
         m_pin3d.m_pd3dPrimaryDevice->DrawFullscreenTexturedQuad(m_pin3d.m_pd3dPrimaryDevice->StereoShader);
      }
      else
      {
         // STEREO_OFF: nothing to do
         assert(renderedRT == m_pin3d.m_pd3dPrimaryDevice->GetOutputBackBuffer());
      }
   }
   // Upscale: When using downscaled backbuffer (for performance reason), upscaling is done after postprocessing
   else if (useUpscaler)
   {
      assert(renderedRT != m_pin3d.m_pd3dPrimaryDevice->GetOutputBackBuffer()); // At this point, renderedRT may be PP1, PP2 or backbuffer
      outputRT = m_pin3d.m_pd3dPrimaryDevice->GetOutputBackBuffer();
      m_pin3d.m_pd3dPrimaryDevice->SetRenderTarget("Upscale"s, outputRT, false);
      m_pin3d.m_pd3dPrimaryDevice->AddRenderTargetDependency(renderedRT);
      m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTexture(SHADER_tex_fb_filtered, renderedRT->GetColorSampler());
      m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTechnique(SHADER_TECHNIQUE_fb_copy);
      m_pin3d.m_pd3dPrimaryDevice->DrawFullscreenTexturedQuad(m_pin3d.m_pd3dPrimaryDevice->FBShader);
      renderedRT = outputRT;
   }

   if (m_stereo3D != STEREO_VR)
   {
      // Except for VR, render LiveUI after tonemapping and stereo (otherwise it would break the calibration process for stereo anaglyph)
      g_frameProfiler.EnterProfileSection(FrameProfiler::PROFILE_MISC);
      m_pin3d.m_pd3dPrimaryDevice->SetRenderTarget("ImGui"s, m_pin3d.m_pd3dPrimaryDevice->GetOutputBackBuffer());
      m_pin3d.m_pd3dPrimaryDevice->RenderLiveUI();
      g_frameProfiler.ExitProfileSection();
   }

   if (GetProfilingMode() == PF_ENABLED)
      m_pin3d.m_gpu_profiler.Timestamp(GTS_PostProcess);
}

void Player::SetScreenOffset(const float x, const float y)
{
   const float rotation = ANGTORAD(m_ptable->mViewSetups[m_ptable->m_BG_current_set].GetRotation(m_pin3d.m_pd3dPrimaryDevice->m_width, m_pin3d.m_pd3dPrimaryDevice->m_height));
   const float c = cosf(-rotation), s = sinf(-rotation);
   m_ScreenOffset.x = x * c - y * s;
   m_ScreenOffset.y = x * s + y * c;
}

void Player::LockForegroundWindow(const bool enable)
{
    if (m_fullScreen || (m_wnd_width == m_screenwidth && m_wnd_height == m_screenheight)) // detect windowed fullscreen
    {
        if(enable)
        {
            while (ShowCursor(TRUE) < 0) ;
            while (ShowCursor(FALSE) >= 0) ;
        }
        else
        {
            while (ShowCursor(FALSE) >= 0) ;
            while (ShowCursor(TRUE) < 0) ;
        }
    }

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

void Player::OnIdle()
{
   assert(m_stereo3D != STEREO_VR || (m_videoSyncMode == VideoSyncMode::VSM_NONE && m_maxFramerate == 0)); // Stereo must be run unthrotlled to let OpenVR set the frame pace according to the head set

   if (m_videoSyncMode == VideoSyncMode::VSM_FRAME_PACING)
   {
      // The main loop tries to perform a constant input/physics cycle at a 1ms pace while feeding the GPU command queue at a stable rate, without multithreading.
      // These 2 tasks are designed as follows:
      // - Input/Phyics: acquire, then process input (executing script events that will trigger the PinMAME controller), then allow
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

      // Render frame following these: Prepare / Submit to GPU / Present frame when monitor is ready to display a new frame and GPU has finished rendering (not for DX9)
      switch (m_mainLoopPhase)
      {
      case 0:
      {
         g_frameProfiler.NewFrame(m_time_msec);
         PLOGI_IF(debugLog) << "Frame Collect [Last frame length: " << ((double)g_frameProfiler.GetPrev(FrameProfiler::PROFILE_FRAME) / 1000.0) << "ms] at " << usec();
         PrepareFrame();
         m_mainLoopPhase = 1;
      }
      break;

      case 1:
      {
         PLOGI_IF(debugLog) << "Frame Submit at " << usec();
         SubmitFrame();
         g_frameProfiler.EnterProfileSection(FrameProfiler::PROFILE_SLEEP);
         m_mainLoopPhase = 2;
      }
      break;

      case 2:
      {
         // Wait for at least one VBlank after last frame submission (adaptive sync)
         if (m_pin3d.m_pd3dPrimaryDevice->m_vsyncCount == 0)
         {
            m_curFrameSyncOnVBlank = true;
            break;
         }

         // If the user asked to sync on a lower frame rate than the refresh rate, then wait for it
         if (m_maxFramerate != m_refreshrate)
         {
            const U64 now = usec();
            const int refreshLength = (int)(1000000ul / m_refreshrate);
            const int minimumFrameLength = 1000000ull / m_maxFramerate;
            const int maximumFrameLength = 5 * refreshLength;
            const int targetFrameLength = clamp(refreshLength - 2000, min(minimumFrameLength, maximumFrameLength), maximumFrameLength);
            if (now < m_lastPresentFrameTick + targetFrameLength)
            {
               m_curFrameSyncOnFPS = true;
               break;
            }
            m_lastPresentFrameTick = now;
         }

         // Schedule frame presentation, ask for an asynchronous VBlank, start preparing next frame
         m_lastFrameSyncOnVBlank = m_curFrameSyncOnVBlank;
         m_lastFrameSyncOnFPS = m_curFrameSyncOnFPS;
         PLOGI_IF(debugLog) << "Frame Scheduled at " << usec() << ", Waited for VBlank: " << m_curFrameSyncOnVBlank << ", Waited for FPS: " << m_curFrameSyncOnFPS;
         m_pin3d.m_pd3dPrimaryDevice->m_vsyncCount = 0;
         g_frameProfiler.ExitProfileSection(); // Out of Sleep section
         g_frameProfiler.EnterProfileSection(FrameProfiler::PROFILE_GPU_FLIP);
         m_pin3d.m_pd3dPrimaryDevice->Flip();
         m_pin3d.m_pd3dPrimaryDevice->WaitForVSync(true);
         g_frameProfiler.ExitProfileSection();
         if (FinishFrame())
            return;
         m_curFrameSyncOnVBlank = m_curFrameSyncOnFPS = false;
         m_mainLoopPhase = 0;
      }
      break;

      default: assert(false);
      }

      // Update physics. Do it continuously for lower latency between input <-> controler/physics (avoid catching up once per frame)
      if (!m_pause)
      {
         // Trigger key events before processing physics, also allows to sync with VPM
         m_pininput.ProcessKeys(/*sim_msec,*/ -(int)msec());
         UpdatePhysics();
         PLOGI_IF(debugLog && m_phys_iterations > 0) << "Input/Physics done (" << m_phys_iterations << " iterations)";
      }
   }
   else
   {
      // Legacy main loop performs the frame as a single block. This leads to having the input <-> physics stall between frames increasing 
      // the latency and causing syncing problems with PinMAME (which runs in realtime and expects realtime inputs, especially for video modes
      // with repeated button presses like Black Rose's "Walk the Plank Video Mode" or Lethal Weapon 3's "Battle Video Mode")
      // This also leads to filling up the GPU render queue leading to a few frame latency, depending on driver setup (hence the use of a limiter).

      // Collect stats from previous frame and starts profiling a new frame
      g_frameProfiler.NewFrame(m_time_msec);

      // In pause mode: input, physics, animation and audio are not processed but rendering is still performed. This allows to modify properties (transform, visibility,..) using the debugger and get direct feedback
      if (!m_pause)
         m_pininput.ProcessKeys(/*sim_msec,*/ -(int)(m_startFrameTick / 1000)); // trigger key events mainly for VPM<->VP roundtrip

      // Physics/Timer updates, done at the last moment, especially to handle key input (VP<->VPM roundtrip) and animation triggers
      if (!m_pause && m_minphyslooptime == 0) // (vsync) latency reduction code not active? -> Do Physics Updates here
         UpdatePhysics();

      PrepareFrame();

      SubmitFrame();

      // (Optionally) force queue flushing of the driver. Can be used to artifically limit latency on DX9 (depends on OS/GFXboard/driver if still useful nowadays). This must be done after submiting render commands
      m_limiter.Execute(m_pin3d.m_pd3dPrimaryDevice);

      // DJRobX's crazy latency-reduction code active? Insert some Physics updates before vsync'ing
      if (!m_pause && m_minphyslooptime > 0)
      {
         UpdatePhysics();
         m_pininput.ProcessKeys(/*sim_msec,*/ -(int)(m_startFrameTick / 1000)); // trigger key events mainly for VPM<->VP rountrip
      }

      // Present & VSync
      g_frameProfiler.EnterProfileSection(FrameProfiler::PROFILE_GPU_FLIP);
      m_pin3d.m_pd3dPrimaryDevice->Flip();
      #ifndef ENABLE_SDL // DirectX 9 does not support native adaptive sync, so we must emulate it at the application level
      if (m_videoSyncMode == VideoSyncMode::VSM_ADAPTIVE_VSYNC && m_fps > m_maxFramerate * ADAPT_VSYNC_FACTOR)
         m_pin3d.m_pd3dPrimaryDevice->WaitForVSync(false);
      #endif
      g_frameProfiler.ExitProfileSection();

      if (FinishFrame())
         return;

      // Adjust framerate if requested by user (i.e. not using a synchronization mode that will lead to blocking calls aligned to the display refresh rate)
      if (m_maxFramerate != 0 // User has requested a target FPS
      && (m_videoSyncMode == VideoSyncMode::VSM_NONE || m_maxFramerate != m_refreshrate)) // The synchronization is not already performed by the VSYNC
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
   }
}

void Player::PrepareFrame()
{
   // Rendering outputs to m_pd3dPrimaryDevice->GetBackBufferTexture(). If MSAA is used, it is resolved as part of the rendering (i.e. this surface is NOT the MSAA rneder surface but its resolved copy)
   // Then it is tonemapped/bloom/dither/... to m_pd3dPrimaryDevice->GetPostProcessRenderTarget1() if needed for postprocessing (sharpen, FXAA,...), or directly to the main output framebuffer otherwise
   // The optional postprocessing is done from m_pd3dPrimaryDevice->GetPostProcessRenderTarget1() to the main output framebuffer

   m_overall_frames++;
   m_LastKnownGoodCounter++;
   m_startFrameTick = usec();
   g_frameProfiler.OnPrepare();

   // Reset per frame debug counters
   #ifdef DEBUGPHYSICS
   c_hitcnts = 0;
   c_collisioncnt = 0;
   c_contactcnt = 0;
   #ifdef C_DYNAMIC
   c_staticcnt = 0;
   #endif
   c_embedcnts = 0;
   c_timesearch = 0;

   c_kDNextlevels = 0;
   //c_quadNextlevels = 0; // not updated per frame so keep!

   c_traversed = 0;
   c_tested = 0;
   c_deepTested = 0;
   #endif

   // Update all non-physics-controlled animated parts (e.g. primitives, reels, gates, lights, bumper-skirts, hittargets, etc)
   if (!m_pause)
   {
      const float diff_time_msec = (float)(m_time_msec - m_last_frame_time_msec);
      m_last_frame_time_msec = m_time_msec;
      if(diff_time_msec > 0.f)
         for (size_t i = 0; i < m_ptable->m_vedit.size(); ++i)
         {
            Hitable *const ph = m_ptable->m_vedit[i]->GetIHitable();
            if (ph)
               ph->UpdateAnimation(diff_time_msec);
         }
   }

   // Fire all '-1' (the ones which are synced to the refresh rate) and '-2' (the ones used to sync with the controller) timers after physics and animation update but before rendering, to avoid the script being one frame late
   for (HitTimer *const pht : m_vht)
      if (pht->m_interval < 0)
      {
         g_frameProfiler.EnterScriptSection(DISPID_TimerEvents_Timer, pht->m_name);
         pht->m_pfe->FireGroupEvent(DISPID_TimerEvents_Timer);
         g_frameProfiler.ExitScriptSection(pht->m_name);
      }

   // Kill the profiler so that it does not affect performance => FIXME move to player
   if (m_infoMode != IF_PROFILING)
      m_pin3d.m_gpu_profiler.Shutdown();

#ifndef ENABLE_SDL
   if (GetProfilingMode() == PF_ENABLED)
      m_pin3d.m_gpu_profiler.BeginFrame(m_pin3d.m_pd3dPrimaryDevice->GetCoreDevice());
#endif

   RenderStaticPrepass();

   g_frameProfiler.EnterProfileSection(FrameProfiler::PROFILE_GPU_COLLECT);
   m_pin3d.m_pd3dPrimaryDevice->SetRenderTarget("Render Scene"s, m_pin3d.m_pd3dPrimaryDevice->GetMSAABackBufferTexture());
   if (m_stereo3D == STEREO_VR || GetInfoMode() == IF_DYNAMIC_ONLY)
   {
      // For VR start from a clear render
      m_pin3d.m_pd3dPrimaryDevice->Clear(clearType::TARGET | clearType::ZBUFFER, 0, 1.0f, 0L);
   }
   else
   {
      // copy static buffers to back buffer including z buffer
      m_pin3d.m_pd3dPrimaryDevice->AddRenderTargetDependency(m_staticPrepassRT);
      m_pin3d.m_pd3dPrimaryDevice->BlitRenderTarget(m_staticPrepassRT, m_pin3d.m_pd3dPrimaryDevice->GetMSAABackBufferTexture());
   }

   // Update camera point of view
   #ifdef ENABLE_VR
   if (m_stereo3D == STEREO_VR)
      m_pin3d.m_pd3dPrimaryDevice->UpdateVRPosition(m_pin3d.GetMVP());
   else 
   #endif
   if (m_headTracking)
      // #ravarcade: UpdateBAMHeadTracking will set proj/view matrix to add BAM view and head tracking
      m_pin3d.UpdateBAMHeadTracking();
   else if (m_liveUI->IsTweakMode())
      m_pin3d.InitLayout();

   // Setup ball rendering: collect all lights that can reflect on balls
   m_ballTrailMeshBufferPos = 0;
   m_ballReflectedLights.clear();
   for (size_t i = 0; i < m_ptable->m_vedit.size(); i++)
   {
      IEditable * const item = m_ptable->m_vedit[i];
      if (item && item->GetItemType() == eItemLight && ((Light *)item)->m_d.m_showReflectionOnBall)
         m_ballReflectedLights.push_back((Light *)item);
   }
   // We don't need to set the dependency on the previous frame render as this would be a cross frame dependency which does not have any meaning since dependencies are resolved per frame
   // m_pin3d.m_pd3dPrimaryDevice->AddRenderTargetDependency(m_pin3d.m_pd3dPrimaryDevice->GetPreviousBackBufferTexture());
   m_pin3d.m_pd3dPrimaryDevice->m_ballShader->SetTexture(SHADER_tex_ball_playfield, m_pin3d.m_pd3dPrimaryDevice->GetPreviousBackBufferTexture()->GetColorSampler());

   if (GetInfoMode() != IF_STATIC_ONLY)
      RenderDynamics();

   // Resolve MSAA buffer to a normal one (noop if not using MSAA), allowing sampling it for postprocessing
   m_pin3d.m_pd3dPrimaryDevice->ResolveMSAA();

   if (!m_pause && (m_videoSyncMode != VideoSyncMode::VSM_FRAME_PACING))
      m_pininput.ProcessKeys(/*sim_msec,*/ -(int)(m_startFrameTick / 1000)); // trigger key events mainly for VPM<->VP roundtrip

   // Check if we should turn animate the plunger light.
   hid_set_output(HID_OUTPUT_PLUNGER, ((m_time_msec - m_LastPlungerHit) < 512) && ((m_time_msec & 512) > 0));

   g_frameProfiler.EnterProfileSection(FrameProfiler::PROFILE_MISC);
   if (m_stereo3D != STEREO_VR)
      m_liveUI->Update(m_pin3d.m_pd3dPrimaryDevice->GetOutputBackBuffer());
   else if (m_liveUI->IsTweakMode())
      m_liveUI->Update(m_pin3d.m_pd3dPrimaryDevice->GetOffscreenVR(0));
   g_frameProfiler.ExitProfileSection();

   PrepareVideoBuffers();

   g_frameProfiler.ExitProfileSection();
}

void Player::SubmitFrame()
{
   // Submit to GPU render queue
   g_frameProfiler.EnterProfileSection(FrameProfiler::PROFILE_GPU_SUBMIT);
   m_pin3d.m_pd3dPrimaryDevice->FlushRenderFrame();
   if (m_stereo3D == STEREO_VR && m_vrPreview != VRPREVIEW_DISABLED && !m_liveUI->IsTweakMode() && m_liveUI->IsOpened())
   {
      m_pin3d.m_pd3dPrimaryDevice->SetRenderTarget("ImgUI-Preview"s, m_pin3d.m_pd3dPrimaryDevice->GetOutputBackBuffer(), false);
      m_liveUI->Update(m_pin3d.m_pd3dPrimaryDevice->GetOutputBackBuffer());
      m_pin3d.m_pd3dPrimaryDevice->RenderLiveUI();
      m_pin3d.m_pd3dPrimaryDevice->FlushRenderFrame();
   }
   m_pin3d.m_pd3dPrimaryDevice->SwapBackBufferRenderTargets(); // Keep previous render as a reflection probe for ball reflection and for hires motion blur
   g_frameProfiler.ExitProfileSection();

   // Trigger captures
   #ifdef ENABLE_SDL
   if (m_stereo3D == STEREO_VR)
      UpdateExtCaptures();
   #endif
}

bool Player::FinishFrame()
{
   // switch to texture output buffer again
   m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTextureNull(SHADER_tex_fb_filtered);
   m_pin3d.m_pd3dPrimaryDevice->FBShader->SetTextureNull(SHADER_tex_fb_unfiltered);

   m_lastFlipTime = usec();

   if (GetProfilingMode() != PF_DISABLED)
      m_pin3d.m_gpu_profiler.EndFrame();

   // Update FPS counter
   m_fps = (float) (1e6 / g_frameProfiler.GetSlidingAvg(FrameProfiler::PROFILE_FRAME));

#ifndef ACCURATETIMERS
   // do the en/disable changes for the timers that piled up
   for (size_t i = 0; i < m_changed_vht.size(); ++i)
       if (m_changed_vht[i].enabled) // add the timer?
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

   Ball * const old_pactiveball = m_pactiveball;
   m_pactiveball = nullptr;  // No ball is the active ball for timers/key events

   for (size_t i=0;i<m_vht.size();i++)
   {
      HitTimer * const pht = m_vht[i];
      if (pht->m_interval >= 0 && pht->m_nextfire <= m_time_msec) 
      {
         g_frameProfiler.EnterScriptSection(DISPID_TimerEvents_Timer, pht->m_name);
         const unsigned int curnextfire = pht->m_nextfire;
         pht->m_pfe->FireGroupEvent(DISPID_TimerEvents_Timer);
         // Only add interval if the next fire time hasn't changed since the event was run. 
         // Handles corner case:
         //Timer1.Enabled = False
         //Timer1.Interval = 1000
         //Timer1.Enabled = True
         if (curnextfire == pht->m_nextfire)
            pht->m_nextfire += pht->m_interval;
         g_frameProfiler.ExitScriptSection(pht->m_name);
      }
   }

   m_pactiveball = old_pactiveball;
#else
   if (!m_pause && (m_videoSyncMode != VideoSyncMode::VSM_FRAME_PACING))
      m_pininput.ProcessKeys(/*sim_msec,*/ -(int)(m_startFrameTick / 1000)); // trigger key events mainly for VPM<->VP rountrip
#endif

   // Update music stream
   if (!m_pause && m_audio)
   {
      if (!m_audio->MusicActive())
      {
         delete m_audio;
         m_audio = nullptr;
         m_ptable->FireVoidEvent(DISPID_GameEvents_MusicDone);
      }
   }

   // Pause after performing a simulation step
   #ifdef STEPPING
   if ((m_pauseTimeTarget > 0) && (m_pauseTimeTarget <= m_time_msec))
   {
      m_pauseTimeTarget = 0;
      m_userDebugPaused = true;
      RecomputePseudoPauseState();
      if(m_debuggerDialog.IsWindow())
        m_debuggerDialog.SendMessage(RECOMPUTEBUTTONCHECK, 0, 0);
   }
   #endif

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
      if (g_pvp->m_disable_pause_menu || m_stereo3D == STEREO_VR)
         m_closing = CS_STOP_PLAY;
      else
         m_liveUI->OpenMainSplash();
   }

   // Brute force stop: blast into space
   if (m_closing == CS_FORCE_STOP)
      exit(-9999); 

   // Close player (moving back to editor or to system is handled after player has been closed)
   if (m_closing == CS_STOP_PLAY || m_closing == CS_CLOSE_APP)
   {
      OnClose();
      return true;
   }

   // Open debugger window
   if (m_showDebugger && !g_pvp->m_disable_pause_menu && !m_ptable->IsLocked())
   {
      m_debugMode = true;
      m_showDebugger = false;
      while(ShowCursor(FALSE) >= 0) ;
      while(ShowCursor(TRUE) < 0) ;

      if (!m_debuggerDialog.IsWindow())
      {
         m_debuggerDialog.Create(GetHwnd());
         m_debuggerDialog.ShowWindow();
      }
      else
         m_debuggerDialog.SetForegroundWindow();

      EndDialog( g_pvp->GetHwnd(), ID_DEBUGWINDOW );
   }

   // Try to bring PinMAME window back on top
   if (m_overall_frames < 10)
   {
      const HWND hVPMWnd = FindWindow("MAME", nullptr);
      if (hVPMWnd != nullptr)
      {
         if (::IsWindowVisible(hVPMWnd))
            ::SetWindowPos(
               hVPMWnd, HWND_TOPMOST, 0, 0, 0, 0, (SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW | SWP_NOACTIVATE)); // in some strange cases the VPinMAME window is not on top, so enforce it
      }
   }

   return false;
}

void Player::PauseMusic()
{
   if (m_pauseRefCount == 0)
   {
      if (m_audio)
         m_audio->MusicPause();

      // signal the script that the game is now paused
      m_ptable->FireVoidEvent(DISPID_GameEvents_Paused);
   }

   m_pauseRefCount++;
}

void Player::UnpauseMusic()
{
   m_pauseRefCount--;
   if (m_pauseRefCount == 0)
   {
      if (m_audio)
         m_audio->MusicUnpause();

      // signal the script that the game is now running again
      m_ptable->FireVoidEvent(DISPID_GameEvents_UnPaused);
   }
   else if (m_pauseRefCount < 0)
      m_pauseRefCount = 0;
}

void Player::DrawStatics()
{
   const unsigned int mask = m_render_mask;
   m_render_mask |= STATIC_ONLY;
   for (Hitable *hitable : m_vhitables)
      hitable->Render(m_render_mask);
   m_render_mask = mask;
}

void Player::DrawDynamics(bool onlyBalls)
{
   if (!onlyBalls)
   {
      // Update Bulb light buffer and set up render pass dependencies
      DrawBulbLightBuffer();

      // Draw all parts
      const unsigned int mask = m_render_mask;
      m_render_mask |= DYNAMIC_ONLY;
      for (Hitable *hitable : m_vhitables)
         hitable->Render(m_render_mask);
      m_render_mask = mask;
   }
   
   for (Ball* ball : m_vball)
      ball->m_pballex->Render(m_render_mask);
}

struct DebugMenuItem
{
   int objectindex;
   vector<int> *pvdispid;
   HMENU hmenu;
};

void AddEventToDebugMenu(const char *sz, int index, int dispid, LPARAM lparam)
{
   const DebugMenuItem * const pdmi = (DebugMenuItem *)lparam;
   const HMENU hmenu = pdmi->hmenu;
   const int menuid = ((pdmi->objectindex + 1) << 16) | (int)pdmi->pvdispid->size();
   pdmi->pvdispid->push_back(dispid);
   AppendMenu(hmenu, MF_STRING, menuid, sz);
}

void Player::DoDebugObjectMenu(const int x, const int y)
{
   if (m_vdebugho.empty())
   {
      // First time the debug hit-testing has been used
      InitDebugHitStructure();
   }

   Matrix3D mat3D = m_pin3d.GetMVP().GetModelViewProj(0);
   mat3D.Invert();

   ViewPort vp;
   m_pin3d.m_pd3dPrimaryDevice->GetViewport(&vp);
   const float rClipWidth = (float)vp.Width*0.5f;
   const float rClipHeight = (float)vp.Height*0.5f;

   const float xcoord = ((float)x - rClipWidth) / rClipWidth;
   const float ycoord = (rClipHeight - (float)y) / rClipHeight;

   // Use the inverse of our 3D transform to determine where in 3D space the
   // screen pixel the user clicked on is at.  Get the point at the near
   // clipping plane (z=0) and the far clipping plane (z=1) to get the whole
   // range we need to hit test
   Vertex3Ds v3d, v3d2;
   mat3D.MultiplyVector(Vertex3Ds(xcoord, ycoord, 0.f), v3d);
   mat3D.MultiplyVector(Vertex3Ds(xcoord, ycoord, 1.f), v3d2);

   // Create a ray (ball) that travels in 3D space from the screen pixel at
   // the near clipping plane to the far clipping plane, and find what
   // it intersects with.
   Ball ballT;
   ballT.m_d.m_pos = v3d;
   ballT.m_d.m_vel = v3d2 - v3d;
   ballT.m_d.m_radius = 0.f;
   ballT.m_coll.m_hittime = 1.0f;

   ballT.CalcHitBBox(); // need to update here, as only done lazily

   //const float slope = (v3d2.y - v3d.y)/(v3d2.z - v3d.z);
   //const float yhit = v3d.y - (v3d.z*slope);

   //const float slopex = (v3d2.x - v3d.x)/(v3d2.z - v3d.z);
   //const float xhit = v3d.x - (v3d.z*slopex);

   vector<HitObject*> vhoHit;
   m_hitoctree_dynamic.HitTestXRay(&ballT, vhoHit, ballT.m_coll);
   m_hitoctree.HitTestXRay(&ballT, vhoHit, ballT.m_coll);
   m_debugoctree.HitTestXRay(&ballT, vhoHit, ballT.m_coll);

   if (vhoHit.empty())
   {
      // Nothing was hit-tested
      return;
   }

   PauseMusic();

   const HMENU hmenu = CreatePopupMenu();

   vector<IFireEvents*> vpfe;
   vector<HMENU> vsubmenu;
   vector< vector<int>* > vvdispid;
   for (size_t i = 0; i < vhoHit.size(); i++)
   {
      HitObject * const pho = vhoHit[i];
      // Make sure we don't do the same object twice through 2 different Hitobjs.
      if (pho->m_pfedebug && (FindIndexOf(vpfe, pho->m_pfedebug) == -1))
      {
         vpfe.push_back(pho->m_pfedebug);
         CComVariant var;
         DISPPARAMS dispparams = {
            nullptr,
            nullptr,
            0,
            0
         };
         const HRESULT hr = pho->m_pfedebug->GetDispatch()->Invoke(
            0x80010000, IID_NULL,
            LOCALE_USER_DEFAULT,
            DISPATCH_PROPERTYGET,
            &dispparams, &var, nullptr, nullptr);

         const HMENU submenu = CreatePopupMenu();
         vsubmenu.push_back(submenu);
         if (hr == S_OK)
         {
            WCHAR *wzT;
            wzT = V_BSTR(&var);
            AppendMenuW(hmenu, MF_STRING | MF_POPUP, (UINT_PTR)submenu, wzT);

            vector<int> *pvdispid = new vector<int>();
            vvdispid.push_back(pvdispid);

            DebugMenuItem dmi;
            dmi.objectindex = (int)i;
            dmi.pvdispid = pvdispid;
            dmi.hmenu = submenu;
            EnumEventsFromDispatch(pho->m_pfedebug->GetDispatch(), AddEventToDebugMenu, (LPARAM)&dmi);
         }

         IDebugCommands * const pdc = pho->m_pfedebug->GetDebugCommands();
         if (pdc)
         {
            vector<int> vids;
            vector<int> vcommandid;

            pdc->GetDebugCommands(vids, vcommandid);
            for (size_t l = 0; l < vids.size(); l++)
            {
               const LocalString ls(vids[l]);
               AppendMenu(submenu, MF_STRING, ((i + 1) << 16) | vcommandid[l] | 0x8000, ls.m_szbuffer);
            }
         }
      }
      else
      {
         vvdispid.push_back(nullptr); // Put a spacer in so we can keep track of indexes
      }
   }

   POINT pt;
   pt.x = x;
   pt.y = y;
   ClientToScreen(pt);

   const int icmd = TrackPopupMenuEx(hmenu, TPM_RETURNCMD | TPM_RIGHTBUTTON, pt.x, pt.y, GetHwnd(), nullptr);

   if (icmd != 0 && !vsubmenu.empty())
   {
      const int highword = HIWORD(icmd) - 1;
      const int lowword = icmd & 0xffff;
      IFireEvents * const pfe = vhoHit[highword]->m_pfedebug;
      if (lowword & 0x8000) // custom debug command
      {
         pfe->GetDebugCommands()->RunDebugCommand(lowword & 0x7fff);
      }
      else
      {
         const int dispid = (*vvdispid[highword])[lowword];
         m_pactiveball = m_pactiveballDebug;
         pfe->FireGroupEvent(dispid);
         m_pactiveball = nullptr;
      }
   }

   DestroyMenu(hmenu);
   for (size_t i = 0; i < vsubmenu.size(); i++)
      DestroyMenu(vsubmenu[i]);

   for (size_t i = 0; i < vvdispid.size(); i++)
      delete vvdispid[i];

   UnpauseMusic();
}

LRESULT Player::WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(GetHwnd(), uMsg, wParam, lParam))
      return true;

    switch (uMsg)
    {
    case MM_MIXM_CONTROL_CHANGE:
        mixer_get_volume();
        break;

    case WM_KEYDOWN:
        m_drawCursor = false;
        SetCursor(nullptr);
        break;

    case WM_MOUSEMOVE:
        if (m_lastcursorx != LOWORD(lParam) || m_lastcursory != HIWORD(lParam))
        {
            m_drawCursor = true;
            m_lastcursorx = LOWORD(lParam);
            m_lastcursory = HIWORD(lParam);
        }
        break;

#ifdef STEPPING
#ifdef MOUSEPAUSE
    case WM_LBUTTONDOWN:
        if (m_pause)
        {
            m_step = true;
        }
        break;

    case WM_RBUTTONDOWN:
        if (!m_pause)
        {
            m_pause = true;

            m_gameWindowActive = false;
            RecomputePauseState();
            RecomputePseudoPauseState();
        }
        else
        {
            m_pause = false;

            m_gameWindowActive = true;
            SetCursor(nullptr);
            m_noTimeCorrect = true;
        }
        break;
#endif
#endif
    case WM_RBUTTONUP:
    {
        if (m_debugMode)
        {
            const int x = lParam & 0xffff;
            const int y = (lParam >> 16) & 0xffff;
            DoDebugObjectMenu(x, y);
        }
        break;
    }
    

    case WM_POINTERDOWN:
    case WM_POINTERUP:
    {
#ifndef TEST_TOUCH_WITH_MOUSE
        if (!GetPointerInfo)
            GetPointerInfo = (pGPI)GetProcAddress(GetModuleHandle(TEXT("user32.dll")),
                "GetPointerInfo");
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
                ScreenToClient(pointerInfo.ptPixelLocation);
                for (unsigned int i = 0; i < MAX_TOUCHREGION; ++i)
               if ((m_touchregion_pressed[i] != (uMsg == WM_POINTERDOWN))
                  && Intersect(touchregion[i], m_wnd_width, m_wnd_height, pointerInfo.ptPixelLocation,
                     m_ptable->mViewSetups[m_ptable->m_BG_current_set].GetRotation(m_pin3d.m_pd3dPrimaryDevice->m_width, m_pin3d.m_pd3dPrimaryDevice->m_height) != 0.f))
                    {
                        m_touchregion_pressed[i] = (uMsg == WM_POINTERDOWN);

                        DIDEVICEOBJECTDATA didod;
                        didod.dwOfs = m_rgKeys[touchkeymap[i]];
                        didod.dwData = m_touchregion_pressed[i] ? 0x80 : 0;
                        m_pininput.PushQueue(&didod, APP_KEYBOARD/*, curr_time_msec*/);
                    }
            }
        }
        break;
    }
    

    case WM_ACTIVATE:
        if (wParam != WA_INACTIVE)
            SetCursor(nullptr);
        {
            if (wParam != WA_INACTIVE)
            {
                m_gameWindowActive = true;
                m_noTimeCorrect = true;
#ifdef STEPPING
                m_pause = false;
#endif
            }
            else
            {
                m_gameWindowActive = false;
#ifdef STEPPING
                m_pause = true;
#endif
            }
            RecomputePauseState();
        }
        break;

    case WM_EXITMENULOOP:
        m_noTimeCorrect = true;
        break;

    case WM_SETCURSOR:
        if (LOWORD(lParam) == HTCLIENT && !m_drawCursor)
        {
            SetCursor(nullptr);
        }
        else
        {
            SetCursor(LoadCursor(nullptr, IDC_ARROW));
        }
        return TRUE;
    }

    return WndProcDefault(uMsg, wParam, lParam);
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
