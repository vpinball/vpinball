// Win32++   Version 9.1
// Release Date: 26th September 2022
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
//
// The CScrollView class adds scrolling to a view window. Inherit your
// view window from CScrollView, and use the SetScrollSizes function
// to set up the scrolling. Override OnDraw and perform the drawing to
// the window as usual.
//
// Use SetScrollSizes to specify the scrolling sizes.
// Use SetScollSizes(CSize(0,0)) to disable scrolling.
//
// The default scrolling background is white. Use the SetScrollBkgnd
// to set a different brush color.
//
/////////////////////////////////////////////////////////



#ifndef _WIN32XX_SCROLLVIEW_H_
#define _WIN32XX_SCROLLVIEW_H_

#include "wxx_appcore0.h"

#ifndef WHEEL_DELTA
  #define WHEEL_DELTA                     120
#endif

#ifndef GET_WHEEL_DELTA_WPARAM
  #define GET_WHEEL_DELTA_WPARAM(wparam)  ((short)HIWORD(wparam))
#endif

#ifndef WM_MOUSEWHEEL
  #define WM_MOUSEWHEEL                   0x020A
#endif

namespace Win32xx
{

    //////////////////////////////////////////////////////////////////////
    // The CScrollView class adds scrolling to a view window. Inherit your
    // view window from CScrollView, and use the SetScrollSizes function
    // to set up the scrolling. Override OnDraw and perform the drawing to
    // the window as usual.
    class CScrollView : public CWnd
    {
    public:
        CScrollView();
        virtual ~CScrollView();

        CBrush GetScrollBkgnd() const    { return m_bkgndBrush; }
        CPoint GetScrollPosition() const { return m_currentPos; }
        CSize GetLineScrollSize() const  { return m_lineSize;  }
        CSize GetPageScrollSize() const  { return m_pageSize;  }
        CSize GetTotalScrollSize() const { return m_totalSize; }
        BOOL IsHScrollVisible() const    { return (GetStyle() &  WS_HSCROLL) != FALSE; }
        BOOL IsVScrollVisible() const    { return (GetStyle() &  WS_VSCROLL) != FALSE; }
        void SetScrollPosition(POINT pt);
        void SetScrollSizes(CSize totalSize = CSize(0,0), CSize pageSize = CSize(0,0), CSize lineSize = CSize(0,0));
        void SetScrollBkgnd(CBrush bkgndBrush) { m_bkgndBrush = bkgndBrush; }

    protected:
        virtual void    FillOutsideRect(CDC& dc, HBRUSH brush);
        virtual BOOL    OnEraseBkgnd(CDC& dc);
        virtual LRESULT OnHScroll(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual LRESULT OnKeyScroll(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual LRESULT OnMouseWheel(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual LRESULT OnPaint(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual LRESULT OnVScroll(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual LRESULT OnWindowPosChanged(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual LRESULT OnWindowPosChanging(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual void    PreCreate(CREATESTRUCT& cs);
        LRESULT WndProcDefault(UINT msg, WPARAM wparam, LPARAM lparam);

    private:
        CScrollView(const CScrollView&);               // Disable copy construction
        CScrollView& operator = (const CScrollView&);  // Disable assignment operator
        void UpdateBars();

        CPoint m_currentPos;
        CSize m_totalSize;
        CSize m_pageSize;
        CSize m_lineSize;
        CBrush m_bkgndBrush;
    };

}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


namespace Win32xx
{

    ////////////////////////////////////////
    // Definitions for the CScrollView class
    //

    inline CScrollView::CScrollView()
    {
        m_bkgndBrush.CreateSolidBrush(RGB(255, 255, 255));
    }

    inline CScrollView::~CScrollView()
    {
    }

    // Fills the area of the view that appears outside of the scrolling area.
    // Can be used in OnEraseBkgnd to draw the background efficiently.
    inline void CScrollView::FillOutsideRect(CDC& dc, HBRUSH brush)
    {
        // Get the window size in client area co-ordinates
        CRect windowRect = GetWindowRect();
        VERIFY(ScreenToClient(windowRect));

        // Fill the right side with the specified brush
        CRect rcRight(m_totalSize.cx, 0, windowRect.right, windowRect.bottom);
        dc.FillRect(rcRight, brush);

        // Fill the bottom side with the specified brush
        CRect rcBottom(0, m_totalSize.cy, m_totalSize.cx, windowRect.bottom);
        dc.FillRect(rcBottom, brush);
    }

    // Called when the background for the window is erased.
    inline BOOL CScrollView::OnEraseBkgnd(CDC&)
    {
        if (m_totalSize == CSize(0, 0))
            return FALSE;   // Allow background erasure when the scroll bars are disabled
        else
            return TRUE;    // Prevent background erasure to reduce flicker
    }

    // Called when an event occurs in the horizontal scroll bar.
    inline LRESULT CScrollView::OnHScroll(UINT, WPARAM wparam, LPARAM)
    {
        CPoint newPos = m_currentPos;
        SCROLLINFO si;
        ZeroMemory(&si, sizeof(si));
        si.cbSize = sizeof(si);
        si.fMask = SIF_TRACKPOS;

        switch (LOWORD(wparam))
        {
        case SB_PAGELEFT: // User clicked the scroll bar shaft left of the scroll box.
            newPos.x -= m_pageSize.cx;
            break;

        case SB_PAGERIGHT: // User clicked the scroll bar shaft right of the scroll box.
            newPos.x += m_pageSize.cx;
            break;

        case SB_LINELEFT: // User clicked the left arrow.
            newPos.x -= m_lineSize.cx;
            break;

        case SB_LINERIGHT: // User clicked the right arrow.
            newPos.x += m_lineSize.cx;
            break;

        case SB_THUMBTRACK: // User dragging the scroll box.
            // Retrieve 32 bit track position
            GetScrollInfo(SB_HORZ, si);
            newPos.x = si.nTrackPos;
            break;

        default:
            break;
        }

        int maxPosX = m_totalSize.cx - GetClientRect().Width();
        newPos.x = MAX(0, MIN(newPos.x, maxPosX));

        // Scroll the window
        int deltaX = newPos.x - m_currentPos.x;
        ScrollWindowEx(-deltaX, 0, NULL, NULL, 0, NULL, SW_INVALIDATE);
        SetScrollPosition(newPos);

        return 0;
    }

    // Called when a scroll key is pressed.
    inline LRESULT CScrollView::OnKeyScroll(UINT, WPARAM wparam, LPARAM)
    {
        CPoint newPos = m_currentPos;
        bool control = ((::GetKeyState(VK_CONTROL) & 0x8000) != 0);

        switch (wparam)
        {
        case VK_HOME:       // HOME key
            if (control)
                newPos = CPoint(0, 0);      // scroll to the top
            else
                newPos.x = 0;               // scroll to the left side
            break;

        case VK_END:        // END key
            if (control)
                newPos = m_totalSize;       // scroll to the bottom
            else
                newPos.x = m_totalSize.cx;  // scroll to the right side
            break;

        case VK_PRIOR:      //PAGEUP key
            newPos.y -= m_pageSize.cy;
            break;

        case VK_NEXT:       // PAGEDOWN key
            newPos.y += m_pageSize.cy;
            break;

        case VK_UP:         // UPARROW key
            newPos.y -= m_lineSize.cy;
            break;

        case VK_LEFT:       // LEFTARROW key
            newPos.x -= m_lineSize.cx;
            break;

        case VK_RIGHT:      // RIGHTARROW key
            newPos.x += m_lineSize.cx;
            break;

        case VK_DOWN:       // DOWNARROW key
            newPos.y +=  m_lineSize.cy;
            break;

        default:
            break;
        }

        if (newPos != m_currentPos)
        {
            int maxPosX = m_totalSize.cx - GetClientRect().Width();
            int maxPosY = m_totalSize.cy - GetClientRect().Height();
            newPos.x = MAX(0, MIN(newPos.x, maxPosX));
            newPos.y = MAX(0, MIN(newPos.y, maxPosY));

            // Scroll the window.
            int deltaX = newPos.x - m_currentPos.x;
            int deltaY = newPos.y - m_currentPos.y;
            ScrollWindowEx(-deltaX, -deltaY, NULL, NULL, 0, NULL, SW_INVALIDATE);
            SetScrollPosition(newPos);
        }

        return 0;
    }

    // Overrides OnPaint and call OnDraw with a memory DC.
    inline LRESULT CScrollView::OnPaint(UINT msg, WPARAM wparam, LPARAM lparam)
    {

        if (m_totalSize != CSize(0, 0))
        {
            CPaintDC dc(*this);
            CMemDC memDC(dc);

            // negative sizes are not allowed.
            assert(m_totalSize.cx > 0);
            assert(m_totalSize.cy > 0);

            // Create the compatible bitmap for the memory DC
            memDC.CreateCompatibleBitmap(GetDC(), m_totalSize.cx, m_totalSize.cy);

            // Set the background color
            CRect rcTotal(CPoint(0, 0), m_totalSize);
            memDC.FillRect(rcTotal, m_bkgndBrush);

            // Call the overridden OnDraw function
            OnDraw(memDC);

            // Copy the modified memory DC to the window's DC with scrolling offsets
            dc.BitBlt(0, 0, m_totalSize.cx, m_totalSize.cy, memDC, m_currentPos.x, m_currentPos.y, SRCCOPY);

            // Set the area outside the scrolling area
            FillOutsideRect(dc, m_bkgndBrush);

            // No more drawing required
            return 0;
        }

        // Do default OnPaint if m_sizeTotal is zero
        return CWnd::OnPaint(msg, wparam, lparam);
    }

    // Called when the mouse wheel is rotated.
    inline LRESULT CScrollView::OnMouseWheel(UINT, WPARAM wparam, LPARAM)
    {
        int WheelDelta = GET_WHEEL_DELTA_WPARAM(wparam);
        int cyPos = ::MulDiv(WheelDelta, m_lineSize.cy, WHEEL_DELTA);
        CPoint newPos = GetScrollPosition();
        newPos.y -= cyPos;

        int maxPosY = m_totalSize.cy - GetClientRect().Height();
        newPos.y = MAX(0, MIN(newPos.y, maxPosY));

        // Scroll the window.
        int deltaY = newPos.y - m_currentPos.y;
        ScrollWindowEx(0, -deltaY, NULL, NULL, 0, NULL, SW_INVALIDATE);
        SetScrollPosition(newPos);

        return 0;
    }

    // Called when an event occurs in the vertical scroll bar.
    inline LRESULT CScrollView::OnVScroll(UINT, WPARAM wparam, LPARAM)
    {
        CPoint newPos = m_currentPos;
        SCROLLINFO si;
        ZeroMemory(&si, sizeof(si));
        si.cbSize = sizeof(si);
        si.fMask = SIF_TRACKPOS;

        switch (LOWORD(wparam))
        {
            case SB_PAGEUP: // User clicked the scroll bar shaft above the scroll box.
                newPos.y -= m_pageSize.cy;
                break;

            case SB_PAGEDOWN: // User clicked the scroll bar shaft below the scroll box.
                newPos.y += m_pageSize.cy;
                break;

            case SB_LINEUP: // User clicked the top arrow.
                newPos.y -= m_lineSize.cy;
                break;

            case SB_LINEDOWN: // User clicked the bottom arrow.
                newPos.y += m_lineSize.cy;
                break;

            case SB_THUMBTRACK: // User dragging the scroll box.
                // Retrieve 32 bit track position
                GetScrollInfo(SB_VERT, si);
                newPos.y = si.nTrackPos;
                break;

            default:
               break;
        }

        int maxPosY = m_totalSize.cy - GetClientRect().Height();
        newPos.y = MAX(0, MIN(newPos.y, maxPosY));

        // Scroll the window.
        int deltaY = newPos.y - m_currentPos.y;
        ScrollWindowEx(0, -deltaY, NULL, NULL, 0, NULL, SW_INVALIDATE);
        SetScrollPosition(newPos);

        return 0;
    }

    // Called after a window's size has changed.
    inline LRESULT CScrollView::OnWindowPosChanged(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        UpdateBars();
        Invalidate();

        return FinalWindowProc(msg, wparam, lparam);
    }

    // Called before the window's size is changed.
    inline LRESULT CScrollView::OnWindowPosChanging(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        // We hide the scrollbars early in response to WM_WINDOWPOSCHANGING.
        // If we did this in response to WM_WINDOWPOSCHANGED we could get
        //  unexpected results due to recursion.

        // Retrieve the future size of the window
        LPWINDOWPOS pWinPos = (LPWINDOWPOS)lparam;
        assert(pWinPos);
        if (!pWinPos) return 0;

        CRect rc(0, 0, pWinPos->cx, pWinPos->cy);

        // Possibly hide the horizontal scroll bar
        if (rc.Width() > m_totalSize.cx)
        {
            ShowScrollBar(SB_HORZ, FALSE);  // Can resize the view
        }

        // Possibly hide the vertical scroll bar
        if (rc.Height() > m_totalSize.cy)
        {
            ShowScrollBar(SB_VERT, FALSE);  // Can resize the view
        }

        return FinalWindowProc(msg, wparam, lparam);
    }

    inline void CScrollView::PreCreate(CREATESTRUCT& cs)
    {
        // Set the Window Class name
        cs.lpszClass = _T("ScrollView");

        cs.style = WS_CHILD | WS_HSCROLL | WS_VSCROLL;
    }

    // Sets the current scroll position.
    inline void CScrollView::SetScrollPosition(POINT pt)
    {
        assert(pt.x >= 0 && pt.x <= m_totalSize.cx);
        assert(pt.y >= 0 && pt.y <= m_totalSize.cy);

        m_currentPos = pt;
        UpdateBars();
    }

    // Sets the various Scroll Size parameters.
    // Note that a sizeTotal of CSize(0,0) turns scrolling off.
    inline void CScrollView::SetScrollSizes(CSize totalSize, CSize pageSize, CSize lineSize)
    {
        if (IsWindow())
        {
            ShowScrollBar(SB_BOTH, FALSE);
            Invalidate();
        }

        m_totalSize = totalSize;
        m_pageSize = pageSize;
        m_lineSize = lineSize;

        if (m_pageSize.cx == 0)
            m_pageSize.cx = m_totalSize.cx / 10;
        if (m_pageSize.cy == 0)
            m_pageSize.cy = m_totalSize.cy / 10;
        if (m_lineSize.cx == 0)
            m_lineSize.cx = m_pageSize.cx / 10;
        if (m_lineSize.cy == 0)
            m_lineSize.cy = m_pageSize.cy / 10;

        m_currentPos = CPoint(0, 0);

        UpdateBars();
    }

    // Updates the display state of the scrollbars and the scrollbar positions.
    // Also scrolls display view as required by window resizing.
    // Note: This function can be called recursively.
    inline void CScrollView::UpdateBars()
    {
        // Acknowledgement:
        // A special thanks to Robert C. Tausworthe for his contribution to the
        // scrollbar logic used here.
        if (IsWindow())
        {
            if (m_totalSize == CSize(0, 0))
            {
                ShowScrollBar(SB_BOTH, FALSE);
                m_currentPos = CPoint(0, 0);
            }
            else
            {
                DWORD exStyle = GetExStyle();
                CRect totalRect(0, 0, m_totalSize.cx, m_totalSize.cy);
                AdjustWindowRectEx(&totalRect, 0, FALSE, exStyle);

                // CRect of view, size affected by scroll bars
                CRect clientRect = GetClientRect();
                AdjustWindowRectEx(&clientRect, 0, FALSE, exStyle);

                // CRect of view, unaffected by scroll bars
                CRect viewRect = GetWindowRect();

                SCROLLINFO si;
                ZeroMemory(&si, sizeof(si));
                si.cbSize = sizeof(si);
                si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
                si.nMin = 0;

                bool isHBAlwaysOn = (totalRect.Width() > viewRect.Width());         // Horizontal Bar always on
                bool isVBAlwaysOn = (totalRect.Height() > viewRect.Height());       // Vertical Bar always on

                // Horizontal Bars are always shown if the total width is greater than the window's width.
                // They are also shown if the vertical bar is shown, and the total width is greater than
                // the client width. Client width = window width - scrollbar width.
                if (isHBAlwaysOn || ((totalRect.Width() > clientRect.Width()) && isVBAlwaysOn))
                {
                    si.nMax = totalRect.Width();
                    si.nPage = static_cast<UINT>(clientRect.Width());
                    si.nPos = m_currentPos.x;
                    SetScrollInfo(SB_HORZ, si, TRUE);
                    ShowScrollBar(SB_HORZ, TRUE);
                }
                else
                {
                    m_currentPos.x = 0;
                    ShowScrollBar(SB_HORZ, FALSE);
                }

                // Vertical Bars are always shown if the total height is greater than the window's height.
                // They are also shown if the horizontal bar is shown, and the total height is greater than
                // the client height. Client height = window height - scrollbar width.
                if (isVBAlwaysOn || ((totalRect.Height() > clientRect.Height()) && isHBAlwaysOn))
                {
                    si.nMax = totalRect.Height();
                    si.nPage = static_cast<UINT>(clientRect.Height());
                    si.nPos = m_currentPos.y;
                    SetScrollInfo(SB_VERT, si, TRUE);
                    ShowScrollBar(SB_VERT, TRUE);
                }
                else
                {
                    m_currentPos.y = 0;
                    ShowScrollBar(SB_VERT, FALSE);
                }

                // Perform any additional scrolling required by window resizing
                int cxScroll = IsVScrollVisible() ? ::GetSystemMetrics(SM_CXVSCROLL) : 0;
                int cyScroll = IsHScrollVisible() ? ::GetSystemMetrics(SM_CYHSCROLL) : 0;
                int xNewPos = MIN(m_currentPos.x, totalRect.Width() - viewRect.Width() + cxScroll);
                xNewPos = MAX(xNewPos, 0);
                int xDelta = xNewPos - m_currentPos.x;
                int yNewPos = MIN(m_currentPos.y, totalRect.Height() - viewRect.Height() + cyScroll);
                yNewPos = MAX(yNewPos, 0);
                int yDelta = yNewPos - m_currentPos.y;
                ScrollWindowEx(-xDelta, -yDelta, NULL, NULL, 0, NULL, SW_INVALIDATE);

                m_currentPos.x = xNewPos;
                m_currentPos.y = yNewPos;
            }
        }
    }

    // Default message handling.
    inline LRESULT CScrollView::WndProcDefault(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        switch (msg)
        {
        case WM_HSCROLL:            return OnHScroll(msg, wparam, lparam);
        case WM_KEYDOWN:            return OnKeyScroll(msg, wparam, lparam);
        case WM_MOUSEWHEEL:         return OnMouseWheel(msg, wparam, lparam);
        case WM_PAINT:              return OnPaint(msg, wparam, lparam);
        case WM_VSCROLL:            return OnVScroll(msg, wparam, lparam);
        case WM_WINDOWPOSCHANGED:   return OnWindowPosChanged(msg, wparam, lparam);
        case WM_WINDOWPOSCHANGING:  return OnWindowPosChanging(msg, wparam, lparam);
        }

        // Pass unhandled messages on for default processing
        return CWnd::WndProcDefault(msg, wparam, lparam);
    }
}

#endif // _WIN32XX_SCROLLVIEW_H_
