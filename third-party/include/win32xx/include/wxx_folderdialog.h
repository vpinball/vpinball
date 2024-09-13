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
// CFolderDialog class.
//  This class wraps SHBrowseForFolder to display a dialog for
//  choosing a folder.
//
//  The memory for the PIDL(s) created by this class is
//  freed when the object goes out of scope.


////////////////////////////////////////////////////////
//
//  Coding example for Win2000 and below.
//
//      CFolderDialog fd;
//
//      // Set the root folder to list the computer's drives (or C:).
//      ITEMIDLIST* pidlRoot = nullptr;
//      SHGetSpecialFolderLocation(nullptr, CSIDL_DRIVES, &pidlRoot);
//      fd.SetRoot(pidlRoot);
//
//      // Set the title for the dialog.
//      fd.SetTitle(_T("Choose a folder"));
//
//      // Display the dialog
//      if (fd.DoModal() == IDOK)
//      {
//          // Do something with the folder found
//          MessageBox(fd.GetFolderPath(), _T("Folder Chosen"), MB_OK);
//      }
//
//      // Release the memory allocated for our pidlRoot.
//      CoTaskMemFree(pidlRoot);
//
////////////////////////////////////////////////////////

////////////////////////////////////////////////////////
//
//  Coding example for WinXP and above.
//
//      CFolderDialog fd;
//
//      // Set the root folder to list the computer's drives (or C:).
//      PIDLIST_ABSOLUTE pidlRoot = ILCreateFromPath(_T("C:"));
//      fd.SetRoot(pidlRoot);
//
//      // Set the title for the dialog.
//      fd.SetTitle(_T("Choose a folder"));
//
//      // Display the dialog
//      if (fd.DoModal() == IDOK)
//      {
//          // Do something with the folder found
//          MessageBox(fd.GetFolderPath(), _T("Folder Chosen"), MB_OK);
//      }
//
//      // Release the memory allocated for our pidlRoot.
//      ILFree(pidlRoot);
//
////////////////////////////////////////////////////////


#ifndef _WIN32XX_FOLDERDIALOG_H_
#define _WIN32XX_FOLDERDIALOG_H_

#include "wxx_dialog.h"


namespace Win32xx
{
    ////////////////////////////////////////////////////////
    // CFolderDialog manages a dialog box that allows users
    // to select a folder.
    class CFolderDialog : public CDialog
    {
    public:
        CFolderDialog();
        virtual ~CFolderDialog() override;

        virtual INT_PTR DoModal(HWND parent = nullptr) override;

        CString GetDisplayName() const       { return m_displayName; }
        CString GetFolderPath() const;
        LPITEMIDLIST GetFolderPidl() const   { return m_fullPidl; }
        int  GetImageIndex() const           { return m_imageIndex; }
        UINT GetFlags() const                { return m_flags; }
        void EnableOK(BOOL enable = TRUE) const;
        void SetExpanded(LPCWSTR path) const;
        void SetExpanded(LPITEMIDLIST pItemIDList) const;
        void SetFlags(UINT flags) { m_flags = flags; }
        void SetOKText(LPCWSTR text) const;
        void SetRoot(LPITEMIDLIST pItemIDList);
        void SetSelection(LPITEMIDLIST pItemIDList) const;
        void SetSelection(LPCTSTR path) const;
        void SetStatusText(LPCTSTR text) const;
        void SetTitle(LPCTSTR title);

    protected:
        virtual void OnCancel() override;
        virtual void OnInitialized();
        virtual void OnIUnknown(LPARAM lparam);
        virtual void OnOK() override;
        virtual void OnSelChanged();
        virtual int  OnValidateFailed(LPARAM lparam);

    private:
        CFolderDialog(const CFolderDialog&) = delete;
        CFolderDialog& operator=(const CFolderDialog&) = delete;

        static int CALLBACK BrowseCallbackProc(HWND wnd, UINT msg, LPARAM param1, LPARAM lparam2);

        CString m_displayName;
        CString m_title;
        BROWSEINFO m_bi;
        LPITEMIDLIST m_pidlRoot;
        LPITEMIDLIST m_fullPidl;
        int m_imageIndex;
        UINT m_flags;
    };

}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~



namespace Win32xx
{

    inline CFolderDialog::CFolderDialog() : m_pidlRoot(nullptr), m_fullPidl(nullptr), m_imageIndex(0)
    {
        m_bi = {};
        m_bi.lpfn = BrowseCallbackProc;
        m_bi.lParam = reinterpret_cast<LPARAM>(this);

        // Set the default flags.
        //  BIF_NEWDIALOGSTYLE    - Only return file system directories.
        //  BIF_NEWDIALOGSTYLE    - Provides a resizable dialog without an edit box.
        //  BIF_NONEWFOLDERBUTTON - Do not include the New Folder button in the browse dialog box.
        m_flags = BIF_RETURNONLYFSDIRS |BIF_NEWDIALOGSTYLE | BIF_NONEWFOLDERBUTTON;
    }

    inline CFolderDialog::~CFolderDialog()
    {
        // Free the memory allocated to our pidls.
        CoTaskMemFree(m_fullPidl);
    }

    // The callback function used to send messages to and process messages
    // from a Browse dialog box displayed in response to a call to SHBrowseForFolder.
    inline int CALLBACK CFolderDialog::BrowseCallbackProc(HWND wnd, UINT msg, LPARAM param1, LPARAM param2)
    {
        CFolderDialog* pThis = reinterpret_cast<CFolderDialog*>(param2);
        int result = 0;

        if (pThis->GetHwnd() == nullptr)
        {
            pThis->m_wnd = wnd;
            pThis->AddToMap();
        }

        switch (msg)
        {
        case BFFM_INITIALIZED:
            pThis->OnInitialized();
            break;
        case BFFM_SELCHANGED:
            pThis->OnSelChanged();
            break;
        case BFFM_VALIDATEFAILED:
            result = pThis->OnValidateFailed(param1);
            break;
        case BFFM_IUNKNOWN:
            pThis->OnIUnknown(param1);
            break;
        }

        return result;
    }

    // Displays the folder browser dialog.
    inline INT_PTR CFolderDialog::DoModal(HWND parent)
    {
        if (m_fullPidl != nullptr)
            CoTaskMemFree(m_fullPidl);
        m_fullPidl = nullptr;
        m_bi.lpszTitle = m_title.c_str();
        m_bi.pszDisplayName = m_displayName.GetBuffer(MAX_PATH);
        m_bi.ulFlags = m_flags;
        m_bi.hwndOwner = parent;
        m_bi.pidlRoot = m_pidlRoot;

        LPITEMIDLIST pidl = ::SHBrowseForFolder(&m_bi);
        m_displayName.ReleaseBuffer();

        INT_PTR result = 0;
        if (pidl)
        {
            m_fullPidl = pidl;
            result = IDOK;
            OnOK();
        }
        else
        {
            result = IDCANCEL;
            OnCancel();
        }

        // Prepare the CWnd for reuse.
        Cleanup();

        return result;
    }

    // Enables or disables the OK button.
    inline void CFolderDialog::EnableOK(BOOL enable /*TRUE*/) const
    {
        SendMessage(BFFM_ENABLEOK, static_cast<WPARAM>(enable), 0);
    }

    // Returns the path of the selected folder.
    // Refer to SHGetPathFromIDList in the Windows API documentation for more information.
    inline CString CFolderDialog::GetFolderPath() const
    {
        CString str;
        SHGetPathFromIDList(m_fullPidl, str.GetBuffer(MAX_PATH));
        str.ReleaseBuffer();

        return str;
    }

    // Called when the cancel button is pressed.
    inline void CFolderDialog::OnCancel()
    {
    }

    // Called when the Folder dialog is displayed.
    // Override this function to perform tasks when the dialog starts.
    inline void CFolderDialog::OnInitialized()
    {
    //  An example of things that can be done here.

    //  EnableOK(FALSE);
    //  SetOKText(L"OK Text");
    //  SetStatusText(_T("Here is some status text"));
    //  SetExpanded(L"C:\\Program Files");
    //  SetSelection(_T("C:\\Temp"));
    }

    // Called when an IUnknown interface is available to the dialog box.
    inline void CFolderDialog::OnIUnknown(LPARAM)
    {
    }

    // Called when the OK button is pressed.
    inline void CFolderDialog::OnOK()
    {
    }

    // Called when the selection has changed in the dialog box.
    inline void CFolderDialog::OnSelChanged()
    {
    }

    // Called when the user typed an invalid name into the dialog's edit box.
    inline int CFolderDialog::OnValidateFailed(LPARAM)
    {
        // returns zero to dismiss the dialog or nonzero to keep the dialog displayed.
        return 1;
    }

    // Specifies the path of a folder to expand in the Browse dialog box.
    // Refer to BFFM_SETEXPANDED in the Windows API documentation for more information.
    inline void CFolderDialog::SetExpanded(LPCWSTR path) const
    {
        WPARAM wparam = static_cast<WPARAM>(TRUE);
        LPARAM lparam = reinterpret_cast<LPARAM>(path);
        SendMessage(BFFM_SETEXPANDED, wparam, lparam);
    }

    // Specifies the path of a folder to expand in the Browse dialog box.
    // Refer to BFFM_SETEXPANDED in the Windows API documentation for more information.
    inline void CFolderDialog::SetExpanded(LPITEMIDLIST pItemIDList) const
    {
        WPARAM wparam = static_cast<WPARAM>(FALSE);
        LPARAM lparam = reinterpret_cast<LPARAM>(pItemIDList);
        SendMessage(BFFM_SETEXPANDED, wparam, lparam);
    }

    // Sets the text of the OK button.
    // Refer to BFFM_SETOKTEXT in the Windows API documentation for more information.
    inline void CFolderDialog::SetOKText(LPCWSTR text) const
    {
        SendMessage(BFFM_SETOKTEXT, 0, reinterpret_cast<LPARAM>(text));
    }

    // Sets the location of the root folder from which to start browsing.
    inline void CFolderDialog::SetRoot(LPITEMIDLIST pItemIDList)
    {
        m_pidlRoot = pItemIDList;
    }

    // Specifies the path of a folder to select.
    // Refer to BFFM_SETSELECTION in the Windows API documentation for more information.
    inline void CFolderDialog::SetSelection(LPITEMIDLIST pItemIDList) const
    {
        SendMessage(BFFM_SETSELECTION, FALSE, reinterpret_cast<LPARAM>(pItemIDList));
    }

    // Specifies the path of a folder to select.
    // Refer to BFFM_SETSELECTION in the Windows API documentation for more information.
    inline void CFolderDialog::SetSelection(LPCTSTR path) const
    {
        SendMessage(BFFM_SETSELECTION, TRUE, reinterpret_cast<LPARAM>(path));
    }

    // Sets the status text.
    // This is incompatible with the BIF_USENEWUI or BIF_NEWDIALOGSTYLE flags.
    // Refer to BFFM_SETSTATUSTEXT in the Windows API documentation for more information.
    inline void CFolderDialog::SetStatusText(LPCTSTR text) const
    {
        SendMessage(BFFM_SETSTATUSTEXT, 0, reinterpret_cast<LPARAM>(text));
    }

    // Sets the title of the browse for folder dialog.
    inline void CFolderDialog::SetTitle(LPCTSTR title)
    {
        if (title)
            m_title = title;
        else
            m_title.Empty();

        m_bi.lpszTitle = m_title.c_str();
    }

}
#endif // _WIN32XX_FOLDERDIALOG_H_

