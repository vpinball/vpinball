// Win32++   Version 8.8
// Release Date: 15th October 2020
//
//      David Nash
//      email: dnash@bigpond.net.au
//      url: https://sourceforge.net/projects/win32-framework
//
//
// Copyright (c) 2005-2020  David Nash
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

#ifndef _WIN32XX_COMMONDLG_H_
#define _WIN32XX_COMMONDLG_H_

#include "wxx_dialog.h"
#include "wxx_richedit.h"



///////////////////////////////////////////////////////////////////
// Definitions of the CCommonDialog, CColorDialog, CFileDialog,
// CFindReplaceDialog, and CFontDialog classes.
//
// CCommonDialog is the base class for all common dialogs.
// CColorDialog supports the color common dialog.
// CFileDialog supports the FileOpen and FileSave common dialogs.
// CFindReplaceDialog supports the Find and Replace common dialogs.
// CFontDialog supports the Font common dialog.
//
// Note: A CWinException is thrown if the dialog can't be displayed.
// Use CWinException's GetMessageID to retrieve CommDlgExtendedError.



namespace Win32xx
{

#ifndef _WIN32_WCE
    // registered message used by common dialogs
    const UINT UWM_HELPMSGSTRING = ::RegisterWindowMessage(HELPMSGSTRING);      // Used by common dialogs. Sent when the user clicks the Help button.
    const UINT UWM_FILEOKSTRING  = ::RegisterWindowMessage(FILEOKSTRING);       // Used by common dialogs. Sent when the user specifies a file name and clicks the OK button.
    const UINT UWM_LBSELCHSTRING = ::RegisterWindowMessage(LBSELCHSTRING);      // Used by the File common dialog. Sent when the selection changes in any of the list boxes or combo boxes.
    const UINT UWM_SHAREVISTRING = ::RegisterWindowMessage(SHAREVISTRING);      // Used by the File common dialog. Sent if a sharing violation occurs for the selected file when the user clicks the OK button.
    const UINT UWM_FINDMSGSTRING = ::RegisterWindowMessage(FINDMSGSTRING);      // Used by the Find/Replace common dialog. Sent when the user clicks the Find Next, Replace, or Replace All button, or closes the dialog box.
#endif


    //////////////////////////////////////////////////////////
    // CCommonDialog is the base class for all common dialogs.
    class CCommonDialog : public CDialog
    {
    public:
        CCommonDialog(UINT resID = 0) : CDialog(resID) {}
        virtual ~CCommonDialog(){}

    protected:
        virtual void    OnCancel()  {}  // a required to override
        virtual void    OnHelpButton();
        virtual BOOL    OnInitDialog();
        virtual void    OnOK()  {}      // a required to override

        // static callback
        static INT_PTR CALLBACK CDHookProc(HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam);

    private:
        CCommonDialog(const CCommonDialog&);              // Disable copy construction
        CCommonDialog& operator = (const CCommonDialog&); // Disable assignment operator
    };


    //////////////////////////////////////////////////////////////
    // CColorDialog manages Color dialog box that enables the user
    // to select a color.
    class CColorDialog : public CCommonDialog
    {
    public:
        CColorDialog(COLORREF initColor = 0, DWORD flags = 0);
        virtual ~CColorDialog(){}

        virtual INT_PTR DoModal(HWND owner = 0);
        COLORREF  GetColor() const              { return m_ofn.rgbResult;}
        COLORREF* GetCustomColors()             { return m_customColors;}
        const CHOOSECOLOR& GetParameters() const { return m_ofn; }
        void    SetColor(COLORREF clr)          { m_ofn.rgbResult = clr;}
        void    SetCustomColors(const COLORREF* pColors = NULL);
        void    SetParameters(const CHOOSECOLOR& cc);

    protected:
        virtual INT_PTR DialogProc(UINT, WPARAM, LPARAM);

        // Not intended to be overridden
        INT_PTR DialogProcDefault(UINT msg, WPARAM wparam, LPARAM lparam);

    private:
        CColorDialog(const CColorDialog&);              // Disable copy construction
        CColorDialog& operator = (const CColorDialog&); // Disable assignment operator

        CHOOSECOLOR     m_ofn;                   // ChooseColor parameters
        COLORREF        m_customColors[16];      // Custom colors array
    };


    ///////////////////////////////////////////////////////
    // CFileDialog manages the file open and save-as common
    // dialog boxes.
    class CFileDialog : public CCommonDialog
    {
    public:

        // Constructor/destructor
        CFileDialog (BOOL isOpenFileDialog = TRUE,
                LPCTSTR pDefExt = NULL,
                LPCTSTR pInitFileDir = NULL,
                LPCTSTR pFileName = NULL,
                DWORD   flags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
                LPCTSTR pFilter   = NULL );

        virtual ~CFileDialog()  {}

        // Operations
        virtual INT_PTR DoModal(HWND owner = 0);

        // methods valid after successful DoModal()
        CString GetFileName() const;
        CString GetFileExt() const;
        CString GetFileTitle() const;
        CString GetFolderPath() const;
        CString GetPathName() const;
        const OPENFILENAME& GetParameters() const { return m_ofn; }

        // methods for setting parameters before DoModal()
        BOOL    IsOpenFileDialog()  const           { return m_isOpenFileDialog; }
        void    SetDefExt(LPCTSTR pExt);
        void    SetFileName(LPCTSTR pFileName);
        void    SetFilter(LPCTSTR pFilter);
        void    SetParameters(const OPENFILENAME& ofn);
        void    SetTitle(LPCTSTR pTitle);

        // Enumerating multiple file selections
        CString GetNextPathName(int& pos) const;

    protected:
        virtual INT_PTR DialogProc(UINT, WPARAM, LPARAM);
        virtual void    OnFileNameChange();
        virtual LRESULT OnFileNameOK();
        virtual void    OnFolderChange();
        virtual void    OnInitDone();
        virtual void    OnLBSelChangedNotify(UINT boxID, UINT curSel, UINT code);
        virtual LRESULT OnNotify(WPARAM, LPARAM);
        virtual LRESULT OnShareViolation(LPCTSTR pPathName);
        virtual void    OnTypeChange();

        // Not intended to be overridden
        INT_PTR DialogProcDefault(UINT msg, WPARAM wparam, LPARAM lparam);

    private:
        CFileDialog(const CFileDialog&);              // Disable copy construction
        CFileDialog& operator = (const CFileDialog&); // Disable assignment operator

        BOOL            m_isOpenFileDialog;  // TRUE = open, FALSE = save
        CString         m_filter;          // File filter string
        CString         m_fileName;        // File name string
        CString         m_title;           // Dialog title
        CString         m_defExt;          // Default extension string
        OPENFILENAME    m_ofn;              // OpenFileName parameters
    };



    // Find/FindReplace modeless dialog class
    class CFindReplaceDialog : public CCommonDialog
    {
    public:
        // constructor and destructor
        CFindReplaceDialog(BOOL isFindDialogOnly = TRUE);
        virtual ~CFindReplaceDialog() {}

        virtual HWND Create(HWND parent = 0);
        virtual BOOL Create(BOOL isFindDialogOnly,
                        LPCTSTR pFindWhat,
                        LPCTSTR pReplaceWith = NULL,
                        DWORD   flags = FR_DOWN,
                        HWND    parent = 0);

        virtual BOOL IsModal() const                    { return FALSE; }

        // Operations:
        BOOL    FindNext() const;           // TRUE = find next
        CString GetFindString() const;      // get find string
        CString GetReplaceString() const;   // get replacement string
        const   FINDREPLACE& GetParameters() const  { return m_fr; }
        BOOL    IsFindDialogOnly() const            { return m_isFindDialogOnly; }
        BOOL    IsTerminating();            // TRUE = terminate dialog
        BOOL    MatchCase() const;          // TRUE = matching case
        BOOL    MatchWholeWord() const;     // TRUE = whole words only
        BOOL    ReplaceAll() const;         // TRUE = all occurrences
        BOOL    ReplaceCurrent() const;     // TRUE = current string
        BOOL    SearchDown() const;         // TRUE = down, FALSE = up
        void    SetParameters(const FINDREPLACE& fr);

        static CFindReplaceDialog* GetNotifier(LPARAM lparam);

    protected:
        virtual INT_PTR DialogProc(UINT, WPARAM, LPARAM);

        // Not intended to be overridden
        INT_PTR DialogProcDefault(UINT msg, WPARAM wparam, LPARAM lparam);

    private:
        CFindReplaceDialog(const CFindReplaceDialog&);              // Disable copy construction
        CFindReplaceDialog& operator = (const CFindReplaceDialog&); // Disable assignment operator

        FINDREPLACE     m_fr;               // FindReplace parameters
        BOOL            m_isFindDialogOnly; // TRUE for a find only dialog
        CString         m_findWhat;         // The Find string
        CString         m_replaceWith;      // The Replace string
    };


    ////////////////////////////////////////////////
    // CFontDialog manages a dialog box that allows
    // users to select a font.
    class CFontDialog : public CCommonDialog
    {
    public:
        CFontDialog(const LOGFONT& initial, DWORD flags = 0, HDC printer = 0);
        CFontDialog(const CHARFORMAT& charformat, DWORD flags = 0, HDC printer = 0);
        CFontDialog(DWORD flags = 0, HDC printer = 0);

        virtual ~CFontDialog()  {}

        virtual INT_PTR DoModal(HWND owner = 0);
        CHARFORMAT  GetCharFormat() const;
        COLORREF    GetColor() const            { return m_cf.rgbColors;}
        CString GetFaceName() const             { return m_logFont.lfFaceName;}
        LOGFONT GetLogFont() const              { return m_logFont;}
        const CHOOSEFONT& GetParameters() const { return m_cf; }
        int     GetSize() const;
        CString GetStyleName() const            { return m_styleName;}
        long    GetWeight() const               { return m_logFont.lfWeight;}
        BOOL    IsBold() const                  { return (m_logFont.lfWeight >= FW_SEMIBOLD);}
        BOOL    IsItalic() const                { return m_logFont.lfItalic;}
        BOOL    IsStrikeOut() const             { return m_logFont.lfStrikeOut;}
        BOOL    IsUnderline() const             { return m_logFont.lfUnderline;}
        void    SetColor(COLORREF color)        { m_cf.rgbColors = color;}
        void    SetParameters(const CHOOSEFONT& cf);

    protected:
        virtual INT_PTR DialogProc(UINT, WPARAM, LPARAM);
        virtual void    OnOK();

        // Not intended to be overridden
        INT_PTR DialogProcDefault(UINT msg, WPARAM wparam, LPARAM lparam);

    private:
        CFontDialog(const CFontDialog&);              // Disable copy construction
        CFontDialog& operator = (const CFontDialog&); // Disable assignment operator
        DWORD FillInLogFont(const CHARFORMAT& cf);

        // private data
        LOGFONT     m_logFont;          // Font characteristics
        CHOOSEFONT  m_cf;               // ChooseFont parameters
        CString     m_styleName;     // Style name on the dialog
    };

}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


namespace Win32xx
{

    //////////////////////////////////////////
    // Definitions for the CCommonDialog class
    //

    // Override this function to respond to the help button. The common dialog
    // requires the appropriate flag to be specified before a help button
    // is displayed.
    inline  void CCommonDialog::OnHelpButton()
    {
    }

    // Called when the dialog is initialized. Override it in your derived class
    // to automatically perform tasks. The return value is used by WM_INITDIALOG
    inline BOOL CCommonDialog::OnInitDialog()
    {
        return TRUE;
    }

    // The callback function for the common dialog's hook procedure. Messages
    // intercepted by the hook procedure are processed here, and forwarded
    // on to the virtual DialogProc function.
    inline INT_PTR CALLBACK CCommonDialog::CDHookProc(HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam)
    {
        // Find the CWnd pointer mapped to this HWND
        CCommonDialog* pCommonDlg = static_cast<CCommonDialog*>(GetCWndPtr(wnd));
        if (pCommonDlg == 0)
        {
            // The HWND wasn't in the map, so add it now
            TLSData* pTLSData = GetApp()->GetTlsData();
            assert(pTLSData);
            if (!pTLSData) return 0;

            // Retrieve pointer to CWnd object from Thread Local Storage TLS
            pCommonDlg = static_cast<CCommonDialog*>(pTLSData->pWnd);
            assert(pCommonDlg);
            pTLSData->pWnd = NULL;

            // Attach the HWND to the CommonDialog object
            pCommonDlg->Attach(wnd);
        }

        return pCommonDlg->DialogProc(msg, wparam, lparam);
    }


    /////////////////////////////////////////
    // Definitions for the CColorDialog class
    //

    // Construct a CColorDialog object. The initial color, and flags for the
    // CHOOSECOLOR struct can be specified.  Refer to the description of the
    // CHOOSECOLOR struct in the Windows API documentation.
    inline CColorDialog::CColorDialog(COLORREF initColor /* = 0 */, DWORD flags /* = 0 */)
    {
        // set the parameters in the CHOOSECOLOR struct
        ZeroMemory(&m_ofn,  sizeof(m_ofn));
        m_ofn.rgbResult = initColor;
        m_ofn.Flags = flags;

        // Set all custom colors to white
        for (int i = 0; i <= 15; ++i)
            m_customColors[i] = RGB(255,255,255);

        // Enable the hook proc for the help button
        if (m_ofn.Flags & CC_SHOWHELP)
            m_ofn.Flags |= CC_ENABLEHOOK;

        // Set the CHOOSECOLOR struct parameters to safe values
        SetParameters(m_ofn);
    }

    // Dialog procedure for the Color dialog. Override this function to
    // customise the message handling.
    inline INT_PTR CColorDialog::DialogProc(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        //  Message intercepted by the hook procedure are passed here.
        //  Note: OnCancel and OnOK are not called here. They are called by DoModal
        //  in response to value returned by called by ChooseColor.

        //  A typical function might look like this:

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

        // Always pass unhandled messages on to DialogProcDefault
        return DialogProcDefault(msg, wparam, lparam);
    }

    // The Default message handling for CColorDialog. Don't override this
    // function, override DialogProc instead.
    // Note: OnCancel and OnOK are called by DoModal.
    inline INT_PTR CColorDialog::DialogProcDefault(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        UNREFERENCED_PARAMETER(lparam);

        switch (msg)
        {
        case WM_INITDIALOG:     return OnInitDialog();
        case WM_COMMAND:        if (LOWORD(wparam) == pshHelp)  OnHelpButton();
        }

        // Return 0 to allow default processing of the message.
        return 0;
    }

    // Display the ChooseColor common dialog box and select the current color.
    // An exception is thrown if the dialog box isn't created.
    inline INT_PTR CColorDialog::DoModal(HWND owner /* = 0 */)
    {
        assert( GetApp() );    // Test if Win32++ has been started
        assert(!IsWindow());    // Only one window per CWnd instance allowed

        // Ensure this thread has the TLS index set
        TLSData* pTLSData = GetApp()->SetTlsData();

        // Create the modal dialog
        pTLSData->pWnd = this;

        m_ofn.hwndOwner = owner;

        // invoke the control and save the result on success
        BOOL isValid = ::ChooseColor(&m_ofn);

        m_wnd = 0;

        if (!isValid)
        {
            DWORD error = CommDlgExtendedError();
            if ((error != 0) && (error != CDERR_DIALOGFAILURE))
                // ignore the exception caused by closing the dialog
                throw CWinException(g_msgWndDoModal, error);

            OnCancel();
            return IDCANCEL;
        }

        OnOK();
        return IDOK;
    }

    // The pColors parameter is a pointer to an array of 16 COLORREF.
    // If the pCustomColors is NULL, all custom colors are set to white,
    // otherwise they are set to the colors specified in the pCustomColors array.
    inline void CColorDialog::SetCustomColors(const COLORREF* pColors /* = NULL */)
    {
        for (UINT i = 0; i < 16; i++)
        {
            COLORREF clr = pColors ? pColors[i] : RGB(255,255,255);
            m_customColors[i] = clr;
        }
    }

    // Sets the various parameters of the CHOOSECOLOR struct.
    // The parameters are set to sensible values.
    inline void CColorDialog::SetParameters(const CHOOSECOLOR& cc)
    {
        m_ofn.lStructSize    = sizeof(m_ofn);
        m_ofn.hwndOwner      = 0;            // Set this in DoModal
        m_ofn.hInstance      = cc.hInstance;
        m_ofn.rgbResult      = cc.rgbResult;
        m_ofn.lpCustColors   = m_customColors;
        m_ofn.Flags          = cc.Flags;
        m_ofn.lCustData      = cc.lCustData;
        m_ofn.lpfnHook       = reinterpret_cast<LPCCHOOKPROC>(CDHookProc);
        m_ofn.lpTemplateName = cc.lpTemplateName;
    }


    ////////////////////////////////////////
    // Definitions for the CFileDialog class
    //

    // Construct a CFileDialog object. isOpenFileDialog specifies the type of
    // dialog box, OpenFile or SaveFile. The file's default extent and name can
    // be specified, along with the flags for the OPENFILENAME struct.
    // The pFilter contains a series of string pairs that specify file filters,
    // separated by '\0' or '|' chars. Refer to the description of the OPENFILENAME
    // struct in the Windows API documentation.
    inline CFileDialog::CFileDialog(BOOL isOpenFileDialog  /* = TRUE */,
        LPCTSTR pDefExt /* = NULL */,
        LPCTSTR pInitFileDir /* = NULL */,
        LPCTSTR pFileName /* = NULL */,
        DWORD   flags /* = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT */,
        LPCTSTR pFilter /* = NULL */)
    {
        // set open/saveas toggle
        m_isOpenFileDialog = isOpenFileDialog;

        // clear out the OPENFILENAME structure
        ZeroMemory(&m_ofn, sizeof(m_ofn));

        // fill in the OPENFILENAME struct
        m_ofn.lpstrFile     = const_cast<LPTSTR>(pFileName);
        m_ofn.lpstrFilter   = pFilter;
        m_ofn.lpstrInitialDir = const_cast<LPTSTR>(pInitFileDir);
        m_ofn.lpstrDefExt   = pDefExt;
        m_ofn.Flags         = flags;

        // Enable the hook proc for the help button
        if (m_ofn.Flags & OFN_SHOWHELP)
            m_ofn.Flags |= OFN_ENABLEHOOK;

        // Safely set the remaining OPENFILENAME values
        SetParameters(m_ofn);
    }

    // Dialog procedure for the FileOpen and FileSave dialogs. Override
    // this function to customise the message handling.
    inline INT_PTR CFileDialog::DialogProc(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        //  Message intercepted by the hook procedure are passed here.
        //  Note: OnCancel and OnOK are not called here. They are called by DoModal
        //  in response to value returned by called by GetOpenFileName or GetSaveFileName.

        //  A typical function might look like this:

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

        // Always pass unhandled messages on to DialogProcDefault
        return DialogProcDefault(msg, wparam, lparam);
    }

    // The Default message handling for CFileDialog. Don't override this function,
    // override DialogProc instead.
    // Note: OnCancel and OnOK are called by DoModal.
    inline  INT_PTR CFileDialog::DialogProcDefault(UINT message, WPARAM wparam, LPARAM lparam)
    {
        switch (message)
        {
            case WM_INITDIALOG:
            {
                // handle the initialization message
                return OnInitDialog();
            }

            case WM_COMMAND:
            {
                // handle the HELP button for old-style file dialogs:
                if (LOWORD(wparam) == pshHelp)
                    OnHelpButton();

                return 0;
            }

            case WM_NOTIFY:
            {
                // handle messages for Explorer-style hook procedures:
                if (m_ofn.Flags & OFN_EXPLORER)
                {
                    LRESULT result = OnNotify(wparam, lparam);
                    SetWindowLongPtr(DWLP_MSGRESULT, result);
                    return result;
                }

                return 0;
            }
        }

        // dispatch special open/save file dialog messages
        if (message == UWM_HELPMSGSTRING)
        {   // handle the HELP button for old-style file dialogs:
            // (this alternate handler may still be used in some systems)
            OnHelpButton();
            return 0;
        }

        if (message == UWM_LBSELCHSTRING)
        {   // handle the registered list box selection change
            // notifications:
            OnLBSelChangedNotify(static_cast<UINT>(wparam), LOWORD(lparam), HIWORD(lparam));
            return 0;
        }

        if (message == UWM_SHAREVISTRING)
        {   // handle a sharing violation for the selected file that
            // occurred when the user clicked the OK button.
            return OnShareViolation(reinterpret_cast<LPCTSTR>(lparam));
        }

        if (message == UWM_FILEOKSTRING)
        {
            return OnFileNameOK();
        }

        // not processed here
        return 0;
    }

    // Display either a FileOpen or FileSave dialog, and allow the user to
    // select various options. An exception is thrown if the dialog isn't created.
    // If the OFN_ALLOWMULTISELECT flag is used, the size of the buffer required
    // to hold the file names can be quite large. An exception is thrown if the
    // buffer size specified by m_OFN.nMaxFile turns out to be too small.
    // Use SetParamaters to set a larger size if required.
    inline INT_PTR CFileDialog::DoModal(HWND owner /* = 0 */)
    {
        assert( GetApp() );    // Test if Win32++ has been started
        assert(!IsWindow());    // Only one window per CWnd instance allowed

        // Ensure this thread has the TLS index set
        TLSData* pTLSData = GetApp()->SetTlsData();

        // Create the modal dialog
        pTLSData->pWnd = this;

        m_ofn.hwndOwner = owner;
        m_ofn.lpstrFile = m_fileName.GetBuffer(m_ofn.nMaxFile);
        int ok = (m_isOpenFileDialog ? ::GetOpenFileName(&m_ofn) : ::GetSaveFileName(&m_ofn));
        m_fileName.ReleaseBuffer(m_ofn.nMaxFile);
        m_ofn.lpstrFile = const_cast<LPTSTR>(m_fileName.c_str());
        m_wnd = 0;

        // the result of the file choice box is processed here:
        if (!ok)
        {
            DWORD error = CommDlgExtendedError();
            if (error != 0)
            {
                // ignore the exception caused by closing the dialog
                if (error != CDERR_DIALOGFAILURE || (m_ofn.Flags & OFN_EXPLORER))
                    throw CWinException(g_msgWndDoModal, error);
            }

            OnCancel();
            return  IDCANCEL;
        }

        OnOK();
        return IDOK;
    }

    // Return the name of the file that was entered in the DoModal() operation.
    // This name consists of only the file title and extension. If the
    // OFN_ALLOWMULTISELECT flag is specified, only the first file path selected
    // will be returned. If so, GetNextPathName can be used to retrieve subsequent
    // file names.
    inline CString CFileDialog::GetFileName() const
    {
        CString fileName = GetPathName();
        int pos = fileName.ReverseFind(_T('\\'));
        if (pos >= 0)
            return fileName.Mid(pos + 1);

        fileName.Empty();
        return fileName;
    }

    // Return the file name's extension entered during the DoModal() operation.
    // If the OFN_ALLOWMULTISELECT option set, only the extension on the first
    // file path selected will be returned.
    inline CString CFileDialog::GetFileExt() const
    {
        CString fileExt = GetFileName();
        int pos = fileExt.ReverseFind(_T("."));
        if (pos >= 0)
            return fileExt.Mid(pos);

        fileExt.Empty();
        return fileExt;
    }

    // Return the title of the file entered in the DoModal() operation. The
    // title consists of the full path name with directory path and extension
    // removed.
    inline CString CFileDialog::GetFileTitle() const
    {
        CString fileTitle = GetFileName();
        int pos = fileTitle.ReverseFind(_T("."));
        if (pos >= 0)
            return fileTitle.Left(pos);

        return fileTitle;
    }

    // Return the next file path name from a group of files selected. The
    // OFN_ALLOWMULTISELECT flag allows multiple files to be selected. Use pos = 0
    // to retrieve the first file. The pos parameter is updated to point to the
    // next file name. The pos parameter is set to -1 when the last file is retrieved.
    inline CString CFileDialog::GetNextPathName(int& pos) const
    {
        assert(pos >= 0);

        BOOL isExplorer = m_ofn.Flags & OFN_EXPLORER;
        TCHAR delimiter = (isExplorer ? _T('\0') : _T(' '));

        int bufferSize = MIN(MAX_PATH, m_ofn.nMaxFile - pos);
        CString fileNames(m_ofn.lpstrFile + pos, bufferSize); // strFile can contain NULLs
        int index = 0;
        if (pos == 0)
        {
            index = fileNames.Find(delimiter);

            if ( (index < 0) || (fileNames.GetAt(++index) == _T('\0')))
            {
                // Only one file selected. m_OFN.lpstrFile contains a single string
                // consisting of the path and file name.
                pos = -1;
                return m_ofn.lpstrFile;
            }
        }

        // Multiple files selected. m_OFN.lpstrFile contains a set of substrings separated
        // by delimiters. The first substring is the path, the following ones are file names.

        CString pathName = m_ofn.lpstrFile; // strPath is terminated by first NULL
        if (!isExplorer)
        {
            int pathIndex = pathName.Find(delimiter);
            pathName = pathName.Left(pathIndex);
        }

        CString fileName = m_ofn.lpstrFile + pos + index;
        if (!isExplorer)
        {
            int fileIndex = fileName.Find(delimiter);
            if (fileIndex > 0)
                fileName = fileName.Left(fileIndex);
        }

        // Update pos to point to the next file
        int fileLength = lstrlen(fileName);
        if (fileNames.GetAt(index + fileLength + 1) == _T('\0'))
            pos = -1;
        else
            pos = pos + index + fileLength +1;

        if (!pathName.IsEmpty())
        {
            // Get the last character from the path
            int nPathLen = pathName.GetLength();
            TCHAR termination = pathName.GetAt(nPathLen -1);

            if (termination == _T('\\'))
            {
                // Path already ends with _T('\\')
                return pathName + fileName;
            }
        }

        // Add _T('\\') to the end of the path
        return pathName + _T('\\') + fileName;
    }

    // Return the path name of the folder or directory of files retrieved
    // from the dialog. The final character of the name includes the
    // directory separation character.
    inline CString CFileDialog::GetFolderPath() const
    {
        CString folderName = GetPathName();
        int pos = folderName.ReverseFind(_T('\\'));
        if (pos >= 0)
            return folderName.Left(pos + 1);

        folderName.Empty();
        return folderName;
    }

    // Returns the full path name of the file that was retrieved from the dialog.
    // If the m_OFN.Flags member includes the OFN_ALLOWMULTISELECT option,
    // this method returns only the first selected file. Use the GetNextPathName
    // to retrieve the remaining file names.
    inline CString CFileDialog::GetPathName() const
    {
        if ((m_ofn.Flags & OFN_ALLOWMULTISELECT) == 0)
        {
            // just retrieve the path from the OPENFILENAME structure
            return m_ofn.lpstrFile;
        }
        else
        {
            int pos = 0;
            return GetNextPathName(pos);
        }
    }

    // Normally, overriding this function is not warranted because the
    // framework provides default validation of file names.
    // Override this function and return TRUE to reject a file name for
    // any application specific reasons.
    inline LRESULT CFileDialog::OnFileNameOK()
    {
        return 0;
    }

    // Override this method to handle the WM_NOTIFY CDN_INITDONE message.
    // The notification message is sent when the system has finished arranging
    // controls in the Open or SaveAs dialog to make room for the controls of
    // the child dialog box, but before the dialog is visible. Notification is
    // sent only if the dialog box was created with the OFN_EXPLORER style.
    inline void CFileDialog::OnInitDone()
    {
    }

    // Override this method to handle the WM_NOTIFY CDN_SELCHANGE message.
    // The notification message is sent when the user selects a new file or
    // folder in the file list of the Open or SaveAs dialog box. Notification
    // is sent only if the dialog box was created with the OFN_EXPLORER style.
    inline void CFileDialog::OnFileNameChange()
    {
    }

    // Override this method to handle the WM_NOTIFY CDN_SELCHANGE message.
    // The notification message is sent when the user selects a new file or
    // folder in the file list of the Open or SaveAs dialog box. Notification
    // is sent only if the dialog box was created with the OFN_EXPLORER style.
    inline void CFileDialog::OnFolderChange()
    {
    }

    // This method is called whenever the current selection in a list box is
    // about to change. Override this method to provide custom handling of
    // selection changes in the list box. The ID of the list or combo box in
    // which the selection occurred is boxID. The index of the current
    // selection is curSel. The control notification code is code.
    inline void CFileDialog::OnLBSelChangedNotify(UINT boxID, UINT curSel, UINT code)
    {
        UNREFERENCED_PARAMETER(boxID);
        UNREFERENCED_PARAMETER(curSel);
        UNREFERENCED_PARAMETER(code);

    }

    // This method handles the WM_NOTIFY message loop functions of the hook
    // procedure.
    inline LRESULT CFileDialog::OnNotify(WPARAM wparam, LPARAM lparam)
    {
        UNREFERENCED_PARAMETER(wparam);

        OFNOTIFY* pNotify = reinterpret_cast<OFNOTIFY*>(lparam);
        assert(pNotify);
        if (!pNotify) return 0;

        switch(pNotify->hdr.code)
        {
            case CDN_INITDONE:
                OnInitDone();
                return TRUE;

            case CDN_SELCHANGE:
                OnFileNameChange();
                return TRUE;

            case CDN_FOLDERCHANGE:
                OnFolderChange();
                return TRUE;

            case CDN_SHAREVIOLATION:
                return OnShareViolation(pNotify->pszFile);

            case CDN_HELP:
                OnHelpButton();
                return TRUE;

            case CDN_FILEOK:
                return OnFileNameOK();

            case CDN_TYPECHANGE:
                OnTypeChange();
                return TRUE;
        }

        // The framework will call SetWindowLongPtr(DWLP_MSGRESULT, result) for non-zero returns
        return FALSE;   // not handled
    }

    // Override this function to provide custom handling of share violations.
    // Normally, this function is not needed because the framework provides
    // default checking of share violations and displays a message box if a
    // share violation occurs. The path of the file on which the share
    // violation occurred is pPathName. To disable share violation checking,
    // use the bitwise OR operator to combine the flag OFN_SHAREAWARE with
    // m_OFN.Flags.

    // Return one of the following values to indicate how the dialog box
    // should handle the sharing violation.
    // OFN_SHAREFALLTHROUGH  - Accept the file name
    // OFN_SHARENOWARN  - Reject the file name but do not warn the user.
    //                    The application is responsible for displaying a warning message.
    // OFN_SHAREWARN    - Reject the file name and displays a warning message
    //                    (the same result as if there were no hook procedure).
    inline LRESULT CFileDialog::OnShareViolation(LPCTSTR pPathName )
    {
        UNREFERENCED_PARAMETER(pPathName);

        return OFN_SHAREWARN; // default:
    }

    // Override this method to handle the WM_NOTIFY CDN_TYPECHANGE message.
    // The notification message is sent when the user selects a new file type
    // from the list of file types in the Open or SaveAs dialog box.
    // Notification is sent only if the dialog box was created with the
    // OFN_EXPLORER style.
    inline void CFileDialog::OnTypeChange()
    {
    }

    // Set the default extension of the dialog box to pExt.
    // Only the first three characters are sent to the dialog.
    inline void CFileDialog::SetDefExt(LPCTSTR pExt)
    {
        if (pExt)
        {
            m_defExt = pExt;
            m_ofn.lpstrDefExt = m_defExt.c_str();
        }
        else
        {
            m_defExt.Empty();
            m_ofn.lpstrDefExt = NULL;
        }
    }

    // Set the initial file name in the dialog box to pFileName.
    inline void CFileDialog::SetFileName(LPCTSTR pFileName)
    {
        // setup initial file name
        if (pFileName)
        {
            m_fileName = pFileName;
            m_ofn.lpstrFile = const_cast<LPTSTR>(m_fileName.c_str());
        }
        else
        {
            m_fileName.Empty();
            m_ofn.lpstrFile = NULL;
        }
    }

    // Set the file choice dialog file name filter string to pFilter.
    // The string is a pair of strings delimited by NULL or '|'
    // The string must be either double terminated, or use '|' instead of '\0'
    // For Example: _T("Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0")
    //          or: _T("Text Files (*.txt)|*.txt|All Files (*.*)|*.*|")
    inline void CFileDialog::SetFilter(LPCTSTR pFilter)
    {
        // Clear any existing filter
        m_ofn.lpstrFilter = NULL;

        // convert any '|' characters in pFilter to NULL characters
        if (pFilter)
        {
            CString str = pFilter;
            if (str.Find(_T('|')) >= 0)
            {
                str.Replace(_T('|'), _T('\0'));
                m_filter = str;
                m_ofn.lpstrFilter = m_filter.c_str();
            }
            else
            {
                // pFilter doesn't contain '|', so it should be double terminated
                int i = 0;
                while (i < MAX_PATH)
                {
                    // Search for double termination
                    if (pFilter[i] == _T('\0') && pFilter[i + 1] == _T('\0'))
                    {
                        m_filter.Assign(pFilter, i+1);
                        m_ofn.lpstrFilter = m_filter.c_str();
                        break;
                    }
                    ++i;
                }
            }
        }
    }

    // Sets the various parameters of the OPENFILENAME struct.
    // The parameters are set to sensible values.
    inline void CFileDialog::SetParameters(const OPENFILENAME& ofn)
    {
        // Set the correct struct size for all Windows versions and compilers
        DWORD StructSize = sizeof(m_ofn);

  #if defined OPENFILENAME_SIZE_VERSION_400
        if (GetWinVersion() < 2500)
            StructSize = OPENFILENAME_SIZE_VERSION_400;
        if (GetWinVersion() >= 2500)
            m_ofn.FlagsEx =     ofn.FlagsEx;
  #endif

        SetFileName(ofn.lpstrFile);
        SetFilter(ofn.lpstrFilter);
        SetTitle(ofn.lpstrFileTitle);

        m_ofn.lStructSize       = StructSize;
        m_ofn.hwndOwner         = 0;            // Set this in DoModal
        m_ofn.hInstance         = GetApp()->GetInstanceHandle();
        m_ofn.lpstrCustomFilter = ofn.lpstrCustomFilter;
        m_ofn.nMaxCustFilter    = MAX(MAX_PATH, ofn.nMaxCustFilter);
        m_ofn.nFilterIndex      = ofn.nFilterIndex;

        // Allocate a bigger buffer for multiple files
        if (ofn.Flags & OFN_ALLOWMULTISELECT)
            m_ofn.nMaxFile = MAX(MAX_PATH * 256, ofn.nMaxFile);
        else
            m_ofn.nMaxFile = MAX(MAX_PATH, ofn.nMaxFile);

        m_ofn.lpstrFileTitle    = ofn.lpstrFileTitle;
        m_ofn.nMaxFileTitle     = MAX(MAX_PATH, ofn.nMaxFileTitle);
        m_ofn.lpstrInitialDir   = ofn.lpstrInitialDir;
        m_ofn.Flags             = ofn.Flags;
        m_ofn.nFileOffset       = ofn.nFileOffset;
        m_ofn.nFileExtension    = ofn.nFileExtension;
        m_ofn.lpstrDefExt       = ofn.lpstrDefExt;
        m_ofn.lCustData         = ofn.lCustData;
        m_ofn.lpfnHook          = reinterpret_cast<LPCCHOOKPROC>(CDHookProc);
    }

    // Sets the title of the fileopen or filesave dialog.
    inline void CFileDialog::SetTitle(LPCTSTR pTitle)

    {
        if (pTitle)
        {
            m_title = pTitle;
            m_ofn.lpstrTitle = m_title.c_str();
        }
        else
        {
            m_title.Empty();
            m_ofn.lpstrTitle = NULL;
        }
    }


    ///////////////////////////////////////////////
    // Definitions for the CFindReplaceDialog class
    //

    // Constructor for CCFindReplaceDialog. Refer to the Windows API documentation
    // for information of the FINDREPLACE structure.
    inline CFindReplaceDialog::CFindReplaceDialog(BOOL isFindDialogOnly /* = TRUE */)
    {
        ZeroMemory(&m_fr, sizeof(m_fr));
        m_isFindDialogOnly = isFindDialogOnly;
        SetParameters(m_fr);
    }

    // Create and display either a Find or FindReplace dialog box.
    inline HWND CFindReplaceDialog::Create(HWND parent /* = 0*/)
    {
        Create(m_isFindDialogOnly, m_fr.lpstrFindWhat, m_fr.lpstrReplaceWith, m_fr.Flags, parent);
        return *this;
    }

    // Create and display either a Find or FindReplace dialog box. pFindWhat
    // is the search string, and pReplaceWith is the replace string.
    // Set flags to a combination of one or more flags the dialog box.
    // Set parent to the handle of the dialog box’s parent or owner window.
    // An exception is thrown if the window isn't created.
    inline BOOL CFindReplaceDialog::Create(BOOL isFindDialogOnly, LPCTSTR pFindWhat,
            LPCTSTR pReplaceWith, DWORD flags, HWND parent /* = 0*/)
    {
        assert( GetApp() );    // Test if Win32++ has been started
        assert(!IsWindow());    // Only one window per CWnd instance allowed

        m_isFindDialogOnly = isFindDialogOnly;

        // Ensure this thread has the TLS index set
        TLSData* pTLSData = GetApp()->SetTlsData();
        pTLSData->pWnd = this;

        // Initialize the FINDREPLACE struct values
        m_fr.Flags = flags;
        SetParameters(m_fr);
        m_fr.hwndOwner = parent;

        m_fr.lpstrFindWhat = m_findWhat.GetBuffer(m_fr.wFindWhatLen);
        if (pFindWhat)
            StrCopy(m_fr.lpstrFindWhat, pFindWhat, m_fr.wFindWhatLen);

        m_fr.lpstrReplaceWith = m_replaceWith.GetBuffer(m_fr.wReplaceWithLen);
        if (pReplaceWith)
            StrCopy(m_fr.lpstrReplaceWith, pReplaceWith, m_fr.wReplaceWithLen);

        // Display the dialog box
        HWND wnd;
        if (isFindDialogOnly)
            wnd = ::FindText(&m_fr);
        else
            wnd = ::ReplaceText(&m_fr);

        if (wnd == 0)
        {
            // Throw an exception when window creation fails
            throw CWinException(g_msgWndDoModal);
        }

        m_findWhat.ReleaseBuffer();
        m_fr.lpstrFindWhat = const_cast<LPTSTR>(m_findWhat.c_str());
        m_replaceWith.ReleaseBuffer();
        m_fr.lpstrReplaceWith = const_cast<LPTSTR>(m_replaceWith.c_str());

        return TRUE;
    }

    // Dialog procedure for the Find and Replace dialogs. Override this function
    // to customise the message handling.
    inline INT_PTR CFindReplaceDialog::DialogProc(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        //  Message intercepted by the hook procedure are passed here.
        //  A typical function might look like this:

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

        // Always pass unhandled messages on to DialogProcDefault
        return DialogProcDefault(msg, wparam, lparam);
    }

    // The Default message handling for CFindReplaceDialog. Don't override this function,
    // override DialogProc instead.
    // Note: OnCancel and OnOK are called by DoModal.
    inline INT_PTR CFindReplaceDialog::DialogProcDefault(UINT message, WPARAM wparam, LPARAM lparam)
    {
        UNREFERENCED_PARAMETER(lparam);

        switch (message)
        {
        case WM_INITDIALOG:
            {     // handle the initialization message
                return OnInitDialog();
            }

        case WM_COMMAND:
            {
                if (LOWORD(wparam) == pshHelp)
                    OnHelpButton();

                return 0;
            }
        }

        return 0;
    }

    // Call this function to determine whether the user wants to find the next
    // occurrence of the search string.
    inline BOOL CFindReplaceDialog::FindNext() const
    {
        return ((m_fr.Flags & FR_FINDNEXT )!= 0);
    }

    // Call this function to return the default string to find.
    inline  CString CFindReplaceDialog::GetFindString() const
    {
        return m_fr.lpstrFindWhat;
    }

    // Return a pointer to the current Find/Replace dialog box. This may be
    // used when owner window responds to the UWM_FINDMSGSTRING message.
    // The lparam value is that passed in the UWM_FINDMSGSTRING message.
    inline CFindReplaceDialog* CFindReplaceDialog::GetNotifier(LPARAM lparam)
    {
        assert(lparam != 0);
        LPFINDREPLACE pFR = reinterpret_cast<LPFINDREPLACE>(lparam);
        if (!pFR) return NULL;

        CFindReplaceDialog* pDlg = reinterpret_cast<CFindReplaceDialog*>(pFR->lCustData);
        return pDlg;
    }

    // Call this function to return the current replace string.
    inline CString CFindReplaceDialog::GetReplaceString() const
    {
        return m_fr.lpstrReplaceWith == NULL ? _T("") : m_fr.lpstrReplaceWith;
    }

    // Returns TRUE if the user has decided to terminate the dialog box;
    inline BOOL CFindReplaceDialog::IsTerminating()
    {
        return ((m_fr.Flags & FR_DIALOGTERM) != 0);
    }

    // Return TRUE if the user wants to find occurrences of the search string
    // that exactly match the case of the search string; otherwise FALSE.
    inline BOOL CFindReplaceDialog::MatchCase() const
    {
        return ((m_fr.Flags & FR_MATCHCASE) != 0);
    }

    // Return TRUE if the user wants to match only the entire words of the
    // search string.
    inline BOOL CFindReplaceDialog::MatchWholeWord() const
    {
        return ((m_fr.Flags & FR_WHOLEWORD) != 0);
    }

    // Return TRUE if the user has requested that all strings matching the
    // replace string be replaced.
    inline BOOL CFindReplaceDialog::ReplaceAll() const
    {
        return ((m_fr.Flags & FR_REPLACEALL) != 0);
    }

    // Return TRUE if the user has requested that the currently selected string
    // be replaced with the replace string.
    inline BOOL CFindReplaceDialog::ReplaceCurrent() const
    {
        return ((m_fr.Flags & FR_REPLACE) != 0);
    }

    // Return TRUE if the user wants the search to proceed in a downward
    // direction; FALSE if the user wants the search to proceed in an upward
    // direction.
    inline BOOL CFindReplaceDialog::SearchDown() const
    {
        return ((m_fr.Flags & FR_DOWN) != 0);
    }

    // Sets the various parameters of the FINDREPLACE struct.
    // The parameters are set to sensible values.
    inline void CFindReplaceDialog::SetParameters(const FINDREPLACE& fr)
    {
        int maxChars = 128;

        if (fr.lpstrFindWhat)
        {
            m_findWhat = fr.lpstrFindWhat;
            maxChars = MAX(maxChars, lstrlen(fr.lpstrFindWhat));
        }
        else
            m_findWhat.Empty();

        if (fr.lpstrReplaceWith)
        {
            m_replaceWith = fr.lpstrReplaceWith;
            maxChars = MAX(maxChars, lstrlen(fr.lpstrReplaceWith));
        }
        else
            m_replaceWith.Empty();

        m_fr.lStructSize        = sizeof(m_fr);
        m_fr.hwndOwner          = 0;        // Set this in Create
        m_fr.hInstance          = GetApp()->GetInstanceHandle();
        m_fr.Flags              = fr.Flags;
        m_fr.lpstrFindWhat      = const_cast<LPTSTR>(m_findWhat.c_str());
        m_fr.lpstrReplaceWith   = const_cast<LPTSTR>(m_replaceWith.c_str());
        m_fr.wFindWhatLen       = static_cast<WORD>(MAX(fr.wFindWhatLen, maxChars));
        m_fr.wReplaceWithLen    = static_cast<WORD>(MAX(fr.wReplaceWithLen, maxChars));
        m_fr.lCustData          = reinterpret_cast<LPARAM>(this);
        m_fr.lpfnHook           = reinterpret_cast<LPCCHOOKPROC>(CDHookProc);
        m_fr.lpTemplateName     = fr.lpTemplateName;

        // Enable the hook proc for the help button
        if (m_fr.Flags & FR_SHOWHELP)
            m_fr.Flags |= FR_ENABLEHOOK;
    }


    ////////////////////////////////////////
    // Definitions for the CFontDialog class
    //

    // Construct a CFontDialog object from values given. Note that these are
    // stored into the members of the CHOOSEFONT structure.
    // Refer to the description of the CHOOSEFONT structure in the Windows API
    // documentation for more information on these parameters.
    inline CFontDialog::CFontDialog(const LOGFONT& initial, DWORD flags /* = 0 */,
        HDC printer /* = 0 */)
    {
          // clear out logfont, style name, and choose font structure
        ZeroMemory(&m_logFont, sizeof(m_logFont));
        ZeroMemory(&m_cf, sizeof(m_cf));

        // set dialog parameters
        m_cf.rgbColors   = 0; // black
        m_cf.lStructSize = sizeof(m_cf);
        m_cf.Flags  = flags;
        m_cf.Flags |= CF_INITTOLOGFONTSTRUCT;
        m_cf.lpLogFont = const_cast<LOGFONT*>(&initial);

        if (printer)
        {
            m_cf.hDC = printer;
            m_cf.Flags |= CF_PRINTERFONTS;
        }

        // Enable the hook proc for the help button
        if (m_cf.Flags & CF_SHOWHELP)
            m_cf.Flags |= CF_ENABLEHOOK;

        SetParameters(m_cf);
    }

    // Construct a CFontDialog dialog object that can be used to create a font
    // for use in a rich edit control. The charformat points to a CHARFORMAT
    // data structure that allows setting some of the font's characteristics
    // for a rich edit control.
    // Refer to the description of the CHOOSEFONT structure in the Windows API
    // documentation for more information on these parameters.
    inline CFontDialog::CFontDialog(const CHARFORMAT& charformat, DWORD flags /* = 0 */,
        HDC printer /* =  0 */)
    {
        // clear out logfont, style name, and choose font structure
        ZeroMemory(&m_logFont, sizeof(m_logFont));
        ZeroMemory(&m_cf, sizeof(m_cf));
        m_cf.lStructSize = sizeof(m_cf);

        // set dialog parameters
        FillInLogFont(charformat);
        m_cf.lpLogFont = &m_logFont;
        m_cf.Flags  = flags | CF_INITTOLOGFONTSTRUCT;

        if (charformat.dwMask & CFM_COLOR)
            m_cf.rgbColors = charformat.crTextColor;

        if (printer)
        {
            m_cf.hDC = printer;
            m_cf.Flags |= CF_PRINTERFONTS;
        }

        // Enable the hook proc for the help button
        if (m_cf.Flags & CF_SHOWHELP)
            m_cf.Flags |= CF_ENABLEHOOK;

        SetParameters(m_cf);
    }

    // Construct a default CFontDialog object.
    inline CFontDialog::CFontDialog(DWORD flags /* = 0 */, HDC printer /* =  0 */)
    {
        // clear out logfont, style name, and choose font structure
        ZeroMemory(&m_logFont, sizeof(m_logFont));
        ZeroMemory(&m_cf, sizeof(m_cf));

        // set dialog parameters
        m_cf.rgbColors = 0; // black
        m_cf.lStructSize = sizeof(m_cf);
        m_cf.Flags  = flags;

        if (printer)
        {
            m_cf.hDC = printer;
            m_cf.Flags |= CF_PRINTERFONTS;
        }

        // Enable the hook proc for the help button
        if (m_cf.Flags & CF_SHOWHELP)
            m_cf.Flags |= CF_ENABLEHOOK;

        SetParameters(m_cf);
    }

    // Dialog procedure for the Font dialog. Override this function
    // to customise the message handling.
    inline INT_PTR CFontDialog::DialogProc(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        //  Message intercepted by the hook procedure are passed here.
        //  Note: OnCancel and OnOK are not called here. They are called by DoModal
        //  in response to value returned by called by ChooseFont.

        //  A typical function might look like this:

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

        // Always pass unhandled messages on to DialogProcDefault
        return DialogProcDefault(msg, wparam, lparam);
    }

    // The Default message handling for CFontDialog.
    // Don't override this function, override DialogProc instead.
    // Note: OnCancel and OnOK are called by DoModal.
    inline INT_PTR CFontDialog::DialogProcDefault(UINT message, WPARAM wparam, LPARAM lparam)
    {
        UNREFERENCED_PARAMETER(lparam);
        if (message == WM_INITDIALOG)
        {
            OnInitDialog();
        }

        // dispatch special commdlg messages
        if (message == WM_COMMAND && LOWORD(wparam) == pshHelp)
            OnHelpButton();

        // default processing
        return 0;
    }

    // Display the FontDialog. hOwner specifies dialog's owner window.
    inline INT_PTR CFontDialog::DoModal(HWND owner /* = 0 */)
    {
        assert( GetApp() );    // Test if Win32++ has been started
        assert(!IsWindow());    // Only one window per CWnd instance allowed

        // Ensure this thread has the TLS index set
        TLSData* pTLSData = GetApp()->SetTlsData();

        // Create the modal dialog
        pTLSData->pWnd = this;

        m_cf.hwndOwner = owner;
        m_cf.lpszStyle = m_styleName.GetBuffer(80);

        // open the font choice dialog
        BOOL ok = ::ChooseFont(&m_cf);

        m_styleName.ReleaseBuffer();
        m_cf.lpszStyle = const_cast<LPTSTR>(m_styleName.c_str());
        m_wnd = 0;

        // process the result of the font choice box:
        if (!ok)
        {
            DWORD error = CommDlgExtendedError();
            if ((error != 0) && (error != CDERR_DIALOGFAILURE))
                // ignore the exception caused by closing the dialog
                throw CWinException(g_msgWndDoModal, error);

            OnCancel();
            return IDCANCEL;
        }

        OnOK();
        return IDOK;
    }

    // Returns the CHARFORMAT of this font object, as translated from the
    // information in the m_FR CHOOSEFONT struct.
    inline CHARFORMAT CFontDialog::GetCharFormat() const
    {
        CHARFORMAT chfmt;
        ZeroMemory(&chfmt, sizeof(chfmt));
        chfmt.cbSize = sizeof(chfmt);

        if ((m_cf.Flags & CF_NOSTYLESEL) == 0)
        {
            chfmt.dwMask    |= CFM_BOLD | CFM_ITALIC;
            chfmt.dwEffects |= (IsBold()) ? CFE_BOLD : 0;
            chfmt.dwEffects |= (IsItalic()) ? CFE_ITALIC : 0;
        }

        if ((m_cf.Flags & CF_NOSIZESEL) == 0)
        {
            chfmt.dwMask |= CFM_SIZE;
              // GetSize() returns 1/10th points, convert to  twips
            chfmt.yHeight = GetSize() * 2;
        }

        if ((m_cf.Flags & CF_NOFACESEL) == 0)
        {
            chfmt.dwMask |= CFM_FACE;
            chfmt.bPitchAndFamily = m_cf.lpLogFont->lfPitchAndFamily;
            StrCopy(chfmt.szFaceName, GetFaceName().c_str(), LF_FACESIZE);
        }

        if (m_cf.Flags & CF_EFFECTS)
        {
            chfmt.dwMask |= CFM_UNDERLINE | CFM_STRIKEOUT | CFM_COLOR;
            chfmt.dwEffects |= (IsUnderline()) ? CFE_UNDERLINE : 0;
            chfmt.dwEffects |= (IsStrikeOut()) ? CFE_STRIKEOUT : 0;
            chfmt.crTextColor = GetColor();
        }

        if ((m_cf.Flags & CF_NOSCRIPTSEL) == 0)
        {
            chfmt.bCharSet = m_cf.lpLogFont->lfCharSet;
            chfmt.dwMask |= CFM_CHARSET;
        }

        chfmt.yOffset = 0;
        return chfmt;
    }

    // Return the current font size, in 1/10th points (1 pt = 1/72 inch).
    inline int CFontDialog::GetSize() const
    {
        HDC dc = ::GetDC(NULL); // the device context for the entire screen

        // number of pixels per inch along the screen height.
        int pxpi = GetDeviceCaps(dc, LOGPIXELSY);

        // point size is (pixel height) * 72 / pxpi, so in 1/10ths size is
        int charsize = -MulDiv(m_logFont.lfHeight, 720, pxpi);
        ::ReleaseDC(NULL, dc);
        return charsize;
    }

    // Called when the OK button is pressed on the Font Dialog.
    inline void CFontDialog::OnOK()
    {
    }

    // Translate the font character format cf properties of a CRichEdit control
    // into elements of the m_logFont member and settings of the option flags.
    inline DWORD CFontDialog::FillInLogFont(const CHARFORMAT& cf)
    {
        DWORD flags = 0;
        if (cf.dwMask & CFM_SIZE)
        {
            CDC dc;
            dc.CreateDC(_T("DISPLAY"), NULL, NULL, NULL);
            LONG yPerInch = dc.GetDeviceCaps(LOGPIXELSY);
            m_logFont.lfHeight = - (cf.yHeight * yPerInch) / 1440;
        }
        else
            m_logFont.lfHeight = 0;

        m_logFont.lfWidth = 0;
        m_logFont.lfEscapement = 0;
        m_logFont.lfOrientation = 0;

        if ((cf.dwMask & (CFM_ITALIC|CFM_BOLD)) == (CFM_ITALIC|CFM_BOLD))
        {
            m_logFont.lfWeight = (cf.dwEffects & CFE_BOLD) ? FW_BOLD : FW_NORMAL;
            m_logFont.lfItalic = (cf.dwEffects & CFE_ITALIC) ? TRUE : FALSE;
        }
        else
        {
            flags |= CF_NOSTYLESEL;
            m_logFont.lfWeight = FW_DONTCARE;
            m_logFont.lfItalic = FALSE;
        }

        if ((cf.dwMask & (CFM_UNDERLINE|CFM_STRIKEOUT|CFM_COLOR)) ==
            (CFM_UNDERLINE|CFM_STRIKEOUT|CFM_COLOR))
        {
            flags |= CF_EFFECTS;
            m_logFont.lfUnderline = (cf.dwEffects & CFE_UNDERLINE) ? TRUE : FALSE;
            m_logFont.lfStrikeOut = (cf.dwEffects & CFE_STRIKEOUT) ? TRUE : FALSE;
        }
        else
        {
            m_logFont.lfUnderline = (BYTE)FALSE;
            m_logFont.lfStrikeOut = (BYTE)FALSE;
        }

        if (cf.dwMask & CFM_CHARSET)
            m_logFont.lfCharSet = cf.bCharSet;
        else
            flags |= CF_NOSCRIPTSEL;

        m_logFont.lfOutPrecision = OUT_DEFAULT_PRECIS;
        m_logFont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
        m_logFont.lfQuality = DEFAULT_QUALITY;

        if (cf.dwMask & CFM_FACE)
        {
            m_logFont.lfPitchAndFamily = cf.bPitchAndFamily;
            StrCopy(m_logFont.lfFaceName, cf.szFaceName, LF_FACESIZE);
        }
        else
        {
            m_logFont.lfPitchAndFamily = DEFAULT_PITCH|FF_DONTCARE;
            m_logFont.lfFaceName[0] = (TCHAR)0;
        }

        return flags;
    }

    // Sets the various parameters of the CHOOSEFONT struct.
    // The parameters are set to safe values.
    inline void CFontDialog::SetParameters(const CHOOSEFONT& cf)
    {
        if (cf.lpLogFont)
            m_logFont = *cf.lpLogFont;
        else
            ZeroMemory(&m_logFont, sizeof(m_logFont));

        if (cf.lpszStyle)
            m_styleName = cf.lpszStyle;
        else
            m_styleName.Empty();

        m_cf.lStructSize    = sizeof(m_cf);
        m_cf.hwndOwner      = 0;        // Set this in DoModal
        m_cf.hDC            = cf.hDC;
        m_cf.lpLogFont      = &m_logFont;
        m_cf.iPointSize     = cf.iPointSize;
        m_cf.Flags          = cf.Flags;
        m_cf.rgbColors      = cf.rgbColors;
        m_cf.lCustData      = cf.lCustData;
        m_cf.lpfnHook       = reinterpret_cast<LPCCHOOKPROC>(CDHookProc);
        m_cf.lpTemplateName = cf.lpTemplateName;
        m_cf.hInstance      = GetApp()->GetInstanceHandle();
        m_cf.lpszStyle      = const_cast<LPTSTR>(m_styleName.c_str());
        m_cf.nFontType      = cf.nFontType;
        m_cf.nSizeMin       = cf.nSizeMin;
        m_cf.nSizeMax       = cf.nSizeMax;
    }

}



#endif // _WIN32XX_COMMONDLG_H_

