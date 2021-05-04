// Win32++   Version 8.9
// Release Date: 29th April 2021
//
//      David Nash
//      email: dnash@bigpond.net.au
//      url: https://sourceforge.net/projects/win32-framework
//
//
// Copyright (c) 2005-2021  David Nash
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
//      if (failed)
//          throw CUserException(id));
//      ...
//  }
//  catch(const CException &e) // catch all exceptions inherited from CException
//  {
//      // display the exception in a message box
//      ::MessageBox(0, LoadString(e.GetMessageID()), AtoT(e.what()), MB_ICONERROR);
//  }

//
// Sample code, demonstrating how to use CUserException with a text string:
//
//  try
//  {
//      ...
//      if (failed)
//          throw CUserException(_T("Some Text"));
//      ...
//  }
//  catch(const CException &e) // catch all exceptions inherited from CException
//  {
//      // display the exception in a message box
//      ::MessageBox(0, e.GetText(), AtoT(e.what()), MB_ICONERROR);
//  }



namespace Win32xx
{
    ////////////////////////////////////////
    // This is the base class for all exceptions defined by Win32++.
    // This class has a pure virtual function and is an abstract class.
    // We can't throw a CException directly, but we can throw any exception
    // inherited from CException. We can catch all exceptions inherited from
    // CException with a single catch statement.
    //
    class CException : public std::exception
    {
    public:
        CException(int messageID) throw();
        CException(LPCTSTR text = NULL, int messageID = 0) throw();
        virtual ~CException() throw();

        DWORD GetError() const throw();
        LPCTSTR GetErrorString() const throw();
        int GetMessageID() const throw();
        LPCTSTR GetText() const throw();
        virtual const char* what() const throw() = 0; // pure virtual function

    private:
        TCHAR m_text[MAX_STRING_SIZE];
        TCHAR m_errorString[MAX_STRING_SIZE];
        DWORD m_messageID;
        DWORD m_error;
    };


    ///////////////////////////////////////////////////////////
    // This exception is used by CArchive and CFile to indicate
    // a problem creating or accessing a file.
    // Note: Each function guarantees not to throw an exception
    class CFileException : public CException
    {
    public:
        CFileException(LPCTSTR filePath, int messageID) throw();
        CFileException(LPCTSTR filePath, LPCTSTR text= NULL, int messageID = 0) throw();
        virtual ~CFileException() throw();

        LPCTSTR GetFilePath() const throw();
        LPCTSTR GetFileName() const throw();
        virtual const char* what () const throw();

    private:
        TCHAR m_filePath[MAX_STRING_SIZE];
    };


    //////////////////////////////////////////////////////////////
    // This exception is used by the Win32++ framework to indicate
    // errors that prevent Win32++ from running.
    // Note: Each function guarantees not to throw an exception
    class CNotSupportedException : public CException
    {
    public:
        CNotSupportedException(int messageID) throw();
        CNotSupportedException(LPCTSTR text = NULL, int messageID = 0) throw();
        virtual ~CNotSupportedException() throw();
        virtual const char* what() const throw();
    };


    //////////////////////////////////////////////////////////////
    // This exception is used by the Win32++ framework to indicate
    // a failure to create a GDI resource.
    // Note: Each function guarantees not to throw an exception
    class CResourceException : public CException
    {
    public:
        CResourceException(int messageID) throw();
        CResourceException(LPCTSTR text = NULL, int messageID = 0) throw();
        virtual ~CResourceException() throw();
        virtual const char* what() const throw();
    };


    ////////////////////////////////////////////////////////////////////////
    // This exception it thrown by CDataExchange when verifications fail.
    // It is also the exception that is typically thrown by users.
    // Users have the option of specifying text when the exception is thrown,
    // and the option of specifying a message ID which could load text from
    // a resource.
    // Note: Each function guarantees not to throw an exception
    class CUserException : public CException
    {
    public:
        CUserException(int messageID) throw();
        CUserException(LPCTSTR text = NULL, int messageID = 0) throw();
        virtual ~CUserException() throw();
        virtual const char* what() const throw();
    };


    /////////////////////////////////////////////////////////////////////
    // This is thrown when an attempt to create a thread or window fails.
    // GetErrorString can be used to retrieve the reason for the failure.
    // Note: Each function guarantees not to throw an exception
    class CWinException : public CException
    {
    public:
        CWinException(int messageID) throw();
        CWinException(LPCTSTR text= NULL, int messageID = 0) throw();
        virtual ~CWinException() throw();
        virtual const char* what () const throw();
    };


    ///////////////////////////////////////
    // Definitions for the CException class
    //

    // CException constructor
    inline CException::CException(int messageID) throw()
            : m_messageID(messageID), m_error(::GetLastError())
    {
        memset(m_text, 0, MAX_STRING_SIZE * sizeof(TCHAR));
        memset(m_errorString, 0, MAX_STRING_SIZE * sizeof(TCHAR));


        // Store error information in m_errorString
        DWORD flags = FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;
        ::FormatMessage(flags, NULL, m_error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), m_errorString, MAX_STRING_SIZE-1, NULL);
    }


    // CException constructor
    inline CException::CException(LPCTSTR text /*= NULL*/, int messageID /*= 0*/) throw()
            : m_messageID(messageID), m_error(::GetLastError())
    {
        memset(m_text, 0, MAX_STRING_SIZE * sizeof(TCHAR));
        memset(m_errorString, 0, MAX_STRING_SIZE * sizeof(TCHAR));

        if (text)
            StrCopy(m_text, text, MAX_STRING_SIZE);

        // Store error information in m_errorString
        DWORD flags = FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;
        ::FormatMessage(flags, NULL, m_error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), m_errorString, MAX_STRING_SIZE-1, NULL);
    }

    // CException destructor
    inline CException::~CException() throw()
    {
    }

    // Returns the error reported by GetLastError
    inline DWORD CException::GetError() const throw()
    {
        return m_error;
    }

    // Retrieves the error string from GetLastError.
    inline LPCTSTR CException::GetErrorString() const throw()
    {
        return m_errorString;
    }

    // Retrieves the message ID specified when the exception is thrown.
    // This could be a resource ID for a string in the resource script (resource.rc).
    inline int CException::GetMessageID() const throw()
    {
        return m_messageID;
    }

    // Retrieves the string specified when the exception is thrown.
    inline LPCTSTR CException::GetText() const throw()
    {
        return m_text;
    }


    ///////////////////////////////////////////
    // Definitions for the CFileException class
    //

    // CFileException constructor
    inline CFileException::CFileException(LPCTSTR pFilePath, int messageID) throw()
        : CException(messageID)
    {
        // Display some text in the debugger
        ::OutputDebugString(_T("*** CFileException thrown ***\n"));

        memset(m_filePath, 0, MAX_STRING_SIZE * sizeof(TCHAR));

        if (pFilePath)
        {
            StrCopy(m_filePath, pFilePath, MAX_STRING_SIZE);
            ::OutputDebugString(_T("File name: "));
            ::OutputDebugString(pFilePath);
            ::OutputDebugString(_T("\n"));
        }

        if (GetError() != 0)
            ::OutputDebugString(GetErrorString());
    }

    // CFileException constructor
    inline CFileException::CFileException(LPCTSTR pFilePath, LPCTSTR text /*= NULL*/, int messageID /*= 0*/) throw()
        : CException(text, messageID)
    {
        // Display some text in the debugger
        ::OutputDebugString(_T("*** CFileException thrown ***\n"));

        memset(m_filePath, 0, MAX_STRING_SIZE * sizeof(TCHAR));

        if (pFilePath)
        {
            StrCopy(m_filePath, pFilePath, MAX_STRING_SIZE);
            ::OutputDebugString(_T("File name: "));
            ::OutputDebugString(pFilePath);
            ::OutputDebugString(_T("\n"));
        }

        if (text)
        {
            ::OutputDebugString(text);
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
        return m_filePath;
    }

    // Returns the filename excluding the path.
    inline LPCTSTR CFileException::GetFileName() const throw()
    {
        // Get the index of the first character after the last '\'
        int index = lstrlen(m_filePath);
        while ( index > 0  &&  m_filePath[index-1] != _T('\\') )
        {
            --index;
        }

        return m_filePath + index;    // pointer arithmetic
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
    inline CNotSupportedException::CNotSupportedException(int messageID) throw()
        : CException(messageID)
    {
        // Display some text in the debugger
        ::OutputDebugString(_T("*** CNotSupportedException thrown ***\n"));

        if (GetError() != 0)
            ::OutputDebugString(GetErrorString());
    }

    // CNotSupportedException constructor
    inline CNotSupportedException::CNotSupportedException(LPCTSTR text /*= NULL*/, int messageID /*= 0*/) throw()
        : CException(text, messageID)
    {
        // Display some text in the debugger
        ::OutputDebugString(_T("*** CNotSupportedException thrown ***\n"));

        if (text)
        {
            ::OutputDebugString(text);
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
    inline CResourceException::CResourceException(int messageID) throw()
        : CException(messageID)
    {
        // Display some text in the debugger
        ::OutputDebugString(_T("*** CResourceException thrown ***\n"));

        if (GetError() != 0)
            ::OutputDebugString(GetErrorString());
    }

    // CResourceException constructor
    inline CResourceException::CResourceException(LPCTSTR text /*= NULL*/, int messageID /*= 0*/) throw()
        : CException(text, messageID)
    {
        // Display some text in the debugger
        ::OutputDebugString(_T("*** CResourceException thrown ***\n"));

        if (text)
        {
            ::OutputDebugString(text);
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
    inline CUserException::CUserException(int messageID) throw()
            : CException(messageID)
    {
        // Display some text in the debugger
        ::OutputDebugString(_T("*** CUserException thrown ***\n"));

        if (GetError() != 0)
            ::OutputDebugString(GetErrorString());
    }

    // CUserException constructor
    inline CUserException::CUserException(LPCTSTR text /*= NULL*/, int messageID /*= 0*/) throw()
            : CException(text, messageID)
    {
        // Display some text in the debugger
        ::OutputDebugString(_T("*** CUserException thrown ***\n"));

        if (text)
        {
            ::OutputDebugString(text);
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
    inline CWinException::CWinException(int messageID) throw()
        : CException(messageID)
    {
        // Display some text in the debugger
        ::OutputDebugString(_T("*** CWinException thrown ***\n"));

        if (GetError() != 0)
            ::OutputDebugString(GetErrorString());
    }

    // CWinException constructor
    inline CWinException::CWinException(LPCTSTR text, int messageID) throw()
        : CException(text, messageID)
    {
        // Display some text in the debugger
        ::OutputDebugString(_T("*** CWinException thrown ***\n"));
        if (text)
        {
            ::OutputDebugString(text);
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
