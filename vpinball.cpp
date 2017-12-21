// VPinball.cpp: implementation of the VPinball class.
//
////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "resource.h"
#include "ProtectTableDialog.h"
#include "KeysConfigDialog.h"

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


#define	RECENT_FIRST_MENU_IDM	5000	// ID of the first recent file list filename
#define	RECENT_LAST_MENU_IDM	RECENT_FIRST_MENU_IDM+LAST_OPENED_TABLE_COUNT

#define AUTOSAVE_DEFAULT_TIME 10

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
// menu locations
enum {
   FILEMENU = 0,
   EDITMENU,
   VIEWMENU,
   INSERTMENU,
   TABLEMENU,
   LAYERMENU,
   PREFMENU,
   WINDOWMENU,
   HELPMENU,
   NUM_MENUS
};

/*
TBButton:
typedef struct {
int       iBitmap;
int       idCommand;
BYTE      fsState;
BYTE      fsStyle;
DWORD_PTR dwData;
INT_PTR   iString;
} TBBUTTON, *PTBBUTTON, *LPTBBUTTON;
*/

#ifdef _WIN64
static TBBUTTON const g_tbbuttonMain[] = {
   // icon number,
   {14, ID_TABLE_MAGNIFY, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, 0, 0, 0, 0, IDS_TB_MAGNIFY, 0},
   {0, IDC_SELECT, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP | TBSTYLE_DROPDOWN, 0, 0, 0, 0, 0, 0, IDS_TB_SELECT, 1},
   {13, ID_EDIT_PROPERTIES, TBSTATE_ENABLED, TBSTYLE_CHECK, 0, 0, 0, 0, 0, 0, IDS_TB_PROPERTIES, 2},
   {18, ID_EDIT_SCRIPT, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, 0, 0, 0, IDS_TB_SCRIPT, 3},
   {19, ID_EDIT_BACKGLASSVIEW, TBSTATE_ENABLED, TBSTYLE_CHECK, 0, 0, 0, 0, 0, 0, IDS_TB_BACKGLASS, 4},
   {2, ID_TABLE_PLAY, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, 0, 0, 0, IDS_TB_PLAY, 5},
};

static TBBUTTON const g_tbbuttonPalette[] = {
   // icon number,
   {1, ID_INSERT_WALL, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, 0, 0, 0, 0, IDS_TB_WALL, 0},
   {15, ID_INSERT_GATE, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, 0, 0, 0, 0, IDS_TB_GATE, 1},
   {17, ID_INSERT_RAMP, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, 0, 0, 0, 0, IDS_TB_RAMP, 2},
   {3, ID_INSERT_FLIPPER, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, 0, 0, 0, 0, IDS_TB_FLIPPER, 3},
   {5, ID_INSERT_PLUNGER, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, 0, 0, 0, 0, IDS_TB_PLUNGER, 4},
   {7, ID_INSERT_BUMPER, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, 0, 0, 0, 0, IDS_TB_BUMPER, 5},
   {16, ID_INSERT_SPINNER, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, 0, 0, 0, 0, IDS_TB_SPINNER, 6},
   {4, ID_INSERT_TIMER, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, 0, 0, 0, 0, IDS_TB_TIMER, 7},
   {8, ID_INSERT_TRIGGER, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, 0, 0, 0, 0, IDS_TB_TRIGGER, 8},
   {9, ID_INSERT_LIGHT, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, 0, 0, 0, 0, IDS_TB_LIGHT, 9},
   {10, ID_INSERT_KICKER, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, 0, 0, 0, 0, IDS_TB_KICKER, 10},
   {11, ID_INSERT_TARGET, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, 0, 0, 0, 0, IDS_TB_TARGET, 11},
   {12, ID_INSERT_DECAL, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, 0, 0, 0, 0, IDS_TB_DECAL, 12},
   {6, ID_INSERT_TEXTBOX, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, 0, 0, 0, 0, IDS_TB_TEXTBOX, 13},
   {20, ID_INSERT_DISPREEL, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, 0, 0, 0, 0, IDS_TB_DISPREEL, 14},
   {21, ID_INSERT_LIGHTSEQ, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, 0, 0, 0, 0, IDS_TB_LIGHTSEQ, 15},
   {22, ID_INSERT_PRIMITIVE, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, 0, 0, 0, 0, IDS_TB_PRIMITIVE, 16},
   {35, ID_INSERT_FLASHER, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, 0, 0, 0, 0, IDS_TB_FLASHER, 17},
   {36, ID_INSERT_RUBBER, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, 0, 0, 0, 0, IDS_TB_RUBBER, 18},
};

static TBBUTTON const g_tbbuttonLayers[] = {
   {23, ID_LAYER_LAYER1, TBSTATE_ENABLED | TBSTATE_CHECKED, TBSTYLE_CHECK, 0, 0, 0, 0, 0, 0, 0, 0},
   {24, ID_LAYER_LAYER2, TBSTATE_ENABLED | TBSTATE_CHECKED, TBSTYLE_CHECK, 0, 0, 0, 0, 0, 0, 0, 1},
   {25, ID_LAYER_LAYER3, TBSTATE_ENABLED | TBSTATE_CHECKED, TBSTYLE_CHECK, 0, 0, 0, 0, 0, 0, 0, 2},
   {26, ID_LAYER_LAYER4, TBSTATE_ENABLED | TBSTATE_CHECKED, TBSTYLE_CHECK, 0, 0, 0, 0, 0, 0,  0, 3},
   {27, ID_LAYER_LAYER5, TBSTATE_ENABLED | TBSTATE_CHECKED, TBSTYLE_CHECK, 0, 0, 0, 0, 0, 0, 0, 4},
   {28, ID_LAYER_LAYER6, TBSTATE_ENABLED | TBSTATE_CHECKED, TBSTYLE_CHECK, 0, 0, 0, 0, 0, 0, 0, 5},
   {29, ID_LAYER_LAYER7, TBSTATE_ENABLED | TBSTATE_CHECKED, TBSTYLE_CHECK, 0, 0, 0, 0, 0, 0, 0, 6},
   {30, ID_LAYER_LAYER8, TBSTATE_ENABLED | TBSTATE_CHECKED, TBSTYLE_CHECK, 0, 0, 0, 0, 0, 0, 0, 7},
   {31, ID_LAYER_TOGGLEALL, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, 0, 0, 0, 0, 8},
};
#else
static TBBUTTON const g_tbbuttonMain[] = {
   // icon number,
   { 14, ID_TABLE_MAGNIFY, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, IDS_TB_MAGNIFY, 0 },
   { 0, IDC_SELECT, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP | TBSTYLE_DROPDOWN, 0, 0, IDS_TB_SELECT, 1 },
   { 13, ID_EDIT_PROPERTIES, TBSTATE_ENABLED, TBSTYLE_CHECK, 0, 0, IDS_TB_PROPERTIES, 2 },
   { 18, ID_EDIT_SCRIPT, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, IDS_TB_SCRIPT, 3 },
   { 19, ID_EDIT_BACKGLASSVIEW, TBSTATE_ENABLED, TBSTYLE_CHECK, 0, 0, IDS_TB_BACKGLASS, 4 },
   { 2, ID_TABLE_PLAY, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, IDS_TB_PLAY, 5 },
};

static TBBUTTON const g_tbbuttonPalette[] = {
   // icon number,
   { 1, ID_INSERT_WALL, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, IDS_TB_WALL, 0 },
   { 15, ID_INSERT_GATE, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, IDS_TB_GATE, 1 },
   { 17, ID_INSERT_RAMP, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, IDS_TB_RAMP, 2 },
   { 3, ID_INSERT_FLIPPER, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, IDS_TB_FLIPPER, 3 },
   { 5, ID_INSERT_PLUNGER, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, IDS_TB_PLUNGER, 4 },
   { 7, ID_INSERT_BUMPER, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, IDS_TB_BUMPER, 5 },
   { 16, ID_INSERT_SPINNER, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, IDS_TB_SPINNER, 6 },
   { 4, ID_INSERT_TIMER, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, IDS_TB_TIMER, 7 },
   { 8, ID_INSERT_TRIGGER, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, IDS_TB_TRIGGER, 8 },
   { 9, ID_INSERT_LIGHT, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, IDS_TB_LIGHT, 9 },
   { 10, ID_INSERT_KICKER, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, IDS_TB_KICKER, 10 },
   { 11, ID_INSERT_TARGET, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, IDS_TB_TARGET, 11 },
   { 12, ID_INSERT_DECAL, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, IDS_TB_DECAL, 12 },
   { 6, ID_INSERT_TEXTBOX, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, IDS_TB_TEXTBOX, 13 },
   { 20, ID_INSERT_DISPREEL, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, IDS_TB_DISPREEL, 14 },
   { 21, ID_INSERT_LIGHTSEQ, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, IDS_TB_LIGHTSEQ, 15 },
   { 22, ID_INSERT_PRIMITIVE, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, IDS_TB_PRIMITIVE, 16 },
   { 35, ID_INSERT_FLASHER, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, IDS_TB_FLASHER, 17 },
   { 36, ID_INSERT_RUBBER, TBSTATE_ENABLED, TBSTYLE_CHECKGROUP, 0, 0, IDS_TB_RUBBER, 18 },
};

static TBBUTTON const g_tbbuttonLayers[] = {
   { 23, ID_LAYER_LAYER1, TBSTATE_ENABLED | TBSTATE_CHECKED, TBSTYLE_CHECK, 0, 0, 0, 0 },
   { 24, ID_LAYER_LAYER2, TBSTATE_ENABLED | TBSTATE_CHECKED, TBSTYLE_CHECK, 0, 0, 0, 1 },
   { 25, ID_LAYER_LAYER3, TBSTATE_ENABLED | TBSTATE_CHECKED, TBSTYLE_CHECK, 0, 0, 0, 2 },
   { 26, ID_LAYER_LAYER4, TBSTATE_ENABLED | TBSTATE_CHECKED, TBSTYLE_CHECK, 0, 0, 0, 3 },
   { 27, ID_LAYER_LAYER5, TBSTATE_ENABLED | TBSTATE_CHECKED, TBSTYLE_CHECK, 0, 0, 0, 4 },
   { 28, ID_LAYER_LAYER6, TBSTATE_ENABLED | TBSTATE_CHECKED, TBSTYLE_CHECK, 0, 0, 0, 5 },
   { 29, ID_LAYER_LAYER7, TBSTATE_ENABLED | TBSTATE_CHECKED, TBSTYLE_CHECK, 0, 0, 0, 6 },
   { 30, ID_LAYER_LAYER8, TBSTATE_ENABLED | TBSTATE_CHECKED, TBSTYLE_CHECK, 0, 0, 0, 7 },
   { 31, ID_LAYER_TOGGLEALL, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, 8 },
};
#endif

static const int allLayers[8] =
{
   ID_LAYER_LAYER1,
   ID_LAYER_LAYER2,
   ID_LAYER_LAYER3,
   ID_LAYER_LAYER4,
   ID_LAYER_LAYER5,
   ID_LAYER_LAYER6,
   ID_LAYER_LAYER7,
   ID_LAYER_LAYER8
};

static char recentNumber[LAST_OPENED_TABLE_COUNT];
static char recentMenuname[MAX_PATH];

#define TBCOUNTMAIN (sizeof(g_tbbuttonMain) / sizeof(TBBUTTON))
#define TBCOUNTPALETTE (sizeof(g_tbbuttonPalette) / sizeof(TBBUTTON))
#define TBCOUNTLAYERS (sizeof(g_tbbuttonLayers) / sizeof(TBBUTTON))

LRESULT CALLBACK VPWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK VPSideBarWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

INT_PTR CALLBACK FontManagerProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK TableInfoProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK SecurityOptionsProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

typedef struct _tagSORTDATA
{
    HWND hwndList;
    int subItemIndex;
    int sortUpDown;
}SORTDATA;

SORTDATA SortData;
int columnSortOrder[4] = { 0 };

void AddToolTip(char *text, HWND parentHwnd, HWND toolTipHwnd, HWND controlHwnd)
{
    TOOLINFO toolInfo = { 0 };
    toolInfo.cbSize = sizeof(toolInfo);
    toolInfo.hwnd = parentHwnd;
    toolInfo.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
    toolInfo.uId = (UINT_PTR)controlHwnd;
    toolInfo.lpszText = text;
    SendMessage(toolTipHwnd, TTM_ADDTOOL, 0, (LPARAM)&toolInfo);
}

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
   memset(m_currentTablePath, 0, MAX_PATH);

   m_mouseCursorPosition.x = 0.0f;
   m_mouseCursorPosition.y = 0.0f;
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
   FreeLibrary(m_scintillaDll);
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
         break;
      szEnd--;
   }

   // truncate the filename
   *(szEnd + 1) = '\0'; // Get rid of exe name

   // store 2x
   lstrcpy(m_szMyPath, szPath);
   MultiByteToWideChar(CP_ACP, 0, szPath, -1, m_wzMyPath, MAX_PATH);
}

// Class Variables
bool VPinball::m_open_minimized;

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
   m_NextTableID = 1;

   m_ptableActive = NULL;
   m_hwndSideBar = NULL;										//Handle for left Sidebar
   m_hwndWork = NULL;											//Handle for Workarea

   m_workerthread = NULL;										//Workerthread - only for hanging scripts and autosave - will be created later

   GetMyPath();													//Store path of vpinball.exe in m_szMyPath and m_wzMyPath

   RegisterClasses();											//TODO - brief description of what happens in the function

#ifdef _WIN64
	m_scintillaDll = LoadLibrary("SciLexerVP64.DLL");
#else
	m_scintillaDll = LoadLibrary("SciLexerVP.DLL");
#endif
   if (m_scintillaDll == NULL)
	{
#ifdef _WIN64
		m_scintillaDll = LoadLibrary("SciLexer64.DLL");
#else
		m_scintillaDll = LoadLibrary("SciLexer.DLL");
#endif
		if (m_scintillaDll == NULL)
#ifdef _WIN64
			ShowError("Unable to load SciLexer64.DLL");
#else
			ShowError("Unable to load SciLexer.DLL");
#endif
	}
   char szName[256];
   LoadString(g_hinst, IDS_PROJNAME, szName, 256);
   // loading String "Visual Pinball" from Exe properties

   const int screenwidth = GetSystemMetrics(SM_CXSCREEN);		// width of primary monitor
   const int screenheight = GetSystemMetrics(SM_CYSCREEN);		// height of primary monitor

   const int x = (screenwidth - MAIN_WINDOW_WIDTH) / 2;
   const int y = (screenheight - MAIN_WINDOW_HEIGHT) / 2;
   const int width = MAIN_WINDOW_WIDTH;
   const int height = MAIN_WINDOW_HEIGHT;

   LPTSTR lpCmdLine = GetCommandLine();						//this line necessary for _ATL_MIN_CRT

   if (strstr(lpCmdLine, "minimized"))
      SetOpenMinimized();

//    m_hwnd = ::CreateWindowEx(WS_EX_OVERLAPPEDWINDOW, "VPinball", szName,
//       (m_open_minimized ? WS_MINIMIZE : 0) | WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_SIZEBOX | WS_MINIMIZEBOX | WS_MAXIMIZEBOX,
//       x, y, width, height, NULL, NULL, g_hinst, 0);				// get handle to and create main Window
   m_hwnd = CreateEx(WS_EX_OVERLAPPEDWINDOW, "VPinball", szName,
      (m_open_minimized ? WS_MINIMIZE : 0) | WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_SIZEBOX | WS_MINIMIZEBOX | WS_MAXIMIZEBOX,
      x, y, width, height, NULL, NULL);				// get handle to and create main Window

   // See if we have previous window size information
   {
      int left, top, right, bottom;
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

         ::GetWindowPlacement(m_hwnd, &winpl);

         winpl.rcNormalPosition.left = left;
         winpl.rcNormalPosition.top = top;
         winpl.rcNormalPosition.right = right;
         winpl.rcNormalPosition.bottom = bottom;

         if (m_open_minimized)
            winpl.showCmd |= SW_MINIMIZE;
         else if (hrmax == S_OK && fMaximized)
            winpl.showCmd |= SW_MAXIMIZE;

         ::SetWindowPlacement(m_hwnd, &winpl);
      }
   }

   ::ShowWindow(m_hwnd, SW_SHOW);

   ::SetWindowLongPtr(m_hwnd, GWLP_USERDATA, (size_t)this);	// set this class (vpinball) as callback for MDI Child / has to be confirmed
   // can be a problem for 64 bit compatibility.
   // maybe use SetWindowLongPtr instead

   CreateSideBar();									// Create Sidebar

   CreateMDIClient();								// Create MDI Child

   int foo[5] = { 120, 240, 400, 600, 800 };

   m_hwndStatusBar = CreateStatusWindow(WS_CHILD | WS_VISIBLE,
      "",
      m_hwnd,
      1);				// Create Status Line at the bottom

   ::SendMessage(m_hwndStatusBar, SB_SETPARTS, 5, (size_t)foo);	// Initialize Status bar with 5 empty cells

   InitRegValues();									// get default values from registry

   m_sb.Init(m_hwnd);								// initialize smartbrowser (Property bar on the right) - see propbrowser.cpp

   ::SendMessage(m_hwnd, WM_SIZE, 0, 0);				// Make our window relay itself out

   InitTools();
   InitPinDirectSound();

   m_fBackglassView = false;						// we are viewing Pinfield and not the backglass at first

   SetEnableToolbar();

   UpdateRecentFileList(NULL);						// update the recent loaded file list

   wintimer_init();								    // calibrate the timer routines
   if (m_fPropertiesFloating)
      ::SetForegroundWindow(m_hwnd);

#ifdef SLINTF
   // see slintf.cpp
   slintf_init();								    // initialize debug console (can be popupped by the following command)
   slintf_popup_console();
   slintf("Debug output:\n");
#endif
}

void VPinball::InitPinDirectSound()
{
	int DSidx1 = 0, DSidx2 = 0;
	GetRegInt("Player", "SoundDevice", &DSidx1);
	GetRegInt("Player", "SoundDeviceBG", &DSidx2);
	GetRegInt("Player", "Sound3D", &m_pds.m_i3DSoundMode);
	m_pds.InitDirectSound(m_hwnd, false);						// init Direct Sound (in pinsound.cpp)
	// If these are the same device, and we are not in 3d mode, just point the backglass device to the main one. 
	// For 3D we want two separate instances, one in basic stereo for music, and the other surround mode. 
	if (m_pds.m_i3DSoundMode==SNDCFG_SND3D2CH && DSidx1 == DSidx2)
	{
		m_pbackglassds = &m_pds;
	}
	else
	{
		m_pbackglassds = new PinDirectSound();
		m_pbackglassds->InitDirectSound(m_hwnd, true);
	}
}

///<summary>
///Ensure that worker thread exists
///<para>Starts worker Thread otherwise</para>
///</summary>
void VPinball::EnsureWorkerThread()
{
   if (!m_workerthread)
   {
      g_hWorkerStarted = CreateEvent(NULL, TRUE, FALSE, NULL);
      m_workerthread = (HANDLE)_beginthreadex(NULL, 0, VPWorkerThreadStart, 0, 0, &m_workerthreadid); //!! _beginthreadex is safer
      if (WaitForSingleObject(g_hWorkerStarted, 5000) == WAIT_TIMEOUT)
      {
      }
      SetThreadPriority(m_workerthread, THREAD_PRIORITY_LOWEST);
   }
}

///<summary>
///Post Work to the worker Thread
///<para>Creates Worker-Thread if not present</para>
///<para>See Worker::VPWorkerThreadStart for infos</para>
///<param name="workid">int for the type of message (COMPLETE_AUTOSAVE | HANG_SNOOP_START | HANG_SNOOP_STOP)</param>
///<param name="lParam">Second Parameter for message (AutoSavePackage (see worker.h) if COMPLETE_AUTOSAVE, otherwise NULL)</param>
///<returns>Handle to Event that get ack. If event is finished (unsure)</returns>
///</summary>
HANDLE VPinball::PostWorkToWorkerThread(int workid, LPARAM lParam)
{
   EnsureWorkerThread();										// Check if Workerthread was created once, otherwise create

   HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

   PostThreadMessage(m_workerthreadid, workid, (WPARAM)hEvent, lParam);

   return hEvent;
}

///<summary>
///Sets m_autosavetime
///<param name="minutes">int Minutes between autosave</param>
///</summary>
void VPinball::SetAutoSaveMinutes(const int minutes)
{
   m_autosaveTime = (minutes <= 0) ? -1 : minutes * (60 * 1000); // convert to milliseconds
}

///<summary>
///Post Work to the worker Thread
///<para>Creates Worker-Thread if not present</para>
///<para>See Worker::VPWorkerThreadStart for infos</para>
///<param name="workid">int for the type of message (COMPLETE_AUTOSAVE | HANG_SNOOP_START | HANG_SNOOP_STOP)</param>
///<param name="lparam">Second Parameter for message (AutoSavePackage (see worker.h) if COMPLETE_AUTOSAVE, otherwise NULL)</param>
///<returns>Handle to Event that get ack. If event is finished (unsure)</returns>
///</summary>
void VPinball::InitTools()
{
   // was the properties panel open last time we used VP?
   int state;
   const HRESULT hr = GetRegInt("Editor", "PropertiesVisible", (int *)&state);
   if ((hr == S_OK) && (state == 1))
   {
      // if so then re-open it
      ParseCommand(ID_EDIT_PROPERTIES, m_hwnd, 1); //display
   }

   m_ToolCur = IDC_SELECT;

   SendMessage(m_hwndToolbarMain, TB_CHECKBUTTON, IDC_SELECT, MAKELONG(TRUE, 0));
}

///<summary>
///Initializes Default Values of many variables (from Registry if keys are present). 
///<para>Registry Values under HKEY-CURRENT-USER/Software/Visual Pinball</para>
///<para>Deadzone, ShowDragPoints, DrawLightCenters,</para>
///<para>AutoSaveOn, AutoSaveTime, SecurityLevel</para>
///<para>Gets the last loaded Tables (List under File-Menu)</para>
///</summary>
void VPinball::InitRegValues()
{
   HRESULT hr;

   int deadz;
   deadz = GetRegIntWithDefault("Player", "DeadZone", 0);
   SetRegValueInt("Player", "DeadZone", deadz);

   m_fAlwaysDrawDragPoints = GetRegBoolWithDefault("Editor", "ShowDragPoints", false);
   m_fAlwaysDrawLightCenters = GetRegBoolWithDefault("Editor", "DrawLightCenters", false);
   m_gridSize = GetRegIntWithDefault("Editor", "GridSize", 50);

   BOOL fAutoSave = GetRegIntWithDefault("Editor", "AutoSaveOn", fTrue);
   m_fPropertiesFloating = !!GetRegIntWithDefault("Editor", "PropertiesFloating", fTrue);

   if (fAutoSave)
   {
      m_autosaveTime = GetRegIntWithDefault("Editor", "AutoSaveTime", AUTOSAVE_DEFAULT_TIME);
      SetAutoSaveMinutes(m_autosaveTime);
   }
   else
      m_autosaveTime = -1;

   m_securitylevel = GetRegIntWithDefault("Player", "SecurityLevel", DEFAULT_SECURITY_LEVEL);
   DWORD type = REG_DWORD;
   hr = GetRegValue("Editor", "DefaultMaterialColor", &type, &g_pvp->dummyMaterial.m_cBase, 4);
   if (FAILED(hr))
      g_pvp->dummyMaterial.m_cBase = 0xB469FF;

   hr = GetRegValue("Editor", "ElementSelectColor", &type, &g_pvp->m_elemSelectColor, 4);
   if (FAILED(hr))
      g_pvp->m_elemSelectColor = 0x00FF0000;

   hr = GetRegValue("Editor", "ElementSelectLockedColor", &type, &g_pvp->m_elemSelectLockedColor, 4);
   if (FAILED(hr))
      g_pvp->m_elemSelectLockedColor = 0x00A7726D;

   hr = GetRegValue("Editor", "BackgroundColor", &type, &g_pvp->m_backgroundColor, 4);
   if (FAILED(hr))
      g_pvp->m_backgroundColor = 0x008D8D8D;

   hr = GetRegValue("Editor", "FillColor", &type, &g_pvp->m_fillColor, 4);
   if (FAILED(hr))
      g_pvp->m_fillColor = 0x00B1CFB3;

   if (m_securitylevel < eSecurityNone || m_securitylevel > eSecurityNoControls)
      m_securitylevel = eSecurityNoControls;

   // get the list of the last n loaded tables
   for (int i = 0; i < LAST_OPENED_TABLE_COUNT; i++)
   {
      char szRegName[MAX_PATH];
      sprintf_s(szRegName, "TableFileName%d", i);
      m_szRecentTableList[i][0] = 0x00;
      GetRegString("RecentDir", szRegName, m_szRecentTableList[i], MAX_PATH);
   }
}

///<summary>
///Registers a window classes for subsequent use in calls to the CreateWindow or CreateWindowEx function. 
///</summary>
void VPinball::RegisterClasses()
{
   WNDCLASSEX wcex;
   ZeroMemory(&wcex, sizeof(WNDCLASSEX));
   wcex.cbSize = sizeof(WNDCLASSEX);
   wcex.style = CS_DBLCLKS;//CS_NOCLOSE | CS_OWNDC;
   wcex.lpfnWndProc = (WNDPROC)VPWndProc;
   wcex.hInstance = g_hinst;
   wcex.hIcon = LoadIcon(g_hinst, MAKEINTRESOURCE(IDI_VPINBALL));
   wcex.lpszClassName = "VPinball";
   wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
   wcex.lpszMenuName = MAKEINTRESOURCE(IDR_APPMENU);
   RegisterClassEx(&wcex);

   // Register dummy child class
   wcex.lpszClassName = "VPStaticChild";
   wcex.lpfnWndProc = VPSideBarWndProc;
   wcex.lpszMenuName = NULL;
   wcex.hIcon = NULL;
   wcex.hbrBackground = HBRUSH(COLOR_BTNFACE + 1);
   RegisterClassEx(&wcex);
}

///<summary>
///Creates Sidebar (left)
///<para>Creates handles to upper and lower(scrollable) left Sidebar </para>
///<para>Creates Buttons in Sidebar (via VPinball::CreateToolbar(...))</para>
///<para>Sets Scrollposition to 0</para>
///</summary>
void VPinball::CreateSideBar()
{
   RECT rc;
   ::GetWindowRect(m_hwnd, &rc);

   m_hwndSideBar = ::CreateWindowEx(/*WS_EX_WINDOWEDGE*/0, "VPStaticChild", "", WS_VISIBLE | WS_CHILD | WS_BORDER,
      0, 0, TOOLBAR_WIDTH + SCROLL_WIDTH, rc.bottom - rc.top, m_hwnd, NULL, g_hinst, 0);
   unsigned w,h;
   m_hwndToolbarMain = CreateToolbar((TBBUTTON *)g_tbbuttonMain, TBCOUNTMAIN, m_hwndSideBar, w,h);

   m_hwndSideBarLayers = ::CreateWindowEx(0, "VPStaticChild", "", WS_VISIBLE | WS_CHILD,
      0, h * (TBCOUNTMAIN / 2)+10, TOOLBAR_WIDTH /*+ SCROLL_WIDTH*/, rc.bottom - rc.top, m_hwndSideBar, NULL, g_hinst, 0);
   m_hwndToolbarLayers = CreateLayerToolbar(m_hwndSideBarLayers, w,h);

   m_hwndSideBarScroll = ::CreateWindowEx(0, "VPStaticChild", "", WS_VISIBLE | WS_CHILD | WS_VSCROLL,
      0, h * (TBCOUNTLAYERS / 3)+10, TOOLBAR_WIDTH + SCROLL_WIDTH, rc.bottom - rc.top, m_hwndSideBarLayers, NULL, g_hinst, 0);
   m_hwndToolbarPalette = CreateToolbar((TBBUTTON *)g_tbbuttonPalette, TBCOUNTPALETTE, m_hwndSideBarScroll, w,h);

   m_palettescroll = 0;
}

HWND VPinball::CreateLayerToolbar(HWND hwndParent, unsigned int &buttonwidth, unsigned int &buttonheight)
{
   HWND hwnd = CreateToolbarEx(hwndParent,
      WS_CHILD | WS_VISIBLE | TBSTYLE_BUTTON | TBSTYLE_WRAPABLE,
      1, TBCOUNTLAYERS, g_hinst, IDB_TOOLBAR, g_tbbuttonLayers, TBCOUNTLAYERS, 24, 24, 24, 24,
      sizeof(TBBUTTON));

   ::SendMessage(hwnd, TB_SETEXTENDEDSTYLE, 0, TBSTYLE_EX_DRAWDDARROWS);

#ifdef IMSPANISH
   SendMessage(m_hwnd, TB_SETBUTTONWIDTH, 0,
      (LPARAM)(DWORD)MAKELONG(50,50));
#elif defined(IMGERMAN)
   SendMessage(m_hwnd, TB_SETBUTTONWIDTH, 0,
      (LPARAM)(DWORD)MAKELONG(50,50));
#else
   SendMessage(hwnd, TB_SETBUTTONWIDTH, 0,
      (LPARAM)(DWORD)MAKELONG(50, 50));
#endif

   for (unsigned int i = 0; i < TBCOUNTLAYERS; i++)
   {
      TBBUTTONINFO tbbi;
      ZeroMemory(&tbbi, sizeof(TBBUTTONINFO));
      tbbi.cbSize = sizeof(TBBUTTONINFO);
      tbbi.dwMask = TBIF_SIZE | TBIF_COMMAND | TBIF_STATE | TBIF_STYLE;
      SendMessage(hwnd, TB_GETBUTTONINFO, g_tbbuttonLayers[i].idCommand, (LPARAM)&tbbi);
      if (tbbi.fsStyle & TBSTYLE_DROPDOWN)
         tbbi.cx = 48;
      SendMessage(hwnd, TB_SETBUTTONINFO, g_tbbuttonLayers[i].idCommand, (LPARAM)&tbbi);
   }

   SendMessage(hwnd, TB_AUTOSIZE, 0, 0);

   const LRESULT wh = SendMessage(hwnd, TB_GETBUTTONSIZE, 0, 0);
   buttonwidth = wh&0xFFFF;
   buttonheight = wh>>16;

   return hwnd;
}

///<summary>
///Creates Buttons in Toolbar-Windows (left Toolbar)
///<param name="*p_tbbutton">Pointer to Buttons as TBBUTTON[]</param>
///<param name="count">Number of Buttons to create</param>
///<param name="hwndParent">Parentwindow (left Toolbar (top or bottom))</param>
///<returns>Handle to Toolbar</returns>
///</summary>
HWND VPinball::CreateToolbar(TBBUTTON *p_tbbutton, int count, HWND hwndParent, unsigned int &buttonwidth, unsigned int &buttonheight)
{
   HWND hwnd = CreateToolbarEx(hwndParent,
      WS_CHILD | WS_VISIBLE | TBSTYLE_FLAT | TBSTYLE_WRAPABLE,
      1, count, g_hinst, IDB_TOOLBAR, p_tbbutton, count, 24, 24, 24, 24,
      sizeof(TBBUTTON));

   SendMessage(hwnd, TB_SETEXTENDEDSTYLE, 0, TBSTYLE_EX_DRAWDDARROWS);

#define MAXRESLEN 128

   char szBuf[MAXRESLEN];

   for (int i = 0; i < count; i++)
   {
      LoadString(g_hinst, p_tbbutton[i].dwData, szBuf, MAXRESLEN - 2);
      szBuf[lstrlen(szBuf) + 1] = 0;  //Double-null terminate.
      /*const int foo =*/ SendMessage(hwnd, TB_ADDSTRING, 0, (LPARAM)szBuf);
   }

#ifdef IMSPANISH
   SendMessage(m_hwnd, TB_SETBUTTONWIDTH, 0,
      (LPARAM)(DWORD)MAKELONG(50,50));
#elif defined(IMGERMAN)
   SendMessage(m_hwnd, TB_SETBUTTONWIDTH, 0,
      (LPARAM)(DWORD)MAKELONG(50,50));
#else
   SendMessage(hwnd, TB_SETBUTTONWIDTH, 0,
      (LPARAM)(DWORD)MAKELONG(50, 50));
#endif

   for (int i = 0; i < count; i++)
   {
      TBBUTTONINFO tbbi;
      ZeroMemory(&tbbi, sizeof(TBBUTTONINFO));
      tbbi.cbSize = sizeof(TBBUTTONINFO);
      tbbi.dwMask = TBIF_SIZE | TBIF_COMMAND | TBIF_STATE | TBIF_STYLE;
      /*int foo =*/ SendMessage(hwnd, TB_GETBUTTONINFO, p_tbbutton[i].idCommand, (LPARAM)&tbbi);
      if (tbbi.fsStyle & TBSTYLE_DROPDOWN)
         tbbi.cx = 48;
      /*foo =*/ SendMessage(hwnd, TB_SETBUTTONINFO, p_tbbutton[i].idCommand, (LPARAM)&tbbi);
   }

   SendMessage(hwnd, TB_AUTOSIZE, 0, 0);

   const LRESULT wh = SendMessage(hwnd, TB_GETBUTTONSIZE, 0, 0);
   buttonwidth = wh&0xFFFF;
   buttonheight = wh>>16;

   return hwnd;
}

void VPinball::CreateMDIClient()
{
   RECT rc;
   ::GetWindowRect(m_hwnd, &rc);

   CLIENTCREATESTRUCT ccs;
   ccs.hWindowMenu = ::GetSubMenu(::GetMenu(g_pvp->m_hwnd), WINDOWMENU); // Window menu is third from the left
   ccs.idFirstChild = 4000;//129;

   m_hwndWork = ::CreateWindowEx(0, "MDICLIENT", "", WS_VISIBLE | WS_CHILD | WS_CLIPCHILDREN | WS_VSCROLL | WS_HSCROLL,
      TOOLBAR_WIDTH + SCROLL_WIDTH, 0, rc.right - rc.left - (TOOLBAR_WIDTH + SCROLL_WIDTH), rc.bottom - rc.top, m_hwnd, NULL, g_hinst, &ccs);
}

void VPinball::SetClipboard(Vector<IStream> *pvstm)
{
   for (int i = 0; i < m_vstmclipboard.Size(); i++)
      m_vstmclipboard.ElementAt(i)->Release();
   m_vstmclipboard.RemoveAllElements();

   if (pvstm)
      for (int i = 0; i < pvstm->Size(); i++)
         m_vstmclipboard.AddElement(pvstm->ElementAt(i));
}

void VPinball::SetCursorCur(HINSTANCE hInstance, LPCTSTR lpCursorName)
{
   HCURSOR hcursor = LoadCursor(hInstance, lpCursorName);
   SetCursor(hcursor);
}

void VPinball::SetActionCur(char *szaction)
{
   SendMessage(m_hwndStatusBar, SB_SETTEXT, 3 | 0, (size_t)szaction);
}

void VPinball::SetStatusBarElementInfo(const char *info)
{
   SendMessage(m_hwndStatusBar, SB_SETTEXT, 4 | 0, (size_t)info);
}

void VPinball::SetPosCur(float x, float y)
{
   char szT[256];
   sprintf_s(szT, "%.4f, %.4f", x, y);
   m_mouseCursorPosition.x = x;
   m_mouseCursorPosition.y = y;
   SendMessage(m_hwndStatusBar, SB_SETTEXT, 0 | 0, (size_t)szT);
}

void VPinball::SetObjectPosCur(float x, float y)
{
   char szT[256];
   sprintf_s(szT, "%.4f, %.4f", x, y);
   SendMessage(m_hwndStatusBar, SB_SETTEXT, 1 | 0, (size_t)szT);
}

void VPinball::ClearObjectPosCur()
{
   SendMessage(m_hwndStatusBar, SB_SETTEXT, 1 | 0, (size_t)"");
}

void VPinball::SetPropSel(VectorProtected<ISelect> *pvsel)
{
   m_sb.CreateFromDispatch(m_hwnd, pvsel);
}

void VPinball::DeletePropSel(void)
{
   m_sb.RemoveSelection();
}

HMENU VPinball::GetMainMenu(int id)
{
   HMENU hmenu = ::GetMenu(m_hwnd);
   const int count = GetMenuItemCount(hmenu);
   return GetSubMenu(hmenu, id + ((count > NUM_MENUS) ? 1 : 0)); // MDI has added its stuff (table icon for first menu item)
}


void VPinball::ParseCommand(size_t code, HWND hwnd, size_t notify)
{
   CComObject<PinTable> *ptCur;

   // check if it's an Editable tool
   ItemTypeEnum type = EditableRegistry::TypeFromToolID((int)code);
   if (type != eItemInvalid)
   {
      SendMessage(m_hwndToolbarMain, TB_CHECKBUTTON, m_ToolCur, MAKELONG(FALSE, 0));
      SendMessage(m_hwndToolbarPalette, TB_CHECKBUTTON, code, MAKELONG(TRUE, 0));

      m_ToolCur = (int)code;

      if (notify == 1) // accelerator - mouse can be over table already
      {
         POINT pt;
         GetCursorPos(&pt);
         SetCursorPos(pt.x, pt.y);
      }
      return;
   }

   /* a MDI client window starts with ID 4000 and is incremented by Windows if a new window(table) is loaded 
      if the user switches a table (multiple tables are loaded) the code is 4000+ support up to 30 loaded tables here */
   if (code >= 4000 && code < 4030)
   {
       /* close all dialogs if the table is changed to prevent further issues */
       CloseAllDialogs();
   }

   switch (code)
   {
   case IDM_NEW:
   case ID_NEW_BLANKTABLE:
   case ID_NEW_EXAMPLETABLE:
   {
      CComObject<PinTable> *pt;
      CComObject<PinTable>::CreateInstance(&pt);
      pt->AddRef();
      pt->Init(this,code != ID_NEW_EXAMPLETABLE);
      //pt = new PinTable(this);
      m_vtable.AddElement(pt);
      SetEnableToolbar();
      break;
   }
   case ID_DELETE:
   {
      ptCur = GetActiveTable();
      if (ptCur)
      {
         if (ptCur->CheckPermissions(DISABLE_CUTCOPYPASTE))
            ShowPermissionError();
         else
            ptCur->OnDelete();
      }
      break;
   }
   case ID_TABLE_CAMERAMODE:
   case ID_TABLE_PLAY:
   {
       DoPlay(code == ID_TABLE_CAMERAMODE);
       break;
   }
   case ID_SCRIPT_SHOWIDE:
   case ID_EDIT_SCRIPT:
   {
      ptCur = GetActiveTable();
      if (ptCur)
      {
         if (ptCur->CheckPermissions(DISABLE_SCRIPT_EDITING))
            ShowPermissionError();
         else
            ptCur->m_pcv->SetVisible(fTrue);
      }
      break;
   }
   case ID_EDIT_PROPERTIES:
   {
      BOOL fShow = fFalse;

      if (!g_pplayer) fShow = m_sb.GetVisible(); // Get the current display state 

      switch (notify)
      {
      case 0: fShow = !fShow; //!!?
         break;
      case 1: fShow = fTrue;  //set
         break;
      case 2:                 //re-display 
         break;
      default: fShow = !fShow;//toggle
         break;
      }

      SetRegValue("Editor", "PropertiesVisible", REG_DWORD, &fShow, 4);

      if (!g_pplayer)
      {
         // Set toolbar button to the correct state
         TBBUTTONINFO tbinfo;
         ZeroMemory(&tbinfo, sizeof(TBBUTTONINFO));
         tbinfo.cbSize = sizeof(TBBUTTONINFO);
         tbinfo.dwMask = TBIF_STATE;
         SendMessage(m_hwndToolbarMain, TB_GETBUTTONINFO, ID_EDIT_PROPERTIES, (size_t)&tbinfo);

         if (notify == 2) fShow = (tbinfo.fsState & TBSTATE_CHECKED) != 0;

         if (fShow ^ ((tbinfo.fsState & TBSTATE_CHECKED) != 0))
         {
            tbinfo.fsState ^= TBSTATE_CHECKED;
         }

         SendMessage(m_hwndToolbarMain, TB_SETBUTTONINFO, ID_EDIT_PROPERTIES, (size_t)&tbinfo);
      }

      m_sb.SetVisible(fShow);

      SendMessage(m_hwnd, WM_SIZE, 0, 0);
      if (fShow)
      {
         ptCur = GetActiveTable();
         if (ptCur)
         {
            if (!ptCur->CheckPermissions(DISABLE_TABLEVIEW))
               m_sb.CreateFromDispatch(m_hwnd, &ptCur->m_vmultisel);
         }
      }
      break;
   }
   case ID_EDIT_BACKGLASSVIEW:
   {
      const bool fShow = !m_fBackglassView;

      SendMessage(m_hwndToolbarMain, TB_CHECKBUTTON, ID_EDIT_BACKGLASSVIEW, MAKELONG(fShow, 0));

      m_fBackglassView = fShow;

      for (int i = 0; i < m_vtable.Size(); i++)
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
            // Set selection to something in the new view (unless hiding table elements)
            ptCur->AddMultiSel((ISelect *)ptCur, false);
      }

      SetEnableToolbar();
      break;
   }
   case ID_EDIT_SEARCH:
   {
      ptCur = GetActiveTable();
      if (ptCur)
      {
         if (!ptCur->m_searchSelectDlg.IsWindow())
         {
            ptCur->m_searchSelectDlg.Create(m_hwnd);

            char windowName[256];
            strcpy_s(windowName, "Search/Select Element - ");
            strncat_s(windowName, ptCur->m_szFileName, 255);
            ::SetWindowText(ptCur->m_searchSelectDlg.GetHwnd(), windowName);

            ptCur->m_searchSelectDlg.ShowWindow();
         }
         else
         {
            ptCur->m_searchSelectDlg.SetForegroundWindow();
         }
      }
      break;
   }
   case ID_EDIT_SETDEFAULTPHYSICS:
   {
      ptCur = GetActiveTable();
      if (ptCur)
      {
         LocalString ls(IDS_DEFAULTPHYSICS);
         const int answ = ::MessageBox(m_hwnd, ls.m_szbuffer, "Continue?", MB_YESNO | MB_ICONWARNING);
         if (answ == IDYES)
         {
            ptCur->BeginUndo();
            for (int i = 0; i < ptCur->m_vmultisel.size(); i++)
               ptCur->m_vmultisel.ElementAt(i)->SetDefaultPhysics(true);
            ptCur->EndUndo();
            m_sb.RefreshProperties();
         }
      }
      break;
   }
   case ID_LOCK:
   {
      ptCur = GetActiveTable();
      if (ptCur)
         ptCur->LockElements();
      break;

   }
   case ID_EDIT_DRAWINGORDER_HIT:
   {
      //DialogBoxParam(g_hinst, MAKEINTRESOURCE(IDD_DRAWING_ORDER), m_hwnd, DrawingOrderProc, 0);
      ShowDrawingOrderDialog(false);
      break;
   }
   case ID_EDIT_DRAWINGORDER_SELECT:
   {
      //DialogBoxParam(g_hinst, MAKEINTRESOURCE(IDD_DRAWING_ORDER), m_hwnd, DrawingOrderProc, 0);
      ShowDrawingOrderDialog(true);
      break;
   }
   case ID_VIEW_SOLID:
   case ID_VIEW_OUTLINE:
   {
       ptCur = GetActiveTable();
       if (ptCur)
       {
           ptCur->m_renderSolid = (code == ID_VIEW_SOLID);
           ptCur->SetDirtyDraw();
           SetRegValueBool("Editor", "RenderSolid", ptCur->m_renderSolid);
       }
       break;
   }
   case ID_VIEW_GRID:
   {
       ptCur = GetActiveTable();
       if (ptCur)
           ptCur->put_DisplayGrid(!ptCur->m_fGrid);
       break;
   }
   case ID_VIEW_BACKDROP:
   {
       ptCur = GetActiveTable();
       if (ptCur)
           ptCur->put_DisplayBackdrop(!ptCur->m_fBackdrop);
       break;
   }
   case IDC_SELECT:
   case ID_TABLE_MAGNIFY:
   {
      SendMessage(m_hwndToolbarPalette, TB_CHECKBUTTON, m_ToolCur, MAKELONG(FALSE, 0));
      SendMessage(m_hwndToolbarMain, TB_CHECKBUTTON, code, MAKELONG(TRUE, 0));

      m_ToolCur = (int)code;

      if (notify == 1) // accelerator - mouse can be over table already
      {
         POINT pt;
         GetCursorPos(&pt);
         SetCursorPos(pt.x, pt.y);
      }
      break;
   }
   case ID_ADD_CTRL_POINT:
   {
      ptCur = GetActiveTable();
      if (ptCur == NULL)
         break;
   
      if (ptCur->m_vmultisel.Size() > 0)
      {
         ISelect *psel = ptCur->m_vmultisel.ElementAt(0);
         if (psel != NULL)
         {
            POINT pt;
            GetCursorPos(&pt);
            ::ScreenToClient(ptCur->m_hwnd, &pt);

            switch (psel->GetItemType())
            {
            case eItemRamp:
            {
               Ramp * const pRamp = (Ramp*)psel;
               pRamp->AddPoint(pt.x, pt.y, false);
               break;
            }
            case eItemLight:
            {
               Light * const pLight = (Light*)psel;
               pLight->AddPoint(pt.x, pt.y, false);
               break;
            }
            case eItemSurface:
            {
               Surface * const pSurf = (Surface*)psel;
               pSurf->AddPoint(pt.x, pt.y, false);
               break;
            }
            default:
               break;
            }
         }//if (psel != NULL)
      }
      break;
   }
   case ID_ADD_SMOOTH_CTRL_POINT:
   {
      ptCur = GetActiveTable();
      if (ptCur == NULL)
         break;

      if (ptCur->m_vmultisel.Size() > 0)
      {
         ISelect *psel = ptCur->m_vmultisel.ElementAt(0);
         if (psel != NULL)
         {
            POINT pt;
            GetCursorPos(&pt);
            ::ScreenToClient(ptCur->m_hwnd, &pt);
            switch (psel->GetItemType())
            {
            case eItemRamp:
            {
               Ramp * const pRamp = (Ramp*)psel;
               pRamp->AddPoint(pt.x, pt.y,true);
               break;
            }
            case eItemLight:
            {
               Light * const pLight = (Light*)psel;
               pLight->AddPoint(pt.x, pt.y,true);
               break;
            }
            case eItemSurface:
            {
               Surface * const pSurf = (Surface*)psel;
               pSurf->AddPoint(pt.x, pt.y,true);
               break;
            }
            default:
               break;
            }
         }
      }
      break;
   }
   case IDM_SAVE:
   {
      ptCur = GetActiveTable();
      if (ptCur)
      {
         if (ptCur->CheckPermissions(DISABLE_TABLE_SAVE))
            ShowPermissionError();
         else
         {
            HRESULT hr = ptCur->TableSave();
            if (hr == S_OK)
               UpdateRecentFileList(ptCur->m_szFileName);
         }
      }
      break;
   }
   case IDM_SAVEAS:
   {
      ptCur = GetActiveTable();
      if (ptCur)
      {
         if (ptCur->CheckPermissions(DISABLE_TABLE_SAVE))
            ShowPermissionError();
         else
         {
            HRESULT hr = ptCur->SaveAs();
            if (hr == S_OK)
               UpdateRecentFileList(ptCur->m_szFileName);
         }
      }
      break;
   }
   case IDM_SAVEASPROTECTED:
   {
      ptCur = GetActiveTable();
      if (ptCur)
      {
         if ((ptCur->CheckPermissions(DISABLE_TABLE_SAVE)) ||
            (ptCur->CheckPermissions(DISABLE_TABLE_SAVEPROT)))
            ShowPermissionError();
         else
         {
            ProtectTableDialog *protectDlg = new ProtectTableDialog();
            INT_PTR foo = protectDlg->DoModal();
            delete protectDlg;

            // if the dialog returned ok then perform a normal save as
            if (foo==1)
            {
               HRESULT foo2 = ptCur->SaveAs();
               if (foo2 == S_OK)
               {
                  // if the save was successful then the permissions take effect immediatly
                  SetEnableToolbar();			// disable any tool bars
                  ptCur->SetDirtyDraw();		// redraw the screen (incase hiding elements)
                  UpdateRecentFileList(ptCur->m_szFileName);
               }
               else
                  // if the save failed, then reset the permissions
                  ptCur->ResetProtectionBlock();
            }
         }
      }
      break;
   }
   case IDM_UNLOCKPROTECTED:
   {
      ptCur = GetActiveTable();
      if (ptCur)
      {
         UnprotectDialog *unprotectDlg = new UnprotectDialog();
         INT_PTR foo = unprotectDlg->DoModal();
         delete unprotectDlg;

         // if the dialog returned ok then table is unlocked
         if (foo==1)
         {
            // re-enable any disabled menu items
            SetEnableToolbar();			// disable any tool bars
            ptCur->SetDirtyDraw();		// redraw the screen (incase hiding elements)
         }
      }
      break;
   }

   case RECENT_FIRST_MENU_IDM:
   case RECENT_FIRST_MENU_IDM + 1:
   case RECENT_FIRST_MENU_IDM + 2:
   case RECENT_FIRST_MENU_IDM + 3:
   case RECENT_FIRST_MENU_IDM + 4:
   case RECENT_FIRST_MENU_IDM + 5:
   case RECENT_FIRST_MENU_IDM + 6:
   case RECENT_FIRST_MENU_IDM + 7:
   {
      char szFileName[MAX_PATH];
      // get the index into the recent list menu
      const size_t Index = code - RECENT_FIRST_MENU_IDM;
      // copy it into a temporary string so it can be correctly processed
      memcpy(szFileName, m_szRecentTableList[Index], sizeof(szFileName));
      LoadFileName(szFileName);
      break;
   }

   case IDM_OPEN:
   {
       LoadFile();
       break;
   }
   case IDM_CLOSE:
   {
      ptCur = GetActiveTable();
      if (ptCur)
         CloseTable(ptCur);
      break;
   }
   case IDC_COPY:
   {
      ptCur = GetActiveTable();
      if (ptCur)
      {
         if (ptCur->CheckPermissions(DISABLE_CUTCOPYPASTE))
            ShowPermissionError();
         else
            ptCur->Copy();
      }
      break;
   }
   case IDC_PASTE:
   {
      ptCur = GetActiveTable();
      if (ptCur)
         ptCur->Paste(fFalse, 0, 0);
      break;
   }
   case IDC_PASTEAT:
   {
      ptCur = GetActiveTable();
      if (ptCur)
      {
         POINT ptCursor;
         GetCursorPos(&ptCursor);
         ::ScreenToClient(ptCur->m_hwnd, &ptCursor);
         ptCur->Paste(fTrue, ptCursor.x, ptCursor.y);
      }
      break;
   }
   case ID_EDIT_UNDO:
   {
      ptCur = GetActiveTable();
      if (ptCur)
         ptCur->Undo();
      break;
   }
   case ID_FILE_EXPORT_BLUEPRINT:
   {
      ptCur = GetActiveTable();
      if (ptCur)
      {
         if (ptCur->CheckPermissions(DISABLE_TABLE_SAVE))
            ShowPermissionError();
         else
            ptCur->ExportBlueprint();
      }
      break;
   }
   case ID_EXPORT_TABLEMESH:
   {
      ptCur = GetActiveTable();
      if (ptCur)
      {
         if (ptCur->CheckPermissions(DISABLE_TABLE_SAVE))
            ShowPermissionError();
         else
            ptCur->ExportTableMesh();
      }
      break;
   }
   case ID_IMPORT_BACKDROPPOV:
   {
       ptCur = GetActiveTable();
       if (ptCur)
       {
           if (ptCur->CheckPermissions(DISABLE_TABLE_SAVE))
               ShowPermissionError();
           else
               ptCur->ImportBackdropPOV();
       }
       break;
   }
   case ID_EXPORT_BACKDROPPOV:
   {
       ptCur = GetActiveTable();
       if (ptCur)
       {
           if (ptCur->CheckPermissions(DISABLE_TABLE_SAVE))
               ShowPermissionError();
           else
               ptCur->ExportBackdropPOV();
       }
       break;
   }
   case ID_FILE_EXIT:
   {
       PostMessage(m_hwnd, WM_CLOSE, 0, 0);
       break;
   }
   case ID_EDIT_AUDIOOPTIONS:
   {
       m_audioOptDialog.DoModal(m_hwnd);
      break;
   }
   case ID_EDIT_PHYSICSOPTIONS:
   {
       m_physicsOptDialog.DoModal(m_hwnd);
       break;
   }
   case ID_EDIT_EDITOROPTIONS:
   {
       m_editorOptDialog.DoModal(m_hwnd);
      // refresh editor options from the registry
      InitRegValues();
      // force a screen refresh (it an active table is loaded)
      ptCur = GetActiveTable();
      if (ptCur)
         ptCur->SetDirtyDraw();
      break;
   }
   case ID_EDIT_VIDEOOPTIONS:
   {
       m_videoOptDialog.DoModal(m_hwnd);
       break;
   }
   case ID_TABLE_TABLEINFO:
   {
       ptCur = GetActiveTable();
       if (ptCur)
       {
           m_tableInfoDialog.DoModal(m_hwnd);
       }
       break;
   }
   case IDM_IMAGE_EDITOR:
   case ID_TABLE_IMAGEMANAGER:
   {
       ptCur = GetActiveTable();
       if (ptCur)
       {
           if (ptCur->CheckPermissions(DISABLE_OPEN_MANAGERS))
               ShowPermissionError();
           else
           {
               ShowSubDialog(m_imageMngDlg);

               m_sb.PopulateDropdowns(); // May need to update list of images
               m_sb.RefreshProperties();
           }
       }
       break;
   }
   case IDM_SOUND_EDITOR:
   case ID_TABLE_SOUNDMANAGER:
   {
       ptCur = GetActiveTable();
       if (ptCur)
       {
           if (ptCur->CheckPermissions(DISABLE_OPEN_MANAGERS))
               ShowPermissionError();
           else
           {
               if (!m_soundMngDlg.IsWindow())
               {
                   m_soundMngDlg.Create(m_hwnd);
                   m_soundMngDlg.ShowWindow();
               }
               else
                   m_soundMngDlg.SetForegroundWindow();
           }
       }
       break;
   }
   case IDM_MATERIAL_EDITOR:
   case ID_TABLE_MATERIALMANAGER:
   {
       ptCur = GetActiveTable();
       if (ptCur)
       {
           if (ptCur->CheckPermissions(DISABLE_OPEN_MANAGERS))
               ShowPermissionError();
           else
           {
               ShowSubDialog(m_materialDialog);

               m_sb.PopulateDropdowns(); // May need to update list of images
               m_sb.RefreshProperties();
           }
       }
       break;
   }
   case ID_TABLE_FONTMANAGER:
   {
       ptCur = GetActiveTable();
       if (ptCur)
       {
           if (ptCur->CheckPermissions(DISABLE_OPEN_MANAGERS))
               ShowPermissionError();
           else
           {
               /*const DWORD foo =*/ DialogBoxParam(g_hinst, MAKEINTRESOURCE(IDD_FONTDIALOG),
                   m_hwnd, FontManagerProc, (size_t)ptCur);
           }
       }
       break;
   }
   case ID_TABLE_DIMENSIONMANAGER:
   {
       ptCur = GetActiveTable();
       if (ptCur)
       {
           if (ptCur->CheckPermissions(DISABLE_OPEN_MANAGERS))
           {
               ShowPermissionError();
               break;
           }
       }
       ShowSubDialog(m_dimensionDialog);
       break;
   }
   case IDM_COLLECTION_EDITOR:
   case ID_TABLE_COLLECTIONMANAGER:
   {
       ptCur = GetActiveTable();
       if (ptCur)
       {
           if (ptCur->CheckPermissions(DISABLE_OPEN_MANAGERS))
               ShowPermissionError();
           else
           {
               ShowSubDialog(m_collectionMngDlg);
               
               m_sb.PopulateDropdowns(); // May need to update list of collections
               m_sb.RefreshProperties();
           }
       }
       break;
   }
   case ID_PREFERENCES_SECURITYOPTIONS:
   {
      DialogBoxParam(g_hinst, MAKEINTRESOURCE(IDD_SECURITY_OPTIONS), m_hwnd, SecurityOptionsProc, 0);

      // refresh editor options from the registry
      InitRegValues();
      break;
   }
   case ID_EDIT_KEYS:
   {
      KeysConfigDialog *keysConfigDlg = new KeysConfigDialog();
      keysConfigDlg->DoModal();
      delete keysConfigDlg;
      break;
   }
   case ID_LAYER_LAYER1:
   {
      setLayerStatus(0);
      break;
   }
   case ID_LAYER_LAYER2:
   {
      setLayerStatus(1);
      break;
   }
   case ID_LAYER_LAYER3:
   {
      setLayerStatus(2);
      break;
   }
   case ID_LAYER_LAYER4:
   {
      setLayerStatus(3);
      break;
   }
   case ID_LAYER_LAYER5:
   {
      setLayerStatus(4);
      break;
   }
   case ID_LAYER_LAYER6:
   {
      setLayerStatus(5);
      break;
   }
   case ID_LAYER_LAYER7:
   {
      setLayerStatus(6);
      break;
   }
   case ID_LAYER_LAYER8:
   {
      setLayerStatus(7);
      break;
   }
   case ID_LAYER_MERGEALL:
   {
      ptCur = GetActiveTable();
      if (!ptCur) break;
      HMENU hmenu = ::GetMenu(m_hwnd);
      ptCur->MergeAllLayers();
      for (int i = 0; i < 8; i++) ptCur->m_activeLayers[i] = false;
      for (int i = 0; i < 8; i++) setLayerStatus(i);
      break;
   }
   case ID_LAYER_TOGGLEALL:
   {
      ptCur = GetActiveTable();
      if (!ptCur) break;
      HMENU hmenu = ::GetMenu(m_hwnd);
      for (int i = 0; i < 8; i++) ptCur->m_activeLayers[i] = !ptCur->m_toggleAllLayers;
      for (int i = 0; i < 8; i++) setLayerStatus(i);
      ptCur->m_toggleAllLayers ^= true;
      break;
   }
   case ID_HELP_ABOUT:
   {
       ShowSubDialog(m_aboutDialog);
      break;
   }
   case ID_WINDOW_CASCADE:
   {
      SendMessage(m_hwndWork, WM_MDICASCADE, 0, 0);
      break;
   }
   case ID_WINDOW_TILE:
   {
      SendMessage(m_hwndWork, WM_MDITILE, 0, 0);
      break;
   }
   case ID_WINDOW_ARRANGEICONS:
   {
      SendMessage(m_hwndWork, WM_MDIICONARRANGE, 0, 0);
      break;
   }
   }
}

void VPinball::ReInitPinDirectSound()
{
	for (int i = 0; i < m_vtable.Size(); i++)
	{
		PinTable * const ptT = m_vtable.ElementAt(i);
		for (int j = 0; j < ptT->m_vsound.Size(); j++)
		{
			ptT->m_vsound.ElementAt(j)->UnInitialize();
		}
	}
	InitPinDirectSound();
	for (int i = 0; i < m_vtable.Size(); i++)
	{
		PinTable * const ptT = m_vtable.ElementAt(i);
		for (int j = 0; j < ptT->m_vsound.Size(); j++)
		{
			ptT->m_vsound.ElementAt(j)->ReInitialize();
		}
	}
}

void VPinball::setLayerStatus(int layerNumber)
{
   CComObject<PinTable> *ptCur;

   ptCur = GetActiveTable();
   if (!ptCur || layerNumber > 7) return;

   SendMessage(m_hwndToolbarLayers, TB_CHECKBUTTON, allLayers[layerNumber], MAKELONG((!ptCur->m_activeLayers[layerNumber]), 0));

   ptCur->SwitchToLayer(layerNumber);
}


void VPinball::SetEnablePalette()
{
   PinTable * const ptCur = GetActiveTable();

   bool fTableActive = (ptCur != NULL) && !g_pplayer;

   // if we can't view the table elements then make the table as not active as that
   // ensure all menu and toolbars are disabled.
   if (ptCur)
   {
      if (ptCur->CheckPermissions(DISABLE_TABLEVIEW))
         fTableActive = false;
   }

   const unsigned state = (m_fBackglassView ? VIEW_BACKGLASS : VIEW_PLAYFIELD);

   for (unsigned int i = 0; i < TBCOUNTPALETTE; ++i)
   {
      const int id = g_tbbuttonPalette[i].idCommand;

      // Targets don't have their own Editable type, they're just surfaces
      ItemTypeEnum type = EditableRegistry::TypeFromToolID(id);
      const unsigned int enablecode = EditableRegistry::GetAllowedViews(type);

      const bool fEnable = fTableActive && ((enablecode & state) != 0);

      // Set toolbar state
      SendMessage(m_hwndToolbarPalette, TB_ENABLEBUTTON, id, MAKELONG(fEnable, 0));

      // Set menu item state
      HMENU hmenuInsert = GetMainMenu(INSERTMENU);
      EnableMenuItem(hmenuInsert, id, MF_BYCOMMAND | (fEnable ? MF_ENABLED : MF_GRAYED));
   }
}


void VPinball::SetEnableToolbar()
{
   PinTable *ptCur = GetActiveTable();

   const bool fTableActive = (ptCur != NULL) && !g_pplayer;

   static const int toolList[] = {
      ID_TABLE_MAGNIFY,
      IDC_SELECT,
      ID_EDIT_PROPERTIES,
      ID_EDIT_SCRIPT,
      ID_EDIT_BACKGLASSVIEW,
      ID_TABLE_PLAY
   };

   for (unsigned int i = 0; i < 6; ++i)
   {
      const int id = toolList[i];
      bool fEnable = fTableActive;

      if (ptCur)
      {
         if ((id == ID_EDIT_SCRIPT) && ptCur->CheckPermissions(DISABLE_SCRIPT_EDITING))
            fEnable = false;
      }

      // Set toolbar state
      SendMessage(m_hwndToolbarMain, TB_ENABLEBUTTON, id, MAKELONG(fEnable, 0));
   }

   // set layer button states
   if (ptCur)
   {
      for (int i = 0; i < 8; ++i)
      {
         SendMessage(m_hwndToolbarLayers, TB_CHECKBUTTON, allLayers[i], MAKELONG(ptCur->m_activeLayers[i], 0));
      }
   }

   SetEnablePalette();
   ParseCommand(ID_EDIT_PROPERTIES, m_hwnd, 2);//redisplay 
}

void VPinball::DoPlay(bool _cameraMode)
{
   NumVideoBytes = 0;
   CComObject<PinTable> * const ptCur = GetActiveTable();
   if (ptCur)
      ptCur->Play(_cameraMode);
}

bool VPinball::LoadFile()
{
   char szFileName[1024];
   char szInitialDir[1024];
   szFileName[0] = '\0';

   OPENFILENAME ofn;
   ZeroMemory(&ofn, sizeof(OPENFILENAME));
   ofn.lStructSize = sizeof(OPENFILENAME);
   ofn.hInstance = g_hinst;
   ofn.hwndOwner = g_pvp->m_hwnd;
   // TEXT
   ofn.lpstrFilter = "Visual Pinball Tables (*.vpx)\0*.vpx\0Old Visual Pinball Tables(*.vpt)\0*.vpt\0";
   ofn.lpstrFile = szFileName;
   ofn.nMaxFile = _MAX_PATH;
   ofn.lpstrDefExt = "vpx";
   ofn.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;

   const HRESULT hr = GetRegString("RecentDir", "LoadDir", szInitialDir, 1024);
   char szFoo[MAX_PATH];
   if (hr == S_OK)
   {
      ofn.lpstrInitialDir = szInitialDir;
   }
   else
   {
      lstrcpy(szFoo, m_szMyPath);
      lstrcat(szFoo, "Tables");
      ofn.lpstrInitialDir = szFoo;
   }

   const int ret = GetOpenFileName(&ofn);

   if (ret == 0)
      return false;

   LoadFileName(szFileName);

   return true;
}

void VPinball::LoadFileName(char *szFileName)
{
   PathFromFilename(szFileName, m_currentTablePath);
   CloseAllDialogs();

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
      ppt->InitPostLoad(this);
      
	  TitleFromFilename(szFileName, ppt->m_szTitle);
      
	  ppt->SetCaption(ppt->m_szTitle);

	  // auto-import POV settings if exist...

	  char szFileNameAuto[MAX_PATH];
	  strcpy_s(szFileNameAuto, m_currentTablePath);
	  strcat_s(szFileNameAuto, ppt->m_szTitle);
	  strcat_s(szFileNameAuto, ".pov");
	  if (Exists(szFileNameAuto)) // We check if there is a table pov settings first
	  {
		  ppt->ImportBackdropPOV(szFileNameAuto);
	  }
	  else // Otherwise, we seek for autopov settings
	  {
		  strcpy_s(szFileNameAuto, m_currentTablePath);
		  strcat_s(szFileNameAuto, "autopov.pov");
		  if (Exists(szFileNameAuto))
		  {
			  ppt->ImportBackdropPOV(szFileNameAuto);
		  }
	  }

      // get the load path from the filename
	  SetRegValue("RecentDir", "LoadDir", REG_SZ, m_currentTablePath, lstrlen(m_currentTablePath));

      // make sure the load directory is the active directory
	  SetCurrentDirectory(m_currentTablePath);

      UpdateRecentFileList(szFileName);

      SetEnableToolbar();
   }
}

CComObject<PinTable> *VPinball::GetActiveTable()
{
   HWND hwndT = (HWND)SendMessage(m_hwndWork, WM_MDIGETACTIVE, 0, 0);

   if (hwndT)
   {
      CComObject<PinTable> *pt = (CComObject<PinTable> *)::GetWindowLongPtr(hwndT, GWLP_USERDATA);
      return pt;
   }
   else
      return NULL;
}

BOOL VPinball::FCanClose()
{
   while (m_vtable.Size())
   {
      const BOOL fCanClose = CloseTable(m_vtable.ElementAt(0));

      if (!fCanClose)
         return fFalse;
   }

   return fTrue;
}


BOOL VPinball::CloseTable(PinTable *ppt)
{
   if ((ppt->FDirty()) && (!ppt->CheckPermissions(DISABLE_TABLE_SAVE)))
   {
      LocalString ls1(IDS_SAVE_CHANGES1);
      LocalString ls2(IDS_SAVE_CHANGES2);
      char *szText = new char[lstrlen(ls1.m_szbuffer) + lstrlen(ls2.m_szbuffer) + lstrlen(ppt->m_szTitle) + 1];
      lstrcpy(szText, ls1.m_szbuffer/*"Do you want to save the changes you made to '"*/);
      lstrcat(szText, ppt->m_szTitle);
      lstrcat(szText, ls2.m_szbuffer);
      // TEXT
      const int result = ::MessageBox(m_hwnd, szText, "Visual Pinball", MB_YESNOCANCEL | MB_DEFBUTTON3 | MB_ICONWARNING);
      delete[] szText;
      if (result == IDCANCEL)
         return fFalse;

      if (result == IDYES)
      {
         if (ppt->TableSave() != S_OK)
         {
            LocalString ls3(IDS_SAVEERROR);
            ::MessageBox(m_hwnd, ls3.m_szbuffer, "Visual Pinball", MB_ICONERROR);
            return fFalse;
         }
      }
   }

   if (ppt->m_searchSelectDlg.IsWindow())
      ppt->m_searchSelectDlg.Destroy();

   CloseAllDialogs();

   ppt->FVerifySaveToClose();

   if (m_sb.GetBaseISel() && (ppt == m_sb.GetBaseISel()->GetPTable()))
      SetPropSel(NULL);

   m_vtable.RemoveElement(ppt);
   ppt->m_pcv->CleanUpScriptEngine();
   ppt->Release();

   SetEnableToolbar();

   return fTrue;
}

void VPinball::ShowPermissionError()
{
   LocalString ls(IDS_PERMISSION_ERROR);
   ::MessageBox(m_hwnd, ls.m_szbuffer, "Visual Pinball", MB_ICONWARNING);
}

void VPinball::SetEnableMenuItems()
{
   CComObject<PinTable> * const ptCur = GetActiveTable();

   // Set menu item to the correct state
   HMENU hmenu = ::GetMenu(m_hwnd);

   CheckMenuItem(hmenu, ID_EDIT_PROPERTIES, MF_BYCOMMAND | (m_sb.GetVisible() ? MF_CHECKED : MF_UNCHECKED));
   CheckMenuItem(hmenu, ID_EDIT_BACKGLASSVIEW, MF_BYCOMMAND | (m_fBackglassView ? MF_CHECKED : MF_UNCHECKED));

   // is there a valid table??
   if (ptCur)
   {
      EnableMenuItem(hmenu, IDM_CLOSE, MF_BYCOMMAND | MF_ENABLED);
      EnableMenuItem(hmenu, ID_EDIT_UNDO, MF_BYCOMMAND | MF_ENABLED);
      EnableMenuItem(hmenu, ID_EDIT_BACKGLASSVIEW, MF_BYCOMMAND | MF_ENABLED);
      EnableMenuItem(hmenu, ID_TABLE_PLAY, MF_BYCOMMAND | MF_ENABLED);
      EnableMenuItem(hmenu, ID_TABLE_CAMERAMODE, MF_BYCOMMAND | MF_ENABLED);
      EnableMenuItem(hmenu, ID_TABLE_MAGNIFY, MF_BYCOMMAND | MF_ENABLED);
      EnableMenuItem(hmenu, ID_TABLE_TABLEINFO, MF_BYCOMMAND | MF_ENABLED);
      EnableMenuItem(hmenu, ID_EDIT_SEARCH, MF_BYCOMMAND | MF_ENABLED);
      EnableMenuItem(hmenu, ID_EDIT_DRAWINGORDER_HIT, MF_BYCOMMAND | MF_ENABLED);
      EnableMenuItem(hmenu, ID_EDIT_DRAWINGORDER_SELECT, MF_BYCOMMAND | MF_ENABLED);
      // enable/disable save options
      UINT flags = MF_BYCOMMAND | (ptCur->CheckPermissions(DISABLE_TABLE_SAVE) ? MF_GRAYED : MF_ENABLED);
      EnableMenuItem(hmenu, IDM_SAVE, flags);
      EnableMenuItem(hmenu, IDM_SAVEAS, flags);
      EnableMenuItem(hmenu, IDM_SAVEASPROTECTED, flags);
      EnableMenuItem(hmenu, ID_FILE_EXPORT_BLUEPRINT, flags);
      EnableMenuItem(hmenu, ID_EXPORT_TABLEMESH, flags);
      EnableMenuItem(hmenu, ID_EXPORT_BACKDROPPOV, flags);
      EnableMenuItem(hmenu, ID_IMPORT_BACKDROPPOV, flags);

      // if we can do a normal save but not a protected save then disable 'save as protected'
      // (if we cant do any saves it is already disabled)
      if ((!ptCur->CheckPermissions(DISABLE_TABLE_SAVE)) &&
         (ptCur->CheckPermissions(DISABLE_TABLE_SAVEPROT)))
         EnableMenuItem(hmenu, IDM_SAVEASPROTECTED, MF_BYCOMMAND | MF_GRAYED);

      // enable/disable script option
      flags = MF_BYCOMMAND | (ptCur->CheckPermissions(DISABLE_SCRIPT_EDITING) ? MF_GRAYED : MF_ENABLED);
      EnableMenuItem(hmenu, ID_EDIT_SCRIPT, flags);

      // enable/disable managers options
      flags = MF_BYCOMMAND | (ptCur->CheckPermissions(DISABLE_OPEN_MANAGERS) ? MF_GRAYED : MF_ENABLED);
      EnableMenuItem(hmenu, ID_TABLE_SOUNDMANAGER, flags);
      EnableMenuItem(hmenu, ID_TABLE_IMAGEMANAGER, flags);
      EnableMenuItem(hmenu, ID_TABLE_FONTMANAGER, flags);
      EnableMenuItem(hmenu, ID_TABLE_MATERIALMANAGER, flags);
      EnableMenuItem(hmenu, ID_TABLE_COLLECTIONMANAGER, flags);

      // enable/disable editing options
      flags = MF_BYCOMMAND | (ptCur->CheckPermissions(DISABLE_CUTCOPYPASTE) ? MF_GRAYED : MF_ENABLED);
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

      CheckMenuItem(hmenu, ID_VIEW_SOLID, MF_BYCOMMAND | (ptCur->RenderSolid() ? MF_CHECKED : MF_UNCHECKED));
      CheckMenuItem(hmenu, ID_VIEW_OUTLINE, MF_BYCOMMAND | (ptCur->RenderSolid() ? MF_UNCHECKED : MF_CHECKED));

      CheckMenuItem(hmenu, ID_VIEW_GRID, MF_BYCOMMAND | (ptCur->m_fGrid ? MF_CHECKED : MF_UNCHECKED));
      CheckMenuItem(hmenu, ID_VIEW_BACKDROP, MF_BYCOMMAND | (ptCur->m_fBackdrop ? MF_CHECKED : MF_UNCHECKED));

      for (int i = 0; i < sizeof(allLayers) / sizeof(allLayers[0]); ++i)
         CheckMenuItem(hmenu, allLayers[i], MF_BYCOMMAND | (ptCur->m_activeLayers[i] ? MF_CHECKED : MF_UNCHECKED));
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
      EnableMenuItem(hmenu, ID_EXPORT_TABLEMESH, MF_BYCOMMAND | MF_GRAYED);
      EnableMenuItem(hmenu, ID_EXPORT_BACKDROPPOV, MF_BYCOMMAND | MF_GRAYED);
      EnableMenuItem(hmenu, ID_IMPORT_BACKDROPPOV, MF_BYCOMMAND | MF_GRAYED);

      EnableMenuItem(hmenu, ID_EDIT_UNDO, MF_BYCOMMAND | MF_GRAYED);
      EnableMenuItem(hmenu, IDC_COPY, MF_BYCOMMAND | MF_GRAYED);
      EnableMenuItem(hmenu, IDC_PASTE, MF_BYCOMMAND | MF_GRAYED);
      EnableMenuItem(hmenu, IDC_PASTEAT, MF_BYCOMMAND | MF_GRAYED);
      EnableMenuItem(hmenu, ID_DELETE, MF_BYCOMMAND | MF_GRAYED);
      EnableMenuItem(hmenu, ID_EDIT_SCRIPT, MF_BYCOMMAND | MF_GRAYED);
      EnableMenuItem(hmenu, ID_EDIT_BACKGLASSVIEW, MF_BYCOMMAND | MF_GRAYED);

      EnableMenuItem(hmenu, ID_TABLE_PLAY, MF_BYCOMMAND | MF_GRAYED);
      EnableMenuItem(hmenu, ID_TABLE_CAMERAMODE, MF_BYCOMMAND | MF_GRAYED);
      EnableMenuItem(hmenu, ID_TABLE_SOUNDMANAGER, MF_BYCOMMAND | MF_GRAYED);
      EnableMenuItem(hmenu, ID_TABLE_IMAGEMANAGER, MF_BYCOMMAND | MF_GRAYED);
      EnableMenuItem(hmenu, ID_TABLE_FONTMANAGER, MF_BYCOMMAND | MF_GRAYED);
      EnableMenuItem(hmenu, ID_TABLE_MATERIALMANAGER, MF_BYCOMMAND | MF_GRAYED);
      EnableMenuItem(hmenu, ID_TABLE_COLLECTIONMANAGER, MF_BYCOMMAND | MF_GRAYED);
      EnableMenuItem(hmenu, ID_TABLE_TABLEINFO, MF_BYCOMMAND | MF_GRAYED);
      EnableMenuItem(hmenu, ID_TABLE_MAGNIFY, MF_BYCOMMAND | MF_GRAYED);
      EnableMenuItem(hmenu, ID_EDIT_SEARCH, MF_BYCOMMAND | MF_GRAYED);
      EnableMenuItem(hmenu, ID_EDIT_DRAWINGORDER_HIT, MF_BYCOMMAND | MF_GRAYED);
      EnableMenuItem(hmenu, ID_EDIT_DRAWINGORDER_SELECT, MF_BYCOMMAND | MF_GRAYED);
   }
}

void VPinball::UpdateRecentFileList(char *szfilename)
{
   // if the loaded file name is not null then add it to the top of the list
   if (szfilename != NULL)
   {
      // does this file name aready exist in the list?
      bool bFound = false;
      int i;
      for (i = 0; i < LAST_OPENED_TABLE_COUNT; i++)
      {
         if (strcmp(m_szRecentTableList[i], szfilename) == 0)
         {
            // yes it does
            bFound = true;
            break;
         }
      }

      // if the entry is already in the list then copy all the items above it down one position
      const int index = (bFound) ? i - 1 :
         // else copy the entire list down
         (LAST_OPENED_TABLE_COUNT - 2);

      // copy the entrys in the list down one position
      for (i = index; i >= 0; i--)
      {
         memcpy(m_szRecentTableList[i + 1], m_szRecentTableList[i], MAX_PATH);
      }
      // copy the current file into the first position
      memcpy(m_szRecentTableList[0], szfilename, MAX_PATH);

      // write the list of the last n loaded tables to the registry
      for (i = 0; i < LAST_OPENED_TABLE_COUNT; i++)
      {
         char szRegName[MAX_PATH];

         // if this entry is empty then all the rest are empty
         if (m_szRecentTableList[i][0] == 0x00) break;
         // write entry to the registry
         sprintf_s(szRegName, "TableFileName%d", i);
         SetRegValue("RecentDir", szRegName, REG_SZ, m_szRecentTableList[i], lstrlen(m_szRecentTableList[i]) + 1);
      }
   }

   // update the file menu to contain the last n recent loaded files
   // must be at least 1 recent file in the list
   if (m_szRecentTableList[0][0] != 0x00)
   {
      MENUITEMINFO menuInfo;

      // update the file menu to contain the last n recent loaded files
      HMENU hmenuFile = GetMainMenu(FILEMENU);

      // delete all the recent file IDM's from this menu
      for (int i = RECENT_FIRST_MENU_IDM; i <= RECENT_LAST_MENU_IDM; i++)
         DeleteMenu(hmenuFile, i, MF_BYCOMMAND);

      // get the number of entrys in the file menu
      // insert the items before the EXIT menu (assuming it is the last entry)
      int count = GetMenuItemCount(hmenuFile) - 1;

      // set up the menu info block
      ZeroMemory(&menuInfo, sizeof(menuInfo));
      menuInfo.cbSize = sizeof(menuInfo);
      menuInfo.fMask = MIIM_ID | MIIM_TYPE;
      menuInfo.fType = MFT_STRING;

      // add in the list of recently accessed files
      for (int i = 0; i < LAST_OPENED_TABLE_COUNT; i++)
      {
         // if this entry is empty then all the rest are empty
         if (m_szRecentTableList[i][0] == 0x00) break;
         _itoa_s(i + 1, recentNumber, 10);
         strcpy_s(recentMenuname, "&");
         strcat_s(recentMenuname, recentNumber);
         strcat_s(recentMenuname, " ");
         strcat_s(recentMenuname, m_szRecentTableList[i]);
         // set the IDM of this menu item
         menuInfo.wID = RECENT_FIRST_MENU_IDM + i;
         menuInfo.dwTypeData = recentMenuname;
         menuInfo.cch = lstrlen(recentMenuname);

         InsertMenuItem(hmenuFile, count, TRUE, &menuInfo);
         count++;
      }

      // add a separator onto the end
      menuInfo.fType = MFT_SEPARATOR;
      menuInfo.wID = RECENT_LAST_MENU_IDM;
      InsertMenuItem(hmenuFile, count, TRUE, &menuInfo);

      // update the menu bar
      ::DrawMenuBar(m_hwnd);
   }
}

BOOL VPinball::processKeyInputForDialogs(MSG *pmsg)
{
    if(g_pvp->m_ptableActive)
    {
        if(g_pvp->m_materialDialog.IsWindow())
            return g_pvp->m_materialDialog.IsDialogMessage(*pmsg);
        if(g_pvp->m_imageMngDlg.IsWindow())
            return g_pvp->m_imageMngDlg.IsDialogMessage(*pmsg);
        if(g_pvp->m_soundMngDlg.IsWindow())
            return g_pvp->m_soundMngDlg.IsDialogMessage(*pmsg);
        if(g_pvp->m_collectionMngDlg.IsWindow())
            return g_pvp->m_collectionMngDlg.IsDialogMessage(*pmsg);
        if(g_pvp->m_dimensionDialog.IsWindow())
            return g_pvp->m_dimensionDialog.IsDialogMessage(*pmsg);
    }
    return false;
}

HRESULT VPinball::ApcHost_OnTranslateMessage(MSG* pmsg, BOOL* pfConsumed)
{
   *pfConsumed = FALSE;

   if (!g_pplayer)
   {
       *pfConsumed = processKeyInputForDialogs(pmsg);
       if(*pfConsumed)
           return NOERROR;

      for (unsigned i = 0; i < m_sb.m_vhwndDialog.size(); i++)
      {
         if (::IsDialogMessage(m_sb.m_vhwndDialog[i], pmsg))
            *pfConsumed = TRUE;
      }
      if (m_pcv && m_pcv->m_hwndMain)
      {
         //if (pmsg->hwnd == m_pcv->m_hwndMain)
         {
            int fTranslated = fFalse;

            if ((pmsg->hwnd == m_pcv->m_hwndMain) || ::IsChild(m_pcv->m_hwndMain, pmsg->hwnd))
               fTranslated = TranslateAccelerator(m_pcv->m_hwndMain, m_pcv->m_haccel, pmsg);

            if (fTranslated)
               *pfConsumed = TRUE;
            else
            {
               if (::IsDialogMessage(m_pcv->m_hwndMain, pmsg))
                  *pfConsumed = TRUE;
            }
         }
      }

      if (m_pcv && m_pcv->m_hwndFind)
      {
         if (::IsDialogMessage(m_pcv->m_hwndFind, pmsg))
            *pfConsumed = TRUE;
      }      
      if (!(*pfConsumed))
      {
         const int fTranslated = TranslateAccelerator(m_hwnd, g_haccel, pmsg);

         if (fTranslated != 0)
            *pfConsumed = TRUE;
      }

      if (!(*pfConsumed))
         /*const int fTranslated =*/ TranslateMessage(pmsg);
   }
   else
   {
      if (g_pplayer->m_fDebugMode)
      {
         if (::IsDialogMessage(g_pplayer->m_hwndDebugger, pmsg))
            *pfConsumed = TRUE;
         else if (::IsDialogMessage(g_pplayer->m_hwndLightDebugger, pmsg))
               *pfConsumed = TRUE;
         else if (::IsDialogMessage(g_pplayer->m_hwndMaterialDebugger, pmsg))
            *pfConsumed = TRUE;
      }
   }

   return NOERROR;
}

HRESULT VPinball::MainMsgLoop()
{
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
            DispatchMessage(&msg);
      }
      else
      {
         if (g_pplayer && !g_pplayer->m_fPause)
            ApcHost_OnIdle(&fConsumed);
         else
            WaitMessage();
      }
   }

   return S_OK;
}

HRESULT VPinball::ApcHost_OnIdle(BOOL* pfContinue)
{
   g_pplayer->Render();
   *pfContinue = TRUE;

   return S_OK;
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


HRESULT VPinball::GetTypeLibInfo(
   HINSTANCE    *phinstOut,
   const GUID  **pplibidOut,
   SHORT        *pwMajLib,
   SHORT        *pwMinLib,
   const CLSID **ppclsidOut,
   const IID   **ppiidOut,
   ITypeLib   ***ppptlOut)
{
   return S_OK;
}

STDMETHODIMP_(ULONG) VPinball::AddRef()
{
   ASSERT(m_cref, "bad m_cref");
   return ++m_cref;
}

STDMETHODIMP_(ULONG) VPinball::Release()
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

void VPinball::OnClose()
{
   PinTable *ptable = g_pvp->GetActiveTable();
   if (ptable)
   {
      while (ptable->m_savingActive)
         Sleep(THREADS_PAUSE);
   }
   if (g_pplayer)
      SendMessage(g_pplayer->m_hwnd, WM_CLOSE, 0, 0);

   BOOL fCanClose = g_pvp->FCanClose();
   if (fCanClose)
   {
      WINDOWPLACEMENT winpl;
      winpl.length = sizeof(winpl);

      if (::GetWindowPlacement(m_hwnd, &winpl))
      {
         SetRegValue("Editor", "WindowLeft", REG_DWORD, &winpl.rcNormalPosition.left, 4);
         SetRegValue("Editor", "WindowTop", REG_DWORD, &winpl.rcNormalPosition.top, 4);
         SetRegValue("Editor", "WindowRight", REG_DWORD, &winpl.rcNormalPosition.right, 4);
         SetRegValue("Editor", "WindowBottom", REG_DWORD, &winpl.rcNormalPosition.bottom, 4);

         BOOL fMaximized = ::IsZoomed(m_hwnd);
         SetRegValue("Editor", "WindowMaximized", REG_DWORD, &fMaximized, 4);
      }
      CWnd::OnClose();
   }
}

void VPinball::ShowSubDialog( CDialog &dlg )
{
    if (!dlg.IsWindow())
    {
       dlg.Create(m_hwnd);
       dlg.ShowWindow();
    }
    else
       dlg.SetForegroundWindow();
}

#if 0
int VPinball::OnCreate(CREATESTRUCT& cs)
{
    // OnCreate controls the way the frame is created.
    // Overriding CFrame::OnCreate is optional.
    // Uncomment the lines below to change frame options.

    SetUseIndicatorStatus(FALSE);	// Don't show keyboard indicators in the StatusBar
    SetUseMenuStatus(FALSE);			// Don't show menu descriptions in the StatusBar
    SetUseReBar(FALSE);				// Don't use a ReBar
    SetUseThemes(FALSE);				// Don't use themes
    SetUseToolBar(FALSE);			// Don't use a ToolBar


    // call the base class function
    return CMDIDockFrame::OnCreate(cs);
}

LRESULT VPinball::WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HWND hwnd = GetHwnd();
    switch(uMsg)
    {

        case WM_DESTROY:
        PostMessage(hwnd, WM_QUIT, 0, 0);
        break;

        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = ::BeginPaint(hwnd, &ps);
            RECT rc;
            ::GetClientRect(hwnd, &rc);
            SelectObject(hdc, GetStockObject(WHITE_BRUSH));
            PatBlt(hdc, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, PATCOPY);
            ::EndPaint(hwnd, &ps);
        }
        break;

        case WM_SIZE:
        if(g_pvp && g_pvp->m_hwndSideBar)
        {
            RECT rc;
            ::GetClientRect(hwnd, &rc);

            SendMessage(g_pvp->m_hwndStatusBar, WM_SIZE, wParam, lParam);

            RECT rcStatus;
            ::GetWindowRect(g_pvp->m_hwndStatusBar, &rcStatus);
            const int statheight = rcStatus.bottom - rcStatus.top;

            //const int scrollwindowtop = 48*(TBCOUNTMAIN/2);
            const int scrollwindowtop = 48 * (TBCOUNTMAIN / 2) + 28 * (TBCOUNTLAYERS / 2);
            const int scrollwindowheight = rc.bottom - rc.top - statheight - scrollwindowtop;
            ::SetWindowPos(g_pvp->m_hwndSideBarScroll, NULL, 0, scrollwindowtop, TOOLBAR_WIDTH + SCROLL_WIDTH, scrollwindowheight, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER);

            HWND hwndSB = g_pvp->m_sb.GetHWnd();
            int SBwidth = g_pvp->m_sb.m_maxdialogwidth;

            if(g_pvp->m_fPropertiesFloating)
                SBwidth = 0;
            else
            {
                if(g_pvp->m_sb.GetVisible())
                {
                    ::SetWindowPos(hwndSB, NULL, rc.right - rc.left - SBwidth, 0, SBwidth, rc.bottom - rc.top - statheight, SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOZORDER);
                }
                else
                    SBwidth = 0;
            }

            // Set scroll info for the palette scrollbar
            SCROLLINFO si;
            const size_t padding = SendMessage(g_pvp->m_hwndToolbarPalette, TB_GETPADDING, 0, 0);
            const size_t buttonsize = SendMessage(g_pvp->m_hwndToolbarPalette, TB_GETBUTTONSIZE, 0, 0);
            const int vertpadding = HIWORD(padding);
            const int vertbutsize = HIWORD(buttonsize);
            ZeroMemory(&si, sizeof(SCROLLINFO));
            si.cbSize = sizeof(si);
            si.fMask = SIF_ALL;
            si.nMin = 0;
            si.nMax = ((vertbutsize + vertpadding) * (TBCOUNTPALETTE / 2)) + 4; // Add 4 padding
            si.nPage = scrollwindowheight;
            si.nPos = g_pvp->m_palettescroll;

            ::SetScrollInfo(g_pvp->m_hwndSideBarScroll, SB_VERT, &si, TRUE);

            // check if we have any blank space at the bottom and fill it in by moving the scrollbar up
            if((int)(si.nPos + si.nPage) > si.nMax)
            {
                g_pvp->m_palettescroll = si.nMax - si.nPage;
                if(g_pvp->m_palettescroll < 0)
                    g_pvp->m_palettescroll = 0;

                ::SetScrollPos(hwnd, SB_VERT, g_pvp->m_palettescroll, TRUE);

                ::SetWindowPos(g_pvp->m_hwndToolbarPalette, NULL, 0, -g_pvp->m_palettescroll, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER);
            }

            int sidebarwidth = TOOLBAR_WIDTH;
            if(scrollwindowheight < si.nMax)
                sidebarwidth += SCROLL_WIDTH;

            if(g_pvp->m_fPropertiesFloating && hwndSB)
            {
                RECT smartRect;
                ::GetWindowRect(hwndSB, &smartRect);

                int sbHeight = smartRect.bottom - smartRect.top;//(rc.bottom - rc.top) - 100;
                int smartWidth = smartRect.right - smartRect.left;
                int sbX = rc.right - eSmartBrowserWidth - 20;

                ::SetWindowPos(hwndSB, NULL, sbX, 40, smartWidth, sbHeight, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER);
            }
            ::SetWindowPos(g_pvp->m_hwndSideBar, NULL, 0, 0, sidebarwidth, rc.bottom - rc.top - statheight, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER);

            ::SetWindowPos(g_pvp->m_hwndWork, NULL, sidebarwidth, 0, rc.right - rc.left - (sidebarwidth)-SBwidth, rc.bottom - rc.top - statheight, SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOZORDER);
            return 0;
        }
        break;

        case WM_COMMAND:
        g_pvp->ParseCommand(LOWORD(wParam), (HWND)lParam, HIWORD(wParam));
        break;

        case WM_INITMENUPOPUP:
        g_pvp->SetEnableMenuItems();
        break;

    }
    return WndProcDefault(uMsg, wParam, lParam);
}
#endif 

LRESULT CALLBACK VPWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {

        case WM_DESTROY:
        PostMessage(hwnd, WM_QUIT, 0, 0);
        break;

        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = ::BeginPaint(hwnd, &ps);
            RECT rc;
            ::GetClientRect(hwnd, &rc);
            SelectObject(hdc, GetStockObject(WHITE_BRUSH));
            PatBlt(hdc, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, PATCOPY);
            ::EndPaint(hwnd, &ps);
        }
        break;

        case WM_SIZE:
        if(g_pvp && g_pvp->m_hwndSideBar)
        {
            RECT rc;
            ::GetClientRect(hwnd, &rc);

            SendMessage(g_pvp->m_hwndStatusBar, WM_SIZE, wParam, lParam);

            RECT rcStatus;
            ::GetWindowRect(g_pvp->m_hwndStatusBar, &rcStatus);
            const int statheight = rcStatus.bottom - rcStatus.top;

            //const int scrollwindowtop = 48*(TBCOUNTMAIN/2);
            const int scrollwindowtop = 48 * (TBCOUNTMAIN / 2) + 28 * (TBCOUNTLAYERS / 2);
            const int scrollwindowheight = rc.bottom - rc.top - statheight - scrollwindowtop;
            ::SetWindowPos(g_pvp->m_hwndSideBarScroll, NULL, 0, scrollwindowtop, TOOLBAR_WIDTH + SCROLL_WIDTH, scrollwindowheight, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER);

            HWND hwndSB = g_pvp->m_sb.GetHWnd();
            int SBwidth = g_pvp->m_sb.m_maxdialogwidth;

            if(g_pvp->m_fPropertiesFloating)
                SBwidth = 0;
            else
            {
                if(g_pvp->m_sb.GetVisible())
                {
                    ::SetWindowPos(hwndSB, NULL, rc.right - rc.left - SBwidth, 0, SBwidth, rc.bottom - rc.top - statheight, SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOZORDER);
                }
                else
                    SBwidth = 0;
            }

            // Set scroll info for the palette scrollbar
            SCROLLINFO si;
            const size_t padding = SendMessage(g_pvp->m_hwndToolbarPalette, TB_GETPADDING, 0, 0);
            const size_t buttonsize = SendMessage(g_pvp->m_hwndToolbarPalette, TB_GETBUTTONSIZE, 0, 0);
            const int vertpadding = HIWORD(padding);
            const int vertbutsize = HIWORD(buttonsize);
            ZeroMemory(&si, sizeof(SCROLLINFO));
            si.cbSize = sizeof(si);
            si.fMask = SIF_ALL;
            si.nMin = 0;
            si.nMax = ((vertbutsize + vertpadding) * (TBCOUNTPALETTE / 2)) + 4; // Add 4 padding
            si.nPage = scrollwindowheight;
            si.nPos = g_pvp->m_palettescroll;

            ::SetScrollInfo(g_pvp->m_hwndSideBarScroll, SB_VERT, &si, TRUE);

            // check if we have any blank space at the bottom and fill it in by moving the scrollbar up
            if((int)(si.nPos + si.nPage) > si.nMax)
            {
                g_pvp->m_palettescroll = si.nMax - si.nPage;
                if(g_pvp->m_palettescroll < 0)
                    g_pvp->m_palettescroll = 0;

                ::SetScrollPos(hwnd, SB_VERT, g_pvp->m_palettescroll, TRUE);

                ::SetWindowPos(g_pvp->m_hwndToolbarPalette, NULL, 0, -g_pvp->m_palettescroll, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER);
            }

            int sidebarwidth = TOOLBAR_WIDTH;
            if(scrollwindowheight < si.nMax)
                sidebarwidth += SCROLL_WIDTH;

            if(g_pvp->m_fPropertiesFloating && hwndSB)
            {
                RECT smartRect;
                ::GetWindowRect(hwndSB, &smartRect);

                int sbHeight = smartRect.bottom - smartRect.top;//(rc.bottom - rc.top) - 100;
                int smartWidth = smartRect.right - smartRect.left;
                int sbX = rc.right - eSmartBrowserWidth - 20;

                ::SetWindowPos(hwndSB, NULL, sbX, 40, smartWidth, sbHeight, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER);
            }
            ::SetWindowPos(g_pvp->m_hwndSideBar, NULL, 0, 0, sidebarwidth, rc.bottom - rc.top - statheight, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER);

            ::SetWindowPos(g_pvp->m_hwndWork, NULL, sidebarwidth, 0, rc.right - rc.left - (sidebarwidth)-SBwidth, rc.bottom - rc.top - statheight, SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOZORDER);
            return 0;
        }
        break;

        case WM_COMMAND:
        g_pvp->ParseCommand(LOWORD(wParam), (HWND)lParam, HIWORD(wParam));
        break;

        case WM_INITMENUPOPUP:
        g_pvp->SetEnableMenuItems();
        break;

    }
    return g_pvp ? DefFrameProc(hwnd, g_pvp->m_hwndWork, uMsg, wParam, lParam) : 0;
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
               break;

            HMENU hmenu = CreatePopupMenu();

            HWND hwndList = CreateWindowEx(0, "ListBox", "", WS_CHILD | LBS_SORT, 0, 0, 10, 10, hwnd, NULL, g_hinst, 0);

            int menucount = 0;
            for (int i = 0; i < pt->m_vedit.Size(); i++)
            {
               IEditable * const piedit = pt->m_vedit.ElementAt(i);
               // check scriptable - decals don't have scripts and therefore don't have names
               if (piedit->GetScriptable() && piedit->m_fBackglass == g_pvp->m_fBackglassView && piedit->m_isVisible)
               {
                  char szT[64]; // Names can only be 32 characters (plus terminator)
                  WideCharToMultiByte(CP_ACP, 0, pt->m_vedit.ElementAt(i)->GetScriptable()->m_wzName, -1, szT, 64, NULL, NULL);

                  const size_t index = SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)szT);
                  SendMessage(hwndList, LB_SETITEMDATA, index, i + 1);// menu can't have an item with id 0, so bump everything up one
               }
            }

            for (int i = 0; i < pt->m_vcollection.Size(); i++)
            {
               char szT[64]; // Names can only be 32 characters (plus terminator)

               WideCharToMultiByte(CP_ACP, 0, pt->m_vcollection.ElementAt(i)->m_wzName, -1, szT, 64, NULL, NULL);
               strncat_s(szT, " (COL)", 64);
               const size_t index = SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)szT);
               SendMessage(hwndList, LB_SETITEMDATA, index, i | 0x80000000);
            }

            const size_t listcount = SendMessage(hwndList, LB_GETCOUNT, 0, 0);

            // Take the items from our sorted list and put them into the menu
            for (size_t i = 0; i < listcount; i++)
            {
               char szT[64];
               int flags = MF_STRING;

               if ((menucount % 30 == 0) && (menucount != 0))
                  flags |= MF_MENUBARBREAK;

               SendMessage(hwndList, LB_GETTEXT, i, (LPARAM)szT);
               const size_t data = SendMessage(hwndList, LB_GETITEMDATA, i, 0);

               AppendMenu(hmenu, flags, data, szT);
               menucount++;
            }

            DestroyWindow(hwndList);

            POINT mousept;
            GetCursorPos(&mousept);

            const int ksshift = GetKeyState(VK_SHIFT);
            const bool fAdd = ((ksshift & 0x80000000) != 0);

            const int icmd = TrackPopupMenuEx(hmenu, TPM_RETURNCMD | 16384/*TPM_NOANIMATION*/, mousept.x, mousept.y, hwnd, NULL);

            if (icmd != 0)
            {
               if (icmd & 0x80000000) // collection
               {
                  Collection * const pcol = pt->m_vcollection.ElementAt(icmd & 0x7fffffff);
                  for (int i = 0; i < pcol->m_visel.Size(); i++)
                     pt->AddMultiSel(pcol->m_visel.ElementAt(i), i == 0 ? fAdd : true);
               }
               else
                  pt->AddMultiSel(pt->m_vedit.ElementAt(icmd - 1)->GetISelect(), fAdd);
            }
            DestroyMenu(hmenu);
         }
      }
      break;
      }
   }
   break;

   case WM_COMMAND:
      g_pvp->ParseCommand(wParam, (HWND)lParam, (size_t)hwnd);
      break;

   case WM_VSCROLL:
   {
      SCROLLINFO si;
      ZeroMemory(&si, sizeof(SCROLLINFO));
      si.cbSize = sizeof(SCROLLINFO);
      si.fMask = SIF_ALL;
      GetScrollInfo(hwnd, SB_VERT, &si);
      switch (LOWORD(wParam))
      {
      case SB_LINEUP:
         g_pvp->m_palettescroll -= si.nPage / 10;
         break;
      case SB_LINEDOWN:
         g_pvp->m_palettescroll += si.nPage / 10;
         break;
      case SB_PAGEUP:
         g_pvp->m_palettescroll -= si.nPage / 2;
         break;
      case SB_PAGEDOWN:
         g_pvp->m_palettescroll += si.nPage / 2;
         break;
      case SB_THUMBTRACK:
      {
         const int delta = (int)(g_pvp->m_palettescroll - si.nPos);
         g_pvp->m_palettescroll = ((short)HIWORD(wParam) + delta);
      }
      break;
      }

      SetScrollPos(hwnd, SB_VERT, g_pvp->m_palettescroll, TRUE);

      SetWindowPos(g_pvp->m_hwndToolbarPalette, NULL,
         0, -g_pvp->m_palettescroll, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER);
   }
   break;
   }

   return DefWindowProc(hwnd, uMsg, wParam, lParam);
}


STDMETHODIMP VPinball::PlaySound(BSTR bstr)
{
   if (g_pplayer) g_pplayer->m_ptable->PlaySound(bstr, 0, 1.f, 0.f, 0.f, 0, VARIANT_FALSE, VARIANT_TRUE, 0.f);

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


void VPinball::Quit()
{
   if (g_pplayer) {
      g_pplayer->m_fCloseDown = true;
      g_pplayer->m_fCloseType = 1;
   }
   else
      PostMessage(m_hwnd, WM_CLOSE, 0, 0);
}


int CALLBACK MyCompProc(LPARAM lSortParam1, LPARAM lSortParam2, LPARAM lSortOption)
{
   LVFINDINFO lvf;
   char buf1[MAX_PATH], buf2[MAX_PATH];

   SORTDATA *lpsd = (SORTDATA *)lSortOption;

   lvf.flags = LVFI_PARAM;
   lvf.lParam = lSortParam1;
   const int nItem1 = ListView_FindItem(lpsd->hwndList, -1, &lvf);

   lvf.lParam = lSortParam2;
   const int nItem2 = ListView_FindItem(lpsd->hwndList, -1, &lvf);

   ListView_GetItemText(lpsd->hwndList, nItem1, lpsd->subItemIndex, buf1, sizeof(buf1));

   ListView_GetItemText(lpsd->hwndList, nItem2, lpsd->subItemIndex, buf2, sizeof(buf2));
   if(nItem2 == -1 || nItem1 == -1)
       return 0;
   if(lpsd->sortUpDown == 1)
      return(_stricmp(buf1, buf2));
   else
      return(_stricmp(buf1, buf2) * -1);
}

int CALLBACK MyCompProcIntValues(LPARAM lSortParam1, LPARAM lSortParam2, LPARAM lSortOption)
{
    LVFINDINFO lvf;
    char buf1[MAX_PATH], buf2[MAX_PATH];
    int value1, value2;

    SORTDATA *lpsd = (SORTDATA *)lSortOption;

    lvf.flags = LVFI_PARAM;
    lvf.lParam = lSortParam1;
    const int nItem1 = ListView_FindItem(lpsd->hwndList, -1, &lvf);

    lvf.lParam = lSortParam2;
    const int nItem2 = ListView_FindItem(lpsd->hwndList, -1, &lvf);

    ListView_GetItemText(lpsd->hwndList, nItem1, lpsd->subItemIndex, buf1, sizeof(buf1));
    ListView_GetItemText(lpsd->hwndList, nItem2, lpsd->subItemIndex, buf2, sizeof(buf2));
    sscanf_s(buf1, "%i", &value1);
    sscanf_s(buf2, "%i", &value2);

    if(lpsd->sortUpDown == 1)
        return(value1-value2);
    else
        return(value2-value1);
}

const int rgDlgIDFromSecurityLevel[] = { IDC_ACTIVEX0, IDC_ACTIVEX1, IDC_ACTIVEX2, IDC_ACTIVEX3, IDC_ACTIVEX4 };

INT_PTR CALLBACK SecurityOptionsProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
         (rcMain.right + rcMain.left) / 2 - (rcDlg.right - rcDlg.left) / 2,
         (rcMain.bottom + rcMain.top) / 2 - (rcDlg.bottom - rcDlg.top) / 2,
         0, 0, SWP_NOOWNERZORDER | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE/* | SWP_NOMOVE*/);

      int security;
      HRESULT hr = GetRegInt("Player", "SecurityLevel", &security);
      if (hr != S_OK)
         security = DEFAULT_SECURITY_LEVEL;

      if (security < 0 || security > 4)
         security = 0;

      const int buttonid = rgDlgIDFromSecurityLevel[security];

      HWND hwndCheck = GetDlgItem(hwndDlg, buttonid);

      SendMessage(hwndCheck, BM_SETCHECK, BST_CHECKED, 0);

      HWND hwndDetectHang = GetDlgItem(hwndDlg, IDC_HANGDETECT);
      int hangdetect;
      hr = GetRegInt("Player", "DetectHang", &hangdetect);
      if (hr != S_OK)
         hangdetect = fFalse;
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
            for (int i = 0; i < 5; i++)
            {
               HWND hwndCheck = GetDlgItem(hwndDlg, rgDlgIDFromSecurityLevel[i]);
               size_t checked = SendMessage(hwndCheck, BM_GETCHECK, 0, 0);
               if (checked == BST_CHECKED)
                  SetRegValue("Player", "SecurityLevel", REG_DWORD, &i, 4);
            }

            HWND hwndCheck = GetDlgItem(hwndDlg, IDC_HANGDETECT);
            size_t hangdetect = SendMessage(hwndCheck, BM_GETCHECK, 0, 0);
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

INT_PTR CALLBACK FontManagerProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   CCO(PinTable) *pt = (CCO(PinTable) *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

   switch (uMsg)
   {
   case WM_INITDIALOG:
   {
      SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);

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

      pt = (CCO(PinTable) *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

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
            ZeroMemory(&ofn, sizeof(OPENFILENAME));
            ofn.lStructSize = sizeof(OPENFILENAME);
            ofn.hInstance = g_hinst;
            ofn.hwndOwner = g_pvp->m_hwnd;
            // TEXT
            ofn.lpstrFilter = "Font Files (*.ttf)\0*.ttf\0";
            ofn.lpstrFile = szFileName;
            ofn.nMaxFile = _MAX_PATH;
            ofn.lpstrDefExt = "ttf";
            ofn.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;

            HRESULT hr = GetRegString("RecentDir", "FontDir", szInitialDir, 1024);
            ofn.lpstrInitialDir = (hr == S_OK) ? szInitialDir : NULL;

            const int ret = GetOpenFileName(&ofn);
            if (ret)
            {
               strcpy_s(szInitialDir, sizeof(szInitialDir), szFileName);
               szInitialDir[ofn.nFileOffset] = 0;
               SetRegValue("RecentDir", "FontDir", REG_SZ, szInitialDir, lstrlen(szInitialDir));
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
         ZeroMemory(&ofn, sizeof(OPENFILENAME));
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


void VPinball::ShowDrawingOrderDialog(bool select)
{
   DrawingOrderDialog *orderDlg = new DrawingOrderDialog(select);
   orderDlg->DoModal();
   delete orderDlg;
}

void VPinball::CloseAllDialogs()
{
   if (m_imageMngDlg.IsWindow())
      m_imageMngDlg.Destroy();
   if (m_soundMngDlg.IsWindow())
      m_soundMngDlg.Destroy();
   if (m_audioOptDialog.IsWindow())
      m_audioOptDialog.Destroy();
   if (m_editorOptDialog.IsWindow())
      m_editorOptDialog.Destroy();
   if (m_videoOptDialog.IsWindow())
      m_videoOptDialog.Destroy();
   if (m_collectionMngDlg.IsWindow())
      m_collectionMngDlg.Destroy();
   if (m_physicsOptDialog.IsWindow())
      m_physicsOptDialog.Destroy();
   if (m_tableInfoDialog.IsWindow())
      m_tableInfoDialog.Destroy();
   if (m_dimensionDialog.IsWindow())
      m_dimensionDialog.Destroy();
   if (m_materialDialog.IsWindow())
      m_materialDialog.Destroy();
   if (m_aboutDialog.IsWindow())
      m_aboutDialog.Destroy();
}
