#pragma once

//#define DISABLE_FORCE_NVIDIA_OPTIMUS // do not enable NVIDIA Optimus cards (on Laptops, etc) by default
//#define NVAPI_DEPTH_READ // forces NVAPI to do copydepth (as officially unspecified in DX9)

//#define TWOSIDED_TRANSPARENCY // transparent hit targets are rendered backsided first, then frontsided

#define USE_D3D9EX // if enabled can use extended API, does not benefit on XP though // currently only allows to enable adaptive vsync again (on win vista/7 only, seems to be broken in win8)

#define COMPRESS_MESHES // uses miniz for compressing the meshes

#define CRASH_HANDLER

//#define _CRTDBG_MAP_ALLOC
#ifdef _CRTDBG_MAP_ALLOC
 #include <crtdbg.h>
#endif

//#define DEBUG_NUDGE // debug new nudge code

//#define DEBUG_NO_SOUND
//#define DEBUG_REFCOUNT_TRIGGER

//#define DEBUG_XXX // helps to detect out-of-bounds access, needs to link dbghelp.lib then
//#define SLINTF    // enable debug console output

#define EDITOR_BG_WIDTH 1000
#define EDITOR_BG_HEIGHT 750

#define BASEDEPTHBIAS 5e-5f

#define THREADS_PAUSE 1000 // msecs/time to wait for threads to finish up

#include "physconst.h"

//

#define NUM_BG_SETS        3  // different backdrop settings: DT,FS,FSS
#define BG_DESKTOP         0
#define BG_FULLSCREEN      1
#define BG_FSS             2

#define MAX_BALL_TRAIL_POS 10 // fake/artistic ball motion trail

#define MAX_REELS          32

#define LIGHTSEQGRIDSCALE  20
#define	LIGHTSEQGRIDWIDTH  EDITOR_BG_WIDTH/LIGHTSEQGRIDSCALE
#define	LIGHTSEQGRIDHEIGHT (2*EDITOR_BG_WIDTH)/LIGHTSEQGRIDSCALE

#define LIGHTSEQQUEUESIZE  100

#define MAX_LIGHT_SOURCES  2
#define MAX_BALL_LIGHT_SOURCES  8

//

#define ADAPT_VSYNC_FACTOR 0.95 // safety factor where vsync is turned off (f.e. drops below 60fps * 0.95 = 57fps)

#define ACCURATETIMERS 1        // if commented out, timers will only be triggered as often as frames are rendered (e.g. they can fall behind)
#define MAX_TIMER_MSEC_INTERVAL 1 // amount of msecs to wait (at least) until same timer can be triggered again (e.g. they can fall behind, if set to > 1, as update cycle is 1000Hz)
#define MAX_TIMERS_MSEC_OVERALL 5 // amount of msecs that all timers combined can take per frame (e.g. they can fall behind, if set to < somelargevalue)

#define FPS 1                   // Enable FPS computation (default 'F11')
#define STEPPING 1              // Enable Physics stepping

#if defined(_DEBUG) && defined(STEPPING)
 #define MOUSEPAUSE 1
#endif

//#define PLAYBACK

//#define LOG

//#define _DEBUGPHYSICS
//#define EVENTIME 1

#define DEBUG_BALL_SPIN

//

#define _WINSOCKAPI_ // workaround some issue where windows.h is included before winsock2.h in some of the various includes

#if !defined(AFX_STDAFX_H__35BEBBA5_0A4C_4321_A65C_AFFE89589F15__INCLUDED_)
#define AFX_STDAFX_H__35BEBBA5_0A4C_4321_A65C_AFFE89589F15__INCLUDED_

#define _SECURE_SCL 0
#define _HAS_ITERATOR_DEBUGGING 0

#define STRICT

#ifndef _WIN32_WINNT
#if defined(_WIN64) && defined(CRASH_HANDLER)
 #define _WIN32_WINNT 0x0501
#elif _MSC_VER >= 1800
 #define _WIN32_WINNT 0x0500
#elif _MSC_VER < 1600
 #define _WIN32_WINNT 0x0400
#else
 #define _WIN32_WINNT 0x0403
#endif
#endif
#define _ATL_APARTMENT_THREADED

#include <atlbase.h>

#if(_WIN32_WINNT < 0x0500) // for Win32++ and old compilers
 #define WM_MENURBUTTONUP                0x0122
 #define WM_UNINITMENUPOPUP              0x0125
#endif

#include "main.h"

#include <wxx_appcore.h>		// Add CCriticalSection, CObject, CWinThread, CWinApp
#include <wxx_archive.h>		// Add CArchive
#include <wxx_commondlg.h>		// Add CCommonDialog, CColorDialog, CFileDialog, CFindReplace, CFontDialog 
#include <wxx_controls.h>		// Add CAnimation, CComboBox, CComboBoxEx, CDateTime, CHeader, CHotKey, CIPAddress, CProgressBar, CSpinButton, CScrollBar, CSlider, CToolTip
#include <wxx_cstring.h>		// Add CString, CStringA, CStringW
#include <wxx_ddx.h>			// Add CDataExchange
#include <wxx_dialog.h>			// Add CDialog, CResizer
#include <wxx_dockframe.h>		// Add CDockFrame, CMDIDockFrame
#include <wxx_docking.h>		// Add CDocker, CDockContainer
#include <wxx_exception.h>		// Add CException, CFileException, CNotSupportedException, CResourceException, CUserException, CWinException
#include <wxx_file.h>			// Add CFile
#include <wxx_frame.h>			// Add CFrame
#include <wxx_gdi.h>			// Add CDC, CGDIObject, CBitmap, CBrush, CFont, CPalatte, CPen, CRgn
#include <wxx_imagelist.h>		// Add CImageList
#include <wxx_listview.h>		// Add CListView
#include <wxx_mdi.h>			// Add CMDIChild, CMDIFrame, CDockMDIFrame
#include <wxx_printdialogs.h>	// Add CPageSetupDialog, CPrintSetupDialog
#include <wxx_propertysheet.h>	// Add CPropertyPage, CPropertySheet
#include <wxx_rebar.h>			// Add CRebar
#include <wxx_regkey.h>			// Add CRegKey
//#include <wxx_ribbon.h>		// Add CRibbon, CRibbonFrame
#include <wxx_richedit.h>		// Add CRichEdit
#include <wxx_shared_ptr.h>		// Add Shared_Ptr
#include <wxx_socket.h>			// Add CSocket
#include <wxx_statusbar.h>		// Add CStatusBar
#include <wxx_stdcontrols.h>	// Add CButton, CEdit, CListBox
#include <wxx_tab.h>			// Add CTab, CTabbedMDI
//#include <wxx_taskdialog.h>	// Add CTaskDialog
#include <wxx_time.h>			// Add CTime
#include <wxx_toolbar.h>		// Add CToolBar
#include <wxx_treeview.h>		// Add CTreeView
#include <wxx_webbrowser.h>		// Add CAXWindow, CWebBrowser
#include "wxx_wincore.h"

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__35BEBBA5_0A4C_4321_A65C_AFFE89589F15__INCLUDED)
