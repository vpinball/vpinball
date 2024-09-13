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


////////////////////////////////////////////////////////
// wxx_menumetrics.h
//  Declaration of the following classes and structs:
//  Margins, MenuItemData, and CMenuMetrics.


#ifndef _WIN32XX_MENUMETRICS_H_
#define _WIN32XX_MENUMETRICS_H_

#include "wxx_wincore.h"
#include <Uxtheme.h>
#include <vsstyle.h>
#include <vssym32.h>


namespace Win32xx
{
    ////////////////////////////////////////////////////////////////
    // The Margins struct is returned by the GetThemeMargins Windows
    // API function. Margins are the dimension of the thin border
    // around the various theme parts.
    struct Margins
    {
        Margins(int cxLeft, int cxRight, int cyTop, int cyBottom)
            : cxLeftWidth(cxLeft), cxRightWidth(cxRight),
              cyTopHeight(cyTop), cyBottomHeight(cyBottom)
        {}

        Margins() : cxLeftWidth(0), cxRightWidth(0), cyTopHeight(0), cyBottomHeight(0)
        {}

        int Width() const       { return cxLeftWidth + cxRightWidth; }
        int Height() const      { return cyTopHeight + cyBottomHeight; }
        void SetMargins(int cxLeft, int cxRight, int cyTop, int cyBottom)
        {
            cxLeftWidth    = cxLeft;
            cxRightWidth   = cxRight;
            cyTopHeight    = cyTop;
            cyBottomHeight = cyBottom;
        }

        int cxLeftWidth;      // width of left border that retains its size
        int cxRightWidth;     // width of right border that retains its size
        int cyTopHeight;      // height of top border that retains its size
        int cyBottomHeight;   // height of bottom border that retains its size
    };


    /////////////////////////////////////////////////////////////
    // MenuItemData defines the data for each dropdown menu item.
    struct MenuItemData
    {
        MenuItemData() : menu(nullptr), pos(0)
        {
            mii = {};
        }

        HMENU menu;
        MENUITEMINFO mii;
        UINT  pos;
        CString itemText;
    };


    ///////////////////////////////////////////////////////////////////////////
    // CMenuMetrics is used by CFrameT to retrieve the size of the components
    // used to perform owner-drawing of menu items. For Windows Vista and above
    // it loads uxtheme.dll and uses its visual style functions for the
    // calculations.
    class CMenuMetrics
    {
    public:
        CMenuMetrics();
        ~CMenuMetrics();

        CRect GetCheckBackgroundRect(const CRect& item) const;
        CRect GetCheckRect(const CRect& item) const;
        CRect GetGutterRect(const CRect& item) const;
        CSize GetItemSize(MenuItemData* pmd, CClientDC& dc) const;
        int   GetMenuIconHeight() const;
        CRect GetSelectionRect(const CRect& item) const;
        CRect GetSeperatorRect(const CRect& item) const;
        CRect GetTextRect(const CRect& item) const;
        CSize GetTextSize(MenuItemData* pmd, CClientDC& dc) const;
        void  Initialize();
        BOOL  IsVistaMenu() const;
        void  SetMetrics(HWND frame);
        int   ToItemStateId(UINT itemState) const;
        int   ToCheckBackgroundStateId(int stateID) const;
        int   ToCheckStateId(UINT type, int stateID) const;

        // Wrappers for Windows API functions.
        HRESULT CloseThemeData() const;
        HRESULT DrawThemeBackground(HDC dc, int partID, int stateID, const RECT* pRect, const RECT* pClipRect) const;
        HRESULT DrawThemeText(HDC dc, int partID, int stateID, LPCWSTR text, int charCount, DWORD textFlags,
                              DWORD textFlags2, LPCRECT pRect) const;
        HRESULT GetThemePartSize(HDC dc, int partID, int stateID, LPCRECT prc, THEMESIZE eSize, SIZE* psz) const;
        HRESULT GetThemeInt(int partID, int stateID, int propID, int* pVal) const;
        HRESULT GetThemeMargins(HDC dc, int partID, int stateID, int propID, LPRECT prc, Margins* pMargins) const;
        HRESULT GetThemeTextExtent(HDC dc, int partID, int stateID, LPCWSTR text, int charCount, DWORD textFlags,
                                   LPCRECT pBoundingRect, LPRECT pExtentRect) const;
        BOOL    IsThemeBackgroundPartiallyTransparent(int partID, int stateID) const;
        HANDLE  OpenThemeData(HWND wnd, LPCWSTR classList) const;

    private:
        HANDLE  m_theme;                // Theme handle returned by OpenThemeData.

        Margins m_marCheck;             // The check margins value returned by the GetThemeMargins function.
        Margins m_marCheckBackground;   // The check background margins value returned by the GetThemeMargins function.
        Margins m_marItem;              // The item margins value returned by the GetThemeMargins function.
        Margins m_marText;              // The text margins value returned by the GetThemeMargins function.

        CSize   m_sizeCheck;            // The size of the image used for menu check boxes and radio boxes.
        CSize   m_sizeSeparator;        // The size of the separator menu item.

        using CLOSETHEMEDATA = HRESULT(WINAPI*)(HANDLE);
        using DRAWTHEMEBACKGROUND = HRESULT (WINAPI*)(HANDLE, HDC, int, int, const RECT*, const RECT*);
        using DRAWTHEMETEXT = HRESULT (WINAPI*)(HANDLE, HDC, int, int, LPCWSTR, int, DWORD, DWORD, LPCRECT);
        using GETTHEMEPARTSIZE = HRESULT (WINAPI*)(HANDLE, HDC, int, int, LPCRECT, THEMESIZE, SIZE*);
        using GETTHEMEINT = HRESULT (WINAPI*)(HANDLE, int, int, int, int*);
        using GETTHEMEMARGINS = HRESULT (WINAPI*)(HANDLE, HDC, int, int, int, LPRECT, Margins*);
        using GETTHEMETEXTEXTENT = HRESULT (WINAPI*)(HANDLE, HDC, int, int, LPCWSTR, int, DWORD, LPCRECT, LPCRECT);
        using ISTHEMEBGPARTTRANSPARENT = BOOL (WINAPI*)(HANDLE, int, int);
        using OPENTHEMEDATA = HANDLE (WINAPI*)(HWND, LPCWSTR);

        // Pointers to functions defined in uxTheme.dll
        CLOSETHEMEDATA           m_pfnCloseThemeData;
        DRAWTHEMEBACKGROUND      m_pfnDrawThemeBackground;
        DRAWTHEMETEXT            m_pfnDrawThemeText;
        GETTHEMEPARTSIZE         m_pfnGetThemePartSize;
        GETTHEMEINT              m_pfnGetThemeInt;
        GETTHEMEMARGINS          m_pfnGetThemeMargins;
        GETTHEMETEXTEXTENT       m_pfnGetThemeTextExtent;
        ISTHEMEBGPARTTRANSPARENT m_pfnIsThemeBGPartTransparent;
        OPENTHEMEDATA            m_pfnOpenThemeData;
    };

}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


namespace Win32xx
{

    //////////////////////////////////////////
    // Definitions for the CMenuMetrics class.
    //
    inline CMenuMetrics::CMenuMetrics() : m_theme(nullptr), m_pfnCloseThemeData(nullptr), m_pfnDrawThemeBackground(nullptr),
                                            m_pfnDrawThemeText(nullptr), m_pfnGetThemePartSize(nullptr), m_pfnGetThemeInt(nullptr),
                                            m_pfnGetThemeMargins(nullptr), m_pfnGetThemeTextExtent(nullptr),
                                            m_pfnIsThemeBGPartTransparent(nullptr), m_pfnOpenThemeData(nullptr)
    {
        Initialize();
    }

    inline CMenuMetrics::~CMenuMetrics()
    {
        if (m_theme != nullptr)
            CloseThemeData();
    }

    // Closes the theme data handle.
    inline HRESULT CMenuMetrics::CloseThemeData() const
    {
        if (m_pfnCloseThemeData)
            return m_pfnCloseThemeData(m_theme);

        return E_NOTIMPL;
    }

    // Draws the border and fill defined by the visual style for the specified control part.
    inline HRESULT CMenuMetrics::DrawThemeBackground(HDC dc, int partID, int stateID, const RECT *pRect, const RECT *pClipRect) const
    {
        assert(m_theme);
        if (m_pfnDrawThemeBackground)
            return m_pfnDrawThemeBackground(m_theme, dc, partID, stateID, pRect, pClipRect);

        return E_NOTIMPL;
    }

    // Draws text using the color and font defined by the visual style.
    inline HRESULT CMenuMetrics::DrawThemeText(HDC dc, int partID, int stateID, LPCWSTR text, int charCount,
                                               DWORD textFlags, DWORD textFlags2, LPCRECT pRect) const
    {
        assert(m_theme);
        if (m_pfnDrawThemeText)
            return m_pfnDrawThemeText(m_theme, dc, partID, stateID, text, charCount, textFlags, textFlags2, pRect);

        return E_NOTIMPL;
    }

    inline CRect CMenuMetrics::GetCheckBackgroundRect(const CRect& item) const
    {
        int cx = m_sizeCheck.cx + m_marCheck.Width();
        int cy = m_sizeCheck.cy + m_marCheck.Height();

        int x = item.left + m_marCheckBackground.cxLeftWidth + m_marItem.cxLeftWidth;
        int y = item.top + (item.Height() - cy) / 2;

        return CRect(x, y, x + cx, y + cy);
    }

    inline CRect CMenuMetrics::GetGutterRect(const CRect& item) const
    {
        int x = item.left;
        int y = item.top;
        int cx = m_marItem.cxLeftWidth + m_marCheckBackground.Width() + m_marCheck.Width() + m_sizeCheck.cx;
        int cy = item.Height();

        return CRect(x, y, x + cx, y + cy);
    }

    inline CRect CMenuMetrics::GetCheckRect(const CRect& item) const
    {
        int x = item.left + m_marCheckBackground.cxLeftWidth + m_marCheck.cxLeftWidth + m_marItem.cxLeftWidth;
        int y = item.top + (item.Height() - m_sizeCheck.cy) / 2;

        return CRect(x, y, x + m_sizeCheck.cx, y + m_sizeCheck.cy);
    }

    // Retrieve the size of the menu item.
    inline CSize CMenuMetrics::GetItemSize(MenuItemData* pmd, CClientDC& dc) const
    {
        CSize size;

        // Add icon/check width.
        CRect rc;
        size.cx += GetGutterRect(rc).Width();

        if (pmd->mii.fType & MFT_SEPARATOR)
        {
            // separator height
            size.cy = m_sizeSeparator.cy + m_marItem.Height();
        }
        else
        {
            // Add check background horizontal padding.
            size.cx += m_marCheckBackground.Width();

            // Add selection margin padding.
            size.cx += m_marItem.Width();

            size.cx += GetMenuIconHeight() / 4;

            // Account for text width and checkmark height.
            CSize sizeText = GetTextSize(pmd, dc);
            size.cx += sizeText.cx;
            size.cy = std::max(sizeText.cy, m_sizeCheck.cy) + m_marCheckBackground.Height() +
                m_marCheck.Height();
        }

        return (size);
    }

    inline int CMenuMetrics::GetMenuIconHeight() const
    {
        int value = m_sizeCheck.cy + m_marCheck.Height();
        return value;
    }

    inline CRect CMenuMetrics::GetSelectionRect(const CRect& item) const
    {
        int x = item.left + m_marItem.cxLeftWidth;
        int y = item.top;

        return CRect(x, y, item.right - m_marItem.cxRightWidth, y + item.Height());
    }

    inline CRect CMenuMetrics::GetSeperatorRect(const CRect& item) const
    {
        int left = GetGutterRect(item).right;
        int top = item.top;
        int right = item.right - m_marItem.cxRightWidth;
        int bottom = item.top + m_sizeSeparator.cy;

        return CRect(left, top, right, bottom);
    }

    inline CSize CMenuMetrics::GetTextSize(MenuItemData* pmd, CClientDC& dc) const
    {
        // Calculate the size of the text.
        CSize sizeText;
        CString itemText = pmd->itemText;
        if (IsVistaMenu())
        {
            CRect rcText;
            GetThemeTextExtent(dc, MENU_POPUPITEM, 0, TtoW(itemText), itemText.GetLength(),
                DT_EXPANDTABS, nullptr, &rcText);

            sizeText.SetSize(rcText.right, rcText.bottom);
        }
        else
        {
            // Calculate the size of the text.
            sizeText = dc.GetTextExtentPoint32(itemText, itemText.GetLength());
        }

        sizeText.cx += m_marText.Width();
        sizeText.cy += m_marText.Height();
        return sizeText;
    }

    inline CRect CMenuMetrics::GetTextRect(const CRect& item) const
    {
        int left = GetGutterRect(item).Width() + m_marText.cxLeftWidth + GetMenuIconHeight() / 4;
        int top = item.top + m_marText.cyTopHeight;
        int right = item.right - m_marItem.cxRightWidth - m_marText.cxRightWidth;
        int bottom = item.bottom - m_marText.cyBottomHeight;

        return CRect(left, top, right, bottom);
    }

    // Calculates the original size of the part defined by a visual style.
    inline HRESULT CMenuMetrics::GetThemePartSize(HDC dc, int partID, int stateID, LPCRECT prc,
                                                  THEMESIZE eSize, SIZE* psz) const
    {
        assert(m_theme);
        if (m_pfnGetThemePartSize)
            return m_pfnGetThemePartSize(m_theme, dc, partID, stateID, prc, eSize, psz);

        return E_NOTIMPL;
    }

    // Retrieves the value of an int property.
    inline HRESULT CMenuMetrics::GetThemeInt(int partID, int stateID, int propID, int* pVal) const
    {
        assert(m_theme);
        if (m_pfnGetThemeInt)
            return m_pfnGetThemeInt(m_theme, partID, stateID, propID, pVal);

        return E_NOTIMPL;
    }

    // Retrieves the value of a MARGINS property.
    inline HRESULT CMenuMetrics::GetThemeMargins(HDC dc, int partID, int stateID, int propID,
                                                 LPRECT prc, Margins* pMargins) const
    {
        assert(m_theme);
        if (m_pfnGetThemeMargins)
            return m_pfnGetThemeMargins(m_theme, dc, partID, stateID, propID, prc, pMargins);

        return E_NOTIMPL;
    }

    // Calculates the size and location of the specified text when rendered in the visual style font.
    inline HRESULT CMenuMetrics::GetThemeTextExtent(HDC dc, int partID, int stateID, LPCWSTR text,
                                                    int charCount, DWORD textFlags, LPCRECT pBoundingRect,
                                                    LPRECT pExtentRect) const
    {
        assert(m_theme);
        if (m_pfnGetThemeTextExtent)
            return m_pfnGetThemeTextExtent(m_theme, dc, partID, stateID, text, charCount, textFlags, pBoundingRect, pExtentRect);

        return E_NOTIMPL;
    }

    // Initializes the CMenuMetrics member variables.
    inline void CMenuMetrics::Initialize()
    {
        HMODULE uxTheme = ::GetModuleHandle(_T("uxtheme.dll"));

        if (uxTheme != nullptr)
        {
            m_pfnCloseThemeData = reinterpret_cast<CLOSETHEMEDATA>(
                reinterpret_cast<void*>(::GetProcAddress(uxTheme, "CloseThemeData")));
            m_pfnDrawThemeBackground = reinterpret_cast<DRAWTHEMEBACKGROUND>(
                reinterpret_cast<void*>(::GetProcAddress(uxTheme, "DrawThemeBackground")));
            m_pfnDrawThemeText = reinterpret_cast<DRAWTHEMETEXT>(
                reinterpret_cast<void*>(::GetProcAddress(uxTheme, "DrawThemeText")));
            m_pfnGetThemePartSize = reinterpret_cast<GETTHEMEPARTSIZE>(
                reinterpret_cast<void*>(::GetProcAddress(uxTheme, "GetThemePartSize")));
            m_pfnGetThemeInt = reinterpret_cast<GETTHEMEINT>(
                reinterpret_cast<void*>(::GetProcAddress(uxTheme, "GetThemeInt")));
            m_pfnGetThemeMargins = reinterpret_cast<GETTHEMEMARGINS>(
                reinterpret_cast<void*>(::GetProcAddress(uxTheme, "GetThemeMargins")));
            m_pfnGetThemeTextExtent = reinterpret_cast<GETTHEMETEXTEXTENT>(
                reinterpret_cast<void*>(::GetProcAddress(uxTheme, "GetThemeTextExtent")));
            m_pfnIsThemeBGPartTransparent = reinterpret_cast<ISTHEMEBGPARTTRANSPARENT>(
                reinterpret_cast<void*>(::GetProcAddress(uxTheme, "IsThemeBackgroundPartiallyTransparent")));
            m_pfnOpenThemeData = reinterpret_cast<OPENTHEMEDATA>(
                reinterpret_cast<void*>(::GetProcAddress(uxTheme, "OpenThemeData")));
        }
    }

    inline void CMenuMetrics::SetMetrics(HWND frame)
    {
        if (m_theme != nullptr)
        {
            CloseThemeData();
            m_theme = nullptr;
        }

        m_theme = OpenThemeData(frame, VSCLASS_MENU);

        if (m_theme != nullptr)
        {
            int borderSize = 0;    // Border space between item text and accelerator.
            int bgBorderSize = 0;  // Border space between item text and gutter.
            GetThemePartSize(nullptr, MENU_POPUPCHECK, 0, nullptr, TS_TRUE, &m_sizeCheck);
            GetThemePartSize(nullptr, MENU_POPUPSEPARATOR, 0, nullptr, TS_TRUE, &m_sizeSeparator);
            GetThemeInt(MENU_POPUPITEM, 0, TMT_BORDERSIZE, &borderSize);
            GetThemeInt(MENU_POPUPBACKGROUND, 0, TMT_BORDERSIZE, &bgBorderSize);
            GetThemeMargins(nullptr, MENU_POPUPCHECK, 0, TMT_CONTENTMARGINS, nullptr, &m_marCheck);
            GetThemeMargins(nullptr, MENU_POPUPCHECKBACKGROUND, 0, TMT_CONTENTMARGINS, nullptr, &m_marCheckBackground);
            GetThemeMargins(nullptr, MENU_POPUPITEM, 0, TMT_CONTENTMARGINS, nullptr, &m_marItem);

            // Popup text margins
            m_marText = m_marItem;
            m_marText.cxRightWidth = borderSize;
            m_marText.cxLeftWidth = bgBorderSize;
        }
        else
        {
            m_sizeCheck.SetSize(::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON));
            m_sizeSeparator.SetSize(1, 7);
            m_marCheck.SetMargins(4, 4, 2, 2);
            m_marCheckBackground.SetMargins(0, 0, 0, 0);
            m_marItem.SetMargins(0, 0, 0, 0);
            m_marText.SetMargins(8, 16, 0, 0);
        }
    }

    // Retrieves whether the background specified by the visual style has transparent pieces or alpha-blended pieces.
    inline BOOL CMenuMetrics::IsThemeBackgroundPartiallyTransparent(int partID, int stateID) const
    {
        assert(m_theme);
        if (m_pfnIsThemeBGPartTransparent)
            return m_pfnIsThemeBGPartTransparent(m_theme, partID, stateID);

        return FALSE;
    }

    // Opens the theme data for a window and its associated class.
    inline HANDLE CMenuMetrics::OpenThemeData(HWND wnd, LPCWSTR classList) const
    {
        if (m_pfnOpenThemeData)
            return m_pfnOpenThemeData(wnd, classList);

        return 0;
    }

    inline BOOL CMenuMetrics::IsVistaMenu() const
    {
        return (m_theme != nullptr);
    }

    // Convert from item state to MENU_POPUPITEM state.
    inline int CMenuMetrics::ToItemStateId(UINT itemState) const
    {
        const bool      isDisabled   = ((itemState & (ODS_INACTIVE | ODS_DISABLED)) != 0);
        const bool      isHot        = ((itemState & (ODS_HOTLIGHT | ODS_SELECTED)) != 0);
        POPUPITEMSTATES state;

        if (isDisabled)
            state = (isHot ? MPI_DISABLEDHOT : MPI_DISABLED);
        else if (isHot)
            state = MPI_HOT;
        else
            state= MPI_NORMAL;

        return state;
    }

    // Convert to MENU_POPUPCHECKBACKGROUND.
    inline int CMenuMetrics::ToCheckBackgroundStateId(int stateID) const
    {
        POPUPCHECKBACKGROUNDSTATES stateIdCheckBackground;

        // Determine the check background state.
        if (stateID == MPI_DISABLED || stateID == MPI_DISABLEDHOT)
            stateIdCheckBackground = MCB_DISABLED;
        else
            stateIdCheckBackground = MCB_NORMAL;

        return stateIdCheckBackground;
    }

    // Convert to MENU_POPUPCHECK state.
    inline int CMenuMetrics::ToCheckStateId(UINT type, int stateID) const
    {
        POPUPCHECKSTATES stateIdCheck;

        if (type & MFT_RADIOCHECK)
        {
            if (stateID == MPI_DISABLED || stateID == MPI_DISABLEDHOT)
                stateIdCheck = MC_BULLETDISABLED;
            else
                stateIdCheck = MC_BULLETNORMAL;
        }
        else
        {
            if (stateID == MPI_DISABLED || stateID == MPI_DISABLEDHOT)
                stateIdCheck = MC_CHECKMARKDISABLED;
            else
                stateIdCheck = MC_CHECKMARKNORMAL;
        }

        return stateIdCheck;
    }

}

#endif // _WIN32XX_MENUMETRICS_H_
