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


#ifndef WIN32XX_METAFILE_H_
#define WIN32XX_METAFILE_H_

namespace Win32xx
{
    // Managed data structure for CMetaFile
    struct MetaFileData
    {
        MetaFileData() : metaFile(nullptr) {}
        ~MetaFileData() { ::DeleteMetaFile(metaFile); }

        HMETAFILE metaFile;
    };

    // Managed data structure for CEnhMetaFile
    struct EnhMetaFileData
    {
        EnhMetaFileData() : enhMetaFile(nullptr) {}
        ~EnhMetaFileData() { ::DeleteEnhMetaFile(enhMetaFile); }

        HENHMETAFILE enhMetaFile;
    };

    //////////////////////////////////////////////////////////////////////////
    // CMetaFile wraps a HMETAFILE. CMetaFile can be used anywhere a HMETAFILE
    // can be used. CMetaFile objects are reference counted, so they can be
    // safely copied. CMetaFile automatically deletes the HMETAFILE when the
    // last copy of the CMetaFile object goes out of scope. The
    // CMetaFileDC::Close function returns a CMetaFile object.
    class CMetaFile final
    {
        friend class CMetaFileDC;

    public:
        CMetaFile();
        CMetaFile(const CMetaFile& rhs) = default;
        CMetaFile(CMetaFile&& rhs) = default;
        CMetaFile& operator=(const CMetaFile& rhs) = default;
        CMetaFile& operator=(CMetaFile&& rhs) = default;
        ~CMetaFile() = default;

        operator HMETAFILE() const;

    private:
        CMetaFile(HMETAFILE metaFile);

        MetaDataPtr m_pData;
    };

    //////////////////////////////////////////////////////////////////////////
    // CEnhMetaFile wraps a HENHMETAFILE. CEnhMetaFile can be used anywhere a
    // HENHMETAFILE can be used. CEnhMetaFile objects are reference counted,
    // so they can be safely copied. CEnhMetaFile automatically deletes the
    // HENHMETAFILE when the last copy of the CEnhMetaFile object goes out of
    // scope. The CMetaFileDC::CloseEnhanced function returns a CEnhMetaFile
    // object.
    class CEnhMetaFile final
    {
        friend class CEnhMetaFileDC;

    public:
        CEnhMetaFile();

        CEnhMetaFile(const CEnhMetaFile& rhs) = default;
        CEnhMetaFile(CEnhMetaFile&& rhs) = default;
        CEnhMetaFile& operator=(const CEnhMetaFile& rhs) = default;
        CEnhMetaFile& operator=(CEnhMetaFile&& rhs) = default;
        ~CEnhMetaFile() = default;

        operator HENHMETAFILE() const;

    private:
        CEnhMetaFile(HENHMETAFILE enhMetaFile);

        EnhMetaDataPtr m_pData;
    };
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

namespace Win32xx
{

    ///////////////////////////////////////////
    // Definitions for the the CMetaFile class.
    //
    inline CMetaFile::CMetaFile() : m_pData(std::make_shared<MetaFileData>())
    {
    }

    // A private constructor used by CMetaFileDC.
    inline CMetaFile::CMetaFile(HMETAFILE metaFile) : m_pData(
        std::make_shared<MetaFileData>())
    {
        m_pData->metaFile = metaFile;
    }

    inline CMetaFile::operator HMETAFILE() const
    {
        return m_pData ? m_pData->metaFile : nullptr;
    }

    //////////////////////////////////////////////
    // Definitions for the the CEnhMetaFile class.
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

    inline CEnhMetaFile::operator HENHMETAFILE() const
    {
        return m_pData ? m_pData->enhMetaFile : nullptr;
    }

}

#endif // WIN32XX_METAFILE_H_
