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

    ///////////////////////////////////////////////////////////////////////////////
    // CMetaFile wraps a HMETAFILE. CMetaFile can be used anywhere a HMETAFILE can
    // be used. CMetaFile objects are reference counted, so they can be safely
    // copied. CMetatFile automatically deletes the HMETAFILE when the last copy of
    // the CMetaFile object goes out of scope. The CMetaFileDC::Close function
    // returns a CMetaFile object.
    class CMetaFile
    {
    public:
        CMetaFile();
        CMetaFile(HMETAFILE metaFile);
        CMetaFile(const CMetaFile& rhs);
        virtual ~CMetaFile();
        CMetaFile& operator = (const CMetaFile& rhs);
        void operator = (const HMETAFILE metaFile);
        operator HMETAFILE() { return m_pData->metaFile; }

    private:
        struct CMetaFile_Data   // A structure that contains the data members for CMetaFile
        {
            // Constructor
            CMetaFile_Data() : metaFile(0), count(1L)  {}

            HMETAFILE metaFile;
            long    count;
        };

        void Attach(HMETAFILE metaFile);
        void Release();

        CMetaFile_Data* m_pData;
    };


    /////////////////////////////////////////////////////
    // CEnhMetaFile wraps a HENHMETAFILE. CEnhMetaFile can be used anywhere a
    // HENHMETAFILE can be used. CEnhMetaFile objects are reference counted,
    // so they can be safely copied. CEnhMetaFile automatically deletes the
    // HENHMETAFILE when the last copy of the CEnhMetaFile object goes out of
    // scope. The CMetaFileDC::CloseEnhanced function returns a CEnhMetaFile
    // object.
    class CEnhMetaFile
    {
    public:
        CEnhMetaFile();
        CEnhMetaFile(HENHMETAFILE enhMetaFile);
        CEnhMetaFile(const CEnhMetaFile& rhs);
        ~CEnhMetaFile();
        CEnhMetaFile& operator = (const CEnhMetaFile& rhs);
        void operator = (const HENHMETAFILE enhMetaFile);
        operator HENHMETAFILE() { return m_pData->enhMetaFile; }

    private:
        struct CEnhMetaFile_Data    // A structure that contains the data members for CEnhMetaFile.
        {
            // Constructor
            CEnhMetaFile_Data() : enhMetaFile(0), count(1L) {}

            HENHMETAFILE enhMetaFile;
            long    count;
        };

        void Attach(HENHMETAFILE enhMetaFile);
        void Release();

        CEnhMetaFile_Data* m_pData;
    };

}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


namespace Win32xx
{

    /////////////////////////////////////////////////////
    // Definitions for the the CMetaFile class
    //
    inline CMetaFile::CMetaFile()
    {
        m_pData = new CMetaFile_Data;
    }

    inline CMetaFile::CMetaFile(HMETAFILE metaFile)
    {
        m_pData = new CMetaFile_Data;
        m_pData->metaFile = metaFile;
    }

    inline CMetaFile::CMetaFile(const CMetaFile& rhs)
    {
        m_pData = rhs.m_pData;
        InterlockedIncrement(&m_pData->count);
    }

    inline CMetaFile::~CMetaFile()
    {
        Release();
    }

    inline CMetaFile& CMetaFile::operator = (const CMetaFile& rhs)
    {
        if (this != &rhs)
        {
            InterlockedIncrement(&rhs.m_pData->count);
            Release();
            m_pData = rhs.m_pData;
        }

        return *this;
    }

    inline void CMetaFile::operator = (const HMETAFILE metaFile)
    {
        Attach(metaFile);
    }

    // Attaches an existing HMETAFILE to this CMetaFile.
    // The HMETAFILE can be 0.
    inline void CMetaFile::Attach(HMETAFILE metaFile)
    {
        assert(m_pData);

        if (m_pData && metaFile != m_pData->metaFile)
        {
            // Release any existing enhanced metafile.
            if (m_pData->metaFile != 0)
            {
                Release();
                m_pData = new CMetaFile_Data;
            }

            m_pData->metaFile = metaFile;
        }
    }

    inline void CMetaFile::Release()
    {
        assert(m_pData);

        if (m_pData && InterlockedDecrement(&m_pData->count) == 0)
        {
            if (m_pData->metaFile != 0)
            {
                ::DeleteMetaFile(m_pData->metaFile);
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

    inline CEnhMetaFile::CEnhMetaFile(HENHMETAFILE enhMetaFile)
    {
        m_pData = new CEnhMetaFile_Data;
        m_pData->enhMetaFile = enhMetaFile;
    }

    inline CEnhMetaFile::CEnhMetaFile(const CEnhMetaFile& rhs)
    {
        m_pData = rhs.m_pData;
        InterlockedIncrement(&m_pData->count);
    }

    inline CEnhMetaFile::~CEnhMetaFile()
    {
        Release();
    }

    inline CEnhMetaFile& CEnhMetaFile::operator = (const CEnhMetaFile& rhs)
    {
        if (this != &rhs)
        {
            InterlockedIncrement(&rhs.m_pData->count);
            Release();
            m_pData = rhs.m_pData;
        }

        return *this;
    }

    inline void CEnhMetaFile::operator = (const HENHMETAFILE enhMetaFile)
    {
        Attach(enhMetaFile);
    }

    // Attaches an existing HENHMETAFILE to this CEnhMetaFile.
    // The HENHMETAFILE can be 0.
    inline void CEnhMetaFile::Attach(HENHMETAFILE enhMetaFile)
    {
        assert(m_pData);

        if (m_pData && enhMetaFile != m_pData->enhMetaFile)
        {
            // Release any existing enhanced metafile
            if (m_pData->enhMetaFile != 0)
            {
                Release();
                m_pData = new CEnhMetaFile_Data;
            }

            m_pData->enhMetaFile = enhMetaFile;
        }
    }

    inline void CEnhMetaFile::Release()
    {
        assert(m_pData);

        if (m_pData && InterlockedDecrement(&m_pData->count) == 0)
        {
            if (m_pData->enhMetaFile != 0)
            {
                ::DeleteEnhMetaFile(m_pData->enhMetaFile);
            }

            delete m_pData;
            m_pData = 0;
        }
    }

}  // namespace Win32xx

#endif // _WIN32XX_METAFILE_H_
