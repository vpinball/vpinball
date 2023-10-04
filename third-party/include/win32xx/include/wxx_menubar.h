// Win32++   Version 9.4
// Release Date: 25th September 2023
//
//      David Nash
//      email: dnash@bigpond.net.au
//      url: https://sourceforge.net/projects/win32-framework
//
//
// Copyright (c) 2005-2023  David Nash
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


#ifndef _WIN32XX_MENUBAR_H_
#define _WIN32XX_MENUBAR_H_

#include "wxx_wincore.h"
#include "wxx_gdi.h"
#include "wxx_controls.h"
#include "wxx_toolbar.h"





namespace Win32xx
{

    /////////////////////////////////////////////////////////////
    // The CMenuBar class provides a menu inside a rebar control.
    // CMenuBar inherits from CToolBar.
    class CMenuBar : public CToolBar
    {

    public:
        CMenuBar();
        virtual ~CMenuBar();

        void    DrawAllMDIButtons(CDC& drawDC);
        HMENU   GetBarMenu() const {return m_topMenu;}
        BOOL    IsAltMode() const { return m_isAltMode; }
        LRESULT MenuChar(UINT msg, WPARAM wparam, LPARAM lparam);
        void    SetupMenuBar(HMENU menu);
        LRESULT SysCommand(UINT msg, WPARAM wparam, LPARAM lparam);

    protected:
        // Overridables
        virtual void OnAttach();
        virtual LRESULT OnDrawItem(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual LRESULT OnExitMenuLoop(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual LRESULT OnInitMenuPopup(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual LRESULT OnKeyDown(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual LRESULT OnKillFocus(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual LRESULT OnLButtonDown(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual LRESULT OnLButtonUp(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual LRESULT OnMeasureItem(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual LRESULT OnMenuChar(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual BOOL    OnMenuInput(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual LRESULT OnMouseLeave(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual LRESULT OnMouseMove(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual LRESULT OnNotifyReflect(WPARAM wparam, LPARAM lparam);
        virtual LRESULT OnSysCommand(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual LRESULT OnSysKeyDown(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual LRESULT OnSysKeyUp(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual LRESULT OnTBNDropDown(LPNMTOOLBAR pNMTB);
        virtual LRESULT OnTBNHotItemChange(LPNMTBHOTITEM pNMHI);
        virtual LRESULT OnWindowPosChanged(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual LRESULT OnWindowPosChanging(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual void    PreCreate(CREATESTRUCT& cs);
        virtual void    PreRegisterClass(WNDCLASS& wc);
        virtual BOOL    PreTranslateMessage(MSG& msg);

        // Not intended to be overridden
        LRESULT WndProcDefault(UINT msg, WPARAM wparam, LPARAM lparam);

    private:
        CMenuBar(const CMenuBar&);              // Disable copy construction
        CMenuBar& operator = (const CMenuBar&); // Disable assignment operator

        void Press(int buttonID, BOOL press) const
        {
            PressButton(static_cast<UINT>(buttonID), press);
        }

        void Cancel() const;
        void DoAltKey(WORD keyCode);
        void DrawMDIButton(CDC& drawDC, int button, UINT state) const;
        void ExitMenu();
        CWnd* GetActiveMDIChild() const;
        CWnd* GetMDIClient() const;
        void GrabFocus();
        BOOL IsMDIChildMaxed() const;
        BOOL IsMDIFrame() const;
        LRESULT OnPopupMenu();
        void ReleaseFocus();
        void StoreHotItem(int hotItem);
        void UpdateMDIButtons(WPARAM wparam, LPARAM lparam) const;
        static LRESULT CALLBACK StaticMsgHook(int code, WPARAM wparam, LPARAM lparam);

        enum MDIButtonType
        {
            MDI_MIN = 0,
            MDI_RESTORE = 1,
            MDI_CLOSE = 2
        };

        BOOL  m_isAltMode;      // Alt key toggled
        BOOL  m_isExitAfter;    // Exit after Popup menu ends
        BOOL  m_isKeyMode;      // keyboard navigation mode
        BOOL  m_isMenuActive;   // popup menu active
        BOOL  m_isSelPopup;     // a popup (cascade) menu is selected
        HMENU m_popupMenu;      // handle to the popup menu
        HMENU m_selMenu;        // handle to the cascaded popup menu
        HMENU m_topMenu;        // handle to the top level menu
        HWND  m_prevFocus;      // handle to window that had focus
        CRect m_mdiRect[3];     // array of CRect for MDI buttons
        int   m_hotItem;        // hot item
        CPoint m_oldMousePos;   // old Mouse position

    };  // class CMenuBar

}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


namespace Win32xx
{

    /////////////////////////////////////
    // Definitions for the CMenuBar class
    //

    inline CMenuBar::CMenuBar()
    {
        m_isExitAfter  = FALSE;
        m_topMenu      = 0;
        m_hotItem      = -1;
        m_isSelPopup   = FALSE;
        m_selMenu      = 0;
        m_isMenuActive = FALSE;
        m_isKeyMode    = FALSE;
        m_isAltMode    = FALSE;
        m_prevFocus    = 0;
        m_popupMenu    = 0;
    }

    inline CMenuBar::~CMenuBar()
    {
    }

    // Cancel certain modes, such as mouse capture.
    inline void CMenuBar::Cancel() const
    {
        SendMessage(WM_CANCELMODE, 0, 0);
    }

    // Handle key pressed with Alt held down
    inline void CMenuBar::DoAltKey(WORD keyCode)
    {
        TCHAR key = static_cast<TCHAR>(keyCode);
        int item = MapAccelerator(key);
        if (item >= 0)
        {
            GrabFocus();
            m_isKeyMode = TRUE;
            m_isAltMode = TRUE;
            StoreHotItem(item);
            PostMessage(UWM_POPUPMENU, 0, 0);
        }
        else
            ::MessageBeep(MB_OK);
    }

    // Draws all the MDI buttons on a MDI frame with a maximized MDI child.
    inline void CMenuBar::DrawAllMDIButtons(CDC& drawDC)
    {
        if (!IsMDIFrame())
            return;

        if (IsMDIChildMaxed())
        {
            int cx = ::GetSystemMetrics(SM_CXSMICON) * GetWindowDpi(*this) / GetWindowDpi(HWND_DESKTOP);
            int cy = ::GetSystemMetrics(SM_CYSMICON) * GetWindowDpi(*this) / GetWindowDpi(HWND_DESKTOP);
            CRect rc = GetClientRect();
            int gap = 4;
            rc.right -= gap;

            // Assign values to each element of the CRect array
            for (int i = 0 ; i < 3 ; ++i)
            {
                int left = rc.right - (i+1)*cx - gap*(i+1);
                int top = rc.bottom/2 - cy/2;
                int right = rc.right - i*cx - gap*(i+1);
                int bottom = rc.bottom/2 + cy/2;
                VERIFY(::SetRect(&m_mdiRect[2 - i], left, top, right, bottom));
            }

            // Hide the MDI button if it won't fit
            for (int k = 0 ; k <= 2 ; ++k)
            {

                if (m_mdiRect[k].left < GetMaxSize().cx)
                {
                    VERIFY(::SetRectEmpty(&m_mdiRect[k]));
                }
            }

            DrawMDIButton(drawDC, MDI_MIN, 0);
            DrawMDIButton(drawDC, MDI_RESTORE, 0);
            DrawMDIButton(drawDC, MDI_CLOSE, 0);
        }
    }

    // Draws an individual MDI button.
    inline void CMenuBar::DrawMDIButton(CDC& drawDC, int button, UINT state) const
    {
        if (!IsRectEmpty(&m_mdiRect[button]))
        {
            // Use the Marlett font to draw special characters
            CFont marlett;
            marlett.CreatePointFont(100, _T("Marlett"));
            drawDC.SetBkMode(TRANSPARENT);
            LOGFONT lf = DpiScaleLogfont(marlett.GetLogFont(), 10);
            drawDC.CreateFontIndirect(lf);

            COLORREF grey(GetSysColor(COLOR_BTNFACE));
            COLORREF black(RGB(0, 0, 0));
            COLORREF white(RGB(255, 255, 255));

            switch (state)
            {
            case 0:
                // Draw a grey box for the normal button to erase other highlighting.
                drawDC.SetTextColor(grey);
                drawDC.DrawText(_T("c"), 1, m_mdiRect[button], DT_CENTER | DT_SINGLELINE | DT_VCENTER);
                drawDC.DrawText(_T("d"), 1, m_mdiRect[button], DT_CENTER | DT_SINGLELINE | DT_VCENTER);
                break;
            case 1:
                // Draw popped up button, black on right and bottom.
                drawDC.SetTextColor(white);
                drawDC.DrawText(_T("c"), 1, m_mdiRect[button], DT_CENTER | DT_SINGLELINE | DT_VCENTER);
                drawDC.SetTextColor(black);
                drawDC.DrawText(_T("d"), 1, m_mdiRect[button], DT_CENTER | DT_SINGLELINE | DT_VCENTER);
                break;
            case 2:
                // Draw pressed button, black on left and top.
                drawDC.SetTextColor(black);
                drawDC.DrawText(_T("c"), 1, m_mdiRect[button], DT_CENTER | DT_SINGLELINE | DT_VCENTER);
                drawDC.SetTextColor(white);
                drawDC.DrawText(_T("d"), 1, m_mdiRect[button], DT_CENTER | DT_SINGLELINE | DT_VCENTER);
                break;
            }

            switch (button)
            {
            case MDI_MIN:
            {
                // Draw the minimize button.
                CRect rc = m_mdiRect[0];
                rc.OffsetRect(DpiScaleInt(1), 0);

                drawDC.SetTextColor(black);
                drawDC.DrawText(_T("0"), 1, rc, DT_CENTER | DT_SINGLELINE | DT_VCENTER);
                break;
            }
            case MDI_RESTORE:
            {
                // Draw the restore button.
                CRect rc = m_mdiRect[1];
                rc.OffsetRect(1, 0);

                drawDC.SetTextColor(black);
                drawDC.DrawText(_T("2"), 1, rc, DT_CENTER | DT_SINGLELINE | DT_VCENTER);
                break;
            }
            case MDI_CLOSE:
            {
                // Draw the close button (a Marlett "r" looks like "X").
                drawDC.SetTextColor(black);
                drawDC.DrawText(_T("r"), 1, m_mdiRect[2], DT_CENTER | DT_SINGLELINE | DT_VCENTER);
                break;
            }

            }
        }
    }

    // Used when a popup menu is closed.
    inline void CMenuBar::ExitMenu()
    {
        ReleaseFocus();
        m_isKeyMode = FALSE;
        m_isMenuActive = FALSE;
        Press(m_hotItem, FALSE);
        StoreHotItem(-1);

        CPoint pt = GetCursorPos();
        VERIFY(ScreenToClient(pt));

        // Update mouse mouse position for hot tracking.
        SendMessage(WM_MOUSEMOVE, 0, MAKELONG(pt.x, pt.y));
    }

    // Retrieves a pointer to the active MDI child if any.
    inline CWnd* CMenuBar::GetActiveMDIChild() const
    {
        CWnd* pMDIChild = NULL;
        if (GetMDIClient())
        {
            HWND mdiChild = reinterpret_cast<HWND>(GetMDIClient()->SendMessage(WM_MDIGETACTIVE, 0, 0));
            pMDIChild = GetCWndPtr(mdiChild);
        }

        return pMDIChild;
    }

    // Retrieves a pointer to the MDIClient. Returns NULL if there
    // is no MDIClient.
    inline CWnd* CMenuBar::GetMDIClient() const
    {
        CWnd* pMDIClient = NULL;

        // Is the ancestor a CMDIFrame?
        if (GetAncestor().SendMessage(UWM_GETCMDIFRAMET) != 0)
        {
            // Retrieve the window handle of the MDI frame's view.
            HWND wnd = reinterpret_cast<HWND>(GetAncestor().SendMessage(UWM_GETFRAMEVIEW));
            pMDIClient = GetCWndPtr(wnd);
        }

        return pMDIClient;
    }

    // Stores the old focus and captures mouse input.
    inline void CMenuBar::GrabFocus()
    {
        if (::GetFocus() != *this)
            m_prevFocus = ::SetFocus(*this);
        ::SetCapture(*this);
        ::SetCursor(::LoadCursor(0, IDC_ARROW));
    }

    // Returns TRUE of the MDI child is maximized.
    inline BOOL CMenuBar::IsMDIChildMaxed() const
    {
        BOOL isMaxed = FALSE;
        if (GetMDIClient())
        {
            LPARAM lparam = reinterpret_cast<LPARAM>(&isMaxed);
            GetMDIClient()->SendMessage(WM_MDIGETACTIVE, 0, lparam);
        }

        return isMaxed;
    }

    // Returns TRUE if the frame is a MDI frame.
    inline BOOL CMenuBar::IsMDIFrame() const
    {
        return static_cast<BOOL>(GetAncestor().SendMessage(UWM_GETCMDIFRAMET));
    }

    inline LRESULT CMenuBar::OnMenuChar(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        TCHAR keyCode = static_cast<TCHAR>(LOWORD(wparam));
        if (!m_isMenuActive)
            DoAltKey(keyCode);

        return FinalWindowProc(msg, wparam, lparam);
    }

    inline LRESULT CMenuBar::MenuChar(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        return OnMenuChar(msg, wparam, lparam);
    }

    // Called when the window handle (HWND) is attached to this object.
    inline void CMenuBar::OnAttach()
    {
        // We must send this message before sending the TB_ADDBITMAP or TB_ADDBUTTONS message
        SendMessage(TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0);
    }

    // Forwards owner drawing to the frame.
    inline LRESULT CMenuBar::OnDrawItem(UINT, WPARAM wparam, LPARAM lparam)
    {
        GetAncestor().SendMessage(WM_DRAWITEM, wparam, lparam);
        return TRUE; // handled
    }

    // Called when a popup menu is closed.
    inline LRESULT CMenuBar::OnExitMenuLoop(UINT, WPARAM wparam, LPARAM lparam)
    {
        if (m_isExitAfter)
            ExitMenu();
        GetAncestor().SendMessage(WM_EXITMENULOOP, wparam, lparam);

        return 0;
    }

    // Called when a popup menu is created.
    inline LRESULT CMenuBar::OnInitMenuPopup(UINT, WPARAM wparam, LPARAM lparam)
    {
        GetAncestor().SendMessage(WM_INITMENUPOPUP, wparam, lparam);
        return 0;
    }

    // Called when a key is pressed while the menubar has the mouse captured.
    inline LRESULT CMenuBar::OnKeyDown(UINT, WPARAM wparam, LPARAM)
    {
        int first = IsMDIChildMaxed() ? 1 : 0;
        TCHAR keycode = static_cast<TCHAR>(wparam);

        switch (keycode)
        {
        case VK_ESCAPE:
            m_isAltMode = FALSE;
            ExitMenu();
            break;

        case VK_SPACE:
            m_isAltMode = FALSE;
            ExitMenu();
            // Bring up the system menu
            GetAncestor().PostMessage(WM_SYSCOMMAND, SC_KEYMENU, VK_SPACE);
            break;

        // Handle VK_DOWN, and VK_UP together
        case VK_DOWN:
        case VK_UP:
            // Always use PostMessage for USER_POPUPMENU (not SendMessage)
            PostMessage(UWM_POPUPMENU, 0, 0);
            break;
        case VK_RETURN:
            m_isAltMode = FALSE;
            // Always use PostMessage for USER_POPUPMENU (not SendMessage)
            PostMessage(UWM_POPUPMENU, 0, 0);
            break;

        case VK_LEFT:
            // Move left to next topmenu item
            if (m_hotItem > first)
                StoreHotItem(m_hotItem -1);
            else
                StoreHotItem(GetButtonCount() -1);

            break;

        case VK_RIGHT:
            // Move right to next topmenu item
            if (m_hotItem < GetButtonCount() -1)
                StoreHotItem(m_hotItem +1);
            else
                StoreHotItem(first);

            break;

        default:
            // Handle Accelerator keys with Alt toggled down
            if (m_isKeyMode)
            {
                int item = MapAccelerator(keycode);
                if (item >= 0)
                {
                    m_hotItem = item;
                    PostMessage(UWM_POPUPMENU, 0, 0);
                }
                else
                    ::MessageBeep(MB_OK);
            }
            break;
        } // switch (wparam)

        return 0;  // Discard these messages
    }

    // Called when the menu bar loses focus.
    inline LRESULT CMenuBar::OnKillFocus(UINT, WPARAM, LPARAM)
    {
        m_isAltMode = FALSE;
        ExitMenu();
        return 0;
    }

    // Called when the left mouse button is pressed.
    inline LRESULT CMenuBar::OnLButtonDown(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        ExitMenu();

        // Updates hot item with current mouse position.
        FinalWindowProc(WM_MOUSEMOVE, wparam, lparam);

        // Do default processing first.
        FinalWindowProc(msg, wparam, lparam);

        if (IsMDIFrame())
        {
            if (IsMDIChildMaxed())
            {
                int mdiButton = -1;
                CPoint pt(lparam);

                if (m_mdiRect[0].PtInRect(pt)) mdiButton = 0;
                if (m_mdiRect[1].PtInRect(pt)) mdiButton = 1;
                if (m_mdiRect[2].PtInRect(pt)) mdiButton = 2;

                // Draw the three MDI buttons.
                if (mdiButton >= 0)
                {
                    CClientDC MenuBarDC(*this);
                    DrawMDIButton(MenuBarDC, MDI_MIN,     (mdiButton == 0) ? 2U : 0U);
                    DrawMDIButton(MenuBarDC, MDI_RESTORE, (mdiButton == 1) ? 2U : 0U);
                    DrawMDIButton(MenuBarDC, MDI_CLOSE,   (mdiButton == 2) ? 2U : 0U);
                }

                // Bring up the MDI Child window's system menu when the icon is pressed
                if (HitTest() == 0)
                {
                    m_hotItem = 0;
                    PostMessage(UWM_POPUPMENU, 0, 0);
                }
            }
        }

        return 0;
    }

    // Called when a key is released.
    inline LRESULT CMenuBar::OnLButtonUp(UINT, WPARAM, LPARAM)
    {
        if (IsMDIFrame())
        {
            CWnd* pMDIClient = GetMDIClient();
            CWnd* pMDIChild = GetActiveMDIChild();
            assert(pMDIClient);

            if (pMDIChild && pMDIClient && IsMDIChildMaxed())
            {
                CPoint pt = GetCursorPos();
                VERIFY(ScreenToClient(pt));

                // Process the MDI button action when the left mouse button is up
                if (m_mdiRect[0].PtInRect(pt))
                {
                     pMDIChild->ShowWindow(SW_MINIMIZE);
                }

                if (m_mdiRect[1].PtInRect(pt))
                {
                    WPARAM wparam = reinterpret_cast<WPARAM>(pMDIChild->GetHwnd());
                    pMDIClient->PostMessage(WM_MDIRESTORE, wparam, 0);
                }

                if (m_mdiRect[2].PtInRect(pt))
                {
                    pMDIChild->PostMessage(WM_SYSCOMMAND, SC_CLOSE, 0);
                }
            }
        }

        ExitMenu();

        return 0;
    }

    // Forwards the owner draw processing to the frame.
    inline LRESULT CMenuBar::OnMeasureItem(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        GetAncestor().SendMessage(msg, wparam, lparam);
        return TRUE; // handled
    }

    // When a popup menu is active, StaticMsgHook directs all menu messages here
    inline BOOL CMenuBar::OnMenuInput(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        int first = (IsMDIChildMaxed() ? 1 : 0);

        switch(msg)
        {
        case WM_KEYDOWN:
            m_isExitAfter = FALSE;
            {
                switch (wparam)
                {
                case VK_ESCAPE:
                    // Use default processing if inside a Sub Menu
                    if ((m_selMenu) &&(m_selMenu != m_popupMenu))
                        return FALSE;

                    m_isMenuActive = FALSE;
                    m_isKeyMode = TRUE;
                    Cancel();
                    Press(m_hotItem, FALSE);
                    SetHotItem(m_hotItem);
                    ExitMenu();
                    break;

                case VK_LEFT:
                {
                    // Use default processing if inside a Sub Menu
                    if ((m_selMenu) && (m_selMenu != m_popupMenu))
                        return FALSE;

                    Press(m_hotItem, FALSE);

                    // Move left to next topmenu item
                    m_hotItem = (m_hotItem > first) ? m_hotItem -1 : GetButtonCount() -1;
                    Cancel();

                    // Always use PostMessage for USER_POPUPMENU (not SendMessage)
                    PostMessage(UWM_POPUPMENU, 0, 0);
                    PostMessage(WM_KEYDOWN, VK_DOWN, 0);
                    break;
                }

                case VK_RIGHT:
                {
                    // Use default processing to open Sub Menu
                    if (m_isSelPopup)
                        return FALSE;

                    Press(m_hotItem, FALSE);

                    // Move right to next topmenu item
                    m_hotItem = (m_hotItem < GetButtonCount() -1) ? m_hotItem +1 : first;
                    Cancel();

                    // Always use PostMessage for USER_POPUPMENU (not SendMessage)
                    PostMessage(UWM_POPUPMENU, 0, 0);
                    PostMessage(WM_KEYDOWN, VK_DOWN, 0);
                    break;
                }

                case VK_RETURN:
                    m_isExitAfter = TRUE;
                    break;

                } // switch (wparam)

            } // case WM_KEYDOWN

            return FALSE;

        case WM_CHAR:
            m_isExitAfter = TRUE;
            return FALSE;

        case WM_LBUTTONDOWN:
            {
                m_isExitAfter = TRUE;
                if (HitTest() >= 0)
                {
                    // Cancel popup when we hit a button a second time.
                    Cancel();
                    return TRUE;
                }
            }
            return FALSE;

        case WM_LBUTTONDBLCLK:
            // Perform default action for DblClick on MDI Maxed icon.
            if (IsMDIChildMaxed() && (0 == HitTest()))
            {
                CWnd* pMDIChild = GetActiveMDIChild();
                assert(pMDIChild);
                if (pMDIChild)
                {
                    CMenu childMenu = pMDIChild->GetSystemMenu(FALSE);

                    UINT id = childMenu.GetDefaultItem(FALSE, 0);
                    if (id)
                        pMDIChild->PostMessage(WM_SYSCOMMAND, static_cast<WPARAM>(id), 0);
                }
            }

            m_isExitAfter = TRUE;
            return FALSE;

        case WM_MENUSELECT:
            {
                // store info about selected item
                m_selMenu = reinterpret_cast<HMENU>(lparam);
                m_isSelPopup = ((HIWORD(wparam) & MF_POPUP)) ? TRUE : FALSE;

                // Reflect message back to the frame window
                GetAncestor().SendMessage(WM_MENUSELECT, wparam, lparam);
            }
            return TRUE;

        case WM_MOUSEMOVE:
            {
                CPoint pt;
                pt.x = GET_X_LPARAM(lparam);
                pt.y = GET_Y_LPARAM(lparam);

                // Skip if mouse hasn't moved
                if ((pt.x == m_oldMousePos.x) && (pt.y == m_oldMousePos.y))
                    return FALSE;

                m_oldMousePos.x = pt.x;
                m_oldMousePos.y = pt.y;
                VERIFY(ScreenToClient(pt));

                // Reflect messages back to the MenuBar for hot tracking
                SendMessage(WM_MOUSEMOVE, 0, MAKELPARAM(pt.x, pt.y));
            }
            break;

        }
        return FALSE;
    }

    // Called when the cursor leave the client area of the window.
    inline LRESULT CMenuBar::OnMouseLeave(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        if (IsMDIFrame())
        {
            if (IsMDIChildMaxed())
            {
                CClientDC MenuBarDC(*this);

                DrawMDIButton(MenuBarDC, MDI_MIN,     0);
                DrawMDIButton(MenuBarDC, MDI_RESTORE, 0);
                DrawMDIButton(MenuBarDC, MDI_CLOSE,   0);
            }
        }

        return FinalWindowProc(msg, wparam, lparam);
    }

    // Called when the cursor moves.
    inline LRESULT CMenuBar::OnMouseMove(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        UpdateMDIButtons(wparam, lparam);
        return FinalWindowProc(msg, wparam, lparam);
    }

    // Process the menubar's notifications.
    inline LRESULT CMenuBar::OnNotifyReflect(WPARAM, LPARAM lparam)
    {
        LPNMHDR pHeader = reinterpret_cast<LPNMHDR>(lparam);
        switch (pHeader->code)
        {
        case TBN_DROPDOWN:      return OnTBNDropDown((LPNMTOOLBAR) lparam);
        case TBN_HOTITEMCHANGE: return OnTBNHotItemChange((LPNMTBHOTITEM) lparam);
        }

        return 0;
    }

    // Display the popup menu for a menu item. This function doesn't complete until
    // the popup menu is closed.
    inline LRESULT CMenuBar::OnPopupMenu()
    {
        if (m_isKeyMode)
            // Simulate a down arrow key press.
            PostMessage(WM_KEYDOWN, VK_DOWN, 0);

        m_isKeyMode = FALSE;
        m_isExitAfter = FALSE;
        m_oldMousePos = GetCursorPos();

        CWnd* pMaxMDIChild = NULL;
        if (IsMDIChildMaxed())
            pMaxMDIChild = GetActiveMDIChild();

        // Load the submenu.
        int maxedOffset = IsMDIChildMaxed()? 1:0;
        m_popupMenu = ::GetSubMenu(m_topMenu, m_hotItem - maxedOffset);
        if (pMaxMDIChild && IsMDIChildMaxed() && (m_hotItem == 0) )
            m_popupMenu = pMaxMDIChild->GetSystemMenu(FALSE);

        // Retrieve the bounding rectangle for the toolbar button.
        CRect rc = GetItemRect(m_hotItem);

        // Convert rectangle to desktop coordinates.
        VERIFY(ClientToScreen(rc));

        // Position popup above toolbar if it won't fit below.
        TPMPARAMS tpm;
        tpm.cbSize = sizeof(tpm);
        tpm.rcExclude = rc;

        // Set the hot button
        SetHotItem(m_hotItem);
        Press(m_hotItem, TRUE);

        m_isSelPopup = FALSE;
        m_selMenu = 0;
        m_isMenuActive = TRUE;

        // We hook mouse input to process mouse and keyboard input during
        //  the popup menu. Messages are sent to StaticMsgHook.

        // Remove any remaining hook first.
        TLSData* pTLSData = GetApp()->GetTlsData();
        pTLSData->pMenuBar = this;
        if (pTLSData->msgHook != NULL)
            ::UnhookWindowsHookEx(pTLSData->msgHook);

        // Hook messages about to be processed by the shortcut menu.
        pTLSData->msgHook = ::SetWindowsHookEx(WH_MSGFILTER, (HOOKPROC)StaticMsgHook, NULL, ::GetCurrentThreadId());

        // Display the shortcut menu.
        bool isRightToLeft = false;

#if (WINVER >= 0x0500)
        isRightToLeft = (((GetAncestor().GetExStyle()) & WS_EX_LAYOUTRTL)) != 0;
#endif

        int xPos = isRightToLeft? rc.right : rc.left;
        UINT id = static_cast<UINT>(::TrackPopupMenuEx(m_popupMenu, TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_VERTICAL,
                                       xPos, rc.bottom, *this, &tpm));

        // We get here once the TrackPopupMenuEx has ended.
        m_isMenuActive = FALSE;

        // Remove the message hook
        ::UnhookWindowsHookEx(pTLSData->msgHook);
        pTLSData->msgHook = NULL;

        // Process MDI Child system menu.
        if (IsMDIChildMaxed())
        {
            if (pMaxMDIChild && pMaxMDIChild->GetSystemMenu(FALSE) == m_popupMenu )
            {
                if (id)
                {
                    WPARAM wparam = static_cast<WPARAM>(id);
                    pMaxMDIChild->SendMessage(WM_SYSCOMMAND, wparam, 0);
                }
            }
        }

        SetHotItem(-1);

        // Unpress any currently pressed buttons.
        for (int i = 0; i < GetButtonCount(); ++i)
        {
            id = GetCommandID(i);
            PressButton(id, FALSE);
        }

        // Re-establish Focus.
        if (m_isKeyMode)
            GrabFocus();

        return 0;
    }

    inline LRESULT CMenuBar::OnSysCommand(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        if (SC_KEYMENU == wparam)
        {
            if (lparam == 0)
            {
                // Alt/F10 key toggled
                GrabFocus();
                m_isKeyMode = TRUE;
                m_isAltMode = TRUE;
                int maxedOffset = (IsMDIChildMaxed()? 1:0);
                StoreHotItem(maxedOffset);
            }
            else
                // Handle key pressed with Alt held down
                DoAltKey(static_cast<WORD>(lparam));
        }

        return FinalWindowProc(msg, wparam, lparam);
    }

    inline LRESULT CMenuBar::OnSysKeyDown(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        if ((m_isKeyMode) && ((VK_MENU == wparam) || (VK_F10 == wparam)))
        {
            StoreHotItem(-1);
            return 0;
        }

        return FinalWindowProc(msg, wparam, lparam);
    }

    inline LRESULT CMenuBar::OnSysKeyUp(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        ExitMenu();

        if ((m_isKeyMode) && ((VK_MENU == wparam) || (VK_F10 == wparam)))
        {
            return 0;
        }

        m_isAltMode = FALSE;
        RedrawWindow();
        return FinalWindowProc(msg, wparam, lparam);
    }

    inline LRESULT CMenuBar::OnTBNDropDown(LPNMTOOLBAR pNMTB)
    {
        m_hotItem = pNMTB->iItem;

        // Always use PostMessage for USER_POPUPMENU (not SendMessage).
        PostMessage(UWM_POPUPMENU, 0, 0);

        return 0;
    }

    // Called when a hot item change is about to occur.
    // This is used to bring up a new popup menu when required.
    inline LRESULT CMenuBar::OnTBNHotItemChange(LPNMTBHOTITEM pNMHI)
    {
        CPoint pt = GetCursorPos();
        if (*this == ::WindowFromPoint(pt)) // MenuBar window must be on top.
        {
            DWORD flag = pNMHI->dwFlags;
            if ((flag & HICF_MOUSE) && !(flag & HICF_LEAVING))
            {
                int button = HitTest();
                if ((m_isMenuActive) && (button != m_hotItem))
                {
                    Press(m_hotItem, FALSE);
                    m_hotItem = button;
                    Cancel();

                    //Always use PostMessage for USER_POPUPMENU (not SendMessage).
                    PostMessage(UWM_POPUPMENU, 0, 0);
                }
                m_hotItem = button;
            }

            // Handle escape from popup menu.
            if ((flag & HICF_LEAVING) && m_isKeyMode)
            {
                m_hotItem = pNMHI->idOld;
                WPARAM wparam = static_cast<WPARAM>(m_hotItem);
                PostMessage(TB_SETHOTITEM, wparam, 0);
            }
        }

        return 0;
    }

    // Called when the menubar has been resized.
    inline LRESULT CMenuBar::OnWindowPosChanged(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        InvalidateRect(m_mdiRect[0], TRUE);
        InvalidateRect(m_mdiRect[1], TRUE);
        InvalidateRect(m_mdiRect[2], TRUE);
        {
            CClientDC MenuBarDC(*this);
            DrawAllMDIButtons(MenuBarDC);
        }

        RedrawWindow();
        return FinalWindowProc(msg, wparam, lparam);
    }

    inline LRESULT CMenuBar::OnWindowPosChanging(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        // Bypass CToolBar::WndProcDefault for this message.
        return FinalWindowProc(msg, wparam, lparam);
    }

    // Set the window style, before it is created.
    inline void CMenuBar::PreCreate(CREATESTRUCT& cs)
    {
        cs.style = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | TBSTYLE_TOOLTIPS | TBSTYLE_LIST | TBSTYLE_FLAT | CCS_NODIVIDER | CCS_NORESIZE;
    }

    // Set the window class,
    inline void CMenuBar::PreRegisterClass(WNDCLASS& wc)
    {
        wc.lpszClassName =  TOOLBARCLASSNAME;
    }

    inline BOOL CMenuBar::PreTranslateMessage(MSG& msg)
    {
        // Discard mouse move messages unless over a button.
        if (m_isKeyMode && msg.message == WM_MOUSEMOVE)
        {
            UpdateMDIButtons(msg.wParam, msg.lParam);
            int first = (IsMDIChildMaxed() ? 1 : 0);
            if (HitTest() < first)
                return TRUE;
        }

        // Forward mouse left button down to the Ancestor window
        // if pressed on it's non client area.
        if (m_isKeyMode && msg.message == WM_LBUTTONDOWN)
        {
            CPoint screenPos = GetCursorPos();
            CPoint clientPos = screenPos;
            VERIFY(GetAncestor().ScreenToClient(clientPos));
            CRect rcWindow = GetAncestor().GetWindowRect();
            VERIFY(GetAncestor().ScreenToClient(rcWindow));
            CRect rcClient = GetAncestor().GetClientRect();

            if (rcWindow.PtInRect(clientPos) && !rcClient.PtInRect(clientPos))
            {
                ReleaseFocus();

                // Pass the Left Mouse Click back up to the frame window (and update cursor).
                LPARAM lparam = MAKELPARAM(screenPos.x, screenPos.y);
                HWND frame = GetAncestor();
                WPARAM wparam = static_cast<WPARAM>(::SendMessage(frame, WM_NCHITTEST, 0, lparam));
                ::SendMessage(frame, WM_NCMOUSEMOVE, wparam, lparam);
                ::SendMessage(frame, WM_SETCURSOR, reinterpret_cast<WPARAM>(frame),
                              MAKELPARAM(wparam, WM_NCMOUSEMOVE));

                ::SendMessage(frame, WM_NCLBUTTONDOWN, wparam, lparam);
            }
        }

        return FALSE;
    }

    // Releases mouse capture and returns keyboard focus.
    inline void CMenuBar::ReleaseFocus()
    {
        if (m_prevFocus)
            ::SetFocus(m_prevFocus);

        m_prevFocus = 0;
        ::ReleaseCapture();
    }

    // Set the menubar's (toolbar's) hot item.
    inline void CMenuBar::StoreHotItem(int hotItem)
    {
        m_hotItem = hotItem;
        SetHotItem(m_hotItem);
        RedrawWindow();
    }

    // Builds the list of menubar (toolbar) buttons from the top level menu.
    inline void CMenuBar::SetupMenuBar(HMENU menu)
    {
        assert(IsWindow());

        m_topMenu = menu;
        int maxedOffset = (IsMDIChildMaxed()? 1:0);

        // Remove any existing buttons.
        while (GetButtonCount() > 0)
        {
            if (!DeleteButton(0))
                break;
        }

        // Set the Bitmap size to zero.
        SetBitmapSize(0, 0);

        if (IsMDIChildMaxed())
        {
            // Create an extra button for the MDI child system menu.
            // Later we will custom draw the window icon over this button.
            TBBUTTON tbb;
            ZeroMemory(&tbb, sizeof(tbb));
            tbb.fsState = TBSTATE_ENABLED;
            tbb.fsStyle = TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE ;
            tbb.iString = reinterpret_cast<INT_PTR>(_T(" "));
            AddButtons(1, &tbb);
            SetButtonText(0, _T("    "));
        }

        for (int i = 0 ; i < ::GetMenuItemCount(menu); ++i)
        {
            // Assign the ToolBar Button struct.
            TBBUTTON tbb;
            ZeroMemory(&tbb, sizeof(tbb));
            tbb.idCommand = i  + maxedOffset;  // Each button needs a unique ID.
            tbb.fsState = TBSTATE_ENABLED;
            tbb.fsStyle = TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE | TBSTYLE_DROPDOWN;
            tbb.iString = reinterpret_cast<INT_PTR>(_T(" "));
            AddButtons(1, &tbb);

            // Add the menu title to the string table.
            CString menuText;
            GetMenuString(menu, static_cast<UINT>(i), menuText.GetBuffer(WXX_MAX_STRING_SIZE), WXX_MAX_STRING_SIZE, MF_BYPOSITION);
            menuText.ReleaseBuffer();

            // Add extra spaces to menuText for high DPI.
            if (GetWindowDpi(*this) >= 150)
                menuText = _T(" ") + menuText + _T(" ");

            SetButtonText(static_cast<UINT>(i + maxedOffset), menuText);
        }
    }

    // This callback used to capture keyboard input while a popup menu is active.
    inline LRESULT CALLBACK CMenuBar::StaticMsgHook(int code, WPARAM wparam, LPARAM lparam)
    {
        MSG* pMsg = reinterpret_cast<MSG*>(lparam);
        TLSData* pTLSData = GetApp()->GetTlsData();

        if (pTLSData != NULL)
        {
            CMenuBar* pMenuBar = pTLSData->pMenuBar;

            if ((pMenuBar != NULL) && (MSGF_MENU == code))
            {
                // Process menu message.
                if (pMenuBar->OnMenuInput(pMsg->message, pMsg->wParam, pMsg->lParam))
                {
                    return TRUE;
                }
            }

            return CallNextHookEx(pTLSData->msgHook, code, wparam, lparam);
        }


        return 0;
    }

    inline LRESULT CMenuBar::SysCommand(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        return OnSysCommand(msg, wparam, lparam);
    }

    // Updates the pressed state of the MDI Buttons.
    inline void CMenuBar::UpdateMDIButtons(WPARAM wparam, LPARAM lparam) const
    {
        CPoint pt;
        pt.x = GET_X_LPARAM(lparam);
        pt.y = GET_Y_LPARAM(lparam);

        if (IsMDIFrame())
        {
            if (IsMDIChildMaxed())
            {
                CClientDC MenuBarDC(*this);
                int MDIButton = -1;
                if (m_mdiRect[0].PtInRect(pt)) MDIButton = 0;
                if (m_mdiRect[1].PtInRect(pt)) MDIButton = 1;
                if (m_mdiRect[2].PtInRect(pt)) MDIButton = 2;

                if (MK_LBUTTON == wparam)  // Mouse moved with left mouse button is held down.
                {
                    // Toggle the MDI button image pressed/unpressed as required.
                    if (MDIButton >= 0)
                    {
                        DrawMDIButton(MenuBarDC, MDI_MIN,     (MDIButton == 0) ? 2U : 0U);
                        DrawMDIButton(MenuBarDC, MDI_RESTORE, (MDIButton == 1) ? 2U : 0U);
                        DrawMDIButton(MenuBarDC, MDI_CLOSE,   (MDIButton == 2) ? 2U : 0U);
                    }
                    else
                    {
                        DrawMDIButton(MenuBarDC, MDI_MIN, 0);
                        DrawMDIButton(MenuBarDC, MDI_RESTORE, 0);
                        DrawMDIButton(MenuBarDC, MDI_CLOSE, 0);
                    }
                }
                else    // Mouse moved without left mouse button held down.
                {
                    if (MDIButton >= 0)
                    {
                        DrawMDIButton(MenuBarDC, MDI_MIN,     (MDIButton == 0) ? 1U : 0U);
                        DrawMDIButton(MenuBarDC, MDI_RESTORE, (MDIButton == 1) ? 1U : 0U);
                        DrawMDIButton(MenuBarDC, MDI_CLOSE,   (MDIButton == 2) ? 1U : 0U);
                    }
                    else
                    {
                        DrawMDIButton(MenuBarDC, MDI_MIN, 0);
                        DrawMDIButton(MenuBarDC, MDI_RESTORE, 0);
                        DrawMDIButton(MenuBarDC, MDI_CLOSE, 0);
                    }
                }
            }
        }
    }

    // Provides default message processing for the menubar.
    inline LRESULT CMenuBar::WndProcDefault(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        switch (msg)
        {
        case WM_CHAR:               return 0;  // Discard these messages.
        case WM_DRAWITEM:           return OnDrawItem(msg, wparam, lparam);
        case WM_EXITMENULOOP:       return OnExitMenuLoop(msg, wparam, lparam);
        case WM_INITMENUPOPUP:      return OnInitMenuPopup(msg, wparam, lparam);
        case WM_KEYDOWN:            return OnKeyDown(msg, wparam, lparam);
        case WM_KILLFOCUS:          return OnKillFocus(msg, wparam, lparam);
        case WM_LBUTTONDBLCLK:      return OnLButtonDown(msg, wparam, lparam);
        case WM_LBUTTONDOWN:        return OnLButtonDown(msg, wparam, lparam);
        case WM_LBUTTONUP:          return OnLButtonUp(msg, wparam, lparam);
        case WM_MEASUREITEM:        return OnMeasureItem(msg, wparam, lparam);
        case WM_MOUSELEAVE:         return OnMouseLeave(msg, wparam, lparam);
        case WM_MOUSEMOVE:          return OnMouseMove(msg, wparam, lparam);
        case WM_SYSKEYDOWN:         return OnSysKeyDown(msg, wparam, lparam);
        case WM_SYSKEYUP:           return OnSysKeyUp(msg, wparam, lparam);
        case WM_WINDOWPOSCHANGED:   return OnWindowPosChanged(msg, wparam, lparam);
        case WM_WINDOWPOSCHANGING:  return OnWindowPosChanging(msg, wparam, lparam);
        case WM_UNINITMENUPOPUP:    return GetAncestor().SendMessage(msg, wparam, lparam);
        case WM_MENURBUTTONUP:      return GetAncestor().SendMessage(msg, wparam, lparam);

        // Messages defined by Win32++
        case UWM_POPUPMENU:         return OnPopupMenu();
        case UWM_GETCMENUBAR:       return reinterpret_cast<LRESULT>(this);

        } // switch (msg)

        return CToolBar::WndProcDefault(msg, wparam, lparam);
    } // LRESULT CMenuBar::WndProcDefault(...)

}

#endif  // _WIN32XX_MENUBAR_H_
