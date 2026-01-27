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

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
END_OBJECT_MAP()


static const char** CommandLineToArgvA(const char* const CmdLine, int* const _argc)
{
   const size_t len = strlen(CmdLine);
   size_t i = ((len + 2) / 2) * sizeof(void*) + sizeof(void*);

   const char** argv = (const char**)malloc(i + (len + 2) * sizeof(char));
   char* _argv = (char*)(((unsigned char*)argv) + i);

   int argc = 0;
   argv[argc] = _argv;
   bool in_QM = false;
   bool in_TEXT = false;
   bool in_SPACE = true;
   i = 0;
   size_t j = 0;

   char a;
   while ((a = CmdLine[i])) {
      if (in_QM) {
         if (a == '\"') {
            in_QM = false;
         }
         else {
            _argv[j] = a;
            j++;
         }
      }
      else {
         switch (a) {
         case '\"':
            in_QM = true;
            in_TEXT = true;
            if (in_SPACE) {
               argv[argc] = _argv + j;
               argc++;
            }
            in_SPACE = false;
            break;
         case ' ':
         case '\t':
         case '\n':
         case '\r':
            if (in_TEXT) {
               _argv[j] = '\0';
               j++;
            }
            in_TEXT = false;
            in_SPACE = true;
            break;
         default:
            in_TEXT = true;
            if (in_SPACE) {
               argv[argc] = _argv + j;
               argc++;
            }
            _argv[j] = a;
            j++;
            in_SPACE = false;
            break;
         }
      }
      i++;
   }
   _argv[j] = '\0';
   argv[argc] = nullptr;

   (*_argc) = argc;
   return argv;
}
#endif

#ifdef __STANDALONE__
int g_argc;
const char **g_argv;
#endif

static const string options[] = { // keep in sync with option_names & option_descs!
   "h"s,
   "Help"s,
   "?"s,
   "UnregServer"s,
   "RegServer"s,
   "DisableTrueFullscreen"s,
   "EnableTrueFullscreen"s,
   "Minimized"s,
   "ExtMinimized"s,
   "GLES"s,
   "LessCPUthreads"s,
   "Edit"s,
   #ifdef _DEBUG
   "LiveEdit"s,
   #endif
   "Play"s,
   "PovEdit"s,
   "Pov"s,
   "ExtractVBS"s,
   "Ini"s,
   "TableIni"s,
   "TournamentFile"s,
   "v"s,
   "exit"s, // (ab)used by frontend, not handled by us
   "Audit"s,
   "CaptureAttract"s,
   "c1"s,
   "c2"s,
   "c3"s,
   "c4"s,
   "c5"s,
   "c6"s,
   "c7"s,
   "c8"s,
   "c9"s,
   "PrefPath"s,
   ""s
};
static const string option_descs[] =
{
   string(),
   string(),
   string(),
   "Unregister VP functions"s,
   "Register VP functions"s,
   "Force-disable True Fullscreen setting"s,
   "Force-enable True Fullscreen setting"s,
   "Start VP in the 'invisible' minimized window mode"s,
   "Start VP in the 'invisible' minimized window mode, but with enabled Pause Menu"s,
   "[value]  Overrides the global emission scale (day/night setting, value range: 0.115..0.925)"s,
   "Limit the amount of parallel execution"s,
   "[filename]  Load file into VP"s,
   #ifdef _DEBUG
   "Start VP in live editor mode. if a filename is provided, loads it as the table to edit instead of the default table"s,
   #endif
   "[filename]  Load and play file"s,
   "[filename]  Load and run file in live editing mode, then export new pov on exit"s,
   "[filename]  Load, export pov and close"s,
   "[filename]  Load, export table script and close"s,
   "[filename]  Use a custom settings file instead of loading it from the default location"s,
   "[filename]  Use a custom table settings file. This option is only available in conjunction with a command which specifies a table filename like Play, Edit,..."s,
   "[table filename] [tournament filename]  Load a table and tournament file and convert to .png"s,
   "Displays the version"s,
   string(),
   "[table filename] Audit the table"s,
   "Capture an attract mode video"s,
   "Custom value 1"s,
   "Custom value 2"s,
   "Custom value 3"s,
   "Custom value 4"s,
   "Custom value 5"s,
   "Custom value 6"s,
   "Custom value 7"s,
   "Custom value 8"s,
   "Custom value 9"s,
   "[path]  Use a custom preferences path instead of default"s,
   ""s
};
enum option_names
{
   OPTION_H,
   OPTION_HELP,
   OPTION_QMARK,
   OPTION_UNREGSERVER,
   OPTION_REGSERVER,
   OPTION_DISABLETRUEFULLSCREEN,
   OPTION_ENABLETRUEFULLSCREEN,
   OPTION_MINIMIZED,
   OPTION_EXTMINIMIZED,
   OPTION_GLES,
   OPTION_LESSCPUTHREADS,
   OPTION_EDIT,
   #ifdef _DEBUG
   OPTION_LIVE_EDIT,
   #endif
   OPTION_PLAY,
   OPTION_POVEDIT,
   OPTION_POV,
   OPTION_EXTRACTVBS,
   OPTION_INI,
   OPTION_TABLE_INI,
   OPTION_TOURNAMENT,
   OPTION_VERSION,
   OPTION_FRONTEND_EXIT,
   OPTION_AUDIT,
   OPTION_CAPTURE_ATTRACT,
   OPTION_CUSTOM1,
   OPTION_CUSTOM2,
   OPTION_CUSTOM3,
   OPTION_CUSTOM4,
   OPTION_CUSTOM5,
   OPTION_CUSTOM6,
   OPTION_CUSTOM7,
   OPTION_CUSTOM8,
   OPTION_CUSTOM9,
   OPTION_PREFPATH,
   OPTION_INVALID,
};

static bool compare_option(const string& arg, const option_names option)
{
   return (StrCompareNoCase(arg, '-' + options[option]) ||
           StrCompareNoCase(arg, '/' + options[option]));
}

VPApp::VPApp(HINSTANCE hInstance)
{
   g_app = this;
   m_vpinball.AddRef();
   SetThreadName("Main"s);

   #ifndef __STANDALONE__
      m_vpinball.theInstance = GetInstanceHandle();
      SetResourceHandle(m_vpinball.theInstance);
   #endif
   g_pvp = &m_vpinball;

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

      //!! max(2u, std::thread::hardware_concurrency()) ??
      SYSTEM_INFO sysinfo;
      GetSystemInfo(&sysinfo);
      m_vpinball.SetLogicalNumberOfProcessors(sysinfo.dwNumberOfProcessors); //!! this ignores processor groups, so if at some point we need extreme multi threading, implement this in addition!

      #ifdef _ATL_FREE_THREADED
         const HRESULT hRes = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
      #else
         const HRESULT hRes = CoInitialize(nullptr);
      #endif
      _ASSERTE(SUCCEEDED(hRes));
      _Module.Init(ObjectMap, m_vpinball.theInstance, &LIBID_VPinballLib);

      // load and register VP type library for COM integration
      {
         ITypeLib *ptl = nullptr;
         const wstring wFileName = GetModulePath<wstring>(m_vpinball.theInstance);
         if (SUCCEEDED(LoadTypeLib(wFileName.c_str(), &ptl)))
         {
            // first try to register system-wide (if running as admin)
            HRESULT hr = RegisterTypeLib(ptl, wFileName.c_str(), nullptr);
            if (!SUCCEEDED(hr))
            {
               // if failed, register only for current user
               hr = RegisterTypeLibForUser(ptl, (OLECHAR*)wFileName.c_str(), nullptr);
               if (!SUCCEEDED(hr))
                  m_vpinball.MessageBox("Could not register type library. Try running Visual Pinball as administrator.", "Error", MB_ICONERROR);
            }
            ptl->Release();
         }
         else
            m_vpinball.MessageBox("Could not load type library.", "Error", MB_ICONERROR);
      }

      #ifdef _ATL_FREE_THREADED
         const HRESULT hRes2 = _Module.RegisterClassObjects(CLSCTX_LOCAL_SERVER, REGCLS_MULTIPLEUSE | REGCLS_SUSPENDED);
         _ASSERTE(SUCCEEDED(hRes));
         hRes2 = CoResumeClassObjects();
      #else
         const HRESULT hRes2 = _Module.RegisterClassObjects(CLSCTX_LOCAL_SERVER, REGCLS_MULTIPLEUSE);
      #endif
      _ASSERTE(SUCCEEDED(hRes2));

      INITCOMMONCONTROLSEX iccex;
      iccex.dwSize = sizeof(INITCOMMONCONTROLSEX);
      iccex.dwICC = ICC_COOL_CLASSES;
      InitCommonControlsEx(&iccex);
   #else
      m_vpinball.SetLogicalNumberOfProcessors(SDL_GetNumLogicalCPUCores());

   #endif

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
   m_vpinball.Release();

   #ifndef __STANDALONE__
      _Module.RevokeClassObjects();
      _Module.Term();
      CoUninitialize();
   #endif
   g_pvp = nullptr;
   g_app = nullptr;

   #ifdef _CRTDBG_MAP_ALLOC
      _CrtDumpMemoryLeaks();
   #endif
}

string VPApp::GetPathFromArg(const string& arg, bool setCurrentPath)
{
   #ifndef __STANDALONE__
      string path = arg;
      if ((arg[0] == '-') || (arg[0] == '/')) // Remove leading - or /
         path = arg.substr(1, arg.size() - 1);
   #else
      string path = trim_string(arg);
   #endif

   if (path[0] == '"') // Remove " "
      path = path.substr(1, path.size() - 1);

   #ifndef __STANDALONE__
      if (path[1] != ':') // Add current path
      {
         char szLoadDir[MAXSTRING];
         GetCurrentDirectory(MAXSTRING, szLoadDir);
         path = string(szLoadDir) + PATH_SEPARATOR_CHAR + path;
      }
   #else
      if (path[0] != '/') // Add current path
      {
         char szLoadDir[MAXSTRING];
         GetCurrentDirectory(MAXSTRING, szLoadDir);
         path = string(szLoadDir) + PATH_SEPARATOR_CHAR + path;
      }
   #endif
   else if (setCurrentPath) // Or set the current path from the arg
   {
      const string dir = PathFromFilename(path);
      SetCurrentDirectory(dir.c_str());
   }

   #ifdef __STANDALONE__
      path = std::filesystem::weakly_canonical(std::filesystem::path(path));
   #endif

   return path;
}

string VPApp::GetCommandLineHelp()
{
   return
   #ifndef __STANDALONE__
      "-"    +options[OPTION_UNREGSERVER]+          "  "+option_descs[OPTION_UNREGSERVER]+
      "\n-"  +options[OPTION_REGSERVER]+            "  "+option_descs[OPTION_REGSERVER]+
      "\n\n"+
   #endif
      "-"    +options[OPTION_DISABLETRUEFULLSCREEN]+"  "+option_descs[OPTION_DISABLETRUEFULLSCREEN]+
      "\n-"  +options[OPTION_ENABLETRUEFULLSCREEN]+ "  "+option_descs[OPTION_ENABLETRUEFULLSCREEN]+
      "\n-"  +options[OPTION_MINIMIZED]+            "  "+option_descs[OPTION_MINIMIZED]+
      "\n-"  +options[OPTION_EXTMINIMIZED]+         "  "+option_descs[OPTION_EXTMINIMIZED]+
      "\n\n-"+options[OPTION_GLES]+                 "  "+option_descs[OPTION_GLES]+
      "\n\n-"+options[OPTION_LESSCPUTHREADS]+       "  "+option_descs[OPTION_LESSCPUTHREADS]+
      "\n\n-"+options[OPTION_EDIT]+                 "  "+option_descs[OPTION_EDIT]+
      #ifdef _DEBUG
      "\n-"  +options[OPTION_LIVE_EDIT]+            "  "+option_descs[OPTION_LIVE_EDIT]+
      #endif
      "\n-"  +options[OPTION_PLAY]+                 "  "+option_descs[OPTION_PLAY]+
      "\n-"  +options[OPTION_POVEDIT]+              "  "+option_descs[OPTION_POVEDIT]+
      "\n-"  +options[OPTION_POV]+                  "  "+option_descs[OPTION_POV]+
      "\n-"  +options[OPTION_AUDIT]+                "  "+option_descs[OPTION_AUDIT]+
      "\n-"  +options[OPTION_EXTRACTVBS]+           "  "+option_descs[OPTION_EXTRACTVBS]+
      "\n-"  +options[OPTION_INI]+                  "  "+option_descs[OPTION_INI]+
      "\n-"  +options[OPTION_TABLE_INI]+            "  "+option_descs[OPTION_TABLE_INI]+
      "\n\n-"+options[OPTION_TOURNAMENT]+           "  "+option_descs[OPTION_TOURNAMENT]+
      "\n\n-"+options[OPTION_VERSION]+              "  "+option_descs[OPTION_VERSION]+
      "\n\n-"+options[OPTION_CAPTURE_ATTRACT]+      "  "+option_descs[OPTION_CAPTURE_ATTRACT]+
      "\n\n-"+options[OPTION_PREFPATH]+             "  "+option_descs[OPTION_PREFPATH]+
      "\n\n-c1 [customparam] .. -c9 [customparam]  Custom user parameters that can be accessed in the script via GetCustomParam(X)";
}

void VPApp::OnCommandLineError(const string& title, const string& message)
{
   #ifndef __STANDALONE__
      ::MessageBox(NULL, message.c_str(), title.c_str(), MB_ICONERROR);
   #else
      std::cout << title << "\n\n" << message << "\n\n";
   #endif
}

void VPApp::ProcessCommandLine()
{
#ifndef __STANDALONE__
   int nArgs;
   const char** szArglist = CommandLineToArgvA(GetCommandLine(), &nArgs);
   ProcessCommandLine(nArgs, szArglist);
   free(szArglist);

#else
   ProcessCommandLine(g_argc, g_argv);

#endif
}

void VPApp::ProcessCommandLine(int nArgs, const char* szArglist[])
{
   m_run = true;
   m_play = false;
   m_extractPov = false;
   m_extractScript = false;
   m_audit = false;
   m_tableFileName.clear();
   m_tableIniFileName.clear();
   m_iniFileName.clear();
   m_tournamentFileName.clear();

   for (int i = 1; i < nArgs; ++i) // skip szArglist[0], contains executable name
   {
      option_names opt = option_names::OPTION_INVALID;
      for (size_t i2 = 0; i2 < std::size(options); ++i2)
      {
         if (compare_option(szArglist[i], static_cast<option_names>(i2)))
         {
            opt = static_cast<option_names>(i2);
            break;
         }
      }
      switch (opt)
      {
      case OPTION_INVALID:
         // If the only parameter passed is a vpx table, consider it as /play command
         if ((nArgs == 2) && (i == 1))
         {
            string filename = GetPathFromArg(szArglist[i], false);
            if (extension_from_path(filename) == "vpx" && FileExists(filename))
            {
               m_play = true;
               m_vpinball.m_open_minimized = true;
               m_tableFileName = filename;
               i++;
            }
            break;
         }
         // ignore stuff (directories) that is passed in via frontends (not considered as invalid)
         if ((szArglist[i][0] != '-') && (szArglist[i][0] != '/'))
            break;
         // Really an invalid parameter, show help and exit
         OnCommandLineError("Visual Pinball Usage"s, "Invalid Parameter "s + szArglist[i] + "\n\nValid Parameters are:\n\n" + GetCommandLineHelp());
         exit(1);
         break;

      case OPTION_H:
      case OPTION_HELP:
      case OPTION_QMARK:
         #ifndef __STANDALONE__
            ::MessageBox(NULL, GetCommandLineHelp().c_str(), "Visual Pinball Usage", MB_OK);
         #else
            std::cout << "Visual Pinball Usage" << "\n\n" << GetCommandLineHelp() << "\n\n";
         #endif
         exit(0);
         break;

      case OPTION_VERSION:
      {
         static const string ver = "Visual Pinball "s + VP_VERSION_STRING_FULL_LITERAL;
         #ifndef __STANDALONE__
            ::MessageBox(NULL, ver.c_str(), "Visual Pinball", MB_OK);
         #else
            std::cout << ver << "\n\n";
         #endif
         exit(0);
         break;
      }

      case OPTION_LESSCPUTHREADS:
      {
         int procCount = m_vpinball.GetLogicalNumberOfProcessors();
         m_vpinball.SetLogicalNumberOfProcessors(max(min(procCount, 2), procCount/4)); // only use 1/4th the threads, but at least 2 (if there are 2)
         break;
      }

      // FIXME remove as this is now handled by the ini system
      case OPTION_DISABLETRUEFULLSCREEN:
         m_vpinball.m_disEnableTrueFullscreen = 0;
         break;

      // FIXME remove as this is now handled by the ini system
      case OPTION_ENABLETRUEFULLSCREEN:
         m_vpinball.m_disEnableTrueFullscreen = 1;
         break;

      // FIXME remove as this is now handled by the ini system
      case OPTION_GLES: // global emission scale parameter handling
         if (i + 1 < nArgs)
         {
            const char* lpszStr;
            if ((szArglist[i + 1][0] == '-') || (szArglist[i + 1][0] == '/'))
               lpszStr = szArglist[i + 1] + 1;
            else
               lpszStr = szArglist[i + 1];
            m_vpinball.m_fgles = clamp(sz2f(lpszStr), 0.115f, 0.925f);
            m_vpinball.m_bgles = true;
         }
         else
         {
            OnCommandLineError("Command Line Error"s, "Missing global emission scale adjustment"s);
         }
         break;

      case OPTION_CUSTOM1:
      case OPTION_CUSTOM2:
      case OPTION_CUSTOM3:
      case OPTION_CUSTOM4:
      case OPTION_CUSTOM5:
      case OPTION_CUSTOM6:
      case OPTION_CUSTOM7:
      case OPTION_CUSTOM8:
      case OPTION_CUSTOM9:
      {
         if ((i + 1 < nArgs) && (opt - OPTION_CUSTOM1) <= 9)
         {
            m_vpinball.m_customParameters[opt - OPTION_CUSTOM1] = MakeWString(szArglist[i + 1]);
            ++i; // two params processed
         }
         else
         {
            OnCommandLineError("Command Line Error"s, "Missing custom option value after /c..."s);
         }
         break;
      }

      case OPTION_MINIMIZED:
         m_vpinball.m_open_minimized = true;
         m_vpinball.m_disable_pause_menu = true;
         break;

      case OPTION_EXTMINIMIZED:
         m_vpinball.m_open_minimized = true;
         break;

      #ifdef _DEBUG
      case OPTION_LIVE_EDIT:
         m_liveedit = opt == OPTION_LIVE_EDIT;
         if (i + 1 < nArgs)
         {
            m_tableFileName = GetPathFromArg(szArglist[i + 1], false);
            if (FileExists(m_tableFileName))
               i++;
            else
               m_tableFileName.clear();
         }
         break;
      #endif

      case OPTION_POVEDIT:
      case OPTION_PLAY:
      case OPTION_EDIT:
      case OPTION_AUDIT:
      case OPTION_POV:
      case OPTION_EXTRACTVBS:
         if (i + 1 >= nArgs)
         {
            OnCommandLineError("Command Line Error"s, "Option '"s + szArglist[i] + "' must be followed by a valid table file path");
            exit(1);
         }
         m_vpinball.m_povEdit = opt == OPTION_POVEDIT;
         m_play = (opt == OPTION_PLAY) || (opt == OPTION_POVEDIT);
         m_audit = opt == OPTION_AUDIT;
         m_extractPov = opt == OPTION_POV;
         m_extractScript = opt == OPTION_EXTRACTVBS;
         m_vpinball.m_open_minimized = opt != OPTION_EDIT;
         m_run = !(opt == OPTION_AUDIT || opt == OPTION_POV || opt == OPTION_EXTRACTVBS); // Don't run the UI
         m_tableFileName = GetPathFromArg(szArglist[i + 1], false);
         i++;
         break;

      case OPTION_CAPTURE_ATTRACT:
         if (i + 3 >= nArgs)
         {
            OnCommandLineError("Command Line Error"s, "Option '"s + szArglist[i] + "' must be followed by the number of frames to capture, the framerate and a valid table file path");
            exit(1);
         }
         m_play = true;
         m_vpinball.m_open_minimized = true;
         m_run = true;
         if (!try_parse_int(szArglist[i + 1], m_vpinball.m_captureAttract) || m_vpinball.m_captureAttract <= 0)
         {
            OnCommandLineError("Command Line Error"s, "Invalid number of frames"s);
            exit(1);
         }
         if (!try_parse_int(szArglist[i + 2], m_vpinball.m_captureAttractFPS) || m_vpinball.m_captureAttractFPS <= 0)
         {
            OnCommandLineError("Command Line Error"s, "Invalid framerate"s);
            exit(1);
         }
         m_tableFileName = GetPathFromArg(szArglist[i + 3], false);
         if (i + 4 < nArgs && StrCompareNoCase("noloop"s, szArglist[i + 4]))
         {
            m_vpinball.m_captureAttractLoop = false;
            i += 4;
         }
         else
         {
            m_vpinball.m_captureAttractLoop = true;
            i += 3;
         }
         break;

      case OPTION_INI:
         if (i + 1 >= nArgs)
         {
            OnCommandLineError("Command Line Error"s, "Option '"s + szArglist[i] + "' must be followed by a valid setting file path");
            exit(1);
         }
         m_iniFileName = GetPathFromArg(szArglist[i + 1], false);
         i++;
         break;

      case OPTION_TABLE_INI:
         if (i + 1 >= nArgs)
         {
            OnCommandLineError("Command Line Error"s, "Option '"s + szArglist[i] + "' must be followed by a valid setting file path");
            exit(1);
         }
         m_tableIniFileName = GetPathFromArg(szArglist[i + 1], false);
         i++;
         break;

      case OPTION_TOURNAMENT:
         if (i + 2 >= nArgs)
         {
            OnCommandLineError("Command Line Error"s, "Option '"s + szArglist[i] + "' must be followed by a valid table file path and a valid tournament file path");
            exit(1);
         }
         m_run = false;
         m_vpinball.m_open_minimized = true;
         m_tableIniFileName = GetPathFromArg(szArglist[i + 1], false);
         i++;
         m_tournamentFileName = GetPathFromArg(szArglist[i + 2], false);
         i++;
         if (!FileExists(m_tournamentFileName))
         {
            OnCommandLineError("Command Line Error"s, "Tournament file '" + m_tournamentFileName + "' was not found");
            exit(1);
         }
         break;

      #ifndef __STANDALONE__
      case OPTION_UNREGSERVER:
      {
         _Module.UpdateRegistryFromResource(IDR_VPINBALL, FALSE);
         const HRESULT ret = _Module.UnregisterServer(TRUE);
         if (ret != S_OK)
            ShowError("Unregister VP functions failed");
         m_run = false;
         break;
      }

      case OPTION_REGSERVER:
      {
         _Module.UpdateRegistryFromResource(IDR_VPINBALL, TRUE);
         const HRESULT ret = _Module.RegisterServer(TRUE);
         if (ret != S_OK)
            ShowError("Register VP functions failed");
         m_run = false;
         break;
      }
      #endif

      case OPTION_PREFPATH:
         if (i + 1 >= nArgs)
         {
            OnCommandLineError("Command Line Error"s, "Option '"s + szArglist[i] + "' must be followed by a valid folder path");
            exit(1);
         }
         {
            string path = GetPathFromArg(szArglist[i + 1], false);
            if (!DirExists(path))
            {
               std::error_code ec;
               if (std::filesystem::create_directories(path, ec))
               {
                  PLOGI << "Pref path created: " << path;
               }
               else
               {
                  PLOGE << "Unable to create pref path: " << path;
               }
            }
            m_vpinball.SetPrefPath(path);
         }
         i++;
         break;

      default:
         assert(false);
         break;
      }

      if (!m_tableFileName.empty() && !FileExists(m_tableFileName))
      {
         OnCommandLineError("Command Line Error"s, "Table file '" + m_tableFileName + "' was not found");
         exit(1);
      }
   }
}

BOOL VPApp::InitInstance()
{
   m_vpinball.Create(nullptr);

   // Define settings location and load them
   if (m_iniFileName.empty())
   {
      std::filesystem::path defaultPath = m_vpinball.GetAppPath(VPinball::AppSubFolder::Preferences) / "VPinballX.ini";
      std::filesystem::path appPath = m_vpinball.GetAppPath(VPinball::AppSubFolder::Root) / "VPinballX.ini";
      if (FileExists(defaultPath))
         m_iniFileName = defaultPath.string();
      else if (FileExists(appPath))
         m_iniFileName = appPath.string();
      else
         m_iniFileName = defaultPath.string();
   }
   m_vpinball.m_settings.SetIniPath(m_iniFileName);
   m_vpinball.m_settings.Load(true);

   // The file layout must be defined before loading the settings file, so we apply the following rules:
   // - if we have a settings location commandline override, we loads it and use the setting in it (to locate other files than the ini)
   // - if not but we have a settings file in the default preference location, we use it and update the setting accordingly ('Table' layout mode)
   // - if not but we have a settings file along the app executable, we use it and update the setting accordingly ('App' layout mode)
   // - if we don't have anything, then we use the default ('Table' layout mode)

   Logger::SetupLogger(m_vpinball.m_settings.GetEditor_EnableLog());
   PLOGI << "Starting VPX - " << VP_VERSION_STRING_FULL_LITERAL;
   PLOGI << "Settings file was loaded from " << m_iniFileName;
   PLOGI << "Number of logical CPU cores: " << m_vpinball.GetLogicalNumberOfProcessors();
   PLOGI << "Application path: " << m_vpinball.GetAppPath(VPinball::AppSubFolder::Root);
   PLOGI << "Preference path: " << m_vpinball.GetAppPath(VPinball::AppSubFolder::Preferences);
   
   Settings::SetRecentDir_ImportDir_Default(g_pvp->GetAppPath(VPinball::AppSubFolder::Tables).string() + PATH_SEPARATOR_CHAR);
   Settings::SetRecentDir_LoadDir_Default(g_pvp->GetAppPath(VPinball::AppSubFolder::Tables).string() + PATH_SEPARATOR_CHAR);
   Settings::SetRecentDir_FontDir_Default(g_pvp->GetAppPath(VPinball::AppSubFolder::Tables).string() + PATH_SEPARATOR_CHAR);
   Settings::SetRecentDir_PhysicsDir_Default(g_pvp->GetAppPath(VPinball::AppSubFolder::Tables).string() + PATH_SEPARATOR_CHAR);
   Settings::SetRecentDir_ImageDir_Default(g_pvp->GetAppPath(VPinball::AppSubFolder::Tables).string() + PATH_SEPARATOR_CHAR);
   Settings::SetRecentDir_MaterialDir_Default(g_pvp->GetAppPath(VPinball::AppSubFolder::Tables).string() + PATH_SEPARATOR_CHAR);
   Settings::SetRecentDir_SoundDir_Default(g_pvp->GetAppPath(VPinball::AppSubFolder::Tables).string() + PATH_SEPARATOR_CHAR);
   Settings::SetRecentDir_POVDir_Default(g_pvp->GetAppPath(VPinball::AppSubFolder::Tables).string() + PATH_SEPARATOR_CHAR);

   m_vpinball.m_settings.SetVersion_VPinball(string(VP_VERSION_STRING_DIGITS), false);

   m_vpinball.LoadEditorSetupFromSettings();

   SDL_SetHint(SDL_HINT_WINDOW_ALLOW_TOPMOST, "0");
   if (!SDL_InitSubSystem(SDL_INIT_VIDEO))
   {
      PLOGE << "SDL_InitSubSystem(SDL_INIT_VIDEO) failed: " << SDL_GetError();
      // FIXME this is not correct as we may be running something else than the player (extract vbs, ...)
      exit(1);
   }
   
   return TRUE;
}

int VPApp::Run()
{
   //SET_CRT_DEBUG_FIELD( _CRTDBG_LEAK_CHECK_DF );

   if (m_vpinball.m_captureAttract)
   {
      PLOGI << "Video capture mode requested for " << m_vpinball.m_captureAttract << " frames at " << m_vpinball.m_captureAttractFPS << "FPS from table '" << m_tableFileName << "' "
            << (m_vpinball.m_captureAttractLoop ? "with " : "without ") << "loop truncation";
   }

   if (!m_tableFileName.empty())
   {
      PLOGI << "Loading table from command line option: " << m_tableFileName;
      m_vpinball.LoadFileName(m_tableFileName, !m_liveedit && !m_play && m_run);
      m_vpinball.m_table_played_via_command_line = m_play;
      if (m_vpinball.m_ptableActive && !m_tableIniFileName.empty())
         m_vpinball.m_ptableActive->SetSettingsFileName(m_tableIniFileName);
      if (!m_vpinball.m_ptableActive && m_vpinball.m_open_minimized)
         m_vpinball.QuitPlayer(Player::CloseState::CS_CLOSE_APP);
   }
   #ifdef _DEBUG
   else if (!m_liveedit && m_vpinball.m_settings.GetEditor_SelectTableOnStart())
   #else
   else if (m_vpinball.m_settings.GetEditor_SelectTableOnStart())
   #endif
   {
      m_vpinball.m_table_played_via_SelectTableOnStart = m_vpinball.LoadFile(false);
   }

   if (m_extractScript && m_vpinball.m_ptableActive)
   {
      string scriptFilename = m_vpinball.m_ptableActive->m_filename;
      if(ReplaceExtensionFromFilename(scriptFilename, "vbs"s))
         m_vpinball.m_ptableActive->m_pcv->SaveToFile(scriptFilename);
   }

   if (m_audit && m_vpinball.m_ptableActive)
   {
      m_vpinball.m_ptableActive->AuditTable(true);
   }

   if (m_extractPov && m_vpinball.m_ptableActive)
   {
      for (int i = 0; i < 3; i++)
         m_vpinball.m_ptableActive->mViewSetups[i].SaveToTableOverrideSettings(m_vpinball.m_ptableActive->m_settings, (ViewSetupID) i);
      m_vpinball.m_ptableActive->m_settings.Save();
   }

   if (!m_tournamentFileName.empty() && m_vpinball.m_ptableActive)
   {
      m_vpinball.GenerateImageFromTournamentFile(m_vpinball.m_ptableActive->m_filename, m_tournamentFileName);
   }

   #ifdef _DEBUG
   if (m_liveedit)
   {
      if (m_vpinball.m_ptableActive == nullptr)
         m_vpinball.ParseCommand(ID_NEW_BLANKTABLE, false);
      if (m_vpinball.m_ptableActive != nullptr)
         m_vpinball.DoPlay(3);
      else
         m_run = false;
   }
   else
   #endif
   if ((m_play || m_vpinball.m_table_played_via_SelectTableOnStart) && m_vpinball.m_ptableActive)
   {
      m_vpinball.DoPlay(m_vpinball.m_povEdit ? 1 : 0);
   }

   if (!m_run)
   {
      m_vpinball.QuitPlayer(Player::CloseState::CS_CLOSE_APP);
   }

   const int retval = MainMsgLoop();

   m_vpinball.m_settings.Save();

   return retval;
}

BOOL VPApp::OnIdle(LONG count)
{
#ifndef __STANDALONE__
   MsgPI::MsgPluginManager::GetInstance().ProcessAsyncCallbacks();
   if (!g_pplayer && m_vpinball.m_table_played_via_SelectTableOnStart)
   {
      // If player has been closed in the meantime, check if we should display the file open dialog again to select/play the next table
      // first close the current table
      CComObject<PinTable>* const pt = m_vpinball.GetActiveTable();
      if (pt)
         m_vpinball.CloseTable(pt);
      // then select the new one, and if one was selected, play it
      m_vpinball.m_table_played_via_SelectTableOnStart = m_vpinball.LoadFile(false);
      if (m_vpinball.m_table_played_via_SelectTableOnStart)
         m_vpinball.DoPlay(0);
   }
   else if (!g_pplayer && m_vpinball.m_open_minimized)
   {
      // If started to play and for whatever reason (end of play, frontend closing the player window, failed loading,...)
      // we do not have a player, just close back to system.
      m_vpinball.PostMessage(WM_CLOSE, 0, 0);
   }
   else
   {
      // Otherwise wait for next event
      return FALSE;
   }
#endif
   return TRUE;
}

bool VPApp::StepMsgLoop()
{
#ifndef __STANDALONE__
   MSG msg;
   if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
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
#endif
   return false;
}

int VPApp::MainMsgLoop()
{
   int retval = 0;
#ifndef __STANDALONE__
   while (!StepMsgLoop())
   {
   }
#else
   CComObject<PinTable>* const pt = m_vpinball.GetActiveTable();
   if (pt)
   {
      if (pt->m_pcv->m_scriptError)
         retval = 1;
      m_vpinball.CloseTable(pt);
   }
#endif
   return retval;
}
