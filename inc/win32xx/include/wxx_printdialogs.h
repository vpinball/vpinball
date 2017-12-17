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


////////////////////////////////////////////////////////
// Acknowledgement:
//
// The original author of these classes is:
//
//      Robert C. Tausworthe
//      email: robert.c.tausworthe@ieee.org
//
////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
// wxx_printdialogs.h
// Declaration of the CPrintDialog and CPageSetupDialog class, along
// with the CGlobalLock template class.

// CPrintDialog displays the Print dialog which allows the user to
// select the printer to use.

// CPageSetupDialog displays the PageSetup dialg which allows the user
// to select the page settings.

// The CPrintDialog and CPageSetupDialog classes share global memory
// for their hDevMode and hDevNames handles. This global memory
// persists after the CPrintDialog and CPageSetupDialog objects are
// destructed, and is managed by CWinApp. There is no need for the
// user to manually free the hDevMode and hDevNames memory.

// CDevMode and CDevNames are typedefs for the CGlobalLock class. They
// provide self unlocking pointers to the hDevMode and hDevNames memory.
// They are used as follows:
//   CPrintDialog PrintDialog;
//   CDevNames pDevNames = PrintDialog.GetDevNames();
//   CDevMode  pDevMode  = PrintDialog.GetDevMode();
//
// NOTE: CPrintDialog and CPageSetupDialog throw a CWinException
// if they are unable to display the dialog.

// NOTE: CPrintDialog and CPageSetupDialog will throw an exception if
// there is no default printer. You should always wrap DoModal in a in
// a try/catch block to take reasonable (or no) action if there isn't
// a default printer.
//
// A system always has a default printer if it has a printer.
//

#ifndef _WIN32XX_PRINTDIALOGS_H_
#define _WIN32XX_PRINTDIALOGS_H_

#include "wxx_wincore.h"
#include "wxx_commondlg.h"


namespace Win32xx
{

    ///////////////////////////////////////////////////////////////////
    // Acknowledgement:
    //  CGlobalLock is based on code by Rob Caldecott
    //
    //////////////////////////////////////////////////////////////////


    //////////////////////////////////////
    // CGlobaLock is a template class used to provide a self unlocking
    // object to global memory. It is used to provide convenient access
    // to the memory provided by hDevMode and hDevNames handles.
    template <typename T>
    class CGlobalLock
    {
    public:
        // Constructors and Destructors
        CGlobalLock() : m_h(0), m_p(0)  {}
        CGlobalLock(HANDLE h) : m_h(h)  { Lock(); }
        ~CGlobalLock()                  { Unlock(); }

        T* Get() const          { return m_p; }                 // Returns the pointer

        // operator overloads
        operator T*() const     { return m_p; }                 // Conversion operator to pointer
        T* operator->() const   { assert(m_p); return m_p; }    // Pointer operator
        operator LPCTSTR() const;                               // Conversion operator to LPCTSTR (for DEVNAMES only)
        T& operator*() const    { assert(m_p); return *m_p; }   // Dereference operator

        CGlobalLock& operator=(HANDLE h)                        // Assignment operator
        {
            Unlock();
            m_h = h;
            Lock();
            return *this;
        }

    private:
        // Lock the handle
        void Lock()
        {
            if (m_h != 0)
            {
                m_p = reinterpret_cast<T*>(::GlobalLock(m_h));
                // Did the lock succeed?
                if (m_p == 0)
                {
                    // The handle is probably invalid
                    throw CWinException(_T("CGlobalLock::Lock failed to lock handle"));
                }
            }
            else
                m_p = 0;
        }

        // Unlock the handle
        void Unlock()
        {
            if (m_h != 0)
            {
                ::GlobalUnlock(m_h);
                m_h = 0;
            }
        }

        HANDLE m_h;     // The handle to lock/unlock
        T* m_p;         // Pointer returned by ::GlobalLock
    };

    template <>
    inline CGlobalLock<DEVNAMES>::operator LPCTSTR() const
    {
        assert(m_p != NULL);
        return reinterpret_cast<LPCTSTR>(m_p);
    }


    //////////////////////////////////////////////////////////////////////
    // A set of typedefs to simplify the use of CGlobalLock.
    // These provide self unlocking objects which can be used for pointers
    // to global memory. There is no need to unlock the pointer.
    //
    // Example usage:
    //   CDevMode  pDevMode(hDevMode);      // and use pDevMode as if it were a LPDEVMODE
    //   CDevName  pDevNames(hDevNames);    // and use pDevNames as if it were a LPDEVNAMES
    //   assert(hDevNames.Get());           // Get can be used to access the underlying pointer
    //
    /////////////////////////////////////////////////////////////////////

    typedef CGlobalLock<DEVMODE>    CDevMode;
    typedef CGlobalLock<DEVNAMES>   CDevNames;


    ////////////////////////////////////////////////////////////////////
    // This class encapsulates the Windows API PrintDlg function.
    // The PrintDlg function displays a Print Dialog. The Print dialog
    // enables the user to specify the properties of a particular print job.
    // NOTE: DoModal throws an exception if there is no default printer
    class CPrintDialog : public CCommonDialog
    {
    public:
        CPrintDialog(DWORD dwFlags = PD_ALLPAGES | PD_USEDEVMODECOPIES | PD_NOPAGENUMS |
                                        PD_HIDEPRINTTOFILE | PD_NOSELECTION );
        virtual ~CPrintDialog();

        virtual INT_PTR DoModal(HWND hWndOwner = 0);
        int     GetCopies() const;
        BOOL    GetDefaults();
        CDevMode GetDevMode() const;
        CDevNames GetDevNames() const;
        CString GetDeviceName() const;
        CString GetDriverName() const;
        int     GetFromPage() const;
        const   PRINTDLG& GetParameters()  const { return m_PD; }
        CString GetPortName() const;
        CDC     GetPrinterDC() const;
        int     GetToPage() const;
        BOOL    PrintAll() const;
        BOOL    PrintCollate() const;
        BOOL    PrintRange() const;
        BOOL    PrintSelection() const;
        void    SetParameters(PRINTDLG& pd);

    protected:
        // Override these functions as required
        virtual INT_PTR DialogProc(UINT, WPARAM, LPARAM);

        // Not intended to be overridden
        INT_PTR DialogProcDefault(UINT, WPARAM, LPARAM);

    private:
        void GlobalFreeAll()
        {
            GetApp().GlobalFreeAll(GetApp().m_hDevMode);
            GetApp().GlobalFreeAll(GetApp().m_hDevNames);
            GetApp().m_hDevMode = 0;
            GetApp().m_hDevNames = 0;
        }

        // printer resources
        PRINTDLG        m_PD;           // printer selection dlg structure
    };


    ////////////////////////////////////////////////////////////////////
    // This class encapsulates the Windows API PageSetupDlg function.
    // The PageSetupDlg function creates a Page Setup dialog box that
    // enables the user to specify the attributes of a printed page.
    // These attributes include the paper size and source, the page
    // orientation (portrait or landscape), and the width of the page
    // margins.
    // NOTE: DoModal throws an exception if there is no default printer
    class CPageSetupDialog : public CCommonDialog
    {
    public:
        CPageSetupDialog( DWORD dwFlags = PSD_MARGINS );
        virtual ~CPageSetupDialog() {}

        virtual INT_PTR DoModal(HWND hWndOwner = 0);
        CDevMode GetDevMode() const;
        CDevNames GetDevNames() const;
        void    GetMargins(RECT& rcMargin, RECT& rcMinMargin) const;
        CSize   GetPaperSize() const;
        const   PAGESETUPDLG& GetParameters() const { return m_PSD; }
        void    SetParameters(PAGESETUPDLG& psd);

    protected:
        // Override these functions as required
        virtual INT_PTR DialogProc(UINT, WPARAM, LPARAM);
        virtual UINT    OnDrawPage(HDC, UINT, const RECT&);
        virtual UINT    OnPreDrawPage(WORD wPaper, WORD wFlags, const PAGESETUPDLG& PSD);

        // Not intended to be overridden
        INT_PTR DialogProcDefault(UINT, WPARAM, LPARAM);
        static INT_PTR CALLBACK PaintHookProc(HWND, UINT, WPARAM, LPARAM);

    private:
        void GlobalFreeAll()
        {
            GetApp().GlobalFreeAll(GetApp().m_hDevMode);
            GetApp().GlobalFreeAll(GetApp().m_hDevNames);
            GetApp().m_hDevMode = 0;
            GetApp().m_hDevNames = 0;
        }

        PAGESETUPDLG    m_PSD;          // page setup dlg structure
    };

}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

namespace Win32xx
{

    /////////////////////////////////////////
    // Definitions for the CWinApp class
    //


    // Allocates the global memory for the default printer if required.
    // Resets the global memory if we were using the default printer, and the
    // default printer has changed.
    inline void CWinApp::UpdateDefaultPrinter()
    {
        if (m_hDevNames == 0)
        {
            // Allocate global printer memory by specifying the default printer.
            CPrintDialog pd;
            pd.GetDefaults();
        }
        else
        {
            // Global memory has already been allocated
            LPDEVNAMES pDevNames = (LPDEVNAMES)::GlobalLock(m_hDevNames);
            assert (pDevNames);

            // If our gobal memory is referencing the default printer
            if (pDevNames && pDevNames->wDefault & DN_DEFAULTPRN)
            {
                // Get current default printer
                PRINTDLG pd;
                ZeroMemory(&pd, sizeof(pd));
                pd.lStructSize = sizeof(pd);
                pd.Flags = PD_RETURNDEFAULT;
                PrintDlg(&pd);

                if (pd.hDevNames == 0)
                {
                    // Printer was default, but now there are no printers.
                    ::GlobalUnlock(m_hDevNames);
                    ::GlobalUnlock(m_hDevMode);
                    GlobalFreeAll(m_hDevMode);
                    GlobalFreeAll(m_hDevNames);
                    m_hDevMode = 0;
                    m_hDevNames = 0;
                }
                else
                {
                    // Compare current default printer to the one in global memory
                    LPDEVNAMES pDefDevNames = reinterpret_cast<LPDEVNAMES>(::GlobalLock(pd.hDevNames));
                    if (pDefDevNames)
                    {
                        if (lstrcmp(reinterpret_cast<LPCTSTR>(pDevNames) + pDevNames->wDriverOffset,
                            reinterpret_cast<LPCTSTR>(pDefDevNames) + pDefDevNames->wDriverOffset) != 0 ||
                            lstrcmp(reinterpret_cast<LPCTSTR>(pDevNames) + pDevNames->wDeviceOffset,
                                reinterpret_cast<LPCTSTR>(pDefDevNames) + pDefDevNames->wDeviceOffset) != 0 ||
                            lstrcmp(reinterpret_cast<LPCTSTR>(pDevNames) + pDevNames->wOutputOffset,
                                reinterpret_cast<LPCTSTR>(pDefDevNames) + pDefDevNames->wOutputOffset) != 0)
                        {
                            // Default printer has changed. Reset the global memory.
                            ::GlobalUnlock(m_hDevNames);
                            ::GlobalUnlock(pd.hDevNames);
                            GlobalFreeAll(m_hDevMode);
                            GlobalFreeAll(m_hDevNames);

                            m_hDevMode = pd.hDevMode;
                            m_hDevNames = pd.hDevNames;
                        }
                    }
                    else
                    {
                        // Printer was default, and unchanged. Keep the existing global memory.
                        ::GlobalUnlock(m_hDevNames);
                        ::GlobalUnlock(pd.hDevNames);

                        GlobalFreeAll(pd.hDevMode);
                        GlobalFreeAll(pd.hDevNames);
                    }
                }
            }

            ::GlobalUnlock(m_hDevNames);
        }
    }

    /////////////////////////////////////////
    // Definitions for the CPrintDialog class
    //


    // Constructor for CPrintDialog class. The dwFlags parameter specifies the
    // flags for the PRINTDLG structure. Refer to the description of the
    // PRINTDLG struct in the Windows API documentation.
    inline CPrintDialog::CPrintDialog(DWORD dwFlags /* = PD_ALLPAGES | PD_USEDEVMODECOPIES | PD_NOPAGENUMS | PD_HIDEPRINTTOFILE | PD_NOSELECTION */)
    {
        // initialize the PRINTDLG member
        ZeroMemory(&m_PD, sizeof(m_PD));

        m_PD.Flags = dwFlags;

        // Support the PD_PRINTSETUP flag which displays the obsolete PrintSetup dialog.
        // Note: CPageSetupDialog should be used instead of the PrintSetup dialog.
        if (dwFlags & PD_PRINTSETUP)
        {
            m_PD.Flags &= ~PD_RETURNDC;
        }
        else
        {
            m_PD.Flags |= PD_RETURNDC;
        }

        m_PD.Flags &= ~PD_RETURNIC;

        // Enable the hook proc for the help button
        if (m_PD.Flags & PD_SHOWHELP)
            m_PD.Flags |= PD_ENABLEPRINTHOOK;

        SetParameters(m_PD);
    }

    inline CPrintDialog::~CPrintDialog()
    {
        if (m_PD.hDC)
            ::DeleteDC(m_PD.hDC);
    }


    // Returns the printer's device context.
    inline CDC CPrintDialog::GetPrinterDC() const
    {
        CThreadLock Threadlock(GetApp().m_csPrintLock);
        CDC dc;
        GetApp().UpdateDefaultPrinter();

        if ((GetApp().m_hDevNames != 0) && (GetApp().m_hDevMode != 0))
        {
            LPDEVNAMES pDevNames = CDevNames(GetApp().m_hDevNames);
            if (pDevNames)
            {
                LPDEVMODE pDevMode = CDevMode(GetApp().m_hDevMode);
                if (pDevMode)
                {
                    dc.CreateDC(reinterpret_cast<LPCTSTR>(pDevNames) + pDevNames->wDriverOffset,
                        reinterpret_cast<LPCTSTR>(pDevNames) + pDevNames->wDeviceOffset,
                        reinterpret_cast<LPCTSTR>(pDevNames) + pDevNames->wOutputOffset,
                        pDevMode);
                }
            }
        }

        return dc;
    }


    // Dialog procedure for the Font dialog. Override this function
    // to customise the message handling.
    inline INT_PTR CPrintDialog::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        //  A typical override might look like this:

        //  switch (uMsg)
        //  {
        //  case MESSAGE1:      // Some Windows API message
        //      OnMessage1();   // A user defined function
        //      break;      // Also do default processing

        //  case MESSAGE2:
        //      OnMessage2();
        //      return x;   // Don't do default processing, but
        //              // instead return a value recommended
        //              // by the Windows API documentation
        //  }

        // Always pass unhandled messages on to DialogProcDefault
        return DialogProcDefault(uMsg, wParam, lParam);
    }


    //  The default message handling for CPrintDialog. Don't override this
    //  function, override DialogProc instead.
    //  Note: OnCancel and OnOK are called by DoModal.
    inline INT_PTR CPrintDialog::DialogProcDefault(UINT message, WPARAM wParam, LPARAM lParam)
    {
        UNREFERENCED_PARAMETER(lParam);

        switch (message)
        {
            case WM_INITDIALOG:
            {     // handle the initialization message
                return OnInitDialog();
            }

            case WM_COMMAND:
            {
                switch (LOWORD(wParam))
                {
                    case pshHelp:
                    OnHelpButton();
                    return TRUE;
                }
                break;
            }
        }

        return 0;
    }


    // Display the print dialog, and allow the user to select various options.
    // An exception is thrown if the dialog isn't created.
    // An exception is thrown if there is no default printer.
    inline INT_PTR CPrintDialog::DoModal( HWND hWndOwner /* = 0 */)
    {
        assert( &GetApp() );    // Test if Win32++ has been started
        assert(!IsWindow());    // Only one window per CWnd instance allowed

        // Ensure only one print dialog is running at a time.
        CThreadLock Threadlock(GetApp().m_csPrintLock);
                                
        // Update the default printer
        GetApp().UpdateDefaultPrinter();

        // Assign values to the PRINTDLG structure
        m_PD.hDevMode = GetApp().m_hDevMode;
        m_PD.hDevNames = GetApp().m_hDevNames;
        m_PD.hwndOwner = hWndOwner;

        if (m_PD.hDC != 0)
        {
            ::DeleteDC(m_PD.hDC);
            m_PD.hDC = 0;
        }

        // Ensure this thread has the TLS index set
        TLSData* pTLSData = GetApp().SetTlsData();
        // Create the modal dialog
        pTLSData->pWnd = this;

        // invoke the control and save the result
        BOOL ok = ::PrintDlg(&m_PD);

        if (ok)
        {
            GetApp().m_hDevMode = m_PD.hDevMode;
            GetApp().m_hDevNames = m_PD.hDevNames;
            OnOK();
            ok = IDOK;
        }
        else
        {
            GlobalUnlock(GetApp().m_hDevMode);
            GlobalUnlock(GetApp().m_hDevNames);

            DWORD dwError = CommDlgExtendedError();
            if ((dwError != 0) && (dwError != CDERR_DIALOGFAILURE))
            // ignore the exception caused by closing the dialog
            {
                // Reset global memory
                GlobalFreeAll();
                throw CWinException(_T("CPrintDialog::DoModal Failed"), dwError);
            }

            OnCancel();
            ok = IDCANCEL;
        }

        m_PD.hDevMode = 0;
        m_PD.hDevNames = 0;

        return ok;
    }


    // Retrieves the number of copies requested.
    inline int CPrintDialog::GetCopies() const
    {
        if (m_PD.Flags & PD_USEDEVMODECOPIES)
            return GetDevMode()->dmCopies;
        else
            return m_PD.nCopies;
    }


    // Sets the printer and the page settings to default, without displaying a dialog.
    // The hDevMode and hDevNames memory is freed and reallocated.
    // Returns TRUE if a default printer exists.
    inline BOOL CPrintDialog::GetDefaults()
    {
        CThreadLock Threadlock(GetApp().m_csPrintLock);

        // Reset global memory
        GlobalFreeAll();

        if (m_PD.hDC)
        {
            ::DeleteDC(m_PD.hDC);
            m_PD.hDC = 0;
        }

        m_PD.Flags |= PD_RETURNDEFAULT;
        ::PrintDlg(&m_PD);
        m_PD.Flags &= ~PD_RETURNDEFAULT;

        GetApp().m_hDevMode = m_PD.hDevMode;
        GetApp().m_hDevNames = m_PD.hDevNames;

        m_PD.hDevMode = 0;
        m_PD.hDevNames = 0;

        // Return TRUE if default printer exists
        return (GetApp().m_hDevNames != 0);
    }


    // Retrieves the name of the currently selected printer device.
    inline CString CPrintDialog::GetDeviceName() const
    {
        CString str;
        if (GetApp().m_hDevNames != 0)
        {
            CDevNames pDev = GetDevNames();
            LPCTSTR name = static_cast<LPCTSTR>(pDev) + pDev->wDeviceOffset;
            str = name;
        }

        return str;
    }


    // Returns a pointer to the locked hDevMode memory encapsulated in a CDevMode object.
    // There is no need to unlock this memory. The CDevMode object automatically
    // unlocks the memory when it goes out of scope.
    // Usage:
    //  CDevMode pDevMode = GetDevMode();
    //  Then use pDevMode as if it were a LPDEVMODE
    inline CDevMode CPrintDialog::GetDevMode() const
    {
        return CDevMode(GetApp().m_hDevMode);
    }


    // Returns a pointer to the locked hDevNames memory encapsulated in a CDevNames object.
    // There is no need to unlock this memory. The CDevNames object automatically
    // unlocks the memory when it goes out of scope.
    // Usage:
    //  CDevNames pDevNames = GetDevNames();
    //  Then use pDevNames as if it were a LPDEVNAMES
    inline CDevNames CPrintDialog::GetDevNames() const
    {
        return CDevNames(GetApp().m_hDevNames);
    }


    // Retrieves the name of the currently selected printer driver.
    inline CString CPrintDialog::GetDriverName() const
    {
        CString str;
        if (GetApp().m_hDevNames != 0)
        {
            CDevNames pDev = GetDevNames();
            LPCTSTR name = static_cast<LPCTSTR>(pDev) + pDev->wDriverOffset;
            str =  name;
        }

        return str;
    }


    // Retrieves the starting page of the print range.
    inline int CPrintDialog::GetFromPage() const
    {
        return (PrintRange() ? m_PD.nFromPage : -1);
    }


    // Retrieves the name of the currently selected printer port.
    inline CString CPrintDialog::GetPortName() const
    {
        CString str;
        if (GetApp().m_hDevNames != 0)
        {
            CDevNames pDev = GetDevNames();
            LPCTSTR name = static_cast<LPCTSTR>(pDev) + pDev->wOutputOffset;
            str = name;
        }
        return str;
    }


    // Retrieves the ending page of the print range.
    inline int CPrintDialog::GetToPage() const
    {
        return (PrintRange() ? m_PD.nToPage : -1);
    }


    // Call this function after calling DoModal to determine whether to print
    // all pages in the document.
    inline BOOL CPrintDialog::PrintAll() const
    {
        return !PrintRange() && !PrintSelection() ? TRUE : FALSE;
    }


    // Call this function after calling DoModal to determine whether the printer
    // should collate all printed copies of the document.
    inline BOOL CPrintDialog::PrintCollate() const
    {
        if (m_PD.Flags & PD_USEDEVMODECOPIES)
            return (GetDevMode()->dmCollate == DMCOLLATE_TRUE);
        else
            return (m_PD.Flags & PD_COLLATE ? TRUE : FALSE);
    }


    // Call this function after calling DoModal to determine whether to print
    // only a range of pages in the document.
    inline BOOL CPrintDialog::PrintRange() const
    {
        return m_PD.Flags & PD_PAGENUMS ? TRUE : FALSE;
    }


    // Call this function after calling DoModal to determine whether to print
    // only the currently selected items.
    inline BOOL CPrintDialog::PrintSelection() const
    {
        return m_PD.Flags & PD_SELECTION ? TRUE : FALSE;
    }


    // Set the parameters of the PRINTDLG structure to sensible values
    inline void CPrintDialog::SetParameters(PRINTDLG& pd)
    {
        m_PD.lStructSize    = sizeof(m_PD);
        m_PD.hwndOwner      = 0;            // Set this in DoModal
        m_PD.Flags          = pd.Flags;
        m_PD.nFromPage      = pd.nFromPage;
        m_PD.nToPage        = pd.nToPage;
        m_PD.nMinPage       = pd.nMinPage;
        m_PD.nMaxPage       = pd.nMaxPage;
        m_PD.nCopies        = pd.nCopies;
        m_PD.hInstance      = GetApp().GetResourceHandle();
        m_PD.lpfnPrintHook  = reinterpret_cast<LPCCHOOKPROC>(CDHookProc);
        m_PD.lpfnSetupHook  = reinterpret_cast<LPCCHOOKPROC>(CDHookProc);
        m_PD.lCustData      = pd.lCustData;
        m_PD.hPrintTemplate = pd.hPrintTemplate;
        m_PD.hSetupTemplate = pd.hSetupTemplate;
        m_PD.lpPrintTemplateName = pd.lpPrintTemplateName;
        m_PD.lpSetupTemplateName = pd.lpSetupTemplateName;
    }


    /////////////////////////////////////////////
    // Definitions for the CPageSetupDialog class
    //


    // Constructor for CPageSetupDialog class. The dwFlags parameter specifies the
    // flags for the PAGESETUPDLG structure. Refer to the description of the
    // PAGESETUPDLG struct in the Windows API documentation.
    inline CPageSetupDialog::CPageSetupDialog( DWORD dwFlags /* = PSD_MARGINS */ )
    {
        ZeroMemory(&m_PSD, sizeof(m_PSD));
        m_PSD.Flags = dwFlags;

        // Enable the hook proc for the help button
        if (m_PSD.Flags & PSD_SHOWHELP)
            m_PSD.Flags |= PSD_ENABLEPAGESETUPHOOK;

        SetParameters(m_PSD);
    }


    // Dialog procedure for the PageSetup dialog. Override this function
    // to customise the message handling.
    inline INT_PTR CPageSetupDialog::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        //  A typical override might look like this:

        //  switch (uMsg)
        //  {
        //  case MESSAGE1:      // Some Windows API message
        //      OnMessage1();   // A user defined function
        //      break;      // Also do default processing

        //  case MESSAGE2:
        //      OnMessage2();
        //      return x;   // Don't do default processing, but
        //              // instead return a value recommended
        //              // by the Windows API documentation
        //  }

        // Always pass unhandled messages on to DialogProcDefault
        return DialogProcDefault(uMsg, wParam, lParam);
    }


    //  The Default message handling for CPageSetupDialog. Don't override this
    //  function, override DialogProc instead.
    //  Note: OnCancel and OnOK are called by DoModal.
    inline INT_PTR CPageSetupDialog::DialogProcDefault(UINT message, WPARAM wParam, LPARAM lParam)
    {
        UNREFERENCED_PARAMETER(lParam);

        switch (message)
        {
        case WM_INITDIALOG:
            {     // handle the initialization message
                return OnInitDialog();
            }

        case WM_COMMAND:
            {
                switch (LOWORD(wParam))
                {
                case pshHelp:
                    OnHelpButton();
                    return TRUE;

                }
                break;
            }
        }

        return 0;
    }


    // Display the Page Setup dialog, and allow the user to select various options.
    // An exception is thrown if the dialog isn't created.
    // An exception is thrown if there is no default printer.
    inline INT_PTR CPageSetupDialog::DoModal(HWND hWndOwner /* = 0 */)
    {
        assert(&GetApp());      // Test if Win32++ has been started
        assert(!IsWindow());    // Only one window per CWnd instance allowed
        
        // Ensure only one page-setup dialog is running at a time.
        CThreadLock Threadlock(GetApp().m_csPrintLock);

        // Update the default printer
        GetApp().UpdateDefaultPrinter();

        // Assign values to the PAGESETUPDLG structure
        m_PSD.hDevMode = GetApp().m_hDevMode;
        m_PSD.hDevNames = GetApp().m_hDevNames;
        m_PSD.hwndOwner = hWndOwner;

        // Ensure this thread has the TLS index set
        TLSData* pTLSData = GetApp().SetTlsData();

        // Create the modal dialog
        pTLSData->pWnd = this;
        BOOL ok = ::PageSetupDlg(&m_PSD);

        if (ok)
        {
            GetApp().m_hDevMode = m_PSD.hDevMode;
            GetApp().m_hDevNames = m_PSD.hDevNames;
            OnOK();
            ok = IDOK;
        }
        else
        {
            GlobalUnlock(GetApp().m_hDevMode);
            GlobalUnlock(GetApp().m_hDevNames);

            DWORD dwError = CommDlgExtendedError();
            if ((dwError != 0) && (dwError != CDERR_DIALOGFAILURE)) // ignore the exception caused by closing the dialog
            {
                // Reset global memory
                GlobalFreeAll();
                throw CWinException(_T("CPageSetupDialog::DoModal Failed"), dwError);
            }

            OnCancel();
            ok = IDCANCEL;
        }

        m_PSD.hDevMode = 0;
        m_PSD.hDevNames = 0;

        return ok;
    }


    // Returns a pointer to the locked hDevMode memory encapsulated in a CDevMode object.
    // There is no need to unlock this memory. The CDevMode object automatically
    // unlocks the memory when it goes out of scope.
    // Usage:
    //  CDevMode pDevMode = GetDevMode();
    //  Then use pDevMode as if it were a LPDEVMODE
    inline CDevMode CPageSetupDialog::GetDevMode() const
    {
        return CDevMode(GetApp().m_hDevMode);
    }


    // Returns a pointer to the locked hDevNames memory encapsulated in a CDevNames object.
    // There is no need to unlock this memory. The CDevNames object automatically
    // unlocks the memory when it goes out of scope.
    // Usage:
    //  CDevNames pDevNames = GetDevNames();
    //  Then use pDevNames as if it were a LPDEVNAMES
    inline CDevNames CPageSetupDialog::GetDevNames() const
    {
        return CDevNames(GetApp().m_hDevNames);
    }


    // Call this function after a call to DoModal to retrieve the margins of the printer.
    inline void CPageSetupDialog::GetMargins(RECT& rcMargin, RECT& rcMinMargin) const
    {
        rcMargin    = m_PSD.rtMargin;
        rcMinMargin = m_PSD.rtMinMargin;
    }


    // Call this function to retrieve the size of the paper selected for printing.
    inline CSize CPageSetupDialog::GetPaperSize() const
    {
        return CSize(m_PSD.ptPaperSize.x, m_PSD.ptPaperSize.y);
    }


    // This function receives messages that allow the drawing of the paint sample page
    // in the Page Setup dialog box to be customized.
    inline INT_PTR CALLBACK CPageSetupDialog::PaintHookProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        if (hWnd == 0)
            return 0;

        // The parent of paint sample window is the Page Setup dialog.
        CPageSetupDialog* pDlg = static_cast<CPageSetupDialog*>(GetCWndPtr(::GetParent(hWnd)));
        if (pDlg == NULL)
            return 0;

        switch (message)
        {
        case WM_PSD_PAGESETUPDLG:
            {
                assert(lParam);
                PAGESETUPDLG psd = *((LPPAGESETUPDLG)lParam);
                return pDlg->OnPreDrawPage(LOWORD(wParam), HIWORD(wParam), psd);
            }

        case WM_PSD_FULLPAGERECT:
        case WM_PSD_MINMARGINRECT:
        case WM_PSD_MARGINRECT:
        case WM_PSD_GREEKTEXTRECT:
        case WM_PSD_ENVSTAMPRECT:
        case WM_PSD_YAFULLPAGERECT:
            {
                assert(lParam);
                RECT rc = *((LPRECT)lParam);
                return pDlg->OnDrawPage(reinterpret_cast<HDC>(wParam), message, rc);
            }
        }
        return 0;
    }


    // Override this function to customize drawing of the sample page in the Page Setup dialog box.
    // It is called in response to the following messages: WM_PSD_FULLPAGERECT; WM_PSD_MINMARGINRECT;
    // WM_PSD_MARGINRECT; WM_PSD_GREEKTEXTRECT; WM_PSD_ENVSTAMPRECT; and WM_PSD_YAFULLPAGERECT.
    inline UINT CPageSetupDialog::OnDrawPage(HDC hDC, UINT nMessage, const RECT& /* rc*/)
    {
        UNREFERENCED_PARAMETER(hDC);
        UNREFERENCED_PARAMETER(nMessage);

        return 0; // do the default
    }


    // Called before drawing is preformed on the sample page.
    inline UINT CPageSetupDialog::OnPreDrawPage(WORD wPaper, WORD wFlags, const PAGESETUPDLG& /*PSD*/)
    {
        UNREFERENCED_PARAMETER(wPaper);
        UNREFERENCED_PARAMETER(wFlags);

        return 0;
    }


    // Set the parameters of the PAGESETUPDLG structure to sensible values
    inline void CPageSetupDialog::SetParameters(PAGESETUPDLG& psd)
    {
        m_PSD.lStructSize       = sizeof(m_PSD);
        m_PSD.hwndOwner         = 0;            // Set this in DoModal
        m_PSD.Flags             = psd.Flags;
        m_PSD.ptPaperSize       = psd.ptPaperSize;
        m_PSD.rtMinMargin       = psd.rtMinMargin;
        m_PSD.rtMargin          = psd.rtMargin;
        m_PSD.hInstance         = GetApp().GetResourceHandle();
        m_PSD.lCustData         = psd.lCustData;
        m_PSD.lpfnPageSetupHook = reinterpret_cast<LPCCHOOKPROC>(CDHookProc);
        m_PSD.lpfnPagePaintHook = reinterpret_cast<LPCCHOOKPROC>(CPageSetupDialog::PaintHookProc);
        m_PSD.lpPageSetupTemplateName = psd.lpPageSetupTemplateName;
        m_PSD.hPageSetupTemplate = psd.hPageSetupTemplate;
    }

}

#endif // _WIN32XX_PRINTDIALOGS_H_
