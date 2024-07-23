#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#ifdef ENABLE_SDL
#ifndef __STANDALONE__
 #include <d3d11_1.h>
 #include <dxgi1_2.h>
 #pragma comment(lib,"d3d11.lib")
 #pragma comment(lib,"dxgi.lib")
#endif
#endif

#ifdef __STANDALONE__
#define __null 0
#define ONLY_USE_BASS
#endif

#include <windows.h>
#include <mmsystem.h>

#ifdef __STANDALONE__
#undef PlaySound
#endif

#ifdef USE_DINPUT8
 #define DIRECTINPUT_VERSION 0x0800
#else
 #define DIRECTINPUT_VERSION 0x0700
#endif

#ifdef __STANDALONE__
#define RPC_NO_WINDOWS_H
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
#ifndef __STANDALONE__
#include <atlbase.h>
#else
extern "C" {
   #include <atlbase.h>
}
#endif
//#include <atlhost.h>

#ifndef __STANDALONE__
#include <atlctl.h>
#endif
//#include <cassert>

//#include <commctrl.h>

//#include <cstdio>
//#include "wchar.h"

#ifdef __STANDALONE__
#undef strncpy
#include <wchar.h>
#endif

#ifdef _MSC_VER
#define PATH_SEPARATOR_CHAR '\\'
#define PATH_SEPARATOR_WCHAR L'\\'
#else
#define PATH_SEPARATOR_CHAR '/'
#define PATH_SEPARATOR_WCHAR L'/'
#endif
#define PATH_TABLES  (g_pvp->m_szMyPrefPath + "tables"  + PATH_SEPARATOR_CHAR)
#define PATH_SCRIPTS (g_pvp->m_szMyPrefPath + "scripts" + PATH_SEPARATOR_CHAR)
#define PATH_MUSIC   (g_pvp->m_szMyPrefPath + "music"   + PATH_SEPARATOR_CHAR)
#define PATH_USER    (g_pvp->m_szMyPrefPath + "user"    + PATH_SEPARATOR_CHAR)

#include <oleauto.h>

#include <wincrypt.h>

#ifndef __STANDALONE__
#include <intrin.h>
#endif
#if defined(_M_IX86) || (defined(_M_IX86_FP) && _M_IX86_FP >= 2) || defined(__SSE2__) || defined(_M_X64) || defined(_M_AMD64) || defined(__i386__) || defined(__i386) || defined(__i486__) || defined(__i486) || defined(i386) || defined(__ia64__) || defined(__x86_64__)
 #define ENABLE_SSE_OPTIMIZATIONS
 #include <xmmintrin.h>
#elif (defined(_M_ARM) || defined(_M_ARM64) || defined(__arm__) || defined(__arm64__) || defined(__aarch64__)) && (!defined(__ARM_ARCH) || __ARM_ARCH >= 7) && (!defined(_MSC_VER) || defined(__clang__)) //!! disable sse2neon if MSVC&non-clang
 #define ENABLE_SSE_OPTIMIZATIONS
 #include "sse2neon.h"
#endif

#ifdef __SSSE3__
 #include <tmmintrin.h>
#endif

#include <vector>
#include <string>
#include <algorithm>
#include <iomanip>
#include <commdlg.h>

using namespace std::string_literals;
using std::string;
using std::wstring;
using std::vector;

// try to load the file from the current directory
// if that fails, try the User, Scripts and Tables sub-directorys under where VP was loaded from
// if that also fails, try the standard installation path
static string defaultFileNameSearch[] = { string(), string(), string(), string(), string(), string(), string() };
static const string defaultPathSearch[]     = { string(), "user"s +PATH_SEPARATOR_CHAR, "scripts"s +PATH_SEPARATOR_CHAR, "tables"s +PATH_SEPARATOR_CHAR, string(), string(), string() };

#include <dlgs.h>
#include <cderr.h>

#ifndef __STANDALONE__
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
#endif

#ifdef __STANDALONE__
#define fopen_s(pFile, filename, mode) (((*(pFile)) = fopen((filename), (mode))) == NULL)
#define fprintf_s fprintf
#define fread_s(buffer, bufferSize, elementSize, count, stream) fread(buffer, bufferSize, count, stream)
#define fscanf_s fscanf

#define sscanf_s sscanf

#define localtime_s(x, y) localtime_r(y, x)
#define gmtime_s(x, y) gmtime_r(y, x)

#define _aligned_malloc(size, align) aligned_alloc(align, size)
#define _aligned_free free

#undef lstrlen
#define lstrlen lstrlenA

#undef lstrcmpi
#define lstrcmpi lstrcmpiA

#undef lstrcmp
#define lstrcmp lstrcmpA

#define strcpy_s(A, B, C) strncpy(A, C, B)
#define strnlen_s strnlen
#define sprintf_s snprintf
#define _snprintf_s snprintf
#define swprintf_s swprintf
#define StrStrI strcasestr
#define CString string

#define STRNCPY_S3(a, b, c) strncpy(a, b, c)
#define STRNCPY_S4(a, b, c, d) strncpy(a, c, d)
#define GET_MACRO(_1, _2, _3, _4, NAME, ...) NAME
#define strncpy_s(...) \
  GET_MACRO(__VA_ARGS__, STRNCPY_S4, STRNCPY_S3)(__VA_ARGS__)

#define _T(x) x
#define AtoT(x) x
#define _ASSERTE(expr) ((void)0)

#undef GetCurrentDirectory
#define GetCurrentDirectory GetCurrentDirectoryA

#undef SetCurrentDirectory
#define SetCurrentDirectory SetCurrentDirectoryA

#undef GetModuleFileName
#define GetModuleFileName GetModuleFileNameA

#undef MessageBox
#define MessageBox MessageBoxA

#undef OutputDebugString
#define OutputDebugString OutputDebugStringA

#undef CharLowerBuff
#define CharLowerBuff CharLowerBuffA

#define FINDREPLACE FINDREPLACEA
#define CREATESTRUCT CREATESTRUCTA
#define WNDCLASS WNDCLASSA
#define LOGFONT LOGFONTA
#define MONITORINFOEX MONITORINFOEXA

typedef LPSTR LPTSTR;
typedef LPCSTR LPCTSTR;

class SearchSelectDialog { };
class LayersListDialog { };
class ImageDialog { };
class SoundDialog { };
class AudioOptionsDialog { };
class VideoOptionsDialog { };
class VROptionsDialog { };
class EditorOptionsDialog { };
class CollectionManagerDialog { };
class PhysicsOptionsDialog { };
class TableInfoDialog { };
class DimensionDialog { };
class RenderProbeDialog { };
class MaterialDialog { };
class AboutDialog { };
class ToolbarDialog { };
class NotesDialog { };
class PropertyDialog { };
class ColorButton { };
class SCNotification { };
#endif

#include "Logger.h"

#ifdef __STANDALONE__
#include "standalone/inc/atl/atldef.h"
#include "standalone/inc/atl/atlbase.h"
#include "standalone/inc/atl/atlcom.h"
#include "standalone/inc/atl/atlcomcli.h"
#include "standalone/inc/atl/atlsafe.h"

#include "standalone/inc/atlmfc/afx.h"
#include "standalone/inc/atlmfc/afxdlgs.h"
#include "standalone/inc/atlmfc/afxwin.h"
#include "standalone/inc/atlmfc/atltypes.h"

#include "standalone/inc/win32xx/win32xx.h"
#endif

#include "helpers.h"

#ifdef __STANDALONE__
#include <cstdint>
#endif

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
#ifndef __STANDALONE__
#include "vpinball.h"
#else
#include "standalone/vpinball_standalone_i.h"
#endif
#include "core/Settings.h"

#include "idebug.h"

#include "wintimer.h"

#include "eventproxy.h"

#include "worker.h"

#include "audio/audioplayer.h"
#include "media/fileio.h"
#include "pinundo.h"
#include "iselect.h"

#include "ieditable.h"
#include "codeview.h"

#include "media/lzwreader.h"
#include "media/lzwwriter.h"

#ifndef __STANDALONE__
#include "audio/wavread.h"
#endif

#include "audio/pinsound.h"
#include "pinbinary.h"

#include "extern.h"

#include "vpinball_h.h"
#include "parts/pintable.h"

#include "mesh.h"
#include "physics/collide.h"
#include "pin3d.h"

#include "sur.h"
#include "paintsur.h"
#include "hitsur.h"
#include "hitrectsur.h"

#include "parts/ball.h"

#include "physics/collideex.h"
#include "physics/hitball.h"
#include "physics/hittimer.h"
#include "physics/hitable.h"
#include "physics/hitflipper.h"
#include "physics/hitplunger.h"
#include "core/player.h"

#include "color.h"

#include "dragpoint.h"
#include "parts/timer.h"
#include "parts/flipper.h"
#include "parts/plunger.h"
#include "parts/textbox.h"
#include "parts/surface.h"
#include "parts/dispreel.h"
#include "parts/lightseq.h"
#include "parts/bumper.h"
#include "parts/trigger.h"
#include "parts/light.h"
#include "parts/kicker.h"
#include "parts/decal.h"
#include "parts/primitive.h"
#include "parts/hittarget.h"
#include "parts/gate.h"
#include "parts/spinner.h"
#include "parts/ramp.h"
#include "parts/flasher.h"
#include "parts/rubber.h"
#include "mixer.h"
#include "ushock.h"
#include "plumb.h"

#include "physics/kdtree.h"

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
   return (float)g_pplayer->m_wnd_width * (float)(1.0 / EDITOR_BG_WIDTH) * g_pplayer->m_AAfactor;
}

__forceinline float getBGymult()
{
   //return getBGxmult() / (((float)g_pplayer->m_screenwidth / (float)g_pplayer->m_screenheight) / (float)((double)EDITOR_BG_WIDTH / EDITOR_BG_HEIGHT));
   return getBGxmult() / (((float)g_pplayer->m_wnd_width / (float)g_pplayer->m_wnd_height) / (float)((double)EDITOR_BG_WIDTH / EDITOR_BG_HEIGHT));
}
