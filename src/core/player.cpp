// license:GPLv3+

#include "core/stdafx.h"

#ifndef __STANDALONE__
#include "BAM/BAMView.h"
#endif

#ifndef __STANDALONE__
#define SDL_MAIN_NOIMPL
#include <SDL3/SDL_main.h>
#endif

#include "imgui/imgui_impl_sdl3.h"

#ifdef __STANDALONE__
#include "unordered_dense.h"
#endif

#ifdef __LIBVPINBALL__
#include "lib/src/VPinballLib.h"
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
#include "renderer/RenderCommand.h"
#ifdef EXT_CAPTURE
#include "renderer/captureExt.h"
#endif
#ifdef _MSC_VER
// Used to log which program steals the focus from VPX
#include "psapi.h"
#pragma comment(lib, "Psapi")
#endif
#include "tinyxml2/tinyxml2.h"
#include "ThreadPool.h"

#include "plugins/MsgPlugin.h"
#include "plugins/VPXPlugin.h"
#include "core/VPXPluginAPIImpl.h"

#include "input/ScanCodes.h"

#include "utils/ushock_output.h"

// MSVC Concurrency Viewer support
// This requires to add the MSVC Concurrency SDK to the project
//#define MSVC_CONCURRENCY_VIEWER
#ifdef MSVC_CONCURRENCY_VIEWER
#include <cvmarkersobj.h>
using namespace Concurrency::diagnostic;
extern marker_series series;
#endif

using namespace VPX;

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


Player::Player(PinTable *const table, const int playMode)
   : m_ptable(table)
   , m_backglassOutput(VPXWindowId::VPXWINDOW_Backglass)
   , m_scoreViewOutput(VPXWindowId::VPXWINDOW_ScoreView)
   , m_topperOutput(VPXWindowId::VPXWINDOW_Topper)
   , m_audioPlayer(std::make_unique<VPX::AudioPlayer>(
        table->m_settings.GetPlayer_SoundDeviceBG(), table->m_settings.GetPlayer_SoundDevice(), static_cast<VPX::SoundConfigTypes>(table->m_settings.GetPlayer_Sound3D())))
   , m_resURIResolver(MsgPI::MsgPluginManager::GetInstance().GetMsgAPI(), VPXPluginAPIImpl::GetInstance().GetVPXEndPointId(), true, true, true, true)
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

   bool useVR = false;
   #if defined(ENABLE_VR) || defined(ENABLE_XR)
      const int vrDetectionMode = m_ptable->m_settings.GetPlayerVR_AskToTurnOn();
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

   #ifdef ENABLE_DX9
   const StereoMode stereo3D = STEREO_OFF;
   #else
   const StereoMode stereo3D = useVR ? STEREO_VR : m_ptable->m_settings.GetPlayer_Stereo3D();
   #endif

   m_capExtDMD = (stereo3D == STEREO_VR) && m_ptable->m_settings.GetPlayer_CaptureExternalDMD();
   m_capPUP = (stereo3D == STEREO_VR) && m_ptable->m_settings.GetPlayer_CapturePUP();
   m_headTracking = (stereo3D == STEREO_VR) ? false : m_ptable->m_settings.GetPlayer_BAMHeadTracking();
   m_detectScriptHang = m_ptable->m_settings.GetPlayer_DetectHang();

   m_NudgeShake = m_ptable->m_settings.GetPlayer_NudgeStrength();

   m_minphyslooptime = m_ptable->m_settings.GetPlayer_MinPhysLoopTime();

   PLOGI << "Creating main window"; // For profiling
   {
      #if defined(_MSC_VER) && !defined(__STANDALONE__)
         WNDCLASS wc = {};
         wc.hInstance = g_pvp->theInstance;
         #ifdef _UNICODE
         wc.lpfnWndProc = ::DefWindowProcW;
         #else
         wc.lpfnWndProc = ::DefWindowProcA;
         #endif
         wc.lpszClassName = WIN32_PLAYER_WND_CLASSNAME;
         wc.hIcon = LoadIcon(g_pvp->theInstance, MAKEINTRESOURCE(IDI_TABLE));
         wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
         ::RegisterClass(&wc);
         SDL_RegisterApp(WIN32_PLAYER_WND_CLASSNAME, 0, g_pvp->theInstance);
      #endif
      
      const Settings& settings = g_pvp->m_settings; // Always use main application settings (not overridable per table)
      m_playfieldWnd = new VPX::Window("Visual Pinball Player"s, settings, stereo3D == STEREO_VR ? VPXWindowId::VPXWINDOW_VRPreview : VPXWindowId::VPXWINDOW_Playfield);
      g_pvp->ShowWindow(SW_HIDE);

      const float pfRefreshRate = m_playfieldWnd->GetRefreshRate(); 
      m_maxFramerate = static_cast<float>(m_ptable->m_settings.GetPlayer_MaxFramerate());
      if(m_maxFramerate > 0.f && m_maxFramerate < 24.f) // at least 24 fps
         m_maxFramerate = 24.f;
      if (m_maxFramerate < 0.f) // Negative is display refresh rate
         m_maxFramerate = pfRefreshRate;
      if (m_maxFramerate == 0.f) // 0 is unbound refresh rate
         m_maxFramerate = 10000.f;
      m_videoSyncMode = static_cast<VideoSyncMode>(m_ptable->m_settings.GetPlayer_SyncMode());
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

   // FIXME remove or at least move legacy ushock to a plugin
   ushock_output_init();

   // General player initialization

   TRACE_FUNCTION();

   PLOGI << "Initializing player"; // For profiling

   set_lowest_possible_win_timer_resolution();

   m_progressDialog.SetProgress("Initializing Visuals..."s, 10);

   m_PlayMusic = m_ptable->m_settings.GetPlayer_PlayMusic();
   m_PlaySound = m_ptable->m_settings.GetPlayer_PlaySound();
   m_MusicVolume = m_ptable->m_settings.GetPlayer_MusicVolume();
   m_SoundVolume = m_ptable->m_settings.GetPlayer_SoundVolume();
   UpdateVolume();

   //

   PLOGI << "Initializing renderer (global states & resources)"; // For profiling

   if (ViewSetup &viewSetup = m_ptable->GetViewSetup(); viewSetup.mMode == VLM_WINDOW)
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

   m_backglassOutput.SetMode(m_ptable->m_settings, static_cast<RenderOutput::OutputMode>(m_ptable->m_settings.GetWindow_Mode(VPXWindowId::VPXWINDOW_Backglass)));
   m_scoreViewOutput.SetMode(m_ptable->m_settings, static_cast<RenderOutput::OutputMode>(m_ptable->m_settings.GetWindow_Mode(VPXWindowId::VPXWINDOW_ScoreView)));
   m_topperOutput.SetMode(m_ptable->m_settings, static_cast<RenderOutput::OutputMode>(m_ptable->m_settings.GetWindow_Mode(VPXWindowId::VPXWINDOW_Topper)));
   #if defined(ENABLE_BGFX)
   if (m_vrDevice == nullptr) // Ancillary windows are not yet supported while in VR mode
   {
      if (m_backglassOutput.GetMode() == VPX::RenderOutput::OM_WINDOW)
         m_renderer->m_renderDevice->AddWindow(m_backglassOutput.GetWindow());
      if (m_scoreViewOutput.GetMode() == VPX::RenderOutput::OM_WINDOW)
         m_renderer->m_renderDevice->AddWindow(m_scoreViewOutput.GetWindow());
      if (m_topperOutput.GetMode() == VPX::RenderOutput::OM_WINDOW)
         m_renderer->m_renderDevice->AddWindow(m_topperOutput.GetWindow());
   }
   #endif

   // Disable static prerendering for VR and legacy headtracking (this won't be reenabled)
   if (m_headTracking || (stereo3D == STEREO_VR))
      m_renderer->DisableStaticPrePass(true);

   m_renderer->m_renderDevice->m_vsyncCount = 1;

   PLOGI << "Initializing inputs & implicit objects"; // For profiling

   Ball::ResetBallIDCounter();

   // Add a playfield primitive if it is missing
   bool hasExplicitPlayfield = false;
   for (size_t i = 0; i < m_ptable->m_vedit.size(); i++)
   {
      const IEditable *const pedit = m_ptable->m_vedit[i];
      if (pedit->GetItemType() == ItemTypeEnum::eItemPrimitive && ((const Primitive *)pedit)->IsPlayfield())
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
         m_ptable->m_undo.Undo(true);
      }
   }

   // Adjust the implicit playfield reflection probe
   RenderProbe *pf_reflection_probe = m_ptable->GetRenderProbe(RenderProbe::PLAYFIELD_REFLECTION_RENDERPROBE_NAME);
   if (pf_reflection_probe)
   {
      constexpr vec4 plane{0.f, 0.f, 1.f, 0.f};
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
   const float slope = lerp(minSlope, maxSlope, m_ptable->m_globalDifficulty);
   m_physics->SetGravity(slope, m_ptable->m_overridePhysics ? m_ptable->m_fOverrideGravityConstant : m_ptable->m_Gravity);

   InitFPS();

   //----------------------------------------------------------------------------------

   // We need to initialize the perf counter before creating the UI which uses it
   wintimer_init();
   m_liveUI = new LiveUI(m_renderer->m_renderDevice);
   m_liveUI->m_ballControl.LoadSettings(m_ptable->m_settings);

   m_ptable->m_tblMirrorEnabled = m_ptable->m_settings.GetPlayer_Mirror();
   #ifndef __STANDALONE__
   {
      const int vkLeftFlip = m_pininput.GetWindowVirtualKeyForAction(m_pininput.GetLeftFlipperActionId());
      const int vkRightFlip = m_pininput.GetWindowVirtualKeyForAction(m_pininput.GetRightFlipperActionId());
      const bool leftFlipPressed = (GetAsyncKeyState(VK_LSHIFT) & 0x8000) || ((vkLeftFlip != ~0u) && (GetAsyncKeyState(vkLeftFlip) & 0x8000));
      const bool rightFlipPressed = (GetAsyncKeyState(VK_RSHIFT) & 0x8000) || ((vkRightFlip != ~0u) && (GetAsyncKeyState(vkRightFlip) & 0x8000));

      // if both flippers are hold, then mirror table
      if (leftFlipPressed && rightFlipPressed)
      {
         PLOGI << "Both flipper buttons detected as pressed during load, enabling table mirroring";
         m_ptable->m_tblMirrorEnabled = true;
      }

      // if left flipper is hold during load, then swap DT/FS view (for quick testing)
      if (m_ptable->GetViewMode() != BG_FSS && !m_ptable->m_tblMirrorEnabled && leftFlipPressed)
      {
         PLOGI << "Left flipper button detected as pressed during load, swapping playfield/backglass view";
         switch (m_ptable->GetViewMode())
         {
         case BG_DESKTOP: m_ptable->SetViewSetupOverride(BG_FSS); break;
         case BG_FSS: m_ptable->SetViewSetupOverride(BG_DESKTOP); break;
         default: break;
         }
      }
   }
   #endif

   if (m_ptable->m_tblMirrorEnabled)
   {
      m_audioPlayer->SetMirrored(true);
      int rotation = (int)(m_ptable->GetViewSetup().GetRotation(m_renderer->m_stereo3D, m_playfieldWnd->GetWidth(), m_playfieldWnd->GetHeight())) / 90;
      m_renderer->GetMVP().SetFlip(rotation == 0 || rotation == 2 ? ModelViewProj::FLIPX : ModelViewProj::FLIPY);
   }

   m_progressDialog.SetProgress("Loading Textures..."s, 50);

   {
      tinyxml2::XMLDocument xmlDoc;
      tinyxml2::XMLElement *preloadCache = nullptr;
      if ((m_ptable->m_settings.GetPlayer_CacheMode() > 0) && FileExists(m_ptable->m_filename))
      {
         try
         {
            std::filesystem::path path = g_pvp->GetTablePath(m_ptable, VPinball::TableSubFolder::Cache, false) / "used_textures.xml";
            if (FileExists(path))
            {
               PLOGI << "Texture cache found at " << path;
               std::stringstream buffer;
               std::ifstream myFile(path);
               buffer << myFile.rdbuf();
               myFile.close();
               const string xml = buffer.str();
               if (xmlDoc.Parse(xml.c_str()) == tinyxml2::XML_SUCCESS)
                  preloadCache = xmlDoc.FirstChildElement("textures");
            }
         }
         catch (...) // something failed while trying to preload images
         {
            PLOGE << "Failed to load texture preload cache";
         }
      }

      std::mutex mutex;
      int nLoadInProgress = 0;
      vector<Texture *> failedPreloads;
      const unsigned int maxTexDim = static_cast<unsigned int>(m_ptable->m_settings.GetPlayer_MaxTexDimension());
      auto loadImage = [maxTexDim, &mutex, &nLoadInProgress, preloadCache, this, &failedPreloads](Texture *image, bool resizeOnLowMem)
      {
         bool readyToLoad = false;
         while (!readyToLoad)
         {
            {
               const std::lock_guard<std::mutex> lock(mutex);
               if (nLoadInProgress == 0)
                  readyToLoad = true;
               else
               {
                  size_t neededMem= image->GetEstimatedGPUSize() * 3; // 3x the estimated size is one for the image loader, one for the BaseTexture instance and one for the rendering API copy
                  #ifdef _MSC_VER
                     MEMORYSTATUSEX statex;
                     statex.dwLength = sizeof(statex);
                     GlobalMemoryStatusEx(&statex);
                     readyToLoad = statex.ullAvailPhys > neededMem;
                  #else
                     // TODO implement for other platforms
                     // struct sysinfo memInfo;
                     // sysinfo(&memInfo);
                     // readyToLoad = memInfo.freeram > neededMem;
                     readyToLoad = true;
                  #endif
               }
               if (readyToLoad)
                  nLoadInProgress++;
            }
            if (!readyToLoad)
               std::this_thread::sleep_for(std::chrono::milliseconds(100));
            else
            {
               const auto buffer = image->GetRawBitmap(resizeOnLowMem, maxTexDim);
               const std::lock_guard<std::mutex> lock(mutex);
               if (buffer)
               {
                  image->IsOpaque();
                  bool uploaded = false;
                  if (preloadCache)
                     for (auto node = preloadCache->FirstChildElement("texture"); node != nullptr; node = node->NextSiblingElement())
                     {
                        bool linearRGB = false;
                        const char *name = node->GetText();
                        if (name != nullptr && image->m_name == name && node->QueryBoolAttribute("linear", &linearRGB) == tinyxml2::XML_SUCCESS)
                        {
                           #ifdef ENABLE_OPENGL
                           // Uploading texture in OpenGL uses the state machine which will be wrong if done concurrently
                           const std::lock_guard<std::mutex> lock2(mutex);
                           #endif
                           m_renderer->m_renderDevice->UploadTexture(image, linearRGB);
                           uploaded = true;
                           break;
                        }
                     }
                  if (!uploaded)
                  {
                     // We could upload all images, but this would need support for dynamic change of 'force linear' and would lead to load all VR textures on lower end systems
                     // Instead we register it to the texture manager that will hold a strong reference and upload when needed
                     m_renderer->m_renderDevice->m_texMan.AddPendingUpload(image);
                  }
                  if ((image->m_width > buffer->width()) || (image->m_height > buffer->height()))
                  {
                     const bool isError = (buffer->width() < maxTexDim) || (buffer->height() < maxTexDim);
                     PLOG(isError ? plog::Severity::error : plog::Severity::warning) << "Image '" << image->m_name << "' was downsized from "
                           << image->m_width << 'x' << image->m_height << " to " << buffer->width() << 'x' << buffer->height() << (isError ? " due to low memory " : " due to user settings");
                     if (isError)
                        m_liveUI->PushNotification("Image '" + image->m_name + "' was downsized due to low memory", 5000);
                  }
                  PLOGD << "Image '" << image->m_name << "' loaded to " << (uploaded ? "GPU" : "RAM");
               }
               else if (resizeOnLowMem)
               {
                  PLOGE << "Image '" << image->m_name << "' could not be loaded, skipping it";
                  m_liveUI->PushNotification("Image '" + image->m_name + "' could not be loaded", 5000);
                  m_renderer->m_renderDevice->m_texMan.AddPlaceHolder(image);
               }
               else
               {
                  failedPreloads.push_back(image);
               }
            }
         }
         {
            const std::lock_guard<std::mutex> lock(mutex);
            nLoadInProgress--;
         }
      };

      // Try to load all image concurrently. Note that this dramatically increases the amount of temporary memory needed, especially if Max Texture Dimension is set (as then all the additional conversion/rescale mem is also needed 'in parallel')
      #ifdef ENABLE_BGFX
      m_renderer->m_renderDevice->m_frameMutex.unlock();
      #endif
      ThreadPool pool(g_pvp->GetLogicalNumberOfProcessors());
      for (auto image : m_ptable->m_vimage)
         pool.enqueue(loadImage, image, false);
      pool.wait_until_empty();
      pool.wait_until_nothing_in_flight();
      #ifdef ENABLE_BGFX
      m_renderer->m_renderDevice->m_frameMutex.lock();
      #endif

      // Due to multithreaded loading and pre-allocation, check if some images could not be loaded, and perform a retry since more memory is available now
      for (auto image : failedPreloads)
         loadImage(image, true);
   }

   //----------------------------------------------------------------------------------

   PLOGI << "Initializing renderer"; // For profiling
   m_progressDialog.SetProgress("Initializing Renderer..."s, 60);

   // Setup rendering and timers
   RenderState state;
   state.SetRenderState(RenderState::CULLMODE, m_ptable->m_tblMirrorEnabled ? RenderState::CULL_CW : RenderState::CULL_CCW);
   m_renderer->m_renderDevice->CopyRenderStates(false, state);
   m_renderer->m_renderDevice->SetDefaultRenderState();
   m_renderer->SetAnisoFiltering(m_ptable->m_settings.GetPlayer_ForceAnisotropicFiltering());
   m_renderer->InitLayout();
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

   #if defined(EXT_CAPTURE)
   if (m_renderer->m_stereo3D == STEREO_VR)
   {
      if (m_capExtDMD)
         StartDMDCapture();
      if (m_capPUP)
         StartPUPCapture();
   }
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
            const ItemTypeEnum type = ph->GetItemType();
            if (type == ItemTypeEnum::eItemBumper || type == ItemTypeEnum::eItemDispReel || type == ItemTypeEnum::eItemFlipper || type == ItemTypeEnum::eItemGate
               || type == ItemTypeEnum::eItemHitTarget || type == ItemTypeEnum::eItemLight || type == ItemTypeEnum::eItemSpinner || type == ItemTypeEnum::eItemTrigger)
               ph->GetIHitable()->GetEventProxyBase()->FireVoidEvent(DISPID_AnimateEvents_Animate);
         }
      }
      m_ptable->FireOptionEvent(PinTable::OptionEventType::Initialized);
      m_ptable->FireVoidEvent(DISPID_GameEvents_Paused);
   }

   // Apply cabinet autofit (after script startup as the script may change what is visible and therefore taken in account, like a VR cabinet model)
   SetCabinetAutoFitMode(m_ptable->m_settings.GetPlayer_CabinetAutofitMode());
   SetCabinetAutoFitPos(m_ptable->m_settings.GetPlayer_CabinetAutofitPos());

   // Initialize stereo rendering
   m_renderer->UpdateStereoShaderState();

#ifdef PLAYBACK
   if (m_playback)
      m_fplaylog = fopen("c:\\badlog.txt", "r");
#endif

   const MsgPluginAPI *msgApi = &MsgPI::MsgPluginManager::GetInstance().GetMsgAPI();

   m_onPrepareFrameMsgId = msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_EVT_ON_PREPARE_FRAME);
   m_onAudioUpdatedMsgId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_AUDIO_ON_UPDATE_MSG);
   msgApi->SubscribeMsg(VPXPluginAPIImpl::GetInstance().GetVPXEndPointId(), m_onAudioUpdatedMsgId, OnAudioUpdated, this);

   m_getAuxRendererId = msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_MSG_GET_AUX_RENDERER);
   m_onAuxRendererChgId = msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_EVT_AUX_RENDERER_CHG);
   msgApi->SubscribeMsg(VPXPluginAPIImpl::GetInstance().GetVPXEndPointId(), m_onAuxRendererChgId, OnAuxRendererChanged, this);
   OnAuxRendererChanged(m_onAuxRendererChgId, this, nullptr);

   // Signal plugins before performing static prerendering. The only thing not fully initialized is the physics (is this ok ?)
   VPXPluginAPIImpl::GetInstance().OnGameStart();

   // Open UI if requested (this also disables static prerendering, so must be done before performing it)
   if (playMode == 1)
      m_liveUI->OpenInGameUI();
   else if (playMode == 2 && m_renderer->m_stereo3D != STEREO_VR)
      m_liveUI->OpenEditorUI();

   // Pre-render all non-changing elements such as static walls, rails, backdrops, etc. and also static playfield reflections
   // This is done after starting the script and firing the Init event to allow script to adjust static parts on startup
   PLOGI << "Prerendering static parts"; // For profiling
   wintimer_init();
   m_physics->StartPhysics();
   m_renderer->RenderFrame();

   // Reset the perf counter to start time when physics starts
   wintimer_init();
   m_physics->StartPhysics();

   m_progressDialog.SetProgress("Starting..."s, 100);
   if (!IsEditorMode())
      m_ptable->FireVoidEvent(DISPID_GameEvents_UnPaused);

   PLOGI << "Startup done"; // For profiling

#ifdef __LIBVPINBALL__
   VPinballLib::VPinballLib::SendEvent(VPINBALL_EVENT_PLAYER_STARTED, nullptr);
#endif

#ifndef __STANDALONE__
   // Disable editor (Note that now that the played table use a copy, we could allow editing while playing but problem may arise with shared parts like images and mesh data)
   g_pvp->GetPropertiesDocker()->EnableWindow(FALSE);
   g_pvp->GetLayersDocker()->EnableWindow(FALSE);
   g_pvp->GetToolbarDocker()->EnableWindow(FALSE);
   if(g_pvp->GetNotesDocker()!=nullptr)
      g_pvp->GetNotesDocker()->EnableWindow(FALSE);
   if (m_ptable->m_liveBaseTable)
      m_ptable->m_liveBaseTable->EnableWindow(FALSE);
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
   m_playfieldWnd->Show();
   m_playfieldWnd->RaiseAndFocus();

   // Popup notification on startup
   if (m_renderer->m_stereo3D != STEREO_OFF && m_renderer->m_stereo3D != STEREO_VR && !m_renderer->m_stereo3Denabled)
      m_liveUI->PushNotification("3D Stereo is enabled but currently toggled off"s, 4000);
   const int numberOfTimesToShowTouchMessage = g_pvp->m_settings.GetPlayer_NumberOfTimesToShowTouchMessage();
   if (m_pininput.HasTouchInput() && numberOfTimesToShowTouchMessage != 0) //!! visualize with real buttons or at least the areas?? Add extra buttons?
   {
      g_pvp->m_settings.SetPlayer_NumberOfTimesToShowTouchMessage(max(numberOfTimesToShowTouchMessage - 1, 0), false);
      m_liveUI->PushNotification("You can use Touch controls on this display: bottom left area to Start Game, bottom right area to use the Plunger\n"
                                 "lower left/right for Flippers, upper left/right for Magna buttons, top left for Credits and (hold) top right to Exit"s, 12000);
   }
}

Player::~Player()
{
   assert(g_pplayer == this && g_pplayer->m_closing != CS_CLOSED);

   // note if application exit was requested, and set the new closing state to CLOSED
   const bool appExitRequested = (m_closing == CS_CLOSE_APP);
   m_closing = CS_CLOSED;
   PLOGI << "Closing player...";

   // Signal plugins early since most fields will become invalid
   VPXPluginAPIImpl::GetInstance().OnGameEnd();

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
   const MsgPluginAPI *msgApi = &MsgPI::MsgPluginManager::GetInstance().GetMsgAPI();
   msgApi->UnsubscribeMsg(m_onAudioUpdatedMsgId, OnAudioUpdated);
   msgApi->ReleaseMsgID(m_onAudioUpdatedMsgId);
   msgApi->ReleaseMsgID(m_onPrepareFrameMsgId);
   msgApi->UnsubscribeMsg(m_onAuxRendererChgId, OnAuxRendererChanged);
   msgApi->ReleaseMsgID(m_getAuxRendererId);
   msgApi->ReleaseMsgID(m_onAuxRendererChgId);

   // Save list of used textures to avoid stuttering in next play
   if ((m_ptable->m_settings.GetPlayer_CacheMode() > 0) && FileExists(m_ptable->m_filename))
   {
      try
      {
         std::filesystem::path dir = g_pvp->GetTablePath(m_ptable, VPinball::TableSubFolder::Cache, true);
         tinyxml2::XMLDocument xmlDoc;
         tinyxml2::XMLElement *root;
         ankerl::unordered_dense::map<string, tinyxml2::XMLElement *> textureAge;
         const std::filesystem::path path = dir / "used_textures.xml";
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
               for (tinyxml2::XMLElement *node = root->FirstChildElement("texture"); node != nullptr; node = node->NextSiblingElement())
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
               for (tinyxml2::XMLElement *node : toRemove)
                  root->DeleteChild(node);
            }
         }
         else
         {
            root = xmlDoc.NewElement("textures");
            xmlDoc.InsertEndChild(xmlDoc.NewDeclaration());
            xmlDoc.InsertEndChild(root);
         }

         vector<ITexManCacheable *> textures = m_renderer->m_renderDevice->m_texMan.GetLoadedTextures();
         for (ITexManCacheable *memtex : textures)
         {
            auto tex = std::ranges::find_if(m_ptable->m_vimage.begin(), m_ptable->m_vimage.end(), [&memtex](Texture *&x) { return (!x->m_name.empty()) && x == memtex; });
            if (tex != m_ptable->m_vimage.end())
            {
               tinyxml2::XMLElement *node = textureAge[(*tex)->m_name];
               if (node == nullptr)
               {
                  node = xmlDoc.NewElement("texture");
                  node->SetText((*tex)->m_name.c_str());
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
      catch (...)
      {
         PLOGE << "Failed to update list of texture used in table cache";
      }
   }

   // Save adjusted VR settings
   if (m_renderer->m_stereo3D == STEREO_VR)
      m_vrDevice->SaveVRSettings(g_pvp->m_settings);

   // FIXME remove or at least move legacy ushock to a plugin
   ushock_output_shutdown();

#ifdef EXT_CAPTURE
   StopCaptures();
   g_DXGIRegistry.ReleaseAll();
#endif

   delete m_liveUI;
   m_liveUI = nullptr;
   delete m_physics;
   m_physics = nullptr;

   #ifdef ENABLE_DX9
      m_renderer->m_renderDevice->m_basicShader->UnbindSamplers();
      m_renderer->m_renderDevice->m_DMDShader->UnbindSamplers();
      m_renderer->m_renderDevice->m_FBShader->UnbindSamplers();
      m_renderer->m_renderDevice->m_flasherShader->UnbindSamplers();
      m_renderer->m_renderDevice->m_lightShader->UnbindSamplers();
      m_renderer->m_renderDevice->m_ballShader->UnbindSamplers();
   #endif

   for (auto probe : m_ptable->m_vrenderprobe)
      probe->RenderRelease();
   for (auto renderable : m_vhitables)
      renderable->GetIHitable()->RenderRelease();
   for (auto hitable : m_vhitables)
      hitable->GetIHitable()->TimerRelease();
   assert(m_vballDelete.empty());
   m_vball.clear();

   if (m_implicitPlayfieldMesh && FindIndexOf(m_ptable->m_vedit, (IEditable *)m_implicitPlayfieldMesh) != -1)
   {
      RemoveFromVectorSingle(m_ptable->m_vedit, (IEditable *)m_implicitPlayfieldMesh);
      m_ptable->m_pcv->RemoveItem(m_implicitPlayfieldMesh->GetScriptable());
      m_implicitPlayfieldMesh->Release();
      m_implicitPlayfieldMesh = nullptr;
   }

   m_renderer->m_renderDevice->m_DMDShader->SetTextureNull(SHADER_tex_dmd);
   if (m_dmdFrame)
   {
      m_renderer->m_renderDevice->m_texMan.UnloadTexture(m_dmdFrame.get());
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
      if (m_ptable->m_liveBaseTable)
      {
         m_ptable->m_liveBaseTable->EnableWindow();
         m_ptable->m_liveBaseTable->SetFocus();
         m_ptable->m_liveBaseTable->SetActiveWindow();
         m_ptable->m_liveBaseTable->SetDirtyDraw();
         m_ptable->m_liveBaseTable->RefreshProperties();
         m_ptable->m_liveBaseTable->BeginAutoSaveCounter();
      }
   }

   ::UnregisterClass(WIN32_PLAYER_WND_CLASSNAME, g_pvp->theInstance);
   SDL_UnregisterApp();
#endif

   // If the table is a shallow copy, then we own it and need to dispose it
   if (m_ptable->m_liveBaseTable)
      delete m_ptable;

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
   PLOGI << "Player closed.";

#ifdef __LIBVPINBALL__
   VPinballLib::VPinballLib::SendEvent(VPINBALL_EVENT_PLAYER_CLOSED, nullptr);
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

void Player::SetPlayState(const bool isPlaying, const uint32_t delayBeforePauseMs)
{
   const bool wasPlaying = IsPlaying();
   if (isPlaying || delayBeforePauseMs == 0)
   {
      m_pauseTimeTarget = 0;
      const bool willPlay = isPlaying && m_playfieldWnd->IsFocused();
      if (wasPlaying != willPlay)
      {
         ApplyPlayingState(willPlay);
         m_playing = isPlaying;
      }
   }
   else if (wasPlaying)
      m_pauseTimeTarget = m_time_msec + delayBeforePauseMs;
}

void Player::OnFocusChanged()
{
   // A lost focus event happens during player destruction when the main window is destroyed
   if (m_closing == CS_CLOSED)
      return;
   if (m_playfieldWnd->IsFocused())
   {
      PLOGI << "Playfield window gained focus";
   }
   else
   {
      #ifdef _MSC_VER
         HWND foregroundWnd = GetForegroundWindow();
         if (foregroundWnd)
         {
            string focusedWnd = "undefined"s;
            DWORD foregroundProcessId;
            const DWORD foregroundThreadId = GetWindowThreadProcessId(foregroundWnd, &foregroundProcessId);
            char tmp[MAXSTRING];
            if (foregroundProcessId)
            {
               HANDLE foregroundProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION /* PROCESS_QUERY_INFORMATION | PROCESS_VM_READ */, FALSE, foregroundProcessId);
               if (foregroundProcess)
               {
                  if (GetProcessImageFileName(foregroundProcess, tmp, std::size(tmp)))
                     focusedWnd = tmp;
               }
            }
            GetWindowText(foregroundWnd, tmp, std::size(tmp));
            PLOGI << "Playfield window lost focus to window with title: '" << tmp << "' created by application: " << focusedWnd;
         }
         else
         {
            PLOGI << "Playfield window lost focus.";
         }

      #else
         PLOGI << "Playfield window lost focus.";
      #endif
   }
   const bool wasPlaying = IsPlaying();
   const bool willPlay = m_playing && m_playfieldWnd->IsFocused();
   if (wasPlaying != willPlay)
      ApplyPlayingState(willPlay);
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
   if (m_liveUI->m_ballControl.GetDraggedBall() == pHitBall)
      m_liveUI->m_ballControl.SetDraggedBall(nullptr);
}

void Player::SetCabinetAutoFitMode(int mode)
{
   m_cabinetAutoFitMode = mode;
   if (m_cabinetAutoFitMode != 0)
   {
      Vertex3Ds playerPos(m_ptable->m_settings.GetPlayer_ScreenPlayerX(), m_ptable->m_settings.GetPlayer_ScreenPlayerY(), m_ptable->m_settings.GetPlayer_ScreenPlayerZ());
      m_ptable->GetViewSetup().SetWindowAutofit(m_ptable, playerPos, m_renderer->GetDisplayAspectRatio(), m_cabinetAutoFitPos, m_cabinetAutoFitMode == 2, [](string) { });
   }
}

void Player::SetCabinetAutoFitPos(float pos)
{
   m_cabinetAutoFitPos = pos;
   if (m_cabinetAutoFitMode == 1)
   {
      Vertex3Ds playerPos(m_ptable->m_settings.GetPlayer_ScreenPlayerX(), m_ptable->m_settings.GetPlayer_ScreenPlayerY(), m_ptable->m_settings.GetPlayer_ScreenPlayerZ());
      m_ptable->GetViewSetup().SetWindowAutofit(m_ptable, playerPos, m_renderer->GetDisplayAspectRatio(), m_cabinetAutoFitPos, m_cabinetAutoFitMode == 2, [](string) { });
   }
}

void Player::FireSyncTimer(int timerValue)
{
   // Legacy implementation: timers with magic interval value have special behaviors: -2 for controller sync event
   for (HitTimer *const pht : m_vht)
      if (pht->m_interval == timerValue)
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


string Player::GetPerfInfo()
{
   // Make it more or less readable by updating only once per second
   static string txt;
   static uint32_t lastUpdate = 0;
   const uint32_t now = msec();
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
        << ((m_renderer->GetNPrerenderTris() + m_renderer->m_renderDevice->m_frameDrawnTriangles + 999) / 1000) << "k overall.\n";
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
      << ((int64_t)(m_pininput.m_leftkey_down_usec_rotate_to_end - m_pininput.m_leftkey_down_usec) < 0 ? int_as_float(0x7FC00000) : (double)(m_pininput.m_leftkey_down_usec_rotate_to_end - m_pininput.m_leftkey_down_usec) / 1000.) << " ms ("
      << ((int)(m_pininput.m_leftkey_down_frame_rotate_to_end - m_pininput.m_leftkey_down_frame) < 0 ? -1 : (int)(m_pininput.m_leftkey_down_frame_rotate_to_end - m_pininput.m_leftkey_down_frame)) << " f) to eos: "
      << ((int64_t)(m_pininput.m_leftkey_down_usec_EOS - m_pininput.m_leftkey_down_usec) < 0 ? int_as_float(0x7FC00000) : (double)(m_pininput.m_leftkey_down_usec_EOS - m_pininput.m_leftkey_down_usec) / 1000.) << " ms ("
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

void Player::ProcessOSMessages()
{
   const uint64_t startTick = usec();
   SDL_Event e;
   bool isPFWnd = true;
   static Vertex2D dragStart;
   static int dragging = 0;
#ifndef __LIBVPINBALL__
   while (SDL_PollEvent(&e) != 0)
#else
   while (VPinballLib::VPinballLib::Instance().PollAppEvent(e))
#endif
   {
      switch (e.type)
      {
      case SDL_EVENT_QUIT: SetCloseState(Player::CloseState::CS_STOP_PLAY); break;
      case SDL_EVENT_WINDOW_FOCUS_GAINED:
      case SDL_EVENT_WINDOW_FOCUS_LOST:
         isPFWnd = SDL_GetWindowFromID(e.window.windowID) == m_playfieldWnd->GetCore();
         OnFocusChanged();
         break;

      case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
         isPFWnd = SDL_GetWindowFromID(e.window.windowID) == m_playfieldWnd->GetCore();
         SetCloseState(Player::CloseState::CS_STOP_PLAY);
         break;

      case SDL_EVENT_KEY_UP:
      case SDL_EVENT_KEY_DOWN:
         isPFWnd = SDL_GetWindowFromID(e.key.windowID) == m_playfieldWnd->GetCore();
         ShowMouseCursor(false);
         break;

      case SDL_EVENT_TEXT_INPUT: isPFWnd = SDL_GetWindowFromID(e.text.windowID) == m_playfieldWnd->GetCore(); break;
      case SDL_EVENT_MOUSE_WHEEL: isPFWnd = SDL_GetWindowFromID(e.wheel.windowID) == m_playfieldWnd->GetCore(); break;
      case SDL_EVENT_MOUSE_BUTTON_DOWN:
      case SDL_EVENT_MOUSE_BUTTON_UP:
         isPFWnd = SDL_GetWindowFromID(e.button.windowID) == m_playfieldWnd->GetCore();
         if (!isPFWnd)
         {
            if (e.type == SDL_EVENT_MOUSE_BUTTON_UP)
               dragging = 0;
            else if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN && dragging == 0)
               dragging = 1;
         }
         break;

      case SDL_EVENT_MOUSE_MOTION:
         isPFWnd = SDL_GetWindowFromID(e.motion.windowID) == m_playfieldWnd->GetCore();
         if (isPFWnd)
         {
            static float m_lastcursorx = FLT_MAX, m_lastcursory = FLT_MAX;
            if (m_lastcursorx != e.motion.x || m_lastcursory != e.motion.y)
            {
               m_lastcursorx = e.motion.x;
               m_lastcursory = e.motion.y;
               ShowMouseCursor(true);
            }
         }
         else if (dragging)
         {
            // Handle dragging of auxiliary windows
            static const std::vector<string> settingPages = {
               "settings/display_backglass"s,
               "settings/display_scoreview"s,
               "settings/display_topper"s,
            };
            const SDL_Window *const sdlWnd = SDL_GetWindowFromID(e.motion.windowID);
            std::vector<VPX::Window *> windows = {
               m_backglassOutput.GetWindow(),
               m_scoreViewOutput.GetWindow(),
               m_topperOutput.GetWindow(),
            };
            for (int i = 0; i <3; i++)
            {
               VPX::Window *wnd = windows[i];
               if (wnd && (sdlWnd == wnd->GetCore()) && m_liveUI->m_inGameUI.IsOpened(settingPages[i]))
               {
                  SDL_Point point;
                  wnd->GetPos(point.x, point.y);
                  SDL_DisplayID display = SDL_GetDisplayForPoint(&point);
                  Vertex2D click((float)point.x + e.motion.x, (float)point.y + e.motion.y);
                  point.x = static_cast<int>(static_cast<float>(point.x) + click.x - dragStart.x);
                  point.y = static_cast<int>(static_cast<float>(point.y) + click.y - dragStart.y);
                  if (display)
                  {
                     SDL_Rect bounds;
                     SDL_GetDisplayBounds(display, &bounds);
                     point.x = clamp(point.x, bounds.x, bounds.x + bounds.w - wnd->GetWidth());
                     point.y = clamp(point.y, bounds.y, bounds.y + bounds.h - wnd->GetHeight());
                  }
                  if (dragging > 1)
                     wnd->SetPos(point.x, point.y);
                  dragStart = click;
                  dragging = 2;
                  break;
               }
            }
         }
         break;
      }

      // Forward events to ImGui, including touch/pen events which are forwarded as mouse events
      if (isPFWnd)
         m_liveUI->HandleSDLEvent(e);

      m_pininput.HandleSDLEvent(e);

      // Limit to 1ms of OS message processing per call
      if ((usec() - startTick) > 1000ull)
         break;
   }
};

class AttractCapture
{
public:
   AttractCapture(Player* player)
      : m_player(player)
      , m_captureTime(usec())
      , m_captureStartupEndTime(usec())
      , m_captureStartupEndPhysicsTime(usec())
      , m_lightStates(g_pvp->m_captureAttract)
   {
      m_nLights = 0;
      for (const auto &edit : m_player->m_ptable->m_vedit)
         if (edit->GetItemType() == eItemLight)
            m_nLights++;

      m_captureRequestMask = 1;
      if (m_player->m_backglassOutput.GetMode() == RenderOutput::OutputMode::OM_WINDOW)
         m_captureRequestMask |= 2;
      #if defined(ENABLE_BGFX)
         if (bgfx::getCaps()->rendererType == bgfx::RendererType::Metal) // Metal backend does not support screenshot from other framebuffers
            m_captureRequestMask &= 1;
      #endif
   }
   
   void Update()
   {
      std::lock_guard lock(m_captureMutex);

      m_player->m_physics->UpdatePhysics(max(m_captureTime, m_player->m_physics->GetCurrentTime()));
      m_player->FireSyncTimer(-2);
      
      // Fast forward to capture start time (startup +30s)
      while (m_player->m_physics->GetCurrentTime() < m_player->m_physics->GetStartTime() + 30 * 1000000)
      {
         m_captureTime = min(m_captureTime + 1000000 / 120, m_player->m_physics->GetStartTime() + 30 * 1000000 + PHYSICS_STEPTIME);
         m_player->ApplyDeferredTimerChanges();
         m_player->m_overall_frames++;
         const float diff_time_msec = (float)(m_player->m_time_msec - m_player->m_last_frame_time_msec);
         m_player->m_last_frame_time_msec = m_player->m_time_msec;
         for (size_t i = 0; i < m_player->m_ptable->m_vedit.size(); ++i)
            if (Hitable *const ph = m_player->m_ptable->m_vedit[i]->GetIHitable(); ph)
               ph->UpdateAnimation(diff_time_msec);
         m_player->FireSyncTimer(-1);
         m_player->FireSyncTimer(-2);
         m_player->m_physics->UpdatePhysics(m_captureTime);
         MsgPI::MsgPluginManager::GetInstance().ProcessAsyncCallbacks();
         m_captureStartupEndTime = usec();
         m_captureStartupEndPhysicsTime = m_player->m_physics->GetCurrentTime();
      }
      
      // Run 1s of normal emulation to stabilize
      if (m_player->m_physics->GetCurrentTime() < m_captureStartupEndPhysicsTime + 1000000)
      {
         m_captureTime = m_captureStartupEndPhysicsTime + usec() - m_captureStartupEndTime;
      }
      
      // Stepped emulation & rendering at the capture frequency
      else if (!m_captureRequested && m_player->GetCloseState() == Player::CS_PLAYING)
      {
         m_captureRequested = true;
         switch (m_captureRequestMask)
         {
         case 1:
            m_player->m_renderer->m_renderDevice->CaptureScreenshot(
               { m_player->m_playfieldWnd }, { GetFilename(VPXWindowId::VPXWINDOW_Playfield, m_captureFrameNumber, true) },
               [this](bool success) { OnCapture(success); }, 1);
            break;
         case 3:
            m_player->m_renderer->m_renderDevice->CaptureScreenshot(
               { m_player->m_playfieldWnd, m_player->m_backglassOutput.GetWindow() },
               { GetFilename(VPXWindowId::VPXWINDOW_Playfield, m_captureFrameNumber, true), GetFilename(VPXWindowId::VPXWINDOW_Backglass, m_captureFrameNumber, true) },
               [this](bool success) { OnCapture(success); }, 1);
            break;
         }
      }
   }
   
private:
   void OnCapture(bool success)
   {
      std::lock_guard lock(m_captureMutex);

      if (!success)
      {
         PLOGE << "Screenshot capture failed. Attract video capture cancelled.";
         m_player->SetCloseState(Player::CloseState::CS_CLOSE_APP);
         return;
      }

      // Request next capture (from main thread)
      m_captureRequested = false;

      // Store and log light state
      std::stringstream ss;
      for (const auto& edit : m_player->m_ptable->m_vedit)
      {
         if (edit->GetItemType() == eItemLight)
         {
            Light* const light = static_cast<Light*>(edit);
            const float state = (light->m_d.m_intensity * light->m_d.m_intensity_scale) == 0.f ? 0.f :
               clamp(light->m_currentIntensity / (light->m_d.m_intensity * light->m_d.m_intensity_scale), 0.f, 1.f);
            m_lightStates[m_captureFrameNumber - 1].push_back(state);
            if (static_cast<int>(state * 9.f) == 0)
               ss << ' ';
            else
               ss << static_cast<int>(state * 9.f);
         }
      }
      
      // Evaluate best loop against previous frames
      int minLoopLength = max(5, g_pvp->m_captureAttract / 4);
      if (m_captureFrameNumber > minLoopLength)
      {
         float lowestDistance = FLT_MAX;
         int bestStart = -1;
         for (int j = 0; j < m_captureFrameNumber - minLoopLength; j++)
         {
            // distance favor longer loops with lowest difference between light states
            float distance = 1.f;
            for (int k = 0; k < m_nLights; k++)
               distance += powf(m_lightStates[m_captureFrameNumber - 1][k] - m_lightStates[j][k], 2.f);
            distance = distance * 100.f / static_cast<float>(m_nLights); // Normalize against a 'standard' number of lights
            distance = distance / static_cast<float>(m_captureFrameNumber - j); // Take loop length in account
            if (distance < lowestDistance)
            {
               lowestDistance = distance;
               bestStart = j;
            }
         }
         ss << " Best loop: #" << std::setw(2) << (bestStart + 1) << ", length: " << std::setw(2) << (m_captureFrameNumber - bestStart) << " (error: " << lowestDistance << ')';
         if (lowestDistance < m_bestLoopDistance)
         {
            m_bestLoopDistance = lowestDistance;
            m_bestLoopStart = bestStart;
            m_bestLoopEnd = m_captureFrameNumber - 1;
         }
      }

      // Step simulation & request next frame
      PLOGI << "Captured frame #" << std::setw(2) << m_captureFrameNumber << ", State of " << m_nLights << " lights : " << ss.str();
      m_captureFrameNumber++;
      m_captureTime += 1000000 / g_pvp->m_captureAttractFPS;
      if (m_captureFrameNumber <= g_pvp->m_captureAttract)
         return;
      
      // Capture is finished, process result and exit
      m_player->SetCloseState(Player::CloseState::CS_CLOSE_APP);
      if (!g_pvp->m_captureAttractLoop)
         return;

      // Evaluate the less lit frame to use it as the first of our loop since playback loop stutters are a bit less obvious on dark frames
      float minLightFrame = FLT_MAX;
      int minLightFrameIndex = -1;
      for (int i = m_bestLoopStart; i < m_bestLoopEnd; i++)
      {
         float totalLight = 0.f;
         for (int k = 0; k < m_nLights; k++)
            totalLight += m_lightStates[i][k];
         if (totalLight < minLightFrame)
         {
            minLightFrame = totalLight;
            minLightFrameIndex = i - m_bestLoopStart;
         }
      }

      PLOGI << "Truncating captured sequence to the best loop found from #" << (m_bestLoopStart + 1) << " to #" << m_bestLoopEnd; // Exclude last frame to actually get a loop
      for (int w = 0; w < 2; w++)
      {
         const VPXWindowId wndId = w == 0 ? VPXWindowId::VPXWINDOW_Playfield : VPXWindowId::VPXWINDOW_Backglass;
         if (wndId == VPXWindowId::VPXWINDOW_Backglass && ((m_captureRequestMask & 2) == 0))
            continue;
         for (int i = 0; i < m_bestLoopStart; i++)
            std::filesystem::remove(GetFilename(wndId, i + 1, true));
         for (int i = m_bestLoopStart; i < m_bestLoopEnd; i++)
            std::filesystem::rename(GetFilename(wndId, i + 1, true), GetFilename(wndId, i - m_bestLoopStart + 1, true));
         for (int i = m_bestLoopEnd; i <= g_pvp->m_captureAttract; i++)
            std::filesystem::remove(GetFilename(wndId, i + 1, true));
         for (int i = 0; i < m_bestLoopEnd - m_bestLoopStart; i++)
            if (i < minLightFrameIndex)
               std::filesystem::rename(GetFilename(wndId, i + 1, true), GetFilename(wndId, i - minLightFrameIndex + 1 + (m_bestLoopEnd - m_bestLoopStart), false));
            else
               std::filesystem::rename(GetFilename(wndId, i + 1, true), GetFilename(wndId, i - minLightFrameIndex + 1, false));
      }
   }

   string GetFilename(VPXWindowId id, int index, bool isTmp) const
   {
      // The critical path is disk access and memory management:
      // - png is well compressed but far too slow
      // - bmp is fast to save but huge on disk (multiple times faster than png, but huge)
      // - qoi is both faster to save and small enough on disk (twice faster than bmp)
      // So we use qoi as it offers a good balance and is lossless and supported by all major video tools (ffmpeg, vlc,...)
      std::stringstream ss;
      ss << PathFromFilename(m_player->m_ptable->m_filename) << "Capture" << PATH_SEPARATOR_CHAR
         << (id == VPXWindowId::VPXWINDOW_Playfield ? "Playfield_" : 
             id == VPXWindowId::VPXWINDOW_Backglass ? "Backglass_" : "") 
         << std::setw(5) << std::setfill('0') << index << (isTmp ? "_tmp.qoi" : ".qoi");
      return ss.str();
   };

   Player *const m_player;

   std::mutex m_captureMutex;

   int m_captureRequestMask;
   int m_captureFrameNumber = 1;
   bool m_captureRequested = false;
   
   uint64_t m_captureTime;
   uint64_t m_captureStartupEndTime;
   uint64_t m_captureStartupEndPhysicsTime;
   
   int m_nLights;
   vector<vector<float>> m_lightStates;
   
   int m_bestLoopStart = -1;
   int m_bestLoopEnd = -1;
   double m_bestLoopDistance = FLT_MAX;
};

void Player::UpdateGameLogic()
{
   #ifdef MSVC_CONCURRENCY_VIEWER
   //series.write_flag(_T("Sync"));
   span *tagSpan = new span(series, 1, _T("Sync"));
   #endif

   ProcessOSMessages();

   if (g_pvp->m_captureAttract)
   {
      static std::unique_ptr<AttractCapture> capture;
      if (capture == nullptr)
         capture = std::make_unique<AttractCapture>(this);
      capture->Update();
   }
   else if (!IsEditorMode())
   {
      m_pininput.ProcessInput(); // Trigger key events to sync with controller
      m_physics->UpdatePhysics(usec()); // Update physics (also triggering events, syncing with controller)
      // TODO These updates should also be done directly in the physics engine after collision events
      FireSyncTimer(-2); // Trigger script sync event (to sync solenoids back)
   }

   MsgPI::MsgPluginManager::GetInstance().ProcessAsyncCallbacks();

   #ifdef MSVC_CONCURRENCY_VIEWER
   delete tagSpan;
   #endif
}

void Player::GameLoop()
{
   // Stereo must be run unthrottled to let OpenVR set the frame pace according to the head set
   assert(!(m_renderer->m_stereo3D == STEREO_VR && (m_videoSyncMode != VideoSyncMode::VSM_NONE || m_maxFramerate < 1000.f)));

   #ifdef ENABLE_BGFX
      // Flush any pending frame
      m_renderer->m_renderDevice->m_frameReadySem.post();

      m_renderer->m_renderDevice->m_frameMutex.unlock();
      m_logicProfiler.SetThreadLock();

      #ifdef __LIBVPINBALL__
         auto gameLoop = [this]() {
            MultithreadedGameLoop();
         };
         VPinballLib::VPinballLib::Instance().SetGameLoop(gameLoop);
      #else
         MultithreadedGameLoop();
      #endif
   #else
      delete m_renderProfiler;
      m_renderProfiler = &m_logicProfiler;
      if (m_videoSyncMode == VideoSyncMode::VSM_FRAME_PACING)
         FramePacingGameLoop();
      else
         GPUQueueStuffingGameLoop();
   #endif
}

void Player::MultithreadedGameLoop()
{
#ifdef ENABLE_BGFX
   while (GetCloseState() == CS_PLAYING || GetCloseState() == CS_USER_INPUT
#ifdef __LIBVPINBALL__
      || GetCloseState() == CS_CLOSE_CAPTURE_SCREENSHOT
#endif
   )
   {
      // Continuously process input, synchronize with emulation and step physics to keep latency low
      UpdateGameLogic();

      // If rendering thread is ready, push a new frame as soon as possible
      if (!m_renderer->m_renderDevice->m_framePending && m_renderer->m_renderDevice->m_frameMutex.try_lock())
      {
         FinishFrame();
         m_lastFrameSyncOnFPS = (m_videoSyncMode != VideoSyncMode::VSM_NONE) && ((m_logicProfiler.GetSlidingAvg(FrameProfiler::PROFILE_FRAME) - 100) * m_playfieldWnd->GetRefreshRate() < 1000000);
         PrepareFrame();
         m_renderer->m_renderDevice->m_framePending = true;
         m_renderer->m_renderDevice->m_frameReadySem.post();
         m_renderer->m_renderDevice->m_frameMutex.unlock();
      }
#ifndef __LIBVPINBALL__
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
#else
      // Android and iOS use SDL main callbacks and use SDL_AppIterate
      break;
#endif
   }
#endif
}

void Player::GPUQueueStuffingGameLoop()
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

      UpdateGameLogic();

      PrepareFrame();

      UpdateGameLogic();

      SubmitFrame();

      UpdateGameLogic();

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

void Player::FramePacingGameLoop()
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
   // It also shows that since frames arrive late, they are pushed to the display out of sync. Whether they will wait for the next VBlank or 
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

      UpdateGameLogic();

      PLOGI_IF(debugLog) << "Frame Collect [Last frame length: " << ((double)m_logicProfiler.GetPrev(FrameProfiler::PROFILE_FRAME) / 1000.0) << "ms] at " << usec();
      PrepareFrame();

      UpdateGameLogic();

      PLOGI_IF(debugLog) << "Frame Submit at " << usec();
      SubmitFrame();
      m_renderProfiler->EnterProfileSection(FrameProfiler::PROFILE_RENDER_SLEEP);

      // Wait for at least one VBlank after last frame submission (adaptive sync)
      while (m_renderer->m_renderDevice->m_vsyncCount == 0)
      {
         m_curFrameSyncOnVBlank = true;
         YieldProcessor();
         UpdateGameLogic();
      }

      // If the user asked to sync on a lower frame rate than the refresh rate, then wait for it
      if (m_maxFramerate != m_playfieldWnd->GetRefreshRate())
      {
         const uint64_t now = usec();
         const unsigned int refreshLength = static_cast<unsigned int>(1000000. / (double)m_playfieldWnd->GetRefreshRate());
         const unsigned int minimumFrameLength = static_cast<unsigned int>(1000000. / (double)m_maxFramerate);
         const unsigned int maximumFrameLength = 5 * refreshLength;
         const unsigned int targetFrameLength = clamp(refreshLength - 2000, min(minimumFrameLength, maximumFrameLength), maximumFrameLength);
         while (now - m_renderer->m_renderDevice->m_lastPresentFrameTick < targetFrameLength)
         {
            m_curFrameSyncOnFPS = true;
            YieldProcessor();
            UpdateGameLogic();
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

void Player::PrepareFrame()
{
   // Rendering outputs to m_renderDevice->GetBackBufferTexture(). If MSAA is used, it is resolved as part of the rendering (i.e. this surface is NOT the MSAA render surface but its resolved copy)
   // Then it is tonemapped/bloom/dither/... to m_renderDevice->GetPostProcessRenderTarget1() if needed for postprocessing (sharpen, FXAA,...), or directly to the main output framebuffer otherwise
   // The optional postprocessing is done from m_renderDevice->GetPostProcessRenderTarget1() to the main output framebuffer
   #ifdef MSVC_CONCURRENCY_VIEWER
   span* tagSpan = new span(series, 1, _T("Prepare"));
   #endif

   m_logicProfiler.NewFrame(m_time_msec);
   m_logicProfiler.EnterProfileSection(FrameProfiler::PROFILE_PREPARE_FRAME);

   m_overall_frames++; // This causes the next VPinMAME <-> VPX sync to update light status which can be heavy since it needs to perform PWM integration of all lights
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
   UpdateGameLogic();
   #endif

   #ifdef MSVC_CONCURRENCY_VIEWER
   delete tagSpan;
   #endif

   #ifdef MSVC_CONCURRENCY_VIEWER
   tagSpan = new span(series, 1, _T("Build.RF"));
   #endif

   // Prepare main 3D scene frame, then apply screenspace transforms, including ancillary window rendering (MSAA, AO, AA, stereo, ball motion blur, tonemapping, dithering, bloom,...)
   m_renderer->RenderFrame();

   m_physics->ResetPerFrameStats();

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
   if (IsPlaying())
   {
      bool musicPlaying = m_audioPlayer->IsMusicPlaying();
      if (m_musicPlaying && !musicPlaying)
         m_ptable->FireVoidEvent(DISPID_GameEvents_MusicDone);
      m_musicPlaying = musicPlaying;
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
      if (m_ptable->m_liveBaseTable)
         m_ptable->m_liveBaseTable->m_pcv->m_scriptError = true;
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
      if (g_pvp->m_disable_pause_menu)
         m_closing = CS_STOP_PLAY;
      else {
         m_liveUI->OpenMainSplash();
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
      // Legacy hacky Win32 focus management: keep VPX focused & overlayed by the ancillary COM created window
      // This is very hacky and does not seem to always work (we are requesting focus but also ask these window to stay on top of us as an overlay)
      // This also means that user interaction with these ancillary windows is disabled during the first seconds after starting a table which is nothing but intuitive
      // Finally, this means that we are doing the z ordering of these windows in between themselves (for example B2S DMD vs Freezy's DMD) which is not clean too...
      // Window priority order is defined by the order of names in the overlaylist array below
      // The clean way of handling this is by using windows managed by VPX through plugins & ancillary window rendering support (overlays are rendered inside the containing window)
      if (m_time_msec < 3000)
      {
         static const std::array<string, 15> overlaylist {
            // Backglass overlays
            "B2S Backglass Server"s, "B2S Backglass"s, "B2S Background"s, "Form1"s, // B2S, including legacy 'B2S Background' and 'Form1' windows
            "PUPSCREEN2"s, "PUPSCREEN0"s, // PinUp Player, including topper (after backglass, if overlayed over backglass)
            "pygame"s, // PROC controller
            // DMD & alpha segment overlays
            "formDMD"s, // Not sure which one is this ?
            "B2S DMD"s, // B2S (usually 16:9 DMD, eventually overlayed by Freezy's or VPinMAME DMD)
            "PUPSCREEN5"s, "PUPSCREEN1"s, // PinUp Player DMD display (16:9 FullDMD then 4:1 standard DMD)
            "VPinMAME"s, // VPinMAME
            "Virtual Alphanumeric Display"s, "Virtual DMD"s, // Freezy's ExtDMD (DMD but also windows for alpha segment displays)
         };
         for (const string &windowtext : overlaylist)
         {
            HWND hVPMWnd = FindWindow(nullptr, windowtext.c_str());
            if (hVPMWnd == nullptr)
               hVPMWnd = FindWindow(windowtext.c_str(), nullptr);
            if (hVPMWnd != nullptr && IsWindowVisible(hVPMWnd))
            {
               // Make sure the window is always on top of us, but does not take focus away from us (no activation flag, and not using BringWindowToTop which request focus)
               SetWindowPos(hVPMWnd, HWND_TOPMOST, 0, 0, 0, 0, (SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW | SWP_NOACTIVATE));
               // Keep input focus on the VPX playfield window
               // TODO With the latest changes (early hiding of editor window, cleaner focus management), do we really need this anymore ? Not doing it would allow user interaction with ancillary windows
               if (GetForegroundWindow() == hVPMWnd)
               {
                  PLOGI << "Ancillary overlay window '" << windowtext << "' has taken input focus, keeping focus on playfield.";
                  m_playfieldWnd->RaiseAndFocus();
               }
            }
         }
      }
   #endif
}

void Player::OnAuxRendererChanged(const unsigned int msgId, void* userData, void* msgData)
{
   Player * const me = static_cast<Player *>(userData);
   const MsgPluginAPI *m_msgApi = &MsgPI::MsgPluginManager::GetInstance().GetMsgAPI();
   for (int i = 0; i <= VPXWindowId::VPXWINDOW_Topper; i++)
   {
      const VPXWindowId window = (VPXWindowId) i;
      const string section = window == VPXWindowId::VPXWINDOW_Backglass ? "Backglass"s
                           : window == VPXWindowId::VPXWINDOW_ScoreView ? "ScoreView"s
                                                                        : "Topper"s;
      GetAncillaryRendererMsg getAuxRendererMsg { window, 0, 0, nullptr };
      m_msgApi->BroadcastMsg(VPXPluginAPIImpl::GetInstance().GetVPXEndPointId(), me->m_getAuxRendererId, &getAuxRendererMsg);
      me->m_ancillaryWndRenderers[window].resize(getAuxRendererMsg.count);
      getAuxRendererMsg = { window, getAuxRendererMsg.count, 0, me->m_ancillaryWndRenderers[window].data() };
      m_msgApi->BroadcastMsg(VPXPluginAPIImpl::GetInstance().GetVPXEndPointId(), me->m_getAuxRendererId, &getAuxRendererMsg);
      for (const auto& renderer : me->m_ancillaryWndRenderers[window])
         Settings::GetRegistry().Register(std::make_unique<VPX::Properties::IntPropertyDef>(section, "Priority."s.append(renderer.id), ""s, ""s, false, 0, 1000, 0));
      std::ranges::sort(me->m_ancillaryWndRenderers[window],
         [&](const AncillaryRendererDef &a, const AncillaryRendererDef &b)
         {
            int pa = me->m_ptable->m_settings.GetInt(Settings::GetRegistry().GetPropertyId(section, "Priority."s.append(a.id)).value());
            int pb = me->m_ptable->m_settings.GetInt(Settings::GetRegistry().GetPropertyId(section, "Priority."s.append(b.id)).value());
            return pa > pb; // Sort in descending order (first is the most wanted)
         });
      std::erase_if(me->m_ancillaryWndRenderers[window],
         [section, me](const AncillaryRendererDef &a) { return me->m_ptable->m_settings.GetInt(Settings::GetRegistry().GetPropertyId(section, "Priority."s.append(a.id)).value()) < 0; });
   }
}

void Player::UpdateVolume()
{
   m_audioPlayer->SetMainVolume(m_PlayMusic ? dequantizeSignedPercent(m_MusicVolume) : 0.f, m_PlaySound ? dequantizeSignedPercent(m_SoundVolume) : 0.f);
}

void Player::OnAudioUpdated(const unsigned int msgId, void* userData, void* msgData)
{
   Player *me = static_cast<Player *>(userData);
   AudioUpdateMsg &msg = *static_cast<AudioUpdateMsg *>(msgData);
   const auto &entry = me->m_audioStreams.find(msg.id.id);
   if (entry != me->m_audioStreams.end() && me->m_audioPlayer->IsOpened(entry->second))
   {
      VPX::AudioPlayer::AudioStreamID const stream = entry->second;
      if (msg.buffer != nullptr && msg.bufferSize != 0)
      {
         me->m_audioPlayer->SetStreamVolume(stream, msg.volume);
         me->m_audioPlayer->EnqueueStream(stream, msg.buffer, msg.bufferSize);
      }
      else
      {
         me->m_audioPlayer->CloseAudioStream(stream, false);
         me->m_audioStreams.erase(entry);
      }
   }
   else if (msg.buffer != nullptr)
   {
      MsgEndpointInfo info;
      MsgPI::MsgPluginManager::GetInstance().GetMsgAPI().GetEndpointInfo(msg.id.endpointId, &info);
      const int nChannels = (msg.type == CTLPI_AUDIO_SRC_BACKGLASS_MONO) ? 1 : 2;
      VPX::AudioPlayer::AudioStreamID const stream = me->m_audioPlayer->OpenAudioStream("Plugin."s + info.name + '.' + std::to_string(msg.id.resId), static_cast<int>(msg.sampleRate), nChannels, msg.format == CTLPI_AUDIO_FORMAT_SAMPLE_FLOAT);
      if (stream)
      {
         me->m_audioStreams[msg.id.id] = stream;
         me->m_audioPlayer->SetStreamVolume(stream, msg.volume);
         me->m_audioPlayer->EnqueueStream(stream, msg.buffer, msg.bufferSize);
      }
   }
}

void Player::PauseMusic()
{
   if (m_pauseMusicRefCount == 0)
      m_audioPlayer->PauseMusic();
   m_pauseMusicRefCount++;
}

void Player::UnpauseMusic()
{
   m_pauseMusicRefCount--;
   if (m_pauseMusicRefCount == 0)
      m_audioPlayer->UnpauseMusic();
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

      if (szWord == "Key"s)
      {
         sscanf_s(szLine, "%s %s %d",szWord, (unsigned)_countof(szWord), szSubWord, (unsigned)_countof(szSubWord), &index);
         CComVariant rgvar[1] = { CComVariant(index) };
         DISPPARAMS dispparams = { rgvar, nullptr, 1, 0 };
         g_pplayer->m_ptable->FireDispID(szSubWord == "Down"s ? DISPID_GameEvents_KeyDown : DISPID_GameEvents_KeyUp, &dispparams);
      }
      else if (szWord == "Physics"s)
      {
         sscanf_s(szLine, "%s %s %f",szWord, (unsigned)_countof(szWord), szSubWord, (unsigned)_countof(szSubWord), &dtime);
      }
      else if (szWord == "Frame"s)
      {
         int a,b,c,d;
         sscanf_s(szLine, "%s %s %f %u %u %u %u",szWord, (unsigned)_countof(szWord), szSubWord, (unsigned)_countof(szSubWord), &dtime, &a, &b, &c, &d);
         pli1->HighPart = a;
         pli1->LowPart = b;
         pli2->HighPart = c;
         pli2->LowPart = d;
      }
      else if (szWord == "Step"s)
      {
         int a,b,c,d;
         sscanf_s(szLine, "%s %s %u %u %u %u",szWord, (unsigned)_countof(szWord), szSubWord, (unsigned)_countof(szSubWord), &a, &b, &c, &d);
         pli1->HighPart = a;
         pli1->LowPart = b;
         pli2->HighPart = c;
         pli2->LowPart = d;
      }
      else if (szWord == "End"s)
      {
         return dtime;
      }
   }
}

#endif
