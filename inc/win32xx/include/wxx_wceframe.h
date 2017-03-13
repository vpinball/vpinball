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


//////////////////////////////////////////////////////
// wxx_wceframe.h
// Definitions for the CCmdBar and CWceFrame

// These classes are provide a frame window for use on Window CE devices such
// as Pocket PCs. The frame uses CommandBar (a control unique to the Windows CE
// operating systems) to display the menu and toolbar.
//
// Use the PocketPCWceFrame generic application as the starting point for your own
// frame based applications on the Pocket PC.
//
// Refer to the Scribble demo application for an example of how these classes
// can be used.


#ifndef _WIN32XX_WCEFRAME_H_
#define _WIN32XX_WCEFRAME_H_


#include "wxx_wincore.h"
#include <commctrl.h>
#include <vector>
#include "default_resource.h"

#if defined(WIN32_PLATFORM_PSPC) || defined(WIN32_PLATFORM_WFSP)
  #define SHELL_AYGSHELL
#endif

#ifdef SHELL_AYGSHELL
  #include <aygshell.h>
  #pragma comment(lib, "aygshell.lib")
#endif // SHELL_AYGSHELL

#if (_WIN32_WCE < 0x500 && defined(SHELL_AYGSHELL)) || _WIN32_WCE == 420
  #pragma comment(lib, "ccrtrtti.lib")
#endif


namespace Win32xx
{

	////////////////////////////////////
	// Declaration of the CCmdBar class
	//
	class CCmdBar : public CWnd
	{
	public:
		CCmdBar();
		virtual ~CCmdBar();
		virtual BOOL AddAdornments(DWORD dwFlags) const;
		virtual int  AddBitmap(int idBitmap, int iNumImages, int iImageWidth, int iImageHeight) const;
		virtual BOOL AddButtons(int nButtons, TBBUTTON* pTBButton) const;
		virtual HWND Create(HWND hwndParent);
		virtual int  GetHeight() const;
		virtual HWND InsertComboBox(int iWidth, UINT dwStyle, WORD idComboBox, WORD iButton) const;
		virtual BOOL IsVisible() const;
		virtual BOOL Show(BOOL fShow) const;

	private:

#ifdef SHELL_AYGSHELL
		SHMENUBARINFO m_mbi;
#endif

	};


	//////////////////////////////////////
	// Declaration of the CWceFrame class
	//  A mini frame based on CCmdBar
	class CWceFrame : public CWnd
	{
	public:
		CWceFrame();
		virtual ~CWceFrame();
		virtual void AddToolBarButton(UINT nID);
		CRect GetViewRect() const;
		virtual CCmdBar& GetMenuBar() const { return m_MenuBar; }
		virtual void OnActivate(UINT uMsg, WPARAM wParam, LPARAM lParam);
		virtual int  OnCreate(CREATESTRUCT& cs);		
		virtual void PreCreate(CREATESTRUCT& cs);
		virtual void RecalcLayout();
		virtual void SetButtons(const std::vector<UINT> ToolBarData);
		virtual	LRESULT WndProcDefault(UINT uMsg, WPARAM wParam, LPARAM lParam);

	protected:
		std::vector<UINT> m_ToolBarData;

	private:
		mutable CCmdBar m_MenuBar;
		CString m_strAppName;

#ifdef SHELL_AYGSHELL
		SHACTIVATEINFO m_sai;
#endif

	};

	//////////////////////////////////////////
	// Definitions for the CCmdBar class
	//  This class wraps CommandBar_Create which
	//  creates a CommandBar at the top of the window
	inline CCmdBar::CCmdBar()
	{
	}

	inline CCmdBar::~CCmdBar()
	{
		if (IsWindow())
			::CommandBar_Destroy(*this);
	}

	inline BOOL CCmdBar::AddAdornments(DWORD dwFlags) const
	// Adds a button, and optionally, Help and OK buttons, to the command bar.
	{
		assert(IsWindow());
		return CommandBar_AddAdornments(*this, dwFlags, 0);
	}

	inline int CCmdBar::AddBitmap(int idBitmap, int iNumImages, int iImageWidth, int iImageHeight) const
	// Adds one or more images to the list of button images available in the command bar.
	{
		assert(IsWindow());
		HINSTANCE hInst = GetApp().GetInstanceHandle();
		return 	CommandBar_AddBitmap(*this, hInst, idBitmap, iNumImages, iImageWidth, iImageHeight);
	}

	inline BOOL CCmdBar::AddButtons(int nButtons, TBBUTTON* pTBButton) const
	// Adds one or more toolbar buttons to a command bar control.
	{
		assert(IsWindow());
		return CommandBar_AddButtons(*this, nButtons, pTBButton);
	}

	inline HWND CCmdBar::Create(HWND hParent)
	// Creates the command bar control.
	{
#ifdef SHELL_AYGSHELL
		SHMENUBARINFO mbi;

		memset(&mbi, 0, sizeof(SHMENUBARINFO));
		mbi.cbSize     = sizeof(SHMENUBARINFO);
		mbi.hwndParent = hParent;
		mbi.nToolBarId = IDW_MAIN;
		mbi.hInstRes   = GetApp().GetInstanceHandle();
		mbi.nBmpId     = 0;
		mbi.cBmpImages = 0;

		if (SHCreateMenuBar(&mbi))
		{
			Attach(mbi.hwndMB);
		}
		
#else
		*this = CommandBar_Create(GetApp().GetInstanceHandle(), hParent, IDW_MENUBAR);
		assert (*this);

		CommandBar_InsertMenubar(*this, GetApp().GetInstanceHandle(), IDW_MAIN, 0);
#endif
		assert(IsWindow());
		return *this;
	}

	inline int CCmdBar::GetHeight() const
	// Retrieves the height of the command bar in pixels.
	{
		assert(IsWindow());
		return CommandBar_Height(*this);
	}

	inline HWND CCmdBar::InsertComboBox(int iWidth, UINT dwStyle, WORD idComboBox, WORD iButton) const
	// Inserts a combo box into the command bar.
	{
		HINSTANCE hInst = GetApp().GetInstanceHandle();
		return CommandBar_InsertComboBox(*this, hInst, iWidth, dwStyle, idComboBox, iButton);
	}

	inline BOOL CCmdBar::IsVisible() const
	// Retrieves the visibility state of the command bar.
	{
		assert(IsWindow());
		return ::CommandBar_IsVisible(*this);
	}

	inline BOOL CCmdBar::Show(BOOL fShow) const
	// Shows or hides the command bar.
	{
		assert(IsWindow());
		return ::CommandBar_Show(*this, fShow);
	}


	/////////////////////////////////////////
	// Definitions for the CWceFrame class
	//  This class creates a simple frame using CCmdBar
	inline CWceFrame::CWceFrame()
	{
#ifdef SHELL_AYGSHELL
		// Initialize the shell activate info structure
		memset (&m_sai, 0, sizeof (m_sai));
		m_sai.cbSize = sizeof (m_sai);
#endif
	}

	inline CWceFrame::~CWceFrame()
	{
	}

	inline void CWceFrame::AddToolBarButton(UINT nID)
	// Adds Resource IDs to toolbar buttons.
	// A resource ID of 0 is a separator
	{
		m_ToolBarData.push_back(nID);
	}

	inline CRect CWceFrame::GetViewRect() const
	// Returns a RECT structure which contains the dimensions of the client area of the frame.
	{
		CRect r;
		::GetClientRect(*this, &r);

#ifndef SHELL_AYGSHELL
		// Reduce the size of the client rectangle, by the commandbar height
		r.top += GetMenuBar()->GetHeight();
#endif

		return r;
	}

	inline int CWceFrame::OnCreate(CREATESTRUCT& cs)
	// Called during window creation. Override this function to perform tasks such as
	//  creating child windows.
	{
		// Create the Commandbar
		GetMenuBar().Create(*this);

		// Set the keyboard accelerators
		HACCEL hAccel = LoadAccelerators(GetApp().GetResourceHandle(), MAKEINTRESOURCE(IDW_MAIN));
		GetApp().SetAccelerators(hAccel, *this);

		// Add the toolbar buttons
		if (m_ToolBarData.size() > 0)
			SetButtons(m_ToolBarData);

#ifndef SHELL_AYGSHELL
		// Add close button
		GetMenuBar()->AddAdornments(0);
#endif

		return 0;
	}

	inline void CWceFrame::OnActivate(UINT, WPARAM wParam, LPARAM lParam)
	// Called when the frame is activated.
	{
#ifdef SHELL_AYGSHELL
		// Notify shell of our activate message
		SHHandleWMActivate(*this, wParam, lParam, &m_sai, FALSE);

		UINT fActive = LOWORD(wParam);
		if ((fActive == WA_ACTIVE) || (fActive == WA_CLICKACTIVE))
		{
			// Reposition the window when it's activated
			RecalcLayout();
		}
#endif
	}

	inline void CWceFrame::PreCreate(CREATESTRUCT& cs)
	// Called before the window is created. Override this function to set the window creation parameters.
	{
		cs.style = WS_VISIBLE;
		m_strAppName = _T("Win32++ Application");

		// Choose a unique class name for this app
		if (LoadString(IDW_MAIN) != _T(""))
		{
			m_strAppName = LoadString(IDW_MAIN);
		}
			
		cs.lpszClass = m_strAppName;
	}

	inline void CWceFrame::RecalcLayout()
	// Repositions the client area of the frame.
	{
		HWND hwndCB = GetMenuBar().GetHwnd();
		if (hwndCB)
		{
			CRect rc;			// Desktop window size
			CRect rcMenuBar;	// MenuBar window size

			::SystemParametersInfo(SPI_GETWORKAREA, 0, &rc, 0);
			::GetWindowRect(hwndCB, &rcMenuBar);
			rc.bottom -= (rcMenuBar.bottom - rcMenuBar.top);

			MoveWindow(rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, FALSE);
		}

		ShowWindow(TRUE);
		UpdateWindow();
	}

	inline void CWceFrame::SetButtons(const std::vector<UINT> ToolBarData)
	// Define the resource IDs for the toolbar like this in the Frame's constructor
	// m_ToolBarData.push_back ( 0 );				// Separator
	// m_ToolBarData.clear();
	// m_ToolBarData.push_back ( IDM_FILE_NEW   );
	// m_ToolBarData.push_back ( IDM_FILE_OPEN  );
	// m_ToolBarData.push_back ( IDM_FILE_SAVE  );

	{
		int iImages = 0;
		int iNumButtons = (int)ToolBarData.size();

		if (iNumButtons > 0)
		{
			// Create the TBBUTTON array for each button
			std::vector<TBBUTTON> vTBB(iNumButtons);
			TBBUTTON* tbbArray = &vTBB.front();

			for (int j = 0 ; j < iNumButtons; j++)
			{
				ZeroMemory(&tbbArray[j], sizeof(TBBUTTON));

				if (ToolBarData[j] == 0)
				{
					tbbArray[j].fsStyle = TBSTYLE_SEP;
				}
				else
				{
					tbbArray[j].iBitmap = iImages++;
					tbbArray[j].idCommand = ToolBarData[j];
					tbbArray[j].fsState = TBSTATE_ENABLED;
					tbbArray[j].fsStyle = TBSTYLE_BUTTON;
					tbbArray[j].iString = -1;
				}
			}

			// Add the bitmap
			GetMenuBar().AddBitmap(IDW_MAIN, iImages , 16, 16);

			// Add the buttons
			GetMenuBar().AddButtons(iNumButtons, tbbArray);
		}
	}

	inline LRESULT CWceFrame::WndProcDefault(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (uMsg)
		{
			case WM_DESTROY:
				PostQuitMessage(0);
				break;
			case WM_ACTIVATE:
				OnActivate(uMsg, wParam, lParam);
     			break;

#ifdef SHELL_AYGSHELL

			case WM_SETTINGCHANGE:
				SHHandleWMSettingChange(*this, wParam, lParam, &m_sai);
     			break;
#endif

		}
		return CWnd::WndProcDefault(uMsg, wParam, lParam);
	}


} // namespace Win32xx

#endif // _WIN32XX_WCEFRAME_H_

