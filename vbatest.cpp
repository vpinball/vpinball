// VBATest.cpp : Implementation of WinMain

#include "StdAfx.h"

#include "resource.h"
#include <initguid.h>

#define  SET_CRT_DEBUG_FIELD(a)   _CrtSetDbgFlag((a) | _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG))

#include "VBATest_i.c"


#if !defined(DEBUG_XXX) && !defined(_CRTDBG_MAP_ALLOC)
void *operator new( const size_t size_req )
{
   return _aligned_malloc( size_req, 16 );
}
void operator delete( void *address )
{
   _aligned_free(address);
}
void *operator new[]( const size_t size_req )
{
   return _aligned_malloc( size_req, 16 );
}
void operator delete[]( void *address )
{
   _aligned_free(address);
}
#endif

const DWORD dwPause = 1000; // time to wait for threads to finish up

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
	g_hinst = hInstance;
   
#if _WIN32_WINNT >= 0x0400 & defined(_ATL_FREE_THREADED)
    HRESULT hRes = CoInitializeEx(NULL, COINIT_MULTITHREADED);
#else
    HRESULT hRes = CoInitialize(NULL);
#endif
    _ASSERTE(SUCCEEDED(hRes));
    _Module.Init(ObjectMap, hInstance, &LIBID_VBATESTLib);

	bool fFile = false;
	bool fPlay = false;
    bool bRun = true;
	TCHAR szTableFileName[_MAX_PATH] = {0};
    int nRet = 0;

    int nArgs;
	LPSTR *szArglist = CommandLineToArgvA(GetCommandLine(), &nArgs);

	for(int i=0; i < nArgs; ++i)
    {
        if (lstrcmpi(szArglist[i], _T("-UnregServer"))==0 || lstrcmpi(szArglist[i], _T("/UnregServer"))==0)
        {
            _Module.UpdateRegistryFromResource(IDR_VBATest, FALSE);
            nRet = _Module.UnregisterServer(TRUE);
            bRun = false;
			break;
        }
        if (lstrcmpi(szArglist[i], _T("-RegServer"))==0 || lstrcmpi(szArglist[i], _T("/RegServer"))==0)
        {
            _Module.UpdateRegistryFromResource(IDR_VBATest, TRUE);
            nRet = _Module.RegisterServer(TRUE);
            bRun = false;
			break;
        }

		const bool editfile = (lstrcmpi(szArglist[i], _T("-Edit"))==0 || lstrcmpi(szArglist[i], _T("/Edit"))==0);
		const bool playfile = (lstrcmpi(szArglist[i], _T("-Play"))==0 || lstrcmpi(szArglist[i], _T("/Play"))==0);
        if ((editfile || playfile) && (i+1 < nArgs))
        {
			fFile = true;
			fPlay = playfile;

			// Remove leading - or /
			char* filename;
			if((szArglist[i+1][0] == '-') || (szArglist[i+1][0] == '/'))
				filename = szArglist[i+1]+1;
			else
				filename = szArglist[i+1];

			// Remove " "
			if(filename[0] == '"') {
				strcpy_s(szTableFileName,filename+1);
				szTableFileName[strlen(szTableFileName)] = 0;
			}
			else
				strcpy_s(szTableFileName,filename);

			// Add current path
			char szLoadDir[MAX_PATH];
			if(szTableFileName[1] != ':') {
				GetCurrentDirectory(MAX_PATH,szLoadDir);
				strcat_s(szLoadDir,"\\");
				strcat_s(szLoadDir,szTableFileName);
				strcpy_s(szTableFileName,szLoadDir);
			} else
				// Or set from table path
				if(playfile) {
					PathFromFilename(szTableFileName, szLoadDir);
					SetCurrentDirectory(szLoadDir);
				}

			if(playfile)
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
		g_haccel = LoadAccelerators(g_hinst,MAKEINTRESOURCE(IDR_VPACCEL));

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

    _Module.Term();
    CoUninitialize();
#ifdef _CRTDBG_MAP_ALLOC
	_CrtDumpMemoryLeaks();
#endif
#ifdef DEBUG_XXX  //disable this in perference to DevPartner
	_CrtSetDumpClient(MemLeakAlert);
	_CrtDumpMemoryLeaks( );
#endif
	//SET_CRT_DEBUG_FIELD( _CRTDBG_LEAK_CHECK_DF );

    return nRet;
}
