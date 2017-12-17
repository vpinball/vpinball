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
// wxx_taskdialog.h
//  Declaration of the CTaskDialog class

// A task dialog is a dialog box that can be used to display information
// and receive simple input from the user. Like a message box, it is
// formatted by the operating system according to parameters you set.
// However, a task dialog has many more features than a message box.

// NOTES:
//  Task Dialogs are only supported on Windows Vista and above.
//  Task Dialogs require XP themes enabled (use version 6 of Common Controls)
//  Task Dialogs are always modal.


#ifndef _WIN32XX_TASKDIALOG_H_
#define _WIN32XX_TASKDIALOG_H_

#include "wxx_wincore.h"

namespace Win32xx
{

    ///////////////////////////////////////////////
    // The CTaskDialog class provides the functionality a task dialog.
    // A task dialog is similar to, while much more flexible than, a basic message box.
    class CTaskDialog : public CWnd
    {
    public:
        CTaskDialog();
        virtual ~CTaskDialog() {}

        void AddCommandControl(int nButtonID, LPCTSTR pszCaption);
        void AddRadioButton(int nRadioButtonID, LPCTSTR pszCaption);
        void AddRadioButtonGroup(int nIDRadioButtonsFirst, int nIDRadioButtonsLast);
        void ClickButton(int nButtonID) const;
        void ClickRadioButton(int nRadioButtonID) const;
        LRESULT DoModal(HWND hParent = NULL);
        void ElevateButton(int nButtonID, BOOL IsElevated) const;
        void EnableButton(int nButtonID, BOOL IsEnabled) const;
        void EnableRadioButton(int nButtonID, BOOL IsEnabled) const;
        TASKDIALOGCONFIG GetConfig() const;
        TASKDIALOG_FLAGS GetOptions() const;
        int GetSelectedButtonID() const;
        int GetSelectedRadioButtonID() const;
        BOOL GetVerificationCheckboxState() const;
        static BOOL IsSupported();
        void NavigateTo(CTaskDialog& TaskDialog) const;
        void SetCommonButtons(TASKDIALOG_COMMON_BUTTON_FLAGS dwCommonButtons);
        void SetContent(LPCTSTR pszContent);
        void SetDefaultButton(int nButtonID);
        void SetDefaultRadioButton(int nRadioButtonID);
        void SetDialogWidth(UINT nWidth = 0);
        void SetExpansionArea(LPCTSTR pszExpandedInfo, LPCTSTR pszExpandedLabel = _T(""), LPCTSTR pszCollapsedLabel = _T(""));
        void SetFooterIcon(HICON hFooterIcon);
        void SetFooterIcon(LPCTSTR lpszFooterIcon);
        void SetFooterText(LPCTSTR pszFooter);
        void SetMainIcon(HICON hMainIcon);
        void SetMainIcon(LPCTSTR lpszMainIcon);
        void SetMainInstruction(LPCTSTR pszMainInstruction);
        void SetOptions(TASKDIALOG_FLAGS dwFlags);
        void SetProgressBarMarquee(BOOL IsEnabled = TRUE, int nMarqueeSpeed = 0) const;
        void SetProgressBarPosition(int nProgressPos) const;
        void SetProgressBarRange(int nMinRange, int nMaxRange) const;
        void SetProgressBarState(int nNewState = PBST_NORMAL) const;
        void SetVerificationCheckbox(BOOL IsChecked) const;
        void SetVerificationCheckboxText(LPCTSTR pszVerificationText);
        void SetWindowTitle(LPCTSTR pszWindowTitle);
        void UpdateElementText(TASKDIALOG_ELEMENTS eElement, LPCTSTR pszNewText) const;

    protected:
        // Override these functions as required
        virtual BOOL OnTDButtonClicked(int nButtonID);
        virtual void OnTDConstructed();
        virtual void OnTDCreated();
        virtual void OnTDDestroyed();
        virtual void OnTDExpandButtonClicked(BOOL IsExpanded);
        virtual void OnTDHelp();
        virtual void OnTDHyperlinkClicked(LPCTSTR pszHref);
        virtual void OnTDNavigatePage();
        virtual BOOL OnTDRadioButtonClicked(int nRadioButtonID);
        virtual BOOL OnTDTimer(DWORD dwTickCount);
        virtual void OnTDVerificationCheckboxClicked(BOOL IsChecked);
        virtual LRESULT TaskDialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

        // Not intended to be overwritten
        LRESULT TaskDialogProcDefault(UINT uMsg, WPARAM wParam, LPARAM lParam);

    private:
        struct TaskButton
        {
            TaskButton(UINT nID, LPCTSTR pszText) : buttonID(nID)
            {
                if (IS_INTRESOURCE(pszText))        // support MAKEINTRESOURCE
                    buttonText = LoadString(reinterpret_cast<UINT>(pszText));
                else
                    buttonText = pszText;
            }

            UINT buttonID;
            CString buttonText;
        };

        CTaskDialog(const CTaskDialog&);                // Disable copy construction
        CTaskDialog& operator = (const CTaskDialog&);   // Disable assignment operator
        CString CTaskDialog::FillString(LPCTSTR pText);
        void Reset();
        static HRESULT CALLBACK StaticTaskDialogProc(HWND hWnd, UINT uNotification, WPARAM wParam, LPARAM lParam, LONG_PTR dwRefData);

        std::vector<TaskButton> m_vButtons;
        std::vector<TaskButton> m_vRadioButtons;

        CString m_strCollapsedControlText;
        CString m_strContent;
        CString m_strExpandedControlText;
        CString m_strExpandedInformation;
        CString m_strFooter;
        CString m_strMainInstruction;
        CString m_strVerificationText;
        CString m_strWindowTitle;

        TASKDIALOGCONFIG m_tc;
        int     m_SelectedButtonID;
        int     m_SelectedRadioButtonID;
        BOOL    m_VerificationCheckboxState;
    };

}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


namespace Win32xx
{

    inline CTaskDialog::CTaskDialog() : m_SelectedButtonID(0), m_SelectedRadioButtonID(0), m_VerificationCheckboxState(FALSE)
    {
        ZeroMemory(&m_tc, sizeof(m_tc));
        m_tc.cbSize = sizeof(m_tc);
        m_tc.pfCallback = CTaskDialog::StaticTaskDialogProc;
    }


    // Adds a command control or push button to the Task Dialog.
    inline void CTaskDialog::AddCommandControl(int nButtonID, LPCTSTR pszCaption)
    {
        assert (GetHwnd() == NULL);

        TaskButton tb(nButtonID, pszCaption);
        m_vButtons.push_back(tb);
    }


    // Adds a radio button to the Task Dialog.
    inline void CTaskDialog::AddRadioButton(int nRadioButtonID, LPCTSTR pszCaption)
    {
        assert (GetHwnd() == NULL);

        TaskButton tb(nRadioButtonID, pszCaption);
        m_vRadioButtons.push_back(tb);
    }


    // Adds a range of radio buttons to the Task Dialog.
    // Assumes the resource ID of the button and it's string match
    inline void CTaskDialog::AddRadioButtonGroup(int nIDRadioButtonsFirst, int nIDRadioButtonsLast)
    {
        assert (GetHwnd() == NULL);
        assert(nIDRadioButtonsFirst > 0);
        assert(nIDRadioButtonsLast > nIDRadioButtonsFirst);

        for (int nID = nIDRadioButtonsFirst; nID <= nIDRadioButtonsLast; ++nID)
        {
            TaskButton tb(nID, MAKEINTRESOURCEW(nID));
            m_vRadioButtons.push_back(tb);
        }
    }


    // Simulates the action of a button click in the Task Dialog.
    inline void CTaskDialog::ClickButton(int nButtonID) const
    {
        assert(GetHwnd());
        SendMessage(TDM_CLICK_BUTTON, nButtonID, 0);
    }


    // Simulates the action of a radio button click in the TaskDialog.
    inline void CTaskDialog::ClickRadioButton(int nRadioButtonID) const
    {
        assert(GetHwnd());
        SendMessage(TDM_CLICK_RADIO_BUTTON, nRadioButtonID, 0);
    }


    // Creates and displays the Task Dialog.
    inline LRESULT CTaskDialog::DoModal(HWND hParent /* = NULL */)
    {
        assert (GetHwnd() == NULL);

        std::vector<TaskButton>::const_iterator it;

        // Build a vector of button info. This will be used later as an array.
        std::vector<TASKDIALOG_BUTTON> vButtons;
        for (it = m_vButtons.begin(); it != m_vButtons.end(); ++it)
        {
            TASKDIALOG_BUTTON tb;
            tb.nButtonID = (*it).buttonID;
            tb.pszButtonText = (*it).buttonText;
            vButtons.push_back(tb);
        }

        // Build a vector of radio button info. This will be used later as an array.
        std::vector<TASKDIALOG_BUTTON> vRadioButtons;
        for (it = m_vRadioButtons.begin(); it != m_vRadioButtons.end(); ++it)
        {
            TASKDIALOG_BUTTON tb;
            tb.nButtonID = (*it).buttonID;
            tb.pszButtonText = (*it).buttonText;
            vRadioButtons.push_back(tb);
        }

        m_hWnd = 0;

        // Fill the TASKDIALOGCONFIG struct.
        m_tc.cbSize = sizeof(m_tc);
        m_tc.pButtons = vButtons.empty()? NULL : &vButtons.front();
        m_tc.cButtons = vButtons.size();
        m_tc.pRadioButtons = vRadioButtons.empty()? NULL : &vRadioButtons.front();
        m_tc.cRadioButtons = vRadioButtons.size();
        m_tc.hwndParent = hParent;

        // Ensure this thread has the TLS index set
        TLSData* pTLSData = GetApp().SetTlsData();

        // Store the CWnd pointer in thread local storage
        pTLSData->pWnd = this;

        // Declare a pointer to the TaskDialogIndirect function
        HMODULE hComCtl = LoadLibrary(_T("COMCTL32.DLL"));
        assert(hComCtl);
        typedef HRESULT WINAPI TASKDIALOGINDIRECT(const TASKDIALOGCONFIG*, int*, int*, BOOL*);
        TASKDIALOGINDIRECT* pTaskDialogIndirect = reinterpret_cast<TASKDIALOGINDIRECT*>(::GetProcAddress(hComCtl, "TaskDialogIndirect"));

        // Call TaskDialogIndirect through our function pointer
        LRESULT lr = pTaskDialogIndirect(&m_tc, &m_SelectedButtonID, &m_SelectedRadioButtonID, &m_VerificationCheckboxState);

        ::FreeLibrary(hComCtl);
        pTLSData->pWnd = NULL;
        m_hWnd = 0;
        Reset();

        if (lr != S_OK)
        {
            // Throw an exception to indicate task dialog creation failure
            if (lr == E_OUTOFMEMORY)
                throw CWinException(_T("TaskDialogIndirect failed, out of memory"));
            if (lr == E_INVALIDARG)
                throw CWinException(_T("TaskDialogIndirect failed, invalid argument"));
            else
                throw CWinException(_T("TaskDialogIndirect failed"));
        }

        return lr;
    }


    // Adds a shield icon to indicate that the button's action requires elevated privileges.
    inline void CTaskDialog::ElevateButton(int nButtonID, BOOL IsElevated) const
    {
        assert(GetHwnd());
        SendMessage(TDM_SET_BUTTON_ELEVATION_REQUIRED_STATE, nButtonID, IsElevated);
    }


    // Enables or disables a push button in the TaskDialog.
    inline void CTaskDialog::EnableButton(int nButtonID, BOOL IsEnabled) const
    {
        assert(GetHwnd());
        SendMessage(TDM_ENABLE_BUTTON, nButtonID, IsEnabled);
    }


    // Enables or disables a radio button in the TaskDialog.
    inline void CTaskDialog::EnableRadioButton(int nRadioButtonID, BOOL IsEnabled) const
    {
        assert(GetHwnd());
        SendMessage(TDM_ENABLE_RADIO_BUTTON, nRadioButtonID, IsEnabled);
    }

    
    // Fills a CString from a string resource or a text string.
    inline CString CTaskDialog::FillString(LPCTSTR pText)
    {
        if (IS_INTRESOURCE(pText))      // support MAKEINTRESOURCE
            return LoadString(reinterpret_cast<UINT>(pText));
        else
            return pText;
    }


    // Returns the TASKDIALOGCONFIG structure for the Task Dialog.
    inline TASKDIALOGCONFIG CTaskDialog::GetConfig() const
    {
        return m_tc;
    }


    // Returns the Task Dialog's options. These are a combination of:
    //  TDF_ENABLE_HYPERLINKS, TDF_USE_HICON_MAIN, TDF_USE_HICON_FOOTER, TDF_ALLOW_DIALOG_CANCELLATION,
    //  TDF_USE_COMMAND_LINKS, TDF_USE_COMMAND_LINKS_NO_ICON, TDF_EXPAND_FOOTER_AREA, TDF_EXPANDED_BY_DEFAULT,
    //  TDF_VERIFICATION_FLAG_CHECKED, TDF_SHOW_PROGRESS_BAR, TDF_SHOW_MARQUEE_PROGRESS_BAR, TDF_CALLBACK_TIMER,
    //  TDF_POSITION_RELATIVE_TO_WINDOW, TDF_RTL_LAYOUT, TDF_NO_DEFAULT_RADIO_BUTTON, TDF_CAN_BE_MINIMIZED.
    inline TASKDIALOG_FLAGS CTaskDialog::GetOptions() const
    {
        return m_tc.dwFlags;
    }


    // Returns the ID of the selected button.
    inline int CTaskDialog::GetSelectedButtonID() const
    {
        assert (GetHwnd() == NULL);
        return m_SelectedButtonID;
    }


    // Returns the ID of the selected radio button.
    inline int CTaskDialog::GetSelectedRadioButtonID() const
    {
        assert (GetHwnd() == NULL);
        return m_SelectedRadioButtonID;
    }


    // Returns the state of the verification check box.
    inline BOOL CTaskDialog::GetVerificationCheckboxState() const
    {
        assert (GetHwnd() == NULL);
        return m_VerificationCheckboxState;
    }


    // Returns true if TaskDialogs are supported on this system.
    inline BOOL CTaskDialog::IsSupported()
    {
        HMODULE hModule = LoadLibrary(_T("COMCTL32.DLL"));
        assert(hModule);

        BOOL Succeeded = (::GetProcAddress(hModule, "TaskDialogIndirect") != FALSE);

        ::FreeLibrary(hModule);
        return Succeeded;
    }


    // Replaces the information displayed by the task dialog.
    inline void CTaskDialog::NavigateTo(CTaskDialog& TaskDialog) const
    {
        assert(GetHwnd());
        TASKDIALOGCONFIG tc = TaskDialog.GetConfig();
        SendMessage(TDM_NAVIGATE_PAGE, 0, reinterpret_cast<LPARAM>(&tc));
    }


    // Called when the user selects a button or command link.
    inline BOOL CTaskDialog::OnTDButtonClicked(int nButtonID)
    {
        UNREFERENCED_PARAMETER(nButtonID);

        // return TRUE to prevent the task dialog from closing
        return FALSE;
    }


    // Called when the task dialog is constructed, before it is displayed.
    inline void CTaskDialog::OnTDConstructed()
    {
    }


    // Called when the task dialog is displayed.
    inline void CTaskDialog::OnTDCreated()
    {
    }


    // Called when the task dialog is destroyed.
    inline void CTaskDialog::OnTDDestroyed()
    {
        Reset();
    }


    // Called when the expand button is clicked.
    inline void CTaskDialog::OnTDExpandButtonClicked(BOOL IsExpanded)
    {
        UNREFERENCED_PARAMETER(IsExpanded);
    }


    // Called when the user presses F1 on the keyboard.
    inline void CTaskDialog::OnTDHelp()
    {
    }


    // Called when the user clicks on a hyperlink.
    inline void CTaskDialog::OnTDHyperlinkClicked(LPCTSTR pszHref)
    {
        UNREFERENCED_PARAMETER(pszHref);
    }


    // Called when a navigation has occurred.
    inline void CTaskDialog::OnTDNavigatePage()
    {
    }


    // Called when the user selects a radio button.
    inline BOOL CTaskDialog::OnTDRadioButtonClicked(int nRadioButtonID)
    {
        UNREFERENCED_PARAMETER(nRadioButtonID);
        return TRUE;
    }


    // Called every 200 milliseconds (approximately) when the TDF_CALLBACK_TIMER flag is set.
    inline BOOL CTaskDialog::OnTDTimer(DWORD dwTickCount)
    {
        UNREFERENCED_PARAMETER(dwTickCount);

        // return TRUE to reset the tick count
        return FALSE;
    }


    // Called when the user clicks the Task Dialog verification check box.
    inline void CTaskDialog::OnTDVerificationCheckboxClicked(BOOL IsChecked)
    {
        UNREFERENCED_PARAMETER(IsChecked);
    }


    // Returns the dialog to its default state.
    inline void CTaskDialog::Reset()
    {
        assert (GetHwnd() == NULL);

        m_vButtons.clear();
        m_vRadioButtons.clear();
        ZeroMemory(&m_tc, sizeof(m_tc));
        m_tc.cbSize = sizeof(m_tc);
        m_tc.pfCallback = CTaskDialog::StaticTaskDialogProc;

        m_SelectedButtonID = 0;
        m_SelectedRadioButtonID = 0;
        m_VerificationCheckboxState = FALSE;

        m_strCollapsedControlText.Empty();
        m_strContent.Empty();
        m_strExpandedControlText.Empty();
        m_strExpandedInformation.Empty();
        m_strFooter.Empty();
        m_strMainInstruction.Empty();
        m_strVerificationText.Empty();
        m_strWindowTitle.Empty();
    }


    // The dwCommonButtons parameter can be a combination of:
    //  TDCBF_OK_BUTTON         OK button
    //  TDCBF_YES_BUTTON        Yes button
    //  TDCBF_NO_BUTTON         No button
    //  TDCBF_CANCEL_BUTTON     Cancel button
    //  TDCBF_RETRY_BUTTON      Retry button
    //  TDCBF_CLOSE_BUTTON      Close button
    inline void CTaskDialog::SetCommonButtons(TASKDIALOG_COMMON_BUTTON_FLAGS dwCommonButtons)
    {
        assert (GetHwnd() == NULL);
        m_tc.dwCommonButtons = dwCommonButtons;
    }


    // Sets the task dialog's primary content.
    inline void CTaskDialog::SetContent(LPCTSTR pszContent)
    {
        m_strContent = FillString(pszContent);
        m_tc.pszContent = m_strContent.c_str();

        if (IsWindow())
            SendMessage(TDM_SET_ELEMENT_TEXT, TDE_CONTENT, reinterpret_cast<LPARAM>(m_strContent.c_str()));
    }


    // Sets the task dialog's default button.
    // Can be either a button ID or one of the common buttons.
    inline void CTaskDialog::SetDefaultButton(int nButtonID)
    {
        assert (GetHwnd() == NULL);
        m_tc.nDefaultButton = nButtonID;
    }


    // Sets the default radio button.
    inline void CTaskDialog::SetDefaultRadioButton(int nRadioButtonID)
    {
        assert (GetHwnd() == NULL);
        m_tc.nDefaultRadioButton = nRadioButtonID;
    }


    // The width of the task dialog's client area. If 0, the
    // task dialog manager will calculate the ideal width.
    inline void CTaskDialog::SetDialogWidth(UINT nWidth /*= 0*/)
    {
        assert (GetHwnd() == NULL);
        m_tc.cxWidth = nWidth;
    }


    // Sets the text in the expandable area of the Task Dialog.
    inline void CTaskDialog::SetExpansionArea(LPCTSTR pszExpandedInfo, LPCTSTR pszExpandedLabel /* = _T("")*/, LPCTSTR pszCollapsedLabel /* = _T("")*/)
    {
        m_strExpandedInformation = FillString(pszExpandedInfo);
        m_tc.pszExpandedInformation = m_strExpandedInformation.c_str();

        m_strExpandedControlText = FillString(pszExpandedLabel);
        m_tc.pszExpandedControlText = m_strExpandedControlText.c_str();

        m_strCollapsedControlText = FillString(pszCollapsedLabel);
        m_tc.pszCollapsedControlText = m_strCollapsedControlText.c_str();

        if (IsWindow())
            SendMessage(TDM_SET_ELEMENT_TEXT, TDE_EXPANDED_INFORMATION, reinterpret_cast<LPARAM>(m_strExpandedInformation.c_str()));
    }


    // Sets the icon that will be displayed in the Task Dialog's footer.
    inline void CTaskDialog::SetFooterIcon(HICON hFooterIcon)
    {
        m_tc.hFooterIcon = hFooterIcon;

        if (IsWindow())
            SendMessage(TDM_UPDATE_ICON, TDIE_ICON_FOOTER, reinterpret_cast<LPARAM>(hFooterIcon));
    }


    // Sets the icon that will be displayed in the Task Dialog's footer.
    // Possible icons:
    // TD_ERROR_ICON        A stop-sign icon appears in the task dialog.
    // TD_WARNING_ICON      An exclamation-point icon appears in the task dialog.
    // TD_INFORMATION_ICON  An icon consisting of a lowercase letter i in a circle appears in the task dialog.
    // TD_SHIELD_ICON       A shield icon appears in the task dialog.
    //  or a value passed via MAKEINTRESOURCE
    inline void CTaskDialog::SetFooterIcon(LPCTSTR lpszFooterIcon)
    {
        m_tc.pszFooterIcon = const_cast<LPCWSTR>(lpszFooterIcon);

        if (IsWindow())
            SendMessage(TDM_UPDATE_ICON, TDIE_ICON_FOOTER, reinterpret_cast<LPARAM>(lpszFooterIcon));
    }


    // Sets the text that will be displayed in the Task Dialog's footer.
    inline void CTaskDialog::SetFooterText(LPCTSTR pszFooter)
    {
        m_strFooter = FillString(pszFooter);
        m_tc.pszFooter = m_strFooter.c_str();

        if (IsWindow())
            SendMessage(TDM_SET_ELEMENT_TEXT, TDE_FOOTER, reinterpret_cast<LPARAM>(m_strFooter.c_str()));
    }


    // Sets Task Dialog's main icon.
    inline void CTaskDialog::SetMainIcon(HICON hMainIcon)
    {
        m_tc.hMainIcon = hMainIcon;

        if (IsWindow())
            SendMessage(TDM_UPDATE_ICON, TDIE_ICON_MAIN, reinterpret_cast<LPARAM>(hMainIcon));
    }


    // Sets Task Dialog's main icon.
    // Possible icons:
    // TD_ERROR_ICON        A stop-sign icon appears in the task dialog.
    // TD_WARNING_ICON      An exclamation-point icon appears in the task dialog.
    // TD_INFORMATION_ICON  An icon consisting of a lowercase letter i in a circle appears in the task dialog.
    // TD_SHIELD_ICON       A shield icon appears in the task dialog.
    inline void CTaskDialog::SetMainIcon(LPCTSTR lpszMainIcon)
    {
        m_tc.pszMainIcon = const_cast<LPCWSTR>(lpszMainIcon);

        if (IsWindow())
            SendMessage(TDM_UPDATE_ICON, TDIE_ICON_MAIN, reinterpret_cast<LPARAM>(lpszMainIcon));
    }


    // Sets the Task Dialog's main instruction text.
    inline void CTaskDialog::SetMainInstruction(LPCTSTR pszMainInstruction)
    {
        m_strMainInstruction = FillString(pszMainInstruction);
        m_tc.pszMainInstruction = m_strMainInstruction.c_str();

        if (IsWindow())
            SendMessage(TDM_SET_ELEMENT_TEXT, TDE_FOOTER, reinterpret_cast<LPARAM>(m_strMainInstruction.c_str()));
    }


    // Sets the Task Dialog's options. These are a combination of:
    //  TDF_ENABLE_HYPERLINKS, TDF_USE_HICON_MAIN, TDF_USE_HICON_FOOTER, TDF_ALLOW_DIALOG_CANCELLATION,
    //  TDF_USE_COMMAND_LINKS, TDF_USE_COMMAND_LINKS_NO_ICON, TDF_EXPAND_FOOTER_AREA, TDF_EXPANDED_BY_DEFAULT,
    //  TDF_VERIFICATION_FLAG_CHECKED, TDF_SHOW_PROGRESS_BAR, TDF_SHOW_MARQUEE_PROGRESS_BAR, TDF_CALLBACK_TIMER,
    //  TDF_POSITION_RELATIVE_TO_WINDOW, TDF_RTL_LAYOUT, TDF_NO_DEFAULT_RADIO_BUTTON, TDF_CAN_BE_MINIMIZED.
    inline void CTaskDialog::SetOptions(TASKDIALOG_FLAGS dwFlags)
    {
        assert (GetHwnd() == NULL);
        m_tc.dwFlags = dwFlags;
    }


    // Starts and stops the marquee display of the progress bar, and sets the speed of the marquee.
    inline void CTaskDialog::SetProgressBarMarquee(BOOL IsEnabled /* = TRUE*/, int nMarqueeSpeed /* = 0*/) const
    {
        assert(GetHwnd());
        SendMessage(TDM_SET_PROGRESS_BAR_MARQUEE, IsEnabled, nMarqueeSpeed);
    }


    // Sets the current position for a progress bar.
    inline void CTaskDialog::SetProgressBarPosition(int nProgressPos) const
    {
        assert(GetHwnd());
        SendMessage(TDM_SET_PROGRESS_BAR_POS, nProgressPos, 0);
    }


    // Sets the minimum and maximum values for the hosted progress bar.
    inline void CTaskDialog::SetProgressBarRange(int nMinRange, int nMaxRange) const
    {
        assert(GetHwnd());
        SendMessage(TDM_SET_PROGRESS_BAR_RANGE, 0, MAKELPARAM(nMinRange, nMaxRange));
    }


    // Sets the current state of the progress bar. Possible states are:
    //  PBST_NORMAL, PBST_PAUSE, PBST_ERROR.
    inline void CTaskDialog::SetProgressBarState(int nNewState /* = PBST_NORMAL*/) const
    {
        assert(GetHwnd());
        SendMessage(TDM_SET_PROGRESS_BAR_STATE, nNewState, 0);
    }


    // Simulates a click on the verification checkbox of the Task Dialog, if it exists.
    inline void CTaskDialog::SetVerificationCheckbox(BOOL IsChecked) const
    {
        assert(GetHwnd());
        SendMessage(TDM_CLICK_VERIFICATION, IsChecked, IsChecked);
    }


    // Sets the text for the verification check box.
    inline void CTaskDialog::SetVerificationCheckboxText(LPCTSTR pszVerificationText)
    {
        assert (GetHwnd() == NULL);
        m_strVerificationText = FillString(pszVerificationText);
        m_tc.pszVerificationText = m_strVerificationText.c_str();
    }


    // Sets the Task Dialog's window title.
    inline void CTaskDialog::SetWindowTitle(LPCTSTR pszWindowTitle)
    {
        assert (GetHwnd() == NULL);
        m_strWindowTitle = FillString(pszWindowTitle);
        m_tc.pszWindowTitle = m_strWindowTitle.c_str();
    }


    // TaskDialogs direct their messages here.
    inline HRESULT CALLBACK CTaskDialog::StaticTaskDialogProc(HWND hWnd, UINT uNotification, WPARAM wParam, LPARAM lParam, LONG_PTR dwRefData)
    {
        UNREFERENCED_PARAMETER(dwRefData);

        assert( &GetApp() );

        CTaskDialog* t = static_cast<CTaskDialog*>(GetCWndPtr(hWnd));
        if (t == 0)
        {
            // The CTaskDialog pointer wasn't found in the map, so add it now

            // Retrieve the pointer to the TLS Data
            TLSData* pTLSData = GetApp().GetTlsData();
            assert(pTLSData);

            // Retrieve pointer to CTaskDialog object from Thread Local Storage TLS
            t = static_cast<CTaskDialog*>(pTLSData->pWnd);
            assert(t);
            pTLSData->pWnd = NULL;

            // Store the CTaskDialog pointer in the HWND map
            t->m_hWnd = hWnd;
            t->AddToMap();
        }

        return t->TaskDialogProc(uNotification, wParam, lParam);

    } // LRESULT CALLBACK StaticTaskDialogProc(...)


    // Provides default handling of Task Dialog's messages.
    inline LRESULT CTaskDialog::TaskDialogProcDefault(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        switch(uMsg)
        {
        case TDN_BUTTON_CLICKED:
            return OnTDButtonClicked(wParam);

        case TDN_CREATED:
            OnTDCreated();
            break;
        case TDN_DESTROYED:
            OnTDDestroyed();
            break;
        case TDN_DIALOG_CONSTRUCTED:
            OnTDConstructed();
            break;
        case TDN_EXPANDO_BUTTON_CLICKED:
            OnTDExpandButtonClicked(wParam);
            break;
        case TDN_HELP:
            OnTDHelp();
            break;
        case TDN_HYPERLINK_CLICKED:
            OnTDHyperlinkClicked(WtoT(reinterpret_cast<LPCWSTR>(lParam)));
            break;
        case TDN_NAVIGATED:
            OnTDNavigatePage();
            break;
        case TDN_RADIO_BUTTON_CLICKED:
            OnTDRadioButtonClicked(wParam);
            break;
        case TDN_TIMER:
            return OnTDTimer(wParam);

        case TDN_VERIFICATION_CLICKED:
            OnTDVerificationCheckboxClicked(wParam);
            break;
        }

        return S_OK;
    }


    // Override this function modify how the task dialog's messages are handled.
    inline LRESULT CTaskDialog::TaskDialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        // A typical function might look like this:

        //  switch (uMsg)
        //  {
        //  case MESSAGE1:      // Some Windows API message
        //      OnMessage1();   // A user defined function
        //      break;          // Also do default processing
        //  case MESSAGE2:
        //      OnMessage2();
        //      return x;       // Don't do default processing, but instead return
        //                      //  a value recommended by the Windows API documentation
        //  }

        // Always pass unhandled messages on to TaskDialogProcDefault
        return TaskDialogProcDefault(uMsg, wParam, lParam);
    }


    // Updates a text element on the TaskDialog after it is created. The size of the TaskDialog
    // is not adjusted to accomodate the new text.
    // Possible eElement values are:
    // TDE_CONTENT, TDE_EXPANDED_INFORMATION, TDE_FOOTER, TDE_MAIN_INSTRUCTION.
    inline void CTaskDialog::UpdateElementText(TASKDIALOG_ELEMENTS eElement, LPCTSTR pszNewText) const
    {
        assert(GetHwnd());
        CString NewText = pszNewText;
        SendMessage(TDM_UPDATE_ELEMENT_TEXT, eElement, reinterpret_cast<LPARAM>(NewText.c_str()));
    }

}



#endif // _WIN32XX_TASKDIALOG_H_
