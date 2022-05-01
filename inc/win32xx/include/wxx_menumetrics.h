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


////////////////////////////////////////////////////////
// wxx_menumetrics.h
//  Declaration of the following classes and structs:
//  Margins, MenuItemData, and CMenuMetrics.


#ifndef _WIN32XX_MENUMETRICS_H_
#define _WIN32XX_MENUMETRICS_H_

#include "wxx_wincore.h"


#ifndef TMT_BORDERSIZE
  #define TMT_BORDERSIZE    2403
#endif
#ifndef TMT_CONTENTMARGINS
  #define TMT_CONTENTMARGINS    3602
#endif
#ifndef VSCLASS_MENU
  #define VSCLASS_MENU  L"MENU"
#endif
#ifndef ODS_NOACCEL
  #define ODS_NOACCEL 0x0100
#endif
#ifndef ODS_HOTLIGHT
  #define ODS_HOTLIGHT 0x0040
#endif
#ifndef ODS_INACTIVE
  #define ODS_INACTIVE 0x0080
#endif
#ifndef DT_HIDEPREFIX
  #define DT_HIDEPREFIX 0x00100000
#endif

#if defined (_MSC_VER) && (_MSC_VER >= 1920)   // >= VS2019
  #pragma warning ( push )
  #pragma warning ( disable : 26812 )            // enum type is unscoped.
#endif // (_MSC_VER) && (_MSC_VER >= 1920)


namespace Win32xx
{
    /////////////////////////////////////////////////////////
    // This struct provides the margins used by CMenuMetrics.
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
        MenuItemData() : menu(0), pos(0)
        {
            ZeroMemory(&mii, GetSizeofMenuItemInfo());
            itemText.assign(WXX_MAX_STRING_SIZE, _T('\0'));
        }
        LPTSTR GetItemText() {return &itemText[0];}

        HMENU menu;
        MENUITEMINFO mii;
        UINT  pos;
        std::vector<TCHAR> itemText;
    };


    ///////////////////////////////////////////////////////////////////////////
    // CMenuMetrics is used by CFrameT to retrieve the size of the components
    // used to perform owner-drawing of menu items. For Windows Vista and above
    // it loads uxtheme.dll and uses its visual style functions for the
    // calculations.
    class CMenuMetrics
    {
        // A local copy of enums from uxtheme.h. They're declared within the
        // CMenuMetrics class to avoid name clashes when uxtheme.h is included.
        enum POPUPCHECKBACKGROUNDSTATES
        {
            MCB_DISABLED = 1,
            MCB_NORMAL = 2,
            MCB_BITMAP = 3
        };

        enum POPUPCHECKSTATES
        {
            MC_CHECKMARKNORMAL = 1,
            MC_CHECKMARKDISABLED = 2,
            MC_BULLETNORMAL = 3,
            MC_BULLETDISABLED = 4
        };

        enum POPUPITEMSTATES
        {
            MPI_NORMAL = 1,
            MPI_HOT = 2,
            MPI_DISABLED = 3,
            MPI_DISABLEDHOT = 4
        };

        enum POPUPSUBMENUSTATES
        {
            MSM_NORMAL = 1,
            MSM_DISABLED = 2
        };

        enum THEMESIZE
        {
            TS_MIN,             // minimum size
            TS_TRUE,            // size without stretching
            TS_DRAW             // size that theme mgr will use to draw part
        };

        enum MENUPARTS
        {
            MENU_MENUITEM_TMSCHEMA = 1,
            MENU_MENUDROPDOWN_TMSCHEMA = 2,
            MENU_MENUBARITEM_TMSCHEMA = 3,
            MENU_MENUBARDROPDOWN_TMSCHEMA = 4,
            MENU_CHEVRON_TMSCHEMA = 5,
            MENU_SEPARATOR_TMSCHEMA = 6,
            MENU_BARBACKGROUND = 7,
            MENU_BARITEM = 8,
            MENU_POPUPBACKGROUND = 9,
            MENU_POPUPBORDERS = 10,
            MENU_POPUPCHECK = 11,
            MENU_POPUPCHECKBACKGROUND = 12,
            MENU_POPUPGUTTER = 13,
            MENU_POPUPITEM = 14,
            MENU_POPUPSEPARATOR = 15,
            MENU_POPUPSUBMENU = 16,
            MENU_SYSTEMCLOSE = 17,
            MENU_SYSTEMMAXIMIZE = 18,
            MENU_SYSTEMMINIMIZE = 19,
            MENU_SYSTEMRESTORE = 20
        };

    public:
        CMenuMetrics();
        ~CMenuMetrics();

        CRect GetCheckBackgroundRect(const CRect& item) const;
        CRect GetCheckRect(const CRect& item) const;
        CRect GetGutterRect(const CRect& item) const;
        CSize GetItemSize(MenuItemData* pmd) const;
        CRect GetSelectionRect(const CRect& item) const;
        CRect GetSeperatorRect(const CRect& item) const;
        CRect GetTextRect(const CRect& item) const;
        CSize GetTextSize(MenuItemData* pmd) const;
        void  Initialize(HWND frame);
        BOOL  IsVistaMenu() const;
        CRect ScaleRect(const CRect& item) const;
        CSize ScaleSize(const CSize& item) const;
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

        HANDLE  m_theme;                // Theme handle
        HWND    m_frame;                // Handle to the frame window
        HMODULE m_uxTheme;              // Module handle to the UXTheme dll

        Margins m_marCheck;            // Check margins
        Margins m_marCheckBackground;  // Check background margins
        Margins m_marItem;             // Item margins
        Margins m_marText;             // Text margins

        CSize   m_sizeCheck;            // Check size metric
        CSize   m_sizeSeparator;        // Separator size metric

    private:
        typedef HRESULT WINAPI CLOSETHEMEDATA(HANDLE);
        typedef HRESULT WINAPI DRAWTHEMEBACKGROUND(HANDLE, HDC, int, int, const RECT*, const RECT*);
        typedef HRESULT WINAPI DRAWTHEMETEXT(HANDLE, HDC, int, int, LPCWSTR, int, DWORD, DWORD, LPCRECT);
        typedef HRESULT WINAPI GETTHEMEPARTSIZE(HANDLE, HDC, int, int, LPCRECT, THEMESIZE, SIZE*);
        typedef HRESULT WINAPI GETTHEMEINT(HANDLE, int, int, int, int*);
        typedef HRESULT WINAPI GETTHEMEMARGINS(HANDLE, HDC, int, int, int, LPRECT, Margins*);
        typedef HRESULT WINAPI GETTHEMETEXTEXTENT(HANDLE, HDC, int, int, LPCWSTR, int, DWORD, LPCRECT, LPCRECT);
        typedef BOOL    WINAPI ISTHEMEBGPARTTRANSPARENT(HANDLE, int, int);
        typedef HANDLE  WINAPI OPENTHEMEDATA(HWND, LPCWSTR);

        // Pointers to functions defined in uxTheme.dll
        CLOSETHEMEDATA*           m_pfnCloseThemeData;
        DRAWTHEMEBACKGROUND*      m_pfnDrawThemeBackground;
        DRAWTHEMETEXT*            m_pfnDrawThemeText;
        GETTHEMEPARTSIZE*         m_pfnGetThemePartSize;
        GETTHEMEINT*              m_pfnGetThemeInt;
        GETTHEMEMARGINS*          m_pfnGetThemeMargins;
        GETTHEMETEXTEXTENT*       m_pfnGetThemeTextExtent;
        ISTHEMEBGPARTTRANSPARENT* m_pfnIsThemeBGPartTransparent;
        OPENTHEMEDATA*            m_pfnOpenThemeData;
    };

}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


namespace Win32xx
{

    //////////////////////////////////////////
    // Definitions for the CMenuMetrics class.
    //
    inline CMenuMetrics::CMenuMetrics() : m_theme(0), m_uxTheme(0), m_pfnCloseThemeData(0), m_pfnDrawThemeBackground(0),
                                            m_pfnDrawThemeText(0), m_pfnGetThemePartSize(0), m_pfnGetThemeInt(0),
                                            m_pfnGetThemeMargins(0), m_pfnGetThemeTextExtent(0),
                                            m_pfnIsThemeBGPartTransparent(0), m_pfnOpenThemeData(0)
    {
        m_frame = 0;
    }

    inline CMenuMetrics::~CMenuMetrics()
    {
        if (m_theme != 0)
            CloseThemeData();

        if (m_uxTheme != 0)
            ::FreeLibrary(m_uxTheme);
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

        int x = item.left + m_marCheckBackground.cxLeftWidth;
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
        int x = item.left + m_marCheckBackground.cxLeftWidth + m_marCheck.cxLeftWidth;
        int y = item.top + (item.Height() - m_sizeCheck.cy) / 2;

        return CRect(x, y, x + m_sizeCheck.cx, y + m_sizeCheck.cy);
    }

    // Retrieve the size of the menu item
    inline CSize CMenuMetrics::GetItemSize(MenuItemData* pmd) const
    {
        CSize size;

        // Add icon/check width.
        size.cx += m_sizeCheck.cx + m_marCheck.Width();

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

            // Account for text size
            CSize sizeText = ScaleSize(GetTextSize(pmd));
            size.cx += sizeText.cx;
            size.cy = MAX(size.cy, sizeText.cy);

            // Account for icon or check height.
            size.cy = MAX(size.cy, m_sizeCheck.cy + m_marCheckBackground.Height() + m_marCheck.Height());
        }

        return (size);
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

    inline CSize CMenuMetrics::GetTextSize(MenuItemData* pmd) const
    {
        CSize sizeText;
        assert(m_frame);
        CClientDC DesktopDC(0);
        LPCTSTR szItemText = pmd->GetItemText();

        if (IsVistaMenu())
        {
            CRect rcText;
            GetThemeTextExtent(DesktopDC, MENU_POPUPITEM, 0, TtoW(szItemText), lstrlen(szItemText),
                DT_LEFT | DT_SINGLELINE, NULL, &rcText);

            sizeText.SetSize(rcText.right + m_marText.Width(), rcText.bottom + m_marText.Height());
        }
        else
        {
            // Get the font used in menu items.
            NONCLIENTMETRICS info = GetNonClientMetrics();

            // Default menu items are bold, so take this into account.
            if (static_cast<int>(::GetMenuDefaultItem(pmd->menu, TRUE, GMDI_USEDISABLED)) != -1)
                info.lfMenuFont.lfWeight = FW_BOLD;

            // Calculate the size of the text.
            DesktopDC.CreateFontIndirect(info.lfMenuFont);
            sizeText = DesktopDC.GetTextExtentPoint32(szItemText, lstrlen(szItemText));
            sizeText.cx += m_marText.cxRightWidth;
            sizeText.cy += m_marText.Height();
        }

        if (_tcschr(szItemText, _T('\t')))
            sizeText.cx += 8;   // Add POST_TEXT_GAP if the text includes a tab.

        return sizeText;
    }

    inline CRect CMenuMetrics::GetTextRect(const CRect& item) const
    {
        int left = GetGutterRect(item).Width() + m_marText.cxLeftWidth;
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

    inline void CMenuMetrics::Initialize(HWND frame)
    {
        assert(IsWindow(frame));
        m_frame = frame;

        if (m_uxTheme == 0)
            m_uxTheme = ::LoadLibrary(_T("UXTHEME.DLL"));

        if (m_uxTheme != 0)
        {
            m_pfnCloseThemeData = reinterpret_cast<CLOSETHEMEDATA*>(::GetProcAddress(m_uxTheme, "CloseThemeData"));
            m_pfnDrawThemeBackground = reinterpret_cast<DRAWTHEMEBACKGROUND*>(::GetProcAddress(m_uxTheme, "DrawThemeBackground"));
            m_pfnDrawThemeText = reinterpret_cast<DRAWTHEMETEXT*>(::GetProcAddress(m_uxTheme, "DrawThemeText"));
            m_pfnGetThemePartSize = reinterpret_cast<GETTHEMEPARTSIZE*>(::GetProcAddress(m_uxTheme, "GetThemePartSize"));
            m_pfnGetThemeInt = reinterpret_cast<GETTHEMEINT*>(::GetProcAddress(m_uxTheme, "GetThemeInt"));
            m_pfnGetThemeMargins = reinterpret_cast<GETTHEMEMARGINS*>(::GetProcAddress(m_uxTheme, "GetThemeMargins"));
            m_pfnGetThemeTextExtent = reinterpret_cast<GETTHEMETEXTEXTENT*>(::GetProcAddress(m_uxTheme, "GetThemeTextExtent"));
            m_pfnIsThemeBGPartTransparent = reinterpret_cast<ISTHEMEBGPARTTRANSPARENT*>(::GetProcAddress(m_uxTheme, "IsThemeBackgroundPartiallyTransparent"));
            m_pfnOpenThemeData = reinterpret_cast<OPENTHEMEDATA*>(::GetProcAddress(m_uxTheme, "OpenThemeData"));
        }

        if (m_theme != 0)
        {
            CloseThemeData();
            m_theme = 0;
        }

        m_theme = OpenThemeData(m_frame, VSCLASS_MENU);

        if (m_theme != 0)
        {
            int borderSize = 0;    // Border space between item text and accelerator
            int bgBorderSize = 0;  // Border space between item text and gutter
            GetThemePartSize(0, MENU_POPUPCHECK, 0, NULL, TS_TRUE, &m_sizeCheck);
            GetThemePartSize(0, MENU_POPUPSEPARATOR, 0, NULL, TS_TRUE, &m_sizeSeparator);
            GetThemeInt(MENU_POPUPITEM, 0, TMT_BORDERSIZE, &borderSize);
            GetThemeInt(MENU_POPUPBACKGROUND, 0, TMT_BORDERSIZE, &bgBorderSize);
            GetThemeMargins(0, MENU_POPUPCHECK, 0, TMT_CONTENTMARGINS, NULL, &m_marCheck);
            GetThemeMargins(0, MENU_POPUPCHECKBACKGROUND, 0, TMT_CONTENTMARGINS, NULL, &m_marCheckBackground);
            GetThemeMargins(0, MENU_POPUPITEM, 0, TMT_CONTENTMARGINS, NULL, &m_marItem);

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
        assert(wnd);
        if (m_pfnOpenThemeData)
            return m_pfnOpenThemeData(wnd, classList);

        return 0;
    }

    inline BOOL CMenuMetrics::IsVistaMenu() const
    {
        return (m_theme != FALSE);
    }

    // Re-scale the CRect to support the system's DPI.
    inline CRect CMenuMetrics::ScaleRect(const CRect& item) const
    {
        // DC for the desktop
        CWindowDC dc(0);

        int dpiX = dc.GetDeviceCaps(LOGPIXELSX);
        int dpiY = dc.GetDeviceCaps(LOGPIXELSY);

        CRect rc  = item;
        rc.left   = MulDiv(rc.left, dpiX, 96);
        rc.right  = MulDiv(rc.right, dpiX, 96);
        rc.top    = MulDiv(rc.top, dpiY, 96);
        rc.bottom = MulDiv(rc.bottom, dpiY, 96);

        return rc;
    }

    // Re-scale the CSize to support the system's DPI.
    inline CSize CMenuMetrics::ScaleSize(const CSize& item) const
    {
        // DC for the desktop
        CWindowDC dc(0);

        int dpiX = dc.GetDeviceCaps(LOGPIXELSX);
        int dpiY = dc.GetDeviceCaps(LOGPIXELSY);

        CSize sz = item;
        sz.cx = MulDiv(sz.cx, dpiX, 96);
        sz.cy = MulDiv(sz.cy, dpiY, 96);

        return sz;
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

#if defined (_MSC_VER) && (_MSC_VER >= 1920)
 #pragma warning ( pop )
#endif // (_MSC_VER) && (_MSC_VER >= 1920)

#endif // _WIN32XX_MENUMETRICS_H_
