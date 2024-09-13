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


///////////////////////////////////////////////////////
// wxx_tab.h
//  Declaration of the CTab and CMDITab classes.

#ifndef _WIN32XX_TAB_H_
#define _WIN32XX_TAB_H_

#include "wxx_wincore.h"
#include "wxx_dialog.h"
#include "wxx_regkey.h"
#include "default_resource.h"

namespace Win32xx
{

    // This struct holds the information for each tab page.
    struct TabPageInfo
    {
        CString tabText;    // The tab's text
        HICON tabIcon;      // The tab's icon
        int tabImage;       // index of this tab's image
        int tabID;          // identifier for this tab (used by TabbedMDI)
        CWnd* pView;        // pointer to the view window
        TabPageInfo() : tabIcon(nullptr), tabImage(0), tabID(0), pView(nullptr) {}    // constructor
    };

    struct TABNMHDR
    {
        NMHDR hdr;
        int page;
    };


    //////////////////////////////////////////////////////////////
    // CTab manages a tab control. A tab control is analogous to
    // the dividers in a notebook or the labels in a file cabinet.
    // By using a tab control, an application can define multiple
    // pages for the same area of a window or dialog box.
    class CTab : public CWnd
    {
    protected:
        // Declaration of the CSelectDialog class, a nested class of CTab.
        // It creates the dialog to choose which tab to activate.
        // It is used when the tab control has 9 or more tabs.
        class CSelectDialog : public CDialog
        {
        public:
            CSelectDialog(LPCDLGTEMPLATE pDlgTemplate);
            virtual ~CSelectDialog() override {}
            void AddItem(LPCTSTR string);

        protected:
            virtual BOOL OnInitDialog() override;
            virtual void OnOK() override;
            virtual void OnCancel() override { EndDialog(-2); }

        private:
            CSelectDialog(const CSelectDialog&) = delete;
            CSelectDialog& operator=(const CSelectDialog&) = delete;

            std::vector<CString> m_items;
            UINT IDC_LIST;
        };

    public:
        CTab();
        virtual ~CTab() override;
        virtual CWnd*  AddTabPage(CWnd* pView, LPCTSTR tabText, HICON icon, int tabID);
        virtual CWnd*  AddTabPage(CWnd* pView, LPCTSTR tabText, UINT iconID, int tabID = 0);
        virtual CWnd*  AddTabPage(CWnd* pView, LPCTSTR tabText);
        virtual CWnd*  AddTabPage(WndPtr view, LPCTSTR tabText, HICON icon, int tabID);
        virtual CWnd*  AddTabPage(WndPtr view, LPCTSTR tabText, UINT iconID, int tabID = 0);
        virtual CWnd*  AddTabPage(WndPtr view, LPCTSTR tabText);
        virtual void   SelectPage(int page);
        virtual void   RecalcLayout();
        virtual void   RemoveTabPage(int page);
        virtual void   SetTabsFontSize(int fontSize = 9);
        virtual void   SetTabsIconSize(int iconSize = 16);
        virtual void   SwapTabs(int tab1, int tab2);
        virtual void   UpdateTabs();

        // Accessors
        CWnd* GetActiveView() const         { return m_pActiveView; }
        const std::vector<TabPageInfo>& GetAllTabs() const { return m_allTabPageInfo; }
        CRect GetCloseRect() const;
        CMenu GetListMenu();
        CRect GetListRect() const;
        SIZE GetMaxTabSize() const;
        CImageList& GetImages()             { return m_images; }
        BOOL GetShowButtons() const         { return m_isShowingButtons; }
        BOOL GetTabsAtTop() const;
        CFont GetTabFont() const            { return m_tabFont; }
        int GetTabHeight() const;
        int GetTabImageID(int tab) const;
        int GetTabIndex(CWnd* pWnd) const;
        TabPageInfo GetTabPageInfo(int tab) const;
        CString GetTabText(int tab) const;
        int GetTextHeight() const;

        //  Mutators
        void SetBlankPageColor(COLORREF color)          { m_blankPageColor = color; }
        void SetFixedWidth(BOOL isEnabled);
        void SetListDialog(LPCDLGTEMPLATE pDlgTemplate) { m_pDlgTemplate = pDlgTemplate; }
        void SetOwnerDraw(BOOL isEnabled);
        void SetShowButtons(BOOL show);
        void SetTabFont(HFONT font);
        void SetTabIcon(int tab, UINT iconID);
        void SetTabIcon(int tab, HICON icon);
        void SetTabsAtTop(BOOL isAtTop);
        void SetTabText(int tab, LPCTSTR text);

        // State functions
        void ShowListDialog();
        void ShowListMenu();

        // Wrappers for Win32 Macros
        void        AdjustRect(BOOL isLarger, RECT* prc) const;
        BOOL        DeleteAllItems() const;
        BOOL        DeleteItem(int tab) const;
        void        DeselectAll(UINT excludeFocus) const;
        int         GetCurFocus() const;
        int         GetCurSel() const;
        DWORD       GetExtendedStyle() const;
        CImageList  GetImageList() const;
        BOOL        GetItem(int tab, LPTCITEM pTabInfo) const;
        int         GetItemCount() const;
        BOOL        GetItemRect(int tab, RECT& rc) const;
        int         GetRowCount() const;
        HWND        GetToolTips() const;
        BOOL        HighlightItem(int tabID, WORD highlight) const;
        int         HitTest(TCHITTESTINFO& hitInfo) const;
        int         InsertItem(int tab, const LPTCITEM pTabInfo) const;
        void        RemoveImage(int image) const;
        void        SetCurFocus(int tab) const;
        int         SetCurSel(int tab) const;
        DWORD       SetExtendedStyle(DWORD exStyle) const;
        CImageList  SetImageList(HIMAGELIST newImages);
        BOOL        SetItem(int tab, LPTCITEM pTabInfo) const;
        BOOL        SetItemExtra(int cb) const;
        DWORD       SetItemSize(int cx, int cy) const;
        int         SetMinTabWidth(int cx) const;
        void        SetPadding(int cx, int cy) const;
        void        SetToolTips(HWND toolTip) const;

    protected:
        virtual void    DrawCloseButton(CDC& dc);
        virtual void    DrawListButton(CDC& dc);
        virtual void    DrawTabs(CDC& dc);
        virtual void    DrawTabBorders(CDC& dc, RECT& rc);
        virtual void    OnAttach() override;
        virtual void    OnDestroy() override;
        virtual BOOL    OnEraseBkgnd(CDC&) override { return TRUE;}
        virtual LRESULT OnEraseBkgnd(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual LRESULT OnLButtonDblClk(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual LRESULT OnLButtonDown(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual LRESULT OnLButtonUp(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual LRESULT OnMouseLeave(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual LRESULT OnMouseMove(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual LRESULT OnKillFocus(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual LRESULT OnPaint(UINT msg, WPARAM wparam, LPARAM lparam) override;
        virtual LRESULT OnNCHitTest(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual LRESULT OnNotifyReflect(WPARAM wparam, LPARAM lparam) override;
        virtual LRESULT OnSetFocus(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual LRESULT OnTCNSelChange(LPNMHDR pNMHDR);
        virtual LRESULT OnDpiChangedAfterParent(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual LRESULT OnWindowPosChanged(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual LRESULT OnWindowPosChanging(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual void    NotifyChanged();
        virtual void    NotifyDragged();
        virtual BOOL    NotifyTabClosing(int page);
        virtual void    Paint();
        virtual void    PreCreate(CREATESTRUCT& cs) override;
        virtual void    PreRegisterClass(WNDCLASS& wc) override;
        virtual void    SetTabSize();

        // Not intended to be overridden
        LRESULT WndProcDefault(UINT msg, WPARAM wparam, LPARAM lparam) override;

    private:
        CTab(const CTab&) = delete;
        CTab& operator=(const CTab&) = delete;

        void ShowActiveView(CWnd* pView);
        void UpdateImageList();

        std::vector<TabPageInfo> m_allTabPageInfo;
        std::vector<WndPtr> m_tabViews;
        CFont m_tabFont;                // Font used for tab text with owner draw.
        CImageList m_images;            // Image-list for the tab control.
        LPCDLGTEMPLATE m_pDlgTemplate;  // Dialog template for the list dialog.
        CWnd* m_pActiveView;
        CPoint m_oldMousePos;
        CMenu m_listMenu;
        BOOL m_isShowingButtons;        // Show or hide the close and list button.
        BOOL m_isTracking;
        BOOL m_isClosePressed;
        BOOL m_isListPressed;
        BOOL m_isListMenuActive;
        COLORREF m_blankPageColor;
    };

    ////////////////////////////////////////////////////////////////////
    // The CTabbedMDI class combines many of the features of a MDI Frame
    // and a tab control. Each MDI child is displayed as a separate tab
    // page.
    class CTabbedMDI : public CWnd
    {
    public:
        CTabbedMDI();
        virtual ~CTabbedMDI() override;

        virtual CWnd* AddMDIChild(CWnd* pView, LPCTSTR tabText, int mdiChildID = 0);
        virtual CWnd* AddMDIChild(WndPtr view, LPCTSTR tabText, int mdiChildID = 0);
        virtual void  CloseActiveMDI();
        virtual void  CloseAllMDIChildren();
        virtual void  CloseMDIChild(int tab);
        virtual HWND  Create(HWND hWndParent) override;
        virtual BOOL  LoadRegistrySettings(LPCTSTR keyName);
        virtual BOOL  SaveRegistrySettings(LPCTSTR keyName);
        virtual void  ShowListDialog() { GetTab().ShowListDialog(); }

        // Accessors and mutators
        CWnd*   GetActiveMDIChild() const;
        int     GetActiveMDITab() const;
        CWnd*   GetMDIChild(int tab) const;
        int     GetMDIChildCount() const;
        int     GetMDIChildID(int tab) const;
        LPCTSTR GetMDIChildTitle(int tab) const;
        CMenu   GetListMenu() const { return GetTab().GetListMenu(); }
        CTab&   GetTab() const { return *m_pTab; }
        void    SetActiveMDIChild(CWnd* pWnd) const;
        void    SetActiveMDITab(int tab) const;
        void    SetTab(CTab& tab) { m_pTab = &tab; }

    protected:
        virtual WndPtr  NewMDIChildFromID(int mdiChildID);
        virtual void    OnAttach() override;
        virtual void    OnDestroy() override;
        virtual LRESULT OnNotify(WPARAM wparam, LPARAM lparam) override;
        virtual LRESULT OnSetFocus(UINT, WPARAM, LPARAM);
        virtual BOOL    OnTabClose(int tab);
        virtual LRESULT OnWindowPosChanged(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual void    RecalcLayout();

        // Not intended to be overwritten
        LRESULT WndProcDefault(UINT msg, WPARAM wparam, LPARAM lparam) override;

    private:
        CTabbedMDI(const CTabbedMDI&) = delete;
        CTabbedMDI& operator=(const CTabbedMDI&) = delete;

        CTab m_tab;
        CTab* m_pTab;
    };

}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


namespace Win32xx
{

    //////////////////////////////////////////////////////////////
    // Definitions for the CSelectDialog class nested within CTab.
    //
    inline CTab::CSelectDialog::CSelectDialog(LPCDLGTEMPLATE pDlgTemplate) :
                    CDialog(pDlgTemplate), IDC_LIST(122)
    {
    }

    inline BOOL CTab::CSelectDialog::OnInitDialog()
    {
        for (UINT u = 0; u < m_items.size(); ++u)
        {
            LPARAM text = reinterpret_cast<LPARAM>(m_items[u].c_str());
            SendDlgItemMessage(IDC_LIST, LB_ADDSTRING, 0, text);
        }

        return true;
    }

    inline void CTab::CSelectDialog::AddItem(LPCTSTR string)
    {
        m_items.push_back(string);
    }

    inline void CTab::CSelectDialog::OnOK()
    {
        int select = static_cast<int>(SendDlgItemMessage(IDC_LIST, LB_GETCURSEL, 0, 0));
        if (select != LB_ERR)
            EndDialog(select);
        else
            EndDialog(-2);
    }


    //////////////////////////////////
    // Definitions for the CTab class.
    //

    inline CTab::CTab() : m_pActiveView(nullptr), m_isShowingButtons(FALSE), m_isTracking(FALSE),
                          m_isClosePressed(FALSE), m_isListPressed(FALSE), m_isListMenuActive(FALSE)
    {
        /*
        103 DIALOGEX 0, 0, 208, 202
        STYLE DS_SHELLFONT | DS_MODALFRAME | WS_POPUP | WS_VISIBLE | WS_CAPTION | WS_SYSMENU
        EXSTYLE WS_EX_APPWINDOW
        CAPTION "Select Tab"
        LANGUAGE LANG_NEUTRAL, 0x1
        FONT 8, "MS Shell Dlg"
        {
           CONTROL "", 122, LISTBOX, LBS_NOTIFY | LBS_NOINTEGRALHEIGHT | WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_TABSTOP, 16, 16, 176, 163 , WS_EX_STATICEDGE
           CONTROL "Cancel", 2, BUTTON, BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE | WS_TABSTOP, 39, 183, 49, 14
           CONTROL "OK", 1, BUTTON, BS_DEFPUSHBUTTON | WS_CHILD | WS_VISIBLE | WS_TABSTOP, 119, 183, 50, 14
        }
        */

        // Dialog template for the dialog definition shown above.
        static const unsigned char dlgTemplate[] =
        {
            0x01,0x00,0xff,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x04,0x00,0xc8,0x00,0xc8,0x90,0x03,
            0x00,0x00,0x00,0x00,0x00,0xd0,0x00,0xca,0x00,0x00,0x00,0x00,0x00,0x53,0x00,0x65,
            0x00,0x6c,0x00,0x65,0x00,0x63,0x00,0x74,0x00,0x20,0x00,0x54,0x00,0x61,0x00,0x62,
            0x00,0x00,0x00,0x08,0x00,0x00,0x00,0x00,0x01,0x4d,0x00,0x53,0x00,0x20,0x00,0x53,
            0x00,0x68,0x00,0x65,0x00,0x6c,0x00,0x6c,0x00,0x20,0x00,0x44,0x00,0x6c,0x00,0x67,
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x00,0x01,0x01,0x21,0x50,0x10,
            0x00,0x10,0x00,0xb0,0x00,0xa3,0x00,0x7a,0x00,0x00,0x00,0xff,0xff,0x83,0x00,0x00,
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x50,0x27,
            0x00,0xb7,0x00,0x31,0x00,0x0e,0x00,0x02,0x00,0x00,0x00,0xff,0xff,0x80,0x00,0x43,
            0x00,0x61,0x00,0x6e,0x00,0x63,0x00,0x65,0x00,0x6c,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x01,0x50,0x77,0x00,0xb7,0x00,0x32,
            0x00,0x0e,0x00,0x01,0x00,0x00,0x00,0xff,0xff,0x80,0x00,0x4f,0x00,0x4b,0x00,0x00,
            0x00,0x00,0x00
        };

        m_blankPageColor = GetSysColor(COLOR_BTNFACE);
        SetListDialog(reinterpret_cast<LPCDLGTEMPLATE>(dlgTemplate));
    }

    inline CTab::~CTab()
    {
    }

    // Adds a tab along with the specified view window.
    // The framework assumes ownership of the CWnd pointer provided,
    // and deletes the CWnd object when the tab is removed or destroyed.
    // Returns a pointer to the view window that was supplied.
    // Use RemoveTabPage to remove the tab and page added in this manner.
    inline CWnd* CTab::AddTabPage(CWnd* pView, LPCTSTR tabText, HICON icon, int tabID)
    {
        assert(pView);
        return AddTabPage(WndPtr(pView), tabText, icon, tabID);
    }

    // Adds a tab along with the specified view window.
    // The framework assumes ownership of the CWnd pointer provided,
    // and deletes the CWnd object when the tab is removed or destroyed.
    // Use RemoveTabPage to remove the tab and page added in this manner.
    inline CWnd* CTab::AddTabPage(CWnd* pView, LPCTSTR tabText, UINT iconID, int tabID /* = 0*/)
    {
        HICON icon = static_cast<HICON>(GetApp()->LoadImage(iconID, IMAGE_ICON, 0, 0, LR_SHARED));
        return AddTabPage(pView, tabText, icon, tabID);
    }

    // Adds a tab along with the specified view window.
    // The framework assumes ownership of the CWnd pointer provided,
    // and deletes the CWnd object when the tab is removed or destroyed.
    // Use RemoveTabPage to remove the tab and page added in this manner.
    inline CWnd* CTab::AddTabPage(CWnd* pView, LPCTSTR tabText)
    {
        return AddTabPage(pView, tabText, reinterpret_cast<HICON>(0), 0);
    }

    // Adds a tab along with the specified view window.
    // The framework assumes ownership of the WndPtr provided,
    // and deletes the CWnd object when the tab is removed or destroyed.
    // Returns a pointer to the view window that was supplied.
    // Use RemoveTabPage to remove the tab and page added in this manner.
    inline CWnd* CTab::AddTabPage(WndPtr view, LPCTSTR tabText, HICON icon, int tabID)
    {
        assert(IsWindow());
        CWnd* pView = view.get();
        assert(pView);
        assert(lstrlen(tabText) < WXX_MAX_STRING_SIZE);
        if (pView == nullptr)
            return nullptr;

        // Set the TabPageInfo.
        TabPageInfo tpi;
        tpi.pView = pView;
        tpi.tabIcon = icon;
        tpi.tabID = tabID;
        tpi.tabText = tabText;
        if (icon != nullptr)
            tpi.tabImage = GetImages().Add(icon);
        else
            tpi.tabImage = -1;

        int page = static_cast<int>(m_allTabPageInfo.size());
        m_allTabPageInfo.push_back(tpi);
        m_tabViews.push_back(std::move(view));

        // Add the tab to the tab control.
        TCITEM tie{};
        tie.mask = TCIF_TEXT | TCIF_IMAGE;
        tie.iImage = tpi.tabImage;
        tie.pszText = const_cast<LPTSTR>(tpi.tabText.c_str());
        InsertItem(page, &tie);
        SetTabSize();
        SelectPage(page);

        return pView;
    }

    // Adds a tab along with the specified view window.
    // The framework assumes ownership of the WndPtr provided,
    // and deletes the CWnd object when the tab is removed or destroyed.
    // Use RemoveTabPage to remove the tab and page added in this manner.
    inline CWnd* CTab::AddTabPage(WndPtr view, LPCTSTR tabText, UINT iconID, int tabID /* = 0 */)
    {
        HICON icon = static_cast<HICON>(GetApp()->LoadImage(iconID, IMAGE_ICON, 0, 0, LR_SHARED));
        return AddTabPage(std::move(view), tabText, icon, tabID);
    }

    // Adds a tab along with the specified view window.
    // The framework assumes ownership of the WndPtr provided,
    // and deletes the CWnd object when the tab is removed or destroyed.
    // Use RemoveTabPage to remove the tab and page added in this manner.
    inline CWnd* CTab::AddTabPage(WndPtr view, LPCTSTR tabText)
    {
        return AddTabPage(std::move(view), tabText, reinterpret_cast<HICON>(0), 0);
    }

    // Draws the close button
    inline void CTab::DrawCloseButton(CDC& dc)
    {
        if (!m_isShowingButtons) return;
        if (!GetActiveView()) return;
        if (!(GetStyle() & TCS_FIXEDWIDTH)) return;
        if (!(GetStyle() & TCS_OWNERDRAWFIXED)) return;

        // Determine the close button's drawing position relative to the window.
        CRect rcClose = GetCloseRect();

        CPoint pt = GetCursorPos();
        VERIFY(ScreenToClient(pt));
        UINT state = rcClose.PtInRect(pt)? m_isClosePressed? 2U: 1U: 0U;

        // Draw the outer highlight for the close button.
        if (!IsRectEmpty(&rcClose))
        {
            // Use the Marlett font to draw special characters.
            CFont marlett;
            marlett.CreatePointFont(100, _T("Marlett"));
            dc.SetBkMode(TRANSPARENT);
            LOGFONT lf = DpiScaleLogfont(marlett.GetLogFont(), 10);
            dc.CreateFontIndirect(lf);

            COLORREF grey(RGB(232, 228, 220));
            COLORREF black(RGB(0, 0, 0));
            COLORREF white(RGB(255, 255, 255));

            switch (state)
            {
            case 0:
            {
                // Draw a grey box for the normal button using two special characters.
                dc.SetTextColor(grey);
                dc.TextOut(rcClose.left, rcClose.top, _T("\x63"), 1);
                dc.TextOut(rcClose.left, rcClose.top, _T("\x64"), 1);
            }
            break;

            case 1:
            {
                // Draw popped up button, black on right and bottom.
                dc.SetTextColor(white);
                dc.TextOut(rcClose.left, rcClose.top, _T("\x63"), 1);
                dc.SetTextColor(black);
                dc.TextOut(rcClose.left, rcClose.top, _T("\x64"), 1);
            }
            break;

            case 2:
            {
                // Draw popped up button, black on right and bottom.
                dc.SetTextColor(black);
                dc.TextOut(rcClose.left, rcClose.top, _T("\x63"), 1);
                dc.SetTextColor(white);
                dc.TextOut(rcClose.left, rcClose.top, _T("\x64"), 1);
            }
            break;

            }

            // Draw the close button (a Marlett "r" looks like "X").
            dc.SetTextColor(RGB(0, 0, 0));
            dc.TextOut(rcClose.left, rcClose.top, _T("\x72"), 1);
        }
    }

    // Draws the list button.
    inline void CTab::DrawListButton(CDC& dc)
    {
        if (!m_isShowingButtons) return;
        if (!GetActiveView()) return;
        if (!(GetStyle() & TCS_FIXEDWIDTH)) return;
        if (!(GetStyle() & TCS_OWNERDRAWFIXED)) return;

        // Determine the list button's drawing position relative to the window.
        CRect rcList = GetListRect();

        CPoint pt = GetCursorPos();
        VERIFY(ScreenToClient(pt));
        UINT uState = rcList.PtInRect(pt)? 1U: 0U;
        if (m_isListMenuActive) uState = 2;

        // Draw the outer highlight for the list button.
        if (!IsRectEmpty(&rcList))
        {
            // Use the Marlett font to draw special characters.
            CFont marlett;
            marlett.CreatePointFont(100, _T("Marlett"));
            dc.SetBkMode(TRANSPARENT);
            LOGFONT lf = DpiScaleLogfont(marlett.GetLogFont(), 10);
            dc.CreateFontIndirect(lf);

            COLORREF grey(RGB(232, 228, 220));
            COLORREF black(RGB(0, 0, 0));
            COLORREF white(RGB(255, 255, 255));

            switch (uState)
            {
            case 0:
            {
                // Draw a grey box for the normal button using two special characters.
                dc.SetTextColor(grey);
                dc.TextOut(rcList.left, rcList.top, _T("\x63"), 1);
                dc.TextOut(rcList.left, rcList.top, _T("\x64"), 1);
            }
            break;

            case 1:
            {
                // Draw popped up button, black on right and bottom.
                dc.SetTextColor(white);
                dc.TextOut(rcList.left, rcList.top, _T("\x63"), 1);
                dc.SetTextColor(black);
                dc.TextOut(rcList.left, rcList.top, _T("\x64"), 1);
            }
            break;

            case 2:
            {
                // Draw pressed button, black on left and top.
                dc.SetTextColor(black);
                dc.TextOut(rcList.left, rcList.top, _T("\x63"), 1);
                dc.SetTextColor(white);
                dc.TextOut(rcList.left, rcList.top, _T("\x64"), 1);
            }
            break;

            }

            if (GetWinVersion() >= 3000)  // Windows 10 or later.
                rcList.OffsetRect(1, -1);

            // Draw the down arrow button.
            dc.SetTextColor(black);
            dc.TextOut(rcList.left, rcList.top, _T("\x75"), 1);
        }
    }

    // Draw the tabs.
    inline void CTab::DrawTabs(CDC& dc)
    {
        // Draw the tab buttons:
        for (int i = 0; i < GetItemCount(); ++i)
        {
            CRect rcItem;
            GetItemRect(i, rcItem);
            if (!rcItem.IsRectEmpty())
            {
                if (i == GetCurSel())
                {
                    dc.CreateSolidBrush(RGB(248,248,248));
                    dc.SetBkColor(RGB(248,248,248));
                }
                else
                {
                    dc.CreateSolidBrush(RGB(200,200,200));
                    dc.SetBkColor(RGB(200,200,200));
                }

                dc.CreatePen(PS_SOLID, 1, RGB(160, 160, 160));
                dc.RoundRect(rcItem.left, rcItem.top, rcItem.right +1, rcItem.bottom, 6, 6);

                CSize szImage = GetImages().GetIconSize();
                int padding = DpiScaleInt(4);

                if (rcItem.Width() >= szImage.cx + 2 * padding)
                {
                    CString str = GetTabText(i);
                    int image = GetTabImageID(i);
                    int yOffset = (rcItem.Height() - szImage.cy)/2;

                    // Draw the icon.
                    int drawleft = rcItem.left + padding;
                    int drawtop = rcItem.top + yOffset;
                    GetImages().Draw(dc, image,  CPoint(drawleft, drawtop), ILD_NORMAL);

                    // Calculate the size of the text.
                    CRect rcText = rcItem;
                    if (image >= 0)
                        rcText.left += szImage.cx + padding;

                    rcText.left += padding;

                    // Draw the text.
                    dc.SelectObject(m_tabFont);
                    dc.DrawText(str, -1, rcText, DT_LEFT | DT_VCENTER |
                        DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
                }
            }
        }
    }

    // Draw the tab borders.
    inline void CTab::DrawTabBorders(CDC& dc, RECT& rc)
    {
        bool isBottomTab = (GetStyle() & TCS_BOTTOM) != 0;

        // Draw a lighter rectangle touching the tab buttons.
        CRect rcItem;
        int gap = 3;
        GetItemRect(0, rcItem);
        int left = rcItem.left;
        int right = rc.right;
        int top = rc.bottom;
        int bottom = top + gap;

        if (!isBottomTab)
        {
            int rcTop = rc.top;
            bottom = std::max(rcTop, GetTabHeight() + gap + 1);
            top = bottom - gap;
        }

        dc.CreateSolidBrush(RGB(248, 248, 248));
        dc.CreatePen(PS_SOLID, 1, RGB(248, 248, 248));
        if (!rcItem.IsRectEmpty())
        {
            dc.Rectangle(left, top, right, bottom);

            // Draw a darker line below the rectangle.
            dc.CreatePen(PS_SOLID, 1, RGB(160, 160, 160));
            if (isBottomTab)
            {
                dc.MoveTo(left-1, bottom);
                dc.LineTo(right-1, bottom);
            }
            else
            {
                dc.MoveTo(left-1, top-1);
                dc.LineTo(right-1, top-1);
            }

            // Draw a lighter line over the darker line for the selected tab.
            dc.CreatePen(PS_SOLID, 1, RGB(248, 248, 248));
            GetItemRect(GetCurSel(), rcItem);
            OffsetRect(&rcItem, 0, 1);

            if (isBottomTab)
            {
                dc.MoveTo(rcItem.left, bottom);
                dc.LineTo(rcItem.right, bottom);
            }
            else
            {
                dc.MoveTo(rcItem.left, top-1);
                dc.LineTo(rcItem.right, top-1);
            }
        }
    }

    // Returns the dimensions of the bounding rectangle of the close button.
    inline CRect CTab::GetCloseRect() const
    {
        CRect rc;
        if (GetShowButtons())
        {
            rc = GetClientRect();
            int gap = DpiScaleInt(1);
            int cx = GetSystemMetrics(SM_CXSMICON) * GetWindowDpi(*this) / GetWindowDpi(HWND_DESKTOP);
            int cy = GetSystemMetrics(SM_CXSMICON) * GetWindowDpi(*this) / GetWindowDpi(HWND_DESKTOP);
            rc.right -= gap;
            rc.left = rc.right - cx;

            if (GetTabsAtTop())
            {
                rc.top = (GetTabHeight() - cy) / 2;
                rc.bottom = rc.top + cy;
            }
            else
            {
                rc.bottom = rc.bottom - (GetTabHeight() - cy) / 2;
                rc.top = rc.bottom - cy;
            }

            if (GetExStyle() & WS_EX_LAYOUTRTL)
            {
                rc.left = rc.left + gap;
                rc.right = rc.left + cx;
            }
        }

        return rc;
    }

    // Updates and returns the list menu.
    inline CMenu CTab::GetListMenu()
    {
        if (!IsMenu(m_listMenu))
            m_listMenu.CreatePopupMenu();

        // Remove any current menu items.
        while (m_listMenu.GetMenuItemCount() > 0)
        {
            m_listMenu.RemoveMenu(0, MF_BYPOSITION);
        }

        // Add the menu items.
        for (size_t i = 0; i < std::min(GetAllTabs().size(), size_t(9)); ++i)
        {
            CString menuString;
            CString tabText = GetAllTabs()[i].tabText;
            menuString.Format(_T("&%d %s"), i+1, tabText.c_str());
            m_listMenu.AppendMenu(MF_STRING, IDW_FIRSTCHILD + UINT_PTR(i), menuString);
        }
        if (GetAllTabs().size() >= 10)
            m_listMenu.AppendMenu(MF_STRING, IDW_FIRSTCHILD +9, _T("More Tabs"));

        // Add a checkmark to the menu.
        UINT selected = static_cast<UINT>(GetCurSel());
        if (selected < 9)
            m_listMenu.CheckMenuItem(selected, MF_BYPOSITION|MF_CHECKED);

        return m_listMenu;
    }

    // Returns the dimensions of the bounding rectangle of the list button.
    inline CRect CTab::GetListRect() const
    {
        CRect rcList;
        if (GetShowButtons())
        {
            CRect rcClose = GetCloseRect();
            rcList = rcClose;
            rcList.OffsetRect(-(rcClose.Width() + DpiScaleInt(2)), 0);
            rcList.InflateRect(-1, 0);
        }
        return rcList;
    }

    // Returns the size of the largest tab.
    inline SIZE CTab::GetMaxTabSize() const
    {
        CSize Size;

        for (int i = 0; i < GetItemCount(); ++i)
        {
            CClientDC dcClient(*this);
            dcClient.SelectObject(m_tabFont);
            CString str;
            TCITEM tcItem{};
            tcItem.mask = TCIF_TEXT |TCIF_IMAGE;
            tcItem.cchTextMax = WXX_MAX_STRING_SIZE;
            tcItem.pszText = str.GetBuffer(WXX_MAX_STRING_SIZE);
            GetItem(i, &tcItem);
            str.ReleaseBuffer();
            CSize TempSize = dcClient.GetTextExtentPoint32(str, lstrlen(str));

            int imageSize = 0;
            int padding = DpiScaleInt(10);
            if (tcItem.iImage >= 0)
            {
                imageSize = m_images.GetIconSize().cx;
            }

            TempSize.cx += imageSize + padding;

            if (TempSize.cx > Size.cx)
                Size = TempSize;
        }

        return Size;
    }

    // Returns TRUE if the control's tabs are placed at the top.
    inline BOOL CTab::GetTabsAtTop() const
    {
        DWORD style = GetStyle();
        return (!(style & TCS_BOTTOM));
    }

    // Retrieves the height of the text.
    inline int CTab::GetTextHeight() const
    {
        CClientDC dcClient(*this);
        dcClient.SelectObject(m_tabFont);
        CSize szText = dcClient.GetTextExtentPoint32(_T("Text"), lstrlen(_T("Text")));
        return szText.cy;
    }

    // Retrieves the tab height calculated from the icon height and text height.
    inline int CTab::GetTabHeight() const
    {
        int heightGap = DpiScaleInt(5);
        int iconSizeX = m_images.GetIconSize().cx;
        return std::max(iconSizeX, GetTextHeight()) + heightGap;
    }

    // Returns the index of the tab given its view window.
    inline int CTab::GetTabIndex(CWnd* pWnd) const
    {
        assert(pWnd);

        for (size_t i = 0; i < m_allTabPageInfo.size(); ++i)
        {
            if (m_allTabPageInfo[i].pView == pWnd)
                return static_cast<int>(i);
        }

        return -1;
    }

    // Returns the tab page info struct for the specified tab.
    inline TabPageInfo CTab::GetTabPageInfo(int tab) const
    {
        size_t tabIndex = static_cast<size_t>(tab);
        assert (tabIndex < m_allTabPageInfo.size());
        return m_allTabPageInfo[tabIndex];
    }

    // Returns the image ID for the specified tab.
    inline int CTab::GetTabImageID(int tab) const
    {
        size_t tabIndex = static_cast<size_t>(tab);
        assert (tabIndex < m_allTabPageInfo.size());
        return m_allTabPageInfo[tabIndex].tabImage;
    }

    // Returns the text for the specified tab.
    inline CString CTab::GetTabText(int tab) const
    {
        size_t tabIndex = static_cast<size_t>(tab);
        assert (tabIndex < m_allTabPageInfo.size());
        return m_allTabPageInfo[tabIndex].tabText;
    }

    // Sends a UMN_TABCHANGED notification.
    inline void CTab::NotifyChanged()
    {
        NMHDR nmhdr{};
        nmhdr.hwndFrom = *this;
        nmhdr.code = UMN_TABCHANGED;
        LPARAM lparam = reinterpret_cast<LPARAM>(&nmhdr);

        if (GetParent().IsWindow())
            GetParent().SendMessage(WM_NOTIFY, 0, lparam);
    }

    // Sends a UWN_TABDRAGGED notification.
    inline void CTab::NotifyDragged()
    {
        NMHDR nmhdr{};
        nmhdr.hwndFrom = *this;
        nmhdr.code = UWN_TABDRAGGED;
        LPARAM lparam = reinterpret_cast<LPARAM>(&nmhdr);
        GetParent().SendMessage(WM_NOTIFY, 0, lparam);
    }

    // Sends a UWN_TABCLOSE notification.
    inline BOOL CTab::NotifyTabClosing(int page)
    {
        UINT controlID = GetDlgCtrlID();
        TABNMHDR tabNMHDR{};
        tabNMHDR.hdr.code = UWN_TABCLOSE;
        tabNMHDR.hdr.hwndFrom = *this;
        tabNMHDR.hdr.idFrom = controlID;
        tabNMHDR.page = page;
        WPARAM wparam = static_cast<WPARAM>(controlID);
        LPARAM lparam = reinterpret_cast<LPARAM>(&tabNMHDR);

        // The default return value is zero.
        return static_cast<BOOL>(GetParent().SendMessage(WM_NOTIFY, wparam, lparam));
    }

    // Called when this object is attached to a tab control.
    inline void CTab::OnAttach()
    {
        UpdateTabs();
    }

    // Called when the background is erased.
    inline LRESULT CTab::OnEraseBkgnd(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        if (GetStyle() & TCS_OWNERDRAWFIXED)
            return 0;

        return FinalWindowProc(msg, wparam, lparam);
    }

    // Called when the left mouse button is pressed.
    inline LRESULT CTab::OnLButtonDown(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        CPoint pt(GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam));

        if (GetCloseRect().PtInRect(pt))
        {
            m_isClosePressed = TRUE;
            SetCapture();
            CClientDC dc(*this);
            DrawCloseButton(dc);
        }
        else
            m_isClosePressed = FALSE;

        if (GetListRect().PtInRect(pt))
        {
            ShowListMenu();
        }

        return FinalWindowProc(msg, wparam, lparam);
    }

    // Called when the left mouse button is released.
    inline LRESULT CTab::OnLButtonUp(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        ReleaseCapture();
        CPoint pt(GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam));
        if (m_isClosePressed && GetCloseRect().PtInRect(pt))
        {
            int page = GetCurSel();

            // Send a notification to parent asking if its OK to close the tab.
            if (!NotifyTabClosing(page))
            {
                RemoveTabPage(page);
                if (page > 0)
                    SelectPage(page -1);

                if (GetActiveView())
                    GetActiveView()->RedrawWindow();
            }
        }

        m_isClosePressed = FALSE;

        return FinalWindowProc(msg, wparam, lparam);
    }

    // Called when the mouse moves outside of the window.
    inline LRESULT CTab::OnMouseLeave(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        CClientDC dc(*this);
        DrawCloseButton(dc);
        DrawListButton(dc);

        m_isTracking = FALSE;

        return FinalWindowProc(msg, wparam, lparam);
    }

    // Called when the mouse is moved.
    inline LRESULT CTab::OnMouseMove(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        CPoint pt;
        pt.x = GET_X_LPARAM(lparam);
        pt.y = GET_Y_LPARAM(lparam);

        if (!m_isListMenuActive && m_isListPressed)
        {
            m_isListPressed = FALSE;
        }

        if (!m_isTracking)
        {
            TRACKMOUSEEVENT TrackMouseEventStruct{};
            TrackMouseEventStruct.cbSize = sizeof(TrackMouseEventStruct);
            TrackMouseEventStruct.dwFlags = TME_LEAVE;
            TrackMouseEventStruct.hwndTrack = *this;
            _TrackMouseEvent(&TrackMouseEventStruct);
            m_isTracking = TRUE;
        }

        // Skip if mouse hasn't moved.
        if ((pt.x != m_oldMousePos.x) || (pt.y != m_oldMousePos.y))
        {
            if (IsLeftButtonDown())
                NotifyDragged();
        }

        m_oldMousePos.x = pt.x;
        m_oldMousePos.y = pt.y;

        CClientDC dc(*this);
        DrawCloseButton(dc);
        DrawListButton(dc);

        return FinalWindowProc(msg, wparam, lparam);
    }

    // Perform a hit test on the non-client area.
    inline LRESULT CTab::OnNCHitTest(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        // Ensure we have an arrow cursor when the tab has no view window.
        if (GetAllTabs().size() == 0)
            SetCursor(LoadCursor(0, IDC_ARROW));

        // Cause WM_LBUTTONUP and WM_LBUTTONDOWN messages to be sent for buttons.
        CPoint pt(GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam));
        VERIFY(ScreenToClient(pt));
        if (GetCloseRect().PtInRect(pt)) return HTCLIENT;
        if (GetListRect().PtInRect(pt))  return HTCLIENT;

        return CWnd::WndProcDefault(msg, wparam, lparam);
    }

    // Handle the notifications we send.
    inline LRESULT CTab::OnNotifyReflect(WPARAM, LPARAM lparam)
    {
        LPNMHDR pHeader = (LPNMHDR)lparam;
        switch (pHeader->code)
        {
        case TCN_SELCHANGE: return OnTCNSelChange(pHeader);
        }

        return 0;
    }

    // Called when a different tab is selected.
    inline LRESULT CTab::OnTCNSelChange(LPNMHDR)
    {
        // Display the newly selected tab page.
        int page = GetCurSel();
        ShowActiveView(m_allTabPageInfo[static_cast<size_t>(page)].pView);

        return 0;
    }

    // Called when this tab control loses focus.
    inline LRESULT CTab::OnKillFocus(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        m_isClosePressed = FALSE;
        return FinalWindowProc(msg, wparam, lparam);
    }

    // Called when the left mouse button is double clicked.
    inline LRESULT CTab::OnLButtonDblClk(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        return OnLButtonDown(msg, wparam, lparam);
    }

    // Called when this control needs to be painted.
    inline LRESULT CTab::OnPaint(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        if (GetStyle() & TCS_OWNERDRAWFIXED)
        {
            // Remove all pending paint requests.
            PAINTSTRUCT ps;
            ::BeginPaint(*this, &ps);
            ::EndPaint(*this, &ps);

            // Now call our local Paint.
            Paint();
            return 0;
        }

        return FinalWindowProc(msg, wparam, lparam);
    }

    // Called when this control acquires keyboard focus.
    inline LRESULT CTab::OnSetFocus(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        // Prevent the tab control from grabbing focus when we have a view.
        if (GetActiveView() && !m_isClosePressed)
        {
            GetActiveView()->SetFocus();
        }

        return FinalWindowProc(msg, wparam, lparam);
    }

    // Called when a window is destroyed.
    inline void CTab::OnDestroy()
    {
        // Remove any current pages.
        int pages = static_cast<int>(m_allTabPageInfo.size());
        for (int i = pages - 1; i >= 0; i--)
        {
            RemoveTabPage(i);
        }
    }

    // Called in response to a WM_DPICHANGED_AFTERPARENT message that is sent to child
    // windows after a DPI change. A WM_DPICHANGED_AFTERPARENT is only received when the
    // application is DPI_AWARENESS_PER_MONITOR_AWARE.
    inline LRESULT CTab::OnDpiChangedAfterParent(UINT, WPARAM, LPARAM)
    {
        UpdateTabs();

        return 0;
    }

    // Called after the tab control is resized.
    inline LRESULT CTab::OnWindowPosChanged(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        RecalcLayout();
        return FinalWindowProc(msg, wparam, lparam);
    }

    // Called while the tab control is being resized.
    inline LRESULT CTab::OnWindowPosChanging(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        // A little hack to reduce tab flicker.
        if (IsWindowVisible() && (GetStyle() & TCS_OWNERDRAWFIXED))
        {
            LPWINDOWPOS pWinPos = (LPWINDOWPOS)lparam;
            pWinPos->flags |= SWP_NOREDRAW;

            // Do a manual paint when OnDraw isn't called.
            if (GetWindowRect().Height() < GetTabHeight())
                Paint();
        }

        return FinalWindowProc(msg, wparam, lparam);
    }

    // Paint the control manually.
    // Microsoft's drawing for a tab control has quite a bit of flicker on some
    // of its operating systems, so we do our own.
    // We use double buffering and regions to eliminate flicker.
    inline void CTab::Paint()
    {
        bool isRTL = FALSE;
        isRTL = ((GetExStyle() & WS_EX_LAYOUTRTL)) != 0;

        // Create the memory DC and bitmap.
        CClientDC dcView(*this);
        CMemDC memDC(dcView);
        CRect rcClient = GetClientRect();
        memDC.CreateCompatibleBitmap(dcView, rcClient.Width(), rcClient.Height());

        if (GetItemCount() == 0)
        {
            // No tabs, so simply display a gray background and exit.
            dcView.SolidFill(m_blankPageColor, rcClient);
            return;
        }

        // Create a clipping region. Its the overall tab window's region,
        //  less the region belonging to the individual tab view's client area.
        CRgn rgnSrc1;
        rgnSrc1.CreateRectRgnIndirect(rcClient);
        CRect rcTab = GetClientRect();
        AdjustRect(FALSE, &rcTab);
        if (rcTab.Height() < 0)
            rcTab.top = rcTab.bottom;
        if (rcTab.Width() < 0)
            rcTab.left = rcTab.right;

        int offset = isRTL ? -1 : 0;  // Required for RTL layout.
        CRgn rgnSrc2;
        rgnSrc2.CreateRectRgn(rcTab.left, rcTab.top, rcTab.right + offset, rcTab.bottom);
        CRgn rgnClip;
        rgnClip.CreateRectRgn(0, 0, 0, 0);
        rgnClip.CombineRgn(rgnSrc1, rgnSrc2, RGN_DIFF);

        // Use the region in the memory DC to paint the gray background.
        memDC.SelectClipRgn(rgnClip);
        memDC.CreateSolidBrush( GetSysColor(COLOR_BTNFACE) );
        memDC.PaintRgn(rgnClip);

        // Draw the tab buttons on the memory DC:
        DrawTabs(memDC);

        // Draw buttons and tab borders.
        DrawCloseButton(memDC);
        DrawListButton(memDC);
        DrawTabBorders(memDC, rcTab);

        // Now copy from our memory DC to the window DC.
        dcView.SelectClipRgn(rgnClip);

        if (isRTL)
        {
            // BitBlt offset bitmap copies by one for Right-To-Left layout.
            dcView.BitBlt(0, 0, 1, rcClient.Height(), memDC, 1, 0, SRCCOPY);
            dcView.BitBlt(1, 0, rcClient.Width(), rcClient.Height(), memDC, 1, 0, SRCCOPY);
        }
        else
            dcView.BitBlt(0, 0, rcClient.Width(), rcClient.Height(), memDC, 0, 0, SRCCOPY);
    }

    // Set the window style before it is created.
    inline void CTab::PreCreate(CREATESTRUCT &cs)
    {
        // For Tabs on the bottom, add the TCS_BOTTOM style.
        cs.style = WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE;
    }

    // Set the window class.
    inline void CTab::PreRegisterClass(WNDCLASS& wc)
    {
        wc.lpszClassName = WC_TABCONTROL;
    }

    // Repositions the child windows of the tab control.
    inline void CTab::RecalcLayout()
    {
        if (IsWindow())
        {
            SetTabSize();

            if (GetActiveView())
            {
                // Position the View over the tab control's display area.
                CRect rc = GetClientRect();
                MapWindowPoints(GetParent(), rc);
                AdjustRect(FALSE, &rc);
                VERIFY(GetActiveView()->SetWindowPos(HWND_TOP, rc, SWP_SHOWWINDOW));
            }

            RedrawWindow(RDW_INVALIDATE | RDW_NOCHILDREN);
        }
    }

    // Removes a tab and its view page.
    inline void CTab::RemoveTabPage(int page)
    {
        if ((page < 0) || (page > static_cast<int>(m_allTabPageInfo.size() -1)))
            return;

        // Remove the tab.
        DeleteItem(page);

        // Remove the TapPageInfo entry.
        auto itTPI = m_allTabPageInfo.begin() + page;
        CWnd* pView = (*itTPI).pView;
        int image = (*itTPI).tabImage;
        if (image >= 0)
            RemoveImage(image);

        if (pView == m_pActiveView)
            m_pActiveView = 0;

        (*itTPI).pView->Destroy();
        m_allTabPageInfo.erase(itTPI);

        for (auto itView = m_tabViews.begin(); itView != m_tabViews.end(); ++itView)
        {
            if ((*itView).get() == pView)
            {
                m_tabViews.erase(itView);
                break;
            }
        }

        if (IsWindow())
        {
            if (m_allTabPageInfo.size() > 0)
            {
                SetTabSize();
                SelectPage(0);
            }
            else
                ShowActiveView(nullptr);

            NotifyChanged();
        }
    }

    // Selects the tab and the view page.
    inline void CTab::SelectPage(int page)
    {
        if ((page >= 0) && (page < GetItemCount()))
        {
            if (page != GetCurSel())
                SetCurSel(page);

            ShowActiveView(m_allTabPageInfo[static_cast<size_t>(page)].pView);
        }
    }

    // Enable or disable fixed-with tabs.
    // When fixed-width is enabled, all tabs have the same width.
    inline void CTab::SetFixedWidth(BOOL isEnabled)
    {
        DWORD style = GetStyle();
        style = isEnabled ? style | TCS_FIXEDWIDTH : style & ~TCS_FIXEDWIDTH;
        SetStyle(style);
        UpdateImageList();
        RecalcLayout();
    }

    // Enable or disable owner-draw.
    // When owner-draw is enabled, the tabs are drawn by Paint().
    // Note: Setting owner-draw also sets fixed-width.
    inline void CTab::SetOwnerDraw(BOOL isEnabled)
    {
        DWORD style = GetStyle();
        style = isEnabled ? style | TCS_OWNERDRAWFIXED | TCS_FIXEDWIDTH : style & ~TCS_OWNERDRAWFIXED;
        SetStyle(style);
        UpdateImageList();
        RecalcLayout();
    }

    // Allows the list and close buttons to be shown or hidden.
    inline void CTab::SetShowButtons(BOOL show)
    {
        m_isShowingButtons = show;
        RecalcLayout();
    }

    // Sets the font used by the tabs and adjusts the tab height to match.
    inline void CTab::SetTabFont(HFONT font)
    {
        m_tabFont = font;

        // Set the font used without owner draw.
        SetFont(font);
    }

    // Changes or sets the tab's icon.
    inline void CTab::SetTabIcon(int tab, UINT iconID)
    {
        HICON icon = static_cast<HICON>(GetApp()->LoadImage(iconID, IMAGE_ICON, 0, 0, LR_SHARED));
        SetTabIcon(tab, icon);
    }

    // Changes or sets the tab's icon.
    inline void CTab::SetTabIcon(int tab, HICON icon)
    {
        assert (GetItemCount() > tab);
        TCITEM tci{};
        tci.mask = TCIF_IMAGE;
        GetItem(tab, &tci);
        if (tci.iImage >= 0)
        {
            GetImages().Replace(tab, icon);
        }
        else
        {
            int image = GetImages().Add(icon);
            tci.iImage = image;
            SetItem(tab, &tci);
            m_allTabPageInfo[static_cast<size_t>(tab)].tabImage = image;
        }

        m_allTabPageInfo[static_cast<size_t>(tab)].tabIcon = icon;
        SetTabSize();
    }

    // Positions the tabs at the top or bottom of the control.
    inline void CTab::SetTabsAtTop(BOOL isAtTop)
    {
        DWORD style = GetStyle();

        if (isAtTop)
            style &= ~TCS_BOTTOM;
        else
            style |= TCS_BOTTOM;

        SetStyle(style);
        RecalcLayout();
    }

    // Sets the width and height of tabs in a fixed-width or owner-drawn tab control.
    inline void CTab::SetTabSize()
    {
        if (GetItemCount() > 0)
        {
            DWORD style = GetStyle();
            if (style & TCS_OWNERDRAWFIXED)
            {
                CRect rc = GetClientRect();
                AdjustRect(FALSE, &rc);
                int padding = DpiScaleInt(2);
                if (m_isShowingButtons)
                    padding = GetCloseRect().Width() + GetListRect().Width() + DpiScaleInt(4);

                int maxTabSizeX = GetMaxTabSize().cx;
                int itemWidth = std::min(maxTabSizeX, (rc.Width() - padding) / GetItemCount());
                itemWidth = std::max(itemWidth, 0);
                SetItemSize(itemWidth, GetTabHeight());
                NotifyChanged();
            }
            else
            {
                int itemWidth = GetMaxTabSize().cx;
                SetItemSize(itemWidth, GetTabHeight());
                NotifyChanged();
            }
        }
    }

    // Allows the text to be changed on an existing tab.
    inline void CTab::SetTabText(int tab, LPCTSTR text)
    {
        size_t tabIndex = static_cast<size_t>(tab);
        if (tabIndex < GetAllTabs().size())
        {
            TCITEM Item{};
            Item.mask = TCIF_TEXT;
            Item.pszText = const_cast<LPTSTR>(text);

            if (SetItem(tab, &Item))
                m_allTabPageInfo[tabIndex].tabText = text;

            SetTabSize();
        }
    }

    // Sets or changes the view window displayed within the tab page.
    inline void CTab::ShowActiveView(CWnd* pView)
    {
        if (pView != m_pActiveView)
        {
            // Hide the old view.
            if (GetActiveView() && (GetActiveView()->IsWindow()))
                GetActiveView()->ShowWindow(SW_HIDE);

            // Assign the view window.
            m_pActiveView = pView;

            if (m_pActiveView && *this)
            {
                if (!m_pActiveView->IsWindow())
                {
                    // The tab control is already created, so create the new view too.
                    GetActiveView()->Create( GetParent() );
                }

                // Position the view window over the tab control's display area
                CRect rc = GetClientRect();
                AdjustRect(FALSE, &rc);
                MapWindowPoints(GetParent(), rc);
                VERIFY(GetActiveView()->SetWindowPos(HWND_TOP, rc, SWP_SHOWWINDOW));
                GetActiveView()->SetFocus();
            }
        }
    }

    // Displays the list of windows in a popup menu.
    inline void CTab::ShowListMenu()
    {
        if (!m_isListPressed)
        {
            m_isListPressed = TRUE;

            CPoint pt(GetListRect().left, GetListRect().top + GetTabHeight());
            VERIFY(ClientToScreen(pt));

            // Choosing the frame's CWnd for the menu's messages will automatically theme the popup menu.
            int page = 0;
            m_isListMenuActive = TRUE;
            page = GetListMenu().TrackPopupMenuEx(TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD, pt.x, pt.y, GetAncestor(), nullptr) - IDW_FIRSTCHILD;
            if ((page >= 0) && (page < 9)) SelectPage(page);
            if (page == 9) ShowListDialog();
            m_isListMenuActive = FALSE;
        }

        CClientDC dc(*this);
        DrawListButton(dc);
    }

    // Definition of a dialog template that displays a List Box.
    inline void CTab::ShowListDialog()
    {
        // Display the modal dialog. The dialog is defined in the dialog template rather
        // than in the resource script (rc) file.
        CSelectDialog SelectDialog(m_pDlgTemplate);
        for (UINT u = 0; u < GetAllTabs().size(); ++u)
        {
            SelectDialog.AddItem(GetAllTabs()[u].tabText);
        }

        int selected = static_cast<int>(SelectDialog.DoModal(*this));
        if (selected >= 0)
            SelectPage(selected);
    }

    // Swaps the two specified tabs.
    inline void CTab::SwapTabs(int tab1, int tab2)
    {
        int allTabs = static_cast<int>(GetAllTabs().size());
        if ((tab1 < allTabs) && (tab2 < allTabs) && (tab1 != tab2))
        {
            TabPageInfo t1 = GetTabPageInfo(tab1);
            TabPageInfo t2 = GetTabPageInfo(tab2);
            int length = 30;

            TCITEM item1{};
            item1.mask = TCIF_IMAGE | TCIF_PARAM | TCIF_RTLREADING | TCIF_STATE | TCIF_TEXT;
            item1.cchTextMax = length;
            item1.pszText = const_cast<LPTSTR>(t1.tabText.c_str());
            GetItem(tab1, &item1);

            TCITEM item2{};
            item2.mask = TCIF_IMAGE | TCIF_PARAM | TCIF_RTLREADING | TCIF_STATE | TCIF_TEXT;
            item2.cchTextMax = length;
            item2.pszText = const_cast<LPTSTR>(t2.tabText.c_str());
            GetItem(tab2, &item2);

            SetItem(tab1, &item2);
            SetItem(tab2, &item1);
            m_allTabPageInfo[static_cast<size_t>(tab1)] = t2;
            m_allTabPageInfo[static_cast<size_t>(tab2)] = t1;
        }
    }

    // Sets the tabs font size, adjusted for the window's DPI.
    inline void CTab::SetTabsFontSize(int fontSize)
    {
        // Set the font used in the tabs.
        CFont font;
        NONCLIENTMETRICS info = GetNonClientMetrics();
        int dpi = GetWindowDpi(*this);
        LOGFONT lf = info.lfStatusFont;
        lf.lfHeight = -MulDiv(fontSize, dpi, POINTS_PER_INCH);
        font.CreateFontIndirect(lf);
        SetTabFont(font);
        RecalcLayout();
    }

    // Sets the tabs icon size, adjusted for the window's DPI.
    inline void CTab::SetTabsIconSize(int iconSize)
    {
        int iconHeight = DpiScaleInt(iconSize);
        iconHeight = iconHeight - iconHeight % 8;

        const std::vector<TabPageInfo>& v = GetAllTabs();
        GetImages().Create(iconHeight, iconHeight, ILC_MASK | ILC_COLOR32, 0, 0);
        for (size_t i = 0; i < v.size(); ++i)
        {
            // Set the icons for the container parent.
            GetImages().Add(v[i].tabIcon);
        }

        // Assign the ImageList.
        UpdateImageList();

        RecalcLayout();
    }

    // Assigns or removes tab control's image-list as required.
    inline void CTab::UpdateImageList()
    {
        DWORD style = GetStyle();
        if (style & TCS_FIXEDWIDTH && style & TCS_OWNERDRAWFIXED)
            // Remove the image-list to allow very narrow tabs.
            SetImageList(0);
        else
            SetImageList(m_images);
    }

    // Updates the font and icons in the tabs.
    // Called when the dpi changes.
    inline void CTab::UpdateTabs()
    {
        SetTabsFontSize();
        SetTabsIconSize();
    }

    // Provides the default message handling for the tab control.
    inline LRESULT CTab::WndProcDefault(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        switch(msg)
        {
        case WM_PAINT:              return OnPaint(msg, wparam, lparam);
        case WM_ERASEBKGND:         return OnEraseBkgnd(msg, wparam, lparam);
        case WM_KILLFOCUS:          return OnKillFocus(msg, wparam, lparam);
        case WM_LBUTTONDBLCLK:      return OnLButtonDblClk(msg, wparam, lparam);
        case WM_LBUTTONDOWN:        return OnLButtonDown(msg, wparam, lparam);
        case WM_LBUTTONUP:          return OnLButtonUp(msg, wparam, lparam);
        case WM_MOUSEMOVE:          return OnMouseMove(msg, wparam, lparam);
        case WM_MOUSELEAVE:         return OnMouseLeave(msg, wparam, lparam);
        case WM_NCHITTEST:          return OnNCHitTest(msg, wparam, lparam);
        case WM_SETFOCUS:           return OnSetFocus(msg, wparam, lparam);
        case WM_WINDOWPOSCHANGED:   return OnWindowPosChanged(msg, wparam, lparam);
        case WM_WINDOWPOSCHANGING:  return OnWindowPosChanging(msg, wparam, lparam);
        case WM_DPICHANGED_AFTERPARENT: return OnDpiChangedAfterParent(msg, wparam, lparam);
        }

        // Pass unhandled messages on for default processing.
        return CWnd::WndProcDefault(msg, wparam, lparam);
    }

    // Wrappers for Win32 Macros.

    // Calculates a tab control's display area given a window rectangle, or calculates
    //  the window rectangle that would correspond to a specified display area.
    // Refer to TCM_ADJUSTRECT in the Windows API documentation for more information.
    inline void CTab::AdjustRect(BOOL isLarger, RECT *prc) const
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(isLarger);
        LPARAM lparam = reinterpret_cast<LPARAM>(prc);
        SendMessage(TCM_ADJUSTRECT, wparam, lparam);
    }

    // Removes all items from a tab control. Use this function to remove tabs added by InsertItem.
    // Refer to TabCtrl_DeleteAllItems in the Windows API documentation for more information.
    inline BOOL CTab::DeleteAllItems() const
    {
        assert(IsWindow());
        return TabCtrl_DeleteAllItems(*this);
    }

    // Removes an tab from a tab control.
    // Note: Use this function to remove a tab added by InsertItem.
    //       Use RemoveTabPage to remove a tab and page added by AddTabPage.
    // Refer to TabCtrl_DeleteItem in the Windows API documentation for more information.
    inline BOOL CTab::DeleteItem(int tab) const
    {
        assert(IsWindow());
        return TabCtrl_DeleteItem(*this, tab);
    }

    // Resets items in a tab control, clearing any that were set to the TCIS_BUTTONPRESSED state.
    // Refer to TabCtrl_DeselectAll in the Windows API documentation for more information.
    inline void CTab::DeselectAll(UINT excludeFocus) const
    {
        assert(IsWindow());
        TabCtrl_DeselectAll(*this, excludeFocus);
    }

    // Returns the index of the tab that has the focus in a tab control.
    // Refer to TabCtrl_GetCurFocus in the Windows API documentation for more information.
    inline int CTab::GetCurFocus() const
    {
        assert(IsWindow());
        return TabCtrl_GetCurFocus(*this);
    }

    // Determines the currently selected tab in a tab control.
    // Refer to TabCtrl_GetCurSel in the Windows API documentation for more information.
    inline int CTab::GetCurSel() const
    {
        assert(IsWindow());
        return TabCtrl_GetCurSel(*this);
    }

    // Retrieves the extended styles that are currently in use for the tab control.
    // Refer to TabCtrl_GetExtendedStyle in the Windows API documentation for more information.
    inline DWORD CTab::GetExtendedStyle() const
    {
        assert(IsWindow());
        return TabCtrl_GetExtendedStyle(*this);
    }

    // Retrieves the image list associated with a tab control.
    // Refer to TabCtrl_GetImageList in the Windows API documentation for more information.
    inline CImageList CTab::GetImageList() const
    {
        assert(IsWindow());
        HIMAGELIST images = TabCtrl_GetImageList(*this);
        return CImageList(images);
    }

    // Retrieves information about a tab in a tab control.
    // Refer to TabCtrl_GetItem in the Windows API documentation for more information.
    inline BOOL CTab::GetItem(int tab, LPTCITEM pItemInfo) const
    {
        assert(IsWindow());
        return TabCtrl_GetItem(*this, tab, pItemInfo);
    }

    // Retrieves the number of tabs in the tab control.
    // Refer to TabCtrl_GetItemCount in the Windows API documentation for more information.
    inline int CTab::GetItemCount() const
    {
        assert(IsWindow());
        return TabCtrl_GetItemCount(*this);
    }

    // Retrieves the bounding rectangle for a tab in a tab control.
    // Refer to TabCtrl_GetItemRect in the Windows API documentation for more information.
    inline BOOL CTab::GetItemRect(int tab, RECT& rc) const
    {
        assert(IsWindow());
        return TabCtrl_GetItemRect(*this, tab, &rc);
    }

    // Retrieves the current number of rows of tabs in a tab control.
    // Refer to TabCtrl_GetRowCount in the Windows API documentation for more information.
    inline int CTab::GetRowCount() const
    {
        assert(IsWindow());
        return TabCtrl_GetRowCount(*this);
    }

    // Retrieves a handle to the ToolTip control associated with a tab control.
    // Refer to TabCtrl_GetToolTips in the Windows API documentation for more information.
    inline HWND CTab::GetToolTips() const
    {
        assert(IsWindow());
        return TabCtrl_GetToolTips(*this);
    }

    // Sets the highlight state of a tab item.
    // Refer to TabCtrl_HighlightItem in the Windows API documentation for more information.
    inline BOOL CTab::HighlightItem(int tabID, WORD highlight) const
    {
        assert(IsWindow());
        return TabCtrl_HighlightItem(*this, tabID, highlight);
    }

    // Determines which tab, if any, is at a specified screen position.
    // Refer to TabCtrl_HitTest in the Windows API documentation for more information.
    inline int CTab::HitTest(TCHITTESTINFO& hitInfo) const
    {
        assert(IsWindow());
        return TabCtrl_HitTest(*this, &hitInfo);
    }

    // Inserts a new tab in a tab control. Use this to add a tab without a tab page.
    // Refer to TabCtrl_InsertItem in the Windows API documentation for more information.
    inline int CTab::InsertItem(int tab, const LPTCITEM pItemInfo) const
    {
        assert(IsWindow());
        assert(pItemInfo);
        return TabCtrl_InsertItem(*this, tab, pItemInfo);
    }

    // Removes an image from a tab control's image list.
    // Refer to TabCtrl_RemoveImage in the Windows API documentation for more information.
    inline void CTab::RemoveImage(int image) const
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(image);
        TabCtrl_RemoveImage(*this, wparam);
    }

    // Sets the focus to a specified tab in a tab control.
    // Refer to TabCtrl_SetCurFocus in the Windows API documentation for more information.
    inline void CTab::SetCurFocus(int tab) const
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(tab);
        TabCtrl_SetCurFocus(*this, wparam);
    }

    // Selects a tab in a tab control.
    // Refer to TabCtrl_SetCurSel in the Windows API documentation for more information.
    inline int CTab::SetCurSel(int tab) const
    {
        assert(IsWindow());
        return TabCtrl_SetCurSel(*this, tab);
    }

    // Sets the extended styles that the tab control will use.
    // Refer to TabCtrl_SetExtendedStyle in the Windows API documentation for more information.
    inline DWORD CTab::SetExtendedStyle(DWORD dwExStyle) const
    {
        assert(IsWindow());
        LPARAM lparam = static_cast<LPARAM>(dwExStyle);
        return TabCtrl_SetExtendedStyle(*this, lparam);
    }

    // Assigns an image list to a tab control.
    // Refer to TabCtrl_SetImageList in the Windows API documentation for more information.
    inline CImageList CTab::SetImageList(HIMAGELIST newImages)
    {
        assert(IsWindow());
        HIMAGELIST images = TabCtrl_SetImageList( *this, newImages);
        if (newImages != 0)
            m_images = newImages;
        return images;
    }

    // Sets some or all of a tab's attributes.
    // Refer to TabCtrl_SetItem in the Windows API documentation for more information.
    inline BOOL CTab::SetItem(int tab, LPTCITEM pTabInfo) const
    {
        assert(IsWindow());
        assert(pTabInfo);
        return TabCtrl_SetItem(*this, tab, pTabInfo);
    }

    // Sets the number of bytes per tab reserved for application-defined data in a tab control.
    // Refer to TabCtrl_SetItemExtra in the Windows API documentation for more information.
    inline BOOL CTab::SetItemExtra(int cb) const
    {
        assert(IsWindow());
        return TabCtrl_SetItemExtra(*this, cb);
    }

    // Sets the width and height of tabs in a fixed-width or owner-drawn tab control.
    // Refer to TabCtrl_SetItemSize in the Windows API documentation for more information.
    inline DWORD CTab::SetItemSize(int cx, int cy) const
    {
        assert(IsWindow());
        return TabCtrl_SetItemSize(*this, cx, cy);
    }

    // Sets the minimum width of items in a tab control.
    // Refer to TabCtrl_SetMinTabWidth in the Windows API documentation for more information.
    inline int CTab::SetMinTabWidth(int cx) const
    {
        assert(IsWindow());
        return TabCtrl_SetMinTabWidth(*this, cx);
    }

    // Sets the amount of space (padding) around each tab's icon and label in a tab control.
    // Refer to TabCtrl_SetPadding in the Windows API documentation for more information.
    inline void CTab::SetPadding(int cx, int cy) const
    {
        assert(IsWindow());
        TabCtrl_SetPadding(*this, cx, cy);
    }

    // Assigns a ToolTip control to a tab control.
    // Refer to TabCtrl_SetToolTips in the Windows API documentation for more information.
    inline void CTab::SetToolTips(HWND toolTip) const
    {
        assert(IsWindow());
        TabCtrl_SetToolTips(*this, toolTip);
    }

    ////////////////////////////////////////
    // Definitions for the CTabbedMDI class.

    inline CTabbedMDI::CTabbedMDI()
    {
        SetTab(m_tab);
    }

    inline CTabbedMDI::~CTabbedMDI()
    {
    }

    // Adds a MDI tab, given a pointer to the view window, and the tab's text.
    // The framework assumes ownership of the CWnd pointer provided, and deletes
    // the CWnd object when the window is destroyed.
    inline CWnd* CTabbedMDI::AddMDIChild(CWnd* pView, LPCTSTR tabText, int mdiChildID)
    {
        assert(pView); // Cannot add null CWnd*.
        assert(lstrlen(tabText) < WXX_MAX_STRING_SIZE);

        return AddMDIChild(WndPtr(pView), tabText, mdiChildID);
    }

    // Adds a MDI tab, given a pointer to the view window, and the tab's text.
    // The framework assumes ownership of the CWnd pointer provided, and deletes
    // the CWnd object when the window is destroyed.
    inline CWnd* CTabbedMDI::AddMDIChild(WndPtr view, LPCTSTR tabText, int mdiChildID)
    {
        CWnd* pView = view.get();
        assert(pView); // Cannot add null CWnd*.
        assert(lstrlen(tabText) < WXX_MAX_STRING_SIZE);

        GetTab().AddTabPage(std::move(view), tabText, 0U, mdiChildID);

        // Fake a WM_MOUSEACTIVATE to propagate focus change to dockers.
        if (IsWindow())
        {
            WPARAM wparam = reinterpret_cast<WPARAM>(GetAncestor().GetHwnd());
            LPARAM lparam = MAKELPARAM(HTCLIENT, WM_LBUTTONDOWN);
            GetParent().SendMessage(WM_MOUSEACTIVATE, wparam, lparam);
        }

        return pView;
    }

    // Closes the active MDI child.
    inline void CTabbedMDI::CloseActiveMDI()
    {
        int tab = GetTab().GetCurSel();
        if (tab >= 0)
            GetTab().RemoveTabPage(tab);

        RecalcLayout();
    }

    // Closes all MDI children.
    inline void CTabbedMDI::CloseAllMDIChildren()
    {
        while (GetMDIChildCount() > 0)
        {
            GetTab().RemoveTabPage(0);
        }
    }

    // Closes the specified MDI child.
    inline void CTabbedMDI::CloseMDIChild(int tab)
    {
        GetTab().RemoveTabPage(tab);

        if (GetActiveMDIChild())
            GetActiveMDIChild()->RedrawWindow();
    }

    // Creates the TabbedMDI window.
    inline HWND CTabbedMDI::Create(HWND parent /* = 0*/)
    {
        CLIENTCREATESTRUCT clientcreate{};
        clientcreate.hWindowMenu  = *this;
        clientcreate.idFirstChild = IDW_FIRSTCHILD ;
        DWORD style = WS_CHILD | WS_VISIBLE | MDIS_ALLCHILDSTYLES | WS_CLIPCHILDREN;

        // Create the MDICLIENT view window.
        if (!CreateEx(0, _T("MDICLIENT"), _T(""),
            style, 0, 0, 0, 0, parent, nullptr, (PSTR) &clientcreate))
                throw CWinException(GetApp()->MsgWndCreate());

        return *this;
    }

    // Retrieves a pointer to the active MDI child.
    inline CWnd* CTabbedMDI::GetActiveMDIChild() const
    {
        CWnd* pView = nullptr;
        int tab = GetTab().GetCurSel();
        if (tab >= 0)
        {
            TabPageInfo tbi = GetTab().GetTabPageInfo(tab);
            pView = tbi.pView;
        }

        return pView;
    }

    // Retrieves the index of the active MDI child.
    inline int CTabbedMDI::GetActiveMDITab() const
    {
        return GetTab().GetCurSel();
    }

    // Retrieves a pointer to the specified MDI child.
    inline CWnd* CTabbedMDI::GetMDIChild(int tab) const
    {
        assert(tab >= 0);
        assert(tab < GetMDIChildCount());
        return GetTab().GetTabPageInfo(tab).pView;
    }

    // Returns the count of MDI children.
    inline int CTabbedMDI::GetMDIChildCount() const
    {
        return static_cast<int>( GetTab().GetAllTabs().size() );
    }

    // Retrieves the ID of the specified MDI child.
    inline int CTabbedMDI::GetMDIChildID(int tab) const
    {
        assert(tab >= 0);
        assert(tab < GetMDIChildCount());
        return GetTab().GetTabPageInfo(tab).tabID;
    }

    // Retrieves the title of the specified MDI child.
    inline LPCTSTR CTabbedMDI::GetMDIChildTitle(int tab) const
    {
        assert(tab >= 0);
        assert(tab < GetMDIChildCount());
        return GetTab().GetTabPageInfo(tab).tabText;
    }

    // Load the MDI children layout from the registry.
    inline BOOL CTabbedMDI::LoadRegistrySettings(LPCTSTR keyName)
    {
        BOOL isLoaded = FALSE;

        if (keyName)
        {
            const CString mdiKeyName = _T("Software\\") + CString(keyName) + _T("\\MDI Children");
            CRegKey mdiChildKey;
            if (ERROR_SUCCESS == mdiChildKey.Open(HKEY_CURRENT_USER, mdiKeyName))
            {
                DWORD dwTabID;
                int i = 0;
                CString tabKeyName;
                CString TabText;
                tabKeyName.Format(_T("ID%d"), i);

                // Fill the DockList vector from the registry.
                while (ERROR_SUCCESS == mdiChildKey.QueryDWORDValue(tabKeyName, dwTabID))
                {
                    tabKeyName.Format(_T("Text%d"), i);
                    DWORD dwBufferSize = 0;
                    if (ERROR_SUCCESS == mdiChildKey.QueryStringValue(tabKeyName, 0, &dwBufferSize))
                    {
                        int bufferSize = static_cast<int>(dwBufferSize);
                        mdiChildKey.QueryStringValue(tabKeyName, TabText.GetBuffer(bufferSize), &dwBufferSize);
                        TabText.ReleaseBuffer();
                    }

                    int tabID = static_cast<int>(dwTabID);
                    WndPtr child = NewMDIChildFromID(tabID);
                    if (child.get())
                    {
                        AddMDIChild(std::move(child), TabText, tabID);
                        i++;
                        tabKeyName.Format(_T("ID%d"), i);
                        isLoaded = TRUE;
                    }
                    else
                    {
                        TRACE("Failed to get TabbedMDI info from registry");
                        isLoaded = FALSE;
                        break;
                    }
                }

                if (isLoaded)
                {
                    // Load Active MDI Tab from the registry.
                    tabKeyName = _T("Active MDI Tab");
                    DWORD tab;
                    if (ERROR_SUCCESS == mdiChildKey.QueryDWORDValue(tabKeyName, tab))
                        SetActiveMDITab(static_cast<int>(tab));
                    else
                        SetActiveMDITab(0);
                }
            }
        }

        if (!isLoaded)
            CloseAllMDIChildren();

        return isLoaded;
    }

    // Override this function to create new MDI children from IDs.
    inline WndPtr CTabbedMDI::NewMDIChildFromID(int /* mdiChildID */)
    {
        // Override this function to create new MDI children from IDs as shown below
        WndPtr view;
    /*  switch(mdiChildID)
        {
        case ID_SIMPLE:
            view = std::make_unique<CViewSimple>();
            break;
        case ID_RECT:
            view = std::make_unique<CViewRect>();
            break;
        default:
            TRACE("Unknown MDI child ID\n");
            break;
        } */

        return view;
    }

    // Called when the TabbeMDI window is created. (The HWND is attached to CTabbedMDI).
    inline void CTabbedMDI::OnAttach()
    {
        GetTab().Create(*this);
        GetTab().SetShowButtons(TRUE);
        GetTab().SetFixedWidth(TRUE);
        GetTab().SetOwnerDraw(TRUE);
    }

    // Called when the TabbeMDI window is destroyed.
    inline void CTabbedMDI::OnDestroy()
    {
        CloseAllMDIChildren();
    }

    // Handles notifications.
    inline LRESULT CTabbedMDI::OnNotify(WPARAM /*wparam*/, LPARAM lparam)
    {
        LPNMHDR pHeader = reinterpret_cast<LPNMHDR>(lparam);
        assert(pHeader);
        if (pHeader != nullptr)
        {

            switch(pHeader->code)
            {

            case UMN_TABCHANGED:
                RecalcLayout();
                break;

            case UWN_TABDRAGGED:
                {
                    CPoint pt = GetCursorPos();
                    VERIFY(GetTab().ScreenToClient(pt));

                    TCHITTESTINFO info{};
                    info.pt = pt;
                    int tab = GetTab().HitTest(info);
                    if (tab >= 0)
                    {
                        if (tab !=  GetActiveMDITab())
                        {
                            GetTab().SwapTabs(tab, GetActiveMDITab());
                            SetActiveMDITab(tab);
                        }
                    }

                    break;
                }

            case UWN_TABCLOSE:
                {
                    TABNMHDR* pTabNMHDR = reinterpret_cast<TABNMHDR*>(pHeader);
                    return !OnTabClose(pTabNMHDR->page);
                }

            }   // switch(pnmhdr->code)

        }   // if (pHeader == nullptr)

        return 0;
    }

    // Override this function to determine what happens when a tab is about to close.
    // Return TRUE to allow the tab to close, or FALSE to prevent the tab closing.
    inline BOOL CTabbedMDI::OnTabClose(int)
    {
        // Allow the tab to be close
        return TRUE;
    }

    // Called when the tabbedMDI window is resized.
    inline LRESULT CTabbedMDI::OnWindowPosChanged(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        RecalcLayout();
        return FinalWindowProc(msg, wparam, lparam);
    }

    // Repositions the child windows.
    inline void CTabbedMDI::RecalcLayout()
    {
        if (GetTab().IsWindow())
        {
            CRect rcClient = GetClientRect();
            GetTab().SetWindowPos(HWND_TOP, rcClient, SWP_SHOWWINDOW);
        }
    }

    // Saves the MDI children layout in the registry.
    inline BOOL CTabbedMDI::SaveRegistrySettings(LPCTSTR keyName)
    {
        if (keyName)
        {
            const CString appKeyName = _T("Software\\") + CString(keyName);
            const CString mdiChildrenName = _T("MDI Children");
            try
            {
                CRegKey appKey;
                if (ERROR_SUCCESS != appKey.Create(HKEY_CURRENT_USER, appKeyName))
                    throw CUserException();
                if (ERROR_SUCCESS != appKey.Open(HKEY_CURRENT_USER, appKeyName))
                    throw CUserException();

                appKey.DeleteSubKey(mdiChildrenName);
                CRegKey mdiChildKey;
                if (ERROR_SUCCESS != mdiChildKey.Create(appKey, mdiChildrenName))
                    throw CUserException();
                if (ERROR_SUCCESS != mdiChildKey.Open(appKey, mdiChildrenName))
                    throw CUserException();

                for (int i = 0; i < GetMDIChildCount(); ++i)
                {
                    CString tabKeyName;
                    TabPageInfo pdi = GetTab().GetTabPageInfo(i);

                    tabKeyName.Format(_T("ID%d"), i);
                    DWORD tabID = static_cast<DWORD>(pdi.tabID);
                    if (ERROR_SUCCESS != mdiChildKey.SetDWORDValue(tabKeyName, tabID))
                        throw CUserException();

                    tabKeyName.Format(_T("Text%d"), i);
                    CString TabText = GetTab().GetTabPageInfo(i).tabText;
                    if (ERROR_SUCCESS != mdiChildKey.SetStringValue(tabKeyName, TabText))
                        throw CUserException();
                }

                // Add Active Tab to the registry.
                CString tabKeyName = _T("Active MDI Tab");
                DWORD tab = static_cast<DWORD>(GetActiveMDITab());
                if (ERROR_SUCCESS != mdiChildKey.SetDWORDValue(tabKeyName, tab))
                    throw CUserException();
            }
            catch (const CUserException&)
            {
                TRACE("*** Failed to save TabbedMDI settings in registry. ***\n");

                // Roll back the registry changes by deleting the subkeys.
                CRegKey appKey;
                if (ERROR_SUCCESS == appKey.Open(HKEY_CURRENT_USER, appKeyName))
                {
                    appKey.DeleteSubKey(mdiChildrenName);
                }

                return FALSE;
            }
        }

        return TRUE;
    }

    // Makes the specified MDI child active.
    inline void CTabbedMDI::SetActiveMDIChild(CWnd* pWnd) const
    {
        assert(pWnd);
        int page = GetTab().GetTabIndex(pWnd);
        if (page >= 0)
            GetTab().SelectPage(page);
    }

    // Makes the specified MDI child active.
    inline void CTabbedMDI::SetActiveMDITab(int tab) const
    {
        assert(IsWindow());
        assert(GetTab().IsWindow());
        GetTab().SelectPage(tab);
    }

    // Called when the CTabbedMDI window acquires keyboard focus.
    inline LRESULT CTabbedMDI::OnSetFocus(UINT, WPARAM, LPARAM)
    {
        if (GetActiveMDIChild() && GetActiveMDIChild()->IsWindow())
            GetActiveMDIChild()->SetFocus();

        return 0;
    }

    // Provides default handling for the window's messages.
    inline LRESULT CTabbedMDI::WndProcDefault(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        switch(msg)
        {
        case WM_SETFOCUS:           return OnSetFocus(msg, wparam, lparam);
        case WM_WINDOWPOSCHANGED:   return OnWindowPosChanged(msg, wparam, lparam);
        case UWM_GETCTABBEDMDI:     return reinterpret_cast<LRESULT>(this);
        }

        return CWnd::WndProcDefault(msg, wparam, lparam);
    }

} // namespace Win32xx

#endif  // _WIN32XX_TAB_H_
