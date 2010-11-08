// VPinball.cpp: implementation of the VPinball class.
//
/////////////////////////////////////////////////////

#include "StdAfx.h"

#include "buildnumber.h"
#include "SVNRevision.h"
#include "resource.h"

//#include "Freeimage.h" //ADDED BDS

#if _MSC_VER <= 1310 // VC 2003 and before
 #define _itoa_s(a,b,c,d) _itoa(a,b,d)
#endif

#if defined(IMSPANISH)
#define TOOLBAR_WIDTH 152
#elif defined(IMGERMAN)
#define TOOLBAR_WIDTH 152
#else
#define TOOLBAR_WIDTH 102 //98 //102
#endif

#define SCROLL_WIDTH GetSystemMetrics(SM_CXVSCROLL)

#define MAIN_WINDOW_WIDTH		800
#define MAIN_WINDOW_HEIGHT		550

#define GET_WINDOW_MODES		WM_USER+100
#define GET_FULLSCREENMODES		WM_USER+101
#define RESET_SIZELIST_CONTENT	WM_USER+102

#define	RECENT_FIRST_MENU_IDM	5000	// ID of the first recent file list filename
#define	RECENT_LAST_MENU_IDM	RECENT_FIRST_MENU_IDM+LAST_OPENED_TABLE_COUNT

#define AUTOSAVE_DEFAULT_TIME 10
//extern int uShockType;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//Which column the window menu is, for MDI
#define WINDOWMENU 5

TBBUTTON const g_tbbuttonMain[] = {
	// icon number,
	{14, IDC_MAGNIFY, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, IDS_TB_MAGNIFY, 0},
	{0, IDC_SELECT, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP | TBSTYLE_DROPDOWN, 0, 0, IDS_TB_SELECT, 1},
	{13, ID_EDIT_PROPERTIES, TBSTATE_ENABLED, TBSTYLE_CHECK, 0, 0, IDS_TB_PROPERTIES, 2},
	{18, ID_EDIT_SCRIPT, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, IDS_TB_SCRIPT, 3},
	{19, ID_EDIT_BACKGLASSVIEW, TBSTATE_ENABLED, TBSTYLE_CHECK, 0, 0, IDS_TB_BACKGLASS, 4},
	{2, ID_TABLE_PLAY, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, IDS_TB_PLAY, 5},
	};

TBBUTTON const g_tbbuttonPalette[] = {
	// icon number,
	{1, IDC_WALL, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, IDS_TB_WALL, 0},
	{15, IDC_GATE, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, IDS_TB_GATE, 1},
	{17, IDC_RAMP, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, IDS_TB_RAMP, 2},
	{3, IDC_FLIPPER, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, IDS_TB_FLIPPER, 3},
	{5, IDC_PLUNGER, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, IDS_TB_PLUNGER, 4},
	{7, IDC_BUMPER, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, IDS_TB_BUMPER, 5},
	{16, IDC_SPINNER, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, IDS_TB_SPINNER, 6},
	{4, IDC_TIMER, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, IDS_TB_TIMER, 7},
	{8, IDC_TRIGGER, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, IDS_TB_TRIGGER, 8},
	{9, IDC_LIGHT, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, IDS_TB_LIGHT, 9},
	{10, IDC_KICKER, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, IDS_TB_KICKER, 10},
	{11, IDC_TARGET, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, IDS_TB_TARGET, 11},
	{12, IDC_DECAL, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, IDS_TB_DECAL, 12},
	{6, IDC_TEXTBOX, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, IDS_TB_TEXTBOX, 13},
    {20, IDC_DISPREEL, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, IDS_TB_DISPREEL, 14},
	{21, IDC_LIGHTSEQ, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, IDS_TB_LIGHTSEQ, 15},
	//{22, IDC_COMCONTROL, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, IDS_TB_COMCONTROL, 16},
	};

MiniBitmapID const g_rgminibitmap[] = {
	{L"{44D58C78-14BA-47F3-B82C-425853ABB698}", IDB_ICON_TRIGGER}, // Trigger
	{L"{A0B914E6-56A8-4CC1-A846-45FFF4D8CA17}", IDB_ICON_TIMER}, // Timer
	{L"{68AB2BBC-8209-40F3-B6F4-54F8ADAA0DC7}", IDB_ICON_FLIPPER}, // Flipper
	{L"{31DD37E7-DB9B-4AB1-94C9-FAA06B252DFA}", IDB_ICON_LIGHT}, // Light
	NULL, -1,
	};

#define TBCOUNTMAIN (sizeof(g_tbbuttonMain) / sizeof(TBBUTTON))
#define TBCOUNTPALETTE (sizeof(g_tbbuttonPalette) / sizeof(TBBUTTON))

LRESULT CALLBACK VPWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK VPSideBarWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int CALLBACK SoundManagerProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
int CALLBACK ImageManagerProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
int CALLBACK FontManagerProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
int CALLBACK CollectManagerProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
int CALLBACK CollectionProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
int CALLBACK VideoOptionsProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
int CALLBACK AudioOptionsProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
int CALLBACK EditorOptionsProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
int CALLBACK ProtectTableProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
int CALLBACK UnlockTableProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
int CALLBACK KeysProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
int CALLBACK TableInfoProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
int CALLBACK SecurityOptionsProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

int CALLBACK AboutProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

///<summary>
///VPinball Constructor
///<para>Init</para>
///</summary>
VPinball::VPinball()
	{
//	DLL_API void DLL_CALLCONV FreeImage_Initialise(BOOL load_local_plugins_only FI_DEFAULT(FALSE)); //add FreeImage support BDS

	m_cref = 0;				//inits Reference Count for IUnknown Interface. Every com Object must 
							//implement this and StdMethods QueryInterface, AddRef and Release
	m_open_minimized = 0;

	NumPlays = 0;			// for Statistics in Registry (?)

	m_pcv = NULL;			// no currently active code window
	}

///<summary>
///VPinball Destructor
///<para>deletes clipboard</para>
///<para>Releases Resources for Script editor</para>
///</summary>
VPinball::~VPinball()
	{
//	DLL_API void DLL_CALLCONV FreeImage_DeInitialise(); //remove FreeImage support BDS
	SetClipboard(NULL);

	Scintilla_ReleaseResources();
	}

///<summary>
///Store path of exe (without the exe's filename) in Class Variable
///<para>Stores path as char[MAX_PATH] in m_szMyPath (8 bit ansi)</para>
///<para>Stores path as WCHAR[MAX_PATH] in m_wzMyPath (16 bit Unicode)</para>
///</summary>
void VPinball::GetMyPath()
	{
	char szPath[MAX_PATH];

	GetModuleFileName(NULL, szPath, MAX_PATH);

	char *szEnd = szPath + lstrlen(szPath);

	// search for first backslash
	while (szEnd > szPath)
		{
		if (*szEnd == '\\')
			{
			break;
			}
		szEnd--;
		}

	// truncate the filename
	*(szEnd+1) = '\0'; // Get rid of exe name

	// store 2x
	lstrcpy(m_szMyPath, szPath);
	MultiByteToWideChar(CP_ACP, 0, szPath, -1, m_wzMyPath, MAX_PATH);
	}

// Class Variables
bool VPinball::m_open_minimized;
int VPinball::NumPlays;

///<summary>
///Sets m_open_minimized to 1
///Called by CLI Option minimized 
///</summary>
void VPinball::SetOpenMinimized()
{
	m_open_minimized = 1;
}

///<summary>
///Main Init function
///<para>sets some init-values to variables</para>
///<para>registers scintilla Editor</para>
///<para>creates and shows Main Window and all Toolbars</para>
///<para>creates toolbars and statusbar</para>
///<para>Sets this class as MDI Callback</para>
///<para>creates APC VBA Host</para>
///<para>initializes Direct Sound and Direct Draw</para>
///<para>Calibrates Timer</para>
///<para>Inits Debug-Window</para>
///</summary>
void VPinball::Init() 
	{

	HRESULT hr;
	m_NextTableID = 1;

	m_bWinHelp = false;											//unused or neccessary for VBA?
	m_lcidVBA = 1033;											//local ID: english - used when creating VBA APC Host

	m_ptableActive = NULL;
	m_hwndSideBar = NULL;										//Handle for left Sidebar
	m_hwndWork = NULL;											//Handle for Workarea

	m_workerthread = NULL;										//Workerthread - only for hanging scripts and autosave - will be created later

	//m_pistgClipboard = NULL;

	GetMyPath();												//Store path of vpinball.exe in m_szMyPath and m_wzMyPath

	RegisterClasses();											//TODO - brief description of what happens in the function
	Scintilla_RegisterClasses(g_hinst);							//registering Scintilla with current Application instance number.

	char szName[256];
	LoadString(g_hinstres, IDS_PROJNAME, szName, 256);
																// loading String "Visual Pinball" from Exe properties

	const int screenwidth = GetSystemMetrics(SM_CXSCREEN);		// width of primary monitor
	const int screenheight = GetSystemMetrics(SM_CYSCREEN);		// height of primary monitor

	const int x = (screenwidth - MAIN_WINDOW_WIDTH)/2;
	const int y = (screenheight - MAIN_WINDOW_HEIGHT)/2;
	const int width = MAIN_WINDOW_WIDTH;
	const int height = MAIN_WINDOW_HEIGHT;

    LPTSTR lpCmdLine = GetCommandLine();						//this line necessary for _ATL_MIN_CRT

	if( strstr( lpCmdLine, "minimized" ) )
	{
		SetOpenMinimized();
	}

	m_hwnd = ::CreateWindowEx(WS_EX_OVERLAPPEDWINDOW,"VPinball",szName,
			( m_open_minimized ? WS_MINIMIZE : 0 ) | WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_SIZEBOX | WS_MINIMIZEBOX | WS_MAXIMIZEBOX,
			x,y,width,height,NULL,NULL,g_hinst,0);				// get handle to and create main Window

	// See if we have previous window size information
		{
		int left,top,right,bottom;
		BOOL fMaximized;

		const HRESULT hrleft = GetRegInt("Editor", "WindowLeft", &left);
		const HRESULT hrtop = GetRegInt("Editor", "WindowTop", &top);
		const HRESULT hrright = GetRegInt("Editor", "WindowRight", &right);
		const HRESULT hrbottom = GetRegInt("Editor", "WindowBottom", &bottom);

		const HRESULT hrmax = GetRegInt("Editor", "WindowMaximized", &fMaximized);

		if (hrleft == S_OK && hrtop == S_OK && hrright == S_OK && hrbottom == S_OK)
			{
			WINDOWPLACEMENT winpl;
			winpl.length = sizeof(winpl);

			GetWindowPlacement(m_hwnd, &winpl);

			winpl.rcNormalPosition.left = left;
			winpl.rcNormalPosition.top = top;
			winpl.rcNormalPosition.right = right;
			winpl.rcNormalPosition.bottom = bottom;

			if( m_open_minimized )
			{
				winpl.showCmd |= SW_MINIMIZE;
			}
			else if (hrmax == S_OK && fMaximized)
			{
				winpl.showCmd |= SW_MAXIMIZE;
			}

			SetWindowPlacement(m_hwnd, &winpl);
			}
		}

	ShowWindow(m_hwnd, SW_SHOW);

	SetWindowLong(m_hwnd, GWL_USERDATA, (long)this);	// set this class (vpinball) as callback for MDI Child / has to be confirmed
														// can be a problem for 64 bit compatibility.
														// maybe use SetWindowLongPtr instead

	CreateSideBar();									// Create Sidebar

	CreateMDIClient();									// Create MDI Child

	int foo[4] = {120,240,400,600};

    m_hwndStatusBar = CreateStatusWindow(WS_CHILD | WS_VISIBLE,
                                       "",
                                       m_hwnd,
                                       1);				// Create Status Line at the bottom

	SendMessage(m_hwndStatusBar, SB_SETPARTS, 4, (long)foo);	// Initilise Status bar with 4 empty cells

	m_sb.Init(m_hwnd);									// initilize smartbrowser (Property bar on the right) - see propbrowser.cpp

	SendMessage(m_hwnd, WM_SIZE, 0, 0);					// Make our window relay itself out

	InitTools();										// eventually show smartbrowser

	InitRegValues();									// get default values from registry

	InitVBA();											//Create APC VBA host

	m_pds.InitDirectSound(m_hwnd);						// init Direct Sound (in pinsound.cpp)
	hr = m_pdd.InitDD();								// init direct draw (in pinimage.cpp)

	// check if Direct draw could be initilized
	if (hr != S_OK)
		{
		SendMessage(m_hwnd, WM_CLOSE, 0, 0);
		}

	//m_music.Foo();
	m_fBackglassView = fFalse;							// we are viewing Pinfield and not the backglass at first

	SetEnableToolbar();
	SetEnableMenuItems();
	UpdateRecentFileList(NULL);							// update the recent loaded file list

    wintimer_init();									// calibrate the timer routines
	slintf_init();										// initialize debug console (can be popupped by the following command)
														// slintf_popup_console();
														// see slintf.cpp
	}

void VPinball::EnsureWorkerThread()
	{
	if (!m_workerthread)
		{
		g_hWorkerStarted = CreateEvent(NULL,TRUE,FALSE,NULL);
		m_workerthread = CreateThread(NULL, 0, VPWorkerThreadStart, 0, 0, &m_workerthreadid);
		if (WaitForSingleObject(g_hWorkerStarted, 5000) == WAIT_TIMEOUT)
			{
			}
		//SetThreadPriority(m_workerthread, THREAD_PRIORITY_LOWEST);
		}
	}

HANDLE VPinball::PostWorkToWorkerThread(int workid, LPARAM lParam)
	{
	EnsureWorkerThread();

	HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	PostThreadMessage(m_workerthreadid, workid, (WPARAM)hEvent, lParam);

	return hEvent;
	}

void VPinball::SetAutoSaveMinutes(int minutes)
	{
	if (minutes <= 0)
		{
		m_autosaveTime = -1;
		}
	else
		{
		m_autosaveTime = minutes * 60 * 1000; // convert to milliseconds
		}
	}

void VPinball::InitTools()
	{
	// was the properties panel open last time we used VP?
	int		state;
	const HRESULT hr = GetRegInt("Editor", "PropertiesVisible", (int *)&state);
	if ((hr == S_OK) && (state == 1))
		{
		// if so then re-open it
		ParseCommand(ID_EDIT_PROPERTIES, m_hwnd, 1);//display
		}

	m_ToolCur = IDC_SELECT;

	SendMessage(m_hwndToolbarMain,TB_CHECKBUTTON,IDC_SELECT,MAKELONG(TRUE,0));
	}

void VPinball::InitRegValues()
	{
	HRESULT hr;


	hr = GetRegInt("Player", "HardwareRender", &m_fHardwareAccel);
	if (hr != S_OK)
		{
		g_pvp->m_pdd.m_fHardwareAccel = 0; // default value
		}

	hr = GetRegInt("Player", "DeadZone", &DeadZ);
	if (hr != S_OK)
		{
		DeadZ = 0; // default value
		}
	SetRegValue("Player", "DeadZone", REG_DWORD, &DeadZ, 4);

	hr = GetRegInt("Player", "AlternateRender", &m_fAlternateRender);
	if (hr != S_OK)
		{
		g_pvp->m_pdd.m_fAlternateRender=0; // default value
		}

	hr = GetRegInt("Editor", "ShowDragPoints", &m_fAlwaysDrawDragPoints);
	if (hr != S_OK)
		{
		m_fAlwaysDrawDragPoints = fFalse; // default value
		}

	hr = GetRegInt("Editor", "DrawLightCenters", &m_fAlwaysDrawLightCenters);
	if (hr != S_OK)
		{
		m_fAlwaysDrawLightCenters = fFalse; // default value
		}

	BOOL fAutoSave;
	hr = GetRegInt("Editor", "AutoSaveOn", &fAutoSave);
	if (hr != S_OK)
		{
		fAutoSave = fTrue; // default value
		}

	if (fAutoSave)
		{
		int autosavetime;
		hr = GetRegInt("Editor", "AutoSaveTime", &autosavetime);
		if (hr != S_OK)
			{
			autosavetime = AUTOSAVE_DEFAULT_TIME;
			}
		SetAutoSaveMinutes(autosavetime);
		}
	else
		{
		m_autosaveTime = -1;
		}

	hr = GetRegInt("Player", "SecurityLevel", &m_securitylevel);
	if (hr != S_OK)
		{
		m_securitylevel = DEFAULT_SECURITY_LEVEL; // The default
		}

	if (m_securitylevel < eSecurityNone || eSecurityNoControls > 4)
		{
		m_securitylevel = eSecurityNoControls;
		}

	// get the list of the last n loaded tables
	for (int i=0; i<LAST_OPENED_TABLE_COUNT; i++)
		{
		char szRegName[MAX_PATH];
		sprintf_s(szRegName, "TableFileName%d", i);
		m_szRecentTableList[i][0] = 0x00;
		hr = GetRegString("RecentDir",szRegName, m_szRecentTableList[i], MAX_PATH);
		}
	}

void VPinball::RegisterClasses()
	{
	WNDCLASSEX wcex;
	memset(&wcex, 0, sizeof(WNDCLASSEX));
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_DBLCLKS;//CS_NOCLOSE | CS_OWNDC;
	wcex.lpfnWndProc = (WNDPROC) VPWndProc;
	wcex.hInstance = g_hinst;
	wcex.hIcon = LoadIcon(g_hinst, MAKEINTRESOURCE(IDI_APPICON));
	wcex.lpszClassName = "VPinball";
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.lpszMenuName = MAKEINTRESOURCE(IDR_APPMENU);
	RegisterClassEx(&wcex);

	// Register dummy child class
	wcex.lpszClassName = "VPStaticChild";
	wcex.lpfnWndProc = VPSideBarWndProc;
	wcex.lpszMenuName = NULL;
	wcex.hIcon = NULL;
	wcex.hbrBackground = HBRUSH(COLOR_BTNFACE+1);
	RegisterClassEx(&wcex);

	wcex.hbrBackground = HBRUSH(COLOR_WINDOW+1);
	wcex.lpszClassName = "VPStaticWorkChild";
	RegisterClassEx(&wcex);
	}

void VPinball::CreateSideBar()
	{
	RECT rc;
	GetWindowRect(m_hwnd, &rc);

	m_hwndSideBar = ::CreateWindowEx(/*WS_EX_WINDOWEDGE*/0,"VPStaticChild","",WS_VISIBLE | WS_CHILD | WS_BORDER,
			0,0,TOOLBAR_WIDTH + SCROLL_WIDTH,rc.bottom - rc.top,m_hwnd,NULL,g_hinst,0);

	m_hwndSideBarScroll = ::CreateWindowEx(0,"VPStaticChild","",WS_VISIBLE | WS_CHILD | WS_VSCROLL,
			0,48*(TBCOUNTMAIN/2),TOOLBAR_WIDTH + SCROLL_WIDTH,rc.bottom - rc.top,m_hwndSideBar,NULL,g_hinst,0);

	m_hwndToolbarMain = CreateToolbar((TBBUTTON *)g_tbbuttonMain, TBCOUNTMAIN, m_hwndSideBar);
	m_hwndToolbarPalette = CreateToolbar((TBBUTTON *)g_tbbuttonPalette, TBCOUNTPALETTE, m_hwndSideBarScroll);

	palettescroll = 0;
	}

HWND VPinball::CreateToolbar(TBBUTTON *p_tbbutton, int count, HWND hwndParent)
	{
	HWND hwnd = CreateToolbarEx(hwndParent,
		WS_CHILD | WS_VISIBLE | TBSTYLE_FLAT | TBSTYLE_WRAPABLE,
		1, count, g_hinst, IDB_TB_SELECT, p_tbbutton, count, 24, 24, 24, 24,
		sizeof(TBBUTTON));

	SendMessage(hwnd, TB_SETEXTENDEDSTYLE, 0, TBSTYLE_EX_DRAWDDARROWS);

#define MAXRESLEN 128

	char szBuf[MAXRESLEN];

	for (int i=0;i<count;i++)
		{
		LoadString(g_hinstres, p_tbbutton[i].dwData, szBuf, MAXRESLEN-1);
		szBuf[lstrlen(szBuf) + 1] = 0;  //Double-null terminate.
		/*const int foo =*/ SendMessage(hwnd, TB_ADDSTRING, 0, (LPARAM) szBuf);
		}

#ifdef IMSPANISH
	SendMessage(m_hwnd, TB_SETBUTTONWIDTH, 0,
		(LPARAM)(DWORD)MAKELONG(50,48));
#elif defined(IMGERMAN)
	SendMessage(m_hwnd, TB_SETBUTTONWIDTH, 0,
		(LPARAM)(DWORD)MAKELONG(50,48));
#else
	SendMessage(hwnd, TB_SETBUTTONWIDTH, 0,
		(LPARAM)(DWORD)MAKELONG(50,50));
#endif

	for (int i=0;i<count;i++)
		{
		TBBUTTONINFO tbbi;
		tbbi.cbSize = sizeof(tbbi);
		tbbi.dwMask = TBIF_SIZE | TBIF_COMMAND | TBIF_STATE | TBIF_STYLE;
		/*int foo =*/ SendMessage(hwnd, TB_GETBUTTONINFO, p_tbbutton[i].idCommand, (LPARAM)&tbbi);
		if (tbbi.fsStyle & TBSTYLE_DROPDOWN)
			{
			tbbi.cx = 48;
			}
		/*foo =*/ SendMessage(hwnd, TB_SETBUTTONINFO, p_tbbutton[i].idCommand, (LPARAM)&tbbi);
		}

	SendMessage(hwnd, TB_AUTOSIZE, 0, 0);

	return hwnd;
	}

void VPinball::CreateMDIClient()
	{
	RECT rc;
	GetWindowRect(m_hwnd, &rc);

	CLIENTCREATESTRUCT ccs;
	ccs.hWindowMenu = GetSubMenu(GetMenu(g_pvp->m_hwnd), WINDOWMENU); // Window menu is third from the left
	ccs.idFirstChild = 4000;//129;

	m_hwndWork = ::CreateWindowEx(0,"MDICLIENT","",WS_VISIBLE | WS_CHILD | WS_CLIPCHILDREN | WS_VSCROLL | WS_HSCROLL,
			TOOLBAR_WIDTH + SCROLL_WIDTH,0,rc.right - rc.left - (TOOLBAR_WIDTH+SCROLL_WIDTH),rc.bottom - rc.top,m_hwnd,NULL,g_hinst,&ccs);
	}

void VPinball::SetClipboard(Vector<IStream> *pvstm)
	{
	for (int i=0;i<m_vstmclipboard.Size();i++)
		{
		m_vstmclipboard.ElementAt(i)->Release();
		}
	m_vstmclipboard.RemoveAllElements();

	if (pvstm)
		{
		for (int i=0;i<pvstm->Size();i++)
			{
			m_vstmclipboard.AddElement(pvstm->ElementAt(i));
			}
		}
	}

void VPinball::SetCursorCur(HINSTANCE hInstance, LPCTSTR lpCursorName)
	{
	HCURSOR hcursor = LoadCursor(hInstance, lpCursorName);
	SetCursor(hcursor);
	}

void VPinball::SetActionCur(char *szaction)
	{
	SendMessage(m_hwndStatusBar, SB_SETTEXT, 3 | 0, (long)szaction);
	}

void VPinball::SetPosCur(float x, float y)
	{
	char szT[256];

#ifndef PERFTEST
	sprintf_s(szT, "%.4f, %.4f", x, y);
#endif
	SendMessage(m_hwndStatusBar, SB_SETTEXT, 0 | 0, (long)szT);
	}

void VPinball::SetObjectPosCur(float x, float y)
	{
	char szT[256];

#ifndef PERFTEST
	sprintf_s(szT, "%.4f, %.4f", x, y);
#endif
	SendMessage(m_hwndStatusBar, SB_SETTEXT, 1 | 0, (long)szT);
	}

void VPinball::ClearObjectPosCur()
	{
	SendMessage(m_hwndStatusBar, SB_SETTEXT, 1 | 0, (long)"");
	}

void VPinball::SetPropSel(Vector<ISelect> *pvsel)
	{
	m_sb.CreateFromDispatch(m_hwnd, pvsel);
	}

void VPinball::ParseCommand(int code, HWND hwnd, int notify)
	{
	CComObject<PinTable> *ptCur;

	switch (code)
		{
		case IDM_NEW:
			{
			CComObject<PinTable> *pt;
			CComObject<PinTable>::CreateInstance(&pt);
			pt->AddRef();
			pt->Init(this);
			//pt = new PinTable(this);
			m_vtable.AddElement(pt);
			SetEnableToolbar();
			SetEnableMenuItems();
			}
			break;

		case ID_DELETE:
			{
			ptCur = GetActiveTable();
			if (ptCur)
				{
				if (ptCur->CheckPermissions(DISABLE_CUTCOPYPASTE))
					{
					ShowPermissionError();
					}
				else
					{
					ptCur->OnDelete();
					}
				}
			}
			break;

		case ID_TABLE_PLAY:
//				MessageBox(g_pvp->m_hwnd, FreeImage_GetCopyrightMessage(), "Error", MB_OK | MB_ICONEXCLAMATION); //ADDED BDS
			DoPlay();
			break;

		case ID_SCRIPT_SHOWIDE:
		case ID_EDIT_SCRIPT:
			{
			ptCur = GetActiveTable();
			if (ptCur)
				{
				if (ptCur->CheckPermissions(DISABLE_SCRIPT_EDITING))
					{
					ShowPermissionError();
					}
				else
					{
					ptCur->m_pcv->SetVisible(fTrue);
					}
				}
			}
			break;

		case ID_EDIT_PROPERTIES:
			{
#ifdef VBAPropBrowser
			IApcPropertiesWindow *papw;
			g_pvp->ApcHost->get_PropertiesWindow(&papw);
			short fVisible;
			papw->get_Visible(&fVisible);
			papw->put_Visible(!fVisible);
#endif
			BOOL fShow = fFalse;

			if(!g_pplayer) fShow = m_sb.GetVisible(); // Get the current display state 

			switch(notify)//rlc set, redisplay, toggle
			{
			case 0: 
			case 1: fShow = !fShow; //set
				break;
			case 2:  //re-display 
				break;
			
			default: fShow = !fShow; //toggle
				break;
			}
			
			SetRegValue("Editor", "PropertiesVisible", REG_DWORD, &fShow, 4);

			// Set toolbar button to the correct state
			TBBUTTONINFO tbinfo;
			tbinfo.cbSize = sizeof(TBBUTTONINFO);
			tbinfo.dwMask  = TBIF_STATE;
			//+++ Modified by Chris ID_EDIT_PROPERTIES is now on m_hwndToolbarMain
			SendMessage(m_hwndToolbarMain,TB_GETBUTTONINFO,ID_EDIT_PROPERTIES,(long)&tbinfo);

			if(!g_pplayer)
			{
				if(notify == 2) fShow = (tbinfo.fsState & TBSTATE_CHECKED) != 0;

				if (fShow ^ ((tbinfo.fsState & TBSTATE_CHECKED) != 0))
				{
					tbinfo.fsState ^= TBSTATE_CHECKED;
				}
				SendMessage(m_hwndToolbarMain,TB_SETBUTTONINFO,ID_EDIT_PROPERTIES,(long)&tbinfo);
			}
			

			// Set menu item to the correct state
			HMENU hmenu = GetMenu(m_hwnd);
			const int count = GetMenuItemCount(hmenu);
			HMENU hmenuEdit;
			if (count > 7)
				{
				// MDI has added it's crap
				hmenuEdit = GetSubMenu(hmenu, 2);
				}
			else
				{
				hmenuEdit = GetSubMenu(hmenu, 1);
				}
			/*const DWORD foo =*/ CheckMenuItem(hmenuEdit, ID_EDIT_PROPERTIES, MF_BYCOMMAND | (fShow ? MF_CHECKED : MF_UNCHECKED));

			m_sb.SetVisible(fShow);

			SendMessage(m_hwnd, WM_SIZE, 0, 0);
			if (fShow)
				{
				ptCur = GetActiveTable();
				if (ptCur)
					{
					if (!ptCur->CheckPermissions(DISABLE_TABLEVIEW))
						{
						m_sb.CreateFromDispatch(m_hwnd, &ptCur->m_vmultisel);
						}
					}
				}
			}
			break;

		case ID_EDIT_BACKGLASSVIEW:
			{
			const BOOL fShow = !m_fBackglassView;

			TBBUTTONINFO tbinfo;
			tbinfo.cbSize = sizeof(TBBUTTONINFO);
			tbinfo.dwMask  = TBIF_STATE;
			//+++ Modified by Chris ID_EDIT_PROPERTIES is now on m_hwndToolbarMain
			SendMessage(m_hwndToolbarMain,TB_GETBUTTONINFO,ID_EDIT_BACKGLASSVIEW,(long)&tbinfo);

			// Set toolbar button to the correct state
			if (fShow ^ ((tbinfo.fsState & TBSTATE_CHECKED) != 0))
				{
				tbinfo.fsState ^= TBSTATE_CHECKED;
				}

			SendMessage(m_hwndToolbarMain,TB_SETBUTTONINFO,ID_EDIT_BACKGLASSVIEW,(long)&tbinfo);

			// Set menu item to the correct state
			HMENU hmenu = GetMenu(m_hwnd);
			const int count = GetMenuItemCount(hmenu);
			HMENU hmenuEdit;
			if (count > 7)
				{
				// MDI has added it's crap
				hmenuEdit = GetSubMenu(hmenu, 2);
				}
			else
				{
				hmenuEdit = GetSubMenu(hmenu, 1);
				}
			/*const DWORD foo =*/ CheckMenuItem(hmenuEdit, ID_EDIT_BACKGLASSVIEW, MF_BYCOMMAND | (fShow ? MF_CHECKED : MF_UNCHECKED));

			m_fBackglassView = fShow;

			for (int i=0;i<m_vtable.Size();i++)
				{
				PinTable * const ptT = m_vtable.ElementAt(i);
				ptT->SetDefaultView();
				ptT->SetDirtyDraw();
				ptT->SetMyScrollInfo();
				}

			ptCur = GetActiveTable();
			if (ptCur)
				{
				if (!ptCur->CheckPermissions(DISABLE_TABLEVIEW))
					{
					// Set selection to something in the new view (unless hiding table elements)
					ptCur->AddMultiSel((ISelect *)ptCur, fFalse, fTrue);
					}
				}

			SetEnableToolbar();
			}
			break;

		case IDC_SELECT:
		case IDC_MAGNIFY:
		case IDC_WALL:
		case IDC_FLIPPER:
		case IDC_TIMER:
		case IDC_PLUNGER:
		case IDC_TEXTBOX:
		case IDC_BUMPER:
		case IDC_TRIGGER:
		case IDC_LIGHT:
		case IDC_KICKER:
		case IDC_TARGET:
		case IDC_DECAL:
		case IDC_GATE:
		case IDC_SPINNER:
		case IDC_RAMP:
        case IDC_DISPREEL:
		case IDC_LIGHTSEQ:
		case IDC_COMCONTROL:
			{
			switch (code)
				{
				case IDC_SELECT:
				case IDC_MAGNIFY:
					SendMessage(m_hwndToolbarPalette,TB_CHECKBUTTON,m_ToolCur,MAKELONG(FALSE,0));
					SendMessage(m_hwndToolbarMain,TB_CHECKBUTTON,code,MAKELONG(TRUE,0));
					break;

				default:
					SendMessage(m_hwndToolbarMain,TB_CHECKBUTTON,m_ToolCur,MAKELONG(FALSE,0));
					SendMessage(m_hwndToolbarPalette,TB_CHECKBUTTON,code,MAKELONG(TRUE,0));
					break;
				}

			m_ToolCur = code;// - IDC_SELECT;

			//HCURSOR hcur = LoadCursor(g_hinst, MAKEINTRESOURCE(IDC_MAGNIFY));
			//SetCursor(hcur);

			if (notify == 1) // accelerator - mouse can be over table already
				{
				POINT pt;
				GetCursorPos(&pt);
				SetCursorPos(pt.x, pt.y);
				}
			}
			break;

		case IDM_SAVE:
			{
			ptCur = GetActiveTable();
			if (ptCur)
				{
				if (ptCur->CheckPermissions(DISABLE_TABLE_SAVE))
					{
					ShowPermissionError();
					}
				else
					{
					HRESULT hr = ptCur->TableSave();
					if (hr == S_OK)
						{
							UpdateRecentFileList(ptCur->m_szFileName);
						}
					}
				}
			}
			break;

		case IDM_SAVEAS:
			{
			ptCur = GetActiveTable();
			if (ptCur)
				{
				if (ptCur->CheckPermissions(DISABLE_TABLE_SAVE))
					{
					ShowPermissionError();
					}
				else
					{
					HRESULT hr = ptCur->SaveAs();
					if (hr == S_OK)
						{
							UpdateRecentFileList(ptCur->m_szFileName);
						}
					}
				}
			}
			break;

		case IDM_SAVEASPROTECTED:
			{
			ptCur = GetActiveTable();
			if (ptCur)
				{
				if ( (ptCur->CheckPermissions(DISABLE_TABLE_SAVE)) ||
					 (ptCur->CheckPermissions(DISABLE_TABLE_SAVEPROT)) )
					{
					ShowPermissionError();
					}
				else
					{
					int foo = DialogBoxParam(g_hinstres, MAKEINTRESOURCE(IDD_PROTECT_DIALOG),
						  m_hwnd, ProtectTableProc, 0);
					// if the dialog returned ok then perform a normal save as
					if (foo)
						{
						foo = ptCur->SaveAs();
						if (foo == S_OK)
							{
							// if the save was succesfull then the permissions take effect immediatly
							SetEnableToolbar();			// disable any tool bars
							SetEnableMenuItems();		// disable any menu bars
							ptCur->SetDirtyDraw();		// redraw the screen (incase hiding elements)
							UpdateRecentFileList(ptCur->m_szFileName);
							}
						else
							{
							// if the save failed, then reset the permissions
							ptCur->ResetProtectionBlock();
							}
						}
					}
				}
			}
			break;

		case IDM_UNLOCKPROTECTED:
			{
			ptCur = GetActiveTable();
			if (ptCur)
				{
				int foo = DialogBoxParam(g_hinstres, MAKEINTRESOURCE(IDD_UNLOCK_DIALOG),
					  m_hwnd, UnlockTableProc, 0);
				// if the dialog returned ok then table is unlocked
				if (foo)
					{
					// re-enable any disabled menu items
					SetEnableToolbar();			// disable any tool bars
					SetEnableMenuItems();		// disable any menu bars
					ptCur->SetDirtyDraw();		// redraw the screen (incase hiding elements)
					}
				}
			}
			break;
//<<<

//>>> added by Chris 
		case RECENT_FIRST_MENU_IDM:
		case RECENT_FIRST_MENU_IDM+1:
		case RECENT_FIRST_MENU_IDM+2:
		case RECENT_FIRST_MENU_IDM+3:
		case RECENT_FIRST_MENU_IDM+4:
		case RECENT_FIRST_MENU_IDM+5:
		case RECENT_FIRST_MENU_IDM+6:
		case RECENT_FIRST_MENU_IDM+7:
			{
			char	szFileName[MAX_PATH];
			// get the index into the recent list menu
			const int Index = code - RECENT_FIRST_MENU_IDM;
			// copy it into a temporary string so it can be correctly processed
			memcpy(szFileName, m_szRecentTableList[Index], sizeof(szFileName));
			LoadFileName(szFileName);
			}
			break;
//<<<

		case IDM_OPEN:
			LoadFile();
			break;

		case IDM_CLOSE:
			{
			ptCur = GetActiveTable();
			if (ptCur)
				{
				CloseTable(ptCur);
				}
			}
			break;

		case IDC_COPY:
			{
			ptCur = GetActiveTable();
			if (ptCur)
				{
				if (ptCur->CheckPermissions(DISABLE_CUTCOPYPASTE))
					{
					ShowPermissionError();
					}
				else
					{
					ptCur->Copy();
					}
				}
			}
			break;

		case IDC_PASTE:
			{
			ptCur = GetActiveTable();
			if (ptCur)
				{
				ptCur->Paste(fFalse, 0, 0);
				}
			}
			break;

		case IDC_PASTEAT:
			{
			ptCur = GetActiveTable();
			if (ptCur)
				{
				POINT ptCursor;
				GetCursorPos(&ptCursor);
				ScreenToClient(ptCur->m_hwnd, &ptCursor);
				ptCur->Paste(fTrue, ptCursor.x, ptCursor.y);
				}
			}
			break;

		case ID_EDIT_UNDO:
			{
			ptCur = GetActiveTable();
			if (ptCur)
				{
				ptCur->Undo();
				}
			}
			break;

		case ID_FILE_EXPORT_BLUEPRINT:
			{
			ptCur = GetActiveTable();
			if (ptCur)
				{
				if (ptCur->CheckPermissions(DISABLE_TABLE_SAVE))
					{
					ShowPermissionError();
					}
				else
					{
					ptCur->ExportBlueprint();
					}
				}
			}
			break;

		case ID_FILE_EXIT:
			PostMessage(m_hwnd, WM_CLOSE, 0, 0);
			break;

		case ID_EDIT_AUDIOOPTIONS:
			{
			/*const DWORD foo =*/ DialogBoxParam(g_hinstres, MAKEINTRESOURCE(IDD_AUDIO_OPTIONS),
				  m_hwnd, AudioOptionsProc, 0);
			}
			break;


		case ID_EDIT_EDITOROPTIONS:
			{
			/*const DWORD foo =*/ DialogBoxParam(g_hinstres, MAKEINTRESOURCE(IDD_EDITOR_OPTIONS),
				  m_hwnd, EditorOptionsProc, 0);
			// refresh editor options from the registry
			InitRegValues();
			// force a screen refresh (it an active table is loaded)
			ptCur = GetActiveTable();
			if (ptCur)
				{
				ptCur->SetDirtyDraw();
				}
			}
			break;

		case ID_EDIT_VIDEOOPTIONS:
			{
			/*const DWORD foo =*/ DialogBoxParam(g_hinstres, MAKEINTRESOURCE(IDD_VIDEO_OPTIONS),
				  m_hwnd, VideoOptionsProc, 0);
			}
			break;

		case ID_PREFERENCES_SECURITYOPTIONS:
			{
			/*const DWORD foo =*/ DialogBoxParam(g_hinstres, MAKEINTRESOURCE(IDD_SECURITY_OPTIONS),
				  m_hwnd, SecurityOptionsProc, 0);

			// refresh editor options from the registry
			InitRegValues();
			}
			break;

		case ID_EDIT_KEYS:
			{
			/*const DWORD foo =*/ DialogBoxParam(g_hinstres, MAKEINTRESOURCE(IDD_KEYS),m_hwnd, KeysProc, 0);
			}
			break;

		case ID_TABLE_TABLEINFO:
			{
			ptCur = GetActiveTable();
			if (ptCur)
				{
#ifdef VBA
				ApcHost->BeginModalDialog();
#endif
				/*const DWORD foo =*/ DialogBoxParam(g_hinstres, MAKEINTRESOURCE(IDD_TABLEINFO),
					  m_hwnd, TableInfoProc, (long)ptCur);
#ifdef VBA
				ApcHost->EndModalDialog();
#endif
				}
			}
			break;

		case ID_TABLE_SOUNDMANAGER:
			{
			ptCur = GetActiveTable();
			if (ptCur)
				{
				if (ptCur->CheckPermissions(DISABLE_OPEN_MANAGERS))
					{
					ShowPermissionError();
					}
				else
					{
#ifdef VBA
					ApcHost->BeginModalDialog();
#endif
					/*const DWORD foo =*/ DialogBoxParam(g_hinstres, MAKEINTRESOURCE(IDD_SOUNDDIALOG),m_hwnd, SoundManagerProc, (long)ptCur);
#ifdef VBA
					ApcHost->EndModalDialog();
#endif					
					}
				}
			}
			break;

		case ID_TABLE_IMAGEMANAGER:
			{
			ptCur = GetActiveTable();
			if (ptCur)
				{
				if (ptCur->CheckPermissions(DISABLE_OPEN_MANAGERS))
					{
					ShowPermissionError();
					}
				else
					{
#ifdef VBA
					ApcHost->BeginModalDialog();
#endif
					/*const DWORD foo =*/ DialogBoxParam(g_hinstres, MAKEINTRESOURCE(IDD_IMAGEDIALOG), m_hwnd, ImageManagerProc, (long)ptCur);
					m_sb.PopulateDropdowns(); // May need to update list of images
					m_sb.RefreshProperties();
#ifdef VBA
					ApcHost->EndModalDialog();
#endif
					}
				}
			}
			break;

		case ID_TABLE_FONTMANAGER:
			{
			ptCur = GetActiveTable();
			if (ptCur)
				{
				if (ptCur->CheckPermissions(DISABLE_OPEN_MANAGERS))
					{
					ShowPermissionError();
					}
				else
					{
#ifdef VBA
					ApcHost->BeginModalDialog();
#endif
					/*const DWORD foo =*/ DialogBoxParam(g_hinstres, MAKEINTRESOURCE(IDD_FONTDIALOG),
						  m_hwnd, FontManagerProc, (long)ptCur);
#ifdef VBA
					ApcHost->EndModalDialog();
#endif
					}
				}
			}
			break;

		case ID_TABLE_COLLECTIONMANAGER:
			{
			ptCur = GetActiveTable();
			if (ptCur)
				{
				if (ptCur->CheckPermissions(DISABLE_OPEN_MANAGERS))
					{
					ShowPermissionError();
					}
				else
					{
#ifdef VBA
					ApcHost->BeginModalDialog();
#endif
					/*const DWORD foo =*/ DialogBoxParam(g_hinstres, MAKEINTRESOURCE(IDD_COLLECTDIALOG),
						  m_hwnd, CollectManagerProc, (long)ptCur);

					m_sb.PopulateDropdowns(); // May need to update list of collections
					m_sb.RefreshProperties();
#ifdef VBA
					ApcHost->EndModalDialog();
#endif
					}
				}
			}
			break;

		case ID_HELP_ABOUT:
			{
#ifdef VBA
			ApcHost->BeginModalDialog();
#endif
			/*const DWORD foo =*/ DialogBoxParam(g_hinstres, MAKEINTRESOURCE(IDD_ABOUT),
				  m_hwnd, AboutProc, 0);
#ifdef VBA
			ApcHost->EndModalDialog();
#endif
			}
			break;
		}
	}

const int rgToolEnable[22][2] = {
	IDC_WALL, 1,
	IDC_GATE, 1,
	IDC_RAMP, 1,
	IDC_FLIPPER, 1,
	IDC_PLUNGER, 1,
	IDC_BUMPER, 1,
	IDC_SPINNER, 1,
	IDC_TIMER, 3,
	IDC_TRIGGER, 1,
	IDC_LIGHT, 3,
	IDC_KICKER, 1,
	IDC_TARGET, 1,
	IDC_DECAL, 3,
	IDC_TEXTBOX, 2,
	IDC_DISPREEL, 2,
	IDC_LIGHTSEQ, 3,
	IDC_COMCONTROL, 2,
	//>>> these five are handled separately (see below code)
	ID_EDIT_SCRIPT, 0,
	ID_TABLE_PLAY, 0,
	ID_EDIT_BACKGLASSVIEW, 0,
	IDC_MAGNIFY, 0,
	IDC_SELECT, 0,
	};

void VPinball::SetEnablePalette()
	{
	PinTable * const ptCur = GetActiveTable();

	bool fTableActive = (ptCur != NULL) && !g_pplayer;

	// if we can't view the table elements then make the table as not active as that
	// ensure all menu and toolbars are disabled.
	if (ptCur)
		{
		if (ptCur->CheckPermissions(DISABLE_TABLEVIEW))
			{
			fTableActive = false;
			}
		}

	const int state = (m_fBackglassView ? 2 : 1);

	for (int i=0;i<TBCOUNTPALETTE;i++)		//<<< changed by Chris from 0->14 to 0->15
		{
		const int id = rgToolEnable[i][0];
		const int enablecode = rgToolEnable[i][1];

		const BOOL fEnable = fTableActive && ((enablecode & state) != 0);

		// Set toolbar state
		TBBUTTONINFO tbinfo;
		tbinfo.cbSize = sizeof(TBBUTTONINFO);
		tbinfo.dwMask  = TBIF_STATE;
		SendMessage(m_hwndToolbarPalette,TB_GETBUTTONINFO,id,(long)&tbinfo);

		if (fEnable ^ ((tbinfo.fsState & TBSTATE_ENABLED) != 0))
			{
			tbinfo.fsState ^= TBSTATE_ENABLED;
			}

		SendMessage(m_hwndToolbarPalette,TB_SETBUTTONINFO,id,(long)&tbinfo);

		// Set menu item
		HMENU hmenu = GetMenu(m_hwnd);
		const int count = GetMenuItemCount(hmenu);
		HMENU hmenuEdit;
		// Get the insert menu
		if (count > 7)
			{
			// MDI has added it's crap
			hmenuEdit = GetSubMenu(hmenu, 3);
			}
		else
			{
			hmenuEdit = GetSubMenu(hmenu, 2);
			}
		/*const DWORD foo =*/ EnableMenuItem(hmenuEdit, id, MF_BYCOMMAND | (fEnable ? MF_ENABLED : MF_GRAYED));
		}
	}


void VPinball::SetEnableToolbar()
	{
	PinTable *ptCur = GetActiveTable();

	const bool fTableActive = (ptCur != NULL) && !g_pplayer;

	//int state = (m_fBackglassView ? 2 : 1);

	for (int i=TBCOUNTPALETTE;i<(TBCOUNTPALETTE+5);i++)
		{
		const int id = rgToolEnable[i][0];
		BOOL fEnable = fTableActive;

		if (ptCur)
			{
			if ((id == ID_EDIT_SCRIPT) && (ptCur->CheckPermissions(DISABLE_SCRIPT_EDITING)))
				{
					fEnable = fFalse;
				}
			}

		// Set toolbar state
		TBBUTTONINFO tbinfo;
		tbinfo.cbSize = sizeof(TBBUTTONINFO);
		tbinfo.dwMask  = TBIF_STATE;
		SendMessage(m_hwndToolbarMain,TB_GETBUTTONINFO,id,(long)&tbinfo);

		if (fEnable ^ ((tbinfo.fsState & TBSTATE_ENABLED) != 0))
			{
			tbinfo.fsState ^= TBSTATE_ENABLED;
			}

		SendMessage(m_hwndToolbarMain,TB_SETBUTTONINFO,id,(long)&tbinfo);
		}

	SetEnablePalette();

	ParseCommand(ID_EDIT_PROPERTIES, m_hwnd, 2);//redisplay 
	}

void VPinball::DoPlay()
	{
	NumVideoBytes = 0;
	CComObject<PinTable> * const ptCur = GetActiveTable();
	if (ptCur)
		{
		ptCur->Play();
		}
	}

void VPinball::LoadFile()
	{
	char szFileName[1024];
	char szInitialDir[1024];
	szFileName[0] = '\0';

	OPENFILENAME ofn;
	memset(&ofn, 0, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hInstance = g_hinst;
	ofn.hwndOwner = g_pvp->m_hwnd;
	// TEXT
	//ofn.lpstrFilter = "Visual Pinball Tables (*.vpt)\0*.vpt\0";
	ofn.lpstrFilter = "Visual Pinball Tables (*.vpt)\0*.vpt\0"; //test.rlc
	ofn.lpstrFile = szFileName;
	ofn.nMaxFile = _MAX_PATH;
	ofn.lpstrDefExt = "vpt";
	ofn.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;

	const HRESULT hr = GetRegString("RecentDir","LoadDir", szInitialDir, 1024);
	if (hr == S_OK)
		{
		ofn.lpstrInitialDir = szInitialDir;
		}
	else
		{
		char szFoo[MAX_PATH];
		lstrcpy(szFoo, m_szMyPath);
		lstrcat(szFoo, "Tables");
		ofn.lpstrInitialDir = szFoo;
		}

#ifdef VBA
	ApcHost->BeginModalDialog();
#endif
	const int ret = GetOpenFileName(&ofn);
#ifdef VBA
	ApcHost->EndModalDialog();
#endif

	if(ret == 0)
		{
		return;
		}
//>>> moved by chris down into loadfilename()
//	strcpy(szInitialDir, szFileName);
//	szInitialDir[ofn.nFileOffset] = 0;
//	hr = SetRegValue("RecentDir","LoadDir", REG_SZ, szInitialDir, strlen(szInitialDir));
//<<<
	LoadFileName(szFileName);
	}

void VPinball::LoadFileName(char *szFileName)
	{
	CComObject<PinTable> *ppt;
	CComObject<PinTable>::CreateInstance(&ppt);
	ppt->AddRef();
	//ppt->Init(this);
	m_vtable.AddElement(ppt);
	const HRESULT hr = ppt->LoadGameFromFilename(szFileName);

	if (!SUCCEEDED(hr))
		{
		if (hr == E_ACCESSDENIED)
			{
			LocalString ls(IDS_CORRUPTFILE);
			ShowError(ls.m_szbuffer);
			}

		m_vtable.RemoveElement(ppt);
		ppt->Release();
		}
	else
		{
		char szLoadDir[MAX_PATH];

		ppt->InitPostLoad(this);
		TitleFromFilename(szFileName, ppt->m_szTitle);
		ppt->SetCaption(ppt->m_szTitle);

		// get the load path from the filename
		PathFromFilename(szFileName, szLoadDir);
		SetRegValue("RecentDir","LoadDir", REG_SZ, szLoadDir, strlen(szLoadDir));

		// make sure the load directory is the active directory
		DWORD err = SetCurrentDirectory(szLoadDir);
		if (err == 0)
			{
			err = GetLastError();
			}

		UpdateRecentFileList(szFileName);
		}

	SetEnableToolbar();
	SetEnableMenuItems();					//>>> added by Chris
	}

CComObject<PinTable> *VPinball::GetActiveTable()
	{
	HWND hwndT = (HWND)SendMessage(m_hwndWork, WM_MDIGETACTIVE, 0, 0);

	if (hwndT)
		{
		CComObject<PinTable> *pt = (CComObject<PinTable> *)GetWindowLong(hwndT, GWL_USERDATA);
		return pt;
		}
	else
		{
		return NULL;
		}
	}

BOOL VPinball::FCanClose()
	{
	BOOL fCanClose;

	while(m_vtable.Size())
		{
		fCanClose = CloseTable(m_vtable.ElementAt(0));

		if (!fCanClose)
			{
			return fFalse;
			}
		}

	return fTrue;

}


BOOL VPinball::CloseTable(PinTable *ppt)
{
	if ((ppt->FDirty()) && (!ppt->CheckPermissions(DISABLE_TABLE_SAVE)) )
		{
		LocalString ls1(IDS_SAVE_CHANGES1);
		LocalString ls2(IDS_SAVE_CHANGES2);
		char *szText = new char[lstrlen(ls1.m_szbuffer) + lstrlen(ls2.m_szbuffer) + lstrlen(ppt->m_szTitle) + 1];
		lstrcpy(szText, ls1.m_szbuffer/*"Do you want to save the changes you made to '"*/);
		lstrcat(szText, ppt->m_szTitle);
		lstrcat(szText, ls2.m_szbuffer);
		// TEXT
		const int result = MessageBox(m_hwnd, szText, "Visual Pinball", MB_YESNOCANCEL | MB_DEFBUTTON3 | MB_ICONWARNING);
		delete szText;
		if (result == IDCANCEL)
			{
			return fFalse;
			}
		if (result == IDYES)
			{
			if (ppt->TableSave() != S_OK)
				{
				LocalString ls3(IDS_SAVEERROR);
				MessageBox(m_hwnd, ls3.m_szbuffer, "Visual Pinball", MB_ICONERROR);
				return fFalse;
				}
			}
		}

	/*const BOOL fSafe =*/ ppt->FVerifySaveToClose();

	if (m_sb.GetBaseISel() && (ppt == m_sb.GetBaseISel()->GetPTable()))
		{
		SetPropSel(NULL);
		}

#ifdef VBA
	ppt->CloseVBA();
	ppt->m_pStg->Release();
#endif
	m_vtable.RemoveElement(ppt);
	ppt->m_pcv->CleanUpScriptEngine();
	ppt->Release();

	SetEnableToolbar();
	SetEnableMenuItems();

	return fTrue;
}


BOOL VPinball::FDefaultCheckBlit()
{
	const DWORD ver = GetVersion();

	const bool fCheckBlt = ((ver & 0x80000000) != 0); // check blt status on Win9x

	return fCheckBlt;
}


void VPinball::InitVBA()
	{
	m_fDebugging = false;
#ifdef VBA
	m_ptinfoCls       = NULL;
	m_ptinfoInt       = NULL;

	HRESULT hr = S_OK;

	BSTR bstrLicKey = SysAllocString(wszEvalLicKey);
	hr = ApcHost.Create(m_hwnd, L"Visual Pinball", GetDispatch(), bstrLicKey, m_lcidVBA);
	SysFreeString(bstrLicKey);

	if (hr != S_OK)
		{
		ShowError("Could not create VBA.");
		}

	AddMiniBitmaps();
#endif
	}

HRESULT VPinball::AddMiniBitmaps()
{
#ifdef VBA
	IApcMiniBitmaps* pBmps;
	HRESULT hr;
	if(SUCCEEDED(hr = ApcHost->APC_GET(MiniBitmaps)(&pBmps)))
	{
		PICTDESC pd;
		IPictureDisp* pDisp;
		pd.cbSizeofstruct = sizeof(PICTDESC);
		pd.picType = PICTYPE_BITMAP;
		pd.bmp.hpal = NULL;

		int i=0;
		while (g_rgminibitmap[i].resid != -1)
			{
			pd.bmp.hbitmap = LoadBitmap(g_hinst, MAKEINTRESOURCE(g_rgminibitmap[i].resid));

			if(SUCCEEDED(hr = OleCreatePictureIndirect(&pd, IID_IPictureDisp, TRUE, (LPVOID*)&pDisp)))
				{
					hr = pBmps->APC_RAW(Add)(g_rgminibitmap[i].wzGUID, pDisp, (::OLE_COLOR)RGB(255,0,255), NULL);

					pDisp->Release();
					pDisp = NULL;
				}
			i++;
			}

		pBmps->Release();
	}

	return hr;
#else
	return S_OK;
#endif
}

//>>> added by Chris
void VPinball::ShowPermissionError()
	{
	LocalString ls(IDS_PERMISSION_ERROR);
	MessageBox(m_hwnd, ls.m_szbuffer, "Visual Pinball", MB_ICONWARNING);
	}

void VPinball::SetEnableMenuItems()
	{
	CComObject<PinTable> * const ptCur = GetActiveTable();

	// Set menu item to the correct state
	HMENU hmenu = GetMenu(m_hwnd);
	// is there a valid table??
	if (ptCur)
		{
		EnableMenuItem(hmenu, IDM_CLOSE, MF_BYCOMMAND | MF_ENABLED);
		EnableMenuItem(hmenu, ID_EDIT_UNDO, MF_BYCOMMAND | MF_ENABLED);
		EnableMenuItem(hmenu, ID_EDIT_BACKGLASSVIEW, MF_BYCOMMAND | MF_ENABLED);
		EnableMenuItem(hmenu, ID_TABLE_PLAY, MF_BYCOMMAND | MF_ENABLED);
		EnableMenuItem(hmenu, IDC_MAGNIFY, MF_BYCOMMAND | MF_ENABLED);
		EnableMenuItem(hmenu, ID_TABLE_TABLEINFO, MF_BYCOMMAND | MF_ENABLED);

		// enable/disable save options
		UINT flags;
		if (ptCur->CheckPermissions(DISABLE_TABLE_SAVE))
			{
			flags = MF_BYCOMMAND | MF_GRAYED;
			}
		else
			{
			flags = MF_BYCOMMAND | MF_ENABLED;
			}
		EnableMenuItem(hmenu, IDM_SAVE, flags);
		EnableMenuItem(hmenu, IDM_SAVEAS, flags);
		EnableMenuItem(hmenu, IDM_SAVEASPROTECTED, flags);
		EnableMenuItem(hmenu, ID_FILE_EXPORT_BLUEPRINT, flags);

		// if we can do a normal save but not a protected save then disable 'save as protected'
		// (if we cant do any saves it is already disabled)
		if ( (!ptCur->CheckPermissions(DISABLE_TABLE_SAVE))	&&
			 (ptCur->CheckPermissions(DISABLE_TABLE_SAVEPROT)) )
			{
			EnableMenuItem(hmenu, IDM_SAVEASPROTECTED, MF_BYCOMMAND | MF_GRAYED);
			}

		// enable/disable script option
		if (ptCur->CheckPermissions(DISABLE_SCRIPT_EDITING))
			{
			flags = MF_BYCOMMAND | MF_GRAYED;
			}
		else
			{
			flags = MF_BYCOMMAND | MF_ENABLED;
			}
		EnableMenuItem(hmenu, ID_EDIT_SCRIPT, flags);

		// enable/disable managers options
		if (ptCur->CheckPermissions(DISABLE_OPEN_MANAGERS))
			{
			flags = MF_BYCOMMAND | MF_GRAYED;
			}
		else
			{
			flags = MF_BYCOMMAND | MF_ENABLED;
			}
		EnableMenuItem(hmenu, ID_TABLE_SOUNDMANAGER, flags);
		EnableMenuItem(hmenu, ID_TABLE_IMAGEMANAGER, flags);
		//EnableMenuItem(hmenu, ID_TABLE_FONTMANAGER, flags);
		EnableMenuItem(hmenu, ID_TABLE_COLLECTIONMANAGER, flags);

		// enable/disable editing options
		if (ptCur->CheckPermissions(DISABLE_CUTCOPYPASTE))
			{
			flags = MF_BYCOMMAND | MF_GRAYED;
			}
		else
			{
			flags = MF_BYCOMMAND | MF_ENABLED;
			}
		EnableMenuItem(hmenu, IDC_COPY, flags);
		EnableMenuItem(hmenu, IDC_PASTE, flags);
		EnableMenuItem(hmenu, IDC_PASTEAT, flags);
		EnableMenuItem(hmenu, ID_DELETE, flags);

		// if the table is protected enable the unlock and disable 'save as protected' always
		if (ptCur->IsTableProtected())
			{
			EnableMenuItem(hmenu, IDM_SAVEASPROTECTED, MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem(hmenu, IDM_UNLOCKPROTECTED, MF_BYCOMMAND | MF_ENABLED);
			}
		else
			{
			// table is not protected, disable the unlock feature
			EnableMenuItem(hmenu, IDM_UNLOCKPROTECTED, MF_BYCOMMAND | MF_GRAYED);
			}
		}
	else
		{
			/* no valid table, disable a few items */
			EnableMenuItem(hmenu, IDM_CLOSE, MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem(hmenu, IDM_SAVE, MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem(hmenu, IDM_SAVEAS, MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem(hmenu, IDM_SAVEASPROTECTED, MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem(hmenu, IDM_UNLOCKPROTECTED, MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem(hmenu, ID_FILE_EXPORT_BLUEPRINT, MF_BYCOMMAND | MF_GRAYED);

			EnableMenuItem(hmenu, ID_EDIT_UNDO, MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem(hmenu, IDC_COPY, MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem(hmenu, IDC_PASTE, MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem(hmenu, IDC_PASTEAT, MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem(hmenu, ID_DELETE, MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem(hmenu, ID_EDIT_SCRIPT, MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem(hmenu, ID_EDIT_BACKGLASSVIEW, MF_BYCOMMAND | MF_GRAYED);

			EnableMenuItem(hmenu, ID_TABLE_PLAY, MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem(hmenu, ID_TABLE_SOUNDMANAGER, MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem(hmenu, ID_TABLE_IMAGEMANAGER, MF_BYCOMMAND | MF_GRAYED);
			//EnableMenuItem(hmenu, ID_TABLE_FONTMANAGER, MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem(hmenu, ID_TABLE_COLLECTIONMANAGER, MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem(hmenu, ID_TABLE_TABLEINFO, MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem(hmenu, IDC_MAGNIFY, MF_BYCOMMAND | MF_GRAYED);
		}
	}
//<<<

//>>> added by Chris
void VPinball::UpdateRecentFileList(char *szfilename)
	{
	// if the loaded file name is not null then add it to the top of the list
	if (szfilename != NULL)
		{
		// does this file name aready exist in the list?
		bool bFound = false;
		int i;
		for (i=0; i<LAST_OPENED_TABLE_COUNT; i++)
			{
			if (strcmp(m_szRecentTableList[i], szfilename) == 0)
				{
				// yes it does
				bFound = true;
				break;
				}
			}

		// if the entry is already in the list then copy all the items above it down one position
		const int index = (bFound) ? i-1 :
					// else copy the entire list down
					(LAST_OPENED_TABLE_COUNT-2);

		// copy the entrys in the list down one position
		for (i=index; i>=0; i--)
			{
			memcpy(m_szRecentTableList[i+1], m_szRecentTableList[i], MAX_PATH);
			}
		// copy the current file into the first position
		memcpy(m_szRecentTableList[0], szfilename, MAX_PATH);

		// write the list of the last n loaded tables to the registry
		for (i=0; i<LAST_OPENED_TABLE_COUNT; i++)
			{
			char szRegName[MAX_PATH];

			// if this entry is empty then all the rest are empty
			if (m_szRecentTableList[i][0] == 0x00) break;
			// write entry to the registry
			sprintf_s(szRegName, "TableFileName%d", i);
			SetRegValue("RecentDir", szRegName, REG_SZ, m_szRecentTableList[i], strlen(m_szRecentTableList[i]));
			}
		} // (szfilename != NULL)

   	// update the file menu to contain the last n recent loaded files
	// must be at least 1 recent file in the list
	if (m_szRecentTableList[0][0] != 0x00)
		{
		MENUITEMINFO menuInfo;

	   	// update the file menu to contain the last n recent loaded files
	   	HMENU hmenu = GetMenu(m_hwnd);
	   	int count = GetMenuItemCount(hmenu);
	   	HMENU hmenuFile;
	   	if (count > 7)
	   		{
	   		// MDI has added it's stuff (table icon for first menu item)
	   		hmenuFile = GetSubMenu(hmenu, 1);
	   		}
	   	else
	   		{
	   		hmenuFile = GetSubMenu(hmenu, 0);
	   		}

	   	// delete all the recent file IDM's from this menu
	   	for (int i=RECENT_FIRST_MENU_IDM; i<=RECENT_LAST_MENU_IDM; i++)
	   		{
	   		DeleteMenu(hmenuFile, i, MF_BYCOMMAND);
	   		}

	   	// get the number of entrys in the file menu
	   	count = GetMenuItemCount(hmenuFile);
		// inset the items before the EXIT menu (assuming it is the last entry)
		count--;

		// set up the menu info block
		memset(&menuInfo, 0x00, sizeof(menuInfo));
		menuInfo.cbSize = sizeof(menuInfo);
		menuInfo.fMask = MIIM_ID | MIIM_TYPE;
		menuInfo.fType = MFT_STRING;

	   	// add in the list of recently accessed files
		for (int i=0; i<LAST_OPENED_TABLE_COUNT; i++)
			{
			// if this entry is empty then all the rest are empty
			if (m_szRecentTableList[i][0] == 0x00) break;

			// set the IDM of this menu item
			menuInfo.wID = RECENT_FIRST_MENU_IDM + i;
			menuInfo.dwTypeData = m_szRecentTableList[i];
			menuInfo.cch = strlen(m_szRecentTableList[i]);

			InsertMenuItem(hmenuFile, count, TRUE, &menuInfo);
			count++;
			}

		// add a separator onto the end
		menuInfo.fType = MFT_SEPARATOR;
		menuInfo.wID = RECENT_LAST_MENU_IDM;
		InsertMenuItem(hmenuFile, count, TRUE, &menuInfo);

		// update the menu bar
		DrawMenuBar(m_hwnd);
		}
	}
//<<<

HRESULT VPinball::ApcHost_OnTranslateMessage(MSG* pmsg, BOOL* pfConsumed)
	{
	*pfConsumed = FALSE;
	
	if (!g_pplayer)
		{
		for (int i=0;i<m_sb.m_vhwndDialog.Size();i++)
			{
			if (IsDialogMessage(m_sb.m_vhwndDialog.ElementAt(i), pmsg))
				{
				*pfConsumed = TRUE;
				}
			}
		if (m_pcv && m_pcv->m_hwndMain)
			{
			//if (pmsg->hwnd == m_pcv->m_hwndMain)
				{
				int fTranslated = fFalse;

				if ((pmsg->hwnd == m_pcv->m_hwndMain) || IsChild(m_pcv->m_hwndMain, pmsg->hwnd))
					{
					fTranslated = TranslateAccelerator(m_pcv->m_hwndMain, m_pcv->m_haccel, pmsg);
					}

				if (fTranslated)
					{
					*pfConsumed = TRUE;
					}
				else
					{
					if (IsDialogMessage(m_pcv->m_hwndMain, pmsg))
						{
						*pfConsumed = TRUE;
						}
					}
				}
			}

		if (m_pcv && m_pcv->m_hwndFind)
			{
				{
				if (IsDialogMessage(m_pcv->m_hwndFind, pmsg))
					{
					*pfConsumed = TRUE;
					}
				}
			}

		if (!(*pfConsumed))
			{
			const int fTranslated = TranslateAccelerator(m_hwnd, g_haccel, pmsg);

			if (fTranslated != 0)
				{
				*pfConsumed = TRUE;
				}
			}

		if (!(*pfConsumed))
			{
			/*const int fTranslated =*/ TranslateMessage(pmsg);
			}
		}
	else
		{
		if (g_pplayer->m_fDebugMode)
			{
			if (IsDialogMessage(g_pplayer->m_hwndDebugger, pmsg))
				{
				*pfConsumed = TRUE;
				}
			}
		}

	return NOERROR;
	}

HRESULT VPinball::MainMsgLoop()
{
#ifdef VBA
	return ApcHost.MessageLoop();
#else
	BOOL fConsumed;
	MSG msg;

	for (;;)
		{
		if (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE))
			{
			if (msg.message == WM_QUIT)
				break;

			fConsumed = fFalse;

			ApcHost_OnTranslateMessage(&msg, &fConsumed);

			if (!fConsumed)
				{
				DispatchMessage(&msg);
				}
			}
		else
			{
			if (g_pplayer && !g_pplayer->m_fPause)
				{
				ApcHost_OnIdle(&fConsumed);
				}
			else
				{
				WaitMessage();
				}
			}
		}

	return S_OK;
#endif
}

#ifdef VBA
HRESULT VPinball::ApcHost_OnIdle(BOOL* pfContinue)
	{
#ifdef VBA
	ApcHost.OnIdle(pfContinue);
#endif

	if (g_pplayer && !m_fDebugging)
		{
		g_pplayer->Render();
		*pfContinue = TRUE;
		}

	return S_OK;
	}
#endif

HRESULT VPinball::ApcHost_OnIdle(BOOL* pfContinue)
	{
#ifdef VBA
	ApcHost.OnIdle(pfContinue);
#endif

	if (!m_fDebugging)
		{
		g_pplayer->Render();
		*pfContinue = TRUE;
		}

	return S_OK;
	}

HRESULT VPinball::ApcHost_BeforePause()
	{
	m_fDebugging = true;
	if (g_pplayer)
		{
		g_pplayer->m_fNoTimeCorrect = fTrue; // So ball doesn't jump ahead next frame
		}
	return S_OK;
	}

HRESULT VPinball::ApcHost_AfterPause()
	{
	m_fDebugging = false;
	return S_OK;
	}

HRESULT VPinball::ShowIDE()
	{
#ifdef VBA
	IApcIde *pIDE = NULL;
	VARIANT_BOOL vbVisible = VARIANT_TRUE;
	HRESULT hr;
	if(SUCCEEDED(hr = ApcHost->APC_GET(Ide)(&pIDE)))
		{
		hr = pIDE->APC_PUT(Visible)(vbVisible);
		pIDE->Release();
		}
	return hr;
#else
	return S_OK;
#endif
	}

STDMETHODIMP VPinball::QueryInterface(REFIID iid, void **ppvObjOut)
	{
	if (!ppvObjOut)
		return E_INVALIDARG;

	*ppvObjOut = NULL;

	if (*ppvObjOut)
		{
		this->AddRef();
		return S_OK;
		}

	return E_NOINTERFACE;
}


HRESULT VPinball::GetTypeLibInfo
(
  HINSTANCE    *phinstOut,
  const GUID  **pplibidOut,
  SHORT        *pwMajLib,
  SHORT        *pwMinLib,
  const CLSID **ppclsidOut,
  const IID   **ppiidOut,
  ITypeLib   ***ppptlOut
)
{
  return S_OK;
}


STDMETHODIMP_(ULONG) VPinball::AddRef ()
{
	ASSERT(m_cref, "bad m_cref");
	return ++m_cref;
}


STDMETHODIMP_(ULONG) VPinball::Release ()
{
	ASSERT(m_cref, "bad m_cref");
	m_cref--;

	if (!m_cref)
	{
		delete this;
		return 0;
    }

	return m_cref;
}


LRESULT CALLBACK VPWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
	switch (uMsg)
		{
		case WM_CLOSE:
			{
			BOOL fCanClose;
#ifdef VBA
			g_pvp->ApcHost.WmClose(fCanClose);
#else
			if (g_pplayer)
				{
				SendMessage(g_pplayer->m_hwnd, WM_CLOSE, 0, 0);
				}

			fCanClose = fTrue;
#endif
			if (fCanClose)
				{
				fCanClose = g_pvp->FCanClose();
				if (fCanClose)
					{

					WINDOWPLACEMENT winpl;
					winpl.length = sizeof(winpl);

					if (GetWindowPlacement(hwnd, &winpl))
						{
						SetRegValue("Editor", "WindowLeft", REG_DWORD, &winpl.rcNormalPosition.left, 4);
						SetRegValue("Editor", "WindowTop", REG_DWORD, &winpl.rcNormalPosition.top, 4);
						SetRegValue("Editor", "WindowRight", REG_DWORD, &winpl.rcNormalPosition.right, 4);
						SetRegValue("Editor", "WindowBottom", REG_DWORD, &winpl.rcNormalPosition.bottom, 4);

						BOOL fMaximized = IsZoomed(hwnd);
						SetRegValue("Editor", "WindowMaximized", REG_DWORD, &fMaximized, 4);
						}

					DestroyWindow(hwnd);
					}
				}

			return 0;
			}
			break;

		case WM_DESTROY:
			PostMessage(hwnd, WM_QUIT, 0, 0);
			break;

		case WM_PAINT:
			{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd,&ps);
			RECT rc;
			GetClientRect(hwnd, &rc);
			SelectObject(hdc, GetStockObject(WHITE_BRUSH));
			PatBlt(hdc, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, PATCOPY);
			EndPaint(hwnd,&ps);
			}
			break;

		case WM_SIZE:
			if (g_pvp && g_pvp->m_hwndSideBar)
				{
				RECT rc;
				GetClientRect(hwnd, &rc);

				SendMessage(g_pvp->m_hwndStatusBar, WM_SIZE, wParam, lParam);

				RECT rcStatus;
				GetWindowRect(g_pvp->m_hwndStatusBar, &rcStatus);
				const int statheight = rcStatus.bottom - rcStatus.top;

				const int scrollwindowtop = 48*(TBCOUNTMAIN/2);
				const int scrollwindowheight = rc.bottom - rc.top - statheight - scrollwindowtop;
				SetWindowPos(g_pvp->m_hwndSideBarScroll,NULL,
					0, scrollwindowtop, TOOLBAR_WIDTH + SCROLL_WIDTH, scrollwindowheight, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER);

				HWND hwndSB = g_pvp->m_sb.GetHWnd();
				int SBwidth = g_pvp->m_sb.m_maxdialogwidth;

				if (hwndSB && IsWindowVisible(hwndSB))
					{
					SetWindowPos(hwndSB,NULL,
					rc.right - rc.left - SBwidth, 0, SBwidth, rc.bottom - rc.top - statheight, SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOZORDER);
					}
				else
					{
					SBwidth = 0;
					}

				// Set scroll info for the palette scrollbar
				SCROLLINFO si;
				const long padding = SendMessage(g_pvp->m_hwndToolbarPalette, TB_GETPADDING, 0, 0);
				const long buttonsize = SendMessage(g_pvp->m_hwndToolbarPalette, TB_GETBUTTONSIZE, 0, 0);
				const int vertpadding = HIWORD(padding);
				const int vertbutsize = HIWORD(buttonsize);

				si.cbSize = sizeof(si);
				si.fMask = SIF_ALL;
				si.nMin = 0;
				si.nMax = ((vertbutsize+vertpadding) * (TBCOUNTPALETTE/2)) + 4; // Add 4 padding
				si.nPage = scrollwindowheight;
				si.nPos = g_pvp->palettescroll;

				SetScrollInfo(g_pvp->m_hwndSideBarScroll, SB_VERT, &si, TRUE);

				// check if we have any blank space at the bottom and fill it in by moving the scrollbar up
				if ((int)(si.nPos + si.nPage) > si.nMax)
					{
					g_pvp->palettescroll = si.nMax - si.nPage;
					if (g_pvp->palettescroll < 0)
						{
						g_pvp->palettescroll = 0;
						}

					SetScrollPos(hwnd, SB_VERT, g_pvp->palettescroll, TRUE);

					SetWindowPos(g_pvp->m_hwndToolbarPalette,NULL,
						0, -g_pvp->palettescroll, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER);
					}

				int sidebarwidth = TOOLBAR_WIDTH;
				if (scrollwindowheight < si.nMax)
					{
					sidebarwidth += SCROLL_WIDTH;
					}

				SetWindowPos(g_pvp->m_hwndSideBar,NULL,
					0, 0, sidebarwidth, rc.bottom - rc.top - statheight, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER);

				SetWindowPos(g_pvp->m_hwndWork,NULL,
					sidebarwidth, 0, rc.right - rc.left - (sidebarwidth) - SBwidth, rc.bottom - rc.top - statheight, SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOZORDER);
				return 0;
				}
			break;

		case WM_COMMAND:
			g_pvp->ParseCommand(LOWORD(wParam), (HWND)lParam, HIWORD(wParam));
			break;

#ifdef VBA
		case WM_ACTIVATE:
			g_pvp->ApcHost.WmActivate(wParam);
			break;

		case WM_ENABLE:
			g_pvp->ApcHost.WmEnable(wParam);
			break;
#endif
		}

	return DefFrameProc(hwnd, g_pvp->m_hwndWork, uMsg, wParam, lParam);
}


LRESULT CALLBACK VPSideBarWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_NOTIFY:
		{
			LPNMHDR pnmhdr = (LPNMHDR)lParam;
			switch (pnmhdr->code)
			{
				case TBN_DROPDOWN:
				{
					PinTable * const pt = g_pvp->GetActiveTable();

					if (pt)
					{
						if (pt->CheckPermissions(DISABLE_TABLEVIEW))
							{
							break;
							}

						HMENU hmenu = CreatePopupMenu();

						HWND hwndList = CreateWindowEx(0, "ListBox", "", WS_CHILD | LBS_SORT, 0, 0, 10, 10, hwnd, NULL, g_hinst, 0);

						int menucount=0;
						for (int i=0;i<pt->m_vedit.Size();i++)
						{
							IEditable * const piedit = pt->m_vedit.ElementAt(i);
							// check scriptable - decals don't have scripts and therefore don't have names
							if (piedit->GetScriptable() && piedit->m_fBackglass == g_pvp->m_fBackglassView)
							{
								char szT[64]; // Names can only be 32 characters (plus terminator)
								WideCharToMultiByte(CP_ACP, 0, pt->m_vedit.ElementAt(i)->GetScriptable()->m_wzName, -1, szT, 64, NULL, NULL);

								const int index = SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)szT);
								SendMessage(hwndList, LB_SETITEMDATA, index, i+1);// menu can't have an item with id 0, so bump everything up one
							}
						}

						for (int i=0;i<pt->m_vcollection.Size();i++)
						{
							char szT[64]; // Names can only be 32 characters (plus terminator)
							WideCharToMultiByte(CP_ACP, 0, pt->m_vcollection.ElementAt(i)->m_wzName, -1, szT, 64, NULL, NULL);

							const int index = SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)szT);
							SendMessage(hwndList, LB_SETITEMDATA, index, i | 0x80000000);
						}

						const int listcount = SendMessage(hwndList, LB_GETCOUNT, 0, 0);

						// Take the items from our sorted list and put them into the menu
						for (int i=0;i<listcount;i++)
						{
							char szT[64];
							int flags = MF_STRING;

							if ((menucount%30 == 0) && (menucount != 0))
							{
								flags |= MF_MENUBARBREAK;
							}

							SendMessage(hwndList, LB_GETTEXT, i, (LPARAM)szT);
							const int data = SendMessage(hwndList, LB_GETITEMDATA, i, 0);

							AppendMenu(hmenu, flags, data, szT);
							menucount++;
						}

						DestroyWindow(hwndList);

						POINT mousept;
						GetCursorPos(&mousept);

						const int ksshift = GetKeyState(VK_SHIFT);
						const BOOL fAdd = ((ksshift & 0x80000000) != 0);

						const int icmd = TrackPopupMenuEx(hmenu, TPM_RETURNCMD | 16384/*TPM_NOANIMATION*/,
							mousept.x, mousept.y, hwnd, NULL);

						if (icmd != 0)
						{
							if (icmd & 0x80000000) // collection
							{
								Collection * const pcol = pt->m_vcollection.ElementAt(icmd & 0x7fffffff);
								for (int i=0;i<pcol->m_visel.Size();i++)
								{
									pt->AddMultiSel(pcol->m_visel.ElementAt(i), i == 0 ? fAdd : TRUE, TRUE);
								}
							}
							else
							{
								pt->AddMultiSel(pt->m_vedit.ElementAt(icmd-1)->GetISelect(), fAdd, TRUE);
							}
						}
						DestroyMenu(hmenu);
					}
				}
				break;
				}
			}
			break;

		case WM_COMMAND:
			g_pvp->ParseCommand(wParam, (HWND)lParam, (int)hwnd);
			break;

		case WM_VSCROLL:
			{
			SCROLLINFO si;
			si.cbSize = sizeof(SCROLLINFO);
			si.fMask = SIF_ALL;
			GetScrollInfo(hwnd, SB_VERT, &si);
			switch (LOWORD(wParam))
				{
				case SB_LINEUP:
					g_pvp->palettescroll -= si.nPage/10;
					break;
				case SB_LINEDOWN:
					g_pvp->palettescroll += si.nPage/10;
					break;
				case SB_PAGEUP:
					g_pvp->palettescroll -= si.nPage/2;
					break;
				case SB_PAGEDOWN:
					g_pvp->palettescroll += si.nPage/2;
					break;
				case SB_THUMBTRACK:
					{
					const int delta = (int)(g_pvp->palettescroll - si.nPos);
					g_pvp->palettescroll = ((short)HIWORD(wParam) + delta);
					}
					break;
				}

			SetScrollPos(hwnd, SB_VERT, g_pvp->palettescroll, TRUE);

			SetWindowPos(g_pvp->m_hwndToolbarPalette,NULL,
				0, -g_pvp->palettescroll, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER);
			}
			break;
		}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}


STDMETHODIMP VPinball::PlaySound(BSTR bstr)
{
	if (g_pplayer) g_pplayer->m_ptable->PlaySound(bstr, 0, 1);

	return S_OK;
}


STDMETHODIMP VPinball::FireKnocker(int Count)
{
	if (g_pplayer) g_pplayer->m_ptable->FireKnocker(Count);

	return S_OK;

}

STDMETHODIMP VPinball::QuitPlayer(int CloseType)
{
	if (g_pplayer)g_pplayer->m_ptable->QuitPlayer(CloseType);

	return S_OK;
}

STDMETHODIMP VPinball::StartShake()
{
	if (g_pplayer) g_pplayer->m_ptable->StartShake();

	return S_OK;
}


STDMETHODIMP VPinball::StopShake()
{
	if (g_pplayer) g_pplayer->m_ptable->StopShake();

	return S_OK;
}


void VPinball::Quit()
{
	if( g_pplayer ) g_pplayer->m_fCloseDown = fTrue;
	else
	{
		PostMessage(m_hwnd, WM_CLOSE, 0, 0);
	}
}


int CALLBACK SoundManagerProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CCO(PinTable) *pt = (CCO(PinTable) *)GetWindowLong(hwndDlg, GWL_USERDATA);

	switch (uMsg)
		{
		case WM_INITDIALOG:
			{
			SetWindowLong(hwndDlg, GWL_USERDATA, lParam);
			LVCOLUMN lvcol;

			lvcol.mask = LVCF_TEXT | LVCF_WIDTH;
			LocalString ls(IDS_NAME);
			lvcol.pszText = ls.m_szbuffer;// = "Name";
			lvcol.cx = 100;
			ListView_InsertColumn(GetDlgItem(hwndDlg, IDC_SOUNDLIST), 0, &lvcol);

			LocalString ls2(IDS_IMPORTPATH);
			lvcol.pszText = ls2.m_szbuffer; // = "Import Path";
			lvcol.cx = 200;
			ListView_InsertColumn(GetDlgItem(hwndDlg, IDC_SOUNDLIST), 1, &lvcol);

			pt = (CCO(PinTable) *)GetWindowLong(hwndDlg, GWL_USERDATA);

			pt->ListSounds(GetDlgItem(hwndDlg, IDC_SOUNDLIST));

			return TRUE;
			}
			break;

		case WM_CLOSE:
			EndDialog(hwndDlg, FALSE);
			break;

		case WM_NOTIFY:
			{
			LPNMHDR pnmhdr = (LPNMHDR)lParam;
			switch (pnmhdr->code)
				{
				case LVN_ENDLABELEDIT:
					{
					NMLVDISPINFO *pinfo = (NMLVDISPINFO *)lParam;
					if (pinfo->item.pszText == NULL || pinfo->item.pszText[0] == '\0')
						{
						return FALSE;
						}
					ListView_SetItemText(GetDlgItem(hwndDlg, IDC_SOUNDLIST), pinfo->item.iItem, 0, pinfo->item.pszText);
					LVITEM lvitem;
					lvitem.mask = LVIF_PARAM;
					lvitem.iItem = pinfo->item.iItem;
					lvitem.iSubItem = 0;
					ListView_GetItem(GetDlgItem(hwndDlg, IDC_SOUNDLIST), &lvitem);
					PinSound *pps = (PinSound *)lvitem.lParam;
					lstrcpy(pps->m_szName, pinfo->item.pszText);
					lstrcpy(pps->m_szInternalName, pinfo->item.pszText);
					CharLowerBuff(pps->m_szInternalName, lstrlen(pps->m_szInternalName));
					pt->SetNonUndoableDirty(eSaveDirty);
					return TRUE;
					}
					break;

				case LVN_ITEMCHANGED:
					{
					const int count = ListView_GetSelectedCount(GetDlgItem(hwndDlg, IDC_SOUNDLIST));
					const int fEnable = !(count > 1);
					EnableWindow(GetDlgItem(hwndDlg, IDC_REIMPORTFROM), fEnable);
					EnableWindow(GetDlgItem(hwndDlg, IDC_RENAME), fEnable);
					EnableWindow(GetDlgItem(hwndDlg, IDC_PLAY), fEnable);
					}
				break;
				}
			}
			break;

		case WM_COMMAND:
			switch (HIWORD(wParam))
				{
				case BN_CLICKED:
					switch (LOWORD(wParam))
						{
						case IDOK:
							EndDialog(hwndDlg, TRUE);
							break;

						case IDCANCEL:
							EndDialog(hwndDlg, FALSE);
							break;

						case IDC_IMPORT:
							{
							char szFileName[10240];
							char szInitialDir[10240];
							char szT[MAX_PATH];

							szFileName[0] = '\0';

							OPENFILENAME ofn;
							memset(&ofn, 0, sizeof(OPENFILENAME));
							ofn.lStructSize = sizeof(OPENFILENAME);
							ofn.hInstance = g_hinst;
							ofn.hwndOwner = g_pvp->m_hwnd;
							// TEXT
							ofn.lpstrFilter = "Sound Files (*.wav)\0*.wav\0";
							ofn.lpstrFile = szFileName;
							ofn.nMaxFile = 10240;
							ofn.lpstrDefExt = "wav";
							ofn.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY | OFN_EXPLORER | OFN_ALLOWMULTISELECT;

							HRESULT hr = GetRegString("RecentDir","SoundDir", szInitialDir, 1024);
							if (hr == S_OK)
								{
								ofn.lpstrInitialDir = szInitialDir;
								}
							else
								{
								ofn.lpstrInitialDir = NULL;
								}

							const int ret = GetOpenFileName(&ofn);

							if(ret)
								{
								strcpy_s(szInitialDir, sizeof(szInitialDir), szFileName);

								int len = lstrlen(szFileName);
								if (len < ofn.nFileOffset)
									{
									// Multi-file select
									lstrcpy(szT, szFileName);
									lstrcat(szT, "\\");
									len++;
									int filenamestart = ofn.nFileOffset;
									int filenamelen = lstrlen(&szFileName[filenamestart]);
									while (filenamelen > 0)
										{
										lstrcpy(&szT[len], &szFileName[filenamestart]);
										pt->ImportSound(GetDlgItem(hwndDlg, IDC_SOUNDLIST), szT, fFalse);
										filenamestart += filenamelen+1;
										filenamelen = lstrlen(&szFileName[filenamestart]);
										}
									}
								else
									{
									szInitialDir[ofn.nFileOffset] = 0;
									pt->ImportSound(GetDlgItem(hwndDlg, IDC_SOUNDLIST), szFileName, fTrue);
									}
								hr = SetRegValue("RecentDir","SoundDir", REG_SZ, szInitialDir, strlen(szInitialDir));
								pt->SetNonUndoableDirty(eSaveDirty);
								}
							}
							break;

						case IDC_REIMPORT:
							{
							const int count = ListView_GetSelectedCount(GetDlgItem(hwndDlg, IDC_SOUNDLIST));
							if (count > 0)
								{
								LocalString ls(IDS_REPLACESOUND);
								const int ans = MessageBox(hwndDlg, ls.m_szbuffer/*"Are you sure you want to remove this image?"*/, "Visual Pinball", MB_YESNO | MB_DEFBUTTON2);
								if (ans == IDYES)
									{
									int sel = ListView_GetNextItem(GetDlgItem(hwndDlg, IDC_SOUNDLIST), -1, LVNI_SELECTED);
									while (sel != -1)
										{										
										LVITEM lvitem;
										lvitem.mask = LVIF_PARAM;
										lvitem.iItem = sel;
										lvitem.iSubItem = 0;
										ListView_GetItem(GetDlgItem(hwndDlg, IDC_SOUNDLIST), &lvitem);
										PinSound * const pps = (PinSound *)lvitem.lParam;

										//pt->ReImportSound(GetDlgItem(hwndDlg, IDC_SOUNDLIST), pps, pps->m_szPath, count == 1);									
										HANDLE hFile = CreateFile(pps->m_szPath,GENERIC_READ, FILE_SHARE_READ,	
														NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

										if (hFile != INVALID_HANDLE_VALUE)
											{
											CloseHandle(hFile);	
											pt->ReImportSound(GetDlgItem(hwndDlg, IDC_SOUNDLIST), pps, pps->m_szPath, count == 1);									
											pt->SetNonUndoableDirty(eSaveDirty);
											}
										else MessageBox(hwndDlg,pps->m_szPath, "  FILE NOT FOUND!  ", MB_OK);
										sel = ListView_GetNextItem(GetDlgItem(hwndDlg, IDC_SOUNDLIST), sel, LVNI_SELECTED);
										}
									}
								//pt->SetNonUndoableDirty(eSaveDirty);
								}
							}
							break;

						case IDC_REIMPORTFROM:
							{
							const int sel = ListView_GetNextItem(GetDlgItem(hwndDlg, IDC_SOUNDLIST), -1, LVNI_SELECTED);
							if (sel != -1)
								{
								char szFileName[1024];
								char szInitialDir[1024];

								LocalString ls(IDS_REPLACESOUND);
								int ans = MessageBox(hwndDlg, ls.m_szbuffer/*"Are you sure you want to replace this sound with a new one?"*/, "Visual Pinball", MB_YESNO | MB_DEFBUTTON2);
								if (ans == IDYES)
									{
									szFileName[0] = '\0';

									OPENFILENAME ofn;
									memset(&ofn, 0, sizeof(OPENFILENAME));
									ofn.lStructSize = sizeof(OPENFILENAME);
									ofn.hInstance = g_hinst;
									ofn.hwndOwner = g_pvp->m_hwnd;
									// TEXT
									ofn.lpstrFilter = "Sound Files (*.wav)\0*.wav\0";
									ofn.lpstrFile = szFileName;
									ofn.nMaxFile = _MAX_PATH;
									ofn.lpstrDefExt = "wav";
									ofn.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;

									HRESULT hr = GetRegString("RecentDir","SoundDir", szInitialDir, 1024);
									if (hr == S_OK)
										{
										ofn.lpstrInitialDir = szInitialDir;
										}
									else
										{
										ofn.lpstrInitialDir = NULL;
										}

									const int ret = GetOpenFileName(&ofn);

									if(ret)
										{
										LVITEM lvitem;
										lvitem.mask = LVIF_PARAM;
										lvitem.iItem = sel;
										lvitem.iSubItem = 0;
										ListView_GetItem(GetDlgItem(hwndDlg, IDC_SOUNDLIST), &lvitem);
										PinSound *pps = (PinSound *)lvitem.lParam;

										pt->ReImportSound(GetDlgItem(hwndDlg, IDC_SOUNDLIST), pps, ofn.lpstrFile, fTrue);
										pt->SetNonUndoableDirty(eSaveDirty);
										}
									}
								}
							}
							break;

//++++++++++++++++++++++++++++++++++++++++++++++++//rlc

						case IDC_SNDEXPORT:
							{								
							if(ListView_GetSelectedCount(GetDlgItem(hwndDlg, IDC_SOUNDLIST)))
								{	
								OPENFILENAME ofn;
								LVITEM lvitem;
								char szInitialDir[2096];
								int sel = ListView_GetNextItem(GetDlgItem(hwndDlg, IDC_SOUNDLIST), -1, LVNI_SELECTED); //next selected item 	
								while (sel != -1)
									{									
									lvitem.mask = LVIF_PARAM;
									lvitem.iItem = sel;
									lvitem.iSubItem = 0;
									ListView_GetItem(GetDlgItem(hwndDlg, IDC_SOUNDLIST), &lvitem);
									PinSound *pps = (PinSound *)lvitem.lParam;								

									memset(&ofn, 0, sizeof(OPENFILENAME));
									ofn.lStructSize = sizeof(OPENFILENAME);
									ofn.hInstance = g_hinst;
									ofn.hwndOwner = g_pvp->m_hwnd;
									//TEXT
									ofn.lpstrFilter = "Sound Files (*.wav)\0*.wav\0";
									
									int begin;		//select only file name from pathfilename
									const int len = lstrlen(pps->m_szPath);

									for (begin=len;begin>=0;begin--)
										{
										if (pps->m_szPath[begin] == '\\')
											{
											begin++;
											break;
											}
										}
									ofn.lpstrFile = &pps->m_szPath[begin];
									ofn.nMaxFile = 2096;
									ofn.lpstrDefExt = "wav";
									const HRESULT hr = GetRegString("RecentDir","SoundDir", szInitialDir,2096);

									if (hr == S_OK)ofn.lpstrInitialDir = szInitialDir;
									else ofn.lpstrInitialDir = NULL;	
																
									ofn.lpstrTitle = "SAVE AS";
									ofn.Flags = OFN_NOREADONLYRETURN | OFN_CREATEPROMPT | OFN_OVERWRITEPROMPT | OFN_EXPLORER;
									
									szInitialDir[ofn.nFileOffset] = 0;

									if (GetSaveFileName(&ofn))	//Get filename from user
										{																	
										if (pt->ExportSound(GetDlgItem(hwndDlg, IDC_SOUNDLIST), pps, ofn.lpstrFile))
											{
											//pt->ReImportSound(GetDlgItem(hwndDlg, IDC_SOUNDLIST), pps, ofn.lpstrFile, fTrue);
											//pt->SetNonUndoableDirty(eSaveDirty);
											}
										}
									sel = ListView_GetNextItem(GetDlgItem(hwndDlg, IDC_SOUNDLIST), sel, LVNI_SELECTED); //next selected item
									
									}
								/*const HRESULT hr =*/ SetRegValue("RecentDir","SoundDir", REG_SZ, szInitialDir, strlen(szInitialDir));
								EndDialog(hwndDlg, TRUE);
								}
							}
							break;

//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<//rlc

						case IDC_PLAY:
							{
							const int sel = ListView_GetNextItem(GetDlgItem(hwndDlg, IDC_SOUNDLIST), -1, LVNI_SELECTED);
							if (sel != -1)
								{
								LVITEM lvitem;
								lvitem.mask = LVIF_PARAM;
								lvitem.iItem = sel;
								lvitem.iSubItem = 0;
								ListView_GetItem(GetDlgItem(hwndDlg, IDC_SOUNDLIST), &lvitem);
								PinSound * const pps = (PinSound *)lvitem.lParam;
								pps->m_pDSBuffer->Play(0,0,0);
								}
							}
							break;
							

						case IDC_RENAME:
							{
							const int sel = ListView_GetNextItem(GetDlgItem(hwndDlg, IDC_SOUNDLIST), -1, LVNI_SELECTED);
							if (sel != -1)
								{
								SetFocus(GetDlgItem(hwndDlg, IDC_SOUNDLIST));
								/*const HWND hwndFoo =*/ ListView_EditLabel(GetDlgItem(hwndDlg, IDC_SOUNDLIST), sel);
								}
							}
							break;

						case IDC_DELETE:
							{
							const int count = ListView_GetSelectedCount(GetDlgItem(hwndDlg, IDC_SOUNDLIST));
							if (count > 0)
								{
								LocalString ls(IDS_REMOVESOUND);
								const int ans = MessageBox(hwndDlg, ls.m_szbuffer/*"Are you sure you want to remove this image?"*/, "Visual Pinball", MB_YESNO | MB_DEFBUTTON2);
								if (ans == IDYES)
									{
									int sel = ListView_GetNextItem(GetDlgItem(hwndDlg, IDC_SOUNDLIST), -1, LVNI_SELECTED);
									while (sel != -1)
										{										
										LVITEM lvitem;
										lvitem.mask = LVIF_PARAM;
										lvitem.iItem = sel;
										lvitem.iSubItem = 0;
										ListView_GetItem(GetDlgItem(hwndDlg, IDC_SOUNDLIST), &lvitem);
										PinSound *pps = (PinSound *)lvitem.lParam;
										pt->RemoveSound(pps);
										ListView_DeleteItem(GetDlgItem(hwndDlg, IDC_SOUNDLIST), sel);

										// The previous selection is now deleted, so look again from the top of the list
										sel = ListView_GetNextItem(GetDlgItem(hwndDlg, IDC_SOUNDLIST), -1, LVNI_SELECTED);
										}
									}
								pt->SetNonUndoableDirty(eSaveDirty);
								}
							}
							break;
						}
					break;
				}
			break;
		}

	return FALSE;
	}

int CALLBACK ImageManagerProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
	CCO(PinTable) *pt = (CCO(PinTable) *)GetWindowLong(hwndDlg, GWL_USERDATA);

	switch (uMsg)
		{
		case WM_INITDIALOG:
			{
			SetWindowLong(hwndDlg, GWL_USERDATA, lParam);
			LVCOLUMN lvcol;

			lvcol.mask = LVCF_TEXT | LVCF_WIDTH;
			LocalString ls(IDS_NAME);
			lvcol.pszText = ls.m_szbuffer;// = "Name";
			lvcol.cx = 100;
			ListView_InsertColumn(GetDlgItem(hwndDlg, IDC_SOUNDLIST), 0, &lvcol);

			LocalString ls2(IDS_IMPORTPATH);
			lvcol.pszText = ls2.m_szbuffer; // = "Import Path";
			lvcol.cx = 200;
			ListView_InsertColumn(GetDlgItem(hwndDlg, IDC_SOUNDLIST), 1, &lvcol);

			pt = (CCO(PinTable) *)GetWindowLong(hwndDlg, GWL_USERDATA);

			pt->ListImages(GetDlgItem(hwndDlg, IDC_SOUNDLIST));

			return TRUE;
			}
			break;

		case WM_CLOSE:
			EndDialog(hwndDlg, FALSE);
			break;

		case GET_COLOR_TABLE:
			{
			*((unsigned long **)lParam) = pt->rgcolorcustom;
			return TRUE;
			}
			break;

		case WM_NOTIFY:
			{
			LPNMHDR pnmhdr = (LPNMHDR)lParam;
			switch (pnmhdr->code)
				{
				case LVN_ENDLABELEDIT:
					{
					NMLVDISPINFO * const pinfo = (NMLVDISPINFO *)lParam;
					if (pinfo->item.pszText == NULL || pinfo->item.pszText[0] == '\0')
						{
						return FALSE;
						}
					ListView_SetItemText(GetDlgItem(hwndDlg, IDC_SOUNDLIST), pinfo->item.iItem, 0, pinfo->item.pszText);
					LVITEM lvitem;
					lvitem.mask = LVIF_PARAM;
					lvitem.iItem = pinfo->item.iItem;
					lvitem.iSubItem = 0;
					ListView_GetItem(GetDlgItem(hwndDlg, IDC_SOUNDLIST), &lvitem);
					PinImage * const ppi = (PinImage *)lvitem.lParam;
					lstrcpy(ppi->m_szName, pinfo->item.pszText);
					lstrcpy(ppi->m_szInternalName, pinfo->item.pszText);
					CharLowerBuff(ppi->m_szInternalName, lstrlen(ppi->m_szInternalName));
					pt->SetNonUndoableDirty(eSaveDirty);
					return TRUE;
					}
					break;
				case LVN_ITEMCHANGING:
					{
					NMLISTVIEW * const plistview = (LPNMLISTVIEW)lParam;
					if ((plistview->uNewState & LVIS_SELECTED) != (plistview->uOldState & LVIS_SELECTED))
						{
						InvalidateRect(GetDlgItem(hwndDlg, IDC_PICTUREPREVIEW), NULL, fTrue);
						if (plistview->uNewState & LVIS_SELECTED)
							{
							const int sel = plistview->iItem;
							LVITEM lvitem;
							lvitem.mask = LVIF_PARAM;
							lvitem.iItem = sel;
							lvitem.iSubItem = 0;
							ListView_GetItem(GetDlgItem(hwndDlg, IDC_SOUNDLIST), &lvitem);
							PinImage * const ppi = (PinImage *)lvitem.lParam;
							HWND hwndColor = GetDlgItem(hwndDlg, IDC_COLOR);
							SendMessage(hwndColor, CHANGE_COLOR, 0, ppi->m_rgbTransparent);
							InvalidateRect(hwndColor, NULL, FALSE);
							}
						}
					}
					break;

					case LVN_ITEMCHANGED:
						{
						const int count = ListView_GetSelectedCount(GetDlgItem(hwndDlg, IDC_SOUNDLIST));
						const int fEnable = !(count > 1);
						EnableWindow(GetDlgItem(hwndDlg, IDC_REIMPORTFROM), fEnable);
						EnableWindow(GetDlgItem(hwndDlg, IDC_RENAME), fEnable);
						//EnableWindow(GetDlgItem(hwndDlg, IDC_EXPORT), fEnable);
						}
					break;
				}
			}
			break;

		case WM_DRAWITEM:
			{
			DRAWITEMSTRUCT * const pdis = (DRAWITEMSTRUCT *)lParam;
			const int sel = ListView_GetNextItem(GetDlgItem(hwndDlg, IDC_SOUNDLIST), -1, LVNI_SELECTED);
			if (sel != -1)
				{
				LVITEM lvitem;
				lvitem.mask = LVIF_PARAM;
				lvitem.iItem = sel;
				lvitem.iSubItem = 0;
				ListView_GetItem(GetDlgItem(hwndDlg, IDC_SOUNDLIST), &lvitem);
				PinImage * const ppi = (PinImage *)lvitem.lParam;
				HDC hdcDD;
				//DDSURFACEDESC2 ddsd;
				//ddsd.dwSize = sizeof(ddsd);
				//ppi->m_pdsBuffer->GetSurfaceDesc( &ddsd );

				RECT rcClient;

				GetWindowRect(pdis->hwndItem , &rcClient);

				const int xsize = rcClient.right - rcClient.left;
				const int ysize =	rcClient.bottom - rcClient.top;

				const float controlaspect = (float)xsize/(float)ysize;
				const float aspect = (float)ppi->m_width/(float)ppi->m_height;

				int width, height;
				if (aspect > controlaspect)
					{
					width = xsize;
					height = (int)(xsize/aspect);
					}
				else
					{
					height = ysize;
					width = (int)(ysize*aspect);
					}

				const int x = (xsize - width) / 2;
				const int y = (ysize - height) / 2;

				ppi->m_pdsBuffer->GetDC(&hdcDD);
				StretchBlt(pdis->hDC, x, y, width, height, hdcDD, 0, 0, ppi->m_width, ppi->m_height, SRCCOPY);
				ppi->m_pdsBuffer->ReleaseDC(hdcDD);
				}
			else
				{
				// Nothing currently selected
				RECT rcClient;
				GetClientRect(pdis->hwndItem, &rcClient);
				SelectObject(pdis->hDC, GetStockObject(NULL_BRUSH));
				SelectObject(pdis->hDC, GetStockObject(BLACK_PEN));
				Rectangle(pdis->hDC, rcClient.left+5, rcClient.top+5, rcClient.right-5, rcClient.bottom-5);

				SetBkMode(pdis->hDC, TRANSPARENT);

				RECT rcText = rcClient;

				//ExtTextOut(pdis->hDC, 0, 20, 0, NULL, "Image\nPreview", 13, NULL);
				LocalString ls(IDS_IMAGE_PREVIEW);
				const int len = lstrlen(ls.m_szbuffer);
				DrawText(pdis->hDC, ls.m_szbuffer/*"Image\n\nPreview"*/, len, &rcText, DT_CALCRECT);

				const int halfheight = (rcClient.bottom - rcClient.top) / 2;
				const int halffont = (rcText.bottom - rcText.top) / 2;

				rcText.left = rcClient.left;
				rcText.right = rcClient.right;
				rcText.top = halfheight - halffont;
				rcText.bottom = halfheight + halffont;

				DrawText(pdis->hDC, ls.m_szbuffer/*"Image\n\nPreview"*/, len, &rcText, DT_CENTER);
				}
			return TRUE;
			}
			break;

		case WM_COMMAND:
			switch (HIWORD(wParam))
				{
				case COLOR_CHANGED:
					{
					const int count = ListView_GetSelectedCount(GetDlgItem(hwndDlg, IDC_SOUNDLIST));
					if (count > 0)
						{
						int color = SendMessage((HWND)lParam, WM_GETTEXT, 0, 0);
						int sel = ListView_GetNextItem(GetDlgItem(hwndDlg, IDC_SOUNDLIST), -1, LVNI_SELECTED);
						while (sel != -1)
							{							
							LVITEM lvitem;
							lvitem.mask = LVIF_PARAM;
							lvitem.iItem = sel;
							lvitem.iSubItem = 0;
							ListView_GetItem(GetDlgItem(hwndDlg, IDC_SOUNDLIST), &lvitem);
							PinImage * const ppi = (PinImage *)lvitem.lParam;
							ppi->SetTransparentColor(color);

							/*const HRESULT hr =*/ SetRegValue("Editor", "TransparentColorKey", REG_DWORD, &color, 4);

							// The previous selection is now deleted, so look again from the top of the list
							sel = ListView_GetNextItem(GetDlgItem(hwndDlg, IDC_SOUNDLIST), sel, LVNI_SELECTED);
							}

						pt->SetNonUndoableDirty(eSaveDirty);
						}
					}
					break;

				case BN_CLICKED:
					switch (LOWORD(wParam))
						{
						case IDOK:
							EndDialog(hwndDlg, TRUE);
							break;

						case IDCANCEL:
							EndDialog(hwndDlg, FALSE);
							break;

						case IDC_IMPORT:
							{
							char szFileName[10240];
							char szInitialDir[10240];
							char szT[MAX_PATH];
							szFileName[0] = '\0';

							OPENFILENAME ofn;
							memset(&ofn, 0, sizeof(OPENFILENAME));
							ofn.lStructSize = sizeof(OPENFILENAME);
							ofn.hInstance = g_hinst;
							ofn.hwndOwner = g_pvp->m_hwnd;
							// TEXT
							ofn.lpstrFilter = "Bitmap and JPEG Files (*.bmp, *.jpg, *.png)\0*.bmp;*.jpg;*.jpeg;*.png\0";
							ofn.lpstrFile = szFileName;
							ofn.nMaxFile = 10240;
							ofn.lpstrDefExt = "bmp";
							ofn.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY | OFN_EXPLORER | OFN_ALLOWMULTISELECT;

							HRESULT hr = GetRegString("RecentDir","ImageDir", szInitialDir, 1024);
							ofn.lpstrInitialDir = (hr == S_OK) ? szInitialDir : NULL;

							const int ret = GetOpenFileName(&ofn);

							if(ret)
								{
								strcpy_s(szInitialDir, sizeof(szInitialDir), szFileName);

								int len = lstrlen(szFileName);
								if (len < ofn.nFileOffset)
									{
									// Multi-file select
									lstrcpy(szT, szFileName);
									lstrcat(szT, "\\");
									len++;
									int filenamestart = ofn.nFileOffset;
									int filenamelen = lstrlen(&szFileName[filenamestart]);
									while (filenamelen > 0)
										{
										lstrcpy(&szT[len], &szFileName[filenamestart]);
										pt->ImportImage(GetDlgItem(hwndDlg, IDC_SOUNDLIST), szT);
										filenamestart += filenamelen+1;
										filenamelen = lstrlen(&szFileName[filenamestart]);
										}
									}
								else
									{
									szInitialDir[ofn.nFileOffset] = 0;
									pt->ImportImage(GetDlgItem(hwndDlg, IDC_SOUNDLIST), szFileName);
									}
								hr = SetRegValue("RecentDir","ImageDir", REG_SZ, szInitialDir, strlen(szInitialDir));
								pt->SetNonUndoableDirty(eSaveDirty);
								}
							}
							break;

						case IDC_RENAME:
							{
							const int sel = ListView_GetNextItem(GetDlgItem(hwndDlg, IDC_SOUNDLIST), -1, LVNI_SELECTED);
							if (sel != -1)
								{
								SetFocus(GetDlgItem(hwndDlg, IDC_SOUNDLIST));
								/*const HWND hwndFoo =*/ ListView_EditLabel(GetDlgItem(hwndDlg, IDC_SOUNDLIST), sel);
								}
							}
							break;
/////////////////////////////rlc  begin
						case IDC_EXPORT:
							{
							if(ListView_GetSelectedCount(GetDlgItem(hwndDlg, IDC_SOUNDLIST)))	// if some items are selected???
                            	{
								char szInitialDir[2096];
								int sel = ListView_GetNextItem(GetDlgItem(hwndDlg, IDC_SOUNDLIST), -1, LVNI_SELECTED);								
								while (sel != -1)
									{									
									LVITEM lvitem;
									lvitem.mask = LVIF_PARAM;
									lvitem.iItem = sel;
									lvitem.iSubItem = 0;
									ListView_GetItem(GetDlgItem(hwndDlg, IDC_SOUNDLIST), &lvitem);
									PinImage * const ppi = (PinImage *)lvitem.lParam;									

									OPENFILENAME ofn;
									memset(&ofn, 0, sizeof(OPENFILENAME));
									ofn.lStructSize = sizeof(OPENFILENAME);
									ofn.hInstance = g_hinst;
									ofn.hwndOwner = g_pvp->m_hwnd;
									//TEXT
									ofn.lpstrFilter = "Bitmap and JPEG Files (*.bmp, *.jpg, *.png)\0*.bmp;*.jpg;*.jpeg;*.png\0";
									
									int begin;		//select only file name from pathfilename
									const int len = lstrlen(ppi->m_szPath);

									for (begin=len;begin>=0;begin--)
										{
										if (ppi->m_szPath[begin] == '\\')
											{
											begin++;
											break;
											}
										}
									ofn.lpstrFile = &ppi->m_szPath[begin];
									ofn.nMaxFile = 2096;
									ofn.lpstrDefExt = "bmp";

									const HRESULT hr = GetRegString("RecentDir","ImageDir", szInitialDir, 2096);

									if (hr == S_OK)ofn.lpstrInitialDir = szInitialDir;
									else ofn.lpstrInitialDir = NULL;	
																
									ofn.lpstrTitle = "SAVE AS";
									ofn.Flags = OFN_NOREADONLYRETURN | OFN_CREATEPROMPT | OFN_OVERWRITEPROMPT | OFN_EXPLORER;
									
									szInitialDir[ofn.nFileOffset] = 0;

									if (GetSaveFileName(&ofn))	//Get filename from user
										{																	
										if (pt->ExportImage(GetDlgItem(hwndDlg, IDC_SOUNDLIST), ppi, ofn.lpstrFile))
											{
											//pt->ReImportImage(GetDlgItem(hwndDlg, IDC_SOUNDLIST), ppi, ofn.lpstrFile);
											//pt->SetNonUndoableDirty(eSaveDirty);
											}
										}
									sel = ListView_GetNextItem(GetDlgItem(hwndDlg, IDC_SOUNDLIST), sel, LVNI_SELECTED);
									} // finished all selected items
								/*const HRESULT hr =*/ SetRegValue("RecentDir","ImageDir", REG_SZ, szInitialDir, strlen(szInitialDir));
								EndDialog(hwndDlg, TRUE);
								}							
							}	

							break;
///////////////////////////rlc end
						case IDC_DELETE:
							{
							const int count = ListView_GetSelectedCount(GetDlgItem(hwndDlg, IDC_SOUNDLIST));
							if (count > 0)
								{
								LocalString ls(IDS_REMOVEIMAGE);
								const int ans = MessageBox(hwndDlg, ls.m_szbuffer/*"Are you sure you want to remove this image?"*/, "Visual Pinball", MB_YESNO | MB_DEFBUTTON2);
								if (ans == IDYES)
									{
									int sel = ListView_GetNextItem(GetDlgItem(hwndDlg, IDC_SOUNDLIST), -1, LVNI_SELECTED);
									while (sel != -1)
										{										
										LVITEM lvitem;
										lvitem.mask = LVIF_PARAM;
										lvitem.iItem = sel;
										lvitem.iSubItem = 0;
										ListView_GetItem(GetDlgItem(hwndDlg, IDC_SOUNDLIST), &lvitem);
										PinImage * const ppi = (PinImage *)lvitem.lParam;
										pt->RemoveImage(ppi);
										ListView_DeleteItem(GetDlgItem(hwndDlg, IDC_SOUNDLIST), sel);

										// The previous selection is now deleted, so look again from the top of the list
										sel = ListView_GetNextItem(GetDlgItem(hwndDlg, IDC_SOUNDLIST), -1, LVNI_SELECTED);
										}
									}
								pt->SetNonUndoableDirty(eSaveDirty);
								}
							}
							break;

						case IDC_REIMPORT:
							{
							const int count = ListView_GetSelectedCount(GetDlgItem(hwndDlg, IDC_SOUNDLIST));
							if (count > 0)
								{
								LocalString ls(IDS_REPLACEIMAGE);
								const int ans = MessageBox(hwndDlg, ls.m_szbuffer/*"Are you sure you want to replace this image?"*/, "Visual Pinball", MB_YESNO | MB_DEFBUTTON2);
								if (ans == IDYES)
									{
									int sel = ListView_GetNextItem(GetDlgItem(hwndDlg, IDC_SOUNDLIST), -1, LVNI_SELECTED);
									while (sel != -1)
										{										
										LVITEM lvitem;
										lvitem.mask = LVIF_PARAM;
										lvitem.iItem = sel;
										lvitem.iSubItem = 0;
										ListView_GetItem(GetDlgItem(hwndDlg, IDC_SOUNDLIST), &lvitem);
										PinImage * const ppi = (PinImage *)lvitem.lParam;
										HANDLE hFile = CreateFile(ppi->m_szPath,GENERIC_READ, FILE_SHARE_READ,	
														NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

										if (hFile != INVALID_HANDLE_VALUE)
											{
											CloseHandle(hFile);	
											pt->ReImportImage(GetDlgItem(hwndDlg, IDC_SOUNDLIST), ppi, ppi->m_szPath);
											pt->SetNonUndoableDirty(eSaveDirty);
											}
										else MessageBox(hwndDlg,ppi->m_szPath, "  FILE NOT FOUND!  ", MB_OK);
										sel = ListView_GetNextItem(GetDlgItem(hwndDlg, IDC_SOUNDLIST), sel, LVNI_SELECTED);
										}
									}
								// Display new image
								InvalidateRect(GetDlgItem(hwndDlg, IDC_PICTUREPREVIEW), NULL, fTrue);								
								}
							}
							break;

						case IDC_REIMPORTFROM:
							{
							int sel = ListView_GetNextItem(GetDlgItem(hwndDlg, IDC_SOUNDLIST), -1, LVNI_SELECTED);
							if (sel != -1)
								{
								char szFileName[1024];
								char szInitialDir[1024];

								LocalString ls(IDS_REPLACEIMAGE);
								const int ans = MessageBox(hwndDlg, ls.m_szbuffer/*"Are you sure you want to replace this image with a new one?"*/, "Visual Pinball", MB_YESNO | MB_DEFBUTTON2);
								if (ans == IDYES)
									{
									szFileName[0] = '\0';

									OPENFILENAME ofn;
									memset(&ofn, 0, sizeof(OPENFILENAME));
									ofn.lStructSize = sizeof(OPENFILENAME);
									ofn.hInstance = g_hinst;
									ofn.hwndOwner = g_pvp->m_hwnd;
									// TEXT
									ofn.lpstrFilter = "Bitmap and JPEG Files (*.bmp, *.jpg, *.png)\0*.bmp;*.jpg;*.jpeg;*.png\0";
									ofn.lpstrFile = szFileName;
									ofn.nMaxFile = _MAX_PATH;
									ofn.lpstrDefExt = "bmp";
									ofn.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;

									HRESULT hr = GetRegString("RecentDir","ImageDir", szInitialDir, 1024);
									ofn.lpstrInitialDir = (hr == S_OK) ? szInitialDir : NULL;

									const int ret = GetOpenFileName(&ofn);

									if(ret)
										{
										LVITEM lvitem;
										lvitem.mask = LVIF_PARAM;
										lvitem.iItem = sel;
										lvitem.iSubItem = 0;
										ListView_GetItem(GetDlgItem(hwndDlg, IDC_SOUNDLIST), &lvitem);
										PinImage * const ppi = (PinImage *)lvitem.lParam;

										strcpy_s(szInitialDir, sizeof(szInitialDir), szFileName);
										szInitialDir[ofn.nFileOffset] = 0;
										hr = SetRegValue("RecentDir","ImageDir", REG_SZ, szInitialDir, strlen(szInitialDir));

										pt->ReImportImage(GetDlgItem(hwndDlg, IDC_SOUNDLIST), ppi, ofn.lpstrFile);
										pt->SetNonUndoableDirty(eSaveDirty);

										// Display new image
										InvalidateRect(GetDlgItem(hwndDlg, IDC_PICTUREPREVIEW), NULL, fTrue);
										}
									}
								}
							}
							break;
						}
					break;
				}
			break;
		}

	return FALSE;
	}

int CALLBACK AboutProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
	switch (uMsg)
		{
		case WM_INITDIALOG:
			{
			HWND hwndParent = GetParent(hwndDlg);
			RECT rcDlg;
			RECT rcMain;
			GetWindowRect(hwndParent, &rcMain);
			GetWindowRect(hwndDlg, &rcDlg);

			SetWindowPos(hwndDlg, NULL,
				(rcMain.right + rcMain.left)/2 - (rcDlg.right - rcDlg.left)/2,
				(rcMain.bottom + rcMain.top)/2 - (rcDlg.bottom - rcDlg.top)/2,
				0, 0, SWP_NOOWNERZORDER | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE/* | SWP_NOMOVE*/);

			char szVersion[256];
			char szBuild[32];
//			char szTime[]=__TIME__;
//			char szDate[]=__DATE__; 

//			char szSVNDate[]= SVNDATE;
//			char szSVNRev[]= SVNREVISION;
		
			lstrcpy(szVersion, "Version "); //rlc add time and date to compilation version
			_itoa_s(BUILD_NUMBER, szBuild, sizeof(szBuild), 10);
			lstrcat(szVersion, szBuild);
//			lstrcat(szVersion, "-");
			
//			lstrcat(szVersion, szTime);
//			lstrcat(szVersion, "-");
//			lstrcat(szVersion, szDate);

//			lstrcat(szVersion, ")\n\nSVN");		//rlc place Subversion revision and date in About Dialog

//			szSVNRev[0]= ' '; szSVNRev[strlen(szSVNRev)-1] = 0; 
//			lstrcat(szVersion, szSVNRev);
//
//			szSVNDate[0] = ' '; szSVNDate[strlen(szSVNDate)-1] = 0;
//			lstrcat(szVersion, szSVNDate);
			

			HWND hwndVersion = GetDlgItem(hwndDlg, IDC_VERSION);
			SetWindowText(hwndVersion, szVersion);

#if !(defined(IMSPANISH) | defined(IMGERMAN) | defined(IMFRENCH))
			HWND hwndTransName = GetDlgItem(hwndDlg, IDC_TRANSNAME);
			ShowWindow(hwndTransName, SW_HIDE);
#endif

#if !(defined(IMSPANISH))
			HWND hwndTransSite = GetDlgItem(hwndDlg, IDC_TRANSLATEWEBSITE);
			ShowWindow(hwndTransSite, SW_HIDE);
#endif
			}

			return TRUE;
			break;

		case WM_COMMAND:
			{
			switch (HIWORD(wParam))
				{
				case BN_CLICKED:
					switch (LOWORD(wParam))
						{
						case IDOK:
							EndDialog(hwndDlg, TRUE);
							break;

						case IDC_WEBSITE:
						case IDC_TRANSSITE:
							{
							HRESULT hr;
							if (LOWORD(wParam) == IDC_WEBSITE)
								{
								hr = OpenURL("http://www.vpforums.org");
								}
							else
								{
								HWND hwndTransURL = GetDlgItem(hwndDlg, IDC_TRANSWEBSITE);
								char szSite[MAX_PATH];
								GetWindowText(hwndTransURL, szSite, MAX_PATH);

								// "http://perso.wanadoo.es/tecnopinball/"
								hr = OpenURL(szSite);
								}
							/*IUniformResourceLocator* pURL;

							HRESULT hres = CoCreateInstance(CLSID_InternetShortcut, NULL, CLSCTX_INPROC_SERVER, IID_IUniformResourceLocator, (void**) &pURL);
							if (!SUCCEEDED(hres))
								{
								return FALSE;
								}

							if (LOWORD(wParam) == IDC_WEBSITE)
								{
								hres = pURL->SetURL("http://www.visualpinball.com", IURL_SETURL_FL_GUESS_PROTOCOL);
								}
							else
								{
								HWND hwndTransURL = GetDlgItem(hwndDlg, IDC_TRANSWEBSITE);
								char szSite[MAX_PATH];
								GetWindowText(hwndTransURL, szSite, MAX_PATH);

								// "http://perso.wanadoo.es/tecnopinball/"
								hres = pURL->SetURL(szSite, IURL_SETURL_FL_GUESS_PROTOCOL);
								}

							if (!SUCCEEDED(hres))
								{
								pURL->Release();
								return FALSE;
								}

							//Open the URL by calling InvokeCommand
							URLINVOKECOMMANDINFO ivci;
							ivci.dwcbSize = sizeof(URLINVOKECOMMANDINFO);
							ivci.dwFlags = IURL_INVOKECOMMAND_FL_ALLOW_UI;
							ivci.hwndParent = g_pvp->m_hwnd;
							ivci.pcszVerb = "open";
							hres = pURL->InvokeCommand(&ivci);
							pURL->Release();
							return (SUCCEEDED(hres));*/
							return (SUCCEEDED(hr));
							}
							break;
						}
				}
			}
			break;

		case WM_CLOSE:
			EndDialog(hwndDlg, TRUE);
			break;
		}

	return FALSE;
	}

struct VideoMode
	{
	int width;
	int height;
	int depth;
	int refreshrate;
	};

struct EnumVideoModeStruct
	{
	int widthcur;
	int heightcur;
	int depthcur;
	HWND hwndList;
	};

HRESULT WINAPI EnumModesCallback2(LPDDSURFACEDESC2 lpDDSurfaceDesc, LPVOID lpContext)
	{
//#ifdef ULTRACADE
	// Only allow 32-bit color depths on minimum 640 size monitors.
//	if (lpDDSurfaceDesc->dwWidth >= 640 && lpDDSurfaceDesc->ddpfPixelFormat.dwRGBBitCount >= 32)
//#else
	// Throw away displays we won't do (ModeX and 8-bit)
	if (lpDDSurfaceDesc->dwWidth >= 640 && lpDDSurfaceDesc->ddpfPixelFormat.dwRGBBitCount >= 16)
//#endif		
	{
		char szT[128];
		EnumVideoModeStruct *pevms = (EnumVideoModeStruct *)lpContext;
		HWND hwndList = pevms->hwndList;
		const int widthcur = pevms->widthcur;
		const int heightcur = pevms->heightcur;
		const int depthcur = pevms->depthcur;
		sprintf_s(szT, "%u x %u x %u", lpDDSurfaceDesc->dwWidth, lpDDSurfaceDesc->dwHeight, lpDDSurfaceDesc->ddpfPixelFormat.dwRGBBitCount);
		const int index = SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)szT);

		VideoMode * const pvm = new VideoMode();
		pvm->width = lpDDSurfaceDesc->dwWidth;
		pvm->height = lpDDSurfaceDesc->dwHeight;
		pvm->depth = lpDDSurfaceDesc->ddpfPixelFormat.dwRGBBitCount;
		pvm->refreshrate = 0;
		SendMessage(hwndList, LB_SETITEMDATA, index, (LPARAM)pvm);

		if (pvm->width == widthcur && pvm->height == heightcur && pvm->depth == depthcur)
			{
			SendMessage(hwndList, LB_SETCURSEL, index, 0);
			}
		}
	return DDENUMRET_OK;
	}


const int rgwindowsize[] = {640, 720, 800, 912, 1024, 1152, 1280, 1600};  //rlc  What is this?

int CALLBACK VideoOptionsProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
	switch (uMsg)
		{
		case WM_INITDIALOG:
			{
			HWND hwndParent = GetParent(hwndDlg);
			RECT rcDlg;
			RECT rcMain;
			GetWindowRect(hwndParent, &rcMain);
			GetWindowRect(hwndDlg, &rcDlg);

			SetWindowPos(hwndDlg, NULL,
				(rcMain.right + rcMain.left)/2 - (rcDlg.right - rcDlg.left)/2,
				(rcMain.bottom + rcMain.top)/2 - (rcDlg.bottom - rcDlg.top)/2,
				0, 0, SWP_NOOWNERZORDER | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE/* | SWP_NOMOVE*/);

			HWND hwndCheck = GetDlgItem(hwndDlg, IDC_CHECKBLIT);
			int checkblit;
			HRESULT hr = GetRegInt("Player", "CheckBlit", &checkblit);
			if (hr != S_OK)
				{
				checkblit = g_pvp->FDefaultCheckBlit(); // The default
				}
			SendMessage(hwndCheck, BM_SETCHECK, checkblit ? BST_CHECKED : BST_UNCHECKED, 0);

			hwndCheck = GetDlgItem(hwndDlg, IDC_SHADOW);
			int shadow;
			hr = GetRegInt("Player", "BallShadows", &shadow);
			if (hr != S_OK)
				{
				shadow = fTrue; // The default
				}
			SendMessage(hwndCheck, BM_SETCHECK, shadow ? BST_CHECKED : BST_UNCHECKED, 0);

			hwndCheck = GetDlgItem(hwndDlg, IDC_DECAL);
			int decal;
			hr = GetRegInt("Player", "BallDecals", &decal);
			if (hr != S_OK)
				{
				decal = fTrue; // The default
				}
			SendMessage(hwndCheck, BM_SETCHECK, decal ? BST_CHECKED : BST_UNCHECKED, 0);

			hwndCheck = GetDlgItem(hwndDlg, IDC_ANTIALIAS);
			int antialias;
			hr = GetRegInt("Player", "BallAntialias", &antialias);
			if (hr != S_OK)
				{
				antialias = fTrue; // The default
				}
			SendMessage(hwndCheck, BM_SETCHECK, antialias ? BST_CHECKED : BST_UNCHECKED, 0);

			hwndCheck = GetDlgItem(hwndDlg, 211); //HardwareRender
			int hardrend;
			hr = GetRegInt("Player", "HardwareRender", &hardrend);
			if (hr != S_OK)
				{
				hardrend = fFalse;
				}
			SendMessage(hwndCheck, BM_SETCHECK, hardrend ? BST_CHECKED : BST_UNCHECKED, 0);

			hwndCheck = GetDlgItem(hwndDlg, 216); //AlternateRender
			int altrender;
			hr = GetRegInt("Player", "AlternateRender", &altrender);
			if (hr != S_OK)
				{
				altrender = fFalse; // The default
				}
			SendMessage(hwndCheck, BM_SETCHECK, altrender ? BST_CHECKED : BST_UNCHECKED, 0);


			int widthcur = 0;
			//int indexcur = -1;
			hr = GetRegInt("Player", "Width", &widthcur);
			if (hr != S_OK)
				{
				widthcur = DEFAULT_PLAYER_WIDTH; // The default
				}

			int heightcur = 0;
			hr = GetRegInt("Player", "Height", &heightcur);
			if (hr != S_OK)
				{
				heightcur = widthcur*3/4; // The default
				}

			int depthcur = 0;
			hr = GetRegInt("Player", "ColorDepth", &depthcur);
			if (hr != S_OK)
				{
				depthcur = 32; // The default
				}

			int fullscreen;
			hr = GetRegInt("Player", "FullScreen", &fullscreen);
			if (hr != S_OK)
				{
				fullscreen = 0; // The default
				}

			if (fullscreen)
				{
				SendMessage(hwndDlg, GET_FULLSCREENMODES, widthcur, heightcur<<16 | depthcur);
				HWND hwndRadio = GetDlgItem(hwndDlg, IDC_FULLSCREEN);
				SendMessage(hwndRadio, BM_SETCHECK, BST_CHECKED, 0);
				}
			else
				{
				SendMessage(hwndDlg, GET_WINDOW_MODES, widthcur, 0);
				HWND hwndRadio = GetDlgItem(hwndDlg, IDC_WINDOW);
				SendMessage(hwndRadio, BM_SETCHECK, BST_CHECKED, 0);
				}
			}

			return TRUE;
			break;

		case WM_COMMAND:
			{
			switch (HIWORD(wParam))
				{
				case BN_CLICKED:
					switch (LOWORD(wParam))
						{
						case IDOK:
							{
							HWND hwndFullScreen = GetDlgItem(hwndDlg, IDC_FULLSCREEN);
							int fullscreen = SendMessage(hwndFullScreen, BM_GETCHECK, 0, 0);
							SetRegValue("Player", "FullScreen", REG_DWORD, &fullscreen, 4);

							HWND hwndList = GetDlgItem(hwndDlg, IDC_SIZELIST);
							int index = SendMessage(hwndList, LB_GETCURSEL, 0, 0);
							VideoMode* pvm = (VideoMode*)SendMessage(hwndList, LB_GETITEMDATA, index, 0);
							SetRegValue("Player", "Width", REG_DWORD, &pvm->width, 4);
							SetRegValue("Player", "Height", REG_DWORD, &pvm->height, 4);
							if (fullscreen)
								{
								SetRegValue("Player", "ColorDepth", REG_DWORD, &pvm->depth, 4);
								SetRegValue("Player", "RefreshRate", REG_DWORD, &pvm->refreshrate, 4);
								}

							HWND hwndCheck = GetDlgItem(hwndDlg, IDC_CHECKBLIT);
							int checkblit = SendMessage(hwndCheck, BM_GETCHECK, 0, 0);
							SetRegValue("Player", "CheckBlit", REG_DWORD, &checkblit, 4);

							HWND hwndShadows = GetDlgItem(hwndDlg, IDC_SHADOW);
							int shadow = SendMessage(hwndShadows, BM_GETCHECK, 0, 0);
							SetRegValue("Player", "BallShadows", REG_DWORD, &shadow, 4);

							HWND hwndDecals = GetDlgItem(hwndDlg, IDC_DECAL);
							int decal = SendMessage(hwndDecals, BM_GETCHECK, 0, 0);
							SetRegValue("Player", "BallDecals", REG_DWORD, &decal, 4);
							
							HWND hwndAlias = GetDlgItem(hwndDlg, IDC_ANTIALIAS);
							int antialias = SendMessage(hwndAlias, BM_GETCHECK, 0, 0);
							SetRegValue("Player", "BallAntialias", REG_DWORD, &antialias, 4);

							HWND hwndHWR = GetDlgItem(hwndDlg, 211);
							int hardrend = SendMessage(hwndHWR, BM_GETCHECK, 0, 0);
							SetRegValue("Player", "HardwareRender", REG_DWORD, &hardrend, 4);
							g_pvp->m_pdd.m_fHardwareAccel = (hardrend != 0);

							HWND hwndARend = GetDlgItem(hwndDlg, 216);
							int altrend = SendMessage(hwndARend, BM_GETCHECK, 0, 0);
							SetRegValue("Player", "AlternateRender", REG_DWORD, &altrend, 4);
							g_pvp->m_pdd.m_fAlternateRender = (altrend != 0);


							EndDialog(hwndDlg, TRUE);
							}
							break;

						case IDC_WINDOW:
							{
							SendMessage(hwndDlg, GET_WINDOW_MODES, 0, 0);
							}
							break;

						case IDC_FULLSCREEN:
							{
							SendMessage(hwndDlg, GET_FULLSCREENMODES, 0, 0);
							}
							break;

						case IDCANCEL:
							EndDialog(hwndDlg, FALSE);
							break;
						}
				}
			}
			break;

		case GET_WINDOW_MODES:
			{
			int indexcur = -1;
			int widthcur = wParam;

			SendMessage(hwndDlg, RESET_SIZELIST_CONTENT, 0, 0);
			HWND hwndList = GetDlgItem(hwndDlg, IDC_SIZELIST);

			const int csize = sizeof(rgwindowsize)/sizeof(int);
			const int screenwidth = GetSystemMetrics(SM_CXSCREEN);
			char szT[128];
			for (int i=0;i<csize;i++)
				{
				const int xsize = rgwindowsize[i];
				if (xsize <= screenwidth)
					{
					if (xsize == widthcur)
						{
						indexcur = i;
						}
					sprintf_s(szT, "%d x %d", xsize, xsize*3/4);
					const int index = SendMessage(hwndList, LB_ADDSTRING, 0, (long)szT);
					VideoMode * const pvm = new VideoMode();
					pvm->width = xsize;
					pvm->height = xsize*3/4;
					SendMessage(hwndList, LB_SETITEMDATA, index, (LPARAM)pvm);
					}
				}

			if (indexcur != -1)
				{
				SendMessage(hwndList, LB_SETCURSEL, indexcur, 0);
				}
			else
				{
				SendMessage(hwndList, LB_SETCURSEL, 0, 0);
				}
			}
			break;

		case GET_FULLSCREENMODES:
			{
			SendMessage(hwndDlg, RESET_SIZELIST_CONTENT, 0, 0);
			HWND hwndList = GetDlgItem(hwndDlg, IDC_SIZELIST);
			EnumVideoModeStruct evms;
			evms.widthcur = wParam;
			evms.heightcur = lParam>>16;
			evms.depthcur = lParam & 0xffff;
			evms.hwndList = hwndList;
			g_pvp->m_pdd.m_pDD->EnumDisplayModes(0, NULL, &evms, EnumModesCallback2);

			if (SendMessage(hwndList, LB_GETCURSEL, 0, 0) == -1)
				{
				SendMessage(hwndList, LB_SETCURSEL, 0, 0);
				}
			}
			break;

		case RESET_SIZELIST_CONTENT:
			{
			HWND hwndList = GetDlgItem(hwndDlg, IDC_SIZELIST);
			const int size = SendMessage(hwndList, LB_GETCOUNT, 0, 0);
			for (int i=0;i<size;i++)
				{
				VideoMode * const pvm = (VideoMode *)SendMessage(hwndList, LB_GETITEMDATA, i, 0);
				delete pvm;
				}
			SendMessage(hwndList, LB_RESETCONTENT, 0, 0);
			}
			break;

		case WM_CLOSE:
			EndDialog(hwndDlg, FALSE);
			break;

		case WM_DESTROY:
			SendMessage(hwndDlg, RESET_SIZELIST_CONTENT, 0, 0);
			break;
		}

	return FALSE;
	}

int rgDlgIDFromSecurityLevel [] = {IDC_ACTIVEX0, IDC_ACTIVEX1, IDC_ACTIVEX2, IDC_ACTIVEX3, IDC_ACTIVEX4};

int CALLBACK SecurityOptionsProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
	switch (uMsg)
		{
		case WM_INITDIALOG:
			{
			HWND hwndParent = GetParent(hwndDlg);
			RECT rcDlg;
			RECT rcMain;
			GetWindowRect(hwndParent, &rcMain);
			GetWindowRect(hwndDlg, &rcDlg);

			SetWindowPos(hwndDlg, NULL,
				(rcMain.right + rcMain.left)/2 - (rcDlg.right - rcDlg.left)/2,
				(rcMain.bottom + rcMain.top)/2 - (rcDlg.bottom - rcDlg.top)/2,
				0, 0, SWP_NOOWNERZORDER | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE/* | SWP_NOMOVE*/);

			int security;
			HRESULT hr = GetRegInt("Player", "SecurityLevel", &security);
			if (hr != S_OK)
				{
				security = DEFAULT_SECURITY_LEVEL; // The default
				}

			if (security < 0 || security > 4)
				{
				security = 0;
				}

			const int buttonid = rgDlgIDFromSecurityLevel[security];

			HWND hwndCheck = GetDlgItem(hwndDlg, buttonid);

			SendMessage(hwndCheck, BM_SETCHECK, BST_CHECKED, 0);
			
			HWND hwndDetectHang = GetDlgItem(hwndDlg, IDC_HANGDETECT);
			int hangdetect;
			hr = GetRegInt("Player", "DetectHang", &hangdetect);
			if (hr != S_OK)
				{
				hangdetect = fFalse; // The default
				}
			SendMessage(hwndDetectHang, BM_SETCHECK, hangdetect ? BST_CHECKED : BST_UNCHECKED, 0);

			return TRUE;
			}
			break;

		case WM_COMMAND:
			{
			switch (HIWORD(wParam))
				{
				case BN_CLICKED:
					switch (LOWORD(wParam))
						{
						case IDOK:
							{
							for (int i=0;i<5;i++)
								{
								HWND hwndCheck = GetDlgItem(hwndDlg, rgDlgIDFromSecurityLevel[i]);
								int checked = SendMessage(hwndCheck, BM_GETCHECK, 0, 0);
								if (checked == BST_CHECKED)
									{
									SetRegValue("Player", "SecurityLevel", REG_DWORD, &i, 4);
									}
								}
								
							HWND hwndCheck = GetDlgItem(hwndDlg, IDC_HANGDETECT);
							int hangdetect = SendMessage(hwndCheck, BM_GETCHECK, 0, 0);
							SetRegValue("Player", "DetectHang", REG_DWORD, &hangdetect, 4);

							EndDialog(hwndDlg, TRUE);
							}
							break;

						case IDCANCEL:
							EndDialog(hwndDlg, FALSE);
							break;
						}
				}
			}
			break;

		case WM_CLOSE:
			EndDialog(hwndDlg, FALSE);
			break;
		}

	return FALSE;
	}

int CALLBACK FontManagerProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
	CCO(PinTable) *pt = (CCO(PinTable) *)GetWindowLong(hwndDlg, GWL_USERDATA);

	switch (uMsg)
		{
		case WM_INITDIALOG:
			{
			SetWindowLong(hwndDlg, GWL_USERDATA, lParam);
			
			LVCOLUMN lvcol;
			lvcol.mask = LVCF_TEXT | LVCF_WIDTH;
			LocalString ls(IDS_NAME);
			lvcol.pszText = ls.m_szbuffer;// = "Name";
			lvcol.cx = 100;
			ListView_InsertColumn(GetDlgItem(hwndDlg, IDC_SOUNDLIST), 0, &lvcol);

			LocalString ls2(IDS_IMPORTPATH);
			lvcol.pszText = ls2.m_szbuffer; // = "Import Path";
			lvcol.cx = 200;
			ListView_InsertColumn(GetDlgItem(hwndDlg, IDC_SOUNDLIST), 1, &lvcol);

			pt = (CCO(PinTable) *)GetWindowLong(hwndDlg, GWL_USERDATA);

			pt->ListFonts(GetDlgItem(hwndDlg, IDC_SOUNDLIST));

			return TRUE;
			}
			break;

		case WM_CLOSE:
			EndDialog(hwndDlg, FALSE);
			break;

		case WM_COMMAND:
			switch (HIWORD(wParam))
				{
				case BN_CLICKED:
					switch (LOWORD(wParam))
						{
						case IDOK:
							EndDialog(hwndDlg, TRUE);
							break;

						case IDCANCEL:
							EndDialog(hwndDlg, FALSE);
							break;

						case IDC_IMPORT:
							{
							char szFileName[1024];
							char szInitialDir[1024];
							
							szFileName[0] = '\0';

							OPENFILENAME ofn;
							memset(&ofn, 0, sizeof(OPENFILENAME));
							ofn.lStructSize = sizeof(OPENFILENAME);
							ofn.hInstance = g_hinst;
							ofn.hwndOwner = g_pvp->m_hwnd;
							// TEXT
							ofn.lpstrFilter = "Font Files (*.ttf)\0*.ttf\0";
							ofn.lpstrFile = szFileName;
							ofn.nMaxFile = _MAX_PATH;
							ofn.lpstrDefExt = "ttf";
							ofn.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;

							HRESULT hr = GetRegString("RecentDir","FontDir", szInitialDir, 1024);
							ofn.lpstrInitialDir = (hr == S_OK) ? szInitialDir : NULL;

							const int ret = GetOpenFileName(&ofn);
							if(ret)
								{
								strcpy_s(szInitialDir, sizeof(szInitialDir), szFileName);
								szInitialDir[ofn.nFileOffset] = 0;
								hr = SetRegValue("RecentDir","FontDir", REG_SZ, szInitialDir, strlen(szInitialDir));
								pt->ImportFont(GetDlgItem(hwndDlg, IDC_SOUNDLIST), ofn.lpstrFile);
								}
							}
							break;

						/*case IDC_RENAME:
							{
							int sel = ListView_GetNextItem(GetDlgItem(hwndDlg, IDC_SOUNDLIST), -1, LVNI_SELECTED);
							if (sel != -1)
								{
								SetFocus(GetDlgItem(hwndDlg, IDC_SOUNDLIST));
								HWND hwndFoo = ListView_EditLabel(GetDlgItem(hwndDlg, IDC_SOUNDLIST), sel);
								}
							}
							break;*/

						case IDC_DELETE:
							{
							const int sel = ListView_GetNextItem(GetDlgItem(hwndDlg, IDC_SOUNDLIST), -1, LVNI_SELECTED);
							if (sel != -1)
								{
								// TEXT
								const int ans = MessageBox(hwndDlg, "Are you sure you want to remove this font?", "Confirm Deletion", MB_YESNO | MB_DEFBUTTON2);
								if (ans == IDYES)
									{
									LVITEM lvitem;
									lvitem.mask = LVIF_PARAM;
									lvitem.iItem = sel;
									lvitem.iSubItem = 0;
									ListView_GetItem(GetDlgItem(hwndDlg, IDC_SOUNDLIST), &lvitem);
									PinFont * const ppf = (PinFont *)lvitem.lParam;
									pt->RemoveFont(ppf);
									ListView_DeleteItem(GetDlgItem(hwndDlg, IDC_SOUNDLIST), sel);
									}
								}
							}
							break;

						/*case IDC_REIMPORT:
							{
							const int ans = MessageBox(hwndDlg, "Are you sure you want to replace this image with a new one?", "Confirm Reimport", MB_YESNO | MB_DEFBUTTON2);
							if (ans == IDYES)
								{
								const int sel = ListView_GetNextItem(GetDlgItem(hwndDlg, IDC_SOUNDLIST), -1, LVNI_SELECTED);
								if (sel != -1)
									{
									LVITEM lvitem;
									lvitem.mask = LVIF_PARAM;
									lvitem.iItem = sel;
									lvitem.iSubItem = 0;
									ListView_GetItem(GetDlgItem(hwndDlg, IDC_SOUNDLIST), &lvitem);
									PinImage * const ppi = (PinImage *)lvitem.lParam;

									pt->ReImportImage(GetDlgItem(hwndDlg, IDC_SOUNDLIST), ppi, ppi->m_szPath);

									// Display new image
									InvalidateRect(GetDlgItem(hwndDlg, IDC_PICTUREPREVIEW), NULL, fTrue);
									}
								}
							}
							break;

						case IDC_REIMPORTFROM:
							{
							const int sel = ListView_GetNextItem(GetDlgItem(hwndDlg, IDC_SOUNDLIST), -1, LVNI_SELECTED);
							if (sel != -1)
								{
								char szFileName[1024];

								const int ans = MessageBox(hwndDlg, "Are you sure you want to replace this image with a new one?", "Confirm Reimport", MB_YESNO | MB_DEFBUTTON2);
								if (ans == IDYES)
									{
									szFileName[0] = '\0';

									OPENFILENAME ofn;
									memset(&ofn, 0, sizeof(OPENFILENAME));
									ofn.lStructSize = sizeof(OPENFILENAME);
									ofn.hInstance = g_hinst;
									ofn.hwndOwner = g_pvp->m_hwnd;
									ofn.lpstrFilter = "Font Files (*.ttf)\0*.ttf\0";
									ofn.lpstrFile = szFileName;
									ofn.nMaxFile = _MAX_PATH;
									ofn.lpstrDefExt = "ttf";
									ofn.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;

									const int ret = GetOpenFileName(&ofn);

									if(ret)
										{
										LVITEM lvitem;
										lvitem.mask = LVIF_PARAM;
										lvitem.iItem = sel;
										lvitem.iSubItem = 0;
										ListView_GetItem(GetDlgItem(hwndDlg, IDC_SOUNDLIST), &lvitem);
										PinImage * const ppi = (PinImage *)lvitem.lParam;

										pt->ReImportImage(GetDlgItem(hwndDlg, IDC_SOUNDLIST), ppi, ofn.lpstrFile);

										// Display new image
										InvalidateRect(GetDlgItem(hwndDlg, IDC_PICTUREPREVIEW), NULL, fTrue);
										}
									}
								}
							}
							break;*/
						}
					break;
				}
			break;
		}

	return FALSE;
	}

class CollectionDialogStruct
	{
public:
	Collection *pcol;
	PinTable *ppt;
	};

int CALLBACK CollectManagerProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
	CCO(PinTable) *pt = (CCO(PinTable) *)GetWindowLong(hwndDlg, GWL_USERDATA);

	switch (uMsg)
		{
		case WM_INITDIALOG:
			{
			SetWindowLong(hwndDlg, GWL_USERDATA, lParam);

			LVCOLUMN lvcol;
			lvcol.mask = LVCF_TEXT | LVCF_WIDTH;
			LocalString ls(IDS_NAME);
			lvcol.pszText = ls.m_szbuffer;// = "Name";
			lvcol.cx = 200;
			ListView_InsertColumn(GetDlgItem(hwndDlg, IDC_SOUNDLIST), 0, &lvcol);

			pt = (CCO(PinTable) *)GetWindowLong(hwndDlg, GWL_USERDATA);

			pt->ListCollections(GetDlgItem(hwndDlg, IDC_SOUNDLIST));

			return TRUE;
			}
			break;

		case WM_NOTIFY:
			{
			LPNMHDR pnmhdr = (LPNMHDR)lParam;
			switch (pnmhdr->code)
				{
				case LVN_ENDLABELEDIT:
					NMLVDISPINFO *pinfo = (NMLVDISPINFO *)lParam;
					if (pinfo->item.pszText == NULL || pinfo->item.pszText[0] == '\0')
						{
						return FALSE;
						}
					LVITEM lvitem;
					lvitem.mask = LVIF_PARAM;
					lvitem.iItem = pinfo->item.iItem;
					lvitem.iSubItem = 0;
					ListView_GetItem(GetDlgItem(hwndDlg, IDC_SOUNDLIST), &lvitem);
					Collection * const pcol = (Collection *)lvitem.lParam;
					//lstrcpy(pps->m_szName, pinfo->item.pszText);
					//lstrcpy(pps->m_szInternalName, pinfo->item.pszText);
					//CharLowerBuff(pps->m_szInternalName, lstrlen(pps->m_szInternalName));
					pt->SetCollectionName(pcol, pinfo->item.pszText, GetDlgItem(hwndDlg, IDC_SOUNDLIST), pinfo->item.iItem);
					pt->SetNonUndoableDirty(eSaveDirty);
					return TRUE;
					break;
				}
			}
			break;

		case WM_CLOSE:
			EndDialog(hwndDlg, FALSE);
			break;

		case WM_COMMAND:
			switch (HIWORD(wParam))
				{
				case BN_CLICKED:
					switch (LOWORD(wParam))
						{
						case IDOK:
							EndDialog(hwndDlg, TRUE);
							break;

						case IDCANCEL:
							EndDialog(hwndDlg, FALSE);
							break;

						case IDC_NEW:
							pt->NewCollection(GetDlgItem(hwndDlg, IDC_SOUNDLIST), fFalse);
							pt->SetNonUndoableDirty(eSaveDirty);
							break;

						case IDC_CREATEFROMSELECTION:
							pt->NewCollection(GetDlgItem(hwndDlg, IDC_SOUNDLIST), fTrue);
							pt->SetNonUndoableDirty(eSaveDirty);
							break;

						case IDC_EDIT:
							{
							const int sel = ListView_GetNextItem(GetDlgItem(hwndDlg, IDC_SOUNDLIST), -1, LVNI_SELECTED);
							if (sel != -1)
								{
								LVITEM lvitem;
								lvitem.mask = LVIF_PARAM;
								lvitem.iItem = sel;
								lvitem.iSubItem = 0;
								ListView_GetItem(GetDlgItem(hwndDlg, IDC_SOUNDLIST), &lvitem);
								CComObject<Collection> * const pcol = (CComObject<Collection> *)lvitem.lParam;

								CollectionDialogStruct cds;
								cds.pcol = pcol;
								cds.ppt = pt;

								const int ret = DialogBoxParam(g_hinstres, MAKEINTRESOURCE(IDD_COLLECTION),
									hwndDlg, CollectionProc, (long)&cds/*pcol*/);

								if (ret)
									{
									pt->SetNonUndoableDirty(eSaveDirty);
									}

								char szT[MAX_PATH];
								WideCharToMultiByte(CP_ACP, 0, pcol->m_wzName, -1, szT, MAX_PATH, NULL, NULL);
								ListView_SetItemText(GetDlgItem(hwndDlg, IDC_SOUNDLIST), sel, 0, szT);
								}
							}
							break;

						case IDC_RENAME:
							{
							const int sel = ListView_GetNextItem(GetDlgItem(hwndDlg, IDC_SOUNDLIST), -1, LVNI_SELECTED);
							if (sel != -1)
								{
								SetFocus(GetDlgItem(hwndDlg, IDC_SOUNDLIST));
								/*const HWND hwndFoo =*/ ListView_EditLabel(GetDlgItem(hwndDlg, IDC_SOUNDLIST), sel);
								}
							}
							break;

						case IDC_DELETE:
							{
							const int sel = ListView_GetNextItem(GetDlgItem(hwndDlg, IDC_SOUNDLIST), -1, LVNI_SELECTED);
							if (sel != -1)
								{
								// TEXT
								const int ans = MessageBox(hwndDlg, "Are you sure you want to remove this collection?", "Confirm Deletion", MB_YESNO | MB_DEFBUTTON2);
								if (ans == IDYES)
									{
									LVITEM lvitem;
									lvitem.mask = LVIF_PARAM;
									lvitem.iItem = sel;
									lvitem.iSubItem = 0;
									ListView_GetItem(GetDlgItem(hwndDlg, IDC_SOUNDLIST), &lvitem);
									CComObject<Collection> * const pcol = (CComObject<Collection> *)lvitem.lParam;
									pt->RemoveCollection(pcol);
									ListView_DeleteItem(GetDlgItem(hwndDlg, IDC_SOUNDLIST), sel);
									pt->SetNonUndoableDirty(eSaveDirty);
									}
								}
							}
							break;
						}
					break;
				}
			break;
		}

	return FALSE;
	}

int CALLBACK CollectionProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
	//CCO(PinTable) *pt;
	//pt = (CCO(PinTable) *)GetWindowLong(hwndDlg, GWL_USERDATA);

	switch (uMsg)
		{
		case WM_INITDIALOG:
			{
			CollectionDialogStruct * const pcds = (CollectionDialogStruct *)lParam;
			SetWindowLong(hwndDlg, GWL_USERDATA, lParam);

			Collection * const pcol = pcds->pcol;

			HWND hwndName = GetDlgItem(hwndDlg, IDC_NAME);

			char szT[MAX_PATH];
			WideCharToMultiByte(CP_ACP, 0, pcol->m_wzName, -1, szT, MAX_PATH, NULL, NULL);

			SetWindowText(hwndName, szT);

			HWND hwndFireEvents = GetDlgItem(hwndDlg, IDC_FIRE);
			SendMessage(hwndFireEvents, BM_SETCHECK, pcol->m_fFireEvents ? BST_CHECKED : BST_UNCHECKED, 0);

			HWND hwndStopSingle = GetDlgItem(hwndDlg, IDC_SUPPRESS);
			SendMessage(hwndStopSingle, BM_SETCHECK, pcol->m_fStopSingleEvents ? BST_CHECKED : BST_UNCHECKED, 0);

			HWND hwndOut = GetDlgItem(hwndDlg, IDC_OUTLIST);
			HWND hwndIn = GetDlgItem(hwndDlg, IDC_INLIST);

			for (int i=0;i<pcol->m_visel.Size();i++)
				{
				ISelect * const pisel = pcol->m_visel.ElementAt(i);
				IEditable * const piedit = pisel->GetIEditable();
				IScriptable * const piscript = piedit->GetScriptable();
				if (piscript)
					{
					WideCharToMultiByte(CP_ACP, 0, piscript->m_wzName, -1, szT, MAX_PATH, NULL, NULL);
					const int index = SendMessage(hwndIn, LB_ADDSTRING, 0, (long)szT);
					SendMessage(hwndIn, LB_SETITEMDATA, index, (long)piscript);
					}
				}

			PinTable * const ppt = pcds->ppt;

			for (int i=0;i<ppt->m_vedit.Size();i++)
				{
				IEditable * const piedit = ppt->m_vedit.ElementAt(i);
				IScriptable * const piscript = piedit->GetScriptable();
				ISelect * const pisel = piedit->GetISelect();

				// Only process objects not in this collection
				int l;
				for (l=0; l<pcol->m_visel.Size(); l++)
					{
					if (pisel == pcol->m_visel.ElementAt(l))
						{
						break;
						}
					}

				if ((l == pcol->m_visel.Size()) && piscript)
				//if (!piedit->m_pcollection)
					{
					WideCharToMultiByte(CP_ACP, 0, piscript->m_wzName, -1, szT, MAX_PATH, NULL, NULL);
					const int index = SendMessage(hwndOut, LB_ADDSTRING, 0, (long)szT);
					SendMessage(hwndOut, LB_SETITEMDATA, index, (long)piscript);
					}
				}

			return TRUE;
			}
			break;

		case WM_CLOSE:
			EndDialog(hwndDlg, FALSE);
			break;

		case WM_COMMAND:
			switch (HIWORD(wParam))
				{
				case BN_CLICKED:
					switch (LOWORD(wParam))
						{
						case IDC_UP:
						case IDC_DOWN:
							{
							// Mode items up or down in the collection list
							HWND hwndList = GetDlgItem(hwndDlg, IDC_INLIST);
							const int listsize = SendMessage(hwndList, LB_GETCOUNT, 0, 0);
							const int count = SendMessage(hwndList, LB_GETSELCOUNT, 0, 0);
							int * const rgsel = new int[count];
							SendMessage(hwndList, LB_GETSELITEMS, count, (LPARAM)rgsel);

							for (int loop=0;loop<count;loop++)
							//for (i=count-1;i>=0;i--)
								{
								const int i = (LOWORD(wParam) == IDC_UP) ? loop : (count - loop - 1);

								const int len = SendMessage(hwndList, LB_GETTEXTLEN, rgsel[i], 0);
								char * const szT = new char[len+1]; // include null terminator
								SendMessage(hwndList, LB_GETTEXT, rgsel[i], (LPARAM)szT);
								const int data = SendMessage(hwndList, LB_GETITEMDATA, rgsel[i], 0);

								const int newindex = (LOWORD(wParam) == IDC_UP) ? max(rgsel[i]-1, i) : min(rgsel[i]+2, listsize - (count - 1) + i);
								int oldindex = rgsel[i];

								if (oldindex > newindex)
									{
									oldindex++; // old item will be one lower when we try to delete it
									}

								const int index = SendMessage(hwndList, LB_INSERTSTRING, newindex, (LPARAM)szT);
								SendMessage(hwndList, LB_SETITEMDATA, index, data);
								// Set the new value to be selected, like the old one was
								SendMessage(hwndList, LB_SETSEL, TRUE, index);
								// Delete the old value
								SendMessage(hwndList, LB_DELETESTRING, oldindex, 0);
								delete szT;
								}
							}
							break;

						case IDC_IN:
						case IDC_OUT:
							{
							HWND hwndOut;
							HWND hwndIn;

							if (LOWORD(wParam) == IDC_IN)
								{
								hwndOut = GetDlgItem(hwndDlg, IDC_OUTLIST);
								hwndIn = GetDlgItem(hwndDlg, IDC_INLIST);
								}
							else
								{
								hwndOut = GetDlgItem(hwndDlg, IDC_INLIST);
								hwndIn = GetDlgItem(hwndDlg, IDC_OUTLIST);
								}

							const int count = SendMessage(hwndOut, LB_GETSELCOUNT, 0, 0);
							int * const rgsel = new int[count];
							SendMessage(hwndOut, LB_GETSELITEMS, count, (LPARAM)rgsel);
							for (int i=0;i<count;i++)
								{
								const int len = SendMessage(hwndOut, LB_GETTEXTLEN, rgsel[i], 0);
								char * const szT = new char[len+1]; // include null terminator
								SendMessage(hwndOut, LB_GETTEXT, rgsel[i], (LPARAM)szT);
								const int data = SendMessage(hwndOut, LB_GETITEMDATA, rgsel[i], 0);

								const int index = SendMessage(hwndIn, LB_ADDSTRING, 0, (LPARAM)szT);
								SendMessage(hwndIn, LB_SETITEMDATA, index, data);
								delete szT;
								}

							// Remove the old strings after everything else, to avoid messing up indices
							// Remove things in reverse order, so we don't get messed up inside this loop
							for (int i=0;i<count;i++)
								{
								SendMessage(hwndOut, LB_DELETESTRING, rgsel[count-i-1], 0);
								}

							delete rgsel;
							}
							break;

						case IDOK:
							{
							CollectionDialogStruct * const pcds = (CollectionDialogStruct *)GetWindowLong(hwndDlg, GWL_USERDATA);

							Collection * const pcol = pcds->pcol;

							for (int i=0;i<pcol->m_visel.Size();i++)
								{
								const int index = pcol->m_visel.ElementAt(i)->GetIEditable()->m_vCollection.IndexOf(pcol);
								if (index != -1)
									{
									pcol->m_visel.ElementAt(i)->GetIEditable()->m_vCollection.RemoveElementAt(index);
									pcol->m_visel.ElementAt(i)->GetIEditable()->m_viCollection.RemoveElementAt(index);
									}
								}

							pcol->m_visel.RemoveAllElements();

							HWND hwndIn = GetDlgItem(hwndDlg, IDC_INLIST);

							const int count = SendMessage(hwndIn, LB_GETCOUNT, 0, 0);

							for (int i=0;i<count;i++)
								{
								IScriptable * const piscript = (IScriptable *)SendMessage(hwndIn, LB_GETITEMDATA, i, 0);
								ISelect * const pisel = piscript->GetISelect();								
								if (pisel) // Not sure how we could possibly get an iscript here that was never an iselect
									{
									pcol->m_visel.AddElement(pisel);
									pisel->GetIEditable()->m_vCollection.AddElement(pcol);
									pisel->GetIEditable()->m_viCollection.AddElement((void *)i);
									}
								}

							HWND hwndFireEvents = GetDlgItem(hwndDlg, IDC_FIRE);
							const int fEvents = SendMessage(hwndFireEvents, BM_GETCHECK, 0, 0);
							pcol->m_fFireEvents = fEvents;

							HWND hwndStopSingle = GetDlgItem(hwndDlg, IDC_SUPPRESS);
							const int fStopSingle = SendMessage(hwndStopSingle, BM_GETCHECK, 0, 0);
							pcol->m_fStopSingleEvents = fStopSingle;

							char szT[1024];
							HWND hwndName = GetDlgItem(hwndDlg, IDC_NAME);
							GetWindowText(hwndName, szT, 1024);

							pcds->ppt->SetCollectionName(pcol, szT, NULL, 0);

							EndDialog(hwndDlg, TRUE);
							}
							break;

						case IDCANCEL:
							EndDialog(hwndDlg, FALSE);
							break;
						}
					break;
				}
			break;
		}

	return FALSE;
	}

void VPGetDialogItemText(HWND hDlg, int nIDDlgItem, char **psztext)
	{
	HWND hwndItem = GetDlgItem(hDlg, nIDDlgItem);

	const int length = GetWindowTextLength(hwndItem);
	*psztext = new char[length+1];

	GetWindowText(hwndItem, *psztext, length+1);
	}

int CALLBACK TableInfoProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
	CCO(PinTable) *pt = (CCO(PinTable) *)GetWindowLong(hwndDlg, GWL_USERDATA);

	switch (uMsg)
		{
		case WM_INITDIALOG:
			{
			SetWindowLong(hwndDlg, GWL_USERDATA, lParam);
			pt = (CCO(PinTable) *)GetWindowLong(hwndDlg, GWL_USERDATA);

			HWND hwndParent = GetParent(hwndDlg);
			RECT rcDlg;
			RECT rcMain;
			GetWindowRect(hwndParent, &rcMain);
			GetWindowRect(hwndDlg, &rcDlg);

			SetWindowPos(hwndDlg, NULL,
				(rcMain.right + rcMain.left)/2 - (rcDlg.right - rcDlg.left)/2,
				(rcMain.bottom + rcMain.top)/2 - (rcDlg.bottom - rcDlg.top)/2,
				0, 0, SWP_NOOWNERZORDER | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE/* | SWP_NOMOVE*/);

			SendDlgItemMessage(hwndDlg, IDC_BLURB, EM_LIMITTEXT, 100, 0);

			SetDlgItemText(hwndDlg, IDC_TABLENAME, pt->m_szTableName);
			SetDlgItemText(hwndDlg, IDC_TABLEAUTHOR, pt->m_szAuthor);
			SetDlgItemText(hwndDlg, IDC_VERSION, pt->m_szVersion);
			SetDlgItemText(hwndDlg, IDC_RELEASE, pt->m_szReleaseDate);
			SetDlgItemText(hwndDlg, IDC_EMAIL, pt->m_szAuthorEMail);
			SetDlgItemText(hwndDlg, IDC_WEBSITE, pt->m_szWebSite);
			SetDlgItemText(hwndDlg, IDC_BLURB, pt->m_szBlurb);
			SetDlgItemText(hwndDlg, IDC_DESCRIPTION, pt->m_szDescription);
			SetDlgItemText(hwndDlg, IDC_RULES, pt->m_szRules);

			// Init list of images

			HWND hwndList = GetDlgItem(hwndDlg, IDC_SCREENSHOT);

			LocalString ls(IDS_NONE);
			SendMessage(hwndList, CB_ADDSTRING, 0, (LPARAM)ls.m_szbuffer);

			for (int i=0;i<pt->m_vimage.Size();i++)
				{
				PinImage * const pin = pt->m_vimage.ElementAt(i);
				if (pin->m_ppb)
					{
					SendMessage(hwndList, CB_ADDSTRING, 0, (LPARAM)pin->m_szName);
					}
				}

			SendMessage(hwndList, CB_SELECTSTRING, ~0u, (LPARAM)pt->m_szScreenShot);

			// Set up custom info list
				{
				LVCOLUMN lvcol;
				lvcol.mask = LVCF_TEXT | LVCF_WIDTH;
				LocalString ls3(IDS_NAME);
				lvcol.pszText = ls3.m_szbuffer;// = "Name";
				lvcol.cx = 90;
				ListView_InsertColumn(GetDlgItem(hwndDlg, IDC_CUSTOMLIST), 0, &lvcol);

				LocalString ls2(IDS_VALUE);
				lvcol.pszText = ls2.m_szbuffer; // = "Value";
				lvcol.cx = 100;
				ListView_InsertColumn(GetDlgItem(hwndDlg, IDC_CUSTOMLIST), 1, &lvcol);

				pt->ListCustomInfo(GetDlgItem(hwndDlg, IDC_CUSTOMLIST));
				}

			return TRUE;
			}
			break;

		case WM_NOTIFY:
			{
			LPNMHDR pnmhdr = (LPNMHDR)lParam;
			switch (pnmhdr->code)
				{
				case LVN_ITEMCHANGING:
					NMLISTVIEW * const plistview = (LPNMLISTVIEW)lParam;
					if ((plistview->uNewState & LVIS_SELECTED) != (plistview->uOldState & LVIS_SELECTED))
						{
						if (plistview->uNewState & LVIS_SELECTED)
							{
							const int sel = plistview->iItem;
							char szT[1024];
	
							ListView_GetItemText(GetDlgItem(hwndDlg, IDC_CUSTOMLIST), sel, 0, szT, 1024);
							SetWindowText(GetDlgItem(hwndDlg, IDC_CUSTOMNAME), szT);

							ListView_GetItemText(GetDlgItem(hwndDlg, IDC_CUSTOMLIST), sel, 1, szT, 1024);
							SetWindowText(GetDlgItem(hwndDlg, IDC_CUSTOMVALUE), szT);
							}
						}
					break;
				}
			}
			break;

		case WM_CLOSE:
			EndDialog(hwndDlg, FALSE);
			break;

		case WM_COMMAND:
			switch (HIWORD(wParam))
				{
				case BN_CLICKED:
					switch (LOWORD(wParam))
						{
						case IDC_ADD:
							{
							char *szCustomName;
							VPGetDialogItemText(hwndDlg, IDC_CUSTOMNAME, &szCustomName);
							if (lstrlen(szCustomName) > 0)
								{
								LVFINDINFO lvfi;							
								lvfi.flags = LVFI_STRING;
								lvfi.psz = szCustomName;

								const int found = ListView_FindItem(GetDlgItem(hwndDlg, IDC_CUSTOMLIST), -1, &lvfi);

								if (found != -1)
									{
									ListView_DeleteItem(GetDlgItem(hwndDlg, IDC_CUSTOMLIST), found);
									}

								char *szCustomValue;
								VPGetDialogItemText(hwndDlg, IDC_CUSTOMVALUE, &szCustomValue);
								pt->AddListItem(GetDlgItem(hwndDlg, IDC_CUSTOMLIST), szCustomName, szCustomValue, NULL);
								delete szCustomValue;
								}
							delete szCustomName;
							}
							break;

						case IDC_DELETE:
							{
							const int sel = ListView_GetNextItem(GetDlgItem(hwndDlg, IDC_CUSTOMLIST), -1, LVNI_SELECTED);
							ListView_DeleteItem(GetDlgItem(hwndDlg, IDC_CUSTOMLIST), sel);
							}
							break;

						case IDOK:
							{
							SAFE_DELETE(pt->m_szTableName);
							SAFE_DELETE(pt->m_szAuthor);
							SAFE_DELETE(pt->m_szVersion);
							SAFE_DELETE(pt->m_szReleaseDate);
							SAFE_DELETE(pt->m_szAuthorEMail);
							SAFE_DELETE(pt->m_szWebSite);
							SAFE_DELETE(pt->m_szBlurb);
							SAFE_DELETE(pt->m_szDescription);
							SAFE_DELETE(pt->m_szRules);

							VPGetDialogItemText(hwndDlg, IDC_TABLENAME, &pt->m_szTableName);
							VPGetDialogItemText(hwndDlg, IDC_TABLEAUTHOR, &pt->m_szAuthor);
							VPGetDialogItemText(hwndDlg, IDC_VERSION, &pt->m_szVersion);
							VPGetDialogItemText(hwndDlg, IDC_RELEASE, &pt->m_szReleaseDate);
							VPGetDialogItemText(hwndDlg, IDC_EMAIL, &pt->m_szAuthorEMail);
							VPGetDialogItemText(hwndDlg, IDC_WEBSITE, &pt->m_szWebSite);
							VPGetDialogItemText(hwndDlg, IDC_BLURB, &pt->m_szBlurb);
							VPGetDialogItemText(hwndDlg, IDC_DESCRIPTION, &pt->m_szDescription);
							VPGetDialogItemText(hwndDlg, IDC_RULES, &pt->m_szRules);
							
							HWND hwndList = GetDlgItem(hwndDlg, IDC_SCREENSHOT);

							GetWindowText(hwndList, pt->m_szScreenShot, MAXTOKEN);
							
							LocalString ls(IDS_NONE);
							if (!lstrcmp(pt->m_szScreenShot, ls.m_szbuffer))
								{
								// <None> is selected
								pt->m_szScreenShot[0] = '\0';
								}

							// Clear old custom values, read back new ones
							for (int i=0;i<pt->m_vCustomInfoTag.Size();i++)
								{
								delete pt->m_vCustomInfoTag.ElementAt(i);
								delete pt->m_vCustomInfoContent.ElementAt(i);
								}
							pt->m_vCustomInfoTag.RemoveAllElements();
							pt->m_vCustomInfoContent.RemoveAllElements();

							const int customcount = ListView_GetItemCount(GetDlgItem(hwndDlg, IDC_CUSTOMLIST));
							for (int i=0;i<customcount;i++)
								{
								char szT[1024];
								ListView_GetItemText(GetDlgItem(hwndDlg, IDC_CUSTOMLIST), i, 0, szT, 1024);

								char * const szName = new char[lstrlen(szT) + 1];
								lstrcpy(szName, szT);
								pt->m_vCustomInfoTag.AddElement(szName);

								ListView_GetItemText(GetDlgItem(hwndDlg, IDC_CUSTOMLIST), i, 1, szT, 1024);
								char * const szValue = new char[lstrlen(szT) + 1];
								lstrcpy(szValue, szT);
								pt->m_vCustomInfoContent.AddElement(szValue);
								}

							pt->SetNonUndoableDirty(eSaveDirty);

							EndDialog(hwndDlg, TRUE);
							}
							break;

						case IDCANCEL:
							EndDialog(hwndDlg, FALSE);
							break;

						case IDC_GOWEBSITE:
							{
							char *szT;
							VPGetDialogItemText(hwndDlg, IDC_WEBSITE, &szT);
							OpenURL(szT);
							delete szT;
							}
							break;

						case IDC_SENDMAIL:
							{
							char *szEMail;
							char *szTableName;
							char szMail[] = "mailto:";
							char szHeaders[] = "?subject=";
							VPGetDialogItemText(hwndDlg, IDC_EMAIL, &szEMail);
							VPGetDialogItemText(hwndDlg, IDC_TABLENAME, &szTableName);
							char * const szLong = new char[lstrlen(szMail) + lstrlen(szEMail) + lstrlen(szHeaders) + lstrlen(szTableName) + 1];
							lstrcpy(szLong, szMail);
							lstrcat(szLong, szEMail);
							lstrcat(szLong, szHeaders);
							lstrcat(szLong, szTableName);
							OpenURL(szLong);
							delete szLong;
							delete szEMail;
							delete szTableName;
							}
							break;
						}
					break;
				}
			break;
		}

	return FALSE;
	}

const char rgszKeyName[][10] = {
	"",
	"Escape", //DIK_ESCAPE          0x01
	"1", //DIK_1               0x02
	"2", //DIK_2               0x03
	"3", //DIK_3               0x04
	"4", //DIK_4               0x05
	"5", //DIK_5               0x06
	"6", //DIK_6               0x07
	"7", //DIK_7               0x08
	"8", //DIK_8               0x09
	"9", //DIK_9               0x0A
	"0", //DIK_0               0x0B
	"-", //DIK_MINUS           0x0C    /* - on main keyboard */
	"=", //DIK_EQUALS          0x0D
	"Backspace", //DIK_BACK            0x0E    /* backspace */
	"Tab", //DIK_TAB             0x0F
	"Q", //DIK_Q               0x10
	"W", //DIK_W               0x11
	"E", //DIK_E               0x12
	"R", //DIK_R               0x13
	"T", //DIK_T               0x14
	"Y", //DIK_Y               0x15
	"U", //DIK_U               0x16
	"I", //DIK_I               0x17
	"O", //DIK_O               0x18
	"P", //DIK_P               0x19
	"[", //DIK_LBRACKET        0x1A
	"]", //DIK_RBRACKET        0x1B
	"Enter", //DIK_RETURN          0x1C    /* Enter on main keyboard */
	"L Ctrl", //DIK_LCONTROL        0x1D
	"A", //DIK_A               0x1E
	"S", //DIK_S               0x1F
	"D", //DIK_D               0x20
	"F", //DIK_F               0x21
	"G", //DIK_G               0x22
	"H", //DIK_H               0x23
	"J", //DIK_J               0x24
	"K", //DIK_K               0x25
	"L", //DIK_L               0x26
	";", //DIK_SEMICOLON       0x27
	"'", //DIK_APOSTROPHE      0x28
	"`", //DIK_GRAVE           0x29    /* accent grave */
	"L Shift", //DIK_LSHIFT          0x2A
	"\\", //DIK_BACKSLASH       0x2B
	"Z", //DIK_Z               0x2C
	"X", //DIK_X               0x2D
	"C", //DIK_C               0x2E
	"V", //DIK_V               0x2F
	"B", //DIK_B               0x30
	"N", //DIK_N               0x31
	"M", //DIK_M               0x32
	",", //DIK_COMMA           0x33
	".", //DIK_PERIOD          0x34    /* . on main keyboard */
	"/", //DIK_SLASH           0x35    /* / on main keyboard */
	"R Shift", //DIK_RSHIFT          0x36
	"*", //DIK_MULTIPLY        0x37    /* * on numeric keypad */
	"Menu", //DIK_LMENU           0x38    /* left Alt */
	"Space", //DIK_SPACE           0x39
	"Caps Lock", //DIK_CAPITAL         0x3A
	"F1", //DIK_F1              0x3B
	"F2", //DIK_F2              0x3C
	"F3", //DIK_F3              0x3D
	"F4", //DIK_F4              0x3E
	"F5", //DIK_F5              0x3F
	"F6", //DIK_F6              0x40
	"F7", //DIK_F7              0x41
	"F8", //DIK_F8              0x42
	"F9", //DIK_F9              0x43
	"F10", //DIK_F10             0x44
	"NumLock", //DIK_NUMLOCK         0x45
	"ScrollLck", //DIK_SCROLL          0x46    /* Scroll Lock */
	"Numpad 7", //DIK_NUMPAD7         0x47
	"Numpad 8", //DIK_NUMPAD8         0x48
	"Numpad 9", //DIK_NUMPAD9         0x49
	"-", //DIK_SUBTRACT        0x4A    /* - on numeric keypad */
	"Numpad 4", //DIK_NUMPAD4         0x4B
	"Numpad 5", //DIK_NUMPAD5         0x4C
	"Numpad 6", //DIK_NUMPAD6         0x4D
	"+", //DIK_ADD             0x4E    /* + on numeric keypad */
	"Numpad 1", //DIK_NUMPAD1         0x4F
	"Numpad 2", //DIK_NUMPAD2         0x50
	"Numpad 3", //DIK_NUMPAD3         0x51
	"Numpad 0", //DIK_NUMPAD0         0x52
	"Numpad .", //DIK_DECIMAL         0x53    /* . on numeric keypad */
	"", //0x54
	"", //0x55
	"<>|", //DIK_OEM_102         0x56    /* < > | on UK/Germany keyboards */
	"F11", //DIK_F11             0x57
	"F12", //DIK_F12             0x58
	"", //0x59
	"", //0x5A
	"", //0x5B
	"", //0x5C
	"", //0x5D
	"", //0x5E
	"", //0x5F
	"", //0x60
	"", //0x61
	"", //0x62
	"", //0x63
	"F13", //DIK_F13             0x64    /*                     (NEC PC98) */
	"F14", //DIK_F14             0x65    /*                     (NEC PC98) */
	"F15", //DIK_F15             0x66    /*                     (NEC PC98) */
	"", //0x67
	"", //0x68
	"", //0x69
	"", //0x6A
	"", //0x6B
	"", //0x6C
	"", //0x6D
	"", //0x6E
	"", //0x6F

	"", //0x70
	"", //0x71
	"", //0x72
	"", //0x73
	"", //0x74
	"", //0x75
	"", //0x76
	"", //0x77
	"", //0x78
	"", //0x79
	"", //0x7A
	"", //0x7B
	"", //0x7C
	"", //0x7D
	"", //0x7E
	"", //0x7F

	"", //0x80
	"", //0x81
	"", //0x82
	"", //0x83
	"", //0x84
	"", //0x85
	"", //0x86
	"", //0x87
	"", //0x88
	"", //0x89
	"", //0x8A
	"", //0x8B
	"", //0x8C
	"", //0x8D
	"", //0x8E
	"", //0x8F

	"", //0x90
	"", //0x91
	"", //0x92
	"", //0x93
	"", //0x94
	"", //0x95
	"", //0x96
	"", //0x97
	"", //0x98
	"", //0x99
	"", //0x9A
	"", //0x9B

	"Num Enter",	//#define DIK_NUMPADENTER     0x9C    /* Enter on numeric keypad */
	"R Ctrl",		//DIK_RCONTROL        0x9D

	"", //0x9E
	"", //0x9F

	"", //0xA0
	"", //0xA1
	"", //0xA2
	"", //0xA3
	"", //0xA4
	"", //0xA5
	"", //0xA6
	"", //0xA7
	"", //0xA8
	"", //0xA9
	"", //0xAA
	"", //0xAB
	"", //0xAC
	"", //0xAD
	"", //0xAE
	"", //0xAF

	"", //0xB0
	"", //0xB1
	"", //0xB2
	"", //0xB3
	"", //0xB4

	"Numpad /", //#define DIK_DIVIDE          0xB5    /* / on numeric keypad */
	"", //0xB6
	"", //#define DIK_SYSRQ           0xB7
	"R Alt", //DIK_RMENU           0xB8    /* right Alt */

	"", //0xB9
	"", //0xBA
	"", //0xBB
	"", //0xBC
	"", //0xBD
	"", //0xBE
	"", //0xBF

	"", //0xC0
	"", //0xC1
	"", //0xC2
	"", //0xC3
	"", //0xC4
	"", //0xC5
	"", //0xC6

	"Home", //DIK_HOME            0xC7    /* Home on arrow keypad */
	"Up", //DIK_UP              0xC8    /* UpArrow on arrow keypad */
	"Page Up", //DIK_PRIOR           0xC9    /* PgUp on arrow keypad */
	"", //0xCA
	"Left", //DIK_LEFT            0xCB    /* LeftArrow on arrow keypad */
	"", //0xCC
	"Right", //DIK_RIGHT           0xCD    /* RightArrow on arrow keypad */
	"", //0xCE
	"End", //DIK_END             0xCF    /* End on arrow keypad */
	"Down", //DIK_DOWN            0xD0    /* DownArrow on arrow keypad */
	"Page Down", //DIK_NEXT            0xD1    /* PgDn on arrow keypad */
	"Insert", //DIK_INSERT          0xD2    /* Insert on arrow keypad */
	"Delete", //DIK_DELETE          0xD3    /* Delete on arrow keypad */

	"", //0xD4
	"", //0xD5
	"", //0xD6
	"", //0xD7
	"", //0xD8
	"", //0xD9
	"", //0xDA

	"L Windows", //DIK_LWIN            0xDB    /* Left Windows key */
	"R Windows", //DIK_RWIN            0xDC    /* Right Windows key */
	"Apps Menu", //DIK_APPS            0xDD    /* AppMenu key */
	};


class KeyWindowStruct
	{
public:
	PinInput pi;
	HWND hwndKeyControl; // window to get the key assignment
	UINT_PTR m_timerid; // timer id for our key assignment
	};

WNDPROC g_ButtonProc;

LRESULT CALLBACK MyKeyButtonProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
	switch (uMsg)
		{
		case WM_GETDLGCODE:
			// Eat all acceleratable messges
			return DLGC_WANTARROWS | DLGC_WANTTAB | DLGC_WANTALLKEYS | DLGC_WANTCHARS;
			break;
		}

	return CallWindowProc(g_ButtonProc, hwnd, uMsg, wParam, lParam);
	}

int CALLBACK KeysProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
	//CCO(PinTable) *pt;
	//pt = (CCO(PinTable) *)GetWindowLong(hwndDlg, GWL_USERDATA);
//	KeyWindowStruct *pksw;//moved to expand scope
//	pksw = new KeyWindowStruct();// moved to expand scope

	switch (uMsg)
		{
		case WM_INITDIALOG:
			{
			HWND hwndParent = GetParent(hwndDlg);
			RECT rcDlg;
			RECT rcMain;
			GetWindowRect(hwndParent, &rcMain);
			GetWindowRect(hwndDlg, &rcDlg);

			SetWindowPos(hwndDlg, NULL,
				(rcMain.right + rcMain.left)/2 - (rcDlg.right - rcDlg.left)/2,
				(rcMain.bottom + rcMain.top)/2 - (rcDlg.bottom - rcDlg.top)/2,
				0, 0, SWP_NOOWNERZORDER | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE/* | SWP_NOMOVE*/);

			HRESULT hr;
			HWND hwndControl;
			int key;

			hr = GetRegInt("Player","LFlipKey", &key);
			if (hr != S_OK || key > 0xdd)
				{
				key = DIK_LSHIFT;
				}
			hwndControl = GetDlgItem(hwndDlg, IDC_LEFTFLIPPER);
			SetWindowText(hwndControl, rgszKeyName[key]);
			SetWindowLong(hwndControl, GWL_USERDATA, key);

			hr = GetRegInt("Player","RFlipKey", &key);
			if (hr != S_OK || key > 0xdd)
				{
				key = DIK_RSHIFT;
				}
			hwndControl = GetDlgItem(hwndDlg, IDC_RIGHTFLIPPER);
			SetWindowText(hwndControl, rgszKeyName[key]);
			SetWindowLong(hwndControl, GWL_USERDATA, key);

			hr = GetRegInt("Player","LTiltKey", &key);
			if (hr != S_OK || key > 0xdd)
				{
				key = DIK_Z;
				}
			hwndControl = GetDlgItem(hwndDlg, IDC_LEFTTILT);
			SetWindowText(hwndControl, rgszKeyName[key]);
			SetWindowLong(hwndControl, GWL_USERDATA, key);

			hr = GetRegInt("Player","RTiltKey", &key);
			if (hr != S_OK || key > 0xdd)
				{
				key = DIK_SLASH;
				}
			hwndControl = GetDlgItem(hwndDlg, IDC_RIGHTTILT);
			SetWindowText(hwndControl, rgszKeyName[key]);
			SetWindowLong(hwndControl, GWL_USERDATA, key);

			hr = GetRegInt("Player","CTiltKey", &key);
			if (hr != S_OK || key > 0xdd)
				{
				key = DIK_SPACE;
				}
			hwndControl = GetDlgItem(hwndDlg, IDC_CENTERTILT);
			SetWindowText(hwndControl, rgszKeyName[key]);
			SetWindowLong(hwndControl, GWL_USERDATA, key);

			hr = GetRegInt("Player","PlungerKey", &key);
			if (hr != S_OK || key > 0xdd)
				{
				key = DIK_RETURN;
				}
			hwndControl = GetDlgItem(hwndDlg, IDC_PLUNGER);
			SetWindowText(hwndControl, rgszKeyName[key]);
			SetWindowLong(hwndControl, GWL_USERDATA, key);

			hr = GetRegInt("Player","AddCreditKey", &key);
			if (hr != S_OK || key > 0xdd)
				{
				key = DIK_5;
				}
			hwndControl = GetDlgItem(hwndDlg, IDC_ADDCREDIT);
			SetWindowText(hwndControl, rgszKeyName[key]);
			SetWindowLong(hwndControl, GWL_USERDATA, key);

			hr = GetRegInt("Player","AddCreditKey2", &key);
			if (hr != S_OK || key > 0xdd)
				{
				key = DIK_4;
				}
			hwndControl = GetDlgItem(hwndDlg, IDC_ADDCREDITKEY2);
			SetWindowText(hwndControl, rgszKeyName[key]);
			SetWindowLong(hwndControl, GWL_USERDATA, key);

			hr = GetRegInt("Player","StartGameKey", &key);
			if (hr != S_OK || key > 0xdd)
				{
				key = DIK_1;
				}
			hwndControl = GetDlgItem(hwndDlg, IDC_STARTGAME);
			SetWindowText(hwndControl, rgszKeyName[key]);
			SetWindowLong(hwndControl, GWL_USERDATA, key);

			hr = GetRegInt("Player","ExitGameKey", &key);
			if (hr != S_OK || key > 0xdd)
				{
				key = DIK_Q;
				}
			hwndControl = GetDlgItem(hwndDlg, IDC_EXITGAME);
			SetWindowText(hwndControl, rgszKeyName[key]);
			SetWindowLong(hwndControl, GWL_USERDATA, key);

			hr = GetRegInt("Player","FrameCount", &key);
			if (hr != S_OK || key > 0xdd)
				{
				key = DIK_F11;
				}
			hwndControl = GetDlgItem(hwndDlg, IDC_FRAMECOUNT);
			SetWindowText(hwndControl, rgszKeyName[key]);
			SetWindowLong(hwndControl, GWL_USERDATA, key);

			hr = GetRegInt("Player","VolumeUp", &key);
			if (hr != S_OK || key > 0xdd)
				{
				key = DIK_MINUS;
				}
			hwndControl = GetDlgItem(hwndDlg, IDC_VOLUMEUP);
			SetWindowText(hwndControl, rgszKeyName[key]);
			SetWindowLong(hwndControl, GWL_USERDATA, key);

			hr = GetRegInt("Player","VolumeDown", &key);
			if (hr != S_OK || key > 0xdd)
				{
				key = DIK_EQUALS;
				}
			hwndControl = GetDlgItem(hwndDlg, IDC_VOLUMEDN);
			SetWindowText(hwndControl, rgszKeyName[key]);
			SetWindowLong(hwndControl, GWL_USERDATA, key);

			hr = GetRegInt("Player","DebugBalls", &key);
			if (hr != S_OK || key > 0xdd)
				{
				key = DIK_O;
				}
			hwndControl = GetDlgItem(hwndDlg, IDC_DEBUGBALL);
			SetWindowText(hwndControl, rgszKeyName[key]);
			SetWindowLong(hwndControl, GWL_USERDATA, key);

			hr = GetRegInt("Player","RMagnaSave", &key);
			if (hr != S_OK || key > 0xdd)
				{
				//key = DIK_BACKSPACE;
				key = DIK_RCONTROL;		//157 (0x9D) DIK_RCONTROL 
				}
			hwndControl = GetDlgItem(hwndDlg, IDC_RMAGSAVE);
			SetWindowText(hwndControl, rgszKeyName[key]);
			SetWindowLong(hwndControl, GWL_USERDATA, key);

			hr = GetRegInt("Player","LMagnaSave", &key);
			if (hr != S_OK || key > 0xdd)
				{
				//key = DIK_APOSTROPHE;
				key = DIK_LCONTROL; //29 (0x1D)
				}
			hwndControl = GetDlgItem(hwndDlg, IDC_LMAGSAVE );
			SetWindowText(hwndControl, rgszKeyName[key]);
			SetWindowLong(hwndControl, GWL_USERDATA, key);

			hr = GetRegInt("Player","MechTilt", &key);
			if (hr != S_OK || key > 0xdd)
				{
				key = DIK_T;
				}
			hwndControl = GetDlgItem(hwndDlg, IDC_MECHTILT );
			SetWindowText(hwndControl, rgszKeyName[key]);
			SetWindowLong(hwndControl, GWL_USERDATA, key);


			KeyWindowStruct * const pksw = new KeyWindowStruct();
			pksw->pi.Init(hwndDlg);
			pksw->m_timerid = 0;
			SetWindowLong(hwndDlg, GWL_USERDATA, (long)pksw);

				// Set buttons to ignore keyboard shortcuts when using DirectInput
				HWND hwndButton;
				hwndButton = GetDlgItem(hwndDlg, IDC_LEFTFLIPPERBUTTON);
				g_ButtonProc = (WNDPROC)GetWindowLong(hwndButton, GWL_WNDPROC);
				SetWindowLong(hwndButton, GWL_WNDPROC, (long)MyKeyButtonProc);
				SetWindowLong(hwndButton, GWL_USERDATA, (long)pksw);

				hwndButton = GetDlgItem(hwndDlg, IDC_RIGHTFLIPPERBUTTON);
				SetWindowLong(hwndButton, GWL_WNDPROC, (long)MyKeyButtonProc);
				SetWindowLong(hwndButton, GWL_USERDATA, (long)pksw);

				hwndButton = GetDlgItem(hwndDlg, IDC_LEFTTILTBUTTON);
				SetWindowLong(hwndButton, GWL_WNDPROC, (long)MyKeyButtonProc);
				SetWindowLong(hwndButton, GWL_USERDATA, (long)pksw);

				hwndButton = GetDlgItem(hwndDlg, IDC_RIGHTTILTBUTTON);
				SetWindowLong(hwndButton, GWL_WNDPROC, (long)MyKeyButtonProc);
				SetWindowLong(hwndButton, GWL_USERDATA, (long)pksw);

				hwndButton = GetDlgItem(hwndDlg, IDC_CENTERTILTBUTTON);
				SetWindowLong(hwndButton, GWL_WNDPROC, (long)MyKeyButtonProc);
				SetWindowLong(hwndButton, GWL_USERDATA, (long)pksw);

				hwndButton = GetDlgItem(hwndDlg, IDC_PLUNGERBUTTON);
				SetWindowLong(hwndButton, GWL_WNDPROC, (long)MyKeyButtonProc);
				SetWindowLong(hwndButton, GWL_USERDATA, (long)pksw);

				hwndButton = GetDlgItem(hwndDlg, IDC_ADDCREDITBUTTON);
				SetWindowLong(hwndButton, GWL_WNDPROC, (long)MyKeyButtonProc);
				SetWindowLong(hwndButton, GWL_USERDATA, (long)pksw);

				hwndButton = GetDlgItem(hwndDlg, IDC_ADDCREDITBUTTON2);
				SetWindowLong(hwndButton, GWL_WNDPROC, (long)MyKeyButtonProc);
				SetWindowLong(hwndButton, GWL_USERDATA, (long)pksw);

				hwndButton = GetDlgItem(hwndDlg, IDC_STARTGAMEBUTTON);
				SetWindowLong(hwndButton, GWL_WNDPROC, (long)MyKeyButtonProc);
				SetWindowLong(hwndButton, GWL_USERDATA, (long)pksw);

				hwndButton = GetDlgItem(hwndDlg, IDC_EXITGAMEBUTTON);
				SetWindowLong(hwndButton, GWL_WNDPROC, (long)MyKeyButtonProc);
				SetWindowLong(hwndButton, GWL_USERDATA, (long)pksw);

				hwndButton = GetDlgItem(hwndDlg, IDC_FRAMECOUNTBUTTON);
				SetWindowLong(hwndButton, GWL_WNDPROC, (long)MyKeyButtonProc);
				SetWindowLong(hwndButton, GWL_USERDATA, (long)pksw);

				hwndButton = GetDlgItem(hwndDlg, IDC_VOLUPBUTTON);
				SetWindowLong(hwndButton, GWL_WNDPROC, (long)MyKeyButtonProc);
				SetWindowLong(hwndButton, GWL_USERDATA, (long)pksw);

				hwndButton = GetDlgItem(hwndDlg, IDC_VOLDOWNBUTTON);
				SetWindowLong(hwndButton, GWL_WNDPROC, (long)MyKeyButtonProc);
				SetWindowLong(hwndButton, GWL_USERDATA, (long)pksw);

				hwndButton = GetDlgItem(hwndDlg, IDC_DEBUGBALLSBUTTON);
				SetWindowLong(hwndButton, GWL_WNDPROC, (long)MyKeyButtonProc);
				SetWindowLong(hwndButton, GWL_USERDATA, (long)pksw);

				hwndButton = GetDlgItem(hwndDlg, IDC_RMAGSAVEBUTTON);
				SetWindowLong(hwndButton, GWL_WNDPROC, (long)MyKeyButtonProc);
				SetWindowLong(hwndButton, GWL_USERDATA, (long)pksw);

				hwndButton = GetDlgItem(hwndDlg, IDC_LMAGSAVEBUTTON);
				SetWindowLong(hwndButton, GWL_WNDPROC, (long)MyKeyButtonProc);
				SetWindowLong(hwndButton, GWL_USERDATA, (long)pksw);

				hwndButton = GetDlgItem(hwndDlg, IDC_MECHTILTBUTTON);
				SetWindowLong(hwndButton, GWL_WNDPROC, (long)MyKeyButtonProc);
				SetWindowLong(hwndButton, GWL_USERDATA, (long)pksw);

			return TRUE;
			}
			break;

		case WM_DESTROY:
			{
			KeyWindowStruct * const pksw = (KeyWindowStruct *)GetWindowLong(hwndDlg, GWL_USERDATA);
			if (pksw->m_timerid)
				{
				KillTimer(hwndDlg, pksw->m_timerid);
				pksw->m_timerid = 0;
				}
			pksw->pi.UnInit();
			}
			break;

		case WM_TIMER:
			{
			KeyWindowStruct * const pksw = (KeyWindowStruct *)GetWindowLong(hwndDlg, GWL_USERDATA);
			const int key = pksw->pi.GetNextKey();
			if (key != 0)
				{
				if(key < 0xDD)	//0xDD		//rlc   Key mapping, add cases for joystick here!!!!!!!!!
					{
					if (key == DIK_ESCAPE)
						{
						// reset key to old value
						const int oldkey = GetWindowLong(pksw->hwndKeyControl, GWL_USERDATA);
						SetWindowText(pksw->hwndKeyControl, rgszKeyName[oldkey]);
						}
					else
						{
						SetWindowText(pksw->hwndKeyControl, rgszKeyName[key]);
						SetWindowLong(pksw->hwndKeyControl, GWL_USERDATA, key);
						}
					KillTimer(hwndDlg, pksw->m_timerid);
					pksw->m_timerid = 0;
					}
				}
			}
			break;

		case WM_COMMAND:
			{
			switch (HIWORD(wParam))
				{
				case BN_CLICKED:
					KeyWindowStruct * const pksw = (KeyWindowStruct *)GetWindowLong(hwndDlg, GWL_USERDATA);
					switch (LOWORD(wParam))
						{
						case IDC_LEFTFLIPPERBUTTON:
						case IDC_RIGHTFLIPPERBUTTON:
						case IDC_LEFTTILTBUTTON:
						case IDC_RIGHTTILTBUTTON:
						case IDC_CENTERTILTBUTTON:
						case IDC_PLUNGERBUTTON:
						case IDC_ADDCREDITBUTTON:
						case IDC_ADDCREDITBUTTON2:
						case IDC_STARTGAMEBUTTON:
						case IDC_EXITGAMEBUTTON:
						case IDC_FRAMECOUNTBUTTON:
						case IDC_DEBUGBALLSBUTTON:
						case IDC_VOLUPBUTTON:
						case IDC_VOLDOWNBUTTON:
						case IDC_RMAGSAVEBUTTON:
						case IDC_LMAGSAVEBUTTON:
						case IDC_MECHTILTBUTTON:
							{
							HWND hwndKeyWindow;
							if(pksw->m_timerid == NULL) //add
							{ //add
							switch (LOWORD(wParam))
								{
								case IDC_LEFTFLIPPERBUTTON:
									hwndKeyWindow = GetDlgItem(hwndDlg, IDC_LEFTFLIPPER);
									break;

								case IDC_RIGHTFLIPPERBUTTON:
									hwndKeyWindow = GetDlgItem(hwndDlg, IDC_RIGHTFLIPPER);
									break;

								case IDC_LEFTTILTBUTTON:
									hwndKeyWindow = GetDlgItem(hwndDlg, IDC_LEFTTILT);
									break;

								case IDC_RIGHTTILTBUTTON:
									hwndKeyWindow = GetDlgItem(hwndDlg, IDC_RIGHTTILT);
									break;

								case IDC_CENTERTILTBUTTON:
									hwndKeyWindow = GetDlgItem(hwndDlg, IDC_CENTERTILT);
									break;

								case IDC_PLUNGERBUTTON:
									hwndKeyWindow = GetDlgItem(hwndDlg, IDC_PLUNGER);
									break;

								case IDC_ADDCREDITBUTTON:
									hwndKeyWindow = GetDlgItem(hwndDlg, IDC_ADDCREDIT);
									break;

								case IDC_ADDCREDITBUTTON2:
									hwndKeyWindow = GetDlgItem(hwndDlg, IDC_ADDCREDIT2);
									break;

								case IDC_STARTGAMEBUTTON:
									hwndKeyWindow = GetDlgItem(hwndDlg, IDC_STARTGAME);
									break;

								case IDC_EXITGAMEBUTTON:
									hwndKeyWindow = GetDlgItem(hwndDlg, IDC_EXITGAME);
									break;

								case IDC_FRAMECOUNTBUTTON:
									hwndKeyWindow = GetDlgItem(hwndDlg, IDC_FRAMECOUNT);
									break;

								case IDC_VOLUPBUTTON:
									hwndKeyWindow = GetDlgItem(hwndDlg, IDC_VOLUMEUP);
									break;

								case IDC_VOLDOWNBUTTON:
									hwndKeyWindow = GetDlgItem(hwndDlg, IDC_VOLUMEDN);
									break;

								case IDC_DEBUGBALLSBUTTON:
									hwndKeyWindow = GetDlgItem(hwndDlg, IDC_DEBUGBALL);
									break;

								case IDC_RMAGSAVEBUTTON:
									hwndKeyWindow = GetDlgItem(hwndDlg, IDC_RMAGSAVE);
									break;

								case IDC_LMAGSAVEBUTTON:
									hwndKeyWindow = GetDlgItem(hwndDlg, IDC_LMAGSAVE);
									break;

								case IDC_MECHTILTBUTTON:
									hwndKeyWindow = GetDlgItem(hwndDlg, IDC_MECHTILT);
									break;
								}

								// corrects input error with spacebar
								const int key = pksw->pi.GetNextKey();
								if (key == 0x39)
								{
								pksw->pi.GetNextKey(); // Clear the current buffer out
								break;
								}


							pksw->pi.GetNextKey(); // Clear the current buffer out

							pksw->m_timerid = SetTimer(hwndDlg, 100, 50, NULL);
							pksw->hwndKeyControl = hwndKeyWindow;
							SetWindowText(pksw->hwndKeyControl, "????");
							while (pksw->pi.GetNextKey()!=NULL) //clear entire keyboard buffer contents
								{
									pksw->pi.GetNextKey();
								}
							}
							} //add
							break;

						case IDOK:
							{
							HWND hwndControl;
							int key;

							hwndControl = GetDlgItem(hwndDlg, IDC_LEFTFLIPPER);
							key = GetWindowLong(hwndControl, GWL_USERDATA);
							SetRegValue("Player", "LFlipKey", REG_DWORD, &key, 4);

							hwndControl = GetDlgItem(hwndDlg, IDC_RIGHTFLIPPER);
							key = GetWindowLong(hwndControl, GWL_USERDATA);
							SetRegValue("Player", "RFlipKey", REG_DWORD, &key, 4);

							hwndControl = GetDlgItem(hwndDlg, IDC_LEFTTILT);
							key = GetWindowLong(hwndControl, GWL_USERDATA);
							SetRegValue("Player", "LTiltKey", REG_DWORD, &key, 4);

							hwndControl = GetDlgItem(hwndDlg, IDC_RIGHTTILT);
							key = GetWindowLong(hwndControl, GWL_USERDATA);
							SetRegValue("Player", "RTiltKey", REG_DWORD, &key, 4);

							hwndControl = GetDlgItem(hwndDlg, IDC_CENTERTILT);
							key = GetWindowLong(hwndControl, GWL_USERDATA);
							SetRegValue("Player", "CTiltKey", REG_DWORD, &key, 4);

							hwndControl = GetDlgItem(hwndDlg, IDC_PLUNGER);
							key = GetWindowLong(hwndControl, GWL_USERDATA);
							SetRegValue("Player", "PlungerKey", REG_DWORD, &key, 4);

							hwndControl = GetDlgItem(hwndDlg, IDC_ADDCREDIT);
							key = GetWindowLong(hwndControl, GWL_USERDATA);
							SetRegValue("Player", "AddCreditKey", REG_DWORD, &key, 4);

							hwndControl = GetDlgItem(hwndDlg, IDC_ADDCREDITKEY2);
							key = GetWindowLong(hwndControl, GWL_USERDATA);
							SetRegValue("Player", "AddCreditKey2", REG_DWORD, &key, 4);

							hwndControl = GetDlgItem(hwndDlg, IDC_STARTGAME);
							key = GetWindowLong(hwndControl, GWL_USERDATA);
							SetRegValue("Player", "StartGameKey", REG_DWORD, &key, 4);

							hwndControl = GetDlgItem(hwndDlg, IDC_EXITGAME);
							key = GetWindowLong(hwndControl, GWL_USERDATA);
							SetRegValue("Player", "ExitGameKey", REG_DWORD, &key, 4);

							hwndControl = GetDlgItem(hwndDlg, IDC_FRAMECOUNT);
							key = GetWindowLong(hwndControl, GWL_USERDATA);
							SetRegValue("Player", "FrameCount", REG_DWORD, &key, 4);

							hwndControl = GetDlgItem(hwndDlg, IDC_VOLUMEUP);
							key = GetWindowLong(hwndControl, GWL_USERDATA);
							SetRegValue("Player", "VolumeUp", REG_DWORD, &key, 4);

							hwndControl = GetDlgItem(hwndDlg, IDC_VOLUMEDN);
							key = GetWindowLong(hwndControl, GWL_USERDATA);
							SetRegValue("Player", "VolumeDown", REG_DWORD, &key, 4);

							hwndControl = GetDlgItem(hwndDlg, IDC_DEBUGBALL);
							key = GetWindowLong(hwndControl, GWL_USERDATA);
							SetRegValue("Player", "DebugBalls", REG_DWORD, &key, 4);

							hwndControl = GetDlgItem(hwndDlg, IDC_RMAGSAVE);
							key = GetWindowLong(hwndControl, GWL_USERDATA);
							SetRegValue("Player", "RMagnaSave", REG_DWORD, &key, 4);

							hwndControl = GetDlgItem(hwndDlg, IDC_LMAGSAVE);
							key = GetWindowLong(hwndControl, GWL_USERDATA);
							SetRegValue("Player", "LMagnaSave", REG_DWORD, &key, 4);
							
							hwndControl = GetDlgItem(hwndDlg, IDC_MECHTILT);
							key = GetWindowLong(hwndControl, GWL_USERDATA);
							SetRegValue("Player", "MechTilt", REG_DWORD, &key, 4);

							EndDialog(hwndDlg, TRUE);
							}
							break;

						case IDCANCEL:
							EndDialog(hwndDlg, FALSE);
							break;
						}
				}
			}
			break;

		case WM_CLOSE:
			EndDialog(hwndDlg, FALSE);
			break;
		}

	return FALSE;
	}

int CALLBACK AudioOptionsProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
	switch (uMsg)
		{
		case WM_INITDIALOG:
			{
			HWND hwndParent = GetParent(hwndDlg);
			RECT rcDlg;
			RECT rcMain;
			GetWindowRect(hwndParent, &rcMain);
			GetWindowRect(hwndDlg, &rcDlg);

			SetWindowPos(hwndDlg, NULL,
				(rcMain.right + rcMain.left)/2 - (rcDlg.right - rcDlg.left)/2,
				(rcMain.bottom + rcMain.top)/2 - (rcDlg.bottom - rcDlg.top)/2,
				0, 0, SWP_NOOWNERZORDER | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE/* | SWP_NOMOVE*/);

			int fmusic = 0;
			HRESULT hr;
			HWND hwndControl;
			hr = GetRegInt("Player", "PlayMusic", &fmusic);
			if (hr != S_OK)
				{
				fmusic = 1; // The default
				}

			hwndControl = GetDlgItem(hwndDlg, IDC_PLAY_MUSIC);
			SendMessage(hwndControl, BM_SETCHECK, fmusic ? BST_CHECKED : BST_UNCHECKED, 0);
			if (!fmusic)
				{
				HWND hwndSlider = GetDlgItem(hwndDlg, IDC_MUSIC_SLIDER);
				HWND hwndText = GetDlgItem(hwndDlg, IDC_STATIC_MUSIC);
				EnableWindow(hwndSlider, FALSE);
				EnableWindow(hwndText, FALSE);
				}

			hr = GetRegInt("Player", "PlaySound", &fmusic);
			if (hr != S_OK)
				{
				fmusic = 1; // The default
				}

			hwndControl = GetDlgItem(hwndDlg, IDC_PLAY_SOUND);
			SendMessage(hwndControl, BM_SETCHECK, fmusic ? BST_CHECKED : BST_UNCHECKED, 0);
			if (!fmusic)
				{
				HWND hwndSlider = GetDlgItem(hwndDlg, IDC_SOUND_SLIDER);
				HWND hwndText = GetDlgItem(hwndDlg, IDC_STATIC_SOUND);
				EnableWindow(hwndSlider, FALSE);
				EnableWindow(hwndText, FALSE);
				}

			hr = GetRegInt("Player", "MusicVolume", &fmusic);
			if (hr != S_OK)
				{
				fmusic = 100; // The default
				}
			hwndControl = GetDlgItem(hwndDlg, IDC_MUSIC_SLIDER);
			::SendMessage(hwndControl, TBM_SETRANGE, fTrue, MAKELONG(0, 100));
			::SendMessage(hwndControl, TBM_SETTICFREQ, 10, 0);
			::SendMessage(hwndControl, TBM_SETLINESIZE, 0, 1);
			::SendMessage(hwndControl, TBM_SETPAGESIZE, 0, 10);
			::SendMessage(hwndControl, TBM_SETTHUMBLENGTH, 10, 0);
			::SendMessage(hwndControl, TBM_SETPOS, TRUE, fmusic);

			hr = GetRegInt("Player", "SoundVolume", &fmusic);
			if (hr != S_OK)
				{
				fmusic = 100; // The default
				}
			hwndControl = GetDlgItem(hwndDlg, IDC_SOUND_SLIDER);
			::SendMessage(hwndControl, TBM_SETRANGE, fTrue, MAKELONG(0, 100));
			::SendMessage(hwndControl, TBM_SETTICFREQ, 10, 0);
			::SendMessage(hwndControl, TBM_SETLINESIZE, 0, 1);
			::SendMessage(hwndControl, TBM_SETPAGESIZE, 0, 10);
			::SendMessage(hwndControl, TBM_SETTHUMBLENGTH, 10, 0);
			::SendMessage(hwndControl, TBM_SETPOS, TRUE, fmusic);
			}
			return TRUE;
			break;

		case WM_COMMAND:
			{
			switch (HIWORD(wParam))
				{
				case BN_CLICKED:
					switch (LOWORD(wParam))
						{
						case IDOK:
							{
							HWND hwndControl;
							int checked;
							int fmusic;
							int volume;

							hwndControl = GetDlgItem(hwndDlg, IDC_PLAY_MUSIC);
							checked = SendMessage(hwndControl, BM_GETCHECK, 0, 0);
							fmusic = (checked == BST_CHECKED) ? 1:0;
							SetRegValue("Player", "PlayMusic", REG_DWORD, &fmusic, 4);

							hwndControl = GetDlgItem(hwndDlg, IDC_PLAY_SOUND);
							checked = SendMessage(hwndControl, BM_GETCHECK, 0, 0);
							fmusic = (checked == BST_CHECKED) ? 1:0;
							SetRegValue("Player", "PlaySound", REG_DWORD, &fmusic, 4);

							hwndControl = GetDlgItem(hwndDlg, IDC_MUSIC_SLIDER);
							volume = SendMessage(hwndControl, TBM_GETPOS, 0, 0);
							SetRegValue("Player", "MusicVolume", REG_DWORD, &volume, 4);

							hwndControl = GetDlgItem(hwndDlg, IDC_SOUND_SLIDER);
							volume = SendMessage(hwndControl, TBM_GETPOS, 0, 0);
							SetRegValue("Player", "SoundVolume", REG_DWORD, &volume, 4);

							EndDialog(hwndDlg, TRUE);
							}
							break;

						case IDCANCEL:
							EndDialog(hwndDlg, FALSE);
							break;

						case IDC_PLAY_MUSIC:
							{
							const int checked = SendDlgItemMessage(hwndDlg, IDC_PLAY_MUSIC, BM_GETCHECK, 0, 0);
							HWND hwndSlider = GetDlgItem(hwndDlg, IDC_MUSIC_SLIDER);
							HWND hwndText = GetDlgItem(hwndDlg, IDC_STATIC_MUSIC);
							
							EnableWindow(hwndSlider, (checked == BST_CHECKED));
							EnableWindow(hwndText, (checked == BST_CHECKED));
							}
							break;

						case IDC_PLAY_SOUND:
							{
							const int checked = SendDlgItemMessage(hwndDlg, IDC_PLAY_SOUND, BM_GETCHECK, 0, 0);
							HWND hwndSlider = GetDlgItem(hwndDlg, IDC_SOUND_SLIDER);
							HWND hwndText = GetDlgItem(hwndDlg, IDC_STATIC_SOUND);
							
							EnableWindow(hwndSlider, (checked == BST_CHECKED));
							EnableWindow(hwndText, (checked == BST_CHECKED));
							}
							break;
						}
				}
			}
			break;

		case WM_CLOSE:
			EndDialog(hwndDlg, FALSE);
			break;
		}

	return FALSE;
	}

int CALLBACK EditorOptionsProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
	switch (uMsg)
		{
		case WM_INITDIALOG:
			{
			HWND hwndParent = GetParent(hwndDlg);
			RECT rcDlg;
			RECT rcMain;
			GetWindowRect(hwndParent, &rcMain);
			GetWindowRect(hwndDlg, &rcDlg);

			SetWindowPos(hwndDlg, NULL,
				(rcMain.right + rcMain.left)/2 - (rcDlg.right - rcDlg.left)/2,
				(rcMain.bottom + rcMain.top)/2 - (rcDlg.bottom - rcDlg.top)/2,
				0, 0, SWP_NOOWNERZORDER | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE/* | SWP_NOMOVE*/);

			// drag points
			int	fdrawpoints = 0;
			HRESULT hr;
			HWND 	hwndControl;
			hr = GetRegInt("Editor", "ShowDragPoints", &fdrawpoints);
			if (hr != S_OK)
				{
				fdrawpoints = 0; // The default (draw points)
				}
			hwndControl = GetDlgItem(hwndDlg, IDC_DRAW_DRAGPOINTS);
			SendMessage(hwndControl, BM_SETCHECK, fdrawpoints ? BST_CHECKED : BST_UNCHECKED, 0);

			// light centers
			int	fdrawcenters = 0;
			hr = GetRegInt("Editor", "DrawLightCenters", &fdrawcenters);
			if (hr != S_OK)
				{
				fdrawcenters = 0; // The default (don't draw centers)
				}
			hwndControl = GetDlgItem(hwndDlg, IDC_DRAW_LIGHTCENTERS);
			SendMessage(hwndControl, BM_SETCHECK, fdrawcenters ? BST_CHECKED : BST_UNCHECKED, 0);
			
			int fautosave = 0;
			hr = GetRegInt("Editor", "AutoSaveOn", &fautosave);
			if (hr != S_OK)
				{
				fautosave = 1; // The default
				}
			SendDlgItemMessage(hwndDlg, IDC_AUTOSAVE, BM_SETCHECK, fautosave ? BST_CHECKED : BST_UNCHECKED, 0);

			int fautosavetime = 0;
			hr = GetRegInt("Editor", "AutoSaveTime", &fautosavetime);
			if (hr != S_OK)
				{
				fautosavetime = AUTOSAVE_DEFAULT_TIME; // The default
				}
			SetDlgItemInt(hwndDlg, IDC_AUTOSAVE_MINUTES, fautosavetime, FALSE);
			}

			return TRUE;
			break;

		case WM_COMMAND:
			{
			switch (HIWORD(wParam))
				{
				case BN_CLICKED:
					switch (LOWORD(wParam))
						{
						case IDOK:
							{
							HWND hwndControl;
							int checked;
							// drag points
							int fdrawpoints;
							hwndControl = GetDlgItem(hwndDlg, IDC_DRAW_DRAGPOINTS);
							checked = SendMessage(hwndControl, BM_GETCHECK, 0, 0);
							fdrawpoints = (checked == BST_CHECKED) ? 1:0;
							SetRegValue("Editor", "ShowDragPoints", REG_DWORD, &fdrawpoints, 4);

							// light centers
							hwndControl = GetDlgItem(hwndDlg, IDC_DRAW_LIGHTCENTERS);
							checked = SendMessage(hwndControl, BM_GETCHECK, 0, 0);
							fdrawpoints = (checked == BST_CHECKED) ? 1:0;
							SetRegValue("Editor", "DrawLightCenters", REG_DWORD, &fdrawpoints, 4);

							int fautosave = SendDlgItemMessage(hwndDlg, IDC_AUTOSAVE, BM_GETCHECK, 0, 0);
							SetRegValue("Editor", "AutoSaveOn", REG_DWORD, &fautosave, 4);

							int autosavetime = GetDlgItemInt(hwndDlg, IDC_AUTOSAVE_MINUTES, NULL, TRUE);
							if (autosavetime < 0)
								{
								autosavetime = 0;
								}
							SetRegValue("Editor", "AutoSaveTime", REG_DWORD, &autosavetime, 4);

							// Go through and reset the autosave time on all the tables
							g_pvp->SetAutoSaveMinutes(autosavetime);
							for (int i=0;i<g_pvp->m_vtable.Size();i++)
								{
								g_pvp->m_vtable.ElementAt(i)->BeginAutoSaveCounter();
								}

							EndDialog(hwndDlg, TRUE);
							}
							break;

						case IDCANCEL:
							EndDialog(hwndDlg, FALSE);
							break;
						}
				}
			}
			break;

		case WM_CLOSE:
			EndDialog(hwndDlg, FALSE);
			break;
		}

	return FALSE;
	}

int CALLBACK ProtectTableProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
	switch (uMsg)
		{
		case WM_INITDIALOG:
			{
			HWND hwndParent = GetParent(hwndDlg);
			RECT rcDlg;
			RECT rcMain;
			GetWindowRect(hwndParent, &rcMain);
			GetWindowRect(hwndDlg, &rcDlg);

			SetWindowPos(hwndDlg, NULL,
				(rcMain.right + rcMain.left)/2 - (rcDlg.right - rcDlg.left)/2,
				(rcMain.bottom + rcMain.top)/2 - (rcDlg.bottom - rcDlg.top)/2,
				0, 0, SWP_NOOWNERZORDER | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE/* | SWP_NOMOVE*/);

			// limit the password fields to 16 characters (or PROT_PASSWORD_LENGTH)
			HWND hwndPassword = GetDlgItem(hwndDlg, IDC_PROTECT_PASSWORD);
	        SendMessage(hwndPassword, EM_LIMITTEXT, PROT_PASSWORD_LENGTH, 0L);
			hwndPassword = GetDlgItem(hwndDlg, IDC_PROTECT_PASSWORD2);
	        SendMessage(hwndPassword, EM_LIMITTEXT, PROT_PASSWORD_LENGTH, 0L);
			}
			return TRUE;
			break;

		case WM_COMMAND:
			{
			switch (HIWORD(wParam))
				{
				case BN_CLICKED:
					switch (LOWORD(wParam))
						{
						case IDC_PROTECT_TOTALLOCK:
							{
							// if the total lock check box is checked then disable any other options
							const int checked = SendDlgItemMessage(hwndDlg, IDC_PROTECT_TOTALLOCK, BM_GETCHECK, 0, 0);

							HWND hwndScript = GetDlgItem(hwndDlg,IDC_PROTECT_SCRIPT);
							HWND hwndSaveAs = GetDlgItem(hwndDlg,IDC_PROTECT_SAVEAS);
							HWND hwndSaveAsProt = GetDlgItem(hwndDlg,IDC_PROTECT_SAVEASPROT);
							HWND hwndManagers = GetDlgItem(hwndDlg,IDC_PROTECT_MANAGERS);
							HWND hwndCopy = GetDlgItem(hwndDlg,IDC_PROTECT_COPY);
							HWND hwndView = GetDlgItem(hwndDlg,IDC_PROTECT_VIEWTABLE);
							HWND hwndDebugger = GetDlgItem(hwndDlg,IDC_PROTECT_DEBUGGER);

							const int checkstate = !(checked == BST_CHECKED);

							EnableWindow(hwndScript, checkstate);
							EnableWindow(hwndSaveAs, checkstate);
							EnableWindow(hwndSaveAsProt, checkstate);
							EnableWindow(hwndManagers, checkstate);
							EnableWindow(hwndCopy, checkstate);
							EnableWindow(hwndView, checkstate);
							EnableWindow(hwndDebugger, checkstate);
							}
							break;

						case IDD_PROTECT_SHOWPASSWORD:
							{
							HWND hwndPassword = GetDlgItem(hwndDlg, IDC_PROTECT_PASSWORD);
							HWND hwndPassword2 = GetDlgItem(hwndDlg, IDC_PROTECT_PASSWORD2);

							const int checked = SendDlgItemMessage(hwndDlg, IDD_PROTECT_SHOWPASSWORD, BM_GETCHECK, 0, 0);
							if (checked == BST_CHECKED)
								{
	        						SendMessage(hwndPassword,  EM_SETPASSWORDCHAR, 0, 0L);
	        						SendMessage(hwndPassword2, EM_SETPASSWORDCHAR, 0, 0L);
								}
							else
								{
	        						SendMessage(hwndPassword,  EM_SETPASSWORDCHAR, '*', 0L);
	        						SendMessage(hwndPassword2, EM_SETPASSWORDCHAR, '*', 0L);
								}
							InvalidateRect(hwndPassword,  NULL, FALSE);
							InvalidateRect(hwndPassword2, NULL, FALSE);
							}
							break;

						case IDOK:
							{
							BOOL fail = fFalse;
							BOOL rc;

							// get the check box status(s)
							unsigned long flags = 0;
							const int checked1 = SendDlgItemMessage(hwndDlg, IDC_PROTECT_SAVEAS, BM_GETCHECK, 0, 0);
							if (checked1 == BST_CHECKED) flags |= DISABLE_TABLE_SAVE;
							const int checked2 = SendDlgItemMessage(hwndDlg, IDC_PROTECT_SAVEASPROT, BM_GETCHECK, 0, 0);
							if (checked2 == BST_CHECKED) flags |= DISABLE_TABLE_SAVEPROT;
							const int checked3 = SendDlgItemMessage(hwndDlg, IDC_PROTECT_SCRIPT, BM_GETCHECK, 0, 0);
							if (checked3 == BST_CHECKED) flags |= DISABLE_SCRIPT_EDITING;
							const int checked4 = SendDlgItemMessage(hwndDlg, IDC_PROTECT_MANAGERS, BM_GETCHECK, 0, 0);
							if (checked4 == BST_CHECKED) flags |= DISABLE_OPEN_MANAGERS;
							const int checked5 = SendDlgItemMessage(hwndDlg, IDC_PROTECT_COPY, BM_GETCHECK, 0, 0);
							if (checked5 == BST_CHECKED) flags |= DISABLE_CUTCOPYPASTE;
							const int checked6 = SendDlgItemMessage(hwndDlg, IDC_PROTECT_VIEWTABLE, BM_GETCHECK, 0, 0);
							if (checked6 == BST_CHECKED) flags |= DISABLE_TABLEVIEW;
							const int checked7 = SendDlgItemMessage(hwndDlg, IDC_PROTECT_DEBUGGER, BM_GETCHECK, 0, 0);
							if (checked7 == BST_CHECKED) flags |= DISABLE_DEBUGGER;
							const int checked0 = SendDlgItemMessage(hwndDlg, IDC_PROTECT_TOTALLOCK, BM_GETCHECK, 0, 0);
							if (checked0 == BST_CHECKED) flags |= DISABLE_EVERYTHING;

							// get the passwords
							char pw[PROT_PASSWORD_LENGTH+1];
							memset (pw,  0x00, sizeof(pw));
							HWND hwndPw = GetDlgItem(hwndDlg, IDC_PROTECT_PASSWORD);
							GetWindowText(hwndPw,  pw,  sizeof(pw));

							char pw2[PROT_PASSWORD_LENGTH+2];
							memset (pw2, 0x00, sizeof(pw2));
							HWND hwndPw2 = GetDlgItem(hwndDlg, IDC_PROTECT_PASSWORD2);
							GetWindowText(hwndPw2, pw2, sizeof(pw2));

							// is there at least one box checked?? (flags must contain at least 1 protection bit)
							if (flags == 0)
								{
								LocalString ls(IDS_PROTECT_ONETICKED);
								MessageBox(hwndDlg, ls.m_szbuffer, "Visual Pinball", MB_ICONWARNING);
								fail = fTrue;
								}
							else
								{
								// if both strings are empty then bomb out
								if ( (strlen(pw) == 0) && (strlen(pw2) == 0) )
									{
									LocalString ls(IDS_PROTECT_PW_ZEROLEN);
									MessageBox(hwndDlg, ls.m_szbuffer, "Visual Pinball", MB_ICONWARNING);
									fail = fTrue;
									}
								else
									{
									// do both strings match?
									if (strcmp(pw, pw2) != 0)
										{
										LocalString ls(IDS_PROTECT_PW_MISMATCH);
										MessageBox(hwndDlg, ls.m_szbuffer, "Visual Pinball", MB_ICONWARNING);
										fail = fTrue;
										}
									}
								}

							// has anything failed the sanity check?
							if (!fail)
								{
								// nope.. lets get started
								PinTable *pt = g_pvp->GetActiveTable();
								rc = pt->SetupProtectionBlock((unsigned char *)pw, flags);
								EndDialog(hwndDlg, rc);
								}
							}
							break;

						case IDCANCEL:
							EndDialog(hwndDlg, FALSE);
							break;
						}
				}
			}
			break;

		case WM_CLOSE:
			EndDialog(hwndDlg, FALSE);
			break;
		}

	return FALSE;
	}

int CALLBACK UnlockTableProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
	switch (uMsg)
		{
		case WM_INITDIALOG:
			{
			HWND hwndParent = GetParent(hwndDlg);
			RECT rcDlg;
			RECT rcMain;
			GetWindowRect(hwndParent, &rcMain);
			GetWindowRect(hwndDlg, &rcDlg);

			SetWindowPos(hwndDlg, NULL,
				(rcMain.right + rcMain.left)/2 - (rcDlg.right - rcDlg.left)/2,
				(rcMain.bottom + rcMain.top)/2 - (rcDlg.bottom - rcDlg.top)/2,
				0, 0, SWP_NOOWNERZORDER | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE/* | SWP_NOMOVE*/);

			// limit the password fields to 16 characters (or PROT_PASSWORD_LENGTH)
			HWND hwndPassword = GetDlgItem(hwndDlg, IDC_UNLOCK_PASSWORD);
	        SendMessage(hwndPassword, EM_LIMITTEXT, PROT_PASSWORD_LENGTH, 0L);
			}
			return TRUE;
			break;

		case WM_COMMAND:
			{
			switch (HIWORD(wParam))
				{
				case BN_CLICKED:
					switch (LOWORD(wParam))
						{
						case IDOK:
							{
							// get the password
							char pw[PROT_PASSWORD_LENGTH+1];
							memset (pw,  0x00, sizeof(pw));
							HWND hwndPw = GetDlgItem(hwndDlg, IDC_UNLOCK_PASSWORD);
							GetWindowText(hwndPw,  pw,  sizeof(pw));

							// if both password is empty bring up a message box
							if (strlen(pw) == 0)
								{
								LocalString ls(IDS_PROTECT_PW_ZEROLEN);
								MessageBox(hwndDlg, ls.m_szbuffer, "Visual Pinball", MB_ICONWARNING);
								}
							else
								{
								PinTable * const pt = g_pvp->GetActiveTable();
								const BOOL rc = pt->UnlockProtectionBlock((unsigned char *)pw);
								if (rc)
									{
										LocalString ls(IDS_UNLOCK_SUCCESS);
										MessageBox(hwndDlg, ls.m_szbuffer, "Visual Pinball", MB_ICONINFORMATION);
										EndDialog(hwndDlg, TRUE);
									}
									else
									{
										LocalString ls(IDS_UNLOCK_FAILED);
										MessageBox(hwndDlg, ls.m_szbuffer, "Visual Pinball", MB_ICONWARNING);
									}
								}
							}
							break;

						case IDCANCEL:
							EndDialog(hwndDlg, FALSE);
							break;
						}
				}
			}
			break;

		case WM_CLOSE:
			EndDialog(hwndDlg, FALSE);
			break;
		}

	return FALSE;
	}
