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



////////////////////////////////////////////////////////////////////
// wxx_printdialogex.h
// Declaration of the CPrintDialogEx class.

// CPrintDialogEx displays the Print dialog which allows the user to
// select the printer to use.

// The CPrintDialogEx class stores its global memory in CWinApp.
// This global memory persists after the CPrintDialogEx is destroyed.
// There is no need for the user to manually free the hDevMode and
// hDevNames memory.

// CPrintDialogEx uses IPrintDialogCallback to forward the print
// dialog's messages for the child dialog box in the lower portion of
// the General page to DialogProc.

// CPrintDialogEx supports the use of IPrintDialogServices to retrieve
// the currently printer's DevMode, printer name and port name while
// print dialog is displayed.

// NOTE: CPrintDialogEx throws a CWinException if it is unable
// to display the dialog.

// NOTE: CPrintDialogEx will throw an exception if there is no default
// printer. You should always wrap DoModal in a in a try/catch block to
// take reasonable (or no) action if there isn't a default printer.
//
// A system always has a default printer if it has a printer.
//
// NOTE: Use the following to retrieve the printer's device context
// for the default or currently selected printer:
//     CPrintDialogEx printDialog;
//     CDC printerDC = printDialog.GetPrinterDC();
//
// NOTE: CPrintDialogEx requires Win2000 or greater (WINVER >= 0x0500).

#ifndef _WIN32XX_PRINTDIALOGEX_H_
#define _WIN32XX_PRINTDIALOGEX_H_

#include "wxx_wincore.h"
#include "wxx_commondlg.h"
#include "wxx_printdialogs.h"

namespace Win32xx
{

    ////////////////////////////////////////////////////////////////////
    // This class encapsulates the Windows API PrintDlgEx function.
    // The PrintDlgEx function displays a property sheet. The property
    // sheet allows the user to specify the printer, and the properties
    // of the print job.
    // NOTE: DoModal throws an exception if there is no default printer
    class CPrintDialogEx : public CDialog,
                           public IPrintDialogCallback,
                           public IObjectWithSite
    {
    public:
        // Constructor
        CPrintDialogEx(DWORD flags = PD_ALLPAGES | PD_USEDEVMODECOPIES | PD_NOPAGENUMS
                                   | PD_NOSELECTION | PD_NOCURRENTPAGE);

        // Destructor
        virtual ~CPrintDialogEx() {}

        virtual void OnApply() {}    // Apply then cancel button pressed.
        virtual void OnCancel() {}   // Cancel button pressed or closed.
        virtual void OnPrint() {}    // Print button pressed.

        // Operations
        INT_PTR DoModal(HWND owner /* = 0 */);
        int GetCopies() const;
        CDevMode GetCurrentDevMode();
        CStringW GetCurrentPortName() const;
        CStringW GetCurrentPrinterName() const;
        BOOL GetDefaults();
        CString GetDeviceName() const;
        CDevMode GetDevMode() const;
        CDevNames GetDevNames() const;
        CString GetDriverName() const;
        const PRINTDLGEX& GetParameters()  const { return m_pdex; }
        CString GetPortName() const;
        CDC GetPrinterDC() const;
        BOOL PrintAll() const;
        BOOL PrintCollate() const;
        BOOL PrintRange() const;
        BOOL PrintSelection() const;
        BOOL PrintToFile() const;
        void SetDefaults(HGLOBAL hDevMode, HGLOBAL hDevNames);
        void SetParameters(const PRINTDLGEX& pdx);

    protected:
        virtual INT_PTR DialogProc(UINT msg, WPARAM wparam, LPARAM lparam);

        // IUnknown
        STDMETHOD(QueryInterface)(REFIID riid, void** ppvObject);
        STDMETHOD_(ULONG, AddRef)()   { return 1; }
        STDMETHOD_(ULONG, Release)()  { return 1; }

        // IPrintDialogCallback
        STDMETHOD(InitDone)()                 { return S_FALSE; }
        STDMETHOD(SelectionChange)()          { return S_FALSE; }
        STDMETHOD(HandleMessage)(HWND wnd, UINT msg, WPARAM wparam,
                                   LPARAM lparam, LRESULT* pResult);

        // IObjectWithSite
        STDMETHOD(GetSite)(REFIID riid, void** ppvSite);
        STDMETHOD(SetSite)(IUnknown* pUnknown);

    private:
        CPrintDialogEx(const CPrintDialogEx&);              // Disable copy construction
        CPrintDialogEx& operator = (const CPrintDialogEx&); // Disable assignment operator
        PRINTDLGEX m_pdex;
        IPrintDialogServices* m_pServices;
        CHGlobal m_currentModeBuffer;
    };

    // Constructor for CPrintDialogEx class. The flags parameter specifies the
    // flags for the PRINTDLGEX structure. Refer to the description of the
    // PRINTDLGEX struct in the Windows API documentation.
    inline CPrintDialogEx::CPrintDialogEx(DWORD flags) : m_pServices(NULL)
    {
        ZeroMemory(&m_pdex, sizeof(m_pdex));
        m_pdex.lStructSize = sizeof(m_pdex);
        m_pdex.Flags = flags;
        m_pdex.nStartPage = START_PAGE_GENERAL;
        m_pdex.Flags &= ~PD_RETURNIC;   // information context not used.
        m_pdex.Flags &= ~PD_RETURNDC;   // not used, use GetPrinterDC to retrieve the dc.
    }

    // The dialog's message procedure. Override this function in your derived class
    // if you wish to handle messages.
    inline INT_PTR CPrintDialogEx::DialogProc(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        // A typical function might look like this:

        //  switch (msg)
        //  {
        //  case MESSAGE1:      // Some Windows API message
        //      OnMessage1();   // A user defined function
        //      break;          // Also do default processing
        //  case MESSAGE2:
        //      OnMessage2();
        //      return x;       // Don't do default processing, but instead return
        //                      //  a value recommended by the Windows API documentation
        //  }

        // Always pass unhandled messages on to DialogProcDefault
        return DialogProcDefault(msg, wparam, lparam);
    }

    // Display the print dialog, and allow the user to select various options.
    // Use SetParameters to specify the pages before calling DoModal.
    // An exception is thrown if the dialog isn't created.
    // An exception is thrown if there is no default printer.
    // Returns PD_RESULT_PRINT, PD_RESULT_APPLY, or PD_RESULT_CANCEL.
    inline INT_PTR CPrintDialogEx::DoModal(HWND owner /* = 0 */)
    {
        assert(!IsWindow());    // Only one window per CWnd instance allowed

        // Ensure only one print dialog is running at a time.
        CThreadLock lock(GetApp()->m_printLock);

        // Update the default printer
        GetApp()->UpdateDefaultPrinter();

        // Assign values to the PRINTDLGEX structure
        m_pdex.hDevMode = GetApp()->m_devMode;
        m_pdex.hDevNames = GetApp()->m_devNames;
        if (::IsWindow(owner))
            m_pdex.hwndOwner = owner;
        else
            m_pdex.hwndOwner = GetActiveWindow();

        m_pdex.lpCallback = (IPrintDialogCallback*)this;
        m_pdex.dwResultAction = 0;

        // Create the dialog
        if (S_OK != PrintDlgEx(&m_pdex))
        {
            int error = static_cast<int>(CommDlgExtendedError());
            throw CWinException(GetApp()->MsgWndDialog(), error);
        }

        switch (m_pdex.dwResultAction)
        {
        case PD_RESULT_APPLY:
        {
            // The user clicked the Apply button and later clicked the Cancel
            // button. This indicates that the user wants to apply the changes
            // made in the property sheet, but does not yet want to print.
            GetApp()->m_devMode.Reassign(m_pdex.hDevMode);
            GetApp()->m_devNames.Reassign(m_pdex.hDevNames);
            OnApply();
            break;
        }
        case PD_RESULT_CANCEL:
        {
            // The user clicked the cancel or close button.
            OnCancel();
            break;
        }
        case PD_RESULT_PRINT:
        {
            // The user clicked the print button.
            GetApp()->m_devMode.Reassign(m_pdex.hDevMode);
            GetApp()->m_devNames.Reassign(m_pdex.hDevNames);
            OnPrint();
            break;
        }
        }

        m_pdex.hDevMode = 0;
        m_pdex.hDevNames = 0;

        // Prepare this CWnd for reuse.
        Cleanup();

        return m_pdex.dwResultAction;
    }

    // Retrieves the number of copies requested.
    inline int CPrintDialogEx::GetCopies() const
    {
        if ((m_pdex.Flags & PD_USEDEVMODECOPIES) != 0)
            return GetDevMode()->dmCopies;

        return static_cast<int>(m_pdex.nCopies);
    }

    // Fill a DEVMODE structure with information about the currently
    // selected printer, while the print dialog is displayed.
    inline CDevMode CPrintDialogEx::GetCurrentDevMode()
    {
        if (m_pServices != 0)
        {
            // Retrieve the size of the current DevMode.
            UINT size = 0;
            DEVMODE tempMode;
            ZeroMemory(&tempMode, sizeof(tempMode));
            m_pServices->GetCurrentDevMode(&tempMode, &size);

            // Retrieve the current DevMode.
            m_currentModeBuffer.Alloc(size);
            CDevMode devMode(m_currentModeBuffer);
            m_pServices->GetCurrentDevMode(devMode, &size);
            return devMode;
        }

        return CDevMode(0);
    }

    // Returns the port name for the currently selected printer, while
    // the print dialog is displayed.
    inline CStringW CPrintDialogEx::GetCurrentPortName() const
    {
        CStringW str;
        if (m_pServices != 0)
        {
            UINT size = 0;
            m_pServices->GetCurrentPortName(0, &size);
            int bufferSize = static_cast<int>(size);
            m_pServices->GetCurrentPortName(str.GetBuffer(bufferSize), &size);
            str.ReleaseBuffer();
        }

        return str;
    }

    // Returns the printer name for the currently selected printer, while
    // the print dialog is displayed.
    inline CStringW CPrintDialogEx::GetCurrentPrinterName() const
    {
        CStringW str;
        if (m_pServices != 0)
        {
            UINT size = 0;
            m_pServices->GetCurrentPrinterName(0, &size);
            int bufferSize = static_cast<int>(size);
            m_pServices->GetCurrentPrinterName(str.GetBuffer(bufferSize), &size);
            str.ReleaseBuffer();
        }

        return str;
    }

    // Sets the printer and the page settings to default, without displaying a dialog.
    // The hDevMode and hDevNames memory is freed and reallocated.
    // Returns TRUE if a default printer exists.
    inline BOOL CPrintDialogEx::GetDefaults()
    {
        CThreadLock lock(GetApp()->m_printLock);

        if (m_pdex.hDC)
        {
            ::DeleteDC(m_pdex.hDC);
            m_pdex.hDC = 0;
        }

        HWND oldOwner = m_pdex.hwndOwner;
        if (!::IsWindow(m_pdex.hwndOwner))
            m_pdex.hwndOwner = ::GetActiveWindow();

        m_pdex.Flags |= PD_RETURNDEFAULT;
        ::PrintDlgEx(&m_pdex);
        m_pdex.Flags &= ~PD_RETURNDEFAULT;

        m_pdex.hwndOwner = oldOwner;

        // Reset global memory
        SetDefaults(m_pdex.hDevMode, m_pdex.hDevNames);

        m_pdex.hDevMode = 0;
        m_pdex.hDevNames = 0;

        // Return TRUE if default printer exists
        return (GetApp()->m_devNames.Get()) ? TRUE : FALSE;
    }

    // Retrieves the name of the default or currently selected printer device.
    inline CString CPrintDialogEx::GetDeviceName() const
    {
        CThreadLock lock(GetApp()->m_printLock);

        if (GetApp()->m_devNames.Get() == 0)
            GetApp()->UpdateDefaultPrinter();

        CString str;
        if (GetApp()->m_devNames.Get() != 0)
            str = GetDevNames().GetDeviceName();

        return str;
    }

    // Returns a pointer to the locked hDevMode memory encapsulated in a CDevMode object.
    // There is no need to unlock this memory. The CDevMode object automatically
    // unlocks the memory when it goes out of scope.
    // Usage:
    //  CDevMode pDevMode = GetDevMode();
    //  Then use pDevMode as if it were a LPDEVMODE
    inline CDevMode CPrintDialogEx::GetDevMode() const
    {
        CThreadLock lock(GetApp()->m_printLock);

        if (GetApp()->m_devMode.Get() == 0)
            GetApp()->UpdateDefaultPrinter();

        if (GetApp()->m_devMode.Get() == 0)
            throw CResourceException(GetApp()->MsgPrintFound());

        return CDevMode(GetApp()->m_devMode);
    }

    // Returns a pointer to the locked hDevNames memory encapsulated in a CDevNames object.
    // There is no need to unlock this memory. The CDevNames object automatically
    // unlocks the memory when it goes out of scope.
    // Usage:
    //  CDevNames pDevNames = GetDevNames();
    //  Then use pDevNames as if it were a LPDEVNAMES
    inline CDevNames CPrintDialogEx::GetDevNames() const
    {
        CThreadLock lock(GetApp()->m_printLock);

        if (GetApp()->m_devNames.Get() == 0)
            GetApp()->UpdateDefaultPrinter();

        if (GetApp()->m_devNames.Get() == 0)
            throw CResourceException(GetApp()->MsgPrintFound());

        return CDevNames(GetApp()->m_devNames);
    }

    // Retrieves the name of the default or currently selected printer driver.
    inline CString CPrintDialogEx::GetDriverName() const
    {
        CThreadLock lock(GetApp()->m_printLock);

        if (GetApp()->m_devNames.Get() == 0)
            GetApp()->UpdateDefaultPrinter();

        CString str;
        if (GetApp()->m_devNames.Get() != 0)
            str = GetDevNames().GetDriverName();

        return str;
    }

    // Retrieves the name of the default or currently selected printer port.
    inline CString CPrintDialogEx::GetPortName() const
    {
        CThreadLock lock(GetApp()->m_printLock);

        if (GetApp()->m_devNames.Get() == 0)
            GetApp()->UpdateDefaultPrinter();

        CString str;
        if (GetApp()->m_devNames.Get() != 0)
            str = GetDevNames().GetPortName();

        return str;
    }

    // Returns the device context of the default or currently chosen printer.
    // Throws on failure.
    inline CDC CPrintDialogEx::GetPrinterDC() const
    {
        CThreadLock lock(GetApp()->m_printLock);
        CDC dc;
        if (GetApp()->m_devNames.Get() == 0)
            GetApp()->UpdateDefaultPrinter();

        if ((GetApp()->m_devNames.Get() != 0) && (GetApp()->m_devMode.Get() != 0))
        {
            dc.CreateDC(GetDriverName(), GetDeviceName(),
                GetPortName(), GetDevMode());
        }

        if (dc.GetHDC() == 0)
            throw CResourceException(GetApp()->MsgPrintFound());

        return dc;
    }

    inline DECLSPEC_NOTHROW HRESULT CPrintDialogEx::GetSite(REFIID riid, void** ppvSite)
    {
        if (riid == IID_IPrintDialogServices)
        {
            *ppvSite = m_pServices;
            return S_OK;
        }

        return QueryInterface(riid, ppvSite);
    }

    // Passes messages on to the DialogProc for processing.
    inline DECLSPEC_NOTHROW HRESULT CPrintDialogEx::HandleMessage(HWND wnd, UINT msg, WPARAM wparam,
                                                      LPARAM lparam, LRESULT* pResult)
    {
        if (GetHwnd() == 0)
            Attach(wnd);

        *pResult = DialogProc(msg, wparam, lparam);
        return S_FALSE;  // Perform its default message handling
    }

    // Returns TRUE if collate checked.
    inline BOOL CPrintDialogEx::PrintCollate() const
    {
        return (m_pdex.Flags & PD_COLLATE) ? TRUE : FALSE;
    }

    // Returns TRUE if printing selection.
    inline BOOL CPrintDialogEx::PrintSelection() const
    {
        return (m_pdex.Flags & PD_SELECTION) ? TRUE : FALSE;
    }

    // Returns TRUE if printing all pages.
    inline BOOL CPrintDialogEx::PrintAll() const
    {
        return (!PrintRange() && !PrintSelection()) ? TRUE : FALSE;
    }

    // Returns TRUE if printing page range.
    inline BOOL CPrintDialogEx::PrintRange() const
    {
        return (m_pdex.Flags & PD_PAGENUMS) ? TRUE : FALSE;
    }

    // Returns TRUE if printing to a file.
    inline BOOL CPrintDialogEx::PrintToFile() const
    {
        return (m_pdex.Flags & PD_PRINTTOFILE) ? TRUE : FALSE;
    }

    // Returns a pointer to the requested object.
    inline DECLSPEC_NOTHROW HRESULT CPrintDialogEx::QueryInterface(REFIID riid, void** ppvObject)
    {
        if (ppvObject == NULL)
            return E_POINTER;

        if (IsEqualGUID(riid, IID_IUnknown))
        {
            *ppvObject = (IPrintDialogCallback*)this;
            return S_OK;
        }
        else if (IsEqualGUID(riid, IID_IPrintDialogCallback))
        {
            *ppvObject = (IPrintDialogCallback*)this;
            return S_OK;
        }
        else if (IsEqualGUID(riid, IID_IObjectWithSite))
        {
            *ppvObject = (IObjectWithSite*)this;
            return S_OK;
        }

        return E_NOINTERFACE;
    }

    // Assigns the application's default printer settings to the values
    // specified by hDevMode and hDevNames. CWinApp now owns the global
    // memory and is responsible for freeing it.
    inline void CPrintDialogEx::SetDefaults(HGLOBAL hDevMode, HGLOBAL hDevNames)
    {
        CThreadLock lock(GetApp()->m_printLock);

        // Reset global memory
        if (hDevMode != GetApp()->m_devMode)
        {
            GetApp()->m_devMode.Free();
            GetApp()->m_devMode.Reassign(hDevMode);
        }

        if (hDevNames != GetApp()->m_devNames)
        {
            GetApp()->m_devNames.Free();
            GetApp()->m_devNames.Reassign(hDevNames);
        }
    }

    // Set the parameters of the PRINTDLGEX structure to sensible values
    inline void CPrintDialogEx::SetParameters(const PRINTDLGEX& pdex)
    {
        m_pdex.lStructSize      = sizeof(m_pdex);            // size of structure in bytes
        m_pdex.Flags            = pdex.Flags;                // flags
        m_pdex.Flags2           = pdex.Flags2;               // reserved
        m_pdex.ExclusionFlags   = pdex.ExclusionFlags;       // items to exclude from driver pages
        m_pdex.nPageRanges      = pdex.nPageRanges;          // number of page ranges
        m_pdex.nMaxPageRanges   = pdex.nMaxPageRanges;       // max number of page ranges
        m_pdex.lpPageRanges     = pdex.lpPageRanges;         // array of page ranges
        m_pdex.nMinPage         = pdex.nMinPage;             // min page number
        m_pdex.nMaxPage         = pdex.nMaxPage;             // max page number
        m_pdex.nCopies          = pdex.nCopies;              // number of copies
        m_pdex.hInstance        = pdex.hInstance;            // instance handle
        m_pdex.lpPrintTemplateName = pdex.lpPrintTemplateName;   // template name for app specific area
        m_pdex.nPropertyPages   = pdex.nPropertyPages;       // number of app property pages in lphPropertyPages
        m_pdex.lphPropertyPages = pdex.lphPropertyPages;     // array of app property page handles
        m_pdex.nStartPage       = pdex.nStartPage;           // start page id
        m_pdex.dwResultAction   = pdex.dwResultAction;       // result action if S_OK is returned
        m_pdex.Flags &= ~PD_RETURNIC;   // information context not used.
        m_pdex.Flags &= ~PD_RETURNDC;   // not used, use GetPrinterDC to retrieve the dc.

        // These are set in DoModal
        // m_pdex.hwndOwner    - caller's window handle
        // m_pdex.lpCallback   - app callback interface
    }

    inline DECLSPEC_NOTHROW HRESULT CPrintDialogEx::SetSite(IUnknown* pUnknown)
    {
        m_pServices = (IPrintDialogServices*)pUnknown;
        return S_OK;
    }
}


#endif  // define _WIN32XX_PRINTDIALOGEX_H_
