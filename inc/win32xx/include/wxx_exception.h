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


// A macro to support both old and new compilers.
#if defined(__clang_major__) || (defined (_MSC_VER) && (_MSC_VER >= 1900)) // Clang or VS2015 and higher
  #define WXX_NOEXCEPT noexcept
#else
  #define WXX_NOEXCEPT throw()
#endif


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
        CException(int messageID) WXX_NOEXCEPT;
        CException(LPCTSTR text = NULL, int messageID = 0) WXX_NOEXCEPT;
        virtual ~CException() WXX_NOEXCEPT;

        DWORD GetError() const WXX_NOEXCEPT;
        LPCTSTR GetErrorString() const WXX_NOEXCEPT;
        int GetMessageID() const WXX_NOEXCEPT;
        LPCTSTR GetText() const WXX_NOEXCEPT;
        virtual const char* what() const WXX_NOEXCEPT = 0; // pure virtual function

    private:
        TCHAR m_text[WXX_MAX_STRING_SIZE];
        TCHAR m_errorString[WXX_MAX_STRING_SIZE];
        int m_messageID;
        DWORD m_error;
    };


    ///////////////////////////////////////////////////////////
    // This exception is used by CArchive and CFile to indicate
    // a problem creating or accessing a file.
    // Note: Each function guarantees not to throw an exception
    class CFileException : public CException
    {
    public:
        CFileException(LPCTSTR filePath, int messageID) WXX_NOEXCEPT;
        CFileException(LPCTSTR filePath, LPCTSTR text= NULL, int messageID = 0) WXX_NOEXCEPT;
        virtual ~CFileException() WXX_NOEXCEPT;

        LPCTSTR GetFilePath() const WXX_NOEXCEPT;
        LPCTSTR GetFileName() const WXX_NOEXCEPT;
        virtual const char* what () const WXX_NOEXCEPT;

    private:
        TCHAR m_filePath[WXX_MAX_STRING_SIZE];
    };


    //////////////////////////////////////////////////////////////
    // This exception is used by the Win32++ framework to indicate
    // errors that prevent Win32++ from running.
    // Note: Each function guarantees not to throw an exception
    class CNotSupportedException : public CException
    {
    public:
        CNotSupportedException(int messageID) WXX_NOEXCEPT;
        CNotSupportedException(LPCTSTR text = NULL, int messageID = 0) WXX_NOEXCEPT;
        virtual ~CNotSupportedException() WXX_NOEXCEPT;
        virtual const char* what() const WXX_NOEXCEPT;
    };


    //////////////////////////////////////////////////////////////
    // This exception is used by the Win32++ framework to indicate
    // a failure to create a GDI resource.
    // Note: Each function guarantees not to throw an exception.
    class CResourceException : public CException
    {
    public:
        CResourceException(int messageID) WXX_NOEXCEPT;
        CResourceException(LPCTSTR text = NULL, int messageID = 0) WXX_NOEXCEPT;
        virtual ~CResourceException() WXX_NOEXCEPT;
        virtual const char* what() const WXX_NOEXCEPT;
    };


    ////////////////////////////////////////////////////////////////////////
    // This exception it thrown by CDataExchange when verifications fail.
    // It is also the exception that is typically thrown by users.
    // Users have the option of specifying text when the exception is thrown,
    // and the option of specifying a message ID which could load text from
    // a resource.
    // Note: Each function guarantees not to throw an exception.
    class CUserException : public CException
    {
    public:
        CUserException(int messageID) WXX_NOEXCEPT;
        CUserException(LPCTSTR text = NULL, int messageID = 0) WXX_NOEXCEPT;
        virtual ~CUserException() WXX_NOEXCEPT;
        virtual const char* what() const WXX_NOEXCEPT;
    };


    /////////////////////////////////////////////////////////////////////
    // This is thrown when an attempt to create a thread or window fails.
    // GetErrorString can be used to retrieve the reason for the failure.
    // Note: Each function guarantees not to throw an exception.
    class CWinException : public CException
    {
    public:
        CWinException(int messageID) WXX_NOEXCEPT;
        CWinException(LPCTSTR text= NULL, int messageID = 0) WXX_NOEXCEPT;
        virtual ~CWinException() WXX_NOEXCEPT;
        virtual const char* what () const WXX_NOEXCEPT;
    };


    ///////////////////////////////////////
    // Definitions for the CException class
    //

    // CException constructor.
    inline CException::CException(int messageID) WXX_NOEXCEPT
            : m_messageID(messageID), m_error(::GetLastError())
    {
        ZeroMemory(m_text, WXX_MAX_STRING_SIZE * sizeof(TCHAR));
        ZeroMemory(m_errorString, WXX_MAX_STRING_SIZE * sizeof(TCHAR));

        // Store error information in m_errorString.
        DWORD flags = FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;
        ::FormatMessage(flags, NULL, m_error, 0, m_errorString, WXX_MAX_STRING_SIZE-1, NULL);
    }


    // CException constructor.
    inline CException::CException(LPCTSTR text /*= NULL*/, int messageID /*= 0*/) WXX_NOEXCEPT
            : m_messageID(messageID), m_error(::GetLastError())
    {
        ZeroMemory(m_text, WXX_MAX_STRING_SIZE * sizeof(TCHAR));
        ZeroMemory(m_errorString, WXX_MAX_STRING_SIZE * sizeof(TCHAR));

        if (text)
            StrCopy(m_text, text, WXX_MAX_STRING_SIZE);

        // Store error information in m_errorString.
        DWORD flags = FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;
        ::FormatMessage(flags, NULL, m_error, 0, m_errorString, WXX_MAX_STRING_SIZE-1, NULL);
    }

    // CException destructor.
    inline CException::~CException() WXX_NOEXCEPT
    {
    }

    // Returns the error reported by GetLastError.
    inline DWORD CException::GetError() const WXX_NOEXCEPT
    {
        return m_error;
    }

    // Retrieves the error string from GetLastError.
    inline LPCTSTR CException::GetErrorString() const WXX_NOEXCEPT
    {
        return m_errorString;
    }

    // Retrieves the message ID specified when the exception is thrown.
    // This could be a resource ID for a string in the resource script (resource.rc).
    inline int CException::GetMessageID() const WXX_NOEXCEPT
    {
        return m_messageID;
    }

    // Retrieves the string specified when the exception is thrown.
    inline LPCTSTR CException::GetText() const WXX_NOEXCEPT
    {
        return m_text;
    }


    ////////////////////////////////////////////
    // Definitions for the CFileException class.
    //

    // CFileException constructor
    inline CFileException::CFileException(LPCTSTR filePath, int messageID) WXX_NOEXCEPT
        : CException(messageID)
    {
        ZeroMemory(m_filePath, WXX_MAX_STRING_SIZE * sizeof(TCHAR));

        // Display some text in the debugger.
        ::OutputDebugString(_T("*** CFileException thrown ***\n"));

        if (filePath)
        {
            StrCopy(m_filePath, filePath, WXX_MAX_STRING_SIZE);
            ::OutputDebugString(_T("File name: "));
            ::OutputDebugString(filePath);
            ::OutputDebugString(_T("\n"));
        }

        if (GetError() != 0)
            ::OutputDebugString(GetErrorString());
    }

    // CFileException constructor
    inline CFileException::CFileException(LPCTSTR filePath, LPCTSTR text /*= NULL*/, int messageID /*= 0*/) WXX_NOEXCEPT
        : CException(text, messageID)
    {
        ZeroMemory(m_filePath, WXX_MAX_STRING_SIZE * sizeof(TCHAR));

        // Display some text in the debugger.
        ::OutputDebugString(_T("*** CFileException thrown ***\n"));

        if (filePath)
        {
            StrCopy(m_filePath, filePath, WXX_MAX_STRING_SIZE);
            ::OutputDebugString(_T("File name: "));
            ::OutputDebugString(filePath);
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

    // CFileException destructor.
    inline CFileException::~CFileException() WXX_NOEXCEPT
    {
    }

    // Returns the filename and path specified when the exception was thrown.
    inline LPCTSTR CFileException::GetFilePath() const WXX_NOEXCEPT
    {
        return m_filePath;
    }

    // Returns the filename excluding the path.
    inline LPCTSTR CFileException::GetFileName() const WXX_NOEXCEPT
    {
        // Get the index of the first character after the last '\'
        int index = lstrlen(m_filePath);
        while ( index > 0  &&  m_filePath[index-1] != _T('\\') )
        {
            --index;
        }

        return m_filePath + index;    // pointer arithmetic
    }

    // Returns the exception type as a char string. Use AtoT to convert this to TCHAR.
    inline const char* CFileException::what() const WXX_NOEXCEPT
    {
        return "Win32xx::CFileException";
    }


    ///////////////////////////////////////////////////
    // Definitions of the CNotSupportedException class.
    //

    // CNotSupportedException constructor
    inline CNotSupportedException::CNotSupportedException(int messageID) WXX_NOEXCEPT
        : CException(messageID)
    {
        // Display some text in the debugger
        ::OutputDebugString(_T("*** CNotSupportedException thrown ***\n"));

        if (GetError() != 0)
            ::OutputDebugString(GetErrorString());
    }

    // CNotSupportedException constructor.
    inline CNotSupportedException::CNotSupportedException(LPCTSTR text /*= NULL*/, int messageID /*= 0*/) WXX_NOEXCEPT
        : CException(text, messageID)
    {
        // Display some text in the debugger.
        ::OutputDebugString(_T("*** CNotSupportedException thrown ***\n"));

        if (text)
        {
            ::OutputDebugString(text);
            ::OutputDebugString(_T("\n"));
        }

        if (GetError() != 0)
            ::OutputDebugString(GetErrorString());
    }

    // CNotSupportedException destructor.
    inline CNotSupportedException::~CNotSupportedException() WXX_NOEXCEPT
    {
        if (GetError() != 0)
            ::OutputDebugString(GetErrorString());
    }

    // Returns the exception type as a char string. Use AtoT to convert this to TCHAR.
    inline const char* CNotSupportedException::what() const WXX_NOEXCEPT
    {
        return "Win32xx::CNotSupportedException";
    }


    //////////////////////////////////////////////////
    // Definitions of the CResourceException class.
    //

    // CResourceException constructor
    inline CResourceException::CResourceException(int messageID) WXX_NOEXCEPT
        : CException(messageID)
    {
        // Display some text in the debugger
        ::OutputDebugString(_T("*** CResourceException thrown ***\n"));

        if (GetError() != 0)
            ::OutputDebugString(GetErrorString());
    }

    // CResourceException constructor.
    inline CResourceException::CResourceException(LPCTSTR text /*= NULL*/, int messageID /*= 0*/) WXX_NOEXCEPT
        : CException(text, messageID)
    {
        // Display some text in the debugger.
        ::OutputDebugString(_T("*** CResourceException thrown ***\n"));

        if (text)
        {
            ::OutputDebugString(text);
            ::OutputDebugString(_T("\n"));
        }

        if (GetError() != 0)
            ::OutputDebugString(GetErrorString());
    }

    // CResourceException destructor.
    inline CResourceException::~CResourceException() WXX_NOEXCEPT
    {
    }

    // Returns the exception type as a char string. Use AtoT to convert this to TCHAR.
    inline const char* CResourceException::what() const WXX_NOEXCEPT
    {
        return "Win32xx::CResourceException";
    }


    ///////////////////////////////////////////
    // Definitions of the CUserException class.
    //

    // CUserException constructor
    inline CUserException::CUserException(int messageID) WXX_NOEXCEPT
            : CException(messageID)
    {
        // Display some text in the debugger.
        ::OutputDebugString(_T("*** CUserException thrown ***\n"));

        if (GetError() != 0)
            ::OutputDebugString(GetErrorString());
    }

    // CUserException constructor
    inline CUserException::CUserException(LPCTSTR text /*= NULL*/, int messageID /*= 0*/) WXX_NOEXCEPT
            : CException(text, messageID)
    {
        // Display some text in the debugger.
        ::OutputDebugString(_T("*** CUserException thrown ***\n"));

        if (text)
        {
            ::OutputDebugString(text);
            ::OutputDebugString(_T("\n"));
        }

        if (GetError() != 0)
            ::OutputDebugString(GetErrorString());
    }

    // CUserException destructor.
    inline CUserException::~CUserException() WXX_NOEXCEPT
    {
    }

    // Returns the exception type as a char string. Use AtoT to convert this to TCHAR.
    inline const char* CUserException::what() const WXX_NOEXCEPT
    {
        return "Win32xx::CUserException";
    }


    ///////////////////////////////////////////
    // Definitions for the CWinException class.
    //

    // CWinException constructor
    inline CWinException::CWinException(int messageID) WXX_NOEXCEPT
        : CException(messageID)
    {
        // Display some text in the debugger.
        ::OutputDebugString(_T("*** CWinException thrown ***\n"));

        if (GetError() != 0)
            ::OutputDebugString(GetErrorString());
    }

    // CWinException constructor.
    inline CWinException::CWinException(LPCTSTR text, int messageID) WXX_NOEXCEPT
        : CException(text, messageID)
    {
        // Display some text in the debugger.
        ::OutputDebugString(_T("*** CWinException thrown ***\n"));
        if (text)
        {
            ::OutputDebugString(text);
            ::OutputDebugString(_T("\n"));
        }

        if (GetError() != 0)
            ::OutputDebugString(GetErrorString());
    }

    // CWinException destructor.
    inline CWinException::~CWinException() WXX_NOEXCEPT
    {
    }

    // Returns the exception type as a char string. Use AtoT to convert this to TCHAR.
    inline const char * CWinException::what() const WXX_NOEXCEPT
    {
        return "Win32xx::CWinException";
    }


} // namespace Win32xx


#endif // _WIN32XX_EXCEPTION_H_
