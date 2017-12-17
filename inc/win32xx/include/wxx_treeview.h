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




#ifndef _WIN32XX_TREEVIEW_H_
#define _WIN32XX_TREEVIEW_H_

#include "wxx_wincore.h"
#include "wxx_controls.h"
#include <commctrl.h>


// Disable macros from Windowsx.h
#undef GetNextSibling
#undef GetPrevSibling

namespace Win32xx
{

    // The CTreeView class provides the functionality of a tree view control.
    class CTreeView : public CWnd
    {
    public:
        CTreeView() {}
        virtual ~CTreeView() {}
        virtual void PreRegisterClass(WNDCLASS& wc);

// Attributes
        COLORREF GetBkColor() const;
        HTREEITEM GetChild(HTREEITEM hItem) const;
        UINT    GetCount() const;
        HTREEITEM GetDropHiLightItem() const;
        HWND    GetEditControl() const;
        HTREEITEM GetFirstVisible() const;
        CImageList GetImageList(int iImageType) const;
        UINT    GetIndent() const;
        COLORREF GetInsertMarkColor() const;
        BOOL    GetItem(TVITEM& Item) const;
        DWORD_PTR GetItemData(HTREEITEM hItem) const;
        int     GetItemHeight() const;
        BOOL    GetItemImage(HTREEITEM hItem, int& nImage, int& nSelectedImage ) const;
        BOOL    GetItemRect(HTREEITEM hItem, RECT& rc, BOOL IsTextOnly) const;
        CString GetItemText(HTREEITEM hItem, UINT nTextMax = 260) const;
        HTREEITEM GetLastVisible() const;
        HTREEITEM GetNextItem(HTREEITEM hItem, UINT nCode) const;
        HTREEITEM GetNextSibling(HTREEITEM hItem) const;
        HTREEITEM GetNextVisible(HTREEITEM hItem) const;
        HTREEITEM GetParentItem(HTREEITEM hItem) const;
        HTREEITEM GetPrevSibling(HTREEITEM hItem) const;
        HTREEITEM GetPrevVisible(HTREEITEM hItem) const;
        HTREEITEM GetRootItem() const;
        int     GetScrollTime() const;
        HTREEITEM GetSelection() const;
        COLORREF GetTextColor() const;
        HWND    GetToolTips() const;
        UINT    GetVisibleCount() const;
        BOOL    ItemHasChildren(HTREEITEM hItem) const;
        COLORREF SetBkColor(COLORREF clrBk) const;
        CImageList SetImageList(HIMAGELIST himlNew, int nType = TVSIL_NORMAL) const;
        void    SetIndent(int indent) const;
        BOOL    SetInsertMark(HTREEITEM hItem, BOOL After = TRUE) const;
        COLORREF SetInsertMarkColor(COLORREF clrInsertMark) const;
        BOOL    SetItem(TVITEM& Item) const;
        BOOL    SetItem(HTREEITEM hItem, UINT nMask, LPCTSTR szText, int nImage, int nSelectedImage, UINT nState, UINT nStateMask, LPARAM lParam) const;
        BOOL    SetItemData(HTREEITEM hItem, DWORD_PTR dwData) const;
        int     SetItemHeight(SHORT cyItem) const;
        BOOL    SetItemImage(HTREEITEM hItem, int nImage, int nSelectedImage) const;
        BOOL    SetItemText(HTREEITEM hItem, LPCTSTR szText) const;
        UINT    SetScrollTime(UINT uScrollTime) const;
        COLORREF SetTextColor(COLORREF clrText) const;
        HWND    SetToolTips(HWND hToolTip) const;

// Operations
        CImageList CreateDragImage(HTREEITEM hItem) const;
        BOOL    DeleteAllItems() const;
        BOOL    DeleteItem(HTREEITEM hItem) const;
        HWND    EditLabel(HTREEITEM hItem) const;
        BOOL    EndEditLabelNow(BOOL fCancel) const;
        BOOL    EnsureVisible(HTREEITEM hItem) const;
        BOOL    Expand(HTREEITEM hItem, UINT nCode) const;
        HTREEITEM HitTest(TVHITTESTINFO& ht) const;
        HTREEITEM InsertItem(TVINSERTSTRUCT& tvIS) const;
        BOOL    Select(HTREEITEM hitem, UINT flag) const;
        BOOL    SelectDropTarget(HTREEITEM hItem) const;
        BOOL    SelectItem(HTREEITEM hItem) const;
        BOOL    SelectSetFirstVisible(HTREEITEM hItem) const;
        BOOL    SortChildren(HTREEITEM hItem, BOOL Recurse) const;
        BOOL    SortChildrenCB(TVSORTCB& sort, BOOL Recurse) const;

    private:
        CTreeView(const CTreeView&);                // Disable copy construction
        CTreeView& operator = (const CTreeView&); // Disable assignment operator

    };

}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

namespace Win32xx
{

    inline void CTreeView::PreRegisterClass(WNDCLASS& wc)
    {
        // Set the Window Class
        wc.lpszClassName =  WC_TREEVIEW;
    }

// Attributes

    // Retrieves the current background color of the control.
    inline COLORREF CTreeView::GetBkColor() const
    {
        assert(IsWindow());
        return TreeView_GetBkColor( *this );
    }


    // Retrieves the first child item of the specified tree-view item.
    inline HTREEITEM CTreeView::GetChild(HTREEITEM hItem) const
    {
        assert(IsWindow());
        return TreeView_GetChild(*this, hItem);
    }


    // Retrieves a count of the items in a tree-view control.
    inline UINT CTreeView::GetCount() const
    {
        assert(IsWindow());
        return TreeView_GetCount( *this );
    }


    // Retrieves the tree-view item that is the target of a drag-and-drop operation.
    inline HTREEITEM CTreeView::GetDropHiLightItem() const
    {
        assert(IsWindow());
        return TreeView_GetDropHilight(*this);
    }


    // Retrieves the handle to the edit control being used to edit a tree-view item's text.
    inline HWND CTreeView::GetEditControl() const
    {
        assert(IsWindow());
        return TreeView_GetEditControl(*this);
    }


    // Retrieves the first visible item in a tree-view control window.
    inline HTREEITEM CTreeView::GetFirstVisible() const
    {
        assert(IsWindow());
        return TreeView_GetFirstVisible(*this);
    }


    // Retrieves the handle to the normal or state image list associated with a tree-view control.
    inline CImageList CTreeView::GetImageList(int iImageType) const
    {
        assert(IsWindow());
        HIMAGELIST himl = TreeView_GetImageList( *this, iImageType );
        return CImageList(himl);
    }


    // Retrieves the amount, in pixels, that child items are indented relative to their parent items.
    inline UINT CTreeView::GetIndent() const
    {
        assert(IsWindow());
        return TreeView_GetIndent( *this );
    }


    // Retrieves the color used to draw the insertion mark for the tree view.
    inline COLORREF CTreeView::GetInsertMarkColor() const
    {
        assert(IsWindow());
        return TreeView_GetInsertMarkColor( *this );
    }


    // Retrieves some or all of a tree-view item's attributes.
    inline BOOL CTreeView::GetItem(TVITEM& Item) const
    {
        assert(IsWindow());
        return TreeView_GetItem( *this, &Item );
    }


    // Retrieves a tree-view item's application data.
    inline DWORD_PTR CTreeView::GetItemData(HTREEITEM hItem) const
    {
        assert(IsWindow());

        TVITEM tvi;
        ZeroMemory(&tvi, sizeof(tvi));
        tvi.mask = TVIF_PARAM;
        tvi.hItem = hItem;
        TreeView_GetItem( *this, &tvi );
        return tvi.lParam;
    }


    // Retrieves the current height of the tree-view item.
    inline int  CTreeView::GetItemHeight() const
    {
        assert(IsWindow());
        return TreeView_GetItemHeight( *this );
    }


    // Retrieves the index of the tree-view item's image and selected image.
    inline BOOL CTreeView::GetItemImage(HTREEITEM hItem, int& nImage, int& nSelectedImage ) const
    {
        assert(IsWindow());

        TVITEM tvi;
        ZeroMemory(&tvi, sizeof(tvi));
        tvi.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE;
        tvi.hItem = hItem;
        BOOL Succeeded = TreeView_GetItem( *this, &tvi );
        nImage = tvi.iImage;
        nSelectedImage = tvi.iSelectedImage;
        return Succeeded;
    }


    // Retrieves the bounding rectangle for a tree-view item and indicates whether the item is visible.
    inline BOOL CTreeView::GetItemRect(HTREEITEM hItem, RECT& rc, BOOL IsTextOnly) const
    {
        assert(IsWindow());
        return TreeView_GetItemRect( *this, hItem, &rc, IsTextOnly );
    }


    // Retrieves the text for a tree-view item.
    // Note: Although the tree-view control allows any length string to be stored
    //       as item text, only the first 260 characters are displayed.
    inline CString CTreeView::GetItemText(HTREEITEM hItem, UINT nTextMax /* = 260 */) const
    {
        assert(IsWindow());

        CString str;
        if (nTextMax > 0)
        {
            TVITEM tvi;
            ZeroMemory(&tvi, sizeof(tvi));
            tvi.hItem = hItem;
            tvi.mask = TVIF_TEXT;
            tvi.cchTextMax = nTextMax;
            tvi.pszText = str.GetBuffer(nTextMax);
            SendMessage(TVM_GETITEM, 0L, reinterpret_cast<LPARAM>(&tvi));
            str.ReleaseBuffer();
        }
        return str;
    }


    // Retrieves the last expanded item in a tree-view control.
    // This does not retrieve the last item visible in the tree-view window.
    inline HTREEITEM CTreeView::GetLastVisible() const
    {
        assert(IsWindow());
        return TreeView_GetLastVisible(*this);
    }


    // Retrieves the tree-view item that bears the specified relationship to a specified item.
    inline HTREEITEM CTreeView::GetNextItem(HTREEITEM hItem, UINT nCode) const
    {
        assert(IsWindow());
        return TreeView_GetNextItem( *this, hItem, nCode);
    }


    // Retrieves the next sibling item of a specified item in a tree-view control.
    inline HTREEITEM CTreeView::GetNextSibling(HTREEITEM hItem) const
    {
        assert(IsWindow());
        return TreeView_GetNextSibling(*this, hItem);
    }


    // Retrieves the next visible item that follows a specified item in a tree-view control.
    inline HTREEITEM CTreeView::GetNextVisible(HTREEITEM hItem) const
    {
        assert(IsWindow());
        return TreeView_GetNextVisible(*this, hItem);
    }


    // Retrieves the parent item of the specified tree-view item.
    inline HTREEITEM CTreeView::GetParentItem(HTREEITEM hItem) const
    {
        assert(IsWindow());
        return TreeView_GetParent(*this, hItem);
    }


    // Retrieves the previous sibling item of a specified item in a tree-view control.
    inline HTREEITEM CTreeView::GetPrevSibling(HTREEITEM hItem) const
    {
        assert(IsWindow());
        return TreeView_GetPrevSibling(*this, hItem);
    }


    // Retrieves the first visible item that precedes a specified item in a tree-view control.
    inline HTREEITEM CTreeView::GetPrevVisible(HTREEITEM hItem) const
    {
        assert(IsWindow());
        return TreeView_GetPrevVisible(*this, hItem);
    }


    // Retrieves the topmost or very first item of the tree-view control.
    inline HTREEITEM CTreeView::GetRootItem() const
    {
        assert(IsWindow());
        return TreeView_GetRoot(*this);
    }


    // Retrieves the maximum scroll time for the tree-view control.
    inline int CTreeView::GetScrollTime() const
    {
        assert(IsWindow());
        return TreeView_GetScrollTime( *this );
    }


    // Retrieves the currently selected item in a tree-view control.
    inline HTREEITEM CTreeView::GetSelection() const
    {
        assert(IsWindow());
        return TreeView_GetSelection(*this);
    }


    // Retrieves the current text color of the control.
    inline COLORREF CTreeView::GetTextColor() const

    {
        assert(IsWindow());
        return TreeView_GetTextColor( *this );
    }


    // Retrieves the handle to the child ToolTip control used by a tree-view control.
    inline HWND CTreeView::GetToolTips() const
    {
        assert(IsWindow());
        return TreeView_GetToolTips(*this);
    }


    // Obtains the number of items that can be fully visible in the client window of a tree-view control.
    inline UINT CTreeView::GetVisibleCount() const
    {
        assert(IsWindow());
        return TreeView_GetVisibleCount( *this );
    }


    // Returns true of the tree-view item has one or more children.
    inline BOOL CTreeView::ItemHasChildren(HTREEITEM hItem) const
    {
        assert(IsWindow());

        if (TreeView_GetChild( *this, hItem ))
            return TRUE;

        return FALSE;
    }


    // Sets the background color of the control.
    inline COLORREF CTreeView::SetBkColor(COLORREF clrBk) const
    {
        assert(IsWindow());
        return TreeView_SetBkColor( *this, clrBk );
    }


    // Sets the normal or state image list for a tree-view control
    // and redraws the control using the new images.
    inline CImageList CTreeView::SetImageList(HIMAGELIST himlNew, int nType /*= TVSIL_NORMAL*/) const
    {
        assert(IsWindow());
        HIMAGELIST himl = TreeView_SetImageList( *this, himlNew, nType );
        return CImageList(himl);
    }


    // Sets the width of indentation for a tree-view control
    // and redraws the control to reflect the new width.
    inline void CTreeView::SetIndent(int indent) const
    {
        assert(IsWindow());
        TreeView_SetIndent( *this, indent );
    }


    // Sets the insertion mark in a tree-view control.
    inline BOOL CTreeView::SetInsertMark(HTREEITEM hItem, BOOL After/* = TRUE*/) const
    {
        assert(IsWindow());
        return TreeView_SetInsertMark( *this, hItem, After );
    }


    // Sets the color used to draw the insertion mark for the tree view.
    inline COLORREF CTreeView::SetInsertMarkColor(COLORREF clrInsertMark) const
    {
        assert(IsWindow());
        return TreeView_SetInsertMarkColor( *this, clrInsertMark );
    }


    // Sets some or all of a tree-view item's attributes.
    inline BOOL CTreeView::SetItem(TVITEM& Item) const
    {
        assert(IsWindow());
        return TreeView_SetItem( *this, &Item );
    }


    // Sets some or all of a tree-view item's attributes.
    inline BOOL CTreeView::SetItem(HTREEITEM hItem, UINT nMask, LPCTSTR szText, int nImage, int nSelectedImage, UINT nState, UINT nStateMask, LPARAM lParam) const
    {
        assert(IsWindow());

        TVITEM tvi;
        ZeroMemory(&tvi, sizeof(tvi));
        tvi.hItem = hItem;
        tvi.mask  = nMask;
        tvi.pszText = const_cast<LPTSTR>(szText);
        tvi.iImage  = nImage;
        tvi.iSelectedImage = nSelectedImage;
        tvi.state = nState;
        tvi.stateMask = nStateMask;
        tvi.lParam = lParam;
        return TreeView_SetItem( *this, &tvi );
    }


    // Sets the tree-view item's application data.
    inline BOOL CTreeView::SetItemData(HTREEITEM hItem, DWORD_PTR dwData) const
    {
        assert(IsWindow());

        TVITEM tvi;
        ZeroMemory(&tvi, sizeof(tvi));
        tvi.hItem = hItem;
        tvi.mask = TVIF_PARAM;
        tvi.lParam = dwData;
        return TreeView_SetItem( *this, &tvi );
    }


    // Sets the height of the tree-view items.
    inline int  CTreeView::SetItemHeight(SHORT cyItem) const
    {
        assert(IsWindow());
        return TreeView_SetItemHeight( *this, cyItem );
    }


    // Sets the tree-view item's application image.
    inline BOOL CTreeView::SetItemImage(HTREEITEM hItem, int nImage, int nSelectedImage) const
    {
        assert(IsWindow());

        TVITEM tvi;
        ZeroMemory(&tvi, sizeof(tvi));
        tvi.hItem = hItem;
        tvi.iImage = nImage;
        tvi.iSelectedImage = nSelectedImage;
        tvi.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE;
        return TreeView_SetItem(*this, &tvi );
    }


    // Sets the tree-view item's application text.
    inline BOOL CTreeView::SetItemText(HTREEITEM hItem, LPCTSTR szText) const
    {
        assert(IsWindow());

        TVITEM tvi;
        ZeroMemory(&tvi, sizeof(tvi));
        tvi.hItem = hItem;
        tvi.pszText = const_cast<LPTSTR>(szText);
        tvi.mask = TVIF_TEXT;
        return TreeView_SetItem(*this, &tvi );
    }


    // Sets the maximum scroll time for the tree-view control.
    inline UINT CTreeView::SetScrollTime(UINT uScrollTime) const
    {
        assert(IsWindow());
        return TreeView_SetScrollTime( *this, uScrollTime );
    }


    // Sets the text color of the control.
    inline COLORREF CTreeView::SetTextColor(COLORREF clrText) const
    {
        assert(IsWindow());
        return TreeView_SetTextColor( *this, clrText );
    }


    // Sets a tree-view control's child ToolTip control.
    inline HWND CTreeView::SetToolTips(HWND hToolTip) const
    {
        assert(IsWindow());
        return TreeView_SetToolTips(*this, hToolTip);
    }

    // Operations


    // Creates a dragging bitmap for the specified item in a tree-view control.
    // It also creates an image list for the bitmap and adds the bitmap to the image list.
    // An application can display the image when dragging the item by using the image list functions.
    inline CImageList CTreeView::CreateDragImage(HTREEITEM hItem) const
    {
        assert(IsWindow());
        HIMAGELIST himl = TreeView_CreateDragImage(*this, hItem);
        return CImageList(himl);
    }


    // Deletes all items from a tree-view control.
    inline BOOL CTreeView::DeleteAllItems() const
    {
        assert(IsWindow());
        return TreeView_DeleteAllItems( *this );
    }


    // Removes an item and all its children from a tree-view control.
    inline BOOL CTreeView::DeleteItem(HTREEITEM hItem) const
    {
        assert(IsWindow());
        return TreeView_DeleteItem( *this, hItem );
    }


    // Begins in-place editing of the specified item's text, replacing the text of the item
    // with a single-line edit control containing the text.
    // The specified item  is implicitly selected and focused.
    inline HWND CTreeView::EditLabel(HTREEITEM hItem) const
    {
        assert(IsWindow());
        return TreeView_EditLabel( *this, hItem );
    }


    // Ends the editing of a tree-view item's label.
    inline BOOL CTreeView::EndEditLabelNow(BOOL fCancel) const
    {
        assert(IsWindow());
        return TreeView_EndEditLabelNow(*this, fCancel);
    }


    // Ensures that a tree-view item is visible, expanding the parent item or
    // scrolling the tree-view control, if necessary.
    inline BOOL CTreeView::EnsureVisible(HTREEITEM hItem) const
    {
        assert(IsWindow());
        return TreeView_EnsureVisible( *this, hItem );
    }


    // The TreeView_Expand macro expands or collapses the list of child items associated
    // with the specified parent item, if any.
    inline BOOL CTreeView::Expand(HTREEITEM hItem, UINT nCode) const
    {
        assert(IsWindow());
        return TreeView_Expand( *this, hItem, nCode );
    }


    // Determines the location of the specified point relative to the client area of a tree-view control.
    inline HTREEITEM CTreeView::HitTest(TVHITTESTINFO& ht) const
    {
        assert(IsWindow());
        return TreeView_HitTest( *this, &ht );
    }


    // Inserts a new item in a tree-view control.
    inline HTREEITEM CTreeView::InsertItem(TVINSERTSTRUCT& tvIS) const
    {
        assert(IsWindow());
        return TreeView_InsertItem( *this, &tvIS );
    }


    // Selects the specified tree-view item, scrolls the item into view, or redraws
    // the item in the style used to indicate the target of a drag-and-drop operation.
    inline BOOL CTreeView::Select(HTREEITEM hitem, UINT flag) const
    {
        assert(IsWindow());
        return TreeView_Select(*this, hitem, flag );
    }


    // Redraws a specified tree-view control item in the style used to indicate the
    // target of a drag-and-drop operation.
    inline BOOL CTreeView::SelectDropTarget(HTREEITEM hItem) const
    {
        assert(IsWindow());
        return TreeView_SelectDropTarget(*this, hItem);
    }


    // Selects the specified tree-view item.
    inline BOOL CTreeView::SelectItem(HTREEITEM hItem) const
    {
        assert(IsWindow());
        return TreeView_SelectItem(*this, hItem);
    }


    // Scrolls the tree-view control vertically to ensure that the specified item is visible.
    // If possible, the specified item becomes the first visible item at the top of the control's window.
    inline BOOL CTreeView::SelectSetFirstVisible(HTREEITEM hItem) const
    {
        assert(IsWindow());
        return TreeView_SelectSetFirstVisible(*this, hItem);
    }


    // Sorts the child items of the specified parent item in a tree-view control.
    inline BOOL CTreeView::SortChildren(HTREEITEM hItem, BOOL Recurse) const
    {
        assert(IsWindow());
        return TreeView_SortChildren( *this, hItem, Recurse );
    }


    // Sorts tree-view items using an application-defined callback function that compares the items.
    inline BOOL CTreeView::SortChildrenCB(TVSORTCB& sort, BOOL Recurse) const
    {
        assert(IsWindow());
        return TreeView_SortChildrenCB( *this, &sort, Recurse );
    }


} // namespace Win32xx

#endif // #ifndef _WIN32XX_TREEVIEW_H_

