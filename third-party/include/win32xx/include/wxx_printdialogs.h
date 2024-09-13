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
// with the CGlobalLock class template.

// CPrintDialog displays the Print dialog, which allows the user to
// select the printer to use.

// CPageSetupDialog displays the PageSetup dialog, which allows the user
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
        virtual ~CPrintDialog() override;

        virtual INT_PTR DoModal(HWND owner = nullptr) override;
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
        // Override these functions as required.
        virtual INT_PTR DialogProc(UINT, WPARAM, LPARAM) override;

        // Not intended to be overridden.
        INT_PTR DialogProcDefault(UINT, WPARAM, LPARAM) override;

    private:
        CPrintDialog(const CPrintDialog&) = delete;
        CPrintDialog& operator=(const CPrintDialog&) = delete;

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
        virtual ~CPageSetupDialog() override {}

        virtual INT_PTR DoModal(HWND owner = nullptr) override;
        CDevMode GetDevMode() const;
        CDevNames GetDevNames() const;
        void    GetMargins(RECT& margin, RECT& minMargin) const;
        CSize   GetPaperSize() const;
        const   PAGESETUPDLG& GetParameters() const { return m_psd; }
        void    SetParameters(const PAGESETUPDLG& psd);

    protected:
        // Override these functions as required.
        virtual INT_PTR DialogProc(UINT, WPARAM, LPARAM) override;
        virtual UINT    OnDrawPage(HDC, UINT, const RECT&);
        virtual UINT    OnPreDrawPage(WORD paper, WORD flags, const PAGESETUPDLG& psd);

        // Not intended to be overridden.
        INT_PTR DialogProcDefault(UINT, WPARAM, LPARAM) override;
        static INT_PTR CALLBACK PaintHookProc(HWND, UINT, WPARAM, LPARAM);

    private:
        CPageSetupDialog(const CPageSetupDialog&) = delete;
        CPageSetupDialog& operator=(const CPageSetupDialog&) = delete;

        PAGESETUPDLG    m_psd;          // page setup dlg structure
    };

}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

namespace Win32xx
{
    /////////////////////////////////////////
    // Definitions for the CPrintDialog class
    //

    // Constructor for CPrintDialog class. The flags parameter specifies the
    // flags for the PRINTDLG structure. Refer to the description of the
    // PRINTDLG struct in the Windows API documentation.
    inline CPrintDialog::CPrintDialog(DWORD flags /* = PD_ALLPAGES | PD_USEDEVMODECOPIES | PD_NOPAGENUMS | PD_HIDEPRINTTOFILE | PD_NOSELECTION */)
    {
        // Initialize the PRINTDLG member
        m_pd = {};
        m_pd.Flags = flags;
        m_pd.Flags &= ~PD_RETURNIC;
        m_pd.Flags &= ~PD_RETURNDC;   // Use GetPrinterDC to retrieve the dc.

        // Enable the hook proc for the help button.
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
        CDC dc;
        if (GetApp()->GetHDevNames().Get() == nullptr)
            GetApp()->UpdateDefaultPrinter();

        if ((GetApp()->GetHDevNames().Get() != nullptr) && (GetApp()->GetHDevMode().Get() != nullptr))
        {
            dc.CreateDC(GetDriverName(), GetDeviceName(),
                GetPortName(), GetDevMode());
        }

        if (dc.GetHDC() == nullptr)
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

        // Always pass unhandled messages on to DialogProcDefault.
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
    inline INT_PTR CPrintDialog::DoModal( HWND owner /* = nullptr */)
    {
        assert(!IsWindow());    // Only one window per CWnd instance allowed

        // Update the default printer
        GetApp()->UpdateDefaultPrinter();

        // Assign values to the PRINTDLG structure
        m_pd.hDevMode = GetApp()->GetHDevMode();
        m_pd.hDevNames = GetApp()->GetHDevNames();
        m_pd.hwndOwner = owner;

        // Retrieve this thread's TLS data
        TLSData* pTLSData = GetApp()->GetTlsData();

        // Create the modal dialog
        pTLSData->pWnd = this;

        // invoke the control and save the result
        BOOL ok = ::PrintDlg(&m_pd);

        if (ok)
        {
            GetApp()->UpdatePrinterMemory(m_pd.hDevMode, m_pd.hDevNames);
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
                GetApp()->ResetPrinterMemory();
                throw CWinException(GetApp()->MsgWndDialog(), error);
            }

            OnCancel();
            ok = IDCANCEL;
        }

        m_pd.hDevMode = nullptr;
        m_pd.hDevNames = nullptr;

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
        GetApp()->ResetPrinterMemory();
        GetApp()->UpdateDefaultPrinter();

        // Return TRUE if default printer exists
        return (GetApp()->GetHDevNames().Get()) ? TRUE : FALSE;
    }

    // Retrieves the name of the default or currently selected printer device.
    inline CString CPrintDialog::GetDeviceName() const
    {

        if (GetApp()->GetHDevNames().Get() == nullptr)
            GetApp()->UpdateDefaultPrinter();

        CString str;
        if (GetApp()->GetHDevNames().Get() != nullptr)
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

        if (GetApp()->GetHDevMode().Get() == nullptr)
            GetApp()->UpdateDefaultPrinter();

        if (GetApp()->GetHDevMode().Get() == nullptr)
            throw CResourceException(GetApp()->MsgPrintFound());

        return CDevMode(GetApp()->GetHDevMode());
    }

    // Returns a pointer to the locked hDevNames memory encapsulated in a CDevNames object.
    // There is no need to unlock this memory. The CDevNames object automatically
    // unlocks the memory when it goes out of scope.
    // Usage:
    //  CDevNames pDevNames = GetDevNames();
    //  Then use pDevNames as if it were a LPDEVNAMES.
    inline CDevNames CPrintDialog::GetDevNames() const
    {
        if (GetApp()->GetHDevNames().Get() == nullptr)
            GetApp()->UpdateDefaultPrinter();

        if (GetApp()->GetHDevNames().Get() == nullptr)
            throw CResourceException(GetApp()->MsgPrintFound());

        return CDevNames(GetApp()->GetHDevNames());
    }

    // Retrieves the name of the default or currently selected printer driver.
    inline CString CPrintDialog::GetDriverName() const
    {
        if (GetApp()->GetHDevNames().Get() == nullptr)
            GetApp()->UpdateDefaultPrinter();

        CString str;
        if (GetApp()->GetHDevNames().Get() != nullptr)
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
        if (GetApp()->GetHDevNames().Get() == nullptr)
            GetApp()->UpdateDefaultPrinter();

        CString str;
        if (GetApp()->GetHDevNames().Get() != nullptr)
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
    // specified by hDevMode and hDevNames.
    inline void CPrintDialog::SetDefaults(HGLOBAL hDevMode, HGLOBAL hDevNames)
    {
        GetApp()->UpdatePrinterMemory(hDevMode, hDevNames);
    }

    // Set the parameters of the PRINTDLG structure to sensible values
    inline void CPrintDialog::SetParameters(const PRINTDLG& pd)
    {
        m_pd.lStructSize    = sizeof(m_pd);
        m_pd.hwndOwner      = nullptr;            // Set this in DoModal.
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
        m_pd.Flags &= ~PD_RETURNDC;   // Use GetPrinterDC to retrieve the dc.
    }


    //////////////////////////////////////////////
    // Definitions for the CPageSetupDialog class.
    //


    // Constructor for CPageSetupDialog class. The flags parameter specifies the
    // flags for the PAGESETUPDLG structure. Refer to the description of the
    // PAGESETUPDLG struct in the Windows API documentation.
    inline CPageSetupDialog::CPageSetupDialog( DWORD flags /* = PSD_MARGINS */ )
    {
        m_psd = {};
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

        // Always pass unhandled messages on to DialogProcDefault.
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
            {   // Handle the initialization message.
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
    inline INT_PTR CPageSetupDialog::DoModal(HWND owner /* = nullptr */)
    {
        assert(!IsWindow());    // Only one window per CWnd instance allowed.

        // Update the default printer.
        GetApp()->UpdateDefaultPrinter();

        // Assign values to the PAGESETUPDLG structure
        m_psd.hDevMode = GetApp()->GetHDevMode();
        m_psd.hDevNames = GetApp()->GetHDevNames();
        m_psd.hwndOwner = owner;

        // Retrieve this thread's TLS data.
        TLSData* pTLSData = GetApp()->GetTlsData();

        // Create the modal dialog.
        pTLSData->pWnd = this;
        BOOL ok = ::PageSetupDlg(&m_psd);

        if (ok)
        {
            GetApp()->UpdatePrinterMemory(m_psd.hDevMode, m_psd.hDevNames);
            OnOK();
            ok = IDOK;
        }
        else
        {
            int error = static_cast<int>(CommDlgExtendedError());
            if ((error != 0) && (error != CDERR_DIALOGFAILURE)) // ignore the exception caused by closing the dialog
            {
                // Reset global memory
                GetApp()->ResetPrinterMemory();
                throw CWinException(GetApp()->MsgWndDialog(), error);
            }

            OnCancel();
            ok = IDCANCEL;
        }

        m_psd.hDevMode = nullptr;
        m_psd.hDevNames = nullptr;

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
        if (GetApp()->GetHDevMode().Get() == nullptr)
            GetApp()->UpdateDefaultPrinter();

        if (GetApp()->GetHDevMode().Get() == nullptr)
            throw CResourceException(GetApp()->MsgPrintFound());

        return CDevMode(GetApp()->GetHDevMode());
    }

    // Returns a pointer to the locked hDevNames memory encapsulated in a CDevNames object.
    // There is no need to unlock this memory. The CDevNames object automatically
    // unlocks the memory when it goes out of scope.
    // Usage:
    //  CDevNames pDevNames = GetDevNames();
    //  Then use pDevNames as if it were a LPDEVNAMES
    inline CDevNames CPageSetupDialog::GetDevNames() const
    {
        if (GetApp()->GetHDevNames().Get() == nullptr)
            GetApp()->UpdateDefaultPrinter();

        if (GetApp()->GetHDevNames().Get() == nullptr)
            throw CResourceException(GetApp()->MsgPrintFound());

        return CDevNames(GetApp()->GetHDevNames());
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
        if (wnd == nullptr)
            return 0;

        // The parent of paint sample window is the Page Setup dialog.
        CPageSetupDialog* pDlg = static_cast<CPageSetupDialog*>(GetCWndPtr(::GetParent(wnd)));
        if (pDlg == nullptr)
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
        m_psd.hwndOwner         = nullptr;            // Set this in DoModal
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
