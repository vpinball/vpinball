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



#ifndef _WIN32XX_EXCEPTION_H_
#define _WIN32XX_EXCEPTION_H_


//
// Sample code, demonstrating how to use CUserException with a string resource:
//
//  try
//  {
//      ...
//      if (Failed)
//          throw CUserException(nID));
//      ...
//  }
//  catch(const CException &e) // catch all exceptions inherited from CException
//  {
//      // display the exception in a message box
//      ::MessageBox(NULL, LoadString(e.GetMessageID()), AtoT(e.what()), MB_ICONERROR);
//  }

//
// Sample code, demonstrating how to use CUserException with a text string:
//
//  try
//  {
//      ...
//      if (Failed)
//          throw CUserException(_T("Some Text"));
//      ...
//  }
//  catch(const CException &e) // catch all exceptions inherited from CException
//  {
//      // display the exception in a message box
//      ::MessageBox(NULL, e.GetText(), AtoT(e.what()), MB_ICONERROR);
//  }



namespace Win32xx
{
    ////////////////////////////////////////
    // Declaration of the CException class
    //
    //  This class has a pure virtual function and is an abstract class.
    //  We can't throw a CException directly, but we can throw any exception
    //  inherited from CException. We can catch all exceptions inherited from
    //  CException with a single catch statement.
    //
    class CException : public std::exception
    {
    public:
        CException(int nMessageID);
        CException(LPCTSTR pszText = NULL, int nMessageID = 0);
        virtual ~CException() throw();

        DWORD GetError() const throw();
        LPCTSTR GetErrorString() const throw();
        int GetMessageID() const throw();
        LPCTSTR GetText() const throw();
        virtual const char* what() const throw() = 0; // pure virtual function

    private:
        TCHAR m_szText[MAX_STRING_SIZE];
        TCHAR m_szErrorString[MAX_STRING_SIZE];
        DWORD  m_nMessageID;
        DWORD  m_Error;
    };


    //////////////////////////////////////////////////
    // Declaration of the CFileException class
    //
    // This exception is used by CArchive and CFile to indicate
    // a problem creating or accessing a file.
    // Note: Each function guarantees not to throw an exception
    class CFileException : public CException
    {
    public:
        CFileException(LPCTSTR szFilePath, int nMessageID);
        CFileException(LPCTSTR szFilePath, LPCTSTR pszText= NULL, int nMessageID = 0);
        virtual ~CFileException() throw();

        LPCTSTR GetFilePath() const throw();
        LPCTSTR GetFileName() const throw();
        virtual const char* what () const throw();

    private:
        TCHAR m_szFilePath[MAX_STRING_SIZE];
    };


    //////////////////////////////////////////////////
    // Declaration of the CNotSupportedException class
    //
    // This exception is used by the Win32++ framework to indicate
    // errors that prevent Win32++ from running.
    // Note: Each function guarantees not to throw an exception
    class CNotSupportedException : public CException
    {
    public:
        CNotSupportedException(int nMessageID);
        CNotSupportedException(LPCTSTR pszText = NULL, int nMessageID = 0);
        virtual ~CNotSupportedException() throw();
        virtual const char* what() const throw();
    };


    //////////////////////////////////////////////////
    // Declaration of the CResourceException class
    //
    // This exception is used by the Win32++ framework to indicate
    // a failure to create a GDI resource.
    // Note: Each function guarantees not to throw an exception
    class CResourceException : public CException
    {
    public:
        CResourceException(int nMessageID);
        CResourceException(LPCTSTR pszText = NULL, int nMessageID = 0);
        virtual ~CResourceException() throw();
        virtual const char* what() const throw();
    };


    //////////////////////////////////////////
    // Declaration of the CUserException class
    //
    // This exception it thrown by CDataExchange when verifications fail.
    // It is also the exception that is typically thrown by users.
    // Users have the option of specifying text when the exception is thrown,
    // and the option of specifying a message ID which could load text from
    // a resource.
    // Note: Each function guarantees not to throw an exception
    class CUserException : public CException
    {
    public:
        CUserException(int nMessageID);
        CUserException(LPCTSTR pszText = NULL, int nMessageID = 0);
        virtual ~CUserException() throw();
        virtual const char* what() const throw();
    };


    ////////////////////////////////////////
    // Declaration of the CWinException class
    //
    // This is thrown when an attempt to create a thread or window fails.
    // GetErrorString can be used to retrieve the reason for the failure.
    // Note: Each function guarantees not to throw an exception
    class CWinException : public CException
    {
    public:
        CWinException(int nMessageID);
        CWinException(LPCTSTR pszText= NULL, int nMessageID = 0);
        virtual ~CWinException() throw();
        virtual const char* what () const throw();
    };


    //////////////////////////////////////////
    // Definitions for the CException class
    //

    // CException constructor
    inline CException::CException(int nMessageID)
            : m_nMessageID(nMessageID), m_Error(::GetLastError())
    {
        memset(m_szText, 0, MAX_STRING_SIZE * sizeof(TCHAR));
        memset(m_szErrorString, 0, MAX_STRING_SIZE * sizeof(TCHAR));


        // Store error information in m_szErrorString
        DWORD dwFlags = FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;
        ::FormatMessage(dwFlags, NULL, m_Error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), m_szErrorString, MAX_STRING_SIZE-1, NULL);
    }


    // CException constructor
    inline CException::CException(LPCTSTR pszText /*= NULL*/, int nMessageID /*= 0*/)
            : m_nMessageID(nMessageID), m_Error(::GetLastError())
    {
        memset(m_szText, 0, MAX_STRING_SIZE * sizeof(TCHAR));
        memset(m_szErrorString, 0, MAX_STRING_SIZE * sizeof(TCHAR));

        if (pszText)
            lstrcpyn(m_szText, pszText, MAX_STRING_SIZE-1);

        // Store error information in m_szErrorString
        DWORD dwFlags = FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;
        ::FormatMessage(dwFlags, NULL, m_Error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), m_szErrorString, MAX_STRING_SIZE-1, NULL);
    }


    // CException destructor
    inline CException::~CException() throw()
    {
    }


    // Returns the error reported by GetLastError
    inline DWORD CException::GetError() const throw()
    {
        return m_Error;
    }


    // Retrieves the error string from GetLastError.
    inline LPCTSTR CException::GetErrorString() const throw()
    {
        return m_szErrorString;
    }


    // Retrieves the message ID specified when the exception is thrown.
    // This could be a resource ID for a string in the resource script (resource.rc).
    inline int CException::GetMessageID() const throw()
    {
        return m_nMessageID;
    }


    // Retrieves the string specified when the exception is thrown.
    inline LPCTSTR CException::GetText() const throw()
    {
        return m_szText;
    }


    ///////////////////////////////////////////
    // Definitions for the CFileException class
    //

    // CFileException constructor
    inline CFileException::CFileException(LPCTSTR szFilePath, int nMessageID)
        : CException(nMessageID)
    {
        // Display some text in the debugger
        ::OutputDebugString(_T("*** CFileException thrown ***\n"));

        memset(m_szFilePath, 0, MAX_STRING_SIZE * sizeof(TCHAR));

        if (szFilePath)
        {
            lstrcpyn(m_szFilePath, szFilePath, MAX_STRING_SIZE-1);
            ::OutputDebugString(_T("File name: "));
            ::OutputDebugString(szFilePath);
            ::OutputDebugString(_T("\n"));
        }

        if (GetError() != 0)
            ::OutputDebugString(GetErrorString());
    }


    // CFileException constructor
    inline CFileException::CFileException(LPCTSTR szFilePath, LPCTSTR pszText /*= NULL*/, int nMessageID /*= 0*/)
        : CException(pszText, nMessageID)
    {
        // Display some text in the debugger
        ::OutputDebugString(_T("*** CFileException thrown ***\n"));

        memset(m_szFilePath, 0, MAX_STRING_SIZE * sizeof(TCHAR));

        if (szFilePath)
        {
            lstrcpyn(m_szFilePath, szFilePath, MAX_STRING_SIZE-1);
            ::OutputDebugString(_T("File name: "));
            ::OutputDebugString(szFilePath);
            ::OutputDebugString(_T("\n"));
        }

        if (pszText)
        {
            ::OutputDebugString(pszText);
            ::OutputDebugString(_T("\n"));
        }

        if (GetError() != 0)
            ::OutputDebugString(GetErrorString());
    }


    // CFileException destructor
    inline CFileException::~CFileException() throw()
    {
    }


    // Returns the filename and path specified when the exception was thrown
    inline LPCTSTR CFileException::GetFilePath() const throw()
    {
        return m_szFilePath;
    }


    // Returns the filename excluding the path.
    inline LPCTSTR CFileException::GetFileName() const throw()
    {
        // Get the index of the first character after the last '\'
        int index = lstrlen(m_szFilePath);
        while ( index > 0  &&  m_szFilePath[index-1] != _T('\\') )
        {
            --index;
        }

        return m_szFilePath + index;    // pointer arithmetic
    }


    // Returns the exception type as a char string. Use AtoT to convert this to TCHAR
    inline const char* CFileException::what() const throw()
    {
        return "Win32xx::CFileException";
    }


    //////////////////////////////////////////////////
    // Definitions of the CNotSupportedException class
    //

    // CNotSupportedException constructor
    inline CNotSupportedException::CNotSupportedException(int nMessageID)
        : CException(nMessageID)
    {
        // Display some text in the debugger
        ::OutputDebugString(_T("*** CNotSupportedException thrown ***\n"));

        if (GetError() != 0)
            ::OutputDebugString(GetErrorString());
    }


    // CNotSupportedException constructor
    inline CNotSupportedException::CNotSupportedException(LPCTSTR pszText /*= NULL*/, int nMessageID /*= 0*/)
        : CException(pszText, nMessageID)
    {
        // Display some text in the debugger
        ::OutputDebugString(_T("*** CNotSupportedException thrown ***\n"));

        if (pszText)
        {
            ::OutputDebugString(pszText);
            ::OutputDebugString(_T("\n"));
        }

        if (GetError() != 0)
            ::OutputDebugString(GetErrorString());
    }


    // CNotSupportedException destructor
    inline CNotSupportedException::~CNotSupportedException() throw()
    {
        if (GetError() != 0)
            ::OutputDebugString(GetErrorString());
    }


    // Returns the exception type as a char string. Use AtoT to convert this to TCHAR
    inline const char* CNotSupportedException::what() const throw()
    {
        return "Win32xx::CNotSupportedException";
    }


    //////////////////////////////////////////////////
    // Definitions of the CResourceException class
    //

    // CResourceException constructor
    inline CResourceException::CResourceException(int nMessageID)
        : CException(nMessageID)
    {
        // Display some text in the debugger
        ::OutputDebugString(_T("*** CResourceException thrown ***\n"));

        if (GetError() != 0)
            ::OutputDebugString(GetErrorString());
    }


    // CResourceException constructor
    inline CResourceException::CResourceException(LPCTSTR pszText /*= NULL*/, int nMessageID /*= 0*/)
        : CException(pszText, nMessageID)
    {
        // Display some text in the debugger
        ::OutputDebugString(_T("*** CResourceException thrown ***\n"));

        if (pszText)
        {
            ::OutputDebugString(pszText);
            ::OutputDebugString(_T("\n"));
        }

        if (GetError() != 0)
            ::OutputDebugString(GetErrorString());
    }


    // CResourceException destructor
    inline CResourceException::~CResourceException() throw()
    {
    }


    // Returns the exception type as a char string. Use AtoT to convert this to TCHAR
    inline const char* CResourceException::what() const throw()
    {
        return "Win32xx::CResourceException";
    }


    ////////////////////////////////////////
    // Definitions of the CUserException class
    //

    // CUserException constructor
    inline CUserException::CUserException(int nMessageID)
            : CException(nMessageID)
    {
        // Display some text in the debugger
        ::OutputDebugString(_T("*** CUserException thrown ***\n"));

        if (GetError() != 0)
            ::OutputDebugString(GetErrorString());
    }


    // CUserException constructor
    inline CUserException::CUserException(LPCTSTR pszText /*= NULL*/, int nMessageID /*= 0*/)
            : CException(pszText, nMessageID)
    {
        // Display some text in the debugger
        ::OutputDebugString(_T("*** CUserException thrown ***\n"));

        if (pszText)
        {
            ::OutputDebugString(pszText);
            ::OutputDebugString(_T("\n"));
        }

        if (GetError() != 0)
            ::OutputDebugString(GetErrorString());
    }


    // CUserException destructor
    inline CUserException::~CUserException() throw()
    {
    }


    // Returns the exception type as a char string. Use AtoT to convert this to TCHAR
    inline const char* CUserException::what() const throw()
    {
        return "Win32xx::CUserException";
    }


    //////////////////////////////////////////
    // Definitions for the CWinException class
    //

    // CWinException constructor
    inline CWinException::CWinException(int nMessageID)
        : CException(nMessageID)
    {
        // Display some text in the debugger
        ::OutputDebugString(_T("*** CWinException thrown ***\n"));

        if (GetError() != 0)
            ::OutputDebugString(GetErrorString());
    }


    // CWinException constructor
    inline CWinException::CWinException(LPCTSTR pszText, int nMessageID)
        : CException(pszText, nMessageID)
    {
        // Display some text in the debugger
        ::OutputDebugString(_T("*** CWinException thrown ***\n"));
        if (pszText)
        {
            ::OutputDebugString(pszText);
            ::OutputDebugString(_T("\n"));
        }

        if (GetError() != 0)
            ::OutputDebugString(GetErrorString());
    }


    // CWinException destructor
    inline CWinException::~CWinException() throw()
    {
    }


    // Returns the exception type as a char string. Use AtoT to convert this to TCHAR
    inline const char * CWinException::what() const throw()
    {
        return "Win32xx::CWinException";
    }


} // namespace Win32xx


#endif // _WIN32XX_EXCEPTION_H_
