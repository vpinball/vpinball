// Win32++   Version 8.8
// Release Date: 15th October 2020
//
//      David Nash
//      email: dnash@bigpond.net.au
//      url: https://sourceforge.net/projects/win32-framework
//
//
// Copyright (c) 2005-2020  David Nash
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


#ifndef _WIN32XX_APPCORE_H_
#define _WIN32XX_APPCORE_H_


///////////////////////////////////////////////////////
// wxx_appcore.h
// This file contains the definitions of the following set of classes.
//
// 1) CObject: A base class for CWnd and any other class that uses serialization.
//             It provides a virtual Serialize function for use by CArchive.
//
// 2) CWinApp: This class is used start Win32++ and run the message loop. You
//            should inherit from this class to start Win32++ in your own
//            application.
//
// 3) CWinThread: This class is the parent class for CWinApp. It is also the
//            class used to create additional GUI and worker threads.


#include "wxx_appcore0.h"
#include "wxx_textconv.h"
#include "wxx_wincore0.h"
#include "wxx_exception.h"

namespace Win32xx
{
    /////////////////////////////////////////////
    // Definitions for the CCriticalSection class
    //
    inline CCriticalSection::CCriticalSection() : m_count(0)
    {
#if defined (_MSC_VER) && (_MSC_VER >= 1400)
#pragma warning ( push )
#pragma warning ( disable : 28125 )       // call within __try __catch block.
#endif // (_MSC_VER) && (_MSC_VER >= 1400)

        ::InitializeCriticalSection(&m_cs);

#if defined (_MSC_VER) && (_MSC_VER >= 1400)
#pragma warning ( pop )
#endif // (_MSC_VER) && (_MSC_VER >= 1400)
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


    ///////////////////////////////////////
    // Definitions for the CHGlobal class
    //

    // Allocates a new global memory buffer for this object
    inline void CHGlobal::Alloc(size_t size)
    {
        Free();
        m_hGlobal = ::GlobalAlloc(GHND, size);
        if (m_hGlobal == 0)
            throw std::bad_alloc();
    }

    // Manually frees the global memory assigned to this object
    inline void CHGlobal::Free()
    {
        if (m_hGlobal != 0)
            ::GlobalFree(m_hGlobal);

        m_hGlobal = 0;
    }

    // Reassign is used when global memory has been reassigned, as
    // can occur after a call to ::PrintDlg or ::PageSetupDlg.
    // It assigns a new memory handle to be managed by this object
    // and assumes any old memory has already been freed.
    inline void  CHGlobal::Reassign(HGLOBAL hGlobal)
    {
        m_hGlobal = hGlobal;
    }

    ///////////////////////////////////////
    // Definitions for the CObject class
    //
    inline void CObject::Serialize(CArchive& /* ar */ )
    {
    //  Override Serialize in the class inherited from CObject like this.

    //  if (ar.IsStoring())
    //  {
    //      // Store a member variable in the archive
    //      ar << m_someValue;
    //  }
    //  else
    //  {
    //      // Load a member variable from the archive
    //      ar >> m_someValue;
    //  }

    }

    ///////////////////////////////////////
    // Definitions for the CWinThread class
    //

    // CWinThread constructor.
    // Override CWinThread and use this constructor for GUI threads.
    // InitInstance will be called when the thread runs.
    inline CWinThread::CWinThread() : m_pfnThreadProc(0), m_pThreadParams(0), m_thread(0),
                                       m_threadID(0), m_threadIDForWinCE(0), m_accel(0), m_accelWnd(0)
    {
    }

    // CWinThread constructor.
    // Use CWinThread directly and call this constructor for worker threads.
    // Specify a pointer to the function to run when the thread starts.
    // Specifying pParam for a worker thread is optional.
    inline CWinThread::CWinThread(PFNTHREADPROC pfnThreadProc, LPVOID pParam) : m_pfnThreadProc(0),
                        m_pThreadParams(0), m_thread(0), m_threadID(0), m_threadIDForWinCE(0), m_accel(0), m_accelWnd(0)
    {
        m_pfnThreadProc = pfnThreadProc;
        m_pThreadParams = pParam;
    }

    inline CWinThread::~CWinThread()
    {
        if (m_thread)
        {
            // A thread's state is set to signalled when the thread terminates.
            // If your thread is still running at this point, you have a bug.
            if (IsRunning())
            {
                TRACE("*** Warning *** Ending CWinThread before ending its thread\n");
            }

            // Close the thread's handle
            VERIFY(::CloseHandle(m_thread));
        }
    }

    // Creates a new thread
    // Valid argument values:
    // initflag                 Either CREATE_SUSPENDED or 0
    // stack_size               Either the stack size or 0
    // pSecurityAttributes      Either a pointer to SECURITY_ATTRIBUTES or 0
    // Refer to CreateThread in the Windows API documentation for more information.
    inline HANDLE CWinThread::CreateThread(unsigned initflag /* = 0 */, unsigned stack_size/* = 0 */, LPSECURITY_ATTRIBUTES pSecurityAttributes /*= NULL*/)
    {
        if (NULL == m_pfnThreadProc) m_pfnThreadProc = CWinThread::StaticThreadProc;
        if (NULL == m_pThreadParams) m_pThreadParams = this;

        // Reusing the CWinThread
        if (m_thread)
        {
            assert(!IsRunning());
            VERIFY(CloseHandle(m_thread));
        }

#ifdef _WIN32_WCE
        m_thread = reinterpret_cast<HANDLE>(::CreateThread(pSecurityAttributes, stack_size, (LPTHREAD_START_ROUTINE)m_pfnThreadProc, m_pThreadParams, initflag, &m_threadIDForWinCE));
#else
        m_thread = reinterpret_cast<HANDLE>(::_beginthreadex(pSecurityAttributes, stack_size, m_pfnThreadProc, m_pThreadParams, initflag, &m_threadID));
#endif

        if (m_thread == 0)
            throw CWinException(g_msgAppThreadFailed);

        return m_thread;
    }

    // Retrieves a handle to the main window for this thread.
    // Note: CFrame set's itself as the main window of its thread
    inline HWND CWinThread::GetMainWnd() const
    {
        TLSData* pTLSData = GetApp()->GetTlsData();

        // Will assert if the thread doesn't have TLSData assigned.
        // TLSData is assigned when the first window in the thread is created.
        assert (pTLSData);

        return pTLSData ? pTLSData->mainWnd : 0;
    }

    // Retrieves the handle of this thread.
    inline HANDLE CWinThread::GetThread() const
    {
        return m_thread;
    }

    // Retrieves the thread's ID.
    inline int CWinThread::GetThreadID() const
    {
        assert(m_thread);

#ifdef _WIN32_WCE
        return m_threadIDForWinCE;
#endif

        return m_threadID;
    }

    // Retrieves this thread's priority
    // Refer to GetThreadPriority in the Windows API documentation for more information.
    inline int CWinThread::GetThreadPriority() const
    {
        assert(m_thread);
        return ::GetThreadPriority(m_thread);
    }

    // Override this function to perform tasks when the thread starts.
    // return TRUE to run a message loop, otherwise return FALSE.
    // A thread with a window must run a message loop.
    inline BOOL CWinThread::InitInstance()
    {
        return FALSE;
    }

    // This function manages the way window message are dispatched
    // to a window procedure.
    inline int CWinThread::MessageLoop()
    {
        MSG Msg;
        ZeroMemory(&Msg, sizeof(Msg));
        int status = 1;
        LONG lCount = 0;

        while (status != 0)
        {
            // While idle, perform idle processing until OnIdle returns FALSE
            while (!::PeekMessage(&Msg, 0, 0, 0, PM_NOREMOVE) && OnIdle(lCount) != FALSE)
                ++lCount;

            lCount = 0;

            // Now wait until we get a message
            if ((status = ::GetMessage(&Msg, NULL, 0, 0)) == -1)
                return -1;

            if (!PreTranslateMessage(Msg))
            {
                ::TranslateMessage(&Msg);
                ::DispatchMessage(&Msg);
            }

        }

        return LOWORD(Msg.wParam);
    }

    // This function is called by the MessageLoop. It is called when the message queue
    // is empty. Return TRUE to continue idle processing or FALSE to end idle processing
    // until another message is queued. lCount is incremented each time OnIdle is called,
    // and reset to 0 each time a new messages is processed.
    inline BOOL CWinThread::OnIdle(LONG count)
    {
        UNREFERENCED_PARAMETER(count);

        return FALSE;
    }

    // This function is called by the MessageLoop. It processes the
    // keyboard accelerator keys and calls CWnd::PreTranslateMessage for
    // keyboard and mouse events.
    inline BOOL CWinThread::PreTranslateMessage(MSG& msg)
    {
        BOOL isProcessed = FALSE;

        // only pre-translate mouse and keyboard input events
        if ((msg.message >= WM_KEYFIRST && msg.message <= WM_KEYLAST) ||
            (msg.message >= WM_MOUSEFIRST && msg.message <= WM_MOUSELAST))
        {
            // Process keyboard accelerators
            if ( ::TranslateAccelerator(GetAcceleratorsWindow(), GetAcceleratorTable(), &msg))
                isProcessed = TRUE;
            else
            {
                // Search the chain of parents for pretranslated messages.
                for (HWND wnd = msg.hwnd; wnd != NULL; wnd = ::GetParent(wnd))
                {
                    CWnd* pWnd = GetApp()->GetCWndFromMap(wnd);
                    if (pWnd)
                    {
                        isProcessed = pWnd->PreTranslateMessage(msg);
                        if (isProcessed)
                            break;
                    }
                }
            }
        }

        return isProcessed;
    }

    // Posts a message to the thread. The message will reach the MessageLoop, but
    // will not call a CWnd's WndProc.
    // Refer to PostThreadMessage in the Windows API documentation for more information.
    inline BOOL CWinThread::PostThreadMessage(UINT msg, WPARAM wparam, LPARAM lparam) const
    {
        assert(m_thread);
        return ::PostThreadMessage(GetThreadID(), msg, wparam, lparam);
    }

    // Resumes a thread that has been suspended, or created with the CREATE_SUSPENDED flag.
    // Refer to ResumeThread in the Windows API documentation for more information.
    inline DWORD CWinThread::ResumeThread() const
    {
        assert(m_thread);
        return ::ResumeThread(m_thread);
    }

    // accel is the handle of the accelerator table
    // accelWnd is the window handle for translated messages.
    inline void CWinThread::SetAccelerators(HACCEL accel, HWND accelWnd)
    {
        m_accelWnd = accelWnd;
        m_accel = accel;
    }

    // Sets the main window for this thread.
    // Note: CFrame set's itself as the main window of its thread
    inline void CWinThread::SetMainWnd(HWND wnd)
    {
        TLSData* pTLSData = GetApp()->SetTlsData();
        pTLSData->mainWnd = wnd;
    }

    // Sets the priority of this thread. The nPriority parameter can
    // be -7, -6, -5, -4, -3, 3, 4, 5, or 6 or other values permitted
    // by the SetThreadPriority Windows API function.
    // Refer to SetThreadPriority in the Windows API documentation for more information.
    inline BOOL CWinThread::SetThreadPriority(int priority) const
    {
        assert(m_thread);
        return ::SetThreadPriority(m_thread, priority);
    }

    // Suspends this thread. Use ResumeThread to resume the thread.
    // Refer to SuspendThread in the Windows API documentation for more information.
    inline DWORD CWinThread::SuspendThread() const
    {
        assert(m_thread);
        return ::SuspendThread(m_thread);
    }

    // When the thread starts, it runs this function.
    inline UINT WINAPI CWinThread::StaticThreadProc(LPVOID pCThread)
    {
        // Get the pointer for this CWinThread object
        CWinThread* pThread = static_cast<CWinThread*>(pCThread);
        assert(pThread);

        if (pThread->InitInstance())
        {
            GetApp()->SetTlsData();
            return pThread->MessageLoop();
        }

        return 0;
    }


    ////////////////////////////////////
    // Definitions for the CWinApp class
    //

    // To begin Win32++, inherit your application class from this one.
    // You must run only one instance of the class inherited from CWinApp.

    inline CWinApp::CWinApp() : m_callback(NULL)
    {
        if ( 0 != SetnGetThis() )
        {
            // Test if this is the only instance of CWinApp
            throw CNotSupportedException(g_msgAppInstanceFailed);
        }

        m_tlsData = ::TlsAlloc();
        if (m_tlsData == TLS_OUT_OF_INDEXES)
        {
            // We only get here in the unlikely event that all TLS indexes are already allocated by this app
            // At least 64 TLS indexes per process are allowed. Win32++ requires only one TLS index.
            throw CNotSupportedException(g_msgAppTLSFailed);
        }

        SetnGetThis(this);

        // Set the instance handle
#ifdef _WIN32_WCE
        m_instance = (HINSTANCE)GetModuleHandle(0);
#else
        MEMORY_BASIC_INFORMATION mbi;
        ZeroMemory(&mbi, sizeof(mbi));
        static int Address = 0;
        VirtualQuery( &Address, &mbi, sizeof(mbi) );
        assert(mbi.AllocationBase);
        m_instance = (HINSTANCE)mbi.AllocationBase;
#endif

        m_resource = m_instance;
        SetCallback();
    }

    inline CWinApp::~CWinApp()
    {
        // Forcibly destroy any remaining windows now. Windows created from
        //  static CWnds or dangling pointers are destroyed here.
        std::map<HWND, CWnd*, CompareHWND>::const_iterator m;
        for (m = m_mapHWND.begin(); m != m_mapHWND.end(); ++m)
        {
            HWND wnd = (*m).first;
            if (::IsWindow(wnd))
            {
                ::DestroyWindow(wnd);
            }
        }

        // Do remaining tidy up
        m_allTLSData.clear();
        if (m_tlsData != TLS_OUT_OF_INDEXES)
        {
            ::TlsSetValue(m_tlsData, NULL);
            ::TlsFree(m_tlsData);
        }

        SetnGetThis(0, true);
    }

    // Adds a HDC and CDC_Data* pair to the map.
    inline void CWinApp::AddCDCData(HDC dc, CDC_Data* pData)
    {
        CThreadLock mapLock(m_gdiLock);
        m_mapCDCData.insert(std::make_pair(dc, pData));
    }

    // Adds a HGDIOBJ and CGDI_Data* pair to the map.
    inline void CWinApp::AddCGDIData(HGDIOBJ gdi, CGDI_Data* pData)
    {
        CThreadLock mapLock(m_gdiLock);
        m_mapCGDIData.insert(std::make_pair(gdi, pData));
    }

    // Adds a HIMAGELIST and Ciml_Data* pair to the map.
    inline void CWinApp::AddCImlData(HIMAGELIST images, CIml_Data* pData)
    {
        CThreadLock mapLock(m_wndLock);
        m_mapCImlData.insert(std::make_pair(images, pData));
    }

#ifndef _WIN32_WCE

    // Adds a HMENU and CMenu_Data* to the map.
    inline void CWinApp::AddCMenuData(HMENU menu, CMenu_Data* pData)
    {
        CThreadLock mapLock(m_wndLock);
        m_mapCMenuData.insert(std::make_pair(menu, pData));
    }

#endif

    // Retrieves a pointer to CDC_Data from the map
    inline CDC_Data* CWinApp::GetCDCData(HDC dc)
    {
        std::map<HDC, CDC_Data*, CompareHDC>::const_iterator m;

        // Find the CDC data mapped to this HDC
        CDC_Data* pCDCData = 0;
        CThreadLock mapLock(m_gdiLock);
        m = m_mapCDCData.find(dc);

        if (m != m_mapCDCData.end())
            pCDCData = m->second;

        return pCDCData;
    }

    // Retrieves a pointer to CGDI_Data from the map
    inline CGDI_Data* CWinApp::GetCGDIData(HGDIOBJ object)
    {
        std::map<HGDIOBJ, CGDI_Data*, CompareGDI>::const_iterator m;

        // Find the CGDIObject data mapped to this HGDIOBJ
        CGDI_Data* pCGDIData = 0;
        CThreadLock mapLock(m_gdiLock);
        m = m_mapCGDIData.find(object);

        if (m != m_mapCGDIData.end())
            pCGDIData = m->second;

        return pCGDIData;
    }

    // Retrieves a pointer to CIml_Data from the map
    inline CIml_Data* CWinApp::GetCImlData(HIMAGELIST images)
    {
        std::map<HIMAGELIST, CIml_Data*, CompareHIMAGELIST>::const_iterator m;

        // Find the CImageList data mapped to this HIMAGELIST
        CIml_Data* pCImlData = 0;
        CThreadLock mapLock(m_wndLock);
        m = m_mapCImlData.find(images);

        if (m != m_mapCImlData.end())
            pCImlData = m->second;

        return pCImlData;
    }

#ifndef _WIN32_WCE

    // Retrieves a pointer to CMenu_Data from the map
    inline CMenu_Data* CWinApp::GetCMenuData(HMENU menu)
    {
        std::map<HMENU, CMenu_Data*, CompareHMENU>::const_iterator m;

        // Find the CMenu data mapped to this HMENU
        CMenu_Data* pCMenuData = 0;
        CThreadLock mapLock(m_wndLock);
        m = m_mapCMenuData.find(menu);

        if (m != m_mapCMenuData.end())
            pCMenuData = m->second;

        return pCMenuData;
    }

#endif

    // Retrieves the CWnd pointer associated with the specified wnd.
    inline CWnd* CWinApp::GetCWndFromMap(HWND wnd)
    {
        // Allocate an iterator for our HWND map
        std::map<HWND, CWnd*, CompareHWND>::const_iterator m;

        // Find the CWnd pointer mapped to this HWND
        CWnd* pWnd = 0;
        CThreadLock mapLock(m_wndLock);
        m = m_mapHWND.find(wnd);

        if (m != m_mapHWND.end())
            pWnd = m->second;

        return pWnd;
    }

    // Retrieves the pointer to the Thread Local Storage data for the current thread.
    inline TLSData* CWinApp::GetTlsData() const
    {
        return static_cast<TLSData*>(TlsGetValue(m_tlsData));
    }

    // InitInstance contains the initialization code for your application
    // You should override this function with the code to run when the application starts.
    // return TRUE to indicate success. FALSE will end the application,
    inline BOOL CWinApp::InitInstance()
    {
        return TRUE;
    }

    // Loads the cursor resource from the resource script (resource.rc)
    // Refer to LoadCursor in the Windows API documentation for more information.
    inline HCURSOR CWinApp::LoadCursor(LPCTSTR pResourceName) const
    {
        return ::LoadCursor(GetResourceHandle(), pResourceName);
    }

    // Loads the cursor resource from the resource script (resource.rc)
    // Refer to LoadCursor in the Windows API documentation for more information.
    inline HCURSOR CWinApp::LoadCursor(int cursorID) const
    {
        return ::LoadCursor(GetResourceHandle(), MAKEINTRESOURCE (cursorID));
    }

    // Returns the handle of a standard cursor. Standard cursors include:
    // IDC_APPSTARTING, IDC_ARROW, IDC_CROSS, IDC_HAND, IDC_HELP, IDC_IBEAM, IDC_NO, IDC_SIZEALL,
    // IDC_SIZENESW, IDC_SIZENS, IDC_SIZENWSE, IDC_SIZEWE, IDC_UPARROW, IDC_WAIT
    // Refer to LoadCursor in the Windows API documentation for more information.
    inline HCURSOR CWinApp::LoadStandardCursor(LPCTSTR pCursorName) const
    {
        return ::LoadCursor(0, pCursorName);
    }

    // Loads the icon resource whose size conforms to the SM_CXICON and SM_CYICON system metric values.
    // For other icon sizes, use the LoadImage windows API function.
    // Refer to LoadIcon in the Windows API documentation for more information.
    inline HICON CWinApp::LoadIcon(LPCTSTR pResourceName) const
    {
        return ::LoadIcon(GetResourceHandle(), pResourceName);
    }

    // Loads the icon resource whose size conforms to the SM_CXICON and SM_CYICON system metric values.
    // Refer to LoadIcon in the Windows API documentation for more information.
    inline HICON CWinApp::LoadIcon(int iconID) const
    {
        return ::LoadIcon(GetResourceHandle(), MAKEINTRESOURCE (iconID));
    }

    // Returns the handle of a standard Icon. Standard Icons include:
    // IDI_APPLICATION, IDI_ASTERISK, IDI_ERROR, IDI_EXCLAMATION,
    // IDI_HAND, IDI_INFORMATION, IDI_QUESTION, IDI_WARNING
    // Refer to LoadIcon in the Windows API documentation for more information.
    inline HICON CWinApp::LoadStandardIcon(LPCTSTR pIconName) const
    {
        return ::LoadIcon(0, pIconName);
    }

    // Loads an icon, cursor, animated cursor, or bitmap image.
    // uType is the image type. It can be IMAGE_BITMAP, IMAGE_CURSOR or IMAGE_ICON.
    // cx and cy are the desired width and height in pixels.
    // fuLoad can be LR_DEFAULTCOLOR, LR_CREATEDIBSECTION, LR_DEFAULTSIZE, LR_LOADFROMFILE,
    // LR_LOADMAP3DCOLORS, R_LOADTRANSPARENT, LR_MONOCHROME, LR_SHARED, LR_VGACOLOR.
    // Ideally the image should be destroyed unless it is loaded with LR_SHARED.
    // Refer to LoadImage in the Windows API documentation for more information.
    inline HANDLE CWinApp::LoadImage(LPCTSTR pResourceName, UINT type, int cx, int cy, UINT flags) const
    {
        return ::LoadImage(GetResourceHandle(), pResourceName, type, cx, cy, flags);
    }

    // Loads an icon, cursor, animated cursor, or bitmap.
    // uType is the image type. It can be IMAGE_BITMAP, IMAGE_CURSOR or IMAGE_ICON.
    // cx and cy are the desired width and height in pixels.
    // fuLoad can be LR_DEFAULTCOLOR, LR_CREATEDIBSECTION, LR_DEFAULTSIZE, LR_LOADFROMFILE,
    // LR_LOADMAP3DCOLORS, R_LOADTRANSPARENT, LR_MONOCHROME, LR_SHARED, LR_VGACOLOR.
    // Ideally the image should be destroyed unless it is loaded with LR_SHARED.
    // Refer to LoadImage in the Windows API documentation for more information.
    inline HANDLE CWinApp::LoadImage(int imageID, UINT type, int cx, int cy, UINT flags) const
    {
        return ::LoadImage(GetResourceHandle(), MAKEINTRESOURCE (imageID), type, cx, cy, flags);
    }

    // Runs the application and starts the message loop.
    inline int CWinApp::Run()
    {
        // InitInstance runs the App's initialization code
        if (InitInstance())
        {
            // Dispatch the window messages
            return MessageLoop();
        }
        else
        {
            TRACE("InitInstance failed!  Terminating program\n");
            ::PostQuitMessage(-1);
            return -1;
        }
    }

    // Registers a temporary window class so we can get the callback
    // address of CWnd::StaticWindowProc.
    // This technique works for all Window versions, including WinCE.
    inline void CWinApp::SetCallback()
    {
        WNDCLASS defaultWC;
        ZeroMemory(&defaultWC, sizeof(defaultWC));

        LPCTSTR pClassName    = _T("Win32++ Temporary Window Class");
        defaultWC.hInstance     = GetInstanceHandle();
        defaultWC.lpfnWndProc   = CWnd::StaticWindowProc;
        defaultWC.lpszClassName = pClassName;

        VERIFY(::RegisterClass(&defaultWC) != 0);

        // Retrieve the class information
        ZeroMemory(&defaultWC, sizeof(defaultWC));
        VERIFY(::GetClassInfo(GetInstanceHandle(), pClassName, &defaultWC));

        // Save the callback address of CWnd::StaticWindowProc
        assert(defaultWC.lpfnWndProc);  // Assert fails when running UNICODE build on ANSI OS.
        m_callback = defaultWC.lpfnWndProc;
        VERIFY(::UnregisterClass(pClassName, GetInstanceHandle()) != 0);
    }

    // Sets the current cursor and returns the previous one.
    // Note:The cursor will be set to the window's class cursor (if one is set) each time the
    // mouse is moved over the window. You can specify different cursors for different
    // conditions while processing WM_SETCURSOR.
    // Refer to SetCursor in the Windows API documentation for more information.
    inline HCURSOR CWinApp::SetCursor(HCURSOR cursor) const
    {
        return ::SetCursor(cursor);
    }

    // This function stores the 'this' pointer in a static variable.
    // Once stored, it can be used later to return the 'this' pointer.
    // CWinApp's constructor calls this function and sets the static variable.
    // CWinApp's destructor resets pWinApp to 0.
    inline CWinApp* CWinApp::SetnGetThis(CWinApp* pThis /*= 0*/, bool reset /*= false*/)
    {
        static CWinApp* pWinApp = 0;

        if (pWinApp == 0)
            pWinApp = pThis;
        else
            assert(pThis == 0);

        if (reset)
            pWinApp = 0;

        return pWinApp;
    }

    // This function can be used to load a resource dll.
    // A resource dll can be used to define resources in different languages.
    // To use this function, place code like this in InitInstance
    //   HINSTANCE resource = LoadLibrary(_T("MyResourceDLL.dll"));
    //   SetResourceHandle(resource);
    inline void CWinApp::SetResourceHandle(HINSTANCE resource)
    {
        m_resource = resource;
    }

    // Creates the Thread Local Storage data for the current thread if none already exists.
    inline TLSData* CWinApp::SetTlsData()
    {
        TLSData* pTLSData = GetTlsData();
        if (NULL == pTLSData)
        {
            pTLSData = new TLSData;

            CThreadLock TLSLock(m_appLock);
            m_allTLSData.push_back(pTLSData); // store as a Shared_Ptr

            ::TlsSetValue(m_tlsData, pTLSData);
        }

        return pTLSData;
    }


    ////////////////////////////////////////
    // Global Functions
    //

    // Returns a pointer to the CWinApp derived class.
    inline CWinApp* GetApp()
    {
        CWinApp* pApp = CWinApp::SetnGetThis();
        return pApp;
    }


    // The following functions perform string copies. The size of the dst buffer
    // is specified, much like strcpy_s. The dst buffer is always null terminated.
    // Null or zero arguments cause an assert.

    // Copies an ANSI string from src to dst.
    inline void StrCopyA(char* dst, const char* src, size_t dst_size)
    {
        assert(dst != 0);
        assert(src != 0);
        assert(dst_size != 0);

        if (dst && src && dst_size != 0)
        {
            size_t index;

            // Copy each character.
            for (index = 0; index < dst_size - 1; ++index)
            {
                dst[index] = src[index];
                if (src[index] == '\0')
                    break;
            }

            // Add null termination if required.
            if (dst[index] != '\0')
                dst[dst_size - 1] = '\0';
        }
    }

    // Copies a wide string from src to dst.
    inline void StrCopyW(wchar_t* dst, const wchar_t* src, size_t dst_size)
    {
        assert(dst != 0);
        assert(src != 0);
        assert(dst_size != 0);

        if (dst && src && dst_size != 0)
        {
            size_t index;

            // Copy each character.
            for (index = 0; index < dst_size - 1; ++index)
            {
                dst[index] = src[index];
                if (src[index] == '\0')
                    break;
            }

            // Add null termination if required.
            if (dst[index] != '\0')
                dst[dst_size - 1] = '\0';
        }
    }

    // Copies a TCHAR string from src to dst.
    inline void StrCopy(TCHAR* dst, const TCHAR* src, size_t dst_size)
    {
#ifdef UNICODE
        StrCopyW(dst, src, dst_size);
#else
        StrCopyA(dst, src, dst_size);
#endif
    }


} // namespace Win32xx

#endif // _WIN32XX_APPCORE_H_

