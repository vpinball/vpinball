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


////////////////////////////////////////////////////////
// Acknowledgement:
//
// The original author of CFileFind is:
//
//      Robert C. Tausworthe
//      email: robert.c.tausworthe@ieee.org
//
////////////////////////////////////////////////////////


////////////////////////////////////////////////////////
// The CFileFind provides a means of finding one or more
// files that match a search string. Information can be
// extracted from each file found.


// Example code
/*

CFileFind ff;
if (ff.FindFirstFile(_T("C:\\SomeFolder\\*.*")))
{
    do
    {
        // Do something with each file found
        std::cout << ff.GetFilePath() << std::endl;
    }
    while (ff.FindNextFile());
}

*/


#ifndef _WIN32XX_FILEFIND_H_
#define _WIN32XX_FILEFIND_H_

#include "wxx_wincore.h"


namespace Win32xx
{

    ////////////////////////////////////////////////////////
    // CFindFile finds one or more files matching the string
    // specified by FindFirstFile.
    class CFileFind
    {
     public:
         CFileFind();
         virtual ~CFileFind();

         BOOL        FindFirstFile(LPCTSTR fileSearch = NULL);
         BOOL        FindNextFile();
         FILETIME    GetCreationTime() const;
         DWORD       GetFileAttributes() const;
         CString     GetFileName() const;
         CString     GetFilePath() const;
         CString     GetFileTitle() const;
         CString     GetFileURL() const;
         FILETIME    GetLastAccessTime() const;
         FILETIME    GetLastWriteTime() const;
         ULONGLONG   GetLength() const;
         CString     GetRoot() const;
         BOOL        IsArchived() const;
         BOOL        IsCompressed() const;
         BOOL        IsDirectory() const;
         BOOL        IsDots() const;
         BOOL        IsHidden() const;
         BOOL        IsNormal() const;
         BOOL        IsReadOnly() const;
         BOOL        IsSystem() const;
         BOOL        IsTemporary() const;

     private:
         void    Close();

         WIN32_FIND_DATA m_findData;
         HANDLE      m_fileFind;
         CString     m_root;
    };

}



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


namespace Win32xx
{

    //////////////////////////////////////
    // Definitions for the CFindFile class
    //

    //  Construct a CFileFind object for file searches.
    inline CFileFind::CFileFind()
    {
        m_fileFind = INVALID_HANDLE_VALUE;
        ZeroMemory(&m_findData, sizeof(m_findData));
    }

    inline CFileFind::~CFileFind()
    {
        Close();
    }

    //  Closes the FileFind handle and returns the CFileFind object to default.
    inline void CFileFind::Close()
    {
        ZeroMemory(&m_findData, sizeof(m_findData));

        if (m_fileFind != INVALID_HANDLE_VALUE)
            ::FindClose(m_fileFind);

        m_root.Empty();
        m_fileFind = INVALID_HANDLE_VALUE;
    }

    //  Searches a directory for a file or subdirectory with a name that matches
    //  the fileSearch. The fileSearch can contain a valid directory or path, and a
    //  file name that can contain the '?' and '*' wildcard characters.
    //  A name of "*.*" is used if no name is specified.
    //  Returns TRUE if a matching file was found, FALSE otherwise.
    //  Refer to FindFirstFile in the Windows API documentation for more information.
    inline BOOL CFileFind::FindFirstFile(LPCTSTR fileSearch /* = NULL */)
    {
        // Reset the this to default
        Close();

        // Default the name to all files in the current directory
        if (fileSearch == NULL)
            fileSearch = _T("*.*");

        // Search for the first file matching the name
        m_fileFind = ::FindFirstFile(fileSearch, &m_findData);

        if (m_fileFind == INVALID_HANDLE_VALUE)
        {
            Close();
            return FALSE;
        }

        // extract the directory part of the name (if any)
        CString str = fileSearch;
        int delimiter = str.ReverseFind(_T('\\'));
        if (delimiter >= 0)
        {
            m_root = str.Left(delimiter);
            m_root += _T('\\');
        }

        return TRUE;
    }

    //  Finds the next file or directory that matches the string specified in FindFirstFile.
    //  Return TRUE if the next file was found, FALSE on failure.
    //  Refer to FindNextFile in the Windows API documentation for more information.
    inline BOOL CFileFind::FindNextFile()
    {
        assert(m_fileFind != INVALID_HANDLE_VALUE);

        BOOL isFound = ::FindNextFile(m_fileFind, &m_findData);

        if (!isFound)
            Close();

        return isFound;
    }

    // Return the found file's creation time
    inline FILETIME CFileFind::GetCreationTime() const
    {
        assert(m_fileFind != INVALID_HANDLE_VALUE);
        return m_findData.ftCreationTime;
    }

    //  Returns the found file's attributes. Possible attributes are:
    //  FILE_ATTRIBUTE_ARCHIVE; FILE_ATTRIBUTE_COMPRESSED; FILE_ATTRIBUTE_DIRECTORY;
    //  FILE_ATTRIBUTE_ENCRYPTED; FILE_ATTRIBUTE_HIDDEN; FILE_ATTRIBUTE_NORMAL;
    //  FILE_ATTRIBUTE_OFFLINE; FILE_ATTRIBUTE_READONLY; FILE_ATTRIBUTE_REPARSE_POINT;
    //  FILE_ATTRIBUTE_SPARSE_FILE; FILE_ATTRIBUTE_SYSTEM; FILE_ATTRIBUTE_TEMPORARY.
    inline DWORD CFileFind::GetFileAttributes() const
    {
        assert(m_fileFind != INVALID_HANDLE_VALUE);
        return m_findData.dwFileAttributes;
    }

    //  Return the found file's name, including the file's extension.
    inline CString CFileFind::GetFileName() const
    {
        assert(m_fileFind != INVALID_HANDLE_VALUE);
        return m_findData.cFileName;
    }

    //  Returns the full path of the found file, including the directory,
    //  file title, and extension.
    //  Refer to GetFullPathName in the Windows API documentation for more information.
    inline CString CFileFind::GetFilePath() const
    {
        assert(m_fileFind != INVALID_HANDLE_VALUE);

        CString searchName = m_root + m_findData.cFileName;
        CString filePath;

        int buffSize = static_cast<int>(::GetFullPathName(searchName, 0, 0, 0));
        if (buffSize > 0)
        {
            ::GetFullPathName(searchName, static_cast<DWORD>(buffSize), filePath.GetBuffer(buffSize), 0);
            filePath.ReleaseBuffer();
        }

        return filePath;
    }

    //  Return the file name, without the extension.
    inline CString CFileFind::GetFileTitle() const
    {
        assert(m_fileFind != INVALID_HANDLE_VALUE);

        CString name = m_findData.cFileName;
        if (!IsDots())
        {
            int dot = name.ReverseFind(_T('.'));
            if (dot >= 0)
                name = name.Left(dot);
        }
        return name;
    }

    //  Return the URL form of the path name, viz., file://path
    inline CString CFileFind::GetFileURL() const
    {
        assert(m_fileFind != INVALID_HANDLE_VALUE);

        CString str(_T("file://"));
        str += GetFilePath();
        return str;
    }

    //  Return the last access time of the found file
    inline FILETIME CFileFind::GetLastAccessTime() const
    {
        assert(m_fileFind != INVALID_HANDLE_VALUE);
        return m_findData.ftLastAccessTime;
    }

    //  Return the last write time of the found file
    inline FILETIME CFileFind::GetLastWriteTime() const
    {
        assert(m_fileFind != INVALID_HANDLE_VALUE);
        return m_findData.ftLastWriteTime;
    }

    //  Return the length of the found file, in bytes.
    inline ULONGLONG CFileFind::GetLength() const
    {
        assert(m_fileFind != INVALID_HANDLE_VALUE);
        ULONGLONG length = m_findData.nFileSizeHigh;
        length <<= 32;
        return length | m_findData.nFileSizeLow;
    }

    //  Return the directory part of the name search string.
    inline CString CFileFind::GetRoot() const
    {
        assert(m_fileFind != INVALID_HANDLE_VALUE);
        return m_root;
    }

    //  Return TRUE if the archive attribute is set.
    inline BOOL CFileFind::IsArchived() const
    {
        assert(m_fileFind != INVALID_HANDLE_VALUE);
        return (m_findData.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE) ? TRUE : FALSE;
    }

    //  Return TRUE if the found file is compressed.
    inline BOOL CFileFind::IsCompressed() const
    {
        assert(m_fileFind != INVALID_HANDLE_VALUE);
        return (m_findData.dwFileAttributes & FILE_ATTRIBUTE_COMPRESSED) ? TRUE : FALSE;
    }

    //  Return TRUE if the found file is a directory.
    inline BOOL CFileFind::IsDirectory() const
    {
        assert(m_fileFind != INVALID_HANDLE_VALUE);
        return (m_findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? TRUE : FALSE;
    }

    //  Return TRUE if the found file is the current directory or parent directory.
    inline BOOL CFileFind::IsDots() const
    {
        assert(m_fileFind != INVALID_HANDLE_VALUE);
        LPCTSTR fn = m_findData.cFileName;
        return (IsDirectory() && (fn[0] == _T('.')) && ((fn[1] == _T('\0') ||
            (fn[1] == _T('.') && fn[2] == _T('\0')))));
    }

    //  Return TRUE if the found file is hidden.
    inline BOOL CFileFind::IsHidden() const
    {
        assert(m_fileFind != INVALID_HANDLE_VALUE);
        return (m_findData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) ? TRUE : FALSE;
    }

    //  Return TRUE if the found file has the FILE_ATTRIBUTE_NORMAL attribute.
    inline BOOL CFileFind::IsNormal() const
    {
        assert(m_fileFind != INVALID_HANDLE_VALUE);
        return (m_findData.dwFileAttributes & FILE_ATTRIBUTE_NORMAL) ? TRUE : FALSE;
    }

    //  Return TRUE if the found file is read only.
    inline BOOL CFileFind::IsReadOnly() const
    {
        assert(m_fileFind != INVALID_HANDLE_VALUE);
        return (m_findData.dwFileAttributes & FILE_ATTRIBUTE_READONLY) ? TRUE : FALSE;
    }

    //  Return TRUE if the found file is a system file.
    inline BOOL CFileFind::IsSystem() const
    {
        assert(m_fileFind != INVALID_HANDLE_VALUE);
        return (m_findData.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) ? TRUE : FALSE;
    }

    //  Return TRUE if the found file is a temporary file.
    inline BOOL CFileFind::IsTemporary() const
    {
        assert(m_fileFind != INVALID_HANDLE_VALUE);
        return (m_findData.dwFileAttributes & FILE_ATTRIBUTE_TEMPORARY) ? TRUE : FALSE;
    }


}


#endif // _WIN32XX_FILEFIND_H_
