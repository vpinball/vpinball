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


// Acknowledgements:
// Thanks to Adam Szulc for his initial CString code.

////////////////////////////////////////////////////////
// wxx_cstring.h
//  Declaration of the CString class

// This class is intended to provide a simple alternative to the MFC/ATL
// CString class that ships with Microsoft compilers. The CString class
// specified here is compatible with other compilers such as Embarcadero
// and MinGW.

// Differences between this class and the MFC/ATL CString class
// ------------------------------------------------------------
// 1) The constructors for this class accepts both ANSI and Unicode characters
//    and automatically converts these to TCHAR as required.
//
// 2) This class is not reference counted, so these CStrings should be passed
//    as references or const references when used as function arguments. As a
//    result there is no need for functions like LockBuffer and UnLockBuffer.
//
// 3) The Format functions only accepts POD (Plain Old Data) arguments. It does
//    not accept arguments that are class or struct objects. In particular it
//    does not accept CString objects, unless these are cast to LPCTSTR.
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
//    Note: The MFC/ATL CString class uses a non portable hack to make its
//          CString class behave like a POD. Other compilers (such as the
//          MinGW compiler) specifically prohibit the use of non POD types for
//          functions with variable argument lists.
//
// 4) This class provides a few additional functions:
//       c_str          Returns a const TCHAR string. This is an alternative
//                      for casting to LPCTSTR.
//       GetErrorString Assigns CString to the error string for the specified
//                      System Error Code (from ::GetLastError() for example).
//       GetString      Returns a reference to the underlying
//                      std::basic_string<TCHAR>. This reference can be used to
//                      modify the string directly.


#ifndef WIN32XX_CSTRING_H_
#define WIN32XX_CSTRING_H_


// The wxx_setup.h file defines the set of macros and includes the C, C++,
// and windows header files required by Win32++.
#include "wxx_setup.h"
#include "wxx_textconv.h"


namespace Win32xx
{
    // Declaration of the CStringT class.
    template <class T>
    class CStringT;

    // CStringA is a CHAR only version of CString.
    using CStringA = CStringT<CHAR>;

    // CStringW is a WCHAR only version of CString.
    using CStringW = CStringT<WCHAR>;

    // Declaration of the CString class.
    class CString;

    /////////////////////////////////////////////////
    // CStringT is a class template used to implement
    // CStringA and CStringW. CString inherits from
    // CStringT<TCHAR>.
    template <class T>
    class CStringT
    {
        // Allow CString to access private members of CStringT.
        friend class CString;

        // Friend functions for CString global functions.
        friend CString operator+(const CString& string1, const CString& string2);
        friend CString operator+(const CString& string1, const CStringA& string2);
        friend CString operator+(const CString& string1, const CStringW& string2);
        friend CString operator+(const CString& string1, const CHAR* text);
        friend CString operator+(const CString& string1, const WCHAR* text);
        friend CString operator+(const CString& string1, CHAR ch);
        friend CString operator+(const CString& string1, WCHAR ch);
        friend CString operator+(const CStringA& string1, const CString& string2);
        friend CString operator+(const CStringW& string1, const CString& string2);
        friend CString operator+(const CHAR* text, const CString& string1);
        friend CString operator+(const WCHAR* text, const CString& string1);
        friend CString operator+(CHAR ch, const CString& string1);
        friend CString operator+(WCHAR ch, const CString& string1);
        friend CString operator+(const CStringA& string1, const CStringW& string2);
        friend CString operator+(const CStringW& string1, const CStringA& string2);

        // Friend functions for CStringA global functions.
        friend CStringA operator+(const CStringA& string1, const CStringA& string2);
        friend CStringA operator+(const CStringA& string1, const CHAR* text);
        friend CStringA operator+(const CStringA& string1, CHAR ch);
        friend CStringA operator+(const CHAR* text, const CStringA& string1);
        friend CStringA operator+(CHAR ch, const CStringA& string1);

        // Friend functions for CStringW global functions.
        friend CStringW operator+(const CStringW& string1, const CStringW& string2);
        friend CStringW operator+(const CStringW& string1, const WCHAR* text);
        friend CStringW operator+(const CStringW& string1, WCHAR ch);
        friend CStringW operator+(const WCHAR* text, const CStringW& string1);
        friend CStringW operator+(WCHAR ch, const CStringW& string1);

        public:
        CStringT() = default;
        CStringT(const T* text);
        CStringT(const T* text, int length);
        CStringT(T ch, int repeat = 1);
        explicit CStringT(const std::string& str);
        explicit CStringT(const std::wstring& str);
        CStringT(const CStringT& str);
        CStringT(CStringT&& str) noexcept;
        ~CStringT() = default;

        CStringT& operator=(T ch);
        CStringT& operator=(const T* text);
        CStringT& operator=(const CStringT& str);
        CStringT& operator=(CStringT&& str) noexcept;

        bool     operator==(const T* text) const;
        bool     operator==(const CStringT& str) const;
        bool     operator!=(const T* text) const;
        bool     operator!=(const CStringT& str) const;
                 operator const T*() const;
        T&       operator[](int index);
        const T& operator[](int index) const;
        CStringT& operator+=(const CStringT& str);
        CStringT& operator+=(const T* text);
        CStringT& operator+=(T ch);

        // Accessors
        const T* c_str() const noexcept;
        const std::basic_string<T>& GetString() const;
        int GetLength() const;

        // Operations
        BSTR     AllocSysString() const;
        void     Append(const std::basic_string<T>& str);
        void     AppendFormat(const T* format,...);
        void     AppendFormat(UINT formatID, ...);
        void     Assign(const T* text, int count);
        int      Collate(const T* text) const;
        int      CollateNoCase(const T* text) const;
        int      Compare(const T* text) const;
        int      CompareNoCase(const T* text) const;
        int      Delete(int index, int count = 1);
        int      Find(T ch, int index = 0 ) const;
        int      Find(const T* text, int index = 0) const;
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

    private:
        std::basic_string<T> m_str;
    };


    ///////////////////////////////////////////////
    // A CString object provides a safer and a more
    // convenient alternative to an array of CHAR
    // or WCHAR when working with text strings.
    class CString : public CStringT<TCHAR>
    {
        // Global friend addition operators
        friend CString operator+(const CString& string1, const CString& string2);
        friend CString operator+(const CString& string1, const CStringA& string2);
        friend CString operator+(const CString& string1, const CStringW& string2);
        friend CString operator+(const CString& string1, const CHAR* text);
        friend CString operator+(const CString& string1, const WCHAR* text);
        friend CString operator+(const CString& string1, CHAR ch);
        friend CString operator+(const CString& string1, WCHAR ch);
        friend CString operator+(const CStringA& string1, const CString& string2);
        friend CString operator+(const CStringW& string1, const CString& string2);
        friend CString operator+(const CHAR* text, const CString& string1);
        friend CString operator+(const WCHAR* text, const CString& string1);
        friend CString operator+(CHAR ch, const CString& string1);
        friend CString operator+(WCHAR ch, const CString& string1);
        friend CString operator+(const CStringA& string1, const CStringW& string2);
        friend CString operator+(const CStringW& string1, const CStringA& string2);

    public:
        // Unhide base class overloads so assignments like:
        // myCString = myStdString or myCString += myStdString still compile.
        using CStringT<TCHAR>::operator=;
        using CStringT<TCHAR>::operator+=;

        // Constructors & Destructor.
        CString() = default;
        CString(const CString& str) : CStringT<TCHAR>(str) {}
        CString(CString&& str) noexcept : CStringT<TCHAR>(std::move(str)) {}
        ~CString() = default;

        // Cross-class conversion constructors.
        CString(const CStringA& str);
        CString(const CStringW& str);
        explicit CString(const std::string& str);
        explicit CString(const std::wstring& str);

        // Raw character string constructors.
        CString(LPCSTR text);
        CString(LPCWSTR text);
        CString(LPCSTR text, int length);
        CString(LPCWSTR text, int length);

        CString(CHAR ch, int repeat = 1);
        CString(WCHAR ch, int repeat = 1);

        // Derived assignment overrides.
        CString& operator=(const CString& str);
        CString& operator=(CString&& str) noexcept;
        CString& operator=(const CStringA& str);
        CString& operator=(const CStringW& str);
        CString& operator=(CHAR ch);
        CString& operator=(WCHAR ch);
        CString& operator=(LPCSTR text);
        CString& operator=(LPCWSTR text);

        // Derived addition-assignment overrides.
        CString& operator+=(const CString& str);
        CString& operator+=(const CStringA& str);
        CString& operator+=(const CStringW& str);
        CString& operator+=(LPCSTR text);
        CString& operator+=(LPCWSTR text);
        CString& operator+=(CHAR ch);
        CString& operator+=(WCHAR ch);
    };

} // namespace Win32xx

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

namespace Win32xx
{

    /////////////////////////////////////////////
    // Definition of the CStringT class template.
    //

    // Constructor. Assigns from from a const T* character array.
    template <class T>
    inline CStringT<T>::CStringT(const T* text)
    {
        assert(text != nullptr);
        m_str.assign(text);
    }

    // Constructor. Assigns from a const T* possibly containing null characters.
    template <class T>
    inline CStringT<T>::CStringT(const T* text, int length)
    {
        assert(length >= 0);
        assert(text != nullptr || length == 0);
        m_str.assign(text, static_cast<size_t>(length));
    }

    // Constructor. Assigns from 1 or more T characters.
    template <class T>
    inline CStringT<T>::CStringT(T ch, int repeat)
    {
        assert(repeat >= 0);
        m_str.assign(static_cast<size_t>(repeat), ch);
    }

    // Constructor. Assign from std::string.
    template <class T>
    inline CStringT<T>::CStringT(const std::string& str)
    {
        using CleanT = std::remove_cv_t<T>;
        if constexpr (std::is_same_v<CleanT, char> || std::is_same_v<CleanT, CHAR>)
        {
            m_str = str;
        }
        else // T is wchar_t (WCHAR).
        {
            int length = static_cast<int>(str.size());
            if (length > 0)
            {
                AtoT converted(str.c_str(), CP_ACP, length);
                m_str.assign(converted.c_str(), static_cast<size_t>(length));
            }
        }
    }

    // Constructor. Assign from std::wstring.
    template <class T>
    inline CStringT<T>::CStringT(const std::wstring& str)
    {
        using CleanT = std::remove_cv_t<T>;
        if constexpr (std::is_same_v<CleanT, char> || std::is_same_v<CleanT, CHAR>)
        {
            int length = static_cast<int>(str.size());
            if (length > 0)
            {
                WtoT converted(str.c_str(), CP_ACP, length);
                m_str.assign(converted.c_str(), static_cast<size_t>(length));
            }
        }
        else // T is wchar_t (WCHAR).
        {
            m_str = str;
        }
    }

    // Copy constructor.
    template <class T>
    inline CStringT<T>::CStringT(const CStringT& str) : m_str(str.m_str)
    {
    }

    // Move constructor.
    template <class T>
    inline CStringT<T>::CStringT(CStringT&& str) noexcept
        : m_str(std::move(str.m_str))
    {
    }

    // Assign from a const CStringT<T>.
    template <class T>
    inline CStringT<T>& CStringT<T>::operator=(const CStringT<T>& str)
    {
        if (this != &str)
            m_str.assign(str.m_str);
        return *this;
    }

    // Move assignment.
    template <class T>
    inline CStringT<T>& CStringT<T>::operator=(CStringT&& str) noexcept
    {
        if (this != &str)
            m_str = std::move(str.m_str);
        return *this;
    }

    // Assign from a T character.
    template <class T>
    inline CStringT<T>& CStringT<T>::operator=(T ch)
    {
        m_str.assign(1, ch);
        return *this;
    }

    // Assign from a const T* character array.
    template <class T>
    inline CStringT<T>& CStringT<T>::operator=(const T* text)
    {
        assert(text != nullptr);
        m_str.assign(text);
        return *this;
    }

    // Returns TRUE if the strings have the same content.
    template <class T>
    inline bool CStringT<T>::operator==(const T* text) const
    {
        assert(text != nullptr);
        return (Compare(text) == 0);
    }

    // Returns TRUE if the strings have the same content.
    // Can compare CStringTs containing null characters.
    template <class T>
    inline bool CStringT<T>::operator==(const CStringT& str) const
    {
        return m_str == str.m_str;
    }

    // Returns TRUE if the strings have a different content.
    template <class T>
    inline bool CStringT<T>::operator!=(const T* text) const
    {
        assert(text != nullptr);
        return Compare(text) != 0;
    }

    // Returns TRUE if the strings have a different content.
    // Can compares CStringTs containing null characters.
    template <class T>
    inline bool CStringT<T>::operator!=(const CStringT& str) const
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
    inline T& CStringT<T>::operator[](int index)
    {
        if (index < 0 || index >= static_cast<int>(m_str.size()))
            throw std::out_of_range("Index out of bounds");
        return m_str[static_cast<size_t>(index)];
    }

    // Subscript operator. Returns the T character at the specified index.
    template <class T>
    inline const T& CStringT<T>::operator[](int index) const
    {
        if (index < 0 || index >= static_cast<int>(m_str.size()))
           throw std::out_of_range("Index out of bounds");
        return m_str[static_cast<size_t>(index)];
    }

    // Addition assignment. Appends CStringT<T>.
    template <class T>
    inline CStringT<T>& CStringT<T>::operator+=(const CStringT& str)
    {
        m_str.append(str.m_str);
        return *this;
    }

    // Addition assignment. Appends const T* character array.
    template <class T>
    inline CStringT<T>& CStringT<T>::operator+=(const T* text)
    {
        assert(text != nullptr);
        m_str.append(text);
        return *this;
    }

    // Addition assignment. Appends a T character.
    template <class T>
    inline CStringT<T>& CStringT<T>::operator+=(T ch)
    {
        m_str.append(1, ch);
        return *this;
    }

    // Allocates a BSTR from the CStringT content.
    // Note: Free the returned string later with SysFreeString to avoid a
    // memory leak.
    template <class T>
    inline BSTR CStringT<T>::AllocSysString() const
    {
        BSTR bstr = nullptr;
        using CleanT = std::remove_cv_t<T>;

        if constexpr (std::is_same_v<CleanT, char> || std::is_same_v<CleanT, CHAR>)
        {
            // Pass the string data and exact size to retain embedded nulls
            AtoW wideStr(m_str.data(), CP_ACP, static_cast<int>(m_str.size()));
            const wchar_t* pWide = wideStr.c_str();

            // Allocate BSTR using the calculated length from m_str
            bstr = ::SysAllocStringLen(pWide, static_cast<UINT>(m_str.size()));
        }
        else // T is wchar_t (WCHAR).
        {
            const wchar_t* widePtr = reinterpret_cast<const wchar_t*>(m_str.data());
            bstr = ::SysAllocStringLen(widePtr, static_cast<UINT>(m_str.size()));
        }

        if (bstr == nullptr)
            throw std::runtime_error("AllocSysString failed");
        return bstr;
    }

    // Appends a std::basic_string<T> to this CStringT.
    template <class T>
    inline void CStringT<T>::Append(const std::basic_string<T>& str)
    {
        m_str.append(str);
    }

    // Appends formatted data to this CStringT.
    template <class T>
    inline void CStringT<T>::AppendFormat(const T* format,...)
    {
        assert(format != nullptr);
        CStringT str;

        va_list args;
        va_start(args, format);
        str.FormatV(format, args);
        va_end(args);
        m_str.append(str.m_str);
    }

    // Assigns the specified number of characters from the text array to this
    // string. The text array can contain null characters.
    template <class T>
    inline void CStringT<T>::Assign(const T* text, int count)
    {
        assert(count >= 0);
        assert(text != nullptr || count == 0);
        m_str.assign(text, static_cast<size_t>(count));
    }

    // Provides an alternative for casting to LPCTSTR.
    template <class T>
    inline const T* CStringT<T>::c_str() const noexcept
    {
        return m_str.c_str();
    }

    // Performs a case sensitive comparison of the two strings using
    // locale-specific information.
    template <class T>
    inline int CStringT<T>::Collate(const T* text) const
    {
        assert(text != nullptr);
        if (text == nullptr) return -1;

        int res = 0;
        using CleanT = std::remove_cv_t<T>;
        int textLen = static_cast<int>(std::char_traits<CleanT>::length(text));
        if constexpr (std::is_same_v<CleanT, char> || std::is_same_v<CleanT, CHAR>)
        {
            res = ::CompareStringA(LOCALE_USER_DEFAULT, 0,
                m_str.data(), static_cast<int>(m_str.size()),
                reinterpret_cast<const char*>(text), textLen);
        }
        else  // T is wchar_t (WCHAR).
        {
            res = ::CompareStringW(LOCALE_USER_DEFAULT, 0,
                reinterpret_cast<const wchar_t*>(m_str.data()), static_cast<int>(m_str.size()),
                reinterpret_cast<const wchar_t*>(text), textLen);
        }

        if (res == 0) throw std::runtime_error("Locale comparison failed");
        return res - 2; // Convert CompareString return values to: -1, 0, 1.
    }

    // Case-insensitive locale-specific comparison respecting embedded nulls.
    template <class T>
    inline int CStringT<T>::CollateNoCase(const T* text) const
    {
        assert(text != nullptr);
        if (text == nullptr) return -1;

        int res = 0;
        using CleanT = std::remove_cv_t<T>;
        int textLen = static_cast<int>(std::char_traits<CleanT>::length(text));
        if constexpr (std::is_same_v<CleanT, char> || std::is_same_v<CleanT, CHAR>)
        {
            res = ::CompareStringA(LOCALE_USER_DEFAULT, NORM_IGNORECASE,
                m_str.data(), static_cast<int>(m_str.size()),
                reinterpret_cast<const char*>(text), textLen);
        }
        else  // T is wchar_t (WCHAR).
        {
            res = ::CompareStringW(LOCALE_USER_DEFAULT, NORM_IGNORECASE,
                reinterpret_cast<const wchar_t*>(m_str.data()), static_cast<int>(m_str.size()),
                reinterpret_cast<const wchar_t*>(text), textLen);
        }

        if (res == 0) throw std::runtime_error("Locale comparison failed");
        return res - 2; // Convert CompareString return values to: -1, 0, 1.
    }

    // Performs a case sensitive comparison of the two strings.
    template <class T>
    inline int CStringT<T>::Compare(const T* text) const
    {
        assert(text != nullptr);
        return m_str.compare(text);
    }

    // Performs a binary-safe case insensitive comparison of the two strings.
    template <class T>
    inline int CStringT<T>::CompareNoCase(const T* text) const
    {
        assert(text != nullptr);
        if (text == nullptr) return -1;

        using CleanT = std::remove_cv_t<T>;
        int textLen = static_cast<int>(std::char_traits<CleanT>::length(text));

        // If lengths mismatch, they cannot be identical with embedded nulls
        int minLen = (static_cast<int>(m_str.size()) < textLen) ?
            static_cast<int>(m_str.size()) : textLen;

        int result = 0;
        if constexpr (std::is_same_v<CleanT, char> || std::is_same_v<CleanT, CHAR>)
        {
            result = ::_memicmp(m_str.data(), text, static_cast<size_t>(minLen));
        }
        else // T is wchar_t (WCHAR).
        {
            result = ::_wcsnicmp(reinterpret_cast<const wchar_t*>(m_str.data()),
                reinterpret_cast<const wchar_t*>(text), static_cast<size_t>(minLen));
        }

        if (result != 0) return result;

        // If prefixes matched, the longer string is greater
        if (static_cast<int>(m_str.size()) < textLen) return -1;
        if (static_cast<int>(m_str.size()) > textLen) return 1;
        return 0;
    }

    // Deletes a character or characters from the string.
    template <class T>
    inline int CStringT<T>::Delete(int index, int count /* = 1 */)
    {
        assert(index >= 0);
        assert(count >= 0);
        assert(index <= static_cast<int>(m_str.size()));

        if ((index >= 0) && (count >= 0) && (index < static_cast<int>(m_str.size())))
        {
            int maxCount = static_cast<int>(m_str.size()) - index;
            if (count > maxCount) count = maxCount;

            m_str.erase(static_cast<size_t>(index), static_cast<size_t>(count));
        }
        return static_cast<int>(m_str.size());
    }

    // Erases the contents of the string.
    template <class T>
    inline void CStringT<T>::Empty()
    {
        m_str.clear();
    }

    // Finds a character in the string, starting from the specified index.
    template <class T>
    inline int CStringT<T>::Find(T ch, int index /* = 0 */) const
    {
        assert(index >= 0);

        size_t pos = std::basic_string<T>::npos;
        if (index >= 0)
            pos = m_str.find(ch, static_cast<size_t>(index));
        return (pos != std::basic_string<T>::npos) ? static_cast<int>(pos) : -1;
    }

    // Finds a substring within the string, starting from the specified index.
    template <class T>
    inline int CStringT<T>::Find(const T* text, int index /* = 0 */) const
    {
        assert(text != nullptr);
        assert(index >= 0);

        size_t pos = std::basic_string<T>::npos;
        if ((text != nullptr) && (index >= 0))
            pos = m_str.find(text, static_cast<size_t>(index));
        return (pos != std::basic_string<T>::npos) ? static_cast<int>(pos) : -1;
    }

    // Finds the first matching character from a set.
    template <class T>
    inline int CStringT<T>::FindOneOf(const T* text) const
    {
        assert(text != nullptr);

        size_t pos = std::basic_string<T>::npos;
        if (text != nullptr)
            pos = m_str.find_first_of(text);
        return (pos != std::basic_string<T>::npos) ? static_cast<int>(pos) : -1;
    }

    // Formats the string as sprintf does.
    template <class T>
    inline void CStringT<T>::Format(const T* format, ...)
    {
        va_list args;
        va_start(args, format);
        FormatV(format, args);
        va_end(args);
    }

    // Formats the string using a variable list of arguments.
    template <class T>
    inline void CStringT<T>::FormatV(const T* format, va_list args)
    {
        assert(format != nullptr);
        if (format != nullptr)
        {
            int length = 0;
            va_list argsCopy;
            va_copy(argsCopy, args);
            using CleanT = std::remove_cv_t<T>;
            if constexpr (std::is_same_v<CleanT, char> || std::is_same_v<CleanT, CHAR>)
                length = ::vsnprintf(nullptr, 0, format, argsCopy);
            else // T is wchar_t (WCHAR).
                length = ::_vscwprintf(format, argsCopy);

            va_end(argsCopy);

            if (length > 0)
            {
                const size_t bufSize = static_cast<size_t>(length) + 1;
                m_str.resize(bufSize);
                if constexpr (std::is_same_v<CleanT, char> || std::is_same_v<CleanT, CHAR>)
                {
                    ::vsnprintf(m_str.data(), bufSize, format, args);
                }
                else // T is wchar_t (WCHAR).
                {
#if defined(_MSC_VER)  // For Microsoft compilers.
                    ::_vsnwprintf_s(m_str.data(),
                        bufSize,
                        static_cast<size_t>(length),
                        format, args);
#else
                    ::_vsnwprintf(m_str.data(),
                        bufSize,
                        format, args);
#endif
                }

                m_str.resize(static_cast<size_t>(length));
            }
            else
                m_str.clear();
        }
    }

    // Formats a message string.
    template <class T>
    inline void CStringT<T>::FormatMessage(const T* format, ...)
    {
        va_list args;
        va_start(args, format);
        FormatMessageV(format, args);
        va_end(args);
    }

    // Formats a message string using a variable argument list.
    template <class T>
    inline void CStringT<T>::FormatMessageV(const T* format, va_list args)
    {
        assert(format != nullptr);
        if (format != nullptr)
        {
            const DWORD flags = FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ALLOCATE_BUFFER;
            DWORD result = 0;

            using CleanT = std::remove_cv_t<T>;
            if constexpr (std::is_same_v<CleanT, char> || std::is_same_v<CleanT, CHAR>)
            {
                LPSTR temp = nullptr;
                result = ::FormatMessageA(flags, format, 0, 0,
                    reinterpret_cast<LPSTR>(&temp), 0, &args);

                if (result == 0 || temp == nullptr)
                    throw std::runtime_error("FormatMessageV failed");

                m_str.assign(temp, result);
                ::LocalFree(temp);
            }
            else // T is WCHAR (wchar_t).
            {
                LPWSTR temp = nullptr;
                result = ::FormatMessageW(flags, format, 0, 0,
                    reinterpret_cast<LPWSTR>(&temp), 0, &args);

                if (result == 0 || temp == nullptr)
                    throw std::runtime_error("FormatMessageV failed");

                m_str.assign(reinterpret_cast<T*>(temp), result);
                ::LocalFree(temp);
            }
        }
        else
            m_str.clear();
    }

    // Returns the character at the specified location within the string.
    template <class T>
    inline T CStringT<T>::GetAt(int index) const
    {
        assert(index >= 0);
        assert(index < GetLength());

        if ((index >= 0) && (index < static_cast<int>(m_str.size())))
            return m_str[static_cast<size_t>(index)];
        else
            throw std::out_of_range("Index out of bounds");
    }

    // Creates a buffer of minBufLength characters (+1 extra for null
    // termination) and returns a pointer to this buffer. This buffer can be
    // used by any function that accepts a LPTSTR.
    // Care must be taken not to exceed the length of the buffer. Use
    // ReleaseBuffer to safely copy this buffer back to the CStringT object.
    template <class T>
    inline T* CStringT<T>::GetBuffer(int minBufLength)
    {
        assert(minBufLength >= 0);
        if (minBufLength < 0) minBufLength = 0;

        m_str.resize(static_cast<size_t>(minBufLength) + 1);
        return m_str.data();
    }

    // Sets the string to the value of the specified environment variable.
    template <class T>
    inline bool CStringT<T>::GetEnvironmentVariable(const T* var)
    {
        assert(var != nullptr);
        if (var == nullptr) return false;

        using CleanT = std::remove_cv_t<T>;
        if constexpr (std::is_same_v<CleanT, char> || std::is_same_v<CleanT, CHAR>)
        {
            const char* ansiVar = reinterpret_cast<const char*>(var);
            DWORD sizeNeeded = ::GetEnvironmentVariableA(ansiVar, nullptr, 0);
            if (sizeNeeded > 0)
            {
                char* buffer = GetBuffer(static_cast<int>(sizeNeeded));
                ::GetEnvironmentVariableA(ansiVar, buffer, sizeNeeded);
                ReleaseBuffer(static_cast<int>(sizeNeeded - 1));
                return true;
            }
        }
        else // T is wchar_t (WCHAR).
        {
            const wchar_t* wideVar = reinterpret_cast<const wchar_t*>(var);
            DWORD sizeNeeded = ::GetEnvironmentVariableW(wideVar, nullptr, 0);
            if (sizeNeeded > 0)
            {
                wchar_t* buffer = GetBuffer(static_cast<int>(sizeNeeded));
                ::GetEnvironmentVariableW(wideVar, buffer, sizeNeeded);
                ReleaseBuffer(static_cast<int>(sizeNeeded - 1));
                return true;
            }
        }

        Empty();
        return false;
    }

    // Retrieves a window's text.
    template <class T>
    inline void CStringT<T>::GetWindowText(HWND wnd)
    {
        assert(::IsWindow(wnd));
        Empty();

        if (::IsWindow(wnd))
        {
            using CleanT = std::remove_cv_t<T>;
            if constexpr (std::is_same_v<CleanT, char> || std::is_same_v<CleanT, CHAR>)
            {
                int length = ::GetWindowTextLengthA(wnd);
                if (length > 0)
                {
                    char* buffer = GetBuffer(length);
                    ::GetWindowTextA(wnd, buffer, length + 1);
                    ReleaseBuffer(length);
                }
            }
            else // T is wchar_t (WCHAR).
            {
                int length = ::GetWindowTextLengthW(wnd);
                if (length > 0)
                {
                    wchar_t* buffer = GetBuffer(length);
                    ::GetWindowTextW(wnd, buffer, length + 1);
                    ReleaseBuffer(length);
                }
            }
        }
    }

    // Returns the error string for the specified System Error Code from GetLastError.
    template <class T>
    inline void CStringT<T>::GetErrorString(DWORD error)
    {
        Empty();
        wchar_t* buffer = nullptr;
        const DWORD flags = FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS;

        DWORD length = ::FormatMessageW(flags, nullptr, error, 0,
            reinterpret_cast<LPWSTR>(&buffer), 0, nullptr);

        using CleanT = std::remove_cv_t<T>;
        constexpr bool IsChar = std::is_same_v<CleanT, char> || std::is_same_v<CleanT, CHAR>;
        if (length > 0 && buffer != nullptr)
        {
            if constexpr (IsChar)
            {
                WtoA ansiBuffer(buffer);
                m_str.assign(ansiBuffer.c_str(), ansiBuffer.GetLength());
            }
            else // T is wchar_t
                m_str.assign(buffer, static_cast<size_t>(length));
            ::LocalFree(buffer);
        }
        else
        {
            if constexpr (IsChar)
                m_str.assign("Unknown System Error");
            else // T is wchar_t
                m_str.assign(L"Unknown System Error");
        }
    }

    // Returns the length in characters.
    template <class T>
    inline int CStringT<T>::GetLength() const
    {
        return static_cast<int>(m_str.length());
    }

    // Returns const reference to CString's internal std::basic_string<T>.
    template <class T>
    inline const std::basic_string<T>& CStringT<T>::GetString() const
    {
        return m_str;
    }

    // Inserts a single character at the given index within the string.
    template <class T>
    inline int CStringT<T>::Insert(int index, T ch)
    {
        assert(index >= 0);
        assert(index <= static_cast<int>(m_str.size()));

        int targetIndex = index;
        if (targetIndex < 0)
            targetIndex = 0;
        if (targetIndex > static_cast<int>(m_str.size()))
            targetIndex = static_cast<int>(m_str.size());

        m_str.insert(static_cast<size_t>(targetIndex), 1, ch);
        return static_cast<int>(m_str.size());
    }

    // Inserts a substring at the given index within the string.
    template <class T>
    inline int CStringT<T>::Insert(int index, const CStringT& str)
    {
        assert(index >= 0);
        assert(index <= static_cast<int>(m_str.size()));

        int targetIndex = index;
        if (targetIndex < 0) targetIndex = 0;
        if (targetIndex > static_cast<int>(m_str.size())) targetIndex = static_cast<int>(m_str.size());

        m_str.insert(static_cast<size_t>(targetIndex), str.m_str);
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
        if (count <= 0)
            return CStringT{};

        const size_t uCount = std::min(static_cast<size_t>(count), m_str.size());
        CStringT str;
        str.m_str.assign(m_str, 0, uCount);
        return str;
    }

    // Converts all the characters in this string to lowercase characters.
    template <class T>
    inline void CStringT<T>::MakeLower()
    {
        if (m_str.empty()) return;

        using CleanT = std::remove_cv_t<T>;
        if constexpr (std::is_same_v<CleanT, char> || std::is_same_v<CleanT, CHAR>)
            ::CharLowerBuffA(m_str.data(), static_cast<DWORD>(m_str.size()));
        else // T is wchar_t (WCHAR).
            ::CharLowerBuffW(reinterpret_cast<wchar_t*>(m_str.data()), static_cast<DWORD>(m_str.size()));
    }

    // Reverses the string.
    template <class T>
    inline void CStringT<T>::MakeReverse()
    {
        std::reverse(m_str.begin(), m_str.end());
    }

    // Converts all the characters in this string to uppercase characters.
    template <class T>
    inline void CStringT<T>::MakeUpper()
    {
        if (m_str.empty()) return;

        using CleanT = std::remove_cv_t<T>;
        if constexpr (std::is_same_v<CleanT, char> || std::is_same_v<CleanT, CHAR>)
            ::CharUpperBuffA(m_str.data(), static_cast<DWORD>(m_str.size()));
        else // T is wchar_t (WCHAR).
            ::CharUpperBuffW(reinterpret_cast<wchar_t*>(m_str.data()), static_cast<DWORD>(m_str.size()));
    }

    // Retrieves the middle part of a string.
    template <class T>
    inline CStringT<T> CStringT<T>::Mid(int first) const
    {
        return Mid(first, GetLength());
    }

    // Retrieves the middle part of a string.
    template <class T>
    inline CStringT<T> CStringT<T>::Mid(int first, int count) const
    {
        assert(first >= 0);
        assert(count >= 0);
        assert(first <= GetLength());

        CStringT str;
        if ((first >= 0) && (count > 0) && (first <= GetLength()))
        {
            size_t uFirst = static_cast<size_t>(first);
            size_t uCount = static_cast<size_t>(count);
            if (uFirst + uCount > m_str.size())
                uCount = m_str.size() - uFirst;

            str.m_str.assign(m_str, uFirst, uCount);
        }
        return str;
    }

    // This copies the contents of the buffer (acquired by GetBuffer) to this
    // CStringT. The default length of -1 copies from the buffer until a null
    // terminator is reached. If the buffer doesn't contain a null terminator,
    // you must specify the buffer's length.
    template <class T>
    inline void CStringT<T>::ReleaseBuffer(int newLength /*= -1*/)
    {
        if (newLength >= 0)
            m_str.resize(static_cast<size_t>(newLength));
        else
        {
            size_t actualLength = std::char_traits<T>::length(m_str.c_str());
            m_str.resize(actualLength);
        }
        m_str.shrink_to_fit();
    }

    // Removes each occurrence of the specified substring from the string.
    template <class T>
    inline int CStringT<T>::Remove(const T* text)
    {
        assert(text != nullptr);
        if (text == nullptr) return 0;

        size_t count = 0;
        size_t pos = 0;
        size_t len = std::char_traits<T>::length(text);
        if (len > 0)
        {
            while ((pos = m_str.find(text, pos, len)) != std::basic_string<T>::npos)
            {
                m_str.erase(pos, len);
                ++count;
            }
        }
        return static_cast<int>(count);
    }

    // Removes each occurrence of the specified character from the string.
    template <class T>
    inline int CStringT<T>::Remove(T ch)
    {
        size_t originalSize = m_str.size();
        auto newEnd = std::remove(m_str.begin(), m_str.end(), ch);
        m_str.erase(newEnd, m_str.end());
        return static_cast<int>(originalSize - m_str.size());
    }

    // Replaces each occurrence of the old character with the new character.
    template <class T>
    inline int CStringT<T>::Replace(T oldChar, T newChar)
    {
        if (oldChar == newChar) return 0;

        int count = 0;
        for (auto& ch : m_str)
        {
            if (ch == oldChar)
            {
                ch = newChar;
                ++count;
            }
        }
        return count;
    }

    // Replaces each occurrence of the old substring with the new substring.
    template <class T>
    inline int CStringT<T>::Replace(const T* oldText, const T* newText)
    {
        assert(oldText != nullptr);
        assert(newText != nullptr);
        if (oldText == nullptr || newText == nullptr) return 0;

        size_t count = 0;
        size_t pos = 0;
        size_t lenOld = std::char_traits<T>::length(oldText);
        size_t lenNew = std::char_traits<T>::length(newText);
        if (lenOld > 0)
        {
            while ((pos = m_str.find(oldText, pos, lenOld)) != std::basic_string<T>::npos)
            {
                m_str.replace(pos, lenOld, newText, lenNew);
                pos += (lenNew > 0) ? lenNew : 1;
                ++count;
            }
        }
        return static_cast<int>(count);
    }

    // Search for a character within the string, starting from the end.
    template <class T>
    inline int CStringT<T>::ReverseFind(T ch, int end /* = -1 */) const
    {
        size_t safeEnd = std::basic_string<T>::npos;
        if (end != -1 && end >= 0)
            safeEnd = std::min(static_cast<size_t>(end), m_str.size());

        size_t pos = m_str.rfind(ch, safeEnd);
        return (pos != std::basic_string<T>::npos) ? static_cast<int>(pos) : -1;
    }

    // Search for a substring within the string, starting from the end.
    template <class T>
    inline int CStringT<T>::ReverseFind(const T* text, int end /* = -1 */) const
    {
        assert(text != nullptr);
        if (text == nullptr) return -1;

        size_t safeEnd = std::basic_string<T>::npos;
        if (end != -1 && end >= 0)
            safeEnd = std::min(static_cast<size_t>(end), m_str.size());

        size_t len = std::char_traits<T>::length(text);

        // Pass the explicit token length so rfind checks through embedded null boundaries
        size_t pos = m_str.rfind(text, safeEnd, len);
        return (pos != std::basic_string<T>::npos) ? static_cast<int>(pos) : -1;
    }

    // Retrieves count characters from the right part of the string.
    template <class T>
    inline CStringT<T> CStringT<T>::Right(int count) const
    {
        assert(count >= 0);
        if (count <= 0) return CStringT{};

        CStringT str;
        const size_t uCount = std::min(static_cast<size_t>(count), m_str.size());
        str.m_str.assign(m_str, m_str.size() - uCount, uCount);
        return str;
    }

    // Sets the character at the specified position to the specified value.
    template <class T>
    inline void CStringT<T>::SetAt(int index, T ch)
    {
        assert(index >= 0);
        assert(index < GetLength());

        if ((index >= 0) && (index < static_cast<int>(m_str.size())))
            m_str[static_cast<size_t>(index)] = ch;
    }

    // Explicit specialization for SetSysString (CStringW), fully preserving embedded nulls.
    template <class T>
    inline BSTR CStringT<T>::SetSysString(BSTR* pBstr) const
    {
        assert(pBstr != nullptr);
        if (pBstr == nullptr) return nullptr;

        using CleanT = std::remove_cv_t<T>;
        if constexpr (std::is_same_v<CleanT, char> || std::is_same_v<CleanT, CHAR>)
        {
            AtoW wideStr(m_str.data(), static_cast<int>(m_str.size()));
            const wchar_t* pWide = wideStr;
            if (!::SysReAllocStringLen(pBstr, pWide, static_cast<UINT>(m_str.size())))
                throw std::runtime_error("SetSysString failed");
        }
        else // T is wchar_t (WCHAR).
        {
            if (!::SysReAllocStringLen(pBstr, reinterpret_cast<const wchar_t*>(m_str.data()), static_cast<UINT>(m_str.size())))
                throw std::runtime_error("SetSysString failed");
        }
        return *pBstr;
    }

    // Extracts characters from the string up to the first character matching the set.
    template <class T>
    inline CStringT<T> CStringT<T>::SpanExcluding(const T* text) const
    {
        assert(text != nullptr);
        if (text == nullptr) return CStringT{};

        size_t pos = m_str.find_first_of(text);
        if (pos == std::basic_string<T>::npos)
            return *this;

        CStringT str;
        str.m_str.assign(m_str, 0, pos);
        return str;
    }

    // Extracts a substring containing ONLY the characters inside the specified set.
    template <class T>
    inline CStringT<T> CStringT<T>::SpanIncluding(const T* text) const
    {
        assert(text != nullptr);
        if (text == nullptr) return CStringT{};

        // Find the FIRST character that is NOT in the set.
        size_t pos = m_str.find_first_not_of(text);
        if (pos == std::basic_string<T>::npos)
            return *this;

        CStringT str;
        str.m_str.assign(m_str, 0, pos);
        return str;
    }

    // Extracts specified tokens in a target string sequentially.
    template <class T>
    inline CStringT<T> CStringT<T>::Tokenize(const T* tokens, int& start) const
    {
        assert(tokens != nullptr);

        CStringT str;
        if (tokens == nullptr || start < 0 || static_cast<size_t>(start) > m_str.size())
        {
            start = -1;
            return str;
        }

        // Find the start of the actual token (skipping leading delimiters).
        size_t pos1 = m_str.find_first_not_of(tokens, static_cast<size_t>(start));

        if (pos1 != std::basic_string<T>::npos)
        {
            // Find the end of the token (the next delimiter character)
            size_t pos2 = m_str.find_first_of(tokens, pos1);

            if (pos2 == std::basic_string<T>::npos)
            {
                str.m_str = m_str.substr(pos1);
                start = -1; // Flag the absolute end of the string
            }
            else
            {
                str.m_str = m_str.substr(pos1, pos2 - pos1);
                start = static_cast<int>(pos2) + 1;
            }
        }
        else
            start = -1;  // No more tokens found.

        return str;
    }

    // Trims all leading and trailing white-space characters from the string.
    template <class T>
    inline void CStringT<T>::Trim()
    {
        TrimLeft();
        TrimRight();
    }

    // Trims leading white-space characters from the string.
    template <class T>
    inline void CStringT<T>::TrimLeft()
    {
        using CleanT = std::remove_cv_t<T>;
        if constexpr (std::is_same_v<CleanT, char> || std::is_same_v<CleanT, CHAR>)
        {
            auto nonSpaceIt = std::find_if_not(m_str.begin(), m_str.end(),
                [](unsigned char c) { return std::isspace(c); });

            m_str.erase(m_str.begin(), nonSpaceIt);
        }
        else // T is wchar_t (WCHAR).
        {
            auto nonSpaceIt = std::find_if_not(m_str.begin(), m_str.end(),
                [](wchar_t c) { return std::iswspace(static_cast<wint_t>(c)); });

            m_str.erase(m_str.begin(), nonSpaceIt);
        }
    }

    // Trims the specified character from the beginning of the string.
    template <class T>
    inline void CStringT<T>::TrimLeft(T target)
    {
        const size_t firstNonTarget = m_str.find_first_not_of(target);
        if (firstNonTarget != std::basic_string<T>::npos)
            m_str.erase(0, firstNonTarget);
        else
            m_str.clear(); // The string was made entirely of target characters.
    }

    // Trims the specified set of characters from the beginning of the string.
    template <class T>
    inline void CStringT<T>::TrimLeft(const T* targets)
    {
        assert(targets != nullptr);
        if (targets == nullptr || m_str.empty()) return;

        size_t len = std::char_traits<T>::length(targets);
        if (len == 0) return;

        const size_t firstNonTarget = m_str.find_first_not_of(targets, 0, len);
        if (firstNonTarget != std::basic_string<T>::npos)
            m_str.erase(0, firstNonTarget);
        else
            m_str.clear();
    }

    // Trims trailing white-space characters from the string.
    template <class T>
    inline void CStringT<T>::TrimRight()
    {
        if (m_str.empty()) return;

        using CleanT = std::remove_cv_t<T>;
        if constexpr (std::is_same_v<CleanT, char> || std::is_same_v<CleanT, CHAR>)
        {
            auto nonSpaceIt = std::find_if_not(m_str.rbegin(), m_str.rend(),
                [](unsigned char c) { return std::isspace(c); });
            m_str.erase(nonSpaceIt.base(), m_str.end());
        }
        else // T is wchar_t (WCHAR).
        {
            auto nonSpaceIt = std::find_if_not(m_str.rbegin(), m_str.rend(),
                [](wchar_t c) { return std::iswspace(static_cast<wint_t>(c)); });
            m_str.erase(nonSpaceIt.base(), m_str.end());
        }
    }

    // Trims the specified character from the end of the string.
    template <class T>
    inline void CStringT<T>::TrimRight(T target)
    {
        size_t pos = m_str.find_last_not_of(target);
        if (pos != std::basic_string<T>::npos)
            m_str.erase(++pos);
        else
            m_str.clear();
    }

    // Trims the specified set of characters from the end of the string.
    template <class T>
    inline void CStringT<T>::TrimRight(const T* targets)
    {
        assert(targets != nullptr);
        if (targets == nullptr || m_str.empty()) return;

        size_t len = std::char_traits<T>::length(targets);
        if (len == 0) return;

        size_t pos = m_str.find_last_not_of(targets, std::basic_string<T>::npos, len);
        if (pos != std::basic_string<T>::npos)
            m_str.erase(++pos);
        else
            m_str.clear();
    }

    // Reduces the length of the string to the specified amount.
    template <class T>
    inline void CStringT<T>::Truncate(int newLength)
    {
        assert(newLength >= 0);

        if ((newLength >= 0) && static_cast<size_t>(newLength) < m_str.size())
            m_str.resize(static_cast<size_t>(newLength));
    }

    /////////////////////////////
    // Global ToCString functions
    //

    // Convert the specified value to CStringA. Used by operator<<.
    template <class V>
    inline CStringA ToCStringA(const V& value)
    {
        std::basic_stringstream<CHAR> streamA;
        streamA << value;
        auto s = streamA.str();
        return CStringA(s.data(), static_cast<int>(s.length()));
    }

    // Convert CStringA to CStringA. Used by operator<<.
    inline CStringA ToCStringA(const CStringA& str)
    {
        return str;
    }

    // Convert the specified value to CStringW. Used by operator<<.
    template <class V>
    inline CStringW ToCStringW(const V& value)
    {
        std::basic_stringstream<WCHAR> streamW;
        streamW << value;
        auto s = streamW.str();
        return CStringW(s.data(), static_cast<int>(s.length()));
    }

    // Convert CStringW to CStringW. Used by operator<<.
    inline CStringW ToCStringW(const CStringW& str)
    {
        return str;
    }

    // Convert the specified value to CString. Used by operator<<.
    template <class V>
    inline CString ToCString(const V& value)
    {
#ifdef _UNICODE
        return ToCStringW(value);
#else
        return ToCStringA(value);
#endif
    }

    // Convert CStringA to CString. Used by operator<<.
    inline CString ToCString(const CStringA& str)
    {
#ifdef _UNICODE
        CAtoW wideStr(str.c_str(), CP_ACP, str.GetLength());
        return CStringW(wideStr.c_str(), str.GetLength());
#else
        return str;
#endif
    }

    // Convert CStringW to CString. Used by operator<<.
    inline CString ToCString(const CStringW& str)
    {
#ifdef _UNICODE
        return str;
#else
        CWtoA ansiStr(str.c_str(), CP_ACP, str.GetLength());
        return CStringA(ansiStr.c_str(), str.GetLength());
#endif
    }

    // Convert std::string to CString. Used by operator<<.
    inline CString ToCString(const std::string& str)
    {
#ifdef _UNICODE
        return CString(AtoW(str.c_str(), CP_ACP, static_cast<int>(str.size())), static_cast<int>(str.size()));
#else
        return CString(str.c_str(), static_cast<int>(str.size()));
#endif
     }

    // Convert std::stringw to CString. Used by operator<<.
    inline CString ToCString(const std::wstring& str)
    {
#ifdef _UNICODE
        return CString(str.c_str(), static_cast<int>(str.size()));
#else
        return CString(WtoA(str.c_str(), CP_ACP, static_cast<int>(str.size())), static_cast<int>(str.size()));
#endif
    }


    //////////////////////////////////////
    // CStringT global operator functions
    //

    // Addition operator: CStringA + CStringA
    inline CStringA operator+(const CStringA& string1, const CStringA& string2)
    {
        CStringA str(string1);
        str += string2;
        return str;
    }

    // Addition operator: CStringW + CStringW
    inline CStringW operator+(const CStringW& string1, const CStringW& string2)
    {
        CStringW str(string1);
        str += string2;
        return str;
    }

    // Addition operator: CStringA + const CHAR*
    inline CStringA operator+(const CStringA& string1, const CHAR* text)
    {
        assert(text != nullptr);
        CStringA str(string1);
        if (text != nullptr)
            str += text;
        return str;
    }

    // Addition operator: CStringW + const WCHAR*
    inline CStringW operator+(const CStringW& string1, const WCHAR* text)
    {
        assert(text != nullptr);
        CStringW str(string1);
        if (text != nullptr)
            str += text;
        return str;
    }

    // Addition operator: CStringA + CHAR
    inline CStringA operator+(const CStringA& string1, CHAR ch)
    {
        CStringA str(string1);
        str += ch;
        return str;
    }

    // Addition operator: CStringW + WCHAR
    inline CStringW operator+(const CStringW& string1, WCHAR ch)
    {
        CStringW str(string1);
        str += ch;
        return str;
    }

    // Addition operator: const CHAR* + CStringA
    inline CStringA operator+(const CHAR* text, const CStringA& string1)
    {
        assert(text != nullptr);
        CStringA str;
        if (text != nullptr)
        {
            str += text;
            str += string1;
        }
        return str;
    }

    // Addition operator: const WCHAR* + CStringW
    inline CStringW operator+(const WCHAR* text, const CStringW& string1)
    {
        assert(text != nullptr);
        CStringW str;
        if (text != nullptr)
        {
            str += text;
            str += string1;
        }
        return str;
    }

    // Addition operator: CHAR + CStringA
    inline CStringA operator+(CHAR ch, const CStringA& string1)
    {
        CStringA str;
        str += ch;
        str += string1;
        return str;
    }

    // Addition operator: WCHAR + CStringW
    inline CStringW operator+(WCHAR ch, const CStringW& string1)
    {
        CStringW str;
        str += ch;
        str += string1;
        return str;
    }

    // Performs a case sensitive comparison: CStringT < CStringT.
    template <class T>
    inline bool operator<(const CStringT<T>& string1, const CStringT<T>& string2)
    {
        return (string1.GetString() < string2.GetString());
    }

    // Performs a case sensitive comparison: CStringT > CStringT
    template <class T>
    inline bool operator>(const CStringT<T>& string1, const CStringT<T>& string2)
    {
        return (string1.GetString() > string2.GetString());
    }

    // Performs a case sensitive comparison: CStringT <= CStringT
    template <class T>
    inline bool operator<=(const CStringT<T>& string1, const CStringT<T>& string2)
    {
        return (string1.GetString() <= string2.GetString());
    }

    // Performs a case sensitive comparison: CStringT >= CStringT
    template <class T>
    inline bool operator>=(const CStringT<T>& string1, const CStringT<T>& string2)
    {
        return (string1.GetString() >= string2.GetString());
    }

    // --- Right-Hand Raw Pointer Overloads ---

    template <class T>
    inline bool operator<(const CStringT<T>& string1, const T* text)
    {
        assert(text != nullptr);
        if (text == nullptr) return false;
        return (string1.Compare(text) < 0);
    }

    template <class T>
    inline bool operator>(const CStringT<T>& string1, const T* text)
    {
        assert(text != nullptr);
        if (text == nullptr) return true;
        return (string1.Compare(text) > 0);
    }

    template <class T>
    inline bool operator<=(const CStringT<T>& string1, const T* text)
    {
        assert(text != nullptr);
        if (text == nullptr) return false;
        return (string1.Compare(text) <= 0);
    }

    template <class T>
    inline bool operator>=(const CStringT<T>& string1, const T* text)
    {
        assert(text != nullptr);
        if (text == nullptr) return true;
        return (string1.Compare(text) >= 0);
    }

    // --- Left-Hand Raw Pointer Overloads ---

    template <class T>
    inline bool operator<(const T* text, const CStringT<T>& string1)
    {
        assert(text != nullptr);
        if (text == nullptr) return true;
        return (string1.Compare(text) > 0); // Correctly mirrored conditional logic
    }

    template <class T>
    inline bool operator>(const T* text, const CStringT<T>& string1)
    {
        assert(text != nullptr);
        if (text == nullptr) return false;
        return (string1.Compare(text) < 0);
    }

    template <class T>
    inline bool operator<=(const T* text, const CStringT<T>& string1)
    {
        assert(text != nullptr);
        if (text == nullptr) return true;
        return (string1.Compare(text) >= 0);
    }

    template <class T>
    inline bool operator>=(const T* text, const CStringT<T>& string1)
    {
        assert(text != nullptr);
        if (text == nullptr) return false;
        return (string1.Compare(text) <= 0);
    }

    // --- CStringA Stream Insertion Operators ---

    // Appends the specified value to the string.
    template <class V, typename = std::enable_if_t<
        !std::is_same_v<std::decay_t<V>, CStringA> &&
        !std::is_same_v<std::decay_t<V>, CHAR>
        >>
        inline CStringA& operator<<(CStringA& str, const V& value)
    {
        str += ToCStringA(value);
        return str;
    }

    // Appends the specified character to the string.
    inline CStringA& operator<<(CStringA& str, CHAR ch)
    {
        str += ch;
        return str;
    }

    // Appends the specifed CStringA to the string.
    inline CStringA& operator<<(CStringA& str, const CStringA& value)
    {
        str += value;
        return str;
    }

    // --- CStringW Stream Insertion Operators ---

    // Appends the specified value to the string.
    template <class V, typename = std::enable_if_t<
        !std::is_same_v<std::decay_t<V>, CStringW> &&
        !std::is_same_v<std::decay_t<V>, WCHAR>
        >>
        inline CStringW& operator<<(CStringW& str, const V& value)
    {
        str += ToCStringW(value);
        return str;
    }

    // Appends the specified character to the string.
    inline CStringW& operator<<(CStringW& str, WCHAR ch)
    {
        str += ch;
        return str;
    }

    // Appends the specified CStringW to the string.
    inline CStringW& operator<<(CStringW& str, const CStringW& value)
    {
        str += value;
        return str;
    }

    ///////////////////////////////////
    // Definition of the CString class.
    //

    // Construct CString from CHAR characters.
    inline CString::CString(CHAR ch, int repeat) : CStringT<TCHAR>()
    {
        assert(repeat >= 0);
        if (repeat > 0)
        {
            AtoT converted(&ch, CP_ACP, 1);
            m_str.append(static_cast<size_t>(repeat), converted.c_str()[0]);
        }
    }

    // Construct CString from WChar characters.
    inline CString::CString(WCHAR ch, int repeat) : CStringT<TCHAR>()
    {
        assert(repeat >= 0);
        if (repeat > 0)
        {
            WtoT converted(&ch, CP_ACP, 1);
            m_str.append(static_cast<size_t>(repeat), converted.c_str()[0]);
        }
    }

    // Construct CString from CStringA.
    inline CString::CString(const CStringA& str) : CStringT<TCHAR>()
    {
        int length = str.GetLength();
        if (length > 0)
        {
            AtoT converted(str.c_str(), CP_ACP, length);
            m_str.assign(converted.c_str(), static_cast<size_t>(length));
        }
    }

    // Construct CString from CStringW.
    inline CString::CString(const CStringW& str) : CStringT<TCHAR>()
    {
        int length = str.GetLength();
        if (length > 0)
        {
            WtoT converted(str.c_str(), CP_ACP, length);
            m_str.assign(converted.c_str(), static_cast<size_t>(length));
        }
    }

    // Construct CString from std::string.
    inline CString::CString(const std::string& str)
    {
        int length = static_cast<int>(str.size());
        if (length > 0)
        {
            AtoT converted(str.c_str(), CP_ACP, length);
            m_str.assign(converted.c_str(), static_cast<size_t>(length));
        }
    }

    // Construct CString from std::wstring.
    inline CString::CString(const std::wstring& str)
    {
        int length = static_cast<int>(str.size());
        if (length > 0)
        {
            WtoT converted(str.c_str(), CP_ACP, length);
            m_str.assign(converted.c_str(), static_cast<size_t>(length));
        }
    }

    // Construct CString from CHAR character array.
    inline CString::CString(LPCSTR text) : CStringT<TCHAR>()
    {
        if (text != nullptr)
        {
            AtoT converted(text);
            size_t length = std::char_traits<TCHAR>::length(converted.c_str());
            m_str.assign(converted.c_str(), length);
        }
    }

    // Construct CString from WCHAR character array.
    inline CString::CString(LPCWSTR text) : CStringT<TCHAR>()
    {
        if (text != nullptr)
        {
            WtoT converted(text);
            size_t length = std::char_traits<TCHAR>::length(converted.c_str());
            m_str.assign(converted.c_str(), length);
        }
    }

    // Construct CString from CHAR character array of specified length.
    inline CString::CString(LPCSTR text, int length) : CStringT<TCHAR>()
    {
        assert(length >= 0);
        if (text != nullptr && length > 0)
        {
            AtoT converted(text, CP_ACP, length);
            m_str.assign(converted.c_str(), static_cast<size_t>(length));
        }
    }

    // Construct CString from WCHAR character array of specified length.
    inline CString::CString(LPCWSTR text, int length) : CStringT<TCHAR>()
    {
        assert(length >= 0);
        if (text != nullptr && length > 0)
        {
            WtoT converted(text, CP_ACP, length);
            m_str.assign(converted.c_str(), static_cast<size_t>(length));
        }
    }

    // Assignment operator.
    inline CString& CString::operator=(const CString& str)
    {
        if (this != &str)
            m_str.assign(str.GetString());
        return *this;
    }

    // Move Assignment.
    inline CString& CString::operator=(CString&& str) noexcept
    {
        if (this != &str)
            m_str = std::move(str.m_str);
        return *this;
    }

    // Assign CString from CStringA.
    inline CString& CString::operator=(const CStringA& str)
    {
        int length = str.GetLength();
        AtoT converted(str.c_str(), CP_ACP, length);
        m_str.assign(converted.c_str(), static_cast<size_t>(length));
        return *this;
    }

    // Assign CString from CStringW.
    inline CString& CString::operator=(const CStringW& str)
    {
        int length = str.GetLength();
        WtoT converted(str.c_str(), CP_ACP, length);
        m_str.assign(converted.c_str(), static_cast<size_t>(length));
        return *this;
    }

    // Assign CString from a CHAR character.
    inline CString& CString::operator=(CHAR ch)
    {
        AtoT converted(&ch, CP_ACP, 1);
        m_str.assign(1, converted.c_str()[0]);
        return *this;
    }

    // Assign CString from a WCHAR character.
    inline CString& CString::operator=(WCHAR ch)
    {
        WtoT converted(&ch, CP_ACP, 1);
        m_str.assign(1, converted.c_str()[0]);
        return *this;
    }

    // Assign CString from a CHAR array.
    inline CString& CString::operator=(LPCSTR text)
    {
        if (text != nullptr)
        {
            AtoT converted(text);
            size_t length = std::char_traits<TCHAR>::length(converted.c_str());
            m_str.assign(converted.c_str(), length);
        }
        else
            m_str.clear();
        return *this;
    }

    // Assign CString from a WCHAR array.
    inline CString& CString::operator=(LPCWSTR text)
    {
        if (text != nullptr)
        {
            WtoT converted(text);
            size_t length = std::char_traits<TCHAR>::length(converted.c_str());
            m_str.assign(converted.c_str(), length);
        }
        else
            m_str.clear();
        return *this;
    }

    // Append and assign from a CString.
    inline CString& CString::operator+=(const CString& str)
    {
        m_str.append(str.m_str);
        return *this;
    }

    // Append and assign from CStringA.
    inline CString& CString::operator+=(const CStringA& str)
    {
        int length = str.GetLength();
        if (length > 0)
        {
            AtoT converted(str.c_str(), CP_ACP, length);
            m_str.append(converted.c_str(), static_cast<size_t>(length));
        }
        return *this;
    }

    // Append and assign from CStringW.
    inline CString& CString::operator+=(const CStringW& str)
    {
        int length = str.GetLength();
        if (length > 0)
        {
            WtoT converted(str.c_str(), CP_ACP, length);
            m_str.append(converted.c_str(), static_cast<size_t>(length));
        }
        return *this;
    }

    // Append and assign from a CHAR array.
    inline CString& CString::operator+=(LPCSTR text)
    {
        if (text != nullptr)
        {
            AtoT converted(text);
            size_t length = std::char_traits<TCHAR>::length(converted.c_str());
            m_str.append(converted.c_str(), length);
        }
        return *this;
    }

    // Append and assign from a WCHAR array.
    inline CString& CString::operator+=(LPCWSTR text)
    {
        if (text != nullptr)
        {
            WtoT converted(text);
            size_t length = std::char_traits<TCHAR>::length(converted.c_str());
            m_str.append(converted.c_str(), length);
        }
        return *this;
    }

    // Append and assign from a CHAR character.
    inline CString& CString::operator+=(CHAR ch)
    {
        AtoT converted(&ch, CP_ACP, 1);
        m_str.append(1, *converted.c_str());
        return *this;
    }

    inline CString& CString::operator+=(WCHAR ch)
    {
        WtoT converted(&ch, CP_ACP, 1);
        m_str.append(1, *converted.c_str());
        return *this;
    }


    /////////////////////////////////////
    // CString global operator functions.
    //

    // Add a CString to a CString.
    inline CString operator+(const CString& string1, const CString& string2)
    {
        CString str(string1);
        str.m_str.append(string2.m_str);
        return str;
    }

    // Add a CStringA to a CString.
    inline CString operator+(const CString& string1, const CStringA& string2)
    {
        CString str(string1);
        int length = string2.GetLength();
        if (length > 0)
        {
            AtoT converted(string2.c_str(), CP_ACP, length);
            str.m_str.append(converted.c_str(), static_cast<size_t>(length));
        }
        return str;
    }

    // Add a CStringW to a CString.
    inline CString operator+(const CString& string1, const CStringW& string2)
    {
        CString str(string1);
        int length = string2.GetLength();
        if (length > 0)
        {
            WtoT converted(string2.c_str(), CP_ACP, length);
            str.m_str.append(converted.c_str(), static_cast<size_t>(length));
        }
        return str;
    }

    // Add a CHAR array to a CString.
    inline CString operator+(const CString& string1, const CHAR* text)
    {
        CString str(string1);
        if (text != nullptr)
        {
            AtoT converted(text);
            size_t length = std::char_traits<TCHAR>::length(converted.c_str());
            str.m_str.append(converted.c_str(), length);
        }
        return str;
    }

    // Add a WCHAR array to a CString.
    inline CString operator+(const CString& string1, const WCHAR* text)
    {
        CString str(string1);
        if (text != nullptr)
        {
            WtoT converted(text);
            size_t length = std::char_traits<TCHAR>::length(converted.c_str());
            str.m_str.append(converted.c_str(), length);
        }
        return str;
    }

    // Add a CHAR character to a CString.
    inline CString operator+(const CString& string1, CHAR ch)
    {
        CString str(string1);
        AtoT converted(&ch, CP_ACP, 1);
        str.m_str.append(1, *converted.c_str());
        return str;
    }

    // Add a WCHAR character to a CString.
    inline CString operator+(const CString& string1, WCHAR ch)
    {
        CString str(string1);
        WtoT converted(&ch, CP_ACP, 1);
        str.m_str.append(1, *converted.c_str());
        return str;
    }

    // Add a CString to a CStringA.
    inline CString operator+(const CStringA& string1, const CString& string2)
    {
        CString str(string1);
        str.m_str.append(string2.m_str);
        return str;
    }

    // Add a CString to a CStringW.
    inline CString operator+(const CStringW& string1, const CString& string2)
    {
        CString str(string1);
        str.m_str.append(string2.m_str);
        return str;
    }

    // Add a CString to a CHAR array.
    inline CString operator+(const CHAR* text, const CString& string1)
    {
        CString str(text);
        str.m_str.append(string1.m_str);
        return str;
    }

    // Add a CString to a WCHAR array.
    inline CString operator+(const WCHAR* text, const CString& string1)
    {
        CString str(text);
        str.m_str.append(string1.m_str);
        return str;
    }

    // Add a CString to a CHAR character.
    inline CString operator+(CHAR ch, const CString& string1)
    {
        CString str(ch);
        str.m_str.append(string1.m_str);
        return str;
    }

    // Add a CString to a WCHAR character.
    inline CString operator+(WCHAR ch, const CString& string1)
    {
        CString str(ch);
        str.m_str.append(string1.m_str);
        return str;
    }

    // Add a CStringW to a CStringA.
    inline CString operator+(const CStringA& string1, const CStringW& string2)
    {
        CString str(string1);
        int length = string2.GetLength();
        if (length > 0)
        {
            WtoT converted(string2.c_str(), CP_ACP, length);
            str.m_str.append(converted.c_str(), static_cast<size_t>(length));
        }
        return str;
    }

    // Add a CStringA to a CStringW.
    inline CString operator+(const CStringW& string1, const CStringA& string2)
    {
        CString str(string1);
        int length = string2.GetLength();
        if (length > 0)
        {
            AtoT converted(string2.c_str(), CP_ACP, length);
            str.m_str.append(converted.c_str(), static_cast<size_t>(length));
        }
        return str;
    }

    // Appends the specified CString.
    inline CString& operator<<(CString& string1, const CString& string2)
    {
        string1 += string2;
        return string1;
    }

    // Appends the specified CStringA.
    inline CString& operator<<(CString& str, const CStringA& value)
    {
        str += value;
        return str;
    }

    // Appends the specified CStringW.
    inline CString& operator<<(CString& str, const CStringW& value)
    {
        str += value;
        return str;
    }

    // Appends the specified numeric value (int, float, double, bool, etc.)
    // or std::string or std::wstring.
    template <class V, typename = std::enable_if_t<
        std::is_arithmetic_v<V> ||
        std::is_same_v<std::decay_t<V>, std::string> ||
        std::is_same_v<std::decay_t<V>, std::wstring>
        >>
    inline CString& operator<<(CString& str, V value)
    {
        str += ToCString(value);
        return str;
    }

    // Appends the specified mutable CHAR text array.
    inline CString& operator<<(CString& str, LPSTR text)
    {
        str += text;
        return str;
    }

    // Appends the specified mutable WCHAR text array.
    inline CString& operator<<(CString& str, LPWSTR text)
    {
        str += text;
        return str;
    }

    // Appends the specified const CHAR text array.
    inline CString& operator<<(CString& str, LPCSTR text)
    {
        str += text;
        return str;
    }

    // Appends the specified const WCHAR text array.
    inline CString& operator<<(CString& str, LPCWSTR text)
    {
        str += text;
        return str;
    }

    // Appends the specified CHAR character.
    inline CString& operator<<(CString& str, CHAR ch)
    {
        str += ch;
        return str;
    }

    // Appends the specified WCHAR character.
    inline CString& operator<<(CString& str, WCHAR ch)
    {
        str += ch;
        return str;
    }

}   // namespace Win32xx

#endif // WIN32XX_CSTRING_H_
