// VPinball.cpp: implementation of the VPinball class.
//
////////////////////////////////////////////////////////

#include "StdAfx.h"

#include "resource.h"

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

#define GET_SOUNDDEVICES		WM_USER+103
#define RESET_SoundList_CONTENT	WM_USER+104

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

static const int allLayers[8]=
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

#define TBCOUNTMAIN (sizeof(g_tbbuttonMain) / sizeof(TBBUTTON))
#define TBCOUNTPALETTE (sizeof(g_tbbuttonPalette) / sizeof(TBBUTTON))
#define TBCOUNTLAYERS (sizeof(g_tbbuttonLayers) / sizeof(TBBUTTON))

LRESULT CALLBACK VPWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK VPSideBarWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

INT_PTR CALLBACK DimensionProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK SoundManagerProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK ImageManagerProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK MaterialManagerProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK FontManagerProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK CollectManagerProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK CollectionProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK VideoOptionsProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK AudioOptionsProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK PhysicsOptionsProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK EditorOptionsProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK ProtectTableProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK UnlockTableProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK KeysProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK TableInfoProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK SecurityOptionsProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

INT_PTR CALLBACK AboutProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

INT_PTR CALLBACK SearchSelectProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DrawingOrderProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

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
   *(szEnd+1) = '\0'; // Get rid of exe name

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

   m_scintillaDll = LoadLibrary("SciLexer.DLL");
   if ( m_scintillaDll==NULL )
      ShowError("Unable to load SciLexer.DLL");

   char szName[256];
   LoadString(g_hinst, IDS_PROJNAME, szName, 256);
   // loading String "Visual Pinball" from Exe properties

   const int screenwidth = GetSystemMetrics(SM_CXSCREEN);		// width of primary monitor
   const int screenheight = GetSystemMetrics(SM_CYSCREEN);		// height of primary monitor

   const int x = (screenwidth - MAIN_WINDOW_WIDTH)/2;
   const int y = (screenheight - MAIN_WINDOW_HEIGHT)/2;
   const int width = MAIN_WINDOW_WIDTH;
   const int height = MAIN_WINDOW_HEIGHT;

   LPTSTR lpCmdLine = GetCommandLine();						//this line necessary for _ATL_MIN_CRT

   if( strstr( lpCmdLine, "minimized" ) )
      SetOpenMinimized();

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
            winpl.showCmd |= SW_MINIMIZE;
         else if (hrmax == S_OK && fMaximized)
            winpl.showCmd |= SW_MAXIMIZE;

         SetWindowPlacement(m_hwnd, &winpl);
      }
   }

   ShowWindow(m_hwnd, SW_SHOW);

   SetWindowLongPtr(m_hwnd, GWLP_USERDATA, (size_t)this);	// set this class (vpinball) as callback for MDI Child / has to be confirmed
   // can be a problem for 64 bit compatibility.
   // maybe use SetWindowLongPtr instead

   CreateSideBar();									// Create Sidebar

   CreateMDIClient();								// Create MDI Child

   int foo[4] = {120,240,400,600};

   m_hwndStatusBar = CreateStatusWindow(WS_CHILD | WS_VISIBLE,
      "",
      m_hwnd,
      1);				// Create Status Line at the bottom

   SendMessage(m_hwndStatusBar, SB_SETPARTS, 4, (size_t)foo);	// Initialise Status bar with 4 empty cells

   m_sb.Init(m_hwnd);								// initialize smartbrowser (Property bar on the right) - see propbrowser.cpp

   SendMessage(m_hwnd, WM_SIZE, 0, 0);				// Make our window relay itself out

   InitTools();										// eventually show smartbrowser

   InitRegValues();									// get default values from registry

   int DSidx1 = 0, DSidx2 = 0;
   GetRegInt("Player", "SoundDevice", &DSidx1);
   GetRegInt("Player", "SoundDeviceBG", &DSidx2);

   m_pds.InitDirectSound(m_hwnd, false);						// init Direct Sound (in pinsound.cpp)
   if (DSidx1==DSidx2) // If these are the same device, just point the backglass device to the main one. 
   {
	    m_pbackglassds = &m_pds;
   }
   else
   {
	   m_pbackglassds = new PinDirectSound();
	   m_pbackglassds->InitDirectSound(m_hwnd, true);
   }

   m_fBackglassView = fFalse;						// we are viewing Pinfield and not the backglass at first

   SetEnableToolbar();

   UpdateRecentFileList(NULL);						// update the recent loaded file list

   wintimer_init();								    // calibrate the timer routines

#ifdef SLINTF
   // see slintf.cpp
   slintf_init();								    // initialize debug console (can be popupped by the following command)
   slintf_popup_console();
   slintf("Debug output:\n");
#endif
}

///<summary>
///Ensure that worker thread exists
///<para>Starts worker Thread otherwise</para>
///</summary>
void VPinball::EnsureWorkerThread()
{
   if (!m_workerthread)
   {
      g_hWorkerStarted = CreateEvent(NULL,TRUE,FALSE,NULL);
      m_workerthread = CreateThread(NULL, 0, VPWorkerThreadStart, 0, 0, &m_workerthreadid); //!! _beginthreadex is safer
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
void VPinball::SetAutoSaveMinutes(int minutes)
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

   SendMessage(m_hwndToolbarMain,TB_CHECKBUTTON,IDC_SELECT,MAKELONG(TRUE,0));
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

   m_fAlwaysDrawDragPoints = GetRegIntWithDefault("Editor", "ShowDragPoints", fFalse);
   m_fAlwaysDrawLightCenters = GetRegIntWithDefault("Editor", "DrawLightCenters", fFalse);
   m_gridSize = GetRegIntWithDefault("Editor", "GridSize", 50);

   BOOL fAutoSave = GetRegIntWithDefault("Editor", "AutoSaveOn", fTrue);

   if (fAutoSave)
   {
      m_autosaveTime = GetRegIntWithDefault("Editor", "AutoSaveTime", AUTOSAVE_DEFAULT_TIME);
      SetAutoSaveMinutes(m_autosaveTime);
   }
   else
      m_autosaveTime = -1;

   m_securitylevel = GetRegIntWithDefault("Player", "SecurityLevel", DEFAULT_SECURITY_LEVEL);

   if (m_securitylevel < eSecurityNone || m_securitylevel > eSecurityNoControls)
      m_securitylevel = eSecurityNoControls;

   // get the list of the last n loaded tables
   for (int i=0; i<LAST_OPENED_TABLE_COUNT; i++)
   {
      char szRegName[MAX_PATH];
      sprintf_s(szRegName, "TableFileName%d", i);
      m_szRecentTableList[i][0] = 0x00;
      hr = GetRegString("RecentDir",szRegName, m_szRecentTableList[i], MAX_PATH);
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
   wcex.lpfnWndProc = (WNDPROC) VPWndProc;
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
   wcex.hbrBackground = HBRUSH(COLOR_BTNFACE+1);
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
   GetWindowRect(m_hwnd, &rc);

   m_hwndSideBar = ::CreateWindowEx(/*WS_EX_WINDOWEDGE*/0,"VPStaticChild","",WS_VISIBLE | WS_CHILD | WS_BORDER,
      0,0,TOOLBAR_WIDTH + SCROLL_WIDTH,rc.bottom - rc.top,m_hwnd,NULL,g_hinst,0);

   m_hwndSideBarLayers = ::CreateWindowEx(0,"VPStaticChild","",WS_VISIBLE | WS_CHILD,
      0,48*(TBCOUNTMAIN/2),TOOLBAR_WIDTH + SCROLL_WIDTH,rc.bottom-rc.top,m_hwndSideBar,NULL,g_hinst,0);

   m_hwndSideBarScroll = ::CreateWindowEx(0,"VPStaticChild","",WS_VISIBLE | WS_CHILD | WS_VSCROLL,
      0,28*(TBCOUNTLAYERS/2),TOOLBAR_WIDTH + SCROLL_WIDTH,rc.bottom - rc.top,m_hwndSideBarLayers,NULL,g_hinst,0);

   m_hwndToolbarMain = CreateToolbar((TBBUTTON *)g_tbbuttonMain, TBCOUNTMAIN, m_hwndSideBar);
   m_hwndToolbarLayers = CreateLayerToolbar(m_hwndSideBarLayers);
   m_hwndToolbarPalette = CreateToolbar((TBBUTTON *)g_tbbuttonPalette, TBCOUNTPALETTE, m_hwndSideBarScroll);
   m_palettescroll = 0;
}

HWND VPinball::CreateLayerToolbar(HWND hwndParent)
{
   HWND hwnd = CreateToolbarEx(hwndParent,
      WS_CHILD | WS_VISIBLE | TBSTYLE_BUTTON | TBSTYLE_WRAPABLE,
      1, TBCOUNTLAYERS, g_hinst, IDB_TOOLBAR, g_tbbuttonLayers, TBCOUNTLAYERS, 24, 24, 24, 24,
      sizeof(TBBUTTON));

   SendMessage(hwnd, TB_SETEXTENDEDSTYLE, 0, TBSTYLE_EX_DRAWDDARROWS);

#ifdef IMSPANISH
   SendMessage(m_hwnd, TB_SETBUTTONWIDTH, 0,
      (LPARAM)(DWORD)MAKELONG(50,50));
#elif defined(IMGERMAN)
   SendMessage(m_hwnd, TB_SETBUTTONWIDTH, 0,
      (LPARAM)(DWORD)MAKELONG(50,50));
#else
   SendMessage(hwnd, TB_SETBUTTONWIDTH, 0,
      (LPARAM)(DWORD)MAKELONG(50,50));
#endif

   for (unsigned int i=0;i<TBCOUNTLAYERS;i++)
   {
      TBBUTTONINFO tbbi;
      ZeroMemory(&tbbi,sizeof(TBBUTTONINFO));
      tbbi.cbSize = sizeof(TBBUTTONINFO);
      tbbi.dwMask = TBIF_SIZE | TBIF_COMMAND | TBIF_STATE | TBIF_STYLE;
      SendMessage(hwnd, TB_GETBUTTONINFO, g_tbbuttonLayers[i].idCommand, (LPARAM)&tbbi);
      if (tbbi.fsStyle & TBSTYLE_DROPDOWN)
         tbbi.cx = 48;
      SendMessage(hwnd, TB_SETBUTTONINFO, g_tbbuttonLayers[i].idCommand, (LPARAM)&tbbi);
   }

   SendMessage(hwnd, TB_AUTOSIZE, 0, 0);

   return hwnd;
}

///<summary>
///Creates Buttons in Toolbar-Windows (left Toolbar)
///<param name="*p_tbbutton">Pointer to Buttons as TBBUTTON[]</param>
///<param name="count">Number of Buttons to create</param>
///<param name="hwndParent">Parentwindow (left Toolbar (top or bottom))</param>
///<returns>Handle to Toolbar</returns>
///</summary>
HWND VPinball::CreateToolbar(TBBUTTON *p_tbbutton, int count, HWND hwndParent)
{
   HWND hwnd = CreateToolbarEx(hwndParent,
      WS_CHILD | WS_VISIBLE | TBSTYLE_FLAT | TBSTYLE_WRAPABLE,
      1, count, g_hinst, IDB_TOOLBAR, p_tbbutton, count, 24, 24, 24, 24,
      sizeof(TBBUTTON));

   SendMessage(hwnd, TB_SETEXTENDEDSTYLE, 0, TBSTYLE_EX_DRAWDDARROWS);

#define MAXRESLEN 128

   char szBuf[MAXRESLEN];

   for (int i=0;i<count;i++)
   {
      LoadString(g_hinst, p_tbbutton[i].dwData, szBuf, MAXRESLEN-1);
      szBuf[lstrlen(szBuf) + 1] = 0;  //Double-null terminate.
      /*const int foo =*/ SendMessage(hwnd, TB_ADDSTRING, 0, (LPARAM) szBuf);
   }

#ifdef IMSPANISH
   SendMessage(m_hwnd, TB_SETBUTTONWIDTH, 0,
      (LPARAM)(DWORD)MAKELONG(50,50));
#elif defined(IMGERMAN)
   SendMessage(m_hwnd, TB_SETBUTTONWIDTH, 0,
      (LPARAM)(DWORD)MAKELONG(50,50));
#else
   SendMessage(hwnd, TB_SETBUTTONWIDTH, 0,
      (LPARAM)(DWORD)MAKELONG(50,50));
#endif

   for (int i=0;i<count;i++)
   {
      TBBUTTONINFO tbbi;
      ZeroMemory(&tbbi,sizeof(TBBUTTONINFO));
      tbbi.cbSize = sizeof(TBBUTTONINFO);
      tbbi.dwMask = TBIF_SIZE | TBIF_COMMAND | TBIF_STATE | TBIF_STYLE;
      /*int foo =*/ SendMessage(hwnd, TB_GETBUTTONINFO, p_tbbutton[i].idCommand, (LPARAM)&tbbi);
      if (tbbi.fsStyle & TBSTYLE_DROPDOWN)
         tbbi.cx = 48;
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
      m_vstmclipboard.ElementAt(i)->Release();
   m_vstmclipboard.RemoveAllElements();

   if (pvstm)
      for (int i=0;i<pvstm->Size();i++)
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

void VPinball::SetPosCur(float x, float y)
{
   char szT[256];
   sprintf_s(szT, "%.4f, %.4f", x, y);
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

void VPinball::SetPropSel(Vector<ISelect> *pvsel)
{
   m_sb.CreateFromDispatch(m_hwnd, pvsel);
}

HMENU VPinball::GetMainMenu(int id)
{
    HMENU hmenu = GetMenu(m_hwnd);
    const int count = GetMenuItemCount(hmenu);
    return GetSubMenu(hmenu, id + ((count > NUM_MENUS) ? 1 : 0)); // MDI has added its stuff (table icon for first menu item)
}

void VPinball::ParseCommand(size_t code, HWND hwnd, size_t notify)
{
   CComObject<PinTable> *ptCur;

   // check if it's an Editable tool
   ItemTypeEnum type = (code == ID_INSERT_TARGET ? eItemSurface : EditableRegistry::TypeFromToolID(code));
   if (type != eItemInvalid)
   {
       SendMessage(m_hwndToolbarMain, TB_CHECKBUTTON, m_ToolCur, MAKELONG(FALSE,0));
       SendMessage(m_hwndToolbarPalette, TB_CHECKBUTTON, code, MAKELONG(TRUE,0));

       m_ToolCur = code;

       if (notify == 1) // accelerator - mouse can be over table already
       {
           POINT pt;
           GetCursorPos(&pt);
           SetCursorPos(pt.x, pt.y);
       }
       return;
   }

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
      }
      break;

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
      }
      break;

   case ID_TABLE_PLAY:
      DoPlay();
      break;

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
      }
      break;

   case ID_EDIT_PROPERTIES:
      {
         BOOL fShow = fFalse;

         if(!g_pplayer) fShow = m_sb.GetVisible(); // Get the current display state 

         switch(notify)
         {
         case 0: fShow = !fShow;  //!!?
            break;
         case 1: fShow = fTrue;   //set
            break;
         case 2:				  //re-display 
            break;			
         default: fShow = !fShow; //toggle
            break;
         }

         SetRegValue("Editor", "PropertiesVisible", REG_DWORD, &fShow, 4);

         if(!g_pplayer)
         {
            // Set toolbar button to the correct state
            TBBUTTONINFO tbinfo;
            ZeroMemory(&tbinfo,sizeof(TBBUTTONINFO));
            tbinfo.cbSize = sizeof(TBBUTTONINFO);
            tbinfo.dwMask = TBIF_STATE;
            SendMessage(m_hwndToolbarMain,TB_GETBUTTONINFO,ID_EDIT_PROPERTIES,(size_t)&tbinfo);

            if(notify == 2) fShow = (tbinfo.fsState & TBSTATE_CHECKED) != 0;

            if (fShow ^ ((tbinfo.fsState & TBSTATE_CHECKED) != 0))
            {
               tbinfo.fsState ^= TBSTATE_CHECKED;
            }

            SendMessage(m_hwndToolbarMain,TB_SETBUTTONINFO,ID_EDIT_PROPERTIES,(size_t)&tbinfo);
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
      }
      break;

   case ID_EDIT_BACKGLASSVIEW:
      {
         const BOOL fShow = !m_fBackglassView;

         SendMessage(m_hwndToolbarMain, TB_CHECKBUTTON, ID_EDIT_BACKGLASSVIEW, MAKELONG(fShow,0));

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
               // Set selection to something in the new view (unless hiding table elements)
               ptCur->AddMultiSel((ISelect *)ptCur, false);
         }

         SetEnableToolbar();
      }
      break;
   case ID_EDIT_SEARCH:
      {
         DialogBoxParam(g_hinst, MAKEINTRESOURCE(IDD_SEARCH_SELECT_ELEMENT), m_hwnd, SearchSelectProc, 0);
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
      ptCur = GetActiveTable();
      if (ptCur)
      {
          ptCur->m_renderSolid = (code == ID_VIEW_SOLID);
          ptCur->SetDirtyDraw();
          SetRegValueBool("Editor", "RenderSolid", ptCur->m_renderSolid);
      }
      break;
   case ID_VIEW_GRID:
      ptCur = GetActiveTable();
      if (ptCur)
          ptCur->put_DisplayGrid( !ptCur->m_fGrid );
      break;
   case ID_VIEW_BACKDROP:
      ptCur = GetActiveTable();
      if (ptCur)
          ptCur->put_DisplayBackdrop( !ptCur->m_fBackdrop );
      break;
   case IDC_SELECT:
   case ID_TABLE_MAGNIFY:
      {
            SendMessage(m_hwndToolbarPalette,TB_CHECKBUTTON,m_ToolCur,MAKELONG(FALSE,0));
            SendMessage(m_hwndToolbarMain,TB_CHECKBUTTON,code,MAKELONG(TRUE,0));

          m_ToolCur = code;

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
               ShowPermissionError();
            else
            {
               HRESULT hr = ptCur->TableSave();
               if (hr == S_OK)
                  UpdateRecentFileList(ptCur->m_szFileName);
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
               ShowPermissionError();
            else
            {
               HRESULT hr = ptCur->SaveAs();
               if (hr == S_OK)
                  UpdateRecentFileList(ptCur->m_szFileName);
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
               ShowPermissionError();
            else
            {
               size_t foo = DialogBoxParam(g_hinst, MAKEINTRESOURCE(IDD_PROTECT_DIALOG),
                  m_hwnd, ProtectTableProc, 0);
               // if the dialog returned ok then perform a normal save as
               if (foo)
               {
                  HRESULT foo2 = ptCur->SaveAs();
                  if (foo2 == S_OK)
                  {
                     // if the save was succesfull then the permissions take effect immediatly
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
      }
      break;

   case IDM_UNLOCKPROTECTED:
      {
         ptCur = GetActiveTable();
         if (ptCur)
         {
            size_t foo = DialogBoxParam(g_hinst, MAKEINTRESOURCE(IDD_UNLOCK_DIALOG),
               m_hwnd, UnlockTableProc, 0);
            // if the dialog returned ok then table is unlocked
            if (foo)
            {
               // re-enable any disabled menu items
               SetEnableToolbar();			// disable any tool bars
               ptCur->SetDirtyDraw();		// redraw the screen (incase hiding elements)
            }
         }
      }
      break;

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
         const size_t Index = code - RECENT_FIRST_MENU_IDM;
         // copy it into a temporary string so it can be correctly processed
         memcpy(szFileName, m_szRecentTableList[Index], sizeof(szFileName));
         LoadFileName(szFileName);
      }
      break;

   case IDM_OPEN:
      LoadFile();
      break;

   case IDM_CLOSE:
      {
         ptCur = GetActiveTable();
         if (ptCur)
            CloseTable(ptCur);
      }
      break;

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
      }
      break;

   case IDC_PASTE:
      {
         ptCur = GetActiveTable();
         if (ptCur)
            ptCur->Paste(fFalse, 0, 0);
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
            ptCur->Undo();
      }
      break;

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
      }
      break;

   case ID_FILE_EXIT:
      PostMessage(m_hwnd, WM_CLOSE, 0, 0);
      break;

   case ID_EDIT_AUDIOOPTIONS:
      {
         /*const DWORD foo =*/ DialogBoxParam(g_hinst, MAKEINTRESOURCE(IDD_AUDIO_OPTIONS),
            m_hwnd, AudioOptionsProc, 0);
      }
      break;

   case ID_EDIT_PHYSICSOPTIONS:
      {
         /*const DWORD foo =*/ DialogBoxParam(g_hinst, MAKEINTRESOURCE(IDD_PHYSICS_OPTIONS),
            m_hwnd, PhysicsOptionsProc, 0);
      }
      break;

   case ID_EDIT_EDITOROPTIONS:
      {
         /*const DWORD foo =*/ DialogBoxParam(g_hinst, MAKEINTRESOURCE(IDD_EDITOR_OPTIONS),
            m_hwnd, EditorOptionsProc, 0);
         // refresh editor options from the registry
         InitRegValues();
         // force a screen refresh (it an active table is loaded)
         ptCur = GetActiveTable();
         if (ptCur)
            ptCur->SetDirtyDraw();
      }
      break;

   case ID_EDIT_VIDEOOPTIONS:
      {
         /*const DWORD foo =*/ DialogBoxParam(g_hinst, MAKEINTRESOURCE(IDD_VIDEO_OPTIONS),
            m_hwnd, VideoOptionsProc, 0);
      }
      break;

   case ID_PREFERENCES_SECURITYOPTIONS:
      {
         /*const DWORD foo =*/ DialogBoxParam(g_hinst, MAKEINTRESOURCE(IDD_SECURITY_OPTIONS),
            m_hwnd, SecurityOptionsProc, 0);

         // refresh editor options from the registry
         InitRegValues();
      }
      break;

   case ID_EDIT_KEYS:
      {
         /*const DWORD foo =*/ DialogBoxParam(g_hinst, MAKEINTRESOURCE(IDD_KEYS),m_hwnd, KeysProc, 0);
      }
      break;

   case ID_TABLE_TABLEINFO:
      {
         ptCur = GetActiveTable();
         if (ptCur)
         {
            /*const DWORD foo =*/ DialogBoxParam(g_hinst, MAKEINTRESOURCE(IDD_TABLEINFO),
               m_hwnd, TableInfoProc, (size_t)ptCur);
         }
      }
      break;
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
               /*const DWORD foo =*/ DialogBoxParam(g_hinst, MAKEINTRESOURCE(IDD_SOUNDDIALOG),m_hwnd, SoundManagerProc, (size_t)ptCur);
            }
         }
      }
      break;

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
               /*const DWORD foo =*/ DialogBoxParam(g_hinst, MAKEINTRESOURCE(IDD_IMAGEDIALOG), m_hwnd, ImageManagerProc, (size_t)ptCur);
               m_sb.PopulateDropdowns(); // May need to update list of images
               m_sb.RefreshProperties();
            }
         }
      }
      break;

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
                   /*const DWORD foo =*/ DialogBoxParam(g_hinst, MAKEINTRESOURCE(IDD_MATERIALDIALOG), m_hwnd, MaterialManagerProc, (size_t)ptCur);
                   m_sb.PopulateDropdowns(); // May need to update list of images
                   m_sb.RefreshProperties();
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
               ShowPermissionError();
            else
            {
               /*const DWORD foo =*/ DialogBoxParam(g_hinst, MAKEINTRESOURCE(IDD_FONTDIALOG),
                  m_hwnd, FontManagerProc, (size_t)ptCur);
            }
         }
      }
      break;

   case ID_TABLE_DIMENSIONMANAGER:
      {
         ptCur = GetActiveTable();
         if (ptCur)
         {
            if (ptCur->CheckPermissions(DISABLE_OPEN_MANAGERS))
               ShowPermissionError();
            else
            {
               DialogBoxParam(g_hinst, MAKEINTRESOURCE(IDD_DIMENSION_CALCULATOR), m_hwnd, DimensionProc, (size_t)ptCur);
            }
         }
      }
      break;

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
               /*const DWORD foo =*/ DialogBoxParam(g_hinst, MAKEINTRESOURCE(IDD_COLLECTDIALOG),
                  m_hwnd, CollectManagerProc, (size_t)ptCur);

               m_sb.PopulateDropdowns(); // May need to update list of collections
               m_sb.RefreshProperties();
            }
         }
      }
      break;
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
         if( !ptCur ) break;
         HMENU hmenu = GetMenu(m_hwnd);
         ptCur->MergeAllLayers();
         for( int i=0;i<8;i++ ) ptCur->m_activeLayers[i]=false;
         for( int i=0;i<8;i++ ) setLayerStatus(i);
         break;
      }
   case ID_LAYER_TOGGLEALL:
      {
         ptCur = GetActiveTable();
         if( !ptCur ) break;
         HMENU hmenu = GetMenu(m_hwnd);
         for( int i=0;i<8;i++ ) ptCur->m_activeLayers[i] = !ptCur->m_toggleAllLayers;
         for( int i=0;i<8;i++ ) setLayerStatus(i);
         ptCur->m_toggleAllLayers ^= true;   
         break;
      }
   case ID_HELP_ABOUT:
      {
         /*const DWORD foo =*/ DialogBoxParam(g_hinst, MAKEINTRESOURCE(IDD_ABOUT),
            m_hwnd, AboutProc, 0);
      }
      break;
   }
}

void VPinball::setLayerStatus( int layerNumber )
{
   CComObject<PinTable> *ptCur;

   ptCur = GetActiveTable();
   if( !ptCur || layerNumber>7 ) return;

   SendMessage(m_hwndToolbarLayers, TB_CHECKBUTTON, allLayers[layerNumber], MAKELONG(( !ptCur->m_activeLayers[layerNumber] ), 0));

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

   for (unsigned int i=0; i<TBCOUNTPALETTE; ++i)
   {
      const int id = g_tbbuttonPalette[i].idCommand;

      // Targets don't have their own Editable type, they're just surfaces
      ItemTypeEnum type = (id == ID_INSERT_TARGET ? eItemSurface : EditableRegistry::TypeFromToolID(id));
      const unsigned int enablecode = EditableRegistry::GetAllowedViews(type);

      const bool fEnable = fTableActive && ((enablecode & state) != 0);

      // Set toolbar state
      SendMessage(m_hwndToolbarPalette, TB_ENABLEBUTTON, id, MAKELONG(fEnable,0));

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
      SendMessage(m_hwndToolbarMain, TB_ENABLEBUTTON, id, MAKELONG(fEnable,0));
   }

   // set layer button states
   if (ptCur)
      {
       for (int i=0; i<8; ++i)
       {
           SendMessage(m_hwndToolbarLayers, TB_CHECKBUTTON, allLayers[i], MAKELONG(ptCur->m_activeLayers[i], 0));
      }
   }

   SetEnablePalette();
   ParseCommand(ID_EDIT_PROPERTIES, m_hwnd, 2);//redisplay 
}

void VPinball::DoPlay()
{
   NumVideoBytes = 0;
   CComObject<PinTable> * const ptCur = GetActiveTable();
   if (ptCur)
      ptCur->Play();
}

void VPinball::LoadFile()
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

   const HRESULT hr = GetRegString("RecentDir","LoadDir", szInitialDir, 1024);
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

   if(ret == 0)
      return;

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
      SetRegValue("RecentDir","LoadDir", REG_SZ, szLoadDir, lstrlen(szLoadDir));

      // make sure the load directory is the active directory
      DWORD err = SetCurrentDirectory(szLoadDir);
      if (err == 0)
         err = GetLastError();

      UpdateRecentFileList(szFileName);
   }

   SetEnableToolbar();
}

CComObject<PinTable> *VPinball::GetActiveTable()
{
   HWND hwndT = (HWND)SendMessage(m_hwndWork, WM_MDIGETACTIVE, 0, 0);

   if (hwndT)
   {
      CComObject<PinTable> *pt = (CComObject<PinTable> *)GetWindowLongPtr(hwndT, GWLP_USERDATA);
      return pt;
   }
   else
      return NULL;
}

BOOL VPinball::FCanClose()
{
   while(m_vtable.Size())
   {
      const BOOL fCanClose = CloseTable(m_vtable.ElementAt(0));

      if (!fCanClose)
         return fFalse;
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
      delete [] szText;
      if (result == IDCANCEL)
         return fFalse;

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
   MessageBox(m_hwnd, ls.m_szbuffer, "Visual Pinball", MB_ICONWARNING);
}

void VPinball::SetEnableMenuItems()
{
   CComObject<PinTable> * const ptCur = GetActiveTable();

   // Set menu item to the correct state
   HMENU hmenu = GetMenu(m_hwnd);

   CheckMenuItem(hmenu, ID_EDIT_PROPERTIES, MF_BYCOMMAND | (m_sb.GetVisible() ? MF_CHECKED : MF_UNCHECKED));
   CheckMenuItem(hmenu, ID_EDIT_BACKGLASSVIEW, MF_BYCOMMAND | (m_fBackglassView ? MF_CHECKED : MF_UNCHECKED));

   // is there a valid table??
   if (ptCur)
   {
      EnableMenuItem(hmenu, IDM_CLOSE, MF_BYCOMMAND | MF_ENABLED);
      EnableMenuItem(hmenu, ID_EDIT_UNDO, MF_BYCOMMAND | MF_ENABLED);
      EnableMenuItem(hmenu, ID_EDIT_BACKGLASSVIEW, MF_BYCOMMAND | MF_ENABLED);
      EnableMenuItem(hmenu, ID_TABLE_PLAY, MF_BYCOMMAND | MF_ENABLED);
      EnableMenuItem(hmenu, ID_TABLE_MAGNIFY, MF_BYCOMMAND | MF_ENABLED);
      EnableMenuItem(hmenu, ID_TABLE_TABLEINFO, MF_BYCOMMAND | MF_ENABLED);
      EnableMenuItem(hmenu, ID_EDIT_SEARCH, MF_BYCOMMAND | MF_ENABLED );
      EnableMenuItem(hmenu, ID_EDIT_DRAWINGORDER_HIT, MF_BYCOMMAND | MF_ENABLED );
      EnableMenuItem(hmenu, ID_EDIT_DRAWINGORDER_SELECT, MF_BYCOMMAND | MF_ENABLED );
      // enable/disable save options
      UINT flags = MF_BYCOMMAND | (ptCur->CheckPermissions(DISABLE_TABLE_SAVE) ? MF_GRAYED : MF_ENABLED);
      EnableMenuItem(hmenu, IDM_SAVE, flags);
      EnableMenuItem(hmenu, IDM_SAVEAS, flags);
      EnableMenuItem(hmenu, IDM_SAVEASPROTECTED, flags);
      EnableMenuItem(hmenu, ID_FILE_EXPORT_BLUEPRINT, flags);

      // if we can do a normal save but not a protected save then disable 'save as protected'
      // (if we cant do any saves it is already disabled)
      if ( (!ptCur->CheckPermissions(DISABLE_TABLE_SAVE))	&&
         (ptCur->CheckPermissions(DISABLE_TABLE_SAVEPROT)) )
         EnableMenuItem(hmenu, IDM_SAVEASPROTECTED, MF_BYCOMMAND | MF_GRAYED);

      // enable/disable script option
      flags = MF_BYCOMMAND | (ptCur->CheckPermissions(DISABLE_SCRIPT_EDITING) ? MF_GRAYED : MF_ENABLED);
      EnableMenuItem(hmenu, ID_EDIT_SCRIPT, flags);

      // enable/disable managers options
      flags = MF_BYCOMMAND | (ptCur->CheckPermissions(DISABLE_OPEN_MANAGERS) ? MF_GRAYED : MF_ENABLED);
      EnableMenuItem(hmenu, ID_TABLE_SOUNDMANAGER, flags);
      EnableMenuItem(hmenu, ID_TABLE_IMAGEMANAGER, flags);
      //EnableMenuItem(hmenu, ID_TABLE_FONTMANAGER, flags);
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

      CheckMenuItem(hmenu, ID_VIEW_SOLID,   MF_BYCOMMAND | (ptCur->RenderSolid() ? MF_CHECKED : MF_UNCHECKED));
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
      EnableMenuItem(hmenu, ID_TABLE_MAGNIFY, MF_BYCOMMAND | MF_GRAYED);
      EnableMenuItem(hmenu, ID_EDIT_SEARCH, MF_BYCOMMAND | MF_GRAYED );
      EnableMenuItem(hmenu, ID_EDIT_DRAWINGORDER_HIT, MF_BYCOMMAND | MF_GRAYED );
      EnableMenuItem(hmenu, ID_EDIT_DRAWINGORDER_SELECT, MF_BYCOMMAND | MF_GRAYED );
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
         SetRegValue("RecentDir", szRegName, REG_SZ, m_szRecentTableList[i], lstrlen(m_szRecentTableList[i])+1);
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
      for (int i=RECENT_FIRST_MENU_IDM; i<=RECENT_LAST_MENU_IDM; i++)
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
      for (int i=0; i<LAST_OPENED_TABLE_COUNT; i++)
      {
         char number[8];
         char menuname[MAX_PATH];

         // if this entry is empty then all the rest are empty
         if (m_szRecentTableList[i][0] == 0x00) break;
         _itoa_s(i+1,number,10);
         strcpy_s(menuname,"&");
         strcat_s(menuname,number);
         strcat_s(menuname," ");
         strcat_s(menuname,m_szRecentTableList[i]);
         // set the IDM of this menu item
         menuInfo.wID = RECENT_FIRST_MENU_IDM + i;
         menuInfo.dwTypeData = menuname;
         menuInfo.cch = lstrlen(menuname);

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

HRESULT VPinball::ApcHost_OnTranslateMessage(MSG* pmsg, BOOL* pfConsumed)
{
   *pfConsumed = FALSE;

   if (!g_pplayer)
   {
      for (int i=0;i<m_sb.m_vhwndDialog.Size();i++)
      {
         if (IsDialogMessage(m_sb.m_vhwndDialog.ElementAt(i), pmsg))
            *pfConsumed = TRUE;
      }
      if (m_pcv && m_pcv->m_hwndMain)
      {
         //if (pmsg->hwnd == m_pcv->m_hwndMain)
         {
            int fTranslated = fFalse;

            if ((pmsg->hwnd == m_pcv->m_hwndMain) || IsChild(m_pcv->m_hwndMain, pmsg->hwnd))
               fTranslated = TranslateAccelerator(m_pcv->m_hwndMain, m_pcv->m_haccel, pmsg);

            if (fTranslated)
               *pfConsumed = TRUE;
            else
            {
               if (IsDialogMessage(m_pcv->m_hwndMain, pmsg))
                  *pfConsumed = TRUE;
            }
         }
      }

      if (m_pcv && m_pcv->m_hwndFind)
      {
         if (IsDialogMessage(m_pcv->m_hwndFind, pmsg))
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
         if (IsDialogMessage(g_pplayer->m_hwndDebugger, pmsg))
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
         PinTable *ptable = g_pvp->GetActiveTable();
         if ( ptable )
         {
            while( ptable->m_savingActive )
               Sleep(1000);
         }
         if (g_pplayer)
            SendMessage(g_pplayer->m_hwnd, WM_CLOSE, 0, 0);

         BOOL fCanClose = g_pvp->FCanClose();
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

         //const int scrollwindowtop = 48*(TBCOUNTMAIN/2);
         const int scrollwindowtop = 48*(TBCOUNTMAIN/2)+28*(TBCOUNTLAYERS/2);
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
		 const size_t padding = SendMessage(g_pvp->m_hwndToolbarPalette, TB_GETPADDING, 0, 0);
		 const size_t buttonsize = SendMessage(g_pvp->m_hwndToolbarPalette, TB_GETBUTTONSIZE, 0, 0);
         const int vertpadding = HIWORD(padding);
         const int vertbutsize = HIWORD(buttonsize);
         ZeroMemory(&si,sizeof(SCROLLINFO));
         si.cbSize = sizeof(si);
         si.fMask = SIF_ALL;
         si.nMin = 0;
         si.nMax = ((vertbutsize+vertpadding) * (TBCOUNTPALETTE/2)) + 4; // Add 4 padding
         si.nPage = scrollwindowheight;
         si.nPos = g_pvp->m_palettescroll;

         SetScrollInfo(g_pvp->m_hwndSideBarScroll, SB_VERT, &si, TRUE);

         // check if we have any blank space at the bottom and fill it in by moving the scrollbar up
         if ((int)(si.nPos + si.nPage) > si.nMax)
         {
            g_pvp->m_palettescroll = si.nMax - si.nPage;
            if (g_pvp->m_palettescroll < 0)
               g_pvp->m_palettescroll = 0;

            SetScrollPos(hwnd, SB_VERT, g_pvp->m_palettescroll, TRUE);

            SetWindowPos(g_pvp->m_hwndToolbarPalette,NULL,
               0, -g_pvp->m_palettescroll, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER);
         }

         int sidebarwidth = TOOLBAR_WIDTH;
         if (scrollwindowheight < si.nMax)
            sidebarwidth += SCROLL_WIDTH;

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

                  int menucount=0;
                  for (int i=0;i<pt->m_vedit.Size();i++)
                  {
                     IEditable * const piedit = pt->m_vedit.ElementAt(i);
                     // check scriptable - decals don't have scripts and therefore don't have names
                     if (piedit->GetScriptable() && piedit->m_fBackglass == g_pvp->m_fBackglassView)
                     {
                        char szT[64]; // Names can only be 32 characters (plus terminator)
                        WideCharToMultiByte(CP_ACP, 0, pt->m_vedit.ElementAt(i)->GetScriptable()->m_wzName, -1, szT, 64, NULL, NULL);

						const size_t index = SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)szT);
                        SendMessage(hwndList, LB_SETITEMDATA, index, i+1);// menu can't have an item with id 0, so bump everything up one
                     }
                  }

                  for (int i=0;i<pt->m_vcollection.Size();i++)
                  {
                     char szT[64]; // Names can only be 32 characters (plus terminator)
                     WideCharToMultiByte(CP_ACP, 0, pt->m_vcollection.ElementAt(i)->m_wzName, -1, szT, 64, NULL, NULL);

					 const size_t index = SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)szT);
                     SendMessage(hwndList, LB_SETITEMDATA, index, i | 0x80000000);
                  }

				  const size_t listcount = SendMessage(hwndList, LB_GETCOUNT, 0, 0);

                  // Take the items from our sorted list and put them into the menu
				  for (size_t i = 0; i<listcount; i++)
                  {
                     char szT[64];
                     int flags = MF_STRING;

                     if ((menucount%30 == 0) && (menucount != 0))
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
                        for (int i=0;i<pcol->m_visel.Size();i++)
                           pt->AddMultiSel(pcol->m_visel.ElementAt(i), i == 0 ? fAdd : true);
                     }
                     else
                        pt->AddMultiSel(pt->m_vedit.ElementAt(icmd-1)->GetISelect(), fAdd);
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
         ZeroMemory(&si,sizeof(SCROLLINFO));
         si.cbSize = sizeof(SCROLLINFO);
         si.fMask = SIF_ALL;
         GetScrollInfo(hwnd, SB_VERT, &si);
         switch (LOWORD(wParam))
         {
         case SB_LINEUP:
            g_pvp->m_palettescroll -= si.nPage/10;
            break;
         case SB_LINEDOWN:
            g_pvp->m_palettescroll += si.nPage/10;
            break;
         case SB_PAGEUP:
            g_pvp->m_palettescroll -= si.nPage/2;
            break;
         case SB_PAGEDOWN:
            g_pvp->m_palettescroll += si.nPage/2;
            break;
         case SB_THUMBTRACK:
            {
               const int delta = (int)(g_pvp->m_palettescroll - si.nPos);
               g_pvp->m_palettescroll = ((short)HIWORD(wParam) + delta);
            }
            break;
         }

         SetScrollPos(hwnd, SB_VERT, g_pvp->m_palettescroll, TRUE);

         SetWindowPos(g_pvp->m_hwndToolbarPalette,NULL,
            0, -g_pvp->m_palettescroll, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER);
      }
      break;
   }

   return DefWindowProc(hwnd, uMsg, wParam, lParam);
}


STDMETHODIMP VPinball::PlaySound(BSTR bstr)
{
   if (g_pplayer) g_pplayer->m_ptable->PlaySound(bstr, 0, 1.f, 0.f, 0.f, 0, false, true);

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
   if( g_pplayer ) {
	   g_pplayer->m_fCloseDown = true;
	   g_pplayer->m_fCloseType = 1;
   }
   else
      PostMessage(m_hwnd, WM_CLOSE, 0, 0);
}


INT_PTR CALLBACK SoundManagerProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   CCO(PinTable) *pt = (CCO(PinTable) *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

   switch (uMsg)
   {
   case WM_INITDIALOG:
      {
         SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
         LVCOLUMN lvcol;

         ListView_SetExtendedListViewStyle(GetDlgItem(hwndDlg, IDC_SOUNDLIST), LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
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
                  return FALSE;
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
               char szT[10240];

               szFileName[0] = '\0';

               OPENFILENAME ofn;
               ZeroMemory(&ofn, sizeof(OPENFILENAME));
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
                  hr = SetRegValue("RecentDir","SoundDir", REG_SZ, szInitialDir, lstrlen(szInitialDir));
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
                     ZeroMemory(&ofn, sizeof(OPENFILENAME));
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
                     ofn.lpstrInitialDir = (hr == S_OK) ? szInitialDir : NULL;

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
                        ListView_SetItemText(GetDlgItem(hwndDlg, IDC_SOUNDLIST), sel, 1, ofn.lpstrFile);
                        pt->SetNonUndoableDirty(eSaveDirty);
                     }
                  }
               }
            }
            break;

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

                     ZeroMemory(&ofn, sizeof(OPENFILENAME));
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
                  /*const HRESULT hr =*/ SetRegValue("RecentDir","SoundDir", REG_SZ, szInitialDir, lstrlen(szInitialDir));
                  EndDialog(hwndDlg, TRUE);
               }
            }
            break;
		case IDC_SNDTOBG:
            {								
               if(ListView_GetSelectedCount(GetDlgItem(hwndDlg, IDC_SOUNDLIST)))
               {	     
                  LVITEM lvitem;
                  int sel = ListView_GetNextItem(GetDlgItem(hwndDlg, IDC_SOUNDLIST), -1, LVNI_SELECTED); //next selected item 	
                  while (sel != -1)
                  {									
                     lvitem.mask = LVIF_PARAM;
                     lvitem.iItem = sel;
                     lvitem.iSubItem = 0;
                     ListView_GetItem(GetDlgItem(hwndDlg, IDC_SOUNDLIST), &lvitem);
                     PinSound *pps = (PinSound *)lvitem.lParam;								

					 strcpy_s(pps->m_szPath, "* Backglass Output *");
                     sel = ListView_GetNextItem(GetDlgItem(hwndDlg, IDC_SOUNDLIST), sel, LVNI_SELECTED); //next selected item

                  }
                  EndDialog(hwndDlg, TRUE);
               }
            }
            break;

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

INT_PTR CALLBACK ImageManagerProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   CCO(PinTable) *pt = (CCO(PinTable) *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

   switch (uMsg)
   {
   case WM_INITDIALOG:
      {
         SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
         LVCOLUMN lvcol;
         ListView_SetExtendedListViewStyle(GetDlgItem(hwndDlg, IDC_SOUNDLIST), LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

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

         pt->ListImages(GetDlgItem(hwndDlg, IDC_SOUNDLIST));

         return TRUE;
      }
      break;

   case WM_CLOSE:
      EndDialog(hwndDlg, FALSE);
      break;

   case GET_COLOR_TABLE:
      {
         *((unsigned long **)lParam) = pt->m_rgcolorcustom;
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
               Texture * const ppi = (Texture *)lvitem.lParam;
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
                     Texture * const ppi = (Texture *)lvitem.lParam;
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
            Texture * const ppi = (Texture *)lvitem.lParam;

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

            HDC hdcDD;
            ppi->GetTextureDC(&hdcDD);
            StretchBlt(pdis->hDC, x, y, width, height, hdcDD, 0, 0, ppi->m_width, ppi->m_height, SRCCOPY);
            ppi->ReleaseTextureDC(hdcDD);
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
               const size_t color = GetWindowLongPtr((HWND)lParam, GWLP_USERDATA);
               int sel = ListView_GetNextItem(GetDlgItem(hwndDlg, IDC_SOUNDLIST), -1, LVNI_SELECTED);
               while (sel != -1)
               {							
                  LVITEM lvitem;
                  lvitem.mask = LVIF_PARAM;
                  lvitem.iItem = sel;
                  lvitem.iSubItem = 0;
                  ListView_GetItem(GetDlgItem(hwndDlg, IDC_SOUNDLIST), &lvitem);
                  Texture * const ppi = (Texture *)lvitem.lParam;
                  ppi->SetTransparentColor(color);

                  SetRegValue("Editor", "TransparentColorKey", REG_DWORD, &color, 4);

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
               char szT[10240];
               szFileName[0] = '\0';

               OPENFILENAME ofn;
               ZeroMemory(&ofn, sizeof(OPENFILENAME));
               ofn.lStructSize = sizeof(OPENFILENAME);
               ofn.hInstance = g_hinst;
               ofn.hwndOwner = g_pvp->m_hwnd;

               ofn.lpstrFilter = "Bitmap, JPEG and PNG Files (.bmp/.jpg/.png)\0*.bmp;*.jpg;*.jpeg;*.png\0";
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
                  hr = SetRegValue("RecentDir","ImageDir", REG_SZ, szInitialDir, lstrlen(szInitialDir));
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
                     Texture * const ppi = (Texture*)lvitem.lParam;									

                     OPENFILENAME ofn;
                     ZeroMemory(&ofn, sizeof(OPENFILENAME));
                     ofn.lStructSize = sizeof(OPENFILENAME);
                     ofn.hInstance = g_hinst;
                     ofn.hwndOwner = g_pvp->m_hwnd;
                     //TEXT
                     ofn.lpstrFilter = "*.bmp\0*.bmp\0*.jpg\0*.jpeg;*.png;*.gif;*.ico;*.IFF;*.PCX;*.PICT;*.psd;*.tga;*.tiff;*.tif\0";

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
                  SetRegValue("RecentDir","ImageDir", REG_SZ, szInitialDir, lstrlen(szInitialDir));
               }							
            }	
            break;

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
                        Texture * const ppi = (Texture*)lvitem.lParam;
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
                        Texture * const ppi = (Texture*)lvitem.lParam;
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
                     ZeroMemory(&ofn, sizeof(OPENFILENAME));
                     ofn.lStructSize = sizeof(OPENFILENAME);
                     ofn.hInstance = g_hinst;
                     ofn.hwndOwner = g_pvp->m_hwnd;
                     // TEXT
                     ofn.lpstrFilter = "Bitmap, JPEG and PNG Files (.bmp/.jpg/.png)\0*.bmp;*.jpg;*.jpeg;*.png\0";
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
                        Texture * const ppi = (Texture*)lvitem.lParam;

                        strcpy_s(szInitialDir, sizeof(szInitialDir), szFileName);
                        szInitialDir[ofn.nFileOffset] = 0;
                        hr = SetRegValue("RecentDir","ImageDir", REG_SZ, szInitialDir, lstrlen(szInitialDir));

                        pt->ReImportImage(GetDlgItem(hwndDlg, IDC_SOUNDLIST), ppi, ofn.lpstrFile);
                        ListView_SetItemText(GetDlgItem(hwndDlg, IDC_SOUNDLIST), sel, 1, ppi->m_szPath);
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


INT_PTR CALLBACK MaterialManagerProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CCO(PinTable) *pt = (CCO(PinTable) *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

    switch (uMsg)
    {
        case WM_INITDIALOG:
        {
            SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
            LVCOLUMN lvcol;

            ListView_SetExtendedListViewStyle(GetDlgItem(hwndDlg, IDC_MATERIAL_LIST), LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
            lvcol.mask = LVCF_TEXT | LVCF_WIDTH;
            LocalString ls(IDS_NAME);
            lvcol.pszText = ls.m_szbuffer;// = "Name";
            lvcol.cx = 280;
            ListView_InsertColumn(GetDlgItem(hwndDlg, IDC_MATERIAL_LIST), 0, &lvcol);

            pt = (CCO(PinTable) *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
            pt->ListMaterials(GetDlgItem(hwndDlg, IDC_MATERIAL_LIST));
            ListView_SetItemState(GetDlgItem(hwndDlg, IDC_MATERIAL_LIST), 0, LVIS_SELECTED, LVIS_SELECTED)
            return TRUE;
        }
        case WM_CLOSE:
        {
            EndDialog(hwndDlg, FALSE);
            break;
        }

        case GET_COLOR_TABLE:
        {
            *((unsigned long **)lParam) = pt->m_rgcolorcustom;
            return TRUE;
        }

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
                    ListView_SetItemText(GetDlgItem(hwndDlg, IDC_MATERIAL_LIST), pinfo->item.iItem, 0, pinfo->item.pszText);
                    LVITEM lvitem;
                    lvitem.mask = LVIF_PARAM;
                    lvitem.iItem = pinfo->item.iItem;
                    lvitem.iSubItem = 0;
                    ListView_GetItem(GetDlgItem(hwndDlg, IDC_MATERIAL_LIST), &lvitem);
                    Material * const pmat = (Material*)lvitem.lParam;
                    lstrcpy(pmat->m_szName, pinfo->item.pszText);
                    pt->SetNonUndoableDirty(eSaveDirty);
                    return TRUE;
                }
                case LVN_ITEMCHANGING:
                {
                    NMLISTVIEW * const plistview = (LPNMLISTVIEW)lParam;                    
                    if ((plistview->uNewState & LVIS_SELECTED) != (plistview->uOldState & LVIS_SELECTED))
                    {
                        if (plistview->uNewState & LVIS_SELECTED)
                        {
                            const int sel = plistview->iItem;
                            LVITEM lvitem;
                            lvitem.mask = LVIF_PARAM;
                            lvitem.iItem = sel;
                            lvitem.iSubItem = 0;
                            ListView_GetItem(GetDlgItem(hwndDlg, IDC_MATERIAL_LIST), &lvitem);
                            Material * const pmat = (Material*)lvitem.lParam;
                            HWND hwndColor = GetDlgItem(hwndDlg, IDC_COLOR);
                            SendMessage(hwndColor, CHANGE_COLOR, 0, pmat->m_cBase);
                            hwndColor = GetDlgItem(hwndDlg, IDC_COLOR2);
                            SendMessage(hwndColor, CHANGE_COLOR, 0, pmat->m_cGlossy);
                            hwndColor = GetDlgItem(hwndDlg, IDC_COLOR3);
                            SendMessage(hwndColor, CHANGE_COLOR, 0, pmat->m_cClearcoat);
                            char textBuf[256];
                            f2sz(pmat->m_fWrapLighting,textBuf);
                            SetDlgItemText(hwndDlg, IDC_DIFFUSE_EDIT, textBuf);
                            f2sz(pmat->m_fRoughness,textBuf);
                            SetDlgItemText(hwndDlg, IDC_GLOSSY_EDIT, textBuf);
                            f2sz(pmat->m_fEdge,textBuf);
                            SetDlgItemText(hwndDlg, IDC_SPECULAR_EDIT, textBuf);
                            int op = (int)(pmat->m_fOpacity*100.0f+0.5f);
                            _itoa_s( op, textBuf, 10 );
                            SetDlgItemText(hwndDlg, IDC_OPACITY_EDIT, textBuf);

                            HWND checkboxHwnd = GetDlgItem(hwndDlg, IDC_DIFFUSE_CHECK);
                            SendMessage(checkboxHwnd, BM_SETCHECK, pmat->m_bIsMetal ? BST_CHECKED : BST_UNCHECKED, 0);
                            checkboxHwnd = GetDlgItem(hwndDlg, IDC_OPACITY_CHECK);
                            SendMessage(checkboxHwnd, BM_SETCHECK, pmat->m_bOpacityActive ? BST_CHECKED : BST_UNCHECKED, 0);

                            InvalidateRect(hwndColor, NULL, FALSE);
                        }
                    }
                }
                break;
                case LVN_ITEMCHANGED:
                {
                    NMLISTVIEW * const plistview = (LPNMLISTVIEW)lParam;                    
                    const int sel = plistview->iItem;
                    LVITEM lvitem;
                    lvitem.mask = LVIF_PARAM;
                    lvitem.iItem = sel;
                    lvitem.iSubItem = 0;
                    ListView_GetItem(GetDlgItem(hwndDlg, IDC_MATERIAL_LIST), &lvitem);
                    Material * const pmat = (Material*)lvitem.lParam;
                    if( (plistview->uNewState & LVIS_SELECTED)==0 )
                    {
                        char textBuf[256];
                        GetDlgItemText(hwndDlg, IDC_DIFFUSE_EDIT, textBuf, 31);
                        pmat->m_fWrapLighting = sz2f(textBuf);
                        GetDlgItemText(hwndDlg, IDC_GLOSSY_EDIT, textBuf, 31);
                        pmat->m_fRoughness = sz2f(textBuf);
                        GetDlgItemText(hwndDlg, IDC_SPECULAR_EDIT, textBuf, 31);
                        pmat->m_fEdge = sz2f(textBuf);
                        GetDlgItemText(hwndDlg, IDC_OPACITY_EDIT, textBuf, 31);
                        int op = atoi(textBuf);
                        if( op>100 ) op=100;
                        if( op<0 ) op=0;
                        pmat->m_fOpacity = (float)op/100.0f;
                        size_t checked = SendDlgItemMessage(hwndDlg, IDC_DIFFUSE_CHECK, BM_GETCHECK, 0, 0);
                        pmat->m_bIsMetal = checked==1;
                        checked = SendDlgItemMessage(hwndDlg, IDC_OPACITY_CHECK, BM_GETCHECK, 0, 0);
                        pmat->m_bOpacityActive = checked==1;
                    }
                    else
                    {
                        HWND hwndColor = GetDlgItem(hwndDlg, IDC_COLOR);
                        SendMessage(hwndColor, CHANGE_COLOR, 0, pmat->m_cBase);
                        hwndColor = GetDlgItem(hwndDlg, IDC_COLOR2);
                        SendMessage(hwndColor, CHANGE_COLOR, 0, pmat->m_cGlossy);
                        hwndColor = GetDlgItem(hwndDlg, IDC_COLOR3);
                        SendMessage(hwndColor, CHANGE_COLOR, 0, pmat->m_cClearcoat);
                        char textBuf[256];
                        f2sz(pmat->m_fWrapLighting,textBuf);
                        SetDlgItemText(hwndDlg, IDC_DIFFUSE_EDIT, textBuf);
                        f2sz(pmat->m_fRoughness,textBuf);
                        SetDlgItemText(hwndDlg, IDC_GLOSSY_EDIT, textBuf);
                        f2sz(pmat->m_fEdge,textBuf);
                        SetDlgItemText(hwndDlg, IDC_SPECULAR_EDIT, textBuf);
                        int op = (int)(pmat->m_fOpacity*100.0f+0.5f);
                        _itoa_s( op, textBuf, 10 );
                        SetDlgItemText(hwndDlg, IDC_OPACITY_EDIT, textBuf);
                        HWND checkboxHwnd = GetDlgItem(hwndDlg, IDC_DIFFUSE_CHECK);
                        SendMessage(checkboxHwnd, BM_SETCHECK, pmat->m_bIsMetal ? BST_CHECKED : BST_UNCHECKED, 0);
                        checkboxHwnd = GetDlgItem(hwndDlg, IDC_OPACITY_CHECK);
                        SendMessage(checkboxHwnd, BM_SETCHECK, pmat->m_bOpacityActive ? BST_CHECKED : BST_UNCHECKED, 0);
                    }
                    const int count = ListView_GetSelectedCount(GetDlgItem(hwndDlg, IDC_MATERIAL_LIST));
                    const int fEnable = !(count > 1);
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
            const int sel = ListView_GetNextItem(GetDlgItem(hwndDlg, IDC_MATERIAL_LIST), -1, LVNI_SELECTED);
            if (sel != -1)
            {
                LVITEM lvitem;
                lvitem.mask = LVIF_PARAM;
                lvitem.iItem = sel;
                lvitem.iSubItem = 0;
                ListView_GetItem(GetDlgItem(hwndDlg, IDC_MATERIAL_LIST), &lvitem);
            }
            else
            {
                // Nothing currently selected
            }
            return TRUE;
        }
        break;

        case WM_COMMAND:
        {
            switch (HIWORD(wParam))
            {
                case COLOR_CHANGED:
                {
                    const int count = ListView_GetSelectedCount(GetDlgItem(hwndDlg, IDC_MATERIAL_LIST));
                    if (count > 0)
                    {
                       const size_t color = GetWindowLongPtr((HWND)lParam, GWLP_USERDATA);
                        HWND hwndcolor1 = GetDlgItem(hwndDlg, IDC_COLOR);
                        HWND hwndcolor2 = GetDlgItem(hwndDlg, IDC_COLOR2);
                        HWND hwndcolor3 = GetDlgItem(hwndDlg, IDC_COLOR3);
                        int sel = ListView_GetNextItem(GetDlgItem(hwndDlg, IDC_MATERIAL_LIST), -1, LVNI_SELECTED);
                        while (sel != -1)
                        {							
                            LVITEM lvitem;
                            lvitem.mask = LVIF_PARAM;
                            lvitem.iItem = sel;
                            lvitem.iSubItem = 0;
                            ListView_GetItem(GetDlgItem(hwndDlg, IDC_MATERIAL_LIST), &lvitem);
                            Material * const pmat = (Material*)lvitem.lParam;
                            if( hwndcolor1 == (HWND)lParam )
                                pmat->m_cBase = color;
                            else if( hwndcolor2 == (HWND)lParam )
                                pmat->m_cGlossy = color;
                            else if( hwndcolor3 == (HWND)lParam )
                                pmat->m_cClearcoat = color;

                            // The previous selection is now deleted, so look again from the top of the list
                            sel = ListView_GetNextItem(GetDlgItem(hwndDlg, IDC_MATERIAL_LIST), sel, LVNI_SELECTED);
                        }

                        pt->SetNonUndoableDirty(eSaveDirty);
                    }
                }
                break;

                case BN_CLICKED:
                {
                    switch (LOWORD(wParam))
                    {
                        case IDOK:
                        {
                            const int count = ListView_GetSelectedCount(GetDlgItem(hwndDlg, IDC_MATERIAL_LIST));
                            if (count > 0)
                            {
                                int sel = ListView_GetNextItem(GetDlgItem(hwndDlg, IDC_MATERIAL_LIST), -1, LVNI_SELECTED);
                                while (sel != -1)
                                {							
                                    LVITEM lvitem;
                                    lvitem.mask = LVIF_PARAM;
                                    lvitem.iItem = sel;
                                    lvitem.iSubItem = 0;
                                    ListView_GetItem(GetDlgItem(hwndDlg, IDC_MATERIAL_LIST), &lvitem);
                                    Material * const pmat = (Material*)lvitem.lParam;
                                    char textBuf[256];
                                    GetDlgItemText(hwndDlg, IDC_DIFFUSE_EDIT, textBuf, 31);
                                    pmat->m_fWrapLighting = sz2f(textBuf);
                                    GetDlgItemText(hwndDlg, IDC_GLOSSY_EDIT, textBuf, 31);
                                    pmat->m_fRoughness = sz2f(textBuf);
                                    GetDlgItemText(hwndDlg, IDC_SPECULAR_EDIT, textBuf, 31);
                                    pmat->m_fEdge = sz2f(textBuf);
                                    GetDlgItemText(hwndDlg, IDC_OPACITY_EDIT, textBuf, 31);
                                    int op = atoi(textBuf);
                                    if( op>100 ) op=100;
                                    if( op<0 ) op=0;
                                    pmat->m_fOpacity = (float)op/100.0f;
                                    size_t checked = SendDlgItemMessage(hwndDlg, IDC_DIFFUSE_CHECK, BM_GETCHECK, 0, 0);
                                    pmat->m_bIsMetal = checked==1;
                                    checked = SendDlgItemMessage(hwndDlg, IDC_OPACITY_CHECK, BM_GETCHECK, 0, 0);
                                    pmat->m_bOpacityActive = checked==1;
                                    // The previous selection is now deleted, so look again from the top of the list
                                    sel = ListView_GetNextItem(GetDlgItem(hwndDlg, IDC_MATERIAL_LIST), sel, LVNI_SELECTED);
                                }
                            }
                            EndDialog(hwndDlg, TRUE);
                            break;
                        }
                        case IDCANCEL:
                        {
                            EndDialog(hwndDlg, FALSE);
                            break;
                        }
                        case IDC_ADD_BUTTON:
                        {
                            Material *pmat = new Material();
                            pt = (CCO(PinTable) *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
                
                            pt->AddMaterial( pmat );
                            pt->AddListMaterial(GetDlgItem(hwndDlg, IDC_MATERIAL_LIST), pmat);
                        
                            break;
                        }
                        case IDC_IMPORT:
                        {
                            char szFileName[10240];
                            char szInitialDir[10240];
                            szFileName[0] = '\0';

                            OPENFILENAME ofn;
                            ZeroMemory(&ofn, sizeof(OPENFILENAME));
                            ofn.lStructSize = sizeof(OPENFILENAME);
                            ofn.hInstance = g_hinst;
                            ofn.hwndOwner = g_pvp->m_hwnd;

                            ofn.lpstrFilter = "Material Files (.mat)\0*.mat\0";
                            ofn.lpstrFile = szFileName;
                            ofn.nMaxFile = 10240;
                            ofn.lpstrDefExt = "mat";
                            ofn.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY | OFN_EXPLORER ;

                            HRESULT hr = GetRegString("RecentDir","MaterialDir", szInitialDir, 1024);
                            ofn.lpstrInitialDir = (hr == S_OK) ? szInitialDir : NULL;

                            const int ret = GetOpenFileName(&ofn);

                            if(ret)
                            {
                                int materialCount=0;
                                int versionNumber=0;
                                FILE *f;
                                fopen_s(&f,ofn.lpstrFile,"rb");

                                fread(&versionNumber,1,4,f);
                                if( versionNumber!=1 )
                                {
                                    ShowError("Materials are not compatible with this version!");
                                    fclose(f);
                                    break;
                                }
                                fread(&materialCount,1 ,4,f );
                                for( int i=0;i<materialCount;i++ )
                                {
                                    Material *pmat = new Material();
                                    SaveMaterial mat;

                                    fread(&mat, 1, sizeof(SaveMaterial), f);
                                    pmat->m_cBase = mat.cBase;
                                    pmat->m_cGlossy = mat.cGlossy;
                                    pmat->m_cClearcoat = mat.cClearcoat;
									pmat->m_fWrapLighting = mat.fWrapLighting;
                                    pmat->m_fRoughness = mat.fRoughness;
                                    pmat->m_fEdge = mat.fEdge;
                                    pmat->m_bIsMetal = mat.bIsMetal;
                                    pmat->m_fOpacity = mat.fOpacity;
                                    pmat->m_bOpacityActive = mat.bOpacityActive;
                                    memcpy(pmat->m_szName, mat.szName,32);
                                    pt = (CCO(PinTable) *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

                                    pt->AddMaterial( pmat );
                                    pt->AddListMaterial(GetDlgItem(hwndDlg, IDC_MATERIAL_LIST), pmat);
                                }

                                hr = SetRegValue("RecentDir","MaterialDir", REG_SZ, szInitialDir, lstrlen(szInitialDir));
                                pt->SetNonUndoableDirty(eSaveDirty);
                            }
                            break;
                        }

                        case IDC_RENAME:
                        {
                            const int sel = ListView_GetNextItem(GetDlgItem(hwndDlg, IDC_MATERIAL_LIST), -1, LVNI_SELECTED);
                            if (sel != -1)
                            {
                                SetFocus(GetDlgItem(hwndDlg, IDC_MATERIAL_LIST));
                                /*const HWND hwndFoo =*/ ListView_EditLabel(GetDlgItem(hwndDlg, IDC_MATERIAL_LIST), sel);
                            }
                        }
                        break;

                        case IDC_EXPORT:
                        {
                            if(ListView_GetSelectedCount(GetDlgItem(hwndDlg, IDC_MATERIAL_LIST)))	// if some items are selected???
                            {
                                char szFileName[10240];
                                char szInitialDir[2096];
                                int sel = ListView_GetNextItem(GetDlgItem(hwndDlg, IDC_MATERIAL_LIST), -1, LVNI_SELECTED);	
                                int selCount = ListView_GetSelectedCount(GetDlgItem(hwndDlg, IDC_MATERIAL_LIST));
                                if( sel==-1 )
                                    break;

                                strcpy_s( szFileName,"Materials.mat" );
                                OPENFILENAME ofn;
                                ZeroMemory(&ofn, sizeof(OPENFILENAME));
                                ofn.lStructSize = sizeof(OPENFILENAME);
                                ofn.hInstance = g_hinst;
                                ofn.hwndOwner = g_pvp->m_hwnd;
                                ofn.lpstrFile = szFileName;
                                //TEXT
                                ofn.lpstrFilter = "*.mat\0";
                                ofn.nMaxFile = 2096;
                                ofn.lpstrDefExt = "mat";

                                const HRESULT hr = GetRegString("RecentDir","MaterialDir", szInitialDir, 2096);

                                if (hr == S_OK)ofn.lpstrInitialDir = szInitialDir;
                                else ofn.lpstrInitialDir = NULL;	

                                ofn.lpstrTitle = "Export materials";
                                ofn.Flags = OFN_NOREADONLYRETURN | OFN_CREATEPROMPT | OFN_OVERWRITEPROMPT | OFN_EXPLORER;

                                szInitialDir[ofn.nFileOffset] = 0;

                                if (GetSaveFileName(&ofn))	//Get filename from user
                                {					
                                    FILE *f;
                                    fopen_s(&f,ofn.lpstrFile,"wb");
                                    const int MATERIAL_VERSION=1;
                                    fwrite(&MATERIAL_VERSION,1,4,f);
                                    fwrite(&selCount,1 ,4,f );
                                    while (sel != -1)
                                    {									
                                        LVITEM lvitem;
                                        lvitem.mask = LVIF_PARAM;
                                        lvitem.iItem = sel;
                                        lvitem.iSubItem = 0;
                                        ListView_GetItem(GetDlgItem(hwndDlg, IDC_MATERIAL_LIST), &lvitem);
                                        Material * const pmat = (Material*)lvitem.lParam;									
                                        SaveMaterial mat;
                                        mat.cBase = pmat->m_cBase;
                                        mat.cGlossy = pmat->m_cGlossy;
                                        mat.cClearcoat = pmat->m_cClearcoat;
                                        mat.fRoughness = pmat->m_fRoughness;
                                        mat.fEdge = pmat->m_fEdge;
                                        mat.fWrapLighting = pmat->m_fWrapLighting;
                                        mat.bIsMetal = pmat->m_bIsMetal;
                                        mat.fOpacity = pmat->m_fOpacity;
                                        mat.bOpacityActive = pmat->m_bOpacityActive;
                                        memcpy(mat.szName, pmat->m_szName, 32 );
                                        fwrite(&mat, 1, sizeof(SaveMaterial), f );

                                        sel = ListView_GetNextItem(GetDlgItem(hwndDlg, IDC_MATERIAL_LIST), sel, LVNI_SELECTED);
                                    }
                                    fclose(f);
                                }
                                SetRegValue("RecentDir","MaterialDir", REG_SZ, szInitialDir, lstrlen(szInitialDir));
                            }							
                        }	
                        break;

                    case IDC_DELETE:
                        {
                            const int count = ListView_GetSelectedCount(GetDlgItem(hwndDlg, IDC_MATERIAL_LIST));
                            if (count > 0)
                            {
                                LocalString ls(IDS_REMOVEMATERIAL);
                                const int ans = MessageBox(hwndDlg, ls.m_szbuffer/*"Are you sure you want to remove this material?"*/, "Visual Pinball", MB_YESNO | MB_DEFBUTTON2);
                                if (ans == IDYES)
                                {
                                    pt = (CCO(PinTable) *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
                                    int sel = ListView_GetNextItem(GetDlgItem(hwndDlg, IDC_MATERIAL_LIST), -1, LVNI_SELECTED);
                                    while (sel != -1)
                                    {										
                                        LVITEM lvitem;
                                        lvitem.mask = LVIF_PARAM;
                                        lvitem.iItem = sel;
                                        lvitem.iSubItem = 0;
                                        ListView_GetItem(GetDlgItem(hwndDlg, IDC_MATERIAL_LIST), &lvitem);
                                        ListView_DeleteItem(GetDlgItem(hwndDlg, IDC_MATERIAL_LIST), sel);
                                        Material * const pmat = (Material*)lvitem.lParam;
                                        pt->RemoveMaterial( pmat );
                                        // The previous selection is now deleted, so look again from the top of the list
                                        sel = ListView_GetNextItem(GetDlgItem(hwndDlg, IDC_MATERIAL_LIST), -1, LVNI_SELECTED);
                                    }
                                }
                                pt->SetNonUndoableDirty(eSaveDirty);
                            }
                        }
                        break;
                    }
                    break;
                }
            }
            break;
        }
    }

    return FALSE;
}


INT_PTR CALLBACK AboutProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
                     hr = OpenURL("http://www.vpforums.org");
                  else
                  {
                     HWND hwndTransURL = GetDlgItem(hwndDlg, IDC_TRANSWEBSITE);
                     char szSite[MAX_PATH];
                     GetWindowText(hwndTransURL, szSite, MAX_PATH);
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


void FillVideoModesList(HWND hwnd, const std::vector<VideoMode>& modes, const VideoMode* curSelMode=0)
{
    SendMessage(hwnd, LB_RESETCONTENT, 0, 0);

    for (unsigned i = 0; i < modes.size(); ++i)
    {
        char szT[128];
        if (modes[i].depth)
            sprintf_s(szT, "%d x %d x %d", modes[i].width, modes[i].height, modes[i].depth);
        else
            sprintf_s(szT, "%d x %d", modes[i].width, modes[i].height);
        SendMessage(hwnd, LB_ADDSTRING, 0, (LPARAM)szT);

        if (curSelMode &&
              modes[i].width == curSelMode->width &&
              modes[i].height == curSelMode->height &&
              modes[i].depth == curSelMode->depth)
            SendMessage(hwnd, LB_SETCURSEL, i, 0);
    }
}


const int rgwindowsize[] = {640, 720, 800, 912, 1024, 1152, 1280, 1600};  // windowed resolutions for selection list

std::vector<VideoMode> allVideoModes;

INT_PTR CALLBACK VideoOptionsProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
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

         int maxTexDim;
         HRESULT hr = GetRegInt("Player", "MaxTexDimension", &maxTexDim);
         if (hr != S_OK)
            maxTexDim = 0; // default: Don't resize textures
         switch(maxTexDim)
         {
         case 512:	SendMessage(GetDlgItem(hwndDlg, IDC_Tex512),BM_SETCHECK, BST_CHECKED,0);
            break;
         case 1024:  SendMessage(GetDlgItem(hwndDlg, IDC_Tex1024),BM_SETCHECK, BST_CHECKED,0);
            break;
         case 2048:  SendMessage(GetDlgItem(hwndDlg, IDC_Tex2048),BM_SETCHECK, BST_CHECKED,0);
            break;
         default:	SendMessage(GetDlgItem(hwndDlg, IDC_TexUnlimited),BM_SETCHECK, BST_CHECKED,0);
         }

         HWND hwndCheck = GetDlgItem(hwndDlg, IDC_GLOBAL_REFLECTION_CHECK);
         int reflection;
         hr = GetRegInt("Player", "BallReflection", &reflection);
         if (hr != S_OK)
            reflection = fTrue;
         SendMessage(hwndCheck, BM_SETCHECK, reflection ? BST_CHECKED : BST_UNCHECKED, 0);

         hwndCheck = GetDlgItem(hwndDlg, IDC_GLOBAL_TRAIL_CHECK);
         int trail;
         hr = GetRegInt("Player", "BallTrail", &trail);
         if (hr != S_OK)
            trail = fTrue;
         SendMessage(hwndCheck, BM_SETCHECK, trail ? BST_CHECKED : BST_UNCHECKED, 0);
		 
		 int vsync;
         hr = GetRegInt("Player", "AdaptiveVSync", &vsync);
         if (hr != S_OK)
            vsync = 0;
		 SetDlgItemInt(hwndDlg, IDC_ADAPTIVE_VSYNC, vsync, FALSE);

         int maxPrerenderedFrames;
         hr = GetRegInt("Player", "MaxPrerenderedFrames", &maxPrerenderedFrames);
         if (hr != S_OK)
             maxPrerenderedFrames = 2;
         SetDlgItemInt(hwndDlg, IDC_MAX_PRE_FRAMES, maxPrerenderedFrames, FALSE);

         hwndCheck = GetDlgItem(hwndDlg, IDC_AA_ALL_TABLES);
         int m_useAA;
         hr = GetRegInt("Player", "USEAA", &m_useAA);
         if (hr != S_OK)
            m_useAA = 0;
         SendMessage(hwndCheck, BM_SETCHECK, (m_useAA != 0) ? BST_CHECKED : BST_UNCHECKED, 0);

         int fxaa;
         hr = GetRegInt("Player", "FXAA", &fxaa);
         if (hr != S_OK)
            fxaa = 0;
         hwndCheck = GetDlgItem(hwndDlg, IDC_FFXAA);
         SendMessage(hwndCheck, BM_SETCHECK, (fxaa == 1) ? BST_CHECKED : BST_UNCHECKED, 0);
         hwndCheck = GetDlgItem(hwndDlg, IDC_QFXAA);
         SendMessage(hwndCheck, BM_SETCHECK, (fxaa == 2) ? BST_CHECKED : BST_UNCHECKED, 0);

         hwndCheck = GetDlgItem(hwndDlg, IDC_3D_STEREO);
         int stereo3D;
         hr = GetRegInt("Player", "Stereo3D", &stereo3D);
         if (hr != S_OK)
            stereo3D = 0;
         SendMessage(hwndCheck, BM_SETCHECK, (stereo3D != 0) ? BST_CHECKED : BST_UNCHECKED, 0);

         hwndCheck = GetDlgItem(hwndDlg, IDC_3D_STEREO_AA);
         int stereo3DAA;
         hr = GetRegInt("Player", "Stereo3DAntialias", &stereo3DAA);
         if (hr != S_OK)
            stereo3DAA = fFalse;
         SendMessage(hwndCheck, BM_SETCHECK, stereo3DAA ? BST_CHECKED : BST_UNCHECKED, 0);

         hwndCheck = GetDlgItem(hwndDlg, IDC_3D_STEREO_Y);
         int stereo3DY;
         hr = GetRegInt("Player", "Stereo3DYAxis", &stereo3DY);
         if (hr != S_OK)
            stereo3DY = fFalse;
         SendMessage(hwndCheck, BM_SETCHECK, stereo3DY ? BST_CHECKED : BST_UNCHECKED, 0);

		 char tmp[256];
         float stereo3DMS;
         hr = GetRegStringAsFloat("Player", "Stereo3DMaxSeparation", &stereo3DMS);
         if (hr != S_OK)
            stereo3DMS = 0.03f;
		 sprintf_s(tmp,256,"%f",stereo3DMS);
 		 SetDlgItemTextA(hwndDlg, IDC_3D_STEREO_MS, tmp);

         float stereo3DZPD;
         hr = GetRegStringAsFloat("Player", "Stereo3DZPD", &stereo3DZPD);
         if (hr != S_OK)
            stereo3DZPD = 0.5f;
		 sprintf_s(tmp,256,"%f",stereo3DZPD);
 		 SetDlgItemTextA(hwndDlg, IDC_3D_STEREO_ZPD, tmp);

         const bool forceAniso = (GetRegIntWithDefault("Player", "ForceAnisotropicFiltering", 0) != 0);
         SendMessage(GetDlgItem(hwndDlg, IDC_FORCE_ANISO), BM_SETCHECK, forceAniso ? BST_CHECKED : BST_UNCHECKED, 0);

         const bool softwareVP = (GetRegIntWithDefault("Player", "SoftwareVertexProcessing", 0) != 0);
         SendMessage(GetDlgItem(hwndDlg, IDC_SOFTWARE_VP), BM_SETCHECK, softwareVP ? BST_CHECKED : BST_UNCHECKED, 0);

         int widthcur;
         hr = GetRegInt("Player", "Width", &widthcur);
         if (hr != S_OK)
            widthcur = DEFAULT_PLAYER_WIDTH;

         int heightcur;
         hr = GetRegInt("Player", "Height", &heightcur);
         if (hr != S_OK)
            heightcur = widthcur*3/4;

         int depthcur;
         hr = GetRegInt("Player", "ColorDepth", &depthcur);
         if (hr != S_OK)
            depthcur = 32;

         int fullscreen;
         hr = GetRegInt("Player", "FullScreen", &fullscreen);
         if (hr != S_OK)
            fullscreen = 0;

         if (fullscreen)
         {
            SendMessage(hwndDlg, GET_FULLSCREENMODES, widthcur, heightcur<<16 | depthcur);
            HWND hwndRadio = GetDlgItem(hwndDlg, IDC_FULLSCREEN);
            SendMessage(hwndRadio, BM_SETCHECK, BST_CHECKED, 0);
         }
         else
         {
            SendMessage(hwndDlg, GET_WINDOW_MODES, widthcur, heightcur);
            HWND hwndRadio = GetDlgItem(hwndDlg, IDC_WINDOW);
            SendMessage(hwndRadio, BM_SETCHECK, BST_CHECKED, 0);
         }

         int alphaRampsAccuracy;
         hr = GetRegInt("Player", "AlphaRampAccuracy", &alphaRampsAccuracy);
         if (hr != S_OK)
            alphaRampsAccuracy = 10;
         HWND hwndARASlider = GetDlgItem(hwndDlg, IDC_ARASlider);
         SendMessage(hwndARASlider, TBM_SETRANGE, fTrue, MAKELONG(0, 10));
         SendMessage(hwndARASlider, TBM_SETTICFREQ, 1, 0);
         SendMessage(hwndARASlider, TBM_SETLINESIZE, 0, 1);
         SendMessage(hwndARASlider, TBM_SETPAGESIZE, 0, 1);
         SendMessage(hwndARASlider, TBM_SETTHUMBLENGTH, 5, 0);
         SendMessage(hwndARASlider, TBM_SETPOS, TRUE, alphaRampsAccuracy);

         int ballStretchMode;
         hr = GetRegInt("Player", "BallStretchMode", &ballStretchMode);
         if (hr != S_OK)
            ballStretchMode = 0;
         switch(ballStretchMode)
         {
         case 0:  SendMessage(GetDlgItem(hwndDlg, IDC_StretchNo),BM_SETCHECK, BST_CHECKED,0);
            break;
         case 1:  SendMessage(GetDlgItem(hwndDlg, IDC_StretchYes),BM_SETCHECK, BST_CHECKED,0);
            break;
         case 2:  SendMessage(GetDlgItem(hwndDlg, IDC_StretchMonitor),BM_SETCHECK, BST_CHECKED,0);
            break;
         default: SendMessage(GetDlgItem(hwndDlg, IDC_StretchNo),BM_SETCHECK, BST_CHECKED,0);
         }

         // set selected Monitors
         // Monitors: 4:3, 16:9, 16:10, 21:10
         int selected;
         hr = GetRegInt("Player", "BallStretchMonitor", &selected);
         if (hr != S_OK)
            selected = 1; // assume 16:9 as standard
         SendMessage(GetDlgItem(hwndDlg, IDC_MonitorCombo), CB_ADDSTRING, 0, (LPARAM)"4:3");
         SendMessage(GetDlgItem(hwndDlg, IDC_MonitorCombo), CB_ADDSTRING, 0, (LPARAM)"16:9");
         SendMessage(GetDlgItem(hwndDlg, IDC_MonitorCombo), CB_ADDSTRING, 0, (LPARAM)"16:10");
         SendMessage(GetDlgItem(hwndDlg, IDC_MonitorCombo), CB_ADDSTRING, 0, (LPARAM)"21:10"); 
         SendMessage(GetDlgItem(hwndDlg, IDC_MonitorCombo), CB_ADDSTRING, 0, (LPARAM)"3:4 (R)");
         SendMessage(GetDlgItem(hwndDlg, IDC_MonitorCombo), CB_ADDSTRING, 0, (LPARAM)"9:16 (R)");
         SendMessage(GetDlgItem(hwndDlg, IDC_MonitorCombo), CB_ADDSTRING, 0, (LPARAM)"10:16 (R)");
         SendMessage(GetDlgItem(hwndDlg, IDC_MonitorCombo), CB_ADDSTRING, 0, (LPARAM)"10:21 (R)"); 

         SendMessage(GetDlgItem(hwndDlg, IDC_MonitorCombo), CB_SETCURSEL, selected, 0);
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
				  size_t fullscreen = SendMessage(hwndFullScreen, BM_GETCHECK, 0, 0);
                  SetRegValue("Player", "FullScreen", REG_DWORD, &fullscreen, 4);

                  HWND hwndList = GetDlgItem(hwndDlg, IDC_SIZELIST);
				  size_t index = SendMessage(hwndList, LB_GETCURSEL, 0, 0);
                  VideoMode* pvm = &allVideoModes[index];
                  SetRegValue("Player", "Width", REG_DWORD, &pvm->width, 4);
                  SetRegValue("Player", "Height", REG_DWORD, &pvm->height, 4);
                  if (fullscreen)
                  {
                     SetRegValue("Player", "ColorDepth", REG_DWORD, &pvm->depth, 4);
                     SetRegValue("Player", "RefreshRate", REG_DWORD, &pvm->refreshrate, 4);
                  }

                  HWND maxTexDim512 = GetDlgItem(hwndDlg, IDC_Tex512);
                  HWND maxTexDim1024 = GetDlgItem(hwndDlg, IDC_Tex1024);
                  HWND maxTexDim2048 = GetDlgItem(hwndDlg, IDC_Tex2048);
                  //HWND maxTexDimUnlimited = GetDlgItem(hwndDlg, IDC_TexUnlimited);
                  int maxTexDim = 0;
                  if (SendMessage(maxTexDim512, BM_GETCHECK, 0, 0) == BST_CHECKED)
                     maxTexDim = 512;
                  if (SendMessage(maxTexDim1024, BM_GETCHECK, 0, 0) == BST_CHECKED)
                     maxTexDim = 1024;
                  if (SendMessage(maxTexDim2048, BM_GETCHECK, 0, 0) == BST_CHECKED)
                     maxTexDim = 2048;
                  SetRegValue("Player", "MaxTexDimension", REG_DWORD, &maxTexDim,4);

                  HWND hwndReflect = GetDlgItem(hwndDlg, IDC_GLOBAL_REFLECTION_CHECK);
				  size_t reflection = SendMessage(hwndReflect, BM_GETCHECK, 0, 0);
                  SetRegValue("Player", "BallReflection", REG_DWORD, &reflection, 4);

                  HWND hwndTrail = GetDlgItem(hwndDlg, IDC_GLOBAL_TRAIL_CHECK);
				  size_t trail = SendMessage(hwndTrail, BM_GETCHECK, 0, 0);
                  SetRegValue("Player", "BallTrail", REG_DWORD, &trail, 4);

                  int vsync = GetDlgItemInt(hwndDlg, IDC_ADAPTIVE_VSYNC, NULL, TRUE);
                  SetRegValue("Player", "AdaptiveVSync", REG_DWORD, &vsync, 4);

                  int maxPrerenderedFrames = GetDlgItemInt(hwndDlg, IDC_MAX_PRE_FRAMES, NULL, TRUE);
                  SetRegValue("Player","MaxPrerenderedFrames", REG_DWORD, &maxPrerenderedFrames, 4);

                  HWND hwndFXAA = GetDlgItem(hwndDlg, IDC_FFXAA);
				  size_t ffxaa = SendMessage(hwndFXAA, BM_GETCHECK, 0, 0);
				  hwndFXAA = GetDlgItem(hwndDlg, IDC_QFXAA);
				  size_t qfxaa = SendMessage(hwndFXAA, BM_GETCHECK, 0, 0) * 2;
				  if(qfxaa)
					SetRegValue("Player", "FXAA", REG_DWORD, &qfxaa, 4);
				  else
					SetRegValue("Player", "FXAA", REG_DWORD, &ffxaa, 4);

                  HWND hwndUseAA = GetDlgItem(hwndDlg, IDC_AA_ALL_TABLES);
				  size_t m_useAA = SendMessage(hwndUseAA, BM_GETCHECK, 0, 0);
                  SetRegValue("Player", "USEAA", REG_DWORD, &m_useAA, 4);

                  HWND hwndStereo3D = GetDlgItem(hwndDlg, IDC_3D_STEREO);
				  size_t stereo3D = SendMessage(hwndStereo3D, BM_GETCHECK, 0, 0);
                  SetRegValue("Player", "Stereo3D", REG_DWORD, &stereo3D, 4);
                  SetRegValue("Player", "Stereo3DEnabled", REG_DWORD, &stereo3D, 4);

                  HWND hwndStereo3DAA = GetDlgItem(hwndDlg, IDC_3D_STEREO_AA);
				  size_t stereo3DAA = SendMessage(hwndStereo3DAA, BM_GETCHECK, 0, 0);
                  SetRegValue("Player", "Stereo3DAntialias", REG_DWORD, &stereo3DAA, 4);

                  HWND hwndStereo3DY = GetDlgItem(hwndDlg, IDC_3D_STEREO_Y);
				  size_t stereo3DY = SendMessage(hwndStereo3DY, BM_GETCHECK, 0, 0);
                  SetRegValue("Player", "Stereo3DYAxis", REG_DWORD, &stereo3DY, 4);

                  HWND hwndForceAniso = GetDlgItem(hwndDlg, IDC_FORCE_ANISO);
				  size_t forceAniso = SendMessage(hwndForceAniso, BM_GETCHECK, 0, 0);
                  SetRegValue("Player", "ForceAnisotropicFiltering", REG_DWORD, &forceAniso, 4);

                  HWND hwndSoftwareVP = GetDlgItem(hwndDlg, IDC_SOFTWARE_VP);
				  size_t softwareVP = SendMessage(hwndSoftwareVP, BM_GETCHECK, 0, 0);
                  SetRegValueBool("Player", "SoftwareVertexProcessing", softwareVP != 0);

                  HWND hwndAraSlider = GetDlgItem(hwndDlg, IDC_ARASlider);
				  size_t alphaRampsAccuracy = SendMessage(hwndAraSlider, TBM_GETPOS, 0, 0);
                  SetRegValue("Player", "AlphaRampAccuracy", REG_DWORD, &alphaRampsAccuracy, 4);

				  char strTmp[256];
				  GetDlgItemTextA(hwndDlg, IDC_3D_STEREO_MS, strTmp, 256);
				  SetRegValue("Player", "Stereo3DMaxSeparation", REG_SZ, &strTmp,lstrlen(strTmp));

				  GetDlgItemTextA(hwndDlg, IDC_3D_STEREO_ZPD, strTmp, 256);
				  SetRegValue("Player", "Stereo3DZPD", REG_SZ, &strTmp,lstrlen(strTmp));

				  //HWND hwndBallStretchNo = GetDlgItem(hwndDlg, IDC_StretchNo);
                  HWND hwndBallStretchYes = GetDlgItem(hwndDlg, IDC_StretchYes);
                  HWND hwndBallStretchMonitor = GetDlgItem(hwndDlg, IDC_StretchMonitor);
                  int ballStretchMode = 0;
                  if (SendMessage(hwndBallStretchYes, BM_GETCHECK, 0, 0) == BST_CHECKED)
                     ballStretchMode = 1;
                  if (SendMessage(hwndBallStretchMonitor, BM_GETCHECK, 0, 0) == BST_CHECKED)
                     ballStretchMode = 2;
                  SetRegValue("Player", "BallStretchMode", REG_DWORD, &ballStretchMode,4);

                  // get selected Monitors
                  // Monitors: 4:3, 16:9, 16:10, 21:10
                  HWND hwndBallStretchCombo = GetDlgItem(hwndDlg, IDC_MonitorCombo);
				  size_t selected = SendMessage(hwndBallStretchCombo, CB_GETCURSEL, 0, 0);
                  if (selected == LB_ERR)
                     selected = 1; // assume a 16:9 Monitor as standard
                  SetRegValue("Player", "BallStretchMonitor", REG_DWORD, &selected,4);

                  EndDialog(hwndDlg, TRUE);
               }
               break;

            case IDC_WINDOW:
               SendMessage(hwndDlg, GET_WINDOW_MODES, 0, 0);
               break;

            case IDC_FULLSCREEN:
               SendMessage(hwndDlg, GET_FULLSCREENMODES, 0, 0);
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
         size_t indexcur = -1;
         int widthcur = (int)wParam, heightcur = (int)lParam;

         SendMessage(hwndDlg, RESET_SIZELIST_CONTENT, 0, 0);
         HWND hwndList = GetDlgItem(hwndDlg, IDC_SIZELIST);

         const size_t csize = sizeof(rgwindowsize)/sizeof(int);
         const int screenwidth = GetSystemMetrics(SM_CXSCREEN);

         allVideoModes.clear();

         for (size_t i=0; i<csize; ++i)
         {
            const int xsize = rgwindowsize[i];
            if (xsize <= screenwidth)
            {
               if (xsize == widthcur)
                  indexcur = i;

               VideoMode mode;
               mode.width = xsize;
               mode.height = xsize*3/4;
               mode.depth = 0;
               mode.refreshrate = 0;

               allVideoModes.push_back(mode);
            }
         }

         FillVideoModesList(hwndList, allVideoModes);

         // set up windowed fullscreen mode
         VideoMode mode;
         // TODO: use multi-monitor functions
         mode.width = GetSystemMetrics( SM_CXSCREEN );
         mode.height = GetSystemMetrics( SM_CYSCREEN );
         mode.depth = 0;
         mode.refreshrate = 0;
         allVideoModes.push_back(mode);

         char szT[128];
         sprintf_s(szT, "%d x %d (windowed fullscreen)", mode.width, mode.height);
         SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)szT);
         if (mode.width == widthcur && mode.height == heightcur)
             indexcur = SendMessage(hwndList, LB_GETCOUNT, 0, 0) - 1;

         SendMessage(hwndList, LB_SETCURSEL, (indexcur != -1) ? indexcur : 0, 0);
      }
      break;

   case GET_FULLSCREENMODES:
      {
         HWND hwndList = GetDlgItem(hwndDlg, IDC_SIZELIST);
         EnumerateDisplayModes(0, allVideoModes);

         VideoMode curSelMode;
         curSelMode.width = wParam;
         curSelMode.height = lParam>>16;
         curSelMode.depth = lParam & 0xffff;

         FillVideoModesList(hwndList, allVideoModes, &curSelMode);

         if (SendMessage(hwndList, LB_GETCURSEL, 0, 0) == -1)
            SendMessage(hwndList, LB_SETCURSEL, 0, 0);
      }
      break;

   case RESET_SIZELIST_CONTENT:
      {
         HWND hwndList = GetDlgItem(hwndDlg, IDC_SIZELIST);
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

const int rgDlgIDFromSecurityLevel [] = {IDC_ACTIVEX0, IDC_ACTIVEX1, IDC_ACTIVEX2, IDC_ACTIVEX3, IDC_ACTIVEX4};

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
            (rcMain.right + rcMain.left)/2 - (rcDlg.right - rcDlg.left)/2,
            (rcMain.bottom + rcMain.top)/2 - (rcDlg.bottom - rcDlg.top)/2,
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
                  for (int i=0;i<5;i++)
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

               HRESULT hr = GetRegString("RecentDir","FontDir", szInitialDir, 1024);
               ofn.lpstrInitialDir = (hr == S_OK) ? szInitialDir : NULL;

               const int ret = GetOpenFileName(&ofn);
               if(ret)
               {
                  strcpy_s(szInitialDir, sizeof(szInitialDir), szFileName);
                  szInitialDir[ofn.nFileOffset] = 0;
                  hr = SetRegValue("RecentDir","FontDir", REG_SZ, szInitialDir, lstrlen(szInitialDir));
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

class CollectionDialogStruct
{
public:
   Collection *pcol;
   PinTable *ppt;
};

INT_PTR CALLBACK CollectManagerProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
         lvcol.cx = 200;
         ListView_InsertColumn(GetDlgItem(hwndDlg, IDC_SOUNDLIST), 0, &lvcol);

         pt = (CCO(PinTable) *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

         pt->ListCollections(GetDlgItem(hwndDlg, IDC_SOUNDLIST));

         return TRUE;
      }
      break;

   case WM_NOTIFY:
      {
         LPNMHDR pnmhdr = (LPNMHDR)lParam;
         if(pnmhdr->code == LVN_ENDLABELEDIT)
		 {
            NMLVDISPINFO *pinfo = (NMLVDISPINFO *)lParam;
            if (pinfo->item.pszText == NULL || pinfo->item.pszText[0] == '\0')
               return FALSE;
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

				  const size_t ret = DialogBoxParam(g_hinst, MAKEINTRESOURCE(IDD_COLLECTION),
                     hwndDlg, CollectionProc, (size_t)&cds/*pcol*/);

                  if (ret)
                     pt->SetNonUndoableDirty(eSaveDirty);

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
                  ListView_EditLabel(GetDlgItem(hwndDlg, IDC_SOUNDLIST), sel);
               }
            }
            break;
         case IDC_COL_UP_BUTTON:
             {
                 HWND listHwnd = GetDlgItem(hwndDlg, IDC_SOUNDLIST);
                 const int idx = ListView_GetNextItem(listHwnd, -1, LVNI_SELECTED);
                 if (idx != -1 && idx>0)
                 {
                     SetFocus(listHwnd);
                     LVITEM lvitem1;
                     lvitem1.mask = LVCF_TEXT | LVIF_PARAM;
                     lvitem1.iItem = idx;
                     lvitem1.iSubItem = 0;
                     ListView_GetItem( listHwnd, &lvitem1 );
                     CComObject<Collection> * const pcol = (CComObject<Collection> *)lvitem1.lParam;
                     pt->MoveCollectionUp(pcol);
                     ListView_DeleteItem( listHwnd, idx );
                     lvitem1.mask = LVIF_PARAM;
                     lvitem1.iItem = idx-1;
                     ListView_InsertItem( listHwnd, &lvitem1 );
                     char szT[MAX_PATH];
                     WideCharToMultiByte(CP_ACP, 0, pcol->m_wzName, -1, szT, MAX_PATH, NULL, NULL);
                     ListView_SetItemText( listHwnd, idx-1, 0, szT );
                     ListView_SetItemState( listHwnd, -1, 0, LVIS_SELECTED);
                     ListView_SetItemState( listHwnd, idx-1, LVIS_SELECTED, LVIS_SELECTED);
                     ListView_SetItemState( listHwnd, idx-1, LVIS_FOCUSED, LVIS_FOCUSED);
                 }
             }
             break;
         case IDC_COL_DOWN_BUTTON:
             {
                 HWND listHwnd = GetDlgItem(hwndDlg, IDC_SOUNDLIST);
                 const int idx = ListView_GetNextItem(listHwnd, -1, LVNI_SELECTED);
                 if (idx != -1 && (idx<pt->m_vcollection.Size()-1) )
                 {
                     SetFocus(listHwnd);
                     LVITEM lvitem1;
                     lvitem1.mask = LVCF_TEXT | LVIF_PARAM;
                     lvitem1.iItem = idx;
                     lvitem1.iSubItem = 0;
                     ListView_GetItem( listHwnd, &lvitem1 );
                     CComObject<Collection> * const pcol = (CComObject<Collection> *)lvitem1.lParam;
                     pt->MoveCollectionDown(pcol);
                     ListView_DeleteItem( listHwnd, idx );
                     lvitem1.mask = LVIF_PARAM;
                     lvitem1.iItem = idx+1;
                     ListView_InsertItem( listHwnd, &lvitem1 );
                     char szT[MAX_PATH];
                     WideCharToMultiByte(CP_ACP, 0, pcol->m_wzName, -1, szT, MAX_PATH, NULL, NULL);
                     ListView_SetItemText( listHwnd, idx+1, 0, szT );
                     ListView_SetItemState( listHwnd, -1, 0, LVIS_SELECTED);
                     ListView_SetItemState( listHwnd, idx+1, LVIS_SELECTED, LVIS_SELECTED);
                     ListView_SetItemState( listHwnd, idx+1, LVIS_FOCUSED, LVIS_FOCUSED);
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

INT_PTR CALLBACK CollectionProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   //CCO(PinTable) *pt;
   //pt = (CCO(PinTable) *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

   switch (uMsg)
   {
   case WM_INITDIALOG:
      {
         CollectionDialogStruct * const pcds = (CollectionDialogStruct *)lParam;
         SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);

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
			   const size_t index = SendMessage(hwndIn, LB_ADDSTRING, 0, (size_t)szT);
               SendMessage(hwndIn, LB_SETITEMDATA, index, (size_t)piscript);
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
			   const size_t index = SendMessage(hwndOut, LB_ADDSTRING, 0, (size_t)szT);
               SendMessage(hwndOut, LB_SETITEMDATA, index, (size_t)piscript);
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
			   const size_t listsize = SendMessage(hwndList, LB_GETCOUNT, 0, 0);
			   const size_t count = SendMessage(hwndList, LB_GETSELCOUNT, 0, 0);
               int * const rgsel = new int[count]; //!! size_t?
               SendMessage(hwndList, LB_GETSELITEMS, count, (LPARAM)rgsel);

               for (size_t loop=0;loop<count;loop++)
                  //for (i=count-1;i>=0;i--)
               {
                  const size_t i = (LOWORD(wParam) == IDC_UP) ? loop : (count - loop - 1);

				  const size_t len = SendMessage(hwndList, LB_GETTEXTLEN, rgsel[i], 0);
                  char * const szT = new char[len+1]; // include null terminator
                  SendMessage(hwndList, LB_GETTEXT, rgsel[i], (LPARAM)szT);
				  const size_t data = SendMessage(hwndList, LB_GETITEMDATA, rgsel[i], 0);

                  const int newindex = (LOWORD(wParam) == IDC_UP) ? max(rgsel[i]-1, (int)i) : min(rgsel[i]+2, (int)(listsize - (count - 1) + i)); //!! see above
                  int oldindex = rgsel[i];

                  if (oldindex > newindex)
                     oldindex++; // old item will be one lower when we try to delete it

				  const size_t index = SendMessage(hwndList, LB_INSERTSTRING, newindex, (LPARAM)szT);
                  SendMessage(hwndList, LB_SETITEMDATA, index, data);
                  // Set the new value to be selected, like the old one was
                  SendMessage(hwndList, LB_SETSEL, TRUE, index);
                  // Delete the old value
                  SendMessage(hwndList, LB_DELETESTRING, oldindex, 0);
                  delete [] szT;
               }
               delete [] rgsel;
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

			   const size_t count = SendMessage(hwndOut, LB_GETSELCOUNT, 0, 0);
               int * const rgsel = new int[count];
               SendMessage(hwndOut, LB_GETSELITEMS, count, (LPARAM)rgsel);
               for (size_t i=0;i<count;i++)
               {
				  const size_t len = SendMessage(hwndOut, LB_GETTEXTLEN, rgsel[i], 0);
                  char * const szT = new char[len+1]; // include null terminator
                  SendMessage(hwndOut, LB_GETTEXT, rgsel[i], (LPARAM)szT);
				  const size_t data = SendMessage(hwndOut, LB_GETITEMDATA, rgsel[i], 0);

				  const size_t index = SendMessage(hwndIn, LB_ADDSTRING, 0, (LPARAM)szT);
                  SendMessage(hwndIn, LB_SETITEMDATA, index, data);
                  delete [] szT;
               }

               // Remove the old strings after everything else, to avoid messing up indices
               // Remove things in reverse order, so we don't get messed up inside this loop
               for (size_t i=0;i<count;i++)
                  SendMessage(hwndOut, LB_DELETESTRING, rgsel[count-i-1], 0);

               delete [] rgsel;
            }
            break;

         case IDOK:
            {
               CollectionDialogStruct * const pcds = (CollectionDialogStruct *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

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

			   const size_t count = SendMessage(hwndIn, LB_GETCOUNT, 0, 0);

			   for (size_t i = 0; i<count; i++)
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
			   const size_t fEvents = SendMessage(hwndFireEvents, BM_GETCHECK, 0, 0);
               pcol->m_fFireEvents = fEvents;

               HWND hwndStopSingle = GetDlgItem(hwndDlg, IDC_SUPPRESS);
			   const size_t fStopSingle = SendMessage(hwndStopSingle, BM_GETCHECK, 0, 0);
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

INT_PTR CALLBACK TableInfoProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   CCO(PinTable) *pt = (CCO(PinTable) *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

   switch (uMsg)
   {
   case WM_INITDIALOG:
      {
         SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
         pt = (CCO(PinTable) *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

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
            Texture * const pin = pt->m_vimage.ElementAt(i);
            if (pin->m_ppb)
               SendMessage(hwndList, CB_ADDSTRING, 0, (LPARAM)pin->m_szName);
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
               if (szCustomName[0] != '\0')
               {
                  LVFINDINFO lvfi;							
                  lvfi.flags = LVFI_STRING;
                  lvfi.psz = szCustomName;

                  const int found = ListView_FindItem(GetDlgItem(hwndDlg, IDC_CUSTOMLIST), -1, &lvfi);

                  if (found != -1)
                     ListView_DeleteItem(GetDlgItem(hwndDlg, IDC_CUSTOMLIST), found);

                  char *szCustomValue;
                  VPGetDialogItemText(hwndDlg, IDC_CUSTOMVALUE, &szCustomValue);
                  pt->AddListItem(GetDlgItem(hwndDlg, IDC_CUSTOMLIST), szCustomName, szCustomValue, NULL);
                  delete [] szCustomValue;
               }
               delete [] szCustomName;
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
               SAFE_VECTOR_DELETE(pt->m_szTableName);
               SAFE_VECTOR_DELETE(pt->m_szAuthor);
               SAFE_VECTOR_DELETE(pt->m_szVersion);
               SAFE_VECTOR_DELETE(pt->m_szReleaseDate);
               SAFE_VECTOR_DELETE(pt->m_szAuthorEMail);
               SAFE_VECTOR_DELETE(pt->m_szWebSite);
               SAFE_VECTOR_DELETE(pt->m_szBlurb);
               SAFE_VECTOR_DELETE(pt->m_szDescription);
               SAFE_VECTOR_DELETE(pt->m_szRules);

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
               delete [] szT;
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
               delete [] szLong;
               delete [] szEMail;
               delete [] szTableName;
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
   if(uMsg == WM_GETDLGCODE)
      // Eat all acceleratable messges
      return (DLGC_WANTARROWS | DLGC_WANTTAB | DLGC_WANTALLKEYS | DLGC_WANTCHARS);
   else
	  return CallWindowProc(g_ButtonProc, hwnd, uMsg, wParam, lParam);
}

INT_PTR CALLBACK KeysProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   //CCO(PinTable) *pt;
   //pt = (CCO(PinTable) *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
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
         HWND hwndCheck;
         int key;
         int selected;

         hwndCheck = GetDlgItem(hwndDlg, IDC_DefaultLayout);
         hr = GetRegInt("Player", "PBWDefaultLayout", &key);
         if (hr != S_OK)
            key = fFalse;
         SendMessage(hwndCheck, BM_SETCHECK, key ? BST_CHECKED : BST_UNCHECKED, 0);

         hwndCheck = GetDlgItem(hwndDlg, IDC_DisableESC_CB);
         hr = GetRegInt("Player", "DisableESC", &key);
         if (hr != S_OK)
            key = fFalse;
         SendMessage(hwndCheck, BM_SETCHECK, key ? BST_CHECKED : BST_UNCHECKED, 0);

         hwndCheck = GetDlgItem(hwndDlg, IDC_CBGLOBALROTATION);
         hr = GetRegInt("Player", "PBWRotationCB", &key);
         if (hr != S_OK)
            key = 0;
         SendMessage(hwndCheck, BM_SETCHECK, key ? BST_CHECKED : BST_UNCHECKED, 0);

         hr = GetRegInt("Player", "PBWRotationValue", &key);
         if (hr != S_OK)
            key = 0;
         SetDlgItemInt(hwndDlg, IDC_GLOBALROTATION, key, FALSE);

         hwndCheck = GetDlgItem(hwndDlg, IDC_CBGLOBALTILT);
         hr = GetRegInt("Player", "TiltSensCB", &key);
         if (hr != S_OK)
            key = 0;
         SendMessage(hwndCheck, BM_SETCHECK, key ? BST_CHECKED : BST_UNCHECKED, 0);

         hr = GetRegInt("Player", "TiltSensValue", &key);
         if (hr != S_OK)
            key = 400;
         SetDlgItemInt(hwndDlg, IDC_GLOBALTILT, key, FALSE);

         hr = GetRegInt("Player", "DeadZone", &key);
         if (hr != S_OK)
            key = 0;
         SetDlgItemInt(hwndDlg, IDC_DEADZONEAMT, key, FALSE);

         hwndCheck = GetDlgItem(hwndDlg, IDC_GLOBALACCEL);
         hr = GetRegInt("Player", "PBWEnabled", &key);
         if (hr != S_OK)
            key = 0;
         SendMessage(hwndCheck, BM_SETCHECK, key ? BST_CHECKED : BST_UNCHECKED, 0);

         hwndCheck = GetDlgItem(hwndDlg, IDC_GLOBALNMOUNT);
         hr = GetRegInt("Player", "PBWNormalMount", &key);
         if (hr != S_OK)
            key = 0;
         SendMessage(hwndCheck, BM_SETCHECK, key ? BST_CHECKED : BST_UNCHECKED, 0);

         hwndCheck = GetDlgItem(hwndDlg, IDC_ReversePlunger);
         hr = GetRegInt("Player", "ReversePlungerAxis", &key);
         if (hr != S_OK)
            key = fFalse;
         SendMessage(hwndCheck, BM_SETCHECK, key ? BST_CHECKED : BST_UNCHECKED, 0);

         hwndCheck = GetDlgItem(hwndDlg, IDC_LRAXISFLIP);
         hr = GetRegInt("Player", "LRAxisFlip", &key);
         if (hr != S_OK)
            key = fFalse;
         SendMessage(hwndCheck, BM_SETCHECK, key ? BST_CHECKED : BST_UNCHECKED, 0);

         hwndCheck = GetDlgItem(hwndDlg, IDC_UDAXISFLIP);
         hr = GetRegInt("Player", "UDAxisFlip", &key);
         if (hr != S_OK)
            key = fFalse;
         SendMessage(hwndCheck, BM_SETCHECK, key ? BST_CHECKED : BST_UNCHECKED, 0);

         hr = GetRegInt("Player", "PBWAccelGainX", &key);
         if (hr != S_OK)
            key = 150;
         SetDlgItemInt(hwndDlg, IDC_LRAXISGAIN, key, FALSE);

         hr = GetRegInt("Player", "PBWAccelGainY", &key);
         if (hr != S_OK)
            key = 150;
         SetDlgItemInt(hwndDlg, IDC_UDAXISGAIN, key, FALSE);

         hr = GetRegInt("Player", "PBWAccelMaxX", &key);
         if (hr != S_OK)
            key = 100;
         SetDlgItemInt(hwndDlg, IDC_XMAX_EDIT, key, FALSE);

         hr = GetRegInt("Player", "PBWAccelMaxY", &key);
         if (hr != S_OK)
            key = 100;
         SetDlgItemInt(hwndDlg, IDC_YMAX_EDIT, key, FALSE);

         hwndCheck = GetDlgItem(hwndDlg, IDC_ENABLE_MOUSE_PLAYER);
         hr = GetRegInt("Player", "EnableMouseInPlayer", &key);
         if (hr != S_OK)
            key = fTrue;
         SendMessage(hwndCheck, BM_SETCHECK, key ? BST_CHECKED : BST_UNCHECKED, 0);

         hwndCheck = GetDlgItem(hwndDlg, IDC_ENABLE_NUDGE_FILTER);
         hr = GetRegInt("Player", "EnableNudgeFilter", &key);
         if (hr != S_OK)
            key = fFalse;
         SendMessage(hwndCheck, BM_SETCHECK, key ? BST_CHECKED : BST_UNCHECKED, 0);

		 for(unsigned int i = 0; i <= 28; ++i)
		 {
			 int item;
			 switch(i)
			 {
			 case 0: hr = GetRegInt("Player", "JoyLFlipKey", &selected); item = IDC_JOYLFLIPCOMBO; break;
			 case 1: hr = GetRegInt("Player", "JoyRFlipKey", &selected); item = IDC_JOYRFLIPCOMBO; break;
			 case 2: hr = GetRegInt("Player", "JoyPlungerKey", &selected); item = IDC_JOYPLUNGERCOMBO; break;
			 case 3: hr = GetRegInt("Player", "JoyAddCreditKey", &selected); item = IDC_JOYADDCREDITCOMBO; break;
			 case 4: hr = GetRegInt("Player", "JoyAddCredit2Key", &selected); item = IDC_JOYADDCREDIT2COMBO; break;
			 case 5: hr = GetRegInt("Player", "JoyLMagnaSave", &selected); item = IDC_JOYLMAGNACOMBO; break;
			 case 6: hr = GetRegInt("Player", "JoyRMagnaSave", &selected); item = IDC_JOYRMAGNACOMBO; break;
			 case 7: hr = GetRegInt("Player", "JoyStartGameKey", &selected); item = IDC_JOYSTARTCOMBO; break;
			 case 8: hr = GetRegInt("Player", "JoyExitGameKey", &selected); item = IDC_JOYEXITCOMBO; break;
			 case 9: hr = GetRegInt("Player", "JoyFrameCount", &selected); item = IDC_JOYFPSCOMBO; break;
			 case 10:hr = GetRegInt("Player", "JoyVolumeUp", &selected); item = IDC_JOYVOLUPCOMBO; break;
			 case 11:hr = GetRegInt("Player", "JoyVolumeDown", &selected); item = IDC_JOYVOLDNCOMBO; break;
			 case 12:hr = GetRegInt("Player", "JoyLTiltKey", &selected); item = IDC_JOYLTILTCOMBO; break;
			 case 13:hr = GetRegInt("Player", "JoyCTiltKey", &selected); item = IDC_JOYCTILTCOMBO; break;
			 case 14:hr = GetRegInt("Player", "JoyRTiltKey", &selected); item = IDC_JOYRTILTCOMBO; break;
			 case 15:hr = GetRegInt("Player", "JoyMechTiltKey", &selected); item = IDC_JOYMECHTILTCOMBO; break;
			 case 16:hr = GetRegInt("Player", "JoyDebugKey", &selected); item = IDC_JOYDEBUGCOMBO; break;
			 case 17:hr = GetRegInt("Player", "JoyCustom1", &selected); item = IDC_JOYCUSTOM1COMBO; break;
			 case 18:hr = GetRegInt("Player", "JoyCustom2", &selected); item = IDC_JOYCUSTOM2COMBO; break;
			 case 19:hr = GetRegInt("Player", "JoyCustom3", &selected); item = IDC_JOYCUSTOM3COMBO; break;
			 case 20:hr = GetRegInt("Player", "JoyCustom4", &selected); item = IDC_JOYCUSTOM4COMBO; break;
			 case 21:hr = GetRegInt("Player", "JoyPMBuyIn", &selected); item = IDC_JOYPMBUYIN; break;
			 case 22:hr = GetRegInt("Player", "JoyPMCoin3", &selected); item = IDC_JOYPMCOIN3; break;
			 case 23:hr = GetRegInt("Player", "JoyPMCoin4", &selected); item = IDC_JOYPMCOIN4; break;
			 case 24:hr = GetRegInt("Player", "JoyPMCoinDoor", &selected); item = IDC_JOYPMCOINDOOR; break;
			 case 25:hr = GetRegInt("Player", "JoyPMCancel", &selected); item = IDC_JOYPMCANCEL; break;
			 case 26:hr = GetRegInt("Player", "JoyPMDown", &selected); item = IDC_JOYPMDOWN; break;
			 case 27:hr = GetRegInt("Player", "JoyPMUp", &selected); item = IDC_JOYPMUP; break;
			 case 28:hr = GetRegInt("Player", "JoyPMEnter", &selected); item = IDC_JOYPMENTER; break;
			 }

			 if (hr != S_OK)
				selected = 0; // assume no assignment as standard

			 SendMessage(GetDlgItem(hwndDlg, item), CB_ADDSTRING, 0, (LPARAM)" (none)");
			 SendMessage(GetDlgItem(hwndDlg, item), CB_ADDSTRING, 0, (LPARAM)"Button 1");
			 SendMessage(GetDlgItem(hwndDlg, item), CB_ADDSTRING, 0, (LPARAM)"Button 2");
			 SendMessage(GetDlgItem(hwndDlg, item), CB_ADDSTRING, 0, (LPARAM)"Button 3"); 
			 SendMessage(GetDlgItem(hwndDlg, item), CB_ADDSTRING, 0, (LPARAM)"Button 4");
			 SendMessage(GetDlgItem(hwndDlg, item), CB_ADDSTRING, 0, (LPARAM)"Button 5");
			 SendMessage(GetDlgItem(hwndDlg, item), CB_ADDSTRING, 0, (LPARAM)"Button 6");
			 SendMessage(GetDlgItem(hwndDlg, item), CB_ADDSTRING, 0, (LPARAM)"Button 7"); 
			 SendMessage(GetDlgItem(hwndDlg, item), CB_ADDSTRING, 0, (LPARAM)"Button 8"); 
			 SendMessage(GetDlgItem(hwndDlg, item), CB_ADDSTRING, 0, (LPARAM)"Button 9"); 
			 SendMessage(GetDlgItem(hwndDlg, item), CB_ADDSTRING, 0, (LPARAM)"Button 10"); 
			 SendMessage(GetDlgItem(hwndDlg, item), CB_ADDSTRING, 0, (LPARAM)"Button 11"); 
			 SendMessage(GetDlgItem(hwndDlg, item), CB_ADDSTRING, 0, (LPARAM)"Button 12"); 
			 SendMessage(GetDlgItem(hwndDlg, item), CB_ADDSTRING, 0, (LPARAM)"Button 13"); 
			 SendMessage(GetDlgItem(hwndDlg, item), CB_ADDSTRING, 0, (LPARAM)"Button 14");
			 SendMessage(GetDlgItem(hwndDlg, item), CB_ADDSTRING, 0, (LPARAM)"Button 15");
			 SendMessage(GetDlgItem(hwndDlg, item), CB_ADDSTRING, 0, (LPARAM)"Button 16");
			 SendMessage(GetDlgItem(hwndDlg, item), CB_ADDSTRING, 0, (LPARAM)"Button 17"); 
			 SendMessage(GetDlgItem(hwndDlg, item), CB_ADDSTRING, 0, (LPARAM)"Button 18"); 
			 SendMessage(GetDlgItem(hwndDlg, item), CB_ADDSTRING, 0, (LPARAM)"Button 19"); 
			 SendMessage(GetDlgItem(hwndDlg, item), CB_ADDSTRING, 0, (LPARAM)"Button 20"); 
			 SendMessage(GetDlgItem(hwndDlg, item), CB_ADDSTRING, 0, (LPARAM)"Button 21"); 
			 SendMessage(GetDlgItem(hwndDlg, item), CB_ADDSTRING, 0, (LPARAM)"Button 22"); 
			 SendMessage(GetDlgItem(hwndDlg, item), CB_ADDSTRING, 0, (LPARAM)"Button 23"); 
			 SendMessage(GetDlgItem(hwndDlg, item), CB_ADDSTRING, 0, (LPARAM)"Button 24"); 
			 SendMessage(GetDlgItem(hwndDlg, item), CB_SETCURSEL, selected, 0);
		 }

		 //

         hr = GetRegInt("Player", "PlungerAxis", &selected);
         if (hr != S_OK)
            selected = 3; // assume Z Axis as standard
         SendMessage(GetDlgItem(hwndDlg, IDC_PLUNGERAXIS), CB_ADDSTRING, 0, (LPARAM)"(disabled)");
         SendMessage(GetDlgItem(hwndDlg, IDC_PLUNGERAXIS), CB_ADDSTRING, 0, (LPARAM)"X Axis");
         SendMessage(GetDlgItem(hwndDlg, IDC_PLUNGERAXIS), CB_ADDSTRING, 0, (LPARAM)"Y Axis");
         SendMessage(GetDlgItem(hwndDlg, IDC_PLUNGERAXIS), CB_ADDSTRING, 0, (LPARAM)"Z Axis"); 
         SendMessage(GetDlgItem(hwndDlg, IDC_PLUNGERAXIS), CB_ADDSTRING, 0, (LPARAM)"rX Axis");
         SendMessage(GetDlgItem(hwndDlg, IDC_PLUNGERAXIS), CB_ADDSTRING, 0, (LPARAM)"rY Axis");
         SendMessage(GetDlgItem(hwndDlg, IDC_PLUNGERAXIS), CB_ADDSTRING, 0, (LPARAM)"rZ Axis");
         SendMessage(GetDlgItem(hwndDlg, IDC_PLUNGERAXIS), CB_ADDSTRING, 0, (LPARAM)"Slider 1"); 
         SendMessage(GetDlgItem(hwndDlg, IDC_PLUNGERAXIS), CB_ADDSTRING, 0, (LPARAM)"Slider 2"); 
         SendMessage(GetDlgItem(hwndDlg, IDC_PLUNGERAXIS), CB_SETCURSEL, selected, 0);

         hr = GetRegInt("Player", "LRAxis", &selected);
         if (hr != S_OK)
            selected = 1; // assume X Axis as standard
         SendMessage(GetDlgItem(hwndDlg, IDC_LRAXISCOMBO), CB_ADDSTRING, 0, (LPARAM)"(disabled)");
         SendMessage(GetDlgItem(hwndDlg, IDC_LRAXISCOMBO), CB_ADDSTRING, 0, (LPARAM)"X Axis");
         SendMessage(GetDlgItem(hwndDlg, IDC_LRAXISCOMBO), CB_ADDSTRING, 0, (LPARAM)"Y Axis");
         SendMessage(GetDlgItem(hwndDlg, IDC_LRAXISCOMBO), CB_ADDSTRING, 0, (LPARAM)"Z Axis"); 
         SendMessage(GetDlgItem(hwndDlg, IDC_LRAXISCOMBO), CB_ADDSTRING, 0, (LPARAM)"rX Axis"); 
         SendMessage(GetDlgItem(hwndDlg, IDC_LRAXISCOMBO), CB_ADDSTRING, 0, (LPARAM)"rY Axis");
         SendMessage(GetDlgItem(hwndDlg, IDC_LRAXISCOMBO), CB_ADDSTRING, 0, (LPARAM)"rZ Axis"); 
         SendMessage(GetDlgItem(hwndDlg, IDC_LRAXISCOMBO), CB_ADDSTRING, 0, (LPARAM)"Slider 1"); 
         SendMessage(GetDlgItem(hwndDlg, IDC_LRAXISCOMBO), CB_ADDSTRING, 0, (LPARAM)"Slider 2"); 
         SendMessage(GetDlgItem(hwndDlg, IDC_LRAXISCOMBO), CB_SETCURSEL, selected, 0);

         hr = GetRegInt("Player", "UDAxis", &selected);
         if (hr != S_OK)
            selected = 2; // assume Y Axis as standard
         SendMessage(GetDlgItem(hwndDlg, IDC_UDAXISCOMBO), CB_ADDSTRING, 0, (LPARAM)"(disabled)");
         SendMessage(GetDlgItem(hwndDlg, IDC_UDAXISCOMBO), CB_ADDSTRING, 0, (LPARAM)"X Axis");
         SendMessage(GetDlgItem(hwndDlg, IDC_UDAXISCOMBO), CB_ADDSTRING, 0, (LPARAM)"Y Axis");
         SendMessage(GetDlgItem(hwndDlg, IDC_UDAXISCOMBO), CB_ADDSTRING, 0, (LPARAM)"Z Axis"); 
         SendMessage(GetDlgItem(hwndDlg, IDC_UDAXISCOMBO), CB_ADDSTRING, 0, (LPARAM)"rX Axis");
         SendMessage(GetDlgItem(hwndDlg, IDC_UDAXISCOMBO), CB_ADDSTRING, 0, (LPARAM)"rY Axis");
         SendMessage(GetDlgItem(hwndDlg, IDC_UDAXISCOMBO), CB_ADDSTRING, 0, (LPARAM)"rZ Axis"); 
         SendMessage(GetDlgItem(hwndDlg, IDC_UDAXISCOMBO), CB_ADDSTRING, 0, (LPARAM)"Slider 1"); 
         SendMessage(GetDlgItem(hwndDlg, IDC_UDAXISCOMBO), CB_ADDSTRING, 0, (LPARAM)"Slider 2"); 
         SendMessage(GetDlgItem(hwndDlg, IDC_UDAXISCOMBO), CB_SETCURSEL, selected, 0);

         hr = GetRegInt("Player","LFlipKey", &key);
         if (hr != S_OK || key > 0xdd)
            key = DIK_LSHIFT;
         hwndControl = GetDlgItem(hwndDlg, IDC_LEFTFLIPPER);
         SetWindowText(hwndControl, rgszKeyName[key]);
         SetWindowLongPtr(hwndControl, GWLP_USERDATA, key);

         hr = GetRegInt("Player","RFlipKey", &key);
         if (hr != S_OK || key > 0xdd)
            key = DIK_RSHIFT;
         hwndControl = GetDlgItem(hwndDlg, IDC_RIGHTFLIPPER);
         SetWindowText(hwndControl, rgszKeyName[key]);
         SetWindowLongPtr(hwndControl, GWLP_USERDATA, key);

         hr = GetRegInt("Player","LTiltKey", &key);
         if (hr != S_OK || key > 0xdd)
            key = DIK_Z;
         hwndControl = GetDlgItem(hwndDlg, IDC_LEFTTILT);
         SetWindowText(hwndControl, rgszKeyName[key]);
         SetWindowLongPtr(hwndControl, GWLP_USERDATA, key);

         hr = GetRegInt("Player","RTiltKey", &key);
         if (hr != S_OK || key > 0xdd)
            key = DIK_SLASH;
         hwndControl = GetDlgItem(hwndDlg, IDC_RIGHTTILT);
         SetWindowText(hwndControl, rgszKeyName[key]);
         SetWindowLongPtr(hwndControl, GWLP_USERDATA, key);

         hr = GetRegInt("Player","CTiltKey", &key);
         if (hr != S_OK || key > 0xdd)
            key = DIK_SPACE;
         hwndControl = GetDlgItem(hwndDlg, IDC_CENTERTILT);
         SetWindowText(hwndControl, rgszKeyName[key]);
         SetWindowLongPtr(hwndControl, GWLP_USERDATA, key);

         hr = GetRegInt("Player","PlungerKey", &key);
         if (hr != S_OK || key > 0xdd)
            key = DIK_RETURN;
         hwndControl = GetDlgItem(hwndDlg, IDC_PLUNGER_TEXT);
         SetWindowText(hwndControl, rgszKeyName[key]);
         SetWindowLongPtr(hwndControl, GWLP_USERDATA, key);

         hr = GetRegInt("Player","AddCreditKey", &key);
         if (hr != S_OK || key > 0xdd)
            key = DIK_5;
         hwndControl = GetDlgItem(hwndDlg, IDC_ADDCREDIT);
         SetWindowText(hwndControl, rgszKeyName[key]);
         SetWindowLongPtr(hwndControl, GWLP_USERDATA, key);

         hr = GetRegInt("Player","AddCreditKey2", &key);
         if (hr != S_OK || key > 0xdd)
            key = DIK_4;
         hwndControl = GetDlgItem(hwndDlg, IDC_ADDCREDITKEY2);
         SetWindowText(hwndControl, rgszKeyName[key]);
         SetWindowLongPtr(hwndControl, GWLP_USERDATA, key);

         hr = GetRegInt("Player","StartGameKey", &key);
         if (hr != S_OK || key > 0xdd)
            key = DIK_1;
         hwndControl = GetDlgItem(hwndDlg, IDC_STARTGAME);
         SetWindowText(hwndControl, rgszKeyName[key]);
         SetWindowLongPtr(hwndControl, GWLP_USERDATA, key);

         hr = GetRegInt("Player","ExitGameKey", &key);
         if (hr != S_OK || key > 0xdd)
            key = DIK_Q;
         hwndControl = GetDlgItem(hwndDlg, IDC_EXITGAME);
         SetWindowText(hwndControl, rgszKeyName[key]);
         SetWindowLongPtr(hwndControl, GWLP_USERDATA, key);

         hr = GetRegInt("Player","FrameCount", &key);
         if (hr != S_OK || key > 0xdd)
            key = DIK_F11;
         hwndControl = GetDlgItem(hwndDlg, IDC_FRAMECOUNT);
         SetWindowText(hwndControl, rgszKeyName[key]);
         SetWindowLongPtr(hwndControl, GWLP_USERDATA, key);

         hr = GetRegInt("Player","VolumeUp", &key);
         if (hr != S_OK || key > 0xdd)
            key = DIK_MINUS;
         hwndControl = GetDlgItem(hwndDlg, IDC_VOLUMEUP);
         SetWindowText(hwndControl, rgszKeyName[key]);
         SetWindowLongPtr(hwndControl, GWLP_USERDATA, key);

         hr = GetRegInt("Player","VolumeDown", &key);
         if (hr != S_OK || key > 0xdd)
            key = DIK_EQUALS;
         hwndControl = GetDlgItem(hwndDlg, IDC_VOLUMEDN);
         SetWindowText(hwndControl, rgszKeyName[key]);
         SetWindowLongPtr(hwndControl, GWLP_USERDATA, key);

         hr = GetRegInt("Player","DebugBalls", &key);
         if (hr != S_OK || key > 0xdd)
            key = DIK_O;
         hwndControl = GetDlgItem(hwndDlg, IDC_DEBUGBALL);
         SetWindowText(hwndControl, rgszKeyName[key]);
         SetWindowLongPtr(hwndControl, GWLP_USERDATA, key);

         hr = GetRegInt("Player","RMagnaSave", &key);
         if (hr != S_OK || key > 0xdd)
            //key = DIK_BACKSPACE;
            key = DIK_RCONTROL;		//157 (0x9D) DIK_RCONTROL 
         hwndControl = GetDlgItem(hwndDlg, IDC_RMAGSAVE);
         SetWindowText(hwndControl, rgszKeyName[key]);
         SetWindowLongPtr(hwndControl, GWLP_USERDATA, key);

         hr = GetRegInt("Player","LMagnaSave", &key);
         if (hr != S_OK || key > 0xdd)
            //key = DIK_APOSTROPHE;
            key = DIK_LCONTROL; //29 (0x1D)
         hwndControl = GetDlgItem(hwndDlg, IDC_LMAGSAVE );
         SetWindowText(hwndControl, rgszKeyName[key]);
         SetWindowLongPtr(hwndControl, GWLP_USERDATA, key);

         hr = GetRegInt("Player","MechTilt", &key);
         if (hr != S_OK || key > 0xdd)
            key = DIK_T;
         hwndControl = GetDlgItem(hwndDlg, IDC_MECHTILT );
         SetWindowText(hwndControl, rgszKeyName[key]);
         SetWindowLongPtr(hwndControl, GWLP_USERDATA, key);

         hr = GetRegInt("Player","JoyCustom1Key", &key);
         if (hr != S_OK || key > 0xdd)
            key = DIK_UP;
         hwndControl = GetDlgItem(hwndDlg, IDC_JOYCUSTOM1 );
         SetWindowText(hwndControl, rgszKeyName[key]);
         SetWindowLongPtr(hwndControl, GWLP_USERDATA, key);

         hr = GetRegInt("Player","JoyCustom2Key", &key);
         if (hr != S_OK || key > 0xdd)
            key = DIK_DOWN;
         hwndControl = GetDlgItem(hwndDlg, IDC_JOYCUSTOM2 );
         SetWindowText(hwndControl, rgszKeyName[key]);
         SetWindowLongPtr(hwndControl, GWLP_USERDATA, key);

         hr = GetRegInt("Player","JoyCustom3Key", &key);
         if (hr != S_OK || key > 0xdd)
            key = DIK_LEFT;
         hwndControl = GetDlgItem(hwndDlg, IDC_JOYCUSTOM3 );
         SetWindowText(hwndControl, rgszKeyName[key]);
         SetWindowLongPtr(hwndControl, GWLP_USERDATA, key);

         hr = GetRegInt("Player","JoyCustom4Key", &key);
         if (hr != S_OK || key > 0xdd)
            key = DIK_RIGHT;
         hwndControl = GetDlgItem(hwndDlg, IDC_JOYCUSTOM4 );
         SetWindowText(hwndControl, rgszKeyName[key]);
         SetWindowLongPtr(hwndControl, GWLP_USERDATA, key);

		 //

         KeyWindowStruct * const pksw = new KeyWindowStruct();
         pksw->pi.Init(hwndDlg);
         pksw->m_timerid = 0;
         SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (size_t)pksw);

         // Set buttons to ignore keyboard shortcuts when using DirectInput
         HWND hwndButton;
         hwndButton = GetDlgItem(hwndDlg, IDC_LEFTFLIPPERBUTTON);
         g_ButtonProc = (WNDPROC)GetWindowLongPtr(hwndButton, GWLP_WNDPROC);
         SetWindowLongPtr(hwndButton, GWLP_WNDPROC, (size_t)MyKeyButtonProc);
         SetWindowLongPtr(hwndButton, GWLP_USERDATA, (size_t)pksw);

         hwndButton = GetDlgItem(hwndDlg, IDC_RIGHTFLIPPERBUTTON);
         SetWindowLongPtr(hwndButton, GWLP_WNDPROC, (size_t)MyKeyButtonProc);
         SetWindowLongPtr(hwndButton, GWLP_USERDATA, (size_t)pksw);

         hwndButton = GetDlgItem(hwndDlg, IDC_LEFTTILTBUTTON);
         SetWindowLongPtr(hwndButton, GWLP_WNDPROC, (size_t)MyKeyButtonProc);
         SetWindowLongPtr(hwndButton, GWLP_USERDATA, (size_t)pksw);

         hwndButton = GetDlgItem(hwndDlg, IDC_RIGHTTILTBUTTON);
         SetWindowLongPtr(hwndButton, GWLP_WNDPROC, (size_t)MyKeyButtonProc);
         SetWindowLongPtr(hwndButton, GWLP_USERDATA, (size_t)pksw);

         hwndButton = GetDlgItem(hwndDlg, IDC_CENTERTILTBUTTON);
         SetWindowLongPtr(hwndButton, GWLP_WNDPROC, (size_t)MyKeyButtonProc);
         SetWindowLongPtr(hwndButton, GWLP_USERDATA, (size_t)pksw);

         hwndButton = GetDlgItem(hwndDlg, IDC_PLUNGERBUTTON);
         SetWindowLongPtr(hwndButton, GWLP_WNDPROC, (size_t)MyKeyButtonProc);
         SetWindowLongPtr(hwndButton, GWLP_USERDATA, (size_t)pksw);

         hwndButton = GetDlgItem(hwndDlg, IDC_ADDCREDITBUTTON);
         SetWindowLongPtr(hwndButton, GWLP_WNDPROC, (size_t)MyKeyButtonProc);
         SetWindowLongPtr(hwndButton, GWLP_USERDATA, (size_t)pksw);

         hwndButton = GetDlgItem(hwndDlg, IDC_ADDCREDITBUTTON2);
         SetWindowLongPtr(hwndButton, GWLP_WNDPROC, (size_t)MyKeyButtonProc);
         SetWindowLongPtr(hwndButton, GWLP_USERDATA, (size_t)pksw);

         hwndButton = GetDlgItem(hwndDlg, IDC_STARTGAMEBUTTON);
         SetWindowLongPtr(hwndButton, GWLP_WNDPROC, (size_t)MyKeyButtonProc);
         SetWindowLongPtr(hwndButton, GWLP_USERDATA, (size_t)pksw);

         hwndButton = GetDlgItem(hwndDlg, IDC_EXITGAMEBUTTON);
         SetWindowLongPtr(hwndButton, GWLP_WNDPROC, (size_t)MyKeyButtonProc);
         SetWindowLongPtr(hwndButton, GWLP_USERDATA, (size_t)pksw);

         hwndButton = GetDlgItem(hwndDlg, IDC_FRAMECOUNTBUTTON);
         SetWindowLongPtr(hwndButton, GWLP_WNDPROC, (size_t)MyKeyButtonProc);
         SetWindowLongPtr(hwndButton, GWLP_USERDATA, (size_t)pksw);

         hwndButton = GetDlgItem(hwndDlg, IDC_VOLUPBUTTON);
         SetWindowLongPtr(hwndButton, GWLP_WNDPROC, (size_t)MyKeyButtonProc);
         SetWindowLongPtr(hwndButton, GWLP_USERDATA, (size_t)pksw);

         hwndButton = GetDlgItem(hwndDlg, IDC_VOLDOWNBUTTON);
         SetWindowLongPtr(hwndButton, GWLP_WNDPROC, (size_t)MyKeyButtonProc);
         SetWindowLongPtr(hwndButton, GWLP_USERDATA, (size_t)pksw);

         hwndButton = GetDlgItem(hwndDlg, IDC_DEBUGBALLSBUTTON);
         SetWindowLongPtr(hwndButton, GWLP_WNDPROC, (size_t)MyKeyButtonProc);
         SetWindowLongPtr(hwndButton, GWLP_USERDATA, (size_t)pksw);

         hwndButton = GetDlgItem(hwndDlg, IDC_RMAGSAVEBUTTON);
         SetWindowLongPtr(hwndButton, GWLP_WNDPROC, (size_t)MyKeyButtonProc);
         SetWindowLongPtr(hwndButton, GWLP_USERDATA, (size_t)pksw);

         hwndButton = GetDlgItem(hwndDlg, IDC_LMAGSAVEBUTTON);
         SetWindowLongPtr(hwndButton, GWLP_WNDPROC, (size_t)MyKeyButtonProc);
         SetWindowLongPtr(hwndButton, GWLP_USERDATA, (size_t)pksw);

         hwndButton = GetDlgItem(hwndDlg, IDC_MECHTILTBUTTON);
         SetWindowLongPtr(hwndButton, GWLP_WNDPROC, (size_t)MyKeyButtonProc);
         SetWindowLongPtr(hwndButton, GWLP_USERDATA, (size_t)pksw);

         hwndButton = GetDlgItem(hwndDlg, IDC_JOYCUSTOM1BUTTON);
         SetWindowLongPtr(hwndButton, GWLP_WNDPROC, (size_t)MyKeyButtonProc);
         SetWindowLongPtr(hwndButton, GWLP_USERDATA, (size_t)pksw);

         hwndButton = GetDlgItem(hwndDlg, IDC_JOYCUSTOM2BUTTON);
         SetWindowLongPtr(hwndButton, GWLP_WNDPROC, (size_t)MyKeyButtonProc);
         SetWindowLongPtr(hwndButton, GWLP_USERDATA, (size_t)pksw);

         hwndButton = GetDlgItem(hwndDlg, IDC_JOYCUSTOM3BUTTON);
         SetWindowLongPtr(hwndButton, GWLP_WNDPROC, (size_t)MyKeyButtonProc);
         SetWindowLongPtr(hwndButton, GWLP_USERDATA, (size_t)pksw);

         hwndButton = GetDlgItem(hwndDlg, IDC_JOYCUSTOM4BUTTON);
         SetWindowLongPtr(hwndButton, GWLP_WNDPROC, (size_t)MyKeyButtonProc);
         SetWindowLongPtr(hwndButton, GWLP_USERDATA, (size_t)pksw);

         return TRUE;
      }
      break;

   case WM_DESTROY:
      {
         KeyWindowStruct * const pksw = (KeyWindowStruct *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
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
         KeyWindowStruct * const pksw = (KeyWindowStruct *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
         const int key = pksw->pi.GetNextKey();
         if (key != 0)
         {
            if(key < 0xDD)	// Key mapping
            {
               if (key == DIK_ESCAPE)
               {
                  // reset key to old value
				  const size_t oldkey = GetWindowLongPtr(pksw->hwndKeyControl, GWLP_USERDATA);
                  SetWindowText(pksw->hwndKeyControl, rgszKeyName[oldkey]);
               }
               else
               {
                  SetWindowText(pksw->hwndKeyControl, rgszKeyName[key]);
                  SetWindowLongPtr(pksw->hwndKeyControl, GWLP_USERDATA, key);
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
            KeyWindowStruct * const pksw = (KeyWindowStruct *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
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
            case IDC_JOYCUSTOM1BUTTON:
            case IDC_JOYCUSTOM2BUTTON:
            case IDC_JOYCUSTOM3BUTTON:
            case IDC_JOYCUSTOM4BUTTON:
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
                        hwndKeyWindow = GetDlgItem(hwndDlg, IDC_PLUNGER_TEXT);
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

                     case IDC_JOYCUSTOM1BUTTON:
                        hwndKeyWindow = GetDlgItem(hwndDlg, IDC_JOYCUSTOM1);
                        break;

                     case IDC_JOYCUSTOM2BUTTON:
                        hwndKeyWindow = GetDlgItem(hwndDlg, IDC_JOYCUSTOM2);
                        break;

                     case IDC_JOYCUSTOM3BUTTON:
                        hwndKeyWindow = GetDlgItem(hwndDlg, IDC_JOYCUSTOM3);
                        break;

                     case IDC_JOYCUSTOM4BUTTON:
                        hwndKeyWindow = GetDlgItem(hwndDlg, IDC_JOYCUSTOM4);
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
				  size_t key;
				  size_t selected;
				  size_t newvalue;

                  hwndControl = GetDlgItem(hwndDlg, IDC_JOYCUSTOM1COMBO);
                  selected = SendMessage(hwndControl, CB_GETCURSEL, 0, 0);
                  if (selected == LB_ERR)
                     selected = 0; // assume no assignment as standard
                  SetRegValue("Player", "JoyCustom1", REG_DWORD, &selected, 4);

                  hwndControl = GetDlgItem(hwndDlg, IDC_JOYCUSTOM2COMBO);
                  selected = SendMessage(hwndControl, CB_GETCURSEL, 0, 0);
                  if (selected == LB_ERR)
                     selected = 0; // assume no assignment as standard
                  SetRegValue("Player", "JoyCustom2", REG_DWORD, &selected, 4);

                  hwndControl = GetDlgItem(hwndDlg, IDC_JOYCUSTOM3COMBO);
                  selected = SendMessage(hwndControl, CB_GETCURSEL, 0, 0);
                  if (selected == LB_ERR)
                     selected = 0; // assume no assignment as standard
                  SetRegValue("Player", "JoyCustom3", REG_DWORD, &selected, 4);

                  hwndControl = GetDlgItem(hwndDlg, IDC_JOYCUSTOM4COMBO);
                  selected = SendMessage(hwndControl, CB_GETCURSEL, 0, 0);
                  if (selected == LB_ERR)
                     selected = 0; // assume no assignment as standard
                  SetRegValue("Player", "JoyCustom4", REG_DWORD, &selected, 4);

                  hwndControl = GetDlgItem(hwndDlg, IDC_JOYPMBUYIN);
                  selected = SendMessage(hwndControl, CB_GETCURSEL, 0, 0);
                  if (selected == LB_ERR)
                     selected = 0; // assume no assignment as standard
                  SetRegValue("Player", "JoyPMBuyIn", REG_DWORD, &selected, 4);

                  hwndControl = GetDlgItem(hwndDlg, IDC_JOYPMCOIN3);
                  selected = SendMessage(hwndControl, CB_GETCURSEL, 0, 0);
                  if (selected == LB_ERR)
                     selected = 0; // assume no assignment as standard
                  SetRegValue("Player", "JoyPMCoin3", REG_DWORD, &selected, 4);

                  hwndControl = GetDlgItem(hwndDlg, IDC_JOYPMCOIN4);
                  selected = SendMessage(hwndControl, CB_GETCURSEL, 0, 0);
                  if (selected == LB_ERR)
                     selected = 0; // assume no assignment as standard
                  SetRegValue("Player", "JoyPMCoin4", REG_DWORD, &selected, 4);

                  hwndControl = GetDlgItem(hwndDlg, IDC_JOYPMCOINDOOR);
                  selected = SendMessage(hwndControl, CB_GETCURSEL, 0, 0);
                  if (selected == LB_ERR)
                     selected = 0; // assume no assignment as standard
                  SetRegValue("Player", "JoyPMCoinDoor", REG_DWORD, &selected, 4);

                  hwndControl = GetDlgItem(hwndDlg, IDC_JOYPMCANCEL);
                  selected = SendMessage(hwndControl, CB_GETCURSEL, 0, 0);
                  if (selected == LB_ERR)
                     selected = 0; // assume no assignment as standard
                  SetRegValue("Player", "JoyPMCancel", REG_DWORD, &selected, 4);

                  hwndControl = GetDlgItem(hwndDlg, IDC_JOYPMDOWN);
                  selected = SendMessage(hwndControl, CB_GETCURSEL, 0, 0);
                  if (selected == LB_ERR)
                     selected = 0; // assume no assignment as standard
                  SetRegValue("Player", "JoyPMDown", REG_DWORD, &selected, 4);

                  hwndControl = GetDlgItem(hwndDlg, IDC_JOYPMUP);
                  selected = SendMessage(hwndControl, CB_GETCURSEL, 0, 0);
                  if (selected == LB_ERR)
                     selected = 0; // assume no assignment as standard
                  SetRegValue("Player", "JoyPMUp", REG_DWORD, &selected, 4);

                  hwndControl = GetDlgItem(hwndDlg, IDC_JOYPMENTER);
                  selected = SendMessage(hwndControl, CB_GETCURSEL, 0, 0);
                  if (selected == LB_ERR)
                     selected = 0; // assume no assignment as standard
                  SetRegValue("Player", "JoyPMEnter", REG_DWORD, &selected, 4);

                  hwndControl = GetDlgItem(hwndDlg, IDC_JOYLFLIPCOMBO);
                  selected = SendMessage(hwndControl, CB_GETCURSEL, 0, 0);
                  if (selected == LB_ERR)
                     selected = 0; // assume no assignment as standard
                  SetRegValue("Player", "JoyLFlipKey", REG_DWORD, &selected, 4);

                  hwndControl = GetDlgItem(hwndDlg, IDC_JOYRFLIPCOMBO);
                  selected = SendMessage(hwndControl, CB_GETCURSEL, 0, 0);
                  if (selected == LB_ERR)
                     selected = 0; // assume no assignment as standard
                  SetRegValue("Player", "JoyRFlipKey", REG_DWORD, &selected, 4);

                  hwndControl = GetDlgItem(hwndDlg, IDC_JOYPLUNGERCOMBO);
                  selected = SendMessage(hwndControl, CB_GETCURSEL, 0, 0);
                  if (selected == LB_ERR)
                     selected = 0; // assume no assignment as standard
                  SetRegValue("Player", "JoyPlungerKey", REG_DWORD, &selected, 4);

                  hwndControl = GetDlgItem(hwndDlg, IDC_JOYADDCREDITCOMBO);
                  selected = SendMessage(hwndControl, CB_GETCURSEL, 0, 0);
                  if (selected == LB_ERR)
                     selected = 0; // assume no assignment as standard
                  SetRegValue("Player", "JoyAddCreditKey", REG_DWORD, &selected, 4);

                  hwndControl = GetDlgItem(hwndDlg, IDC_JOYADDCREDIT2COMBO);
                  selected = SendMessage(hwndControl, CB_GETCURSEL, 0, 0);
                  if (selected == LB_ERR)
                     selected = 0; // assume no assignment as standard
                  SetRegValue("Player", "JoyAddCredit2Key", REG_DWORD, &selected, 4);

                  hwndControl = GetDlgItem(hwndDlg, IDC_JOYLMAGNACOMBO);
                  selected = SendMessage(hwndControl, CB_GETCURSEL, 0, 0);
                  if (selected == LB_ERR)
                     selected = 0; // assume no assignment as standard
                  SetRegValue("Player", "JoyLMagnaSave", REG_DWORD, &selected, 4);

                  hwndControl = GetDlgItem(hwndDlg, IDC_JOYRMAGNACOMBO);
                  selected = SendMessage(hwndControl, CB_GETCURSEL, 0, 0);
                  if (selected == LB_ERR)
                     selected = 0; // assume no assignment as standard
                  SetRegValue("Player", "JoyRMagnaSave", REG_DWORD, &selected, 4);

                  hwndControl = GetDlgItem(hwndDlg, IDC_JOYSTARTCOMBO);
                  selected = SendMessage(hwndControl, CB_GETCURSEL, 0, 0);
                  if (selected == LB_ERR)
                     selected = 0; // assume no assignment as standard
                  SetRegValue("Player", "JoyStartGameKey", REG_DWORD, &selected, 4);

                  hwndControl = GetDlgItem(hwndDlg, IDC_JOYEXITCOMBO);
                  selected = SendMessage(hwndControl, CB_GETCURSEL, 0, 0);
                  if (selected == LB_ERR)
                     selected = 0; // assume no assignment as standard
                  SetRegValue("Player", "JoyExitGameKey", REG_DWORD, &selected, 4);

                  hwndControl = GetDlgItem(hwndDlg, IDC_JOYFPSCOMBO);
                  selected = SendMessage(hwndControl, CB_GETCURSEL, 0, 0);
                  if (selected == LB_ERR)
                     selected = 0; // assume no assignment as standard
                  SetRegValue("Player", "JoyFrameCount", REG_DWORD, &selected, 4);

                  hwndControl = GetDlgItem(hwndDlg, IDC_JOYVOLUPCOMBO);
                  selected = SendMessage(hwndControl, CB_GETCURSEL, 0, 0);
                  if (selected == LB_ERR)
                     selected = 0; // assume no assignment as standard
                  SetRegValue("Player", "JoyVolumeUp", REG_DWORD, &selected, 4);

                  hwndControl = GetDlgItem(hwndDlg, IDC_JOYVOLDNCOMBO);
                  selected = SendMessage(hwndControl, CB_GETCURSEL, 0, 0);
                  if (selected == LB_ERR)
                     selected = 0; // assume no assignment as standard
                  SetRegValue("Player", "JoyVolumeDown", REG_DWORD, &selected, 4);

                  hwndControl = GetDlgItem(hwndDlg, IDC_JOYLTILTCOMBO);
                  selected = SendMessage(hwndControl, CB_GETCURSEL, 0, 0);
                  if (selected == LB_ERR)
                     selected = 0; // assume no assignment as standard
                  SetRegValue("Player", "JoyLTiltKey", REG_DWORD, &selected, 4);

                  hwndControl = GetDlgItem(hwndDlg, IDC_JOYCTILTCOMBO);
                  selected = SendMessage(hwndControl, CB_GETCURSEL, 0, 0);
                  if (selected == LB_ERR)
                     selected = 0; // assume no assignment as standard
                  SetRegValue("Player", "JoyCTiltKey", REG_DWORD, &selected, 4);

                  hwndControl = GetDlgItem(hwndDlg, IDC_JOYRTILTCOMBO);
                  selected = SendMessage(hwndControl, CB_GETCURSEL, 0, 0);
                  if (selected == LB_ERR)
                     selected = 0; // assume no assignment as standard
                  SetRegValue("Player", "JoyRTiltKey", REG_DWORD, &selected, 4);

                  hwndControl = GetDlgItem(hwndDlg, IDC_JOYMECHTILTCOMBO);
                  selected = SendMessage(hwndControl, CB_GETCURSEL, 0, 0);
                  if (selected == LB_ERR)
                     selected = 0; // assume no assignment as standard
                  SetRegValue("Player", "JoyMechTiltKey", REG_DWORD, &selected, 4);

                  hwndControl = GetDlgItem(hwndDlg, IDC_JOYDEBUGCOMBO);
                  selected = SendMessage(hwndControl, CB_GETCURSEL, 0, 0);
                  if (selected == LB_ERR)
                     selected = 0; // assume no assignment as standard
                  SetRegValue("Player", "JoyDebugKey", REG_DWORD, &selected, 4);

                  hwndControl = GetDlgItem(hwndDlg, IDC_PLUNGERAXIS);
                  selected = SendMessage(hwndControl, CB_GETCURSEL, 0, 0);
                  if (selected == LB_ERR)
                     selected = 3; // assume Z Axis as standard
                  SetRegValue("Player", "PlungerAxis", REG_DWORD, &selected, 4);

                  hwndControl = GetDlgItem(hwndDlg, IDC_LRAXISCOMBO);
                  selected = SendMessage(hwndControl, CB_GETCURSEL, 0, 0);
                  if (selected == LB_ERR)
                     selected = 1; // assume X Axis as standard
                  SetRegValue("Player", "LRAxis", REG_DWORD, &selected, 4);

                  hwndControl = GetDlgItem(hwndDlg, IDC_UDAXISCOMBO);
                  selected = SendMessage(hwndControl, CB_GETCURSEL, 0, 0);
                  if (selected == LB_ERR)
                     selected = 2; // assume Y Axis as standard
                  SetRegValue("Player", "UDAxis", REG_DWORD, &selected, 4);

                  newvalue = GetDlgItemInt(hwndDlg, IDC_LRAXISGAIN, NULL, TRUE);
                  if ((SSIZE_T)newvalue < 0)	{newvalue = 0;}
                  SetRegValue("Player", "PBWAccelGainX", REG_DWORD, &newvalue, 4);

                  newvalue = GetDlgItemInt(hwndDlg, IDC_UDAXISGAIN, NULL, TRUE);
                  if ((SSIZE_T)newvalue < 0)	{newvalue = 0;}
                  SetRegValue("Player", "PBWAccelGainY", REG_DWORD, &newvalue, 4);

                  newvalue = GetDlgItemInt(hwndDlg, IDC_DEADZONEAMT, NULL, TRUE);
                  if ((SSIZE_T)newvalue < 0) {newvalue = 0;}
                  if (newvalue > 100) {newvalue = 100;}
                  SetRegValue("Player", "DeadZone", REG_DWORD, &newvalue, 4);

                  newvalue = GetDlgItemInt(hwndDlg, IDC_XMAX_EDIT, NULL, TRUE);
                  if ((SSIZE_T)newvalue < 0) {newvalue = 0;}
                  if (newvalue > 100) {newvalue = 100;}
                  SetRegValue("Player", "PBWAccelMaxX", REG_DWORD, &newvalue, 4);

                  newvalue = GetDlgItemInt(hwndDlg, IDC_YMAX_EDIT, NULL, TRUE);
                  if ((SSIZE_T)newvalue < 0) {newvalue = 0;}
                  if (newvalue > 100) {newvalue = 100;}
                  SetRegValue("Player", "PBWAccelMaxY", REG_DWORD, &newvalue, 4);

                  hwndControl = GetDlgItem(hwndDlg, IDC_DefaultLayout);
                  selected = SendMessage(hwndControl, BM_GETCHECK, 0, 0);
                  SetRegValue("Player", "PBWDefaultLayout", REG_DWORD, &selected, 4);

                  hwndControl = GetDlgItem(hwndDlg, IDC_DisableESC_CB);
                  selected = SendMessage(hwndControl, BM_GETCHECK, 0, 0);
                  SetRegValue("Player", "DisableESC", REG_DWORD, &selected, 4);

                  hwndControl = GetDlgItem(hwndDlg, IDC_LRAXISFLIP);
                  selected = SendMessage(hwndControl, BM_GETCHECK, 0, 0);
                  SetRegValue("Player", "LRAxisFlip", REG_DWORD, &selected, 4);

                  hwndControl = GetDlgItem(hwndDlg, IDC_UDAXISFLIP);
                  selected = SendMessage(hwndControl, BM_GETCHECK, 0, 0);
                  SetRegValue("Player", "UDAxisFlip", REG_DWORD, &selected, 4);

                  hwndControl = GetDlgItem(hwndDlg, IDC_ReversePlunger);
                  selected = SendMessage(hwndControl, BM_GETCHECK, 0, 0);
                  SetRegValue("Player", "ReversePlungerAxis", REG_DWORD, &selected, 4);

                  hwndControl = GetDlgItem(hwndDlg, IDC_GLOBALACCEL);
                  selected = SendMessage(hwndControl, BM_GETCHECK, 0, 0);
                  SetRegValue("Player", "PBWEnabled", REG_DWORD, &selected, 4);

                  hwndControl = GetDlgItem(hwndDlg, IDC_GLOBALNMOUNT);
                  selected = SendMessage(hwndControl, BM_GETCHECK, 0, 0);
                  SetRegValue("Player", "PBWNormalMount", REG_DWORD, &selected, 4);

                  hwndControl = GetDlgItem(hwndDlg, IDC_CBGLOBALROTATION);
                  key = SendMessage(hwndControl, BM_GETCHECK, 0, 0);
                  SetRegValue("Player", "PBWRotationCB", REG_DWORD, &key, 4);
                  newvalue = GetDlgItemInt(hwndDlg, IDC_GLOBALROTATION, NULL, TRUE);
                  SetRegValue("Player", "PBWRotationValue", REG_DWORD, &newvalue, 4);

                  hwndControl = GetDlgItem(hwndDlg, IDC_CBGLOBALTILT);
                  key = SendMessage(hwndControl, BM_GETCHECK, 0, 0);
                  SetRegValue("Player", "TiltSensCB", REG_DWORD, &key, 4);
                  newvalue = GetDlgItemInt(hwndDlg, IDC_GLOBALTILT, NULL, TRUE);
				  if ((SSIZE_T)newvalue < 0) {newvalue = 0;}
				  if (newvalue > 1000) {newvalue = 1000;}
                  SetRegValue("Player", "TiltSensValue", REG_DWORD, &newvalue, 4);
                  if (key == 1)
                     SetRegValue("Player", "TiltSensitivity", REG_DWORD, &newvalue, 4);
                  else
                  {
                     HKEY hkey;
                     RegOpenKey(HKEY_CURRENT_USER, "Software\\Visual Pinball\\Player", &hkey);
                     RegDeleteValue(hkey, "TiltSensitivity");
                     RegCloseKey(hkey);
                  }

                  hwndControl = GetDlgItem(hwndDlg, IDC_LEFTFLIPPER);
                  key = GetWindowLongPtr(hwndControl, GWLP_USERDATA);
                  SetRegValue("Player", "LFlipKey", REG_DWORD, &key, 4);

                  hwndControl = GetDlgItem(hwndDlg, IDC_RIGHTFLIPPER);
                  key = GetWindowLongPtr(hwndControl, GWLP_USERDATA);
                  SetRegValue("Player", "RFlipKey", REG_DWORD, &key, 4);

                  hwndControl = GetDlgItem(hwndDlg, IDC_LEFTTILT);
                  key = GetWindowLongPtr(hwndControl, GWLP_USERDATA);
                  SetRegValue("Player", "LTiltKey", REG_DWORD, &key, 4);

                  hwndControl = GetDlgItem(hwndDlg, IDC_RIGHTTILT);
                  key = GetWindowLongPtr(hwndControl, GWLP_USERDATA);
                  SetRegValue("Player", "RTiltKey", REG_DWORD, &key, 4);

                  hwndControl = GetDlgItem(hwndDlg, IDC_CENTERTILT);
                  key = GetWindowLongPtr(hwndControl, GWLP_USERDATA);
                  SetRegValue("Player", "CTiltKey", REG_DWORD, &key, 4);

                  hwndControl = GetDlgItem(hwndDlg, IDC_PLUNGER_TEXT);
                  key = GetWindowLongPtr(hwndControl, GWLP_USERDATA);
                  SetRegValue("Player", "PlungerKey", REG_DWORD, &key, 4);

                  hwndControl = GetDlgItem(hwndDlg, IDC_ADDCREDIT);
                  key = GetWindowLongPtr(hwndControl, GWLP_USERDATA);
                  SetRegValue("Player", "AddCreditKey", REG_DWORD, &key, 4);

                  hwndControl = GetDlgItem(hwndDlg, IDC_ADDCREDITKEY2);
                  key = GetWindowLongPtr(hwndControl, GWLP_USERDATA);
                  SetRegValue("Player", "AddCreditKey2", REG_DWORD, &key, 4);

                  hwndControl = GetDlgItem(hwndDlg, IDC_STARTGAME);
                  key = GetWindowLongPtr(hwndControl, GWLP_USERDATA);
                  SetRegValue("Player", "StartGameKey", REG_DWORD, &key, 4);

                  hwndControl = GetDlgItem(hwndDlg, IDC_EXITGAME);
                  key = GetWindowLongPtr(hwndControl, GWLP_USERDATA);
                  SetRegValue("Player", "ExitGameKey", REG_DWORD, &key, 4);

                  hwndControl = GetDlgItem(hwndDlg, IDC_FRAMECOUNT);
                  key = GetWindowLongPtr(hwndControl, GWLP_USERDATA);
                  SetRegValue("Player", "FrameCount", REG_DWORD, &key, 4);

                  hwndControl = GetDlgItem(hwndDlg, IDC_VOLUMEUP);
                  key = GetWindowLongPtr(hwndControl, GWLP_USERDATA);
                  SetRegValue("Player", "VolumeUp", REG_DWORD, &key, 4);

                  hwndControl = GetDlgItem(hwndDlg, IDC_VOLUMEDN);
                  key = GetWindowLongPtr(hwndControl, GWLP_USERDATA);
                  SetRegValue("Player", "VolumeDown", REG_DWORD, &key, 4);

                  hwndControl = GetDlgItem(hwndDlg, IDC_DEBUGBALL);
                  key = GetWindowLongPtr(hwndControl, GWLP_USERDATA);
                  SetRegValue("Player", "DebugBalls", REG_DWORD, &key, 4);

                  hwndControl = GetDlgItem(hwndDlg, IDC_RMAGSAVE);
                  key = GetWindowLongPtr(hwndControl, GWLP_USERDATA);
                  SetRegValue("Player", "RMagnaSave", REG_DWORD, &key, 4);

                  hwndControl = GetDlgItem(hwndDlg, IDC_LMAGSAVE);
                  key = GetWindowLongPtr(hwndControl, GWLP_USERDATA);
                  SetRegValue("Player", "LMagnaSave", REG_DWORD, &key, 4);

                  hwndControl = GetDlgItem(hwndDlg, IDC_MECHTILT);
                  key = GetWindowLongPtr(hwndControl, GWLP_USERDATA);
                  SetRegValue("Player", "MechTilt", REG_DWORD, &key, 4);

                  hwndControl = GetDlgItem(hwndDlg, IDC_JOYCUSTOM1);
                  key = GetWindowLongPtr(hwndControl, GWLP_USERDATA);
                  SetRegValue("Player", "JoyCustom1Key", REG_DWORD, &key, 4);

                  hwndControl = GetDlgItem(hwndDlg, IDC_JOYCUSTOM2);
                  key = GetWindowLongPtr(hwndControl, GWLP_USERDATA);
                  SetRegValue("Player", "JoyCustom2Key", REG_DWORD, &key, 4);

                  hwndControl = GetDlgItem(hwndDlg, IDC_JOYCUSTOM3);
                  key = GetWindowLongPtr(hwndControl, GWLP_USERDATA);
                  SetRegValue("Player", "JoyCustom3Key", REG_DWORD, &key, 4);

                  hwndControl = GetDlgItem(hwndDlg, IDC_JOYCUSTOM4);
                  key = GetWindowLongPtr(hwndControl, GWLP_USERDATA);
                  SetRegValue("Player", "JoyCustom4Key", REG_DWORD, &key, 4);

                  hwndControl = GetDlgItem(hwndDlg, IDC_ENABLE_MOUSE_PLAYER);
                  selected = SendMessage(hwndControl, BM_GETCHECK, 0, 0);
                  SetRegValue("Player", "EnableMouseInPlayer", REG_DWORD, &selected, 4);

                  hwndControl = GetDlgItem(hwndDlg, IDC_ENABLE_NUDGE_FILTER);
                  selected = SendMessage(hwndControl, BM_GETCHECK, 0, 0);
                  SetRegValue("Player", "EnableNudgeFilter", REG_DWORD, &selected, 4);

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

INT_PTR CALLBACK AudioOptionsProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
            fmusic = 1;

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
            fmusic = 1;

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
            fmusic = 100;
         hwndControl = GetDlgItem(hwndDlg, IDC_MUSIC_SLIDER);
         ::SendMessage(hwndControl, TBM_SETRANGE, fTrue, MAKELONG(0, 100));
         ::SendMessage(hwndControl, TBM_SETTICFREQ, 10, 0);
         ::SendMessage(hwndControl, TBM_SETLINESIZE, 0, 1);
         ::SendMessage(hwndControl, TBM_SETPAGESIZE, 0, 10);
         ::SendMessage(hwndControl, TBM_SETTHUMBLENGTH, 10, 0);
         ::SendMessage(hwndControl, TBM_SETPOS, TRUE, fmusic);

         hr = GetRegInt("Player", "SoundVolume", &fmusic);
         if (hr != S_OK)
            fmusic = 100;
         hwndControl = GetDlgItem(hwndDlg, IDC_SOUND_SLIDER);
         ::SendMessage(hwndControl, TBM_SETRANGE, fTrue, MAKELONG(0, 100));
         ::SendMessage(hwndControl, TBM_SETTICFREQ, 10, 0);
         ::SendMessage(hwndControl, TBM_SETLINESIZE, 0, 1);
         ::SendMessage(hwndControl, TBM_SETPAGESIZE, 0, 10);
         ::SendMessage(hwndControl, TBM_SETTHUMBLENGTH, 10, 0);
         ::SendMessage(hwndControl, TBM_SETPOS, TRUE, fmusic);

         int sd, sdbg;
         hr = GetRegInt("Player", "SoundDevice", &sd);
         if (hr != S_OK)
            sd = 0;
		  hr = GetRegInt("Player", "SoundDeviceBG", &sdbg);
         if (hr != S_OK)
         {
            sdbg = 0; // The default
         }
         SendMessage(hwndDlg, GET_SOUNDDEVICES, sd, sdbg);

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
                  HWND hwndControl;
				  size_t checked;
                  int fmusic;
				  size_t volume;

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

                  HWND hwndSoundList = GetDlgItem(hwndDlg, IDC_SoundList);
				  size_t soundindex = SendMessage(hwndSoundList, LB_GETCURSEL, 0, 0);
				  size_t sd = SendMessage(hwndSoundList, LB_GETITEMDATA, soundindex, 0);
                  SetRegValue("Player", "SoundDevice", REG_DWORD, &sd, 4);
				  hwndSoundList = GetDlgItem(hwndDlg, IDC_SoundListBG);
                  soundindex = SendMessage(hwndSoundList, LB_GETCURSEL, 0, 0);
                  sd = SendMessage(hwndSoundList, LB_GETITEMDATA, soundindex, 0);
                  SetRegValue("Player", "SoundDeviceBG", REG_DWORD, &sd, 4);

                  EndDialog(hwndDlg, TRUE);
               }
               break;

            case IDCANCEL:
               EndDialog(hwndDlg, FALSE);
               break;

            case IDC_PLAY_MUSIC:
               {
                  const size_t checked = SendDlgItemMessage(hwndDlg, IDC_PLAY_MUSIC, BM_GETCHECK, 0, 0);
                  HWND hwndSlider = GetDlgItem(hwndDlg, IDC_MUSIC_SLIDER);
                  HWND hwndText = GetDlgItem(hwndDlg, IDC_STATIC_MUSIC);

                  EnableWindow(hwndSlider, (checked == BST_CHECKED));
                  EnableWindow(hwndText, (checked == BST_CHECKED));
               }
               break;

            case IDC_PLAY_SOUND:
               {
				  const size_t checked = SendDlgItemMessage(hwndDlg, IDC_PLAY_SOUND, BM_GETCHECK, 0, 0);
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

   case GET_SOUNDDEVICES:
      {
         SendMessage(hwndDlg, RESET_SoundList_CONTENT, 0, 0);
         HWND hwndList = GetDlgItem(hwndDlg, IDC_SoundList);
		 HWND hwndListBG = GetDlgItem(hwndDlg, IDC_SoundListBG);


         DSAudioDevices DSads;
         if (!FAILED (DirectSoundEnumerate (DSEnumCallBack, &DSads)))
         {
            for (size_t i=0;i<DSads.size();i++)
            {
			   const size_t index = SendMessage(hwndList, LB_ADDSTRING, 0, (size_t)DSads[i]->description.c_str());
               SendMessage(hwndList, LB_SETITEMDATA, index, (LPARAM)i);
			   const size_t indexbg = SendMessage(hwndListBG, LB_ADDSTRING, 0, (size_t)DSads[i]->description.c_str());
			   SendMessage(hwndListBG, LB_SETITEMDATA, index, (LPARAM)i);
               delete DSads[i];
            }
         }

         SendMessage(hwndList, LB_SETCURSEL, (wParam < DSads.size()) ? wParam : 0, 0);
		 SendMessage(hwndListBG, LB_SETCURSEL, (wParam < DSads.size()) ? lParam : 0, 0);

      }
      break;

   case RESET_SoundList_CONTENT:
      {
         HWND hwndList = GetDlgItem(hwndDlg, IDC_SoundList);
		 HWND hwndListBG = GetDlgItem(hwndDlg, IDC_SoundListBG);
         SendMessage(hwndList, LB_RESETCONTENT, 0, 0);
         SendMessage(hwndListBG, LB_RESETCONTENT, 0, 0);
      }
      break;

   case WM_CLOSE:
      EndDialog(hwndDlg, FALSE);
      break;

   case WM_DESTROY:
      SendMessage(hwndDlg, RESET_SoundList_CONTENT, 0, 0);
      break;
   }

   return FALSE;
}

const unsigned int num_physicsoptions = 8;
static char * physicsoptions[num_physicsoptions] = {NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
static unsigned int physicsselection = 0;

void savecurrentphysicssetting(HWND hwndDlg)
{
    char tmp[256];
    char tmp2[256];
	
	GetDlgItemTextA(hwndDlg, DISPID_Flipper_Speed, tmp, 256);
    sprintf_s(tmp2,256,"FlipperPhysicsMass%u",physicsselection);
	SetRegValue("Player", tmp2, REG_SZ, tmp, lstrlen(tmp));	

	GetDlgItemTextA(hwndDlg, 19, tmp, 256);
    sprintf_s(tmp2,256,"FlipperPhysicsStrength%u",physicsselection);
	SetRegValue("Player", tmp2, REG_SZ, tmp, lstrlen(tmp));

	GetDlgItemTextA(hwndDlg, 21, tmp, 256);
    sprintf_s(tmp2,256,"FlipperPhysicsElasticity%u",physicsselection);
	SetRegValue("Player", tmp2, REG_SZ, tmp, lstrlen(tmp));

	GetDlgItemTextA(hwndDlg, 112, tmp, 256);
    sprintf_s(tmp2,256,"FlipperPhysicsScatter%u",physicsselection);
	SetRegValue("Player", tmp2, REG_SZ, tmp, lstrlen(tmp));

	GetDlgItemTextA(hwndDlg, 23, tmp, 256);
    sprintf_s(tmp2,256,"FlipperPhysicsReturnStrength%u",physicsselection);
	SetRegValue("Player", tmp2, REG_SZ, tmp, lstrlen(tmp));

	GetDlgItemTextA(hwndDlg, 22, tmp, 256);
    sprintf_s(tmp2,256,"FlipperPhysicsElasticityFalloff%u",physicsselection);
	SetRegValue("Player", tmp2, REG_SZ, tmp, lstrlen(tmp));

	GetDlgItemTextA(hwndDlg, 109, tmp, 256);
    sprintf_s(tmp2,256,"FlipperPhysicsFriction%u",physicsselection);
	SetRegValue("Player", tmp2, REG_SZ, tmp, lstrlen(tmp));

	GetDlgItemTextA(hwndDlg, 110, tmp, 256);
    sprintf_s(tmp2,256,"FlipperPhysicsCoilRampUp%u",physicsselection);
	SetRegValue("Player", tmp2, REG_SZ, tmp, lstrlen(tmp));


	GetDlgItemTextA(hwndDlg, 1100, tmp, 256);
    sprintf_s(tmp2,256,"TablePhysicsGravityConstant%u",physicsselection);
	SetRegValue("Player", tmp2, REG_SZ, tmp, lstrlen(tmp));

	GetDlgItemTextA(hwndDlg, 1101, tmp, 256);
    sprintf_s(tmp2,256,"TablePhysicsContactFriction%u",physicsselection);
	SetRegValue("Player", tmp2, REG_SZ, tmp, lstrlen(tmp));

	GetDlgItemTextA(hwndDlg, 1102, tmp, 256);
    sprintf_s(tmp2,256,"TablePhysicsContactScatterAngle%u",physicsselection);
	SetRegValue("Player", tmp2, REG_SZ, tmp, lstrlen(tmp));

	GetDlgItemTextA(hwndDlg, 1110, tmp, 256);
    sprintf_s(tmp2,256,"PhysicsSetName%u",physicsselection);
	SetRegValue("Player", tmp2, REG_SZ, tmp, lstrlen(tmp));
}

INT_PTR CALLBACK PhysicsOptionsProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
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

		 //

		 char tmp[256];

         HWND hwndList = GetDlgItem(hwndDlg, IDC_PhysicsList);

		 const size_t size = SendMessage(hwndList, LB_GETCOUNT, 0, 0);
		 for (size_t i = 0; i<size; i++)
         {
			if(physicsoptions[i])
				delete [] physicsoptions[i];
            int* sd = (int *)SendMessage(hwndList, LB_GETITEMDATA, i, 0);
            delete sd;
         }
         SendMessage(hwndList, LB_RESETCONTENT, 0, 0);

		 for (unsigned int i=0;i<num_physicsoptions;i++)
         {
			physicsoptions[i] = new char[256];
			sprintf_s(tmp,256,"PhysicsSetName%u",i);
			if(GetRegString("Player", tmp, physicsoptions[i], 256) != S_OK)
				sprintf_s(physicsoptions[i],256,"Set %u",i+1);
			sprintf_s(tmp,256,"%u: %s",i+1,physicsoptions[i]);
			const size_t index = SendMessage(hwndList, LB_ADDSTRING, 0, (size_t)tmp);
            int * const sd = new int;
            *sd = i;
            SendMessage(hwndList, LB_SETITEMDATA, index, (LPARAM)sd);
         }
         SendMessage(hwndList, LB_SETCURSEL, physicsselection, 0);

		 //

         HRESULT hr;

		 float FlipperPhysicsMass = 0.15f;
		 sprintf_s(tmp,256,"FlipperPhysicsMass%u",physicsselection);
         hr = GetRegStringAsFloat("Player", tmp, &FlipperPhysicsMass);
         if (hr != S_OK)
            FlipperPhysicsMass = 0.15f;

		 sprintf_s(tmp,256,"%f",FlipperPhysicsMass);
 		 SetDlgItemTextA(hwndDlg, DISPID_Flipper_Speed, tmp);

		 float FlipperPhysicsStrength = 3.f;
		 sprintf_s(tmp,256,"FlipperPhysicsStrength%u",physicsselection);
         hr = GetRegStringAsFloat("Player", tmp, &FlipperPhysicsStrength);
         if (hr != S_OK)
            FlipperPhysicsStrength = 3.f;

		 sprintf_s(tmp,256,"%f",FlipperPhysicsStrength);
 		 SetDlgItemTextA(hwndDlg, 19, tmp);

 		 float FlipperPhysicsElasticity = 0.55f;
		 sprintf_s(tmp,256,"FlipperPhysicsElasticity%u",physicsselection);
         hr = GetRegStringAsFloat("Player", tmp, &FlipperPhysicsElasticity);
         if (hr != S_OK)
            FlipperPhysicsElasticity = 0.55f;

		 sprintf_s(tmp,256,"%f",FlipperPhysicsElasticity);
 		 SetDlgItemTextA(hwndDlg, 21, tmp);

  		 float FlipperPhysicsScatter = -11.f;
		 sprintf_s(tmp,256,"FlipperPhysicsScatter%u",physicsselection);
         hr = GetRegStringAsFloat("Player", tmp, &FlipperPhysicsScatter);
         if (hr != S_OK)
            FlipperPhysicsScatter = -11.f;

		 sprintf_s(tmp,256,"%f",FlipperPhysicsScatter);
 		 SetDlgItemTextA(hwndDlg, 112, tmp);

  		 float FlipperPhysicsReturnStrength = 0.09f;
		 sprintf_s(tmp,256,"FlipperPhysicsReturnStrength%u",physicsselection);
         hr = GetRegStringAsFloat("Player", tmp, &FlipperPhysicsReturnStrength);
         if (hr != S_OK)
            FlipperPhysicsReturnStrength = 0.09f;

		 sprintf_s(tmp,256,"%f",FlipperPhysicsReturnStrength);
 		 SetDlgItemTextA(hwndDlg, 23, tmp);

		 float FlipperPhysicsElasticityFalloff = 0.43f;
		 sprintf_s(tmp,256,"FlipperPhysicsElasticityFalloff%u",physicsselection);
         hr = GetRegStringAsFloat("Player", tmp, &FlipperPhysicsElasticityFalloff);
         if (hr != S_OK)
            FlipperPhysicsElasticityFalloff = 0.43f;

		 sprintf_s(tmp,256,"%f",FlipperPhysicsElasticityFalloff);
 		 SetDlgItemTextA(hwndDlg, 22, tmp);

  		 float FlipperPhysicsFriction = 0.8f;
		 sprintf_s(tmp,256,"FlipperPhysicsFriction%u",physicsselection);
         hr = GetRegStringAsFloat("Player", tmp, &FlipperPhysicsFriction);
         if (hr != S_OK)
            FlipperPhysicsFriction = 0.8f;

		 sprintf_s(tmp,256,"%f",FlipperPhysicsFriction);
 		 SetDlgItemTextA(hwndDlg, 109, tmp);

		 float FlipperPhysicsCoilRampUp = 0.f;
		 sprintf_s(tmp,256,"FlipperPhysicsCoilRampUp%u",physicsselection);
         hr = GetRegStringAsFloat("Player", tmp, &FlipperPhysicsCoilRampUp);
         if (hr != S_OK)
            FlipperPhysicsCoilRampUp = 0.f;

		 sprintf_s(tmp,256,"%f",FlipperPhysicsCoilRampUp);
 		 SetDlgItemTextA(hwndDlg, 110, tmp);

		 float TablePhysicsGravityConstant = DEFAULT_TABLE_GRAVITY;
		 sprintf_s(tmp,256,"TablePhysicsGravityConstant%u",physicsselection);
         hr = GetRegStringAsFloat("Player", tmp, &TablePhysicsGravityConstant);
         if (hr != S_OK)
            TablePhysicsGravityConstant = DEFAULT_TABLE_GRAVITY;

		 sprintf_s(tmp,256,"%f",TablePhysicsGravityConstant);
 		 SetDlgItemTextA(hwndDlg, 1100, tmp);

		 float TablePhysicsContactFriction = DEFAULT_TABLE_CONTACTFRICTION;
		 sprintf_s(tmp,256,"TablePhysicsContactFriction%u",physicsselection);
         hr = GetRegStringAsFloat("Player", tmp, &TablePhysicsContactFriction);
         if (hr != S_OK)
            TablePhysicsContactFriction = DEFAULT_TABLE_CONTACTFRICTION;

		 sprintf_s(tmp,256,"%f",TablePhysicsContactFriction);
 		 SetDlgItemTextA(hwndDlg, 1101, tmp);

		 float TablePhysicsContactScatterAngle = DEFAULT_TABLE_SCATTERANGLE;
		 sprintf_s(tmp,256,"TablePhysicsContactScatterAngle%u",physicsselection);
         hr = GetRegStringAsFloat("Player", tmp, &TablePhysicsContactScatterAngle);
         if (hr != S_OK)
            TablePhysicsContactScatterAngle = DEFAULT_TABLE_SCATTERANGLE;

		 sprintf_s(tmp,256,"%f",TablePhysicsContactScatterAngle);
 		 SetDlgItemTextA(hwndDlg, 1102, tmp);

 		 SetDlgItemTextA(hwndDlg, 1110, physicsoptions[physicsselection]);

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
				  savecurrentphysicssetting(hwndDlg);

				  EndDialog(hwndDlg, TRUE);
               }
               break;

            case IDCANCEL:
               EndDialog(hwndDlg, FALSE);
               break;
            
			case 1111:
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
			   ofn.lpstrFilter = "Visual Pinball Physics (*.vpp)\0*.vpp\0";
			   ofn.lpstrFile = szFileName;
			   ofn.nMaxFile = _MAX_PATH;
			   ofn.lpstrDefExt = "vpp";
			   ofn.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;

			   const HRESULT hr = GetRegString("RecentDir","LoadDir", szInitialDir, 1024);
			   char szFoo[MAX_PATH];
			   if (hr == S_OK)
			   {
				  ofn.lpstrInitialDir = szInitialDir;
			   }
			   else
			   {
				  lstrcpy(szFoo, "c:\\");
				  ofn.lpstrInitialDir = szFoo;
			   }

			   const int ret = GetOpenFileName(&ofn);
			   if(ret == 0)
				  break;

			   FILE *f;
			   fopen_s(&f,ofn.lpstrFile,"r");
			   if( !f )
				   break;

			   char tmp[256];
			   char tmp2[256];

			   float FlipperPhysicsMass,FlipperPhysicsStrength,FlipperPhysicsElasticity,FlipperPhysicsScatter,FlipperPhysicsReturnStrength,FlipperPhysicsElasticityFalloff,FlipperPhysicsFriction,FlipperPhysicsCoilRampUp;
			   fscanf_s(f,"%f %f %f %f %f %f %f %f\n", &FlipperPhysicsMass,&FlipperPhysicsStrength,&FlipperPhysicsElasticity,&FlipperPhysicsScatter,&FlipperPhysicsReturnStrength,&FlipperPhysicsElasticityFalloff,&FlipperPhysicsFriction,&FlipperPhysicsCoilRampUp);
			   float TablePhysicsGravityConstant,TablePhysicsContactFriction,TablePhysicsContactScatterAngle;
			   fscanf_s(f,"%f %f %f\n", &TablePhysicsGravityConstant,&TablePhysicsContactFriction,&TablePhysicsContactScatterAngle);
			   fscanf_s(f,"%s",tmp2);
			   fclose(f);

   				 sprintf_s(tmp,256,"%f",FlipperPhysicsMass);
 				 SetDlgItemTextA(hwndDlg, DISPID_Flipper_Speed, tmp);

				 sprintf_s(tmp,256,"%f",FlipperPhysicsStrength);
 				 SetDlgItemTextA(hwndDlg, 19, tmp);

				 sprintf_s(tmp,256,"%f",FlipperPhysicsElasticity);
 				 SetDlgItemTextA(hwndDlg, 21, tmp);

				 sprintf_s(tmp,256,"%f",FlipperPhysicsScatter);
 				 SetDlgItemTextA(hwndDlg, 112, tmp);

				 sprintf_s(tmp,256,"%f",FlipperPhysicsReturnStrength);
 				 SetDlgItemTextA(hwndDlg, 23, tmp);

				 sprintf_s(tmp,256,"%f",FlipperPhysicsElasticityFalloff);
 				 SetDlgItemTextA(hwndDlg, 22, tmp);

				 sprintf_s(tmp,256,"%f",FlipperPhysicsFriction);
 				 SetDlgItemTextA(hwndDlg, 109, tmp);

				 sprintf_s(tmp,256,"%f",FlipperPhysicsCoilRampUp);
 				 SetDlgItemTextA(hwndDlg, 110, tmp);

				 sprintf_s(tmp,256,"%f",TablePhysicsGravityConstant);
 				 SetDlgItemTextA(hwndDlg, 1100, tmp);

				 sprintf_s(tmp,256,"%f",TablePhysicsContactFriction);
 				 SetDlgItemTextA(hwndDlg, 1101, tmp);

				 sprintf_s(tmp,256,"%f",TablePhysicsContactScatterAngle);
 				 SetDlgItemTextA(hwndDlg, 1102, tmp);

 				 SetDlgItemTextA(hwndDlg, 1110, tmp2);
			   }
			   break;

			case 1112:
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
			   ofn.lpstrFilter = "Visual Pinball Physics (*.vpp)\0*.vpp\0";
			   ofn.lpstrFile = szFileName;
			   ofn.nMaxFile = _MAX_PATH;
			   ofn.lpstrDefExt = "vpp";
			   ofn.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;

			   const HRESULT hr = GetRegString("RecentDir","LoadDir", szInitialDir, 1024);
			   char szFoo[MAX_PATH];
			   if (hr == S_OK)
			   {
				  ofn.lpstrInitialDir = szInitialDir;
			   }
			   else
			   {
				  lstrcpy(szFoo, "c:\\");
				  ofn.lpstrInitialDir = szFoo;
			   }

			   const int ret = GetSaveFileName(&ofn);
			   if(ret == 0)
				  break;

			   FILE *f;
			   fopen_s(&f,ofn.lpstrFile,"w");
			   if( !f )
				   break;

				char tmp[256];
	
				GetDlgItemTextA(hwndDlg, DISPID_Flipper_Speed, tmp, 256);
				fprintf_s(f,"%s ",tmp);

				GetDlgItemTextA(hwndDlg, 19, tmp, 256);
				fprintf_s(f,"%s ",tmp);

				GetDlgItemTextA(hwndDlg, 21, tmp, 256);
				fprintf_s(f,"%s ",tmp);

				GetDlgItemTextA(hwndDlg, 112, tmp, 256);
				fprintf_s(f,"%s ",tmp);

				GetDlgItemTextA(hwndDlg, 23, tmp, 256);
				fprintf_s(f,"%s ",tmp);

				GetDlgItemTextA(hwndDlg, 22, tmp, 256);
				fprintf_s(f,"%s ",tmp);

				GetDlgItemTextA(hwndDlg, 109, tmp, 256);
				fprintf_s(f,"%s ",tmp);

				GetDlgItemTextA(hwndDlg, 110, tmp, 256);
				fprintf_s(f,"%s\n",tmp);


				GetDlgItemTextA(hwndDlg, 1100, tmp, 256);
				fprintf_s(f,"%s ",tmp);

				GetDlgItemTextA(hwndDlg, 1101, tmp, 256);
				fprintf_s(f,"%s ",tmp);

				GetDlgItemTextA(hwndDlg, 1102, tmp, 256);
				fprintf_s(f,"%s ",tmp);

				GetDlgItemTextA(hwndDlg, 1103, tmp, 256);
				fprintf_s(f,"%s ",tmp);

				GetDlgItemTextA(hwndDlg, 1106, tmp, 256);
				fprintf_s(f,"%s\n",tmp);


				GetDlgItemTextA(hwndDlg, 1110, tmp, 256);
				fprintf_s(f,"%s",tmp);

			   fclose(f);
			   }
			   break;
			}
			break;

		 case LBN_SELCHANGE:
			{
				HWND hwndList = GetDlgItem(hwndDlg, IDC_PhysicsList);

				const size_t tmp = SendMessage(hwndList, LB_GETCURSEL, 0, 0);

				if(tmp != physicsselection)
				{
					int result = MessageBox(NULL,"Save","Save current physics set?",MB_YESNOCANCEL | MB_ICONQUESTION);
					if(result == IDYES)
					    savecurrentphysicssetting(hwndDlg);
					if(result != IDCANCEL)
					{
						physicsselection = tmp;
						SendMessage(hwndDlg, WM_INITDIALOG, 0, 0); // reinit all boxes
					}
					else
						SendMessage(hwndList, LB_SETCURSEL, physicsselection, 0);
				}
			}
		 break;
         }
      }
      break;

   case WM_CLOSE:
      EndDialog(hwndDlg, FALSE);
      break;

   case WM_DESTROY:
	  {
         HWND hwndList = GetDlgItem(hwndDlg, IDC_PhysicsList);
		 const size_t size = SendMessage(hwndList, LB_GETCOUNT, 0, 0);
		 for (size_t i = 0; i<size; i++)
         {
			if(physicsoptions[i])
				delete [] physicsoptions[i];
            int* sd = (int *)SendMessage(hwndList, LB_GETITEMDATA, i, 0);
            delete sd;
         }
         SendMessage(hwndList, LB_RESETCONTENT, 0, 0);
	  }
      break;
   }

   return FALSE;
}

INT_PTR CALLBACK EditorOptionsProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
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

         HWND hwndControl;

		 // drag points
         int fdrawpoints = GetRegIntWithDefault("Editor", "ShowDragPoints", 0);
         hwndControl = GetDlgItem(hwndDlg, IDC_DRAW_DRAGPOINTS);
         SendMessage(hwndControl, BM_SETCHECK, fdrawpoints ? BST_CHECKED : BST_UNCHECKED, 0);

         // light centers
         int fdrawcenters = GetRegIntWithDefault("Editor", "DrawLightCenters", 0);
         hwndControl = GetDlgItem(hwndDlg, IDC_DRAW_LIGHTCENTERS);
         SendMessage(hwndControl, BM_SETCHECK, fdrawcenters ? BST_CHECKED : BST_UNCHECKED, 0);

         int fautosave = GetRegIntWithDefault("Editor", "AutoSaveOn", 1);
         SendDlgItemMessage(hwndDlg, IDC_AUTOSAVE, BM_SETCHECK, fautosave ? BST_CHECKED : BST_UNCHECKED, 0);

         int fautosavetime = GetRegIntWithDefault("Editor", "AutoSaveTime", AUTOSAVE_DEFAULT_TIME);
         SetDlgItemInt(hwndDlg, IDC_AUTOSAVE_MINUTES, fautosavetime, FALSE);

         int gridsize = GetRegIntWithDefault("Editor", "GridSize", 50);
         SetDlgItemInt(hwndDlg, IDC_GRID_SIZE, gridsize, FALSE);
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
                  bool checked;

                  // drag points
                  checked = (SendDlgItemMessage(hwndDlg, IDC_DRAW_DRAGPOINTS, BM_GETCHECK, 0, 0) == BST_CHECKED);
                  SetRegValueBool("Editor", "ShowDragPoints", checked);

                  // light centers
                  checked = (SendDlgItemMessage(hwndDlg, IDC_DRAW_LIGHTCENTERS, BM_GETCHECK, 0, 0) == BST_CHECKED);
                  SetRegValueBool("Editor", "DrawLightCenters", checked);

                  // auto save
                  checked = (SendDlgItemMessage(hwndDlg, IDC_AUTOSAVE_MINUTES, BM_GETCHECK, 0, 0) == BST_CHECKED);
                  SetRegValueBool("Editor", "AutoSaveOn", checked);

                  int autosavetime = GetDlgItemInt(hwndDlg, IDC_AUTOSAVE_MINUTES, NULL, FALSE);
                  SetRegValueInt("Editor", "AutoSaveTime", autosavetime);

                  int gridsize = GetDlgItemInt(hwndDlg, IDC_GRID_SIZE, NULL, FALSE);
                  SetRegValueInt("Editor", "GridSize", gridsize);

                  // Go through and reset the autosave time on all the tables
                  g_pvp->SetAutoSaveMinutes(autosavetime);
                  for (int i=0;i<g_pvp->m_vtable.Size();i++)
                     g_pvp->m_vtable.ElementAt(i)->BeginAutoSaveCounter();
       
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

INT_PTR CALLBACK ProtectTableProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
				  const size_t checked = SendDlgItemMessage(hwndDlg, IDC_PROTECT_TOTALLOCK, BM_GETCHECK, 0, 0);

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

				  const size_t checked = SendDlgItemMessage(hwndDlg, IDD_PROTECT_SHOWPASSWORD, BM_GETCHECK, 0, 0);
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

                  // get the check box status(s)
                  unsigned long flags = 0;
				  const size_t checked1 = SendDlgItemMessage(hwndDlg, IDC_PROTECT_SAVEAS, BM_GETCHECK, 0, 0);
                  if (checked1 == BST_CHECKED) flags |= DISABLE_TABLE_SAVE;
				  const size_t checked2 = SendDlgItemMessage(hwndDlg, IDC_PROTECT_SAVEASPROT, BM_GETCHECK, 0, 0);
                  if (checked2 == BST_CHECKED) flags |= DISABLE_TABLE_SAVEPROT;
				  const size_t checked3 = SendDlgItemMessage(hwndDlg, IDC_PROTECT_SCRIPT, BM_GETCHECK, 0, 0);
                  if (checked3 == BST_CHECKED) flags |= DISABLE_SCRIPT_EDITING;
				  const size_t checked4 = SendDlgItemMessage(hwndDlg, IDC_PROTECT_MANAGERS, BM_GETCHECK, 0, 0);
                  if (checked4 == BST_CHECKED) flags |= DISABLE_OPEN_MANAGERS;
				  const size_t checked5 = SendDlgItemMessage(hwndDlg, IDC_PROTECT_COPY, BM_GETCHECK, 0, 0);
                  if (checked5 == BST_CHECKED) flags |= DISABLE_CUTCOPYPASTE;
				  const size_t checked6 = SendDlgItemMessage(hwndDlg, IDC_PROTECT_VIEWTABLE, BM_GETCHECK, 0, 0);
                  if (checked6 == BST_CHECKED) flags |= DISABLE_TABLEVIEW;
				  const size_t checked7 = SendDlgItemMessage(hwndDlg, IDC_PROTECT_DEBUGGER, BM_GETCHECK, 0, 0);
                  if (checked7 == BST_CHECKED) flags |= DISABLE_DEBUGGER;
				  const size_t checked0 = SendDlgItemMessage(hwndDlg, IDC_PROTECT_TOTALLOCK, BM_GETCHECK, 0, 0);
                  if (checked0 == BST_CHECKED) flags |= DISABLE_EVERYTHING;

                  // get the passwords
                  char pw[PROT_PASSWORD_LENGTH+1];
                  ZeroMemory (pw, sizeof(pw));
                  HWND hwndPw = GetDlgItem(hwndDlg, IDC_PROTECT_PASSWORD);
                  GetWindowText(hwndPw,  pw,  sizeof(pw));

                  char pw2[PROT_PASSWORD_LENGTH+2];
                  ZeroMemory (pw2, sizeof(pw2));
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
                     if ( (pw[0] == '\0') && (pw2[0] == '\0') )
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
                     BOOL rc = pt->SetupProtectionBlock((unsigned char *)pw, flags);
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

INT_PTR CALLBACK UnlockTableProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
                  ZeroMemory (pw, sizeof(pw));
                  HWND hwndPw = GetDlgItem(hwndDlg, IDC_UNLOCK_PASSWORD);
                  GetWindowText(hwndPw,  pw,  sizeof(pw));

                  // if both password is empty bring up a message box
                  if (pw[0] == '\0')
                  {
                     LocalString ls(IDS_PROTECT_PW_ZEROLEN);
                     MessageBox(hwndDlg, ls.m_szbuffer, "Visual Pinball", MB_ICONWARNING);
                  }
                  else
                  {
                     PinTable * const pt = g_pvp->GetActiveTable();
                     if (pt)
                     {
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


INT_PTR CALLBACK SearchSelectProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   //CCO(PinTable) *pt;
   //pt = (CCO(PinTable) *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

   switch (uMsg)
   {
   case WM_INITDIALOG:
      {
         SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);

//          HWND hwndName = GetDlgItem(hwndDlg, IDC_NAME);
//          WideCharToMultiByte(CP_ACP, 0, pcol->m_wzName, -1, szT, MAX_PATH, NULL, NULL);

         char szT[MAX_PATH];
         HWND listBox = GetDlgItem(hwndDlg, IDC_ELEMENT_LIST);
         PinTable *pt = g_pvp->GetActiveTable();

         for (int i=0;i<pt->m_vedit.Size();i++)
         {
            IEditable * const piedit = pt->m_vedit.ElementAt(i);
            IScriptable * const piscript = piedit->GetScriptable();
            if (piscript)
            {
               WideCharToMultiByte(CP_ACP, 0, piscript->m_wzName, -1, szT, MAX_PATH, NULL, NULL);
			   const size_t index = SendMessage(listBox, LB_ADDSTRING, 0, (size_t)szT);
               SendMessage(listBox, LB_SETITEMDATA, index, (size_t)piscript);
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
         case IDOK:
            {
               HWND listBox = GetDlgItem(hwndDlg, IDC_ELEMENT_LIST);
			   const size_t count = SendMessage(listBox, LB_GETSELCOUNT, 0, 0);
               int * const rgsel = new int[count];
               SendMessage(listBox, LB_GETSELITEMS, count, (LPARAM)rgsel);

               PinTable *pt = g_pvp->GetActiveTable();
               pt->ClearMultiSel();
               for (size_t i=0;i<count;i++)
               {
				   const size_t len = SendMessage(listBox, LB_GETTEXTLEN, rgsel[i], 0);
                  char * const szT = new char[len+1]; // include null terminator
                  SendMessage(listBox, LB_GETTEXT, rgsel[i], (LPARAM)szT);
                  IScriptable * const piscript = (IScriptable *)SendMessage(listBox, LB_GETITEMDATA, rgsel[i], 0);
                  ISelect * const pisel = piscript->GetISelect();
                  if (pisel)
                     pt->AddMultiSel(pisel, true);
               }
               delete[] rgsel;
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

static bool drawing_order_select; //!! meh

void VPinball::ShowDrawingOrderDialog(bool select)
{
   drawing_order_select = select; //!! meh
   DialogBoxParam(g_hinst, MAKEINTRESOURCE(IDD_DRAWING_ORDER), m_hwnd, DrawingOrderProc, 0);
}

void UpdateDrawingOrder( HWND hwndDlg, IEditable *ptr, bool up )
{
   HWND hw=GetDlgItem( hwndDlg, IDC_DRAWING_ORDER_LIST);
   PinTable *pt = g_pvp->GetActiveTable();
   char text0[256], text1[256], text2[256]; 
   LVITEM lv;
   lv.mask = LVIF_TEXT;
   int idx = ListView_GetNextItem( hw, -1, LVNI_FOCUSED);
   if( idx==-1 )
      return;

   if ( up )
   {
      if ( idx>0 )
      {
         ListView_GetItemText( hw, idx, 0, text0, 256 );
         ListView_GetItemText( hw, idx, 1, text1, 256 );
         ListView_GetItemText( hw, idx, 2, text2, 256 );
         ListView_DeleteItem( hw, idx );
         lv.iItem = idx-1;
         lv.iSubItem=0;
         lv.pszText = text0;
         ListView_InsertItem( hw, &lv );
         ListView_SetItemText( hw, idx-1, 1, text1 );
         ListView_SetItemText( hw, idx-1, 2, text2 );
         ListView_SetItemState( hw, -1, 0, LVIS_SELECTED);
         ListView_SetItemState( hw, idx-1, LVIS_SELECTED, LVIS_SELECTED);
         ListView_SetItemState( hw, idx-1, LVIS_FOCUSED, LVIS_FOCUSED);
         SetFocus(hw);
         if(drawing_order_select)
         {
			 ISelect *psel = pt->m_vmultisel.ElementAt(idx);
			 pt->m_vmultisel.RemoveElementAt(idx);

			 if ( idx-1<0 )
				pt->m_vmultisel.InsertElementAt(psel,0);
			 else
				pt->m_vmultisel.InsertElementAt(psel, idx-1);

			 for ( int i=pt->m_vmultisel.Size()-1;i>=0;i-- )
			 {
				IEditable *pedit = pt->m_vmultisel.ElementAt(i)->GetIEditable();
				int t=pt->m_vedit.IndexOf(pedit);
				pt->m_vedit.RemoveElementAt(t);
			 }

			 for ( int i=pt->m_vmultisel.Size()-1;i>=0;i-- )
			 {
				IEditable *pedit = pt->m_vmultisel.ElementAt(i)->GetIEditable();
				pt->m_vedit.AddElement(pedit);
			 }
         }
         else
         {
			 ISelect *psel = pt->m_allHitElements.ElementAt(idx);
			 pt->m_allHitElements.RemoveElementAt(idx);

			 if ( idx-1<0 )
				pt->m_allHitElements.InsertElementAt(psel,0);
			 else
				pt->m_allHitElements.InsertElementAt(psel, idx-1);

			 for ( int i=pt->m_allHitElements.Size()-1;i>=0;i-- )
			 {
				IEditable *pedit = pt->m_allHitElements.ElementAt(i)->GetIEditable();
				int t=pt->m_vedit.IndexOf(pedit);
				pt->m_vedit.RemoveElementAt(t);
			 }

			 for ( int i=pt->m_allHitElements.Size()-1;i>=0;i-- )
			 {
				IEditable *pedit = pt->m_allHitElements.ElementAt(i)->GetIEditable();
				pt->m_vedit.AddElement(pedit);
			 }
         }
      }
   }
   else
   {
      if(drawing_order_select)
      {
		  if ( idx<pt->m_vmultisel.Size()-1 )
		  {
           ListView_GetItemText( hw, idx, 0, text0, 256 );
           ListView_GetItemText( hw, idx, 1, text1, 256 );
           ListView_GetItemText( hw, idx, 2, text2, 256 );
           ListView_DeleteItem( hw, idx );
           lv.iItem = idx+1;
           lv.iSubItem=0;
           lv.pszText = text0;
           ListView_InsertItem( hw, &lv );
           ListView_SetItemText( hw, idx+1, 1, text1 );
           ListView_SetItemText( hw, idx+1, 2, text2 );
           ListView_SetItemState( hw, -1, 0, LVIS_SELECTED);
           ListView_SetItemState( hw, idx+1, LVIS_SELECTED, LVIS_SELECTED);
           ListView_SetItemState( hw, idx+1, LVIS_FOCUSED, LVIS_FOCUSED);
           SetFocus(hw);
			 ISelect *psel = pt->m_vmultisel.ElementAt(idx);
			 pt->m_vmultisel.RemoveElementAt(idx);

			 if ( idx+1>=pt->m_vmultisel.Size() )
				pt->m_vmultisel.AddElement(psel);
			 else
				pt->m_vmultisel.InsertElementAt(psel, idx+1);

			 for ( int i=pt->m_vmultisel.Size()-1;i>=0;i-- )
			 {
				IEditable *pedit = pt->m_vmultisel.ElementAt(i)->GetIEditable();
				int t=pt->m_vedit.IndexOf(pedit);
				pt->m_vedit.RemoveElementAt(t);
			 }

			 for ( int i=pt->m_vmultisel.Size()-1;i>=0;i-- )
			 {
				IEditable *pedit = pt->m_vmultisel.ElementAt(i)->GetIEditable();
				pt->m_vedit.AddElement(pedit);
			 }
		  }
	  }
      else
      {
		  if ( idx<pt->m_allHitElements.Size()-1 )
		  {
           ListView_GetItemText( hw, idx, 0, text0, 256 );
           ListView_GetItemText( hw, idx, 1, text1, 256 );
           ListView_GetItemText( hw, idx, 2, text2, 256 );
           ListView_DeleteItem( hw, idx );
           lv.iItem = idx+1;
           lv.iSubItem=0;
           lv.pszText = text0;
           ListView_InsertItem( hw, &lv );
           ListView_SetItemText( hw, idx+1, 1, text1 );
           ListView_SetItemText( hw, idx+1, 2, text2 );
           ListView_SetItemState( hw, -1, 0, LVIS_SELECTED);
           ListView_SetItemState( hw, idx+1, LVIS_SELECTED, LVIS_SELECTED);
           ListView_SetItemState( hw, idx+1, LVIS_FOCUSED, LVIS_FOCUSED);
           SetFocus(hw);

			 ISelect *psel = pt->m_allHitElements.ElementAt(idx);
			 pt->m_allHitElements.RemoveElementAt(idx);

			 if ( idx+1>=pt->m_allHitElements.Size() )
				pt->m_allHitElements.AddElement(psel);
			 else
				pt->m_allHitElements.InsertElementAt(psel, idx+1);

			 for ( int i=pt->m_allHitElements.Size()-1;i>=0;i-- )
			 {
				IEditable *pedit = pt->m_allHitElements.ElementAt(i)->GetIEditable();
				int t=pt->m_vedit.IndexOf(pedit);
				pt->m_vedit.RemoveElementAt(t);
			 }

			 for ( int i=pt->m_allHitElements.Size()-1;i>=0;i-- )
			 {
				IEditable *pedit = pt->m_allHitElements.ElementAt(i)->GetIEditable();
				pt->m_vedit.AddElement(pedit);
			 }
		  }
      }
   }
}


INT_PTR CALLBACK DrawingOrderProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   //CCO(PinTable) *pt;
   //pt = (CCO(PinTable) *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

   switch (uMsg)
   {
   case WM_INITDIALOG:
      {
         HWND listHwnd=GetDlgItem( hwndDlg, IDC_DRAWING_ORDER_LIST);
         PinTable *pt = g_pvp->GetActiveTable();
         LVCOLUMN lvc;
         LVITEM lv;
         
         ListView_SetExtendedListViewStyle( listHwnd, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
         memset( &lvc, 0, sizeof(LVCOLUMN));
         lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_FMT;
         lvc.cx=200;
         lvc.pszText = TEXT("Name");
         ListView_InsertColumn( listHwnd, 0, &lvc );
         lvc.cx=100;
         lvc.pszText = TEXT("Height/Z");
         ListView_InsertColumn( listHwnd, 1, &lvc );
         lvc.cx=100;
         lvc.pszText = TEXT("Type");
         ListView_InsertColumn( listHwnd, 2, &lvc );
         
         if( listHwnd!=NULL )
            ListView_DeleteAllItems( listHwnd );
         lv.mask = LVIF_TEXT;
         char textBuf[256];
         // create a selection in the same drawing order as the selected elements are stored in the main vector
         Vector<ISelect> selection;
         if ( drawing_order_select )
         {
            for( int i=pt->m_vedit.Size()-1;i>=0;i-- )
               for( int t=0;t<pt->m_vmultisel.Size();t++ )
               {
                  if( pt->m_vmultisel.ElementAt(t) == pt->m_vedit.ElementAt(i)->GetISelect() )
                     selection.AddElement(pt->m_vmultisel.ElementAt(t));
               }
         }
         for( int i=0; i<(drawing_order_select ? selection.Size() : pt->m_allHitElements.Size()); i++ )
         {
            IEditable *pedit = drawing_order_select ? selection.ElementAt(i)->GetIEditable() : pt->m_allHitElements.ElementAt(i)->GetIEditable();
            if ( pedit )
            {
               char *szTemp;
               szTemp = pt->GetElementName(pedit);
               if( szTemp )
               {
                  lv.iItem = i;
                  lv.iSubItem=0;
                  lv.pszText = szTemp;
                  ListView_InsertItem( listHwnd, &lv );
                  if ( pedit->GetItemType()==eItemSurface )
                  {
                     Surface *sur = (Surface*)pedit;
                     sprintf_s(textBuf,"%.02f", sur->m_d.m_heighttop);
                     ListView_SetItemText( listHwnd, i, 1, textBuf);
                     ListView_SetItemText( listHwnd, i, 2, "Wall");
                  }
                  else if ( pedit->GetItemType()==eItemPrimitive )
                  {
                     Primitive *prim = (Primitive*)pedit;
                     sprintf_s(textBuf,"%.02f", prim->m_d.m_vPosition.z);
                     ListView_SetItemText( listHwnd, i, 1, textBuf);
                     ListView_SetItemText( listHwnd, i, 2, "Primitive");
                  }
                  else if ( pedit->GetItemType()==eItemRamp )
                  {
                     Ramp *ramp= (Ramp*)pedit;
                     sprintf_s(textBuf,"%.02f", ramp->m_d.m_heighttop);
                     ListView_SetItemText( listHwnd, i, 1, textBuf);
                     ListView_SetItemText( listHwnd, i, 2, "Ramp");
                  }
                  else if ( pedit->GetItemType()==eItemFlasher )
                  {
                     Flasher *flasher= (Flasher*)pedit;
                     sprintf_s(textBuf,"%.02f", flasher->m_d.m_height);
                     ListView_SetItemText( listHwnd, i, 1, textBuf);
                     ListView_SetItemText( listHwnd, i, 2, "Flasher");
                  }
                  else if ( pedit->GetItemType()==eItemRubber )
                  {
                      Rubber *rubber= (Rubber*)pedit;
                      sprintf_s(textBuf,"%.02f", rubber->m_d.m_height);
                      ListView_SetItemText( listHwnd, i, 1, textBuf);
                      ListView_SetItemText( listHwnd, i, 2, "Rubber");
                  }
                  else if ( pedit->GetItemType()==eItemSpinner )
                  {
                     Spinner *spin= (Spinner*)pedit;
                     sprintf_s(textBuf,"%.02f", spin->m_d.m_height);
                     ListView_SetItemText( listHwnd, i, 1, textBuf);
                     ListView_SetItemText( listHwnd, i, 2, "Spinner");
                  }
                  else if ( pedit->GetItemType()==eItemKicker )
                  {
                     Kicker *kick= (Kicker*)pedit;
                     sprintf_s(textBuf,"%.02f", kick->m_d.m_hit_height);
                     ListView_SetItemText( listHwnd, i, 1, textBuf);
                     ListView_SetItemText( listHwnd, i, 2, "Kicker");
                  }
                  else if ( pedit->GetItemType()==eItemLight )
                  {
                     Light *light= (Light*)pedit;
                     ListView_SetItemText( listHwnd, i, 1, "n.a.");
                     ListView_SetItemText( listHwnd, i, 2, "Light");
                  }
                  else if ( pedit->GetItemType()==eItemBumper )
                  {
                     Bumper *bump= (Bumper*)pedit;
                     ListView_SetItemText( listHwnd, i, 1, "n.a.");
                     ListView_SetItemText( listHwnd, i, 2, "Bumper");
                  }
                  else if ( pedit->GetItemType()==eItemFlipper )
                  {
                     Flipper *flip= (Flipper*)pedit;
                     sprintf_s(textBuf,"%.02f", flip->m_d.m_height);
                     ListView_SetItemText( listHwnd, i, 1, textBuf);
                     ListView_SetItemText( listHwnd, i, 2, "Flipper");
                  }
                  else if ( pedit->GetItemType()==eItemGate )
                  {
                     Gate *gate= (Gate*)pedit;
                     sprintf_s(textBuf,"%.02f", gate->m_d.m_height);
                     ListView_SetItemText( listHwnd, i, 1, textBuf);
                     ListView_SetItemText( listHwnd, i, 2, "Gate");
                  }
                  else if ( pedit->GetItemType()==eItemPlunger )
                  {
                     Plunger *plung= (Plunger*)pedit;
                     sprintf_s(textBuf,"%.02f", plung->m_d.m_height);
                     ListView_SetItemText( listHwnd, i, 1, textBuf);
                     ListView_SetItemText( listHwnd, i, 2, "Plunger");
                  }
               }
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
         case IDC_DRAWING_ORDER_UP:
            {
               //PinTable *pt = g_pvp->GetActiveTable();
               //for( int i=0;i<pt->m_vmultisel.Size();i++ )
               //{
                  //IEditable *ptr = pt->m_vmultisel.ElementAt(i)->GetIEditable();
                  UpdateDrawingOrder(hwndDlg, NULL, true );
               //}
               break;
            }
         case IDC_DRAWING_ORDER_DOWN:
            {
               //PinTable *pt = g_pvp->GetActiveTable();
               //for( int i=0;i<pt->m_vmultisel.Size();i++ )
               //{
                  //IEditable *ptr = pt->m_vmultisel.ElementAt(i)->GetIEditable();
                  UpdateDrawingOrder(hwndDlg, NULL, false );
               //}
               break;
            }
         case IDOK:
            {
               EndDialog(hwndDlg, TRUE);
               break;
            }
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

struct ManufacturerDimensions
{
   char name[32];
   float width;
   float height;
};

#define DIM_TABLE_SIZE 34
ManufacturerDimensions dimTable[DIM_TABLE_SIZE] =
{
   {"Atari (widebody)", 27.0f, 45.0f },
   {"Bally 70s EM (standard)", 20.25f, 41.0f },
   {"Bally (standard)", 20.25f, 42.0f },
   {"Bally (widebody)", 26.75f, 42.0f },
   {"Capcom", 20.25f, 46.0f },
   {"Data East/Sega (standard)", 20.25f, 46.0f },
   {"Data East/Sega (widebody)", 23.25f, 46.0f },
   {"Game Plan", 20.25f, 42.0f },
   {"Gottlieb 70s EM (standard)", 20.25f, 41.0f },
   {"Gottlieb System 1 (standard)", 20.25f ,42.0f },
   {"Gottlieb System 3", 20.25f, 46.0f },
   {"Gottlieb System 80 (standard)", 20.25f, 42.0f },
   {"Gottlieb System 80 (widebody)", 23.75f, 46.5f },
   {"Gottlieb System 80 (extrawide)", 26.75f, 46.5f },
   {"Stern (widebody)", 23.875f, 45.0f },
   {"Stern (standard)", 23.25f, 42.0f },
   {"Stern Modern (standard)", 20.25f, 45.0f },
   {"WMS Pinball 2000", 20.5f, 43.0f },
   {"WMS System 1-11 (standard)", 20.25f, 42.0f },
   {"WMS System 1-11 (widebody)", 27.0f, 42.0f },
   {"WPC (through 1987)", 20.5f, 42.0f },
   {"WPC (1987 on)", 20.5f, 46.0f },
   {"WPC (superpin)", 23.25f ,46.0f },
   {"Zaccaria (standard)", 20.25f, 42.0f },
   {"Black Knight 2000 (1991)", 20.25f ,46.0f },
   {"Bride Of Pinbot (1991)", 20.25f ,45.25f },
   {"BSD Dracula (1993)", 20.25f ,45.0f },
   {"Doctor Who (1992)", 20.25f ,45.0625f },
   {"Genie (1979)", 16.75f ,47.0f },
   {"Hercules (1979 Atari)", 36.0f ,72.0f },
   {"Mystery Castle (Alvin G)", 20.25f ,46.0f },
   {"Safecracker", 16.5f ,41.5f },
   {"Varkon (1982)", 24.0f ,21.0f },
   {"World Cup Soccer (1994)", 20.25f ,45.75f }
};

INT_PTR CALLBACK DimensionProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   switch (uMsg)
   {
      case WM_INITDIALOG:
      {
         HWND listHwnd=GetDlgItem( hwndDlg, IDC_TABLE_DIM_LIST);
         PinTable *pt = g_pvp->GetActiveTable();
         LVCOLUMN lvc;
         LVITEM lv;

         ListView_SetExtendedListViewStyle( listHwnd, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
         memset( &lvc, 0, sizeof(LVCOLUMN));
         lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_FMT;
         lvc.cx=150;
         lvc.pszText = TEXT("Manufacturer");
         ListView_InsertColumn( listHwnd, 0, &lvc );
         lvc.cx=60;
         lvc.pszText = TEXT("Width");
         ListView_InsertColumn( listHwnd, 1, &lvc );
         lvc.cx=60;
         lvc.pszText = TEXT("Height");
         ListView_InsertColumn( listHwnd, 2, &lvc );

         if( listHwnd!=NULL )
            ListView_DeleteAllItems( listHwnd );
         lv.mask = LVIF_TEXT;
         char textBuf[32];
         for ( int i=0;i<DIM_TABLE_SIZE; i++ )
         {
            lv.iItem = i;
            lv.iSubItem = 0;
            lv.pszText = dimTable[i].name;
            ListView_InsertItem( listHwnd, &lv );
            sprintf_s(textBuf, "%.03f", dimTable[i].width );
            ListView_SetItemText( listHwnd, i, 1, textBuf);
            sprintf_s(textBuf, "%.03f", dimTable[i].height );
            ListView_SetItemText( listHwnd, i, 2, textBuf);
         }
         return TRUE;
      }
      break;

   case WM_CLOSE:
      EndDialog(hwndDlg, FALSE);
      break;
   case WM_NOTIFY:
      {
         HWND listhw=GetDlgItem( hwndDlg, IDC_DRAWING_ORDER_LIST);
         LPNMHDR pnmhdr = (LPNMHDR)lParam;
         switch (pnmhdr->code)
         {
            case LVN_ITEMCHANGED:
            {
               NMLISTVIEW * const plistview = (LPNMLISTVIEW)lParam;
               int idx = plistview->iItem;
               if ( idx>DIM_TABLE_SIZE || idx<0 )
                  break;

               int width = (int)floor(dimTable[idx].width*47.0f+0.5f);
               int height = (int)floor(dimTable[idx].height*47.0f+0.5f);
               char textBuf[32];
               sprintf_s(textBuf,"%i",width);
               SetDlgItemText(hwndDlg, IDC_VP_WIDTH, textBuf);
               sprintf_s(textBuf,"%i",height);
               SetDlgItemText(hwndDlg, IDC_VP_HEIGHT, textBuf);
               sprintf_s(textBuf,"%.03f",dimTable[idx].width);
               SetDlgItemText(hwndDlg, IDC_SIZE_WIDTH, textBuf);
               sprintf_s(textBuf,"%.03f",dimTable[idx].height);
               SetDlgItemText(hwndDlg, IDC_SIZE_HEIGHT, textBuf);
               break;
            }
         }
      }
   case WM_COMMAND:
      switch (HIWORD(wParam))
      {
         case EN_KILLFOCUS:
         {
            float sizeWidth, sizeHeight;
            int vpWidth, vpHeight;
            int ret=0;
            if( LOWORD(wParam)==IDC_SIZE_WIDTH )              
            {
               char textBuf[32];
               GetDlgItemText(hwndDlg, IDC_SIZE_WIDTH, textBuf,31);
               ret = sscanf_s(textBuf,"%f",&sizeWidth);
               if (ret!=1 || sizeWidth<0.0f )
                  sizeWidth=0;
               int width = (int)floor(sizeWidth*47.0f+0.5f);
               sprintf_s(textBuf,"%i",width);
               SetDlgItemText(hwndDlg, IDC_VP_WIDTH, textBuf);
            }
            if( LOWORD(wParam)==IDC_SIZE_HEIGHT )              
            {
               char textBuf[32];
               GetDlgItemText(hwndDlg, IDC_SIZE_HEIGHT, textBuf,31);
               ret = sscanf_s(textBuf,"%f",&sizeHeight);
               if (ret!=1 || sizeHeight<0.0f)
                  sizeHeight=0;
               int height = (int)floor(sizeHeight*47.0f+0.5f);
               sprintf_s(textBuf,"%i",height);
               SetDlgItemText(hwndDlg, IDC_VP_HEIGHT, textBuf);
            }
            if( LOWORD(wParam)==IDC_VP_WIDTH )              
            {
               char textBuf[32];
               GetDlgItemText(hwndDlg, IDC_VP_WIDTH, textBuf,31);
               ret = sscanf_s(textBuf,"%i",&vpWidth);
               if (ret!=1 || vpWidth<0 )
                  vpWidth=0;
               float width = (float)vpWidth/47.0f;
               sprintf_s(textBuf,"%.3f",width);
               SetDlgItemText(hwndDlg, IDC_SIZE_WIDTH, textBuf);
            }
            if( LOWORD(wParam)==IDC_VP_HEIGHT )              
            {
               char textBuf[32];
               GetDlgItemText(hwndDlg, IDC_VP_HEIGHT, textBuf,31);
               ret = sscanf_s(textBuf,"%i",&vpHeight);
               if (ret!=1 || vpHeight<0 )
                  vpHeight=0;
               float height = (float)vpHeight/47.0f;
               sprintf_s(textBuf,"%.03f",height);
               SetDlgItemText(hwndDlg, IDC_SIZE_HEIGHT, textBuf);
            }
            break;
         }
      case BN_CLICKED:
         switch (LOWORD(wParam))
         {
            case IDOK:
            {
               HWND hwndTest = GetFocus();
               HWND okButtonHwnd = GetDlgItem(hwndDlg,IDOK);
               if ( hwndTest==okButtonHwnd )
               {
                  EndDialog(hwndDlg, TRUE);
                  break;
               }
               PostMessage(hwndDlg, WM_NEXTDLGCTL, 0, 0L);
               break;
            }
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
