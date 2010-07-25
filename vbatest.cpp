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


LPCTSTR FindOneOf(LPCTSTR p1, LPCTSTR p2, LPTSTR pOut)
{
	bool fQuoteOn = false;

    while (p1 != NULL && *p1 != NULL)
    {
        LPCTSTR p = p2;
        while (p != NULL && *p != NULL)
        {
            if (*p1 == *p)
				{
				p1 = CharNext(p1);

				// Allow spaces contained within quotes
				if (*p1 == '"')
					{
					fQuoteOn = !fQuoteOn;
					}

				while (p1 != NULL && *p1 != NULL && (fQuoteOn || *p1 != ' '))
					{
					*pOut = *p1;
					pOut = CharNext(pOut);
					p1 = CharNext(p1);
					}
				*pOut = 0;
                return p1;
				}
			p = CharNext(p);
        }
        p1 = CharNext(p1);
    }
    return NULL;

}


/////////////////////////////////////////////////////////////////////////////
//
//rlc the beginning of it all  <<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>>>
//
//
extern "C" int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpCmdLine, int /*nShowCmd*/)
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

	SYSTEMTIME systime; // time bomb
	GetSystemTime(&systime);

    lpCmdLine = GetCommandLine(); //this line necessary for _ATL_MIN_CRT

	BOOL fFile = fFalse;
	BOOL fPlay = fFalse;
	TCHAR szTableFileName[_MAX_PATH] = {0};
	char  szTableResolution[64] = {0};
	//BOOL  fullscreen = fFalse;

	TCHAR szOption[256]= {0};

#if _WIN32_WINNT >= 0x0400 & defined(_ATL_FREE_THREADED)
    HRESULT hRes = CoInitializeEx(NULL, COINIT_MULTITHREADED);
#else
    HRESULT hRes = CoInitialize(NULL);
#endif
    _ASSERTE(SUCCEEDED(hRes));
    _Module.Init(ObjectMap, hInstance, &LIBID_VBATESTLib);
    _Module.dwThreadID = GetCurrentThreadId();
    TCHAR szTokens[] = _T("-/");

    int nRet = 0;
    BOOL bRun = TRUE;
    LPCTSTR lpszToken = FindOneOf(lpCmdLine, szTokens, szOption);


    while (lpszToken != NULL)
    {
        if (lstrcmpi(szOption, _T("UnregServer"))==0)
        {
            _Module.UpdateRegistryFromResource(IDR_VBATest, FALSE);
            nRet = _Module.UnregisterServer(TRUE);
            bRun = FALSE;
            break;
        }
        if (lstrcmpi(szOption, _T("RegServer"))==0)
        {
            _Module.UpdateRegistryFromResource(IDR_VBATest, TRUE);
            nRet = _Module.RegisterServer(TRUE);
            bRun = FALSE;
            break;
        }
        if (lstrcmpi(szOption, _T("Edit"))==0)
        {
			fFile = fTrue;
			lpszToken = FindOneOf(lpszToken, szTokens, szTableFileName);
			//MessageBox(NULL, szTableFileName, "File", 0);
            break;
        }
        if (lstrcmpi(szOption, _T("Play"))==0)
        {
			fFile = fTrue;
			fPlay = fTrue;
			lpszToken = FindOneOf(lpszToken, szTokens, szTableFileName);

//#ifdef ULTRACADE
			char *play = StrStrI( lpCmdLine, "Play");
		
			if( play )
			{
				sprintf_s( szTableFileName, "%s", play+6 );
				VPinball::SetOpenMinimized();
				if(1)
				{
					char szLoadDir[MAX_PATH];
					PathFromFilename(szTableFileName, szLoadDir);
					/*const DWORD err =*/ SetCurrentDirectory(szLoadDir);
				}
			}

			char *resolution = StrStrI( lpCmdLine, "Resolution");

			if( resolution )
			{
				sprintf_s( szTableResolution, "%s", resolution+11 );				
			}

			//fullscreen = StrStrI( lpCmdLine, "Fullscreen") != NULL;

//#endif
            break;
        }
        lpszToken = FindOneOf(lpszToken, szTokens, szOption);
    }

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
			int len;
			// Strip header and trailer quotes (but only if they exist - AMH)
			len = lstrlen(szTableFileName);

			if( szTableFileName[0] == '"' )
			{
				szTableFileName[len-1] = 0;
				g_pvp->LoadFileName(&szTableFileName[1]);
			}
			else
			{
				g_pvp->LoadFileName(&szTableFileName[0]);
			}

			if (fPlay)
				{
				g_pvp->DoPlay();
				}
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
#ifdef DEBUGxxx  //disable this in perference to DevPartner
	_CrtSetDumpClient(MemLeakAlert);
	_CrtDumpMemoryLeaks( );
#endif
	//SET_CRT_DEBUG_FIELD( _CRTDBG_LEAK_CHECK_DF );

    return nRet;
}
