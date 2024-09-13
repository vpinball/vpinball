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



#ifndef _WIN32XX_LISTVIEW_H_
#define _WIN32XX_LISTVIEW_H_

#include "wxx_wincore.h"


namespace Win32xx
{

    /////////////////////////////////////////////////////////////
    // ClistView manages a List View control. A list-view control
    // is a window that displays a collection of items. Each item
    // consists of an icon and a label.
    class CListView : public CWnd
    {
    public:
        CListView() {}
        virtual ~CListView() override {}
        virtual void OnAttach() override;
        virtual void PreCreate(CREATESTRUCT& cs) override;
        virtual void PreRegisterClass(WNDCLASS& wc) override;

        // Accessors and mutators
        CSize   ApproximateViewRect(CSize sz = CSize(-1, -1), int count = -1) const;
        COLORREF GetBkColor( ) const;
        BOOL    GetBkImage( LVBKIMAGE& image ) const;
        UINT    GetCallbackMask( ) const;
        BOOL    GetCheckState( int item ) const;
        BOOL    GetColumn( int col, LVCOLUMN& colInfo ) const;
        BOOL    GetColumnOrderArray( LPINT pArrayOfCol, int count = -1 ) const;
        int     GetColumnWidth( int col ) const;
        int     GetCountPerPage( ) const;
        HWND    GetEditControl( ) const;
        DWORD   GetExtendedStyle( ) const;
        HWND    GetHeader( ) const;
        HCURSOR GetHotCursor( );
        int     GetHotItem( ) const;
        DWORD   GetHoverTime( ) const;
        CImageList GetImageList( int imageType ) const;
        BOOL    GetItem( LVITEM& itemInfo ) const;
        int     GetItemCount( ) const;
        DWORD_PTR GetItemData( int item ) const;
        BOOL    GetItemPosition( int item, CPoint& pt ) const;
        BOOL    GetItemRect( int item, RECT& rc, UINT code ) const;
        UINT    GetItemState( int item, UINT mask ) const;
        CString GetItemText( int item, int subItem, int textMax = 260 ) const;
        int     GetNextItem( int item, int flags ) const;
        UINT    GetNumberOfWorkAreas( ) const;
        BOOL    GetOrigin( CPoint& pt ) const;
        UINT    GetSelectedCount( ) const;
        int     GetSelectionMark( ) const;
        int     GetStringWidth( LPCTSTR string ) const;
        BOOL    GetSubItemRect( int item, int subItem, int code, RECT& rc ) const;
        COLORREF GetTextBkColor( ) const;
        COLORREF GetTextColor( ) const;
        HWND    GetToolTips( ) const;
        int     GetTopIndex( ) const;
        BOOL    GetViewRect( RECT& rc ) const;
        void    GetWorkAreas( int workAreas, LPRECT pRectArray ) const;
        BOOL    SetBkColor( COLORREF color ) const;
        BOOL    SetBkImage( LVBKIMAGE& pImage ) const;
        BOOL    SetCallbackMask( UINT mask ) const;
        void    SetCheckState( int item, BOOL checked = TRUE ) const;
        BOOL    SetColumn( int col, const LVCOLUMN& columnInfo ) const;
        BOOL    SetColumnOrderArray( int count, LPINT pArrayOfCol) const;
        BOOL    SetColumnWidth( int col, int cx ) const;
        DWORD   SetExtendedStyle( DWORD exStyle ) const;
        HCURSOR SetHotCursor( HCURSOR cursor ) const;
        int     SetHotItem( int index ) const;
        DWORD   SetHoverTime( DWORD hoverTime = static_cast<DWORD>(-1) ) const;
        CSize   SetIconSpacing( int cx, int cy ) const;
        CSize   SetIconSpacing( CSize sz ) const;
        CImageList SetImageList( HIMAGELIST images, int imageListType );
        BOOL    SetItem( LVITEM& itemInfo ) const;
        BOOL    SetItem( int item, int subItem, UINT mask, LPCTSTR text, int image,
                        UINT state, UINT stateMask, LPARAM lparam, int indent ) const;
        void    SetItemCount( int count ) const;
        void    SetItemCountEx( int count, DWORD flags = LVSICF_NOINVALIDATEALL ) const;
        BOOL    SetItemData( int item, DWORD_PTR data ) const;
        BOOL    SetItemPosition( int item, CPoint& pt ) const;
        BOOL    SetItemState( int item, LVITEM& itemInfo ) const;
        void    SetItemState( int item, UINT state, UINT mask ) const;
        void    SetItemText( int item, int subItem, LPCTSTR text ) const;
        int     SetSelectionMark( int index ) const;
        BOOL    SetTextBkColor( COLORREF color ) const;
        BOOL    SetTextColor( COLORREF color ) const;
        HWND    SetToolTips ( HWND toolTip ) const;
        void    SetWorkAreas( int workAreas, LPCRECT pRectArray ) const;
        int     SubItemHitTest( LVHITTESTINFO& hitInfo) const;

        // Operations
        BOOL    Arrange( UINT code ) const;
        CImageList CreateDragImage( int item, CPoint& pt ) const;
        BOOL    DeleteAllItems( ) const;
        BOOL    DeleteColumn( int col ) const;
        BOOL    DeleteItem( int item ) const;
        HWND    EditLabel( int item ) const;
        BOOL    EnsureVisible( int item, BOOL isPartialOK ) const;
        int     FindItem( LVFINDINFO& findInfo, int start = -1 ) const;
        int     HitTest( LVHITTESTINFO& hitTestInfo ) const;
        int     HitTest( CPoint pt, UINT* flags = nullptr ) const;
        int     InsertColumn( int col, const LVCOLUMN& colInfo ) const;
        int     InsertColumn( int col, LPCTSTR columnHeading, int format = LVCFMT_LEFT,
                            int width = -1, int subItem = -1 ) const;
        int     InsertItem( const LVITEM& itemInfo ) const;
        int     InsertItem( int item, LPCTSTR text ) const;
        int     InsertItem( int item, LPCTSTR text, int image ) const;
        int     InsertItem( UINT mask, int item, LPCTSTR text, UINT state,
                            UINT stateMask, int image, LPARAM lparam ) const;
        BOOL    RedrawItems( int first, int last ) const;
        BOOL    Scroll( CSize sz ) const;
        BOOL    SortItems( PFNLVCOMPARE pCompareFn, DWORD_PTR data ) const;
        BOOL    Update( int item ) const;

    private:
        CListView(const CListView&) = delete;
        CListView& operator=(const CListView&) = delete;

        CImageList m_normalImages;
        CImageList m_smallImages;
        CImageList m_stateImages;
    };

}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

namespace Win32xx
{

    // Calculates the approximate width and height required to display a given number of items.
    // Refer to ListView_ApproximateViewRect in the Windows API documentation for more information.
    inline CSize CListView::ApproximateViewRect(CSize sz /*= CSize(-1, -1)*/, int count /* = -1*/) const
    {
        assert(IsWindow());
        return CSize( ListView_ApproximateViewRect( *this, sz.cx, sz.cy, count ) );
    }

    // Arranges items in icon view.
    // Refer to ListView_Arrange in the Windows API documentation for more information.
    inline BOOL CListView::Arrange(UINT code) const
    {
        assert(IsWindow());
        return ListView_Arrange(*this, code);
    }

    // Creates a drag image list for the specified item.
    // Refer to ListView_CreateDragImage in the Windows API documentation for more information.
    inline CImageList CListView::CreateDragImage(int item, CPoint& pt) const
    {
        assert(IsWindow());
        CImageList images;
        images.CreateDragImage(*this, item, pt);
        return images;
    }

    // Removes all items from the list-view control.
    // Refer to ListView_DeleteAllItems in the Windows API documentation for more information.
    inline BOOL CListView::DeleteAllItems() const
    {
        assert(IsWindow());
        return ListView_DeleteAllItems(*this);
    }

    // Removes a column from the list-view control.
    // Refer to ListView_DeleteColumn in the Windows API documentation for more information.
    inline BOOL CListView::DeleteColumn(int col) const
    {
        assert(IsWindow());
        return ListView_DeleteColumn(*this, col);
    }

    // Removes an item from the list-view control.
    // Refer to ListView_DeleteItem in the Windows API documentation for more information.
    inline BOOL CListView::DeleteItem(int item) const
    {
        assert(IsWindow());
        return ListView_DeleteItem(*this, item);
    }

    // Begins in-place editing of the list-view item's text.
    // Refer to ListView_EditLabel in the Windows API documentation for more information.
    inline HWND CListView::EditLabel(int item) const
    {
        assert(IsWindow());
        return ListView_EditLabel(*this, item);
    }

    // Ensures that a list-view item is either entirely or partially visible,
    // scrolling the list-view control if necessary.
    // Refer to LVM_ENSUREVISIBLE in the Windows API documentation for more information.
    inline BOOL CListView::EnsureVisible(int item, BOOL isPartialOK) const
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(item);
        LPARAM lparam = static_cast<LPARAM>(isPartialOK);
        return static_cast<BOOL>(SendMessage(LVM_ENSUREVISIBLE, wparam, lparam));
    }

    // Searches for a list-view item with the specified characteristics.
    // Refer to ListView_FindItem in the Windows API documentation for more information.
    inline int CListView::FindItem(LVFINDINFO& findInfo, int start /*= -1*/) const
    {
        assert(IsWindow());
        return ListView_FindItem(*this, start, &findInfo);
    }

    // Retrieves the background color of the list-view control.
    // Refer to ListView_GetBkColor in the Windows API documentation for more information.
    inline COLORREF CListView::GetBkColor( ) const
    {
        assert(IsWindow());
        return ListView_GetBkColor( *this );
    }

    // Retrieves the background image in the list-view control.
    // Refer to ListView_GetBkImage in the Windows API documentation for more information.
    inline BOOL CListView::GetBkImage( LVBKIMAGE& image ) const
    {
        assert(IsWindow());
        return ListView_GetBkImage( *this, &image );
    }

    // Retrieves the callback mask for the list-view control.
    // Refer to ListView_GetCallbackMask in the Windows API documentation for more information.
    inline UINT CListView::GetCallbackMask( ) const
    {
        assert(IsWindow());
        return static_cast<UINT>(ListView_GetCallbackMask( *this ));
    }

    // Determines if an item in the list-view control is selected.
    // Refer to ListView_GetCheckState in the Windows API documentation for more information.
    inline BOOL CListView::GetCheckState( int item ) const
    {
        assert(IsWindow());
        return static_cast<BOOL>(ListView_GetCheckState( *this, item ));
    }

    // Retrieves the attributes of the list-view control's column.
    // Refer to ListView_GetColumn in the Windows API documentation for more information.
    inline BOOL CListView::GetColumn( int col, LVCOLUMN& colInfo ) const
    {
        assert(IsWindow());
        return ListView_GetColumn( *this, col, &colInfo);
    }

    // Retrieves the current left-to-right order of columns in the list-view control.
    // Refer to ListView_GetColumnOrderArray in the Windows API documentation for more information.
    inline BOOL CListView::GetColumnOrderArray( LPINT pArrayOfCol, int count /*= -1*/ ) const
    {
        assert(IsWindow());
        return ListView_GetColumnOrderArray( *this, count, pArrayOfCol);
    }

    // Retrieves the width of a column in report or list view.
    // Refer to ListView_GetColumnWidth in the Windows API documentation for more information.
    inline int CListView::GetColumnWidth( int col ) const
    {
        assert(IsWindow());
        return ListView_GetColumnWidth( *this, col );
    }

    // Calculates the number of items that can fit vertically in the visible area of the
    // list-view control when in list or report view. Only fully visible items are counted.
    // Refer to ListView_GetCountPerPage in the Windows API documentation for more information.
    inline int CListView::GetCountPerPage( ) const
    {
        assert(IsWindow());
        return ListView_GetCountPerPage( *this );
    }

    // Retrieves the handle to the edit control being used to edit the list-view item's text.
    // Refer to ListView_GetEditControl in the Windows API documentation for more information.
    inline HWND CListView::GetEditControl( ) const
    {
        assert(IsWindow());
        return ListView_GetEditControl(*this);
    }

    // Retrieves the extended styles that are currently in use for the list-view control.
    // Refer to ListView_GetExtendedListViewStyle in the Windows API documentation for more information.
    inline DWORD CListView::GetExtendedStyle( ) const
    {
        assert(IsWindow());
        return ListView_GetExtendedListViewStyle( *this );
    }

    // Retrieves the handle to the header control used by the list-view control.
    // Refer to ListView_GetHeader in the Windows API documentation for more information.
    inline HWND CListView::GetHeader( ) const
    {
        assert(IsWindow());
        return ListView_GetHeader(*this);
    }

    // Retrieves the HCURSOR used when the pointer is over an item while hot tracking is enabled.
    // Refer to ListView_GetHotCursor in the Windows API documentation for more information.
    inline HCURSOR CListView::GetHotCursor( )
    {
        assert(IsWindow());
        return ListView_GetHotCursor( *this );
    }

    // Retrieves the index of the hot item.
    // Refer to ListView_GetHotItem in the Windows API documentation for more information.
    inline int CListView::GetHotItem( ) const
    {
        assert(IsWindow());
        return ListView_GetHotItem( *this );
    }

    // Retrieves the amount of time that the mouse cursor must hover over an item before it is selected.
    // Refer to ListView_GetHoverTime in the Windows API documentation for more information.
    inline DWORD CListView::GetHoverTime( ) const
    {
        assert(IsWindow());
        return ListView_GetHoverTime( *this );
    }

    // Retrieves the handle to an image list used for drawing list-view items.
    // imageType:  LVSIL_NORMAL; LVSIL_SMALL; or LVSIL_STATE.
    // Refer to ListView_GetImageList in the Windows API documentation for more information.
    inline CImageList CListView::GetImageList( int imageType ) const
    {
        assert(IsWindow());
        HIMAGELIST images = ListView_GetImageList( *this, imageType );
        return CImageList(images);
    }

    // Retrieves some or all of the list-view item's attributes.
    // Refer to ListView_GetItem in the Windows API documentation for more information.
    inline BOOL CListView::GetItem( LVITEM& itemInfo ) const
    {
        assert(IsWindow());
        return ListView_GetItem( *this, &itemInfo );
    }

    // Retrieves the number of items in the list-view control.
    // Refer to ListView_GetItemCount in the Windows API documentation for more information.
    inline int CListView::GetItemCount( ) const
    {
        assert(IsWindow());
        return ListView_GetItemCount( *this );
    }

    // Retrieves the value(lparam) specific to the item.
    // Refer to LVM_GETITEM in the Windows API documentation for more information.
    inline DWORD_PTR CListView::GetItemData( int item ) const
    {
        assert(IsWindow());

        LVITEM lvi{};
        lvi.iItem = item;
        lvi.mask = LVIF_PARAM;
        SendMessage(LVM_GETITEM, 0, reinterpret_cast<LPARAM>(&lvi));
        return static_cast<DWORD_PTR>(lvi.lParam);
    }

    // Retrieves the position of the list-view item.
    // Refer to ListView_GetItemPosition in the Windows API documentation for more information.
    inline BOOL CListView::GetItemPosition( int item, CPoint& pt ) const
    {
        assert(IsWindow());
        return ListView_GetItemPosition( *this, item, &pt );
    }

    // Retrieves the bounding rectangle for all or part of an item in the current view.
    // code: LVIR_BOUNDS; LVIR_ICON; LVIR_LABEL; or LVIR_SELECTBOUNDS.
    // Refer to ListView_GetItemRect in the Windows API documentation for more information.
    inline BOOL CListView::GetItemRect( int item, RECT& rc, UINT code ) const
    {
        assert(IsWindow());
        return ListView_GetItemRect( *this, item, &rc, static_cast<LONG>(code) );
    }

    // Retrieves the state of the list-view item.
    // Possible values of mask:
    // LVIS_CUT             The item is marked for a cut-and-paste operation.
    // LVIS_DROPHILITED     The item is highlighted as a drag-and-drop target.
    // LVIS_FOCUSED         The item has the focus, so it is surrounded by a standard focus rectangle.
    // LVIS_SELECTED        The item is selected.
    // LVIS_OVERLAYMASK     Use this mask to retrieve the item's overlay image index.
    // LVIS_STATEIMAGEMASK  Use this mask to retrieve the item's state image index.
    // Refer to ListView_GetItemState in the Windows API documentation for more information.
    inline UINT CListView::GetItemState( int item, UINT mask ) const
    {
        assert(IsWindow());
        return  ListView_GetItemState( *this, item, mask );
    }

    // Retrieves the text of the list-view item.
    // Note: Although the list-view control allows any length string to be stored
    //       as item text, only the first 260 characters are displayed.
    // Refer to LVM_GETITEM in the Windows API documentation for more information.
    inline CString CListView::GetItemText( int item, int subItem, int textMax /* = 260 */ ) const
    {
        assert(IsWindow());

        CString str;
        if (textMax > 0)
        {
            LVITEM lvi{};
            lvi.iItem = item;
            lvi.iSubItem = subItem;
            lvi.mask = LVIF_TEXT;
            lvi.cchTextMax = textMax;
            lvi.pszText = str.GetBuffer(textMax);
            SendMessage(LVM_GETITEM, 0 , reinterpret_cast<LPARAM>(&lvi));
            str.ReleaseBuffer();
        }
        return str;
    }

    // Searches for a list-view item that has the specified properties and
    // bears the specified relationship to a specified item.
    // Refer to ListView_GetNextItem in the Windows API documentation for more information.
    inline int CListView::GetNextItem( int item, int flags ) const
    {
        assert(IsWindow());
        return ListView_GetNextItem( *this, item, flags );
    }

    // Retrieves the working areas from the list-view control.
    // Refer to LVM_GETNUMBEROFWORKAREAS in the Windows API documentation for more information.
    inline UINT CListView::GetNumberOfWorkAreas( ) const
    {
        assert(IsWindow());
        UINT workAreas = 0;
        SendMessage(LVM_GETNUMBEROFWORKAREAS, 0, reinterpret_cast<LPARAM>(&workAreas));
        return workAreas;
    }

    // Retrieves the current view origin for the list-view control.
    // Refer to ListView_GetOrigin in the Windows API documentation for more information.
    inline BOOL CListView::GetOrigin( CPoint& pt ) const
    {
        assert(IsWindow());
        return ListView_GetOrigin( *this, &pt );
    }

    // Determines the number of selected items in the list-view control.
    // Refer to LVM_GETSELECTEDCOUNT in the Windows API documentation for more information.
    inline UINT CListView::GetSelectedCount( ) const
    {
        assert(IsWindow());
        return static_cast<UINT>(SendMessage( LVM_GETSELECTEDCOUNT, 0, 0 ));
    }

    // Retrieves the selection mark from the list-view control.
    // Refer to LVM_GETSELECTIONMARK in the Windows API documentation for more information.
    inline int CListView::GetSelectionMark( ) const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage( LVM_GETSELECTIONMARK, 0, 0 ));
    }

    // Determines the width of a specified string using the list-view control's current font.
    // Refer to LVM_GETSTRINGWIDTH in the Windows API documentation for more information.
    inline int CListView::GetStringWidth( LPCTSTR string ) const
    {
        assert(IsWindow());
        LPARAM lparam = reinterpret_cast<LPARAM>(string);
        return static_cast<int>(SendMessage( LVM_GETSTRINGWIDTH, 0, lparam));
    }

    // Retrieves information about the rectangle that surrounds a subitem in the list-view control.
    // Refer to ListView_GetSubItemRect in the Windows API documentation for more information.
    inline BOOL CListView::GetSubItemRect( int item, int subItem, int code, RECT& rc ) const
    {
        assert(IsWindow());
        return ListView_GetSubItemRect( *this, item, subItem, code, &rc );
    }

    // Retrieves the text background color of the list-view control.
    // Refer to ListView_GetTextBkColor in the Windows API documentation for more information.
    inline COLORREF CListView::GetTextBkColor( ) const
    {
        assert(IsWindow());
        return ListView_GetTextBkColor( *this );
    }

    // Retrieves the text color of the list-view control.
    // Refer to ListView_GetTextColor in the Windows API documentation for more information.
    inline COLORREF CListView::GetTextColor( ) const
    {
        assert(IsWindow());
        return ListView_GetTextColor( *this );
    }

    // Retrieves the ToolTip control that the list-view control uses to display ToolTips.
    // Refer to ListView_GetToolTips in the Windows API documentation for more information.
    inline HWND CListView::GetToolTips( ) const
    {
        assert(IsWindow());
        return ListView_GetToolTips(*this);
    }

    // Retrieves the index of the topmost visible item when in list or report view.
    // Refer to ListView_GetTopIndex in the Windows API documentation for more information.
    inline int CListView::GetTopIndex( ) const
    {
        assert(IsWindow());
        return ListView_GetTopIndex( *this );
    }

    // Retrieves the bounding rectangle of all items in the list-view control.
    // Refer to ListView_GetViewRect in the Windows API documentation for more information.
    inline BOOL CListView::GetViewRect( RECT& rc ) const
    {
        assert(IsWindow());
        return ListView_GetViewRect( *this, &rc );
    }

    // Retrieves the working areas from the list-view control.
    // Refer to LVM_GETWORKAREAS in the Windows API documentation for more information.
    inline void CListView::GetWorkAreas( int workAreas, LPRECT pRectArray ) const
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(workAreas);
        LPARAM lparam = reinterpret_cast<LPARAM>(pRectArray);
        SendMessage(LVM_GETWORKAREAS, wparam, lparam);
    }

    // Determines which list-view item, if any, is at a specified position.
    // Refer to ListView_HitTest in the Windows API documentation for more information.
    inline int CListView::HitTest(LVHITTESTINFO& hitInfo) const
    {
        assert(IsWindow());
        return ListView_HitTest(*this, &hitInfo);
    }

    // Determines which list-view item, if any, is at a specified position.
    // Refer to ListView_HitTest in the Windows API documentation for more information.
    inline int CListView::HitTest(CPoint pt, UINT* pFlags /*= nullptr*/) const
    {
        assert(IsWindow());

        LVHITTESTINFO hti{};
        hti.flags = *pFlags;
        hti.pt = pt;
        return ListView_HitTest(*this, &hti);
    }

    // Inserts a new column in the list-view control.
    // Refer to ListView_InsertColumn in the Windows API documentation for more information.
    inline int CListView::InsertColumn(int col, const LVCOLUMN& colInfo) const
    {
        assert(IsWindow());
        return ListView_InsertColumn(*this, col, &colInfo);
    }

    // Inserts a new column in the list-view control.
    // format :
    //  LVCFMT_BITMAP_ON_RIGHT    The bitmap appears to the right of text.
    //  LVCFMT_CENTER             Text is centered.
    //  LVCFMT_IMAGE              The item displays an image from an image list.
    //  LVCFMT_JUSTIFYMASK        A bitmask used to select those bits of fmt that control field justification.
    //  LVCFMT_LEFT               Text is left-aligned.
    //  LVCFMT_RIGHT              Text is right-aligned.
    // Refer to ListView_InsertColumn in the Windows API documentation for more information.
    inline int CListView::InsertColumn(int col, LPCTSTR columnHeading, int format /*= LVCFMT_LEFT*/,
        int width /*= -1*/, int subItem /*= -1*/) const
    {
        assert(IsWindow());

        LVCOLUMN lvc{};
        lvc.mask = LVCF_TEXT | LVCF_ORDER | LVCF_FMT;
        if (-1 != width)
        {
            lvc.mask |= LVCF_WIDTH;
            lvc.cx = width;
        }
        if (-1 != subItem)
        {
            lvc.mask |= LVCF_SUBITEM;
            lvc.iSubItem = subItem;
        }

        lvc.iOrder = col;
        lvc.pszText = const_cast<LPTSTR>(columnHeading);
        lvc.fmt = format;
        lvc.iSubItem = subItem;
        return ListView_InsertColumn(*this, col, &lvc);
    }

    // Inserts a new item in the list-view control.
    // Refer to ListView_InsertItem in the Windows API documentation for more information.
    inline int CListView::InsertItem(const LVITEM& itemInfo) const
    {
        assert(IsWindow());
        return ListView_InsertItem(*this, &itemInfo);
    }

    // Inserts a new item in the list-view control.
    // Refer to ListView_InsertItem in the Windows API documentation for more information.
    inline int CListView::InsertItem(int item, LPCTSTR text) const
    {
        assert(IsWindow());

        LVITEM lvi{};
        lvi.iItem = item;
        lvi.pszText = const_cast<LPTSTR>(text);
        lvi.mask = LVIF_TEXT;
        return ListView_InsertItem(*this, &lvi);
    }

    // Inserts a new item in the list-view control.
    // Refer to ListView_InsertItem in the Windows API documentation for more information.
    inline int CListView::InsertItem(int item, LPCTSTR text, int image) const
    {
        assert(IsWindow());

        LVITEM lvi{};
        lvi.iItem = item;
        lvi.pszText = const_cast<LPTSTR>(text);
        lvi.iImage = image;
        lvi.mask = LVIF_TEXT | LVIF_IMAGE;
        return ListView_InsertItem(*this, &lvi);
    }

    inline int CListView::InsertItem(UINT mask, int item, LPCTSTR text, UINT state,
        UINT stateMask, int image, LPARAM lparam) const
    {
        assert(IsWindow());

        LVITEM lvi{};
        lvi.mask = mask;
        lvi.iItem = item;
        lvi.pszText = const_cast<LPTSTR>(text);
        lvi.iImage = image;
        lvi.state = state;
        lvi.stateMask = stateMask;
        lvi.lParam = lparam;
        return ListView_InsertItem(*this, &lvi);
    }

    // This function is called when a list-view window is attached to the CListView.
    inline void CListView::OnAttach()
    {
        // LVS_SHAREIMAGELISTS:
        // The image list will not be deleted when the control is destroyed.
        // Allows Win32++ to control the destruction of the image list.
        DWORD style = GetStyle();
        SetStyle(style | LVS_SHAREIMAGELISTS);
    }

    // Sets the window creation parameters.
    inline void CListView::PreCreate(CREATESTRUCT& cs)
    {
        cs.style = WS_CHILD | WS_VISIBLE | LVS_SHAREIMAGELISTS;

        // LVS_SHAREIMAGELISTS:
        // The image list will not be deleted when the control is destroyed.
        // Allows Win32++ to control the destruction of the image list.
    }

    // Sets the window class parameters.
    inline void CListView::PreRegisterClass(WNDCLASS& wc)
    {
        wc.lpszClassName = WC_LISTVIEW;
    }

    // Forces the list-view control to redraw a range of items.
    // Refer to ListView_RedrawItems in the Windows API documentation for more information.
    inline BOOL CListView::RedrawItems(int first, int last) const
    {
        assert(IsWindow());
        return ListView_RedrawItems(*this, first, last);
    }

    // Scrolls the content of the list-view control.
    // Refer to ListView_Scroll in the Windows API documentation for more information.
    inline BOOL CListView::Scroll(CSize sz) const
    {
        assert(IsWindow());
        return ListView_Scroll(*this, sz.cx, sz.cy);
    }

    // Sets the background color of the list-view control.
    // Refer to ListView_SetBkColor in the Windows API documentation for more information.
    inline BOOL CListView::SetBkColor( COLORREF bkColor ) const
    {
        assert(IsWindow());
        return ListView_SetBkColor( *this, bkColor);
    }

    // Sets the background image in the list-view control.
    // Refer to ListView_SetBkImage in the Windows API documentation for more information.
    inline BOOL CListView::SetBkImage( LVBKIMAGE& bkImage ) const
    {
        assert(IsWindow());
        return ListView_SetBkImage( *this, &bkImage);
    }

    // Changes the callback mask for the list-view control.
    // Refer to ListView_SetCallbackMask in the Windows API documentation for more information.
    inline BOOL CListView::SetCallbackMask( UINT mask ) const
    {
        assert(IsWindow());
        return ListView_SetCallbackMask( *this, mask );
    }

    // Used to select or deselect an item in the list-view control.
    // This should only be used for list-view controls with the LVS_EX_CHECKBOXES style.
    // Refer to ListView_SetItemState in the Windows API documentation for more information.
    inline void CListView::SetCheckState( int item, BOOL checked /*= TRUE*/ ) const
    {
        assert(IsWindow());
        ListView_SetItemState(*this, item, INDEXTOSTATEIMAGEMASK((checked!=FALSE)?2U:1U),LVIS_STATEIMAGEMASK);
    }

    // Sets the attributes of the list-view column.
    // Refer to ListView_SetColumn in the Windows API documentation for more information.
    inline BOOL CListView::SetColumn( int col, const LVCOLUMN& colInfo ) const
    {
        assert(IsWindow());
        return ListView_SetColumn( *this, col, &colInfo );
    }

    // Sets the left-to-right order of columns in the list-view control.
    // Refer to ListView_SetColumnOrderArray in the Windows API documentation for more information.
    inline BOOL CListView::SetColumnOrderArray( int count, LPINT pArrayOfCol) const
    {
        assert(IsWindow());
        return ListView_SetColumnOrderArray( *this, count, pArrayOfCol );
    }

    // Used to change the width of a column in report view or the width of all columns in list-view mode.
    // Refer to ListView_SetColumnWidth in the Windows API documentation for more information.
    inline BOOL CListView::SetColumnWidth( int col, int cx ) const
    {
        assert(IsWindow());
        return ListView_SetColumnWidth( *this, col, cx );
    }

    // Sets extended styles for the list-view control.
    // Refer to ListView_SetExtendedListViewStyle in the Windows API documentation for more information.
    inline DWORD CListView::SetExtendedStyle( DWORD exStyle ) const
    {
        assert(IsWindow());
        return ListView_SetExtendedListViewStyle( *this, static_cast<LPARAM>(exStyle) );
    }

    // Sets the HCURSOR that the list-view control uses when the pointer is
    // over an item while hot tracking is enabled.
    // Refer to ListView_SetHotCursor in the Windows API documentation for more information.
    inline HCURSOR CListView::SetHotCursor( HCURSOR cursor ) const
    {
        assert(IsWindow());
        return ListView_SetHotCursor( *this, cursor );
    }

    // Sets the hot item in the list-view control.
    // Refer to ListView_SetHotItem in the Windows API documentation for more information.
    inline int CListView::SetHotItem( int index ) const
    {
        assert(IsWindow());
        return ListView_SetHotItem( *this, index );
    }

    // Sets the amount of time that the mouse cursor must hover over an item before it is selected.
    // Refer to ListView_SetHoverTime in the Windows API documentation for more information.
    inline DWORD CListView::SetHoverTime( DWORD hoverTime /*= -1*/ ) const
    {
        assert(IsWindow());
        return ListView_SetHoverTime( *this, hoverTime );
    }

    // Sets the spacing between icons in list-view controls set to the LVS_ICON style.
    // Refer to ListView_SetIconSpacing in the Windows API documentation for more information.
    inline CSize CListView::SetIconSpacing( int cx, int cy ) const
    {
        assert(IsWindow());
        return CSize( ListView_SetIconSpacing( *this, cx, cy ) );
    }

    // Sets the spacing between icons in the list-view control set to the LVS_ICON style.
    // Refer to ListView_SetIconSpacing in the Windows API documentation for more information.
    inline CSize CListView::SetIconSpacing( CSize sz ) const
    {
        assert(IsWindow());
        return CSize( ListView_SetIconSpacing( *this, sz.cx, sz.cy ) );
    }

    // Assigns an image list to the list-view control.
    // Valid imageListType values: LVSIL_NORMAL, LVSIL_SMALL, LVSIL_STATE.
    // Refer to ListView_SetImageList in the Windows API documentation for more information.
    inline CImageList CListView::SetImageList( HIMAGELIST images, int imageListType )
    {
        assert(IsWindow());
        CImageList oldImages = ListView_SetImageList( *this, images, imageListType );
        if (imageListType == LVSIL_STATE)
            m_stateImages = images;
        else if (imageListType == LVSIL_SMALL)
            m_smallImages = images;
        else
            m_normalImages = images;

        return oldImages;
    }

    // Sets some or all of a list-view item's attributes.
    // The declaration for TVITEM:
    //  typedef struct _LVITEM {
    //      UINT mask;
    //      int iItem;
    //      int iSubItem;
    //      UINT state;
    //      UINT stateMask;
    //      LPTSTR pText;
    //      int cchTextMax;
    //      int iImage;
    //      LPARAM lparam;
    // } LVITEM, *LVITEM&;
    // Refer to ListView_SetItem in the Windows API documentation for more information.
    inline BOOL CListView::SetItem( LVITEM& itemInfo ) const
    {
        assert(IsWindow());
        return ListView_SetItem( *this, &itemInfo );
    }

    // Sets some or all of a list-view item's attributes.
    // Refer to ListView_SetItem in the Windows API documentation for more information.
    inline BOOL CListView::SetItem( int item, int subItem, UINT mask, LPCTSTR text, int image,
                    UINT state, UINT stateMask, LPARAM lparam, int indent ) const
    {
        assert(IsWindow());

        LVITEM lvi{};
        lvi.iItem = item;
        lvi.iSubItem = subItem;
        lvi.mask = mask;
        lvi.pszText = const_cast<LPTSTR>(text);
        lvi.iImage = image;
        lvi.state = state;
        lvi.stateMask = stateMask;
        lvi.lParam = lparam;
        lvi.iIndent = indent;

        return ListView_SetItem( *this, &lvi);
    }

    // Causes the list-view control to allocate memory for the specified number of items.
    // Refer to ListView_SetItemCount in the Windows API documentation for more information.
    inline void CListView::SetItemCount( int count ) const
    {
        assert(IsWindow());
        ListView_SetItemCount( *this, count );
    }

    // Sets the virtual number of items in a virtual list view.
    // Refer to ListView_SetItemCountEx in the Windows API documentation for more information.
    inline void CListView::SetItemCountEx( int count, DWORD flags /*= LVSICF_NOINVALIDATEALL*/ ) const
    {
        assert(IsWindow());
        ListView_SetItemCountEx( *this, count, flags );
    }

    // Sets the value(lparam) specific to the item.
    // Refer to ListView_SetItem in the Windows API documentation for more information.
    inline BOOL CListView::SetItemData( int item, DWORD_PTR data ) const
    {
        assert(IsWindow());

        LVITEM lvi{};
        lvi.iItem = item;
        lvi.lParam = static_cast<LPARAM>(data);
        lvi.mask = LVIF_PARAM;
        return ListView_SetItem(*this, &lvi);
    }

    // Moves an item to a specified position in the list-view control (in icon or small icon view).
    // Refer to ListView_SetItemPosition in the Windows API documentation for more information.
    inline BOOL CListView::SetItemPosition( int item, CPoint& pt ) const
    {
        assert(IsWindow());
        return ListView_SetItemPosition( *this, item, pt.x, pt.y );
    }

    // Changes the state of an item in the list-view control.
    // Possible values of the mask:
    // LVIS_CUT             The item is marked for a cut-and-paste operation.
    // LVIS_DROPHILITED     The item is highlighted as a drag-and-drop target.
    // LVIS_FOCUSED         The item has the focus, so it is surrounded by a standard focus rectangle.
    // LVIS_SELECTED        The item is selected.
    // LVIS_OVERLAYMASK     Use this mask to retrieve the item's overlay image index.
    // LVIS_STATEIMAGEMASK  Use this mask to retrieve the item's state image index.
    // Refer to ListView_SetItemState in the Windows API documentation for more information.
    inline BOOL CListView::SetItemState( int item, LVITEM& itemInfo ) const
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(item);
        LPARAM lparam = reinterpret_cast<LPARAM>(&itemInfo);
        return static_cast<BOOL>(SendMessage(LVM_SETITEMSTATE, wparam, lparam));
    }

    // Changes the state of an item in the list-view control.
    // Possible values of the state and mask:
    // LVIS_CUT             The item is marked for a cut-and-paste operation.
    // LVIS_DROPHILITED     The item is highlighted as a drag-and-drop target.
    // LVIS_FOCUSED         The item has the focus, so it is surrounded by a standard focus rectangle.
    // LVIS_SELECTED        The item is selected.
    // LVIS_OVERLAYMASK     Use this mask to retrieve the item's overlay image index.
    // LVIS_STATEIMAGEMASK  Use this mask to retrieve the item's state image index.
    // Refer to ListView_SetItemState in the Windows API documentation for more information.
    inline void CListView::SetItemState( int item, UINT state, UINT mask ) const
    {
        assert(IsWindow());
        ListView_SetItemState(*this, item, state, mask);
    }

    // Changes the text of a list-view item or subitem.
    // Refer to ListView_SetItemText in the Windows API documentation for more information.
    inline void CListView::SetItemText( int item, int subItem, LPCTSTR text ) const
    {
        assert(IsWindow());
        ListView_SetItemText(*this, item, subItem, const_cast<LPTSTR>(text) );
    }

    // Sets the selection mark in the list-view control.
    // Refer to ListView_SetSelectionMark in the Windows API documentation for more information.
    inline int CListView::SetSelectionMark( int index ) const
    {
        assert(IsWindow());
        return ListView_SetSelectionMark( *this, index );
    }

    // Sets the background color of text in the list-view control.
    // Refer to ListView_SetTextBkColor in the Windows API documentation for more information.
    inline BOOL CListView::SetTextBkColor( COLORREF color ) const
    {
        assert(IsWindow());
        return ListView_SetTextBkColor( *this, color);
    }

    // Sets the text color of the list-view control.
    // Refer to ListView_SetTextColor in the Windows API documentation for more information.
    inline BOOL CListView::SetTextColor( COLORREF color ) const
    {
        assert(IsWindow());
        return ListView_SetTextColor( *this, color);
    }

    // Sets the ToolTip control that the list-view control will use to display ToolTips.
    // Refer to LVM_SETTOOLTIPS in the Windows API documentation for more information.
    inline HWND CListView::SetToolTips( HWND toolTip ) const
    {
        assert(IsWindow());
        WPARAM wparam = reinterpret_cast<WPARAM>(toolTip);
        return reinterpret_cast<HWND>(SendMessage(LVM_SETTOOLTIPS, wparam, 0));
    }

    // Sets the working area within the list-view control.
    // Refer to LVM_SETWORKAREAS in the Windows API documentation for more information.
    inline void CListView::SetWorkAreas( int workAreas, LPCRECT pRectArray ) const
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(workAreas);
        LPARAM lparam = reinterpret_cast<LPARAM>(pRectArray);
        SendMessage(LVM_SETWORKAREAS, wparam, lparam);
    }

    // Uses an application-defined comparison function to sort the items of the list-view control.
    // Refer to ListView_SortItems in the Windows API documentation for more information.
    inline BOOL CListView::SortItems(PFNLVCOMPARE pCompareFn, DWORD_PTR data) const
    {
        assert(IsWindow());
        return ListView_SortItems(*this, pCompareFn, data);
    }

    // Determines which list-view item or subitem is located at a given position.
    // Refer to ListView_SubItemHitTest in the Windows API documentation for more information.
    inline int CListView::SubItemHitTest( LVHITTESTINFO& hitInfo ) const
    {
        assert(IsWindow());
        return ListView_SubItemHitTest( *this, &hitInfo );
    }

    // Updates a list-view item. If the list-view control has the LVS_AUTOARRANGE style,
    // the list-view control is rearranged.
    // Refer to ListView_Update in the Windows API documentation for more information.
    inline BOOL CListView::Update( int item ) const
    {
        assert(IsWindow());
        return ListView_Update( *this, item );
    }

} // namespace Win32xx

#endif // _WIN32XX_LISTVIEW_H_

