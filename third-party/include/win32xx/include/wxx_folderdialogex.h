// Win32++   Version 10.2.0
// Release Date: 20th September 2025
//
//      David Nash
//      email: dnash@bigpond.net.au
//      url: https://sourceforge.net/projects/win32-framework
//           https://github.com/DavidNash2024/Win32xx
//
//
// Copyright (c) 2005-2025  David Nash
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


////////////////////////////////////////////////////////////////////////
// CFolderDialogEx class.
//  This class provides a modal dialog that allows users to select a
//  folder. It is based on the IFileDialog interface.
//
//  CFolderDialogEx is a modern replacement for the CFolderDialog class.
//  It supports dark mode applications, whereas CFolderDialog does not.
//
//  Note: CFolderDialogEx requires Windows Vista or later.


/////////////////////////////////////////////////////////////
// Coding example.
//
//  CFolderDialogEx chooseFolder;
//  chooseFolder.SetInitialFolder(L"C:\\Temp");
//  chooseFolder.SetTitle(L"This is a title!");
//  if (chooseFolder.DoModal() == IDOK)
//  {
//      TaskDialog(nullptr, nullptr, L"Choosen Folder:",
//          chooseFolder.GetFolderName(), 0, TDCBF_OK_BUTTON,
//          TD_INFORMATION_ICON, nullptr);
//  }


#ifndef _WIN32XX_FOLDERDIALOGEX_H_
#define _WIN32XX_FOLDERDIALOGEX_H_

#include "wxx_dialog.h"


namespace Win32xx
{
    ////////////////////////////////////////////////////////////
    // CFolderDialogEx uses the IFileDialog interface to display
    // a dialog that allows the user to select a folder.
    class CFolderDialogEx : public CDialog
    {
    public:
        CFolderDialogEx() = default;                  // Constructor
        virtual ~CFolderDialogEx() = default;         // Destructor

        virtual INT_PTR DoModal(HWND hParent = nullptr);

        const CString& GetFolderName() const;
        void SetInitialFolder(const CStringW& initialFolder);
        void SetTitle(const CStringW& title);

    private:
        CFolderDialogEx(const CFolderDialogEx&) = delete;
        CFolderDialogEx& operator=(const CFolderDialogEx&) = delete;

        CString m_folderName;
        CString m_initialFolderName;
        CString m_title;
    };
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

namespace Win32xx
{

    /////////////////////////////////////////////
    // Definitions for the CFolderDialogEx class.
    //

    // Displays a modal dialog that allows the user to select a folder.
    // Returns IDOK when a folder is selected, and returns IDCANCEL otherwise.
    // If a folder is selected, it's name can be retrieved by GetFolderName().
    inline INT_PTR CFolderDialogEx::DoModal(HWND hParent)
    {
        INT_PTR result = IDCANCEL;
        m_folderName.Empty();

        // Create the IFileDialog interface.
        IFileDialog* pFileDialog;
        if (SUCCEEDED(::CoCreateInstance(CLSID_FileOpenDialog, nullptr,
            CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pFileDialog))))
        {
            // Set the option to display only folders.
            DWORD options;
            if (SUCCEEDED(pFileDialog->GetOptions(&options)))
            {
                pFileDialog->SetOptions(options | FOS_PICKFOLDERS);

                // Set the initial folder if specified.
                if (!m_initialFolderName.IsEmpty())
                {
                    using PSHCREATEITEMFROMPARSINGNAME = HRESULT(WINAPI*)(PCWSTR, IBindCtx*, REFIID, void**);
                    HMODULE shell32 = ::GetModuleHandle(_T("Shell32.dll"));
                    if (shell32 != nullptr)
                    {
                        PSHCREATEITEMFROMPARSINGNAME pSHCreateItemFromParsingName = reinterpret_cast<PSHCREATEITEMFROMPARSINGNAME>(
                            reinterpret_cast<void*>(::GetProcAddress(shell32, "SHCreateItemFromParsingName")));

                        if (pSHCreateItemFromParsingName != nullptr)
                        {
                            IShellItem* pFolder = nullptr;
                            if (SUCCEEDED(pSHCreateItemFromParsingName(m_initialFolderName,
                                nullptr, IID_PPV_ARGS(&pFolder))))
                            {
                                pFileDialog->SetFolder(pFolder);
                                pFolder->Release();
                            }
                        }
                    }
                }

                // Set the dialog's title if specified.
                if (!m_title.IsEmpty())
                    pFileDialog->SetTitle(m_title);

                // Display the dialog.
                if (SUCCEEDED(pFileDialog->Show(hParent)))
                {
                    IShellItem* pShellItem;
                    if (SUCCEEDED(pFileDialog->GetResult(&pShellItem)))
                    {
                        PWSTR pFilePath = 0;
                        if (SUCCEEDED(pShellItem->GetDisplayName(SIGDN_DESKTOPABSOLUTEPARSING, &pFilePath)))
                        {
                            m_folderName = pFilePath;
                            CoTaskMemFree(pFilePath);
                            result = IDOK;
                        }
                    }

                    pShellItem->Release();
                }
            }

            pFileDialog->Release();
        }
        else
            throw CWinException(GetApp()->MsgWndDialog());

        return result;
    }

    // Retrieves a const reference to a CString containing the name of the
    // folder selected by the user when the modal dialog is displayed.
    inline const CString& CFolderDialogEx::GetFolderName() const
    {
        return m_folderName;
    }

    // Sets the initial folder displayed by the modal dialog. This can be
    // assigned from either a LPCWSTR, or a CStringW, or a CString when
    // compiled with the Unicode character set.
    inline void CFolderDialogEx::SetInitialFolder(const CStringW& initialFolder)
    {
        m_initialFolderName = initialFolder;
    }

    // Sets the caption of the modal dialog.  This can be assigned from either
    // a LPCWSTR, or a CStringW, or a CString when compiled with the Unicode
    // character set.
    inline void CFolderDialogEx::SetTitle(const CStringW& title)
    {
        m_title = title;
    }

} // namespace Win32xx


#endif // _WIN32XX_FOLDERDIALOGEX_H_
