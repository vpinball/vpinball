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

    ///////////////////////////////////////////////////////////////
    // CMDIChild manages a MDI child window. CMDIChild also manages
    // the creation and position of the MDI Child's view window.
    class CMDIChild : public CWnd
    {
    public:
        CMDIChild();
        virtual ~CMDIChild();

        // These are the functions you might wish to override
        virtual HWND Create(HWND parent = 0);
        virtual void RecalcLayout();

        // These functions aren't virtual, and shouldn't be overridden
        HACCEL GetChildAccel() const { return m_childAccel; }
        HMENU GetChildMenu() const { return m_childMenu; }
        CWnd& GetView() const;
        void SetView(CWnd& view);
        void SetHandles(HMENU menu, HACCEL accel);
        void MDIActivate() const;
        void MDIDestroy() const;
        void MDIMaximize() const;
        void MDIRestore() const;

    protected:
        // These are the functions you might wish to override
        virtual void OnClose();
        virtual int  OnCreate(CREATESTRUCT& cs);
        virtual LRESULT OnMDIActivate(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual LRESULT OnWindowPosChanged(UINT msg, WPARAM wparam, LPARAM lparam);

        // Not intended to be overridden
        LRESULT FinalWindowProc(UINT msg, WPARAM wparam, LPARAM lparam);
        LRESULT WndProcDefault(UINT msg, WPARAM wparam, LPARAM lparam);

    private:
        CMDIChild(const CMDIChild&);              // Disable copy construction
        CMDIChild& operator = (const CMDIChild&); // Disable assignment operator

        CWnd* m_pView;              // pointer to the View CWnd object
        CMenu m_childMenu;
        HACCEL m_childAccel;
    };

    /////////////////////////////////////
    // CMDIClient manages the MDI frame's MDI client window.
    // The MDI client window manages the arrangement of the MDI child windows.
    template <class T>
    class CMDIClient : public T     // The template parameter T is either CWnd, or CDocker::CDockClient
    {
    public:
        CMDIClient() {}
        virtual ~CMDIClient() {}

    protected:
        // Overridable virtual functions
        virtual HWND Create(HWND parent);
        virtual LRESULT OnMDIActivate(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual LRESULT OnMDIDestroy(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual LRESULT OnMDIGetActive(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual LRESULT WndProc(UINT msg, WPARAM wparam, LPARAM lparam);
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

        // Override these functions as required.
        virtual CMDIChild* AddMDIChild(CMDIChild* pMDIChild);
        virtual HWND Create(HWND parent = 0);
        virtual CWnd& GetMDIClient() const { return *m_pMdiClient; }
        virtual BOOL IsMDIFrame() const { return TRUE; }
        virtual void RemoveMDIChild(HWND wnd);
        virtual BOOL RemoveAllMDIChildren();

        // These functions aren't virtual. Don't override these.
        CMDIChild* GetActiveMDIChild() const;
        CMenu GetActiveMenu() const;
        const std::vector<MDIChildPtr>& GetAllMDIChildren() const { return m_mdiChildren; }
        BOOL IsMDIChildMaxed() const;
        void MDICascade(int nType = 0) const;
        void MDIIconArrange() const;
        void MDIMaximize() const;
        void MDINext() const;
        void MDIPrev() const;
        void MDIRestore() const;
        void MDITile(int nType = 0) const;
        void SetActiveMDIChild(CMDIChild* pChild);
        void SetMDIClient(CMDIClient<CWnd>& mdiClient) { m_pMdiClient = &mdiClient; }

    protected:
        // Overridable virtual functions
        virtual void    OnClose();
        virtual LRESULT OnInitMenuPopup(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual LRESULT OnMDIActivated(UINT msg, WPARAM wparam, LPARAM);
        virtual LRESULT OnMDIDestroyed(UINT msg, WPARAM wparam, LPARAM);
        virtual void    OnMDIMaximized(BOOL isMaxed);
        virtual void    OnMenuUpdate(UINT id);
        virtual BOOL    OnViewStatusBar();
        virtual BOOL    OnViewToolBar();
        virtual LRESULT OnWindowPosChanged(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual BOOL    PreTranslateMessage(MSG& msg);
        virtual void    RecalcLayout();
        virtual void    UpdateFrameMenu(CMenu menu);

        // Not intended to be overridden
        LRESULT FinalWindowProc(UINT msg, WPARAM wparam, LPARAM lparam);
        LRESULT WndProcDefault(UINT msg, WPARAM wparam, LPARAM lparam);

    private:
        CMDIFrameT(const CMDIFrameT&);              // Disable copy construction
        CMDIFrameT& operator = (const CMDIFrameT&); // Disable assignment operator
        void AppendMDIMenu(CMenu menuWindow);

        std::vector<MDIChildPtr> m_mdiChildren;
        CMDIClient<CWnd> m_mdiClient;
        CMDIClient<CWnd>* m_pMdiClient;
    };


    /////////////////////////////////////////
    // The CMDIFrame class provides a frame window that can host one or
    // more MDI child windows. CMDIFrame also manages the creation and
    // position of child windows, such as the menubar, toolbar, and statusbar.
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
        SetMDIClient(m_mdiClient);
    }

    // Create the MDI frame.
    template <class T>
    inline HWND CMDIFrameT<T>::Create(HWND parent /* = 0 */)
    {
        // The view for a CMDIFrame is the MDIClient.
        T::SetView(GetMDIClient());

        return T::Create(parent);
    }

    // Adds a MDI child to the MDI frame. The pointer to the MDI child will be
    // automatically deleted when the MDI Frame destroys the MDI child.
    template <class T>
    inline CMDIChild* CMDIFrameT<T>::AddMDIChild(CMDIChild* pMDIChild)
    {
        assert(NULL != pMDIChild); // Cannot add Null MDI Child

        m_mdiChildren.push_back(MDIChildPtr(pMDIChild));
        pMDIChild->Create(GetMDIClient());

        return pMDIChild;
    }

    // Adds the additional menu items the the "Window" submenu when
    // MDI child windows are created.
    template <class T>
    inline void CMDIFrameT<T>::AppendMDIMenu(CMenu windowMenu)
    {
        if (!windowMenu.GetHandle())
            return;

        // Delete previously appended items
        int items = windowMenu.GetMenuItemCount();
        UINT lastID = windowMenu.GetMenuItemID(--items);
        if ((lastID >= IDW_FIRSTCHILD) && (lastID < IDW_FIRSTCHILD + 10))
        {
            while ((lastID >= IDW_FIRSTCHILD) && (lastID < IDW_FIRSTCHILD + 10))
            {
                windowMenu.DeleteMenu(items, MF_BYPOSITION);
                lastID = windowMenu.GetMenuItemID(--items);
            }

            //delete the separator too
            windowMenu.DeleteMenu(items, MF_BYPOSITION);
        }

        int window = 0;

        // Allocate an iterator for our MDIChild vector
        std::vector<MDIChildPtr>::const_iterator v;

        for (v = GetAllMDIChildren().begin(); v < GetAllMDIChildren().end(); ++v)
        {
            if ((*v)->IsWindowVisible())
            {
                // Add Separator
                if (window == 0)
                    windowMenu.AppendMenu(MF_SEPARATOR, 0, reinterpret_cast<LPCTSTR>(NULL));

                // Add a menu entry for each MDI child (up to 9)
                if (window < 9)
                {
                    CString strMenuItem ( (*v)->GetWindowText() );

                    if (strMenuItem.GetLength() > WXX_MAX_STRING_SIZE -10)
                    {
                        // Truncate the string if its too long
                        strMenuItem.Delete(strMenuItem.GetLength() - WXX_MAX_STRING_SIZE -10);
                        strMenuItem += _T(" ...");
                    }

                    CString menuString;
                    menuString.Format(_T("&%d %s"), window+1, strMenuItem.c_str());

                    windowMenu.AppendMenu(MF_STRING, IDW_FIRSTCHILD + UINT_PTR(window), menuString);

                    if (GetActiveMDIChild() == (*v).get())
                        windowMenu.CheckMenuItem(IDW_FIRSTCHILD + window, MF_CHECKED);

                    ++window;
                }
                else if (9 == window)
                // For the 10th MDI child, add this menu item and return
                {
                    windowMenu.AppendMenu(MF_STRING, IDW_FIRSTCHILD + UINT_PTR(window), _T("&Windows..."));
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
        CMenu menu = T::GetFrameMenu();

        if (GetActiveMDIChild())
            if (GetActiveMDIChild()->GetChildMenu())
                menu = GetActiveMDIChild()->GetChildMenu();

        return menu;
    }

    // Overrides CWnd::FinalWindowProc to call DefFrameProc instead of DefWindowProc.
    template <class T>
    inline LRESULT CMDIFrameT<T>::FinalWindowProc(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        return ::DefFrameProc(*this, GetMDIClient(), msg, wparam, lparam);
    }

    // Returns a pointer to the active MDI child, or NULL if there is no active MDI child.
    template <class T>
    inline CMDIChild* CMDIFrameT<T>::GetActiveMDIChild() const
    {
        HWND activeChild = reinterpret_cast<HWND>(GetMDIClient().SendMessage(WM_MDIGETACTIVE, 0, 0));
        return static_cast<CMDIChild*>(T::GetCWndPtr(activeChild));
    }

    // Returns TRUE if a MDI child is maximized
    // Refer to WM_MDIGETACTIVE in the Windows API documentation for more information.
    template <class T>
    inline BOOL CMDIFrameT<T>::IsMDIChildMaxed() const
    {
        BOOL isMaxed = FALSE;
        GetMDIClient().SendMessage(WM_MDIGETACTIVE, 0, (LPARAM)&isMaxed);
        return isMaxed;
    }

    // Arranges the MDI children in a cascade formation.
    // Possible values for type are:
    // MDITILE_SKIPDISABLED Prevents disabled MDI child windows from being cascaded.
    // Refer to WM_MDICASCADE in the Windows API documentation for more information.
    template <class T>
    inline void CMDIFrameT<T>::MDICascade(int type /* = 0*/) const
    {
        assert(T::IsWindow());
        GetMDIClient().SendMessage(WM_MDICASCADE, type, 0);
    }

    // Re-arranges the icons for minimized MDI children.
    // Refer to WM_MDIICONARRANGE in the Windows API documentation for more information.
    template <class T>
    inline void CMDIFrameT<T>::MDIIconArrange() const
    {
        assert(T::IsWindow());
        GetMDIClient().SendMessage(WM_MDIICONARRANGE, 0, 0);
    }

    // Maximize the MDI child.
    // Refer to WM_MDIMAXIMIZE in the Windows API documentation for more information.
    template <class T>
    inline void CMDIFrameT<T>::MDIMaximize() const
    {
        assert(T::IsWindow());
        WPARAM mdiChild = GetMDIClient().SendMessage(WM_MDIGETACTIVE, 0, 0);
        GetMDIClient().SendMessage(WM_MDIMAXIMIZE, mdiChild, 0);
    }

    // Activates the next MDI child.
    // Refer to WM_MDINEXT in the Windows API documentation for more information.
    template <class T>
    inline void CMDIFrameT<T>::MDINext() const
    {
        assert(T::IsWindow());
        WPARAM mdiChild = GetMDIClient().SendMessage(WM_MDIGETACTIVE, 0, 0);
        GetMDIClient().SendMessage(WM_MDINEXT, mdiChild, FALSE);
    }

    // Activates the previous MDI child.
    // Refer to WM_MDINEXT in the Windows API documentation for more information.
    template <class T>
    inline void CMDIFrameT<T>::MDIPrev() const
    {
        assert(T::IsWindow());
        WPARAM mdiChild = GetMDIClient().SendMessage(WM_MDIGETACTIVE, 0, 0);
        GetMDIClient().SendMessage(WM_MDINEXT, mdiChild, TRUE);
    }

    // Restores a mimimized MDI child.
    // Refer to WM_MDIRESTORE in the Windows API documentation for more information.
    template <class T>
    inline void CMDIFrameT<T>::MDIRestore() const
    {
        assert(T::IsWindow());
        WPARAM mdiChild = GetMDIClient().SendMessage(WM_MDIGETACTIVE, 0, 0);
        GetMDIClient().SendMessage(WM_MDIRESTORE, mdiChild, 0);
    }

    // Arrange all of the MDI child windows in a tile format.
    // Possible values for type are:
    // MDITILE_HORIZONTAL   Tiles MDI child windows so that one window appears above another.
    // MDITILE_SKIPDISABLED Prevents disabled MDI child windows from being tiled.
    // MDITILE_VERTICAL     Tiles MDI child windows so that one window appears beside another.
    // Refer to WM_MDITILE in the Windows API documentation for more information.
    template <class T>
    inline void CMDIFrameT<T>::MDITile(int type /* = 0*/) const
    {
        assert(T::IsWindow());
        GetMDIClient().SendMessage(WM_MDITILE, (WPARAM)type, 0);
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
    inline LRESULT CMDIFrameT<T>::OnInitMenuPopup(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        if (IsMDIChildMaxed())
        {
            CWnd* pMaxMDIChild = GetActiveMDIChild();
            assert(pMaxMDIChild);
            if (!pMaxMDIChild)  return 0;

            // Suppress owner drawing of the MDI child's system menu
            if (::GetSystemMenu(*pMaxMDIChild, FALSE) == reinterpret_cast<HMENU>(wparam))
                return CWnd::WndProcDefault(WM_INITMENUPOPUP, wparam, lparam);
        }

        return T::OnInitMenuPopup(msg, wparam, lparam);
    }

    // Updates the check buttons before displaying the menu.
    template <class T>
    inline void CMDIFrameT<T>::OnMenuUpdate(UINT id)
    {
        CMenu activeMenu = GetActiveMenu();
        switch (id)
        {
        case IDW_VIEW_STATUSBAR:
            {
                bool isVisible = T::GetStatusBar().IsWindow() && T::GetStatusBar().IsWindowVisible();
                activeMenu.CheckMenuItem(id, isVisible ? MF_CHECKED : MF_UNCHECKED);
            }
            break;

        case IDW_VIEW_TOOLBAR:
            {
                bool isVisible = T::GetToolBar().IsWindow() && T::GetToolBar().IsWindowVisible();
                activeMenu.EnableMenuItem(id, T::IsUsingToolBar() ? MF_ENABLED : MF_DISABLED);
                activeMenu.CheckMenuItem(id, isVisible ? MF_CHECKED : MF_UNCHECKED);
            }
            break;
        }
    }

    // Called when a MDI child is activated.
    template <class T>
    inline LRESULT CMDIFrameT<T>::OnMDIActivated(UINT, WPARAM wparam, LPARAM)
    {
        HWND activeMDIChild = reinterpret_cast<HWND>(wparam);

        if (activeMDIChild)
        {
            CMDIChild* pMDIChild = static_cast<CMDIChild*>(T::GetCWndPtr(activeMDIChild));
            assert ( dynamic_cast<CMDIChild*>(T::GetCWndPtr(activeMDIChild)) );
            if (pMDIChild->GetChildMenu())
                UpdateFrameMenu(pMDIChild->GetChildMenu());
            else
                UpdateFrameMenu(T::GetFrameMenu());
            if (pMDIChild->GetChildAccel())
                GetApp()->SetAccelerators(pMDIChild->GetChildAccel(), *this);
            else
                GetApp()->SetAccelerators(T::GetFrameAccel(), *this);
        }

        return 0;
    }

    // Called when a MDI child is destroyed.
    template <class T>
    inline LRESULT CMDIFrameT<T>::OnMDIDestroyed(UINT, WPARAM wparam, LPARAM)
    {
        RemoveMDIChild(reinterpret_cast<HWND>(wparam));
        return 0;
    }

    // Called when a MDI Child maximized state of the MDI child is checked.
    template <class T>
    inline void CMDIFrameT<T>::OnMDIMaximized(BOOL)
    {
        // Override this function to take an action when a MDI child is
        // maximized or restored from maximized.
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
    inline LRESULT CMDIFrameT<T>::OnWindowPosChanged(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        if (T::GetMenuBar().IsWindow())
        {
            // Refresh MenuBar Window
            T::GetMenuBar().SetMenu(T::GetMenuBar().GetMenu());
        }

        return FinalWindowProc(msg, wparam, lparam);
    }

    // Used to translate window messages before they are dispatched to 
    // the message loop.
    template <class T>
    inline BOOL CMDIFrameT<T>::PreTranslateMessage(MSG& msg)
    {
        if (WM_KEYFIRST <= msg.message && msg.message <= WM_KEYLAST)
        {
            if (TranslateMDISysAccel(T::GetView().GetHwnd(), &msg))
                return TRUE;
        }

        return T::PreTranslateMessage(msg);
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
        BOOL succeeded = TRUE;
        int children = static_cast<int>(m_mdiChildren.size());

        // Remove the children in reverse order
        for (int i = children-1; i >= 0; --i)
        {
            MDIChildPtr pMDIChild = m_mdiChildren[i];

            // Ask the window to close. If it is destroyed, RemoveMDIChild gets called.
            pMDIChild->SendMessage(WM_SYSCOMMAND, (WPARAM)SC_CLOSE, 0);

            if (pMDIChild->IsWindow())
                succeeded = FALSE;
        }

        return succeeded;
    }

    // Removes an individual MDI child.
    template <class T>
    inline void CMDIFrameT<T>::RemoveMDIChild(HWND wnd)
    {
        // Allocate an iterator for our HWND map
        std::vector<MDIChildPtr>::iterator v;

        for (v = m_mdiChildren.begin(); v!= m_mdiChildren.end(); ++v)
        {
            if ((*v)->GetHwnd() == wnd)
            {
                m_mdiChildren.erase(v);
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
                GetApp()->SetAccelerators(GetActiveMDIChild()->GetChildAccel(), *this);
            else
                GetApp()->SetAccelerators(T::GetFrameAccel(), *this);
        }
        else
        {
            if (T::GetMenuBar().IsWindow())
                T::GetMenuBar().SetMenu( T::GetFrameMenu() );
            else
                T::SetMenu( T::GetFrameMenu() );

            GetApp()->SetAccelerators(T::GetFrameAccel(), *this);
        }
    }

    // Activates the specified MDI child.
    // Refer to WM_MDIACTIVATE in the Windows API documentation for more information.
    template <class T>
    inline void CMDIFrameT<T>::SetActiveMDIChild(CMDIChild* pChild)
    {
        assert ( pChild->IsWindow() );

        GetMDIClient().SendMessage(WM_MDIACTIVATE, (WPARAM)(pChild->GetHwnd()), 0);
    }

    // Updates the frame menu. Also puts the list of CMDIChild windows in the "window" menu.
    // The "window" menu item is assumed to be the second from the right.
    template <class T>
    inline void CMDIFrameT<T>::UpdateFrameMenu(CMenu menu)
    {
        if (menu.GetHandle())
        {
            int menuItems = menu.GetMenuItemCount();
            if (menuItems > 0)
            {
                // The Window menu is typically second from the right
                int windowItem = MAX(menuItems - 2, 0);
                CMenu menuWindow = menu.GetSubMenu(windowItem);

                if (menuWindow.GetHandle())
                {
                    if (T::GetMenuBar().IsWindow())
                    {
                        AppendMDIMenu(menuWindow);
                        T::GetMenuBar().SetMenu(menu);
                    }
                    else
                    {
                        GetMDIClient().SendMessage(WM_MDISETMENU, (WPARAM)(menu.GetHandle()),
                            (LPARAM)(menuWindow.GetHandle()));
                        T::DrawMenuBar();
                    }
                }
            }
        }
    }

    // Provides default processing for the MDI frame window's messages.
    template <class T>
    inline LRESULT CMDIFrameT<T>::WndProcDefault(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        switch (msg)
        {
            case UWM_MDIACTIVATED:      return OnMDIActivated(msg, wparam, lparam);
            case UWM_MDIDESTROYED:      return OnMDIDestroyed(msg, wparam, lparam);
            case UWM_MDIGETACTIVE:
            {
                if (wparam && lparam)
                {
                    LPBOOL pIsMDIChildMax = (LPBOOL)lparam;
                    OnMDIMaximized(*pIsMDIChildMax);
                }
                break;

            }
            case WM_WINDOWPOSCHANGED:   return OnWindowPosChanged(msg, wparam, lparam);
        }

        return T::WndProcDefault(msg, wparam, lparam);
    }


    //////////////////////////////////////
    //Definitions for the CMDIClient class
    //

    // Creates the MDIClient window.
    template<class T>
    inline HWND CMDIClient<T>::Create(HWND parent)
    {
        assert(parent != 0);

        CLIENTCREATESTRUCT clientcreate;
        clientcreate.hWindowMenu  = 0;
        clientcreate.idFirstChild = IDW_FIRSTCHILD;
        DWORD style = WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | MDIS_ALLCHILDSTYLES;

        // Create the view window
        return T::CreateEx(WS_EX_CLIENTEDGE, _T("MDIClient"), _T(""), style, 0, 0, 0, 0, parent, NULL, (PSTR) &clientcreate);
    }

    // Called when the MDI child window is activated.
    template <class T>
    inline LRESULT CMDIClient<T>::OnMDIActivate(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        // Suppress redraw to avoid flicker when activating maximized MDI children
        T::SetRedraw(FALSE);
        LRESULT result = T::FinalWindowProc(msg, wparam, lparam);
        T::SetRedraw(TRUE);
        T::RedrawWindow(RDW_FRAME | RDW_INVALIDATE | RDW_ALLCHILDREN);

        return result;
    }

    // Called when a MDI child window is destroyed.
    template <class T>
    inline LRESULT CMDIClient<T>::OnMDIDestroy(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        // Do default processing first
        T::FinalWindowProc(msg, wparam, lparam);

        // Now remove MDI child
        T::GetParent().SendMessage(UWM_MDIDESTROYED, wparam, 0);

        return 0;
    }

    // Called when the active MDI child is requested.
    template <class T>
    inline LRESULT CMDIClient<T>::OnMDIGetActive(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        // Do default processing first
        LRESULT result = T::FinalWindowProc(msg, wparam, lparam);

        SendMessage(T::GetParent(), UWM_MDIGETACTIVE, (WPARAM)result, lparam);
        return result;
    }

    // Provides default processing for the MDIClient window's messages.
    template <class T>
    inline LRESULT CMDIClient<T>::WndProc(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        switch (msg)
        {
        case WM_MDIACTIVATE:    return OnMDIActivate(msg, wparam, lparam);
        case WM_MDIDESTROY:     return OnMDIDestroy(msg, wparam, lparam);
        case WM_MDIGETACTIVE:   return OnMDIGetActive(msg, wparam, lparam);
        }

        return CWnd::WndProcDefault(msg, wparam, lparam);
    }


    /////////////////////////////////////
    //Definitions for the CMDIChild class
    //

    // Sets the MDI Child's menu and accelerator in the constructor, like this ...
    //   HMENU hChildMenu = LoadMenu(GetApp()->GetResourceHandle(), _T("MdiMenuView"));
    //   HACCEL hChildAccel = LoadAccelerators(GetApp()->GetResourceHandle(), _T("MDIAccelView"));
    //   SetHandles(hChildMenu, hChildAccel);
    //   SetView(m_view);
    inline CMDIChild::CMDIChild() : m_pView(NULL), m_childAccel(0)
    {

    }

    inline CMDIChild::~CMDIChild()
    {
        if (IsWindow())
            GetParent().SendMessage(WM_MDIDESTROY, (WPARAM)GetHwnd(), 0);
    }

    // Create the MDI child window and then maximize if required.
    // This technique avoids unnecessary flicker when creating maximized MDI children.
    inline HWND CMDIChild::Create(HWND parent /*= 0*/)
    {
        CREATESTRUCT cs;
        WNDCLASS wc;

        ZeroMemory(&cs, sizeof(cs));
        ZeroMemory(&wc, sizeof(wc));

        //Call PreCreate in case its overloaded
        PreCreate(cs);

        //Determine if the window should be created maximized
        BOOL Max = FALSE;
        CWnd* pParent = GetCWndPtr(parent);
        assert(pParent);
        if (!pParent)  return 0;

        pParent->SendMessage(WM_MDIGETACTIVE, 0, (LPARAM)&Max);

        Max = Max | (cs.style & WS_MAXIMIZE);

        // Set the Window Class Name
        CString ClassName = _T("Win32++ MDI Child");
        if (cs.lpszClass)
            ClassName = cs.lpszClass;

        // Set the window style
        DWORD style;
        style = cs.style & ~WS_MAXIMIZE;
        style |= WS_VISIBLE | WS_OVERLAPPEDWINDOW ;

        // Set window size and position
        int x = CW_USEDEFAULT;
        int y = CW_USEDEFAULT;
        int cx = CW_USEDEFAULT;
        int cy = CW_USEDEFAULT;
        if (cs.cx && cs.cy)
        {
            x = cs.x;
            y = cs.y;
            cx = cs.cx;
            cy = cs.cy;
        }

        // Set the extended style
        DWORD exStyle = cs.dwExStyle | WS_EX_MDICHILD;

        // Turn off redraw while creating the window
        pParent->SetRedraw(FALSE);

        // Create the window
        CreateEx(exStyle, ClassName, cs.lpszName, style, x, y,
            cx, cy, pParent->GetHwnd(), cs.hMenu, cs.lpCreateParams);

        if (Max)
            ShowWindow(SW_MAXIMIZE);

        // Turn redraw back on
        pParent->SetRedraw(TRUE);
        pParent->RedrawWindow(RDW_INVALIDATE | RDW_ALLCHILDREN);

        // Ensure bits revealed by round corners (XP themes) are redrawn
        VERIFY(SetWindowPos(0, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE|SWP_FRAMECHANGED));

        return GetHwnd();
    }

    // Overrides CWnd::FinalWindowProc to call DefMDIChildProc instead of DefWindowProc.
    inline LRESULT CMDIChild::FinalWindowProc(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        return ::DefMDIChildProc(*this, msg, wparam, lparam);
    }

    // Returns a reference to the view window.
    inline CWnd& CMDIChild::GetView() const
    {
        // Note: Use SetView to set the view window.
        assert(m_pView);
        return *m_pView;
    }

    // Activate a MDI child.
    // Refer to WM_MDIACTIVATE in the Windows API documentation for more information.
    inline void CMDIChild::MDIActivate() const
    {
        GetParent().SendMessage(WM_MDIACTIVATE, (WPARAM)GetHwnd(), 0);
    }

    // Destroy a MDI child.
    // Refer to WM_MDIDESTROY in the Windows API documentation for more information.
    inline void CMDIChild::MDIDestroy() const
    {
        GetParent().SendMessage(WM_MDIDESTROY, (WPARAM)GetHwnd(), 0);
    }

    // Maximize a MDI child.
    // Refer to WM_MDIMAXIMIZE in the Windows API documentation for more information.
    inline void CMDIChild::MDIMaximize() const
    {
        GetParent().SendMessage(WM_MDIMAXIMIZE, (WPARAM)GetHwnd(), 0);
    }

    // Restore a MDI child.
    // Refer to WM_MDIRESTORE in the Windows API documentation for more information.
    inline void CMDIChild::MDIRestore() const
    {
        GetParent().SendMessage(WM_MDIRESTORE, (WPARAM)GetHwnd(), 0);
    }

    // Override this to customize what happens when the window asks to be closed.
    inline void CMDIChild::OnClose()
    {
        MDIDestroy();
    }

    // Called when the MDI child is created.
    inline int CMDIChild::OnCreate(CREATESTRUCT&)
    {
        // Create the view window
        assert( &GetView() );           // Use SetView in CMDIChild's constructor to set the view window.
        GetView().Create(*this);
        GetView().SetFocus();
        RecalcLayout();

        return 0;
    }

    // Called when the MDI child is activated.
    inline LRESULT CMDIChild::OnMDIActivate(UINT, WPARAM, LPARAM lparam)
    {
        // This child is being activated
        if (lparam == (LPARAM)GetHwnd())
        {
            GetAncestor().SendMessage(UWM_MDIACTIVATED, (WPARAM)GetHwnd(), 0);
            GetView().SetFocus();
        }

        // No child is being activated
        if (lparam == 0)
        {
            GetAncestor().SendMessage(UWM_MDIACTIVATED, 0, 0);
        }

        return 0;
    }

    // Called when the MDI child is resized.
    inline LRESULT CMDIChild::OnWindowPosChanged(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        RecalcLayout();
        return FinalWindowProc(msg, wparam, lparam);
    }

    // Repositions the MDI child's child windows.
    inline void CMDIChild::RecalcLayout()
    {
        // Resize the View window
        CRect rc = GetClientRect();
        VERIFY(GetView().SetWindowPos( 0, rc.left, rc.top, rc.Width(), rc.Height(), SWP_SHOWWINDOW ));
    }

    // Sets the MDI child's menu and accelerator handles.
    inline void CMDIChild::SetHandles(HMENU menu, HACCEL accel)
    {
        m_childMenu.Attach(menu);
        m_childAccel = accel;

        // Note: It is valid to call SetHandles before the window is created.
        if (IsWindow())
        {
            GetParent().SendMessage(WM_MDIACTIVATE, (WPARAM)GetHwnd(), 0);
            GetAncestor().SendMessage(UWM_MDIACTIVATED, (WPARAM)GetHwnd(), 0);
        }
    }

    // Sets or changes the MDI child's view window.
    inline void CMDIChild::SetView(CWnd& view)
    {
        if (m_pView != &view)
        {
            // Hide the existing view window (if any)
            if (m_pView && m_pView->IsWindow())
                m_pView->ShowWindow(SW_HIDE);

            // Assign the view window
            m_pView = &view;

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
    inline LRESULT CMDIChild::WndProcDefault(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        switch (msg)
        {
            case WM_MDIACTIVATE:        return OnMDIActivate(msg, wparam, lparam);
            case WM_WINDOWPOSCHANGED:   return OnWindowPosChanged(msg, wparam, lparam);
        }
        return CWnd::WndProcDefault(msg, wparam, lparam);
    }


} // namespace Win32xx

#endif // _WIN32XX_MDI_H_

