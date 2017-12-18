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
   i = ((len + 2) / 2)*sizeof(PVOID) + sizeof(PVOID);

   argv = (PCHAR*)malloc(i + (len + 2)*sizeof(CHAR));
   _argv = (PCHAR)(((PUCHAR)argv) + i);

   argc = 0;
   argv[argc] = _argv;
   in_QM = FALSE;
   in_TEXT = FALSE;
   in_SPACE = TRUE;
   i = 0;
   j = 0;

   while (a = CmdLine[i]) {
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
bool disableTrueFullscreen = false;


class VPApp : public CWinApp
{
private:
   HINSTANCE theInstance;
   HRESULT hRes;
   bool bRun;
   bool fPlay;
   bool fPov;
   bool fFile;
   TCHAR szTableFileName[_MAX_PATH];

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
      hRes = CoInitializeEx(NULL, COINIT_MULTITHREADED);
#else
      hRes = CoInitialize(NULL);
#endif
      _ASSERTE(SUCCEEDED(hRes));
      _Module.Init(ObjectMap, theInstance, &LIBID_VPinballLib);

      fFile = false;
      fPlay = false;
      fPov = false;
      bRun = true;
      int nRet = 0;
      memset(szTableFileName, 0, _MAX_PATH);
      // Start VP with file dialog open and then also playing that one?
      int stos;
      HRESULT hr = GetRegInt("Editor", "SelectTableOnStart", &stos);
      if (hr != S_OK)
         stos = 1; // The default = on
      if (stos)
      {
         fFile = true;
         fPlay = true;
         fPov = false;
      }

      int nArgs;
      LPSTR *szArglist = CommandLineToArgvA(GetCommandLine(), &nArgs);

      for (int i = 0; i < nArgs; ++i)
      {
         if (lstrcmpi(szArglist[i], _T("-h")) == 0 || lstrcmpi(szArglist[i], _T("/h")) == 0
            || lstrcmpi(szArglist[i], _T("-Help")) == 0 || lstrcmpi(szArglist[i], _T("/Help")) == 0
            || lstrcmpi(szArglist[i], _T("-?")) == 0 || lstrcmpi(szArglist[i], _T("/?")) == 0)
         {
            ShowError("-UnregServer  Unregister VP functions\n-RegServer  Register VP functions\n\n-DisableTrueFullscreen  Force-disable True Fullscreen setting\n\n-Edit [filename]  load file into VP\n-Play [filename]  load and play file\n-Pov [filename]  load, export pov and close");
            bRun = false;
            break;
         }

         if (lstrcmpi(szArglist[i], _T("-UnregServer")) == 0 || lstrcmpi(szArglist[i], _T("/UnregServer")) == 0)
         {
            _Module.UpdateRegistryFromResource(IDR_VPINBALL, FALSE);
            nRet = _Module.UnregisterServer(TRUE);
            bRun = false;
            break;
         }
         if (lstrcmpi(szArglist[i], _T("-RegServer")) == 0 || lstrcmpi(szArglist[i], _T("/RegServer")) == 0)
         {
            _Module.UpdateRegistryFromResource(IDR_VPINBALL, TRUE);
            nRet = _Module.RegisterServer(TRUE);
            bRun = false;
            break;
         }

         disableTrueFullscreen |= (lstrcmpi(szArglist[i], _T("-DisableTrueFullscreen")) == 0 || lstrcmpi(szArglist[i], _T("/DisableTrueFullscreen")) == 0);

         const bool editfile = (lstrcmpi(szArglist[i], _T("-Edit")) == 0 || lstrcmpi(szArglist[i], _T("/Edit")) == 0);
         const bool playfile = (lstrcmpi(szArglist[i], _T("-Play")) == 0 || lstrcmpi(szArglist[i], _T("/Play")) == 0);
		 const bool povfile = (lstrcmpi(szArglist[i], _T("-Pov")) == 0 || lstrcmpi(szArglist[i], _T("/Pov")) == 0);
		 if ((editfile || playfile || povfile) && (i + 1 < nArgs))
         {
            fFile = true;
            fPlay = playfile;
			fPov = povfile;

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

            if (playfile || povfile)
               VPinball::SetOpenMinimized();

            break;
         }
      }

      free(szArglist);

      // load and register VP type library for COM integration
      char szFileName[_MAX_PATH];
      if (GetModuleFileName(theInstance, szFileName, _MAX_PATH))
      {
         ITypeLib *ptl = NULL;
         MAKE_WIDEPTR_FROMANSI(wszFileName, szFileName);
         if (SUCCEEDED(LoadTypeLib(wszFileName, &ptl)))
         {
            // first try to register system-wide (if running as admin)
            hr = RegisterTypeLib(ptl, wszFileName, NULL);
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
      if (bRun)
      {
#if _WIN32_WINNT >= 0x0400 & defined(_ATL_FREE_THREADED)
         hRes = _Module.RegisterClassObjects(CLSCTX_LOCAL_SERVER,
            REGCLS_MULTIPLEUSE | REGCLS_SUSPENDED);
         _ASSERTE(SUCCEEDED(hRes));
         hRes = CoResumeClassObjects();
#else
         hRes = _Module.RegisterClassObjects(CLSCTX_LOCAL_SERVER,
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

         if (fFile)
         {
            bool lf = true;
            if (szTableFileName[0] != '\0')
               g_pvp->LoadFileName(szTableFileName);
            else
               lf = g_pvp->LoadFile();

            if (fPlay && lf)
               g_pvp->DoPlay();

			if (fPov && lf)
			   g_pvp->Quit();
         }

         // VBA APC handles message loop (bastards)
         g_pvp->MainMsgLoop();

         g_pvp->Release();

         DestroyAcceleratorTable(g_haccel);

         _Module.RevokeClassObjects();
         Sleep(THREADS_PAUSE); //wait for any threads to finish
      }
      return 0;
   }
};

extern "C" int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR /*lpCmdLine*/, int /*nShowCmd*/)
{
   try
   {
      // Start Win32++
      VPApp theApp(hInstance);
      theApp.InitInstance();
      // Run the application
      return theApp.Run();
   }

   // catch all CException types
   catch (const CException &e)
   {
      // Display the exception and quit
      MessageBox(NULL, e.GetText(), AtoT(e.what()), MB_ICONERROR);

      return -1;
   }
}
