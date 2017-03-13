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
// wxx_stdcontrols.h
//  Declaration of the CButton, CEdit, CListBox and CStatic classes

// The Button, Edit, ListBox and Static controls are often referred to 
// as "standard controls". These set of older controls were originally 
// developed for Win16 operating systems (Windows 3.1 and 3.11). They use an
// older form of notification, and send their notifications via a WM_COMMAND
// message. Newer controls send their notifications via a WM_NOTIFY message.


#ifndef _WIN32XX_STDCONTROLS_H_
#define _WIN32XX_STDCONTROLS_H_

#include "wxx_wincore.h"


namespace Win32xx
{
	class CButton : public CWnd
	{
	public:
		CButton() {}
		virtual ~CButton() {}

		// Attributes
		HBITMAP GetBitmap() const;
		UINT GetButtonStyle() const;
		int GetCheck() const;
		HCURSOR GetCursor() const;
		HICON GetIcon() const;
		UINT GetState() const;
		HBITMAP SetBitmap(HBITMAP hBitmap) const;
		void SetButtonStyle(DWORD dwStyle, BOOL Redraw) const;
		void SetCheck(int nCheckState) const;
		HCURSOR SetCursor(HCURSOR hCursor) const;
		HICON SetIcon(HICON hIcon) const;
		void SetState(BOOL IsHighlighted) const;

	protected:
		// Overridables
		virtual void PreRegisterClass(WNDCLASS& wc);

	private:
		CButton(const CButton&);				// Disable copy construction
		CButton& operator = (const CButton&);	// Disable assignment operator
	};

	class CEdit : public CWnd
	{
	public:
		// Construction
		CEdit() {}
		virtual ~CEdit() {}

		// Attributes
		BOOL CanUndo() const;
		int CharFromPos(CPoint pt) const;
		int GetFirstVisibleLine() const;
		HLOCAL GetHandle() const;
		UINT GetLimitText() const;
		int GetLine(int nIndex, LPTSTR lpszBuffer) const;
		int GetLine(int nIndex, LPTSTR lpszBuffer, int nMaxLength) const;
		int GetLineCount() const;
		DWORD GetMargins() const;
		BOOL GetModify() const;
		TCHAR GetPasswordChar() const;
		void GetRect(RECT& rc) const;
		void GetSel(int& nStartChar, int& nEndChar) const;
		DWORD GetSel() const;
		CPoint PosFromChar(UINT nChar) const;
		void SetHandle(HLOCAL hBuffer) const;
		void SetLimitText(UINT nMax) const;
		void SetMargins(UINT nLeft, UINT nRight) const;
		void SetModify(BOOL IsModified = TRUE) const;

		// Operations
		void EmptyUndoBuffer() const;
		BOOL FmtLines(BOOL AddEOL) const;
		void LimitText(int nChars = 0) const;
		int LineFromChar(int nIndex = -1) const;
		int LineIndex(int nLine = -1) const;
		int LineLength(int nLine = -1) const;
		void LineScroll(int nLines, int nChars = 0) const;
		void ReplaceSel(LPCTSTR lpszNewText, BOOL CanUndo) const;
		void SetPasswordChar(TCHAR ch) const;
		BOOL SetReadOnly(BOOL IsReadOnly = TRUE) const;
		void SetRect(const RECT& rc) const;
		void SetRectNP(const RECT& rc) const;
		void SetSel(DWORD dwSelection, BOOL NotScrolled) const;
		void SetSel(int nStartChar, int nEndChar, BOOL NotScrolled) const;
		BOOL SetTabStops(int nTabStops, LPINT rgTabStops) const;
		BOOL SetTabStops() const;
		BOOL SetTabStops(const int& cxEachStop) const;

		//Clipboard Operations
		void Clear() const;
		void Copy() const;
		void Cut() const;
		void Paste() const;
		void Undo() const;

	protected:
		// Overridables
		virtual void PreRegisterClass(WNDCLASS& wc);

	private:
		CEdit(const CEdit&);				// Disable copy construction
		CEdit& operator = (const CEdit&);	// Disable assignment operator
	};

	class CListBox : public CWnd
	{
	public:
		CListBox() {}
		virtual ~CListBox() {}

		// General Operations
		int  GetCount() const;
		int  GetHorizontalExtent() const;
		DWORD GetItemData(int nIndex) const;
		void* GetItemDataPtr(int nIndex) const;
		int  GetItemHeight(int nIndex) const;
		int  GetItemRect(int nIndex, RECT& rc) const;
		LCID GetLocale() const;
		int  GetSel(int nIndex) const;
		int  GetText(int nIndex, LPTSTR lpszBuffer) const;
		int  GetTextLen(int nIndex) const;
		int  GetTopIndex() const;
		UINT ItemFromPoint(CPoint pt, BOOL& bOutside ) const;
		void SetColumnWidth(int cxWidth) const;
		void SetHorizontalExtent(int cxExtent) const;
		int  SetItemData(int nIndex, DWORD dwItemData) const;
		int  SetItemDataPtr(int nIndex, void* pData) const;
		int  SetItemHeight(int nIndex, UINT cyItemHeight) const;
		LCID SetLocale(LCID nNewLocale) const;
		BOOL SetTabStops(int nTabStops, LPINT rgTabStops) const;
		void SetTabStops() const;
		BOOL SetTabStops(const int& cxEachStop) const;
		int  SetTopIndex(int nIndex) const;

		// Single-Selection Operations
		int  GetCurSel() const;
		int  SetCurSel(int nSelect) const;

		// Multiple-Selection Operations
		int  GetAnchorIndex() const;
		int  GetCaretIndex() const;
		int  GetSelCount() const;
		int  GetSelItems(int nMaxItems, LPINT rgIndex) const;
		int  SelItemRange(BOOL IsSelected, int nFirstItem, int nLastItem) const;
		void SetAnchorIndex(int nIndex) const;
		int  SetCaretIndex(int nIndex, BOOL PartialScroll) const;
		int  SetSel(int nIndex, BOOL IsSelected) const;

		// String Operations
		int  AddString(LPCTSTR lpszItem) const;
		int  DeleteString(UINT nIndex) const;
		int  Dir(UINT attr, LPCTSTR lpszWildCard) const;
		int  FindString(int nStartAfter, LPCTSTR lpszItem) const;
		int  FindStringExact(int nIndexStart, LPCTSTR lpszFind) const;
		int  InsertString(int nIndex, LPCTSTR lpszItem) const;
		void ResetContent() const;
		int  SelectString(int nStartAfter, LPCTSTR lpszItem) const;

	protected:
		// Overridables
		virtual void PreRegisterClass(WNDCLASS& wc);

	private:
		CListBox(const CListBox&);				// Disable copy construction
		CListBox& operator = (const CListBox&);	// Disable assignment operator
	};

	class CStatic : public CWnd
	{
	public:
		CStatic() {}
		virtual ~CStatic() {}

		// Operations
		HBITMAP  GetBitmap() const;
		HCURSOR GetCursor() const;
		HENHMETAFILE GetEnhMetaFile() const;
		HICON  GetIcon() const;
		HBITMAP SetBitmap(HBITMAP hBitmap) const;
		HCURSOR SetCursor(HCURSOR hCursor) const;
		HENHMETAFILE SetEnhMetaFile(HENHMETAFILE hMetaFile) const;
		HICON SetIcon(HICON hIcon) const;

	protected:
		// Overridables
		virtual void PreRegisterClass(WNDCLASS& wc);

	private:
		CStatic(const CStatic&);				// Disable copy construction
		CStatic& operator = (const CStatic&);	// Disable assignment operator
	};

}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


namespace Win32xx
{

	////////////////////////////////////////
	// Definitions for the CButton class
	//
	inline HBITMAP CButton::GetBitmap() const
	// returns the handle to the bitmap associated with the button
	{
		assert(IsWindow());
		return reinterpret_cast<HBITMAP>(SendMessage(BM_GETIMAGE, IMAGE_BITMAP, 0L));
	}

	inline UINT CButton::GetButtonStyle() const
	// returns the style of the button
	{
		assert(IsWindow());
		return static_cast<UINT>(GetWindowLongPtr(GWL_STYLE) & 0xFFFF);
	}

	inline int CButton::GetCheck() const
	// returns the check state of the button
	{
		assert(IsWindow());
		return static_cast<int>(SendMessage(BM_GETCHECK, 0L, 0L));
	}

	inline HCURSOR CButton::GetCursor() const
	// returns the handle to the cursor associated with the button
	{
		assert(IsWindow());
		return reinterpret_cast<HCURSOR>(SendMessage(BM_GETIMAGE, IMAGE_CURSOR, 0L));
	}

	inline HICON CButton::GetIcon() const
	// returns the handle to the icon associated with the button
	{
		assert(IsWindow());
		return reinterpret_cast<HICON>(SendMessage(BM_GETIMAGE, IMAGE_ICON, 0L));
	}

	inline UINT CButton::GetState() const
	// returns the state of the button
	{
		assert(IsWindow());
		return static_cast<UINT>(SendMessage(BM_GETSTATE, 0L, 0L));
	}

	inline HBITMAP CButton::SetBitmap(HBITMAP hBitmap) const
	// sets the bitmap associated with the button
	{
		assert(IsWindow());
		return reinterpret_cast<HBITMAP>(SendMessage(BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBitmap));
	}

	inline void CButton::SetButtonStyle(DWORD dwStyle, BOOL Redraw) const
	// sets the button style
	{
		assert(IsWindow());
		SendMessage(BM_SETSTYLE, dwStyle, Redraw);
	}

	inline void CButton::SetCheck(int nCheckState) const
	// sets the button check state
	{
		assert(IsWindow());
		SendMessage(BM_SETCHECK, (WPARAM)nCheckState, 0L);
	}

	inline HCURSOR CButton::SetCursor(HCURSOR hCursor) const
	// sets the cursor associated with the button
	{
		assert(IsWindow());
		return reinterpret_cast<HCURSOR>(SendMessage(STM_SETIMAGE, IMAGE_CURSOR, (LPARAM)hCursor));
	}

	inline HICON CButton::SetIcon(HICON hIcon) const
	// sets the icon associated with the button
	{
		assert(IsWindow());
		return reinterpret_cast<HICON>(SendMessage( BM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon));
	}

	inline void CButton::SetState(BOOL IsHighlighted) const
	// sets the button state
	{
		assert(IsWindow());
		SendMessage(BM_SETSTATE, IsHighlighted, 0L);
	}

	inline void CButton::PreRegisterClass(WNDCLASS& wc)
	{
		// Set the Window Class
		wc.lpszClassName =  _T("Button");
	}


	////////////////////////////////////////
	// Definitions for the CEdit class
	//
	inline BOOL CEdit::CanUndo() const
	// Returns TRUE if the edit control operation can be undone.
	{
		assert(IsWindow());
		return static_cast<BOOL>(SendMessage(EM_CANUNDO, 0L, 0L));
	}

	inline int CEdit::CharFromPos(CPoint pt) const
	// Returns the character index and line index of the character nearest the specified point.
	{
		assert(IsWindow());
		return static_cast<int>(SendMessage(EM_CHARFROMPOS, 0L, MAKELPARAM(pt.x, pt.y)));
	}

	inline int CEdit::GetFirstVisibleLine() const
	// Returns the zero-based index of the first visible character in a single-line edit control 
	//  or the zero-based index of the uppermost visible line in a multiline edit control.
	{
		assert(IsWindow());
		return static_cast<int>(SendMessage(EM_GETFIRSTVISIBLELINE, 0L, 0L));
	}

	inline HLOCAL CEdit::GetHandle() const
	// Returns a handle identifying the buffer containing the multiline edit control's text. 
	//  It is not processed by single-line edit controls.
	{
		assert(IsWindow());
		return reinterpret_cast<HLOCAL>(SendMessage(EM_GETHANDLE, 0L, 0L));
	}

	inline UINT CEdit::GetLimitText() const
	// Returns the current text limit, in characters.
	{
		assert(IsWindow());
		return static_cast<UINT>(SendMessage(EM_GETLIMITTEXT, 0L, 0L));
	}

	inline int CEdit::GetLine(int nIndex, LPTSTR lpszBuffer) const
	// Copies characters to a buffer and returns the number of characters copied.
	{
		assert(IsWindow());
		return static_cast<int>(SendMessage(EM_GETLINE, (WPARAM)nIndex, (LPARAM)lpszBuffer));
	}

	inline int CEdit::GetLine(int nIndex, LPTSTR lpszBuffer, int nMaxLength) const
	// Copies characters to a buffer and returns the number of characters copied.
	{
		assert(IsWindow());
		*(LPWORD)lpszBuffer = static_cast<WORD>(nMaxLength);
		return static_cast<int>(SendMessage(EM_GETLINE, (WPARAM)nIndex, (LPARAM)lpszBuffer));
	}

	inline int CEdit::GetLineCount() const
	// Returns the number of lines in the edit control.
	{
		assert(IsWindow());
		return static_cast<int>(SendMessage(EM_GETLINECOUNT, 0L, 0L));
	}

	inline DWORD CEdit::GetMargins() const
	// Returns the widths of the left and right margins.
	{
		assert(IsWindow());
		return static_cast<DWORD>(SendMessage(EM_GETMARGINS, 0L, 0L));
	}

	inline BOOL CEdit::GetModify() const
	// Returns a flag indicating whether the content of an edit control has been modified.
	{
		assert(IsWindow());
		return static_cast<BOOL>(SendMessage(EM_GETMODIFY, 0L, 0L));
	}

	inline TCHAR CEdit::GetPasswordChar() const
	// Returns the character that edit controls use in conjunction with the ES_PASSWORD style.
	{
		assert(IsWindow());
		return static_cast<TCHAR>(SendMessage(EM_GETPASSWORDCHAR, 0L, 0L));
	}

	inline void CEdit::GetRect(RECT& rc) const
	// Returns the coordinates of the formatting rectangle in an edit control.
	{
		assert(IsWindow());
		SendMessage(EM_GETRECT, 0L, (LPARAM)&rc);
	}

	inline void CEdit::GetSel(int& nStartChar, int& nEndChar) const
	// Returns the starting and ending character positions of the current selection in the edit control.
	{
		assert(IsWindow());
		SendMessage(EM_GETSEL, (WPARAM)&nStartChar,(LPARAM)&nEndChar);
	}

	inline DWORD CEdit::GetSel() const
	// Returns the starting and ending character positions of the current selection in the edit control.
	{
		assert(IsWindow());
		return static_cast<DWORD>(SendMessage(EM_GETSEL, 0L, 0L));
	}

	inline CPoint CEdit::PosFromChar(UINT nChar) const
	// Returns the client coordinates of the specified character.
	{
		assert(IsWindow());
		return CPoint( static_cast<DWORD_PTR>(SendMessage(EM_POSFROMCHAR, (WPARAM)nChar, 0L)) );
	}

	inline void CEdit::SetHandle(HLOCAL hBuffer) const
	// Sets a handle to the memory used as a text buffer, empties the undo buffer, 
	//  resets the scroll positions to zero, and redraws the window.
	{
		assert(IsWindow());
		SendMessage(EM_SETHANDLE, (WPARAM)hBuffer, 0L);
	}

	inline void CEdit::SetLimitText(UINT nMax) const
	// Sets the maximum number of characters the user may enter in the edit control.
	{
		assert(IsWindow());
		SendMessage(EM_SETLIMITTEXT, (WPARAM)nMax, 0L);
	}

	inline void CEdit::SetMargins(UINT nLeft, UINT nRight) const
	// Sets the widths of the left and right margins, and redraws the edit control to reflect the new margins.
	{
		assert(IsWindow());
		SendMessage(EM_SETMARGINS, EC_LEFTMARGIN|EC_RIGHTMARGIN, MAKELONG(nLeft, nRight));
	}

	inline void CEdit::SetModify(BOOL IsModified) const
	// Sets or clears the modification flag to indicate whether the edit control has been modified.
	{
		assert(IsWindow());
		SendMessage(EM_SETMODIFY, IsModified, 0L);
	}

	inline void CEdit::EmptyUndoBuffer() const
	// Empties the undo buffer and sets the undo flag retrieved by the EM_CANUNDO message to FALSE.
	{
		assert(IsWindow());
		SendMessage(EM_EMPTYUNDOBUFFER, 0L, 0L);
	}

	inline BOOL CEdit::FmtLines(BOOL AddEOL) const
	// Adds or removes soft line-break characters (two carriage returns and a line feed) to the ends of wrapped lines 
	//  in a multiline edit control. It is not processed by single-line edit controls.
	{
		assert(IsWindow());
		return static_cast<BOOL>(SendMessage(EM_FMTLINES, AddEOL, 0L));
	}

	inline void CEdit::LimitText(int nChars) const
	// Sets the text limit of an edit control. The text limit is the maximum amount of text, in TCHARs, 
	//  that the user can type into the edit control.
	{
		assert(IsWindow());
		SendMessage(EM_LIMITTEXT, (WPARAM)nChars, 0L);
	}

	inline int CEdit::LineFromChar(int nIndex) const
	// Returns the zero-based number of the line in a multiline edit control that contains a specified character index.
	//  This message is the reverse of the EM_LINEINDEX message.
	{
		assert(IsWindow());
		return static_cast<int>(SendMessage(EM_LINEFROMCHAR, (WPARAM)nIndex, 0L));
	}

	inline int CEdit::LineIndex(int nLine) const
	// Returns the character of a line in a multiline edit control. 
	// This message is the reverse of the EM_LINEFROMCHAR message
	{
		assert(IsWindow());
		return static_cast<int>(SendMessage(EM_LINEINDEX, (WPARAM)nLine, 0L));
	}

	inline int CEdit::LineLength(int nLine) const
	// Returns the length, in characters, of a single-line edit control. In a multiline edit control, 
	//	returns the length, in characters, of a specified line.
	{
		assert(IsWindow());
		return static_cast<int>(SendMessage(EM_LINELENGTH, (WPARAM)nLine, 0L));
	}

	inline void CEdit::LineScroll(int nLines, int nChars) const
	// Scrolls the text vertically in a single-line edit control or horizontally in a multiline edit control.
	{
		assert(IsWindow()); 
		SendMessage(EM_LINESCROLL, (WPARAM)nChars, (LPARAM)nLines);
	}

	inline void CEdit::ReplaceSel(LPCTSTR lpszNewText, BOOL CanUndo) const
	// Replaces the current selection with the text in an application-supplied buffer, sends the parent window 
	//  EN_UPDATE and EN_CHANGE messages, and updates the undo buffer.
	{
		assert(IsWindow());
		SendMessage(EM_REPLACESEL, (WPARAM) CanUndo, (LPARAM)lpszNewText);
	}

	inline void CEdit::SetPasswordChar(TCHAR ch) const
	// Defines the character that edit controls use in conjunction with the ES_PASSWORD style.
	{
		assert(IsWindow());
		SendMessage(EM_SETPASSWORDCHAR, (WPARAM)ch, 0L);
	}

	inline BOOL CEdit::SetReadOnly(BOOL IsReadOnly) const
	// Sets or removes the read-only style (ES_READONLY) in an edit control.
	{
		assert(IsWindow());
		return static_cast<BOOL>(SendMessage(EM_SETREADONLY, IsReadOnly, 0L));
	}

	inline void CEdit::SetRect(const RECT& rc) const
	// Sets the formatting rectangle for the multiline edit control and redraws the window.
	{
		assert(IsWindow());
		SendMessage(EM_SETRECT, 0L, (LPARAM)&rc);
	}

	inline void CEdit::SetRectNP(const RECT& rc) const
	// Sets the formatting rectangle for the multiline edit control but does not redraw the window.
	{
		assert(IsWindow());
		SendMessage(EM_SETRECTNP, 0L, (LPARAM)&rc);
	}

	inline void CEdit::SetSel(DWORD dwSelection, BOOL NotScrolled) const
	// Selects a range of characters in the edit control by setting the starting and ending positions to be selected.
	{
		assert(IsWindow());
		SendMessage(EM_SETSEL, LOWORD(dwSelection), HIWORD(dwSelection));
		if (!NotScrolled)
			SendMessage(EM_SCROLLCARET, 0L, 0L);
	}

	inline void CEdit::SetSel(int nStartChar, int nEndChar, BOOL NotScrolled) const
	// Selects a range of characters in the edit control by setting the starting and ending positions to be selected.
	{
		assert(IsWindow());
		SendMessage(EM_SETSEL, nStartChar, nEndChar);
		if (!NotScrolled)
			SendMessage(EM_SCROLLCARET, 0L, 0L);
	}

	inline BOOL CEdit::SetTabStops(int nTabStops, LPINT rgTabStops) const
	// Sets tab-stop positions in the multiline edit control.
	{
		assert(IsWindow());
		return static_cast<BOOL>(SendMessage(EM_SETTABSTOPS, (WPARAM)nTabStops, (LPARAM)rgTabStops));
	}

	inline BOOL CEdit::SetTabStops() const
	// Sets tab-stop positions in the multiline edit control.
	{
		assert(IsWindow());
		return static_cast<BOOL>(SendMessage( EM_SETTABSTOPS, 0L, 0L));
	}

	inline BOOL CEdit::SetTabStops(const int& cxEachStop) const
	// Sets tab-stop positions in the multiline edit control.
	{
		assert(IsWindow());
		return static_cast<BOOL>(SendMessage(EM_SETTABSTOPS, (WPARAM)1, (LPARAM)(LPINT)&cxEachStop));
	}

	inline void CEdit::Clear() const
	// Clears the current selection, if any, in an edit control.
	{
		assert(IsWindow());
		SendMessage(WM_CLEAR, 0L, 0L);
	}

	inline void CEdit::Copy() const
	// Copies text to the clipboard unless the style is ES_PASSWORD, in which case the message returns zero.
	{
		assert(IsWindow());
		SendMessage(WM_COPY, 0L, 0L);
	}

	inline void CEdit::Cut() const
	// Cuts the selection to the clipboard, or deletes the character to the left of the cursor if there is no selection.
	{
		assert(IsWindow());
		SendMessage(WM_CUT, 0L, 0L);
	}

	inline void CEdit::Paste() const
	// Pastes text from the clipboard into the edit control window at the caret position.
	{
		assert(IsWindow());
		SendMessage(WM_PASTE, 0L, 0L);
	}

	inline void CEdit::Undo() const
	// Removes any text that was just inserted or inserts any deleted characters and sets the selection to the inserted text.
	{
		assert(IsWindow());
		SendMessage(EM_UNDO, 0L, 0L);
	}

	inline void CEdit::PreRegisterClass(WNDCLASS& wc)
	{
		// Set the Window Class
		wc.lpszClassName =  _T("Edit");
	}


	////////////////////////////////////////
	// Definitions for the CListbox class
	//
	inline int CListBox::GetCount() const
	// Returns the number of items in the list box.
	{
		assert(IsWindow());
		return static_cast<int>(SendMessage(LB_GETCOUNT, 0L, 0L));
	}

	inline int CListBox::GetHorizontalExtent() const
	// Returns the scrollable width, in pixels, of a list box.
	{
		assert(IsWindow());
		return static_cast<int>(SendMessage(LB_GETHORIZONTALEXTENT,	0L, 0L));
	}

	inline DWORD CListBox::GetItemData(int nIndex) const
	// Returns the value associated with the specified item.
	{
		assert(IsWindow());
		return static_cast<DWORD>(SendMessage(LB_GETITEMDATA, (WPARAM)nIndex, 0L));
	}

	inline void* CListBox::GetItemDataPtr(int nIndex) const
	// Returns the value associated with the specified item.
	{
		assert(IsWindow());
		return reinterpret_cast<LPVOID>(SendMessage(LB_GETITEMDATA, (WPARAM)nIndex, 0L));
	}

	inline int CListBox::GetItemHeight(int nIndex) const
	// Returns the height, in pixels, of an item in a list box.
	{
		assert(IsWindow());
		return static_cast<int>(SendMessage(LB_GETITEMHEIGHT, (WPARAM)nIndex, 0L));
	}

	inline int CListBox::GetItemRect(int nIndex, RECT& rc) const
	// Retrieves the client coordinates of the specified list box item.
	{
		assert(IsWindow());
		return static_cast<int>(SendMessage(LB_GETITEMRECT, (WPARAM)nIndex, (LPARAM)&rc));
	}

	inline LCID CListBox::GetLocale() const
	// Retrieves the locale of the list box. The high-order word contains the country/region code 
	//  and the low-order word contains the language identifier.
	{
		assert(IsWindow());
		return static_cast<LCID>(SendMessage(LB_GETLOCALE, 0L, 0L));
	}

	inline int CListBox::GetSel(int nIndex) const
	// Returns the selection state of a list box item.
	{
		assert(IsWindow());
		return static_cast<int>(SendMessage(LB_GETSEL, (WPARAM)nIndex, 0L));
	}

	inline int CListBox::GetText(int nIndex, LPTSTR lpszBuffer) const
	// Retrieves the string associated with a specified item and the length of the string.
	{
		assert(IsWindow());
		return static_cast<int>(SendMessage(LB_GETTEXT, (WPARAM)nIndex, (LPARAM)lpszBuffer));
	}

	inline int CListBox::GetTextLen(int nIndex) const
	// Returns the length, in characters, of the string associated with a specified item.
	{
		assert(IsWindow());
		return static_cast<int>(SendMessage( LB_GETTEXTLEN, (WPARAM)nIndex, 0L));
	}

	inline int CListBox::GetTopIndex() const
	// Returns the index of the first visible item in a list box.
	{
		assert(IsWindow());
		return static_cast<int>(SendMessage(LB_GETTOPINDEX, 0L, 0L));
	}

	inline UINT CListBox::ItemFromPoint(CPoint pt, BOOL& bOutside) const
	// Retrieves the zero-based index of the item nearest the specified point in a list box.
	{
		assert(IsWindow());
		DWORD dw = static_cast<DWORD>(SendMessage(LB_ITEMFROMPOINT, 0L, MAKELPARAM(pt.x, pt.y)));
		bOutside = !!HIWORD(dw);
		return LOWORD(dw);
	}

	inline void CListBox::SetColumnWidth(int cxWidth) const
	// Sets the width, in pixels, of all columns in a list box.
	{
		assert(IsWindow());
		SendMessage(LB_SETCOLUMNWIDTH, (WPARAM)cxWidth, 0L);
	}

	inline void CListBox::SetHorizontalExtent(int cxExtent) const
	// Sets the scrollable width, in pixels, of a list box.
	{
		assert(IsWindow());
		SendMessage(LB_SETHORIZONTALEXTENT, (WPARAM)cxExtent, 0L);
	}

	inline int CListBox::SetItemData(int nIndex, DWORD dwItemData) const
	// Associates a value with a list box item.
	{
		assert(IsWindow());
		return static_cast<int>(SendMessage(LB_SETITEMDATA, (WPARAM)nIndex, (LPARAM)dwItemData));
	}

	inline int CListBox::SetItemDataPtr(int nIndex, void* pData) const
	// Associates a value with a list box item.
	{
		assert(IsWindow());
		return static_cast<int>(SendMessage(LB_SETITEMDATA, (WPARAM)nIndex, (LPARAM)pData));
	}

	inline int CListBox::SetItemHeight(int nIndex, UINT cyItemHeight) const
	// Sets the height, in pixels, of an item or items in a list box.
	{
		assert(IsWindow());
		return static_cast<int>(SendMessage(LB_SETITEMHEIGHT, (WPARAM)nIndex, MAKELONG(cyItemHeight, 0)));
	}

	inline LCID CListBox::SetLocale(LCID nNewLocale) const
	// Sets the locale of a list box and returns the previous locale identifier.
	{
		assert(IsWindow());
		return static_cast<LCID>(SendMessage(LB_SETLOCALE, (WPARAM)nNewLocale, 0L));
	}

	inline BOOL CListBox::SetTabStops(int nTabStops, LPINT rgTabStops) const
	// Sets the tab stops to those specified in a specified array.
	{
		assert(IsWindow());
		return static_cast<BOOL>(SendMessage(LB_SETTABSTOPS, (WPARAM)nTabStops, (LPARAM)rgTabStops));
	}

	inline void CListBox::SetTabStops() const
	// Sets the tab stops to those specified in a specified array.
	{
		assert(IsWindow());
		SendMessage(LB_SETTABSTOPS, 0L, 0L);
	}

	inline BOOL CListBox::SetTabStops(const int& cxEachStop) const
	// Sets the tab stops to those specified in a specified array.
	{
		assert(IsWindow());
		return static_cast<BOOL>(SendMessage(LB_SETTABSTOPS, 1, (LPARAM)(LPINT)&cxEachStop));
	}

	inline int CListBox::SetTopIndex(int nIndex) const
	// Scrolls the list box so the specified item is at the top of the visible range.
	{
		assert(IsWindow());
		return static_cast<int>(SendMessage(LB_SETTOPINDEX, (WPARAM)nIndex, 0L));
	}

	inline int CListBox::GetCurSel() const
	// Returns the index of the currently selected item.
	{
		assert(IsWindow());
		return static_cast<int>(SendMessage(LB_GETCURSEL, 0L, 0L));
	}

	inline int CListBox::SetCurSel(int nSelect) const
	// Selects a specified list box item.
	{
		assert(IsWindow());
		return static_cast<int>(SendMessage(LB_SETCURSEL, (WPARAM)nSelect, 0L));
	}

	inline int CListBox::GetAnchorIndex() const
	// Returns the index of the item that the mouse last selected.
	{
		assert(IsWindow());
		return static_cast<int>(SendMessage(LB_GETANCHORINDEX, 0L, 0L));
	}

	inline int CListBox::GetCaretIndex() const
	// Returns the index of the item that has the focus rectangle.
	{
		assert(IsWindow());
		return static_cast<int>(SendMessage(LB_GETCARETINDEX, 0L, 0L));
	}

	inline int CListBox::GetSelCount() const
	// Returns the number of selected items in a multiple-selection list box.
	{
		assert(IsWindow());
		return static_cast<int>(SendMessage(LB_GETSELCOUNT, 0L, 0L));
	}

	inline int CListBox::GetSelItems(int nMaxItems, LPINT rgIndex) const
	// Creates an array of the indexes of all selected items in a multiple-selection list box 
	//  and returns the total number of selected items.
	{
		assert(IsWindow());
		return static_cast<int>(SendMessage(LB_GETSELITEMS, (WPARAM)nMaxItems, (LPARAM)rgIndex));
	}

	inline int CListBox::SelItemRange(BOOL IsSelected, int nFirstItem, int nLastItem) const
	// Selects a specified range of items in a list box.
	{
		assert(IsWindow());
		if (IsSelected)
			return static_cast<int>(SendMessage(LB_SELITEMRANGEEX, (WPARAM)nFirstItem, (LPARAM)nLastItem));
		else
			return static_cast<int>(SendMessage(LB_SELITEMRANGEEX, (WPARAM)nLastItem, (LPARAM)nFirstItem));
	}

	inline void CListBox::SetAnchorIndex(int nIndex) const
	// Sets the item that the mouse last selected to a specified item.
	{
		assert(IsWindow());
		SendMessage(LB_SETANCHORINDEX, (WPARAM)nIndex, 0L);
	}

	inline int CListBox::SetCaretIndex(int nIndex, BOOL PartialScroll) const
	// Sets the focus rectangle to a specified list box item.
	{
		assert(IsWindow());
		return static_cast<int>(SendMessage(LB_SETCARETINDEX, (WPARAM)nIndex, MAKELONG(PartialScroll, 0)));
	}

	inline int CListBox::SetSel(int nIndex, BOOL IsSelected) const
	// Selects an item in a multiple-selection list box.
	{
		assert(IsWindow());
		return static_cast<int>(SendMessage(LB_SETSEL, IsSelected, (LPARAM)nIndex));
	}

	inline int CListBox::AddString(LPCTSTR lpszItem) const
	// Adds a string to a list box and returns its index.
	{
		assert(IsWindow());
		return static_cast<int>(SendMessage(LB_ADDSTRING, 0L, (LPARAM)lpszItem));
	}

	inline int CListBox::DeleteString(UINT nIndex) const
	// Removes a string from a list box and returns the number of strings remaining in the list.
	{
		assert(IsWindow());
		return static_cast<int>(SendMessage(LB_DELETESTRING, (WPARAM)nIndex, 0L));
	}

	inline int CListBox::Dir(UINT attr, LPCTSTR lpszWildCard) const
	// Adds a list of filenames to a list box and returns the index of the last filename added.
	{
		assert(IsWindow());
		return static_cast<int>(SendMessage(LB_DIR, (WPARAM)attr, (LPARAM)lpszWildCard));
	}

	inline int CListBox::FindString(int nStartAfter, LPCTSTR lpszItem) const
	// Returns the index of the first string in the list box that begins with a specified string.
	{
		assert(IsWindow());
		return static_cast<int>(SendMessage(LB_FINDSTRING, (WPARAM)nStartAfter, (LPARAM)lpszItem));
	}

	inline int CListBox::FindStringExact(int nIndexStart, LPCTSTR lpszFind) const
	// Returns the index of the string in the list box that is equal to a specified string.
	{
		assert(IsWindow());
		return static_cast<int>(SendMessage(LB_FINDSTRINGEXACT, (WPARAM)nIndexStart, (LPARAM)lpszFind));
	}

	inline int CListBox::InsertString(int nIndex, LPCTSTR lpszItem) const
	// Inserts a string at a specified index in a list box.
	{
		assert(IsWindow());
		return static_cast<int>(SendMessage(LB_INSERTSTRING, (WPARAM)nIndex, (LPARAM)lpszItem));
	}

	inline void CListBox::ResetContent() const
	// Removes all items from a list box.
	{
		assert(IsWindow());
		SendMessage(LB_RESETCONTENT, 0L, 0L);
	}

	inline int CListBox::SelectString(int nStartAfter, LPCTSTR lpszItem) const
	// Selects the first string it finds that matches a specified prefix.
	{
		assert(IsWindow());
		return static_cast<int>(SendMessage(LB_SELECTSTRING, (WPARAM)nStartAfter, (LPARAM)lpszItem));
	}

	inline void CListBox::PreRegisterClass(WNDCLASS& wc)
	{
		// Set the Window Class
		wc.lpszClassName =  _T("ListBox");
	}


	////////////////////////////////////////
	// Definitions for the CStatic class
	//
	inline HBITMAP CStatic::GetBitmap() const
	// Returns the handle to the bitmap for the static control set by CStatic::SetBitmap
	{
		assert(IsWindow());
		return reinterpret_cast<HBITMAP>(SendMessage(STM_GETIMAGE, IMAGE_BITMAP, 0L));
	}

	inline HCURSOR CStatic::GetCursor() const
	// Returns the handle to the cursor for the static control set by CStatic::SetCursor
	{
		assert(IsWindow());
		return reinterpret_cast<HCURSOR>(SendMessage(STM_GETIMAGE, IMAGE_CURSOR, 0L));
	}

	inline HENHMETAFILE CStatic::GetEnhMetaFile() const
	// Returns the handle to the enhanced meta-file for the static control set by CStatic::SetEnhMetaFile
	{
		assert(IsWindow());
		return reinterpret_cast<HENHMETAFILE>(SendMessage(STM_GETIMAGE, IMAGE_ENHMETAFILE, 0L));
	}

	inline HICON CStatic::GetIcon() const
	// Returns the handle to the icon for the static control set by CStatic::SetIcon
	{
		assert(IsWindow());
		return reinterpret_cast<HICON>(SendMessage(STM_GETIMAGE, IMAGE_ICON, 0L));
	}

	inline HBITMAP CStatic::SetBitmap(HBITMAP hBitmap) const
	// Associates a new bitmap with the static control.
	// The bitmap will be drawn in the upper-left corner, and the static 
	//  control will be resized to the size of the bitmap.
	// This function requires the SS_BITMAP style
	{
		assert(IsWindow());
		return reinterpret_cast<HBITMAP>(SendMessage(STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBitmap));
	}

	inline HCURSOR CStatic::SetCursor(HCURSOR hCursor) const
	// Associates a new cursor image with the static control. 
	// The cursor will be drawn in the upper-left corner and the static 
	//  control will be resized to the size of the cursor. 
	// This function requires the SS_ICON style
	{
		assert(IsWindow());
		return reinterpret_cast<HCURSOR>(SendMessage(STM_SETIMAGE, IMAGE_CURSOR, (LPARAM)hCursor));
	}

	inline HENHMETAFILE CStatic::SetEnhMetaFile(HENHMETAFILE hMetaFile) const
	// Associates a new enhanced metafile image with the static control. 
	// The enhanced metafile will be drawn in the upper-left corner and the static 
	//  control will be resized to the size of the enhanced metafile. 
	// This function requires the SS_ENHMETAFILE style
	{
		assert(IsWindow());
		return reinterpret_cast<HENHMETAFILE>(SendMessage(STM_SETIMAGE, IMAGE_ENHMETAFILE, (LPARAM)hMetaFile));
	}

	inline HICON CStatic::SetIcon(HICON hIcon) const
	// Associates a new icon image with the static control. 
	// The icon will be drawn in the upper-left corner and the static 
	//  control will be resized to the size of the icon. 
	// This function requires the SS_ICON style
	{
		assert(IsWindow());
		return reinterpret_cast<HICON>(SendMessage(STM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon));
	}

	inline void CStatic::PreRegisterClass(WNDCLASS& wc)
	{
		// Set the Window Class
		wc.lpszClassName =  _T("Static");
	}

}

#endif	// _WIN32XX_STDCONTROLS_H_

