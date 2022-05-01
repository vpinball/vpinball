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
// wxx_dockframe.h
//  Declaration of CDockFrame and CMDIDockFrame


#ifndef _WIN32XX_DOCKFRAME_H_
#define _WIN32XX_DOCKFRAME_H_


#include "wxx_docking.h"
#include "wxx_frame.h"
#include "wxx_mdi.h"


///////////////////////////////////////////////////////////////////////////////
// The CDockFrame class adds support for docking to a Single Document Interface
// (SDI) frame. CDockFrame inherits from CFrameT<Docker>. CDockFrame has access
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
        CDockFrame() {}
        virtual ~CDockFrame() {}
        virtual CWnd& GetView() const       { return CDocker::GetView(); }
        virtual void SetView(CWnd& wndView) { CDocker::SetView(wndView); }

    protected:
        virtual LRESULT OnActivate(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual int     OnCreate(CREATESTRUCT& cs);
        virtual void    OnDestroy();
        virtual LRESULT OnDockActivated(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual LRESULT OnDockDestroyed(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual LRESULT OnMouseActivate(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual LRESULT OnNotify(WPARAM wparam, LPARAM lparam);
        virtual LRESULT OnSysColorChange(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual void    RecalcViewLayout();
        virtual LRESULT WndProcDefault(UINT msg, WPARAM wparam, LPARAM lparam);

    private:
        CDockFrame(const CDockFrame&);              // Disable copy construction
        CDockFrame& operator = (const CDockFrame&); // Disable assignment operator
    };


    //////////////////////////////////////////////////////////////////
    // Provides a frame which supports the Multiple Document Interface
    // (MDI) and docking. It also manages the creation and position
    // of child windows, such as the menubar, toolbar, and statusbar.
    class CMDIDockFrame : public CMDIFrameT<CDockFrame>
    {
    public:
        CMDIDockFrame();
        virtual ~CMDIDockFrame() {}

        virtual CWnd& GetMDIClient() const { return *m_pDockMDIClient; }
        void SetDockClient(CMDIClient<CDocker::CDockClient>& dockClient) { m_pDockMDIClient = &dockClient; }

    protected:
        virtual int OnCreate(CREATESTRUCT& cs);

    private:
        CMDIClient<CDocker::CDockClient> m_dockMDIClient;   // MDIClient for docking
        CMDIClient<CDocker::CDockClient>* m_pDockMDIClient;
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

        // Set the caption height based on text height
        SetCaptionHeight( MAX(20, GetTextHeight() + 5) );
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

    inline LRESULT CDockFrame::OnMouseActivate(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        return CDocker::OnMouseActivate(msg, wparam, lparam);
    }

    inline LRESULT CDockFrame::OnNotify(WPARAM wparam, LPARAM lparam)
    // Called when a notification from a child window (WM_NOTIFY) is received.
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
        RecalcDockLayout();
    }

    // Process the frame's window messages.
    inline LRESULT CDockFrame::WndProcDefault(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        switch (msg)
        {
        case WM_ACTIVATE:           return OnActivate(msg, wparam, lparam);
        case WM_MOUSEACTIVATE:      return OnMouseActivate(msg, wparam, lparam);
        case WM_SYSCOLORCHANGE:     return OnSysColorChange(msg, wparam, lparam);

        // Messages defined by Win32++
        case UWM_DOCKACTIVATE:      return OnDockActivated(msg, wparam, lparam);
        case UWM_DOCKDESTROYED:     return OnDockDestroyed(msg, wparam, lparam);
        case UWM_GETCDOCKER:        return reinterpret_cast<LRESULT>(this);

        } // switch msg

        return CFrameT<CDocker>::WndProcDefault(msg, wparam, lparam);
    }

    //////////////////////////////////////////
    // Definitions for the CMDIDockFrame class
    //

    // Constructor.
    inline CMDIDockFrame::CMDIDockFrame()
    {
        // The view window for a CMDIDockFrame is the MDI Client
        CDocker::SetDockClient(m_dockMDIClient);
        SetDockClient(m_dockMDIClient);

        SetView(GetMDIClient());
        GetDockClient().SetDocker(this);
    }

    // Called when the frame window is created
    inline int CMDIDockFrame::OnCreate(CREATESTRUCT& cs)
    {
        return CFrameT<CDocker>::OnCreate(cs);
    }

} // namespace Win32xx

#endif // _WIN32XX_DOCKFRAME_H_
