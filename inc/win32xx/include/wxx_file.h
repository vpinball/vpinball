// Win32++   Version 8.4
// Release Date: 10th March 2017
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

	class CFile
	{
	public:

		enum OpenFlags
		{
			modeCreate =        CREATE_ALWAYS,	// Creates a new file. Truncates existing file to length 0.
			modeNoTruncate =    OPEN_ALWAYS, // Creates a new file or opens an existing one.
			shareExclusive =    0x0010,	// Denies read and write access to all others.
			shareDenyWrite =    0x0020,	// Denies write access to all others.
			shareDenyRead =     0x0030,	// Denies read access to all others.
			shareDenyNone =     0x0040,	// No sharing restrictions.
			modeRead =          0x0100,	// Requests read access only.
			modeWrite =         0x0200,	// Requests write access only.
			modeReadWrite =     0x0300	// Requests read and write access.
		};

		CFile();
		CFile(HANDLE hFile);
		CFile(LPCTSTR pszFileName, UINT nOpenFlags);
		virtual ~CFile();
		operator HANDLE() const;

		virtual BOOL Close();
		virtual BOOL Flush();
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
		static BOOL Remove(LPCTSTR pszFileName);
		static BOOL Rename(LPCTSTR pszOldName, LPCTSTR pszNewName);
		virtual ULONGLONG Seek(LONGLONG lOff, UINT nFrom);
		virtual void SeekToBegin();
		virtual ULONGLONG SeekToEnd();
		virtual BOOL SetLength(ULONGLONG NewLen);
		virtual void Write(const void* pBuf, UINT nCount);

#ifndef _WIN32_WCE
		virtual BOOL LockRange(ULONGLONG Pos, ULONGLONG Count);
		virtual void SetFilePath(LPCTSTR pszNewName);
		virtual BOOL UnlockRange(ULONGLONG Pos, ULONGLONG Count);
#endif

	private:
		CFile(const CFile&);				// Disable copy construction
		CFile& operator = (const CFile&);	// Disable assignment operator
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

	inline CFile::CFile(LPCTSTR pszFileName, UINT nOpenFlags) : m_hFile(0)
	// Possible nOpenFlag values: CREATE_NEW, CREATE_ALWAYS, OPEN_EXISTING, OPEN_ALWAYS, TRUNCATE_EXISTING
	// Default value: OPEN_EXISTING | modeReadWrite
	//
	// The following modes are also supported:
	//	modeCreate		Creates a new file. Truncates an existing file to length 0.
	//	modeNoTruncate	Creates a a new file, or opens an existing one.
	//	modeRead		Requests read access only.
	//	modeWrite		Requests write access only.
	//	modeReadWrite	Requests read and write access.
	//	shareExclusive	Denies read and write access to all others.
	//	shareDenyWrite	Denies write access to all others.
	//	shareDenyRead	Denies read access to all others.
	//	shareDenyNone	No sharing restrictions.
	{
		assert(pszFileName);
		Open(pszFileName, nOpenFlags);	// throws CFileException on failure
	}

	inline CFile::~CFile()
	{
		Close();
	}

	inline CFile::operator HANDLE() const
	{
		return m_hFile;
	}

	inline BOOL CFile::Close()
	// Closes the file associated with this object. Closed file can no longer be read or written to.
	{
		BOOL IsClosed = TRUE;
		if (m_hFile != 0)
			IsClosed = CloseHandle(m_hFile);

		m_hFile = 0;
		return IsClosed;
	}

	inline BOOL CFile::Flush()
	// Causes any remaining data in the file buffer to be written to the file.
	{
		assert(m_hFile);
		return FlushFileBuffers(m_hFile);
	}

	inline HANDLE CFile::GetHandle() const
	// Returns the file handle associated with this object.
	{
		return m_hFile;
	}

	inline CString CFile::GetFileDirectory() const
	// Returns the directory of the file associated with this object.
	{
		CString Directory;

		int sep = m_FilePath.ReverseFind(_T("\\"));
		if (sep > 0)
			Directory = m_FilePath.Left(sep);

		return Directory;
	}

	inline const CString& CFile::GetFileName() const
	// Returns the filename of the file associated with this object, not including the directory.
	{
		return m_FileName;
	}

	inline CString CFile::GetFileNameExt() const
	// Returns the extension part of the filename of the file associated with this object.
	{
		CString Extension;

		int dot = m_FileName.ReverseFind(_T("."));
		if (dot > 1)
			Extension = m_FileName.Mid(dot+1, lstrlen(m_FileName));

		return Extension;
	}

	inline CString CFile::GetFileNameWOExt() const
	// Returns the filename of the file associated with this object, not including the directory, without its extension.
	{
		CString FileNameWOExt = m_FileName;

		int dot = m_FileName.ReverseFind(_T("."));
		if (dot > 0)
			FileNameWOExt = m_FileName.Left(dot);

		return FileNameWOExt;
	}

	inline const CString& CFile::GetFilePath() const
	// Returns the full filename including the directory of the file associated with this object.
	{
		return m_FilePath;
	}

	inline CString CFile::GetFileTitle() const
	// Returns the string that the system would use to display the file name to
	// the user. The string might or might not contain the filename's extension
	// depending on user settings.
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

	inline ULONGLONG CFile::GetLength() const
	// Returns the length of the file in bytes.
	{
		assert(m_hFile);

		LONG HighPosCur = 0;
		LONG HighPosEnd = 0;

		DWORD LowPosCur = SetFilePointer(m_hFile, 0, &HighPosCur, FILE_CURRENT);
		DWORD LowPosEnd = SetFilePointer(m_hFile, 0, &HighPosEnd, FILE_END);
		SetFilePointer(m_hFile, LowPosCur, &HighPosCur, FILE_BEGIN);

		ULONGLONG Result = ((ULONGLONG)HighPosEnd << 32) + LowPosEnd;
		return Result;
	}

	inline ULONGLONG CFile::GetPosition() const
	// Returns the current value of the file pointer, which can be used in subsequent calls to Seek.
	{
		assert(m_hFile);
		LONG High = 0;
		DWORD LowPos = SetFilePointer(m_hFile, 0, &High, FILE_CURRENT);

		ULONGLONG Result = ((ULONGLONG)High << 32) + LowPos;
		return Result;
	}

#ifndef _WIN32_WCE
	inline BOOL CFile::LockRange(ULONGLONG Pos, ULONGLONG Count)
	// Locks a range of bytes in and open file.
	{
		assert(m_hFile);

		DWORD dwPosHigh = (DWORD)(Pos >> 32);
		DWORD dwPosLow = (DWORD)(Pos & 0xFFFFFFFF);
		DWORD dwCountHigh = (DWORD)(Count >> 32);
		DWORD dwCountLow = (DWORD)(Count & 0xFFFFFFFF);

		return ::LockFile(m_hFile, dwPosLow, dwPosHigh, dwCountLow, dwCountHigh);
	}
#endif

	inline void CFile::Open(LPCTSTR pszFileName, UINT nOpenFlags)
	// Prepares a file to be written to or read from.
	// Possible nOpenFlag values: CREATE_NEW, CREATE_ALWAYS, OPEN_EXISTING, OPEN_ALWAYS, TRUNCATE_EXISTING
	// Default value: OPEN_EXISTING | modeReadWrite
	//
	// The following modes are also supported:
	//	modeCreate		Creates a new file. Truncates an existing file to length 0.
	//	modeNoTruncate	Creates a a new file, or opens an existing one.
	//	modeRead		Requests read access only.
	//	modeWrite		Requests write access only.
	//	modeReadWrite	Requests read and write access.
	//	shareExclusive	Denies read and write access to all others.
	//	shareDenyWrite	Denies write access to all others.
	//	shareDenyRead	Denies read access to all others.
	//	shareDenyNone	No sharing restrictions.
	{
		if (m_hFile != 0) Close();

		DWORD dwAccess = 0;
		switch (nOpenFlags & 0xF00)
		{
		case modeRead:
			dwAccess = GENERIC_READ;	break;
		case modeWrite:
			dwAccess = GENERIC_WRITE;	break;
		case modeReadWrite:
			dwAccess = GENERIC_READ | GENERIC_WRITE; break;
		default:
			dwAccess = GENERIC_READ | GENERIC_WRITE; break;
		}

		DWORD dwShare = 0;
		switch (nOpenFlags & 0xF0)
		{
		case shareExclusive:	dwShare = 0; break;
		case shareDenyWrite:	dwShare = FILE_SHARE_READ;	break;
		case shareDenyRead:		dwShare = FILE_SHARE_WRITE; break;
		case shareDenyNone:		dwShare = FILE_SHARE_READ | FILE_SHARE_WRITE; break;
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

	inline UINT CFile::Read(void* pBuf, UINT nCount)
	// Reads from the file, storing the contents in the specified buffer.
	{
		assert(m_hFile);

		if (nCount == 0) return 0;

		assert(pBuf);
		DWORD dwRead = 0;

		if (!::ReadFile(m_hFile, pBuf, nCount, &dwRead, NULL))
			throw CFileException(GetFilePath(), _T("Failed to read from file"));

		return dwRead;
	}

	inline BOOL CFile::Rename(LPCTSTR pszOldName, LPCTSTR pszNewName)
	// Renames the specified file.
	{
		return ::MoveFile(pszOldName, pszNewName);
	}

	inline BOOL CFile::Remove(LPCTSTR pszFileName)
	// Deletes the specified file.
	{
		return ::DeleteFile(pszFileName);
	}

	inline ULONGLONG CFile::Seek(LONGLONG lOff, UINT nFrom)
	// Positions the current file pointer.
	// Permitted values for nFrom are: FILE_BEGIN, FILE_CURRENT, or FILE_END.
	{
		assert(m_hFile);
		assert(nFrom == FILE_BEGIN || nFrom == FILE_CURRENT || nFrom == FILE_END);

		LONG High = LONG(lOff >> 32);
		LONG Low = (LONG)(lOff & 0xFFFFFFFF);

		DWORD LowPos = SetFilePointer(m_hFile, Low, &High, nFrom);

		ULONGLONG Result = ((ULONGLONG)High << 32) + LowPos;
		return Result;
	}

	inline void CFile::SeekToBegin()
	// Sets the current file pointer to the beginning of the file.
	{
		assert(m_hFile);
		Seek(0, FILE_BEGIN);
	}

	inline ULONGLONG CFile::SeekToEnd()
	// Sets the current file pointer to the end of the file.
	{
		assert(m_hFile);
		return Seek(0, FILE_END);
	}

#ifndef _WIN32_WCE
	inline void CFile::SetFilePath(LPCTSTR pszFileName)
	// Assigns the specified full file path to this object.
	// Call this function if the file path is not supplied when the CFile is constructed.
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

	inline BOOL CFile::SetLength(ULONGLONG NewLen)
	// Changes the length of the file to the specified value.
	{
		assert(m_hFile);

		Seek(NewLen, FILE_BEGIN);
		return ::SetEndOfFile(m_hFile);
	}

#ifndef _WIN32_WCE
	inline BOOL CFile::UnlockRange(ULONGLONG Pos, ULONGLONG Count)
	// Unlocks a range of bytes in an open file.
	{
		assert(m_hFile);

		DWORD dwPosHigh = (DWORD)(Pos >> 32);
		DWORD dwPosLow = (DWORD)(Pos & 0xFFFFFFFF);
		DWORD dwCountHigh = (DWORD)(Count >> 32);
		DWORD dwCountLow = (DWORD)(Count & 0xFFFFFFFF);

		return ::UnlockFile(m_hFile, dwPosLow, dwPosHigh, dwCountLow, dwCountHigh);
	}
#endif

	inline void CFile::Write(const void* pBuf, UINT nCount)
	// Writes the specified buffer to the file.
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


}	// namespace Win32xx

#endif
