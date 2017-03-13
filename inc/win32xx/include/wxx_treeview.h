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

	class CTreeView : public CWnd
	{
	public:
		CTreeView() {}
		virtual ~CTreeView() {}
		virtual void PreRegisterClass(WNDCLASS& wc);

// Attributes
		COLORREF GetBkColor() const;
		HTREEITEM GetChild(HTREEITEM hItem) const;
		UINT	GetCount() const;
		HTREEITEM GetDropHiLightItem() const;
		HWND	GetEditControl() const;
		HTREEITEM GetFirstVisible() const;
		CImageList GetImageList(int iImageType) const;
		UINT	GetIndent() const;
		COLORREF GetInsertMarkColor() const;
		BOOL	GetItem(TVITEM& Item) const;
		DWORD_PTR GetItemData(HTREEITEM hItem) const;
		int		GetItemHeight() const;
		BOOL	GetItemImage(HTREEITEM hItem, int& nImage, int& nSelectedImage ) const;
		BOOL	GetItemRect(HTREEITEM hItem, CRect& rc, BOOL IsTextOnly) const;
		CString GetItemText(HTREEITEM hItem, UINT nTextMax /* = 260 */) const;
		HTREEITEM GetLastVisible() const;
		HTREEITEM GetNextItem(HTREEITEM hItem, UINT nCode) const;
		HTREEITEM GetNextSibling(HTREEITEM hItem) const;
		HTREEITEM GetNextVisible(HTREEITEM hItem) const;
		HTREEITEM GetParentItem(HTREEITEM hItem) const;
		HTREEITEM GetPrevSibling(HTREEITEM hItem) const;
		HTREEITEM GetPrevVisible(HTREEITEM hItem) const;
		HTREEITEM GetRootItem() const;
		int		GetScrollTime() const;
		HTREEITEM GetSelection() const;
		COLORREF GetTextColor() const;
		HWND	GetToolTips() const;
		UINT	GetVisibleCount() const;
		BOOL	ItemHasChildren(HTREEITEM hItem) const;
		COLORREF SetBkColor(COLORREF clrBk) const;
		CImageList SetImageList(HIMAGELIST himlNew, int nType) const;
		void	SetIndent(int indent) const;
		BOOL	SetInsertMark(HTREEITEM hItem, BOOL After = TRUE) const;
		COLORREF SetInsertMarkColor(COLORREF clrInsertMark) const;
		BOOL	SetItem(TVITEM& Item) const;
		BOOL	SetItem(HTREEITEM hItem, UINT nMask, LPCTSTR szText, int nImage, int nSelectedImage, UINT nState, UINT nStateMask, LPARAM lParam) const;
		BOOL	SetItemData(HTREEITEM hItem, DWORD_PTR dwData) const;
		int		SetItemHeight(SHORT cyItem) const;
		BOOL	SetItemImage(HTREEITEM hItem, int nImage, int nSelectedImage) const;
		BOOL	SetItemText(HTREEITEM hItem, LPCTSTR szText) const;
		UINT	SetScrollTime(UINT uScrollTime) const;
		COLORREF SetTextColor(COLORREF clrText) const;
		HWND	SetToolTips(HWND hToolTip) const;

// Operations
		CImageList CreateDragImage(HTREEITEM hItem) const;
		BOOL	DeleteAllItems() const;
		BOOL	DeleteItem(HTREEITEM hItem) const;
		HWND	EditLabel(HTREEITEM hItem) const;
		BOOL	EndEditLabelNow(BOOL fCancel) const;
		BOOL	EnsureVisible(HTREEITEM hItem) const;
		BOOL	Expand(HTREEITEM hItem, UINT nCode) const;
		HTREEITEM HitTest(TVHITTESTINFO& ht) const;
		HTREEITEM InsertItem(TVINSERTSTRUCT& tvIS) const;
		BOOL	Select(HTREEITEM hitem, UINT flag) const;
		BOOL	SelectDropTarget(HTREEITEM hItem) const;
		BOOL	SelectItem(HTREEITEM hItem) const;
		BOOL	SelectSetFirstVisible(HTREEITEM hItem) const;
		BOOL	SortChildren(HTREEITEM hItem, BOOL Recurse) const;
		BOOL	SortChildrenCB(TVSORTCB& sort, BOOL Recurse) const;

	private:
		CTreeView(const CTreeView&);				// Disable copy construction
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
	inline COLORREF CTreeView::GetBkColor() const
	// Retrieves the current background color of the control.
	{
		assert(IsWindow());
		return TreeView_GetBkColor( *this );
	}

	inline HTREEITEM CTreeView::GetChild(HTREEITEM hItem) const
	// Retrieves the first child item of the specified tree-view item.
	{
		assert(IsWindow());
		return TreeView_GetChild(*this, hItem);
	}

	inline UINT CTreeView::GetCount() const
	// Retrieves a count of the items in a tree-view control.
	{
		assert(IsWindow());
		return TreeView_GetCount( *this );
	}

	inline HTREEITEM CTreeView::GetDropHiLightItem() const
	// Retrieves the tree-view item that is the target of a drag-and-drop operation.
	{
		assert(IsWindow());
		return TreeView_GetDropHilight(*this);
	}

	inline HWND CTreeView::GetEditControl() const
	// Retrieves the handle to the edit control being used to edit a tree-view item's text.
	{
		assert(IsWindow());
		return TreeView_GetEditControl(*this);
	}

	inline HTREEITEM CTreeView::GetFirstVisible() const
	// Retrieves the first visible item in a tree-view control window.
	{
		assert(IsWindow());
		return TreeView_GetFirstVisible(*this);
	}

	inline CImageList CTreeView::GetImageList(int iImageType) const
	// Retrieves the handle to the normal or state image list associated with a tree-view control.
	{
		assert(IsWindow());
		HIMAGELIST himl = TreeView_GetImageList( *this, iImageType );
		return CImageList(himl);
	}

	inline UINT CTreeView::GetIndent() const
	// Retrieves the amount, in pixels, that child items are indented relative to their parent items.
	{
		assert(IsWindow());
		return TreeView_GetIndent( *this );
	}

	inline COLORREF CTreeView::GetInsertMarkColor() const
	// Retrieves the color used to draw the insertion mark for the tree view.
	{
		assert(IsWindow());
		return TreeView_GetInsertMarkColor( *this );
	}

	inline BOOL CTreeView::GetItem(TVITEM& Item) const
	// Retrieves some or all of a tree-view item's attributes.
	{
		assert(IsWindow());
		return TreeView_GetItem( *this, &Item );
	}

	inline DWORD_PTR CTreeView::GetItemData(HTREEITEM hItem) const
	// Retrieves a tree-view item's application data.
	{
		assert(IsWindow());

		TVITEM tvi;
		ZeroMemory(&tvi, sizeof(TVITEM));
		tvi.mask = TVIF_PARAM;
		tvi.hItem = hItem;
		TreeView_GetItem( *this, &tvi );
		return tvi.lParam;
	}

	inline int  CTreeView::GetItemHeight() const
	// Retrieves the current height of the tree-view item.
	{
		assert(IsWindow());
		return TreeView_GetItemHeight( *this );
	}

	inline BOOL CTreeView::GetItemImage(HTREEITEM hItem, int& nImage, int& nSelectedImage ) const
	// Retrieves the index of the tree-view item's image and selected image.
	{
		assert(IsWindow());

		TVITEM tvi;
		ZeroMemory(&tvi, sizeof(TVITEM));
		tvi.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE;
		tvi.hItem = hItem;
		BOOL Succeeded = TreeView_GetItem( *this, &tvi );
		nImage = tvi.iImage;
		nSelectedImage = tvi.iSelectedImage;
		return Succeeded;
	}

	inline BOOL CTreeView::GetItemRect(HTREEITEM hItem, CRect& rc, BOOL IsTextOnly) const
	// Retrieves the bounding rectangle for a tree-view item and indicates whether the item is visible.
	{
		assert(IsWindow());
		return TreeView_GetItemRect( *this, hItem, &rc, IsTextOnly );
	}

	inline CString CTreeView::GetItemText(HTREEITEM hItem, UINT nTextMax /* = 260 */) const
	// Retrieves the text for a tree-view item.
	// Note: Although the tree-view control allows any length string to be stored 
	//       as item text, only the first 260 characters are displayed.
	{
		assert(IsWindow());

		CString str;
		if (nTextMax > 0)
		{
			TVITEM tvi;
			ZeroMemory(&tvi, sizeof(TVITEM));
			tvi.hItem = hItem;
			tvi.mask = TVIF_TEXT;
			tvi.cchTextMax = nTextMax;
			tvi.pszText = str.GetBuffer(nTextMax);
			SendMessage(TVM_GETITEM, 0L, (LPARAM)&tvi);
			str.ReleaseBuffer();
		}
		return str;
	}

	inline HTREEITEM CTreeView::GetLastVisible() const
	// Retrieves the last expanded item in a tree-view control.
	// This does not retrieve the last item visible in the tree-view window.
	{
		assert(IsWindow());
		return TreeView_GetLastVisible(*this);
	}

	inline HTREEITEM CTreeView::GetNextItem(HTREEITEM hItem, UINT nCode) const
	// Retrieves the tree-view item that bears the specified relationship to a specified item.
	{
		assert(IsWindow());
		return TreeView_GetNextItem( *this, hItem, nCode);
	}

	inline HTREEITEM CTreeView::GetNextSibling(HTREEITEM hItem) const
	// Retrieves the next sibling item of a specified item in a tree-view control.
	{
		assert(IsWindow());
		return TreeView_GetNextSibling(*this, hItem);
	}

	inline HTREEITEM CTreeView::GetNextVisible(HTREEITEM hItem) const
	// Retrieves the next visible item that follows a specified item in a tree-view control.
	{
		assert(IsWindow());
		return TreeView_GetNextVisible(*this, hItem);
	}

	inline HTREEITEM CTreeView::GetParentItem(HTREEITEM hItem) const
	// Retrieves the parent item of the specified tree-view item.
	{
		assert(IsWindow());
		return TreeView_GetParent(*this, hItem);
	}

	inline HTREEITEM CTreeView::GetPrevSibling(HTREEITEM hItem) const
	// Retrieves the previous sibling item of a specified item in a tree-view control.
	{
		assert(IsWindow());
		return TreeView_GetPrevSibling(*this, hItem);
	}

	inline HTREEITEM CTreeView::GetPrevVisible(HTREEITEM hItem) const
	// Retrieves the first visible item that precedes a specified item in a tree-view control.
	{
		assert(IsWindow());
		return TreeView_GetPrevVisible(*this, hItem);
	}

	inline HTREEITEM CTreeView::GetRootItem() const
	// Retrieves the topmost or very first item of the tree-view control.
	{
		assert(IsWindow());
		return TreeView_GetRoot(*this);
	}

	inline int CTreeView::GetScrollTime() const
	// Retrieves the maximum scroll time for the tree-view control.
	{
		assert(IsWindow());
		return TreeView_GetScrollTime( *this );
	}

	inline HTREEITEM CTreeView::GetSelection() const
	// Retrieves the currently selected item in a tree-view control.
	{
		assert(IsWindow());
		return TreeView_GetSelection(*this);
	}

	inline COLORREF CTreeView::GetTextColor() const
	// Retrieves the current text color of the control.
	{
		assert(IsWindow());
		return TreeView_GetTextColor( *this );
	}

	inline HWND CTreeView::GetToolTips() const
	// Retrieves the handle to the child ToolTip control used by a tree-view control.
	{
		assert(IsWindow());
		return TreeView_GetToolTips(*this);
	}

	inline UINT CTreeView::GetVisibleCount() const
	// Obtains the number of items that can be fully visible in the client window of a tree-view control.
	{
		assert(IsWindow());
		return TreeView_GetVisibleCount( *this );
	}

	inline BOOL CTreeView::ItemHasChildren(HTREEITEM hItem) const
	// Returns true of the tree-view item has one or more children
	{
		assert(IsWindow());

		if (TreeView_GetChild( *this, hItem ))
			return TRUE;

		return FALSE;
	}

	inline COLORREF CTreeView::SetBkColor(COLORREF clrBk) const
	// Sets the background color of the control.
	{
		assert(IsWindow());
		return TreeView_SetBkColor( *this, clrBk );
	}

	inline CImageList CTreeView::SetImageList(HIMAGELIST himlNew, int nType) const
	// Sets the normal or state image list for a tree-view control
	//  and redraws the control using the new images.
	{
		assert(IsWindow());
		HIMAGELIST himl = TreeView_SetImageList( *this, himlNew, nType );
		return CImageList(himl);
	}

	inline void CTreeView::SetIndent(int indent) const
	// Sets the width of indentation for a tree-view control
	//  and redraws the control to reflect the new width.
	{
		assert(IsWindow());
		TreeView_SetIndent( *this, indent );
	}

	inline BOOL CTreeView::SetInsertMark(HTREEITEM hItem, BOOL After/* = TRUE*/) const
	// Sets the insertion mark in a tree-view control.
	{
		assert(IsWindow());
		return TreeView_SetInsertMark( *this, hItem, After );
	}

	inline COLORREF CTreeView::SetInsertMarkColor(COLORREF clrInsertMark) const
	// Sets the color used to draw the insertion mark for the tree view.
	{
		assert(IsWindow());
		return TreeView_SetInsertMarkColor( *this, clrInsertMark );
	}

	inline BOOL CTreeView::SetItem(TVITEM& Item) const
	// Sets some or all of a tree-view item's attributes.
	{
		assert(IsWindow());
		return TreeView_SetItem( *this, &Item );
	}

	inline BOOL CTreeView::SetItem(HTREEITEM hItem, UINT nMask, LPCTSTR szText, int nImage, int nSelectedImage, UINT nState, UINT nStateMask, LPARAM lParam) const
	// Sets some or all of a tree-view item's attributes.
	{
		assert(IsWindow());

		TVITEM tvi;
		ZeroMemory(&tvi, sizeof(TVITEM));
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

	inline BOOL CTreeView::SetItemData(HTREEITEM hItem, DWORD_PTR dwData) const
	// Sets the tree-view item's application data.
	{
		assert(IsWindow());

		TVITEM tvi;
		ZeroMemory(&tvi, sizeof(TVITEM));
		tvi.hItem = hItem;
		tvi.mask = TVIF_PARAM;
		tvi.lParam = dwData;
		return TreeView_SetItem( *this, &tvi );
	}

	inline int  CTreeView::SetItemHeight(SHORT cyItem) const
	// Sets the height of the tree-view items.
	{
		assert(IsWindow());
		return TreeView_SetItemHeight( *this, cyItem );
	}

	inline BOOL CTreeView::SetItemImage(HTREEITEM hItem, int nImage, int nSelectedImage) const
	// Sets the tree-view item's application image.
	{
		assert(IsWindow());

		TVITEM tvi;
		ZeroMemory(&tvi, sizeof(TVITEM));
		tvi.hItem = hItem;
		tvi.iImage = nImage;
		tvi.iSelectedImage = nSelectedImage;
		tvi.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE;
		return TreeView_SetItem(*this, &tvi );
	}

	inline BOOL CTreeView::SetItemText(HTREEITEM hItem, LPCTSTR szText) const
	// Sets the tree-view item's application text.
	{
		assert(IsWindow());

		TVITEM tvi;
		ZeroMemory(&tvi, sizeof(TVITEM));
		tvi.hItem = hItem;
		tvi.pszText = const_cast<LPTSTR>(szText);
		tvi.mask = TVIF_TEXT;
		return TreeView_SetItem(*this, &tvi );
	}

	inline UINT CTreeView::SetScrollTime(UINT uScrollTime) const
	// Sets the maximum scroll time for the tree-view control.
	{
		assert(IsWindow());
		return TreeView_SetScrollTime( *this, uScrollTime );
	}

	inline COLORREF CTreeView::SetTextColor(COLORREF clrText) const
	// Sets the text color of the control.
	{
		assert(IsWindow());
		return TreeView_SetTextColor( *this, clrText );
	}

	inline HWND CTreeView::SetToolTips(HWND hToolTip) const
	// Sets a tree-view control's child ToolTip control.
	{
		assert(IsWindow());
		return TreeView_SetToolTips(*this, hToolTip);
	}

	// Operations

	inline CImageList CTreeView::CreateDragImage(HTREEITEM hItem) const
	// Creates a dragging bitmap for the specified item in a tree-view control.
	// It also creates an image list for the bitmap and adds the bitmap to the image list.
	// An application can display the image when dragging the item by using the image list functions.
	{
		assert(IsWindow());
		HIMAGELIST himl = TreeView_CreateDragImage(*this, hItem);
		return CImageList(himl);
	}

	inline BOOL CTreeView::DeleteAllItems() const
	// Deletes all items from a tree-view control.
	{
		assert(IsWindow());
		return TreeView_DeleteAllItems( *this );
	}

	inline BOOL CTreeView::DeleteItem(HTREEITEM hItem) const
	// Removes an item and all its children from a tree-view control.
	{
		assert(IsWindow());
		return TreeView_DeleteItem( *this, hItem );
	}

	inline HWND CTreeView::EditLabel(HTREEITEM hItem) const
	// Begins in-place editing of the specified item's text, replacing the text of the item
	// with a single-line edit control containing the text.
	// The specified item  is implicitly selected and focused.
	{
		assert(IsWindow());
		return TreeView_EditLabel( *this, hItem );
	}

	inline BOOL CTreeView::EndEditLabelNow(BOOL fCancel) const
	// Ends the editing of a tree-view item's label.
	{
		assert(IsWindow());
		return TreeView_EndEditLabelNow(*this, fCancel);
	}

	inline BOOL CTreeView::EnsureVisible(HTREEITEM hItem) const
	// Ensures that a tree-view item is visible, expanding the parent item or
	// scrolling the tree-view control, if necessary.
	{
		assert(IsWindow());
		return TreeView_EnsureVisible( *this, hItem );
	}

	inline BOOL CTreeView::Expand(HTREEITEM hItem, UINT nCode) const
	// The TreeView_Expand macro expands or collapses the list of child items associated
	// with the specified parent item, if any.
	{
		assert(IsWindow());
		return TreeView_Expand( *this, hItem, nCode );
	}

	inline HTREEITEM CTreeView::HitTest(TVHITTESTINFO& ht) const
	// Determines the location of the specified point relative to the client area of a tree-view control.
	{
		assert(IsWindow());
		return TreeView_HitTest( *this, &ht );
	}

	inline HTREEITEM CTreeView::InsertItem(TVINSERTSTRUCT& tvIS) const
	// Inserts a new item in a tree-view control.
	{
		assert(IsWindow());
		return TreeView_InsertItem( *this, &tvIS );
	}

	inline BOOL CTreeView::Select(HTREEITEM hitem, UINT flag) const
	// Selects the specified tree-view item, scrolls the item into view, or redraws
	// the item in the style used to indicate the target of a drag-and-drop operation.
	{
		assert(IsWindow());
		return TreeView_Select(*this, hitem, flag );
	}

	inline BOOL CTreeView::SelectDropTarget(HTREEITEM hItem) const
	// Redraws a specified tree-view control item in the style used to indicate the
	// target of a drag-and-drop operation.
	{
		assert(IsWindow());
		return TreeView_SelectDropTarget(*this, hItem);
	}

	inline BOOL CTreeView::SelectItem(HTREEITEM hItem) const
	// Selects the specified tree-view item.
	{
		assert(IsWindow());
		return TreeView_SelectItem(*this, hItem);
	}

	inline BOOL CTreeView::SelectSetFirstVisible(HTREEITEM hItem) const
	// Scrolls the tree-view control vertically to ensure that the specified item is visible.
	// If possible, the specified item becomes the first visible item at the top of the control's window.
	{
		assert(IsWindow());
		return TreeView_SelectSetFirstVisible(*this, hItem);
	}

	inline BOOL CTreeView::SortChildren(HTREEITEM hItem, BOOL Recurse) const
	// Sorts the child items of the specified parent item in a tree-view control.
	{
		assert(IsWindow());
		return TreeView_SortChildren( *this, hItem, Recurse );
	}

	inline BOOL CTreeView::SortChildrenCB(TVSORTCB& sort, BOOL Recurse) const
	// Sorts tree-view items using an application-defined callback function that compares the items.
	{
		assert(IsWindow());
		return TreeView_SortChildrenCB( *this, &sort, Recurse );
	}


} // namespace Win32xx

#endif // #ifndef _WIN32XX_TREEVIEW_H_

