// Win32++   Version 8.8
// Release Date: 15th October 2020
//
//      David Nash
//      email: dnash@bigpond.net.au
//      url: https://sourceforge.net/projects/win32-framework
//
//
// Copyright (c) 2005-2020  David Nash
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
  #define TMT_BORDERSIZE    2403
#endif
#ifndef TMT_CONTENTMARGINS
  #define TMT_CONTENTMARGINS    3602
#endif
#ifndef VSCLASS_MENU
  #define VSCLASS_MENU  L"MENU"
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
  #define WM_UNINITMENUPOPUP        0x0125
#endif

#ifndef WM_MENURBUTTONUP
  #define WM_MENURBUTTONUP      0x0122
#endif

#if defined (_MSC_VER) && (_MSC_VER >= 1400)
#pragma warning ( push )
#pragma warning ( disable : 26812 )       // enum type is unscoped.
#endif // (_MSC_VER) && (_MSC_VER >= 1400)


namespace Win32xx
{

    ////////////////////////////////////////////////
    // Declarations of structures for themes
    //

    // define some structs and enums from uxtheme.h, vssym32.h and vsstyle.h

#ifndef _UXTHEME_H_

    struct MARGINS
    {
        int cxLeftWidth;      // width of left border that retains its size
        int cxRightWidth;     // width of right border that retains its size
        int cyTopHeight;      // height of top border that retains its size
        int cyBottomHeight;   // height of bottom border that retains its size
    };

    enum THEMESIZE
    {
        TS_MIN,             // minimum size
        TS_TRUE,            // size without stretching
        TS_DRAW             // size that theme mgr will use to draw part
    };

#endif

    class CMargins : public MARGINS
    {
    public:
        CMargins(int cxLeft, int cxRight, int cyTop, int cyBottom)
        {
            cxLeftWidth    = cxLeft;    cxRightWidth   = cxRight;
            cyTopHeight    = cyTop;     cyBottomHeight = cyBottom;
        }
        CMargins()
        {
            cxLeftWidth    = 0;         cxRightWidth   = 0;
            cyTopHeight    = 0;         cyBottomHeight = 0;
        }
        int Width() const       { return cxLeftWidth + cxRightWidth; }
        int Height() const      { return cyTopHeight + cyBottomHeight; }
        void SetMargins(int cxLeft, int cxRight, int cyTop, int cyBottom)
        {
            cxLeftWidth    = cxLeft;    cxRightWidth   = cxRight;
            cyTopHeight    = cyTop;     cyBottomHeight = cyBottom;
        }
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
        HMENU menu;
        MENUITEMINFO mii;
        UINT  pos;
        std::vector<TCHAR> itemText;

        MenuItemData() : menu(0), pos(0)
        {
            ZeroMemory(&mii, GetSizeofMenuItemInfo());
            itemText.assign(MAX_MENU_STRING, _T('\0'));
        }
        LPTSTR GetItemText() {return &itemText[0];}
    };


    ////////////////////////////////////////////////////////////////////////////////
    // CMenuMetrics is used by CFrameT to retrieve the size of the components
    // used to perform owner-drawing of menu items.  Windows Visual Styles
    // are used to render menu items for systems running Aero (Vista and above).
    // Win32++ custom themes are used to render menu items for systems without Aero.
    class CMenuMetrics
    {
    public:
        CMenuMetrics();
        ~CMenuMetrics();

        CRect GetCheckBackgroundRect(const CRect& item) const;
        CRect GetCheckRect(const CRect& item) const;
        CRect GetGutterRect(const CRect& item) const;
        CSize GetItemSize(MenuItemData* pmd) const;
        CRect GetSelectionRect(const CRect& item) const;
        CRect GetSeperatorRect(const CRect& item) const;
        CRect GetTextRect(const CRect& item) const;
        CSize GetTextSize(MenuItemData* pmd) const;
        void  Initialize(HWND frame);
        BOOL  IsVistaMenu() const;
        CRect ScaleRect(const CRect& item) const;
        CSize ScaleSize(const CSize& item) const;
        int   ToItemStateId(UINT itemState) const;
        int   ToCheckBackgroundStateId(int stateID) const;
        int   ToCheckStateId(UINT type, int stateID) const;

        // Wrappers for Windows API functions
        HRESULT CloseThemeData() const;
        HRESULT DrawThemeBackground(HDC dc, int partID, int stateID, const RECT* pRect, const RECT* pClipRect) const;
        HRESULT DrawThemeText(HDC dc, int partID, int stateID, LPCWSTR text, int charCount, DWORD textFlags, DWORD textFlags2, LPCRECT pRect) const;
        HRESULT GetThemePartSize(HDC dc, int partID, int stateID, LPCRECT prc, THEMESIZE eSize, SIZE* psz) const;
        HRESULT GetThemeInt(int partID, int stateID, int propID, int* pVal) const;
        HRESULT GetThemeMargins(HDC dc, int partID, int stateID, int propID, LPRECT prc, MARGINS* pMargins) const;
        HRESULT GetThemeTextExtent(HDC dc, int partID, int stateID, LPCWSTR text, int charCount, DWORD textFlags, LPCRECT pBoundingRect, LPRECT pExtentRect) const;
        BOOL    IsThemeBackgroundPartiallyTransparent(int partID, int stateID) const;
        HANDLE  OpenThemeData(HWND wnd, LPCWSTR pClassList) const;

        HANDLE  m_theme;                // Theme handle
        HWND    m_frame;                // Handle to the frame window
        HMODULE m_uxTheme;          // Module handle to the UXTheme dll

        CMargins m_marCheck;            // Check margins
        CMargins m_marCheckBackground;  // Check background margins
        CMargins m_marItem;             // Item margins
        CMargins m_marText;             // Text margins

        CSize   m_sizeCheck;            // Check size metric
        CSize   m_sizeSeparator;        // Separator size metric

    private:
        typedef HRESULT WINAPI CLOSETHEMEDATA(HANDLE);
        typedef HRESULT WINAPI DRAWTHEMEBACKGROUND(HANDLE, HDC, int, int, const RECT*, const RECT*);
        typedef HRESULT WINAPI DRAWTHEMETEXT(HANDLE, HDC, int, int, LPCWSTR, int, DWORD, DWORD, LPCRECT);
        typedef HRESULT WINAPI GETTHEMEPARTSIZE(HANDLE, HDC, int, int, LPCRECT, THEMESIZE, SIZE*);
        typedef HRESULT WINAPI GETTHEMEINT(HANDLE, int, int, int, int*);
        typedef HRESULT WINAPI GETTHEMEMARGINS(HANDLE, HDC, int, int, int, LPRECT, MARGINS*);
        typedef HRESULT WINAPI GETTHEMETEXTEXTENT(HANDLE, HDC, int, int, LPCWSTR, int, DWORD, LPCRECT, LPCRECT);
        typedef BOOL    WINAPI ISTHEMEBGPARTTRANSPARENT(HANDLE, int, int);
        typedef HANDLE  WINAPI OPENTHEMEDATA(HWND, LPCWSTR);

        // Pointers to functions defined in uxTheme.dll
        CLOSETHEMEDATA*           m_pfnCloseThemeData;
        DRAWTHEMEBACKGROUND*      m_pfnDrawThemeBackground;
        DRAWTHEMETEXT*            m_pfnDrawThemeText;
        GETTHEMEPARTSIZE*         m_pfnGetThemePartSize;
        GETTHEMEINT*              m_pfnGetThemeInt;
        GETTHEMEMARGINS*          m_pfnGetThemeMargins;
        GETTHEMETEXTEXTENT*       m_pfnGetThemeTextExtent;
        ISTHEMEBGPARTTRANSPARENT* m_pfnIsThemeBGPartTransparent;
        OPENTHEMEDATA*            m_pfnOpenThemeData;
    };


    //////////////////////////////////
    // CFrameT is the base class for all frames in Win32++.
    // The template parameter T is either CWnd or CDocker.
    template <class T>
    class CFrameT : public T
    {
        typedef Shared_Ptr<MenuItemData> ItemDataPtr;

    public:

        struct InitValues
        {
            CRect position;
            int   showCmd;
            BOOL  showStatusBar;
            BOOL  showToolBar;

            // Display StatusBar and ToolBar by default
            InitValues() : showCmd(SW_SHOW), showStatusBar(TRUE), showToolBar(TRUE) {}  // constructor
        };

        CFrameT();
        virtual ~CFrameT();

        // Override these functions as required
        virtual void AdjustFrameRect(const RECT& viewRect);
        virtual CString GetThemeName() const;
        virtual CRect GetViewRect() const;
        virtual BOOL IsMDIChildMaxed() const { return FALSE; }
        virtual BOOL IsMDIFrame() const      { return FALSE; }
        virtual void SetStatusIndicators();
        virtual void RecalcLayout();
        virtual void RecalcViewLayout();

        virtual CWnd& GetView() const        { return *m_pView; }
        virtual void SetView(CWnd& view);

        // Virtual Attributes
        // If you need to modify the default behaviour of the MenuBar, ReBar,
        // StatusBar or ToolBar, inherit from those classes, and override
        // the following attribute functions.
        virtual CMenuBar& GetMenuBar()  const       { return m_menuBar; }
        virtual CReBar& GetReBar() const            { return m_reBar; }
        virtual CStatusBar& GetStatusBar() const    { return m_statusBar; }
        virtual CToolBar& GetToolBar() const        { return m_toolBar; }

        // Non-virtual Attributes
        // These functions aren't virtual, and shouldn't be overridden
        CRect ExcludeChildRect(const CRect& clientRect, HWND child) const;
        HACCEL GetFrameAccel() const                    { return m_accel; }
        CMenu  GetFrameMenu() const                     { return m_menu; }
        const InitValues& GetInitValues() const         { return m_initValues; }
        const MenuTheme& GetMenuBarTheme() const        { return m_mbTheme; }
        const CMenuMetrics& GetMenuMetrics() const      { return m_menuMetrics; }
        const std::vector<CString>& GetMRUEntries() const { return m_mruEntries; }
        CString GetMRUEntry(UINT index);
        UINT GetMRULimit() const                        { return m_maxMRU; }
        CString GetRegistryKeyName() const              { return m_keyName; }
        const ReBarTheme& GetReBarTheme() const         { return m_rbTheme; }
        const StatusBarTheme& GetStatusBarTheme() const { return m_sbTheme; }
        const std::vector<UINT>& GetToolBarData() const { return m_toolBarData; }
        const ToolBarTheme& GetToolBarTheme() const     { return m_tbTheme; }
        CString GetStatusText() const                   { return m_statusText; }
        CString GetTitle() const                        { return T::GetWindowText(); }
        void SetAccelerators(UINT accelID);
        void SetFrameMenu(UINT menuID);
        void SetFrameMenu(HMENU menu);
        void SetInitValues(const InitValues& values);
        void SetKbdHook();
        void SetMenuTheme(const MenuTheme& mt);
        void SetMRULimit(UINT MRULimit);
        void SetReBarTheme(const ReBarTheme& rbt);
        void SetStatusBarTheme(const StatusBarTheme& sbt);
        void SetStatusText(LPCTSTR text);
        void SetTitle(LPCTSTR text)                    { T::SetWindowText(text); }
        void SetToolBarTheme(const ToolBarTheme& tbt);

    protected:
        // Override these functions as required.
        virtual void AddDisabledMenuImage(HICON icon, COLORREF mask, int iconWidth = 16);
        virtual BOOL AddMenuIcon(int menuItemID, int iconID, int iconWidth = 16);
        virtual BOOL AddMenuIcon(int menuItemID, HICON icon, int iconWidth = 16);
        virtual UINT AddMenuIcons(const std::vector<UINT>& menuData, COLORREF mask, UINT bitmapID, UINT disabledID);
        virtual void AddMenuBarBand();
        virtual void AddMRUEntry(LPCTSTR pMRUEntry);
        virtual void AddToolBarBand(CToolBar& tb, DWORD bandStyle, UINT id);
        virtual void AddToolBarButton(UINT id, BOOL isEnabled = TRUE, LPCTSTR text = 0, int image = -1);
        virtual void CreateToolBar();
        virtual LRESULT CustomDrawMenuBar(NMHDR* pNMHDR);
        virtual LRESULT CustomDrawToolBar(NMHDR* pNMHDR);
        virtual void DrawMenuItem(LPDRAWITEMSTRUCT pDIS);
        virtual void DrawMenuItemBkgnd(LPDRAWITEMSTRUCT pDIS);
        virtual void DrawMenuItemCheckmark(LPDRAWITEMSTRUCT pDIS);
        virtual void DrawMenuItemIcon(LPDRAWITEMSTRUCT pDIS);
        virtual void DrawMenuItemText(LPDRAWITEMSTRUCT pDIS);
        virtual BOOL DrawReBarBkgnd(CDC& dc, CReBar& reBar);
        virtual BOOL DrawStatusBarBkgnd(CDC& dc, CStatusBar& statusBar);
        virtual int  GetMenuItemPos(HMENU menu, LPCTSTR pItem) const;
        virtual BOOL LoadRegistrySettings(LPCTSTR pKeyName);
        virtual BOOL LoadRegistryMRUSettings(UINT maxMRU = 0);
        virtual void MeasureMenuItem(MEASUREITEMSTRUCT* pMIS);
        virtual LRESULT OnActivate(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual void OnClose();
        virtual int  OnCreate(CREATESTRUCT& cs);
        virtual LRESULT OnCustomDraw(LPNMHDR pNMHDR);
        virtual void OnDestroy();
        virtual LRESULT OnDrawItem(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual LRESULT OnDrawRBBkgnd(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual LRESULT OnDrawSBBkgnd(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual BOOL OnHelp();
        virtual LRESULT OnInitMenuPopup(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual LRESULT OnMeasureItem(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual LRESULT OnMenuChar(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual LRESULT OnMenuSelect(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual void OnMenuUpdate(UINT id);
        virtual LRESULT OnNotify(WPARAM wparam, LPARAM lparam);
        virtual LRESULT OnRBNHeightChange(LPNMHDR pNMHDR);
        virtual LRESULT OnRBNLayoutChanged(LPNMHDR pNMHDR);
        virtual LRESULT OnRBNMinMax(LPNMHDR pNMHDR);
        virtual LRESULT OnSetFocus(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual LRESULT OnSettingChange(UINT, WPARAM, LPARAM);
        virtual LRESULT OnSize(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual LRESULT OnSysColorChange(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual LRESULT OnSysCommand(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual LRESULT OnTBNDropDown(LPNMTOOLBAR pNMTB);
        virtual LRESULT OnTTNGetDispInfo(LPNMTTDISPINFO pNMTDI);
        virtual LRESULT OnUndocked();
        virtual LRESULT OnUnInitMenuPopup(UINT, WPARAM wparam, LPARAM lparam);
        virtual BOOL OnViewStatusBar();
        virtual BOOL OnViewToolBar();
        virtual void PreCreate(CREATESTRUCT& cs);
        virtual void PreRegisterClass(WNDCLASS& wc);
        virtual void RemoveMRUEntry(LPCTSTR pMRUEntry);
        virtual BOOL SaveRegistryMRUSettings();
        virtual BOOL SaveRegistrySettings();
        virtual void SetMenuBarBandSize();
        virtual UINT SetMenuIcons(const std::vector<UINT>& menuData, COLORREF mask, UINT toolBarID, UINT toolBarDisabledID);
        virtual void SetTBImageList(CToolBar& toolBar, CImageList& imageList, UINT id, COLORREF mask);
        virtual void SetTBImageListDis(CToolBar& toolBar, CImageList& imageList, UINT id, COLORREF mask);
        virtual void SetTBImageListHot(CToolBar& toolBar, CImageList& imageList, UINT id, COLORREF mask);
        virtual void SetupMenuIcons();
        virtual void SetupToolBar();
        virtual void SetTheme();
        virtual void SetToolBarImages(COLORREF mask, UINT toolBarID, UINT toolBarHotID, UINT toolBarDisabledID);
        virtual void ShowMenu(BOOL show);
        virtual void ShowStatusBar(BOOL show);
        virtual void ShowToolBar(BOOL show);
        virtual void UpdateMRUMenu();

        // Not intended to be overridden
        BOOL IsReBarSupported() const { return (GetComCtlVersion() > 470); }
        BOOL IsUsingIndicatorStatus() const { return m_useIndicatorStatus; }
        BOOL IsUsingMenuStatus() const { return m_useMenuStatus; }
        BOOL IsUsingReBar() const { return m_useReBar; }
        BOOL IsUsingStatusBar() const { return m_useStatusBar; }
        BOOL IsUsingThemes() const { return m_useThemes; }
        BOOL IsUsingToolBar() const { return m_useToolBar; }
        BOOL IsUsingVistaMenu() const { return m_menuMetrics.IsVistaMenu(); }
        void UseIndicatorStatus(BOOL useIndicatorStatus) { m_useIndicatorStatus = useIndicatorStatus; }
        void UseMenuStatus(BOOL useMenuStatus) { m_useMenuStatus = useMenuStatus; }
        void UseReBar(BOOL useReBar) { m_useReBar = useReBar; }
        void UseStatusBar(BOOL useStatusBar) { m_useStatusBar = useStatusBar; }
        void UseThemes(BOOL useThemes) { m_useThemes = useThemes; }
        void UseToolBar(BOOL useToolBar) { m_useToolBar = useToolBar; }

        LRESULT WndProcDefault(UINT msg, WPARAM wparam, LPARAM lparam);

    private:
        CFrameT(const CFrameT&);                // Disable copy construction
        CFrameT& operator = (const CFrameT&);   // Disable assignment operator
        CSize GetTBImageSize(CBitmap* pBitmap);
        void UpdateMenuBarBandSize();
        static LRESULT CALLBACK StaticKeyboardProc(int code, WPARAM wparam, LPARAM lparam);

        std::vector<ItemDataPtr> m_menuItemData;   // vector of MenuItemData pointers
        std::vector<CString> m_mruEntries;  // Vector of CStrings for MRU entries
        std::vector<UINT> m_menuIcons;      // vector of menu icon resource IDs
        std::vector<UINT> m_toolBarData;    // vector of resource IDs for ToolBar buttons
        InitValues m_initValues;            // struct of initial values
        CDialog m_aboutDialog;              // Help about dialog
        mutable CMenuBar m_menuBar;         // CMenuBar object
        mutable CReBar m_reBar;             // CReBar object
        mutable CStatusBar m_statusBar;     // CStatusBar object
        mutable CToolBar m_toolBar;         // CToolBar object
        CMenu m_menu;                       // handle to the frame menu
        CFont m_menuBarFont;                // MenuBar font
        CFont m_statusBarFont;              // StatusBar font
        CImageList m_toolBarImages;         // Image list for the ToolBar buttons
        CImageList m_toolBarDisabledImages; // Image list for the Disabled ToolBar buttons
        CImageList m_toolBarHotImages;      // Image list for the Hot ToolBar buttons
        CString m_oldStatus[3];             // Array of CString holding old status;
        CString m_keyName;                  // CString for Registry key name
        CString m_statusText;               // CString for status text
        CString m_tooltip;                  // CString for tool tips
        CString m_xpThemeName;              // CString for Windows Theme Name
        MenuTheme m_mbTheme;                // struct of theme info for the popup Menu and MenuBar
        ReBarTheme m_rbTheme;               // struct of theme info for the ReBar
        StatusBarTheme m_sbTheme;           // struct of theme info for the StatusBar
        ToolBarTheme m_tbTheme;             // struct of theme info for the ToolBar
        HACCEL m_accel;                     // handle to the frame's accelerator table (used by MDI without MDI child)
        CWnd* m_pView;                      // pointer to the View CWnd object
        UINT m_maxMRU;                      // maximum number of MRU entries
        HWND m_oldFocus;                    // The window which had focus prior to the app's deactivation
        BOOL m_drawArrowBkgrnd;             // True if a separate arrow background is to be drawn on toolbar
        HHOOK m_kbdHook;                    // Keyboard hook.

        CMenuMetrics m_menuMetrics;         // The MenuMetrics object
        CImageList m_menuImages;            // Imagelist of menu icons
        CImageList m_menuDisabledImages;    // Imagelist of disabled menu icons
        BOOL m_useIndicatorStatus;          // set to TRUE to see indicators in status bar
        BOOL m_useMenuStatus;               // set to TRUE to see menu and toolbar updates in status bar
        BOOL m_useReBar;                    // set to TRUE if ReBars are to be used
        BOOL m_useStatusBar;                // set to TRUE if the statusbar is used
        BOOL m_useThemes;                   // set to TRUE if themes are to be used
        BOOL m_useToolBar;                  // set to TRUE if the toolbar is used

    };  // class CFrameT


    /////////////////////////////////////////
    // CFrame manages the frame window. CFrame also manages the
    // creation and position of child windows, such as the menubar,
    // toolbar, view window and statusbar.
    class CFrame : public CFrameT<CWnd>
    {
    public:
        CFrame() {}
        virtual ~CFrame() {}

    private:
        CFrame(const CFrame&);              // Disable copy construction
        CFrame& operator = (const CFrame&); // Disable assignment operator
    };

}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


namespace Win32xx
{

    /////////////////////////////////////////
    // Definitions for the CMenuMetrics class
    //
    inline CMenuMetrics::CMenuMetrics() : m_theme(0), m_uxTheme(0), m_pfnCloseThemeData(0), m_pfnDrawThemeBackground(0),
                                            m_pfnDrawThemeText(0), m_pfnGetThemePartSize(0), m_pfnGetThemeInt(0), m_pfnGetThemeMargins(0),
                                            m_pfnGetThemeTextExtent(0), m_pfnIsThemeBGPartTransparent(0), m_pfnOpenThemeData(0)
    {
        m_frame = 0;
    }

    inline CMenuMetrics::~CMenuMetrics()
    {
        if (m_theme != 0)
            CloseThemeData();

        if (m_uxTheme != 0)
            ::FreeLibrary(m_uxTheme);
    }

    // Closes the theme data handle.
    inline HRESULT CMenuMetrics::CloseThemeData() const
    {
        if (m_pfnCloseThemeData)
            return m_pfnCloseThemeData(m_theme);

        return E_NOTIMPL;
    }

    // Draws the border and fill defined by the visual style for the specified control part.
    inline HRESULT CMenuMetrics::DrawThemeBackground(HDC dc, int partID, int stateID, const RECT *pRect, const RECT *pClipRect) const
    {
        assert(m_theme);
        if (m_pfnDrawThemeBackground)
            return m_pfnDrawThemeBackground(m_theme, dc, partID, stateID, pRect, pClipRect);

        return E_NOTIMPL;
    }

    // Draws text using the color and font defined by the visual style.
    inline HRESULT CMenuMetrics::DrawThemeText(HDC dc, int partID, int stateID, LPCWSTR text, int charCount, DWORD textFlags, DWORD textFlags2, LPCRECT pRect) const
    {
        assert(m_theme);
        if (m_pfnDrawThemeText)
            return m_pfnDrawThemeText(m_theme, dc, partID, stateID, text, charCount, textFlags, textFlags2, pRect);

        return E_NOTIMPL;
    }

    inline CRect CMenuMetrics::GetCheckBackgroundRect(const CRect& item) const
    {
        int cx = m_sizeCheck.cx + m_marCheck.Width();
        int cy = m_sizeCheck.cy + m_marCheck.Height();

        int x = item.left + m_marCheckBackground.cxLeftWidth;
        int y = item.top + (item.Height() - cy) / 2;

        return CRect(x, y, x + cx, y + cy);
    }

    inline CRect CMenuMetrics::GetGutterRect(const CRect& item) const
    {
        int x = item.left;
        int y = item.top;
        int cx = m_marItem.cxLeftWidth + m_marCheckBackground.Width() + m_marCheck.Width() + m_sizeCheck.cx;
        int cy = item.Height();

        return CRect(x, y, x + cx, y + cy);
    }

    inline CRect CMenuMetrics::GetCheckRect(const CRect& item) const
    {
        int x = item.left + m_marCheckBackground.cxLeftWidth + m_marCheck.cxLeftWidth;
        int y = item.top + (item.Height() - m_sizeCheck.cy) / 2;

        return CRect(x, y, x + m_sizeCheck.cx, y + m_sizeCheck.cy);
    }

    // Retrieve the size of the menu item
    inline CSize CMenuMetrics::GetItemSize(MenuItemData* pmd) const
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

    inline CRect CMenuMetrics::GetSelectionRect(const CRect& item) const
    {
        int x = item.left + m_marItem.cxLeftWidth;
        int y = item.top;

        return CRect(x, y, item.right - m_marItem.cxRightWidth, y + item.Height());
    }

    inline CRect CMenuMetrics::GetSeperatorRect(const CRect& item) const
    {
        int left = GetGutterRect(item).right;
        int top = item.top;
        int right = item.right - m_marItem.cxRightWidth;
        int bottom = item.top + m_sizeSeparator.cy;

        return CRect(left, top, right, bottom);
    }

    inline CSize CMenuMetrics::GetTextSize(MenuItemData* pmd) const
    {
        CSize sizeText;
        assert(m_frame);
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
            NONCLIENTMETRICS info = GetNonClientMetrics();

            // Default menu items are bold, so take this into account
            if (static_cast<int>(::GetMenuDefaultItem(pmd->menu, TRUE, GMDI_USEDISABLED)) != -1)
                info.lfMenuFont.lfWeight = FW_BOLD;

            // Calculate the size of the text
            DesktopDC.CreateFontIndirect(info.lfMenuFont);
            sizeText = DesktopDC.GetTextExtentPoint32(szItemText, lstrlen(szItemText));
            sizeText.cx += m_marText.cxRightWidth;
            sizeText.cy += m_marText.Height();
        }

        if (_tcschr(szItemText, _T('\t')))
            sizeText.cx += 8;   // Add POST_TEXT_GAP if the text includes a tab

        return sizeText;
    }

    inline CRect CMenuMetrics::GetTextRect(const CRect& item) const
    {
        int left = GetGutterRect(item).Width() + m_marText.cxLeftWidth;
        int top = item.top + m_marText.cyTopHeight;
        int right = item.right - m_marItem.cxRightWidth - m_marText.cxRightWidth;
        int bottom = item.bottom - m_marText.cyBottomHeight;

        return CRect(left, top, right, bottom);
    }

    // Calculates the original size of the part defined by a visual style.
    inline HRESULT CMenuMetrics::GetThemePartSize(HDC dc, int partID, int stateID, LPCRECT prc, THEMESIZE eSize, SIZE* psz) const
    {
        assert(m_theme);
        if (m_pfnGetThemePartSize)
            return m_pfnGetThemePartSize(m_theme, dc, partID, stateID, prc, eSize, psz);

        return E_NOTIMPL;
    }

    // Retrieves the value of an int property.
    inline HRESULT CMenuMetrics::GetThemeInt(int partID, int stateID, int propID, int* pVal) const
    {
        assert(m_theme);
        if (m_pfnGetThemeInt)
            return m_pfnGetThemeInt(m_theme, partID, stateID, propID, pVal);

        return E_NOTIMPL;
    }

    // Retrieves the value of a MARGINS property.
    inline HRESULT CMenuMetrics::GetThemeMargins(HDC dc, int partID, int stateID, int propID, LPRECT prc, MARGINS* pMargins) const
    {
        assert(m_theme);
        if (m_pfnGetThemeMargins)
            return m_pfnGetThemeMargins(m_theme, dc, partID, stateID, propID, prc, pMargins);

        return E_NOTIMPL;
    }

    // Calculates the size and location of the specified text when rendered in the visual style font.
    inline HRESULT CMenuMetrics::GetThemeTextExtent(HDC dc, int partID, int stateID, LPCWSTR text, int charCount, DWORD textFlags, LPCRECT pBoundingRect, LPRECT pExtentRect) const
    {
        assert(m_theme);
        if (m_pfnGetThemeTextExtent)
            return m_pfnGetThemeTextExtent(m_theme, dc, partID, stateID, text, charCount, textFlags, pBoundingRect, pExtentRect);

        return E_NOTIMPL;
    }

    inline void CMenuMetrics::Initialize(HWND frame)
    {
        assert(IsWindow(frame));
        m_frame = frame;

        if (m_uxTheme == 0)
            m_uxTheme = ::LoadLibrary(_T("UXTHEME.DLL"));

        if (m_uxTheme != 0)
        {
            m_pfnCloseThemeData = reinterpret_cast<CLOSETHEMEDATA*>(::GetProcAddress(m_uxTheme, "CloseThemeData"));
            m_pfnDrawThemeBackground = reinterpret_cast<DRAWTHEMEBACKGROUND*>(::GetProcAddress(m_uxTheme, "DrawThemeBackground"));
            m_pfnDrawThemeText = reinterpret_cast<DRAWTHEMETEXT*>(::GetProcAddress(m_uxTheme, "DrawThemeText"));
            m_pfnGetThemePartSize = reinterpret_cast<GETTHEMEPARTSIZE*>(::GetProcAddress(m_uxTheme, "GetThemePartSize"));
            m_pfnGetThemeInt = reinterpret_cast<GETTHEMEINT*>(::GetProcAddress(m_uxTheme, "GetThemeInt"));
            m_pfnGetThemeMargins = reinterpret_cast<GETTHEMEMARGINS*>(::GetProcAddress(m_uxTheme, "GetThemeMargins"));
            m_pfnGetThemeTextExtent = reinterpret_cast<GETTHEMETEXTEXTENT*>(::GetProcAddress(m_uxTheme, "GetThemeTextExtent"));
            m_pfnIsThemeBGPartTransparent = reinterpret_cast<ISTHEMEBGPARTTRANSPARENT*>(::GetProcAddress(m_uxTheme, "IsThemeBackgroundPartiallyTransparent"));
            m_pfnOpenThemeData = reinterpret_cast<OPENTHEMEDATA*>(::GetProcAddress(m_uxTheme, "OpenThemeData"));
        }

        if (m_theme != 0)
        {
            CloseThemeData();
            m_theme = 0;
        }

        m_theme = OpenThemeData(m_frame, VSCLASS_MENU);

        if (m_theme != 0)
        {
            int borderSize = 0;    // Border space between item text and accelerator
            int bgBorderSize = 0;  // Border space between item text and gutter
            GetThemePartSize(NULL, MENU_POPUPCHECK, 0, NULL, TS_TRUE, &m_sizeCheck);
            GetThemePartSize(NULL, MENU_POPUPSEPARATOR, 0, NULL, TS_TRUE, &m_sizeSeparator);
            GetThemeInt(MENU_POPUPITEM, 0, TMT_BORDERSIZE, &borderSize);
            GetThemeInt(MENU_POPUPBACKGROUND, 0, TMT_BORDERSIZE, &bgBorderSize);
            GetThemeMargins(NULL, MENU_POPUPCHECK, 0, TMT_CONTENTMARGINS, NULL, &m_marCheck);
            GetThemeMargins(NULL, MENU_POPUPCHECKBACKGROUND, 0, TMT_CONTENTMARGINS, NULL, &m_marCheckBackground);
            GetThemeMargins(NULL, MENU_POPUPITEM, 0, TMT_CONTENTMARGINS, NULL, &m_marItem);

            // Popup text margins
            m_marText = m_marItem;
            m_marText.cxRightWidth = borderSize;
            m_marText.cxLeftWidth = bgBorderSize;
        }
        else
        {
            m_sizeCheck.SetSize(::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON));
            m_sizeSeparator.SetSize(1, 7);
            m_marCheck.SetMargins(4, 4, 2, 2);
            m_marCheckBackground.SetMargins(0, 0, 0, 0);
            m_marItem.SetMargins(0, 0, 0, 0);
            m_marText.SetMargins(8, 16, 0, 0);
        }
    }

    // Retrieves whether the background specified by the visual style has transparent pieces or alpha-blended pieces.
    inline BOOL CMenuMetrics::IsThemeBackgroundPartiallyTransparent(int partID, int stateID) const
    {
        assert(m_theme);
        if (m_pfnIsThemeBGPartTransparent)
            return m_pfnIsThemeBGPartTransparent(m_theme, partID, stateID);

        return FALSE;
    }

    // Opens the theme data for a window and its associated class.
    inline HANDLE CMenuMetrics::OpenThemeData(HWND wnd, LPCWSTR pClassList) const
    {
        assert(wnd);
        if (m_pfnOpenThemeData)
            return m_pfnOpenThemeData(wnd, pClassList);

        return NULL;
    }

    inline BOOL CMenuMetrics::IsVistaMenu() const
    {
        return (m_theme != FALSE);
    }

    // Re-scale the CRect to support the system's DPI
    inline CRect CMenuMetrics::ScaleRect(const CRect& item) const
    {
        // DC for the desktop
        CWindowDC dc(NULL);

        int dpiX = dc.GetDeviceCaps(LOGPIXELSX);
        int dpiY = dc.GetDeviceCaps(LOGPIXELSY);

        CRect rc  = item;
        rc.left   = MulDiv(rc.left, dpiX, 96);
        rc.right  = MulDiv(rc.right, dpiX, 96);
        rc.top    = MulDiv(rc.top, dpiY, 96);
        rc.bottom = MulDiv(rc.bottom, dpiY, 96);

        return rc;
    }

    // Re-scale the CSize to support the system's DPI
    inline CSize CMenuMetrics::ScaleSize(const CSize& item) const
    {
        // DC for the desktop
        CWindowDC dc(NULL);

        int dpiX = dc.GetDeviceCaps(LOGPIXELSX);
        int dpiY = dc.GetDeviceCaps(LOGPIXELSY);

        CSize sz = item;
        sz.cx = MulDiv(sz.cx, dpiX, 96);
        sz.cy = MulDiv(sz.cy, dpiY, 96);

        return sz;
    }

    // Convert from item state to MENU_POPUPITEM state
    inline int CMenuMetrics::ToItemStateId(UINT itemState) const
    {
        const bool      isDisabled   = ((itemState & (ODS_INACTIVE | ODS_DISABLED)) != 0);
        const bool      isHot        = ((itemState & (ODS_HOTLIGHT | ODS_SELECTED)) != 0);
        POPUPITEMSTATES state;

        if (isDisabled)
            state = (isHot ? MPI_DISABLEDHOT : MPI_DISABLED);
        else if (isHot)
            state = MPI_HOT;
        else
            state= MPI_NORMAL;

        return state;
    }

    // Convert to MENU_POPUPCHECKBACKGROUND
    inline int CMenuMetrics::ToCheckBackgroundStateId(int stateID) const
    {
        POPUPCHECKBACKGROUNDSTATES stateIdCheckBackground;

        // Determine the check background state.
        if (stateID == MPI_DISABLED || stateID == MPI_DISABLEDHOT)
            stateIdCheckBackground = MCB_DISABLED;
        else
            stateIdCheckBackground = MCB_NORMAL;

        return stateIdCheckBackground;
    }

    // Convert to MENU_POPUPCHECK state
    inline int CMenuMetrics::ToCheckStateId(UINT type, int stateID) const
    {
        POPUPCHECKSTATES stateIdCheck;

        if (type & MFT_RADIOCHECK)
        {
            if (stateID == MPI_DISABLED || stateID == MPI_DISABLEDHOT)
                stateIdCheck = MC_BULLETDISABLED;
            else
                stateIdCheck = MC_BULLETNORMAL;
        }
        else
        {
            if (stateID == MPI_DISABLED || stateID == MPI_DISABLEDHOT)
                stateIdCheck = MC_CHECKMARKDISABLED;
            else
                stateIdCheck = MC_CHECKMARKNORMAL;
        }

        return stateIdCheck;
    }


    ///////////////////////////////////
    // Definitions for the CFrame class
    //
    template <class T>
    inline CFrameT<T>::CFrameT() : m_aboutDialog(IDW_ABOUT), m_accel(0), m_pView(NULL), m_maxMRU(0), m_oldFocus(0),
                              m_drawArrowBkgrnd(FALSE), m_kbdHook(0), m_useIndicatorStatus(TRUE),
                              m_useMenuStatus(TRUE), m_useStatusBar(TRUE), m_useThemes(TRUE), m_useToolBar(TRUE)
    {
        ZeroMemory(&m_mbTheme, sizeof(m_mbTheme));
        ZeroMemory(&m_rbTheme, sizeof(m_rbTheme));
        ZeroMemory(&m_sbTheme, sizeof(m_sbTheme));
        ZeroMemory(&m_tbTheme, sizeof(m_tbTheme));

        m_statusText = LoadString(IDW_READY);

        // Load the common controls. Uses InitCommonControlsEx.
        LoadCommonControls();

        // By default, we use the rebar if we can.
        m_useReBar = (GetComCtlVersion() > 470)? TRUE : FALSE;

        // Set the fonts.
        NONCLIENTMETRICS info = GetNonClientMetrics();
        m_menuBarFont.CreateFontIndirect(info.lfMenuFont);
        m_statusBarFont.CreateFontIndirect(info.lfStatusFont);
    }

    template <class T>
    inline CFrameT<T>::~CFrameT()
    {
        if (m_kbdHook != 0) UnhookWindowsHookEx(m_kbdHook);
    }

    // Adds the grayscale image of the specified icon the disabled menu image-list.
    // This function is called by AddMenuIcon.
    template <class T>
    inline void CFrameT<T>::AddDisabledMenuImage(HICON icon, COLORREF mask, int iconWidth)
    {
        CClientDC desktopDC(NULL);
        CMemDC memDC(NULL);
        int cx = iconWidth;
        int cy = iconWidth;

        memDC.CreateCompatibleBitmap(desktopDC, cx, cy);
        CRect rc;
        rc.SetRect(0, 0, cx, cy);

        // Set the mask color to grey for the new ImageList
        if (GetDeviceCaps(desktopDC, BITSPIXEL) < 24)
        {
            HPALETTE hPal = reinterpret_cast<HPALETTE>(GetCurrentObject(desktopDC, OBJ_PAL));
            UINT index = GetNearestPaletteIndex(hPal, mask);
            if (index != CLR_INVALID) mask = PALETTEINDEX(index);
        }

        memDC.SolidFill(mask, rc);

        // Draw the icon on the memory DC
        memDC.DrawIconEx(0, 0, icon, cx, cy, 0, 0, DI_NORMAL);

        // Detach the bitmap so we can use it.
        CBitmap bitmap = memDC.DetachBitmap();
        bitmap.ConvertToDisabled(mask);

        if (m_menuDisabledImages.GetHandle() == 0)
            m_menuDisabledImages.Create(cx, cy, ILC_COLOR24 | ILC_MASK, 1, 0);

        m_menuDisabledImages.Add(bitmap, mask);
    }

    // Adds an icon to an internal ImageList for use with popup menu items.
    template <class T>
    inline BOOL CFrameT<T>::AddMenuIcon(int menuItemID, int iconID, int iconWidth /* = 16*/)
    {
        int cx = iconWidth;
        int cy = iconWidth;
        HICON icon = static_cast<HICON>(GetApp()->LoadImage(iconID, IMAGE_ICON, cx, cy, LR_SHARED));
        return AddMenuIcon(menuItemID, icon, iconWidth);
    }

    // Adds an icon to an internal ImageList for use with popup menu items.
    template <class T>
    inline BOOL CFrameT<T>::AddMenuIcon(int menuItemID, HICON icon, int iconWidth /* = 16*/)
    {
        int cxImage = iconWidth;
        int cyImage = iconWidth;

        // Create a new ImageList if required.
        if (NULL == m_menuImages.GetHandle())
        {
            m_menuImages.Create(cxImage, cyImage, ILC_COLOR32 | ILC_MASK, 1, 0);
            m_menuIcons.clear();
        }

        if (m_menuImages.Add(icon) != -1)
        {
            m_menuIcons.push_back(menuItemID);

            // Set the mask color to grey for the new ImageList
            COLORREF mask = RGB(200, 200, 200);
            CClientDC desktopDC(HWND_DESKTOP);
            if (GetDeviceCaps(desktopDC, BITSPIXEL) < 24)
            {
                HPALETTE hPal = reinterpret_cast<HPALETTE>(GetCurrentObject(desktopDC, OBJ_PAL));
                UINT index = GetNearestPaletteIndex(hPal, mask);
                if (index != CLR_INVALID) mask = PALETTEINDEX(index);
            }

            AddDisabledMenuImage(icon, mask, iconWidth);

            return TRUE;
        }

        return FALSE;
    }

    // Adds the icons from a bitmap resource to an internal ImageList for use with popup menu items.
    // Note:  Images for menu icons should be sized 16x16 or 16x15 pixels. Larger images are ignored.
    template <class T>
    inline UINT CFrameT<T>::AddMenuIcons(const std::vector<UINT>& menuData, COLORREF mask, UINT bitmapID, UINT disabledID)
    {
        // Count the MenuData entries excluding separators.
        int images = 0;
        for (UINT i = 0 ; i < menuData.size(); ++i)
        {
            if (menuData[i] != 0)   // Don't count separators
                ++images;
        }

        // Load the button images from Resource ID.
        CBitmap bitmap(bitmapID);

        if ((0 == images) || (!bitmap))
            return static_cast<UINT>(m_menuIcons.size());  // No valid images, so nothing to do!

        BITMAP data = bitmap.GetBitmapData();
        int cxImage = MAX(data.bmHeight, 16);
        int cyImage = data.bmHeight;

        // Create the ImageList if required.
        if (NULL == m_menuImages.GetHandle())
        {
            m_menuImages.Create(cxImage, cyImage, ILC_COLOR32 | ILC_MASK, images, 0);
            m_menuIcons.clear();
        }

        // Add the resource IDs to the m_menuIcons vector.
        std::vector<UINT>::const_iterator iter;
        for (iter = menuData.begin(); iter != menuData.end(); ++iter)
        {
            if ((*iter) != 0)
                m_menuIcons.push_back(*iter);
        }

        // Add the images to the ImageList.
        m_menuImages.Add(bitmap, mask);

        // Create the Disabled imagelist.
        if (disabledID != 0)
        {
            m_menuDisabledImages.DeleteImageList();
            m_menuDisabledImages.Create(cxImage, cyImage, ILC_COLOR32 | ILC_MASK, images, 0);

            CBitmap disabled(disabledID);

            m_menuDisabledImages.Add(disabled, mask);
        }
        else
        {
            m_menuDisabledImages.DeleteImageList();
            m_menuDisabledImages.CreateDisabledImageList(m_menuImages);
        }

        // return the number of menu icons.
        return static_cast<UINT>(m_menuIcons.size());
    }

    // Adds a MenuBar to the rebar control.
    template <class T>
    inline void CFrameT<T>::AddMenuBarBand()
    {
        REBARBANDINFO rbbi;
        ZeroMemory(&rbbi, sizeof(rbbi));

        rbbi.fMask      = RBBIM_STYLE | RBBIM_CHILD | RBBIM_ID;
        rbbi.fStyle     = RBBS_BREAK | RBBS_VARIABLEHEIGHT;
        rbbi.hwndChild  = GetMenuBar();
        rbbi.wID        = IDW_MENUBAR;

        // Note: rbbi.cbSize is set inside the InsertBand function.
        GetReBar().InsertBand(-1, rbbi);
        GetReBar().SetMenuBar(GetMenuBar());

        if (GetReBarTheme().LockMenuBand)
            GetReBar().ShowGripper(GetReBar().GetBand(GetMenuBar()), FALSE);

        UpdateMenuBarBandSize();
    }

    // Adds an entry to the Most Recently Used (MRU) list.
    template <class T>
    inline void CFrameT<T>::AddMRUEntry(LPCTSTR mruEntry)
    {
        // Erase possible duplicate entries from vector.
        RemoveMRUEntry(mruEntry);

        // Insert the entry at the beginning of the vector.
        m_mruEntries.insert(m_mruEntries.begin(), mruEntry);

        // Delete excessive MRU entries.
        if (m_mruEntries.size() > m_maxMRU)
            m_mruEntries.erase(m_mruEntries.begin() + m_maxMRU, m_mruEntries.end());

        UpdateMRUMenu();
    }

    // Adds a ToolBar to the rebar control.
    template <class T>
    inline void CFrameT<T>::AddToolBarBand(CToolBar& tb, DWORD bandStyle, UINT id)
    {
        // Create the ToolBar Window.
        DWORD style = WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | TBSTYLE_TOOLTIPS | TBSTYLE_FLAT | CCS_NODIVIDER | CCS_NORESIZE | CCS_NOPARENTALIGN;
        tb.CreateEx(0, TOOLBARCLASSNAME, 0, style, CRect(0,0,0,0), GetReBar(), 0);

        // Fill the REBARBAND structure.
        REBARBANDINFO rbbi;
        ZeroMemory(&rbbi, sizeof(rbbi));

        rbbi.fMask      = RBBIM_STYLE |  RBBIM_CHILD | RBBIM_ID;
        rbbi.fStyle     = bandStyle;
        rbbi.hwndChild  = tb;
        rbbi.wID        = id;

        // Note: rbbi.cbSize is set inside the InsertBand function.
        GetReBar().InsertBand(-1, rbbi);
    }

    // Adds Resource IDs to toolbar buttons.
    // A resource ID of 0 is a separator.
    template <class T>
    inline void CFrameT<T>::AddToolBarButton(UINT id, BOOL isEnabled /* = TRUE*/, LPCTSTR text /* = 0 */, int image /* = -1 */)
    {
        m_toolBarData.push_back(id);

        GetToolBar().AddButton(id, isEnabled, image);

        if (0 != text)
            GetToolBar().SetButtonText(id, text);
    }

    // Adjust the size of the frame to accommodate the View window's dimensions.
    template <class T>
    inline void CFrameT<T>::AdjustFrameRect(const RECT& viewRect)
    {
        // Adjust for the view styles
        CRect rc = viewRect;
        DWORD style = GetView().GetStyle();
        DWORD exStyle = GetView().GetExStyle();
        AdjustWindowRectEx(&rc, style, FALSE, exStyle);

        // Calculate the new frame height
        CRect frameBefore = T::GetWindowRect();
        CRect viewBefore = GetViewRect();
        int height = rc.Height() + frameBefore.Height() - viewBefore.Height();

        // Adjust for the frame styles
        style = T::GetStyle();
        exStyle = T::GetExStyle();
        AdjustWindowRectEx(&rc, style, FALSE, exStyle);

        // Calculate final rect size, and reposition frame
        T::SetWindowPos(NULL, 0, 0, rc.Width(), height, SWP_NOMOVE);
    }

    // Creates the frame's toolbar. Additional toolbars can be added with AddToolBarBand
    // if the frame uses a rebar.
    template <class T>
    inline void CFrameT<T>::CreateToolBar()
    {
        if (GetReBar().IsWindow())
            AddToolBarBand(GetToolBar(), RBBS_BREAK|RBBS_GRIPPERALWAYS, IDW_TOOLBAR);   // Create the toolbar inside rebar
        else
            GetToolBar().Create(*this); // Create the toolbar without a rebar.

        // Set a default ImageList for the ToolBar.
        CBitmap bm(IDW_MAIN);
        if (bm.GetHandle() != 0)
            SetToolBarImages(RGB(192,192,192), IDW_MAIN, 0, 0);

        SetupToolBar();

        if (GetToolBarData().size() == 0)
        {
            TRACE("Warning ... No resource IDs assigned to the toolbar\n");
        }

        if (GetReBar().IsWindow())
        {
            SIZE MaxSize = GetToolBar().GetMaxSize();
            GetReBar().SendMessage(UWM_TBRESIZE, (WPARAM)(GetToolBar().GetHwnd()),
                (LPARAM)(&MaxSize));

            if (GetReBarTheme().UseThemes && GetReBarTheme().LockMenuBand)
            {
                // Hide gripper for single toolbar.
                if (GetReBar().GetBandCount() <= 2)
                    GetReBar().ShowGripper(GetReBar().GetBand(GetToolBar()), FALSE);
            }
        }
    }

    // CustomDraw is used to render the MenuBar's toolbar buttons
    template <class T>
    inline LRESULT CFrameT<T>::CustomDrawMenuBar(NMHDR* pNMHDR)
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
                CRect rc = lpNMCustomDraw->nmcd.rc;
                int state = lpNMCustomDraw->nmcd.uItemState;
                DWORD item = static_cast<DWORD>(lpNMCustomDraw->nmcd.dwItemSpec);

                if (IsMDIChildMaxed() && (item == 0))
                // Draw over MDI Max button
                {
                    CDC drawDC(lpNMCustomDraw->nmcd.hdc);
                    CWnd* pActiveChild = GetMenuBar().GetActiveMDIChild();
                    assert(pActiveChild);
                    if (pActiveChild)
                    {
                        HICON icon = reinterpret_cast<HICON>(pActiveChild->SendMessage(WM_GETICON, ICON_SMALL, 0));
                        if (NULL == icon)
                            icon = GetApp()->LoadStandardIcon(IDI_APPLICATION);

                        int cx = ::GetSystemMetrics(SM_CXSMICON);
                        int cy = ::GetSystemMetrics(SM_CYSMICON);
                        int y = 1 + (GetMenuBar().GetWindowRect().Height() - cy) / 2;
                        int x = (rc.Width() - cx) / 2;
                        drawDC.DrawIconEx(x, y, icon, cx, cy, 0, NULL, DI_NORMAL);
                    }
                    return CDRF_SKIPDEFAULT;  // No further drawing
                }

                if (GetMenuBarTheme().UseThemes)
                {
                    // Leave a pixel gap above and below the drawn rectangle.
                    if (IsUsingVistaMenu())
                        rc.InflateRect(0, -2);
                    else
                        rc.InflateRect(0, -1);

                    CDC drawDC(lpNMCustomDraw->nmcd.hdc);
                    if (state & (CDIS_HOT | CDIS_SELECTED))
                    {

                        if ((state & CDIS_SELECTED) || (GetMenuBar().GetButtonState(item) & TBSTATE_PRESSED))
                        {
                            drawDC.GradientFill(GetMenuBarTheme().clrPressed1, GetMenuBarTheme().clrPressed2, rc, FALSE);
                        }
                        else if (state & CDIS_HOT)
                        {
                            drawDC.GradientFill(GetMenuBarTheme().clrHot1, GetMenuBarTheme().clrHot2, rc, FALSE);
                        }

                        // Draw border.
                        CPen Pen(PS_SOLID, 1, GetMenuBarTheme().clrOutline);
                        CPen oldPen = drawDC.SelectObject(Pen);
                        drawDC.MoveTo(rc.left, rc.bottom);
                        drawDC.LineTo(rc.left, rc.top);
                        drawDC.LineTo(rc.right-1, rc.top);
                        drawDC.LineTo(rc.right-1, rc.bottom);
                        drawDC.MoveTo(rc.right-1, rc.bottom);
                        drawDC.LineTo(rc.left, rc.bottom);
                        drawDC.SelectObject(oldPen);
                    }

                    int itemID = static_cast<int>(lpNMCustomDraw->nmcd.dwItemSpec);
                    CString str = GetMenuBar().GetButtonText(itemID);

                    // Draw highlight text.
                    CFont Font = GetMenuBar().GetFont();
                    CFont oldFont = drawDC.SelectObject(Font);

                    rc.bottom += 1;
                    drawDC.SetBkMode(TRANSPARENT);
                    drawDC.DrawText(str, str.GetLength(), rc, DT_VCENTER | DT_CENTER | DT_SINGLELINE);
                    drawDC.SelectObject(oldFont);

                    return CDRF_SKIPDEFAULT;  // No further drawing
                }
            }
            return CDRF_DODEFAULT ;   // Do default drawing

        // Painting cycle has completed.
        case CDDS_POSTPAINT:
            // Draw MDI Minimise, Restore and Close buttons.
            {
                CDC dc(lpNMCustomDraw->nmcd.hdc);
                GetMenuBar().DrawAllMDIButtons(dc);
            }
            break;
        }

        return 0;
    }

    // With CustomDraw we manually control the drawing of each toolbar button.
    template <class T>
    inline LRESULT CFrameT<T>::CustomDrawToolBar(NMHDR* pNMHDR)
    {
        if ((GetToolBarTheme().UseThemes) && (GetComCtlVersion() > 470))
        {
            LPNMTBCUSTOMDRAW pCustomDraw = (LPNMTBCUSTOMDRAW)pNMHDR;
            CToolBar* pTB = static_cast<CToolBar*>(T::GetCWndPtr(pNMHDR->hwndFrom));

            if (pTB)
            {
                switch (pCustomDraw->nmcd.dwDrawStage)
                {
                // Begin paint cycle.
                case CDDS_PREPAINT:
                    // Send NM_CUSTOMDRAW item draw, and post-paint notification messages.
                    return CDRF_NOTIFYITEMDRAW | CDRF_NOTIFYPOSTPAINT ;

                // An item is about to be drawn.
                case CDDS_ITEMPREPAINT:
                    {
                        CDC drawDC(pCustomDraw->nmcd.hdc);
                        CRect rc = pCustomDraw->nmcd.rc;
                        int state = pCustomDraw->nmcd.uItemState;
                        DWORD item = static_cast<DWORD>(pCustomDraw->nmcd.dwItemSpec);
                        DWORD tbStyle = static_cast<DWORD>(pTB->SendMessage(TB_GETSTYLE, 0, 0));
                        int style = pTB->GetButtonStyle(item);
                        int button = pTB->CommandToIndex(item);
                        TBBUTTON tbb;
                        ZeroMemory(&tbb, sizeof(tbb));
                        pTB->SendMessage(TB_GETBUTTON, (WPARAM)button, (LPARAM)(&tbb));
                        int image = static_cast<int>(tbb.iBitmap);

                        // Calculate text size.
                        CString str;
                        CSize textSize;
                        if (pTB->HasText()) // Does any button have text?
                        {
                            drawDC.SelectObject(pTB->GetFont());
                            str = pTB->GetButtonText(item);
                            textSize = drawDC.GetTextExtentPoint32(str, str.GetLength());
                        }


                        // Draw outline rectangle.
                        if (state & (CDIS_HOT | CDIS_SELECTED | CDIS_CHECKED))
                        {
                            drawDC.CreatePen(PS_SOLID, 1, GetToolBarTheme().clrOutline);
                            drawDC.MoveTo(rc.left, rc.top);
                            drawDC.LineTo(rc.left, rc.bottom-1);
                            drawDC.LineTo(rc.right-1, rc.bottom-1);
                            drawDC.LineTo(rc.right-1, rc.top);
                            drawDC.LineTo(rc.left, rc.top);
                        }

                        // Draw filled gradient background.
                        rc.InflateRect(-1, -1);
                        if ((state & (CDIS_SELECTED|CDIS_CHECKED)) || (pTB->GetButtonState(item) & TBSTATE_PRESSED))
                        {
                            drawDC.GradientFill(GetToolBarTheme().clrPressed1, GetToolBarTheme().clrPressed2, rc, FALSE);
                        }
                        else if (state & CDIS_HOT)
                        {
                            drawDC.GradientFill(GetToolBarTheme().clrHot1, GetToolBarTheme().clrHot2, rc, FALSE);
                        }

                        // Get the appropriate image list depending on the button state.
                        CImageList toolBarImages;
                        if (state & CDIS_DISABLED)
                        {
                            toolBarImages = pTB->GetDisabledImageList();
                            if (toolBarImages.GetHandle() == 0)
                                toolBarImages.CreateDisabledImageList(pTB->GetImageList());
                        }
                        else if (state & (CDIS_HOT | CDIS_SELECTED | CDIS_CHECKED))
                        {
                            toolBarImages = pTB->GetHotImageList();
                            if (toolBarImages.GetHandle() == 0)
                                toolBarImages = pTB->GetImageList();
                        }
                        else
                        {
                            toolBarImages = pTB->GetImageList();
                        }

                        BOOL isWin95 = (1400 == (GetWinVersion()) || (2400 == GetWinVersion()));

                        // Calculate image position.
                        CSize szImage = toolBarImages.GetIconSize();

                        int yImage = (rc.bottom + rc.top - szImage.cy - textSize.cy )/2;
                        int xImage = (rc.right + rc.left - szImage.cx)/2 + ((state & (CDIS_SELECTED|CDIS_CHECKED))? 1:0);
                        if (tbStyle & TBSTYLE_LIST)
                        {
                            xImage = rc.left + (IsXPThemed()?2:4) + ((state & CDIS_SELECTED)? 1:0);
                            yImage = (rc.bottom - rc.top - szImage.cy +2)/2 + ((state & (CDIS_SELECTED|CDIS_CHECKED))? 1:0);
                        }

                        // Handle the TBSTYLE_DROPDOWN and BTNS_WHOLEDROPDOWN styles.
                        if ((style & TBSTYLE_DROPDOWN) || ((style & 0x0080) && (!isWin95)))
                        {
                            // Calculate the dropdown arrow position
                            int xAPos = (style & TBSTYLE_DROPDOWN)? rc.right -6 : (rc.right + rc.left + szImage.cx + 4)/2;
                            int yAPos = (style & TBSTYLE_DROPDOWN)? (rc.bottom - rc.top +1)/2 : (szImage.cy)/2;
                            if (tbStyle & TBSTYLE_LIST)
                            {
                                xAPos = (style & TBSTYLE_DROPDOWN)? rc.right -6: rc.right -5;
                                yAPos = (rc.bottom - rc.top +1)/2 + ((style & TBSTYLE_DROPDOWN)?0:1);
                            }

                            xImage -= (style & TBSTYLE_DROPDOWN)?((tbStyle & TBSTYLE_LIST)? (IsXPThemed()?-4:0):6):((tbStyle & TBSTYLE_LIST)? 0:4);

                            // Draw separate background for dropdown arrow.
                            if ((m_drawArrowBkgrnd) && (state & CDIS_HOT))
                            {
                                CRect arrowRect = rc;
                                arrowRect.left = arrowRect.right - 13;
                                drawDC.GradientFill(GetToolBarTheme().clrPressed1, GetToolBarTheme().clrPressed2, arrowRect, FALSE);
                            }

                            m_drawArrowBkgrnd = FALSE;

                            // Manually draw the dropdown arrow
                            drawDC.CreatePen(PS_SOLID, 1, RGB(0,0,0));
                            for (int i = 2; i >= 0; --i)
                            {
                                drawDC.MoveTo(xAPos -i-1, yAPos - i+1);
                                drawDC.LineTo(xAPos +i,   yAPos - i+1);
                            }

                            // Draw line between icon and dropdown arrow
                            if ((style & TBSTYLE_DROPDOWN) && ((state & CDIS_SELECTED) || state & CDIS_HOT))
                            {
                                drawDC.CreatePen(PS_SOLID, 1, GetToolBarTheme().clrOutline);
                                drawDC.MoveTo(rc.right - 13, rc.top);
                                drawDC.LineTo(rc.right - 13, rc.bottom);
                            }
                        }

                        // Draw the button image.
                        if (xImage > 0)
                        {
                            toolBarImages.Draw(drawDC, image, CPoint(xImage, yImage), ILD_TRANSPARENT);
                        }

                        //Draw Text.
                        if (!str.IsEmpty())
                        {
                            int width = rc.right - rc.left - ((state & TBSTYLE_DROPDOWN)?13:0);
                            CRect textRect(0, 0, MIN(textSize.cx, width), textSize.cy);

                            int xOffset = (rc.right + rc.left - textRect.right + textRect.left - ((state & TBSTYLE_DROPDOWN)? 11 : 1))/2;
                            int yOffset = yImage + szImage.cy +1;

                            if (tbStyle & TBSTYLE_LIST)
                            {
                                xOffset = rc.left + szImage.cx + ((state & TBSTYLE_DROPDOWN)?(IsXPThemed()?10:6): 6) + ((state & CDIS_SELECTED)? 1:0);
                                yOffset = (2+ rc.bottom - rc.top - textRect.bottom + textRect.top)/2 + ((state & CDIS_SELECTED)? 1:0);
                                textRect.right = MIN(textRect.right, rc.right - xOffset);
                            }

                            OffsetRect(&textRect, xOffset, yOffset);

                            int mode = drawDC.SetBkMode(TRANSPARENT);
                            drawDC.SelectObject(pTB->GetFont());

                            if (state & (CDIS_DISABLED))
                            {
                                // Draw text twice for embossed look
                                textRect.OffsetRect(1, 1);
                                drawDC.SetTextColor(RGB(255,255,255));
                                drawDC.DrawText(str, str.GetLength(), textRect, DT_LEFT);
                                textRect.OffsetRect(-1, -1);
                                drawDC.SetTextColor(GetSysColor(COLOR_GRAYTEXT));
                                drawDC.DrawText(str, str.GetLength(), textRect, DT_LEFT);
                            }
                            else
                            {
                                drawDC.SetTextColor(GetSysColor(COLOR_BTNTEXT));
                                drawDC.DrawText(str, str.GetLength(), textRect, DT_LEFT | DT_END_ELLIPSIS);
                            }
                            drawDC.SetBkMode(mode);

                        }
                    }
                    return CDRF_SKIPDEFAULT;  // No further drawing
                }
            }
        }
        return 0;
    }

    // Called by OnDrawItem to render the popup menu items.
    template <class T>
    inline void CFrameT<T>::DrawMenuItem(LPDRAWITEMSTRUCT pDIS)
    {
        MenuItemData* pmid = reinterpret_cast<MenuItemData*>(pDIS->itemData);
        int stateID = GetMenuMetrics().ToItemStateId(pDIS->itemState);
        const MenuTheme& mbt = GetMenuBarTheme();
        CDC drawDC(pDIS->hDC);

        if (IsUsingVistaMenu() && GetMenuMetrics().IsThemeBackgroundPartiallyTransparent(MENU_POPUPITEM, stateID))
        {
            GetMenuMetrics().DrawThemeBackground(pDIS->hDC, MENU_POPUPBACKGROUND, 0, &pDIS->rcItem, NULL);
        }

        // Draw the gutter.
        CRect gutter = GetMenuMetrics().GetGutterRect(pDIS->rcItem);
        if (IsUsingVistaMenu())
            GetMenuMetrics().DrawThemeBackground(pDIS->hDC, MENU_POPUPGUTTER, 0, &gutter, NULL);
        else
            drawDC.GradientFill(mbt.clrPressed1, mbt.clrPressed2, gutter, TRUE);

        if (pmid->mii.fType & MFT_SEPARATOR)
        {
            // Draw the separator.
            if (IsUsingVistaMenu())
            {
                CRect sepRect = GetMenuMetrics().GetSeperatorRect(pDIS->rcItem);
                GetMenuMetrics().DrawThemeBackground(pDIS->hDC, MENU_POPUPSEPARATOR, 0, &sepRect, NULL);
            }
            else
            {
                CRect rc = pDIS->rcItem;
                CRect sepRect = pDIS->rcItem;
                sepRect.left = GetMenuMetrics().GetGutterRect(rc).Width();
                drawDC.SolidFill(RGB(255,255,255), sepRect);
                sepRect.top += (rc.bottom - rc.top)/2;
                drawDC.DrawEdge(sepRect,  EDGE_ETCHED, BF_TOP);
            }
        }
        else
        {
            // Draw the item background.
            DrawMenuItemBkgnd(pDIS);

            // Draw Checkmark or icon.
            if (pDIS->itemState & ODS_CHECKED)
                DrawMenuItemCheckmark(pDIS);
            else
                DrawMenuItemIcon(pDIS);

            // Draw the text.
            DrawMenuItemText(pDIS);
        }

        if (IsUsingVistaMenu())
        {
            // Draw the Submenu arrow.
            // We extract the bitmap from DrawFrameControl to support Windows 10
            if (pmid->mii.hSubMenu)
            {
                CRect subMenuRect = pDIS->rcItem;
                subMenuRect.left = pDIS->rcItem.right - GetMenuMetrics().m_marItem.cxRightWidth - GetMenuMetrics().m_marText.cxRightWidth;

                // Copy the menu's arrow to a memory DC
                CMemDC memDC(drawDC);
                memDC.CreateBitmap(subMenuRect.Width(), subMenuRect.Height(), 1, 1, NULL);
                CRect rc(0, 0, subMenuRect.Width(), subMenuRect.Height());
                memDC.DrawFrameControl(rc, DFC_MENU, DFCS_MENUARROW);

                // Mask the arrow image back to the menu's dc
                CMemDC maskDC(drawDC);
                maskDC.CreateCompatibleBitmap(pDIS->hDC, subMenuRect.Width(), subMenuRect.Height());
                maskDC.BitBlt(0, 0, subMenuRect.Width(), subMenuRect.Height(), maskDC, 0, 0, WHITENESS);
                maskDC.BitBlt(0, 0, subMenuRect.Width(), subMenuRect.Height(), memDC, 0, 0, SRCAND);
                drawDC.BitBlt(subMenuRect.left, subMenuRect.top, subMenuRect.Width(), subMenuRect.Height(), maskDC, 0, 0, SRCAND);
            }

            // Suppress further drawing to prevent an incorrect Submenu arrow being drawn.
            CRect rc = pDIS->rcItem;
            drawDC.ExcludeClipRect(rc);
        }
    }

    // Called by DrawMenuItem to render the popup menu background.
    template <class T>
    inline void CFrameT<T>::DrawMenuItemBkgnd(LPDRAWITEMSTRUCT pDIS)
    {
        // Draw the item background
        CRect selRect = GetMenuMetrics().GetSelectionRect(pDIS->rcItem);
        if (IsUsingVistaMenu())
        {
            int stateID = GetMenuMetrics().ToItemStateId(pDIS->itemState);
            GetMenuMetrics().DrawThemeBackground(pDIS->hDC, MENU_POPUPITEM, stateID, &selRect, NULL);
        }
        else
        {
            BOOL isDisabled = pDIS->itemState & ODS_GRAYED;
            BOOL isSelected = pDIS->itemState & ODS_SELECTED;
            CRect drawRect = pDIS->rcItem;
            CDC drawDC(pDIS->hDC);
            const MenuTheme& mbt = GetMenuBarTheme();

            if ((isSelected) && (!isDisabled))
            {
                // draw selected item background
                CBrush brush(mbt.clrHot1);
                CBrush oldBrush = drawDC.SelectObject(brush);
                CPen pen(PS_SOLID, 1, mbt.clrOutline);
                CPen oldPen = drawDC.SelectObject(pen);
                drawDC.Rectangle(drawRect.left, drawRect.top, drawRect.right, drawRect.bottom);
                drawDC.SelectObject(oldBrush);
                drawDC.SelectObject(oldPen);
            }
            else
            {
                // draw non-selected item background
                drawRect.left = GetMenuMetrics().GetGutterRect(pDIS->rcItem).Width();
                drawDC.SolidFill(RGB(255,255,255), drawRect);
            }
        }
    }

    // Draws the checkmark or radiocheck transparently.
    template <class T>
    inline void CFrameT<T>::DrawMenuItemCheckmark(LPDRAWITEMSTRUCT pDIS)
    {
        CRect rc = pDIS->rcItem;
        MenuItemData* pmid = reinterpret_cast<MenuItemData*>(pDIS->itemData);
        UINT fType = pmid->mii.fType;
        const MenuTheme& mbt = GetMenuBarTheme();
        CRect bkRect;
        CDC drawDC(pDIS->hDC);

        if (IsUsingVistaMenu())
        {
            int stateID = GetMenuMetrics().ToItemStateId(pDIS->itemState);
            CRect rcCheckBackground = GetMenuMetrics().GetCheckBackgroundRect(pDIS->rcItem);
            GetMenuMetrics().DrawThemeBackground(pDIS->hDC, MENU_POPUPCHECKBACKGROUND, GetMenuMetrics().ToCheckBackgroundStateId(stateID), &rcCheckBackground, NULL);

            CRect rcCheck = GetMenuMetrics().GetCheckRect(pDIS->rcItem);
            GetMenuMetrics().DrawThemeBackground(pDIS->hDC, MENU_POPUPCHECK, GetMenuMetrics().ToCheckStateId(pmid->mii.fType, stateID), &rcCheck, NULL);
        }
        else
        {
            // Draw the checkmark's background rectangle first.
            int xIcon = GetMenuMetrics().m_sizeCheck.cx;
            int yIcon = GetMenuMetrics().m_sizeCheck.cy;
            int left = GetMenuMetrics().m_marCheck.cxLeftWidth;
            int top = rc.top + (rc.Height() - yIcon) / 2;
            bkRect.SetRect(left, top, left + xIcon, top + yIcon);

            CBrush brush(mbt.clrHot2);
            CBrush oldBrush = drawDC.SelectObject(brush);
            CPen pen(PS_SOLID, 1, mbt.clrOutline);
            CPen oldPen = drawDC.SelectObject(pen);

            // Draw the checkmark's background rectangle.
            drawDC.Rectangle(bkRect.left, bkRect.top, bkRect.right, bkRect.bottom);

            CMemDC memDC(drawDC);
            int cxCheck = ::GetSystemMetrics(SM_CXMENUCHECK);
            int cyCheck = ::GetSystemMetrics(SM_CYMENUCHECK);
            memDC.CreateBitmap(cxCheck, cyCheck, 1, 1, NULL);
            CRect checkRect(0, 0, cxCheck, cyCheck);

            // Copy the check mark bitmap to hdcMem.
            if (MFT_RADIOCHECK == fType)
                memDC.DrawFrameControl(checkRect, DFC_MENU, DFCS_MENUBULLET);
            else
                memDC.DrawFrameControl(checkRect, DFC_MENU, DFCS_MENUCHECK);

            int xoffset = (bkRect.Width() - checkRect.Width()  +1) / 2;
            int yoffset = (bkRect.Height() - checkRect.Height() +1) / 2;
            if (MFT_RADIOCHECK != fType) yoffset--;

            // Draw a white or black check mark as required.
            // Unfortunately MaskBlt isn't supported on Win95, 98 or ME, so we do it the hard way.
            CMemDC maskDC(drawDC);
            maskDC.CreateCompatibleBitmap(drawDC, cxCheck, cyCheck);
            maskDC.BitBlt(0, 0, cxCheck, cyCheck, maskDC, 0, 0, WHITENESS);

            if ((pDIS->itemState & ODS_SELECTED))
            {
                // Draw a white checkmark
                memDC.BitBlt(0, 0, cxCheck, cyCheck, memDC, 0, 0, DSTINVERT);
                maskDC.BitBlt(0, 0, cxCheck, cyCheck, memDC, 0, 0, SRCAND);
                drawDC.BitBlt(bkRect.left + xoffset, bkRect.top + yoffset, cxCheck, cyCheck, maskDC, 0, 0, SRCPAINT);
            }
            else
            {
                // Draw a black checkmark
                maskDC.BitBlt(0, 0, cxCheck, cyCheck, memDC, 0, 0, SRCAND);
                drawDC.BitBlt(bkRect.left + xoffset, bkRect.top + yoffset, cxCheck, cyCheck, maskDC, 0, 0, SRCAND);
            }

            drawDC.SelectObject(oldBrush);
            drawDC.SelectObject(oldPen);
        }
    }

    // Called by DrawMenuItem to draw icons in popup menus.
    template <class T>
    inline void CFrameT<T>::DrawMenuItemIcon(LPDRAWITEMSTRUCT pDIS)
    {
        if ( 0 == m_menuImages.GetHandle() )
            return;

        // Get icon size
        CSize iconSize = m_menuImages.GetIconSize();
        int xIcon = iconSize.cx;
        int yIcon = iconSize.cy;

        // get the drawing rectangle
        CRect rc = pDIS->rcItem;
        int left = GetMenuMetrics().m_marCheck.cxLeftWidth;
        int top = rc.top + (rc.Height() - yIcon)/2;
        rc.SetRect(left, top, left + xIcon, top + yIcon);

        // get the icon's location in the imagelist
        int image = -1;
        for (int i = 0 ; i < static_cast<int>(m_menuIcons.size()); ++i)
        {
            if (pDIS->itemID == m_menuIcons[i])
                image = i;
        }

        // draw the image
        if (image >= 0 )
        {
            BOOL isDisabled = pDIS->itemState & ODS_GRAYED;
            if ((isDisabled) && (m_menuDisabledImages.GetHandle()))
                m_menuDisabledImages.Draw(pDIS->hDC, image, CPoint(rc.left, rc.top), ILD_TRANSPARENT);
            else
                m_menuImages.Draw(pDIS->hDC, image, CPoint(rc.left, rc.top), ILD_TRANSPARENT);
        }
    }

    // Called by DrawMenuItem to render the text for popup menus.
    template <class T>
    inline void CFrameT<T>::DrawMenuItemText(LPDRAWITEMSTRUCT pDIS)
    {
        MenuItemData* pmid = reinterpret_cast<MenuItemData*>(pDIS->itemData);

        LPCTSTR pItem = pmid->GetItemText();
        BOOL isDisabled = pDIS->itemState & ODS_GRAYED;
        COLORREF colorText = GetSysColor(isDisabled ?  COLOR_GRAYTEXT : COLOR_MENUTEXT);

        // Calculate the text rect size
        CRect textRect = GetMenuMetrics().GetTextRect(pDIS->rcItem);

        // find the position of tab character
        int tab = -1;
        int len = lstrlen(pItem);
        for (int i = 0; i < len; ++i)
        {
            if (_T('\t') == pItem[i])
            {
                tab = i;
                break;
            }
        }

        // Draw the item text.
        if (IsUsingVistaMenu())
        {
            ULONG accel = ((pDIS->itemState & ODS_NOACCEL) ? DT_HIDEPREFIX : 0);
            int stateID = GetMenuMetrics().ToItemStateId(pDIS->itemState);

            // Draw the item text before the tab
            GetMenuMetrics().DrawThemeText(pDIS->hDC, MENU_POPUPITEM, stateID, TtoW(pItem), tab, DT_SINGLELINE | DT_LEFT | DT_VCENTER | accel, 0, &textRect);

            // Draw text after tab, right aligned
            if (tab != -1)
                GetMenuMetrics().DrawThemeText(pDIS->hDC, MENU_POPUPITEM, stateID, TtoW(&pItem[tab + 1]), -1, DT_SINGLELINE | DT_RIGHT | DT_VCENTER | accel, 0, &textRect);
        }
        else
        {
            SetTextColor(pDIS->hDC, colorText);
            int mode = SetBkMode(pDIS->hDC, TRANSPARENT);
            DrawText(pDIS->hDC, pItem, tab, textRect, DT_SINGLELINE | DT_LEFT | DT_VCENTER);

            // Draw text after tab, right aligned
            if (tab != -1)
                DrawText(pDIS->hDC, &pItem[tab + 1], -1, textRect, DT_SINGLELINE | DT_RIGHT | DT_VCENTER);

            SetBkMode(pDIS->hDC, mode);
        }
    }

    // Draws the ReBar's background when ReBar themes are enabled.
    // Returns TRUE when the default background drawing is suppressed.
    template <class T>
    inline BOOL CFrameT<T>::DrawReBarBkgnd(CDC& dc, CReBar& rebar)
    {
        BOOL isDrawn = TRUE;

        const ReBarTheme& rt = GetReBarTheme();
        if (!rt.UseThemes)
            isDrawn = FALSE;

        if (!rt.clrBkgnd1 && !rt.clrBkgnd2 && !rt.clrBand1 && !rt.clrBand2)
            isDrawn = FALSE;

        if (isDrawn)
        {
            assert(rebar.IsWindow());

            BOOL isVertical = rebar.GetStyle() & CCS_VERT;

            // Create our memory DC
            CRect rebarRect = rebar.GetClientRect();
            int rebarWidth = rebarRect.Width();
            int rebarHeight = rebarRect.Height();
            CMemDC memDC(dc);
            memDC.CreateCompatibleBitmap(dc, rebarWidth, rebarRect.Height());

            // Draw to ReBar background to the memory DC
            memDC.SolidFill(rt.clrBkgnd2, rebarRect);
            CRect rcBkGnd = rebarRect;
            rcBkGnd.right = 600;
            memDC.GradientFill(rt.clrBkgnd1, rt.clrBkgnd2, rebarRect, TRUE);

            if (rt.clrBand1 || rt.clrBand2)
            {
                // Draw the individual band backgrounds
                for (int band = 0 ; band < rebar.GetBandCount(); ++band)
                {
                    if (rebar.IsBandVisible(band))
                    {
                        if (band != rebar.GetBand(GetMenuBar()))
                        {
                            // Determine the size of this band
                            CRect bandRect = rebar.GetBandRect(band);

                            if (isVertical)
                            {
                                int right = bandRect.right;
                                bandRect.right = bandRect.bottom;
                                bandRect.bottom = right;
                            }

                            // Determine the size of the child window
                            REBARBANDINFO rbbi;
                            ZeroMemory(&rbbi, sizeof(rbbi));
                            rbbi.fMask = RBBIM_CHILD ;
                            rebar.GetBandInfo(band, rbbi);
                            CRect childRect;
                            ::GetWindowRect(rbbi.hwndChild, &childRect);
                            T::ScreenToClient(childRect);

                            // Determine our drawing rectangle
                            int startPad = IsXPThemed()? 2: 0;
                            CRect drawRect = bandRect;
                            CRect borderRect = rebar.GetBandBorders(band);
                            if (isVertical)
                            {
                                drawRect.bottom = drawRect.top + childRect.Height() + borderRect.top;
                                drawRect.top -= startPad;
                            }
                            else
                            {
                                drawRect.right = drawRect.left + childRect.Width() + borderRect.left;
                                drawRect.left -= startPad;
                            }

                            if (!rt.FlatStyle)
                                ::InflateRect(&drawRect, 1, 1);

                            // Fill the Source CDC with the band's background
                            CMemDC sourceDC(dc);
                            sourceDC.CreateCompatibleBitmap(dc, rebarWidth, rebarHeight);
                            sourceDC.GradientFill(rt.clrBand1, rt.clrBand2, drawRect, isVertical);

                            // Set Curve amount for rounded edges
                            int curve = rt.RoundBorders? 12 : 0;

                            // Create our mask for rounded edges using RoundRect
                            CMemDC maskDC(dc);
                            maskDC.CreateCompatibleBitmap(dc, rebarWidth, rebarHeight);

                            int left = drawRect.left;
                            int right = drawRect.right;
                            int top = drawRect.top;
                            int bottom = drawRect.bottom;
                            int cx = drawRect.Width();// + xPad;
                            int cy = drawRect.Height();

                            if (rt.FlatStyle)
                            {
                                maskDC.SolidFill(RGB(0,0,0), drawRect);
                                maskDC.BitBlt(left, top, cx, cy, maskDC, left, top, PATINVERT);
                                maskDC.RoundRect(left, top, right, bottom, curve, curve);
                            }
                            else
                            {
                                maskDC.SolidFill(RGB(0,0,0), drawRect);
                                maskDC.RoundRect(left, top, right, bottom, curve, curve);
                                maskDC.BitBlt(left, top, cx, cy, maskDC, left, top, PATINVERT);
                            }

                            // Copy Source DC to Memory DC using the RoundRect mask
                            memDC.BitBlt(left, top, cx, cy, sourceDC, left, top, SRCINVERT);
                            memDC.BitBlt(left, top, cx, cy, maskDC,   left, top, SRCAND);
                            memDC.BitBlt(left, top, cx, cy, sourceDC, left, top, SRCINVERT);
                        }
                    }
                }
            }

            if (rt.UseLines)
            {
                // Draw lines between bands
                for (int j = 0; j < rebar.GetBandCount()-1; ++j)
                {
                    CRect bandRect = rebar.GetBandRect(j);
                    if (isVertical)
                    {
                        bandRect.top = MAX(0, rebarRect.top - 4);
                        bandRect.right +=2;
                    }
                    else
                    {
                        bandRect.left = MAX(0, rebarRect.left - 4);
                        bandRect.bottom +=2;
                    }
                    memDC.DrawEdge(bandRect, EDGE_ETCHED, BF_BOTTOM | BF_ADJUST);
                }
            }

            // Copy the Memory DC to the window's DC
            dc.BitBlt(0, 0, rebarWidth, rebarHeight, memDC, 0, 0, SRCCOPY);
        }

        return isDrawn;
    }

    // Draws the StatusBar's background when StatusBar themes are enabled.
    // Returns TRUE when the default background drawing is suppressed.
    template <class T>
    inline BOOL CFrameT<T>::DrawStatusBarBkgnd(CDC& dc, CStatusBar& statusBar)
    {
        BOOL isDrawn = FALSE;

        // XP Themes are required to modify the statusbar's background
        if (IsXPThemed())
        {
            const StatusBarTheme& sbTheme = GetStatusBarTheme();
            if (sbTheme.UseThemes)
            {
                // Fill the background with a color gradient
                dc.GradientFill(sbTheme.clrBkgnd1, sbTheme.clrBkgnd2, statusBar.GetClientRect(), TRUE);
                isDrawn = TRUE;
            }
        }

        return isDrawn;
    }

    // Calculates the remaining client rect when a child window is excluded.
    // Note: Assumes the child window touches 3 of the client rect's borders.
    //  e.g.   CRect rc = ExcludeChildRect(GetClientRect(), GetStatusBar());
    template <class T>
    inline CRect CFrameT<T>::ExcludeChildRect(const CRect& clientRect, HWND child) const
    {
        CRect clientRC = clientRect;
        T::ClientToScreen(clientRC);

        CRect childRect;
        ::GetWindowRect(child, &childRect);

        if (clientRC.Width() == childRect.Width())
        {
            if (clientRC.top == childRect.top)
                clientRC.top += childRect.Height();
            else
                clientRC.bottom -= childRect.Height();
        }
        else
        {
            if (clientRC.left == childRect.left)
                clientRC.left += childRect.Width();
            else
                clientRC.right -= childRect.Width();
        }

        T::ScreenToClient(clientRC);

        return clientRC;
    }

    // Returns the position of the menu item, given it's name.
    template <class T>
    inline int CFrameT<T>::GetMenuItemPos(HMENU menu, LPCTSTR pItem) const
    {
        int menuItemCount = ::GetMenuItemCount(menu);
        MENUITEMINFO mii;
        ZeroMemory(&mii, sizeof(mii));
        mii.cbSize = GetSizeofMenuItemInfo();

        for (int item = 0 ; item < menuItemCount; ++item)
        {
            std::vector<TCHAR> menuString( MAX_MENU_STRING+1, _T('\0') );
            TCHAR* pMenuString = &menuString[0];

            std::vector<TCHAR> strippedString( MAX_MENU_STRING+1, _T('\0') );
            TCHAR* pStrippedString = &strippedString[0];

            mii.fMask      = MIIM_TYPE;
            mii.fType      = MFT_STRING;
            mii.dwTypeData = pMenuString;
            mii.cch        = MAX_MENU_STRING;

            // Fill the contents of szStr from the menu item
            if (::GetMenuItemInfo(menu, item, TRUE, &mii))
            {
                int len = lstrlen(pMenuString);
                if (len <= MAX_MENU_STRING)
                {
                    // Strip out any & characters
                    int j = 0;
                    for (int i = 0; i < len; ++i)
                    {
                        if (pMenuString[i] != _T('&'))
                            pStrippedString[j++] = pMenuString[i];
                    }
                    pStrippedString[j] = _T('\0');   // Append null tchar

                    // Compare the strings
                    if (lstrcmp(pStrippedString, pItem) == 0)
                        return item;
                }
            }
        }

        return -1;
    }

    // Returns a MRU entry given its index.
    template <class T>
    inline CString CFrameT<T>::GetMRUEntry(UINT index)
    {
        CString pathName;
        if (index < m_mruEntries.size())
        {
            pathName = m_mruEntries[index];

            // Now put the selected entry at index 0
            AddMRUEntry(pathName);
        }
        return pathName;
    }

    // Returns the size of a bitmap image.
    template <class T>
    inline CSize CFrameT<T>::GetTBImageSize(CBitmap* pBitmap)
    {
        assert(pBitmap);
        if (!pBitmap) return CSize(0, 0);

        assert(pBitmap->GetHandle());
        BITMAP data = pBitmap->GetBitmapData();
        int cy = data.bmHeight;
        int cx = MAX(data.bmHeight, 16);

        return CSize(cx, cy);
    }

    // Returns the dimensions of the view window.
    template <class T>
    inline CRect CFrameT<T>::GetViewRect() const
    {
        CRect clientRect = T::GetClientRect();

        if (GetStatusBar().IsWindow() && GetStatusBar().IsWindowVisible())
            clientRect = ExcludeChildRect(clientRect, GetStatusBar());

        if (GetReBar().IsWindow() && GetReBar().IsWindowVisible())
            clientRect = ExcludeChildRect(clientRect, GetReBar());
        else
            if (GetToolBar().IsWindow() && GetToolBar().IsWindowVisible())
                clientRect = ExcludeChildRect(clientRect, GetToolBar());

        return clientRect;
    }

    // Returns the XP theme name.
    template <class T>
    inline CString CFrameT<T>::GetThemeName() const
    {
        HMODULE theme = ::LoadLibrary(_T("uxtheme.dll"));
        WCHAR themeName[31] = L"";
        if (theme != 0)
        {
            typedef HRESULT (__stdcall *PFNGETCURRENTTHEMENAME)(LPWSTR pThemeFileName, int maxNameChars,
                LPWSTR pColorBuff, int maxColorChars, LPWSTR pSizeBuff, int maxSizeChars);

            PFNGETCURRENTTHEMENAME pfn = (PFNGETCURRENTTHEMENAME)GetProcAddress(theme, "GetCurrentThemeName");
            pfn(0, 0, themeName, 30, 0, 0);

            ::FreeLibrary(theme);
        }

        return CString(themeName);
    }

    // Load the MRU list from the registry.
    template <class T>
    inline BOOL CFrameT<T>::LoadRegistryMRUSettings(UINT maxMRU /*= 0*/)
    {
        assert(!m_keyName.IsEmpty()); // KeyName must be set before calling LoadRegistryMRUSettings

        CRegKey key;
        BOOL loaded = FALSE;
        SetMRULimit(maxMRU);
        std::vector<CString> mruEntries;
        CString strKey = _T("Software\\") + m_keyName + _T("\\Recent Files");

        if (ERROR_SUCCESS == key.Open(HKEY_CURRENT_USER, strKey, KEY_READ))
        {
            CString pathName;
            CString subKeyName;
            for (UINT i = 0; i < m_maxMRU; ++i)
            {
                DWORD bufferSize = 0;
                subKeyName.Format(_T("File %d"), i+1);

                if (ERROR_SUCCESS == key.QueryStringValue(subKeyName, NULL, &bufferSize))
                {
                    // load the entry from the registry.
                    if (ERROR_SUCCESS == key.QueryStringValue(subKeyName, pathName.GetBuffer(bufferSize), &bufferSize))
                    {
                        pathName.ReleaseBuffer();

                        if (pathName.GetLength() > 0)
                            mruEntries.push_back( pathName );
                    }
                    else
                    {
                        pathName.ReleaseBuffer();
                        TRACE(_T("LoadRegistryMRUSettings: RegQueryValueEx failed\n"));
                    }
                }
            }

            // successfully loaded all MRU values, so store them.
            m_mruEntries = mruEntries;
            loaded = TRUE;
        }

        return loaded;
    }

    // Loads various frame settings from the registry.
    template <class T>
    inline BOOL CFrameT<T>::LoadRegistrySettings(LPCTSTR pKeyName)
    {
        assert (NULL != pKeyName);

        m_keyName = pKeyName;
        CString strKey = _T("Software\\") + m_keyName + _T("\\Frame Settings");
        BOOL isOK = FALSE;
        InitValues values;
        CRegKey key;
        if (ERROR_SUCCESS == key.Open(HKEY_CURRENT_USER, strKey, KEY_READ))
        {
            try
            {
                DWORD top, left, width, height, showCmd, statusBar, toolBar;

                if (ERROR_SUCCESS != key.QueryDWORDValue(_T("Top"), top))
                    throw CUserException(_T("RegQueryValueEx Failed"));
                if (ERROR_SUCCESS != key.QueryDWORDValue(_T("Left"), left))
                    throw CUserException(_T("RegQueryValueEx Failed"));
                if (ERROR_SUCCESS != key.QueryDWORDValue(_T("Width"), width))
                    throw CUserException(_T("RegQueryValueEx Failed"));
                if (ERROR_SUCCESS != key.QueryDWORDValue(_T("Height"), height))
                    throw CUserException(_T("RegQueryValueEx Failed"));
                if (ERROR_SUCCESS != key.QueryDWORDValue(_T("ShowCmd"), showCmd))
                    throw CUserException(_T("RegQueryValueEx Failed"));
                if (ERROR_SUCCESS != key.QueryDWORDValue(_T("StatusBar"), statusBar))
                    throw CUserException(_T("RegQueryValueEx Failed"));
                if (ERROR_SUCCESS != key.QueryDWORDValue(_T("ToolBar"), toolBar))
                    throw CUserException(_T("RegQueryValueEx Failed"));

                values.position = CRect(left, top, left + width, top + height);
                values.showCmd = (SW_MAXIMIZE == showCmd) ? SW_MAXIMIZE : SW_SHOW;
                values.showStatusBar = statusBar & 1;
                values.showToolBar = toolBar & 1;

                isOK = TRUE;
            }

            catch (const CUserException& e)
            {
                Trace("*** Failed to load values from registry, using defaults. ***\n");
                Trace(e.GetText()); Trace(strKey); Trace("\n");

                // Delete the bad key from the registry
                CString strParentKey = _T("Software\\") + m_keyName;
                CRegKey parentKey;
                if (ERROR_SUCCESS == parentKey.Open(HKEY_CURRENT_USER, strParentKey, KEY_READ))
                    parentKey.DeleteSubKey(_T("Frame Settings"));

                InitValues defaultValues;
                values = defaultValues;
            }
        }

        SetInitValues(values);
        return isOK;
    }

    // Determines the size of the popup menus.
    template <class T>
    inline void CFrameT<T>::MeasureMenuItem(MEASUREITEMSTRUCT *pMIS)
    {
        // Initialize the menu metrics on first use.
        if (m_menuMetrics.m_frame == 0)
        {
            m_menuMetrics.Initialize(*this);
        }

        MenuItemData* pMID = reinterpret_cast<MenuItemData*>(pMIS->itemData);
        assert(::IsMenu(pMID->menu));  // Does itemData contain a valid MenuItemData struct?

        CSize size = GetMenuMetrics().GetItemSize(pMID);

        // Return the composite sizes.
        pMIS->itemWidth = size.cx;
        pMIS->itemHeight = size.cy;
    }

    // Called when the frame is activated (WM_ACTIVATE received)
    template <class T>
    inline LRESULT CFrameT<T>::OnActivate(UINT, WPARAM wparam, LPARAM lparam)
    {
        // Perform default processing first
        CWnd::WndProcDefault(WM_ACTIVATE, wparam, lparam);


        if (LOWORD(wparam) == WA_INACTIVE)
        {
            // Save the hwnd of the window which currently has focus.
            // This must be CFrame window itself or a child window.
            if (!T::IsIconic()) m_oldFocus = ::GetFocus();
        }
        else
        {
            // Now set the focus to the appropriate child window.
            if (m_oldFocus != 0) ::SetFocus(m_oldFocus);
        }

        // Update DockClient captions
        T::PostMessage(UWM_DOCKACTIVATE);


        // Also update DockClient captions if the view is a docker
        if (GetView().IsWindow() && GetView().SendMessage(UWM_GETCDOCKER))
            GetView().PostMessage(UWM_DOCKACTIVATE);

        return 0;
    }

    // Called in response to a WM_CLOSE message for the frame.
    template <class T>
    inline void CFrameT<T>::OnClose()
    {
        SaveRegistrySettings();
        T::ShowWindow(SW_HIDE);
        T::Destroy();
    }

    // This is called when the frame window is being created.
    // Override this in CMainFrame if you wish to modify what happens here.
    template <class T>
    inline int CFrameT<T>::OnCreate(CREATESTRUCT& cs)
    {
        UNREFERENCED_PARAMETER(cs);

        // Start the keyboard hook to capture the CapsLock, NumLock,
        // ScrollLock and Insert keys.
        SetKbdHook();

        // Set the icon.
        // Note: The MinGW compiler requires the T:: scope specifier here.
        //       It would not be required in a class that inherits from CFrameT.
        T::SetIconLarge(IDW_MAIN);
        T::SetIconSmall(IDW_MAIN);

        // Set the keyboard accelerators
        SetAccelerators(IDW_MAIN);

        // Set the Caption
        T::SetWindowText(LoadString(IDW_MAIN));

        // Set the theme for the frame elements
        SetTheme();

        // Create the rebar and menubar
        if (IsReBarSupported() && IsUsingReBar())
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
        CMenu menu(IDW_MAIN);
        SetFrameMenu(menu); // 0 if IDW_MAIN menu resource is missing
        if (::IsMenu(menu))
        {
            if (m_maxMRU > 0)
                UpdateMRUMenu();
        }

        // Create the ToolBar
        if (IsUsingToolBar())
        {
            CreateToolBar();
            ShowToolBar(GetInitValues().showToolBar);
        }
        else
        {
            if (IsMenu(GetFrameMenu()))
                GetFrameMenu().EnableMenuItem(IDW_VIEW_TOOLBAR, MF_GRAYED);
        }

        SetupMenuIcons();

        // Create the status bar
        if (IsUsingStatusBar())
        {
            GetStatusBar().Create(*this);
            GetStatusBar().SetFont(m_statusBarFont, FALSE);
            ShowStatusBar(GetInitValues().showStatusBar);
        }
        else
        {
            if (IsMenu(GetFrameMenu()))
                GetFrameMenu().EnableMenuItem(IDW_VIEW_STATUSBAR, MF_GRAYED);
        }

        if (IsUsingIndicatorStatus())
            SetStatusIndicators();

        // Create the view window
        assert(&GetView());         // Use SetView in CMainFrame's constructor to set the view window

        if (!GetView().IsWindow())
            GetView().Create(*this);
        GetView().SetFocus();

        // Adjust fonts to match the desktop theme
        T::SendMessage(WM_SYSCOLORCHANGE);

        // Reposition the child windows
        RecalcLayout();

        return 0;
    }

    // Handles CustomDraw notification from WM_NOTIFY.
    template <class T>
    inline LRESULT CFrameT<T>::OnCustomDraw(LPNMHDR pNMHDR)
    {
        if ( ::SendMessage(pNMHDR->hwndFrom, UWM_GETCTOOLBAR, 0, 0) )
        {
            if (pNMHDR->hwndFrom == GetMenuBar())
                return CustomDrawMenuBar(pNMHDR);
            else
                return CustomDrawToolBar(pNMHDR);
        }

        return 0;
    }

    // Called when the frame is about to be destroyed (WM_DESTROY received).
    // Calls PostQuitMessage to end the application.
    template <class T>
    inline void CFrameT<T>::OnDestroy()
    {
        GetMenuBar().Destroy();
        GetToolBar().Destroy();
        GetReBar().Destroy();
        GetStatusBar().Destroy();
        GetView().Destroy();

        ::PostQuitMessage(0);   // Terminates the application
    }

    // OwnerDraw is used to render the popup menu items.
    template <class T>
    inline LRESULT CFrameT<T>::OnDrawItem(UINT, WPARAM wparam, LPARAM lparam)
    {
        LPDRAWITEMSTRUCT pdis = (LPDRAWITEMSTRUCT)lparam;
        assert(pdis);

        if (pdis && IsMenu(reinterpret_cast<HMENU>(pdis->hwndItem)) && (!IsRectEmpty(&pdis->rcItem)))
        {
            DrawMenuItem(pdis);
            return TRUE;
        }

        return CWnd::WndProcDefault(WM_DRAWITEM, wparam, lparam);
    }

    // Called when the Rebar's background is redrawn.
    template<class T>
    inline LRESULT CFrameT<T>::OnDrawRBBkgnd(UINT, WPARAM wparam, LPARAM lparam)
    {
        CDC* pDC = reinterpret_cast<CDC*>(wparam);
        assert(dynamic_cast<CDC*>(pDC));

        CReBar* pRebar = reinterpret_cast<CReBar*>(lparam);
        assert(dynamic_cast<CReBar*>(pRebar));

        if (!pDC || !pRebar)
            return 0;

        return DrawReBarBkgnd(*pDC, *pRebar);
    }

    // Called when the StatusBar's background is redrawn.
    template<class T>
    inline LRESULT CFrameT<T>::OnDrawSBBkgnd(UINT, WPARAM wparam, LPARAM lparam)
    {
        CDC* pDC = reinterpret_cast<CDC*>(wparam);
        assert(dynamic_cast<CDC*>(pDC));

        CStatusBar* pStatusBar = reinterpret_cast<CStatusBar*>(lparam);
        assert(dynamic_cast<CStatusBar*>(pStatusBar));

        if (!pDC || !pStatusBar)
            return 0;

        return DrawStatusBarBkgnd(*pDC, *pStatusBar);
    }

    // Called to display help (WM_HELP received or selected via menu).
    template <class T>
    inline BOOL CFrameT<T>::OnHelp()
    {
        // Ensure only one dialog displayed even for multiple hits of the F1 button
        if (!m_aboutDialog.IsWindow())
        {
            m_aboutDialog.DoModal(*this);
        }

        return TRUE;
    }

    // Called when the menu's modal loop begins (WM_INITMENUPOPUP received).
    template <class T>
    inline LRESULT CFrameT<T>::OnInitMenuPopup(UINT, WPARAM wparam, LPARAM lparam)
    {
        // The system menu shouldn't be owner drawn
        if (HIWORD(lparam))
            return CWnd::WndProcDefault(WM_INITMENUPOPUP, wparam, lparam);

        // Not supported on Win95 or WinNT
        if ((GetWinVersion() == 1400) || (GetWinVersion() == 2400))
            return CWnd::WndProcDefault(WM_INITMENUPOPUP, wparam, lparam);

        CMenu menu(reinterpret_cast<HMENU>(wparam));

        for (UINT i = 0; i < menu.GetMenuItemCount(); ++i)
        {
            MenuItemData* pItem = new MenuItemData;         // deleted in OnExitMenuLoop
            m_menuItemData.push_back(ItemDataPtr(pItem));  // Store pItem in smart pointer for later automatic deletion

            MENUITEMINFO mii;
            ZeroMemory(&mii, sizeof(mii));
            mii.cbSize = GetSizeofMenuItemInfo();

            // Use old fashioned MIIM_TYPE instead of MIIM_FTYPE for MS VC6 compatibility
            mii.fMask = MIIM_STATE | MIIM_ID | MIIM_SUBMENU |MIIM_CHECKMARKS | MIIM_TYPE | MIIM_DATA;
            mii.dwTypeData = pItem->GetItemText();  // assign TCHAR pointer, text is assigned by GetMenuItemInfo
            mii.cch = MAX_MENU_STRING;

            // Send message for menu updates
            UINT menuItem = menu.GetMenuItemID(i);
            T::SendMessage(UWM_UPDATECOMMAND, menuItem, 0);

            // Specify owner-draw for the menu item type
            if (menu.GetMenuItemInfo(i, mii, TRUE))
            {
                if (mii.dwItemData == 0)
                {
                    pItem->menu = menu;
                    pItem->pos = i;
                    pItem->mii = mii;
                    mii.dwItemData = reinterpret_cast<DWORD_PTR>(pItem);
                    mii.fType |= MFT_OWNERDRAW;
                    menu.SetMenuItemInfo(i, mii, TRUE); // Store pItem in mii
                }
            }
        }

        return 0;
    }

    // Called before the Popup menu is displayed, so that the MEASUREITEMSTRUCT
    // values can be assigned with the menu item's dimensions.
    template <class T>
    inline LRESULT CFrameT<T>::OnMeasureItem(UINT, WPARAM wparam, LPARAM lparam)
    {
        LPMEASUREITEMSTRUCT pmis = (LPMEASUREITEMSTRUCT) lparam;
        if (pmis->CtlType != ODT_MENU)
            return CWnd::WndProcDefault(WM_MEASUREITEM, wparam, lparam);

        MeasureMenuItem(pmis);
        return TRUE;
    }

    // Called when a menu is active, and a key is pressed other than an accelerator.
    template <class T>
    inline LRESULT CFrameT<T>::OnMenuChar(UINT, WPARAM wparam, LPARAM lparam)
    {
        if ((GetMenuBar().IsWindow()) && (LOWORD(wparam)!= VK_SPACE))
        {
            // Activate MenuBar for key pressed with Alt key held down
            GetMenuBar().OnMenuChar(WM_MENUCHAR, wparam, lparam);
            return -1;
        }

        return T::FinalWindowProc(WM_MENUCHAR, wparam, lparam);
    }

    // Called when a menu item is selected.
    template <class T>
    inline LRESULT CFrameT<T>::OnMenuSelect(UINT, WPARAM wparam, LPARAM lparam)
    {
        // Set the StatusBar text when we hover over a menu.
        // Only popup submenus have status strings.
        if (IsUsingMenuStatus() && GetStatusBar().IsWindow())
        {
            int id = LOWORD (wparam);
            CMenu menu(reinterpret_cast<HMENU>(lparam));

            if ((menu != T::GetMenu()) && (id != 0) && !(HIWORD(wparam) & MF_POPUP))
                GetStatusBar().SetWindowText(LoadString(id));
            else
                GetStatusBar().SetWindowText(m_statusText);

        }

        return 0;
    }

    // Called when a menu item is about to be displayed. Override this function to enable
    // or disable the menu item, or add a check mark. Also call this base function to
    // update the 'Tool Bar' and 'Status Bar' menu status.
    template <class T>
    inline void CFrameT<T>::OnMenuUpdate(UINT id)
    {
        // Update the check buttons before displaying the menu.
        switch(id)
        {
        case IDW_VIEW_STATUSBAR:
            {
                BOOL isVisible = GetStatusBar().IsWindow() && GetStatusBar().IsWindowVisible();
                GetFrameMenu().CheckMenuItem(id, isVisible ? MF_CHECKED : MF_UNCHECKED);
            }
            break;
        case IDW_VIEW_TOOLBAR:
            {
                BOOL isVisible = GetToolBar().IsWindow() && GetToolBar().IsWindowVisible();
                GetFrameMenu().EnableMenuItem(id, GetToolBar().IsWindow() ? MF_ENABLED : MF_DISABLED);
                GetFrameMenu().CheckMenuItem(id, isVisible ? MF_CHECKED : MF_UNCHECKED);
            }
            break;
        }
    }

    // Called when a notification from a child window (WM_NOTIFY) is received.
    template <class T>
    inline LRESULT CFrameT<T>::OnNotify(WPARAM wparam, LPARAM lparam)
    {
        UNREFERENCED_PARAMETER(wparam);

        LPNMHDR pNMHDR = (LPNMHDR)lparam;
        switch (pNMHDR->code)
        {
        case NM_CUSTOMDRAW:     return OnCustomDraw(pNMHDR);
        case RBN_HEIGHTCHANGE:  return OnRBNHeightChange(pNMHDR);
        case RBN_LAYOUTCHANGED: return OnRBNLayoutChanged(pNMHDR);
        case RBN_MINMAX:        return OnRBNMinMax(pNMHDR);
        case TBN_DROPDOWN:      return OnTBNDropDown((LPNMTOOLBAR)lparam);
        case TTN_GETDISPINFO:   return OnTTNGetDispInfo((LPNMTTDISPINFO)lparam);
        case UWN_UNDOCKED:      return OnUndocked();
        }

        return CWnd::OnNotify(wparam, lparam);
    }

    // Called when the rebar's height changes.
    template <class T>
    inline LRESULT CFrameT<T>::OnRBNHeightChange(LPNMHDR pNMHDR)
    {
        UNREFERENCED_PARAMETER(pNMHDR);

        RecalcLayout();

        return 0;
    }

    // Notification of rebar layout change.
    template <class T>
    inline LRESULT CFrameT<T>::OnRBNLayoutChanged(LPNMHDR pNMHDR)
    {
        UNREFERENCED_PARAMETER(pNMHDR);

        if (GetReBarTheme().UseThemes && GetReBarTheme().BandsLeft)
            GetReBar().MoveBandsLeft();

        return 0;
    }

    // Notification of a rebar band minimized or maximized.
    template <class T>
    inline LRESULT CFrameT<T>::OnRBNMinMax(LPNMHDR pNMHDR)
    {
        UNREFERENCED_PARAMETER(pNMHDR);

        if (GetReBarTheme().UseThemes && GetReBarTheme().ShortBands)
            return 1;  // Suppress maximise or minimise rebar band

        return 0;
    }

    // Press of drop-down button on ToolBar.
    template <class T>
    inline LRESULT CFrameT<T>::OnTBNDropDown(LPNMTOOLBAR pNMTB)
    {
        int item = pNMTB->iItem;
        CToolBar* pTB = static_cast<CToolBar*>(T::GetCWndPtr(pNMTB->hdr.hwndFrom));

        if (pTB)
        {
            // a boolean expression
            m_drawArrowBkgrnd = (pTB->GetButtonStyle(item) & TBSTYLE_DROPDOWN);
        }

        return 0;
    }

    // Tool tip notification from the toolbar.
    template <class T>
    inline LRESULT CFrameT<T>::OnTTNGetDispInfo(LPNMTTDISPINFO pNMTDI)
    {
        // Find the ToolBar that generated the tooltip
        CPoint pt(GetMessagePos());
        HWND wnd = ::WindowFromPoint(pt);
        CToolBar* pToolBar = reinterpret_cast<CToolBar*>(::SendMessage(wnd, UWM_GETCTOOLBAR, 0, 0));

        // Set the tooltip's text from the ToolBar button's CommandID.
        if (pToolBar && (pToolBar != &m_menuBar))
        {
            assert(dynamic_cast<CToolBar*>(pToolBar));
            LPNMTTDISPINFO lpDispInfo = pNMTDI;
            int index =  pToolBar->HitTest();
            if (index >= 0)
            {
                int id = pToolBar->GetCommandID(index);
                if (id > 0)
                {
                    m_tooltip = LoadString(id);
                    lpDispInfo->lpszText = const_cast<LPTSTR>(m_tooltip.c_str());
                }
                else
                    m_tooltip = _T("");
            }
        }

        return 0;
    }

    // Called when the frame window (not a child window) receives focus.
    template <class T>
    inline LRESULT CFrameT<T>::OnSetFocus(UINT, WPARAM wparam, LPARAM lparam)
    {
        UNREFERENCED_PARAMETER(wparam);
        UNREFERENCED_PARAMETER(lparam);

        SetStatusIndicators();
        return 0;
    }

    // Called when the SystemParametersInfo function changes a system-wide
    // setting or when policy settings have changed.
    template <class T>
    inline LRESULT CFrameT<T>::OnSettingChange(UINT, WPARAM, LPARAM)
    {
        T::RedrawWindow();
        return 0;
    }

    // Called when the frame window is resized.
    template <class T>
    inline LRESULT CFrameT<T>::OnSize(UINT, WPARAM wparam, LPARAM lparam)
    {
        UNREFERENCED_PARAMETER(wparam);
        UNREFERENCED_PARAMETER(lparam);

        RecalcLayout();
        return 0;
    }

    // Called in response to a WM_SYSCOLORCHANGE message. This message is sent
    // to all top-level windows when a change is made to a system color setting.
    template <class T>
    inline LRESULT CFrameT<T>::OnSysColorChange(UINT, WPARAM wparam, LPARAM lparam)
    {
        UNREFERENCED_PARAMETER(wparam);
        UNREFERENCED_PARAMETER(lparam);

        // Honour theme color changes
        if (GetReBar().IsWindow())
        {
            for (int band = 0; band <= GetReBar().GetBandCount(); ++band)
            {
                GetReBar().SetBandColor(band, GetSysColor(COLOR_BTNTEXT), GetSysColor(COLOR_BTNFACE));
            }
        }

        NONCLIENTMETRICS info = GetNonClientMetrics();

        if (GetStatusBar().IsWindow())
        {
            // Update the status bar font and text
            m_statusBarFont.DeleteObject();
            m_statusBarFont.CreateFontIndirect(info.lfStatusFont);
            GetStatusBar().SetFont(m_statusBarFont, TRUE);
            if (IsUsingMenuStatus())
                GetStatusBar().SetWindowText(m_statusText);

            SetStatusIndicators();
        }

        if (GetMenuBar().IsWindow())
        {
            // Update the MenuBar font and button size
            m_menuBarFont.DeleteObject();
            m_menuBarFont.CreateFontIndirect(info.lfMenuFont);
            GetMenuBar().SetFont(m_menuBarFont, TRUE);
            GetMenuBar().SetMenu( GetFrameMenu() );

            // Update the MenuBar band size
            UpdateMenuBarBandSize();
        }

        if (m_xpThemeName != GetThemeName())
            SetTheme();

        // Reposition and redraw everything
        RecalcLayout();
        T::RedrawWindow(RDW_ERASE | RDW_FRAME | RDW_INVALIDATE | RDW_ALLCHILDREN);

        // Forward the message to the view window
        if (GetView().IsWindow())
            GetView().PostMessage(WM_SYSCOLORCHANGE, 0, 0);

        return 0;
    }

    // Called in response to a WM_SYSCOMMAND notification. This notification
    // is passed on to the MenuBar to process alt keys and maximise or restore.
    template <class T>
    inline LRESULT CFrameT<T>::OnSysCommand(UINT, WPARAM wparam, LPARAM lparam)
    {
        if ((SC_KEYMENU == wparam) && (VK_SPACE != lparam) && GetMenuBar().IsWindow())
        {
            GetMenuBar().OnSysCommand(WM_SYSCOMMAND, wparam, lparam);
            return 0;
        }

        if (SC_MINIMIZE == wparam)
            m_oldFocus = ::GetFocus();

        // Pass remaining system commands on for default processing
        return T::FinalWindowProc(WM_SYSCOMMAND, wparam, lparam);
    }

    // Notification of undocked from CDocker received via OnNotify
    template <class T>
    inline LRESULT CFrameT<T>::OnUndocked()
    {
        m_oldFocus = 0;
        return 0;
    }

    // Called when the drop-down menu or submenu has been destroyed.
    // Win95 & WinNT don't support the WM_UNINITMENUPOPUP message.
    template <class T>
    inline LRESULT CFrameT<T>::OnUnInitMenuPopup(UINT, WPARAM wparam, LPARAM lparam)
    {
        UNREFERENCED_PARAMETER(wparam);
        UNREFERENCED_PARAMETER(lparam);

        for (int item = static_cast<int>(m_menuItemData.size()) - 1; item >= 0; --item)
        {
            // Undo OwnerDraw and put the text back.
            MENUITEMINFO mii;
            ZeroMemory(&mii, sizeof(mii));
            mii.cbSize = GetSizeofMenuItemInfo();
            mii.fMask = MIIM_TYPE | MIIM_DATA;
            mii.fType = m_menuItemData[item]->mii.fType;
            mii.dwTypeData = m_menuItemData[item]->GetItemText();
            mii.cch = lstrlen(m_menuItemData[item]->GetItemText());
            mii.dwItemData = 0;
            ::SetMenuItemInfo(m_menuItemData[item]->menu, m_menuItemData[item]->pos, TRUE, &mii);
            int pos = m_menuItemData[item]->pos;
            m_menuItemData.pop_back();

            // Break when we reach the top of this popup menu.
            if (pos == 0)
                break;
        }

        return 0;
    }

    // Called in response to menu input to display or hide the status bar.
    template <class T>
    inline BOOL CFrameT<T>::OnViewStatusBar()
    {
        BOOL show = !(GetStatusBar().IsWindow() && GetStatusBar().IsWindowVisible());
        ShowStatusBar(show);
        return TRUE;
    }

    // Called in response to menu input to display or hide the tool bar.
    template <class T>
    inline BOOL CFrameT<T>::OnViewToolBar()
    {
        BOOL show = GetToolBar().IsWindow() && !GetToolBar().IsWindowVisible();
        ShowToolBar(show);
        return TRUE;
    }

    // Sets frame window creation parameters prior to the frame window's creation.
    template <class T>
    inline void CFrameT<T>::PreCreate(CREATESTRUCT& cs)
    {
        // Set the frame window styles
        cs.style = WS_OVERLAPPEDWINDOW | WS_VISIBLE;

        if (GetInitValues().showCmd == SW_MAXIMIZE) cs.style |= WS_MAXIMIZE;

        CWindowDC dcDesktop(0);

        // Does the window fit on the desktop?
        CRect initPos = GetInitValues().position;
        if (RectVisible(dcDesktop, &initPos) && (initPos.Width() > 0))
        {
            // Set the original window position
            cs.x  = initPos.left;
            cs.y  = initPos.top;
            cs.cx = initPos.Width();
            cs.cy = initPos.Height();
        }
    }

    // Set the frame's class parameters prior to the frame window's creation.
    template <class T>
    inline void CFrameT<T>::PreRegisterClass(WNDCLASS& wc)
    {
        // Set the Window Class
        wc.lpszClassName =  _T("Win32++ Frame");
    }

    // Repositions the frame's child windows.
    template <class T>
    inline void CFrameT<T>::RecalcLayout()
    {
        // Resize the status bar
        if (GetStatusBar().IsWindow() && GetStatusBar().IsWindowVisible())
        {
            GetStatusBar().SetWindowPos(NULL, 0, 0, 0, 0, SWP_SHOWWINDOW);
            SetStatusIndicators();
        }

        // Resize the rebar or toolbar
        if (GetReBar().IsWindow())
        {
            GetReBar().SendMessage(WM_SIZE, 0, 0);
            GetReBar().Invalidate();
        }
        else if (GetToolBar().IsWindow() && GetToolBar().IsWindowVisible())
            GetToolBar().Autosize();

        // Position the view window
        if (GetView().IsWindow())
            RecalcViewLayout();

        // Adjust rebar bands
        if (GetReBar().IsWindow())
        {
            if (GetReBarTheme().UseThemes && GetReBarTheme().BandsLeft)
                GetReBar().MoveBandsLeft();

            if (GetMenuBar().IsWindow())
                SetMenuBarBandSize();
        }
    }

    // Resize the view window.
    template <class T>
    inline void CFrameT<T>::RecalcViewLayout()
    {
        GetView().SetWindowPos( NULL, GetViewRect(), SWP_SHOWWINDOW);
    }

    // Removes an entry from the MRU list.
    template <class T>
    inline void CFrameT<T>::RemoveMRUEntry(LPCTSTR pMRUEntry)
    {
        std::vector<CString>::iterator it;
        for (it = m_mruEntries.begin(); it != m_mruEntries.end(); ++it)
        {
            if ((*it) == pMRUEntry)
            {
                m_mruEntries.erase(it);
                break;
            }
        }

        UpdateMRUMenu();
    }

    // Saves the current MRU settings in the registry.
    template <class T>
    inline BOOL CFrameT<T>::SaveRegistryMRUSettings()
    {
        // Store the MRU entries in the registry
        try
        {
            // Delete Old MRUs
            CString keyParentName = _T("Software\\") + m_keyName;
            CRegKey keyParent;
            keyParent.Open(HKEY_CURRENT_USER, keyParentName);
            keyParent.DeleteSubKey(_T("Recent Files"));

            if (m_maxMRU > 0)
            {
                CString keyName = _T("Software\\") + m_keyName + _T("\\Recent Files");
                CRegKey key;

                // Add Current MRUs
                if (ERROR_SUCCESS != key.Create(HKEY_CURRENT_USER, keyName))
                    throw CUserException(_T("RegCreateKeyEx failed"));

                if (ERROR_SUCCESS != key.Open(HKEY_CURRENT_USER, keyName))
                    throw CUserException(_T("RegCreateKeyEx failed"));

                CString subKeyName;
                CString pathName;
                for (UINT i = 0; i < m_maxMRU; ++i)
                {
                    subKeyName.Format(_T("File %d"), i + 1);

                    if (i < m_mruEntries.size())
                    {
                        pathName = m_mruEntries[i];

                        if (ERROR_SUCCESS != key.SetStringValue(subKeyName, pathName.c_str()))
                            throw CUserException(_T("RegSetValueEx failed"));
                    }
                }
            }
        }

        catch (const CUserException& e)
        {
            Trace("*** Failed to save registry MRU settings. ***\n");
            Trace(e.GetText()); Trace("\n");

            CString keyName = _T("Software\\") + m_keyName;
            CRegKey key;

            if (ERROR_SUCCESS == key.Open(HKEY_CURRENT_USER, keyName))
            {
                // Roll back the registry changes by deleting this subkey
                key.DeleteSubKey(_T("Recent Files"));
            }

            return FALSE;
        }

        return TRUE;
    }

    // Saves various frame window settings in the registry.
    template <class T>
    inline BOOL CFrameT<T>::SaveRegistrySettings()
    {
        if (!m_keyName.IsEmpty())
        {
            try
            {
                CString keyName = _T("Software\\") + m_keyName + _T("\\Frame Settings");
                CRegKey key;

                if (ERROR_SUCCESS != key.Create(HKEY_CURRENT_USER, keyName))
                    throw CUserException(_T("RegCreateKeyEx failed"));
                if (ERROR_SUCCESS != key.Open(HKEY_CURRENT_USER, keyName))
                    throw CUserException(_T("RegCreateKeyEx failed"));

                // Store the window position in the registry
                WINDOWPLACEMENT wndpl;
                ZeroMemory(&wndpl, sizeof(wndpl));
                wndpl.length = sizeof(wndpl);

                if (T::GetWindowPlacement(wndpl))
                {
                    // Get the Frame's window position
                    CRect rc = wndpl.rcNormalPosition;
                    DWORD top = MAX(rc.top, 0);
                    DWORD left = MAX(rc.left, 0);
                    DWORD width = MAX(rc.Width(), 100);
                    DWORD height = MAX(rc.Height(), 50);
                    DWORD showCmd = wndpl.showCmd;

                    if (ERROR_SUCCESS != key.SetDWORDValue(_T("Top"), top))
                        throw CUserException(_T("RegSetValueEx failed"));
                    if (ERROR_SUCCESS != key.SetDWORDValue(_T("Left"), left))
                        throw CUserException(_T("RegSetValueEx failed"));
                    if (ERROR_SUCCESS != key.SetDWORDValue(_T("Width"), width))
                        throw CUserException(_T("RegSetValueEx failed"));
                    if (ERROR_SUCCESS != key.SetDWORDValue(_T("Height"), height))
                        throw CUserException(_T("RegSetValueEx failed"));
                    if (ERROR_SUCCESS != key.SetDWORDValue(_T("ShowCmd"), showCmd))
                        throw CUserException(_T("RegSetValueEx failed"));
                }

                // Store the ToolBar and statusbar states
                DWORD showToolBar = GetToolBar().IsWindow() && GetToolBar().IsWindowVisible();
                DWORD showStatusBar = GetStatusBar().IsWindow() && GetStatusBar().IsWindowVisible();

                if (ERROR_SUCCESS != key.SetDWORDValue(_T("ToolBar"), showToolBar))
                    throw CUserException(_T("RegSetValueEx failed"));
                if (ERROR_SUCCESS != key.SetDWORDValue(_T("StatusBar"), showStatusBar))
                    throw CUserException(_T("RegSetValueEx failed"));
            }

            catch (const CUserException& e)
            {
                Trace("*** Failed to save registry settings. ***\n");
                Trace(e.GetText()); Trace("\n");

                CString keyName = _T("Software\\") + m_keyName;
                CRegKey key;

                if (ERROR_SUCCESS == key.Open(HKEY_CURRENT_USER, keyName))
                {
                    // Roll back the registry changes by deleting this subkey
                    key.DeleteSubKey(_T("Frame Settings"));
                }

                return FALSE;
            }

            return SaveRegistryMRUSettings();
        }

        return TRUE;
    }

    // Sets the accelerator table for the application for this window.
    template <class T>
    inline void CFrameT<T>::SetAccelerators(UINT accelID)
    {
        m_accel = LoadAccelerators(GetApp()->GetResourceHandle(), MAKEINTRESOURCE(accelID));
        if (m_accel)
            GetApp()->SetAccelerators(m_accel, *this);
    }

    // Sets the frame's menu from a Resource ID.
    // A resource ID of 0 removes the menu from the frame.
    template <class T>
    inline void CFrameT<T>::SetFrameMenu(UINT menuID)
    {
        CMenu menu;
        if (menuID != 0)
        {
            // Sets the frame's menu from a resource ID.
            menu.LoadMenu(menuID);
            assert (::IsMenu(menu));
        }

        SetFrameMenu(menu);
    }

    // Sets the frame's menu.
    template <class T>
    inline void CFrameT<T>::SetFrameMenu(HMENU menu)
    {
        m_menu.Attach(menu);

        if (GetMenuBar().IsWindow())
        {
            GetMenuBar().SetMenu( GetFrameMenu() );
            BOOL show = (menu != NULL);    // boolean expression
            ShowMenu(show);
        }
        else
        {
            T::SetMenu(m_menu);
            T::DrawMenuBar();
        }
    }

    // Sets the initial values from the InitValues struct. The InitValues struct
    // is used to load and store the initial values, usually saved in the
    // registry or an INI file.
    template <class T>
    inline void CFrameT<T>::SetInitValues(const InitValues& values)
    {
        m_initValues = values;
    }

    // Sets the menu icons. Any previous menu icons are removed.
    template <class T>
    inline UINT CFrameT<T>::SetMenuIcons(const std::vector<UINT>& menuData, COLORREF mask, UINT toolBarID, UINT toolBarDisabledID)
    {
        // Remove any existing menu icons
        m_menuImages.DeleteImageList();
        m_menuDisabledImages.DeleteImageList();
        m_menuIcons.clear();

        // Exit if no ToolBarID is specified
        if (toolBarID == 0) return 0;

        // Add the menu icons from the bitmap IDs
        return AddMenuIcons(menuData, mask, toolBarID, toolBarDisabledID);
    }

    // Implements a keyboard hook. The hook is used to detect the CAPs lock,
    // NUM lock, Scroll lock and Insert keys.
    template <class T>
    inline void CFrameT<T>::SetKbdHook()
    {
        TLSData* pTLSData = GetApp()->SetTlsData();
        pTLSData->mainWnd = T::GetHwnd();
        m_kbdHook = ::SetWindowsHookEx(WH_KEYBOARD, StaticKeyboardProc, NULL, ::GetCurrentThreadId());
    }

    // Sets the minimum width of the MenuBar band to the width of the rebar.
    // This prevents other bands from moving to this MenuBar's row.
    template <class T>
    inline void CFrameT<T>::SetMenuBarBandSize()
    {
        CRect rcClient = T::GetClientRect();
        CReBar& rb = GetReBar();
        int band = rb.GetBand(GetMenuBar());
        if (band >= 0)
        {
            CRect rcBorder = rb.GetBandBorders(band);

            REBARBANDINFO rbbi;
            ZeroMemory(&rbbi, sizeof(rbbi));
            rbbi.fMask = RBBIM_CHILDSIZE | RBBIM_SIZE;
            rb.GetBandInfo(band, rbbi);

            int width;
            if ((GetReBarTheme().UseThemes) && (GetReBarTheme().LockMenuBand))
                width = rcClient.Width() - rcBorder.Width() - 2;
            else
                width = GetMenuBar().GetMaxSize().cx;

            rbbi.cxMinChild = width;
            rbbi.cx         = width;

            rb.SetBandInfo(band, rbbi);
        }
    }

    // Sets the theme colors for the MenuBar and the popup Menu items.
    // Note: If Aero Themes are supported, they are used for popup menu items instead.
    template <class T>
    inline void CFrameT<T>::SetMenuTheme(const MenuTheme& mt)
    {
        m_mbTheme = mt;

        if (GetMenuBar().IsWindow())
            GetMenuBar().Invalidate();
    }

    // Sets the maximum number of MRU entries.
    template <class T>
    inline void CFrameT<T>::SetMRULimit(UINT mruLimit)
    {
        // Remove any excess MRU entries
        if (mruLimit < m_mruEntries.size())
        {
            m_mruEntries.erase(m_mruEntries.begin() + mruLimit, m_mruEntries.end());
        }

        m_maxMRU = mruLimit;
        UpdateMRUMenu();
    }

    // Stores the rebar's theme colors.
    template <class T>
    inline void CFrameT<T>::SetReBarTheme(const ReBarTheme& rbt)
    {
        m_rbTheme = rbt;
    }

    // Stores the statusbar's theme colors.
    template <class T>
    inline void CFrameT<T>::SetStatusBarTheme(const StatusBarTheme& sbt)
    {
        m_sbTheme = sbt;
    }

    // Creates 4 panes in the status bar and displays status and key states.
    template <class T>
    inline void CFrameT<T>::SetStatusIndicators()
    {
        if (GetStatusBar().IsWindow() && (IsUsingIndicatorStatus()))
        {
            // Calculate the width of the text indicators
            CClientDC statusDC(GetStatusBar());
            statusDC.SelectObject(GetStatusBar().GetFont());
            CString cap = LoadString(IDW_INDICATOR_CAPS);
            CString num = LoadString(IDW_INDICATOR_NUM);
            CString scrl = LoadString(IDW_INDICATOR_SCRL);
            CSize capSize = statusDC.GetTextExtentPoint32(cap, cap.GetLength());
            CSize numSize = statusDC.GetTextExtentPoint32(num, num.GetLength());
            CSize scrlSize = statusDC.GetTextExtentPoint32(scrl, scrl.GetLength());

            BOOL hasGripper = GetStatusBar().GetStyle() & SBARS_SIZEGRIP;
            int cxGripper = hasGripper? 20 : 0;
            int cxBorder = 8;

            // Adjust for DPI aware
            int defaultDPI = 96;
            int xDPI = statusDC.GetDeviceCaps(LOGPIXELSX);
            cxGripper = MulDiv(cxGripper, xDPI, defaultDPI);
            capSize.cx += cxBorder;
            numSize.cx += cxBorder;
            scrlSize.cx += cxBorder;

            // Get the coordinates of the window's client area.
            CRect clientRect = T::GetClientRect();
            int width = MAX(300, clientRect.right);

            // Create 4 panes
            GetStatusBar().SetPartWidth(0, width - (capSize.cx + numSize.cx + scrlSize.cx + cxGripper));
            GetStatusBar().SetPartWidth(1, capSize.cx);
            GetStatusBar().SetPartWidth(2, numSize.cx);
            GetStatusBar().SetPartWidth(3, scrlSize.cx);

            CString status1 = (::GetKeyState(VK_CAPITAL) & 0x0001)? cap : CString("");
            CString status2 = (::GetKeyState(VK_NUMLOCK) & 0x0001)? num : CString("");
            CString status3 = (::GetKeyState(VK_SCROLL)  & 0x0001)? scrl: CString("");

            // Only update indicators if the text has changed
            if (status1 != m_oldStatus[0])  GetStatusBar().SetPartText(1, status1);
            if (status2 != m_oldStatus[1])  GetStatusBar().SetPartText(2, status2);
            if (status3 != m_oldStatus[2])  GetStatusBar().SetPartText(3, status3);

            m_oldStatus[0] = status1;
            m_oldStatus[1] = status2;
            m_oldStatus[2] = status3;
        }
    }

    // Stores the status text and displays it in the StatusBar.
    template <class T>
    inline void CFrameT<T>::SetStatusText(LPCTSTR text)
    {
        m_statusText = text;

        if (GetStatusBar().IsWindow())
        {
            // Place text in the 1st pane
            GetStatusBar().SetPartText(0, m_statusText);
        }
    }

    // Sets the theme colors for the frame's rebar, toolbar and menubar.
    // Override this function to modify the theme colors.
    template <class T>
    inline void CFrameT<T>::SetTheme()
    {
        // Avoid themes if using less than 16 bit colors
        CClientDC DesktopDC(NULL);
        if (DesktopDC.GetDeviceCaps(BITSPIXEL) < 16)
            UseThemes(FALSE);

        BOOL t = TRUE;
        BOOL f = FALSE;

        if (IsUsingThemes())
        {
            // Retrieve the XP theme name
            m_xpThemeName = GetThemeName();

            enum Themetype{ Win8, Win7, XP_Blue, XP_Silver, XP_Olive, Grey };

            // For Win2000 and below
            int theme = Grey;

            if (GetWinVersion() < 2600) // For Windows XP
            {
                if (m_xpThemeName == _T("NormalColor"))
                    theme = XP_Blue;
                if (m_xpThemeName == _T("Metallic"))
                    theme = XP_Silver;
                if (m_xpThemeName == _T("HomeStead"))
                    theme = XP_Olive;
            }
            else if (GetWinVersion() <= 2601)
            {
                // For Vista and Windows 7
                theme = Win7;
            }
            else
            {
                // For Windows 8 and above
                theme = Win8;
            }

            switch (theme)
            {
            case Win8:  // A pale blue scheme without gradients, suitable for Windows 8, 8.1, and 10
                {
                    MenuTheme mt = {t, RGB(180, 250, 255), RGB(140, 190, 255), RGB(240, 250, 255), RGB(120, 170, 220), RGB(127, 127, 255)};
                    ReBarTheme rbt = {t, RGB(235, 237, 250), RGB(235, 237, 250), RGB(235, 237, 250), RGB(235, 237, 250), f, t, t, f, t, f };
                    StatusBarTheme sbt = {t, RGB(235, 237, 250), RGB(235, 237, 250)};
                    ToolBarTheme tbt = {t, RGB(180, 250, 255), RGB(140, 190, 255), RGB(150, 220, 255), RGB(80, 100, 255), RGB(127, 127, 255)};

                    SetMenuTheme(mt);   // Sets the theme for popup menus and MenuBar
                    SetReBarTheme(rbt);
                    SetStatusBarTheme(sbt);
                    SetToolBarTheme(tbt);
                }
                break;

            case Win7:  // A pale blue color scheme suitable for Vista and Windows 7
                {
                    MenuTheme mt = {t, RGB(180, 250, 255), RGB(140, 190, 255), RGB(240, 250, 255), RGB(120, 170, 220), RGB(127, 127, 255)};
                    ReBarTheme rbt = {t, RGB(225, 230, 255), RGB(240, 242, 250), RGB(248, 248, 248), RGB(180, 200, 230), f, t, t, t, t, f};
                    StatusBarTheme sbt = {t, RGB(225, 230, 255), RGB(240, 242, 250)};
                    ToolBarTheme tbt = {t, RGB(180, 250, 255), RGB(140, 190, 255), RGB(150, 220, 255), RGB(80, 100, 255), RGB(127, 127, 255)};

                    SetMenuTheme(mt);   // Sets the theme for popup menus and MenuBar
                    SetReBarTheme(rbt);
                    SetStatusBarTheme(sbt);
                    SetToolBarTheme(tbt);
                }
                break;


            case XP_Blue:
                {
                    // Used for XP default (blue) color scheme
                    MenuTheme mt = {t, RGB(255, 230, 190), RGB(255, 190, 100), RGB(220,230,250), RGB(150,190,245), RGB(128, 128, 200)};
                    ReBarTheme rbt = {t, RGB(150,190,245), RGB(196,215,250), RGB(220,230,250), RGB( 70,130,220), f, t, t, t, t, f};
                    StatusBarTheme sbt = {t, RGB(150,190,245), RGB(196,215,250)};
                    ToolBarTheme tbt = {t, RGB(255, 230, 190), RGB(255, 190, 100), RGB(255, 140, 40), RGB(255, 180, 80), RGB(192, 128, 255)};

                    SetMenuTheme(mt);   // Sets the theme for popup menus and MenuBar
                    SetReBarTheme(rbt);
                    SetStatusBarTheme(sbt);
                    SetToolBarTheme(tbt);
                }
                break;

            case XP_Silver:
                {
                    // Used for XP Silver color scheme
                    MenuTheme mt = {t, RGB(196, 215, 250), RGB( 120, 180, 220), RGB(240, 240, 245), RGB(170, 165, 185), RGB(128, 128, 150)};
                    ReBarTheme rbt = {t, RGB(225, 220, 240), RGB(240, 240, 245), RGB(245, 240, 255), RGB(160, 155, 180), f, t, t, t, t, f};
                    StatusBarTheme sbt = {t, RGB(225, 220, 240), RGB(240, 240, 245)};
                    ToolBarTheme tbt = {t, RGB(192, 210, 238), RGB(192, 210, 238), RGB(152, 181, 226), RGB(152, 181, 226), RGB(49, 106, 197)};

                    SetMenuTheme(mt);   // Sets the theme for popup menus and MenuBar
                    SetReBarTheme(rbt);
                    SetStatusBarTheme(sbt);
                    SetToolBarTheme(tbt);
                }
                break;

            case XP_Olive:
                {
                    // Used for XP Olive color scheme
                    MenuTheme mt = {t, RGB(255, 230, 190), RGB(255, 190, 100), RGB(249, 255, 227), RGB(178, 191, 145), RGB(128, 128, 128)};
                    ReBarTheme rbt = {t, RGB(215, 216, 182), RGB(242, 242, 230), RGB(249, 255, 227), RGB(178, 191, 145), f, t, t, t, t, f};
                    StatusBarTheme sbt = {t, RGB(215, 216, 182), RGB(242, 242, 230)};
                    ToolBarTheme tbt = {t, RGB(255, 230, 190), RGB(255, 190, 100), RGB(255, 140, 40), RGB(255, 180, 80), RGB(200, 128, 128)};

                    SetMenuTheme(mt);   // Sets the theme for popup menus and MenuBar
                    SetReBarTheme(rbt);
                    SetStatusBarTheme(sbt);
                    SetToolBarTheme(tbt);
                }
                break;

            case Grey:  // A color scheme suitable for 16 bit colors. Suitable for Windows older than XP.
                {
                    MenuTheme mt = {t, RGB(182, 189, 210), RGB( 182, 189, 210), RGB(200, 196, 190), RGB(200, 196, 190), RGB(100, 100, 100)};
                    ReBarTheme rbt = {t, RGB(212, 208, 200), RGB(212, 208, 200), RGB(230, 226, 222), RGB(220, 218, 208), f, t, t, t, t, f};
                    StatusBarTheme sbt = {t, RGB(212, 208, 200), RGB(212, 208, 200)};
                    ToolBarTheme tbt = {t, RGB(182, 189, 210), RGB(182, 189, 210), RGB(133, 146, 181), RGB(133, 146, 181), RGB(10, 36, 106)};

                    SetMenuTheme(mt);   // Sets the theme for popup menus and MenuBar
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

    // Sets the Image List for additional Toolbars.
    // The specified CToolBar should be a member of CMainFrame to ensure it remains in scope.
    // The specified CImageList should be a member of CMainFrame to ensure it remains in scope.
    // A Disabled image list is created from ToolBarID if one doesn't already exist.
    template <class T>
    inline void CFrameT<T>::SetTBImageList(CToolBar& toolBar, CImageList& imageList, UINT id, COLORREF mask)
    {
        // Get the image size
        CBitmap bm(id);
        CSize sz = GetTBImageSize(&bm);

        // Set the toolbar's image list
        imageList.DeleteImageList();
        imageList.Create(sz.cx, sz.cy, ILC_COLOR32 | ILC_MASK, 0, 0);
        imageList.Add(bm, mask);
        toolBar.SetImageList(imageList);

        // Inform the Rebar of the change to the Toolbar
        if (GetReBar().IsWindow())
        {
            SIZE MaxSize = toolBar.GetMaxSize();
            GetReBar().SendMessage(UWM_TBRESIZE, (WPARAM)(toolBar.GetHwnd()),
                (LPARAM)(&MaxSize));
        }
    }

    // Sets the Disabled Image List for additional Toolbars.
    // The specified CToolBar should be a member of CMainFrame to ensure it remains in scope.
    // The specified CImageList should be a member of CMainFrame to ensure it remains in scope.
    template <class T>
    inline void CFrameT<T>::SetTBImageListDis(CToolBar& toolBar, CImageList& imageList, UINT id, COLORREF mask)
    {
        if (id != 0)
        {
            // Get the image size
            CBitmap bm(id);
            CSize sz = GetTBImageSize(&bm);

            // Set the toolbar's image list
            imageList.DeleteImageList();
            imageList.Create(sz.cx, sz.cy, ILC_COLOR32 | ILC_MASK, 0, 0);
            imageList.Add(bm, mask);
            toolBar.SetDisableImageList(imageList);
        }
        else
        {
            imageList.DeleteImageList();
            imageList.CreateDisabledImageList(m_toolBarImages);
            toolBar.SetDisableImageList(imageList);
        }

        // Inform the Rebar of the change to the Toolbar
        if (GetReBar().IsWindow())
        {
            SIZE maxSize = toolBar.GetMaxSize();
            GetReBar().SendMessage(UWM_TBRESIZE, (WPARAM)(toolBar.GetHwnd()),
                (LPARAM)(&maxSize));
        }
    }

    // Sets the Hot Image List for additional Toolbars.
    // The specified CToolBar should be a member of CMainFrame to ensure it remains in scope.
    // The specified CImageList should be a member of CMainFrame to ensure it remains in scope.
    template <class T>
    inline void CFrameT<T>::SetTBImageListHot(CToolBar& toolBar, CImageList& imageList, UINT id, COLORREF mask)
    {
        if (id != 0)
        {
            // Get the image size
            CBitmap bm(id);
            CSize sz = GetTBImageSize(&bm);

            // Set the toolbar's image list
            imageList.DeleteImageList();
            imageList.Create(sz.cx, sz.cy, ILC_COLOR32 | ILC_MASK, 0, 0);
            imageList.Add(bm, mask);
            toolBar.SetHotImageList(imageList);
        }
        else
        {
            imageList.DeleteImageList();
            toolBar.SetHotImageList(0);
        }

        // Inform the Rebar of the change to the Toolbar
        if (GetReBar().IsWindow())
        {
            SIZE MaxSize = toolBar.GetMaxSize();
            GetReBar().SendMessage(UWM_TBRESIZE, (WPARAM)(toolBar.GetHwnd()),
                (LPARAM)(&MaxSize));
        }
    }

    // Either sets the imagelist or adds/replaces bitmap depending on ComCtl32.dll version
    // The ToolBarIDs are bitmap resources containing a set of toolbar button images.
    // Each toolbar button image must have a minimum height of 16. Its height must equal its width.
    // The colour mask is ignored for 32bit bitmaps, but is required for 24bit bitmaps
    // The colour mask is often grey RGB(192,192,192) or magenta (255,0,255)
    // The Hot and disabled bitmap resources can be 0.
    // A Disabled image list is created from ToolBarID if one isn't provided.
    template <class T>
    inline void CFrameT<T>::SetToolBarImages(COLORREF mask, UINT toolBarID, UINT toolBarHotID, UINT toolBarDisabledID)
    {
        if (GetComCtlVersion() < 470)   // only on Win95
        {
            // We are using COMCTL32.DLL version 4.0, so we can't use an ImageList.
            // Instead we simply set the bitmap.
            GetToolBar().AddReplaceBitmap(toolBarID);
            return;
        }

        // Set the button images
        SetTBImageList(GetToolBar(),    m_toolBarImages, toolBarID, mask);
        SetTBImageListHot(GetToolBar(), m_toolBarHotImages, toolBarHotID, mask);
        SetTBImageListDis(GetToolBar(), m_toolBarDisabledImages, toolBarDisabledID, mask);
    }

    // Assigns icons to the dropdown menu items. By default the toolbar icons are
    // added to the menu items. Override this function to assign additional or
    // different icons to the drop down menu items.
    template <class T>
    inline void CFrameT<T>::SetupMenuIcons()
    {
        // Add the set of toolbar images to the menu
        if (GetToolBarData().size() > 0)
        {
            // Add the icons for popup menu
            AddMenuIcons(GetToolBarData(), RGB(192, 192, 192), IDW_MAIN, 0);
        }
    }

    // Override this function to set the Resource IDs for the toolbar(s).
    template <class T>
    inline void CFrameT<T>::SetupToolBar()
    {
/*      // Set the Resource IDs for the toolbar buttons
        AddToolBarButton( IDM_FILE_NEW   );
        AddToolBarButton( IDM_FILE_OPEN  );
        AddToolBarButton( IDM_FILE_SAVE  );

        AddToolBarButton( 0 );                      // Separator
        AddToolBarButton( IDM_EDIT_CUT,   FALSE );  // disabled button
        AddToolBarButton( IDM_EDIT_COPY,  FALSE );  // disabled button
        AddToolBarButton( IDM_EDIT_PASTE, FALSE );  // disabled button

        AddToolBarButton( 0 );                      // Separator
        AddToolBarButton( IDM_FILE_PRINT );

        AddToolBarButton( 0 );                      // Separator
        AddToolBarButton( IDM_HELP_ABOUT );
*/
    }

    // Stores the tool bar's theme colors
    template <class T>
    inline void CFrameT<T>::SetToolBarTheme(const ToolBarTheme& tbt)
    {
        m_tbTheme = tbt;
        if (GetToolBar().IsWindow())
            GetToolBar().GetParent().RedrawWindow(RDW_INVALIDATE|RDW_ALLCHILDREN);
    }

    // Sets or changes the frame's view window.
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

    // Hides or shows the menu.
    template <class T>
    inline void CFrameT<T>::ShowMenu(BOOL show)
    {
        if (show)
        {
            if (GetReBar().IsWindow())
                GetReBar().ShowBand(GetReBar().GetBand(GetMenuBar()), TRUE);
            else
                T::SetMenu(m_menu);
        }
        else
        {
            if (GetReBar().IsWindow())
                GetReBar().ShowBand(GetReBar().GetBand(GetMenuBar()), FALSE);
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

    // Hides or shows the status bar.
    template <class T>
    inline void CFrameT<T>::ShowStatusBar(BOOL show)
    {
        if (GetStatusBar().IsWindow())
        {
            if (show)
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

    // Hides or shows the tool bar.
    template <class T>
    inline void CFrameT<T>::ShowToolBar(BOOL show)
    {
        if (GetToolBar().IsWindow())
        {
            if (show)
            {
                if (GetReBar().IsWindow())
                    GetReBar().ShowBand(GetReBar().GetBand(GetToolBar()), TRUE);
                else
                    GetToolBar().ShowWindow(SW_SHOW);
            }
            else
            {
                if (GetReBar().IsWindow())
                    GetReBar().ShowBand(GetReBar().GetBand(GetToolBar()), FALSE);
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

    // Called by the keyboard hook to update status information.
    template <class T>
    inline LRESULT CALLBACK CFrameT<T>::StaticKeyboardProc(int code, WPARAM wparam, LPARAM lparam)
    {
        TLSData* pTLSData = GetApp()->GetTlsData();
        HWND hFrame = pTLSData->mainWnd;
        CFrameT<T>* pFrame = reinterpret_cast< CFrameT<T>* >(CWnd::GetCWndPtr(hFrame));
        assert(pFrame);

        if (pFrame && HC_ACTION == code)
        {
            if ((wparam ==  VK_CAPITAL) || (wparam == VK_NUMLOCK) ||
                (wparam == VK_SCROLL) || (wparam == VK_INSERT))
            {
                pFrame->SetStatusIndicators();
            }
        }

        // The HHOOK parameter in CallNextHookEx should be supplied for Win95, Win98 and WinME.
        // The HHOOK parameter is ignored for Windows NT and above.
        return pFrame? ::CallNextHookEx(pFrame->m_kbdHook, code, wparam, lparam) : 0;
    }

    // Update the MenuBar band size.
    template <class T>
    inline void CFrameT<T>::UpdateMenuBarBandSize()
    {
        int nBand = GetReBar().GetBand(GetMenuBar());
        if (nBand >= 0)
        {
            REBARBANDINFO rbbi;
            ZeroMemory(&rbbi, sizeof(rbbi));
            CClientDC menuBarDC(GetMenuBar());
            menuBarDC.SelectObject(GetMenuBar().GetFont());
            CSize sizeMenuBar = menuBarDC.GetTextExtentPoint32(_T("\tSomeText"), lstrlen(_T("\tSomeText")));
            int MenuBar_Height = sizeMenuBar.cy + 6;
            rbbi.fMask      = RBBIM_CHILDSIZE;
            rbbi.cyMinChild = MenuBar_Height;
            rbbi.cyMaxChild = MenuBar_Height;
            GetReBar().SetBandInfo(nBand, rbbi);
        }
    }

    // Updates the menu item information for the Most Recently Used (MRU) list.
    template <class T>
    inline void CFrameT<T>::UpdateMRUMenu()
    {
        if (!T::IsWindow() ) return;

        // Create a vector of CStrings containing the MRU menu entries
        std::vector<CString> mruStrings;

        if (m_mruEntries.size() > 0)
        {
            for (UINT n = 0; n < m_mruEntries.size(); ++n)
            {
                CString str = m_mruEntries[n];

                // Prefix the string with its number
                CString count;
                count.Format(_T("%d "), n + 1);
                str = count + str;

                // Trim the string if its too long
                if (str.GetLength() > MAX_MENU_STRING)
                {
                    // Extract the first part of the string up until the first '\\'
                    CString prefix;
                    int index = str.Find(_T('\\'));
                    if (index >= 0)
                        prefix = str.Left(index + 1);

                    // Reduce the string to fit within MAX_MENU_STRING
                    CString gap = _T("...");
                    str.Delete(0, str.GetLength() - MAX_MENU_STRING + prefix.GetLength() + gap.GetLength()+1);

                    // Remove the front of the string up to the next '\\' if any.
                    index = str.Find(_T('\\'));
                    if (index >= 0)
                        str.Delete(0, index);

                    str = prefix + gap + str;
                }

                mruStrings.push_back(str);
            }
        }
        else
        {
            mruStrings.push_back(_T("Recent Files"));
        }

        // Set MRU menu items
        MENUITEMINFO mii;
        ZeroMemory(&mii, sizeof(mii));
        mii.cbSize = GetSizeofMenuItemInfo();

        // We place the MRU items under the left most menu item
        CMenu fileMenu = GetFrameMenu().GetSubMenu(0);

        if (fileMenu.GetHandle())
        {
            // Remove all but the first MRU Menu entry
            for (UINT u = IDW_FILE_MRU_FILE2; u <= IDW_FILE_MRU_FILE1 + mruStrings.size(); ++u)
            {
                fileMenu.DeleteMenu(u, MF_BYCOMMAND);
            }

            int maxMRUIndex = static_cast<int>(mruStrings.size() -1);

            for (int index = maxMRUIndex; index >= 0; --index)
            {
                mii.fMask = MIIM_TYPE | MIIM_ID | MIIM_STATE;
                mii.fState = (0 == m_mruEntries.size())? MFS_GRAYED : 0;
                mii.fType = MFT_STRING;
                mii.wID = IDW_FILE_MRU_FILE1 + index;
                mii.dwTypeData = const_cast<LPTSTR>(mruStrings[index].c_str());

                BOOL result;
                if (index == maxMRUIndex)
                    // Replace the last MRU entry first
                    result = fileMenu.SetMenuItemInfo(IDW_FILE_MRU_FILE1, mii, FALSE);
                else
                    // Insert the other MRU entries next
                    result = fileMenu.InsertMenuItem(IDW_FILE_MRU_FILE1 + index + 1, mii, FALSE);

                if (!result)
                {
                    TRACE("Failed to set MRU menu item\n");
                    break;
                }
            }
        }

        T::DrawMenuBar();
    }

    // Provides default processing of the frame window's messages.
    // The frame's WndProc function should pass unhandled window messages to this function.
    template <class T>
    inline LRESULT CFrameT<T>::WndProcDefault(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        switch (msg)
        {
        case WM_ACTIVATE:       return OnActivate(msg, wparam, lparam);
        case WM_DRAWITEM:       return OnDrawItem(msg, wparam, lparam);
        case WM_ERASEBKGND:     return 0;
        case WM_HELP:           return OnHelp();
        case WM_INITMENUPOPUP:  return OnInitMenuPopup(msg, wparam, lparam);
        case WM_MENUCHAR:       return OnMenuChar(msg, wparam, lparam);
        case WM_MEASUREITEM:    return OnMeasureItem(msg, wparam, lparam);
        case WM_MENUSELECT:     return OnMenuSelect(msg, wparam, lparam);
        case WM_SETFOCUS:       return OnSetFocus(msg, wparam, lparam);
        case WM_SETTINGCHANGE:  return OnSettingChange(msg, wparam, lparam);
        case WM_SIZE:           return OnSize(msg, wparam, lparam);
        case WM_SYSCOLORCHANGE: return OnSysColorChange(msg, wparam, lparam);
        case WM_SYSCOMMAND:     return OnSysCommand(msg, wparam, lparam);
        case WM_UNINITMENUPOPUP:  return OnUnInitMenuPopup(msg, wparam, lparam);
        case WM_WINDOWPOSCHANGED: return T::FinalWindowProc(msg, wparam, lparam);

        // Messages defined by Win32++
        case UWM_GETFRAMEVIEW:      return reinterpret_cast<LRESULT>(GetView().GetHwnd());
        case UWM_GETMBTHEME:        return reinterpret_cast<LRESULT>(&GetMenuBarTheme());
        case UWM_GETRBTHEME:        return reinterpret_cast<LRESULT>(&GetReBarTheme());
        case UWM_GETSBTHEME:        return reinterpret_cast<LRESULT>(&GetStatusBarTheme());
        case UWM_GETTBTHEME:        return reinterpret_cast<LRESULT>(&GetToolBarTheme());
        case UWM_DRAWRBBKGND:       return OnDrawRBBkgnd(msg, wparam, lparam);
        case UWM_DRAWSBBKGND:       return OnDrawSBBkgnd(msg, wparam, lparam);
        case UWM_GETCFRAMET:        return reinterpret_cast<LRESULT>(this);

        } // switch msg

        return CWnd::WndProcDefault(msg, wparam, lparam);
    }


} // namespace Win32xx

#if defined (_MSC_VER) && (_MSC_VER >= 1400)
#pragma warning ( pop )
#endif // (_MSC_VER) && (_MSC_VER >= 1400)

#endif // _WIN32XX_FRAME_H_
