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


////////////////////////////////////////////////////////
// A metafile is a collection of structures that store a picture in a 
// device-independent format. Device independence is the one feature that sets
// metafiles apart from bitmaps. Unlike a bitmap, a metafile guarantees device
// independence. There is a drawback to metafiles however, they are generally
// drawn more slowly than bitmaps.
//
// In 1993, the 32-bit version of Win32/GDI introduced the Enhanced Metafile
// (EMF), a newer version with additional commands. EMF is also used as a
// graphics language for printer drivers. Microsoft recommends that
// "Windows-format" (WMF) functions only "rarely" be used and "enhanced-format"
// (EMF) functions be used instead.
//
////////////////////////////////////////////////////////


#ifndef _WIN32XX_METAFILE_H_
#define _WIN32XX_METAFILE_H_

namespace Win32xx
{

#ifndef _WIN32_WCE


	/////////////////////////////////////////////////////
	// Declaration of the the CMetaFile class
	//
	// CMetaFile wraps a HMETAFILE. CMetaFile can be used anywhere a HMETAFILE can
	// be used. CMetaFile objects are reference counted, so they can be safely
	// copied. CMetatFile automatically deletes the HMETAFILE when the last copy of
	// the CMetaFile object goes out of scope. The CMetaFileDC::Close function
	// returns a CMetaFile object.
	//
	class CMetaFile
	{
	public:
		CMetaFile();
		CMetaFile(HMETAFILE hMetaFile);
		CMetaFile(const CMetaFile& rhs);
		~CMetaFile();
		CMetaFile& operator = (const CMetaFile& rhs);
		void operator = (const HMETAFILE hMetaFile);
		operator HMETAFILE() { return m_pData->hMetaFile; }

	private:
		struct CMetaFile_Data	// A structure that contains the data members for CMetaFile
		{
			// Constructor
			CMetaFile_Data() : hMetaFile(0), Count(1L)  {}

			HMETAFILE hMetaFile;
			long	Count;
		};
		
		void Attach(HMETAFILE hMetaFile);
		void Release();
		
		CMetaFile_Data* m_pData;
	};


	/////////////////////////////////////////////////////
	// Declaration of the the CEnhMetaFile class
	//
	// CEnhMetaFile wraps a HENHMETAFILE. CEnhMetaFile can be used anywhere a
	// HENHMETAFILE can be used. CEnhMetaFile objects are reference counted, 
	// so they can be safely copied. CEnhMetaFile automatically deletes the
	// HENHMETAFILE when the last copy of the CEnhMetaFile object goes out of
	// scope. The CMetaFileDC::CloseEnhanced function returns a CEnhMetaFile
	// object.
	//
	class CEnhMetaFile
	{
	public:
		CEnhMetaFile();
		CEnhMetaFile(HENHMETAFILE hEnhMetaFile);
		CEnhMetaFile(const CEnhMetaFile& rhs);
		~CEnhMetaFile();
		CEnhMetaFile& operator = (const CEnhMetaFile& rhs);
		void operator = (const HENHMETAFILE hEnhMetaFile);
		operator HENHMETAFILE() { return m_pData->hEnhMetaFile; }

	private:
		struct CEnhMetaFile_Data	// A structure that contains the data members for CEnhMetaFile
		{
			// Constructor
			CEnhMetaFile_Data() : hEnhMetaFile(0), Count(1L) {}

			HENHMETAFILE hEnhMetaFile;
			long	Count;
		};
	
		void Attach(HENHMETAFILE hEnhMetaFile);
		void Release();

		CEnhMetaFile_Data* m_pData;
	};

#endif // _WIN32_WCE

}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


namespace Win32xx
{
#ifndef _WIN32_WCE
	/////////////////////////////////////////////////////
	// Definitions for the the CMetaFile class
	//
	inline CMetaFile::CMetaFile()
	{
		m_pData = new CMetaFile_Data;
	}

	inline CMetaFile::CMetaFile(HMETAFILE hMetaFile)
	{
		m_pData = new CMetaFile_Data;
		m_pData->hMetaFile = hMetaFile;
	}

	inline CMetaFile::CMetaFile(const CMetaFile& rhs)
	{
		m_pData = rhs.m_pData;
		InterlockedIncrement(&m_pData->Count);
	}

	inline CMetaFile::~CMetaFile()
	{
		Release();
	}

	inline CMetaFile& CMetaFile::operator = (const CMetaFile& rhs)
	{
		if (this != &rhs)
		{
			InterlockedIncrement(&rhs.m_pData->Count);
			Release();
			m_pData = rhs.m_pData;
		}

		return *this;
	}

	inline void CMetaFile::operator = (const HMETAFILE hMetaFile)
	{
		Attach(hMetaFile);
	}

	inline void CMetaFile::Attach(HMETAFILE hMetaFile)
		// Attaches an existing HMETAFILE to this CMetaFile
		// The HMETAFILE can be NULL
	{
		assert(m_pData);

		if (hMetaFile != m_pData->hMetaFile)
		{
			// Release any existing enhanced metafile
			if (m_pData->hMetaFile != 0)
			{
				Release();
				m_pData = new CMetaFile_Data;
			}

			m_pData->hMetaFile = hMetaFile;
		}
	}

	inline void CMetaFile::Release()
	{
		assert(m_pData);

		if (InterlockedDecrement(&m_pData->Count) == 0)
		{
			if (m_pData->hMetaFile != NULL)
			{
				::DeleteMetaFile(m_pData->hMetaFile);
			}

			delete m_pData;
			m_pData = 0;
		}
	}


	/////////////////////////////////////////////////////
	// Definitions for the the CEnhMetaFile class
	//
	inline CEnhMetaFile::CEnhMetaFile()
	{
		m_pData = new CEnhMetaFile_Data;
	}

	inline CEnhMetaFile::CEnhMetaFile(HENHMETAFILE hEnhMetaFile)
	{
		m_pData = new CEnhMetaFile_Data;
		m_pData->hEnhMetaFile = hEnhMetaFile;
	}

	inline CEnhMetaFile::CEnhMetaFile(const CEnhMetaFile& rhs)
	{
		m_pData = rhs.m_pData;
		InterlockedIncrement(&m_pData->Count);
	}

	inline CEnhMetaFile::~CEnhMetaFile()
	{
		Release();
	}

	inline CEnhMetaFile& CEnhMetaFile::operator = (const CEnhMetaFile& rhs)
	{
		if (this != &rhs)
		{
			InterlockedIncrement(&rhs.m_pData->Count);
			Release();
			m_pData = rhs.m_pData;
		}

		return *this;
	}

	inline void CEnhMetaFile::operator = (const HENHMETAFILE hEnhMetaFile)
	{
		Attach(hEnhMetaFile);
	}

	inline void CEnhMetaFile::Attach(HENHMETAFILE hEnhMetaFile)
		// Attaches an existing HENHMETAFILE to this CEnhMetaFile
		// The HENHMETAFILE can be NULL
	{
		assert(m_pData);

		if (hEnhMetaFile != m_pData->hEnhMetaFile)
		{
			// Release any existing enhanced metafile
			if (m_pData->hEnhMetaFile != 0)
			{
				Release();
				m_pData = new CEnhMetaFile_Data;
			}

			m_pData->hEnhMetaFile = hEnhMetaFile;
		}
	}

	inline void CEnhMetaFile::Release()
	{
		assert(m_pData);

		if (InterlockedDecrement(&m_pData->Count) == 0)
		{
			if (m_pData->hEnhMetaFile != NULL)
			{
				::DeleteEnhMetaFile(m_pData->hEnhMetaFile);
			}

			delete m_pData;
			m_pData = 0;
		}
	}

#endif // _WIN32_WCE

}  // namespace Win32xx

#endif // _WIN32XX_METAFILE_H_
