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
        CCriticalSection (const CCriticalSection&) = delete;
        CCriticalSection& operator=(const CCriticalSection&) = delete;

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
        CThreadLock(const CThreadLock&) = delete;
        CThreadLock& operator= (const CThreadLock&) = delete;
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
        ::InitializeCriticalSection(&m_cs);
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
