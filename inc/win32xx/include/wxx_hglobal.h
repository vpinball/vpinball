// Win32++   Version 9.0
// Release Date: 30th April 2022
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


#ifndef _WIN32XX_HGLOBAL_H_
#define _WIN32XX_HGLOBAL_H_

namespace Win32xx
{
    ////////////////////////////////////////////////////////////////
    // CHGlobal is a class used to wrap a global memory handle.
    // It automatically frees the global memory when the object goes
    // out of scope. This class is used by CDevMode and CDevNames
    // defined in wxx_printdialogs.h
    class CHGlobal
    {
    public:
        CHGlobal() : m_global(0) {}
        CHGlobal(HGLOBAL handle) : m_global(handle) {}
        CHGlobal(size_t size) : m_global(0) { Alloc(size); }
        ~CHGlobal()                     { Free(); }

        void Alloc(size_t size);
        void Free();
        HGLOBAL Get() const             { return m_global; }
        void Reassign(HGLOBAL handle);

        operator HGLOBAL() const        { return m_global; }

    private:
        CHGlobal(const CHGlobal&);              // Disable copy
        CHGlobal& operator = (const CHGlobal&); // Disable assignment

        HGLOBAL m_global;
    };

}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

namespace Win32xx
{

    ///////////////////////////////////////
    // Definitions for the CHGlobal class
    //

    // Allocates a new global memory buffer for this object
    inline void CHGlobal::Alloc(size_t size)
    {
        Free();
        m_global = ::GlobalAlloc(GHND, size);
        if (m_global == 0)
            throw std::bad_alloc();
    }

    // Manually frees the global memory assigned to this object
    inline void CHGlobal::Free()
    {
        if (m_global != 0)
            VERIFY(::GlobalFree(m_global) == 0);  // Fails if the memory was already freed.

        m_global = 0;
    }

    // Reassign is used when global memory has been reassigned, as
    // can occur after a call to ::PrintDlg, ::PrintDlgEx, or ::PageSetupDlg.
    // It assigns a new memory handle to be managed by this object
    // and assumes any old memory has already been freed.
    inline void  CHGlobal::Reassign(HGLOBAL global)
    {
        m_global = global;
    }

}


#endif // _WIN32XX_HGLOBAL_H_
