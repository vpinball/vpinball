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

    ////////////////////////////////////////////////////////////////////
    // The CButton class provides the functionality of a button control.
    // A button is a control the user can click to provide input to an
    // application.
    class CButton : public CWnd
    {
    public:
        CButton() {}
        virtual ~CButton() {}

        // Accessors and mutators
        HBITMAP GetBitmap() const;
        UINT GetButtonStyle() const;
        int GetCheck() const;
        HCURSOR GetCursor() const;
        HICON GetIcon() const;
        UINT GetState() const;
        HBITMAP SetBitmap(HBITMAP bitmap) const;
        void SetButtonStyle(DWORD style, BOOL redraw) const;
        void SetCheck(int checkState) const;
        HCURSOR SetCursor(HCURSOR cursor) const;
        HICON SetIcon(HICON icon) const;
        void SetState(UINT state) const;

    protected:
        // Overridables
        virtual void PreRegisterClass(WNDCLASS& wc);

    private:
        CButton(const CButton&);                // Disable copy construction
        CButton& operator = (const CButton&);   // Disable assignment operator
    };


    ///////////////////////////////////////////////////////////////////
    // CEdit manages an edit control. An edit control is a rectangular
    // control window typically used in a dialog box to permit the user
    // to enter and edit text by typing on the keyboard.
    class CEdit : public CWnd
    {
    public:
        // Construction
        CEdit() {}
        virtual ~CEdit() {}

        // Attributes
        void   AppendText(LPCTSTR text) const;
        BOOL   CanUndo() const;
        int    CharFromPos(CPoint pt) const;
        int    GetFirstVisibleLine() const;
        HLOCAL GetHandle() const;
        UINT   GetLimitText() const;
        int    GetLine(int index, LPTSTR buffer) const;
        int    GetLine(int index, LPTSTR buffer, int maxLength) const;
        int    GetLineCount() const;
        DWORD  GetMargins() const;
        BOOL   GetModify() const;
        TCHAR  GetPasswordChar() const;
        void   GetRect(RECT& rc) const;
        void   GetSel(int& startChar, int& endChar) const;
        DWORD  GetSel() const;
        long   GetTextLength() const;
        CPoint PosFromChar(UINT index) const;
        void   SetHandle(HLOCAL buffer) const;
        void   SetLimitText(UINT max) const;
        void   SetMargins(UINT left, UINT right) const;
        void   SetModify(BOOL isModified = TRUE) const;

        // Operations
        void   EmptyUndoBuffer() const;
        BOOL   FmtLines(BOOL addEOL) const;
        void   LimitText(int limit = 0) const;
        int    LineFromChar(int index = -1) const;
        int    LineIndex(int line = -1) const;
        int    LineLength(int line = -1) const;
        void   LineScroll(int lines, int chars = 0) const;
        void   ReplaceSel(LPCTSTR newText, BOOL canUndo) const;
        void   SetPasswordChar(TCHAR ch) const;
        BOOL   SetReadOnly(BOOL isReadOnly = TRUE) const;
        void   SetRect(const RECT& rc) const;
        void   SetRectNP(const RECT& rc) const;
        void   SetSel(DWORD selection, BOOL isScrolled = TRUE) const;
        void   SetSel(int startChar, int endChar, BOOL isScrolled = TRUE) const;
        BOOL   SetTabStops(int tabStops, LPINT pTabStopsArray) const;
        BOOL   SetTabStops() const;
        BOOL   SetTabStops(const int& cxEachStop) const;

        //Clipboard Operations
        void   Clear() const;
        void   Copy() const;
        void   Cut() const;
        void   Paste() const;
        void   Undo() const;

    protected:
        // Overridables
        virtual void PreRegisterClass(WNDCLASS& wc);

    private:
        CEdit(const CEdit&);                // Disable copy construction
        CEdit& operator = (const CEdit&);   // Disable assignment operator
    };


    ////////////////////////////////////////////////////////////
    // CListBox manages a list box control. List boxes display a
    // list from which a user can select one or more items.
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
        DWORD GetItemData(int index) const;
        void* GetItemDataPtr(int index) const;
        int  GetItemHeight(int index) const;
        int  GetItemRect(int index, RECT& rc) const;
        LCID GetLocale() const;
        int  GetSel(int index) const;
        int  GetText(int index, LPTSTR buffer) const;
        int  GetTextLen(int index) const;
        int  GetTopIndex() const;
        UINT ItemFromPoint(CPoint pt, BOOL& isOutside ) const;
        void SetColumnWidth(int cxWidth) const;
        void SetHorizontalExtent(int cxExtent) const;
        int  SetItemData(int index, DWORD itemData) const;
        int  SetItemDataPtr(int index, void* pData) const;
        int  SetItemHeight(int index, UINT cyItemHeight) const;
        LCID SetLocale(LCID newLocale) const;
        BOOL SetTabStops(int tabStops, LPINT pTabStopsArray) const;
        void SetTabStops() const;
        BOOL SetTabStops(const int& cxEachStop) const;
        int  SetTopIndex(int index) const;

        // Single-Selection Operations
        int  GetCurSel() const;
        int  SetCurSel(int select) const;

        // Multiple-Selection Operations
        int  GetAnchorIndex() const;
        int  GetCaretIndex() const;
        int  GetSelCount() const;
        int  GetSelItems(int maxItems, LPINT pItemArray) const;
        int  SelItemRange(BOOL isSelected, int firstItem, int lastItem) const;
        void SetAnchorIndex(int index) const;
        int  SetCaretIndex(int index, BOOL partialScroll) const;
        int  SetSel(int index, BOOL isSelected) const;

        // String Operations
        int  AddString(LPCTSTR string) const;
        int  DeleteString(UINT index) const;
        int  Dir(UINT attr, LPCTSTR wildCard) const;
        int  FindString(int startAfter, LPCTSTR string) const;
        int  FindStringExact(int indexStart, LPCTSTR findString) const;
        int  InsertString(int index, LPCTSTR string) const;
        void ResetContent() const;
        int  SelectString(int startAfter, LPCTSTR string) const;

    protected:
        // Overridables
        virtual LRESULT OnMessageReflect(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual void PreRegisterClass(WNDCLASS& wc);

    private:
        CListBox(const CListBox&);              // Disable copy construction
        CListBox& operator = (const CListBox&); // Disable assignment operator
    };


    ///////////////////////////////////////////////////////////
    // CStatic manages a static control. Applications often use
    // static controls to label other controls or to separate a
    // group of controls.
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
        HBITMAP SetBitmap(HBITMAP bitmap) const;
        HCURSOR SetCursor(HCURSOR cursor) const;
        HENHMETAFILE SetEnhMetaFile(HENHMETAFILE metaFile) const;
        HICON SetIcon(HICON icon) const;

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
    // Refer to BM_GETIMAGE in the Windows API documentation for more information.
    inline HBITMAP CButton::GetBitmap() const
    {
        assert(IsWindow());
        return reinterpret_cast<HBITMAP>(SendMessage(BM_GETIMAGE, IMAGE_BITMAP, 0));
    }

    // Returns the style of the button. Possible styles are:
    // BS_3STATE, BS_AUTO3STATE, BS_AUTOCHECKBOX, BS_AUTORADIOBUTTON, BS_CHECKBOX, BS_DEFPUSHBUTTON, BS_GROUPBOX,
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
    // Refer to BM_GETCHECK in the Windows API documentation for more information.
    inline int CButton::GetCheck() const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(BM_GETCHECK, 0, 0));
    }

    // Returns the handle to the cursor associated with the button.
    // Refer to BM_GETIMAGE in the Windows API documentation for more information.
    inline HCURSOR CButton::GetCursor() const
    {
        assert(IsWindow());
        return reinterpret_cast<HCURSOR>(SendMessage(BM_GETIMAGE, IMAGE_CURSOR, 0));
    }

    // Returns the handle to the icon associated with the button.
    // Refer to BM_GETIMAGE in the Windows API documentation for more information.
    inline HICON CButton::GetIcon() const
    {
        assert(IsWindow());
        return reinterpret_cast<HICON>(SendMessage(BM_GETIMAGE, IMAGE_ICON, 0));
    }

    // Returns the state of the button. Possible states are:
    // BST_CHECKED
    // BST_FOCUS
    // BST_INDETERMINATE
    // BST_PUSHED
    // BST_UNCHECKED
    // Refer to BM_GETSTATE in the Windows API documentation for more information.
    inline UINT CButton::GetState() const
    {
        assert(IsWindow());
        return static_cast<UINT>(SendMessage(BM_GETSTATE, 0, 0));
    }

    // Sets the window class.
    inline void CButton::PreRegisterClass(WNDCLASS& wc)
    {
        wc.lpszClassName = _T("Button");
    }

    // sets the bitmap associated with the button.
    // Refer to BM_SETIMAGE in the Windows API documentation for more information.
    inline HBITMAP CButton::SetBitmap(HBITMAP bitmap) const
    {
        assert(IsWindow());
        LPARAM lparam = reinterpret_cast<LPARAM>(bitmap);
        return reinterpret_cast<HBITMAP>(SendMessage(BM_SETIMAGE, IMAGE_BITMAP, lparam));
    }

    // Sets the button style. Possible styles are:
    // BS_3STATE, BS_AUTO3STATE, BS_AUTOCHECKBOX, BS_AUTORADIOBUTTON, BS_CHECKBOX, BS_DEFPUSHBUTTON, BS_GROUPBOX,
    // BS_LEFTEXT, BS_OWNERDRAW, BS_BITMAP, BS_BOTTOM, BS_CENTER, BS_ICON, BS_FLAT, BS_MULTILINE, BS_PUSHLIKE,
    // BS_RIGHT, BS_RIGHTBUTTON, BS_TEXT, BS_TOP, BS_TYHPEMASK, BS_VCENTER.
    // Refer to BM_SETSTYLE in the Windows API documentation for more information.
    inline void CButton::SetButtonStyle(DWORD style, BOOL redraw) const
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(style);
        LPARAM lparam = static_cast<LPARAM>(redraw);
        SendMessage(BM_SETSTYLE, wparam, lparam);
    }

    // Sets the button check state. The possible states are:
    // BST_CHECKED Button is checked.
    // BST_INDETERMINATE Button is grayed.
    // BST_UNCHECKED Button is cleared.
    // Refer to BM_SETCHECK in the Windows API documentation for more information.
    inline void CButton::SetCheck(int checkState) const
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(checkState);
        SendMessage(BM_SETCHECK, wparam, 0);
    }

    // Sets the cursor associated with the button.
    // Refer to BM_SETIMAGE in the Windows API documentation for more information.
    inline HCURSOR CButton::SetCursor(HCURSOR cursor) const
    {
        assert(IsWindow());
        LPARAM lparam = reinterpret_cast<LPARAM>(cursor);
        return reinterpret_cast<HCURSOR>(SendMessage(BM_SETIMAGE, IMAGE_CURSOR, lparam));
    }

    // Sets the icon associated with the button.
    // Refer to BM_SETIMAGE in the Windows API documentation for more information.
    inline HICON CButton::SetIcon(HICON icon) const
    {
        assert(IsWindow());
        LPARAM lparam = reinterpret_cast<LPARAM>(icon);
        return reinterpret_cast<HICON>(SendMessage( BM_SETIMAGE, IMAGE_ICON, lparam));
    }

    // Sets  the state of the button. Possible states are:
    // BST_CHECKED
    // BST_FOCUS
    // BST_INDETERMINATE
    // BST_PUSHED
    // BST_UNCHECKED
    // Refer to BM_SETSTATE in the Windows API documentation for more information.
    inline void CButton::SetState(UINT state) const
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(state);
        SendMessage(BM_SETSTATE, wparam, 0);
    }


    ////////////////////////////////////////
    // Definitions for the CEdit class
    //

    // Adds text to the end of the document.
    inline void CEdit::AppendText(LPCTSTR text) const
    {
        LRESULT position = SendMessage(WM_GETTEXTLENGTH, 0, 0);
        WPARAM wparam = static_cast<WPARAM>(position);
        LPARAM lparam = static_cast<LPARAM>(position);
        SendMessage(EM_SETSEL, wparam, lparam);
        lparam = reinterpret_cast<LPARAM>(text);
        SendMessage(EM_REPLACESEL, 0, lparam);
    }

    // Returns TRUE if the edit control operation can be undone.
    // Refer to EM_CANUNDO in the Windows API documentation for more information.
    inline BOOL CEdit::CanUndo() const
    {
        assert(IsWindow());
        return static_cast<BOOL>(SendMessage(EM_CANUNDO, 0, 0));
    }

    // Returns the character index and line index of the character nearest the specified point.
    // Refer to EM_CHARFROMPOS in the Windows API documentation for more information.
    inline int CEdit::CharFromPos(CPoint pt) const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(EM_CHARFROMPOS, 0, MAKELPARAM(pt.x, pt.y)));
    }

    // Clears the current selection, if any, in an edit control.
    // Refer to WM_CLEAR in the Windows API documentation for more information.
    inline void CEdit::Clear() const
    {
        assert(IsWindow());
        SendMessage(WM_CLEAR, 0, 0);
    }

    // Copies text to the clipboard unless the style is ES_PASSWORD, in which case the message returns zero.
    // Refer to WM_COPY in the Windows API documentation for more information.
    inline void CEdit::Copy() const
    {
        assert(IsWindow());
        SendMessage(WM_COPY, 0, 0);
    }

    // Cuts the selection to the clipboard, or deletes the character to the left of the cursor if there is no selection.
    // Refer to WM_CUT in the Windows API documentation for more information.
    inline void CEdit::Cut() const
    {
        assert(IsWindow());
        SendMessage(WM_CUT, 0, 0);
    }

    // Returns the zero-based index of the first visible character in a single-line edit control
    // or the zero-based index of the uppermost visible line in a multi-line edit control.
    // Refer to EM_GETFIRSTVISIBLELINE in the Windows API documentation for more information.
    inline int CEdit::GetFirstVisibleLine() const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(EM_GETFIRSTVISIBLELINE, 0, 0));
    }

    // Returns a handle identifying the buffer containing the multi-line edit control's text.
    // It is not processed by single-line edit controls.
    // Refer to EM_GETHANDLE in the Windows API documentation for more information.
    inline HLOCAL CEdit::GetHandle() const
    {
        assert(IsWindow());
        return reinterpret_cast<HLOCAL>(SendMessage(EM_GETHANDLE, 0, 0));
    }

    // Returns the current text limit, in characters.
    // Refer to EM_GETLIMITTEXT in the Windows API documentation for more information.
    inline UINT CEdit::GetLimitText() const
    {
        assert(IsWindow());
        return static_cast<UINT>(SendMessage(EM_GETLIMITTEXT, 0, 0));
    }

    // Copies characters to a buffer and returns the number of characters copied.
    // Set the first word of buffer to the size of the buffer in TCHARs.
    // The copied line does not contain a terminating null character.
    // Refer to EM_GETLINE in the Windows API documentation for more information.
    inline int CEdit::GetLine(int index, LPTSTR buffer) const
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(index);
        LPARAM lparam = reinterpret_cast<LPARAM>(buffer);
        return static_cast<int>(SendMessage(EM_GETLINE, wparam, lparam));
    }

    // Copies characters to a buffer and returns the number of characters copied.
    // The copied line does not contain a terminating null character
    // Refer to EM_GETLINE in the Windows API documentation for more information.
    inline int CEdit::GetLine(int index, LPTSTR buffer, int maxLength) const
    {
        assert(IsWindow());

        // set the first word of this buffer to the size, in TCHARs, of the buffer.
        *reinterpret_cast<LPWORD>(buffer) = static_cast<WORD>(maxLength);
        WPARAM wparam = static_cast<WPARAM>(index);
        LPARAM lparam = reinterpret_cast<LPARAM>(buffer);
        return static_cast<int>(SendMessage(EM_GETLINE, wparam, lparam));
    }

    // Returns the number of lines in the edit control.
    // Refer to EM_GETLINECOUNT in the Windows API documentation for more information.
    inline int CEdit::GetLineCount() const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(EM_GETLINECOUNT, 0, 0));
    }

    // Returns the widths of the left and right margins.
    // Refer to EM_GETMARGINS in the Windows API documentation for more information.
    inline DWORD CEdit::GetMargins() const
    {
        assert(IsWindow());
        return static_cast<DWORD>(SendMessage(EM_GETMARGINS, 0, 0));
    }

    // Returns a flag indicating whether the content of an edit control has been modified.
    // Refer to EM_GETMODIFY in the Windows API documentation for more information.
    inline BOOL CEdit::GetModify() const
    {
        assert(IsWindow());
        return static_cast<BOOL>(SendMessage(EM_GETMODIFY, 0, 0));
    }

    // Returns the character that edit controls use in conjunction with the ES_PASSWORD style.
    // Refer to EM_GETPASSWORDCHAR in the Windows API documentation for more information.
    inline TCHAR CEdit::GetPasswordChar() const
    {
        assert(IsWindow());
        return static_cast<TCHAR>(SendMessage(EM_GETPASSWORDCHAR, 0, 0));
    }

    // Returns the coordinates of the formatting rectangle in an edit control.
    // Refer to EM_GETRECT in the Windows API documentation for more information.
    inline void CEdit::GetRect(RECT& rc) const
    {
        assert(IsWindow());
        LPARAM lparam = reinterpret_cast<LPARAM>(&rc);
        SendMessage(EM_GETRECT, 0, lparam);
    }

    // Retrieves the starting and ending character positions of the current selection in the edit control.
    // Refer to EM_GETSEL in the Windows API documentation for more information.
    inline void CEdit::GetSel(int& startChar, int& endChar) const
    {
        assert(IsWindow());
        WPARAM wparam = reinterpret_cast<WPARAM>(&startChar);
        LPARAM lparam = reinterpret_cast<LPARAM>(&endChar);
        SendMessage(EM_GETSEL, wparam, lparam);
    }

    // Retrieves the starting and ending character positions of the current selection in the edit control.
    // Refer to EM_GETSEL in the Windows API documentation for more information.
    inline DWORD CEdit::GetSel() const
    {
        assert(IsWindow());
        return static_cast<DWORD>(SendMessage(EM_GETSEL, 0, 0));
    }

    // Retrieves the length of the text, in characters. Does not include the terminating null character.
    // Refer to WM_GETTEXTLENGTH in the Windows API documentation for more information.
    inline long CEdit::GetTextLength() const
    {
        assert(IsWindow());
        return static_cast<long>(SendMessage(WM_GETTEXTLENGTH, 0, 0));
    }

    // Empties the undo buffer. The CanUndo function will now return FALSE.
    // Refer to EM_EMPTYUNDOBUFFER in the Windows API documentation for more information.
    inline void CEdit::EmptyUndoBuffer() const
    {
        assert(IsWindow());
        SendMessage(EM_EMPTYUNDOBUFFER, 0, 0);
    }

    // Adds or removes soft line-break characters (two carriage returns and a line feed) to the ends of wrapped lines
    // in a multi-line edit control. It is not processed by single-line edit controls.
    // Refer to EM_FMTLINES in the Windows API documentation for more information.
    inline BOOL CEdit::FmtLines(BOOL addEOL) const
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(addEOL);
        return static_cast<BOOL>(SendMessage(EM_FMTLINES, wparam, 0));
    }

    // Sets the text limit of an edit control. The text limit is the maximum amount of text, in TCHARs,
    // that the user can type into the edit control.
    // Refer to EM_LIMITTEXT in the Windows API documentation for more information.
    inline void CEdit::LimitText(int limit) const
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(limit);
        SendMessage(EM_LIMITTEXT, wparam, 0);
    }

    // Returns the zero-based number of the line in a multi-line edit control that contains a specified character index.
    // This message is the reverse of LineIndex function.
    // Refer to EM_LINEFROMCHAR in the Windows API documentation for more information.
    inline int CEdit::LineFromChar(int index) const
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(index);
        return static_cast<int>(SendMessage(EM_LINEFROMCHAR, wparam, 0));
    }

    // Returns the character of a line in a multi-line edit control.
    // This message is the reverse of the LineFromChar function.
    // Refer to EM_LINEINDEX in the Windows API documentation for more information.
    inline int CEdit::LineIndex(int line) const
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(line);
        return static_cast<int>(SendMessage(EM_LINEINDEX, wparam, 0));
    }

    // Returns the length, in characters, of a single-line edit control. In a multi-line edit control,
    //  returns the length, in characters, of a specified line.
    // Refer to EM_LINELENGTH in the Windows API documentation for more information.
    inline int CEdit::LineLength(int line) const
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(line);
        return static_cast<int>(SendMessage(EM_LINELENGTH, wparam, 0));
    }

    // Scrolls the text vertically in a single-line edit control or horizontally in a multi-line edit control.
    // Refer to EM_LINESCROLL in the Windows API documentation for more information.
    inline void CEdit::LineScroll(int lines, int chars) const
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(chars);
        LPARAM lparam = static_cast<LPARAM>(lines);
        SendMessage(EM_LINESCROLL, wparam, lparam);
    }

    // Pastes text from the clipboard into the edit control window at the caret position.
    // Refer to WM_PASTE in the Windows API documentation for more information.
    inline void CEdit::Paste() const
    {
        assert(IsWindow());
        SendMessage(WM_PASTE, 0, 0);
    }

    // Returns the client coordinates of the specified character.
    // Refer to EM_POSFROMCHAR in the Windows API documentation for more information.
    inline CPoint CEdit::PosFromChar(UINT index) const
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(index);
        return static_cast<CPoint>(SendMessage(EM_POSFROMCHAR, wparam, 0));
    }

    // Set the window class
    inline void CEdit::PreRegisterClass(WNDCLASS& wc)
    {
        wc.lpszClassName =  _T("Edit");
    }

    // Replaces the current selection with the text in an application-supplied buffer, sends the parent window
    // EN_UPDATE and EN_CHANGE messages, and updates the undo buffer.
    // Refer to EM_REPLACESEL in the Windows API documentation for more information.
    inline void CEdit::ReplaceSel(LPCTSTR newText, BOOL canUndo) const
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(canUndo);
        LPARAM lparam = reinterpret_cast<LPARAM>(newText);
        SendMessage(EM_REPLACESEL, wparam, lparam);
    }

    // Sets a handle to the memory used as a text buffer, empties the undo buffer,
    // resets the scroll positions to zero, and redraws the window.
    // Refer to EM_SETHANDLE in the Windows API documentation for more information.
    inline void CEdit::SetHandle(HLOCAL buffer) const
    {
        assert(IsWindow());
        WPARAM wparam = reinterpret_cast<WPARAM>(buffer);
        SendMessage(EM_SETHANDLE, wparam, 0);
    }

    // Sets the maximum number of characters the user may enter in the edit control.
    // Refer to EM_SETLIMITTEXT in the Windows API documentation for more information.
    inline void CEdit::SetLimitText(UINT max) const
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(max);
        SendMessage(EM_SETLIMITTEXT, wparam, 0);
    }

    // Sets the widths of the left and right margins, and redraws the edit control to reflect the new margins.
    // Refer to EM_SETMARGINS in the Windows API documentation for more information.
    inline void CEdit::SetMargins(UINT left, UINT right) const
    {
        assert(IsWindow());
        LPARAM lparam = static_cast<LPARAM>(MAKELONG(left, right));
        SendMessage(EM_SETMARGINS, (EC_LEFTMARGIN | EC_RIGHTMARGIN), lparam);
    }

    // Sets or clears the modification flag to indicate whether the edit control has been modified.
    // Refer to EM_SETMODIFY in the Windows API documentation for more information.
    inline void CEdit::SetModify(BOOL isModified) const
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(isModified);
        SendMessage(EM_SETMODIFY, wparam, 0);
    }

    // Defines the character that edit controls use in conjunction with the ES_PASSWORD style.
    // Refer to EM_SETPASSWORDCHAR in the Windows API documentation for more information.
    inline void CEdit::SetPasswordChar(TCHAR ch) const
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(ch);
        SendMessage(EM_SETPASSWORDCHAR, wparam, 0);
    }

    // Sets or removes the read-only style (ES_READONLY) in an edit control.
    // Refer to EM_SETREADONLY in the Windows API documentation for more information.
    inline BOOL CEdit::SetReadOnly(BOOL isReadOnly) const
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(isReadOnly);
        return static_cast<BOOL>(SendMessage(EM_SETREADONLY, wparam, 0));
    }

    // Sets the formatting rectangle for the multi-line edit control and redraws the window.
    // Refer to EM_SETRECT in the Windows API documentation for more information.
    inline void CEdit::SetRect(const RECT& rc) const
    {
        assert(IsWindow());
        LPARAM lparam = reinterpret_cast<LPARAM>(&rc);
        SendMessage(EM_SETRECT, 0, lparam);
    }

    // Sets the formatting rectangle for the multi-line edit control but does not redraw the window.
    // Refer to EM_SETRECTNP in the Windows API documentation for more information.
    inline void CEdit::SetRectNP(const RECT& rc) const
    {
        assert(IsWindow());
        LPARAM lparam = reinterpret_cast<LPARAM>(&rc);
        SendMessage(EM_SETRECTNP, 0, lparam);
    }

    // Selects a range of characters in the edit control by setting the starting
    // and ending positions to be selected.
    // Refer to EM_SETSEL in the Windows API documentation for more information.
    inline void CEdit::SetSel(DWORD selection, BOOL isScrolled) const
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(LOWORD(selection));
        LPARAM lparam = static_cast<LPARAM>(HIWORD(selection));
        SendMessage(EM_SETSEL, wparam, lparam);
        if (isScrolled)
            SendMessage(EM_SCROLLCARET, 0, 0);
    }

    // Selects a range of characters in the edit control by setting the starting
    // and ending positions to be selected.
    // Refer to EM_SETSEL in the Windows API documentation for more information.
    inline void CEdit::SetSel(int startChar, int endChar, BOOL isScrolled) const
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(startChar);
        LPARAM lparam = static_cast<LPARAM>(endChar);
        SendMessage(EM_SETSEL, wparam, lparam);
        if (isScrolled)
            SendMessage(EM_SCROLLCARET, 0, 0);
    }

    // Sets tab-stop positions in the multi-line edit control.
    // Refer to EM_SETTABSTOPS in the Windows API documentation for more information.
    inline BOOL CEdit::SetTabStops(int tabStops, LPINT pTabStopsArray) const
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(tabStops);
        LPARAM lparam = reinterpret_cast<LPARAM>(pTabStopsArray);
        return static_cast<BOOL>(SendMessage(EM_SETTABSTOPS, wparam, lparam));
    }

    // Sets tab-stop positions in the multi-line edit control.
    // Refer to EM_SETTABSTOPS in the Windows API documentation for more information.
    inline BOOL CEdit::SetTabStops() const
    {
        assert(IsWindow());
        return static_cast<BOOL>(SendMessage(EM_SETTABSTOPS, 0, 0));
    }

    // Sets tab-stop positions in the multi-line edit control.
    // Refer to EM_SETTABSTOPS in the Windows API documentation for more information.
    inline BOOL CEdit::SetTabStops(const int& cxEachStop) const
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(1);
        LPARAM lparam = reinterpret_cast<LPARAM>(&cxEachStop);
        return static_cast<BOOL>(SendMessage(EM_SETTABSTOPS, wparam, lparam));
    }

    // Removes any text that was just inserted or inserts any deleted characters and sets the selection to the inserted text.
    // Refer to EM_UNDO in the Windows API documentation for more information.
    inline void CEdit::Undo() const
    {
        assert(IsWindow());
        SendMessage(EM_UNDO, 0, 0);
    }


    ////////////////////////////////////////
    // Definitions for the CListbox class
    //

    // Adds a string to a list box and returns its index.
    // Refer to LB_ADDSTRING in the Windows API documentation for more information.
    inline int CListBox::AddString(LPCTSTR string) const
    {
        assert(IsWindow());
        LPARAM lparam = reinterpret_cast<LPARAM>(string);
        return static_cast<int>(SendMessage(LB_ADDSTRING, 0, lparam));
    }

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

    // Removes a string from a list box and returns the number of strings remaining in the list.
    // Refer to LB_DELETESTRING in the Windows API documentation for more information.
    inline int CListBox::DeleteString(UINT index) const
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(index);
        return static_cast<int>(SendMessage(LB_DELETESTRING, wparam, 0));
    }

    // Adds a list of filenames to a list box and returns the index of the last filename added.
    // Refer to LB_DIR in the Windows API documentation for more information.
    inline int CListBox::Dir(UINT attr, LPCTSTR wildCard) const
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(attr);
        LPARAM lparam = reinterpret_cast<LPARAM>(wildCard);
        return static_cast<int>(SendMessage(LB_DIR, wparam, lparam));
    }

    // Returns the index of the first string in the list box that begins with a specified string.
    // Refer to LB_FINDSTRING in the Windows API documentation for more information.
    inline int CListBox::FindString(int startAfter, LPCTSTR string) const
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(startAfter);
        LPARAM lparam = reinterpret_cast<LPARAM>(string);
        return static_cast<int>(SendMessage(LB_FINDSTRING, wparam, lparam));
    }

    // Returns the index of the string in the list box that is equal to a specified string.
    // Refer to LB_FINDSTRINGEXACT in the Windows API documentation for more information.
    inline int CListBox::FindStringExact(int indexStart, LPCTSTR findString) const
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(indexStart);
        LPARAM lparam = reinterpret_cast<LPARAM>(findString);
        return static_cast<int>(SendMessage(LB_FINDSTRINGEXACT, wparam, lparam));
    }

    // Inserts a string at a specified index in a list box.
    // Refer to LB_INSERTSTRING in the Windows API documentation for more information.
    inline int CListBox::InsertString(int index, LPCTSTR string) const
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(index);
        LPARAM lparam = reinterpret_cast<LPARAM>(string);
        return static_cast<int>(SendMessage(LB_INSERTSTRING, wparam, lparam));
    }

    // Returns the index of the item that the mouse last selected.
    // Refer to LB_GETANCHORINDEX in the Windows API documentation for more information.
    inline int CListBox::GetAnchorIndex() const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(LB_GETANCHORINDEX, 0, 0));
    }

    // Returns the index of the item that has the focus rectangle.
    // Refer to LB_GETCARETINDEX in the Windows API documentation for more information.
    inline int CListBox::GetCaretIndex() const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(LB_GETCARETINDEX, 0, 0));
    }

    // Returns the number of items in the list box.
    // Refer to LB_GETCOUNT in the Windows API documentation for more information.
    inline int CListBox::GetCount() const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(LB_GETCOUNT, 0, 0));
    }

    // Returns the index of the currently selected item.
    // Refer to LB_GETCURSEL in the Windows API documentation for more information.
    inline int CListBox::GetCurSel() const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(LB_GETCURSEL, 0, 0));
    }

    // Returns the scrollable width, in pixels, of a list box.
    // Refer to LB_GETHORIZONTALEXTENT in the Windows API documentation for more information.
    inline int CListBox::GetHorizontalExtent() const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(LB_GETHORIZONTALEXTENT, 0, 0));
    }

    // Returns the value associated with the specified item.
    // Refer to LB_GETITEMDATA in the Windows API documentation for more information.
    inline DWORD CListBox::GetItemData(int index) const
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(index);
        return static_cast<DWORD>(SendMessage(LB_GETITEMDATA, wparam, 0));
    }

    // Returns the value associated with the specified item.
    // Refer to LB_GETITEMDATA in the Windows API documentation for more information.
    inline void* CListBox::GetItemDataPtr(int index) const
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(index);
        return reinterpret_cast<void*>(SendMessage(LB_GETITEMDATA, wparam, 0));
    }

    // Returns the height, in pixels, of an item in a list box.
    // Refer to LB_GETITEMHEIGHT in the Windows API documentation for more information.
    inline int CListBox::GetItemHeight(int index) const
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(index);
        return static_cast<int>(SendMessage(LB_GETITEMHEIGHT, wparam, 0));
    }

    // Retrieves the client coordinates of the specified list box item.
    // Refer to LB_GETITEMRECT in the Windows API documentation for more information.
    inline int CListBox::GetItemRect(int index, RECT& rc) const
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(index);
        LPARAM lparam = reinterpret_cast<LPARAM>(&rc);
        return static_cast<int>(SendMessage(LB_GETITEMRECT, wparam, lparam));
    }

    // Retrieves the locale of the list box. The high-order word contains the country/region code
    // and the low-order word contains the language identifier.
    // Refer to LB_GETLOCALE in the Windows API documentation for more information.
    inline LCID CListBox::GetLocale() const
    {
        assert(IsWindow());
        return static_cast<LCID>(SendMessage(LB_GETLOCALE, 0, 0));
    }

    // Returns the selection state of a list box item.
    // Refer to LB_GETSEL in the Windows API documentation for more information.
    inline int CListBox::GetSel(int index) const
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(index);
        return static_cast<int>(SendMessage(LB_GETSEL, wparam, 0));
    }

    // Returns the number of selected items in a multiple-selection list box.
    // Refer to LB_GETSELCOUNT in the Windows API documentation for more information.
    inline int CListBox::GetSelCount() const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(LB_GETSELCOUNT, 0, 0));
    }

    // Creates an array of the indexes of all selected items in a multiple-selection list box
    // and returns the total number of selected items.
    // Refer to LB_GETSELITEMS in the Windows API documentation for more information.
    inline int CListBox::GetSelItems(int maxItems, LPINT pIndexArray) const
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(maxItems);
        LPARAM lparam = reinterpret_cast<LPARAM>(pIndexArray);
        return static_cast<int>(SendMessage(LB_GETSELITEMS, wparam, lparam));
    }

    // Retrieves the string associated with a specified item and the length of the string.
    // Refer to LB_GETTEXT in the Windows API documentation for more information.
    inline int CListBox::GetText(int index, LPTSTR buffer) const
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(index);
        LPARAM lparam = reinterpret_cast<LPARAM>(buffer);
        return static_cast<int>(SendMessage(LB_GETTEXT, wparam, lparam));
    }

    // Returns the length, in characters, of the string associated with a specified item.
    // Refer to LB_GETTEXTLEN in the Windows API documentation for more information.
    inline int CListBox::GetTextLen(int index) const
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(index);
        return static_cast<int>(SendMessage( LB_GETTEXTLEN, wparam, 0));
    }

    // Returns the index of the first visible item in a list box.
    // Refer to LB_GETTOPINDEX in the Windows API documentation for more information.
    inline int CListBox::GetTopIndex() const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(LB_GETTOPINDEX, 0, 0));
    }

    // Retrieves the zero-based index of the item nearest the specified point in a list box.
    // Refer to LB_ITEMFROMPOINT in the Windows API documentation for more information.
    inline UINT CListBox::ItemFromPoint(CPoint pt, BOOL& isOutside) const
    {
        assert(IsWindow());
        DWORD result = static_cast<DWORD>(SendMessage(LB_ITEMFROMPOINT, 0, MAKELPARAM(pt.x, pt.y)));
        isOutside = !!HIWORD(result);
        return LOWORD(result);
    }

    // Handle messages reflected back from the parent window.
    // Override this function in your derived class to handle these special messages:
    // WM_COMMAND, WM_CTLCOLORBTN, WM_CTLCOLOREDIT, WM_CTLCOLORDLG, WM_CTLCOLORLISTBOX,
    // WM_CTLCOLORSCROLLBAR, WM_CTLCOLORSTATIC, WM_CHARTOITEM,  WM_VKEYTOITEM,
    // WM_HSCROLL, WM_VSCROLL, WM_DRAWITEM, WM_MEASUREITEM, WM_DELETEITEM,
    // WM_COMPAREITEM, WM_PARENTNOTIFY.
    inline LRESULT CListBox::OnMessageReflect(UINT msg, WPARAM, LPARAM lparam)
    {
        switch (msg)
        {
        case WM_COMPAREITEM:
        {
            LPCOMPAREITEMSTRUCT pCompareItemStruct = (LPCOMPAREITEMSTRUCT)lparam;
            return CompareItem(pCompareItemStruct);
        }
        }

        return 0;   // Allow other messages to be handled elsewhere.
    }

    // Set the window class.
    inline void CListBox::PreRegisterClass(WNDCLASS& wc)
    {
        wc.lpszClassName = _T("ListBox");
    }

    // Removes all items from a list box.
    // Refer to LB_RESETCONTENT in the Windows API documentation for more information.
    inline void CListBox::ResetContent() const
    {
        assert(IsWindow());
        SendMessage(LB_RESETCONTENT, 0, 0);
    }

    // Selects the first string it finds that matches a specified prefix.
    // Refer to LB_SELECTSTRING in the Windows API documentation for more information.
    inline int CListBox::SelectString(int startAfter, LPCTSTR string) const
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(startAfter);
        LPARAM lparam = reinterpret_cast<LPARAM>(string);
        return static_cast<int>(SendMessage(LB_SELECTSTRING, wparam, lparam));
    }

    // Selects a specified range of items in a list box.
    // Refer to LB_SELITEMRANGEEX in the Windows API documentation for more information.
    inline int CListBox::SelItemRange(BOOL isSelected, int firstItem, int lastItem) const
    {
        assert(IsWindow());

        if (isSelected)
        {
            WPARAM wparam = static_cast<WPARAM>(firstItem);
            LPARAM lparam = static_cast<LPARAM>(lastItem);
            return static_cast<int>(SendMessage(LB_SELITEMRANGEEX, wparam, lparam));
        }
        else
        {
            WPARAM wparam = static_cast<WPARAM>(lastItem);
            LPARAM lparam = static_cast<LPARAM>(firstItem);
            return static_cast<int>(SendMessage(LB_SELITEMRANGEEX, wparam, lparam));
        }
    }

    // Sets the item that the mouse last selected to a specified item.
    // Refer to LB_SETANCHORINDEX in the Windows API documentation for more information.
    inline void CListBox::SetAnchorIndex(int index) const
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(index);
        SendMessage(LB_SETANCHORINDEX, wparam, 0);
    }

    // Sets the focus rectangle to a specified list box item.
    // Refer to LB_SETCARETINDEX in the Windows API documentation for more information.
    inline int CListBox::SetCaretIndex(int index, BOOL partialScroll) const
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(index);
        LPARAM lparam = static_cast<LPARAM>(MAKELONG(partialScroll, 0));
        return static_cast<int>(SendMessage(LB_SETCARETINDEX, wparam, lparam));
    }

    // Sets the width, in pixels, of all columns in a list box.
    // Refer to LB_SETCOLUMNWIDTH in the Windows API documentation for more information.
    inline void CListBox::SetColumnWidth(int cxWidth) const
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(cxWidth);
        SendMessage(LB_SETCOLUMNWIDTH, wparam, 0);
    }

    // Selects a specified list box item.
    // Refer to LB_SETCURSEL in the Windows API documentation for more information.
    inline int CListBox::SetCurSel(int select) const
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(select);
        return static_cast<int>(SendMessage(LB_SETCURSEL, wparam, 0));
    }

    // Sets the scrollable width, in pixels, of a list box.
    // Refer to LB_SETHORIZONTALEXTENT in the Windows API documentation for more information.
    inline void CListBox::SetHorizontalExtent(int cxExtent) const
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(cxExtent);
        SendMessage(LB_SETHORIZONTALEXTENT, wparam, 0);
    }

    // Associates a value with a list box item.
    // Refer to LB_SETITEMDATA in the Windows API documentation for more information.
    inline int CListBox::SetItemData(int index, DWORD itemData) const
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(index);
        LPARAM lparam = static_cast<LPARAM>(itemData);
        return static_cast<int>(SendMessage(LB_SETITEMDATA, wparam, lparam));
    }

    // Associates a value with a list box item.
    // Refer to LB_SETITEMDATA in the Windows API documentation for more information.
    inline int CListBox::SetItemDataPtr(int index, void* pData) const
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(index);
        LPARAM lparam = reinterpret_cast<LPARAM>(pData);
        return static_cast<int>(SendMessage(LB_SETITEMDATA, wparam, lparam));
    }

    // Sets the height, in pixels, of an item or items in a list box.
    // Refer to LB_SETITEMHEIGHT in the Windows API documentation for more information.
    inline int CListBox::SetItemHeight(int index, UINT cyItemHeight) const
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(index);
        LPARAM lparam = static_cast<LPARAM>(MAKELONG(cyItemHeight, 0));
        return static_cast<int>(SendMessage(LB_SETITEMHEIGHT, wparam, lparam));
    }

    // Sets the locale of a list box and returns the previous locale identifier.
    // Refer to LB_SETLOCALE in the Windows API documentation for more information.
    inline LCID CListBox::SetLocale(LCID newLocale) const
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(newLocale);
        return static_cast<LCID>(SendMessage(LB_SETLOCALE, wparam, 0));
    }

    // Selects an item in a multiple-selection list box.
    // Refer to LB_SETSEL in the Windows API documentation for more information.
    inline int CListBox::SetSel(int index, BOOL isSelected) const
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(isSelected);
        LPARAM lparam = static_cast<LPARAM>(index);
        return static_cast<int>(SendMessage(LB_SETSEL, wparam, lparam));
    }

    // Sets the tab stops to those specified in a specified array.
    // Refer to LB_SETTABSTOPS in the Windows API documentation for more information.
    inline BOOL CListBox::SetTabStops(int tabStops, LPINT pTabStopsArray) const
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(tabStops);
        LPARAM lparam = reinterpret_cast<LPARAM>(pTabStopsArray);
        return static_cast<BOOL>(SendMessage(LB_SETTABSTOPS, wparam, lparam));
    }

    // Sets the tab stops to those specified in a specified array.
    // Refer to LB_SETTABSTOPS in the Windows API documentation for more information.
    inline void CListBox::SetTabStops() const
    {
        assert(IsWindow());
        SendMessage(LB_SETTABSTOPS, 0, 0);
    }

    inline BOOL CListBox::SetTabStops(const int& cxEachStop) const
    // Sets the tab stops to those specified in a specified array.
    // Refer to LB_SETTABSTOPS in the Windows API documentation for more information.
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(1);
        LPARAM lparam = reinterpret_cast<LPARAM>(&cxEachStop);
        return static_cast<BOOL>(SendMessage(LB_SETTABSTOPS, wparam, lparam));
    }

    // Scrolls the list box so the specified item is at the top of the visible range.
    // Refer to LB_SETTOPINDEX in the Windows API documentation for more information.
    inline int CListBox::SetTopIndex(int index) const
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(index);
        return static_cast<int>(SendMessage(LB_SETTOPINDEX, wparam, 0));
    }


    ////////////////////////////////////////
    // Definitions for the CStatic class
    //

    // Returns the handle to the bitmap for the static control set by CStatic::SetBitmap.
    // Refer to STM_GETIMAGE in the Windows API documentation for more information.
    inline HBITMAP CStatic::GetBitmap() const
    {
        assert(IsWindow());
        return reinterpret_cast<HBITMAP>(SendMessage(STM_GETIMAGE, IMAGE_BITMAP, 0));
    }

    // Returns the handle to the cursor for the static control set by CStatic::SetCursor.
    // Refer to STM_GETIMAGE in the Windows API documentation for more information.
    inline HCURSOR CStatic::GetCursor() const
    {
        assert(IsWindow());
        return reinterpret_cast<HCURSOR>(SendMessage(STM_GETIMAGE, IMAGE_CURSOR, 0));
    }

    // Returns the handle to the enhanced meta-file for the static control set by CStatic::SetEnhMetaFile.
    // Refer to STM_GETIMAGE in the Windows API documentation for more information.
    inline HENHMETAFILE CStatic::GetEnhMetaFile() const
    {
        assert(IsWindow());
        return reinterpret_cast<HENHMETAFILE>(SendMessage(STM_GETIMAGE, IMAGE_ENHMETAFILE, 0));
    }

    // Returns the handle to the icon for the static control set by CStatic::SetIcon.
    // Refer to STM_GETIMAGE in the Windows API documentation for more information.
    inline HICON CStatic::GetIcon() const
    {
        assert(IsWindow());
        return reinterpret_cast<HICON>(SendMessage(STM_GETIMAGE, IMAGE_ICON, 0));
    }

    // Set the window class.
    inline void CStatic::PreRegisterClass(WNDCLASS& wc)
    {
        wc.lpszClassName = _T("Static");
    }

    // Associates a new bitmap with the static control.
    // The bitmap will be drawn in the upper-left corner, and the static
    // control will be resized to the size of the bitmap.
    // This function requires the SS_BITMAP style.
    // Refer to STM_SETIMAGE in the Windows API documentation for more information.
    inline HBITMAP CStatic::SetBitmap(HBITMAP bitmap) const
    {
        assert(IsWindow());
        LPARAM lparam = reinterpret_cast<LPARAM>(bitmap);
        return reinterpret_cast<HBITMAP>(SendMessage(STM_SETIMAGE, IMAGE_BITMAP, lparam));
    }

    // Associates a new cursor image with the static control.
    // The cursor will be drawn in the upper-left corner and the static
    // control will be resized to the size of the cursor.
    // This function requires the SS_ICON style
    // Refer to STM_SETIMAGE in the Windows API documentation for more information.
    inline HCURSOR CStatic::SetCursor(HCURSOR cursor) const
    {
        assert(IsWindow());
        LPARAM lparam = reinterpret_cast<LPARAM>(cursor);
        return reinterpret_cast<HCURSOR>(SendMessage(STM_SETIMAGE, IMAGE_CURSOR, lparam));
    }

    // Associates a new enhanced metafile image with the static control.
    // The enhanced metafile will be drawn in the upper-left corner and the static
    // control will be resized to the size of the enhanced metafile.
    // This function requires the SS_ENHMETAFILE style
    // Refer to STM_SETIMAGE in the Windows API documentation for more information.
    inline HENHMETAFILE CStatic::SetEnhMetaFile(HENHMETAFILE metaFile) const
    {
        assert(IsWindow());
        LPARAM lparam = reinterpret_cast<LPARAM>(metaFile);
        return reinterpret_cast<HENHMETAFILE>(SendMessage(STM_SETIMAGE, IMAGE_ENHMETAFILE, lparam));
    }

    // Associates a new icon image with the static control.
    // The icon will be drawn in the upper-left corner and the static
    // control will be resized to the size of the icon.
    // This function requires the SS_ICON style
    // Refer to STM_SETIMAGE in the Windows API documentation for more information.
    inline HICON CStatic::SetIcon(HICON icon) const
    {
        assert(IsWindow());
        LPARAM lparam = reinterpret_cast<LPARAM>(icon);
        return reinterpret_cast<HICON>(SendMessage(STM_SETIMAGE, IMAGE_ICON, lparam));
    }

}

#endif  // _WIN32XX_STDCONTROLS_H_

