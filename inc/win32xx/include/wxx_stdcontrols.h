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

    ///////////////////////////////////////////////
    // The CButton class provides the functionality of a button control.
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
        void SetState(UINT uState) const;

    protected:
        // Overridables
        virtual void PreRegisterClass(WNDCLASS& wc);

    private:
        CButton(const CButton&);                // Disable copy construction
        CButton& operator = (const CButton&);   // Disable assignment operator
    };


    ///////////////////////////////////////////////
    // The CEdit class provides the functionality of an edit control.
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
        CEdit(const CEdit&);                // Disable copy construction
        CEdit& operator = (const CEdit&);   // Disable assignment operator
    };


    ///////////////////////////////////////////////
    // The CListBox class provides the functionality of a list box control.
    class CListBox : public CWnd
    {
    public:
        CListBox() {}
        virtual ~CListBox() {}

        // Virtual functions
        virtual int CompareItem(LPCOMPAREITEMSTRUCT pCompareItemStruct);

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
        virtual LRESULT OnMessageReflect(UINT uMsg, WPARAM wParam, LPARAM lParam);
        virtual void PreRegisterClass(WNDCLASS& wc);

    private:
        CListBox(const CListBox&);              // Disable copy construction
        CListBox& operator = (const CListBox&); // Disable assignment operator
    };


    ///////////////////////////////////////////////
    // The CStatic class provides the functionality of a static control.
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
        CStatic(const CStatic&);                // Disable copy construction
        CStatic& operator = (const CStatic&);   // Disable assignment operator
    };

}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


namespace Win32xx
{

    ////////////////////////////////////////
    // Definitions for the CButton class
    //

    // Returns the handle to the bitmap associated with the button.
    inline HBITMAP CButton::GetBitmap() const
    {
        assert(IsWindow());
        return reinterpret_cast<HBITMAP>(SendMessage(BM_GETIMAGE, IMAGE_BITMAP, 0L));
    }


    // Returns the style of the button. Possible styles are:
    // BS_3STATE, BS_AUTO3STATE, BS_AUTOCHECKBOX, VS_AUTORADIOBUTTON, BS_CHECKBOX, BS_DEFPUSHBUTTON, BS_GROUPBOX,
    // BS_LEFTEXT, BS_OWNERDRAW, BS_BITMAP, BS_BOTTOM, BS_CENTER, BS_ICON, BS_FLAT, BS_MULTILINE, BS_PUSHLIKE,
    // BS_RIGHT, BS_RIGHTBUTTON, BS_TEXT, BS_TOP, BS_TYHPEMASK, BS_VCENTER.
    inline UINT CButton::GetButtonStyle() const
    {
        assert(IsWindow());
        return static_cast<UINT>(GetStyle() & 0xFFFF);
    }


    // Returns the check state of the button. The possible states are:
    // BST_CHECKED Button is checked.
    // BST_INDETERMINATE Button is grayed.
    // BST_UNCHECKED Button is cleared.
    inline int CButton::GetCheck() const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(BM_GETCHECK, 0L, 0L));
    }


    // Returns the handle to the cursor associated with the button.
    inline HCURSOR CButton::GetCursor() const
    {
        assert(IsWindow());
        return reinterpret_cast<HCURSOR>(SendMessage(BM_GETIMAGE, IMAGE_CURSOR, 0L));
    }


    // Returns the handle to the icon associated with the button.
    inline HICON CButton::GetIcon() const
    {
        assert(IsWindow());
        return reinterpret_cast<HICON>(SendMessage(BM_GETIMAGE, IMAGE_ICON, 0L));
    }


    // Returns the state of the button. Possible states are:
    // BST_CHECKED
    // BST_FOCUS
    // BST_INDETERMINATE
    // BST_PUSHED
    // BST_UNCHECKED
    inline UINT CButton::GetState() const
    {
        assert(IsWindow());
        return static_cast<UINT>(SendMessage(BM_GETSTATE, 0L, 0L));
    }


    // sets the bitmap associated with the button.
    inline HBITMAP CButton::SetBitmap(HBITMAP hBitmap) const
    {
        assert(IsWindow());
        return reinterpret_cast<HBITMAP>(SendMessage(BM_SETIMAGE, IMAGE_BITMAP, reinterpret_cast<LPARAM>(hBitmap)));
    }


    // Sets the button style. Possible styles are:
    // BS_3STATE, BS_AUTO3STATE, BS_AUTOCHECKBOX, VS_AUTORADIOBUTTON, BS_CHECKBOX, BS_DEFPUSHBUTTON, BS_GROUPBOX,
    // BS_LEFTEXT, BS_OWNERDRAW, BS_BITMAP, BS_BOTTOM, BS_CENTER, BS_ICON, BS_FLAT, BS_MULTILINE, BS_PUSHLIKE,
    // BS_RIGHT, BS_RIGHTBUTTON, BS_TEXT, BS_TOP, BS_TYHPEMASK, BS_VCENTER.
    inline void CButton::SetButtonStyle(DWORD dwStyle, BOOL Redraw) const
    {
        assert(IsWindow());
        SendMessage(BM_SETSTYLE, dwStyle, Redraw);
    }


    // Sets the button check state. The possible states are:
    // BST_CHECKED Button is checked.
    // BST_INDETERMINATE Button is grayed.
    // BST_UNCHECKED Button is cleared.
    inline void CButton::SetCheck(int nCheckState) const
    {
        assert(IsWindow());
        SendMessage(BM_SETCHECK, nCheckState, 0L);
    }


    // Sets the cursor associated with the button.
    inline HCURSOR CButton::SetCursor(HCURSOR hCursor) const
    {
        assert(IsWindow());
        return reinterpret_cast<HCURSOR>(SendMessage(STM_SETIMAGE, IMAGE_CURSOR, reinterpret_cast<LPARAM>(hCursor)));
    }


    // Sets the icon associated with the button.
    inline HICON CButton::SetIcon(HICON hIcon) const
    {
        assert(IsWindow());
        return reinterpret_cast<HICON>(SendMessage( BM_SETIMAGE, IMAGE_ICON, reinterpret_cast<LPARAM>(hIcon)));
    }


    // Sets  the state of the button. Possible states are:
    // BST_CHECKED
    // BST_FOCUS
    // BST_INDETERMINATE
    // BST_PUSHED
    // BST_UNCHECKED
    inline void CButton::SetState(UINT uState) const
    {
        assert(IsWindow());
        SendMessage(BM_SETSTATE, uState, 0L);
    }


    // Sets the window class.
    inline void CButton::PreRegisterClass(WNDCLASS& wc)
    {
        wc.lpszClassName =  _T("Button");
    }


    ////////////////////////////////////////
    // Definitions for the CEdit class
    //

    // Returns TRUE if the edit control operation can be undone.
    inline BOOL CEdit::CanUndo() const
    {
        assert(IsWindow());
        return static_cast<BOOL>(SendMessage(EM_CANUNDO, 0L, 0L));
    }


    // Returns the character index and line index of the character nearest the specified point.
    inline int CEdit::CharFromPos(CPoint pt) const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(EM_CHARFROMPOS, 0L, MAKELPARAM(pt.x, pt.y)));
    }


    // Returns the zero-based index of the first visible character in a single-line edit control
    // or the zero-based index of the uppermost visible line in a multi-line edit control.
    inline int CEdit::GetFirstVisibleLine() const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(EM_GETFIRSTVISIBLELINE, 0L, 0L));
    }


    // Returns a handle identifying the buffer containing the multi-line edit control's text.
    // It is not processed by single-line edit controls.
    inline HLOCAL CEdit::GetHandle() const
    {
        assert(IsWindow());
        return reinterpret_cast<HLOCAL>(SendMessage(EM_GETHANDLE, 0L, 0L));
    }


    // Returns the current text limit, in characters.
    inline UINT CEdit::GetLimitText() const
    {
        assert(IsWindow());
        return static_cast<UINT>(SendMessage(EM_GETLIMITTEXT, 0L, 0L));
    }


    // Copies characters to a buffer and returns the number of characters copied.
    inline int CEdit::GetLine(int nIndex, LPTSTR lpszBuffer) const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(EM_GETLINE, nIndex, reinterpret_cast<LPARAM>(lpszBuffer)));
    }


    // Copies characters to a buffer and returns the number of characters copied.
    inline int CEdit::GetLine(int nIndex, LPTSTR lpszBuffer, int nMaxLength) const
    {
        assert(IsWindow());
        *(LPWORD)lpszBuffer = static_cast<WORD>(nMaxLength);
        return static_cast<int>(SendMessage(EM_GETLINE, nIndex, reinterpret_cast<LPARAM>(lpszBuffer)));
    }


    // Returns the number of lines in the edit control.
    inline int CEdit::GetLineCount() const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(EM_GETLINECOUNT, 0L, 0L));
    }


    // Returns the widths of the left and right margins.
    inline DWORD CEdit::GetMargins() const
    {
        assert(IsWindow());
        return static_cast<DWORD>(SendMessage(EM_GETMARGINS, 0L, 0L));
    }


    // Returns a flag indicating whether the content of an edit control has been modified.
    inline BOOL CEdit::GetModify() const
    {
        assert(IsWindow());
        return static_cast<BOOL>(SendMessage(EM_GETMODIFY, 0L, 0L));
    }


    // Returns the character that edit controls use in conjunction with the ES_PASSWORD style.
    inline TCHAR CEdit::GetPasswordChar() const
    {
        assert(IsWindow());
        return static_cast<TCHAR>(SendMessage(EM_GETPASSWORDCHAR, 0L, 0L));
    }


    // Returns the coordinates of the formatting rectangle in an edit control.
    inline void CEdit::GetRect(RECT& rc) const
    {
        assert(IsWindow());
        SendMessage(EM_GETRECT, 0L, reinterpret_cast<LPARAM>(&rc));
    }


    // Returns the starting and ending character positions of the current selection in the edit control.
    inline void CEdit::GetSel(int& nStartChar, int& nEndChar) const
    {
        assert(IsWindow());
        SendMessage(EM_GETSEL, reinterpret_cast<WPARAM>(&nStartChar), reinterpret_cast<LPARAM>(&nEndChar));
    }


    // Returns the starting and ending character positions of the current selection in the edit control.
    inline DWORD CEdit::GetSel() const
    {
        assert(IsWindow());
        return static_cast<DWORD>(SendMessage(EM_GETSEL, 0L, 0L));
    }


    // Returns the client coordinates of the specified character.
    inline CPoint CEdit::PosFromChar(UINT nChar) const
    {
        assert(IsWindow());
        return CPoint( static_cast<DWORD_PTR>(SendMessage(EM_POSFROMCHAR, nChar, 0L)) );
    }


    // Sets a handle to the memory used as a text buffer, empties the undo buffer,
    // resets the scroll positions to zero, and redraws the window.
    inline void CEdit::SetHandle(HLOCAL hBuffer) const
    {
        assert(IsWindow());
        SendMessage(EM_SETHANDLE, reinterpret_cast<WPARAM>(hBuffer), 0L);
    }


    // Sets the maximum number of characters the user may enter in the edit control.
    inline void CEdit::SetLimitText(UINT nMax) const
    {
        assert(IsWindow());
        SendMessage(EM_SETLIMITTEXT, nMax, 0L);
    }


    // Sets the widths of the left and right margins, and redraws the edit control to reflect the new margins.
    inline void CEdit::SetMargins(UINT nLeft, UINT nRight) const
    {
        assert(IsWindow());
        SendMessage(EM_SETMARGINS, EC_LEFTMARGIN|EC_RIGHTMARGIN, MAKELONG(nLeft, nRight));
    }


    // Sets or clears the modification flag to indicate whether the edit control has been modified.
    inline void CEdit::SetModify(BOOL IsModified) const
    {
        assert(IsWindow());
        SendMessage(EM_SETMODIFY, IsModified, 0L);
    }


    // Empties the undo buffer. The CanUndo function will now return FALSE.
    inline void CEdit::EmptyUndoBuffer() const
    {
        assert(IsWindow());
        SendMessage(EM_EMPTYUNDOBUFFER, 0L, 0L);
    }


    // Adds or removes soft line-break characters (two carriage returns and a line feed) to the ends of wrapped lines
    // in a multi-line edit control. It is not processed by single-line edit controls.
    inline BOOL CEdit::FmtLines(BOOL AddEOL) const
    {
        assert(IsWindow());
        return static_cast<BOOL>(SendMessage(EM_FMTLINES, AddEOL, 0L));
    }


    // Sets the text limit of an edit control. The text limit is the maximum amount of text, in TCHARs,
    // that the user can type into the edit control.
    inline void CEdit::LimitText(int nChars) const
    {
        assert(IsWindow());
        SendMessage(EM_LIMITTEXT, nChars, 0L);
    }


    // Returns the zero-based number of the line in a multi-line edit control that contains a specified character index.
    // This message is the reverse of LineIndex function.
    inline int CEdit::LineFromChar(int nIndex) const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(EM_LINEFROMCHAR, nIndex, 0L));
    }


    // Returns the character of a line in a multi-line edit control.
    // This message is the reverse of the LineFromChar function.
    inline int CEdit::LineIndex(int nLine) const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(EM_LINEINDEX, nLine, 0L));
    }


    // Returns the length, in characters, of a single-line edit control. In a multi-line edit control,
    //  returns the length, in characters, of a specified line.
    inline int CEdit::LineLength(int nLine) const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(EM_LINELENGTH, nLine, 0L));
    }


    // Scrolls the text vertically in a single-line edit control or horizontally in a multi-line edit control.
    inline void CEdit::LineScroll(int nLines, int nChars) const
    {
        assert(IsWindow());
        SendMessage(EM_LINESCROLL, nChars, nLines);
    }


    // Replaces the current selection with the text in an application-supplied buffer, sends the parent window
    // EN_UPDATE and EN_CHANGE messages, and updates the undo buffer.
    inline void CEdit::ReplaceSel(LPCTSTR lpszNewText, BOOL CanUndo) const
    {
        assert(IsWindow());
        SendMessage(EM_REPLACESEL, CanUndo, reinterpret_cast<LPARAM>(lpszNewText));
    }


    // Defines the character that edit controls use in conjunction with the ES_PASSWORD style.
    inline void CEdit::SetPasswordChar(TCHAR ch) const
    {
        assert(IsWindow());
        SendMessage(EM_SETPASSWORDCHAR, ch, 0L);
    }


    // Sets or removes the read-only style (ES_READONLY) in an edit control.
    inline BOOL CEdit::SetReadOnly(BOOL IsReadOnly) const
    {
        assert(IsWindow());
        return static_cast<BOOL>(SendMessage(EM_SETREADONLY, IsReadOnly, 0L));
    }


    // Sets the formatting rectangle for the multi-line edit control and redraws the window.
    inline void CEdit::SetRect(const RECT& rc) const
    {
        assert(IsWindow());
        SendMessage(EM_SETRECT, 0L, reinterpret_cast<LPARAM>(&rc));
    }


    // Sets the formatting rectangle for the multi-line edit control but does not redraw the window.
    inline void CEdit::SetRectNP(const RECT& rc) const
    {
        assert(IsWindow());
        SendMessage(EM_SETRECTNP, 0L, reinterpret_cast<LPARAM>(&rc));
    }


    // Selects a range of characters in the edit control by setting the starting and ending positions to be selected.
    inline void CEdit::SetSel(DWORD dwSelection, BOOL NotScrolled) const
    {
        assert(IsWindow());
        SendMessage(EM_SETSEL, LOWORD(dwSelection), HIWORD(dwSelection));
        if (!NotScrolled)
            SendMessage(EM_SCROLLCARET, 0L, 0L);
    }


    // Selects a range of characters in the edit control by setting the starting and ending positions to be selected.
    inline void CEdit::SetSel(int nStartChar, int nEndChar, BOOL NotScrolled) const
    {
        assert(IsWindow());
        SendMessage(EM_SETSEL, nStartChar, nEndChar);
        if (!NotScrolled)
            SendMessage(EM_SCROLLCARET, 0L, 0L);
    }


    // Sets tab-stop positions in the multi-line edit control.
    inline BOOL CEdit::SetTabStops(int nTabStops, LPINT rgTabStops) const
    {
        assert(IsWindow());
        return static_cast<BOOL>(SendMessage(EM_SETTABSTOPS, nTabStops, reinterpret_cast<LPARAM>(rgTabStops)));
    }


    // Sets tab-stop positions in the multi-line edit control.
    inline BOOL CEdit::SetTabStops() const
    {
        assert(IsWindow());
        return static_cast<BOOL>(SendMessage( EM_SETTABSTOPS, 0L, 0L));
    }


    // Sets tab-stop positions in the multi-line edit control.
    inline BOOL CEdit::SetTabStops(const int& cxEachStop) const
    {
        assert(IsWindow());
        return static_cast<BOOL>(SendMessage(EM_SETTABSTOPS, 1, reinterpret_cast<LPARAM>(&cxEachStop)));
    }


    // Clears the current selection, if any, in an edit control.
    inline void CEdit::Clear() const
    {
        assert(IsWindow());
        SendMessage(WM_CLEAR, 0L, 0L);
    }


    // Copies text to the clipboard unless the style is ES_PASSWORD, in which case the message returns zero.
    inline void CEdit::Copy() const
    {
        assert(IsWindow());
        SendMessage(WM_COPY, 0L, 0L);
    }


    // Cuts the selection to the clipboard, or deletes the character to the left of the cursor if there is no selection.
    inline void CEdit::Cut() const
    {
        assert(IsWindow());
        SendMessage(WM_CUT, 0L, 0L);
    }


    // Pastes text from the clipboard into the edit control window at the caret position.
    inline void CEdit::Paste() const
    {
        assert(IsWindow());
        SendMessage(WM_PASTE, 0L, 0L);
    }


    // Removes any text that was just inserted or inserts any deleted characters and sets the selection to the inserted text.
    inline void CEdit::Undo() const
    {
        assert(IsWindow());
        SendMessage(EM_UNDO, 0L, 0L);
    }


    // Set the window class
    inline void CEdit::PreRegisterClass(WNDCLASS& wc)
    {
        wc.lpszClassName =  _T("Edit");
    }


    ////////////////////////////////////////
    // Definitions for the CListbox class
    //

    // Called by in response to the WM_COMPAREITEM message to determine the relative position
    // of a new item in a sorted owner-draw list box. Override this function in an owner-drawn
    // List-Box to specify the sort order when items are added using AddString.
    inline int CListBox::CompareItem(LPCOMPAREITEMSTRUCT)
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

    // Returns the number of items in the list box.
    inline int CListBox::GetCount() const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(LB_GETCOUNT, 0L, 0L));
    }


    // Returns the scrollable width, in pixels, of a list box.
    inline int CListBox::GetHorizontalExtent() const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(LB_GETHORIZONTALEXTENT, 0L, 0L));
    }


    // Returns the value associated with the specified item.
    inline DWORD CListBox::GetItemData(int nIndex) const
    {
        assert(IsWindow());
        return static_cast<DWORD>(SendMessage(LB_GETITEMDATA, nIndex, 0L));
    }


    // Returns the value associated with the specified item.
    inline void* CListBox::GetItemDataPtr(int nIndex) const
    {
        assert(IsWindow());
        return reinterpret_cast<LPVOID>(SendMessage(LB_GETITEMDATA, nIndex, 0L));
    }


    // Returns the height, in pixels, of an item in a list box.
    inline int CListBox::GetItemHeight(int nIndex) const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(LB_GETITEMHEIGHT, nIndex, 0L));
    }


    // Retrieves the client coordinates of the specified list box item.
    inline int CListBox::GetItemRect(int nIndex, RECT& rc) const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(LB_GETITEMRECT, nIndex, reinterpret_cast<LPARAM>(&rc)));
    }


    // Retrieves the locale of the list box. The high-order word contains the country/region code
    // and the low-order word contains the language identifier.
    inline LCID CListBox::GetLocale() const
    {
        assert(IsWindow());
        return static_cast<LCID>(SendMessage(LB_GETLOCALE, 0L, 0L));
    }


    // Returns the selection state of a list box item.
    inline int CListBox::GetSel(int nIndex) const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(LB_GETSEL, nIndex, 0L));
    }


    // Retrieves the string associated with a specified item and the length of the string.
    inline int CListBox::GetText(int nIndex, LPTSTR lpszBuffer) const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(LB_GETTEXT, nIndex, reinterpret_cast<LPARAM>(lpszBuffer)));
    }


    // Returns the length, in characters, of the string associated with a specified item.
    inline int CListBox::GetTextLen(int nIndex) const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage( LB_GETTEXTLEN, nIndex, 0L));
    }


    // Returns the index of the first visible item in a list box.
    inline int CListBox::GetTopIndex() const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(LB_GETTOPINDEX, 0L, 0L));
    }


    // Retrieves the zero-based index of the item nearest the specified point in a list box.
    inline UINT CListBox::ItemFromPoint(CPoint pt, BOOL& bOutside) const
    {
        assert(IsWindow());
        DWORD dw = static_cast<DWORD>(SendMessage(LB_ITEMFROMPOINT, 0L, MAKELPARAM(pt.x, pt.y)));
        bOutside = !!HIWORD(dw);
        return LOWORD(dw);
    }


    // Sets the width, in pixels, of all columns in a list box.
    inline void CListBox::SetColumnWidth(int cxWidth) const
    {
        assert(IsWindow());
        SendMessage(LB_SETCOLUMNWIDTH, cxWidth, 0L);
    }


    // Sets the scrollable width, in pixels, of a list box.
    inline void CListBox::SetHorizontalExtent(int cxExtent) const
    {
        assert(IsWindow());
        SendMessage(LB_SETHORIZONTALEXTENT, cxExtent, 0L);
    }


    // Associates a value with a list box item.
    inline int CListBox::SetItemData(int nIndex, DWORD dwItemData) const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(LB_SETITEMDATA, nIndex, dwItemData));
    }


    // Associates a value with a list box item.
    inline int CListBox::SetItemDataPtr(int nIndex, void* pData) const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(LB_SETITEMDATA, nIndex, reinterpret_cast<LPARAM>(pData)));
    }


    // Sets the height, in pixels, of an item or items in a list box.
    inline int CListBox::SetItemHeight(int nIndex, UINT cyItemHeight) const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(LB_SETITEMHEIGHT, nIndex, MAKELONG(cyItemHeight, 0)));
    }


    // Sets the locale of a list box and returns the previous locale identifier.
    inline LCID CListBox::SetLocale(LCID nNewLocale) const
    {
        assert(IsWindow());
        return static_cast<LCID>(SendMessage(LB_SETLOCALE, nNewLocale, 0L));
    }


    // Sets the tab stops to those specified in a specified array.
    inline BOOL CListBox::SetTabStops(int nTabStops, LPINT rgTabStops) const
    {
        assert(IsWindow());
        return static_cast<BOOL>(SendMessage(LB_SETTABSTOPS, nTabStops, reinterpret_cast<LPARAM>(rgTabStops)));
    }


    // Sets the tab stops to those specified in a specified array.
    inline void CListBox::SetTabStops() const
    {
        assert(IsWindow());
        SendMessage(LB_SETTABSTOPS, 0L, 0L);
    }


    inline BOOL CListBox::SetTabStops(const int& cxEachStop) const
    // Sets the tab stops to those specified in a specified array.
    {
        assert(IsWindow());
        return static_cast<BOOL>(SendMessage(LB_SETTABSTOPS, 1, reinterpret_cast<LPARAM>(&cxEachStop)));
    }


    // Scrolls the list box so the specified item is at the top of the visible range.
    inline int CListBox::SetTopIndex(int nIndex) const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(LB_SETTOPINDEX, nIndex, 0L));
    }


    // Returns the index of the currently selected item.
    inline int CListBox::GetCurSel() const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(LB_GETCURSEL, 0L, 0L));
    }


    // Selects a specified list box item.
    inline int CListBox::SetCurSel(int nSelect) const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(LB_SETCURSEL, nSelect, 0L));
    }


    // Returns the index of the item that the mouse last selected.
    inline int CListBox::GetAnchorIndex() const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(LB_GETANCHORINDEX, 0L, 0L));
    }


    // Returns the index of the item that has the focus rectangle.
    inline int CListBox::GetCaretIndex() const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(LB_GETCARETINDEX, 0L, 0L));
    }


    // Returns the number of selected items in a multiple-selection list box.
    inline int CListBox::GetSelCount() const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(LB_GETSELCOUNT, 0L, 0L));
    }


    // Creates an array of the indexes of all selected items in a multiple-selection list box
    // and returns the total number of selected items.
    inline int CListBox::GetSelItems(int nMaxItems, LPINT rgIndex) const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(LB_GETSELITEMS, nMaxItems, reinterpret_cast<LPARAM>(rgIndex)));
    }


    // Selects a specified range of items in a list box.
    inline int CListBox::SelItemRange(BOOL IsSelected, int nFirstItem, int nLastItem) const
    {
        assert(IsWindow());
        if (IsSelected)
            return static_cast<int>(SendMessage(LB_SELITEMRANGEEX, nFirstItem, nLastItem));
        else
            return static_cast<int>(SendMessage(LB_SELITEMRANGEEX, nLastItem, nFirstItem));
    }


    // Sets the item that the mouse last selected to a specified item.
    inline void CListBox::SetAnchorIndex(int nIndex) const
    {
        assert(IsWindow());
        SendMessage(LB_SETANCHORINDEX, nIndex, 0L);
    }


    // Sets the focus rectangle to a specified list box item.
    inline int CListBox::SetCaretIndex(int nIndex, BOOL PartialScroll) const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(LB_SETCARETINDEX, nIndex, MAKELONG(PartialScroll, 0)));
    }


    // Selects an item in a multiple-selection list box.
    inline int CListBox::SetSel(int nIndex, BOOL IsSelected) const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(LB_SETSEL, IsSelected, nIndex));
    }


    // Adds a string to a list box and returns its index.
    inline int CListBox::AddString(LPCTSTR lpszItem) const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(LB_ADDSTRING, 0L, reinterpret_cast<LPARAM>(lpszItem)));
    }


    // Removes a string from a list box and returns the number of strings remaining in the list.
    inline int CListBox::DeleteString(UINT nIndex) const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(LB_DELETESTRING, nIndex, 0L));
    }


    // Adds a list of filenames to a list box and returns the index of the last filename added.
    inline int CListBox::Dir(UINT attr, LPCTSTR lpszWildCard) const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(LB_DIR, attr, reinterpret_cast<LPARAM>(lpszWildCard)));
    }


    // Returns the index of the first string in the list box that begins with a specified string.
    inline int CListBox::FindString(int nStartAfter, LPCTSTR lpszItem) const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(LB_FINDSTRING, nStartAfter, reinterpret_cast<LPARAM>(lpszItem)));
    }


    // Returns the index of the string in the list box that is equal to a specified string.
    inline int CListBox::FindStringExact(int nIndexStart, LPCTSTR lpszFind) const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(LB_FINDSTRINGEXACT, nIndexStart, reinterpret_cast<LPARAM>(lpszFind)));
    }


    // Inserts a string at a specified index in a list box.
    inline int CListBox::InsertString(int nIndex, LPCTSTR lpszItem) const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(LB_INSERTSTRING, nIndex, reinterpret_cast<LPARAM>(lpszItem)));
    }


    // Removes all items from a list box.
    inline void CListBox::ResetContent() const
    {
        assert(IsWindow());
        SendMessage(LB_RESETCONTENT, 0L, 0L);
    }


    // Selects the first string it finds that matches a specified prefix.
    inline int CListBox::SelectString(int nStartAfter, LPCTSTR lpszItem) const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(LB_SELECTSTRING, nStartAfter, reinterpret_cast<LPARAM>(lpszItem)));
    }


    // Handle messages reflected back from the parent window.
    // Override this function in your derived class to handle these special messages:
    // WM_COMMAND, WM_CTLCOLORBTN, WM_CTLCOLOREDIT, WM_CTLCOLORDLG, WM_CTLCOLORLISTBOX,
    // WM_CTLCOLORSCROLLBAR, WM_CTLCOLORSTATIC, WM_CHARTOITEM,  WM_VKEYTOITEM,
    // WM_HSCROLL, WM_VSCROLL, WM_DRAWITEM, WM_MEASUREITEM, WM_DELETEITEM,
    // WM_COMPAREITEM, WM_PARENTNOTIFY.
    inline LRESULT CListBox::OnMessageReflect(UINT uMsg, WPARAM, LPARAM lParam)
    {
        switch (uMsg)
        {
        case WM_COMPAREITEM:
            {
                LPCOMPAREITEMSTRUCT pCompareItemStruct = (LPCOMPAREITEMSTRUCT)lParam;
                return CompareItem(pCompareItemStruct);
            }
        }

        return 0;   // Allow other messages to be handled elsewhere. 
    }


    // Set the window class.
    inline void CListBox::PreRegisterClass(WNDCLASS& wc)
    {
        wc.lpszClassName =  _T("ListBox");
    }


    ////////////////////////////////////////
    // Definitions for the CStatic class
    //

    // Returns the handle to the bitmap for the static control set by CStatic::SetBitmap.
    inline HBITMAP CStatic::GetBitmap() const
    {
        assert(IsWindow());
        return reinterpret_cast<HBITMAP>(SendMessage(STM_GETIMAGE, IMAGE_BITMAP, 0L));
    }


    // Returns the handle to the cursor for the static control set by CStatic::SetCursor.
    inline HCURSOR CStatic::GetCursor() const
    {
        assert(IsWindow());
        return reinterpret_cast<HCURSOR>(SendMessage(STM_GETIMAGE, IMAGE_CURSOR, 0L));
    }


    // Returns the handle to the enhanced meta-file for the static control set by CStatic::SetEnhMetaFile.
    inline HENHMETAFILE CStatic::GetEnhMetaFile() const

    {
        assert(IsWindow());
        return reinterpret_cast<HENHMETAFILE>(SendMessage(STM_GETIMAGE, IMAGE_ENHMETAFILE, 0L));
    }


    // Returns the handle to the icon for the static control set by CStatic::SetIcon.
    inline HICON CStatic::GetIcon() const
    {
        assert(IsWindow());
        return reinterpret_cast<HICON>(SendMessage(STM_GETIMAGE, IMAGE_ICON, 0L));
    }


    // Associates a new bitmap with the static control.
    // The bitmap will be drawn in the upper-left corner, and the static
    // control will be resized to the size of the bitmap.
    // This function requires the SS_BITMAP style.
    inline HBITMAP CStatic::SetBitmap(HBITMAP hBitmap) const
    {
        assert(IsWindow());
        return reinterpret_cast<HBITMAP>(SendMessage(STM_SETIMAGE, IMAGE_BITMAP, reinterpret_cast<LPARAM>(hBitmap)));
    }


    // Associates a new cursor image with the static control.
    // The cursor will be drawn in the upper-left corner and the static
    // control will be resized to the size of the cursor.
    // This function requires the SS_ICON style
    inline HCURSOR CStatic::SetCursor(HCURSOR hCursor) const
    {
        assert(IsWindow());
        return reinterpret_cast<HCURSOR>(SendMessage(STM_SETIMAGE, IMAGE_CURSOR, reinterpret_cast<LPARAM>(hCursor)));
    }


    // Associates a new enhanced metafile image with the static control.
    // The enhanced metafile will be drawn in the upper-left corner and the static
    // control will be resized to the size of the enhanced metafile.
    // This function requires the SS_ENHMETAFILE style
    inline HENHMETAFILE CStatic::SetEnhMetaFile(HENHMETAFILE hMetaFile) const
    {
        assert(IsWindow());
        return reinterpret_cast<HENHMETAFILE>(SendMessage(STM_SETIMAGE, IMAGE_ENHMETAFILE, reinterpret_cast<LPARAM>(hMetaFile)));
    }


    // Associates a new icon image with the static control.
    // The icon will be drawn in the upper-left corner and the static
    // control will be resized to the size of the icon.
    // This function requires the SS_ICON style
    inline HICON CStatic::SetIcon(HICON hIcon) const
    {
        assert(IsWindow());
        return reinterpret_cast<HICON>(SendMessage(STM_SETIMAGE, IMAGE_ICON, reinterpret_cast<LPARAM>(hIcon)));
    }


    // Set the window class.
    inline void CStatic::PreRegisterClass(WNDCLASS& wc)
    {

        wc.lpszClassName =  _T("Static");
    }

}

#endif  // _WIN32XX_STDCONTROLS_H_

