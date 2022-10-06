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
// This file defines the set of macros and includes the
// C, C++, and windows header files required by Win32++.
// It also defines the CObject class.


#ifndef _WIN32XX_SETUP_H_
#define _WIN32XX_SETUP_H_

// Set the version macros if they aren't already set.
// These values are suitable for Windows 10 and Windows 11.
#ifndef WINVER
  #define WINVER            0x0A00
  #undef  _WIN32_WINNT
  #define _WIN32_WINNT      0x0A00
  #undef  _WIN32_IE
  #define _WIN32_IE         0x0A00
  #undef  NTDDI_VERSION
  #define NTDDI_VERSION     0x0A000000
#endif
#ifndef _WIN32_WINDOWS
  #define _WIN32_WINDOWS    WINVER
#endif
#ifndef _WIN32_IE
  #define _WIN32_IE         WINVER
#endif


// Remove pointless warning messages.
#ifdef _MSC_VER
  #if _MSC_VER < 1310    // before VS2003
    #pragma warning (disable : 4511) // copy operator could not be generated
    #pragma warning (disable : 4512) // assignment operator could not be generated
    #pragma warning (disable : 4702) // unreachable code (bugs in Microsoft's STL)
    #pragma warning (disable : 4786) // identifier was truncated
  #endif
#endif

#ifdef __BORLANDC__
  #pragma option -w-8026            // functions with exception specifications are not expanded inline
  #pragma option -w-8027            // function not expanded inline
  #pragma option -w-8030            // Temporary used for 'rhs'
  #pragma option -w-8004            // Assigned value is never used
  #define STRICT 1
#endif


#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "wxx_shared_ptr.h"

#include <WinSock2.h>   // must include before windows.h
#include <Windows.h>
#include <CommCtrl.h>
#include <Shlwapi.h>

#include <cassert>
#include <vector>
#include <algorithm>
#include <string>
#include <map>
#include <sstream>
#include <stdio.h>
#include <stdarg.h>
#include <tchar.h>
#include <process.h>

// Required by compilers lacking Win64 support.
#ifndef  GetWindowLongPtr
  #define GetWindowLongPtr   GetWindowLong
  #define SetWindowLongPtr   SetWindowLong
  #define GWLP_WNDPROC       GWL_WNDPROC
  #define GWLP_HINSTANCE     GWL_HINSTANCE
  #define GWLP_ID            GWL_ID
  #define GWLP_USERDATA      GWL_USERDATA
  #define DWLP_DLGPROC       DWL_DLGPROC
  #define DWLP_MSGRESULT     DWL_MSGRESULT
  #define DWLP_USER          DWL_USER
  #define DWORD_PTR          DWORD
  #define LONG_PTR           LONG
  #define ULONG_PTR          LONG
#endif
#ifndef GetClassLongPtr
  #define GetClassLongPtr    GetClassLong
  #define SetClassLongPtr    SetClassLong
  #define GCLP_HBRBACKGROUND GCL_HBRBACKGROUND
  #define GCLP_HCURSOR       GCL_HCURSOR
  #define GCLP_HICON         GCL_HICON
  #define GCLP_HICONSM       GCL_HICONSM
  #define GCLP_HMODULE       GCL_HMODULE
  #define GCLP_MENUNAME      GCL_MENUNAME
  #define GCLP_WNDPROC       GCL_WNDPROC
#endif


// Automatically include the Win32xx namespace.
// define NO_USING_NAMESPACE to skip this step.
namespace Win32xx {}
#ifndef NO_USING_NAMESPACE
using namespace Win32xx;
#endif


// Ensure UNICODE and _UNICODE definitions are consistent.
#ifdef _UNICODE
  #ifndef UNICODE
    #define UNICODE
  #endif
#endif

#ifdef UNICODE
  #ifndef _UNICODE
    #define _UNICODE
  #endif
#endif

// Define our own MIN and MAX macros.
// This avoids inconsistencies with MinGW and other compilers, and
// avoids conflicts between typical min/max macros and std::min/std::max
#define MAX(a,b)        (((a) > (b)) ? (a) : (b))
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))

// Version macro
#define _WIN32XX_VER 0x0910     // Win32++ version 9.1.0

// Define the TRACE Macro.
// In debug mode, TRACE send text to the debug/output pane, or an external debugger
// In release mode, TRACE is ignored.
#ifndef TRACE
  #ifdef NDEBUG
    #define TRACE(str) (void(0))
  #else
    #define TRACE(str) Trace(str)
  #endif
#endif

// Define the VERIFY macro
// In debug mode, VERIFY asserts if the expression evaluates to zero
// In release mode, VERIFY evaluates the expression, but doesn't assert.
#ifndef VERIFY
  #ifdef NDEBUG
    #define VERIFY(f) ((void)(f))
  #else
    #define VERIFY(f) assert(f)
  #endif
#endif

// tString is a TCHAR std::string
typedef std::basic_string<TCHAR> tString;
typedef std::basic_stringstream<TCHAR> tStringStream;

namespace Win32xx
{

    class CArchive;

    ////////////////////////////////////////////////////////////////////
    // The CObject class provides support for Serialization by CArchive.
    class CObject
    {
    public:
        CObject() {}
        virtual ~CObject() {}

        virtual void Serialize(CArchive& ar);
    };


    ///////////////////////////////////////
    // Definitions for the CObject class
    //
    inline void CObject::Serialize(CArchive& /* ar */ )
    {
    //  Override Serialize in the class inherited from CObject like this.

    //  if (ar.IsStoring())
    //  {
    //      // Store a member variable in the archive
    //      ar << m_someValue;
    //  }
    //  else
    //  {
    //      // Load a member variable from the archive
    //      ar >> m_someValue;
    //  }

    }

    ////////////////////////////////////////
    // Global Functions
    //

    // Retrieves the version of common control dll used.
    // return values and DLL versions
    // 400  dll ver 4.00    Windows 95/Windows NT 4.0
    // 470  dll ver 4.70    Internet Explorer 3.x
    // 471  dll ver 4.71    Internet Explorer 4.0
    // 472  dll ver 4.72    Internet Explorer 4.01 and Windows 98
    // 580  dll ver 5.80    Internet Explorer 5
    // 581  dll ver 5.81    Windows 2000 and Windows ME
    // 582  dll ver 5.82    Windows XP, Vista, Windows 7 etc. without XP themes
    // 600  dll ver 6.00    Windows XP with XP themes
    // 610  dll ver 6.10    Windows Vista with XP themes
    // 616  dll ver 6.16    Windows Vista SP1 or above with XP themes
    inline int GetComCtlVersion()
    {
        // Load the Common Controls DLL.
        HMODULE comCtl = ::LoadLibrary(_T("COMCTL32.DLL"));
        if (comCtl == 0)
            return 0;

        DWORD comCtlVer = 400;

        if (::GetProcAddress(comCtl, "InitCommonControlsEx"))
        {
            // InitCommonControlsEx is unique to 4.7 and later.
            comCtlVer = 470;

            if (::GetProcAddress(comCtl, "DllGetVersion"))
            {
                DLLGETVERSIONPROC pfnDLLGetVersion;

                pfnDLLGetVersion = reinterpret_cast<DLLGETVERSIONPROC>(
                    reinterpret_cast<void*>(::GetProcAddress(comCtl, "DllGetVersion")));
                if (pfnDLLGetVersion)
                {
                    DLLVERSIONINFO dvi;
                    ZeroMemory(&dvi, sizeof(dvi));
                    dvi.cbSize = sizeof dvi;
                    if (NOERROR == pfnDLLGetVersion(&dvi))
                    {
                        DWORD verMajor = dvi.dwMajorVersion;
                        DWORD verMinor = dvi.dwMinorVersion;
                        comCtlVer = 100 * verMajor + verMinor;
                    }
                }
            }
            else if (::GetProcAddress(comCtl, "InitializeFlatSB"))
                comCtlVer = 471;    // InitializeFlatSB is unique to version 4.71.
        }

        ::FreeLibrary(comCtl);

        return static_cast<int>(comCtlVer);
    }

    // Retrieves the window version
    // Return values and window versions:
    //  1400     Windows 95
    //  1410     Windows 98
    //  1490     Windows ME
    //  2400     Windows NT
    //  2500     Windows 2000
    //  2501     Windows XP
    //  2502     Windows Server 2003
    //  2600     Windows Vista and Windows Server 2008
    //  2601     Windows 7 and Windows Server 2008 r2
    //  2602     Windows 8 and Windows Server 2012
    //  2603     Windows 8.1 and Windows Server 2012 r2
    //  3000     Windows 10
    // Note: For windows 8.1 and above, the value returned is also affected by the embedded manifest
    //       Applications not manifested for Windows 8.1 or Windows 10 will return the Windows 8 OS (2602).
    inline int GetWinVersion()
    {
#if defined (_MSC_VER) && (_MSC_VER >= 1400)   // >= VS2005
#pragma warning ( push )
#pragma warning ( disable : 4996 )           // GetVersion declared deprecated.
#pragma warning ( disable : 28159 )          // Deprecated function.
#endif // (_MSC_VER) && (_MSC_VER >= 1400)

#if defined __clang_major__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"  // Disable Clang deprecated warning.
#endif

        OSVERSIONINFO osvi;
        ZeroMemory(&osvi, sizeof(osvi));
        osvi.dwOSVersionInfoSize = sizeof(osvi);
        GetVersionEx(&osvi);

#if defined __clang_major__
#pragma clang diagnostic pop
#endif

#if defined (_MSC_VER) && (_MSC_VER >= 1400)
#pragma warning ( pop )
#endif // (_MSC_VER) && (_MSC_VER >= 1400)

        DWORD platform = osvi.dwPlatformId;
        DWORD majorVer = osvi.dwMajorVersion;
        DWORD minorVer = osvi.dwMinorVersion;

        DWORD result = 1000 * platform + 100 * majorVer + minorVer;
        return static_cast<int>(result);
    }

    // Returns a NONCLIENTMETRICS struct filled from the system parameters.
    // Refer to NONCLIENTMETRICS in the Windows API documentation for more information.
    inline NONCLIENTMETRICS GetNonClientMetrics()
    {
        NONCLIENTMETRICS ncm;
        ZeroMemory(&ncm, sizeof(ncm));
        ncm.cbSize = sizeof(ncm);

#if (WINVER >= 0x0600)
        // If OS version less than Vista, adjust size to correct value.
        if (GetWinVersion() < 2600)
            ncm.cbSize = CCSIZEOF_STRUCT(NONCLIENTMETRICS, lfMessageFont);
#endif

        VERIFY(::SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0));

        return ncm;
    }

    // Reports the state of the left mouse button.
    // Refer to GetAsyncKeyState in the Windows API documentation for more information.
    inline BOOL IsLeftButtonDown()
    {
        SHORT state;
        if (::GetSystemMetrics(SM_SWAPBUTTON))
            // Mouse buttons are swapped.
            state = ::GetAsyncKeyState(VK_RBUTTON);
        else
            // Mouse buttons are not swapped.
            state = ::GetAsyncKeyState(VK_LBUTTON);

        // Returns true if the left mouse button is down.
        return (state & 0x8000);
    }

    // Loads the common controls using InitCommonControlsEx or InitCommonControls.
    // Returns TRUE if InitCommonControlsEx is used.
    // Refer to InitCommonControlsEx in the Windows API documentation for more information.
    inline void LoadCommonControls()
    {
        // Load the Common Controls DLL
        HMODULE comCtl = ::LoadLibrary(_T("COMCTL32.DLL"));
        if (comCtl == 0)
            comCtl = ::LoadLibrary(_T("COMMCTRL.DLL"));

        if (comCtl)
        {
            // Declare a typedef for the InItCommonControlsEx function.
            typedef BOOL WINAPI INIT_EX(INITCOMMONCONTROLSEX*);

            INIT_EX* pfnInitEx = reinterpret_cast<INIT_EX*>(
                reinterpret_cast<void*>(::GetProcAddress(comCtl, "InitCommonControlsEx")));

            if (pfnInitEx)
            {
                // Load the full set of common controls.
                INITCOMMONCONTROLSEX InitStruct;
                InitStruct.dwSize = sizeof(InitStruct);
                InitStruct.dwICC = ICC_WIN95_CLASSES | ICC_BAR_CLASSES | ICC_COOL_CLASSES | ICC_DATE_CLASSES;


#if (_WIN32_IE >= 0x0401)
                if (GetComCtlVersion() > 470)
                    InitStruct.dwICC |= ICC_INTERNET_CLASSES | ICC_NATIVEFNTCTL_CLASS | ICC_PAGESCROLLER_CLASS | ICC_USEREX_CLASSES;
#endif

                // Call InitCommonControlsEx.
                if (!(pfnInitEx(&InitStruct)))
                    InitCommonControls();
            }
            else
            {
                // InitCommonControlsEx not supported. Use older InitCommonControls.
                InitCommonControls();
            }

            ::FreeLibrary(comCtl);
        }
    }

    // The following functions perform string copies. The size of the dst buffer
    // is specified, much like strcpy_s. The dst buffer is always null terminated.
    // Null or zero arguments cause an assert.

    // Copies an ANSI string from src to dst.
    inline void StrCopyA(char* dst, const char* src, size_t dst_size)
    {
        assert(dst != 0);
        assert(src != 0);
        assert(dst_size != 0);

        if (dst && src && dst_size != 0)
        {
            size_t index;

            // Copy each character.
            for (index = 0; index < dst_size - 1; ++index)
            {
                dst[index] = src[index];
                if (src[index] == '\0')
                    break;
            }

            // Add null termination if required.
            if (dst[index] != '\0')
                dst[dst_size - 1] = '\0';
        }
    }

    // Copies a wide string from src to dst.
    inline void StrCopyW(wchar_t* dst, const wchar_t* src, size_t dst_size)
    {
        assert(dst != 0);
        assert(src != 0);
        assert(dst_size != 0);

        if (dst && src && dst_size != 0)
        {
            size_t index;

            // Copy each character.
            for (index = 0; index < dst_size - 1; ++index)
            {
                dst[index] = src[index];
                if (src[index] == '\0')
                    break;
            }

            // Add null termination if required.
            if (dst[index] != '\0')
                dst[dst_size - 1] = '\0';
        }
    }

    // Copies a TCHAR string from src to dst.
    inline void StrCopy(TCHAR* dst, const TCHAR* src, size_t dst_size)
    {
#ifdef UNICODE
        StrCopyW(dst, src, dst_size);
#else
        StrCopyA(dst, src, dst_size);
#endif
    }

}

#endif // _WIN32XX_SETUP_H_
