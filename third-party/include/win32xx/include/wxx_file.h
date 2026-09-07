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


#ifndef WIN32XX_FILE_H_
#define WIN32XX_FILE_H_


#include "wxx_wincore.h"


namespace Win32xx
{

    //////////////////////////////////////////////////////////
    // The CFile class manages files. It can be used to
    // create, read from, write to, rename, and remove a file.
    class CFile
    {
    public:
        // File open configuration bit-flags
        enum OpenFlags
        {
            modeCreate = CREATE_ALWAYS,    // Creates new file. Truncates existing file to length 0.
            modeNoTruncate = OPEN_ALWAYS,  // Creates new file or opens existing one.
            shareExclusive = 0x0010,       // Denies read/write access to all other processes.
            shareDenyWrite = 0x0020,       // Denies write access to all other processes.
            shareDenyRead = 0x0030,        // Denies read access to all other processes.
            shareDenyNone = 0x0040,        // No sharing restrictions.
            modeRead = 0x0100,             // Requests read access only.
            modeWrite = 0x0200,            // Requests write access only.
            modeReadWrite = 0x0300,        // Requests read and write access.
            modeNone = 0x0400              // Requests neither read nor write access.
        };

        // Lifecycle & Initialization
        CFile();
        CFile(HANDLE file);
        CFile(LPCTSTR fileName, UINT openFlags);
        CFile(LPCTSTR fileName, UINT openFlags, DWORD attributes);
        virtual ~CFile();

        // Implicit conversions and handles
        operator HANDLE() const;
        HANDLE GetHandle() const;

        // Core File Actions
        void Open(LPCTSTR fileName, UINT openFlags, DWORD attributes = FILE_ATTRIBUTE_NORMAL);
        void Close();
        void Flush() const;
        UINT Read(void* buffer, UINT count) const;
        void Write(const void* buffer, UINT count) const;

        // File Position & Size Mechanics
        ULONGLONG Seek(LONGLONG seekTo, UINT method) const;
        void SeekToBegin() const;
        ULONGLONG SeekToEnd() const;
        ULONGLONG GetPosition() const;
        ULONGLONG GetLength() const;
        void SetLength(LONGLONG length) const;

        // Concurrent File Locking
        void LockRange(ULONGLONG pos, ULONGLONG count) const;
        void UnlockRange(ULONGLONG pos, ULONGLONG count) const;

        // File Path Utilities (Object-bound)
        void SetFilePath(LPCTSTR fileName);
        const CString& GetFilePath() const;
        CString GetFileDirectory() const;
        const CString& GetFileName() const;
        CString GetFileNameExt() const;
        CString GetFileNameWOExt() const;

#ifndef WIN32_LEAN_AND_MEAN
        CString GetFileTitle() const;
#endif

        // Static Filesystem Utilities
        static void Rename(LPCTSTR oldName, LPCTSTR newName);
        static void Remove(LPCTSTR fileName);

    private:
        CFile(const CFile&) = delete;
        CFile& operator=(const CFile&) = delete;

        // Internal State Variables
        HANDLE m_file;
        CString m_filePath;
        CString m_fileName;
    };

}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

namespace Win32xx
{

    ///////////////////////////////////
    // Definitions for the CFile class.
    //

    inline CFile::CFile() : m_file(INVALID_HANDLE_VALUE)
    {
    }

    inline CFile::CFile(HANDLE file) : m_file(file)
    {
    }

    // Possible nOpenFlag values: CREATE_NEW, CREATE_ALWAYS, OPEN_EXISTING,
    //                            OPEN_ALWAYS, TRUNCATE_EXISTING.
    // Default value: OPEN_EXISTING | modeReadWrite
    // The following modes are also supported:
    //  modeCreate      Creates a new file. Truncates an existing file to length 0.
    //  modeNoTruncate  Creates a a new file, or opens an existing one.
    //  modeRead        Requests read access only.
    //  modeWrite       Requests write access only.
    //  modeReadWrite   Requests read and write access.
    //  modeNone        Requests neither read nor write access.
    //  shareExclusive  Denies read and write access to all others.
    //  shareDenyWrite  Denies write access to all others.
    //  shareDenyRead   Denies read access to all others.
    //  shareDenyNone   No sharing restrictions.
    // Refer to CreateFile in the Windows API documentation for more information.
    inline CFile::CFile(LPCTSTR fileName, UINT openFlags) : m_file(INVALID_HANDLE_VALUE)
    {
        assert(fileName);
        Open(fileName, openFlags);  // Throws CFileException on failure.
    }

    // Possible attribute values:
    //   FILE_ATTRIBUTE_ARCHIVE, FILE_ATTRIBUTE_ENCRYPTED,
    //   FILE_ATTRIBUTE_HIDDEN, FILE_ATTRIBUTE_NORMAL,
    //   FILE_ATTRIBUTE_NOT_CONTENT_INDEXED, FILE_ATTRIBUTE_OFFLINE,
    //   FILE_ATTRIBUTE_READONLY, FILE_ATTRIBUTE_SYSTEM,
    //   FILE_ATTRIBUTE_TEMPORARY, FILE_FLAG_BACKUP_SEMANTICS,
    //   FILE_FLAG_DELETE_ON_CLOSE, FILE_FLAG_NO_BUFFERING,
    //   FILE_FLAG_OPEN_NO_RECALL, FILE_FLAG_OPEN_REPARSE_POINT,
    //   FILE_FLAG_OVERLAPPED, FILE_FLAG_POSIX_SEMANTICS,
    //   FILE_FLAG_RANDOM_ACCESS, FILE_FLAG_SEQUENTIAL_SCAN,
    //   FILE_FLAG_WRITE_THROUGH.
    // Refer to CreateFile in the Windows API documentation for more information.
    inline CFile::CFile(LPCTSTR fileName, UINT openFlags, DWORD attributes) : m_file(INVALID_HANDLE_VALUE)
    {
        assert(fileName);
        Open(fileName, openFlags, attributes);  // Throws CFileException on failure.
    }

    inline CFile::~CFile()
    {
        if (m_file != INVALID_HANDLE_VALUE)
            ::CloseHandle(m_file);
    }

    inline CFile::operator HANDLE() const
    {
        return m_file;
    }

    // Closes the file associated with this object. Closed file can no longer
    // be read or written to.
    // Refer to CloseHandle in the Windows API documentation for more information.
    inline void CFile::Close()
    {
        if (m_file != INVALID_HANDLE_VALUE)
        {
            if (!::CloseHandle(m_file))
            {
                m_file = INVALID_HANDLE_VALUE;
                CString failedPath = m_filePath;
                m_fileName.Empty();
                m_filePath.Empty();
                throw CFileException(failedPath, GetApp()->MsgFileClose());
            }
        }
        m_fileName.Empty();
        m_filePath.Empty();
        m_file = INVALID_HANDLE_VALUE;
    }

    // Causes any remaining data in the file buffer to be written to the file.
    // Refer to FlushFileBuffers in the Windows API documentation for more information.
    inline void CFile::Flush() const
    {
        assert(m_file != INVALID_HANDLE_VALUE);
        if (!::FlushFileBuffers(m_file))
            throw CFileException(GetFilePath(), GetApp()->MsgFileFlush());
    }

    // Returns the directory of the file associated with this object.
    inline CString CFile::GetFileDirectory() const
    {
        CString directory;
        int sep = m_filePath.ReverseFind(_T('\\'));
        if (sep > 0)
        {
            if (sep == 2 && m_filePath.GetAt(1) == _T(':'))
                directory = m_filePath.Left(sep + 1); // Preserves C:\ style structures
            else
                directory = m_filePath.Left(sep);
        }
        return directory;
    }

    // Returns the filename of the file associated with this object, not
    // including the directory.
    inline const CString& CFile::GetFileName() const
    {
        return m_fileName;
    }

    // Returns the extension part of the filename of the file associated with
    // this object.
    inline CString CFile::GetFileNameExt() const
    {
        CString extension;
        int dot = m_fileName.ReverseFind(_T('.'));
        if (dot >= 0)
            extension = m_fileName.Mid(dot + 1);

        return extension;
    }

    // Returns the filename of the file associated with this object, not
    // including the directory, without its extension.
    inline CString CFile::GetFileNameWOExt() const
    {
        CString fileNameWOExt = m_fileName;
        int dot = m_fileName.ReverseFind(_T('.'));
        int sep = m_fileName.ReverseFind(_T('\\'));

        if (dot > sep)
            fileNameWOExt = m_fileName.Left(dot);
        return fileNameWOExt;
    }

    // Returns the full filename including the directory of the file associated
    // with this object.
    inline const CString& CFile::GetFilePath() const
    {
        return m_filePath;
    }

#ifndef WIN32_LEAN_AND_MEAN
    // Returns the string that the system would use to display the file name to
    // the user. The string might or might not contain the filename's extension
    // depending on user settings.
    // Refer to GetFileTitle in the Windows API documentation for more information.
    inline CString CFile::GetFileTitle() const
    {
        CString fileTitle;
        LPTSTR pBuffer = fileTitle.GetBuffer(MAX_PATH);
        if (::GetFileTitle(m_filePath, pBuffer, MAX_PATH) == 0)
            fileTitle.ReleaseBuffer();
        else
        {
            fileTitle.ReleaseBuffer();
            fileTitle.Empty();
        }
        return fileTitle;
    }
#endif


    // Returns the file handle associated with this object.
    inline HANDLE CFile::GetHandle() const
    {
        return m_file;
    }

    // Returns the length of the file in bytes.
    // Refer to GetFileSizeEx in the Windows API documentation for more information.
    inline ULONGLONG CFile::GetLength() const
    {
        assert(m_file != INVALID_HANDLE_VALUE);

        LARGE_INTEGER fileSize;
        if (!::GetFileSizeEx(m_file, &fileSize))
            throw CFileException(m_fileName, GetApp()->MsgFileRead());

        return static_cast<ULONGLONG>(fileSize.QuadPart);
    }

    // Returns the current value of the file pointer that can be used in
    // subsequent calls to Seek.
    // Refer to SetFilePointer in the Windows API documentation for more information.
    inline ULONGLONG CFile::GetPosition() const
    {
        return Seek(0, FILE_CURRENT);
    }

    // Locks a range of bytes in and open file.
    // Refer to LockFile in the Windows API documentation for more information.
    inline void CFile::LockRange(ULONGLONG pos, ULONGLONG count) const
    {
        assert(m_file != INVALID_HANDLE_VALUE);

        DWORD posHigh = static_cast<DWORD>(pos >> 32);
        DWORD posLow = static_cast<DWORD>(pos & 0xFFFFFFFF);
        DWORD countHigh = static_cast<DWORD>(count >> 32);
        DWORD countLow = static_cast<DWORD>(count & 0xFFFFFFFF);

        if (!::LockFile(m_file, posLow, posHigh, countLow, countHigh))
            throw CFileException(GetFilePath(), GetApp()->MsgFileLock());
    }

    // Prepares a file to be written to or read from.
    // Possible openFlag values: CREATE_NEW, CREATE_ALWAYS, OPEN_EXISTING,
    //                           OPEN_ALWAYS, TRUNCATE_EXISTING
    // Default value: OPEN_EXISTING | modeReadWrite
    // The following modes are also supported:
    //  modeCreate      Creates a new file. Truncates an existing file to length 0.
    //  modeNoTruncate  Creates a a new file, or opens an existing one.
    //  modeRead        Requests read access only.
    //  modeWrite       Requests write access only.
    //  modeReadWrite   Requests read and write access.
    //  modeNone        Requests neither read nor write access.
    //  shareExclusive  Denies read and write access to all others.
    //  shareDenyWrite  Denies write access to all others.
    //  shareDenyRead   Denies read access to all others.
    //  shareDenyNone   No sharing restrictions.
    // Possible attribute values:
    //   FILE_ATTRIBUTE_ARCHIVE, FILE_ATTRIBUTE_ENCRYPTED,
    //   FILE_ATTRIBUTE_HIDDEN, FILE_ATTRIBUTE_NORMAL,
    //   FILE_ATTRIBUTE_NOT_CONTENT_INDEXED, FILE_ATTRIBUTE_OFFLINE,
    //   FILE_ATTRIBUTE_READONLY, FILE_ATTRIBUTE_SYSTEM,
    //   FILE_ATTRIBUTE_TEMPORARY, FILE_FLAG_BACKUP_SEMANTICS,
    //   FILE_FLAG_DELETE_ON_CLOSE, FILE_FLAG_NO_BUFFERING,
    //   FILE_FLAG_OPEN_NO_RECALL, FILE_FLAG_OPEN_REPARSE_POINT,
    //   FILE_FLAG_OVERLAPPED, FILE_FLAG_POSIX_SEMANTICS,
    //   FILE_FLAG_RANDOM_ACCESS, FILE_FLAG_SEQUENTIAL_SCAN,
    //   FILE_FLAG_WRITE_THROUGH.
    // Refer to CreateFile in the Windows API documentation for more information.
    inline void CFile::Open(LPCTSTR fileName, UINT openFlags, DWORD attributes)
    {
        if (m_file != INVALID_HANDLE_VALUE)
            Close();

        DWORD access = 0;
        switch (openFlags & 0xF00)
        {
        case modeNone:      access = 0; break;
        case modeRead:      access = GENERIC_READ;    break;
        case modeWrite:     access = GENERIC_WRITE;   break;
        case modeReadWrite: access = GENERIC_READ | GENERIC_WRITE; break;
        default:            access = GENERIC_READ | GENERIC_WRITE; break;
        }

        DWORD share = 0;
        switch (openFlags & 0xF0)
        {
        case shareExclusive:    share = 0; break;
        case shareDenyWrite:    share = FILE_SHARE_READ;  break;
        case shareDenyRead:     share = FILE_SHARE_WRITE; break;
        case shareDenyNone:     share = FILE_SHARE_READ | FILE_SHARE_WRITE; break;
        default:                share = 0; break;
        }

        DWORD create = openFlags & 0xF;
        if (create == 0) create = OPEN_EXISTING;

        m_file = ::CreateFile(fileName, access, share, nullptr, create, attributes, 0);

        if (m_file == INVALID_HANDLE_VALUE)
        {
            throw CFileException(fileName, GetApp()->MsgFileOpen());
        }

        SetFilePath(fileName);
    }

    // Reads from the file, storing the contents in the specified buffer.
    // Refer to ReadFile in the Windows API documentation for more information.
    inline UINT CFile::Read(void* buffer, UINT count) const
    {
        assert(m_file != INVALID_HANDLE_VALUE);

        if (count == 0) return 0;

        assert(buffer);
        DWORD read = 0;

        if (!::ReadFile(m_file, buffer, count, &read, nullptr))
        {
            DWORD error = ::GetLastError();
            if (error != ERROR_IO_PENDING)
            {
                throw CFileException(GetFilePath(), GetApp()->MsgFileRead());
            }
        }
        return read;
    }

    // Renames the specified file.
    // Refer to MoveFile in the Windows API documentation for more information.
    inline void CFile::Rename(LPCTSTR oldName, LPCTSTR newName)
    {
        if (!::MoveFile(oldName, newName))
            throw CFileException(oldName, GetApp()->MsgFileRename());
    }

    // Deletes the specified file.
    // Refer to DeleteFile in the Windows API documentation for more information.
    inline void CFile::Remove(LPCTSTR fileName)
    {
        if (!::DeleteFile(fileName))
            throw CFileException(fileName, GetApp()->MsgFileRemove());
    }

    // Positions the current file pointer.
    // Permitted values for method are: FILE_BEGIN, FILE_CURRENT, or FILE_END.
    // Refer to SetFilePointer in the Windows API documentation for more information.
    inline ULONGLONG CFile::Seek(LONGLONG seekTo, UINT method) const
    {
        assert(m_file != INVALID_HANDLE_VALUE);
        assert(method == FILE_BEGIN || method == FILE_CURRENT || method == FILE_END);

        LARGE_INTEGER distanceToMove;
        distanceToMove.QuadPart = seekTo;
        LARGE_INTEGER newFilePointer;

        if (!::SetFilePointerEx(m_file, distanceToMove, &newFilePointer, method))
            throw CFileException(GetFilePath(), GetApp()->MsgFileSeek());

        return static_cast<ULONGLONG>(newFilePointer.QuadPart);
    }

    // Sets the current file pointer to the beginning of the file.
    // Refer to Seek in the Windows API documentation for more information.
    inline void CFile::SeekToBegin() const
    {
        Seek(0, FILE_BEGIN);
    }

    // Sets the current file pointer to the end of the file.
    // Refer to Seek in the Windows API documentation for more information.
    inline ULONGLONG CFile::SeekToEnd() const
    {
        return Seek(0, FILE_END);
    }

    // Assigns the specified full file path to this object.
    // Call this function if the file path is not supplied when the CFile is
    // constructed.
    // Note: this function does not open or create the specified file.
    inline void CFile::SetFilePath(LPCTSTR fileName)
    {
        LPTSTR pFileName = nullptr;
        DWORD buffSize = ::GetFullPathName(fileName, 0, nullptr, nullptr);

        if (buffSize > 0)
        {
            LPTSTR pBuffer = m_filePath.GetBuffer(buffSize);
            ::GetFullPathName(fileName, buffSize, pBuffer, &pFileName);

            // Extract the filename before releasing the buffer.
            if (pFileName != nullptr)
                m_fileName = pFileName;
            else
                m_fileName = _T("");

            m_filePath.ReleaseBuffer();
        }
        else
        {
            m_filePath = _T("");
            m_fileName = _T("");
        }
    }

    // Changes the length of the file to the specified value.
    // Refer to SetEndOfFile in the Windows API documentation for more information.
    inline void CFile::SetLength(LONGLONG length) const
    {
        assert(m_file != INVALID_HANDLE_VALUE);

        ULONGLONG currentPos = GetPosition();
        Seek(length, FILE_BEGIN);

        BOOL bSuccess = ::SetEndOfFile(m_file);
        Seek(static_cast<LONGLONG>(currentPos), FILE_BEGIN);

        if (!bSuccess)
            throw CFileException(GetFilePath(), GetApp()->MsgFileLength());
    }

    // Unlocks a range of bytes in an open file.
    // Refer to UnlockFile in the Windows API documentation for more information.
    inline void CFile::UnlockRange(ULONGLONG pos, ULONGLONG count) const
    {
        assert(m_file != INVALID_HANDLE_VALUE);

        DWORD posHigh = static_cast<DWORD>(pos >> 32);
        DWORD posLow = static_cast<DWORD>(pos & 0xFFFFFFFF);
        DWORD countHigh = static_cast<DWORD>(count >> 32);
        DWORD countLow = static_cast<DWORD>(count & 0xFFFFFFFF);

        if (!::UnlockFile(m_file, posLow, posHigh, countLow, countHigh))
            throw CFileException(GetFilePath(), GetApp()->MsgFileUnlock());
    }

    // Writes the specified buffer to the file.
    // Refer to WriteFile in the Windows API documentation for more information.
    inline void CFile::Write(const void* buffer, UINT count) const
    {
        assert(m_file != INVALID_HANDLE_VALUE);

        if (count == 0) return;

        assert(buffer);
        DWORD written = 0;
        if (!::WriteFile(m_file, buffer, count, &written, nullptr))
            throw CFileException(GetFilePath(), GetApp()->MsgFileWrite());

        if (written != count)
            throw CFileException(GetFilePath(), GetApp()->MsgFileWrite());
    }

} // namespace Win32xx

#endif // WIN32XX_FILE_H_
