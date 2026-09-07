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


//////////////////////////////////////////////////
// wxx_textconv.h
//  Definitions of the CAtoA, CAtoW, CWtoA, CWtoW,
//    CAtoBSTR and CWtoBSTR classes

#ifndef WIN32XX_TEXTCONV_H_
#define WIN32XX_TEXTCONV_H_

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
    // TCHAR characters are Unicode if the _UNICODE macro is defined, otherwise
    //       they are ANSI.
    // BSTR (Basic String) is a string type used in Visual Basic and COM programming.
    // OLE is the same as WCHAR. It is used in Visual Basic and COM programming.

    // Usage:
    //   CAtoW wideString("Some Text");
    //   CAtoW utf8String("Some Text", CP_UTF8);
    //
    // or
    //   SetWindowTextW( WtoA("Some Text") ); The Wide version of SetWindowText
    //
    //   CWtoA ansiString(L"Some Text");
    //   CWtoA utf8String(L"Some Text", CP_UTF8);
    //
    // or
    //   SetWindowTextA( WtoA(L"Some Text") ); The ANSI version of SetWindowText


    // Forward declarations of our classes. They are defined later.
    class CAtoA;
    class CAtoW;
    class CWtoA;
    class CWtoW;
    class CAtoBSTR;
    class CWtoBSTR;

    // The using declarations for the well known text conversions.
    using AtoW = CAtoW;
    using WtoA = CWtoA;
    using WtoBSTR  = CWtoBSTR;
    using AtoBSTR = CAtoBSTR;
    using BSTRtoA = CWtoA;
    using BSTRtoW = CWtoW;

#ifdef UNICODE
    using AtoT = CAtoW;
    using TtoA = CWtoA;
    using TtoW = CWtoW;
    using WtoT = CWtoW;
    using TtoBSTR = CWtoBSTR;
    using BSTRtoT = BSTRtoW;
#else
    using AtoT = CAtoA;
    using TtoA = CAtoA;
    using TtoW = CAtoW;
    using WtoT = CWtoA;
    using TtoBSTR = CAtoBSTR;
    using BSTRtoT = BSTRtoA;
#endif

    using AtoOLE = AtoW;
    using TtoOLE = TtoW;
    using WtoOLE = CWtoW;
    using OLEtoA = WtoA;
    using OLEtoT = WtoT;
    using OLEtoW = CWtoW;


    ///////////////////////////////////////
    // The CAtoA class doesn't modify text.
    // It is used by AtoT and TtoA.
    //
    class CAtoA
    {
    public:
        CAtoA(LPCSTR str, UINT codePage = CP_ACP, int charCount = -1);
        operator LPCSTR() { return m_str; }
        LPCSTR c_str() const { return m_str; }
        int GetLength() const { return m_length; }

    private:
        CAtoA(const CAtoA&) = delete;
        CAtoA& operator= (const CAtoA&) = delete;

        LPCSTR m_str;
        int m_length;
    };

    /////////////////////////////////////////////////////////////////
    // The CAtoBSTR class creates a BSTR from a char character array.
    //
    class CAtoBSTR
    {
    public:
        CAtoBSTR(LPCSTR str);
        ~CAtoBSTR() { ::SysFreeString(m_bstrString); }
        operator BSTR() { return m_bstrString; }
        int GetLength() const { return m_length; }

    private:
        CAtoBSTR(const CAtoBSTR&) = delete;
        CAtoBSTR& operator= (const CAtoBSTR&) = delete;
        BSTR m_bstrString;
        int m_length;
    };

    ///////////////////////////////////////////////////////
    // The CAtoW class creates a wchar_t character array from
    // a char character array.
    //
    class CAtoW
    {
    public:
        CAtoW(LPCSTR str, UINT codePage = CP_ACP, int charCount = -1);
        ~CAtoW() = default;
        operator LPCWSTR() { return m_str? m_wideArray.data() : nullptr; }
        operator LPOLESTR() { return m_str? reinterpret_cast<LPOLESTR>(
            m_wideArray.data()) : nullptr; }

        LPCWSTR c_str() { return m_str ? m_wideArray.data() : nullptr; }
        int GetLength() const { return m_length; }

    private:
        CAtoW(const CAtoW&) = delete;
        CAtoW& operator= (const CAtoW&) = delete;
        std::vector<wchar_t> m_wideArray;
        LPCSTR m_str;
        int m_length;
    };

    ////////////////////////////////////////////////////////
    // The CWtoA class creates a char character array from a
    // wchar_t character array.
    //
    class CWtoA
    {
    public:
        CWtoA(LPCWSTR str, UINT codePage = CP_ACP, int charCount = -1);
        ~CWtoA() = default;
        operator LPCSTR() { return m_str? m_ansiArray.data() : nullptr; }
        LPCSTR c_str() { return m_str ? m_ansiArray.data() : nullptr; }
        int GetLength() const { return m_length; }

    private:
        CWtoA(const CWtoA&) = delete;
        CWtoA& operator= (const CWtoA&) = delete;
        std::vector<char> m_ansiArray;
        LPCWSTR m_str;
        int m_length;
    };

    ////////////////////////////////////////////////////////////////////
    // The CWtoBSTR class creates a BSTR from a wchar_t character array.
    //
    class CWtoBSTR
    {
    public:
        CWtoBSTR(LPCWSTR pWStr);
        ~CWtoBSTR() { ::SysFreeString(m_bstrString); }
        operator BSTR() { return m_bstrString;}
        int GetLength() const { return m_length; }

    private:
        CWtoBSTR(const CWtoBSTR&) = delete;
        CWtoBSTR& operator= (const CWtoBSTR&) = delete;
        BSTR m_bstrString;
        int m_length;
    };

    ///////////////////////////////////////
    // The CWtoW class doesn't modify text.
    // It is used by WtoT and TtoW.
    //
    class CWtoW
    {
    public:
        CWtoW(LPCWSTR pWStr, UINT codePage = CP_ACP, int charCount = -1);
        operator LPCWSTR() { return m_str; }
        operator LPOLESTR() { return const_cast<LPOLESTR>(m_str); }
        LPCWSTR c_str() const { return m_str; }
        int GetLength() const { return m_length; }

    private:
        CWtoW(const CWtoW&) = delete;
        CWtoW& operator= (const CWtoW&) = delete;

        LPCWSTR m_str;
        int m_length;
    };

}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

namespace Win32xx
{
    inline CAtoA::CAtoA(LPCSTR str, UINT /*codePage = CP_ACP*/, int charCount) : m_str(str)
    {
        if (str == nullptr)
            m_length = 0;
        else
            m_length = (charCount == -1) ? static_cast<int>(std::strlen(str)) : charCount;
    }

    inline CAtoBSTR::CAtoBSTR(LPCSTR str) : m_bstrString(nullptr), m_length(0)
    {
        if (str != nullptr)
        {
            CAtoW wideConv(str);

            // Create the BSTR from the wide character array.
            m_bstrString = ::SysAllocString(wideConv.c_str());
            if (m_bstrString != nullptr)
                m_length = static_cast<int>(::SysStringLen(m_bstrString));
        }
    }

    inline CAtoW::CAtoW(LPCSTR str, UINT codePage /*= CP_ACP*/, int charCount /*= -1*/)
        : m_str(str)
    {
        if (str == nullptr)
        {
            m_length = 0;
            m_wideArray.assign(1, L'\0');
            return;
        }

        // Fill our vector with the converted wchar_t array.
        int length = MultiByteToWideChar(codePage, 0, str, charCount, nullptr, 0);
        m_wideArray.assign(static_cast<size_t>(length) + 1, L'\0');
        MultiByteToWideChar(codePage, 0, str, charCount, m_wideArray.data(), length);

        if (charCount == -1)
            m_length = (length > 0) ? (length - 1) : 0;
        else
            m_length = length;
    }

    inline CWtoA::CWtoA(LPCWSTR str, UINT codePage /*= CP_ACP*/, int charCount /*= -1*/)
        : m_str(str)
    {
        if (str == nullptr)
        {
            m_length = 0;
            m_ansiArray.assign(1, '\0');
            return;
        }

        // Fill our vector with the converted char array.
        int length = WideCharToMultiByte(codePage, 0, str, charCount, nullptr,
            0, nullptr, nullptr);
        m_ansiArray.assign(static_cast<size_t>(length) + 1, '\0');
        WideCharToMultiByte(codePage, 0, str, charCount, m_ansiArray.data(),
            length, nullptr, nullptr);

        if (charCount == -1)
            m_length = (length > 0) ? (length - 1) : 0;
        else
            m_length = length;
    }

    inline CWtoBSTR::CWtoBSTR(LPCWSTR pWStr) : m_bstrString(nullptr), m_length(0)
    {
        if (pWStr != nullptr)
        {
            // Create the BSTR from the wide character array.
            m_bstrString = ::SysAllocString(pWStr);
            if (m_bstrString != nullptr)
                m_length = static_cast<int>(::SysStringLen(m_bstrString));
        }
    }

    inline CWtoW::CWtoW(LPCWSTR str, UINT /*codePage = CP_ACP*/,
        int charCount /*= -1*/) : m_str(str)
    {
        if (str == nullptr)
            m_length = 0;
        else
            m_length = (charCount == -1) ? static_cast<int>(std::wcslen(str)) : charCount;
    }


    ////////////////////
    // Global Functions.
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

#endif // WIN32XX_TEXTCONV_H_
