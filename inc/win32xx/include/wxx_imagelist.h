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
    // The CImageList class which provides the functionality of image lists.
    // An image list is a collection of images of the same size, each of
    // which can be referred to by its index.
    class CImageList
    {
        friend class CWinApp;

    public:
        //Construction
        CImageList();
        CImageList(HIMAGELIST himl);
        CImageList(const CImageList& rhs);
        CImageList& operator = (const CImageList& rhs);
        void operator = (const HIMAGELIST hImageLIst);
        ~CImageList();

        //Initialization
        BOOL Create(int cx, int cy, UINT nFlags, int nInitial, int nGrow);
        BOOL Create(UINT nBitmapID, int cx, int nGrow, COLORREF crMask);
        BOOL Create(LPCTSTR lpszBitmapID, int cx, int nGrow, COLORREF crMask);
        BOOL Create(HIMAGELIST hImageList);

#ifndef _WIN32_WCE
        BOOL CreateDisabledImageList(HIMAGELIST himlNormal);
#endif

        //Operations
        int Add(HBITMAP hbmImage, HBITMAP hbmMask) const;
        int Add(HBITMAP hbmImage, COLORREF crMask) const;
        int Add(HICON hIcon) const;
        void Attach(HIMAGELIST hImageList);
        BOOL BeginDrag(int nImage, CPoint ptHotSpot) const;
        void DeleteImageList();
        HIMAGELIST Detach();
        BOOL DragEnter(HWND hWndLock, CPoint point) const;
        BOOL DragLeave(HWND hWndLock) const;
        BOOL DragMove(CPoint pt) const;
        BOOL DragShowNolock(BOOL Show) const;
        BOOL Draw(HDC hdc, int nImage, POINT pt, UINT nStyle) const;
        BOOL DrawEx(HDC hdc, int nImage, POINT pt, SIZE sz, COLORREF clrBk, COLORREF clrFg, UINT nStyle) const;
        BOOL DrawIndirect(const IMAGELISTDRAWPARAMS& imldp);
        BOOL Remove(int nImage) const;
        BOOL Replace(int nImage, HBITMAP hbmImage, HBITMAP hbmMask) const;
        int Replace(int nImage, HICON hIcon) const;
        HIMAGELIST GetHandle() const;

        //Attributes
        HICON GetIcon(int iImage, UINT nFlags) const;
        CSize GetIconSize() const;
        int GetImageCount() const;
        BOOL GetImageInfo(int nImage, IMAGEINFO& ImageInfo) const;

        //Operators
        operator HIMAGELIST () const;

    private:
        void AddToMap() const;
        void Release();
        BOOL RemoveFromMap() const;

        CIml_Data* m_pData;
    };


    ///////////////////////////////////////
    // Definitions for the CImageList class
    //

    inline CImageList::CImageList()
    {
        m_pData = new CIml_Data;
    }


    inline CImageList::CImageList(HIMAGELIST himl)
    {
        m_pData = new CIml_Data;
        Attach(himl);
    }


    // Note: A copy of a CImageList is a clone of the original.
    //       Both objects manipulate the one HIMAGELIST.
    inline CImageList::CImageList(const CImageList& rhs)
    {
        m_pData = rhs.m_pData;
        InterlockedIncrement(&m_pData->Count);
    }


    // Note: A copy of a CImageList is a clone of the original.
    inline CImageList& CImageList::operator = (const CImageList& rhs)
    {
        if (this != &rhs)
        {
            InterlockedIncrement(&rhs.m_pData->Count);
            Release();
            m_pData = rhs.m_pData;
        }

        return *this;
    }


    inline void CImageList::operator = (const HIMAGELIST hImageLIst)
    {
        Attach(hImageLIst);
    }


    inline CImageList::~CImageList()
    {
        Release();
    }


    // Store the HIMAGELIST and CImageList pointer in the HIMAGELIST map
    inline void CImageList::AddToMap() const
    {
        assert( &GetApp() );
        assert(m_pData->hImageList);

        GetApp().AddCImlData(m_pData->hImageList, m_pData);
    }


    inline BOOL CImageList::RemoveFromMap() const
    {
        BOOL Success = FALSE;

        if ( &GetApp() )
        {
            // Allocate an iterator for our CImageList data
            std::map<HIMAGELIST, CIml_Data*, CompareHIMAGELIST>::iterator m;

            CWinApp& App = GetApp();
            CThreadLock mapLock(App.m_csMapLock);
            m = App.m_mapCImlData.find(m_pData->hImageList);
            if (m != App.m_mapCImlData.end())
            {
                // Erase the CImageList data entry from the map
                App.m_mapCImlData.erase(m);
                Success = TRUE;
            }

        }

        return Success;
    }


    // Adds an image or images to an image list, generating a mask from the specified bitmap.
    // The hbmMask parameter can be NULL.
    inline int CImageList::Add(HBITMAP hbmImage, HBITMAP hbmMask) const
    {
        assert(m_pData);
        assert (m_pData->hImageList);
        return ImageList_Add(m_pData->hImageList, hbmImage, hbmMask );
    }


    // Adds an image or images to an image list, using the specified color as the mask.
    inline int CImageList::Add(HBITMAP hbmImage, COLORREF crMask) const
    {
        assert(m_pData);
        assert (m_pData->hImageList);
        return ImageList_AddMasked(m_pData->hImageList, hbmImage, crMask);
    }


    // Adds an Icon to the image list
    inline int CImageList::Add(HICON hIcon) const
    {
        assert(m_pData);
        assert (m_pData->hImageList);

        // Append the icon to the image list
        return ImageList_ReplaceIcon(m_pData->hImageList, -1, hIcon);
    }


    // Attaches an existing ImageList to this CImageList
    // hImageList can be NULL
    inline void CImageList::Attach(HIMAGELIST hImageList)
    {
        assert(m_pData);

        if (hImageList != m_pData->hImageList)
        {
            // Release any existing ImageList
            if (m_pData->hImageList)
            {
                Release();
                m_pData = new CIml_Data;
            }

            if (hImageList)
            {
                // Add the image list to this CImageList
                CIml_Data* pCImlData = GetApp().GetCImlData(hImageList);
                if (pCImlData)
                {
                    delete m_pData;
                    m_pData = pCImlData;
                    InterlockedIncrement(&m_pData->Count);
                }
                else
                {
                    m_pData->hImageList = hImageList;
                    AddToMap();
                }
            }
        }
    }


    // Begins dragging an image.
    inline BOOL CImageList::BeginDrag(int nImage, CPoint ptHotSpot) const
    {
        assert(m_pData);
        assert(m_pData->hImageList);
        return ImageList_BeginDrag(m_pData->hImageList, nImage, ptHotSpot.x, ptHotSpot.y);
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
    inline BOOL CImageList::Create(int cx, int cy, UINT nFlags, int nInitial, int nGrow)
    {
        assert(m_pData);

        HIMAGELIST himlNew = ImageList_Create(cx, cy, nFlags, nInitial, nGrow);

        if (himlNew)
        {
        //  m_pData->hImageList = himlNew;
        //  AddToMap();

            Attach(himlNew);
            m_pData->IsManagedHiml = TRUE;
        }

        return ( himlNew != 0 );
    }


    // Creates a new image list.
    // cx       The width of each image.
    // nGrow    The number of images by which the image list can grow when the system needs to make room for new images.
    // crMask   The color used to generate a mask. Each pixel of this color in the specified bitmap is changed to black,
    //          and the corresponding bit in the mask is set to 1. If this parameter is the CLR_NONE value, no mask is generated.
    inline BOOL CImageList::Create(UINT nBitmapID, int cx, int nGrow, COLORREF crMask)
    {
        assert(m_pData);
        assert(NULL == m_pData->hImageList);

        LPCTSTR lpszBitmapID = MAKEINTRESOURCE (nBitmapID);
        return Create(lpszBitmapID, cx, nGrow, crMask);
    }


    // Creates a new image list.
    //
    // cx       The width of each image.
    // nGrow    The number of images by which the image list can grow when the system needs to make room for new images.
    // crMask   The color used to generate a mask. Each pixel of this color in the specified bitmap is changed to black,
    //          and the corresponding bit in the mask is set to 1. If this parameter is the CLR_NONE value, no mask is generated.
    inline BOOL CImageList::Create(LPCTSTR lpszBitmapID, int cx, int nGrow, COLORREF crMask)
    {
        assert(m_pData);
        assert(NULL == m_pData->hImageList);
        HIMAGELIST himlNew = ImageList_LoadBitmap(GetApp().GetInstanceHandle(), lpszBitmapID, cx, nGrow, crMask);

        if (himlNew)
        {
            Attach(himlNew);
            m_pData->IsManagedHiml = TRUE;
        }

        return ( himlNew!= 0 );
    }


    // Creates a duplicate ImageList
    inline BOOL CImageList::Create(HIMAGELIST hImageList)
    {
        assert(m_pData);
        HIMAGELIST himlCopy = ImageList_Duplicate(hImageList);

        if (himlCopy)
        {
            Attach(himlCopy);
            m_pData->IsManagedHiml = TRUE;
        }

        return ( himlCopy!= 0 );
    }


    // Destroys an image list.
    inline void CImageList::DeleteImageList()
    {
        assert(m_pData);
        if (m_pData->hImageList != 0)
        {
            ImageList_Destroy(Detach());
        }
    }


    // Detaches the HIMAGELIST from all CImageList objects.
    inline HIMAGELIST CImageList::Detach()
    {
        assert(m_pData);
        HIMAGELIST hImageList = m_pData->hImageList;
        RemoveFromMap();
        m_pData->hImageList = 0;

        if (m_pData->Count > 0)
        {
            if (InterlockedDecrement(&m_pData->Count) == 0)
            {
                delete m_pData;
            }
        }

        m_pData = new CIml_Data;

        return hImageList;
    }


    // Displays the drag image at the specified position within the window.
    inline BOOL CImageList::DragEnter(HWND hWndLock, CPoint point) const
    {
        assert(m_pData->hImageList);
        return ImageList_DragEnter(hWndLock, point.x, point.y);
    }


    // Unlocks the specified window and hides the drag image, allowing the window to be updated.
    inline BOOL CImageList::DragLeave(HWND hWndLock) const
    {
        return ImageList_DragLeave(hWndLock);
    }


    // Moves the image that is being dragged during a drag-and-drop operation.
    // This function is typically called in response to a WM_MOUSEMOVE message.
    inline BOOL CImageList::DragMove(CPoint pt) const
    {
        return ImageList_DragMove(pt.x, pt.y);
    }


    // Shows or hides the drag image during a drag operation, without locking the window.
    inline BOOL CImageList::DragShowNolock(BOOL Show) const
    {
        return ImageList_DragShowNolock(Show);
    }


    // Draws an image list item in the specified device context.
    inline BOOL CImageList::Draw(HDC hdc, int nImage, POINT pt, UINT nStyle) const
    {
        assert(m_pData->hImageList);
        return ImageList_Draw(m_pData->hImageList, nImage, hdc , pt.x, pt.y, nStyle);
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
    inline BOOL CImageList::DrawEx(HDC hdc, int nImage, POINT pt, SIZE sz, COLORREF clrBk, COLORREF clrFg, UINT nStyle) const
    {
        assert(m_pData->hImageList);
        return ImageList_DrawEx(m_pData->hImageList, nImage, hdc, pt.x, pt.y, sz.cx, sz.cy, clrBk, clrFg, nStyle);
    }


    // Draws an image list image based on an IMAGELISTDRAWPARAMS structure.
    inline BOOL CImageList::DrawIndirect(const IMAGELISTDRAWPARAMS& imldp)
    {
        assert(m_pData->hImageList);
        return ImageList_DrawIndirect((LPIMAGELISTDRAWPARAMS)&imldp);
    }


    // Creates an icon from an image and mask in an image list.
    inline HICON CImageList::GetIcon(int iImage, UINT nFlags) const
    {
        assert(m_pData->hImageList);
        return ImageList_GetIcon(m_pData->hImageList, iImage, nFlags);
    }


    // Retrieves the dimensions of images in an image list.
    inline CSize CImageList::GetIconSize() const
    {
        assert(m_pData->hImageList);
        int cx = 0;
        int cy = 0;
        ImageList_GetIconSize(m_pData->hImageList, &cx, &cy);
        return CSize(cx, cy);
    }


    // Retrieves the number of images in an image list.
    inline int CImageList::GetImageCount() const
    {
        assert(m_pData->hImageList);
        return ImageList_GetImageCount(m_pData->hImageList);
    }


    // Retrieves information about an image.
    inline BOOL CImageList::GetImageInfo(int nImage, IMAGEINFO& ImageInfo) const
    {
        assert(m_pData->hImageList);
        return ImageList_GetImageInfo(m_pData->hImageList, nImage, &ImageInfo);
    }


    // Removes an image from an image list.
    inline BOOL CImageList::Remove(int nImage) const
    {
        assert(m_pData->hImageList);
        return ImageList_Remove(m_pData->hImageList, nImage);
    }


    // Replaces an image in an image list with a new image.
    inline BOOL CImageList::Replace(int nImage, HBITMAP hbmImage, HBITMAP hbmMask) const
    {
        assert (m_pData->hImageList);
        return ImageList_Replace(m_pData->hImageList, nImage, hbmImage, hbmMask);
    }


    // Replaces an image with an icon or cursor.
    inline int CImageList::Replace(int nImage, HICON hIcon) const
    {
        assert (m_pData->hImageList);
        return ImageList_ReplaceIcon(m_pData->hImageList, nImage, hIcon);
    }


    // Returns the HIMAGELIST assigned to this CImageList.
    inline HIMAGELIST CImageList::GetHandle() const
    {
        return m_pData->hImageList;
    }


    // Retrieves the image list's handle.
    inline CImageList::operator HIMAGELIST () const

    {
        return m_pData->hImageList;
    }


    inline void CImageList::Release()
    {
        assert(m_pData);

        if (InterlockedDecrement(&m_pData->Count) == 0)
        {
            if (m_pData->hImageList != NULL)
            {
                if (m_pData->IsManagedHiml)
                {
                    ImageList_Destroy(m_pData->hImageList);
                }

                RemoveFromMap();
            }

            delete m_pData;
            m_pData = 0;
        }
    }


#ifndef _WIN32_WCE

    // Creates a gray scale image list from the specified color image list.
    inline BOOL CImageList::CreateDisabledImageList(HIMAGELIST himlNormal)
    {
        assert(NULL == m_pData->hImageList);
        assert(himlNormal);

        int nCount = ImageList_GetImageCount(himlNormal);
        if (nCount > 0)
        {
            int cx, cy;
            ImageList_GetIconSize(himlNormal, &cx, &cy);

            // Create the disabled ImageList
            Create(cx, cy, ILC_COLOR24 | ILC_MASK, nCount, 0);

            // Process each image in the ImageList
            for (int i = 0 ; i < nCount; ++i)
            {
                CClientDC DesktopDC(NULL);
                CMemDC dcMem(NULL);
                dcMem.CreateCompatibleBitmap(DesktopDC, cx, cx);
                CRect rc;
                rc.SetRect(0, 0, cx, cx);

                // Set the mask color to grey for the new ImageList
                COLORREF crMask = RGB(200, 199, 200);
                if ( GetDeviceCaps(DesktopDC, BITSPIXEL) < 24)
                {
                    HPALETTE hPal = reinterpret_cast<HPALETTE>(GetCurrentObject(DesktopDC, OBJ_PAL));
                    UINT Index = GetNearestPaletteIndex(hPal, crMask);
                    if (Index != CLR_INVALID) crMask = PALETTEINDEX(Index);
                }

                dcMem.SolidFill(crMask, rc);

                // Draw the image on the memory DC
                ImageList_Draw(himlNormal, i, dcMem , 0, 0, ILD_NORMAL);

                // Convert colored pixels to gray
                for (int x = 0 ; x < cx; ++x)
                {
                    for (int y = 0; y < cy; ++y)
                    {
                        COLORREF clr = ::GetPixel(dcMem, x, y);

                        if (clr != crMask)
                        {
                            BYTE byGray = (BYTE) (95 + (GetRValue(clr) *3 + GetGValue(clr)*6 + GetBValue(clr))/20);
                            dcMem.SetPixel(x, y, RGB(byGray, byGray, byGray));
                        }
                    }
                }

                // Detach the bitmap so we can use it.
                CBitmap Bitmap = dcMem.DetachBitmap();
                Add(Bitmap, crMask);
            }
        }

        return ( m_pData->hImageList != 0 );
    }

#endif

}   // namespace Win32xx

#endif  // _WIN32XX_IMAGELIST_H_

