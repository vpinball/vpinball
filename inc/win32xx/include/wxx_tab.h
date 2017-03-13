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


///////////////////////////////////////////////////////
// wxx_tab.h
//  Declaration of the CTab and CMDITab classes

#ifndef _WIN32XX_TAB_H_
#define _WIN32XX_TAB_H_

#include "wxx_wincore.h"
#include "wxx_dialog.h"
#include "wxx_gdi.h"
#include "wxx_regkey.h"
#include "default_resource.h"

namespace Win32xx
{

	struct TabPageInfo
	{
		CString TabText;	// The tab's text
		int iImage;			// index of this tab's image
		int idTab;			// identifier for this tab (used by TabbedMDI)
		CWnd* pView;		// pointer to the view window
		TabPageInfo() : iImage(0), idTab(0), pView(0) {}	// constructor
	};

	struct TABNMHDR
	{
		NMHDR hdr;
		UINT nPage;
	};

	class CTab : public CWnd
	{
	protected:
		// Declaration of the CSelectDialog class, a nested class of CTab
		// It creates the dialog to choose which tab to activate
		class CSelectDialog : public CDialog
		{
		public:
			CSelectDialog(LPCDLGTEMPLATE lpTemplate);
			virtual ~CSelectDialog() {}
			virtual void AddItem(LPCTSTR szString);
			virtual BOOL IsTab() const { return FALSE; }

		protected:
			virtual BOOL OnInitDialog();
			virtual void OnOK();
			virtual void OnCancel() { EndDialog(-2); }

		private:
			CSelectDialog(const CSelectDialog&);				// Disable copy construction
			CSelectDialog& operator = (const CSelectDialog&); // Disable assignment operator

			std::vector<CString> m_vItems;
			int IDC_LIST;
		};

	public:
		CTab();
		virtual ~CTab();
		virtual int  AddTabPage(CWnd* pView, LPCTSTR szTabText, HICON hIcon, UINT idTab);
		virtual int  AddTabPage(CWnd* pView, LPCTSTR szTabText, int nID_Icon, UINT idTab = 0);
		virtual int  AddTabPage(CWnd* pView, LPCTSTR szTabText);
		virtual CRect GetCloseRect() const;
		virtual CRect GetListRect() const;
		virtual CMenu& GetListMenu();
		virtual BOOL GetTabsAtTop() const;
		virtual int  GetTabIndex(CWnd* pWnd) const;
		virtual TabPageInfo GetTabPageInfo(UINT nTab) const;
		virtual int GetTabImageID(UINT nTab) const;
		virtual CString GetTabText(UINT nTab) const;
		virtual int GetTextHeight() const;
		virtual void RecalcLayout();
		virtual void RemoveTabPage(int nPage);
		virtual void SelectPage(int nPage);
		virtual void SetFixedWidth(BOOL IsEnabled);
		virtual void SetFont(HFONT hFont, BOOL Redraw = TRUE);
		virtual void SetOwnerDraw(BOOL IsEnabled);
		virtual void SetShowButtons(BOOL Show);
		virtual void SetTabIcon(int i, HICON hIcon);
		virtual void SetTabsAtTop(BOOL IsAtTop);
		virtual void SetTabText(UINT nTab, LPCTSTR szText);
		virtual void ShowListDialog();
		virtual void ShowListMenu();
		virtual void SwapTabs(UINT nTab1, UINT nTab2);

		// Attributes
		const std::vector<TabPageInfo>& GetAllTabs() const { return m_vTabPageInfo; }
		CImageList GetODImageList() const	{ return m_imlODTab; }
		CFont& GetTabFont() const		{ return m_TabFont; }
		BOOL GetShowButtons() const		{ return m_IsShowingButtons; }
		int GetTabHeight() const		{ return m_nTabHeight; }
		SIZE GetMaxTabSize() const;
		CWnd* GetActiveView() const		{ return m_pActiveView; }
		void SetTabHeight(int nTabHeight) { m_nTabHeight = nTabHeight; NotifyChanged();}

		// Wrappers for Win32 Macros
		void		AdjustRect(BOOL IsLarger, RECT *prc) const;
		BOOL		DeleteAllItems() const;
		BOOL		DeleteItem(int iItem) const;
		void		DeselectAll(UINT fExcludeFocus) const;
		int			GetCurFocus() const;
		int			GetCurSel() const;
		DWORD		GetExtendedStyle() const;
		CImageList  GetImageList() const;
		BOOL		GetItem(int iItem, LPTCITEM pitem) const;
		int			GetItemCount() const;
		BOOL		GetItemRect(int iItem, RECT& rc) const;
		int			GetRowCount() const;
		HWND		GetToolTips() const;
		BOOL		HighlightItem(INT idItem, WORD Highlight) const;
		int			HitTest(TCHITTESTINFO& info) const;
		int			InsertItem(int iItem, const LPTCITEM pItem) const;
		void		RemoveImage(int iImage) const;
		void		SetCurFocus(int iItem) const;
		int			SetCurSel(int iItem) const;
		DWORD		SetExtendedStyle(DWORD dwExStyle) const;
		CImageList  SetImageList(HIMAGELIST himlNew) const;
		BOOL		SetItem(int iItem, LPTCITEM pitem) const;
		BOOL		SetItemExtra(int cb) const;
		DWORD		SetItemSize(int cx, int cy) const;
		int			SetMinTabWidth(int cx) const;
		void		SetPadding(int cx, int cy) const;
		void		SetToolTips(HWND hToolTip) const;

	protected:
		virtual void	DrawCloseButton(CDC& dcDraw);
		virtual void	DrawListButton(CDC& dcDraw);
		virtual void	DrawTabs(CDC& dcMem);
		virtual void	DrawTabBorders(CDC& dcMem, CRect& rcTab);
		virtual void    OnAttach();
		virtual BOOL    OnEraseBkgnd(CDC&) { return TRUE;}
		virtual LRESULT OnEraseBkgnd(UINT uMsg, WPARAM wParam, LPARAM lParam);
		virtual LRESULT OnLButtonDblClk(UINT uMsg, WPARAM wParam, LPARAM lParam);
		virtual LRESULT OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam);
		virtual LRESULT OnLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam);
		virtual LRESULT OnMouseLeave(UINT uMsg, WPARAM wParam, LPARAM lParam);
		virtual LRESULT OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam);
		virtual LRESULT OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam);
		virtual LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam);
		virtual LRESULT OnNCHitTest(UINT uMsg, WPARAM wParam, LPARAM lParam);
		virtual LRESULT OnNotifyReflect(WPARAM wParam, LPARAM lParam);
		virtual LRESULT OnSetFocus(UINT uMsg, WPARAM wParam, LPARAM lParam);
		virtual LRESULT OnTCNSelChange(LPNMHDR pNMHDR);
		virtual LRESULT OnWindowPosChanged(UINT uMsg, WPARAM wParam, LPARAM lParam);
		virtual LRESULT OnWindowPosChanging(UINT uMsg, WPARAM wParam, LPARAM lParam);
		virtual void	NotifyChanged();
		virtual void	NotifyDragged();
		virtual BOOL	NotifyTabClosing(int nPage);
		virtual void	Paint();
		virtual void    PreCreate(CREATESTRUCT& cs);
		virtual void	PreRegisterClass(WNDCLASS& wc);
		virtual void    SetTabSize();

		// Not intended to be overridden
		LRESULT WndProcDefault(UINT uMsg, WPARAM wParam, LPARAM lParam);

	private:
		CTab(const CTab&);				// Disable copy construction
		CTab& operator = (const CTab&); // Disable assignment operator

		void ShowActiveView(CWnd* pView);

		std::vector<TabPageInfo> m_vTabPageInfo;
		std::vector<WndPtr> m_vTabViews;
		mutable CFont m_TabFont;
		mutable CImageList m_imlODTab;	// Image List for Owner Draw Tabs
		CMenu m_ListMenu;
		CWnd* m_pActiveView;
		CPoint m_OldMousePos;
		BOOL m_IsShowingButtons;	// Show or hide the close and list button
		BOOL m_IsTracking;
		BOOL m_IsClosePressed;
		BOOL m_IsListPressed;
		BOOL m_IsListMenuActive;
		int m_nTabHeight;
	};

	////////////////////////////////////////
	// Declaration of the CTabbedMDI class
	class CTabbedMDI : public CWnd
	{
	public:
		CTabbedMDI();
		virtual ~CTabbedMDI();
		virtual CWnd* AddMDIChild(CWnd* pView, LPCTSTR szTabText, int idMDIChild = 0);
		virtual void  CloseActiveMDI();
		virtual void  CloseAllMDIChildren();
		virtual void  CloseMDIChild(int nTab);
		virtual CWnd*  GetActiveMDIChild() const;
		virtual int	  GetActiveMDITab() const;
		virtual CWnd* GetMDIChild(int nTab) const;
		virtual int   GetMDIChildCount() const;
		virtual int   GetMDIChildID(int nTab) const;
		virtual LPCTSTR GetMDIChildTitle(int nTab) const;
		virtual CMenu& GetListMenu() const { return GetTab().GetListMenu(); }
		virtual void   ShowListDialog() { GetTab().ShowListDialog(); }
		virtual CTab& GetTab() const	{ return m_Tab; }
		virtual BOOL LoadRegistrySettings(LPCTSTR szKeyName);
		virtual void RecalcLayout();
		virtual BOOL SaveRegistrySettings(LPCTSTR szKeyName);
		virtual void SetActiveMDIChild(CWnd* pWnd) const;
		virtual void SetActiveMDITab(int nTab) const;

	protected:
		virtual HWND    Create(HWND hWndParent);
		virtual CWnd*   NewMDIChildFromID(int idMDIChild);
		virtual void 	OnAttach();
		virtual void    OnDestroy();
		virtual LRESULT OnNotify(WPARAM wParam, LPARAM lParam);
		virtual LRESULT OnSetFocus(UINT, WPARAM, LPARAM);
		virtual BOOL	OnTabClose(int nPage);
		virtual LRESULT OnWindowPosChanged(UINT uMsg, WPARAM wParam, LPARAM lParam);

		// Not intended to be overwritten
		LRESULT WndProcDefault(UINT uMsg, WPARAM wParam, LPARAM lParam);

	private:
		CTabbedMDI(const CTabbedMDI&);				// Disable copy construction
		CTabbedMDI& operator = (const CTabbedMDI&); // Disable assignment operator

		mutable CTab m_Tab;
	};

}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


namespace Win32xx
{

	/////////////////////////////////////////////////////////////
	// Definitions for the CSelectDialog class nested within CTab
	//
	inline CTab::CSelectDialog::CSelectDialog(LPCDLGTEMPLATE lpTemplate) :
					CDialog(lpTemplate), IDC_LIST(121)
	{
	}

	inline BOOL CTab::CSelectDialog::OnInitDialog()
	{
		for (UINT u = 0; u < m_vItems.size(); ++u)
		{
			SendDlgItemMessage(IDC_LIST, LB_ADDSTRING, 0, (LPARAM) m_vItems[u].c_str());
		}

		return true;
	}

	inline void CTab::CSelectDialog::AddItem(LPCTSTR szString)
	{
		m_vItems.push_back(szString);
	}

	inline void CTab::CSelectDialog::OnOK()
	{
		int iSelect = (int)SendDlgItemMessage(IDC_LIST, LB_GETCURSEL, 0, 0);
		if (iSelect != LB_ERR)
			EndDialog(iSelect);
		else
			EndDialog(-2);
	}


	//////////////////////////////////////////////////////////
	// Definitions for the CTab class
	//
	inline CTab::CTab() : m_pActiveView(NULL), m_IsShowingButtons(FALSE), m_IsTracking(FALSE), m_IsClosePressed(FALSE),
							m_IsListPressed(FALSE), m_IsListMenuActive(FALSE), m_nTabHeight(0)
	{
	}

	inline CTab::~CTab()
	{
	}

	inline int CTab::AddTabPage(CWnd* pView, LPCTSTR szTabText, HICON hIcon, UINT idTab)
	// Adds a tab. The framework assumes ownership of the CWnd pointer provided,
	// and deletes the CWnd object when the window is destroyed.
	{
		assert(pView);
		assert(lstrlen(szTabText) < MAX_MENU_STRING);

		m_vTabViews.push_back(WndPtr(pView));

		TabPageInfo tpi;
		tpi.pView = pView;
		tpi.idTab = idTab;
		tpi.TabText = szTabText;
		if (hIcon != 0)
			tpi.iImage = GetODImageList().Add(hIcon);
		else
			tpi.iImage = -1;

		int iNewPage = (int)m_vTabPageInfo.size();
		m_vTabPageInfo.push_back(tpi);

		if (IsWindow())
		{
			TCITEM tie;
			ZeroMemory(&tie, sizeof(TCITEM));
			tie.mask = TCIF_TEXT | TCIF_IMAGE;
			tie.iImage = tpi.iImage;
			tie.pszText = const_cast<LPTSTR>(tpi.TabText.c_str());
			InsertItem(iNewPage, &tie);

			SetTabSize();
			SelectPage(iNewPage);
			NotifyChanged();
		}

		return iNewPage;
	}

	inline int CTab::AddTabPage(CWnd* pView, LPCTSTR szTabText, int idIcon, UINT idTab /* = 0*/)
	// Adds a tab. The framework assumes ownership of the CWnd pointer provided,
	// and deletes the CWnd object when the window is destroyed.
	{
		HICON hIcon = (HICON)LoadImage(GetApp().GetResourceHandle(), MAKEINTRESOURCE(idIcon), IMAGE_ICON, 0, 0, LR_SHARED);
		return AddTabPage(pView, szTabText, hIcon, idTab);
	}

	inline int CTab::AddTabPage(CWnd* pView, LPCTSTR szTabText)
	// Adds a tab. The framework assumes ownership of the CWnd pointer provided,
	// and deletes the CWnd object when the window is destroyed.
	{
		return AddTabPage(pView, szTabText, (HICON)0, 0);
	}

	inline void CTab::DrawCloseButton(CDC& dcDraw)
	{
		// The close button isn't displayed on Win95
		if (GetWinVersion() == 1400)  return;

		if (!m_IsShowingButtons) return;
		if (!GetActiveView()) return;
		if (!(GetWindowLongPtr(GWL_STYLE) & TCS_FIXEDWIDTH)) return;
		if (!(GetWindowLongPtr(GWL_STYLE) & TCS_OWNERDRAWFIXED)) return;

		// Determine the close button's drawing position relative to the window
		CRect rcClose = GetCloseRect();

		CPoint pt = GetCursorPos();
		ScreenToClient(pt);
		UINT uState = rcClose.PtInRect(pt)? m_IsClosePressed? 2: 1: 0;

		// Draw the outer highlight for the close button
		if (!IsRectEmpty(&rcClose))
		{
			switch (uState)
			{
			case 0:
				{
					dcDraw.CreatePen(PS_SOLID, 1, RGB(232, 228, 220));

					dcDraw.MoveTo(rcClose.left, rcClose.bottom);
					dcDraw.LineTo(rcClose.right, rcClose.bottom);
					dcDraw.LineTo(rcClose.right, rcClose.top);
					dcDraw.LineTo(rcClose.left, rcClose.top);
					dcDraw.LineTo(rcClose.left, rcClose.bottom);
					break;
				}

			case 1:
				{
					// Draw outline, white at top, black on bottom
					dcDraw.CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
					dcDraw.MoveTo(rcClose.left, rcClose.bottom);
					dcDraw.LineTo(rcClose.right, rcClose.bottom);
					dcDraw.LineTo(rcClose.right, rcClose.top);
					dcDraw.CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
					dcDraw.LineTo(rcClose.left, rcClose.top);
					dcDraw.LineTo(rcClose.left, rcClose.bottom);
				}

				break;
			case 2:
				{
					// Draw outline, black on top, white on bottom
					dcDraw.CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
					dcDraw.MoveTo(rcClose.left, rcClose.bottom);
					dcDraw.LineTo(rcClose.right, rcClose.bottom);
					dcDraw.LineTo(rcClose.right, rcClose.top);
					dcDraw.CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
					dcDraw.LineTo(rcClose.left, rcClose.top);
					dcDraw.LineTo(rcClose.left, rcClose.bottom);
				}
				break;
			}

			// Manually draw close button
			dcDraw.CreatePen(PS_SOLID, 1, RGB(64, 64, 64));

			dcDraw.MoveTo(rcClose.left + 3, rcClose.top +3);
			dcDraw.LineTo(rcClose.right - 2, rcClose.bottom -2);

			dcDraw.MoveTo(rcClose.left + 4, rcClose.top +3);
			dcDraw.LineTo(rcClose.right - 2, rcClose.bottom -3);

			dcDraw.MoveTo(rcClose.left + 3, rcClose.top +4);
			dcDraw.LineTo(rcClose.right - 3, rcClose.bottom -2);

			dcDraw.MoveTo(rcClose.right -3, rcClose.top +3);
			dcDraw.LineTo(rcClose.left + 2, rcClose.bottom -2);

			dcDraw.MoveTo(rcClose.right -3, rcClose.top +4);
			dcDraw.LineTo(rcClose.left + 3, rcClose.bottom -2);

			dcDraw.MoveTo(rcClose.right -4, rcClose.top +3);
			dcDraw.LineTo(rcClose.left + 2, rcClose.bottom -3);
		}
	}

	inline void CTab::DrawListButton(CDC& dcDraw)
	{
		// The list button isn't displayed on Win95
		if (GetWinVersion() == 1400)  return;

		if (!m_IsShowingButtons) return;
		if (!GetActiveView()) return;
		if (!(GetWindowLongPtr(GWL_STYLE) & TCS_FIXEDWIDTH)) return;
		if (!(GetWindowLongPtr(GWL_STYLE) & TCS_OWNERDRAWFIXED)) return;

		// Determine the list button's drawing position relative to the window
		CRect rcList = GetListRect();

		CPoint pt = GetCursorPos();
		ScreenToClient(pt);
		UINT uState = rcList.PtInRect(pt)? 1: 0;
		if (m_IsListMenuActive) uState = 2;

		// Draw the outer highlight for the list button
		if (!IsRectEmpty(&rcList))
		{
			switch (uState)
			{
			case 0:
				{
					dcDraw.CreatePen(PS_SOLID, 1, RGB(232, 228, 220));

					dcDraw.MoveTo(rcList.left, rcList.bottom);
					dcDraw.LineTo(rcList.right, rcList.bottom);
					dcDraw.LineTo(rcList.right, rcList.top);
					dcDraw.LineTo(rcList.left, rcList.top);
					dcDraw.LineTo(rcList.left, rcList.bottom);
					break;
				}

			case 1:
				{
					// Draw outline, white at top, black on bottom
					dcDraw.CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
					dcDraw.MoveTo(rcList.left, rcList.bottom);
					dcDraw.LineTo(rcList.right, rcList.bottom);
					dcDraw.LineTo(rcList.right, rcList.top);
					dcDraw.CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
					dcDraw.LineTo(rcList.left, rcList.top);
					dcDraw.LineTo(rcList.left, rcList.bottom);
				}

				break;
			case 2:
				{
					// Draw outline, black on top, white on bottom
					dcDraw.CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
					dcDraw.MoveTo(rcList.left, rcList.bottom);
					dcDraw.LineTo(rcList.right, rcList.bottom);
					dcDraw.LineTo(rcList.right, rcList.top);
					dcDraw.CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
					dcDraw.LineTo(rcList.left, rcList.top);
					dcDraw.LineTo(rcList.left, rcList.bottom);
				}
				break;
			}

			// Manually draw list button
			dcDraw.CreatePen(PS_SOLID, 1, RGB(64, 64, 64));

			int MaxLength = (int)(0.65 * rcList.Width());
			int topGap = 1 + rcList.Height()/3;
			for (int i = 0; i <= MaxLength/2; ++i)
			{
				int Length = MaxLength - 2*i;
				dcDraw.MoveTo(rcList.left +1 + (rcList.Width() - Length)/2, rcList.top +topGap +i);
				dcDraw.LineTo(rcList.left +1 + (rcList.Width() - Length)/2 + Length, rcList.top +topGap +i);
			}
		}
	}

	inline void CTab::DrawTabs(CDC& dcMem)
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
					dcMem.CreateSolidBrush(RGB(248,248,248));
					dcMem.SetBkColor(RGB(248,248,248));
				}
				else
				{
					dcMem.CreateSolidBrush(RGB(200,200,200));
					dcMem.SetBkColor(RGB(200,200,200));
				}

				dcMem.CreatePen(PS_SOLID, 1, RGB(160, 160, 160));
				dcMem.RoundRect(rcItem.left, rcItem.top, rcItem.right +1, rcItem.bottom, 6, 6);

				if (rcItem.Width() >= 24)
				{
					CString str = GetTabText(i);
					int iImage = GetTabImageID(i);
					CSize szImage = m_imlODTab.GetIconSize();
					int yOffset = (rcItem.Height() - szImage.cy)/2;

					// Draw the icon
					m_imlODTab.Draw(dcMem, iImage,  CPoint(rcItem.left+5, rcItem.top+yOffset), ILD_NORMAL);

					// Draw the text
					dcMem.SelectObject(m_TabFont);

					// Calculate the size of the text
					CRect rcText = rcItem;

					int iImageSize = 20;
					int iPadding = 4;
					if (iImage >= 0)
						rcText.left += iImageSize;

					rcText.left += iPadding;
					dcMem.DrawText(str, -1, rcText, DT_LEFT|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS);
				}
			}
		}
	}

	inline void CTab::DrawTabBorders(CDC& dcMem, CRect& rcTab)
	{
		BOOL IsBottomTab = (BOOL)GetWindowLongPtr(GWL_STYLE) & TCS_BOTTOM;

		// Draw a lighter rectangle touching the tab buttons
		CRect rcItem;
		GetItemRect(0, rcItem);
		int left = rcItem.left +1;
		int right = rcTab.right;
		int top = rcTab.bottom;
		int bottom = top + 3;

		if (!IsBottomTab)
		{
			bottom = MAX(rcTab.top, m_nTabHeight +4);
			top = bottom -3;
		}

		dcMem.CreateSolidBrush(RGB(248,248,248));
		dcMem.CreatePen(PS_SOLID, 1, RGB(248,248,248));
		if (!rcItem.IsRectEmpty())
		{
			dcMem.Rectangle(left, top, right, bottom);

			// Draw a darker line below the rectangle
			dcMem.CreatePen(PS_SOLID, 1, RGB(160, 160, 160));
			if (IsBottomTab)
			{
				dcMem.MoveTo(left-1, bottom);
				dcMem.LineTo(right, bottom);
			}
			else
			{
				dcMem.MoveTo(left-1, top-1);
				dcMem.LineTo(right, top-1);
			}

			// Draw a lighter line over the darker line for the selected tab
			dcMem.CreatePen(PS_SOLID, 1, RGB(248,248,248));
			GetItemRect(GetCurSel(), rcItem);
			OffsetRect(&rcItem, 1, 1);

			if (IsBottomTab)
			{
				dcMem.MoveTo(rcItem.left, bottom);
				dcMem.LineTo(rcItem.right, bottom);
			}
			else
			{
				dcMem.MoveTo(rcItem.left, top-1);
				dcMem.LineTo(rcItem.right, top-1);
			}
		}
	}

	inline CRect CTab::GetCloseRect() const
	// Returns the dimensions of the bounding rectangle of the close button.
	{
		CRect rcClose;
		if (GetShowButtons())
		{
			rcClose= GetClientRect();
			int Gap = 2;
			int cx = GetSystemMetrics(SM_CXSMICON) -1;
			int cy = GetSystemMetrics(SM_CYSMICON) -1;
			rcClose.right -= Gap;
			rcClose.left = rcClose.right - cx;

			if (GetTabsAtTop())
				rcClose.top = Gap;
			else
				rcClose.top = MAX(Gap, rcClose.bottom - m_nTabHeight);

			rcClose.bottom = rcClose.top + cy;
		}
		return rcClose;
	}

	inline CMenu& CTab::GetListMenu()
	// Returns a reference to the list menu.
	{
		if (!IsMenu(m_ListMenu))
			m_ListMenu.CreatePopupMenu();

		// Remove any current menu items
		while (m_ListMenu.GetMenuItemCount() > 0)
		{
			m_ListMenu.RemoveMenu(0, MF_BYPOSITION);
		}

		// Add the menu items
		for(UINT u = 0; u < MIN(GetAllTabs().size(), 9); ++u)
		{
			CString MenuString;
			CString TabText = GetAllTabs()[u].TabText;
			MenuString.Format(_T("&%d %s"), u+1, TabText.c_str());
			m_ListMenu.AppendMenu(MF_STRING, IDW_FIRSTCHILD +u, MenuString);
		}
		if (GetAllTabs().size() >= 10)
			m_ListMenu.AppendMenu(MF_STRING, IDW_FIRSTCHILD +9, _T("More Windows"));

		// Add a checkmark to the menu
		int iSelected = GetCurSel();
		if (iSelected < 9)
			m_ListMenu.CheckMenuItem(iSelected, MF_BYPOSITION|MF_CHECKED);

		return m_ListMenu;
	}

	inline CRect CTab::GetListRect() const
	// Returns the dimensions of the bounding rectangle of the list button.
	{
		CRect rcList;
		if (GetShowButtons())
		{
			CRect rcClose = GetCloseRect();
			rcList = rcClose;
			rcList.OffsetRect( -(rcClose.Width() + 2), 0);
			rcList.InflateRect(-1, 0);
		}
		return rcList;
	}

	inline SIZE CTab::GetMaxTabSize() const
	// Returns the size of the largest tab
	{
		CSize Size;

		for (int i = 0; i < GetItemCount(); ++i)
		{
			CClientDC dcClient(*this);
			dcClient.SelectObject(m_TabFont);
			CString str;
			TCITEM tcItem;
			ZeroMemory(&tcItem, sizeof(TCITEM));
			tcItem.mask = TCIF_TEXT |TCIF_IMAGE;
			tcItem.cchTextMax = MAX_MENU_STRING;
			tcItem.pszText = str.GetBuffer(MAX_MENU_STRING);
			GetItem(i, &tcItem);
			str.ReleaseBuffer();
			CSize TempSize = dcClient.GetTextExtentPoint32(str, lstrlen(str));

			int iImageSize = 0;
			int iPadding = 6;
			if (tcItem.iImage >= 0)
				iImageSize = 20;
			TempSize.cx += iImageSize + iPadding;

			if (TempSize.cx > Size.cx)
				Size = TempSize;
		}

		return Size;
	}

	inline BOOL CTab::GetTabsAtTop() const
	// Returns TRUE if the control's tabs are placed at the top
	{
		DWORD dwStyle = (DWORD)GetWindowLongPtr(GWL_STYLE);
		return (!(dwStyle & TCS_BOTTOM));
	}

	inline int CTab::GetTextHeight() const
	{
		CClientDC dcClient(*this);
		dcClient.SelectObject(m_TabFont);
		CSize szText = dcClient.GetTextExtentPoint32(_T("Text"), lstrlen(_T("Text")));
		return szText.cy;
	}

	inline int CTab::GetTabIndex(CWnd* pWnd) const
	// Returns the index of the tab given its view window.
	{
		assert(pWnd);

		for (int i = 0; i < (int)m_vTabPageInfo.size(); ++i)
		{
			if (m_vTabPageInfo[i].pView == pWnd)
				return i;
		}

		return -1;
	}

	inline TabPageInfo CTab::GetTabPageInfo(UINT nTab) const
	// Returns the tab page info struct for the specified tab.
	{
		assert (nTab < m_vTabPageInfo.size());

		return m_vTabPageInfo[nTab];
	}

	inline int CTab::GetTabImageID(UINT nTab) const
	// Returns the image ID for the specified tab.
	{
		assert (nTab < m_vTabPageInfo.size());
		return m_vTabPageInfo[nTab].iImage;
	}

	inline CString CTab::GetTabText(UINT nTab) const
	// Returns the text for the specified tab.
	{
		assert (nTab < m_vTabPageInfo.size());
		return m_vTabPageInfo[nTab].TabText;
	}

	inline void CTab::NotifyChanged()
	{
		NMHDR nmhdr;
		ZeroMemory(&nmhdr, sizeof(NMHDR));
		nmhdr.hwndFrom = *this;
		nmhdr.code = UMN_TABCHANGED;

		if (GetParent().IsWindow())
			GetParent().SendMessage(WM_NOTIFY, 0L, (LPARAM)&nmhdr);
	}

	inline void CTab::NotifyDragged()
	{
		NMHDR nmhdr;
		ZeroMemory(&nmhdr, sizeof(NMHDR));
		nmhdr.hwndFrom = *this;
		nmhdr.code = UWN_TABDRAGGED;
		GetParent().SendMessage(WM_NOTIFY, 0L, (LPARAM)&nmhdr);
	}

	inline BOOL CTab::NotifyTabClosing(int nPage)
	{
		int idCtrl = GetDlgCtrlID();
		TABNMHDR TabNMHDR;
		TabNMHDR.hdr.code = UWN_TABCLOSE;
		TabNMHDR.hdr.hwndFrom = *this;
		TabNMHDR.hdr.idFrom = idCtrl;
		TabNMHDR.nPage = nPage;

		// The default return value is zero
		return (BOOL)GetParent().SendMessage(WM_NOTIFY, idCtrl, (LPARAM)&TabNMHDR);
	}

	inline void CTab::OnAttach()
	// Called when this object is attached to a tab control
	{
		// Create and assign the image list
		m_imlODTab.DeleteImageList();
		m_imlODTab.Create(16, 16, ILC_MASK|ILC_COLOR32, 0, 0);

		// Set the tab control's font
		m_TabFont.DeleteObject();
		NONCLIENTMETRICS info;
		ZeroMemory(&info, sizeof(NONCLIENTMETRICS));
		info.cbSize = GetSizeofNonClientMetrics();
		SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(info), &info, 0);
		m_TabFont.CreateFontIndirect(info.lfStatusFont);

		SetFont(m_TabFont, TRUE);

		// Assign ImageList unless we are owner drawn
		if (!(GetWindowLongPtr(GWL_STYLE) & TCS_OWNERDRAWFIXED))
			SetImageList(m_imlODTab);

		for (int i = 0; i < (int)m_vTabPageInfo.size(); ++i)
		{
			// Add tabs for each view.
			TCITEM tie;
			ZeroMemory(&tie, sizeof(TCITEM));
			tie.mask = TCIF_TEXT | TCIF_IMAGE;
			tie.iImage = m_vTabPageInfo[i].iImage;
			tie.pszText = const_cast<LPTSTR>(m_vTabPageInfo[i].TabText.c_str());
			InsertItem(i, &tie);
		}

		int HeightGap = 5;
		SetTabHeight( MAX(20, GetTextHeight() + HeightGap) );
		SelectPage(0);
	}

	inline LRESULT CTab::OnEraseBkgnd(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		if (GetWindowLongPtr(GWL_STYLE) & TCS_OWNERDRAWFIXED)
			return 0;

		return FinalWindowProc(uMsg, wParam, lParam);
	}

	inline LRESULT CTab::OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		CPoint pt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

		if (GetCloseRect().PtInRect(pt))
		{
			m_IsClosePressed = TRUE;
			SetCapture();
			CClientDC dc(*this);
			DrawCloseButton(dc);
		}
		else
			m_IsClosePressed = FALSE;

		if (GetListRect().PtInRect(pt))
		{
			ShowListMenu();
		}

		return FinalWindowProc(uMsg, wParam, lParam);
	}

	inline LRESULT CTab::OnLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		ReleaseCapture();
		CPoint pt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		if (m_IsClosePressed && GetCloseRect().PtInRect(pt))
		{
			int nPage = GetCurSel();

			// Send a notification to parent asking if its OK to close the tab.
			if (!NotifyTabClosing(nPage))
			{
				RemoveTabPage(nPage);
				if (nPage > 0)
					SelectPage(nPage -1);

				if (GetActiveView())
					GetActiveView()->RedrawWindow();
			}
		}

		m_IsClosePressed = FALSE;

		return FinalWindowProc(uMsg, wParam, lParam);
	}

	inline LRESULT CTab::OnMouseLeave(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		CClientDC dc(*this);
		DrawCloseButton(dc);
		DrawListButton(dc);

		m_IsTracking = FALSE;

		return FinalWindowProc(uMsg, wParam, lParam);
	}

	inline LRESULT CTab::OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		CPoint pt;
		pt.x = GET_X_LPARAM(lParam);
		pt.y = GET_Y_LPARAM(lParam);



		if (!m_IsListMenuActive && m_IsListPressed)
		{
			m_IsListPressed = FALSE;
		}

		if (!m_IsTracking)
		{
			TRACKMOUSEEVENT TrackMouseEventStruct;
			ZeroMemory(&TrackMouseEventStruct, sizeof(TRACKMOUSEEVENT));
			TrackMouseEventStruct.cbSize = sizeof(TrackMouseEventStruct);
			TrackMouseEventStruct.dwFlags = TME_LEAVE;
			TrackMouseEventStruct.hwndTrack = *this;
			_TrackMouseEvent(&TrackMouseEventStruct);
			m_IsTracking = TRUE;
		}

		// Skip if mouse hasn't moved
		if ((pt.x != m_OldMousePos.x) || (pt.y != m_OldMousePos.y))
		{
			if (IsLeftButtonDown())
				NotifyDragged();
		}

		m_OldMousePos.x = pt.x;
		m_OldMousePos.y = pt.y;

		CClientDC dc(*this);
		DrawCloseButton(dc);
		DrawListButton(dc);

		return FinalWindowProc(uMsg, wParam, lParam);
	}

	inline LRESULT CTab::OnNCHitTest(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		// Ensure we have an arrow cursor when the tab has no view window
		if (GetAllTabs().size() == 0)
			SetCursor(LoadCursor(NULL, IDC_ARROW));

		// Cause WM_LBUTTONUP and WM_LBUTTONDOWN messages to be sent for buttons
		CPoint pt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		ScreenToClient(pt);
		if (GetCloseRect().PtInRect(pt)) return HTCLIENT;
		if (GetListRect().PtInRect(pt))  return HTCLIENT;

		return CWnd::WndProcDefault(uMsg, wParam, lParam);
	}

	inline LRESULT CTab::OnNotifyReflect(WPARAM wParam, LPARAM lParam)
	{
		UNREFERENCED_PARAMETER(wParam);

		LPNMHDR pNMHDR = (LPNMHDR)lParam;
		switch (pNMHDR->code)
		{
		case TCN_SELCHANGE:	return OnTCNSelChange(pNMHDR);
		}

		// The framework will call SetWindowLongPtr(DWLP_MSGRESULT, lr) for non-zero returns
		return 0L;
	}

	inline LRESULT CTab::OnTCNSelChange(LPNMHDR pNMHDR)
	{
		UNREFERENCED_PARAMETER(pNMHDR);

		// Display the newly selected tab page
		int nPage = GetCurSel();
		ShowActiveView(m_vTabPageInfo[nPage].pView);

		return 0L;
	}

	inline LRESULT CTab::OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		m_IsClosePressed = FALSE;
		return FinalWindowProc(uMsg, wParam, lParam);
	}

	inline LRESULT CTab::OnLButtonDblClk(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		return OnLButtonDown(uMsg, wParam, lParam);
	}

	inline LRESULT CTab::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		if (GetWindowLongPtr(GWL_STYLE) & TCS_OWNERDRAWFIXED)
		{
			// Remove all pending paint requests
			PAINTSTRUCT ps;
			::BeginPaint(*this, &ps);
			::EndPaint(*this, &ps);

			// Now call our local Paint
			Paint();
			return 0L;
		}

		return FinalWindowProc(uMsg, wParam, lParam);
	}

	inline LRESULT CTab::OnSetFocus(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		// Prevent the tab control from grabbing focus when we have a view
		if (GetActiveView() && !m_IsClosePressed)
		{
			GetActiveView()->SetFocus();
		}

		return FinalWindowProc(uMsg, wParam, lParam);
	}

	inline LRESULT CTab::OnWindowPosChanged(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		RecalcLayout();
		return FinalWindowProc(uMsg, wParam, lParam);
	}

	inline LRESULT CTab::OnWindowPosChanging(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		// A little hack to reduce tab flicker
		if (IsWindowVisible() && (GetWindowLongPtr(GWL_STYLE) & TCS_OWNERDRAWFIXED))
		{
			LPWINDOWPOS pWinPos = (LPWINDOWPOS)lParam;
			pWinPos->flags |= SWP_NOREDRAW;

			// Do a manual paint when OnDraw isn't called.
			if (GetWindowRect().Height() < GetTabHeight())
				Paint();
		}

		return FinalWindowProc(uMsg, wParam, lParam);
	}

	inline void CTab::Paint()
	{
		// Microsoft's drawing for a tab control has quite a bit of flicker, so we do our own.
		// We use double buffering and regions to eliminate flicker

		BOOL RTL = FALSE;
#if (WINVER >= 0x0500)
		RTL = (GetWindowLongPtr(GWL_EXSTYLE) & WS_EX_LAYOUTRTL);
#endif

		// Create the memory DC and bitmap
		CClientDC dcView(*this);
		CMemDC dcMem(dcView);
		CRect rcClient = GetClientRect();
		dcMem.CreateCompatibleBitmap(dcView, rcClient.Width(), rcClient.Height());

		if (GetItemCount() == 0)
		{
			// No tabs, so simply display a grey background and exit
			COLORREF rgbDialog = GetSysColor(COLOR_BTNFACE);
			dcView.SolidFill(rgbDialog, rcClient);
			return;
		}

		// Create a clipping region. Its the overall tab window's region,
		//  less the region belonging to the individual tab view's client area
		CRgn rgnSrc1;
		rgnSrc1.CreateRectRgnIndirect(rcClient);
		CRect rcTab = GetClientRect();
		AdjustRect(FALSE, &rcTab);
		if (rcTab.Height() < 0)
			rcTab.top = rcTab.bottom;
		if (rcTab.Width() < 0)
			rcTab.left = rcTab.right;

		int offset = RTL ? -1 : 0;	// Required for RTL layout
		CRgn rgnSrc2;
		rgnSrc2.CreateRectRgn(rcTab.left, rcTab.top, rcTab.right + offset, rcTab.bottom);
		CRgn rgnClip;
		rgnClip.CreateRectRgn(0, 0, 0, 0);
		rgnClip.CombineRgn(rgnSrc1, rgnSrc2, RGN_DIFF);

		// Use the region in the memory DC to paint the grey background
		dcMem.SelectClipRgn(rgnClip);
		dcMem.CreateSolidBrush( GetSysColor(COLOR_BTNFACE) );
		dcMem.PaintRgn(rgnClip);

		// Draw the tab buttons on the memory DC:
		DrawTabs(dcMem);

		// Draw buttons and tab borders
		DrawCloseButton(dcMem);
		DrawListButton(dcMem);
		DrawTabBorders(dcMem, rcTab);

		// Now copy our from our memory DC to the window DC
		dcView.SelectClipRgn(rgnClip);

		if (RTL)
		{
			// BitBlt offset bitmap copies by one for Right-To-Left layout
			dcView.BitBlt(0, 0, 1, rcClient.Height(), dcMem, 1, 0, SRCCOPY);
			dcView.BitBlt(1, 0, rcClient.Width(), rcClient.Height(), dcMem, 1, 0, SRCCOPY);
		}
		else
			dcView.BitBlt(0, 0, rcClient.Width(), rcClient.Height(), dcMem, 0, 0, SRCCOPY);
	}

	inline void CTab::PreCreate(CREATESTRUCT &cs)
	{
		// For Tabs on the bottom, add the TCS_BOTTOM style
		cs.style = WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE;
	}

	inline void CTab::PreRegisterClass(WNDCLASS& wc)
	{
		wc.lpszClassName = WC_TABCONTROL;
	}

	inline void CTab::RecalcLayout()
	// Repositions the child windows of the tab control.
	{
		if (IsWindow())
		{
			SetTabSize();
			if (GetActiveView())
			{
				// Position the View over the tab control's display area
				CRect rc = GetClientRect();
				MapWindowPoints(GetParent(), rc);
				AdjustRect(FALSE, &rc);
				GetActiveView()->SetWindowPos(NULL, rc, SWP_SHOWWINDOW);
			}

			RedrawWindow(RDW_INVALIDATE|RDW_NOCHILDREN);
		}
	}

	inline void CTab::RemoveTabPage(int nPage)
	// Removes a tab and its view page
	{
		if ((nPage < 0) || (nPage > (int)m_vTabPageInfo.size() -1))
			return;

		// Remove the tab
		DeleteItem(nPage);

		// Remove the TapPageInfo entry
		std::vector<TabPageInfo>::iterator itTPI = m_vTabPageInfo.begin() + nPage;
		CWnd* pView = (*itTPI).pView;
		int iImage = (*itTPI).iImage;
		if (iImage >= 0)
			RemoveImage(iImage);

		if (pView == m_pActiveView)
			m_pActiveView = 0;

		(*itTPI).pView->Destroy();
		m_vTabPageInfo.erase(itTPI);

		std::vector<WndPtr>::iterator itView;
		for (itView = m_vTabViews.begin(); itView != m_vTabViews.end(); ++itView)
		{
			if ((*itView).get() == pView)
			{
				m_vTabViews.erase(itView);
				break;
			}
		}

		if (IsWindow())
		{
			if (m_vTabPageInfo.size() > 0)
			{
				SetTabSize();
				SelectPage(0);
			}
			else
				ShowActiveView(NULL);

			NotifyChanged();
		}

	}

	inline void CTab::SelectPage(int nPage)
	// Selects the tab and the view page
	{
		if ((nPage >= 0) && (nPage < GetItemCount()))
		{
			if (nPage != GetCurSel())
				SetCurSel(nPage);

			ShowActiveView(m_vTabPageInfo[nPage].pView);
		}
	}

	inline void CTab::SetFixedWidth(BOOL IsEnabled)
	// Enable or disable fixed tab width.
	{
		DWORD dwStyle = (DWORD)GetWindowLongPtr(GWL_STYLE);
		if (IsEnabled)
		{
			SetWindowLongPtr(GWL_STYLE, dwStyle | TCS_FIXEDWIDTH);

			// Remove Image list for fixed width and Owner drawn tabs
			if (dwStyle & TCS_OWNERDRAWFIXED)
				SetImageList(NULL);
			else
				SetImageList(m_imlODTab);
		}
		else
		{
			SetWindowLongPtr(GWL_STYLE, dwStyle & ~TCS_FIXEDWIDTH);
			SetImageList(m_imlODTab);
		}

		RecalcLayout();
	}

	inline void CTab::SetFont(HFONT hFont, BOOL Redraw /* = 1 */)
	// Sets the font and adjusts the tab height to match
	{
		int HeightGap = 5;
		SetTabHeight( MAX(20, GetTextHeight() + HeightGap) );
		CWnd::SetFont(hFont, Redraw);
	}

	inline void CTab::SetOwnerDraw(BOOL IsEnabled)
	// Enable or disable owner draw
	{
		DWORD dwStyle = (DWORD)GetWindowLongPtr(GWL_STYLE);
		if (IsEnabled)
		{
			SetWindowLongPtr(GWL_STYLE, dwStyle | TCS_OWNERDRAWFIXED);

			// Remove Image list for tabs with both fixed width and Owner drawn tabs
			if (dwStyle & TCS_FIXEDWIDTH)
				SetImageList(NULL);
			else
				SetImageList(m_imlODTab);
		}
		else
		{
			SetWindowLongPtr(GWL_STYLE, dwStyle & ~TCS_OWNERDRAWFIXED);
			SetImageList(m_imlODTab);
		}

		RecalcLayout();
	}

	inline void CTab::SetShowButtons(BOOL Show)
	// Allows the list and close buttons to be shown or hidden.
	{
		m_IsShowingButtons = Show;
		RecalcLayout();
	}

	inline void CTab::SetTabIcon(int i, HICON hIcon)
	// Changes or sets the tab's icon
	{
		assert (GetItemCount() > i);
		TCITEM tci;
		ZeroMemory(&tci, sizeof(TCITEM));
		tci.mask = TCIF_IMAGE;
		GetItem(i, &tci);
		if (tci.iImage >= 0)
		{
			GetODImageList().Replace(i, hIcon);
		}
		else
		{
			int iImage = GetODImageList().Add(hIcon);
			tci.iImage = iImage;
			SetItem(i, &tci);
			m_vTabPageInfo[i].iImage = iImage;
		}
	}

	inline void CTab::SetTabsAtTop(BOOL IsAtTop)
	// Positions the tabs at the top or bottom of the control
	{
		DWORD dwStyle = (DWORD)GetWindowLongPtr(GWL_STYLE);

		if (IsAtTop)
			dwStyle &= ~TCS_BOTTOM;
		else
			dwStyle |= TCS_BOTTOM;

		SetWindowLongPtr(GWL_STYLE, dwStyle);
		
		RedrawWindow();
		RecalcLayout();
	}

	inline void CTab::SetTabSize()
	// Sets the width and height of tabs in a fixed-width or owner-drawn tab control.
	{
		if (GetItemCount() > 0)
		{
			CRect rc = GetClientRect();
			AdjustRect(FALSE, &rc);

			int xGap = 2;
			if (m_IsShowingButtons) xGap += GetCloseRect().Width() + GetListRect().Width() +2;

			int nItemWidth = MIN( GetMaxTabSize().cx, (rc.Width() - xGap)/GetItemCount() );
			nItemWidth = MAX(nItemWidth, 0);

			SendMessage(TCM_SETITEMSIZE, 0L, MAKELPARAM(nItemWidth, m_nTabHeight));

			NotifyChanged();
		}
	}

	inline void CTab::SetTabText(UINT nTab, LPCTSTR szText)
	{
		// Allows the text to be changed on an existing tab
		if (nTab < GetAllTabs().size())
		{
			TCITEM Item;
			ZeroMemory(&Item, sizeof(TCITEM));
			Item.mask = TCIF_TEXT;
			Item.pszText = const_cast<LPTSTR>(szText);

			if (SetItem(nTab, &Item))
				m_vTabPageInfo[nTab].TabText = szText;
		}
	}

	inline void CTab::ShowActiveView(CWnd* pView)
	// Sets or changes the View window displayed within the tab page
	{
		if (pView != m_pActiveView)
		{
			// Hide the old view
			if (GetActiveView() && (GetActiveView()->IsWindow()))
				GetActiveView()->ShowWindow(SW_HIDE);

			// Assign the view window
			m_pActiveView = pView;

			if (m_pActiveView && *this)
			{
				if (!m_pActiveView->IsWindow())
				{
					// The tab control is already created, so create the new view too
					GetActiveView()->Create( GetParent() );
				}

				// Position the View over the tab control's display area
				CRect rc = GetClientRect();
				AdjustRect(FALSE, &rc);
                MapWindowPoints(GetParent(), rc);
				GetActiveView()->SetWindowPos(0, rc, SWP_SHOWWINDOW);
				GetActiveView()->SetFocus();
			}
		}
	}

	inline void CTab::ShowListMenu()
	// Displays the list of windows in a popup menu
	{
		if (!m_IsListPressed)
		{
			m_IsListPressed = TRUE;

			CPoint pt(GetListRect().left, GetListRect().top + GetTabHeight());
			ClientToScreen(pt);

			// Choosing the frame's CWnd for the menu's messages will automatically theme the popup menu
			int nPage = 0;
			m_IsListMenuActive = TRUE;
			nPage = GetListMenu().TrackPopupMenuEx(TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD, pt.x, pt.y, GetAncestor(), NULL) - IDW_FIRSTCHILD;
			if ((nPage >= 0) && (nPage < 9)) SelectPage(nPage);
			if (nPage == 9) ShowListDialog();
			m_IsListMenuActive = FALSE;
		}

		CClientDC dc(*this);
		DrawListButton(dc);
	}

	inline void CTab::ShowListDialog()
	{
		// Definition of a dialog template which displays a List Box
		unsigned char dlg_Template[] =
		{
			0x01,0x00,0xff,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x04,0x00,0xc8,0x00,0xc8,0x90,0x03,
			0x00,0x00,0x00,0x00,0x00,0xdc,0x00,0x8e,0x00,0x00,0x00,0x00,0x00,0x53,0x00,0x65,
			0x00,0x6c,0x00,0x65,0x00,0x63,0x00,0x74,0x00,0x20,0x00,0x57,0x00,0x69,0x00,0x6e,
			0x00,0x64,0x00,0x6f,0x00,0x77,0x00,0x00,0x00,0x08,0x00,0x00,0x00,0x00,0x01,0x4d,
			0x00,0x53,0x00,0x20,0x00,0x53,0x00,0x68,0x00,0x65,0x00,0x6c,0x00,0x6c,0x00,0x20,
			0x00,0x44,0x00,0x6c,0x00,0x67,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x01,0x00,0x01,0x50,0x40,0x00,0x7a,0x00,0x25,0x00,0x0f,0x00,0x01,
			0x00,0x00,0x00,0xff,0xff,0x80,0x00,0x4f,0x00,0x4b,0x00,0x00,0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x50,0x7a,0x00,0x7a,0x00,0x25,
			0x00,0x0f,0x00,0x02,0x00,0x00,0x00,0xff,0xff,0x80,0x00,0x43,0x00,0x61,0x00,0x6e,
			0x00,0x63,0x00,0x65,0x00,0x6c,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
			0x00,0x02,0x00,0x01,0x01,0x21,0x50,0x06,0x00,0x06,0x00,0xcf,0x00,0x6d,0x00,0x79,
			0x00,0x00,0x00,0xff,0xff,0x83,0x00,0x00,0x00,0x00,0x00
		};

		// Display the modal dialog. The dialog is defined in the dialog template rather
		// than in the resource script (rc) file.
		CSelectDialog SelectDialog((LPCDLGTEMPLATE) dlg_Template);
		for(UINT u = 0; u < GetAllTabs().size(); ++u)
		{
			SelectDialog.AddItem(GetAllTabs()[u].TabText);
		}

		int iSelected = (int)SelectDialog.DoModal(*this);
		if (iSelected >= 0) SelectPage(iSelected);
	}

	inline void CTab::SwapTabs(UINT nTab1, UINT nTab2)
	// Swaps the two specified tabs.
	{
		if ((nTab1 < GetAllTabs().size()) && (nTab2 < GetAllTabs().size()) && (nTab1 != nTab2))
		{
			TabPageInfo T1 = GetTabPageInfo(nTab1);
			TabPageInfo T2 = GetTabPageInfo(nTab2);
			int nLength = 30;

			TCITEM Item1;
			ZeroMemory(&Item1, sizeof(TCITEM));
			Item1.mask = TCIF_IMAGE | TCIF_PARAM | TCIF_RTLREADING | TCIF_STATE | TCIF_TEXT;
			Item1.cchTextMax = nLength;
			Item1.pszText = const_cast<LPTSTR>(T1.TabText.c_str());
			GetItem(nTab1, &Item1);

			TCITEM Item2;
			ZeroMemory(&Item2, sizeof(TCITEM));
			Item2.mask = TCIF_IMAGE | TCIF_PARAM | TCIF_RTLREADING | TCIF_STATE | TCIF_TEXT;
			Item2.cchTextMax = nLength;
			Item2.pszText = const_cast<LPTSTR>(T2.TabText.c_str());
			GetItem(nTab2, &Item2);

			SetItem(nTab1, &Item2);
			SetItem(nTab2, &Item1);
			m_vTabPageInfo[nTab1] = T2;
			m_vTabPageInfo[nTab2] = T1;
		}
	}

	inline LRESULT CTab::WndProcDefault(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch(uMsg)
		{
		case WM_PAINT:				return OnPaint(uMsg, wParam, lParam);
		case WM_ERASEBKGND:			return OnEraseBkgnd(uMsg, wParam, lParam);
		case WM_KILLFOCUS:			return OnKillFocus(uMsg, wParam, lParam);
		case WM_LBUTTONDBLCLK:		return OnLButtonDblClk(uMsg, wParam, lParam);
		case WM_LBUTTONDOWN:		return OnLButtonDown(uMsg, wParam, lParam);
		case WM_LBUTTONUP:			return OnLButtonUp(uMsg, wParam, lParam);
		case WM_MOUSEMOVE:			return OnMouseMove(uMsg, wParam, lParam);
		case WM_MOUSELEAVE:			return OnMouseLeave(uMsg, wParam, lParam);
		case WM_NCHITTEST:			return OnNCHitTest(uMsg, wParam, lParam);
		case WM_SETFOCUS:			return OnSetFocus(uMsg, wParam, lParam);
		case WM_WINDOWPOSCHANGED:	return OnWindowPosChanged(uMsg, wParam, lParam);
		case WM_WINDOWPOSCHANGING:	return OnWindowPosChanging(uMsg, wParam, lParam);
		}

		// pass unhandled messages on for default processing
		return CWnd::WndProcDefault(uMsg, wParam, lParam);
	}

	// Wrappers for Win32 Macros
	inline void CTab::AdjustRect(BOOL IsLarger, RECT *prc) const
	// Calculates a tab control's display area given a window rectangle, or calculates
	//  the window rectangle that would correspond to a specified display area.
	{
		assert(IsWindow());
		TabCtrl_AdjustRect(*this, IsLarger, prc);
	}

	inline BOOL CTab::DeleteAllItems() const
	// Removes all items from a tab control.
	{
		assert(IsWindow());
		return TabCtrl_DeleteAllItems(*this);
	}

	inline BOOL CTab::DeleteItem(int iItem) const
	// Removes an item from a tab control.
	{
		assert(IsWindow());
		return TabCtrl_DeleteItem(*this, iItem);
	}

	inline void CTab::DeselectAll(UINT fExcludeFocus) const
	// Resets items in a tab control, clearing any that were set to the TCIS_BUTTONPRESSED state.
	{
		assert(IsWindow());
		TabCtrl_DeselectAll(*this, fExcludeFocus);
	}

	inline int CTab::GetCurFocus() const
	// Returns the index of the item that has the focus in a tab control.
	{
		assert(IsWindow());
		return TabCtrl_GetCurFocus(*this);
	}

	inline int CTab::GetCurSel() const
	// Determines the currently selected tab in a tab control.
	{
		assert(IsWindow());
		return TabCtrl_GetCurSel(*this);
	}

	inline DWORD CTab::GetExtendedStyle() const
	// Retrieves the extended styles that are currently in use for the tab control.
	{
		assert(IsWindow());
		return TabCtrl_GetExtendedStyle(*this);
	}

	inline CImageList CTab::GetImageList() const
	// Retrieves the image list associated with a tab control.
	{
		assert(IsWindow());
		HIMAGELIST himl = TabCtrl_GetImageList(*this);
		return CImageList(himl);
	}

	inline BOOL CTab::GetItem(int iItem, LPTCITEM pitem) const
	// Retrieves information about a tab in a tab control.
	{
		assert(IsWindow());
		return TabCtrl_GetItem(*this, iItem, pitem);
	}

	inline int CTab::GetItemCount() const
	// Retrieves the number of tabs in the tab control.
	{
		assert(IsWindow());
		return TabCtrl_GetItemCount(*this);
	}

	inline BOOL CTab::GetItemRect(int iItem, RECT& rc) const
	// Retrieves the bounding rectangle for a tab in a tab control.
	{
		assert(IsWindow());
		return TabCtrl_GetItemRect(*this, iItem, &rc);
	}

	inline int CTab::GetRowCount() const
	// Retrieves the current number of rows of tabs in a tab control.
	{
		assert(IsWindow());
		return TabCtrl_GetRowCount(*this);
	}

	inline HWND CTab::GetToolTips() const
	// Retrieves a handle to the ToolTip control associated with a tab control.
	{
		assert(IsWindow());
		return TabCtrl_GetToolTips(*this);
	}

	inline BOOL CTab::HighlightItem(INT idItem, WORD Highlight) const
	// Sets the highlight state of a tab item.
	{
		assert(IsWindow());
		return TabCtrl_HighlightItem(*this, idItem, Highlight);
	}

	inline int CTab::HitTest(TCHITTESTINFO& info) const
	// Determines which tab, if any, is at a specified screen position.
	{
		assert(IsWindow());
		return TabCtrl_HitTest(*this, &info);
	}

	inline int CTab::InsertItem(int iItem, const LPTCITEM pItem) const
	// Inserts a new tab in a tab control.
	{
		assert(IsWindow());
		assert(pItem);
		return TabCtrl_InsertItem(*this, iItem, pItem);
	}

	inline void CTab::RemoveImage(int iImage) const
	// Removes an image from a tab control's image list.
	{
		assert(IsWindow());
		TabCtrl_RemoveImage(*this, iImage);
	}

	inline void CTab::SetCurFocus(int iItem) const
	// Sets the focus to a specified tab in a tab control.
	{
		assert(IsWindow());
		TabCtrl_SetCurFocus(*this, iItem);
	}

	inline int CTab::SetCurSel(int iItem) const
	// Selects a tab in a tab control.
	{
		assert(IsWindow());
		return TabCtrl_SetCurSel(*this, iItem);
	}

	inline DWORD CTab::SetExtendedStyle(DWORD dwExStyle) const
	// Sets the extended styles that the tab control will use.
	{
		assert(IsWindow());
		return TabCtrl_SetExtendedStyle(*this, dwExStyle);
	}

	inline CImageList CTab::SetImageList(HIMAGELIST himlNew) const
	// Assigns an image list to a tab control.
	{
		assert(IsWindow());
		HIMAGELIST himl = TabCtrl_SetImageList( *this, himlNew );
		return CImageList(himl);
	}

	inline BOOL CTab::SetItem(int iItem, LPTCITEM pItem) const
	// Sets some or all of a tab's attributes.
	{
		assert(IsWindow());
		assert(pItem);
		return TabCtrl_SetItem(*this, iItem, pItem);
	}

	inline BOOL CTab::SetItemExtra(int cb) const
	// Sets the number of bytes per tab reserved for application-defined data in a tab control.
	{
		assert(IsWindow());
		return TabCtrl_SetItemExtra(*this, cb);
	}

	inline DWORD CTab::SetItemSize(int cx, int cy) const
	// Sets the width and height of tabs in a fixed-width or owner-drawn tab control.
	{
		assert(IsWindow());
		return TabCtrl_SetItemSize(*this, cx, cy);
	}

	inline int CTab::SetMinTabWidth(int cx) const
	// Sets the minimum width of items in a tab control.
	{
		assert(IsWindow());
		return TabCtrl_SetMinTabWidth(*this, cx);
	}

	inline void CTab::SetPadding(int cx, int cy) const
	// Sets the amount of space (padding) around each tab's icon and label in a tab control.
	{
		assert(IsWindow());
		TabCtrl_SetPadding(*this, cx, cy);
	}

	inline void CTab::SetToolTips(HWND hToolTip) const
	// Assigns a ToolTip control to a tab control.
	{
		assert(IsWindow());
		TabCtrl_SetToolTips(*this, hToolTip);
	}

	////////////////////////////////////////
	// Definitions for the CTabbedMDI class
	inline CTabbedMDI::CTabbedMDI()
	{
		GetTab().SetShowButtons(TRUE);
	}

	inline CTabbedMDI::~CTabbedMDI()
	{
	}

	inline CWnd* CTabbedMDI::AddMDIChild(CWnd* pView, LPCTSTR szTabText, int idMDIChild /*= 0*/)
	// Adds a MDI tab, given a pointer to the view window, and the tab's text.
	// The framework assumes ownership of the CWnd pointer provided, and deletes
	// the CWnd object when the window is destroyed.
	{
		assert(pView); // Cannot add Null CWnd*
		assert(lstrlen(szTabText) < MAX_MENU_STRING);

		GetTab().AddTabPage(pView, szTabText, 0, idMDIChild);

		// Fake a WM_MOUSEACTIVATE to propagate focus change to dockers
		if (IsWindow())
			GetParent().SendMessage(WM_MOUSEACTIVATE, (WPARAM)GetAncestor().GetHwnd(), MAKELPARAM(HTCLIENT,WM_LBUTTONDOWN));

		return pView;
	}

	inline void CTabbedMDI::CloseActiveMDI()
	{
		int nTab = GetTab().GetCurSel();
		if (nTab >= 0)
			GetTab().RemoveTabPage(nTab);

		RecalcLayout();
	}

	inline void CTabbedMDI::CloseAllMDIChildren()
	{
		while (GetMDIChildCount() > 0)
		{
			GetTab().RemoveTabPage(0);
		}
	}

	inline void CTabbedMDI::CloseMDIChild(int nTab)
	{
		GetTab().RemoveTabPage(nTab);

		if (GetActiveMDIChild())
			GetActiveMDIChild()->RedrawWindow();
	}

	inline HWND CTabbedMDI::Create(HWND hWndParent /* = NULL*/)
	{
		CLIENTCREATESTRUCT clientcreate ;
		clientcreate.hWindowMenu  = *this;
		clientcreate.idFirstChild = IDW_FIRSTCHILD ;
		DWORD dwStyle = WS_CHILD | WS_VISIBLE | MDIS_ALLCHILDSTYLES | WS_CLIPCHILDREN;

		// Create the MDICLIENT view window
		if (!CreateEx(0, _T("MDICLIENT"), _T(""),
			dwStyle, 0, 0, 0, 0, hWndParent, NULL, (PSTR) &clientcreate))
				throw CWinException(_T("CMDIClient::Create ... CreateEx failed"));

		return *this;
	}

	inline CWnd* CTabbedMDI::GetActiveMDIChild() const
	{
		CWnd* pView = NULL;
		int nTab = GetTab().GetCurSel();
		if (nTab >= 0)
		{
			TabPageInfo tbi = GetTab().GetTabPageInfo(nTab);
			pView = tbi.pView;
		}

		return pView;
	}

	inline int CTabbedMDI::GetActiveMDITab() const
	{
		return GetTab().GetCurSel();
	}

	inline CWnd* CTabbedMDI::GetMDIChild(int nTab) const
	{
		assert(nTab >= 0);
		assert(nTab < GetMDIChildCount());
		return GetTab().GetTabPageInfo(nTab).pView;
	}

	inline int CTabbedMDI::GetMDIChildCount() const
	{
		return static_cast<int>( GetTab().GetAllTabs().size() );
	}

	inline int   CTabbedMDI::GetMDIChildID(int nTab) const
	{
		assert(nTab >= 0);
		assert(nTab < GetMDIChildCount());
		return GetTab().GetTabPageInfo(nTab).idTab;
	}

	inline LPCTSTR CTabbedMDI::GetMDIChildTitle(int nTab) const
	{
		assert(nTab >= 0);
		assert(nTab < GetMDIChildCount());
		return GetTab().GetTabPageInfo(nTab).TabText;
	}

	inline BOOL CTabbedMDI::LoadRegistrySettings(LPCTSTR szKeyName)
	{
		BOOL IsLoaded = FALSE;

		if (szKeyName)
		{
			CString KeyName = _T("Software\\") + CString(szKeyName) + _T("\\MDI Children");
			CRegKey Key;
			if (ERROR_SUCCESS == Key.Open(HKEY_CURRENT_USER, KeyName))
			{
				DWORD dwIDTab;
				int i = 0;
				CString SubKeyName;
				CString TabText;
				SubKeyName.Format(_T("ID%d"), i);

				// Fill the DockList vector from the registry
				while (ERROR_SUCCESS == Key.QueryDWORDValue(SubKeyName, dwIDTab))
				{
					SubKeyName.Format(_T("Text%d"), i);
					DWORD dwBufferSize = 0;
					if (ERROR_SUCCESS == Key.QueryStringValue(SubKeyName, 0, &dwBufferSize))
					{
						Key.QueryStringValue(SubKeyName, TabText.GetBuffer(dwBufferSize), &dwBufferSize);
						TabText.ReleaseBuffer();
					}

					CWnd* pWnd = NewMDIChildFromID(dwIDTab);
					if (pWnd)
					{
						AddMDIChild(pWnd, TabText, dwIDTab);
						i++;
						SubKeyName.Format(_T("ID%d"), i);
						IsLoaded = TRUE;
					}
					else
					{
						TRACE("Failed to get TabbedMDI info from registry");
						IsLoaded = FALSE;
						break;
					}
				}

				if (IsLoaded)
				{
					// Load Active MDI Tab from the registry
					SubKeyName = _T("Active MDI Tab");
					DWORD dwTab;
					if (ERROR_SUCCESS == Key.QueryDWORDValue(SubKeyName, dwTab))
						SetActiveMDITab(dwTab);
					else
						SetActiveMDITab(0);
				}
			}
		}

		if (!IsLoaded)
			CloseAllMDIChildren();

		return IsLoaded;
	}

	inline CWnd* CTabbedMDI::NewMDIChildFromID(int /*idMDIChild*/)
	{
		// Override this function to create new MDI children from IDs as shown below
		CWnd* pView = NULL;
	/*	switch(idTab)
		{
		case ID_SIMPLE:
			pView = new CViewSimple;
			break;
		case ID_RECT:
			pView = new CViewRect;
			break;
		default:
			TRACE("Unknown MDI child ID\n");
			break;
		} */

		return pView;
	}

	inline void CTabbedMDI::OnAttach()
	{
		GetTab().Create(*this);
		GetTab().SetFixedWidth(TRUE);
		GetTab().SetOwnerDraw(TRUE);
	}

	inline void CTabbedMDI::OnDestroy()
	{
		CloseAllMDIChildren();
	}

	inline LRESULT CTabbedMDI::OnNotify(WPARAM /*wParam*/, LPARAM lParam)
	{
		LPNMHDR pnmhdr = (LPNMHDR)lParam;
		assert(pnmhdr);

		switch(pnmhdr->code)
		{

		case UMN_TABCHANGED:
			RecalcLayout();
			break;

		case UWN_TABDRAGGED:
			{
				CPoint pt = GetCursorPos();
				GetTab().ScreenToClient(pt);

				TCHITTESTINFO info;
				ZeroMemory(&info, sizeof(TCHITTESTINFO));
				info.pt = pt;
				int nTab = GetTab().HitTest(info);
				if (nTab >= 0)
				{
					if (nTab !=  GetActiveMDITab())
					{
						GetTab().SwapTabs(nTab, GetActiveMDITab());
						SetActiveMDITab(nTab);
					}
				}

				break;
			}

		case UWN_TABCLOSE:
			{
				TABNMHDR* pTabNMHDR = (TABNMHDR*)lParam;
				return !OnTabClose(pTabNMHDR->nPage);
			}

		}	// switch(pnmhdr->code)

		// The framework will call SetWindowLongPtr(DWLP_MSGRESULT, lr) for non-zero returns
		return 0L;
	}

	inline BOOL CTabbedMDI::OnTabClose(int nPage)
	// Override this function to determine what happens when a tab is about to close.
	// Return TRUE to allow the tab to close, or FALSE to prevent the tab closing.
	{
		UNREFERENCED_PARAMETER(nPage);

		// Allow the tab to be close
		return TRUE;
	}

	inline LRESULT CTabbedMDI::OnWindowPosChanged(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		RecalcLayout();
		return FinalWindowProc(uMsg, wParam, lParam);
	}

	inline void CTabbedMDI::RecalcLayout()
	{
		if (GetTab().IsWindow())
		{
			if (GetTab().GetItemCount() >0)
			{
				CRect rcClient = GetClientRect();
				GetTab().SetWindowPos(NULL, rcClient, SWP_SHOWWINDOW);
				GetTab().UpdateWindow();
			}
			else
			{
				CRect rcClient = GetClientRect();
				GetTab().SetWindowPos(NULL, rcClient, SWP_HIDEWINDOW);
				Invalidate();
			}
		}
	}

	inline BOOL CTabbedMDI::SaveRegistrySettings(LPCTSTR szKeyName)
	{
		if (szKeyName)
		{
			CString KeyName = _T("Software\\") + CString(szKeyName);
			HKEY hKey = NULL;
			HKEY hKeyMDIChild = NULL;

			try
			{
				if (ERROR_SUCCESS != RegCreateKeyEx(HKEY_CURRENT_USER, KeyName, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL))
					throw (CUserException(_T("RegCreateKeyEx Failed")));

				RegDeleteKey(hKey, _T("MDI Children"));
				if (ERROR_SUCCESS != RegCreateKeyEx(hKey, _T("MDI Children"), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKeyMDIChild, NULL))
					throw (CUserException(_T("RegCreateKeyEx Failed")));

				for (int i = 0; i < GetMDIChildCount(); ++i)
				{
					CString SubKeyName;
					TabPageInfo pdi = GetTab().GetTabPageInfo(i);

					SubKeyName.Format(_T("ID%d"), i);
					if (ERROR_SUCCESS != RegSetValueEx(hKeyMDIChild, SubKeyName, 0, REG_DWORD, (LPBYTE)&pdi.idTab, sizeof(int)))
						throw (CUserException(_T("RegSetValueEx Failed")));

					SubKeyName.Format(_T("Text%d"), i);
					CString TabText = GetTab().GetTabPageInfo(i).TabText;
					if (ERROR_SUCCESS != RegSetValueEx(hKeyMDIChild, SubKeyName, 0, REG_SZ, (LPBYTE)TabText.c_str(), (1 + TabText.GetLength() )*sizeof(TCHAR)))
						throw (CUserException(_T("RegSetValueEx Failed")));
				}

				// Add Active Tab to the registry
				CString SubKeyName = _T("Active MDI Tab");
				int nTab = GetActiveMDITab();
				if(ERROR_SUCCESS != RegSetValueEx(hKeyMDIChild, SubKeyName, 0, REG_DWORD, (LPBYTE)&nTab, sizeof(int)))
					throw (CUserException(_T("RegSetValueEx failed")));

				RegCloseKey(hKeyMDIChild);
				RegCloseKey(hKey);
			}
			catch (const CUserException& e)
			{
				Trace("*** Failed to save TabbedMDI settings in registry. ***\n");
				Trace(e.GetText()); Trace("\n");

				// Roll back the registry changes by deleting the subkeys
				if (hKey != 0)
				{
					if (hKeyMDIChild)
					{
						RegDeleteKey(hKeyMDIChild, _T("MDI Children"));
						RegCloseKey(hKeyMDIChild);
					}

					RegDeleteKey(HKEY_CURRENT_USER, KeyName);
					RegCloseKey(hKey);
				}

				return FALSE;
			}
		}

		return TRUE;
	}

	inline void CTabbedMDI::SetActiveMDIChild(CWnd* pWnd) const
	{
		assert(pWnd);
		int nPage = GetTab().GetTabIndex(pWnd);
		if (nPage >= 0)
			GetTab().SelectPage(nPage);
	}

	inline void CTabbedMDI::SetActiveMDITab(int iTab) const
	{
		assert(IsWindow());
		assert(GetTab().IsWindow());
		GetTab().SelectPage(iTab);
	}

	inline LRESULT CTabbedMDI::OnSetFocus(UINT, WPARAM, LPARAM)
	{
		if (GetActiveMDIChild() && GetActiveMDIChild()->IsWindow())
			GetActiveMDIChild()->SetFocus();

		return 0L;
	}

	inline LRESULT CTabbedMDI::WndProcDefault(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch(uMsg)
		{
		case WM_SETFOCUS:			return OnSetFocus(uMsg, wParam, lParam);
		case WM_WINDOWPOSCHANGED:	return OnWindowPosChanged(uMsg, wParam, lParam);
		case UWM_GETCTABBEDMDI:		return reinterpret_cast<LRESULT>(this);
		}

		return CWnd::WndProcDefault(uMsg, wParam, lParam);
	}

} // namespace Win32xx

#endif  // _WIN32XX_TAB_H_
