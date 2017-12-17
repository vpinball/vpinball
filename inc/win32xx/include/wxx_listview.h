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



#ifndef _WIN32XX_LISTVIEW_H_
#define _WIN32XX_LISTVIEW_H_

#include "wxx_wincore.h"
#include "wxx_controls.h"
#include <commctrl.h>


namespace Win32xx
{

    /////////////////////////////////////
    // The ClistView class provides the functionality of a List View control
    class CListView : public CWnd
    {
    public:
        CListView() {}
        virtual ~CListView() {}
        virtual void PreRegisterClass(WNDCLASS& wc);

        // Attributes
        CSize   ApproximateViewRect(CSize sz = CSize(-1, -1), int iCount = -1) const;
        COLORREF GetBkColor( ) const;
        BOOL    GetBkImage( LVBKIMAGE& lvbkImage ) const;
        UINT    GetCallbackMask( ) const;
        BOOL    GetCheckState( UINT nItem ) const;
        BOOL    GetColumn( int iCol, LVCOLUMN& Column ) const;
        BOOL    GetColumnOrderArray( LPINT piArray, int iCount = -1 ) const;
        int     GetColumnWidth( int iCol ) const;
        int     GetCountPerPage( ) const;
        HWND    GetEditControl( ) const;
        DWORD   GetExtendedStyle( ) const;
        HWND    GetHeader( ) const;
        HCURSOR GetHotCursor( );
        int     GetHotItem( ) const;
        DWORD   GetHoverTime( ) const;
        CImageList GetImageList( int nImageType ) const;
        BOOL    GetItem( LVITEM& lvItem ) const;
        int     GetItemCount( ) const;
        DWORD_PTR GetItemData( int iItem ) const;
        BOOL    GetItemPosition( int iItem, CPoint& pt ) const;
        BOOL    GetItemRect( int iItem, RECT& rc, UINT nCode ) const;
        UINT    GetItemState( int iItem, UINT nMask ) const;
        CString GetItemText( int iItem, int iSubItem, UINT nTextMax = 260 ) const;
        int     GetNextItem( int iItem, int iFlags ) const;
        UINT    GetNumberOfWorkAreas( ) const;
        BOOL    GetOrigin( CPoint& pt ) const;
        UINT    GetSelectedCount( ) const;
        int     GetSelectionMark( ) const;
        int     GetStringWidth( LPCTSTR pszString ) const;
        BOOL    GetSubItemRect( int iItem, int iSubItem, int iCode, RECT& rc ) const;
        COLORREF GetTextBkColor( ) const;
        COLORREF GetTextColor( ) const;
        HWND    GetToolTips( ) const;
        int     GetTopIndex( ) const;
        BOOL    GetViewRect( RECT& rc ) const;
        void    GetWorkAreas( int iWorkAreas, LPRECT pRectArray ) const;
        BOOL    SetBkColor( COLORREF clrBk ) const;
        BOOL    SetBkImage( LVBKIMAGE& plvbkImage ) const;
        BOOL    SetCallbackMask( UINT nMask ) const;
        void    SetCheckState( int iItem, BOOL Checked = TRUE ) const;
        BOOL    SetColumn( int iCol, const LVCOLUMN& pColumn ) const;
        BOOL    SetColumnOrderArray( int iCount, LPINT piArray ) const;
        BOOL    SetColumnWidth( int iCol, int cx ) const;
        DWORD   SetExtendedStyle( DWORD dwNewStyle ) const;
        HCURSOR SetHotCursor( HCURSOR hCursor ) const;
        int     SetHotItem( int nIndex ) const;
        DWORD   SetHoverTime( DWORD dwHoverTime = static_cast<DWORD>(-1) ) const;
        CSize   SetIconSpacing( int cx, int cy ) const;
        CSize   SetIconSpacing( CSize sz ) const;
        CImageList SetImageList( HIMAGELIST himlNew, int iImageListType ) const;
        BOOL    SetItem( LVITEM& Item ) const;
        BOOL    SetItem( int iItem, int iSubItem, UINT nMask, LPCTSTR pszText, int iImage,
                        UINT nState, UINT nStateMask, LPARAM lParam, int iIndent ) const;
        void    SetItemCount( int iCount ) const;
        void    SetItemCountEx( int iCount, DWORD dwFlags = LVSICF_NOINVALIDATEALL ) const;
        BOOL    SetItemData( int iItem, DWORD_PTR dwData ) const;
        BOOL    SetItemPosition( int iItem, CPoint& pt ) const;
        BOOL    SetItemState( int iItem, LVITEM& Item ) const;
        void    SetItemState( int iItem, UINT nState, UINT nMask ) const;
        void    SetItemText( int iItem, int iSubItem, LPCTSTR pszText ) const;
        int     SetSelectionMark( int iIndex ) const;
        BOOL    SetTextBkColor( COLORREF clrBkText ) const;
        BOOL    SetTextColor( COLORREF clrText ) const;
        HWND    SetToolTips ( HWND hToolTip ) const;
        void    SetWorkAreas( int nWorkAreas, LPCRECT pRectArray ) const;
        int     SubItemHitTest( LVHITTESTINFO& htInfo ) const;

        // Operations
        BOOL    Arrange( UINT nCode ) const;
        CImageList CreateDragImage( int iItem, CPoint& pt ) const;
        BOOL    DeleteAllItems( ) const;
        BOOL    DeleteColumn( int iCol ) const;
        BOOL    DeleteItem( int iItem ) const;
        HWND    EditLabel( int iItem ) const;
        BOOL    EnsureVisible( int iItem, BOOL IsPartialOK ) const;
        int     FindItem( LVFINDINFO& FindInfo, int iStart = -1 ) const;
        int     HitTest( LVHITTESTINFO& HitTestInfo ) const;
        int     HitTest( CPoint pt, UINT* pFlags = NULL ) const;
        int     InsertColumn( int iCol, const LVCOLUMN& pColumn ) const;
        int     InsertColumn( int iCol, LPCTSTR pszColumnHeading, int iFormat = LVCFMT_LEFT,
                            int iWidth = -1, int iSubItem = -1 ) const;
        int     InsertItem( const LVITEM& pItem ) const;
        int     InsertItem( int iItem, LPCTSTR pszText ) const;
        int     InsertItem( int iItem, LPCTSTR pszText, int iImage ) const;
        BOOL    RedrawItems( int iFirst, int iLast ) const;
        BOOL    Scroll( CSize sz ) const;
        BOOL    SortItems( PFNLVCOMPARE pfnCompare, DWORD_PTR dwData ) const;
        BOOL    Update( int iItem ) const;

    private:
        CListView(const CListView&);              // Disable copy construction
        CListView& operator = (const CListView&); // Disable assignment operator

    };

}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

namespace Win32xx
{

    // Sets the window class.
    inline void CListView::PreRegisterClass(WNDCLASS& wc)
    {
        wc.lpszClassName =  WC_LISTVIEW;
    }


    // Calculates the approximate width and height required to display a given number of items.
    inline CSize CListView::ApproximateViewRect(CSize sz /*= CSize(-1, -1)*/, int iCount /* = -1*/) const
    {
        assert(IsWindow());
        return CSize( ListView_ApproximateViewRect( *this, sz.cx, sz.cy, iCount ) );
    }


    // Retrieves the background color of the list-view control.
    inline COLORREF CListView::GetBkColor( ) const
    {
        assert(IsWindow());
        return ListView_GetBkColor( *this );
    }


    // Retrieves the background image in the list-view control.
    inline BOOL CListView::GetBkImage( LVBKIMAGE& lvbkImage ) const
    {
        assert(IsWindow());
        return ListView_GetBkImage( *this, &lvbkImage );
    }


    // Retrieves the callback mask for the list-view control.
    inline UINT CListView::GetCallbackMask( ) const
    {
        assert(IsWindow());
        return ListView_GetCallbackMask( *this );
    }


    // Determines if an item in the list-view control is selected.
    inline BOOL CListView::GetCheckState( UINT nItem ) const
    {
        assert(IsWindow());
        return ListView_GetCheckState( *this, nItem );
    }


    // Retrieves the attributes of the list-view control's column.
    inline BOOL CListView::GetColumn( int iCol, LVCOLUMN& Column ) const
    {
        assert(IsWindow());
        return ListView_GetColumn( *this, iCol, &Column );
    }


    // Retrieves the current left-to-right order of columns in the list-view control.
    inline BOOL CListView::GetColumnOrderArray( LPINT piArray, int iCount /*= -1*/ ) const
    {
        assert(IsWindow());
        return ListView_GetColumnOrderArray( *this, iCount, piArray );
    }


    // Retrieves the width of a column in report or list view.
    inline int CListView::GetColumnWidth( int iCol ) const
    {
        assert(IsWindow());
        return ListView_GetColumnWidth( *this, iCol );
    }


    // Calculates the number of items that can fit vertically in the visible area of the
    // list-view control when in list or report view. Only fully visible items are counted.
    inline int CListView::GetCountPerPage( ) const
    {
        assert(IsWindow());
        return ListView_GetCountPerPage( *this );
    }


    // Retrieves the handle to the edit control being used to edit the list-view item's text.
    inline HWND CListView::GetEditControl( ) const
    {
        assert(IsWindow());
        return ListView_GetEditControl(*this);
    }


    // Retrieves the extended styles that are currently in use for the list-view control.
    inline DWORD CListView::GetExtendedStyle( ) const
    {
        assert(IsWindow());
        return ListView_GetExtendedListViewStyle( *this );
    }


    // Retrieves the handle to the header control used by the list-view control.
    inline HWND CListView::GetHeader( ) const
    {
        assert(IsWindow());
        return ListView_GetHeader(*this);
    }


    // Retrieves the HCURSOR used when the pointer is over an item while hot tracking is enabled.
    inline HCURSOR CListView::GetHotCursor( )
    {
        assert(IsWindow());
        return ListView_GetHotCursor( *this );
    }


    // Retrieves the index of the hot item.
    inline int CListView::GetHotItem( ) const
    {
        assert(IsWindow());
        return ListView_GetHotItem( *this );
    }


    // Retrieves the amount of time that the mouse cursor must hover over an item before it is selected.
    inline DWORD CListView::GetHoverTime( ) const
    {
        assert(IsWindow());
        return ListView_GetHoverTime( *this );
    }


    // Retrieves the handle to an image list used for drawing list-view items.
    inline CImageList CListView::GetImageList( int nImageType ) const
    {
        assert(IsWindow());
        HIMAGELIST himl = ListView_GetImageList( *this, nImageType );
        return CImageList(himl);
    }


    // Retrieves some or all of the list-view item's attributes.
    inline BOOL CListView::GetItem( LVITEM& Item ) const
    {
        assert(IsWindow());
        return ListView_GetItem( *this, &Item );
    }


    // Retrieves the number of items in the list-view control.
    inline int CListView::GetItemCount( ) const
    {
        assert(IsWindow());
        return ListView_GetItemCount( *this );
    }


    // Retrieves the value(lParam) specific to the item.
    inline DWORD_PTR CListView::GetItemData( int iItem ) const
    {
        assert(IsWindow());

        LVITEM lvi;
        ZeroMemory(&lvi, sizeof(lvi));
        lvi.iItem = iItem;
        lvi.mask = LVIF_PARAM;
        ListView_GetItem(*this, &lvi);
        return lvi.lParam;
    }


    // Retrieves the position of the list-view item.
    inline BOOL CListView::GetItemPosition( int iItem, CPoint& pt ) const
    {
        assert(IsWindow());
        return ListView_GetItemPosition( *this, iItem, &pt );
    }


    // Retrieves the bounding rectangle for all or part of an item in the current view.
    inline BOOL CListView::GetItemRect( int iItem, RECT& rc, UINT nCode ) const
    {
        assert(IsWindow());
        return ListView_GetItemRect( *this, iItem, &rc, nCode );
    }


    // Retrieves the state of the list-view item.
    // Possible values of nMask:
    // LVIS_CUT             The item is marked for a cut-and-paste operation.
    // LVIS_DROPHILITED     The item is highlighted as a drag-and-drop target.
    // LVIS_FOCUSED         The item has the focus, so it is surrounded by a standard focus rectangle.
    // LVIS_SELECTED        The item is selected.
    // LVIS_OVERLAYMASK     Use this mask to retrieve the item's overlay image index.
    // LVIS_STATEIMAGEMASK  Use this mask to retrieve the item's state image index.
    inline UINT CListView::GetItemState( int iItem, UINT nMask ) const
    {
        assert(IsWindow());
        return  ListView_GetItemState( *this, iItem, nMask );
    }


    // Retrieves the text of the list-view item.
    // Note: Although the list-view control allows any length string to be stored
    //       as item text, only the first 260 characters are displayed.
    inline CString CListView::GetItemText( int iItem, int iSubItem, UINT nTextMax /* = 260 */ ) const
    {
        assert(IsWindow());

        CString str;
        if (nTextMax > 0)
        {
            LVITEM lvi;
            ZeroMemory(&lvi, sizeof(lvi));
            lvi.iItem = iItem;
            lvi.iSubItem = iSubItem;
            lvi.mask = LVIF_TEXT;
            lvi.cchTextMax = nTextMax;
            lvi.pszText = str.GetBuffer(nTextMax);
            ListView_GetItem( *this, &lvi );
            str.ReleaseBuffer();
        }
        return str;
    }


    // Searches for a list-view item that has the specified properties and
    // bears the specified relationship to a specified item.
    inline int CListView::GetNextItem( int iItem, int iFlags ) const
    {
        assert(IsWindow());
        return ListView_GetNextItem( *this, iItem, iFlags );
    }


    // Retrieves the working areas from the list-view control.
    inline UINT CListView::GetNumberOfWorkAreas( ) const
    {
        assert(IsWindow());
        UINT nWorkAreas = 0;
        ListView_GetNumberOfWorkAreas( *this, &nWorkAreas );
        return nWorkAreas;
    }


    // Retrieves the current view origin for the list-view control.
    inline BOOL CListView::GetOrigin( CPoint& pt ) const
    {
        assert(IsWindow());
        return ListView_GetOrigin( *this, &pt );
    }


    // Determines the number of selected items in the list-view control.
    inline UINT CListView::GetSelectedCount( ) const
    {
        assert(IsWindow());
        return static_cast<UINT>(SendMessage( LVM_GETSELECTEDCOUNT, 0L, 0L ));
    }


    // Retrieves the selection mark from the list-view control.
    inline int CListView::GetSelectionMark( ) const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage( LVM_GETSELECTIONMARK, 0L, 0L ));
    }


    // Determines the width of a specified string using the list-view control's current font.
    inline int CListView::GetStringWidth( LPCTSTR pszString ) const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage( LVM_GETSTRINGWIDTH, 0L, reinterpret_cast<LPARAM>(pszString)));
    }


    // Retrieves information about the rectangle that surrounds a subitem in the list-view control.
    inline BOOL CListView::GetSubItemRect( int iItem, int iSubItem, int iCode, RECT& rc ) const
    {
        assert(IsWindow());
        return ListView_GetSubItemRect( *this, iItem, iSubItem, iCode, &rc );
    }


    // Retrieves the text background color of the list-view control.
    inline COLORREF CListView::GetTextBkColor( ) const
    {
        assert(IsWindow());
        return ListView_GetTextBkColor( *this );
    }


    // Retrieves the text color of the list-view control.
    inline COLORREF CListView::GetTextColor( ) const
    {
        assert(IsWindow());
        return ListView_GetTextColor( *this );
    }


    // Retrieves the ToolTip control that the list-view control uses to display ToolTips.
    inline HWND CListView::GetToolTips( ) const
    {
        assert(IsWindow());
        return ListView_GetToolTips(*this);
    }


    // Retrieves the index of the topmost visible item when in list or report view.
    inline int CListView::GetTopIndex( ) const
    {
        assert(IsWindow());
        return ListView_GetTopIndex( *this );
    }


    // Retrieves the bounding rectangle of all items in the list-view control.
    inline BOOL CListView::GetViewRect( RECT& rc ) const
    {
        assert(IsWindow());
        return ListView_GetViewRect( *this, &rc );
    }


    // Retrieves the working areas from the list-view control.
    inline void CListView::GetWorkAreas( int iWorkAreas, LPRECT pRectArray ) const
    {
        assert(IsWindow());
        ListView_GetWorkAreas( *this, iWorkAreas, pRectArray );
    }


    // Sets the background color of the list-view control.
    inline BOOL CListView::SetBkColor( COLORREF clrBk ) const
    {
        assert(IsWindow());
        return ListView_SetBkColor( *this, clrBk );
    }


    // Sets the background image in the list-view control.
    inline BOOL CListView::SetBkImage( LVBKIMAGE& lvbkImage ) const
    {
        assert(IsWindow());
        return ListView_SetBkImage( *this, &lvbkImage );
    }


    // Changes the callback mask for the list-view control.
    inline BOOL CListView::SetCallbackMask( UINT nMask ) const
    {
        assert(IsWindow());
        return ListView_SetCallbackMask( *this, nMask );
    }


    // Used to select or deselect an item in the list-view control.
    // This should only be used for list-view controls with the LVS_EX_CHECKBOXES style.
    inline void CListView::SetCheckState( int iItem, BOOL Checked /*= TRUE*/ ) const
    {
        assert(IsWindow());
        ListView_SetItemState(*this, iItem, INDEXTOSTATEIMAGEMASK((Checked!=FALSE)?2:1),LVIS_STATEIMAGEMASK);
    }


    // Sets the attributes of the list-view column.
    inline BOOL CListView::SetColumn( int iCol, const LVCOLUMN& Column ) const
    {
        assert(IsWindow());
        return ListView_SetColumn( *this, iCol, &Column );
    }


    // Sets the left-to-right order of columns in the list-view control.
    inline BOOL CListView::SetColumnOrderArray( int iCount, LPINT piArray ) const
    {
        assert(IsWindow());
        return ListView_SetColumnOrderArray( *this, iCount, piArray );
    }


    // Used to change the width of a column in report view or the width of all columns in list-view mode.
    inline BOOL CListView::SetColumnWidth( int iCol, int cx ) const
    {
        assert(IsWindow());
        return ListView_SetColumnWidth( *this, iCol, cx );
    }


    // Sets extended styles for the list-view control.
    inline DWORD CListView::SetExtendedStyle( DWORD dwNewStyle ) const
    {
        assert(IsWindow());
        return ListView_SetExtendedListViewStyle( *this, dwNewStyle );
    }


    // Sets the HCURSOR that the list-view control uses when the pointer is
    // over an item while hot tracking is enabled.
    inline HCURSOR CListView::SetHotCursor( HCURSOR hCursor ) const
    {
        assert(IsWindow());
        return ListView_SetHotCursor( *this, hCursor );
    }


    // Sets the hot item in the list-view control.
    inline int CListView::SetHotItem( int nIndex ) const
    {
        assert(IsWindow());
        return ListView_SetHotItem( *this, nIndex );
    }


    // Sets the amount of time that the mouse cursor must hover over an item before it is selected.
    inline DWORD CListView::SetHoverTime( DWORD dwHoverTime /*= -1*/ ) const
    {
        assert(IsWindow());
        return ListView_SetHoverTime( *this, dwHoverTime );
    }


    // Sets the spacing between icons in list-view controls set to the LVS_ICON style.
    inline CSize CListView::SetIconSpacing( int cx, int cy ) const
    {
        assert(IsWindow());
        return CSize( ListView_SetIconSpacing( *this, cx, cy ) );
    }


    // Sets the spacing between icons in the list-view control set to the LVS_ICON style.
    inline CSize CListView::SetIconSpacing( CSize sz ) const
    {
        assert(IsWindow());
        return CSize( ListView_SetIconSpacing( *this, sz.cx, sz.cy ) );
    }


    // Assigns an image list to the list-view control.
    // Valid iImageListType values: LVSIL_NORMAL, LVSIL_SMALL, LVSIL_STATE.
    inline CImageList CListView::SetImageList( HIMAGELIST himlNew, int iImageListType ) const
    {
        assert(IsWindow());
        HIMAGELIST himl = ListView_SetImageList( *this, himlNew, iImageListType );
        return CImageList(himl);
    }


    // Sets some or all of a list-view item's attributes.
    // The declaration for TVITEM:
    //  typedef struct _LVITEM {
    //      UINT mask;
    //      int iItem;
    //      int iSubItem;
    //      UINT state;
    //      UINT stateMask;
    //      LPTSTR pszText;
    //      int cchTextMax;
    //      int iImage;
    //      LPARAM lParam;
    // } LVITEM, *LVITEM&;
    inline BOOL CListView::SetItem( LVITEM& Item ) const
    {
        assert(IsWindow());
        return ListView_SetItem( *this, &Item );
    }


    // Sets some or all of a list-view item's attributes.
    inline BOOL CListView::SetItem( int iItem, int iSubItem, UINT nMask, LPCTSTR pszText, int iImage,
                    UINT nState, UINT nStateMask, LPARAM lParam, int iIndent ) const
    {
        assert(IsWindow());

        LVITEM lvi;
        ZeroMemory(&lvi, sizeof(lvi));
        lvi.iItem = iItem;
        lvi.iSubItem = iSubItem;
        lvi.mask = nMask;
        lvi.pszText = const_cast<LPTSTR>(pszText);
        lvi.iImage = iImage;
        lvi.state = nState;
        lvi.stateMask = nStateMask;
        lvi.lParam = lParam;
        lvi.iIndent = iIndent;

        return ListView_SetItem( *this, &lvi);
    }


    // Causes the list-view control to allocate memory for the specified number of items.
    inline void CListView::SetItemCount( int iCount ) const
    {
        assert(IsWindow());
        ListView_SetItemCount( *this, iCount );
    }


    // Sets the virtual number of items in a virtual list view.
    inline void CListView::SetItemCountEx( int iCount, DWORD dwFlags /*= LVSICF_NOINVALIDATEALL*/ ) const
    {
        assert(IsWindow());
        ListView_SetItemCountEx( *this, iCount, dwFlags );
    }


    // Sets the value(lParam) specific to the item.
    inline BOOL CListView::SetItemData( int iItem, DWORD_PTR dwData ) const
    {
        assert(IsWindow());

        LVITEM lvi;
        ZeroMemory(&lvi, sizeof(lvi));
        lvi.iItem = iItem;
        lvi.lParam = dwData;
        lvi.mask = LVIF_PARAM;
        return ListView_SetItem(*this, &lvi);
    }


    // Moves an item to a specified position in the list-view control (in icon or small icon view).
    inline BOOL CListView::SetItemPosition( int iItem, CPoint& pt ) const
    {
        assert(IsWindow());
        return ListView_SetItemPosition( *this, iItem, pt.x, pt.y );
    }


    // Changes the state of an item in the list-view control.
    // Possible values of nMask:
    // LVIS_CUT             The item is marked for a cut-and-paste operation.
    // LVIS_DROPHILITED     The item is highlighted as a drag-and-drop target.
    // LVIS_FOCUSED         The item has the focus, so it is surrounded by a standard focus rectangle.
    // LVIS_SELECTED        The item is selected.
    // LVIS_OVERLAYMASK     Use this mask to retrieve the item's overlay image index.
    // LVIS_STATEIMAGEMASK  Use this mask to retrieve the item's state image index.
    inline BOOL CListView::SetItemState( int iItem, LVITEM& Item ) const
    {
        assert(IsWindow());
        return static_cast<BOOL>(SendMessage(LVM_SETITEMSTATE, iItem, reinterpret_cast<LPARAM>(&Item)));
    }


    // Changes the state of an item in the list-view control.
    inline void CListView::SetItemState( int iItem, UINT nState, UINT nMask ) const
    {
        assert(IsWindow());
        ListView_SetItemState(*this, iItem, nState, nMask);
    }


    // Sets the text color of the list-view control.
    inline void CListView::SetItemText( int iItem, int iSubItem, LPCTSTR pszText ) const
    {
        assert(IsWindow());
        ListView_SetItemText(*this, iItem, iSubItem, const_cast<LPTSTR>(pszText) );
    }


    // Sets the selection mark in the list-view control.
    inline int CListView::SetSelectionMark( int iIndex ) const
    {
        assert(IsWindow());
        return ListView_SetSelectionMark( *this, iIndex );
    }


    // Sets the background color of text in the list-view control.
    inline BOOL CListView::SetTextBkColor( COLORREF clrBkText ) const
    {
        assert(IsWindow());
        return ListView_SetTextBkColor( *this, clrBkText );
    }


    // Sets the text color of the list-view control.
    inline BOOL CListView::SetTextColor( COLORREF clrText ) const
    {
        assert(IsWindow());
        return ListView_SetTextColor( *this, clrText );
    }


    // Sets the ToolTip control that the list-view control will use to display ToolTips.
    inline HWND CListView::SetToolTips( HWND hToolTip ) const
    {
        assert(IsWindow());
        return reinterpret_cast<HWND>(SendMessage(LVM_SETTOOLTIPS, reinterpret_cast<WPARAM>(hToolTip), 0L));
    }


    // Sets the working area within the list-view control.
    inline void CListView::SetWorkAreas( int nWorkAreas, LPCRECT pRectArray ) const
    {
        assert(IsWindow());
        ListView_SetWorkAreas( *this, nWorkAreas, pRectArray );
    }


    // Determines which list-view item or subitem is located at a given position.
    inline int CListView::SubItemHitTest( LVHITTESTINFO& htInfo ) const
    {
        assert(IsWindow());
        return ListView_SubItemHitTest( *this, &htInfo );
    }


    /////////////
    // Operations

    // Arranges items in icon view.
    inline BOOL CListView::Arrange( UINT nCode ) const
    {
        assert(IsWindow());
        return ListView_Arrange( *this, nCode );
    }


    // Creates a drag image list for the specified item.
    inline CImageList CListView::CreateDragImage( int iItem, CPoint& pt ) const
    {
        assert(IsWindow());
        HIMAGELIST himl = ListView_CreateDragImage( *this, iItem, &pt );
        return CImageList(himl);
    }


    // Removes all items from the list-view control.
    inline BOOL CListView::DeleteAllItems( ) const
    {
        assert(IsWindow());
        return ListView_DeleteAllItems( *this );
    }


    // Removes a column from the list-view control.
    inline BOOL CListView::DeleteColumn( int iCol ) const
    {
        assert(IsWindow());
        return ListView_DeleteColumn( *this, iCol );
    }


    // Removes an item from the list-view control.
    inline BOOL CListView::DeleteItem( int iItem ) const
    {
        assert(IsWindow());
        return ListView_DeleteItem( *this, iItem );
    }


    // Begins in-place editing of the list-view item's text.
    inline HWND CListView::EditLabel( int iItem ) const
    {
        assert(IsWindow());
        return ListView_EditLabel( *this, iItem );
    }


    // Ensures that a list-view item is either entirely or partially visible,
    // scrolling the list-view control if necessary.
    inline BOOL CListView::EnsureVisible( int iItem, BOOL IsPartialOK ) const
    {
        assert(IsWindow());
        return static_cast<BOOL>(SendMessage(LVM_ENSUREVISIBLE, iItem, IsPartialOK ));
    }


    // Searches for a list-view item with the specified characteristics.
    inline int CListView::FindItem( LVFINDINFO& FindInfo, int iStart /*= -1*/ ) const
    {
        assert(IsWindow());
        return ListView_FindItem( *this, iStart, &FindInfo );
    }


    // Determines which list-view item, if any, is at a specified position.
    inline int CListView::HitTest( LVHITTESTINFO& HitTestInfo ) const
    {
        assert(IsWindow());
        return ListView_HitTest( *this, &HitTestInfo );
    }


    // Determines which list-view item, if any, is at a specified position.
    inline int CListView::HitTest( CPoint pt, UINT* pFlags /*= NULL*/ ) const
    {
        assert(IsWindow());

        LVHITTESTINFO hti;
        ZeroMemory(&hti, sizeof(hti));
        hti.flags = *pFlags;
        hti.pt = pt;
        return ListView_HitTest( *this, &hti );
    }


    // Inserts a new column in the list-view control.
    inline int CListView::InsertColumn( int iCol, const LVCOLUMN& Column ) const
    {
        assert(IsWindow());
        return ListView_InsertColumn( *this, iCol, &Column );
    }


    // Inserts a new column in the list-view control.
    inline int CListView::InsertColumn( int iCol, LPCTSTR pszColumnHeading, int iFormat /*= LVCFMT_LEFT*/,
                        int iWidth /*= -1*/, int iSubItem /*= -1*/ ) const
    {
        assert(IsWindow());

        LVCOLUMN lvc;
        ZeroMemory(&lvc, sizeof(lvc));
        lvc.mask = LVCF_TEXT|LVCF_ORDER|LVCF_FMT;
        if (-1 != iWidth)
        {
            lvc.mask |= LVCF_WIDTH;
            lvc.cx = iWidth;
        }
        if (-1 !=  iSubItem)
        {
            lvc.mask |= LVCF_SUBITEM;
            lvc.iSubItem = iSubItem;
        }

        lvc.iOrder = iCol;
        lvc.pszText = const_cast<LPTSTR>(pszColumnHeading);
        lvc.fmt = iFormat;
        lvc.iSubItem = iSubItem;
        return ListView_InsertColumn( *this, iCol, &lvc );
    }


    // Inserts a new item in the list-view control.
    inline int CListView::InsertItem( const LVITEM& Item ) const
    {
        assert(IsWindow());
        return ListView_InsertItem( *this, &Item );
    }


    // Inserts a new item in the list-view control.
    inline int CListView::InsertItem( int iItem, LPCTSTR pszText ) const
    {
        assert(IsWindow());

        LVITEM lvi;
        ZeroMemory(&lvi, sizeof(lvi));
        lvi.iItem = iItem;
        lvi.pszText = const_cast<LPTSTR>(pszText);
        lvi.mask = LVIF_TEXT;
        return ListView_InsertItem( *this, &lvi );
    }


    // Inserts a new item in the list-view control.
    inline int CListView::InsertItem( int iItem, LPCTSTR pszText, int iImage ) const
    {
        assert(IsWindow());

        LVITEM lvi;
        ZeroMemory(&lvi, sizeof(lvi));
        lvi.iItem = iItem;
        lvi.pszText = const_cast<LPTSTR>(pszText);
        lvi.iImage = iImage;
        lvi.mask = LVIF_TEXT | LVIF_IMAGE;
        return ListView_InsertItem( *this, &lvi );
    }


    // Forces the list-view control to redraw a range of items.
    inline BOOL CListView::RedrawItems( int iFirst, int iLast ) const
    {
        assert(IsWindow());
        return ListView_RedrawItems( *this, iFirst, iLast );
    }


    // Scrolls the content of the list-view control.
    inline BOOL CListView::Scroll( CSize sz ) const
    {
        assert(IsWindow());
        return ListView_Scroll( *this, sz.cx, sz.cy );
    }


    // Uses an application-defined comparison function to sort the items of the list-view control.
    inline BOOL CListView::SortItems( PFNLVCOMPARE pfnCompare, DWORD_PTR dwData ) const
    {
        assert(IsWindow());
        return ListView_SortItems( *this, pfnCompare, dwData );
    }


    // Updates a list-view item. If the list-view control has the LVS_AUTOARRANGE style,
    // the list-view control is rearranged.
    inline BOOL CListView::Update( int iItem ) const
    {
        assert(IsWindow());
        return ListView_Update( *this, iItem );
    }

} // namespace Win32xx

#endif // #ifndef _WIN32XX_LISTVIEW_H_

