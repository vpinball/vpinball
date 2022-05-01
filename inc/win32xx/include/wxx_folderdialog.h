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
//      ITEMIDLIST* pidlRoot = 0;
//      SHGetSpecialFolderLocation(0, CSIDL_DRIVES, &pidlRoot);
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

#ifdef _MSC_VER
#pragma warning ( push )
#pragma warning (disable : 4091)  // temporarily disable warning: 'typedef': ignored
#endif // _MSC_VER

#include <shlobj.h>

#ifdef _MSC_VER
#pragma warning ( pop )
#endif // _MSC_VER


// Support older compilers
#ifndef BIF_NONEWFOLDERBUTTON

  #define BIF_NONEWFOLDERBUTTON 0x0200

  #ifndef BIF_NEWDIALOGSTYLE
    #define BIF_NEWDIALOGSTYLE    0x0040
  #endif

  // messages from browser
  #define BFFM_INITIALIZED        1
  #define BFFM_SELCHANGED         2
  #define BFFM_VALIDATEFAILEDA    3   // lParam:szPath ret:1(cont),0(EndDialog)
  #define BFFM_VALIDATEFAILEDW    4   // lParam:wzPath ret:1(cont),0(EndDialog)
  #define BFFM_IUNKNOWN           5   // provides IUnknown to client. lParam: IUnknown*

  // messages to browser
  #define BFFM_SETSTATUSTEXTA     (WM_USER + 100)
  #define BFFM_ENABLEOK           (WM_USER + 101)
  #define BFFM_SETSELECTIONA      (WM_USER + 102)
  #define BFFM_SETSELECTIONW      (WM_USER + 103)
  #define BFFM_SETSTATUSTEXTW     (WM_USER + 104)
  #define BFFM_SETOKTEXT          (WM_USER + 105) // Unicode only
  #define BFFM_SETEXPANDED        (WM_USER + 106) // Unicode only

#endif

namespace Win32xx
{
    ////////////////////////////////////////////////////////
    // CFolderDialog manages a dialog box which allows users
    // to select a folder.
    class CFolderDialog : public CDialog
    {
    public:
        CFolderDialog(UINT id = 0);
        virtual ~CFolderDialog();

        virtual INT_PTR DoModal(HWND parent = 0);

        CString GetDisplayName() const       { return m_displayName; }
        CString GetFolderPath() const;
        LPITEMIDLIST GetFolderPidl() const   { return m_fullPidl.back(); }
        int  GetImageIndex() const           { return m_imageIndex; }
        int  GetFlags() const                { return m_flags; }
        void EnableOK(BOOL enable = TRUE);
        void SetExpanded(LPCWSTR path);
        void SetExpanded(LPITEMIDLIST pItemIDList);
        void SetFlags(UINT flags) { m_flags = flags; }
        void SetOKText(LPCWSTR text);
        void SetRoot(LPITEMIDLIST pItemIDList);
        void SetSelection(LPITEMIDLIST pItemIDList);
        void SetSelection(LPCTSTR path);
        void SetStatusText(LPCTSTR text);
        void SetTitle(LPCTSTR title);

    protected:
        virtual void OnCancel();
        virtual void OnInitialized();
        virtual void OnIUnknown(LPARAM lparam);
        virtual void OnOK();
        virtual void OnSelChanged();
        virtual int  OnValidateFailed(LPARAM lparam);

    private:
        CFolderDialog(const CFolderDialog&);              // Disable copy construction.
        CFolderDialog& operator = (const CFolderDialog&); // Disable assignment operator.

        static int CALLBACK BrowseCallbackProc(HWND wnd, UINT msg, LPARAM param1, LPARAM lparam2);

        CString m_displayName;
        CString m_title;
        BROWSEINFO m_bi;
        LPITEMIDLIST m_pidlRoot;
        int m_imageIndex;
        UINT m_flags;
        std::vector<LPITEMIDLIST> m_fullPidl;
    };

}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~



namespace Win32xx
{

    inline CFolderDialog::CFolderDialog(UINT id) : CDialog(id), m_pidlRoot(0), m_imageIndex(0)
    {
        ZeroMemory(&m_bi, sizeof(m_bi));
        m_bi.lpfn = BrowseCallbackProc;
        m_bi.lParam = (LPARAM)this;

        // Set the default flags.
        //  BIF_NEWDIALOGSTYLE    - Only return file system directories.
        //  BIF_NEWDIALOGSTYLE    - Provides a resizable dialog without an edit box.
        //  BIF_NONEWFOLDERBUTTON - Do not include the New Folder button in the browse dialog box.
        m_flags = BIF_RETURNONLYFSDIRS |BIF_NEWDIALOGSTYLE | BIF_NONEWFOLDERBUTTON;
    }

    inline CFolderDialog::~CFolderDialog()
    {
        // Free the memory allocated to our pidls.
        std::vector<LPITEMIDLIST>::iterator it;
        for (it = m_fullPidl.begin(); it != m_fullPidl.end(); ++it)
            CoTaskMemFree(*it);
    }

    // The callback function used used to send messages to and process messages
    // from a Browse dialog box displayed in response to a call to SHBrowseForFolder.
    inline int CALLBACK CFolderDialog::BrowseCallbackProc(HWND wnd, UINT msg, LPARAM param1, LPARAM param2)
    {
        CFolderDialog* pThis = reinterpret_cast<CFolderDialog*>(param2);
        int result = 0;

        if (pThis->GetHwnd() == 0)
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
            m_fullPidl.push_back(pidl);
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
    inline void CFolderDialog::EnableOK(BOOL enable /*TRUE*/)
    {
        SendMessage(BFFM_ENABLEOK, (WPARAM)enable, 0);
    }

    // Returns the path of the selected folder.
    // Refer to SHGetPathFromIDList in the Windows API documentation for more information.
    inline CString CFolderDialog::GetFolderPath() const
    {
        CString str;
        SHGetPathFromIDList(m_fullPidl.back(), str.GetBuffer(MAX_PATH));
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
    inline void CFolderDialog::SetExpanded(LPCWSTR path)
    {
        SendMessage(BFFM_SETEXPANDED, (WPARAM)TRUE, (LPARAM)path);
    }

    // Specifies the path of a folder to expand in the Browse dialog box.
    // Refer to BFFM_SETEXPANDED in the Windows API documentation for more information.
    inline void CFolderDialog::SetExpanded(LPITEMIDLIST pItemIDList)
    {
        SendMessage(BFFM_SETEXPANDED, (WPARAM)FALSE, (LPARAM)pItemIDList);
    }

    // Sets the text of the OK button.
    // Refer to BFFM_SETOKTEXT in the Windows API documentation for more information.
    inline void CFolderDialog::SetOKText(LPCWSTR text)
    {
        SendMessage(BFFM_SETOKTEXT, 0, (LPARAM)text);
    }

    // Sets the location of the root folder from which to start browsing.
    inline void CFolderDialog::SetRoot(LPITEMIDLIST pItemIDList)
    {
        m_pidlRoot = pItemIDList;
    }

    // Specifies the path of a folder to select.
    // Refer to BFFM_SETSELECTION in the Windows API documentation for more information.
    inline void CFolderDialog::SetSelection(LPITEMIDLIST pItemIDList)
    {
        SendMessage(BFFM_SETSELECTION, FALSE, (LPARAM)pItemIDList);
    }

    // Specifies the path of a folder to select.
    // Refer to BFFM_SETSELECTION in the Windows API documentation for more information.
    inline void CFolderDialog::SetSelection(LPCTSTR path)
    {
        SendMessage(BFFM_SETSELECTION, TRUE, (LPARAM)path);
    }

    // Sets the status text.
    // This is incompatible with the BIF_USENEWUI or BIF_NEWDIALOGSTYLE flags.
    // Refer to BFFM_SETSTATUSTEXT in the Windows API documentation for more information.
    inline void CFolderDialog::SetStatusText(LPCTSTR text)
    {
        SendMessage(BFFM_SETSTATUSTEXT, 0, (LPARAM)text);
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

