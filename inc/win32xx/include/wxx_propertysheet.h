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
// wxx_propertysheet.h
//  Declaration of the following classes:
//  CPropertyPage and CPropertySheet

// These classes add support for property sheets to Win32++. A property sheet
// will have one or more property pages. These pages are much like dialogs
// which are presented within a tabbed dialog or within a wizard. The data
// on a property page can be validated before the next page is presented.
// Property sheets have three modes of use: Modal, Modeless, and Wizard.
//
// Refer to the PropertySheet demo program for an example of how property sheets
// can be used.


#ifndef _WIN32XX_PROPERTYSHEET_H_
#define _WIN32XX_PROPERTYSHEET_H_

#include "wxx_dialog.h"

#define ID_APPLY_NOW   0x3021
#define ID_WIZBACK     0x3023
#define ID_WIZNEXT     0x3024
#define ID_WIZFINISH   0x3025
#define ID_HELP        0xE146

#ifndef PROPSHEETHEADER_V1_SIZE
 #define PROPSHEETHEADER_V1_SIZE 40
#endif

namespace Win32xx
{
    class CPropertyPage;
    typedef Shared_Ptr<CPropertyPage> PropertyPagePtr;


    // This class provides support for property pages. A property page is an
    // individual page used in a property sheet.
    class CPropertyPage : public CDialog
    {
    public:
        CPropertyPage (UINT nIDTemplate, LPCTSTR szTitle = NULL);
        virtual ~CPropertyPage() {}

        virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
        virtual BOOL OnApply();
        virtual void OnCancel();
        virtual void OnHelp();
        virtual BOOL OnInitDialog();
        virtual BOOL OnKillActive();
        virtual LRESULT OnNotify(WPARAM wParam, LPARAM lParam);
        virtual void OnOK();
        virtual BOOL OnQueryCancel();
        virtual BOOL OnQuerySiblings(UINT uMsg, WPARAM wParam, LPARAM lParam);
        virtual void OnReset();
        virtual BOOL OnSetActive();
        virtual BOOL OnWizardBack();
        virtual BOOL OnWizardFinish();
        virtual BOOL OnWizardNext();
        virtual BOOL PreTranslateMessage(MSG& Msg);

        void CancelToClose() const;
        INT_PTR DialogProcDefault(UINT uMsg, WPARAM wParam, LPARAM lParam);
        PROPSHEETPAGE GetPSP() const {return m_PSP;}
        BOOL IsButtonEnabled(int iButton) const;
        LRESULT QuerySiblings(WPARAM wParam, LPARAM lParam) const;
        void SetModified(BOOL IsChanged) const;
        void SetTitle(LPCTSTR szTitle);
        void SetWizardButtons(DWORD dwFlags) const;


    private:
        CPropertyPage(const CPropertyPage&);                // Disable copy construction
        CPropertyPage& operator = (const CPropertyPage&);   // Disable assignment operator

        static UINT CALLBACK StaticPropSheetPageProc(HWND hwnd, UINT uMsg, LPPROPSHEETPAGE ppsp);
        static INT_PTR CALLBACK StaticDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

        PROPSHEETPAGE m_PSP;
        CString m_Title;
    };


    // This class provides support for a property sheet. A property sheet is
    // also known as a tab dialog box. It has one or more property pages.
    class CPropertySheet : public CWnd
    {
    public:
        CPropertySheet(UINT nIDCaption, HWND hParent = 0);
        CPropertySheet(LPCTSTR pszCaption = NULL, HWND hParent = 0);
        virtual ~CPropertySheet() {}

        // Operations
        virtual CPropertyPage* AddPage(CPropertyPage* pPage);
        virtual HWND Create(HWND hParent = 0);
        virtual INT_PTR CreatePropertySheet(LPCPROPSHEETHEADER ppsph);
        virtual void DestroyButton(int iButton);
        virtual void Destroy();
        virtual int DoModal();
        virtual void RemovePage(CPropertyPage* pPage);

        // State functions
        BOOL IsModeless() const;
        BOOL IsWizard() const;

        //Attributes
        CPropertyPage* GetActivePage() const;
        int GetPageCount() const;
        int GetPageIndex(CPropertyPage* pPage) const;
        HWND GetTabControl() const;
        virtual BOOL SetActivePage(int nPage);
        virtual BOOL SetActivePage(CPropertyPage* pPage);
        virtual void SetIcon(UINT idIcon);
        virtual void SetTitle(LPCTSTR szTitle);
        virtual void SetWizardMode(BOOL IsWizard);

    protected:
        virtual BOOL PreTranslateMessage(MSG& Msg);


    private:
        CPropertySheet(const CPropertySheet&);              // Disable copy construction
        CPropertySheet& operator = (const CPropertySheet&); // Disable assignment operator
        void BuildPageArray();
        static void CALLBACK Callback(HWND hwnd, UINT uMsg, LPARAM lParam);

        CString m_Title;
        std::vector<PropertyPagePtr> m_vPages;  // vector of CPropertyPage
        std::vector<PROPSHEETPAGE> m_vPSP;      // vector of PROPSHEETPAGE
        PROPSHEETHEADER m_PSH;
    };

}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

namespace Win32xx
{

    //////////////////////////////////////////
    // Definitions for the CPropertyPage class
    //

    inline CPropertyPage::CPropertyPage(UINT nIDTemplate, LPCTSTR szTitle /* = 0*/) : CDialog(static_cast<UINT>(0))
    {
        ZeroMemory(&m_PSP, sizeof(m_PSP));
        SetTitle(szTitle);

        m_PSP.dwSize        = sizeof(m_PSP);
        m_PSP.dwFlags       |= PSP_USECALLBACK;
        m_PSP.hInstance     = GetApp().GetResourceHandle();
        m_PSP.pszTemplate   = MAKEINTRESOURCE(nIDTemplate);
        m_PSP.pszTitle      = m_Title;
        m_PSP.pfnDlgProc    = (DLGPROC)CPropertyPage::StaticDialogProc;
        m_PSP.lParam        = reinterpret_cast<LPARAM>(this);
        m_PSP.pfnCallback   = CPropertyPage::StaticPropSheetPageProc;
    }


    // Disables the Cancel button and changes the text of the OK button to "Close."
    inline void CPropertyPage::CancelToClose() const
    {
        assert(IsWindow());
        SendMessage(PSM_CANCELTOCLOSE, 0L, 0L);
    }


    // Override this function to process the property page's window message
    inline INT_PTR CPropertyPage::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        // A typical function might look like this:

        //  switch (uMsg)
        //  {
        //  case MESSAGE1:      // Some Win32 API message
        //      OnMessage1();   // A user defined function
        //      break;          // Also do default processing
        //  case MESSAGE2:
        //      OnMessage2();
        //      return x;       // Don't do default processing, but instead return
        //                      //  a value recommended by the Win32 API documentation
        //  }

        // Always pass unhandled messages on to DialogProcDefault
        return DialogProcDefault(uMsg, wParam, lParam);
    }


    // Provides default handling for the property page's message.
    // The DialogProc functions should pass unhandled messages to this function
    inline INT_PTR CPropertyPage::DialogProcDefault(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        switch (uMsg)
        {
        case PSM_QUERYSIBLINGS:
            return OnQuerySiblings(uMsg, wParam, lParam);
        }

        return CDialog::DialogProcDefault(uMsg, wParam, lParam);

    } // INT_PTR CALLBACK CPropertyPage::DialogProc(...)


    // Returns TRUE if the button is enabled
    inline BOOL CPropertyPage::IsButtonEnabled(int iButton) const
    {
        assert(IsWindow());
        return GetParent().GetDlgItem(iButton).IsWindowEnabled();
    }


    // This function is called for each page when the Apply, OK or Close button is pressed.
    // Override this function in your derived class to perform tasks when actions are applied.
    inline BOOL CPropertyPage::OnApply()
    {
        // Return TRUE to accept the changes; otherwise FALSE.
        return TRUE;
    }


    // This function is called for each page when the Cancel button is pressed
    // Override this function in your derived class when if the cancel button is pressed.
    inline void CPropertyPage::OnCancel()
    {
    }


    // This function is called in response to the PSN_HELP notification.
    inline void CPropertyPage::OnHelp()
    {

        SendMessage(*this, WM_COMMAND, ID_HELP, 0L);
    }


    // Called when the cancel button is pressed, and before the cancel has taken place
    // Returns TRUE to prevent the cancel operation, or FALSE to allow it.
    inline BOOL CPropertyPage::OnQueryCancel()
    {
        return FALSE;    // Allow cancel to proceed
    }


    // Responds to a query request from the Property Sheet.
    // The values for wParam and lParam are the ones set by the CPropertySheet::QuerySiblings call.
    // return FALSE to allow other siblings to be queried, or
    // return TRUE to stop query at this page.
    inline BOOL CPropertyPage::OnQuerySiblings(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        UNREFERENCED_PARAMETER(uMsg);
        UNREFERENCED_PARAMETER(wParam);
        UNREFERENCED_PARAMETER(lParam);

        return FALSE;
    }

    // Called when the user chooses the Cancel button
    inline void CPropertyPage::OnReset()
    {
        OnCancel();
    }


    // Called when the property page is created.
    // Override this function to perform operations when the property page is created.
    inline BOOL CPropertyPage::OnInitDialog()
    {
        return TRUE; // Pass Keyboard control to handle in WPARAM
    }


    // This is called in response to a PSN_KILLACTIVE notification, which
    // is sent whenever the OK or Apply button is pressed.
    // It provides an opportunity to validate the page contents before it's closed.
    // Return TRUE to prevent the page from losing the activation, or FALSE to allow it.
    inline BOOL CPropertyPage::OnKillActive()
    {
        return FALSE;
    }


    // Called when the OK button is pressed if OnApply for each page returns TRUE.
    // Override this function to perform tasks when the property sheet is closed.
    inline void CPropertyPage::OnOK()
    {
        // Close the modeless propertysheet
        GetParent().PostMessage(WM_CLOSE);
    }


    // Handles the WM_NOTIFY message and call the appropriate functions.
    inline LRESULT CPropertyPage::OnNotify(WPARAM wParam, LPARAM lParam)
    {

        UNREFERENCED_PARAMETER(wParam);

        LPPSHNOTIFY pNotify = (LPPSHNOTIFY)lParam;
        assert(pNotify);

        switch(pNotify->hdr.code)
        {
        case PSN_SETACTIVE:
            return OnSetActive() ? 0L : -1L;
        case PSN_KILLACTIVE:
            return OnKillActive();
        case PSN_APPLY:
            if (OnApply())
            {
                if (pNotify->lParam)
                    OnOK();
                return PSNRET_NOERROR;
            }
            return PSNRET_INVALID_NOCHANGEPAGE;
        case PSN_RESET:
            OnReset();
        case PSN_QUERYCANCEL:
            return OnQueryCancel();
        case PSN_WIZNEXT:
            return OnWizardNext()? 0L : -1L;
        case PSN_WIZBACK:
            return OnWizardBack()? 0L : -1L;
        case PSN_WIZFINISH:
            return !OnWizardFinish();
        case PSN_HELP:
            OnHelp();
            break;
        default:
            return FALSE;   // notification not handled
        }

        // notification handled
        // The framework will call SetWindowLongPtr(DWLP_MSGRESULT, lr) for non-zero returns
        return TRUE;
    }


    // Called when a page becomes active. Override this function to perform tasks
    // when a page is activated.
    // Return TRUE if the page was successfully set active; otherwise FALSE.
    inline BOOL CPropertyPage::OnSetActive()
    {
        return TRUE;
    }


    // This function is called when the Back button is pressed on a wizard page.
    // Override this function to perform tasks when the back button is pressed.
    // Return TRUE to allow the wizard to go to the previous page; otherwise return FALSE.
    inline BOOL CPropertyPage::OnWizardBack()
    {
        return TRUE;
    }


    // This function is called when the Finish button is pressed on a wizard page.
    // Override this function to perform tasks when the wizard is finished.
    // Return TRUE if the property sheet is destroyed when the wizard finishes; otherwise return FALSE.
    inline BOOL CPropertyPage::OnWizardFinish()
    {
        return TRUE; // Allow wizard to finish
    }


    // This function is called when the Next button is pressed on a wizard page.
    // Override this function in your derived class when the next button is pressed.
    // Return TRUE to allow the wizard to go to the next page; otherwise return FALSE.
    inline BOOL CPropertyPage::OnWizardNext()
    {
        return TRUE;
    }


    // Override this function to filter mouse and keyboard messages prior to
    // being passed to the DialogProc.
    inline BOOL CPropertyPage::PreTranslateMessage(MSG& Msg)
    {
        // allow the tab control to translate keyboard input
        if (Msg.message == WM_KEYDOWN && GetAsyncKeyState(VK_CONTROL) < 0 &&
            (Msg.wParam == VK_TAB || Msg.wParam == VK_PRIOR || Msg.wParam == VK_NEXT))
        {
            if (GetParent().SendMessage(PSM_ISDIALOGMESSAGE, 0L, reinterpret_cast<LPARAM>(&Msg)))
                return TRUE;
        }

        // allow the dialog to translate keyboard input
        if ((Msg.message >= WM_KEYFIRST) && (Msg.message <= WM_KEYLAST))
        {
            if (IsDialogMessage(Msg))
                return TRUE;
        }

        return CWnd::PreTranslateMessage(Msg);
    }


    // Sent to a property sheet, which then forwards the message to each of its pages.
    // Set wParam and lParam to values you want passed to the property pages.
    // Returns the non-zero value from a page in the property sheet, or zero if no page
    // returns a non-zero value.
    inline LRESULT CPropertyPage::QuerySiblings(WPARAM wParam, LPARAM lParam) const
    {
        assert(IsWindow());
        return GetParent().SendMessage(PSM_QUERYSIBLINGS, wParam, lParam);
    }


    // The property sheet will enable the Apply button if IsChanged is TRUE.
    inline void CPropertyPage::SetModified(BOOL IsChanged) const
    {
        assert(IsWindow());

        if (IsChanged)
            GetParent().SendMessage(PSM_CHANGED, reinterpret_cast<WPARAM>(GetHwnd()), 0L);
        else
            GetParent().SendMessage(PSM_UNCHANGED, reinterpret_cast<WPARAM>(GetHwnd()), 0L);
    }


    // Sets the title of the property page.
    inline void CPropertyPage::SetTitle(LPCTSTR szTitle)
    {
        if (szTitle)
        {
            m_Title = szTitle;
            m_PSP.dwFlags |= PSP_USETITLE;
        }
        else
        {
            m_Title.Empty();
            m_PSP.dwFlags &= ~PSP_USETITLE;
        }

        m_PSP.pszTitle = m_Title;
    }


    // Enables or disables the various buttons on a wizard property page.
    // dwFlags:  A value that specifies which wizard buttons are enabled. You can combine one or more of the following flags.
    //  PSWIZB_BACK             Enable the Back button. If this flag is not set, the Back button is displayed as disabled.
    //  PSWIZB_DISABLEDFINISH   Display a disabled Finish button.
    //  PSWIZB_FINISH           Display an enabled Finish button.
    //  PSWIZB_NEXT             Enable the Next button. If this flag is not set, the Next button is displayed as disabled.
    inline void CPropertyPage::SetWizardButtons(DWORD dwFlags) const
    {
        assert ( IsWindow() );
        PropSheet_SetWizButtons(::GetParent(*this), dwFlags);
    }


    inline UINT CALLBACK CPropertyPage::StaticPropSheetPageProc(HWND hwnd, UINT uMsg, LPPROPSHEETPAGE ppsp)
    {
        assert( &GetApp() );
        UNREFERENCED_PARAMETER(hwnd);

        // Note: the hwnd is always NULL

        switch (uMsg)
        {
        case PSPCB_CREATE:
            {
                TLSData* pTLSData = GetApp().GetTlsData();
                assert(pTLSData);

                // Store the CPropertyPage pointer in Thread Local Storage
                pTLSData->pWnd = reinterpret_cast<CWnd*>(ppsp->lParam);
            }
            break;
        }

        return TRUE;
    }


    inline INT_PTR CALLBACK CPropertyPage::StaticDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        assert( &GetApp() );

        // Find matching CWnd pointer for this HWND
        CPropertyPage* pPage = static_cast<CPropertyPage*>(GetCWndPtr(hwndDlg));
        if (!pPage)
        {
            // matching CWnd pointer not found, so add it to HWNDMap now
            TLSData* pTLSData = GetApp().GetTlsData();
            pPage = static_cast<CPropertyPage*>(pTLSData->pWnd);

            // Set the hWnd members and call DialogProc for this message
            pPage->m_hWnd = hwndDlg;
            pPage->AddToMap();
        }

        return pPage->DialogProc(uMsg, wParam, lParam);
    }


    ///////////////////////////////////////////
    // Definitions for the CPropertySheet class
    //

    inline CPropertySheet::CPropertySheet(UINT nIDCaption, HWND hParent /* = 0*/)
    {
        ZeroMemory(&m_PSH, sizeof(m_PSH));
        SetTitle(LoadString(nIDCaption));

#ifdef _WIN32_WCE
        m_PSH.dwSize = sizeof(PROPSHEETHEADER);
#else
        if (GetComCtlVersion() >= 471)
            m_PSH.dwSize = sizeof(PROPSHEETHEADER);
        else
            m_PSH.dwSize = PROPSHEETHEADER_V1_SIZE;
#endif

        m_PSH.dwFlags          = PSH_PROPSHEETPAGE | PSH_USECALLBACK;
        m_PSH.hwndParent       = hParent;
        m_PSH.hInstance        = GetApp().GetInstanceHandle();
        m_PSH.pfnCallback      = (PFNPROPSHEETCALLBACK)CPropertySheet::Callback;
    }


    inline CPropertySheet::CPropertySheet(LPCTSTR pszCaption /*= NULL*/, HWND hParent /* = 0*/)
    {
        ZeroMemory(&m_PSH, sizeof (m_PSH));
        SetTitle(pszCaption);

#ifdef _WIN32_WCE
        m_PSH.dwSize = PROPSHEETHEADER_V1_SIZE;
#else
        if (GetComCtlVersion() >= 471)
            m_PSH.dwSize = sizeof(PROPSHEETHEADER);
        else
            m_PSH.dwSize = PROPSHEETHEADER_V1_SIZE;
#endif

        m_PSH.dwFlags          = PSH_PROPSHEETPAGE | PSH_USECALLBACK;
        m_PSH.hwndParent       = hParent;
        m_PSH.hInstance        = GetApp().GetInstanceHandle();
        m_PSH.pfnCallback      = (PFNPROPSHEETCALLBACK)CPropertySheet::Callback;
    }


    // Adds a Property Page to the Property Sheet
    // The framework assumes ownership of the CPropertyPage pointer provided,
    // and deletes the CPropertyPage object when the PropertySheet is destroyed.
    inline CPropertyPage* CPropertySheet::AddPage(CPropertyPage* pPage)
    {
        assert(NULL != pPage);

        m_vPages.push_back(PropertyPagePtr(pPage));

        if (*this)
        {
            // property sheet already exists, so add page to it
            PROPSHEETPAGE psp = pPage->GetPSP();
            HPROPSHEETPAGE hpsp = ::CreatePropertySheetPage(&psp);
            PropSheet_AddPage(*this, hpsp);
        }

        m_PSH.nPages = static_cast<int>(m_vPages.size());

        return pPage;
    }


    // Builds the array of PROPSHEETPAGE.
    inline void CPropertySheet::BuildPageArray()
    {
        m_vPSP.clear();
        std::vector<PropertyPagePtr>::const_iterator iter;
        for (iter = m_vPages.begin(); iter != m_vPages.end(); ++iter)
            m_vPSP.push_back((*iter)->GetPSP());

        PROPSHEETPAGE* pPSPArray = &m_vPSP.front(); // Array of PROPSHEETPAGE
        m_PSH.ppsp = pPSPArray;
    }

    // Override this function to filter mouse and keyboard messages prior to
    // being passed to the DialogProc.
    inline void CALLBACK CPropertySheet::Callback(HWND hwnd, UINT uMsg, LPARAM lParam)
    {
        assert( &GetApp() );

        switch(uMsg)
        {
        //called before the dialog is created, hwnd = NULL, lParam points to dialog resource
        case PSCB_PRECREATE:
            {
                LPDLGTEMPLATE  lpTemplate = (LPDLGTEMPLATE)lParam;

                if(!(lpTemplate->style & WS_SYSMENU))
                {
                    lpTemplate->style |= WS_SYSMENU;
                }
            }
            break;

        //called after the dialog is created
        case PSCB_INITIALIZED:
            {
                // Retrieve pointer to CWnd object from Thread Local Storage
                TLSData* pTLSData = GetApp().GetTlsData();
                assert(pTLSData);

                CPropertySheet* w = static_cast<CPropertySheet*>(pTLSData->pWnd);
                assert(w);

                w->Attach(hwnd);
            }
            break;
        }
    }


    // Creates a modeless Property Sheet
    inline HWND CPropertySheet::Create(HWND hParent /*= 0*/)
    {
        assert( &GetApp() );
        assert(!IsWindow());        // Only one window per CWnd instance allowed

        if (hParent)
        {
            m_PSH.hwndParent = hParent;
        }

        BuildPageArray();
        PROPSHEETPAGE* pPSPArray = &m_vPSP.front();
        m_PSH.ppsp = pPSPArray;

        // Create a modeless Property Sheet
        m_PSH.dwFlags &= ~PSH_WIZARD;
        m_PSH.dwFlags |= PSH_MODELESS;
        HWND hWnd = reinterpret_cast<HWND>(CreatePropertySheet(&m_PSH));
        if (hWnd == 0)
            throw CWinException(_T("CreatePropertySheet failed"));

        return hWnd;
    }


    // Display either a modal or modeless property sheet, depending on the PROPSHEETHEADER flags.
    inline INT_PTR CPropertySheet::CreatePropertySheet(LPCPROPSHEETHEADER ppsph)
    {
        assert( &GetApp() );

        // Only one window per CWnd instance allowed
        assert(!IsWindow());

        INT_PTR ipResult = 0;
        m_hWnd = 0;

        // Ensure this thread has the TLS index set
        TLSData* pTLSData = GetApp().SetTlsData();

        // Store the 'this' pointer in Thread Local Storage
        pTLSData->pWnd = this;

        // Create the property sheet
        ipResult = PropertySheet(ppsph);

        // Tidy up
        pTLSData->pWnd = NULL;

        if (ipResult == -1)
            throw CWinException(_T("PropertySheet failed"));

        return ipResult;
    }


    // Removes the specified button.
    inline void CPropertySheet::DestroyButton(int IDButton)
    {
        assert(IsWindow());

        HWND hwndButton = ::GetDlgItem(*this, IDButton);
        if (hwndButton != NULL)
        {
            // Hide and disable the button
            ::ShowWindow(hwndButton, SW_HIDE);
            ::EnableWindow(hwndButton, FALSE);
        }
    }


    // Called when a property sheet is destroyed.
    // Note: To destroy a property sheet from within an application, post a WM_CLOSE
    inline void CPropertySheet::Destroy()
    {
        CWnd::Destroy();
        m_vPages.clear();
    }


    // Create a modal property sheet
    inline int CPropertySheet::DoModal()
    {
        assert( &GetApp() );
        assert(!IsWindow());        // Only one window per CWnd instance allowed

        BuildPageArray();
        PROPSHEETPAGE* pPSPArray = &m_vPSP.front();
        m_PSH.ppsp = pPSPArray;

        // Create the Property Sheet
        m_PSH.dwFlags &= ~PSH_MODELESS;
        int nResult = static_cast<int>(CreatePropertySheet(&m_PSH));

        m_vPages.clear();

        return nResult;
    }


    // Retrieves the property sheets active property page.
    inline CPropertyPage* CPropertySheet::GetActivePage() const
    {
        assert(IsWindow());

        CPropertyPage* pPage = NULL;
        if (GetHwnd() != 0)
        {
            HWND hPage = reinterpret_cast<HWND>(SendMessage(PSM_GETCURRENTPAGEHWND, 0L, 0L));
            pPage = static_cast<CPropertyPage*>(GetCWndPtr(hPage));
        }

        return pPage;
    }


    // Returns the number of Property Pages in this Property Sheet
    inline int CPropertySheet::GetPageCount() const
    {
        assert(IsWindow());
        return static_cast<int>(m_vPages.size());
    }


    // Returns the index of the specified property page.
    inline int CPropertySheet::GetPageIndex(CPropertyPage* pPage) const
    {
        assert(IsWindow());

        for (int i = 0; i < GetPageCount(); i++)
        {
            if (m_vPages[i].get() == pPage)
                return i;
        }
        return -1;
    }


    // Returns the handle to the Property Sheet's tab control
    inline HWND CPropertySheet::GetTabControl() const
    {
        assert(IsWindow());
        return reinterpret_cast<HWND>(SendMessage(PSM_GETTABCONTROL, 0L, 0L));
    }


    // Returns TRUE of the property sheet is modeless.
    inline BOOL CPropertySheet::IsModeless() const
    {
        return (m_PSH.dwFlags & PSH_MODELESS);
    }


    // Returns TRUE if this property sheet is a wizard.
    inline BOOL CPropertySheet::IsWizard() const
    {
        return (m_PSH.dwFlags & PSH_WIZARD);
    }


    // Removes a Property Page from the Property Sheet.
    inline void CPropertySheet::RemovePage(CPropertyPage* pPage)
    {
        assert(IsWindow());

        int nPage = GetPageIndex(pPage);
        if (GetHwnd() != 0)
            SendMessage(*this, PSM_REMOVEPAGE, nPage, 0L);

        m_vPages.erase(m_vPages.begin() + nPage, m_vPages.begin() + nPage+1);
        m_PSH.nPages = static_cast<int>(m_vPages.size());
    }


    // Override this function to filter mouse and keyboard messages prior to
    // being passed to WndProc.
    inline BOOL CPropertySheet::PreTranslateMessage(MSG& Msg)
    {
        // allow sheet to translate Ctrl+Tab, Shift+Ctrl+Tab, Ctrl+PageUp, and Ctrl+PageDown
        if (Msg.message == WM_KEYDOWN && GetAsyncKeyState(VK_CONTROL) < 0 &&
            (Msg.wParam == VK_TAB || Msg.wParam == VK_PRIOR || Msg.wParam == VK_NEXT))
        {
            if (SendMessage(PSM_ISDIALOGMESSAGE, 0L, reinterpret_cast<LPARAM>(&Msg)))
                return TRUE;
        }

        // allow the dialog to translate keyboard input
        if ((Msg.message >= WM_KEYFIRST) && (Msg.message <= WM_KEYLAST))
        {
            return GetActivePage()->PreTranslateMessage(Msg);
        }

        return CWnd::PreTranslateMessage(Msg);
    }


    // Activates the specified property page.
    inline BOOL CPropertySheet::SetActivePage(int nPage)
    {
        assert(IsWindow());
        return static_cast<BOOL>(SendMessage(*this, PSM_SETCURSEL, nPage, 0L));
    }


    // Activates the specified property page.
    inline BOOL CPropertySheet::SetActivePage(CPropertyPage* pPage)
    {
        assert(IsWindow());
        int nPage = GetPageIndex(pPage);
        if ((nPage >= 0))
            return SetActivePage(nPage);

        return FALSE;
    }


    // Sets the property sheet's icon
    inline void CPropertySheet::SetIcon(UINT idIcon)
    {
        m_PSH.pszIcon = MAKEINTRESOURCE(idIcon);
        m_PSH.dwFlags |= PSH_USEICONID;
    }


    // Sets the property sheet's title
    inline void CPropertySheet::SetTitle(LPCTSTR szTitle)
    {
        if (szTitle)
            m_Title = szTitle;
        else
            m_Title.Empty();

        m_PSH.pszCaption = m_Title;
    }


    // A Wizard is a form of property sheet that displays the pages in sequence.
    // This function enables or disables Wizard mode for the property sheet.
    inline void CPropertySheet::SetWizardMode(BOOL IsWizard)
    {
        if (IsWizard)
            m_PSH.dwFlags |= PSH_WIZARD;
        else
            m_PSH.dwFlags &= ~PSH_WIZARD;
    }

}

#endif // _WIN32XX_PROPERTYSHEET_H_
