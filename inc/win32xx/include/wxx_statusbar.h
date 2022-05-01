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


#ifndef _WIN32XX_STATUSBAR_H_
#define _WIN32XX_STATUSBAR_H_

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
        CStatusBar();
        virtual ~CStatusBar() {}

        // Overridables
        virtual HWND Create(HWND hParent);
        virtual BOOL OnEraseBkgnd(CDC& dc);
        virtual void PreCreate(CREATESTRUCT& cs);

        // Accessors and mutators
        int GetParts() const;
        CRect GetPartRect(int part) const;
        CString GetPartText(int part) const;
        BOOL IsSimple() const;
        BOOL SetPartText(int part, LPCTSTR text, UINT style = 0) const;
        BOOL SetPartWidth(int part, int width) const;
        HICON GetPartIcon(int part) const;
        BOOL SetPartIcon(int part, HICON icon) const;

        // Operations
        BOOL CreateParts(int parts, const int paneWidths[]) const;
        void SetSimple(BOOL isSimple = TRUE) const;

    private:
        CStatusBar(const CStatusBar&);              // Disable copy construction
        CStatusBar& operator = (const CStatusBar&); // Disable assignment operator
    };

}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


namespace Win32xx
{

    //////////////////////////////////////
    // Definitions for the CStatusBar class
    //
    inline CStatusBar::CStatusBar()
    {
    }

    // Creates the window. This is the default method of window creation.
    inline HWND CStatusBar::Create(HWND parent)
    {
        // Acquire the CREATESTRUCT parameters.
        CREATESTRUCT cs;
        ZeroMemory(&cs, sizeof(cs));

        // Add the gripper style if the parent window is resizable.
        DWORD dwParentStyle = static_cast<DWORD>(::GetWindowLongPtr(parent, GWL_STYLE));
        if (dwParentStyle & WS_THICKFRAME)
        {
            cs.style |= SBARS_SIZEGRIP;
        }

        PreCreate(cs);

        // Create the status bar window.
        HWND wnd = CreateEx(cs.dwExStyle, STATUSCLASSNAME, 0, cs.style,
            cs.x, cs.y, cs.cx, cs.cy, parent, 0, cs.lpCreateParams);

        return wnd;
    }

    // Sets the number of parts in a status window and the coordinate of the right edge of each part.
    // If an element of iPaneWidths is -1, the right edge of the corresponding part extends
    // to the border of the window.
    // Refer to SB_SETPARTS in the Windows API documentation for more information.
    inline BOOL CStatusBar::CreateParts(int parts, const int paneWidths[]) const
    {
        assert(IsWindow());
        assert(parts <= 256);

        return (SendMessage(SB_SETPARTS, (WPARAM)parts, (LPARAM)paneWidths) != 0);
    }

    // Retrieves a count of the parts in the status bar.
    // Refer to SB_GETPARTS in the Windows API documentation for more information.
    inline int CStatusBar::GetParts() const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(SB_GETPARTS, 0, 0));
    }

    // Retrieves the icon for a part in the status bar.
    // Refer to SB_GETICON in the Windows API documentation for more information.
    inline HICON CStatusBar::GetPartIcon(int part) const
    {
        assert(IsWindow());
        return reinterpret_cast<HICON>(SendMessage(SB_GETICON, (WPARAM)part, 0));
    }

    // Retrieves the bounding rectangle of a part in the status bar.
    // Refer to SB_GETRECT in the Windows API documentation for more information.
    inline CRect CStatusBar::GetPartRect(int part) const
    {
        assert(IsWindow());

        CRect rc;
        SendMessage(SB_GETRECT, (WPARAM)part, (LPARAM)&rc);
        return rc;
    }

    // Retrieves the text from a part in the status bar.
    // Refer to SB_GETTEXT in the Windows API documentation for more information.
    inline CString CStatusBar::GetPartText(int part) const
    {
        assert(IsWindow());
        CString PaneText;

        // Get size of Text array
        int chars = LOWORD (SendMessage(SB_GETTEXTLENGTH, (WPARAM)part, 0));
        CString str;

        SendMessage(SB_GETTEXT, (WPARAM)part, (LPARAM)str.GetBuffer(chars));
        str.ReleaseBuffer();
        return str;
    }

    // Checks the status bar control to determine if it is in simple mode.
    // Refer to SB_ISSIMPLE in the Windows API documentation for more information.
    inline BOOL CStatusBar::IsSimple() const
    {
        assert(IsWindow());
        return (SendMessage(SB_ISSIMPLE, 0, 0) != 0);
    }

    // Called when the background needs erasing
    inline BOOL CStatusBar::OnEraseBkgnd(CDC& dc)
    {
        // Permit the parent window to handle the drawing of the StatusBar's background.
        // Return TRUE to suppress default background drawing.
        return (GetParent().SendMessage(UWM_DRAWSBBKGND, (WPARAM)&dc,
            (LPARAM)this) != 0);
    }

    // Called by CStatusBar::Create to set some window parameters
    inline void CStatusBar::PreCreate(CREATESTRUCT& cs)
    {
        // cs.style is preset to SBARS_SIZEGRIP if the parent has the WS_THICKFRAME style.
        cs.style |= WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | CCS_BOTTOM;
    }

    // Set the text in a status bar part.
    // The Style parameter can be a combinations of ...
    //0                 The text is drawn with a border to appear lower than the plane of the window.
    //SBT_NOBORDERS     The text is drawn without borders.
    //SBT_OWNERDRAW     The text is drawn by the parent window.
    //SBT_POPOUT        The text is drawn with a border to appear higher than the plane of the window.
    //SBT_RTLREADING    The text will be displayed in the opposite direction to the text in the parent window.
    // Refer to SB_SETTEXT in the Windows API documentation for more information.
    inline BOOL CStatusBar::SetPartText(int part, LPCTSTR text, UINT style) const
    {
        assert(IsWindow());

        BOOL result = FALSE;
        if (static_cast<int>(SendMessage(SB_GETPARTS, 0, 0) >= part))
            result = (SendMessage(SB_SETTEXT, (WPARAM)(part | style), (LPARAM)text) != 0);

        return result;
    }

    // Sets the icon for a part in the status bar.
    // Refer to SB_SETICON in the Windows API documentation for more information.
    inline BOOL CStatusBar::SetPartIcon(int part, HICON icon) const
    {
        assert(IsWindow());
        return (SendMessage(SB_SETICON, (WPARAM)part, (LPARAM)icon) != 0);
    }

    // Changes the width of an existing pane, or creates a new pane with the specified width.
    // A width of -1 for the last part sets the width to the border of the window.
    // Refer to SB_SETPARTS in the Windows API documentation for more information.
    inline BOOL CStatusBar::SetPartWidth(int part, int width) const
    {
        assert(IsWindow());
        assert(part >= 0 && part <= 255);

        // Fill the PartWidths vector with the current width of the StatusBar parts
        int partsCount = static_cast<int>(SendMessage(SB_GETPARTS, 0, 0));
        std::vector<int> partWidths(partsCount, 0);
        int* pPartWidthArray = &partWidths[0];
        SendMessage(SB_GETPARTS, (WPARAM)partsCount, (LPARAM)pPartWidthArray);

        // Fill the NewPartWidths vector with the new width of the StatusBar parts
        int newPartsCount = MAX(part+1, partsCount);
        std::vector<int> newPartWidths(newPartsCount, 0);
        newPartWidths = partWidths;
        int* pNewPartWidthArray = &newPartWidths[0];

        if (part == 0)
            pNewPartWidthArray[part] = width;
        else
        {
            if (width >= 0)
                pNewPartWidthArray[part] = pNewPartWidthArray[part -1] + width;
            else
                pNewPartWidthArray[part] = -1;
        }

        // Set the StatusBar parts with our new parts count and part widths
        BOOL result = (SendMessage(SB_SETPARTS, (WPARAM)newPartsCount, (LPARAM)pNewPartWidthArray) != 0);

        return result;
    }

    // Specifies whether a status window displays simple text or displays all window parts
    // set by a previous SB_SETPARTS message.
    // Refer to SB_SIMPLE in the Windows API documentation for more information.
    inline void CStatusBar::SetSimple(BOOL isSimple /* = TRUE*/) const
    {
        assert(IsWindow());
        SendMessage(SB_SIMPLE, isSimple, 0);
    }

} // namespace Win32xx

#endif // #ifndef _WIN32XX_STATUSBAR_H_
