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


///////////////////////////////////////////////////////
// wxx_themes.h
//  Declaration of the following structs:
//  MenuTheme, ReBarTheme, StatusBarTheme and ToolBarTheme


#ifndef _WIN32XX_THEMES_H_
#define _WIN32XX_THEMES_H_


namespace Win32xx
{

    ////////////////////////////////////////////////
    // Declarations of structures for themes
    //

    // Defines the theme colors for the MenuBar and popup menues.
    // The popup menu colors are replaced by the Aero theme if available (Vista and above)
    struct MenuTheme
    {
        BOOL UseThemes;         // TRUE if themes are used
        COLORREF clrHot1;       // Colour 1 for top menu. Color of selected menu item
        COLORREF clrHot2;       // Colour 2 for top menu. Color of checkbox
        COLORREF clrPressed1;   // Colour 1 for pressed top menu and side bar
        COLORREF clrPressed2;   // Colour 2 for pressed top menu and side bar
        COLORREF clrOutline;    // Colour for border outline
    };


    // Defines the theme colors and options for the ReBar
    struct ReBarTheme
    {
        BOOL UseThemes;         // TRUE if themes are used
        COLORREF clrBkgnd1;     // Colour 1 for rebar background
        COLORREF clrBkgnd2;     // Colour 2 for rebar background
        COLORREF clrBand1;      // Colour 1 for rebar band background. Use NULL if not required
        COLORREF clrBand2;      // Colour 2 for rebar band background. Use NULL if not required
        BOOL FlatStyle;         // Bands are rendered with flat rather than raised style
        BOOL BandsLeft;         // Position bands left on rearrange
        BOOL LockMenuBand;      // Lock MenuBar's band in dedicated top row, without gripper
        BOOL RoundBorders;      // Use rounded band borders
        BOOL ShortBands;        // Allows bands to be shorter than maximum available width
        BOOL UseLines;          // Displays horizontal lines between bands
    };


    // Defines the theme colors and options for the StatusBar
    struct StatusBarTheme
    {
        BOOL UseThemes;         // TRUE if themes are used
        COLORREF clrBkgnd1;     // Colour 1 for statusbar background
        COLORREF clrBkgnd2;     // Colour 2 for statusbar background
    };


    // Defines the theme colors for the ToolBar
    struct ToolBarTheme
    {
        BOOL UseThemes;         // TRUE if themes are used
        COLORREF clrHot1;       // Colour 1 for hot button
        COLORREF clrHot2;       // Colour 2 for hot button
        COLORREF clrPressed1;   // Colour 1 for pressed button
        COLORREF clrPressed2;   // Colour 2 for pressed button
        COLORREF clrOutline;    // Colour for border outline
    };


  #ifndef _WIN32_WCE        // for Win32/64 operating systems, not WinCE

    // Returns TRUE if Aero themes are being used.
    inline BOOL IsAeroThemed()
    {
        BOOL IsAeroThemed = FALSE;

        // Test if Windows version is XP or greater
        if (GetWinVersion() >= 2501)
        {
            HMODULE hMod = ::LoadLibrary(_T("uxtheme.dll"));

            if(hMod != 0)
            {
                // Declare pointers to IsCompositionActive function
                FARPROC pIsCompositionActive = ::GetProcAddress(hMod, "IsCompositionActive");

                if(pIsCompositionActive)
                {
                    if(pIsCompositionActive())
                    {
                        IsAeroThemed = TRUE;
                    }
                }
                ::FreeLibrary(hMod);
            }
        }

        return IsAeroThemed;
    }


    // Returns TRUE if XP themes are being used.
    inline BOOL IsXPThemed()
    {
        BOOL IsXPThemed = FALSE;

        // Test if Windows version is XP or greater
        if (GetWinVersion() >= 2501)
        {
            HMODULE hMod = ::LoadLibrary(_T("uxtheme.dll"));
            if(hMod != 0)
            {
                // Declare pointers to functions
                FARPROC pIsAppThemed   = ::GetProcAddress(hMod, "IsAppThemed");
                FARPROC pIsThemeActive = ::GetProcAddress(hMod, "IsThemeActive");

                if(pIsAppThemed && pIsThemeActive)
                {
                    if(pIsAppThemed() && pIsThemeActive())
                    {
                        // Test if ComCtl32 dll used is version 6 or later
                        IsXPThemed = (GetComCtlVersion() >= 600);
                    }
                }
                ::FreeLibrary(hMod);
            }
        }

        return IsXPThemed;
    }

  #endif // #ifndef _WIN32_WCE


} // namespace Win32xx

#endif // _WIN32XX_THEMES_H_

