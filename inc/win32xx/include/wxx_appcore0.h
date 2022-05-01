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



#ifndef _WIN32XX_APPCORE0_H_
#define _WIN32XX_APPCORE0_H_


///////////////////////////////////////////////////////////////////////////
// wxx_appcore0.h
// This file contains the declarations of the CWinApp class.
// This class is used start Win32++ and run the message loop. You should
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
    struct MenuItemData;
    struct TLSData;

    // Define the maximum size for TCHAR strings
    const int WXX_MAX_STRING_SIZE = 255;

    // tString is a TCHAR std::string
    typedef std::basic_string<TCHAR> tString;
    typedef std::basic_stringstream<TCHAR> tStringStream;

    // Some useful smart pointers
    // Note: Modern C++ compilers can use these typedefs instead.
    // typedef std::shared_ptr<CBitmap> BitmapPtr;
    // typedef std::shared_ptr<CBrush> BrushPtr;
    // typedef std::shared_ptr<CDC> DCPtr;
    // typedef std::shared_ptr<CDocker> DockPtr;
    // typedef std::shared_ptr<CFont> FontPtr;
    // typedef std::shared_ptr<CGDIObject> GDIPtr;
    // typedef std::shared_ptr<CImageList> ImageListPtr;
    // typedef std::shared_ptr<CMDIChild> MDIChildPtr;
    // typedef std::shared_ptr<CMenu> MenuPtr;
    // typedef std::shared_ptr<MenuItemData> ItemDataPtr;
    // typedef std::shared_ptr<CPalette> PalettePtr;
    // typedef std::shared_ptr<CPen> PenPtr;
    // typedef std::shared_ptr<CPropertyPage> PropertyPagePtr;
    // typedef std::shared_ptr<CRgn> RgnPtr;
    // typedef std::shared_ptr<TLSData> TLSDataPtr;
    // typedef std::shared_ptr<CWinThread> WinThreadPtr;
    // typedef std::shared_ptr<CWorkThread> WorkThreadPtr;
    // typedef std::shared_ptr<CWnd> WndPtr;
    typedef Shared_Ptr<CBitmap> BitmapPtr;
    typedef Shared_Ptr<CBrush> BrushPtr;
    typedef Shared_Ptr<CDC> DCPtr;
    typedef Shared_Ptr<CDocker> DockPtr;
    typedef Shared_Ptr<CFont> FontPtr;
    typedef Shared_Ptr<CGDIObject> GDIPtr;
    typedef Shared_Ptr<CImageList> ImageListPtr;
    typedef Shared_Ptr<CMDIChild> MDIChildPtr;
    typedef Shared_Ptr<CMenu> MenuPtr;
    typedef Shared_Ptr<MenuItemData> ItemDataPtr;
    typedef Shared_Ptr<CPalette> PalettePtr;
    typedef Shared_Ptr<CPen> PenPtr;
    typedef Shared_Ptr<CPropertyPage> PropertyPagePtr;
    typedef Shared_Ptr<CRgn> RgnPtr;
    typedef Shared_Ptr<TLSData> TLSDataPtr;
    typedef Shared_Ptr<CWinThread> WinThreadPtr;
    typedef Shared_Ptr<CWorkThread> WorkThreadPtr;
    typedef Shared_Ptr<CWnd> WndPtr;


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
        CWnd* pWnd;         // Pointer to CWnd object for window creation
        HWND  mainWnd;      // Handle to the main window for the thread (usually CFrame)
        CMenuBar* pMenuBar; // Pointer to CMenuBar object used for the WH_MSGFILTER hook
        HHOOK msgHook;      // WH_MSGFILTER hook for CMenuBar and modal dialogs
        long  dlgHooks;     // Number of dialog MSG hooks

        TLSData() : pWnd(0), mainWnd(0), pMenuBar(0), msgHook(0), dlgHooks(0) {} // Constructor
    };


    ///////////////////////////////////////////////////////////////
    // CWinApp manages the application. Its constructor initializes
    // the Win32++ framework. The Run function calls InitInstance,
    // and starts the message loop on the main thread.
    // There can only be one instance of CWinApp.
    class CWinApp : public CMessagePump
    {
        // Provide these access to CWinApp's private members:
        friend class CDC;
        friend class CDialog;
        friend class CGDIObject;
        friend class CImageList;
        friend class CMenu;
        friend class CPageSetupDialog;
        friend class CPrintDialog;
        friend class CPrintDialogEx;
        friend class CPropertyPage;
        friend class CWinThread;
        friend class CWnd;
        friend CWinApp* GetApp();

    public:
        CWinApp();
        virtual ~CWinApp();

        // Operations
        CWnd* GetCWndFromMap(HWND wnd);
        HINSTANCE GetInstanceHandle() const { return m_instance; }
        HWND      GetMainWnd() const;
        HINSTANCE GetResourceHandle() const { return (m_resource ? m_resource : m_instance); }
        TLSData*  GetTlsData() const;
        HCURSOR   LoadCursor(LPCTSTR resourceName) const;
        HCURSOR   LoadCursor(int cursorID) const;
        HICON     LoadIcon(LPCTSTR resourceName) const;
        HICON     LoadIcon(int iconID) const;
        HANDLE    LoadImage(LPCTSTR resourceName, UINT type, int cx, int  cy, UINT flags = LR_DEFAULTCOLOR) const;
        HANDLE    LoadImage(int imageID, UINT type, int cx, int cy, UINT flags = LR_DEFAULTCOLOR) const;
        HCURSOR   LoadStandardCursor(LPCTSTR cursorName) const;
        HICON     LoadStandardIcon(LPCTSTR iconName) const;
        HCURSOR   SetCursor(HCURSOR cursor) const;
        void      SetMainWnd(HWND wnd) const;
        void      SetResourceHandle(HINSTANCE resource);

    private:
        CWinApp(const CWinApp&);                // Disable copy construction
        CWinApp& operator = (const CWinApp&);   // Disable assignment operator

        void AddCDCData(HDC dc, CDC_Data* pData);
        void AddCGDIData(HGDIOBJ gdi, CGDI_Data* pData);
        void AddCImlData(HIMAGELIST images, CIml_Data* pData);
        void AddCMenuData(HMENU menu, CMenu_Data* pData);
        CDC_Data*   GetCDCData(HDC dc);
        CGDI_Data*  GetCGDIData(HGDIOBJ object);
        CIml_Data*  GetCImlData(HIMAGELIST images);
        CMenu_Data* GetCMenuData(HMENU menu);
        void SetCallback();
        void SetTlsData();
        void UpdateDefaultPrinter();

        static CWinApp* SetnGetThis(CWinApp* pThis = 0, bool reset = false);

        std::map<HDC, CDC_Data*, CompareHDC> m_mapCDCData;
        std::map<HGDIOBJ, CGDI_Data*, CompareGDI> m_mapCGDIData;
        std::map<HIMAGELIST, CIml_Data*, CompareHIMAGELIST> m_mapCImlData;
        std::map<HMENU, CMenu_Data*, CompareHMENU> m_mapCMenuData;
        std::map<HWND, CWnd*, CompareHWND> m_mapHWND;       // maps window handles to CWnd objects
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
        // Messages used for exceptions.
        virtual CString MsgAppThread() const;
        virtual CString MsgArReadFail() const;
        virtual CString MsgArNotCStringA() const;
        virtual CString MsgArNotCStringW() const;
        virtual CString MsgCriticalSection() const;
        virtual CString MsgMtxEvent() const;
        virtual CString MsgMtxMutex() const;
        virtual CString MsgMtxSemaphore() const;

        virtual CString MsgWndCreate() const;
        virtual CString MsgWndDialog() const;
        virtual CString MsgWndGlobalLock() const;
        virtual CString MsgWndPropertSheet() const;
        virtual CString MsgSocWSAStartup() const;
        virtual CString MsgSocWS2Dll() const;
        virtual CString MsgIPControl() const;
        virtual CString MsgRichEditDll() const;
        virtual CString MsgTaskDialog() const;

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

        virtual CString MsgGdiDC() const;
        virtual CString MsgGdiIC() const;
        virtual CString MsgGdiBitmap() const;
        virtual CString MsgGdiBrush() const;
        virtual CString MsgGdiFont() const;
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

        // DDX anomaly prompting messages
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
