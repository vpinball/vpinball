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


#ifndef _WIN32XX_FILE_H_
#define _WIN32XX_FILE_H_


#include "wxx_cstring.h"
#include "wxx_exception.h"


namespace Win32xx
{

    /////////////////////////////////////
    // The CFile class manages files. It can be used to: create; read from;
    // write to; rename; and remove a file.
    class CFile
    {
    public:

        enum OpenFlags
        {
            modeCreate =        CREATE_ALWAYS,  // Creates a new file. Truncates existing file to length 0.
            modeNoTruncate =    OPEN_ALWAYS, // Creates a new file or opens an existing one.
            shareExclusive =    0x0010, // Denies read and write access to all others.
            shareDenyWrite =    0x0020, // Denies write access to all others.
            shareDenyRead =     0x0030, // Denies read access to all others.
            shareDenyNone =     0x0040, // No sharing restrictions.
            modeRead =          0x0100, // Requests read access only.
            modeWrite =         0x0200, // Requests write access only.
            modeReadWrite =     0x0300  // Requests read and write access.
        };

        CFile();
        CFile(HANDLE hFile);
        CFile(LPCTSTR pszFileName, UINT nOpenFlags);
        virtual ~CFile();
        operator HANDLE() const;

        virtual void Close();
        virtual void Flush();
        virtual CString GetFileDirectory() const;
        virtual const CString& GetFileName() const;
        virtual CString GetFileNameExt() const;
        virtual CString GetFileNameWOExt() const;
        virtual const CString& GetFilePath() const;
        virtual CString GetFileTitle() const;
        HANDLE GetHandle() const;
        virtual ULONGLONG GetLength() const;
        virtual ULONGLONG GetPosition() const;
        virtual void Open(LPCTSTR pszFileName, UINT nOpenFlags);
        virtual UINT Read(void* pBuf, UINT nCount);
        static void Remove(LPCTSTR pszFileName);
        static void Rename(LPCTSTR pszOldName, LPCTSTR pszNewName);
        virtual ULONGLONG Seek(LONGLONG lOff, UINT nFrom);
        virtual void SeekToBegin();
        virtual ULONGLONG SeekToEnd();
        virtual void SetLength(ULONGLONG NewLen);
        virtual void Write(const void* pBuf, UINT nCount);

#ifndef _WIN32_WCE
        virtual void LockRange(ULONGLONG Pos, ULONGLONG Count);
        virtual void SetFilePath(LPCTSTR pszNewName);
        virtual void UnlockRange(ULONGLONG Pos, ULONGLONG Count);
#endif

    private:
        CFile(const CFile&);                // Disable copy construction
        CFile& operator = (const CFile&);   // Disable assignment operator
        CString m_FileName;
        CString m_FilePath;
        HANDLE m_hFile;
    };

}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

namespace Win32xx
{
    inline CFile::CFile() : m_hFile(0)
    {
    }

    inline CFile::CFile(HANDLE hFile) : m_hFile(hFile)
    {
    }


    // Possible nOpenFlag values: CREATE_NEW, CREATE_ALWAYS, OPEN_EXISTING, OPEN_ALWAYS, TRUNCATE_EXISTING
    // Default value: OPEN_EXISTING | modeReadWrite
    // The following modes are also supported:
    //  modeCreate      Creates a new file. Truncates an existing file to length 0.
    //  modeNoTruncate  Creates a a new file, or opens an existing one.
    //  modeRead        Requests read access only.
    //  modeWrite       Requests write access only.
    //  modeReadWrite   Requests read and write access.
    //  shareExclusive  Denies read and write access to all others.
    //  shareDenyWrite  Denies write access to all others.
    //  shareDenyRead   Denies read access to all others.
    //  shareDenyNone   No sharing restrictions.
    inline CFile::CFile(LPCTSTR pszFileName, UINT nOpenFlags) : m_hFile(0)

    {
        assert(pszFileName);
        Open(pszFileName, nOpenFlags);  // throws CFileException on failure
    }


    inline CFile::~CFile()
    {
        if (m_hFile != 0)
            ::CloseHandle(m_hFile);
    }


    inline CFile::operator HANDLE() const
    {
        return m_hFile;
    }


    // Closes the file associated with this object. Closed file can no longer be read or written to.
    inline void CFile::Close()
    {

        if (m_hFile != 0)
        {
            if (!::CloseHandle(m_hFile))
            {
                m_hFile = 0;
                throw CFileException(GetFilePath(), _T("Failed to close file"));
            }
        }

        m_hFile = 0;
    }


    // Causes any remaining data in the file buffer to be written to the file.
    inline void CFile::Flush()
    {
        assert(m_hFile);
        if ( !::FlushFileBuffers(m_hFile))
            throw CFileException(GetFilePath(), _T("Failed to flush file"));
    }


    // Returns the file handle associated with this object.
    inline HANDLE CFile::GetHandle() const
    {
        return m_hFile;
    }


    // Returns the directory of the file associated with this object.
    inline CString CFile::GetFileDirectory() const
    {
        CString Directory;

        int sep = m_FilePath.ReverseFind(_T("\\"));
        if (sep > 0)
            Directory = m_FilePath.Left(sep);

        return Directory;
    }


    // Returns the filename of the file associated with this object, not including the directory.
    inline const CString& CFile::GetFileName() const
    {
        return m_FileName;
    }


    // Returns the extension part of the filename of the file associated with this object.
    inline CString CFile::GetFileNameExt() const
    {
        CString Extension;

        int dot = m_FileName.ReverseFind(_T("."));
        if (dot > 1)
            Extension = m_FileName.Mid(dot+1, lstrlen(m_FileName));

        return Extension;
    }


    // Returns the filename of the file associated with this object, not including the directory, without its extension.
    inline CString CFile::GetFileNameWOExt() const
    {
        CString FileNameWOExt = m_FileName;

        int dot = m_FileName.ReverseFind(_T("."));
        if (dot > 0)
            FileNameWOExt = m_FileName.Left(dot);

        return FileNameWOExt;
    }


    // Returns the full filename including the directory of the file associated with this object.
    inline const CString& CFile::GetFilePath() const
    {
        return m_FilePath;
    }


    // Returns the string that the system would use to display the file name to
    // the user. The string might or might not contain the filename's extension
    // depending on user settings.
    inline CString CFile::GetFileTitle() const
    {
        CString FileTitle;
        int nBuffSize = m_FilePath.GetLength();
        if (nBuffSize > 0)
        {
            ::GetFileTitle(m_FilePath, FileTitle.GetBuffer(nBuffSize), (WORD)nBuffSize);
            FileTitle.ReleaseBuffer();
        }

        return FileTitle;
    }


    // Returns the length of the file in bytes.
    inline ULONGLONG CFile::GetLength() const
    {
        assert(m_hFile);

        LONG HighPosCur = 0;
        LONG HighPosEnd = 0;

        DWORD LowPosCur = SetFilePointer(m_hFile, 0, &HighPosCur, FILE_CURRENT);
        DWORD LowPosEnd = SetFilePointer(m_hFile, 0, &HighPosEnd, FILE_END);
        SetFilePointer(m_hFile, LowPosCur, &HighPosCur, FILE_BEGIN);

        ULONGLONG Result = (static_cast<ULONGLONG>(HighPosEnd) << 32) + LowPosEnd;
        return Result;
    }


    // Returns the current value of the file pointer, which can be used in subsequent calls to Seek.
    inline ULONGLONG CFile::GetPosition() const
    {
        assert(m_hFile);
        LONG High = 0;
        DWORD LowPos = SetFilePointer(m_hFile, 0, &High, FILE_CURRENT);

        ULONGLONG Result = (static_cast<ULONGLONG>(High) << 32) + LowPos;
        return Result;
    }

#ifndef _WIN32_WCE

    // Locks a range of bytes in and open file.
    inline void CFile::LockRange(ULONGLONG Pos, ULONGLONG Count)
    {
        assert(m_hFile);

        DWORD dwPosHigh = static_cast<DWORD>(Pos >> 32);
        DWORD dwPosLow = static_cast<DWORD>(Pos & 0xFFFFFFFF);
        DWORD dwCountHigh = static_cast<DWORD>(Count >> 32);
        DWORD dwCountLow = static_cast<DWORD>(Count & 0xFFFFFFFF);

        if (!::LockFile(m_hFile, dwPosLow, dwPosHigh, dwCountLow, dwCountHigh))
            throw CFileException(GetFilePath(), _T("Failed to lock the file"));
    }

#endif

    // Prepares a file to be written to or read from.
    // Possible nOpenFlag values: CREATE_NEW, CREATE_ALWAYS, OPEN_EXISTING, OPEN_ALWAYS, TRUNCATE_EXISTING
    // Default value: OPEN_EXISTING | modeReadWrite
    // The following modes are also supported:
    //  modeCreate      Creates a new file. Truncates an existing file to length 0.
    //  modeNoTruncate  Creates a a new file, or opens an existing one.
    //  modeRead        Requests read access only.
    //  modeWrite       Requests write access only.
    //  modeReadWrite   Requests read and write access.
    //  shareExclusive  Denies read and write access to all others.
    //  shareDenyWrite  Denies write access to all others.
    //  shareDenyRead   Denies read access to all others.
    //  shareDenyNone   No sharing restrictions.
    inline void CFile::Open(LPCTSTR pszFileName, UINT nOpenFlags)
    {
        if (m_hFile != 0) Close();

        DWORD dwAccess = 0;
        switch (nOpenFlags & 0xF00)
        {
        case modeRead:
            dwAccess = GENERIC_READ;    break;
        case modeWrite:
            dwAccess = GENERIC_WRITE;   break;
        case modeReadWrite:
            dwAccess = GENERIC_READ | GENERIC_WRITE; break;
        default:
            dwAccess = GENERIC_READ | GENERIC_WRITE; break;
        }

        DWORD dwShare = 0;
        switch (nOpenFlags & 0xF0)
        {
        case shareExclusive:    dwShare = 0; break;
        case shareDenyWrite:    dwShare = FILE_SHARE_READ;  break;
        case shareDenyRead:     dwShare = FILE_SHARE_WRITE; break;
        case shareDenyNone:     dwShare = FILE_SHARE_READ | FILE_SHARE_WRITE; break;
        default:                dwShare = 0; break;
        }

        DWORD dwCreate = nOpenFlags & 0xF;
        if (dwCreate & OPEN_ALWAYS) nOpenFlags = OPEN_ALWAYS;
        if (dwCreate == 0) dwCreate = OPEN_EXISTING;

        m_hFile = ::CreateFile(pszFileName, dwAccess, dwShare, NULL, dwCreate, FILE_ATTRIBUTE_NORMAL, NULL);

        if (INVALID_HANDLE_VALUE == m_hFile)
        {
            m_hFile = 0;
            throw CFileException(pszFileName, _T("Failed to open file"));
        }

#ifndef _WIN32_WCE
        if (m_hFile != 0)
        {
            SetFilePath(pszFileName);
        }
#endif

    }


    // Reads from the file, storing the contents in the specified buffer.
    inline UINT CFile::Read(void* pBuf, UINT nCount)
    {
        assert(m_hFile);

        if (nCount == 0) return 0;

        assert(pBuf);
        DWORD dwRead = 0;

        if (!::ReadFile(m_hFile, pBuf, nCount, &dwRead, NULL))
            throw CFileException(GetFilePath(), _T("Failed to read from file"));

        return dwRead;
    }


    // Renames the specified file.
    inline void CFile::Rename(LPCTSTR pszOldName, LPCTSTR pszNewName)
    {
        if (!::MoveFile(pszOldName, pszNewName))
            throw CFileException(pszOldName, _T("Failed to rename file"));
    }


    // Deletes the specified file.
    inline void CFile::Remove(LPCTSTR pszFileName)
    {
        if (!::DeleteFile(pszFileName))
            throw CFileException(pszFileName, _T("Failed to delete file"));
    }


    // Positions the current file pointer.
    // Permitted values for nFrom are: FILE_BEGIN, FILE_CURRENT, or FILE_END.
    inline ULONGLONG CFile::Seek(LONGLONG lOff, UINT nFrom)
    {
        assert(m_hFile);
        assert(nFrom == FILE_BEGIN || nFrom == FILE_CURRENT || nFrom == FILE_END);

        LONG High = LONG(lOff >> 32);
        LONG Low = (LONG)(lOff & 0xFFFFFFFF);

        DWORD LowPos = SetFilePointer(m_hFile, Low, &High, nFrom);

        ULONGLONG Result = (static_cast<ULONGLONG>(High) << 32) + LowPos;
        return Result;
    }


    // Sets the current file pointer to the beginning of the file.
    inline void CFile::SeekToBegin()
    {
        assert(m_hFile);
        Seek(0, FILE_BEGIN);
    }


    // Sets the current file pointer to the end of the file.
    inline ULONGLONG CFile::SeekToEnd()
    {
        assert(m_hFile);
        return Seek(0, FILE_END);
    }

#ifndef _WIN32_WCE

    // Assigns the specified full file path to this object.
    // Call this function if the file path is not supplied when the CFile is constructed.
    inline void CFile::SetFilePath(LPCTSTR pszFileName)
    {
        LPTSTR pFileName = NULL;

        int nBuffSize = ::GetFullPathName(pszFileName, 0, 0, 0);
        if (nBuffSize > 0)
        {
            ::GetFullPathName(pszFileName, nBuffSize, m_FilePath.GetBuffer(nBuffSize), &pFileName);

            if (pFileName != NULL)
                m_FileName = pFileName;
            else
                m_FileName = _T("");

            m_FilePath.ReleaseBuffer();
        }
    }

#endif


    // Changes the length of the file to the specified value.
    inline void CFile::SetLength(ULONGLONG NewLen)
    {
        assert(m_hFile);

        Seek(NewLen, FILE_BEGIN);
        if (!::SetEndOfFile(m_hFile))
            throw CFileException(GetFilePath(), _T("Failed to change the file length"));
    }

#ifndef _WIN32_WCE

    // Unlocks a range of bytes in an open file.
    inline void CFile::UnlockRange(ULONGLONG Pos, ULONGLONG Count)
    {
        assert(m_hFile);

        DWORD dwPosHigh = static_cast<DWORD>(Pos >> 32);
        DWORD dwPosLow = static_cast<DWORD>(Pos & 0xFFFFFFFF);
        DWORD dwCountHigh = static_cast<DWORD>(Count >> 32);
        DWORD dwCountLow = static_cast<DWORD>(Count & 0xFFFFFFFF);

        if (!::UnlockFile(m_hFile, dwPosLow, dwPosHigh, dwCountLow, dwCountHigh))
            throw CFileException(GetFilePath(), _T("Failed to unlock the file"));
    }

#endif


    // Writes the specified buffer to the file.
    inline void CFile::Write(const void* pBuf, UINT nCount)
    {
        assert(m_hFile);

        if (nCount == 0) return;

        assert(pBuf);
        DWORD dwWritten = 0;
        if (!::WriteFile(m_hFile, pBuf, nCount, &dwWritten, NULL))
            throw CFileException(GetFilePath(), _T("Failed to write to file"));

        if (dwWritten != nCount)
            throw CFileException(GetFilePath(), _T("Failed to write to file"));
    }


}   // namespace Win32xx

#endif
