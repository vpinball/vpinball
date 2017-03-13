// Win32++   Version 8.4
// Release Date: 10th March 2017
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
	// Declaration of the CDockFrame class
	// Provides a Single Document Interface (SDI) frame with docking
	//
	class CDockFrame : public CFrameT<CDocker>
	{
	public:
		CDockFrame() {}
		virtual ~CDockFrame() {}
		virtual CWnd& GetView()	const		{ return CDocker::GetView(); }
		virtual void SetView(CWnd& wndView)	{ CDocker::SetView(wndView); }

	protected:
		virtual void RecalcViewLayout();
		virtual int OnCreate(CREATESTRUCT& cs);
		virtual void OnDestroy();
		virtual LRESULT OnNotify(WPARAM wParam, LPARAM lParam);
		virtual LRESULT WndProcDefault(UINT uMsg, WPARAM wParam, LPARAM lParam);

	};
	
	
	/////////////////////////////////////////
	// Declaration of the CMDIDockFrame class
	// Provides a Multiple Document Interface (MDI) frame with docking
	//
	class CMDIDockFrame : public CMDIFrameT<CDockFrame>
	{
	public:
		CMDIDockFrame();
		virtual ~CMDIDockFrame() {}

		virtual CWnd& GetMDIClient() const		{ return m_DockMDIClient; }
		virtual CDocker::CDockClient& GetDockClient() const { return m_DockMDIClient; }

	protected:
		virtual int OnCreate(CREATESTRUCT& cs);

	private:
		mutable CMDIClient<CDocker::CDockClient> m_DockMDIClient;	// MDIClient for docking
	};	

}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


namespace Win32xx
{

	/////////////////////////////////////////
	// Definitions for the CDockFrame class
	//
	
	inline void CDockFrame::RecalcViewLayout()
	// Re-positions the view window
	{ 
		RecalcDockLayout(); 
	}

	inline int CDockFrame::OnCreate(CREATESTRUCT& cs)
	// Called when the frame window is created
	{
		GetDockClient().Create(GetHwnd());
		GetView().Create(GetDockClient());
		return CFrameT<CDocker>::OnCreate(cs);
	}

	inline void CDockFrame::OnDestroy()
	// Called when the frame window is destroyed
	{
		CDocker::OnDestroy();
		CFrameT<CDocker>::OnDestroy();
	}

	inline LRESULT CDockFrame::OnNotify(WPARAM wParam, LPARAM lParam)
	// Called when a notification from a child window (WM_NOTIFY) is received.
	{
		LRESULT lr = CFrameT<CDocker>::OnNotify(wParam, lParam);
		if (lr == 0)
			lr = CDocker::OnNotify(wParam, lParam);

		// The framework will call SetWindowLongPtr(DWLP_MSGRESULT, lr) for non-zero returns
		return lr;
	}

	inline LRESULT CDockFrame::WndProcDefault(UINT uMsg, WPARAM wParam, LPARAM lParam)
	// Handle the frame's window messages.
	{
		switch (uMsg)
		{			
		case WM_ACTIVATE:			CDocker::OnActivate(uMsg, wParam, lParam); break;
		case WM_MOUSEACTIVATE:		return CDocker::OnMouseActivate(uMsg, wParam, lParam);
		case WM_SYSCOLORCHANGE:		CDocker::OnSysColorChange(uMsg, wParam, lParam); break;
		
		// Messages defined by Win32++
		case UWM_DOCKACTIVATE:		return CDocker::OnDockActivated(uMsg, wParam, lParam);
		case UWM_DOCKDESTROYED:		return CDocker::OnDockDestroyed(uMsg, wParam, lParam);
		case UWM_GETCDOCKER:		return reinterpret_cast<LRESULT>(this);		

		} // switch uMsg

		return CFrameT<CDocker>::WndProcDefault(uMsg, wParam, lParam);
	}

	//////////////////////////////////////////
	// Definitions for the CMDIDockFrame class
	//	
	
	inline CMDIDockFrame::CMDIDockFrame()
	// Constructor.
	{ 
		// The view window for a CMDIDockFrame is the MDI Client
		SetView(GetMDIClient());
	}
	
	inline int CMDIDockFrame::OnCreate(CREATESTRUCT& cs)
	// Called when the frame window is created
	{ 
		return CFrameT<CDocker>::OnCreate(cs);
	}

} // namespace Win32xx

#endif // _WIN32XX_DOCKFRAME_H_
