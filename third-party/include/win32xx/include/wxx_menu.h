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
//      Popup.TrackPopupMenu(0, pt.x, pt.y, *this);
//  }


#ifndef _WIN32XX_MENU_H_
#define _WIN32XX_MENU_H_


#include "wxx_appcore0.h"


namespace Win32xx
{

    // This class provides support for menus. It provides member functions for creating,
    // tracking, updating, and destroying a menu.
    class CMenu
    {
    public:
        // Construction
        CMenu();
        CMenu(UINT id);
        CMenu(HMENU menu);
        CMenu(const CMenu& rhs);
        CMenu& operator=(const CMenu& rhs);
        CMenu& operator=(HMENU menu);
        virtual ~CMenu();

        // Initialization
        void Attach(HMENU menu);
        void CreateMenu();
        void CreatePopupMenu();
        void DestroyMenu();
        HMENU Detach();

        HMENU GetHandle() const;
        BOOL LoadMenu(LPCTSTR resourceName);
        BOOL LoadMenu(UINT resourceID);
        BOOL LoadMenuIndirect(const LPMENUTEMPLATE pMenuTemplate);

        // Menu Operations
        BOOL TrackPopupMenu(UINT flags, int x, int y, HWND wnd, LPCRECT pRect = 0) const;
        BOOL TrackPopupMenuEx(UINT flags, int x, int y, HWND wnd, LPTPMPARAMS pTPMP) const;

        // Menu Item Operations
        BOOL AppendMenu(UINT flags, UINT_PTR idOrHandle = 0, LPCTSTR newItemName = nullptr);
        BOOL AppendMenu(UINT flags, UINT_PTR idOrHandle, HBITMAP bitmap);
        UINT CheckMenuItem(UINT checkItemID, UINT check) const;
        BOOL CheckMenuRadioItem(UINT firstID, UINT lastID, UINT itemID, UINT flags) const;
        BOOL DeleteMenu(UINT position, UINT flags) const;
        UINT EnableMenuItem(UINT enableItemID, UINT enable) const;
        int FindMenuItem(LPCTSTR menuName) const;
        UINT GetDefaultItem(UINT flags, BOOL byPosition = FALSE) const;
        DWORD GetMenuContextHelpId() const;
        BOOL GetMenuInfo(MENUINFO& mi) const;
        int GetMenuItemCount() const;
        ULONG_PTR GetMenuItemData(UINT idOrPos, BOOL byPosition = FALSE) const;
        UINT GetMenuItemID(int pos) const;
        BOOL GetMenuItemInfo(UINT idOrPos, MENUITEMINFO& menuItemInfo, BOOL byPosition = FALSE) const;
        UINT GetMenuState(UINT idOrPos, UINT flags) const;
        int GetMenuString(UINT idOrPos, LPTSTR string, int maxCount, UINT flags) const;
        int GetMenuString(UINT idOrPos, CString& string, UINT flags) const;
        CMenu GetSubMenu(int pos) const;
        BOOL InsertMenu(UINT pos, UINT flags, UINT_PTR idOrHandle = 0, LPCTSTR newItemName = nullptr) const;
        BOOL InsertMenu(UINT pos, UINT flags, UINT_PTR idOrHandle, HBITMAP bitmap) const;
        BOOL InsertMenuItem(UINT idOrPos, MENUITEMINFO& menuItemInfo, BOOL byPosition = FALSE) const;
        BOOL InsertPopupMenu(UINT pos, UINT flags, HMENU popupMenu, LPCTSTR newItemName) const;
        BOOL ModifyMenu(UINT pos, UINT flags, UINT_PTR idOrHandle = 0, LPCTSTR newItemName = nullptr) const;
        BOOL ModifyMenu(UINT pos, UINT flags, UINT_PTR idOrHandle, HBITMAP bitmap) const;
        BOOL RemoveMenu(UINT pos, UINT flags) const;
        BOOL SetDefaultItem(UINT idOrPos, BOOL byPosition = FALSE) const;
        BOOL SetMenuContextHelpId(DWORD contextHelpID) const;
        BOOL SetMenuInfo(const MENUINFO& mi) const;
        BOOL SetMenuItemBitmaps(UINT pos, UINT flags, HBITMAP unchecked, HBITMAP checked) const;
        BOOL SetMenuItemInfo(UINT idOrPos, MENUITEMINFO& menuItemInfo, BOOL byPosition = FALSE) const;

        // Operators
        operator HMENU () const;

    private:
        void AddToMap() const;
        void Assign(HMENU menu);
        void Release();
        BOOL RemoveFromMap() const;
        std::shared_ptr<CMenu_Data> m_pData;

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
        TRACE("*** Warning: GetSizeofMenuItemInfo is deprecated. ***\n");
        return sizeof(MENUITEMINFO);
    }

    ////////////////////////////////////////
    // Definitions of CMenu
    //
    inline CMenu::CMenu()
    {
        m_pData = std::make_shared<CMenu_Data>();
    }

    inline CMenu::CMenu(UINT id)
    {
        m_pData = std::make_shared<CMenu_Data>();
        HMENU menu = ::LoadMenu(GetApp()->GetResourceHandle(), MAKEINTRESOURCE(id));
        if (menu != nullptr)
        {
            Assign(menu);
        }
    }

    inline CMenu::CMenu(HMENU menu)
    {
        m_pData = std::make_shared<CMenu_Data>();
        if (menu != nullptr)
            Attach(menu);
    }

    // Note: A copy of a CMenu is a clone of the original.
    //       Both objects manipulate the same HMENU and m_pData.
    inline CMenu::CMenu(const CMenu& rhs)
    {
        m_pData = rhs.m_pData;
    }

    // Note: A copy of a CMenu is a clone of the original.
    inline CMenu& CMenu::operator=(const CMenu& rhs)
    {
        if (this != &rhs)
        {
            Release();
            m_pData = rhs.m_pData;
        }

        return *this;
    }

    inline CMenu& CMenu::operator=(HMENU menu)
    {
        Attach(menu);
        return *this;
    }

    inline CMenu::~CMenu()
    {
        Release();
    }

    // Store the HMENU and CMenu pointer in the HMENU map.
    inline void CMenu::AddToMap() const
    {
        assert(m_pData);
        assert(IsMenu(m_pData->menu));

        GetApp()->AddCMenuData(m_pData->menu, m_pData);
    }

    // Destroys m_pData if the reference count is zero.
    inline void CMenu::Release()
    {
        assert(m_pData);

        if (CWinApp::SetnGetThis())
            CThreadLock mapLock(GetApp()->m_gdiLock);

        if (m_pData.use_count() == 1)
        {
            if (m_pData->menu != nullptr)
            {
                if (m_pData->isManagedMenu)
                {
                    // Menu will already be destroyed if assigned to a destroyed window.
                    if (IsMenu(m_pData->menu))
                        ::DestroyMenu(m_pData->menu);
                }

                RemoveFromMap();
            }

            m_pData = nullptr;
        }
    }

    inline BOOL CMenu::RemoveFromMap() const
    {
        assert(m_pData);
        BOOL success = FALSE;

        if (CWinApp::SetnGetThis() != nullptr)    // Is the CWinApp object still valid?
        {
            CThreadLock mapLock(GetApp()->m_wndLock);

            // Erase the CMenu data pointer in the map.
            auto m = GetApp()->m_mapCMenuData.find(m_pData->menu);
            if (m != GetApp()->m_mapCMenuData.end())
            {
                // Erase the CMenu data pointer from the map.
                GetApp()->m_mapCMenuData.erase(m);
                success = TRUE;
            }
        }

        return success;
    }

    // Appends a new item to the end of the specified menu bar, drop-down menu, submenu, or shortcut menu.
    // Refer to AppendMenu in the Windows API documentation for more information.
    inline BOOL CMenu::AppendMenu(UINT flags, UINT_PTR idOrHandle /*= 0*/, LPCTSTR newItemName /*= nullptr*/)
    {
        assert(m_pData);
        assert(IsMenu(m_pData->menu));

        return ::AppendMenu(m_pData->menu, flags, idOrHandle, newItemName);
    }

    // Appends a new item to the end of the specified menu bar, drop-down menu, submenu, or shortcut menu.
    // Refer to AppendMenu in the Windows API documentation for more information.
    inline BOOL CMenu::AppendMenu(UINT flags, UINT_PTR idOrHandle, HBITMAP bitmap)
    {
        assert(m_pData);
        assert(IsMenu(m_pData->menu));

        return ::AppendMenu(m_pData->menu, flags, idOrHandle, reinterpret_cast<LPCTSTR>(bitmap));
    }

    // Attaches and owns the menu handle.
    inline void CMenu::Assign(HMENU menu)
    {
        CThreadLock mapLock(GetApp()->m_gdiLock);
        Attach(menu);
        m_pData->isManagedMenu = true;
    }

    // Attaches an existing menu to this CMenu.
    inline void CMenu::Attach(HMENU menu)
    {
        CThreadLock mapLock(GetApp()->m_wndLock);
        assert(m_pData);

        if (menu != m_pData->menu)
        {
            // Release any existing menu.
            if (m_pData->menu != nullptr)
            {
                Release();
                m_pData = std::make_shared< CMenu_Data>();
            }

            if (menu != nullptr)
            {
                // Add the menu to this CMenu.
                std::shared_ptr<CMenu_Data> pCMenuData = GetApp()->GetCMenuData(menu).lock();
                if (pCMenuData)
                {
                    m_pData = pCMenuData;
                }
                else
                {
                    m_pData->menu = menu;
                    AddToMap();
                }
            }
        }
    }

    // Sets the state of the specified menu item's check-mark attribute to either selected or clear.
    // Refer to CheckMenuItem in the Windows API documentation for more information.
    inline UINT CMenu::CheckMenuItem(UINT checkItemID, UINT check) const
    {
        assert(m_pData);
        assert(IsMenu(m_pData->menu));

        return ::CheckMenuItem(m_pData->menu, checkItemID, check);
    }

    // Checks a specified menu item and makes it a radio item. At the same time, the function clears
    // all other menu items in the associated group and clears the radio-item type flag for those items.
    // Refer to CheckMenuRadioItem in the Windows API documentation for more information.
    inline BOOL CMenu::CheckMenuRadioItem(UINT firstID, UINT lastID, UINT itemID, UINT flags) const
    {
        assert(m_pData);
        assert(IsMenu(m_pData->menu));

        return ::CheckMenuRadioItem(m_pData->menu, firstID, lastID, itemID, flags);
    }

    // Creates an empty menu.
    // Refer to CreateMenu in the Windows API documentation for more information.
    inline void CMenu::CreateMenu()
    {
        assert(m_pData);

        HMENU menu = ::CreateMenu();
        if (menu == nullptr)
            throw CResourceException(GetApp()->MsgMenu());

        Assign(menu);
    }

    // Creates a drop-down menu, submenu, or shortcut menu. The menu is initially empty.
    // Refer to CreatePopupMenu in the Windows API documentation for more information.
    inline void CMenu::CreatePopupMenu()
    {
        assert(m_pData);

        HMENU menu = ::CreatePopupMenu();
        if (menu == nullptr)
            throw CResourceException(GetApp()->MsgMenu());

        Assign(menu);
    }

    // Deletes an item from the specified menu.
    // Refer to DeleteMenu in the Windows API documentation for more information.
    inline BOOL CMenu::DeleteMenu(UINT pos, UINT flags) const
    {
        assert(m_pData);
        assert(IsMenu(m_pData->menu));

        return ::DeleteMenu(m_pData->menu, pos, flags);
    }

    // Destroys the menu and frees any memory that the menu occupies.
    // Refer to DestroyMenu in the Windows API documentation for more information.
    inline void CMenu::DestroyMenu()
    {
        CThreadLock mapLock(GetApp()->m_gdiLock);
        assert(m_pData);
        assert(IsMenu(m_pData->menu));

        if (m_pData && m_pData->menu != nullptr)
        {
            RemoveFromMap();

            ::DestroyMenu(m_pData->menu);
            m_pData->menu = nullptr;
            m_pData->isManagedMenu = false;
        }
    }

    // Detaches the HMENU from this CMenu object and all its copies.
    // The CMenu object and its copies are returned to the default state.
    // Note: We rarely need to detach the HMENU from a CMenu. The framework will
    //       delete the HMENU automatically if required when the last copy of
    //       the CMenu goes out of scope.
    inline HMENU CMenu::Detach()
    {
        CThreadLock mapLock(GetApp()->m_gdiLock);
        assert(m_pData);

        HMENU menu = m_pData->menu;
        RemoveFromMap();

        // Nullify all copies of m_pData.
        *m_pData.get() = {};

        // Make a new shared_ptr for this object.
        m_pData = std::make_shared<CMenu_Data>();

        return menu;
    }

    // Returns the HMENU assigned to this CMenu.
    inline HMENU CMenu::GetHandle() const
    {
        assert(m_pData);
        return m_pData->menu;
    }

    // Enables, disables, or grays the specified menu item.
    // The enable parameter must be a combination of either MF_BYCOMMAND or MF_BYPOSITION
    // and MF_ENABLED, MF_DISABLED, or MF_GRAYED.
    // MF_DISABLED and MF_GRAYED are different without XP themes, but the same with XP themes enabled.
    // Refer to EnableMenuItem in the Windows API documentation for more information.
    inline UINT CMenu::EnableMenuItem(UINT enableItemID, UINT enable) const
    {
        assert(m_pData);
        assert(IsMenu(m_pData->menu));

        return static_cast<UINT>(::EnableMenuItem(m_pData->menu, enableItemID, enable));
    }

    // Finds the position of a menu item with the specified string.
    // Returns -1 if the string is not found.
    inline int CMenu::FindMenuItem(LPCTSTR menuName) const
    {
        assert(m_pData);
        assert(IsMenu(m_pData->menu));

        CString str;
        int item = -1;
        int count = GetMenuItemCount();
        for (int i = 0; i < count; i++)
        {
            if (GetMenuString(static_cast<UINT>(i), str, MF_BYPOSITION))
            {
                if (str == menuName)
                    item = i;
            }
        }

        return item;
    }

    // Determines the default menu item.
    // The flags parameter specifies how the function searches for menu items.
    // This parameter can be zero or more of the following values: GMDI_GOINTOPOPUPS; GMDI_USEDISABLED.
    // Refer to GetMenuDefaultItem in the Windows API documentation for more information.
    inline UINT CMenu::GetDefaultItem(UINT flags, BOOL byPosition /*= FALSE*/) const
    {
        assert(m_pData);
        assert(IsMenu(m_pData->menu));

        return ::GetMenuDefaultItem(m_pData->menu, static_cast<UINT>(byPosition), flags);
    }

    // Retrieves the Help context identifier associated with the menu.
    // Refer to GetMenuContextHelpId in the Windows API documentation for more information.
    inline DWORD CMenu::GetMenuContextHelpId() const
    {
        assert(m_pData);
        assert(IsMenu(m_pData->menu));

        return ::GetMenuContextHelpId(m_pData->menu);
    }

    // Retrieves the menu information.
    // Refer to GetMenuInfo in the Windows API documentation for more information.
    inline BOOL CMenu::GetMenuInfo(MENUINFO& mi) const
    {
        assert(m_pData);
        assert(IsMenu(m_pData->menu));

        return ::GetMenuInfo(m_pData->menu, &mi);
    }

    // Retrieves the number of menu items.
    // Refer to GetMenuItemCount in the Windows API documentation for more information.
    inline int CMenu::GetMenuItemCount() const
    {
        assert(m_pData);
        assert(IsMenu(m_pData->menu));

        return ::GetMenuItemCount(m_pData->menu);
    }

    // Retrieves the data assigned to the specified menu item.
    // Refer to the description of the dwItemData member of MENUITEMINFO
    // in the Windows API documentation for more information.
    inline ULONG_PTR CMenu::GetMenuItemData(UINT idOrPos, BOOL byPosition /* = FALSE*/) const
    {
        MENUITEMINFO mii{};
        mii.cbSize = sizeof(MENUITEMINFO);
        mii.fMask = MIIM_TYPE | MIIM_DATA;
        ULONG_PTR pData = 0;

        if (GetMenuItemInfo(idOrPos, mii, byPosition))
            pData = mii.dwItemData;

        return pData;
    }

    // Retrieves the menu item identifier of a menu item located at the specified position.
    // Refer to GetMenuItemID in the Windows API documentation for more information.
    inline UINT CMenu::GetMenuItemID(int pos) const
    {
        assert(m_pData);
        assert(IsMenu(m_pData->menu));

        return ::GetMenuItemID(m_pData->menu, pos);
    }

    // Retrieves information about the specified menu item.
    // Refer to GetMenuItemInfo in the Windows API documentation for more information.
    inline BOOL CMenu::GetMenuItemInfo(UINT idOrPos, MENUITEMINFO& menuItemInfo, BOOL byPosition /*= FALSE*/) const
    {
        assert(m_pData);
        assert(IsMenu(m_pData->menu));

        menuItemInfo.cbSize = sizeof(MENUITEMINFO);
        return ::GetMenuItemInfo(m_pData->menu, idOrPos, byPosition, &menuItemInfo);
    }

    // Retrieves the menu flags associated with the specified menu item.
    // Possible values for flags are: MF_BYCOMMAND or MF_BYPOSITION.
    // Refer to GetMenuState in the Windows API documentation for more information.
    inline UINT CMenu::GetMenuState(UINT idOrPos, UINT flags) const
    {
        assert(m_pData);
        assert(IsMenu(m_pData->menu));

        return ::GetMenuState(m_pData->menu, idOrPos, flags);
    }

    // Copies the text string of the specified menu item into the specified buffer.
    // Refer to GetMenuString in the Windows API documentation for more information.
    inline int CMenu::GetMenuString(UINT idOrPos, LPTSTR string, int maxCount, UINT flags) const
    {
        assert(m_pData);
        assert(IsMenu(m_pData->menu));
        assert(string != nullptr);

        return ::GetMenuString(m_pData->menu, idOrPos, string, maxCount, flags);
    }

    // Copies the text string of the specified menu item into the specified buffer.
    // Refer to GetMenuString in the Windows API documentation for more information.
    inline int CMenu::GetMenuString(UINT idOrPos, CString& string, UINT flags) const
    {
        assert(m_pData);
        assert(IsMenu(m_pData->menu));

        int n = ::GetMenuString(m_pData->menu, idOrPos, string.GetBuffer(WXX_MAX_STRING_SIZE), WXX_MAX_STRING_SIZE, flags);
        string.ReleaseBuffer();
        return n;
    }

    // Retrieves the CMenu object of a drop-down menu or submenu activated by the specified menu item.
    // Refer to GetSubMenu in the Windows API documentation for more information.
    inline CMenu CMenu::GetSubMenu(int pos) const
    {
        assert(m_pData);
        assert(IsMenu(m_pData->menu));

        return CMenu(::GetSubMenu(m_pData->menu, pos));
    }

    // Inserts a new menu item into a menu, moving other items down the menu.
    // Refer to InsertMenu in the Windows API documentation for more information.
    inline BOOL CMenu::InsertMenu(UINT pos, UINT flags, UINT_PTR idOrHandle /*= 0*/, LPCTSTR newItemName /*= nullptr*/) const
    {
        assert(m_pData);
        assert(IsMenu(m_pData->menu));

        return ::InsertMenu(m_pData->menu, pos, flags, idOrHandle, newItemName);
    }

    // Inserts a new menu item into a menu, moving other items down the menu.
    // Refer to InsertMenu in the Windows API documentation for more information.
    inline BOOL CMenu::InsertMenu(UINT pos, UINT flags, UINT_PTR idOrHandle, HBITMAP bitmap) const
    {
        assert(m_pData);
        assert(IsMenu(m_pData->menu));

        return ::InsertMenu(m_pData->menu, pos, flags, idOrHandle, reinterpret_cast<LPCTSTR>(bitmap));
    }

    // Inserts a new menu item at the specified position in the menu.
    // Refer to InsertMenuItem in the Windows API documentation for more information.
    inline BOOL CMenu::InsertMenuItem(UINT idOrPos, MENUITEMINFO& menuItemInfo, BOOL byPosition /*= FALSE*/) const
    {
        assert(m_pData);
        assert(IsMenu(m_pData->menu));

        menuItemInfo.cbSize = sizeof(MENUITEMINFO);
        return ::InsertMenuItem(m_pData->menu, idOrPos, byPosition, &menuItemInfo);
    }

    // Inserts a popup menu item at the specified position in the menu.
    // Refer to InsertMenu in the Windows API documentation for more information.
    inline BOOL CMenu::InsertPopupMenu(UINT pos, UINT flags, HMENU popupMenu, LPCTSTR newItemName) const
    {
        assert(popupMenu);
        assert(m_pData);
        assert(IsMenu(m_pData->menu));

        // Ensure the correct flags are set.
        flags &= ~MF_BITMAP;
        flags &= ~MF_OWNERDRAW;
        flags |= MF_POPUP;

        return ::InsertMenu(m_pData->menu, pos, flags, (UINT_PTR)popupMenu, newItemName);
    }

    // Loads the menu from the specified windows resource.
    // Refer to LoadMenu in the Windows API documentation for more information.
    inline BOOL CMenu::LoadMenu(LPCTSTR resourceName)
    {
        assert(m_pData);
        assert(m_pData->menu == nullptr);
        assert(resourceName);

        HMENU menu = ::LoadMenu(GetApp()->GetResourceHandle(), resourceName);
        if (menu != nullptr)
        {
            Assign(menu);
        }

        return m_pData->menu != nullptr;
    }

    // Loads the menu from the specified windows resource.
    // Refer to LoadMenu in the Windows API documentation for more information.
    inline BOOL CMenu::LoadMenu(UINT resourceID)
    {
        assert(m_pData);
        assert(m_pData->menu == nullptr);

        HMENU menu = ::LoadMenu(GetApp()->GetResourceHandle(), MAKEINTRESOURCE(resourceID));
        if (menu != nullptr)
        {
            Assign(menu);
        }

        return m_pData->menu != nullptr;
    }

    // Loads the specified menu template and assigns it to this CMenu.
    // Refer to LoadMenuIndirect in the Windows API documentation for more information.
    inline BOOL CMenu::LoadMenuIndirect(const LPMENUTEMPLATE pMenuTemplate)
    {
        assert(m_pData);
        assert(pMenuTemplate);

        HMENU menu = ::LoadMenuIndirect(pMenuTemplate);
        if (menu != nullptr)
        {
            Assign(menu);
        }

        return m_pData->menu ? TRUE : FALSE;
    }

    // Changes an existing menu item. This function is used to specify the content, appearance, and behavior of the menu item.
    // Refer to ModifyMenu in the Windows API documentation for more information.
    inline BOOL CMenu::ModifyMenu(UINT pos, UINT flags, UINT_PTR idOrHandle /*= 0*/, LPCTSTR newItemName /*= nullptr*/) const
    {
        assert(m_pData);
        assert(IsMenu(m_pData->menu));

        return ::ModifyMenu(m_pData->menu, pos, flags, idOrHandle, newItemName);
    }

    // Changes an existing menu item. This function is used to specify the content, appearance, and behavior of the menu item.
    // Refer to ModifyMenu in the Windows API documentation for more information.
    inline BOOL CMenu::ModifyMenu(UINT pos, UINT flags, UINT_PTR idOrHandle, HBITMAP bitmap) const
    {
        assert(m_pData);
        assert(IsMenu(m_pData->menu));

        return ::ModifyMenu(m_pData->menu, pos, flags, idOrHandle, reinterpret_cast<LPCTSTR>(bitmap));
    }

    // Deletes a menu item or detaches a submenu from the menu.
    // Refer to RemoveMenu in the Windows API documentation for more information.
    inline BOOL CMenu::RemoveMenu(UINT pos, UINT flags) const
    {
        assert(m_pData);
        assert(IsMenu(m_pData->menu));

        return ::RemoveMenu(m_pData->menu, pos, flags);
    }

    //  sets the default menu item for the menu.
    // Refer to SetMenuDefaultItem in the Windows API documentation for more information.
    inline BOOL CMenu::SetDefaultItem(UINT idOrPos, BOOL byPosition /*= FALSE*/) const
    {
        assert(m_pData);
        assert(IsMenu(m_pData->menu));

        return ::SetMenuDefaultItem(m_pData->menu, idOrPos, static_cast<UINT>(byPosition));
    }

    // Associates a Help context identifier with the menu.
    // Refer to SetMenuContextHelpId in the Windows API documentation for more information.
    inline BOOL CMenu::SetMenuContextHelpId(DWORD contextHelpID) const
    {
        assert(m_pData);
        assert(IsMenu(m_pData->menu));

        return ::SetMenuContextHelpId(m_pData->menu, contextHelpID);
    }

    // Associates the specified bitmap with a menu item.
    // Refer to SetMenuItemBitmaps in the Windows API documentation for more information.
    inline BOOL CMenu::SetMenuItemBitmaps(UINT pos, UINT flags, HBITMAP unchecked, HBITMAP checked) const
    {
        assert(m_pData);
        assert(IsMenu(m_pData->menu));

        return ::SetMenuItemBitmaps(m_pData->menu, pos, flags, unchecked, checked);
    }

    // Sets the menu information from the specified MENUINFO structure.
    // Refer to SetMenuInfo in the Windows API documentation for more information.
    inline BOOL CMenu::SetMenuInfo(const MENUINFO& mi) const
    {
        assert(m_pData);
        assert(IsMenu(m_pData->menu));

        return ::SetMenuInfo(m_pData->menu, &mi);
    }

    // Changes information about a menu item.
    // Refer to SetMenuItemInfo in the Windows API documentation for more information.
    inline BOOL CMenu::SetMenuItemInfo(UINT idOrPos, MENUITEMINFO& menuItemInfo, BOOL byPosition /*= FALSE*/) const
    {
        assert(m_pData);
        assert(IsMenu(m_pData->menu));

        menuItemInfo.cbSize = sizeof(MENUITEMINFO);
        return ::SetMenuItemInfo(m_pData->menu, idOrPos, byPosition, &menuItemInfo);
    }

    // Displays a shortcut menu at the specified location and tracks the selection of items on the menu.
    // Refer to TrackPopupMenu in the Windows API documentation for more information.
    inline BOOL CMenu::TrackPopupMenu(UINT flags, int x, int y, HWND wnd, LPCRECT pRect /*= nullptr*/) const
    {
        assert(m_pData);
        assert(IsMenu(m_pData->menu));

        return ::TrackPopupMenu(m_pData->menu, flags, x, y, 0, wnd, pRect);
    }

    // Displays a shortcut menu at the specified location and tracks the selection of items on the shortcut menu.
    // Refer to TrackPopupMenuEx in the Windows API documentation for more information.
    inline BOOL CMenu::TrackPopupMenuEx(UINT flags, int x, int y, HWND wnd, LPTPMPARAMS pTPMP) const
    {
        assert(m_pData);
        assert(IsMenu(m_pData->menu));

        return ::TrackPopupMenuEx(m_pData->menu, flags, x, y, wnd, pTPMP);
    }

    // Retrieves the menu's handle.
    inline CMenu::operator HMENU () const
    {
        return GetHandle();
    }

}   // namespace Win32xx

#endif  // _WIN32XX_MENU_H_

