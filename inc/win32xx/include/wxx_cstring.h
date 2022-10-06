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
//      // This is invalid, and produces undefined behavior.
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
//                      (from ::GetLastError() for example).
//       GetString      Returns a reference to the underlying std::basic_string<TCHAR>. This
//                      reference can be used to modify the string directly.


#ifndef _WIN32XX_CSTRING_H_
#define _WIN32XX_CSTRING_H_


// The wxx_setup.h file defines the set of macros and includes the C, C++,
// and windows header files required by Win32++.
#include "wxx_setup.h"
#include "wxx_textconv.h"


namespace Win32xx
{

    /////////////////////////////////////////////////
    // CStringT is a class template used to implement
    // CStringA, CStringW and CString.
    template <class T>
    class CStringT
    {
        // Friend functions allow the left hand side to be something other than CStringT

        // These specialized friend declarations are compatible with all supported compilers
        friend CStringT<CHAR> operator + (const CStringT<CHAR>& string1, const CStringT<CHAR>& string2);
        friend CStringT<CHAR> operator + (const CStringT<CHAR>& string1, const CHAR* text);
        friend CStringT<CHAR> operator + (const CStringT<CHAR>& string1, CHAR ch);
        friend CStringT<CHAR> operator + (const CHAR* text, const CStringT<CHAR>& string1);
        friend CStringT<CHAR> operator + (CHAR ch, const CStringT<CHAR>& string1);

        friend CStringT<WCHAR> operator + (const CStringT<WCHAR>& string1, const CStringT<WCHAR>& string2);
        friend CStringT<WCHAR> operator + (const CStringT<WCHAR>& string1, const WCHAR* text);
        friend CStringT<WCHAR> operator + (const CStringT<WCHAR>& string1, WCHAR ch);
        friend CStringT<WCHAR> operator + (const WCHAR* text, const CStringT<WCHAR>& string1);
        friend CStringT<WCHAR> operator + (WCHAR ch, const CStringT<WCHAR>& string1);

        // These global functions don't need to be friends
    //  bool operator < (const CStringT<T>& string1, const CStringT<T>& string2);
    //  bool operator > (const CStringT<T>& string1, const CStringT<T>& string2);
    //  bool operator <= (const CStringT<T>& string1, const CStringT<T>& string2);
    //  bool operator >= (const CStringT<T>& string1, const CStringT<T>& string2);
    //  bool operator < (const CStringT<T>& string1, const T* text);
    //  bool operator > (const CStringT<T>& string1, const T* text);
    //  bool operator <= (const CStringT<T>& string1, const T* text);
    //  bool operator >= (const CStringT<T>& string1, const T* text);

        public:
        CStringT();
        virtual ~CStringT();
        CStringT(const CStringT& str);
        CStringT(const T * text);
        CStringT(T ch, int repeat = 1);
        CStringT(const T * text, int length);

        CStringT& operator = (const CStringT& str);
        CStringT& operator = (const T ch);
        CStringT& operator = (const T* text);

        bool     operator == (const T* text) const;
        bool     operator == (const CStringT& str) const;
        bool     operator != (const T* text) const;
        bool     operator != (const CStringT& str) const;
                 operator const T*() const;
        T&       operator [] (int index);
        const T& operator [] (int index) const;
        CStringT& operator += (const CStringT& str);
        CStringT& operator += (const T* text);
        CStringT& operator += (const T ch);

        // Accessors
        const T* c_str() const          { return m_str.c_str(); }                // alternative for casting to const T*
        const std::basic_string<T>& GetString() const { return m_str; }          // returns const reference to CString's internal std::basic_string<T>
        int      GetLength() const  { return static_cast<int>(m_str.length()); } // returns the length in characters

        // Operations
        BSTR     AllocSysString() const;
        void     AppendFormat(const T* format,...);
        void     AppendFormat(UINT formatID, ...);
        void     Assign(const T* text, int count);
        int      Collate(const T* text) const;
        int      CollateNoCase(const T* text) const;
        int      Compare(const T* text) const;
        int      CompareNoCase(const T* text) const;
        int      Delete(int index, int count = 1);
        int      Find(T ch, int index = 0 ) const;
        int      Find(const T* text, int start = 0) const;
        int      FindOneOf(const T* text) const;
        void     Format(UINT id, ...);
        void     Format(const T* format,...);
        void     FormatV(const T* format, va_list args);
        void     FormatMessage(const T* format,...);
        void     FormatMessageV(const T* format, va_list args);
        T        GetAt(int index) const;
        T*       GetBuffer(int minBufLength);
        bool     GetEnvironmentVariable(const T* var);
        void     GetErrorString(DWORD error);
        void     GetWindowText(HWND wnd);
        void     Empty();
        int      Insert(int index, T ch);
        int      Insert(int index, const CStringT& str);
        bool     IsEmpty() const;
        CStringT Left(int count) const;
        bool     LoadString(UINT id);      // defined in wincore.h
        void     MakeLower();
        void     MakeReverse();
        void     MakeUpper();
        CStringT Mid(int first) const;
        CStringT Mid(int first, int count) const;
        void     ReleaseBuffer( int newLength = -1 );
        int      Remove(const T* text);
        int      Remove(T ch);
        int      Replace(T oldChar, T newChar);
        int      Replace(const T* oldText, const T* newText);
        int      ReverseFind(T ch, int end = -1) const;
        int      ReverseFind(const T* text, int end = -1) const;
        CStringT Right(int count) const;
        void     SetAt(int index, T ch);
        BSTR     SetSysString(BSTR* pBstr) const;
        CStringT SpanExcluding(const T* text) const;
        CStringT SpanIncluding(const T* text) const;
        CStringT Tokenize(const T* tokens, int& start) const;
        void     Trim();
        void     TrimLeft();
        void     TrimLeft(T target);
        void     TrimLeft(const T* targets);
        void     TrimRight();
        void     TrimRight(T target);
        void     TrimRight(const T* targets);
        void     Truncate(int newLength);

    protected:
        std::basic_string<T> m_str;
        std::vector<T> m_buf;

    private:
        int     lstrlenT(const CHAR* text) const  { return ::lstrlenA(text); }
        int     lstrlenT(const WCHAR* text) const { return ::lstrlenW(text); }

        // These functions return CHAR instead of int.
        static CHAR ToLower(CHAR c) { return static_cast<CHAR>(::tolower(static_cast<unsigned char>(c)) & 0xFF); }
        static CHAR ToUpper(CHAR c) { return static_cast<CHAR>(::toupper(static_cast<unsigned char>(c)) & 0xFF); }
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
        friend CString operator + (const CString& string1, const WCHAR* text);
        friend CString operator + (const CString& string1, const CHAR* text);
        friend CString operator + (const CString& string1, CHAR ch);
        friend CString operator + (const CString& string1, WCHAR ch);
        friend CString operator + (const TCHAR* text, const CString& string1);
        friend CString operator + (CHAR ch, const CString& string1);
        friend CString operator + (WCHAR ch, const CString& string1);

    public:
        CString() {}
        CString(const CString& str)            : CStringT<TCHAR>(str) {}
        CString(LPCSTR text)                   : CStringT<TCHAR>(AtoT(text)) {}
        CString(LPCWSTR text)                  : CStringT<TCHAR>(WtoT(text))    {}
        CString(LPCSTR text, int length)       : CStringT<TCHAR>(AtoT(text, CP_ACP, length), length) {}
        CString(LPCWSTR text, int length)      : CStringT<TCHAR>(WtoT(text, CP_ACP, length), length) {}

        CString(char ch, int repeat = 1)
        {
            for (int i = 0; i < repeat; ++i)
            {
                operator +=(ch);
            }
        }

        CString(WCHAR ch, int repeat = 1)
        {
            for (int i = 0; i < repeat; ++i)
            {
                operator +=(ch);
            }
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

        CString& operator = (LPCSTR text)
        {
            m_str.assign(AtoT(text));
            return *this;
        }

        CString& operator = (LPCWSTR text)
        {
            m_str.assign(WtoT(text));
            return *this;
        }

        CString& operator += (const CString& str)
        {
            m_str.append(str.m_str);
            return *this;
        }

        CString& operator += (LPCSTR text)
        {
            m_str.append(AtoT(text));
            return *this;
        }

        CString& operator += (LPCWSTR text)
        {
            m_str.append(WtoT(text));
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

        CString Left(int count) const
        {
            CString str;
            str = CStringT<TCHAR>::Left(count);
            return str;
        }

        CString Mid(int first) const
        {
            CString str;
            str = CStringT<TCHAR>::Mid(first);
            return str;
        }

        CString Mid(int first, int count) const
        {
            CString str;
            str = CStringT<TCHAR>::Mid(first, count);
            return str;
        }

        CString Right(int count) const
        {
            CString str;
            str = CStringT<TCHAR>::Right(count);
            return str;
        }

        CString SpanExcluding(LPCTSTR text) const
        {
            CString str;
            str = CStringT<TCHAR>::SpanExcluding(text);
            return str;
        }

        CString SpanIncluding(LPCTSTR text) const
        {
            CString str;
            str = CStringT<TCHAR>::SpanIncluding(text);
            return str;
        }

        CString Tokenize(LPCTSTR tokens, int& start) const
        {
            CString str;
            str = CStringT<TCHAR>::Tokenize(tokens, start);
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
    inline CStringT<T>::CStringT(const T* text)
    {
        assert(text != NULL);
        m_str.assign(text);
    }

    // Constructor. Assigns from 1 or more T characters.
    template <class T>
    inline CStringT<T>::CStringT(T ch, int repeat)
    {
        m_str.assign(static_cast<size_t>(repeat), ch);
    }

    // Constructor. Assigns from a const T* possibly containing null characters.
    // Ensure the size of the text buffer holds length or more characters
    template <class T>
    inline CStringT<T>::CStringT(const T* text, int length)
    {
        memcpy(GetBuffer(length), text, length*sizeof(T));
        ReleaseBuffer(length);
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
        m_str.assign(1, ch);
        return *this;
    }

    // Assign from a const T* character array.
    template <class T>
    inline CStringT<T>& CStringT<T>::operator = (const T* text)
    {
        m_str.assign(text);
        return *this;
    }

    // Returns TRUE if the strings have the same content.
    template <class T>
    inline bool CStringT<T>::operator == (const T* text) const
    {
        assert(text != 0);
        return (Compare(text) == 0);
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
    inline bool CStringT<T>::operator != (const T* text) const
    {
        assert(text != 0);
        return Compare(text) != 0;
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
    inline T& CStringT<T>::operator [] (int index)
    {
        assert(index >= 0);
        assert(index < GetLength());
        return m_str[static_cast<size_t>(index)];
    }

    // Subscript operator. Returns the T character at the specified index.
    template <class T>
    inline const T& CStringT<T>::operator [] (int index) const
    {
        assert(index >= 0);
        assert(index < GetLength());
        return m_str[index];
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
    inline CStringT<T>& CStringT<T>::operator += (const T* text)
    {
        m_str.append(text);
        return *this;
    }

    // Addition assignment. Appends a T character.
    template <class T>
    inline CStringT<T>& CStringT<T>::operator += (T ch)
    {
        m_str.append(1, ch);
        return *this;
    }

    // Allocates a BSTR from the CStringT content.
    // Note: Ensure the returned BSTR is freed later with SysFreeString to avoid a memory leak.
    template <>
    inline BSTR CStringT<CHAR>::AllocSysString() const
    {
        BSTR bstr = ::SysAllocStringLen(AtoW(m_str.c_str()), static_cast<UINT>(m_str.size()));
        if (bstr == 0)
            throw std::bad_alloc();

        return bstr;
    }

    // Allocates a BSTR from the CStringT content.
    // Note: Free the returned string later with SysFreeString to avoid a memory leak.
    template <>
    inline BSTR CStringT<WCHAR>::AllocSysString() const
    {
        BSTR bstr = ::SysAllocStringLen(m_str.c_str(), static_cast<UINT>(m_str.size()));
        if (bstr == 0)
            throw std::bad_alloc();

        return bstr;
    }

    // Appends formatted data to an the CStringT content.
    template <class T>
    inline void CStringT<T>::AppendFormat(const T* format,...)
    {
        CStringT str;

        va_list args;
        va_start(args, format);
        str.FormatV(format, args);
        va_end(args);

        m_str.append(str);
    }

    // Assigns the specified number of characters from text to the CStringT.
    template <class T>
    inline void CStringT<T>::Assign(const T* text, int count)
    {
        m_str.assign(text, static_cast<size_t>(count));
    }

    // Performs a case sensitive comparison of the two strings using locale-specific information.
    template <>
    inline int CStringT<CHAR>::Collate(const CHAR* text) const
    {
        assert(text != 0);
        int res = ::CompareStringA(LOCALE_USER_DEFAULT, 0, m_str.c_str(), -1, text, -1);

        assert(res);
        if      (res == CSTR_LESS_THAN) return -1;
        else if (res == CSTR_GREATER_THAN) return 1;

        return 0;
    }

    // Performs a case sensitive comparison of the two strings using locale-specific information.
    template <>
    inline int CStringT<WCHAR>::Collate(const WCHAR* text) const
    {
        assert(text != 0);
        int res = ::CompareStringW(LOCALE_USER_DEFAULT, 0, m_str.c_str(), -1, text, -1);

        assert(res);
        if      (res == CSTR_LESS_THAN) return -1;
        else if (res == CSTR_GREATER_THAN) return 1;

        return 0;
    }

    // Performs a case insensitive comparison of the two strings using locale-specific information.
    template <>
    inline int CStringT<CHAR>::CollateNoCase(const CHAR* text) const
    {
        assert(text != 0);
        int res = ::CompareStringA(LOCALE_USER_DEFAULT, NORM_IGNORECASE, m_str.c_str(), -1, text, -1);

        assert(res);
        if      (res == CSTR_LESS_THAN) return -1;
        else if (res == CSTR_GREATER_THAN) return 1;

        return 0;
    }

    // Performs a case insensitive comparison of the two strings using locale-specific information.
    template <>
    inline int CStringT<WCHAR>::CollateNoCase(const WCHAR* text) const
    {
        assert(text != 0);
        int res = ::CompareStringW(LOCALE_USER_DEFAULT, NORM_IGNORECASE, m_str.c_str(), -1, text, -1);

        assert(res);
        if      (res == CSTR_LESS_THAN) return -1;
        else if (res == CSTR_GREATER_THAN) return 1;

        return 0;
    }

    // Performs a case sensitive comparison of the two strings.
    template <>
    inline int CStringT<CHAR>::Compare(const CHAR* text) const
    {
        assert(text != 0);
        return ::lstrcmpA(m_str.c_str(), text);
    }

    // Performs a case sensitive comparison of the two strings.
    template <>
    inline int CStringT<WCHAR>::Compare(const WCHAR* text) const
    {
        assert(text != 0);
        return ::lstrcmpW(m_str.c_str(), text);
    }

    // Performs a case insensitive comparison of the two strings.
    template <>
    inline int CStringT<CHAR>::CompareNoCase(const CHAR* text) const
    {
        assert(text != 0);
        return ::lstrcmpiA(m_str.c_str(), text);
    }

    // Performs a case insensitive comparison of the two strings.
    template <>
    inline int CStringT<WCHAR>::CompareNoCase(const WCHAR* text) const
    {
        assert(text != 0);
        return ::lstrcmpiW(m_str.c_str(), text);
    }

    // Deletes a character or characters from the string.
    template <class T>
    inline int CStringT<T>::Delete(int index, int count /* = 1 */)
    {
        assert(index >= 0);
        assert(count >= 0);

        if (index < GetLength())
            m_str.erase(static_cast<size_t>(index), static_cast<size_t>(count));

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
    inline int CStringT<T>::Find(T ch, int index /* = 0 */) const
    {
        assert(index >= 0);

        size_t s = m_str.find(ch, static_cast<size_t>(index));
        return static_cast<int>(s);
    }

    // Finds a substring within the string.
    template <class T>
    inline int CStringT<T>::Find(const T* text, int index /* = 0 */) const
    {
        assert(text != 0);
        assert(index >= 0);

        size_t s = m_str.find(text, static_cast<size_t>(index));
        return static_cast<int>(s);
    }

    // Finds the first matching character from a set.
    template <class T>
    inline int CStringT<T>::FindOneOf(const T* text) const
    {
        assert(text != 0);

        size_t s = m_str.find_first_of(text);
        return static_cast<int>(s);
    }

    // Formats the string as sprintf does.
    template <class T>
    inline void CStringT<T>::Format(const T* format,...)
    {
        va_list args;
        va_start(args, format);
        FormatV(format, args);
        va_end(args);
    }

    // Formats the string using a variable list of arguments.
    template <>
    inline void CStringT<CHAR>::FormatV(const CHAR*  format, va_list args)
    {

        if (format)
        {
            int result = -1;
            size_t length = 256;

            // A vector is used to store the CHAR array
            std::vector<CHAR> buffer;

            while (result == -1)
            {
                buffer.assign( size_t(length)+1, 0 );

#if !defined (_MSC_VER) ||  ( _MSC_VER < 1400 )
                result = _vsnprintf(&buffer.front(), length, format, args);
#else
                result = _vsnprintf_s(&buffer.front(), length, length -1, format, args);
#endif
                length *= 2;
            }
            m_str.assign(&buffer.front());
        }
    }

    // Formats the string using a variable list of arguments.
    template <>
    inline void CStringT<WCHAR>::FormatV(const WCHAR*  format, va_list args)
    {

        if (format)
        {
            int result = -1;
            size_t length = 256;

            // A vector is used to store the WCHAR array
            std::vector<WCHAR> buffer;

            while (result == -1)
            {
                buffer.assign( size_t(length)+1, 0 );
#if !defined (_MSC_VER) ||  ( _MSC_VER < 1400 )
                result = _vsnwprintf(&buffer.front(), length, format, args);
#else
                result = _vsnwprintf_s(&buffer.front(), length, length -1, format, args);
#endif
                length *= 2;
            }
            m_str.assign(&buffer.front());
        }
    }

    // Formats a message string.
    template <class T>
    inline void CStringT<T>::FormatMessage(const T* format,...)
    {
        va_list args;
        va_start(args, format);
        FormatMessageV(format, args);
        va_end(args);
    }

    // Formats a message string using a variable argument list.
    template <>
    inline void CStringT<CHAR>::FormatMessageV(const CHAR* format, va_list args)
    {
        LPSTR temp = 0;
        if (format)
        {
            DWORD result = ::FormatMessageA(FORMAT_MESSAGE_FROM_STRING|FORMAT_MESSAGE_ALLOCATE_BUFFER,
                                   format, 0, 0, reinterpret_cast<LPSTR>(&temp), 0, &args);

            if ( result == 0 || temp == 0 )
                throw std::bad_alloc();

            m_str = temp;
            ::LocalFree(temp);
        }
    }

    // Formats a message string using a variable argument list.
    template <>
    inline void CStringT<WCHAR>::FormatMessageV(const WCHAR* format, va_list args)
    {
        LPWSTR temp = 0;
        if (format)
        {
            DWORD result = ::FormatMessageW(FORMAT_MESSAGE_FROM_STRING|FORMAT_MESSAGE_ALLOCATE_BUFFER,
                                  format, 0, 0, (LPWSTR)&temp, 0, &args);

            if ( result == 0 || temp == 0 )
                throw std::bad_alloc();

            m_str = temp;
            ::LocalFree(temp);
        }
    }

    // Returns the character at the specified location within the string.
    template <class T>
    inline T CStringT<T>::GetAt(int index) const
    {
        assert(index >= 0);
        assert(index < GetLength());
        T ch = 0;

        if ((index >= 0) && (index < GetLength()))
            ch = m_str[static_cast<size_t>(index)];

        return ch;
    }

    // Creates a buffer of minBufLength characters (+1 extra for NULL termination) and returns
    // a pointer to this buffer. This buffer can be used by any function which accepts a LPTSTR.
    // Care must be taken not to exceed the length of the buffer. Use ReleaseBuffer to safely
    // copy this buffer back to the CStringT object.
    // Note: The buffer uses a vector. Vectors are required to be contiguous in memory under
    //       the current standard, whereas std::strings do not have this requirement.
    template <class T>
    inline T* CStringT<T>::GetBuffer(int minBufLength)
    {
        assert (minBufLength >= 0);

        T ch = 0;
        m_buf.assign(size_t(minBufLength) + 1, ch);
        typename std::basic_string<T>::iterator it_end;

        if (m_str.length() >= static_cast<size_t>(minBufLength))
        {
            it_end = m_str.begin();
            std::advance(it_end, minBufLength);
        }
        else
            it_end = m_str.end();

        std::copy(m_str.begin(), it_end, m_buf.begin());

        return &m_buf.front();
    }

    // Sets the string to the value of the specified environment variable.
    template <>
    inline bool CStringT<CHAR>::GetEnvironmentVariable(const CHAR* var)
    {
        assert(var);
        Empty();

        DWORD length = ::GetEnvironmentVariableA(var, NULL, 0);
        if (length > 0)
        {
            std::vector<CHAR> buffer(size_t(length) +1, 0 );
            ::GetEnvironmentVariableA(var, &buffer.front(), length);
            m_str = &buffer.front();
        }

        return (length != 0);
    }

    // Sets the string to the value of the specified environment variable.
    template <>
    inline bool CStringT<WCHAR>::GetEnvironmentVariable(const WCHAR* var)
    {
        assert(var);
        Empty();

        DWORD length = ::GetEnvironmentVariableW(var, NULL, 0);
        if (length > 0)
        {
            std::vector<WCHAR> buffer(static_cast<size_t>(length) +1, 0);
            ::GetEnvironmentVariableW(var, &buffer.front(), length);
            m_str = &buffer.front();
        }

        return (length != 0);
    }

    // Retrieves the a window's text.
    template <>
    inline void CStringT<CHAR>::GetWindowText(HWND wnd)
    {
        Empty();
        int length = ::GetWindowTextLengthA(wnd);
        if (length > 0)
        {
            std::vector<CHAR> buffer(size_t(length) +1, 0 );
            ::GetWindowTextA(wnd, &buffer.front(), length +1);
            m_str = &buffer.front();
        }
    }

    // Retrieves a window's text.
    template <>
    inline void CStringT<WCHAR>::GetWindowText(HWND wnd)
    {
        Empty();
        int length = ::GetWindowTextLengthW(wnd);
        if (length > 0)
        {
            std::vector<WCHAR> buffer(size_t(length) +1, 0 );
            ::GetWindowTextW(wnd, &buffer.front(), length +1);
            m_str = &buffer.front();
        }
    }

    // Returns the error string for the specified System Error Code (e.g from GetLastError).
    template <>
    inline void CStringT<CHAR>::GetErrorString(DWORD error)
    {
        Empty();
        CHAR* temp = 0;
        DWORD flags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;
        ::FormatMessageA(flags, NULL, error, 0, reinterpret_cast<LPSTR>(&temp), 1, NULL);
        m_str.assign(temp);
        ::LocalFree(temp);
    }

    // Returns the error string for the specified System Error Code (e.g from GetLastError).
    template <>
    inline void CStringT<WCHAR>::GetErrorString(DWORD error)
    {
        Empty();
        WCHAR* temp = 0;
        DWORD flags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;
        ::FormatMessageW(flags, NULL, error, 0, reinterpret_cast<LPWSTR>(&temp), 1, NULL);
        m_str.assign(temp);
        ::LocalFree(temp);
    }

    // Inserts a single character at the given index within the string.
    template <class T>
    inline int CStringT<T>::Insert(int index, T ch)
    {
        assert(index >= 0);
        assert(ch);

        index = MIN(index, GetLength());
        m_str.insert(index, &ch, 1);

        return static_cast<int>(m_str.size());
    }

    // Inserts a substring at the given index within the string.
    template <class T>
    inline int CStringT<T>::Insert(int index, const CStringT& str)
    {
        assert(index >= 0);

        index = MIN(index, GetLength());
        m_str.insert(index, str);

        return static_cast<int>(m_str.size());
    }

    // Returns TRUE if the string is empty.
    template <class T>
    inline bool CStringT<T>::IsEmpty() const
    {
        return m_str.empty();
    }

    // Retrieves the left part of a string. The count parameter specifies the
    // number of characters.
    template <class T>
    inline CStringT<T> CStringT<T>::Left(int count) const
    {
        assert(count >= 0);

        CStringT str;
        str.m_str.assign(m_str, 0, static_cast<size_t>(count));
        return str;
    }

    // Converts all the characters in this string to lowercase characters.
    template <>
    inline void CStringT<CHAR>::MakeLower()
    {
        std::transform(m_str.begin(), m_str.end(), m_str.begin(), ToLower);
    }

    // Converts all the characters in this string to lowercase characters.
    template <>
    inline void CStringT<WCHAR>::MakeLower()
    {
        std::transform(m_str.begin(), m_str.end(), m_str.begin(), ::towlower);
    }

    // Reverses the string.
    template <class T>
    inline void CStringT<T>::MakeReverse()
    {
        std::reverse(m_str.begin(), m_str.end());
    }

    // Converts all the characters in this string to uppercase characters.
    template <>
    inline void CStringT<CHAR>::MakeUpper()
    {
        std::transform(m_str.begin(), m_str.end(), m_str.begin(), ToUpper);
    }

    // Converts all the characters in this string to uppercase characters.
    template <>
    inline void CStringT<WCHAR>::MakeUpper()
    {
        std::transform(m_str.begin(), m_str.end(), m_str.begin(), ::towupper);
    }

    // Retrieves the middle part of a string.
    // The first parameter specifies the zero based index of the first character.
    template <class T>
    inline CStringT<T> CStringT<T>::Mid(int first) const
    {
        return Mid(first, GetLength());
    }

    // Retrieves the middle part of a string.
    // The first parameter specifies the zero based index of the first character.
    // The count parameter specifies the number of characters.
    template <class T>
    inline CStringT<T> CStringT<T>::Mid(int first, int count) const
    {
        assert(first >= 0);
        assert(count >= 0);

        CStringT str;
        if (first <= GetLength())
            str.m_str.assign(m_str, static_cast<size_t>(first), static_cast<size_t>(count));

        return str;
    }

    // This copies the contents of the buffer (acquired by GetBuffer) to this CStringT.
    // The default length of -1 copies from the buffer until a null terminator is reached.
    // If the buffer doesn't contain a null terminator, you must specify the buffer's length.
    template <class T>
    inline void CStringT<T>::ReleaseBuffer( int newLength /*= -1*/ )
    {
        if (newLength == -1)
        {
            newLength = lstrlenT(&m_buf.front());
        }

        assert(m_buf.size() > 0);
        assert(newLength <= static_cast<int>(m_buf.size() -1));
        newLength = MIN(newLength, static_cast<int>(m_buf.size() -1));

        T ch = 0;
        m_str.assign(static_cast<size_t>(newLength), ch);

        typename std::vector<T>::iterator it_end = m_buf.begin();
        std::advance(it_end, static_cast<size_t>(newLength));

        std::copy(m_buf.begin(), it_end, m_str.begin());
        m_buf.clear();
    }

    // Removes each occurrence of the specified substring from the string.
    template <class T>
    inline int CStringT<T>::Remove(const T* text)
    {
        assert(text != 0);

        int count = 0;
        size_t pos = 0;
        size_t len = lstrlenT(text);
        if (len > 0)
        {
            while ((pos = m_str.find(text, pos)) != std::string::npos)
            {
                m_str.erase(pos, len);
                ++count;
            }
        }

        return count;
    }

    // Removes each occurrence of the specified character from the string.
    template <class T>
    inline int CStringT<T>::Remove(T ch)
    {
        int count = 0;
        size_t pos = 0;

        while ((pos = m_str.find(ch, pos)) != std::string::npos)
        {
            m_str.erase(pos, 1);
            ++count;
        }

        return count;
    }

    // Replaces each occurrence of the old character with the new character.
    template <class T>
    inline int CStringT<T>::Replace(T oldChar, T newChar)
    {
        int count = 0;
        typename std::basic_string<T>::iterator it;
        it = m_str.begin();
        while (it != m_str.end())
        {
            if (*it == oldChar)
            {
                *it = newChar;
                ++count;
            }
            ++it;
        }
        return count;
    }

    // Replaces each occurrence of the old substring with the new substring.
    template <class T>
    inline int CStringT<T>::Replace(const T* oldText, const T* newText)
    {
        assert(oldText);
        assert(newText);

        int count = 0;
        size_t pos = 0;
        size_t lenOld = static_cast<size_t>(lstrlenT(oldText));
        size_t lenNew = static_cast<size_t>(lstrlenT(newText));
        if (lenOld > 0 && lenNew > 0)
        {
            while ((pos = m_str.find(oldText, pos)) != std::string::npos)
            {
                m_str.replace(pos, lenOld, newText);
                pos += lenNew;
                ++count;
            }
        }
        return count;
    }

    // Search for a character within the string, starting from the end.
    template <class T>
    inline int CStringT<T>::ReverseFind(T ch, int end /* -1 */) const
    {
        size_t found = m_str.rfind(ch, static_cast<size_t>(end));
        return static_cast<int>(found);
    }

    // Search for a substring within the string, starting from the end.
    template <class T>
    inline int CStringT<T>::ReverseFind(const T* text, int end /* = -1 */) const
    {
        assert(text != 0);
        if (!text) return -1;

        if (lstrlenT(text) == 1)
            return ReverseFind(text[0], end);
        else
            return static_cast<int>(m_str.rfind(text, static_cast<size_t>(end)));
    }

    // Retrieves count characters from the right part of the string.
    // The count parameter specifies the number of characters.
    template <class T>
    inline CStringT<T> CStringT<T>::Right(int count) const
    {
        assert(count >= 0);

        CStringT str;
        count = MIN(count, GetLength());
        str.m_str.assign(m_str, m_str.size() - static_cast<size_t>(count), static_cast<size_t>(count));
        return str;
    }

    // Sets the character at the specified position to the specified value.
    template <class T>
    inline void CStringT<T>::SetAt(int index, T ch)
    {
        assert(index >= 0);
        assert(index < GetLength());

        if ((index >= 0) && (index < GetLength()))
            m_str[index] = ch;
    }

    // Sets an existing BSTR object to the string.
    // Note: Ensure the returned BSTR is freed later with SysFreeString to avoid a memory leak.
    template <>
    inline BSTR CStringT<CHAR>::SetSysString(BSTR* pBstr) const
    {
        assert(pBstr);

        if ( !::SysReAllocStringLen(pBstr, AtoW(m_str.c_str()), static_cast<UINT>(m_str.length())) )
            throw std::bad_alloc();

        return pBstr? *pBstr : 0;
    }

    // Sets an existing BSTR object to the string.
    template <>
    inline BSTR CStringT<WCHAR>::SetSysString(BSTR* pBstr) const
    {
        assert(pBstr);

        if ( !::SysReAllocStringLen(pBstr, m_str.c_str(), static_cast<UINT>(m_str.length())) )
            throw std::bad_alloc();

        return pBstr ? *pBstr : 0;
    }

    // Extracts characters from the string, starting with the first character,
    // that are not in the set of characters identified by text.
    template <class T>
    inline CStringT<T> CStringT<T>::SpanExcluding(const T* text) const
    {
        assert (text);

        CStringT str;
        size_t pos = 0;

        while ((pos = m_str.find_first_not_of(text, pos)) != std::string::npos)
        {
            str.m_str.append(1, m_str[pos++]);
        }

        return str;
    }

    // Extracts a substring that contains only the characters in a set.
    template <class T>
    inline CStringT<T> CStringT<T>::SpanIncluding(const T* text) const
    {
        assert (text);

        CStringT str;
        size_t pos = 0;

        while ((pos = m_str.find_first_of(text, pos)) != std::string::npos)
        {
            str.m_str.append(1, m_str[pos++]);
        }

        return str;
    }

    // Extracts specified tokens in a target string.
    template <class T>
    inline CStringT<T> CStringT<T>::Tokenize(const T* tokens, int& start) const
    {
        assert(tokens);

        CStringT str;
        if (start >= 0)
        {
        size_t pos1 = m_str.find_first_not_of(tokens, static_cast<size_t>(start));
        size_t pos2 = m_str.find_first_of(tokens, pos1);

        start = static_cast<int>(pos2) + 1;
        if (pos2 == m_str.npos)
            start = -1;

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
    template <>
    inline void CStringT<CHAR>::TrimLeft()
    {
        // This method is supported by the Borland 5.5 compiler
        std::basic_string<CHAR>::iterator iter;
        for (iter = m_str.begin(); iter != m_str.end(); ++iter)
        {
            if (!::isspace(static_cast<unsigned char>(*iter)))
                break;
        }

        m_str.erase(m_str.begin(), iter);
    }

    // Trims leading whitespace characters from the string.
    template <>
    inline void CStringT<WCHAR>::TrimLeft()
    {
        // This method is supported by the Borland 5.5 compiler
        std::basic_string<WCHAR>::iterator iter;
        for (iter = m_str.begin(); iter != m_str.end(); ++iter)
        {
            if (!iswspace(*iter))
                break;
        }

        m_str.erase(m_str.begin(), iter);
    }

    // Trims the specified character from the beginning of the string.
    template <class T>
    inline void CStringT<T>::TrimLeft(T target)
    {
        m_str.erase(0, m_str.find_first_not_of(target));
    }

    // Trims the specified set of characters from the beginning of the string.
    template <class T>
    inline void CStringT<T>::TrimLeft(const T* targets)
    {
        assert(targets);
        m_str.erase(0, m_str.find_first_not_of(targets));
    }

    // Trims trailing whitespace characters from the string.
    template <>
    inline void CStringT<CHAR>::TrimRight()
    {
        // This method is supported by the Borland 5.5 compiler
        std::basic_string<CHAR>::reverse_iterator riter;
        for (riter = m_str.rbegin(); riter < m_str.rend(); ++riter)
        {
            if (!::isspace(static_cast<unsigned char>(*riter)))
                break;
        }

        m_str.erase(riter.base(), m_str.end());
    }

    // Trims trailing whitespace characters from the string.
    template <>
    inline void CStringT<WCHAR>::TrimRight()
    {
        // This method is supported by the Borland 5.5 compiler
        std::basic_string<WCHAR>::reverse_iterator riter;
        for (riter = m_str.rbegin(); riter < m_str.rend(); ++riter)
        {
            if (!iswspace(*riter))
                break;
        }

        m_str.erase(riter.base(), m_str.end());
    }

    // Trims the specified character from the end of the string.
    template <class T>
    inline void CStringT<T>::TrimRight(T target)
    {
        size_t pos = m_str.find_last_not_of(target);
        if (pos != std::string::npos)
            m_str.erase(++pos);
    }

    // Trims the specified set of characters from the end of the string.
    template <class T>
    inline void CStringT<T>::TrimRight(const T* targets)
    {
        assert(targets);

        size_t pos = m_str.find_last_not_of(targets);
        if (pos != std::string::npos)
            m_str.erase(++pos);
    }

    // Reduces the length of the string to the specified amount.
    template <class T>
    inline void CStringT<T>::Truncate(int newLength)
    {
        if (newLength < GetLength())
        {
            assert(newLength >= 0);
            m_str.erase(newLength);
        }
    }

    /////////////////////////////
    // Global ToCString functions
    //

    template <class V>
    CStringA ToCStringA(V value)
    {
        std::basic_stringstream<CHAR> streamA;
        streamA << value;
        return CStringA(streamA.str().c_str());
    }

    template <class V>
    CStringW ToCStringW(V value)
    {
        std::basic_stringstream<WCHAR> streamW;
        streamW << value;
        return CStringW(streamW.str().c_str());
    }

    template <class V>
    CString ToCString(V value)
    {
        tStringStream streamT;
        streamT << value;
        return CString(streamT.str().c_str());
    }


    //////////////////////////////////////
    // CStringT global operator functions
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
    inline CStringT<CHAR> operator + (const CStringT<CHAR>& string1, const CHAR* text)
    {
        CStringT<CHAR> str(string1);
        str.m_str.append(text);
        return str;
    }

    // Addition operator.
    inline CStringT<WCHAR> operator + (const CStringT<WCHAR>& string1, const WCHAR* text)
    {
        CStringT<WCHAR> str(string1);
        str.m_str.append(text);
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
    inline CStringT<CHAR> operator + (const CHAR* text, const CStringT<CHAR>& string1)
    {
        CStringT<CHAR> str(text);
        str.m_str.append(string1);
        return str;
    }

    // Addition operator.
    inline CStringT<WCHAR> operator + (const WCHAR* text, const CStringT<WCHAR>& string1)
    {
        CStringT<WCHAR> str(text);
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
    inline bool operator < (const CStringT<T>& string1, const T* text)
    {
        return (string1.Compare(text) < 0);  // boolean expression
    }

    // Performs a case sensitive comparison of the two strings.
    template <class T>
    inline bool operator > (const CStringT<T>& string1, const T* text)
    {
        return (string1.Compare(text) > 0);  // boolean expression
    }

    // Performs a case sensitive comparison of the two strings.
    template <class T>
    inline bool operator <= (const CStringT<T>& string1, const T* text)
    {
        return (string1.Compare(text) <= 0); // boolean expression
    }

    // Performs a case sensitive comparison of the two strings.
    template <class T>
    inline bool operator >= (const CStringT<T>& string1, const T* text)
    {
        return (string1.Compare(text) >= 0); // boolean expression
    }

    template <class V>
    inline CStringT<CHAR>& operator << (CStringT<CHAR>& str, V value)
    {
        str += ToCStringA(value);
        return str;
    }

    template <>  // Explicit specialization
    inline CStringT<CHAR>& operator << (CStringT<CHAR>& str, CStringT<CHAR>& value)
    {
        str += value;
        return str;
    }

    template <class V>
    inline CStringT<WCHAR>& operator << (CStringT<WCHAR>& str, V value)
    {
        str += ToCStringW(value);
        return str;
    }

    template <>  // Explicit specialization
    inline CStringT<WCHAR>& operator << (CStringT<WCHAR>& str, CStringT<WCHAR> value)
    {
       str += value;
       return str;
    }


    //////////////////////////////////////////////
    // CString global operator functions
    //
    inline CString operator + (const CString& string1, const CString& string2)
    {
        CString str(string1);
        str.m_str.append(string2.m_str);
        return str;
    }

    inline CString operator + (const CString& string1, const CHAR* text)
    {
        CString str(string1);
        str.m_str.append(AtoT(text));
        return str;
    }

    inline CString operator + (const CString& string1, const WCHAR* text)
    {
        CString str(string1);
        str.m_str.append(WtoT(text));
        return str;
    }

    inline CString operator + (const CString& string1, CHAR ch)
    {
        CString str(string1);
        CString str1(ch);
        str += str1;
        return str;
    }

    inline CString operator + (const CString& string1, WCHAR ch)
    {
        CString str(string1);
        CString str1(ch);
        str += str1;
        return str;
    }

    inline CString operator + (const TCHAR* text, const CString& string1)
    {
        CString str(text);
        str.m_str.append(string1);
        return str;
    }

    inline CString operator + (CHAR ch, const CString& string1)
    {
        CString str1(string1);
        CString str(ch);
        str += str1;
        return str;
    }

    inline CString operator + (WCHAR ch, const CString& string1)
    {
        CString str1(string1);
        CString str(ch);
        str += str1;
        return str;
    }

    // Appends the specified string to the string.
    inline CString& operator << (CString& str, const CString& str2)
    {
        str += str2;
        return str;
    }

    template <class V>
    inline CString& operator << (CString& str, V value)
    {
        str += ToCString(value);
        return str;
    }

    // Appends the specified text to the string.
    inline CString& operator << (CString& str, LPCSTR text)
    {
        str += text;
        return str;
    }

    // Appends the specified text to the string.
    inline CString& operator << (CString& str, LPCWSTR text)
    {
        str += text;
        return str;
    }

    // Appends the specified character to the string.
    inline CString& operator << (CString& str, char ch)
    {
        str += ch;
        return str;
    }

    // Appends the specified character to the string.
    inline CString& operator << (CString& str, WCHAR ch)
    {
        str += ch;
        return str;
    }

}   // namespace Win32xx

#endif//_WIN32XX_CSTRING_H_
