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
// wxx_dockframe.h
//  Declaration of CDockFrame and CMDIDockFrame


#ifndef _WIN32XX_DOCKFRAME_H_
#define _WIN32XX_DOCKFRAME_H_


#include "wxx_docking.h"
#include "wxx_frame.h"
#include "wxx_mdi.h"


///////////////////////////////////////////////////////////////////////////////
// The CDockFrame class adds support for docking to a Single Document Interface
// (SDI) frame. CDockFrame inherits from CFrameT<CDocker>. CDockFrame has access
// to the public and protected functions defined by CFrameT and CDocker, as well
// as those defined by CWnd.
//
// The CMDIDockFrame class adds support for docking to a Multiple Document
// Interface (MDI) frame. CMDIDockFrame inherits from CMDIFrameT<CDocker>.
// CMDIDockFrame has access to the public and protected functions defined by
// CMDIFrameT, CFrameT, CDocker and CWnd.
//
// Refer to the documentation for CMDIFrameT, CFrameT, CDocker, and CWnd for a
// description of the functions available.
///////////////////////////////////////////////////////////////////////////////


namespace Win32xx
{

    /////////////////////////////////////////
    // Provides a Single Document Interface (SDI) frame with docking
    class CDockFrame : public CFrameT<CDocker>
    {
    public:
        CDockFrame() : m_isDpiChanging(false) {}
        virtual ~CDockFrame() override {}

    protected:
        virtual LRESULT OnActivate(UINT msg, WPARAM wparam, LPARAM lparam) override;
        virtual int     OnCreate(CREATESTRUCT& cs) override;
        virtual void    OnDestroy() override;
        virtual LRESULT OnDockActivated(UINT msg, WPARAM wparam, LPARAM lparam) override;
        virtual LRESULT OnDockDestroyed(UINT msg, WPARAM wparam, LPARAM lparam) override;
        virtual LRESULT OnDpiChanged(UINT msg, WPARAM wparam, LPARAM lparam) override;
        virtual LRESULT OnGetDpiScaledSize(UINT msg, WPARAM wparam, LPARAM lparam) override;
        virtual LRESULT OnMouseActivate(UINT msg, WPARAM wparam, LPARAM lparam) override;
        virtual LRESULT OnNotify(WPARAM wparam, LPARAM lparam) override;
        virtual LRESULT OnSysColorChange(UINT msg, WPARAM wparam, LPARAM lparam) override;
        virtual void    RecalcViewLayout() override;
        virtual LRESULT WndProcDefault(UINT msg, WPARAM wparam, LPARAM lparam) override;

    private:
        CDockFrame(const CDockFrame&) = delete;
        CDockFrame& operator=(const CDockFrame&) = delete;
        bool m_isDpiChanging;
    };


    //////////////////////////////////////////////////////////////////
    // Provides a frame that supports the Multiple Document Interface
    // (MDI) and docking. It also manages the creation and position
    // of child windows, such as the menubar, toolbar, and statusbar.
    class CMDIDockFrame : public CMDIFrameT<CDockFrame>
    {
    public:
        CMDIDockFrame();
        virtual ~CMDIDockFrame() override {}

    protected:
        virtual int OnCreate(CREATESTRUCT& cs) override;

    private:
        CMDIDockFrame(const CMDIDockFrame&) = delete;
        CMDIDockFrame& operator=(const CMDIDockFrame&) = delete;

        CMDIClient<CDocker::CDockClient> m_dockMDIClient;   // Both a DockClient and MDIClient
    };

}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


namespace Win32xx
{

    /////////////////////////////////////////
    // Definitions for the CDockFrame class
    //

    // Called when the frame window is activated.
    inline LRESULT CDockFrame::OnActivate(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        CDocker::OnActivate(msg, wparam, lparam);
        return CFrameT<CDocker>::OnActivate(msg, wparam, lparam);
    }

    // Called when the frame window is created.
    inline int CDockFrame::OnCreate(CREATESTRUCT& cs)
    {
        GetDockClient().Create(GetHwnd());
        GetView().Create(GetDockClient());

        // Set the caption height based on text height.
        SetDefaultCaptionHeight();
        return CFrameT<CDocker>::OnCreate(cs);
    }

    // Called when the frame window is destroyed.
    inline void CDockFrame::OnDestroy()
    {
        CDocker::OnDestroy();
        CFrameT<CDocker>::OnDestroy();
    }

    // Called when a docker is activated.
    inline LRESULT CDockFrame::OnDockActivated(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        return CDocker::OnDockActivated(msg, wparam, lparam);
    }

    // Called when a docker is destroyed.
    inline LRESULT CDockFrame::OnDockDestroyed(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        return CDocker::OnDockDestroyed(msg, wparam, lparam);
    }

    // Called when the effective dots per inch (dpi) for a window has changed.
    // This occurs when:
    //  - The window is moved to a new monitor that has a different DPI.
    //  - The DPI of the monitor displaying the window changes.
    inline LRESULT CDockFrame::OnDpiChanged(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        CFrameT<CDocker>::OnDpiChanged(msg, wparam, lparam);
        SetDefaultCaptionHeight();
        DpiUpdateDockerSizes();
        m_isDpiChanging = false;

        return 0;
    }

    // Called when the DPI is about to change.
    inline LRESULT CDockFrame::OnGetDpiScaledSize(UINT, WPARAM, LPARAM)
    {
        m_isDpiChanging = true;
        return 0;
    }

    // Called when the cursor is in an inactive window and the user presses a mouse button.
    inline LRESULT CDockFrame::OnMouseActivate(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        return CDocker::OnMouseActivate(msg, wparam, lparam);
    }

    // Called when a notification from a child window (WM_NOTIFY) is received.
    inline LRESULT CDockFrame::OnNotify(WPARAM wparam, LPARAM lparam)
    {
        LRESULT result = CFrameT<CDocker>::OnNotify(wparam, lparam);
        if (result == 0)
            result = CDocker::OnNotify(wparam, lparam);

        return result;
    }

    // Called when the system colors are changed.
    inline LRESULT CDockFrame::OnSysColorChange(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        CDocker::OnSysColorChange(msg, wparam, lparam);
        return CFrameT<CDocker>::OnSysColorChange(msg, wparam, lparam);
    }

    // Repositions the view window
    inline void CDockFrame::RecalcViewLayout()
    {
        // Skip RecalcDockLayout during DPI changes.
        if (!m_isDpiChanging)
            RecalcDockLayout();
    }

    // Process the frame's window messages.
    inline LRESULT CDockFrame::WndProcDefault(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        switch (msg)
        {
        case WM_ACTIVATE:           return OnActivate(msg, wparam, lparam);
        case WM_GETDPISCALEDSIZE:   return OnGetDpiScaledSize(msg, wparam, lparam);
        case WM_DPICHANGED:         return OnDpiChanged(msg, wparam, lparam);
        case WM_MOUSEACTIVATE:      return OnMouseActivate(msg, wparam, lparam);
        case WM_SIZE:               return OnSize(msg, wparam, lparam);
        case WM_SYSCOLORCHANGE:     return OnSysColorChange(msg, wparam, lparam);

        // Messages defined by Win32++
        case UWM_DOCKACTIVATE:      return OnDockActivated(msg, wparam, lparam);
        case UWM_DOCKDESTROYED:     return OnDockDestroyed(msg, wparam, lparam);
        case UWM_GETCDOCKER:        return reinterpret_cast<LRESULT>(this);
        }

        return CFrameT<CDocker>::WndProcDefault(msg, wparam, lparam);
    }

    //////////////////////////////////////////
    // Definitions for the CMDIDockFrame class
    //

    // Constructor.
    inline CMDIDockFrame::CMDIDockFrame()
    {
        // Assign m_dockMDIClient as this MDI frame's MDI client.
        SetMDIClient(m_dockMDIClient);

        // Assign m_dockMDIClient as this docker's dock client.
        SetDockClient(m_dockMDIClient);
    }

    // Called when the frame window is created.
    inline int CMDIDockFrame::OnCreate(CREATESTRUCT& cs)
    {
        return CFrameT<CDocker>::OnCreate(cs);
    }


} // namespace Win32xx

#endif // _WIN32XX_DOCKFRAME_H_
