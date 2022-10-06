// Win32++   Version 9.1
// Release Date: 26th September 2022
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


#ifndef _WIN32XX_APPCORE_H_
#define _WIN32XX_APPCORE_H_


///////////////////////////////////////////////////////
// wxx_appcore.h
// This file contains the definitions of the CWinApp class.
// This class is used start Win32++ and run the message loop. You
// should inherit from this class to start Win32++ in your own
// application.


#include "wxx_appcore0.h"
#include "wxx_textconv.h"
#include "wxx_wincore0.h"
#include "wxx_exception.h"
#include "wxx_cstring.h"
#include "wxx_messagepump.h"


namespace Win32xx
{

    ////////////////////////////////////
    // Definitions for the CWinApp class
    //

    // Constructor
    inline CWinApp::CWinApp() : m_callback(NULL)
    {
        static CCriticalSection cs;
        CThreadLock appLock(cs);

        if (SetnGetThis() == 0)
        {
            m_tlsData = ::TlsAlloc();
            if (m_tlsData != TLS_OUT_OF_INDEXES)
            {
                SetnGetThis(this);

                // Set the instance handle
                MEMORY_BASIC_INFORMATION mbi;
                ZeroMemory(&mbi, sizeof(mbi));
                static int Address = 0;
                VirtualQuery(&Address, &mbi, sizeof(mbi));
                assert(mbi.AllocationBase);
                m_instance = (HINSTANCE)mbi.AllocationBase;

                m_resource = m_instance;
                SetTlsData();
                SetCallback();
                LoadCommonControls();

                // Initialize the COM library.
                // Note: The ribbon UI requires the COINIT_APARTMENTTHREADED
                //       concurrency model. OleInitialize calls CoInitializeEx
                //       with COINIT_APARTMENTTHREADED, and provides support
                //       for other OLE functionality.
                VERIFY(SUCCEEDED(OleInitialize(NULL)));
            }
            else
            {
                // Should not get here.
                // All TLS indexes are already allocated by this app.
                // At least 64 TLS indexes per process are allowed.
                // Win32++ requires only one TLS index.
                TRACE("\n*** Error: Unable to allocate Thread Local Storage. ***");
                TRACE("\n*** Error: Win32++ hasn't been started. ***\n\n");
            }
        }
        else
        {
            // Should not get here.
            TRACE("\n*** Warning: Win32++ has already been started. ***");
            TRACE("\n*** Warning: This instance of CWinApp has been ignored. ***\n\n");
        }
    }

    // Destructor
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
        if (m_resource != m_instance)
            ::FreeLibrary(m_resource);

        OleUninitialize();
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

    // Adds a HMENU and CMenu_Data* to the map.
    inline void CWinApp::AddCMenuData(HMENU menu, CMenu_Data* pData)
    {
        CThreadLock mapLock(m_wndLock);
        m_mapCMenuData.insert(std::make_pair(menu, pData));
    }

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

    // Retrieves a handle to the main window for this thread.
    // Note: CFrame set's itself as the main window of its thread
    inline HWND CWinApp::GetMainWnd() const
    {
        TLSData* pTLSData = GetApp()->GetTlsData();

        // Will assert if the thread doesn't have TLSData assigned.
        // TLSData is assigned when the first window in the thread is created.
        assert(pTLSData);

        return pTLSData ? pTLSData->mainWnd : 0;
    }

    // Retrieves the pointer to the Thread Local Storage data for the current thread.
    inline TLSData* CWinApp::GetTlsData() const
    {
        return static_cast<TLSData*>(TlsGetValue(m_tlsData));
    }

    // Loads the cursor resource from the resource script (resource.rc)
    // Refer to LoadCursor in the Windows API documentation for more information.
    inline HCURSOR CWinApp::LoadCursor(LPCTSTR resourceName) const
    {
        return ::LoadCursor(GetResourceHandle(), resourceName);
    }

    // Loads the cursor resource from the resource script (resource.rc)
    // Refer to LoadCursor in the Windows API documentation for more information.
    inline HCURSOR CWinApp::LoadCursor(UINT cursorID) const
    {
        return ::LoadCursor(GetResourceHandle(), MAKEINTRESOURCE (cursorID));
    }

    // Returns the handle of a standard cursor. Standard cursors include:
    // IDC_APPSTARTING, IDC_ARROW, IDC_CROSS, IDC_HAND, IDC_HELP, IDC_IBEAM, IDC_NO, IDC_SIZEALL,
    // IDC_SIZENESW, IDC_SIZENS, IDC_SIZENWSE, IDC_SIZEWE, IDC_UPARROW, IDC_WAIT
    // Refer to LoadCursor in the Windows API documentation for more information.
    inline HCURSOR CWinApp::LoadStandardCursor(LPCTSTR cursorName) const
    {
        return ::LoadCursor(0, cursorName);
    }

    // Loads the icon resource whose size conforms to the SM_CXICON and SM_CYICON system metric values.
    // For other icon sizes, use the LoadImage windows API function.
    // Refer to LoadIcon in the Windows API documentation for more information.
    inline HICON CWinApp::LoadIcon(LPCTSTR resourceName) const
    {
        return ::LoadIcon(GetResourceHandle(), resourceName);
    }

    // Loads the icon resource whose size conforms to the SM_CXICON and SM_CYICON system metric values.
    // Refer to LoadIcon in the Windows API documentation for more information.
    inline HICON CWinApp::LoadIcon(UINT iconID) const
    {
        return ::LoadIcon(GetResourceHandle(), MAKEINTRESOURCE (iconID));
    }

    // Returns the handle of a standard Icon. Standard Icons include:
    // IDI_APPLICATION, IDI_ASTERISK, IDI_ERROR, IDI_EXCLAMATION,
    // IDI_HAND, IDI_INFORMATION, IDI_QUESTION, IDI_WARNING
    // Refer to LoadIcon in the Windows API documentation for more information.
    inline HICON CWinApp::LoadStandardIcon(LPCTSTR iconName) const
    {
        return ::LoadIcon(0, iconName);
    }

    // Loads an icon, cursor, animated cursor, or bitmap image.
    // uType is the image type. It can be IMAGE_BITMAP, IMAGE_CURSOR or IMAGE_ICON.
    // cx and cy are the desired width and height in pixels.
    // flags can be LR_DEFAULTCOLOR, LR_CREATEDIBSECTION, LR_DEFAULTSIZE, LR_LOADFROMFILE,
    // LR_LOADMAP3DCOLORS, R_LOADTRANSPARENT, LR_MONOCHROME, LR_SHARED, LR_VGACOLOR.
    // Ideally the image should be destroyed unless it is loaded with LR_SHARED.
    // Refer to LoadImage in the Windows API documentation for more information.
    inline HANDLE CWinApp::LoadImage(LPCTSTR resourceName, UINT type, int cx, int cy, UINT flags) const
    {
        return ::LoadImage(GetResourceHandle(), resourceName, type, cx, cy, flags);
    }

    // Loads an icon, cursor, animated cursor, or bitmap.
    // uType is the image type. It can be IMAGE_BITMAP, IMAGE_CURSOR or IMAGE_ICON.
    // cx and cy are the desired width and height in pixels.
    // flags can be LR_DEFAULTCOLOR, LR_CREATEDIBSECTION, LR_DEFAULTSIZE, LR_LOADFROMFILE,
    // LR_LOADMAP3DCOLORS, R_LOADTRANSPARENT, LR_MONOCHROME, LR_SHARED, LR_VGACOLOR.
    // Ideally the image should be destroyed unless it is loaded with LR_SHARED.
    // Refer to LoadImage in the Windows API documentation for more information.
    inline HANDLE CWinApp::LoadImage(UINT imageID, UINT type, int cx, int cy, UINT flags) const
    {
        return ::LoadImage(GetResourceHandle(), MAKEINTRESOURCE (imageID), type, cx, cy, flags);
    }

    // Registers a temporary window class so we can get the callback
    // address of CWnd::StaticWindowProc.
    inline void CWinApp::SetCallback()
    {
        WNDCLASS defaultWC;
        ZeroMemory(&defaultWC, sizeof(defaultWC));

        LPCTSTR className    = _T("Win32++ Temporary Window Class");
        defaultWC.hInstance     = GetInstanceHandle();
        defaultWC.lpfnWndProc   = CWnd::StaticWindowProc;
        defaultWC.lpszClassName = className;

        VERIFY(::RegisterClass(&defaultWC));

        // Retrieve the class information
        ZeroMemory(&defaultWC, sizeof(defaultWC));
        VERIFY(::GetClassInfo(GetInstanceHandle(), className, &defaultWC));

        // Save the callback address of CWnd::StaticWindowProc
        assert(defaultWC.lpfnWndProc);  // Assert fails when running UNICODE build on ANSI OS.
        m_callback = defaultWC.lpfnWndProc;
        VERIFY(::UnregisterClass(className, GetInstanceHandle()));
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

        if (reset)
        {
            pWinApp = 0;
        }
        else
        {
            if (pWinApp == 0)
                pWinApp = pThis;
            else
                assert(pThis == 0);
        }

        return pWinApp;
    }

    // Sets the main window for this thread.
    inline void CWinApp::SetMainWnd(HWND wnd) const
    {
        TLSData* pTLSData = GetApp()->GetTlsData();
        pTLSData->mainWnd = wnd;
    }

    // This function can be used to load a resource dll.
    // A resource dll can be used to define resources in different languages.
    // To use this function, place code like this in InitInstance
    //   HINSTANCE resource = LoadLibrary(_T("MyResourceDLL.dll"));
    //   SetResourceHandle(resource);
    // A resource of 0 can be specified to revert the resource back to m_instance.
    // The resource is automatically freed when it is no longer required.
    inline void CWinApp::SetResourceHandle(HINSTANCE resource)
    {
        // Free the current resource if appropriate.
        if ((m_resource != resource) && (m_resource != m_instance))
            ::FreeLibrary(m_resource);

        // Set the resources back to default.
        if (resource == 0)
            resource = m_instance;

        m_resource = resource;
    }

    // Creates the Thread Local Storage data for the current thread if none already exists,
    // and returns a pointer to the TLS data.
    inline void CWinApp::SetTlsData()
    {
        TLSData* pTLSData = GetTlsData();
        if (pTLSData == NULL)
        {
            pTLSData = new TLSData;
            TLSDataPtr dataPtr(pTLSData);

            CThreadLock TLSLock(m_appLock);
            m_allTLSData.push_back(dataPtr); // store as a smart pointer

            VERIFY(::TlsSetValue(m_tlsData, pTLSData));
        }
    }

    // Messages used for exceptions.
    inline CString CWinApp::MsgAppThread() const
    { return _T("Failed to create thread."); }

    inline CString CWinApp::MsgArReadFail() const
    { return _T("Failed to read from archive."); }

    inline CString CWinApp::MsgArNotCStringA() const
    { return _T("ANSI characters stored. Not a CStringW."); }

    inline CString CWinApp::MsgArNotCStringW() const
    { return _T("Unicode characters stored. Not a CStringA."); }

    inline CString CWinApp::MsgCriticalSection() const
    { return _T("Failed to create critical section."); }

    inline CString CWinApp::MsgMtxEvent() const
    { return _T("Unable to create event."); }

    inline CString CWinApp::MsgMtxMutex() const
    { return _T("Unable to create mutex."); }

    inline CString CWinApp::MsgMtxSemaphore() const
    { return _T("Unable to create semaphore."); }

    inline CString CWinApp::MsgWndCreate() const
    { return _T("Failed to create window."); }

    inline CString CWinApp::MsgWndDialog() const
    { return _T("Failed to create dialog."); }

    inline CString CWinApp::MsgWndGlobalLock() const
    { return _T("CGlobalLock failed to lock handle."); }

    inline CString CWinApp::MsgWndPropertSheet() const
    { return _T("Failed to create PropertySheet."); }

    inline CString CWinApp::MsgSocWSAStartup() const
    { return _T("WSAStartup failed."); }

    inline CString CWinApp::MsgSocWS2Dll() const
    { return _T("Failed to load WS2_2.dll."); }

    inline CString CWinApp::MsgIPControl() const
    { return _T("IP Address Control not supported!."); }

    inline CString CWinApp::MsgRichEditDll() const
    { return _T("Failed to load RICHED32.DLL."); }

    inline CString CWinApp::MsgTaskDialog() const
    { return _T("Failed to create Task Dialog."); }

    // CFile Messages
    inline CString CWinApp::MsgFileClose() const
    { return _T("Failed to close file."); }

    inline CString CWinApp::MsgFileFlush() const
    { return _T("Failed to flush file."); }

    inline CString CWinApp::MsgFileLock() const
    { return _T("Failed to lock the file."); }

    inline CString CWinApp::MsgFileOpen() const
    { return _T("Failed to open file."); }

    inline CString CWinApp::MsgFileRead() const
    { return _T("Failed to read from file."); }

    inline CString CWinApp::MsgFileRename() const
    { return _T("Failed to rename file."); }

    inline CString CWinApp::MsgFileRemove() const
    { return _T("Failed to delete file."); }

    inline CString CWinApp::MsgFileLength() const
    { return _T("Failed to change the file length."); }

    inline CString CWinApp::MsgFileUnlock() const
    { return _T("Failed to unlock the file."); }

    inline CString CWinApp::MsgFileWrite() const
    { return _T("Failed to write to file."); }

    // GDI Messages
    inline CString CWinApp::MsgGdiDC() const
    { return _T("Failed to create device context."); }

    inline CString CWinApp::MsgGdiIC() const
    { return _T("Failed to create information context."); }

    inline CString CWinApp::MsgGdiBitmap() const
    { return _T("Failed to create bitmap."); }

    inline CString CWinApp::MsgGdiBrush() const
    { return _T("Failed to create brush."); }

    inline CString CWinApp::MsgGdiFont() const
    { return _T("Failed to create font."); }

    inline CString CWinApp::MsgGdiPalette() const
    { return _T("Failed to create palette."); }

    inline CString CWinApp::MsgGdiPen() const
    { return _T("Failed to create pen."); }

    inline CString CWinApp::MsgGdiRegion() const
    { return  _T("Failed to create region."); }

    inline CString CWinApp::MsgGdiGetDC() const
    { return  _T("GetDC failed."); }

    inline CString CWinApp::MsgGdiGetDCEx() const
    { return _T("GetDCEx failed."); }

    inline CString CWinApp::MsgGdiSelObject() const
    { return _T("Failed to select object into device context."); }

    inline CString CWinApp::MsgGdiGetWinDC() const
    { return _T("GetWindowDC failed."); }

    inline CString CWinApp::MsgGdiBeginPaint() const
    { return _T("BeginPaint failed."); }

    // Image list, Menu and Printer messages
    inline CString CWinApp::MsgImageList() const
    { return _T("Failed to create imagelist."); }

    inline CString CWinApp::MsgMenu() const
    { return _T("Failed to create menu."); }

    inline CString CWinApp::MsgPrintFound() const
    { return _T("No printer available."); }

    // DDX anomaly prompting messages
    inline CString CWinApp::MsgDDX_Byte() const
    { return _T("Please enter an integer between 0 and 255."); }

    inline CString CWinApp::MsgDDX_Int() const
    { return _T("Please enter an integer."); }

    inline CString CWinApp::MsgDDX_Long() const
    { return _T("Please enter a long integer."); }

    inline CString CWinApp::MsgDDX_Short() const
    { return _T("Please enter a short integer."); }

    inline CString CWinApp::MsgDDX_Real() const
    { return _T("Please enter a number."); }

    inline CString CWinApp::MsgDDX_UINT() const
    { return _T("Please enter a positive integer."); }

    inline CString CWinApp::MsgDDX_ULONG() const
    { return _T("Please enter a positive long integer."); }

    // DDV formats and prompts
    inline CString CWinApp::MsgDDV_IntRange() const
    { return _T("Please enter an integer in (%ld, %ld)."); }

    inline CString CWinApp::MsgDDV_UINTRange() const
    { return _T("Please enter an positive integer in (%lu, %lu)."); }

    inline CString CWinApp::MsgDDV_RealRange() const
    { return _T("Please enter a number in (%.*g, %.*g)."); }

    inline CString CWinApp::MsgDDV_StringSize() const
    { return _T("%s\n is too long.\nPlease enter no more than %ld characters."); }

} // namespace Win32xx

#endif // _WIN32XX_APPCORE_H_

