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


#ifndef _WIN32XX_MENUBAR_H_
#define _WIN32XX_MENUBAR_H_

#include "wxx_wincore.h"
#include "wxx_gdi.h"
#include "wxx_controls.h"
#include "wxx_toolbar.h"


#ifndef WM_UNINITMENUPOPUP
  #define WM_UNINITMENUPOPUP        0x0125
#endif

#ifndef WM_MENURBUTTONUP
  #define WM_MENURBUTTONUP      0x0122
#endif


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
        virtual void SetMenu(HMENU menu);

        void DrawAllMDIButtons(CDC& drawDC);
        CWnd* GetMDIClient() const;
        CWnd* GetActiveMDIChild() const;
        HMENU GetMenu() const {return m_topMenu;}
        virtual LRESULT OnMenuChar(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual LRESULT OnSysCommand(UINT msg, WPARAM wparam, LPARAM lparam);

    protected:
        //Overridables
        virtual void OnAttach();
        virtual LRESULT OnDrawItem(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual LRESULT OnExitMenuLoop(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual LRESULT OnInitMenuPopup(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual LRESULT OnKeyDown(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual LRESULT OnKillFocus(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual LRESULT OnLButtonDown(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual LRESULT OnLButtonUp(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual LRESULT OnMeasureItem(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual BOOL    OnMenuInput(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual LRESULT OnMouseLeave(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual LRESULT OnMouseMove(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual LRESULT OnNotifyReflect(WPARAM wparam, LPARAM lparam);
        virtual LRESULT OnSysKeyDown(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual LRESULT OnSysKeyUp(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual LRESULT OnTBNDropDown(LPNMTOOLBAR pNMTB);
        virtual LRESULT OnTBNHotItemChange(LPNMTBHOTITEM pNMHI);
        virtual LRESULT OnWindowPosChanged(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual LRESULT OnWindowPosChanging(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual void    PreCreate(CREATESTRUCT& cs);
        virtual void    PreRegisterClass(WNDCLASS& wc);

        // Not intended to be overridden
        LRESULT WndProcDefault(UINT msg, WPARAM wparam, LPARAM lparam);

    private:
        CMenuBar(const CMenuBar&);              // Disable copy construction
        CMenuBar& operator = (const CMenuBar&); // Disable assignment operator
        void DoAltKey(WORD keyCode);
        void DrawMDIButton(CDC& drawDC, int button, UINT state);
        void ExitMenu();
        void GrabFocus();
        BOOL IsMDIChildMaxed() const;
        BOOL IsMDIFrame() const;
        LRESULT OnPopupMenu();
        void ReleaseFocus();
        void SetHotItem(int nHot);
        static LRESULT CALLBACK StaticMsgHook(int code, WPARAM wparam, LPARAM lparam);

        enum MDIButtonType
        {
            MDI_MIN = 0,
            MDI_RESTORE = 1,
            MDI_CLOSE = 2
        };

        BOOL  m_isExitAfter;    // Exit after Popup menu ends
        BOOL  m_isKeyMode;      // keyboard navigation mode
        BOOL  m_isMenuActive;   // popup menu active
        BOOL  m_isSelPopup;     // a popup (cascade) menu is selected
        HMENU m_popupMenu;      // handle to the popup menu
        HMENU m_selMenu;        // handle to the cascaded popup menu
        HMENU m_topMenu;        // handle to the top level menu
        HWND  m_prevFocus;      // handle to window which had focus
        CRect m_mdiRect[3];     // array of CRect for MDI buttons
        int   m_hotItem;        // hot item
        int   m_mdiButton;      // the MDI button (MDIButtonType) pressed
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
        m_isExitAfter   = FALSE;
        m_topMenu      = NULL;
        m_hotItem       = -1;
        m_isSelPopup    = FALSE;
        m_selMenu      = NULL;
        m_isMenuActive  = FALSE;
        m_isKeyMode     = FALSE;
        m_prevFocus    = NULL;
        m_mdiButton     = 0;
        m_popupMenu    = 0;
    }

    inline CMenuBar::~CMenuBar()
    {
    }

    //Handle key pressed with Alt held down
    inline void CMenuBar::DoAltKey(WORD keyCode)
    {
        UINT item;
        if (SendMessage(TB_MAPACCELERATOR, (WPARAM)keyCode, (LPARAM)&item))
        {
            GrabFocus();
            m_isKeyMode = TRUE;
            SetHotItem(item);
            m_isMenuActive = TRUE;
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
            int cx = ::GetSystemMetrics(SM_CXSMICON);
            int cy = ::GetSystemMetrics(SM_CYSMICON);
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
                ::SetRect(&m_mdiRect[2 - i], left, top, right, bottom);
            }

            // Hide the MDI button if it won't fit
            for (int k = 0 ; k <= 2 ; ++k)
            {

                if (m_mdiRect[k].left < GetMaxSize().cx)
                {
                    ::SetRectEmpty(&m_mdiRect[k]);
                }
            }

            DrawMDIButton(drawDC, MDI_MIN, 0);
            DrawMDIButton(drawDC, MDI_RESTORE, 0);
            DrawMDIButton(drawDC, MDI_CLOSE, 0);
        }
    }

    // Draws an individual MDI button.
    inline void CMenuBar::DrawMDIButton(CDC& drawDC, int button, UINT state)
    {
        if (!IsRectEmpty(&m_mdiRect[button]))
        {
            switch (state)
            {
            case 0:
                {
                    // Draw a grey outline
                drawDC.CreatePen(PS_SOLID, 1, GetSysColor(COLOR_BTNFACE));
                drawDC.MoveTo(m_mdiRect[button].left, m_mdiRect[button].bottom);
                drawDC.LineTo(m_mdiRect[button].right, m_mdiRect[button].bottom);
                drawDC.LineTo(m_mdiRect[button].right, m_mdiRect[button].top);
                drawDC.LineTo(m_mdiRect[button].left, m_mdiRect[button].top);
                drawDC.LineTo(m_mdiRect[button].left, m_mdiRect[button].bottom);
                }
                break;
            case 1:
                {
                    // Draw outline, white at top, black on bottom
                drawDC.CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
                drawDC.MoveTo(m_mdiRect[button].left, m_mdiRect[button].bottom);
                drawDC.LineTo(m_mdiRect[button].right, m_mdiRect[button].bottom);
                drawDC.LineTo(m_mdiRect[button].right, m_mdiRect[button].top);
                drawDC.CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
                drawDC.LineTo(m_mdiRect[button].left, m_mdiRect[button].top);
                drawDC.LineTo(m_mdiRect[button].left, m_mdiRect[button].bottom);
                }

                break;
            case 2:
                {
                    // Draw outline, black on top, white on bottom
                drawDC.CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
                drawDC.MoveTo(m_mdiRect[button].left, m_mdiRect[button].bottom);
                drawDC.LineTo(m_mdiRect[button].right, m_mdiRect[button].bottom);
                drawDC.LineTo(m_mdiRect[button].right, m_mdiRect[button].top);
                drawDC.CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
                drawDC.LineTo(m_mdiRect[button].left, m_mdiRect[button].top);
                drawDC.LineTo(m_mdiRect[button].left, m_mdiRect[button].bottom);
                }
                break;
            }

            drawDC.CreatePen(PS_SOLID, 1, RGB(0, 0, 0));

            switch (button)
            {
            case MDI_MIN:
                // Manually Draw Minimise button
                drawDC.MoveTo(m_mdiRect[0].left + 4, m_mdiRect[0].bottom -4);
                drawDC.LineTo(m_mdiRect[0].right - 4, m_mdiRect[0].bottom - 4);

                drawDC.MoveTo(m_mdiRect[0].left + 4, m_mdiRect[0].bottom -5);
                drawDC.LineTo(m_mdiRect[0].right - 4, m_mdiRect[0].bottom - 5);
                break;
            case MDI_RESTORE:
                // Manually Draw Restore Button
                drawDC.MoveTo(m_mdiRect[1].left + 3, m_mdiRect[1].top + 7);
                drawDC.LineTo(m_mdiRect[1].left + 3, m_mdiRect[1].bottom -4);
                drawDC.LineTo(m_mdiRect[1].right - 6, m_mdiRect[1].bottom -4);
                drawDC.LineTo(m_mdiRect[1].right - 6, m_mdiRect[1].top + 7);
                drawDC.LineTo(m_mdiRect[1].left + 3, m_mdiRect[1].top + 7);

                drawDC.MoveTo(m_mdiRect[1].left + 3, m_mdiRect[1].top + 8);
                drawDC.LineTo(m_mdiRect[1].right - 6, m_mdiRect[1].top + 8);

                drawDC.MoveTo(m_mdiRect[1].left + 5, m_mdiRect[1].top + 7);
                drawDC.LineTo(m_mdiRect[1].left + 5, m_mdiRect[1].top + 4);
                drawDC.LineTo(m_mdiRect[1].right - 4, m_mdiRect[1].top + 4);
                drawDC.LineTo(m_mdiRect[1].right - 4, m_mdiRect[1].bottom -6);
                drawDC.LineTo(m_mdiRect[1].right - 6, m_mdiRect[1].bottom -6);

                drawDC.MoveTo(m_mdiRect[1].left + 5, m_mdiRect[1].top + 5);
                drawDC.LineTo(m_mdiRect[1].right - 4, m_mdiRect[1].top + 5);
                break;
            case MDI_CLOSE:
                // Manually Draw Close Button
                drawDC.MoveTo(m_mdiRect[2].left + 4, m_mdiRect[2].top +5);
                drawDC.LineTo(m_mdiRect[2].right - 4, m_mdiRect[2].bottom -3);

                drawDC.MoveTo(m_mdiRect[2].left + 5, m_mdiRect[2].top +5);
                drawDC.LineTo(m_mdiRect[2].right - 4, m_mdiRect[2].bottom -4);

                drawDC.MoveTo(m_mdiRect[2].left + 4, m_mdiRect[2].top +6);
                drawDC.LineTo(m_mdiRect[2].right - 5, m_mdiRect[2].bottom -3);

                drawDC.MoveTo(m_mdiRect[2].right -5, m_mdiRect[2].top +5);
                drawDC.LineTo(m_mdiRect[2].left + 3, m_mdiRect[2].bottom -3);

                drawDC.MoveTo(m_mdiRect[2].right -5, m_mdiRect[2].top +6);
                drawDC.LineTo(m_mdiRect[2].left + 4, m_mdiRect[2].bottom -3);

                drawDC.MoveTo(m_mdiRect[2].right -6, m_mdiRect[2].top +5);
                drawDC.LineTo(m_mdiRect[2].left + 3, m_mdiRect[2].bottom -4);
                break;
            }
        }
    }

    // Used when a popup menu is closed.
    inline void CMenuBar::ExitMenu()
    {
        ReleaseFocus();
        m_isKeyMode = FALSE;
        m_isMenuActive = FALSE;
        SendMessage(TB_PRESSBUTTON, m_hotItem, MAKELONG (FALSE, 0));
        SetHotItem(-1);

        CPoint pt = GetCursorPos();
        ScreenToClient(pt);

        // Update mouse mouse position for hot tracking
        SendMessage(WM_MOUSEMOVE, 0, (LPARAM)MAKELONG(pt.x, pt.y));
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

        // We use GetAncestor to send our message to the frame.
        HWND wnd = reinterpret_cast<HWND>(GetAncestor().SendMessage(UWM_GETFRAMEVIEW, 0, 0));
        CWnd* pWnd = GetCWndPtr(wnd);

        // Only MDI frames have a MDIClient
        if (pWnd && pWnd->GetClassName() == _T("MDIClient"))
            pMDIClient = pWnd;

        return pMDIClient;
    }

    // Stores the old focus and captures mouse input.
    inline void CMenuBar::GrabFocus()
    {
        if (::GetFocus() != *this)
            m_prevFocus = ::SetFocus(*this);
        ::SetCapture(*this);
        ::SetCursor(::LoadCursor(NULL, IDC_ARROW));
    }

    // Returns TRUE of the MDI child is maximized.
    inline BOOL CMenuBar::IsMDIChildMaxed() const
    {
        BOOL isMaxed = FALSE;
        if (GetMDIClient())
            GetMDIClient()->SendMessage(WM_MDIGETACTIVE, 0, (LPARAM)&isMaxed);

        return isMaxed;
    }

    // Returns TRUE if the frame is a MDI frame.
    inline BOOL CMenuBar::IsMDIFrame() const
    {
        return (GetMDIClient() != 0);
    }

    inline LRESULT CMenuBar::OnMenuChar(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        UNREFERENCED_PARAMETER(lparam);

        if (!m_isMenuActive)
            DoAltKey(LOWORD(wparam));

        return FinalWindowProc(msg, wparam, lparam);
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
    inline LRESULT CMenuBar::OnKeyDown(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        UNREFERENCED_PARAMETER(msg);
        UNREFERENCED_PARAMETER(lparam);

        switch (wparam)
        {
        case VK_ESCAPE:
            ExitMenu();
            break;

        case VK_SPACE:
            ExitMenu();
            // Bring up the system menu
            GetAncestor().PostMessage(WM_SYSCOMMAND, (WPARAM)SC_KEYMENU, (LPARAM)VK_SPACE);
            break;

        // Handle VK_DOWN,VK_UP and VK_RETURN together
        case VK_DOWN:
        case VK_UP:
        case VK_RETURN:
            // Always use PostMessage for USER_POPUPMENU (not SendMessage)
            PostMessage(UWM_POPUPMENU, 0, 0);
            break;

        case VK_LEFT:
            // Move left to next topmenu item
            (m_hotItem > 0)? SetHotItem(m_hotItem -1) : SetHotItem(GetButtonCount()-1);
            break;

        case VK_RIGHT:
            // Move right to next topmenu item
            (m_hotItem < GetButtonCount() -1)? SetHotItem(m_hotItem +1) : SetHotItem(0);
            break;

        default:
            // Handle Accelerator keys with Alt toggled down
            if (m_isKeyMode)
            {
                UINT item;
                if (SendMessage(TB_MAPACCELERATOR, wparam, (LPARAM)&item))
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
    inline LRESULT CMenuBar::OnKillFocus(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        UNREFERENCED_PARAMETER(msg);
        UNREFERENCED_PARAMETER(wparam);
        UNREFERENCED_PARAMETER(lparam);

        ExitMenu();
        return 0;
    }

    // Called when the left mouse button is pressed.
    inline LRESULT CMenuBar::OnLButtonDown(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        UNREFERENCED_PARAMETER(msg);

        // Do default processing first
        CallWindowProc(GetPrevWindowProc(), WM_LBUTTONDOWN, wparam, lparam);

        GrabFocus();
        m_mdiButton = 0;
        CPoint pt;

        pt.x = GET_X_LPARAM(lparam);
        pt.y = GET_Y_LPARAM(lparam);

        if (IsMDIFrame())
        {
            if (IsMDIChildMaxed())
            {
                CClientDC MenuBarDC(*this);
                m_mdiButton = -1;

                if (m_mdiRect[0].PtInRect(pt)) m_mdiButton = 0;
                if (m_mdiRect[1].PtInRect(pt)) m_mdiButton = 1;
                if (m_mdiRect[2].PtInRect(pt)) m_mdiButton = 2;

                if (m_mdiButton >= 0)
                {
                    DrawMDIButton(MenuBarDC, MDI_MIN,     (0 == m_mdiButton)? 2 : 0);
                    DrawMDIButton(MenuBarDC, MDI_RESTORE, (1 == m_mdiButton)? 2 : 0);
                    DrawMDIButton(MenuBarDC, MDI_CLOSE,   (2 == m_mdiButton)? 2 : 0);
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
    inline LRESULT CMenuBar::OnLButtonUp(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        UNREFERENCED_PARAMETER(msg);
        UNREFERENCED_PARAMETER(wparam);
        UNREFERENCED_PARAMETER(lparam);

        if (IsMDIFrame())
        {
            CWnd* pMDIClient = GetMDIClient();
            CWnd* pMDIChild = GetActiveMDIChild();
            assert(pMDIClient);

            if (pMDIChild && pMDIClient && IsMDIChildMaxed())
            {
                CPoint pt = GetCursorPos();
                ScreenToClient(pt);

                // Process the MDI button action when the left mouse button is up
                if (m_mdiRect[0].PtInRect(pt))
                {
                    if (MDI_MIN == m_mdiButton)
                        pMDIChild->ShowWindow(SW_MINIMIZE);
                }

                if (m_mdiRect[1].PtInRect(pt))
                {
                    if (MDI_RESTORE == m_mdiButton)
                        pMDIClient->PostMessage(WM_MDIRESTORE, (WPARAM)(pMDIChild->GetHwnd()), 0);
                }

                if (m_mdiRect[2].PtInRect(pt))
                {
                    if (MDI_CLOSE == m_mdiButton)
                        pMDIChild->PostMessage(WM_SYSCOMMAND, (WPARAM)SC_CLOSE, 0);
                }
            }
        }
        m_mdiButton = 0;
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
                    SendMessage(WM_CANCELMODE, 0, 0);
                    SendMessage(TB_PRESSBUTTON, (WPARAM)m_hotItem, (LPARAM)MAKELONG(FALSE, 0));
                    SendMessage(TB_SETHOTITEM, (WPARAM)m_hotItem, 0);
                    ExitMenu();
                    break;

                case VK_LEFT:
                    // Use default processing if inside a Sub Menu
                    if ((m_selMenu) &&(m_selMenu != m_popupMenu))
                        return FALSE;

                    SendMessage(TB_PRESSBUTTON, (WPARAM)m_hotItem, (LPARAM)MAKELONG(FALSE, 0));

                    // Move left to next topmenu item
                    (m_hotItem > 0)? --m_hotItem : m_hotItem = GetButtonCount()-1;
                    SendMessage(WM_CANCELMODE, 0, 0);

                    // Always use PostMessage for USER_POPUPMENU (not SendMessage)
                    PostMessage(UWM_POPUPMENU, 0, 0);
                    PostMessage(WM_KEYDOWN, (WPARAM)VK_DOWN, 0);
                    break;

                case VK_RIGHT:
                    // Use default processing to open Sub Menu
                    if (m_isSelPopup)
                        return FALSE;

                    SendMessage(TB_PRESSBUTTON, (WPARAM)m_hotItem, (LPARAM)MAKELONG(FALSE, 0));

                    // Move right to next topmenu item
                    (m_hotItem < GetButtonCount()-1)? ++m_hotItem : m_hotItem = 0;
                    SendMessage(WM_CANCELMODE, 0, 0);

                    // Always use PostMessage for USER_POPUPMENU (not SendMessage)
                    PostMessage(UWM_POPUPMENU, 0, 0);
                    PostMessage(WM_KEYDOWN, (WPARAM)VK_DOWN, 0);
                    break;

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
                    // Cancel popup when we hit a button a second time
                    SendMessage(WM_CANCELMODE, 0, 0);
                    return TRUE;
                }
            }
            return FALSE;

        case WM_LBUTTONDBLCLK:
            // Perform default action for DblClick on MDI Maxed icon
            if (IsMDIChildMaxed() && (0 == HitTest()))
            {
                CWnd* pMDIChild = GetActiveMDIChild();
                assert(pMDIChild);
                if (pMDIChild)
                {
                    CMenu childMenu = pMDIChild->GetSystemMenu(FALSE);

                    UINT id = childMenu.GetDefaultItem(FALSE, 0);
                    if (id)
                        pMDIChild->PostMessage(WM_SYSCOMMAND, (WPARAM)id, 0);
                }
            }

            m_isExitAfter = TRUE;
            return FALSE;

        case WM_MENUSELECT:
            {
                // store info about selected item
                m_selMenu = reinterpret_cast<HMENU>(lparam);
                m_isSelPopup = ((HIWORD(wparam) & MF_POPUP) != 0);

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
                ScreenToClient(pt);

                // Reflect messages back to the MenuBar for hot tracking
                SendMessage(WM_MOUSEMOVE, 0, (LPARAM)MAKELPARAM(pt.x, pt.y));
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

                if (MK_LBUTTON == wparam)  // mouse moved with left mouse button is held down
                {
                    // toggle the MDI button image pressed/unpressed as required
                    if (MDIButton >= 0)
                    {
                        DrawMDIButton(MenuBarDC, MDI_MIN,     ((0 == MDIButton) && (0 == m_mdiButton))? 2 : 0);
                        DrawMDIButton(MenuBarDC, MDI_RESTORE, ((1 == MDIButton) && (1 == m_mdiButton))? 2 : 0);
                        DrawMDIButton(MenuBarDC, MDI_CLOSE,   ((2 == MDIButton) && (2 == m_mdiButton))? 2 : 0);
                    }
                    else
                    {
                        DrawMDIButton(MenuBarDC, MDI_MIN,     0);
                        DrawMDIButton(MenuBarDC, MDI_RESTORE, 0);
                        DrawMDIButton(MenuBarDC, MDI_CLOSE,   0);
                    }
                }
                else    // mouse moved without left mouse button held down
                {
                    if (MDIButton >= 0)
                    {
                        DrawMDIButton(MenuBarDC, MDI_MIN,     (0 == MDIButton)? 1 : 0);
                        DrawMDIButton(MenuBarDC, MDI_RESTORE, (1 == MDIButton)? 1 : 0);
                        DrawMDIButton(MenuBarDC, MDI_CLOSE,   (2 == MDIButton)? 1 : 0);
                    }
                    else
                    {
                        DrawMDIButton(MenuBarDC, MDI_MIN,     0);
                        DrawMDIButton(MenuBarDC, MDI_RESTORE, 0);
                        DrawMDIButton(MenuBarDC, MDI_CLOSE,   0);
                    }
                }
            }
        }

        return FinalWindowProc(msg, wparam, lparam);
    }

    // Process the menubar's notifications.
    inline LRESULT CMenuBar::OnNotifyReflect(WPARAM wparam, LPARAM lparam)
    {
        UNREFERENCED_PARAMETER(wparam);

        switch (((LPNMHDR)lparam)->code)
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
            // Simulate a down arrow key press
            PostMessage(WM_KEYDOWN, (WPARAM)VK_DOWN, 0);

        m_isKeyMode = FALSE;
        m_isExitAfter = FALSE;
        m_oldMousePos = GetCursorPos();

        CWnd* pMaxMDIChild = NULL;
        if (IsMDIChildMaxed())
            pMaxMDIChild = GetActiveMDIChild();

        // Load the submenu
        int maxedOffset = IsMDIChildMaxed()? 1:0;
        m_popupMenu = ::GetSubMenu(m_topMenu, m_hotItem - maxedOffset);
        if (pMaxMDIChild && IsMDIChildMaxed() && (0 == m_hotItem) )
            m_popupMenu = pMaxMDIChild->GetSystemMenu(FALSE);

        // Retrieve the bounding rectangle for the toolbar button
        CRect rc = GetItemRect(m_hotItem);

        // convert rectangle to desktop coordinates
        ClientToScreen(rc);

        // Position popup above toolbar if it won't fit below
        TPMPARAMS tpm;
        tpm.cbSize = sizeof(tpm);
        tpm.rcExclude = rc;

        // Set the hot button
        SendMessage(TB_SETHOTITEM, (WPARAM)m_hotItem, 0);
        SendMessage(TB_PRESSBUTTON, (WPARAM)m_hotItem, (LPARAM)MAKELONG(TRUE, 0));

        m_isSelPopup = FALSE;
        m_selMenu = NULL;
        m_isMenuActive = TRUE;

        // We hook mouse input to process mouse and keyboard input during
        //  the popup menu. Messages are sent to StaticMsgHook.

        // Remove any remaining hook first
        TLSData* pTLSData = GetApp()->GetTlsData();
        pTLSData->pMenuBar = this;
        if (pTLSData->msgHook != NULL)
            ::UnhookWindowsHookEx(pTLSData->msgHook);

        // Hook messages about to be processed by the shortcut menu
        pTLSData->msgHook = ::SetWindowsHookEx(WH_MSGFILTER, (HOOKPROC)StaticMsgHook, NULL, ::GetCurrentThreadId());

        // Display the shortcut menu
        BOOL isRightToLeft = FALSE;

#if (WINVER >= 0x0500)
        isRightToLeft = ((GetAncestor().GetExStyle()) & WS_EX_LAYOUTRTL);
#endif

        int xPos = isRightToLeft? rc.right : rc.left;
        UINT id = ::TrackPopupMenuEx(m_popupMenu, TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_VERTICAL,
            xPos, rc.bottom, *this, &tpm);

        // We get here once the TrackPopupMenuEx has ended
        m_isMenuActive = FALSE;

        // Remove the message hook
        ::UnhookWindowsHookEx(pTLSData->msgHook);
        pTLSData->msgHook = NULL;

        // Process MDI Child system menu
        if (IsMDIChildMaxed())
        {
            if (pMaxMDIChild && pMaxMDIChild->GetSystemMenu(FALSE) == m_popupMenu )
            {
                if (id)
                    pMaxMDIChild->SendMessage(WM_SYSCOMMAND, id, 0);
            }
        }

        // Re-establish Focus
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
                int maxedOffset = (IsMDIChildMaxed()? 1:0);
                SetHotItem(maxedOffset);
            }
            else
                // Handle key pressed with Alt held down
                DoAltKey(static_cast<WORD>(lparam));
        }

        return FinalWindowProc(msg, wparam, lparam);
    }

    inline LRESULT CMenuBar::OnSysKeyDown(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        UNREFERENCED_PARAMETER(lparam);

        if ((VK_MENU == wparam) || (VK_F10 == wparam))
            return 0;

        return FinalWindowProc(msg, wparam, lparam);
    }

    inline LRESULT CMenuBar::OnSysKeyUp(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        UNREFERENCED_PARAMETER(lparam);

        if ((VK_MENU == wparam) || (VK_F10 == wparam))
        {
            ExitMenu();
            return 0;
        }

        return FinalWindowProc(msg, wparam, lparam);
    }

    inline LRESULT CMenuBar::OnTBNDropDown(LPNMTOOLBAR pNMTB)
    {
        UNREFERENCED_PARAMETER(pNMTB);

        // Always use PostMessage for USER_POPUPMENU (not SendMessage)
        PostMessage(UWM_POPUPMENU, 0, 0);

        return 0;
    }

    // Called when a hot item change is about to occur.
    // This is used to bring up a new popup menu when required.
    inline LRESULT CMenuBar::OnTBNHotItemChange(LPNMTBHOTITEM pNMHI)
    {
        CPoint pt = GetCursorPos();
        if (*this == ::WindowFromPoint(pt)) // MenuBar window must be on top
        {
            DWORD flag = pNMHI->dwFlags;
            if ((flag & HICF_MOUSE) && !(flag & HICF_LEAVING))
            {
                int button = HitTest();
                if ((m_isMenuActive) && (button != m_hotItem))
                {
                    SendMessage(TB_PRESSBUTTON, (WPARAM)m_hotItem, (LPARAM)MAKELONG(FALSE, 0));
                    m_hotItem = button;
                    SendMessage(WM_CANCELMODE, 0, 0);

                    //Always use PostMessage for USER_POPUPMENU (not SendMessage)
                    PostMessage(UWM_POPUPMENU, 0, 0);
                }
                m_hotItem = button;
            }

            // Handle escape from popup menu
            if ((flag & HICF_LEAVING) && m_isKeyMode)
            {
                m_hotItem = pNMHI->idOld;
                PostMessage(TB_SETHOTITEM, (WPARAM)m_hotItem, 0);
            }
        }

        return 0;
    }

    // Called when the menubar has been resized.
    inline LRESULT CMenuBar::OnWindowPosChanged(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        UNREFERENCED_PARAMETER(wparam);
        UNREFERENCED_PARAMETER(lparam);

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
        // Bypass CToolBar::WndProcDefault for this message
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

    // Releases mouse capture and returns keyboard focus.
    inline void CMenuBar::ReleaseFocus()
    {
        if (m_prevFocus)
            ::SetFocus(m_prevFocus);

        m_prevFocus = NULL;
        ::ReleaseCapture();
    }

    // Set the menubar's (toolbar's) hot item.
    inline void CMenuBar::SetHotItem(int hotItem)
    {
        m_hotItem = hotItem;
        SendMessage(TB_SETHOTITEM, (WPARAM)m_hotItem, 0);
    }

    // Builds the list of menubar (toolbar) buttons from the top level menu.
    inline void CMenuBar::SetMenu(HMENU menu)
    {
        assert(IsWindow());

        m_topMenu = menu;
        int maxedOffset = (IsMDIChildMaxed()? 1:0);

        // Remove any existing buttons
        while (SendMessage(TB_BUTTONCOUNT,  0, 0) > 0)
        {
            if (!SendMessage(TB_DELETEBUTTON, 0, 0))
                break;
        }

        // Set the Bitmap size to zero
        SendMessage(TB_SETBITMAPSIZE, 0, (LPARAM)MAKELPARAM(0, 0));

        if (IsMDIChildMaxed())
        {
            // Create an extra button for the MDI child system menu
            // Later we will custom draw the window icon over this button
            TBBUTTON tbb;
            ZeroMemory(&tbb, sizeof(tbb));
            tbb.fsState = TBSTATE_ENABLED;
            tbb.fsStyle = TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE ;
            tbb.iString = reinterpret_cast<INT_PTR>(_T(" "));
            SendMessage(TB_ADDBUTTONS, (WPARAM)1, (LPARAM)(&tbb));
            SetButtonText(0, _T("    "));
        }

        for (int i = 0 ; i < ::GetMenuItemCount(menu); ++i)
        {
            // Assign the ToolBar Button struct
            TBBUTTON tbb;
            ZeroMemory(&tbb, sizeof(tbb));
            tbb.idCommand = i  + maxedOffset;  // Each button needs a unique ID
            tbb.fsState = TBSTATE_ENABLED;
            tbb.fsStyle = TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE | TBSTYLE_DROPDOWN;
            tbb.iString = reinterpret_cast<INT_PTR>(_T(" "));
            SendMessage(TB_ADDBUTTONS, (WPARAM)1, (LPARAM)&tbb);

            // Add the menu title to the string table
            std::vector<TCHAR> menuName( MAX_MENU_STRING+1, _T('\0') );
            TCHAR* pMenuName = &menuName[0];
            GetMenuString(menu, i, pMenuName, MAX_MENU_STRING, MF_BYPOSITION);
            SetButtonText(i + maxedOffset, pMenuName);
        }
    }

    // This callback used to capture keyboard input while a popup menu is active.
    inline LRESULT CALLBACK CMenuBar::StaticMsgHook(int code, WPARAM wparam, LPARAM lparam)
    {
        assert( GetApp() );
        MSG* pMsg = reinterpret_cast<MSG*>(lparam);
        TLSData* pTLSData = GetApp()->GetTlsData();
        assert(pTLSData);
        if (!pTLSData) return 0;

        CMenuBar* pMenuBar = pTLSData->pMenuBar;

        if (pMenuBar && (MSGF_MENU == code))
        {
            // process menu message
            if (pMenuBar->OnMenuInput(pMsg->message, pMsg->wParam, pMsg->lParam))
            {
                return TRUE;
            }
        }

        return CallNextHookEx(pTLSData->msgHook, code, wparam, lparam);
    }

    // Provides default message processing for the menubar.
    inline LRESULT CMenuBar::WndProcDefault(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        switch (msg)
        {
        case WM_CHAR:               return 0;  // Discard these messages
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

        } // switch (msg)

        return CToolBar::WndProcDefault(msg, wparam, lparam);
    } // LRESULT CMenuBar::WndProcDefault(...)

}

#endif  // _WIN32XX_MENUBAR_H_
