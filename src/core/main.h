// license:GPLv3+

#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifdef __STANDALONE__
#define __null 0
#define __WINE_WINCON_H
#endif

#include <windows.h>

#ifdef __STANDALONE__
#ifdef GetClassInfo
#undef GetClassInfo
#endif
#endif

#ifndef __STANDALONE__
#define RPC_NO_WINDOWS_H
#define COM_NO_WINDOWS_H
#include <objbase.h>
#endif

#if defined(ENABLE_DX9)
 #ifdef _DEBUG
  #define D3D_DEBUG_INFO
 #endif
 #include "minid3d9.h"
#endif

#ifndef __STANDALONE__
#include <mmsystem.h>
#endif

#include <atlbase.h>
#ifndef __STANDALONE__
#include <atlctl.h>
#endif

#ifdef _MSC_VER
#define PATH_SEPARATOR_CHAR '\\'
#define PATH_SEPARATOR_WCHAR L'\\'
#else
#define PATH_SEPARATOR_CHAR '/'
#define PATH_SEPARATOR_WCHAR L'/'
#endif

#include <oleauto.h>

#ifndef __STANDALONE__
#include <wincrypt.h>
#endif

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
#include <format>
#include <algorithm>
#ifndef __STANDALONE__
#include <commdlg.h>
#include <dlgs.h>
#include <cderr.h>
#endif

using namespace std::string_literals;
using namespace std::string_view_literals;
using std::string;
using std::wstring;
using std::vector;


#ifndef __STANDALONE__

#ifndef WM_THEMECHANGED
  #define WM_THEMECHANGED            0x031A
#endif

#include <wxx_appcore.h>		// Add CCriticalSection, CObject, CWinThread, CWinApp
//#include <wxx_archive.h>		// Add CArchive
#include <wxx_commondlg.h>		// Add CCommonDialog, CColorDialog, CFileDialog, CFindReplace, CFontDialog 
#include <wxx_scrollview.h>
#include <wxx_controls.h>		// Add CAnimation, CComboBox, CComboBoxEx, CDateTime, CHeader, CHotKey, CIPAddress, CProgressBar, CSpinButton, CScrollBar, CSlider, CToolTip
//#include <wxx_cstring.h>		// Add CString, CStringA, CStringW
//#include <wxx_ddx.h>			// Add CDataExchange
//#include <wxx_dialog.h>			// Add CDialog, CResizer
//#include <wxx_dockframe.h>		// Add CDockFrame, CMDIDockFrame
//#include <wxx_docking.h>		// Add CDocker, CDockContainer
//#include <wxx_exception.h>		// Add CException, CFileException, CNotSupportedException, CResourceException, CUserException, CWinException
//#include <wxx_file.h>			// Add CFile
//#include <wxx_frame.h>			// Add CFrame
//#include <wxx_gdi.h>			// Add CDC, CGDIObject, CBitmap, CBrush, CFont, CPalette, CPen, CRgn
//#include <wxx_imagelist.h>		// Add CImageList
#include <wxx_listview.h>		// Add CListView
//#include <wxx_mdi.h>			// Add CMDIChild, CMDIFrame, CDockMDIFrame
//#include <wxx_printdialogs.h>	// Add CPageSetupDialog, CPrintSetupDialog
//#include <wxx_propertysheet.h>	// Add CPropertyPage, CPropertySheet
//#include <wxx_rebar.h>			// Add CRebar
//#include <wxx_regkey.h>			// Add CRegKey
//#include <wxx_ribbon.h>		// Add CRibbon, CRibbonFrame
//#include <wxx_richedit.h>		// Add CRichEdit
//#include <wxx_socket.h>			// Add CSocket
//#include <wxx_statusbar.h>		// Add CStatusBar
#include <wxx_stdcontrols.h>	// Add CButton, CEdit, CListBox
//#include <wxx_tab.h>			// Add CTab, CTabbedMDI
//#include <wxx_taskdialog.h>	// Add CTaskDialog
//#include <wxx_time.h>			// Add CTime
//#include <wxx_toolbar.h>		// Add CToolBar
#include <wxx_treeview.h>		// Add CTreeView
//#include <wxx_webbrowser.h>		// Add CAXWindow, CWebBrowser
//#include <wxx_wincore.h>

#else

#define fopen_s(pFile, filename, mode) (((*(pFile)) = fopen((filename), (mode))) == nullptr)
#define fprintf_s fprintf
#define fread_s(buffer, bufferSize, elementSize, count, stream) fread(buffer, bufferSize, count, stream)
#define fscanf_s fscanf

#define sscanf_s sscanf

#ifndef __MINGW32__
#define localtime_s(x, y) localtime_r(y, x)
#define gmtime_s(x, y) gmtime_r(y, x)
#define _aligned_malloc(size, align) aligned_alloc(align, size)
#define _aligned_free free
#endif

#define _T(x) (x)
#define AtoT(x) (x)
#define _ASSERTE(expr) ((void)0)

#undef SetCurrentDirectory
#define SetCurrentDirectory SetCurrentDirectoryA

#undef MessageBox
#define MessageBox MessageBoxA

typedef ULONG_PTR HCRYPTPROV;
typedef ULONG_PTR HCRYPTHASH;
typedef ULONG_PTR HCRYPTKEY;

#pragma pack(push, 1)
typedef struct {
   WORD wFormatTag;
   WORD nChannels;
   DWORD nSamplesPerSec;
   DWORD nAvgBytesPerSec;
   WORD nBlockAlign;
   WORD wBitsPerSample;
   WORD cbSize;
} WAVEFORMATEX, *LPWAVEFORMATEX;
#pragma pack(pop)

typedef struct {
   DWORD lStructSize;
   HWND hwndOwner;
   HINSTANCE hInstance;
   DWORD Flags;
   LPSTR lpstrFindWhat;
   LPSTR lpstrReplaceWith;
   WORD wFindWhatLen;
   WORD wReplaceWithLen;
   LPARAM lCustData;
   void* lpfnHook;
   LPCSTR lpTemplateName;
} FINDREPLACEA;

#define FINDREPLACE FINDREPLACEA
#define CREATESTRUCT CREATESTRUCTA
#define WNDCLASS WNDCLASSA
#define LOGFONT LOGFONTA

typedef LPSTR LPTSTR;
typedef LPCSTR LPCTSTR;

class ImageDialog final { };
class SoundDialog final { };
class EditorOptionsDialog final { };
class CollectionManagerDialog final { };
class PhysicsOptionsDialog final { };
class TableInfoDialog final { };
class DimensionDialog final { };
class RenderProbeDialog final { };
class MaterialDialog final { };
class AboutDialog final { };
class ToolbarDialog final { };
class NotesDialog final { };
class PropertyDialog final { };
class SCNotification final { };
#endif

#include "utils/Logger.h"

#ifdef __STANDALONE__
#include <atldef.h>
#include <atlcom.h>
#include <atlcomcli.h>
#include <atlsafe.h>

#include <afx.h>
#include <afxdlgs.h>
#include <afxwin.h>
#include <atltypes.h>

#include "standalone/inc/win32xx/win32xx.h"
#endif

#include "def.h"

#include "math/math.h"
#include "math/vector.h"
#include "math/matrix.h"
#include "math/bbox.h"
#include "math/MeshUtils.h"

#include "ui/win/resource.h"

#include "dispid.h"

#include "utils/vector.h"
#include "utils/vectorsort.h"
#include "utils/color.h"

#ifndef __STANDALONE__
#include "vpinball.h"
#else
#include "standalone/vpinball_standalone_i.h"
#endif

#include "core/Settings.h"

#include "utils/wintimer.h"

#include "utils/eventproxy.h"

#include "utils/fileio.h"

#include "pinundo.h"

#include "iselect.h"

#include "core/Scriptable.h"

#include "parts/Collection.h"

#include "core/ieditable.h"

#include "extern.h"

#include "ui/win/WinEditor.h"

#include "core/VPApp.h"

#include "parts/pintable.h"

#include "renderer/Renderer.h"

#include "ui/win/sur.h"

#include "physics/hitable.h"

#include "core/player.h"

#include "renderer/trace.h"

#include "editablereg.h"
