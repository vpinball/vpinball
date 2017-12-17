// Win32++   Version 8.5
// Release Date: 1st December 2017
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
  #define GET_WHEEL_DELTA_WPARAM(wParam)  ((short)HIWORD(wParam))
#endif

namespace Win32xx
{

    ///////////////////////////////////////////////
    // The CScrollView class adds scrolling to a view window. Inherit your
    // view window from CScrollView, and use the SetScrollSizes function
    // to set up the scrolling. Override OnDraw and perform the drawing to
    // the window as usual.
    class CScrollView : public CWnd
    {
    public:
        CScrollView();
        virtual ~CScrollView();

        CBrush GetScrollBkgnd() const    { return m_brushBkgnd; }
        CPoint GetScrollPosition() const { return m_CurrentPos; }
        CSize GetTotalScrollSize() const { return m_sizeTotal; }
        BOOL IsHScrollVisible() const    { return (GetStyle() &  WS_HSCROLL) != FALSE; }
        BOOL IsVScrollVisible() const    { return (GetStyle() &  WS_VSCROLL) != FALSE; }
        void SetScrollPosition(POINT pt);
        void SetScrollSizes(CSize sizeTotal = CSize(0,0), CSize sizePage = CSize(0,0), CSize sizeLine = CSize(0,0));
        void SetScrollBkgnd(CBrush brushBkgnd) { m_brushBkgnd = brushBkgnd; }

    protected:
        virtual void    FillOutsideRect(CDC& dc, HBRUSH hBrush);
        virtual BOOL    OnEraseBkgnd(CDC& dc);
        virtual LRESULT OnHScroll(UINT uMsg, WPARAM wParam, LPARAM lParam);
        virtual LRESULT OnMouseWheel(UINT uMsg, WPARAM wParam, LPARAM lParam);
        virtual LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam);
        virtual LRESULT OnVScroll(UINT uMsg, WPARAM wParam, LPARAM lParam);
        virtual LRESULT OnWindowPosChanged(UINT uMsg, WPARAM wParam, LPARAM lParam);
        virtual LRESULT OnWindowPosChanging(UINT uMsg, WPARAM wParam, LPARAM lParam);
        virtual void    PreCreate(CREATESTRUCT& cs);

        LRESULT WndProcDefault(UINT uMsg, WPARAM wParam, LPARAM lParam);

    private:
        void UpdateBars();
        CPoint m_CurrentPos;
        CSize m_sizeTotal;
        CSize m_sizePage;
        CSize m_sizeLine;
        CBrush m_brushBkgnd;
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
        m_brushBkgnd.CreateSolidBrush(RGB(255, 255, 255));
    }


    inline CScrollView::~CScrollView()
    {
    }


    // Fills the area of the view that appears outside of the scrolling area.
    // Can be used in OnEraseBkgnd to draw the background efficiently.
    inline void CScrollView::FillOutsideRect(CDC& dc, HBRUSH hBrush)
    {
        // Get the window size in client area co-ordinates
        CRect rcWindow = GetWindowRect();
        ScreenToClient(rcWindow);

        // Fill the right side with the specified brush
        CRect rcRight(m_sizeTotal.cx, 0, rcWindow.right, rcWindow.bottom);
        dc.FillRect(rcRight, hBrush);

        // Fill the bottom side with the specified brush
        CRect rcBottom(0, m_sizeTotal.cy, m_sizeTotal.cx, rcWindow.bottom);
        dc.FillRect(rcBottom, hBrush);
    }


    // Called when the background for the window is erased.
    inline BOOL CScrollView::OnEraseBkgnd(CDC& dc)
    {
        UNREFERENCED_PARAMETER(dc);

        if (m_sizeTotal == CSize(0, 0))
            return FALSE;   // Allow background erasure when the scroll bars are disabled
        else
            return TRUE;    // Prevent background erasure to reduce flicker
    }


    // Called when an event occurs in the horizontal scroll bar.
    inline LRESULT CScrollView::OnHScroll(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        UNREFERENCED_PARAMETER(uMsg);
        UNREFERENCED_PARAMETER(lParam);

        int xNewPos;

        switch (LOWORD(wParam))
        {
        case SB_PAGEUP: // User clicked the scroll bar shaft left of the scroll box.
            xNewPos = m_CurrentPos.x - m_sizePage.cx;
            break;

        case SB_PAGEDOWN: // User clicked the scroll bar shaft right of the scroll box.
            xNewPos = m_CurrentPos.x + m_sizePage.cx;
            break;

        case SB_LINEUP: // User clicked the left arrow.
            xNewPos = m_CurrentPos.x - m_sizeLine.cx;
            break;

        case SB_LINEDOWN: // User clicked the right arrow.
            xNewPos = m_CurrentPos.x + m_sizeLine.cx;
            break;

        case SB_THUMBTRACK: // User dragging the scroll box.
            xNewPos = HIWORD(wParam);
            break;

        default:
            xNewPos = m_CurrentPos.x;
        }

        // Scroll the window.
        xNewPos = MAX(0, xNewPos);
        xNewPos = MIN(xNewPos, m_sizeTotal.cx - GetClientRect().Width());
        int xDelta = xNewPos - m_CurrentPos.x;
        m_CurrentPos.x = xNewPos;
        ScrollWindowEx(-xDelta, 0, NULL, NULL, NULL, NULL, SW_INVALIDATE);

        UpdateBars();

        return 0L;
    }

    inline LRESULT CScrollView::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam)
    // Override OnPaint so we can wrap code around OnDraw
    {

        if (m_sizeTotal != CSize(0, 0))
        {
            CPaintDC dc(*this);
            CMemDC dcMem(dc);

            // negative sizes are not allowed.
            assert(m_sizeTotal.cx > 0);
            assert(m_sizeTotal.cy > 0);

            // Create the compatible bitmap for the memory DC
            dcMem.CreateCompatibleBitmap(GetDC(), m_sizeTotal.cx, m_sizeTotal.cy);

            // Set the background color
            CRect rcTotal(CPoint(0, 0), m_sizeTotal);
            dcMem.FillRect(rcTotal, m_brushBkgnd);

            // Call the overridden OnDraw function
            OnDraw(dcMem);

            // Copy the modified memory DC to the window's DC with scrolling offsets
            dc.BitBlt(0, 0, m_sizeTotal.cx, m_sizeTotal.cy, dcMem, m_CurrentPos.x, m_CurrentPos.y, SRCCOPY);

            // Set the area outside the scrolling area
            FillOutsideRect(dc, m_brushBkgnd);

            // No more drawing required
            return 0L;
        }

        // Do default OnPaint if m_sizeTotal is zero
        return CWnd::OnPaint(uMsg, wParam, lParam);
    }

    inline LRESULT CScrollView::OnMouseWheel(UINT uMsg, WPARAM wParam, LPARAM lParam)
    // Called when the mouse wheel is rotated.
    {
        UNREFERENCED_PARAMETER(uMsg);
        UNREFERENCED_PARAMETER(lParam);

        int WheelDelta = GET_WHEEL_DELTA_WPARAM(wParam);
        int cyPos = ::MulDiv(WheelDelta, m_sizeLine.cy, WHEEL_DELTA);

        // Scroll the window.
        int yNewPos = m_CurrentPos.y - cyPos;
        yNewPos = MIN(yNewPos, m_sizeTotal.cy - GetClientRect().Height());
        yNewPos = MAX(yNewPos, 0);
        int yDelta = yNewPos - m_CurrentPos.y;
        m_CurrentPos.y = yNewPos;
        ScrollWindowEx(0, -yDelta, NULL, NULL, NULL, NULL, SW_INVALIDATE);

        UpdateBars();

        return 0L;
    }

    inline LRESULT CScrollView::OnVScroll(UINT uMsg, WPARAM wParam, LPARAM lParam)
    // Called when an event occurs in the vertical scroll bar.
    {
        UNREFERENCED_PARAMETER(uMsg);
        UNREFERENCED_PARAMETER(lParam);

        int yNewPos;

        switch (LOWORD(wParam))
        {
            case SB_PAGEUP: // User clicked the scroll bar shaft above the scroll box.
                yNewPos = m_CurrentPos.y - m_sizePage.cy;
                break;

            case SB_PAGEDOWN: // User clicked the scroll bar shaft below the scroll box.
                yNewPos = m_CurrentPos.y + m_sizePage.cy;
                break;

            case SB_LINEUP: // User clicked the top arrow.
                yNewPos = m_CurrentPos.y - m_sizeLine.cy;
                break;

            case SB_LINEDOWN: // User clicked the bottom arrow.
                yNewPos = m_CurrentPos.y + m_sizeLine.cy;
                break;

            case SB_THUMBTRACK: // User dragging the scroll box.
                yNewPos = HIWORD(wParam);
                break;

            default:
                yNewPos = m_CurrentPos.y;
        }

        // Scroll the window.
        yNewPos = MAX(0, yNewPos);
        yNewPos = MIN( yNewPos, m_sizeTotal.cy - GetClientRect().Height() );
        int yDelta = yNewPos - m_CurrentPos.y;
        m_CurrentPos.y = yNewPos;
        ScrollWindowEx(0, -yDelta, NULL, NULL, NULL, NULL, SW_INVALIDATE);

        UpdateBars();

        return 0L;
    }

    inline LRESULT CScrollView::OnWindowPosChanged(UINT uMsg, WPARAM wParam, LPARAM lParam)
    // Called after a window's size has changed.
    {
        UNREFERENCED_PARAMETER(uMsg);
        UNREFERENCED_PARAMETER(wParam);
        UNREFERENCED_PARAMETER(lParam);

        UpdateBars();
        Invalidate();

        return FinalWindowProc(uMsg, wParam, lParam);
    }

    inline LRESULT CScrollView::OnWindowPosChanging(UINT uMsg, WPARAM wParam, LPARAM lParam)
    // Called before the window's size is changed.
    {
        // We hide the scrollbars early in response to WM_WINDOWPOSCHANGING.
        // If we did this in response to WM_WINDOWPOSCHANGED we could get
        //  unexpected results due to recursion.

        // Retrieve the future size of the window
        LPWINDOWPOS pWinPos = (LPWINDOWPOS)lParam;
        assert(pWinPos);
        CRect rc(0, 0, pWinPos->cx, pWinPos->cy);

        // Possibly hide the horizontal scroll bar
        if (rc.Width() > m_sizeTotal.cx)
        {
            ShowScrollBar(SB_HORZ, FALSE);  // Can resize the window
        }

        // Possibly hide the vertical scroll bar
        if (rc.Height() > m_sizeTotal.cy)
        {
            ShowScrollBar(SB_VERT, FALSE);  // Can resize the window
        }

        return FinalWindowProc(uMsg, wParam, lParam);
    }

    inline void CScrollView::PreCreate(CREATESTRUCT& cs)
    {
        // Set the Window Class name
        cs.lpszClass = _T("ScrollView");

        cs.style = WS_CHILD | WS_HSCROLL | WS_VSCROLL;
    }

    inline void CScrollView::SetScrollPosition(POINT pt)
    // Sets the current scroll position.
    {
        assert(pt.x >= 0 && pt.x <= m_sizeTotal.cx);
        assert(pt.y >= 0 && pt.y <= m_sizeTotal.cy);

        m_CurrentPos = pt;
        UpdateBars();
    }

    inline void CScrollView::SetScrollSizes(CSize sizeTotal, CSize sizePage, CSize sizeLine)
    // Sets the various Scroll Size parameters.
    // Note that a sizeTotal of CSize(0,0) turns scrolling off.
    {
        if (IsWindow())
        {
            ShowScrollBar(SB_BOTH, FALSE);
            Invalidate();
        }

        m_sizeTotal = sizeTotal;
        m_sizePage = sizePage;
        m_sizeLine = sizeLine;

        if (m_sizePage.cx == 0)
            m_sizePage.cx = m_sizeTotal.cx / 10;
        if (m_sizePage.cy == 0)
            m_sizePage.cy = m_sizeTotal.cy / 10;
        if (m_sizeLine.cx == 0)
            m_sizeLine.cx = m_sizePage.cx / 10;
        if (m_sizeLine.cy == 0)
            m_sizeLine.cy = m_sizePage.cy / 10;

        m_CurrentPos = CPoint(0, 0);

        UpdateBars();
    }

    inline void CScrollView::UpdateBars()
    // Updates the display state of the scrollbars and the scrollbar positions.
    // Also scrolls display view as required.
    // Note: This function can be called recursively.
    {
        if (IsWindow())
        {
            if (m_sizeTotal == CSize(0, 0))
            {
                ShowScrollBar(SB_BOTH, FALSE);
                m_CurrentPos = CPoint(0, 0);
            }
            else
            {
                DWORD dwExStyle = GetExStyle();
                CRect rcTotal(0, 0, m_sizeTotal.cx, m_sizeTotal.cy);
                AdjustWindowRectEx(&rcTotal, 0, FALSE, dwExStyle);

                // CRect of view, size affected by scroll bars
                CRect rcClient = GetClientRect();
                AdjustWindowRectEx(&rcClient, 0, FALSE, dwExStyle);

                // CRect of view, unaffected by scroll bars
                CRect rcView = GetWindowRect();

                SCROLLINFO si;
                ZeroMemory(&si, sizeof(si));
                si.cbSize = sizeof(si);
                si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
                si.nMin = 0;
                bool IsBarNotRequired = ((rcView.Width() >= rcTotal.Width()) && (rcView.Height() >= rcTotal.Height()));

                if  ( (rcClient.Width() >= rcTotal.Width()) || IsBarNotRequired )
                {
                    m_CurrentPos.x = 0;
                    ShowScrollBar(SB_HORZ, FALSE);
                }
                else
                {
                    si.nMax = rcTotal.Width();
                    si.nPage = rcClient.Width();
                    si.nPos = m_CurrentPos.x;
                    SetScrollInfo(SB_HORZ, si, TRUE);
                    ShowScrollBar(SB_HORZ, TRUE);
                }

                if ( (rcClient.Height() >= rcTotal.Height()) || IsBarNotRequired )
                {
                    m_CurrentPos.y = 0;
                    ShowScrollBar(SB_VERT, FALSE);
                }
                else
                {
                    si.nMax = rcTotal.Height();
                    si.nPage = rcClient.Height();
                    si.nPos = m_CurrentPos.y;
                    SetScrollInfo(SB_VERT, si, TRUE);
                    ShowScrollBar(SB_VERT, TRUE);
                }

                int cxScroll = IsVScrollVisible() ? GetSystemMetrics(SM_CXVSCROLL) : 0;
                int cyScroll = IsHScrollVisible() ? GetSystemMetrics(SM_CYHSCROLL) : 0;

                int xNewPos = MIN(m_CurrentPos.x, rcTotal.Width() - rcView.Width() + cxScroll);
                xNewPos = MAX(xNewPos, 0);
                int xDelta = xNewPos - m_CurrentPos.x;

                int yNewPos = MIN(m_CurrentPos.y, rcTotal.Height() - rcView.Height() + cyScroll);
                yNewPos = MAX(yNewPos, 0);
                int yDelta = yNewPos - m_CurrentPos.y;

                ScrollWindowEx(-xDelta, -yDelta, NULL, NULL, NULL, NULL, SW_INVALIDATE);
                m_CurrentPos.x = xNewPos;
                m_CurrentPos.y = yNewPos;
            }
        }
    }

    inline LRESULT CScrollView::WndProcDefault(UINT uMsg, WPARAM wParam, LPARAM lParam)
    // Default message handling
    {
        switch (uMsg)
        {
        case WM_HSCROLL:            return OnHScroll(uMsg, wParam, lParam);
        case WM_MOUSEWHEEL:         return OnMouseWheel(uMsg, wParam, lParam);
        case WM_PAINT:              return OnPaint(uMsg, wParam, lParam);
        case WM_VSCROLL:            return OnVScroll(uMsg, wParam, lParam);
        case WM_WINDOWPOSCHANGED:   return OnWindowPosChanged(uMsg, wParam, lParam);
        case WM_WINDOWPOSCHANGING:  return OnWindowPosChanging(uMsg, wParam, lParam);
        }

        // Pass unhandled messages on for default processing
        return CWnd::WndProcDefault(uMsg, wParam, lParam);
    }
}

#endif // _WIN32XX_SCROLLVIEW_H_
