// Win32++   Version 10.3.0
// Release Date: 4th September 2026
//
//      David Nash
//      email: dnash@bigpond.net.au
//      url: https://sourceforge.net/projects/win32-framework
//           https://github.com/DavidNash2024/Win32xx
//
//
// Copyright (c) 2005-2026  David Nash
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


#ifndef WIN32XX_STATUSBAR_H_
#define WIN32XX_STATUSBAR_H_

#include "wxx_wincore.h"

namespace Win32xx
{
    //////////////////////////////////////////////////////////////////
    // CStatusBar manages a status bar control. A status bar is a
    // horizontal window at the bottom of a parent window in which
    // an application can display various kinds of status information.
    class CStatusBar : public CWnd
    {
    public:
        CStatusBar() = default;
        virtual ~CStatusBar() override = default;

        // Overridables
        virtual BOOL OnEraseBkgnd(CDC& dc) override;
        virtual void PreCreate(CREATESTRUCT& cs) override;
        virtual void PreRegisterClass(WNDCLASS& wc) override;

        // Accessors and mutators
        int GetParts(int parts = 0, int paneWidths[] = nullptr) const;
        CRect GetPartRect(int part) const;
        CString GetPartText(int part) const;
        BOOL IsSimple() const;
        BOOL SetParts(int parts, const int paneWidths[]);
        BOOL SetPartText(int part, LPCTSTR text, UINT style = 0);
        BOOL SetPartWidth(int part, int width);
        HICON GetPartIcon(int part) const;
        BOOL SetPartIcon(int part, HICON icon);
        void SetSimple(BOOL isSimple = TRUE);

    private:
        CStatusBar(const CStatusBar&) = delete;
        CStatusBar& operator=(const CStatusBar&) = delete;
    };

}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

namespace Win32xx
{

    ////////////////////////////////////////
    // Definitions for the CStatusBar class.
    //

    // Retrieves a count of the parts in the status bar.
    // Refer to SB_GETPARTS in the Windows API documentation for more information.
    inline int CStatusBar::GetParts(int parts, int paneWidths[]) const
    {
        assert(IsWindow());
        assert(parts <= 256);

        WPARAM wparam = static_cast<WPARAM>(parts);
        LPARAM lparam = reinterpret_cast<LPARAM>(paneWidths);
        return static_cast<int>(SendMessage(SB_GETPARTS, wparam, lparam));
    }

    // Retrieves the icon for a part in the status bar.
    // Refer to SB_GETICON in the Windows API documentation for more information.
    inline HICON CStatusBar::GetPartIcon(int part) const
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(part);
        return reinterpret_cast<HICON>(SendMessage(SB_GETICON, wparam, 0));
    }

    // Retrieves the bounding rectangle of a part in the status bar.
    // Refer to SB_GETRECT in the Windows API documentation for more information.
    inline CRect CStatusBar::GetPartRect(int part) const
    {
        assert(IsWindow());

        CRect rc;
        WPARAM wparam = static_cast<WPARAM>(part);
        LPARAM lparam = reinterpret_cast<LPARAM>(&rc);
        if (SendMessage(SB_GETRECT, wparam, lparam))
            return rc;
        else
            return CRect();
    }

    // Retrieves the text from a part in the status bar.
    // Refer to SB_GETTEXT in the Windows API documentation for more information.
    inline CString CStatusBar::GetPartText(int part) const
    {
        assert(IsWindow());

        // Get size of Text array
        WPARAM wparam = static_cast<WPARAM>(part);
        int chars = LOWORD (SendMessage(SB_GETTEXTLENGTH, wparam, 0));
        CString str;

        LPARAM lparam = reinterpret_cast<LPARAM>(str.GetBuffer(chars + 1));
        SendMessage(SB_GETTEXT, wparam, lparam);
        str.ReleaseBuffer();
        return str;
    }

    // Checks the status bar control to determine if it is in simple mode.
    // Refer to SB_ISSIMPLE in the Windows API documentation for more information.
    inline BOOL CStatusBar::IsSimple() const
    {
        assert(IsWindow());
        return SendMessage(SB_ISSIMPLE, 0, 0) ? TRUE : FALSE;
    }

    // Called when the background needs erasing.
    inline BOOL CStatusBar::OnEraseBkgnd(CDC& dc)
    {
        // Permit the parent window to handle the drawing of the StatusBar's
        // background. Return TRUE to suppress default background drawing.
        return (GetParent().SendMessage(UWM_DRAWSBBKGND, reinterpret_cast<WPARAM>(&dc),
            reinterpret_cast<LPARAM>(this))) ? TRUE : FALSE;
    }

    // Called by Create to set the window creation parameters.
    inline void CStatusBar::PreCreate(CREATESTRUCT& cs)
    {
        cs.style |= WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | CCS_BOTTOM;

        // Add the gripper style if the parent window is resizable.
        HWND parent = cs.hwndParent;
        DWORD dwParentStyle = static_cast<DWORD>(::GetWindowLongPtr(parent, GWL_STYLE));
        if (dwParentStyle & WS_THICKFRAME)
        {
            cs.style |= SBARS_SIZEGRIP;
        }
    }

    // Called by Create to set the window class parameters.
    inline void CStatusBar::PreRegisterClass(WNDCLASS& wc)
    {
        wc.lpszClassName = STATUSCLASSNAME;
    }

    // Sets the number of parts in a status window and the coordinate of the
    // right edge of each part. If an element of iPaneWidths is -1, the right
    // edge of the corresponding part extends to the border of the window.
    // Refer to SB_SETPARTS in the Windows API documentation for more information.
    inline BOOL CStatusBar::SetParts(int parts, const int paneWidths[])
    {
        assert(IsWindow());
        assert(parts <= 256);

        WPARAM wparam = static_cast<WPARAM>(parts);
        LPARAM lparam = reinterpret_cast<LPARAM>(paneWidths);
        return SendMessage(SB_SETPARTS, wparam, lparam) ? TRUE : FALSE;
    }

    // Set the text in a status bar part.
    // The Style parameter can be a combinations of ...
    // 0                 The text is drawn with a border to appear lower than the
    //                   plane of the window.
    // SBT_NOBORDERS     The text is drawn without borders.
    // SBT_OWNERDRAW     The text is drawn by the parent window.
    // SBT_POPOUT        The text is drawn with a border to appear higher than
    //                   the plane of the window.
    // SBT_RTLREADING    The text will be displayed in the opposite direction
    //                   to the text in the parent window.
    // Refer to SB_SETTEXT in the Windows API documentation for more information.
    inline BOOL CStatusBar::SetPartText(int part, LPCTSTR text, UINT style)
    {
        assert(IsWindow());

        BOOL result = FALSE;
        int partCount = static_cast<int>(SendMessage(SB_GETPARTS, 0, 0));
        if (part >= 0 && part < partCount)
        {
            WPARAM wparam = MAKEWPARAM(part, style);
            LPARAM lparam = reinterpret_cast<LPARAM>(text);
            result = (SendMessage(SB_SETTEXT, wparam, lparam)) ? TRUE : FALSE;
        }

        return result;
    }

    // Sets the icon for a part in the status bar.
    // Refer to SB_SETICON in the Windows API documentation for more information.
    inline BOOL CStatusBar::SetPartIcon(int part, HICON icon)
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(part);
        LPARAM lparam = reinterpret_cast<LPARAM>(icon);
        return SendMessage(SB_SETICON, wparam, lparam) ? TRUE : FALSE;
    }

    // Changes the width of an existing pane, or creates a new pane with the
    // specified width. A width of -1 for the last part sets the width to the
    // border of the window.
    // Refer to SB_SETPARTS in the Windows API documentation for more information.
    inline BOOL CStatusBar::SetPartWidth(int part, int width)
    {
        assert(IsWindow());
        assert(part >= 0 && part <= 255);

        const int oldCount = GetParts();
        const int newCount = std::max(part + 1, oldCount);
        std::vector<int> partWidths(newCount, 0);

        if (oldCount > 0)
            GetParts(oldCount, partWidths.data());
        else
            for (int i = 0; i < newCount; ++i)
                partWidths[i] = 0;

        if (part == 0)
            partWidths[0] = width;
        else
            partWidths[part] = (width >= 0) ? (partWidths[part - 1] + width) : -1;

       return SetParts(newCount, partWidths.data());
    }

    // Specifies whether a status window displays simple text or displays all
    // window parts set by a previous SB_SETPARTS message.
    // Refer to SB_SIMPLE in the Windows API documentation for more information.
    inline void CStatusBar::SetSimple(BOOL isSimple /* = TRUE*/)
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(isSimple);
        SendMessage(SB_SIMPLE, wparam, 0);
    }

} // namespace Win32xx

#endif // WIN32XX_STATUSBAR_H_
