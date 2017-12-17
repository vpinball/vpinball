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
// wxx_mdi.h
//  Declaration of the CMDIChild and CMDIFrame classes

// The classes defined here add MDI frames support to Win32++. MDI
// (Multiple Document Interface) frames host one or more child windows. The
// child windows hosted by a MDI frame can be different types. For example,
// some MDI child windows could be used to edit text, while others could be
// used to display a bitmap. Four classes are defined here to support MDI
// frames:


// 1) CMDIFrameT. This class inherits from CFrameT, and adds the functionality
//    required by MDI frames. It keeps track of the MDI children created and
//    destroyed, and adjusts the menu when a MDI child is activated. Use the
//    AddMDIChild function to add MDI child windows to the MDI frame. Inherit
//    from CMDIFrame to create your own MDI frame.
//
// 2) CMDIChild: All MDI child windows (ie. CWnd classes) should inherit from
//    this class. Each MDI child type can have a different frame menu.
//
// 3) CMDIClient: This class is used by CMDIFrameT to host the MDIChild windows.
//
// 4) CMDIFrame: This class inherits from CMDIFrameT<CFrame>. It provides a
//    MDI frame. Inherit your CMainFrame class from CMDIFrame to create a MDI
//    frame.
//
// 5) CMDIDockFrame: This clas inherits from CMDIFrameT<CDockFrame>. It provides
//    a MDI frame which supports docking. Inherit your CMainFrame from CMDIDockFrame
//    to create a MDI frame which supports docking.

// Use the MDIFrame generic application as the starting point for your own MDI
// frame applications.
// Refer to the MDIDemo sample for an example on how to use these classes to
// create a MDI frame application with different types of MDI child windows.


#ifndef _WIN32XX_MDI_H_
#define _WIN32XX_MDI_H_

#include "wxx_frame.h"
#include <vector>


namespace Win32xx
{
    class CMDIChild;
    typedef Shared_Ptr<CMDIChild> MDIChildPtr;

    /////////////////////////////////////
    // The CMDIChild class the functionality of a MDI child window.
    class CMDIChild : public CWnd
    {
    public:
        CMDIChild();
        virtual ~CMDIChild();

        // These are the functions you might wish to override
        virtual HWND Create(HWND hWndParent = NULL);
        virtual void RecalcLayout();

        // These functions aren't virtual, and shouldn't be overridden
        HACCEL GetChildAccel() const { return m_hChildAccel; }
        HMENU GetChildMenu() const { return m_ChildMenu; }
        CWnd& GetView() const           { assert(m_pView); return *m_pView; }
        void SetView(CWnd& wndView);
        void SetHandles(HMENU MenuName, HACCEL AccelName);
        void MDIActivate() const;
        void MDIDestroy() const;
        void MDIMaximize() const;
        void MDIRestore() const;

    protected:
        // These are the functions you might wish to override
        virtual void OnClose();
        virtual int  OnCreate(CREATESTRUCT& cs);
        virtual LRESULT OnMDIActivate(UINT uMsg, WPARAM wParam, LPARAM lParam);
        virtual LRESULT OnWindowPosChanged(UINT uMsg, WPARAM wParam, LPARAM lParam);

        // Not intended to be overridden
        LRESULT FinalWindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
        LRESULT WndProcDefault(UINT uMsg, WPARAM wParam, LPARAM lParam);

    private:
        CMDIChild(const CMDIChild&);                // Disable copy construction
        CMDIChild& operator = (const CMDIChild&); // Disable assignment operator

        CWnd* m_pView;              // pointer to the View CWnd object
        CMenu m_ChildMenu;
        HACCEL m_hChildAccel;
    };

    /////////////////////////////////////
    // The CMDIClient class provides the functionality of a MDI client window.
    // This is used as the view window for a CMDIFrame window.
    template <class T>
    class CMDIClient : public T     // The template parameter T is either CWnd, or CDocker::CDockClient
    {
    public:
        CMDIClient() {}
        virtual ~CMDIClient() {}

    protected:
        // Overridable virtual functions
        virtual HWND Create(HWND hWndParent);
        virtual LRESULT OnMDIActivate(UINT uMsg, WPARAM wParam, LPARAM lParam);
        virtual LRESULT OnMDIDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam);
        virtual LRESULT OnMDIGetActive(UINT uMsg, WPARAM wParam, LPARAM lParam);
        virtual LRESULT WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
    };

    /////////////////////////////////////
    // The CMDIFrameT class is the base class for all MDI frames. MDI Frames can hold
    // one or more MDI children. The template parameter T is typically either CWnd or CDocker.
    template <class T>
    class CMDIFrameT : public T
    {
    public:
        CMDIFrameT();
        virtual ~CMDIFrameT() {}

        // Overridable virtual functions
        virtual CMDIChild* AddMDIChild(CMDIChild* pMDIChild);
        virtual CMDIChild* GetActiveMDIChild() const;
        virtual CMenu GetActiveMenu() const;
        virtual CWnd& GetMDIClient() const { return m_MDIClient; }
        virtual BOOL IsMDIChildMaxed() const;
        virtual BOOL IsMDIFrame() const { return TRUE; }
        virtual void RemoveMDIChild(HWND hWnd);
        virtual BOOL RemoveAllMDIChildren();

        // These functions aren't virtual. Don't override these
        const std::vector<MDIChildPtr>& GetAllMDIChildren() const { return m_vMDIChild; }
        void MDICascade(int nType = 0) const;
        void MDIIconArrange() const;
        void MDIMaximize() const;
        void MDINext() const;
        void MDIPrev() const;
        void MDIRestore() const;
        void MDITile(int nType = 0) const;
        void SetActiveMDIChild(CMDIChild* pChild);

    protected:
        // Overridable virtual functions
        virtual void    OnClose();
        virtual LRESULT OnInitMenuPopup(UINT uMsg, WPARAM wParam, LPARAM lParam);
        virtual LRESULT OnMDIActivated(UINT uMsg, WPARAM wParam, LPARAM);
        virtual LRESULT OnMDIDestroyed(UINT uMsg, WPARAM wParam, LPARAM);
        virtual void    OnMDIMaximized(BOOL IsMaxed);
        virtual void    OnMenuUpdate(UINT nID);
        virtual BOOL    OnViewStatusBar();
        virtual BOOL    OnViewToolBar();
        virtual LRESULT OnWindowPosChanged(UINT uMsg, WPARAM wParam, LPARAM lParam);
        virtual BOOL    PreTranslateMessage(MSG& Msg);
        virtual void    RecalcLayout();
        virtual void    UpdateFrameMenu(CMenu Menu);

        // Not intended to be overridden
        LRESULT FinalWindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
        LRESULT WndProcDefault(UINT uMsg, WPARAM wParam, LPARAM lParam);

    private:
        CMDIFrameT(const CMDIFrameT&);              // Disable copy construction
        CMDIFrameT& operator = (const CMDIFrameT&); // Disable assignment operator
        void AppendMDIMenu(CMenu MenuWindow);

        std::vector<MDIChildPtr> m_vMDIChild;
        mutable CMDIClient<CWnd> m_MDIClient;
    };


    /////////////////////////////////////////
    // The CMDIFrame class provides a frame window that can host one or more MDI child windows.
    class CMDIFrame : public CMDIFrameT<CFrame>
    {
    public:
        CMDIFrame() {}
        virtual ~CMDIFrame() {}
    };

}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


namespace Win32xx
{

    /////////////////////////////////////
    // Definitions for the CMDIFrameT class
    //

    template <class T>
    inline CMDIFrameT<T>::CMDIFrameT()
    {
        // The view for a CMDIFrame is the MDIClient
        T::SetView(GetMDIClient());
    }


    // Adds a MDI child to the MDI frame. The pointer to the MDI child will be
    // automatically deleted when the MDI Frame destroys the MDI child.
    template <class T>
    inline CMDIChild* CMDIFrameT<T>::AddMDIChild(CMDIChild* pMDIChild)
    {
        assert(NULL != pMDIChild); // Cannot add Null MDI Child

        m_vMDIChild.push_back(MDIChildPtr(pMDIChild));
        pMDIChild->Create(GetMDIClient());

        return pMDIChild;
    }


    // Adds the additional menu items the the "Window" submenu when
    // MDI child windows are created.
    template <class T>
    inline void CMDIFrameT<T>::AppendMDIMenu(CMenu MenuWindow)
    {
        if (!MenuWindow.GetHandle())
            return;

        // Delete previously appended items
        int nItems = MenuWindow.GetMenuItemCount();
        UINT uLastID = MenuWindow.GetMenuItemID(--nItems);
        if ((uLastID >= IDW_FIRSTCHILD) && (uLastID < IDW_FIRSTCHILD + 10))
        {
            while ((uLastID >= IDW_FIRSTCHILD) && (uLastID < IDW_FIRSTCHILD + 10))
            {
                MenuWindow.DeleteMenu(nItems, MF_BYPOSITION);
                uLastID = MenuWindow.GetMenuItemID(--nItems);
            }

            //delete the separator too
            MenuWindow.DeleteMenu(nItems, MF_BYPOSITION);
        }

        int nWindow = 0;

        // Allocate an iterator for our MDIChild vector
        std::vector<MDIChildPtr>::const_iterator v;

        for (v = GetAllMDIChildren().begin(); v < GetAllMDIChildren().end(); ++v)
        {
            if ((*v)->IsWindowVisible())
            {
                // Add Separator
                if (nWindow == 0)
                    MenuWindow.AppendMenu(MF_SEPARATOR, 0, reinterpret_cast<LPCTSTR>(NULL));

                // Add a menu entry for each MDI child (up to 9)
                if (nWindow < 9)
                {
                    CString strMenuItem ( (*v)->GetWindowText() );

                    if (strMenuItem.GetLength() > MAX_MENU_STRING -10)
                    {
                        // Truncate the string if its too long
                        strMenuItem.Delete(strMenuItem.GetLength() - MAX_MENU_STRING +10);
                        strMenuItem += _T(" ...");
                    }

                    CString MenuString;
                    MenuString.Format(_T("&%d %s"), nWindow+1, strMenuItem.c_str());

                    MenuWindow.AppendMenu(MF_STRING, IDW_FIRSTCHILD + nWindow, MenuString);

                    if (GetActiveMDIChild() == (*v).get())
                        MenuWindow.CheckMenuItem(IDW_FIRSTCHILD+nWindow, MF_CHECKED);

                    ++nWindow;
                }
                else if (9 == nWindow)
                // For the 10th MDI child, add this menu item and return
                {
                    MenuWindow.AppendMenu(MF_STRING, IDW_FIRSTCHILD + nWindow, _T("&Windows..."));
                    return;
                }
            }
        }
    }


    // Returns the menu of the Active MDI Child if any,
    // otherwise returns the MDI Frame's menu.
    template <class T>
    inline CMenu CMDIFrameT<T>::GetActiveMenu() const
    {
        CMenu Menu = T::GetFrameMenu();

        if(GetActiveMDIChild())
            if (GetActiveMDIChild()->GetChildMenu())
                Menu = GetActiveMDIChild()->GetChildMenu();

        return Menu;
    }


    // Overrides CWnd::FinalWindowProc to call DefFrameProc instead of DefWindowProc.
    template <class T>
    inline LRESULT CMDIFrameT<T>::FinalWindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        return ::DefFrameProc(*this, GetMDIClient(), uMsg, wParam, lParam);
    }


    // Returns a pointer to the active MDI child, or NULL if there is no active MDI child.
    template <class T>
    inline CMDIChild* CMDIFrameT<T>::GetActiveMDIChild() const
    {
        HWND hActiveChild = reinterpret_cast<HWND>(GetMDIClient().SendMessage(WM_MDIGETACTIVE, 0L, 0L));
        return static_cast<CMDIChild*>(T::GetCWndPtr(hActiveChild));
    }

    // Returns TRUE if a MDI child is maximized
    template <class T>
    inline BOOL CMDIFrameT<T>::IsMDIChildMaxed() const
    {
        BOOL IsMaxed = FALSE;
        GetMDIClient().SendMessage(WM_MDIGETACTIVE, 0L, reinterpret_cast<LPARAM>(&IsMaxed));
        return IsMaxed;
    }


    // Arranges the MDI children in a cascade formation.
    // Possible values for nType are:
    // MDITILE_SKIPDISABLED Prevents disabled MDI child windows from being cascaded.
    template <class T>
    inline void CMDIFrameT<T>::MDICascade(int nType /* = 0*/) const
    {
        assert(T::IsWindow());
        GetMDIClient().SendMessage(WM_MDICASCADE, nType, 0L);
    }


    // Re-arranges the icons for minimized MDI children.
    template <class T>
    inline void CMDIFrameT<T>::MDIIconArrange() const
    {
        assert(T::IsWindow());
        GetMDIClient().SendMessage(WM_MDIICONARRANGE, 0L, 0L);
    }


    // Maximize the MDI child.
    template <class T>
    inline void CMDIFrameT<T>::MDIMaximize() const
    {
        assert(T::IsWindow());
        WPARAM hMDIChild = GetMDIClient().SendMessage(WM_MDIGETACTIVE, 0L, 0L);
        GetMDIClient().SendMessage(WM_MDIMAXIMIZE, hMDIChild, 0L);
    }


    // Activates the next MDI child.
    template <class T>
    inline void CMDIFrameT<T>::MDINext() const
    {
        assert(T::IsWindow());
        WPARAM hMDIChild = GetMDIClient().SendMessage(WM_MDIGETACTIVE, 0L, 0L);
        GetMDIClient().SendMessage(WM_MDINEXT, hMDIChild, FALSE);
    }


    // Activates the previous MDI child.
    template <class T>
    inline void CMDIFrameT<T>::MDIPrev() const
    {
        assert(T::IsWindow());
        WPARAM hMDIChild = GetMDIClient().SendMessage(WM_MDIGETACTIVE, 0L, 0L);
        GetMDIClient().SendMessage(WM_MDINEXT, hMDIChild, TRUE);
    }


    // Restores a mimimized MDI child.
    template <class T>
    inline void CMDIFrameT<T>::MDIRestore() const
    {
        assert(T::IsWindow());
        WPARAM hMDIChild = GetMDIClient().SendMessage(WM_MDIGETACTIVE, 0L, 0L);
        GetMDIClient().SendMessage(WM_MDIRESTORE, hMDIChild, 0L);
    }


    // Arrange all of the MDI child windows in a tile format.
    // Possible values for nType are:
    // MDITILE_HORIZONTAL   Tiles MDI child windows so that one window appears above another.
    // MDITILE_SKIPDISABLED Prevents disabled MDI child windows from being tiled.
    // MDITILE_VERTICAL     Tiles MDI child windows so that one window appears beside another.
    template <class T>
    inline void CMDIFrameT<T>::MDITile(int nType /* = 0*/) const
    {


        assert(T::IsWindow());
        GetMDIClient().SendMessage(WM_MDITILE, nType, 0L);
    }


    // Called when the MDI frame is about to close.
    template <class T>
    inline void CMDIFrameT<T>::OnClose()
    {
        if (RemoveAllMDIChildren())
        {
            T::OnClose();
        }
    }


    // Called when the menu's modal loop begins (WM_INITMENUPOPUP received).
    template <class T>
    inline LRESULT CMDIFrameT<T>::OnInitMenuPopup(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        if (IsMDIChildMaxed())
        {
            CWnd* pMaxMDIChild = GetActiveMDIChild();
            assert(pMaxMDIChild);

            // Suppress owner drawing of the MDI child's system menu
            if (::GetSystemMenu(*pMaxMDIChild, FALSE) == reinterpret_cast<HMENU>(wParam))
                return CWnd::WndProcDefault(WM_INITMENUPOPUP, wParam, lParam);
        }

        return T::OnInitMenuPopup(uMsg, wParam, lParam);
    }


    // Updates the check buttons before displaying the menu.
    template <class T>
    inline void CMDIFrameT<T>::OnMenuUpdate(UINT nID)
    {
        switch (nID)
        {
        case IDW_VIEW_STATUSBAR:
            {
                BOOL IsVisible = T::GetStatusBar().IsWindow() && T::GetStatusBar().IsWindowVisible();
                GetActiveMenu().CheckMenuItem(nID, IsVisible ? MF_CHECKED : MF_UNCHECKED);
            }
            break;

        case IDW_VIEW_TOOLBAR:
            {
                BOOL IsVisible = T::GetToolBar().IsWindow() && T::GetToolBar().IsWindowVisible();
                GetActiveMenu().EnableMenuItem(nID, T::GetUseToolBar() ? MF_ENABLED : MF_DISABLED);
                GetActiveMenu().CheckMenuItem(nID, IsVisible ? MF_CHECKED : MF_UNCHECKED);
            }
            break;
        }
    }


    // Called when a MDI child is activated.
    template <class T>
    inline LRESULT CMDIFrameT<T>::OnMDIActivated(UINT, WPARAM wParam, LPARAM)
    {
        HWND hActiveMDIChild = reinterpret_cast<HWND>(wParam);

        if (hActiveMDIChild)
        {
            CMDIChild* pMDIChild = static_cast<CMDIChild*>(T::GetCWndPtr(hActiveMDIChild));
            assert ( dynamic_cast<CMDIChild*>(T::GetCWndPtr(hActiveMDIChild)) );
            if (pMDIChild->GetChildMenu())
                UpdateFrameMenu(pMDIChild->GetChildMenu());
            else
                UpdateFrameMenu(T::GetFrameMenu());
            if (pMDIChild->GetChildAccel())
                GetApp().SetAccelerators(pMDIChild->GetChildAccel(), *this);
            else
                GetApp().SetAccelerators(T::GetFrameAccel(), *this);
        }

        return 0L;
    }


    // Called when a MDI child is destroyed.
    template <class T>
    inline LRESULT CMDIFrameT<T>::OnMDIDestroyed(UINT, WPARAM wParam, LPARAM)
    {
        RemoveMDIChild(reinterpret_cast<HWND>(wParam));
        return 0L;
    }


    // Called when a MDI Child maximized state of the MDI child is checked.
    template <class T>
    inline void CMDIFrameT<T>::OnMDIMaximized(BOOL IsMax)
    {
        // Override this function to take an action when a MDI child is
        // maximized or restored from maximized.

        UNREFERENCED_PARAMETER(IsMax);
    }


    // Called when the StatusBar is shown.
    template <class T>
    inline BOOL CMDIFrameT<T>::OnViewStatusBar()
    {
        T::OnViewStatusBar();
        T::GetView().RedrawWindow(RDW_FRAME | RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN);
        return TRUE;
    }


    // Called when the ToolBar window is shown.
    template <class T>
    inline BOOL CMDIFrameT<T>::OnViewToolBar()
    {
        T::OnViewToolBar();
        T::GetView().RedrawWindow(RDW_FRAME | RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN);
        return TRUE;
    }


    // Called when the MDI frame has been resized.
    template <class T>
    inline LRESULT CMDIFrameT<T>::OnWindowPosChanged(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        if (T::IsMenuBarUsed())
        {
            // Refresh MenuBar Window
            T::GetMenuBar().SetMenu(T::GetMenuBar().GetMenu());
        }

        return FinalWindowProc(uMsg, wParam, lParam);
    }


    // Used to translate window messages before they are dispatched to they are handled in
    // the message loop.
    template <class T>
    inline BOOL CMDIFrameT<T>::PreTranslateMessage(MSG& Msg)
    {
        if (WM_KEYFIRST <= Msg.message && Msg.message <= WM_KEYLAST)
        {
            if (TranslateMDISysAccel(T::GetView().GetHwnd(), &Msg))
                return TRUE;
        }

        return T::PreTranslateMessage(Msg);
    }


    // Repositions the child windows of the MDI frame, such as the toolbar, status bar and view window.
    template <class T>
    inline void CMDIFrameT<T>::RecalcLayout()
    {
        T::RecalcLayout();

        if (T::GetView().IsWindow())
            MDIIconArrange();
    }


    // Removes all MDI children.
    template <class T>
    inline BOOL CMDIFrameT<T>::RemoveAllMDIChildren()
    {
        BOOL Succeeded = TRUE;
        int Children = static_cast<int>(m_vMDIChild.size());

        // Remove the children in reverse order
        for (int i = Children-1; i >= 0; --i)
        {
            MDIChildPtr pMDIChild = m_vMDIChild[i];

            // Ask the window to close. If it is destroyed, RemoveMDIChild gets called.
            pMDIChild->SendMessage(WM_SYSCOMMAND, SC_CLOSE, 0L);

            if (pMDIChild->IsWindow())
                Succeeded = FALSE;
        }

        return Succeeded;
    }


    // Removes an individual MDI child.
    template <class T>
    inline void CMDIFrameT<T>::RemoveMDIChild(HWND hWnd)
    {
        // Allocate an iterator for our HWND map
        std::vector<MDIChildPtr>::iterator v;

        for (v = m_vMDIChild.begin(); v!= m_vMDIChild.end(); ++v)
        {
            if ((*v)->GetHwnd() == hWnd)
            {
                m_vMDIChild.erase(v);
                break;
            }
        }

        if (GetActiveMDIChild())
        {
            // Update the "Window" menu items
            if (GetActiveMDIChild()->GetChildMenu())
                UpdateFrameMenu(GetActiveMDIChild()->GetChildMenu());

            // Update the accelerators
            if (GetActiveMDIChild()->GetChildAccel())
                GetApp().SetAccelerators(GetActiveMDIChild()->GetChildAccel(), *this);
            else
                GetApp().SetAccelerators(T::GetFrameAccel(), *this);
        }
        else
        {
            if (T::IsMenuBarUsed())
                T::GetMenuBar().SetMenu( T::GetFrameMenu() );
            else
                T::SetMenu( T::GetFrameMenu() );

            GetApp().SetAccelerators(T::GetFrameAccel(), *this);
        }
    }


    // Activates the specified MDI child.
    template <class T>
    inline void CMDIFrameT<T>::SetActiveMDIChild(CMDIChild* pChild)
    {
        assert ( pChild->IsWindow() );

        GetMDIClient().SendMessage(WM_MDIACTIVATE, reinterpret_cast<WPARAM>(pChild->GetHwnd()), 0L);
    }


    // Updates the frame menu. Also puts the list of CMDIChild windows in the "window" menu.
    // The "window" menu item is assumed to be the second from the right.
    template <class T>
    inline void CMDIFrameT<T>::UpdateFrameMenu(CMenu Menu)
    {
        if (Menu.GetHandle())
        {
            int nMenuItems = Menu.GetMenuItemCount();
            if (nMenuItems > 0)
            {
                // The Window menu is typically second from the right
                int nWindowItem = MAX(nMenuItems - 2, 0);
                CMenu MenuWindow = Menu.GetSubMenu(nWindowItem);

                if (MenuWindow.GetHandle())
                {
                    if (T::IsMenuBarUsed())
                    {
                        AppendMDIMenu(MenuWindow);
                        T::GetMenuBar().SetMenu(Menu);
                    }
                    else
                    {
                        GetMDIClient().SendMessage(WM_MDISETMENU, reinterpret_cast<WPARAM>(Menu.GetHandle()),
                            reinterpret_cast<LPARAM>(MenuWindow.GetHandle()));
                        T::DrawMenuBar();
                    }
                }
            }
        }
    }


    // Provides default processing for the MDI frame window's messages.
    template <class T>
    inline LRESULT CMDIFrameT<T>::WndProcDefault(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        switch (uMsg)
        {
            case UWM_MDIACTIVATED:      return OnMDIActivated(uMsg, wParam, lParam);
            case UWM_MDIDESTROYED:      return OnMDIDestroyed(uMsg, wParam, lParam);
            case UWM_MDIGETACTIVE:
            {
                if (wParam && lParam)
                {
                    LPBOOL pIsMDIChildMax = (LPBOOL)lParam;
                    OnMDIMaximized(*pIsMDIChildMax);
                }
                break;

            }
            case WM_WINDOWPOSCHANGED:   return OnWindowPosChanged(uMsg, wParam, lParam);
        }

        return T::WndProcDefault(uMsg, wParam, lParam);
    }


    //////////////////////////////////////
    //Definitions for the CMDIClient class
    //

    // Creates the MDIClient window.
    template<class T>
    inline HWND CMDIClient<T>::Create(HWND hWndParent)
    {
        assert(hWndParent != 0);

        CLIENTCREATESTRUCT clientcreate;
        clientcreate.hWindowMenu  = 0;
        clientcreate.idFirstChild = IDW_FIRSTCHILD;
        DWORD dwStyle = WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | MDIS_ALLCHILDSTYLES;

        // Create the view window
        return T::CreateEx(WS_EX_CLIENTEDGE, _T("MDIClient"), _T(""), dwStyle, 0, 0, 0, 0, hWndParent, NULL, (PSTR) &clientcreate);
    }


    // Called when the MDI child window is activated.
    template <class T>
    inline LRESULT CMDIClient<T>::OnMDIActivate(UINT, WPARAM wParam, LPARAM lParam)
    {
        // Suppress redraw to avoid flicker when activating maximised MDI children
        T::SetRedraw(FALSE);
        LRESULT lr = T::CallWindowProc(T::GetPrevWindowProc(), WM_MDIACTIVATE, wParam, lParam);
        T::SetRedraw(TRUE);
        T::RedrawWindow(RDW_FRAME | RDW_INVALIDATE | RDW_ALLCHILDREN);

        return lr;
    }


    // Called when a MDI child window is destroyed.
    template <class T>
    inline LRESULT CMDIClient<T>::OnMDIDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        // Do default processing first
        T::CallWindowProc(T::GetPrevWindowProc(), uMsg, wParam, lParam);

        // Now remove MDI child
        T::GetParent().SendMessage(UWM_MDIDESTROYED, wParam, 0);

        return 0L;
    }


    // Called when the active MDI child is requested.
    template <class T>
    inline LRESULT CMDIClient<T>::OnMDIGetActive(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        // Do default processing first
        LRESULT lr = T::CallWindowProc(T::GetPrevWindowProc(), uMsg, wParam, lParam);

        SendMessage(T::GetParent(), UWM_MDIGETACTIVE, lr, lParam);
        return lr;
    }


    // Provides default processing for the MDIClient window's messages.
    template <class T>
    inline LRESULT CMDIClient<T>::WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        switch (uMsg)
        {
        case WM_MDIACTIVATE:    return OnMDIActivate(uMsg, wParam, lParam);
        case WM_MDIDESTROY:     return OnMDIDestroy(uMsg, wParam, lParam);
        case WM_MDIGETACTIVE:   return OnMDIGetActive(uMsg, wParam, lParam);
        }

        return CWnd::WndProcDefault(uMsg, wParam, lParam);
    }


    /////////////////////////////////////
    //Definitions for the CMDIChild class
    //

    // Sets the MDI Child's menu and accelerator in the constructor, like this ...
    //   HMENU hChildMenu = LoadMenu(GetApp().GetResourceHandle(), _T("MdiMenuView"));
    //   HACCEL hChildAccel = LoadAccelerators(GetApp().GetResourceHandle(), _T("MDIAccelView"));
    //   SetHandles(hChildMenu, hChildAccel);
    //   SetView(m_View);
    inline CMDIChild::CMDIChild() : m_pView(NULL), m_hChildAccel(0)
    {

    }


    inline CMDIChild::~CMDIChild()
    {
        if (IsWindow())
            GetParent().SendMessage(WM_MDIDESTROY, reinterpret_cast<WPARAM>(GetHwnd()), 0L);
    }


    // Create the MDI child window and then maximize if required.
    // This technique avoids unnecessary flicker when creating maximized MDI children.
    inline HWND CMDIChild::Create(HWND hWndParent /*= NULL*/)
    {
        CREATESTRUCT cs;
        WNDCLASS wc;

        ZeroMemory(&cs, sizeof(cs));
        ZeroMemory(&wc, sizeof(wc));

        //Call PreCreate in case its overloaded
        PreCreate(cs);

        //Determine if the window should be created maximized
        BOOL Max = FALSE;
        CWnd* pParent = GetCWndPtr(hWndParent);
        assert(pParent);
        pParent->SendMessage(WM_MDIGETACTIVE, 0L, reinterpret_cast<LPARAM>(&Max));
        Max = Max | (cs.style & WS_MAXIMIZE);

        // Set the Window Class Name
        CString ClassName = _T("Win32++ MDI Child");
        if (cs.lpszClass)
            ClassName = cs.lpszClass;

        // Set the window style
        DWORD dwStyle;
        dwStyle = cs.style & ~WS_MAXIMIZE;
        dwStyle |= WS_VISIBLE | WS_OVERLAPPEDWINDOW ;

        // Set window size and position
        int x = CW_USEDEFAULT;
        int y = CW_USEDEFAULT;
        int cx = CW_USEDEFAULT;
        int cy = CW_USEDEFAULT;
        if(cs.cx && cs.cy)
        {
            x = cs.x;
            y = cs.y;
            cx = cs.cx;
            cy = cs.cy;
        }

        // Set the extended style
        DWORD dwExStyle = cs.dwExStyle | WS_EX_MDICHILD;

        // Turn off redraw while creating the window
        pParent->SetRedraw(FALSE);

        // Create the window
        CreateEx(dwExStyle, ClassName, cs.lpszName, dwStyle, x, y,
            cx, cy, pParent->GetHwnd(), cs.hMenu, cs.lpCreateParams);

        if (Max)
            ShowWindow(SW_MAXIMIZE);

        // Turn redraw back on
        pParent->SetRedraw(TRUE);
        pParent->RedrawWindow(RDW_INVALIDATE | RDW_ALLCHILDREN);

        // Ensure bits revealed by round corners (XP themes) are redrawn
        SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_FRAMECHANGED);

        return GetHwnd();
    }


    // Overrides CWnd::FinalWindowProc to call DefMDIChildProc instead of DefWindowProc.
    inline LRESULT CMDIChild::FinalWindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        return ::DefMDIChildProc(*this, uMsg, wParam, lParam);
    }


    // Activate a MDI child.
    inline void CMDIChild::MDIActivate() const
    {
        GetParent().SendMessage(WM_MDIACTIVATE, reinterpret_cast<WPARAM>(GetHwnd()), 0L);
    }


    // Destroy a MDI child.
    inline void CMDIChild::MDIDestroy() const
    {
        GetParent().SendMessage(WM_MDIDESTROY, reinterpret_cast<WPARAM>(GetHwnd()), 0L);
    }


    // Maximize a MDI child.
    inline void CMDIChild::MDIMaximize() const
    {
        GetParent().SendMessage(WM_MDIMAXIMIZE, reinterpret_cast<WPARAM>(GetHwnd()), 0L);
    }


    // Restore a MDI child.
    inline void CMDIChild::MDIRestore() const
    {
        GetParent().SendMessage(WM_MDIRESTORE, reinterpret_cast<WPARAM>(GetHwnd()), 0L);
    }


    // Override this to customise what happens when the window asks to be closed.
    inline void CMDIChild::OnClose()
    {
        MDIDestroy();
    }


    // Called when the MDI child is created.
    inline int CMDIChild::OnCreate(CREATESTRUCT& cs)
    {
        UNREFERENCED_PARAMETER(cs);

        // Create the view window
        assert( &GetView() );           // Use SetView in CMDIChild's constructor to set the view window
        GetView().Create(*this);
        GetView().SetFocus();
        RecalcLayout();

        return 0;
    }


    // Called when the MDI child is activated.
    inline LRESULT CMDIChild::OnMDIActivate(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        UNREFERENCED_PARAMETER(uMsg);
        UNREFERENCED_PARAMETER(wParam);

        // This child is being activated
        if (lParam == reinterpret_cast<LPARAM>(GetHwnd()))
        {
            GetAncestor().SendMessage(UWM_MDIACTIVATED, reinterpret_cast<WPARAM>(GetHwnd()), 0);
            GetView().SetFocus();
        }

        // No child is being activated
        if (lParam == 0)
        {
            GetAncestor().SendMessage(UWM_MDIACTIVATED, 0, 0);
        }

        return 0L;
    }


    // Called when the MDI child is resized.
    inline LRESULT CMDIChild::OnWindowPosChanged(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        RecalcLayout();
        return FinalWindowProc(uMsg, wParam, lParam);
    }


    // Repositions the MDI child's child windows.
    inline void CMDIChild::RecalcLayout()
    {
        // Resize the View window
        CRect rc = GetClientRect();
        GetView().SetWindowPos( NULL, rc.left, rc.top, rc.Width(), rc.Height(), SWP_SHOWWINDOW );
    }


    // Sets the MDI child's menu and accelerator handles.
    inline void CMDIChild::SetHandles(HMENU hMenu, HACCEL hAccel)
    {
        m_ChildMenu.Attach(hMenu);
        m_hChildAccel = hAccel;

        // Note: It is valid to call SetHandles before the window is created
        if (IsWindow())
        {
            GetParent().SendMessage(WM_MDIACTIVATE, reinterpret_cast<WPARAM>(GetHwnd()), 0L);
            GetAncestor().SendMessage(UWM_MDIACTIVATED, reinterpret_cast<WPARAM>(GetHwnd()), 0L);
        }
    }


    // Sets or changes the MDI child's view window.
    inline void CMDIChild::SetView(CWnd& wndView)
    {
        if (m_pView != &wndView)
        {
            // Hide the existing view window (if any)
            if (m_pView && m_pView->IsWindow())
                m_pView->ShowWindow(SW_HIDE);

            // Assign the view window
            m_pView = &wndView;

            if (GetHwnd() != 0)
            {
                if (!GetView().IsWindow())
                    GetView().Create(*this);
                else
                {
                    GetView().SetParent(*this);
                    GetView().ShowWindow();
                }

                RecalcLayout();
            }
        }
    }


    // Provides the default processing for the MDI child's window messages.
    inline LRESULT CMDIChild::WndProcDefault(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        switch (uMsg)
        {
            case WM_MDIACTIVATE:        return OnMDIActivate(uMsg, wParam, lParam);
            case WM_WINDOWPOSCHANGED:   return OnWindowPosChanged(uMsg, wParam, lParam);
        }
        return CWnd::WndProcDefault(uMsg, wParam, lParam);
    }


} // namespace Win32xx

#endif // _WIN32XX_MDI_H_

