// Win32++   Version 10.0.0
// Release Date: 9th September 2024
//
//      David Nash
//      email: dnash@bigpond.net.au
//      url: https://sourceforge.net/projects/win32-framework
//           https://github.com/DavidNash2024/Win32xx
//
//
// Copyright (c) 2005-2024  David Nash
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


#include "wxx_dialog.h"
#include "wxx_docking.h"
#include "wxx_menubar.h"
#include "wxx_menumetrics.h"
#include "wxx_rebar.h"
#include "wxx_regkey.h"
#include "wxx_statusbar.h"
#include "wxx_toolbar.h"
#include "default_resource.h"


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

            // The constructor displays the statusbar and toolbar by default.
            InitValues() : showCmd(SW_SHOW), showStatusBar(TRUE), showToolBar(TRUE)
            {}
        };

        CFrameT();
        virtual ~CFrameT() override;

        // Accessors and mutators for the menubar, rebar, statusbar, and toolbar.
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
        const CFont& GetMenuFont() const                  { return m_menuFont; }
        const MenuTheme& GetMenuBarTheme() const          { return m_mbTheme; }
        int GetMenuIconHeight() const;
        const CMenuMetrics& GetMenuMetrics() const        { return m_menuMetrics; }
        const std::vector<CString>& GetMRUEntries() const { return m_mruEntries; }
        CString GetMRUEntry(UINT index);
        size_t GetMRULimit() const                        { return m_maxMRU; }
        CString GetRegistryKeyName() const                { return m_keyName; }
        const ReBarTheme& GetReBarTheme() const           { return m_rbTheme; }
        const CFont& GetStatusBarFont() const             { return m_statusBarFont; }
        const StatusBarTheme& GetStatusBarTheme() const   { return m_sbTheme; }
        const std::vector<UINT>& GetToolBarData() const   { return m_toolBarData; }
        const ToolBarTheme& GetToolBarTheme() const       { return m_tbTheme; }
        CString GetStatusText() const                     { return m_statusText; }
        CString GetTitle() const                          { return T::GetWindowText(); }
        CWnd& GetView() const;
        CString GetXPThemeName() const;
        BOOL IsMDIFrame() const                           { return static_cast<BOOL>(T::SendMessage(UWM_GETCMDIFRAMET)); }
        void RemoveKbdHook();
        void ResetMenuMetrics()                           { m_menuMetrics.SetMetrics(*this); }
        void SetAccelerators(UINT accelID);
        void SetFrameMenu(UINT menuID);
        void SetFrameMenu(CMenu menu);
        void SetInitValues(const InitValues& values);
        void SetKbdHook();
        void SetMenuFont(HFONT font);
        void SetMenuTheme(const MenuTheme& mt);
        void SetMRULimit(UINT MRULimit);
        void SetReBarTheme(const ReBarTheme& rbt);
        void SetStatusBarFont(HFONT font);
        void SetStatusBarTheme(const StatusBarTheme& sbt);
        void SetStatusText(LPCTSTR text);
        void SetTitle(LPCTSTR text)                       { T::SetWindowText(text); }
        void SetToolBarTheme(const ToolBarTheme& tbt);
        void SetView(CWnd& view);

    protected:
        // Override these functions as required.
        virtual void AddDisabledMenuImage(HICON icon, COLORREF mask);
        virtual BOOL AddMenuIcon(UINT menuItemID, UINT iconID, UINT disabledIconID = 0);
        virtual BOOL AddMenuIcon(UINT menuItemID, HICON icon, HICON disabledIcon = nullptr);
        virtual UINT AddMenuIcons(const std::vector<UINT>& menuData, COLORREF mask, UINT bitmapID, UINT disabledID = 0);
        virtual void AddMenuBarBand();
        virtual void AddMRUEntry(LPCTSTR MRUEntry);
        virtual void AddToolBarBand(CToolBar& tb, DWORD bandStyle, UINT id);
        virtual void AddToolBarButton(UINT id, BOOL isEnabled = TRUE, LPCTSTR text = nullptr, int image = -1);
        virtual void AdjustFrameRect(const RECT& viewRect);
        virtual void ClearMenuIcons();
        virtual void CreateToolBar();
        virtual LRESULT CustomDrawMenuBar(NMHDR* pNMHDR);
        virtual LRESULT CustomDrawToolBar(NMHDR* pNMHDR);
        virtual void DrawMenuItem(LPDRAWITEMSTRUCT pDrawItem);
        virtual void DrawMenuItemBkgnd(LPDRAWITEMSTRUCT pDrawItem);
        virtual void DrawMenuItemCheckmark(LPDRAWITEMSTRUCT pDrawItem);
        virtual void DrawMenuItemIcon(LPDRAWITEMSTRUCT pDrawItem);
        virtual void DrawMenuItemText(LPDRAWITEMSTRUCT pDrawItem);
        virtual BOOL DrawReBarBkgnd(CDC& dc, CReBar& rebar);
        virtual void DrawStatusBar(LPDRAWITEMSTRUCT pDrawItem);
        virtual BOOL DrawStatusBarBkgnd(CDC& dc, CStatusBar& statusBar);
        virtual void DrawVistaMenuBkgnd(LPDRAWITEMSTRUCT pDrawItem);
        virtual void DrawVistaMenuCheckmark(LPDRAWITEMSTRUCT pDrawItem);
        virtual void DrawVistaMenuText(LPDRAWITEMSTRUCT pDrawItem);
        virtual CRect GetViewRect() const;
        virtual BOOL LoadRegistrySettings(LPCTSTR keyName);
        virtual BOOL LoadRegistryMRUSettings(UINT maxMRU = 0);
        virtual void MeasureMenuItem(MEASUREITEMSTRUCT* pMIS);
        virtual LRESULT OnActivate(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual void    OnClose() override;
        virtual int     OnCreate(CREATESTRUCT& cs) override;
        virtual LRESULT OnCustomDraw(LPNMHDR pNMHDR);
        virtual void    OnDestroy() override;
        virtual LRESULT OnDpiChanged(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual LRESULT OnDrawItem(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual LRESULT OnDrawRBBkgnd(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual LRESULT OnDrawSBBkgnd(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual BOOL    OnHelp();
        virtual LRESULT OnInitMenuPopup(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual void    OnKeyboardHook(int code, WPARAM wparam, LPARAM lparam);
        virtual LRESULT OnMeasureItem(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual LRESULT OnMenuChar(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual LRESULT OnMenuSelect(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual void    OnMenuUpdate(UINT id) override;
        virtual LRESULT OnNotify(WPARAM wparam, LPARAM lparam) override;
        virtual LRESULT OnRBNHeightChange(LPNMHDR pNMHDR);
        virtual LRESULT OnRBNLayoutChanged(LPNMHDR pNMHDR);
        virtual LRESULT OnRBNMinMax(LPNMHDR pNMHDR);
        virtual LRESULT OnSettingChange(UINT, WPARAM, LPARAM);
        virtual LRESULT OnSize(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual LRESULT OnSysColorChange(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual LRESULT OnSysCommand(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual LRESULT OnThemeChanged(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual LRESULT OnTTNGetDispInfo(LPNMTTDISPINFO pNMTDI);
        virtual LRESULT OnUndocked();
        virtual LRESULT OnUnInitMenuPopup(UINT, WPARAM wparam, LPARAM lparam);
        virtual BOOL    OnViewStatusBar();
        virtual BOOL    OnViewToolBar();
        virtual void PreCreate(CREATESTRUCT& cs) override;
        virtual void PreRegisterClass(WNDCLASS& wc) override;
        virtual void RecalcLayout();
        virtual void RecalcViewLayout();
        virtual void RemoveMRUEntry(LPCTSTR MRUEntry);
        virtual BOOL SaveRegistryMRUSettings();
        virtual BOOL SaveRegistrySettings();
        virtual void SetMenuBarBandSize();
        virtual UINT SetMenuIcons(const std::vector<UINT>& menuData, COLORREF mask, UINT toolBarID, UINT toolBarDisabledID = 0);
        virtual void SetStatusIndicators();
        virtual void SetStatusParts();
        virtual void SetTBImageList(CToolBar& toolBar, UINT id, COLORREF mask);
        virtual void SetTBImageListDis(CToolBar& toolBar, UINT id, COLORREF mask);
        virtual void SetTBImageListHot(CToolBar& toolBar, UINT id, COLORREF mask);
        virtual void SetTheme();
        virtual void SetToolBarImages(COLORREF mask, UINT toolBarID, UINT toolBarHotID = 0, UINT toolBarDisabledID = 0);
        virtual void SetToolBarImages(CToolBar& toolbar, COLORREF mask, UINT toolBarID, UINT toolBarHotID = 0, UINT toolBarDisabledID = 0);
        virtual void SetupMenuIcons();
        virtual void SetupToolBar();
        virtual void ShowMenu(BOOL show);
        virtual void ShowStatusBar(BOOL show);
        virtual void ShowToolBar(BOOL show);
        virtual void UpdateMRUMenu();
        virtual void UpdateSettings();

        // Not intended to be overridden
        CRect ExcludeChildRect(const CRect& clientRect, HWND child) const;
        BOOL IsReBarSupported() const;
        BOOL IsUsingDarkMenu() const { return m_useDarkMenu; }
        BOOL IsUsingIndicatorStatus() const { return m_useIndicatorStatus; }
        BOOL IsUsingMenuStatus() const { return m_useMenuStatus; }
        BOOL IsUsingOwnerDrawnMenu() const { return m_useOwnerDrawnMenu; }
        BOOL IsUsingReBar() const { return m_useReBar; }
        BOOL IsUsingStatusBar() const { return m_useStatusBar; }
        BOOL IsUsingThemes() const { return m_useThemes; }
        BOOL IsUsingToolBar() const { return m_useToolBar; }
        BOOL IsUsingVistaMenu() const { return m_menuMetrics.IsVistaMenu(); }
        void UseDarkMenu(BOOL useDarkMenu) { m_useDarkMenu = useDarkMenu; }
        void UseIndicatorStatus(BOOL useIndicatorStatus) { m_useIndicatorStatus = useIndicatorStatus; }
        void UseMenuStatus(BOOL useMenuStatus) { m_useMenuStatus = useMenuStatus; }
        void UseOwnerDrawnMenu(BOOL useOwnerDraw) { m_useOwnerDrawnMenu = useOwnerDraw; }
        void UseReBar(BOOL useReBar) { m_useReBar = useReBar; }
        void UseStatusBar(BOOL useStatusBar) { m_useStatusBar = useStatusBar; }
        void UseThemes(BOOL useThemes) { m_useThemes = useThemes; }
        void UseToolBar(BOOL useToolBar) { m_useToolBar = useToolBar; }

        LRESULT WndProcDefault(UINT msg, WPARAM wparam, LPARAM lparam) override;

    private:
        using MenuData = std::vector<MenuItemDataPtr>;     // all menu item data for a popup menu

        CFrameT(const CFrameT&) = delete;
        CFrameT& operator=(const CFrameT&) = delete;
        CSize GetTBImageSize(CBitmap* pBitmap) const;
        void UpdateMenuBarBandSize();
        static LRESULT CALLBACK StaticKeyboardProc(int code, WPARAM wparam, LPARAM lparam);

        std::vector<MenuData> m_menusData;  // vector of menu data for multiple popup menus
        std::vector<CString> m_mruEntries;  // vector of CStrings for MRU entries
        std::vector<UINT> m_menuItemIDs;    // vector of menu icon resource IDs
        std::vector<UINT> m_toolBarData;    // vector of resource IDs for ToolBar buttons
        std::vector<CString> m_indicators;  // vector of CStrings for status indicators
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
        CMenu m_menu;                       // The menu used by the menubar or the frame's window
        CFont m_menuFont;                   // Menu and menubar font
        CFont m_statusBarFont;              // StatusBar font
        CString m_keyName;                  // CString for Registry key name
        CString m_statusText;               // CString for status text
        CString m_tooltip;                  // CString for tool tips
        MenuTheme m_mbTheme;                // struct of theme info for the popup Menu and MenuBar
        ReBarTheme m_rbTheme;               // struct of theme info for the ReBar
        StatusBarTheme m_sbTheme;           // struct of theme info for the StatusBar
        ToolBarTheme m_tbTheme;             // struct of theme info for the ToolBar
        HACCEL m_accel;                     // handle to the frame's accelerator table (used by MDI without MDI child)
        CWnd* m_pView;                      // pointer to the View CWnd object
        size_t m_maxMRU;                    // maximum number of MRU entries
        HWND m_oldFocus;                    // The window that had focus prior to the app's deactivation
        HHOOK m_kbdHook;                    // Keyboard hook.

        CMenuMetrics m_menuMetrics;         // The MenuMetrics object
        CImageList m_menuImages;            // Imagelist of menu icons
        CImageList m_menuDisabledImages;    // Imagelist of disabled menu icons
        BOOL m_useOwnerDrawnMenu;           // Set to TRUE for custom drawn menu items.
        BOOL m_useDarkMenu;                 // Set to TRUE to manually draw a dark menu.
        BOOL m_useIndicatorStatus;          // Set to TRUE to see indicators in status bar.
        BOOL m_useMenuStatus;               // Set to TRUE to see menu and toolbar updates in status bar.
        BOOL m_useReBar;                    // Set to TRUE if ReBars are to be used.
        BOOL m_useStatusBar;                // Set to TRUE if the statusbar is used.
        BOOL m_useThemes;                   // Set to TRUE if themes are to be used.
        BOOL m_useToolBar;                  // Set to TRUE if the toolbar is used.
        BOOL m_altKeyPressed;               // Set to TRUE if the alt key is held down.

    };  // class CFrameT


    /////////////////////////////////////////
    // CFrame manages the frame window. CFrame also manages the
    // creation and position of child windows, such as the menubar,
    // toolbar, view window and statusbar.
    class CFrame : public CFrameT<CWnd>
    {
    public:
        CFrame() {}
        virtual ~CFrame() override {}

    private:
        CFrame(const CFrame&) = delete;
        CFrame& operator=(const CFrame&) = delete;
    };

}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


namespace Win32xx
{

    ///////////////////////////////////
    // Definitions for the CFrame class
    //
    template <class T>
    inline CFrameT<T>::CFrameT() : m_aboutDialog(IDW_ABOUT), m_accel(nullptr), m_pView(nullptr), m_maxMRU(0), m_oldFocus(nullptr),
                              m_kbdHook(nullptr), m_useOwnerDrawnMenu(TRUE), m_useDarkMenu(FALSE), m_useIndicatorStatus(TRUE),
                              m_useMenuStatus(TRUE), m_useStatusBar(TRUE), m_useThemes(TRUE), m_useToolBar(TRUE),
                              m_altKeyPressed(FALSE)
    {
        m_mbTheme = {};
        m_rbTheme = {};
        m_sbTheme = {};
        m_tbTheme = {};
        m_indicators.assign(3, CString());

        // By default, we use the rebar.
        m_useReBar = TRUE;

        // Assign the default menubar, rebar, statusbar and toolbar.
        SetMenuBar(m_menuBar);
        SetReBar(m_reBar);
        SetStatusBar(m_statusBar);
        SetToolBar(m_toolBar);
    }

    template <class T>
    inline CFrameT<T>::~CFrameT()
    {
        if (m_kbdHook != nullptr)
            ::UnhookWindowsHookEx(m_kbdHook);
    }

    // Adds the grayscale image of the specified icon the disabled menu image-list.
    // This function is called by AddMenuIcon.
    template <class T>
    inline void CFrameT<T>::AddDisabledMenuImage(HICON icon, COLORREF mask)
    {
        CClientDC desktopDC(*this);
        CMemDC memDC(desktopDC);

        // m_menuImages should already have this image.
        assert(m_menuImages.GetHandle() != nullptr);

        int cxImage = m_menuImages.GetIconSize().cx;
        int cyImage = cxImage;

        memDC.CreateCompatibleBitmap(desktopDC, cxImage, cyImage);
        CRect rc;
        rc.SetRect(0, 0, cxImage, cyImage);

        // Set the mask color to gray for the new ImageList.
        if (GetDeviceCaps(desktopDC, BITSPIXEL) < 24)
        {
            HPALETTE hPal = desktopDC.GetCurrentPalette();
            UINT index = ::GetNearestPaletteIndex(hPal, mask);
            if (index != CLR_INVALID) mask = PALETTEINDEX(index);
        }

        memDC.SolidFill(mask, rc);

        // Draw the icon on the memory DC.
        memDC.DrawIconEx(0, 0, icon, cxImage, cyImage, 0, 0, DI_NORMAL);

        // Detach the bitmap so we can use it.
        CBitmap bitmap = memDC.DetachBitmap();
        bitmap.ConvertToDisabled(mask);

        if (m_menuDisabledImages.GetHandle() == nullptr)
            m_menuDisabledImages.Create(cxImage, cyImage, ILC_COLOR24 | ILC_MASK, 1, 0);

        m_menuDisabledImages.Add(bitmap, mask);
    }

    // Adds an icon to an internal ImageList for use with popup menu items.
    template <class T>
    inline BOOL CFrameT<T>::AddMenuIcon(UINT menuItemID, UINT iconID, UINT disabledIconID)
    {
        HICON icon = static_cast<HICON>(GetApp()->LoadImage(iconID, IMAGE_ICON, 0, 0, LR_SHARED));
        HICON disabledIcon = static_cast<HICON>(GetApp()->LoadImage(disabledIconID, IMAGE_ICON, 0, 0, LR_SHARED));
        return AddMenuIcon(menuItemID, icon, disabledIcon);
    }

    // Adds an icon to an internal ImageList for use with popup menu items.
    template <class T>
    inline BOOL CFrameT<T>::AddMenuIcon(UINT menuItemID, HICON icon, HICON disabledIcon)
    {
        assert(icon != nullptr);

        int cxImage;
        int cyImage;

        // Create a new ImageList if required.
        if (m_menuImages.GetHandle() == nullptr)
        {
            cyImage = GetMenuIconHeight();
            cxImage = cyImage;
            m_menuImages.Create(cxImage, cyImage, ILC_COLOR32 | ILC_MASK, 1, 0);
            m_menuItemIDs.clear();
        }
        else
        {
            cxImage = m_menuImages.GetIconSize().cx;
            cyImage = cxImage;
        }

        if (m_menuImages.Add(icon) != -1)
        {
            m_menuItemIDs.push_back(menuItemID);

            // Set the mask color to gray for the new ImageList.
            COLORREF mask = RGB(192, 192, 192);
            CClientDC desktopDC(*this);
            if (GetDeviceCaps(desktopDC, BITSPIXEL) < 24)
            {
                HPALETTE hPal = desktopDC.GetCurrentPalette();
                UINT index = ::GetNearestPaletteIndex(hPal, mask);
                if (index != CLR_INVALID) mask = PALETTEINDEX(index);
            }

            if (m_menuDisabledImages.GetHandle() == nullptr)
                m_menuDisabledImages.Create(cxImage, cyImage, ILC_COLOR32 | ILC_MASK, 1, 0);

            if (disabledIcon == nullptr)
                AddDisabledMenuImage(icon, mask);
            else
                m_menuDisabledImages.Add(disabledIcon);

            return TRUE;
        }

        return FALSE;
    }

    // Adds the icons from a bitmap resource to an internal ImageList for use with popup menu items.
    // Note:  Images for menu icons can be sized 16x16 or 16x15 pixels or higher.
    //        If the images are too big to fit in the menu, they are ignored.
    template <class T>
    inline UINT CFrameT<T>::AddMenuIcons(const std::vector<UINT>& menuData, COLORREF mask, UINT bitmapID, UINT disabledID)
    {
        // Count the MenuData entries excluding separators.
        int images = 0;
        for (const UINT& data : menuData)
        {
            if (data != 0)   // Don't count separators
                ++images;
        }

        // Load the button images from the resource ID.
        CBitmap bitmap(bitmapID);

        // Assert if we failed to load the bitmap.
        assert(bitmap.GetHandle() != nullptr);

        if ((images == 0) || (bitmap.GetHandle() == nullptr))
            return static_cast<UINT>(m_menuItemIDs.size());  // No valid images, so nothing to do!

        // Resize the bitmap
        CSize bitmapSize = bitmap.GetSize();
        int scale = GetMenuIconHeight() / bitmapSize.cy;
        m_menuItemIDs.clear();
        if (scale > 0)
        {
            bitmap = ScaleUpBitmap(bitmap, scale);
            int bitmapSizeY = bitmap.GetSize().cy;
            int newSize = std::max(bitmapSizeY, 16);

            // Create the ImageList.
            m_menuImages.Create(newSize, newSize, ILC_COLOR32 | ILC_MASK, images, 0);

            // Add the resource IDs to the m_menuIcons vector.
            for (auto it = menuData.begin(); it != menuData.end(); ++it)
            {
                if ((*it) != 0)
                    m_menuItemIDs.push_back(*it);
            }

            // Add the images to the imageList.
            m_menuImages.Add(bitmap, mask);

            // Add the images to the disabled imagelist.
            if (disabledID != 0)
            {
                // Create the disabled ImageList from disabledID.
                CBitmap disabled(disabledID);
                scale = GetMenuIconHeight() / disabled.GetSize().cy;
                if (scale > 0)
                {
                    disabled = ScaleUpBitmap(disabled, scale);
                    int disabledSizeY = disabled.GetSize().cy;
                    newSize = std::max(disabledSizeY, 16);
                    m_menuDisabledImages.Create(newSize, newSize, ILC_COLOR32 | ILC_MASK, images, 0);
                    m_menuDisabledImages.Add(disabled, mask);
                }
                else
                    m_menuDisabledImages.CreateDisabledImageList(m_menuImages);
            }
            else
                m_menuDisabledImages.CreateDisabledImageList(m_menuImages);
        }

        // return the number of menu icons.
        return static_cast<UINT>(m_menuItemIDs.size());
    }

    // Adds a MenuBar to the rebar control.
    template <class T>
    inline void CFrameT<T>::AddMenuBarBand()
    {
        REBARBANDINFO rbbi{};
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
            m_mruEntries.erase(m_mruEntries.begin() + static_cast<int>(m_maxMRU), m_mruEntries.end());

        UpdateMRUMenu();
    }

    // Adds a ToolBar to the rebar control.
    template <class T>
    inline void CFrameT<T>::AddToolBarBand(CToolBar& tb, DWORD bandStyle, UINT id)
    {
        // Create the ToolBar Window.
        DWORD style = WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | TBSTYLE_TOOLTIPS | TBSTYLE_FLAT | CCS_NODIVIDER | CCS_NORESIZE | CCS_NOPARENTALIGN;
        tb.CreateEx(0, TOOLBARCLASSNAME, 0, style, 0, 0, 0, 0, GetReBar(), 0);

        // Fill the REBARBAND structure.
        REBARBANDINFO rbbi{};
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
    inline void CFrameT<T>::AddToolBarButton(UINT id, BOOL isEnabled /* = TRUE*/, LPCTSTR text /* = nullptr */, int image /* = -1 */)
    {
        m_toolBarData.push_back(id);

        GetToolBar().AddButton(id, isEnabled, image);

        if (text != nullptr)
            GetToolBar().SetButtonText(id, text);
    }

    // Adjust the size of the frame to accommodate the View window's dimensions.
    template <class T>
    inline void CFrameT<T>::AdjustFrameRect(const RECT& viewRect)
    {
        // Adjust for the view styles.
        CRect rc = viewRect;
        DWORD style = GetView().GetStyle();
        DWORD exStyle = GetView().GetExStyle();
        VERIFY(AdjustWindowRectEx(&rc, style, FALSE, exStyle));

        // Adjust for the frame's non-client area.
        int width = T::GetWindowRect().Width() - T::GetClientRect().Width() + rc.Width();
        int height = T::GetWindowRect().Height() - GetViewRect().Height() + rc.Height();

        // Calculate final rect size, and reposition frame.
        VERIFY(T::SetWindowPos(HWND_TOP, 0, 0, width, height, SWP_NOMOVE));
    }

    // Clears the normal and disabled imagelists for menu icons.
    template <class T>
    inline void CFrameT<T>::ClearMenuIcons()
    {
        m_menuItemIDs.clear();
        m_menuImages.DeleteImageList();
        m_menuDisabledImages.DeleteImageList();
    }

    // Creates the frame's toolbar. Additional toolbars can be added with AddToolBarBand
    // if the frame uses a rebar.
    template <class T>
    inline void CFrameT<T>::CreateToolBar()
    {
        m_toolBarData.clear();

        if (GetReBar().IsWindow())
            AddToolBarBand(GetToolBar(), RBBS_BREAK|RBBS_GRIPPERALWAYS, IDW_TOOLBAR);   // Create the toolbar inside rebar.
        else
            GetToolBar().Create(*this); // Create the toolbar without a rebar.

        SetupToolBar();

        if (GetToolBarData().size() > 0)
        {
            // Load the default images if no images are loaded.
            // A mask of 192,192,192 is compatible with AddBitmap (for Win95).
            if (!GetToolBar().SendMessage(TB_GETIMAGELIST, 0, 0))
                SetToolBarImages(RGB(192, 192, 192), IDW_MAIN, 0, 0);

            GetToolBar().Autosize();
        }
        else
        {
            TRACE("\n*** WARNING: No resource IDs assigned to the toolbar. ***\n\n");
            ShowToolBar(FALSE);
        }

        if (GetReBar().IsWindow())
        {
            SIZE MaxSize = GetToolBar().GetMaxSize();
            WPARAM wparam = reinterpret_cast<WPARAM>(GetToolBar().GetHwnd());
            LPARAM lparam = reinterpret_cast<LPARAM>(&MaxSize);
            GetReBar().SendMessage(UWM_TBRESIZE, wparam, lparam);

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
        assert(pMenubar != nullptr);

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
                UINT state = lpNMCustomDraw->nmcd.uItemState;
                DWORD item = static_cast<DWORD>(lpNMCustomDraw->nmcd.dwItemSpec);

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
                        UINT buttonState = pMenubar->GetButtonState(item);
                        if ((state & CDIS_SELECTED) || (buttonState & TBSTATE_PRESSED))
                        {
                            drawDC.GradientFill(GetMenuBarTheme().clrPressed1, GetMenuBarTheme().clrPressed2, rc, TRUE);
                        }
                        else if (state & CDIS_HOT)
                        {
                            drawDC.GradientFill(GetMenuBarTheme().clrHot1, GetMenuBarTheme().clrHot2, rc, TRUE);
                        }

                        // Draw border.
                        drawDC.CreatePen(PS_SOLID, 1, GetMenuBarTheme().clrOutline);
                        drawDC.MoveTo(rc.left, rc.bottom);
                        drawDC.LineTo(rc.left, rc.top);
                        drawDC.LineTo(rc.right-1, rc.top);
                        drawDC.LineTo(rc.right-1, rc.bottom);
                        drawDC.MoveTo(rc.right-1, rc.bottom);
                        drawDC.LineTo(rc.left, rc.bottom);
                    }

                    UINT itemID = static_cast<UINT>(lpNMCustomDraw->nmcd.dwItemSpec);
                    CString str = pMenubar->GetButtonText(itemID);

                    // Draw highlight text.
                    CFont font = pMenubar->GetFont();
                    drawDC.SelectObject(font);

                    rc.bottom += 1;
                    drawDC.SetBkMode(TRANSPARENT);
                    drawDC.SetTextColor(GetMenuBarTheme().clrText);
                    UINT format = DT_VCENTER | DT_CENTER | DT_SINGLELINE | DT_HIDEPREFIX;

                    // Turn off 'hide prefix' style for keyboard navigation.
                    if (m_altKeyPressed || pMenubar->IsAltMode())
                        format &= ~DT_HIDEPREFIX;

                    drawDC.DrawText(str, str.GetLength(), rc, format);

                    return CDRF_SKIPDEFAULT;  // No further drawing.
                }
            }
            return CDRF_DODEFAULT;   // Do default drawing.

        }

        return 0;
    }

    // With CustomDraw we manually control the drawing of each toolbar button.
    // Supports toolbars with or without the TBSTYLE_LIST style.
    // Supports buttons with or without the BTNS_WHOLEDROPDOWN and BTNS_DROPDOWN styles.
    // Requires the toolbar buttons to have images.
    template <class T>
    inline LRESULT CFrameT<T>::CustomDrawToolBar(NMHDR* pNMHDR)
    {
        if (GetToolBarTheme().UseThemes)
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
                        UINT state = pCustomDraw->nmcd.uItemState;
                        UINT item = static_cast<UINT>(pCustomDraw->nmcd.dwItemSpec);

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
                        bool isHot = (state & CDIS_HOT) != 0;
                        if (isHot)
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
                        bool isPressed = (pTB->GetButtonState(item) & TBSTATE_PRESSED) != 0;
                        if (isPressed)
                        {
                            drawDC.GradientFill(GetToolBarTheme().clrPressed1, GetToolBarTheme().clrPressed2, rc, TRUE);
                        }
                        else if (isHot)
                        {
                            drawDC.GradientFill(GetToolBarTheme().clrHot1, GetToolBarTheme().clrHot2, rc, TRUE);
                        }

                        // Get the appropriate image list depending on the button state.
                        CImageList toolBarImages;
                        bool isDisabled = (state & CDIS_DISABLED) != 0;
                        if (isDisabled)
                        {
                            toolBarImages = pTB->GetDisabledImageList();
                            if (toolBarImages.GetHandle() == nullptr)
                            {
                                CImageList toolBarDisabledImages;
                                toolBarDisabledImages.CreateDisabledImageList(pTB->GetImageList());
                                pTB->SetDisableImageList(toolBarDisabledImages);
                                toolBarImages = pTB->GetDisabledImageList();
                            }
                        }
                        else if (isHot)
                        {
                            toolBarImages = pTB->GetHotImageList();
                            if (toolBarImages.GetHandle() == nullptr)
                                // Use normal images as hot images.
                                toolBarImages = pTB->GetImageList();
                        }
                        else
                        {
                            toolBarImages = pTB->GetImageList();
                        }

                        // Assert if the toolbar images aren't set.
                        assert(toolBarImages.GetHandle() != nullptr);

                        DWORD style = pTB->GetButtonStyle(item);
                        DWORD exStyle = pTB->GetExtendedStyle();
                        bool isDropDown = ((style & BTNS_DROPDOWN) && (exStyle & TBSTYLE_EX_DRAWDDARROWS));
                        bool isWholeDropDown = (style & BTNS_WHOLEDROPDOWN) != 0;
                        bool isListToolbar = (pTB->GetStyle() & TBSTYLE_LIST) != 0;

                        // Calculate dropdown width.
                        int dropDownWidth = 0;
                        if (isDropDown || isWholeDropDown)
                        {
                            // Use the internal Marlett font to determine the width for the drop down arrow section.
                            int cyMenuCheck = ::GetSystemMetrics(SM_CYMENUCHECK) * GetWindowDpi(*this) / GetWindowDpi(HWND_DESKTOP);
                            drawDC.CreateFont(cyMenuCheck, 0, 0, 0, FW_NORMAL, 0, 0, 0,
                                              SYMBOL_CHARSET, 0, 0, 0, 0, _T("Marlett"));

                            drawDC.GetCharWidth('6', '6', &dropDownWidth);
                        }

                        // Calculate image position.
                        CSize szImage = toolBarImages.GetIconSize();
                        int xImage = 0;
                        int yImage = 0;
                        int pressedOffset = (state & CDIS_SELECTED) ? 1 : 0;

                        if (isListToolbar)
                        {
                            // Calculate the image position for the TBSTYLE_LIST toolbar style.
                            // This style positions the button text to the right of the bitmap.
                            xImage = rc.left + pressedOffset;
                            yImage = (rc.bottom - rc.top - szImage.cy +2) / 2 + pressedOffset;
                        }
                        else
                        {
                            // Calculate the image position without the TBSTYLE_LIST toolbar style.
                            int dropAjust = (dropDownWidth * 3) / 4;
                            xImage = (rc.right + rc.left - szImage.cx - dropAjust) / 2 + pressedOffset;
                            yImage = (rc.bottom + rc.top - szImage.cy - textSize.cy) / 2;
                        }

                        if (isDropDown || isWholeDropDown)
                        {
                            // Calculate arrow position for the TBSTYLE_DROPDOWN and BTNS_WHOLEDROPDOWN button styles.
                            int arrowHeight = (dropDownWidth + 1) / 5;
                            int xArrow = rc.right - dropDownWidth / 2;
                            int yArrow = (yImage + szImage.cy + arrowHeight) / 2;

                            if (isDropDown)
                            {
                                yArrow = (rc.Height() + arrowHeight) / 2;
                            }

                            if (isListToolbar)
                            {
                                yArrow += 1;
                            }

                            // Draw separate background for dropdown arrow.
                            if (isHot && isDropDown)
                            {
                                CRect arrowRect = rc;
                                arrowRect.left = arrowRect.right - dropDownWidth;
                                drawDC.GradientFill(GetToolBarTheme().clrPressed1, GetToolBarTheme().clrPressed2, arrowRect, TRUE);
                            }

                            // Draw the dropdown arrow.
                            drawDC.CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
                            for (int i = arrowHeight; i >= 0; --i)
                            {
                                drawDC.MoveTo(xArrow - i-1, yArrow - i+1);
                                drawDC.LineTo(xArrow + i,   yArrow - i+1);
                            }

                            // Draw the line between icon and dropdown arrow.
                            if (isDropDown && isHot)
                            {
                                drawDC.CreatePen(PS_SOLID, 1, GetToolBarTheme().clrOutline);
                                drawDC.MoveTo(rc.right - dropDownWidth, rc.top);
                                drawDC.LineTo(rc.right - dropDownWidth, rc.bottom);
                            }
                        }


                        // Draw the button image.
                        TBBUTTON tbb{};
                        int button = pTB->CommandToIndex(item);
                        pTB->GetButton(button, tbb);
                        int image = tbb.iBitmap;
                        toolBarImages.Draw(drawDC, image, CPoint(xImage, yImage), ILD_TRANSPARENT);

                        // Draw the text.
                        if (!str.IsEmpty())
                        {
                            // Calculate the text position
                            int width = rc.right - rc.left - (isDropDown ? dropDownWidth : 0);
                            int textSizeX = textSize.cx;
                            int textSizeY = textSize.cy;
                            CRect textRect(0, 0, std::min(textSizeX, width), textSizeY);

                            int xOffset = rc.left + (rc.Width() - textRect.Width()) / 2;
                            if (isDropDown)
                                xOffset -= dropDownWidth / 2;

                            int yOffset = yImage + szImage.cy + 1;

                            if (isListToolbar)
                            {
                                int textSpace = rc.Width() - szImage.cx - textRect.Width() - dropDownWidth;
                                xOffset = rc.left + szImage.cx + textSpace / 2;
                                yOffset = (rc.Height() - textRect.Height())/2 + pressedOffset + 1;
                            }

                            OffsetRect(&textRect, xOffset, yOffset);

                            // Select the toolbar's font with a transparent background.
                            int mode = drawDC.SetBkMode(TRANSPARENT);
                            drawDC.SelectObject(pTB->GetFont());

                            if (isDisabled)
                            {
                                // Draw disabled text twice for embossed look.
                                textRect.OffsetRect(1, 1);
                                drawDC.SetTextColor(RGB(255, 255, 255));
                                drawDC.DrawText(str, str.GetLength(), textRect, DT_LEFT);
                                textRect.OffsetRect(-1, -1);
                                drawDC.SetTextColor(GetSysColor(COLOR_GRAYTEXT));
                                drawDC.DrawText(str, str.GetLength(), textRect, DT_LEFT);
                            }
                            else
                            {
                                // Draw normal text.
                                if ((GetMenuBarTheme().UseThemes))
                                   drawDC.SetTextColor(GetMenuBarTheme().clrText);
                                else
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
    inline void CFrameT<T>::DrawMenuItem(LPDRAWITEMSTRUCT pDrawItem)
    {
        MenuItemData* pmid = reinterpret_cast<MenuItemData*>(pDrawItem->itemData);

        // Create and configure the memory DC.
        CDC drawDC = pDrawItem->hDC;
        CRect itemRect = pDrawItem->rcItem;
        CMemDC memDC(drawDC);
        memDC.CreateCompatibleBitmap(drawDC, itemRect.Width(), itemRect.Height());
        memDC.BitBlt(0, 0, itemRect.Width(), itemRect.Height(), drawDC, itemRect.left, itemRect.top, SRCCOPY);
        memDC.SelectObject(GetMenuFont());

        // Swap the PDIS->hDC with a memory DC for double buffering.
        pDrawItem->hDC = memDC;
        pDrawItem->rcItem.top = 0;
        pDrawItem->rcItem.bottom = itemRect.Height();

        if (IsUsingVistaMenu() && !IsUsingDarkMenu())
        {
            DrawVistaMenuBkgnd(pDrawItem);

            if (!(pmid->mii.fType & MFT_SEPARATOR))
            {
                if (pDrawItem->itemState & ODS_CHECKED)
                    DrawVistaMenuCheckmark(pDrawItem);

                DrawVistaMenuText(pDrawItem);
            }
        }
        else
        {
            // Draw the gutter.
            CRect gutter = GetMenuMetrics().GetGutterRect(pDrawItem->rcItem);

            if (IsUsingThemes())
            {
                const MenuTheme& mbt = GetMenuBarTheme();
                memDC.GradientFill(mbt.clrPressed1, mbt.clrPressed2, gutter, FALSE);
            }
            else
            {
                memDC.SolidFill(RGB(255, 255, 255), gutter);
            }

            if (pmid->mii.fType & MFT_SEPARATOR)
            {
                // Draw the separator.
                CRect sepRect = pDrawItem->rcItem;
                sepRect.left = gutter.Width();
                if (IsUsingDarkMenu())
                    memDC.SolidFill(RGB(0, 0, 0), sepRect);
                else
                    memDC.SolidFill(RGB(255, 255, 255), sepRect);

                sepRect.top += sepRect.Height() / 2;
                memDC.CreatePen(PS_SOLID, 1, RGB(128, 128, 128));
                memDC.MoveTo(sepRect.left, sepRect.top);
                memDC.LineTo(sepRect.right, sepRect.top);
            }
            else
            {
                DrawMenuItemBkgnd(pDrawItem);
                DrawMenuItemText(pDrawItem);

                if (pDrawItem->itemState & ODS_CHECKED)
                    DrawMenuItemCheckmark(pDrawItem);
            }
        }

        if (!(pmid->mii.fType & MFT_SEPARATOR))
        {
            if (!(pDrawItem->itemState & ODS_CHECKED))
                DrawMenuItemIcon(pDrawItem);
        }

        // Copy from the memory DC to the menu item drawing DC.
        drawDC.BitBlt(itemRect.left, itemRect.top, itemRect.Width(), itemRect.Height(), memDC, 0, 0, SRCCOPY);

        // Return pDrawItem to its previous values.
        pDrawItem->hDC = drawDC;
        pDrawItem->rcItem = itemRect;
    }

    // Draws the popup menu background if uxtheme.dll is not loaded.
    template <class T>
    inline void CFrameT<T>::DrawMenuItemBkgnd(LPDRAWITEMSTRUCT pDrawItem)
    {
        // Draw the item background
        bool isDisabled = (pDrawItem->itemState & ODS_GRAYED) != 0;
        bool isSelected = (pDrawItem->itemState & ODS_SELECTED) != 0;
        CRect drawRect = pDrawItem->rcItem;
        CDC drawDC(pDrawItem->hDC);

        if ((isSelected) && (!isDisabled))
        {
            // draw selected item background.
            if (IsUsingThemes())
            {
                const MenuTheme& mbt = GetMenuBarTheme();
                drawDC.CreateSolidBrush(mbt.clrHot1);
                drawDC.CreatePen(PS_SOLID, 1, mbt.clrOutline);
            }
            else
            {
                drawDC.CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
                drawDC.CreatePen(PS_SOLID, 1, GetSysColor(COLOR_BTNFACE));
            }

            drawDC.Rectangle(drawRect.left, drawRect.top, drawRect.right, drawRect.bottom);
        }
        else
        {
            // draw non-selected item background.
            drawRect.left = GetMenuMetrics().GetGutterRect(pDrawItem->rcItem).Width();
            if (IsUsingDarkMenu())
                drawDC.SolidFill(RGB(0, 0, 0), drawRect);
            else
                drawDC.SolidFill(RGB(255, 255, 255), drawRect);
        }
    }

    // Draws the popup menu checkmark or radiocheck if uxtheme.dll is not loaded.
    template <class T>
    inline void CFrameT<T>::DrawMenuItemCheckmark(LPDRAWITEMSTRUCT pDrawItem)
    {
        CRect rc = pDrawItem->rcItem;
        MenuItemData* pmid = reinterpret_cast<MenuItemData*>(pDrawItem->itemData);
        UINT buttonType = pmid->mii.fType;
        const MenuTheme& mbt = GetMenuBarTheme();
        int cxCheck = T::DpiScaleInt(16);
        int cyCheck = T::DpiScaleInt(16);
        CRect gutter = GetMenuMetrics().GetGutterRect(rc);
        int left = (gutter.Width() - cxCheck) / 2;
        int top = rc.top + (rc.Height() - cyCheck) / 2;
        CRect bkRect(left, top, left + cxCheck, top + cyCheck);
        CDC drawDC(pDrawItem->hDC);

        // Draw the checkmark's background rectangle first.
        if (IsUsingThemes() && !IsUsingVistaMenu())
        {
            drawDC.CreatePen(PS_SOLID, 1, mbt.clrOutline);
            if (IsUsingThemes())
                drawDC.CreateSolidBrush(mbt.clrHot2);
            else
                drawDC.CreateSolidBrush(GetSysColor(COLOR_BTNFACE));

            drawDC.Rectangle(bkRect);
        }

        CMemDC maskDC(drawDC);
        maskDC.CreateBitmap(cxCheck, cyCheck, 1, 1, nullptr);
        CRect checkRect(0, 0, cxCheck, cyCheck);

        // Copy the check mark bitmap to the mask's device context.
        if (buttonType == MFT_RADIOCHECK)
            maskDC.DrawFrameControl(checkRect, DFC_MENU, DFCS_MENUBULLET);
        else
            maskDC.DrawFrameControl(checkRect, DFC_MENU, DFCS_MENUCHECK);

        int xoffset = (buttonType == MFT_RADIOCHECK)? 1 : 2;
        int yoffset = 0;

        // Draw a white or black check mark as required.
        CMemDC memDC(drawDC);
        memDC.CreateCompatibleBitmap(drawDC, cxCheck, cyCheck);
        memDC.BitBlt(0, 0, cxCheck, cyCheck, memDC, 0, 0, WHITENESS);

        if ((pDrawItem->itemState & ODS_SELECTED) && IsUsingThemes())
        {
            // Draw a white checkmark
            maskDC.BitBlt(0, 0, cxCheck, cyCheck, maskDC, 0, 0, DSTINVERT);
            memDC.BitBlt(0, 0, cxCheck, cyCheck, maskDC, 0, 0, SRCAND);
            drawDC.BitBlt(bkRect.left + xoffset, bkRect.top + yoffset, cxCheck, cyCheck, memDC, 0, 0, SRCPAINT);
        }
        else
        {
            // Draw a black checkmark.
            memDC.BitBlt(0, 0, cxCheck, cyCheck, maskDC, 0, 0, SRCAND);
            drawDC.BitBlt(bkRect.left + xoffset, bkRect.top + yoffset, cxCheck, cyCheck, memDC, 0, 0, SRCAND);

            if (IsUsingDarkMenu())
            {
                // Draw a gray checkmark over black checkmark.
                CRect rcCheck(0, 0, cxCheck, cyCheck);
                constexpr COLORREF gray = RGB(192, 192, 192);
                memDC.SolidFill(gray, rcCheck);
                maskDC.BitBlt(0, 0, cxCheck, cyCheck, maskDC, 0, 0, DSTINVERT);
                memDC.BitBlt(0, 0, cxCheck, cyCheck, maskDC, 0, 0, SRCAND);
                drawDC.BitBlt(bkRect.left + xoffset, bkRect.top + yoffset, cxCheck, cyCheck, memDC, 0, 0, SRCPAINT);
            }
        }
    }

    // Called by DrawMenuItem to draw icons in popup menus.
    template <class T>
    inline void CFrameT<T>::DrawMenuItemIcon(LPDRAWITEMSTRUCT pDrawItem)
    {
        if (m_menuImages.GetHandle() == 0)
            return;

        // Get icon size.
        CSize iconSize = m_menuImages.GetIconSize();
        int xIcon = iconSize.cx;
        int yIcon = iconSize.cy;

        // get the drawing rectangle.
        CRect itemRect = pDrawItem->rcItem;
        CRect gutter = GetMenuMetrics().GetGutterRect(pDrawItem->rcItem);
        int left = (gutter.Width() - xIcon) / 2;
        int top = itemRect.top + (itemRect.Height() - yIcon) / 2;

        // get the icon's location in the imagelist.
        int image = -1;
        for (size_t i = 0 ; i < m_menuItemIDs.size(); ++i)
        {
            if (pDrawItem->itemID == m_menuItemIDs[i])
               image = static_cast<int>(i);
        }

        // Daw the image.
        if (image >= 0 )
        {
            bool isDisabled = (pDrawItem->itemState & (ODS_GRAYED | ODS_DISABLED)) != 0;

            if ((isDisabled) && (m_menuDisabledImages.GetHandle()))
                m_menuDisabledImages.Draw(pDrawItem->hDC, image, CPoint(left, top), ILD_TRANSPARENT);
            else
                m_menuImages.Draw(pDrawItem->hDC, image, CPoint(left, top), ILD_TRANSPARENT);
        }
    }

    // Draws the popup menu text if uxtheme.dll is not loaded.
    template <class T>
    inline void CFrameT<T>::DrawMenuItemText(LPDRAWITEMSTRUCT pDrawItem)
    {
        MenuItemData* pmid = reinterpret_cast<MenuItemData*>(pDrawItem->itemData);
        CString itemText = pmid->itemText;
        bool isDisabled = (pDrawItem->itemState & ODS_GRAYED) != 0;
        COLORREF colorText = GetSysColor(isDisabled ?  COLOR_GRAYTEXT : COLOR_MENUTEXT);
        if (IsUsingDarkMenu())
        {
            colorText = isDisabled ? RGB(192, 192, 192) : RGB(255, 255, 255);
        }

        // Calculate the text rect size.
        CRect textRect = GetMenuMetrics().GetTextRect(pDrawItem->rcItem);

        // Find the position of tab character.
        int tab = itemText.Find(_T('\t'));

        // Draw the item text.
        SetTextColor(pDrawItem->hDC, colorText);
        int mode = SetBkMode(pDrawItem->hDC, TRANSPARENT);

        UINT format = DT_VCENTER | DT_LEFT | DT_SINGLELINE;
        // Turn on 'hide prefix' style for mouse navigation.
        CMenuBar* pMenubar = reinterpret_cast<CMenuBar*>
                             (::SendMessage(pDrawItem->hwndItem, UWM_GETCMENUBAR, 0, 0));
        if (pMenubar != nullptr)
        {
            if (!m_altKeyPressed && !pMenubar->IsAltMode())
                format |= DT_HIDEPREFIX;
        }

        DrawText(pDrawItem->hDC, itemText, tab, textRect, format);

        // Draw text after tab, right aligned.
        if (tab != -1)
        {
            DrawText(pDrawItem->hDC, itemText.Mid(tab + 1), -1, textRect, DT_SINGLELINE | DT_RIGHT | DT_VCENTER);
        }

        SetBkMode(pDrawItem->hDC, mode);
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

            bool isVertical = (rebar.GetStyle() & CCS_VERT) != 0;

            // Create our memory DC.
            CRect rebarRect = rebar.GetClientRect();
            int width = rebarRect.Width();
            int height = rebarRect.Height();
            CMemDC memDC(dc);
            memDC.CreateCompatibleBitmap(dc, width, height);

            // Draw to ReBar background to the memory DC.
            memDC.SolidFill(rt.clrBkgnd2, rebarRect);
            memDC.GradientFill(rt.clrBkgnd1, rt.clrBkgnd2, rebarRect, FALSE);

            if (rt.clrBand1 || rt.clrBand2)
            {
                // Draw the individual band backgrounds.
                for (int band = 0; band < rebar.GetBandCount(); ++band)
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
                            REBARBANDINFO rbbi{};
                            rbbi.fMask = RBBIM_CHILD;
                            rebar.GetBandInfo(band, rbbi);
                            CWnd* pChild = T::GetCWndPtr(rbbi.hwndChild);
                            assert(pChild);
                            CRect childRect = pChild->GetWindowRect();
                            pChild->ScreenToClient(childRect);

                            // Determine our drawing rectangle.
                            int startPad = IsXPThemed() ? 2 : 0;
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
                            sourceDC.CreateCompatibleBitmap(dc, width, height);
                            sourceDC.GradientFill(rt.clrBand1, rt.clrBand2, drawRect, !isVertical);

                            // Set Curve amount for rounded edges.
                            int curve = rt.RoundBorders ? T::DpiScaleInt(12) : 0;

                            // Create our mask for rounded edges using RoundRect.
                            CMemDC maskDC(dc);
                            maskDC.CreateCompatibleBitmap(dc, width, height);

                            int left = drawRect.left;
                            int right = drawRect.right;
                            int top = drawRect.top;
                            int bottom = drawRect.bottom;
                            int cx = drawRect.Width();// + xPad;
                            int cy = drawRect.Height();

                            if (rt.FlatStyle)
                            {
                                maskDC.SolidFill(RGB(0, 0, 0), drawRect);
                                maskDC.BitBlt(left, top, cx, cy, maskDC, left, top, PATINVERT);
                                maskDC.RoundRect(left, top, right, bottom, curve, curve);
                            }
                            else
                            {
                                maskDC.SolidFill(RGB(0, 0, 0), drawRect);
                                maskDC.RoundRect(left, top, right, bottom, curve, curve);
                                maskDC.BitBlt(left, top, cx, cy, maskDC, left, top, PATINVERT);
                            }

                            // Copy Source DC to Memory DC using the RoundRect mask.
                            memDC.BitBlt(left, top, cx, cy, sourceDC, left, top, SRCINVERT);
                            memDC.BitBlt(left, top, cx, cy, maskDC, left, top, SRCAND);
                            memDC.BitBlt(left, top, cx, cy, sourceDC, left, top, SRCINVERT);
                        }
                    }
                }
            }

            if (rt.UseLines)
            {
                // Draw lines between bands.
                for (int j = 0; j < GetReBar().GetBandCount() - 1; ++j)
                {
                    CRect bandRect = GetReBar().GetBandRect(j);
                    if (isVertical)
                    {
                        int rebarTop = rebarRect.top;
                        bandRect.top = std::max(0, rebarTop - 4);
                        bandRect.right += 2;
                    }
                    else
                    {
                        int rebarLeft = rebarRect.left;
                        bandRect.left = std::max(0, rebarLeft - 4);
                        bandRect.bottom += 2;
                    }
                    memDC.DrawEdge(bandRect, EDGE_ETCHED, BF_BOTTOM | BF_ADJUST);
                }
            }

            // Copy the Memory DC to the window's DC.
            dc.BitBlt(0, 0, width, height, memDC, 0, 0, SRCCOPY);
        }

        return isDrawn;
    }

    // Draws the status bar text with the appropriate color.
    template <class T>
    inline void CFrameT<T>::DrawStatusBar(LPDRAWITEMSTRUCT pDrawItem)
    {
        CDC dc(pDrawItem->hDC);
        CRect partRect = pDrawItem->rcItem;
        dc.SetBkMode(TRANSPARENT);
        if (IsUsingThemes())
            dc.SetTextColor(GetStatusBarTheme().clrText);
        else
            dc.SetTextColor(RGB(0, 0, 0));

        assert(pDrawItem->itemData != 0);
        CString text = reinterpret_cast<LPCTSTR>(pDrawItem->itemData);
        dc.DrawText(text, text.GetLength(), partRect, DT_SINGLELINE | DT_VCENTER);
    }

    // Draws the StatusBar's background when StatusBar themes are enabled.
    // Returns TRUE when the default background drawing is suppressed.
    template <class T>
    inline BOOL CFrameT<T>::DrawStatusBarBkgnd(CDC& dc, CStatusBar& statusbar)
    {
        BOOL isDrawn = FALSE;

        // XP Themes are required to modify the statusbar's background.
        if (IsXPThemed())
        {
            const StatusBarTheme& sbTheme = GetStatusBarTheme();
            if (sbTheme.UseThemes)
            {
                // Create our memory DC.
                CRect rc = statusbar.GetClientRect();
                int width = rc.Width();
                int height = rc.Height();
                CMemDC memDC(dc);
                memDC.CreateCompatibleBitmap(dc, width, height);

                // Fill the background with a color gradient.
                memDC.GradientFill(sbTheme.clrBkgnd1, sbTheme.clrBkgnd2, statusbar.GetClientRect(), FALSE);

                // Copy the Memory DC to the window's DC.
                dc.BitBlt(0, 0, width, height, memDC, 0, 0, SRCCOPY);

                isDrawn = TRUE;
            }
        }

        return isDrawn;
    }

    // Draws the popup menu background if uxtheme.dll is loaded.
    template <class T>
    inline void CFrameT<T>::DrawVistaMenuBkgnd(LPDRAWITEMSTRUCT pDrawItem)
    {
        int stateID = GetMenuMetrics().ToItemStateId(pDrawItem->itemState);

        if (GetMenuMetrics().IsThemeBackgroundPartiallyTransparent(MENU_POPUPITEM, stateID))
        {
            GetMenuMetrics().DrawThemeBackground(pDrawItem->hDC, MENU_POPUPBACKGROUND, 0, &pDrawItem->rcItem, nullptr);
        }

        // Draw the gutter.
        CRect gutter = GetMenuMetrics().GetGutterRect(pDrawItem->rcItem);
        if (GetMenuMetrics().IsThemeBackgroundPartiallyTransparent(MENU_POPUPITEM, stateID))
        {
            GetMenuMetrics().DrawThemeBackground(pDrawItem->hDC, MENU_POPUPGUTTER, 0, &gutter, nullptr);
        }

        MenuItemData* pmid = reinterpret_cast<MenuItemData*>(pDrawItem->itemData);
        if (pmid->mii.fType & MFT_SEPARATOR)
        {
            // Draw the separator.
            CRect sepRect = GetMenuMetrics().GetSeperatorRect(pDrawItem->rcItem);
            GetMenuMetrics().DrawThemeBackground(pDrawItem->hDC, MENU_POPUPSEPARATOR, 0, &sepRect, nullptr);
        }

        CRect selRect = GetMenuMetrics().GetSelectionRect(pDrawItem->rcItem);
        GetMenuMetrics().DrawThemeBackground(pDrawItem->hDC, MENU_POPUPITEM, stateID, &selRect, nullptr);
    }

    // Draws the popup menu checkmark if uxtheme.dll is loaded.
    template <class T>
    inline void CFrameT<T>::DrawVistaMenuCheckmark(LPDRAWITEMSTRUCT pDrawItem)
    {
        MenuItemData* pmid = reinterpret_cast<MenuItemData*>(pDrawItem->itemData);

        // Draw the checkmark background.
        int stateID = GetMenuMetrics().ToItemStateId(pDrawItem->itemState);
        CRect rcCheckBackground = GetMenuMetrics().GetCheckBackgroundRect(pDrawItem->rcItem);
        int backgroundStateID = GetMenuMetrics().ToCheckBackgroundStateId(stateID);
        GetMenuMetrics().DrawThemeBackground(pDrawItem->hDC, MENU_POPUPCHECKBACKGROUND, backgroundStateID, &rcCheckBackground, nullptr);

        // Draw the checkmark.
        CRect rcCheck = GetMenuMetrics().GetCheckRect(pDrawItem->rcItem);
        int checkStateID = GetMenuMetrics().ToCheckStateId(pmid->mii.fType, stateID);
        GetMenuMetrics().DrawThemeBackground(pDrawItem->hDC, MENU_POPUPCHECK, checkStateID, &rcCheck, nullptr);
    }

    // Draws the popup menu text if uxtheme.dll is loaded.
    template <class T>
    inline void CFrameT<T>::DrawVistaMenuText(LPDRAWITEMSTRUCT pDrawItem)
    {
        MenuItemData* pmid = reinterpret_cast<MenuItemData*>(pDrawItem->itemData);

        // Calculate the text rect size.
        CStringW itemText(TtoW(pmid->itemText));
        CRect textRect = GetMenuMetrics().GetTextRect(pDrawItem->rcItem);

        // find the position of tab character.
        int tab = itemText.Find(L'\t');

        // Draw the item text before the tab.
        ULONG accel = ((pDrawItem->itemState & ODS_NOACCEL) ? DT_HIDEPREFIX : 0U);
        int stateID = GetMenuMetrics().ToItemStateId(pDrawItem->itemState);
        DWORD flags = DT_SINGLELINE | DT_LEFT | DT_VCENTER | accel;
        GetMenuMetrics().DrawThemeText(pDrawItem->hDC, MENU_POPUPITEM, stateID, itemText, tab, flags, 0, &textRect);

        // Draw the item text after the tab.
        if (tab != -1)
        {
            flags = DT_SINGLELINE | DT_RIGHT | DT_VCENTER | accel;
            CStringW text = itemText.Mid(tab + 1);
            GetMenuMetrics().DrawThemeText(pDrawItem->hDC, MENU_POPUPITEM, stateID, text, -1, flags, 0, &textRect);
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

    // Returns the preferred height of menu icons that fits nicely within a
    // menu item. The value returned will be a multiple of 8 (e.g. 16, 24 or 32).
    template <class T>
    inline int CFrameT<T>::GetMenuIconHeight() const
    {
        return GetMenuMetrics().GetMenuIconHeight();
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
    inline CSize CFrameT<T>::GetTBImageSize(CBitmap* pBitmap) const
    {
        assert(pBitmap);
        if (!pBitmap) return CSize(0, 0);

        assert(pBitmap->GetHandle());
        BITMAP data = pBitmap->GetBitmapData();
        int cy = data.bmHeight;
        int cx = std::max(cy, 16);

        return CSize(cx, cy);
    }

    // Returns the XP theme name.
    template <class T>
    inline CString CFrameT<T>::GetXPThemeName() const
    {
        HMODULE theme = ::GetModuleHandle(_T("uxtheme.dll"));
        WCHAR themeName[31] = L"";
        if (theme != nullptr)
        {
            using GETCURRENTTHEMENAME = HRESULT (WINAPI*)(LPWSTR, int, LPWSTR, int, LPWSTR, int);
            GETCURRENTTHEMENAME pfn = reinterpret_cast<GETCURRENTTHEMENAME>(
                reinterpret_cast<void*>(::GetProcAddress(theme, "GetCurrentThemeName")));
            pfn(nullptr, 0, themeName, 30, nullptr, 0);
        }

        return CString(themeName);
    }

    template <class T>
    BOOL CFrameT<T>::IsReBarSupported() const
    {
        TRACE("*** Warning: CFrameT::IsReBarSupported is deprecated. ***\n");
        return TRUE;
    }

    // Returns a reference to the view window.
    template <>
    inline CWnd& CFrameT<CDocker>::GetView() const
    {
        return CDocker::GetView();
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

                if (ERROR_SUCCESS == recentKey.QueryStringValue(fileKeyName, nullptr, &bufferSize))
                {
                    // load the entry from the registry.
                    int buffer = static_cast<int>(bufferSize);
                    if (ERROR_SUCCESS == recentKey.QueryStringValue(fileKeyName, pathName.GetBuffer(buffer), &bufferSize))
                    {
                        pathName.ReleaseBuffer();

                        if (pathName.GetLength() > 0)
                            mruEntries.push_back( pathName );
                    }
                    else
                    {
                        pathName.ReleaseBuffer();
                        TRACE(_T("\n*** WARNING: LoadRegistryMRUSettings: QueryStringValue failed. ***\n"));
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
        assert (keyName != nullptr);

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

                int l = static_cast<int>(left);
                int t = static_cast<int>(top);
                int r = static_cast<int>(left + width);
                int b = static_cast<int>(top + height);

                CPoint midpoint((l + r) / 2, (t + b) / 2);
                CPoint midtop((l + r)/2, t);

                HMONITOR monitor = ::MonitorFromPoint(midpoint, MONITOR_DEFAULTTONULL);
                if (monitor == nullptr)
                    throw CUserException();

                MONITORINFO mi{};
                mi.cbSize = sizeof(mi);
                ::GetMonitorInfo(monitor, &mi);
                CRect workArea = mi.rcWork;

                // Check if window is mostly within work area.
                if (!workArea.PtInRect(midpoint))
                    throw CUserException();

                // Check if the caption is within the work area.
                if (!workArea.PtInRect(midtop))
                    throw CUserException();

                if (width <= 0 || height <= 0)
                    throw CUserException();

                values.position = CRect(l, t, r, b);
                values.showCmd = (SW_MAXIMIZE == showCmd) ? SW_MAXIMIZE : SW_SHOW;
                values.showStatusBar = (statusBar & 1) ? TRUE : FALSE;
                values.showToolBar = (toolBar & 1) ? TRUE : FALSE;

                isOK = TRUE;
            }

            catch (const CUserException&)
            {
                TRACE("*** ERROR: Failed to load values from registry, using defaults. ***\n");

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
        MenuItemData* pMID = reinterpret_cast<MenuItemData*>(pMIS->itemData);
        assert(::IsMenu(pMID->menu));  // Does itemData contain a valid MenuItemData struct?

        // Get the font used in menu items.
        CClientDC dc(*this);
        LOGFONT lf = GetMenuFont().GetLogFont();

        // Default menu items are bold, so take this into account.
        if (static_cast<int>(::GetMenuDefaultItem(pMID->menu, TRUE, GMDI_USEDISABLED)) != -1)
            lf.lfWeight = FW_BOLD;

        dc.CreateFontIndirect(lf);
        CSize size = GetMenuMetrics().GetItemSize(pMID, dc);

        if (~pMID->mii.fType & MFT_SEPARATOR)  // if the type is not a separator.
        {
            // Account for icon height.
            int iconGap = T::DpiScaleInt(4);
            int sizeY = size.cy;
            size.cy = std::max(sizeY, GetMenuIconHeight() + iconGap);
        }

        // Fill the MEASUREITEMSTRUCT struct with the new size.
        pMIS->itemWidth = static_cast<UINT>(size.cx);
        pMIS->itemHeight = static_cast<UINT>(size.cy);
    }

    // Called when the frame is activated (WM_ACTIVATE received).
    template <class T>
    inline LRESULT CFrameT<T>::OnActivate(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        // Perform default processing first
        CWnd::WndProcDefault(msg, wparam, lparam);

        if (LOWORD(wparam) == WA_INACTIVE)
        {
            // Remove the keyboard hook used by the keyboard indicators.
            RemoveKbdHook();

            // Save the hwnd of the window that currently has focus.
            // This must be CFrame window itself or a child window.
            if (!T::IsIconic()) m_oldFocus = ::GetFocus();

            // Ensure no toolbar button is still hot.
            if (GetToolBar().IsWindow())
                GetToolBar().SendMessage(TB_SETHOTITEM, static_cast<WPARAM>(-1), 0);
        }
        else
        {
            // Set the keyboard hook by the keyboard indicators if
            // m_useIndicatorStatus is TRUE.
            SetKbdHook();

            // Now set the focus to the appropriate child window.
            if (m_oldFocus != nullptr) ::SetFocus(m_oldFocus);
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
        // Set the icon.
        T::SetIconLarge(IDW_MAIN);
        T::SetIconSmall(IDW_MAIN);

        // Set the keyboard accelerators.
        SetAccelerators(IDW_MAIN);

        // Set the Caption.
        SetTitle(LoadString(IDW_MAIN));

        // Set the theme for the frame elements.
        SetTheme();

        // Create the rebar and menubar
        if (IsUsingReBar())
        {
            // Create the rebar.
            GetReBar().Create(*this);

            // Create the menubar inside rebar.
            GetMenuBar().Create(GetReBar());
            AddMenuBarBand();

            // Disable XP themes for the menubar.
            GetMenuBar().SetWindowTheme(L" ", L" ");
        }

        // Setup the menu if IDW_MAIN menu resource is defined.
        CMenu menu(IDW_MAIN);
        if (::IsMenu(menu))
        {
            SetFrameMenu(menu);
            if (m_maxMRU > 0)
                UpdateMRUMenu();
        }
        else
            ShowMenu(FALSE);  // No menu if IDW_MAIN menu resource isn't defined.

        // Configure the menu metrics and fonts.
        ResetMenuMetrics();
        UpdateSettings();

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

        // Use SetView in CMainFrame's constructor to set the view window.
        assert(&GetView());

        // Create the view window.
        if (!GetView().IsWindow())
            GetView().Create(*this);
        GetView().SetFocus();

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

    // Called in response to a WM_DPICHANGED message that is sent to a top-level
    // window when the DPI changes. Only top-level windows receive a WM_DPICHANGED message.
    template <class T>
    inline LRESULT CFrameT<T>::OnDpiChanged(UINT, WPARAM, LPARAM lparam)
    {
        T::SetRedraw(FALSE);

        // Resize the frame, using the suggested new window size.
        RECT* const pWindowRect = reinterpret_cast<RECT*>(lparam);
        assert(pWindowRect);
        T::SetWindowPos(HWND_TOP, *pWindowRect, SWP_NOZORDER | SWP_NOACTIVATE);

        // Update the rebar, menubar and statusbar.
        ResetMenuMetrics();
        UpdateSettings();

        // Destroy and re-create the current toolbar.
        if (GetToolBar().IsWindow())
        {
            BOOL isToolbarShown = GetToolBar().IsWindowVisible();
            if (GetReBar().IsWindow())
            {
                int band = GetReBar().GetBand(GetToolBar());
                if (band >= 0)
                    GetReBar().DeleteBand(band);
            }

            GetToolBar().Destroy();
            CreateToolBar();    // CreateToolbar calls SetupToolBar.
            ShowToolBar(isToolbarShown);
        }

        // Update the menu icons.
        ClearMenuIcons();
        SetupMenuIcons();

        RecalcLayout();

        T::SetRedraw(TRUE);
        T::RedrawWindow();
        return 0;
    }

    // OwnerDraw is used to render the popup menu items.
    template <class T>
    inline LRESULT CFrameT<T>::OnDrawItem(UINT, WPARAM, LPARAM lparam)
    {
        LPDRAWITEMSTRUCT pDrawItem = (LPDRAWITEMSTRUCT)lparam;

        if (pDrawItem != nullptr)
        {
            // Some control types, such as status bars, do not set the value of CtlType.
            // We use hwndItem to detect the control.
            if (pDrawItem->hwndItem == GetStatusBar())
            {
                DrawStatusBar(pDrawItem);
            }
            else if (::IsMenu(reinterpret_cast<HMENU>(pDrawItem->hwndItem)))
            {
                DrawMenuItem(pDrawItem);
            }
        }

        return TRUE;
    }

    // Called when the Rebar's background is redrawn.
    template<class T>
    inline LRESULT CFrameT<T>::OnDrawRBBkgnd(UINT, WPARAM wparam, LPARAM lparam)
    {
        CDC* pDC = reinterpret_cast<CDC*>(wparam);
        assert(dynamic_cast<CDC*>(pDC));

        CReBar* pRebar = reinterpret_cast<CReBar*>(lparam);
        assert(dynamic_cast<CReBar*>(pRebar));

        if (pDC == nullptr || pRebar == nullptr)
            return 0;

        return DrawReBarBkgnd(*pDC, *pRebar);
    }

    // Called when the StatusBar's background is redrawn.
    template<class T>
    inline LRESULT CFrameT<T>::OnDrawSBBkgnd(UINT, WPARAM wparam, LPARAM lparam)
    {
        CDC* pDC = reinterpret_cast<CDC*>(wparam);
        assert(dynamic_cast<CDC*>(pDC));

        CStatusBar* pStatusbar = reinterpret_cast<CStatusBar*>(lparam);
        assert(dynamic_cast<CStatusBar*>(pStatusbar));

        if (pDC == nullptr || pStatusbar == nullptr)
            return 0;

        return DrawStatusBarBkgnd(*pDC, *pStatusbar);
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
        CMenu menu = reinterpret_cast<HMENU>(wparam);

        // The system menu shouldn't be owner drawn.
        if (HIWORD(lparam) || (T::GetSystemMenu() == menu))
            return CWnd::WndProcDefault(msg, wparam, lparam);

        if (IsUsingThemes())
        {
            MENUINFO mi{};
            mi.cbSize = sizeof(mi);
            mi.fMask = MIM_BACKGROUND | MIM_APPLYTOSUBMENUS;

            // Change the background brush color for dark menu.
            if (IsUsingDarkMenu())
                mi.hbrBack = static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
            else
                mi.hbrBack = 0;

            menu.SetMenuInfo(mi);
        }

        if (m_useOwnerDrawnMenu)
        {
            // A vector to store this menu's item data.
            MenuData menuData;

            bool hasTabs = false;

            for (int i = 0; i < menu.GetMenuItemCount(); ++i)
            {
                // The MenuItemData pointer is deleted in OnUnInitMenuPopup.
                MenuItemDataPtr itemDataPtr(std::make_unique<MenuItemData>());
                MENUITEMINFO mii{};
                mii.cbSize = sizeof(mii);

                // Use old fashioned MIIM_TYPE instead of MIIM_FTYPE for Win95 compatibility.
                mii.fMask = MIIM_TYPE | MIIM_DATA;
                mii.dwTypeData = itemDataPtr->itemText.GetBuffer(WXX_MAX_STRING_SIZE);
                mii.cch = WXX_MAX_STRING_SIZE;

                // Send message for menu updates.
                UINT menuItem = menu.GetMenuItemID(i);
                T::SendMessage(UWM_UPDATECOMMAND, menuItem, 0);

                // Specify owner-draw for the menu item type.
                UINT position = static_cast<UINT>(i);
                if (menu.GetMenuItemInfo(position, mii, TRUE))
                {
                    itemDataPtr->itemText.ReleaseBuffer();
                    if (itemDataPtr->itemText.Find(_T("\t")) >= 0)
                        hasTabs = true;

                    if (mii.dwItemData == 0)
                    {
                        itemDataPtr->menu = menu;
                        itemDataPtr->pos = position;
                        itemDataPtr->mii = mii;
                        mii.dwItemData = reinterpret_cast<ULONG_PTR>(itemDataPtr.get());
                        mii.fType |= MFT_OWNERDRAW;
                        menu.SetMenuItemInfo(position, mii, TRUE); // Store pItem in mii
                        menuData.push_back(std::move(itemDataPtr));
                    }
                }
            }

            // If one item has a tab, all should have a tab.
            if (hasTabs)
            {
                UINT itemCount = static_cast<UINT>(menu.GetMenuItemCount());
                for (UINT u = 0; u < itemCount; ++u)
                {
                    MenuItemData* pData = reinterpret_cast<MenuItemData*>(menu.GetMenuItemData(u, TRUE));
                    if (pData && pData->itemText.Find(_T('\t')) < 0)
                        pData->itemText += _T('\t'); //  Append a tab
                }
            }

            // m_menuItemIDs can store the menu item data for multiple popup menus.
            // There will be multiple popup menus if submenus are opened.
            if (menuData.size() != 0)
                m_menusData.push_back(std::move(menuData));
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
            bool keyState = (lparam & 0x80000000) != 0;
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
            GetMenuBar().MenuChar(msg, wparam, lparam);
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
            UINT id = LOWORD (wparam);
            HMENU menu = reinterpret_cast<HMENU>(lparam);

            if ((menu != T::GetMenu()) && (id != 0) && !(HIWORD(wparam) & MF_POPUP))
                GetStatusBar().SetPartText(0, LoadString(id), SBT_OWNERDRAW);
            else
                GetStatusBar().SetPartText(0, m_statusText, SBT_OWNERDRAW);
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
                bool isWindow = (GetStatusBar().IsWindow() != 0);  // != 0 converts BOOL to bool.
                bool isVisible = GetStatusBar().IsWindow() && GetStatusBar().IsWindowVisible();
                if (isWindow)
                    GetFrameMenu().EnableMenuItem(id, MF_ENABLED);
                else
                    GetFrameMenu().EnableMenuItem(id, MF_DISABLED);

                if (isVisible)
                    GetFrameMenu().CheckMenuItem(id, MF_CHECKED);
                else
                    GetFrameMenu().CheckMenuItem(id, MF_UNCHECKED);
            }
            break;
        case IDW_VIEW_TOOLBAR:
            {
                bool isWindow = (GetToolBar().IsWindow() != 0);  // != 0 converts BOOL to bool.
                bool isVisible = GetToolBar().IsWindow() && GetToolBar().IsWindowVisible();
                if (isWindow)
                    GetFrameMenu().EnableMenuItem(id, MF_ENABLED);
                else
                    GetFrameMenu().EnableMenuItem(id, MF_DISABLED);

                if (isVisible)
                    GetFrameMenu().CheckMenuItem(id, MF_CHECKED);
                else
                    GetFrameMenu().CheckMenuItem(id, MF_UNCHECKED);
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
        case static_cast<UINT>(NM_CUSTOMDRAW): return OnCustomDraw(pHeader);   // The UINT cast is required by some 32bit MinGW compilers.
        case RBN_HEIGHTCHANGE:    return OnRBNHeightChange(pHeader);
        case RBN_LAYOUTCHANGED:   return OnRBNLayoutChanged(pHeader);
        case RBN_MINMAX:          return OnRBNMinMax(pHeader);
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

    // Tool tip notification from the toolbar.
    template <class T>
    inline LRESULT CFrameT<T>::OnTTNGetDispInfo(LPNMTTDISPINFO pNMTDI)
    {
        // Find the ToolBar that generated the tooltip
        CPoint pt(static_cast<LPARAM>(GetMessagePos()));
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
                UINT id = static_cast<UINT>(pToolBar->GetCommandID(index));
                if (id != 0)
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

    // Called when the SystemParametersInfo function changes a system-wide
    // setting or when policy settings have changed.
    template <class T>
    inline LRESULT CFrameT<T>::OnSettingChange(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        OnSysColorChange(msg, wparam, lparam);
        UpdateSettings();
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
            for (int band = 0; band < GetReBar().GetBandCount(); ++band)
            {
                GetReBar().SetBandColor(band, GetSysColor(COLOR_BTNTEXT), GetSysColor(COLOR_BTNFACE));
            }
        }

        UpdateSettings();
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
        // The "wparam & 0xFFF0" below is a requirement mentioned in the
        // description of WM_SYSCOMMAND in the Windows API documentation.

        if ((SC_KEYMENU == (wparam & 0xFFF0)) && (VK_SPACE != lparam) && GetMenuBar().IsWindow())
        {
            GetMenuBar().SysCommand(msg, wparam, lparam);
            return 0;
        }

        if (SC_MINIMIZE == (wparam & 0xFFF0))
            m_oldFocus = ::GetFocus();

        // Pass remaining system commands on for default processing.
        return CWnd::WndProcDefault(msg, wparam, lparam);
    }

    // Called in response to a WM_THEMECHANGED message. This message is sent
    // to all top-level windows following a theme change event.
    template <class T>
    inline LRESULT CFrameT<T>::OnThemeChanged(UINT, WPARAM, LPARAM)
    {
        // Reset the menu metrics on theme change.
        ResetMenuMetrics();

        // Update the menu icons.
        SetupMenuIcons();

        return 0;
    }

    // Notification of undocked from CDocker received via OnNotify
    template <class T>
    inline LRESULT CFrameT<T>::OnUndocked()
    {
        m_oldFocus = nullptr;
        return 0;
    }

    // Called when the drop-down menu or submenu has been destroyed.
    // Win95 & WinNT don't support the WM_UNINITMENUPOPUP message.
    template <class T>
    inline LRESULT CFrameT<T>::OnUnInitMenuPopup(UINT, WPARAM wparam, LPARAM)
    {
        CMenu menu = reinterpret_cast<HMENU>(wparam);
        bool doPopBack = false;

        for (int i = 0; i < menu.GetMenuItemCount(); i++)
        {
            UINT position = static_cast<UINT>(i);
            MENUITEMINFO mii{};
            mii.cbSize = sizeof(mii);
            mii.fMask = MIIM_STATE | MIIM_ID | MIIM_SUBMENU | MIIM_CHECKMARKS | MIIM_TYPE | MIIM_DATA;
            menu.GetMenuItemInfo(position, mii, TRUE);

            MenuItemData* pItemData = reinterpret_cast<MenuItemData*>(mii.dwItemData);
            if (pItemData != nullptr)
            {
                mii.fType = pItemData->mii.fType;
                mii.dwTypeData = const_cast<LPTSTR>(pItemData->itemText.c_str());
                mii.cch = static_cast<UINT>(pItemData->itemText.GetLength());
                mii.dwItemData = 0;
                VERIFY(menu.SetMenuItemInfo(position, mii, TRUE));
                doPopBack = true;
            }
        }

        // Release the memory allocated for all the menu items for this popup menu.
        if (doPopBack)
            m_menusData.pop_back();

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
        if (GetInitValues().showCmd == SW_MAXIMIZE)
            cs.style |= WS_MAXIMIZE;

        // Set the original window position.
        CRect initPos = GetInitValues().position;
        if ((initPos.Width() > 0) && (initPos.Height()) > 0)
        {
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
            VERIFY(GetStatusBar().SetWindowPos(HWND_TOP, 0, 0, 0, 0, SWP_SHOWWINDOW));

            SetStatusParts();
        }

        // Resize the rebar or toolbar.
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
        VERIFY(GetView().SetWindowPos(HWND_TOP, GetViewRect(), SWP_SHOWWINDOW));
    }

    // Removes an entry from the MRU list.
    template <class T>
    inline void CFrameT<T>::RemoveMRUEntry(LPCTSTR MRUEntry)
    {
        for (auto it = m_mruEntries.begin(); it != m_mruEntries.end(); ++it)
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
                for (size_t i = 0; i < m_maxMRU; ++i)
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
            TRACE("*** ERROR: Failed to save registry MRU settings. ***\n");

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
                WINDOWPLACEMENT wndpl{};
                wndpl.length = sizeof(wndpl);

                if (T::GetWindowPlacement(wndpl))
                {
                    // Get the Frame's window position
                    CRect rc = wndpl.rcNormalPosition;
                    DWORD top = static_cast<DWORD>(rc.top);
                    DWORD left = static_cast<DWORD>(rc.left);
                    DWORD width = static_cast<DWORD>(rc.Width());
                    DWORD height = static_cast<DWORD>(rc.Height());
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
                TRACE("*** ERROR: Failed to save registry settings. ***\n");

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

    // Removes the keyboard hook installed by SetKbdHook. The hook is used to
    // display the keyboard indicator status for the CAPs lock, NUM lock,
    // Scroll lock and Insert keys.
    template<class T>
    inline void CFrameT<T>::RemoveKbdHook()
    {
        if (m_kbdHook != nullptr)
            ::UnhookWindowsHookEx(m_kbdHook);

        m_kbdHook = nullptr;
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
    inline void CFrameT<T>::SetFrameMenu(CMenu menu)
    {
        m_menu = menu;

        if (GetMenuBar().IsWindow())
        {
            GetMenuBar().SetupMenuBar( GetFrameMenu() );
            BOOL show = (menu.GetHandle())? TRUE : FALSE;
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

    // Installs a keyboard hook if m_useIndicatorStatus is true. The
    // hook is used to display the keyboard indicator status for the
    // CAPs lock, NUM lock, Scroll lock and Insert keys.
    template <class T>
    inline void CFrameT<T>::SetKbdHook()
    {
        if (m_useIndicatorStatus)
        {
            GetApp()->SetMainWnd(*this);
            m_kbdHook = ::SetWindowsHookEx(WH_KEYBOARD, StaticKeyboardProc, 0, ::GetCurrentThreadId());
            SetStatusIndicators();
        }
    }

    // Sets the menu icons. Any previous menu icons are removed.
    template <class T>
    inline UINT CFrameT<T>::SetMenuIcons(const std::vector<UINT>& menuData, COLORREF mask, UINT toolBarID, UINT)
    {
        // Remove any existing menu icons.
        ClearMenuIcons();

        // Exit if no ToolBarID is specified.
        if (toolBarID == 0) return 0;

        // Add the menu icons from the bitmap IDs.
        return AddMenuIcons(menuData, mask, toolBarID, 0);
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

            REBARBANDINFO rbbi{};
            rbbi.fMask = RBBIM_CHILDSIZE | RBBIM_SIZE;
            rb.GetBandInfo(band, rbbi);

            int width;
            if ((GetReBarTheme().UseThemes) && (GetReBarTheme().LockMenuBand))
                width = rcClient.Width() - rcBorder.Width() - 2;
            else
                width = GetMenuBar().GetMaxSize().cx;

            rbbi.cxMinChild = static_cast<UINT>(width);
            rbbi.cx         = static_cast<UINT>(width);

            rb.SetBandInfo(band, rbbi);
        }
    }

    // Sets the font used by the menubar and popup menu items.
    template <class T>
    inline void CFrameT<T>::SetMenuFont(HFONT font)
    {
        m_menuFont = font;

        if (GetMenuBar().IsWindow())
        {
            GetMenuBar().SetFont(font);
            SetFrameMenu(GetFrameMenu());
            UpdateMenuBarBandSize();
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
        // Remove any excess MRU entries.
        if (mruLimit < m_mruEntries.size())
        {
            m_mruEntries.erase(m_mruEntries.begin() + static_cast<int>(mruLimit), m_mruEntries.end());
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

    // Sets the font used by the status bar.
    template <class T>
    inline void CFrameT<T>::SetStatusBarFont(HFONT font)
    {
        m_statusBarFont = font;

        if (GetStatusBar().IsWindow())
            GetStatusBar().SetFont(font);
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
            CString empty;

            CString old0 = m_indicators[0];
            CString old1 = m_indicators[1];
            CString old2 = m_indicators[2];

            m_indicators[0] = (::GetKeyState(VK_CAPITAL) & 0x0001) ? cap : empty;
            m_indicators[1] = (::GetKeyState(VK_NUMLOCK) & 0x0001) ? num : empty;
            m_indicators[2] = (::GetKeyState(VK_SCROLL) & 0x0001) ? scrl : empty;

            // Update the indicators text.
            // We need member variables for owner drawn text to keep the text in scope.
            // Only set the indicator if it changed.
            if (old0 != m_indicators[0])
                GetStatusBar().SetPartText(1, m_indicators[0], SBT_OWNERDRAW);
            if (old1 != m_indicators[1])
                GetStatusBar().SetPartText(2, m_indicators[1], SBT_OWNERDRAW);
            if (old2 != m_indicators[2])
                GetStatusBar().SetPartText(3, m_indicators[2], SBT_OWNERDRAW);
        }
    }

    // Creates and resizes the 4 parts in the status bar.
    template <class T>
    inline void CFrameT<T>::SetStatusParts()
    {
        if (IsUsingIndicatorStatus() && GetStatusBar().IsWindow())
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

            bool hasGripper = (GetStatusBar().GetStyle() & SBARS_SIZEGRIP) != 0;
            int cxGripper = hasGripper ? 20 : 0;
            int cxBorder = 8;

            // Adjust for DPI aware.
            int dpi = GetWindowDpi(*this);
            cxGripper = MulDiv(cxGripper, dpi, USER_DEFAULT_SCREEN_DPI);
            capSize.cx += cxBorder;
            numSize.cx += cxBorder;
            scrlSize.cx += cxBorder;

            // Get the coordinates of the window's client area.
            CRect clientRect = T::GetClientRect();
            int clientRight = clientRect.right;
            int width = std::max(300, clientRight);

            // Create 4 panes.
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
            GetStatusBar().SetPartText(0, m_statusText, SBT_OWNERDRAW);
        }
    }

    // Sets the theme colors for the frame's rebar, toolbar and menubar.
    // Override this function to modify the theme colors.
    template <class T>
    inline void CFrameT<T>::SetTheme()
    {
        // Avoid themes if using less than 16 bit colors.
        CClientDC DesktopDC(*this);
        if (DesktopDC.GetDeviceCaps(BITSPIXEL) < 16)
            UseThemes(FALSE);

        BOOL t = TRUE;
        BOOL f = FALSE;

        if (IsUsingThemes())
        {
            // Retrieve the XP theme name.
            CString xpThemeName = GetXPThemeName();

            // Predifined themes.
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
            case Win8:  // A pale blue scheme without gradients, suitable for Windows 8, 8.1, 10 and 11.
                {
                    MenuTheme mt = {t, RGB(180, 250, 255), RGB(140, 190, 255), RGB(240, 250, 255), RGB(120, 170, 220), RGB(127, 127, 255), RGB(0, 0, 0) };
                    ReBarTheme rbt = {t, RGB(235, 237, 250), RGB(235, 237, 250), RGB(235, 237, 250), RGB(235, 237, 250), f, t, t, f, t, f };
                    StatusBarTheme sbt = {t, RGB(235, 237, 250), RGB(235, 237, 250), RGB(0, 0, 0) };
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
                    StatusBarTheme sbt = {t, RGB(225, 230, 255), RGB(240, 242, 250), RGB(0, 0, 0) };
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
                    StatusBarTheme sbt = {t, RGB(150,190,245), RGB(196,215,250), RGB(0, 0, 0) };
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
                    StatusBarTheme sbt = {t, RGB(225, 220, 240), RGB(240, 240, 245), RGB(0, 0, 0) };
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
                    StatusBarTheme sbt = {t, RGB(215, 216, 182), RGB(242, 242, 230), RGB(0, 0, 0) };
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
                    StatusBarTheme sbt = {t, RGB(212, 208, 200), RGB(212, 208, 200), RGB(0, 0, 0) };
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

    // Sets the Image List for additional toolbars.
    template <class T>
    inline void CFrameT<T>::SetTBImageList(CToolBar& toolBar, UINT id, COLORREF mask)
    {
        // Get the image size.
        CBitmap bm(id);

        // Assert if we failed to load the bitmap.
        assert(bm.GetHandle() != nullptr);

        // Scale the bitmap to the window's DPI.
        CBitmap dpiImage = T::DpiScaleUpBitmap(bm);
        CSize sz = GetTBImageSize(&dpiImage);

        // Set the toolbar's image list.
        CImageList imageList;
        imageList.Create(sz.cx, sz.cy, ILC_COLOR32 | ILC_MASK, 0, 0);
        imageList.Add(dpiImage, mask);
        toolBar.SetImageList(imageList);

        // Inform the Rebar of the change to the Toolbar.
        if (GetReBar().IsWindow())
        {
            SIZE maxSize = toolBar.GetMaxSize();
            WPARAM wparam = reinterpret_cast<WPARAM>(toolBar.GetHwnd());
            LPARAM lparam = reinterpret_cast<LPARAM>(&maxSize);
            GetReBar().SendMessage(UWM_TBRESIZE, wparam, lparam);
        }
    }

    // Sets the Disabled Image List for additional toolbars.
    template <class T>
    inline void CFrameT<T>::SetTBImageListDis(CToolBar& toolBar, UINT id, COLORREF mask)
    {
        // Get the image size.
        CBitmap bm(id);

        // Assert if we failed to load the bitmap.
        assert(bm.GetHandle() != nullptr);

        // Scale the bitmap to the window's DPI.
        CBitmap dpiImage = T::DpiScaleUpBitmap(bm);
        CSize sz = GetTBImageSize(&dpiImage);

        // Set the toolbar's disabled image list.
        CImageList imageList;
        imageList.Create(sz.cx, sz.cy, ILC_COLOR32 | ILC_MASK, 0, 0);
        imageList.Add(dpiImage, mask);
        toolBar.SetDisableImageList(imageList);

        // Inform the Rebar of the change to the Toolbar.
        if (GetReBar().IsWindow())
        {
            SIZE maxSize = toolBar.GetMaxSize();
            WPARAM wparam = reinterpret_cast<WPARAM>(toolBar.GetHwnd());
            LPARAM lparam = reinterpret_cast<LPARAM>(&maxSize);
            GetReBar().SendMessage(UWM_TBRESIZE, wparam, lparam);
        }
    }

    // Sets the Hot Image List for additional toolbars.
    template <class T>
    inline void CFrameT<T>::SetTBImageListHot(CToolBar& toolBar, UINT id, COLORREF mask)
    {
        // Get the image size.
        CBitmap bm(id);

        // Assert if we failed to load the bitmap.
        assert(bm.GetHandle() != nullptr);

        // Scale the bitmap to the window's DPI.
        CBitmap dpiImage = T::DpiScaleUpBitmap(bm);
        CSize sz = GetTBImageSize(&dpiImage);

        // Set the toolbar's hot image list
        CImageList imageList;
        imageList.Create(sz.cx, sz.cy, ILC_COLOR32 | ILC_MASK, 0, 0);
        imageList.Add(dpiImage, mask);
        toolBar.SetHotImageList(imageList);

        // Inform the Rebar of the change to the Toolbar.
        if (GetReBar().IsWindow())
        {
            SIZE MaxSize = toolBar.GetMaxSize();
            WPARAM wparam = reinterpret_cast<WPARAM>(toolBar.GetHwnd());
            LPARAM lparam = reinterpret_cast<LPARAM>(&MaxSize);
            GetReBar().SendMessage(UWM_TBRESIZE, wparam, lparam);
        }
    }

    // Either sets the imagelist or adds/replaces bitmap depending on ComCtl32.dll version
    // The ToolBarIDs are bitmap resources containing a set of toolbar button images.
    // Each toolbar button image must have a minimum height of 16. Its height must equal its width.
    // The color mask is ignored for 32bit bitmaps, but is required for 24bit bitmaps
    // The color mask is often gray RGB(192,192,192) or magenta (255,0,255)
    // The Hot and disabled bitmap resources can be 0.
    // A Disabled image list is created from toolBarID if one isn't provided.
    template <class T>
    inline void CFrameT<T>::SetToolBarImages(CToolBar& toolbar, COLORREF mask, UINT toolBarID, UINT toolBarHotID, UINT toolBarDisabledID)
    {
        // Set the normal imagelist.
        SetTBImageList(toolbar, toolBarID, mask);

        // Set the hot imagelist.
        if (toolBarHotID != 0)
        {
            SetTBImageListHot(toolbar, toolBarHotID, mask);
        }

        if (toolBarDisabledID != 0)
        {
            SetTBImageListDis(toolbar, toolBarDisabledID, mask);
        }
        else
        {
            CImageList toolBarDisabledImages;
            toolBarDisabledImages.CreateDisabledImageList(toolbar.GetImageList());
            toolbar.SetDisableImageList(toolBarDisabledImages);
        }
    }

    // Either sets the imagelist or adds/replaces bitmap depending on ComCtl32.dll version
    // The ToolBarIDs are bitmap resources containing a set of toolbar button images.
    // Each toolbar button image must have a minimum height of 16. Its height must equal its width.
    // The color mask is ignored for 32bit bitmaps, but is required for 24bit bitmaps
    // The color mask is often gray RGB(192,192,192) or magenta (255,0,255)
    // The Hot and disabled bitmap resources can be 0.
    // A Disabled image list is created from toolBarID if one isn't provided.
    template <class T>
    inline void CFrameT<T>::SetToolBarImages(COLORREF mask, UINT toolBarID, UINT toolBarHotID, UINT toolBarDisabledID)
    {
        SetToolBarImages(GetToolBar(), mask, toolBarID, toolBarHotID, toolBarDisabledID);
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
            // Add the icons for popup menu.
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
    template <>
    inline void CFrameT<CDocker>::SetView(CWnd& wndView)
    {
        CDocker::SetView(wndView);
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
                VERIFY(GetView().SetWindowPos(HWND_TOP, rc, SWP_SHOWWINDOW));
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
                T::SetMenu(nullptr);
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
    }

    // Called by the keyboard hook to update status information.
    template <class T>
    inline LRESULT CALLBACK CFrameT<T>::StaticKeyboardProc(int code, WPARAM wparam, LPARAM lparam)
    {
        HWND frame = GetApp()->GetMainWnd();
        CFrameT<T>* pFrame = static_cast< CFrameT<T>* >(CWnd::GetCWndPtr(frame));
        assert(dynamic_cast<CFrameT<T>*>(pFrame) != nullptr);

        if (pFrame != nullptr)
        {
            pFrame->OnKeyboardHook(code, wparam, lparam);

            // The HHOOK parameter is used in CallNextHookEx for Win95, Win98 and WinME.
            // The HHOOK parameter is ignored for Windows NT and above.
            return ::CallNextHookEx(pFrame->m_kbdHook, code, wparam, lparam);
        }

        return ::CallNextHookEx(0, code, wparam, lparam);
    }

    // Update the MenuBar band size.
    template <class T>
    inline void CFrameT<T>::UpdateMenuBarBandSize()
    {
        int band = GetReBar().GetBand(GetMenuBar());
        if (band >= 0)
        {
            CSize sizeMenuBar = GetMenuBar().GetMaxSize();
            GetReBar().ResizeBand(band, sizeMenuBar);
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
        MENUITEMINFO mii{};
        mii.cbSize = sizeof(mii);

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
            for (int item = maxMRUIndex; item >= 0; --item)
            {
                size_t index = static_cast<size_t>(item);
                UINT pos = static_cast<UINT>(item);
                mii.fMask = MIIM_TYPE | MIIM_ID | MIIM_STATE;
                mii.fState = (m_mruEntries.size() == 0)? MFS_GRAYED : 0U;
                mii.fType = MFT_STRING;
                mii.wID = IDW_FILE_MRU_FILE1 + pos;
                mii.dwTypeData = const_cast<LPTSTR>(mruStrings[index].c_str());

                if (item == maxMRUIndex)
                    // Replace the last MRU entry first.
                    VERIFY(fileMenu.SetMenuItemInfo(IDW_FILE_MRU_FILE1, mii, FALSE));
                else
                    // Insert the other MRU entries next.
                    VERIFY(fileMenu.InsertMenuItem(IDW_FILE_MRU_FILE1 + pos + 1, mii, FALSE));
            }
        }

        T::DrawMenuBar();
    }

    // Updates the settings for the rebar, menubar and status bar to
    // account for changes in the frame window's DPI.
    template <class T>
    inline void CFrameT<T>::UpdateSettings()
    {
        // Create the menubar and statusbar fonts.
        int dpi = GetWindowDpi(*this);
        NONCLIENTMETRICS metrics = GetNonClientMetrics();
        metrics.lfMenuFont.lfHeight = -MulDiv(9, dpi, POINTS_PER_INCH);
        metrics.lfStatusFont.lfHeight = -MulDiv(9, dpi, POINTS_PER_INCH);
        SetMenuFont(CFont(metrics.lfMenuFont));
        SetStatusBarFont(CFont(metrics.lfStatusFont));
    }

    // Provides default processing of the frame window's messages.
    // The frame's WndProc function should pass unhandled window messages to this function.
    template <class T>
    inline LRESULT CFrameT<T>::WndProcDefault(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        switch (msg)
        {
        case WM_ACTIVATE:       return OnActivate(msg, wparam, lparam);
        case WM_DPICHANGED:     return OnDpiChanged(msg, wparam, lparam);
        case WM_THEMECHANGED:   return OnThemeChanged(msg, wparam, lparam);
        case WM_DRAWITEM:       return OnDrawItem(msg, wparam, lparam);
        case WM_ERASEBKGND:     return 0;
        case WM_HELP:           return OnHelp();
        case WM_INITMENUPOPUP:  return OnInitMenuPopup(msg, wparam, lparam);
        case WM_MENUCHAR:       return OnMenuChar(msg, wparam, lparam);
        case WM_MEASUREITEM:    return OnMeasureItem(msg, wparam, lparam);
        case WM_MENUSELECT:     return OnMenuSelect(msg, wparam, lparam);
        case WM_SETTINGCHANGE:  return OnSettingChange(msg, wparam, lparam);
        case WM_SIZE:           return OnSize(msg, wparam, lparam);
        case WM_SYSCOLORCHANGE: return OnSysColorChange(msg, wparam, lparam);
        case WM_SYSCOMMAND:     return OnSysCommand(msg, wparam, lparam);
        case WM_UNINITMENUPOPUP:  return OnUnInitMenuPopup(msg, wparam, lparam);

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

#endif // _WIN32XX_FRAME_H_
