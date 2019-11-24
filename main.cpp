// VPinball.cpp : Implementation of WinMain

#include "StdAfx.h"

#ifdef CRASH_HANDLER
#include "StackTrace.h"
#include "CrashHandler.h"
#include "BlackBox.h"
#endif

#include "resource.h"
#include <initguid.h>

#define  SET_CRT_DEBUG_FIELD(a)   _CrtSetDbgFlag((a) | _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG))

#include "vpinball_i.c"

#ifdef CRASH_HANDLER
extern "C" int __cdecl _purecall(void)
{
   ShowError("Pure Virtual Function Call");

   CONTEXT Context;
   ZeroMemory(&Context, sizeof(CONTEXT));
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

   char callStack[2048];
   ZeroMemory(callStack, sizeof(callStack));
   rde::StackTrace::GetCallStack(&Context, true, callStack, sizeof(callStack) - 1);

   ShowError(callStack);

   return 0;
}
#endif

#ifndef DISABLE_FORCE_NVIDIA_OPTIMUS
extern "C" {
   __declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}
#else
extern "C" {
   __declspec(dllexport) DWORD NvOptimusEnablement = 0x00000000;
}
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

static pSDARP SetDisplayAutoRotationPreferences = NULL;
#endif

#if !defined(DEBUG_XXX) && !defined(_CRTDBG_MAP_ALLOC)
void *operator new(const size_t size_req)
{
   return _aligned_malloc(size_req, 16);
}
void operator delete(void *address)
{
   _aligned_free(address);
}
void *operator new[](const size_t size_req)
{
   return _aligned_malloc(size_req, 16);
}
void operator delete[](void *address)
{
   _aligned_free(address);
}
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
   argv[argc] = NULL;

   (*_argc) = argc;
   return argv;
}

std::map<ItemTypeEnum, EditableInfo> EditableRegistry::m_map;
int disEnableTrueFullscreen = -1;


class VPApp : public CWinApp
{
private:
   HINSTANCE theInstance;
   bool run;
   bool play;
   bool extractPov;
   bool file;
   bool extractScript;
   TCHAR szTableFileName[MAXSTRING];

public:
   VPApp(HINSTANCE hInstance)
   {
       theInstance = GetInstanceHandle();
   }
   
   virtual ~VPApp() 
   {
      _Module.Term();
      CoUninitialize();
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

      // disable auto-rotate on tablets
#if (WINVER <= 0x0601)
      SetDisplayAutoRotationPreferences = (pSDARP)GetProcAddress(GetModuleHandle(TEXT("user32.dll")),
         "SetDisplayAutoRotationPreferences");
      if (SetDisplayAutoRotationPreferences)
         SetDisplayAutoRotationPreferences(ORIENTATION_PREFERENCE_LANDSCAPE);
#else
      SetDisplayAutoRotationPreferences(ORIENTATION_PREFERENCE_LANDSCAPE);
#endif

      g_hinst = theInstance;
#if _WIN32_WINNT >= 0x0400 & defined(_ATL_FREE_THREADED)
      const HRESULT hRes = CoInitializeEx(NULL, COINIT_MULTITHREADED);
#else
      const HRESULT hRes = CoInitialize(NULL);
#endif
      _ASSERTE(SUCCEEDED(hRes));
      _Module.Init(ObjectMap, theInstance, &LIBID_VPinballLib);

      file = false;
      play = false;
      extractPov = false;
      run = true;
      extractScript = false;

      memset(szTableFileName, 0, MAXSTRING);

      // Start VP with file dialog open and then also playing that one?
      const bool stos = LoadValueBoolWithDefault("Editor", "SelectTableOnStart", true);
      if (stos)
      {
         file = true;
         play = true;
         extractPov = false;
      }

      int nArgs;
      LPSTR *szArglist = CommandLineToArgvA(GetCommandLine(), &nArgs);

      for (int i = 0; i < nArgs; ++i)
      {
         if (lstrcmpi(szArglist[i], _T("-h")) == 0 || lstrcmpi(szArglist[i], _T("/h")) == 0
            || lstrcmpi(szArglist[i], _T("-Help")) == 0 || lstrcmpi(szArglist[i], _T("/Help")) == 0
            || lstrcmpi(szArglist[i], _T("-?")) == 0 || lstrcmpi(szArglist[i], _T("/?")) == 0)
         {
            ShowError("-UnregServer  Unregister VP functions\n-RegServer  Register VP functions\n\n-DisableTrueFullscreen  Force-disable True Fullscreen setting\n\n-EnableTrueFullscreen  Force-enable True Fullscreen setting\n\n-Edit [filename]  load file into VP\n-Play [filename]  load and play file\n-Pov [filename]  load, export pov and close\n-ExtractVBS [filename]  load, export table script and close\n-c1 [customparam] .. -c9 [customparam]  custom user parameters that can be accessed in the script via GetCustomParam(X)");
            run = false;
            break;
         }

         //

         if (lstrcmpi(szArglist[i], _T("-UnregServer")) == 0 || lstrcmpi(szArglist[i], _T("/UnregServer")) == 0)
         {
            _Module.UpdateRegistryFromResource(IDR_VPINBALL, FALSE);
            const HRESULT ret = _Module.UnregisterServer(TRUE);
            if (ret != S_OK)
                ShowError("Unregister VP functions failed");
            run = false;
            break;
         }
         if (lstrcmpi(szArglist[i], _T("-RegServer")) == 0 || lstrcmpi(szArglist[i], _T("/RegServer")) == 0)
         {
            _Module.UpdateRegistryFromResource(IDR_VPINBALL, TRUE);
            const HRESULT ret = _Module.RegisterServer(TRUE);
            if (ret != S_OK)
                ShowError("Register VP functions failed");
            run = false;
            break;
         }

         //

         if (lstrcmpi(szArglist[i], _T("-DisableTrueFullscreen")) == 0 || lstrcmpi(szArglist[i], _T("/DisableTrueFullscreen")) == 0)
         {
             disEnableTrueFullscreen = 0;
             continue;
         }
         if (lstrcmpi(szArglist[i], _T("-EnableTrueFullscreen")) == 0 || lstrcmpi(szArglist[i], _T("/EnableTrueFullscreen")) == 0)
         {
             disEnableTrueFullscreen = 1;
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
             VPinball::m_customParameters[customIdx - 1] = new WCHAR[len + 1];

             MultiByteToWideChar(CP_ACP, 0, szArglist[i + 1], (int)len, VPinball::m_customParameters[customIdx - 1], (int)len + 1);
             VPinball::m_customParameters[customIdx - 1][len] = L'\0';

             ++i; // two params processed

             continue;
         }

         //

         const bool editfile = (lstrcmpi(szArglist[i], _T("-Edit")) == 0 || lstrcmpi(szArglist[i], _T("/Edit")) == 0);
         const bool playfile = (lstrcmpi(szArglist[i], _T("-Play")) == 0 || lstrcmpi(szArglist[i], _T("/Play")) == 0);

         const bool extractpov = (lstrcmpi(szArglist[i], _T("-Pov")) == 0 || lstrcmpi(szArglist[i], _T("/Pov")) == 0);
         const bool extractscript = (lstrcmpi(szArglist[i], _T("-ExtractVBS")) == 0 || lstrcmpi(szArglist[i], _T("/ExtractVBS")) == 0);

         if ((editfile || playfile || extractpov || extractscript) && (i + 1 < nArgs))
         {
            file = true;
            play = playfile;
            extractPov = extractpov;
            extractScript = extractscript;

            // Remove leading - or /
            char* filename;
            if ((szArglist[i + 1][0] == '-') || (szArglist[i + 1][0] == '/'))
               filename = szArglist[i + 1] + 1;
            else
               filename = szArglist[i + 1];

            // Remove " "
            if (filename[0] == '"') {
               strcpy_s(szTableFileName, filename + 1);
               szTableFileName[lstrlen(szTableFileName) - 1] = '\0';
            }
            else
               strcpy_s(szTableFileName, filename);

            // Add current path
            char szLoadDir[MAX_PATH];
            if (szTableFileName[1] != ':') {
               GetCurrentDirectory(MAX_PATH, szLoadDir);
               strcat_s(szLoadDir, "\\");
               strcat_s(szLoadDir, szTableFileName);
               strcpy_s(szTableFileName, szLoadDir);
            }
            else
               // Or set from table path
               if (playfile) {
                  PathFromFilename(szTableFileName, szLoadDir);
                  SetCurrentDirectory(szLoadDir);
               }

            if (playfile || extractpov || extractscript)
               VPinball::SetOpenMinimized();

            ++i; // two params processed

            if (extractpov || extractscript)
               break;
            else
               continue;
         }
      }

      free(szArglist);

      // load and register VP type library for COM integration
      char szFileName[MAXSTRING];
      if (GetModuleFileName(theInstance, szFileName, MAXSTRING))
      {
         ITypeLib *ptl = NULL;
         MAKE_WIDEPTR_FROMANSI(wszFileName, szFileName);
         if (SUCCEEDED(LoadTypeLib(wszFileName, &ptl)))
         {
            // first try to register system-wide (if running as admin)
            HRESULT hr = RegisterTypeLib(ptl, wszFileName, NULL);
            if (!SUCCEEDED(hr))
            {
               // if failed, register only for current user
               hr = RegisterTypeLibForUser(ptl, wszFileName, NULL);
               if (!SUCCEEDED(hr))
                  MessageBox(0, "Could not register type library. Try running Visual Pinball as administrator.", "Error", MB_ICONWARNING);
            }
            ptl->Release();
         }
         else
            MessageBox(0, "Could not load type library.", "Error", MB_ICONSTOP);
      }

      //SET_CRT_DEBUG_FIELD( _CRTDBG_LEAK_CHECK_DF );
      return TRUE;
   }

   virtual int Run()
   {
      if (run)
      {
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

         g_pvp = new VPinball();
         g_pvp->AddRef();
         g_pvp->Init();
         g_haccel = LoadAccelerators(g_hinst, MAKEINTRESOURCE(IDR_VPACCEL));

         if (file)
         {
            bool lf = true;
            if (szTableFileName[0] != '\0')
               g_pvp->LoadFileName(szTableFileName);
            else
               lf = g_pvp->LoadFile();

            if (extractScript && lf)
            {
               TCHAR szScriptFilename[MAX_PATH];
               strcpy_s(szScriptFilename, szTableFileName);
               TCHAR *pos = strrchr(szScriptFilename, '.');
               if (pos)
               {
                  *pos = 0;
                  strcat_s(szScriptFilename, ".vbs");
                  g_pvp->m_ptableActive->m_pcv->SaveToFile(szScriptFilename);
               }
               g_pvp->Quit();
            }
            if (extractPov && lf)
            {
               TCHAR szPOVFilename[MAX_PATH];
               strcpy_s(szPOVFilename, szTableFileName);
               TCHAR *pos = strrchr(szPOVFilename, '.');
               if (pos)
               {
                  *pos = 0;
                  strcat_s(szPOVFilename, ".pov");
                  g_pvp->m_ptableActive->ExportBackdropPOV(szPOVFilename);
               }
               g_pvp->Quit();
            }

            if (play && lf)
               g_pvp->DoPlay(false);
         }

         // VBA APC handles message loop (bastards)
         g_pvp->MainMsgLoop();

         g_pvp->Release();

         // delete g_pvp;
         // Above causes frequent crashing!  COM objects should self destruct when they reach refcount 0?!
         g_pvp = NULL;

         DestroyAcceleratorTable(g_haccel);

         _Module.RevokeClassObjects();
         Sleep(THREADS_PAUSE); //wait for any threads to finish
      }
      return 0;
   }
};

extern "C" int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR /*lpCmdLine*/, int /*nShowCmd*/)
{
   int retval;
   try
   {
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
      MessageBox(NULL, e.GetText(), AtoT(e.what()), MB_ICONERROR);

      retval = -1;
   }
   return retval;
}
