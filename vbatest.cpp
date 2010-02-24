// VBATest.cpp : Implementation of WinMain


// Note: Proxy/Stub Information
//      To build a separate proxy/stub DLL,
//      run nmake -f VBATestps.mk in the project directory.

#include "stdafx.h"

#include "resource.h"
#include <initguid.h>
#include "main.h"

#define  SET_CRT_DEBUG_FIELD(a)   _CrtSetDbgFlag((a) | _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG))

#include "VBATest_i.c"


const DWORD dwTimeOut = 5000; // time for EXE to be idle before shutting down
const DWORD dwPause = 1000; // time to wait for threads to finish up

// Passed to CreateThread to monitor the shutdown event
static DWORD WINAPI MonitorProc(void* pv)
{
    CExeModule* p = (CExeModule*)pv;
    p->MonitorShutdown();
    return 0;
}

LONG CExeModule::Unlock()
{
    LONG l = CComModule::Unlock();
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
	BOOL fQuoteOn = fFalse;

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
extern "C" int WINAPI _tWinMain(HINSTANCE hInstance,
    HINSTANCE /*hPrevInstance*/, LPTSTR lpCmdLine, int /*nShowCmd*/)
{

#ifdef GLOBALLOG
	logfile = fopen("c:\\vpgloballog.txt","w");
#endif
	
	g_hinst = hInstance;

	g_hinstres = LoadLibrary("vpinres.dll");

	if (g_hinstres == NULL)
		{
		g_hinstres = g_hinst;
		}

	SYSTEMTIME systime; // time bomb
	GetSystemTime(&systime);

	/*if (systime.wYear > 2008)
		{
		LocalString ls(IDS_BETAEXPIRE);
		MessageBox(NULL, ls.m_szbuffer, "Visual Pinball", 0);
		return 10;
		}*/

    lpCmdLine = GetCommandLine(); //this line necessary for _ATL_MIN_CRT

	BOOL fFile = fFalse;
	BOOL fPlay = fFalse;
	TCHAR szTableFileName[_MAX_PATH];

	TCHAR szOption[256];

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
			//MessageBox(NULL, szTableFileName, "File", 0);
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
		
		if (fFile && fPlay)
			{
			// This value has to be set before calling Init, or the editor window will appear
			g_pvp->m_fPlayOnly = fTrue;
			}
			
		g_pvp->Init();
		g_haccel = LoadAccelerators(g_hinstres,MAKEINTRESOURCE(IDR_VPACCEL));

		if (fFile)
			{
			int len;
			// Strip header and trailer quotes
			len = lstrlen(szTableFileName);
			szTableFileName[len-1] = 0;
			g_pvp->LoadFileName(&szTableFileName[1]);

			if (fPlay)
				{
				g_pvp->DoPlay();
				}
			}

		// VBA APC handles message loop (bastards)
		g_pvp->MainMsgLoop();

		//delete g_pvp;

#ifdef VBA
		g_pvp->ApcHost.Destroy();
#endif
		g_pvp->Release();

        /*MSG msg;
        while (GetMessage(&msg, 0, 0, 0))
            DispatchMessage(&msg);*/

		DestroyAcceleratorTable(g_haccel);

        _Module.RevokeClassObjects();
        Sleep(dwPause); //wait for any threads to finish
    }
    
#ifdef GLOBALLOG
	fclose(logfile);
#endif

    _Module.Term();
    CoUninitialize();
	_CrtSetDumpClient(MemLeakAlert);
	_CrtDumpMemoryLeaks( );
	//SET_CRT_DEBUG_FIELD( _CRTDBG_LEAK_CHECK_DF );

    return nRet;
}

