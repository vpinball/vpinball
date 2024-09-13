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


#ifndef _WIN32XX_THREAD_H_
#define _WIN32XX_THREAD_H_

namespace Win32xx
{
    using THREADPROC = UINT (WINAPI*)(LPVOID);

    ////////////////////////////////////////////////////
    // CThreadT is the class template used by CWinThread
    // and CWorkThread.
    template <class T>
    class CThreadT : public T
    {
    public:
        CThreadT();
        CThreadT(THREADPROC pfnThreadProc, LPVOID pParam);
        virtual ~CThreadT() override;

        // Operations
        HANDLE  CreateThread(unsigned initflag = 0, unsigned stack_size = 0, LPSECURITY_ATTRIBUTES pSecurityAttributes = nullptr);
        HANDLE  GetThread() const;
        UINT    GetThreadID() const;
        int     GetThreadPriority() const;
        BOOL    IsRunning() const { return (WaitForSingleObject(m_thread, 0) == WAIT_TIMEOUT); }
        BOOL    PostThreadMessage(UINT message, WPARAM wparam, LPARAM lparam) const;
        DWORD   ResumeThread() const;
        BOOL    SetThreadPriority(int priority) const;
        DWORD   SuspendThread() const;
        operator HANDLE () const { return GetThread(); }

    private:
        CThreadT(const CThreadT&) = delete;
        CThreadT& operator=(const CThreadT&) = delete;

        THREADPROC m_pfnThreadProc;     // Thread callback function.
        LPVOID m_pThreadParams;         // Thread parameter.
        HANDLE m_thread;                // Handle of this thread.
        UINT m_threadID;                // ID of this thread.
    };

    using WorkThread = CThreadT<CObject>;
    using WinThread = CThreadT<CMessagePump>;


    //////////////////////////////////////////////////////////////
    // CWorkThread manages a worker thread. Use the constructor to
    // specify the thread's callback procedure and an optional
    // parameter.
    class CWorkThread : public WorkThread
    {
    public:
        CWorkThread(THREADPROC pfnThreadProc, LPVOID pParam)
              : WorkThread(pfnThreadProc, pParam) {}
        virtual ~CWorkThread() override {}

    private:
        CWorkThread(const CWorkThread&) = delete;
        CWorkThread& operator=(const CWorkThread&) = delete;
    };


    /////////////////////////////////////////////////////////////
    // CWinThread manages a thread that is capable of supporting
    // windows. It runs a message loop.
    class CWinThread : public WinThread
    {
    public:
        CWinThread() : WinThread(StaticThreadProc, this) {}
        virtual ~CWinThread() override;

    private:
        CWinThread(const CWinThread&) = delete;
        CWinThread& operator=(const CWinThread&) = delete;

        static  UINT WINAPI StaticThreadProc(LPVOID pCThread);
    };

}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

namespace Win32xx
{

    //////////////////////////////////////////////
    // Definitions for the CThreadT class template
    //

    // CThreadT constructor.
    template <class T>
    inline CThreadT<T>::CThreadT() : m_pfnThreadProc(0), m_pThreadParams(0), m_thread(0),
        m_threadID(0)
    {
    }

    // CThreadT constructor.
    template <class T>
    inline CThreadT<T>::CThreadT(THREADPROC pfnThreadProc, LPVOID pParam) :m_pfnThreadProc(0),
        m_pThreadParams(0), m_thread(0), m_threadID(0)
    {
        m_pfnThreadProc = pfnThreadProc;
        m_pThreadParams = pParam;
    }

    // CThreadT destructor.
    template <class T>
    inline CThreadT<T>::~CThreadT()
    {
        if (m_thread)
        {
            // A thread's state is set to signalled when the thread terminates.
            // If your thread is still running at this point, you have a bug.
            if (IsRunning())
            {
                TRACE("*** Warning *** Ending CWinThread before ending its thread\n");
            }

            // Close the thread's handle.
            ::CloseHandle(m_thread);
        }
    }

    // Creates a new thread
    // Valid argument values:
    // initflag                 Either CREATE_SUSPENDED or 0.
    // stack_size               Either the stack size or 0.
    // pSecurityAttributes      Either a pointer to SECURITY_ATTRIBUTES or 0.
    // Refer to CreateThread in the Windows API documentation for more information.
    template <class T>
    inline HANDLE CThreadT<T>::CreateThread(unsigned initflag /* = 0*/, unsigned stack_size /* = 0*/,
        LPSECURITY_ATTRIBUTES pSecurityAttributes /* = nullptr*/)
    {
        // Reusing the CWinThread.
        if (m_thread)
        {
            assert(!IsRunning());
            ::CloseHandle(m_thread);
        }

        m_thread = reinterpret_cast<HANDLE>(::_beginthreadex(pSecurityAttributes, stack_size,
            m_pfnThreadProc, m_pThreadParams, initflag, &m_threadID));

        if (m_thread == nullptr)
            throw CWinException(GetApp()->MsgAppThread());

        return m_thread;
    }

    // Retrieves the handle of this thread.
    template <class T>
    inline HANDLE CThreadT<T>::GetThread() const
    {
        return m_thread;
    }

    // Retrieves the thread's ID.
    template <class T>
    inline UINT CThreadT<T>::GetThreadID() const
    {
        assert(m_thread);
        return m_threadID;
    }

    // Retrieves this thread's priority.
    // Refer to GetThreadPriority in the Windows API documentation for more information.
    template <class T>
    inline int CThreadT<T>::GetThreadPriority() const
    {
        assert(m_thread);
        return ::GetThreadPriority(m_thread);
    }

    // Posts a message to the thread. The message will reach the MessageLoop, but
    // will not call a CWnd's WndProc.
    // Refer to PostThreadMessage in the Windows API documentation for more information.
    template <class T>
    inline BOOL CThreadT<T>::PostThreadMessage(UINT msg, WPARAM wparam, LPARAM lparam) const
    {
        assert(m_thread);
        return ::PostThreadMessage(GetThreadID(), msg, wparam, lparam);
    }

    // Resumes a thread that has been suspended, or created with the CREATE_SUSPENDED flag.
    // Refer to ResumeThread in the Windows API documentation for more information.
    template <class T>
    inline DWORD CThreadT<T>::ResumeThread() const
    {
        assert(m_thread);
        return ::ResumeThread(m_thread);
    }

    // Sets the priority of this thread. The priority parameter can be:
    // THREAD_PRIORITY_IDLE, THREAD_PRIORITY_LOWEST, THREAD_PRIORITY_BELOW_NORMAL,
    // THREAD_PRIORITY_NORMAL, THREAD_PRIORITY_ABOVE_NORMAL, THREAD_PRIORITY_HIGHEST,
    // THREAD_PRIORITY_TIME_CRITICAL or other values permitted by the
    // SetThreadPriority Windows API function.
    // Refer to SetThreadPriority in the Windows API documentation for more information.
    template <class T>
    inline BOOL CThreadT<T>::SetThreadPriority(int priority) const
    {
        assert(m_thread);
        return ::SetThreadPriority(m_thread, priority);
    }

    // Suspends this thread. Use ResumeThread to resume the thread.
    // Refer to SuspendThread in the Windows API documentation for more information.
    template <class T>
    inline DWORD CThreadT<T>::SuspendThread() const
    {
        assert(m_thread);
        return ::SuspendThread(m_thread);
    }


    ////////////////////////////////////////
    // Definitions for the CWinThread class.
    //

    inline CWinThread::~CWinThread()
    {
        if (GetThread() != nullptr)
        {
            // Post a WM_QUIT to safely end the thread.
            PostThreadMessage(WM_QUIT, 0, 0);
        }

        // Wait up to 1 second for the thread to end.
        ::WaitForSingleObject(*this, 1000);
    }

    // When the GUI thread starts, it runs this function.
    inline UINT WINAPI CWinThread::StaticThreadProc(LPVOID pCThread)
    {
        // Get the pointer for this CWinThread object.
        CWinThread* pThread = static_cast<CWinThread*>(pCThread);
        assert(pThread != nullptr);

        if (pThread != nullptr)
        {
            // Set the thread's TLS Data.
            GetApp()->SetTlsData();

            // Run the thread's message loop if InitInstance returns TRUE.
            if (pThread->InitInstance())
            {
                return static_cast<UINT>(pThread->MessageLoop());
            }
        }

        return 0;
    }

}

#endif // _WIN32XX_THREAD_H_
