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

    //////////////////////////////////////////////////////////////////
    // CTaskDialog manages a task dialog. A task dialog is similar to,
    // while much more flexible than, a basic message box.
    class CTaskDialog : public CWnd
    {
    public:
        CTaskDialog();
        virtual ~CTaskDialog() {}

        void AddCommandControl(int buttonID, LPCWSTR caption);
        void AddRadioButton(int radioButtonID, LPCWSTR caption);
        void AddRadioButtonGroup(int firstRadioButtonID, int lastRadioButtonID);
        void ClickButton(int buttonID) const;
        void ClickRadioButton(int radioButtonID) const;
        HRESULT DoModal(HWND parent = 0);
        void ElevateButton(int buttonID, BOOL isElevated) const;
        void EnableButton(int buttonID, BOOL isEnabled) const;
        void EnableRadioButton(int buttonID, BOOL isEnabled) const;
        TASKDIALOGCONFIG GetConfig() const;
        TASKDIALOG_FLAGS GetOptions() const;
        int GetSelectedButtonID() const;
        int GetSelectedRadioButtonID() const;
        BOOL GetVerificationCheckboxState() const;
        static BOOL IsSupported();
        void NavigateTo(CTaskDialog& taskDialog) const;
        void SetCommonButtons(TASKDIALOG_COMMON_BUTTON_FLAGS commonButtons);
        void SetContent(LPCWSTR content);
        void SetDefaultButton(int buttonID);
        void SetDefaultRadioButton(int radioButtonID);
        void SetDialogWidth(UINT width = 0);
        void SetExpansionArea(LPCWSTR expandedInfo, LPCWSTR expandedLabel = L"", LPCWSTR collapsedLabel = L"");
        void SetFooterIcon(HICON icon);
        void SetFooterIcon(LPCWSTR footerIcon);
        void SetFooterText(LPCWSTR text);
        void SetMainIcon(HICON icon);
        void SetMainIcon(LPCWSTR mainIcon);
        void SetMainInstruction(LPCWSTR mainInstruction);
        void SetOptions(TASKDIALOG_FLAGS flags);
        void SetProgressBarMarquee(BOOL isEnabled = TRUE, int marqueeSpeed = 0) const;
        void SetProgressBarPosition(int progressPos) const;
        void SetProgressBarRange(int minRange, int maxRange) const;
        void SetProgressBarState(int newState = PBST_NORMAL) const;
        void SetVerificationCheckbox(BOOL isChecked) const;
        void SetVerificationCheckboxText(LPCWSTR verificationText);
        void SetWindowTitle(LPCWSTR windowTitle);
        void UpdateElementText(TASKDIALOG_ELEMENTS element, LPCWSTR newText) const;

    protected:
        // Override these functions as required
        virtual BOOL OnTDButtonClicked(int buttonID);
        virtual void OnTDConstructed();
        virtual void OnTDCreated();
        virtual void OnTDDestroyed();
        virtual void OnTDExpandButtonClicked(BOOL isExpanded);
        virtual void OnTDHelp();
        virtual void OnTDHyperlinkClicked(LPCWSTR hyperLink);
        virtual void OnTDNavigatePage();
        virtual BOOL OnTDRadioButtonClicked(int radioButtonID);
        virtual BOOL OnTDTimer(DWORD tickCount);
        virtual void OnTDVerificationCheckboxClicked(BOOL isChecked);
        virtual LRESULT TaskDialogProc(UINT msg, WPARAM wparam, LPARAM lparam);

        // Not intended to be overwritten
        LRESULT TaskDialogProcDefault(UINT msg, WPARAM wparam, LPARAM lparam);

    private:
        struct TaskButton
        {
            TaskButton(UINT id, LPCWSTR text) : buttonID(id)
            {
                if (IS_INTRESOURCE(text))        // support MAKEINTRESOURCE
                    buttonText.LoadString((UINT)(UINT_PTR)text);
                else
                    buttonText = text;
            }

            UINT buttonID;
            CStringW buttonText;
        };

        using CWnd::WndProc;                            // Make WndProc private
        CTaskDialog(const CTaskDialog&);                // Disable copy construction
        CTaskDialog& operator = (const CTaskDialog&);   // Disable assignment operator

        CStringW FillString(LPCWSTR text);
        static HRESULT CALLBACK StaticTaskDialogProc(HWND wnd, UINT notification, WPARAM wparam, LPARAM lparam, LONG_PTR refData);

        std::vector<TaskButton> m_buttons;
        std::vector<TaskButton> m_radioButtons;

        CStringW m_collapsedControlText;
        CStringW m_content;
        CStringW m_expandedControlText;
        CStringW m_expandedInformation;
        CStringW m_footer;
        CStringW m_mainInstruction;
        CStringW m_verificationText;
        CStringW m_windowTitle;

        TASKDIALOGCONFIG m_tc;
        int     m_selectedButtonID;
        int     m_selectedRadioButtonID;
        BOOL    m_verificationCheckboxState;
    };

}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


namespace Win32xx
{

    inline CTaskDialog::CTaskDialog() : m_selectedButtonID(0), m_selectedRadioButtonID(0), m_verificationCheckboxState(FALSE)
    {
        ZeroMemory(&m_tc, sizeof(m_tc));
        m_tc.cbSize = sizeof(m_tc);
        m_tc.pfCallback = CTaskDialog::StaticTaskDialogProc;
    }

    // Adds a command control or push button to the Task Dialog.
    inline void CTaskDialog::AddCommandControl(int buttonID, LPCWSTR caption)
    {
        assert (GetHwnd() == 0);

        TaskButton tb(buttonID, caption);
        m_buttons.push_back(tb);
    }

    // Adds a radio button to the Task Dialog.
    inline void CTaskDialog::AddRadioButton(int radioButtonID, LPCWSTR caption)
    {
        assert (GetHwnd() == 0);

        TaskButton tb(radioButtonID, caption);
        m_radioButtons.push_back(tb);
    }

    // Adds a range of radio buttons to the Task Dialog.
    // Assumes the resource ID of the button and it's string match
    inline void CTaskDialog::AddRadioButtonGroup(int firstRadioButtonID, int lastRadioButtonID)
    {
        assert (GetHwnd() == 0);
        assert(firstRadioButtonID > 0);
        assert(lastRadioButtonID > firstRadioButtonID);

        for (int id = firstRadioButtonID; id <= lastRadioButtonID; ++id)
        {
            TaskButton tb(id, MAKEINTRESOURCEW(id));
            m_radioButtons.push_back(tb);
        }
    }

    // Simulates the action of a button click in the Task Dialog.
    // Refer to TDM_CLICK_BUTTON in the Windows API documentation for more information.
    inline void CTaskDialog::ClickButton(int buttonID) const
    {
        assert(GetHwnd());
        SendMessage(TDM_CLICK_BUTTON, (WPARAM)buttonID, 0);
    }

    // Simulates the action of a radio button click in the TaskDialog.
    // Refer to TDM_CLICK_RADIO_BUTTON in the Windows API documentation for more information.
    inline void CTaskDialog::ClickRadioButton(int radioButtonID) const
    {
        assert(GetHwnd());
        SendMessage(TDM_CLICK_RADIO_BUTTON, (WPARAM)radioButtonID, 0);
    }

    // Creates and displays the Task Dialog.
    // Refer to TaskDialogIndirect in the Windows API documentation for more information.
    inline HRESULT CTaskDialog::DoModal(HWND parent /* = 0 */)
    {
        assert (GetHwnd() == 0);
        m_selectedButtonID = 0;
        m_selectedRadioButtonID = 0;
        m_verificationCheckboxState = FALSE;

        std::vector<TaskButton>::const_iterator it;

        // Build a vector of button info. This will be used later as an array.
        std::vector<TASKDIALOG_BUTTON> buttons;
        for (it = m_buttons.begin(); it != m_buttons.end(); ++it)
        {
            TASKDIALOG_BUTTON tb;
            tb.nButtonID = (*it).buttonID;
            tb.pszButtonText = (*it).buttonText;
            buttons.push_back(tb);
        }

        // Build a vector of radio button info. This will be used later as an array.
        std::vector<TASKDIALOG_BUTTON> radioButtons;
        for (it = m_radioButtons.begin(); it != m_radioButtons.end(); ++it)
        {
            TASKDIALOG_BUTTON tb;
            tb.nButtonID = (*it).buttonID;
            tb.pszButtonText = (*it).buttonText;
            radioButtons.push_back(tb);
        }

        m_wnd = 0;

        // Fill the TASKDIALOGCONFIG struct.
        m_tc.cbSize = sizeof(m_tc);
        m_tc.pButtons = buttons.empty()? NULL : &buttons.front();
        m_tc.cButtons = static_cast<UINT>(buttons.size());
        m_tc.pRadioButtons = radioButtons.empty()? NULL : &radioButtons.front();
        m_tc.cRadioButtons = static_cast<UINT>(radioButtons.size());
        m_tc.hwndParent = parent;

        // Retrieve this thread's TLS data
        TLSData* pTLSData = GetApp()->GetTlsData();

        // Store the CWnd pointer in thread local storage.
        pTLSData->pWnd = this;

        // Declare a pointer to the TaskDialogIndirect function.
        HMODULE comCtl = LoadLibraryW(L"COMCTL32.DLL");
        assert(comCtl);
        typedef HRESULT WINAPI TASKDIALOGINDIRECT(const TASKDIALOGCONFIG*, int*, int*, BOOL*);
        HRESULT result = E_FAIL;

        if (comCtl)
        {
            TASKDIALOGINDIRECT* pTaskDialogIndirect = reinterpret_cast<TASKDIALOGINDIRECT*>(::GetProcAddress(comCtl, "TaskDialogIndirect"));

            // Call TaskDialogIndirect through our function pointer.
            result = pTaskDialogIndirect(&m_tc, &m_selectedButtonID, &m_selectedRadioButtonID, &m_verificationCheckboxState);

            ::FreeLibrary(comCtl);
        }
        pTLSData->pWnd = NULL;
        Cleanup();

        if (result != S_OK)
        {
            // Throw an exception to indicate task dialog creation failure.
            throw CWinException(GetApp()->MsgTaskDialog());
        }

        return result;
    }

    // Adds a shield icon to indicate that the button's action requires elevated privileges.
    // Refer to TDM_SET_BUTTON_ELEVATION_REQUIRED_STATE in the Windows API documentation for more information.
    inline void CTaskDialog::ElevateButton(int buttonID, BOOL isElevated) const
    {
        assert(GetHwnd());
        SendMessage(TDM_SET_BUTTON_ELEVATION_REQUIRED_STATE, (WPARAM)buttonID, (LPARAM)isElevated);
    }

    // Enables or disables a push button in the TaskDialog.
    // Refer to TDM_ENABLE_BUTTON in the Windows API documentation for more information.
    inline void CTaskDialog::EnableButton(int buttonID, BOOL isEnabled) const
    {
        assert(GetHwnd());
        SendMessage(TDM_ENABLE_BUTTON, (WPARAM)buttonID, (LPARAM)isEnabled);
    }

    // Enables or disables a radio button in the TaskDialog.
    // Refer to TDM_ENABLE_RADIO_BUTTON in the Windows API documentation for more information.
    inline void CTaskDialog::EnableRadioButton(int radioButtonID, BOOL isEnabled) const
    {
        assert(GetHwnd());
        SendMessage(TDM_ENABLE_RADIO_BUTTON, (WPARAM)radioButtonID, (LPARAM)isEnabled);
    }

    // Fills a CString from a string resource or a text string.
    inline CStringW CTaskDialog::FillString(LPCWSTR text)
    {
        CStringW str;
        if (IS_INTRESOURCE(text))      // support MAKEINTRESOURCE
            str.LoadString((UINT)(UINT_PTR)text);
        else
            str = text;

        return str;
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
        assert (GetHwnd() == 0);
        return m_selectedButtonID;
    }

    // Returns the ID of the selected radio button.
    inline int CTaskDialog::GetSelectedRadioButtonID() const
    {
        assert (GetHwnd() == 0);
        return m_selectedRadioButtonID;
    }

    // Returns the state of the verification check box.
    inline BOOL CTaskDialog::GetVerificationCheckboxState() const
    {
        assert (GetHwnd() == 0);
        return m_verificationCheckboxState;
    }

    // Returns true if TaskDialogs are supported on this system.
    inline BOOL CTaskDialog::IsSupported()
    {
        BOOL result = FALSE;
        HMODULE comctl = LoadLibraryW(L"COMCTL32.DLL");
        assert(comctl);
        if (comctl)
        {
            result = (::GetProcAddress(comctl, "TaskDialogIndirect") != FALSE);
            ::FreeLibrary(comctl);
        }

        return result;
    }

    // Replaces the information displayed by the task dialog.
    // Refer to TDM_NAVIGATE_PAGE in the Windows API documentation for more information.
    inline void CTaskDialog::NavigateTo(CTaskDialog& taskDialog) const
    {
        assert(GetHwnd());
        TASKDIALOGCONFIG tc = taskDialog.GetConfig();
        SendMessage(TDM_NAVIGATE_PAGE, 0, (WPARAM)&tc);
    }

    // Called when the user selects a button or command link.
    inline BOOL CTaskDialog::OnTDButtonClicked(int)
    {
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
    }

    // Called when the expand button is clicked.
    inline void CTaskDialog::OnTDExpandButtonClicked(BOOL)
    {
    }

    // Called when the user presses F1 on the keyboard.
    inline void CTaskDialog::OnTDHelp()
    {
    }

    // Called when the user clicks on a hyperlink.
    inline void CTaskDialog::OnTDHyperlinkClicked(LPCWSTR)
    {
    }

    // Called when a navigation has occurred.
    inline void CTaskDialog::OnTDNavigatePage()
    {
    }

    // Called when the user selects a radio button.
    inline BOOL CTaskDialog::OnTDRadioButtonClicked(int)
    {
        return TRUE;
    }

    // Called approximately every 200 milliseconds when the TDF_CALLBACK_TIMER flag is set.
    inline BOOL CTaskDialog::OnTDTimer(DWORD)
    {
        // Return TRUE to reset the tick count.
        return FALSE;
    }

    // Called when the user clicks the Task Dialog verification check box.
    inline void CTaskDialog::OnTDVerificationCheckboxClicked(BOOL)
    {
    }

    // The commonButtons parameter can be a combination of:
    //  TDCBF_OK_BUTTON         OK button
    //  TDCBF_YES_BUTTON        Yes button
    //  TDCBF_NO_BUTTON         No button
    //  TDCBF_CANCEL_BUTTON     Cancel button
    //  TDCBF_RETRY_BUTTON      Retry button
    //  TDCBF_CLOSE_BUTTON      Close button
    inline void CTaskDialog::SetCommonButtons(TASKDIALOG_COMMON_BUTTON_FLAGS commonButtons)
    {
        assert (GetHwnd() == 0);
        m_tc.dwCommonButtons = commonButtons;
    }

    // Sets the task dialog's primary content.
    // Refer to TDM_SET_ELEMENT_TEXT in the Windows API documentation for more information.
    inline void CTaskDialog::SetContent(LPCWSTR content)
    {
        m_content = FillString(content);
        m_tc.pszContent = m_content;

        if (IsWindow())
            SendMessage(TDM_SET_ELEMENT_TEXT, (WPARAM)TDE_CONTENT, (LPARAM)m_content.c_str());
    }

    // Sets the task dialog's default button.
    // Can be either a button ID or one of the common buttons.
    inline void CTaskDialog::SetDefaultButton(int buttonID)
    {
        assert (GetHwnd() == 0);
        m_tc.nDefaultButton = buttonID;
    }

    // Sets the default radio button.
    inline void CTaskDialog::SetDefaultRadioButton(int radioButtonID)
    {
        assert (GetHwnd() == 0);
        m_tc.nDefaultRadioButton = radioButtonID;
    }

    // The width of the task dialog's client area. If 0, the
    // task dialog manager will calculate the ideal width.
    inline void CTaskDialog::SetDialogWidth(UINT width /*= 0*/)
    {
        assert (GetHwnd() == 0);
        m_tc.cxWidth = width;
    }

    // Sets the text in the expandable area of the Task Dialog.
    // Refer to TDM_SET_ELEMENT_TEXT in the Windows API documentation for more information.
    inline void CTaskDialog::SetExpansionArea(LPCWSTR expandedInfo, LPCWSTR expandedLabel, LPCWSTR collapsedLabel)
    {
        m_expandedInformation = FillString(expandedInfo);
        m_tc.pszExpandedInformation = m_expandedInformation;

        m_expandedControlText = FillString(expandedLabel);
        m_tc.pszExpandedControlText = m_expandedControlText;

        m_collapsedControlText = FillString(collapsedLabel);
        m_tc.pszCollapsedControlText = m_collapsedControlText;

        if (IsWindow())
            SendMessage(TDM_SET_ELEMENT_TEXT, (WPARAM)TDE_EXPANDED_INFORMATION, (LPARAM)m_expandedInformation.c_str());
    }

    // Sets the icon that will be displayed in the Task Dialog's footer.
    // Refer to TDM_UPDATE_ICON in the Windows API documentation for more information.
    inline void CTaskDialog::SetFooterIcon(HICON footerIcon)
    {
        m_tc.hFooterIcon = footerIcon;

        if (IsWindow())
            SendMessage(TDM_UPDATE_ICON, (WPARAM)TDIE_ICON_FOOTER, (LPARAM)footerIcon);
    }

    // Sets the icon that will be displayed in the Task Dialog's footer.
    // Possible icons:
    // TD_ERROR_ICON        A stop-sign icon appears in the task dialog.
    // TD_WARNING_ICON      An exclamation-point icon appears in the task dialog.
    // TD_INFORMATION_ICON  An icon consisting of a lowercase letter i in a circle appears in the task dialog.
    // TD_SHIELD_ICON       A shield icon appears in the task dialog.
    //  or a value passed via MAKEINTRESOURCE
    // Refer to TDM_UPDATE_ICON in the Windows API documentation for more information.
    inline void CTaskDialog::SetFooterIcon(LPCWSTR footerIcon)
    {
        m_tc.pszFooterIcon = const_cast<LPCWSTR>(footerIcon);

        if (IsWindow())
            SendMessage(TDM_UPDATE_ICON, (WPARAM)TDIE_ICON_FOOTER, (LPARAM)footerIcon);
    }

    // Sets the text that will be displayed in the Task Dialog's footer.
    // Refer to TDM_SET_ELEMENT_TEXT in the Windows API documentation for more information.
    inline void CTaskDialog::SetFooterText(LPCWSTR footer)
    {
        m_footer = FillString(footer);
        m_tc.pszFooter = m_footer;

        if (IsWindow())
            SendMessage(TDM_SET_ELEMENT_TEXT, (WPARAM)TDE_FOOTER, (LPARAM)(m_footer.c_str()));
    }

    // Sets Task Dialog's main icon.
    // Refer to TDM_UPDATE_ICON in the Windows API documentation for more information.
    inline void CTaskDialog::SetMainIcon(HICON mainIcon)
    {
        m_tc.hMainIcon = mainIcon;

        if (IsWindow())
            SendMessage(TDM_UPDATE_ICON, (WPARAM)TDIE_ICON_MAIN, (LPARAM)mainIcon);
    }

    // Sets Task Dialog's main icon.
    // Possible icons:
    // TD_ERROR_ICON        A stop-sign icon appears in the task dialog.
    // TD_WARNING_ICON      An exclamation-point icon appears in the task dialog.
    // TD_INFORMATION_ICON  An icon consisting of a lowercase letter i in a circle appears in the task dialog.
    // TD_SHIELD_ICON       A shield icon appears in the task dialog.
    // Refer to TDM_UPDATE_ICON in the Windows API documentation for more information.
    inline void CTaskDialog::SetMainIcon(LPCWSTR mainIcon)
    {
        m_tc.pszMainIcon = const_cast<LPCWSTR>(mainIcon);

        if (IsWindow())
            SendMessage(TDM_UPDATE_ICON, (WPARAM)TDIE_ICON_MAIN, (LPARAM)mainIcon);
    }

    // Sets the Task Dialog's main instruction text.
    // Refer to TDM_SET_ELEMENT_TEXT in the Windows API documentation for more information.
    inline void CTaskDialog::SetMainInstruction(LPCWSTR mainInstruction)
    {
        m_mainInstruction = FillString(mainInstruction);
        m_tc.pszMainInstruction = m_mainInstruction;

        if (IsWindow())
            SendMessage(TDM_SET_ELEMENT_TEXT, (WPARAM)TDE_FOOTER, (LPARAM)m_mainInstruction.c_str());
    }

    // Sets the Task Dialog's options. These are a combination of:
    //  TDF_ENABLE_HYPERLINKS, TDF_USE_HICON_MAIN, TDF_USE_HICON_FOOTER, TDF_ALLOW_DIALOG_CANCELLATION,
    //  TDF_USE_COMMAND_LINKS, TDF_USE_COMMAND_LINKS_NO_ICON, TDF_EXPAND_FOOTER_AREA, TDF_EXPANDED_BY_DEFAULT,
    //  TDF_VERIFICATION_FLAG_CHECKED, TDF_SHOW_PROGRESS_BAR, TDF_SHOW_MARQUEE_PROGRESS_BAR, TDF_CALLBACK_TIMER,
    //  TDF_POSITION_RELATIVE_TO_WINDOW, TDF_RTL_LAYOUT, TDF_NO_DEFAULT_RADIO_BUTTON, TDF_CAN_BE_MINIMIZED.
    inline void CTaskDialog::SetOptions(TASKDIALOG_FLAGS flags)
    {
        assert (GetHwnd() == 0);
        m_tc.dwFlags = flags;
    }

    // Starts and stops the marquee display of the progress bar, and sets the speed of the marquee.
    // Refer to TDM_SET_PROGRESS_BAR_MARQUEE in the Windows API documentation for more information.
    inline void CTaskDialog::SetProgressBarMarquee(BOOL isEnabled /* = TRUE*/, int marqueeSpeed /* = 0*/) const
    {
        assert(GetHwnd());
        SendMessage(TDM_SET_PROGRESS_BAR_MARQUEE, (WPARAM)isEnabled, (LPARAM)marqueeSpeed);
    }

    // Sets the current position for a progress bar.
    // Refer to TDM_SET_PROGRESS_BAR_POS in the Windows API documentation for more information.
    inline void CTaskDialog::SetProgressBarPosition(int progressPos) const
    {
        assert(GetHwnd());
        SendMessage(TDM_SET_PROGRESS_BAR_POS, (WPARAM)progressPos, 0);
    }

    // Sets the minimum and maximum values for the hosted progress bar.
    // Refer to TDM_SET_PROGRESS_BAR_RANGE in the Windows API documentation for more information.
    inline void CTaskDialog::SetProgressBarRange(int minRange, int maxRange) const
    {
        assert(GetHwnd());
        SendMessage(TDM_SET_PROGRESS_BAR_RANGE, 0, MAKELPARAM(minRange, maxRange));
    }

    // Sets the current state of the progress bar. Possible states are:
    //  PBST_NORMAL, PBST_PAUSE, PBST_ERROR.
    // Refer to TDM_SET_PROGRESS_BAR_STATE in the Windows API documentation for more information.
    inline void CTaskDialog::SetProgressBarState(int newState /* = PBST_NORMAL*/) const
    {
        assert(GetHwnd());
        SendMessage(TDM_SET_PROGRESS_BAR_STATE, (WPARAM)newState, 0);
    }

    // Simulates a click on the verification checkbox of the Task Dialog, if it exists.
    // Refer to TDM_CLICK_VERIFICATION in the Windows API documentation for more information.
    inline void CTaskDialog::SetVerificationCheckbox(BOOL isChecked) const
    {
        assert(GetHwnd());
        SendMessage(TDM_CLICK_VERIFICATION, (WPARAM)isChecked, (LPARAM)isChecked);
    }

    // Sets the text for the verification check box.
    inline void CTaskDialog::SetVerificationCheckboxText(LPCWSTR verificationText)
    {
        assert (GetHwnd() == 0);
        m_verificationText = FillString(verificationText);
        m_tc.pszVerificationText = m_verificationText;
    }

    // Sets the Task Dialog's window title.
    inline void CTaskDialog::SetWindowTitle(LPCWSTR windowTitle)
    {
        assert (GetHwnd() == 0);
        m_windowTitle = FillString(windowTitle);
        m_tc.pszWindowTitle = m_windowTitle;
    }

    // TaskDialogs direct their messages here.
    inline HRESULT CALLBACK CTaskDialog::StaticTaskDialogProc(HWND wnd, UINT notification, WPARAM wparam, LPARAM lparam, LONG_PTR)
    {
        CTaskDialog* t = static_cast<CTaskDialog*>(GetCWndPtr(wnd));
        if (t == 0)
        {
            // The CTaskDialog pointer wasn't found in the map, so add it now

            // Retrieve the pointer to the TLS Data
            TLSData* pTLSData = GetApp()->GetTlsData();
            assert(pTLSData);
            if (pTLSData)
            {
                // Retrieve pointer to CTaskDialog object from Thread Local Storage TLS
                t = static_cast<CTaskDialog*>(pTLSData->pWnd);
                assert(t);
                pTLSData->pWnd = NULL;

                if (t)
                {
                    // Store the CTaskDialog pointer in the HWND map
                    t->m_wnd = wnd;
                    t->AddToMap();
                }
            }
        }

        if (t == 0)
        {
            // Got a message for a window that's not in the map.
            // We should never get here.
            TRACE("*** Warning in CTaskDialog::StaticTaskDialogProc: HWND not in window map ***\n");
            return 0;
        }

        return static_cast<HRESULT>(t->TaskDialogProc(notification, wparam, lparam));

    } // LRESULT CALLBACK StaticTaskDialogProc(...)

    // Provides default handling of Task Dialog's messages.
    inline LRESULT CTaskDialog::TaskDialogProcDefault(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        switch(msg)
        {
        case TDN_BUTTON_CLICKED:
            return OnTDButtonClicked((int)wparam);

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
            OnTDExpandButtonClicked((BOOL)wparam);
            break;
        case TDN_HELP:
            OnTDHelp();
            break;
        case TDN_HYPERLINK_CLICKED:
            OnTDHyperlinkClicked(reinterpret_cast<LPCWSTR>(lparam));
            break;
        case TDN_NAVIGATED:
            OnTDNavigatePage();
            break;
        case TDN_RADIO_BUTTON_CLICKED:
            OnTDRadioButtonClicked((int)wparam);
            break;
        case TDN_TIMER:
            return OnTDTimer((DWORD)wparam);

        case TDN_VERIFICATION_CLICKED:
            OnTDVerificationCheckboxClicked((BOOL)wparam);
            break;
        }

        return S_OK;
    }

    // Override this function modify how the task dialog's messages are handled.
    inline LRESULT CTaskDialog::TaskDialogProc(UINT msg, WPARAM wparam, LPARAM lparam)
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

        // Always pass unhandled messages on to TaskDialogProcDefault
        return TaskDialogProcDefault(msg, wparam, lparam);
    }

    // Updates a text element on the TaskDialog after it is created. The size of the TaskDialog
    // is not adjusted to accommodate the new text.
    // Possible element values are:
    // TDE_CONTENT, TDE_EXPANDED_INFORMATION, TDE_FOOTER, TDE_MAIN_INSTRUCTION.
    // Refer to TDM_UPDATE_ELEMENT_TEXT in the Windows API documentation for more information.
    inline void CTaskDialog::UpdateElementText(TASKDIALOG_ELEMENTS element, LPCWSTR newText) const
    {
        assert(GetHwnd());
        SendMessage(TDM_UPDATE_ELEMENT_TEXT, (WPARAM)element, (LPARAM)newText);
    }

}


#endif // _WIN32XX_TASKDIALOG_H_
