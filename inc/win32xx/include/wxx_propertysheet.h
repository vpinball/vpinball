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

    /////////////////////////////////////////////////////////////
    // This class provides support for property pages. A property
    // page is an individual page used in a property sheet.
    class CPropertyPage : public CDialog
    {
    public:
        CPropertyPage (UINT templateID, LPCTSTR title = NULL);
        virtual ~CPropertyPage() {}
        virtual INT_PTR DialogProc(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual BOOL OnApply();
        virtual void OnCancel();
        virtual void OnHelp();
        virtual BOOL OnInitDialog();
        virtual BOOL OnKillActive();
        virtual LRESULT OnNotify(WPARAM wparam, LPARAM lparam);
        virtual void OnOK();
        virtual BOOL OnQueryCancel();
        virtual BOOL OnQuerySiblings(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual BOOL OnSetActive();
        virtual BOOL OnWizardBack();
        virtual BOOL OnWizardFinish();
        virtual BOOL OnWizardNext();
        virtual BOOL PreTranslateMessage(MSG& msg);

        void CancelToClose() const;
        INT_PTR DialogProcDefault(UINT msg, WPARAM wparam, LPARAM lparam);
        PROPSHEETPAGE GetPSP() const {return m_psp;}
        BOOL IsButtonEnabled(UINT buttonID) const;
        LRESULT QuerySiblings(WPARAM wparam, LPARAM lparam) const;
        void SetModified(BOOL isChanged) const;
        void SetTitle(LPCTSTR title);
        void SetWizardButtons(DWORD flags) const;

    private:
        CPropertyPage(const CPropertyPage&);                // Disable copy construction
        CPropertyPage& operator = (const CPropertyPage&);   // Disable assignment operator

        static UINT CALLBACK StaticPropSheetPageProc(HWND wnd, UINT msg, LPPROPSHEETPAGE ppsp);
        static INT_PTR CALLBACK StaticDialogProc(HWND hDlg, UINT msg, WPARAM wparam, LPARAM lparam);

        PROPSHEETPAGE m_psp;
        CString m_title;
    };


    ///////////////////////////////////////////////////////////////
    // This class provides support for a property sheet. A property
    // sheet is also known as a tab dialog box. It has one or more
    // property pages. A Property sheet can present its pages as a
    // wizard.
    class CPropertySheet : public CWnd
    {
    public:
        CPropertySheet(UINT captionID, HWND parent = 0);
        CPropertySheet(LPCTSTR caption = NULL, HWND parent = 0);
        virtual ~CPropertySheet() {}

        // Operations
        virtual CPropertyPage* AddPage(CPropertyPage* pPage);
        virtual HWND Create(HWND parent = 0);
        virtual INT_PTR CreatePropertySheet(LPCPROPSHEETHEADER pPSH);
        virtual void DestroyButton(int button);
        virtual void Destroy();
        virtual int DoModal();
        virtual void RemovePage(CPropertyPage* page);

        // State functions
        BOOL IsModeless() const;
        BOOL IsWizard() const;

        // Accessors and mutators
        CPropertyPage* GetActivePage() const;
        int GetPageCount() const;
        int GetPageIndex(CPropertyPage* pPage) const;
        HWND GetTabControl() const;
        BOOL SetActivePage(int page);
        BOOL SetActivePage(CPropertyPage* pPage);
        void SetIcon(UINT iconID);
        void SetTitle(LPCTSTR title);
        void SetWizardMode(BOOL isWizard);

    protected:
        virtual LRESULT OnSetDefID(WPARAM wparam);
        virtual BOOL PreTranslateMessage(MSG& msg);
        virtual LRESULT WndProcDefault(UINT msg, WPARAM wparam, LPARAM lparam);

    private:
        CPropertySheet(const CPropertySheet&);              // Disable copy construction
        CPropertySheet& operator = (const CPropertySheet&); // Disable assignment operator
        void BuildPageArray();
        static void CALLBACK Callback(HWND hwnd, UINT msg, LPARAM lparam);

        CString m_title;
        std::vector<PropertyPagePtr> m_allPages;           // vector of CPropertyPage
        std::vector<PROPSHEETPAGE> m_allSheetPages;        // vector of PROPSHEETPAGE
        PROPSHEETHEADER m_psh;
    };

}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

namespace Win32xx
{

    //////////////////////////////////////////
    // Definitions for the CPropertyPage class
    //

    inline CPropertyPage::CPropertyPage(UINT templateID, LPCTSTR title /* = 0*/)
    {
        ZeroMemory(&m_psp, sizeof(m_psp));
        SetTitle(title);

        m_psp.dwSize        = sizeof(m_psp);
        m_psp.dwFlags       |= PSP_USECALLBACK;
        m_psp.hInstance     = GetApp()->GetInstanceHandle();
        m_psp.pszTemplate   = MAKEINTRESOURCE(templateID);
        m_psp.pszTitle      = m_title;
        m_psp.pfnDlgProc    = (DLGPROC)CPropertyPage::StaticDialogProc;
        m_psp.lParam        = reinterpret_cast<LPARAM>(this);
        m_psp.pfnCallback   = CPropertyPage::StaticPropSheetPageProc;
    }

    // Disables the Cancel button and changes the text of the OK button to "Close."
    // Refer to PSM_CANCELTOCLOSE in the Windows API documentation for more information.
    inline void CPropertyPage::CancelToClose() const
    {
        assert(IsWindow());
        SendMessage(PSM_CANCELTOCLOSE, 0, 0);
    }

    // Override this function to process the property page's window message
    inline INT_PTR CPropertyPage::DialogProc(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        // A typical function might look like this:

        //  switch (msg)
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
        return DialogProcDefault(msg, wparam, lparam);
    }

    // Provides default handling for the property page's message.
    // The DialogProc functions should pass unhandled messages to this function
    inline INT_PTR CPropertyPage::DialogProcDefault(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        switch (msg)
        {
        case PSM_QUERYSIBLINGS:
            return OnQuerySiblings(msg, wparam, lparam);
        }

        return CDialog::DialogProcDefault(msg, wparam, lparam);

    }

    // Returns TRUE if the button is enabled
    inline BOOL CPropertyPage::IsButtonEnabled(UINT buttonID) const
    {
        assert(IsWindow());
        return GetParent().GetDlgItem(buttonID).IsWindowEnabled();
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
        // Close the propertysheet.
        GetParent().Close();
    }

    // This function is called in response to the PSN_HELP notification.
    inline void CPropertyPage::OnHelp()
    {

        SendMessage(*this, WM_COMMAND, ID_HELP, 0);
    }

    // Called when the cancel button is pressed, and before the cancel has taken place
    // Returns TRUE to prevent the cancel operation, or FALSE to allow it.
    inline BOOL CPropertyPage::OnQueryCancel()
    {
        return FALSE;    // Allow cancel to proceed
    }

    // Responds to a query request from the Property Sheet.
    // The values for wparam and lparam are the ones set by the CPropertySheet::QuerySiblings call.
    // return FALSE to allow other siblings to be queried, or
    // return TRUE to stop query at this page.
    inline BOOL CPropertyPage::OnQuerySiblings(UINT, WPARAM, LPARAM)
    {
        return FALSE;
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
        // Close the propertysheet.
        GetParent().Close();
    }

    // Handles the WM_NOTIFY message and call the appropriate functions.
    inline LRESULT CPropertyPage::OnNotify(WPARAM, LPARAM lparam)
    {
        LPPSHNOTIFY pNotify = (LPPSHNOTIFY)lparam;
        assert(pNotify);
        if (!pNotify) return 0;

        // The framework will call SetWindowLongPtr(DWLP_MSGRESULT, result)
        // for non-zero returns.
        switch(pNotify->hdr.code)
        {
        case PSN_SETACTIVE:
            return OnSetActive() ? 0 : -1;
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
            OnCancel();
            break;
        case PSN_QUERYCANCEL:
            return OnQueryCancel();
        case PSN_WIZNEXT:
            return OnWizardNext()? 0 : -1;
        case PSN_WIZBACK:
            return OnWizardBack()? 0 : -1;
        case PSN_WIZFINISH:
            return !OnWizardFinish();
        case PSN_HELP:
            OnHelp();
            break;
        default:
            return PSNRET_NOERROR;   // page is valid
        }

        return PSNRET_NOERROR;   // page is valid
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
    inline BOOL CPropertyPage::PreTranslateMessage(MSG& msg)
    {
        // allow the tab control to translate keyboard input
        if (msg.message == WM_KEYDOWN && GetAsyncKeyState(VK_CONTROL) < 0 &&
            (msg.wParam == VK_TAB || msg.wParam == VK_PRIOR || msg.wParam == VK_NEXT))
        {
            LPARAM lparam = reinterpret_cast<LPARAM>(&msg);
            if (GetParent().SendMessage(PSM_ISDIALOGMESSAGE, 0, lparam))
                return TRUE;
        }

        // allow the dialog to translate keyboard input
        if ((msg.message >= WM_KEYFIRST) && (msg.message <= WM_KEYLAST))
        {
            if (IsDialogMessage(msg))
                return TRUE;
        }

        return CWnd::PreTranslateMessage(msg);
    }

    // Sent to a property sheet, which then forwards the message to each of its pages.
    // Set wparam and lparam to values you want passed to the property pages.
    // Returns the non-zero value from a page in the property sheet, or zero if no page
    // returns a non-zero value.
    // Refer to PSM_QUERYSIBLINGS in the Windows API documentation for more information.
    inline LRESULT CPropertyPage::QuerySiblings(WPARAM wparam, LPARAM lparam) const
    {
        assert(IsWindow());
        return GetParent().SendMessage(PSM_QUERYSIBLINGS, wparam, lparam);
    }

    // The property sheet will enable the Apply button if IsChanged is TRUE.
    // Refer to PSM_CHANGED and PSM_UNCHANGED in the Windows API documentation for more information.
    inline void CPropertyPage::SetModified(BOOL isChanged) const
    {
        assert(IsWindow());

        WPARAM wparam = reinterpret_cast<WPARAM>(GetHwnd());
        if (isChanged)
            GetParent().SendMessage(PSM_CHANGED, wparam, 0);
        else
            GetParent().SendMessage(PSM_UNCHANGED, wparam, 0);
    }

    // Sets the title of the property page.
    inline void CPropertyPage::SetTitle(LPCTSTR title)
    {
        if (title)
        {
            m_title = title;
            m_psp.dwFlags |= PSP_USETITLE;
        }
        else
        {
            m_title.Empty();
            m_psp.dwFlags &= ~PSP_USETITLE;
        }

        m_psp.pszTitle = m_title;
    }

    // Enables or disables the various buttons on a wizard property page.
    // flags:  A value that specifies which wizard buttons are enabled. You can combine one or more of the following flags.
    //  PSWIZB_BACK             Enable the Back button. If this flag is not set, the Back button is displayed as disabled.
    //  PSWIZB_DISABLEDFINISH   Display a disabled Finish button.
    //  PSWIZB_FINISH           Display an enabled Finish button.
    //  PSWIZB_NEXT             Enable the Next button. If this flag is not set, the Next button is displayed as disabled.
    inline void CPropertyPage::SetWizardButtons(DWORD flags) const
    {
        assert ( IsWindow() );
        PropSheet_SetWizButtons(::GetParent(*this), flags);
    }

    inline UINT CALLBACK CPropertyPage::StaticPropSheetPageProc(HWND, UINT msg, LPPROPSHEETPAGE ppsp)
    {
        switch (msg)
        {
        case PSPCB_CREATE:
        {
            TLSData* pTLSData = GetApp()->GetTlsData();
            if (pTLSData == NULL)
            {
                // Thread Local Storage data isn't assigned.
                // We should never get here.
                TRACE("*** Warning in CPropertyPage::StaticPropSheetPageProc: TLS data isn't assigned ***\n");
                return 0;
            }

            // Store the CPropertyPage pointer in Thread Local Storage
            pTLSData->pWnd = reinterpret_cast<CWnd*>(ppsp->lParam);
        }
        break;
        }

        return TRUE;
    }

    inline INT_PTR CALLBACK CPropertyPage::StaticDialogProc(HWND hDlg, UINT msg, WPARAM wparam, LPARAM lparam)
    {
        // Find matching CWnd pointer for this HWND
        CPropertyPage* pPage = static_cast<CPropertyPage*>(GetCWndPtr(hDlg));
        if (!pPage)
        {
            // matching CWnd pointer not found, so add it to HWNDMap now
            TLSData* pTLSData = GetApp()->GetTlsData();
            pPage = static_cast<CPropertyPage*>(pTLSData->pWnd);

            if (pPage)
            {
                // Set the wnd members and call DialogProc for this message
                pPage->m_wnd = hDlg;
                pPage->AddToMap();
            }
        }

        if (pPage == 0)
        {
            // Got a message for a window that's not in the map.
            // We should never get here.
            TRACE("*** Warning in CPropertyPage::StaticDialogProc: HWND not in window map ***\n");
            return 0;
        }

        return pPage->DialogProc(msg, wparam, lparam);
    }


    ///////////////////////////////////////////
    // Definitions for the CPropertySheet class
    //

    inline CPropertySheet::CPropertySheet(UINT captionID, HWND parent /* = 0*/)
    {
        ZeroMemory(&m_psh, sizeof(m_psh));
        SetTitle(LoadString(captionID));

        if (GetComCtlVersion() >= 471)
            m_psh.dwSize = sizeof(PROPSHEETHEADER);
        else
            m_psh.dwSize = PROPSHEETHEADER_V1_SIZE;

        m_psh.dwFlags     = PSH_PROPSHEETPAGE | PSH_USECALLBACK;
        m_psh.hwndParent  = parent;
        m_psh.hInstance   = GetApp()->GetInstanceHandle();
        m_psh.pfnCallback = reinterpret_cast<PFNPROPSHEETCALLBACK>(
            reinterpret_cast<void*>(CPropertySheet::Callback));
    }

    inline CPropertySheet::CPropertySheet(LPCTSTR caption /*= NULL*/, HWND parent /* = 0*/)
    {
        ZeroMemory(&m_psh, sizeof (m_psh));
        SetTitle(caption);

        if (GetComCtlVersion() >= 471)
            m_psh.dwSize = sizeof(PROPSHEETHEADER);
        else
            m_psh.dwSize = PROPSHEETHEADER_V1_SIZE;

        m_psh.dwFlags     = PSH_PROPSHEETPAGE | PSH_USECALLBACK;
        m_psh.hwndParent  = parent;
        m_psh.hInstance   = GetApp()->GetInstanceHandle();
        m_psh.pfnCallback = reinterpret_cast<PFNPROPSHEETCALLBACK>(
            reinterpret_cast<void*>(CPropertySheet::Callback));
    }

    // Adds a Property Page to the Property Sheet.
    // The framework assumes ownership of the CPropertyPage pointer provided,
    // and deletes the CPropertyPage object when the PropertySheet is destroyed.
    inline CPropertyPage* CPropertySheet::AddPage(CPropertyPage* pPage)
    {
        assert(pPage != NULL);
        if (!pPage) return NULL;

        m_allPages.push_back(PropertyPagePtr(pPage));

        if (*this)
        {
            // Property sheet already exists, so add page to it.
            PROPSHEETPAGE psp = pPage->GetPSP();
            HPROPSHEETPAGE hpsp = ::CreatePropertySheetPage(&psp);
            PropSheet_AddPage(*this, hpsp);
        }

        m_psh.nPages = static_cast<UINT>(m_allPages.size());

        return pPage;
    }

    // Builds the array of PROPSHEETPAGE.
    inline void CPropertySheet::BuildPageArray()
    {
        m_allSheetPages.clear();
        std::vector<PropertyPagePtr>::const_iterator iter;
        for (iter = m_allPages.begin(); iter != m_allPages.end(); ++iter)
            m_allSheetPages.push_back((*iter)->GetPSP());

        PROPSHEETPAGE* pPSPArray = &m_allSheetPages.front(); // Array of PROPSHEETPAGE
        m_psh.ppsp = pPSPArray;
    }

    // Perform initial processing of the property sheet messages.
    inline void CALLBACK CPropertySheet::Callback(HWND wnd, UINT msg, LPARAM lparam)
    {
        switch(msg)
        {
        // Called before the property sheet is created.
        // wnd = 0, and lparam points to dialog resource.
        case PSCB_PRECREATE:
            {
                LPDLGTEMPLATE  lpTemplate = (LPDLGTEMPLATE)lparam;

                if (!(lpTemplate->style & WS_SYSMENU))
                {
                    lpTemplate->style |= WS_SYSMENU;
                }
            }
            break;

        // Called when the property sheet is created.
        case PSCB_INITIALIZED:
            {
                // Retrieve pointer to CWnd object from Thread Local Storage.
                TLSData* pTLSData = GetApp()->GetTlsData();
                assert(pTLSData);
                if (!pTLSData) return;

                CPropertySheet* w = static_cast<CPropertySheet*>(pTLSData->pWnd);

                if (w != 0)
                {
                    // Subclass the property sheet, so that messages are forwarded
                    // to CWnd::StaticWindowProc for handling in WndProc.
                    w->Attach(wnd);
                }
                else
                {
                    // Thread Local Storage data isn't assigned.
                    // We should never get here.
                    TRACE("*** Warning in CPropertySheet::Callback: TLS data isn't assigned. ***\n");
                }
            }
            break;
        }
    }

    // Creates a modeless Property Sheet.
    // Refer to PropertySheet in the Windows API documentation for more information.
    inline HWND CPropertySheet::Create(HWND parent /*= 0*/)
    {
        assert(!IsWindow());        // Only one window per CWnd instance allowed.

        if (parent)
        {
            m_psh.hwndParent = parent;
        }

        BuildPageArray();
        PROPSHEETPAGE* pPSPArray = &m_allSheetPages.front();
        m_psh.ppsp = pPSPArray;

        // Create a modeless Property Sheet.
        m_psh.dwFlags &= ~PSH_WIZARD;
        m_psh.dwFlags |= PSH_MODELESS;
        HWND wnd = reinterpret_cast<HWND>(CreatePropertySheet(&m_psh));
        if (wnd == 0)
            throw CWinException(GetApp()->MsgWndPropertSheet());

        return wnd;
    }

    // Display either a modal or modeless property sheet, depending on the PROPSHEETHEADER flags.
    // Refer to PropertySheet in the Windows API documentation for more information.
    inline INT_PTR CPropertySheet::CreatePropertySheet(LPCPROPSHEETHEADER pPSH)
    {
        // Only one window per CWnd instance allowed
        assert(!IsWindow());

        INT_PTR ipResult = 0;
        m_wnd = 0;

        // Retrieve this thread's TLS data
        TLSData* pTLSData = GetApp()->GetTlsData();

        // Store the 'this' pointer in Thread Local Storage.
        pTLSData->pWnd = this;

        // Create the property sheet.
        ipResult = PropertySheet(pPSH);

        // Tidy up.
        pTLSData->pWnd = NULL;

        if (ipResult == -1)
            throw CWinException(GetApp()->MsgWndPropertSheet());

        return ipResult;
    }

    // Removes the specified button.
    inline void CPropertySheet::DestroyButton(int buttonID)
    {
        assert(IsWindow());

        HWND button = ::GetDlgItem(*this, buttonID);
        if (button != 0)
        {
            // Hide and disable the button.
            ::ShowWindow(button, SW_HIDE);
            ::EnableWindow(button, FALSE);
        }
    }

    // Called when a property sheet is destroyed.
    // Note: To destroy a property sheet from within an application, post a WM_CLOSE.
    inline void CPropertySheet::Destroy()
    {
        CWnd::Destroy();
        m_allPages.clear();
    }

    // Create a modal property sheet.
    // Refer to PropertySheet in the Windows API documentation for more information.
    inline int CPropertySheet::DoModal()
    {
        assert(!IsWindow());        // Only one window per CWnd instance allowed.

        BuildPageArray();
        PROPSHEETPAGE* pPSPArray = &m_allSheetPages.front();
        m_psh.ppsp = pPSPArray;

        // Create the Property Sheet.
        m_psh.dwFlags &= ~PSH_MODELESS;
        int nResult = static_cast<int>(CreatePropertySheet(&m_psh));

        m_allPages.clear();
        Cleanup();

        return nResult;
    }

    // Retrieves the property sheets active property page.
    // Refer to PSM_GETCURRENTPAGEHWND in the Windows API documentation for more information.
    inline CPropertyPage* CPropertySheet::GetActivePage() const
    {
        assert(IsWindow());

        CPropertyPage* pPage = NULL;
        if (GetHwnd() != 0)
        {
            HWND hPage = reinterpret_cast<HWND>(SendMessage(PSM_GETCURRENTPAGEHWND, 0, 0));
            pPage = static_cast<CPropertyPage*>(GetCWndPtr(hPage));
        }

        return pPage;
    }

    // Returns the number of Property Pages in this Property Sheet.
    inline int CPropertySheet::GetPageCount() const
    {
        assert(IsWindow());
        return static_cast<int>(m_allPages.size());
    }

    // Returns the index of the specified property page.
    inline int CPropertySheet::GetPageIndex(CPropertyPage* pPage) const
    {
        assert(IsWindow());
        int page = -1;

        for (int i = 0; i < GetPageCount(); i++)
        {
            size_t index = static_cast<size_t>(i);
            if (m_allPages[index].get() == pPage)
            {
                page = i;
                break;
            }
        }

        return page;
    }

    // Returns the handle to the Property Sheet's tab control.
    // Refer to PSM_GETTABCONTROL in the Windows API documentation for more information.
    inline HWND CPropertySheet::GetTabControl() const
    {
        assert(IsWindow());
        return reinterpret_cast<HWND>(SendMessage(PSM_GETTABCONTROL, 0, 0));
    }

    // Returns TRUE of the property sheet is modeless.
    inline BOOL CPropertySheet::IsModeless() const
    {
        return static_cast<BOOL>(m_psh.dwFlags & PSH_MODELESS);
    }

    // Returns TRUE if this property sheet is a wizard.
    inline BOOL CPropertySheet::IsWizard() const
    {
        return static_cast<BOOL>(m_psh.dwFlags & PSH_WIZARD);
    }

    // Called in response to a DM_SETDEFID message.
    // A DM_SETDEFID message is sent when a property page is selected.
    // Override this to modify the property sheet's default ID.
    inline LRESULT CPropertySheet::OnSetDefID(WPARAM wparam)
    {
        if (!IsWizard())
        {
            // Change the default button to IDOK.
            HWND ok = GetDlgItem(IDOK);
            HWND cancel = GetDlgItem(IDCANCEL);

            if (::IsWindow(ok) && ::IsWindowVisible(ok) && ::IsWindowEnabled(ok))
            {
                FinalWindowProc(DM_SETDEFID, IDOK, 0);

                ::SendMessage(cancel, BM_SETSTYLE, BS_PUSHBUTTON, TRUE);
                return TRUE;
            }
        }

        return FinalWindowProc(DM_SETDEFID, wparam, 0);
    }

    // Removes a Property Page from the Property Sheet.
    inline void CPropertySheet::RemovePage(CPropertyPage* pPage)
    {
        assert(IsWindow());

        int page = GetPageIndex(pPage);
        WPARAM wparam = static_cast<WPARAM>(page);
        if (GetHwnd() != 0)
            SendMessage(*this, PSM_REMOVEPAGE, wparam, 0);

        m_allPages.erase(m_allPages.begin() + page, m_allPages.begin() + page+1);
        m_psh.nPages = static_cast<UINT>(m_allPages.size());
    }

    // Override this function to filter mouse and keyboard messages prior to
    // being passed to WndProc.
    inline BOOL CPropertySheet::PreTranslateMessage(MSG& msg)
    {
        // Allow sheet to translate Ctrl+Tab, Shift+Ctrl+Tab, Ctrl+PageUp, and Ctrl+PageDown.
        if (msg.message == WM_KEYDOWN && GetAsyncKeyState(VK_CONTROL) < 0 &&
            (msg.wParam == VK_TAB || msg.wParam == VK_PRIOR || msg.wParam == VK_NEXT))
        {
            LPARAM lparam = reinterpret_cast<LPARAM>(&msg);
            if (SendMessage(PSM_ISDIALOGMESSAGE, 0, lparam))
                return TRUE;
        }

        // Allow the dialog to translate keyboard input.
        if (GetActivePage() && (msg.message >= WM_KEYFIRST) && (msg.message <= WM_KEYLAST))
        {
            return GetActivePage()->PreTranslateMessage(msg);
        }

        return CWnd::PreTranslateMessage(msg);
    }

    // Activates the specified property page.
    // Refer to PSM_SETCURSEL in the Windows API documentation for more information.
    inline BOOL CPropertySheet::SetActivePage(int page)
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(page);
        return static_cast<BOOL>(SendMessage(*this, PSM_SETCURSEL, wparam, 0));
    }

    // Activates the specified property page.
    inline BOOL CPropertySheet::SetActivePage(CPropertyPage* pPage)
    {
        assert(IsWindow());
        int page = GetPageIndex(pPage);
        if ((page >= 0))
            return SetActivePage(page);

        return FALSE;
    }

    // Sets the property sheet's icon.
    inline void CPropertySheet::SetIcon(UINT iconID)
    {
        m_psh.pszIcon = MAKEINTRESOURCE(iconID);
        m_psh.dwFlags |= PSH_USEICONID;
    }

    // Sets the property sheet's title.
    inline void CPropertySheet::SetTitle(LPCTSTR title)
    {
        if (title)
            m_title = title;
        else
            m_title.Empty();

        m_psh.pszCaption = m_title;
    }

    // A Wizard is a form of property sheet that displays the pages in sequence.
    // This function enables or disables Wizard mode for the property sheet.
    inline void CPropertySheet::SetWizardMode(BOOL isWizard)
    {
        if (isWizard)
            m_psh.dwFlags |= PSH_WIZARD;
        else
            m_psh.dwFlags &= ~PSH_WIZARD;
    }

    // Provides default processing of the PropertySheet's messages.
    inline LRESULT CPropertySheet::WndProcDefault(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        switch (msg)
        {
        case DM_SETDEFID:  return OnSetDefID(wparam);
        }
        // Pass unhandled messages on for default processing.
        return CWnd::WndProcDefault(msg, wparam, lparam);
    }
}

#endif // _WIN32XX_PROPERTYSHEET_H_
