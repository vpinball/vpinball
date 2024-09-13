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
// wxx_imagelist.h
//  Declaration of the CImageList class

// The CImageList class manages an image list.
// An image list is a collection of images of the same size, each of which
// can be referred to by its index. Image lists are used to efficiently manage
// large sets of icons or bitmaps. All images in an image list are contained
// in a single, wide bitmap in screen device format. An image list can also
// include a monochrome bitmap that contains masks that are used to draw
// images transparently.


#ifndef _WIN32XX_IMAGELIST_H_
#define _WIN32XX_IMAGELIST_H_


#include "wxx_appcore0.h"
#include "wxx_rect.h"


namespace Win32xx
{
    // Forward declaration
    class CBitmap;


    ///////////////////////////////////////
    // The CImageList class that provides the functionality of image lists.
    // An image list is a collection of images of the same size, each of
    // which can be referred to by its index.
    class CImageList
    {
        friend class CWinApp;

    public:
        //Construction
        CImageList();
        CImageList(HIMAGELIST images);
        CImageList(const CImageList& rhs);
        CImageList& operator=(const CImageList& rhs);
        CImageList& operator=(HIMAGELIST rhs);
        virtual ~CImageList();

        // Initialization
        BOOL Create(int cx, int cy, UINT flags, int initial, int grow);
        BOOL Create(int bitmapID, int cx, int grow, COLORREF mask);
        BOOL Create(LPCTSTR resourceName, int cx, int grow, COLORREF mask);
        BOOL Create(HIMAGELIST images);
        BOOL CreateDisabledImageList(HIMAGELIST normalImages);
        void CreateDragImage(HWND header, int index);
        void CreateDragImage(HWND listView, int item, CPoint& pt);
        void CreateDragImage(HWND treeView, HTREEITEM item);

        // Operations
        int Add(HBITMAP bitmap) const;
        int Add(HBITMAP bitmap, HBITMAP mask) const;
        int Add(HBITMAP bitmap, COLORREF mask) const;
        int Add(HICON icon) const;
        int AddIcon(UINT iconID) const;
        void Attach(HIMAGELIST images);
        BOOL BeginDrag(int image, CPoint hotSpot) const;
        BOOL Copy(int dst, int src, UINT flags /*= ILCF_MOVE*/) const;
        void DeleteImageList();
        HIMAGELIST Detach();
        BOOL DragEnter(HWND lock, CPoint point) const;
        BOOL DragLeave(HWND lock) const;
        BOOL DragMove(CPoint point) const;
        BOOL DragShowNolock(BOOL show) const;
        BOOL Draw(HDC dc, int image, POINT point, UINT style) const;
        BOOL DrawEx(HDC dc, int image, POINT pt, SIZE sz, COLORREF bckgnd, COLORREF frgnd, UINT style) const;
        BOOL DrawIndirect(IMAGELISTDRAWPARAMS* pILDParams) const;
        void EndDrag() const;
        HICON ExtractIcon(int index) const;
        void Read(LPSTREAM pStream);
        BOOL Remove(int image) const;
        BOOL Replace(int image, HBITMAP bitmap, HBITMAP mask) const;
        int Replace(int image, HICON icon) const;
        BOOL Write(LPSTREAM pStream) const;

        // Accessors and mutators
        COLORREF GetBkColor() const;
        HIMAGELIST GetDragImage(POINT* pPoint, POINT* pHotspot) const;
        HIMAGELIST GetHandle() const;
        HICON GetIcon(int image, UINT flags) const;
        CSize GetIconSize() const;
        int GetImageCount() const;
        BOOL GetImageInfo(int image, IMAGEINFO* pImageInfo) const;
        COLORREF SetBkColor(COLORREF bckgnd) const;
        BOOL SetDragCursorImage(int drag, int dxHotspot, int dyHotspot) const;

        // Operators
        operator HIMAGELIST () const;

    private:
        void AddToMap() const;
        void Assign(HIMAGELIST images);
        void Release();
        BOOL RemoveFromMap() const;

        std::shared_ptr<CIml_Data> m_pData;
    };


    ///////////////////////////////////////
    // Definitions for the CImageList class
    //

    inline CImageList::CImageList()
    {
        m_pData = std::make_shared<CIml_Data>();
    }

    inline CImageList::CImageList(HIMAGELIST images)
    {
        m_pData = std::make_shared<CIml_Data>();
        Attach(images);
    }

    // Note: A copy of a CImageList is a clone of the original.
    //       Both objects manipulate the one HIMAGELIST.
    inline CImageList::CImageList(const CImageList& rhs)
    {
        m_pData = rhs.m_pData;
    }

    // Note: A copy of a CImageList is a clone of the original.
    inline CImageList& CImageList::operator=(const CImageList& rhs)
    {
        if (this != &rhs)
        {
            Release();
            m_pData = rhs.m_pData;
        }

        return *this;
    }

    inline CImageList& CImageList::operator=(HIMAGELIST rhs)
    {
        Attach(rhs);
        return *this;
    }

    inline CImageList::~CImageList()
    {
        Release();
    }

    // Store the HIMAGELIST and CImageList pointer in the HIMAGELIST map.
    inline void CImageList::AddToMap() const
    {
        assert(m_pData->images);
        GetApp()->AddCImlData(m_pData->images, m_pData);
    }

    // Adds an image or images to an image list.
    // Refer to ImageList_Add in the Windows API documentation for more information.
    inline int CImageList::Add(HBITMAP bitmap) const
    {
        assert(m_pData);
        assert(m_pData->images);
        return ImageList_Add(m_pData->images, bitmap, 0);
    }

    // Adds an image or images to an image list, generating a mask from the specified bitmap.
    // Refer to ImageList_Add in the Windows API documentation for more information.
    inline int CImageList::Add(HBITMAP bitmap, HBITMAP mask) const
    {
        assert(m_pData);
        assert (m_pData->images);
        return ImageList_Add(m_pData->images, bitmap, mask );
    }

    // Adds an image or images to an image list, using the specified color as the mask.
    // Refer to ImageList_AddMasked in the Windows API documentation for more information.
    inline int CImageList::Add(HBITMAP bitmap, COLORREF mask) const
    {
        assert(m_pData);
        assert (m_pData->images);
        return ImageList_AddMasked(m_pData->images, bitmap, mask);
    }

    // Adds an icon specified by its handle to the image list.
    // Refer to ImageList_ReplaceIcon in the Windows API documentation for more information.
    inline int CImageList::Add(HICON icon) const
    {
        assert(m_pData);
        assert (m_pData->images);

        // Append the icon to the image list
        return ImageList_ReplaceIcon(m_pData->images, -1, icon);
    }

    // Adds an icon specified by its resource ID to the image list.
    // Refer to ImageList_ReplaceIcon in the Windows API documentation for more information.
    inline int CImageList::AddIcon(UINT iconID) const
    {
        HICON icon = GetApp()->LoadIcon(iconID);
        return Add(icon);
    }

    // Attaches an existing ImageList to this CImageList.
    inline void CImageList::Attach(HIMAGELIST images)
    {
        CThreadLock mapLock(GetApp()->m_wndLock);
        assert(m_pData);

        if (images != m_pData->images)
        {
            // Release any existing ImageList.
            if (m_pData->images)
            {
                Release();
                m_pData = std::make_shared<CIml_Data>();
            }

            if (images != nullptr)
            {
                // Add the image list to this CImageList.
                std::shared_ptr<CIml_Data> pCImlData = GetApp()->GetCImlData(images).lock();
                if (pCImlData)
                {
                    m_pData = pCImlData;
                }
                else
                {
                    m_pData->images = images;
                    AddToMap();
                }
            }
        }
    }

    // Attach and own the specfied imagelist.
    inline void CImageList::Assign(HIMAGELIST images)
    {
        CThreadLock mapLock(GetApp()->m_wndLock);
        Attach(images);
        m_pData->isManagedHiml = true;
    }

    // Begins dragging an image.
    // Refer to ImageList_BeginDrag in the Windows API documentation for more information.
    inline BOOL CImageList::BeginDrag(int image, CPoint hotSpot) const
    {
        assert(m_pData);
        assert(m_pData->images);
        return ImageList_BeginDrag(m_pData->images, image, hotSpot.x, hotSpot.y);
    }

    // Copies images within the image list.
    // Possible flags: ILCF_MOVE, CLCF_SWAP.
    // Refer to ImageList_Copy in the Windows API documentation for more information.
    inline BOOL CImageList::Copy(int Dest, int Src, UINT flags /*= ILCF_MOVE*/) const
    {
        assert(m_pData);
        assert(m_pData->images);
        return ImageList_Copy(*this, Dest, *this, Src, flags);
    }

    // Creates a new image list.
    // Possible flag values:
    // ILC_COLOR    Use the default behavior if none of the other ILC_COLOR* flags is specified.
    // ILC_COLOR4   Use a 4-bit (16-color) device-independent bitmap (DIB) section as the bitmap for the image list.
    // ILC_COLOR8   Use an 8-bit DIB section. The colors used for the color table are the same colors as the halftone palette.
    // ILC_COLOR16  Use a 16-bit (32/64k-color) DIB section.
    // ILC_COLOR24  Use a 24-bit DIB section.
    // ILC_COLOR32  Use a 32-bit DIB section.
    // ILC_COLORDDB Use a device-dependent bitmap.
    // ILC_MASK     Use a mask. The image list contains two bitmaps, one of which is a monochrome bitmap used as a mask.
    //              If this value is not included, the image list contains only one bitmap.
    // Refer to ImageList_Create in the Windows API documentation for more information.
    inline BOOL CImageList::Create(int cx, int cy, UINT flags, int initial, int grow)
    {
        assert(m_pData);

        HIMAGELIST images = ImageList_Create(cx, cy, flags, initial, grow);

        if (images == nullptr)
            throw CResourceException(GetApp()->MsgImageList());

        Assign(images);

        return (images != nullptr) ? TRUE : FALSE;
    }

    // Creates a new image list.
    // cx       The width of each image.
    // nGrow    The number of images by which the image list can grow when the system needs to make room for new images.
    // crMask   The color used to generate a mask. Each pixel of this color in the specified bitmap is changed to black,
    //          and the corresponding bit in the mask is set to 1. If this parameter is the CLR_NONE value, no mask is generated.
    // Refer to ImageList_Create in the Windows API documentation for more information.
    inline BOOL CImageList::Create(int bitmapID, int cx, int grow, COLORREF mask)
    {
        assert(m_pData);

        LPCTSTR bitmapName = MAKEINTRESOURCE (bitmapID);
        return Create(bitmapName, cx, grow, mask);
    }

    // Creates a new image list.
    //
    // cx       The width of each image.
    // nGrow    The number of images by which the image list can grow when the system needs to make room for new images.
    // crMask   The color used to generate a mask. Each pixel of this color in the specified bitmap is changed to black,
    //          and the corresponding bit in the mask is set to 1. If this parameter is the CLR_NONE value, no mask is generated.
    // Refer to ImageList_LoadBitmap in the Windows API documentation for more information.
    inline BOOL CImageList::Create(LPCTSTR resourceName, int cx, int grow, COLORREF mask)
    {
        assert(m_pData);

        HIMAGELIST images = ImageList_LoadBitmap(GetApp()->GetResourceHandle(), resourceName, cx, grow, mask);
        if (images == nullptr)
            throw CResourceException(GetApp()->MsgImageList());

        Assign(images);

        return (images != nullptr) ? TRUE : FALSE;
    }

    // Creates a duplicate ImageList
    // Refer to ImageList_Duplicate in the Windows API documentation for more information.
    inline BOOL CImageList::Create(HIMAGELIST images)
    {
        assert(m_pData);

        HIMAGELIST copyImages = ImageList_Duplicate(images);
        if (copyImages == nullptr)
            throw CResourceException(GetApp()->MsgImageList());

        Assign(copyImages);

        return (copyImages != nullptr) ? TRUE : FALSE;
    }

    // Creates a transparent version of an item image within the header control.
    // Refer to Header_CreateDragImage in the Windows API documentation for more information.
    inline void CImageList::CreateDragImage(HWND header, int index)
    {
        assert(::IsWindow(header));
        HIMAGELIST images = Header_CreateDragImage(header, index);

        if (images == nullptr)
            throw CResourceException(GetApp()->MsgGdiImageList());

        Assign(images);
    }

    // Creates a drag image list for the specified item.
    // Refer to ListView_CreateDragImage in the Windows API documentation for more information.
    inline void CImageList::CreateDragImage(HWND listView, int item, CPoint& pt)
    {
        assert(::IsWindow(listView));
        HIMAGELIST images = ListView_CreateDragImage(listView, item, &pt);

        if (images == nullptr)
            throw CResourceException(GetApp()->MsgGdiImageList());

        Assign(images);
    }

    // Creates a dragging bitmap for the specified item in a tree-view control.
    // It also creates an image list for the bitmap and adds the bitmap to the image list.
    // Refer to TreeView_CreateDragImage in the Windows API documentation for more information.
    inline void CImageList::CreateDragImage(HWND treeView, HTREEITEM item)
    {
        assert(::IsWindow(treeView));
        HIMAGELIST images = TreeView_CreateDragImage(treeView, item);

        if (images == nullptr)
            throw CResourceException(GetApp()->MsgGdiImageList());

        Assign(images);
    }

    // Destroys an image list.
    inline void CImageList::DeleteImageList()
    {
        CThreadLock mapLock(GetApp()->m_gdiLock);
        assert(m_pData);

        if (m_pData && m_pData->images != nullptr)
        {
            RemoveFromMap();

            ImageList_Destroy(m_pData->images);
            m_pData->images = nullptr;
            m_pData->isManagedHiml = false;
        }
    }

    // Detaches the HIMAGELIST from all CImageList objects.
    // Note: We rarely need to detach the HIMAGELIST from CImageList.
    // The framework will delete the HIMAGELIST automatically if required.
    inline HIMAGELIST CImageList::Detach()
    {
        CThreadLock mapLock(GetApp()->m_gdiLock);
        assert(m_pData);

        HIMAGELIST images = m_pData->images;
        RemoveFromMap();

        // Nullify all copies of m_pData.
        *m_pData.get() = {};

        // Make a new shared_ptr for this object.
        m_pData = std::make_shared<CIml_Data>();

        return images;
    }

    // Displays the drag image at the specified position within the window.
    // Refer to ImageList_DragEnter in the Windows API documentation for more information.
    inline BOOL CImageList::DragEnter(HWND lock, CPoint point) const
    {
        assert(m_pData->images);
        return ImageList_DragEnter(lock, point.x, point.y);
    }

    // Unlocks the specified window and hides the drag image, allowing the window to be updated.
    // Refer to ImageList_DragLeave in the Windows API documentation for more information.
    inline BOOL CImageList::DragLeave(HWND lock) const
    {
        return ImageList_DragLeave(lock);
    }

    // Moves the image that is being dragged during a drag-and-drop operation.
    // This function is typically called in response to a WM_MOUSEMOVE message.
    // Refer to ImageList_DragMove in the Windows API documentation for more information.
    inline BOOL CImageList::DragMove(CPoint point) const
    {
        return ImageList_DragMove(point.x, point.y);
    }

    // Shows or hides the drag image during a drag operation, without locking the window.
    // Refer to ImageList_DragShowNolock in the Windows API documentation for more information.
    inline BOOL CImageList::DragShowNolock(BOOL show) const
    {
        return ImageList_DragShowNolock(show);
    }

    // Draws an image list item in the specified device context.
    // Refer to ImageList_Draw in the Windows API documentation for more information.
    inline BOOL CImageList::Draw(HDC dc, int image, POINT point, UINT style) const
    {
        assert(m_pData->images);
        return ImageList_Draw(m_pData->images, image, dc , point.x, point.y, style);
    }

    // Draws an image list item in the specified device context. The function uses the specified drawing style
    // and blends the image with the specified color.
    // nStyle can have the following values:
    // ILD_BLEND25, ILD_FOCUS
    //   Draws the image, blending 25 percent with the blend color specified by rgbFg.
    //   This value has no effect if the image list does not contain a mask.
    // ILD_BLEND50, ILD_SELECTED, ILD_BLEND
    //   Draws the image, blending 50 percent with the blend color specified by rgbFg.
    //   This value has no effect if the image list does not contain a mask.
    // ILD_MASK     Draws the mask.
    // ILD_NORMAL   Draws the image using the background color for the image list. If the
    //              background color is the CLR_NONE value, the image is drawn transparently using the mask.
    // ILD_TRANSPARENT  Draws the image transparently using the mask, regardless of the background color.
    //                  This value has no effect if the image list does not contain a mask.
    // ILD_IMAGE    If the overlay does not require a mask to be drawn set the ILD_IMAGE flag.
    //              This causes ImageList_DrawEx to just draw the image, ignoring the mask.
    // Refer to ImageList_Draw in the Windows API documentation for more information.
    inline BOOL CImageList::DrawEx(HDC dc, int image, POINT pt, SIZE sz, COLORREF bckgnd, COLORREF frgnd, UINT style) const
    {
        assert(m_pData->images);
        return ImageList_DrawEx(m_pData->images, image, dc, pt.x, pt.y, sz.cx, sz.cy, bckgnd, frgnd, style);
    }

    // Draws an image list image based on an IMAGELISTDRAWPARAMS structure.
    // Refer to ImageList_DrawIndirect in the Windows API documentation for more information.
    inline BOOL CImageList::DrawIndirect(IMAGELISTDRAWPARAMS* pimldp) const
    {
        assert(m_pData->images);
        return ImageList_DrawIndirect(pimldp);
    }

    // Ends a drag operation.
    // Refer to ImageList_EndDrag in the Windows API documentation for more information.
    inline void CImageList::EndDrag() const
    {
        assert(m_pData->images);
        ImageList_EndDrag();
    }

    // Creates an icon or cursor based on an image and mask in an image list.
    // Refer to ImageList_ExtractIcon in the Windows API documentation for more information.
    inline HICON CImageList::ExtractIcon(int index) const
    {
        assert(m_pData->images);
        return ImageList_ExtractIcon(GetApp()->GetResourceHandle(), *this, index);
    }

    // Retrieves the current background color for an image list.
    // Refer to ImageList_GetBkColor in the Windows API documentation for more information.
    inline COLORREF CImageList::GetBkColor() const
    {
        assert(m_pData->images);
        return ImageList_GetBkColor(*this);
    }

    // Retrieves the temporary image list that is used for the drag image.
    // pHotspot - Pointer to a POINT structure that receives the offset of the
    // drag image relative to the drag position. Can be nullptr.
    // Refer to ImageList_GetDragImage in the Windows API documentation for more information.
    inline HIMAGELIST CImageList::GetDragImage(POINT* pPoint, POINT* pHotspot) const
    {
        assert(m_pData->images);
        return ImageList_GetDragImage(pPoint, pHotspot);
    }

    // Returns the HIMAGELIST assigned to this CImageList.
    inline HIMAGELIST CImageList::GetHandle() const
    {
        return m_pData->images;
    }

    // Creates an icon from an image and mask in an image list.
    // Refer to ImageList_GetIcon in the Windows API documentation for more information.
    inline HICON CImageList::GetIcon(int image, UINT flags) const
    {
        assert(m_pData->images);
        return ImageList_GetIcon(m_pData->images, image, flags);
    }

    // Retrieves the dimensions of images in an image list.
    // Refer to ImageList_GetIconSize in the Windows API documentation for more information.
    inline CSize CImageList::GetIconSize() const
    {
        assert(m_pData->images);
        int cx = 0;
        int cy = 0;
        ImageList_GetIconSize(m_pData->images, &cx, &cy);
        return CSize(cx, cy);
    }

    // Retrieves the number of images in an image list.
    // Refer to ImageList_GetImageCount in the Windows API documentation for more information.
    inline int CImageList::GetImageCount() const
    {
        assert(m_pData->images);
        return ImageList_GetImageCount(m_pData->images);
    }

    // Retrieves information about an image.
    // Refer to ImageList_GetImageInfo in the Windows API documentation for more information.
    inline BOOL CImageList::GetImageInfo(int image, IMAGEINFO* pImageInfo) const
    {
        assert(m_pData->images);
        assert(pImageInfo);
        return ImageList_GetImageInfo(m_pData->images, image, pImageInfo);
    }

    // Reads the image list from a stream.
    // Refer to ImageList_Read in the Windows API documentation for more information.
    inline void CImageList::Read(LPSTREAM pStream)
    {
        HIMAGELIST images =  ImageList_Read(pStream);
        if (images)
        {
            Assign(images);
        }
    }

    // Removes an image from an image list.
    // Refer to ImageList_Remove in the Windows API documentation for more information.
    inline BOOL CImageList::Remove(int image) const
    {
        assert(m_pData->images);
        return ImageList_Remove(m_pData->images, image);
    }

    // Removes the HIMAGELIST and CImageList pointer from the HIMAGELIST map
    inline BOOL CImageList::RemoveFromMap() const
    {
        BOOL success = FALSE;

        if (CWinApp::SetnGetThis() != nullptr)          // Is the CWinApp object still valid?
        {
            CThreadLock mapLock(GetApp()->m_wndLock);

            // Find the CImageList data entry in the map.
            auto it = GetApp()->m_mapCImlData.find(m_pData->images);
            if (it != GetApp()->m_mapCImlData.end())
            {
                // Erase the CImageList data entry from the map
                GetApp()->m_mapCImlData.erase(it);
                success = TRUE;
            }
        }

        return success;
    }

    // Replaces an image in an image list with a new image.
    // Refer to ImageList_Replace in the Windows API documentation for more information.
    inline BOOL CImageList::Replace(int image, HBITMAP bitmap, HBITMAP mask) const
    {
        assert (m_pData->images);
        return ImageList_Replace(m_pData->images, image, bitmap, mask);
    }

    // Replaces an image with an icon or cursor.
    // Refer to ImageList_ReplaceIcon in the Windows API documentation for more information.
    inline int CImageList::Replace(int image, HICON icon) const
    {
        assert (m_pData->images);
        return ImageList_ReplaceIcon(m_pData->images, image, icon);
    }

    // Writes the image list to a stream.
    // Refer to ImageList_Write in the Windows API documentation for more information.
    inline BOOL CImageList::Write(LPSTREAM pStream) const
    {
        assert(m_pData->images);
        return ImageList_Write(*this, pStream);
    }

    // Sets the background color for an image list.This function only works if you add an icon
    // or use ImageList_AddMasked with a black and white bitmap. Without a mask, the entire image
    // is drawn; hence the background color is not visible.
    // Refer to ImageList_SetBkColor in the Windows API documentation for more information.
    inline COLORREF CImageList::SetBkColor(COLORREF bckgnd) const
    {
        assert(m_pData->images);
        return ImageList_SetBkColor(*this, bckgnd);
    }

    // Creates a new drag image by combining the specified image (typically a mouse cursor image)
    // with the current drag image.
    // Refer to ImageList_SetDragCursorImage in the Windows API documentation for more information.
    inline BOOL CImageList::SetDragCursorImage(int drag, int dxHotspot, int dyHotspot) const
    {
        assert(m_pData->images);
        return ImageList_SetDragCursorImage(*this, drag, dxHotspot, dyHotspot);
    }

    // Retrieves the image list's handle.
    inline CImageList::operator HIMAGELIST () const
    {
        return m_pData->images;
    }

    // Decrements the reference count.
    // Destroys m_pData if the reference count is zero.
    inline void CImageList::Release()
    {
        if (CWinApp::SetnGetThis())
            CThreadLock mapLock(GetApp()->m_gdiLock);
        assert(m_pData);

        if (m_pData.use_count() == 1)
        {
            if (m_pData->images != 0)
            {
                if (m_pData->isManagedHiml)
                {
                    ImageList_Destroy(m_pData->images);
                }

                RemoveFromMap();
            }

            m_pData = nullptr;
        }
    }

    // Creates a gray scale image list from the specified color image list.
    inline BOOL CImageList::CreateDisabledImageList(HIMAGELIST normalImages)
    {
        assert(normalImages);

        int count = ImageList_GetImageCount(normalImages);
        if (count > 0)
        {
            int cx, cy;
            ImageList_GetIconSize(normalImages, &cx, &cy);

            // Create the disabled ImageList.
            Create(cx, cy, ILC_COLOR24 | ILC_MASK, count, 0);

            // Process each image in the ImageList.
            for (int i = 0 ; i < count; ++i)
            {
                CClientDC desktopDC(HWND_DESKTOP);
                CMemDC memDC(desktopDC);
                memDC.CreateCompatibleBitmap(desktopDC, cx, cx);
                CRect rc;
                rc.SetRect(0, 0, cx, cx);

                // Set the mask color to gray for the new ImageList
                COLORREF mask = RGB(200, 200, 200);
                if (GetDeviceCaps(desktopDC, BITSPIXEL) < 24)
                {
                    HPALETTE hPal = static_cast<HPALETTE>(GetCurrentObject(desktopDC, OBJ_PAL));
                    UINT index = GetNearestPaletteIndex(hPal, mask);
                    if (index != CLR_INVALID)
                        mask = PALETTEINDEX(index);
                }
                memDC.SolidFill(mask, rc);

                // Draw the image on the memory DC.
                ImageList_Draw(normalImages, i, memDC, 0, 0, ILD_NORMAL);

                // Convert the bitmap to grayscale and add it to the imagelist.
                CBitmap bitmap = memDC.DetachBitmap();
                bitmap.ConvertToDisabled(mask);
                Add(bitmap, mask);
            }
        }

        return (m_pData->images != nullptr) ? TRUE : FALSE;
    }

}   // namespace Win32xx

#endif  // _WIN32XX_IMAGELIST_H_

