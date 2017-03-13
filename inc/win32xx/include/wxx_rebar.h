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


#ifndef _WIN32XX_REBAR_H_
#define _WIN32XX_REBAR_H_

#include "wxx_wincore.h"
#include "wxx_gdi.h"
#include "wxx_controls.h"
#include "wxx_themes.h"


namespace Win32xx
{

	////////////////////////////////////
	// Declaration of the CReBar class
	//
	class CReBar : public CWnd
	{
	public:
		CReBar();
		virtual ~CReBar();

		// Operations
		BOOL DeleteBand(const int nBand) const;
		int  HitTest(RBHITTESTINFO& rbht) const;
		HWND HitTest(POINT pt) const;
		int  IDToIndex(UINT uBandID) const;
		BOOL InsertBand(const int nBand, REBARBANDINFO& rbbi) const;
		BOOL IsBandVisible(int nBand) const;
		void MaximizeBand(UINT uBand, BOOL IsIdeal = FALSE) const;
		void MinimizeBand(UINT uBand) const;
		BOOL MoveBand(UINT uFrom, UINT uTo) const;
		void MoveBandsLeft() const;
		BOOL ResizeBand(const int nBand, const CSize& sz) const;
		BOOL ShowGripper(int nBand, BOOL Show) const;
		BOOL ShowBand(int nBand, BOOL Show) const;
		BOOL SizeToRect(CRect& rect) const;

		// Attributes
		int  GetBand(const HWND hWnd) const;
		CRect GetBandBorders(int nBand) const;
		int  GetBandCount() const;
		BOOL GetBandInfo(const int nBand, REBARBANDINFO& rbbi) const;
		CRect GetBandRect(int i) const;
		UINT GetBarHeight() const;
		BOOL GetBarInfo(REBARINFO& rbi) const;
		HWND GetMenuBar() {return m_hMenuBar;}
		UINT GetRowCount() const;
		int  GetRowHeight(int nRow) const;
		UINT GetSizeofRBBI() const;
		HWND GetToolTips() const;
		BOOL SetBandBitmap(const int nBand, HBITMAP hbmBackground) const;
		BOOL SetBandColor(const int nBand, const COLORREF clrFore, const COLORREF clrBack) const;
		BOOL SetBandInfo(const int nBand, REBARBANDINFO& rbbi) const;
		BOOL SetBarInfo(REBARINFO& rbi) const;
		void SetMenuBar(HWND hMenuBar) {m_hMenuBar = hMenuBar;}
		void SetToolTips(HWND hToolTip) const;

	protected:
		//Overridables
		virtual BOOL OnEraseBkgnd(CDC& dc);
		virtual LRESULT OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam);
		virtual LRESULT OnLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam);
		virtual LRESULT OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam);
		virtual LRESULT OnTBWinPosChanging(UINT uMsg, WPARAM wParam, LPARAM lParam);
		virtual LRESULT OnToolBarResize(UINT uMsg, WPARAM wParam, LPARAM lParam);
		virtual void PreCreate(CREATESTRUCT& cs);
		virtual void PreRegisterClass(WNDCLASS& wc);
		
		// Not intended to be overridden
		LRESULT WndProcDefault(UINT uMsg, WPARAM wParam, LPARAM lParam);

	private:
		CReBar(const CReBar&);				// Disable copy construction
		CReBar& operator = (const CReBar&); // Disable assignment operator

		BOOL m_IsDragging;
		HWND m_hMenuBar;
		LPARAM m_Orig_lParam;
	};

}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


namespace Win32xx
{

	///////////////////////////////////
	// Definitions for the CReBar class
	//
	inline CReBar::CReBar() : m_IsDragging(FALSE), m_hMenuBar(0), m_Orig_lParam(0L)
	{
	}

	inline CReBar::~CReBar()
	{
	}

	inline BOOL CReBar::DeleteBand(int nBand) const
	// Deletes a band from a rebar control.
	{
		assert(IsWindow());
		return static_cast<BOOL>(SendMessage(RB_DELETEBAND, (WPARAM)nBand, 0L));
	}

	inline int CReBar::GetBand(HWND hWnd) const
	// Returns the zero based band number for this window handle
	{
		assert(IsWindow());

		int nResult = -1;
		if (NULL == hWnd) return nResult;

		for (int nBand = 0; nBand < GetBandCount(); ++nBand)
		{
			REBARBANDINFO rbbi;
			ZeroMemory(&rbbi, GetSizeofRBBI());
			rbbi.cbSize = GetSizeofRBBI();
			rbbi.fMask = RBBIM_CHILD;
			GetBandInfo(nBand, rbbi);
			if (rbbi.hwndChild == hWnd)
				nResult = nBand;
		}

		return nResult;
	}

	inline CRect CReBar::GetBandBorders(int nBand) const
	// Retrieves the borders of a band.
	{
		assert(IsWindow());

		CRect rc;
		SendMessage(RB_GETBANDBORDERS, (WPARAM)nBand, (LPARAM)&rc);
		return rc;
	}

	inline int  CReBar::GetBandCount() const
	// Retrieves the count of bands currently in the rebar control.
	{
		assert(IsWindow());
		return static_cast<int>(SendMessage(RB_GETBANDCOUNT, 0L, 0L));
	}

	inline BOOL CReBar::GetBandInfo(int nBand, REBARBANDINFO& rbbi) const
	// Retrieves information about a specified band in a rebar control.
	{
		assert(IsWindow());
		assert(nBand >=  0);

		// REBARBANDINFO describes individual BAND characteristics
		rbbi.cbSize = GetSizeofRBBI();
		return static_cast<BOOL>(SendMessage(RB_GETBANDINFO, (WPARAM)nBand, (LPARAM)&rbbi));
	}

	inline CRect CReBar::GetBandRect(int i) const
	// Retrieves the bounding rectangle for a given band in a rebar control.
	{
		assert(IsWindow());
		CRect rc;
		SendMessage(RB_GETRECT, (WPARAM)i, (LPARAM)&rc);
		return rc;
	}

	inline UINT CReBar::GetBarHeight() const
	// Retrieves the height of the rebar control.
	{
		assert(IsWindow());
		return static_cast<UINT>(SendMessage(RB_GETBARHEIGHT, 0L, 0L));
	}

	inline BOOL CReBar::GetBarInfo(REBARINFO& rbi) const
	// Retrieves information about the rebar control and the image list it uses.
	{
		assert(IsWindow());

		// REBARINFO describes overall rebar control characteristics
		rbi.cbSize = sizeof(REBARINFO);
		return static_cast<BOOL>(SendMessage(RB_GETBARINFO, 0L, (LPARAM)&rbi));
	}

	inline UINT CReBar::GetRowCount() const
	// Retrieves the number of rows of bands in a rebar control.
	{
		assert(IsWindow());
		return static_cast<UINT>(SendMessage(RB_GETROWCOUNT, 0L, 0L));
	}

	inline int CReBar::GetRowHeight(int nRow) const
	// Retrieves the height of a specified row in a rebar control.
	{
		assert(IsWindow());
		return static_cast<int>(SendMessage(RB_GETROWHEIGHT, (WPARAM)nRow, 0L));
	}

	inline UINT CReBar::GetSizeofRBBI() const
	// The size of the REBARBANDINFO struct changes according to _WIN32_WINNT
	// sizeof(REBARBANDINFO) can report an incorrect size for older Window versions,
	// or newer Window version without XP themes enabled.
	// Use this function to get a safe size for REBARBANDINFO.
	{
		assert(IsWindow());

		UINT uSizeof = sizeof(REBARBANDINFO);

	#if defined REBARBANDINFO_V6_SIZE	// only defined for VS2008 or higher
	  #if !defined (_WIN32_WINNT) || _WIN32_WINNT >= 0x0600
		if ((GetWinVersion() < 2600) || (GetComCtlVersion() < 610)) // Vista and Vista themes?
			uSizeof = REBARBANDINFO_V6_SIZE;
	  #endif
	#endif

		return uSizeof;
	}

	inline HWND CReBar::GetToolTips() const
	// Retrieves the handle to any ToolTip control associated with the rebar control.
	{
		assert(IsWindow());
		return reinterpret_cast<HWND>(SendMessage(RB_GETTOOLTIPS, 0L, 0L));
	}

	inline int CReBar::HitTest(RBHITTESTINFO& rbht) const
	// Determines which portion of a rebar band is at a given point on the screen,
	//  if a rebar band exists at that point.
	{
		assert(IsWindow());
		return static_cast<int>(SendMessage(RB_HITTEST, 0L, (LPARAM)&rbht));
	}

	inline HWND CReBar::HitTest(POINT pt) const
	// Return the child HWND at the given point
	{
		assert(IsWindow());

		// Convert the point to client co-ordinates
		ScreenToClient(pt);

		// Get the rebar band with the point
		RBHITTESTINFO rbhti;
		ZeroMemory(&rbhti, sizeof(RBHITTESTINFO));
		rbhti.pt = pt;
		int iBand = HitTest(rbhti);

		if (iBand >= 0)
		{
			// Get the rebar band's hWnd
			REBARBANDINFO rbbi;
			ZeroMemory(&rbbi, GetSizeofRBBI());
			rbbi.cbSize = GetSizeofRBBI();
			rbbi.fMask = RBBIM_CHILD;
			GetBandInfo(iBand, rbbi);

			return rbbi.hwndChild;
		}
		else
			return NULL;
	}

	inline int CReBar::IDToIndex(UINT uBandID) const
	// Converts a band identifier to a band index in a rebar control.
	{
		assert(IsWindow());
		return static_cast<int>(SendMessage(RB_IDTOINDEX, (WPARAM)uBandID, 0L));
	}

	inline BOOL CReBar::InsertBand(int nBand, REBARBANDINFO& rbbi) const
	// Inserts a new band in a rebar control.
	{
		assert(IsWindow());

		rbbi.cbSize = GetSizeofRBBI();
		return static_cast<BOOL>(SendMessage(RB_INSERTBAND, (WPARAM)nBand, (LPARAM)&rbbi));
	}

	inline BOOL CReBar::IsBandVisible(int nBand) const
	// Returns true if the band is visible
	{
		assert(IsWindow());

		REBARBANDINFO rbbi;
		ZeroMemory(&rbbi, GetSizeofRBBI());
		rbbi.cbSize = GetSizeofRBBI();
		rbbi.fMask = RBBIM_STYLE;
		GetBandInfo(nBand, rbbi);

		return !(rbbi.fStyle & RBBS_HIDDEN);
	}

	inline BOOL CReBar::OnEraseBkgnd(CDC& dc)
	{
		// Permit the parent window to handle the drawing of the ReBar's background.
		// Return TRUE to suppress default background drawing.
		return (0 != SendMessage(::GetParent(m_hWnd), UWM_DRAWRBBKGND, (WPARAM)&dc, (LPARAM)this));
	}


	inline void CReBar::PreCreate(CREATESTRUCT& cs)
	// Sets the CREATESTRUCT parameters prior to window creation
	{
		cs.style = WS_VISIBLE | WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS |
                         CCS_NODIVIDER | RBS_VARHEIGHT | RBS_BANDBORDERS ;

	}

	inline void CReBar::PreRegisterClass(WNDCLASS& wc)
	{
		// Set the Window Class
		wc.lpszClassName =  REBARCLASSNAME;
	}

	inline void CReBar::MaximizeBand(UINT uBand, BOOL IsIdeal /*= FALSE*/) const
	// Resizes a band in a rebar control to either its ideal or largest size.
	{
		assert(IsWindow());
		SendMessage(RB_MAXIMIZEBAND, (WPARAM)uBand, (LPARAM)IsIdeal);
	}

	inline void CReBar::MinimizeBand(UINT uBand) const
	// Resizes a band in a rebar control to its smallest size.
	{
		assert(IsWindow());
		SendMessage(RB_MINIMIZEBAND, (WPARAM)uBand, 0L);
	}

	inline BOOL CReBar::MoveBand(UINT uFrom, UINT uTo) const
	// Moves a band from one index to another.
	{
		assert(IsWindow());
		return static_cast<BOOL>(SendMessage(RB_MOVEBAND, (WPARAM)uFrom, (LPARAM)uTo));
	}

	inline void CReBar::MoveBandsLeft() const
	// Repositions the bands so they are left justified
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
					SendMessage(RB_MAXIMIZEBAND, nBand, 0L);
					OldrcTop = rc.top;
				}
			}
		}
	}

	inline LRESULT CReBar::OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		m_Orig_lParam = lParam;	// Store the x,y position
		m_IsDragging = TRUE;
		
		return FinalWindowProc(uMsg, wParam, lParam);
	}

	inline LRESULT CReBar::OnLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		ReBarTheme* pTheme = reinterpret_cast<ReBarTheme*>(GetParent().SendMessage(UWM_GETRBTHEME, 0L, 0L));
		if (pTheme && pTheme->UseThemes && pTheme->LockMenuBand)
		{
			// Use move messages to limit the resizing of bands
			int y = GET_Y_LPARAM(lParam);

			if (y <= GetRowHeight(0))
			{
				// Use x,y from WM_LBUTTONDOWN for WM_LBUTTONUP position
				lParam = m_Orig_lParam;
			}
		}
		m_IsDragging = FALSE;

		return FinalWindowProc(uMsg, wParam, lParam);
	}

	inline LRESULT CReBar::OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		if (m_IsDragging)
		{
			ReBarTheme* pTheme = reinterpret_cast<ReBarTheme*>(GetParent().SendMessage(UWM_GETRBTHEME, 0L, 0L));
			if (pTheme && pTheme->UseThemes && pTheme->LockMenuBand)
			{
				// We want to lock the first row in place, but allow other bands to move!
				// Use move messages to limit the resizing of bands
				int y = GET_Y_LPARAM(lParam);

				if (y <= GetRowHeight(0))
					return 0L;	// throw this message away
			}
		}

		return FinalWindowProc(uMsg, wParam, lParam);
	}

	inline LRESULT CReBar::OnToolBarResize(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		HWND hToolBar = reinterpret_cast<HWND>(wParam);
		LPSIZE pToolBarSize = reinterpret_cast<LPSIZE>(lParam);
		ResizeBand(GetBand(hToolBar), *pToolBarSize);

		return FinalWindowProc(uMsg, wParam, lParam);
	}

	inline LRESULT CReBar::OnTBWinPosChanging(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		UNREFERENCED_PARAMETER(uMsg);
		UNREFERENCED_PARAMETER(wParam);
		UNREFERENCED_PARAMETER(lParam);

		// Adjust size for toolbars inside a rebar
		ReBarTheme* pTheme = reinterpret_cast<ReBarTheme*>(GetParent().SendMessage(UWM_GETRBTHEME, 0L, 0L));

		// A boolean expression
		return ( pTheme && pTheme->UseThemes && pTheme->ShortBands );
	}

	inline BOOL CReBar::ResizeBand(int nBand, const CSize& sz) const
	// Sets a band's size
	{
		assert(IsWindow());

		REBARBANDINFO rbbi;
		ZeroMemory(&rbbi, GetSizeofRBBI());
		rbbi.cbSize = GetSizeofRBBI();
		rbbi.fMask = RBBIM_CHILDSIZE | RBBIM_SIZE;

		GetBandInfo(nBand, rbbi);
		rbbi.cx         = sz.cx + 2;
		rbbi.cxMinChild = sz.cx + 2;
		rbbi.cyMinChild = sz.cy;
		rbbi.cyMaxChild = sz.cy;

		return SetBandInfo(nBand, rbbi );
	}

	inline BOOL CReBar::SetBandBitmap(int nBand, HBITMAP hbmBackground) const
	// Sets the band's bitmaps
	{
		assert(IsWindow());

		REBARBANDINFO rbbi;
		ZeroMemory(&rbbi, GetSizeofRBBI());
		rbbi.cbSize = GetSizeofRBBI();
		rbbi.fMask  = RBBIM_STYLE;
		GetBandInfo(nBand, rbbi);
		rbbi.fMask  |= RBBIM_BACKGROUND;
		rbbi.hbmBack = hbmBackground;

		return static_cast<BOOL>(SendMessage(RB_SETBANDINFO, (WPARAM)nBand, (LPARAM)&rbbi));
	}

	inline BOOL CReBar::SetBandColor(int nBand, COLORREF clrFore, COLORREF clrBack) const
	// Sets the band's color
	// Note:	No effect with XP themes enabled
	//			No effect if a bitmap has been set
	{
		assert(IsWindow());

		REBARBANDINFO rbbi;
		ZeroMemory(&rbbi, GetSizeofRBBI());
		rbbi.cbSize = GetSizeofRBBI();
		rbbi.fMask = RBBIM_COLORS;
		rbbi.clrFore = clrFore;
		rbbi.clrBack = clrBack;

		return static_cast<BOOL>(SendMessage(RB_SETBANDINFO, (WPARAM)nBand, (LPARAM)&rbbi));
	}

	inline BOOL CReBar::SetBandInfo(int nBand, REBARBANDINFO& rbbi) const
	// Sets the characteristics of a rebar control.
	{
		assert(IsWindow());
		assert(nBand >= 0);

		// REBARBANDINFO describes individual BAND characteristics0
		rbbi.cbSize = GetSizeofRBBI();
		return static_cast<BOOL>(SendMessage(RB_SETBANDINFO, (WPARAM)nBand, (LPARAM)&rbbi));
	}

	inline BOOL CReBar::SetBarInfo(REBARINFO& rbi) const
	// REBARINFO associates an image list with the rebar
	// A band will also need to set RBBIM_IMAGE
	{
		assert(IsWindow());

		rbi.cbSize = sizeof(REBARINFO);
		return static_cast<BOOL>(SendMessage(RB_SETBARINFO, 0L, (LPARAM)&rbi));
	}

	inline BOOL CReBar::ShowBand(int nBand, BOOL Show) const
	// Show or hide a band
	{
		assert(IsWindow());
		return static_cast<BOOL>(SendMessage(RB_SHOWBAND, (WPARAM)nBand, (LPARAM)Show));
	}

	inline BOOL CReBar::ShowGripper(int nBand, BOOL Show) const
	// Show or hide the band's gripper
	{
		assert(IsWindow());

		REBARBANDINFO rbbi;
		ZeroMemory(&rbbi, GetSizeofRBBI());
		rbbi.cbSize = GetSizeofRBBI();
		rbbi.fMask = RBBIM_STYLE;
		GetBandInfo(nBand, rbbi);
		if (Show)
		{
			rbbi.fStyle |= RBBS_GRIPPERALWAYS;
			rbbi.fStyle &= ~RBBS_NOGRIPPER;
		}
		else
		{
			rbbi.fStyle &= ~RBBS_GRIPPERALWAYS;
			rbbi.fStyle |= RBBS_NOGRIPPER;
		}

		return SetBandInfo(nBand, rbbi);
	}

	inline BOOL CReBar::SizeToRect(CRect& rect) const
	// Attempts to find the best layout of the bands for the given rectangle.
	// The rebar bands will be arranged and wrapped as necessary to fit the rectangle.
	{
		assert(IsWindow());
		return static_cast<BOOL>(SendMessage(RB_SIZETORECT, 0L, (LPARAM) &rect));
	}

	inline void CReBar::SetToolTips(HWND hToolTip) const
	// Associates a ToolTip control with the rebar control.
	{
		assert(IsWindow());
		SendMessage(RB_SETTOOLTIPS, (WPARAM)hToolTip, 0L);
	}

	inline LRESULT CReBar::WndProcDefault(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{

		switch (uMsg)
		{
		case WM_MOUSEMOVE:		return OnMouseMove(uMsg, wParam, lParam);
		case WM_LBUTTONDOWN:	return OnLButtonDown(uMsg, wParam, lParam);
		case WM_LBUTTONUP:		return OnLButtonUp(uMsg, wParam, lParam);

		// Messages defined by Win32++
		case UWM_TBRESIZE:	return OnToolBarResize(uMsg, wParam, lParam);
		case UWM_TBWINPOSCHANGING:	return OnTBWinPosChanging(uMsg, wParam, lParam);
		}

		// pass unhandled messages on for default processing
		return CWnd::WndProcDefault(uMsg, wParam, lParam);
	}

} // namespace Win32xx

#endif // #ifndef _WIN32XX_REBAR_H_
