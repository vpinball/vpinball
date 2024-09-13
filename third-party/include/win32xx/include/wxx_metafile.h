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

    struct MetaFileData   // A structure that contains the data members for CMetaFile.
    {
        // Constructor
        MetaFileData() : metaFile(nullptr) {}

        HMETAFILE metaFile;
    };

    struct EnhMetaFileData    // A structure that contains the data members for CEnhMetaFile.
    {
        // Constructor
        EnhMetaFileData() : enhMetaFile(nullptr) {}

        HENHMETAFILE enhMetaFile;
    };

    ///////////////////////////////////////////////////////////////////////////////
    // CMetaFile wraps a HMETAFILE. CMetaFile can be used anywhere a HMETAFILE can
    // be used. CMetaFile objects are reference counted, so they can be safely
    // copied. CMetatFile automatically deletes the HMETAFILE when the last copy of
    // the CMetaFile object goes out of scope. The CMetaFileDC::Close function
    // returns a CMetaFile object.
    class CMetaFile
    {
        friend class CMetaFileDC;

    public:
        CMetaFile();
        CMetaFile(const CMetaFile& rhs);
        virtual ~CMetaFile();
        CMetaFile& operator=(const CMetaFile& rhs);
        operator HMETAFILE() { return m_pData->metaFile; }

    private:
        CMetaFile(HMETAFILE metaFile);
        void Release();

        MetaDataPtr m_pData;
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
        friend class CEnhMetaFileDC;

    public:
        CEnhMetaFile();
        CEnhMetaFile(const CEnhMetaFile& rhs);
        ~CEnhMetaFile();
        CEnhMetaFile& operator=(const CEnhMetaFile& rhs);
        operator HENHMETAFILE() { return m_pData->enhMetaFile; }

    private:
        CEnhMetaFile(HENHMETAFILE enhMetaFile);
        void Release();
        EnhMetaDataPtr m_pData;
    };

}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


namespace Win32xx
{

    /////////////////////////////////////////////////////
    // Definitions for the the CMetaFile class
    //
    inline CMetaFile::CMetaFile() : m_pData(std::make_shared<MetaFileData>())
    {
    }

    // A private constructor used by CMetaFileDC.
    inline CMetaFile::CMetaFile(HMETAFILE metaFile) : m_pData(std::make_shared<MetaFileData>())
    {
        m_pData->metaFile = metaFile;
    }

    inline CMetaFile::CMetaFile(const CMetaFile& rhs)
    {
        m_pData = rhs.m_pData;
    }

    inline CMetaFile::~CMetaFile()
    {
        Release();
    }

    inline CMetaFile& CMetaFile::operator=(const CMetaFile& rhs)
    {
        if (this != &rhs)
        {
            Release();
            m_pData = rhs.m_pData;
        }

        return *this;
    }

    inline void CMetaFile::Release()
    {
        if (CWinApp::SetnGetThis())
            CThreadLock mapLock(GetApp()->m_gdiLock);

        // Delete the metafile when the last copy goes out of scope.
        if (m_pData.use_count() == 1 && m_pData->metaFile != nullptr)
        {
            VERIFY(::DeleteMetaFile(m_pData->metaFile));
        }
    }


    /////////////////////////////////////////////////////
    // Definitions for the the CEnhMetaFile class
    //
    inline CEnhMetaFile::CEnhMetaFile() : m_pData(std::make_shared<EnhMetaFileData>())
    {
    }

    // A private constructor used by CEnhMetaFileDC.
    inline CEnhMetaFile::CEnhMetaFile(HENHMETAFILE enhMetaFile)
        : m_pData(std::make_shared<EnhMetaFileData>())
    {
        m_pData->enhMetaFile = enhMetaFile;
    }

    inline CEnhMetaFile::CEnhMetaFile(const CEnhMetaFile& rhs)
    {
        m_pData = rhs.m_pData;
    }

    inline CEnhMetaFile::~CEnhMetaFile()
    {
        Release();
    }

    inline CEnhMetaFile& CEnhMetaFile::operator=(const CEnhMetaFile& rhs)
    {
        if (this != &rhs)
        {
            Release();
            m_pData = rhs.m_pData;
        }
        return *this;
    }

    inline void CEnhMetaFile::Release()
    {
        if (CWinApp::SetnGetThis())
            CThreadLock mapLock(GetApp()->m_gdiLock);

        // Delete the enhanced metafile when the last copy goes out of scope.
        if (m_pData.use_count() == 1 && m_pData->enhMetaFile != nullptr)
        {
            VERIFY(::DeleteEnhMetaFile(m_pData->enhMetaFile));
        }
    }


}  // namespace Win32xx

#endif // _WIN32XX_METAFILE_H_
