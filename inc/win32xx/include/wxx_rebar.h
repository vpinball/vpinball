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


#ifndef _WIN32XX_REBAR_H_
#define _WIN32XX_REBAR_H_

#include "wxx_wincore.h"
#include "wxx_gdi.h"
#include "wxx_controls.h"
#include "wxx_themes.h"


namespace Win32xx
{

    /////////////////////////////////////////////////////
    // CRebar manages a rebar control. Rebar controls act
    // as containers for child windows such as toolbars.
    class CReBar : public CWnd
    {
    public:
        CReBar();
        virtual ~CReBar();

        // Operations
        BOOL DeleteBand(int band) const;
        int  HitTest(RBHITTESTINFO& hitTestInfo) const;
        HWND HitTest(POINT pt) const;
        int  IDToIndex(UINT bandID) const;
        BOOL InsertBand(int band, REBARBANDINFO& bandInfo) const;
        BOOL IsBandVisible(int band) const;
        void MaximizeBand(UINT band, BOOL isIdeal = FALSE) const;
        void MinimizeBand(UINT band) const;
        BOOL MoveBand(UINT from, UINT to) const;
        void MoveBandsLeft() const;
        BOOL ResizeBand(int band, const CSize& sz) const;
        BOOL ShowBand(int band, BOOL show) const;
        BOOL ShowGripper(int band, BOOL show) const;
        BOOL SizeToRect(RECT& rect) const;

        // Accessors and mutators
        int  GetBand(HWND wnd) const;
        CRect GetBandBorders(int band) const;
        int  GetBandCount() const;
        BOOL GetBandInfo(int band, REBARBANDINFO& bandInfo) const;
        CRect GetBandRect(int band) const;
        UINT GetBarHeight() const;
        BOOL GetBarInfo(REBARINFO& rebarInfo) const;
        HWND GetMenuBar() {return m_menuBar;}
        UINT GetRowCount() const;
        int  GetRowHeight(int row) const;
        UINT GetSizeofRBBI() const;
        HWND GetToolTips() const;
        BOOL SetBandBitmap(int band, HBITMAP background) const;
        BOOL SetBandColor(int band, COLORREF foreground, COLORREF background) const;
        BOOL SetBandInfo(int band, REBARBANDINFO& bandInfo) const;
        BOOL SetBarInfo(REBARINFO& rebarInfo) const;
        void SetMenuBar(HWND menuBar) {m_menuBar = menuBar;}
        void SetToolTips(HWND toolTip) const;

    protected:
        // Overridables
        virtual BOOL OnEraseBkgnd(CDC& dc);
        virtual LRESULT OnLButtonDown(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual LRESULT OnLButtonUp(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual LRESULT OnMouseMove(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual LRESULT OnTBWinPosChanging(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual LRESULT OnToolBarResize(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual void PreCreate(CREATESTRUCT& cs);
        virtual void PreRegisterClass(WNDCLASS& wc);

        // Not intended to be overridden
        LRESULT WndProcDefault(UINT msg, WPARAM wparam, LPARAM lparam);

    private:
        CReBar(const CReBar&);              // Disable copy construction
        CReBar& operator = (const CReBar&); // Disable assignment operator

        BOOL m_isDragging;
        HWND m_menuBar;
        LPARAM m_oldParam;
    };

}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


namespace Win32xx
{

    ///////////////////////////////////
    // Definitions for the CReBar class
    //
    inline CReBar::CReBar() : m_isDragging(FALSE), m_menuBar(0), m_oldParam(0)
    {
    }

    inline CReBar::~CReBar()
    {
    }

    // Deletes a band from a rebar control.
    // Refer to RB_DELETEBAND in the Windows API documentation for more information.
    inline BOOL CReBar::DeleteBand(int band) const
    {
        assert(IsWindow());
        return (SendMessage(RB_DELETEBAND, (WPARAM)band, 0) != 0);
    }

    // Returns the zero based band number for this window handle
    inline int CReBar::GetBand(HWND wnd) const
    {
        assert(IsWindow());

        int result = -1;
        if (0 == wnd) return result;

        for (int band = 0; band < GetBandCount(); ++band)
        {
            REBARBANDINFO rbbi;
            ZeroMemory(&rbbi, GetSizeofRBBI());
            rbbi.cbSize = GetSizeofRBBI();
            rbbi.fMask = RBBIM_CHILD;
            GetBandInfo(band, rbbi);
            if (rbbi.hwndChild == wnd)
                result = band;
        }

        return result;
    }

    // Retrieves the borders of a band.
    // Refer to RB_GETBANDBORDERS in the Windows API documentation for more information.
    inline CRect CReBar::GetBandBorders(int band) const
    {
        assert(IsWindow());

        CRect rc;
        SendMessage(RB_GETBANDBORDERS, (WPARAM)band, (LPARAM)&rc);
        return rc;
    }

    // Retrieves the count of bands currently in the rebar control.
    // Refer to RB_GETBANDCOUNT in the Windows API documentation for more information.
    inline int  CReBar::GetBandCount() const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(RB_GETBANDCOUNT, 0, 0));
    }

    // Retrieves information about a specified band in a rebar control.
    // Refer to RB_GETBANDINFO in the Windows API documentation for more information.
    inline BOOL CReBar::GetBandInfo(int band, REBARBANDINFO& bandInfo) const
    {
        assert(IsWindow());
        assert(band >=  0);

        // REBARBANDINFO describes individual BAND characteristics
        bandInfo.cbSize = GetSizeofRBBI();
        return (SendMessage(RB_GETBANDINFO, (WPARAM)band, (LPARAM)&bandInfo) != 0);
    }

    // Retrieves the bounding rectangle for a given band in a rebar control.
    // Refer to RB_GETRECT in the Windows API documentation for more information.
    inline CRect CReBar::GetBandRect(int band) const
    {
        assert(IsWindow());
        CRect rc;
        SendMessage(RB_GETRECT, (WPARAM)band, (LPARAM)&rc);
        return rc;
    }

    // Retrieves the height of the rebar control.
    // Refer to RB_GETBARHEIGHT in the Windows API documentation for more information.
    inline UINT CReBar::GetBarHeight() const
    {
        assert(IsWindow());
        return static_cast<UINT>(SendMessage(RB_GETBARHEIGHT, 0, 0));
    }

    // Retrieves information about the rebar control and the image list it uses.
    // Refer to RB_GETBARINFO in the Windows API documentation for more information.
    inline BOOL CReBar::GetBarInfo(REBARINFO& rebarInfo) const
    {
        assert(IsWindow());

        // REBARINFO describes overall rebar control characteristics
        rebarInfo.cbSize = sizeof(rebarInfo);
        return (SendMessage(RB_GETBARINFO, 0, (LPARAM)&rebarInfo) != 0);
    }

    // Retrieves the number of rows of bands in a rebar control.
    // Refer to RB_GETROWCOUNT in the Windows API documentation for more information.
    inline UINT CReBar::GetRowCount() const
    {
        assert(IsWindow());
        return static_cast<UINT>(SendMessage(RB_GETROWCOUNT, 0, 0));
    }

    // Retrieves the height of a specified row in a rebar control.
    // Refer to RB_GETROWHEIGHT in the Windows API documentation for more information.
    inline int CReBar::GetRowHeight(int row) const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(RB_GETROWHEIGHT, (WPARAM)row, 0));
    }

    // The size of the REBARBANDINFO struct changes according to _WIN32_WINNT
    // sizeof(REBARBANDINFO) can report an incorrect size for older Window versions,
    // or newer Window version without XP themes enabled.
    // Use this function to get a safe size for REBARBANDINFO.
    // Refer to REBARBANDINFO in the Windows API documentation for more information.
    inline UINT CReBar::GetSizeofRBBI() const
    {
        assert(IsWindow());

        UINT uSizeof = sizeof(REBARBANDINFO);

    #if defined REBARBANDINFO_V6_SIZE   // Only defined for VS2008 or higher.
      #if !defined (_WIN32_WINNT) || _WIN32_WINNT >= 0x0600
        if ((GetWinVersion() < 2600) || (GetComCtlVersion() < 610)) // Vista and Vista themes?
            uSizeof = REBARBANDINFO_V6_SIZE;
      #endif
    #endif

        return uSizeof;
    }

    // Retrieves the handle to any ToolTip control associated with the rebar control.
    // Refer to RB_GETTOOLTIPS in the Windows API documentation for more information.
    inline HWND CReBar::GetToolTips() const
    {
        assert(IsWindow());
        return reinterpret_cast<HWND>(SendMessage(RB_GETTOOLTIPS, 0, 0));
    }

    // Determines which portion of a rebar band is at a given point on the screen,
    //  if a rebar band exists at that point.
    // Refer to RB_HITTEST in the Windows API documentation for more information.
    inline int CReBar::HitTest(RBHITTESTINFO& hitTestInfo) const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(RB_HITTEST, 0, (LPARAM)&hitTestInfo));
    }

    // Return the child HWND at the given point.
    inline HWND CReBar::HitTest(POINT pt) const
    {
        assert(IsWindow());

        // Convert the point to client co-ordinates.
        VERIFY(ScreenToClient(pt));

        // Get the rebar band with the point.
        RBHITTESTINFO rbhti;
        ZeroMemory(&rbhti, sizeof(rbhti));
        rbhti.pt = pt;
        int iBand = HitTest(rbhti);

        if (iBand >= 0)
        {
            // Get the rebar band's wnd.
            REBARBANDINFO rbbi;
            ZeroMemory(&rbbi, GetSizeofRBBI());
            rbbi.cbSize = GetSizeofRBBI();
            rbbi.fMask = RBBIM_CHILD;
            GetBandInfo(iBand, rbbi);

            return rbbi.hwndChild;
        }
        else
            return 0;
    }

    // Converts a band identifier to a band index in a rebar control.
    // Refer to RB_IDTOINDEX in the Windows API documentation for more information.
    inline int CReBar::IDToIndex(UINT bandID) const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(RB_IDTOINDEX, (WPARAM)bandID, 0));
    }

    // Inserts a new band in a rebar control.
    // Refer to RB_INSERTBAND in the Windows API documentation for more information.
    inline BOOL CReBar::InsertBand(int nBand, REBARBANDINFO& bandInfo) const
    {
        assert(IsWindow());

        bandInfo.cbSize = GetSizeofRBBI();
        return (SendMessage(RB_INSERTBAND, (WPARAM)nBand, (LPARAM)&bandInfo) != 0);
    }

    // Returns true if the band is visible
    inline BOOL CReBar::IsBandVisible(int band) const
    {
        assert(IsWindow());

        REBARBANDINFO rbbi;
        ZeroMemory(&rbbi, GetSizeofRBBI());
        rbbi.cbSize = GetSizeofRBBI();
        rbbi.fMask = RBBIM_STYLE;
        GetBandInfo(band, rbbi);

        return !(rbbi.fStyle & RBBS_HIDDEN);
    }

    // Permit the parent window to handle the drawing of the ReBar's background.
    // Return TRUE to suppress default background drawing.
    // Refer to UWM_DRAWRBBKGND in the Windows API documentation for more information.
    inline BOOL CReBar::OnEraseBkgnd(CDC& dc)
    {
        return (0 != SendMessage(::GetParent(*this), UWM_DRAWRBBKGND, (WPARAM)&dc, (LPARAM)this));
    }

    // Sets the CREATESTRUCT parameters prior to window creation.
    inline void CReBar::PreCreate(CREATESTRUCT& cs)
    {
        cs.style = WS_VISIBLE | WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS |
                         CCS_NODIVIDER | RBS_VARHEIGHT | RBS_BANDBORDERS ;
    }

    // Set the window class
    inline void CReBar::PreRegisterClass(WNDCLASS& wc)
    {

        wc.lpszClassName =  REBARCLASSNAME;
    }

    // Resizes a band in a rebar control to either its ideal or largest size.
    // Refer to RB_MAXIMIZEBAND in the Windows API documentation for more information.
    inline void CReBar::MaximizeBand(UINT band, BOOL isIdeal /*= FALSE*/) const
    {
        assert(IsWindow());
        SendMessage(RB_MAXIMIZEBAND, (WPARAM)band, (LPARAM)isIdeal);
    }

    // Resizes a band in a rebar control to its smallest size.
    // Refer to RB_MINIMIZEBAND in the Windows API documentation for more information.
    inline void CReBar::MinimizeBand(UINT band) const
    {
        assert(IsWindow());
        SendMessage(RB_MINIMIZEBAND, (WPARAM)band, 0);
    }

    // Moves a band from one index to another.
    // Refer to RB_MOVEBAND in the Windows API documentation for more information.
    inline BOOL CReBar::MoveBand(UINT from, UINT to) const
    {
        assert(IsWindow());
        return (SendMessage(RB_MOVEBAND, (WPARAM)from, (LPARAM)to) != 0);
    }

    // Repositions the bands so they are left justified
    inline void CReBar::MoveBandsLeft() const
    {
        assert(IsWindow());

        int OldrcTop = -1;
        for (int nBand = GetBandCount() -1; nBand >= 0; --nBand)
        {
            CRect rc = GetBandRect(nBand);
            if (rc.top != OldrcTop)
            {
                // Maximize the last band on each row
                if (IsBandVisible(nBand))
                {
                    SendMessage(RB_MAXIMIZEBAND, nBand, 0);
                    OldrcTop = rc.top;
                }
            }
        }
    }

    // Called when the left mouse button is pressed.
    inline LRESULT CReBar::OnLButtonDown(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        m_oldParam = lparam; // Store the x,y position
        m_isDragging = TRUE;

        return FinalWindowProc(msg, wparam, lparam);
    }

    // Called when the left button is released.
    inline LRESULT CReBar::OnLButtonUp(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        ReBarTheme* pTheme = reinterpret_cast<ReBarTheme*>(GetParent().SendMessage(UWM_GETRBTHEME, 0, 0));
        if (pTheme && pTheme->UseThemes && pTheme->LockMenuBand)
        {
            // Use move messages to limit the resizing of bands
            int y = GET_Y_LPARAM(lparam);

            if (y <= GetRowHeight(0))
            {
                // Use x,y from WM_LBUTTONDOWN for WM_LBUTTONUP position
                lparam = m_oldParam;
            }
        }
        m_isDragging = FALSE;

        return FinalWindowProc(msg, wparam, lparam);
    }

    // Called when the mouse is moved over the window.
    inline LRESULT CReBar::OnMouseMove(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        if (m_isDragging)
        {
            ReBarTheme* pTheme = reinterpret_cast<ReBarTheme*>(GetParent().SendMessage(UWM_GETRBTHEME, 0, 0));
            if (pTheme && pTheme->UseThemes && pTheme->LockMenuBand)
            {
                // We want to lock the first row in place, but allow other bands to move!
                // Use move messages to limit the resizing of bands
                int y = GET_Y_LPARAM(lparam);

                if (y <= GetRowHeight(0))
                    return 0;  // throw this message away
            }
        }

        return FinalWindowProc(msg, wparam, lparam);
    }

    // Called when a child toolbar window is resized.
    inline LRESULT CReBar::OnToolBarResize(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        HWND hToolBar = reinterpret_cast<HWND>(wparam);
        LPSIZE pToolBarSize = reinterpret_cast<LPSIZE>(lparam);
        int nBand = GetBand(hToolBar);
        if (nBand != -1)
            ResizeBand(nBand, *pToolBarSize);

        return FinalWindowProc(msg, wparam, lparam);
    }

    // Called when a child toolbar window is resizing.
    inline LRESULT CReBar::OnTBWinPosChanging(UINT, WPARAM, LPARAM)
    {
        // Adjust size for toolbars inside a rebar
        ReBarTheme* pTheme = reinterpret_cast<ReBarTheme*>(GetParent().SendMessage(UWM_GETRBTHEME, 0, 0));

        // A boolean expression
        return ( pTheme && pTheme->UseThemes && pTheme->ShortBands );
    }

    // Sets a band's size.
    inline BOOL CReBar::ResizeBand(int band, const CSize& size) const
    {
        assert(IsWindow());

        REBARBANDINFO rbbi;
        ZeroMemory(&rbbi, GetSizeofRBBI());
        rbbi.cbSize = GetSizeofRBBI();
        rbbi.fMask = RBBIM_CHILDSIZE | RBBIM_SIZE;

        GetBandInfo(band, rbbi);
        rbbi.cx         = size.cx + 2;
        rbbi.cxMinChild = size.cx + 2;
        rbbi.cyMinChild = size.cy;
        rbbi.cyMaxChild = size.cy;

        return SetBandInfo(band, rbbi );
    }

    // Sets the band's bitmaps.
    inline BOOL CReBar::SetBandBitmap(int band, HBITMAP background) const
    {
        assert(IsWindow());

        REBARBANDINFO rbbi;
        ZeroMemory(&rbbi, GetSizeofRBBI());
        rbbi.cbSize = GetSizeofRBBI();
        rbbi.fMask  = RBBIM_STYLE;
        GetBandInfo(band, rbbi);
        rbbi.fMask  |= RBBIM_BACKGROUND;
        rbbi.hbmBack = background;

        return (SendMessage(RB_SETBANDINFO, (WPARAM)band, (LPARAM)&rbbi) != 0);
    }

    // Sets the band's color.
    // Note:    No effect with XP themes enabled
    //          No effect if a bitmap has been set
    inline BOOL CReBar::SetBandColor(int band, COLORREF foreground, COLORREF background) const
    {
        assert(IsWindow());

        REBARBANDINFO rbbi;
        ZeroMemory(&rbbi, GetSizeofRBBI());
        rbbi.cbSize = GetSizeofRBBI();
        rbbi.fMask = RBBIM_COLORS;
        rbbi.clrFore = foreground;
        rbbi.clrBack = background;

        return (SendMessage(RB_SETBANDINFO, (WPARAM)band, (LPARAM)&rbbi) != 0);
    }

    // Sets the characteristics of a rebar control.
    // Refer to RB_SETBANDINFO in the Windows API documentation for more information.
    inline BOOL CReBar::SetBandInfo(int band, REBARBANDINFO& bandInfo) const
    {
        assert(IsWindow());
        assert(band >= 0);

        bandInfo.cbSize = GetSizeofRBBI();
        return (SendMessage(RB_SETBANDINFO, (WPARAM)band, (WPARAM)&bandInfo) != 0);
    }

    // REBARINFO associates an image list with the rebar.
    // A band will also need to set RBBIM_IMAGE.
    // Refer to RB_SETBARINFO in the Windows API documentation for more information.
    inline BOOL CReBar::SetBarInfo(REBARINFO& rebarInfo) const
    {
        assert(IsWindow());

        rebarInfo.cbSize = sizeof(rebarInfo);
        return (SendMessage(RB_SETBARINFO, 0, (LPARAM)&rebarInfo) != 0);
    }

    // Show or hide a band.
    // Refer to RB_SHOWBAND in the Windows API documentation for more information.
    inline BOOL CReBar::ShowBand(int band, BOOL show) const
    {
        assert(IsWindow());
        return (SendMessage(RB_SHOWBAND, (WPARAM)band, (LPARAM)show) != 0);
    }

    // Show or hide the band's gripper.
    inline BOOL CReBar::ShowGripper(int band, BOOL show) const
    {
        assert(IsWindow());

        REBARBANDINFO rbbi;
        ZeroMemory(&rbbi, GetSizeofRBBI());
        rbbi.cbSize = GetSizeofRBBI();
        rbbi.fMask = RBBIM_STYLE;
        GetBandInfo(band, rbbi);
        if (show)
        {
            rbbi.fStyle |= RBBS_GRIPPERALWAYS;
            rbbi.fStyle &= ~RBBS_NOGRIPPER;
        }
        else
        {
            rbbi.fStyle &= ~RBBS_GRIPPERALWAYS;
            rbbi.fStyle |= RBBS_NOGRIPPER;
        }

        return SetBandInfo(band, rbbi);
    }

    // Attempts to find the best layout of the bands for the given rectangle.
    // The rebar bands will be arranged and wrapped as necessary to fit the rectangle.
    // Refer to RB_SIZETORECT in the Windows API documentation for more information.
    inline BOOL CReBar::SizeToRect(RECT& rect) const
    {
        assert(IsWindow());
        return (SendMessage(RB_SIZETORECT, 0, (LPARAM)&rect) != 0);
    }

    // Associates a ToolTip control with the rebar control.
    // Refer to RB_SETTOOLTIPS in the Windows API documentation for more information.
    inline void CReBar::SetToolTips(HWND toolTip) const
    {
        assert(IsWindow());
        SendMessage(RB_SETTOOLTIPS, (WPARAM)toolTip, 0);
    }

    // Provides default processing of this window's messages.
    inline LRESULT CReBar::WndProcDefault(UINT msg, WPARAM wparam, LPARAM lparam)
    {

        switch (msg)
        {
        case WM_MOUSEMOVE:      return OnMouseMove(msg, wparam, lparam);
        case WM_LBUTTONDOWN:    return OnLButtonDown(msg, wparam, lparam);
        case WM_LBUTTONUP:      return OnLButtonUp(msg, wparam, lparam);

        // Messages defined by Win32++
        case UWM_TBRESIZE:  return OnToolBarResize(msg, wparam, lparam);
        case UWM_TBWINPOSCHANGING:  return OnTBWinPosChanging(msg, wparam, lparam);
        }

        // pass unhandled messages on for default processing
        return CWnd::WndProcDefault(msg, wparam, lparam);
    }

} // namespace Win32xx

#endif // #ifndef _WIN32XX_REBAR_H_
