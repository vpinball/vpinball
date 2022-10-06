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


////////////////////////////////////////////////////////
// Acknowledgement:
//
// Developed from code originally provided by:
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

// CPageSetupDialog displays the PageSetup dialog which allows the user
// to select the page settings.

// The CPrintDialog and CPageSetupDialog classes share global memory
// for their hDevMode and hDevNames handles. This global memory
// persists after the CPrintDialog and CPageSetupDialog objects are
// destructed, and is managed by CWinApp. There is no need for the
// user to manually free the hDevMode and hDevNames memory.

// CDevMode and CDevNames are typedefs for the CGlobalLock class. They
// provide self unlocking pointers to the hDevMode and hDevNames memory.
// They are used as follows:
//   CPrintDialog printDialog;
//   CDevNames pDevNames = printDialog.GetDevNames();
//   CDevMode  pDevMode  = printDialog.GetDevMode();
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
// NOTE: Use the following to retrieve the printer's device context
// for the default or currently selected printer:
//     CPrintDialog printDialog;
//     CDC printerDC = printDialog.GetDefaults();
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
    //  See:  https://www.codeproject.com/Articles/16692/WebControls/
    //
    //////////////////////////////////////////////////////////////////


    //////////////////////////////////////
    // CGlobaLock is a template class used to provide a self unlocking
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
        CGlobalLock() : m_h(0), m_p(0)  {}
        CGlobalLock(HANDLE h) : m_h(h)  { Lock(); }
        CGlobalLock(const CGlobalLock& rhs)
        {
            m_h = rhs.m_h;
            m_p = rhs.m_p;
        }
        ~CGlobalLock()                  { Unlock(); }
        CGlobalLock& operator= (const CGlobalLock& rhs)
        {
            m_h = rhs.m_h;
            m_p = rhs.m_p;
            return *this;
        }

        T* Get() const          { return m_p; }                 // Returns the pointer
        LPCTSTR c_str() const;                                  // Returns the LPCTSTR (for DEVNAMES only)
        CString GetDeviceName() const;                          // Returns the printer name (for DEVNAMES only)
        CString GetDriverName() const;                          // Returns the printer driver (for DEVNAMES only)
        CString GetPortName() const;                            // Returns the printer port (for DEVNAMES only)
        bool    IsDefaultPrinter() const;                       // Returns true if this is the default printer (for DEVNAMES only)

        // operator overloads
        operator T*() const     { return m_p; }                 // Conversion operator to pointer
        T* operator->() const   { assert(m_p); return m_p; }    // Pointer operator
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
                    throw CWinException(GetApp()->MsgWndGlobalLock());
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
    inline LPCTSTR CGlobalLock<DEVNAMES>::c_str() const
    {
        assert(m_p != NULL);
        return reinterpret_cast<LPCTSTR>(m_p);
    }


    template<>
    inline CString CGlobalLock<DEVNAMES>::GetDeviceName() const
    {
        return (m_p != NULL)? c_str() + (*this)->wDeviceOffset : _T("");
    }

    template<>
    inline CString CGlobalLock<DEVNAMES>::GetDriverName() const
    {
        return (m_p != NULL)? c_str() + (*this)->wDriverOffset : _T("");
    }

    template<>
    inline CString CGlobalLock<DEVNAMES>::GetPortName() const
    {
        return (m_p != NULL)? c_str() + (*this)->wOutputOffset : _T("");
    }

    template<>
    inline bool CGlobalLock<DEVNAMES>::IsDefaultPrinter() const
    {
        return (m_p != NULL)? ((*this)->wDefault & DN_DEFAULTPRN) : false;
    }



    //////////////////////////////////////////////////////////////////////
    // A set of typedefs to simplify the use of CGlobalLock.
    // These provide self unlocking objects which can be used for pointers
    // to global memory. Using these typedefs eliminate the need to manually
    // lock or unlock the global memory handles.
    // Note: In the examples below, hDevMode and hDevNames can be either a raw
    //       global memory handle, or a CHGlobal object.
    //
    // Example usage:
    //   CDevMode  pDevMode(hDevMode);      // and use pDevMode as if it were a LPDEVMODE
    //   CDevNames pDevNames(hDevNames);    // and use pDevNames as if it were a LPDEVNAMES
    //   assert(pDevNames.Get());           // Get can be used to access the underlying pointer
    //   CDevNames(hDevNames).GetDeviceName // Returns a CString containing the device name.
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
        CPrintDialog(DWORD flags = PD_ALLPAGES | PD_USEDEVMODECOPIES | PD_NOPAGENUMS |
                                        PD_HIDEPRINTTOFILE | PD_NOSELECTION );
        virtual ~CPrintDialog();

        virtual INT_PTR DoModal(HWND owner = 0);
        int     GetCopies() const;
        BOOL    GetDefaults();
        CDevMode GetDevMode() const;
        CDevNames GetDevNames() const;
        CString GetDeviceName() const;
        CString GetDriverName() const;
        int     GetFromPage() const;
        const   PRINTDLG& GetParameters()  const { return m_pd; }
        CString GetPortName() const;
        CDC     GetPrinterDC() const;
        int     GetToPage() const;
        BOOL    PrintAll() const;
        BOOL    PrintCollate() const;
        BOOL    PrintRange() const;
        BOOL    PrintSelection() const;
        void    SetDefaults(HGLOBAL hDevMode, HGLOBAL hDevNames);
        void    SetParameters(const PRINTDLG& pd);

    protected:
        // Override these functions as required
        virtual INT_PTR DialogProc(UINT, WPARAM, LPARAM);

        // Not intended to be overridden
        INT_PTR DialogProcDefault(UINT, WPARAM, LPARAM);

    private:
        CPrintDialog(const CPrintDialog&);              // Disable copy construction
        CPrintDialog& operator = (const CPrintDialog&); // Disable assignment operator

        PRINTDLG        m_pd;           // printer selection dlg structure
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
        CPageSetupDialog( DWORD flags = PSD_MARGINS );
        virtual ~CPageSetupDialog() {}

        virtual INT_PTR DoModal(HWND owner = 0);
        CDevMode GetDevMode() const;
        CDevNames GetDevNames() const;
        void    GetMargins(RECT& margin, RECT& minMargin) const;
        CSize   GetPaperSize() const;
        const   PAGESETUPDLG& GetParameters() const { return m_psd; }
        void    SetParameters(const PAGESETUPDLG& psd);

    protected:
        // Override these functions as required
        virtual INT_PTR DialogProc(UINT, WPARAM, LPARAM);
        virtual UINT    OnDrawPage(HDC, UINT, const RECT&);
        virtual UINT    OnPreDrawPage(WORD paper, WORD flags, const PAGESETUPDLG& psd);

        // Not intended to be overridden
        INT_PTR DialogProcDefault(UINT, WPARAM, LPARAM);
        static INT_PTR CALLBACK PaintHookProc(HWND, UINT, WPARAM, LPARAM);

    private:
        CPageSetupDialog(const CPageSetupDialog&);              // Disable copy construction
        CPageSetupDialog& operator = (const CPageSetupDialog&); // Disable assignment operator

        PAGESETUPDLG    m_psd;          // page setup dlg structure
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
        if (m_devNames.Get() == 0)
        {
            // Allocate global printer memory by specifying the default printer.
            CPrintDialog pd;
            pd.GetDefaults();
        }
        else
        {
            // Global memory has already been allocated
            if (CDevNames(m_devNames).IsDefaultPrinter())
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
                    m_devMode.Free();
                    m_devNames.Free();
                }
                else
                {
                    // Compare current default printer to the one in global memory
                    if (CDevNames(m_devNames).GetDeviceName() != CDevNames(pd.hDevNames).GetDeviceName() ||
                        CDevNames(m_devNames).GetDriverName() != CDevNames(pd.hDevNames).GetDriverName() ||
                        CDevNames(m_devNames).GetPortName()   != CDevNames(pd.hDevNames).GetPortName())
                    {
                        // Default printer has changed. Reset the global memory.
                        m_devMode.Free();
                        m_devNames.Free();
                        m_devMode.Reassign(pd.hDevMode);
                        m_devNames.Reassign(pd.hDevNames);
                    }
                    else
                    {
                        ::GlobalFree(pd.hDevMode);
                        ::GlobalFree(pd.hDevNames);
                    }
                }
            }
        }
    }

    /////////////////////////////////////////
    // Definitions for the CPrintDialog class
    //


    // Constructor for CPrintDialog class. The flags parameter specifies the
    // flags for the PRINTDLG structure. Refer to the description of the
    // PRINTDLG struct in the Windows API documentation.
    inline CPrintDialog::CPrintDialog(DWORD flags /* = PD_ALLPAGES | PD_USEDEVMODECOPIES | PD_NOPAGENUMS | PD_HIDEPRINTTOFILE | PD_NOSELECTION */)
    {
        // initialize the PRINTDLG member
        ZeroMemory(&m_pd, sizeof(m_pd));

        m_pd.Flags = flags;
        m_pd.Flags &= ~PD_RETURNIC;
        m_pd.Flags &= ~PD_RETURNDC;   // use GetPrinterDC to retrieve the dc.

        // Enable the hook proc for the help button
        if (m_pd.Flags & PD_SHOWHELP)
            m_pd.Flags |= PD_ENABLEPRINTHOOK;

        SetParameters(m_pd);
    }

    inline CPrintDialog::~CPrintDialog()
    {
    }

    // Returns the device context of the default or currently chosen printer.
    // Throws on failure.
    inline CDC CPrintDialog::GetPrinterDC() const
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

    // Dialog procedure for the Print dialog. Override this function
    // to customize the message handling.
    inline INT_PTR CPrintDialog::DialogProc(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        //  A typical override might look like this:

        //  switch (msg)
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
        return DialogProcDefault(msg, wparam, lparam);
    }

    //  The default message handling for CPrintDialog. Don't override this
    //  function, override DialogProc instead.
    //  Note: OnCancel and OnOK are called by DoModal.
    inline INT_PTR CPrintDialog::DialogProcDefault(UINT msg, WPARAM wparam, LPARAM)
    {
        switch (msg)
        {
            case WM_INITDIALOG:
            {     // handle the initialization message
                return OnInitDialog();
            }

            case WM_COMMAND:
            {
                switch (LOWORD(wparam))
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
    inline INT_PTR CPrintDialog::DoModal( HWND owner /* = 0 */)
    {
        assert(!IsWindow());    // Only one window per CWnd instance allowed

        // Ensure only one print dialog is running at a time.
        CThreadLock lock(GetApp()->m_printLock);

        // Update the default printer
        GetApp()->UpdateDefaultPrinter();

        // Assign values to the PRINTDLG structure
        m_pd.hDevMode = GetApp()->m_devMode;
        m_pd.hDevNames = GetApp()->m_devNames;
        m_pd.hwndOwner = owner;

        // Retrieve this thread's TLS data
        TLSData* pTLSData = GetApp()->GetTlsData();

        // Create the modal dialog
        pTLSData->pWnd = this;

        // invoke the control and save the result
        BOOL ok = ::PrintDlg(&m_pd);

        if (ok)
        {
            GetApp()->m_devMode.Reassign(m_pd.hDevMode);
            GetApp()->m_devNames.Reassign(m_pd.hDevNames);
            OnOK();
            ok = IDOK;
        }
        else
        {
            int error = static_cast<int>(CommDlgExtendedError());
            if ((error != 0) && (error != CDERR_DIALOGFAILURE))
            // ignore the error caused by closing the dialog
            {
                // Reset global memory
                GetApp()->m_devMode.Free();
                GetApp()->m_devNames.Free();
                throw CWinException(GetApp()->MsgWndDialog(), error);
            }

            OnCancel();
            ok = IDCANCEL;
        }

        m_pd.hDevMode = 0;
        m_pd.hDevNames = 0;

        // Prepare the CWnd for reuse.
        Cleanup();

        return ok;
    }

    // Retrieves the number of copies requested.
    inline int CPrintDialog::GetCopies() const
    {
        if (m_pd.Flags & PD_USEDEVMODECOPIES)
            return GetDevMode()->dmCopies;
        else
            return m_pd.nCopies;
    }

    // Sets the printer and the page settings to default, without displaying a dialog.
    // The hDevMode and hDevNames memory is freed and reallocated.
    // Returns TRUE if a default printer exists.
    inline BOOL CPrintDialog::GetDefaults()
    {
        CThreadLock lock(GetApp()->m_printLock);

        if (m_pd.hDC)
        {
            ::DeleteDC(m_pd.hDC);
            m_pd.hDC = 0;
        }

        m_pd.Flags |= PD_RETURNDEFAULT;
        ::PrintDlg(&m_pd);
        m_pd.Flags &= ~PD_RETURNDEFAULT;

        // Reset global memory
        SetDefaults(m_pd.hDevMode, m_pd.hDevNames);

        m_pd.hDevMode = 0;
        m_pd.hDevNames = 0;

        // Return TRUE if default printer exists
        return (GetApp()->m_devNames.Get()) ? TRUE : FALSE;
    }

    // Retrieves the name of the default or currently selected printer device.
    inline CString CPrintDialog::GetDeviceName() const
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
    inline CDevMode CPrintDialog::GetDevMode() const
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
    inline CDevNames CPrintDialog::GetDevNames() const
    {
        CThreadLock lock(GetApp()->m_printLock);

        if (GetApp()->m_devNames.Get() == 0)
            GetApp()->UpdateDefaultPrinter();

        if (GetApp()->m_devNames.Get() == 0)
            throw CResourceException(GetApp()->MsgPrintFound());

        return CDevNames(GetApp()->m_devNames);
    }

    // Retrieves the name of the default or currently selected printer driver.
    inline CString CPrintDialog::GetDriverName() const
    {
        CThreadLock lock(GetApp()->m_printLock);

        if (GetApp()->m_devNames.Get() == 0)
            GetApp()->UpdateDefaultPrinter();

        CString str;
        if (GetApp()->m_devNames.Get() != 0)
            str = GetDevNames().GetDriverName();

        return str;
    }

    // Retrieves the starting page of the print range.
    inline int CPrintDialog::GetFromPage() const
    {
        return (PrintRange() ? m_pd.nFromPage : -1);
    }

    // Retrieves the name of the default or currently selected printer port.
    inline CString CPrintDialog::GetPortName() const
    {
        CThreadLock lock(GetApp()->m_printLock);

        if (GetApp()->m_devNames.Get() == 0)
            GetApp()->UpdateDefaultPrinter();

        CString str;
        if (GetApp()->m_devNames.Get() != 0)
            str = GetDevNames().GetPortName();

        return str;
    }

    // Retrieves the ending page of the print range.
    inline int CPrintDialog::GetToPage() const
    {
        return (PrintRange() ? m_pd.nToPage : -1);
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
        if (m_pd.Flags & PD_USEDEVMODECOPIES)
            return (GetDevMode()->dmCollate == DMCOLLATE_TRUE);
        else
            return (m_pd.Flags & PD_COLLATE ? TRUE : FALSE);
    }

    // Call this function after calling DoModal to determine whether to print
    // only a range of pages in the document.
    inline BOOL CPrintDialog::PrintRange() const
    {
        return m_pd.Flags & PD_PAGENUMS ? TRUE : FALSE;
    }

    // Call this function after calling DoModal to determine whether to print
    // only the currently selected items.
    inline BOOL CPrintDialog::PrintSelection() const
    {
        return m_pd.Flags & PD_SELECTION ? TRUE : FALSE;
    }

    // Assigns the application's default printer settings to the values
    // specified by hDevMode and hDevNames. CWinApp now owns the global
    // memory and is responsible for freeing it.
    inline void CPrintDialog::SetDefaults(HGLOBAL hDevMode, HGLOBAL hDevNames)
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

    // Set the parameters of the PRINTDLG structure to sensible values
    inline void CPrintDialog::SetParameters(const PRINTDLG& pd)
    {
        m_pd.lStructSize    = sizeof(m_pd);
        m_pd.hwndOwner      = 0;            // Set this in DoModal
        m_pd.Flags          = pd.Flags;
        m_pd.nFromPage      = pd.nFromPage;
        m_pd.nToPage        = pd.nToPage;
        m_pd.nMinPage       = pd.nMinPage;
        m_pd.nMaxPage       = pd.nMaxPage;
        m_pd.nCopies        = pd.nCopies;
        m_pd.hInstance      = GetApp()->GetResourceHandle();
        m_pd.lpfnPrintHook  = reinterpret_cast<LPCCHOOKPROC>(CDHookProc);
        m_pd.lpfnSetupHook  = reinterpret_cast<LPCCHOOKPROC>(CDHookProc);
        m_pd.lCustData      = pd.lCustData;
        m_pd.hPrintTemplate = pd.hPrintTemplate;
        m_pd.hSetupTemplate = pd.hSetupTemplate;
        m_pd.lpPrintTemplateName = pd.lpPrintTemplateName;
        m_pd.lpSetupTemplateName = pd.lpSetupTemplateName;
        m_pd.Flags &= ~PD_RETURNIC;
        m_pd.Flags &= ~PD_RETURNDC;   // use GetPrinterDC to retrieve the dc.
    }


    /////////////////////////////////////////////
    // Definitions for the CPageSetupDialog class
    //


    // Constructor for CPageSetupDialog class. The flags parameter specifies the
    // flags for the PAGESETUPDLG structure. Refer to the description of the
    // PAGESETUPDLG struct in the Windows API documentation.
    inline CPageSetupDialog::CPageSetupDialog( DWORD flags /* = PSD_MARGINS */ )
    {
        ZeroMemory(&m_psd, sizeof(m_psd));
        m_psd.Flags = flags;

        // Enable the hook proc for the help button
        if (m_psd.Flags & PSD_SHOWHELP)
            m_psd.Flags |= PSD_ENABLEPAGESETUPHOOK;

        SetParameters(m_psd);
    }

    // Dialog procedure for the PageSetup dialog. Override this function
    // to customize the message handling.
    inline INT_PTR CPageSetupDialog::DialogProc(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        //  A typical override might look like this:

        //  switch (msg)
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
        return DialogProcDefault(msg, wparam, lparam);
    }

    //  The Default message handling for CPageSetupDialog. Don't override this
    //  function, override DialogProc instead.
    //  Note: OnCancel and OnOK are called by DoModal.
    inline INT_PTR CPageSetupDialog::DialogProcDefault(UINT msg, WPARAM wparam, LPARAM)
    {
        switch (msg)
        {
        case WM_INITDIALOG:
            {   // handle the initialization message
                return OnInitDialog();
            }

        case WM_COMMAND:
            {
                switch (LOWORD(wparam))
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
    inline INT_PTR CPageSetupDialog::DoModal(HWND owner /* = 0 */)
    {
        assert(!IsWindow());    // Only one window per CWnd instance allowed

        // Ensure only one page-setup dialog is running at a time.
        CThreadLock lock(GetApp()->m_printLock);

        // Update the default printer
        GetApp()->UpdateDefaultPrinter();

        // Assign values to the PAGESETUPDLG structure
        m_psd.hDevMode = GetApp()->m_devMode;
        m_psd.hDevNames = GetApp()->m_devNames;
        m_psd.hwndOwner = owner;

        // Retrieve this thread's TLS data
        TLSData* pTLSData = GetApp()->GetTlsData();

        // Create the modal dialog
        pTLSData->pWnd = this;
        BOOL ok = ::PageSetupDlg(&m_psd);

        if (ok)
        {
            GetApp()->m_devMode.Reassign(m_psd.hDevMode);
            GetApp()->m_devNames.Reassign(m_psd.hDevNames);
            OnOK();
            ok = IDOK;
        }
        else
        {
            int error = static_cast<int>(CommDlgExtendedError());
            if ((error != 0) && (error != CDERR_DIALOGFAILURE)) // ignore the exception caused by closing the dialog
            {
                // Reset global memory
                GetApp()->m_devMode.Free();
                GetApp()->m_devNames.Free();
                throw CWinException(GetApp()->MsgWndDialog(), error);
            }

            OnCancel();
            ok = IDCANCEL;
        }

        m_psd.hDevMode = 0;
        m_psd.hDevNames = 0;

        // Prepare the CWnd for reuse.
        Cleanup();

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
    inline CDevNames CPageSetupDialog::GetDevNames() const
    {
        CThreadLock lock(GetApp()->m_printLock);

        if (GetApp()->m_devNames.Get() == 0)
            GetApp()->UpdateDefaultPrinter();

        if (GetApp()->m_devNames.Get() == 0)
            throw CResourceException(GetApp()->MsgPrintFound());

        return CDevNames(GetApp()->m_devNames);
    }

    // Call this function after a call to DoModal to retrieve the margins of the printer.
    inline void CPageSetupDialog::GetMargins(RECT& margin, RECT& minMargin) const
    {
        margin    = m_psd.rtMargin;
        minMargin = m_psd.rtMinMargin;
    }

    // Call this function to retrieve the size of the paper selected for printing.
    inline CSize CPageSetupDialog::GetPaperSize() const
    {
        return CSize(m_psd.ptPaperSize.x, m_psd.ptPaperSize.y);
    }

    // This function receives messages that allow the drawing of the paint sample page
    // in the Page Setup dialog box to be customized.
    inline INT_PTR CALLBACK CPageSetupDialog::PaintHookProc(HWND wnd, UINT message, WPARAM wparam, LPARAM lparam)
    {
        if (wnd == 0)
            return 0;

        // The parent of paint sample window is the Page Setup dialog.
        CPageSetupDialog* pDlg = static_cast<CPageSetupDialog*>(GetCWndPtr(::GetParent(wnd)));
        if (pDlg == NULL)
            return 0;

        switch (message)
        {
        case WM_PSD_PAGESETUPDLG:
            {
                assert(lparam);
                if (lparam == 0) return 0;
                PAGESETUPDLG psd = *((LPPAGESETUPDLG)lparam);
                return static_cast<INT_PTR>(pDlg->OnPreDrawPage(LOWORD(wparam), HIWORD(wparam), psd));
            }

        case WM_PSD_FULLPAGERECT:
        case WM_PSD_MINMARGINRECT:
        case WM_PSD_MARGINRECT:
        case WM_PSD_GREEKTEXTRECT:
        case WM_PSD_ENVSTAMPRECT:
        case WM_PSD_YAFULLPAGERECT:
            {
                assert(lparam);
                if (lparam == 0) return 0;
                RECT rc = *((LPRECT)lparam);
                return static_cast<INT_PTR>(pDlg->OnDrawPage(reinterpret_cast<HDC>(wparam), message, rc));
            }
        }
        return 0;
    }

    // Override this function to customize drawing of the sample page in the Page Setup dialog box.
    // It is called in response to the following messages: WM_PSD_FULLPAGERECT; WM_PSD_MINMARGINRECT;
    // WM_PSD_MARGINRECT; WM_PSD_GREEKTEXTRECT; WM_PSD_ENVSTAMPRECT; and WM_PSD_YAFULLPAGERECT.
    inline UINT CPageSetupDialog::OnDrawPage(HDC, UINT, const RECT&)
    {
        return 0; // do the default
    }

    // Called before drawing is preformed on the sample page.
    inline UINT CPageSetupDialog::OnPreDrawPage(WORD /*paper*/, WORD /*flags*/, const PAGESETUPDLG& /*psd*/)
    {
        return 0;
    }

    // Set the parameters of the PAGESETUPDLG structure to sensible values
    inline void CPageSetupDialog::SetParameters(const PAGESETUPDLG& psd)
    {
        m_psd.lStructSize       = sizeof(m_psd);
        m_psd.hwndOwner         = 0;            // Set this in DoModal
        m_psd.Flags             = psd.Flags;
        m_psd.ptPaperSize       = psd.ptPaperSize;
        m_psd.rtMinMargin       = psd.rtMinMargin;
        m_psd.rtMargin          = psd.rtMargin;
        m_psd.hInstance         = GetApp()->GetResourceHandle();
        m_psd.lCustData         = psd.lCustData;
        m_psd.lpfnPageSetupHook = reinterpret_cast<LPCCHOOKPROC>(CDHookProc);
        m_psd.lpfnPagePaintHook = reinterpret_cast<LPCCHOOKPROC>(CPageSetupDialog::PaintHookProc);
        m_psd.lpPageSetupTemplateName = psd.lpPageSetupTemplateName;
        m_psd.hPageSetupTemplate = psd.hPageSetupTemplate;
    }

}

#endif // _WIN32XX_PRINTDIALOGS_H_
