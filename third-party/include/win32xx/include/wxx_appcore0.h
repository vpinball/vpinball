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



#ifndef _WIN32XX_APPCORE0_H_
#define _WIN32XX_APPCORE0_H_


///////////////////////////////////////////////////////////////////////////
// wxx_appcore0.h
// This file contains the declarations of CGlobalLock and CWinApp.
// CWinApp is used start Win32++ and run the message loop. You should
// should inherit from this class to start Win32++ in your own application.


#include "wxx_setup.h"
#include "wxx_criticalsection.h"
#include "wxx_hglobal.h"
#include "wxx_messagepump0.h"

namespace Win32xx
{

    ////////////////////////////////////////////////
    // Forward declarations.
    //  These classes are defined later or elsewhere
    class CArchive;
    class CBitmap;
    class CBrush;
    class CClientDC;
    class CClientDCEx;
    class CDataExchange;
    class CDialog;
    class CDC;
    class CDocker;
    class CFont;
    class CGDIObject;
    class CImageList;
    class CMDIChild;
    class CMemDC;
    class CMenu;
    class CMenuBar;
    class CPaintDC;
    class CPalette;
    class CPen;
    class CPropertyPage;
    class CRgn;
    class CString;
    class CWinApp;
    class CWinThread;
    class CWorkThread;
    class CWindowDC;
    class CWnd;
    struct CDC_Data;
    struct EnhMetaFileData;
    struct MenuItemData;
    struct MetaFileData;
    struct TLSData;

    // Define the maximum size for TCHAR strings
    constexpr int WXX_MAX_STRING_SIZE = 255;

    // Some useful smart pointer aliases
    using DockPtr = std::unique_ptr<CDocker>;
    using EnhMetaDataPtr = std::shared_ptr<EnhMetaFileData>;
    using MDIChildPtr = std::unique_ptr<CMDIChild>;
    using MenuItemDataPtr = std::unique_ptr<MenuItemData>;
    using MetaDataPtr = std::shared_ptr<MetaFileData>;
    using PropertyPagePtr = std::unique_ptr<CPropertyPage>;
    using TLSDataPtr = std::unique_ptr<TLSData>;
    using WinThreadPtr = std::unique_ptr<CWinThread>;
    using WorkThreadPtr = std::unique_ptr<CWorkThread>;
    using WndPtr = std::unique_ptr<CWnd>;

    // A structure that contains the data members for CGDIObject.
    struct CGDI_Data
    {
        // Constructor
        CGDI_Data() : hGDIObject(nullptr), isManagedObject(false) {}

        HGDIOBJ hGDIObject;
        bool    isManagedObject;
    };

    // A structure that contains the data members for CImageList.
    struct CIml_Data
    {
        // Constructor
        CIml_Data() : images(nullptr), isManagedHiml(false) {}

        HIMAGELIST  images;
        bool        isManagedHiml;
    };

    // A structure that contains the data members for CMenu.
    struct CMenu_Data
    {
        // Constructor
        CMenu_Data() : menu(nullptr), isManagedMenu(false) {}

        HMENU menu;
        bool isManagedMenu;
    };

    // Used for Thread Local Storage (TLS)
    struct TLSData
    {
        CWnd* pWnd;         // Pointer to CWnd object for window creation
        HWND  mainWnd;      // Handle to the main window for the thread (usually CFrame)
        CMenuBar* pMenuBar; // Pointer to the CMenuBar object with the WH_MSGFILTER hook

        TLSData() : pWnd(nullptr), mainWnd(nullptr), pMenuBar(nullptr) {} // Constructor
    };


    ///////////////////////////////////////////////////////////////////
    // Acknowledgement:
    //  CGlobalLock is based on code by Rob Caldecott
    //  See:  https://www.codeproject.com/Articles/16692/A-Template-Wrapper-for-GlobalLock
    //
    ///////////////////////////////////////////////////////////////////


    ///////////////////////////////////////////////////////////////////
    // CGlobaLock is a class template used to provide a self unlocking
    // object to global memory. It is used to provide convenient access
    // to the memory provided by hDevMode and hDevNames handles.
    // The framework uses this class to eliminate the need to manually
    // lock or unlock global memory.
    // This class is typically used with a CHGlobal object but can also
    // be used with a raw global memory handle.
    // CDevMode and CDevNames are typedefs of this class.
    template <typename T>
    class CGlobalLock
    {
    public:
        // Constructors and Destructors
        CGlobalLock() : m_h(nullptr), m_p(nullptr) {}
        CGlobalLock(HANDLE h) : m_h(h) { Lock(); }
        CGlobalLock(const CGlobalLock& rhs);
        ~CGlobalLock() { Unlock(); }

        T* Get() const { return m_p; }                          // Returns the pointer
        LPCTSTR c_str() const;                                  // Returns the LPCTSTR (for DEVNAMES only)
        LPTSTR  GetString() const;                              // Returns the LPTSTR (for DEVNAMES only)
        CString GetDeviceName() const;                          // Returns the printer name (for DEVNAMES only)
        CString GetDriverName() const;                          // Returns the printer driver (for DEVNAMES only)
        CString GetPortName() const;                            // Returns the printer port (for DEVNAMES only)
        bool    IsDefaultPrinter() const;                       // Returns true if this is the default printer (for DEVNAMES only)

        // operator overloads
        operator T* () const { return m_p; }                    // Conversion operator to pointer
        T* operator->() const { assert(m_p); return m_p; }      // Pointer operator
        T& operator*() const { assert(m_p); return *m_p; }      // Dereference operator
        CGlobalLock& operator= (const CGlobalLock& rhs);        // Assignment operator
        CGlobalLock& operator=(HANDLE h);                       // Assignment operator

    private:
        void Lock();
        void Unlock();

        HANDLE m_h;     // The handle to lock/unlock
        T* m_p;         // Pointer returned by ::GlobalLock
    };


    ///////////////////////////////////////////////////////////////
    // CWinApp manages the application. Its constructor initializes
    // the Win32++ framework. The Run function calls InitInstance,
    // and starts the message loop on the main thread.
    // There can only be one instance of CWinApp.
    class CWinApp : public CMessagePump
    {
        // These provide access to CWinApp's private members:
        friend class CDC;
        friend class CDialog;
        friend class CEnhMetaFile;
        friend class CGDIObject;
        friend class CImageList;
        friend class CMenu;
        friend class CMetaFile;
        friend class CPropertyPage;
        friend class CWinThread;
        friend class CWnd;
        friend CWinApp* GetApp();

    public:
        CWinApp();
        virtual ~CWinApp() override;

        // Operations
        CWnd* GetCWndFromMap(HWND wnd);
        const CHGlobal& GetHDevMode() const { return m_devMode; }
        const CHGlobal& GetHDevNames() const { return m_devNames; }
        HINSTANCE GetInstanceHandle() const { return m_instance; }
        HWND      GetMainWnd() const;
        HINSTANCE GetResourceHandle() const { return (m_resource ? m_resource : m_instance); }
        TLSData*  GetTlsData() const;
        HCURSOR   LoadCursor(LPCTSTR resourceName) const;
        HCURSOR   LoadCursor(UINT cursorID) const;
        HICON     LoadIcon(LPCTSTR resourceName) const;
        HICON     LoadIcon(UINT iconID) const;
        HANDLE    LoadImage(LPCTSTR resourceName, UINT type, int cx, int  cy, UINT flags = LR_DEFAULTCOLOR) const;
        HANDLE    LoadImage(UINT imageID, UINT type, int cx, int cy, UINT flags = LR_DEFAULTCOLOR) const;
        HCURSOR   LoadStandardCursor(LPCTSTR cursorName) const;
        HICON     LoadStandardIcon(LPCTSTR iconName) const;
        void      ResetPrinterMemory();
        HCURSOR   SetCursor(HCURSOR cursor) const;
        void      SetMainWnd(HWND wnd) const;
        void      SetResourceHandle(HINSTANCE resource);
        void      UpdateDefaultPrinter();
        void      UpdatePrinterMemory(HGLOBAL hDevMode, HGLOBAL hDevNames);

    private:
        CWinApp(const CWinApp&) = delete;
        CWinApp& operator=(const CWinApp&) = delete;

        void AddCDCData(HDC dc, std::weak_ptr<CDC_Data> pData);
        void AddCGDIData(HGDIOBJ gdi, std::weak_ptr<CGDI_Data> pData);
        void AddCImlData(HIMAGELIST images, std::weak_ptr<CIml_Data> pData);
        void AddCMenuData(HMENU menu, std::weak_ptr<CMenu_Data> pData);
        std::weak_ptr<CDC_Data>  GetCDCData(HDC dc);
        std::weak_ptr<CGDI_Data> GetCGDIData(HGDIOBJ object);
        std::weak_ptr<CIml_Data> GetCImlData(HIMAGELIST images);
        std::weak_ptr<CMenu_Data> GetCMenuData(HMENU menu);
        void SetCallback();
        void SetTlsData();

        static CWinApp* SetnGetThis(CWinApp* pThis = nullptr, bool reset = false);

        std::map<HDC, std::weak_ptr<CDC_Data>> m_mapCDCData;
        std::map<HGDIOBJ, std::weak_ptr<CGDI_Data>> m_mapCGDIData;
        std::map<HIMAGELIST, std::weak_ptr<CIml_Data>> m_mapCImlData;
        std::map<HMENU, std::weak_ptr<CMenu_Data>> m_mapCMenuData;
        std::map<HWND, CWnd*> m_mapHWND;       // maps window handles to CWnd objects
        std::vector<TLSDataPtr> m_allTLSData;     // vector of TLSData smart pointers, one for each thread
        CCriticalSection m_appLock;   // thread synchronization for CWinApp and TLS.
        CCriticalSection m_gdiLock;   // thread synchronization for m_mapCDCData and m_mapCGDIData.
        CCriticalSection m_wndLock;   // thread synchronization for m_mapHWND etc.
        CCriticalSection m_printLock; // thread synchronization for printing.
        HINSTANCE m_instance;         // handle to the application's instance
        HINSTANCE m_resource;         // handle to the application's resources
        DWORD m_tlsData;              // Thread Local Storage data
        WNDPROC m_callback;           // callback address of CWnd::StaticWndowProc
        CHGlobal m_devMode;           // Used by CPrintDialog and CPageSetupDialog
        CHGlobal m_devNames;          // Used by CPrintDialog and CPageSetupDialog

    public:
        // Message strings used for exceptions.
        virtual CString MsgAppThread() const;
        virtual CString MsgArReadFail() const;
        virtual CString MsgArNotCStringA() const;
        virtual CString MsgArNotCStringW() const;
        virtual CString MsgCriticalSection() const;
        virtual CString MsgMtxEvent() const;
        virtual CString MsgMtxMutex() const;
        virtual CString MsgMtxSemaphore() const;

        // Message strings used for windows.
        virtual CString MsgWndCreate() const;
        virtual CString MsgWndDialog() const;
        virtual CString MsgWndGlobalLock() const;
        virtual CString MsgWndPropertSheet() const;
        virtual CString MsgSocWSAStartup() const;
        virtual CString MsgSocWS2Dll() const;
        virtual CString MsgIPControl() const;
        virtual CString MsgRichEditDll() const;
        virtual CString MsgTaskDialog() const;

        // Message strings used for files.
        virtual CString MsgFileClose() const;
        virtual CString MsgFileFlush() const;
        virtual CString MsgFileLock() const;
        virtual CString MsgFileOpen() const;
        virtual CString MsgFileRead() const;
        virtual CString MsgFileRename() const;
        virtual CString MsgFileRemove() const;
        virtual CString MsgFileLength() const;
        virtual CString MsgFileUnlock() const;
        virtual CString MsgFileWrite() const;

        // Message strings used for GDI.
        virtual CString MsgGdiDC() const;
        virtual CString MsgGdiIC() const;
        virtual CString MsgGdiBitmap() const;
        virtual CString MsgGdiBrush() const;
        virtual CString MsgGdiFont() const;
        virtual CString MsgGdiImageList() const;
        virtual CString MsgGdiPalette() const;
        virtual CString MsgGdiPen() const;
        virtual CString MsgGdiRegion() const;
        virtual CString MsgGdiGetDC() const;
        virtual CString MsgGdiGetDCEx() const;
        virtual CString MsgGdiSelObject() const;
        virtual CString MsgGdiGetWinDC() const;
        virtual CString MsgGdiBeginPaint() const;

        virtual CString MsgImageList() const;
        virtual CString MsgMenu() const;
        virtual CString MsgPrintFound() const;

        // DDX anomaly prompting message strings
        virtual CString MsgDDX_Byte() const;
        virtual CString MsgDDX_Int() const;
        virtual CString MsgDDX_Long() const;
        virtual CString MsgDDX_Short() const;
        virtual CString MsgDDX_Real() const;
        virtual CString MsgDDX_UINT() const;
        virtual CString MsgDDX_ULONG() const;

        // DDV formats and prompts
        virtual CString MsgDDV_IntRange() const;
        virtual CString MsgDDV_UINTRange() const;
        virtual CString MsgDDV_RealRange() const;
        virtual CString MsgDDV_StringSize() const;
    };

    // Returns a pointer to the CWinApp derived class.
    inline CWinApp* GetApp()
    {
        CWinApp* pApp = CWinApp::SetnGetThis();
        assert(pApp);  // This assert fails if Win32++ isn't started.
        return pApp;
    }

} // namespace Win32xx


#endif // _WIN32XX_APPCORE0_H_
