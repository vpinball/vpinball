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


#ifndef _WIN32XX_STATUSBAR_H_
#define _WIN32XX_STATUSBAR_H_

#include "wxx_wincore.h"

namespace Win32xx
{

    // The CStatusBar class provides the functionality of a status bar control.
    class CStatusBar : public CWnd
    {
    public:
        CStatusBar();
        virtual ~CStatusBar() {}

        // Overridables
        virtual HWND Create(HWND hWndParent);
        virtual BOOL OnEraseBkgnd(CDC& dc);
        virtual void PreCreate(CREATESTRUCT& cs);

        // Attributes
        int GetParts() const;
        CRect GetPartRect(int iPart) const;
        CString GetPartText(int iPart) const;
        BOOL IsSimple() const;
        BOOL SetPartText(int iPart, LPCTSTR szText, UINT Style = 0) const;
        BOOL SetPartWidth(int iPart, int iWidth) const;
        HICON GetPartIcon(int iPart) const;
        BOOL SetPartIcon(int iPart, HICON hIcon) const;

        // Operations
        CStatusBar(const CStatusBar&);              // Disable copy construction
        CStatusBar& operator = (const CStatusBar&); // Disable assignment operator

        BOOL CreateParts(int iParts, const int iPaneWidths[]) const;
        void SetSimple(BOOL IsSimple = TRUE) const;
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
    inline HWND CStatusBar::Create(HWND hWndParent)
    {
        // Acquire the CREATESTRUCT parameters
        CREATESTRUCT cs;
        ZeroMemory(&cs, sizeof(cs));

        // Add the gripper style if the parent window is resizable
        DWORD dwParentStyle = static_cast<DWORD>(::GetWindowLongPtr(hWndParent, GWL_STYLE));
        if (dwParentStyle & WS_THICKFRAME)
        {
            cs.style |= SBARS_SIZEGRIP;
        }

        PreCreate(cs);

        // Create the status bar window
        HWND hWnd = CreateEx(cs.dwExStyle, STATUSCLASSNAME, 0, cs.style,
            cs.x, cs.y, cs.cx, cs.cy, hWndParent, 0, cs.lpCreateParams);

        return hWnd;
    }


    // Sets the number of parts in a status window and the coordinate of the right edge of each part.
    // If an element of iPaneWidths is -1, the right edge of the corresponding part extends
    // to the border of the window.
    inline BOOL CStatusBar::CreateParts(int iParts, const int iPaneWidths[]) const
    {
        assert(IsWindow());
        assert(iParts <= 256);

        return static_cast<BOOL>(SendMessage(SB_SETPARTS, iParts, reinterpret_cast<LPARAM>(iPaneWidths)));
    }


    // Retrieves a count of the parts in the status bar.
    inline int CStatusBar::GetParts() const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(SB_GETPARTS, 0L, 0L));
    }


    // Retrieves the icon for a part in the status bar.
    inline HICON CStatusBar::GetPartIcon(int iPart) const
    {
        assert(IsWindow());
        return reinterpret_cast<HICON>(SendMessage(SB_GETICON, iPart, 0L));
    }


    // Retrieves the bounding rectangle of a part in the status bar.
    inline CRect CStatusBar::GetPartRect(int iPart) const
    {
        assert(IsWindow());

        CRect rc;
        SendMessage(SB_GETRECT, iPart, reinterpret_cast<LPARAM>(&rc));
        return rc;
    }


    // Retrieves the text from a part in the status bar.
    inline CString CStatusBar::GetPartText(int iPart) const
    {
        assert(IsWindow());
        CString PaneText;

        // Get size of Text array
        int iChars = LOWORD (SendMessage(SB_GETTEXTLENGTH, iPart, 0L));
        CString str;

        SendMessage(SB_GETTEXT, iPart, reinterpret_cast<LPARAM>(str.GetBuffer(iChars)));
        str.ReleaseBuffer();
        return str;
    }


    // Checks the status bar control to determine if it is in simple mode.
    inline BOOL CStatusBar::IsSimple() const
    {
        assert(IsWindow());
        return static_cast<BOOL>(SendMessage(SB_ISSIMPLE, 0L, 0L));
    }


    // Called when the background needs erasing
    inline BOOL CStatusBar::OnEraseBkgnd(CDC& dc)
    {
        // Permit the parent window to handle the drawing of the StatusBar's background.
        // Return TRUE to suppress default background drawing.
        return (GetParent().SendMessage(UWM_DRAWSBBKGND, reinterpret_cast<WPARAM>(&dc),
                                        reinterpret_cast<LPARAM>(this)) != 0);
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
    inline BOOL CStatusBar::SetPartText(int iPart, LPCTSTR szText, UINT Style) const
    {
        assert(IsWindow());

        BOOL Succeeded = FALSE;
        if (static_cast<int>(SendMessage(SB_GETPARTS, 0L, 0L) >= iPart))
            Succeeded = static_cast<BOOL>(SendMessage(SB_SETTEXT, (iPart | Style), 
                            reinterpret_cast<LPARAM>(szText)));

        return Succeeded;
    }


    // Sets the icon for a part in the status bar.
    inline BOOL CStatusBar::SetPartIcon(int iPart, HICON hIcon) const
    {
        assert(IsWindow());
        return static_cast<BOOL>(SendMessage(SB_SETICON, iPart, reinterpret_cast<LPARAM>(hIcon)));
    }


    // Changes the width of an existing pane, or creates a new pane with the specified width.
    // A width of -1 for the last part sets the width to the border of the window.
    inline BOOL CStatusBar::SetPartWidth(int iPart, int iWidth) const
    {
        assert(IsWindow());
        assert(iPart >= 0 && iPart <= 255);

        // Fill the PartWidths vector with the current width of the StatusBar parts
        int PartsCount = static_cast<int>(SendMessage(SB_GETPARTS, 0L, 0L));
        std::vector<int> PartWidths(PartsCount, 0);
        int* pPartWidthArray = &PartWidths[0];
        SendMessage(SB_GETPARTS, PartsCount, reinterpret_cast<LPARAM>(pPartWidthArray));

        // Fill the NewPartWidths vector with the new width of the StatusBar parts
        int NewPartsCount = MAX(iPart+1, PartsCount);
        std::vector<int> NewPartWidths(NewPartsCount, 0);
        NewPartWidths = PartWidths;
        int* pNewPartWidthArray = &NewPartWidths[0];

        if (iPart == 0)
            pNewPartWidthArray[iPart] = iWidth;
        else
        {
            if (iWidth >= 0)
                pNewPartWidthArray[iPart] = pNewPartWidthArray[iPart -1] + iWidth;
            else
                pNewPartWidthArray[iPart] = -1;
        }

        // Set the StatusBar parts with our new parts count and part widths
        BOOL Succeeded = static_cast<BOOL>(SendMessage(SB_SETPARTS, NewPartsCount, 
                                            reinterpret_cast<LPARAM>(pNewPartWidthArray)));

        return Succeeded;
    }


    // Specifies whether a status window displays simple text or displays all window parts
    // set by a previous SB_SETPARTS message.
    inline void CStatusBar::SetSimple(BOOL IsSimple /* = TRUE*/) const
    {
        assert(IsWindow());
        SendMessage(SB_SIMPLE, IsSimple, 0L);
    }

} // namespace Win32xx

#endif // #ifndef _WIN32XX_STATUSBAR_H_
