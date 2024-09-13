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
// wxx_dialog.h
//  Declaration of CResizer and CDialog classes.

// CDialog adds support for dialogs to Win32++. Dialogs are specialised
// windows which are a parent window for common controls. Common controls
// are special window types such as buttons, edit controls, tree views,
// list views, static text etc.

// The layout of a dialog is typically defined in a resource script file
// (often Resource.rc). While this script file can be constructed manually,
// it is often created using a resource editor.  A resource editor is included
// with Microsoft's Visual Studio Studio Community.

// CDialog supports modal and modeless dialogs. It also supports the creation
// of dialogs defined in a resource script file, as well as those defined in
// a dialog template.

// Use the Dialog generic program as the starting point for your own dialog
// applications.
// The DialogDemo sample demonstrates how to use subclassing to customize
// the behaviour of common controls in a dialog.

// Note: The following functions often used with dialogs are provided by CWnd:
//  GetDlgCtrlID, GetDlgItem, GetDlgItemInt, GetDlgItemText, GetNextDlgGroupItem,
//  GetNextDlgTabItem, SendDlgItemMessage, SetDlgItemInt, SetDlgItemText


#ifndef _WIN32XX_DIALOG_H_
#define _WIN32XX_DIALOG_H_

#include "wxx_wincore.h"


namespace Win32xx
{
    ///////////////////////////////////////////////////////////
    // CDialog manages a dialog box. Dialog boxes are typically
    // used by an application to retrieve user input.
    // A modal dialog box requires the user to supply information or
    // cancel the dialog box before allowing the application to continue.
    // A modeless dialog box allows the user to supply information and
    // return to the previous task without closing the dialog box.
    class CDialog : public CWnd
    {

    public:
        CDialog();
        CDialog(UINT resourceID);
        CDialog(LPCTSTR resourceName);
        CDialog(LPCDLGTEMPLATE pDlgTemplate);
        virtual ~CDialog() override;

        // Virtual functions
        virtual void AttachItem(UINT id, CWnd& wnd);
        virtual HWND Create(HWND parent = nullptr)  override { return DoModeless(parent); }
        virtual INT_PTR DoModal(HWND parent = nullptr);
        virtual HWND DoModeless(HWND parent = nullptr);

        // Mutators that assign the dialog resource
        void SetDialogFromID(UINT resourceID);
        void SetDialogResource(LPCTSTR resourceName);
        void SetDialogTemplate(LPCDLGTEMPLATE pDlgTemplate);

        // Wrappers for Windows API functions
        UINT GetDefID() const;
        void GotoDlgCtrl(HWND control) const;
        BOOL MapDialogRect(RECT& rc) const;
        void NextDlgCtrl() const;
        void PrevDlgCtrl() const;
        void SetDefID(UINT id) const;

    protected:
        // Virtual functions you might wish to override
        virtual INT_PTR DialogProc(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual void EndDialog(INT_PTR result);
        virtual void OnCancel();
        virtual void OnClose() override;
        virtual BOOL OnInitDialog();
        virtual void OnOK();
        virtual BOOL PreTranslateMessage(MSG& msg) override;

        // Not intended to be overridden
        virtual INT_PTR DialogProcDefault(UINT msg, WPARAM wparam, LPARAM lparam);

    private:
        using CWnd::WndProc;                  // Make WndProc private
        using CWnd::WndProcDefault;           // Make WndProcDefault private
        CDialog(const CDialog&) = delete;
        CDialog& operator=(const CDialog&) = delete;

        static INT_PTR CALLBACK StaticDialogProc(HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam);
        static LRESULT CALLBACK StaticMsgHook(int code, WPARAM wparam, LPARAM lparam);

        // State functions
        BOOL IsModal() const { return m_isModal; }
        BOOL IsIndirect() const { return (m_pDlgTemplate != nullptr); }

        BOOL m_isModal;                  // a flag for modal dialogs
        HHOOK m_msgHook;                 // the message hook handle
        LPCTSTR m_resourceName;          // the resource name for the dialog
        LPCDLGTEMPLATE m_pDlgTemplate;   // the dialog template for indirect dialogs
    };


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
    const int RD_STRETCH_HEIGHT = 0x0001;  // The item has a variable height.
    const int RD_STRETCH_WIDTH  = 0x0002;  // The item has a variable width.


    ////////////////////////////////////////////////////////////////
    // The CResizer class is used to automatically rearrange child
    // windows when the parent window is resized. It displays scroll
    // bars as required. CResizer is often used with dialogs.
    class CResizer
    {
    public:

        // Alignment specifiers.
        enum Alignment { topleft, topright, bottomleft, bottomright, center, leftcenter, rightcenter, topcenter, bottomcenter };

        CResizer() : m_parent(nullptr), m_xScrollPos(0), m_yScrollPos(0),
                     m_currentDpi(USER_DEFAULT_SCREEN_DPI),
                     m_initDpi(USER_DEFAULT_SCREEN_DPI),
                     m_isDpiChanging(false)
                     {}
        virtual ~CResizer() {}

        virtual void AddChild(HWND wnd, Alignment corner, DWORD style);
        virtual void HandleMessage(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual void Initialize(HWND parent, const RECT& minRect,
                                const RECT& maxRect = CRect(0, 0, 0, 0));
        virtual void OnAfterDpiChange();
        virtual void OnBeforeDpiChange();
        virtual void OnHScroll(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual void OnVScroll(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual void RecalcLayout();
        CRect GetMinRect() const { return m_minRect; }
        CRect GetMaxRect() const { return m_maxRect; }
        void  SetMinRect(const RECT& minRect) { m_minRect = minRect; }
        void  SetMaxRect(const RECT& rcMaxRect) { m_maxRect = rcMaxRect; }

        struct ResizeData
        {
            ResizeData() : corner(topleft), isFixedWidth(false), isFixedHeight(false), wnd(0) {}
            CRect childRect;
            Alignment corner;
            bool isFixedWidth;
            bool isFixedHeight;
            HWND wnd;
        };

    private:
        CResizer(const CResizer&) = delete;
        CResizer& operator=(const CResizer&) = delete;

        static BOOL CALLBACK EnumWindowsProc(HWND wnd, LPARAM lparam);
        void ScaleRect(CRect& rc, double scale);

        HWND m_parent;
        std::vector<ResizeData> m_resizeData;

        CRect m_initRect;
        CRect m_minRect;
        CRect m_maxRect;

        int m_xScrollPos;
        int m_yScrollPos;
        int m_currentDpi;
        int m_initDpi;

        bool m_isDpiChanging;
    };

}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


namespace Win32xx
{
    ////////////////////////////////////
    // Definitions for the CDialog class
    //


    // Default constructor. Use SetDialogTemplate, SetDialogResource or
    // SetDialogResourceFromID to specify the dialog if this constructor
    // is used.
    inline CDialog::CDialog() : m_isModal(FALSE), m_msgHook(nullptr),
        m_resourceName(nullptr), m_pDlgTemplate(nullptr)
    {
    }

    // Constructor that specifies the dialog's resource
    inline CDialog::CDialog(LPCTSTR resourceName) : m_isModal(FALSE),
        m_msgHook(nullptr), m_resourceName(resourceName), m_pDlgTemplate(nullptr)
    {
    }

    // Constructor that specifies the dialog's resource ID
    inline CDialog::CDialog(UINT resourceID) : m_isModal(FALSE), m_msgHook(nullptr),
        m_resourceName(MAKEINTRESOURCE (resourceID)), m_pDlgTemplate(nullptr)
    {
    }

    // Constructor for indirect dialogs, created from a dialog box template in memory.
    inline CDialog::CDialog(LPCDLGTEMPLATE pDlgTemplate) : m_isModal(FALSE),
        m_msgHook(nullptr), m_resourceName(nullptr), m_pDlgTemplate(pDlgTemplate)
    {
    }

    inline CDialog::~CDialog()
    {
        if (GetHwnd() != nullptr)
        {
            if (IsModal())
                ::EndDialog(GetHwnd(), 0);
            else
            {
                CWinApp* pApp = CWinApp::SetnGetThis();
                if (pApp != nullptr)          // Is the CWinApp object still valid?
                {
                    if (GetCWndPtr(*this) == this)  // Is window managed by Win32++?
                    {
                        if (IsWindow())
                            ::DestroyWindow(*this);
                    }

                    RemoveFromMap();
                }
            }
        }
    }

    // Attaches a dialog item to a CWnd.
    inline void CDialog::AttachItem(UINT id, CWnd& wnd)
    {
        wnd.AttachDlgItem(id, *this);
    }

    // The dialog's message procedure. Override this function in your class derived
    // from CDialog if you wish to handle messages.
    inline INT_PTR CDialog::DialogProc(UINT msg, WPARAM wparam, LPARAM lparam)
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

        // Always pass unhandled messages on to DialogProcDefault.
        return DialogProcDefault(msg, wparam, lparam);
    }

    // All DialogProc functions should pass unhandled messages to this function.
    inline INT_PTR CDialog::DialogProcDefault(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        LRESULT result = 0;

        switch (msg)
        {
        case WM_INITDIALOG:
            {
                // Center the dialog.
                CenterWindow();
            }
            return OnInitDialog();
        case WM_CLOSE:
            {
                OnClose();
                return 0;
            }
        case WM_COMMAND:
            {
                if (HIWORD(wparam) == BN_CLICKED)
                {
                    switch (LOWORD(wparam))
                    {
                    case IDOK:
                        OnOK();
                        return TRUE;
                    case IDCANCEL:
                        OnCancel();
                        return TRUE;
                    }
                }

                // Reflect this message if it's from a control.
                CWnd* pWnd = GetCWndPtr(reinterpret_cast<HWND>(lparam));
                if (pWnd != nullptr)
                    result = pWnd->OnCommand(wparam, lparam);

                // Handle user commands.
                if (!result)
                    result =  OnCommand(wparam, lparam);

                if (result) return 0;
            }
            break;  // Some commands require default processing.


        case WM_DESTROY:
            OnDestroy();
            break;
        case WM_NOTIFY:
            {
                // Do notification reflection if message came from a child window.
                // Restricting OnNotifyReflect to child windows avoids double handling.
                LPNMHDR pHeader = reinterpret_cast<LPNMHDR>(lparam);
                assert(pHeader);
                if (pHeader != nullptr)
                {
                    HWND from = pHeader->hwndFrom;
                    CWnd* pFrom = GetApp()->GetCWndFromMap(from);

                    if (pFrom != nullptr)
                        if (::GetParent(from) == m_wnd)
                            result = pFrom->OnNotifyReflect(wparam, lparam);

                    // Handle user notifications.
                    if (!result) result = OnNotify(wparam, lparam);

                    // Set the return code for notifications.
                    if (IsWindow())
                        SetWindowLongPtr(DWLP_MSGRESULT, static_cast<LONG_PTR>(result));
                }
                return result;
            }

        case WM_PAINT:
            {
                if (::GetUpdateRect(*this, nullptr, FALSE))
                {
                    CPaintDC dc(*this);
                    OnDraw(dc);
                }
                else
                // RedrawWindow can require repainting without an update rect.
                {
                    CClientDC dc(*this);
                    OnDraw(dc);
                }

                break;
            }

        case WM_ERASEBKGND:
            {
                CDC dc(reinterpret_cast<HDC>(wparam));
                return OnEraseBkgnd(dc);
            }

        // A set of messages to be reflected back to the control that generated them.
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
            return MessageReflect(msg, wparam, lparam);

        case UWM_GETCDIALOG:    // Returns a pointer to this CDialog object.
        case UWM_GETCWND:
        {
            assert(this == GetCWndPtr(m_wnd));

            // Set the return code.
            if (IsWindow())
                SetWindowLongPtr(DWLP_MSGRESULT, reinterpret_cast<LONG_PTR>(this));

            return TRUE;
        }

        } // switch(msg)

        return 0;

    } // INT_PTR CALLBACK CDialog::DialogProc(...)

    // Creates a modal dialog. A modal dialog box must be closed by the user
    // before the application continues.
    // Refer to DialogBox and DialogBoxIndirect in the Windows API documentation
    // for more information.
    inline INT_PTR CDialog::DoModal(HWND parent /* = nullptr */)
    {
        assert(!IsWindow());        // Only one window per CWnd instance allowed
        assert(m_pDlgTemplate || m_resourceName);  // Dialog layout must be defined.

        INT_PTR result = 0;
        m_isModal=TRUE;
        Cleanup();

        // Set the message hook used to support OnIdle and PreTranslateMessage.
        // The hook is only used in modal dialogs.
        m_msgHook = ::SetWindowsHookEx(WH_MSGFILTER, (HOOKPROC)StaticMsgHook,
                                       0, ::GetCurrentThreadId());

        // Store this CDialog's pointer in thread local storage.
        TLSData* pTLSData = GetApp()->GetTlsData();
        assert(pTLSData);
        pTLSData->pWnd = this;

        // Create a modal dialog.
        HINSTANCE instance = GetApp()->GetInstanceHandle();
        if (m_pDlgTemplate != nullptr)
            result = ::DialogBoxIndirect(instance, m_pDlgTemplate, parent, CDialog::StaticDialogProc);
        else
        {
            if (::FindResource(GetApp()->GetResourceHandle(), m_resourceName, RT_DIALOG))
                instance = GetApp()->GetResourceHandle();
            result = ::DialogBox(instance, m_resourceName, parent, CDialog::StaticDialogProc);
        }

        // Tidy up.
        pTLSData->pWnd = nullptr;
        Cleanup();
        ::UnhookWindowsHookEx(m_msgHook);
        m_msgHook = nullptr;

        // Throw an exception if the dialog creation fails.
        if (result == -1)
        {
            throw CWinException(GetApp()->MsgWndDialog());
        }

        return result;
    }

    // Creates a modeless dialog.
    // Refer to CreateDialog and CreateDialogIndirect in the Windows API documentation for more information.
    inline HWND CDialog::DoModeless(HWND parent /* = nullptr*/)
    {
        assert(m_pDlgTemplate || m_resourceName);  // Dialog layout must be defined.

        m_isModal=FALSE;
        Cleanup();

        // Retrieve this thread's TLS data.
        TLSData* pTLSData = GetApp()->GetTlsData();

        // Store the CWnd pointer in Thread Local Storage.
        pTLSData->pWnd = this;

        HINSTANCE instance = GetApp()->GetInstanceHandle();
        HWND wnd = nullptr;

        // Create the modeless dialog.
        if (m_pDlgTemplate != nullptr)
            wnd = ::CreateDialogIndirect(instance, m_pDlgTemplate, parent, CDialog::StaticDialogProc);
        else
        {
            if (::FindResource(GetApp()->GetResourceHandle(), m_resourceName, RT_DIALOG))
                instance = GetApp()->GetResourceHandle();

            wnd = ::CreateDialog(instance, m_resourceName, parent, CDialog::StaticDialogProc);
        }

        // Tidy up.
        pTLSData->pWnd = nullptr;

        // Display information on dialog creation failure.
        if (wnd == nullptr)
        {
            throw CWinException(GetApp()->MsgWndDialog());
        }

        return wnd;
    }

    // Ends a modal dialog.
    // Refer to EndDialog in the Windows API documentation for more information.
    inline void CDialog::EndDialog(INT_PTR result)
    {
        assert(IsWindow());
        assert(IsModal());

        ::EndDialog(*this, result);
    }

    // Called when the Cancel button is pressed. Automatically closes the dialog.
    // Override to customize OnCancel behavior.
    inline void CDialog::OnCancel()
    {
        if (IsModal())
            EndDialog(IDCANCEL);
        else
            Destroy();
    }

    // Called when the Close button is pressed. Automatically closes the dialog.
    // Override to customize OnClose behavior.
    inline void CDialog::OnClose()
    {
        if (IsModal())
            EndDialog(0);
        else
            Destroy();
    }

    // Called when the dialog is initialized.
    // Override it in your derived class to automatically perform tasks.
    // The return value is used by WM_INITDIALOG.
    inline BOOL CDialog::OnInitDialog()
    {
        return TRUE;
    }

    // Called when the OK button is pressed. Automatically closes the dialog.
    // Override to customize OnOK behavior.
    inline void CDialog::OnOK()
    {
        if (IsModal())
            EndDialog(IDOK);
        else
            Destroy();
    }

    // Override this function to filter mouse and keyboard messages prior to
    // being passed to the DialogProc.
    inline BOOL CDialog::PreTranslateMessage(MSG& msg)
    {
        // Allow the dialog to translate keyboard input.
        if ((msg.message >= WM_KEYFIRST) && (msg.message <= WM_KEYLAST))
        {
            // Process dialog keystrokes for modeless dialogs.
            if (!IsModal())
            {
                if (IsDialogMessage(msg))
                    return TRUE;
            }
        }

        return FALSE;
    }

    // Retrieves the identifier of the default push button control for the dialog.
    // Refer to DM_GETDEFID in the Windows API documentation for more information.
    inline UINT CDialog::GetDefID() const
    {
        assert(IsWindow());
        DWORD id = 0;
        LRESULT result = SendMessage(DM_GETDEFID, 0, 0);
        if (DC_HASDEFID == HIWORD(result))
            id = LOWORD(result);

        return static_cast<UINT>(id);
    }

    // Sets the keyboard focus to the specified control.
    // Refer to WM_NEXTDLGCTL in the Windows API documentation for more information.
    inline void CDialog::GotoDlgCtrl(HWND control) const
    {
        assert(IsWindow());
        assert(::IsWindow(control));
        WPARAM wparam = reinterpret_cast<WPARAM>(control);
        LPARAM lparam = static_cast<LPARAM>(TRUE);
        SendMessage(WM_NEXTDLGCTL, wparam, lparam);
    }

    // Converts the dialog box units to screen units (pixels).
    // Refer to MapDialogRect in the Windows API documentation for more information.
    inline BOOL CDialog::MapDialogRect(RECT& rc) const
    {
        assert(IsWindow());
        return ::MapDialogRect(*this, &rc);
    }

    // Sets the keyboard focus to the next dialog control.
    // Refer to WM_NEXTDLGCTL in the Windows API documentation for more information.
    inline void CDialog::NextDlgCtrl() const
    {
        assert(IsWindow());
        SendMessage(WM_NEXTDLGCTL, 0, 0);
    }

    // Sets the keyboard focus to the previous dialog control.
    // Refer to WM_NEXTDLGCTL in the Windows API documentation for more information.
    inline void CDialog::PrevDlgCtrl() const
    {
        assert(IsWindow());
        SendMessage(WM_NEXTDLGCTL, static_cast<WPARAM>(TRUE), 0);
    }

    // Changes the identifier of the default push button for a dialog box.
    // Refer to DM_SETDEFID in the Windows API documentation for more information.
    inline void CDialog::SetDefID(UINT id) const
    {
        assert(IsWindow());
        SendMessage(DM_SETDEFID, static_cast<WPARAM>(id), 0);
    }

    // Sets the dialog from the specified dialog resource ID.
    inline void CDialog::SetDialogFromID(UINT resourceID)
    {
        m_resourceName = MAKEINTRESOURCE(resourceID);
    }

    // Sets the dialog from the specified dialog resource.
    inline void CDialog::SetDialogResource(LPCTSTR resourceName)
    {
        m_resourceName = resourceName;
    }

    // Sets the dialog from the specified dialog template.
    inline void CDialog::SetDialogTemplate(LPCDLGTEMPLATE pDlgTemplate)
    {
        m_pDlgTemplate = pDlgTemplate;
    }

    // This callback function passes messages to DialogProc
    inline INT_PTR CALLBACK CDialog::StaticDialogProc(HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam)
    {
        // Find the CWnd pointer mapped to this HWND.
        CDialog* pDialog = static_cast<CDialog*>(GetCWndPtr(wnd));
        if (pDialog == nullptr)
        {
            // The HWND wasn't in the map, so add it now.
            TLSData* pTLSData = GetApp()->GetTlsData();
            if (pTLSData)
            {
                // Retrieve pointer to CWnd object from Thread Local Storage TLS.
                pDialog = static_cast<CDialog*>(pTLSData->pWnd);
                if (pDialog)
                {
                    pTLSData->pWnd = nullptr;

                    // Store the window pointer in the HWND map.
                    pDialog->m_wnd = wnd;
                    pDialog->AddToMap();
                }
            }
        }

        assert(pDialog != nullptr);
        if (pDialog == nullptr)
        {
            // Got a message for a window that's not in the map.
            return 0;
        }

        return pDialog->DialogProc(msg, wparam, lparam);

    } // INT_PTR CALLBACK CDialog::StaticDialogProc(...)

    // Used only by modal dialogs for idle processing and PreTranslateMessage.
    inline LRESULT CALLBACK CDialog::StaticMsgHook(int code, WPARAM wparam, LPARAM lparam)
    {
        MSG msg{};
        LONG count = 0;

        // While idle, perform idle processing until OnIdle returns FALSE.
        // Exclude some messages to avoid calling OnIdle excessively.
        while (!::PeekMessage(&msg, 0, 0, 0, PM_NOREMOVE) &&
                            (msg.message != WM_TIMER) &&
                            (msg.message != WM_MOUSEMOVE) &&
                            (msg.message != WM_SETCURSOR) &&
                                GetApp()->OnIdle(count) != FALSE )
        {
            ++count;
        }
        count = 0;

        if (code == MSGF_DIALOGBOX)
        {
            MSG* pMsg = reinterpret_cast<MSG*>(lparam);

            // Only pre-translate keyboard and mouse events.
            if (pMsg && ((pMsg->message >= WM_KEYFIRST && pMsg->message <= WM_KEYLAST) ||
                (pMsg->message >= WM_MOUSEFIRST && pMsg->message <= WM_MOUSELAST)))
            {
                for (HWND wnd = pMsg->hwnd; wnd != 0; wnd = ::GetParent(wnd))
                {
                    // The wnd will be a handle to the dialog or a child control.
                    CDialog* pDialog = dynamic_cast<CDialog*>(GetCWndPtr(wnd));
                    if (pDialog != nullptr)
                    {
                        if (pDialog->PreTranslateMessage(*pMsg))
                            return 1; // Eat the message.

                        // The HHOOK parameter is used in CallNextHookEx for Win95, Win98 and WinME.
                        // The HHOOK parameter is ignored for Windows NT and above.
                        return ::CallNextHookEx(pDialog->m_msgHook, code, wparam, lparam);
                    }
                }
            }
        }

        return ::CallNextHookEx(0, code, wparam, lparam);
    }


    /////////////////////////////////////
    // Definitions for the CResizer class
    //

    // Adds or updates the information about a child window managed by CResizer.
    //
    // The alignment parameter determines the control's position.
    // Allowed values are:
    //  topleft:      Positions top left
    //  topright:     Positions top right
    //  bottomleft:   Positions bottom left
    //  bottomright:  Positions bottom right
    //  center:       Positions proportionally
    //  leftcenter:   Positions proportionally along the left side
    //  rightcenter:  Positions proportionally along the right side
    //  topcenter:    Positions proportionally along the top side
    //  bottomcenter: Positions proportionally along the bottom side
    //
    // The style is a combination of the following flags. If the style is
    // zero, the control's width and height is fixed:
    //  RD_STRETCH_WIDTH:   The width is variable instead of fixed.
    //  RD_STRETCH_HEIGHT:  The height is variable instead of fixed.
    void inline CResizer::AddChild(HWND wnd, Alignment corner, DWORD style)
    {
        assert(::IsWindow(wnd));
        assert(::GetParent(wnd) == m_parent);

        ResizeData rd;
        rd.corner = corner;
        rd.isFixedWidth  = !(style & RD_STRETCH_WIDTH);
        rd.isFixedHeight = !(style & RD_STRETCH_HEIGHT);
        CRect childRect;
        VERIFY(::GetWindowRect(wnd, &childRect));
        ::MapWindowPoints(HWND_DESKTOP, m_parent, (LPPOINT)&childRect, 2);
        rd.childRect = childRect;
        rd.wnd = wnd;

        std::vector<ResizeData>::iterator it;
        for (it = m_resizeData.begin(); it != m_resizeData.end(); ++ it)
        {
            if ( it->wnd == wnd)
            {
                // Replace the value.
                *it = rd;
                break;
            }
        }

        // Add the value.
        if (it == m_resizeData.end())
            m_resizeData.push_back(rd);
    }

    // A callback function used by EnumChildWindows.
    inline BOOL CALLBACK CResizer::EnumWindowsProc(HWND wnd, LPARAM lparam)
    {
        CResizer* pResizer = reinterpret_cast<CResizer*>(lparam);

        // Only for a child, not other descendants.
        if (::GetParent(wnd) == pResizer->m_parent)
        {
            // Add the child window to set of windows managed by CResizer.
            pResizer->AddChild(wnd, topleft, 0);
        }

        return TRUE;
    }

    // Performs the resizing and scrolling. Call this function from within the window's DialogProc.
    inline void CResizer::HandleMessage(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        switch (msg)
        {
        case WM_DPICHANGED:
            OnAfterDpiChange();
            break;

        case WM_DPICHANGED_AFTERPARENT:
            OnAfterDpiChange();
            break;

        case WM_DPICHANGED_BEFOREPARENT:
            OnBeforeDpiChange();
            break;

        case WM_GETDPISCALEDSIZE:
            OnBeforeDpiChange();
            break;

        case WM_SIZE:
            RecalcLayout();
            break;

        case WM_HSCROLL:
            if (lparam == 0)
                OnHScroll(msg, wparam, lparam);
            break;

        case WM_VSCROLL:
            if (lparam == 0)
                OnVScroll(msg, wparam, lparam);
            break;
        }
    }

    // Sets up the Resizer by specifying the parent window (usually a dialog),
    // and the minimum and maximum allowed rectangle sizes.
    // Note: parent can either be a CWnd or a window handle (HWND)
    inline void CResizer::Initialize(HWND parent, const RECT& minRect, const RECT& maxRect)
    {
        assert (::IsWindow(parent));

        m_parent = parent;
        VERIFY(::GetClientRect(parent, &m_initRect));

        m_minRect = minRect;
        m_maxRect = maxRect;

        m_resizeData.clear();

        // Add scroll bar support to the parent window
        LONG_PTR style = static_cast<LONG_PTR>(::GetClassLongPtr(parent, GCL_STYLE));
        style |= WS_HSCROLL | WS_VSCROLL;
        ::SetClassLongPtr(parent, GCL_STYLE, style);

        // Calls AddChild for each child window with default settings.
        ::EnumChildWindows(parent, EnumWindowsProc, reinterpret_cast<LPARAM>(this));

        int dpi = GetWindowDpi(m_parent);
        double scale = static_cast<double>(dpi) / static_cast<double>(m_currentDpi);

        ScaleRect(m_minRect, scale);
        ScaleRect(m_maxRect, scale);

        m_currentDpi = dpi;
        m_initDpi = dpi;
        RecalcLayout();
    }

    inline void CResizer::OnBeforeDpiChange()
    {
        // Set the flag to disable recalculating the layout.
        m_isDpiChanging = true;
    }

    // Rescales the layout to changes in the parent window's dpi.
    inline void CResizer::OnAfterDpiChange()
    {
        int dpi = GetWindowDpi(m_parent);
        double scale = static_cast<double>(dpi) /
                       static_cast<double>(m_currentDpi);

        // Adjust the scrolling position.
        m_xScrollPos = static_cast<int>(m_xScrollPos * scale);
        m_yScrollPos = static_cast<int>(m_yScrollPos * scale);

        // Adjust the rectangles.
        ScaleRect(m_initRect, scale);
        ScaleRect(m_minRect, scale);
        ScaleRect(m_maxRect, scale);

        // Reposition the child windows.
        m_currentDpi = dpi;
        m_isDpiChanging = false;
        RecalcLayout();
    }

    // Called to perform horizontal scrolling.
    inline void CResizer::OnHScroll(UINT, WPARAM wparam, LPARAM)
    {
        int xNewPos;

        switch (LOWORD(wparam))
        {
            case SB_LEFT: // Scrolls to the upper left.
                xNewPos = 0;
                break;

            case SB_RIGHT: // Scrolls to the lower right.
                xNewPos = m_minRect.Width();
                break;

            case SB_PAGELEFT: // User clicked the scroll bar shaft left of the scroll box.
                xNewPos = m_xScrollPos - 50;
                break;

            case SB_PAGERIGHT: // User clicked the scroll bar shaft right of the scroll box.
                xNewPos = m_xScrollPos + 50;
                break;

            case SB_LINELEFT: // User clicked the left arrow.
                xNewPos = m_xScrollPos - 5;
                break;

            case SB_LINERIGHT: // User clicked the right arrow.
                xNewPos = m_xScrollPos + 5;
                break;

            case SB_THUMBPOSITION: // User has dragged the scroll box.
                xNewPos = HIWORD(wparam);
                break;

            case SB_THUMBTRACK: // User is dragging the scroll box.
                xNewPos = HIWORD(wparam);
                break;

            default:
                xNewPos = m_xScrollPos;
        }

        // Scroll the window.
        xNewPos = std::max(0, xNewPos);
        CRect rc;
        VERIFY(::GetClientRect(m_parent, &rc));
        xNewPos = std::min( xNewPos, GetMinRect().Width() - rc.Width() );
        int xDelta = xNewPos - m_xScrollPos;
        m_xScrollPos = xNewPos;
        VERIFY(::ScrollWindow(m_parent, -xDelta, 0, nullptr, nullptr));

        // Update the scroll bar.
        SCROLLINFO si{};
        si.cbSize = sizeof(si);
        si.fMask  = SIF_POS;
        si.nPos   = m_xScrollPos;
        ::SetScrollInfo(m_parent, SB_HORZ, &si, TRUE);
    }

    // Called to perform vertical scrolling.
    void inline CResizer::OnVScroll(UINT, WPARAM wparam, LPARAM)
    {
        int yNewPos;

        switch (LOWORD(wparam))
        {
            case SB_TOP: // Scrolls to the top.
                yNewPos = 0;
                break;

            case SB_BOTTOM: // Scrolls to the bottom.
                yNewPos = m_minRect.Height();
                break;

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

            case SB_THUMBPOSITION: // User has dragged the scroll box.
                yNewPos = HIWORD(wparam);
                break;

            case SB_THUMBTRACK: // User is dragging the scroll box.
                yNewPos = HIWORD(wparam);
                break;

            default:
                yNewPos = m_yScrollPos;
        }

        // Scroll the window.
        yNewPos = std::max(0, yNewPos);
        CRect rc;
        VERIFY(::GetClientRect(m_parent, &rc));
        yNewPos = std::min( yNewPos, GetMinRect().Height() - rc.Height() );
        int yDelta = yNewPos - m_yScrollPos;
        m_yScrollPos = yNewPos;
        VERIFY(::ScrollWindow(m_parent, 0, -yDelta, nullptr, nullptr));

        // Update the scroll bar.
        SCROLLINFO si{};
        si.cbSize = sizeof(si);
        si.fMask  = SIF_POS;
        si.nPos   = m_yScrollPos;
        ::SetScrollInfo(m_parent, SB_VERT, &si, TRUE);
    }

    // Repositions the child windows. Call this function when handling
    // the WM_SIZE message in the parent window.
    void inline CResizer::RecalcLayout()
    {
        assert (m_initRect.Width() > 0 && m_initRect.Height() > 0);
        assert (::IsWindow(m_parent));

        // Skip recalculating the layout if the DPI is changing.
        if (m_isDpiChanging)
            return;

        CRect currentRect;
        VERIFY(::GetClientRect(m_parent, &currentRect));

        // Adjust the scrolling if required
        m_xScrollPos = std::min(m_xScrollPos, std::max(0, m_minRect.Width()  - currentRect.Width() ) );
        m_yScrollPos = std::min(m_yScrollPos, std::max(0, m_minRect.Height() - currentRect.Height()) );
        SCROLLINFO si{};
        si.cbSize = sizeof(si);
        si.fMask  = SIF_RANGE | SIF_PAGE | SIF_POS;
        si.nMax   = m_minRect.Width();
        si.nPage  = static_cast<UINT>(currentRect.Width());
        si.nPos   = m_xScrollPos;
        ::SetScrollInfo(m_parent, SB_HORZ, &si, TRUE);
        si.nMax   = m_minRect.Height();
        si.nPage  = static_cast<UINT>(currentRect.Height());
        si.nPos   = m_yScrollPos;
        ::SetScrollInfo(m_parent, SB_VERT, &si, TRUE);

        // Note: calls to SetScrollInfo may have changed the client rect, so
        // we get it again.
        VERIFY(::GetClientRect(m_parent, &currentRect));

        currentRect.right  = std::max(currentRect.Width(),  m_minRect.Width() );
        currentRect.bottom = std::max(currentRect.Height(), m_minRect.Height() );
        if (!m_maxRect.IsRectEmpty())
        {
            currentRect.right  = std::min(currentRect.Width(),  m_maxRect.Width() );
            currentRect.bottom = std::min(currentRect.Height(), m_maxRect.Height() );
        }

        int dpi = GetWindowDpi(m_parent);
        double scale = static_cast<double>(dpi) / static_cast<double>(m_initDpi);

        // Allocates memory for the multiple-window-position structure.
        HDWP hdwp = ::BeginDeferWindowPos(static_cast<int>(m_resizeData.size()));

        for (const ResizeData& rd : m_resizeData)
        {
            int left   = 0;
            int top    = 0;
            int width  = 0;
            int height = 0;

            CRect childRect = rd.childRect;
            ScaleRect(childRect, scale);

            // Calculate the new size and position of the child window.
            switch(rd.corner )
            {
            case topleft:      // Positions top left.
                width  = rd.isFixedWidth? childRect.Width()  : childRect.Width()  - m_initRect.Width() + currentRect.Width();
                height = rd.isFixedHeight? childRect.Height() : childRect.Height() - m_initRect.Height() + currentRect.Height();
                left   = childRect.left;
                top    = childRect.top;
                break;
            case topright:     // Positions top right.
                width  = rd.isFixedWidth? childRect.Width()  : childRect.Width()  - m_initRect.Width() + currentRect.Width();
                height = rd.isFixedHeight? childRect.Height() : childRect.Height() - m_initRect.Height() + currentRect.Height();
                left   = childRect.right - width - m_initRect.Width() + currentRect.Width();
                top    = childRect.top;
                break;
            case bottomleft:   // Positions bottom left.
                width  = rd.isFixedWidth? childRect.Width()  : childRect.Width()  - m_initRect.Width() + currentRect.Width();
                height = rd.isFixedHeight? childRect.Height() : childRect.Height() - m_initRect.Height() + currentRect.Height();
                left   = childRect.left;
                top    = childRect.bottom - height - m_initRect.Height() + currentRect.Height();
                break;
            case bottomright:  // Positions bottom right.
                width  = rd.isFixedWidth? childRect.Width()  : childRect.Width()  - m_initRect.Width() + currentRect.Width();
                height = rd.isFixedHeight? childRect.Height() : childRect.Height() - m_initRect.Height() + currentRect.Height();
                left   = childRect.right   - width - m_initRect.Width() + currentRect.Width();
                top    = childRect.bottom  - height - m_initRect.Height() + currentRect.Height();
                break;
            case center:       // Positions proportionally.
                width  = rd.isFixedWidth ? childRect.Width() : (childRect.Width() * currentRect.Width()) / m_initRect.Width();
                height = rd.isFixedHeight ? childRect.Height() : (childRect.Height() * currentRect.Height()) / m_initRect.Height();
                left   = (childRect.left * currentRect.Width()) / m_initRect.Width();
                top    = (childRect.top * currentRect.Height()) / m_initRect.Height();
                break;
            case leftcenter:   // Positions proportionally along the left side.
                width  = rd.isFixedWidth ? childRect.Width() : childRect.Width() - m_initRect.Width() + currentRect.Width();
                height = rd.isFixedHeight ? childRect.Height() : (childRect.Height() * currentRect.Height()) / m_initRect.Height();
                left   = childRect.left;
                top    = (childRect.top * currentRect.Height()) / m_initRect.Height();
                break;
            case rightcenter:  // Positions proportionally along the right side.
                width  = rd.isFixedWidth ? childRect.Width() : childRect.Width() - m_initRect.Width() + currentRect.Width();
                height = rd.isFixedHeight ? childRect.Height() : (childRect.Height() * currentRect.Height()) / m_initRect.Height();
                left   = childRect.right - width - m_initRect.Width() + currentRect.Width();
                top    = (childRect.top * currentRect.Height()) / m_initRect.Height();
                break;
            case topcenter:    // Positions proportionally along the top side.
                width  = rd.isFixedWidth ? childRect.Width() : (childRect.Width() * currentRect.Width()) /  m_initRect.Width();
                height = rd.isFixedHeight ? childRect.Height() : childRect.Height() - m_initRect.Height() + currentRect.Height();
                left   = (childRect.left * currentRect.Width()) / m_initRect.Width();
                top    = childRect.top;
                break;
            case bottomcenter: // Positions proportionally along the bottom side.
                width  = rd.isFixedWidth ? childRect.Width() : (childRect.Width() * currentRect.Width()) / m_initRect.Width();
                height = rd.isFixedHeight ? childRect.Height() : childRect.Height() - m_initRect.Height() + currentRect.Height();
                left   = (childRect.left * currentRect.Width()) / m_initRect.Width();
                top    = childRect.bottom - height - m_initRect.Height() + currentRect.Height();
                break;
            }

            // Determine the position of the child window.
            CRect rc(left - m_xScrollPos, top - m_yScrollPos, left + width - m_xScrollPos, top + height - m_yScrollPos);

            // Note: The tab order of the dialog's controls is determined by the order
            //       they are specified in the resource script (resource.rc).

            // Store the window's new position. Repositioning happens later.
            hdwp = ::DeferWindowPos(hdwp, rd.wnd, HWND_TOP, rc.left, rc.top, rc.Width(), rc.Height(), SWP_NOZORDER|SWP_NOCOPYBITS);
        }

        // Reposition all the child windows simultaneously.
        VERIFY(::EndDeferWindowPos(hdwp));
    }

    inline void CResizer::ScaleRect(CRect& rc, double scale)
    {
        rc.left = static_cast<LONG>(rc.left * scale);
        rc.top = static_cast<LONG>(rc.top * scale);
        rc.right = static_cast<LONG>(rc.right * scale);
        rc.bottom = static_cast<LONG>(rc.bottom * scale);
    }

} // namespace Win32xx


#endif // _WIN32XX_DIALOG_H_
