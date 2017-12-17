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
        virtual LRESULT OnActivate(UINT uMsg, WPARAM wParam, LPARAM lParam);
        virtual int OnCreate(CREATESTRUCT& cs);
        virtual void OnDestroy();
        virtual LRESULT OnDockActivated(UINT uMsg, WPARAM wParam, LPARAM lParam);
        virtual LRESULT OnDockDestroyed(UINT uMsg, WPARAM wParam, LPARAM lParam);
        virtual LRESULT OnMouseActivate(UINT uMsg, WPARAM wParam, LPARAM lParam);
        virtual LRESULT OnNotify(WPARAM wParam, LPARAM lParam);
        virtual LRESULT OnSysColorChange(UINT uMsg, WPARAM wParam, LPARAM lParam);
        virtual void RecalcViewLayout();
        virtual LRESULT WndProcDefault(UINT uMsg, WPARAM wParam, LPARAM lParam);

    };


    /////////////////////////////////////////
    // Provides a Multiple Document Interface (MDI) frame with docking
    class CMDIDockFrame : public CMDIFrameT<CDockFrame>
    {
    public:
        CMDIDockFrame();
        virtual ~CMDIDockFrame() {}

        virtual CWnd& GetMDIClient() const      { return m_DockMDIClient; }
        virtual CDocker::CDockClient& GetDockClient() const { return m_DockMDIClient; }

    protected:
        virtual int OnCreate(CREATESTRUCT& cs);

    private:
        mutable CMDIClient<CDocker::CDockClient> m_DockMDIClient;   // MDIClient for docking
    };

}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


namespace Win32xx
{

    /////////////////////////////////////////
    // Definitions for the CDockFrame class
    //

    // Called when the frame window is activated.
    inline LRESULT CDockFrame::OnActivate(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        CDocker::OnActivate(uMsg, wParam, lParam);
        return CFrameT<CDocker>::OnActivate(uMsg, wParam, lParam);
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
    inline LRESULT CDockFrame::OnDockActivated(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        return CDocker::OnDockActivated(uMsg, wParam, lParam);
    }

    // Called when a docker is destroyed.
    inline LRESULT CDockFrame::OnDockDestroyed(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        return CDocker::OnDockDestroyed(uMsg, wParam, lParam);
    }

    inline LRESULT CDockFrame::OnMouseActivate(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        return CDocker::OnMouseActivate(uMsg, wParam, lParam);
    }

    inline LRESULT CDockFrame::OnNotify(WPARAM wParam, LPARAM lParam)
    // Called when a notification from a child window (WM_NOTIFY) is received.
    {
        LRESULT lr = CFrameT<CDocker>::OnNotify(wParam, lParam);
        if (lr == 0)
            lr = CDocker::OnNotify(wParam, lParam);

        return lr;
    }


    // Called when the system colors are changed.
    inline LRESULT CDockFrame::OnSysColorChange(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        CDocker::OnSysColorChange(uMsg, wParam, lParam);
        return CFrameT<CDocker>::OnSysColorChange(uMsg, wParam, lParam);
    }


    // Repositions the view window
    inline void CDockFrame::RecalcViewLayout()
    {
        RecalcDockLayout();
    }


    // Process the frame's window messages.
    inline LRESULT CDockFrame::WndProcDefault(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        switch (uMsg)
        {
        case WM_ACTIVATE:           return OnActivate(uMsg, wParam, lParam);
        case WM_MOUSEACTIVATE:      return OnMouseActivate(uMsg, wParam, lParam);
        case WM_SYSCOLORCHANGE:     return OnSysColorChange(uMsg, wParam, lParam);

        // Messages defined by Win32++
        case UWM_DOCKACTIVATE:      return OnDockActivated(uMsg, wParam, lParam);
        case UWM_DOCKDESTROYED:     return OnDockDestroyed(uMsg, wParam, lParam);
        case UWM_GETCDOCKER:        return reinterpret_cast<LRESULT>(this);

        } // switch uMsg

        return CFrameT<CDocker>::WndProcDefault(uMsg, wParam, lParam);
    }

    //////////////////////////////////////////
    // Definitions for the CMDIDockFrame class
    //

    // Constructor.
    inline CMDIDockFrame::CMDIDockFrame()
    {
        // The view window for a CMDIDockFrame is the MDI Client
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
