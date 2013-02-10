// VBATest.cpp : Implementation of WinMain

// Note: Proxy/Stub Information
//      To build a separate proxy/stub DLL,
//      run nmake -f VBATestps.mk in the project directory.

#include "StdAfx.h"

#include "resource.h"
#include <initguid.h>

#define  SET_CRT_DEBUG_FIELD(a)   _CrtSetDbgFlag((a) | _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG))

#include "VBATest_i.c"


const DWORD dwTimeOut = 5000; // time for EXE to be idle before shutting down
const DWORD dwPause = 1000; // time to wait for threads to finish up

// Passed to CreateThread to monitor the shutdown event
static DWORD WINAPI MonitorProc(void* pv)
{
    CExeModule * const p = (CExeModule*)pv;
    p->MonitorShutdown();
    return 0;
}

LONG CExeModule::Unlock()
{
    const LONG l = CComModule::Unlock();
    if (l == 0)
    {
        bActivity = true;
        SetEvent(hEventShutdown); // tell monitor that we transitioned to zero
    }
    return l;
}

//Monitors the shutdown event
void CExeModule::MonitorShutdown()
{
    while (1)
    {
        WaitForSingleObject(hEventShutdown, INFINITE);
        DWORD dwWait=0;
        do
        {
            bActivity = false;
            dwWait = WaitForSingleObject(hEventShutdown, dwTimeOut);
        } while (dwWait == WAIT_OBJECT_0);
        // timed out
        if (!bActivity && m_nLockCnt == 0) // if no activity let's really bail
        {
#if _WIN32_WINNT >= 0x0400 & defined(_ATL_FREE_THREADED)
            CoSuspendClassObjects();
            if (!bActivity && m_nLockCnt == 0)
#endif
                break;
        }
    }
    CloseHandle(hEventShutdown);
    PostThreadMessage(dwThreadID, WM_QUIT, 0, 0);
}

bool CExeModule::StartMonitor()
{
    hEventShutdown = CreateEvent(NULL, false, false, NULL);
    if (hEventShutdown == NULL)
        return false;
    DWORD dwThreadID;
    HANDLE h = CreateThread(NULL, 0, MonitorProc, this, 0, &dwThreadID);
    return (h != NULL);
}

CExeModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
END_OBJECT_MAP()


PCHAR* CommandLineToArgvA(PCHAR CmdLine, int* _argc)
{
	PCHAR*  argv;
	PCHAR   _argv;
	ULONG   len;
	ULONG   argc;
	CHAR    a;
	ULONG   i, j;

	BOOLEAN  in_QM;
	BOOLEAN  in_TEXT;
	BOOLEAN  in_SPACE;

	len = strlen(CmdLine);
	i = ((len+2)/2)*sizeof(PVOID) + sizeof(PVOID);

	argv = (PCHAR*)malloc(i + (len+2)*sizeof(CHAR));
	_argv = (PCHAR)(((PUCHAR)argv)+i);

	argc = 0;
	argv[argc] = _argv;
	in_QM = FALSE;
	in_TEXT = FALSE;
	in_SPACE = TRUE;
	i = 0;
	j = 0;

	while( a = CmdLine[i] ) {
		if(in_QM) {
			if(a == '\"') {
				in_QM = FALSE;
			} else {
				_argv[j] = a;
				j++;
			}
		} else {
			switch(a) {
			case '\"':
				in_QM = TRUE;
				in_TEXT = TRUE;
				if(in_SPACE) {
					argv[argc] = _argv+j;
					argc++;
				}
				in_SPACE = FALSE;
				break;
			case ' ':
			case '\t':
			case '\n':
			case '\r':
				if(in_TEXT) {
					_argv[j] = '\0';
					j++;
				}
				in_TEXT = FALSE;
				in_SPACE = TRUE;
				break;
			default:
				in_TEXT = TRUE;
				if(in_SPACE) {
					argv[argc] = _argv+j;
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

extern "C" int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR /*lpCmdLine*/, int /*nShowCmd*/)
{
#ifdef GLOBALLOG
	logfile = fopen("c:\\vpgloballog.txt","w");
#endif
	
	g_hinst = hInstance;

//	g_hinstres = LoadLibrary("vpinres.dll");

//	if (g_hinstres == NULL)
//		{
		g_hinstres = g_hinst;
//		}

#if _WIN32_WINNT >= 0x0400 & defined(_ATL_FREE_THREADED)
    HRESULT hRes = CoInitializeEx(NULL, COINIT_MULTITHREADED);
#else
    HRESULT hRes = CoInitialize(NULL);
#endif
    _ASSERTE(SUCCEEDED(hRes));
    _Module.Init(ObjectMap, hInstance, &LIBID_VBATESTLib);
    _Module.dwThreadID = GetCurrentThreadId();

	BOOL fFile = fFalse;
	BOOL fPlay = fFalse;
	TCHAR szTableFileName[_MAX_PATH] = {0};
    int nRet = 0;
    BOOL bRun = TRUE;

    int nArgs;
	LPSTR *szArglist = CommandLineToArgvA(GetCommandLine(), &nArgs);

	for(int i=0; i < nArgs; ++i)
    {
        if (lstrcmpi(szArglist[i], _T("-UnregServer"))==0 || lstrcmpi(szArglist[i], _T("/UnregServer"))==0)
        {
            _Module.UpdateRegistryFromResource(IDR_VBATest, FALSE);
            nRet = _Module.UnregisterServer(TRUE);
            bRun = FALSE;
			break;
        }
        if (lstrcmpi(szArglist[i], _T("-RegServer"))==0 || lstrcmpi(szArglist[i], _T("/RegServer"))==0)
        {
            _Module.UpdateRegistryFromResource(IDR_VBATest, TRUE);
            nRet = _Module.RegisterServer(TRUE);
            bRun = FALSE;
			break;
        }
        if ((lstrcmpi(szArglist[i], _T("-Edit"))==0 || lstrcmpi(szArglist[i], _T("/Edit"))==0) && (i+1 < nArgs))
        {
			fFile = fTrue;
			if(szArglist[i+1][0] == '"') {
				strcpy_s(szTableFileName,szArglist[i+1]+1);
				szTableFileName[strlen(szTableFileName)] = 0;
			}
			else
				strcpy_s(szTableFileName,szArglist[i+1]);
			
			if(szTableFileName[1] != ':') {
				char szLoadDir[MAX_PATH];
				GetCurrentDirectory(MAX_PATH,szLoadDir);
				strcat_s(szLoadDir,"\\");
				strcat_s(szLoadDir,szTableFileName);
				strcpy_s(szTableFileName,szLoadDir);
			}
			break;
        }
        if ((lstrcmpi(szArglist[i], _T("-Play"))==0 || lstrcmpi(szArglist[i], _T("/Play"))==0) && (i+1 < nArgs))
        {
			fFile = fTrue;
			fPlay = fTrue;
			if(szArglist[i+1][0] == '"') {
				strcpy_s(szTableFileName,szArglist[i+1]+1);
				szTableFileName[strlen(szTableFileName)] = 0;
			}
			else
				strcpy_s(szTableFileName,szArglist[i+1]);

			char szLoadDir[MAX_PATH];
			if(szTableFileName[1] != ':') {
				GetCurrentDirectory(MAX_PATH,szLoadDir);
				strcat_s(szLoadDir,"\\");
				strcat_s(szLoadDir,szTableFileName);
				strcpy_s(szTableFileName,szLoadDir);
			} else {
				PathFromFilename(szTableFileName, szLoadDir);
				/*const DWORD err =*/ SetCurrentDirectory(szLoadDir);
			}

			VPinball::SetOpenMinimized();
			break;
        }
    }

	free(szArglist);

	{
	char szFileName[_MAX_PATH];
	if (GetModuleFileName(hInstance, szFileName, _MAX_PATH))
		{
		ITypeLib *ptl = NULL;
		MAKE_WIDEPTR_FROMANSI(wszFileName, szFileName);
		if (SUCCEEDED(LoadTypeLib(wszFileName, &ptl)))
			{
			RegisterTypeLib(ptl, wszFileName, NULL);
			ptl->Release();
			}
		}
	}

    if (bRun)
    {
        //_Module.StartMonitor();
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

		g_pvp = new VPinball();
		g_pvp->AddRef();
		g_pvp->Init();
		g_haccel = LoadAccelerators(g_hinstres,MAKEINTRESOURCE(IDR_VPACCEL));

		if (fFile)
			{
			g_pvp->LoadFileName(szTableFileName);

			if (fPlay)
				g_pvp->DoPlay();
			}

		// VBA APC handles message loop (bastards)
		g_pvp->MainMsgLoop();

#ifdef VBA
		g_pvp->ApcHost.Destroy();
#endif
		g_pvp->Release();

		DestroyAcceleratorTable(g_haccel);

        _Module.RevokeClassObjects();
        Sleep(dwPause); //wait for any threads to finish
    }

#ifdef GLOBALLOG
	fclose(logfile);
#endif

    _Module.Term();
    CoUninitialize();
#ifdef DEBUG_XXX  //disable this in perference to DevPartner
	_CrtSetDumpClient(MemLeakAlert);
	_CrtDumpMemoryLeaks( );
#endif
	//SET_CRT_DEBUG_FIELD( _CRTDBG_LEAK_CHECK_DF );

    return nRet;
}
