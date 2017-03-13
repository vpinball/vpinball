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


////////////////////////////////////////////////////////
// wxx_controls.h
//  Declaration of the following classes:
//  CAnimation, CComboBox, CComboBoxEx, CDateTime, CIPAddress,
//  CHeader, CHotKey, CMonthCalendar, CProgressBar, CScrollBar,
//  CSlider, CSpinButton, and CToolTip.


#ifndef _WIN32XX_CONTROLS_H_
#define _WIN32XX_CONTROLS_H_


#include "wxx_wincore0.h"
#include "wxx_stdcontrols.h"
#include "wxx_imagelist.h"
#include "wxx_ddx.h"


namespace Win32xx
{

	// Forward declarations
	class CMonthCalendar;
	class CToolTip;

	class CAnimation : public CWnd
	{
	public:
		CAnimation() {}
		virtual ~CAnimation() {}

		BOOL Close() const;
		BOOL Open(LPTSTR lpszName) const;
		BOOL Play(int nFrom, int nTo, int nRepeat) const;
		BOOL Seek(int nFrame) const;
		BOOL Stop() const;

	protected:
		// Overridables
		virtual void PreRegisterClass(WNDCLASS& wc) { wc.lpszClassName = ANIMATE_CLASS; }

	private:
		CAnimation(const CAnimation&);				// Disable copy construction
		CAnimation& operator = (const CAnimation&); // Disable assignment operator
	};


	class CComboBox : public CWnd
	{
	public:
		CComboBox() {}
		virtual ~CComboBox() {}

		int   AddString(LPCTSTR lpszString) const;
		void  Clear() const;
		void  Copy() const;
		void  Cut() const;
		int   DeleteString(int nIndex) const;
		int   Dir(UINT attr, LPCTSTR lpszWildCard ) const;
		int   FindString(int nIndexStart, LPCTSTR lpszString) const;
		int   FindStringExact(int nIndexStart, LPCTSTR lpszString) const;
		int   GetCount() const;
		int   GetCurSel() const;
		CRect GetDroppedControlRect() const;
		BOOL  GetDroppedState() const;
		int   GetDroppedWidth() const;
		DWORD GetEditSel() const;
		BOOL  GetExtendedUI() const;
		int   GetHorizontalExtent() const;
		DWORD GetItemData(int nIndex) const;
		int   GetItemHeight(int nIndex) const;
		int   GetLBText(int nIndex, LPTSTR lpszText) const;
		int   GetLBTextLen(int nIndex) const;
		LCID  GetLocale() const;
		int   GetTopIndex() const;
		int   InitStorage(int nItems, int nBytes) const;
		int   InsertString(int nIndex, LPCTSTR lpszString) const;
		void  LimitText(int nMaxChars) const;
		void  Paste() const;
		void  ResetContent() const;
		int   SelectString(int nStartAfter, LPCTSTR lpszString) const;
		int   SetCurSel(int nIndex) const;
		int   SetDroppedWidth(int nWidth) const;
		BOOL  SetEditSel(int nStartChar, int nEndChar) const;
		int   SetExtendedUI(BOOL IsExtended = TRUE) const;
		void  SetHorizontalExtent(UINT nExtent ) const;
		int   SetItemData(int nIndex, DWORD dwItemData) const;
		int   SetItemHeight(int nIndex, UINT cyItemHeight) const;
		LCID  SetLocale( LCID NewLocale ) const;
		int   SetTopIndex(int nIndex) const;
		void  ShowDropDown(BOOL Show = TRUE) const;

	protected:
		// Overridables
		virtual void PreRegisterClass(WNDCLASS& wc) { wc.lpszClassName = _T("ComboBox"); }

	private:
		CComboBox(const CComboBox&);				// Disable copy construction
		CComboBox& operator = (const CComboBox&);	// Disable assignment operator
	};


	class CComboBoxEx : public CComboBox
	{
	public:
		CComboBoxEx() {}
		virtual ~CComboBoxEx() {}

		int  	DeleteItem(int nIndex ) const;
		HWND 	GetComboBoxCtrl() const;
		HWND 	GetEditCtrl() const;
		CImageList GetImageList() const;
		BOOL 	GetItem(COMBOBOXEXITEM& CBItem) const;
		BOOL 	HasEditChanged () const;
		int     InsertItem(const COMBOBOXEXITEM& CBItem) const;
		CImageList SetImageList(HIMAGELIST himlNew) const;
		BOOL 	SetItem(const COMBOBOXEXITEM& CBItem) const;
		DWORD 	GetExtendedStyle() const;
		DWORD 	SetExtendedStyle(DWORD dwExMask, DWORD dwExStyles ) const;

	protected:
		// Overridables
		virtual void PreRegisterClass(WNDCLASS& wc) { wc.lpszClassName = WC_COMBOBOXEX; }

	private:
		CComboBoxEx(const CComboBoxEx&);				// Disable copy construction
		CComboBoxEx& operator = (const CComboBoxEx&);	// Disable assignment operator
	};

	class CHeader : public CWnd
	{
	public:
		CHeader() {}
		virtual ~CHeader() {}

		// Attributes
		CImageList GetImageList() const;
		BOOL	GetItem(int nPos, HDITEM& HeaderItem) const;
		int		GetItemCount() const;
		CRect	GetItemRect(int nIndex) const;
		BOOL	GetOrderArray(LPINT piArray, int iCount) const;
		int		OrderToIndex(int nOrder) const;
		CImageList SetImageList(HIMAGELIST himlNew) const;
		BOOL	SetItem(int nPos, const HDITEM& HeaderItem) const;
		BOOL	SetOrderArray(int iCount, LPINT piArray) const;
		int		GetBitmapMargin() const;
		int		SetBitmapMargin(int nWidth) const;

		// Operations
		CImageList CreateDragImage(int nIndex) const;
		BOOL	DeleteItem(int nPos) const;
		int		InsertItem(int nPos, const HDITEM& phdi) const;
		BOOL	Layout(HDLAYOUT* pHeaderLayout) const;
#ifdef Header_SetHotDivider
		int		SetHotDivider(CPoint pt) const;
		int		SetHotDivider(int nIndex) const;
#endif
#ifdef Header_ClearFilter
		int		ClearAllFilters() const;
		int		ClearFilter(int nColumn) const;
		int		EditFilter(int nColumn, BOOL DiscardChanges) const;
		int		SetFilterChangeTimeout(DWORD dwTimeOut) const;
#endif

	protected:
		// Overridables
		virtual void PreRegisterClass(WNDCLASS& wc) { wc.lpszClassName = WC_HEADER ; }

	private:
		CHeader(const CHeader&);				// Disable copy construction
		CHeader& operator = (const CHeader&);	// Disable assignment operator
	};

	class CHotKey : public CWnd
	{
	public:
		CHotKey() {}
		virtual ~CHotKey() {}

		DWORD GetHotKey() const;
		CString GetKeyName(UINT vk, BOOL IsExtended) const;
		void SetHotKey(DWORD dwKey) const;
		void SetRules(WORD wInvalidComb, WORD wModifiers) const;

	protected:
		// Overridables
		virtual void PreRegisterClass(WNDCLASS& wc) { wc.lpszClassName = HOTKEY_CLASS; }

	private:
		CHotKey(const CHotKey&);				// Disable copy construction
		CHotKey& operator = (const CHotKey&);	// Disable assignment operator
	};

	class CIPAddress : public CWnd
	{
	public:
		CIPAddress();
		virtual ~CIPAddress() {}

		void ClearAddress() const;
		int GetAddress(BYTE& nField0, BYTE& nField1, BYTE& nField2, BYTE& nField3) const;
		int GetAddress(DWORD& dwAddress) const;
		BOOL IsBlank() const;
		void SetAddress(BYTE nField0, BYTE nField1, BYTE nField2, BYTE nField3) const;
		void SetAddress(DWORD dwAddress) const;
		void SetFieldFocus(WORD nField) const;
		void SetFieldRange(int nField, BYTE nLower, BYTE nUpper) const;

	protected:
		// Overridables
		virtual void PreRegisterClass(WNDCLASS& wc) { wc.lpszClassName = WC_IPADDRESS; }

	private:
		CIPAddress(const CIPAddress&);				// Disable copy construction
		CIPAddress& operator = (const CIPAddress&);	// Disable assignment operator
	};

	class CMonthCalendar : public CWnd
	{
	public:
		CMonthCalendar();
		virtual ~CMonthCalendar() {}

		// Attributes
		COLORREF GetColor(int nRegion) const;
		int GetFirstDayOfWeek(BOOL* pbLocal = NULL) const;
		CRect GetMinReqRect() const;
		int GetMonthDelta() const;
		COLORREF SetColor(int nRegion, COLORREF ref) const;
		BOOL SetFirstDayOfWeek(int iDay, int* lpnOld = NULL) const;
		int SetMonthDelta(int iDelta) const;

		// Operations
		SYSTEMTIME GetCurSel() const;
		int GetMaxSelCount() const;
		int GetMonthRange(SYSTEMTIME& MinRange, SYSTEMTIME& MaxRange, DWORD dwFlags) const;
		LRESULT GetRange(SYSTEMTIME& MinRange, SYSTEMTIME& MaxRange) const;
		LRESULT GetSelRange(SYSTEMTIME& MinRange, SYSTEMTIME& MaxRange) const;
		SYSTEMTIME GetToday() const;
		LRESULT HitTest(MCHITTESTINFO& MCHitTest) const;
		BOOL SetCurSel(const SYSTEMTIME& DateTime) const;
		BOOL SetDayState(int nMonths, const MONTHDAYSTATE& States) const;
		BOOL SetMaxSelCount(int nMax) const;
		BOOL SetRange(const SYSTEMTIME& MinRange, const SYSTEMTIME& MaxRange) const;
		BOOL SetSelRange(const SYSTEMTIME& MinRange, const SYSTEMTIME& MaxRange) const;
		void SetToday(const SYSTEMTIME&  DateTime) const;

	protected:
		// Overridables
		virtual void PreRegisterClass(WNDCLASS& wc) { wc.lpszClassName = MONTHCAL_CLASS; }

	private:
		CMonthCalendar(const CMonthCalendar&);				// Disable copy construction
		CMonthCalendar& operator = (const CMonthCalendar&);	// Disable assignment operator
	};

	class CDateTime : public CWnd
	{
	public:
		CDateTime();
		virtual ~CDateTime() {}

		COLORREF GetMonthCalColor(int iColor) const;
		HWND GetMonthCalCtrl() const;
		CFont GetMonthCalFont() const;
		DWORD GetRange(SYSTEMTIME& MinRange, SYSTEMTIME& MaxRange) const;
		SYSTEMTIME GetTime(DWORD* pReturnCode = NULL) const;
		COLORREF SetMonthCalColor(int iColor, COLORREF ref) const;
		BOOL SetFormat(LPCTSTR pstrFormat) const;
		void SetMonthCalFont(HFONT hFont, BOOL Redraw = TRUE) const;
		BOOL SetRange(const SYSTEMTIME& MinRange, const SYSTEMTIME& MaxRange) const;
		BOOL SetTime(const SYSTEMTIME& TimeNew) const;
		BOOL SetTimeNone() const;

	protected:
		// Overridables
		virtual void PreRegisterClass(WNDCLASS& wc) { wc.lpszClassName = DATETIMEPICK_CLASS; }

	private:
		CDateTime(const CDateTime&);				// Disable copy construction
		CDateTime& operator = (const CDateTime&);	// Disable assignment operator								// Used by GetMonthCalFont
	};

	class CProgressBar : public CWnd
	{
	public:
		CProgressBar() {}
		virtual ~CProgressBar() {}

		int  GetPos() const;
		int  GetRange(BOOL WhichLimit, const PBRANGE& PBRange) const;
		int  GetRange(BOOL WhichLimit) const;
		int  OffsetPos(int nIncrement) const;
		int  SetPos(int nNewPos) const;
		int  SetRange(short nMinRange, short nMaxRange) const;
		int  SetStep(int nStepInc) const;
		int  StepIt() const;

	protected:
		// Overridables
		virtual void PreRegisterClass(WNDCLASS& wc) { wc.lpszClassName = PROGRESS_CLASS; }

	private:
		CProgressBar(const CProgressBar&);				// Disable copy construction
		CProgressBar& operator = (const CProgressBar&);	// Disable assignment operator
	};

	class CScrollBar : public CWnd
	{
	public:
		CScrollBar() {}
		virtual ~CScrollBar() {}

		BOOL EnableScrollBar( UINT nArrowFlags = ESB_ENABLE_BOTH )  const;
		BOOL GetScrollInfo(SCROLLINFO& si)  const;
		int  GetScrollPos()  const;
		BOOL GetScrollRange(int& MinPos, int& MaxPos)  const;
		BOOL SetScrollInfo(const SCROLLINFO& si, BOOL Redraw = TRUE )  const;
		int  SetScrollPos(int nPos, BOOL Redraw)  const;
		BOOL SetScrollRange( int nMinPos, int nMaxPos, BOOL Redraw = TRUE )  const;
		BOOL ShowScrollBar(BOOL Show)  const;

	protected:
		// Overridables
		virtual void PreRegisterClass(WNDCLASS& wc) { wc.lpszClassName = _T("SCROLLBAR"); ; }

	private:
		CScrollBar(const CScrollBar&);				// Disable copy construction
		CScrollBar& operator = (const CScrollBar&);	// Disable assignment operator
	};

	class CSlider : public CWnd
	{
	public:
		CSlider() {}
		virtual ~CSlider() {}

		void ClearSel() const;
		void ClearTics(BOOL Redraw = FALSE ) const;
		HWND GetBuddy(BOOL fLocation = TRUE ) const;
		CRect GetChannelRect() const;
		int  GetLineSize() const;
		int  GetNumTics() const;
		int  GetPageSize() const;
		int  GetPos() const;
		int  GetRangeMax() const;
		int  GetRangeMin() const;
		int  GetSelEnd() const;
		int  GetSelStart() const;
		int  GetThumbLength() const;
		CRect GetThumbRect() const;
		int  GetTic(int nTic ) const;
		int  GetTicPos(int nTic) const;
		HWND GetToolTips() const;
		HWND SetBuddy(HWND hBuddy, BOOL fLocation = TRUE ) const;
		int  SetLineSize(int nSize) const;
		int  SetPageSize(int nSize) const;
		void SetPos(int nPos, BOOL Redraw = FALSE) const;
		void SetRangeMax(int nMax, BOOL Redraw = FALSE) const;
		void SetRangeMin(int nMin, BOOL Redraw = FALSE) const;
		void SetSelection(int nMin, int nMax, BOOL Redraw = FALSE) const;
		BOOL SetTic(int nTic) const;
		void SetTicFreq(int nFreq)  const;
		int  SetTipSide(int nLocation) const;
		void SetToolTips(HWND pToolTip) const;

	protected:
		// Overridables
		virtual void PreRegisterClass(WNDCLASS& wc) { wc.lpszClassName = TRACKBAR_CLASS; }

	private:
		CSlider(const CSlider&);				// Disable copy construction
		CSlider& operator = (const CSlider&);	// Disable assignment operator
	};


	// Also known as an Up/Down control
	class CSpinButton : public CWnd
	{
	public:
		CSpinButton() {}
		virtual ~CSpinButton() {}

		int  GetAccel(int cAccels, LPUDACCEL paAccels) const;
		int  GetBase() const;
		HWND GetBuddy() const;
		int  GetPos() const;
		DWORD GetRange() const;
		BOOL SetAccel(int cAccels, LPUDACCEL paAccels) const;
		int  SetBase(int nBase) const;
		HWND SetBuddy(HWND hwndBuddy) const;
		int  SetPos(int nPos) const;
		void SetRange(int nLower, int nUpper) const;

	protected:
		// Overridables
		virtual void PreCreate(CREATESTRUCT& cs);
		virtual void PreRegisterClass(WNDCLASS& wc);

	private:
		CSpinButton(const CSpinButton&);				// Disable copy construction
		CSpinButton& operator = (const CSpinButton&);	// Disable assignment operator
	};

	class CToolTip : public CWnd
	{
	public:
		CToolTip();
		virtual ~CToolTip();

		//Attributes
		int		 GetDelayTime(DWORD dwDuration) const;
		CRect	 GetMargin() const;
		int		 GetMaxTipWidth() const;
		CString	 GetText(HWND hWndControl, UINT uID = -1) const;
		COLORREF GetTipBkColor() const;
		COLORREF GetTipTextColor() const;
		int		 GetToolCount() const;
		TOOLINFO GetToolInfo(HWND hWndControl, UINT uID = -1) const;
		void	 SetDelayTime(UINT nDelay) const;
		void	 SetDelayTime(DWORD dwDuration, int iTime) const;
		void	 SetMargin(const RECT& rc) const;
		int		 SetMaxTipWidth(int iWidth) const;
		void	 SetTipBkColor(COLORREF clr) const;
		void	 SetTipTextColor(COLORREF clr) const;
		void	 SetToolInfo(const TOOLINFO& ToolInfo) const;
#if (_WIN32_IE >=0x0500)
		CSize	 GetBubbleSize(HWND hWndControl, UINT uID = -1) const;
#endif

		//Operations
		void Activate(BOOL Activate) const;
		BOOL AddTool(HWND hWndControl, const RECT& rcTool, UINT uID, UINT nIDText) const;
		BOOL AddTool(HWND hWndControl, UINT nIDText) const;
		BOOL AddTool(HWND hWndControl, const RECT& rcTool, UINT uID, LPCTSTR lpszText = LPSTR_TEXTCALLBACK) const;
		BOOL AddTool(HWND hWndControl, LPCTSTR lpszText = LPSTR_TEXTCALLBACK) const;
		void DelTool(HWND hWndControl, UINT uID = -1) const;
		BOOL HitTest(HWND hWnd, CPoint pt, const TOOLINFO& ToolInfo) const;
		void Pop() const;
		void RelayEvent(MSG& Msg) const;
		void SetToolRect(const RECT& rc, HWND hWndControl, UINT uID = -1) const;
		void Update() const;
		void UpdateTipText(LPCTSTR lpszText, HWND hWndControl, UINT uID = -1) const;
		void UpdateTipText(UINT nIDText, HWND hWndControl, UINT uID = -1) const;

#if (_WIN32_IE >=0x0500)
		BOOL AdjustRect(RECT& rc, BOOL IsLarger = TRUE) const;
  #ifdef TTM_SETTITLE
		BOOL SetTitle(UINT uIcon, LPCTSTR lpstrTitle) const;
  #endif
#endif
#if (WINVER >= 0x0501) && defined(TTM_SETWINDOWTHEME)
		void SetTTWindowTheme(LPCWSTR lpstrTheme) const;
#endif

	protected:
		// Overridables
		virtual void FillToolInfo(TOOLINFO& ti, HWND hControl) const;
		virtual void FillToolInfo(TOOLINFO& ti, HWND hControl, const RECT& rc, UINT uID) const;
		virtual void PreCreate(CREATESTRUCT& cs);
		virtual void PreRegisterClass(WNDCLASS& wc);
	private:
		CToolTip(const CToolTip&);				// Disable copy construction
		CToolTip& operator = (const CToolTip&);	// Disable assignment operator

	};


} // namespace Win32xx

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

namespace Win32xx
{

	////////////////////////////////////////
	// Definitions for the CAnimation class
	//
	inline BOOL CAnimation::Close() const
	// Closes an AVI clip.
	{
		assert(IsWindow());
		return Animate_Close(*this);
	}

	inline BOOL CAnimation::Open(LPTSTR lpszName) const
	// Opens an AVI clip and displays its first frame in an animation control.
	{
		assert(IsWindow());
		return Animate_Open(*this, lpszName);
	}

	inline BOOL CAnimation::Play(int nFrom, int nTo, int nRepeat) const
	// Plays an AVI clip in an animation control. The control plays the clip
	//	in the background while the thread continues executing.
	//	 nFrom : Zero - based index of the frame where playing begins. The value must be less than 65,536.
	//	 nTo:    Zero - based index of the frame where playing ends. The value must be less than 65,536.
	//           A value of - 1 means end with the last frame in the AVI clip.
	//   nRepeat: Number of times to replay the AVI clip.A value of - 1 means replay the clip indefinitely.
	{
		assert(IsWindow());
		return Animate_Play(*this, nFrom, nTo, nRepeat);
	}

	inline BOOL CAnimation::Seek(int nFrame) const
	// Directs an animation control to display a particular frame of an AVI clip.
	// The control displays the clip in the background while the thread continues executing.
	{
		assert(IsWindow());
		return Animate_Seek(*this, nFrame);
	}

	inline BOOL CAnimation::Stop() const
	// Stops playing an AVI clip in an animation control.
	{
		assert(IsWindow());
		return Animate_Stop(*this);
	}


	////////////////////////////////////////
	// Definitions for the CComboBox class
	//
	inline int  CComboBox::AddString(LPCTSTR lpszString) const
	// Adds a string to the list box of a combo box. If the combo box does not
	// have the CBS_SORT style, the string is added to the end of the list.
	// Otherwise, the string is inserted into the list, and the list is sorted.
	{
		assert(IsWindow());
		return static_cast<int>(SendMessage(CB_ADDSTRING, 0L, (LPARAM)lpszString));
	}

	inline void CComboBox::Clear() const
	// Deletes the current selection, if any, from the combo box's edit control.
	{
		assert(IsWindow());
		SendMessage(WM_CLEAR, 0L, 0L);
	}

	inline void CComboBox::Copy() const
	// Copies the current selection to the clipboard in CF_TEXT format.
	{
		assert(IsWindow());
		SendMessage(WM_COPY, 0L, 0L);
	}

	inline void CComboBox::Cut() const
	// Deletes the current selection, if any, in the edit control and copies
	// the deleted text to the clipboard in CF_TEXT format.
	{
		assert(IsWindow());
		SendMessage(WM_CUT, 0L, 0L);
	}

	inline int  CComboBox::DeleteString(int nIndex) const
	// Deletes a string in the list box of a combo box.
	{
		assert(IsWindow());
		return static_cast<int>(SendMessage(CB_DELETESTRING, (WPARAM)nIndex, 0L));
	}

	inline int  CComboBox::Dir(UINT attr, LPCTSTR lpszWildCard ) const
	// Adds the names of directories and files that match a specified string
	// and set of file attributes.
	{
		assert(IsWindow());
		return static_cast<int>(SendMessage(CB_DIR, (WPARAM)attr, (LPARAM)lpszWildCard));
	}

	inline int  CComboBox::FindString(int nIndexStart, LPCTSTR lpszString) const
	// Search the list box of a combo box for an item beginning with the
	// characters in a specified string.
	{
		assert(IsWindow());
		return static_cast<int>(SendMessage(CB_FINDSTRING, (WPARAM)nIndexStart, (LPARAM)lpszString));
	}

	inline int  CComboBox::FindStringExact(int nIndexStart, LPCTSTR lpszString) const
	// Find the first list box string in a combo box that matches the string specified in lpszString.
	{
		assert(IsWindow());
		return static_cast<int>(SendMessage(CB_FINDSTRINGEXACT, (WPARAM)nIndexStart, (LPARAM)lpszString));
	}

	inline int  CComboBox::GetCount() const
	// Retrieves the number of items in the list box of the combo box.
	{
		assert(IsWindow());
		return static_cast<int>(SendMessage(CB_GETCOUNT, 0L, 0L));
	}

	inline int  CComboBox::GetCurSel() const
	// Retrieves the index of the currently selected item, if any, in the list box of the combo box.
	{
		assert(IsWindow());
		return static_cast<int>(SendMessage(CB_GETCURSEL, 0L, 0L));
	}

	inline CRect CComboBox::GetDroppedControlRect() const
	// Retrieves the screen coordinates of the combo box in its dropped-down state.
	{
		assert(IsWindow());
		CRect rc;
		SendMessage(CB_GETDROPPEDCONTROLRECT, 0L, (LPARAM)&rc);
		return rc;
	}

	inline BOOL CComboBox::GetDroppedState() const
	// Determines whether the list box of the combo box is dropped down.
	{
		assert(IsWindow());
		return static_cast<BOOL>(SendMessage(CB_GETDROPPEDSTATE, 0L, 0L));
	}

	inline int  CComboBox::GetDroppedWidth() const
	// Retrieves the minimum allowable width, in pixels, of the list box of the combo box
	// with the CBS_DROPDOWN or CBS_DROPDOWNLIST style.
	{
		assert(IsWindow());
		return static_cast<int>(SendMessage(CB_GETDROPPEDWIDTH, 0L, 0L));
	}

	inline DWORD CComboBox::GetEditSel() const
	// Gets the starting and ending character positions of the current selection
	// in the edit control of the combo box.
	{
		assert(IsWindow());
		return static_cast<int>(SendMessage(CB_GETEDITSEL, 0L, 0L));
	}

	inline BOOL CComboBox::GetExtendedUI() const
	// Determines whether the combo box has the default user interface or the extended user interface.
	{
		assert(IsWindow());
		return static_cast<BOOL>(SendMessage(CB_GETEXTENDEDUI, 0L, 0L));
	}

	inline int  CComboBox::GetHorizontalExtent() const
	// Retrieve from the combo box the width, in pixels, by which the list box can
	// be scrolled horizontally (the scrollable width).
	{
		assert(IsWindow());
		return static_cast<int>(SendMessage(CB_GETHORIZONTALEXTENT, 0L, 0L));
	}

	inline DWORD CComboBox::GetItemData(int nIndex) const
	// Retrieves the application-supplied value associated with the specified item in the combo box.
	{
		assert(IsWindow());
		return static_cast<DWORD>(SendMessage(CB_GETITEMDATA, (WPARAM)nIndex, 0L));
	}

	inline int  CComboBox::GetItemHeight(int nIndex) const
	// Determines the height of list items or the selection field in the combo box.
	{
		assert(IsWindow());
		return static_cast<int>(SendMessage(CB_GETITEMHEIGHT, (WPARAM)nIndex, 0L));
	}

	inline int  CComboBox::GetLBText(int nIndex, LPTSTR lpszText) const
	//  Retrieves a string from the list of the combo box.
	{
		assert(IsWindow());
		return static_cast<int>(SendMessage(CB_GETLBTEXT, (WPARAM)nIndex, (LPARAM)lpszText));
	}

	inline int  CComboBox::GetLBTextLen(int nIndex) const
	// Retrieves the length, in characters, of a string in the list of the combo box.
	{
		assert(IsWindow());
		return static_cast<int>(SendMessage(CB_GETLBTEXTLEN, (WPARAM)nIndex, 0L));
	}

	inline LCID CComboBox::GetLocale() const
	// Retrieves the current locale of the combo box.
	{
		assert(IsWindow());
		return static_cast<LCID>(SendMessage(CB_GETLOCALE, 0L, 0L));
	}

	inline int  CComboBox::GetTopIndex() const
	// Retrieves the zero-based index of the first visible item in the list box portion of the combo box.
	{
		assert(IsWindow());
		return static_cast<int>(SendMessage(CB_GETTOPINDEX, 0L, 0L));
	}

	inline int  CComboBox::InitStorage(int nItems, int nBytes) const
	// Allocates memory for storing list box items. Use this before adding a
	// large number of items to the list box portion of a combo box.
	{
		assert(IsWindow());
		return static_cast<int>(SendMessage(CB_INITSTORAGE, (WPARAM)nItems, (LPARAM)nBytes));
	}

	inline int  CComboBox::InsertString(int nIndex, LPCTSTR lpszString) const
	// Inserts a string into the list box of the combo box. Unlike the AddString,
	// a list with the CBS_SORT style is not sorted.
	{
		assert(IsWindow());
		return static_cast<int>(SendMessage(CB_INSERTSTRING, (WPARAM)nIndex, (LPARAM)lpszString));
	}

	inline void CComboBox::Paste() const
	// Copies the current content of the clipboard to the combo box's edit control at the current caret position.
	{
		assert(IsWindow());
		SendMessage(WM_PASTE, 0L, 0L);
	}

	inline void CComboBox::LimitText(int nMaxChars) const
	// Limits the length of the text the user may type into the edit control of the combo box.
	{
		assert(IsWindow());
		SendMessage(CB_LIMITTEXT, (WPARAM)nMaxChars, 0L);
	}

	inline void CComboBox::ResetContent() const
	// Removes all items from the list box and edit control of the combo box.
	{
		assert(IsWindow());
		SendMessage(CB_RESETCONTENT, 0L, 0L);
	}

	inline int  CComboBox::SelectString(int nStartAfter, LPCTSTR lpszString) const
	// Searches the list of a combo box for an item that begins with the characters in a
	// specified string. If a matching item is found, it is selected and copied to the edit control.
	{
		assert(IsWindow());
		return static_cast<int>(SendMessage(CB_SELECTSTRING, (WPARAM)nStartAfter, (LPARAM)lpszString));
	}

	inline int  CComboBox::SetCurSel(int nIndex) const
	// Selects a string in the list of the combo box. If necessary, the list scrolls the string into view.
	{
		assert(IsWindow());
		return static_cast<int>(SendMessage(CB_SETCURSEL, (WPARAM)nIndex, 0L));
	}

	inline int  CComboBox::SetDroppedWidth(int nWidth) const
	// Sets the maximum allowable width, in pixels, of the list box of the combo box with
	// the CBS_DROPDOWN or CBS_DROPDOWNLIST style.
	{
		assert(IsWindow());
		return static_cast<int>(SendMessage(CB_SETDROPPEDWIDTH, (WPARAM)nWidth, 0L));
	}

	inline BOOL CComboBox::SetEditSel(int nStartChar, int nEndChar) const
	// Selects characters in the edit control of the combo box.
	{
		assert(IsWindow());
		return static_cast<BOOL>(SendMessage(CB_SETEDITSEL, 0L, (LPARAM)MAKELONG(nStartChar,nEndChar)));
	}

	inline int  CComboBox::SetExtendedUI(BOOL IsExtended) const
	// Selects either the default user interface or the extended user interface for the combo box that
	// has the CBS_DROPDOWN or CBS_DROPDOWNLIST style.
	{
		assert(IsWindow());
		return static_cast<int>(SendMessage(CB_SETEXTENDEDUI, (WPARAM)IsExtended, 0L));
	}

	inline void CComboBox::SetHorizontalExtent(UINT nExtent ) const
	// Sets the width, in pixels, by which the list box can be scrolled horizontally (the scrollable width).
	{
		assert(IsWindow());
		SendMessage(CB_SETHORIZONTALEXTENT, (WPARAM)nExtent, 0L);
	}

	inline int  CComboBox::SetItemData(int nIndex, DWORD dwItemData) const
	// Sets the value associated with the specified item in the combo box.
	{
		assert(IsWindow());
		return static_cast<int>(SendMessage(CB_SETITEMDATA, (WPARAM)nIndex, (LPARAM)dwItemData));
	}

	inline int  CComboBox::SetItemHeight(int nIndex, UINT cyItemHeight) const
	// Sets the height of list items or the selection field in the combo box.
	{
		assert(IsWindow());
		return static_cast<int>(SendMessage(CB_SETITEMHEIGHT, (WPARAM)nIndex, (LPARAM)cyItemHeight));
	}

	inline LCID CComboBox::SetLocale( LCID NewLocale ) const
	// Sets the current locale of the combo box.
	{
		assert(IsWindow());
		return static_cast<LCID>(SendMessage(CB_SETLOCALE, (WPARAM)NewLocale, 0L));
	}

	inline int  CComboBox::SetTopIndex(int nIndex) const
	// Ensure that a particular item is visible in the list box of the combo box.
	{
		assert(IsWindow());
		return static_cast<int>(SendMessage(CB_SETTOPINDEX, (WPARAM)nIndex, 0L));
	}

	inline void CComboBox::ShowDropDown(BOOL Show) const
	// Shows or hides the list box of the combo box that has the CBS_DROPDOWN or CBS_DROPDOWNLIST style.
	{
		assert(IsWindow());
		SendMessage(CB_SHOWDROPDOWN, (WPARAM)Show, 0L);
	}


	////////////////////////////////////////
	// Definitions for the CComboBoxEx class
	//
	inline int  CComboBoxEx::DeleteItem(int nIndex ) const
	// Removes an item from the ComboBoxEx control.
	{
		assert(IsWindow());
		return static_cast<int>(SendMessage(CBEM_DELETEITEM, (WPARAM)nIndex, 0L));
	}

	inline HWND CComboBoxEx::GetComboBoxCtrl() const
	// Retrieves the handle to the child combo box control.
	{
		assert(IsWindow());
		return reinterpret_cast<HWND>(SendMessage(CBEM_GETCOMBOCONTROL, 0L, 0L));
	}

	inline HWND CComboBoxEx::GetEditCtrl() const
	// Retrieves the handle to the edit control portion of the ComboBoxEx control.
	{
		assert(IsWindow());
		return reinterpret_cast<HWND>(SendMessage(CBEM_GETEDITCONTROL, 0L, 0L));
	}

	inline DWORD CComboBoxEx::GetExtendedStyle() const
	// Retrieves the extended styles that are in use for the ComboBoxEx control.
	{
		assert(IsWindow());
		return static_cast<DWORD>(SendMessage(CBEM_GETEXTENDEDSTYLE, 0L, 0L));
	}

	inline CImageList CComboBoxEx::GetImageList() const
	// Retrieves the handle to an image list assigned to the ComboBoxEx control.
	{
		assert(IsWindow());
		HIMAGELIST himl = reinterpret_cast<HIMAGELIST>(SendMessage(CBEM_GETIMAGELIST, 0L, 0L));
		return CImageList(himl);
	}

	inline BOOL CComboBoxEx::GetItem(COMBOBOXEXITEM& CBItem) const
	// Retrieves item information for the given ComboBoxEx item.
	{
		assert(IsWindow());
		return static_cast<BOOL>(SendMessage(CBEM_GETITEM, 0L, (LPARAM)&CBItem));
	}

	inline BOOL CComboBoxEx::HasEditChanged () const
	// Determines whether or not the user has changed the text of the ComboBoxEx edit control.
	{
		assert(IsWindow());
		return static_cast<BOOL>(SendMessage(CBEM_HASEDITCHANGED, 0L, 0L));
	}

	inline int CComboBoxEx::InsertItem(const COMBOBOXEXITEM& CBItem) const
	// Inserts a new item in the ComboBoxEx control.
	{
		assert(IsWindow());
		return static_cast<int>(SendMessage(CBEM_INSERTITEM, 0L, (LPARAM)&CBItem));
	}

	inline DWORD CComboBoxEx::SetExtendedStyle(DWORD dwExMask, DWORD dwExStyles ) const
	// Sets extended styles within the ComboBoxEx control.
	{
		assert(IsWindow());
		return static_cast<DWORD>(SendMessage(CBEM_SETEXTENDEDSTYLE, (WPARAM)dwExMask, (LPARAM)dwExStyles));
	}

	inline CImageList CComboBoxEx::SetImageList(HIMAGELIST himlNew) const
	// Sets an image list for the ComboBoxEx control.
	{
		assert(IsWindow());
		HIMAGELIST himl = reinterpret_cast<HIMAGELIST>(SendMessage(CBEM_SETIMAGELIST, 0L, (LPARAM)himlNew));
		return CImageList(himl);
	}

	inline BOOL CComboBoxEx::SetItem(const COMBOBOXEXITEM& CBItem) const
	// Sets the attributes for an item in the ComboBoxEx control.
	{
		assert(IsWindow());
		return static_cast<BOOL>(SendMessage(CBEM_SETITEM, 0L, (LPARAM)&CBItem));
	}

	////////////////////////////////////////
	// Definitions for the CDateTime class
	//
	inline CDateTime::CDateTime()
	{
	}

	inline COLORREF CDateTime::GetMonthCalColor(int iColor) const
	// Retrieves the color for a given portion of the month calendar within the date and time picker (DTP) control.
	{
		assert(IsWindow());
		return static_cast<COLORREF>(DateTime_GetMonthCalColor(*this, iColor));
	}

	inline COLORREF CDateTime::SetMonthCalColor(int iColor, COLORREF clr) const
	// Sets the color for a given portion of the month calendar within the date and time picker (DTP) control.
	{
		assert(IsWindow());
		return static_cast<COLORREF>(DateTime_SetMonthCalColor(*this, iColor, clr));
	}

	inline BOOL CDateTime::SetFormat(LPCTSTR pszFormat) const
	// Sets the display of the date and time picker (DTP) control based on a given format string.
	{
		assert(IsWindow());
		return DateTime_SetFormat(*this, pszFormat);
	}

	inline HWND CDateTime::GetMonthCalCtrl() const
	// Retrieves the handle to the date and time picker's (DTP) child month calendar control.
	{
		assert(IsWindow());
		return reinterpret_cast<HWND>(DateTime_GetMonthCal(*this));
	}

	inline CFont CDateTime::GetMonthCalFont() const
	// Retrieves the font that the date and time picker (DTP) control's child month calendar control is currently using.
	{
		assert(IsWindow());
		HFONT hFont = reinterpret_cast<HFONT>(DateTime_GetMonthCalFont(*this));
		return CFont(hFont);
	}

	inline void CDateTime::SetMonthCalFont(HFONT hFont, BOOL Redraw /*= TRUE*/) const
	// Sets the font to be used by the date and time picker (DTP) control's child month calendar control.
	{
		assert(IsWindow());
		DateTime_SetMonthCalFont(*this, hFont, MAKELONG(Redraw, 0));
	}

	inline DWORD CDateTime::GetRange(SYSTEMTIME& MinRange, SYSTEMTIME& MaxRange) const
	// Retrieves the current minimum and maximum allowable system times for the date and time picker (DTP) control.
	{
		assert(IsWindow());

		SYSTEMTIME Ranges[2];
		ZeroMemory(Ranges, 2 * sizeof(SYSTEMTIME));
		DWORD Result = DateTime_GetRange(*this, Ranges);
		MinRange = Ranges[0];
		MaxRange = Ranges[1];
		return Result;
	}

	inline BOOL CDateTime::SetRange(const SYSTEMTIME& MinRange, const SYSTEMTIME& MaxRange) const
	// Sets the minimum and maximum allowable system times for the date and time picker (DTP) control.
	{
		assert(IsWindow());
		SYSTEMTIME Ranges[2];
		Ranges[0] = MinRange;
		Ranges[1] = MaxRange;
		DWORD flags = GDTR_MIN | GDTR_MAX;

		return DateTime_SetRange(*this, flags, Ranges);
	}

	inline SYSTEMTIME CDateTime::GetTime(DWORD* pReturnCode) const
	// Returns the currently selected time from a date and time picker (DTP) control
	// Possible return codes are: GDT_VALID, GDT_ERROR, and GDT_NONE.
    // A return code of GDT_NONE occurs if the control was set to the DTS_SHOWNONE
	//	style and the control check box was not selected.
	{
		assert(IsWindow());
		SYSTEMTIME SysTime;
		ZeroMemory(&SysTime,  sizeof(SYSTEMTIME));
		DWORD Res = DateTime_GetSystemtime(*this, &SysTime);
		if (pReturnCode)
			*pReturnCode = Res;

		return SysTime;
	}

	inline BOOL CDateTime::SetTime(const SYSTEMTIME& TimeNew) const
	// Sets the date and time picker (DTP) control to a given date and time.
	{
		assert(IsWindow());
		return DateTime_SetSystemtime(*this, GDT_VALID, &TimeNew);
	}

	inline BOOL CDateTime::SetTimeNone() const
	// Set the DTP control to "no date" and clear its check box.
	// Only applies to DTP controls that are set to the DTS_SHOWNONE style.
	{
		assert(IsWindow());
		return DateTime_SetSystemtime(*this, GDT_NONE, NULL);
	}


	////////////////////////////////////////
	// Definitions for the CHotKey class
	//
	inline DWORD CHotKey::GetHotKey() const
	{
		assert(IsWindow());
		return static_cast<DWORD>(SendMessage(HKM_GETHOTKEY, 0L, 0L));
	}

	inline CString CHotKey::GetKeyName(UINT vk, BOOL IsExtended) const
	{
		// Translate the virtual-key code to a scan code
		LONG lScan = MapVirtualKey(vk, 0);

		// Construct an LPARAM with the scan code in Bits 16-23, and an extended flag in bit 24
		LPARAM lParam = lScan << 16;
		if (IsExtended)
			lParam |= 0x01000000L;

		CString str;
		int nBufferLen = 64;
		int nStrLen = nBufferLen;

		// Loop until we have retrieved the entire string
		while(nStrLen == nBufferLen)
		{
			nBufferLen *= 4;
			LPTSTR pszStr = str.GetBuffer(nBufferLen);
			nStrLen = ::GetKeyNameText((LONG)lParam, pszStr, nBufferLen + 1);
			str.ReleaseBuffer();
		}

		return str;
	}

	inline void CHotKey::SetHotKey(DWORD dwKey) const
	{
		assert(IsWindow());
		SendMessage(HKM_SETHOTKEY, (WPARAM)dwKey, 0L);
	}

	inline void CHotKey::SetRules(WORD wInvalidComb, WORD wModifiers) const
	{
		assert(IsWindow());
		SendMessage(HKM_SETRULES, wInvalidComb, wModifiers);
	}


	////////////////////////////////////////
	// Definitions for the CHeader class
	//
	inline CImageList CHeader::CreateDragImage(int nIndex) const
	{
		assert(IsWindow());
		HIMAGELIST himl = Header_CreateDragImage(*this, nIndex);
		return CImageList(himl);
	}

	inline BOOL CHeader::DeleteItem(int nPos) const
	{
		assert(IsWindow());
		return Header_DeleteItem(*this, nPos);
	}

	inline CImageList CHeader::GetImageList() const
	{
		assert(IsWindow());
		HIMAGELIST himl = Header_GetImageList(*this);
		return CImageList(himl);
	}

	inline BOOL CHeader::GetItem(int nPos, HDITEM& HeaderItem) const
	{
		assert(IsWindow());
		return Header_GetItem(*this, nPos, &HeaderItem);
	}

	inline int CHeader::GetItemCount() const
	{
		assert(IsWindow());
		return Header_GetItemCount(*this);
	}

	inline CRect CHeader::GetItemRect(int nIndex) const
	{
		assert(IsWindow());
		CRect rc;
		SendMessage(HDM_GETITEMRECT, (WPARAM)nIndex, (WPARAM)&rc);
		return rc;
	}

	inline BOOL CHeader::GetOrderArray(LPINT piArray, int iCount) const
	{
		assert(IsWindow());
		return Header_GetOrderArray(*this, piArray, iCount);
	}

	inline int CHeader::InsertItem(int nPos, const HDITEM& hdi) const
	{
		assert(IsWindow());
		return Header_InsertItem(*this, nPos, &hdi);
	}

	inline BOOL CHeader::Layout(HDLAYOUT* pHeaderLayout) const
	{
		assert(IsWindow());
		return Header_Layout(*this, pHeaderLayout);
	}

	inline int CHeader::OrderToIndex(int nOrder) const
	{
		assert(IsWindow());
		return Header_OrderToIndex( *this, nOrder);
	}

#ifdef Header_SetHotDivider
	inline int CHeader::SetHotDivider(CPoint pt) const
	{
		assert(IsWindow());
		return Header_SetHotDivider(*this, TRUE, MAKELPARAM(pt.x, pt.y));
	}

	inline int CHeader::SetHotDivider(int nIndex) const
	{
		assert(IsWindow());
		return Header_SetHotDivider(*this, FALSE, nIndex);
	}
#endif

	inline CImageList CHeader::SetImageList(HIMAGELIST himlNew) const
	{
		assert(IsWindow());
		HIMAGELIST himl = Header_SetImageList(*this, himlNew);
		return CImageList(himl);
	}

	inline BOOL CHeader::SetItem(int nPos, const HDITEM& HeaderItem) const
	{
		assert(IsWindow());
		return Header_SetItem(*this, nPos, &HeaderItem);
	}

	inline BOOL CHeader::SetOrderArray(int iCount, LPINT piArray) const
	{
		assert(IsWindow());
		return Header_SetOrderArray(*this, iCount, piArray);
	}

#ifdef Header_ClearFilter
	inline int CHeader::ClearFilter(int nColumn) const
	{
		assert(IsWindow());
		return Header_ClearFilter(*this, nColumn);
	}

	inline int CHeader::ClearAllFilters() const
	{
		assert(IsWindow());
		return Header_ClearAllFilters(*this);
	}

	inline int CHeader::EditFilter(int nColumn, BOOL DiscardChanges) const
	{
		assert(IsWindow());
		return Header_EditFilter(*this, nColumn, LPARAM MAKELPARAM(DiscardChanges, 0));
	}

	inline int CHeader::GetBitmapMargin() const
	{
		assert(IsWindow());
		return Header_GetBitmapMargin(*this);
	}

	inline int CHeader::SetBitmapMargin(int nWidth) const
	{
		assert(IsWindow());
		return Header_SetBitmapMargin(*this, nWidth);
	}

	inline int CHeader::SetFilterChangeTimeout(DWORD dwTimeOut) const
	{
		assert(IsWindow());
		return Header_SetFilterChangeTimeout(*this, dwTimeOut);
	}
#endif


	////////////////////////////////////////
	// Definitions for the CIPAddress class
	//
	inline CIPAddress::CIPAddress()
	{
		if (GetComCtlVersion() > 470)
		{
			// Call InitCommonControlsEx
			INITCOMMONCONTROLSEX InitStruct;
			InitStruct.dwSize = sizeof(INITCOMMONCONTROLSEX);
			InitStruct.dwICC = ICC_INTERNET_CLASSES;
			InitCommonControlsEx(&InitStruct);
		}
		else
			throw CNotSupportedException(_T("IP Address Control not supported!"));
	}

	inline void CIPAddress::ClearAddress() const
	{
		assert(IsWindow());
		SendMessage(IPM_CLEARADDRESS, 0L, 0L);
	}

	inline int CIPAddress::GetAddress(BYTE& nField0, BYTE& nField1, BYTE& nField2, BYTE& nField3) const
	{
		DWORD dwAddr;
		int iValue = GetAddress(dwAddr);
		nField0 = (BYTE)FIRST_IPADDRESS(dwAddr);
		nField1 = (BYTE)SECOND_IPADDRESS(dwAddr);
		nField2 = (BYTE)THIRD_IPADDRESS(dwAddr);
		nField3 = (BYTE)FOURTH_IPADDRESS(dwAddr);
		return iValue;
	}

	inline int CIPAddress::GetAddress(DWORD& dwAddress) const
	{
		assert(IsWindow());
		return static_cast<int>(SendMessage(IPM_GETADDRESS, 0L, (LPARAM)&dwAddress));
	}

	inline BOOL CIPAddress::IsBlank() const
	{
		assert(IsWindow());
		return static_cast<BOOL>(SendMessage(IPM_ISBLANK, 0L, 0L));
	}

	inline void CIPAddress::SetAddress(BYTE nField0, BYTE nField1, BYTE nField2, BYTE nField3) const
	{
		assert(IsWindow());
		SendMessage(IPM_SETADDRESS, 0L, (LPARAM)MAKEIPADDRESS(nField0, nField1, nField2, nField3));
	}

	inline void CIPAddress::SetAddress(DWORD dwAddress) const
	{
		assert(IsWindow());
		SendMessage(IPM_SETADDRESS, 0L, (LPARAM)dwAddress);
	}

	inline void CIPAddress::SetFieldFocus(WORD nField) const
	{
		assert(IsWindow());
		SendMessage(IPM_SETFOCUS, (WPARAM)nField, 0L);
	}

	inline void CIPAddress::SetFieldRange(int nField, BYTE nLower, BYTE nUpper) const
	{
		assert(IsWindow());
		SendMessage(IPM_SETRANGE, MAKEIPRANGE(nLower, nUpper), (LPARAM)nField);
	}


	///////////////////////////////////////////
	// Definitions for the CMonthCalendar class
	//
	inline CMonthCalendar::CMonthCalendar()
	{
	}

	inline COLORREF CMonthCalendar::GetColor(int nRegion) const
	// Retrieves the color for a given portion of the month calendar control.
	{
		assert(IsWindow());
		return static_cast<COLORREF>(MonthCal_GetColor(*this, nRegion));
	}

	inline SYSTEMTIME CMonthCalendar::GetCurSel() const
	// Retrieves the currently selected date.
	{
		assert(IsWindow());
		SYSTEMTIME st;
		ZeroMemory(&st, sizeof(st));
		MonthCal_GetCurSel(*this, &st);
		return st;
	}

	inline int CMonthCalendar::GetFirstDayOfWeek(BOOL* pbLocal /*= NULL*/) const
	// Retrieves the first day of the week.
	{
		assert(IsWindow());
		DWORD dwValue = MonthCal_GetFirstDayOfWeek(*this);

		if (pbLocal)
			*pbLocal = HIWORD(dwValue);

		return LOWORD(dwValue);
	}

	inline int CMonthCalendar::GetMaxSelCount() const
	// Retrieves the maximum date range that can be selected.
	{
		assert(IsWindow());
		return MonthCal_GetMaxSelCount(*this);
	}

	inline CRect CMonthCalendar::GetMinReqRect() const
	// Retrieves the minimum size required to display a full month
	{
		assert(IsWindow());
		CRect rc;
		MonthCal_GetMinReqRect(*this, &rc);
		return rc;
	}

	inline int CMonthCalendar::GetMonthDelta() const
	// Retrieves the scroll rate for the month calendar control.
	{
		assert(IsWindow());
		return MonthCal_GetMonthDelta(*this);
	}

	inline int CMonthCalendar::GetMonthRange(SYSTEMTIME& MinRange, SYSTEMTIME& MaxRange, DWORD dwFlags) const
	// Retrieves date information that represents the high and low limits of the month calendar control's display.
	{
		assert(IsWindow());
		SYSTEMTIME MinMax[2];
		ZeroMemory(MinMax, 2*sizeof(SYSTEMTIME));
		int nCount = MonthCal_GetMonthRange(*this, dwFlags, MinMax);
		MinRange = MinMax[0];
		MaxRange = MinMax[1];
		return nCount;
	}

	inline LRESULT CMonthCalendar::GetRange(SYSTEMTIME& MinRange, SYSTEMTIME& MaxRange) const
	//	Retrieves the minimum and maximum allowable dates set for the month calendar control.
	{
		assert(IsWindow());
		SYSTEMTIME MinMax[2];
		ZeroMemory(MinMax, 2*sizeof(SYSTEMTIME));
		LRESULT Value = MonthCal_GetRange(*this, &MinMax);
		MinRange = MinMax[0];
		MaxRange = MinMax[1];
		return Value;
	}

	inline LRESULT CMonthCalendar::GetSelRange(SYSTEMTIME& MinRange, SYSTEMTIME& MaxRange) const
	// Retrieves date information that represents the upper and lower limits of the date range currently selected by the user.
	{
		assert(IsWindow());
		SYSTEMTIME MinMax[2];
		ZeroMemory(MinMax, 2*sizeof(SYSTEMTIME));
		LRESULT Value = MonthCal_GetSelRange(*this, &MinMax);
		MinRange = MinMax[0];
		MaxRange = MinMax[1];
		return Value;
	}

	inline SYSTEMTIME CMonthCalendar::GetToday() const
	// Retrieves the date information for the date specified as "today" for the month calendar control.
	{
		assert(IsWindow());
		SYSTEMTIME DateTime;
		ZeroMemory(&DateTime, sizeof(DateTime));
		VERIFY(MonthCal_GetToday(*this, &DateTime));
		return DateTime;
	}

	inline LRESULT CMonthCalendar::HitTest(MCHITTESTINFO& MCHitTest) const
	//	Determines which portion of the month calendar control is at a given point on the screen.
	{
		assert(IsWindow());
		return MonthCal_HitTest(*this, &MCHitTest);
	}

	inline COLORREF CMonthCalendar::SetColor(int nRegion, COLORREF clr) const
	// Sets the color for a given portion of the month calendar control.
	{
		assert(IsWindow());
		return static_cast<COLORREF>(MonthCal_SetColor(*this, nRegion, clr));
	}

	inline BOOL CMonthCalendar::SetCurSel(const SYSTEMTIME& DateTime) const
	// Sets the currently selected date for the month calendar control.
	{
		assert(IsWindow());
		return MonthCal_SetCurSel(*this, &DateTime);
	}

	inline BOOL CMonthCalendar::SetDayState(int nMonths, const MONTHDAYSTATE& State) const
	// Sets the day states for all months that are currently visible within the month calendar control.
	{
		assert(IsWindow());
		return static_cast<BOOL>(MonthCal_SetDayState(*this, nMonths, &State));
	}

	inline BOOL CMonthCalendar::SetFirstDayOfWeek(int iDay, int* pnOld/* = NULL*/) const
	// Sets the first day of the week for the month calendar control.
	{
		assert(IsWindow());
		DWORD dwValue = (DWORD)MonthCal_SetFirstDayOfWeek(*this, iDay);

		if(pnOld)
			*pnOld = LOWORD(dwValue);

		return static_cast<BOOL>(HIWORD(dwValue));
	}

	inline BOOL CMonthCalendar::SetMaxSelCount(int nMax) const
	// Sets the maximum number of days that can be selected in the month calendar control.
	{
		assert(IsWindow());
		return MonthCal_SetMaxSelCount(*this, nMax);
	}

	inline int CMonthCalendar::SetMonthDelta(int iDelta) const
	// Sets the scroll rate for the month calendar control.
	{
		assert(IsWindow());
		return MonthCal_SetMonthDelta(*this, iDelta);
	}

	inline BOOL CMonthCalendar::SetRange(const SYSTEMTIME& MinRange, const SYSTEMTIME& MaxRange) const
	// Sets the minimum and maximum allowable dates for the month calendar control.
	{
		SYSTEMTIME MinMax[2];
		DWORD dwLimit = GDTR_MIN | GDTR_MAX;

		MinMax[0] = MinRange;
		MinMax[1] = MaxRange;

		return static_cast<BOOL>(MonthCal_SetRange(*this, dwLimit, &MinMax));
	}

	inline BOOL CMonthCalendar::SetSelRange(const SYSTEMTIME& MinRange, const SYSTEMTIME& MaxRange) const
	// Sets the selection for the month calendar control to a given date range.
	{
		SYSTEMTIME MinMax[2];
		MinMax[0] = MinRange;
		MinMax[1] = MaxRange;
		return static_cast<BOOL>(MonthCal_SetSelRange(*this, &MinMax));
	}

	inline void CMonthCalendar::SetToday(const SYSTEMTIME& DateTime) const
	// Sets the "today" selection for the month calendar control.
	{
		assert(IsWindow());
		MonthCal_SetToday(*this, &DateTime);
	}


	////////////////////////////////////////
	// Definitions for the CProgressBar class
	//
	inline int CProgressBar::GetPos() const
	// Retrieves the current position of the progress bar.
	{
		assert(IsWindow());
		return static_cast<int>(SendMessage(PBM_GETPOS, 0L, 0L));
	}

	inline int CProgressBar::GetRange(BOOL WhichLimit, const PBRANGE& PBRange) const
	// Retrieves information about the current high and low limits of the progress bar control.
	{
		assert(IsWindow());
		return static_cast<int>(SendMessage(PBM_GETRANGE, (WPARAM)WhichLimit, (LPARAM) &PBRange));
	}

	inline int CProgressBar::GetRange(BOOL WhichLimit) const
	// Retrieves information about the current high and low limits of the progress bar control.
	{
		assert(IsWindow());
		return static_cast<int>(SendMessage(PBM_GETRANGE, (WPARAM)WhichLimit, (LPARAM) 0L));
	}

	inline int CProgressBar::OffsetPos(int nIncrement) const
	// Advances the current position of the progress bar by a specified increment and redraws
	// the bar to reflect the new position.
	{
		assert(IsWindow());
		return static_cast<int>(SendMessage(PBM_DELTAPOS, (WPARAM)nIncrement, 0L));
	}

	inline int CProgressBar::SetPos(int nNewPos) const
	// Sets the current position for the progress bar and redraws the bar to reflect the new position.
	{
		assert(IsWindow());
		return static_cast<int>(SendMessage(PBM_SETPOS, (WPARAM)nNewPos, 0L));
	}

	inline int CProgressBar::SetRange(short nMinRange, short nMaxRange) const
	// Sets the minimum and maximum values for the progress bar and redraws the bar to reflect the new range.
	{
		assert(IsWindow());
		return static_cast<int>(SendMessage(PBM_SETRANGE, 0L, (LPARAM) MAKELPARAM (nMinRange, nMaxRange)));
	}

	inline int CProgressBar::SetStep(int nStepInc) const
	// Specifies the step increment for the progress bar.
	{
		assert(IsWindow());
		return static_cast<int>(SendMessage(PBM_SETSTEP, (WPARAM)nStepInc, 0L));
	}

	inline int CProgressBar::StepIt() const
	// Advances the current position for a progress bar by the step increment and
	// redraws the bar to reflect the new position.
	{
		assert(IsWindow());
		return static_cast<int>(SendMessage(PBM_STEPIT, 0L, 0L));
	}


	////////////////////////////////////////
	// Definitions for the CScrollBar class
	//
	inline BOOL CScrollBar::EnableScrollBar( UINT nArrowFlags )  const
	// Enables or disables the scroll bar arrows.
	{
		assert(IsWindow());
		return ::EnableScrollBar(*this, SB_CTL, nArrowFlags);
	}

	inline BOOL CScrollBar::GetScrollInfo(SCROLLINFO& si)  const
	// Retrieves the parameters of a scroll bar, including the minimum and maximum
	// scrolling positions, the page size, and the position of the scroll box (thumb).
	{
		assert(IsWindow());
		return ::GetScrollInfo(*this, SB_CTL, &si);
	}

	inline int CScrollBar::GetScrollPos()  const
	// Retrieves the current position of the scroll box (thumb) in the scroll bar.
	{
		assert(IsWindow());
		return ::GetScrollPos(*this, SB_CTL);
	}

	inline BOOL CScrollBar::GetScrollRange(int& MinPos, int& MaxPos )  const
	// Retrieves the current minimum and maximum scroll box (thumb) positions for the scroll bar.
	{
		assert(IsWindow());
		return ::GetScrollRange(*this, SB_CTL, &MinPos, &MaxPos);
	}

	inline BOOL CScrollBar::SetScrollInfo(const SCROLLINFO& si, BOOL Redraw )  const
	// Sets the parameters of the scroll bar, including the minimum and maximum scrolling positions,
	// the page size, and the position of the scroll box (thumb).
	{
		assert(IsWindow());
		return ::SetScrollInfo(*this, SB_CTL, &si, Redraw);
	}

	inline int CScrollBar::SetScrollPos(int nPos, BOOL Redraw)  const
	// Sets the position of the scroll box (thumb) in the scroll bar and, if requested,
	// redraws the scroll bar to reflect the new position of the scroll box.
	{
		assert(IsWindow());
		return ::SetScrollPos(*this, SB_CTL, nPos, Redraw);
	}

	inline BOOL CScrollBar::SetScrollRange( int nMinPos, int nMaxPos, BOOL Redraw )  const
	// Sets the minimum and maximum scroll box positions for the scroll bar.
	{
		assert(IsWindow());
		return ::SetScrollRange(*this, SB_CTL, nMinPos, nMaxPos, Redraw);
	}

	inline BOOL CScrollBar::ShowScrollBar(BOOL Show)  const
	// Shows or hides the scroll bar.
	{
		assert(IsWindow());
		return ::ShowScrollBar(*this, SB_CTL, Show);
	}

	////////////////////////////////////////
	// Definitions for the CSlider class
	//
	inline void CSlider::ClearSel() const
	// Clears the current selection range in the trackbar.
	{
		assert(IsWindow());
		SendMessage(TBM_CLEARSEL, 0L, 0L);
	}

	inline void CSlider::ClearTics(BOOL Redraw) const
	// Removes the current tick marks from the trackbar.
	{
		assert(IsWindow());
		SendMessage(TBM_CLEARTICS, (WPARAM)Redraw, 0L);
	}

	inline HWND CSlider::GetBuddy(BOOL fLocation) const
	// Retrieves the handle to the trackbar control buddy window at a given location.
	{
		assert(IsWindow());
		return reinterpret_cast<HWND>(SendMessage(TBM_GETBUDDY, (WPARAM)fLocation, 0L));
	}

	inline CRect CSlider::GetChannelRect() const
	// Retrieves the size and position of the bounding rectangle for the trackbar's channel.
	{
		assert(IsWindow());
		CRect rc;
		SendMessage(TBM_GETCHANNELRECT, 0L, (LPARAM)&rc);
		return rc;
	}

	inline int  CSlider::GetLineSize() const
	// Retrieves the number of logical positions the trackbar's slider moves in response
	// to keyboard input from the arrow keys.
	{
		assert(IsWindow());
		return static_cast<int>(SendMessage(TBM_GETLINESIZE, 0L, 0L));
	}

	inline int  CSlider::GetNumTics() const
	// Retrieves the number of tick marks in the trackbar.
	{
		assert(IsWindow());
		return static_cast<int>(SendMessage(TBM_GETNUMTICS, 0L, 0L));
	}

	inline int  CSlider::GetPageSize() const
	// Retrieves the number of logical positions the trackbar's slider moves in response to
	// keyboard input, or mouse input, such as clicks in the trackbar's channel.
	{
		assert(IsWindow());
		return static_cast<int>(SendMessage(TBM_GETPAGESIZE, 0L, 0L));
	}

	inline int  CSlider::GetPos() const
	// Retrieves the current logical position of the slider in the trackbar.
	{
		assert(IsWindow());
		return static_cast<int>(SendMessage(TBM_GETPOS, 0L, 0L));
	}

	inline int  CSlider::GetRangeMax() const
	// Retrieves the maximum position for the slider in the trackbar.
	{
		assert(IsWindow());
		return static_cast<int>(SendMessage(TBM_GETRANGEMAX, 0L, 0L));
	}

	inline int  CSlider::GetRangeMin() const
	// Retrieves the minimum position for the slider in the trackbar.
	{
		assert(IsWindow());
		return static_cast<int>(SendMessage(TBM_GETRANGEMIN, 0L, 0L));
	}

	inline int  CSlider::GetSelEnd() const
	// Retrieves the ending position of the current selection range in the trackbar.
	{
		assert(IsWindow());
		return static_cast<int>(SendMessage(TBM_GETSELEND, 0L, 0L));
	}

	inline int  CSlider::GetSelStart() const
	// Retrieves the starting position of the current selection range in the trackbar.
	{
		assert(IsWindow());
		return static_cast<int>(SendMessage(TBM_GETSELSTART, 0L, 0L));
	}

	inline int  CSlider::GetThumbLength() const
	// Retrieves the length of the slider in the trackbar.
	{
		assert(IsWindow());
		return static_cast<int>(SendMessage(TBM_GETTHUMBLENGTH, 0L, 0L));
	}

	inline CRect CSlider::GetThumbRect() const
	// Retrieves the size and position of the bounding rectangle for the slider in the trackbar.
	{
		CRect rc;
		SendMessage(TBM_GETTHUMBRECT, 0L, (LPARAM)&rc);
		return rc;
	}

	inline int CSlider::GetTic(int nTic ) const
	// Retrieves the logical position of a tick mark in the trackbar.
	{
		assert(IsWindow());
		return static_cast<int>(SendMessage(TBM_GETTIC, (WPARAM)nTic, 0L));
	}

	inline int  CSlider::GetTicPos(int nTic) const
	// Retrieves the current physical position of a tick mark in the trackbar.
	{
		assert(IsWindow());
		return static_cast<int>(SendMessage(TBM_GETTICPOS, (WPARAM)nTic, 0L));
	}

	inline HWND CSlider::GetToolTips() const
	// Retrieves the handle to the ToolTip control assigned to the trackbar, if any.
	{
		assert(IsWindow());
		return reinterpret_cast<HWND>(SendMessage(TBM_GETTOOLTIPS, 0L, 0L));
	}

	inline HWND CSlider::SetBuddy(HWND hBuddy, BOOL fLocation /*= TRUE*/ ) const
	// Assigns a window as the buddy window for the trackbar control.
	{
		assert(IsWindow());
		return reinterpret_cast<HWND>(SendMessage(TBM_SETBUDDY, (WPARAM)fLocation, (LPARAM)hBuddy));
	}

	inline int  CSlider::SetLineSize(int nSize) const
	// Sets the number of logical positions the trackbar's slider moves in response to
	// keyboard input from the arrow keys.
	{
		assert(IsWindow());
		return static_cast<int>(SendMessage(TBM_SETLINESIZE, 0L, (LPARAM)nSize));
	}

	inline int  CSlider::SetPageSize(int nSize) const
	// Sets the number of logical positions the trackbar's slider moves in response to
	// keyboard input, or mouse input such as clicks in the trackbar's channel.
	{
		assert(IsWindow());
		return static_cast<int>(SendMessage(TBM_SETPAGESIZE, 0L, (LPARAM)nSize));
	}

	inline void CSlider::SetPos(int nPos, BOOL Redraw) const
	// Sets the current logical position of the slider in the trackbar.
	{
		assert(IsWindow());
		SendMessage(TBM_SETPOS, (WPARAM)Redraw, (LPARAM)nPos);
	}

	inline void CSlider::SetRangeMax(int nMax, BOOL Redraw) const
	// Sets the maximum logical position for the slider in the trackbar.
	{
		assert(IsWindow());
		SendMessage(TBM_SETRANGEMAX, (WPARAM)Redraw, (LPARAM)nMax);
	}

	inline void CSlider::SetRangeMin(int nMin, BOOL Redraw) const
	// Sets the minimum logical position for the slider in the trackbar.
	{
		assert(IsWindow());
		SendMessage(TBM_SETRANGEMIN, (WPARAM)Redraw, (LPARAM)nMin);
	}

	inline void CSlider::SetSelection(int nMin, int nMax, BOOL Redraw) const
	// Sets the starting and ending positions for the available selection range in the trackbar.
	{
		assert(IsWindow());
		SendMessage(TBM_SETSEL, (WPARAM)Redraw, (LPARAM)MAKELONG(nMax, nMin));
	}

	inline BOOL CSlider::SetTic(int nTic) const
	// Sets a tick mark in the trackbar at the specified logical position.
	{
		assert(IsWindow());
		return static_cast<BOOL>(SendMessage(TBM_SETTIC, 0L, (LPARAM)nTic));
	}

	inline void CSlider::SetTicFreq(int nFreq)  const
	// Sets the interval frequency for tick marks in the trackbar.
	{
		assert(IsWindow());
		SendMessage(TBM_SETTICFREQ, (WPARAM)nFreq, 0L);
	}

	inline int  CSlider::SetTipSide(int nLocation) const
	// Positions a ToolTip control used by the trackbar control.
	{
		assert(IsWindow());
		return static_cast<int>(SendMessage(TBM_SETTIPSIDE, (WPARAM)nLocation, 0L));
	}

	inline void CSlider::SetToolTips(HWND hToolTip) const
	// Assigns a ToolTip control to the trackbar control.
	{
		assert(IsWindow());
		SendMessage(TBM_SETTOOLTIPS, (WPARAM)hToolTip, 0L);
	}

	////////////////////////////////////////
	// Definitions for the CSpinButton class
	//
	inline int CSpinButton::GetAccel(int cAccels, LPUDACCEL paAccels) const
	// Retrieves acceleration information for the up-down control.
	{
		assert(IsWindow());
		return static_cast<int>(SendMessage(UDM_GETACCEL, (WPARAM)cAccels, (LPARAM)paAccels));
	}

	inline int CSpinButton::GetBase() const
	// Retrieves the current radix base (that is, either base 10 or 16) for the up-down control.
	{
		assert(IsWindow());
		return static_cast<int>(SendMessage(UDM_GETBASE, 0L, 0L));
	}

	inline HWND CSpinButton::GetBuddy() const
	// Retrieves the handle to the current buddy window.
	{
		assert(IsWindow());
		return reinterpret_cast<HWND>(SendMessage(UDM_GETBUDDY, 0L, 0L));
	}

	inline int CSpinButton::GetPos() const
	// Retrieves the current position of the up-down control with 16-bit precision.
	{
		assert(IsWindow());
		return static_cast<int>(SendMessage(UDM_GETPOS, 0L, 0L));
	}

	inline DWORD CSpinButton::GetRange() const
	// Retrieves the minimum and maximum positions (range) for the up-down control.
	{
		assert(IsWindow());
		return static_cast<DWORD>(SendMessage(UDM_GETRANGE, 0L, 0L));
	}

	inline void CSpinButton::PreCreate(CREATESTRUCT& cs)
    {
		cs.style = WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VISIBLE |UDS_SETBUDDYINT;
	}

	inline void CSpinButton::PreRegisterClass(WNDCLASS& wc)
	{
		wc.lpszClassName = UPDOWN_CLASS;
	}

	inline BOOL CSpinButton::SetAccel(int cAccels, LPUDACCEL paAccels) const
	// Sets the acceleration for the up-down control.
	{
		assert(IsWindow());
		return static_cast<BOOL>(SendMessage(UDM_SETACCEL, (WPARAM)cAccels, (LPARAM)paAccels));
	}

	inline int CSpinButton::SetBase(int nBase) const
	// Sets the radix base for the up-down control.
	{
		assert(IsWindow());
		return static_cast<int>(SendMessage(UDM_SETBASE, (WPARAM)nBase, 0L));
	}

	inline HWND CSpinButton::SetBuddy(HWND hBuddy) const
	// Sets the buddy window for the up-down control.
	{
		assert(IsWindow());
		return reinterpret_cast<HWND>(SendMessage(UDM_SETBUDDY, (WPARAM)hBuddy, 0L));
	}

	inline int CSpinButton::SetPos(int nPos) const
	// Sets the current position for the up-down control with 16-bit precision.
	{
		assert(IsWindow());
		return static_cast<int>(SendMessage(UDM_SETPOS, 0L, (LPARAM)MAKELONG ((short) nPos, 0)));
	}

	inline void CSpinButton::SetRange(int nLower, int nUpper) const
	// Sets the minimum and maximum positions (range) for the up-down control.
	{
		assert(IsWindow());
		SendMessage(UDM_SETRANGE, 0L, (LPARAM)MAKELONG(nUpper, nLower));
	}

	////////////////////////////////////////
	// Definitions for the CToolTip class
	//

	inline CToolTip::CToolTip()
	{}

	inline CToolTip::~CToolTip()
	{}

	inline void CToolTip::Activate(BOOL Activate) const
	// Activates or deactivates a ToolTip control.
	{
		assert(IsWindow());
		SendMessage(TTM_ACTIVATE, (WPARAM)Activate, 0L);
	}

	inline BOOL CToolTip::AddTool(HWND hWndControl, const RECT& rcTool, UINT uID, UINT nIDText) const
	// Registers a tool with a ToolTip control.
	// hWndControl specifies the window which triggers the tooltip.
	// rcTool specifies the part of the window which triggers the tooltip.
	// nIDText specifies the ID of the text resource.
	// uID is a user defined ID. It is required if the control has multiple tooltips.
	{
		assert(IsWindow());
		TOOLINFO ti;
		FillToolInfo(ti, hWndControl, rcTool, uID);
		ti.hinst = GetApp().GetResourceHandle();
		ti.lpszText = MAKEINTRESOURCE(nIDText);
		return static_cast<BOOL>(SendMessage(TTM_ADDTOOL, 0L, (LPARAM)&ti));
	}

	inline BOOL CToolTip::AddTool(HWND hWndControl, UINT nIDText) const
	// Registers a tool with a ToolTip control.
	// hWndControl specifies the window which triggers the tooltip.
	// nIDText specifies the ID of the text resource
	{
		assert(IsWindow());
		TOOLINFO ti;
		FillToolInfo(ti, hWndControl);
		ti.hinst = GetApp().GetResourceHandle();
		ti.lpszText = MAKEINTRESOURCE(nIDText);
		return static_cast<BOOL>(SendMessage(TTM_ADDTOOL, 0L, (LPARAM)&ti));
	}

	inline BOOL CToolTip::AddTool(HWND hWndControl, const RECT& rcTool, UINT uID, LPCTSTR lpszText /*= LPSTR_TEXTCALLBACK*/) const
	// Registers a tool with a ToolTip control.
	// hWndControl specifies the window which triggers the tooltip.
	// rcTool specifies the part of the window which triggers the tooltip.
	// If lpszText contains the value LPSTR_TEXTCALLBACK, TTN_NEEDTEXT notification
	//	messages are sent to the parent window.
	// uID is a user defined ID. It is required if the control has multiple tooltips.
	{
		assert(IsWindow());
		TOOLINFO ti;
		FillToolInfo(ti, hWndControl, rcTool, uID);
		ti.lpszText = const_cast<LPTSTR>(lpszText);
		return static_cast<BOOL>(SendMessage(TTM_ADDTOOL, 0L, (LPARAM)&ti));
	}

	inline BOOL CToolTip::AddTool(HWND hWndControl, LPCTSTR lpszText /*= LPSTR_TEXTCALLBACK*/) const
	// Registers a tool with a ToolTip control.
	// hWndControl specifies the window which triggers the tooltip.
	// If lpszText contains the value LPSTR_TEXTCALLBACK, TTN_NEEDTEXT notification
	//	messages are sent to the parent window.
	{
		assert(IsWindow());
		TOOLINFO ti;
		FillToolInfo(ti, hWndControl);
		ti.lpszText = const_cast<LPTSTR>(lpszText);
		return static_cast<BOOL>(SendMessage(TTM_ADDTOOL, 0L, (LPARAM)&ti));
	}

	inline void CToolTip::DelTool(HWND hWndControl, UINT uID) const
	// Removes a tool from a ToolTip control.
	{
		assert(IsWindow());
		TOOLINFO ti = GetToolInfo(hWndControl, uID);
		SendMessage(TTM_DELTOOL, 0L, (LPARAM)&ti);
	}

	inline int CToolTip::GetDelayTime(DWORD dwDuration) const
	// Retrieves the initial, pop-up, and reshow durations currently set for a ToolTip control.
	// Returns an intvalue with the specified duration in milliseconds.
	//
	// dwDuration is one of:
	//	TTDT_AUTOPOP - time the ToolTip window remains visible if the pointer is stationary
	//	TTDT_INITIAL - time the pointer must remain stationary before the ToolTip window appears.
	//	TTDT_RESHOW	 - time it takes for subsequent ToolTip windows to appear as the pointer moves from one tool to another.
	{
		assert(IsWindow());
		return static_cast<int>(SendMessage(TTM_GETDELAYTIME, dwDuration, 0L));
	}

	inline CRect CToolTip::GetMargin() const
	// Retrieves the top, left, bottom, and right margins set for a ToolTip window.
	{
		assert(IsWindow());
		CRect rc;
		SendMessage(TTM_GETMARGIN, 0L, (LPARAM)&rc);
		return rc;
	}

	inline int CToolTip::GetMaxTipWidth() const
	// Retrieves the maximum width for a ToolTip window.
	{
		assert(IsWindow());
		return static_cast<int>(SendMessage(TTM_GETMAXTIPWIDTH, 0L, 0L));
	}

	inline CString CToolTip::GetText(HWND hWndControl, UINT uID) const
	// Retrieves the text information a ToolTip control maintains about a tool.
	{
		assert(IsWindow());
		CString str;
		TOOLINFO ti = GetToolInfo(hWndControl, uID);

		LPTSTR pszText = str.GetBuffer(80);	// Maximum allowed ToolTip is 80 characters for Windows XP and below
		ti.lpszText = pszText;
		SendMessage(TTM_GETTEXT, 0L, (LPARAM)&ti);
		str.ReleaseBuffer();

		return str;
	}

	inline COLORREF CToolTip::GetTipBkColor() const
	// Retrieves the background color in a ToolTip window.
	{
		assert(IsWindow());
		return static_cast<COLORREF>(SendMessage(TTM_GETTIPBKCOLOR, 0L, 0L));
	}

	inline COLORREF CToolTip::GetTipTextColor() const
	// Retrieves the text color in a ToolTip window.
	{
		assert(IsWindow());
		return static_cast<COLORREF>(SendMessage(TTM_GETTIPTEXTCOLOR, 0L, 0L));
	}

	inline int CToolTip::GetToolCount() const
	// Retrieves a count of the tools maintained by a ToolTip control.
	{
		assert(IsWindow());
		return static_cast<int>(SendMessage(TTM_GETTOOLCOUNT, 0L, 0L));
	}

	inline TOOLINFO CToolTip::GetToolInfo(HWND hWndControl, UINT uID) const
	// Retrieves the information that a ToolTip control maintains about a tool.
	// hWndControl is the control specified when the tooltip was added with AddTool.
	// uID is the user ID (if any) specified when the tooltip was added with AddTool.
	// Note:
	// The TOOLINFO struct returned does not contain the ToolTip's text.
	// Use GetText to retrieve a tool's text.
	{
		assert(IsWindow());
		TOOLINFO ti;
		ZeroMemory(&ti, sizeof(ti));
		ti.cbSize = sizeof(ti);
		if (uID == (UINT)-1)
		{
			ti.hwnd = GetParent();
			ti.uId = (UINT_PTR)hWndControl;
		}
		else
		{
			ti.hwnd = hWndControl;
			ti.uId = uID;
		}

		VERIFY(SendMessage(TTM_GETTOOLINFO, 0L, (LPARAM)&ti));

		return ti;
	}

	inline BOOL CToolTip::HitTest(HWND hWnd, CPoint pt, const TOOLINFO& ToolInfo) const
	// Tests a point to determine whether it is within the bounding rectangle of the
	//  specified tool and, if it is, retrieves information about the tool.
	{
		assert(IsWindow());
		TTHITTESTINFO hti;
		ZeroMemory(&hti, sizeof(TTHITTESTINFO));
		hti.hwnd = hWnd;
		hti.pt = pt;
		hti.ti = ToolInfo;
		return static_cast<BOOL>(SendMessage(TTM_HITTEST, 0L, (LPARAM)&hti));
	}

	inline void CToolTip::FillToolInfo(TOOLINFO& ti, HWND hControl) const
	// Fills the TOOLINFO structure. Used by AddTool.
	// Notes:
	// 1) Notifications are passed to the parent window.
	// 2) The control is always identified by its hwnd.
	// 3) The tooltip always manages its messages (uses TTF_SUBCLASS).
	// Override this function to specify different uFlags.
	{
		ZeroMemory(&ti, sizeof(TOOLINFO));
		ti.cbSize = sizeof(TOOLINFO);

		ti.hwnd = ::GetParent(m_hWnd);	// pass notifications to the parent window
		ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
		ti.uId = (UINT_PTR)hControl;
	}

	inline void CToolTip::FillToolInfo(TOOLINFO& ti, HWND hControl, const RECT& rc, UINT uID) const
	// Fills the TOOLINFO structure. Used by AddTool.
	// Notes:
	// 1) Notifications are passed to the hControl window, not the parent window.
	// 2) The control is always identified by its hwnd.
	// 3) rc specifies which part of the control triggers the tooltip.
	// 4) A unique uID is required if the control has multiple tooltips.
	// 5) The tooltip always manages its messages (uses TTF_SUBCLASS).
	// 6) The TTF_IDISHWND style is incompatible with using a RECT.
	// Override this function to specify different uFlags.
	{
		ZeroMemory(&ti, sizeof(TOOLINFO));
		ti.cbSize = sizeof(TOOLINFO);

		ti.hwnd = hControl;
		ti.uFlags = TTF_SUBCLASS;
		ti.rect = rc;
		ti.uId = uID;
	}

	inline void CToolTip::Pop() const
	// Removes a displayed ToolTip window from view.
	{
		assert(IsWindow());
		SendMessage(TTM_POP, 0L, 0L);
	}

	inline void CToolTip::PreCreate(CREATESTRUCT& cs)
	{
		cs.dwExStyle = WS_EX_TOOLWINDOW;
		cs.style = WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP;
	}

	inline void  CToolTip::PreRegisterClass(WNDCLASS& wc)
	{
		wc.lpszClassName = TOOLTIPS_CLASS;
	}

	inline void CToolTip::RelayEvent(MSG& Msg) const
	// Passes a mouse message to a ToolTip control for processing.
	{
		assert(IsWindow());
		SendMessage(TTM_RELAYEVENT, 0L, (LPARAM)&Msg);
	}

	inline void CToolTip::SetDelayTime(UINT nDelay) const
	// Sets the initial delay for a ToolTip control.
	{
		assert(IsWindow());
		SendMessage(TTM_SETDELAYTIME, TTDT_INITIAL, (LPARAM)nDelay);
	}

	inline void CToolTip::SetDelayTime(DWORD dwDuration, int iTime) const
	// Sets the initial, pop-up, and reshow durations for a ToolTip control.
	{
		assert(IsWindow());
		SendMessage(TTM_SETDELAYTIME, dwDuration, (LPARAM)iTime);
	}

	inline void CToolTip::SetMargin(const RECT& rc) const
	// Sets the top, left, bottom, and right margins for a ToolTip window.
	{
		assert(IsWindow());
		SendMessage(TTM_SETMARGIN, 0L, (LPARAM)&rc);
	}

	inline int CToolTip::SetMaxTipWidth(int iWidth) const
	// Sets the maximum width for a ToolTip window in pixels.
	{
		assert(IsWindow());
		return static_cast<int>(SendMessage(TTM_SETMAXTIPWIDTH, 0L, (LPARAM)iWidth));
	}

	inline void CToolTip::SetTipBkColor(COLORREF clr) const
	// Sets the background color in a ToolTip window.
	// Ignored when XP themes are active.
	{
		assert(IsWindow());
		SendMessage(TTM_SETTIPBKCOLOR, (WPARAM)clr, 0L);
	}

	inline void CToolTip::SetTipTextColor(COLORREF clr) const
	// Sets the text color in a ToolTip window.
	// Ignored when XP themes are active.
	{
		assert(IsWindow());
		SendMessage(TTM_SETTIPTEXTCOLOR, (WPARAM)clr, 0L);
	}

	inline void CToolTip::SetToolInfo(const TOOLINFO& ToolInfo) const
	// Sets the information that a ToolTip control maintains for a tool.
	{
		assert(IsWindow());
		SendMessage(TTM_SETTOOLINFO, 0L, (LPARAM)&ToolInfo);
	}

	inline void CToolTip::SetToolRect(const RECT& rc, HWND hWndControl, UINT uID) const
	// Sets a new bounding rectangle for a tool.
	{
		assert(IsWindow());
		TOOLINFO ti = GetToolInfo(hWndControl, uID);
		ti.rect = rc;
		SendMessage(TTM_NEWTOOLRECT, 0L, (LPARAM)&ti);
	}

	inline void CToolTip::Update() const
	// Forces the current tool to be redrawn.
	{
		assert(IsWindow());
		SendMessage(TTM_UPDATE, 0L, 0L);
	}

	inline void CToolTip::UpdateTipText(LPCTSTR lpszText, HWND hWndControl, UINT uID) const
	// Sets the ToolTip text for a tool.
	{
		assert(IsWindow());
		TOOLINFO ti = GetToolInfo(hWndControl, uID);
		ti.lpszText = (LPTSTR)lpszText;
		SendMessage(TTM_UPDATETIPTEXT, 0L, (LPARAM)&ti);
	}

	inline void CToolTip::UpdateTipText(UINT nIDText, HWND hWndControl, UINT uID) const
	// Sets the ToolTip text for a tool.
	{
		assert(IsWindow());
		TOOLINFO ti = GetToolInfo(hWndControl, uID);
		ti.lpszText = MAKEINTRESOURCE(nIDText);
		SendMessage(TTM_UPDATETIPTEXT, 0L, (LPARAM)&ti);
	}

#if (_WIN32_IE >=0x0500)
	inline BOOL CToolTip::AdjustRect(RECT& rc, BOOL IsLarger /*= TRUE*/) const
	// Calculates a ToolTip control's text display rectangle from its window rectangle, or the
	//  ToolTip window rectangle needed to display a specified text display rectangle.
	{
		assert(IsWindow());
		return static_cast<BOOL>(SendMessage(TTM_ADJUSTRECT, (WPARAM)IsLarger, (LPARAM)&rc));
	}

	inline CSize CToolTip::GetBubbleSize(HWND hWndControl, UINT uID) const
	// Returns the width and height of a ToolTip control.
	{
		assert(IsWindow());
		TOOLINFO ti = GetToolInfo(hWndControl, uID);
		LRESULT lr = SendMessage(TTM_GETBUBBLESIZE, 0L, (LPARAM)&ti);
		CSize sz(LOWORD(lr), HIWORD(lr));
		return sz;
	}

#ifdef TTM_SETTITLE
	inline BOOL CToolTip::SetTitle(UINT uIcon, LPCTSTR lpstrTitle) const
	// Adds a standard icon and title string to a ToolTip.
	{
		assert(IsWindow());
		return static_cast<BOOL>(SendMessage(TTM_SETTITLE, (WPARAM)uIcon, (LPARAM)lpstrTitle));
	}
#endif
#endif

#if (WINVER >= 0x0501) && defined(TTM_SETWINDOWTHEME)
	inline void CToolTip::SetTTWindowTheme(LPCWSTR lpstrTheme) const
	// Sets the visual style of a ToolTip control.
	{
		assert(IsWindow());
		SendMessage(TTM_SETWINDOWTHEME, 0L, (LPARAM)lpstrTheme);
	}
#endif



} // namespace Win32xx

#endif //  define _WIN32XX_CONTROLS_H_

