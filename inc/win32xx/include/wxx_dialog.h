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
// wxx_dialog.h
//  Declaration of the CDialog class

// CDialog adds support for dialogs to Win32++. Dialogs are specialised
// windows which are a parent window for common controls. Common controls
// are special window types such as buttons, edit controls, tree views,
// list views, static text etc.

// The layout of a dialog is typically defined in a resource script file
// (often Resource.rc). While this script file can be constructed manually,
// it is often created using a resource editor. If your compiler doesn't
// include a resource editor, you might find ResEdit useful. It is a free
// resource editor available for download at:
// http://www.resedit.net/

// CDialog supports modal and modeless dialogs. It also supports the creation
// of dialogs defined in a resource script file, as well as those defined in
// a dialog template.

// Use the Dialog generic program as the starting point for your own dialog
// applications.
// The DialogDemo sample demonstrates how to use subclassing to customise
// the behaviour of common controls in a dialog.

// Note: The following functions often used with dialogs are provided by CWnd:
//  GetDlgCtrlID, GetDlgItem, GetDlgItemInt, GetDlgItemText, GetNextDlgGroupItem,
//  GetNextDlgTabItem, SendDlgItemMessage, SetDlgItemInt, SetDlgItemText


#ifndef _WIN32XX_DIALOG_H_
#define _WIN32XX_DIALOG_H_

#include "wxx_wincore.h"

#ifndef SWP_NOCOPYBITS
    #define SWP_NOCOPYBITS      0x0100
#endif


namespace Win32xx
{
    // This class displays and manages a dialog.
    class CDialog : public CWnd
    {

    public:
        CDialog(UINT nResID);
        CDialog(LPCTSTR lpszResName);
        CDialog(LPCDLGTEMPLATE lpTemplate);
        virtual ~CDialog();

        // You probably won't need to override these functions
        virtual void AttachItem(int nID, CWnd& Wnd);
        virtual HWND Create(HWND hWndParent = 0) { return DoModeless(hWndParent); }
        virtual INT_PTR DoModal(HWND hWndParent = 0);
        virtual HWND DoModeless(HWND hWndParent = 0);
        virtual BOOL IsModal() const { return m_IsModal; }
        BOOL IsIndirect() const { return (NULL != m_lpTemplate); }

    protected:
        // These are the functions you might wish to override
        virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
        virtual void EndDialog(INT_PTR nResult);
        virtual void OnCancel();
        virtual void OnClose();
        virtual BOOL OnInitDialog();
        virtual void OnOK();
        virtual BOOL PreTranslateMessage(MSG& Msg);

        // Not intended to be overridden
        virtual INT_PTR DialogProcDefault(UINT uMsg, WPARAM wParam, LPARAM lParam);

        // Can't override these functions
        DWORD GetDefID() const;
        void GotoDlgCtrl(HWND hWndCtrl);
        BOOL MapDialogRect(RECT& rc) const;
        void NextDlgCtrl() const;
        void PrevDlgCtrl() const;
        void SetDefID(UINT nID);

    private:
        CDialog(const CDialog&);              // Disable copy construction
        CDialog& operator = (const CDialog&); // Disable assignment operator

        static INT_PTR CALLBACK StaticDialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

#ifndef _WIN32_WCE
        static LRESULT CALLBACK StaticMsgHook(int nCode, WPARAM wParam, LPARAM lParam);
#endif

        BOOL m_IsModal;                 // a flag for modal dialogs
        LPCTSTR m_lpszResName;          // the resource name for the dialog
        LPCDLGTEMPLATE m_lpTemplate;    // the dialog template for indirect dialogs
    };


#ifndef _WIN32_WCE

    //////////////////////////////////////
    // Declaration of the CResizer class
    //
    // The CResizer class can be used to rearrange a dialog's child
    // windows when the dialog is resized.

    // To use CResizer, follow the following steps:
    // 1) Use Initialize to specify the dialog's CWnd, and min and max size.
    // 3) Use AddChild for each child window
    // 4) Call HandleMessage from within DialogProc.
    //

    // Resize Dialog Styles
#define RD_STRETCH_WIDTH        0x0001  // The item has a variable width
#define RD_STRETCH_HEIGHT       0x0002  // The item has a variable height

    // Resize Dialog alignments
    enum Alignment { topleft, topright, bottomleft, bottomright };


    // The CResizer class can be used to rearrange a dialog's child
    // windows when the dialog is resized.
    class CResizer
    {
    public:
        CResizer() : m_hParent(0), m_xScrollPos(0), m_yScrollPos(0) {}
        virtual ~CResizer() {}

        virtual void AddChild(HWND hWnd, Alignment corner, DWORD dwStyle);
        virtual void HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
        virtual void Initialize(HWND hParent, const RECT& rcMin, const RECT& rcMax = CRect(0,0,0,0));
        virtual void OnHScroll(UINT uMsg, WPARAM wParam, LPARAM lParam);
        virtual void OnVScroll(UINT uMsg, WPARAM wParam, LPARAM lParam);
        virtual void RecalcLayout();
        CRect GetMinRect() const { return m_rcMin; }
        CRect GetMaxRect() const { return m_rcMax; }

        struct ResizeData
        {
            CRect rcInit;
            CRect rcOld;
            Alignment corner;
            BOOL IsFixedWidth;
            BOOL IsFixedHeight;
            HWND hWnd;
        };

    private:
        static BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam);

        HWND m_hParent;
        std::vector<ResizeData> m_vResizeData;

        CRect m_rcInit;
        CRect m_rcMin;
        CRect m_rcMax;

        int m_xScrollPos;
        int m_yScrollPos;
    };

#endif

}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


namespace Win32xx
{
    ////////////////////////////////////
    // Definitions for the CDialog class
    //

    inline CDialog::CDialog(LPCTSTR lpszResName) : m_IsModal(TRUE),
                        m_lpszResName(lpszResName), m_lpTemplate(NULL)
    {
        // Initialize the common controls.
        LoadCommonControls();
    }

    inline CDialog::CDialog(UINT nResID) : m_IsModal(TRUE),
                        m_lpszResName(MAKEINTRESOURCE (nResID)), m_lpTemplate(NULL)
    {
        // Initialize the common controls.
        LoadCommonControls();
    }

    //For indirect dialogs - created from a dialog box template in memory.
    inline CDialog::CDialog(LPCDLGTEMPLATE lpTemplate) : m_IsModal(TRUE),
                        m_lpszResName(NULL), m_lpTemplate(lpTemplate)
    {
        // Initialize the common controls.
        LoadCommonControls();
    }

    inline CDialog::~CDialog()
    {
        if (GetHwnd() != 0)
        {
            if (IsModal())
                ::EndDialog(GetHwnd(), 0);
            else
                Destroy();
        }
    }


    // Attaches a dialog item to a CWnd
    inline void CDialog::AttachItem(int nID, CWnd& Wnd)
    {
        Wnd.AttachDlgItem(nID, *this);
    }


    // Override this function in your class derived from CDialog if you wish to handle messages.
    inline INT_PTR CDialog::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
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

        // Always pass unhandled messages on to DialogProcDefault
        return DialogProcDefault(uMsg, wParam, lParam);
    }


    // All DialogProc functions should pass unhandled messages to this function.
    inline INT_PTR CDialog::DialogProcDefault(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        LRESULT lr = 0;

        switch (uMsg)
        {
        case WM_INITDIALOG:
            {
                // Center the dialog
                CenterWindow();
            }
            return OnInitDialog();
        case WM_CLOSE:
            {
                OnClose();
                return 0L;
            }
        case WM_COMMAND:
            {
                if (HIWORD(wParam) == BN_CLICKED)
                {
                    switch (LOWORD(wParam))
                    {
                    case IDOK:
                        OnOK();
                        return TRUE;
                    case IDCANCEL:
                        OnCancel();
                        return TRUE;
                    }
                }

                // Reflect this message if it's from a control
                CWnd* pWnd = GetCWndPtr(reinterpret_cast<HWND>(lParam));
                if (pWnd != NULL)
                    lr = pWnd->OnCommand(wParam, lParam);

                // Handle user commands
                if (!lr)
                    lr =  OnCommand(wParam, lParam);

                if (lr) return 0L;
            }
            break;  // Some commands require default processing


        case WM_DESTROY:
            OnDestroy();
            break;
        case WM_NOTIFY:
            {
                // Do notification reflection if message came from a child window.
                // Restricting OnNotifyReflect to child windows avoids double handling.
                LPNMHDR pNmhdr = reinterpret_cast<LPNMHDR>(lParam);
                assert(pNmhdr);
                HWND hwndFrom = pNmhdr->hwndFrom;
                CWnd* pWndFrom = GetApp().GetCWndFromMap(hwndFrom);

                if (pWndFrom != NULL)
                    if (::GetParent(hwndFrom) == m_hWnd)
                        lr = pWndFrom->OnNotifyReflect(wParam, lParam);

                // Handle user notifications
                if (!lr) lr = OnNotify(wParam, lParam);

                // Set the return code for notifications
                if (IsWindow())
                    SetWindowLongPtr(DWLP_MSGRESULT, static_cast<LONG_PTR>(lr));

                return lr;
            }

        case WM_PAINT:
            {
                if (::GetUpdateRect(*this, NULL, FALSE))
                {
                    CPaintDC dc(*this);
                    OnDraw(dc);
                }
                else
                // RedrawWindow can require repainting without an update rect
                {
                    CClientDC dc(*this);
                    OnDraw(dc);
                }

                break;
            }

        case WM_ERASEBKGND:
            {
                CDC dc(reinterpret_cast<HDC>(wParam));
                BOOL PreventErasure;

                PreventErasure = OnEraseBkgnd(dc);
                if (PreventErasure) return TRUE;
            }
            break;

        // A set of messages to be reflected back to the control that generated them
        case WM_CTLCOLORBTN:
        case WM_CTLCOLOREDIT:
        case WM_CTLCOLORDLG:
        case WM_CTLCOLORLISTBOX:
        case WM_CTLCOLORSCROLLBAR:
        case WM_CTLCOLORSTATIC:
        case WM_DRAWITEM:
        case WM_MEASUREITEM:
        case WM_DELETEITEM:
        case WM_COMPAREITEM:
        case WM_CHARTOITEM:
        case WM_VKEYTOITEM:
        case WM_HSCROLL:
        case WM_VSCROLL:
        case WM_PARENTNOTIFY:
            return MessageReflect(uMsg, wParam, lParam);

        case UWM_GETCDIALOG:    // Returns a pointer to this CDialog object
        case UWM_GETCWND:
        {
            assert(this == GetCWndPtr(m_hWnd));

            // Set the return code
            if (IsWindow())
                SetWindowLongPtr(DWLP_MSGRESULT, reinterpret_cast<LONG_PTR>(this));

            return TRUE;
        }

        } // switch(uMsg)

        return 0;

    } // INT_PTR CALLBACK CDialog::DialogProc(...)


    // Creates a modal dialog. A modal dialog box must be closed by the user
    // before the application continues.
    inline INT_PTR CDialog::DoModal(HWND hWndParent /* = 0 */)
    {
        assert( &GetApp() );        // Test if Win32++ has been started
        assert(!IsWindow());        // Only one window per CWnd instance allowed
        assert(m_lpTemplate || m_lpszResName);  // Dialog layout must be defined.

        INT_PTR nResult = 0;
        m_IsModal=TRUE;
        m_hWnd = 0;

        // Ensure this thread has the TLS index set
        TLSData* pTLSData = GetApp().SetTlsData();

    #ifndef _WIN32_WCE
        if (NULL == pTLSData->hMsgHook )
        {
            pTLSData->hMsgHook = ::SetWindowsHookEx(WH_MSGFILTER, (HOOKPROC)StaticMsgHook, NULL, ::GetCurrentThreadId());
        }
        InterlockedIncrement(&pTLSData->nDlgHooks);
    #endif

        HINSTANCE hInstance = GetApp().GetInstanceHandle();
        pTLSData->pWnd = this;

        // Create a modal dialog
        if (IsIndirect())
            nResult = ::DialogBoxIndirect(hInstance, m_lpTemplate, hWndParent, (DLGPROC)CDialog::StaticDialogProc);
        else
        {
            if (::FindResource(GetApp().GetResourceHandle(), m_lpszResName, RT_DIALOG))
                hInstance = GetApp().GetResourceHandle();
            nResult = ::DialogBox(hInstance, m_lpszResName, hWndParent, (DLGPROC)CDialog::StaticDialogProc);
        }

        // Tidy up
        pTLSData->pWnd = NULL;
        m_hWnd = 0;

    #ifndef _WIN32_WCE
        InterlockedDecrement(&pTLSData->nDlgHooks);
        if (pTLSData->nDlgHooks == 0)
        {
            ::UnhookWindowsHookEx(pTLSData->hMsgHook);
            pTLSData->hMsgHook = NULL;
        }

    #endif

        // Throw an exception if the dialog creation fails
        if (nResult == -1)
        {
            throw CWinException(_T("Dialog creation failed"));
        }

        return nResult;
    }


    // Creates a modeless dialog.
    inline HWND CDialog::DoModeless(HWND hParent /* = 0 */)
    {
        assert( &GetApp() );        // Test if Win32++ has been started
        assert(!IsWindow());        // Only one window per CWnd instance allowed
        assert(m_lpTemplate || m_lpszResName);  // Dialog layout must be defined.

        m_IsModal=FALSE;
        m_hWnd = 0;

        // Ensure this thread has the TLS index set
        TLSData* pTLSData = GetApp().SetTlsData();

        // Store the CWnd pointer in Thread Local Storage
        pTLSData->pWnd = this;

        HINSTANCE hInstance = GetApp().GetInstanceHandle();
        HWND hWnd;

        // Create the modeless dialog
        if (IsIndirect())
            hWnd = ::CreateDialogIndirect(hInstance, m_lpTemplate, hParent, (DLGPROC)CDialog::StaticDialogProc);
        else
        {
            if (::FindResource(GetApp().GetResourceHandle(), m_lpszResName, RT_DIALOG))
                hInstance = GetApp().GetResourceHandle();

            hWnd = ::CreateDialog(hInstance, m_lpszResName, hParent, (DLGPROC)CDialog::StaticDialogProc);
        }

        // Tidy up
        pTLSData->pWnd = NULL;

        // Display information on dialog creation failure
        if (hWnd == 0)
        {
            throw CWinException(_T("Dialog creation failed"));
        }

        return hWnd;
    }


    // Ends a modal or modeless dialog.
    inline void CDialog::EndDialog(INT_PTR nResult)
    {
        assert(IsWindow());

        if (IsModal())
            ::EndDialog(*this, nResult);
        else
            Destroy();
    }


    // Called when the Cancel button is pressed. Override to customize OnCancel behaviour.
    inline void CDialog::OnCancel()
    {
        EndDialog(IDCANCEL);
    }


    // Called when the Close button is pressed.
    inline void CDialog::OnClose()
    {
        EndDialog(0);
    }


    // Called when the dialog is initialized.
    // Override it in your derived class to automatically perform tasks.
    // The return value is used by WM_INITDIALOG.
    inline BOOL CDialog::OnInitDialog()
    {
        return TRUE;
    }


    // Called when the OK button is pressed. Override to customize OnOK behaviour.
    inline void CDialog::OnOK()
    {
        if ( IsWindow() )
            EndDialog(IDOK);
    }


    // Override this function to filter mouse and keyboard messages prior to
    // being passed to the DialogProc.
    inline BOOL CDialog::PreTranslateMessage(MSG& Msg)
    {
        // allow the dialog to translate keyboard input
        if ((Msg.message >= WM_KEYFIRST) && (Msg.message <= WM_KEYLAST))
        {
            // Process dialog keystrokes for modeless dialogs
            if (!IsModal())
            {
                TLSData* pTLSData = GetApp().GetTlsData();
                if (NULL == pTLSData->hMsgHook)
                {
                    if (IsDialogMessage(Msg))
                        return TRUE;
                }
                else
                {
                    // A modal message loop is running which performs IsDialogMessage
                    // for us.
                }
            }
        }

        return FALSE;
    }


    // Retrieves the identifier of the default push button control for the dialog.
    inline DWORD CDialog::GetDefID() const
    {
        assert(IsWindow());
        DWORD dwID = 0;
        LRESULT LR = SendMessage(DM_GETDEFID, 0L, 0L);
        if (DC_HASDEFID == HIWORD(LR))
            dwID = LOWORD(LR);

        return dwID;
    }


    // Sets the keyboard focus to the specified control.
    inline void CDialog::GotoDlgCtrl(HWND hWndCtrl)
    {
        assert(IsWindow());
        assert(::IsWindow(hWndCtrl));
        SendMessage(WM_NEXTDLGCTL, reinterpret_cast<WPARAM>(hWndCtrl), TRUE);
    }


    // Converts the dialog box units to screen units (pixels).
    inline BOOL CDialog::MapDialogRect(RECT& rc) const
    {
        assert(IsWindow());
        return ::MapDialogRect(*this, &rc);
    }


    // Sets the keyboard focus to the next dialog control.
    inline void CDialog::NextDlgCtrl() const
    {
        assert(IsWindow());
        SendMessage(WM_NEXTDLGCTL, FALSE, FALSE);
    }


    // Sets the keyboard focus to the previous dialog control.
    inline void CDialog::PrevDlgCtrl() const
    {
        assert(IsWindow());
        SendMessage(WM_NEXTDLGCTL, TRUE, FALSE);
    }


    // Changes the identifier of the default push button for a dialog box.
    inline void CDialog::SetDefID(UINT nID)
    {
        assert(IsWindow());
        SendMessage(DM_SETDEFID, nID, 0L);
    }


    // This callback function passes messages to DialogProc
    inline INT_PTR CALLBACK CDialog::StaticDialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        // Find the CWnd pointer mapped to this HWND
        CDialog* pDialog = static_cast<CDialog*>(GetCWndPtr(hWnd));
        if (pDialog == 0)
        {
            // The HWND wasn't in the map, so add it now
            TLSData* pTLSData = GetApp().GetTlsData();
            assert(pTLSData);

            // Retrieve pointer to CWnd object from Thread Local Storage TLS
            pDialog = static_cast<CDialog*>(pTLSData->pWnd);
            assert(pDialog);
            pTLSData->pWnd = NULL;

            // Store the Window pointer into the HWND map
            pDialog->m_hWnd = hWnd;
            pDialog->AddToMap();
        }

        return pDialog->DialogProc(uMsg, wParam, lParam);

    } // INT_PTR CALLBACK CDialog::StaticDialogProc(...)


#ifndef _WIN32_WCE

    // Used by Modal Dialogs for idle processing and PreTranslateMessage.
    inline LRESULT CALLBACK CDialog::StaticMsgHook(int nCode, WPARAM wParam, LPARAM lParam)
    {
        TLSData* pTLSData = GetApp().GetTlsData();
        MSG Msg;
        ZeroMemory(&Msg, sizeof(Msg));
        LONG lCount = 0;

        // While idle, perform idle processing until OnIdle returns FALSE
        // Exclude some messages to avoid calling OnIdle excessively
        while (!::PeekMessage(&Msg, 0, 0, 0, PM_NOREMOVE) &&
                            (Msg.message != WM_TIMER) &&
                            (Msg.message != WM_MOUSEMOVE) &&
                            (Msg.message != WM_SETCURSOR) &&
                                GetApp().OnIdle(lCount) != FALSE )
        {
            ++lCount;
        }
        lCount = 0;

        if (nCode == MSGF_DIALOGBOX)
        {
            MSG* lpMsg = reinterpret_cast<MSG*>(lParam);
            assert(lpMsg);

            // only pre-translate keyboard and mouse events
            if ((lpMsg->message >= WM_KEYFIRST && lpMsg->message <= WM_KEYLAST) ||
                (lpMsg->message >= WM_MOUSEFIRST && lpMsg->message <= WM_MOUSELAST))
            {
                for (HWND hWnd = lpMsg->hwnd; hWnd != NULL; hWnd = ::GetParent(hWnd))
                {
                    // Only CDialogs respond to this message
                    CDialog* pDialog = reinterpret_cast<CDialog*>(::SendMessage(hWnd, UWM_GETCDIALOG, 0, 0));
                    if (pDialog != 0)
                    {
                        assert(GetCWndPtr(hWnd));
                        if (pDialog->PreTranslateMessage(*lpMsg))
                            return 1; // Eat the message

                        break;  // Pass the message on
                    }
                }
            }
        }

        return ::CallNextHookEx(pTLSData->hMsgHook, nCode, wParam, lParam);
    }
#endif



#ifndef _WIN32_WCE

    /////////////////////////////////////
    // Definitions for the CResizer class
    //

    // Adds or updates the information about a child window managed by CResizer.
    // The alignment corner should be set to the closest corner of the dialog.
    // Allowed values are topleft, topright, bottomleft, and bottomright.
    // Set IsFixedWidth to TRUE if the width should be fixed instead of variable.
    // Set IsFixedHeight to TRUE if the height should be fixed instead of variable.
    void inline CResizer::AddChild(HWND hWnd, Alignment corner, DWORD dwStyle)
    {
        assert(::IsWindow(hWnd));
        assert(::GetParent(hWnd) == m_hParent);

        ResizeData rd;
        rd.corner = corner;
        rd.IsFixedWidth  = !(dwStyle & RD_STRETCH_WIDTH);
        rd.IsFixedHeight = !(dwStyle & RD_STRETCH_HEIGHT);
        CRect rcInit;
        ::GetWindowRect(hWnd, &rcInit);
        ::MapWindowPoints(NULL, m_hParent, (LPPOINT)&rcInit, 2);
        rd.rcInit = rcInit;
        rd.hWnd = hWnd;

        std::vector<ResizeData>::iterator iter;
        for (iter = m_vResizeData.begin(); iter != m_vResizeData.end(); ++ iter)
        {
            if ( iter->hWnd == hWnd)
            {
                // Replace the value
                *iter = rd;
                break;
            }
        }

        // Add the value
        if (iter == m_vResizeData.end())
            m_vResizeData.push_back(rd);
    }


    // A callback function used by EnumChildWindows.
    inline BOOL CALLBACK CResizer::EnumWindowsProc(HWND hwnd, LPARAM lParam)
    {
        CResizer* pResizer = reinterpret_cast<CResizer*>(lParam);

        // Only for a child, not other descendants.
        if (::GetParent(hwnd) == pResizer->m_hParent)
        {
            // Add the child window to set of windows managed by CResizer.
            pResizer->AddChild(hwnd, topleft, 0);
        }

        return TRUE;
    }


    // Performs the resizing and scrolling. Call this function from within the window's DialogProc.
    inline void CResizer::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        switch (uMsg)
        {
        case WM_SIZE:
            RecalcLayout();
            break;

        case WM_HSCROLL:
            if (lParam == 0)
                OnHScroll(uMsg, wParam, lParam);
            break;

        case WM_VSCROLL:
            if (lParam == 0)
                OnVScroll(uMsg, wParam, lParam);
            break;
        }
    }


    // Sets up the Resizer by specifying the parent window (usually a dialog),
    // and the minimum and maximum allowed rectangle sizes.
    // Note:hParent can either be a CWnd or a window handle (HWND)
    void inline CResizer::Initialize(HWND hParent, const RECT& rcMin, const RECT& rcMax)
    {
        assert (::IsWindow(hParent));

        m_hParent = hParent;
        VERIFY(::GetClientRect(hParent, &m_rcInit) != 0);

        m_rcMin = rcMin;
        m_rcMax = rcMax;

        m_vResizeData.clear();

        // Add scroll bar support to the parent window
        DWORD dwStyle = static_cast<DWORD>(::GetClassLongPtr(hParent, GCL_STYLE));
        dwStyle |= WS_HSCROLL | WS_VSCROLL;
        ::SetClassLongPtr(hParent, GCL_STYLE, dwStyle);

        // Calls AddChild for each child window with default settings.
        ::EnumChildWindows(hParent, EnumWindowsProc, reinterpret_cast<LPARAM>(this));
    }


    // Called to perform horizontal scrolling.
    void inline CResizer::OnHScroll(UINT, WPARAM wParam, LPARAM)
    {
        int xNewPos;

        switch (LOWORD(wParam))
        {
            case SB_PAGEUP: // User clicked the scroll bar shaft left of the scroll box.
                xNewPos = m_xScrollPos - 50;
                break;

            case SB_PAGEDOWN: // User clicked the scroll bar shaft right of the scroll box.
                xNewPos = m_xScrollPos + 50;
                break;

            case SB_LINEUP: // User clicked the left arrow.
                xNewPos = m_xScrollPos - 5;
                break;

            case SB_LINEDOWN: // User clicked the right arrow.
                xNewPos = m_xScrollPos + 5;
                break;

            case SB_THUMBPOSITION: // User dragged the scroll box.
                xNewPos = HIWORD(wParam);
                break;

            case SB_THUMBTRACK: // User dragging the scroll box.
                xNewPos = HIWORD(wParam);
                break;

            default:
                xNewPos = m_xScrollPos;
        }

        // Scroll the window.
        xNewPos = MAX(0, xNewPos);
        CRect rc;
        VERIFY(::GetClientRect(m_hParent, &rc) != 0);
        xNewPos = MIN( xNewPos, GetMinRect().Width() - rc.Width() );
        int xDelta = xNewPos - m_xScrollPos;
        m_xScrollPos = xNewPos;
        ::ScrollWindow(m_hParent, -xDelta, 0, NULL, NULL);

        // Reset the scroll bar.
        SCROLLINFO si;
        ZeroMemory(&si, sizeof(si));
        si.cbSize = sizeof(si);
        si.fMask  = SIF_POS;
        si.nPos   = m_xScrollPos;
        ::SetScrollInfo(m_hParent, SB_HORZ, &si, TRUE);
    }


    // Called to perform vertical scrolling.
    void inline CResizer::OnVScroll(UINT, WPARAM wParam, LPARAM)
    {
        int yNewPos;

        switch (LOWORD(wParam))
        {
            case SB_PAGEUP: // User clicked the scroll bar shaft above the scroll box.
                yNewPos = m_yScrollPos - 50;
                break;

            case SB_PAGEDOWN: // User clicked the scroll bar shaft below the scroll box.
                yNewPos = m_yScrollPos + 50;
                break;

            case SB_LINEUP: // User clicked the top arrow.
                yNewPos = m_yScrollPos - 5;
                break;

            case SB_LINEDOWN: // User clicked the bottom arrow.
                yNewPos = m_yScrollPos + 5;
                break;

            case SB_THUMBPOSITION: // User dragged the scroll box.
                yNewPos = HIWORD(wParam);
                break;

            case SB_THUMBTRACK: // User dragging the scroll box.
                yNewPos = HIWORD(wParam);
                break;

            default:
                yNewPos = m_yScrollPos;
        }

        // Scroll the window.
        yNewPos = MAX(0, yNewPos);
        CRect rc;
        VERIFY(::GetClientRect(m_hParent, &rc) != 0);
        yNewPos = MIN( yNewPos, GetMinRect().Height() - rc.Height() );
        int yDelta = yNewPos - m_yScrollPos;
        m_yScrollPos = yNewPos;
        ::ScrollWindow(m_hParent, 0, -yDelta, NULL, NULL);

        // Reset the scroll bar.
        SCROLLINFO si;
        ZeroMemory(&si, sizeof(si));
        si.cbSize = sizeof(si);
        si.fMask  = SIF_POS;
        si.nPos   = m_yScrollPos;
        ::SetScrollInfo(m_hParent, SB_VERT, &si, TRUE);
    }


    // Repositions the child windows. Call this function when handling
    // the WM_SIZE message in the parent window.
    void inline CResizer::RecalcLayout()
    {
        assert (m_rcInit.Width() > 0 && m_rcInit.Height() > 0);
        assert (::IsWindow(m_hParent));

        CRect rcCurrent;
        VERIFY(::GetClientRect(m_hParent, &rcCurrent) != 0);

        // Adjust the scrolling if required
        m_xScrollPos = MIN(m_xScrollPos, MAX(0, m_rcMin.Width()  - rcCurrent.Width() ) );
        m_yScrollPos = MIN(m_yScrollPos, MAX(0, m_rcMin.Height() - rcCurrent.Height()) );
        SCROLLINFO si;
        ZeroMemory(&si, sizeof(si));
        si.cbSize = sizeof(si);
        si.fMask  = SIF_RANGE | SIF_PAGE | SIF_POS;
        si.nMax   = m_rcMin.Width();
        si.nPage  = rcCurrent.Width();
        si.nPos   = m_xScrollPos;
        ::SetScrollInfo(m_hParent, SB_HORZ, &si, TRUE);
        si.nMax   = m_rcMin.Height();
        si.nPage  = rcCurrent.Height();
        si.nPos   = m_yScrollPos;
        ::SetScrollInfo(m_hParent, SB_VERT, &si, TRUE);

        // Note: calls to SetScrollInfo may have changed the client rect, so
        // we get it again.
        VERIFY(::GetClientRect(m_hParent, &rcCurrent) != 0);

        rcCurrent.right  = MAX( rcCurrent.Width(),  m_rcMin.Width() );
        rcCurrent.bottom = MAX( rcCurrent.Height(), m_rcMin.Height() );
        if (!m_rcMax.IsRectEmpty())
        {
            rcCurrent.right  = MIN( rcCurrent.Width(),  m_rcMax.Width() );
            rcCurrent.bottom = MIN( rcCurrent.Height(), m_rcMax.Height() );
        }

        // Declare an iterator to step through the vector
        std::vector<ResizeData>::iterator iter;

        // Allocates memory for a multiple-window- position structure.
        HDWP hdwp = ::BeginDeferWindowPos(static_cast<int>(m_vResizeData.size()));

        for (iter = m_vResizeData.begin(); iter != m_vResizeData.end(); ++iter)
        {
            int left   = 0;
            int top    = 0;
            int width  = 0;
            int height = 0;

            // Calculate the new size and position of the child window
            switch( (*iter).corner )
            {
            case topleft:
                width  = (*iter).IsFixedWidth?  (*iter).rcInit.Width()  : (*iter).rcInit.Width()  - m_rcInit.Width() + rcCurrent.Width();
                height = (*iter).IsFixedHeight? (*iter).rcInit.Height() : (*iter).rcInit.Height() - m_rcInit.Height() + rcCurrent.Height();
                left   = (*iter).rcInit.left;
                top    = (*iter).rcInit.top;
                break;
            case topright:
                width  = (*iter).IsFixedWidth?  (*iter).rcInit.Width()  : (*iter).rcInit.Width()  - m_rcInit.Width() + rcCurrent.Width();
                height = (*iter).IsFixedHeight? (*iter).rcInit.Height() : (*iter).rcInit.Height() - m_rcInit.Height() + rcCurrent.Height();
                left   = (*iter).rcInit.right - width - m_rcInit.Width() + rcCurrent.Width();
                top    = (*iter).rcInit.top;
                break;
            case bottomleft:
                width  = (*iter).IsFixedWidth?  (*iter).rcInit.Width()  : (*iter).rcInit.Width()  - m_rcInit.Width() + rcCurrent.Width();
                height = (*iter).IsFixedHeight? (*iter).rcInit.Height() : (*iter).rcInit.Height() - m_rcInit.Height() + rcCurrent.Height();
                left   = (*iter).rcInit.left;
                top    = (*iter).rcInit.bottom - height - m_rcInit.Height() + rcCurrent.Height();
                break;
            case bottomright:
                width  = (*iter).IsFixedWidth?  (*iter).rcInit.Width()  : (*iter).rcInit.Width()  - m_rcInit.Width() + rcCurrent.Width();
                height = (*iter).IsFixedHeight? (*iter).rcInit.Height() : (*iter).rcInit.Height() - m_rcInit.Height() + rcCurrent.Height();
                left   = (*iter).rcInit.right   - width - m_rcInit.Width() + rcCurrent.Width();
                top    = (*iter).rcInit.bottom  - height - m_rcInit.Height() + rcCurrent.Height();
                break;
            }

            // Determine the position of the child window.
            CRect rc(left - m_xScrollPos, top - m_yScrollPos, left + width - m_xScrollPos, top + height - m_yScrollPos);
            if ( rc != (*iter).rcOld)
            {
                // Note: The tab order of the dialog's controls is determined by the order
                //       they are specified in the resource script (resource.rc).

                // Store the window's new position. Repositioning happens later.
                hdwp = ::DeferWindowPos(hdwp, (*iter).hWnd, NULL, rc.left, rc.top, rc.Width(), rc.Height(), SWP_NOZORDER|SWP_NOCOPYBITS);

                (*iter).rcOld = rc;
            }

        }

        // Reposition all the child windows simultaneously.
        VERIFY(::EndDeferWindowPos(hdwp) != 0);
    }

#endif // #ifndef _WIN32_WCE

} // namespace Win32xx



#endif // _WIN32XX_DIALOG_H_

