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



#ifndef _WIN32XX_CRITICALSECTION_H_
#define _WIN32XX_CRITICALSECTION_H_

namespace Win32xx
{

    /////////////////////////////////////////
    // This class is used for thread synchronisation. A critical section object
    // provides synchronization similar to that provided by a mutex object,
    // except that a critical section can be used only by the threads of a
    // single process. Critical sections are faster and more efficient than mutexes.
    // The CCriticalSection object should be created in the primary thread. Create
    // them as member variables in your CWinApp derived class.
    class CCriticalSection
    {
    public:
        CCriticalSection();
        ~CCriticalSection();

        void Lock();
        void Release();

    private:
        CCriticalSection ( const CCriticalSection& );
        CCriticalSection& operator = ( const CCriticalSection& );

        CRITICAL_SECTION m_cs;
        long m_count;
    };


    /////////////////////////////////////////////////////////////////
    // CThreadLock provides a convenient RAII-style mechanism for
    // owning a CCriticalSection for the duration of a scoped block.
    // Automatically locks the specified CCriticalSection when
    // constructed, and releases the critical section when destroyed.
    class CThreadLock
    {
    public:
        CThreadLock(CCriticalSection& cs) : m_cs(cs) { m_cs.Lock(); }
        ~CThreadLock() { m_cs.Release(); }

    private:
        CThreadLock(const CThreadLock&);                // Disable copy construction
        CThreadLock& operator= (const CThreadLock&);    // Disable assignment operator
        CCriticalSection& m_cs;
    };

}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

namespace Win32xx
{

    /////////////////////////////////////////////
    // Definitions for the CCriticalSection class
    //
    inline CCriticalSection::CCriticalSection() : m_count(0)
    {
#if defined (_MSC_VER) && (_MSC_VER >= 1400)  // >= VS2005
#pragma warning ( push )
#pragma warning ( disable : 28125 )           // call within __try __catch block.
#endif // (_MSC_VER) && (_MSC_VER >= 1400)

        ::InitializeCriticalSection(&m_cs);

#if defined (_MSC_VER) && (_MSC_VER >= 1400)  // Note: Only Windows Server 2003 and Windows XP
#pragma warning ( pop )                       //       require this warning to be suppressed.
#endif // (_MSC_VER) && (_MSC_VER >= 1400)    //       This exception was removed in Vista and above.
    }

    inline CCriticalSection::~CCriticalSection()
    {
        while (m_count > 0)
        {
            Release();
        }

        ::DeleteCriticalSection(&m_cs);
    }

    // Enter the critical section and increment the lock count.
    inline void CCriticalSection::Lock()
    {
        ::EnterCriticalSection(&m_cs);
        InterlockedIncrement(&m_count);
    }

    // Leave the critical section and decrement the lock count.
    inline void CCriticalSection::Release()
    {
        assert(m_count > 0);
        if (m_count > 0)
        {
            ::LeaveCriticalSection(&m_cs);
            ::InterlockedDecrement(&m_count);
        }
    }

}


#endif // _WIN32XX_CRITICALSECTION_H_