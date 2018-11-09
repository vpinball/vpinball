// Win32++   Version 8.6
// Release Date: 2nd November 2018
//
//      David Nash
//      email: dnash@bigpond.net.au
//      url: https://sourceforge.net/projects/win32-framework
//
//
// Copyright (c) 2005-2018  David Nash
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



#ifndef _WIN32XX_APPCORE0_H_
#define _WIN32XX_APPCORE0_H_


///////////////////////////////////////////////////////
// wxx_appcore0.h
// This file contains the declarations of the following set of classes.
//
// 1) CCriticalSection: This class is used internally to manage thread access
//            to shared resources. You can also use this class to lock and
//            release your own critical sections.
//
// 2) CObject: A base class for CWnd and any other class that uses serialization.
//             It provides a virtual Serialize function for use by CArchive.
//
// 3) CThreadLock: Provides a RAII-style wrapper for CCriticalSection.
//
// 4) CWinThread: This class is the parent class for CWinApp. It is also the
//            class used to create additional GUI and worker threads.
//
// 5) CWinApp: This class is used start Win32++ and run the message loop. You
//            should inherit from this class to start Win32++ in your own
//            application.



//////////////////////////////////////
//  Include the C++ and windows header files
//

// Specify minimum acceptable version macros
// These are suitable for Windows 95
#ifndef WINVER
  #define WINVER          0x0400
  #ifndef _WIN32_WINDOWS
    #define _WIN32_WINDOWS  0x0400
  #endif
#endif
#ifndef _WIN32_IE
 #define _WIN32_IE        0x0400
#endif

// Remove pointless warning messages
#ifdef _MSC_VER
  #if _MSC_VER < 1310    // before VS2003
    #pragma warning (disable : 4511) // copy operator could not be generated
    #pragma warning (disable : 4512) // assignment operator could not be generated
    #pragma warning (disable : 4702) // unreachable code (bugs in Microsoft's STL)
    #pragma warning (disable : 4786) // identifier was truncated
  #endif
#endif

#ifdef __BORLANDC__
  #pragma option -w-8026            // functions with exception specifications are not expanded inline
  #pragma option -w-8027            // function not expanded inline
  #pragma option -w-8030            // Temporary used for 'rhs'
  #pragma option -w-8004            // Assigned value is never used
  #define STRICT 1
#endif

#ifdef __GNUC__
  #pragma GCC diagnostic ignored "-Wmissing-braces"
#endif


#include <cassert>
#include <vector>
#include <algorithm>
#include <string>
#include <map>
#include <winsock2.h>
#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <stdarg.h>
#include <tchar.h>
#ifndef _WIN32_WCE
  #include <shlwapi.h>
  #include <process.h>
  #include <sstream>
#endif

#ifdef _WIN32_WCE
  #include "wxx_wcestddef.h"
#endif

#include "wxx_shared_ptr.h"

// Required for WinCE
#ifndef TLS_OUT_OF_INDEXES
  #define TLS_OUT_OF_INDEXES (static_cast<DWORD_PTR>(-1))
#endif
#ifndef WM_PARENTNOTIFY
  #define WM_PARENTNOTIFY 0x0210
#endif


// For compilers lacking Win64 support
#ifndef  GetWindowLongPtr
  #define GetWindowLongPtr   GetWindowLong
  #define SetWindowLongPtr   SetWindowLong
  #define GWLP_WNDPROC       GWL_WNDPROC
  #define GWLP_HINSTANCE     GWL_HINSTANCE
  #define GWLP_ID            GWL_ID
  #define GWLP_USERDATA      GWL_USERDATA
  #define DWLP_DLGPROC       DWL_DLGPROC
  #define DWLP_MSGRESULT     DWL_MSGRESULT
  #define DWLP_USER          DWL_USER
  #define DWORD_PTR          DWORD
  #define LONG_PTR           LONG
  #define ULONG_PTR          LONG
#endif
#ifndef GetClassLongPtr
  #define GetClassLongPtr    GetClassLong
  #define SetClassLongPtr    SetClassLong
  #define GCLP_HBRBACKGROUND GCL_HBRBACKGROUND
  #define GCLP_HCURSOR       GCL_HCURSOR
  #define GCLP_HICON         GCL_HICON
  #define GCLP_HICONSM       GCL_HICONSM
  #define GCLP_HMODULE       GCL_HMODULE
  #define GCLP_MENUNAME      GCL_MENUNAME
  #define GCLP_WNDPROC       GCL_WNDPROC
#endif


// Automatically include the Win32xx namespace
// define NO_USING_NAMESPACE to skip this step
namespace Win32xx {}
#ifndef NO_USING_NAMESPACE
using namespace Win32xx;
#endif


// Ensure UNICODE and _UNICODE definitions are consistent
#ifdef _UNICODE
  #ifndef UNICODE
    #define UNICODE
  #endif
#endif

#ifdef UNICODE
  #ifndef _UNICODE
    #define _UNICODE
  #endif
#endif

// Define our own MIN and MAX macros
// This avoids inconsistencies with MinGW and other compilers, and
// avoids conflicts between typical min/max macros and std::min/std::max
#define MAX(a,b)        (((a) > (b)) ? (a) : (b))
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))

// Version macro
#define _WIN32XX_VER 0x0860     // Win32++ version 8.6.0

// Define the TRACE Macro
// In debug mode, TRACE send text to the debug/output pane, or an external debugger
// In release mode, TRACE is ignored.
#ifndef TRACE
  #ifdef NDEBUG
    #define TRACE(str) (void(0))
  #else
    #define TRACE(str) Trace(str)
  #endif
#endif

// Define the VERIFY macro
// In debug mode, VERIFY asserts if the expression evaluates to zero
// In release mode, VERIFY evaluates the expression, but doesn't assert.
#ifndef VERIFY
  #ifdef NDEBUG
    #define VERIFY(f) ((void)(f))
  #else
    #define VERIFY(f) assert(f)
  #endif
#endif


namespace Win32xx
{

    ////////////////////////////////////////////////
    // Forward declarations.
    //  These classes are defined later or elsewhere
    class CArchive;
    class CDC;
    class CGDIObject;
    class CMenu;
    class CMenuBar;
    class CWinApp;
    class CWnd;
    class CBitmap;
    class CBrush;
    class CFont;
    class CImageList;
    class CPalette;
    class CPen;
    class CRgn;
    class CDataExchange;
    struct CDC_Data;

    // tString is a TCHAR std::string
    typedef std::basic_string<TCHAR> tString;
  #ifndef _WIN32_WCE
    typedef std::basic_stringstream<TCHAR> tStringStream;
  #endif

    // Some useful smart pointers
    typedef Shared_Ptr<CDC> DCPtr;
    typedef Shared_Ptr<CGDIObject> GDIPtr;
    typedef Shared_Ptr<CMenu> MenuPtr;
    typedef Shared_Ptr<CWnd> WndPtr;
    typedef Shared_Ptr<CBitmap> BitmapPtr;
    typedef Shared_Ptr<CBrush> BrushPtr;
    typedef Shared_Ptr<CFont> FontPtr;
    typedef Shared_Ptr<CImageList> ImageListPtr;
    typedef Shared_Ptr<CPalette> PalettePtr;
    typedef Shared_Ptr<CPen> PenPtr;
    typedef Shared_Ptr<CRgn> RgnPtr;


    // A structure that contains the data members for CGDIObject.
    struct CGDI_Data
    {
        // Constructor
        CGDI_Data() : hGDIObject(0), count(1L), isManagedObject(false) {}

        HGDIOBJ hGDIObject;
        long    count;
        bool    isManagedObject;
    };


    // A structure that contains the data members for CImageList.
    struct CIml_Data
    {
        // Constructor
        CIml_Data() : images(0), isManagedHiml(false), count(1L) {}

        HIMAGELIST  images;
        bool        isManagedHiml;
        long        count;
    };

  #ifndef _WIN32_WCE

    // A structure that contains the data members for CMenu.
    struct CMenu_Data
    {
        // Constructor
        CMenu_Data() : menu(0), isManagedMenu(false), count(1L) {}

        std::vector<MenuPtr> vSubMenus; // A vector of smart pointers to CMenu
        HMENU menu;
        bool isManagedMenu;
        long count;
    };

  #endif

    // The comparison function object used by CWinApp::m_mapHDC
    struct CompareHDC
    {
        bool operator()(const HDC a, const HDC b) const
            {return (reinterpret_cast<DWORD_PTR>(a) < reinterpret_cast<DWORD_PTR>(b));}
    };


    // The comparison function object used by CWinApp::m_mapGDI
    struct CompareGDI
    {
        bool operator()(const HGDIOBJ a, const HGDIOBJ b) const
            {return (reinterpret_cast<DWORD_PTR>(a) < reinterpret_cast<DWORD_PTR>(b));}
    };


    // The comparison function object used by CWinApp::m_mapHIMAGELIST
    struct CompareHIMAGELIST
    {
        bool operator()(const HIMAGELIST a, const HIMAGELIST b) const
            {return (reinterpret_cast<DWORD_PTR>(a) < reinterpret_cast<DWORD_PTR>(b));}
    };


    // The comparison function object used by CWinApp::m_mapHMENU
    struct CompareHMENU
    {
        bool operator()(const HMENU a, const HMENU b) const
            {return (reinterpret_cast<DWORD_PTR>(a) < reinterpret_cast<DWORD_PTR>(b));}
    };


    // The comparison function object used by CWinApp::m_mapHWND
    struct CompareHWND
    {
        bool operator()(const HWND a, const HWND b) const
            {return (reinterpret_cast<DWORD_PTR>(a) < reinterpret_cast<DWORD_PTR>(b));}
    };


    // Used for Thread Local Storage (TLS)
    struct TLSData
    {
        CWnd* pWnd;         // pointer to CWnd object for Window creation
        HWND  mainWnd;      //  handle to the main window for the thread (usually CFrame)
        CMenuBar* pMenuBar; // pointer to CMenuBar object used for the WH_MSGFILTER hook
        HHOOK msgHook;      // WH_MSGFILTER hook for CMenuBar and Modal Dialogs
        long  dlgHooks;     // Number of Dialog MSG hooks

        TLSData() : pWnd(0), mainWnd(0), pMenuBar(0), msgHook(0), dlgHooks(0) {} // Constructor
    };


    /////////////////////////////////////////
    // This class is used for thread synchronisation. A critical section object
    // provides synchronization similar to that provided by a mutex object,
    // except that a critical section can be used only by the threads of a
    // single process. Critical sections are faster and more efficient than mutexes.
    // The CCriticalSection object should be created in the primary thread. Create
    // them as member variables in your CWinApp derrived class.
    class CCriticalSection
    {
    public:
        CCriticalSection() : m_count(0) { ::InitializeCriticalSection(&m_cs); }
        ~CCriticalSection()
        {
            while (m_count > 0)
            {
                Release();
            }

            ::DeleteCriticalSection(&m_cs);
        }


        // Enter the critical section and increment the lock count.
        void Lock()
        {
            ::EnterCriticalSection(&m_cs);
            InterlockedIncrement(&m_count);
        }

        // Leave the critical section and decrement the lock count.
        void Release()
        {
            assert (m_count > 0);
            if (m_count > 0)
            {
                ::LeaveCriticalSection(&m_cs);
                ::InterlockedDecrement(&m_count);
            }
        }

    private:
        CCriticalSection ( const CCriticalSection& );
        CCriticalSection& operator = ( const CCriticalSection& );

        CRITICAL_SECTION m_cs;
        long m_count;
    };


    /////////////////////////////////////////
    // Provides a convenient RAII-style mechanism for owning a CCriticalSection
    // for the duration of a scoped block. Automatically locks the specified
    // CCriticalSection when constructed, and releases the critical section
    // when destroyed.
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


    ///////////////////////////////////
    // The CObject class provides support for Serialization by CArchive.
    class CObject
    {
    public:
        CObject() {}
        virtual ~CObject() {}

        virtual void Serialize(CArchive& ar);

    private:
    };


    // typedef for _beginthreadex's callback function.
    typedef UINT (WINAPI *PFNTHREADPROC)(LPVOID);


    //////////////////////////////////////
    // CWinThread manages a thread. For a GUI thread, it runs the message loop.
    class CWinThread : public CObject
    {
    public:
        CWinThread();
        CWinThread(PFNTHREADPROC pfnThreadProc, LPVOID pParam);
        virtual ~CWinThread();

        // Overridables
        virtual BOOL InitInstance();
        virtual int MessageLoop();
        virtual BOOL OnIdle(LONG lCount);
        virtual BOOL PreTranslateMessage(MSG& msg);

        // Operations
        HANDLE  CreateThread(unsigned initflag = 0, unsigned stack_size = 0, LPSECURITY_ATTRIBUTES pSecurityAttributes = NULL);
        HACCEL  GetAcceleratorTable() const { return m_accel; }
        HWND    GetAcceleratorsWindow() const { return m_accelWnd; }
        HWND    GetMainWnd() const;
        HANDLE  GetThread() const;
        int     GetThreadID() const;
        int     GetThreadPriority() const;
        BOOL    IsRunning() const { return (WaitForSingleObject(m_thread, 0) == WAIT_TIMEOUT); }
        BOOL    PostThreadMessage(UINT message, WPARAM wparam, LPARAM lparam) const;
        DWORD   ResumeThread() const;
        void    SetAccelerators(HACCEL accel, HWND hWndAccel);
        void    SetMainWnd(HWND wnd);
        BOOL    SetThreadPriority(int priority) const;
        DWORD   SuspendThread() const;
        operator HANDLE () const { return GetThread(); }

    private:
        CWinThread(const CWinThread&);              // Disable copy construction
        CWinThread& operator = (const CWinThread&); // Disable assignment operator

        static  UINT WINAPI StaticThreadProc(LPVOID pCThread);

        PFNTHREADPROC m_pfnThreadProc;  // Callback function for worker threads
        LPVOID m_pThreadParams;         // Thread parameter for worker threads
        HANDLE m_thread;                // Handle of this thread
        UINT m_threadID;                // ID of this thread
        DWORD m_threadIDForWinCE;       // ID of this thread (for WinCE only)
        HACCEL m_accel;                 // handle to the accelerator table
        HWND m_accelWnd;                // handle to the window for accelerator keys

    };

    ///////////////////////////////////
    // CWinApp manages the application, and runs the application's message loop.
    // There can only be one instance of CWinApp.
    class CWinApp : public CWinThread
    {
        // Provide these access to CWinApp's private members:
        friend class CDC;
        friend class CGDIObject;
        friend class CImageList;
        friend class CMenu;
        friend class CWnd;
        friend class CPrintDialog;
        friend class CPageSetupDialog;
        friend CWinApp& GetApp();

        typedef Shared_Ptr<TLSData> TLSDataPtr;

    public:
        CWinApp();
        virtual ~CWinApp();

        // Overridables
        virtual BOOL InitInstance();
        virtual int Run();

        // Operations
        CWnd* GetCWndFromMap(HWND wnd);
        HINSTANCE GetInstanceHandle() const { return m_instance; }
        HINSTANCE GetResourceHandle() const { return (m_resource ? m_resource : m_instance); }
        TLSData* GetTlsData() const;
        HCURSOR LoadCursor(LPCTSTR pResourceName) const;
        HCURSOR LoadCursor(int cursorID) const;
        HCURSOR LoadStandardCursor(LPCTSTR pCursorName) const;
        HICON   LoadIcon(LPCTSTR pResourceName) const;
        HICON   LoadIcon(int nIDIcon) const;
        HICON   LoadStandardIcon(LPCTSTR pIconName) const;
        HANDLE  LoadImage(LPCTSTR pResourceName, UINT type, int cx, int  cy, UINT flags = LR_DEFAULTCOLOR) const;
        HANDLE  LoadImage(int imageID, UINT type, int cx, int cy, UINT flags = LR_DEFAULTCOLOR) const;
        HCURSOR SetCursor(HCURSOR cursor) const;
        void    SetResourceHandle(HINSTANCE resource);
        TLSData* SetTlsData();

    private:
        CWinApp(const CWinApp&);                // Disable copy construction
        CWinApp& operator = (const CWinApp&);   // Disable assignment operator

        void AddCDCData(HDC dc, CDC_Data* pData);
        void AddCGDIData(HGDIOBJ gdi, CGDI_Data* pData);
        void AddCImlData(HIMAGELIST images, CIml_Data* pData);
        CDC_Data* GetCDCData(HDC dc);
        CGDI_Data* GetCGDIData(HGDIOBJ object);
        CIml_Data* GetCImlData(HIMAGELIST images);
        void GlobalFreeAll(HGLOBAL buffer);
        void SetCallback();
        static CWinApp* SetnGetThis(CWinApp* pThis = 0, bool reset = false);
        void UpdateDefaultPrinter();

        std::map<HDC, CDC_Data*, CompareHDC> m_mapCDCData;
        std::map<HGDIOBJ, CGDI_Data*, CompareGDI> m_mapCGDIData;
        std::map<HIMAGELIST, CIml_Data*, CompareHIMAGELIST> m_mapCImlData;
        std::map<HWND, CWnd*, CompareHWND> m_mapHWND;       // maps window handles to CWnd objects
        std::vector<TLSDataPtr> m_allTLSData;     // vector of TLSData smart pointers, one for each thread
        CCriticalSection m_appLock;   // thread synchronisation for CWinApp and TLS.
        CCriticalSection m_gdiLock;   // thread synchronisation for m_mapCDCData and m_mapCGDIData.
        CCriticalSection m_wndLock;   // thread synchronisation for m_mapHWND etc.
        CCriticalSection m_printLock; // thread synchronisation for printing.
        HINSTANCE m_instance;          // handle to the application's instance
        HINSTANCE m_resource;          // handle to the application's resources
        DWORD m_tlsData;                // Thread Local Storage data
        WNDPROC m_callback;             // callback address of CWnd::StaticWndowProc
        HGLOBAL m_devMode;             // Used by CPrintDialog and CPageSetupDialog
        HGLOBAL m_devNames;            // Used by CPrintDialog and CPageSetupDialog

#ifndef _WIN32_WCE
        void AddCMenuData(HMENU menu, CMenu_Data* pData);
        CMenu_Data* GetCMenuData(HMENU menu);
        std::map<HMENU, CMenu_Data*, CompareHMENU> m_mapCMenuData;
#endif

    };

    ////////////////////////////////////////
    // Global Functions
    //

    // Returns a reference to the CWinApp derived class.
    inline CWinApp& GetApp()
    {
        CWinApp* pApp = CWinApp::SetnGetThis();
        assert(pApp);
        return *pApp;
    }


  #ifndef _WIN32_WCE

    // Retrieves the window version
        // Return values and window versions:
        //  1400     Windows 95
        //  1410     Windows 98
        //  1490     Windows ME
        //  2400     Windows NT
        //  2500     Windows 2000
        //  2501     Windows XP
        //  2502     Windows Server 2003
        //  2600     Windows Vista and Windows Server 2008
        //  2601     Windows 7 and Windows Server 2008 r2
        //  2602     Windows 8 and Windows Server 2012
        //  2603     Windows 8.1 and Windows Server 2012 r2
        // Note: For windows 8.1 and above, the value returned is also affected by the embedded manifest
        //       Applications not manifested for Windows 8.1 or Windows 10 will return the Windows 8 OS (2602).
    inline int GetWinVersion()
    {
#ifdef _MSC_VER
  #pragma warning ( push )
  #pragma warning ( disable : 4996 )
#endif // _MSC_VER

        DWORD version = GetVersion();

#ifdef _MSC_VER
  #pragma warning ( pop )
#endif // _MSC_VER

        int platform = (version < 0x80000000)? 2:1;
        int majorVer = LOBYTE(LOWORD(version));
        int minorVer = HIBYTE(LOWORD(version));

        int result =  1000*platform + 100*majorVer + minorVer;
        return result;
    }


    // Retrieves the version of common control dll used.
    // return values and DLL versions
    // 400  dll ver 4.00    Windows 95/Windows NT 4.0
    // 470  dll ver 4.70    Internet Explorer 3.x
    // 471  dll ver 4.71    Internet Explorer 4.0
    // 472  dll ver 4.72    Internet Explorer 4.01 and Windows 98
    // 580  dll ver 5.80    Internet Explorer 5
    // 581  dll ver 5.81    Windows 2000 and Windows ME
    // 582  dll ver 5.82    Windows XP, Vista, Windows 7 etc. without XP themes
    // 600  dll ver 6.00    Windows XP with XP themes
    // 610  dll ver 6.10    Windows Vista with XP themes
    // 616  dll ver 6.16    Windows Vista SP1 or Windows 7 with XP themes
    inline int GetComCtlVersion()
    {
        // Load the Common Controls DLL
        HMODULE comCtl = ::LoadLibrary(_T("COMCTL32.DLL"));
        if (comCtl == 0)
            return 0;

        int comCtlVer = 400;

        if (::GetProcAddress(comCtl, "InitCommonControlsEx"))
        {
            // InitCommonControlsEx is unique to 4.7 and later
            comCtlVer = 470;

            if (::GetProcAddress(comCtl, "DllGetVersion"))
            {
                typedef HRESULT CALLBACK DLLGETVERSION(DLLVERSIONINFO*);
                DLLGETVERSION* pfnDLLGetVersion = NULL;

                pfnDLLGetVersion = reinterpret_cast<DLLGETVERSION*>(::GetProcAddress(comCtl, "DllGetVersion"));
                if(pfnDLLGetVersion)
                {
                    DLLVERSIONINFO dvi;
                    dvi.cbSize = sizeof dvi;
                    if(NOERROR == pfnDLLGetVersion(&dvi))
                    {
                        DWORD verMajor = dvi.dwMajorVersion;
                        DWORD verMinor = dvi.dwMinorVersion;
                        comCtlVer = 100 * verMajor + verMinor;
                    }
                }
            }
            else if (::GetProcAddress(comCtl, "InitializeFlatSB"))
                comCtlVer = 471;    // InitializeFlatSB is unique to version 4.71
        }

        ::FreeLibrary(comCtl);

        return comCtlVer;
    }
  #endif


    // The following functions perform string copies. The size of the dst buffer
    // is specified, much like strcpy_s. The dst buffer is always null terminated.
    // Null or zero arguments cause an assert.

    // Copies an ANSI string from src to dst. 
    inline void StrCopyA(char* dst, const char* src, size_t dst_size)
    {
        assert(dst != 0);
        assert(src != 0);
        assert(dst_size != 0);

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

    // Copies a wide string from src to dst.
    inline void StrCopyW(wchar_t* dst, const wchar_t* src, size_t dst_size)
    {
        assert(dst != 0);
        assert(src != 0);
        assert(dst_size != 0);

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

#endif // _WIN32XX_APPCORE0_H_
