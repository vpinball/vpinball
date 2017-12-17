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


// Acknowledgements:
// Thanks to Adam Szulc for his initial CString code.

////////////////////////////////////////////////////////
// wxx_cstring.h
//  Declaration of the CString class

// This class is intended to provide a simple alternative to the MFC/ATL
// CString class that ships with Microsoft compilers. The CString class
// specified here is compatible with other compilers such as Borland 5.5
// and MinGW.

// Differences between this class and the MFC/ATL CString class
// ------------------------------------------------------------
// 1) The constructors for this class accepts both ANSI and Unicode characters and
//    automatically converts these to TCHAR as required.
//
// 2) This class is not reference counted, so these CStrings should be passed as
//    references or const references when used as function arguments. As a result there
//    is no need for functions like LockBuffer and UnLockBuffer.
//
// 3) The Format functions only accepts POD (Plain Old Data) arguments. It does not
//    accept arguments which are class or struct objects. In particular it does not
//    accept CString objects, unless these are cast to LPCTSTR.
//    This is demonstrates valid and invalid usage:
//      CString string1(_T("Hello World"));
//      CString string2;
//
//      // This is invalid, and produces undefined behaviour.
//      string2.Format(_T("String1 is: %s"), string1); // No! you can't do this
//
//      // This is ok
//      string2.Format(_T("String1 is: %s"), (LPCTSTR)string1); // Yes, this is correct
//
//      // This is better. It doesn't use casting
//      string2.Format(_T("String1 is: %s"), string1.c_str());  // This is correct too
//
//    Note: The MFC/ATL CString class uses a non portable hack to make its CString class
//          behave like a POD. Other compilers (such as the MinGW compiler) specifically
//          prohibit the use of non POD types for functions with variable argument lists.
//
// 4) This class provides a few additional functions:
//       c_str          Returns a const TCHAR string. This is an alternative for casting to LPCTSTR.
//       GetErrorString Assigns CString to the error string for the specified System Error Code
//                      (from ::GetLastErrror() for example).
//       GetString      Returns a reference to the underlying std::basic_string<TCHAR>. This
//                      reference can be used to modify the string directly.



#ifndef _WIN32XX_CSTRING_H_
#define _WIN32XX_CSTRING_H_


#include <assert.h>
#include <tchar.h>
#include <stdio.h>
#include <string>
#include <algorithm>
#include <windows.h>
#include "wxx_textconv.h"


namespace Win32xx
{

    // Defines the maximum size for TCHAR strings
    enum Constants
    {
        MAX_MENU_STRING = 80,
        MAX_STRING_SIZE = 255
    };

    /////////////////////////////////////////////
    // Declaration of the CStringT class template
    //
    template <class T>
    class CStringT
    {
        // Friend functions allow the left hand side to be something other than CStringT

        // These specialized friend declarations are compatible with all supported compilers
        friend CStringT<CHAR> operator + (const CStringT<CHAR>& string1, const CStringT<CHAR>& string2);
        friend CStringT<CHAR> operator + (const CStringT<CHAR>& string1, const CHAR* pszText);
        friend CStringT<CHAR> operator + (const CStringT<CHAR>& string1, CHAR ch);
        friend CStringT<CHAR> operator + (const CStringT<CHAR>& string1, int val);
        friend CStringT<CHAR> operator + (const CStringT<CHAR>& string1, double val);
        friend CStringT<CHAR> operator + (const CHAR* pszText, const CStringT<CHAR>& string1);
        friend CStringT<CHAR> operator + (CHAR ch, const CStringT<CHAR>& string1);
        friend CStringT<CHAR> operator + (int val, const CStringT<CHAR>& string1);
        friend CStringT<CHAR> operator + (double val, const CStringT<CHAR>& string1);

        friend CStringT<WCHAR> operator + (const CStringT<WCHAR>& string1, const CStringT<WCHAR>& string2);
        friend CStringT<WCHAR> operator + (const CStringT<WCHAR>& string1, const WCHAR* pszText);
        friend CStringT<WCHAR> operator + (const CStringT<WCHAR>& string1, WCHAR ch);
        friend CStringT<WCHAR> operator + (const CStringT<WCHAR>& string1, int val);
        friend CStringT<WCHAR> operator + (const CStringT<WCHAR>& string1, double val);
        friend CStringT<WCHAR> operator + (const WCHAR* pszText, const CStringT<WCHAR>& string1);
        friend CStringT<WCHAR> operator + (WCHAR ch, const CStringT<WCHAR>& string1);
        friend CStringT<WCHAR> operator + (int val, const CStringT<WCHAR>& string1);
        friend CStringT<WCHAR> operator + (double val, const CStringT<WCHAR>& string1);

        // These global functions don't need to be friends
    //  bool operator < (const CStringT<T>& string1, const CStringT<T>& string2);
    //  bool operator > (const CStringT<T>& string1, const CStringT<T>& string2);
    //  bool operator <= (const CStringT<T>& string1, const CStringT<T>& string2);
    //  bool operator >= (const CStringT<T>& string1, const CStringT<T>& string2);
    //  bool operator < (const CStringT<T>& string1, const T* pszText);
    //  bool operator > (const CStringT<T>& string1, const T* pszText);
    //  bool operator <= (const CStringT<T>& string1, const T* pszText);
    //  bool operator >= (const CStringT<T>& string1, const T* pszText);

        public:
        CStringT();
        virtual ~CStringT();
        CStringT(const CStringT& str);
        CStringT(const T * pszText);
        CStringT(T ch, int nLength = 1);
        CStringT(const T * pszText, int nLength);
        CStringT(int val);
        CStringT(double val);

        CStringT& operator = (const CStringT& str);
        CStringT& operator = (const T ch);
        CStringT& operator = (const T* pszText);
        CStringT& operator = (int val);
        CStringT& operator = (double val);

        bool     operator == (const T* pszText) const;
        bool     operator == (const CStringT& str) const;
        bool     operator != (const T* pszText) const;
        bool     operator != (const CStringT& str) const;
                 operator const T*() const;
        T&       operator [] (int nIndex);
        const T& operator [] (int nIndex) const;
        CStringT& operator += (const CStringT& str);
        CStringT& operator += (const T* szText);
        CStringT& operator += (const T ch);
        CStringT& operator += (int val);
        CStringT& operator += (double val);

        // Attributes
        const T* c_str() const          { return m_str.c_str(); }                   // alternative for casting to const T*
        virtual  std::basic_string<T> GetString() const { return m_str; }           // returns copy of the std::basic_string<T>
        int      GetLength() const  { return static_cast<int>(m_str.length()); }    // returns the length in characters

        // Operations
        BSTR     AllocSysString() const;
        void     AppendFormat(const T* pszFormat,...);
        void     AppendFormat(UINT nFormatID, ...);
        void     Assign(const T* pszText, int nCount);
        int      Collate(const T* pszText) const;
        int      CollateNoCase(const T* pszText) const;
        int      Compare(const T* pszText) const;
        int      CompareNoCase(const T* pszText) const;
        int      Delete(int nIndex, int nCount = 1);
        int      Find(T ch, int nIndex = 0 ) const;
        int      Find(const T* pszText, int nStart = 0) const;
        int      FindOneOf(const T* pszText) const;
        void     Format(UINT nID, ...);
        void     Format(const T* pszFormat,...);
        void     FormatV(const T* pszFormat, va_list args);
        void     FormatMessage(const T* pszFormat,...);
        void     FormatMessageV(const T* pszFormat, va_list args);
        T        GetAt(int nIndex) const;
        T*       GetBuffer(int nMinBufLength);
        void     GetErrorString(DWORD dwError);
        void     GetWindowText(HWND hWnd);
        void     Empty();
        int      Insert(int nIndex, T ch);
        int      Insert(int nIndex, const CStringT& str);
        bool     IsEmpty() const;
        CStringT Left(int nCount) const;
        bool     LoadString(UINT nID);      // defined in wincore.h
        void     MakeLower();
        void     MakeReverse();
        void     MakeUpper();
        CStringT Mid(int nFirst) const;
        CStringT Mid(int nFirst, int nCount) const;
        void     ReleaseBuffer( int nNewLength = -1 );
        int      Remove(const T* pszText);
        int      Replace(T chOld, T chNew);
        int      Replace(const T* pszOld, const T* pszNew);
        int      ReverseFind(const T* pszText, int nStart = -1) const;
        CStringT Right(int nCount) const;
        void     SetAt(int nIndex, T ch);
        BSTR     SetSysString(BSTR* pBstr) const;
        CStringT SpanExcluding(const T* pszText) const;
        CStringT SpanIncluding(const T* pszText) const;
        CStringT Tokenize(const T* pszTokens, int& iStart) const;
        void     Trim();
        void     TrimLeft();
        void     TrimLeft(T chTarget);
        void     TrimLeft(const T* pszTargets);
        void     TrimRight();
        void     TrimRight(T chTarget);
        void     TrimRight(const T* pszTargets);
        void     Truncate(int nNewLength);

#ifndef _WIN32_WCE
        bool     GetEnvironmentVariable(const T* pszVar);
#endif

    protected:
        std::basic_string<T> m_str;
        std::vector<T> m_buf;

    private:
        int     lstrlenT(const CHAR* pszText)   { return lstrlenA(pszText); }
        int     lstrlenT(const WCHAR* pszText)  { return lstrlenW(pszText); }
    };

    // CStringA is a char only version of CString
    typedef CStringT<CHAR> CStringA;

    // CStringW is a WCHAR only version of CString
    typedef CStringT<WCHAR> CStringW;

    ///////////////////////////////////
    // A CString object provides a safer and a more convenient alternative to an
    // array of char or WCHAR when working with text strings.
    class CString : public CStringT<TCHAR>
    {
        friend CString operator + (const CString& string1, const CString& string2);
        friend CString operator + (const CString& string1, const TCHAR* pszText);
        friend CString operator + (const CString& string1, TCHAR ch);
        friend CString operator + (const CString& string1, int val);
        friend CString operator + (const CString& string1, double val);
        friend CString operator + (const TCHAR* pszText, const CString& string1);
        friend CString operator + (TCHAR ch, const CString& string1);
        friend CString operator + (int val, const CString& string1);
        friend CString operator + (double val, const CString& string1);

    public:
        CString() {}
        CString(const CString& str)             : CStringT<TCHAR>(str) {}
        CString(LPCSTR pszText)                 : CStringT<TCHAR>(AtoT(pszText)) {}
        CString(LPCWSTR pszText)                : CStringT<TCHAR>(WtoT(pszText))    {}
        CString(LPCSTR pszText, int nLength)    : CStringT<TCHAR>(AtoT(pszText), nLength) {}
        CString(LPCWSTR pszText, int nLength)   : CStringT<TCHAR>(WtoT(pszText), nLength) {}
        CString(int val)                        : CStringT<TCHAR>(val) {}
        CString(double val)                     : CStringT<TCHAR>(val) {}

        CString(char ch, int nLength = 1)
        {
            char str[2] = {0};
            str[0] = ch;
            AtoT tch(str);
            m_str.assign(nLength, static_cast<LPCTSTR>(tch)[0]);
        }

        CString(WCHAR ch, int nLength = 1)
        {
            WCHAR str[2] = {0};
            str[0] = ch;
            WtoT tch(str);
            m_str.assign(nLength, static_cast<LPCTSTR>(tch)[0]);
        }

        CString& operator = (const CString& str)
        {
            m_str.assign(str.GetString());
            return *this;
        }

        CString& operator = (const CStringT<TCHAR>& str)
        {
            m_str.assign(str.GetString());
            return *this;
        }

        CString& operator = (const char ch)
        {
            char str[2] = {0};
            str[0] = ch;
            AtoT tch(str);
            m_str.assign(1, static_cast<LPCTSTR>(tch)[0]);
            return *this;
        }

        CString& operator = (const WCHAR ch)
        {
            WCHAR str[2] = {0};
            str[0] = ch;
            WtoT tch(str);
            m_str.assign(1, static_cast<LPCTSTR>(tch)[0]);
            return *this;
        }

        CString& operator = (LPCSTR pszText)
        {
            m_str.assign(AtoT(pszText));
            return *this;
        }

        CString& operator = (LPCWSTR pszText)
        {
            m_str.assign(WtoT(pszText));
            return *this;
        }

        CString& operator += (LPCSTR szText)
        {
            m_str.append(AtoT(szText));
            return *this;
        }

        CString& operator += (LPCWSTR szText)
        {
            m_str.append(WtoT(szText));
            return *this;
        }

        CString& operator += (const char ch)
        {
            char str[2] = {0};
            str[0] = ch;
            AtoT tch(str);
            m_str.append(1, static_cast<LPCTSTR>(tch)[0]);
            return *this;
        }

        CString& operator += (const WCHAR ch)
        {
            WCHAR str[2] = {0};
            str[0] = ch;
            WtoT tch(str);
            m_str.append(1, static_cast<LPCTSTR>(tch)[0]);
            return *this;
        }

        CString& operator += (int val)
        {
            CString str;
            str.Format(_T("%d"), val);
            m_str.append(str);
            return *this;
        }

        CString& operator += (double val)
        {
            CString str;
            str.Format(_T("%g"), val);
            m_str.append(str);
            return *this;
        }

        CString Left(int nCount) const
        {
            CString str;
            str = CStringT<TCHAR>::Left(nCount);
            return str;
        }

        CString Mid(int nFirst) const
        {
            CString str;
            str = CStringT<TCHAR>::Mid(nFirst);
            return str;
        }

        CString Mid(int nFirst, int nCount) const
        {
            CString str;
            str = CStringT<TCHAR>::Mid(nFirst, nCount);
            return str;
        }

        CString Right(int nCount) const
        {
            CString str;
            str = CStringT<TCHAR>::Right(nCount);
            return str;
        }

        CString SpanExcluding(const TCHAR* pszText) const
        {
            CString str;
            str = CStringT<TCHAR>::SpanExcluding(pszText);
            return str;
        }

        CString SpanIncluding(const TCHAR* pszText) const
        {
            CString str;
            str = CStringT<TCHAR>::SpanIncluding(pszText);
            return str;
        }

        CString Tokenize(const TCHAR* pszTokens, int& iStart) const
        {
            CString str;
            str = CStringT<TCHAR>::Tokenize(pszTokens, iStart);
            return str;
        }

    };

} // namespace Win32xx


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


namespace Win32xx
{

    /////////////////////////////////////////////
    // Definition of the CStringT class template
    //

    // Constructor.
    template <class T>
    inline CStringT<T>::CStringT()
    {
    }


    // Destructor.
    template <class T>
    inline CStringT<T>::~CStringT()
    {
    }


    // Constructor. Assigns from a CStringT<T>.
    template <class T>
    inline CStringT<T>::CStringT(const CStringT& str)
    {
        m_str.assign(str.m_str);
    }


    // Constructor. Assigns from from a const T* character array.
    template <class T>
    inline CStringT<T>::CStringT(const T* pszText)
    {
        m_str.assign(pszText);
    }


    // Constructor. Assigns from 1 or more T characters.
    template <class T>
    inline CStringT<T>::CStringT(T ch, int nLength)
    {
        T str[2] = {0};
        str[0] = ch;
        m_str.assign(nLength, str[0]);
    }


    // Constructor. Assigns from a const T* possibly containing null characters.
    // Ensure the size of the text buffer holds nLength or more characters
    template <class T>
    inline CStringT<T>::CStringT(const T* pszText, int nLength)
    {
        memcpy(GetBuffer(nLength), pszText, nLength*sizeof(T));
        ReleaseBuffer(nLength);
    }


    // Constructor. Assigns from an integer value.
    template <class T>
    inline CStringT<T>::CStringT(int val)
    {
        Format(_T("%d"), val);
    }


    // Constructor. Assigns from a double value.
    template <class T>
    inline CStringT<T>::CStringT(double val)
    {
        Format(_T("%g"), val);
    }


    // Assign from a const CStringT<T>.
    template <class T>
    inline CStringT<T>& CStringT<T>::operator = (const CStringT<T>& str)
    {
        m_str.assign(str.m_str);
        return *this;
    }


    // Assign from a T character.
    template <class T>
    inline CStringT<T>& CStringT<T>::operator = (T ch)
    {
        T str[2] = {0};
        str[0] = ch;
        m_str.assign(1, str[0]);
        return *this;
    }


    // Assign from a const T* character array.
    template <class T>
    inline CStringT<T>& CStringT<T>::operator = (const T* pszText)
    {
        m_str.assign(pszText);
        return *this;
    }


    // Assign from an int converted to CHAR array.
    template <>
    inline CStringT<CHAR>& CStringT<CHAR>::operator = (int val)
    {
        CStringT str;
        str.Format("%d", val);
        m_str.assign(str.m_str);
        return *this;
    }


    // Assign from an int converted to a WCHAR array.
    template <>
    inline CStringT<WCHAR>& CStringT<WCHAR>::operator = (int val)
    {
        CStringT str;
        str.Format(L"%d", val);
        m_str.assign(str.m_str);
        return *this;
    }


    // Assign from a double converted to a CHAR array.
    template <>
    inline CStringT<CHAR>& CStringT<CHAR>::operator = (double val)
    {
        CStringT str;
        str.Format("%g", val);
        m_str.assign(str.m_str);
        return *this;
    }


    // Assign from a double converted to a WCHAR array.
    template <>
    inline CStringT<WCHAR>& CStringT<WCHAR>::operator = (double val)
    {
        CStringT str;
        str.Format(L"%g", val);
        m_str.assign(str.m_str);
        return *this;
    }


    // Returns TRUE if the strings have the same content.
    template <class T>
    inline bool CStringT<T>::operator == (const T* pszText) const
    {
        assert(pszText);
        return (0 == Compare(pszText));
    }


    // Returns TRUE if the strings have the same content.
    // Can compare CStringTs containing null characters.
    template <class T>
    inline bool CStringT<T>::operator == (const CStringT& str) const
    {
        return m_str == str.m_str;
    }


    // Returns TRUE if the strings have a different content.
    template <class T>
    inline bool CStringT<T>::operator != (const T* pszText) const
    {
        assert(pszText);
        return Compare(pszText) != 0;
    }


    // Returns TRUE if the strings have a different content.
    // Can compares CStringTs containing null characters.
    template <class T>
    inline bool CStringT<T>::operator != (const CStringT& str) const
    {
        return m_str != str.m_str;
    }


    // Function call operator. Returns a const T* character array.
    template <class T>
    inline CStringT<T>::operator const T*() const
    {
        return m_str.c_str();
    }


    // Subscript operator. Returns the T character at the specified index.
    template <class T>
    inline T& CStringT<T>::operator [] (int nIndex)
    {
        assert(nIndex >= 0);
        assert(nIndex < GetLength());   
        return m_str[nIndex];
    }


    // Subscript operator. Returns the T character at the specified index.
    template <class T>
    inline const T& CStringT<T>::operator [] (int nIndex) const
    {
        assert(nIndex >= 0);
        assert(nIndex < GetLength());
        return m_str[nIndex];
    }


    // Addition assignment. Appends CStringT<T>.
    template <class T>
    inline CStringT<T>& CStringT<T>::operator += (const CStringT& str)
    {
        m_str.append(str.m_str);
        return *this;
    }


    // Addition assignment. Appends const T* character array.
    template <class T>
    inline CStringT<T>& CStringT<T>::operator += (const T* szText)
    {
        m_str.append(szText);
        return *this;
    }


    // Addition assignment. Appends a T character.
    template <class T>
    inline CStringT<T>& CStringT<T>::operator += (T ch)
    {
        T str[2] = {0};
        str[0] = ch;
        m_str.append(1, str[0]);
        return *this;
    }


    // Addition assignment. Appends an int converted to a CHAR character array.
    template <>
    inline CStringT<CHAR>& CStringT<CHAR>::operator += (int val)
    {
        CStringT str;
        str.Format("%d", val);
        m_str.append(str);
        return *this;
    }


    // Addition assignment. Appends an int converted to a WCHAR character array.
    template <>
    inline CStringT<WCHAR>& CStringT<WCHAR>::operator += (int val)
    {
        CStringT str;
        str.Format(L"%d", val);
        m_str.append(str);
        return *this;
    }


    // Addition assignment. Appends a double converted to a CHAR character array.
    template <>
    inline CStringT<CHAR>& CStringT<CHAR>::operator += (double val)
    {
        CStringT str;
        str.Format("%g", val);
        m_str.append(str);
        return *this;
    }


    // Addition assignment. Appends a double converted to a WCHAR character array.
    template <>
    inline CStringT<WCHAR>& CStringT<WCHAR>::operator += (double val)
    {
        CStringT str;
        str.Format(L"%g", val);
        m_str.append(str);
        return *this;
    }


    // Allocates a BSTR from the CStringT content.
    // Note: Ensure the returned BSTR is freed later with SysFreeString to avoid a memory leak.
    template <>
    inline BSTR CStringT<CHAR>::AllocSysString() const
    {
        BSTR bstr = ::SysAllocStringLen(AtoW(m_str.c_str()), static_cast<UINT>(m_str.size()));
        if (bstr == NULL)
            throw std::bad_alloc();

        return bstr;
    }


    // Allocates a BSTR from the CStringT content.
    // Note: Free the returned string later with SysFreeString to avoid a memory leak.
    template <>
    inline BSTR CStringT<WCHAR>::AllocSysString() const
    {
        BSTR bstr = ::SysAllocStringLen(m_str.c_str(), static_cast<UINT>(m_str.size()));
        if (bstr == NULL)
            throw std::bad_alloc();

        return bstr;
    }


    // Appends formatted data to an the CStringT content.
    template <class T>
    inline void CStringT<T>::AppendFormat(const T* pszFormat,...)
    {
        CStringT str;

        va_list args;
        va_start(args, pszFormat);
        str.FormatV(pszFormat, args);
        va_end(args);

        m_str.append(str);
    }


    // Assigns the specified number of charaters from pszText to the CStringT.
    template <class T>
    inline void CStringT<T>::Assign(const T* pszText, int nCount)
    {
        m_str.assign(pszText, nCount);
    }


    // Performs a case sensitive comparison of the two strings using locale-specific information.
    template <>
    inline int CStringT<CHAR>::Collate(const CHAR* pszText) const
    {
        assert(pszText);
        int res = CompareStringA(LOCALE_USER_DEFAULT, 0, m_str.c_str(), -1, pszText, -1);

        assert(res);
        if      (res == CSTR_LESS_THAN) return -1;
        else if (res == CSTR_GREATER_THAN) return 1;

        return 0;
    }


    // Performs a case sensitive comparison of the two strings using locale-specific information.
    template <>
    inline int CStringT<WCHAR>::Collate(const WCHAR* pszText) const
    {
        assert(pszText);
        int res = CompareStringW(LOCALE_USER_DEFAULT, 0, m_str.c_str(), -1, pszText, -1);

        assert(res);
        if      (res == CSTR_LESS_THAN) return -1;
        else if (res == CSTR_GREATER_THAN) return 1;

        return 0;
    }


    // Performs a case insensitive comparison of the two strings using locale-specific information.
    template <>
    inline int CStringT<CHAR>::CollateNoCase(const CHAR* pszText) const
    {
        assert(pszText);
        int res = CompareStringA(LOCALE_USER_DEFAULT, NORM_IGNORECASE, m_str.c_str(), -1, pszText, -1);

        assert(res);
        if      (res == CSTR_LESS_THAN) return -1;
        else if (res == CSTR_GREATER_THAN) return 1;

        return 0;
    }


    // Performs a case insensitive comparison of the two strings using locale-specific information.
    template <>
    inline int CStringT<WCHAR>::CollateNoCase(const WCHAR* pszText) const
    {
        assert(pszText);
        int res = CompareStringW(LOCALE_USER_DEFAULT, NORM_IGNORECASE, m_str.c_str(), -1, pszText, -1);

        assert(res);
        if      (res == CSTR_LESS_THAN) return -1;
        else if (res == CSTR_GREATER_THAN) return 1;

        return 0;
    }


    // Performs a case sensitive comparison of the two strings.
    template <>
    inline int CStringT<CHAR>::Compare(const CHAR* pszText) const
    {
        assert(pszText);
        return lstrcmpA(m_str.c_str(), pszText);
    }


    // Performs a case sensitive comparison of the two strings.
    template <>
    inline int CStringT<WCHAR>::Compare(const WCHAR* pszText) const
    {
        assert(pszText);
        return lstrcmpW(m_str.c_str(), pszText);
    }


    // Performs a case insensitive comparison of the two strings.
    template <>
    inline int CStringT<CHAR>::CompareNoCase(const CHAR* pszText) const
    {
        assert(pszText);
        return lstrcmpiA(m_str.c_str(), pszText);
    }


    // Performs a case insensitive comparison of the two strings.
    template <>
    inline int CStringT<WCHAR>::CompareNoCase(const WCHAR* pszText) const
    {
        assert(pszText);
        return lstrcmpiW(m_str.c_str(), pszText);
    }


    // Deletes a character or characters from the string.
    template <class T>
    inline int CStringT<T>::Delete(int nIndex, int nCount /* = 1 */)
    {
        assert(nIndex >= 0);
        assert(nCount >= 0);

        if (nIndex < GetLength())
            m_str.erase(nIndex, nCount);
        
        return static_cast<int>(m_str.size());
    }


    // Erases the contents of the string.
    template <class T>
    inline void CStringT<T>::Empty()
    {
        m_str.erase();
    }


    // Finds a character in the string.
    template <class T>
    inline int CStringT<T>::Find(T ch, int nIndex /* = 0 */) const
    {
        assert(nIndex >= 0);
        size_t s = m_str.find(ch, nIndex);

        if (s == std::string::npos) return -1;
        return static_cast<int>(s);
    }


    // Finds a substring within the string.
    template <class T>
    inline int CStringT<T>::Find(const T* pszText, int nIndex /* = 0 */) const
    {
        assert(pszText);
        assert(nIndex >= 0);
        size_t s = m_str.find(pszText, nIndex);

        if (s == std::string::npos) return -1;
        return static_cast<int>(s);
    }


    // Finds the first matching character from a set.
    template <class T>
    inline int CStringT<T>::FindOneOf(const T* pszText) const
    {
        assert(pszText);
        size_t s = m_str.find_first_of(pszText);

        if (s == std::string::npos) return -1;
        return static_cast<int>(s);
    }


    // Formats the string as sprintf does.
    template <class T>
    inline void CStringT<T>::Format(const T* pszFormat,...)
    {
        va_list args;
        va_start(args, pszFormat);
        FormatV(pszFormat, args);
        va_end(args);
    }


    // Formats the string using a variable list of arguments.
    template <>
    inline void CStringT<CHAR>::FormatV(const CHAR*  pszFormat, va_list args)
    {

        if (pszFormat)
        {
            int nResult = -1;
            int nLength = 256;

            // A vector is used to store the CHAR array
            std::vector<CHAR> vBuffer;

            while (-1 == nResult)
            {
                vBuffer.assign( nLength+1, 0 );
                nResult = _vsnprintf(&vBuffer[0], nLength, pszFormat, args);
                nLength *= 2;
            }
            m_str.assign(&vBuffer[0]);
        }
    }


    // Formats the string using a variable list of arguments.
    template <>
    inline void CStringT<WCHAR>::FormatV(const WCHAR*  pszFormat, va_list args)
    {

        if (pszFormat)
        {
            int nResult = -1;
            int nLength = 256;

            // A vector is used to store the WCHAR array
            std::vector<WCHAR> vBuffer;

            while (-1 == nResult)
            {
                vBuffer.assign( nLength+1, 0 );
                nResult = _vsnwprintf(&vBuffer[0], nLength, pszFormat, args);
                nLength *= 2;
            }
            m_str.assign(&vBuffer[0]);
        }
    }


    // Formats a message string.
    template <class T>
    inline void CStringT<T>::FormatMessage(const T* pszFormat,...)
    {
        va_list args;
        va_start(args, pszFormat);
        FormatMessageV(pszFormat, args);
        va_end(args);
    }


    // Formats a message string using a variable argument list.
    template <>
    inline void CStringT<CHAR>::FormatMessageV(const CHAR* pszFormat, va_list args)
    {
        LPSTR pszTemp = 0;
        if (pszFormat)
        {
            DWORD dwResult = ::FormatMessageA(FORMAT_MESSAGE_FROM_STRING|FORMAT_MESSAGE_ALLOCATE_BUFFER,
                                   pszFormat, 0, 0, reinterpret_cast<LPSTR>(&pszTemp), 0, &args);

            if ( dwResult == 0 || pszTemp == 0 )
                throw std::bad_alloc();

            m_str = pszTemp;
            LocalFree(pszTemp);
        }
    }


    // Formats a message string using a variable argument list.
    template <>
    inline void CStringT<WCHAR>::FormatMessageV(const WCHAR* pszFormat, va_list args)
    {
        LPWSTR pszTemp = 0;
        if (pszFormat)
        {
            DWORD dwResult = ::FormatMessageW(FORMAT_MESSAGE_FROM_STRING|FORMAT_MESSAGE_ALLOCATE_BUFFER,
                                  pszFormat, 0, 0, (LPWSTR)&pszTemp, 0, &args);

            if ( dwResult == 0 || pszTemp == 0 )
                throw std::bad_alloc();

            m_str = pszTemp;
            LocalFree(pszTemp);
        }
    }


    // Returns the character at the specified location within the string.
    template <class T>
    inline T CStringT<T>::GetAt(int nIndex) const
    {
        assert(nIndex >= 0);
        assert(nIndex < GetLength());
        T ch = 0;

        if ((nIndex >= 0) && (nIndex < GetLength()))
            ch = m_str[nIndex];

        return ch;
    }


    // Creates a buffer of nMinBufLength characters (+1 extra for NULL termination) and returns
    // a pointer to this buffer. This buffer can be used by any function which accepts a LPTSTR.
    // Care must be taken not to exceed the length of the buffer. Use ReleaseBuffer to safely
    // copy this buffer back to the CStringT object.
    // Note: The buffer uses a vector. Vectors are required to be contiguous in memory under
    //       the current standard, whereas std::strings do not have this requirement.
    template <class T>
    inline T* CStringT<T>::GetBuffer(int nMinBufLength)
    {
        assert (nMinBufLength >= 0);

        T ch = 0;
        m_buf.assign(nMinBufLength + 1, ch);
        typename std::basic_string<T>::iterator it_end;

        if (m_str.length() >= (size_t)nMinBufLength)
        {
            it_end = m_str.begin();
            std::advance(it_end, nMinBufLength);
        }
        else
            it_end = m_str.end();

        std::copy(m_str.begin(), it_end, m_buf.begin());

        return &m_buf[0];
    }

#ifndef _WIN32_WCE

    // Sets the string to the value of the specified environment variable.
    template <>
    inline bool CStringT<CHAR>::GetEnvironmentVariable(const CHAR* pszVar)
    {
        assert(pszVar);
        Empty();

        int nLength = ::GetEnvironmentVariableA(pszVar, NULL, 0);
        if (nLength > 0)
        {
            std::vector<CHAR> vBuffer( nLength+1, 0 );
            ::GetEnvironmentVariableA(pszVar, &vBuffer[0], nLength);
            m_str = &vBuffer[0];
        }

        return (nLength != 0);
    }


    // Sets the string to the value of the specified environment variable.
    template <>
    inline bool CStringT<WCHAR>::GetEnvironmentVariable(const WCHAR* pszVar)
    {
        assert(pszVar);
        Empty();

        int nLength = ::GetEnvironmentVariableW(pszVar, NULL, 0);
        if (nLength > 0)
        {
            std::vector<WCHAR> vBuffer( nLength+1, 0 );
            ::GetEnvironmentVariableW(pszVar, &vBuffer[0], nLength);
            m_str = &vBuffer[0];
        }

        return (nLength != 0);
    }

#endif // _WIN32_WCE


    // Retrieves the a window's text.
    template <>
    inline void CStringT<CHAR>::GetWindowText(HWND hWndCtrl)
    {
        Empty();
        int nLength = ::GetWindowTextLengthA(hWndCtrl);
        if (nLength > 0)
        {
            std::vector<CHAR> vBuffer( nLength+1, 0 );
            ::GetWindowTextA(hWndCtrl, &vBuffer[0], nLength+1);
            m_str = &vBuffer[0];
        }
    }


    // Retrieves a window's text.
    template <>
    inline void CStringT<WCHAR>::GetWindowText(HWND hWndCtrl)
    {
        Empty();
        int nLength = ::GetWindowTextLengthW(hWndCtrl);
        if (nLength > 0)
        {
            std::vector<WCHAR> vBuffer( nLength+1, 0 );
            ::GetWindowTextW(hWndCtrl, &vBuffer[0], nLength+1);
            m_str = &vBuffer[0];
        }
    }


    // Returns the error string for the specified System Error Code (e.g from GetLastErrror).
    template <>
    inline void CStringT<CHAR>::GetErrorString(DWORD dwError)
    {
        Empty();
        CHAR* pTemp = 0;
        DWORD dwFlags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;
        ::FormatMessageA(dwFlags, NULL, dwError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<LPSTR>(&pTemp), 1, NULL);
        m_str.assign(pTemp);
        ::LocalFree(pTemp);
    }


    // Returns the error string for the specified System Error Code (e.g from GetLastErrror).
    template <>
    inline void CStringT<WCHAR>::GetErrorString(DWORD dwError)
    {
        Empty();
        WCHAR* pTemp = 0;
        DWORD dwFlags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;
        ::FormatMessageW(dwFlags, NULL, dwError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&pTemp, 1, NULL);
        m_str.assign(pTemp);
        ::LocalFree(pTemp);
    }


    // Inserts a single character or a substring at the given index within the string.
    template <class T>
    inline int CStringT<T>::Insert(int nIndex, T ch)
    {
        assert(nIndex >= 0);
        assert(ch);

        nIndex = MIN(nIndex, GetLength());
        m_str.insert(nIndex, &ch, 1);
        
        return static_cast<int>(m_str.size());
    }


    // Inserts a single character or a substring at the given index within the string.
    template <class T>
    inline int CStringT<T>::Insert(int nIndex, const CStringT& str)
    {
        assert(nIndex >= 0);

        nIndex = MIN(nIndex, GetLength());
        m_str.insert(nIndex, str);
        
        return static_cast<int>(m_str.size());
    }


    // Returns TRUE if the string is empty
    template <class T>
    inline bool CStringT<T>::IsEmpty() const
    {
        return m_str.empty();
    }


    // Extracts the left part of a string.
    template <class T>
    inline CStringT<T> CStringT<T>::Left(int nCount) const
    {
        assert(nCount >= 0);

        CStringT str;
        str.m_str.assign(m_str, 0, nCount);
        return str;
    }


    // Converts all the characters in this string to lowercase characters.
    template <class T>
    inline void CStringT<T>::MakeLower()
    {
        std::transform(m_str.begin(), m_str.end(), m_str.begin(), &::tolower);
    }


    // Reverses the string.
    template <class T>
    inline void CStringT<T>::MakeReverse()
    {
        // Error 2285 with Borland 5.5 occurs here unless option -tWM is used instead of -tW
        std::reverse(m_str.begin(), m_str.end());
    }


    // Converts all the characters in this string to uppercase characters.
    template <class T>
    inline void CStringT<T>::MakeUpper()
    {
        // Error 2285 with Borland 5.5 occurs here unless option -tWM is used instead of -tW
        std::transform(m_str.begin(), m_str.end(), m_str.begin(), &::toupper);
    }


    // Extracts the middle part of a string.
    template <class T>
    inline CStringT<T> CStringT<T>::Mid(int nFirst) const
    {
        return Mid(nFirst, GetLength());
    }


    // Extracts the middle part of a string.
    template <class T>
    inline CStringT<T> CStringT<T>::Mid(int nFirst, int nCount) const
    {
        assert(nFirst >= 0);
        assert(nCount >= 0);

        CStringT str;
        if (nFirst <= GetLength())
            str.m_str.assign(m_str, nFirst, nCount);

        return str;
    }


    // Search for a substring within the string, starting from the end.
    template <class T>
    inline int CStringT<T>::ReverseFind(const T* pszText, int nIndex /* = -1 */) const
    {
        assert(pszText);
        return static_cast<int>(m_str.rfind(pszText, nIndex));
    }


    // Sets the character at the specified position to the specified value.
    template <class T>
    inline void CStringT<T>::SetAt(int nIndex, T ch)
    {
        assert(nIndex >= 0);
        assert(nIndex < GetLength());
        
        if ((nIndex >= 0) && (nIndex < GetLength()))
            m_str[nIndex] = ch;
    }


    // This copies the contents of the buffer (acquired by GetBuffer) to this CStringT.
    // The default length of -1 copies from the buffer until a null terminator is reached. 
    // If the buffer doesn't contain a null terminator, you must specify the buffer's length.
    template <class T>
    inline void CStringT<T>::ReleaseBuffer( int nNewLength /*= -1*/ )
    {
        assert (nNewLength <= static_cast<int>(m_buf.size()));

        if (-1 == nNewLength)
        {
            nNewLength = lstrlenT(&m_buf[0]);
        }

        T ch = 0;
        m_str.assign(nNewLength, ch);

        typename std::vector<T>::iterator it_end = m_buf.begin();
        std::advance(it_end, nNewLength);

        std::copy(m_buf.begin(), it_end, m_str.begin());
    }


    // Removes each occurrence of the specified substring from the string.
    template <class T>
    inline int CStringT<T>::Remove(const T* pszText)
    {
        assert(pszText);

        int nCount = 0;
        size_t pos = 0;
        size_t len = lstrlenT(pszText);
        while ((pos = m_str.find(pszText, pos)) != std::string::npos)
        {
            m_str.erase(pos, len);
            ++nCount;
        }
        return nCount;
    }


    // Replaces each occurrence of the old character with the new character.
    template <class T>
    inline int CStringT<T>::Replace(T chOld, T chNew)
    {
        int nCount = 0;
        typename std::basic_string<T>::iterator it;
        it = m_str.begin();
        while (it != m_str.end())
        {
            if (*it == chOld)
            {
                *it = chNew;
                ++nCount;
            }
            ++it;
        }
        return nCount;
    }


    // Replaces each occurrence of the old substring with the new substring.
    template <class T>
    inline int CStringT<T>::Replace(const T* pszOld, const T* pszNew)
    {
        assert(pszOld);
        assert(pszNew);

        int nCount = 0;
        size_t pos = 0;
        size_t lenOld = lstrlenT(pszOld);
        size_t lenNew = lstrlenT(pszNew);
        while ((pos = m_str.find(pszOld, pos)) != std::string::npos)
        {
            m_str.replace(pos, lenOld, pszNew);
            pos += lenNew;
            ++nCount;
        }
        return nCount;
    }


    // Extracts the right part of a string.
    template <class T>
    inline CStringT<T> CStringT<T>::Right(int nCount) const
    {
        assert(nCount >= 0);

        CStringT str;
        nCount = MIN(nCount, GetLength());
        str.m_str.assign(m_str, m_str.size() - nCount, nCount);
        return str;
    }


    // Sets an existing BSTR object to the string.
    // Note: Ensure the returned BSTR is freed later with SysFreeString to avoid a memory leak.
    template <>
    inline BSTR CStringT<CHAR>::SetSysString(BSTR* pBstr) const
    {
        assert(pBstr);

        if ( !::SysReAllocStringLen(pBstr, AtoW(m_str.c_str()), static_cast<UINT>(m_str.length())) )
            throw std::bad_alloc();

        return *pBstr;
    }


    // Sets an existing BSTR object to the string.
    template <>
    inline BSTR CStringT<WCHAR>::SetSysString(BSTR* pBstr) const
    {
        assert(pBstr);

        if ( !::SysReAllocStringLen(pBstr, m_str.c_str(), static_cast<UINT>(m_str.length())) )
            throw std::bad_alloc();

        return *pBstr;
    }


    // Extracts characters from the string, starting with the first character,
    // that are not in the set of characters identified by pszCharSet.
    template <class T>
    inline CStringT<T> CStringT<T>::SpanExcluding(const T* pszText) const
    {
        assert (pszText);

        CStringT str;
        size_t pos = 0;

        while ((pos = m_str.find_first_not_of(pszText, pos)) != std::string::npos)
        {
            str.m_str.append(1, m_str[pos++]);
        }

        return str;
    }


    // Extracts a substring that contains only the characters in a set.
    template <class T>
    inline CStringT<T> CStringT<T>::SpanIncluding(const T* pszText) const
    {
        assert (pszText);

        CStringT str;
        size_t pos = 0;

        while ((pos = m_str.find_first_of(pszText, pos)) != std::string::npos)
        {
            str.m_str.append(1, m_str[pos++]);
        }

        return str;
    }


    // Extracts specified tokens in a target string.
    template <class T>
    inline CStringT<T> CStringT<T>::Tokenize(const T* pszTokens, int& iStart) const
    {
        assert(pszTokens);

        CStringT str;
        if(iStart >= 0)
        {
        size_t pos1 = m_str.find_first_not_of(pszTokens, iStart);
        size_t pos2 = m_str.find_first_of(pszTokens, pos1);

        iStart = static_cast<int>(pos2) + 1;
        if (pos2 == m_str.npos)
            iStart = -1;

        if (pos1 != m_str.npos)
            str.m_str = m_str.substr(pos1, pos2-pos1);
        }
        return str;
    }


    // Trims all leading and trailing whitespace characters from the string.
    template <class T>
    inline void CStringT<T>::Trim()
    {
        TrimLeft();
        TrimRight();
    }


    // Trims leading whitespace characters from the string.
    template <class T>
    inline void CStringT<T>::TrimLeft()
    {
        // This method is supported by the Borland 5.5 compiler
        typename std::basic_string<T>::iterator iter;
        for (iter = m_str.begin(); iter != m_str.end(); ++iter)
        {
            if (!_istspace(*iter))
                break;
        }

        m_str.erase(m_str.begin(), iter);
    }


    // Trims the specified character from the beginning of the string.
    template <class T>
    inline void CStringT<T>::TrimLeft(T chTarget)
    {
        m_str.erase(0, m_str.find_first_not_of(chTarget));
    }


    // Trims the specified set of characters from the beginning of the string.
    template <class T>
    inline void CStringT<T>::TrimLeft(const T* pszTargets)
    {
        assert(pszTargets);
        m_str.erase(0, m_str.find_first_not_of(pszTargets));
    }


    // Trims trailing whitespace characters from the string.
    template <class T>
    inline void CStringT<T>::TrimRight()
    {
        // This method is supported by the Borland 5.5 compiler
        typename std::basic_string<T>::reverse_iterator riter;
        for (riter = m_str.rbegin(); riter < m_str.rend(); ++riter)
        {
            if (!_istspace(*riter))
                break;
        }

        m_str.erase(riter.base(), m_str.end());
    }


    // Trims the specified character from the end of the string.
    template <class T>
    inline void CStringT<T>::TrimRight(T chTarget)
    {
        size_t pos = m_str.find_last_not_of(chTarget);
        if (pos != std::string::npos)
            m_str.erase(++pos);
    }


    // Trims the specified set of characters from the end of the string.
    template <class T>
    inline void CStringT<T>::TrimRight(const T* pszTargets)
    {
        assert(pszTargets);

        size_t pos = m_str.find_last_not_of(pszTargets);
        if (pos != std::string::npos)
            m_str.erase(++pos);
    }


    // Reduces the length of the string to the specified amount.
    template <class T>
    inline void CStringT<T>::Truncate(int nNewLength)
    {
        if (nNewLength < GetLength())
        {
            assert(nNewLength >= 0);
            m_str.erase(nNewLength);
        }
    }


    //////////////////////////////////////
    // CStringT global operator functions
    //  These functions are declared as friends of CStringT.
    //

    // Addition operator.
    inline CStringT<CHAR> operator + (const CStringT<CHAR>& string1, const CStringT<CHAR>& string2)
    {
        CStringT<CHAR> str(string1);
        str.m_str.append(string2.m_str);
        return str;
    }


    // Addition operator.
    inline CStringT<WCHAR> operator + (const CStringT<WCHAR>& string1, const CStringT<WCHAR>& string2)
    {
        CStringT<WCHAR> str(string1);
        str.m_str.append(string2.m_str);
        return str;
    }


    // Addition operator.
    inline CStringT<CHAR> operator + (const CStringT<CHAR>& string1, const CHAR* pszText)
    {
        CStringT<CHAR> str(string1);
        str.m_str.append(pszText);
        return str;
    }


    // Addition operator.
    inline CStringT<WCHAR> operator + (const CStringT<WCHAR>& string1, const WCHAR* pszText)
    {
        CStringT<WCHAR> str(string1);
        str.m_str.append(pszText);
        return str;
    }


    // Addition operator.
    inline CStringT<CHAR> operator + (const CStringT<CHAR>& string1, CHAR ch)
    {
        CStringT<CHAR> str(string1);
        str.m_str.append(1, ch);
        return str;
    }


    // Addition operator.
    inline CStringT<WCHAR> operator + (const CStringT<WCHAR>& string1, WCHAR ch)
    {
        CStringT<WCHAR> str(string1);
        str.m_str.append(1, ch);
        return str;
    }


    // Addition operator.
    inline CStringT<CHAR> operator + (const CStringT<CHAR>& string1, int val)
    {
        CStringT<CHAR> str;
        str.Format("%s%a", string1.c_str(), val);
        return str;
    }


    // Addition operator.
    inline CStringT<WCHAR> operator + (const CStringT<WCHAR>& string1, int val)
    {
        CStringT<WCHAR> str;
        str.Format(L"%s%d", string1.c_str(), val);
        return str;
    }


    // Addition operator.
    inline CStringT<CHAR> operator + (const CStringT<CHAR>& string1, double val)
    {
        CStringT<CHAR> str;
        str.Format("%s%g", string1.c_str(), val);
        return str;
    }


    // Addition operator.
    inline CStringT<WCHAR> operator + (const CStringT<WCHAR>& string1, double val)
    {
        CStringT<WCHAR> str;
        str.Format(L"%s%g", string1.c_str(), val);
        return str;
    }


    // Addition operator.
    inline CStringT<CHAR> operator + (const CHAR* pszText, const CStringT<CHAR>& string1)
    {
        CStringT<CHAR> str(pszText);
        str.m_str.append(string1);
        return str;
    }


    // Addition operator.
    inline CStringT<WCHAR> operator + (const WCHAR* pszText, const CStringT<WCHAR>& string1)
    {
        CStringT<WCHAR> str(pszText);
        str.m_str.append(string1);
        return str;
    }


    // Addition operator.
    inline CStringT<CHAR> operator + (CHAR ch, const CStringT<CHAR>& string1)
    {
        CStringT<CHAR> str(ch);
        str.m_str.append(string1);
        return str;
    }


    // Addition operator.
    inline CStringT<WCHAR> operator + (WCHAR ch, const CStringT<WCHAR>& string1)
    {
        CStringT<WCHAR> str(ch);
        str.m_str.append(string1);
        return str;
    }


    // Addition operator.
    inline CStringT<CHAR> operator + (int val, const CStringT<CHAR>& string1)
    {
        CStringT<CHAR> str;
        str.Format("%d%s", val, string1.c_str());
        return str;
    }


    // Addition operator.
    inline CStringT<WCHAR> operator + (int val, const CStringT<WCHAR>& string1)
    {
        CStringT<WCHAR> str;
        str.Format(L"%d%s", val, string1.c_str());
        return str;
    }


    // Addition operator.
    inline CStringT<CHAR> operator + (double val, const CStringT<CHAR>& string1)
    {
        CStringT<CHAR> str;
        str.Format("%g%s", val, string1.c_str());
        return str;
    }


    // Addition operator.
    inline CStringT<WCHAR> operator + (double val, const CStringT<WCHAR>& string1)
    {
        CStringT<WCHAR> str;
        str.Format(L"%g%s", val, string1.c_str());
        return str;
    }


    // Performs a case sensitive comparison of the two strings.
    template <class T>
    inline bool operator < (const CStringT<T>& string1, const CStringT<T>& string2)
    {
        return (string1.Compare(string2) < 0);  // boolean expression
    }


    // Performs a case sensitive comparison of the two strings.
    template <class T>
    inline bool operator > (const CStringT<T>& string1, const CStringT<T>& string2)
    {
        return (string1.Compare(string2) > 0);  // boolean expression
    }


    // Performs a case sensitive comparison of the two strings.
    template <class T>
    inline bool operator <= (const CStringT<T>& string1, const CStringT<T>& string2)
    {
        return (string1.Compare(string2) <= 0); // boolean expression
    }


    // Performs a case sensitive comparison of the two strings.
    template <class T>
    inline bool operator >= (const CStringT<T>& string1, const CStringT<T>& string2)
    {
        return (string1.Compare(string2) >= 0); // boolean expression
    }


    // Performs a case sensitive comparison of the two strings.
    template <class T>
    inline bool operator < (const CStringT<T>& string1, const T* pszText)
    {
        return (string1.Compare(pszText) < 0);  // boolean expression
    }


    // Performs a case sensitive comparison of the two strings.
    template <class T>
    inline bool operator > (const CStringT<T>& string1, const T* pszText)
    {
        return (string1.Compare(pszText) > 0);  // boolean expression
    }


    // Performs a case sensitive comparison of the two strings.
    template <class T>
    inline bool operator <= (const CStringT<T>& string1, const T* pszText)
    {
        return (string1.Compare(pszText) <= 0); // boolean expression
    }


    // Performs a case sensitive comparison of the two strings.
    template <class T>
    inline bool operator >= (const CStringT<T>& string1, const T* pszText)
    {
        return (string1.Compare(pszText) >= 0); // boolean expression
    }



    //////////////////////////////////////////////
    // CString global operator functions
    //  These functions are declared friends of CString
    //
    inline CString operator + (const CString& string1, const CString& string2)
    {
        CString str(string1);
        str.m_str.append(string2.m_str);
        return str;
    }

    inline CString operator + (const CString& string1, const TCHAR* pszText)
    {
        CString str(string1);
        str.m_str.append(pszText);
        return str;
    }

    inline CString operator + (const CString& string1, TCHAR ch)
    {
        CString str(string1);
        str.m_str += ch;
        return str;
    }

    inline CString operator + (const CString& string1, int val)
    {
        CString str;
        str.Format(_T("%s%d"), string1.c_str(), val);
        return str;
    }

    inline CString operator + (const CString& string1, double val)
    {
        CString str;
        str.Format(_T("%s%g"), string1.c_str(), val);
        return str;
    }

    inline CString operator + (const TCHAR* pszText, const CString& string1)
    {
        CString str(pszText);
        str.m_str.append(string1);
        return str;
    }

    inline CString operator + (TCHAR ch, const CString& string1)
    {
        CString str(ch);
        str.m_str.append(string1);
        return str;
    }

    inline CString operator + (int val, const CString& string1)
    {
        CString str;
        str.Format(_T("%d%s"), val, string1.c_str());
        return str;
    }

    inline CString operator + (double val, const CString& string1)
    {
        CString str;
        str.Format(_T("%g%s"), val, string1.c_str());
        return str;
    }

    //////////////////////////////////////////////
    // CString global functions
    //

    // Returns the path to the AppData folder. Returns an empty CString if
    // the Operating System doesn't support the use of an AppData folder.
    // The AppData folder is available in Windows 2000 and above.
    inline CString GetAppDataPath()
    {
        CString AppData;

#ifndef _WIN32_WCE

        HMODULE hShell = ::LoadLibrary(_T("Shell32.dll"));
        if (hShell)
        {
            typedef HRESULT(WINAPI *MYPROC)(HWND, int, HANDLE, DWORD, LPTSTR);

            // Get the function pointer of the SHGetFolderPath function
  #ifdef UNICODE
            MYPROC pSHGetFolderPath = (MYPROC)GetProcAddress(hShell, "SHGetFolderPathW");
  #else
            MYPROC pSHGetFolderPath = (MYPROC)GetProcAddress(hShell, "SHGetFolderPathA");
  #endif

  #ifndef CSIDL_APPDATA
    #define CSIDL_APPDATA     0x001a
  #endif

  #ifndef CSIDL_FLAG_CREATE
    #define CSIDL_FLAG_CREATE 0x8000
  #endif

            if (pSHGetFolderPath)
            {
                // Call the SHGetFolderPath function to retrieve the AppData folder
                pSHGetFolderPath(NULL, CSIDL_APPDATA | CSIDL_FLAG_CREATE, NULL, 0, AppData.GetBuffer(MAX_PATH));
                AppData.ReleaseBuffer();
            }

            ::FreeLibrary(hShell);
        }

#endif // _WIN32_WCE

        return AppData;
    }


    // Retrieves the command line arguments and stores them in a vector of CString.
    // Similar to CommandLineToArgvW, but supports all versions of Windows,
    // supports ANSI and Unicode, and doesn't require the user to use LocalFree.
    inline std::vector<CString> GetCommandLineArgs()
    {
        std::vector<CString> CommandLineArgs;
        CString CommandLine = GetCommandLine();
        int index = 0;
        int endPos = 0;

        while (index < CommandLine.GetLength() )
        {
            // Is the argument quoted?
            bool IsQuoted = (CommandLine[index] == _T('\"'));

            if (IsQuoted)
            {
                // Find the terminating token (quote followed by space)
                endPos = CommandLine.Find( _T("\" ") , index);
                if (endPos == -1) endPos = CommandLine.GetLength()-1;

                // Store the argument in the CStringT vector without the quotes.
                CString s;
                if (endPos - index < 2)
                    s = _T("\"\"");     // "" for a single quote or double quote argument
                else
                    s = CommandLine.Mid(index +1, endPos - index -1);

                CommandLineArgs.push_back(s);
                index = endPos + 2;
            }
            else
            {
                // Find the terminating token (space character)
                endPos = CommandLine.Find( _T(' ') , index);
                if (endPos == -1) endPos = CommandLine.GetLength();

                // Store the argument in the CStringT vector.
                CString s = CommandLine.Mid(index, endPos - index);
                CommandLineArgs.push_back(s);
                index = endPos + 1;
            }

            // skip excess space characters
            while (index < CommandLine.GetLength() && CommandLine[index] == _T(' '))
                index++;
        }

        // CommandLineArgs is a vector of CStringT
        return CommandLineArgs;
    }


}   // namespace Win32xx

#endif//_WIN32XX_CSTRING_H_
