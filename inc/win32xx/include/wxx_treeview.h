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



#ifndef _WIN32XX_TREEVIEW_H_
#define _WIN32XX_TREEVIEW_H_

#include "wxx_wincore.h"
#include "wxx_controls.h"
#include <Commctrl.h>


// Disable macros from Windowsx.h
#undef GetNextSibling
#undef GetPrevSibling

namespace Win32xx
{
    ///////////////////////////////////////////////////////////////
    // CTreeView manages a tree view control. A tree-view control
    // is a window that displays a hierarchical list of items,
    // such as the headings in a document, the entries in an index,
    // or the files and directories on a disk.
    class CTreeView : public CWnd
    {
    public:
        CTreeView() {}
        virtual ~CTreeView() {}
        virtual void PreRegisterClass(WNDCLASS& wc);

        // Accessors and mutators
        COLORREF GetBkColor() const;
        HTREEITEM GetChild(HTREEITEM item) const;
        UINT    GetCount() const;
        HTREEITEM GetDropHiLightItem() const;
        HWND    GetEditControl() const;
        HTREEITEM GetFirstVisible() const;
        CImageList GetImageList(int imageType) const;
        UINT    GetIndent() const;
        COLORREF GetInsertMarkColor() const;
        BOOL    GetItem(TVITEM& itemInfo) const;
        DWORD_PTR GetItemData(HTREEITEM item) const;
        int     GetItemHeight() const;
        BOOL    GetItemImage(HTREEITEM item, int& image, int& selectedImage ) const;
        BOOL    GetItemRect(HTREEITEM item, RECT& rc, BOOL isTextOnly) const;
        CString GetItemText(HTREEITEM item, UINT textMax = 260) const;
        HTREEITEM GetLastVisible() const;
        HTREEITEM GetNextItem(HTREEITEM item, UINT code) const;
        HTREEITEM GetNextSibling(HTREEITEM item) const;
        HTREEITEM GetNextVisible(HTREEITEM item) const;
        HTREEITEM GetParentItem(HTREEITEM item) const;
        HTREEITEM GetPrevSibling(HTREEITEM item) const;
        HTREEITEM GetPrevVisible(HTREEITEM item) const;
        HTREEITEM GetRootItem() const;
        int     GetScrollTime() const;
        HTREEITEM GetSelection() const;
        COLORREF GetTextColor() const;
        HWND    GetToolTips() const;
        UINT    GetVisibleCount() const;
        BOOL    ItemHasChildren(HTREEITEM item) const;
        COLORREF SetBkColor(COLORREF color) const;
        HIMAGELIST SetImageList(HIMAGELIST images, int type = TVSIL_NORMAL) const;
        void    SetIndent(int indent) const;
        BOOL    SetInsertMark(HTREEITEM item, BOOL after = TRUE) const;
        COLORREF SetInsertMarkColor(COLORREF color) const;
        BOOL    SetItem(TVITEM& itemInfo) const;
        BOOL    SetItem(HTREEITEM item, UINT mask, LPCTSTR text, int image, int selectedImage, UINT state, UINT stateMask, LPARAM lparam) const;
        BOOL    SetItemData(HTREEITEM item, DWORD_PTR data) const;
        int     SetItemHeight(SHORT cy) const;
        BOOL    SetItemImage(HTREEITEM item, int image, int selectedImage) const;
        BOOL    SetItemText(HTREEITEM item, LPCTSTR text) const;
        UINT    SetScrollTime(UINT scrollTime) const;
        COLORREF SetTextColor(COLORREF color) const;
        HWND    SetToolTips(HWND toolTip) const;

        // Operations
        CImageList CreateDragImage(HTREEITEM item) const;
        BOOL    DeleteAllItems() const;
        BOOL    DeleteItem(HTREEITEM item) const;
        HWND    EditLabel(HTREEITEM item) const;
        BOOL    EndEditLabelNow(BOOL cancel) const;
        BOOL    EnsureVisible(HTREEITEM item) const;
        BOOL    Expand(HTREEITEM hItem, UINT code) const;
        HTREEITEM HitTest(TVHITTESTINFO& hitInfo) const;
        HTREEITEM InsertItem(TVINSERTSTRUCT& insertInfo) const;
        HTREEITEM InsertItem(UINT mask, LPCTSTR text, int image,
                             int selectedImage, UINT state, UINT stateMask,
                             LPARAM lparam, HTREEITEM parent,
                             HTREEITEM insertAfter) const;
        HTREEITEM InsertItem(LPCTSTR text, HTREEITEM parent = TVI_ROOT,
                             HTREEITEM insertAfter = TVI_LAST) const;
        HTREEITEM InsertItem(LPCTSTR text, int image, int selectedImage,
                             HTREEITEM parent = TVI_ROOT,
                             HTREEITEM insertAfter = TVI_LAST) const;
        BOOL    Select(HTREEITEM item, UINT flag) const;
        BOOL    SelectDropTarget(HTREEITEM item) const;
        BOOL    SelectItem(HTREEITEM item) const;
        BOOL    SelectSetFirstVisible(HTREEITEM item) const;
        BOOL    SortChildren(HTREEITEM item, BOOL recurse) const;
        BOOL    SortChildrenCB(TVSORTCB* pSortFn, BOOL recurse) const;

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

    // Retrieves the current background color of the control.
    // Refer to TreeView_GetBkColor in the Windows API documentation for more information.
    inline COLORREF CTreeView::GetBkColor() const
    {
        assert(IsWindow());
        return TreeView_GetBkColor( *this );
    }

    // Retrieves the first child item of the specified tree-view item.
    // Refer to TreeView_GetChild in the Windows API documentation for more information.
    inline HTREEITEM CTreeView::GetChild(HTREEITEM item) const
    {
        assert(IsWindow());
        return TreeView_GetChild(*this, item);
    }

    // Retrieves a count of the items in a tree-view control.
    // Refer to TreeView_GetCount in the Windows API documentation for more information.
    inline UINT CTreeView::GetCount() const
    {
        assert(IsWindow());
        return TreeView_GetCount( *this );
    }

    // Retrieves the tree-view item that is the target of a drag-and-drop operation.
    // Refer to TreeView_GetDropHilight in the Windows API documentation for more information.
    inline HTREEITEM CTreeView::GetDropHiLightItem() const
    {
        assert(IsWindow());
        return TreeView_GetDropHilight(*this);
    }

    // Retrieves the handle to the edit control being used to edit a tree-view item's text.
    // Refer to TreeView_GetEditControl in the Windows API documentation for more information.
    inline HWND CTreeView::GetEditControl() const
    {
        assert(IsWindow());
        return TreeView_GetEditControl(*this);
    }

    // Retrieves the first visible item in a tree-view control window.
    // Refer to TreeView_GetFirstVisible in the Windows API documentation for more information.
    inline HTREEITEM CTreeView::GetFirstVisible() const
    {
        assert(IsWindow());
        return TreeView_GetFirstVisible(*this);
    }

    // Retrieves the handle to the normal or state image list associated with a tree-view control.
    // Refer to TreeView_GetImageList in the Windows API documentation for more information.
    inline CImageList CTreeView::GetImageList(int imageType) const
    {
        assert(IsWindow());
        HIMAGELIST images = TreeView_GetImageList( *this, imageType );
        return CImageList(images);
    }

    // Retrieves the amount, in pixels, that child items are indented relative to their parent items.
    // Refer to TreeView_GetIndent in the Windows API documentation for more information.
    inline UINT CTreeView::GetIndent() const
    {
        assert(IsWindow());
        return TreeView_GetIndent( *this );
    }

    // Retrieves the color used to draw the insertion mark for the tree view.
    // Refer to TreeView_GetInsertMarkColor in the Windows API documentation for more information.
    inline COLORREF CTreeView::GetInsertMarkColor() const
    {
        assert(IsWindow());
        return TreeView_GetInsertMarkColor( *this );
    }

    // Retrieves some or all of a tree-view item's attributes.
    // Refer to TreeView_GetItem in the Windows API documentation for more information.
    inline BOOL CTreeView::GetItem(TVITEM& itemInfo) const
    {
        assert(IsWindow());
        return TreeView_GetItem( *this, &itemInfo );
    }

    // Retrieves a tree-view item's application data.
    // Refer to TreeView_GetItem in the Windows API documentation for more information.
    inline DWORD_PTR CTreeView::GetItemData(HTREEITEM item) const
    {
        assert(IsWindow());

        TVITEM tvi;
        ZeroMemory(&tvi, sizeof(tvi));
        tvi.mask = TVIF_PARAM;
        tvi.hItem = item;
        SendMessage(TVM_GETITEM, 0, (LPARAM)&tvi);
        return tvi.lParam;
    }

    // Retrieves the current height of the tree-view item.
    // Refer to TreeView_GetItemHeight in the Windows API documentation for more information.
    inline int  CTreeView::GetItemHeight() const
    {
        assert(IsWindow());
        return TreeView_GetItemHeight( *this );
    }

    // Retrieves the index of the tree-view item's image and selected image.
    // Refer to TreeView_GetItem in the Windows API documentation for more information.
    inline BOOL CTreeView::GetItemImage(HTREEITEM item, int& image, int& selectedImage ) const
    {
        assert(IsWindow());

        TVITEM tvi;
        ZeroMemory(&tvi, sizeof(tvi));
        tvi.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE;
        tvi.hItem = item;
        BOOL result = TreeView_GetItem( *this, &tvi );
        image = tvi.iImage;
        selectedImage = tvi.iSelectedImage;
        return result;
    }

    // Retrieves the bounding rectangle for a tree-view item and indicates whether the item is visible.
    // Refer to TreeView_GetItemRect in the Windows API documentation for more information.
    inline BOOL CTreeView::GetItemRect(HTREEITEM item, RECT& rc, BOOL isTextOnly) const
    {
        assert(IsWindow());

        *reinterpret_cast<HTREEITEM*>(&rc) = item;
        return (BOOL)SendMessage(TVM_GETITEMRECT, (WPARAM)isTextOnly, (LPARAM)&rc);
    }

    // Retrieves the text for a tree-view item.
    // Retrieves the text for a tree-view item.
    // Note: Although the tree-view control allows any length string to be stored
    //       as item text, only the first 260 characters are displayed.
    // Refer to TVM_GETITEM in the Windows API documentation for more information.
    inline CString CTreeView::GetItemText(HTREEITEM item, UINT textMax /* = 260 */) const
    {
        assert(IsWindow());

        CString str;
        if (textMax > 0)
        {
            TVITEM tvi;
            ZeroMemory(&tvi, sizeof(tvi));
            tvi.hItem = item;
            tvi.mask = TVIF_TEXT;
            tvi.cchTextMax = textMax;
            tvi.pszText = str.GetBuffer(textMax);
            SendMessage(TVM_GETITEM, 0, (LPARAM)&tvi);
            str.ReleaseBuffer();
        }
        return str;
    }

    // Retrieves the last expanded item in a tree-view control.
    // This does not retrieve the last item visible in the tree-view window.
    // Refer to TreeView_GetLastVisible in the Windows API documentation for more information.
    inline HTREEITEM CTreeView::GetLastVisible() const
    {
        assert(IsWindow());
        return TreeView_GetLastVisible(*this);
    }

    // Retrieves the tree-view item that bears the specified relationship to a specified item.
    // Refer to TreeView_GetNextItem in the Windows API documentation for more information.
    inline HTREEITEM CTreeView::GetNextItem(HTREEITEM item, UINT code) const
    {
        assert(IsWindow());
        return TreeView_GetNextItem( *this, item, code);
    }

    // Retrieves the next sibling item of a specified item in a tree-view control.
    // Refer to TreeView_GetNextSibling in the Windows API documentation for more information.
    inline HTREEITEM CTreeView::GetNextSibling(HTREEITEM item) const
    {
        assert(IsWindow());
        return TreeView_GetNextSibling(*this, item);
    }

    // Retrieves the next visible item that follows a specified item in a tree-view control.
    // Refer to TreeView_GetNextVisible in the Windows API documentation for more information.
    inline HTREEITEM CTreeView::GetNextVisible(HTREEITEM item) const
    {
        assert(IsWindow());
        return TreeView_GetNextVisible(*this, item);
    }

    // Retrieves the parent item of the specified tree-view item.
    // Refer to TreeView_GetParent in the Windows API documentation for more information.
    inline HTREEITEM CTreeView::GetParentItem(HTREEITEM item) const
    {
        assert(IsWindow());
        return TreeView_GetParent(*this, item);
    }

    // Retrieves the previous sibling item of a specified item in a tree-view control.
    // Refer to TreeView_GetPrevSibling in the Windows API documentation for more information.
    inline HTREEITEM CTreeView::GetPrevSibling(HTREEITEM item) const
    {
        assert(IsWindow());
        return TreeView_GetPrevSibling(*this, item);
    }

    // Retrieves the first visible item that precedes a specified item in a tree-view control.
    // Refer to TreeView_GetPrevVisible in the Windows API documentation for more information.
    inline HTREEITEM CTreeView::GetPrevVisible(HTREEITEM item) const
    {
        assert(IsWindow());
        return TreeView_GetPrevVisible(*this, item);
    }

    // Retrieves the topmost or very first item of the tree-view control.
    // Refer to TreeView_GetRoot in the Windows API documentation for more information.
    inline HTREEITEM CTreeView::GetRootItem() const
    {
        assert(IsWindow());
        return TreeView_GetRoot(*this);
    }

    // Retrieves the maximum scroll time for the tree-view control.
    // Refer to TreeView_GetScrollTime in the Windows API documentation for more information.
    inline int CTreeView::GetScrollTime() const
    {
        assert(IsWindow());
        return TreeView_GetScrollTime( *this );
    }

    // Retrieves the currently selected item in a tree-view control.
    // Refer to TreeView_GetSelection in the Windows API documentation for more information.
    inline HTREEITEM CTreeView::GetSelection() const
    {
        assert(IsWindow());
        return TreeView_GetSelection(*this);
    }

    // Retrieves the current text color of the control.
    // Refer to TreeView_GetTextColor in the Windows API documentation for more information.
    inline COLORREF CTreeView::GetTextColor() const
    {
        assert(IsWindow());
        return TreeView_GetTextColor( *this );
    }

    // Retrieves the handle to the child ToolTip control used by a tree-view control.
    // Refer to TreeView_GetToolTips in the Windows API documentation for more information.
    inline HWND CTreeView::GetToolTips() const
    {
        assert(IsWindow());
        return TreeView_GetToolTips(*this);
    }

    // Obtains the number of items that can be fully visible in the client window of a tree-view control.
    // Refer to TreeView_GetVisibleCount in the Windows API documentation for more information.
    inline UINT CTreeView::GetVisibleCount() const
    {
        assert(IsWindow());
        return TreeView_GetVisibleCount( *this );
    }

    // Returns true of the tree-view item has one or more children.
    // Refer to TreeView_GetChild in the Windows API documentation for more information.
    inline BOOL CTreeView::ItemHasChildren(HTREEITEM item) const
    {
        assert(IsWindow());

        if (TreeView_GetChild( *this, item))
            return TRUE;

        return FALSE;
    }

    // Sets the background color of the control.
    // Refer to TreeView_SetBkColor in the Windows API documentation for more information.
    inline COLORREF CTreeView::SetBkColor(COLORREF color) const
    {
        assert(IsWindow());
        return TreeView_SetBkColor( *this, color);
    }

    // Sets the normal or state image list for a tree-view control
    // and redraws the control using the new images.
    // Refer to TreeView_SetImageList in the Windows API documentation for more information.
    inline HIMAGELIST CTreeView::SetImageList(HIMAGELIST images, int type /*= TVSIL_NORMAL*/) const
    {
        assert(IsWindow());
        HIMAGELIST oldImages = TreeView_SetImageList( *this, images, type );
        return oldImages;
    }

    // Sets the width of indentation for a tree-view control
    // and redraws the control to reflect the new width.
    // Refer to TreeView_SetIndent in the Windows API documentation for more information.
    inline void CTreeView::SetIndent(int indent) const
    {
        assert(IsWindow());
        SendMessage(TVM_SETINDENT, (WPARAM)indent, 0);
    }

    // Sets the insertion mark in a tree-view control.
    // Refer to TreeView_SetInsertMark in the Windows API documentation for more information.
    inline BOOL CTreeView::SetInsertMark(HTREEITEM item, BOOL after/* = TRUE*/) const
    {
        assert(IsWindow());
        return TreeView_SetInsertMark( *this, item, after );
    }

    // Sets the color used to draw the insertion mark for the tree view.
    // Refer to TreeView_SetInsertMarkColor in the Windows API documentation for more information.
    inline COLORREF CTreeView::SetInsertMarkColor(COLORREF color) const
    {
        assert(IsWindow());
        return TreeView_SetInsertMarkColor( *this, color );
    }

    // Sets some or all of a tree-view item's attributes.
    // Refer to TreeView_SetItem in the Windows API documentation for more information.
    inline BOOL CTreeView::SetItem(TVITEM& item) const
    {
        assert(IsWindow());
        return TreeView_SetItem( *this, &item);
    }

    // Sets some or all of a tree-view item's attributes.
    // Refer to TreeView_SetItem in the Windows API documentation for more information.
    inline BOOL CTreeView::SetItem(HTREEITEM item, UINT mask, LPCTSTR text, int image, int selectedImage, UINT state, UINT stateMask, LPARAM lparam) const
    {
        assert(IsWindow());

        TVITEM tvi;
        ZeroMemory(&tvi, sizeof(tvi));
        tvi.hItem = item;
        tvi.mask  = mask;
        tvi.pszText = const_cast<LPTSTR>(text);
        tvi.iImage  = image;
        tvi.iSelectedImage = selectedImage;
        tvi.state = state;
        tvi.stateMask = stateMask;
        tvi.lParam = lparam;
        return TreeView_SetItem( *this, &tvi );
    }

    // Sets the tree-view item's application data.
    // Refer to TreeView_SetItem in the Windows API documentation for more information.
    inline BOOL CTreeView::SetItemData(HTREEITEM item, DWORD_PTR data) const
    {
        assert(IsWindow());

        TVITEM tvi;
        ZeroMemory(&tvi, sizeof(tvi));
        tvi.hItem = item;
        tvi.mask = TVIF_PARAM;
        tvi.lParam = data;
        return TreeView_SetItem( *this, &tvi );
    }

    // Sets the height of all the tree-view items.
    // Refer to TreeView_SetItemHeight in the Windows API documentation for more information.
    inline int  CTreeView::SetItemHeight(SHORT cy) const
    {
        assert(IsWindow());
        return TreeView_SetItemHeight( *this, cy );
    }

    // Sets the tree-view item's application image.
    // Refer to TreeView_SetItem in the Windows API documentation for more information.
    inline BOOL CTreeView::SetItemImage(HTREEITEM item, int image, int selectedImage) const
    {
        assert(IsWindow());

        TVITEM tvi;
        ZeroMemory(&tvi, sizeof(tvi));
        tvi.hItem = item;
        tvi.iImage = image;
        tvi.iSelectedImage = selectedImage;
        tvi.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE;
        return TreeView_SetItem(*this, &tvi );
    }

    // Sets the tree-view item's application text.
    // Refer to TreeView_SetItem in the Windows API documentation for more information.
    inline BOOL CTreeView::SetItemText(HTREEITEM item, LPCTSTR text) const
    {
        assert(IsWindow());

        TVITEM tvi;
        ZeroMemory(&tvi, sizeof(tvi));
        tvi.hItem = item;
        tvi.pszText = const_cast<LPTSTR>(text);
        tvi.mask = TVIF_TEXT;
        return TreeView_SetItem(*this, &tvi );
    }

    // Sets the maximum scroll time for the tree-view control.
    // Refer to TreeView_SetScrollTime in the Windows API documentation for more information.
    inline UINT CTreeView::SetScrollTime(UINT scrollTime) const
    {
        assert(IsWindow());
        return TreeView_SetScrollTime( *this, scrollTime );
    }

    // Sets the text color of the control.
    // Refer to TreeView_SetTextColor in the Windows API documentation for more information.
    inline COLORREF CTreeView::SetTextColor(COLORREF color) const
    {
        assert(IsWindow());
        return TreeView_SetTextColor( *this, color );
    }

    // Sets a tree-view control's child ToolTip control.
    // Refer to TreeView_SetToolTips in the Windows API documentation for more information.
    inline HWND CTreeView::SetToolTips(HWND toolTip) const
    {
        assert(IsWindow());
        return TreeView_SetToolTips(*this, toolTip);
    }

    // Operations

    // Creates a dragging bitmap for the specified item in a tree-view control.
    // It also creates an image list for the bitmap and adds the bitmap to the image list.
    // An application can display the image when dragging the item by using the image list functions.
    // Refer to TreeView_CreateDragImage in the Windows API documentation for more information.
    inline CImageList CTreeView::CreateDragImage(HTREEITEM item) const
    {
        assert(IsWindow());
        HIMAGELIST images = TreeView_CreateDragImage(*this, item);
        return CImageList(images);
    }

    // Deletes all items from a tree-view control.
    // Refer to TreeView_DeleteAllItems in the Windows API documentation for more information.
    inline BOOL CTreeView::DeleteAllItems() const
    {
        assert(IsWindow());
        return TreeView_DeleteAllItems( *this );
    }

    // Removes an item and all its children from a tree-view control.
    // Refer to TreeView_DeleteItem in the Windows API documentation for more information.
    inline BOOL CTreeView::DeleteItem(HTREEITEM item) const
    {
        assert(IsWindow());
        return TreeView_DeleteItem( *this, item);
    }

    // Begins in-place editing of the specified item's text, replacing the text of the item
    // with a single-line edit control containing the text.
    // The specified item  is implicitly selected and focused.
    // Refer to TreeView_EditLabel in the Windows API documentation for more information.
    inline HWND CTreeView::EditLabel(HTREEITEM item) const
    {
        assert(IsWindow());
        return TreeView_EditLabel( *this, item);
    }

    // Ends the editing of a tree-view item's label.
    // Refer to TreeView_EndEditLabelNow in the Windows API documentation for more information.
    inline BOOL CTreeView::EndEditLabelNow(BOOL cancel) const
    {
        assert(IsWindow());
        return TreeView_EndEditLabelNow(*this, cancel);
    }

    // Ensures that a tree-view item is visible, expanding the parent item or
    // scrolling the tree-view control, if necessary.
    // Refer to TreeView_EnsureVisible in the Windows API documentation for more information.
    inline BOOL CTreeView::EnsureVisible(HTREEITEM item) const
    {
        assert(IsWindow());
        return TreeView_EnsureVisible( *this, item);
    }

    // The TreeView_Expand macro expands or collapses the list of child items associated
    // with the specified parent item, if any.
    // Refer to TreeView_Expand in the Windows API documentation for more information.
    inline BOOL CTreeView::Expand(HTREEITEM item, UINT code) const
    {
        assert(IsWindow());
        return TreeView_Expand( *this, item, code );
    }

    // Determines the location of the specified point relative to the client area of a tree-view control.
    // Refer to TreeView_HitTest in the Windows API documentation for more information.
    inline HTREEITEM CTreeView::HitTest(TVHITTESTINFO& hitInfo) const
    {
        assert(IsWindow());
        return TreeView_HitTest( *this, &hitInfo );
    }

    // Inserts a new item in a tree-view control.
    // Refer to TreeView_InsertItem in the Windows API documentation for more information.
    inline HTREEITEM CTreeView::InsertItem(TVINSERTSTRUCT& insertInfo) const
    {
        assert(IsWindow());
        return TreeView_InsertItem( *this, &insertInfo );
    }

    // Inserts a new item in a tree-view control.
    // Refer to TreeView_InsertItem in the Windows API documentation for more information.
    inline HTREEITEM CTreeView::InsertItem(UINT mask, LPCTSTR text, int image,
                                    int selectedImage, UINT state, UINT stateMask,
                                    LPARAM lparam, HTREEITEM parent,
                                    HTREEITEM insertAfter) const
    {
        TVITEM tvi;
        ZeroMemory(&tvi, sizeof(tvi));
        tvi.mask = mask;
        tvi.iImage = image;
        tvi.iSelectedImage = selectedImage;
        tvi.state = state;
        tvi.stateMask = stateMask;
        tvi.lParam = lparam;
        tvi.pszText = const_cast<LPTSTR>(text);

        TVINSERTSTRUCT tvis;
        ZeroMemory(&tvis, sizeof(tvis));
        tvis.hParent = parent;
        tvis.hInsertAfter = insertAfter;
        tvis.item = tvi;

        return InsertItem(tvis);
    }

    // Inserts a new item in a tree-view control.
    // Refer to TreeView_InsertItem in the Windows API documentation for more information.
    inline HTREEITEM CTreeView::InsertItem(LPCTSTR text, HTREEITEM parent,
                                    HTREEITEM insertAfter) const
    {
        TVITEM tvi;
        ZeroMemory(&tvi, sizeof(tvi));
        tvi.mask = TVIF_TEXT;
        tvi.pszText = const_cast<LPTSTR>(text);

        TVINSERTSTRUCT tvis;
        ZeroMemory(&tvis, sizeof(tvis));
        tvis.hParent = parent;
        tvis.hInsertAfter = insertAfter;
        tvis.item = tvi;

        return InsertItem(tvis);
    }

    // Inserts a new item in a tree-view control.
    // Refer to TreeView_InsertItem in the Windows API documentation for more information.
    inline HTREEITEM CTreeView::InsertItem(LPCTSTR text, int image, int selectedImage,
                                    HTREEITEM parent, HTREEITEM insertAfter) const
    {
        TVITEM tvi;
        ZeroMemory(&tvi, sizeof(tvi));
        tvi.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
        tvi.iImage = image;
        tvi.iSelectedImage = selectedImage;
        tvi.pszText = const_cast<LPTSTR>(text);

        TVINSERTSTRUCT tvis;
        ZeroMemory(&tvis, sizeof(tvis));
        tvis.hParent = parent;
        tvis.hInsertAfter = insertAfter;
        tvis.item = tvi;

        return InsertItem(tvis);
    }

    // Selects the specified tree-view item, scrolls the item into view, or redraws
    // the item in the style used to indicate the target of a drag-and-drop operation.
    // Refer to TreeView_Select in the Windows API documentation for more information.
    inline BOOL CTreeView::Select(HTREEITEM item, UINT flag) const
    {
        assert(IsWindow());
        return TreeView_Select(*this, item, flag );
    }

    // Redraws a specified tree-view control item in the style used to indicate the
    // target of a drag-and-drop operation.
    // Refer to TreeView_SelectDropTarget in the Windows API documentation for more information.
    inline BOOL CTreeView::SelectDropTarget(HTREEITEM item) const
    {
        assert(IsWindow());
        return TreeView_SelectDropTarget(*this, item);
    }

    // Selects the specified tree-view item.
    // Refer to TreeView_SelectItem in the Windows API documentation for more information.
    inline BOOL CTreeView::SelectItem(HTREEITEM item) const
    {
        assert(IsWindow());
        return TreeView_SelectItem(*this, item);
    }

    // Scrolls the tree-view control vertically to ensure that the specified item is visible.
    // If possible, the specified item becomes the first visible item at the top of the control's window.
    // Refer to TreeView_SelectSetFirstVisible in the Windows API documentation for more information.
    inline BOOL CTreeView::SelectSetFirstVisible(HTREEITEM item) const
    {
        assert(IsWindow());
        return TreeView_SelectSetFirstVisible(*this, item);
    }

    // Sorts the child items of the specified parent item in a tree-view control.
    // Refer to TreeView_SortChildren in the Windows API documentation for more information.
    inline BOOL CTreeView::SortChildren(HTREEITEM item, BOOL recurse) const
    {
        assert(IsWindow());
        return TreeView_SortChildren( *this, item, recurse );
    }

    // Sorts tree-view items using an application-defined callback function that compares the items.
    // Refer to TreeView_SortChildrenCB in the Windows API documentation for more information.
    inline BOOL CTreeView::SortChildrenCB(TVSORTCB* pSortFn, BOOL recurse) const
    {
        assert(IsWindow());
        return TreeView_SortChildrenCB( *this, pSortFn, recurse );
    }


} // namespace Win32xx

#endif // #ifndef _WIN32XX_TREEVIEW_H_

