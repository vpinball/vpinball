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
// Acknowledgement:
//
// The original author of CArchive is:
//
//      Robert C. Tausworthe
//      email: robert.c.tausworthe@ieee.org
//
////////////////////////////////////////////////////////

////////////////////////////////////////////////////////
//  Contents Description:
//  The CArchive class is used to serialize and
//  deserialize data. Data is streamed to and from the
//  file specified by the user, using the >> and <<
//  operators.

// Note: This CArchive doesn't read archive written by MFC's CArchive.

//  Differences between this CArchive and MFC's CArchive
//   In this CArchive:
//    - LPCTSTR strings retrieved from the archive exactly match the string
//       saved to the archive. Strings can contain carriage return and line
//       feed characters. These characters are not stripped.
//    - CStrings retrieved from the archive exactly match the CString
//       saved in the archive. CStrings can contain any character, including
//       embedded null characters, line feeds and carriage returns.
//
//  In MFC's CArchive:
//    - LPCTSTR strings have carriage return characters stripped
//    - LPCTSTR strings are terminated by a \r\n characters.


#ifndef _WIN32XX_ARCHIVE_H_
#define _WIN32XX_ARCHIVE_H_

#include "wxx_wincore.h"
#include "wxx_file.h"


namespace Win32xx
{
    // An object used for serialization that can hold any type of data.
    // Specify a pointer to the data, and the size of the data in bytes.
    struct ArchiveObject
    {
        // member variables
        UINT    m_size;     // not size_t as that is different in x86 and x64
        LPVOID  m_pData;

        // Constructor
        ArchiveObject(LPVOID pData, UINT size) : m_size(size), m_pData(pData) {}
    };
    // A typical usage of ArchiveObject would be of the form:
    //
    //  ArchiveObject ao(&Data, sizeof(Data));
    //  ar << ao; or ar >> ao;


    //////////////////////////////////////////////////////////////
    // CArchive serializes data to and from a file archive.
    // CArchive uses the >> and << operator overloads to serialize
    // the various data types to the archive.
    class CArchive
    {
    public:
        // file modes
        enum Mode {store = 0, load = 1};

        // construction and  destruction
        CArchive(CFile& file, Mode mode);
        CArchive(LPCTSTR fileName, Mode mode);
        virtual ~CArchive();

        // method members
        const CFile&    GetFile();
        UINT    GetObjectSchema();
        bool    IsLoading() const;
        bool    IsStoring() const;
        void    Read(void* buffer, UINT size);
        LPTSTR  ReadString(LPTSTR string, UINT max);
        LPSTR   ReadStringA(LPSTR string, UINT max);
        LPWSTR  ReadStringW(LPWSTR string, UINT max);
        void    SetObjectSchema(UINT schema);
        void    Write(const void* buffer, UINT size);
        void    WriteString(LPCTSTR string);
        void    WriteStringA(LPCSTR string);
        void    WriteStringW(LPCWSTR string);

        // insertion operations
        CArchive& operator<<(BYTE by);
        CArchive& operator<<(WORD w);
        CArchive& operator<<(LONG l);
        CArchive& operator<<(LONGLONG ll);
        CArchive& operator<<(ULONGLONG ull);
        CArchive& operator<<(DWORD dw);
        CArchive& operator<<(float f);
        CArchive& operator<<(double d);
        CArchive& operator<<(int i);
        CArchive& operator<<(short w);
        CArchive& operator<<(char ch);
        CArchive& operator<<(unsigned u);
        CArchive& operator<<(bool b);
        CArchive& operator<<(const CStringA& string);
        CArchive& operator<<(const CStringW& string);
        CArchive& operator<<(const CString& string);
        CArchive& operator<<(const POINT& pt);
        CArchive& operator<<(const RECT& rc);
        CArchive& operator<<(const SIZE& sz);
        CArchive& operator<<(const ArchiveObject& ao);
        CArchive& operator<<(const CObject& object);
#if !defined (_MSC_VER) ||  ( _MSC_VER > 1310 )
        // wchar_t is not an a built-in type on older MS compilers
        CArchive& operator<<(wchar_t ch);
#endif

        // extraction operations
        CArchive& operator>>(BYTE& by);
        CArchive& operator>>(WORD& w);
        CArchive& operator>>(DWORD& dw);
        CArchive& operator>>(LONG& l);
        CArchive& operator>>(LONGLONG& ll);
        CArchive& operator>>(ULONGLONG& ull);
        CArchive& operator>>(float& f);
        CArchive& operator>>(double& d);
        CArchive& operator>>(int& i);
        CArchive& operator>>(short& w);
        CArchive& operator>>(char& ch);
        CArchive& operator>>(unsigned& u);
        CArchive& operator>>(bool& b);
        CArchive& operator>>(CStringA& string);
        CArchive& operator>>(CStringW& string);
        CArchive& operator>>(CString& string);
        CArchive& operator>>(POINT& pt);
        CArchive& operator>>(RECT& rc);
        CArchive& operator>>(SIZE& sz);
        CArchive& operator>>(ArchiveObject& ao);
        CArchive& operator>>(CObject& object);
#if !defined (_MSC_VER) ||  ( _MSC_VER > 1310 )
        // wchar_t is not an a built-in type on older MS compilers
        CArchive& operator>>(wchar_t& ch);
#endif


    private:
        CArchive(const CArchive&);              // Disable copy construction
        CArchive& operator = (const CArchive&); // Disable assignment operator

        // private data members
        CFile*  m_pFile;            // archive file FILE
        UINT    m_schema;           // archive version schema
        bool    m_isStoring;        // archive direction switch
        bool    m_isFileManaged;    // delete the CFile pointer in destructor;
    };

} // namespace Win32xx

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

namespace Win32xx
{

#if defined (_MSC_VER) && (_MSC_VER >= 1920) // >= VS2019
#pragma warning ( push )
#pragma warning ( disable : 26812 )          // enum type is unscoped.
#endif // (_MSC_VER) && (_MSC_VER >= 1920)

    // Constructs a CArchive object.
    // The specified file must already be open for loading or storing.
    inline CArchive::CArchive(CFile& file, CArchive::Mode mode) : m_schema(static_cast<UINT>(-1)), m_isFileManaged(false)
    {
        m_pFile = &file;

        if (mode == load)
        {
            m_isStoring = false;
        }
        else
        {
            m_isStoring = true;
        }
    }

    // Constructs a CArchive object.
    // A file with the specified name is created for storing (if required), and
    // also opened. A failure to open the file will throw an exception.
    inline CArchive::CArchive(LPCTSTR fileName, Mode mode) : m_pFile(0), m_schema(static_cast<UINT>(-1))
    {
        m_isFileManaged = true;

        try
        {
            if (mode == load)
            {
                // Open the archive for loading
                m_pFile = new CFile(fileName, CFile::modeRead);
                m_isStoring = false;
            }
            else
            {
                // Open the archive for storing. Creates file if required
                m_pFile = new CFile(fileName, CFile::modeCreate);
                m_isStoring = true;
            }
        }

        catch(...)
        {
            delete m_pFile;
            throw; // Rethrow the exception
        }
    }

    inline CArchive::~CArchive()
    {
        if (m_pFile)
        {
            // if the file is open
            if (m_pFile->GetHandle())
            {
                // flush if in write mode
                if (IsStoring())
                    m_pFile->Flush();

                m_pFile->Close();
            }

            if (m_isFileManaged)
            {
                delete m_pFile;
            }
        }
    }

    // Returns the file associated with the archive.
    inline const CFile& CArchive::GetFile()
    {
        assert(m_pFile);
        return *m_pFile;
    }

    // Returns the archived data schema. This acts as a version number on
    // the format of the archived data for special handling when there
    // are several versions of the serialized data to be accommodated
    // by the application.
    inline UINT CArchive::GetObjectSchema()
    {
        return m_schema;
    }

    // Returns the current sense of serialization, true if the archive is
    // being loaded.
    inline bool CArchive::IsLoading() const
    {
         return !m_isStoring;
    }

    // Returns the current sense of serialization, true if the archive is
    // being stored.
    inline bool CArchive::IsStoring() const
    {
        return m_isStoring;
    }

    // Reads size bytes from the open archive file into the given buffer.
    // Throws an exception if not successful.
    inline void CArchive::Read(void* buffer, UINT size)
    {
        // read, simply and  in binary mode, the size into the buffer
        assert(m_pFile);

        if (m_pFile)
        {
            UINT nBytes = m_pFile->Read(buffer, size);
            if (nBytes != size)
                throw CFileException(m_pFile->GetFilePath(), GetApp()->MsgArReadFail());
        }
    }

    // Records the archived data schema number.  This acts as a version number
    // on the format of the archived data for special handling when there
    // are several versions of the serialized data to be accommodated
    // by the application.
    inline void CArchive::SetObjectSchema(UINT schema)
    {
        m_schema = schema;
    }

    // Writes size characters of from the buffer into the open archive file.
    // Throws an exception if unsuccessful.
    inline void CArchive::Write(const void* buffer, UINT size)
    {
        // write size characters in buffer to the  file
        assert(m_pFile);
        m_pFile->Write(buffer, size);
    }

    // Writes the BYTE b into the archive file.
    // Throws an exception if an error occurs.
    inline CArchive& CArchive::operator<<(BYTE b)
    {
        ArchiveObject ob(&b, sizeof(b));
        *this << ob;
        return *this;
    }

    // Writes the WORD w into the archive file.
    // Throws an exception if an error occurs.
    inline CArchive& CArchive::operator<<(WORD w)
    {
        ArchiveObject ob(&w, sizeof(w));
        *this << ob;
        return *this;
    }

    // Writes the LONG l into the archive file.
    // Throws an exception if an error occurs.
    inline CArchive& CArchive::operator<<(LONG l)
    {
        ArchiveObject ob(&l, sizeof(l));
        *this << ob;
        return *this;
    }

    // Writes the LONGLONG ll into the archive file.
    // Throw an exception if an error occurs.
    inline CArchive& CArchive::operator<<(LONGLONG ll)

    {
        ArchiveObject ob(&ll, sizeof(ll));
        *this << ob;
        return *this;
    }

    // Writes the ULONGLONG ull into the archive file.
    // Throws an exception if an error occurs.
    inline CArchive& CArchive::operator<<(ULONGLONG ull)
    {
        ArchiveObject ob(&ull, sizeof(ull));
        *this << ob;
        return *this;
    }

    // Writes the DWORD dw into the archive file.
    // Throws an exception if an error occurs.
    inline CArchive& CArchive::operator<<(DWORD dw)
    {
        ArchiveObject ob(&dw, sizeof(dw));
        *this << ob;
        return *this;
    }

    // Writes the float f into the archive file.
    // Throws an exception if an error occurs.
    inline CArchive& CArchive::operator<<(float f)
    {
        ArchiveObject ob(&f, sizeof(f));
        *this << ob;
        return *this;
    }

    // Writes the double d into the archive file.
    // Throws an exception if an error occurs.
    inline CArchive& CArchive::operator<<(double d)
    {
        ArchiveObject ob(&d, sizeof(d));
        *this << ob;
        return *this;
    }

    // Writes the int i into the archive file.
    // Throws an exception if an error occurs.
    inline CArchive& CArchive::operator<<(int i)
    {
        ArchiveObject ob(&i, sizeof(i));
        *this << ob;
        return *this;
    }

    // Writes the short s into the archive file.
    // Throws an exception if an error occurs.
    inline CArchive& CArchive::operator<<(short s)
    {
        ArchiveObject ob(&s, sizeof(s));
        *this << ob;
        return *this;
    }

    // Writes the char c into the archive file.
    // Throws an exception if an error occurs.
    inline CArchive& CArchive::operator<<(char c)
    {
        ArchiveObject ob(&c, sizeof(c));
        *this << ob;
        return *this;
    }

// wchar_t is not an a built-in type on older MS compilers
#if !defined (_MSC_VER) ||  ( _MSC_VER > 1310 )

    // Writes the wchar_t ch into the archive file.
    // Throws an exception if an error occurs.
    inline CArchive& CArchive::operator<<(wchar_t ch)
    {
        ArchiveObject ob(&ch, sizeof(ch));
        *this << ob;
        return *this;
    }

#endif

    // Writes the unsigned u into the archive file.
    // Throws an exception if an error occurs.
    inline CArchive& CArchive::operator<<(unsigned u)
    {
        ArchiveObject ob(&u, sizeof(u));
        *this << ob;
        return *this;
    }

    // Writes the bool b into the archive file.
    // Throws an exception if an error occurs.
    inline CArchive& CArchive::operator<<(bool b)
    {
        ArchiveObject ob(&b, sizeof(b));
        *this << ob;
        return *this;
    }

    // Writes the CStringA string into the archive file.
    // The CStringA can contain any characters including embedded nulls.
    // Throws an exception if an error occurs.
    inline CArchive& CArchive::operator<<(const CStringA& string)
    {
        UINT chars = string.GetLength();
        bool isUnicode = false;

        // Store the Unicode state and number of characters in the archive
        *this << isUnicode;
        *this << chars;

        Write(string.c_str(), chars*sizeof(CHAR));
        return *this;
    }

    // Writes the CStringW string into the archive file.
    // The CStringW can contain any characters including embedded nulls.
    // Throws an exception if an error occurs.
    inline CArchive& CArchive::operator<<(const CStringW& string)
    {
        UINT chars = string.GetLength();
        bool isUnicode = true;

        // Store the Unicode state and number of characters in the archive
        *this << isUnicode;
        *this << chars;

        Write(string.c_str(), chars*sizeof(WCHAR));
        return *this;
    }

    // Writes the CString string into the archive file.
    // The CString can contain any characters including embedded nulls.
    // Throws an exception if an error occurs.
    inline CArchive& CArchive::operator<<(const CString& string)
    {
        UINT chars = string.GetLength();
        bool isUnicode = (sizeof(TCHAR) == sizeof(WCHAR));

        // Store the Unicode state and number of characters in the archive
        *this << isUnicode;
        *this << chars;

        Write(string.c_str(), chars*sizeof(TCHAR));
        return *this;
    }

    // Writes the POINT pt into the archive file.
    // Throws an exception if an error occurs.
    inline CArchive& CArchive::operator<<(const POINT& pt)
    {
        UINT size = sizeof(pt);

        // Write() throws exception upon error
        Write(&size, sizeof(size));
        Write(&pt, size);
        return *this;
    }

    // Writes the RECT rc into the archive file.
    // Throws an exception if an error occurs.
    inline CArchive& CArchive::operator<<(const RECT& rc)
    {
        UINT size = sizeof(rc);

        // Write() throws exception upon error
        Write(&size, sizeof(size));
        Write(&rc, size);
        return *this;
    }

    // Writes the SIZE sz into the archive file.
    // Throws an exception if an error occurs.
    inline CArchive& CArchive::operator<<(const SIZE& sz)
    {
        UINT size = sizeof(sz);

        // Write() throws exception upon error
        Write(&size, sizeof(size));
        Write(&sz, size);
        return *this;
    }

    // Writes arbitrary CArchive object into this CArchive as bytes.
    // Throws an exception if an error occurs..
    inline CArchive& CArchive::operator<<(const ArchiveObject& ao)
    {
        Write(&ao.m_size, sizeof(ao.m_size));

        // Write() throws exception upon error
        Write(ao.m_pData, ao.m_size);
        return *this;
    }

    // Reads a BYTE from the archive and stores it in b.
    // Throws an exception if an error occurs.
    inline CArchive& CArchive::operator>>(BYTE& b)
    {
        ArchiveObject ob(&b, sizeof(b));
        *this >> ob;
        return *this;
    }

    // Reads a WORD from the archive and stores it in w.
    // Throws an exception if an error occurs.
    inline CArchive& CArchive::operator>>(WORD& w)
    {
        ArchiveObject ob(&w, sizeof(w));
        *this >> ob;
        return *this;
    }

    // Reads a LONG from the archive and stores it in l.
    // Throws an exception if an error occurs.
    inline CArchive& CArchive::operator>>(LONG& l)
    {
        ArchiveObject ob(&l, sizeof(l));
        *this >> ob;
        return *this;
    }

    // Reads a LONGLONG from the archive and stores it in ll.
    // Throws an exception if an error occurs.
    inline CArchive& CArchive::operator>>(LONGLONG& ll)
    {
        ArchiveObject ob(&ll, sizeof(ll));
        *this >> ob;
        return *this;
    }

    // Reads a ULONGLONG from the archive and stores it in ull.
    // Throws an exception if an error occurs.
    inline CArchive& CArchive::operator>>(ULONGLONG& ull)
    {
        ArchiveObject ob(&ull, sizeof(ull));
        *this >> ob;
        return *this;
    }

    // Reads a DWORD from the archive and stores it in dw.
    // Throws an exception if an error occurs.
    inline CArchive& CArchive::operator>>(DWORD& dw)
    {
        ArchiveObject ob(&dw, sizeof(dw));
        *this >> ob;
        return *this;
    }

    // Reads a float from the archive and stores it in f.
    // Throws an exception if an error occurs.
    inline CArchive& CArchive::operator>>(float& f)
    {
        ArchiveObject ob(&f, sizeof(f));
        *this >> ob;
        return *this;
    }

    // Reads a double from the archive and stores it in d.
    // Throws an exception if an error occurs.
    inline CArchive& CArchive::operator>>(double& d)
    {
        ArchiveObject ob(&d, sizeof(d));
        *this >> ob;
        return *this;
    }

    // Reads an int from the archive and stores it in i.
    // Throws an exception if an error occurs.
    inline CArchive& CArchive::operator>>(int& i)
    {
        ArchiveObject ob(&i, sizeof(i));
        *this >> ob;
        return *this;
    }

    // Reads a short from the archive and stores it in i.
    // Throws an exception if an error occurs.
    inline CArchive& CArchive::operator>>(short& i)
    {
        ArchiveObject ob(&i, sizeof(i));
        *this >> ob;
        return *this;
    }

    // Reads a char from the archive and  store it in c.
    // Throws an exception if an error occurs.
    inline CArchive& CArchive::operator>>(char& c)
    {
        ArchiveObject ob(&c, sizeof(c));
        *this >> ob;
        return *this;
    }

// wchar_t is not an a built-in type on older MS compilers
#if !defined (_MSC_VER) ||  ( _MSC_VER > 1310 )

    // Reads a wchar_t from the archive and stores it in ch.
    // Throws an exception if an error occurs.
    inline CArchive& CArchive::operator>>(wchar_t& ch)

    {
        ArchiveObject ob(&ch, sizeof(ch));
        *this >> ob;
        return *this;
    }

#endif

    // Reads an unsigned int from the archive and stores it in u.
    // Throws an exception if an error occurs.
    inline CArchive& CArchive::operator>>(unsigned& u)
    {
        ArchiveObject ob(&u, sizeof(u));
        *this >> ob;
        return *this;
    }

    // Reads an bool from the archive and stores it in b.
    // Throws an exception if an error occurs.
    inline CArchive& CArchive::operator>>(bool& b)
    {
        ArchiveObject ob(&b, sizeof(b));
        *this >> ob;
        return *this;
    }

    // Reads a CString from the archive and stores it in string.
    // Throws an exception if an error occurs.
    inline CArchive& CArchive::operator>>(CStringA& string)
    {
        bool isUnicode;
        int chars;

        // Retrieve the Unicode state and number of characters from the archive
        *this >> isUnicode;
        *this >> chars;

        if (isUnicode)
            throw CFileException(m_pFile->GetFilePath(), GetApp()->MsgArNotCStringA());

        Read(string.GetBuffer(chars), chars);
        string.ReleaseBuffer(chars);

        return *this;
    }

    // Reads a CStringW from the archive and stores it in string.
    // Throws an exception if an error occurs.
    inline CArchive& CArchive::operator>>(CStringW& string)
    {
        bool isUnicode;
        int chars;

        // Retrieve the Unicode state and number of characters from the archive
        *this >> isUnicode;
        *this >> chars;

        if (!isUnicode)
            throw CFileException(m_pFile->GetFilePath(), GetApp()->MsgArNotCStringW());

        Read(string.GetBuffer(chars), chars * 2);
        string.ReleaseBuffer(chars);

        return *this;
    }

    // Reads a CString from the archive and stores it in string.
    // Throws an exception if an error occurs.
    inline CArchive& CArchive::operator>>(CString& string)
    {
        bool isUnicode;
        int chars;

        // Retrieve the Unicode state and number of characters from the archive
        *this >> isUnicode;
        *this >> chars;

        if (isUnicode)
        {
            // use a vector to create our WCHAR array
            std::vector<WCHAR> vWChar(size_t(chars) + 1, L'\0');
            WCHAR* buf = &vWChar.front();

            Read(buf, chars*2);

#ifdef UNICODE
            memcpy(string.GetBuffer(chars), buf, size_t(chars)*2);
#else
            // Convert the archive string from Wide to Ansi
            WideCharToMultiByte(CP_ACP, 0, buf, chars, string.GetBuffer(chars), chars, NULL, NULL);
#endif

            string.ReleaseBuffer(chars);
        }
        else
        {
            // use a vector to create our char array
            std::vector<char> vChar(size_t(chars) + 1, '\0');
            char* buf = &vChar.front();

            Read(buf, int(chars));

#ifdef UNICODE
            // Convert the archive string from Ansi to Wide
            MultiByteToWideChar(CP_ACP, 0, buf, chars, string.GetBuffer(chars), int(chars));
#else
            memcpy(string.GetBuffer(chars), buf, chars);
#endif

            string.ReleaseBuffer(int(chars));
        }

        return *this;
    }

    // Reads a POINT from the archive and stores it in pt.
    // Throws an exception if an error occurs.
    inline CArchive& CArchive::operator>>(POINT& pt)
    {
        ArchiveObject ob(&pt, sizeof(pt));
        *this >> ob;
        return *this;
    }

    // Reads a RECT from the archive and stores it in rc.
    // Throws an exception if an error occurs.
    inline CArchive& CArchive::operator>>(RECT& rc)
    {
        ArchiveObject ob(&rc, sizeof(rc));
        *this >> ob;
        return *this;
    }

    // Reads a SIZE from the archive and store it in sz.
    // Throws an exception if an error occurs.
    inline CArchive& CArchive::operator>>(SIZE& sz)
    {
        ArchiveObject ob(&sz, sizeof(sz));
        *this >> ob;
        return *this;
    }

    // Reads an arbitrary CArchive object into this CArchive as bytes.
    // Throws an exception if an error occurs.
    inline CArchive& CArchive::operator>>(ArchiveObject& ao)
    {
        assert(m_pFile);
        UINT size;
        Read(&size, sizeof(size));
        if (size != ao.m_size)
        {
            throw CFileException(m_pFile->GetFilePath(), GetApp()->MsgArReadFail());
        }

        Read(ao.m_pData, ao.m_size);
        return *this;
    }

    // Writes a CObject to the archive.
    // Throws an exception if an error occurs.
    inline CArchive& CArchive::operator<<(const CObject& object)
    {
        ((CObject&)object).Serialize(*this);
        return *this;
    }

    // Reads a CObject from the archive.
    // Throws an exception if an error occurs.
    inline CArchive& CArchive::operator>>(CObject& object)
    {
        object.Serialize(*this);
        return *this;
    }

    // The size (in characters) of the string array must be max or greater.
    // Reads at most max-1 TCHAR characters from the archive and stores it
    // in a string. Strings read from the archive are converted from ANSI
    // or Unicode to TCHAR if required, and are NULL terminated.
    // Throws an exception if an error occurs.
    inline LPTSTR CArchive::ReadString(LPTSTR string, UINT max)
    {
        assert (max > 0);

        CString str;
        *this >> str;
        StrCopy(string, str.c_str(), max);
        return string;
    }

    // The size (in characters) of the string array must be max or greater.
    // Reads at most max-1 TCHAR characters from the archive and store it
    // in a string. Strings read from the archive are converted from ANSI
    // or Unicode to TCHAR if required, and are NULL terminated.
    // Throws an exception if an error occurs.
    inline LPSTR CArchive::ReadStringA(LPSTR string, UINT max)
    {
        assert (max > 0);

        CStringA str;
        *this >> str;
        StrCopyA(string, str.c_str(), max);
        return string;
    }

    // The size (in characters) of the string array must be max or greater.
    // Reads at most max-1 TCHAR characters from the archive and store it
    // in a string. Strings read from the archive are converted from ANSI
    // or Unicode to TCHAR if required, and are NULL terminated.
    // Throws an exception if an error occurs.
    inline LPWSTR CArchive::ReadStringW(LPWSTR string, UINT max)
    {
        assert (max > 0);

        CStringW str;
        *this >> str;
        StrCopyW(string, str.c_str(), max);
        return string;
    }

    // Writes the LPCTSTR string into the archive file.
    // The string must be null terminated.
    // Throws an exception if an error occurs.
    inline void CArchive::WriteString(LPCTSTR string)
    {
        int chars = lstrlen(string);
        bool isUnicode = (sizeof(TCHAR) == sizeof(WCHAR));

        // Store the Unicode state and number of characters in the archive
        *this << isUnicode;
        *this << chars;

        Write(string, chars*sizeof(TCHAR));
    }

    // Writes the LPCSTR string into the archive file.
    // The string must be null terminated.
    // Throws an exception if an error occurs.
    inline void CArchive::WriteStringA(LPCSTR string)
    {
        int chars = lstrlenA(string);
        bool isUnicode = false;

        // Store the Unicode state and number of characters in the archive
        *this << isUnicode;
        *this << chars;

        Write(string, chars*sizeof(CHAR));
    }

    // Writes the LPCWSTR string into the archive file.
    // The string must be null terminated.
    // Throws an exception if an error occurs.
    inline void CArchive::WriteStringW(LPCWSTR string)
    {
        int chars = lstrlenW(string);
        bool isUnicode = true;

        // Store the Unicode state and number of characters in the archive
        *this << isUnicode;
        *this << chars;

        Write(string, chars*sizeof(WCHAR));
    }

#if defined (_MSC_VER) && (_MSC_VER >= 1920)       // >= VS2019
#pragma warning ( pop )  // ( disable : 26812 )    enum type is unscoped.
#endif // (_MSC_VER) && (_MSC_VER >= 1920)

} // namespace Win32xx


#endif // _WIN32XX_ARCHIVE_H_
