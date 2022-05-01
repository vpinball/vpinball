// Win32++   Version 9.0
// Release Date: 30th April 2022
//
//      David Nash
//      email: dnash@bigpond.net.au
//      url: https://sourceforge.net/projects/win32-framework
//
//
// Copyright (c) 2005-2022  David Nash
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
//  CFrameT, CFrame, and CDockFrame.

// The classes declared in this file support frames with the Single
// Document Interface (SDI). Refer to wxx_mdi.h for support for the
// Multiple Document Interface (MDI) frames.

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
#include "wxx_menumetrics.h"
#include "default_resource.h"


#ifndef RBN_MINMAX
  #define RBN_MINMAX (RBN_FIRST - 21)
#endif

#ifndef WM_UNINITMENUPOPUP
  #define WM_UNINITMENUPOPUP        0x0125
#endif

#ifndef WM_MENURBUTTONUP
  #define WM_MENURBUTTONUP      0x0122
#endif

#ifndef WM_DPICHANGED
  #define WM_DPICHANGED         0x02E0
#endif

#if defined (_MSC_VER) && (_MSC_VER >= 1920)   // >= VS2019
  #pragma warning ( push )
  #pragma warning ( disable : 26812 )            // enum type is unscoped.
#endif // (_MSC_VER) && (_MSC_VER >= 1920)


namespace Win32xx
{

    //////////////////////////////////
    // CFrameT is the base class for all frames in Win32++.
    // The template parameter T is either CWnd or CDocker.
    template <class T>
    class CFrameT : public T
    {
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

        // A local copy of the MENUPARTS enum from uxtheme.h. It's declared within
        // the CFrameT class to avoid name clashes when uxtheme.h is included.
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

        CFrameT();
        virtual ~CFrameT();

        // Override these functions as required
        virtual void AdjustFrameRect(const RECT& viewRect);
        virtual CRect GetViewRect() const;
        virtual void SetStatusIndicators();
        virtual void RecalcLayout();
        virtual void RecalcViewLayout();
        virtual void SetStatusParts();

        // Virtual accessors and mutators
        virtual CWnd& GetView() const;
        virtual void SetView(CWnd& view);

        // Virtual state functions
        virtual BOOL IsMDIChildMaxed() const { return FALSE; }
        virtual BOOL IsMDIFrame() const { return FALSE; }

        // Accessors and mutators for the menubar, rebar, statusbar and toolbar.
        CMenuBar& GetMenuBar() const { return *m_pMenuBar; }
        CReBar& GetReBar() const { return *m_pReBar; }
        CStatusBar& GetStatusBar() const { return *m_pStatusBar; }
        CToolBar& GetToolBar() const { return *m_pToolBar; }
        void SetMenuBar(CMenuBar& menuBar) { m_pMenuBar = &menuBar; }
        void SetReBar(CReBar& reBar) { m_pReBar = &reBar; }
        void SetStatusBar(CStatusBar& statusBar) { m_pStatusBar = &statusBar; }
        void SetToolBar(CToolBar& toolBar) { m_pToolBar = &toolBar; }

        // Other accessors and mutators.
        HACCEL GetFrameAccel() const                      { return m_accel; }
        const CMenu&  GetFrameMenu() const                { return m_menu; }
        const InitValues& GetInitValues() const           { return m_initValues; }
        const MenuTheme& GetMenuBarTheme() const          { return m_mbTheme; }
        const CMenuMetrics& GetMenuMetrics() const        { return m_menuMetrics; }
        const std::vector<CString>& GetMRUEntries() const { return m_mruEntries; }
        CString GetMRUEntry(UINT index);
        UINT GetMRULimit() const                          { return m_maxMRU; }
        CString GetRegistryKeyName() const                { return m_keyName; }
        const ReBarTheme& GetReBarTheme() const           { return m_rbTheme; }
        const StatusBarTheme& GetStatusBarTheme() const   { return m_sbTheme; }
        const std::vector<UINT>& GetToolBarData() const   { return m_toolBarData; }
        const ToolBarTheme& GetToolBarTheme() const       { return m_tbTheme; }
        CString GetStatusText() const                     { return m_statusText; }
        CString GetTitle() const                          { return T::GetWindowText(); }
        CString GetXPThemeName() const;
        void SetAccelerators(UINT accelID);
        void SetFrameMenu(int menuID);
        void SetFrameMenu(HMENU menu);
        void SetInitValues(const InitValues& values);
        void SetKbdHook();
        void SetMenuTheme(const MenuTheme& mt);
        void SetMRULimit(UINT MRULimit);
        void SetReBarTheme(const ReBarTheme& rbt);
        void SetStatusBarTheme(const StatusBarTheme& sbt);
        void SetStatusText(LPCTSTR text);
        void SetTitle(LPCTSTR text)                       { T::SetWindowText(text); }
        void SetToolBarTheme(const ToolBarTheme& tbt);

    protected:
        // Override these functions as required.
        virtual void AddDisabledMenuImage(HICON icon, COLORREF mask, int iconWidth = 16);
        virtual BOOL AddMenuIcon(int menuItemID, int iconID, int iconWidth = 16);
        virtual BOOL AddMenuIcon(int menuItemID, HICON icon, int iconWidth = 16);
        virtual UINT AddMenuIcons(const std::vector<UINT>& menuData, COLORREF mask, UINT bitmapID, UINT disabledID);
        virtual void AddMenuBarBand();
        virtual void AddMRUEntry(LPCTSTR MRUEntry);
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
        virtual void DrawVistaMenuBkgnd(LPDRAWITEMSTRUCT pDIS) const;
        virtual void DrawVistaMenuCheckmark(LPDRAWITEMSTRUCT pDIS) const;
        virtual void DrawVistaMenuText(LPDRAWITEMSTRUCT pDIS) const;
        virtual int  GetMenuItemPos(HMENU menu, LPCTSTR itemName) const;
        virtual BOOL LoadRegistrySettings(LPCTSTR keyName);
        virtual BOOL LoadRegistryMRUSettings(UINT maxMRU = 0);
        virtual void MeasureMenuItem(MEASUREITEMSTRUCT* pMIS);
        virtual LRESULT OnActivate(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual void    OnClose();
        virtual int     OnCreate(CREATESTRUCT& cs);
        virtual LRESULT OnCustomDraw(LPNMHDR pNMHDR);
        virtual void    OnDestroy();
        virtual LRESULT OnDrawItem(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual LRESULT OnDrawRBBkgnd(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual LRESULT OnDrawSBBkgnd(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual BOOL    OnHelp();
        virtual LRESULT OnInitMenuPopup(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual void    OnKeyboardHook(int code, WPARAM wparam, LPARAM lparam);
        virtual LRESULT OnMeasureItem(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual LRESULT OnMenuChar(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual LRESULT OnMenuSelect(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual void    OnMenuUpdate(UINT id);
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
        virtual BOOL    OnViewStatusBar();
        virtual BOOL    OnViewToolBar();
        virtual void PreCreate(CREATESTRUCT& cs);
        virtual void PreRegisterClass(WNDCLASS& wc);
        virtual void RemoveMRUEntry(LPCTSTR MRUEntry);
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
        CRect ExcludeChildRect(const CRect& clientRect, HWND child) const;
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
        CMenuBar m_menuBar;                 // Default CMenuBar object
        CReBar m_reBar;                     // Default CReBar object
        CStatusBar m_statusBar;             // Default CStatusBar object
        CToolBar m_toolBar;                 // Default CToolBar object
        CMenuBar* m_pMenuBar;               // Pointer to the CMenuBar object we actually use
        CReBar* m_pReBar;                   // Pointer to the CReBar object we actually use
        CStatusBar* m_pStatusBar;           // Pointer to the CStatusBar object we actually use
        CToolBar* m_pToolBar;               // Pointer to the CToolBar object we actually use
        CMenu m_menu;                       // handle to the frame menu
        CFont m_menuBarFont;                // MenuBar font
        CFont m_statusBarFont;              // StatusBar font
        CImageList m_toolBarImages;         // Image list for the ToolBar buttons
        CImageList m_toolBarDisabledImages; // Image list for the Disabled ToolBar buttons
        CImageList m_toolBarHotImages;      // Image list for the Hot ToolBar buttons
        CString m_keyName;                  // CString for Registry key name
        CString m_statusText;               // CString for status text
        CString m_tooltip;                  // CString for tool tips
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
        BOOL m_altKeyPressed;               // set to TRUE if the alt key is held down;

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

    ///////////////////////////////////
    // Definitions for the CFrame class
    //
    template <class T>
    inline CFrameT<T>::CFrameT() : m_aboutDialog(IDW_ABOUT), m_accel(0), m_pView(NULL), m_maxMRU(0), m_oldFocus(0),
                              m_drawArrowBkgrnd(FALSE), m_kbdHook(0), m_useIndicatorStatus(TRUE),
                              m_useMenuStatus(TRUE), m_useStatusBar(TRUE), m_useThemes(TRUE), m_useToolBar(TRUE),
                              m_altKeyPressed(FALSE)
    {
        ZeroMemory(&m_mbTheme, sizeof(m_mbTheme));
        ZeroMemory(&m_rbTheme, sizeof(m_rbTheme));
        ZeroMemory(&m_sbTheme, sizeof(m_sbTheme));
        ZeroMemory(&m_tbTheme, sizeof(m_tbTheme));

        // By default, we use the rebar if we can.
        m_useReBar = (GetComCtlVersion() > 470)? TRUE : FALSE;

        // Assign the default menubar, rebar, statusbar and toolbar.
        SetMenuBar(m_menuBar);
        SetReBar(m_reBar);
        SetStatusBar(m_statusBar);
        SetToolBar(m_toolBar);

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
        CClientDC desktopDC(0);
        CMemDC memDC(0);
        int cx = iconWidth;
        int cy = iconWidth;

        memDC.CreateCompatibleBitmap(desktopDC, cx, cy);
        CRect rc;
        rc.SetRect(0, 0, cx, cy);

        // Set the mask color to gray for the new ImageList
        if (GetDeviceCaps(desktopDC, BITSPIXEL) < 24)
        {
            HPALETTE hPal = desktopDC.GetCurrentPalette();
            UINT index = ::GetNearestPaletteIndex(hPal, mask);
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
        if (0 == m_menuImages.GetHandle())
        {
            m_menuImages.Create(cxImage, cyImage, ILC_COLOR32 | ILC_MASK, 1, 0);
            m_menuIcons.clear();
        }

        if (m_menuImages.Add(icon) != -1)
        {
            m_menuIcons.push_back(menuItemID);

            // Set the mask color to gray for the new ImageList
            COLORREF mask = RGB(192, 192, 192);
            CClientDC desktopDC(HWND_DESKTOP);
            if (GetDeviceCaps(desktopDC, BITSPIXEL) < 24)
            {
                HPALETTE hPal = desktopDC.GetCurrentPalette();
                UINT index = ::GetNearestPaletteIndex(hPal, mask);
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
        if (0 == m_menuImages.GetHandle())
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
        VERIFY(AdjustWindowRectEx(&rc, style, FALSE, exStyle));

        // Calculate the new frame height
        CRect frameBefore = T::GetWindowRect();
        CRect viewBefore = GetViewRect();
        int height = rc.Height() + frameBefore.Height() - viewBefore.Height();

        // Adjust for the frame styles
        style = T::GetStyle();
        exStyle = T::GetExStyle();
        VERIFY(AdjustWindowRectEx(&rc, style, FALSE, exStyle));

        // Calculate final rect size, and reposition frame
        VERIFY(T::SetWindowPos(0, 0, 0, rc.Width(), height, SWP_NOMOVE));
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

    // CustomDraw is used to render the MenuBar's toolbar buttons.
    template <class T>
    inline LRESULT CFrameT<T>::CustomDrawMenuBar(NMHDR* pNMHDR)
    {
        LPNMTBCUSTOMDRAW lpNMCustomDraw = (LPNMTBCUSTOMDRAW)pNMHDR;
        CMenuBar* pMenubar = reinterpret_cast<CMenuBar*>
                             (::SendMessage(pNMHDR->hwndFrom, UWM_GETCMENUBAR, 0, 0));
        assert(pMenubar != 0);

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
                    CWnd* pActiveChild = pMenubar->GetActiveMDIChild();
                    assert(pActiveChild);
                    if (pActiveChild)
                    {
                        HICON icon = reinterpret_cast<HICON>(pActiveChild->SendMessage(WM_GETICON, ICON_SMALL, 0));
                        if (0 == icon)
                            icon = GetApp()->LoadStandardIcon(IDI_APPLICATION);

                        int cx = ::GetSystemMetrics(SM_CXSMICON);
                        int cy = ::GetSystemMetrics(SM_CYSMICON);
                        int y = 1 + (pMenubar->GetWindowRect().Height() - cy) / 2;
                        int x = (rc.Width() - cx) / 2;
                        drawDC.DrawIconEx(x, y, icon, cx, cy, 0, 0, DI_NORMAL);
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
                        if ((state & CDIS_SELECTED) || (pMenubar->GetButtonState(item) & TBSTATE_PRESSED))
                        {
                            drawDC.GradientFill(GetMenuBarTheme().clrPressed1, GetMenuBarTheme().clrPressed2, rc, FALSE);
                        }
                        else if (state & CDIS_HOT)
                        {
                            drawDC.GradientFill(GetMenuBarTheme().clrHot1, GetMenuBarTheme().clrHot2, rc, FALSE);
                        }

                        // Draw border.
                        CPen pen(PS_SOLID, 1, GetMenuBarTheme().clrOutline);
                        CPen oldPen = drawDC.SelectObject(pen);
                        drawDC.MoveTo(rc.left, rc.bottom);
                        drawDC.LineTo(rc.left, rc.top);
                        drawDC.LineTo(rc.right-1, rc.top);
                        drawDC.LineTo(rc.right-1, rc.bottom);
                        drawDC.MoveTo(rc.right-1, rc.bottom);
                        drawDC.LineTo(rc.left, rc.bottom);
                        drawDC.SelectObject(oldPen);
                    }

                    int itemID = static_cast<int>(lpNMCustomDraw->nmcd.dwItemSpec);
                    CString str = pMenubar->GetButtonText(itemID);

                    // Draw highlight text.
                    CFont font = pMenubar->GetFont();
                    CFont oldFont = drawDC.SelectObject(font);

                    rc.bottom += 1;
                    drawDC.SetBkMode(TRANSPARENT);
                    drawDC.SetTextColor(GetMenuBarTheme().clrText);
                    UINT format = DT_VCENTER | DT_CENTER | DT_SINGLELINE | DT_HIDEPREFIX;

                    // Turn off 'hide prefix' style for keyboard navigation.
                    if (m_altKeyPressed || pMenubar->IsAltMode())
                        format &= ~DT_HIDEPREFIX;

                    drawDC.DrawText(str, str.GetLength(), rc, format);
                    drawDC.SelectObject(oldFont);

                    return CDRF_SKIPDEFAULT;  // No further drawing
                }
            }
            return CDRF_DODEFAULT;   // Do default drawing

        // Painting cycle has completed.
        case CDDS_POSTPAINT:
            // Draw MDI Minimize, Restore and Close buttons.
            {
                CDC dc(lpNMCustomDraw->nmcd.hdc);
                pMenubar->DrawAllMDIButtons(dc);
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

                        bool isWin95 = (1400 == (GetWinVersion()) || (2400 == GetWinVersion()));

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

                            // Manually draw the dropdown arrow.
                            drawDC.CreatePen(PS_SOLID, 1, RGB(0,0,0));
                            for (int i = 2; i >= 0; --i)
                            {
                                drawDC.MoveTo(xAPos -i-1, yAPos - i+1);
                                drawDC.LineTo(xAPos +i,   yAPos - i+1);
                            }

                            // Draw line between icon and dropdown arrow.
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
                    return CDRF_SKIPDEFAULT;  // No further drawing.
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

        if (IsUsingVistaMenu())  // Is uxtheme.dll loaded?
        {
            DrawVistaMenuBkgnd(pDIS);

            if (!(pmid->mii.fType & MFT_SEPARATOR))
            {
                if (pDIS->itemState & ODS_CHECKED)
                    DrawVistaMenuCheckmark(pDIS);

                DrawVistaMenuText(pDIS);
            }
        }
        else
        {
            // Draw the gutter.
            CRect gutter = GetMenuMetrics().GetGutterRect(pDIS->rcItem);
            CDC drawDC(pDIS->hDC);
            const MenuTheme& mbt = GetMenuBarTheme();
            drawDC.GradientFill(mbt.clrPressed1, mbt.clrPressed2, gutter, TRUE);

            if (pmid->mii.fType & MFT_SEPARATOR)
            {
                // Draw the separator.
                CRect rc = pDIS->rcItem;
                CRect sepRect = pDIS->rcItem;
                sepRect.left = GetMenuMetrics().GetGutterRect(rc).Width();

                drawDC.SolidFill(RGB(255, 255, 255), sepRect);
                sepRect.top += (rc.bottom - rc.top) / 2;
                drawDC.DrawEdge(sepRect, EDGE_ETCHED, BF_TOP);
            }
            else
            {
                DrawMenuItemBkgnd(pDIS);
                DrawMenuItemText(pDIS);

                if (pDIS->itemState & ODS_CHECKED)
                    DrawMenuItemCheckmark(pDIS);
            }
        }

        if (!(pmid->mii.fType & MFT_SEPARATOR))
        {
            if (!(pDIS->itemState & ODS_CHECKED))
                DrawMenuItemIcon(pDIS);
        }
    }

    // Draws the popup menu background if uxtheme.dll is not loaded.
    template <class T>
    inline void CFrameT<T>::DrawMenuItemBkgnd(LPDRAWITEMSTRUCT pDIS)
    {
        // Draw the item background
        bool isDisabled = (pDIS->itemState & ODS_GRAYED) != FALSE;
        bool isSelected = (pDIS->itemState & ODS_SELECTED) != FALSE;
        CRect drawRect = pDIS->rcItem;
        CDC drawDC(pDIS->hDC);
        const MenuTheme& mbt = GetMenuBarTheme();

        if ((isSelected) && (!isDisabled))
        {
            // draw selected item background.
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
            // draw non-selected item background.
            drawRect.left = GetMenuMetrics().GetGutterRect(pDIS->rcItem).Width();
            drawDC.SolidFill(RGB(255,255,255), drawRect);
        }
    }

    // Draws the popup menu checkmark or radiocheck if uxtheme.dll is not loaded.
    template <class T>
    inline void CFrameT<T>::DrawMenuItemCheckmark(LPDRAWITEMSTRUCT pDIS)
    {
        CRect rc = pDIS->rcItem;
        MenuItemData* pmid = reinterpret_cast<MenuItemData*>(pDIS->itemData);
        UINT fType = pmid->mii.fType;
        const MenuTheme& mbt = GetMenuBarTheme();
        CRect bkRect;
        CDC drawDC(pDIS->hDC);

        // Draw the checkmark's background rectangle first.
        int xIcon = GetMenuMetrics().m_sizeCheck.cx;
        int yIcon = GetMenuMetrics().m_sizeCheck.cy;
        int left = GetMenuMetrics().m_marCheck.cxLeftWidth;
        int top = rc.top + (rc.Height() - yIcon) / 2;
        bkRect.SetRect(left, top, left + xIcon, top + yIcon);

        // Draw the checkmark's background rectangle.
        drawDC.CreatePen(PS_SOLID, 1, mbt.clrOutline);
        drawDC.CreateSolidBrush(mbt.clrHot2);
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
            bool isDisabled = (pDIS->itemState & ODS_GRAYED) != FALSE;
            if ((isDisabled) && (m_menuDisabledImages.GetHandle()))
                m_menuDisabledImages.Draw(pDIS->hDC, image, CPoint(rc.left, rc.top), ILD_TRANSPARENT);
            else
                m_menuImages.Draw(pDIS->hDC, image, CPoint(rc.left, rc.top), ILD_TRANSPARENT);
        }
    }

    // Draws the popup menu text if uxtheme.dll is not loaded.
    template <class T>
    inline void CFrameT<T>::DrawMenuItemText(LPDRAWITEMSTRUCT pDIS)
    {
        MenuItemData* pmid = reinterpret_cast<MenuItemData*>(pDIS->itemData);
        CString itemText = pmid->GetItemText();
        bool isDisabled = (pDIS->itemState & ODS_GRAYED) != FALSE;
        COLORREF colorText = GetSysColor(isDisabled ?  COLOR_GRAYTEXT : COLOR_MENUTEXT);

        // Calculate the text rect size.
        CRect textRect = GetMenuMetrics().GetTextRect(pDIS->rcItem);

        // find the position of tab character.
        int tab = itemText.Find(_T('\t'));

        // Draw the item text.
        SetTextColor(pDIS->hDC, colorText);
        int mode = SetBkMode(pDIS->hDC, TRANSPARENT);

        UINT format = DT_VCENTER | DT_LEFT | DT_SINGLELINE;
        // Turn on 'hide prefix' style for mouse navigation.
        CMenuBar* pMenubar = reinterpret_cast<CMenuBar*>
                             (::SendMessage(pDIS->hwndItem, UWM_GETCMENUBAR, 0, 0));
        if (pMenubar != 0)
        {
            if (!m_altKeyPressed && !pMenubar->IsAltMode())
                format |= DT_HIDEPREFIX;
        }

        DrawText(pDIS->hDC, itemText, tab, textRect, format);

        // Draw text after tab, right aligned.
        if (tab != -1)
        {
            DrawText(pDIS->hDC, itemText.Mid(tab + 1), -1, textRect, DT_SINGLELINE | DT_RIGHT | DT_VCENTER);
        }

        SetBkMode(pDIS->hDC, mode);
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

            bool isVertical = (rebar.GetStyle() & CCS_VERT) != FALSE;

            // Create our memory DC.
            CRect rebarRect = rebar.GetClientRect();
            int rebarWidth = rebarRect.Width();
            int rebarHeight = rebarRect.Height();
            CMemDC memDC(dc);
            memDC.CreateCompatibleBitmap(dc, rebarWidth, rebarRect.Height());

            // Draw to ReBar background to the memory DC.
            memDC.SolidFill(rt.clrBkgnd2, rebarRect);
            CRect rcBkGnd = rebarRect;
            rcBkGnd.right = 600;
            memDC.GradientFill(rt.clrBkgnd1, rt.clrBkgnd2, rebarRect, TRUE);

            if (rt.clrBand1 || rt.clrBand2)
            {
                // Draw the individual band backgrounds.
                for (int band = 0 ; band < rebar.GetBandCount(); ++band)
                {
                    if (rebar.IsBandVisible(band))
                    {
                        if (band != rebar.GetBand(GetMenuBar()))
                        {
                            // Determine the size of this band.
                            CRect bandRect = rebar.GetBandRect(band);

                            if (isVertical)
                            {
                                int right = bandRect.right;
                                bandRect.right = bandRect.bottom;
                                bandRect.bottom = right;
                            }

                            // Determine the size of the child window.
                            REBARBANDINFO rbbi;
                            ZeroMemory(&rbbi, sizeof(rbbi));
                            rbbi.fMask = RBBIM_CHILD ;
                            rebar.GetBandInfo(band, rbbi);
                            CRect childRect;
                            VERIFY(::GetWindowRect(rbbi.hwndChild, &childRect));
                            VERIFY(T::ScreenToClient(childRect));

                            // Determine our drawing rectangle.
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
                                VERIFY(::InflateRect(&drawRect, 1, 1));

                            // Fill the Source CDC with the band's background.
                            CMemDC sourceDC(dc);
                            sourceDC.CreateCompatibleBitmap(dc, rebarWidth, rebarHeight);
                            sourceDC.GradientFill(rt.clrBand1, rt.clrBand2, drawRect, isVertical);

                            // Set Curve amount for rounded edges.
                            int curve = rt.RoundBorders? 12 : 0;

                            // Create our mask for rounded edges using RoundRect.
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
                // Draw lines between bands.
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

            // Copy the Memory DC to the window's DC.
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

        // XP Themes are required to modify the statusbar's background.
        if (IsXPThemed())
        {
            const StatusBarTheme& sbTheme = GetStatusBarTheme();
            if (sbTheme.UseThemes)
            {
                // Fill the background with a color gradient.
                dc.GradientFill(sbTheme.clrBkgnd1, sbTheme.clrBkgnd2, statusBar.GetClientRect(), TRUE);
                isDrawn = TRUE;
            }
        }

        return isDrawn;
    }

    // Draws the popup menu background if uxtheme.dll is loaded.
    template <class T>
    inline void CFrameT<T>::DrawVistaMenuBkgnd(LPDRAWITEMSTRUCT pDIS) const
    {
        int stateID = GetMenuMetrics().ToItemStateId(pDIS->itemState);

        if (GetMenuMetrics().IsThemeBackgroundPartiallyTransparent(MENU_POPUPITEM, stateID))
        {
            GetMenuMetrics().DrawThemeBackground(pDIS->hDC, MENU_POPUPBACKGROUND, 0, &pDIS->rcItem, NULL);
        }

        // Draw the gutter.
        CRect gutter = GetMenuMetrics().GetGutterRect(pDIS->rcItem);
        if (GetMenuMetrics().IsThemeBackgroundPartiallyTransparent(MENU_POPUPITEM, stateID))
        {
            GetMenuMetrics().DrawThemeBackground(pDIS->hDC, MENU_POPUPGUTTER, 0, &gutter, NULL);
        }

        MenuItemData* pmid = reinterpret_cast<MenuItemData*>(pDIS->itemData);
        if (pmid->mii.fType & MFT_SEPARATOR)
        {
            // Draw the separator.
            CRect sepRect = GetMenuMetrics().GetSeperatorRect(pDIS->rcItem);
            GetMenuMetrics().DrawThemeBackground(pDIS->hDC, MENU_POPUPSEPARATOR, 0, &sepRect, NULL);
        }

        CRect selRect = GetMenuMetrics().GetSelectionRect(pDIS->rcItem);
        GetMenuMetrics().DrawThemeBackground(pDIS->hDC, MENU_POPUPITEM, stateID, &selRect, NULL);
    }

    // Draws the popup menu checkmark if uxtheme.dll is loaded.
    template <class T>
    inline void CFrameT<T>::DrawVistaMenuCheckmark(LPDRAWITEMSTRUCT pDIS) const
    {
        MenuItemData* pmid = reinterpret_cast<MenuItemData*>(pDIS->itemData);

        // Draw the checkmark background.
        int stateID = GetMenuMetrics().ToItemStateId(pDIS->itemState);
        CRect rcCheckBackground = GetMenuMetrics().GetCheckBackgroundRect(pDIS->rcItem);
        int backgroundStateID = GetMenuMetrics().ToCheckBackgroundStateId(stateID);
        GetMenuMetrics().DrawThemeBackground(pDIS->hDC, MENU_POPUPCHECKBACKGROUND, backgroundStateID, &rcCheckBackground, NULL);

        // Draw the checkmark.
        CRect rcCheck = GetMenuMetrics().GetCheckRect(pDIS->rcItem);
        int checkStateID = GetMenuMetrics().ToCheckStateId(pmid->mii.fType, stateID);
        GetMenuMetrics().DrawThemeBackground(pDIS->hDC, MENU_POPUPCHECK, checkStateID, &rcCheck, NULL);
    }

    // Draws the popup menu text if uxtheme.dll is loaded.
    template <class T>
    inline void CFrameT<T>::DrawVistaMenuText(LPDRAWITEMSTRUCT pDIS) const
    {
        MenuItemData* pmid = reinterpret_cast<MenuItemData*>(pDIS->itemData);

        // Calculate the text rect size.
        CStringW itemText = CStringW(TtoW(pmid->GetItemText()));
        CRect textRect = GetMenuMetrics().GetTextRect(pDIS->rcItem);

        // find the position of tab character.
        int tab = itemText.Find(L'\t');

        // Draw the item text before the tab.
        ULONG accel = ((pDIS->itemState & ODS_NOACCEL) ? DT_HIDEPREFIX : 0);
        int stateID = GetMenuMetrics().ToItemStateId(pDIS->itemState);
        DWORD flags = DT_SINGLELINE | DT_LEFT | DT_VCENTER | accel;
        GetMenuMetrics().DrawThemeText(pDIS->hDC, MENU_POPUPITEM, stateID, itemText, tab, flags, 0, &textRect);

        // Draw the item text after the tab.
        if (tab != -1)
        {
            flags = DT_SINGLELINE | DT_RIGHT | DT_VCENTER | accel;
            CStringW text = itemText.Mid(tab + 1);
            GetMenuMetrics().DrawThemeText(pDIS->hDC, MENU_POPUPITEM, stateID, text, -1, flags, 0, &textRect);
        }
    }

    // Calculates the remaining client rect when a child window is excluded.
    // Note: Assumes the child window touches 3 of the client rect's borders.
    //  e.g.   CRect rc = ExcludeChildRect(GetClientRect(), GetStatusBar());
    template <class T>
    inline CRect CFrameT<T>::ExcludeChildRect(const CRect& clientRect, HWND child) const
    {
        CRect clientRC = clientRect;
        VERIFY(T::ClientToScreen(clientRC));

        CRect childRect;
        VERIFY(::GetWindowRect(child, &childRect));

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

        VERIFY(T::ScreenToClient(clientRC));

        return clientRC;
    }

    // Returns the position of the menu item, given it's name.
    template <class T>
    inline int CFrameT<T>::GetMenuItemPos(HMENU menu, LPCTSTR itemName) const
    {
        int menuItemCount = ::GetMenuItemCount(menu);
        MENUITEMINFO mii;
        ZeroMemory(&mii, sizeof(mii));
        mii.cbSize = GetSizeofMenuItemInfo();

        for (int item = 0 ; item < menuItemCount; ++item)
        {
            std::vector<TCHAR> menuString(WXX_MAX_STRING_SIZE +1, _T('\0') );
            TCHAR* menuName = &menuString[0];

            std::vector<TCHAR> strippedString(WXX_MAX_STRING_SIZE +1, _T('\0') );
            TCHAR* pStrippedString = &strippedString.front();

            mii.fMask      = MIIM_TYPE;
            mii.fType      = MFT_STRING;
            mii.dwTypeData = menuName;
            mii.cch        = WXX_MAX_STRING_SIZE;

            // Fill the contents of szStr from the menu item.
            if (::GetMenuItemInfo(menu, item, TRUE, &mii))
            {
                int len = lstrlen(menuName);
                if (len <= WXX_MAX_STRING_SIZE)
                {
                    // Strip out any & characters.
                    int j = 0;
                    for (int i = 0; i < len; ++i)
                    {
                        if (menuName[i] != _T('&'))
                            pStrippedString[j++] = menuName[i];
                    }
                    pStrippedString[j] = _T('\0');   // Append null tchar.

                    // Compare the strings.
                    if (lstrcmp(pStrippedString, itemName) == 0)
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

            // Now put the selected entry at index 0.
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

    // Returns the XP theme name.
    template <class T>
    inline CString CFrameT<T>::GetXPThemeName() const
    {
        HMODULE theme = ::LoadLibrary(_T("uxtheme.dll"));
        WCHAR themeName[31] = L"";
        if (theme != 0)
        {
            typedef HRESULT(__stdcall* PFNGETCURRENTTHEMENAME)(LPWSTR pThemeFileName, int maxNameChars,
                LPWSTR pColorBuff, int maxColorChars, LPWSTR pSizeBuff, int maxSizeChars);

            PFNGETCURRENTTHEMENAME pfn = (PFNGETCURRENTTHEMENAME)GetProcAddress(theme, "GetCurrentThemeName");
            pfn(0, 0, themeName, 30, 0, 0);

            ::FreeLibrary(theme);
        }

        return CString(themeName);
    }

    // Returns a reference to the view window.
    template <class T>
    inline CWnd& CFrameT<T>::GetView() const
    {
        // Note: Use SetView to set the view window.
        assert(m_pView);
        return *m_pView;
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

    // Load the MRU list from the registry.
    template <class T>
    inline BOOL CFrameT<T>::LoadRegistryMRUSettings(UINT maxMRU /*= 0*/)
    {
        assert(!m_keyName.IsEmpty()); // KeyName must be set before calling LoadRegistryMRUSettings.

        CRegKey recentKey;
        BOOL loaded = FALSE;
        SetMRULimit(maxMRU);
        std::vector<CString> mruEntries;
        const CString recentKeyName = _T("Software\\") + m_keyName + _T("\\Recent Files");

        if (ERROR_SUCCESS == recentKey.Open(HKEY_CURRENT_USER, recentKeyName, KEY_READ))
        {
            CString pathName;
            CString fileKeyName;
            for (UINT i = 0; i < m_maxMRU; ++i)
            {
                DWORD bufferSize = 0;
                fileKeyName.Format(_T("File %d"), i+1);

                if (ERROR_SUCCESS == recentKey.QueryStringValue(fileKeyName, NULL, &bufferSize))
                {
                    // load the entry from the registry.
                    if (ERROR_SUCCESS == recentKey.QueryStringValue(fileKeyName, pathName.GetBuffer(bufferSize), &bufferSize))
                    {
                        pathName.ReleaseBuffer();

                        if (pathName.GetLength() > 0)
                            mruEntries.push_back( pathName );
                    }
                    else
                    {
                        pathName.ReleaseBuffer();
                        TRACE(_T("LoadRegistryMRUSettings: QueryStringValue failed\n"));
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
    inline BOOL CFrameT<T>::LoadRegistrySettings(LPCTSTR keyName)
    {
        assert (NULL != keyName);

        m_keyName = keyName;
        const CString settingsKeyName = _T("Software\\") + m_keyName + _T("\\Frame Settings");
        BOOL isOK = FALSE;
        InitValues values;
        CRegKey key;
        if (ERROR_SUCCESS == key.Open(HKEY_CURRENT_USER, settingsKeyName, KEY_READ))
        {
            try
            {
                DWORD top, left, width, height, showCmd, statusBar, toolBar;

                if (ERROR_SUCCESS != key.QueryDWORDValue(_T("Top"), top))
                    throw CUserException();
                if (ERROR_SUCCESS != key.QueryDWORDValue(_T("Left"), left))
                    throw CUserException();
                if (ERROR_SUCCESS != key.QueryDWORDValue(_T("Width"), width))
                    throw CUserException();
                if (ERROR_SUCCESS != key.QueryDWORDValue(_T("Height"), height))
                    throw CUserException();
                if (ERROR_SUCCESS != key.QueryDWORDValue(_T("ShowCmd"), showCmd))
                    throw CUserException();
                if (ERROR_SUCCESS != key.QueryDWORDValue(_T("StatusBar"), statusBar))
                    throw CUserException();
                if (ERROR_SUCCESS != key.QueryDWORDValue(_T("ToolBar"), toolBar))
                    throw CUserException();

                values.position = CRect(left, top, left + width, top + height);
                values.showCmd = (SW_MAXIMIZE == showCmd) ? SW_MAXIMIZE : SW_SHOW;
                values.showStatusBar = statusBar & 1;
                values.showToolBar = toolBar & 1;

                isOK = TRUE;
            }

            catch (const CUserException&)
            {
                TRACE("*** Failed to load values from registry, using defaults. ***\n");

                // Delete the bad key from the registry.
                const CString appKeyName = _T("Software\\") + m_keyName;
                CRegKey appKey;
                if (ERROR_SUCCESS == appKey.Open(HKEY_CURRENT_USER, appKeyName, KEY_READ))
                    appKey.DeleteSubKey(_T("Frame Settings"));

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

    // Called when the frame is activated (WM_ACTIVATE received).
    template <class T>
    inline LRESULT CFrameT<T>::OnActivate(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        // Perform default processing first
        CWnd::WndProcDefault(msg, wparam, lparam);

        if (LOWORD(wparam) == WA_INACTIVE)
        {
            // Save the hwnd of the window which currently has focus.
            // This must be CFrame window itself or a child window.
            if (!T::IsIconic()) m_oldFocus = ::GetFocus();

            // Ensure no toolbar button is still hot.
            if (GetToolBar().IsWindow())
                GetToolBar().SendMessage(TB_SETHOTITEM, (WPARAM)-1, 0);
        }
        else
        {
            // Now set the focus to the appropriate child window.
            if (m_oldFocus != 0) ::SetFocus(m_oldFocus);
        }

        // Update DockClient captions.
        T::PostMessage(UWM_DOCKACTIVATE);

        // Also update DockClient captions if the view is a docker.
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
    inline int CFrameT<T>::OnCreate(CREATESTRUCT&)
    {
        // Start the keyboard hook to capture the CapsLock, NumLock,
        // ScrollLock and Insert keys.
        SetKbdHook();

        // Set the icon.
        // Note: The MinGW compiler requires the T:: scope specifier here.
        //       It would not be required in a class that inherits from CFrameT.
        T::SetIconLarge(IDW_MAIN);
        T::SetIconSmall(IDW_MAIN);

        // Set the keyboard accelerators.
        SetAccelerators(IDW_MAIN);

        // Set the Caption.
        SetTitle(LoadString(IDW_MAIN));

        // Set the theme for the frame elements.
        SetTheme();

        // Create the rebar and menubar
        if (IsReBarSupported() && IsUsingReBar())
        {
            // Create the rebar.
            GetReBar().Create(*this);

            // Create the menubar inside rebar.
            GetMenuBar().Create(GetReBar());
            AddMenuBarBand();

            // Disable XP themes for the menubar.
            GetMenuBar().SetWindowTheme(L" ", L" ");
        }

        // Setup the menu
        CMenu menu(IDW_MAIN);
        if (::IsMenu(menu))
        {
            SetFrameMenu(menu);
            if (m_maxMRU > 0)
                UpdateMRUMenu();
        }
        else
            SetFrameMenu(0);  // No menu if IDW_MAIN menu resource isn't defined.

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

        // Create the status bar.
        if (IsUsingStatusBar())
        {
            GetStatusBar().Create(*this);
            GetStatusBar().SetFont(m_statusBarFont, FALSE);
            ShowStatusBar(GetInitValues().showStatusBar);
            CString status = LoadString(IDW_READY);
            if (!status.IsEmpty())     // Is the IDW_READY string resource defined?
                SetStatusText(status);
        }
        else
        {
            if (IsMenu(GetFrameMenu()))
                GetFrameMenu().EnableMenuItem(IDW_VIEW_STATUSBAR, MF_GRAYED);
        }

        SetStatusParts();
        SetStatusIndicators();

        // Create the view window.
        assert(&GetView());         // Use SetView in CMainFrame's constructor to set the view window.

        if (!GetView().IsWindow())
            GetView().Create(*this);
        GetView().SetFocus();

        // Adjust fonts to match the desktop theme.
        T::SendMessage(WM_SYSCOLORCHANGE);

        // Set the frame as the main window for this thread.
        GetApp()->SetMainWnd(*this);

        // Reposition the child windows.
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
        GetView().Destroy();
        GetMenuBar().Destroy();
        GetToolBar().Destroy();
        GetReBar().Destroy();
        GetStatusBar().Destroy();

        ::PostQuitMessage(0);   // Terminates the application.
    }

    // OwnerDraw is used to render the popup menu items.
    template <class T>
    inline LRESULT CFrameT<T>::OnDrawItem(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        LPDRAWITEMSTRUCT pdis = (LPDRAWITEMSTRUCT)lparam;
        assert(pdis);

        if (pdis && IsMenu(reinterpret_cast<HMENU>(pdis->hwndItem)) && (!IsRectEmpty(&pdis->rcItem)))
        {
            DrawMenuItem(pdis);
            return TRUE;
        }

        return CWnd::WndProcDefault(msg, wparam, lparam);
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
    inline LRESULT CFrameT<T>::OnInitMenuPopup(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        CMenu menu(reinterpret_cast<HMENU>(wparam));

        // The system menu shouldn't be owner drawn.
        if (HIWORD(lparam) || (T::GetSystemMenu() == menu))
            return CWnd::WndProcDefault(msg, wparam, lparam);

        // Not supported on Win95 or WinNT.
        if ((GetWinVersion() == 1400) || (GetWinVersion() == 2400))
            return CWnd::WndProcDefault(msg, wparam, lparam);

        for (UINT i = 0; i < menu.GetMenuItemCount(); ++i)
        {
            MenuItemData* pItem = new MenuItemData;        // Deleted in OnExitMenuLoop.
            m_menuItemData.push_back(ItemDataPtr(pItem));  // Store pItem in smart pointer for later automatic deletion.

            MENUITEMINFO mii;
            ZeroMemory(&mii, sizeof(mii));
            mii.cbSize = GetSizeofMenuItemInfo();

            // Use old fashioned MIIM_TYPE instead of MIIM_FTYPE for MS VC6 compatibility.
            mii.fMask = MIIM_STATE | MIIM_ID | MIIM_SUBMENU |MIIM_CHECKMARKS | MIIM_TYPE | MIIM_DATA;
            mii.dwTypeData = pItem->GetItemText();  // Assign TCHAR pointer, text is assigned by GetMenuItemInfo.
            mii.cch = WXX_MAX_STRING_SIZE;

            // Send message for menu updates.
            UINT menuItem = menu.GetMenuItemID(i);
            T::SendMessage(UWM_UPDATECOMMAND, menuItem, 0);

            // Specify owner-draw for the menu item type.
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

    // Called by the keyboard hook procedure whenever a key is pressed.
    template <class T>
    inline void CFrameT<T>::OnKeyboardHook(int code, WPARAM wparam, LPARAM lparam)
    {
        // Detect the state of the toggle keys.
        if (HC_ACTION == code)
        {
            if ((wparam == VK_CAPITAL) || (wparam == VK_NUMLOCK) ||
                (wparam == VK_SCROLL) || (wparam == VK_INSERT))
            {
                SetStatusIndicators();
            }
        }

        // Detect the press state of the alt key.
        if (wparam == VK_MENU)
        {
            BOOL keyState = lparam & 0x80000000;
            if (!m_altKeyPressed && !keyState)
            {
                m_altKeyPressed = TRUE;
                if (GetMenuBar().IsWindow())
                    GetMenuBar().RedrawWindow();
            }

            if (m_altKeyPressed && keyState)
            {
                m_altKeyPressed = FALSE;
                if (GetMenuBar().IsWindow())
                    GetMenuBar().RedrawWindow();
            }
        }
    }

    // Called before the Popup menu is displayed, so that the MEASUREITEMSTRUCT
    // values can be assigned with the menu item's dimensions.
    template <class T>
    inline LRESULT CFrameT<T>::OnMeasureItem(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        LPMEASUREITEMSTRUCT pmis = (LPMEASUREITEMSTRUCT) lparam;
        if (pmis->CtlType != ODT_MENU)
            return CWnd::WndProcDefault(msg, wparam, lparam);

        MeasureMenuItem(pmis);
        return TRUE;
    }

    // Called when a menu is active, and a key is pressed other than an accelerator.
    template <class T>
    inline LRESULT CFrameT<T>::OnMenuChar(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        if ((GetMenuBar().IsWindow()) && (LOWORD(wparam)!= VK_SPACE))
        {
            // Activate MenuBar for key pressed with Alt key held down
            GetMenuBar().OnMenuChar(msg, wparam, lparam);
            return -1;
        }

        return CWnd::WndProcDefault(msg, wparam, lparam);
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
            HMENU menu = reinterpret_cast<HMENU>(lparam);

            if ((menu != T::GetMenu()) && (id != 0) && !(HIWORD(wparam) & MF_POPUP))

                GetStatusBar().SetPartText(0, LoadString(id));
            else
                GetStatusBar().SetPartText(0, m_statusText);
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
                bool isVisible = GetStatusBar().IsWindow() && GetStatusBar().IsWindowVisible();
                GetFrameMenu().CheckMenuItem(id, isVisible ? MF_CHECKED : MF_UNCHECKED);
            }
            break;
        case IDW_VIEW_TOOLBAR:
            {
                bool isVisible = GetToolBar().IsWindow() && GetToolBar().IsWindowVisible();
                GetFrameMenu().EnableMenuItem(id, GetToolBar().IsWindow() ? MF_ENABLED : MF_DISABLED);
                GetFrameMenu().CheckMenuItem(id, isVisible ? MF_CHECKED : MF_UNCHECKED);
            }
            break;
        }
    }

    // Called when a notification from a child window (WM_NOTIFY) is received.
    template <class T>
    inline LRESULT CFrameT<T>::OnNotify(WPARAM, LPARAM lparam)
    {
        LPNMHDR pHeader = reinterpret_cast<LPNMHDR>(lparam);
        switch (pHeader->code)
        {
        case (UINT)NM_CUSTOMDRAW: return OnCustomDraw(pHeader);   // The UINT cast is required by some 32bit MinGW compilers.
        case RBN_HEIGHTCHANGE:    return OnRBNHeightChange(pHeader);
        case RBN_LAYOUTCHANGED:   return OnRBNLayoutChanged(pHeader);
        case RBN_MINMAX:          return OnRBNMinMax(pHeader);
        case TBN_DROPDOWN:        return OnTBNDropDown((LPNMTOOLBAR)lparam);
        case TTN_GETDISPINFO:     return OnTTNGetDispInfo((LPNMTTDISPINFO)lparam);
        case UWN_UNDOCKED:        return OnUndocked();
        }

        return 0;
    }

    // Called when the rebar's height changes.
    template <class T>
    inline LRESULT CFrameT<T>::OnRBNHeightChange(LPNMHDR)
    {
        RecalcLayout();

        return 0;
    }

    // Notification of rebar layout change.
    template <class T>
    inline LRESULT CFrameT<T>::OnRBNLayoutChanged(LPNMHDR)
    {
        if (GetReBarTheme().UseThemes && GetReBarTheme().BandsLeft)
            GetReBar().MoveBandsLeft();

        return 0;
    }

    // Notification of a rebar band minimized or maximized.
    template <class T>
    inline LRESULT CFrameT<T>::OnRBNMinMax(LPNMHDR)
    {
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
    inline LRESULT CFrameT<T>::OnSetFocus(UINT, WPARAM, LPARAM)
    {
        SetStatusIndicators();
        return 0;
    }

    // Called when the SystemParametersInfo function changes a system-wide
    // setting or when policy settings have changed. Also called in response
    // to a WM_DPICHANGED message.
    template <class T>
    inline LRESULT CFrameT<T>::OnSettingChange(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        OnSysColorChange(msg, wparam, lparam);
        return 0;
    }

    // Called when the frame window is resized.
    template <class T>
    inline LRESULT CFrameT<T>::OnSize(UINT, WPARAM, LPARAM)
    {
        RecalcLayout();
        return 0;
    }

    // Called in response to a WM_SYSCOLORCHANGE message. This message is sent
    // to all top-level windows when a change is made to a system color setting.
    template <class T>
    inline LRESULT CFrameT<T>::OnSysColorChange(UINT msg, WPARAM, LPARAM)
    {
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
            m_statusBarFont.CreateFontIndirect(info.lfStatusFont);
            GetStatusBar().SetFont(m_statusBarFont, TRUE);
            GetStatusBar().Invalidate();
        }

        if (GetMenuBar().IsWindow())
        {
            // Update the MenuBar font and button size.
            m_menuBarFont.CreateFontIndirect(info.lfMenuFont);
            GetMenuBar().SetFont(m_menuBarFont, TRUE);
            GetMenuBar().SetMenu( GetFrameMenu() );

            // Update the MenuBar band size.
            UpdateMenuBarBandSize();
        }

        SetTheme();

        // Reposition and redraw everything.
        RecalcLayout();
        T::RedrawWindow(RDW_ERASE | RDW_FRAME | RDW_INVALIDATE | RDW_ALLCHILDREN);

        // Forward the message to the view window.
        if (GetView().IsWindow())
            GetView().PostMessage(msg, 0, 0);

        return 0;
    }

    // Called in response to a WM_SYSCOMMAND notification. This notification
    // is passed on to the MenuBar to process alt keys and maximize or restore.
    template <class T>
    inline LRESULT CFrameT<T>::OnSysCommand(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        if ((SC_KEYMENU == wparam) && (VK_SPACE != lparam) && GetMenuBar().IsWindow())
        {
            GetMenuBar().OnSysCommand(msg, wparam, lparam);
            return 0;
        }

        if (SC_MINIMIZE == wparam)
            m_oldFocus = ::GetFocus();

        // Pass remaining system commands on for default processing.
        return CWnd::WndProcDefault(msg, wparam, lparam);
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
    inline LRESULT CFrameT<T>::OnUnInitMenuPopup(UINT, WPARAM, LPARAM)
    {
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
            VERIFY(::SetMenuItemInfo(m_menuItemData[item]->menu, m_menuItemData[item]->pos, TRUE, &mii));
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
        BOOL show = !(GetToolBar().IsWindow() && GetToolBar().IsWindowVisible());
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
            // Set the original window position.
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
            VERIFY(GetStatusBar().SetWindowPos(0, 0, 0, 0, 0, SWP_SHOWWINDOW));

            SetStatusParts();
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

        // Position the view window.
        if (GetView().IsWindow())
            RecalcViewLayout();

        // Adjust rebar bands.
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
        VERIFY(GetView().SetWindowPos(0, GetViewRect(), SWP_SHOWWINDOW));
    }

    // Removes an entry from the MRU list.
    template <class T>
    inline void CFrameT<T>::RemoveMRUEntry(LPCTSTR MRUEntry)
    {
        std::vector<CString>::iterator it;
        for (it = m_mruEntries.begin(); it != m_mruEntries.end(); ++it)
        {
            if ((*it) == MRUEntry)
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
        // Store the MRU entries in the registry.
        try
        {
            // Delete Old MRUs
            const CString appKeyName = _T("Software\\") + m_keyName;
            CRegKey appKey;
            appKey.Open(HKEY_CURRENT_USER, appKeyName);
            appKey.DeleteSubKey(_T("Recent Files"));

            if (m_maxMRU > 0)
            {
                const CString recentKeyName = _T("Software\\") + m_keyName + _T("\\Recent Files");
                CRegKey recentKey;

                // Add Current MRUs.
                if (ERROR_SUCCESS != recentKey.Create(HKEY_CURRENT_USER, recentKeyName))
                    throw CUserException();

                if (ERROR_SUCCESS != recentKey.Open(HKEY_CURRENT_USER, recentKeyName))
                    throw CUserException();

                CString subKeyName;
                CString pathName;
                for (UINT i = 0; i < m_maxMRU; ++i)
                {
                    subKeyName.Format(_T("File %d"), i + 1);

                    if (i < m_mruEntries.size())
                    {
                        pathName = m_mruEntries[i];

                        if (ERROR_SUCCESS != recentKey.SetStringValue(subKeyName, pathName.c_str()))
                            throw CUserException();
                    }
                }
            }
        }

        catch (const CUserException&)
        {
            TRACE("*** Failed to save registry MRU settings. ***\n");

            const CString appKeyName = _T("Software\\") + m_keyName;
            CRegKey appKey;

            if (ERROR_SUCCESS == appKey.Open(HKEY_CURRENT_USER, appKeyName))
            {
                // Roll back the registry changes by deleting this subkey.
                appKey.DeleteSubKey(_T("Recent Files"));
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
                const CString settingsKeyName = _T("Software\\") + m_keyName + _T("\\Frame Settings");
                CRegKey settingsKey;

                if (ERROR_SUCCESS != settingsKey.Create(HKEY_CURRENT_USER, settingsKeyName))
                    throw CUserException(_T("CRegKey::Create failed"));
                if (ERROR_SUCCESS != settingsKey.Open(HKEY_CURRENT_USER, settingsKeyName))
                    throw CUserException(_T("CRegKey::Open failed"));

                // Store the window position in the registry.
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

                    if (ERROR_SUCCESS != settingsKey.SetDWORDValue(_T("Top"), top))
                        throw CUserException();
                    if (ERROR_SUCCESS != settingsKey.SetDWORDValue(_T("Left"), left))
                        throw CUserException();
                    if (ERROR_SUCCESS != settingsKey.SetDWORDValue(_T("Width"), width))
                        throw CUserException();
                    if (ERROR_SUCCESS != settingsKey.SetDWORDValue(_T("Height"), height))
                        throw CUserException();
                    if (ERROR_SUCCESS != settingsKey.SetDWORDValue(_T("ShowCmd"), showCmd))
                        throw CUserException();
                }

                // Store the ToolBar and statusbar states.
                DWORD showToolBar = GetToolBar().IsWindow() && GetToolBar().IsWindowVisible();
                DWORD showStatusBar = GetStatusBar().IsWindow() && GetStatusBar().IsWindowVisible();

                if (ERROR_SUCCESS != settingsKey.SetDWORDValue(_T("ToolBar"), showToolBar))
                    throw CUserException();
                if (ERROR_SUCCESS != settingsKey.SetDWORDValue(_T("StatusBar"), showStatusBar))
                    throw CUserException();
            }

            catch (const CUserException&)
            {
                TRACE("*** Failed to save registry settings. ***\n");

                const CString appKeyName = _T("Software\\") + m_keyName;
                CRegKey appKey;

                if (ERROR_SUCCESS == appKey.Open(HKEY_CURRENT_USER, appKeyName))
                {
                    // Roll back the registry changes by deleting this subkey.
                    appKey.DeleteSubKey(_T("Frame Settings"));
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
    inline void CFrameT<T>::SetFrameMenu(int menuID)
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
            BOOL show = (menu != 0);    // boolean expression
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
        // Remove any existing menu icons.
        m_menuImages.DeleteImageList();
        m_menuDisabledImages.DeleteImageList();
        m_menuIcons.clear();

        // Exit if no ToolBarID is specified.
        if (toolBarID == 0) return 0;

        // Add the menu icons from the bitmap IDs.
        return AddMenuIcons(menuData, mask, toolBarID, toolBarDisabledID);
    }

    // Implements a keyboard hook. The hook is used to detect the CAPs lock,
    // NUM lock, Scroll lock and Insert keys.
    template <class T>
    inline void CFrameT<T>::SetKbdHook()
    {
        m_kbdHook = ::SetWindowsHookEx(WH_KEYBOARD, StaticKeyboardProc, 0, ::GetCurrentThreadId());
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

    // Displays the caps lock, mum lock, and scroll lock key states.
    template <class T>
    inline void CFrameT<T>::SetStatusIndicators()
    {
        if (GetStatusBar().IsWindow() && (IsUsingIndicatorStatus()))
        {
            CString cap = LoadString(IDW_INDICATOR_CAPS);
            CString num = LoadString(IDW_INDICATOR_NUM);
            CString scrl = LoadString(IDW_INDICATOR_SCRL);

            CString status1 = (::GetKeyState(VK_CAPITAL) & 0x0001)? cap : CString("");
            CString status2 = (::GetKeyState(VK_NUMLOCK) & 0x0001)? num : CString("");
            CString status3 = (::GetKeyState(VK_SCROLL)  & 0x0001)? scrl: CString("");

            // Update the indicators text.
            GetStatusBar().SetPartText(1, status1);
            GetStatusBar().SetPartText(2, status2);
            GetStatusBar().SetPartText(3, status3);
        }
    }

    // Creates and resizes the 4 parts in the status bar.
    template <class T>
    inline void CFrameT<T>::SetStatusParts()
    {
        if (IsUsingIndicatorStatus())
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
            int cxGripper = hasGripper ? 20 : 0;
            int cxBorder = 8;

            // Adjust for DPI aware.
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
        CClientDC DesktopDC(0);
        if (DesktopDC.GetDeviceCaps(BITSPIXEL) < 16)
            UseThemes(FALSE);

        BOOL t = TRUE;
        BOOL f = FALSE;

        if (IsUsingThemes())
        {
            // Retrieve the XP theme name.
            CString xpThemeName = GetXPThemeName();

            enum Themetype{ Win8, Win7, XP_Blue, XP_Silver, XP_Olive, gray };

            // For Win2000 and below.
            int theme = gray;

            if (GetWinVersion() < 2600) // For Windows XP.
            {
                if (xpThemeName == _T("NormalColor"))
                    theme = XP_Blue;
                if (xpThemeName == _T("Metallic"))
                    theme = XP_Silver;
                if (xpThemeName == _T("HomeStead"))
                    theme = XP_Olive;
            }
            else if (GetWinVersion() <= 2601)
            {
                // For Vista and Windows 7.
                theme = Win7;
            }
            else
            {
                // For Windows 8 and above.
                theme = Win8;
            }

            switch (theme)
            {
            case Win8:  // A pale blue scheme without gradients, suitable for Windows 8, 8.1, and 10.
                {
                    MenuTheme mt = {t, RGB(180, 250, 255), RGB(140, 190, 255), RGB(240, 250, 255), RGB(120, 170, 220), RGB(127, 127, 255), RGB(0, 0, 0) };
                    ReBarTheme rbt = {t, RGB(235, 237, 250), RGB(235, 237, 250), RGB(235, 237, 250), RGB(235, 237, 250), f, t, t, f, t, f };
                    StatusBarTheme sbt = {t, RGB(235, 237, 250), RGB(235, 237, 250)};
                    ToolBarTheme tbt = {t, RGB(180, 250, 255), RGB(140, 190, 255), RGB(150, 220, 255), RGB(80, 100, 255), RGB(127, 127, 255)};

                    SetMenuTheme(mt);   // Sets the theme for popup menus and MenuBar.
                    SetReBarTheme(rbt);
                    SetStatusBarTheme(sbt);
                    SetToolBarTheme(tbt);
                }
                break;

            case Win7:  // A pale blue color scheme suitable for Vista and Windows 7.
                {
                    MenuTheme mt = {t, RGB(180, 250, 255), RGB(140, 190, 255), RGB(240, 250, 255), RGB(120, 170, 220), RGB(127, 127, 255), RGB(0, 0, 0) };
                    ReBarTheme rbt = {t, RGB(225, 230, 255), RGB(240, 242, 250), RGB(248, 248, 248), RGB(180, 200, 230), f, t, t, t, t, f};
                    StatusBarTheme sbt = {t, RGB(225, 230, 255), RGB(240, 242, 250)};
                    ToolBarTheme tbt = {t, RGB(180, 250, 255), RGB(140, 190, 255), RGB(150, 220, 255), RGB(80, 100, 255), RGB(127, 127, 255)};

                    SetMenuTheme(mt);   // Sets the theme for popup menus and MenuBar.
                    SetReBarTheme(rbt);
                    SetStatusBarTheme(sbt);
                    SetToolBarTheme(tbt);
                }
                break;


            case XP_Blue:
                {
                    // Used for XP default (blue) color scheme.
                    MenuTheme mt = {t, RGB(255, 230, 190), RGB(255, 190, 100), RGB(220,230,250), RGB(150,190,245), RGB(128, 128, 200), RGB(0, 0, 0) };
                    ReBarTheme rbt = {t, RGB(150,190,245), RGB(196,215,250), RGB(220,230,250), RGB( 70,130,220), f, t, t, t, t, f};
                    StatusBarTheme sbt = {t, RGB(150,190,245), RGB(196,215,250)};
                    ToolBarTheme tbt = {t, RGB(255, 230, 190), RGB(255, 190, 100), RGB(255, 140, 40), RGB(255, 180, 80), RGB(192, 128, 255)};

                    SetMenuTheme(mt);   // Sets the theme for popup menus and MenuBar.
                    SetReBarTheme(rbt);
                    SetStatusBarTheme(sbt);
                    SetToolBarTheme(tbt);
                }
                break;

            case XP_Silver:
                {
                    // Used for XP Silver color scheme.
                    MenuTheme mt = {t, RGB(196, 215, 250), RGB( 120, 180, 220), RGB(240, 240, 245), RGB(170, 165, 185), RGB(128, 128, 150), RGB(0, 0, 0) };
                    ReBarTheme rbt = {t, RGB(225, 220, 240), RGB(240, 240, 245), RGB(245, 240, 255), RGB(160, 155, 180), f, t, t, t, t, f};
                    StatusBarTheme sbt = {t, RGB(225, 220, 240), RGB(240, 240, 245)};
                    ToolBarTheme tbt = {t, RGB(192, 210, 238), RGB(192, 210, 238), RGB(152, 181, 226), RGB(152, 181, 226), RGB(49, 106, 197)};

                    SetMenuTheme(mt);   // Sets the theme for popup menus and MenuBar.
                    SetReBarTheme(rbt);
                    SetStatusBarTheme(sbt);
                    SetToolBarTheme(tbt);
                }
                break;

            case XP_Olive:
                {
                    // Used for XP Olive color scheme.
                    MenuTheme mt = {t, RGB(255, 230, 190), RGB(255, 190, 100), RGB(249, 255, 227), RGB(178, 191, 145), RGB(128, 128, 128), RGB(0, 0, 0) };
                    ReBarTheme rbt = {t, RGB(215, 216, 182), RGB(242, 242, 230), RGB(249, 255, 227), RGB(178, 191, 145), f, t, t, t, t, f};
                    StatusBarTheme sbt = {t, RGB(215, 216, 182), RGB(242, 242, 230)};
                    ToolBarTheme tbt = {t, RGB(255, 230, 190), RGB(255, 190, 100), RGB(255, 140, 40), RGB(255, 180, 80), RGB(200, 128, 128)};

                    SetMenuTheme(mt);   // Sets the theme for popup menus and MenuBar.
                    SetReBarTheme(rbt);
                    SetStatusBarTheme(sbt);
                    SetToolBarTheme(tbt);
                }
                break;

            case gray:  // A color scheme suitable for 16 bit colors. Suitable for Windows older than XP.
                {
                    MenuTheme mt = {t, RGB(182, 189, 210), RGB( 182, 189, 210), RGB(200, 196, 190), RGB(200, 196, 190), RGB(100, 100, 100), RGB(0, 0, 0) };
                    ReBarTheme rbt = {t, RGB(212, 208, 200), RGB(212, 208, 200), RGB(230, 226, 222), RGB(220, 218, 208), f, t, t, t, t, f};
                    StatusBarTheme sbt = {t, RGB(212, 208, 200), RGB(212, 208, 200)};
                    ToolBarTheme tbt = {t, RGB(182, 189, 210), RGB(182, 189, 210), RGB(133, 146, 181), RGB(133, 146, 181), RGB(10, 36, 106)};

                    SetMenuTheme(mt);   // Sets the theme for popup menus and MenuBar.
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
            // The colors specified here are used for Windows 2000 and below.
            MenuTheme mt = {FALSE, RGB(182, 189, 210), RGB( 182, 189, 210), RGB(200, 196, 190), RGB(200, 196, 190), RGB(100, 100, 100), RGB(0, 0, 0)};
            SetMenuTheme(mt);  // Sets the theme for popup menus and MenuBar.
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

        // Set the toolbar's image list.
        imageList.DeleteImageList();
        imageList.Create(sz.cx, sz.cy, ILC_COLOR32 | ILC_MASK, 0, 0);
        imageList.Add(bm, mask);
        toolBar.SetImageList(imageList);

        // Inform the Rebar of the change to the Toolbar.
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

        // Inform the Rebar of the change to the Toolbar.
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

        // Inform the Rebar of the change to the Toolbar.
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
    // The color mask is ignored for 32bit bitmaps, but is required for 24bit bitmaps
    // The color mask is often gray RGB(192,192,192) or magenta (255,0,255)
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
        // Add the set of toolbar images to the menu.
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

    // Stores the tool bar's theme colors.
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
                VERIFY(GetView().SetWindowPos(0, rc, SWP_SHOWWINDOW));
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
                T::SetMenu(0);
        }

        if (GetReBar().IsWindow())
        {
            if (GetReBarTheme().UseThemes && GetReBarTheme().BandsLeft)
                GetReBar().MoveBandsLeft();
        }

        // Reposition the Windows.
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

        // Reposition the windows.
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

        // Reposition the windows.
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

        if (pFrame)
            pFrame->OnKeyboardHook(code, wparam, lparam);

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

        // Create a vector of CStrings containing the MRU menu entries.
        std::vector<CString> mruStrings;

        if (m_mruEntries.size() > 0)
        {
            for (UINT n = 0; n < m_mruEntries.size(); ++n)
            {
                CString str = m_mruEntries[n];

                // Prefix the string with its number.
                CString count;
                count.Format(_T("%d "), n + 1);
                str = count + str;

                // Trim the string if its too long.
                if (str.GetLength() > WXX_MAX_STRING_SIZE - 10)
                {
                    // Extract the first part of the string up until the first '\\'
                    CString prefix;
                    int index = str.Find(_T('\\'));
                    if (index >= 0)
                        prefix = str.Left(index + 1);

                    // Reduce the string to fit within WXX_MAX_STRING_SIZE.
                    CString gap = _T("...");
                    str.Delete(0, str.GetLength() - WXX_MAX_STRING_SIZE + prefix.GetLength() + gap.GetLength()+1);

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

        // Set MRU menu items.
        MENUITEMINFO mii;
        ZeroMemory(&mii, sizeof(mii));
        mii.cbSize = GetSizeofMenuItemInfo();

        // We place the MRU items under the left most menu item.
        CMenu fileMenu = GetFrameMenu().GetSubMenu(0);

        if (fileMenu.GetHandle())
        {
            // Remove all but the first MRU Menu entry.
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
                    // Replace the last MRU entry first.
                    result = fileMenu.SetMenuItemInfo(IDW_FILE_MRU_FILE1, mii, FALSE);
                else
                    // Insert the other MRU entries next.
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
        case WM_DPICHANGED:     return OnSettingChange(msg, wparam, lparam);
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
        case WM_WINDOWPOSCHANGED: return CWnd::WndProcDefault(msg, wparam, lparam);

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

#if defined (_MSC_VER) && (_MSC_VER >= 1920)
  #pragma warning ( pop )
#endif // (_MSC_VER) && (_MSC_VER >= 1920)

#endif // _WIN32XX_FRAME_H_
