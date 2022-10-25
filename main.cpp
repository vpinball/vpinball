// VPinball.cpp : Implementation of WinMain

#include "stdafx.h"

#ifdef CRASH_HANDLER
#include "StackTrace.h"
#include "CrashHandler.h"
#include "BlackBox.h"
#endif

#include "resource.h"
#include <initguid.h>

#define  SET_CRT_DEBUG_FIELD(a)   _CrtSetDbgFlag((a) | _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG))

#ifndef __STANDALONE__
#include "vpinball_i.c"
#else
#undef __declspec
#define __declspec(...)
#include "vpinball_i_standalone.c"
#endif

#ifdef CRASH_HANDLER
extern "C" int __cdecl _purecall(void)
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

#ifndef __STANDALONE__
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

#if (WINVER <= 0x0601 /* _WIN32_WINNT_WIN7 */ )
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

#if !defined(DEBUG_XXX) && !defined(_CRTDBG_MAP_ALLOC) //&& (!defined(__STDCPP_DEFAULT_NEW_ALIGNMENT__) || (__STDCPP_DEFAULT_NEW_ALIGNMENT__ < 16))
//!! somewhat custom new/delete still needed, otherwise VPX crashes when exiting the player
// is this due to win32xx's whacky Shared_Ptr implementation?
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

robin_hood::unordered_map<ItemTypeEnum, EditableInfo> EditableRegistry::m_map;

class VPApp : public CWinApp
{
private:
   bool run;
   bool play;
   bool extractPov;
   bool file;
   bool loadFileResult;
   bool extractScript;
   bool bgles;
   float fgles;
   string szTableFileName;
   VPinball m_vpinball;

public:
   VPApp(HINSTANCE hInstance)
   {
#ifndef __STANDALONE__
       m_vpinball.theInstance = GetInstanceHandle();
       SetResourceHandle(m_vpinball.theInstance);
#endif
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
   virtual BOOL InitInstance() 
   { 
#ifdef CRASH_HANDLER
      rde::CrashHandler::Init();
#endif

#ifdef _MSC_VER
      // disable auto-rotate on tablets
#if (WINVER <= 0x0601)
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
      m_vpinball.m_logicalNumberOfProcessors = sysinfo.dwNumberOfProcessors; //!! this ignores processor groups, so if at some point we need extreme multi threading, implement this in addition!
#else
      m_vpinball.m_logicalNumberOfProcessors = 1; //!!
#endif

      IsOnWine(); // init static variable in there

      InitXMLregistry(m_vpinball.m_szMyPath);

#ifdef _MSC_VER
#if _WIN32_WINNT >= 0x0400 & defined(_ATL_FREE_THREADED)
      const HRESULT hRes = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
#else
      const HRESULT hRes = CoInitialize(nullptr);
#endif
      _ASSERTE(SUCCEEDED(hRes));
      _Module.Init(ObjectMap, m_vpinball.theInstance, &LIBID_VPinballLib);
#endif

      file = false;
      play = false;
      extractPov = false;
      run = true;
      loadFileResult = true;
      extractScript = false;
      fgles = 0.f;
      bgles = false;

      szTableFileName.clear();

      // Start VP with file dialog open and then also playing that one?
      const bool stos = LoadValueBoolWithDefault(regKey[RegName::Editor], "SelectTableOnStart"s, true);
      if (stos)
      {
         file = true;
         play = true;
      }

#ifndef __STANDALONE__
      int nArgs;
      LPSTR *szArglist = CommandLineToArgvA(GetCommandLine(), &nArgs);
#else
      int nArgs = g_argc;
      char**  szArglist = g_argv;
#endif

      for (int i = 0; i < nArgs; ++i)
      {
         if (lstrcmpi(szArglist[i], _T("-h")) == 0 || lstrcmpi(szArglist[i], _T("/h")) == 0
            || lstrcmpi(szArglist[i], _T("-Help")) == 0 || lstrcmpi(szArglist[i], _T("/Help")) == 0
            || lstrcmpi(szArglist[i], _T("-?")) == 0 || lstrcmpi(szArglist[i], _T("/?")) == 0)
         {
            m_vpinball.MessageBox("-UnregServer  Unregister VP functions\n-RegServer  Register VP functions\n\n-DisableTrueFullscreen  Force-disable True Fullscreen setting\n-EnableTrueFullscreen  Force-enable True Fullscreen setting\n-Minimized  Start VP in the 'invisible' minimized window mode\n-ExtMinimized  Start VP in the 'invisible' minimized window mode, but with enabled Pause Menu\n-Primary  Force VP to render on the Primary/Pixel(0,0) Monitor\n\n-GLES [value]  Overrides the global emission scale (day/night setting, value range: 0.115..0.925)\n\n-LessCPUthreads  Limit the amount of parallel execution\n\n-Edit [filename]  Load file into VP\n-Play [filename]  Load and play file\n-PovEdit [filename]  Load and run file in camera mode, then export new pov on exit\n-Pov [filename]  Load, export pov and close\n-ExtractVBS [filename]  Load, export table script and close\n-c1 [customparam] .. -c9 [customparam]  Custom user parameters that can be accessed in the script via GetCustomParam(X)",
                 "Visual Pinball Usage", MB_OK);
            //run = false;
            exit(0);
         }

         //

         if (lstrcmpi(szArglist[i], _T("-LessCPUthreads")) == 0 || lstrcmpi(szArglist[i], _T("/LessCPUthreads")) == 0)
             m_vpinball.m_logicalNumberOfProcessors = max(min(m_vpinball.m_logicalNumberOfProcessors, 2), m_vpinball.m_logicalNumberOfProcessors/4); // only use 1/4th the threads, but at least 2 (if there are 2)

         //

         if (lstrcmpi(szArglist[i], _T("-UnregServer")) == 0 || lstrcmpi(szArglist[i], _T("/UnregServer")) == 0)
         {
#ifndef __STANDALONE__
            _Module.UpdateRegistryFromResource(IDR_VPINBALL, FALSE);
            const HRESULT ret = _Module.UnregisterServer(TRUE);
            if (ret != S_OK)
                ShowError("Unregister VP functions failed");
#endif
            run = false;
            break;
         }
         if (lstrcmpi(szArglist[i], _T("-RegServer")) == 0 || lstrcmpi(szArglist[i], _T("/RegServer")) == 0)
         {
#ifndef __STANDALONE__
            _Module.UpdateRegistryFromResource(IDR_VPINBALL, TRUE);
            const HRESULT ret = _Module.RegisterServer(TRUE);
            if (ret != S_OK)
                ShowError("Register VP functions failed");
#endif
            run = false;
            break;
         }

         //

         if (lstrcmpi(szArglist[i], _T("-DisableTrueFullscreen")) == 0 || lstrcmpi(szArglist[i], _T("/DisableTrueFullscreen")) == 0)
         {
             m_vpinball.m_disEnableTrueFullscreen = 0;
             continue;
         }
         if (lstrcmpi(szArglist[i], _T("-EnableTrueFullscreen")) == 0 || lstrcmpi(szArglist[i], _T("/EnableTrueFullscreen")) == 0)
         {
             m_vpinball.m_disEnableTrueFullscreen = 1;
             continue;
         }

         //

         bool useCustomParams = false;
         int customIdx = 1;
         for (char t = '1'; t <= '9'; t++)
         {
             const char cmdTemp1[4] = {'-','c',t,0};
             const char cmdTemp2[4] = {'/','c',t,0};
             if (lstrcmpi(szArglist[i], cmdTemp1) == 0 || lstrcmpi(szArglist[i], cmdTemp2) == 0)
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

         const bool minimized = (lstrcmpi(szArglist[i], _T("-Minimized")) == 0 || lstrcmpi(szArglist[i], _T("/Minimized")) == 0);
         if (minimized)
         {
             m_vpinball.m_open_minimized = true;
             m_vpinball.m_disable_pause_menu = true;
         }

         const bool ext_minimized = (lstrcmpi(szArglist[i], _T("-ExtMinimized")) == 0 || lstrcmpi(szArglist[i], _T("/ExtMinimized")) == 0);
         if (ext_minimized)
             m_vpinball.m_open_minimized = true;

         const bool editfile = (lstrcmpi(szArglist[i], _T("-Edit")) == 0 || lstrcmpi(szArglist[i], _T("/Edit")) == 0);
         const bool playfile = (lstrcmpi(szArglist[i], _T("-Play")) == 0 || lstrcmpi(szArglist[i], _T("/Play")) == 0);

         const bool gles = (lstrcmpi(szArglist[i], _T("-GLES")) == 0 || lstrcmpi(szArglist[i], _T("/GLES")) == 0);

         const bool primaryDisplay = (lstrcmpi(szArglist[i], _T("-Primary")) == 0 || lstrcmpi(szArglist[i], _T("/Primary")) == 0);
         if (primaryDisplay)
             m_vpinball.m_primaryDisplay = true;

         const bool povEdit  = (lstrcmpi(szArglist[i], _T("-PovEdit")) == 0 || lstrcmpi(szArglist[i], _T("/PovEdit")) == 0);
         if (povEdit)
             m_vpinball.m_povEdit = true;

         const bool extractpov = (lstrcmpi(szArglist[i], _T("-Pov")) == 0 || lstrcmpi(szArglist[i], _T("/Pov")) == 0);
         const bool extractscript = (lstrcmpi(szArglist[i], _T("-ExtractVBS")) == 0 || lstrcmpi(szArglist[i], _T("/ExtractVBS")) == 0);

         // global emission scale parameter handling
         if (gles && (i + 1 < nArgs))
         {
             char *lpszStr;
             if ((szArglist[i + 1][0] == '-') || (szArglist[i + 1][0] == '/'))
                 lpszStr = szArglist[i + 1] + 1;
             else
                 lpszStr = szArglist[i + 1];

            fgles = clamp((float)atof(lpszStr), 0.115f, 0.925f);
            bgles = true;
         }

         // table name handling
         if ((editfile || playfile || povEdit || extractpov || extractscript) && (i + 1 < nArgs))
         {
            file = true;
            play = playfile || povEdit;
            extractPov = extractpov;
            extractScript = extractscript;

#ifndef __STANDALONE__
            // Remove leading - or /
            if ((szArglist[i + 1][0] == '-') || (szArglist[i + 1][0] == '/'))
               szTableFileName = szArglist[i + 1] + 1;
            else
               szTableFileName = szArglist[i + 1];
#else
            szTableFileName = szArglist[i + 1];
#endif

            // Remove " "
            if (szTableFileName[0] == '"')
               szTableFileName = szTableFileName.substr(1, szTableFileName.size()-1);

#ifndef __STANDALONE__
            // Add current path
            if (szTableFileName[1] != ':') {
               char szLoadDir[MAXSTRING];
               GetCurrentDirectory(MAXSTRING, szLoadDir);
               szTableFileName = string(szLoadDir) + '\\' + szTableFileName;
            }
            else
               // Or set from table path
               if (play) {
                  const string dir = PathFromFilename(szTableFileName);
                  SetCurrentDirectory(dir.c_str());
               }
#endif

            ++i; // two params processed

            if (extractpov || extractscript)
               break;
            else
               continue;
         }
      }

#ifndef __STANDALONE__
      free(szArglist);

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
                  m_vpinball.MessageBox("Could not register type library. Try running Visual Pinball as administrator.", "Error", MB_ICONWARNING);
            }
            ptl->Release();
         }
         else
            m_vpinball.MessageBox("Could not load type library.", "Error", MB_ICONSTOP);
      }
#endif

      InitVPX();
      //SET_CRT_DEBUG_FIELD( _CRTDBG_LEAK_CHECK_DF );
      return TRUE;
   }

   void InitVPX()
   {
#ifndef __STANDALONE__
#if _WIN32_WINNT >= 0x0400 & defined(_ATL_FREE_THREADED)
       const HRESULT hRes = _Module.RegisterClassObjects(CLSCTX_LOCAL_SERVER,
           REGCLS_MULTIPLEUSE | REGCLS_SUSPENDED);
       _ASSERTE(SUCCEEDED(hRes));
       hRes = CoResumeClassObjects();
#else
       const HRESULT hRes = _Module.RegisterClassObjects(CLSCTX_LOCAL_SERVER,
           REGCLS_MULTIPLEUSE);
#endif
       _ASSERTE(SUCCEEDED(hRes));

       INITCOMMONCONTROLSEX iccex;
       iccex.dwSize = sizeof(INITCOMMONCONTROLSEX);
       iccex.dwICC = ICC_COOL_CLASSES;
       InitCommonControlsEx(&iccex);
#endif

       {
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
       }

       m_vpinball.AddRef();
       g_pvp = &m_vpinball;
       m_vpinball.Create(nullptr);
       m_vpinball.m_bgles = bgles;
       m_vpinball.m_fgles = fgles;

#ifndef __STANDALONE__
       g_haccel = LoadAccelerators(m_vpinball.theInstance, MAKEINTRESOURCE(IDR_VPACCEL));
#endif

       if (file)
       {
           if (!szTableFileName.empty())
           {
               m_vpinball.LoadFileName(szTableFileName, !play);
               m_vpinball.m_table_played_via_command_line = play;
           }
           else
           {
               loadFileResult = m_vpinball.LoadFile(!play);
               m_vpinball.m_table_played_via_SelectTableOnStart = LoadValueBoolWithDefault(regKey[RegName::Editor], "SelectTableOnPlayerClose"s, true) ? loadFileResult : false;
           }

           if (extractScript && loadFileResult)
           {
               string szScriptFilename = szTableFileName;
               if(ReplaceExtensionFromFilename(szScriptFilename, "vbs"s))
                   m_vpinball.m_ptableActive->m_pcv->SaveToFile(szScriptFilename);
               m_vpinball.Quit();
           }
           if (extractPov && loadFileResult)
           {
               string szPOVFilename = szTableFileName;
               if (ReplaceExtensionFromFilename(szPOVFilename, "pov"s))
                   m_vpinball.m_ptableActive->ExportBackdropPOV(szPOVFilename);
               m_vpinball.Quit();
           }
       }
   }

   virtual int Run()
   {
      if (run)
      {
         if (play && loadFileResult)
           m_vpinball.DoPlay(m_vpinball.m_povEdit);

         // VBA APC handles message loop (bastards)
         m_vpinball.MainMsgLoop();

         m_vpinball.Release();

#ifndef __STANDALONE__
         DestroyAcceleratorTable(g_haccel);

         _Module.RevokeClassObjects();
#endif
         Sleep(THREADS_PAUSE); //wait for any threads to finish

         SaveXMLregistry(m_vpinball.m_szMyPath);
         ClearXMLregistry();
      }
      return 0;
   }
};

extern "C" int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR /*lpCmdLine*/, int /*nShowCmd*/)
{
   int retval;
   try
   {
#if defined(ENABLE_SDL) || defined(ENABLE_SDL_INPUT)
      SDL_Init(0
#ifdef ENABLE_SDL
         | SDL_INIT_VIDEO
#endif
#ifdef ENABLE_SDL_INPUT
         | SDL_INIT_JOYSTICK
#endif
      );
#endif
      // Start Win32++
      VPApp theApp(hInstance);
      theApp.InitInstance();
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
#if defined(ENABLE_SDL) || defined(ENABLE_SDL_INPUT)
   SDL_Quit();
#endif
   return retval;
}
#ifdef __STANDALONE__
int main(int argc, char** argv) {
   g_argc = argc;
   g_argv = argv;

   return WinMain(nullptr, nullptr, (LPTSTR)"", 0);
}

HRESULT StgOpenStorage(const OLECHAR* pwcsName, IStorage* pstgPriority,
                       DWORD grfMode, SNB snbExclude, DWORD reserved,
                       IStorage** ppstgOpen) {
  PoleStorage* storage = new PoleStorage();
  return storage->OpenStorage((LPCOLESTR)pwcsName, pstgPriority, grfMode,
                              nullptr, reserved, ppstgOpen);
}

HRESULT WINAPI DirectSoundEnumerate(
    LPDSENUMCALLBACKA lpDSEnumCallback,
    LPVOID lpContext) {
   return S_OK;
}

extern "C" int wcsicmp( LPCWSTR str1, LPCWSTR str2 );

DEFINE_GUID(CLSID_Controller, 0xce9ecc7c, 0x960f, 0x407e, 0xb2,0x7b, 0x62,0xe3,0x9a,0xb1,0xe3,0x0f);
//MIDL_DEFINE_GUID(IID, IID_IController,0xce9ecc7c, 0x960f, 0x407e, 0xb2,0x7b, 0x62,0xe3,0x9a,0xb1,0xe3,0x0f);

HRESULT vpinball_create_object(const WCHAR *progid, IClassFactory* cf, IUnknown* obj) {
   if (!wcsicmp(progid, L"VPinMAME.Controller")) {
      IController* pController;

      //const HRESULT hres = CoCreateInstance(CLSID_Controller, 0, CLSCTX_ALL, IID_IController, (LPVOID*)&pController);
   }
    /*if (!wcsicmp(progid, L"Shell.Application")) {
    }
    else if (!wcsicmp(progid, L"WScript.Shell")) {
    }
    else if (!wcsicmp(progid, L"B2S.Server")) {
    }
    else 
    else if (!wcsicmp(progid, L"VPROC.Controller")) {
    }
    else if (!wcsicmp(progid, L"UltraDMD.DMDObject")) {
    }*/

   return E_NOTIMPL;
}
#endif
