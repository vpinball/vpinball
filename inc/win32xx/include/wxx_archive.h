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

#include "wxx_appcore0.h"
#include "wxx_cstring.h"
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


    // The CArchive serializes data to and from a file archive.
    // CArchive uses the >> and << operator overloads to serialize
    // the various data types to the archive.
    class CArchive
    {
    public:
        // file modes
        enum mode {store = 0, load = 1};

        // construction and  destruction
        CArchive(CFile& File, mode Mode);
        CArchive(LPCTSTR FileName, mode nMode);
        ~CArchive();

        // method members
        const CFile&    GetFile();
        UINT    GetObjectSchema();
        bool    IsLoading() const;
        bool    IsStoring() const;
        void    Read(void* lpBuf, UINT size);
        LPTSTR  ReadString(LPTSTR szString, UINT nMax);
        LPSTR   ReadStringA(LPSTR szString, UINT nMax);
        LPWSTR  ReadStringW(LPWSTR szString, UINT nMax);
        void    SetObjectSchema(UINT nSchema);
        void    Write(const void* lpBuf, UINT size);
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
        CArchive& operator<<(const CString& s);
        CArchive& operator<<(const POINT& pt);
        CArchive& operator<<(const RECT& rc);
        CArchive& operator<<(const SIZE& sz);
        CArchive& operator<<(const ArchiveObject& ao);
        CArchive& operator<<(const CObject& Ob);
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
        CArchive& operator>>(CStringA& s);
        CArchive& operator>>(CStringW& s);
        CArchive& operator>>(CString& s);
        CArchive& operator>>(POINT& pt);
        CArchive& operator>>(RECT& rc);
        CArchive& operator>>(SIZE& sz);
        CArchive& operator>>(ArchiveObject& ao);
        CArchive& operator>>(CObject& Ob);
#if !defined (_MSC_VER) ||  ( _MSC_VER > 1310 )
        // wchar_t is not an a built-in type on older MS compilers
        CArchive& operator>>(wchar_t& ch);
#endif


    private:
        CArchive(const CArchive&);              // Disable copy construction
        CArchive& operator = (const CArchive&); // Disable assignment operator

        // private data members
        CFile*  m_pFile;            // archive file FILE
        bool    m_IsStoring;        // archive direction switch
        UINT    m_Schema;           // archive version schema
        bool    m_IsFileManaged;    // delete the CFile pointer in destructor;

    };


} // namespace Win32xx

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

namespace Win32xx
{
    // Constructs a CArchive object.
    // The specified file must already be open for loading or storing.
    inline CArchive::CArchive(CFile& File, mode Mode) : m_Schema(static_cast<UINT>(-1)), m_IsFileManaged(false)
    {
        m_pFile = &File;

        if (Mode == load)
        {
            m_IsStoring = false;
        }
        else
        {
            m_IsStoring = true;
        }
    }


    // Constructs a CArchive object.
    // A file with the specified name is created for storing (if required), and
    // also opened. A failure to open the file will throw an exception.
    inline CArchive::CArchive(LPCTSTR FileName, mode Mode) : m_pFile(0), m_Schema(static_cast<UINT>(-1))
    {
        m_IsFileManaged = true;

        try
        {
            if (Mode == load)
            {
                // Open the archive for loading
                m_pFile = new CFile(FileName, CFile::modeRead);
                m_IsStoring = false;
            }
            else
            {
                // Open the archive for storing. Creates file if required
                m_pFile = new CFile(FileName, CFile::modeCreate);
                m_IsStoring = true;
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

            if (m_IsFileManaged)
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


    // Return the archived data schema. This acts as a version number on
    // the format of the archived data for special handling when there
    // are several versions of the serialized data to be accommodated
    // by the application.
    inline UINT CArchive::GetObjectSchema()
    {
        return m_Schema;
    }


    // Return the current sense of serialization, true if the archive is
    // being loaded.
    inline bool CArchive::IsLoading() const
    {
         return !m_IsStoring;
    }


    // Return the current sense of serialization, true if the archive is
    // being stored.
    inline bool CArchive::IsStoring() const
    {
        return m_IsStoring;
    }


    // Read size bytes from the open archive file into the given lpBuf.
    // Throw an exception if not successful.
    inline void CArchive::Read(void* lpBuf, UINT size)
    {
        // read, simply and  in binary mode, the size into the lpBuf
        assert(m_pFile);
        m_pFile->Read(lpBuf, size);
    }

    // Record the archived data schema number.  This acts as a version number
    // on the format of the archived data for special handling when there
    // are several versions of the serialized data to be accommodated
    // by the application.
    inline void CArchive::SetObjectSchema(UINT nSchema)
    {
        m_Schema = nSchema;
    }


    // Write size characters of from the lpBuf into the open archive file.
    // Throw an exception if unsuccessful.
    inline void CArchive::Write(const void* lpBuf, UINT size)
    {
        // write size characters in lpBuf to the  file
        assert(m_pFile);
        m_pFile->Write(lpBuf, size);
    }


    // Write the BYTE b into the archive file. Throw an exception if an
    // error occurs.
    inline CArchive& CArchive::operator<<(BYTE b)
    {
        ArchiveObject ob(&b, sizeof(b));
        *this << ob;
        return *this;
    }


    // Write the WORD w into the archive file. Throw an exception if an
    // error occurs.
    inline CArchive& CArchive::operator<<(WORD w)
    {
        ArchiveObject ob(&w, sizeof(w));
        *this << ob;
        return *this;
    }


    // Write the LONG l into the archive file. Throw an exception if an
    // error occurs.
    inline CArchive& CArchive::operator<<(LONG l)
    {
        ArchiveObject ob(&l, sizeof(l));
        *this << ob;
        return *this;
    }


    // Write the LONGLONG l into the archive file. Throw an exception if an
    // error occurs.
    inline CArchive& CArchive::operator<<(LONGLONG ll)

    {
        ArchiveObject ob(&ll, sizeof(ll));
        *this << ob;
        return *this;
    }


    // Write the ULONGLONG ull into the archive file. Throw an exception if an
    // error occurs.
    inline CArchive& CArchive::operator<<(ULONGLONG ull)
    {
        ArchiveObject ob(&ull, sizeof(ull));
        *this << ob;
        return *this;
    }


    // Write the DWORD dw into the archive file. Throw an exception if an
    // error occurs.
    inline CArchive& CArchive::operator<<(DWORD dw)
    {
        ArchiveObject ob(&dw, sizeof(dw));
        *this << ob;
        return *this;
    }


    // Write the float f into the archive file. Throw an exception if an
    // error occurs.
    inline CArchive& CArchive::operator<<(float f)
    {
        ArchiveObject ob(&f, sizeof(f));
        *this << ob;
        return *this;
    }


    // Write the double d into the archive file. Throw an exception if an
    // error occurs.
    inline CArchive& CArchive::operator<<(double d)
    {
        ArchiveObject ob(&d, sizeof(d));
        *this << ob;
        return *this;
    }


    // Write the int i into the archive file. Throw an exception if an
    // error occurs.
    inline CArchive& CArchive::operator<<(int i)
    {
        ArchiveObject ob(&i, sizeof(i));
        *this << ob;
        return *this;
    }


    // Write the short s into the archive file. Throw an exception if an
    // error occurs.
    inline CArchive& CArchive::operator<<(short s)
    {
        ArchiveObject ob(&s, sizeof(s));
        *this << ob;
        return *this;
    }


    // Write the char c into the archive file. Throw an exception if an
    // error occurs.
    inline CArchive& CArchive::operator<<(char c)
    {
        ArchiveObject ob(&c, sizeof(c));
        *this << ob;
        return *this;
    }

// wchar_t is not an a built-in type on older MS compilers
#if !defined (_MSC_VER) ||  ( _MSC_VER > 1310 )

    // Write the wchar_t ch into the archive file. Throw an exception if an
    // error occurs.
    inline CArchive& CArchive::operator<<(wchar_t ch)
    {
        ArchiveObject ob(&ch, sizeof(ch));
        *this << ob;
        return *this;
    }

#endif

    // Write the unsigned u into the archive file. Throw an exception if an
    // error occurs.
    inline CArchive& CArchive::operator<<(unsigned u)
    {
        ArchiveObject ob(&u, sizeof(u));
        *this << ob;
        return *this;
    }


    // Write the bool b into the archive file. Throw an exception if an
    // error occurs.
    inline CArchive& CArchive::operator<<(bool b)
    {
        ArchiveObject ob(&b, sizeof(b));
        *this << ob;
        return *this;
    }


    // Write the CStringA string into the archive file.
    // The CStringA can contain any characters including embedded nulls.
    // Throw an exception if an error occurs.
    inline CArchive& CArchive::operator<<(const CStringA& string)
    {
        UINT nChars = string.GetLength();
        bool IsUnicode = false;

        // Store the Unicode state and number of characters in the archive
        *this << IsUnicode;
        *this << nChars;

        Write(string.c_str(), nChars*sizeof(CHAR));
        return *this;
    }


    // Write the CStringW string into the archive file.
    // The CStringW can contain any characters including embedded nulls.
    // Throw an exception if an error occurs.
    inline CArchive& CArchive::operator<<(const CStringW& string)
    {
        UINT nChars = string.GetLength();
        bool IsUnicode = true;

        // Store the Unicode state and number of characters in the archive
        *this << IsUnicode;
        *this << nChars;

        Write(string.c_str(), nChars*sizeof(WCHAR));
        return *this;
    }


    // Write the CString string into the archive file.
    // The CString can contain any characters including embedded nulls.
    // Throw an exception if an error occurs.
    inline CArchive& CArchive::operator<<(const CString& string)
    {
        UINT nChars = string.GetLength();
        bool IsUnicode = sizeof(TCHAR) -1;

        // Store the Unicode state and number of characters in the archive
        *this << IsUnicode;
        *this << nChars;

        Write(string.c_str(), nChars*sizeof(TCHAR));
        return *this;
    }


    // Write the POINT pt into the archive file. Throw an exception
    // if an error occurs.
    inline CArchive& CArchive::operator<<(const POINT& pt)
    {
        UINT size = sizeof(pt);

        // Write() throws exception upon error
        Write(&size, sizeof(size));
        Write(&pt, size);
        return *this;
    }


    // Write the RECT rc into the archive file. Throw an exception
    // if an error occurs.
    inline CArchive& CArchive::operator<<(const RECT& rc)
    {
        UINT size = sizeof(rc);

        // Write() throws exception upon error
        Write(&size, sizeof(size));
        Write(&rc, size);
        return *this;
    }


    // Write the SIZE sz into the archive file. Throw an exception
    // if an error occurs.
    inline CArchive& CArchive::operator<<(const SIZE& sz)
    {
        UINT size = sizeof(sz);

        // Write() throws exception upon error
        Write(&size, sizeof(size));
        Write(&sz, size);
        return *this;
    }


    // Write arbitrary CArchive object into this CArchive. Only ob.size  and
    // pointer ob.p to location are given. Throw an exception if unable
    // to do so successfully.
    inline CArchive& CArchive::operator<<(const ArchiveObject& ao)
    {
        Write(&ao.m_size, sizeof(ao.m_size));

        // Write() throws exception upon error
        Write(ao.m_pData, ao.m_size);
        return *this;
    }


    // Read a BYTE from the archive and  store it in b.  Throw an exception if
    // unable to do so correctly.
    inline CArchive& CArchive::operator>>(BYTE& b)
    {
        ArchiveObject ob(&b, sizeof(b));
        *this >> ob;
        return *this;
    }


    // Read a WORD from the archive and  store it in w.  Throw an exception if
    // unable to do so correctly.
    inline CArchive& CArchive::operator>>(WORD& w)
    {
        ArchiveObject ob(&w, sizeof(w));
        *this >> ob;
        return *this;
    }


    // Read a LONG from the archive and store it in l.  Throw an exception if
    // unable to do so correctly.
    inline CArchive& CArchive::operator>>(LONG& l)
    {
        ArchiveObject ob(&l, sizeof(l));
        *this >> ob;
        return *this;
    }


    // Read a LONGLONG from the archive and store it in ll.  Throw an exception if
    // unable to do so correctly.
    inline CArchive& CArchive::operator>>(LONGLONG& ll)
    {
        ArchiveObject ob(&ll, sizeof(ll));
        *this >> ob;
        return *this;
    }


    // Read a ULONGLONG from the archive and store it in ull.  Throw an exception if
    // unable to do so correctly.
    inline CArchive& CArchive::operator>>(ULONGLONG& ull)
    {
        ArchiveObject ob(&ull, sizeof(ull));
        *this >> ob;
        return *this;
    }

    // Read a DWORD  from the archive and  store it in dw.  Throw an
    // exception if unable to do so correctly.
    inline CArchive& CArchive::operator>>(DWORD& dw)
    {
        ArchiveObject ob(&dw, sizeof(dw));
        *this >> ob;
        return *this;
    }


    // Read a float from the archive and  store it in f.  Throw an exception if
    // unable to do so correctly.
    inline CArchive& CArchive::operator>>(float& f)
    {
        ArchiveObject ob(&f, sizeof(f));
        *this >> ob;
        return *this;
    }


    // Read a double from the archive and  store it in d.  Throw an exception if
    // unable to do so correctly.
    inline CArchive& CArchive::operator>>(double& d)
    {
        ArchiveObject ob(&d, sizeof(d));
        *this >> ob;
        return *this;
    }


    // Read an int from the archive and  store it in i.  Throw an exception if
    // unable to do so correctly.
    inline CArchive& CArchive::operator>>(int& i)
    {
        ArchiveObject ob(&i, sizeof(i));
        *this >> ob;
        return *this;
    }


    // Read a short from the archive and  store it in i.  Throw an exception if
    // unable to do so correctly.
    inline CArchive& CArchive::operator>>(short& i)
    {
        ArchiveObject ob(&i, sizeof(i));
        *this >> ob;
        return *this;
    }


    // Read a char from the archive and  store it in c.  Throw an exception if
    // unable to do so correctly.
    inline CArchive& CArchive::operator>>(char& c)
    {
        ArchiveObject ob(&c, sizeof(c));
        *this >> ob;
        return *this;
    }

// wchar_t is not an a built-in type on older MS compilers
#if !defined (_MSC_VER) ||  ( _MSC_VER > 1310 )

    // Read a wchar_t from the archive and  store it in ch.  Throw an exception if
    // unable to do so correctly.
    inline CArchive& CArchive::operator>>(wchar_t& ch)

    {
        ArchiveObject ob(&ch, sizeof(ch));
        *this >> ob;
        return *this;
    }

#endif

    // Read an unsigned int from the archive and  store it in u.  Throw an
    // exception if unable to do so correctly.
    inline CArchive& CArchive::operator>>(unsigned& u)
    {
        ArchiveObject ob(&u, sizeof(u));
        *this >> ob;
        return *this;
    }


    // Read an bool from the archive and  store it in b.  Throw an
    // exception if unable to do so correctly.
    inline CArchive& CArchive::operator>>(bool& b)
    {
        ArchiveObject ob(&b, sizeof(b));
        *this >> ob;
        return *this;
    }


    // Read a CString from the archive and  store it in string.  Throw an
    // exception if unable to do so correctly.
    inline CArchive& CArchive::operator>>(CStringA& string)
    {
        bool IsUnicode;
        UINT nChars;

        // Retrieve the Unicode state and number of characters from the archive
        *this >> IsUnicode;
        *this >> nChars;

        if (IsUnicode)
            throw CFileException(m_pFile->GetFilePath(), _T("Unicode characters stored. Not a CStringA"));

        Read(string.GetBuffer(nChars), nChars);
        string.ReleaseBuffer(nChars);

        return *this;
    }


    // Read a CStringW from the archive and  store it in string.  Throw an
    // exception if unable to do so correctly.
    inline CArchive& CArchive::operator>>(CStringW& string)
    {
        bool IsUnicode;
        UINT nChars;

        // Retrieve the Unicode state and number of characters from the archive
        *this >> IsUnicode;
        *this >> nChars;

        if (!IsUnicode)
            throw CFileException(m_pFile->GetFilePath(), _T("ANSI characters stored. Not a CStringW"));

        Read(string.GetBuffer(nChars), nChars * 2);
        string.ReleaseBuffer(nChars);

        return *this;
    }


    // Read a CString from the archive and  store it in string.  Throw an
    // exception if unable to do so correctly. Note: exceptions are thrown
    // only on inability to read the recorded number of chars from the archive
    // stream.
    inline CArchive& CArchive::operator>>(CString& string)
    {
        bool IsUnicode;
        UINT nChars;

        // Retrieve the Unicode state and number of characters from the archive
        *this >> IsUnicode;
        *this >> nChars;

        if (IsUnicode)
        {
            // use a vector to create our WCHAR array
            std::vector<WCHAR> vWChar(nChars + 1, L'\0');
            WCHAR* buf = &vWChar.front();

            Read(buf, nChars*2);

#ifdef UNICODE
            memcpy(string.GetBuffer(nChars), buf, nChars*2);
#else
            // Convert the archive string from Wide to Ansi
            WideCharToMultiByte(CP_ACP, 0, buf, nChars, string.GetBuffer(nChars), nChars, NULL,NULL);
#endif

            string.ReleaseBuffer(nChars);
        }
        else
        {
            // use a vector to create our char array
            std::vector<char> vChar(nChars + 1, '\0');
            char* buf = &vChar.front();

            Read(buf, nChars);

#ifdef UNICODE
            // Convert the archive string from Ansi to Wide
            MultiByteToWideChar(CP_ACP, 0, buf, nChars, string.GetBuffer(nChars), nChars);
#else
            memcpy(string.GetBuffer(nChars), buf, nChars);
#endif

            string.ReleaseBuffer(nChars);
        }

        return *this;
    }


    // Read a POINT from the archive and  store it in pt.  Throw an
    // exception if unable to do so correctly.
    inline CArchive& CArchive::operator>>(POINT& pt)
    {
        ArchiveObject ob(&pt, sizeof(pt));
        *this >> ob;
        return *this;
    }


    // Read a RECT from the archive and  store it in rc.  Throw an
    // exception if unable to do so correctly.
    inline CArchive& CArchive::operator>>(RECT& rc)
    {
        ArchiveObject ob(&rc, sizeof(rc));
        *this >> ob;
        return *this;
    }


    // Read a SIZE from the archive and  store it in sz.  Throw an
    // exception if unable to do so correctly.
    inline CArchive& CArchive::operator>>(SIZE& sz)
    {
        ArchiveObject ob(&sz, sizeof(sz));
        *this >> ob;
        return *this;
    }


    // Read a char string of size ob.size from the archive and  store it in
    // the location pointed to by ob.p.  Throw an exception if unable to
    // do so correctly.
    inline CArchive& CArchive::operator>>(ArchiveObject& ao)
    {
        assert(m_pFile);
        UINT size;
        Read(&size, sizeof(size));
        if (size != ao.m_size)
        {
            throw CFileException(m_pFile->GetFilePath(), _T("Unable to read object from archive"));
        }

        Read(ao.m_pData, ao.m_size);
        return *this;
    }


    // Write a CObject to the archive. Throw an exception if unable to
    // do so correctly.
    inline CArchive& CArchive::operator<<(const CObject& Ob)
    {
        ((CObject&)Ob).Serialize(*this);
        return *this;
    }


    // Read a CObject from the archive. Throw an exception if unable to
    // do so correctly.
    inline CArchive& CArchive::operator>>(CObject& Ob)
    {
        Ob.Serialize(*this);
        return *this;
    }


    // The size (in characters) of szString array must be nMax or greater.
    // Reads at most nMax-1 TCHAR characters from the archive and store it
    // in szString. Strings read from the archive are converted from ANSI
    // or Unicode to TCHAR if required, and are NULL terminated.
    // Throw an exception if unable to do so correctly.
    inline LPTSTR CArchive::ReadString(LPTSTR szString, UINT nMax)
    {
        assert (nMax > 0);

        CString str;
        *this >> str;
        lstrcpyn(szString, str.c_str(), nMax);
        return szString;
    }


    // The size (in characters) of szString array must be nMax or greater.
    // Reads at most nMax-1 TCHAR characters from the archive and store it
    // in szString. Strings read from the archive are converted from ANSI
    // or Unicode to TCHAR if required, and are NULL terminated.
    // Throw an exception if unable to do so correctly.
    inline LPSTR CArchive::ReadStringA(LPSTR szString, UINT nMax)
    {
        assert (nMax > 0);

        CStringA str;
        *this >> str;
        lstrcpynA(szString, str.c_str(), nMax);
        return szString;
    }


    // The size (in characters) of szString array must be nMax or greater.
    // Reads at most nMax-1 TCHAR characters from the archive and store it
    // in szString. Strings read from the archive are converted from ANSI
    // or Unicode to TCHAR if required, and are NULL terminated.
    // Throw an exception if unable to do so correctly.
    inline LPWSTR CArchive::ReadStringW(LPWSTR szString, UINT nMax)
    {
        assert (nMax > 0);

        CStringW str;
        *this >> str;
        lstrcpynW(szString, str.c_str(), nMax);
        return szString;
    }


    // Write the LPCTSTR string into the archive file.
    // The string must be null terminated.
    // Throw an exception if an error occurs.
    inline void CArchive::WriteString(LPCTSTR string)
    {
        UINT nChars = lstrlen(string);
        bool IsUnicode = sizeof(TCHAR) -1;

        // Store the Unicode state and number of characters in the archive
        *this << IsUnicode;
        *this << nChars;

        Write(string, nChars*sizeof(TCHAR));
    }


    // Write the LPCSTR string into the archive file.
    // The string must be null terminated.
    // Throw an exception if an error occurs.
    inline void CArchive::WriteStringA(LPCSTR string)
    {
        UINT nChars = lstrlenA(string);
        bool IsUnicode = false;

        // Store the Unicode state and number of characters in the archive
        *this << IsUnicode;
        *this << nChars;

        Write(string, nChars*sizeof(CHAR));
    }


    // Write the LPCWSTR string into the archive file.
    // The string must be null terminated.
    // Throw an exception if an error occurs.
    inline void CArchive::WriteStringW(LPCWSTR string)
    {
        UINT nChars = lstrlenW(string);
        bool IsUnicode = true;

        // Store the Unicode state and number of characters in the archive
        *this << IsUnicode;
        *this << nChars;

        Write(string, nChars*sizeof(WCHAR));
    }

} // namespace Win32xx

#endif // _WIN32XX_ARCHIVE_H_
