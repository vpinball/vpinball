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
// wxx_textconv.h
//  Definitions of the CAtoA, CAtoW, CWtoA, CWtoW,
//    CAtoBSTR and CWtoBSTR classes

#ifndef _WIN32XX_TEXTCONV_H_
#define _WIN32XX_TEXTCONV_H_

#include <vector>

namespace Win32xx
{
    ////////////////////////////////////////////////////////
    // Classes and functions (typedefs) for text conversions
    //
    //  This section defines the following text conversions:
    //  AtoBSTR     ANSI  to BSTR
    //  AtoOLE      ANSI  to OLE
    //  AtoT        ANSI  to TCHAR
    //  AtoW        ANSI  to WCHAR
    //  OLEtoA      OLE   to ANSI
    //  OLEtoT      OLE   to TCHAR
    //  OLEtoW      OLE   to WCHAR
    //  TtoA        TCHAR to ANSI
    //  TtoBSTR     TCHAR to BSTR
    //  TtoOLE      TCHAR to OLE
    //  TtoW        TCHAR to WCHAR
    //  WtoA        WCHAR to ANSI
    //  WtoBSTR     WCHAR to BSTR
    //  WtoOLE      WCHAR to OLE
    //  WtoT        WCHAR to TCHAR

    // About different character and string types:
    // ------------------------------------------
    // char (or CHAR) character types are ANSI (8 bits).
    // wchar_t (or WCHAR) character types are Unicode (16 bits).
    // TCHAR characters are Unicode if the _UNICODE macro is defined, otherwise they are ANSI.
    // BSTR (Basic String) is a type of string used in Visual Basic and COM programming.
    // OLE is the same as WCHAR. It is used in Visual Basic and COM programming.


    // Forward declarations of our classes. They are defined later.
    class CAtoA;
    class CAtoW;
    class CWtoA;
    class CWtoW;
    class CAtoBSTR;
    class CWtoBSTR;

    // typedefs for the well known text conversions
    typedef CAtoW AtoW;
    typedef CWtoA WtoA;
    typedef CWtoBSTR WtoBSTR;
    typedef CAtoBSTR AtoBSTR;
    typedef CWtoA BSTRtoA;
    typedef CWtoW BSTRtoW;

#ifdef UNICODE
    typedef CAtoW AtoT;
    typedef CWtoA TtoA;
    typedef CWtoW TtoW;
    typedef CWtoW WtoT;
    typedef CWtoBSTR TtoBSTR;
    typedef BSTRtoW BSTRtoT;
#else
    typedef CAtoA AtoT;
    typedef CAtoA TtoA;
    typedef CAtoW TtoW;
    typedef CWtoA WtoT;
    typedef CAtoBSTR TtoBSTR;
    typedef BSTRtoA BSTRtoT;
#endif

    typedef AtoW  AtoOLE;
    typedef TtoW  TtoOLE;
    typedef CWtoW WtoOLE;
    typedef WtoA  OLEtoA;
    typedef WtoT  OLEtoT;
    typedef CWtoW OLEtoW;

    class CAtoW
    {
    public:
        CAtoW(LPCSTR str, UINT codePage = CP_ACP, int charCount = -1);
        ~CAtoW();
        operator LPCWSTR() { return m_str? &m_wideArray[0] : NULL; }
        operator LPOLESTR() { return m_str? (LPOLESTR)&m_wideArray[0] : (LPOLESTR)NULL; }
        LPCWSTR c_str() { return m_str ? &m_wideArray[0] : NULL; }

    private:
        CAtoW(const CAtoW&);
        CAtoW& operator= (const CAtoW&);
        std::vector<wchar_t> m_wideArray;
        LPCSTR m_str;
    };

    class CWtoA
    {
    public:
        CWtoA(LPCWSTR str, UINT codePage = CP_ACP, int charCount = -1);
        ~CWtoA();
        operator LPCSTR() { return m_str? &m_ansiArray[0] : NULL; }
        LPCSTR c_str() { return m_str ? &m_ansiArray[0] : NULL; }

    private:
        CWtoA(const CWtoA&);
        CWtoA& operator= (const CWtoA&);
        std::vector<char> m_ansiArray;
        LPCWSTR m_str;
    };

    class CWtoW
    {
    public:
        CWtoW(LPCWSTR pWStr, UINT codePage = CP_ACP, int charCount = -1);
        operator LPCWSTR() { return m_str; }
        operator LPOLESTR() { return (LPOLESTR)m_str; }
        LPCWSTR c_str() { return m_str; }

    private:
        CWtoW(const CWtoW&);
        CWtoW& operator= (const CWtoW&);

        LPCWSTR m_str;
    };

    class CAtoA
    {
    public:
        CAtoA(LPCSTR str, UINT codePage = CP_ACP, int charCount = -1);
        operator LPCSTR() { return m_str; }
        LPCSTR c_str() { return m_str; }

    private:
        CAtoA(const CAtoA&);
        CAtoA& operator= (const CAtoA&);

        LPCSTR m_str;
    };

    class CWtoBSTR
    {
    public:
        CWtoBSTR(LPCWSTR pWStr) { m_bstrString = ::SysAllocString(pWStr); }
        ~CWtoBSTR() { ::SysFreeString(m_bstrString); }
        operator BSTR() { return m_bstrString;}

    private:
        CWtoBSTR(const CWtoBSTR&);
        CWtoBSTR& operator= (const CWtoBSTR&);
        BSTR m_bstrString;
    };

    class CAtoBSTR
    {
    public:
        CAtoBSTR(LPCSTR str) { m_bstrString = ::SysAllocString(AtoW(str)); }
        ~CAtoBSTR() { ::SysFreeString(m_bstrString); }
        operator BSTR() { return m_bstrString;}

    private:
        CAtoBSTR(const CAtoBSTR&);
        CAtoBSTR& operator= (const CAtoBSTR&);
        BSTR m_bstrString;
    };

}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

namespace Win32xx
{

    inline CAtoW::CAtoW(LPCSTR str, UINT codePage /*= CP_ACP*/, int charCount /*= -1*/) : m_str(str)
    {
        if (str)
        {
            // Resize the vector and assign null WCHAR to each element.
            int charSize = static_cast<int>(sizeof(CHAR));
            int charBytes = (charCount == -1) ? -1 : charSize * charCount;
            int length = MultiByteToWideChar(codePage, 0, str, charBytes, NULL, 0) + 1;
            m_wideArray.assign(static_cast<size_t>(length), L'\0');

            // Fill our vector with the converted WCHAR array.
            MultiByteToWideChar(codePage, 0, str, charBytes, &m_wideArray[0], length);
        }
    }

    inline CAtoW::~CAtoW()
    {
        // Clear the array.
        std::fill(m_wideArray.begin(), m_wideArray.end(), L'\0');
    }

    // Usage:
    //   CWtoA ansiString(L"Some Text");
    //   CWtoA utf8String(L"Some Text", CP_UTF8);
    //
    // or
    //   SetWindowTextA( WtoA(L"Some Text") ); The ANSI version of SetWindowText
    inline CWtoA::CWtoA(LPCWSTR str, UINT codePage /*= CP_ACP*/, int charCount /*= -1*/) : m_str(str)
    {
        // Resize the vector and assign null char to each element
        int charSize = static_cast<int>(sizeof(WCHAR));
        int charBytes = (charCount == -1) ? -1 : charSize * charCount;
        int length = WideCharToMultiByte(codePage, 0, str, charBytes, NULL, 0, NULL, NULL) + 1;
        m_ansiArray.assign(static_cast<size_t>(length), '\0');

        // Fill our vector with the converted char array
        WideCharToMultiByte(codePage, 0, str, charCount, &m_ansiArray[0], length, NULL, NULL);
    }

    inline CWtoA::~CWtoA()
    {
        // Clear the array.
        std::fill(m_ansiArray.begin(), m_ansiArray.end(), '\0');
    }

    inline CWtoW::CWtoW(LPCWSTR str, UINT /*codePage = CP_ACP*/, int /*charCount = -1*/) : m_str(str)
    {
    }

    inline CAtoA::CAtoA(LPCSTR str, UINT /*codePage = CP_ACP*/, int /*charCount = -1*/) : m_str(str)
    {
    }


    ////////////////////////////////////////
    // Global Functions
    //

    // Trace sends a string to the debug/output pane, or an external debugger.
    inline void Trace(LPCSTR str)
    {
        OutputDebugString(AtoT(str));
    }

    // Trace sends a string to the debug/output pane, or an external debugger.
    inline void Trace(LPCWSTR str)
    {
        OutputDebugString(WtoT(str));
    }


} // namespace Win32xx

#endif // _WIN32XX_TEXTCONV_H_
