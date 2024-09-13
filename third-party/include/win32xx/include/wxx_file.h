// Win32++   Version 10.0.0
// Release Date: 9th September 2024
//
//      David Nash
//      email: dnash@bigpond.net.au
//      url: https://sourceforge.net/projects/win32-framework
//           https://github.com/DavidNash2024/Win32xx
//
//
// Copyright (c) 2005-2024  David Nash
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


#include "wxx_wincore.h"


namespace Win32xx
{

    //////////////////////////////////////////////////////////
    // The CFile class manages files. It can be used to
    // create, read from, write to, rename, and remove a file.
    class CFile
    {
    public:
        // File open flags.
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
            modeReadWrite =     0x0300, // Requests read and write access.
            modeNone =          0x0400  // Requests neither read nor write access.
        };

        CFile();
        CFile(HANDLE file);
        CFile(LPCTSTR fileName, UINT openFlags);
        CFile(LPCTSTR fileName, UINT openFlags, DWORD attributes);
        virtual ~CFile();
        operator HANDLE() const;

        void Close();
        void Flush() const;
        CString GetFileDirectory() const;
        const CString& GetFileName() const;
        CString GetFileNameExt() const;
        CString GetFileNameWOExt() const;
        const CString& GetFilePath() const;
#ifndef WIN32_LEAN_AND_MEAN
        CString GetFileTitle() const;
#endif
        HANDLE GetHandle() const;
        ULONGLONG GetLength() const;
        ULONGLONG GetPosition() const;
        void LockRange(ULONGLONG pos, ULONGLONG count) const;
        void Open(LPCTSTR fileName, UINT openFlags,
                  DWORD attributes = FILE_ATTRIBUTE_NORMAL);
        UINT Read(void* buffer, UINT count) const;
        void Remove(LPCTSTR fileName) const;
        void Rename(LPCTSTR oldName, LPCTSTR newName) const;
        ULONGLONG Seek(LONGLONG seekTo, UINT method) const;
        void SeekToBegin() const;
        ULONGLONG SeekToEnd() const;
        void SetFilePath(LPCTSTR fileName);
        void SetLength(LONGLONG length) const;
        void UnlockRange(ULONGLONG pos, ULONGLONG count) const;
        void Write(const void* buffer, UINT count) const;

    private:
        CFile(const CFile&) = delete;
        CFile& operator=(const CFile&) = delete;

        CString m_fileName;
        CString m_filePath;
        HANDLE m_file;
    };

}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

namespace Win32xx
{
    inline CFile::CFile() : m_file(INVALID_HANDLE_VALUE)
    {
    }

    inline CFile::CFile(HANDLE file) : m_file(file)
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
    //   FILE_ATTRIBUTE_ARCHIVE, FILE_ATTRIBUTE_ENCRYPTED, FILE_ATTRIBUTE_HIDDEN, FILE_ATTRIBUTE_NORMAL,
    //   FILE_ATTRIBUTE_NOT_CONTENT_INDEXED, FILE_ATTRIBUTE_OFFLINE, FILE_ATTRIBUTE_READONLY, FILE_ATTRIBUTE_SYSTEM,
    //   FILE_ATTRIBUTE_TEMPORARY, FILE_FLAG_BACKUP_SEMANTICS, FILE_FLAG_DELETE_ON_CLOSE, FILE_FLAG_NO_BUFFERING,
    //   FILE_FLAG_OPEN_NO_RECALL, FILE_FLAG_OPEN_REPARSE_POINT, FILE_FLAG_OVERLAPPED, FILE_FLAG_POSIX_SEMANTICS,
    //   FILE_FLAG_RANDOM_ACCESS, FILE_FLAG_SEQUENTIAL_SCAN, FILE_FLAG_WRITE_THROUGH.
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

    // Closes the file associated with this object. Closed file can no longer be read or written to.
    // Refer to CloseHandle in the Windows API documentation for more information.
    inline void CFile::Close()
    {
        m_fileName.Empty();
        m_filePath.Empty();

        if (m_file != INVALID_HANDLE_VALUE)
        {
            if (!::CloseHandle(m_file))
            {
                m_file = INVALID_HANDLE_VALUE;
                throw CFileException(GetFilePath(), GetApp()->MsgFileClose());
            }
        }

        m_file = INVALID_HANDLE_VALUE;
    }

    // Causes any remaining data in the file buffer to be written to the file.
    // Refer to FlushFileBuffers in the Windows API documentation for more information.
    inline void CFile::Flush() const
    {
        assert(m_file != INVALID_HANDLE_VALUE);
        if ( !::FlushFileBuffers(m_file))
            throw CFileException(GetFilePath(), GetApp()->MsgFileFlush());
    }

    // Returns the file handle associated with this object.
    inline HANDLE CFile::GetHandle() const
    {
        return m_file;
    }

    // Returns the directory of the file associated with this object.
    inline CString CFile::GetFileDirectory() const
    {
        CString directory;

        int sep = m_filePath.ReverseFind(_T('\\'));
        if (sep > 0)
            directory = m_filePath.Left(sep);

        return directory;
    }

    // Returns the filename of the file associated with this object, not including the directory.
    inline const CString& CFile::GetFileName() const
    {
        return m_fileName;
    }

    // Returns the extension part of the filename of the file associated with this object.
    inline CString CFile::GetFileNameExt() const
    {
        CString extension;
        int dot = m_fileName.ReverseFind(_T('.'));
        if (dot >= 0)
            extension = m_fileName.Mid(dot+1);

        return extension;
    }

    // Returns the filename of the file associated with this object, not including the directory, without its extension.
    inline CString CFile::GetFileNameWOExt() const
    {
        CString fileNameWOExt = m_fileName;
        int dot = m_fileName.ReverseFind(_T('.'));
        if (dot >= 0)
            fileNameWOExt = m_fileName.Left(dot);

        return fileNameWOExt;
    }

    // Returns the full filename including the directory of the file associated with this object.
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
        int buffSize = m_filePath.GetLength();
        if (buffSize > 0)
        {
            ::GetFileTitle(m_filePath, fileTitle.GetBuffer(buffSize), static_cast<WORD>(buffSize));
            fileTitle.ReleaseBuffer();
        }

        return fileTitle;
    }
#endif

    // Returns the length of the file in bytes.
    // Refer to SetFilePointer in the Windows API documentation for more information.
    inline ULONGLONG CFile::GetLength() const
    {
        assert(m_file != INVALID_HANDLE_VALUE);

        LONG highPosCur = 0;
        LONG highPosEnd = 0;

        DWORD lowPosCur = SetFilePointer(m_file, 0, &highPosCur, FILE_CURRENT);
        DWORD lowPosEnd = SetFilePointer(m_file, 0, &highPosEnd, FILE_END);
        SetFilePointer(m_file, static_cast<LONG>(lowPosCur), &highPosCur, FILE_BEGIN);

        ULONGLONG result = (static_cast<ULONGLONG>(highPosEnd) << 32) + lowPosEnd;
        return result;
    }

    // Returns the current value of the file pointer that can be used in subsequent calls to Seek.
    // Refer to SetFilePointer in the Windows API documentation for more information.
    inline ULONGLONG CFile::GetPosition() const
    {
        assert(m_file != INVALID_HANDLE_VALUE);
        LONG high = 0;
        DWORD lowPos = SetFilePointer(m_file, 0, &high, FILE_CURRENT);

        ULONGLONG result = (static_cast<ULONGLONG>(high) << 32) + lowPos;
        return result;
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
    // Possible openFlag values: CREATE_NEW, CREATE_ALWAYS, OPEN_EXISTING, OPEN_ALWAYS, TRUNCATE_EXISTING
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
    //   FILE_ATTRIBUTE_ARCHIVE, FILE_ATTRIBUTE_ENCRYPTED, FILE_ATTRIBUTE_HIDDEN, FILE_ATTRIBUTE_NORMAL,
    //   FILE_ATTRIBUTE_NOT_CONTENT_INDEXED, FILE_ATTRIBUTE_OFFLINE, FILE_ATTRIBUTE_READONLY, FILE_ATTRIBUTE_SYSTEM,
    //   FILE_ATTRIBUTE_TEMPORARY, FILE_FLAG_BACKUP_SEMANTICS, FILE_FLAG_DELETE_ON_CLOSE, FILE_FLAG_NO_BUFFERING,
    //   FILE_FLAG_OPEN_NO_RECALL, FILE_FLAG_OPEN_REPARSE_POINT, FILE_FLAG_OVERLAPPED, FILE_FLAG_POSIX_SEMANTICS,
    //   FILE_FLAG_RANDOM_ACCESS, FILE_FLAG_SEQUENTIAL_SCAN, FILE_FLAG_WRITE_THROUGH.
    // Refer to CreateFile in the Windows API documentation for more information.
    inline void CFile::Open(LPCTSTR fileName, UINT openFlags, DWORD attributes)
    {
        if (m_file != INVALID_HANDLE_VALUE)
            Close();

        DWORD access = 0;
        switch (openFlags & 0xF00)
        {
        case modeNone:
            access = 0; break;
        case modeRead:
            access = GENERIC_READ;    break;
        case modeWrite:
            access = GENERIC_WRITE;   break;
        case modeReadWrite:
            access = GENERIC_READ | GENERIC_WRITE; break;
        default:
            access = GENERIC_READ | GENERIC_WRITE; break;
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
        if (create & OPEN_ALWAYS) openFlags = OPEN_ALWAYS;
        if (create == 0) create = OPEN_EXISTING;

        m_file = ::CreateFile(fileName, access, share, nullptr, create, attributes, 0);

        if (INVALID_HANDLE_VALUE == m_file)
        {
            throw CFileException(fileName, GetApp()->MsgFileOpen());
        }

        if (m_file != INVALID_HANDLE_VALUE)
        {
            SetFilePath(fileName);
        }

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
            throw CFileException(GetFilePath(), GetApp()->MsgFileRead());

        return read;
    }

    // Renames the specified file.
    // Refer to MoveFile in the Windows API documentation for more information.
    inline void CFile::Rename(LPCTSTR oldName, LPCTSTR newName) const
    {
        if (!::MoveFile(oldName, newName))
            throw CFileException(oldName, GetApp()->MsgFileRename());
    }

    // Deletes the specified file.
    // Refer to DeleteFile in the Windows API documentation for more information.
    inline void CFile::Remove(LPCTSTR fileName) const
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

        LONG high = static_cast<LONG>(seekTo >> 32);
        LONG low  = static_cast<LONG>(seekTo & 0xFFFFFFFF);

        DWORD lowPos = SetFilePointer(m_file, low, &high, method);

        ULONGLONG result = (static_cast<ULONGLONG>(high) << 32) + lowPos;
        return result;
    }

    // Sets the current file pointer to the beginning of the file.
    // Refer to Seek in the Windows API documentation for more information.
    inline void CFile::SeekToBegin() const
    {
        assert(m_file != INVALID_HANDLE_VALUE);
        Seek(0, FILE_BEGIN);
    }

    // Sets the current file pointer to the end of the file.
    // Refer to Seek in the Windows API documentation for more information.
    inline ULONGLONG CFile::SeekToEnd() const
    {
        assert(m_file != INVALID_HANDLE_VALUE);
        return Seek(0, FILE_END);
    }

    // Assigns the specified full file path to this object.
    // Call this function if the file path is not supplied when the CFile is constructed.
    // Note: this function does not open or create the specified file.
    inline void CFile::SetFilePath(LPCTSTR fileName)
    {
        LPTSTR pShortFileName = nullptr;

        DWORD buffSize = ::GetFullPathName(fileName, 0, 0, 0);
        int buffer = static_cast<int>(buffSize);
        if (buffer > 0)
        {
            ::GetFullPathName(fileName, buffSize, m_filePath.GetBuffer(buffer), &pShortFileName);

            if (pShortFileName != nullptr)
                m_fileName = pShortFileName;
            else
                m_fileName = _T("");

            m_filePath.ReleaseBuffer();
        }
    }

    // Changes the length of the file to the specified value.
    // Refer to SetEndOfFile in the Windows API documentation for more information.
    inline void CFile::SetLength(LONGLONG length) const
    {
        assert(m_file != INVALID_HANDLE_VALUE);

        Seek(length, FILE_BEGIN);
        if (!::SetEndOfFile(m_file))
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

}   // namespace Win32xx

#endif
