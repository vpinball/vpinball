// license:GPLv3+

// Implementation of (Win)Main

#include "core/stdafx.h"

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

#define  SET_CRT_DEBUG_FIELD(a)   _CrtSetDbgFlag((a) | _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG))

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

#if !defined(ENABLE_BGFX)
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

#if 0 //!defined(DEBUG_XXX) && !defined(_CRTDBG_MAP_ALLOC) //&& (!defined(__STDCPP_DEFAULT_NEW_ALIGNMENT__) || (__STDCPP_DEFAULT_NEW_ALIGNMENT__ < 16))
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


PCHAR* CommandLineToArgvA(PCHAR CmdLine, int* _argc)
{
   PCHAR*  argv;
   PCHAR   _argv;
   int  len;
   ULONG   argc;
   CHAR    a;
   size_t  i, j;

   BOOLEAN  in_QM;
   BOOLEAN  in_TEXT;
   BOOLEAN  in_SPACE;

   len = lstrlen(CmdLine);
   i = ((len + 2) / 2) * sizeof(PVOID) + sizeof(PVOID);

   argv = (PCHAR*)malloc(i + (len + 2) * sizeof(CHAR));
   _argv = (PCHAR)(((PUCHAR)argv) + i);

   argc = 0;
   argv[argc] = _argv;
   in_QM = FALSE;
   in_TEXT = FALSE;
   in_SPACE = TRUE;
   i = 0;
   j = 0;

   while ((a = CmdLine[i])) {
      if (in_QM) {
         if (a == '\"') {
            in_QM = FALSE;
         }
         else {
            _argv[j] = a;
            j++;
         }
      }
      else {
         switch (a) {
         case '\"':
            in_QM = TRUE;
            in_TEXT = TRUE;
            if (in_SPACE) {
               argv[argc] = _argv + j;
               argc++;
            }
            in_SPACE = FALSE;
            break;
         case ' ':
         case '\t':
         case '\n':
         case '\r':
            if (in_TEXT) {
               _argv[j] = '\0';
               j++;
            }
            in_TEXT = FALSE;
            in_SPACE = TRUE;
            break;
         default:
            in_TEXT = TRUE;
            if (in_SPACE) {
               argv[argc] = _argv + j;
               argc++;
            }
            _argv[j] = a;
            j++;
            in_SPACE = FALSE;
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
char **g_argv;
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
   "Primary"s,
   "GLES"s,
   "LessCPUthreads"s,
   "Edit"s,
   "Play"s,
   "PovEdit"s,
   "Pov"s,
   "ExtractVBS"s,
   "Ini"s,
   "TableIni"s,
   "TournamentFile"s,
   "v"s,
   "exit"s // (ab)used by frontend, not handled by us
#ifdef __STANDALONE__
   ,
   "PrefPath"s,
   "listres"s,
   "listsnd"s
#endif
}; // + c1..c9
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
   "Force VP to render on the Primary/Pixel(0,0) Monitor"s,
   "[value]  Overrides the global emission scale (day/night setting, value range: 0.115..0.925)"s,
   "Limit the amount of parallel execution"s,
   "[filename]  Load file into VP"s,
   "[filename]  Load and play file"s,
   "[filename]  Load and run file in live editing mode, then export new pov on exit"s,
   "[filename]  Load, export pov and close"s,
   "[filename]  Load, export table script and close"s,
   "[filename]  Use a custom settings file instead of loading it from the default location"s,
   "[filename]  Use a custom table settings file. This option is only available in conjunction with a command which specifies a table filename like Play, Edit,..."s,
   "[table filename] [tournament filename]  Load a table and tournament file and convert to .png"s,
   "Displays the version"s,
   string()
#ifdef __STANDALONE__
   ,
   "[path]  Use a custom preferences path instead of $HOME/.vpinball"s,
   "List available fullscreen resolutions"s,
   "List available sound devices"s
#endif   
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
   OPTION_PRIMARY,
   OPTION_GLES,
   OPTION_LESSCPUTHREADS,
   OPTION_EDIT,
   OPTION_PLAY,
   OPTION_POVEDIT,
   OPTION_POV,
   OPTION_EXTRACTVBS,
   OPTION_INI,
   OPTION_TABLE_INI,
   OPTION_TOURNAMENT,
   OPTION_VERSION,
   OPTION_FRONTEND_EXIT
#ifdef __STANDALONE__
   ,
   OPTION_PREFPATH,
   OPTION_LISTRES,
   OPTION_LISTSND
#endif
};

static bool compare_option(const char *const arg, const option_names option)
{
   return ((lstrcmpi(arg, ('-'+options[option]).c_str()) == 0) ||
           (lstrcmpi(arg, ('/'+options[option]).c_str()) == 0));
}

class VPApp final : public CWinApp
{
private:
   bool m_run;
   bool m_play;
   bool m_extractPov;
   bool m_file;
   bool m_loadFileResult;
   bool m_extractScript;
   bool m_tournament;
   bool m_bgles;
   float m_fgles;
#ifdef __STANDALONE__
   string m_szPrefPath;
   bool m_listRes;
   bool m_listSnd;
#endif
   string m_szTableFileName;
   string m_szTableIniFileName;
   string m_szIniFileName;
   string m_szTournamentName;
   VPinball m_vpinball;

public:
   VPApp(HINSTANCE hInstance)
   {
#ifndef __STANDALONE__
       m_vpinball.theInstance = GetInstanceHandle();
       SetResourceHandle(m_vpinball.theInstance);
#endif
       g_pvp = &m_vpinball;
   }

   virtual ~VPApp() 
   {
#ifndef __STANDALONE__
      _Module.Term();
      CoUninitialize();
#endif
      g_pvp = nullptr;

#ifdef _CRTDBG_MAP_ALLOC
#ifdef DEBUG_XXX  //disable this in perference to DevPartner
      _CrtSetDumpClient(MemLeakAlert);
#endif
      _CrtDumpMemoryLeaks();
#endif
   }

   string GetPathFromArg(const string& arg, bool setCurrentPath)
   {
#ifndef __STANDALONE__
      string path = arg;
#else
      string path = trim_string(arg);
#endif

#ifndef __STANDALONE__
      if ((arg[0] == '-') || (arg[0] == '/')) // Remove leading - or /
         path = path.substr(1, path.size() - 1);
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

   BOOL InitInstance() override
   {
      g_pvp = &m_vpinball;

#ifdef CRASH_HANDLER
      rde::CrashHandler::Init();
#endif

#ifdef _MSC_VER
      // disable auto-rotate on tablets
#if (_WIN32_WINNT <= 0x0601)
      SetDisplayAutoRotationPreferences = (pSDARP)GetProcAddress(GetModuleHandle(TEXT("user32.dll")),
         "SetDisplayAutoRotationPreferences");
      if (SetDisplayAutoRotationPreferences)
         SetDisplayAutoRotationPreferences(ORIENTATION_PREFERENCE_LANDSCAPE);
#else
      SetDisplayAutoRotationPreferences(ORIENTATION_PREFERENCE_LANDSCAPE);
#endif

      //!! max(2u, std::thread::hardware_concurrency()) ??
      SYSTEM_INFO sysinfo;
      GetSystemInfo(&sysinfo);
      m_vpinball.SetLogicalNumberOfProcessors(sysinfo.dwNumberOfProcessors); //!! this ignores processor groups, so if at some point we need extreme multi threading, implement this in addition!
#else
      m_vpinball.SetLogicalNumberOfProcessors(SDL_GetNumLogicalCPUCores());
#endif

      IsOnWine(); // init static variable in there

#ifdef _MSC_VER
#if _WIN32_WINNT >= 0x0400 & defined(_ATL_FREE_THREADED)
      const HRESULT hRes = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
#else
      const HRESULT hRes = CoInitialize(nullptr);
#endif
      _ASSERTE(SUCCEEDED(hRes));
      _Module.Init(ObjectMap, m_vpinball.theInstance, &LIBID_VPinballLib);
#endif

      m_file = false;
      m_play = false;
      bool allowLoadOnStart = true;
      m_extractPov = false;
      m_run = true;
      m_loadFileResult = true;
      m_extractScript = false;
      m_tournament = false;
      m_fgles = 0.f;
      m_bgles = false;
#ifdef __STANDALONE__
      m_listRes = false;
      m_listSnd = false;
      m_szPrefPath.clear();
#endif

      m_szTableFileName.clear();

#ifndef __STANDALONE__
      int nArgs;
      LPSTR *szArglist = CommandLineToArgvA(GetCommandLine(), &nArgs);
#else
      int nArgs = g_argc;
      char**  szArglist = g_argv;
#endif

      for (int i = 1; i < nArgs; ++i) // skip szArglist[0], contains executable name
      {
         bool valid_param = false;
         if ((szArglist[i][0] != '-') && (szArglist[i][0] != '/')) // ignore stuff (directories) that is passed in via frontends
            valid_param = true;
         if (!valid_param) for (size_t i2 = 0; i2 < std::size(options); ++i2)
         {
            if (compare_option(szArglist[i], (option_names)i2))
            {
               valid_param = true;
               break;
            }
         }
         if (!valid_param) for (char t = '1'; t <= '9'; ++t)
         {
            if ((lstrcmpi(szArglist[i], ("-c"s+t).c_str()) == 0) || (lstrcmpi(szArglist[i], ("/c"s+t).c_str()) == 0))
            {
               valid_param = true;
               break;
            }
         }

         //
#ifdef __STANDALONE__
         // If the only parameter passed is a vpx table we play it automatically.
         const bool launchfile = (!valid_param) && (nArgs == 2) && (i==1) && strstr(szArglist[i], ".vpx") == (&szArglist[i][strlen(szArglist[i]) - 4]);
         if(launchfile)
         {
            valid_param = true;
            // Backtrack so the rest of the loop sees the table path as the next argument
            i = 0;
         }
#endif
         if (!valid_param
            || compare_option(szArglist[i], OPTION_H)
            || compare_option(szArglist[i], OPTION_HELP)
            || compare_option(szArglist[i], OPTION_QMARK))
         {
            string output = 
#ifndef __STANDALONE__
                            "-"    +options[OPTION_UNREGSERVER]+          "  "+option_descs[OPTION_UNREGSERVER]+
                            "\n-"  +options[OPTION_REGSERVER]+            "  "+option_descs[OPTION_REGSERVER]+
                            "\n\n"+
#endif
                            "-"+options[OPTION_DISABLETRUEFULLSCREEN]+    "  "+option_descs[OPTION_DISABLETRUEFULLSCREEN]+
                            "\n-"  +options[OPTION_ENABLETRUEFULLSCREEN]+ "  "+option_descs[OPTION_ENABLETRUEFULLSCREEN]+
                            "\n-"  +options[OPTION_MINIMIZED]+            "  "+option_descs[OPTION_MINIMIZED]+
                            "\n-"  +options[OPTION_EXTMINIMIZED]+         "  "+option_descs[OPTION_EXTMINIMIZED]+
                            "\n-"  +options[OPTION_PRIMARY]+              "  "+option_descs[OPTION_PRIMARY]+
                            "\n\n-"+options[OPTION_GLES]+                 "  "+option_descs[OPTION_GLES]+
                            "\n\n-"+options[OPTION_LESSCPUTHREADS]+       "  "+option_descs[OPTION_LESSCPUTHREADS]+
                            "\n\n-"+options[OPTION_EDIT]+                 "  "+option_descs[OPTION_EDIT]+
                            "\n-"  +options[OPTION_PLAY]+                 "  "+option_descs[OPTION_PLAY]+
                            "\n-"  +options[OPTION_POVEDIT]+              "  "+option_descs[OPTION_POVEDIT]+
                            "\n-"  +options[OPTION_POV]+                  "  "+option_descs[OPTION_POV]+
                            "\n-"  +options[OPTION_EXTRACTVBS]+           "  "+option_descs[OPTION_EXTRACTVBS]+
                            "\n-"  +options[OPTION_INI]+                  "  "+option_descs[OPTION_INI]+
                            "\n-"  +options[OPTION_TABLE_INI]+            "  "+option_descs[OPTION_TABLE_INI]+
                            "\n\n-"+options[OPTION_TOURNAMENT]+           "  "+option_descs[OPTION_TOURNAMENT]+
                            "\n\n-"+options[OPTION_VERSION]+              "  "+option_descs[OPTION_VERSION]+
#ifdef __STANDALONE__
                            "\n\n-"+options[OPTION_PREFPATH]+             "  "+option_descs[OPTION_PREFPATH]+
                            "\n-"  +options[OPTION_LISTRES]+              "  "+option_descs[OPTION_LISTRES]+
                            "\n-"  +options[OPTION_LISTSND]+              "  "+option_descs[OPTION_LISTSND]+
#endif       
                            "\n\n-c1 [customparam] .. -c9 [customparam]  Custom user parameters that can be accessed in the script via GetCustomParam(X)";
            if (!valid_param)
                output = "Invalid Parameter "s + szArglist[i] + "\n\nValid Parameters are:\n\n" + output;
#ifndef __STANDALONE__
            ::MessageBox(NULL, output.c_str(), "Visual Pinball Usage", valid_param ? MB_OK : MB_ICONERROR);
#else
            std::cout
                << "Visual Pinball Usage"
                << "\n\n"
                << output
                << "\n\n";
#endif

            exit(valid_param ? 0 : 1);
         }

         //

         if (compare_option(szArglist[i], OPTION_VERSION))
         {
            const string ver = "Visual Pinball "s + VP_VERSION_STRING_FULL_LITERAL;
#ifndef __STANDALONE__
            ::MessageBox(NULL, ver.c_str(), "Visual Pinball", MB_OK);
#else
            std::cout << ver.c_str() << "\n\n";
#endif
            exit(0);
         }

         //

         if (compare_option(szArglist[i], OPTION_LESSCPUTHREADS))
         {
            int procCount = m_vpinball.GetLogicalNumberOfProcessors();
            m_vpinball.SetLogicalNumberOfProcessors(max(min(procCount, 2), procCount/4)); // only use 1/4th the threads, but at least 2 (if there are 2)
         }

         //

#ifndef __STANDALONE__
         if (compare_option(szArglist[i], OPTION_UNREGSERVER))
         {
            _Module.UpdateRegistryFromResource(IDR_VPINBALL, FALSE);
            const HRESULT ret = _Module.UnregisterServer(TRUE);
            if (ret != S_OK)
                ShowError("Unregister VP functions failed");
            m_run = false;
            break;
         }
         if (compare_option(szArglist[i], OPTION_REGSERVER))
         {
            _Module.UpdateRegistryFromResource(IDR_VPINBALL, TRUE);
            const HRESULT ret = _Module.RegisterServer(TRUE);
            if (ret != S_OK)
                ShowError("Register VP functions failed");
            m_run = false;
            break;
         }
#endif

         //

         if (compare_option(szArglist[i], OPTION_DISABLETRUEFULLSCREEN))
         {
             m_vpinball.m_disEnableTrueFullscreen = 0;
             continue;
         }
         if (compare_option(szArglist[i], OPTION_ENABLETRUEFULLSCREEN))
         {
             m_vpinball.m_disEnableTrueFullscreen = 1;
             continue;
         }

         //

         bool useCustomParams = false;
         int customIdx = 1;
         for (char t = '1'; t <= '9'; ++t)
         {
             if (lstrcmpi(szArglist[i], ("-c"s+t).c_str()) == 0 || lstrcmpi(szArglist[i], ("/c"s+t).c_str()) == 0)
             {
                 useCustomParams = true;
                 break;
             }
             customIdx++;
         }

         if (useCustomParams && (i+1<nArgs))
         {
             const size_t len = strlen(szArglist[i + 1]);
             m_vpinball.m_customParameters[customIdx - 1] = new WCHAR[len + 1];

             MultiByteToWideCharNull(CP_ACP, 0, szArglist[i + 1], -1, m_vpinball.m_customParameters[customIdx - 1], (int)len + 1);

             ++i; // two params processed

             continue;
         }

         //

         const bool minimized = compare_option(szArglist[i], OPTION_MINIMIZED);
         if (minimized)
         {
             m_vpinball.m_open_minimized = true;
             m_vpinball.m_disable_pause_menu = true;
         }

         const bool ext_minimized = compare_option(szArglist[i], OPTION_EXTMINIMIZED);
         if (ext_minimized)
             m_vpinball.m_open_minimized = true;

         const bool gles = compare_option(szArglist[i], OPTION_GLES);

         const bool primaryDisplay = compare_option(szArglist[i], OPTION_PRIMARY);
         if (primaryDisplay)
             m_vpinball.m_primaryDisplay = true;

         // global emission scale parameter handling
         if (gles && (i + 1 < nArgs))
         {
            char *lpszStr;
            if ((szArglist[i + 1][0] == '-') || (szArglist[i + 1][0] == '/'))
               lpszStr = szArglist[i + 1] + 1;
            else
               lpszStr = szArglist[i + 1];
            m_fgles = clamp((float)atof(lpszStr), 0.115f, 0.925f);
            m_bgles = true;
         }

         const bool editfile = compare_option(szArglist[i], OPTION_EDIT);
         const bool playfile = compare_option(szArglist[i], OPTION_PLAY);
         const bool povEdit = compare_option(szArglist[i], OPTION_POVEDIT);
         const bool extractpov = compare_option(szArglist[i], OPTION_POV);
         const bool extractscript = compare_option(szArglist[i], OPTION_EXTRACTVBS);
#ifdef __STANDALONE__
         const bool prefPath = compare_option(szArglist[i], OPTION_PREFPATH);
         const bool listRes = compare_option(szArglist[i], OPTION_LISTRES);
         const bool listSnd = compare_option(szArglist[i], OPTION_LISTSND);
#endif
         const bool ini = compare_option(szArglist[i], OPTION_INI);
         const bool tableIni = compare_option(szArglist[i], OPTION_TABLE_INI);
         const bool tournament = compare_option(szArglist[i], OPTION_TOURNAMENT);

         if (/*playfile ||*/ extractpov || extractscript || tournament)
            m_vpinball.m_open_minimized = true;

#ifndef __STANDALONE__
         if (ini || tableIni || editfile || playfile || povEdit || extractpov || extractscript || tournament)
#else
         if (prefPath || ini || tableIni || editfile || playfile || launchfile || povEdit || extractpov || extractscript || tournament)
#endif
         {
            if (i + 1 >= nArgs)
            {
#ifndef __STANDALONE__
               ::MessageBox(NULL, ("Option '"s + szArglist[i] + "' must be followed by a valid file path"s).c_str(), "Command Line Error", MB_ICONERROR);
#else
               std::cout
                  << "Command Line Error"
                  << "\n\n"
                  << "Option '" << szArglist[i] << "' must be followed by a valid file path"
                  << "\n\n";
#endif
               exit(1);
            }
            const string path = GetPathFromArg(szArglist[i + 1], false);
#ifndef __STANDALONE__
            if (!FileExists(path) && !ini && !tableIni)
#else
            if (!FileExists(path) && !prefPath && !ini && !tableIni)
#endif
            {
#ifndef __STANDALONE__
               ::MessageBox(NULL, ("File '" + path + "' was not found").c_str(), "Command Line Error", MB_ICONERROR);
#else
               std::cout
                  << "Command Line Error"
                  << "\n\n"
                  << "File '" << path << "' was not found"
                  << "\n\n";
#endif
               exit(1);
            }

            if (tournament && (i + 2 >= nArgs))
            {
               ::MessageBox(NULL, ("Option '"s + szArglist[i] + "' must be followed by two valid file paths").c_str(), "Command Line Error", MB_ICONERROR);
               exit(1);
            }
            if (tournament)
            {
               m_szTournamentName = GetPathFromArg(szArglist[i + 2], false);
               i++; // three params processed
               if (!FileExists(m_szTournamentName))
               {
                  ::MessageBox(NULL, ("File '" + m_szTournamentName + "' was not found").c_str(), "Command Line Error", MB_ICONERROR);
                  exit(1);
               }
            }
            i++; // two params processed

#ifdef __STANDALONE__
            if (prefPath)
               m_szPrefPath = path;
            else
#endif
            if (ini)
               m_szIniFileName = path;
            else if (tableIni)
               m_szTableIniFileName = path;
            else // editfile || playfile || povEdit || extractpov || extractscript || tournament
            {
               allowLoadOnStart = false; // Don't face the user with a load dialog since the file is provided on the command line
               m_file = true;
               if (m_play || m_extractPov || m_extractScript || m_vpinball.m_povEdit || m_tournament)
               {
#ifndef __STANDALONE__
                  ::MessageBox(NULL, ("Only one of " + options[OPTION_EDIT] + ", " + options[OPTION_PLAY] + ", " + options[OPTION_POVEDIT] + ", " + options[OPTION_POV] + ", " + options[OPTION_EXTRACTVBS] + ", " + options[OPTION_TOURNAMENT] + " can be used.").c_str(),
                     "Command Line Error", MB_ICONERROR);
#else
                  std::cout
                     << "Command Line Error"
                     << "\n\n"
                     << "Only one of " 
                     << options[OPTION_EDIT] << ", "
                     << options[OPTION_PLAY] << ", "
                     << options[OPTION_POVEDIT] << ", "
                     << options[OPTION_POV] << ", "
                     << options[OPTION_EXTRACTVBS] << ", "
                     << options[OPTION_TOURNAMENT] << " can be used."
                     << "\n\n";
#endif
                  exit(1);
               }
               m_play = playfile || povEdit;
#ifdef __STANDALONE__
               m_play = m_play || launchfile; 
#endif
               m_extractPov = extractpov;
               m_extractScript = extractscript;
               m_vpinball.m_povEdit = povEdit;
               m_tournament = tournament;
               m_szTableFileName = path;
            }
         }

#ifdef __STANDALONE__
         if (listRes)
            m_listRes = true;

         if (listSnd)
            m_listSnd = true;
#endif
      }

#ifndef __STANDALONE__
      free(szArglist);
#endif

#ifdef __STANDALONE__
   if (!m_szPrefPath.empty()) {
      if (!m_szPrefPath.ends_with(PATH_SEPARATOR_CHAR))
         m_szPrefPath += PATH_SEPARATOR_CHAR;

      if (!DirExists(m_szPrefPath)) {
         try {
            std::filesystem::create_directory(m_szPrefPath);
         }
         catch(...) {
            std::cout
                << "Visual Pinball Error"
                << "\n\n"
                << "Could not create preferences path: " << m_szPrefPath
                << "\n\n";
            exit(1);
         }
      }
      m_vpinball.m_szMyPrefPath = m_szPrefPath;
   }
#endif

   defaultFileNameSearch[4] = PATH_USER;
   defaultFileNameSearch[5] = PATH_SCRIPTS;
   defaultFileNameSearch[6] = PATH_TABLES;

#ifdef __STANDALONE__
#if (defined(__APPLE__) && ((defined(TARGET_OS_IOS) && TARGET_OS_IOS) || (defined(TARGET_OS_TV) && TARGET_OS_TV)))
      copy_folder("assets", m_vpinball.m_szMyPrefPath);
#endif
#endif

      // Default ini path (can be overriden from command line via m_szIniFileName)
      if (m_szIniFileName.empty())
      {
         FILE *f;
         // first check if there is a .ini next to the .exe, otherwise use the default location
         if ((fopen_s(&f, (m_vpinball.m_szMyPath + "VPinballX.ini").c_str(), "r") == 0) && f)
         {
            m_vpinball.m_szMyPrefPath = m_vpinball.m_szMyPath;
            fclose(f);
         }
         m_szIniFileName = m_vpinball.m_szMyPrefPath + "VPinballX.ini";
      }

      m_vpinball.m_settings.LoadFromFile(m_szIniFileName, true);
      m_vpinball.m_settings.SaveValue(Settings::Version, "VPinball"s, VP_VERSION_STRING_DIGITS);

#ifndef __STANDALONE__
      Logger::GetInstance()->SetupLogger(m_vpinball.m_settings.LoadValueWithDefault(Settings::Editor, "EnableLog"s, false));
#else
      Logger::GetInstance()->SetupLogger(m_vpinball.m_settings.LoadValueWithDefault(Settings::Editor, "EnableLog"s, true));
#endif

      PLOGI << "Starting VPX - " << VP_VERSION_STRING_FULL_LITERAL;
      PLOGI << "Setting file is: " << m_szIniFileName;

#ifdef __STANDALONE__
      PLOGI << "Settings file was loaded from " << m_szIniFileName;
      PLOGI << "m_logicalNumberOfProcessors=" << m_vpinball.GetLogicalNumberOfProcessors();
      PLOGI << "m_szMyPath=" << m_vpinball.m_szMyPath;
      PLOGI << "m_szMyPrefPath=" << m_vpinball.m_szMyPrefPath;

      if (!DirExists(PATH_USER))
         std::filesystem::create_directory(PATH_USER);

      if (m_listRes) {
         PLOGI << "Available fullscreen resolutions:";
         vector<VPX::Window::DisplayConfig> displays;
         VPX::Window::GetDisplays(displays);
         for (int display = 0; display < displays.size(); display++) {
            vector<VPX::Window::VideoMode> allVideoModes;
            VPX::Window::GetDisplayModes(display, allVideoModes);
            for (size_t i = 0; i < allVideoModes.size(); ++i) {
               VPX::Window::VideoMode mode = allVideoModes.at(i);
               PLOGI << "display " << displays.at(display).adapter << ": " << mode.width << 'x' << mode.height
                     << " (depth=" << mode.depth << ", refreshRate=" << mode.refreshrate << ')';
            }
         }

         PLOGI << "Available window fullscreen desktop resolutions:";
         for (int display = 0; display < displays.size(); display++) {
            const SDL_DisplayMode* displayMode = SDL_GetDesktopDisplayMode(displays.at(display).adapter);
            PLOGI << "display " << displays.at(display).adapter << ": " << displayMode->w << 'x' << displayMode->h
                  << " (refreshRate=" << displayMode->refresh_rate << ')';
         }

         PLOGI << "Available external window renderers:";
         int numRenderers = SDL_GetNumRenderDrivers();
         for (int renderer = 0; renderer < numRenderers; renderer++)
            PLOGI << "Renderer " << renderer << ": " <<  SDL_GetRenderDriver(renderer);
      }

      if (m_listSnd) {
         PLOGI << "Available sound devices:";
         vector<AudioDevice> allAudioDevices;
         EnumerateAudioDevices(allAudioDevices);
         for (size_t i = 0; i < allAudioDevices.size(); ++i) {
            AudioDevice audioDevice = allAudioDevices.at(i);
            PLOGI << "id " << audioDevice.id << ": name=" << audioDevice.name << ", enabled=" << audioDevice.enabled;
         }
      }

      if (m_listRes || m_listSnd)
         exit(0);

#if (defined(__APPLE__) && ((defined(TARGET_OS_IOS) && TARGET_OS_IOS) || (defined(TARGET_OS_TV) && TARGET_OS_TV))) || defined(__ANDROID__)
      const string launchTable = g_pvp->m_settings.LoadValueWithDefault(Settings::Standalone, "LaunchTable"s, "assets/exampleTable.vpx"s);
      m_szTableFileName = m_vpinball.m_szMyPrefPath + launchTable;
      m_file = true;
      m_play = true;
#endif

#endif

      // Start VP with file dialog open and then also playing that one?
      const bool stos = allowLoadOnStart && m_vpinball.m_settings.LoadValueWithDefault(Settings::Editor, "SelectTableOnStart"s, true);
      if (stos)
      {
         m_file = true;
         m_play = true;
      }

#ifndef __STANDALONE__
      // load and register VP type library for COM integration
      char szFileName[MAXSTRING];
      if (GetModuleFileName(m_vpinball.theInstance, szFileName, MAXSTRING))
      {
         ITypeLib *ptl = nullptr;
         MAKE_WIDEPTR_FROMANSI(wszFileName, szFileName);
         if (SUCCEEDED(LoadTypeLib(wszFileName, &ptl)))
         {
            // first try to register system-wide (if running as admin)
            HRESULT hr = RegisterTypeLib(ptl, wszFileName, nullptr);
            if (!SUCCEEDED(hr))
            {
               // if failed, register only for current user
               hr = RegisterTypeLibForUser(ptl, wszFileName, nullptr);
               if (!SUCCEEDED(hr))
                  m_vpinball.MessageBox("Could not register type library. Try running Visual Pinball as administrator.", "Error", MB_ICONERROR);
            }
            ptl->Release();
         }
         else
            m_vpinball.MessageBox("Could not load type library.", "Error", MB_ICONERROR);
      }
#endif

#ifndef __STANDALONE__
#if _WIN32_WINNT >= 0x0400 & defined(_ATL_FREE_THREADED)
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

      m_vpinball.AddRef();
      m_vpinball.Create(nullptr);
      m_vpinball.m_bgles = m_bgles;
      m_vpinball.m_fgles = m_fgles;

#ifndef __STANDALONE__
      g_haccel = LoadAccelerators(m_vpinball.theInstance, MAKEINTRESOURCE(IDR_VPACCEL));
#endif

      if (m_file)
      {
         if (!m_szTableFileName.empty())
         {
            PLOGI << "Loading table from command line option: " << m_szTableFileName;
            m_vpinball.LoadFileName(m_szTableFileName, !m_play);
            m_vpinball.m_table_played_via_command_line = m_play;
            m_loadFileResult = m_vpinball.m_ptableActive != nullptr;
            if (m_vpinball.m_ptableActive && !m_szTableIniFileName.empty())
               m_vpinball.m_ptableActive->SetSettingsFileName(m_szTableIniFileName);
            if (!m_loadFileResult && m_vpinball.m_open_minimized)
               m_vpinball.QuitPlayer(Player::CloseState::CS_CLOSE_APP);
         }
         else
         {
            m_loadFileResult = m_vpinball.LoadFile(!m_play);
            m_vpinball.m_table_played_via_SelectTableOnStart = m_vpinball.m_settings.LoadValueWithDefault(Settings::Editor, "SelectTableOnPlayerClose"s, true) ? m_loadFileResult : false;
         }

         if (m_extractScript && m_loadFileResult)
         {
            string szScriptFilename = m_szTableFileName;
            if(ReplaceExtensionFromFilename(szScriptFilename, "vbs"s))
               m_vpinball.m_ptableActive->m_pcv->SaveToFile(szScriptFilename);
            m_vpinball.QuitPlayer(Player::CloseState::CS_CLOSE_APP);
         }
         if (m_extractPov && m_loadFileResult)
         {
            for (int i = 0; i < 3; i++)
               m_vpinball.m_ptableActive->mViewSetups[i].SaveToTableOverrideSettings(m_vpinball.m_ptableActive->m_settings, (ViewSetupID) i);
            m_vpinball.m_ptableActive->m_settings.Save();
            m_vpinball.QuitPlayer(Player::CloseState::CS_CLOSE_APP);
         }
         if (m_tournament && m_loadFileResult)
         {
            m_vpinball.GenerateImageFromTournamentFile(m_szTableFileName, m_szTournamentName);
            m_vpinball.QuitPlayer(Player::CloseState::CS_CLOSE_APP);
         }
      }

      //SET_CRT_DEBUG_FIELD( _CRTDBG_LEAK_CHECK_DF );
      return TRUE;
   }

   int Run() override
   {
      int retval = 0;
      if (m_run)
      {
         if (m_play && m_loadFileResult)
            m_vpinball.DoPlay(m_vpinball.m_povEdit);

         // VBA APC handles message loop (bastards)
         retval = m_vpinball.MainMsgLoop();

         m_vpinball.m_settings.Save();

         m_vpinball.Release();

#ifndef __STANDALONE__
         DestroyAcceleratorTable(g_haccel);
         _Module.RevokeClassObjects();
#endif
      }
      return retval;
   }
};

#if defined(ENABLE_OPENGL) && !defined(__STANDALONE__)
// The OpenGL implementation will fail on NVIDIA drivers when Threaded Optimization is enabled so we disable it for this app
// Note: There are quite a lot of applications doing this, but I think this may hide an incorrect OpenGL call somewhere
// that the threaded optimization of NVIDIA drivers ends up to crash. This would be good to find the root cause, if any.

#include "nvapi/nvapi.h"
#include "nvapi/NvApiDriverSettings.h"
#pragma warning(push)
#pragma warning(disable : 4838)
#include "nvapi/NvApiDriverSettings.c"
#pragma warning(pop)

enum NvThreadOptimization
{
   NV_THREAD_OPTIMIZATION_AUTO = 0,
   NV_THREAD_OPTIMIZATION_ENABLE = 1,
   NV_THREAD_OPTIMIZATION_DISABLE = 2,
   NV_THREAD_OPTIMIZATION_NO_SUPPORT = 3
};

static bool NvAPI_OK_Verify(NvAPI_Status status)
{
   if (status == NVAPI_OK)
      return true;

   NvAPI_ShortString szDesc = { 0 };
   NvAPI_GetErrorMessage(status, szDesc);

   PLOGI << "NVAPI error: " << szDesc;

   return false;
}

static NvThreadOptimization GetNVIDIAThreadOptimization()
{
   NvThreadOptimization threadOptimization = NV_THREAD_OPTIMIZATION_NO_SUPPORT;

   NvAPI_Status status;
   status = NvAPI_Initialize();
   if (!NvAPI_OK_Verify(status))
      return threadOptimization;

   NvDRSSessionHandle hSession;
   status = NvAPI_DRS_CreateSession(&hSession);
   if (!NvAPI_OK_Verify(status))
      return threadOptimization;

   status = NvAPI_DRS_LoadSettings(hSession);
   if (!NvAPI_OK_Verify(status))
   {
      NvAPI_DRS_DestroySession(hSession);
      return threadOptimization;
   }

   NvDRSProfileHandle hProfile;
   status = NvAPI_DRS_GetBaseProfile(hSession, &hProfile);
   if (!NvAPI_OK_Verify(status))
   {
      NvAPI_DRS_DestroySession(hSession);
      return threadOptimization;
   }

   NVDRS_SETTING originalSetting = {};
   originalSetting.version = NVDRS_SETTING_VER;
   status = NvAPI_DRS_GetSetting(hSession, hProfile, OGL_THREAD_CONTROL_ID, &originalSetting);
   if (NvAPI_OK_Verify(status))
   {
      threadOptimization = (NvThreadOptimization)originalSetting.u32CurrentValue;
   }

   NvAPI_DRS_DestroySession(hSession);

   return threadOptimization;
}

static void SetNVIDIAThreadOptimization(NvThreadOptimization threadedOptimization)
{
   if (threadedOptimization == NV_THREAD_OPTIMIZATION_NO_SUPPORT)
      return;

   NvAPI_Status status;
   status = NvAPI_Initialize();
   if (!NvAPI_OK_Verify(status))
      return;

   NvDRSSessionHandle hSession;
   status = NvAPI_DRS_CreateSession(&hSession);
   if (!NvAPI_OK_Verify(status))
      return;

   status = NvAPI_DRS_LoadSettings(hSession);
   if (!NvAPI_OK_Verify(status))
   {
      NvAPI_DRS_DestroySession(hSession);
      return;
   }

   NvDRSProfileHandle hProfile;
   status = NvAPI_DRS_GetBaseProfile(hSession, &hProfile);
   if (!NvAPI_OK_Verify(status))
   {
      NvAPI_DRS_DestroySession(hSession);
      return;
   }

   NVDRS_SETTING setting = {};
   setting.version = NVDRS_SETTING_VER;
   setting.settingId = OGL_THREAD_CONTROL_ID;
   setting.settingType = NVDRS_DWORD_TYPE;
   setting.u32CurrentValue = (EValues_OGL_THREAD_CONTROL)threadedOptimization;

   status = NvAPI_DRS_SetSetting(hSession, hProfile, &setting);
   if (!NvAPI_OK_Verify(status))
   {
      NvAPI_DRS_DestroySession(hSession);
      return;
   }

   status = NvAPI_DRS_SaveSettings(hSession);
   NvAPI_OK_Verify(status);

   NvAPI_DRS_DestroySession(hSession);
}
#endif

extern "C" int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR /*lpCmdLine*/, int /*nShowCmd*/)
{
   #if defined(ENABLE_OPENGL) && !defined(__STANDALONE__)
   static NvThreadOptimization s_OriginalNVidiaThreadOptimization = NV_THREAD_OPTIMIZATION_NO_SUPPORT;
   #endif

   Logger::GetInstance()->Init();

   int retval;
   try
   {
      #if defined(ENABLE_OPENGL) && !defined(__STANDALONE__)
      s_OriginalNVidiaThreadOptimization = GetNVIDIAThreadOptimization();
      if (s_OriginalNVidiaThreadOptimization != NV_THREAD_OPTIMIZATION_NO_SUPPORT && s_OriginalNVidiaThreadOptimization != NV_THREAD_OPTIMIZATION_DISABLE)
      {
         PLOGI << "Disabling NVIDIA Threaded Optimization";
         SetNVIDIAThreadOptimization(NV_THREAD_OPTIMIZATION_DISABLE);
      }
      #endif

      #ifdef ENABLE_SDL_VIDEO
         SDL_SetHint(SDL_HINT_WINDOW_ALLOW_TOPMOST, "0");
         if (!SDL_InitSubSystem(SDL_INIT_VIDEO))
         {
            PLOGE << "SDL_InitSubSystem(SDL_INIT_VIDEO) failed: " << SDL_GetError();
            exit(1);
         }
         PLOGI << "Using video driver " << SDL_GetCurrentVideoDriver();
      #endif
      #ifdef ENABLE_SDL_INPUT
         if (!SDL_InitSubSystem(SDL_INIT_JOYSTICK))
         {
            PLOGE << "SDL_InitSubSystem(SDL_INIT_JOYSTICK) failed: " << SDL_GetError();
            exit(1);
         }
      #endif
      #ifdef __STANDALONE__
         TTF_Init();
      #endif

      // Start Win32++
      VPApp theApp(hInstance);
      theApp.InitInstance();
      MsgPluginManager::GetInstance().ScanPluginFolder(g_pvp->m_szMyPath + "plugins", [](MsgPlugin& plugin) {
         const char *enableDisable[] = { "Disabled", "Enabled" };
         int enabled = (int)VPXPluginAPIImpl::GetInstance().getAPI().GetOption(plugin.m_id.c_str(), 
            "Enable", VPX_OPT_SHOW_UI, "Enable plugin", 0.f, 1.f, 1.f, 0.f, VPXPluginAPI::NONE, enableDisable);
         if (enabled)
         {
            plugin.Load(&MsgPluginManager::GetInstance().GetMsgAPI());
         }
         else
         {
            PLOGI << "Plugin " << plugin.m_id << " was found but is disabled (" << plugin.m_library << ")";
         }
      });

      // Run the application
      retval = theApp.Run();
   }

   // catch all CException types
   catch (const CException &e)
   {
      // Display the exception and quit
      MessageBox(nullptr, e.GetText(), AtoT(e.what()), MB_ICONERROR);

      retval = -1;
   }

   MsgPluginManager::GetInstance().UnloadPlugins();
   
   #ifdef ENABLE_SDL_VIDEO
      SDL_QuitSubSystem(SDL_INIT_VIDEO);
   #endif
   #ifdef ENABLE_SDL_INPUT
      SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
   #endif
   #ifdef __STANDALONE__
      TTF_Quit();
   #endif

   #if defined(ENABLE_OPENGL) && !defined(__STANDALONE__) 
   if (s_OriginalNVidiaThreadOptimization != NV_THREAD_OPTIMIZATION_NO_SUPPORT && s_OriginalNVidiaThreadOptimization != NV_THREAD_OPTIMIZATION_DISABLE)
   {
      PLOGI << "Restoring NVIDIA Threaded Optimization";
      SetNVIDIAThreadOptimization(s_OriginalNVidiaThreadOptimization);
   }
   #endif

   PLOGI << "Closing VPX...\n\n";
   #if (defined(__STANDALONE__) && (defined(__APPLE__) && ((defined(TARGET_OS_IOS) && TARGET_OS_IOS) || (defined(TARGET_OS_TV) && TARGET_OS_TV))) || defined(__ANDROID__))
   exit(retval);
   #endif
   return retval;
}

#ifdef __STANDALONE__
#ifdef __ANDROID__
int main(int argc, char** argv) {
   while(true)
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
   return 0;
}
#elif ((defined(__APPLE__) && defined(TARGET_OS_TV) && TARGET_OS_TV) || defined(__linux__))
int main(int argc, char** argv) {
   g_argc = argc;
   g_argv = argv;
   return WinMain(NULL, NULL, NULL, 0);
}
#endif
#endif