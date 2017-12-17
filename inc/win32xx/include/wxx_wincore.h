// Win32++   Version 8.5
// Release Date: 1st December 2017
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


///////////////////////////////////////////////////////
// wxx_wincore.h
//  This file contains the definition of the CWnd class.
//  Including this file adds all the code necessary to start Win32++
//  and create simple windows.
//
//  Refer to the following example code.
//


// Example code
/*

///////////////////////////////////
// main.cpp

// To configure your project:
// 1) Add Win32++'s include directory to the project's additional include directories for C/C++
// 2) Add Win32++'s include directory to the project's additional include directories for Resources

#include "wxx_wincore.h"


// A class that inherits from CWnd. It is used to create the window.
class CView : public CWnd
{
public:
    CView() {}
    virtual ~CView() {}
    virtual void OnDestroy() { PostQuitMessage(0); }    // Ends the program
};



int APIENTRY WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    // Start Win32++
    CWinApp theApp;

    // Create our view window
    CView MyWin;
    MyWin.Create();

    // Run the application's message loop
    return theApp.Run();
}

*/


#ifndef _WIN32XX_WINCORE_H_
#define _WIN32XX_WINCORE_H_



//////////////////////////////////////
//  Include the Win32++ header files

#include "wxx_appcore.h"
#include "wxx_wincore0.h"
#include "wxx_gdi.h"
#ifndef _WIN32_WCE
  #include "wxx_menu.h"
#endif
#include "wxx_ddx.h"



namespace Win32xx
{

    ////////////////////////////////////////
    // Definitions for the CWnd class
    //

    inline CWnd::CWnd() : m_hWnd(NULL), m_PrevWindowProc(NULL)
    {
        // Note: m_hWnd is set in CWnd::CreateEx(...)
    }


    inline CWnd::CWnd(HWND hWnd) : m_PrevWindowProc(NULL)
    {
        // A private constructor, used internally.

        m_hWnd = hWnd;
    }


    inline CWnd::~CWnd()
    {
        // Destroys the window for this object and cleans up resources.
        Destroy();
    }


    // Store the window handle and CWnd pointer in the HWND map
    inline void CWnd::AddToMap()
    {
        // The framework must be started
        assert(&GetApp());

        // This HWND is should not be in the map yet
        assert (NULL == GetApp().GetCWndFromMap(*this));

        // Remove any old map entry for this CWnd (required when the CWnd is reused)
        RemoveFromMap();

        // Add the (HWND, CWnd*) pair to the map
        CThreadLock mapLock(GetApp().m_csMapLock);
        GetApp().m_mapHWND.insert(std::make_pair(GetHwnd(), this));
    }


    // Subclass an existing window and attach it to a CWnd
    inline BOOL CWnd::Attach(HWND hWnd)
    {
        assert( &GetApp() );
        assert( ::IsWindow(hWnd) );
        assert( !IsWindow() );

        // Ensure this thread has the TLS index set
        // Note: Perform the attach from the same thread as the window's message loop
        GetApp().SetTlsData();

        Subclass(hWnd);     // Set the window's callback to CWnd::StaticWindowProc

        OnAttach();

        // Post a message to trigger a call of OnInitialUpdate
        PostMessage(UWM_WINDOWCREATED);

        return TRUE;
    }


    // Converts a dialog item to a CWnd object
    inline BOOL CWnd::AttachDlgItem(UINT nID, HWND hwndParent)
    {
        assert(::IsWindow(hwndParent));

        HWND hWnd = ::GetDlgItem(hwndParent, nID);
        return Attach(hWnd);
    }


    // Centers this window over its parent
    inline void CWnd::CenterWindow() const
    {

    // required for multi-monitor support with Dev-C++ and VC6
#ifndef _WIN32_WCE
  #ifndef MONITOR_DEFAULTTONEAREST
    #define MONITOR_DEFAULTTONEAREST    0x00000002

        DECLARE_HANDLE(HMONITOR);

        typedef struct tagMONITORINFO
        {
            DWORD   cbSize;
            RECT    rcMonitor;
            RECT    rcWork;
            DWORD   dwFlags;
        } MONITORINFO, *LPMONITORINFO;

  #endif    // MONITOR_DEFAULTTONEAREST
#endif  // _WIN32_WCE

        assert(IsWindow());

        CRect rc = GetWindowRect();
        CRect rcParent;
        CRect rcDesktop;

        // Get screen dimensions excluding task bar
        VERIFY(::SystemParametersInfo(SPI_GETWORKAREA, 0, &rcDesktop, 0) != 0);

        // Get the parent window dimensions (parent could be the desktop)
        if (GetParent().GetHwnd() != 0) rcParent = GetParent().GetWindowRect();
        else rcParent = rcDesktop;

 #ifndef _WIN32_WCE
        // Import the GetMonitorInfo and MonitorFromWindow functions
        typedef BOOL(WINAPI* LPGMI)(HMONITOR hMonitor, LPMONITORINFO lpmi);
        typedef HMONITOR(WINAPI* LPMFW)(HWND hwnd, DWORD dwFlags);
        LPMFW pfnMonitorFromWindow = 0;
        HMODULE hUser32 = LoadLibrary(_T("USER32.DLL"));
        LPGMI pfnGetMonitorInfo = 0;
        if (hUser32)
        {

            pfnMonitorFromWindow = (LPMFW)::GetProcAddress(hUser32, "MonitorFromWindow");
  #ifdef UNICODE
            pfnGetMonitorInfo = (LPGMI)::GetProcAddress(hUser32, "GetMonitorInfoW");
  #else
            pfnGetMonitorInfo = (LPGMI)::GetProcAddress(hUser32, "GetMonitorInfoA");
  #endif

            // Take multi-monitor systems into account
            if (pfnGetMonitorInfo && pfnMonitorFromWindow)
            {
                HMONITOR hActiveMonitor = pfnMonitorFromWindow(*this, MONITOR_DEFAULTTONEAREST);
                MONITORINFO mi;
                ZeroMemory(&mi, sizeof(mi));
                mi.cbSize = sizeof(mi);

                if (pfnGetMonitorInfo(hActiveMonitor, &mi))
                {
                    rcDesktop = mi.rcWork;
                    if (GetParent().GetHwnd() == NULL) rcParent = mi.rcWork;
                }
            }
            ::FreeLibrary(hUser32);

        }
 #endif

        // Calculate point to center the dialog over the portion of parent window on this monitor
        rcParent.IntersectRect(rcParent, rcDesktop);
        int x = rcParent.left + (rcParent.Width() - rc.Width())/2;
        int y = rcParent.top + (rcParent.Height() - rc.Height())/2;

        // Keep the dialog wholly on the monitor display
        x = (x < rcDesktop.left)? rcDesktop.left : x;
        x = (x > rcDesktop.right - rc.Width())? rcDesktop.right - rc.Width() : x;
        y = (y < rcDesktop.top) ? rcDesktop.top: y;
        y = (y > rcDesktop.bottom - rc.Height())? rcDesktop.bottom - rc.Height() : y;

        SetWindowPos(0, x, y, 0, 0, SWP_NOSIZE);
    }


    // Returns the CWnd to its default state
    inline void CWnd::Cleanup()
    {
        if ( &GetApp() )
            RemoveFromMap();

        m_hWnd = 0;
        m_PrevWindowProc = 0;
    }


    // Creates the window. This is the default method of window creation.
    inline HWND CWnd::Create(HWND hWndParent /* = 0 */)
    {
        // Test if Win32++ has been started
        assert( &GetApp() );

        WNDCLASS wc;
        ZeroMemory(&wc, sizeof(wc));

        CREATESTRUCT cs;
        ZeroMemory(&cs, sizeof(cs));

        // Set the WNDCLASS parameters
        PreRegisterClass(wc);
        if (wc.lpszClassName)
        {
            RegisterClass(wc);
            cs.lpszClass = wc.lpszClassName;
        }
        else
            cs.lpszClass = _T("Win32++ Window");

        // Set a reasonable default window style
        DWORD dwOverlappedStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
        cs.style = WS_VISIBLE | ((hWndParent)? WS_CHILD : dwOverlappedStyle );

        // Set a reasonable default window position
        if (NULL == hWndParent)
        {
            cs.x  = CW_USEDEFAULT;
            cs.cx = CW_USEDEFAULT;
            cs.y  = CW_USEDEFAULT;
            cs.cy = CW_USEDEFAULT;
        }

        // Allow the CREATESTRUCT parameters to be modified
        PreCreate(cs);

        DWORD dwStyle = cs.style & ~WS_VISIBLE;
        HWND hWnd;

        // Create the window
#ifndef _WIN32_WCE
        hWnd = CreateEx(cs.dwExStyle, cs.lpszClass, cs.lpszName, dwStyle,
                cs.x, cs.y, cs.cx, cs.cy, hWndParent,
                cs.hMenu, cs.lpCreateParams);

        if (cs.style & WS_VISIBLE)
        {
            if      (cs.style & WS_MAXIMIZE) ShowWindow(SW_MAXIMIZE);
            else if (cs.style & WS_MINIMIZE) ShowWindow(SW_MINIMIZE);
            else    ShowWindow();
        }

#else
        hWnd = CreateEx(cs.dwExStyle, cs.lpszClass, cs.lpszName, cs.style,
                cs.x, cs.y, cs.cx, cs.cy, hWndParent,
                0, cs.lpCreateParams);
#endif

        return hWnd;
    }


    // Creates the window by specifying all the window creation parameters
    inline HWND CWnd::CreateEx(DWORD dwExStyle, LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rc, HWND hWndParent, UINT nID, LPVOID lpParam /*= NULL*/)
    {
        int x = rc.left;
        int y = rc.top;
        int cx = rc.right - rc.left;
        int cy = rc.bottom - rc.top;

        INT_PTR idMenu = nID;
        HMENU hMenu = hWndParent ? reinterpret_cast<HMENU>(idMenu) : ::LoadMenu(GetApp().GetResourceHandle(), MAKEINTRESOURCE(nID));

        return CreateEx(dwExStyle, lpszClassName, lpszWindowName, dwStyle, x, y, cx, cy, hWndParent, hMenu, lpParam);
    }


    // Creates the window by specifying all the window creation parameters
    inline HWND CWnd::CreateEx(DWORD dwExStyle, LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, int x, int y, int nWidth, int nHeight, HWND hWndParent, HMENU nIDorHMenu, LPVOID lpParam /*= NULL*/)
    {
        assert( &GetApp() );        // Test if Win32++ has been started
        assert( !IsWindow() );      // Only one window per CWnd instance allowed

        // Ensure a window class is registered
        CString ClassName;
        if (lpszClassName == 0 || lpszClassName[0] == _T('\0'))
            ClassName = _T("Win32++ Window");
        else
            ClassName = lpszClassName;

        WNDCLASS wc;
        ZeroMemory(&wc, sizeof(wc));
        wc.lpszClassName = ClassName;
        wc.hbrBackground = reinterpret_cast<HBRUSH>(::GetStockObject(WHITE_BRUSH));
        wc.hCursor       = ::LoadCursor(NULL, IDC_ARROW);

        // Register the window class (if not already registered)
        if (RegisterClass(wc) == 0)
        {
            TRACE("*** RegisterClass failed ***\n");
            assert( 0 );
        }

        // Ensure this thread has the TLS index set
        TLSData* pTLSData = GetApp().SetTlsData();

        // Store the CWnd pointer in thread local storage
        pTLSData->pWnd = this;
        m_hWnd = 0;

        // Create window
        HWND hWnd = ::CreateWindowEx(dwExStyle, ClassName, lpszWindowName, dwStyle, x, y, nWidth, nHeight,
                                hWndParent, nIDorHMenu, GetApp().GetInstanceHandle(), lpParam);

        // Tidy up
        pTLSData->pWnd = NULL;

        if (hWnd == 0)
        {
            // Throw an exception when window creation fails
            throw CWinException(_T("CreateWindowEx failed"));
        }

        // Automatically subclass predefined window class types
        if (lpszClassName)
        {
            ::GetClassInfo(GetApp().GetInstanceHandle(), lpszClassName, &wc);
            if (wc.lpfnWndProc != GetApp().m_Callback)
            {
                Subclass(hWnd);

                // Override this to perform tasks after the window is attached.
                OnAttach();
            }
        }

        // Clear the CWnd pointer from TLS
        pTLSData->pWnd = NULL;

        // Post a message to trigger a call of OnInitialUpdate
        PostMessage(UWM_WINDOWCREATED);

        return hWnd;
    }


    // Destroys the window and returns the CWnd back to its default state, ready for reuse.
    inline void CWnd::Destroy()
    {
        if (&GetApp())          // Is the CWinApp object still valid?
        {
            if (GetCWndPtr(*this) == this)
            {
                if (IsWindow())
                    ::DestroyWindow(*this);
            }
        }

        // Return the CWnd to its default state
        Cleanup();
    }


    // Reverse an Attach
    inline HWND CWnd::Detach()
    {
        assert(m_PrevWindowProc);   // Only previously attached CWnds can be detached

        if (IsWindow())
            SetWindowLongPtr(GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(m_PrevWindowProc));

        HWND hWnd = GetHwnd();
        Cleanup();

        return hWnd;
    }

#ifndef _WIN32_WCE

    //  This function performs dialog data exchange and dialog data
    //  validation using DDX and DDV functions. Typically this is done for
    //  controls in a dialogs, but controls in any window support DDX and DDV.
    //  Never call this function directly. It is called by the UpdateData()
    //  member function. Call UpdateData() to write data into, or retrieve
    //  validated data from the controls.  Override this method in the dialog
    //  or other window that utilize the DDX/DDV functions.
    inline void CWnd::DoDataExchange(CDataExchange& DX)
    {
          // Any dialog or window using DDX/DDV for its controls should
          // override of this member, and put calls to the DDX and DDV functions
          // there.

        UNREFERENCED_PARAMETER(DX);
    }
#endif

    // Pass messages on to the appropriate default window procedure
    // CMDIChild and CMDIFrame override this function
    inline LRESULT CWnd::FinalWindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        if (m_PrevWindowProc)
            return ::CallWindowProc(m_PrevWindowProc, *this, uMsg, wParam, lParam);
        else
            return ::DefWindowProc(*this, uMsg, wParam, lParam);
    }


    // Retrieves the pointer to the CWnd associated with the specified HWND.
    inline CWnd* CWnd::GetCWndPtr(HWND hWnd)
    {
        assert( &GetApp() );
        return hWnd? GetApp().GetCWndFromMap(hWnd) : 0;
    }


    // The GetAncestor function retrieves the ancestor (root parent)
    // of the window. Supports Win95.
    inline CWnd CWnd::GetAncestor(UINT gaFlags /*= GA_ROOTOWNER*/) const
    {
        assert(IsWindow());
        HWND hWnd = GetHwnd();

        // Load the User32 DLL
        typedef HWND WINAPI GETANCESTOR(HWND, UINT);
        GETANCESTOR* pfnGetAncestor = NULL;
        HMODULE hModule = ::LoadLibrary(_T("USER32.DLL"));

        if (hModule != 0)
        {
            // Declare a pointer to the GetAncestor function
#ifndef _WIN32_WCE
            pfnGetAncestor = reinterpret_cast<GETANCESTOR*>(::GetProcAddress(hModule, "GetAncestor"));
#else
            pfnGetAncestor = reinterpret_cast<GETANCESTOR*>(::GetProcAddress(hModule, L"GetAncestor"));
#endif

            if (pfnGetAncestor)
                hWnd = (*pfnGetAncestor)(*this, gaFlags);

            ::FreeLibrary(hModule);
        }

        if (!pfnGetAncestor)
        {
            // Provide our own GetAncestor if necessary
            HWND hWndParent = ::GetParent(hWnd);
            while (::IsChild(hWndParent, hWnd))
            {
                hWnd = hWndParent;
                hWndParent = ::GetParent(hWnd);
            }
        }

        return CWnd(hWnd);
    }


    // Retrieves the name of the class to which the specified window belongs.
    inline CString CWnd::GetClassName() const
    {
        assert(IsWindow());

        CString str;
        VERIFY(::GetClassName(*this, str.GetBuffer(MAX_STRING_SIZE), MAX_STRING_SIZE) != 0);
        str.ReleaseBuffer();
        return str;
    }


    // Retrieves the title or text associated with a control in a dialog box.
    inline CString CWnd::GetDlgItemText(int nIDDlgItem) const
    {
        assert(IsWindow());

        int nLength = ::GetWindowTextLength(::GetDlgItem(*this, nIDDlgItem));
        CString str;
        ::GetDlgItemText(*this, nIDDlgItem, str.GetBuffer(nLength), nLength+1);
        str.ReleaseBuffer();
        return str;
    }


    // Retrieves the text of the window's title bar.
    inline CString CWnd::GetWindowText() const
    {
        assert(IsWindow());

        CString str;
        str.GetWindowText(*this);
        return str;
    }


    // Called in response to WM_CLOSE. Override to suppress destroying the window
    // WM_CLOSE is sent by SendMessage(WM_SYSCOMMAND, SC_CLOSE, 0) or by clicking X
    //  in the top right corner. Usually, only top level windows receive WM_CLOSE.
    inline void CWnd::OnClose()
    {
        Destroy();
    }


    // Called when the user interacts with the menu or toolar.
    inline BOOL CWnd::OnCommand(WPARAM wParam, LPARAM lParam)
    {
        UNREFERENCED_PARAMETER(wParam);
        UNREFERENCED_PARAMETER(lParam);

        // Override this to handle WM_COMMAND messages, for example

        //  switch (LOWORD(wParam))
        //  {
        //  case IDM_FILE_NEW:
        //      OnFileNew();
        //      TRUE;   // return TRUE for handled commands
        //  }

        // return FALSE for unhandled commands
        return FALSE;
    }


    // This function is called when a window is attached to the CWnd.
    // Override it to automatically perform tasks when the window is attached.
    // Note:  Window controls are attached.
    inline void CWnd::OnAttach()
    {
    }


    // Called during window creation. Override this functions to perform tasks
    // such as creating child windows.
    inline int CWnd::OnCreate(CREATESTRUCT& cs)
    {
        // This function is called when a WM_CREATE message is received
        // Override it to automatically perform tasks during window creation.
        // Return 0 to continue creating the window.

        // Note: Window controls don't call OnCreate. They are sublcassed (attached)
        //  after their window is created.

        UNREFERENCED_PARAMETER (cs);
        return 0;
    }


    // This function is called when a window is destroyed.
    // Override it to do additional tasks, such as ending the application
    //  with PostQuitMessage.
    inline void CWnd::OnDestroy()
    {
    }


    // Called when part of the client area of the window needs to be drawn.
    // Override this function in your derived class to perform drawing tasks.
    inline void CWnd::OnDraw(CDC& dc)
    {
        UNREFERENCED_PARAMETER(dc);
    }


    // Called when the background of the window's client area needs to be erased.
    // Override this function in your derived class to perform drawing tasks.
    // Return Value: Return FALSE to also permit default erasure of the background
    //               Return TRUE to prevent default erasure of the background
    inline BOOL CWnd::OnEraseBkgnd(CDC& dc)
    {
        UNREFERENCED_PARAMETER(dc);

        return FALSE;
    }


    // This function is called automatically once the window is created
    // Override it in your derived class to automatically perform tasks
    // after window creation.
    inline void CWnd::OnInitialUpdate()
    {
    }


    // Called when menu items are about to be displayed. Override this function to
    // enable/disable the menu item, or add/remove the check box or radio button
    // to menu items.
    inline void CWnd::OnMenuUpdate(UINT nID)
    {
        UNREFERENCED_PARAMETER(nID);

        // Override this function to modify the behaviour of menu items,
        // such as adding or removing checkmarks
    }


    // A function used internally to call OnMessageReflect. Don't call or override this function.
    inline LRESULT CWnd::MessageReflect(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        HWND hWnd = 0;
        switch (uMsg)
        {
        case WM_COMMAND:
        case WM_CTLCOLORBTN:
        case WM_CTLCOLOREDIT:
        case WM_CTLCOLORDLG:
        case WM_CTLCOLORLISTBOX:
        case WM_CTLCOLORSCROLLBAR:
        case WM_CTLCOLORSTATIC:
        case WM_CHARTOITEM:
        case WM_VKEYTOITEM:
        case WM_HSCROLL:
        case WM_VSCROLL:
            hWnd = reinterpret_cast<HWND>(lParam);
            break;

        case WM_DRAWITEM:
        case WM_MEASUREITEM:
        case WM_DELETEITEM:
        case WM_COMPAREITEM:
            hWnd = GetDlgItem(static_cast<int>(wParam));
            break;

        case WM_PARENTNOTIFY:
            switch(LOWORD(wParam))
            {
            case WM_CREATE:
            case WM_DESTROY:
                hWnd = reinterpret_cast<HWND>(lParam);
                break;
            }
        }

        CWnd* Wnd = GetApp().GetCWndFromMap(hWnd);

        if (Wnd != NULL)
            return Wnd->OnMessageReflect(uMsg, wParam, lParam);

        return 0L;
    }


    // This function processes those special messages sent by some older controls,
    // and reflects them back to the originating CWnd object.
    // Override this function in your derived class to handle these special messages:
    // WM_COMMAND, WM_CTLCOLORBTN, WM_CTLCOLOREDIT, WM_CTLCOLORDLG, WM_CTLCOLORLISTBOX,
    // WM_CTLCOLORSCROLLBAR, WM_CTLCOLORSTATIC, WM_CHARTOITEM,  WM_VKEYTOITEM,
    // WM_HSCROLL, WM_VSCROLL, WM_DRAWITEM, WM_MEASUREITEM, WM_DELETEITEM,
    // WM_COMPAREITEM, WM_PARENTNOTIFY.
    inline LRESULT CWnd::OnMessageReflect(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        UNREFERENCED_PARAMETER(uMsg);
        UNREFERENCED_PARAMETER(wParam);
        UNREFERENCED_PARAMETER(lParam);
        // This function processes those special messages (see above) sent
        // by some older controls, and reflects them back to the originating CWnd object.
        // Override this function in your derived class to handle these special messages.

        // Your overriding function should look like this ...

        // switch (uMsg)
        // {
        //      Handle your reflected messages here
        // }

        // return 0L for unhandled messages
        return 0L;
    }


    // Processes notification (WM_NOTIFY) messages from a child window.
    inline LRESULT CWnd::OnNotify(WPARAM wParam, LPARAM lParam)
    {
        UNREFERENCED_PARAMETER(wParam);
        UNREFERENCED_PARAMETER(lParam);

        // You can use either OnNotifyReflect or OnNotify to handle notifications
        // Override OnNotifyReflect to handle notifications in the CWnd class that
        //   generated the notification.   OR
        // Override OnNotify to handle notifications in the PARENT of the CWnd class
        //   that generated the notification.

        // Your overriding function should look like this ...

        // switch (((LPNMHDR)lParam)->code)
        // {
        //      Handle your notifications from the CHILD window here
        //      Return the value recommended by the Windows API documentation.
        //      For many notifications, the return value doesn't matter, but for some it does.
        // }

        // return 0L for unhandled notifications
        // The framework will call SetWindowLongPtr(DWLP_MSGRESULT, lr) for dialogs
        return 0L;
    }


    // Processes the notification (WM_NOTIFY) messages in the child window that originated them.
    inline LRESULT CWnd::OnNotifyReflect(WPARAM wParam, LPARAM lParam)
    {
        UNREFERENCED_PARAMETER(wParam);
        UNREFERENCED_PARAMETER(lParam);

        // Override OnNotifyReflect to handle notifications in the CWnd class that
        //   generated the notification.

        // Your overriding function should look like this ...

        // switch (((LPNMHDR)lParam)->code)
        // {
        //      Handle your notifications from this window here
        //      Return the value recommended by the Windows API documentation.
        // }

        // return 0L for unhandled notifications
        // The framework will call SetWindowLongPtr(DWLP_MSGRESULT, lr) for dialogs
        return 0L;
    }


    // Called when the window paints its client area.
    inline LRESULT CWnd::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        // Window controls and other subclassed windows are expected to do their own
        // drawing, so we don't call OnDraw for those.

        // Note: CustomDraw or OwnerDraw are normally used to modify the drawing of
        //       controls, but overriding OnPaint is also an option.

        if (!m_PrevWindowProc)
        {
            if (::GetUpdateRect(*this, NULL, FALSE))
            {
                CPaintDC dc(*this);
                OnDraw(dc);
            }
            else
            // RedrawWindow can require repainting without an update rect
            {
                CClientDC dc(*this);
                OnDraw(dc);
            }

            // No more drawing required
            return 0L;
        }

        // Allow window controls to do their default drawing
        return FinalWindowProc(uMsg, wParam, lParam);
    }


    // Called by CWnd::Create to set some window creation parameters.
    inline void CWnd::PreCreate(CREATESTRUCT& cs)
    {
        UNREFERENCED_PARAMETER(cs);

        // Override this function to set the CREATESTRUCT values prior to window creation.
        // Here we set the initial values for the following:
        //  window styles (WS_VISABLE, WS_CHILD, WS_WS_MAXIMIZEBOX etc.)
        //  window extended styles
        //  window position
        //  window menu
        //  window class name
        //  window name (caption)
    }


    // Called by CWnd::Create to set some window class parameters.
    // Used to set the window type (ClassName) and for setting the background brush and cursor.
    inline void CWnd::PreRegisterClass(WNDCLASS& wc)
    {
        UNREFERENCED_PARAMETER(wc);

        // Override this function to set the WNDCLASS values prior to window creation.
        // for example, for a ToolBar, we use this:
        // wc.lpszClassName = TOOLBARCLASSNAME;

        // ADDITIONAL NOTES:
        // 1) The lpszClassName must be set for this function to take effect.
        // 2) No other defaults are set, so the following settings might prove useful
        //     wc.hCursor = ::LoadCursor(NULL, IDC_ARROW);
        //     wc.hbrBackground = reinterpret_cast<HBRUSH>(::GetStockObject(WHITE_BRUSH));
        //     wc.hIcon = ::LoadIcon(NULL, IDI_APPLICATION);
        // 3) The styles that can be set here are WNDCLASS styles. These are a different
        //     set of styles to those set by CREATESTRUCT (used in PreCreate).
        // 4) RegisterClassEx is not used because its not supported on WinCE.
        //     To set a small icon for the window, use SetIconSmall.
    }


    // Override this function if your class requires input messages to be
    // translated before normal processing. Function which translate messages
    // include TranslateAccelerator, TranslateMDISysAccel and IsDialogMessage.
    // Return TRUE if the message is translated.
    inline BOOL CWnd::PreTranslateMessage(MSG& Msg)
    {
        UNREFERENCED_PARAMETER(Msg);

        return FALSE;
    }


    // A private function used by the CreateEx function to register a window
    // class prior to window creation.
    inline BOOL CWnd::RegisterClass(WNDCLASS& wc)
    {
        assert( &GetApp() );
        assert( ('\0' != wc.lpszClassName[0] && ( lstrlen(wc.lpszClassName) <=  MAX_STRING_SIZE) ) );

        // Check to see if this classname is already registered
        WNDCLASS wcTest;
        ZeroMemory(&wcTest, sizeof(wcTest));
        BOOL Done = FALSE;

        if (::GetClassInfo(GetApp().GetInstanceHandle(), wc.lpszClassName, &wcTest))
        {
            wc = wcTest;
            Done = TRUE;
        }

        if (!Done)
        {
            // Set defaults
            wc.hInstance    = GetApp().GetInstanceHandle();
            wc.lpfnWndProc  = CWnd::StaticWindowProc;

            // Register the WNDCLASS structure
            VERIFY ( ::RegisterClass(&wc) != 0 );

            Done = TRUE;
        }

        return Done;
    }


    // Removes this CWnd's pointer from the application's map.
    inline BOOL CWnd::RemoveFromMap()
    {
        BOOL Success = FALSE;

        if ( &GetApp() )
        {
            // Allocate an iterator for our HWND map
            std::map<HWND, CWnd*, CompareHWND>::iterator m;

            CWinApp& App = GetApp();

            // Erase the CWnd pointer entry from the map
            CThreadLock mapLock(App.m_csMapLock);
            for (m = App.m_mapHWND.begin(); m != App.m_mapHWND.end(); ++m)
            {
                if (this == m->second)
                {
                    App.m_mapHWND.erase(m);
                    Success = TRUE;
                    break;
                }
            }

        }

        return Success;
    }


    // Sets the large icon associated with the window.
    inline HICON CWnd::SetIconLarge(int nIcon)
    {
        assert( &GetApp() );
        assert(IsWindow());

        int cxIcon = ::GetSystemMetrics(SM_CXICON);
        int cyIcon = ::GetSystemMetrics(SM_CYICON);

#ifndef _WIN32_WCE
        HICON hIconLarge = reinterpret_cast<HICON>(GetApp().LoadImage(nIcon, IMAGE_ICON, cxIcon, cyIcon, LR_SHARED));
#else
        HICON hIconLarge = reinterpret_cast<HICON>(GetApp().LoadImage(nIcon, IMAGE_ICON, cxIcon, cyIcon, 0));
#endif

        if (hIconLarge != 0)
            SendMessage (WM_SETICON, WPARAM (ICON_BIG), LPARAM (hIconLarge));
        else
            TRACE("**WARNING** SetIconLarge Failed\n");

        return hIconLarge;
    }


    // Sets the small icon associated with the window.
    inline HICON CWnd::SetIconSmall(int nIcon)
    {
        assert( &GetApp() );
        assert(IsWindow());

        int cxSmallIcon = ::GetSystemMetrics(SM_CXSMICON);
        int cySmallIcon = ::GetSystemMetrics(SM_CYSMICON);

#ifndef _WIN32_WCE
        HICON hIconSmall = reinterpret_cast<HICON>(GetApp().LoadImage(nIcon, IMAGE_ICON, cxSmallIcon, cySmallIcon, LR_SHARED));
#else
        HICON hIconSmall = reinterpret_cast<HICON>(GetApp().LoadImage(nIcon, IMAGE_ICON, cxSmallIcon, cySmallIcon, 0));
#endif

        if (hIconSmall != 0)
            SendMessage (WM_SETICON, WPARAM (ICON_SMALL), LPARAM (hIconSmall));
        else
            TRACE("**WARNING** SetIconSmall Failed\n");

        return hIconSmall;
    }


    // All CWnd windows direct their messages here. This function redirects the message
    // to the CWnd's WndProc function.
    inline LRESULT CALLBACK CWnd::StaticWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        assert( &GetApp() );

        CWnd* w = GetApp().GetCWndFromMap(hWnd);
        if (w == 0)
        {
            // The CWnd pointer wasn't found in the map, so add it now

            // Retrieve the pointer to the TLS Data
            TLSData* pTLSData = GetApp().GetTlsData();
            assert(pTLSData);

            // Retrieve pointer to CWnd object from Thread Local Storage TLS
            w = pTLSData->pWnd;
            assert(w);              // pTLSData->pCWnd is assigned in CreateEx
            pTLSData->pWnd = NULL;

            // Store the CWnd pointer in the HWND map
            w->m_hWnd = hWnd;
            w->AddToMap();
        }

        return w->WndProc(uMsg, wParam, lParam);

    } // LRESULT CALLBACK StaticWindowProc(...)


    // A private function used by CreateEx, Attach and AttachDlgItem.
    inline void CWnd::Subclass(HWND hWnd)
    {
        assert(::IsWindow(hWnd));

        m_hWnd = hWnd;
        AddToMap();         // Store the CWnd pointer in the HWND map
        m_PrevWindowProc = reinterpret_cast<WNDPROC>(::SetWindowLongPtr(hWnd, GWLP_WNDPROC, 
                                         reinterpret_cast<LONG_PTR>(CWnd::StaticWindowProc)));
    }

#ifndef _WIN32_WCE

    //  Dialog Data Exchange support. Call this function to retrieve values from
    //  (RetrieveAndValidate is TRUE) or assign values to (RetrieveAndValidate
    //  is FALSE) a set of controls appearing in DDX/DDV statements in an
    //  override of the DoDataExchange() member method.
    //
    //  Return TRUE if the operation is successful, or FALSE otherwise. If
    //  called when bRetrieveValidate is TRUE, success means the data has
    //  been validated.
    inline BOOL CWnd::UpdateData(CDataExchange& DX, BOOL RetrieveAndValidate)
    {
        // must not update data before the window is created
        assert(IsWindow());

        // A critical section ensures threads update the data seperately
        CCriticalSection ccs;
        CThreadLock TLSLock(ccs);

        DX.Init(*this, RetrieveAndValidate);

        BOOL ok = FALSE;  // Remains FALSE if DoDataExchange throws a CUserException
        try
        {
            DoDataExchange(DX);
            if (DX.GetLastControl() != NULL && DX.GetLastEditControl() != NULL)
            {
                // select all characters in the edit control
                ::SetFocus(DX.GetLastEditControl());
                ::SendMessage(DX.GetLastEditControl(), EM_SETSEL, 0, -1);
            }
            ok = TRUE; // DoDataExchage completed succesfully
        }
        catch(const CUserException& e)
        {
            // Validation has failed. Call the Fail to display the error.
            DX.Fail( e.GetText() );
        }

        return ok;
    }
#endif // _WIN32_WCE


    // Processes this window's message. Override this function in your class
    // derived from CWnd to handle window messages.
    inline LRESULT CWnd::WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        //  A typical function might look like this:

        //  switch (uMsg)
        //  {
        //  case MESSAGE1:  return OnMessage1();
        //  case MESSAGE2:  return OnMessage2();
        //  }

        // The message functions should return a value recommended by the Windows API documentation.
        // Alternatively, return FinalWindowProc to continue with default processing.

        // Always pass unhandled messages on to WndProcDefault
        return WndProcDefault(uMsg, wParam, lParam);
    }


    // Provides default processing for this window's messages.
    // All WndProc functions should pass unhandled window messages to this function.
    inline LRESULT CWnd::WndProcDefault(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        LRESULT lr = 0L;
        if (UWM_WINDOWCREATED == uMsg)
        {
            OnInitialUpdate();
            return 0L;
        }

        switch (uMsg)
        {
        case WM_CLOSE:
            {
                OnClose();
                return 0L;
            }
        case WM_COMMAND:
            {
                // Reflect this message if it's from a control
                CWnd* pWnd = GetCWndPtr(reinterpret_cast<HWND>(lParam));
                if (pWnd != NULL)
                    lr = pWnd->OnCommand(wParam, lParam);

                // Handle user commands
                if (0L == lr)
                    lr =  OnCommand(wParam, lParam);

                if (0L != lr) return 0L;
            }
            break;  // Note: Some MDI commands require default processing
        case WM_CREATE:
            {
                LPCREATESTRUCT pcs = (LPCREATESTRUCT) lParam;
                if (pcs == NULL)
                    throw CWinException(_T("WM_CREATE failed"));

                return OnCreate(*pcs);
            }
        case WM_DESTROY:
            OnDestroy();
            break;  // Note: Some controls require default processing.
        case WM_NOTIFY:
            {
                // Do notification reflection if message came from a child window.
                // Restricting OnNotifyReflect to child windows avoids double handling.
                HWND hwndFrom = ((LPNMHDR)lParam)->hwndFrom;
                CWnd* pWndFrom = GetApp().GetCWndFromMap(hwndFrom);

                if (pWndFrom != NULL)
                    if (::GetParent(hwndFrom) == m_hWnd)
                        lr = pWndFrom->OnNotifyReflect(wParam, lParam);

                // Handle user notifications
                if (lr == 0L) lr = OnNotify(wParam, lParam);
                if (lr != 0L) return lr;
                break;
            }

        case WM_PAINT:
            {
                // OnPaint calls OnDraw when appropriate
                OnPaint(uMsg, wParam, lParam);
            }

            return 0L;

        case WM_ERASEBKGND:
            {
                CDC dc(reinterpret_cast<HDC>(wParam));
                BOOL PreventErasure;

                PreventErasure = OnEraseBkgnd(dc);
                if (PreventErasure) return TRUE;
            }
            break;

        // A set of messages to be reflected back to the control that generated them
        case WM_CTLCOLORBTN:
        case WM_CTLCOLOREDIT:
        case WM_CTLCOLORDLG:
        case WM_CTLCOLORLISTBOX:
        case WM_CTLCOLORSCROLLBAR:
        case WM_CTLCOLORSTATIC:
        case WM_DRAWITEM:
        case WM_MEASUREITEM:
        case WM_DELETEITEM:
        case WM_COMPAREITEM:
        case WM_CHARTOITEM:
        case WM_VKEYTOITEM:
        case WM_HSCROLL:
        case WM_VSCROLL:
        case WM_PARENTNOTIFY:
            {
                lr = MessageReflect(uMsg, wParam, lParam);
                if (lr != 0L) return lr;    // Message processed so return
            }
            break;              // Do default processing when message not already processed

        case UWM_UPDATECOMMAND:
            OnMenuUpdate(static_cast<UINT>(wParam)); // Perform menu updates
            break;

        case UWM_GETCWND:
            {
                assert(this == GetCWndPtr(m_hWnd));
                return reinterpret_cast<LRESULT>(this);
            }

        } // switch (uMsg)

        // Now hand all messages to the default procedure
        return FinalWindowProc(uMsg, wParam, lParam);

    } // LRESULT CWnd::WindowProc(...)


    //
    // Wrappers for Win32 API functions
    //

    // The BeginPaint function prepares the specified window for painting and fills a PAINTSTRUCT
    // structure with information about the painting. Consider using CPaintDC instead.
    inline HDC CWnd::BeginPaint(PAINTSTRUCT& ps) const
    {
        assert(IsWindow());
        return ::BeginPaint(*this, &ps);
    }


    // The BringWindowToTop function brings the specified window to the top
    // of the Z order. If the window is a top-level window, it is activated.
    inline BOOL CWnd::BringWindowToTop() const
    {
        assert(IsWindow());
        return ::BringWindowToTop(*this);
    }


    inline LRESULT CWnd::CallWindowProc(WNDPROC lpPrevWndFunc, UINT Msg, WPARAM wParam, LPARAM lParam) const
    {
        assert(IsWindow());
        return ::CallWindowProc(lpPrevWndFunc, *this, Msg, wParam, lParam);
    }


    // The CheckDlgButton function changes the check state of a button control.
    inline BOOL CWnd::CheckDlgButton(int nIDButton, UINT uCheck) const
    {
        assert(IsWindow());
        return ::CheckDlgButton(*this, nIDButton, uCheck);
    }


    // The CheckRadioButton function adds a check mark to (checks) a specified radio button in a group
    // and removes a check mark from (clears) all other radio buttons in the group.
    inline BOOL CWnd::CheckRadioButton(int nIDFirstButton, int nIDLastButton, int nIDCheckButton) const
    {
        assert(IsWindow());
        return ::CheckRadioButton(*this, nIDFirstButton, nIDLastButton, nIDCheckButton);
    }


    // Determines which, if any, of the child windows belonging to a parent window contains
    // the specified point. The search is restricted to immediate child windows.
    // Grandchildren, and deeper descendant windows are not searched.
    inline CWnd CWnd::ChildWindowFromPoint(POINT pt) const
    {
        assert(IsWindow());
        return CWnd(::ChildWindowFromPoint(*this, pt));
    }


    // The ClientToScreen function converts the client-area coordinates of a specified point to screen coordinates.
    inline BOOL CWnd::ClientToScreen(POINT& pt) const
    {
        assert(IsWindow());
        return ::ClientToScreen(*this, &pt);
    }


    // The ClientToScreen function converts the client-area coordinates of a specified RECT to screen coordinates.
    inline BOOL CWnd::ClientToScreen(RECT& rc) const
    {
        assert(IsWindow());
        return static_cast<BOOL>(::MapWindowPoints(*this, NULL, (LPPOINT)&rc, 2));
    }


    // The DeferWindowPos function updates the specified multiple window position structure for the window.
    // The hWndInsertAfter can one of:  HWND_BOTTOM, HWND_NOTOPMOST, HWND_TOP, or HWND_TOPMOST
    inline HDWP CWnd::DeferWindowPos(HDWP hWinPosInfo, HWND hWndInsertAfter, int x, int y, int cx, int cy, UINT uFlags) const
    {
        assert(IsWindow());
        return ::DeferWindowPos(hWinPosInfo, *this, hWndInsertAfter, x, y, cx, cy, uFlags);
    }


    // The DeferWindowPos function updates the specified multiple window position structure for the window.
    // The hWndInsertAfter can one of:  HWND_BOTTOM, HWND_NOTOPMOST, HWND_TOP, or HWND_TOPMOST
    inline HDWP CWnd::DeferWindowPos(HDWP hWinPosInfo, HWND hWndInsertAfter, const RECT& rc, UINT uFlags) const
    {
        assert(IsWindow());
        return ::DeferWindowPos(hWinPosInfo, *this, hWndInsertAfter, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, uFlags);
    }


    // This function provides default processing for any window messages that an application does not process.
    inline LRESULT CWnd::DefWindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam) const
    {
        assert(IsWindow());
        return ::DefWindowProc(*this, uMsg, wParam, lParam);
    }


    // The DrawMenuBar function redraws the menu bar of the specified window. If the menu bar changes after
    // the system has created the window, this function must be called to draw the changed menu bar.
    inline BOOL CWnd::DrawMenuBar() const
    {
        assert(IsWindow());
        return ::DrawMenuBar(*this);
    }


    // The EnableWindow function enables or disables mouse and
    // keyboard input to the window.
    inline BOOL CWnd::EnableWindow(BOOL Enable /*= TRUE*/) const
    {
        assert(IsWindow());
        return ::EnableWindow(*this, Enable);
    }


    // The EndPaint function marks the end of painting in the specified window. This function is required for
    // each call to the BeginPaint function, but only after painting is complete.
    inline BOOL CWnd::EndPaint(PAINTSTRUCT& ps) const
    {
        assert(IsWindow());
        return ::EndPaint(*this, &ps);
    }


    // The GetActiveWindow function retrieves the active window attached to the calling
    // thread's message queue.
    inline CWnd CWnd::GetActiveWindow() const
    {
        return CWnd(::GetActiveWindow() );
    }


    // The GetCapture function retrieves the window (if any) that has captured the mouse.
    inline CWnd CWnd::GetCapture() const
    {
        return CWnd( ::GetCapture() );
    }


    // The GetClassLongPtr function retrieves the specified value from the
    // WNDCLASSEX structure associated with the window.
    // Possible nIndex values: GCL_CBCLSEXTRA, GCL_CBWNDEXTRA, GCLP_ HBRBACKGROUND, GCLP_HCURSOR,
    // GCLP_HICON, GCLP_HICONSM, GCLP_HMODULE, GCLP_MENUNAME, GCL_STYLE, GCLP_WNDPROC.
    inline ULONG_PTR CWnd::GetClassLongPtr(int nIndex) const
    {
        assert(IsWindow());
        return ::GetClassLongPtr(*this, nIndex);
    }


    // The GetClientRect function retrieves the coordinates of a window's client area.
    // The client coordinates specify the upper-left and lower-right corners of the
    // client area. Because client coordinates are relative to the upper-left corner
    // of a window's client area, the coordinates of the upper-left corner are (0,0).
    inline CRect CWnd::GetClientRect() const
    {
        assert(IsWindow());
        CRect rc;
        VERIFY(::GetClientRect(*this, &rc) != 0);
        return rc;
    }


    // The GetDC function retrieves the display device context (DC)
    // for the client area of the window. Use like this:
    // CDC dc = GetDC;
    inline CDC CWnd::GetDC() const
    {
        assert(IsWindow());
        return CClientDC(*this);
    }


    // The GetDCEx function retrieves a display device context (DC) for the
    // client area or entire area of a window. Use like this:
    // CDC dc = GetDCEx(hrgnClip, flags);
    inline CDC CWnd::GetDCEx(HRGN hrgnClip, DWORD flags) const
    {
        assert(IsWindow());
        return CClientDCEx(*this, hrgnClip, flags);
    }


    // The GetDesktopWindow function retrieves the desktop window.
    inline CWnd CWnd::GetDesktopWindow() const
    {
        return CWnd( ::GetDesktopWindow() );
    }


    // Retrieves the control ID value for any child window.
    inline int CWnd::GetDlgCtrlID() const
    {
        assert(IsWindow());
        return ::GetDlgCtrlID(*this);
    }


    // The GetDlgItem function retrieves a handle to a control in the dialog box.
    inline CWnd CWnd::GetDlgItem(int nIDDlgItem) const
    {
        assert(IsWindow());
        return CWnd( ::GetDlgItem(*this, nIDDlgItem) );
    }


    // The GetDlgItemInt function translates the text of a specified control in a dialog box into an integer value.
    inline UINT CWnd::GetDlgItemInt(int nIDDlgItem, BOOL& IsTranslated, BOOL IsSigned) const
    {
        assert(IsWindow());
        return ::GetDlgItemInt(*this, nIDDlgItem, &IsTranslated, IsSigned);
    }


    // The GetDlgItemInt function translates the text of a specified control in a dialog box into an integer value.
    inline UINT CWnd::GetDlgItemInt(int nIDDlgItem, BOOL IsSigned) const
    {
        assert(IsWindow());
        return ::GetDlgItemInt(*this, nIDDlgItem, NULL, IsSigned);
    }


    // Retrieves the window's extended window style.
    inline DWORD CWnd::GetExStyle() const
    {
        assert(IsWindow());
        return static_cast<DWORD>(GetWindowLongPtr(GWL_EXSTYLE));
    }


    // The GetFocus function retrieves the window that has the keyboard focus, if the window
    // is attached to the calling thread's message queue.
    inline CWnd CWnd::GetFocus() const
    {
        return CWnd( ::GetFocus() );
    }


    // Retrieves the font with which the window is currently drawing its text.
    inline CFont CWnd::GetFont() const
    {
        assert(IsWindow());
        HFONT hFont = reinterpret_cast<HFONT>(SendMessage(WM_GETFONT, 0, 0));
        return CFont(hFont);
    }


    // Retrieves a handle to the large or small icon associated with a window.
    inline HICON CWnd::GetIcon(BOOL IsBigIcon) const
    {
        assert(IsWindow());
        return reinterpret_cast<HICON>(SendMessage(WM_GETICON, IsBigIcon, 0));
    }


    // The GetNextDlgGroupItem function retrieves the first control in a group of controls that
    // precedes (or follows) the specified control in a dialog box.
    inline CWnd CWnd::GetNextDlgGroupItem(HWND hCtl, BOOL IsPrevious) const
    {
        assert(IsWindow());
        return CWnd(::GetNextDlgGroupItem(*this, hCtl, IsPrevious));
    }


    // The GetNextDlgTabItem function retrieves the first control that has the WS_TABSTOP style
    // that precedes (or follows) the specified control.
    inline CWnd CWnd::GetNextDlgTabItem(HWND hCtl, BOOL IsPrevious) const
    {
        assert(IsWindow());
        return CWnd(::GetNextDlgTabItem(*this, hCtl, IsPrevious));
    }


    // The GetParent function retrieves the specified window's parent or owner.
    inline CWnd CWnd::GetParent() const
    {
        assert(IsWindow());
        return CWnd(::GetParent(*this));
    }


    // The GetScrollInfo function retrieves the parameters of a scroll bar, including
    // the minimum and maximum scrolling positions, the page size, and the position
    // of the scroll box (thumb).
    inline BOOL CWnd::GetScrollInfo(int fnBar, SCROLLINFO& si) const
    {
        assert(IsWindow());
        return ::GetScrollInfo(*this, fnBar, &si);
    }


    // Retrieves the window's window style.
    inline DWORD CWnd::GetStyle() const
    {
        assert(IsWindow());
        return static_cast<DWORD>(GetWindowLongPtr(GWL_STYLE));
    }


    // The GetUpdateRect function retrieves the coordinates of the smallest rectangle that completely
    // encloses the update region of the specified window.
    inline CRect CWnd::GetUpdateRect(BOOL Erase) const
    {
        assert(IsWindow());
        CRect rc;
        ::GetUpdateRect(*this, &rc, Erase);
        return rc;
    }


    // The GetUpdateRgn function retrieves the update region of a window by copying it into the specified region.
    inline int CWnd::GetUpdateRgn(HRGN hRgn, BOOL Erase) const
    {
        assert(IsWindow());
        return ::GetUpdateRgn(*this, hRgn, Erase);
    }


    // The GetWindow function retrieves a window that has the specified
    // relationship (Z-Order or owner) to the specified window.
    // Possible uCmd values: GW_CHILD, GW_ENABLEDPOPUP, GW_HWNDFIRST, GW_HWNDLAST,
    // GW_HWNDNEXT, GW_HWNDPREV, GW_OWNER
    inline CWnd CWnd::GetWindow(UINT uCmd) const
    {
        assert(IsWindow());
        return CWnd( ::GetWindow(*this, uCmd) );
    }


    // The GetWindowDC function retrieves the device context (DC) for the entire window,
    // including title bar, menus, and scroll bars. Use like this:
    // CDC dc = GetWindowDC();
    inline CDC CWnd::GetWindowDC() const
    {
        assert(IsWindow());
        return CWindowDC(*this);
    }


    // The GetWindowLongPtr function retrieves information about the window.
    // Possible nIndex values: GWL_EXSTYLE, GWL_STYLE, GWLP_WNDPROC, GWLP_HINSTANCE
    // GWLP_HWNDPARENT, GWLP_ID, GWLP_USERDATA.
    // Additional nIndex values for dialogs: DWLP_DLGPROC, DWLP_MSGRESULT, DWLP_USER.
    inline LONG_PTR CWnd::GetWindowLongPtr(int nIndex) const
    {
        assert(IsWindow());
        return ::GetWindowLongPtr(*this, nIndex);
    }


    // retrieves the dimensions of the bounding rectangle of the window.
    // The dimensions are given in screen coordinates that are relative to the
    // upper-left corner of the screen.
    inline CRect CWnd::GetWindowRect() const
    {
        assert(IsWindow());
        CRect rc;
        VERIFY(::GetWindowRect(*this, &rc) != 0);
        return rc;
    }


    // The GetWindowTextLength function retrieves the length, in characters, of the specified window's
    // title bar text (if the window has a title bar).
    inline int CWnd::GetWindowTextLength() const
    {
        assert(IsWindow());
        return ::GetWindowTextLength(*this);
    }


    // The Invalidate function adds the entire client area to the window's update region.
    // The update region represents the portion of the window's client area that must be redrawn.
    inline void CWnd::Invalidate(BOOL Erase /*= TRUE*/) const
    {
        assert(IsWindow());
        ::InvalidateRect(*this, NULL, Erase);
    }


    // The InvalidateRect function adds a rectangle to the window's update region.
    // The update region represents the portion of the window's client area that must be redrawn.
    inline BOOL CWnd::InvalidateRect(const RECT& rc, BOOL Erase /*= TRUE*/) const
    {
        assert(IsWindow());
        return ::InvalidateRect(*this, &rc, Erase);
    }


    // The InvalidateRect function adds a rectangle to the window's update region.
    // The entire window's client area is redrawn when no rectangle is specified.
    inline BOOL CWnd::InvalidateRect(BOOL Erase /*= TRUE*/) const
    {
        assert(IsWindow());
        return ::InvalidateRect(*this, NULL, Erase);
    }


    // The InvalidateRgn function invalidates the client area within the specified region
    // by adding it to the current update region of a window. The invalidated region,
    // along with all other areas in the update region, is marked for painting when the
    // next WM_PAINT message occurs.
    inline BOOL CWnd::InvalidateRgn(HRGN hRgn, BOOL Erase /*= TRUE*/) const
    {
        assert(IsWindow());
        return ::InvalidateRgn(*this, hRgn, Erase);
    }


    // The IsChild function tests whether a window is a child window or descendant window
    // of a parent window's CWnd.
    inline BOOL CWnd::IsChild(HWND hwndChild) const
    {
        assert(IsWindow());
        return ::IsChild(*this, hwndChild);
    }


    // The IsDialogMessage function determines whether a message is intended for the specified dialog box and,
    // if it is, processes the message.
    inline BOOL CWnd::IsDialogMessage(MSG& Msg) const
    {
        assert(IsWindow());
        return ::IsDialogMessage(*this, &Msg);
    }


    // The IsDlgButtonChecked function determines whether a button control has a check mark next to it
    // or whether a three-state button control is grayed, checked, or neither.
    //
    // The return value from a button created with the BS_AUTOCHECKBOX, BS_AUTORADIOBUTTON, BS_AUTO3STATE, 
    // BS_CHECKBOX, BS_RADIOBUTTON, or BS_3STATE style can be one of the following:
    // BST_CHECKED Button is checked. 
    // BST_INDETERMINATE Button is grayed, indicating an indeterminate state (applies only if the button
    //      has the BS_3STATE or BS_AUTO3STATE style).
    // BST_UNCHECKED Button is cleared.
    //
    // If the button has any other style, the return value is zero.
    inline UINT CWnd::IsDlgButtonChecked(int nIDButton) const
    {
        assert(IsWindow());
        return ::IsDlgButtonChecked(*this, nIDButton);
    }


    // The IsWindowEnabled function determines whether the window is enabled
    // for mouse and keyboard input.
    inline BOOL CWnd::IsWindowEnabled() const
    {
        assert(IsWindow());
        return ::IsWindowEnabled(*this);
    }


    // The IsWindowVisible function retrieves the visibility state of the window.
    inline BOOL CWnd::IsWindowVisible() const
    {
        // Microsoft's IsWindowVisible is buggy, so we do it like this
        assert(IsWindow());
        return (GetWindowLongPtr(GWL_STYLE) & WS_VISIBLE) != 0;
    }


    // The IsWindow function determines whether the window exists.
    inline BOOL CWnd::IsWindow() const
    {
        return ::IsWindow(*this);
    }


    // The MapWindowPoints function converts (maps) a set of points from a coordinate space relative to one
    // window to a coordinate space relative to another window.
    inline void  CWnd::MapWindowPoints(HWND hWndTo, POINT& pt) const
    {
        assert(IsWindow());
        ::MapWindowPoints(*this, hWndTo, &pt, 1);
    }


    // The MapWindowPoints function converts (maps) a set of points from a coordinate space relative to one
    // window to a coordinate space relative to another window.
    inline void CWnd::MapWindowPoints(HWND hWndTo, RECT& rc) const
    {
        assert(IsWindow());
        ::MapWindowPoints(*this, hWndTo, (LPPOINT)&rc, 2);
    }


    // The MapWindowPoints function converts (maps) a set of points from a coordinate space relative to one
    // window to a coordinate space relative to another window.
    inline void CWnd::MapWindowPoints(HWND hWndTo, LPPOINT ptArray, UINT nCount) const
    {
        assert(IsWindow());
        ::MapWindowPoints(*this, hWndTo, (LPPOINT)ptArray, nCount);
    }


    // The MessageBox function creates, displays, and operates a message box.
    // Possible combinations of uType values include: MB_OK, MB_HELP, MB_OKCANCEL, MB_RETRYCANCEL,
    // MB_YESNO, MB_YESNOCANCEL, MB_ICONEXCLAMATION, MB_ICONWARNING, MB_ICONERROR (+ many others).
    inline int CWnd::MessageBox(LPCTSTR lpText, LPCTSTR lpCaption, UINT uType) const
    {
        assert(IsWindow());
        return ::MessageBox(*this, lpText, lpCaption, uType);
    }


    // The MoveWindow function changes the position and dimensions of the window.
    inline BOOL CWnd::MoveWindow(int x, int y, int nWidth, int nHeight, BOOL Repaint /* = TRUE*/) const
    {
        assert(IsWindow());
        return ::MoveWindow(*this, x, y, nWidth, nHeight, Repaint = TRUE);
    }


    // The MoveWindow function changes the position and dimensions of the window.
    inline BOOL CWnd::MoveWindow(const RECT& rc, BOOL Repaint /* = TRUE*/) const
    {
        assert(IsWindow());
        return ::MoveWindow(*this, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, Repaint);
    }


    // The PostMessage function places (posts) a message in the message queue
    // associated with the thread that created the window and returns without
    // waiting for the thread to process the message.
    inline BOOL CWnd::PostMessage(UINT uMsg, WPARAM wParam /*= 0L*/, LPARAM lParam /*= 0L*/) const
    {
        assert(IsWindow());
        return ::PostMessage(*this, uMsg, wParam, lParam);
    }


    // The PostMessage function places (posts) a message in the message queue
    // associated with the thread that created the window and returns without
    // waiting for the thread to process the message.
    inline BOOL CWnd::PostMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) const
    {
        // Required by by some macros
        assert(IsWindow());
        return ::PostMessage(hWnd, uMsg, wParam, lParam);
    }


    // The RedrawWindow function updates the specified rectangle in a window's client area.
    inline BOOL CWnd::RedrawWindow(const RECT& rcUpdate, UINT flags) const
    {
        assert(IsWindow());
        return ::RedrawWindow(*this, &rcUpdate, 0, flags);
    }


    // The RedrawWindow function updates the specified region in a window's client area.
    inline BOOL CWnd::RedrawWindow(HRGN hRgn, UINT flags) const
    {
        assert(IsWindow());
        return ::RedrawWindow(*this, 0, hRgn, flags);
    }


    // The RedrawWindow function updates the entire window's client area.
    inline BOOL CWnd::RedrawWindow(UINT flags) const
    {
        assert(IsWindow());
        return ::RedrawWindow(*this, 0, 0, flags);
    }


    // The ReleaseDC function releases a device context (DC), freeing it for use
    // by other applications.
    inline int CWnd::ReleaseDC(HDC hDC) const
    {
        assert(IsWindow());
        return ::ReleaseDC(*this, hDC);
    }


    // The ScreenToClient function converts the screen coordinates of a specified point on the screen to client-area coordinates.
    inline BOOL CWnd::ScreenToClient(POINT& Point) const
    {
        assert(IsWindow());
        return ::ScreenToClient(*this, &Point);
    }


    // The ScreenToClient function converts the screen coordinates of a specified RECT on the screen to client-area coordinates.
    inline BOOL CWnd::ScreenToClient(RECT& rc) const
    {
        assert(IsWindow());
        return static_cast<BOOL>(::MapWindowPoints(NULL, *this, (LPPOINT)&rc, 2));
    }


    // The SendDlgItemMessage function sends a message to the specified control in a dialog box.
    inline LRESULT CWnd::SendDlgItemMessage(int nIDDlgItem, UINT Msg, WPARAM wParam, LPARAM lParam) const
    {
        assert(IsWindow());
        return ::SendDlgItemMessage(*this, nIDDlgItem, Msg, wParam, lParam);
    }


    // The SendMessage function sends the specified message to a window or windows.
    // It calls the window procedure for the window and does not return until the
    // window procedure has processed the message.
    inline LRESULT CWnd::SendMessage(UINT uMsg, WPARAM wParam /*= 0L*/, LPARAM lParam /*= 0L*/) const
    {
        assert(IsWindow());
        return ::SendMessage(*this, uMsg, wParam, lParam);
    }


    // The SendMessage function sends the specified message to a window or windows.
    // It calls the window procedure for the window and does not return until the
    // window procedure has processed the message.
    inline LRESULT CWnd::SendMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) const
    {
        // Required by by some macros
        assert(IsWindow());
        return ::SendMessage(hWnd, uMsg, wParam, lParam);
    }


    // The SendNotifyMessage function sends the specified message to a window or windows. If the window was created by the
    // calling thread, SendNotifyMessage calls the window procedure for the window and does not return until the window procedure
    // has processed the message. If the window was created by a different thread, SendNotifyMessage passes the message to the
    // window procedure and returns immediately; it does not wait for the window procedure to finish processing the message.
    inline BOOL CWnd::SendNotifyMessage(UINT Msg, WPARAM wParam, LPARAM lParam) const
    {
        assert(IsWindow());
        return ::SendNotifyMessage(*this, Msg, wParam, lParam);
    }


    // The SetActiveWindow function activates the window, but
    // not if the application is in the background.
    inline CWnd CWnd::SetActiveWindow() const
    {
        assert(IsWindow());
        return CWnd( ::SetActiveWindow(*this) );
    }


    // The SetCapture function sets the mouse capture to the window.
    // SetCapture captures mouse input either when the mouse is over the capturing
    // window, or when the mouse button was pressed while the mouse was over the
    // capturing window and the button is still down.
    inline CWnd CWnd::SetCapture() const
    {
        assert(IsWindow());
        return CWnd( ::SetCapture(*this) );
    }


    // The SetClassLongPtr function replaces the specified value at the specified offset in the
    // extra class memory or the WNDCLASSEX structure for the class to which the window belongs.
    // Possible nIndex values: GCL_CBCLSEXTRA, GCL_CBWNDEXTRA, GCLP_ HBRBACKGROUND, GCLP_HCURSOR,
    // GCLP_HICON, GCLP_HICONSM, GCLP_HMODULE, GCLP_MENUNAME, GCL_STYLE, GCLP_WNDPROC.
    inline ULONG_PTR CWnd::SetClassLongPtr(int nIndex, LONG_PTR dwNewLong) const
    {
        assert(IsWindow());
        return ::SetClassLongPtr(*this, nIndex, dwNewLong);
    }


    // Assigns an id to the window. Note that only child windows can have an ID assigned
    inline LONG_PTR CWnd::SetDlgCtrlID(int idCtrl) const
    {
        assert(IsWindow());
        return SetWindowLongPtr(GWLP_ID, idCtrl);
    }


    // The SetDlgItemInt function sets the text of a control in a dialog box to the string representation of a specified integer value.
    inline BOOL CWnd::SetDlgItemInt(int nIDDlgItem, UINT uValue, BOOL IsSigned) const
    {
        assert(IsWindow());
        return ::SetDlgItemInt(*this, nIDDlgItem, uValue, IsSigned);
    }


    // The SetDlgItemText function sets the title or text of a control in a dialog box.
    inline BOOL CWnd::SetDlgItemText(int nIDDlgItem, LPCTSTR lpString) const
    {
        assert(IsWindow());
        return ::SetDlgItemText(*this, nIDDlgItem, lpString);
    }


    // Assigns a new windows extended style to the window.
    inline void CWnd::SetExStyle(DWORD dwExStyle) const
    {
        assert(IsWindow());
        ::SetWindowLongPtr(*this, GWL_EXSTYLE, dwExStyle);
    }


    // The SetFocus function sets the keyboard focus to the window.
    inline CWnd CWnd::SetFocus() const
    {
        assert(IsWindow());
        return CWnd( ::SetFocus(*this) );
    }


    // Specifies the font that the window will use when drawing text.
    inline void CWnd::SetFont(HFONT hFont, BOOL Redraw /* = TRUE*/) const
    {
        assert(IsWindow());
        SendMessage(WM_SETFONT, reinterpret_cast<WPARAM>(hFont), Redraw);
    }


    // Associates a new large or small icon with a window.
    inline HICON CWnd::SetIcon(HICON hIcon, BOOL IsBigIcon) const
    {
        assert(IsWindow());
        return reinterpret_cast<HICON>(SendMessage(WM_SETICON, IsBigIcon, reinterpret_cast<LPARAM>(hIcon)));
    }


    // The SetForegroundWindow function puts the thread that created the window into the
    // foreground and activates the window.
    inline BOOL CWnd::SetForegroundWindow() const
    {
        assert(IsWindow());
        return ::SetForegroundWindow(*this);
    }


    // The SetParent function changes the parent window of the child window.
    inline CWnd CWnd::SetParent(HWND hWndParent) const
    {
        assert(IsWindow());
        return CWnd(::SetParent(*this, hWndParent));
    }


    // This function allows changes in that window to be redrawn or prevents changes
    // in that window from being redrawn.
    inline BOOL CWnd::SetRedraw(BOOL Redraw /*= TRUE*/) const
    {
        assert(IsWindow());
        return static_cast<BOOL>(::SendMessage(*this, WM_SETREDRAW, Redraw, 0L));
    }


    // Assigns a new windows style to the window.
    inline void CWnd::SetStyle(DWORD dwStyle) const
    {
        assert(IsWindow());
        ::SetWindowLongPtr(*this, GWL_STYLE, dwStyle);
    }


    // Creates a timer with the specified time-out value.
    inline UINT_PTR CWnd::SetTimer(UINT_PTR nIDEvent, UINT uElapse, TIMERPROC lpTimerFunc) const
    {
        assert(IsWindow());
        return ::SetTimer(*this, nIDEvent, uElapse, lpTimerFunc);
    }


    // The SetWindowLongPtr function changes an attribute of the window.
    // Possible nIndex values: GWL_EXSTYLE, GWL_STYLE, GWLP_WNDPROC, GWLP_HINSTANCE
    // GWLP_HWNDPARENT, GWLP_ID, GWLP_USERDATA.
    // Additional nIndex values for dialogs: DWLP_DLGPROC, DWLP_MSGRESULT, DWLP_USER.
    inline LONG_PTR CWnd::SetWindowLongPtr(int nIndex, LONG_PTR dwNewLong) const
    {
        assert(IsWindow());
        return ::SetWindowLongPtr(*this, nIndex, dwNewLong);
    }


    // The SetWindowPos function changes the size, position, and Z order of a child, pop-up,
    // or top-level window.
    // The pInsertAfter can one of:  &wndTop, &wndTopMost, &wndBottom, or &wndNoTopMost.
    inline BOOL CWnd::SetWindowPos(HWND hWndInsertAfter, int x, int y, int cx, int cy, UINT uFlags) const
    {
        assert(IsWindow());
        return ::SetWindowPos(*this, hWndInsertAfter, x, y, cx, cy, uFlags);
    }


    // The SetWindowPos function changes the size, position, and Z order of a child, pop-up,
    // or top-level window.
    // The SetWindowPos function changes the size, position, and Z order of a child, pop-up,
    // or top-level window.
    // The pInsertAfter can one of:  &wndTop, &wndTopMost, &wndBottom, or &wndNoTopMost.
    inline BOOL CWnd::SetWindowPos(HWND hWndInsertAfter, const RECT& rc, UINT uFlags) const
    {
        assert(IsWindow());
        return ::SetWindowPos(*this, hWndInsertAfter, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, uFlags);
    }


    // The SetWindowRgn function sets the window region of the window.
    // The window region determines the area within the window where the system permits drawing.
    // The window now owns the region so it is detached from Rgn.
    inline int CWnd::SetWindowRgn(CRgn& Rgn, BOOL Redraw /*= TRUE*/) const
    {
        assert(IsWindow());
        HRGN hRgn = reinterpret_cast<HRGN>(Rgn.GetHandle());
        int iResult = ::SetWindowRgn(*this, hRgn, Redraw);
        if (iResult && hRgn)
            Rgn.Detach();   // The system owns the region now

        return iResult;
    }


    // The SetWindowText function changes the text of the window's title bar (if it has one).
    inline BOOL CWnd::SetWindowText(LPCTSTR lpString) const
    {
        assert(IsWindow());
        return ::SetWindowText(*this, lpString);
    }


    // Set the XP Theme for a window.
    // Examples:
    //  SetWindowTheme(NULL, NULL);     // Reverts the window's XP theme back to default
    //  SetWindowTheme(L" ", L" ");     // Disables XP theme for the window
    inline HRESULT CWnd::SetWindowTheme(LPCWSTR pszSubAppName, LPCWSTR pszSubIdList) const
    {
        HRESULT hr = E_NOTIMPL;

#ifndef _WIN32_WCE

        HMODULE hMod = ::LoadLibrary(_T("uxtheme.dll"));
        if(hMod != 0)
        {
            typedef HRESULT (__stdcall *PFNSETWINDOWTHEME)(HWND hWnd, LPCWSTR pszSubAppName, LPCWSTR pszSubIdList);
            PFNSETWINDOWTHEME pfn = (PFNSETWINDOWTHEME)GetProcAddress(hMod, "SetWindowTheme");

            hr = pfn(*this, pszSubAppName, pszSubIdList);

            ::FreeLibrary(hMod);
        }

#endif

        return hr;
    }


    // The ShowWindow function sets the window's show state.
    inline BOOL CWnd::ShowWindow(int nCmdShow /*= SW_SHOWNORMAL*/) const
    {
        assert(IsWindow());
        return ::ShowWindow(*this, nCmdShow);
    }


    // The UpdateWindow function updates the client area of the window by sending a
    // WM_PAINT message to the window if the window's update region is not empty.
    // If the update region is empty, no message is sent.
    inline BOOL CWnd::UpdateWindow() const
    {
        assert(IsWindow());
        return ::UpdateWindow(*this);
    }


    // The ValidateRect function validates the client area within a rectangle by
    // removing the rectangle from the update region of the window.
    inline BOOL CWnd::ValidateRect(const RECT& rc) const
    {
        assert(IsWindow());
        return ::ValidateRect(*this, &rc);
    }


    // The ValidateRect function validates the entire client area of the window.
    inline BOOL CWnd::ValidateRect() const
    {
        assert(IsWindow());
        return ::ValidateRect(*this, NULL);
    }


    // The ValidateRgn function validates the client area within a region by
    // removing the region from the current update region of the window.
    inline BOOL CWnd::ValidateRgn(HRGN hRgn) const
    {
        assert(IsWindow());
        return ::ValidateRgn(*this, hRgn);
    }


    // Retrieves the window that contains the specified point (in screen coordinates).
    inline CWnd CWnd::WindowFromPoint(POINT pt)
    {
        return CWnd(::WindowFromPoint(pt));
    }

    //
    // These functions aren't supported on WinCE
    //

  #ifndef _WIN32_WCE

    // The CloseWindow function minimizes (but does not destroy) the window.
    // To destroy a window, an application can use the Destroy function.
    inline BOOL CWnd::CloseWindow() const
    {
        assert(IsWindow());
        return ::CloseWindow(*this);
    }


    // The DlgDirList function replaces the contents of a list box with the names of the subdirectories and files
    // in a specified directory. You can filter the list of names by specifying a set of file attributes.
    inline int CWnd::DlgDirList(LPTSTR lpPathSpec, int nIDListBox, int nIDStaticPath, UINT uFileType) const
    {
        assert(IsWindow());
        return ::DlgDirList(*this, lpPathSpec, nIDListBox, nIDStaticPath, uFileType);
    }


    // The DlgDirListComboBox function replaces the contents of a combo box with the names of the subdirectories
    // and files in a specified directory. You can filter the list of names by specifying a set of file attributes.
    inline int CWnd::DlgDirListComboBox(LPTSTR lpPathSpec, int nIDComboBox, int nIDStaticPath, UINT uFiletype) const
    {
        assert(IsWindow());
        return ::DlgDirListComboBox(*this, lpPathSpec, nIDComboBox, nIDStaticPath, uFiletype);
    }


    // The DlgDirSelectEx function retrieves the current selection from a single-selection list box. It assumes that the list box
    // has been filled by the DlgDirList function and that the selection is a drive letter, filename, or directory name.
    inline BOOL CWnd::DlgDirSelectEx(LPTSTR lpString, int nCount, int nIDListBox) const
    {
        assert(IsWindow());
        return ::DlgDirSelectEx(*this, lpString, nCount, nIDListBox);
    }


    // The DlgDirSelectComboBoxEx function retrieves the current selection from a combo box filled by using the
    // DlgDirListComboBox function. The selection is interpreted as a drive letter, a file, or a directory name.
    inline BOOL CWnd::DlgDirSelectComboBoxEx(LPTSTR lpString, int nCount, int nIDComboBox) const
    {
        assert(IsWindow());
        return ::DlgDirSelectComboBoxEx(*this, lpString, nCount, nIDComboBox);
    }

    #ifndef WIN32_LEAN_AND_MEAN

    // Registers whether a window accepts dropped files.
    inline void CWnd::DragAcceptFiles(BOOL Accept) const
    {
        assert(IsWindow());
        ::DragAcceptFiles(*this, Accept);
    }

    #endif

    // The DrawAnimatedRects function draws a wire-frame rectangle and animates it to indicate the opening of
    // an icon or the minimizing or maximizing of a window.
    inline BOOL CWnd::DrawAnimatedRects(int idAni, const RECT& rcFrom, const RECT& rcTo) const
    {
        assert(IsWindow());
        return ::DrawAnimatedRects(*this, idAni, &rcFrom, &rcTo);
    }


    // The DrawCaption function draws a window caption.
    inline BOOL CWnd::DrawCaption(HDC hDC, const RECT& rc, UINT uFlags) const
    {
        assert(IsWindow());
        return ::DrawCaption(*this, hDC, &rc, uFlags);
    }


    // The EnableScrollBar function enables or disables one or both scroll bar arrows.
    inline BOOL CWnd::EnableScrollBar(UINT uSBflags, UINT uArrows) const
    {
        assert(IsWindow());
        return ::EnableScrollBar(*this, uSBflags, uArrows);
    }


    // The GetLastActivePopup function determines which pop-up window owned by the specified window was most recently active.
    inline CWnd CWnd::GetLastActivePopup() const
    {
        assert(IsWindow());
        return CWnd( ::GetLastActivePopup(*this) );
    }


    // The GetMenu function retrieves a handle to the menu assigned to the window.
    inline CMenu CWnd::GetMenu() const
    {
        assert(IsWindow());
        return CMenu(::GetMenu(*this));
    }


    // The GetScrollPos function retrieves the current position of the scroll box
    // (thumb) in the specified scroll bar.
    inline int CWnd::GetScrollPos(int nBar) const
    {
        assert(IsWindow());
        return ::GetScrollPos(*this, nBar);
    }


    // The GetScrollRange function retrieves the current minimum and maximum scroll box
    // (thumb) positions for the specified scroll bar.
    inline BOOL CWnd::GetScrollRange(int nBar, int& MinPos, int& MaxPos) const
    {
        assert(IsWindow());
        return ::GetScrollRange(*this, nBar, &MinPos, &MaxPos );
    }


    // The GetSystemMenu function allows the application to access the window menu (also known as the system menu
    // or the control menu) for copying and modifying.
    inline CMenu CWnd::GetSystemMenu(BOOL RevertToDefault) const
    {
        assert(IsWindow());
        return CMenu( ::GetSystemMenu(*this, RevertToDefault) );
    }


    // The GetTopWindow function examines the Z order of the child windows associated with the parent window and
    // retrieves a handle to the child window at the top of the Z order.
    inline CWnd CWnd::GetTopWindow() const
    {
        assert(IsWindow());
        return CWnd( ::GetTopWindow(*this) );
    }


    // The GetWindowPlacement function retrieves the show state and the restored,
    // minimized, and maximized positions of the window.
    inline BOOL CWnd::GetWindowPlacement(WINDOWPLACEMENT& wndpl) const
    {
        assert(IsWindow());
        return ::GetWindowPlacement(*this, &wndpl);
    }


    // The HiliteMenuItem function highlights or removes the highlighting from an item in a menu bar.
    inline BOOL CWnd::HiliteMenuItem(HMENU hMenu, UINT uItemHilite, UINT uHilite) const
    {
        assert(IsWindow());
        return ::HiliteMenuItem(*this, hMenu, uItemHilite, uHilite);
    }


    // The IsIconic function determines whether the window is minimized (iconic).
    inline BOOL CWnd::IsIconic() const
    {
        assert(IsWindow());
        return ::IsIconic(*this);
    }


    // The IsZoomed function determines whether the window is maximized.
    inline BOOL CWnd::IsZoomed() const
    {
        assert(IsWindow());
        return ::IsZoomed(*this);
    }


    // Destroys the specified timer.
    inline BOOL CWnd::KillTimer(UINT_PTR uIDEvent) const
    {
        assert(IsWindow());
        return ::KillTimer(*this, uIDEvent);
    }


    // Disables drawing in the window. Only one window can be locked at a time.
    // Use UnLockWindowUpdate to re-enable drawing in the window.
    inline BOOL CWnd::LockWindowUpdate() const
    {
        assert(IsWindow());
        return ::LockWindowUpdate(*this);
    }


    // The OpenIcon function restores a minimized (iconic) window to its previous size and position.
    inline BOOL CWnd::OpenIcon() const
    {
        assert(IsWindow());
        return ::OpenIcon(*this);
    }


    // Requests that the window draw itself in the specified device context, most commonly in a printer device context.
    inline void CWnd::Print(HDC hDC, DWORD dwFlags) const
    {
        assert(IsWindow());
        SendMessage(*this, WM_PRINT, reinterpret_cast<WPARAM>(hDC), dwFlags);
    }


    // The ScrollWindow function scrolls the contents of the window's client area.
    // rcScroll specifies the portion of the client area to be scrolled.
    // prcClip points to the clipping rectangle to scroll. Only bits inside this rectangle are scrolled.
    // If prcClip is NULL, no clipping is performed on the scroll rectangle.
    inline BOOL CWnd::ScrollWindow(int XAmount, int YAmount, const RECT& rcScroll, LPCRECT prcClip) const
    {
        assert(IsWindow());
        return ::ScrollWindow(*this, XAmount, YAmount, &rcScroll, prcClip);
    }


    // The ScrollWindow function scrolls the contents of the window's client area.
    // The entire client area is scrolled.
    // prcClip points to the clipping rectangle to scroll. Only bits inside this rectangle are scrolled.
    // If prcClip is NULL, no clipping is performed on the scroll rectangle.
    inline BOOL CWnd::ScrollWindow(int XAmount, int YAmount, LPCRECT prcClip) const
    {
        assert(IsWindow());
        return ::ScrollWindow(*this, XAmount, YAmount, NULL, prcClip);
    }


    // The ScrollWindow function scrolls the contents of the window's client area.
    // prcScroll: Pointer to a RECT structure that specifies the portion of the client area to be scrolled.
    //            If this parameter is NULL, the entire client area is scrolled.
    // prcClip:   Pointer to a RECT structure that contains the coordinates of the clipping rectangle.
    //            Only device bits within the clipping rectangle are affected. This parameter may be NULL.
    // hrgnUpdate: Handle to the region that is modified to hold the region invalidated by scrolling.
    //             This parameter may be NULL.
    // prcUpdate: Pointer to a RECT structure that receives the boundaries of the rectangle invalidated by scrolling.
    //            This parameter may be NULL.
    // flags:     Specifies flags that control scrolling.This parameter can be one of the following values.
    //   SW_ERASE:          Erases the newly invalidated region when specified with the SW_INVALIDATE flag.
    //   SW_INVALIDATE:     Invalidates the region identified by the hrgnUpdate parameter after scrolling.
    //   SW_SCROLLCHILDREN: Scrolls all child windows that intersect the rectangle pointed to by the prcScroll parameter.
    //   SW_SMOOTHSCROLL:   Scrolls using smooth scrolling.
    inline int CWnd::ScrollWindowEx(int dx, int dy, LPCRECT prcScroll, LPCRECT prcClip, HRGN hrgnUpdate, LPRECT prcUpdate, UINT flags) const
    {
        assert(IsWindow());
        return ::ScrollWindowEx(*this, dx, dy, prcScroll, prcClip, hrgnUpdate, prcUpdate, flags);
    }


    // The SetMenu function assigns a menu to the specified window.
    // A hMenu of NULL removes the menu.
    inline BOOL CWnd::SetMenu(HMENU hMenu) const
    {
        assert(IsWindow());
        return ::SetMenu(*this, hMenu);
    }


    // The SetScrollInfo function sets the parameters of a scroll bar, including
    // the minimum and maximum scrolling positions, the page size, and the
    // position of the scroll box (thumb).
    inline int CWnd::SetScrollInfo(int fnBar, const SCROLLINFO& si, BOOL Redraw) const
    {
        assert(IsWindow());
        return ::SetScrollInfo(*this, fnBar, &si, Redraw);
    }


    // The SetScrollPos function sets the position of the scroll box (thumb) in
    // the specified scroll bar.
    inline int CWnd::SetScrollPos(int nBar, int nPos, BOOL Redraw) const
    {
        assert(IsWindow());
        return ::SetScrollPos(*this, nBar, nPos, Redraw);
    }


    // The SetScrollRange function sets the minimum and maximum scroll box positions for the scroll bar.
    inline BOOL CWnd::SetScrollRange(int nBar, int nMinPos, int nMaxPos, BOOL Redraw) const
    {
        assert(IsWindow());
        return ::SetScrollRange(*this, nBar, nMinPos, nMaxPos, Redraw);
    }


    // The SetWindowPlacement function sets the show state and the restored, minimized,
    // and maximized positions of the window.
    inline BOOL CWnd::SetWindowPlacement(const WINDOWPLACEMENT& wndpl) const
    {
        assert(IsWindow());
        return ::SetWindowPlacement(*this, &wndpl);
    }


    // The ShowOwnedPopups function shows or hides all pop-up windows owned by the specified window.
    inline BOOL CWnd::ShowOwnedPopups(BOOL Show) const
    {
        assert(IsWindow());
        return ::ShowOwnedPopups(*this, Show);
    }


    // The ShowScrollBar function shows or hides the specified scroll bar.
    inline BOOL CWnd::ShowScrollBar(int nBar, BOOL Show) const
    {
        assert(IsWindow());
        return ::ShowScrollBar(*this, nBar, Show);
    }


    // The ShowWindowAsync function sets the show state of a window created by a different thread.
    inline BOOL CWnd::ShowWindowAsync(int nCmdShow) const
    {
        assert(IsWindow());
        return ::ShowWindowAsync(*this, nCmdShow);
    }


    // Enables drawing in the window. Only one window can be locked at a time.
    // Use LockWindowUpdate to disable drawing in the window.
    inline BOOL CWnd::UnLockWindowUpdate() const
    {
        assert(IsWindow());
        return ::LockWindowUpdate(0);
    }


    // The WindowFromDC function returns a handle to the window associated with the specified display device context (DC).
    inline CWnd CWnd::WindowFromDC(HDC hDC) const
    {
        return CWnd( ::WindowFromDC(hDC) );
    }

  #endif    // _WIN32_WCE



    /////////////////////////////////////////////////////////
    // Definitions of CString functions that require CWinApp
    //

    // Appends formatted data to the CStringT content.
    template <class T>
    inline void CStringT<T>::AppendFormat(UINT nFormatID, ...)
    {
        CStringT str1;
        CStringT str2;

        if (str1.LoadString(nFormatID))
        {
            va_list args;
            va_start(args, nFormatID);
            str2.FormatV(str1.c_str(), args);
            va_end(args);

            m_str.append(str2);
        }
    }


    // Formats the string as sprintf does.
    template <class T>
    inline void CStringT<T>::Format(UINT nID, ...)
    {
        CStringT str;
        if (str.LoadString(nID))
        {
            va_list args;
            va_start(args, nID);
            FormatV(str.c_str(), args);
            va_end(args);
        }
    }


    // Loads the string from a Windows resource.
    template <>
    inline bool CStringT<CHAR>::LoadString(UINT nID)
    {
        assert (&GetApp());

        int nSize = 64;
        CHAR* pTCharArray = 0;
        std::vector<CHAR> vString;
        int nTChars = nSize;

        Empty();

        // Increase the size of our array in a loop until we load the entire string
        // The ANSI and _UNICODE versions of LoadString behave differently. This technique works for both.
        while ( nSize-1 <= nTChars )
        {
            nSize = nSize * 4;
            vString.assign(nSize+1, 0);
            pTCharArray = &vString[0];
            nTChars = ::LoadStringA (GetApp().GetResourceHandle(), nID, pTCharArray, nSize);
        }

        if (nTChars > 0)
            m_str.assign(pTCharArray);

        return (nTChars != 0);
    }


    // Loads the string from a Windows resource.
    template <>
    inline bool CStringT<WCHAR>::LoadString(UINT nID)
    {
        assert (&GetApp());

        int nSize = 64;
        WCHAR* pTCharArray = 0;
        std::vector<WCHAR> vString;
        int nTChars = nSize;

        Empty();

        // Increase the size of our array in a loop until we load the entire string
        // The ANSI and _UNICODE versions of LoadString behave differently. This technique works for both.
        while ( nSize-1 <= nTChars )
        {
            nSize = nSize * 4;
            vString.assign(nSize+1, 0);
            pTCharArray = &vString[0];
            nTChars = ::LoadStringW (GetApp().GetResourceHandle(), nID, pTCharArray, nSize);
        }

        if (nTChars > 0)
            m_str.assign(pTCharArray);

        return (nTChars != 0);
    }

    ////////////////////////////////////////
    // Global Functions
    //

#ifndef _WIN32_WCE      // for Win32/64 operating systems, not WinCE


    // Returns a NONCLIENTMETRICS struct filled from the system parameters.
    inline NONCLIENTMETRICS GetNonClientMetrics()
    {
        NONCLIENTMETRICS ncm;
        ZeroMemory(&ncm, sizeof(ncm));
        ncm.cbSize = sizeof(NONCLIENTMETRICS);

#if (WINVER >= 0x0600)
        // Is OS version less than Vista, adjust size to correct value
        if (GetWinVersion() < 2600)
            ncm.cbSize = CCSIZEOF_STRUCT(NONCLIENTMETRICS, lfMessageFont);
#endif

        VERIFY(::SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0) != 0);

        return ncm;
    }


    // Reports the state of the left mouse button
    inline BOOL IsLeftButtonDown()
    {
        SHORT state;
        if (GetSystemMetrics(SM_SWAPBUTTON))
            // Mouse buttons are swapped
            state = GetAsyncKeyState(VK_RBUTTON);
        else
            // Mouse buttons are not swapped
            state = GetAsyncKeyState(VK_LBUTTON);

        // returns true if the left mouse button is down
        return (state & 0x8000);
    }


#endif // #ifndef _WIN32_WCE

    // Loads the common controls using InitCommonControlsEx or InitCommonControls.
    // Returns TRUE of InitCommonControlsEx is used.
    inline void LoadCommonControls()
    {
        // Load the Common Controls DLL
        HMODULE hComCtl = ::LoadLibrary(_T("COMCTL32.DLL"));
        if (hComCtl == 0)
            hComCtl = ::LoadLibrary(_T("COMMCTRL.DLL"));

        if (hComCtl)
        {
            // Declare a typedef for the InItCommonControlsEx function
            typedef BOOL WINAPI INIT_EX(INITCOMMONCONTROLSEX*);

#ifdef _WIN32_WCE
            INIT_EX* pfnInitEx = reinterpret_cast<INIT_EX*>(::GetProcAddress(hComCtl, _T("InitCommonControlsEx")));
#else
            INIT_EX* pfnInitEx = reinterpret_cast<INIT_EX*>(::GetProcAddress(hComCtl, "InitCommonControlsEx"));
#endif

            if (pfnInitEx)
            {
                // Load the full set of common controls
                INITCOMMONCONTROLSEX InitStruct;
                InitStruct.dwSize = sizeof(InitStruct);
                InitStruct.dwICC = ICC_WIN95_CLASSES | ICC_BAR_CLASSES | ICC_COOL_CLASSES | ICC_DATE_CLASSES;


#if (!defined _WIN32_WCE && _WIN32_IE >= 0x0401)
                if (GetComCtlVersion() > 470)
                    InitStruct.dwICC |= ICC_INTERNET_CLASSES | ICC_NATIVEFNTCTL_CLASS | ICC_PAGESCROLLER_CLASS | ICC_USEREX_CLASSES;
#endif

                // Call InitCommonControlsEx
                if (!(pfnInitEx(&InitStruct)))
                    InitCommonControls();
            }
            else
            {
                // InitCommonControlsEx not supported. Use older InitCommonControls
                InitCommonControls();
            }

            ::FreeLibrary(hComCtl);
        }
    }


    // Returns a CString containing the specified string resource.
    inline CString LoadString(UINT nID)
    {
        CString str;
        str.LoadString(nID);
        return str;
    }


}



#endif // _WIN32XX_WINCORE_H_

