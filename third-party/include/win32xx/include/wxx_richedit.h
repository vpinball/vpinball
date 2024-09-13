// Win32++   Version 10.0.0
// Release Date: 9th September 2024
//
//      David Nash
//      email: dnash@bigpond.net.au
//      url: https://sourceforge.net/projects/win32-framework
//           https://github.com/DavidNash2024/Win32xx
//
//
// Copyright (c) 2005-2024  David Nash
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
// wxx_rich_edit.h
//  Declaration of the CRichEdit class

#ifndef _WIN32XX_RICHEDIT_H_
#define _WIN32XX_RICHEDIT_H_


#include "wxx_wincore.h"
#include <Richedit.h>
#include <RichOle.h>


namespace Win32xx
{

    ////////////////////////////////////////////////////////////
    // CRichEdit manages a rich edit control. Rich Edit controls
    // support plain text and rich text. Rich text can utilize
    // several fonts in a single document.
    class CRichEdit : public CWnd
    {
    public:
        CRichEdit();
        virtual ~CRichEdit() override;

        void    AppendText(LPCTSTR text) const;
        BOOL    CanPaste(UINT format = 0) const;
        BOOL    CanRedo() const;
        BOOL    CanUndo() const;
        int     CharFromPos(CPoint pt) const;
        void    Clear() const;
        void    Copy() const;
        void    Cut() const;
        BOOL    DisplayBand(const RECT& rc) const;
        void    EmptyUndoBuffer() const;
        long    FindText(DWORD flags, const FINDTEXTEX& findInfo) const;
        DWORD   FindWordBreak(UINT code, DWORD start) const;
        long    FormatRange(const FORMATRANGE& fr, BOOL display = TRUE) const;
        long    FormatRange(BOOL display = FALSE) const;
        CPoint  GetCharPos(long charIndex) const;
        DWORD   GetDefaultCharFormat(CHARFORMAT& format) const;
        DWORD   GetDefaultCharFormat(CHARFORMAT2& format) const;
        long    GetEventMask() const;
        int     GetFirstVisibleLine() const;
        IRichEditOle* GetIRichEditOle() const;
        long    GetLimitText() const;
        int     GetLine(int index, LPTSTR buffer) const;
        int     GetLine(int index, LPTSTR buffer, int maxLength) const;
        int     GetLineCount() const;
        BOOL    GetModify() const;
        UINT    GetOptions() const;
        DWORD   GetParaFormat(PARAFORMAT& format) const;
        DWORD   GetParaFormat(PARAFORMAT2& format) const;
        BOOL    GetPunctuation(UINT type, const PUNCTUATION& puncInfo) const;
        void    GetRect(RECT& rc) const;
        UNDONAMEID GetRedoName() const;
        void    GetSel(CHARRANGE& range) const;
        void    GetSel(long& startChar, long& endChar) const;
        DWORD   GetSelectionCharFormat(CHARFORMAT& cf) const;
        DWORD   GetSelectionCharFormat(CHARFORMAT2& cf) const;
        WORD    GetSelectionType() const;
        long    GetSelText(LPSTR buffer) const;
        CString GetSelText() const;
        long    GetTextLength() const;
        long    GetTextLengthEx(DWORD flags, UINT codePage = -1) const;
        UINT    GetTextMode() const;
        CString GetTextRange(int first, int last) const;
        UNDONAMEID GetUndoName() const;
        void     HideSelection(BOOL hide, BOOL isPermanent) const;
        void    LimitText(long limit = 0) const;
        long    LineFromChar(long charIndex) const;
        int     LineIndex(int lineIndex = -1) const;
        int     LineLength(int charIndex = -1) const;
        void    LineScroll(int lines) const;
        void    Paste() const;
        void    PasteSpecial(UINT clipFormat, DWORD aspect = 0, HMETAFILE mf = nullptr) const;
        CPoint  PosFromChar(UINT fromChar) const;
        BOOL    Redo() const;
        void    ReplaceSel(LPCTSTR newText, BOOL canUndo = FALSE) const;
        void    RequestResize() const;
        BOOL    SetAutoURLDetect(BOOL enable = TRUE) const;
        COLORREF SetBackgroundColor(BOOL isSysColor, COLORREF color) const;
        BOOL    SetDefaultCharFormat(CHARFORMAT& format) const;
        BOOL    SetDefaultCharFormat(CHARFORMAT2& format) const;
        DWORD   SetEventMask(DWORD eventMask) const;
        void    SetModify(BOOL isModified = TRUE) const;
        BOOL    SetOLECallback(IRichEditOleCallback* pCallback) const;
        void    SetOptions(WORD options, DWORD flags) const;
        BOOL    SetParaFormat(PARAFORMAT& pf) const;
        BOOL    SetParaFormat(PARAFORMAT2& pf) const;
        BOOL    SetPunctuation(UINT type, const PUNCTUATION& puncInfo) const;
        BOOL    SetReadOnly(BOOL isReadOnly = TRUE) const;
        void    SetRect(const RECT& rc) const;
        void    SetSel(long startChar, long endChar) const;
        void    SetSel(CHARRANGE& cr) const;
        BOOL    SetSelectionCharFormat(CHARFORMAT& cf) const;
        BOOL    SetSelectionCharFormat(CHARFORMAT2& cf) const;
        BOOL    SetTargetDevice(HDC dc, long lineWidth) const;
        BOOL    SetTextMode(UINT mode) const;
        UINT    SetUndoLimit(UINT limit) const;
        BOOL    SetWordCharFormat(CHARFORMAT& format) const;
        BOOL    SetWordCharFormat(CHARFORMAT2& format) const;
        void    StopGroupTyping() const;
        long    StreamIn(int format, EDITSTREAM& stream) const;
        long    StreamOut(int format, EDITSTREAM& stream) const;
        BOOL    Undo() const;

    protected:
        virtual void PreCreate(CREATESTRUCT& cs) override;
        virtual void PreRegisterClass(WNDCLASS& wc) override;

    private:
        CRichEdit(const CRichEdit&) = delete;
        CRichEdit& operator=(const CRichEdit&) = delete;

        HMODULE m_rich1;
        HMODULE m_rich2;
        HMODULE m_rich4_1;
    };

}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


namespace Win32xx
{

    ///////////////////////////////////////////////
    // Definitions of the CRichEdit class
    //

    inline CRichEdit::CRichEdit()
    {
        // History of the Rich Edit control
        // --------------------------------
        // Windows 95   Includes only Rich Edit 1.0.
        // Windows 98   Includes Rich Edit 1.0 and 2.0.
        // Windows 2000 Includes Rich Edit 3.0 with a Rich Edit 1.0 emulator.
        // Windows XP   Includes Rich Edit 4.1, and Rich Edit 3.0 with a Rich Edit 1.0 emulator.

        CString system;
        ::GetSystemDirectory(system.GetBuffer(MAX_PATH), MAX_PATH);
        system.ReleaseBuffer();

        // Load RichEdit version 1.0. This registers the "RICHEDIT" class.
        // The "RICHEDIT" class is often used in dialogs.
        m_rich1 = ::LoadLibrary(system + _T("\\riched32.dll"));
        if (m_rich1 == nullptr)
            throw CNotSupportedException(GetApp()->MsgRichEditDll());

        // Load RichEdit version 3.0. This registers the "RICHEDIT20A"
        // and "RICHEDIT20W" classes. It is used when UNICODE isn't defined.
        m_rich2 = ::LoadLibrary(system + _T("\\riched20.dll"));
        if (m_rich2 == nullptr)
            throw CNotSupportedException(GetApp()->MsgRichEditDll());

        // Load RichEdit version 4.1. This registers the "RICHEDIT50W" class.
        // RichEdit version 4.1 requires Unicode.
        m_rich4_1 = ::LoadLibrary(system + _T("\\Msftedit.dll"));
        if (m_rich4_1 == nullptr)
            throw CNotSupportedException(GetApp()->MsgRichEditDll());
    }

    inline CRichEdit::~CRichEdit()
    {
        // Destroy the window before freeing the DLL.
        if (IsWindow())
            ::DestroyWindow(*this);

        ::FreeLibrary(m_rich1);
        if (m_rich2)
            ::FreeLibrary(m_rich2);

        if (m_rich4_1)
            ::FreeLibrary(m_rich4_1);
    }

    // Set the default window styles.
    inline void CRichEdit::PreCreate(CREATESTRUCT& cs)
    {
        cs.style = WS_CHILD | WS_VISIBLE | ES_MULTILINE;
    }

    // Set the window class
    inline void CRichEdit::PreRegisterClass(WNDCLASS& wc)
    {
        // Use the latest version of RichEdit available.

        // For RichEdit version 1.0, 2.0 and 3.0.
        wc.lpszClassName = RICHEDIT_CLASS;

        // For RichEdit version 4.1. Requires Unicode.
#if defined UNICODE
        if (m_rich4_1 != nullptr)
            wc.lpszClassName = MSFTEDIT_CLASS;
#else
        TRACE("\n*** WARNING: Using an old version of the RichEdit control ***\n\n");
#endif

    }

    // Adds text to the end of the document.
    inline void CRichEdit::AppendText(LPCTSTR text) const
    {
        LRESULT position = SendMessage(WM_GETTEXTLENGTH, 0, 0);
        WPARAM wparam = static_cast<WPARAM>(position);
        LPARAM lparam = static_cast<LPARAM>(position);
        SendMessage(EM_SETSEL, wparam, lparam);
        lparam = reinterpret_cast<LPARAM>(text);
        SendMessage(EM_REPLACESEL, 0, lparam);
    }

    // Determines whether a rich edit control can paste a specified clipboard format.
    // Refer to EM_CANPASTE in the Windows API documentation for more information.
    inline BOOL CRichEdit::CanPaste(UINT format) const
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(format);
        return static_cast<BOOL>(SendMessage(EM_CANPASTE, wparam, 0));
    }

    // Determines whether there are any actions in the control redo queue.
    // Refer to EM_CANREDO in the Windows API documentation for more information.
    inline BOOL CRichEdit::CanRedo() const
    {
        assert(IsWindow());
        return static_cast<BOOL>(SendMessage(EM_CANREDO, 0, 0));
    }

    // Determines whether there are any actions in an edit control's undo queue.
    // Refer to EM_CANUNDO in the Windows API documentation for more information.
    inline BOOL CRichEdit::CanUndo() const
    {
        assert(IsWindow());
        return static_cast<BOOL>(SendMessage(EM_CANUNDO, 0, 0));
    }

    // Gets information about the character closest to a specified point in the client area of an edit control.
    // Refer to EM_CHARFROMPOS in the Windows API documentation for more information.
    inline int CRichEdit::CharFromPos(CPoint pt) const
    {
        assert(IsWindow());
        WPARAM wparam = reinterpret_cast<WPARAM>(&pt);
        return static_cast<int>(SendMessage(EM_CHARFROMPOS, wparam, 0));
    }

    // Delete (clear) the current selection
    // Refer to WM_CLEAR in the Windows API documentation for more information.
    inline void CRichEdit::Clear() const
    {
        assert(IsWindow());
        SendMessage(WM_CLEAR, 0, 0);
    }

    // Copy the current selection to the clipboard in CF_TEXT format.
    // Refer to WM_COPY in the Windows API documentation for more information.
    inline void CRichEdit::Copy() const
    {
        assert(IsWindow());
        SendMessage(WM_COPY, 0, 0);
    }

    // Delete (cut) the current selection, if any, in the edit control and copy
    // the deleted text to the clipboard in CF_TEXT format.
    // Refer to WM_CUT in the Windows API documentation for more information.
    inline void CRichEdit::Cut() const
    {
        assert(IsWindow());
        SendMessage(WM_CUT, 0, 0);
    }

    // Displays a portion of the contents of a rich edit control, as previously
    // formatted for a device using the EM_FORMATRANGE message.
    // Refer to EM_DISPLAYBAND in the Windows API documentation for more information.
    inline BOOL CRichEdit::DisplayBand(const RECT& rc) const
    {
        assert(IsWindow());
        LPARAM lparam = reinterpret_cast<LPARAM>(&rc);
        return static_cast<BOOL>(SendMessage(EM_DISPLAYBAND, 0, lparam));
    }

    // Resets the undo flag of the rich edit control. The undo flag is set
    // whenever an operation within the edit control can be undone.
    // Refer to EM_EMPTYUNDOBUFFER in the Windows API documentation for more information.
    inline void CRichEdit::EmptyUndoBuffer() const
    {
        assert(IsWindow());
        SendMessage(EM_EMPTYUNDOBUFFER, 0, 0);
    }

    // Finds text within the rich edit control.
    // Refer to EM_FINDTEXTEX in the Windows API documentation for more information.
    inline long CRichEdit::FindText(DWORD flags, const FINDTEXTEX& findInfo) const
    {
        assert(IsWindow());

    #ifdef UNICODE
        UINT em_findText = EM_FINDTEXTEXW;
     #else
        UINT em_findText = EM_FINDTEXTEX;
     #endif

        WPARAM wparam = static_cast<WPARAM>(flags);
        LPARAM lparam = reinterpret_cast<LPARAM>(&findInfo);
        return static_cast<long>(SendMessage(em_findText, wparam, lparam));
    }

    // Finds the next word break before or after the specified character position
    // or retrieves information about the character at that position.
    // Refer to EM_FINDWORDBREAK in the Windows API documentation for more information.
    inline DWORD CRichEdit::FindWordBreak(UINT code, DWORD start) const
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(code);
        LPARAM lparam = static_cast<LPARAM>(start);
        return static_cast<DWORD>(SendMessage(EM_FINDWORDBREAK, wparam, lparam));
    }

    // Formats a range of text in a rich edit control for a specific device (e.g. printer).
    // Refer to EM_FORMATRANGE in the Windows API documentation for more information.
    inline long CRichEdit::FormatRange(const FORMATRANGE& fr, BOOL display /* = TRUE */) const
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(display);
        LPARAM lparam = reinterpret_cast<LPARAM>(&fr);
        return static_cast<long>(SendMessage(EM_FORMATRANGE, wparam, lparam));
    }

    // Free format information cached by the control.
    // Refer to EM_FORMATRANGE in the Windows API documentation for more information.
    inline long CRichEdit::FormatRange(BOOL display /* = FALSE */) const
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(display);
        return static_cast<long>(SendMessage(EM_FORMATRANGE, wparam, 0));
    }

    // Retrieves the client area coordinates of a specified character.
    // Refer to EM_POSFROMCHAR in the Windows API documentation for more information.
    inline CPoint CRichEdit::GetCharPos(long charIndex) const
    {
        assert(IsWindow());
        CPoint pt;
        WPARAM wparam = reinterpret_cast<WPARAM>(&pt);
        LPARAM lparam = static_cast<LPARAM>(charIndex);
        SendMessage(EM_POSFROMCHAR, wparam, lparam);
        return pt;
    }

    // Retrieves the current default character formatting attributes.
    // Refer to EM_GETCHARFORMAT in the Windows API documentation for more information.
    inline DWORD CRichEdit::GetDefaultCharFormat(CHARFORMAT& format) const
    {
        assert(IsWindow());
        LPARAM lparam = reinterpret_cast<LPARAM>(&format);
        return static_cast<DWORD>(SendMessage(EM_GETCHARFORMAT, SCF_DEFAULT, lparam));
    }

    // Retrieves the current default character formatting attributes.
    // Refer to EM_GETCHARFORMAT in the Windows API documentation for more information.
    inline DWORD CRichEdit::GetDefaultCharFormat(CHARFORMAT2& cf) const
    {
        assert(IsWindow());
        LPARAM lparam = reinterpret_cast<LPARAM>(&cf);
        return static_cast<DWORD>(SendMessage(EM_GETCHARFORMAT, SCF_DEFAULT, lparam));
    }

    // Retrieves the event mask. The event mask specifies which notification
    // messages the control sends to its parent window.
    // Refer to EM_GETEVENTMASK in the Windows API documentation for more information.
    inline long CRichEdit::GetEventMask() const
    {
        assert(IsWindow());
        return static_cast<long>(SendMessage(EM_GETEVENTMASK, 0, 0));
    }

    // Gets the zero-based index of the uppermost visible line.
    // Refer to EM_GETFIRSTVISIBLELINE in the Windows API documentation for more information.
    inline int CRichEdit::GetFirstVisibleLine() const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(EM_GETFIRSTVISIBLELINE, 0, 0));
    }

    // Retrieves an IRichEditOle object that a client can use to access the
    // rich edit control's Component Object Model (COM) functionality.
    // Refer to EM_GETOLEINTERFACE in the Windows API documentation for more information.
    inline IRichEditOle* CRichEdit::GetIRichEditOle() const
    {
        assert(IsWindow());

        IRichEditOle* pRichEditOle = nullptr;
        LPARAM lparam = reinterpret_cast<LPARAM>(pRichEditOle);
        SendMessage(EM_GETOLEINTERFACE, 0, lparam);
        return pRichEditOle;
    }

    // Gets the current text limit for the edit control.
    // Refer to EM_GETLIMITTEXT in the Windows API documentation for more information.
    inline long CRichEdit::GetLimitText() const
    {
        assert(IsWindow());
        return static_cast<long>(SendMessage(EM_GETLIMITTEXT, 0, 0));
    }

    // Copies a line of text from the rich edit control and places it in the specified buffer.
    // buffer is a pointer to the buffer that receives a copy of the line. Before sending the message,
    // set the first word of this buffer to the size, in TCHARs, of the buffer.
    // The copied line does not contain a terminating null character.
    // Refer to EM_GETLINE in the Windows API documentation for more information.
    inline int CRichEdit::GetLine(int index, LPTSTR buffer) const
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(index);
        LPARAM lparam = reinterpret_cast<LPARAM>(buffer);
        return static_cast<int>(SendMessage(EM_GETLINE, wparam, lparam));
    }

    // Copies a line of text from the rich edit control and places it in the specified buffer.
    // The copied line does not contain a terminating null character.
    // Refer to EM_GETLINE in the Windows API documentation for more information.
    inline int CRichEdit::GetLine(int index, LPTSTR buffer, int maxLength) const
    {
        assert(IsWindow());
        *reinterpret_cast<LPWORD>(buffer) = static_cast<WORD>(maxLength);
        WPARAM wparam = static_cast<WPARAM>(index);
        LPARAM lparam = reinterpret_cast<LPARAM>(buffer);
        return static_cast<int>(SendMessage(EM_GETLINE, wparam, lparam));
    }

    // Gets the number of lines in a multi-line edit control.
    // Refer to EM_GETLINECOUNT in the Windows API documentation for more information.
    inline int CRichEdit::GetLineCount() const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(EM_GETLINECOUNT, 0, 0));
    }

    // Retrieves a flag than indicates whether the contents of the edit control have been modified.
    // Refer to EM_GETMODIFY in the Windows API documentation for more information.
    inline BOOL CRichEdit::GetModify() const
    {
        assert(IsWindow());
        return static_cast<BOOL>(SendMessage(EM_GETMODIFY, 0, 0));
    }

    // Retrieves the rich edit control options.
    // Refer to EM_GETOPTIONS in the Windows API documentation for more information.
    inline UINT CRichEdit::GetOptions() const
    {
        assert(IsWindow());
        return static_cast<UINT>(SendMessage(EM_GETOPTIONS, 0, 0));
    }

    // Retrieves the paragraph formatting of the current selection.
    // Refer to EM_GETPARAFORMAT in the Windows API documentation for more information.
    inline DWORD CRichEdit::GetParaFormat(PARAFORMAT& format) const
    {
        assert(IsWindow());
        LPARAM lparam = reinterpret_cast<LPARAM>(&format);
        return static_cast<DWORD>(SendMessage(EM_GETPARAFORMAT, 0, lparam));
    }

    // Retrieves the paragraph formatting of the current selection.
    // Refer to EM_GETPARAFORMAT in the Windows API documentation for more information.
    inline DWORD CRichEdit::GetParaFormat(PARAFORMAT2& pf) const
    {
        assert(IsWindow());
        LPARAM lparam = reinterpret_cast<LPARAM>(&pf);
        return static_cast<DWORD>(SendMessage(EM_GETPARAFORMAT, 0, lparam));
    }

    // Retrieves the current punctuation characters for the rich edit control.
    // This is available only in Asian-language versions of the operating system.
    // Refer to EM_GETPUNCTUATION in the Windows API documentation for more information.
    inline BOOL CRichEdit::GetPunctuation(UINT type, const PUNCTUATION& puncInfo) const
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(type);
        LPARAM lparam = reinterpret_cast<LPARAM>(&puncInfo);
        return static_cast<BOOL>(SendMessage(EM_GETPUNCTUATION, wparam, lparam));
    }

    // Retrieves the formatting rectangle. The formatting rectangle is the limiting
    // rectangle into which text can be drawn.
    // Refer to EM_GETRECT in the Windows API documentation for more information.
    inline void CRichEdit::GetRect(RECT& rc) const
    {
        assert(IsWindow());
        LPARAM lparam = reinterpret_cast<LPARAM>(&rc);
        SendMessage(EM_GETRECT, 0, lparam);
    }

    // Retrieves the type of the next action, if any, in the control's redo queue.
    // Refer to EM_GETREDONAME in the Windows API documentation for more information.
    inline UNDONAMEID CRichEdit::GetRedoName() const
    {
        assert(IsWindow());

        return static_cast<UNDONAMEID>(SendMessage(EM_GETREDONAME, 0, 0));
    }

    // Retrieves the starting and ending character positions of the selection.
    // Refer to EM_EXGETSEL in the Windows API documentation for more information.
    inline void CRichEdit::GetSel(CHARRANGE& range) const
    {
        assert(IsWindow());
        LPARAM lparam = reinterpret_cast<LPARAM>(&range);
        SendMessage(EM_EXGETSEL, 0, lparam);
    }

    // Gets the starting and ending positions of the current selection.
    // Refer to EM_EXGETSEL in the Windows API documentation for more information.
    inline void CRichEdit::GetSel(long& startChar, long& endChar) const
    {
        assert(IsWindow());
        CHARRANGE range{};
        LPARAM lparam = reinterpret_cast<LPARAM>(&range);
        SendMessage(EM_EXGETSEL, 0, lparam);
        startChar = range.cpMin;
        endChar   = range.cpMax;
    }

    // Retrieves the character formatting attributes in the current selection.
    // Refer to EM_GETCHARFORMAT in the Windows API documentation for more information.
    inline DWORD CRichEdit::GetSelectionCharFormat(CHARFORMAT& format) const
    {
        assert(IsWindow());
        LPARAM lparam = reinterpret_cast<LPARAM>(&format);
        return static_cast<DWORD>(SendMessage(EM_GETCHARFORMAT, SCF_SELECTION, lparam));
    }

    // Retrieves the character formatting attributes in the current selection.
    // Refer to EM_GETCHARFORMAT in the Windows API documentation for more information.
    inline DWORD CRichEdit::GetSelectionCharFormat(CHARFORMAT2& format) const
    {
        assert(IsWindow());
        LPARAM lparam = reinterpret_cast<LPARAM>(&format);
        return static_cast<DWORD>(SendMessage(EM_GETCHARFORMAT, SCF_SELECTION, lparam));
    }

    // Retrieves the type of contents in the current selection.
    // Refer to EM_SELECTIONTYPE in the Windows API documentation for more information.
    inline WORD CRichEdit::GetSelectionType() const
    {
        assert(IsWindow());
        return static_cast<WORD>(SendMessage(EM_SELECTIONTYPE, 0, 0));
    }

    // Gets the text of the current selection.
    // Refer to EM_GETSELTEXT in the Windows API documentation for more information.
    inline long CRichEdit::GetSelText(LPSTR buffer) const
    {
        assert(IsWindow());
        LPARAM lparam = reinterpret_cast<LPARAM>(buffer);
        return static_cast<long>(SendMessage(EM_GETSELTEXT, 0, lparam));
    }

    // Gets the text of the current selection.
    // Refer to EM_GETSELTEXT in the Windows API documentation for more information.
    inline CString CRichEdit::GetSelText() const
    {
        assert(IsWindow());

        CHARRANGE cr{};
        LPARAM lparam = reinterpret_cast<LPARAM>(&cr);
        SendMessage(EM_EXGETSEL, 0, lparam);
        return GetTextRange(cr.cpMin, cr.cpMax);
    }

    // Retrieves the length of the text, in characters. Does not include the terminating null character.
    // Refer to WM_GETTEXTLENGTH in the Windows API documentation for more information.
    inline long CRichEdit::GetTextLength() const
    {
        assert(IsWindow());
        return static_cast<long>(SendMessage(WM_GETTEXTLENGTH, 0, 0));
    }

    // Returns the number of TCHARs in the rich edit control, depending on the flags specified.
    // Refer to EM_GETTEXTLENGTHEX in the Windows API documentation for more information.
    inline long CRichEdit::GetTextLengthEx(DWORD flags, UINT codePage /* = -1 */) const
    {
        assert(IsWindow());
        GETTEXTLENGTHEX gtle{};
        gtle.flags = flags;
        gtle.codepage = codePage;

        WPARAM wparam = reinterpret_cast<WPARAM>(&gtle);
        return static_cast<long>(SendMessage(EM_GETTEXTLENGTHEX, wparam, 0));
    }

    // Retrieves the current text mode and undo level.
    // Refer to EM_GETTEXTMODE in the Windows API documentation for more information.
    inline UINT CRichEdit::GetTextMode() const
    {
        assert(IsWindow());
        return static_cast<UINT>(SendMessage(EM_GETTEXTMODE, 0, 0));
    }

    // Retrieves the specified range of text.
    // Refer to EM_GETTEXTRANGE in the Windows API documentation for more information.
    inline CString CRichEdit::GetTextRange(int first, int last) const
    {
        assert(IsWindow());
        CHARRANGE range{};
        range.cpMin = first;
        range.cpMax = last;
        int lastChar = (last == -1)? GetTextLength() : last;

        CString rangeString;
        TEXTRANGE tr{};
        tr.chrg = range;
        tr.lpstrText = rangeString.GetBuffer(lastChar - first + 1);
        LPARAM lparam = reinterpret_cast<LPARAM>(&tr);
        SendMessage(EM_GETTEXTRANGE, 0, lparam);
        rangeString.ReleaseBuffer();

        return rangeString;
    }

    // Retrieves the type of the next undo action, if any.
    // Refer to EM_GETREDONAME in the Windows API documentation for more information.
    inline UNDONAMEID CRichEdit::GetUndoName() const
    {
        assert(IsWindow());
        return static_cast<UNDONAMEID>(SendMessage(EM_GETREDONAME, 0, 0));
    }

    // Shows or hides the current selection.
    // Refer to EM_HIDESELECTION in the Windows API documentation for more information.
    inline void CRichEdit::HideSelection(BOOL hide, BOOL isPermanent) const
    {
        assert(IsWindow());
        if (isPermanent)
        {
            SetOptions(ECOOP_SET, ECO_NOHIDESEL);
        }

        WPARAM wparam = static_cast<WPARAM>(hide);
        SendMessage(EM_HIDESELECTION, wparam, 0);
    }

    // Limits the amount of text a user can enter.
    // Refer to EM_EXLIMITTEXT in the Windows API documentation for more information.
    inline void CRichEdit::LimitText(long limit /* = 0 */) const
    {
        assert(IsWindow());
        LPARAM lparam = static_cast<LPARAM>(limit);
        SendMessage(EM_EXLIMITTEXT, 0, lparam);
    }

    // Determines which line contains the given character.
    // Refer to EM_EXLINEFROMCHAR in the Windows API documentation for more information.
    inline long CRichEdit::LineFromChar(long charIndex) const
    {
        assert(IsWindow());
        LPARAM lparam = static_cast<LPARAM>(charIndex);
        return static_cast<long>(SendMessage(EM_EXLINEFROMCHAR, 0, lparam));
    }

    // Retrieves the character index of a given line.
    // index -   Specifies the zero-based line number.
    //           A value of -1 specifies the current line number.
    // Refer to EM_LINEINDEX in the Windows API documentation for more information.
    inline int CRichEdit::LineIndex(int lineIndex /* = -1 */) const
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(lineIndex);
        return static_cast<int>(SendMessage(EM_LINEINDEX, wparam, 0));
    }

    // Retrieves the length of a given line containing the specified character.
    // charIndex - Specifies the character index of a character in the line whose length is to be retrieved.
    //             This parameter can be -1. In this case, the message returns the number of unselected
    //             characters on lines containing selected characters.
    // Refer to EM_LINELENGTH in the Windows API documentation for more information.
    inline int CRichEdit::LineLength(int charIndex /* = -1 */) const
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(charIndex);
        return static_cast<int>(SendMessage(EM_LINELENGTH, wparam, 0));
    }

    // Scrolls the text.
    // Refer to EM_LINESCROLL in the Windows API documentation for more information.
    inline void CRichEdit::LineScroll(int lines) const
    {
        assert(IsWindow());
        LPARAM lparam = static_cast<LPARAM>(lines);
        SendMessage(EM_LINESCROLL, 0, lparam);
    }

    // Inserts the contents of the Clipboard.
    // Refer to WM_PASTE in the Windows API documentation for more information.
    inline void CRichEdit::Paste() const
    {
        assert(IsWindow());
        SendMessage(WM_PASTE, 0, 0);
    }

    // Inserts the contents of the Clipboard in the specified data format.
    // Refer to EM_PASTESPECIAL in the Windows API documentation for more information.
    inline void CRichEdit::PasteSpecial(UINT clipFormat, DWORD aspect /* = nullptr */, HMETAFILE mf /* = nullptr */) const
    {
        assert(IsWindow());

        REPASTESPECIAL rps{};
        rps.dwAspect = aspect;
        rps.dwParam = reinterpret_cast<DWORD_PTR>(mf);
        WPARAM wparam = static_cast<WPARAM>(clipFormat);
        LPARAM lparam = reinterpret_cast<LPARAM>(&rps);
        SendMessage(EM_PASTESPECIAL, wparam, lparam);
    }

    // Retrieves the client area coordinates of a specified character.
    // Refer to EM_POSFROMCHAR in the Windows API documentation for more information.
    inline CPoint CRichEdit::PosFromChar(UINT fromChar) const
    {
        assert(IsWindow());
        LPARAM lparam = static_cast<LPARAM>(fromChar);
        DWORD dwPos = static_cast<DWORD>(SendMessage(EM_POSFROMCHAR, 0, lparam));
        return CPoint(LOWORD(dwPos), HIWORD(dwPos));
    }

    // Redoes the next action in the control's redo queue.
    // Refer to EM_REDO in the Windows API documentation for more information.
    inline BOOL CRichEdit::Redo() const
    {
        assert(IsWindow());
        return static_cast<BOOL>(SendMessage(EM_REDO, 0, 0));
    }

    // Replaces the current selection with specified text.
    // Refer to EM_REPLACESEL in the Windows API documentation for more information.
    inline void CRichEdit::ReplaceSel(LPCTSTR newText, BOOL canUndo /* = FALSE */) const
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(canUndo);
        LPARAM lparam = reinterpret_cast<LPARAM>(newText);
        SendMessage(EM_REPLACESEL, wparam, lparam);
    }

    // Forces the sending of a request resize notifications.
    // Refer to EM_REQUESTRESIZE in the Windows API documentation for more information.
    inline void CRichEdit::RequestResize() const
    {
        assert(IsWindow());
        SendMessage(EM_REQUESTRESIZE, 0, 0);
    }

    // Indicates if the auto URL detection is active.
    // Refer to EM_AUTOURLDETECT in the Windows API documentation for more information.
    inline BOOL CRichEdit::SetAutoURLDetect(BOOL enable /* = TRUE */) const
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(enable);
        return static_cast<BOOL>(SendMessage(EM_AUTOURLDETECT, wparam, 0));
    }

    // Sets the background color.
    // Refer to EM_SETBKGNDCOLOR in the Windows API documentation for more information.
    inline COLORREF CRichEdit::SetBackgroundColor(BOOL isSysColor, COLORREF color) const
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(isSysColor);
        LPARAM lparam = static_cast<LPARAM>(color);
        return static_cast<COLORREF>(SendMessage(EM_SETBKGNDCOLOR, wparam, lparam));
    }

    // Sets the current default character formatting attributes.
    // Refer to EM_SETCHARFORMAT in the Windows API documentation for more information.
    inline BOOL CRichEdit::SetDefaultCharFormat(CHARFORMAT& format) const
    {
        assert(IsWindow());
        LPARAM lparam = reinterpret_cast<LPARAM>(&format);
        return static_cast<BOOL>(SendMessage(EM_SETCHARFORMAT, 0, lparam));
    }

    // Sets the current default character formatting attributes.
    // Refer to EM_SETCHARFORMAT in the Windows API documentation for more information.
    inline BOOL CRichEdit::SetDefaultCharFormat(CHARFORMAT2& format) const
    {
        assert(IsWindow());
        LPARAM lparam = reinterpret_cast<LPARAM>(&format);
        return static_cast<BOOL>(SendMessage(EM_SETCHARFORMAT, 0, lparam));
    }

    // Sets the event mask. The event mask specifies which notification messages the control sends to its parent window.
    // Refer to EM_SETEVENTMASK in the Windows API documentation for more information.
    inline DWORD CRichEdit::SetEventMask(DWORD mask) const
    {
        assert(IsWindow());
        LPARAM lparam = static_cast<LPARAM>(mask);
        return static_cast<DWORD>(SendMessage(EM_SETEVENTMASK, 0, lparam));
    }

    // Sets or clears the modification flag. The modification flag indicates whether the text has been modified.
    // Refer to EM_SETMODIFY in the Windows API documentation for more information.
    inline void CRichEdit::SetModify(BOOL isModified /* = TRUE */) const
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(isModified);
        SendMessage(EM_SETMODIFY, wparam, 0);
    }

    // Sets the IRichEditOleCallback COM object.
    // Refer to EM_SETOLECALLBACK in the Windows API documentation for more information.
    inline BOOL CRichEdit::SetOLECallback(IRichEditOleCallback* pCallback) const
    {
        assert(IsWindow());
        LPARAM lparam = reinterpret_cast<LPARAM>(pCallback);
        return static_cast<BOOL>(SendMessage(EM_SETOLECALLBACK, 0, lparam));
    }

    // Sets the options.
    // Possible options values: ECOOP_SET, ECOOP_OR, ECOOP_AND, ECOOP_XOR
    // Possible flags:   ECO_AUTOWORDSELECTION, ECO_AUTOVSCROLL, ECO_AUTOHSCROLL, ECO_NOHIDESEL
    //                   ECO_READONLY, ECO_WANTRETURN, ECO_SELECTIONBAR, ECO_VERTICAL
    // Refer to EM_SETOPTIONS in the Windows API documentation for more information.
    inline void CRichEdit::SetOptions(WORD options, DWORD flags) const
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(options);
        LPARAM lparam = static_cast<LPARAM>(flags);
        SendMessage(EM_SETOPTIONS, wparam, lparam);
    }

    // Sets the paragraph formatting attributes in the current selection.
    // Refer to EM_SETPARAFORMAT in the Windows API documentation for more information.
    inline BOOL CRichEdit::SetParaFormat(PARAFORMAT& format) const
    {
        assert(IsWindow());
        LPARAM lparam = reinterpret_cast<LPARAM>(&format);
        return static_cast<BOOL>(SendMessage(EM_SETPARAFORMAT, 0, lparam));
    }

    // Sets the paragraph formatting attributes in the current selection.
    // Refer to EM_SETPARAFORMAT in the Windows API documentation for more information.
    inline BOOL CRichEdit::SetParaFormat(PARAFORMAT2& pf) const
    {
        assert(IsWindow());
        LPARAM lparam = reinterpret_cast<LPARAM>(&pf);
        return static_cast<BOOL>(SendMessage(EM_SETPARAFORMAT, 0, lparam));
    }

    // Sets the current punctuation characters for the rich edit control.
    // This is available only in Asian-language versions of the operating system.
    // Refer to EM_SETPUNCTUATION in the Windows API documentation for more information.
    inline BOOL CRichEdit::SetPunctuation(UINT type, const PUNCTUATION& puncInfo) const
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(type);
        LPARAM lparam = reinterpret_cast<LPARAM>(&puncInfo);
        return static_cast<BOOL>(SendMessage(EM_SETPUNCTUATION, wparam, lparam));
    }

    // Sets or removes the read-only style.
    // Refer to EM_SETREADONLY in the Windows API documentation for more information.
    inline BOOL CRichEdit::SetReadOnly(BOOL isReadOnly /* = TRUE*/) const
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(isReadOnly);
        return static_cast<BOOL>(SendMessage(EM_SETREADONLY, wparam, 0));
    }

    // Sets the formatting rectangle. The formatting rectangle is the limiting rectangle into
    // which the control draws the text.
    // Refer to EM_SETRECT in the Windows API documentation for more information.
    inline void CRichEdit::SetRect(const RECT& rc) const
    {
        assert(IsWindow());
        LPARAM lparam = reinterpret_cast<LPARAM>(&rc);
        SendMessage(EM_SETRECT, 0, lparam);
    }

    // Selects a range of characters.
    // Refer to EM_EXSETSEL in the Windows API documentation for more information.
    inline void CRichEdit::SetSel(long startChar, long endChar) const
    {
        assert(IsWindow());

        CHARRANGE cr{};
        cr.cpMin = startChar;
        cr.cpMax = endChar;
        LPARAM lparam = reinterpret_cast<LPARAM>(&cr);
        SendMessage(EM_EXSETSEL, 0, lparam);
    }

    // Selects a range of characters.
    // Refer to EM_EXSETSEL in the Windows API documentation for more information.
    inline void CRichEdit::SetSel(CHARRANGE& range) const
    {
        assert(IsWindow());
        LPARAM lparam = reinterpret_cast<LPARAM>(&range);
        SendMessage(EM_EXSETSEL, 0, lparam);
    }

    // Sets the character formatting attributes in the current selection.
    // Refer to EM_SETCHARFORMAT in the Windows API documentation for more information.
    inline BOOL CRichEdit::SetSelectionCharFormat(CHARFORMAT& format) const
    {
        assert(IsWindow());
        LPARAM lparam = reinterpret_cast<LPARAM>(&format);
        return static_cast<BOOL>(SendMessage(EM_SETCHARFORMAT, SCF_SELECTION, lparam));
    }

    // Sets the character formatting attributes in the current selection.
    // Refer to EM_SETCHARFORMAT in the Windows API documentation for more information.
    inline BOOL CRichEdit::SetSelectionCharFormat(CHARFORMAT2& cf) const
    {
        assert(IsWindow());
        LPARAM lparam = reinterpret_cast<LPARAM>(&cf);
        return static_cast<BOOL>(SendMessage(EM_SETCHARFORMAT, SCF_SELECTION, lparam));
    }

    // Sets the target output device and line width used for "what you see is what you get" (WYSIWYG) formatting.
    // Refer to EM_SETTARGETDEVICE in the Windows API documentation for more information.
    inline BOOL CRichEdit::SetTargetDevice(HDC dc, long lineWidth) const
    {
        assert(IsWindow());
        WPARAM wparam = reinterpret_cast<WPARAM>(dc);
        LPARAM lparam = static_cast<LPARAM>(lineWidth);
        return static_cast<BOOL>(SendMessage(EM_SETTARGETDEVICE, wparam, lparam));
    }

    // Sets the text mode or undo level of the rich edit control. The message fails if the control contains text.
    // Refer to EM_SETTEXTMODE in the Windows API documentation for more information.
    inline BOOL CRichEdit::SetTextMode(UINT mode) const
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(mode);
        return static_cast<BOOL>(SendMessage(EM_SETTEXTMODE, wparam, 0));
    }

    // Sets the maximum number of actions that can stored in the undo queue.
    // This member function fails if the control contains text.
    // Refer to EM_SETUNDOLIMIT in the Windows API documentation for more information.
    inline UINT CRichEdit::SetUndoLimit(UINT limit) const
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(limit);
        return static_cast<UINT>(SendMessage(EM_SETUNDOLIMIT, wparam, 0));
    }

    // Sets the character formatting attributes in the current word.
    // Refer to EM_SETCHARFORMAT in the Windows API documentation for more information.
    inline BOOL CRichEdit::SetWordCharFormat(CHARFORMAT& format) const
    {
        assert(IsWindow());
        LPARAM lparam = reinterpret_cast<LPARAM>(&format);
        return static_cast<BOOL>(SendMessage(EM_SETCHARFORMAT, (SCF_SELECTION | SCF_WORD), lparam));
    }

    // Sets the character formatting attributes in the current word.
    // Refer to EM_SETCHARFORMAT in the Windows API documentation for more information.
    inline BOOL CRichEdit::SetWordCharFormat(CHARFORMAT2& cf) const
    {
        assert(IsWindow());
        LPARAM lparam = reinterpret_cast<LPARAM>(&cf);
        return static_cast<BOOL>(SendMessage(EM_SETCHARFORMAT, (SCF_SELECTION | SCF_WORD), lparam));
    }

    // Stops the control from collecting additional typing actions into the current undo action.
    // The control stores the next typing action, if any, into a new action in the undo queue.
    // Refer to EM_STOPGROUPTYPING in the Windows API documentation for more information.
    inline void CRichEdit::StopGroupTyping() const
    {
        assert(IsWindow());
        SendMessage(EM_STOPGROUPTYPING, 0, 0);
    }

    // Replaces text with text from the specified input stream.
    // Refer to EM_STREAMIN in the Windows API documentation for more information.
    inline long CRichEdit::StreamIn(int format, EDITSTREAM& es) const
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(format);
        LPARAM lparam = reinterpret_cast<LPARAM>(&es);
        return static_cast<long>(SendMessage(EM_STREAMIN, wparam, lparam));
    }

    // Stores text into an output stream.
    // Refer to EM_STREAMOUT in the Windows API documentation for more information.
    inline long CRichEdit::StreamOut(int format, EDITSTREAM& es) const
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(format);
        LPARAM lparam = reinterpret_cast<LPARAM>(&es);
        return static_cast<long>(SendMessage(EM_STREAMOUT, wparam, lparam));
    }

    // Reverses the last editing operation.
    // Refer to EM_UNDO in the Windows API documentation for more information.
    inline BOOL CRichEdit::Undo() const
    {
        assert(IsWindow());
        return static_cast<BOOL>(SendMessage(EM_UNDO, 0, 0));
    }

}

#endif   // _WIN32XX_RICHEDIT_H_
