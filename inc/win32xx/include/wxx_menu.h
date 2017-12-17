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
// wxx_menu.h
//  Declaration of the CMenu class

// Notes
//  1) Owner-drawn menus send the WM_MEASUREITEM and WM_DRAWITEM messages
//     to the window that owns the menu. To manage owner drawing for menus,
//     handle these two messages in the CWnd's WndProc function.
//
//  2) CMenu objects are reference counted and can be safely copied. They can
//     be returned by value from functions and passed by value in function
//     arguments.
//
//  3) In those functions that use a MENUITEMINFO structure, its cbSize member
//     is automatically set to the correct value.

//  Program sample
//  --------------
//  void CView::CreatePopup()
//  {
//      CPoint pt = GetCursorPos();
//
//      // Create the menu
//      CMenu Popup;
//      Popup.CreatePopupMenu();
//
//      // Add some menu items
//      Popup.AppendMenu(MF_STRING, 101, _T("Menu Item &1"));
//      Popup.AppendMenu(MF_STRING, 102, _T("Menu Item &2"));
//      Popup.AppendMenu(MF_STRING, 103, _T("Menu Item &3"));
//      Popup.AppendMenu(MF_SEPARATOR);
//      Popup.AppendMenu(MF_STRING, 104, _T("Menu Item &4"));
//
//      // Set menu item states
//      Popup.CheckMenuRadioItem(101, 101, 101, MF_BYCOMMAND);
//      Popup.CheckMenuItem(102, MF_BYCOMMAND | MF_CHECKED);
//      Popup.EnableMenuItem(103, MF_BYCOMMAND | MF_GRAYED);
//      Popup.SetDefaultItem(104);
//
//      // Display the popup menu
//      Popup.TrackPopupMenu(0, pt.x, pt.y, this);
//  }


#if !defined (_WIN32XX_MENU_H_) && !defined(_WIN32_WCE)
#define _WIN32XX_MENU_H_


#include "wxx_appcore0.h"
#include "wxx_cstring.h"


namespace Win32xx
{

    // This class provides support for menus. It provides member functions for creating,
    // tracking, updating, and destroying a menu.
    class CMenu
    {

    public:
        //Construction
        CMenu();
        CMenu(UINT nID);
        CMenu(HMENU hMenu);
        CMenu(const CMenu& rhs);
        CMenu& operator = (const CMenu& rhs);
        void operator = (const HMENU hImageList);
        ~CMenu();

        //Initialization
        void Attach(HMENU hMenu);
        void CreateMenu() const;
        void CreatePopupMenu() const;
        void DestroyMenu();
        HMENU Detach();

        HMENU GetHandle() const;
        BOOL LoadMenu(LPCTSTR lpszResourceName) const;
        BOOL LoadMenu(UINT uIDResource) const;
        BOOL LoadMenuIndirect(const LPMENUTEMPLATE lpMenuTemplate) const;

        //Menu Operations
        BOOL TrackPopupMenu(UINT uFlags, int x, int y, HWND hWnd, LPCRECT lpRect = 0) const;
        BOOL TrackPopupMenuEx(UINT uFlags, int x, int y, HWND hWnd, LPTPMPARAMS lptpm) const;

        //Menu Item Operations
        BOOL AppendMenu(UINT uFlags, UINT_PTR uIDNewItem = 0, LPCTSTR lpszNewItem = NULL);
        BOOL AppendMenu(UINT uFlags, UINT_PTR uIDNewItem, HBITMAP hBitmap);
        UINT CheckMenuItem(UINT uIDCheckItem, UINT uCheck) const;
        BOOL CheckMenuRadioItem(UINT uIDFirst, UINT uIDLast, UINT uIDItem, UINT uFlags) const;
        BOOL DeleteMenu(UINT uPosition, UINT uFlags) const;
        UINT EnableMenuItem(UINT uIDEnableItem, UINT uEnable) const;
        int FindMenuItem(LPCTSTR szMenuString) const;
        UINT GetDefaultItem(UINT gmdiFlags, BOOL ByPosition = FALSE) const;
        DWORD GetMenuContextHelpId() const;

#if(WINVER >= 0x0500)   // Minimum OS required is Win2000
        BOOL GetMenuInfo(MENUINFO& mi) const;
        BOOL SetMenuInfo(const MENUINFO& mi) const;
#endif

        UINT GetMenuItemCount() const;
        UINT GetMenuItemID(int nPos) const;
        BOOL GetMenuItemInfo(UINT uItem, MENUITEMINFO& MenuItemInfo, BOOL ByPosition = FALSE) const;
        UINT GetMenuState(UINT uID, UINT uFlags) const;
        int GetMenuString(UINT uIDItem, LPTSTR lpString, int nMaxCount, UINT uFlags) const;
        int GetMenuString(UINT uIDItem, CString& rString, UINT uFlags) const;
        CMenu GetSubMenu(int nPos) const;
        BOOL InsertMenu(UINT uPosition, UINT uFlags, UINT_PTR uIDNewItem = 0, LPCTSTR lpszNewItem = NULL) const;
        BOOL InsertMenu(UINT uPosition, UINT uFlags, UINT_PTR uIDNewItem, HBITMAP hBitmap) const;
        BOOL InsertMenuItem(UINT uItem, MENUITEMINFO& MenuItemInfo, BOOL ByPosition = FALSE) const;
        BOOL InsertPopupMenu(UINT uPosition, UINT uFlags, HMENU hPopupMenu, LPCTSTR lpszNewItem) const;
        BOOL ModifyMenu(UINT uPosition, UINT uFlags, UINT_PTR uIDNewItem = 0, LPCTSTR lpszNewItem = NULL) const;
        BOOL ModifyMenu(UINT uPosition, UINT uFlags, UINT_PTR uIDNewItem, HBITMAP hBitmap) const;
        BOOL RemoveMenu(UINT uPosition, UINT uFlags) const;
        BOOL SetDefaultItem(UINT uItem, BOOL ByPosition = FALSE) const;
        BOOL SetMenuContextHelpId(DWORD dwContextHelpId) const;
        BOOL SetMenuItemBitmaps(UINT uPosition, UINT uFlags, HBITMAP hbmUnchecked, HBITMAP hbmChecked) const;
        BOOL SetMenuItemInfo(UINT uItem, MENUITEMINFO& MenuItemInfo, BOOL ByPosition = FALSE) const;

        //Operators
        operator HMENU () const;

    private:
        void AddToMap() const;
        void Release();
        BOOL RemoveFromMap() const;
        CMenu_Data* m_pData;

    };

} // namespace Win32xx

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

namespace Win32xx
{
    ////////////////////////////////////////
    // Global function
    //

    // Returns the correct size of the MENUITEMINFO struct.
    // The size of the MENUIEMINFO struct varies according to the window version.
    inline UINT GetSizeofMenuItemInfo()
    {
        // For Win95 and NT, cbSize needs to be 44
        if (1400 == (GetWinVersion()) || (2400 == GetWinVersion()))
            return CCSIZEOF_STRUCT(MENUITEMINFO, cch);

        return sizeof(MENUITEMINFO);
    }

    ////////////////////////////////////////
    // Definitions of CMenu
    //
    inline CMenu::CMenu()
    {
        m_pData = new CMenu_Data;
    }


    inline CMenu::CMenu(UINT nID)
    {
        m_pData = new CMenu_Data;

        HMENU menu = ::LoadMenu(GetApp().GetResourceHandle(), MAKEINTRESOURCE(nID));
        Attach(menu);
        m_pData->IsManagedMenu = TRUE;
    }


    inline CMenu::CMenu(HMENU hMenu)
    {
        m_pData = new CMenu_Data;
        Attach(hMenu);
    }


    // Note: A copy of a CMenu is a clone of the original.
    //       Both objects manipulate the one HMENU.
    inline CMenu::CMenu(const CMenu& rhs)
    {
        m_pData = rhs.m_pData;
        InterlockedIncrement(&m_pData->Count);
    }


    // Note: A copy of a CMenu is a clone of the original.
    inline CMenu& CMenu::operator = (const CMenu& rhs)
    {
        if (this != &rhs)
        {
            InterlockedIncrement(&rhs.m_pData->Count);
            Release();
            m_pData = rhs.m_pData;
        }

        return *this;
    }


    inline void CMenu::operator = (const HMENU hMenu)
    {
        Attach(hMenu);
    }


    inline CMenu::~CMenu()
    {
        Release();
    }


    // Store the HMENU and CMenu pointer in the HMENU map
    inline void CMenu::AddToMap() const
    {
        assert( &GetApp() );
        assert(m_pData);
        assert(m_pData->hMenu);

        GetApp().AddCMenuData(m_pData->hMenu, m_pData);
    }


    inline void CMenu::Release()
    {
        assert(m_pData);

        if (InterlockedDecrement(&m_pData->Count) == 0)
        {
            if (m_pData->hMenu != NULL)
            {
                if (m_pData->IsManagedMenu)
                {
                    ::DestroyMenu(m_pData->hMenu);
                }

                RemoveFromMap();
            }

            delete m_pData;
            m_pData = 0;
        }
    }


    inline BOOL CMenu::RemoveFromMap() const
    {
        BOOL Success = FALSE;

        if ( &GetApp() )
        {
            // Allocate an iterator for our HMENU map
            std::map<HMENU, CMenu_Data*, CompareHMENU>::iterator m;

            CWinApp& App = GetApp();
            CThreadLock mapLock(App.m_csMapLock);
            m = App.m_mapCMenuData.find(m_pData->hMenu);
            if (m != App.m_mapCMenuData.end())
            {
                // Erase the Menu pointer entry from the map
                App.m_mapCMenuData.erase(m);
                Success = TRUE;
            }

        }

        return Success;
    }


    // Appends a new item to the end of the specified menu bar, drop-down menu, submenu, or shortcut menu.
    inline BOOL CMenu::AppendMenu(UINT uFlags, UINT_PTR uIDNewItem /*= 0*/, LPCTSTR lpszNewItem /*= NULL*/)
    {
        assert(m_pData);
        assert(IsMenu(m_pData->hMenu));
        return ::AppendMenu(m_pData->hMenu, uFlags, uIDNewItem, lpszNewItem);
    }


    // Appends a new item to the end of the specified menu bar, drop-down menu, submenu, or shortcut menu.
    inline BOOL CMenu::AppendMenu(UINT uFlags, UINT_PTR uIDNewItem, HBITMAP hBitmap)
    {
        assert(m_pData);
        assert(IsMenu(m_pData->hMenu));
        return ::AppendMenu(m_pData->hMenu, uFlags, uIDNewItem, reinterpret_cast<LPCTSTR>(hBitmap));
    }


    // Attaches an existing menu to this CMenu. The hMenu can be NULL
    inline void CMenu::Attach(HMENU hMenu)
    {
        assert(m_pData);

        if (hMenu != m_pData->hMenu)
        {
            // Release any existing menu
            if (m_pData->hMenu != 0)
            {
                Release();
                m_pData = new CMenu_Data;
            }

            if (hMenu)
            {
                // Add the menu to this CMenu
                CMenu_Data* pCMenuData = GetApp().GetCMenuData(hMenu);
                if (pCMenuData)
                {
                    delete m_pData;
                    m_pData = pCMenuData;
                    InterlockedIncrement(&m_pData->Count);
                }
                else
                {
                    m_pData->hMenu = hMenu;
                    AddToMap();
                }
            }
        }
    }


    // Sets the state of the specified menu item's check-mark attribute to either selected or clear.
    inline UINT CMenu::CheckMenuItem(UINT uIDCheckItem, UINT uCheck) const
    {
        assert(m_pData);
        assert(IsMenu(m_pData->hMenu));
        return ::CheckMenuItem(m_pData->hMenu, uIDCheckItem, uCheck);
    }


    // Checks a specified menu item and makes it a radio item. At the same time, the function clears
    // all other menu items in the associated group and clears the radio-item type flag for those items.
    inline BOOL CMenu::CheckMenuRadioItem(UINT uIDFirst, UINT uIDLast, UINT uIDItem, UINT uFlags) const
    {
        assert(m_pData);
        assert(IsMenu(m_pData->hMenu));
        return ::CheckMenuRadioItem(m_pData->hMenu, uIDFirst, uIDLast, uIDItem, uFlags);
    }


    // Creates an empty menu.
    inline void CMenu::CreateMenu() const
    {
        assert(m_pData);
        assert(NULL == m_pData->hMenu);
        m_pData->hMenu = ::CreateMenu();
        AddToMap();
        m_pData->IsManagedMenu = TRUE;
    }


    // Creates a drop-down menu, submenu, or shortcut menu. The menu is initially empty.
    inline void CMenu::CreatePopupMenu() const
    {
        assert(m_pData);
        assert(NULL == m_pData->hMenu);
        m_pData->hMenu = ::CreatePopupMenu();
        AddToMap();
        m_pData->IsManagedMenu = TRUE;
    }


    // Deletes an item from the specified menu.
    inline BOOL CMenu::DeleteMenu(UINT uPosition, UINT uFlags) const
    {
        assert(m_pData);
        assert(IsMenu(m_pData->hMenu));
        return ::DeleteMenu(m_pData->hMenu, uPosition, uFlags);
    }


    // Destroys the menu and frees any memory that the menu occupies.
    inline void CMenu::DestroyMenu()
    {
        assert(m_pData);
        if (::IsMenu(m_pData->hMenu))
            ::DestroyMenu( Detach() );
    }


    // Detaches the HMENU from all CMenu objects.
    inline HMENU CMenu::Detach()
    {
        assert(m_pData);
        HMENU hMenu = m_pData->hMenu;
        RemoveFromMap();
        m_pData->hMenu = 0;
        m_pData->vSubMenus.clear();

        if (m_pData->Count > 0)
        {
            if (InterlockedDecrement(&m_pData->Count) == 0)
            {
                delete m_pData;
            }
        }

        m_pData = new CMenu_Data;

        return hMenu;
    }


    // Returns the HMENU assigned to this CMenu
    inline HMENU CMenu::GetHandle() const
    {
        assert(m_pData);
        return m_pData->hMenu;
    }


    // Enables, disables, or grays the specified menu item.
    // The uEnable parameter must be a combination of either MF_BYCOMMAND or MF_BYPOSITION
    // and MF_ENABLED, MF_DISABLED, or MF_GRAYED.
    // MF_DISABLED and MF_GRAYED are different without XP themes, but the same with XP themes enabled.
    inline UINT CMenu::EnableMenuItem(UINT uIDEnableItem, UINT uEnable) const
    {
        assert(m_pData);
        assert(IsMenu(m_pData->hMenu));
        return ::EnableMenuItem(m_pData->hMenu, uIDEnableItem, uEnable);
    }


    // Finds the position of a menu item with the specified string.
    // Returns -1 if the string is not found.
    inline int CMenu::FindMenuItem(LPCTSTR szMenuString) const
    {
        assert(m_pData);
        assert(IsMenu(m_pData->hMenu));

        CString str;
        int count = GetMenuItemCount();
        for (int i = 0; i < count; i++)
        {
            if (GetMenuString(i, str, MF_BYPOSITION))
            {
                if (str == szMenuString)
                    return i;
            }
        }

        return -1;
    }


    // Determines the default menu item.
    // The gmdiFlags parameter specifies how the function searches for menu items.
    // This parameter can be zero or more of the following values: GMDI_GOINTOPOPUPS; GMDI_USEDISABLED.
    inline UINT CMenu::GetDefaultItem(UINT gmdiFlags, BOOL ByPosition /*= FALSE*/) const
    {
        assert(m_pData);
        assert(IsMenu(m_pData->hMenu));
        return ::GetMenuDefaultItem(m_pData->hMenu, ByPosition, gmdiFlags);
    }


    // Retrieves the Help context identifier associated with the menu.
    inline DWORD CMenu::GetMenuContextHelpId() const
    {
        assert(m_pData);
        assert(IsMenu(m_pData->hMenu));
        return ::GetMenuContextHelpId(m_pData->hMenu);
    }


// minimum OS required : Win2000
#if(WINVER >= 0x0500)

    // Retrieves the menu information.
    inline BOOL CMenu::GetMenuInfo(MENUINFO& mi) const
    {
        assert(m_pData);
        assert(IsMenu(m_pData->hMenu));
        return ::GetMenuInfo(m_pData->hMenu, &mi);
    }


    // Sets the menu information from the specified MENUINFO structure.
    inline BOOL CMenu::SetMenuInfo(const MENUINFO& mi) const
    {
        assert(m_pData);
        assert(IsMenu(m_pData->hMenu));
        return ::SetMenuInfo(m_pData->hMenu, &mi);
    }

#endif


    // Retrieves the number of menu items.
    inline UINT CMenu::GetMenuItemCount() const
    {
        assert(m_pData);
        assert(IsMenu(m_pData->hMenu));
        return ::GetMenuItemCount(m_pData->hMenu);
    }


    // Retrieves the menu item identifier of a menu item located at the specified position
    inline UINT CMenu::GetMenuItemID(int nPos) const
    {
        assert(m_pData);
        assert(IsMenu(m_pData->hMenu));
        return ::GetMenuItemID(m_pData->hMenu, nPos);
    }


    // retrieves information about the specified menu item.
    inline BOOL CMenu::GetMenuItemInfo(UINT uItem, MENUITEMINFO& MenuItemInfo, BOOL ByPosition /*= FALSE*/) const
    {
        assert(m_pData);
        assert(IsMenu(m_pData->hMenu));
        MenuItemInfo.cbSize = GetSizeofMenuItemInfo();
        return ::GetMenuItemInfo(m_pData->hMenu, uItem, ByPosition, &MenuItemInfo);
    }


    // Retrieves the menu flags associated with the specified menu item.
    // Possible values for uFlags are: MF_BYCOMMAND (default) or MF_BYPOSITION.
    inline UINT CMenu::GetMenuState(UINT uID, UINT uFlags) const
    {
        assert(m_pData);
        assert(IsMenu(m_pData->hMenu));
        return ::GetMenuState(m_pData->hMenu, uID, uFlags);
    }


    // Copies the text string of the specified menu item into the specified buffer.
    inline int CMenu::GetMenuString(UINT uIDItem, LPTSTR lpString, int nMaxCount, UINT uFlags) const
    {
        assert(m_pData);
        assert(IsMenu(m_pData->hMenu));
        assert(lpString);
        return ::GetMenuString(m_pData->hMenu, uIDItem, lpString, nMaxCount, uFlags);
    }


    // Copies the text string of the specified menu item into the specified buffer.
    inline int CMenu::GetMenuString(UINT uIDItem, CString& rString, UINT uFlags) const
    {
        assert(m_pData);
        assert(IsMenu(m_pData->hMenu));
        int n = ::GetMenuString(m_pData->hMenu, uIDItem, rString.GetBuffer(MAX_MENU_STRING), MAX_MENU_STRING, uFlags);
        rString.ReleaseBuffer();
        return n;
    }


    // Retrieves the CMenu object of a pop-up menu.
    inline CMenu CMenu::GetSubMenu(int nPos) const
    {
        assert(m_pData);
        assert(IsMenu(m_pData->hMenu));
        CMenu* pMenu = new CMenu;
        pMenu->m_pData->hMenu = ::GetSubMenu(m_pData->hMenu, nPos);
        pMenu->m_pData->IsManagedMenu = FALSE;
        m_pData->vSubMenus.push_back(pMenu);
        return *pMenu;
    }


    // Inserts a new menu item into a menu, moving other items down the menu.
    inline BOOL CMenu::InsertMenu(UINT uPosition, UINT uFlags, UINT_PTR uIDNewItem /*= 0*/, LPCTSTR lpszNewItem /*= NULL*/) const
    {
        assert(m_pData);
        assert(IsMenu(m_pData->hMenu));
        return ::InsertMenu(m_pData->hMenu, uPosition, uFlags, uIDNewItem, lpszNewItem);
    }


    // Inserts a new menu item into a menu, moving other items down the menu.
    inline BOOL CMenu::InsertMenu(UINT uPosition, UINT uFlags, UINT_PTR uIDNewItem, HBITMAP hBitmap) const
    {
        assert(m_pData);
        assert(IsMenu(m_pData->hMenu));
        return ::InsertMenu(m_pData->hMenu, uPosition, uFlags, uIDNewItem, reinterpret_cast<LPCTSTR>(hBitmap));
    }


    // Inserts a new menu item at the specified position in the menu.
    inline BOOL CMenu::InsertMenuItem(UINT uItem, MENUITEMINFO& MenuItemInfo, BOOL ByPosition /*= FALSE*/) const
    {
        assert(m_pData);
        assert(IsMenu(m_pData->hMenu));
        MenuItemInfo.cbSize = GetSizeofMenuItemInfo();
        return ::InsertMenuItem(m_pData->hMenu, uItem, ByPosition, &MenuItemInfo);
    }


    // Inserts a popup menu item at the specified position in the menu.
    inline BOOL CMenu::InsertPopupMenu(UINT uPosition, UINT uFlags, HMENU hPopupMenu, LPCTSTR lpszNewItem) const
    {
        assert(hPopupMenu);
        assert(IsMenu(m_pData->hMenu));

        // Ensure the correct flags are set
        uFlags &= ~MF_BITMAP;
        uFlags &= ~MF_OWNERDRAW;
        uFlags |= MF_POPUP;

        return ::InsertMenu(m_pData->hMenu, uPosition, uFlags, (UINT_PTR)hPopupMenu, lpszNewItem);
    }


    // Loads the menu from the specified windows resource.
    inline BOOL CMenu::LoadMenu(LPCTSTR lpszResourceName) const
    {
        assert(m_pData);
        assert(NULL == m_pData->hMenu);
        assert(lpszResourceName);
        m_pData->hMenu = ::LoadMenu(GetApp().GetResourceHandle(), lpszResourceName);
        if (m_pData->hMenu != 0) AddToMap();
        return NULL != m_pData->hMenu;
    }


    // Loads the menu from the specified windows resource.
    inline BOOL CMenu::LoadMenu(UINT uIDResource) const
    {
        assert(m_pData);
        assert(NULL == m_pData->hMenu);
        m_pData->hMenu = ::LoadMenu(GetApp().GetResourceHandle(), MAKEINTRESOURCE(uIDResource));
        if (m_pData->hMenu != 0) AddToMap();
        return NULL != m_pData->hMenu;
    }


    // Loads the specified menu template and assigns it to this CMenu.
    inline BOOL CMenu::LoadMenuIndirect(const LPMENUTEMPLATE lpMenuTemplate) const
    {
        assert(m_pData);
        assert(NULL == m_pData->hMenu);
        assert(lpMenuTemplate);
        m_pData->hMenu = ::LoadMenuIndirect(lpMenuTemplate);
        if (m_pData->hMenu) AddToMap();
        return NULL != m_pData->hMenu;
    }


    // Changes an existing menu item. This function is used to specify the content, appearance, and behaviour of the menu item.
    inline BOOL CMenu::ModifyMenu(UINT uPosition, UINT uFlags, UINT_PTR uIDNewItem /*= 0*/, LPCTSTR lpszNewItem /*= NULL*/) const
    {
        assert(m_pData);
        assert(IsMenu(m_pData->hMenu));
        return ::ModifyMenu(m_pData->hMenu, uPosition, uFlags, uIDNewItem, lpszNewItem);
    }


    // Changes an existing menu item. This function is used to specify the content, appearance, and behaviour of the menu item.
    inline BOOL CMenu::ModifyMenu(UINT uPosition, UINT uFlags, UINT_PTR uIDNewItem, HBITMAP hBitmap) const
    {
        assert(m_pData);
        assert(IsMenu(m_pData->hMenu));
        return ::ModifyMenu(m_pData->hMenu, uPosition, uFlags, uIDNewItem, reinterpret_cast<LPCTSTR>(hBitmap));
    }


    // Deletes a menu item or detaches a submenu from the menu.
    inline BOOL CMenu::RemoveMenu(UINT uPosition, UINT uFlags) const
    {
        assert(m_pData);
        assert(IsMenu(m_pData->hMenu));
        return ::RemoveMenu(m_pData->hMenu, uPosition, uFlags);
    }


    //  sets the default menu item for the menu.
    inline BOOL CMenu::SetDefaultItem(UINT uItem, BOOL ByPosition /*= FALSE*/) const
    {
        assert(m_pData);
        assert(IsMenu(m_pData->hMenu));
        return ::SetMenuDefaultItem(m_pData->hMenu, uItem, ByPosition);
    }


    // Associates a Help context identifier with the menu.
    inline BOOL CMenu::SetMenuContextHelpId(DWORD dwContextHelpId) const
    {
        assert(m_pData);
        assert(IsMenu(m_pData->hMenu));
        return ::SetMenuContextHelpId(m_pData->hMenu, dwContextHelpId);
    }


    // Associates the specified bitmap with a menu item.
    inline BOOL CMenu::SetMenuItemBitmaps(UINT uPosition, UINT uFlags, HBITMAP hbmUnchecked, HBITMAP hbmChecked) const
    {
        assert(m_pData);
        assert(IsMenu(m_pData->hMenu));
        return ::SetMenuItemBitmaps(m_pData->hMenu, uPosition, uFlags, hbmUnchecked, hbmChecked);
    }


    // Changes information about a menu item.
    inline BOOL CMenu::SetMenuItemInfo(UINT uItem, MENUITEMINFO& MenuItemInfo, BOOL ByPosition /*= FALSE*/) const
    {
        assert(m_pData);
        assert(IsMenu(m_pData->hMenu));
        MenuItemInfo.cbSize = GetSizeofMenuItemInfo();
        return ::SetMenuItemInfo(m_pData->hMenu, uItem, ByPosition, &MenuItemInfo);
    }


    // Displays a shortcut menu at the specified location and tracks the selection of items on the menu.
    inline BOOL CMenu::TrackPopupMenu(UINT uFlags, int x, int y, HWND hWnd, LPCRECT lpRect /*= 0*/) const
    {
        assert(m_pData);
        assert(IsMenu(m_pData->hMenu));
        return ::TrackPopupMenu(m_pData->hMenu, uFlags, x, y, 0, hWnd, lpRect);
    }


    // Displays a shortcut menu at the specified location and tracks the selection of items on the shortcut menu.
    inline BOOL CMenu::TrackPopupMenuEx(UINT uFlags, int x, int y, HWND hWnd, LPTPMPARAMS lptpm) const
    {
        assert(m_pData);
        assert(IsMenu(m_pData->hMenu));
        return ::TrackPopupMenuEx(m_pData->hMenu, uFlags, x, y, hWnd, lptpm);
    }


    // Retrieves the menu's handle.
    inline CMenu::operator HMENU () const
    {
        assert(m_pData);
        return m_pData->hMenu;
    }


}   // namespace Win32xx

#endif  // _WIN32XX_MENU_H_

