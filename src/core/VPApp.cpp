// license:GPLv3+

// Implementation of WinMain (Windows with UI) or main (Standalone)

#include "core/stdafx.h"

#include "core/VPApp.h"

#include "vpversion.h"

#include "plugins/VPXPlugin.h"
#include "core/VPXPluginAPIImpl.h"

#ifdef CRASH_HANDLER
#include "utils/StackTrace.h"
#include "utils/CrashHandler.h"
#include "utils/BlackBox.h"
#endif

#include "ui/resource.h"
#include <initguid.h>

#define SET_CRT_DEBUG_FIELD(a) _CrtSetDbgFlag((a) | _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG))

#ifndef __STANDALONE__
#include "vpinball_i.c"
#endif

#include <locale>
#include <codecvt>

#ifdef __ANDROID__
#include <SDL3/SDL_main.h>
#endif

#ifdef __STANDALONE__
#include <SDL3_ttf/SDL_ttf.h>
#include <filesystem>
#endif

#ifndef OVERRIDE
#ifndef __STANDALONE__
   #define OVERRIDE override
#else
   #define OVERRIDE
#endif
#endif

#ifdef CRASH_HANDLER
extern "C" int __cdecl _purecall()
{
   ShowError("Pure Virtual Function Call");

   CONTEXT Context = {};
#ifdef _WIN64
   RtlCaptureContext(&Context);
#else
   Context.ContextFlags = CONTEXT_CONTROL;

   __asm
   {
   Label:
      mov[Context.Ebp], ebp;
      mov[Context.Esp], esp;
      mov eax, [Label];
      mov[Context.Eip], eax;
   }
#endif

   char callStack[2048] = {};
   rde::StackTrace::GetCallStack(&Context, true, callStack, sizeof(callStack) - 1);

   ShowError(callStack);

   return 0;
}
#endif

#if !defined(__STANDALONE__)

#if defined(ENABLE_DX9)
   #ifndef DISABLE_FORCE_NVIDIA_OPTIMUS
      extern "C" {
         __declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
      }
   #else
      extern "C" {
         __declspec(dllexport) DWORD NvOptimusEnablement = 0x00000000;
      }
   #endif
   #ifndef DISABLE_FORCE_AMD_HIGHPERF
      extern "C" { _declspec(dllexport) DWORD AmdPowerXpressRequestHighPerformance = 0x00000001; }
   #endif
#endif

#if (_WIN32_WINNT <= 0x0601 /* _WIN32_WINNT_WIN7 */ )
typedef enum ORIENTATION_PREFERENCE {
   ORIENTATION_PREFERENCE_NONE = 0x0,
   ORIENTATION_PREFERENCE_LANDSCAPE = 0x1,
   ORIENTATION_PREFERENCE_PORTRAIT = 0x2,
   ORIENTATION_PREFERENCE_LANDSCAPE_FLIPPED = 0x4,
   ORIENTATION_PREFERENCE_PORTRAIT_FLIPPED = 0x8
} ORIENTATION_PREFERENCE;
typedef BOOL(WINAPI *pSDARP)(ORIENTATION_PREFERENCE orientation);

static pSDARP SetDisplayAutoRotationPreferences = nullptr;
#endif

#if 0 //!defined(_CRTDBG_MAP_ALLOC) //&& (!defined(__STDCPP_DEFAULT_NEW_ALIGNMENT__) || (__STDCPP_DEFAULT_NEW_ALIGNMENT__ < 16))
// previous: somewhat custom new/delete was still needed, otherwise VPX crashed when exiting the player
//  was this due to old win32xx's whacky Shared_Ptr implementation?
void *operator new(const size_t size_req)
{
   void* const ptr = _aligned_malloc(size_req, 16);
   if (!ptr)
      throw std::bad_alloc{};
   return ptr;
}
void operator delete(void *address)
{
   _aligned_free(address);
}
/*void *operator new[](const size_t size_req)
{
   void* const ptr = _aligned_malloc(size_req, 16);
   if (!ptr)
      throw std::bad_alloc{};
   return ptr;
}
void operator delete[](void *address)
{
   _aligned_free(address);
}*/
#endif

CComModule VPApp::m_module;

BEGIN_OBJECT_MAP(ObjectMap)
END_OBJECT_MAP()

#endif


VPApp::VPApp()
#ifndef __STANDALONE__
   : m_msgLoop(std::make_unique<WinMsgLoop>())
#else
   : m_msgLoop(std::make_unique<StandaloneMsgLoop>())
#endif
{
   g_app = this;
   g_pvp = &m_vpxEditor;
   m_vpxEditor.AddRef();

   SetThreadName("Main"s);

   m_msgLoop->Initialize();

   #ifdef CRASH_HANDLER
      rde::CrashHandler::Init();
   #endif

   IsOnWine(); // init static variable in there

   #ifdef _MSC_VER
      // disable auto-rotate on tablets
      #if (_WIN32_WINNT <= 0x0601)
         SetDisplayAutoRotationPreferences = (pSDARP)GetProcAddress(GetModuleHandle(TEXT("user32.dll")), "SetDisplayAutoRotationPreferences");
         if (SetDisplayAutoRotationPreferences)
            SetDisplayAutoRotationPreferences(static_cast<ORIENTATION_PREFERENCE>(ORIENTATION_PREFERENCE_LANDSCAPE | ORIENTATION_PREFERENCE_LANDSCAPE_FLIPPED));
      #else
         SetDisplayAutoRotationPreferences(static_cast<ORIENTATION_PREFERENCE>(ORIENTATION_PREFERENCE_LANDSCAPE | ORIENTATION_PREFERENCE_LANDSCAPE_FLIPPED));
      #endif

      #ifdef _ATL_FREE_THREADED
         const HRESULT hRes = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
      #else
         const HRESULT hRes = CoInitialize(nullptr);
      #endif
      _ASSERTE(SUCCEEDED(hRes));
      m_module.Init(ObjectMap, m_vpxEditor.theInstance, &LIBID_VPinballLib);

      // load and register VP type library for COM integration
      {
         ITypeLib *ptl = nullptr;
         const wstring wFileName = GetModulePath<wstring>(m_vpxEditor.theInstance);
         if (SUCCEEDED(LoadTypeLib(wFileName.c_str(), &ptl)))
         {
            // first try to register system-wide (if running as admin)
            HRESULT hr = RegisterTypeLib(ptl, wFileName.c_str(), nullptr);
            if (!SUCCEEDED(hr))
            {
               // if failed, register only for current user
               hr = RegisterTypeLibForUser(ptl, (OLECHAR*)wFileName.c_str(), nullptr);
               if (!SUCCEEDED(hr))
                  m_vpxEditor.MessageBox("Could not register type library. Try running Visual Pinball as administrator.", "Error", MB_ICONERROR);
            }
            ptl->Release();
         }
         else
            m_vpxEditor.MessageBox("Could not load type library.", "Error", MB_ICONERROR);
      }

      #ifdef _ATL_FREE_THREADED
         const HRESULT hRes2 = m_module.RegisterClassObjects(CLSCTX_LOCAL_SERVER, REGCLS_MULTIPLEUSE | REGCLS_SUSPENDED);
         _ASSERTE(SUCCEEDED(hRes));
         hRes2 = CoResumeClassObjects();
      #else
         const HRESULT hRes2 = m_module.RegisterClassObjects(CLSCTX_LOCAL_SERVER, REGCLS_MULTIPLEUSE);
      #endif
      _ASSERTE(SUCCEEDED(hRes2));

      INITCOMMONCONTROLSEX iccex;
      iccex.dwSize = sizeof(INITCOMMONCONTROLSEX);
      iccex.dwICC = ICC_COOL_CLASSES;
      InitCommonControlsEx(&iccex);
   #endif
   
   m_logicalNumberOfProcessors = SDL_GetNumLogicalCPUCores();

   EditableRegistry::RegisterEditable<Ball>();
   EditableRegistry::RegisterEditable<Bumper>();
   EditableRegistry::RegisterEditable<Decal>();
   EditableRegistry::RegisterEditable<DispReel>();
   EditableRegistry::RegisterEditable<Flasher>();
   EditableRegistry::RegisterEditable<Flipper>();
   EditableRegistry::RegisterEditable<Gate>();
   EditableRegistry::RegisterEditable<Kicker>();
   EditableRegistry::RegisterEditable<Light>();
   EditableRegistry::RegisterEditable<LightSeq>();
   EditableRegistry::RegisterEditable<Plunger>();
   EditableRegistry::RegisterEditable<Primitive>();
   EditableRegistry::RegisterEditable<Ramp>();
   EditableRegistry::RegisterEditable<Rubber>();
   EditableRegistry::RegisterEditable<Spinner>();
   EditableRegistry::RegisterEditable<Surface>();
   EditableRegistry::RegisterEditable<Textbox>();
   EditableRegistry::RegisterEditable<Timer>();
   EditableRegistry::RegisterEditable<Trigger>();
   EditableRegistry::RegisterEditable<HitTarget>();
   EditableRegistry::RegisterEditable<PartGroup>();

   VPXPluginAPIImpl::GetInstance();
}

VPApp::~VPApp()
{
   m_msgLoop = nullptr;
   m_vpxEditor.Release();

   #ifndef __STANDALONE__
      m_module.RevokeClassObjects();
      m_module.Term();
      CoUninitialize();
   #endif
   g_pvp = nullptr;
   g_app = nullptr;

   #ifdef _CRTDBG_MAP_ALLOC
      _CrtDumpMemoryLeaks();
   #endif
}

void VPApp::LimitMultiThreading()
{
   const int procCount = SDL_GetNumLogicalCPUCores();
   m_logicalNumberOfProcessors = max(min(procCount, 2), procCount / 4); // only use 1/4th the threads, but at least 2 (if there are 2)
}

int VPApp::GetLogicalNumberOfProcessors() const
{
   if (m_logicalNumberOfProcessors < 1)
   {
      PLOGE << "Invalid number of processor " << m_logicalNumberOfProcessors << ". Fallback to single processor.";
      return 1;
   }

   return m_logicalNumberOfProcessors;
}

void VPApp::InitInstance()
{
   m_vpxEditor.Create(nullptr);

   // Define settings location and load them
   if (m_iniFileName.empty())
   {
      std::filesystem::path defaultPath = m_fileLocator.GetAppPath(FileLocator::AppSubFolder::Preferences) / "VPinballX.ini";
      std::filesystem::path appPath = m_fileLocator.GetAppPath(FileLocator::AppSubFolder::Root) / "VPinballX.ini";
      if (FileExists(defaultPath))
         m_iniFileName = defaultPath.string();
      else if (FileExists(appPath))
         m_iniFileName = appPath.string();
      else
         m_iniFileName = defaultPath.string();
   }
   m_settings.SetIniPath(m_iniFileName);
   m_settings.Load(true);

   // The file layout must be defined before loading the settings file, so we apply the following rules:
   // - if we have a settings location commandline override, we loads it and use the setting in it (to locate other files than the ini)
   // - if not but we have a settings file in the default preference location, we use it and update the setting accordingly ('Table' layout mode)
   // - if not but we have a settings file along the app executable, we use it and update the setting accordingly ('App' layout mode)
   // - if we don't have anything, then we use the default ('Table' layout mode)

   Logger::SetupLogger(m_settings.GetEditor_EnableLog());
   PLOGI << "Starting VPX - " << VP_VERSION_STRING_FULL_LITERAL;
   PLOGI << "Settings file was loaded from " << m_iniFileName;
   PLOGI << "Number of logical CPU cores: " << GetLogicalNumberOfProcessors();
   PLOGI << "Application path: " << m_fileLocator.GetAppPath(FileLocator::AppSubFolder::Root);
   PLOGI << "Preference path: " << m_fileLocator.GetAppPath(FileLocator::AppSubFolder::Preferences);
   
   Settings::SetRecentDir_ImportDir_Default(m_fileLocator.GetAppPath(FileLocator::AppSubFolder::Tables).string() + PATH_SEPARATOR_CHAR);
   Settings::SetRecentDir_LoadDir_Default(m_fileLocator.GetAppPath(FileLocator::AppSubFolder::Tables).string() + PATH_SEPARATOR_CHAR);
   Settings::SetRecentDir_FontDir_Default(m_fileLocator.GetAppPath(FileLocator::AppSubFolder::Tables).string() + PATH_SEPARATOR_CHAR);
   Settings::SetRecentDir_PhysicsDir_Default(m_fileLocator.GetAppPath(FileLocator::AppSubFolder::Tables).string() + PATH_SEPARATOR_CHAR);
   Settings::SetRecentDir_ImageDir_Default(m_fileLocator.GetAppPath(FileLocator::AppSubFolder::Tables).string() + PATH_SEPARATOR_CHAR);
   Settings::SetRecentDir_MaterialDir_Default(m_fileLocator.GetAppPath(FileLocator::AppSubFolder::Tables).string() + PATH_SEPARATOR_CHAR);
   Settings::SetRecentDir_SoundDir_Default(m_fileLocator.GetAppPath(FileLocator::AppSubFolder::Tables).string() + PATH_SEPARATOR_CHAR);
   Settings::SetRecentDir_POVDir_Default(m_fileLocator.GetAppPath(FileLocator::AppSubFolder::Tables).string() + PATH_SEPARATOR_CHAR);

   m_settings.SetVersion_VPinball(string(VP_VERSION_STRING_DIGITS), false);
   m_settings.Save();

   m_vpxEditor.LoadEditorSetupFromSettings();
}

int VPApp::Run()
{
   //SET_CRT_DEBUG_FIELD( _CRTDBG_LEAK_CHECK_DF );

   /* FIXME if (m_commandLineProcessor.m_captureAttract)
   {
      PLOGI << "Video capture mode requested for " << m_commandLineProcessor.m_captureAttract << " frames at " << m_commandLineProcessor.m_captureAttractFPS << "FPS from table '"
            << m_commandLineProcessor.m_tableFileName << "' " << (m_commandLineProcessor.m_captureAttractLoop ? "with " : "without ") << "loop truncation";
   }

   if (!m_commandLineProcessor.m_tableFileName.empty())
   {
      PLOGI << "Loading table from command line option: " << m_commandLineProcessor.m_tableFileName;
      m_vpxEditor.LoadFileName(m_commandLineProcessor.m_tableFileName, !m_commandLineProcessor.m_liveedit && !m_commandLineProcessor.m_play && m_commandLineProcessor.m_run);
      m_commandLineProcessor.m_table_played_via_command_line = m_commandLineProcessor.m_play;
      if (m_vpxEditor.m_ptableActive && !m_commandLineProcessor.m_tableIniFileName.empty())
         m_vpxEditor.m_ptableActive->SetSettingsFileName(m_commandLineProcessor.m_tableIniFileName);
      if (!m_vpxEditor.m_ptableActive && m_commandLineProcessor.m_open_minimized)
         m_vpxEditor.QuitPlayer(Player::CloseState::CS_CLOSE_APP);
   }
   #ifdef _DEBUG
   else if (!m_commandLineProcessor.m_liveedit && m_settings.GetEditor_SelectTableOnStart())
   #else
   else if (m_settings.GetEditor_SelectTableOnStart())
   #endif
   {
      m_commandLineProcessor.m_table_played_via_SelectTableOnStart = m_vpxEditor.LoadFile(false);
   }

   if (m_commandLineProcessor.m_extractScript && m_vpxEditor.m_ptableActive)
   {
      string scriptFilename = m_vpxEditor.m_ptableActive->m_filename;
      if(ReplaceExtensionFromFilename(scriptFilename, "vbs"s))
         m_vpxEditor.m_ptableActive->m_pcv->SaveToFile(scriptFilename);
   }

   if (m_commandLineProcessor.m_audit && m_vpxEditor.m_ptableActive)
   {
      m_vpxEditor.m_ptableActive->AuditTable(true);
   }

   if (m_commandLineProcessor.m_extractPov && m_vpxEditor.m_ptableActive)
   {
      for (int i = 0; i < 3; i++)
         m_vpxEditor.m_ptableActive->mViewSetups[i].SaveToTableOverrideSettings(m_vpxEditor.m_ptableActive->m_settings, (ViewSetupID) i);
      m_vpxEditor.m_ptableActive->m_settings.Save();
   }

   if (!m_commandLineProcessor.m_tournamentFileName.empty() && m_vpxEditor.m_ptableActive)
   {
      m_vpxEditor.GenerateImageFromTournamentFile(m_vpxEditor.m_ptableActive->m_filename, m_commandLineProcessor.m_tournamentFileName);
   }

   #ifdef _DEBUG
   if (m_commandLineProcessor.m_liveedit)
   {
      if (m_vpxEditor.m_ptableActive == nullptr)
         m_vpxEditor.ParseCommand(ID_NEW_BLANKTABLE, false);
      if (m_vpxEditor.m_ptableActive != nullptr)
         m_vpxEditor.DoPlay(3);
      else
         m_commandLineProcessor.m_run = false;
   }
   else
   #endif
      if ((m_commandLineProcessor.m_play || m_commandLineProcessor.m_table_played_via_SelectTableOnStart) && m_vpxEditor.m_ptableActive)
   {
      m_vpxEditor.DoPlay(m_commandLineProcessor.m_povEdit ? 1 : 0);
   }

   if (!m_commandLineProcessor.m_run)
   {
      m_vpxEditor.QuitPlayer(Player::CloseState::CS_CLOSE_APP);
   }*/

   const int retval = m_msgLoop->MainMsgLoop();

   m_settings.Save();

   return retval;
}


#ifndef __STANDALONE__

WinMsgLoop::WinMsgLoop()
{
}

void WinMsgLoop::Initialize()
{
   m_vpxEditor = g_pvp;
   m_vpxEditor->theInstance = GetInstanceHandle();
   SetResourceHandle(m_vpxEditor->theInstance);
}

bool WinMsgLoop::StepMsgLoop()
{
   if (MSG msg; PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
   {
      m_idleIndex = 0;
      if (msg.message == WM_QUIT)
         return true;
      if (!PreTranslateMessage(msg))
      {
         TranslateMessage(&msg);
         DispatchMessage(&msg);
      }
   }
   else if (OnIdle(m_idleIndex++) == FALSE)
   {
      WaitMessage();
   }
   return false;
}

int WinMsgLoop::MainMsgLoop()
{
   while (!StepMsgLoop())
   {
      // Nothing to do here, everything is handled in the message loop and idle processing
   }
   return 0;
}

BOOL WinMsgLoop::OnIdle(LONG count)
{
   MsgPI::MsgPluginManager::GetInstance().ProcessAsyncCallbacks();
   /* FIXME if (!g_pplayer && g_app->m_commandLineProcessor.m_table_played_via_SelectTableOnStart)
   {
      // If player has been closed in the meantime, check if we should display the file open dialog again to select/play the next table
      // first close the current table
      if (const auto pt = m_vpxEditor->GetActiveTableEditor(); pt)
         m_vpxEditor->CloseTable(pt);
      // then select the new one, and if one was selected, play it
      g_app->m_commandLineProcessor.m_table_played_via_SelectTableOnStart = m_vpxEditor->LoadFile(false);
      if (g_app->m_commandLineProcessor.m_table_played_via_SelectTableOnStart)
         m_vpxEditor->DoPlay(0);
      return TRUE;
   }
   else if (!g_pplayer && g_app->m_commandLineProcessor.m_open_minimized)
   {
      // If started to play and for whatever reason (end of play, frontend closing the player window, failed loading,...)
      // we do not have a player, just close back to system.
      m_vpxEditor->PostMessage(WM_CLOSE, 0, 0);
      return TRUE;
   }
   else
   {
      // Otherwise wait for next event
      return FALSE;
   } */
   return FALSE;
}

BOOL WinMsgLoop::PreTranslateMessage(MSG& msg)
{
   if ((msg.message >= WM_KEYFIRST && msg.message <= WM_KEYLAST) /* && (msg.wParam == VK_DELETE) */)
   {
      HWND hwndFocus = GetFocus();
      TCHAR className[256];
      if (hwndFocus && GetClassName(hwndFocus, className, 256))
      {
         // If it's an Edit control, skip accelerators
         if (_tcscmp(className, _T("Edit")) == 0)
            return FALSE;
      }
   }
   return __super::PreTranslateMessage(msg);
}

#else

void StandaloneMsgLoop::Initialize()
{
   m_vpxEditor = g_pvp;
}

bool StandaloneMsgLoop::StepMsgLoop()
{
   return false;
}

int StandaloneMsgLoop::MainMsgLoop()
{
   int retval = 0;
   if (auto pt = m_vpxEditor->GetActiveTableEditor(); pt)
   {
      if (pt->m_table->m_pcv->m_scriptError)
         retval = 1;
      m_vpxEditor->CloseTable(pt);
   }
   return retval;
}

#endif
