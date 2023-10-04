#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#ifdef ENABLE_SDL
 #include <d3d11_1.h>
 #include <dxgi1_2.h>
 #pragma comment(lib,"d3d11.lib")
 #pragma comment(lib,"dxgi.lib")
#endif

#include <windows.h>
#include <mmsystem.h>

#ifdef USE_DINPUT8
 #define DIRECTINPUT_VERSION 0x0800
#else
 #define DIRECTINPUT_VERSION 0x0700
#endif
#include <dinput.h>

#ifndef ENABLE_SDL
 #ifdef _DEBUG
  #define D3D_DEBUG_INFO
 #endif
 #include "minid3d9.h"
#endif

#include <dsound.h>


//#include <richedit.h>
//#include <atlcom.h>
#include <atlbase.h>
//#include <atlhost.h>
#include <atlctl.h>
//#include <cassert>

//#include <commctrl.h>

//#include <cstdio>
//#include "wchar.h"

#ifdef _MSC_VER
#define PATH_SEPARATOR_CHAR '\\'
#define PATH_SEPARATOR_WCHAR L'\\'
#define PATH_TABLES  ("c:"s + PATH_SEPARATOR_CHAR + "Visual Pinball" + PATH_SEPARATOR_CHAR + "tables"  + PATH_SEPARATOR_CHAR)
#define PATH_SCRIPTS ("c:"s + PATH_SEPARATOR_CHAR + "Visual Pinball" + PATH_SEPARATOR_CHAR + "scripts" + PATH_SEPARATOR_CHAR)
#define PATH_MUSIC   ("c:"s + PATH_SEPARATOR_CHAR + "Visual Pinball" + PATH_SEPARATOR_CHAR + "music"   + PATH_SEPARATOR_CHAR)
#define PATH_USER    ("c:"s + PATH_SEPARATOR_CHAR + "Visual Pinball" + PATH_SEPARATOR_CHAR + "user"    + PATH_SEPARATOR_CHAR)
#else
#define PATH_SEPARATOR_CHAR '/'
#define PATH_SEPARATOR_WCHAR L'/'
#define PATH_TABLES  (string(getenv("HOME")) + PATH_SEPARATOR_CHAR + ".vpinball" + PATH_SEPARATOR_CHAR + "tables"  + PATH_SEPARATOR_CHAR)
#define PATH_SCRIPTS (string(getenv("HOME")) + PATH_SEPARATOR_CHAR + ".vpinball" + PATH_SEPARATOR_CHAR + "scripts" + PATH_SEPARATOR_CHAR)
#define PATH_MUSIC   (string(getenv("HOME")) + PATH_SEPARATOR_CHAR + ".vpinball" + PATH_SEPARATOR_CHAR + "music"   + PATH_SEPARATOR_CHAR)
#define PATH_USER    (string(getenv("HOME")) + PATH_SEPARATOR_CHAR + ".vpinball" + PATH_SEPARATOR_CHAR + "user"    + PATH_SEPARATOR_CHAR)
#endif

#include <oleauto.h>

#include <wincrypt.h>

#include <intrin.h>
#if defined(_M_IX86) || (defined(_M_IX86_FP) && _M_IX86_FP >= 2) || defined(__SSE2__) || defined(_M_X64) || defined(_M_AMD64) || defined(__i386__) || defined(__i386) || defined(__i486__) || defined(__i486) || defined(i386) || defined(__ia64__) || defined(__x86_64__)
 #define ENABLE_SSE_OPTIMIZATIONS
 #include <xmmintrin.h>
#elif (defined(_M_ARM) || defined(_M_ARM64) || defined(__arm__) || defined(__arm64__) || defined(__aarch64__)) && (!defined(__ARM_ARCH) || __ARM_ARCH >= 7) && (!defined(_MSC_VER) || defined(__clang__)) //!! disable sse2neon if MSVC&non-clang
 #define ENABLE_SSE_OPTIMIZATIONS
 #include "sse2neon.h"
#endif

#include <vector>
#include <string>
#include <algorithm>
#include <commdlg.h>

using std::string;
using std::wstring;
using std::vector;

#include <dlgs.h>
#include <cderr.h>
#include <wxx_appcore.h>		// Add CCriticalSection, CObject, CWinThread, CWinApp
//#include <wxx_archive.h>		// Add CArchive
#include <wxx_commondlg.h>		// Add CCommonDialog, CColorDialog, CFileDialog, CFindReplace, CFontDialog 
#include <wxx_scrollview.h>
//#include <wxx_controls.h>		// Add CAnimation, CComboBox, CComboBoxEx, CDateTime, CHeader, CHotKey, CIPAddress, CProgressBar, CSpinButton, CScrollBar, CSlider, CToolTip
//#include <wxx_cstring.h>		// Add CString, CStringA, CStringW
//#include <wxx_ddx.h>			// Add CDataExchange
//#include <wxx_dialog.h>			// Add CDialog, CResizer
//#include <wxx_dockframe.h>		// Add CDockFrame, CMDIDockFrame
//#include <wxx_docking.h>		// Add CDocker, CDockContainer
//#include <wxx_exception.h>		// Add CException, CFileException, CNotSupportedException, CResourceException, CUserException, CWinException
//#include <wxx_file.h>			// Add CFile
//#include <wxx_frame.h>			// Add CFrame
//#include <wxx_gdi.h>			// Add CDC, CGDIObject, CBitmap, CBrush, CFont, CPalatte, CPen, CRgn
//#include <wxx_imagelist.h>		// Add CImageList
#include <wxx_listview.h>		// Add CListView
//#include <wxx_mdi.h>			// Add CMDIChild, CMDIFrame, CDockMDIFrame
//#include <wxx_printdialogs.h>	// Add CPageSetupDialog, CPrintSetupDialog
//#include <wxx_propertysheet.h>	// Add CPropertyPage, CPropertySheet
//#include <wxx_rebar.h>			// Add CRebar
//#include <wxx_regkey.h>			// Add CRegKey
//#include <wxx_ribbon.h>		// Add CRibbon, CRibbonFrame
#include <wxx_richedit.h>		// Add CRichEdit
//#include <wxx_shared_ptr.h>		// Add Shared_Ptr
//#include <wxx_socket.h>			// Add CSocket
//#include <wxx_statusbar.h>		// Add CStatusBar
//#include <wxx_stdcontrols.h>	// Add CButton, CEdit, CListBox
//#include <wxx_tab.h>			// Add CTab, CTabbedMDI
//#include <wxx_taskdialog.h>	// Add CTaskDialog
//#include <wxx_time.h>			// Add CTime
//#include <wxx_toolbar.h>		// Add CToolBar
#include <wxx_treeview.h>		// Add CTreeView
//#include <wxx_webbrowser.h>		// Add CAXWindow, CWebBrowser
#include <wxx_wincore.h>

#define PLOG_OMIT_LOG_DEFINES 
#define PLOG_NO_DBG_OUT_INSTANCE_ID 1
#include <plog/Log.h>

#include "helpers.h"

#include "def.h"

#include "math/math.h"
#include "math/vector.h"
#include "math/matrix.h"
#include "math/bbox.h"

#include "resource.h"

#include "memutil.h"
#include "disputil.h"

#include "dispid.h"

#include "variant.h"
#include "vector.h"
#include "vectorsort.h"
#include "vpinball.h"
#include "regutil.h"

#include "idebug.h"

#include "wintimer.h"

#include "eventproxy.h"

#include "worker.h"

#include "audioplayer.h"
#include "media/fileio.h"
#include "pinundo.h"
#include "iselect.h"

#include "ieditable.h"
#include "codeview.h"

#include "media/lzwreader.h"
#include "media/lzwwriter.h"

#include "media/wavread.h"

#include "pinsound.h"
#include "pinbinary.h"

#include "extern.h"

#include "vpinball_h.h"
#include "pintable.h"

#include "mesh.h"
#include "pin/collide.h"
#include "pin3d.h"

#include "sur.h"
#include "paintsur.h"
#include "hitsur.h"
#include "hitrectsur.h"

#include "ballex.h"

#include "pin/collideex.h"
#include "pin/ball.h"
#include "pin/hittimer.h"
#include "pin/hitable.h"
#include "pin/hitflipper.h"
#include "pin/hitplunger.h"
#include "pin/player.h"

#include "color.h"

#include "dragpoint.h"
#include "timer.h"
#include "flipper.h"
#include "plunger.h"
#include "textbox.h"
#include "surface.h"
#include "dispreel.h"
#include "lightseq.h"
#include "bumper.h"
#include "trigger.h"
#include "light.h"
#include "kicker.h"
#include "decal.h"
#include "primitive.h"
#include "hittarget.h"
#include "gate.h"
#include "spinner.h"
#include "ramp.h"
#include "flasher.h"
#include "rubber.h"
#include "mixer.h"
#include "hid.h"
#include "plumb.h"

#include "kdtree.h"

#include "slintf.h"
#include "trace.h"

inline void ShowError(const char * const sz)
{
   if(g_pvp)
      g_pvp->MessageBox(sz, "Visual Pinball Error", MB_OK | MB_ICONEXCLAMATION);
   else
      MessageBox(nullptr, sz, "Visual Pinball Error", MB_OK | MB_ICONEXCLAMATION);
}

inline void ShowError(const string& sz)
{
   if(g_pvp)
      g_pvp->MessageBox(sz.c_str(), "Visual Pinball Error", MB_OK | MB_ICONEXCLAMATION);
   else
      MessageBox(nullptr, sz.c_str(), "Visual Pinball Error", MB_OK | MB_ICONEXCLAMATION);
}

#include "editablereg.h"

__forceinline float getBGxmult()
{
   const float aaFactor = g_pplayer->m_ptable->m_useAA == -1 ? g_pplayer->m_AAfactor : g_pplayer->m_ptable->m_useAA == 1 ? 2.0f : 1.0f;
   return (float)g_pplayer->m_wnd_width * (float)(1.0 / EDITOR_BG_WIDTH) * aaFactor;
}

__forceinline float getBGymult()
{
   //return getBGxmult() / (((float)g_pplayer->m_screenwidth / (float)g_pplayer->m_screenheight) / (float)((double)EDITOR_BG_WIDTH / EDITOR_BG_HEIGHT));
   return getBGxmult() / (((float)g_pplayer->m_wnd_width / (float)g_pplayer->m_wnd_height) / (float)((double)EDITOR_BG_WIDTH / EDITOR_BG_HEIGHT));
}
