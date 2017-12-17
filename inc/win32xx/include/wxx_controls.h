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

    class CMonthCalendar;
    class CToolTip;


    // The CAnimation class provides the functionality of an animation control.
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
        CAnimation(const CAnimation&);              // Disable copy construction
        CAnimation& operator = (const CAnimation&); // Disable assignment operator
    };


    // The CComboBox class provides the functionality of a combo box control.
    class CComboBox : public CWnd
    {
    public:
        CComboBox() {}
        virtual ~CComboBox() {}

        virtual int CompareItem(LPCOMPAREITEMSTRUCT pCompareItemStruct);

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
        virtual LRESULT OnMessageReflect(UINT uMsg, WPARAM, LPARAM lParam);
        virtual void PreRegisterClass(WNDCLASS& wc) { wc.lpszClassName = _T("ComboBox"); }

    private:
        CComboBox(const CComboBox&);                // Disable copy construction
        CComboBox& operator = (const CComboBox&);   // Disable assignment operator
    };


    // The CComboBoxEx class provides the functionality of a ComboBoxEx control.
    class CComboBoxEx : public CComboBox
    {
    public:
        CComboBoxEx() {}
        virtual ~CComboBoxEx() {}

        int     DeleteItem(int nIndex ) const;
        HWND    GetComboBoxCtrl() const;
        HWND    GetEditCtrl() const;
        CImageList GetImageList() const;
        BOOL    GetItem(COMBOBOXEXITEM& CBItem) const;
        BOOL    HasEditChanged () const;
        int     InsertItem(const COMBOBOXEXITEM& CBItem) const;
        CImageList SetImageList(HIMAGELIST himlNew) const;
        BOOL    SetItem(const COMBOBOXEXITEM& CBItem) const;
        DWORD   GetExtendedStyle() const;
        DWORD   SetExtendedStyle(DWORD dwExMask, DWORD dwExStyles ) const;

    protected:
        // Overridables
        virtual void PreRegisterClass(WNDCLASS& wc) { wc.lpszClassName = WC_COMBOBOXEX; }

    private:
        CComboBoxEx(const CComboBoxEx&);                // Disable copy construction
        CComboBoxEx& operator = (const CComboBoxEx&);   // Disable assignment operator
    };


    // The CHeader class provides the functionality of a header control.
    class CHeader : public CWnd
    {
    public:
        CHeader() {}
        virtual ~CHeader() {}

        // Attributes
        CImageList GetImageList() const;
        BOOL    GetItem(int nPos, HDITEM& HeaderItem) const;
        int     GetItemCount() const;
        CRect   GetItemRect(int nIndex) const;
        BOOL    GetOrderArray(LPINT piArray, int iCount) const;
        int     OrderToIndex(int nOrder) const;
        CImageList SetImageList(HIMAGELIST himlNew) const;
        BOOL    SetItem(int nPos, const HDITEM& HeaderItem) const;
        BOOL    SetOrderArray(int iCount, LPINT piArray) const;
        int     GetBitmapMargin() const;
        int     SetBitmapMargin(int nWidth) const;

        // Operations
        CImageList CreateDragImage(int nIndex) const;
        BOOL    DeleteItem(int nPos) const;
        int     InsertItem(int nPos, const HDITEM& phdi) const;
        BOOL    Layout(HDLAYOUT* pHeaderLayout) const;
#ifdef Header_SetHotDivider
        int     SetHotDivider(CPoint pt) const;
        int     SetHotDivider(int nIndex) const;
#endif
#ifdef Header_ClearFilter
        int     ClearAllFilters() const;
        int     ClearFilter(int nColumn) const;
        int     EditFilter(int nColumn, BOOL DiscardChanges) const;
        int     SetFilterChangeTimeout(DWORD dwTimeOut) const;
#endif

    protected:
        // Overridables
        virtual void PreRegisterClass(WNDCLASS& wc) { wc.lpszClassName = WC_HEADER ; }

    private:
        CHeader(const CHeader&);                // Disable copy construction
        CHeader& operator = (const CHeader&);   // Disable assignment operator
    };


    // The CHotKey class provides the functionality a hot key control.
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
        CHotKey(const CHotKey&);                // Disable copy construction
        CHotKey& operator = (const CHotKey&);   // Disable assignment operator
    };


    // The CIPAddress class provides the functionality of an IP Address control.
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
        CIPAddress(const CIPAddress&);              // Disable copy construction
        CIPAddress& operator = (const CIPAddress&); // Disable assignment operator
    };


    // The CMonthCalendar class provides the functionality of a Month Calendar control.
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
        CMonthCalendar(const CMonthCalendar&);              // Disable copy construction
        CMonthCalendar& operator = (const CMonthCalendar&); // Disable assignment operator
    };


    // The CDateTime class provides the functionality of a Date Time control.
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
        CDateTime(const CDateTime&);                // Disable copy construction
        CDateTime& operator = (const CDateTime&);   // Disable assignment operator
    };


    // The CProgressBar class provides the functionality of a ProgressBar control.
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
        CProgressBar(const CProgressBar&);              // Disable copy construction
        CProgressBar& operator = (const CProgressBar&); // Disable assignment operator
    };


    // The CScrollBar class provides the functionality of a ScrollBar control.
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
        CScrollBar(const CScrollBar&);              // Disable copy construction
        CScrollBar& operator = (const CScrollBar&); // Disable assignment operator
    };


    // The CSlider class provides the functionality of a TrackBar (aka Slider) control.
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
        CSlider(const CSlider&);                // Disable copy construction
        CSlider& operator = (const CSlider&);   // Disable assignment operator
    };


    // The CSpinButton class provides the functionality of an Up-Down (aka SpinButton) control.
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
        CSpinButton(const CSpinButton&);                // Disable copy construction
        CSpinButton& operator = (const CSpinButton&);   // Disable assignment operator
    };


    // The CToolTip class provides the functionality of a ToolTip control.
    class CToolTip : public CWnd
    {
    public:
        CToolTip();
        virtual ~CToolTip();

        //Attributes
        int      GetDelayTime(DWORD dwDuration) const;
        CRect    GetMargin() const;
        int      GetMaxTipWidth() const;
        CString  GetText(HWND hWndControl, UINT uID = -1) const;
        COLORREF GetTipBkColor() const;
        COLORREF GetTipTextColor() const;
        int      GetToolCount() const;
        TOOLINFO GetToolInfo(HWND hWndControl, UINT uID = -1) const;
        void     SetDelayTime(UINT nDelay) const;
        void     SetDelayTime(DWORD dwDuration, int iTime) const;
        void     SetMargin(const RECT& rc) const;
        int      SetMaxTipWidth(int iWidth) const;
        void     SetTipBkColor(COLORREF clr) const;
        void     SetTipTextColor(COLORREF clr) const;
        void     SetToolInfo(const TOOLINFO& ToolInfo) const;
#if (_WIN32_IE >=0x0500)
        CSize    GetBubbleSize(HWND hWndControl, UINT uID = -1) const;
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
        CToolTip(const CToolTip&);              // Disable copy construction
        CToolTip& operator = (const CToolTip&); // Disable assignment operator

    };


} // namespace Win32xx

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

namespace Win32xx
{

    ////////////////////////////////////////
    // Definitions for the CAnimation class
    //

    // Closes an AVI clip.
    inline BOOL CAnimation::Close() const
    {
        assert(IsWindow());
        return Animate_Close(*this);
    }


    // Opens an AVI clip and displays its first frame in an animation control.
    inline BOOL CAnimation::Open(LPTSTR lpszName) const
    {
        assert(IsWindow());
        return Animate_Open(*this, lpszName);
    }


    // Plays an AVI clip in an animation control. The control plays the clip
    // in the background while the thread continues executing.
    //  nFrom : Zero - based index of the frame where playing begins. The value must be less than 65,536.
    //  nTo:    Zero - based index of the frame where playing ends. The value must be less than 65,536.
    //          A value of - 1 means end with the last frame in the AVI clip.
    //  nRepeat: Number of times to replay the AVI clip.A value of - 1 means replay the clip indefinitely.
    inline BOOL CAnimation::Play(int nFrom, int nTo, int nRepeat) const
    {
        assert(IsWindow());
        return Animate_Play(*this, nFrom, nTo, nRepeat);
    }


    // Directs an animation control to display a particular frame of an AVI clip.
    // The control displays the clip in the background while the thread continues executing.
    inline BOOL CAnimation::Seek(int nFrame) const
    {
        assert(IsWindow());
        return Animate_Seek(*this, nFrame);
    }


    // Stops playing an AVI clip in an animation control.
    inline BOOL CAnimation::Stop() const
    {
        assert(IsWindow());
        return Animate_Stop(*this);
    }


    ////////////////////////////////////////
    // Definitions for the CComboBox class
    //

    // Adds a string to the list box of a combo box. If the combo box does not
    // have the CBS_SORT style, the string is added to the end of the list.
    // Otherwise, the string is inserted into the list, and the list is sorted.
    inline int  CComboBox::AddString(LPCTSTR lpszString) const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(CB_ADDSTRING, 0L, reinterpret_cast<LPARAM>(lpszString)));
    }


    // Called by in response to the WM_COMPAREITEM message to determine the relative position
    // of a new item in a sorted owner-draw list box. Override this function in an owner-drawn
    // Combo-Box to specify the sort order when items are added using AddString.
    inline int CComboBox::CompareItem(LPCOMPAREITEMSTRUCT)
    {
        // The return value indicates the relative position of the two items.
        // It may be any of the values shown in the following table.
        //
        //  Value Meaning
        //  -1 Item 1 precedes item 2 in the sorted order.
        //   0 Items 1 and 2 are equivalent in the sorted order.
        //   1 Item 1 follows item 2 in the sorted order.

        return 0;
    }


    // Deletes the current selection, if any, from the combo box's edit control.
    inline void CComboBox::Clear() const
    {
        assert(IsWindow());
        SendMessage(WM_CLEAR, 0L, 0L);
    }

    // Copies the current selection to the clipboard in CF_TEXT format.
    inline void CComboBox::Copy() const
    {
        assert(IsWindow());
        SendMessage(WM_COPY, 0L, 0L);
    }


    // Deletes the current selection, if any, in the edit control and copies
    // the deleted text to the clipboard in CF_TEXT format.
    inline void CComboBox::Cut() const
    {
        assert(IsWindow());
        SendMessage(WM_CUT, 0L, 0L);
    }


    // Deletes a string in the list box of a combo box.
    inline int  CComboBox::DeleteString(int nIndex) const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(CB_DELETESTRING, nIndex, 0L));
    }


    // Adds the names of directories and files that match a specified string
    // and set of file attributes.
    inline int  CComboBox::Dir(UINT attr, LPCTSTR lpszWildCard ) const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(CB_DIR, attr, reinterpret_cast<LPARAM>(lpszWildCard)));
    }


    // Search the list box of a combo box for an item beginning with the
    // characters in a specified string.
    inline int  CComboBox::FindString(int nIndexStart, LPCTSTR lpszString) const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(CB_FINDSTRING, nIndexStart, reinterpret_cast<LPARAM>(lpszString)));
    }


    // Find the first list box string in a combo box that matches the string specified in lpszString.
    inline int  CComboBox::FindStringExact(int nIndexStart, LPCTSTR lpszString) const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(CB_FINDSTRINGEXACT, nIndexStart, reinterpret_cast<LPARAM>(lpszString)));
    }


    // Retrieves the number of items in the list box of the combo box.
    inline int  CComboBox::GetCount() const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(CB_GETCOUNT, 0L, 0L));
    }


    // Retrieves the index of the currently selected item, if any, in the list box of the combo box.
    inline int  CComboBox::GetCurSel() const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(CB_GETCURSEL, 0L, 0L));
    }


    // Retrieves the screen coordinates of the combo box in its dropped-down state.
    inline CRect CComboBox::GetDroppedControlRect() const
    {
        assert(IsWindow());
        CRect rc;
        SendMessage(CB_GETDROPPEDCONTROLRECT, 0L, reinterpret_cast<LPARAM>(&rc));
        return rc;
    }


    // Determines whether the list box of the combo box is dropped down.
    inline BOOL CComboBox::GetDroppedState() const
    {
        assert(IsWindow());
        return static_cast<BOOL>(SendMessage(CB_GETDROPPEDSTATE, 0L, 0L));
    }


    // Retrieves the minimum allowable width, in pixels, of the list box of the combo box
    // with the CBS_DROPDOWN or CBS_DROPDOWNLIST style.
    inline int  CComboBox::GetDroppedWidth() const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(CB_GETDROPPEDWIDTH, 0L, 0L));
    }


    // Gets the starting and ending character positions of the current selection
    // in the edit control of the combo box.
    inline DWORD CComboBox::GetEditSel() const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(CB_GETEDITSEL, 0L, 0L));
    }


    // Determines whether the combo box has the default user interface or the extended user interface.
    inline BOOL CComboBox::GetExtendedUI() const
    {
        assert(IsWindow());
        return static_cast<BOOL>(SendMessage(CB_GETEXTENDEDUI, 0L, 0L));
    }


    // Retrieve from the combo box the width, in pixels, by which the list box can
    // be scrolled horizontally (the scrollable width).
    inline int  CComboBox::GetHorizontalExtent() const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(CB_GETHORIZONTALEXTENT, 0L, 0L));
    }


    // Retrieves the application-supplied value associated with the specified item in the combo box.
    inline DWORD CComboBox::GetItemData(int nIndex) const
    {
        assert(IsWindow());
        return static_cast<DWORD>(SendMessage(CB_GETITEMDATA, nIndex, 0L));
    }


    // Determines the height of list items or the selection field in the combo box.
    inline int  CComboBox::GetItemHeight(int nIndex) const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(CB_GETITEMHEIGHT, nIndex, 0L));
    }


    //  Retrieves a string from the list of the combo box.
    inline int  CComboBox::GetLBText(int nIndex, LPTSTR lpszText) const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(CB_GETLBTEXT, nIndex, reinterpret_cast<LPARAM>(lpszText)));
    }


    // Retrieves the length, in characters, of a string in the list of the combo box.
    inline int  CComboBox::GetLBTextLen(int nIndex) const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(CB_GETLBTEXTLEN, nIndex, 0L));
    }


    // Retrieves the current locale of the combo box.
    inline LCID CComboBox::GetLocale() const
    {
        assert(IsWindow());
        return static_cast<LCID>(SendMessage(CB_GETLOCALE, 0L, 0L));
    }


    // Retrieves the zero-based index of the first visible item in the list box portion of the combo box.
    inline int  CComboBox::GetTopIndex() const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(CB_GETTOPINDEX, 0L, 0L));
    }


    // Allocates memory for storing list box items. Use this before adding a
    // large number of items to the list box portion of a combo box.
    inline int  CComboBox::InitStorage(int nItems, int nBytes) const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(CB_INITSTORAGE, nItems, nBytes));
    }


    // Inserts a string into the list box of the combo box. Unlike the AddString,
    // a list with the CBS_SORT style is not sorted.
    inline int  CComboBox::InsertString(int nIndex, LPCTSTR lpszString) const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(CB_INSERTSTRING, nIndex, reinterpret_cast<LPARAM>(lpszString)));
    }

    // Handle messages reflected back from the parent window.
    // Override this function in your derived class to handle these special messages:
    // WM_COMMAND, WM_CTLCOLORBTN, WM_CTLCOLOREDIT, WM_CTLCOLORDLG, WM_CTLCOLORLISTBOX,
    // WM_CTLCOLORSCROLLBAR, WM_CTLCOLORSTATIC, WM_CHARTOITEM,  WM_VKEYTOITEM,
    // WM_HSCROLL, WM_VSCROLL, WM_DRAWITEM, WM_MEASUREITEM, WM_DELETEITEM,
    // WM_COMPAREITEM, WM_PARENTNOTIFY.
    inline LRESULT CComboBox::OnMessageReflect(UINT uMsg, WPARAM, LPARAM lParam)
    {
        switch (uMsg)
        {
        case WM_COMPAREITEM:    return CompareItem((LPCOMPAREITEMSTRUCT)lParam);
        }

        return 0;   // Allow other messages to be handled elsewhere. 
    }


    // Copies the current content of the clipboard to the combo box's edit control at the current caret position.
    inline void CComboBox::Paste() const
    {
        assert(IsWindow());
        SendMessage(WM_PASTE, 0L, 0L);
    }


    // Limits the length of the text the user may type into the edit control of the combo box.
    inline void CComboBox::LimitText(int nMaxChars) const
    {
        assert(IsWindow());
        SendMessage(CB_LIMITTEXT, nMaxChars, 0L);
    }


    // Removes all items from the list box and edit control of the combo box.
    inline void CComboBox::ResetContent() const
    {
        assert(IsWindow());
        SendMessage(CB_RESETCONTENT, 0L, 0L);
    }


    // Searches the list of a combo box for an item that begins with the characters in a
    // specified string. If a matching item is found, it is selected and copied to the edit control.
    inline int  CComboBox::SelectString(int nStartAfter, LPCTSTR lpszString) const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(CB_SELECTSTRING, nStartAfter, reinterpret_cast<LPARAM>(lpszString)));
    }


    // Selects a string in the list of the combo box. If necessary, the list scrolls the string into view.
    inline int  CComboBox::SetCurSel(int nIndex) const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(CB_SETCURSEL, nIndex, 0L));
    }


    // Sets the maximum allowable width, in pixels, of the list box of the combo box with
    // the CBS_DROPDOWN or CBS_DROPDOWNLIST style.
    inline int  CComboBox::SetDroppedWidth(int nWidth) const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(CB_SETDROPPEDWIDTH, nWidth, 0L));
    }


    // Selects characters in the edit control of the combo box.
    inline BOOL CComboBox::SetEditSel(int nStartChar, int nEndChar) const
    {
        assert(IsWindow());
        return static_cast<BOOL>(SendMessage(CB_SETEDITSEL, 0L, MAKELONG(nStartChar,nEndChar)));
    }


    // Selects either the default user interface or the extended user interface for the combo box that
    // has the CBS_DROPDOWN or CBS_DROPDOWNLIST style.
    inline int  CComboBox::SetExtendedUI(BOOL IsExtended) const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(CB_SETEXTENDEDUI, IsExtended, 0L));
    }


    // Sets the width, in pixels, by which the list box can be scrolled horizontally (the scrollable width).
    inline void CComboBox::SetHorizontalExtent(UINT nExtent ) const
    {
        assert(IsWindow());
        SendMessage(CB_SETHORIZONTALEXTENT, nExtent, 0L);
    }


    // Sets the value associated with the specified item in the combo box.
    inline int  CComboBox::SetItemData(int nIndex, DWORD dwItemData) const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(CB_SETITEMDATA, nIndex, dwItemData));
    }


    // Sets the height of list items or the selection field in the combo box.
    inline int  CComboBox::SetItemHeight(int nIndex, UINT cyItemHeight) const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(CB_SETITEMHEIGHT, nIndex, cyItemHeight));
    }


    // Sets the current locale of the combo box.
    inline LCID CComboBox::SetLocale( LCID NewLocale ) const
    {
        assert(IsWindow());
        return static_cast<LCID>(SendMessage(CB_SETLOCALE, NewLocale, 0L));
    }


    // Ensure that a particular item is visible in the list box of the combo box.
    inline int  CComboBox::SetTopIndex(int nIndex) const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(CB_SETTOPINDEX, nIndex, 0L));
    }


    // Ensure that a particular item is visible in the list box of the combo box.
    inline void CComboBox::ShowDropDown(BOOL Show) const
    {
        assert(IsWindow());
        SendMessage(CB_SHOWDROPDOWN, Show, 0L);
    }


    ////////////////////////////////////////
    // Definitions for the CComboBoxEx class
    //

    // Removes an item from the ComboBoxEx control.
    inline int  CComboBoxEx::DeleteItem(int nIndex ) const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(CBEM_DELETEITEM, nIndex, 0L));
    }


    // Retrieves the handle to the child combo box control.
    inline HWND CComboBoxEx::GetComboBoxCtrl() const
    {
        assert(IsWindow());
        return reinterpret_cast<HWND>(SendMessage(CBEM_GETCOMBOCONTROL, 0L, 0L));
    }


    // Retrieves the handle to the edit control portion of the ComboBoxEx control.
    inline HWND CComboBoxEx::GetEditCtrl() const
    {
        assert(IsWindow());
        return reinterpret_cast<HWND>(SendMessage(CBEM_GETEDITCONTROL, 0L, 0L));
    }


    // Retrieves the extended styles that are in use for the ComboBoxEx control.
    inline DWORD CComboBoxEx::GetExtendedStyle() const
    {
        assert(IsWindow());
        return static_cast<DWORD>(SendMessage(CBEM_GETEXTENDEDSTYLE, 0L, 0L));
    }


    // Retrieves the handle to an image list assigned to the ComboBoxEx control.
    inline CImageList CComboBoxEx::GetImageList() const
    {
        assert(IsWindow());
        HIMAGELIST himl = reinterpret_cast<HIMAGELIST>(SendMessage(CBEM_GETIMAGELIST, 0L, 0L));
        return CImageList(himl);
    }


    // Retrieves item information for the given ComboBoxEx item.
    inline BOOL CComboBoxEx::GetItem(COMBOBOXEXITEM& CBItem) const
    {
        assert(IsWindow());
        return static_cast<BOOL>(SendMessage(CBEM_GETITEM, 0L, reinterpret_cast<LPARAM>(&CBItem)));
    }


    // Determines whether or not the user has changed the text of the ComboBoxEx edit control.
    inline BOOL CComboBoxEx::HasEditChanged () const
    {
        assert(IsWindow());
        return static_cast<BOOL>(SendMessage(CBEM_HASEDITCHANGED, 0L, 0L));
    }


    // Inserts a new item in the ComboBoxEx control.
    inline int CComboBoxEx::InsertItem(const COMBOBOXEXITEM& CBItem) const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(CBEM_INSERTITEM, 0L, reinterpret_cast<LPARAM>(&CBItem)));
    }


    // Sets extended styles within the ComboBoxEx control.
    inline DWORD CComboBoxEx::SetExtendedStyle(DWORD dwExMask, DWORD dwExStyles ) const
    {
        assert(IsWindow());
        return static_cast<DWORD>(SendMessage(CBEM_SETEXTENDEDSTYLE, dwExMask, dwExStyles));
    }


    // Sets an image list for the ComboBoxEx control.
    inline CImageList CComboBoxEx::SetImageList(HIMAGELIST himlNew) const
    {
        assert(IsWindow());
        HIMAGELIST himl = reinterpret_cast<HIMAGELIST>(SendMessage(CBEM_SETIMAGELIST, 0L, 
                                                       reinterpret_cast<LPARAM>(himlNew)));
        return CImageList(himl);
    }


    // Sets the attributes for an item in the ComboBoxEx control.
    inline BOOL CComboBoxEx::SetItem(const COMBOBOXEXITEM& CBItem) const
    {
        assert(IsWindow());
        return static_cast<BOOL>(SendMessage(CBEM_SETITEM, 0L, reinterpret_cast<LPARAM>(&CBItem)));
    }

    ////////////////////////////////////////
    // Definitions for the CDateTime class
    //

    inline CDateTime::CDateTime()
    {
    }


    // Retrieves the color for a given portion of the month calendar within the date and time picker (DTP) control.
    inline COLORREF CDateTime::GetMonthCalColor(int iColor) const
    {
        assert(IsWindow());
        return static_cast<COLORREF>(DateTime_GetMonthCalColor(*this, iColor));
    }


    // Sets the color for a given portion of the month calendar within the date and time picker (DTP) control.
    inline COLORREF CDateTime::SetMonthCalColor(int iColor, COLORREF clr) const
    {
        assert(IsWindow());
        return static_cast<COLORREF>(DateTime_SetMonthCalColor(*this, iColor, clr));
    }


    // Sets the display of the date and time picker (DTP) control based on a given format string.
    inline BOOL CDateTime::SetFormat(LPCTSTR pszFormat) const
    {
        assert(IsWindow());
        return DateTime_SetFormat(*this, pszFormat);
    }


    // Retrieves the handle to the date and time picker's (DTP) child month calendar control.
    inline HWND CDateTime::GetMonthCalCtrl() const
    {
        assert(IsWindow());
        return reinterpret_cast<HWND>(DateTime_GetMonthCal(*this));
    }


    // Retrieves the font that the date and time picker (DTP) control's child month calendar control is currently using.
    inline CFont CDateTime::GetMonthCalFont() const
    {
        assert(IsWindow());
        HFONT hFont = reinterpret_cast<HFONT>(DateTime_GetMonthCalFont(*this));
        return CFont(hFont);
    }


    // Sets the font to be used by the date and time picker (DTP) control's child month calendar control.
    inline void CDateTime::SetMonthCalFont(HFONT hFont, BOOL Redraw /*= TRUE*/) const
    {
        assert(IsWindow());
        DateTime_SetMonthCalFont(*this, hFont, MAKELONG(Redraw, 0));
    }


    // Retrieves the current minimum and maximum allowable system times for the date and time picker (DTP) control.
    inline DWORD CDateTime::GetRange(SYSTEMTIME& MinRange, SYSTEMTIME& MaxRange) const
    {
        assert(IsWindow());

        SYSTEMTIME Ranges[2];
        ZeroMemory(Ranges, 2 * sizeof(SYSTEMTIME));
        DWORD Result = DateTime_GetRange(*this, Ranges);
        MinRange = Ranges[0];
        MaxRange = Ranges[1];
        return Result;
    }


    // Sets the minimum and maximum allowable system times for the date and time picker (DTP) control.
    inline BOOL CDateTime::SetRange(const SYSTEMTIME& MinRange, const SYSTEMTIME& MaxRange) const
    {
        assert(IsWindow());
        SYSTEMTIME Ranges[2];
        Ranges[0] = MinRange;
        Ranges[1] = MaxRange;
        DWORD flags = GDTR_MIN | GDTR_MAX;

        return DateTime_SetRange(*this, flags, Ranges);
    }


    // Returns the currently selected time from a date and time picker (DTP) control
    // Possible return codes are: GDT_VALID, GDT_ERROR, and GDT_NONE.
    // A return code of GDT_NONE occurs if the control was set to the DTS_SHOWNONE
    // style and the control check box was not selected.
    inline SYSTEMTIME CDateTime::GetTime(DWORD* pReturnCode) const
    {
        assert(IsWindow());
        SYSTEMTIME SysTime;
        ZeroMemory(&SysTime,  sizeof(SysTime));
        DWORD Res = DateTime_GetSystemtime(*this, &SysTime);
        if (pReturnCode)
            *pReturnCode = Res;

        return SysTime;
    }


    // Sets the date and time picker (DTP) control to a given date and time.
    inline BOOL CDateTime::SetTime(const SYSTEMTIME& TimeNew) const
    {
        assert(IsWindow());
        return DateTime_SetSystemtime(*this, GDT_VALID, &TimeNew);
    }


    // Set the DTP control to "no date" and clear its check box.
    // Only applies to DTP controls that are set to the DTS_SHOWNONE style.
    inline BOOL CDateTime::SetTimeNone() const
    {
        assert(IsWindow());
        return DateTime_SetSystemtime(*this, GDT_NONE, NULL);
    }


    ////////////////////////////////////////
    // Definitions for the CHotKey class
    //

    // Gets the virtual key code and modifier flags of a hot key
    inline DWORD CHotKey::GetHotKey() const
    {
        assert(IsWindow());
        return static_cast<DWORD>(SendMessage(HKM_GETHOTKEY, 0L, 0L));
    }


    // Retrieves a string that represents the name of a key.
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
            nStrLen = ::GetKeyNameText(static_cast<LONG>(lParam), pszStr, nBufferLen + 1);
            str.ReleaseBuffer();
        }

        return str;
    }


    // Sets the hot key combination for the control.
    inline void CHotKey::SetHotKey(DWORD dwKey) const
    {
        assert(IsWindow());
        SendMessage(HKM_SETHOTKEY, static_cast<WPARAM>(dwKey), 0L);
    }


    // Defines the invalid combinations and the default modifier combination for the control.
    inline void CHotKey::SetRules(WORD wInvalidComb, WORD wModifiers) const
    {
        assert(IsWindow());
        SendMessage(HKM_SETRULES, wInvalidComb, wModifiers);
    }


    ////////////////////////////////////////
    // Definitions for the CHeader class
    //

    // Creates a transparent version of an item image within the header control.
    inline CImageList CHeader::CreateDragImage(int nIndex) const
    {
        assert(IsWindow());
        HIMAGELIST himl = Header_CreateDragImage(*this, nIndex);
        return CImageList(himl);
    }


    // Deletes an item from the header control.
    inline BOOL CHeader::DeleteItem(int nPos) const
    {
        assert(IsWindow());
        return Header_DeleteItem(*this, nPos);
    }


    // Retrieves the image list that has been set for the header control.
    inline CImageList CHeader::GetImageList() const
    {
        assert(IsWindow());
        HIMAGELIST himl = Header_GetImageList(*this);
        return CImageList(himl);
    }


    // Retrieves information about an item in a header control.
    inline BOOL CHeader::GetItem(int nPos, HDITEM& HeaderItem) const
    {
        assert(IsWindow());
        return Header_GetItem(*this, nPos, &HeaderItem);
    }


    // Retrieves the count of the items in a header control.
    inline int CHeader::GetItemCount() const
    {
        assert(IsWindow());
        return Header_GetItemCount(*this);
    }


    // Retrieves the bounding rectangle for a given item in a header control.
    inline CRect CHeader::GetItemRect(int nIndex) const
    {
        assert(IsWindow());
        CRect rc;
        SendMessage(HDM_GETITEMRECT, static_cast<WPARAM>(nIndex), reinterpret_cast<LPARAM>(&rc));
        return rc;
    }


    // Retrieves the current left-to-right order of items in a header control.
    inline BOOL CHeader::GetOrderArray(LPINT piArray, int iCount) const
    {
        assert(IsWindow());
        return Header_GetOrderArray(*this, piArray, iCount);
    }


    // Inserts a new item into the header control.
    inline int CHeader::InsertItem(int nPos, const HDITEM& hdi) const
    {
        assert(IsWindow());
        return Header_InsertItem(*this, nPos, &hdi);
    }


    // Retrieves the correct size and position of the header control within the parent window.
    inline BOOL CHeader::Layout(HDLAYOUT* pHeaderLayout) const
    {
        assert(IsWindow());
        return Header_Layout(*this, pHeaderLayout);
    }


    // Retrieves an index value for an item based on its order in the header control.
    inline int CHeader::OrderToIndex(int nOrder) const
    {
        assert(IsWindow());
        return Header_OrderToIndex( *this, nOrder);
    }

#ifdef Header_SetHotDivider

    // Changes the color of a divider between header items to indicate the destination of
    // an external drag-and-drop operation.
    inline int CHeader::SetHotDivider(CPoint pt) const
    {
        assert(IsWindow());
        return Header_SetHotDivider(*this, TRUE, MAKELPARAM(pt.x, pt.y));
    }


    // Changes the color of a divider between header items to indicate the destination of
    // an external drag-and-drop operation.
    inline int CHeader::SetHotDivider(int nIndex) const
    {
        assert(IsWindow());
        return Header_SetHotDivider(*this, FALSE, nIndex);
    }

#endif

    // Assigns an image list to the header control.
    inline CImageList CHeader::SetImageList(HIMAGELIST himlNew) const
    {
        assert(IsWindow());
        HIMAGELIST himl = Header_SetImageList(*this, himlNew);
        return CImageList(himl);
    }


    // Sets the attributes of the specified item in a header control.
    inline BOOL CHeader::SetItem(int nPos, const HDITEM& HeaderItem) const
    {
        assert(IsWindow());
        return Header_SetItem(*this, nPos, &HeaderItem);
    }


    // Sets the left-to-right order of header items.
    inline BOOL CHeader::SetOrderArray(int iCount, LPINT piArray) const
    {
        assert(IsWindow());
        return Header_SetOrderArray(*this, iCount, piArray);
    }

#ifdef Header_ClearFilter

    // Clears the filter for the header control.
    inline int CHeader::ClearFilter(int nColumn) const
    {
        assert(IsWindow());
        return Header_ClearFilter(*this, nColumn);
    }


    // Clears all of the filters for the header control.
    inline int CHeader::ClearAllFilters() const
    {
        assert(IsWindow());
        return Header_ClearAllFilters(*this);
    }


    // Moves the input focus to the edit box when a filter button has the focus.
    inline int CHeader::EditFilter(int nColumn, BOOL DiscardChanges) const
    {
        assert(IsWindow());
        return Header_EditFilter(*this, nColumn, MAKELPARAM(DiscardChanges, 0));
    }


    // Gets the width of the bitmap margin for a header control.
    inline int CHeader::GetBitmapMargin() const
    {
        assert(IsWindow());
        return Header_GetBitmapMargin(*this);
    }


    // Sets the width of the margin, specified in pixels, of a bitmap in the header control.
    inline int CHeader::SetBitmapMargin(int nWidth) const
    {
        assert(IsWindow());
        return Header_SetBitmapMargin(*this, nWidth);
    }


    // Sets the timeout interval between the time a change takes place in the filter attributes
    // and the posting of an HDN_FILTERCHANGE notification.
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
            InitStruct.dwSize = sizeof(InitStruct);
            InitStruct.dwICC = ICC_INTERNET_CLASSES;
            InitCommonControlsEx(&InitStruct);
        }
        else
            throw CNotSupportedException(_T("IP Address Control not supported!"));
    }


    // Clears the contents of the IP address control.
    inline void CIPAddress::ClearAddress() const
    {
        assert(IsWindow());
        SendMessage(IPM_CLEARADDRESS, 0L, 0L);
    }


    // Gets the address values for all four fields in the IP address control.
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


    // Gets the address values for all four fields in the IP address control.
    inline int CIPAddress::GetAddress(DWORD& dwAddress) const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(IPM_GETADDRESS, 0L, reinterpret_cast<LPARAM>(&dwAddress)));
    }


    // Determines if all fields in the IP address control are blank.
    inline BOOL CIPAddress::IsBlank() const
    {
        assert(IsWindow());
        return static_cast<BOOL>(SendMessage(IPM_ISBLANK, 0L, 0L));
    }


    // Sets the address values for all four fields in the IP address control.
    inline void CIPAddress::SetAddress(BYTE nField0, BYTE nField1, BYTE nField2, BYTE nField3) const
    {
        assert(IsWindow());
        SendMessage(IPM_SETADDRESS, 0L, MAKEIPADDRESS(nField0, nField1, nField2, nField3));
    }


    // Sets the address values for all four fields in the IP address control.
    inline void CIPAddress::SetAddress(DWORD dwAddress) const
    {
        assert(IsWindow());
        SendMessage(IPM_SETADDRESS, 0L, dwAddress);
    }


    // Sets the keyboard focus to the specified field in the IP address control.
    // All of the text in that field will be selected.
    inline void CIPAddress::SetFieldFocus(WORD nField) const
    {
        assert(IsWindow());
        SendMessage(IPM_SETFOCUS, nField, 0L);
    }


    // Sets the valid range for the specified field in the IP address control.
    inline void CIPAddress::SetFieldRange(int nField, BYTE nLower, BYTE nUpper) const
    {
        assert(IsWindow());
        SendMessage(IPM_SETRANGE, MAKEIPRANGE(nLower, nUpper), nField);
    }


    ///////////////////////////////////////////
    // Definitions for the CMonthCalendar class
    //

    inline CMonthCalendar::CMonthCalendar()
    {
    }


    // Retrieves the color for a given portion of the month calendar control.
    inline COLORREF CMonthCalendar::GetColor(int nRegion) const
    {
        assert(IsWindow());
        return static_cast<COLORREF>(MonthCal_GetColor(*this, nRegion));
    }


    // Retrieves the currently selected date.
    inline SYSTEMTIME CMonthCalendar::GetCurSel() const
    {
        assert(IsWindow());
        SYSTEMTIME st;
        ZeroMemory(&st, sizeof(st));
        MonthCal_GetCurSel(*this, &st);
        return st;
    }


    // Retrieves the first day of the week.
    inline int CMonthCalendar::GetFirstDayOfWeek(BOOL* pbLocal /*= NULL*/) const
    {
        assert(IsWindow());
        DWORD dwValue = MonthCal_GetFirstDayOfWeek(*this);

        if (pbLocal)
            *pbLocal = HIWORD(dwValue);

        return LOWORD(dwValue);
    }


    // Retrieves the maximum date range that can be selected.
    inline int CMonthCalendar::GetMaxSelCount() const
    {
        assert(IsWindow());
        return MonthCal_GetMaxSelCount(*this);
    }


    // Retrieves the minimum size required to display a full month
    inline CRect CMonthCalendar::GetMinReqRect() const
    {
        assert(IsWindow());
        CRect rc;
        MonthCal_GetMinReqRect(*this, &rc);
        return rc;
    }


    // Retrieves the scroll rate for the month calendar control.
    inline int CMonthCalendar::GetMonthDelta() const
    {
        assert(IsWindow());
        return MonthCal_GetMonthDelta(*this);
    }


    // Retrieves date information that represents the high and low limits of the month calendar control's display.
    inline int CMonthCalendar::GetMonthRange(SYSTEMTIME& MinRange, SYSTEMTIME& MaxRange, DWORD dwFlags) const
    {
        assert(IsWindow());
        SYSTEMTIME MinMax[2];
        ZeroMemory(MinMax, 2*sizeof(SYSTEMTIME));
        int nCount = MonthCal_GetMonthRange(*this, dwFlags, MinMax);
        MinRange = MinMax[0];
        MaxRange = MinMax[1];
        return nCount;
    }


    //  Retrieves the minimum and maximum allowable dates set for the month calendar control.
    inline LRESULT CMonthCalendar::GetRange(SYSTEMTIME& MinRange, SYSTEMTIME& MaxRange) const
    {
        assert(IsWindow());
        SYSTEMTIME MinMax[2];
        ZeroMemory(MinMax, 2*sizeof(SYSTEMTIME));
        LRESULT Value = MonthCal_GetRange(*this, &MinMax);
        MinRange = MinMax[0];
        MaxRange = MinMax[1];
        return Value;
    }


    // Retrieves date information that represents the upper and lower limits of the date range currently selected by the user.
    inline LRESULT CMonthCalendar::GetSelRange(SYSTEMTIME& MinRange, SYSTEMTIME& MaxRange) const
    {
        assert(IsWindow());
        SYSTEMTIME MinMax[2];
        ZeroMemory(MinMax, 2*sizeof(SYSTEMTIME));
        LRESULT Value = MonthCal_GetSelRange(*this, &MinMax);
        MinRange = MinMax[0];
        MaxRange = MinMax[1];
        return Value;
    }


    // Retrieves the date information for the date specified as "today" for the month calendar control.
    inline SYSTEMTIME CMonthCalendar::GetToday() const
    {
        assert(IsWindow());
        SYSTEMTIME DateTime;
        ZeroMemory(&DateTime, sizeof(DateTime));
        VERIFY(MonthCal_GetToday(*this, &DateTime));
        return DateTime;
    }


    //  Determines which portion of the month calendar control is at a given point on the screen.
    inline LRESULT CMonthCalendar::HitTest(MCHITTESTINFO& MCHitTest) const
    {
        assert(IsWindow());
        return MonthCal_HitTest(*this, &MCHitTest);
    }


    // Sets the color for a given portion of the month calendar control.
    inline COLORREF CMonthCalendar::SetColor(int nRegion, COLORREF clr) const
    {
        assert(IsWindow());
        return static_cast<COLORREF>(MonthCal_SetColor(*this, nRegion, clr));
    }


    // Sets the currently selected date for the month calendar control.
    inline BOOL CMonthCalendar::SetCurSel(const SYSTEMTIME& DateTime) const
    {
        assert(IsWindow());
        return MonthCal_SetCurSel(*this, &DateTime);
    }


    // Sets the day states for all months that are currently visible within the month calendar control.
    inline BOOL CMonthCalendar::SetDayState(int nMonths, const MONTHDAYSTATE& State) const
    {
        assert(IsWindow());
        return static_cast<BOOL>(MonthCal_SetDayState(*this, nMonths, &State));
    }


    // Sets the first day of the week for the month calendar control.
    inline BOOL CMonthCalendar::SetFirstDayOfWeek(int iDay, int* pnOld/* = NULL*/) const
    {
        assert(IsWindow());
        DWORD dwValue = static_cast<DWORD>(MonthCal_SetFirstDayOfWeek(*this, iDay));

        if(pnOld)
            *pnOld = LOWORD(dwValue);

        return static_cast<BOOL>(HIWORD(dwValue));
    }


    // Sets the maximum number of days that can be selected in the month calendar control.
    inline BOOL CMonthCalendar::SetMaxSelCount(int nMax) const
    {
        assert(IsWindow());
        return MonthCal_SetMaxSelCount(*this, nMax);
    }


    // Sets the scroll rate for the month calendar control.
    inline int CMonthCalendar::SetMonthDelta(int iDelta) const
    {
        assert(IsWindow());
        return MonthCal_SetMonthDelta(*this, iDelta);
    }


    // Sets the minimum and maximum allowable dates for the month calendar control.
    inline BOOL CMonthCalendar::SetRange(const SYSTEMTIME& MinRange, const SYSTEMTIME& MaxRange) const
    {
        SYSTEMTIME MinMax[2];
        DWORD dwLimit = GDTR_MIN | GDTR_MAX;

        MinMax[0] = MinRange;
        MinMax[1] = MaxRange;

        return static_cast<BOOL>(MonthCal_SetRange(*this, dwLimit, &MinMax));
    }


    // Sets the selection for the month calendar control to a given date range.
    inline BOOL CMonthCalendar::SetSelRange(const SYSTEMTIME& MinRange, const SYSTEMTIME& MaxRange) const
    {
        SYSTEMTIME MinMax[2];
        MinMax[0] = MinRange;
        MinMax[1] = MaxRange;
        return static_cast<BOOL>(MonthCal_SetSelRange(*this, &MinMax));
    }


    // Sets the "today" selection for the month calendar control.
    inline void CMonthCalendar::SetToday(const SYSTEMTIME& DateTime) const
    {
        assert(IsWindow());
        MonthCal_SetToday(*this, &DateTime);
    }


    ////////////////////////////////////////
    // Definitions for the CProgressBar class
    //

    // Retrieves the current position of the progress bar.
    inline int CProgressBar::GetPos() const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(PBM_GETPOS, 0L, 0L));
    }


    // Retrieves information about the current high and low limits of the progress bar control.
    inline int CProgressBar::GetRange(BOOL WhichLimit, const PBRANGE& PBRange) const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(PBM_GETRANGE, WhichLimit, reinterpret_cast<LPARAM>(&PBRange)));
    }


    // Retrieves information about the current high and low limits of the progress bar control.
    inline int CProgressBar::GetRange(BOOL WhichLimit) const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(PBM_GETRANGE, WhichLimit, 0L));
    }


    // Advances the current position of the progress bar by a specified increment and redraws
    // the bar to reflect the new position.
    inline int CProgressBar::OffsetPos(int nIncrement) const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(PBM_DELTAPOS, nIncrement, 0L));
    }


    // Sets the current position for the progress bar and redraws the bar to reflect the new position.
    inline int CProgressBar::SetPos(int nNewPos) const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(PBM_SETPOS, nNewPos, 0L));
    }


    // Sets the minimum and maximum values for the progress bar and redraws the bar to reflect the new range.
    inline int CProgressBar::SetRange(short nMinRange, short nMaxRange) const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(PBM_SETRANGE, 0L, MAKELPARAM(nMinRange, nMaxRange)));
    }


    // Specifies the step increment for the progress bar.
    inline int CProgressBar::SetStep(int nStepInc) const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(PBM_SETSTEP, nStepInc, 0L));
    }


    // Advances the current position for a progress bar by the step increment and
    // redraws the bar to reflect the new position.
    inline int CProgressBar::StepIt() const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(PBM_STEPIT, 0L, 0L));
    }


    ////////////////////////////////////////
    // Definitions for the CScrollBar class
    //

    // Enables or disables the scroll bar arrows.
    inline BOOL CScrollBar::EnableScrollBar( UINT nArrowFlags )  const
    {
        assert(IsWindow());
        return ::EnableScrollBar(*this, SB_CTL, nArrowFlags);
    }


    // Retrieves the parameters of a scroll bar, including the minimum and maximum
    // scrolling positions, the page size, and the position of the scroll box (thumb).
    inline BOOL CScrollBar::GetScrollInfo(SCROLLINFO& si)  const
    {
        assert(IsWindow());
        return ::GetScrollInfo(*this, SB_CTL, &si);
    }


    // Retrieves the current position of the scroll box (thumb) in the scroll bar.
    inline int CScrollBar::GetScrollPos()  const
    {
        assert(IsWindow());
        return ::GetScrollPos(*this, SB_CTL);
    }


    // Retrieves the current minimum and maximum scroll box (thumb) positions for the scroll bar.
    inline BOOL CScrollBar::GetScrollRange(int& MinPos, int& MaxPos )  const
    {
        assert(IsWindow());
        return ::GetScrollRange(*this, SB_CTL, &MinPos, &MaxPos);
    }


    // Sets the parameters of the scroll bar, including the minimum and maximum scrolling positions,
    // the page size, and the position of the scroll box (thumb).
    inline BOOL CScrollBar::SetScrollInfo(const SCROLLINFO& si, BOOL Redraw )  const
    {
        assert(IsWindow());
        return ::SetScrollInfo(*this, SB_CTL, &si, Redraw);
    }


    // Sets the position of the scroll box (thumb) in the scroll bar and, if requested,
    // redraws the scroll bar to reflect the new position of the scroll box.
    inline int CScrollBar::SetScrollPos(int nPos, BOOL Redraw)  const
    {
        assert(IsWindow());
        return ::SetScrollPos(*this, SB_CTL, nPos, Redraw);
    }


    // Sets the minimum and maximum scroll box positions for the scroll bar.
    inline BOOL CScrollBar::SetScrollRange( int nMinPos, int nMaxPos, BOOL Redraw )  const
    {
        assert(IsWindow());
        return ::SetScrollRange(*this, SB_CTL, nMinPos, nMaxPos, Redraw);
    }


    // Shows or hides the scroll bar.
    inline BOOL CScrollBar::ShowScrollBar(BOOL Show)  const
    {
        assert(IsWindow());
        return ::ShowScrollBar(*this, SB_CTL, Show);
    }

    ////////////////////////////////////////
    // Definitions for the CSlider class
    //

    // Clears the current selection range in the trackbar.
    inline void CSlider::ClearSel() const
    {
        assert(IsWindow());
        SendMessage(TBM_CLEARSEL, 0L, 0L);
    }


    // Removes the current tick marks from the trackbar.
    inline void CSlider::ClearTics(BOOL Redraw) const
    {
        assert(IsWindow());
        SendMessage(TBM_CLEARTICS, Redraw, 0L);
    }


    // Retrieves the handle to the trackbar control buddy window at a given location.
    inline HWND CSlider::GetBuddy(BOOL fLocation) const
    {
        assert(IsWindow());
        return reinterpret_cast<HWND>(SendMessage(TBM_GETBUDDY, fLocation, 0L));
    }


    // Retrieves the size and position of the bounding rectangle for the trackbar's channel.
    inline CRect CSlider::GetChannelRect() const
    {
        assert(IsWindow());
        CRect rc;
        SendMessage(TBM_GETCHANNELRECT, 0L,reinterpret_cast<LPARAM>(&rc));
        return rc;
    }


    // Retrieves the number of logical positions the trackbar's slider moves in response
    // to keyboard input from the arrow keys.
    inline int  CSlider::GetLineSize() const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(TBM_GETLINESIZE, 0L, 0L));
    }


    // Retrieves the number of tick marks in the trackbar.
    inline int  CSlider::GetNumTics() const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(TBM_GETNUMTICS, 0L, 0L));
    }


    // Retrieves the number of logical positions the trackbar's slider moves in response to
    // keyboard input, or mouse input, such as clicks in the trackbar's channel.
    inline int  CSlider::GetPageSize() const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(TBM_GETPAGESIZE, 0L, 0L));
    }


    // Retrieves the current logical position of the slider in the trackbar.
    inline int  CSlider::GetPos() const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(TBM_GETPOS, 0L, 0L));
    }


    // Retrieves the maximum position for the slider in the trackbar.
    inline int  CSlider::GetRangeMax() const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(TBM_GETRANGEMAX, 0L, 0L));
    }


    // Retrieves the minimum position for the slider in the trackbar.
    inline int  CSlider::GetRangeMin() const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(TBM_GETRANGEMIN, 0L, 0L));
    }


    // Retrieves the ending position of the current selection range in the trackbar.
    inline int  CSlider::GetSelEnd() const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(TBM_GETSELEND, 0L, 0L));
    }


    // Retrieves the starting position of the current selection range in the trackbar.
    inline int  CSlider::GetSelStart() const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(TBM_GETSELSTART, 0L, 0L));
    }


    // Retrieves the length of the slider in the trackbar.
    inline int  CSlider::GetThumbLength() const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(TBM_GETTHUMBLENGTH, 0L, 0L));
    }


    // Retrieves the size and position of the bounding rectangle for the slider in the trackbar.
    inline CRect CSlider::GetThumbRect() const
    {
        CRect rc;
        SendMessage(TBM_GETTHUMBRECT, 0L, reinterpret_cast<LPARAM>(&rc));
        return rc;
    }


    // Retrieves the logical position of a tick mark in the trackbar.
    inline int CSlider::GetTic(int nTic ) const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(TBM_GETTIC, nTic, 0L));
    }


    // Retrieves the current physical position of a tick mark in the trackbar.
    inline int  CSlider::GetTicPos(int nTic) const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(TBM_GETTICPOS, nTic, 0L));
    }


    // Retrieves the handle to the ToolTip control assigned to the trackbar, if any.
    inline HWND CSlider::GetToolTips() const
    {
        assert(IsWindow());
        return reinterpret_cast<HWND>(SendMessage(TBM_GETTOOLTIPS, 0L, 0L));
    }


    // Assigns a window as the buddy window for the trackbar control.
    inline HWND CSlider::SetBuddy(HWND hBuddy, BOOL fLocation /*= TRUE*/ ) const
    {
        assert(IsWindow());
        return reinterpret_cast<HWND>(SendMessage(TBM_SETBUDDY, fLocation, reinterpret_cast<LPARAM>(hBuddy)));
    }


    // Sets the number of logical positions the trackbar's slider moves in response to
    // keyboard input from the arrow keys.
    inline int  CSlider::SetLineSize(int nSize) const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(TBM_SETLINESIZE, 0L, nSize));
    }


    // Sets the number of logical positions the trackbar's slider moves in response to
    // keyboard input, or mouse input such as clicks in the trackbar's channel.
    inline int  CSlider::SetPageSize(int nSize) const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(TBM_SETPAGESIZE, 0L, nSize));
    }


    // Sets the current logical position of the slider in the trackbar.
    inline void CSlider::SetPos(int nPos, BOOL Redraw) const
    {
        assert(IsWindow());
        SendMessage(TBM_SETPOS, Redraw, nPos);
    }


    // Sets the maximum logical position for the slider in the trackbar.
    inline void CSlider::SetRangeMax(int nMax, BOOL Redraw) const
    {
        assert(IsWindow());
        SendMessage(TBM_SETRANGEMAX, Redraw, nMax);
    }


    // Sets the minimum logical position for the slider in the trackbar.
    inline void CSlider::SetRangeMin(int nMin, BOOL Redraw) const
    {
        assert(IsWindow());
        SendMessage(TBM_SETRANGEMIN, Redraw, nMin);
    }


    // Sets the starting and ending positions for the available selection range in the trackbar.
    inline void CSlider::SetSelection(int nMin, int nMax, BOOL Redraw) const
    {
        assert(IsWindow());
        SendMessage(TBM_SETSEL, Redraw, MAKELONG(nMax, nMin));
    }


    // Sets a tick mark in the trackbar at the specified logical position.
    inline BOOL CSlider::SetTic(int nTic) const
    {
        assert(IsWindow());
        return static_cast<BOOL>(SendMessage(TBM_SETTIC, 0L, nTic));
    }


    // Sets the interval frequency for tick marks in the trackbar.
    inline void CSlider::SetTicFreq(int nFreq)  const
    {
        assert(IsWindow());
        SendMessage(TBM_SETTICFREQ, nFreq, 0L);
    }


    // Positions a ToolTip control used by the trackbar control.
    inline int  CSlider::SetTipSide(int nLocation) const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(TBM_SETTIPSIDE, nLocation, 0L));
    }


    // Assigns a ToolTip control to the trackbar control.
    inline void CSlider::SetToolTips(HWND hToolTip) const
    {
        assert(IsWindow());
        SendMessage(TBM_SETTOOLTIPS, reinterpret_cast<WPARAM>(hToolTip), 0L);
    }

    ////////////////////////////////////////
    // Definitions for the CSpinButton class
    //

    // Retrieves acceleration information for the up-down control.
    inline int CSpinButton::GetAccel(int cAccels, LPUDACCEL paAccels) const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(UDM_GETACCEL, cAccels, reinterpret_cast<LPARAM>(paAccels)));
    }


    // Retrieves the current radix base (that is, either base 10 or 16) for the up-down control.
    inline int CSpinButton::GetBase() const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(UDM_GETBASE, 0L, 0L));
    }


    // Retrieves the handle to the current buddy window.
    inline HWND CSpinButton::GetBuddy() const
    {
        assert(IsWindow());
        return reinterpret_cast<HWND>(SendMessage(UDM_GETBUDDY, 0L, 0L));
    }


    // Retrieves the current position of the up-down control with 16-bit precision.
    inline int CSpinButton::GetPos() const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(UDM_GETPOS, 0L, 0L));
    }


    // Retrieves the minimum and maximum positions (range) for the up-down control.
    inline DWORD CSpinButton::GetRange() const
    {
        assert(IsWindow());
        return static_cast<DWORD>(SendMessage(UDM_GETRANGE, 0L, 0L));
    }


    // Sets the CREATESTRUCT parameters before the control is created.
    inline void CSpinButton::PreCreate(CREATESTRUCT& cs)
    {
        cs.style = WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VISIBLE |UDS_SETBUDDYINT;
    }


    // Sets the WNDCLASS parameters before the control is created.
    inline void CSpinButton::PreRegisterClass(WNDCLASS& wc)
    {
        wc.lpszClassName = UPDOWN_CLASS;
    }


    // Sets the acceleration for the up-down control.
    inline BOOL CSpinButton::SetAccel(int cAccels, LPUDACCEL paAccels) const
    {
        assert(IsWindow());
        return static_cast<BOOL>(SendMessage(UDM_SETACCEL, cAccels, reinterpret_cast<LPARAM>(paAccels)));
    }


    // Sets the radix base for the up-down control.
    inline int CSpinButton::SetBase(int nBase) const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(UDM_SETBASE, nBase, 0L));
    }


    // Sets the buddy window for the up-down control.
    inline HWND CSpinButton::SetBuddy(HWND hBuddy) const
    {
        assert(IsWindow());
        return reinterpret_cast<HWND>(SendMessage(UDM_SETBUDDY, reinterpret_cast<WPARAM>(hBuddy), 0L));
    }


    // Sets the current position for the up-down control with 16-bit precision.
    inline int CSpinButton::SetPos(int nPos) const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(UDM_SETPOS, 0L, MAKELONG ((short) nPos, 0)));
    }


    // Sets the minimum and maximum positions (range) for the up-down control.
    inline void CSpinButton::SetRange(int nLower, int nUpper) const
    {
        assert(IsWindow());
        SendMessage(UDM_SETRANGE, 0L, MAKELONG(nUpper, nLower));
    }

    ////////////////////////////////////////
    // Definitions for the CToolTip class
    //

    inline CToolTip::CToolTip()
    {
    }

    inline CToolTip::~CToolTip()
    {
    }


    // Activates or deactivates a ToolTip control.
    inline void CToolTip::Activate(BOOL Activate) const
    {
        assert(IsWindow());
        SendMessage(TTM_ACTIVATE, Activate, 0L);
    }


    // Registers a tool with a ToolTip control.
    // hWndControl specifies the window which triggers the tooltip.
    // rcTool specifies the part of the window which triggers the tooltip.
    // nIDText specifies the ID of the text resource.
    // uID is a user defined ID. It is required if the control has multiple tooltips.
    inline BOOL CToolTip::AddTool(HWND hWndControl, const RECT& rcTool, UINT uID, UINT nIDText) const
    {
        assert(IsWindow());
        TOOLINFO ti;
        FillToolInfo(ti, hWndControl, rcTool, uID);
        ti.hinst = GetApp().GetResourceHandle();
        ti.lpszText = MAKEINTRESOURCE(nIDText);
        return static_cast<BOOL>(SendMessage(TTM_ADDTOOL, 0L, reinterpret_cast<LPARAM>(&ti)));
    }


    // Registers a tool with a ToolTip control.
    // hWndControl specifies the window which triggers the tooltip.
    // nIDText specifies the ID of the text resource
    inline BOOL CToolTip::AddTool(HWND hWndControl, UINT nIDText) const
    {
        assert(IsWindow());
        TOOLINFO ti;
        FillToolInfo(ti, hWndControl);
        ti.hinst = GetApp().GetResourceHandle();
        ti.lpszText = MAKEINTRESOURCE(nIDText);
        return static_cast<BOOL>(SendMessage(TTM_ADDTOOL, 0L, reinterpret_cast<LPARAM>(&ti)));
    }


    // Registers a tool with a ToolTip control.
    // hWndControl specifies the window which triggers the tooltip.
    // rcTool specifies the part of the window which triggers the tooltip.
    // If lpszText contains the value LPSTR_TEXTCALLBACK, TTN_NEEDTEXT notification
    // messages are sent to the parent window.
    // uID is a user defined ID. It is required if the control has multiple tooltips.
    inline BOOL CToolTip::AddTool(HWND hWndControl, const RECT& rcTool, UINT uID, LPCTSTR lpszText /*= LPSTR_TEXTCALLBACK*/) const
    {
        assert(IsWindow());
        TOOLINFO ti;
        FillToolInfo(ti, hWndControl, rcTool, uID);
        ti.lpszText = const_cast<LPTSTR>(lpszText);
        return static_cast<BOOL>(SendMessage(TTM_ADDTOOL, 0L, reinterpret_cast<LPARAM>(&ti)));
    }


    // Registers a tool with a ToolTip control.
    // hWndControl specifies the window which triggers the tooltip.
    // If lpszText contains the value LPSTR_TEXTCALLBACK, TTN_NEEDTEXT notification
    // messages are sent to the parent window.
    inline BOOL CToolTip::AddTool(HWND hWndControl, LPCTSTR lpszText /*= LPSTR_TEXTCALLBACK*/) const
    {
        assert(IsWindow());
        TOOLINFO ti;
        FillToolInfo(ti, hWndControl);
        ti.lpszText = const_cast<LPTSTR>(lpszText);
        return static_cast<BOOL>(SendMessage(TTM_ADDTOOL, 0L, reinterpret_cast<LPARAM>(&ti)));
    }


    // Removes a tool from a ToolTip control.
    inline void CToolTip::DelTool(HWND hWndControl, UINT uID) const
    {
        assert(IsWindow());
        TOOLINFO ti = GetToolInfo(hWndControl, uID);
        SendMessage(TTM_DELTOOL, 0L, reinterpret_cast<LPARAM>(&ti));
    }


    // Retrieves the initial, pop-up, and reshow durations currently set for a ToolTip control.
    // Returns an intvalue with the specified duration in milliseconds.
    //
    // dwDuration is one of:
    //  TTDT_AUTOPOP - time the ToolTip window remains visible if the pointer is stationary
    //  TTDT_INITIAL - time the pointer must remain stationary before the ToolTip window appears.
    //  TTDT_RESHOW  - time it takes for subsequent ToolTip windows to appear as the pointer moves from one tool to another.
    inline int CToolTip::GetDelayTime(DWORD dwDuration) const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(TTM_GETDELAYTIME, dwDuration, 0L));
    }


    // Retrieves the top, left, bottom, and right margins set for a ToolTip window.
    inline CRect CToolTip::GetMargin() const
    {
        assert(IsWindow());
        CRect rc;
        SendMessage(TTM_GETMARGIN, 0L, reinterpret_cast<LPARAM>(&rc));
        return rc;
    }


    // Retrieves the maximum width for a ToolTip window.
    inline int CToolTip::GetMaxTipWidth() const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(TTM_GETMAXTIPWIDTH, 0L, 0L));
    }


    // Retrieves the text information a ToolTip control maintains about a tool.
    inline CString CToolTip::GetText(HWND hWndControl, UINT uID) const
    {
        assert(IsWindow());
        CString str;
        TOOLINFO ti = GetToolInfo(hWndControl, uID);

        LPTSTR pszText = str.GetBuffer(80); // Maximum allowed ToolTip is 80 characters for Windows XP and below
        ti.lpszText = pszText;
        SendMessage(TTM_GETTEXT, 0L, reinterpret_cast<LPARAM>(&ti));
        str.ReleaseBuffer();

        return str;
    }


    // Retrieves the background color in a ToolTip window.
    inline COLORREF CToolTip::GetTipBkColor() const
    {
        assert(IsWindow());
        return static_cast<COLORREF>(SendMessage(TTM_GETTIPBKCOLOR, 0L, 0L));
    }


    // Retrieves the text color in a ToolTip window.
    inline COLORREF CToolTip::GetTipTextColor() const
    {
        assert(IsWindow());
        return static_cast<COLORREF>(SendMessage(TTM_GETTIPTEXTCOLOR, 0L, 0L));
    }


    // Retrieves a count of the tools maintained by a ToolTip control.
    inline int CToolTip::GetToolCount() const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(TTM_GETTOOLCOUNT, 0L, 0L));
    }


    // Retrieves the information that a ToolTip control maintains about a tool.
    // hWndControl is the control specified when the tooltip was added with AddTool.
    // uID is the user ID (if any) specified when the tooltip was added with AddTool.
    // Note:
    // The TOOLINFO struct returned does not contain the ToolTip's text.
    // Use GetText to retrieve a tool's text.
    inline TOOLINFO CToolTip::GetToolInfo(HWND hWndControl, UINT uID) const
    {
        assert(IsWindow());
        TOOLINFO ti;
        ZeroMemory(&ti, sizeof(ti));
        ti.cbSize = sizeof(ti);
        if (uID == static_cast<UINT>(-1))
        {
            ti.hwnd = GetParent();
            ti.uId = (UINT_PTR)hWndControl;
        }
        else
        {
            ti.hwnd = hWndControl;
            ti.uId = uID;
        }

        VERIFY(SendMessage(TTM_GETTOOLINFO, 0L, reinterpret_cast<LPARAM>(&ti)));

        return ti;
    }


    // Tests a point to determine whether it is within the bounding rectangle of the
    //  specified tool and, if it is, retrieves information about the tool.
    inline BOOL CToolTip::HitTest(HWND hWnd, CPoint pt, const TOOLINFO& ToolInfo) const
    {
        assert(IsWindow());
        TTHITTESTINFO hti;
        ZeroMemory(&hti, sizeof(hti));
        hti.hwnd = hWnd;
        hti.pt = pt;
        hti.ti = ToolInfo;
        return static_cast<BOOL>(SendMessage(TTM_HITTEST, 0L, reinterpret_cast<LPARAM>(&hti)));
    }


    // Fills the TOOLINFO structure. Used by AddTool.
    // Notes:
    // 1) Notifications are passed to the parent window.
    // 2) The control is always identified by its hwnd.
    // 3) The tooltip always manages its messages (uses TTF_SUBCLASS).
    // Override this function to specify different uFlags.
    inline void CToolTip::FillToolInfo(TOOLINFO& ti, HWND hControl) const
    {
        ZeroMemory(&ti, sizeof(ti));
        ti.cbSize = sizeof(ti);

        ti.hwnd = ::GetParent(m_hWnd);  // pass notifications to the parent window
        ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
        ti.uId = (UINT_PTR)hControl;
    }


    // Fills the TOOLINFO structure. Used by AddTool.
    // Notes:
    // 1) Notifications are passed to the hControl window, not the parent window.
    // 2) The control is always identified by its hwnd.
    // 3) rc specifies which part of the control triggers the tooltip.
    // 4) A unique uID is required if the control has multiple tooltips.
    // 5) The tooltip always manages its messages (uses TTF_SUBCLASS).
    // 6) The TTF_IDISHWND style is incompatible with using a RECT.
    // Override this function to specify different uFlags.
    inline void CToolTip::FillToolInfo(TOOLINFO& ti, HWND hControl, const RECT& rc, UINT uID) const
    {
        ZeroMemory(&ti, sizeof(ti));
        ti.cbSize = sizeof(ti);

        ti.hwnd = hControl;
        ti.uFlags = TTF_SUBCLASS;
        ti.rect = rc;
        ti.uId = uID;
    }


    // Removes a displayed ToolTip window from view.
    inline void CToolTip::Pop() const
    {
        assert(IsWindow());
        SendMessage(TTM_POP, 0L, 0L);
    }


    // Sets the CREATESTRUCT parameters before the tooltip is created.
    inline void CToolTip::PreCreate(CREATESTRUCT& cs)
    {
        cs.dwExStyle = WS_EX_TOOLWINDOW;
        cs.style = WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP;
    }


    // Sets the WNDCLASS parameters before the tooltip is created.
    inline void  CToolTip::PreRegisterClass(WNDCLASS& wc)
    {
        wc.lpszClassName = TOOLTIPS_CLASS;
    }


    // Passes a mouse message to a ToolTip control for processing.
    inline void CToolTip::RelayEvent(MSG& Msg) const
    {
        assert(IsWindow());
        SendMessage(TTM_RELAYEVENT, 0L, reinterpret_cast<LPARAM>(&Msg));
    }


    // Sets the initial delay for a ToolTip control.
    inline void CToolTip::SetDelayTime(UINT nDelay) const
    {
        assert(IsWindow());
        SendMessage(TTM_SETDELAYTIME, TTDT_INITIAL, nDelay);
    }


    // Sets the initial, pop-up, and reshow durations for a ToolTip control.
    inline void CToolTip::SetDelayTime(DWORD dwDuration, int iTime) const
    {
        assert(IsWindow());
        SendMessage(TTM_SETDELAYTIME, dwDuration, iTime);
    }


    // Sets the top, left, bottom, and right margins for a ToolTip window.
    inline void CToolTip::SetMargin(const RECT& rc) const

    {
        assert(IsWindow());
        SendMessage(TTM_SETMARGIN, 0L, reinterpret_cast<LPARAM>(&rc));
    }


    // Sets the maximum width for a ToolTip window in pixels.
    inline int CToolTip::SetMaxTipWidth(int iWidth) const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(TTM_SETMAXTIPWIDTH, 0L, iWidth));
    }


    // Sets the background color in a ToolTip window.
    // Ignored when XP themes are active.
    inline void CToolTip::SetTipBkColor(COLORREF clr) const
    {
        assert(IsWindow());
        SendMessage(TTM_SETTIPBKCOLOR, clr, 0L);
    }


    // Sets the text color in a ToolTip window.
    // Ignored when XP themes are active.
    inline void CToolTip::SetTipTextColor(COLORREF clr) const
    {
        assert(IsWindow());
        SendMessage(TTM_SETTIPTEXTCOLOR, clr, 0L);
    }


    // Sets the information that a ToolTip control maintains for a tool.
    inline void CToolTip::SetToolInfo(const TOOLINFO& ToolInfo) const
    {
        assert(IsWindow());
        SendMessage(TTM_SETTOOLINFO, 0L, reinterpret_cast<LPARAM>(&ToolInfo));
    }


    // Sets a new bounding rectangle for a tool.
    inline void CToolTip::SetToolRect(const RECT& rc, HWND hWndControl, UINT uID) const
    {
        assert(IsWindow());
        TOOLINFO ti = GetToolInfo(hWndControl, uID);
        ti.rect = rc;
        SendMessage(TTM_NEWTOOLRECT, 0L, reinterpret_cast<LPARAM>(&ti));
    }


    // Forces the current tool to be redrawn.
    inline void CToolTip::Update() const
    {
        assert(IsWindow());
        SendMessage(TTM_UPDATE, 0L, 0L);
    }


    // Sets the ToolTip text for a tool.
    inline void CToolTip::UpdateTipText(LPCTSTR lpszText, HWND hWndControl, UINT uID) const
    {
        assert(IsWindow());
        TOOLINFO ti = GetToolInfo(hWndControl, uID);
        ti.lpszText = const_cast<LPTSTR>(lpszText);
        SendMessage(TTM_UPDATETIPTEXT, 0L, reinterpret_cast<LPARAM>(&ti));
    }


    // Sets the ToolTip text for a tool.
    inline void CToolTip::UpdateTipText(UINT nIDText, HWND hWndControl, UINT uID) const
    {
        assert(IsWindow());
        TOOLINFO ti = GetToolInfo(hWndControl, uID);
        ti.lpszText = MAKEINTRESOURCE(nIDText);
        SendMessage(TTM_UPDATETIPTEXT, 0L, reinterpret_cast<LPARAM>(&ti));
    }

#if (_WIN32_IE >=0x0500)

    // Calculates a ToolTip control's text display rectangle from its window rectangle, or the
    // ToolTip window rectangle needed to display a specified text display rectangle.
    inline BOOL CToolTip::AdjustRect(RECT& rc, BOOL IsLarger /*= TRUE*/) const
    {
        assert(IsWindow());
        return static_cast<BOOL>(SendMessage(TTM_ADJUSTRECT, IsLarger, reinterpret_cast<LPARAM>(&rc)));
    }


    // Returns the width and height of a ToolTip control.
    inline CSize CToolTip::GetBubbleSize(HWND hWndControl, UINT uID) const
    {
        assert(IsWindow());
        TOOLINFO ti = GetToolInfo(hWndControl, uID);
        LRESULT lr = SendMessage(TTM_GETBUBBLESIZE, 0L, reinterpret_cast<LPARAM>(&ti));
        CSize sz(LOWORD(lr), HIWORD(lr));
        return sz;
    }

#ifdef TTM_SETTITLE

    // Adds a standard icon and title string to a ToolTip.
    inline BOOL CToolTip::SetTitle(UINT uIcon, LPCTSTR lpstrTitle) const

    {
        assert(IsWindow());
        return static_cast<BOOL>(SendMessage(TTM_SETTITLE, uIcon, reinterpret_cast<LPARAM>(lpstrTitle)));
    }

#endif
#endif

#if (WINVER >= 0x0501) && defined(TTM_SETWINDOWTHEME)

    // Sets the visual style of a ToolTip control.
    inline void CToolTip::SetTTWindowTheme(LPCWSTR lpstrTheme) const
    {
        assert(IsWindow());
        SendMessage(TTM_SETWINDOWTHEME, 0L, reinterpret_cast<LPARAM>(lpstrTheme));
    }

#endif



} // namespace Win32xx

#endif //  define _WIN32XX_CONTROLS_H_

