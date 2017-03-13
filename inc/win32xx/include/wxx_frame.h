// Win32++   Version 8.4
// Release Date: 10th March 2017
//
//      David Nash
//      email: dnash@bigpond.net.au
//      url: https://sourceforge.net/projects/win32-framework
//
//
// Copyright (c) 2005-2017  David Nash
//
// Permission is hereby granted, free of charge, to
// any person obtaining a copy of this software and
// associated documentation files (the "Software"),
// to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify,
// merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom
// the Software is furnished to do so, subject to the
// following conditions:
//
// The above copyright notice and this permission notice
// shall be included in all copies or substantial portions
// of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF
// ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
// TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
// PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT
// SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
// ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
// ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
// OR OTHER DEALINGS IN THE SOFTWARE.
//
////////////////////////////////////////////////////////


////////////////////////////////////////////////////////
// wxx_frame.h
//  Declaration of the following classes:
//  CMargins, CMenuMetrics, CFrameT, CFrame, and CDockFrame.

// The classes declared in this file support SDI (Single Document Interface)
// frames on Win32/Win64 operating systems (not Windows CE). For Windows CE,
// use wceframe.h instead. SDI frames are a simple frame which supports a
// single view window. Refer to mdi.h for frames that support several
// child windows.

// CFrame inherits from CFrameT<CWnd>.
// CFrameT uses each of the following classes:
// * CReBar for managing the frame's rebar control.
// * CMenuBar for managing the menu inside the rebar.
// * CToolBar for managing the frame's toolbar.
// * CStatusBar for managing the frame's status bar.
// In each case the members for these classes are exposed by a GetXXX
// function, allowing them to be accessed or sent messages.

// CFrame is responsible for creating a "frame" window. This window has a
// menu and and several child windows, including a toolbar (usually hosted
// within a rebar), a status bar, and a view positioned over the frame
// window's non-client area. The "view" window is a separate CWnd object
// assigned to the frame with the SetView function.

// When compiling an application with these classes, it will need to be linked
// with Comctl32.lib.

// To create a SDI frame application, inherit a CMainFrame class from CFrame.
// Use the Frame sample application as the starting point for your own frame
// applications.
// Refer to the Notepad and Scribble samples for examples on how to use these
// classes to create a frame application.

// To create a SDI frame application that supports docking, inherit a CMainFrame
// class from CDockFrame.
// Refer to the Dock, DockContainer and DockTabbedMDI sample for example on how
// to create a docking frame application.


#ifndef _WIN32XX_FRAME_H_
#define _WIN32XX_FRAME_H_

#include "wxx_wincore.h"
#include "wxx_dialog.h"
#include "wxx_gdi.h"
#include "wxx_statusbar.h"
#include "wxx_toolbar.h"
#include "wxx_menubar.h"
#include "wxx_rebar.h"
#include "wxx_regkey.h"
#include "default_resource.h"


#ifndef RBN_MINMAX
  #define RBN_MINMAX (RBN_FIRST - 21)
#endif
#ifndef TMT_BORDERSIZE
  #define TMT_BORDERSIZE	2403
#endif
#ifndef TMT_CONTENTMARGINS
  #define TMT_CONTENTMARGINS	3602
#endif
#ifndef VSCLASS_MENU
  #define VSCLASS_MENU	L"MENU"
#endif
#ifndef ODS_NOACCEL
  #define ODS_NOACCEL 0x0100
#endif
#ifndef ODS_HOTLIGHT
  #define ODS_HOTLIGHT 0x0040
#endif
#ifndef ODS_INACTIVE
  #define ODS_INACTIVE 0x0080
#endif
#ifndef DT_HIDEPREFIX
  #define DT_HIDEPREFIX 0x00100000
#endif

#ifndef WM_UNINITMENUPOPUP
  #define WM_UNINITMENUPOPUP		0x0125
#endif

#ifndef WM_MENURBUTTONUP
  #define WM_MENURBUTTONUP		0x0122
#endif


namespace Win32xx
{

	////////////////////////////////////////////////
	// Declarations of structures for themes
	//

	// define some structs and enums from uxtheme.h and vssym32.h
	struct MARGINS
	{
		int cxLeftWidth;      // width of left border that retains its size
		int cxRightWidth;     // width of right border that retains its size
		int cyTopHeight;      // height of top border that retains its size
		int cyBottomHeight;   // height of bottom border that retains its size
	};

	class CMargins : public MARGINS
	{
	public:
		CMargins(int cxLeft, int cxRight, int cyTop, int cyBottom)
		{
			cxLeftWidth    = cxLeft;	cxRightWidth   = cxRight;
			cyTopHeight    = cyTop;		cyBottomHeight = cyBottom;
		}
		CMargins()
		{
			cxLeftWidth    = 0;			cxRightWidth   = 0;
			cyTopHeight    = 0;			cyBottomHeight = 0;
		}
		int Width() const		{ return cxLeftWidth + cxRightWidth; }
		int Height() const		{ return cyTopHeight + cyBottomHeight; }
		void SetMargins(int cxLeft, int cxRight, int cyTop, int cyBottom)
		{
			cxLeftWidth    = cxLeft;	cxRightWidth   = cxRight;
			cyTopHeight    = cyTop;		cyBottomHeight = cyBottom;
		}
	};

	enum THEMESIZE
	{
		TS_MIN,             // minimum size
		TS_TRUE,            // size without stretching
		TS_DRAW             // size that theme mgr will use to draw part
	};

	enum POPUPCHECKSTATES
	{
		MC_CHECKMARKNORMAL = 1,
		MC_CHECKMARKDISABLED = 2,
		MC_BULLETNORMAL = 3,
		MC_BULLETDISABLED = 4
	};

	enum POPUPCHECKBACKGROUNDSTATES
	{
		MCB_DISABLED = 1,
		MCB_NORMAL = 2,
		MCB_BITMAP = 3
	};

	enum POPUPITEMSTATES
	{
		MPI_NORMAL = 1,
		MPI_HOT = 2,
		MPI_DISABLED = 3,
		MPI_DISABLEDHOT = 4
	};

	enum POPUPSUBMENUSTATES
	{
		MSM_NORMAL = 1,
		MSM_DISABLED = 2
	};

	enum MENUPARTS
	{
		MENU_MENUITEM_TMSCHEMA = 1,
		MENU_MENUDROPDOWN_TMSCHEMA = 2,
		MENU_MENUBARITEM_TMSCHEMA = 3,
		MENU_MENUBARDROPDOWN_TMSCHEMA = 4,
		MENU_CHEVRON_TMSCHEMA = 5,
		MENU_SEPARATOR_TMSCHEMA = 6,
		MENU_BARBACKGROUND = 7,
		MENU_BARITEM = 8,
		MENU_POPUPBACKGROUND = 9,
		MENU_POPUPBORDERS = 10,
		MENU_POPUPCHECK = 11,
		MENU_POPUPCHECKBACKGROUND = 12,
		MENU_POPUPGUTTER = 13,
		MENU_POPUPITEM = 14,
		MENU_POPUPSEPARATOR = 15,
		MENU_POPUPSUBMENU = 16,
		MENU_SYSTEMCLOSE = 17,
		MENU_SYSTEMMAXIMIZE = 18,
		MENU_SYSTEMMINIMIZE = 19,
		MENU_SYSTEMRESTORE = 20
	};


	struct MenuItemData
	// Each Dropdown menu item has this data
	{
		HMENU hMenu;
		MENUITEMINFO mii;
		UINT  nPos;
		std::vector<TCHAR> vItemText;

		MenuItemData() : hMenu(0), nPos(0)
		{
			ZeroMemory(&mii, GetSizeofMenuItemInfo());
			vItemText.assign(MAX_MENU_STRING, _T('\0'));
		}
		LPTSTR GetItemText() {return &vItemText[0];}
	};


	/////////////////////////////////////
	// Declaration of the CMenuMetrics class
	//  This class is used by CFrameT to retrieve the size of the components
	//  used to perform owner-drawing of menu items.  Windows Visual Styles
	//  are used to render menu items for systems running Aero (Vista and above).
	//  Win32++ custom themes are used to render menu items for systems without Aero.
	class CMenuMetrics
	{
	public:
		CMenuMetrics();
		~CMenuMetrics();

		CRect GetCheckBackgroundRect(const CRect& rcItem) const;
		CRect GetCheckRect(const CRect& rcItem) const;
		CRect GetGutterRect(const CRect& rcItem) const;
		CSize GetItemSize(MenuItemData* pmd) const;
		CRect GetSelectionRect(const CRect& rcItem) const;
		CRect GetSeperatorRect(const CRect& rcItem) const;
		CRect GetTextRect(const CRect& rcItem) const;
		CSize GetTextSize(MenuItemData* pmd) const;
		void  Initialize();
		BOOL  IsVistaMenu() const;
		CRect ScaleRect(const CRect& rcItem) const;
		CSize ScaleSize(const CSize& szItem) const;
		int   ToItemStateId(UINT uItemState) const;
		int   ToCheckBackgroundStateId(int iStateId) const;
		int   ToCheckStateId(UINT fType, int iStateId) const;

		// Wrappers for Windows API functions
		HRESULT CloseThemeData() const;
		HRESULT DrawThemeBackground(HDC hdc, int iPartId, int iStateId, const RECT *pRect, const RECT *pClipRect) const;
		HRESULT DrawThemeText(HDC hdc, int iPartId, int iStateId, LPCWSTR pszText, int iCharCount, DWORD dwTextFlags, DWORD dwTextFlags2, LPCRECT pRect) const;
		HRESULT GetThemePartSize(HDC hdc, int iPartId, int iStateId, LPCRECT prc, THEMESIZE eSize, SIZE* psz) const;
		HRESULT GetThemeInt(int iPartId, int iStateId, int iPropId, int* piVal) const;
		HRESULT GetThemeMargins(HDC hdc, int iPartId, int iStateId, int iPropId, LPRECT prc, MARGINS* pMargins) const;
		HRESULT GetThemeTextExtent(HDC hdc, int iPartId, int iStateId, LPCWSTR pszText, int iCharCount, DWORD dwTextFlags, LPCRECT pBoundingRect, LPRECT pExtentRect) const;
		BOOL    IsThemeBackgroundPartiallyTransparent(int iPartId, int iStateId) const;
		HANDLE  OpenThemeData(HWND hwnd, LPCWSTR pszClassList) const;

		HANDLE  m_hTheme;				// Theme handle
		HWND m_hFrame;					// Handle to the frame window
		HMODULE m_hmodUXTheme;			// Module handle to the UXTheme dll

		CMargins m_marCheck;			// Check margins
		CMargins m_marCheckBackground;	// Check background margins
		CMargins m_marItem;				// Item margins
		CMargins m_marText;				// Text margins

		CSize   m_sizeCheck;			// Check size metric
		CSize   m_sizeSeparator;		// Separator size metric

	private:
		typedef HRESULT WINAPI CLOSETHEMEDATA(HANDLE);
		typedef HRESULT WINAPI DRAWTHEMEBACKGROUND(HANDLE, HDC, int, int, const RECT*, const RECT*);
		typedef HRESULT WINAPI DRAWTHEMETEXT(HANDLE, HDC, int, int, LPCWSTR, int, DWORD, DWORD, LPCRECT);
		typedef HRESULT WINAPI GETTHEMEPARTSIZE(HANDLE, HDC, int, int, LPCRECT, THEMESIZE, SIZE*);
		typedef HRESULT WINAPI GETTHEMEINT(HANDLE, int, int, int, int*);
		typedef HRESULT WINAPI GETTHEMEMARGINS(HANDLE, HDC, int, int, int, LPRECT, MARGINS*);
		typedef HRESULT WINAPI GETTHEMETEXTEXTENT(HANDLE, HDC, int, int, LPCWSTR, int, DWORD, LPCRECT, LPCRECT);
		typedef BOOL	WINAPI ISTHEMEBGPARTTRANSPARENT(HANDLE, int, int);
		typedef HANDLE  WINAPI OPENTHEMEDATA(HWND, LPCWSTR);

		// Pointers to functions defined in uxTheme.dll
		CLOSETHEMEDATA*			  m_pfnCloseThemeData;
		DRAWTHEMEBACKGROUND*	  m_pfnDrawThemeBackground;
		DRAWTHEMETEXT*			  m_pfnDrawThemeText;
		GETTHEMEPARTSIZE*		  m_pfnGetThemePartSize;
		GETTHEMEINT*			  m_pfnGetThemeInt;
		GETTHEMEMARGINS*		  m_pfnGetThemeMargins;
		GETTHEMETEXTEXTENT*		  m_pfnGetThemeTextExtent;
		ISTHEMEBGPARTTRANSPARENT* m_pfnIsThemeBGPartTransparent;
		OPENTHEMEDATA*			  m_pfnOpenThemeData;
	};


	//////////////////////////////////
	// Declaration of the CFrameT class template
	//
	template <class T>		// The template parameter T is typically either CWnd or CDocker
	class CFrameT : public T
	{
		typedef Shared_Ptr<MenuItemData> ItemDataPtr;

	public:

		struct InitValues
		{
			CRect rcPos;
			int	  ShowCmd;
			BOOL  ShowStatusBar;
			BOOL  ShowToolBar;

			// Display StatusBar and ToolBar by default
			InitValues() : ShowCmd(SW_SHOW), ShowStatusBar(TRUE), ShowToolBar(TRUE) {}	// constructor
		};

		CFrameT();
		virtual ~CFrameT();

		// Override these functions as required
		virtual void AdjustFrameRect(const RECT& rcView);
		virtual CString GetThemeName() const;
		virtual CRect GetViewRect() const;
		virtual BOOL IsMDIChildMaxed() const { return FALSE; }
		virtual BOOL IsMDIFrame() const { return FALSE; }
		virtual void SetStatusIndicators();
		virtual void RecalcLayout();
		virtual void RecalcViewLayout();

		virtual CWnd& GetView() const		{ return *m_pView; }
		virtual void SetView(CWnd& wndView);

		// Virtual Attributes
		// If you need to modify the default behaviour of the MenuBar, ReBar,
		// StatusBar or ToolBar, inherit from those classes, and override
		// the following attribute functions.
		virtual CMenuBar& GetMenuBar()	const		{ return m_MenuBar; }
		virtual CReBar& GetReBar() const			{ return m_ReBar; }
		virtual CStatusBar& GetStatusBar() const	{ return m_StatusBar; }
		virtual CToolBar& GetToolBar() const		{ return m_ToolBar; }

		// Non-virtual Attributes
		// These functions aren't virtual, and shouldn't be overridden
		CRect ExcludeChildRect(CRect& rcClient, HWND hChild) const;
		HACCEL GetFrameAccel() const				{ return m_hAccel; }
		InitValues GetInitValues() const			{ return m_InitValues; }
		CMenu& GetFrameMenu() const					{ return m_Menu; }
		MenuTheme& GetMenuBarTheme() const			{ return m_MBTheme; }
		CMenuMetrics& GetMenuMetrics()				{ return m_MenuMetrics; }
		std::vector<CString> GetMRUEntries() const	{ return m_vMRUEntries; }
		CString GetMRUEntry(UINT nIndex);
		UINT GetMRULimit()							{ return m_nMaxMRU; }
		CString GetRegistryKeyName() const			{ return m_strKeyName; }
		ReBarTheme& GetReBarTheme()	const			{ return m_RBTheme; }
		StatusBarTheme& GetStatusBarTheme()	const	{ return m_SBTheme; }
		std::vector<UINT>& GetToolBarData()			{ return m_vToolBarData; }
		ToolBarTheme& GetToolBarTheme()	const		{ return m_TBTheme; }
		CString GetStatusText() const				{ return m_strStatusText; }
		CString GetTitle() const					{ return T::GetWindowText(); }
		BOOL IsMenuBarUsed() const					{ return (GetMenuBar().IsWindow()); }
		BOOL IsReBarSupported() const				{ return (GetComCtlVersion() > 470); }
		BOOL IsReBarUsed() const					{ return (GetReBar().IsWindow()); }
		BOOL IsVistaMenuUsed() const				{ return m_MenuMetrics.IsVistaMenu(); }
		void SetAccelerators(UINT ID_ACCEL);
		void SetFrameMenu(UINT ID_MENU);
		void SetFrameMenu(HMENU hMenu);
		void SetInitValues(const InitValues& Values);
		void SetMenuTheme(MenuTheme& MBT);
		void SetMRULimit(UINT MRULimit);
		void SetReBarTheme(ReBarTheme& RBT);
		void SetStatusBarTheme(StatusBarTheme& SBT);
		void SetStatusText(LPCTSTR szText);
		void SetTitle(LPCTSTR szText)				{ T::SetWindowText(szText); }
		void SetToolBarTheme(ToolBarTheme& TBT);

	protected:
		// Override these functions as required.
		virtual BOOL AddMenuIcon(int nID_MenuItem, HICON hIcon);
		virtual UINT AddMenuIcons(const std::vector<UINT>& MenuData, COLORREF crMask, UINT BitmapID, UINT BitmapDisabledID);
		virtual void AddMenuBarBand();
		virtual void AddMRUEntry(LPCTSTR szMRUEntry);
		virtual void AddToolBarBand(CToolBar& TB, DWORD dwBandStyle, UINT nID);
		virtual void AddToolBarButton(UINT nID, BOOL IsEnabled = TRUE, LPCTSTR szText = 0, int iImage = -1);
		virtual void CreateToolBar();
		virtual LRESULT CustomDrawMenuBar(NMHDR* pNMHDR);
		virtual LRESULT CustomDrawToolBar(NMHDR* pNMHDR);
		virtual void DrawMenuItem(LPDRAWITEMSTRUCT pdis);
		virtual void DrawMenuItemBkgnd(LPDRAWITEMSTRUCT pdis);
		virtual void DrawMenuItemCheckmark(LPDRAWITEMSTRUCT pdis);
		virtual void DrawMenuItemIcon(LPDRAWITEMSTRUCT pdis);
		virtual void DrawMenuItemText(LPDRAWITEMSTRUCT pdis);
		virtual BOOL DrawReBarBkgnd(CDC& dc, CReBar& ReBar);
		virtual BOOL DrawStatusBarBkgnd(CDC& dc, CStatusBar& StatusBar);
		virtual int  GetMenuItemPos(HMENU hMenu, LPCTSTR szItem);
		virtual BOOL LoadRegistrySettings(LPCTSTR szKeyName);
		virtual BOOL LoadRegistryMRUSettings(UINT nMaxMRU = 0);
		virtual void MeasureMenuItem(MEASUREITEMSTRUCT *pmis);
		virtual LRESULT OnActivate(UINT uMsg, WPARAM wParam, LPARAM lParam);
		virtual void OnClose();
		virtual int  OnCreate(CREATESTRUCT& cs);
		virtual LRESULT OnCustomDraw(LPNMHDR pNMHDR);
		virtual void OnDestroy();
		virtual LRESULT OnDrawItem(UINT uMsg, WPARAM wParam, LPARAM lParam);	
		virtual BOOL OnHelp();
		virtual LRESULT OnInitMenuPopup(UINT uMsg, WPARAM wParam, LPARAM lParam);
		virtual LRESULT OnMeasureItem(UINT uMsg, WPARAM wParam, LPARAM lParam);
		virtual LRESULT OnMenuChar(UINT uMsg, WPARAM wParam, LPARAM lParam);
		virtual LRESULT OnMenuSelect(UINT uMsg, WPARAM wParam, LPARAM lParam);
		virtual void OnMenuUpdate(UINT nID);
		virtual LRESULT OnNotify(WPARAM wParam, LPARAM lParam);
		virtual LRESULT OnRBNHeightChange(LPNMHDR pNMHDR);
		virtual LRESULT OnRBNLayoutChanged(LPNMHDR pNMHDR);
		virtual LRESULT OnRBNMinMax(LPNMHDR pNMHDR);
		virtual LRESULT OnSetFocus(UINT uMsg, WPARAM wParam, LPARAM lParam);
		virtual LRESULT OnSettingChange(UINT, WPARAM, LPARAM);
		virtual LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam);	
		virtual LRESULT OnSysColorChange(UINT uMsg, WPARAM wParam, LPARAM lParam);
		virtual LRESULT OnSysCommand(UINT uMsg, WPARAM wParam, LPARAM lParam);		
		virtual LRESULT OnTBNDropDown(LPNMTOOLBAR pNMTB);
		virtual LRESULT OnTTNGetDispInfo(LPNMTTDISPINFO pNMTDI);
		virtual LRESULT OnUndocked();
		virtual LRESULT OnUnInitMenuPopup(UINT, WPARAM wParam, LPARAM lParam);	
		virtual BOOL OnViewStatusBar();
		virtual BOOL OnViewToolBar();
		virtual void PreCreate(CREATESTRUCT& cs);
		virtual void PreRegisterClass(WNDCLASS& wc);
		virtual void RemoveMRUEntry(LPCTSTR szMRUEntry);
		virtual BOOL SaveRegistryMRUSettings();
		virtual BOOL SaveRegistrySettings();
		virtual void SetMenuBarBandSize();
		virtual UINT SetMenuIcons(const std::vector<UINT>& MenuData, COLORREF crMask, UINT ToolBarID, UINT ToolBarDisabledID);
		virtual void SetTBImageList(CToolBar& ToolBar, CImageList& ImageList, UINT nID, COLORREF crMask);
		virtual void SetTBImageListDis(CToolBar& ToolBar, CImageList& ImageList, UINT nID, COLORREF crMask);
		virtual void SetTBImageListHot(CToolBar& ToolBar, CImageList& ImageList, UINT nID, COLORREF crMask);
		virtual void SetupMenuIcons();
		virtual void SetupToolBar();
		virtual void SetTheme();
		virtual void SetToolBarImages(COLORREF crMask, UINT ToolBarID, UINT ToolBarHotID, UINT ToolBarDisabledID);
		virtual void ShowMenu(BOOL Show);
		virtual void ShowStatusBar(BOOL Show);
		virtual void ShowToolBar(BOOL Show);		
		virtual void UpdateMRUMenu();

		// Not intended to be overridden
		BOOL GetUseIndicatorStatus() const { return m_UseIndicatorStatus; }
		BOOL GetUseMenuStatus() const { return m_UseMenuStatus; }
		BOOL GetUseReBar() const { return m_UseReBar; }
		BOOL GetUseThemes() const { return m_UseThemes; }
		BOOL GetUseToolBar() const { return m_UseToolBar; }
		void SetUseIndicatorStatus(BOOL UseIndicatorStatus) { m_UseIndicatorStatus = UseIndicatorStatus; }
		void SetUseMenuStatus(BOOL UseMenuStatus) { m_UseMenuStatus = UseMenuStatus; }
		void SetUseReBar(BOOL UseReBar) { m_UseReBar = UseReBar; }
		void SetUseThemes(BOOL UseThemes) { m_UseThemes = UseThemes; }
		void SetUseToolBar(BOOL UseToolBar) { m_UseToolBar = UseToolBar; }

		LRESULT WndProcDefault(UINT uMsg, WPARAM wParam, LPARAM lParam);

	private:
		CFrameT(const CFrameT&);				// Disable copy construction
		CFrameT& operator = (const CFrameT&);	// Disable assignment operator
		CSize GetTBImageSize(CBitmap* pbm);
		void UpdateMenuBarBandSize();
		static LRESULT CALLBACK StaticKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);

		std::vector<ItemDataPtr> m_vMenuItemData;	// vector of MenuItemData pointers
		std::vector<CString> m_vMRUEntries;	// Vector of CStrings for MRU entries
		std::vector<UINT> m_vMenuIcons;		// vector of menu icon resource IDs
		std::vector<UINT> m_vToolBarData;	// vector of resource IDs for ToolBar buttons
		InitValues m_InitValues;			// struct of initial values
		CDialog m_AboutDialog;				// Help about dialog
		mutable CMenuBar m_MenuBar;			// CMenuBar object
		mutable CReBar m_ReBar;				// CReBar object
		mutable CStatusBar m_StatusBar;		// CStatusBar object
		mutable CToolBar m_ToolBar;			// CToolBar object
		mutable CMenu m_Menu;				// handle to the frame menu
		CFont m_fntMenuBar;					// MenuBar font
		CFont m_fntStatusBar;				// StatusBar font
		CImageList m_ToolBarImages;			// Image list for the ToolBar buttons
		CImageList m_ToolBarDisabledImages;	// Image list for the Disabled ToolBar buttons
		CImageList m_ToolBarHotImages;		// Image list for the Hot ToolBar buttons
		CString m_OldStatus[3];				// Array of CString holding old status;
		CString m_strKeyName;				// CString for Registry key name
		CString m_strStatusText;			// CString for status text
		CString m_strTooltip;				// CString for tool tips
		CString m_XPThemeName;				// CString for Windows Theme Name
		mutable MenuTheme m_MBTheme;		// struct of theme info for the popup Menu and MenuBar
		mutable ReBarTheme m_RBTheme;		// struct of theme info for the ReBar
		mutable StatusBarTheme m_SBTheme;	// struct of theme info for the StatusBar
		mutable ToolBarTheme m_TBTheme;		// struct of theme info for the ToolBar
		HACCEL m_hAccel;					// handle to the frame's accelerator table (used by MDI without MDI child)
		CWnd* m_pView;						// pointer to the View CWnd object
		UINT m_nMaxMRU;						// maximum number of MRU entries
		HWND m_hOldFocus;					// The window which had focus prior to the app's deactivation
		BOOL m_DrawArrowBkgrnd;				// True if a separate arrow background is to be drawn on toolbar
		HHOOK m_KbdHook;					// Keyboard hook.

		CMenuMetrics m_MenuMetrics;			// The MenuMetrics object
		CImageList m_imlMenu;				// Imagelist of menu icons
		CImageList m_imlMenuDis;			// Imagelist of disabled menu icons
		BOOL m_UseIndicatorStatus;			// set to TRUE to see indicators in status bar
		BOOL m_UseMenuStatus;				// set to TRUE to see menu and toolbar updates in status bar
		BOOL m_UseReBar;					// set to TRUE if ReBars are to be used
		BOOL m_UseThemes;					// set to TRUE if themes are to be used
		BOOL m_UseToolBar;					// set to TRUE if the toolbar is used

	};  // class CFrameT


	/////////////////////////////////////////
	// Declaration of the CFrame class
	// Provides a Single Document Interface (SDI) frame
	//
	class CFrame : public CFrameT<CWnd> 
	{
	public:
		CFrame() {}
		virtual ~CFrame() {}
	};

}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


namespace Win32xx
{

	/////////////////////////////////////////
	// Definitions for the CMenuMetrics class
	//
	inline CMenuMetrics::CMenuMetrics() : m_hTheme(0), m_hmodUXTheme(0), m_pfnCloseThemeData(0), m_pfnDrawThemeBackground(0),
											m_pfnDrawThemeText(0), m_pfnGetThemePartSize(0), m_pfnGetThemeInt(0), m_pfnGetThemeMargins(0),
											m_pfnGetThemeTextExtent(0), m_pfnIsThemeBGPartTransparent(0), m_pfnOpenThemeData(0)
	{
		m_hFrame = 0;
	}

	inline CMenuMetrics::~CMenuMetrics()
	{
		if (m_hTheme != 0)
			CloseThemeData();

		if (m_hmodUXTheme != 0)
			::FreeLibrary(m_hmodUXTheme);
	}

	inline HRESULT CMenuMetrics::CloseThemeData() const
	// Closes the theme data handle.
	{
		if (m_pfnCloseThemeData)
			return m_pfnCloseThemeData(m_hTheme);

		return E_NOTIMPL;
	}

	inline HRESULT CMenuMetrics::DrawThemeBackground(HDC hdc, int iPartId, int iStateId, const RECT *pRect, const RECT *pClipRect) const
	// Draws the border and fill defined by the visual style for the specified control part.
	{
		assert(m_hTheme);
		if (m_pfnDrawThemeBackground)
			return m_pfnDrawThemeBackground(m_hTheme, hdc, iPartId, iStateId, pRect, pClipRect);

		return E_NOTIMPL;
	}

	inline HRESULT CMenuMetrics::DrawThemeText(HDC hdc, int iPartId, int iStateId, LPCWSTR pszText, int iCharCount, DWORD dwTextFlags, DWORD dwTextFlags2, LPCRECT pRect) const
	// Draws text using the color and font defined by the visual style.
	{
		assert(m_hTheme);
		if (m_pfnDrawThemeText)
			return m_pfnDrawThemeText(m_hTheme, hdc, iPartId, iStateId, pszText, iCharCount, dwTextFlags, dwTextFlags2, pRect);

		return E_NOTIMPL;
	}

	inline CRect CMenuMetrics::GetCheckBackgroundRect(const CRect& rcItem) const
	{
		int cx = m_sizeCheck.cx + m_marCheck.Width();
		int cy = m_sizeCheck.cy + m_marCheck.Height();

		int x = rcItem.left + m_marCheckBackground.cxLeftWidth;
		int y = rcItem.top + (rcItem.Height() - cy) / 2;

		return CRect(x, y, x + cx, y + cy);
	}

	inline CRect CMenuMetrics::GetGutterRect(const CRect& rcItem) const
	{
		int x = rcItem.left;
		int y = rcItem.top;
		int cx = m_marItem.cxLeftWidth + m_marCheckBackground.Width() + m_marCheck.Width() + m_sizeCheck.cx;
		int cy = rcItem.Height();

		return CRect(x, y, x + cx, y + cy);
	}

	inline CRect CMenuMetrics::GetCheckRect(const CRect& rcItem) const
	{
		int x = rcItem.left + m_marCheckBackground.cxLeftWidth + m_marCheck.cxLeftWidth;
		int y = rcItem.top + (rcItem.Height() - m_sizeCheck.cy) / 2;

		return CRect(x, y, x + m_sizeCheck.cx, y + m_sizeCheck.cy);
	}

	inline CSize CMenuMetrics::GetItemSize(MenuItemData* pmd) const
		// Retrieve the size of the menu item
	{
		CSize size;

		// Add icon/check width
		size.cx += m_sizeCheck.cx + m_marCheckBackground.Width() + m_marCheck.Width();

		if (pmd->mii.fType & MFT_SEPARATOR)
		{
			// separator height
			size.cy = m_sizeSeparator.cy + m_marItem.Height();
		}
		else
		{
			// Add check background horizontal padding.
			size.cx += m_marCheckBackground.Width();

			// Add selection margin padding.
			size.cx += m_marItem.Width();

			// Account for text size
			CSize sizeText = ScaleSize(GetTextSize(pmd));
			size.cx += sizeText.cx;
			size.cy = MAX(size.cy, sizeText.cy);

			// Account for icon or check height
			size.cy = MAX(size.cy, m_sizeCheck.cy + m_marCheckBackground.Height() + m_marCheck.Height());
		}

		return (size);
	}

	inline CRect CMenuMetrics::GetSelectionRect(const CRect& rcItem) const
	{
		int x = rcItem.left + m_marItem.cxLeftWidth;
		int y = rcItem.top;

		return CRect(x, y, rcItem.right - m_marItem.cxRightWidth, y + rcItem.Height());
	}

	inline CRect CMenuMetrics::GetSeperatorRect(const CRect& rcItem) const
	{
		int left = GetGutterRect(rcItem).right;
		int top = rcItem.top;
		int right = rcItem.right - m_marItem.cxRightWidth;
		int bottom = rcItem.top + m_sizeSeparator.cy;

		return CRect(left, top, right, bottom);
	}

	inline CSize CMenuMetrics::GetTextSize(MenuItemData* pmd) const
	{
		CSize sizeText;
		assert(m_hFrame);
		CClientDC DesktopDC(NULL);
		LPCTSTR szItemText = pmd->GetItemText();

		if (IsVistaMenu())
		{
			CRect rcText;
			GetThemeTextExtent(DesktopDC, MENU_POPUPITEM, 0, TtoW(szItemText), lstrlen(szItemText),
				DT_LEFT | DT_SINGLELINE, NULL, &rcText);

			sizeText.SetSize(rcText.right + m_marText.Width(), rcText.bottom + m_marText.Height());
		}
		else
		{
			// Get the font used in menu items
			NONCLIENTMETRICS info;
			ZeroMemory(&info, sizeof(NONCLIENTMETRICS));
			info.cbSize = GetSizeofNonClientMetrics();
			SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(info), &info, 0);

			// Default menu items are bold, so take this into account
			if ((int)::GetMenuDefaultItem(pmd->hMenu, TRUE, GMDI_USEDISABLED) != -1)
				info.lfMenuFont.lfWeight = FW_BOLD;

			// Calculate the size of the text
			DesktopDC.CreateFontIndirect(info.lfMenuFont);
			sizeText = DesktopDC.GetTextExtentPoint32(szItemText, lstrlen(szItemText));
			sizeText.cx += m_marText.cxRightWidth;
			sizeText.cy += m_marText.Height();
		}

		if (_tcschr(szItemText, _T('\t')))
			sizeText.cx += 8;	// Add POST_TEXT_GAP if the text includes a tab

		return sizeText;
	}

	inline CRect CMenuMetrics::GetTextRect(const CRect& rcItem) const
	{
		int left = GetGutterRect(rcItem).Width() + m_marText.cxLeftWidth;
		int top = rcItem.top + m_marText.cyTopHeight;
		int right = rcItem.right - m_marItem.cxRightWidth - m_marText.cxRightWidth;
		int bottom = rcItem.bottom - m_marText.cyBottomHeight;

		return CRect(left, top, right, bottom);
	}

	inline HRESULT CMenuMetrics::GetThemePartSize(HDC hdc, int iPartId, int iStateId, LPCRECT prc, THEMESIZE eSize, SIZE* psz) const
	// Calculates the original size of the part defined by a visual style.
	{
		assert(m_hTheme);
		if (m_pfnGetThemePartSize)
			return m_pfnGetThemePartSize(m_hTheme, hdc, iPartId, iStateId, prc, eSize, psz);

		return E_NOTIMPL;
	}

	inline HRESULT CMenuMetrics::GetThemeInt(int iPartId, int iStateId, int iPropId, int* piVal) const
	// Retrieves the value of an int property.
	{
		assert(m_hTheme);
		if (m_pfnGetThemeInt)
			return m_pfnGetThemeInt(m_hTheme, iPartId, iStateId, iPropId, piVal);

		return E_NOTIMPL;
	}

	inline HRESULT CMenuMetrics::GetThemeMargins(HDC hdc, int iPartId, int iStateId, int iPropId, LPRECT prc, MARGINS* pMargins) const
	// Retrieves the value of a MARGINS property.
	{
		assert(m_hTheme);
		if (m_pfnGetThemeMargins)
			return m_pfnGetThemeMargins(m_hTheme, hdc, iPartId, iStateId, iPropId, prc, pMargins);

		return E_NOTIMPL;
	}

	inline HRESULT CMenuMetrics::GetThemeTextExtent(HDC hdc, int iPartId, int iStateId, LPCWSTR pszText, int iCharCount, DWORD dwTextFlags, LPCRECT pBoundingRect, LPRECT pExtentRect) const
	// Calculates the size and location of the specified text when rendered in the visual style font.
	{
		assert(m_hTheme);
		if (m_pfnGetThemeTextExtent)
			return m_pfnGetThemeTextExtent(m_hTheme, hdc, iPartId, iStateId, pszText, iCharCount, dwTextFlags, pBoundingRect, pExtentRect);

		return E_NOTIMPL;
	}

	inline void CMenuMetrics::Initialize()
	{
		assert(m_hFrame);

		if (m_hmodUXTheme == 0)
			m_hmodUXTheme = ::LoadLibrary(_T("UXTHEME.DLL"));

		if (m_hmodUXTheme != 0)
		{
			m_pfnCloseThemeData = reinterpret_cast<CLOSETHEMEDATA*>(::GetProcAddress(m_hmodUXTheme, "CloseThemeData"));
			m_pfnDrawThemeBackground = reinterpret_cast<DRAWTHEMEBACKGROUND*>(::GetProcAddress(m_hmodUXTheme, "DrawThemeBackground"));
			m_pfnDrawThemeText = reinterpret_cast<DRAWTHEMETEXT*>(::GetProcAddress(m_hmodUXTheme, "DrawThemeText"));
			m_pfnGetThemePartSize = reinterpret_cast<GETTHEMEPARTSIZE*>(::GetProcAddress(m_hmodUXTheme, "GetThemePartSize"));
			m_pfnGetThemeInt = reinterpret_cast<GETTHEMEINT*>(::GetProcAddress(m_hmodUXTheme, "GetThemeInt"));
			m_pfnGetThemeMargins = reinterpret_cast<GETTHEMEMARGINS*>(::GetProcAddress(m_hmodUXTheme, "GetThemeMargins"));
			m_pfnGetThemeTextExtent = reinterpret_cast<GETTHEMETEXTEXTENT*>(::GetProcAddress(m_hmodUXTheme, "GetThemeTextExtent"));
			m_pfnIsThemeBGPartTransparent = reinterpret_cast<ISTHEMEBGPARTTRANSPARENT*>(::GetProcAddress(m_hmodUXTheme, "IsThemeBackgroundPartiallyTransparent"));
			m_pfnOpenThemeData = reinterpret_cast<OPENTHEMEDATA*>(::GetProcAddress(m_hmodUXTheme, "OpenThemeData"));
		}

		if (m_hTheme != 0)
		{
			CloseThemeData();
			m_hTheme = 0;
		}

		m_hTheme = OpenThemeData(m_hFrame, VSCLASS_MENU);

		if (m_hTheme != 0)
		{
			int iBorderSize = 0;	// Border space between item text and accelerator
			int iBgBorderSize = 0;	// Border space between item text and gutter
			GetThemePartSize(NULL, MENU_POPUPCHECK, 0, NULL, TS_TRUE, &m_sizeCheck);
			GetThemePartSize(NULL, MENU_POPUPSEPARATOR, 0, NULL, TS_TRUE, &m_sizeSeparator);
			GetThemeInt(MENU_POPUPITEM, 0, TMT_BORDERSIZE, &iBorderSize);
			GetThemeInt(MENU_POPUPBACKGROUND, 0, TMT_BORDERSIZE, &iBgBorderSize);
			GetThemeMargins(NULL, MENU_POPUPCHECK, 0, TMT_CONTENTMARGINS, NULL, &m_marCheck);
			GetThemeMargins(NULL, MENU_POPUPCHECKBACKGROUND, 0, TMT_CONTENTMARGINS, NULL, &m_marCheckBackground);
			GetThemeMargins(NULL, MENU_POPUPITEM, 0, TMT_CONTENTMARGINS, NULL, &m_marItem);

			// Popup text margins
			m_marText = m_marItem;
			m_marText.cxRightWidth = iBorderSize;
			m_marText.cxLeftWidth = iBgBorderSize;
		}
		else
		{
			m_sizeCheck.SetSize(GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON));
			m_sizeSeparator.SetSize(1, 7);
			m_marCheck.SetMargins(4, 4, 2, 2);
			m_marCheckBackground.SetMargins(0, 0, 0, 0);
			m_marItem.SetMargins(0, 0, 0, 0);
			m_marText.SetMargins(8, 16, 0, 0);
		}
	}

	inline BOOL CMenuMetrics::IsThemeBackgroundPartiallyTransparent(int iPartId, int iStateId) const
	// Retrieves whether the background specified by the visual style has transparent pieces or alpha-blended pieces.
	{
		assert(m_hTheme);
		if (m_pfnIsThemeBGPartTransparent)
			return m_pfnIsThemeBGPartTransparent(m_hTheme, iPartId, iStateId);

		return FALSE;
	}

	inline HANDLE CMenuMetrics::OpenThemeData(HWND hwnd, LPCWSTR pszClassList) const
	// Opens the theme data for a window and its associated class.
	{
		assert(hwnd);
		if (m_pfnOpenThemeData)
			return m_pfnOpenThemeData(hwnd, pszClassList);

		return NULL;
	}

	inline BOOL CMenuMetrics::IsVistaMenu() const
	{
		return (m_hTheme != FALSE);
	}

	inline CRect CMenuMetrics::ScaleRect(const CRect& rcItem) const
	// Re-scale the CRect to support the system's DPI
	{
		// DC for the desktop
		CWindowDC dc(NULL);

		int dpiX = dc.GetDeviceCaps(LOGPIXELSX);
		int dpiY = dc.GetDeviceCaps(LOGPIXELSY);

		CRect rc  = rcItem;
		rc.left   = MulDiv(rc.left, dpiX, 96);
		rc.right  = MulDiv(rc.right, dpiX, 96);
		rc.top    = MulDiv(rc.top, dpiY, 96);
		rc.bottom = MulDiv(rc.bottom, dpiY, 96);

		return rc;
	}

	inline CSize CMenuMetrics::ScaleSize(const CSize& szItem) const
	// Re-scale the CSize to support the system's DPI
	{
		// DC for the desktop
		CWindowDC dc(NULL);

		int dpiX = dc.GetDeviceCaps(LOGPIXELSX);
		int dpiY = dc.GetDeviceCaps(LOGPIXELSY);

		CSize sz = szItem;
		sz.cx = MulDiv(sz.cx, dpiX, 96);
		sz.cy = MulDiv(sz.cy, dpiY, 96);

		return sz;
	}

	inline int CMenuMetrics::ToItemStateId(UINT uItemState) const
	// Convert from item state to MENU_POPUPITEM state
	{
		const bool      IsDisabled   = ((uItemState & (ODS_INACTIVE | ODS_DISABLED)) != 0);
		const bool      IsHot        = ((uItemState & (ODS_HOTLIGHT | ODS_SELECTED)) != 0);
		POPUPITEMSTATES iState;

		if (IsDisabled)
			iState = (IsHot ? MPI_DISABLEDHOT : MPI_DISABLED);
		else if (IsHot)
			iState = MPI_HOT;
		else
			iState= MPI_NORMAL;

		return iState;
	}

	inline int CMenuMetrics::ToCheckBackgroundStateId(int iStateId) const
	// Convert to MENU_POPUPCHECKBACKGROUND
	{
		POPUPCHECKBACKGROUNDSTATES iStateIdCheckBackground;

		// Determine the check background state.
		if (iStateId == MPI_DISABLED || iStateId == MPI_DISABLEDHOT)
			iStateIdCheckBackground = MCB_DISABLED;
		else
			iStateIdCheckBackground = MCB_NORMAL;

		return iStateIdCheckBackground;
	}

	inline int CMenuMetrics::ToCheckStateId(UINT fType, int iStateId) const
	// Convert to MENU_POPUPCHECK state
	{
		POPUPCHECKSTATES iStateIdCheck;

		if (fType & MFT_RADIOCHECK)
		{
			if (iStateId == MPI_DISABLED || iStateId == MPI_DISABLEDHOT)
				iStateIdCheck = MC_BULLETDISABLED;
			else
				iStateIdCheck = MC_BULLETNORMAL;
		}
		else
		{
			if (iStateId == MPI_DISABLED || iStateId == MPI_DISABLEDHOT)
				iStateIdCheck = MC_CHECKMARKDISABLED;
			else
				iStateIdCheck = MC_CHECKMARKNORMAL;
		}

		return iStateIdCheck;
	}


	///////////////////////////////////
	// Definitions for the CFrame class
	//
	template <class T>
	inline CFrameT<T>::CFrameT() : m_AboutDialog(IDW_ABOUT), m_hAccel(0), m_pView(NULL), m_nMaxMRU(0), m_hOldFocus(0),
							  m_DrawArrowBkgrnd(FALSE), m_KbdHook(0), m_UseIndicatorStatus(TRUE),
							  m_UseMenuStatus(TRUE), m_UseThemes(TRUE), m_UseToolBar(TRUE)
	{
		ZeroMemory(&m_MBTheme, sizeof(m_MBTheme));
		ZeroMemory(&m_RBTheme, sizeof(m_RBTheme));
		ZeroMemory(&m_SBTheme, sizeof(m_SBTheme));
		ZeroMemory(&m_TBTheme, sizeof(m_TBTheme));

		m_strStatusText = LoadString(IDW_READY);

		// Load the common controls. Uses InitCommonControlsEx
		LoadCommonControls();

		// By default, we use the rebar if we can
		m_UseReBar = (GetComCtlVersion() > 470)? TRUE : FALSE;

		// Set the fonts
		NONCLIENTMETRICS info;
		ZeroMemory(&info, sizeof(NONCLIENTMETRICS));
		info.cbSize = GetSizeofNonClientMetrics();
		SystemParametersInfo (SPI_GETNONCLIENTMETRICS, sizeof(info), &info, 0);
		m_fntMenuBar.CreateFontIndirect(info.lfMenuFont);
		m_fntStatusBar.CreateFontIndirect(info.lfStatusFont);
	}

	template <class T>
	inline CFrameT<T>::~CFrameT()
	{
		if (m_KbdHook != 0) UnhookWindowsHookEx(m_KbdHook);
	}

	template <class T>
	inline BOOL CFrameT<T>::AddMenuIcon(int nID_MenuItem, HICON hIcon)
	// Adds an icon to an internal ImageList for use with popup menu items.
	// The image size for menu icons should be 16x16.
	{
		// Create a new ImageList if required
		if (NULL == m_imlMenu.GetHandle())
		{
			int cxImage = 16;
			int cyImage = 16;
			m_imlMenu.Create(cxImage, cyImage, ILC_COLOR32 | ILC_MASK, 1, 0);
			m_vMenuIcons.clear();
		}

		if (m_imlMenu.Add(hIcon) != -1)
		{
			m_vMenuIcons.push_back(nID_MenuItem);

			// Recreate the Disabled imagelist
			m_imlMenuDis.DeleteImageList();
			m_imlMenuDis.CreateDisabledImageList(m_imlMenu);

			return TRUE;
		}

		return FALSE;
	}

	template <class T>
	inline UINT CFrameT<T>::AddMenuIcons(const std::vector<UINT>& MenuData, COLORREF crMask, UINT BitmapID, UINT BitmapDisabledID)
	// Adds the icons from a bitmap resource to an internal ImageList for use with popup menu items.
	// Note:  Images for menu icons should be sized 16x16 or 16x15 pixels. Larger images are ignored.
	{
		// Count the MenuData entries excluding separators
		int iImages = 0;
		for (UINT i = 0 ; i < MenuData.size(); ++i)
		{
			if (MenuData[i] != 0)	// Don't count separators
			{
				++iImages;
			}
		}

		// Load the button images from Resource ID
		CBitmap Bitmap(BitmapID);

		if ((0 == iImages) || (!Bitmap))
			return static_cast<UINT>(m_vMenuIcons.size());	// No valid images, so nothing to do!

		BITMAP bm = Bitmap.GetBitmapData();
		int cxImage = bm.bmHeight;

		if (cxImage <= 16)		// images should be 16x16 or 16x15
		{
			cxImage = 16;
			int cyImage = 16;

			// Create the ImageList if required
			if (NULL == m_imlMenu.GetHandle())
			{
				m_imlMenu.Create(cxImage, cyImage, ILC_COLOR32 | ILC_MASK, iImages, 0);
				m_vMenuIcons.clear();
			}

			// Add the resource IDs to the m_vMenuIcons vector
			std::vector<UINT>::const_iterator iter;
			for (iter = MenuData.begin(); iter != MenuData.end(); ++iter)
			{
				if ((*iter) != 0)
				{
					m_vMenuIcons.push_back(*iter);
				}
			}

			// Add the images to the ImageList
			m_imlMenu.Add(Bitmap, crMask);

			// Create the Disabled imagelist
			if (BitmapDisabledID != 0)
			{
				m_imlMenuDis.DeleteImageList();
				m_imlMenuDis.Create(cxImage, cyImage, ILC_COLOR32 | ILC_MASK, iImages, 0);

				CBitmap BitmapDisabled(BitmapDisabledID);
				BITMAP bmDis = BitmapDisabled.GetBitmapData();
				int cyImageDis = bmDis.bmHeight;

				// Disabled icons must be 16x16 or 16x15
				if (cyImageDis <= 16)
				{
					m_imlMenuDis.Add(BitmapDisabled, crMask);
				}
				else
				{
					m_imlMenuDis.CreateDisabledImageList(m_imlMenu);
				}
			}
			else
			{
				m_imlMenuDis.DeleteImageList();
				m_imlMenuDis.CreateDisabledImageList(m_imlMenu);
			}

		}
		else
		{
			TRACE("AddMenuIcons: Failed to add images. Menu Icons should be 16x16 or 15x16");
			;;
		}

		// return the number of menu icons
		return static_cast<UINT>(m_vMenuIcons.size());
	}

	template <class T>
	inline void CFrameT<T>::AddMenuBarBand()
	// Adds a MenuBar to the rebar control
	{
		REBARBANDINFO rbbi;
		ZeroMemory(&rbbi, sizeof(REBARBANDINFO));

		rbbi.fMask      = RBBIM_STYLE | RBBIM_CHILD | RBBIM_ID;
		rbbi.fStyle     = RBBS_BREAK | RBBS_VARIABLEHEIGHT;
		rbbi.hwndChild  = GetMenuBar();
		rbbi.wID        = IDW_MENUBAR;

		// Note: rbbi.cbSize is set inside the InsertBand function
		GetReBar().InsertBand(-1, rbbi);
		GetReBar().SetMenuBar(GetMenuBar());

		if (GetReBarTheme().LockMenuBand)
			GetReBar().ShowGripper(GetReBar().GetBand(GetMenuBar()), FALSE);

		UpdateMenuBarBandSize();
	}

	template <class T>
	inline void CFrameT<T>::AddMRUEntry(LPCTSTR szMRUEntry)
	// Adds an entry to the Most Recently Used (MRU) list.
	{
		// Erase possible duplicate entries from vector
		RemoveMRUEntry(szMRUEntry);

		// Insert the entry at the beginning of the vector
		m_vMRUEntries.insert(m_vMRUEntries.begin(), szMRUEntry);

		// Delete excessive MRU entries
		if (m_vMRUEntries.size() > m_nMaxMRU)
			m_vMRUEntries.erase(m_vMRUEntries.begin() + m_nMaxMRU, m_vMRUEntries.end());

		UpdateMRUMenu();
	}

	template <class T>
	inline void CFrameT<T>::AddToolBarBand(CToolBar& TB, DWORD dwBandStyle, UINT nID)
	// Adds a ToolBar to the rebar control
	{
		// Create the ToolBar Window
		DWORD dwStyle = WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | TBSTYLE_TOOLTIPS | TBSTYLE_FLAT	| CCS_NODIVIDER | CCS_NORESIZE | CCS_NOPARENTALIGN;
		TB.CreateEx(0, TOOLBARCLASSNAME, 0, dwStyle, CRect(0,0,0,0), GetReBar(), 0);

		// Fill the REBARBAND structure
		REBARBANDINFO rbbi;
		ZeroMemory(&rbbi, sizeof(REBARBANDINFO));

		rbbi.fMask      = RBBIM_STYLE |  RBBIM_CHILD | RBBIM_ID;
		rbbi.fStyle     = dwBandStyle;
		rbbi.hwndChild  = TB;
		rbbi.wID        = nID;

		// Note: rbbi.cbSize is set inside the InsertBand function
		GetReBar().InsertBand(-1, rbbi);
	}

	template <class T>
	inline void CFrameT<T>::AddToolBarButton(UINT nID, BOOL IsEnabled /* = TRUE*/, LPCTSTR szText /* = 0 */, int iImage /* = -1 */)
	// Adds Resource IDs to toolbar buttons.
	// A resource ID of 0 is a separator
	{
		GetToolBarData().push_back(nID);

		GetToolBar().AddButton(nID, IsEnabled, iImage);

		if(0 != szText)
			GetToolBar().SetButtonText(nID, szText);
	}

	template <class T>
	inline void CFrameT<T>::AdjustFrameRect(const RECT& rcView)
	// Adjust the size of the frame to accommodate the View window's dimensions
	{
		// Adjust for the view styles
		CRect rc = rcView;
		DWORD dwStyle = (DWORD)GetView().GetWindowLongPtr(GWL_STYLE);
		DWORD dwExStyle = (DWORD)GetView().GetWindowLongPtr(GWL_EXSTYLE);
		AdjustWindowRectEx(&rc, dwStyle, FALSE, dwExStyle);

		// Calculate the new frame height
		CRect rcFrameBefore = T::GetWindowRect();
		CRect rcViewBefore = GetViewRect();
		int Height = rc.Height() + rcFrameBefore.Height() - rcViewBefore.Height();

		// Adjust for the frame styles
		dwStyle = (DWORD)T::GetWindowLongPtr(GWL_STYLE);
		dwExStyle = (DWORD)T::GetWindowLongPtr(GWL_EXSTYLE);
		AdjustWindowRectEx(&rc, dwStyle, FALSE, dwExStyle);

		// Calculate final rect size, and reposition frame
		T::SetWindowPos(NULL, 0, 0, rc.Width(), Height, SWP_NOMOVE);
	}

	template <class T>
	inline void CFrameT<T>::CreateToolBar()
	// Creates the frame's toolbar. Additional toolbars can be added with AddToolBarBand
	//  if the frame uses a rebar.
	{
		if (IsReBarSupported() && m_UseReBar)
			AddToolBarBand(GetToolBar(), RBBS_BREAK|RBBS_GRIPPERALWAYS, IDW_TOOLBAR);	// Create the toolbar inside rebar
		else
			GetToolBar().Create(*this);	// Create the toolbar without a rebar

		// Set a default ImageList for the ToolBar
		SetToolBarImages(RGB(192,192,192), IDW_MAIN, 0, 0);

		SetupToolBar();

		if (GetToolBarData().size() == 0)
		{
			TRACE("Warning ... No resource IDs assigned to the toolbar\n");
		}

		if (IsReBarSupported() && m_UseReBar)
		{
			SIZE MaxSize = GetToolBar().GetMaxSize();
			GetReBar().SendMessage(UWM_TBRESIZE, (WPARAM)GetToolBar().GetHwnd(), (LPARAM)&MaxSize);

			if (GetReBarTheme().UseThemes && GetReBarTheme().LockMenuBand)
			{
				// Hide gripper for single toolbar
				if (GetReBar().GetBandCount() <= 2)
					GetReBar().ShowGripper(GetReBar().GetBand(GetToolBar()), FALSE);
			}
		}

	}

	template <class T>
	inline LRESULT CFrameT<T>::CustomDrawMenuBar(NMHDR* pNMHDR)
	// CustomDraw is used to render the MenuBar's toolbar buttons
	{
		LPNMTBCUSTOMDRAW lpNMCustomDraw = (LPNMTBCUSTOMDRAW)pNMHDR;

		switch (lpNMCustomDraw->nmcd.dwDrawStage)
		{
		// Begin paint cycle
		case CDDS_PREPAINT:
			// Send NM_CUSTOMDRAW item draw, and post-paint notification messages.
			return CDRF_NOTIFYITEMDRAW | CDRF_NOTIFYPOSTPAINT ;

		// An item is about to be drawn
		case CDDS_ITEMPREPAINT:
			{
				CRect rcRect = lpNMCustomDraw->nmcd.rc;
				int nState = lpNMCustomDraw->nmcd.uItemState;
				DWORD dwItem = (DWORD)lpNMCustomDraw->nmcd.dwItemSpec;

				if (IsMDIChildMaxed() && (0 == dwItem))
				// Draw over MDI Max button
				{
					CDC dcDraw(lpNMCustomDraw->nmcd.hdc);
					CWnd* pActiveChild = GetMenuBar().GetActiveMDIChild();
					HICON hIcon = (HICON)pActiveChild->SendMessage(WM_GETICON, ICON_SMALL, 0L);
					if (NULL == hIcon)
						hIcon = GetApp().LoadStandardIcon(IDI_APPLICATION);

					int cx = ::GetSystemMetrics (SM_CXSMICON);
					int cy = ::GetSystemMetrics (SM_CYSMICON);
					int y = 1 + (GetMenuBar().GetWindowRect().Height() - cy)/2;
					int x = (rcRect.Width() - cx)/2;
					dcDraw.DrawIconEx(x, y, hIcon, cx, cy, 0, NULL, DI_NORMAL);

					return CDRF_SKIPDEFAULT;  // No further drawing
				}

				if (GetMenuBarTheme().UseThemes)
				{
					// Leave a pixel gap above and below the drawn rectangle
					if (IsVistaMenuUsed())
						rcRect.InflateRect(0, -2);
					else
						rcRect.InflateRect(0, -1);

					CDC dcDraw(lpNMCustomDraw->nmcd.hdc);
					if (nState & (CDIS_HOT | CDIS_SELECTED))
					{

						if ((nState & CDIS_SELECTED) || (GetMenuBar().GetButtonState(dwItem) & TBSTATE_PRESSED))
						{
							dcDraw.GradientFill(GetMenuBarTheme().clrPressed1, GetMenuBarTheme().clrPressed2, rcRect, FALSE);
						}
						else if (nState & CDIS_HOT)
						{
							dcDraw.GradientFill(GetMenuBarTheme().clrHot1, GetMenuBarTheme().clrHot2, rcRect, FALSE);
						}

						// Draw border
						CPen Pen(PS_SOLID, 1, GetMenuBarTheme().clrOutline);
						dcDraw.SelectObject(Pen);
						dcDraw.MoveTo(rcRect.left, rcRect.bottom);
						dcDraw.LineTo(rcRect.left, rcRect.top);
						dcDraw.LineTo(rcRect.right-1, rcRect.top);
						dcDraw.LineTo(rcRect.right-1, rcRect.bottom);
						dcDraw.MoveTo(rcRect.right-1, rcRect.bottom);
						dcDraw.LineTo(rcRect.left, rcRect.bottom);
					}

					CString str;
					int nLength = static_cast<int>(GetMenuBar().SendMessage(TB_GETBUTTONTEXT, lpNMCustomDraw->nmcd.dwItemSpec, 0L));
					if (nLength > 0)
					{
						GetMenuBar().SendMessage(TB_GETBUTTONTEXT, lpNMCustomDraw->nmcd.dwItemSpec, (LPARAM)str.GetBuffer(nLength));
						str.ReleaseBuffer();
					}

					// Draw highlight text
					CFont Font = GetMenuBar().GetFont();
					dcDraw.SelectObject(Font);

					rcRect.bottom += 1;
					dcDraw.SetBkMode(TRANSPARENT);
					dcDraw.DrawText(str, str.GetLength(), rcRect, DT_VCENTER | DT_CENTER | DT_SINGLELINE);

					return CDRF_SKIPDEFAULT;  // No further drawing
				}
			}
			return CDRF_DODEFAULT ;   // Do default drawing

		// Painting cycle has completed
		case CDDS_POSTPAINT:
			// Draw MDI Minimise, Restore and Close buttons
			{
				CDC dc(lpNMCustomDraw->nmcd.hdc);
				GetMenuBar().DrawAllMDIButtons(dc);
			}
			break;
		}

		return 0L;
	}

	template <class T>
	inline LRESULT CFrameT<T>::CustomDrawToolBar(NMHDR* pNMHDR)
	// With CustomDraw we manually control the drawing of each toolbar button
	{
		if ((GetToolBarTheme().UseThemes) && (GetComCtlVersion() > 470))
		{
			LPNMTBCUSTOMDRAW lpNMCustomDraw = (LPNMTBCUSTOMDRAW)pNMHDR;
			CToolBar* pTB = static_cast<CToolBar*>(T::GetCWndPtr(pNMHDR->hwndFrom));

			if (pTB)
			{
				switch (lpNMCustomDraw->nmcd.dwDrawStage)
				{
				// Begin paint cycle
				case CDDS_PREPAINT:
					// Send NM_CUSTOMDRAW item draw, and post-paint notification messages.
					return CDRF_NOTIFYITEMDRAW | CDRF_NOTIFYPOSTPAINT ;

				// An item is about to be drawn
				case CDDS_ITEMPREPAINT:
					{
						CDC dcDraw(lpNMCustomDraw->nmcd.hdc);
						CRect rcRect = lpNMCustomDraw->nmcd.rc;
						int nState = lpNMCustomDraw->nmcd.uItemState;
						DWORD dwItem = (DWORD)lpNMCustomDraw->nmcd.dwItemSpec;
						DWORD dwTBStyle = (DWORD)pTB->SendMessage(TB_GETSTYLE, 0L, 0L);
						int nStyle = pTB->GetButtonStyle(dwItem);

						int nButton = static_cast<int>(pTB->SendMessage(TB_COMMANDTOINDEX, (WPARAM) dwItem, 0L));
						TBBUTTON tbb;
						ZeroMemory(&tbb, sizeof(TBBUTTON));
						pTB->SendMessage(TB_GETBUTTON, (WPARAM)nButton, (LPARAM)&tbb);
						int iImage = (int)tbb.iBitmap;

						// Calculate text size
						CString str;
						CSize TextSize;
						if (pTB->HasText())	// Does any button have text?
						{
							dcDraw.SelectObject(pTB->GetFont());
							LRESULT lr = pTB->SendMessage(TB_GETBUTTONTEXT, dwItem, (LPARAM)str.GetBuffer(MAX_MENU_STRING));
							str.ReleaseBuffer();
							if (lr> 0)
							{
								TextSize = dcDraw.GetTextExtentPoint32(str, str.GetLength());
							}
						}


						// Draw outline rectangle
						if (nState & (CDIS_HOT | CDIS_SELECTED | CDIS_CHECKED))
						{
							dcDraw.CreatePen(PS_SOLID, 1, GetToolBarTheme().clrOutline);
							dcDraw.MoveTo(rcRect.left, rcRect.top);
							dcDraw.LineTo(rcRect.left, rcRect.bottom-1);
							dcDraw.LineTo(rcRect.right-1, rcRect.bottom-1);
							dcDraw.LineTo(rcRect.right-1, rcRect.top);
							dcDraw.LineTo(rcRect.left, rcRect.top);
						}

						// Draw filled gradient background
						rcRect.InflateRect(-1, -1);
						if ((nState & (CDIS_SELECTED|CDIS_CHECKED)) || (pTB->GetButtonState(dwItem) & TBSTATE_PRESSED))
						{
							dcDraw.GradientFill(GetToolBarTheme().clrPressed1, GetToolBarTheme().clrPressed2, rcRect, FALSE);
						}
						else if (nState & CDIS_HOT)
						{
							dcDraw.GradientFill(GetToolBarTheme().clrHot1, GetToolBarTheme().clrHot2, rcRect, FALSE);
						}

						// Get the appropriate image list depending on the button state
						CImageList imlToolBar;
						if (nState & CDIS_DISABLED)
						{
							imlToolBar = pTB->GetDisabledImageList();
						}
						else if (nState & (CDIS_HOT | CDIS_SELECTED | CDIS_CHECKED))
						{
							imlToolBar = pTB->GetHotImageList();
							if (imlToolBar.GetHandle() == 0)
								imlToolBar = pTB->GetImageList();
						}
						else
						{
							imlToolBar = pTB->GetImageList();
						}

						BOOL IsWin95 = (1400 == (GetWinVersion()) || (2400 == GetWinVersion()));

						// Calculate image position
						CSize szImage = imlToolBar.GetIconSize();

						int yImage = (rcRect.bottom + rcRect.top - szImage.cy - TextSize.cy )/2;
						int xImage = (rcRect.right + rcRect.left - szImage.cx)/2 + ((nState & (CDIS_SELECTED|CDIS_CHECKED))? 1:0);
						if (dwTBStyle & TBSTYLE_LIST)
						{
							xImage = rcRect.left + (IsXPThemed()?2:4) + ((nState & CDIS_SELECTED)? 1:0);
							yImage = (rcRect.bottom -rcRect.top - szImage.cy +2)/2 + ((nState & (CDIS_SELECTED|CDIS_CHECKED))? 1:0);
						}

						// Handle the TBSTYLE_DROPDOWN and BTNS_WHOLEDROPDOWN styles
						if ((nStyle & TBSTYLE_DROPDOWN) || ((nStyle & 0x0080) && (!IsWin95)))
						{
							// Calculate the dropdown arrow position
							int xAPos = (nStyle & TBSTYLE_DROPDOWN)? rcRect.right -6 : (rcRect.right + rcRect.left + szImage.cx + 4)/2;
							int yAPos = (nStyle & TBSTYLE_DROPDOWN)? (rcRect.bottom - rcRect.top +1)/2 : (szImage.cy)/2;
							if (dwTBStyle & TBSTYLE_LIST)
							{
								xAPos = (nStyle & TBSTYLE_DROPDOWN)?rcRect.right -6:rcRect.right -5;
								yAPos =	(rcRect.bottom - rcRect.top +1)/2 + ((nStyle & TBSTYLE_DROPDOWN)?0:1);
							}

							xImage -= (nStyle & TBSTYLE_DROPDOWN)?((dwTBStyle & TBSTYLE_LIST)? (IsXPThemed()?-4:0):6):((dwTBStyle & TBSTYLE_LIST)? 0:4);

							// Draw separate background for dropdown arrow
							if ((m_DrawArrowBkgrnd) && (nState & CDIS_HOT))
							{
								CRect rcArrowBkgnd = rcRect;
								rcArrowBkgnd.left = rcArrowBkgnd.right - 13;
								dcDraw.GradientFill(GetToolBarTheme().clrPressed1, GetToolBarTheme().clrPressed2, rcArrowBkgnd, FALSE);
							}

							m_DrawArrowBkgrnd = FALSE;

							// Manually draw the dropdown arrow
							dcDraw.CreatePen(PS_SOLID, 1, RGB(0,0,0));
							for (int i = 2; i >= 0; --i)
							{
								dcDraw.MoveTo(xAPos -i-1, yAPos - i+1);
								dcDraw.LineTo(xAPos +i,   yAPos - i+1);
							}

							// Draw line between icon and dropdown arrow
							if ((nStyle & TBSTYLE_DROPDOWN) && ((nState & CDIS_SELECTED) || nState & CDIS_HOT))
							{
								dcDraw.CreatePen(PS_SOLID, 1, GetToolBarTheme().clrOutline);
								dcDraw.MoveTo(rcRect.right - 13, rcRect.top);
								dcDraw.LineTo(rcRect.right - 13, rcRect.bottom);
							}
						}

						// Draw the button image
						if (xImage > 0)
						{
							imlToolBar.Draw(dcDraw, iImage, CPoint(xImage, yImage), ILD_TRANSPARENT);
						}

						//Draw Text
						if (!str.IsEmpty())
						{
							int iWidth = rcRect.right - rcRect.left - ((nStyle & TBSTYLE_DROPDOWN)?13:0);
							CRect rcText(0, 0, MIN(TextSize.cx, iWidth), TextSize.cy);

							int xOffset = (rcRect.right + rcRect.left - rcText.right + rcText.left - ((nStyle & TBSTYLE_DROPDOWN)? 11 : 1))/2;
							int yOffset = yImage + szImage.cy +1;

							if (dwTBStyle & TBSTYLE_LIST)
							{
								xOffset = rcRect.left + szImage.cx + ((nStyle & TBSTYLE_DROPDOWN)?(IsXPThemed()?10:6): 6) + ((nState & CDIS_SELECTED)? 1:0);
								yOffset = (2+rcRect.bottom - rcRect.top - rcText.bottom + rcText.top)/2 + ((nState & CDIS_SELECTED)? 1:0);
								rcText.right = MIN(rcText.right,  rcRect.right - xOffset);
							}

							OffsetRect(&rcText, xOffset, yOffset);

							int iMode = dcDraw.SetBkMode(TRANSPARENT);
							dcDraw.SelectObject(pTB->GetFont());

							if (nState & (CDIS_DISABLED))
							{
								// Draw text twice for embossed look
								rcText.OffsetRect(1, 1);
								dcDraw.SetTextColor(RGB(255,255,255));
								dcDraw.DrawText(str, str.GetLength(), rcText, DT_LEFT);
								rcText.OffsetRect(-1, -1);
								dcDraw.SetTextColor(GetSysColor(COLOR_GRAYTEXT));
								dcDraw.DrawText(str, str.GetLength(), rcText, DT_LEFT);
							}
							else
							{
								dcDraw.SetTextColor(GetSysColor(COLOR_BTNTEXT));
								dcDraw.DrawText(str, str.GetLength(), rcText, DT_LEFT | DT_END_ELLIPSIS);
							}
							dcDraw.SetBkMode(iMode);

						}
					}
					return CDRF_SKIPDEFAULT;  // No further drawing
				}
			}
		}
		return 0L;
	}

	template <class T>
	inline void CFrameT<T>::DrawMenuItem(LPDRAWITEMSTRUCT pdis)
	// Called by OnDrawItem to render the popup menu items.
	{
		MenuItemData* pmid = reinterpret_cast<MenuItemData*>(pdis->itemData);
		int iStateId = GetMenuMetrics().ToItemStateId(pdis->itemState);
		MenuTheme& MBT = GetMenuBarTheme();
		CDC dcDraw(pdis->hDC);

		if (IsVistaMenuUsed() && GetMenuMetrics().IsThemeBackgroundPartiallyTransparent(MENU_POPUPITEM, iStateId))
		{
			GetMenuMetrics().DrawThemeBackground(pdis->hDC, MENU_POPUPBACKGROUND, 0, &pdis->rcItem, NULL);
		}

		// Draw the gutter
		CRect rcGutter = GetMenuMetrics().GetGutterRect(pdis->rcItem);
		if (IsVistaMenuUsed())
			GetMenuMetrics().DrawThemeBackground(pdis->hDC, MENU_POPUPGUTTER, 0, &rcGutter, NULL);
		else
			dcDraw.GradientFill(MBT.clrPressed1, MBT.clrPressed2, rcGutter, TRUE);

		if (pmid->mii.fType & MFT_SEPARATOR)
		{
			// Draw the separator
			if (IsVistaMenuUsed())
			{
				CRect rcSeparator = GetMenuMetrics().GetSeperatorRect(pdis->rcItem);
				GetMenuMetrics().DrawThemeBackground(pdis->hDC, MENU_POPUPSEPARATOR, 0, &rcSeparator, NULL);
			}
			else
			{
				CRect rc = pdis->rcItem;
				CRect rcSep = pdis->rcItem;
				rcSep.left = GetMenuMetrics().GetGutterRect(rc).Width();
				dcDraw.SolidFill(RGB(255,255,255), rcSep);
				rcSep.top += (rc.bottom - rc.top)/2;
				dcDraw.DrawEdge(rcSep,  EDGE_ETCHED, BF_TOP);
			}
		}
		else
		{
			// Draw the item background
			DrawMenuItemBkgnd(pdis);

			// Draw Checkmark or icon
			if (pdis->itemState & ODS_CHECKED)
				DrawMenuItemCheckmark(pdis);
			else
				DrawMenuItemIcon(pdis);

			// Draw the text.
			DrawMenuItemText(pdis);
		}

		if (IsVistaMenuUsed())
		{
			// Draw the Submenu arrow
			if (pmid->mii.hSubMenu)
			{
				CRect rcSubMenu = pdis->rcItem;
				rcSubMenu.left = pdis->rcItem.right - GetMenuMetrics().m_marItem.cxRightWidth - GetMenuMetrics().m_marText.cxRightWidth;
				GetMenuMetrics().DrawThemeBackground(pdis->hDC, MENU_POPUPSUBMENU, GetMenuMetrics().ToCheckStateId(pmid->mii.fType, iStateId), &rcSubMenu, NULL);
			}

			// Suppress further drawing to prevent an incorrect Submenu arrow being drawn
			CRect rc = pdis->rcItem;
			dcDraw.ExcludeClipRect(rc);
		}

	}

	template <class T>
	inline void CFrameT<T>::DrawMenuItemBkgnd(LPDRAWITEMSTRUCT pdis)
	// Called by DrawMenuItem to render the popup menu background
	{
		// Draw the item background
		CRect rcSelection = GetMenuMetrics().GetSelectionRect(pdis->rcItem);
		if (IsVistaMenuUsed())
		{
			int iStateId = GetMenuMetrics().ToItemStateId(pdis->itemState);
			GetMenuMetrics().DrawThemeBackground(pdis->hDC, MENU_POPUPITEM, iStateId, &rcSelection, NULL);
		}
		else
		{
			BOOL IsDisabled = pdis->itemState & ODS_GRAYED;
			BOOL IsSelected = pdis->itemState & ODS_SELECTED;
			CRect rcDraw = pdis->rcItem;
			CDC dcDraw(pdis->hDC);
			MenuTheme& MBT = GetMenuBarTheme();

			if ((IsSelected) && (!IsDisabled))
			{
				// draw selected item background
				CBrush Brush(MBT.clrHot1);
				dcDraw.SelectObject(Brush);
				CPen Pen(PS_SOLID, 1, MBT.clrOutline);
				dcDraw.SelectObject(Pen);
				dcDraw.Rectangle(rcDraw.left, rcDraw.top, rcDraw.right, rcDraw.bottom);
			}
			else
			{
				// draw non-selected item background
				rcDraw.left = GetMenuMetrics().GetGutterRect(pdis->rcItem).Width();
				dcDraw.SolidFill(RGB(255,255,255), rcDraw);
			}
		}
	}

	template <class T>
	inline void CFrameT<T>::DrawMenuItemCheckmark(LPDRAWITEMSTRUCT pdis)
	// Draws the checkmark or radiocheck transparently
	{
		CRect rc = pdis->rcItem;
		MenuItemData* pmid = reinterpret_cast<MenuItemData*>(pdis->itemData);
		UINT fType = pmid->mii.fType;
		MenuTheme& MBT = GetMenuBarTheme();
		CRect rcBk;
		CDC dcDraw(pdis->hDC);

		if (IsVistaMenuUsed())
		{
			int iStateId = GetMenuMetrics().ToItemStateId(pdis->itemState);
			CRect rcCheckBackground = GetMenuMetrics().GetCheckBackgroundRect(pdis->rcItem);
			GetMenuMetrics().DrawThemeBackground(pdis->hDC, MENU_POPUPCHECKBACKGROUND, GetMenuMetrics().ToCheckBackgroundStateId(iStateId), &rcCheckBackground, NULL);

			CRect rcCheck = GetMenuMetrics().GetCheckRect(pdis->rcItem);
			GetMenuMetrics().DrawThemeBackground(pdis->hDC, MENU_POPUPCHECK, GetMenuMetrics().ToCheckStateId(pmid->mii.fType, iStateId), &rcCheck, NULL);
		}
		else
		{
			// Draw the checkmark's background rectangle first
			int Iconx = GetMenuMetrics().m_sizeCheck.cx;
			int Icony = GetMenuMetrics().m_sizeCheck.cy;
			int left = GetMenuMetrics().m_marCheck.cxLeftWidth;
			int top = rc.top + (rc.Height() - Icony) / 2;
			rcBk.SetRect(left, top, left + Iconx, top + Icony);

			CBrush Brush(MBT.clrHot2);
			dcDraw.SelectObject(Brush);
			CPen Pen(PS_SOLID, 1, MBT.clrOutline);
			dcDraw.SelectObject(Pen);

			// Draw the checkmark's background rectangle
			dcDraw.Rectangle(rcBk.left, rcBk.top, rcBk.right, rcBk.bottom);

			CMemDC dcMem(dcDraw);
			int cxCheck = ::GetSystemMetrics(SM_CXMENUCHECK);
			int cyCheck = ::GetSystemMetrics(SM_CYMENUCHECK);
			dcMem.CreateBitmap(cxCheck, cyCheck, 1, 1, NULL);
			CRect rcCheck(0, 0, cxCheck, cyCheck);

			// Copy the check mark bitmap to hdcMem
			if (MFT_RADIOCHECK == fType)
				dcMem.DrawFrameControl(rcCheck, DFC_MENU, DFCS_MENUBULLET);
			else
				dcMem.DrawFrameControl(rcCheck, DFC_MENU, DFCS_MENUCHECK);

			int xoffset = (rcBk.Width() - rcCheck.Width()  +1) / 2;
			int yoffset = (rcBk.Height() - rcCheck.Height() +1) / 2;
			if (MFT_RADIOCHECK != fType) yoffset--;

			// Draw a white or black check mark as required
			// Unfortunately MaskBlt isn't supported on Win95, 98 or ME, so we do it the hard way
			CMemDC MaskDC(dcDraw);
			MaskDC.CreateCompatibleBitmap(dcDraw, cxCheck, cyCheck);
			MaskDC.BitBlt(0, 0, cxCheck, cyCheck, MaskDC, 0, 0, WHITENESS);

			if ((pdis->itemState & ODS_SELECTED))
			{
				// Draw a white checkmark
				dcMem.BitBlt(0, 0, cxCheck, cyCheck, dcMem, 0, 0, DSTINVERT);
				MaskDC.BitBlt(0, 0, cxCheck, cyCheck, dcMem, 0, 0, SRCAND);
				dcDraw.BitBlt(rcBk.left + xoffset, rcBk.top + yoffset, cxCheck, cyCheck, MaskDC, 0, 0, SRCPAINT);
			}
			else
			{
				// Draw a black checkmark
				MaskDC.BitBlt(0, 0, cxCheck, cyCheck, dcMem, 0, 0, SRCAND);
				dcDraw.BitBlt(rcBk.left + xoffset, rcBk.top + yoffset, cxCheck, cyCheck, MaskDC, 0, 0, SRCAND);
			}
		}
	}

	template <class T>
	inline void CFrameT<T>::DrawMenuItemIcon(LPDRAWITEMSTRUCT pdis)
	// Called by DrawMenuItem to draw icons in popup menus
	{
		if ( 0 == m_imlMenu.GetHandle() )
			return;

		// Get icon size
		int Iconx = GetMenuMetrics().m_sizeCheck.cx;
		int Icony = GetMenuMetrics().m_sizeCheck.cy;

		// get the drawing rectangle
		CRect rc = pdis->rcItem;
		int left = GetMenuMetrics().m_marCheck.cxLeftWidth;
		int top = rc.top + (rc.Height() - Icony)/2;
		rc.SetRect(left, top, left + Iconx, top + Icony);

		// get the icon's location in the imagelist
		int iImage = -1;
		for (int i = 0 ; i < (int)m_vMenuIcons.size(); ++i)
		{
			if (pdis->itemID == m_vMenuIcons[i])
				iImage = i;
		}

		// draw the image
		if (iImage >= 0 )
		{
			BOOL IsDisabled = pdis->itemState & ODS_GRAYED;
			if ((IsDisabled) && (m_imlMenuDis.GetHandle()))
				m_imlMenuDis.Draw(pdis->hDC, iImage, CPoint(rc.left, rc.top), ILD_TRANSPARENT);
			else
				m_imlMenu.Draw(pdis->hDC, iImage, CPoint(rc.left, rc.top), ILD_TRANSPARENT);
		}
	}

	template <class T>
	inline void CFrameT<T>::DrawMenuItemText(LPDRAWITEMSTRUCT pdis)
	// Called by DrawMenuItem to render the text for popup menus.
	{
		MenuItemData* pmid = reinterpret_cast<MenuItemData*>(pdis->itemData);
		LPCTSTR ItemText = pmid->GetItemText();
		BOOL IsDisabled = pdis->itemState & ODS_GRAYED;
		COLORREF colorText = GetSysColor(IsDisabled ?  COLOR_GRAYTEXT : COLOR_MENUTEXT);

		// Calculate the text rect size
		CRect rcText = GetMenuMetrics().GetTextRect(pdis->rcItem);

		// find the position of tab character
		int nTab = -1;
		int len = lstrlen(ItemText);
		for(int i = 0; i < len; ++i)
		{
			if(_T('\t') == ItemText[i])
			{
				nTab = i;
				break;
			}
		}

		// Draw the item text
		if (IsVistaMenuUsed())
		{
			ULONG uAccel = ((pdis->itemState & ODS_NOACCEL) ? DT_HIDEPREFIX : 0);
			int iStateId = GetMenuMetrics().ToItemStateId(pdis->itemState);

			// Draw the item text before the tab
			GetMenuMetrics().DrawThemeText(pdis->hDC, MENU_POPUPITEM, iStateId, TtoW(ItemText), nTab, DT_SINGLELINE | DT_LEFT | DT_VCENTER | uAccel, 0, &rcText);

			// Draw text after tab, right aligned
			if(nTab != -1)
				GetMenuMetrics().DrawThemeText(pdis->hDC, MENU_POPUPITEM, iStateId, TtoW(&ItemText[nTab + 1]), -1, DT_SINGLELINE | DT_RIGHT | DT_VCENTER | uAccel, 0, &rcText);
		}
		else
		{
			SetTextColor(pdis->hDC, colorText);
			int iMode = SetBkMode(pdis->hDC, TRANSPARENT);
			DrawText(pdis->hDC, ItemText, nTab, rcText, DT_SINGLELINE | DT_LEFT | DT_VCENTER);

			// Draw text after tab, right aligned
			if(nTab != -1)
				DrawText(pdis->hDC, &ItemText[nTab + 1], -1, rcText, DT_SINGLELINE | DT_RIGHT | DT_VCENTER);

			SetBkMode(pdis->hDC, iMode);
		}
	}

	template <class T>
	inline BOOL CFrameT<T>::DrawReBarBkgnd(CDC& dc, CReBar& ReBar)
	// Draws the ReBar's background when ReBar themes are enabled.
	// Returns TRUE when the default background drawing is suppressed.
	{
		BOOL IsDrawn = TRUE;

		ReBarTheme& RBTheme = GetReBarTheme();
		if (!RBTheme.UseThemes)
			IsDrawn = FALSE;

		if (!RBTheme.clrBkgnd1 && !RBTheme.clrBkgnd2 && !RBTheme.clrBand1 && !RBTheme.clrBand2)
			IsDrawn = FALSE;

		if (IsDrawn)
		{
			assert(ReBar.IsWindow());

			BOOL IsVertical = ReBar.GetWindowLongPtr(GWL_STYLE) & CCS_VERT;

			// Create our memory DC
			CRect rcReBar = ReBar.GetClientRect();
			CMemDC dcMem(dc);
			dcMem.CreateCompatibleBitmap(dc, rcReBar.Width(), rcReBar.Height());

			// Draw to ReBar background to the memory DC
			dcMem.SolidFill(RBTheme.clrBkgnd2, rcReBar);
			CRect rcBkGnd = rcReBar;
			rcBkGnd.right = 600;
			dcMem.GradientFill(RBTheme.clrBkgnd1, RBTheme.clrBkgnd2, rcReBar, TRUE);

			if (RBTheme.clrBand1 || RBTheme.clrBand2)
			{
				// Draw the individual band backgrounds
				for (int nBand = 0 ; nBand < ReBar.GetBandCount(); ++nBand)
				{
					if (ReBar.IsBandVisible(nBand))
					{
						if (nBand != ReBar.GetBand(GetMenuBar()))
						{
							// Determine the size of this band
							CRect rcBand = ReBar.GetBandRect(nBand);

							if (IsVertical)
							{
								int right = rcBand.right;
								rcBand.right = rcBand.bottom;
								rcBand.bottom = right;
							}

							// Determine the size of the child window
							REBARBANDINFO rbbi;
							ZeroMemory(&rbbi, sizeof(REBARBANDINFO));
							rbbi.fMask = RBBIM_CHILD ;
							ReBar.GetBandInfo(nBand, rbbi);
							CRect rcChild;
							::GetWindowRect(rbbi.hwndChild, &rcChild);
							T::ScreenToClient(rcChild);

							// Determine our drawing rectangle
							int StartPad = IsXPThemed()? 2: 0;							CRect rcDraw = rcBand;
							CRect rcBorders = ReBar.GetBandBorders(nBand);
							if (IsVertical)
							{
								rcDraw.bottom = rcDraw.top + rcChild.Height() + rcBorders.top;
								rcDraw.top -= StartPad;
							}
							else
							{
								rcDraw.right = rcDraw.left + rcChild.Width() + rcBorders.left;
								rcDraw.left -= StartPad;
							}

							if (!RBTheme.FlatStyle)
								::InflateRect(&rcDraw, 1, 1);

							// Fill the Source CDC with the band's background
							CMemDC SourceDC(dc);
							SourceDC.CreateCompatibleBitmap(dc, rcReBar.Width(), rcReBar.Height());
							SourceDC.GradientFill(RBTheme.clrBand1, RBTheme.clrBand2, rcDraw, IsVertical);

							// Set Curve amount for rounded edges
							int Curve = RBTheme.RoundBorders? 12 : 0;

							// Create our mask for rounded edges using RoundRect
							CMemDC MaskDC(dc);
							MaskDC.CreateCompatibleBitmap(dc, rcReBar.Width(), rcReBar.Height());

							int left = rcDraw.left;
							int right = rcDraw.right;
							int top = rcDraw.top;
							int bottom = rcDraw.bottom;
							int cx = rcDraw.Width();// + xPad;
							int cy = rcDraw.Height();

							if (RBTheme.FlatStyle)
							{
								MaskDC.SolidFill(RGB(0,0,0), rcDraw);
								MaskDC.BitBlt(left, top, cx, cy, MaskDC, left, top, PATINVERT);
								MaskDC.RoundRect(left, top, right, bottom, Curve, Curve);
							}
							else
							{
								MaskDC.SolidFill(RGB(0,0,0), rcDraw);
								MaskDC.RoundRect(left, top, right, bottom, Curve, Curve);
								MaskDC.BitBlt(left, top, cx, cy, MaskDC, left, top, PATINVERT);
							}

							// Copy Source DC to Memory DC using the RoundRect mask
							dcMem.BitBlt(left, top, cx, cy, SourceDC, left, top, SRCINVERT);
							dcMem.BitBlt(left, top, cx, cy, MaskDC,   left, top, SRCAND);
							dcMem.BitBlt(left, top, cx, cy, SourceDC, left, top, SRCINVERT);
						}
					}
				}
			}

			if (RBTheme.UseLines)
			{
				// Draw lines between bands
				for (int j = 0; j < ReBar.GetBandCount()-1; ++j)
				{
					CRect rcRand = ReBar.GetBandRect(j);
					if (IsVertical)
					{
						rcRand.top = MAX(0, rcReBar.top - 4);
						rcRand.right +=2;
					}
					else
					{
						rcRand.left = MAX(0, rcReBar.left - 4);
						rcRand.bottom +=2;
					}
					dcMem.DrawEdge(rcRand, EDGE_ETCHED, BF_BOTTOM | BF_ADJUST);
				}
			}

			// Copy the Memory DC to the window's DC
			dc.BitBlt(0, 0, rcReBar.Width(), rcReBar.Height(), dcMem, 0, 0, SRCCOPY);
		}

		return IsDrawn;
	}

	template <class T>
	inline BOOL CFrameT<T>::DrawStatusBarBkgnd(CDC& dc, CStatusBar& StatusBar)
	// Draws the StatusBar's background when StatusBar themes are enabled.
	// Returns TRUE when the default background drawing is suppressed.
	{
		BOOL IsDrawn = FALSE;

		// XP Themes are required to modify the statusbar's background
		if (IsXPThemed())
		{
			StatusBarTheme& SBTheme = GetStatusBarTheme();
			if (SBTheme.UseThemes)
			{
				// Fill the background with a color gradient
				dc.GradientFill(SBTheme.clrBkgnd1, SBTheme.clrBkgnd2, StatusBar.GetClientRect(), TRUE);
				IsDrawn = TRUE;
			}
		}

		return IsDrawn;
	}

	template <class T>
	inline CRect CFrameT<T>::ExcludeChildRect(CRect& rcClient, HWND hChild) const
	// Calculates the remaining client rect when a child window is excluded.
	// Note: Assumes the child window touches 3 of the client rect's borders
	//  e.g.   CRect rc = ExcludeChildRect(GetClientRect(), GetStatusBar());
	{
		T::ClientToScreen(rcClient);

		CRect rcChildWindow;
		::GetWindowRect(hChild, &rcChildWindow);

		if (rcClient.Width() == rcChildWindow.Width())
		{
			if (rcClient.top == rcChildWindow.top)
				rcClient.top += rcChildWindow.Height();
			else
				rcClient.bottom -= rcChildWindow.Height();
		}
		else
		{
			if (rcClient.left == rcChildWindow.left)
				rcClient.left += rcChildWindow.Width();
			else
				rcClient.right -= rcChildWindow.Width();
		}

		T::ScreenToClient(rcClient);

		return rcClient;
	}

	template <class T>
	inline int CFrameT<T>::GetMenuItemPos(HMENU hMenu, LPCTSTR szItem)
	// Returns the position of the menu item, given it's name
	{
		int nMenuItemCount = ::GetMenuItemCount(hMenu);
		MENUITEMINFO mii;
		ZeroMemory(&mii, sizeof(MENUITEMINFO));
		mii.cbSize = GetSizeofMenuItemInfo();

		for (int nItem = 0 ; nItem < nMenuItemCount; ++nItem)
		{
			std::vector<TCHAR> vTChar( MAX_MENU_STRING+1, _T('\0') );
			TCHAR* szStr = &vTChar[0];

			std::vector<TCHAR> vStripped( MAX_MENU_STRING+1, _T('\0') );
			TCHAR* szStripped = &vStripped[0];

			mii.fMask      = MIIM_TYPE;
			mii.fType      = MFT_STRING;
			mii.dwTypeData = szStr;
			mii.cch        = MAX_MENU_STRING;

			// Fill the contents of szStr from the menu item
			if (::GetMenuItemInfo(hMenu, nItem, TRUE, &mii))
			{
				int len = lstrlen(szStr);
				if (len <= MAX_MENU_STRING)
				{
					// Strip out any & characters
					int j = 0;
					for (int i = 0; i < len; ++i)
					{
						if (szStr[i] != _T('&'))
							szStripped[j++] = szStr[i];
					}
					szStripped[j] = _T('\0');	// Append null tchar

					// Compare the strings
					if (lstrcmp(szStripped, szItem) == 0)
						return nItem;
				}
			}
		}

		return -1;
	}

	template <class T>
	inline CString CFrameT<T>::GetMRUEntry(UINT nIndex)
	// Returns a MRU entry given its index.
	{
		CString strPathName;
		if (nIndex < m_vMRUEntries.size())
		{
			strPathName = m_vMRUEntries[nIndex];

			// Now put the selected entry at Index 0
			AddMRUEntry(strPathName);
		}
		return strPathName;
	}

	template <class T>
	inline CSize CFrameT<T>::GetTBImageSize(CBitmap* pbm)
	// Returns the size of a bitmap image
	{
		assert(pbm);
		assert(pbm->GetHandle());
		BITMAP bmData = pbm->GetBitmapData();
		int cy = bmData.bmHeight;
		int cx = MAX(bmData.bmHeight, 16);

		return CSize(cx, cy);
	}

	template <class T>
	inline CRect CFrameT<T>::GetViewRect() const
	// Returns the dimensions of the view window.
	{
		CRect rcClient = T::GetClientRect();

		if (GetStatusBar().IsWindow() && GetStatusBar().IsWindowVisible())
			rcClient = ExcludeChildRect(rcClient, GetStatusBar());

		if (GetReBar().IsWindow() && GetReBar().IsWindowVisible())
			rcClient = ExcludeChildRect(rcClient, GetReBar());
		else
			if (GetToolBar().IsWindow() && GetToolBar().IsWindowVisible())
				rcClient = ExcludeChildRect(rcClient, GetToolBar());

		return rcClient;
	}

	template <class T>
	inline CString CFrameT<T>::GetThemeName() const
	// Returns the XP theme name
	{
		HMODULE hMod = ::LoadLibrary(_T("uxtheme.dll"));
		WCHAR ThemeName[31] = L"";
		if(hMod != 0)
		{
			typedef HRESULT (__stdcall *PFNGETCURRENTTHEMENAME)(LPWSTR pszThemeFileName, int cchMaxNameChars,
				LPWSTR pszColorBuff, int cchMaxColorChars, LPWSTR pszSizeBuff, int cchMaxSizeChars);

			PFNGETCURRENTTHEMENAME pfn = (PFNGETCURRENTTHEMENAME)GetProcAddress(hMod, "GetCurrentThemeName");
			pfn(0, 0, ThemeName, 30, 0, 0);

			::FreeLibrary(hMod);
		}

		return CString(ThemeName);
	}

	template <class T>
	inline BOOL CFrameT<T>::LoadRegistryMRUSettings(UINT nMaxMRU /*= 0*/)
	// Load the MRU list from the registry
	{
		assert(!m_strKeyName.IsEmpty()); // KeyName must be set before calling LoadRegistryMRUSettings

		CRegKey Key;
		BOOL SettingsLoaded = FALSE;
		SetMRULimit(nMaxMRU);
		std::vector<CString> vMRUEntries;
		CString strKey = _T("Software\\") + m_strKeyName + _T("\\Recent Files");

		if (ERROR_SUCCESS == Key.Open(HKEY_CURRENT_USER, strKey, KEY_READ))
		{
			CString PathName;
			CString SubKeyName;
			for (UINT i = 0; i < m_nMaxMRU; ++i)
			{
				DWORD dwBufferSize = 0;
				SubKeyName.Format(_T("File %d"), i+1);

				if (ERROR_SUCCESS == Key.QueryStringValue(SubKeyName, NULL, &dwBufferSize))
				{
					// load the entry from the registry
					if (ERROR_SUCCESS == Key.QueryStringValue(SubKeyName, PathName.GetBuffer(dwBufferSize), &dwBufferSize))
					{
						PathName.ReleaseBuffer();

						if (PathName.GetLength() > 0)
							vMRUEntries.push_back( PathName );
					}
					else
					{
						PathName.ReleaseBuffer();
						TRACE(_T("LoadRegistryMRUSettings: RegQueryValueEx failed\n"));
					}
				}
			}

			// successfully loaded all MRU values, so store them
			m_vMRUEntries = vMRUEntries;
			SettingsLoaded = TRUE;
		}

		return SettingsLoaded;
	}

	template <class T>
	inline BOOL CFrameT<T>::LoadRegistrySettings(LPCTSTR szKeyName)
	// Loads various frame settings from the registry
	{
		assert (NULL != szKeyName);

		m_strKeyName = szKeyName;
		CString strKey = _T("Software\\") + m_strKeyName + _T("\\Frame Settings");
		BOOL isOK = FALSE;
		InitValues Values;
		CRegKey Key;
		if (ERROR_SUCCESS == Key.Open(HKEY_CURRENT_USER, strKey, KEY_READ))
		{
			try
			{
				DWORD dwTop, dwLeft, dwWidth, dwHeight, dwShowCmd, dwStatusBar, dwToolBar;

				if (ERROR_SUCCESS != Key.QueryDWORDValue(_T("Top"), dwTop))
					throw CUserException(_T("RegQueryValueEx Failed"));
				if (ERROR_SUCCESS != Key.QueryDWORDValue(_T("Left"), dwLeft))
					throw CUserException(_T("RegQueryValueEx Failed"));
				if (ERROR_SUCCESS != Key.QueryDWORDValue(_T("Width"), dwWidth))
					throw CUserException(_T("RegQueryValueEx Failed"));
				if (ERROR_SUCCESS != Key.QueryDWORDValue(_T("Height"), dwHeight))
					throw CUserException(_T("RegQueryValueEx Failed"));
				if (ERROR_SUCCESS != Key.QueryDWORDValue(_T("ShowCmd"), dwShowCmd))
					throw CUserException(_T("RegQueryValueEx Failed"));
				if (ERROR_SUCCESS != Key.QueryDWORDValue(_T("StatusBar"), dwStatusBar))
					throw CUserException(_T("RegQueryValueEx Failed"));
				if (ERROR_SUCCESS != Key.QueryDWORDValue(_T("ToolBar"), dwToolBar))
					throw CUserException(_T("RegQueryValueEx Failed"));

				Values.rcPos = CRect(dwLeft, dwTop, dwLeft + dwWidth, dwTop + dwHeight);
				Values.ShowCmd = (SW_MAXIMIZE == dwShowCmd) ? SW_MAXIMIZE : SW_SHOW;
				Values.ShowStatusBar = dwStatusBar & 1;
				Values.ShowToolBar = dwToolBar & 1;

				isOK = TRUE;
			}

			catch (const CUserException& e)
			{
				Trace("*** Failed to load values from registry, using defaults. ***\n");
				Trace(e.GetText()); TRACE(strKey); Trace("\n");

				// Delete the bad key from the registry
				CString strParentKey = _T("Software\\") + m_strKeyName;
				CRegKey ParentKey;
				if (ERROR_SUCCESS == ParentKey.Open(HKEY_CURRENT_USER, strParentKey, KEY_READ))
					ParentKey.DeleteSubKey(_T("Frame Settings"));

				InitValues DefaultValues;
				Values = DefaultValues;
			}
		}

		SetInitValues(Values);
		return isOK;
	}

	template <class T>
	inline void CFrameT<T>::MeasureMenuItem(MEASUREITEMSTRUCT *pmis)
	// Determines the size of the popup menus
	{
		MenuItemData* pmid = reinterpret_cast<MenuItemData*>(pmis->itemData);
		assert(::IsMenu(pmid->hMenu));	// Does itemData contain a valid MenuItemData struct?

		CSize size = GetMenuMetrics().GetItemSize(pmid);

		// Return the composite sizes.
		pmis->itemWidth = size.cx;
		pmis->itemHeight = size.cy;
	}

	template <class T>
	inline LRESULT CFrameT<T>::OnActivate(UINT, WPARAM wParam, LPARAM lParam)
	// Called when the frame is activated (WM_ACTIVATE received)
	{
		// Perform default processing first
		CWnd::WndProcDefault(WM_ACTIVATE, wParam, lParam);


		if (LOWORD(wParam) == WA_INACTIVE)
		{
			// Save the hwnd of the window which currently has focus
			// (this must be CFrame window itself or a child window
			if (!T::IsIconic()) m_hOldFocus = ::GetFocus();
		}
		else
		{
			// Now set the focus to the appropriate child window
			if (m_hOldFocus != 0) ::SetFocus(m_hOldFocus);
		}

		// Update DockClient captions
		T::PostMessage(UWM_DOCKACTIVATE);


		// Also update DockClient captions if the view is a docker
		if (GetView().IsWindow() && GetView().SendMessage(UWM_GETCDOCKER))
			GetView().PostMessage(UWM_DOCKACTIVATE);

		return 0L;
	}

	template <class T>
	inline void CFrameT<T>::OnClose()
	// Called in response to a WM_CLOSE message for the frame.
	{
		SaveRegistrySettings();
		T::ShowWindow(SW_HIDE);
		T::Destroy();
	}

	template <class T>
	inline int CFrameT<T>::OnCreate(CREATESTRUCT& cs)
	// This is called when the frame window is being created.
	// Override this in CMainFrame if you wish to modify what happens here
	{
		UNREFERENCED_PARAMETER(cs);

		// Start the keyboard hook
		TLSData* pTLSData = GetApp().SetTlsData();
		pTLSData->hMainWnd = T::GetHwnd();
		m_KbdHook = ::SetWindowsHookEx(WH_KEYBOARD, StaticKeyboardProc, NULL, ::GetCurrentThreadId());

		// Set the icon
		T::SetIconLarge(IDW_MAIN);
		T::SetIconSmall(IDW_MAIN);

		// Set the keyboard accelerators
		SetAccelerators(IDW_MAIN);

		// Set the Caption
		T::SetWindowText(LoadString(IDW_MAIN));

		// Set the theme for the frame elements
		SetTheme();

		// Create the rebar and menubar
		if (IsReBarSupported() && m_UseReBar)
		{
			// Create the rebar
			GetReBar().Create(*this);

			// Create the menubar inside rebar
			GetMenuBar().Create(GetReBar());
			AddMenuBarBand();

			// Disable XP themes for the menubar
			GetMenuBar().SetWindowTheme(L" ", L" ");
		}

		// Setup the menu
		SetFrameMenu(IDW_MAIN);
		if (m_nMaxMRU > 0)
			UpdateMRUMenu();

		// Initialize the menu metrics
		GetMenuMetrics().m_hFrame = *this;
		GetMenuMetrics().Initialize();

		// Create the ToolBar
		if (m_UseToolBar)
		{
			CreateToolBar();
			ShowToolBar(GetInitValues().ShowToolBar);
		}
		else
		{
			GetFrameMenu().EnableMenuItem(IDW_VIEW_TOOLBAR, MF_GRAYED);
		}

		SetupMenuIcons();

		// Create the status bar
		GetStatusBar().Create(*this);
		GetStatusBar().SetFont(m_fntStatusBar, FALSE);
		ShowStatusBar(GetInitValues().ShowStatusBar);

		if (m_UseIndicatorStatus)
			SetStatusIndicators();

		// Create the view window
		assert(&GetView());			// Use SetView in CMainFrame's constructor to set the view window

		if (!GetView().IsWindow())
			GetView().Create(*this);
		GetView().SetFocus();

		// Adjust fonts to match the desktop theme
		T::SendMessage(WM_SYSCOLORCHANGE);

		// Reposition the child windows
		RecalcLayout();

		return 0;
	}

	template <class T>
	inline LRESULT CFrameT<T>::OnCustomDraw(LPNMHDR pNMHDR)
	// Handles CustomDraw notification from WM_NOTIFY.
	{
		if ( ::SendMessage(pNMHDR->hwndFrom, UWM_GETCTOOLBAR, 0, 0) )
		{
			if (pNMHDR->hwndFrom == GetMenuBar())
				return CustomDrawMenuBar(pNMHDR);
			else
				return CustomDrawToolBar(pNMHDR);
		}

		return 0L;
	}

	template <class T>
	inline void CFrameT<T>::OnDestroy()
	// Called when the frame is about to be destroyed (WM_DESTROY received)
	{
		GetMenuBar().Destroy();
		GetToolBar().Destroy();
		GetReBar().Destroy();
		GetStatusBar().Destroy();
		GetView().Destroy();

		::PostQuitMessage(0);	// Terminates the application
	}

	template <class T>
	inline LRESULT CFrameT<T>::OnDrawItem(UINT, WPARAM wParam, LPARAM lParam)
	// OwnerDraw is used to render the popup menu items
	{
		LPDRAWITEMSTRUCT pdis = (LPDRAWITEMSTRUCT) lParam;
		if (pdis->CtlType != ODT_MENU)
			return CWnd::WndProcDefault(WM_DRAWITEM, wParam, lParam);

		if (!IsRectEmpty(&pdis->rcItem))
			DrawMenuItem(pdis);

		return TRUE;
	}

	template <class T>
	inline BOOL CFrameT<T>::OnHelp()
	// Called to display help (WM_HELP received or selected via menu)
	{
		// Ensure only one dialog displayed even for multiple hits of the F1 button
		if (!m_AboutDialog.IsWindow())
		{
			// Store the window handle that currently has keyboard focus
			HWND hPrevFocus = ::GetFocus();
			if (hPrevFocus == GetMenuBar().GetHwnd())
				hPrevFocus = T::GetHwnd();

			m_AboutDialog.DoModal(*this);

			::SetFocus(hPrevFocus);
		}

		return TRUE;
	}

	template <class T>
	inline LRESULT CFrameT<T>::OnInitMenuPopup(UINT, WPARAM wParam, LPARAM lParam)
	// Called when the menu's modal loop begins (WM_INITMENUPOPUP received)
	{
		// The system menu shouldn't be owner drawn
		if (HIWORD(lParam))
			return CWnd::WndProcDefault(WM_INITMENUPOPUP, wParam, lParam);

		// Not supported on Win95 or WinNT
		if ((GetWinVersion() == 1400) || (GetWinVersion() == 2400))
			return CWnd::WndProcDefault(WM_INITMENUPOPUP, wParam, lParam);

		CMenu Menu((HMENU)wParam);

		for (UINT i = 0; i < Menu.GetMenuItemCount(); ++i)
		{
			MenuItemData* pItem = new MenuItemData;			// deleted in OnExitMenuLoop
			m_vMenuItemData.push_back(ItemDataPtr(pItem));	// Store pItem in smart pointer for later automatic deletion

			MENUITEMINFO mii;
			ZeroMemory(&mii, sizeof(MENUITEMINFO));
			mii.cbSize = GetSizeofMenuItemInfo();

			// Use old fashioned MIIM_TYPE instead of MIIM_FTYPE for MS VC6 compatibility
			mii.fMask = MIIM_STATE | MIIM_ID | MIIM_SUBMENU |MIIM_CHECKMARKS | MIIM_TYPE | MIIM_DATA;
			mii.dwTypeData = pItem->GetItemText();	// assign TCHAR pointer, text is assigned by GetMenuItemInfo
			mii.cch = MAX_MENU_STRING;

			// Send message for menu updates
			UINT menuItem = Menu.GetMenuItemID(i);
			T::SendMessage(UWM_UPDATECOMMAND, (WPARAM)menuItem, 0L);

			// Specify owner-draw for the menu item type
			if (Menu.GetMenuItemInfo(i, mii, TRUE))
			{
				if (mii.dwItemData == 0)
				{
					pItem->hMenu = Menu;
					pItem->nPos = i;
					pItem->mii = mii;
					mii.dwItemData = (DWORD_PTR)pItem;
					mii.fType |= MFT_OWNERDRAW;
					Menu.SetMenuItemInfo(i, mii, TRUE); // Store pItem in mii
				}
			}
		}

		return 0L;
	}

	template <class T>
	inline LRESULT CFrameT<T>::OnMeasureItem(UINT, WPARAM wParam, LPARAM lParam)
	// Called before the Popup menu is displayed, so that the MEASUREITEMSTRUCT
	//  values can be assigned with the menu item's dimensions.
	{
		LPMEASUREITEMSTRUCT pmis = (LPMEASUREITEMSTRUCT) lParam;
		if (pmis->CtlType != ODT_MENU)
			return CWnd::WndProcDefault(WM_MEASUREITEM, wParam, lParam);

		MeasureMenuItem(pmis);
		return TRUE;
	}

	template <class T>
	inline LRESULT CFrameT<T>::OnMenuChar(UINT, WPARAM wParam, LPARAM lParam)
	// Called when a menu is active, and a key is pressed other than an accelerator.
	{
		if ((IsMenuBarUsed()) && (LOWORD(wParam)!= VK_SPACE))
		{
			// Activate MenuBar for key pressed with Alt key held down
			GetMenuBar().OnMenuChar(WM_MENUCHAR, wParam, lParam);
			return -1L;
		}

		return T::FinalWindowProc(WM_MENUCHAR, wParam, lParam);
	}

	template <class T>
	inline LRESULT CFrameT<T>::OnMenuSelect(UINT, WPARAM wParam, LPARAM lParam)
	// Called when a menu item is selected.
	{
		// Set the StatusBar text when we hover over a menu
		// Only popup submenus have status strings
		if (m_UseMenuStatus && GetStatusBar().IsWindow())
		{
			int nID = LOWORD (wParam);
			CMenu Menu((HMENU) lParam);

			if ((Menu != T::GetMenu()) && (nID != 0) && !(HIWORD(wParam) & MF_POPUP))
				GetStatusBar().SetWindowText(LoadString(nID));
			else
				GetStatusBar().SetWindowText(m_strStatusText);

		}

		return 0L;
	}

	template <class T>
	inline void CFrameT<T>::OnMenuUpdate(UINT nID)
	// Called when a menu item is about to be displayed. Override this function to enable
	// or disible the menu item, or add a check mark. Also call this base function to
    // update the 'Tool Bar' and 'Status Bar' menu status.
	{
		// Update the check buttons before displaying the menu
		switch(nID)
		{
		case IDW_VIEW_STATUSBAR:
			{
				BOOL IsVisible = GetStatusBar().IsWindow() && GetStatusBar().IsWindowVisible();
				GetFrameMenu().CheckMenuItem(nID, IsVisible ? MF_CHECKED : MF_UNCHECKED);
			}
			break;
		case IDW_VIEW_TOOLBAR:
			{
				BOOL IsVisible = GetToolBar().IsWindow() && GetToolBar().IsWindowVisible();
				GetFrameMenu().EnableMenuItem(nID, m_UseToolBar ? MF_ENABLED : MF_DISABLED);
				GetFrameMenu().CheckMenuItem(nID, IsVisible ? MF_CHECKED : MF_UNCHECKED);
			}
			break;
		}
	}

	template <class T>
	inline LRESULT CFrameT<T>::OnNotify(WPARAM wParam, LPARAM lParam)
	// Called when a notification from a child window (WM_NOTIFY) is received.
	{
		UNREFERENCED_PARAMETER(wParam);

		LPNMHDR pNMHDR = (LPNMHDR)lParam;
		switch (pNMHDR->code)
		{
		case NM_CUSTOMDRAW:		return OnCustomDraw(pNMHDR);
		case RBN_HEIGHTCHANGE:	return OnRBNHeightChange(pNMHDR);
		case RBN_LAYOUTCHANGED:	return OnRBNLayoutChanged(pNMHDR);
		case RBN_MINMAX:		return OnRBNMinMax(pNMHDR);
		case TBN_DROPDOWN:		return OnTBNDropDown((LPNMTOOLBAR)lParam);
		case TTN_GETDISPINFO:	return OnTTNGetDispInfo((LPNMTTDISPINFO)lParam);
		case UWN_UNDOCKED:		return OnUndocked();
		}

		return CWnd::OnNotify(wParam, lParam);
	}

	template <class T>
	inline LRESULT CFrameT<T>::OnRBNHeightChange(LPNMHDR pNMHDR)
	// Called when the rebar's height changes
	{
		UNREFERENCED_PARAMETER(pNMHDR);

		RecalcLayout();

		return 0L;
	}

	template <class T>
	inline LRESULT CFrameT<T>::OnRBNLayoutChanged(LPNMHDR pNMHDR)
	// Notification of rebar layout change.
	{
		UNREFERENCED_PARAMETER(pNMHDR);

		if (GetReBarTheme().UseThemes && GetReBarTheme().BandsLeft)
			GetReBar().MoveBandsLeft();

		return 0L;
	}

	template <class T>
	inline LRESULT CFrameT<T>::OnRBNMinMax(LPNMHDR pNMHDR)
	// Notification of a rebar band minimized or maximized
	{
		UNREFERENCED_PARAMETER(pNMHDR);

		if (GetReBarTheme().UseThemes && GetReBarTheme().ShortBands)
			return 1L;	// Suppress maximise or minimise rebar band

		return 0L;
	}

	template <class T>
	inline LRESULT CFrameT<T>::OnTBNDropDown(LPNMTOOLBAR pNMTB)
	// Press of Dropdown botton on ToolBar
	{
		int iItem = pNMTB->iItem;
		CToolBar* pTB = static_cast<CToolBar*>(T::GetCWndPtr(pNMTB->hdr.hwndFrom));

		if (pTB)
		{
			// a boolean expression
			m_DrawArrowBkgrnd = (pTB->GetButtonStyle(iItem) & TBSTYLE_DROPDOWN);
		}

		return 0L;
	}

	template <class T>
	inline LRESULT CFrameT<T>::OnTTNGetDispInfo(LPNMTTDISPINFO pNMTDI)
	// Tool tip notification from the toolbar
	{
		// Find the ToolBar that generated the tooltip
		CPoint pt(GetMessagePos());
		HWND hWnd = ::WindowFromPoint(pt);
		CToolBar* pToolBar = (CToolBar*)::SendMessage(hWnd, UWM_GETCTOOLBAR, 0, 0);

		// Set the tooltip's text from the ToolBar button's CommandID
		if (pToolBar)
		{
			LPNMTTDISPINFO lpDispInfo = pNMTDI;
			int iIndex =  pToolBar->HitTest();
			if (iIndex >= 0)
			{
				int nID = pToolBar->GetCommandID(iIndex);
				if (nID > 0)
				{
					m_strTooltip = LoadString(nID);
					lpDispInfo->lpszText = const_cast<LPTSTR>(m_strTooltip.c_str());
				}
				else
					m_strTooltip = _T("");
			}
		}

		return 0L;
	}

	template <class T>
	inline LRESULT CFrameT<T>::OnSetFocus(UINT, WPARAM wParam, LPARAM lParam)
	// Called when the frame window (not a child window) receives focus
	{
		UNREFERENCED_PARAMETER(wParam);
		UNREFERENCED_PARAMETER(lParam);

		SetStatusIndicators();
		return 0L;
	}

	template <class T>
	inline LRESULT CFrameT<T>::OnSettingChange(UINT, WPARAM, LPARAM)
	// Called when the SystemParametersInfo function changes a system-wide
	//	setting or when policy settings have changed.
	{
		T::RedrawWindow();
		return 0L;
	}

	template <class T>
	inline LRESULT CFrameT<T>::OnSize(UINT, WPARAM wParam, LPARAM lParam)
	// Called when the frame window is resized
	{
		UNREFERENCED_PARAMETER(wParam);
		UNREFERENCED_PARAMETER(lParam);

		RecalcLayout();
		return 0L;
	}

	template <class T>
	inline LRESULT CFrameT<T>::OnSysColorChange(UINT, WPARAM wParam, LPARAM lParam)
	// Called in response to a WM_SYSCOLORCHANGE message. This message is sent
	// to all top-level windows when a change is made to a system color setting.
	{
		UNREFERENCED_PARAMETER(wParam);
		UNREFERENCED_PARAMETER(lParam);

		// Honour theme color changes
		if (GetReBar().IsWindow())
		{
			for (int nBand = 0; nBand <= GetReBar().GetBandCount(); ++nBand)
			{
				GetReBar().SetBandColor(nBand, GetSysColor(COLOR_BTNTEXT), GetSysColor(COLOR_BTNFACE));
			}
		}

		NONCLIENTMETRICS info;
		ZeroMemory(&info, sizeof(NONCLIENTMETRICS));
		info.cbSize = GetSizeofNonClientMetrics();
		SystemParametersInfo (SPI_GETNONCLIENTMETRICS, sizeof(info), &info, 0);

		if (GetStatusBar().IsWindow())
		{
			// Update the status bar font and text
			m_fntStatusBar.DeleteObject();
			m_fntStatusBar.CreateFontIndirect(info.lfStatusFont);
			GetStatusBar().SetFont(m_fntStatusBar, TRUE);
			if (m_UseMenuStatus)
				GetStatusBar().SetWindowText(m_strStatusText);

			SetStatusIndicators();
		}

		if (GetMenuBar().IsWindow())
		{
			// Update the MenuBar font and button size
			m_fntMenuBar.DeleteObject();
			m_fntMenuBar.CreateFontIndirect(info.lfMenuFont);
			GetMenuBar().SetFont(m_fntMenuBar, TRUE);
			GetMenuBar().SetMenu( GetFrameMenu() );

			// Update the MenuBar band size
			UpdateMenuBarBandSize();
		}

		if (m_XPThemeName != GetThemeName())
			SetTheme();

		// Reposition and redraw everything
		RecalcLayout();
		T::RedrawWindow(RDW_ERASE | RDW_FRAME | RDW_INVALIDATE | RDW_ALLCHILDREN);

		// Forward the message to the view window
		if (GetView().IsWindow())
			GetView().PostMessage(WM_SYSCOLORCHANGE, 0L, 0L);

		return 0L;
	}

	template <class T>
	inline LRESULT CFrameT<T>::OnSysCommand(UINT, WPARAM wParam, LPARAM lParam)
	// Called in response to a WM_SYSCOMMAND notification. This notification
	// is passed on to the MenuBar to process alt keys and maximise or restore.
	{
		if ((SC_KEYMENU == wParam) && (VK_SPACE != lParam) && IsMenuBarUsed())
		{
			GetMenuBar().OnSysCommand(WM_SYSCOMMAND, wParam, lParam);
			return 0L;
		}

		if (SC_MINIMIZE == wParam)
			m_hOldFocus = ::GetFocus();

		// Pass remaining system commands on for default processing
		return T::FinalWindowProc(WM_SYSCOMMAND, wParam, lParam);
	}

	template <class T>
	inline LRESULT CFrameT<T>::OnUndocked()
	// Notification of undocked from CDocker received via OnNotify
	{
		m_hOldFocus = 0;
		return 0;
	}

	template <class T>
	inline LRESULT CFrameT<T>::OnUnInitMenuPopup(UINT, WPARAM wParam, LPARAM lParam)
	// Called when the drop-down menu or submenu has been destroyed.
	// Win95 & WinNT don't support the WM_UNINITMENUPOPUP message.
	{
		UNREFERENCED_PARAMETER(wParam);
		UNREFERENCED_PARAMETER(lParam);

		for (int nItem = (int)m_vMenuItemData.size() - 1; nItem >= 0; --nItem)
		{
			// Undo OwnerDraw and put the text back
			MENUITEMINFO mii;
			ZeroMemory(&mii, sizeof(MENUITEMINFO));
			mii.cbSize = GetSizeofMenuItemInfo();

			mii.fMask = MIIM_TYPE | MIIM_DATA;
			mii.fType = m_vMenuItemData[nItem]->mii.fType;
			mii.dwTypeData = m_vMenuItemData[nItem]->GetItemText();
			mii.cch = lstrlen(m_vMenuItemData[nItem]->GetItemText());
			mii.dwItemData = 0;
			::SetMenuItemInfo(m_vMenuItemData[nItem]->hMenu, m_vMenuItemData[nItem]->nPos, TRUE, &mii);
			int nPos = m_vMenuItemData[nItem]->nPos;
			m_vMenuItemData.pop_back();

			// Break when we reach the top of this popup menu
			if (nPos == 0)
				break;
		}

		return 0L;
	}

	template <class T>
	inline BOOL CFrameT<T>::OnViewStatusBar()
	// Called in response to menu input to display or hide the status bar.
	{
		BOOL Show = !(GetStatusBar().IsWindow() && GetStatusBar().IsWindowVisible());
		ShowStatusBar(Show);
		return TRUE;
	}

	template <class T>
	inline BOOL CFrameT<T>::OnViewToolBar()
	// Called in response to menu input to display or hide the tool bar.
	{
		BOOL Show = m_UseToolBar && !GetToolBar().IsWindowVisible();
		ShowToolBar(Show);
		return TRUE;
	}

	template <class T>
	inline void CFrameT<T>::PreCreate(CREATESTRUCT& cs)
	// Sets frame window creation parameters prior to the frame window's creation
	{
		// Set the frame window styles
		cs.style = WS_OVERLAPPEDWINDOW | WS_VISIBLE;

		if (GetInitValues().ShowCmd == SW_MAXIMIZE) cs.style |= WS_MAXIMIZE;

		CWindowDC dcDesktop(0);

		// Does the window fit on the desktop?
		CRect rcInitPos = GetInitValues().rcPos;
		if (RectVisible(dcDesktop, &rcInitPos) && (rcInitPos.Width() > 0))
		{
			// Set the original window position
			cs.x  = rcInitPos.left;
			cs.y  = rcInitPos.top;
			cs.cx = rcInitPos.Width();
			cs.cy = rcInitPos.Height();
		}
	}

	template <class T>
	inline void CFrameT<T>::PreRegisterClass(WNDCLASS& wc)
	// Set the frame's class parameters prior to the frame window's creation
	{
		// Set the Window Class
		wc.lpszClassName =  _T("Win32++ Frame");
	}

	template <class T>
	inline void CFrameT<T>::RecalcLayout()
	// Repositions the frame's child windows
	{
		// Resize the status bar
		if (GetStatusBar().IsWindow() && GetStatusBar().IsWindowVisible())
		{
			GetStatusBar().SetWindowPos(NULL, 0, 0, 0, 0, SWP_SHOWWINDOW);
			GetStatusBar().Invalidate();
			if (GetUseMenuStatus())
				GetStatusBar().SetWindowText(GetStatusText());

			SetStatusIndicators();
		}

		// Resize the rebar or toolbar
		if (IsReBarUsed())
		{
			GetReBar().SendMessage(WM_SIZE, 0L, 0L);
			GetReBar().Invalidate();
		}
		else if (GetToolBar().IsWindow() && GetToolBar().IsWindowVisible())
			GetToolBar().SendMessage(TB_AUTOSIZE, 0L, 0L);

		// Position the view window
		if (GetView().IsWindow())
			RecalcViewLayout();

		// Adjust rebar bands
		if (IsReBarUsed())
		{
			if (GetReBarTheme().UseThemes && GetReBarTheme().BandsLeft)
				GetReBar().MoveBandsLeft();

			if (IsMenuBarUsed())
				SetMenuBarBandSize();
		}
	}

	template <class T>
	inline void CFrameT<T>::RecalcViewLayout()
	// Resize the view window
	{
		GetView().SetWindowPos( NULL, GetViewRect(), SWP_SHOWWINDOW);
	}

	template <class T>
	inline void CFrameT<T>::RemoveMRUEntry(LPCTSTR szMRUEntry)
	// Removes an entry from the MRU list
	{
		std::vector<CString>::iterator it;
		for (it = m_vMRUEntries.begin(); it != m_vMRUEntries.end(); ++it)
		{
			if ((*it) == szMRUEntry)
			{
				m_vMRUEntries.erase(it);
				break;
			}
		}

		UpdateMRUMenu();
	}

	template <class T>
	inline BOOL CFrameT<T>::SaveRegistryMRUSettings()
	// Saves the current MRU settings in the registry
	{
		// Store the MRU entries in the registry
		try
		{
			// Delete Old MRUs
			CString KeyParentName = _T("Software\\") + m_strKeyName;
			CRegKey KeyParent;
			KeyParent.Open(HKEY_CURRENT_USER, KeyParentName);
			KeyParent.DeleteSubKey(_T("Recent Files"));

			if (m_nMaxMRU > 0)
			{
				CString KeyName = _T("Software\\") + m_strKeyName + _T("\\Recent Files");
				CRegKey Key;

				// Add Current MRUs
				if (ERROR_SUCCESS != Key.Create(HKEY_CURRENT_USER, KeyName))
					throw CUserException(_T("RegCreateKeyEx failed"));

				if (ERROR_SUCCESS != Key.Open(HKEY_CURRENT_USER, KeyName))
					throw CUserException(_T("RegCreateKeyEx failed"));

				CString SubKeyName;
				CString strPathName;
				for (UINT i = 0; i < m_nMaxMRU; ++i)
				{
					SubKeyName.Format(_T("File %d"), i + 1);

					if (i < m_vMRUEntries.size())
					{
						strPathName = m_vMRUEntries[i];

						if (ERROR_SUCCESS != Key.SetStringValue(SubKeyName, strPathName.c_str()))
							throw CUserException(_T("RegSetValueEx failed"));
					}
				}
			}
		}

		catch (const CUserException& e)
		{
			Trace("*** Failed to save registry MRU settings. ***\n");
			Trace(e.GetText()); Trace("\n");

			CString KeyName = _T("Software\\") + m_strKeyName;
			CRegKey Key;

			if (ERROR_SUCCESS == Key.Open(HKEY_CURRENT_USER, KeyName))
			{
				// Roll back the registry changes by deleting this subkey
				Key.DeleteSubKey(_T("Recent Files"));
			}

			return FALSE;
		}

		return TRUE;
	}

	template <class T>
	inline BOOL CFrameT<T>::SaveRegistrySettings()
	// Saves various frame window settings in the registry
	{
		if (!m_strKeyName.IsEmpty())
		{
			try
			{
				CString KeyName = _T("Software\\") + m_strKeyName + _T("\\Frame Settings");
				CRegKey Key;

				if (ERROR_SUCCESS != Key.Create(HKEY_CURRENT_USER, KeyName))
					throw CUserException(_T("RegCreateKeyEx failed"));
				if (ERROR_SUCCESS != Key.Open(HKEY_CURRENT_USER, KeyName))
					throw CUserException(_T("RegCreateKeyEx failed"));

				// Store the window position in the registry
				WINDOWPLACEMENT Wndpl;
				ZeroMemory(&Wndpl, sizeof(WINDOWPLACEMENT));
				Wndpl.length = sizeof(WINDOWPLACEMENT);

				if (T::GetWindowPlacement(Wndpl))
				{
					// Get the Frame's window position
					CRect rc = Wndpl.rcNormalPosition;
					DWORD dwTop = MAX(rc.top, 0);
					DWORD dwLeft = MAX(rc.left, 0);
					DWORD dwWidth = MAX(rc.Width(), 100);
					DWORD dwHeight = MAX(rc.Height(), 50);
					DWORD dwShowCmd = Wndpl.showCmd;

					if (ERROR_SUCCESS != Key.SetDWORDValue(_T("Top"), dwTop))
						throw CUserException(_T("RegSetValueEx failed"));
					if (ERROR_SUCCESS != Key.SetDWORDValue(_T("Left"), dwLeft))
						throw CUserException(_T("RegSetValueEx failed"));
					if (ERROR_SUCCESS != Key.SetDWORDValue(_T("Width"), dwWidth))
						throw CUserException(_T("RegSetValueEx failed"));
					if (ERROR_SUCCESS != Key.SetDWORDValue(_T("Height"), dwHeight))
						throw CUserException(_T("RegSetValueEx failed"));
					if (ERROR_SUCCESS != Key.SetDWORDValue(_T("ShowCmd"), dwShowCmd))
						throw CUserException(_T("RegSetValueEx failed"));
				}

				// Store the ToolBar and statusbar states
				DWORD dwShowToolBar = GetToolBar().IsWindow() && GetToolBar().IsWindowVisible();
				DWORD dwShowStatusBar = GetStatusBar().IsWindow() && GetStatusBar().IsWindowVisible();

				if (ERROR_SUCCESS != Key.SetDWORDValue(_T("ToolBar"), dwShowToolBar))
					throw CUserException(_T("RegSetValueEx failed"));
				if (ERROR_SUCCESS != Key.SetDWORDValue(_T("StatusBar"), dwShowStatusBar))
					throw CUserException(_T("RegSetValueEx failed"));
			}

			catch (const CUserException& e)
			{
				Trace("*** Failed to save registry settings. ***\n");
				Trace(e.GetText()); Trace("\n");

				CString KeyName = _T("Software\\") + m_strKeyName;
				CRegKey Key;

				if (ERROR_SUCCESS == Key.Open(HKEY_CURRENT_USER, KeyName))
				{
					// Roll back the registry changes by deleting this subkey
					Key.DeleteSubKey(_T("Frame Settings"));
				}

				return FALSE;
			}

			return SaveRegistryMRUSettings();
		}

		return TRUE;
	}

	template <class T>
	inline void CFrameT<T>::SetAccelerators(UINT ID_ACCEL)
	// Sets the accelerator table for the application for this window
	{
		m_hAccel = LoadAccelerators(GetApp().GetResourceHandle(), MAKEINTRESOURCE(ID_ACCEL));
		if (m_hAccel)
			GetApp().SetAccelerators(m_hAccel, *this);
	}

	template <class T>
	inline void CFrameT<T>::SetFrameMenu(UINT ID_MENU)
	// Sets the frame's menu from a Resource ID.
	// A resource ID of 0 removes the menu from the frame.
	{
		HMENU hMenu = 0;
		if (ID_MENU != 0)
		{
		// Sets the frame's menu from a resource ID.
			hMenu = ::LoadMenu(GetApp().GetResourceHandle(), MAKEINTRESOURCE(ID_MENU));
			assert (hMenu);
		}

		SetFrameMenu(hMenu);
	}

	template <class T>
	inline void CFrameT<T>::SetFrameMenu(HMENU hMenu)
	// Sets the frame's menu.
	{
		m_Menu.Attach(hMenu);

		if (IsMenuBarUsed())
		{
			GetMenuBar().SetMenu( GetFrameMenu() );
			BOOL Show = (hMenu != NULL);	// boolean expression
			ShowMenu(Show);
		}
		else
		{
			T::SetMenu(m_Menu);
			T::DrawMenuBar();
		}
	}

	template <class T>
	inline void CFrameT<T>::SetInitValues(const InitValues& Values)
	// Sets the initial values from the InitValues struct. The InitValues struct
	// is used to load and store the initial values, usually saved in the
	// registry or an INI file.
	{
		m_InitValues = Values;
	}

	template <class T>
	inline UINT CFrameT<T>::SetMenuIcons(const std::vector<UINT>& MenuData, COLORREF crMask, UINT ToolBarID, UINT ToolBarDisabledID)
	// Sets the menu icons. Any previous menu icons are removed.
	{
		// Remove any existing menu icons
		m_imlMenu.DeleteImageList();
		m_imlMenuDis.DeleteImageList();
		m_vMenuIcons.clear();

		// Exit if no ToolBarID is specified
		if (ToolBarID == 0) return 0;

		// Add the menu icons from the bitmap IDs
		return AddMenuIcons(MenuData, crMask, ToolBarID, ToolBarDisabledID);
	}

	template <class T>
	inline void CFrameT<T>::SetMenuBarBandSize()
	// Sets the minimum width of the MenuBar band to the width of the rebar
	// This prevents other bands from moving to this MenuBar's row.
	{
		CRect rcClient = T::GetClientRect();
		CReBar& RB = GetReBar();
		int nBand = RB.GetBand(GetMenuBar());
		if (nBand >= 0)
		{
			CRect rcBorder = RB.GetBandBorders(nBand);

			REBARBANDINFO rbbi;
			ZeroMemory(&rbbi, sizeof(REBARBANDINFO));
			rbbi.fMask = RBBIM_CHILDSIZE | RBBIM_SIZE;
			RB.GetBandInfo(nBand, rbbi);

			int Width;
			if ((GetReBarTheme().UseThemes) && (GetReBarTheme().LockMenuBand))
				Width = rcClient.Width() - rcBorder.Width() - 2;
			else
				Width = GetMenuBar().GetMaxSize().cx;

			rbbi.cxMinChild = Width;
			rbbi.cx         = Width;

			RB.SetBandInfo(nBand, rbbi);
		}
	}

	template <class T>
	inline void CFrameT<T>::SetMenuTheme(MenuTheme& MBT)
	// Sets the theme colors for the MenuBar and the popup Menu items
	// Note: If Aero Themes are supported, they are used for popup menu items instead
	{
		m_MBTheme = MBT;

		if (GetMenuBar().IsWindow())
			GetMenuBar().Invalidate();
	}

	template <class T>
	inline void CFrameT<T>::SetMRULimit(UINT MRULimit)
	// Sets the maximum number of MRU entries
	{
		// Remove any excess MRU entries
		if (MRULimit < m_vMRUEntries.size())
		{
			m_vMRUEntries.erase(m_vMRUEntries.begin() + MRULimit, m_vMRUEntries.end());
		}

		m_nMaxMRU = MRULimit;
		UpdateMRUMenu();
	}

	template <class T>
	inline void CFrameT<T>::SetReBarTheme(ReBarTheme& RBT)
	// Stores the rebar's theme colors
	{
		m_RBTheme = RBT;
	}

	template <class T>
	inline void CFrameT<T>::SetStatusBarTheme(StatusBarTheme& SBT)
	// Stores the statusbar's theme colors
	{
		m_SBTheme = SBT;
	}

	template <class T>
	inline void CFrameT<T>::SetStatusIndicators()
	// Creates 4 panes in the status bar and displays status and key states.
	{
		if (GetStatusBar().IsWindow() && (m_UseIndicatorStatus))
		{
			// Calculate the width of the text indicators
			CClientDC dcStatus(GetStatusBar());
			CString CAP = LoadString(IDW_INDICATOR_CAPS);
			CString NUM = LoadString(IDW_INDICATOR_NUM);
			CString SCRL = LoadString(IDW_INDICATOR_SCRL);
			CSize csCAP  = dcStatus.GetTextExtentPoint32(CAP, lstrlen(CAP)+1);
			CSize csNUM  = dcStatus.GetTextExtentPoint32(NUM, lstrlen(NUM)+1);
			CSize csSCRL = dcStatus.GetTextExtentPoint32(SCRL, lstrlen(SCRL)+1);

			// Adjust for DPI aware
			int dpiX = dcStatus.GetDeviceCaps(LOGPIXELSX);
			csCAP.cx  = MulDiv(csCAP.cx, dpiX, 96);
			csNUM.cx  = MulDiv(csNUM.cx, dpiX, 96);
			csSCRL.cx = MulDiv(csSCRL.cx, dpiX, 96);

			// Get the coordinates of the window's client area.
			CRect rcClient = T::GetClientRect();
			int width = MAX(300, rcClient.right);

			// Create 4 panes
			GetStatusBar().SetPartWidth(0, width - (csCAP.cx+csNUM.cx+csSCRL.cx+20));
			GetStatusBar().SetPartWidth(1, csCAP.cx);
			GetStatusBar().SetPartWidth(2, csNUM.cx);
			GetStatusBar().SetPartWidth(3, csSCRL.cx);

			CString Status1 = (::GetKeyState(VK_CAPITAL) & 0x0001)? CAP : CString("");
			CString Status2 = (::GetKeyState(VK_NUMLOCK) & 0x0001)? NUM : CString("");
			CString Status3 = (::GetKeyState(VK_SCROLL)  & 0x0001)? SCRL: CString("");

			// Only update indicators if the text has changed
			if (Status1 != m_OldStatus[0])  GetStatusBar().SetPartText(1, Status1);
			if (Status2 != m_OldStatus[1])  GetStatusBar().SetPartText(2, Status2);
			if (Status3 != m_OldStatus[2])  GetStatusBar().SetPartText(3, Status3);

			m_OldStatus[0] = Status1;
			m_OldStatus[1] = Status2;
			m_OldStatus[2] = Status3;
		}
	}

	template <class T>
	inline void CFrameT<T>::SetStatusText(LPCTSTR szText)
	// Stores the status text and displays it in the StatusBar
	{
		m_strStatusText = szText;

		if (GetStatusBar().IsWindow())
		{
			// Place text in the 1st pane
			GetStatusBar().SetPartText(0, m_strStatusText);
		}
	}

	template <class T>
	inline void CFrameT<T>::SetTheme()
	// Sets the theme colors for the frame's rebar, toolbar and menubar
	// Note: To modify theme colors, override this function in CMainFrame,
	//        and make any modifications there.
	{
		// Avoid themes if using less than 16 bit colors
		CClientDC DesktopDC(NULL);
		if (DesktopDC.GetDeviceCaps(BITSPIXEL) < 16)
			m_UseThemes = FALSE;

		BOOL t = TRUE;
		BOOL f = FALSE;

		if (m_UseThemes)
		{
			// Retrieve the XP theme name
			m_XPThemeName = GetThemeName();

			enum Themetype{ Win8, Win7, XP_Blue, XP_Silver, XP_Olive, Grey };

			// For Win2000 and below
			int Theme = Grey;

			if (GetWinVersion() < 2600) // For Windows XP
			{
				if (m_XPThemeName == _T("NormalColor"))
					Theme = XP_Blue;
				if (m_XPThemeName == _T("Metallic"))
					Theme = XP_Silver;
				if (m_XPThemeName == _T("HomeStead"))
					Theme = XP_Olive;
			}
			else if (GetWinVersion() <= 2601)
			{
				// For Vista and Windows 7
				Theme = Win7;
			}
			else
			{
				// For Windows 8 and above
				Theme = Win8;
			}

			switch (Theme)
			{
			case Win8:	// A pale blue scheme without gradients, suitable for Windows 8, 8.1, and 10
				{
					MenuTheme mt = {t, RGB(180, 250, 255), RGB(140, 190, 255), RGB(240, 250, 255), RGB(120, 170, 220), RGB(127, 127, 255)};
					ReBarTheme rbt = {t, RGB(235, 237, 250), RGB(235, 237, 250), RGB(235, 237, 250), RGB(235, 237, 250), f, f, t, f, t, f };
					StatusBarTheme sbt = {t, RGB(235, 237, 250), RGB(235, 237, 250)};
					ToolBarTheme tbt = {t, RGB(180, 250, 255), RGB(140, 190, 255), RGB(150, 220, 255), RGB(80, 100, 255), RGB(127, 127, 255)};

					SetMenuTheme(mt);	// Sets the theme for popup menus and MenuBar
					SetReBarTheme(rbt);
					SetStatusBarTheme(sbt);
					SetToolBarTheme(tbt);
				}
				break;

			case Win7:	// A pale blue color scheme suitable for Vista and Windows 7
				{
					MenuTheme mt = {t, RGB(180, 250, 255), RGB(140, 190, 255), RGB(240, 250, 255), RGB(120, 170, 220), RGB(127, 127, 255)};
					ReBarTheme rbt = {t, RGB(225, 230, 255), RGB(240, 242, 250), RGB(248, 248, 248), RGB(180, 200, 230), f, f, t, t, t, f};
					StatusBarTheme sbt = {t, RGB(225, 230, 255), RGB(240, 242, 250)};
					ToolBarTheme tbt = {t, RGB(180, 250, 255), RGB(140, 190, 255), RGB(150, 220, 255), RGB(80, 100, 255), RGB(127, 127, 255)};

					SetMenuTheme(mt);	// Sets the theme for popup menus and MenuBar
					SetReBarTheme(rbt);
					SetStatusBarTheme(sbt);
					SetToolBarTheme(tbt);
				}
				break;


			case XP_Blue:
				{
					// Used for XP default (blue) color scheme
					MenuTheme mt = {t, RGB(255, 230, 190), RGB(255, 190, 100), RGB(220,230,250), RGB(150,190,245), RGB(128, 128, 200)};
					ReBarTheme rbt = {t, RGB(150,190,245), RGB(196,215,250), RGB(220,230,250), RGB( 70,130,220), f, f, t, t, t, f};
					StatusBarTheme sbt = {t, RGB(150,190,245), RGB(196,215,250)};
					ToolBarTheme tbt = {t, RGB(255, 230, 190), RGB(255, 190, 100), RGB(255, 140, 40), RGB(255, 180, 80), RGB(192, 128, 255)};

					SetMenuTheme(mt);	// Sets the theme for popup menus and MenuBar
					SetReBarTheme(rbt);
					SetStatusBarTheme(sbt);
					SetToolBarTheme(tbt);
				}
				break;

			case XP_Silver:
				{
					// Used for XP Silver color scheme
					MenuTheme mt = {t, RGB(196, 215, 250), RGB( 120, 180, 220), RGB(240, 240, 245), RGB(170, 165, 185), RGB(128, 128, 150)};
					ReBarTheme rbt = {t, RGB(225, 220, 240), RGB(240, 240, 245), RGB(245, 240, 255), RGB(160, 155, 180), f, f, t, t, t, f};
					StatusBarTheme sbt = {t, RGB(225, 220, 240), RGB(240, 240, 245)};
					ToolBarTheme tbt = {t, RGB(192, 210, 238), RGB(192, 210, 238), RGB(152, 181, 226), RGB(152, 181, 226), RGB(49, 106, 197)};

					SetMenuTheme(mt);	// Sets the theme for popup menus and MenuBar
					SetReBarTheme(rbt);
					SetStatusBarTheme(sbt);
					SetToolBarTheme(tbt);
				}
				break;

			case XP_Olive:
				{
					// Used for XP Olive color scheme
					MenuTheme mt = {t, RGB(255, 230, 190), RGB(255, 190, 100), RGB(249, 255, 227), RGB(178, 191, 145), RGB(128, 128, 128)};
					ReBarTheme rbt = {t, RGB(215, 216, 182), RGB(242, 242, 230), RGB(249, 255, 227), RGB(178, 191, 145), f, f, t, t, t, f};
					StatusBarTheme sbt = {t, RGB(215, 216, 182), RGB(242, 242, 230)};
					ToolBarTheme tbt = {t, RGB(255, 230, 190), RGB(255, 190, 100), RGB(255, 140, 40), RGB(255, 180, 80), RGB(200, 128, 128)};

					SetMenuTheme(mt);	// Sets the theme for popup menus and MenuBar
					SetReBarTheme(rbt);
					SetStatusBarTheme(sbt);
					SetToolBarTheme(tbt);
				}
				break;

			case Grey:	// A color scheme suitable for 16 bit colors. Suitable for Windows older than XP.
				{
					MenuTheme mt = {t, RGB(182, 189, 210), RGB( 182, 189, 210), RGB(200, 196, 190), RGB(200, 196, 190), RGB(100, 100, 100)};
					ReBarTheme rbt = {t, RGB(212, 208, 200), RGB(212, 208, 200), RGB(230, 226, 222), RGB(220, 218, 208), f, f, t, t, t, f};
					StatusBarTheme sbt = {t, RGB(212, 208, 200), RGB(212, 208, 200)};
					ToolBarTheme tbt = {t, RGB(182, 189, 210), RGB(182, 189, 210), RGB(133, 146, 181), RGB(133, 146, 181), RGB(10, 36, 106)};

					SetMenuTheme(mt);	// Sets the theme for popup menus and MenuBar
					SetReBarTheme(rbt);
					SetStatusBarTheme(sbt);
					SetToolBarTheme(tbt);
				}
				break;
			}
		}
		else
		{
			// Set a default menu theme. This allows the menu to display icons.
			// The colours specified here are used for Windows 2000 and below.
			MenuTheme mt = {FALSE, RGB(182, 189, 210), RGB( 182, 189, 210), RGB(200, 196, 190), RGB(200, 196, 190), RGB(100, 100, 100)};
			SetMenuTheme(mt);  // Sets the theme for popup menus and MenuBar
		}

		RecalcLayout();
	}

	template <class T>
	inline void CFrameT<T>::SetTBImageList(CToolBar& ToolBar, CImageList& ImageList, UINT nID, COLORREF crMask)
	// Sets the Image List for additional Toolbars.
	// The specified CToolBar should be a member of CMainFrame to ensure it remains in scope.
	// The specified CImageList should be a member of CMainFrame to ensure it remains in scope.
	// A Disabled image list is created from ToolBarID if one doesn't already exist.
	{
		// Get the image size
		CBitmap bm(nID);
		CSize sz = GetTBImageSize(&bm);

		// Set the toolbar's image list
		ImageList.DeleteImageList();
		ImageList.Create(sz.cx, sz.cy, ILC_COLOR32 | ILC_MASK, 0, 0);
		ImageList.Add(bm, crMask);
		ToolBar.SetImageList(ImageList);

		// Inform the Rebar of the change to the Toolbar
		if (GetReBar().IsWindow())
		{
			SIZE MaxSize = ToolBar.GetMaxSize();
			GetReBar().SendMessage(UWM_TBRESIZE, (WPARAM)ToolBar.GetHwnd(), (LPARAM)&MaxSize);
		}
	}

	template <class T>
	inline void CFrameT<T>::SetTBImageListDis(CToolBar& ToolBar, CImageList& ImageList, UINT nID, COLORREF crMask)
	// Sets the Disabled Image List for additional Toolbars.
	// The specified CToolBar should be a member of CMainFrame to ensure it remains in scope.
	// The specified CImageList should be a member of CMainFrame to ensure it remains in scope.
	{
		if (nID != 0)
		{
			// Get the image size
			CBitmap bm(nID);
			CSize sz = GetTBImageSize(&bm);

			// Set the toolbar's image list
			ImageList.DeleteImageList();
			ImageList.Create(sz.cx, sz.cy, ILC_COLOR32 | ILC_MASK, 0, 0);
			ImageList.Add(bm, crMask);
			ToolBar.SetDisableImageList(ImageList);
		}
		else
		{
			ImageList.DeleteImageList();
			ImageList.CreateDisabledImageList(m_ToolBarImages);
			ToolBar.SetDisableImageList(ImageList);
		}

		// Inform the Rebar of the change to the Toolbar
		if (GetReBar().IsWindow())
		{
			SIZE MaxSize = ToolBar.GetMaxSize();
			GetReBar().SendMessage(UWM_TBRESIZE, (WPARAM)ToolBar.GetHwnd(), (LPARAM)&MaxSize);
		}
	}

	template <class T>
	inline void CFrameT<T>::SetTBImageListHot(CToolBar& ToolBar, CImageList& ImageList, UINT nID, COLORREF crMask)
	// Sets the Hot Image List for additional Toolbars.
	// The specified CToolBar should be a member of CMainFrame to ensure it remains in scope.
	// The specified CImageList should be a member of CMainFrame to ensure it remains in scope.
	{
		if (nID != 0)
		{
			// Get the image size
			CBitmap bm(nID);
			CSize sz = GetTBImageSize(&bm);

			// Set the toolbar's image list
			ImageList.DeleteImageList();
			ImageList.Create(sz.cx, sz.cy, ILC_COLOR32 | ILC_MASK, 0, 0);
			ImageList.Add(bm, crMask);
			ToolBar.SetHotImageList(ImageList);
		}
		else
		{
			ImageList.DeleteImageList();
			ToolBar.SetHotImageList(0);
		}

		// Inform the Rebar of the change to the Toolbar
		if (GetReBar().IsWindow())
		{
			SIZE MaxSize = ToolBar.GetMaxSize();
			GetReBar().SendMessage(UWM_TBRESIZE, (WPARAM)ToolBar.GetHwnd(), (LPARAM)&MaxSize);
		}
	}

	template <class T>
	inline void CFrameT<T>::SetToolBarImages(COLORREF crMask, UINT ToolBarID, UINT ToolBarHotID, UINT ToolBarDisabledID)
	// Either sets the imagelist or adds/replaces bitmap depending on ComCtl32.dll version
	// The ToolBarIDs are bitmap resources containing a set of toolbar button images.
	// Each toolbar button image must have a minimum height of 16. Its height must equal its width.
	// The colour mask is ignored for 32bit bitmaps, but is required for 24bit bitmaps
	// The colour mask is often grey RGB(192,192,192) or magenta (255,0,255)
	// The Hot and disabled bitmap resources can be 0.
	// A Disabled image list is created from ToolBarID if one isn't provided.
	{
		if (GetComCtlVersion() < 470)	// only on Win95
		{
			// We are using COMCTL32.DLL version 4.0, so we can't use an ImageList.
			// Instead we simply set the bitmap.
			GetToolBar().SetBitmap(ToolBarID);
			return;
		}

		// Set the button images
		SetTBImageList(GetToolBar(),    m_ToolBarImages, ToolBarID, crMask);
		SetTBImageListHot(GetToolBar(), m_ToolBarHotImages, ToolBarHotID, crMask);
		SetTBImageListDis(GetToolBar(), m_ToolBarDisabledImages, ToolBarDisabledID, crMask);
	}

	template <class T>
	inline void CFrameT<T>::SetupMenuIcons()
	// Assigns icons to the dropdown menu items. By default the toolbar icons are
	// added to the menu items. Override this function to assign additional or
	// different icons to the drop down menu items.
	{
		// Add the set of toolbar images to the menu
		if (GetToolBarData().size() > 0)
		{
			// Add the icons for popup menu
			AddMenuIcons(GetToolBarData(), RGB(192, 192, 192), IDW_MAIN, 0);
		}
	}

	template <class T>
	inline void CFrameT<T>::SetupToolBar()
	{
		// Use this function to set the Resource IDs for the toolbar(s).

/*		// Set the Resource IDs for the toolbar buttons
		AddToolBarButton( IDM_FILE_NEW   );
		AddToolBarButton( IDM_FILE_OPEN  );
		AddToolBarButton( IDM_FILE_SAVE  );

		AddToolBarButton( 0 );						// Separator
		AddToolBarButton( IDM_EDIT_CUT,   FALSE );	// disabled button
		AddToolBarButton( IDM_EDIT_COPY,  FALSE );	// disabled button
		AddToolBarButton( IDM_EDIT_PASTE, FALSE );	// disabled button

		AddToolBarButton( 0 );						// Separator
		AddToolBarButton( IDM_FILE_PRINT );

		AddToolBarButton( 0 );						// Separator
		AddToolBarButton( IDM_HELP_ABOUT );
*/
	}

	template <class T>
	inline void CFrameT<T>::SetToolBarTheme(ToolBarTheme& TBT)
	// Stores the tool bar's theme colors
	{
		m_TBTheme = TBT;
		if (GetToolBar().IsWindow())
			GetToolBar().GetParent().RedrawWindow(RDW_INVALIDATE|RDW_ALLCHILDREN);
	}

	template <class T>
	inline void CFrameT<T>::SetView(CWnd& wndView)
	{
		if (m_pView != &wndView)
		{
			// Hide the old view if any
			if (m_pView && m_pView->IsWindow())
				m_pView->ShowWindow(SW_HIDE);

			// Assign the new view
			m_pView = &wndView;

			if (T::IsWindow())
			{
				if (!m_pView->IsWindow())
					m_pView->Create(*this);
				else
					GetView().SetParent(*this);

				CRect rc = GetViewRect();
				GetView().SetWindowPos(NULL, rc, SWP_SHOWWINDOW);
			}
		}
	}

	template <class T>
	inline void CFrameT<T>::ShowMenu(BOOL Show)
	// Hides or shows the menu
	{
		if (Show)
		{
			if (IsReBarUsed())
				GetReBar().SendMessage(RB_SHOWBAND, (WPARAM)GetReBar().GetBand(GetMenuBar()), TRUE);
			else
				T::SetMenu(m_Menu);
		}
		else
		{
			if (IsReBarUsed())
				GetReBar().SendMessage(RB_SHOWBAND, (WPARAM)GetReBar().GetBand(GetMenuBar()), FALSE);
			else
				T::SetMenu(NULL);
		}

		if (GetReBar().IsWindow())
		{
			if (GetReBarTheme().UseThemes && GetReBarTheme().BandsLeft)
				GetReBar().MoveBandsLeft();
		}

		// Reposition the Windows
		RecalcLayout();
	}

	template <class T>
	inline void CFrameT<T>::ShowStatusBar(BOOL Show)
	// Hides or shows the status bar
	{
		if (GetStatusBar().IsWindow())
		{
			if (Show)
			{
				GetStatusBar().ShowWindow(SW_SHOW);
			}
			else
			{
				GetStatusBar().ShowWindow(SW_HIDE);
			}
		}

		// Reposition the Windows
		RecalcLayout();
		T::RedrawWindow();
	}

	template <class T>
	inline void CFrameT<T>::ShowToolBar(BOOL Show)
	// Hides or shows the tool bar
	{
		if (GetToolBar().IsWindow())
		{
			if (Show)
			{
				if (IsReBarUsed())
					GetReBar().SendMessage(RB_SHOWBAND, (WPARAM)GetReBar().GetBand(GetToolBar()), TRUE);
				else
					GetToolBar().ShowWindow(SW_SHOW);
			}
			else
			{
				if (IsReBarUsed())
					GetReBar().SendMessage(RB_SHOWBAND, (WPARAM)GetReBar().GetBand(GetToolBar()), FALSE);
				else
					GetToolBar().ShowWindow(SW_HIDE);
			}
		}

		if (GetReBar().IsWindow())
		{
			if (GetReBarTheme().UseThemes && GetReBarTheme().BandsLeft)
				GetReBar().MoveBandsLeft();
		}

		// Reposition the Windows
		RecalcLayout();
		T::RedrawWindow();
	}

	template <class T>
	inline LRESULT CALLBACK CFrameT<T>::StaticKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
	// Called by the keyboard hook to update status information
	{
		TLSData* pTLSData = GetApp().GetTlsData();
		HWND hFrame = pTLSData->hMainWnd;
		CFrameT<T>* pFrame = reinterpret_cast< CFrameT<T>* >(CWnd::GetCWndPtr(hFrame));
		assert(pFrame);

		if (HC_ACTION == nCode)
		{
			if ((wParam ==  VK_CAPITAL) || (wParam == VK_NUMLOCK) || (wParam == VK_SCROLL))
			{
				pFrame->SetStatusIndicators();
			}
		}

		return ::CallNextHookEx(pFrame->m_KbdHook, nCode, wParam, lParam);
	}

	template <class T>
	inline void CFrameT<T>::UpdateMenuBarBandSize()
	// Update the MenuBar band size
	{
		int nBand = GetReBar().GetBand(GetMenuBar());
		if (nBand >= 0)
		{
			REBARBANDINFO rbbi;
			ZeroMemory(&rbbi, sizeof(REBARBANDINFO));
			CClientDC dcMenuBar(GetMenuBar());
			dcMenuBar.SelectObject(GetMenuBar().GetFont());
			CSize sizeMenuBar = dcMenuBar.GetTextExtentPoint32(_T("\tSomeText"), lstrlen(_T("\tSomeText")));
			int MenuBar_Height = sizeMenuBar.cy + 6;
			rbbi.fMask      = RBBIM_CHILDSIZE;
			rbbi.cyMinChild = MenuBar_Height;
			rbbi.cyMaxChild = MenuBar_Height;
			GetReBar().SetBandInfo(nBand, rbbi);
		}
	}

	template <class T>
	inline void CFrameT<T>::UpdateMRUMenu()
	// Updates the menu item information for the Most Recently Used (MRU) list.
	{
		if (!T::IsWindow() ) return;

		// Create a vector of CStrings containing the MRU menu entries
		std::vector<CString> MRUStrings;

		if (m_vMRUEntries.size() > 0)
		{
			for (UINT n = 0; n < m_vMRUEntries.size(); ++n)
			{
				CString str = m_vMRUEntries[n];

				// Prefix the string with its number
				CString Count;
				Count.Format(_T("%d "), n + 1);
				str = Count + str;

				// Trim the string if its too long
				if (str.GetLength() > MAX_MENU_STRING)
				{
					// Extract the first part of the string up until the first "\\"
					CString Prefix;
					int Index = str.Find(_T("\\"));
					if (Index >= 0)
						Prefix = str.Left(Index + 1);

					// Reduce the string to fit within MAX_MENU_STRING
					CString Gap = _T("...");
					str.Delete(0, str.GetLength() - MAX_MENU_STRING + Prefix.GetLength() + Gap.GetLength()+1);

					// Remove the front of the string up to the next "\\" if any.
					Index = str.Find(_T("\\"));
					if (Index >= 0)
						str.Delete(0, Index);

					str = Prefix + Gap + str;
				}

				MRUStrings.push_back(str);
			}
		}
		else
		{
			MRUStrings.push_back(_T("Recent Files"));
		}

		// Set MRU menu items
		MENUITEMINFO mii;
		ZeroMemory(&mii, sizeof(MENUITEMINFO));
		mii.cbSize = GetSizeofMenuItemInfo();

		// We place the MRU items under the left most menu item
		CMenu FileMenu = GetFrameMenu().GetSubMenu(0);

		if (FileMenu.GetHandle())
		{
			// Remove all but the first MRU Menu entry
			for (UINT u = IDW_FILE_MRU_FILE2; u <= IDW_FILE_MRU_FILE1 + MRUStrings.size(); ++u)
			{
				FileMenu.DeleteMenu(u, MF_BYCOMMAND);
			}

			int MaxMRUIndex = static_cast<int>(MRUStrings.size() -1);

			for (int index = MaxMRUIndex; index >= 0; --index)
			{
				mii.fMask = MIIM_TYPE | MIIM_ID | MIIM_STATE;
				mii.fState = (0 == m_vMRUEntries.size())? MFS_GRAYED : 0;
				mii.fType = MFT_STRING;
				mii.wID = IDW_FILE_MRU_FILE1 + index;
				mii.dwTypeData = const_cast<LPTSTR>(MRUStrings[index].c_str());

				BOOL Succeeded;
				if (index == MaxMRUIndex)
					// Replace the last MRU entry first
					Succeeded = FileMenu.SetMenuItemInfo(IDW_FILE_MRU_FILE1, mii, FALSE);
				else
					// Insert the other MRU entries next
					Succeeded = FileMenu.InsertMenuItem(IDW_FILE_MRU_FILE1 + index + 1, mii, FALSE);

				if (!Succeeded)
				{
					TRACE("Failed to set MRU menu item\n");
					break;
				}
			}
		}

		T::DrawMenuBar();
	}

	template <class T>
	inline LRESULT CFrameT<T>::WndProcDefault(UINT uMsg, WPARAM wParam, LPARAM lParam)
	// Handle the frame's window messages.
	{
		switch (uMsg)
		{
		case WM_ACTIVATE:		return OnActivate(uMsg, wParam, lParam);
		case WM_DRAWITEM:		return OnDrawItem(uMsg, wParam, lParam);
		case WM_ERASEBKGND:		return 0L;
		case WM_HELP:			return OnHelp();
		case WM_INITMENUPOPUP:	return OnInitMenuPopup(uMsg, wParam, lParam);
		case WM_MENUCHAR:		return OnMenuChar(uMsg, wParam, lParam);
		case WM_MEASUREITEM:	return OnMeasureItem(uMsg, wParam, lParam);
		case WM_MENUSELECT:		return OnMenuSelect(uMsg, wParam, lParam);
		case WM_SETFOCUS:		return OnSetFocus(uMsg, wParam, lParam);
		case WM_SETTINGCHANGE:  return OnSettingChange(uMsg, wParam, lParam);
		case WM_SIZE:			return OnSize(uMsg, wParam, lParam);
		case WM_SYSCOLORCHANGE:	return OnSysColorChange(uMsg, wParam, lParam);
		case WM_SYSCOMMAND:		return OnSysCommand(uMsg, wParam, lParam);
		case WM_UNINITMENUPOPUP:  return OnUnInitMenuPopup(uMsg, wParam, lParam);
		case WM_WINDOWPOSCHANGED: return T::FinalWindowProc(uMsg, wParam, lParam);

		// Messages defined by Win32++
		case UWM_GETFRAMEVIEW:		return reinterpret_cast<LRESULT>(GetView().GetHwnd());
		case UWM_GETMBTHEME:		return reinterpret_cast<LRESULT>(&GetMenuBarTheme());
		case UWM_GETRBTHEME:		return reinterpret_cast<LRESULT>(&GetReBarTheme());
		case UWM_GETSBTHEME:		return reinterpret_cast<LRESULT>(&GetStatusBarTheme());
		case UWM_GETTBTHEME:		return reinterpret_cast<LRESULT>(&GetToolBarTheme());
		case UWM_DRAWRBBKGND:       return DrawReBarBkgnd(*((CDC*) wParam), *((CReBar*) lParam));
		case UWM_DRAWSBBKGND:       return DrawStatusBarBkgnd(*((CDC*) wParam), *((CStatusBar*) lParam));
		case UWM_GETCFRAMET:		return reinterpret_cast<LRESULT>(this);

		} // switch uMsg

		return CWnd::WndProcDefault(uMsg, wParam, lParam);
	}


} // namespace Win32xx

#endif // _WIN32XX_FRAME_H_
