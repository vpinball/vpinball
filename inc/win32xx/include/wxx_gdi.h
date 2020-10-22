// Win32++   Version 8.8
// Release Date: 15th October 2020
//
//      David Nash
//      email: dnash@bigpond.net.au
//      url: https://sourceforge.net/projects/win32-framework
//
//
// Copyright (c) 2005-2020  David Nash
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
// wxx_gdi.h
//  Declaration of the CDC class, and CBitmapInfoPtr class

// The CDC class provides a device context, along with the various associated
//  objects such as Bitmaps, Brushes, Bitmaps, Fonts and Pens. This class
//  handles the creation, selection, de-selection and deletion of these objects
//  automatically. It also automatically deletes or releases the device context
//  itself as appropriate. Any failure to create the new GDI object throws an
//  exception.
//
// The CDC class is sufficient for most GDI programming needs. Sometimes
//  however we need to have the GDI object separated from the device context.
//  Wrapper classes for GDI objects are provided for this purpose. The classes
//  are CBitmap, CBrush, CFont, CPalette, CPen and CRgn. These classes
//  automatically delete any GDI resources they create when their destructor
//  is called. These wrapper class objects can be attached to the CDC as
//  shown below.
//
// Coding Example without CDC ...
//  void DrawLine()
//  {
//    HDC clientDC = ::GetDC(GetHwnd());
//    HDC memDC = ::CreateCompatibleDC(clientDC);
//    HBITMAP bitmap = ::CreateCompatibleBitmap(clientDC, cx, cy);
//    HBITMAP oldBitmap = reinterpret_cast<HBITMAP>(::SelectObject(memDC, bitmap));
//    HPEN pen1 = ::CreatePen(PS_SOLID, 1, RGB(255,0,0);
//    HPEN oldPen = reinterpret_cast<HPEN>(::SelectObject(memDC, pen1));
//    ::MoveToEx(memDC, 0, 0, NULL);
//    ::LineTo(memDC, 50, 50);
//
//    // Select a new pen into memDC
//    ::SelectObject(memDC, oldPen);
//    ::DeleteObject(pen1);
//    HPEN pen2 = ::CreatePen(PS_SOLID, 1, RGB(0,255,0);
//    oldPen = reinterpret_cast<HPEN>(::SelectObject(memDC, pen2));
//
//    ::LineTo(memDC, 80, 80);
//    ::BitBlt(clientDC, 0, 0, cx, cy, memDC, 0, 0);
//
//    // Cleanup
//    ::SelectObject(memDC, oldPen);
//    ::DeleteObject(pen2);
//    ::SelectObject(memDC, oldBitmap);
//    ::DeleteObject(bitmap);
//    ::DeleteDC(memDC);
//    ::ReleaseDC(GetHwnd(), clientDC);
//  }
//
// Coding Example with CDC classes ...
//  void DrawLine()
//  {
//    CClientDC clientDC(*this)
//    CMemDC memDC(clientDC);
//    memDC.CreateCompatibleBitmap(clientDC, cx, cy);
//    memDC.CreatePen(PS_SOLID, 1, RGB(255,0,0);
//    memDC.MoveTo(0, 0);
//    memDC.LineTo(50, 50);
//    memDC.CreatePen(PS_SOLID, 1, RGB(0,255,0));
//    memDC.LineTo(80,80);
//    clientDC.BitBlt(0, 0, cx, cy, memDC, 0, 0);
//  }
//
// Coding Example with CDC classes and CPen ...
//  void DrawLine()
//  {
//    CClientDC clientDC(*this)
//    CMemDC memDC(clientDC);
//    memDC.CreateCompatibleBitmap(clientDC, cx, cy);
//    CPen pen1(PS_SOLID, 1, RGB(255,0,0));
//    CPen oldPen = memDC.SelectObject(pen1);
//    memDC.MoveTo(0, 0);
//    memDC.LineTo(50, 50);
//
//    // Select a new pen into memDC
//    CPen pen2(PS_SOLID, 1, RGB(0,255,0));
//    memDC.SelectObject(pen2);
//
//    memDC.LineTo(80,80);
//    clientDC.BitBlt(0, 0, cx, cy, memDC, 0, 0);
//
//    // Cleanup. Remove the pen from the DC before the pen is destroyed.
//    memDC.SelectObject(oldPen);
//  }

// Notes:
//  * When the CDC object drops out of scope, its destructor is called, releasing
//     or deleting the device context if Win32++ created it.
//  * When the destructor for CBitmap, CBrush, CPalette, CPen and CRgn are called,
//     the destructor is called deleting their GDI object if Win32++ created it.
//  * When the CDC object's destructor is called, any GDI objects created by one of
//     the CDC member functions (CDC::CreatePen for example) will be deleted.
//  * Bitmaps can only be selected into one device context at a time.
//  * Other GDI resources can be selected into more than one device context at a time.
//  * Palettes use SelectPalatte to select them into device the context.
//  * Clipping regions use SelectClipRgn to select them into the device context.
//  * Other regions use SelectObject to select them into the device context.
//  * All the GDI classes are reference counted. This allows functions to safely
//     pass these objects by value, as well as by pointer or by reference.
//  * If SelectObject is used to select say a CPen into a device context, the
//     CPen shouldn't be changed or destroyed while device context is valid. Use
//     SelectObject to select the old pen back into the device context before
//     changing the pen.
//  * All GDI classes are reference counted and can be copied safely. This means they
//     can be safely returned by value from functions. The associated GDI resource is
//     only deleted (if appropriate) when the last copy of the object goes out of scope.
//  * A copy of a GDI class is a clone of the original. Both class objects manipulate
//     the one GDI resource.


// The CBitmapInfoPtr class is a convenient wrapper for the BITMAPINFO structure.
// The size of the BITMAPINFO structure is dependant on the type of HBITMAP, and its
// space needs to be allocated dynamically. CBitmapInfoPtr automatically allocates
// and deallocates the memory for the structure. A CBitmapInfoPtr object can be
// used anywhere in place of a LPBITMAPINFO. LPBITMAPINFO is used in functions like
// GetDIBits and SetDIBits.
//
// Coding example ...
//  CDC memDC = CreateCompatibleDC(NULL);
//  CBitmapInfoPtr pbmi(bitmap);
//  memDC.GetDIBits(bitmap, 0, pbmi->bmiHeader.biHeight, NULL, pbmi, DIB_RGB_COLORS);


#ifndef _WIN32XX_GDI_H_
#define _WIN32XX_GDI_H_


#include "wxx_appcore0.h"
#include "wxx_wincore0.h"
#include "wxx_exception.h"
#include "wxx_metafile.h"

#ifndef HGDI_ERROR
  #define HGDI_ERROR ((HANDLE)-1)
#endif

// Disable macros from Windowsx.h
#undef CopyRgn

namespace Win32xx
{

    /////////////////////////////////////////////////////////////////
    // CObject provides the functionality of a HGDIOBJECT
    // This is the base class for other classes managing GDI objects,
    // such as bitmaps, brushes, palettes, fonts, pens and regions.
    class CGDIObject
    {
    public:
        CGDIObject();
        CGDIObject(const CGDIObject& rhs);
        virtual ~CGDIObject();
        CGDIObject& operator = (const CGDIObject& rhs);
        void operator = (const HGDIOBJ object);

        void    Attach(HGDIOBJ object);
        void    DeleteObject();
        HGDIOBJ Detach();
        HGDIOBJ GetHandle() const;
        int     GetObject(int count, LPVOID pObject) const;

    protected:
        void    Release();
        void    SetManaged(bool isManaged) const { m_pData->isManagedObject = isManaged; }

    private:
        void    AddToMap();
        BOOL    RemoveFromMap();

        CGDI_Data* m_pData;
    };


    ///////////////////////////////////////
    // CBitmap manages a bitmap GDI object.
    class CBitmap : public CGDIObject
    {

      public:
        CBitmap();
        CBitmap(HBITMAP bitmap);
        CBitmap(LPCTSTR pResName);
        CBitmap(int resourceID);
        operator HBITMAP() const;
        virtual ~CBitmap();

        // Create and load methods
        BOOL LoadBitmap(LPCTSTR pResName);
        BOOL LoadBitmap(int resID);
        BOOL LoadImage(LPCTSTR pResName, UINT flags);
        BOOL LoadImage(UINT resID, UINT flags);
        BOOL LoadOEMBitmap(UINT bitmapID);
        HBITMAP CreateBitmap(int width, int height, UINT planes, UINT bitsPerPixel, LPCVOID pBits);
        HBITMAP CreateCompatibleBitmap(HDC dc, int width, int height);
        HBITMAP CreateDIBSection(HDC dc, const LPBITMAPINFO pBMI, UINT colorUse, LPVOID* ppBits, HANDLE section, DWORD offset);

#ifndef _WIN32_WCE
        void    ConvertToDisabled(COLORREF mask) const;
        HBITMAP CreateDIBitmap(HDC dc, const BITMAPINFOHEADER* pBMIH, DWORD init, LPCVOID pInit, const LPBITMAPINFO pBMI, UINT colorUse);
        HBITMAP CreateMappedBitmap(UINT bitmapID, UINT flags = 0, LPCOLORMAP pColorMap = NULL, int mapSize = 0);
        HBITMAP CreateBitmapIndirect(const BITMAP& bitmap);
        void GrayScaleBitmap();
        void TintBitmap (int red, int green, int blue);
        int GetDIBits(HDC dc, UINT startScan, UINT scanLines,  LPVOID pBits, LPBITMAPINFO pBMI, UINT colorUse) const;
        int SetDIBits(HDC dc, UINT startScan, UINT scanLines, LPCVOID pBits, const LPBITMAPINFO pBMI, UINT colorUse) const;
        CSize GetBitmapDimensionEx() const;
        CSize SetBitmapDimensionEx(int width, int height) const;
#endif // !_WIN32_WCE

        // Attributes
        BITMAP GetBitmapData() const;
    };


    /////////////////////////////////////
    // CBrush manages a brush GDI object.
    class CBrush : public CGDIObject
    {
      public:
        CBrush();
        CBrush(HBRUSH brush);
        CBrush(COLORREF color);
        operator HBRUSH() const;
        virtual ~CBrush();

        HBRUSH CreateSolidBrush(COLORREF color);
        HBRUSH CreatePatternBrush(HBITMAP bitmap);
        LOGBRUSH GetLogBrush() const;

#ifndef _WIN32_WCE
        HBRUSH CreateHatchBrush(int index, COLORREF color);
        HBRUSH CreateBrushIndirect(const LOGBRUSH& logBrush);
        HBRUSH CreateDIBPatternBrush(HGLOBAL hDIBPacked, UINT colorSpec);
        HBRUSH CreateDIBPatternBrushPt(LPCVOID pPackedDIB, UINT usage);
#endif // !defined(_WIN32_WCE)

    };


    ///////////////////////////////////
    // CFont manages a font GDI object.
    class CFont : public CGDIObject
    {
    public:
        CFont();
        CFont(HFONT font);
        CFont(const LOGFONT& logFont);
        operator HFONT() const;
        virtual ~CFont();

        // Create methods
        HFONT CreateFontIndirect(const LOGFONT& logFont);
        HFONT CreatePointFont(int pointSize, LPCTSTR faceName, HDC dc = NULL, BOOL isBold = FALSE, BOOL isItalic = FALSE);
        HFONT CreatePointFontIndirect(const LOGFONT& logFont, HDC dc = NULL);

#ifndef _WIN32_WCE
        HFONT CreateFont(int height, int width, int escapement,
                int orientation, int weight, DWORD italic, DWORD underline,
                DWORD strikeOut, DWORD charSet, DWORD outPrecision,
                DWORD clipPrecision, DWORD quality, DWORD pitchAndFamily,
                LPCTSTR faceName);
#endif // #ifndef _WIN32_WCE

        // Attributes
        LOGFONT GetLogFont() const;
    };


    /////////////////////////////////////////
    // CPalette manages a palette GDI object.
    class CPalette : public CGDIObject
    {
      public:
        CPalette();
        CPalette(HPALETTE palette);
        operator HPALETTE() const;
        virtual ~CPalette();

        // Create methods
        HPALETTE CreatePalette(LPLOGPALETTE pLogPalette);

#ifndef _WIN32_WCE
        HPALETTE CreateHalftonePalette(HDC dc);
#endif // !_WIN32_WCE

        // Attributes
        int GetEntryCount() const;
        UINT GetPaletteEntries(UINT startIndex, UINT entries, LPPALETTEENTRY pPaletteColors) const;
        UINT SetPaletteEntries(UINT startIndex, UINT entries, LPPALETTEENTRY pPaletteColors) const;

        // Operations
#ifndef _WIN32_WCE
        BOOL ResizePalette(UINT entries) const;
        void AnimatePalette(UINT startIndex, UINT entries, LPPALETTEENTRY pPaletteColors) const;
#endif // !_WIN32_WCE

        UINT GetNearestPaletteIndex (COLORREF color) const;

    };


    /////////////////////////////////
    // CPen manages a pen GDI object.
    class CPen : public CGDIObject
    {
    public:
        CPen();
        CPen(HPEN pen);
        CPen(int penStyle, int width, COLORREF color);
#ifndef _WIN32_WCE
        CPen(int penStyle, int width, const LOGBRUSH& logBrush, int styleCount = 0, const DWORD* pStyle = NULL);
#endif // !_WIN32_WCE
        operator HPEN() const;
        virtual ~CPen();

        HPEN CreatePen(int penStyle, int width, COLORREF color);
        HPEN CreatePenIndirect(const LOGPEN& logPen);
        LOGPEN GetLogPen() const;

#ifndef _WIN32_WCE
        HPEN ExtCreatePen(int penStyle, int width, const LOGBRUSH& logBrush, int styleCount = 0, const DWORD* pStyle = NULL) ;
        EXTLOGPEN GetExtLogPen() const;
#endif // !_WIN32_WCE

    };


    ////////////////////////////////////
    // CRgn manages a region GDI object.
    class CRgn : public CGDIObject
    {
      public:
        CRgn();
        CRgn(HRGN rgn);
        operator HRGN() const;
        virtual ~CRgn ();

        // Create methods
        HRGN CreateRectRgn(int x1, int y1, int x2, int y2);
        HRGN CreateRectRgnIndirect(const RECT& rc);
        HRGN CreateFromData(const XFORM* pXForm, int count, const RGNDATA* pRgnData);

#ifndef _WIN32_WCE
        HRGN CreateEllipticRgn(int x1, int y1, int x2, int y2);
        HRGN CreateEllipticRgnIndirect(const RECT& rc);
        HRGN CreatePolygonRgn(LPPOINT pPoints, int count, int mode);
        HRGN CreatePolyPolygonRgn(LPPOINT pPoints, LPINT pPolyCounts, int count, int polyFillMode);
        HRGN CreateRoundRectRgn(int x1, int y1, int x2, int y2, int x3, int y3);
        HRGN CreateFromPath(HDC dc);
#endif // !_WIN32_WCE

        // Operations
        void SetRectRgn(int x1, int y1, int x2, int y2) const;
        void SetRectRgn(const RECT& rc) const;
        int CombineRgn(HRGN src1, HRGN src2, int combineMode) const;
        int CombineRgn(HRGN src, int combineMode) const;
        int CopyRgn(HRGN src) const;
        BOOL EqualRgn(HRGN rgn) const;
        int OffsetRgn(int x, int y) const;
        int OffsetRgn(POINT& pt) const;
        int GetRgnBox(RECT& rc) const;
        BOOL PtInRegion(int x, int y) const;
        BOOL PtInRegion(POINT& pt) const;
        BOOL RectInRegion(const RECT& rc) const;
        int GetRegionData(LPRGNDATA pRgnData, int dataSize) const;
    };


    // A structure that contains the data members for CDC.
    struct CDC_Data
    {
        // Constructor
        CDC_Data() : dc(0), count(1L), isManagedHDC(FALSE), wnd(0), savedDCState(0) {}

        CBitmap bitmap;
        CBrush  brush;
        CFont   font;
        CPalette palette;
        CPen    pen;
        CRgn    rgn;
        HDC     dc;             // The HDC belonging to this CDC
        long    count;          // Reference count
        bool    isManagedHDC;   // Delete/Release the HDC on destruction
        HWND    wnd;            // The HWND of a Window or Client window DC
        int     savedDCState;   // The save state of the HDC.
    };


    /////////////////////////////////////////////////////////////////////////
    // CDC manages a GDI device context. A device context is a structure
    // that defines a set of graphic objects and their associated attributes,
    // as well as the graphic modes that affect output. The graphic objects
    // include a pen for line drawing, a brush for painting and filling, a
    // bitmap for copying or scrolling parts of the screen, a palette for
    // defining the set of available colors, a region for clipping and other
    // operations, and a path for painting and drawing operations.
    class CDC
    {
        friend class CWinApp;
        friend class CWnd;

    public:
        CDC();                                  // Constructs a new CDC without assigning a HDC
        CDC(HDC dc, HWND wnd = 0);              // Constructs a new CDC and assigns a HDC
        CDC(const CDC& rhs);                    // Constructs a new copy of the CDC
        virtual ~CDC();
        operator HDC() const { return m_pData->dc; }   // Converts a CDC to a HDC
        CDC& operator = (const CDC& rhs);       // Assigns a CDC to an existing CDC

        void Attach(HDC dc, HWND wnd = 0);
        void Destroy();
        HDC  Detach();
        HDC GetHDC() const { return m_pData->dc; }
        BOOL RestoreDC(int savedDC) const;
        int SaveDC() const;
        HGDIOBJ SelectObject(HGDIOBJ object) const;
        HBITMAP SelectObject(HBITMAP bitmap) const;
        HBRUSH SelectObject(HBRUSH brush) const;
        HFONT SelectObject(HFONT font) const;
        HPEN SelectObject(HPEN pen) const;
        int SelectObject(HRGN rgn) const;

#ifndef _WIN32_WCE
        void operator = (const HDC dc);
#endif

        // Initialization
        HDC CreateCompatibleDC(HDC source);
        HDC CreateDC(LPCTSTR driver, LPCTSTR device, LPCTSTR output, const DEVMODE* pInitData);
        int GetDeviceCaps(int index) const;
#ifndef _WIN32_WCE
        HDC CreateIC(LPCTSTR driver, LPCTSTR device, LPCTSTR output, const DEVMODE* pInitData);
#endif

        // Create Bitmaps
        void CreateBitmap(int cx, int cy, UINT planes, UINT bitsPerPixel, LPCVOID pColors);
        void CreateCompatibleBitmap(HDC dc, int cx, int cy);
        void CreateDIBSection(HDC dc, const LPBITMAPINFO pBMI, UINT usage, LPVOID* ppBits,
                                        HANDLE section, DWORD offset);
        CBitmap DetachBitmap();

        BOOL LoadBitmap(UINT id);
        BOOL LoadBitmap(LPCTSTR resName);
        BOOL LoadImage(UINT id, UINT flags);
        BOOL LoadImage(LPCTSTR resName, UINT flags);
        BOOL LoadOEMBitmap(UINT bitmapID); // for OBM_/OCR_/OIC

#ifndef _WIN32_WCE
        void CreateBitmapIndirect(const BITMAP& bitmap);
        void CreateDIBitmap(HDC dc, const BITMAPINFOHEADER& pBMIH, DWORD init, LPCVOID pInit,
                                        const LPBITMAPINFO pBMI, UINT usage);
        void CreateMappedBitmap(UINT bitmapID, UINT flags /*= 0*/, LPCOLORMAP pColorMap /*= NULL*/, int mapSize /*= 0*/);
#endif

        // Create Brushes
        void CreatePatternBrush(HBITMAP bitmap);
        void CreateSolidBrush(COLORREF color);

#ifndef _WIN32_WCE
        void CreateBrushIndirect(const LOGBRUSH& logBrush);
        void CreateHatchBrush(int fnStyle, COLORREF color);
        void CreateDIBPatternBrush(HGLOBAL hDIBPacked, UINT colorSpec);
        void CreateDIBPatternBrushPt(LPCVOID pPackedDIB, UINT usage);
#endif

        // Create Fonts
        void CreateFontIndirect(const LOGFONT& lf);
        void CreatePointFont(int pointSize, LPCTSTR faceName, HDC dc = NULL, BOOL isBold = FALSE, BOOL isItalic = FALSE);
        void CreatePointFontIndirect(const LOGFONT& logFont, HDC dc = NULL);

#ifndef _WIN32_WCE
        void CreateFont(int height, int width, int escapement, int orientation, int weight,
                            DWORD italic, DWORD underline, DWORD strikeOut, DWORD charSet,
                            DWORD outputPrecision, DWORD clipPrecision, DWORD quality,
                            DWORD pitchAndFamily, LPCTSTR faceName);
#endif

        // Create and select Palettes
        void CreatePalette(LPLOGPALETTE pLogPalette, BOOL forceBkgnd);
        HPALETTE SelectPalette(HPALETTE palette, BOOL forceBkgnd);

#ifndef _WIN32_WCE
        void CreateHalftonePalette(BOOL forceBkgnd);
#endif

        // Create Pens
        void CreatePen(int style, int width, COLORREF color);
        void CreatePenIndirect(const LOGPEN& logPen);

#ifndef _WIN32_WCE
        void ExtCreatePen(int penStyle, int width, const LOGBRUSH& logBrush, int styleCount, const DWORD* pStyle);
#endif

        // Retrieve and Select Stock Objects
        HGDIOBJ GetStockObject(int index) const;
        HGDIOBJ SelectStockObject(int index) const;

        // Create Regions
        int CreateRectRgn(int left, int top, int right, int bottom);
        int CreateRectRgnIndirect(const RECT& rc);

        int CreateRgnFromData(const XFORM* pXform, DWORD count, const RGNDATA* pRgnData);
        int CreateRgnFromPath(HDC dc);

#ifndef _WIN32_WCE
        int CreateEllipticRgn(int left, int top, int right, int bottom);
        int CreateEllipticRgnIndirect(const RECT& rc);
        int CreatePolygonRgn(LPPOINT pPointArray, int points, int polyFillMode);
        int CreatePolyPolygonRgn(LPPOINT pPointArray, LPINT pPolyCounts, int count, int polyFillMode);
        int CreateRoundRectRgn(int x1, int y1, int x2, int y2, int x3, int y3);
#endif

        // Wrappers for WinAPI functions
#ifndef _WIN32_WCE
        int EnumObjects(int objectType, GOBJENUMPROC pObjectFunc, LPARAM lparam) const;
#endif

        // Point and Line Drawing Functions
        CPoint GetCurrentPosition() const;
        CPoint MoveTo(int x, int y) const;
        CPoint MoveTo(POINT pt) const;
        BOOL LineTo(int x, int y) const;
        BOOL LineTo(POINT pt) const;
        COLORREF GetPixel(int x, int y) const;
        COLORREF GetPixel(POINT pt) const;
        COLORREF SetPixel(int x, int y, COLORREF color) const;
        COLORREF SetPixel(POINT pt, COLORREF color) const;
        int SetROP2(int drawMode) const;

#ifndef _WIN32_WCE
        BOOL Arc(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4) const;
        BOOL Arc(const RECT& rc, POINT start, POINT end) const;
        BOOL ArcTo(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4) const;
        BOOL ArcTo(const RECT& rc, POINT start, POINT end) const;
        BOOL AngleArc(int x, int y, int radius, float startAngle, float sweepAngle) const;
        BOOL CloseFigure() const;
        int  GetROP2() const;
        int  GetArcDirection() const;
        int  SetArcDirection(int arcDirection) const;
        BOOL PolyDraw(const POINT* pPointArray, const BYTE* pTypes, int count) const;
        BOOL Polyline(LPPOINT pPointArray, int count) const;
        BOOL PolyPolyline(const POINT* pPointArray, const DWORD* pPolyPoints, int count) const;
        BOOL PolylineTo(const POINT* pPointArray, int count) const;
        BOOL PolyBezier(const POINT* pPointArray, int count) const;
        BOOL PolyBezierTo(const POINT* pPointArray, int count) const;
        BOOL SetPixelV(int x, int y, COLORREF color) const;
        BOOL SetPixelV(POINT pt, COLORREF color) const;
#endif

        // Shape Drawing Functions
        void DrawFocusRect(const RECT& rc) const;
        BOOL Ellipse(int x1, int y1, int x2, int y2) const;
        BOOL Ellipse(const RECT& rc) const;
        BOOL Polygon(LPPOINT pPointArray, int count) const;
        BOOL Rectangle(int x1, int y1, int x2, int y2) const;
        BOOL Rectangle(const RECT& rc) const;
        BOOL RoundRect(int x1, int y1, int x2, int y2, int width, int height) const;
        BOOL RoundRect(const RECT& rc, int width, int height) const;

#ifndef _WIN32_WCE
        BOOL Chord(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4) const;
        BOOL Chord(const RECT& rc, POINT start, POINT end) const;
        BOOL Pie(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4) const;
        BOOL Pie(const RECT& rc, POINT start, POINT end) const;
        BOOL PolyPolygon(LPPOINT pPointArray, LPINT pPolyCounts, int count) const;
#endif

        // Fill and Image Drawing functions
        BOOL DrawEdge(const RECT& rc, UINT edge, UINT flags) const;
        BOOL DrawIconEx(int xLeft, int yTop, HICON icon, int cxWidth, int cyWidth,
                        UINT index, HBRUSH flickerFreeDraw, UINT flags) const;

        BOOL DrawFrameControl(const RECT& rc, UINT type, UINT state) const;
        BOOL FillRect(const RECT& rc, HBRUSH brush) const;
        BOOL FillRgn(HRGN rgn, HBRUSH brush) const;
        void GradientFill(COLORREF color1, COLORREF color2, const RECT& rc, BOOL isVertical) const;
        BOOL InvertRect(const RECT& rc) const;
        void SolidFill(COLORREF color, const RECT& rc) const;

#if (WINVER >= 0x0410)
        BOOL GradientFill(PTRIVERTEX pVertex, ULONG vertex, PVOID pMesh, ULONG mesh, ULONG mode) const;
#endif

#ifndef _WIN32_WCE
        BOOL DrawIcon(int x, int y, HICON icon) const;
        BOOL DrawIcon(POINT point, HICON icon) const;
        BOOL FrameRect(const RECT& rc, HBRUSH brush) const;
        BOOL FrameRgn(HRGN rgn, HBRUSH brush, int width, int height) const;
        int  GetPolyFillMode() const;
        BOOL PaintRgn(HRGN rgn) const;
        int  SetPolyFillMode(int polyFillMode) const;
#endif

        // Bitmap Functions
        BOOL BitBlt(int x, int y, int width, int height, HDC hSrc, int xSrc, int ySrc, DWORD rop) const;
        void DrawBitmap(int x, int y, int cx, int cy, HBITMAP image, COLORREF mask) const;
        BITMAP  GetBitmapData() const;
        HBITMAP GetCurrentBitmap() const;
        BOOL MaskBlt(int xDest, int yDest, int width, int height, HDC hSrc,
                           int xSrc, int ySrc, HBITMAP mask, int xMask, int yMask,
                           DWORD rop) const;

        BOOL PatBlt(int x, int y, int width, int height, DWORD rop) const;
        int  StretchDIBits(int xDest, int yDest, int destWidth, int destHeight,
                           int xSrc, int ySrc, int srcWidth, int srcHeight,
                           LPCVOID pBits, const LPBITMAPINFO pBMI, UINT usage, DWORD rop) const;

        BOOL StretchBlt(int x, int y, int width, int height, HDC src,
                           int xSrc, int ySrc, int srcWidth, int srcHeight,
                           DWORD rop) const;

#ifndef _WIN32_WCE
        BOOL ExtFloodFill(int x, int y, COLORREF color, UINT fillType) const;
        BOOL FloodFill(int x, int y, COLORREF color) const;
        int  GetDIBits(HBITMAP bitmap, UINT startScan, UINT scanLines, LPVOID pBits,
                        LPBITMAPINFO pBMI, UINT usage) const;

        int  GetStretchBltMode() const;
        int  SetDIBits(HBITMAP bitmap, UINT startScan, UINT scanLines, LPCVOID pBits,
                        LPBITMAPINFO pBMI, UINT colorUse) const;

        int  SetStretchBltMode(int stretchMode) const;
  #if (WINVER >= 0x0410)
        BOOL TransparentBlt(int x, int y, int width, int height, HDC hSrc,
                           int xSrc, int ySrc, int widthSrc, int heightSrc,
                           UINT transparent) const;
  #endif
#endif

        // Brush Functions
        HBRUSH GetCurrentBrush() const;
        LOGBRUSH GetLogBrush() const;

#ifndef _WIN32_WCE
        CPoint   GetBrushOrgEx() const;
        CPoint   SetBrushOrgEx(int x, int y);
#endif

#if (_WIN32_WINNT >= 0x0500)
        COLORREF GetDCBrushColor() const;
        COLORREF SetDCBrushColor(COLORREF color) const;
#endif

        // Font Functions
        HFONT GetCurrentFont() const;
        LOGFONT GetLogFont() const;

#ifndef _WIN32_WCE
        DWORD GetFontData(DWORD table, DWORD offset, LPVOID pBuffer,  DWORD data) const;
        DWORD GetFontLanguageInfo() const;
        DWORD GetGlyphOutline(UINT query, UINT format, LPGLYPHMETRICS pGM, DWORD bufferSize,
                              LPVOID pBuffer, const MAT2* pMAT2) const;

        DWORD GetKerningPairs(DWORD numPairs, LPKERNINGPAIR pKrnPair) const;
        DWORD SetMapperFlags(DWORD flag) const;
#endif

        // Palette and color functions
        HPALETTE GetCurrentPalette() const;
        COLORREF GetNearestColor(COLORREF color) const;
        void RealizePalette() const;

#ifndef _WIN32_WCE
        BOOL GetColorAdjustment(LPCOLORADJUSTMENT pCA) const;
        BOOL SetColorAdjustment(const COLORADJUSTMENT* pCA) const;
        BOOL UpdateColors() const;
#endif

        // Pen Functions
        HPEN GetCurrentPen() const;
        LOGPEN GetLogPen() const;

#ifndef _WIN32_WCE
        BOOL GetMiterLimit(PFLOAT limit) const;
        BOOL SetMiterLimit(FLOAT newLimit, PFLOAT oldLimit) const;
#endif

        // Clipping and Region Functions
        int  ExcludeClipRect(int left, int top, int right, int bottom) const;
        int  ExcludeClipRect(const RECT& rc) const;
        int  GetClipBox(RECT& rc) const;
        int  IntersectClipRect(int left, int top, int right, int bottom) const;
        int  IntersectClipRect(const RECT& rc) const;
        BOOL RectVisible(const RECT& rc) const;
        int  SelectClipRgn(HRGN rgn) const;

#ifndef _WIN32_WCE
        BOOL AbortPath() const;
        BOOL BeginPath() const;
        BOOL EndPath() const;
        int  ExtSelectClipRgn(HRGN rgn, int mode) const;
        BOOL FlattenPath() const;
        int  GetPath(POINT* pPointArray, BYTE* pTypes, int count) const;
        int  OffsetClipRgn(int xOffset, int yOffset) const;
        BOOL PtVisible(int x, int y) const;
        BOOL SelectClipPath(int mode) const;
        BOOL StrokeAndFillPath() const;
        BOOL StrokePath() const;
        BOOL WidenPath() const;
#endif

        // Co-ordinate Functions
#ifndef _WIN32_WCE
        BOOL DPtoLP(LPPOINT pPointArray, int count)  const;
        BOOL DPtoLP(RECT& rc)  const;
        BOOL LPtoDP(LPPOINT pPointArray, int count)  const;
        BOOL LPtoDP(RECT& rc)  const;
#endif

        // Layout Functions
        DWORD GetLayout() const;
        DWORD SetLayout(DWORD layout) const;

        // Mapping functions
#ifndef _WIN32_WCE
        int  GetMapMode() const;
        int  SetMapMode(int mapMode) const;
        BOOL GetViewportOrgEx(LPPOINT pPoint)  const;
        BOOL SetViewportOrgEx(int x, int y, LPPOINT pPoint = NULL) const;
        BOOL SetViewportOrgEx(POINT point, LPPOINT pPointRet = NULL) const;
        BOOL OffsetViewportOrgEx(int width, int height, LPPOINT pPoint = NULL) const;
        BOOL GetViewportExtEx(LPSIZE pSize)  const;
        BOOL SetViewportExtEx(int x, int y, LPSIZE pSize = NULL) const;
        BOOL SetViewportExtEx(SIZE size, LPSIZE pSizeRet = NULL) const;
        BOOL ScaleViewportExtEx(int xNum, int xDenom, int yNum, int yDenom, LPSIZE pSize = NULL) const;
        BOOL GetWindowExtEx(LPSIZE pSize)  const;
        BOOL SetWindowExtEx(int x, int y, LPSIZE pSize = NULL) const;
        BOOL SetWindowExtEx(SIZE size, LPSIZE pSizeRet = NULL) const;
        BOOL ScaleWindowExtEx(int xNum, int xDenom, int yNum, int yDenom, LPSIZE pSize = NULL) const;
        BOOL GetWindowOrgEx(LPPOINT pPoint)  const;
        BOOL SetWindowOrgEx(int x, int y, LPPOINT pPoint = NULL) const;
        BOOL SetWindowOrgEx(POINT point, LPPOINT pPointRet = NULL) const;
        BOOL OffsetWindowOrgEx(int width, int height, LPPOINT pPoint = NULL) const;
#endif

        // MetaFile Functions
#ifndef _WIN32_WCE
        BOOL PlayMetaFile(HMETAFILE metaFile) const;
        BOOL PlayMetaFile(HENHMETAFILE enhMetaFile, const RECT& bounds) const;
#endif

        // Printer Functions
        int StartDoc(LPDOCINFO pDocInfo) const;
        int EndDoc() const;
        int StartPage() const;
        int EndPage() const;
        int AbortDoc() const;
        int SetAbortProc(BOOL (CALLBACK* pfn)(HDC, int)) const;

        // Text Functions
        int   DrawText(LPCTSTR string, int count, const RECT& rc, UINT format) const;
        BOOL  ExtTextOut(int x, int y, UINT options, const RECT& rc, LPCTSTR string, int count = -1, LPINT pDxWidths = NULL) const;
        COLORREF GetBkColor() const;
        int   GetBkMode() const;
        UINT  GetTextAlign() const;
        int   GetTextFace(int count, LPTSTR facename) const;
        COLORREF GetTextColor() const;
        BOOL  GetTextMetrics(TEXTMETRIC& metrics) const;
        COLORREF SetBkColor(COLORREF color) const;
        int   SetBkMode(int bkMode) const;
        UINT  SetTextAlign(UINT flags) const;
        COLORREF SetTextColor(COLORREF color) const;

#ifndef _WIN32_WCE
        int   DrawTextEx(LPTSTR string, int count, const RECT& rc, UINT format, LPDRAWTEXTPARAMS pDTParams) const;
        DWORD GetCharacterPlacement(LPCTSTR string, int count, int maxExtent,
                                    LPGCP_RESULTS pResults, DWORD flags) const;

        BOOL  GetCharABCWidths(UINT firstChar, UINT lastChar, LPABC pABC) const;
        BOOL  GetCharWidth(UINT firstChar, UINT lastChar, float* pBuffer) const;
        CSize GetTabbedTextExtent(LPCTSTR string, int count, int tabPositions, LPINT pTabStopPositions) const;
        int   GetTextCharacterExtra() const;
        CSize GetTextExtentPoint32(LPCTSTR string, int count) const;
        CSize GetTextExtentPoint32(LPCTSTR string) const;
        BOOL  GrayString(HBRUSH brush, GRAYSTRINGPROC pOutputFunc, LPARAM pData, int count, int x, int y, int width, int height) const;
        int   SetTextCharacterExtra(int charExtra) const;
        int   SetTextJustification(int breakExtra, int breakCount) const;
        CSize TabbedTextOut(int x, int y, LPCTSTR string, int count, int tabPositions, LPINT pTabStopPositions, int tabOrigin) const;
        BOOL  TextOut(int x, int y, LPCTSTR string, int count = -1) const;

  #if (_WIN32_WINNT >= 0x0500) && !defined(__GNUC__)
        BOOL  GetCharABCWidthsI(UINT first, UINT cgi, LPWORD pGI, LPABC pABC) const;
        BOOL  GetCharWidthI(UINT first, UINT cgi, LPWORD pGI, int* pBuffer) const;
  #endif // (_WIN32_WINNT >= 0x0500) && !defined(__GNUC__)
#endif  // _WIN32_WCE

    protected:
        void Release();
        void SetManaged(bool IsManaged) { m_pData->isManagedHDC = IsManaged; }

    private:
        void AddToMap();
        BOOL RemoveFromMap();

        CDC_Data* m_pData;      // pointer to the class's data members
    };


    /////////////////////////////////////////////////////////////
    // CClientDC manages a GDI device context for the client area
    // of a window.
    class CClientDC : public CDC
    {
    public:
        CClientDC(HWND wnd)
        {
            if (0 == wnd) wnd = GetDesktopWindow();
            assert(::IsWindow(wnd));

            try
            {
                HDC dc = ::GetDC(wnd);
                if (dc == 0)
                    throw CResourceException(g_msgGdiGetDC);

                Attach(dc, wnd);
                SetManaged(true);
            }

            catch(...)
            {
                Release();  // Cleanup
                throw;      // Rethrow
            }
        }

        virtual ~CClientDC() {}
    };


    ///////////////////////////////////////////////////////////////////
    // CClientDCEx manages a GDI device context for the client area
    // of a window. A clip regions can be specified, along with flags
    // such as DCX_WINDOW, DCX_CACHE, DCX_PARENTCLIP, DCX_CLIPSIBLINGS,
    // DCX_CLIPCHILDREN, DCX_NORESETATTRS, DCX_LOCKWINDOWUPDATE,
    // DCX_EXCLUDERGN, DCX_INTERSECTRGN and DCX_VALIDATE.
    class CClientDCEx : public CDC
    {
    public:
        CClientDCEx(HWND wnd, HRGN hrgnClip, DWORD flags)
        {
            if (0 == wnd) wnd = GetDesktopWindow();
            assert(::IsWindow(wnd));

            try
            {
                HDC dc = ::GetDCEx(wnd, hrgnClip, flags);
                if (dc == 0)
                    throw CResourceException(g_msgGdiGetDCEx);

                Attach(dc, wnd);
                SetManaged(true);
            }

            catch(...)
            {
                Release();  // Cleanup
                throw;      // Rethrow
            }
        }

        virtual ~CClientDCEx() {}
    };


    /////////////////////////////////////////////////////////////////////
    // CMemDC manage a memory device context which is compatable with the
    // specified device context. If this device context 0, the memory DC
    // is compatible with the application's current screen.
    class CMemDC : public CDC
    {
    public:
        explicit CMemDC(HDC dc)
        {
            try
            {
                CreateCompatibleDC(dc);
            }

            catch(...)
            {
                Release();  // Cleanup
                throw;      // Rethrow
            }
        }
        virtual ~CMemDC() {}
    };


    ///////////////////////////////////////////////////////////////
    // CPaintDC manages a GDI device used for painting via WM_PAINT
    // on the specified window.
    class CPaintDC : public CDC
    {
    public:
        CPaintDC(HWND wnd) : m_paint(wnd)
        {
            assert(::IsWindow(wnd));

            try
            {
                HDC dc = ::BeginPaint(wnd, &m_ps);
                if (dc == 0)
                    throw CResourceException(g_msgGdiBeginPaint);

                Attach(dc, wnd);
                SetManaged(true);
            }

            catch(...)
            {
                Release();  // Cleanup
                throw;      // Rethrow
            }
        }

        CPaintDC(const CPaintDC& rhs)  : CDC(rhs) // Copy constructor
        {
            m_paint = rhs.m_paint;
            m_ps = rhs.m_ps;
        }

        CPaintDC& operator = (const CPaintDC& rhs)
        {
            if (this != &rhs)
            {
                CDC::operator=(rhs);
                m_paint = rhs.m_paint;
                m_ps = rhs.m_ps;
            }

            return *this;
        }

        virtual ~CPaintDC()
        {
            ::EndPaint(m_paint, &m_ps);
        }

    private:
        HWND m_paint;
        PAINTSTRUCT m_ps;
    };


    ///////////////////////////////////////////////////////////////////
    // CWindowDC manages a GDI device context for the specified window,
    // including its client and non-client areas.
    class CWindowDC : public CDC
    {
    public:
        CWindowDC(HWND wnd)
        {
            if (0 == wnd) wnd = GetDesktopWindow();
            assert(::IsWindow(wnd));

            try
            {
                HDC dc = ::GetWindowDC(wnd);
                if (dc == 0)
                    throw CResourceException(g_msgGdiGetWinDC);

                Attach(dc, wnd);
                SetManaged(true);
            }

            catch(...)
            {
                Release();  // Cleanup
                throw;      // Rethrow
            }

        }
        virtual ~CWindowDC() {}
    };

#ifndef _WIN32_WCE

    ///////////////////////////////////////////////
    // CMetaFileDC manages a GDI device context for
    // a Windows-format metafile.
    class CMetaFileDC : public CDC
    {
    public:
        CMetaFileDC() {}
        virtual ~CMetaFileDC()
        {
            if (GetHDC())
            {
                // Note we should not get here.
                TRACE("Warning! A MetaFile or EnhMetaFile was created but not closed\n");
                ::DeleteMetaFile(Close());
                Detach();
            }
        }

        void Create(LPCTSTR pFilename = NULL)
        {
            try
            {
                assert(GetHDC() == 0);
                HDC dc = ::CreateMetaFile(pFilename);
                if (dc == 0)
                    throw CResourceException(g_msgGdiDC);

                Attach(dc);
                SetManaged(true);
            }
            catch (...)
            {
                Release();  // Cleanup
                throw;      // Rethrow
            }
        }

        // Closes the metafile and returns a CMetaFile object.
        // The CMetaFile object automatically deletes the HMETAFILE when the last copy
        // of the CMetaFile goes out of scope.
        CMetaFile Close()
        {
            assert(GetHDC());

            HMETAFILE meta = ::CloseMetaFile(GetHDC());
            Detach();
            return CMetaFile(meta);
        }

    };

    ///////////////////////////////////////////////////////////////////
    // CEnhMetaFileDC manages a GDI device context for a Windows-format
    // enhanced metafile.
    class CEnhMetaFileDC : public CDC
    {
    public:
        CEnhMetaFileDC() {}
        virtual ~CEnhMetaFileDC()
        {
            if (GetHDC())
            {
                // Note we should not get here.
                TRACE("Warning! An EnhMetaFile was created but not closed\n");
                ::DeleteEnhMetaFile(CloseEnhanced());
                Detach();
            }
        }

        void CreateEnhanced(HDC ref, LPCTSTR pFileName, const RECT* pBounds, LPCTSTR pDescription)
        {
            try
            {
                assert(GetHDC() == 0);
                HDC dc = ::CreateEnhMetaFile(ref, pFileName, pBounds, pDescription);
                if (dc == 0)
                    throw CResourceException(g_msgGdiDC);

                Attach(dc);
                SetManaged(true);
            }
            catch (...)
            {
                Release();  // Cleanup
                throw;      // Rethrow
            }
        }

        // Closes the enhanced metafile and returns a CEnhMetaFile object.
        // The CEnhMetaFile object automatically deletes the HENHMETAFILE when the last copy
        // of the CEnhMetaFile goes out of scope.
        CEnhMetaFile CloseEnhanced()
        {
            assert(GetHDC());

            HENHMETAFILE enhMeta = ::CloseEnhMetaFile(GetHDC());
            Detach();
            return CEnhMetaFile(enhMeta);
        }

    };


#endif


    ///////////////////////////////////////////////
    // The CBitmapInfoPtr class is a convenient wrapper for the BITMAPINFO
    // structure. The BITMAPINFO structure is used in the GetDIBits and
    // SetDIBits Window API functions.
    //
    // This class creates the colors array of the correct size based on the
    // color format (bit count) of the bitmap, and fills the BITMAPINFOHEADER.
    class CBitmapInfoPtr
    {
    public:
        CBitmapInfoPtr(HBITMAP bitmap)
        {
            BITMAP data;
            VERIFY(::GetObject(bitmap, sizeof(data), &data) != 0);

            // Convert the color format to a count of bits.
            WORD cClrBits = static_cast<WORD>(data.bmPlanes * data.bmBitsPixel);
            if (cClrBits == 1)       cClrBits = 1;
            else if (cClrBits <= 4)  cClrBits = 4;
            else if (cClrBits <= 8)  cClrBits = 8;
            else if (cClrBits <= 16) cClrBits = 16;
            else if (cClrBits <= 24) cClrBits = 24;
            else                     cClrBits = 32;

            // Allocate memory for the BITMAPINFO structure.
            UINT uQuadSize = (cClrBits >= 24)? 0 : UINT(sizeof(RGBQUAD)) * (1 << cClrBits);
            m_bmi.assign(sizeof(BITMAPINFOHEADER) + uQuadSize, 0);
            m_pbmiArray = (LPBITMAPINFO) &m_bmi.front();

            m_pbmiArray->bmiHeader.biSize       = sizeof(BITMAPINFOHEADER);
            m_pbmiArray->bmiHeader.biHeight     = data.bmHeight;
            m_pbmiArray->bmiHeader.biWidth      = data.bmWidth;
            m_pbmiArray->bmiHeader.biPlanes     = data.bmPlanes;
            m_pbmiArray->bmiHeader.biBitCount   = data.bmBitsPixel;
            m_pbmiArray->bmiHeader.biCompression = BI_RGB;
            if (cClrBits < 24)
                m_pbmiArray->bmiHeader.biClrUsed = (1 << cClrBits);
        }
        LPBITMAPINFO get() const { return m_pbmiArray; }
        operator LPBITMAPINFO() const { return m_pbmiArray; }
        LPBITMAPINFO operator->() const { return m_pbmiArray; }

    private:
        CBitmapInfoPtr(const CBitmapInfoPtr&);              // Disable copy construction
        CBitmapInfoPtr& operator = (const CBitmapInfoPtr&); // Disable assignment operator
        LPBITMAPINFO m_pbmiArray;
        std::vector<byte> m_bmi;
    };


}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


namespace Win32xx
{

    ///////////////////////////////////////////////
    // Declarations for the CGDIObject class
    //

    // Constructs the CGDIObject
    inline CGDIObject::CGDIObject()
    {
        m_pData = new CGDI_Data;
    }

    // Note: A copy of a CGDIObject is a clone of the original.
    //       Both objects manipulate the one HGDIOBJ.
    inline CGDIObject::CGDIObject(const CGDIObject& rhs)
    {
        m_pData = rhs.m_pData;
        InterlockedIncrement(&m_pData->count);
    }

    // Deconstructs the CGDIObject
    inline CGDIObject::~CGDIObject()
    {
        Release();
    }

    // Note: A copy of a CGDIObject is a clone of the original.
    //       Both objects manipulate the one HGDIOBJ.
    inline CGDIObject& CGDIObject::operator = ( const CGDIObject& rhs )
    {
        if (this != &rhs)
        {
            InterlockedIncrement(&rhs.m_pData->count);
            Release();
            m_pData = rhs.m_pData;
        }

        return *this;
    }

    inline void CGDIObject::operator = (const HGDIOBJ object)
    {
        Attach(object);
    }

    // Store the HDC and CDC pointer in the HDC map
    inline void CGDIObject::AddToMap()
    {
        assert( GetApp() );
        assert(m_pData->hGDIObject);

        GetApp()->AddCGDIData(m_pData->hGDIObject, m_pData);
    }

    // Attaches a GDI HANDLE to the CGDIObject.
    inline void CGDIObject::Attach(HGDIOBJ object)
    {
        assert(m_pData);

        if (m_pData && object != m_pData->hGDIObject)
        {
            // Release any existing GDI object
            if (m_pData->hGDIObject != 0)
            {
                Release();
                m_pData = new CGDI_Data;
            }

            if (object)
            {
                // Add the GDI object to this CCGDIObject
                CGDI_Data* pCGDIData = GetApp()->GetCGDIData(object);
                if (pCGDIData)
                {
                    delete m_pData;
                    m_pData = pCGDIData;
                    InterlockedIncrement(&m_pData->count);
                }
                else
                {
                    m_pData->hGDIObject = object;
                    AddToMap();
                }
            }
        }
    }

    inline void CGDIObject::DeleteObject()
    {
        assert(m_pData);

        if (m_pData && m_pData->hGDIObject != 0)
        {
            RemoveFromMap();

            ::DeleteObject(m_pData->hGDIObject);
            m_pData->hGDIObject = 0;
        }
    }

    // Detaches the HGDIOBJ from all objects.
    inline HGDIOBJ CGDIObject::Detach()
    {
        assert(m_pData);
        if (!m_pData) return 0;

        assert(m_pData->hGDIObject);

        HGDIOBJ object = m_pData->hGDIObject;
        RemoveFromMap();
        m_pData->hGDIObject = 0;

        if (m_pData->count > 0)
        {
            if (InterlockedDecrement(&m_pData->count) == 0)
            {
                delete m_pData;
            }
        }

        m_pData = new CGDI_Data;
        return object;
    }

    // Returns the GDI handle (HGDIOBJ) associated with this object.
    inline HGDIOBJ CGDIObject::GetHandle() const
    {
        assert(m_pData);
        return m_pData ? m_pData->hGDIObject : 0;
    }

    // Retrieves information for the specified graphics object.
    // Refer to GetObject in the Windows API documentation for more information.
    inline int CGDIObject::GetObject(int count, LPVOID pObject) const
    {
        assert(m_pData);
        return m_pData ? ::GetObject(m_pData->hGDIObject, count, pObject) : 0;
    }

    inline void CGDIObject::Release()
    {
        assert(m_pData);

        if (m_pData && InterlockedDecrement(&m_pData->count) == 0)
        {
            if (m_pData->hGDIObject != NULL)
            {
                if (m_pData->isManagedObject)
                {
                    ::DeleteObject(m_pData->hGDIObject);
                }

                RemoveFromMap();
            }

            delete m_pData;
            m_pData = 0;
        }
    }

    inline BOOL CGDIObject::RemoveFromMap()
    {
        BOOL success = FALSE;

        if ( GetApp() )
        {
            // Allocate an iterator for our HDC map
            std::map<HGDIOBJ, CGDI_Data*, CompareGDI>::iterator m;

            CWinApp* pApp = GetApp();
            CThreadLock mapLock(pApp->m_gdiLock);
            m = pApp->m_mapCGDIData.find(m_pData->hGDIObject);
            if (m != pApp->m_mapCGDIData.end())
            {
                // Erase the CGDIObject pointer entry from the map
                pApp->m_mapCGDIData.erase(m);
                success = TRUE;
            }

        }

        return success;
    }


    ///////////////////////////////////////////////
    // Declarations for the CBitmap class
    //

    inline CBitmap::CBitmap()
    {
    }

    inline CBitmap::CBitmap(HBITMAP bitmap)
    {
        Attach(bitmap);
    }

    inline CBitmap::CBitmap(LPCTSTR pResName)
    {
        LoadBitmap(pResName);
    }

    inline CBitmap::CBitmap(int resID)
    {
        LoadBitmap(resID);
    }

    inline CBitmap::operator HBITMAP() const
    {
        return static_cast<HBITMAP>(GetHandle());
    }

    inline CBitmap::~CBitmap()
    {
    }

    // Loads a bitmap from a resource using the resource ID.
    // Refer to LoadImage in the Windows API documentation for more information.
    inline BOOL CBitmap::LoadBitmap(int resID)
    {
        return LoadBitmap(MAKEINTRESOURCE(resID));
    }

    // Loads a bitmap from a resource using the resource string.
    // Refer to LoadImage in the Windows API documentation for more information.
    inline BOOL CBitmap::LoadBitmap(LPCTSTR pResName)
    {
        assert( GetApp() );

        HBITMAP bitmap = reinterpret_cast<HBITMAP>(::LoadImage(GetApp()->GetResourceHandle(), pResName, IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR));
        if (bitmap != 0)
        {
            Attach(bitmap);
            SetManaged(true);
        }
        return (0 != bitmap);  // boolean expression
    }

    // Loads a bitmap from a resource using the resource ID.
    // Refer to LoadImage in the Windows API documentation for more information.
    inline BOOL CBitmap::LoadImage(UINT resID, UINT flags)
    {
        return LoadImage(MAKEINTRESOURCE(resID), flags);
    }

    // Loads a bitmap from a resource using the resource string.
    // Refer to LoadImage in the Windows API documentation for more information.
    inline BOOL CBitmap::LoadImage(LPCTSTR pResName, UINT flags)
    {
        assert( GetApp() );

        HBITMAP bitmap = reinterpret_cast<HBITMAP>(::LoadImage(GetApp()->GetResourceHandle(), pResName, IMAGE_BITMAP, 0, 0, flags));
        if (bitmap != 0)
        {
            Attach(bitmap);
            SetManaged(true);
        }
        return (0 != bitmap);  // boolean expression
    }

    // Loads a predefined bitmap.
    // Predefined bitmaps include: OBM_BTNCORNERS, OBM_BTSIZE, OBM_CHECK, OBM_CHECKBOXES, OBM_CLOSE, OBM_COMBO
    //  OBM_DNARROW, OBM_DNARROWD, OBM_DNARROWI, OBM_LFARROW, OBM_LFARROWD, OBM_LFARROWI, OBM_MNARROW,OBM_OLD_CLOSE
    //  OBM_OLD_DNARROW, OBM_OLD_LFARROW, OBM_OLD_REDUCE, OBM_OLD_RESTORE, OBM_OLD_RGARROW, OBM_OLD_UPARROW
    //  OBM_OLD_ZOOM, OBM_REDUCE, OBM_REDUCED, OBM_RESTORE, OBM_RESTORED, OBM_RGARROW, OBM_RGARROWD, OBM_RGARROWI
    //  OBM_SIZE, OBM_UPARROW, OBM_UPARROWD, OBM_UPARROWI, OBM_ZOOM, OBM_ZOOMD
    // Refer to LoadBitmap in the Windows API documentation for more information.
    inline BOOL CBitmap::LoadOEMBitmap(UINT bitmapID) // for OBM_/OCR_/OIC_
    {
        HBITMAP bitmap = ::LoadBitmap(NULL, MAKEINTRESOURCE(bitmapID));
        if (bitmap != 0)
        {
            Attach( ::LoadBitmap(NULL, MAKEINTRESOURCE(bitmapID)) );
            SetManaged(true);
        }
        return (0 != bitmap);  // boolean expression
    }

#ifndef _WIN32_WCE

    // Rapidly converts the bitmap image to pale grayscale image suitable for disabled icons.
    // The mask is the transparent color. Pixels with this color are not converted.
    // Supports 32 bit, 24 bit, 16 bit and 8 bit colors.
    // For 16 and 8 bit colors, ensure the mask is a color in the current palette.
    inline void CBitmap::ConvertToDisabled(COLORREF mask) const
    {
        BITMAP data = GetBitmapData();

        // Requires 8 bits per pixel
        if (data.bmBitsPixel < 8)
            return;

        // Copy the image data into the 'bits' byte array.
        CWindowDC dc(HWND_DESKTOP);
        CBitmapInfoPtr pbmi(*this);
        BITMAPINFOHEADER& bmiHeader = pbmi->bmiHeader;
        bmiHeader.biBitCount = 24;
        dc.GetDIBits(*this, 0, data.bmHeight, NULL, pbmi, DIB_RGB_COLORS);
        DWORD size = pbmi->bmiHeader.biSizeImage;
        std::vector<byte> vBits(size, 0);
        byte* bits = &vBits[0];
        dc.GetDIBits(*this, 0, data.bmHeight, bits, pbmi, DIB_RGB_COLORS);

        UINT widthBytes = bmiHeader.biSizeImage / bmiHeader.biHeight;
        int yOffset = 0;
        int xOffset;
        size_t index;

        for (int row = 0; row < bmiHeader.biHeight; ++row)
        {
            xOffset = 0;

            for (int column = 0; column < bmiHeader.biWidth; ++column)
            {
                // Calculate index
                index = size_t(yOffset) + size_t(xOffset);

                // skip for colors matching the mask
                if ((bits[index + 0] != GetRValue(mask)) &&
                    (bits[index + 1] != GetGValue(mask)) &&
                    (bits[index + 2] != GetBValue(mask)))
                {
                    BYTE byGray = BYTE(95 + (bits[index + 2] * 3 + bits[index + 1] * 6 + bits[index + 0]) / 20);
                    bits[index] = byGray;
                    bits[index + 1] = byGray;
                    bits[index + 2] = byGray;
                }

                // Increment the horizontal offset
                xOffset += bmiHeader.biBitCount >> 3;
            }

            // Increment vertical offset
            yOffset += widthBytes;
        }

        dc.SetDIBits(*this, 0, data.bmHeight, bits, pbmi, DIB_RGB_COLORS);
    }

    // Creates a new bitmap using the bitmap data and colors specified by the bitmap resource and the color mapping information.
    // Refer to CreateMappedBitmap in the Windows API documentation for more information.
    inline HBITMAP CBitmap::CreateMappedBitmap(UINT bitmapID, UINT flags /*= 0*/, LPCOLORMAP pColorMap /*= NULL*/, int mapSize /*= 0*/)
    {
        assert(GetApp());
        HBITMAP bitmap = ::CreateMappedBitmap(GetApp()->GetResourceHandle(), bitmapID, static_cast<WORD>(flags), pColorMap, mapSize);
        if (bitmap == 0)
            throw CResourceException(g_msgGdiBitmap);

        Attach(bitmap);
        SetManaged(true);
        return bitmap;
    }

#endif // !_WIN32_WCE

    // Creates a bitmap with the specified width, height, and color format (color planes and bits-per-pixel).
    // Refer to CreateBitmap in the Windows API documentation for more information.
    inline HBITMAP CBitmap::CreateBitmap(int width, int height, UINT planes, UINT bitsPerPixel, LPCVOID pBits)
    {
        HBITMAP bitmap = ::CreateBitmap(width, height, planes, bitsPerPixel, pBits);
        if (bitmap == 0)
            throw CResourceException(g_msgGdiBitmap);

        Attach(bitmap);
        SetManaged(true);
        return bitmap;
    }

#ifndef _WIN32_WCE

    // Creates a bitmap with the width, height, and color format specified in the BITMAP structure.
    // Refer to CreateBitmapIndirect in the Windows API documentation for more information.
    inline HBITMAP CBitmap::CreateBitmapIndirect(const BITMAP& bitmap)
    {
        HBITMAP copyBitmap = ::CreateBitmapIndirect(&bitmap);
        if (copyBitmap == 0)
            throw CResourceException(g_msgGdiBitmap);

        Attach(copyBitmap);
        SetManaged(true);
        return copyBitmap;
    }

#endif // !_WIN32_WCE

    // Creates a bitmap compatible with the device that is associated with the specified device context.
    // Refer to CreateCompatibleBitmap in the Windows API documentation for more information.
    inline HBITMAP CBitmap::CreateCompatibleBitmap(HDC dc, int width, int height)
    {
        HBITMAP bitmap = ::CreateCompatibleBitmap(dc, width, height);
        if (bitmap == 0)
            throw CResourceException(g_msgGdiBitmap);

        Attach(bitmap);
        SetManaged(true);
        return bitmap;
    }

    // Retrieves the BITMAP structure.
    // Refer to GetObject in the Windows API documentation for more information.
    inline BITMAP CBitmap::GetBitmapData() const
    {
        assert(GetHandle() != NULL);
        BITMAP data;
        ZeroMemory(&data, sizeof(data));
        VERIFY(::GetObject(GetHandle(), sizeof(data), &data) != 0);
        return data;
    }

#ifndef _WIN32_WCE

    // Retrieves the dimensions of a compatible bitmap.
    // The retrieved dimensions must have been set by the SetBitmapDimensionEx function.
    // Refer to GetBitmapDimensionEx in the Windows API documentation for more information.
    inline CSize CBitmap::GetBitmapDimensionEx() const
    {
        assert(GetHandle() != NULL);
        CSize Size;
        ::GetBitmapDimensionEx(reinterpret_cast<HBITMAP>(GetHandle()), &Size);
        return Size;
    }

    // The SetBitmapDimensionEx function assigns preferred dimensions to a bitmap.
    // These dimensions can be used by applications; however, they are not used by the system.
    // Refer to SetBitmapDimensionEx in the Windows API documentation for more information.
    inline CSize CBitmap::SetBitmapDimensionEx(int width, int height) const
    {
        assert(GetHandle() != NULL);
        CSize Size;
        ::SetBitmapDimensionEx(reinterpret_cast<HBITMAP>(GetHandle()), width, height, Size);
        return Size;
    }

    // Creates a compatible bitmap (DDB) from a DIB and, optionally, sets the bitmap bits.
    // A CBitmapInfoPtr object can be used for the LPBITMAPINFO parameter.
    // Refer to CreateDIBitmap in the Windows API documentation for more information.
    inline HBITMAP CBitmap::CreateDIBitmap(HDC dc, const BITMAPINFOHEADER* pBMIH, DWORD init, LPCVOID pInit, const LPBITMAPINFO pBMI, UINT colorUse)
    {
        HBITMAP bitmap = ::CreateDIBitmap(dc, pBMIH, init, pInit, pBMI, colorUse);
        Attach(bitmap);
        SetManaged(true);
        return bitmap;
    }

    // Convert a bitmap image to gray scale.
    inline void CBitmap::GrayScaleBitmap()
    {
        // Requires 8 bits per pixel
        BITMAP data = GetBitmapData();
        if (data.bmBitsPixel < 8)
            return;

        // Create our LPBITMAPINFO object
        CBitmapInfoPtr pbmi(*this);
        BITMAPINFOHEADER& bmiHeader = pbmi->bmiHeader;
        bmiHeader.biBitCount = 24;

        // Create the reference DC for GetDIBits to use
        CMemDC memDC(NULL);

        // Use GetDIBits to create a DIB from our DDB, and extract the colour data
        GetDIBits(memDC, 0, bmiHeader.biHeight, NULL, pbmi, DIB_RGB_COLORS);
        std::vector<byte> vBits(bmiHeader.biSizeImage, 0);
        byte* pByteArray = &vBits[0];

        memDC.GetDIBits(*this, 0, bmiHeader.biHeight, pByteArray, pbmi, DIB_RGB_COLORS);
        UINT widthBytes = bmiHeader.biSizeImage/bmiHeader.biHeight;

        int yOffset = 0;
        int xOffset;
        size_t index;

        for (int row=0; row < bmiHeader.biHeight; ++row)
        {
            xOffset = 0;

            for (int column=0; column < bmiHeader.biWidth; ++column)
            {
                // Calculate index
                index = size_t(yOffset) + size_t(xOffset);

                BYTE byGray = (BYTE) ((pByteArray[index] + pByteArray[index +1]*6 + pByteArray[index +2] *3)/10);
                pByteArray[index]   = byGray;
                pByteArray[index +1] = byGray;
                pByteArray[index +2] = byGray;

                // Increment the horizontal offset
                xOffset += bmiHeader.biBitCount >> 3;
            }

            // Increment vertical offset
            yOffset += widthBytes;
        }

        // Save the modified colour back into our source DDB
        SetDIBits(memDC, 0, bmiHeader.biHeight, pByteArray, pbmi, DIB_RGB_COLORS);
    }

    // Modifies the colour of the Device Dependant Bitmap, by the colour.
    // correction values specified. The correction values can range from -255 to +255.
    // This function gains its speed by accessing the bitmap colour information
    // directly, rather than using GetPixel/SetPixel.
    inline void CBitmap::TintBitmap (int cRed, int cGreen, int cBlue)
    {
        // Create our LPBITMAPINFO object
        CBitmapInfoPtr pbmi(*this);
        BITMAPINFOHEADER& bmiHeader = pbmi->bmiHeader;
        bmiHeader.biBitCount = 24;

        // Create the reference DC for GetDIBits to use
        CMemDC memDC(NULL);

        // Use GetDIBits to create a DIB from our DDB, and extract the colour data
        GetDIBits(memDC, 0, bmiHeader.biHeight, NULL, pbmi, DIB_RGB_COLORS);
        std::vector<byte> vBits(bmiHeader.biSizeImage, 0);
        byte* pByteArray = &vBits[0];

        memDC.GetDIBits(*this, 0, bmiHeader.biHeight, pByteArray, pbmi, DIB_RGB_COLORS);
        UINT widthBytes = bmiHeader.biSizeImage/bmiHeader.biHeight;

        // Ensure sane colour correction values
        cBlue  = MIN(cBlue, 255);
        cBlue  = MAX(cBlue, -255);
        cRed   = MIN(cRed, 255);
        cRed   = MAX(cRed, -255);
        cGreen = MIN(cGreen, 255);
        cGreen = MAX(cGreen, -255);

        // Pre-calculate the RGB modification values
        int b1 = 256 - cBlue;
        int g1 = 256 - cGreen;
        int r1 = 256 - cRed;

        int b2 = 256 + cBlue;
        int g2 = 256 + cGreen;
        int r2 = 256 + cRed;

        // Modify the colour
        int yOffset = 0;
        int xOffset;
        size_t index;
        for (int Row=0; Row < bmiHeader.biHeight; ++Row)
        {
            xOffset = 0;

            for (int Column=0; Column < bmiHeader.biWidth; ++Column)
            {
                // Calculate index
                index = size_t(yOffset) + size_t(xOffset);

                // Adjust the colour values
                if (cBlue > 0)
                    pByteArray[index]   = (BYTE)(cBlue + (((pByteArray[index] *b1)) >>8));
                else if (cBlue < 0)
                    pByteArray[index]   = (BYTE)((pByteArray[index] *b2) >>8);

                if (cGreen > 0)
                    pByteArray[index+1] = (BYTE)(cGreen + (((pByteArray[index+1] *g1)) >>8));
                else if (cGreen < 0)
                    pByteArray[index+1] = (BYTE)((pByteArray[index+1] *g2) >>8);

                if (cRed > 0)
                    pByteArray[index+2] = (BYTE)(cRed + (((pByteArray[index+2] *r1)) >>8));
                else if (cRed < 0)
                    pByteArray[index+2] = (BYTE)((pByteArray[index+2] *r2) >>8);

                // Increment the horizontal offset
                xOffset += bmiHeader.biBitCount >> 3;
            }

            // Increment vertical offset
            yOffset += widthBytes;
        }

        // Save the modified colour back into our source DDB
        SetDIBits(memDC, 0, bmiHeader.biHeight, pByteArray, pbmi, DIB_RGB_COLORS);
    }

#endif // !_WIN32_WCE

    // Creates a DIB that applications can write to directly. The function gives you
    // a pointer to the location of the bitmap bit values. You can supply a handle to
    // a file-mapping object that the function will use to create the bitmap,
    // or you can let the system allocate the memory for the bitmap.
    // A CBitmapInfoPtr object can be used for the LPBITMAPINFO parameter.
    inline HBITMAP CBitmap::CreateDIBSection(HDC dc, const LPBITMAPINFO pBMI, UINT colorUse, LPVOID* pBits, HANDLE hSection, DWORD offset)
    {
        HBITMAP bitmap = ::CreateDIBSection(dc, pBMI, colorUse, pBits, hSection, offset);
        Attach(bitmap);
        SetManaged(true);
        return bitmap;
    }

#ifndef _WIN32_WCE

    // Retrieves the bits of the specified compatible bitmap and copies them
    // into a buffer as a DIB using the specified format.
    // A CBitmapInfoPtr object can be used for the LPBITMAPINFO parameter.
    // Refer to GetDIBits in the Windows API documentation for more information.
    inline int CBitmap::GetDIBits(HDC dc, UINT startScan, UINT scanLines,  LPVOID pBits, LPBITMAPINFO pBMI, UINT colorUse) const
    {
        assert(GetHandle() != NULL);
        return ::GetDIBits(dc, reinterpret_cast<HBITMAP>(GetHandle()), startScan, scanLines,  pBits, pBMI, colorUse);
    }

    // Sets the pixels in a compatible bitmap (DDB) using the color data found in the specified DIB.
    // A CBitmapInfoPtr object can be used for the LPBITMAPINFO parameter.
    // Refer to SetDIBits in the Windows API documentation for more information.
    inline int CBitmap::SetDIBits(HDC dc, UINT startScan, UINT scanLines, LPCVOID pBits, const LPBITMAPINFO pBMI, UINT colorUse) const
    {
        assert(GetHandle() != NULL);
        return ::SetDIBits(dc, reinterpret_cast<HBITMAP>(GetHandle()), startScan, scanLines, pBits, pBMI, colorUse);
    }

#endif // !_WIN32_WCE


    ///////////////////////////////////////////////
    // Definitions of the CBrush class
    //
    inline CBrush::CBrush()
    {
    }

    inline CBrush::CBrush(HBRUSH brush)
    {
        Attach(brush);
    }

    inline CBrush::CBrush(COLORREF color)
    {
        try
        {
            CreateSolidBrush(color);
        }

        catch(...)
        {
            Release();  // Cleanup
            throw;      // Rethrow
        }
    }

    inline CBrush::operator HBRUSH() const
    {
        return static_cast<HBRUSH>(GetHandle());
    }

    inline CBrush::~CBrush()
    {
    }

    // Creates a logical brush that has the specified solid color.
    // Refer to CreateSolidBrush in the Windows API documentation for more information.
    inline HBRUSH CBrush::CreateSolidBrush(COLORREF color)
    {
        HBRUSH brush = ::CreateSolidBrush(color);
        if (brush == 0)
            throw CResourceException(g_msgGdiBrush);

        Attach(brush);
        SetManaged(true);
        return brush;
    }

#ifndef _WIN32_WCE

    // Creates a logical brush that has the specified hatch pattern and color.
    // Refer to CreateHatchBrush in the Windows API documentation for more information.
    inline HBRUSH CBrush::CreateHatchBrush(int index, COLORREF color)
    {
        HBRUSH brush = ::CreateHatchBrush(index, color);
        if (brush == 0)
            throw CResourceException(g_msgGdiBrush);

        Attach(brush);
        SetManaged(true);
        return brush;
    }

    // Creates a logical brush from style, color, and pattern specified in the LOGPRUSH struct.
    // Refer to CreateBrushIndirect in the Windows API documentation for more information.
    inline HBRUSH CBrush::CreateBrushIndirect(const LOGBRUSH& logBrush)
    {
        HBRUSH brush = ::CreateBrushIndirect(&logBrush);
        if (brush == 0)
            throw CResourceException(g_msgGdiBrush);

        Attach(brush);
        SetManaged(true);
        return brush;
    }

    // Creates a logical brush that has the pattern specified by the specified device-independent bitmap (DIB).
    // Refer to CreateDIBPatternBrush in the Windows API documentation for more information.
    inline HBRUSH CBrush::CreateDIBPatternBrush(HGLOBAL hDIBPacked, UINT colorSpec)
    {
        HBRUSH brush = ::CreateDIBPatternBrush(hDIBPacked, colorSpec);
        if (brush == 0)
            throw CResourceException(g_msgGdiBrush);

        Attach(brush);
        SetManaged(true);
        return brush;
    }

    // Creates a logical brush that has the pattern specified by the device-independent bitmap (DIB).
    // Refer to CreateDIBPatternBrushPt in the Windows API documentation for more information.
    inline HBRUSH CBrush::CreateDIBPatternBrushPt(LPCVOID pPackedDIB, UINT usage)
    {
        HBRUSH brush = ::CreateDIBPatternBrushPt(pPackedDIB, usage);
        if (brush == 0)
            throw CResourceException(g_msgGdiBrush);

        Attach(brush);
        SetManaged(true);
        return brush;
    }

#endif // !defined(_WIN32_WCE)

    // Creates a logical brush with the specified bitmap pattern. The bitmap can be a DIB section bitmap,
    // which is created by the CreateDIBSection function, or it can be a device-dependent bitmap.
    // Refer to CreatePatternBrush in the Windows API documentation for more information.
    inline HBRUSH CBrush::CreatePatternBrush(HBITMAP bitmap)
    {
        HBRUSH brush = ::CreatePatternBrush(bitmap);
        if (brush == 0)
            throw CResourceException(g_msgGdiBrush);

        Attach(brush);
        SetManaged(true);
        return brush;
    }

    // Retrieves the LOGBRUSH structure that defines the style, color, and pattern of a physical brush.
    // Refer to GetObject in the Windows API documentation for more information.
    inline LOGBRUSH CBrush::GetLogBrush() const
    {
        assert(GetHandle() != NULL);
        LOGBRUSH logBrush;
        ZeroMemory(&logBrush, sizeof(logBrush));
        VERIFY(::GetObject (GetHandle(), sizeof(logBrush), &logBrush) != 0);
        return logBrush;
    }


    ///////////////////////////////////////////////
    // Definitions of the CFont class
    //
    inline CFont::CFont()
    {
    }

    inline CFont::CFont(HFONT font)
    {
        Attach(font);
    }

    inline CFont::CFont(const LOGFONT& logFont)
    {
        try
        {
            CreateFontIndirect(logFont);
        }

        catch(...)
        {
            Release();  // Cleanup
            throw;      // Rethrow;
        }
    }

    inline CFont::operator HFONT() const
    {
        return static_cast<HFONT>(GetHandle());
    }

    inline CFont::~CFont()
    {
    }

    // Creates a logical font that has the specified characteristics.
    // Refer to CreateFontIndirect in the Windows API documentation for more information.
    inline HFONT CFont::CreateFontIndirect(const LOGFONT& logFont)
    {
        HFONT font = ::CreateFontIndirect(&logFont);
        if (font == 0)
            throw CResourceException(g_msgGdiFont);

        Attach(font);
        SetManaged(true);
        return font;
    }

    // Creates a font of a specified typeface and point size.
    // Refer to CreateFontIndirect in the Windows API documentation for more information.
    inline HFONT CFont::CreatePointFont(int pointSize, LPCTSTR faceName, HDC dc /*= NULL*/, BOOL isBold /*= FALSE*/, BOOL isItalic /*= FALSE*/)
    {
        LOGFONT logFont;
        ZeroMemory(&logFont, sizeof(logFont));
        logFont.lfCharSet = DEFAULT_CHARSET;
        logFont.lfHeight = pointSize;

        StrCopy(logFont.lfFaceName, faceName, LF_FACESIZE);

        if (isBold)
            logFont.lfWeight = FW_BOLD;
        if (isItalic)
            logFont.lfItalic = 1;

        return CreatePointFontIndirect(logFont, dc);
    }

    // Creates a font of a specified typeface and point size.
    // This function automatically converts the height in lfHeight to logical
    // units using the specified device context.
    // Refer to CreateFontIndirect in the Windows API documentation for more information.
    inline HFONT CFont::CreatePointFontIndirect(const LOGFONT& logFont, HDC dc /* = NULL*/)
    {
        dc = (dc != NULL) ? dc : ::GetDC(HWND_DESKTOP);

        // convert nPointSize to logical units based on hDC
        LOGFONT logFont1 = logFont;

#ifndef _WIN32_WCE
        POINT pt = { 0, 0 };
        pt.y = ::MulDiv(::GetDeviceCaps(dc, LOGPIXELSY), logFont.lfHeight, 720);   // 72 points/inch, 10 decipoints/point
        ::DPtoLP(dc, &pt, 1);

        POINT ptOrg = { 0, 0 };
        ::DPtoLP(dc, &ptOrg, 1);

        logFont1.lfHeight = -abs(pt.y - ptOrg.y);
#else // CE specific
        // DP and LP are always the same on CE
        logFont1.lfHeight = -abs(((::GetDeviceCaps(dc, LOGPIXELSY)* logFont.lfHeight)/ 720));
#endif // _WIN32_WCE

        return CreateFontIndirect (logFont1);
    }

#ifndef _WIN32_WCE

    // Creates a logical font with the specified characteristics.
    // Refer to CreateFont in the Windows API documentation for more information.
    inline HFONT CFont::CreateFont(int height, int width, int escapement,
            int orientation, int weight, DWORD italic, DWORD underline,
            DWORD strikeOut, DWORD charSet, DWORD outPrecision,
            DWORD clipPrecision, DWORD quality, DWORD pitchAndFamily,
            LPCTSTR faceName)
    {
        HFONT font = ::CreateFont(height, width, escapement,
            orientation, weight, italic, underline, strikeOut,
            charSet, outPrecision, clipPrecision, quality,
            pitchAndFamily, faceName);

        if (font == 0)
            throw CResourceException(g_msgGdiFont);

        Attach(font);
        SetManaged(true);
        return font;
    }

#endif // #ifndef _WIN32_WCE

    // Retrieves the Logfont structure that contains font attributes.
    // Refer to GetObject in the Windows API documentation for more information.
    inline LOGFONT CFont::GetLogFont() const
    {
        assert(GetHandle() != NULL);
        LOGFONT logFont;
        ZeroMemory(&logFont, sizeof(logFont));
        VERIFY(::GetObject(GetHandle(), sizeof(logFont), &logFont) != 0);
        return logFont;
    }


    ///////////////////////////////////////////////
    // Definitions of the CPalette class
    //
    inline CPalette::CPalette()
    {
    }

    inline CPalette::CPalette(HPALETTE palette)
    {
        Attach(palette);
    }

    inline CPalette::operator HPALETTE() const
    {
        return static_cast<HPALETTE>(GetHandle());
    }

    inline CPalette::~CPalette ()
    {
    }

    // Creates a logical palette from the information in the specified LOGPALETTE structure.
    // Refer to CreatePalette in the Windows API documentation for more information.
    inline HPALETTE CPalette::CreatePalette(LPLOGPALETTE lpLogPalette)
    {
        HPALETTE palette = ::CreatePalette (lpLogPalette);
        if (palette == 0)
            throw CResourceException(g_msgGdiPalette);

        Attach(palette);
        SetManaged(true);
        return palette;
    }

#ifndef _WIN32_WCE

    // Creates a halftone palette for the specified device context (DC).
    // Refer to CreateHalftonePalette in the Windows API documentation for more information.
    inline HPALETTE CPalette::CreateHalftonePalette(HDC dc)
    {
        HPALETTE palette = ::CreateHalftonePalette(dc);
        if (palette == 0)
            throw CResourceException(g_msgGdiPalette);

        Attach(palette);
        ::RealizePalette(dc);
        SetManaged(true);
        return palette;
    }

#endif // !_WIN32_WCE

    // Retrieve the number of entries in the palette.
    // Refer to GetObject in the Windows API documentation for more information.
    inline int CPalette::GetEntryCount() const
    {
        assert(GetHandle() != NULL);
        WORD entries = 0;
        VERIFY(::GetObject(GetHandle(), sizeof(WORD), &entries) != 0);
        return static_cast<int>(entries);
    }

    // Retrieves a specified range of palette entries from the palette.
    // Refer to GetPaletteEntries in the Windows API documentation for more information.
    inline UINT CPalette::GetPaletteEntries(UINT startIndex, UINT entries, LPPALETTEENTRY pPaletteColors) const
    {
        assert(GetHandle() != NULL);
        return ::GetPaletteEntries(reinterpret_cast<HPALETTE>(GetHandle()), startIndex, entries, pPaletteColors);
    }

    // Sets RGB (red, green, blue) color values and flags in a range of entries in the palette.
    // Refer to SetPaletteEntries in the Windows API documentation for more information.
    inline UINT CPalette::SetPaletteEntries(UINT startIndex, UINT entries, LPPALETTEENTRY pPaletteColors) const
    {
        assert(GetHandle() != NULL);
        return ::SetPaletteEntries(reinterpret_cast<HPALETTE>(GetHandle()), startIndex, entries, pPaletteColors);
    }

#ifndef _WIN32_WCE

    // Replaces entries in the palette.
    // Refer to AnimatePalette in the Windows API documentation for more information.
    inline void CPalette::AnimatePalette(UINT startIndex, UINT entries, LPPALETTEENTRY pPaletteColors) const
    {
        assert(GetHandle() != NULL);
        ::AnimatePalette(reinterpret_cast<HPALETTE>(GetHandle()), startIndex, entries, pPaletteColors);
    }

    //  Increases or decreases the size of the palette based on the specified value.
    // Refer to ResizePalette in the Windows API documentation for more information.
    inline BOOL CPalette::ResizePalette(UINT entries) const
    {
        assert(GetHandle() != NULL);
        return ::ResizePalette(reinterpret_cast<HPALETTE>(GetHandle()), entries);
    }

#endif // !_WIN32_WCE

    // Retrieves the index for the entry in the palette most closely matching a specified color value.
    // Refer to GetNearestPaletteIndex in the Windows API documentation for more information.
    inline UINT CPalette::GetNearestPaletteIndex(COLORREF color) const
    {
        assert(GetHandle() != NULL);
        return ::GetNearestPaletteIndex(reinterpret_cast<HPALETTE>(GetHandle()), color);
    }


    ///////////////////////////////////////////////
    // Declarations for the CPen class
    //
    inline CPen::CPen()
    {
    }

    inline CPen::CPen(HPEN pen)
    {
        Attach(pen);
    }

    inline CPen::CPen(int penStyle, int width, COLORREF color)
    {
        try
        {
            CreatePen(penStyle, width, color);
        }

        catch(...)
        {
            Release();  // Cleanup
            throw;      // Rethrow
        }
    }

#ifndef _WIN32_WCE

    inline CPen::CPen(int penStyle, int width, const LOGBRUSH& logBrush, int styleCount /*= 0*/, const DWORD* pStyle /*= NULL*/)
    {
        try
        {
            Attach(::ExtCreatePen(penStyle, width, &logBrush, styleCount, pStyle));
        }

        catch(...)
        {
            Release();  // Cleanup
            throw;      // Rethrow
        }
    }

#endif // !_WIN32_WCE

    inline CPen::operator HPEN () const
    {
        return static_cast<HPEN>(GetHandle());
    }

    inline CPen::~CPen()
    {
    }

    // Creates a logical pen that has the specified style, width, and color.
    // Refer to CreatePen in the Windows API documentation for more information.
    inline HPEN CPen::CreatePen(int penStyle, int width, COLORREF color)
    {
        HPEN pen = ::CreatePen(penStyle, width, color);
        Attach(pen);
        SetManaged(true);
        return pen;
    }

    // Creates a logical pen that has the style, width, and color specified in a structure.
    // Refer to CreatePenIndirect in the Windows API documentation for more information.
    inline HPEN CPen::CreatePenIndirect(const LOGPEN& logPen)
    {
        HPEN pen = ::CreatePenIndirect(&logPen);
        Attach(pen);
        SetManaged(true);
        return pen;
    }

    // Retrieves the LOGPEN struct that specifies the pen's style, width, and color.
    // Refer to GetObject in the Windows API documentation for more information.
    inline LOGPEN CPen::GetLogPen() const
    {
        assert(GetHandle() != NULL);

        LOGPEN logPen;
        ZeroMemory(&logPen, sizeof(logPen));
        VERIFY(::GetObject(GetHandle(), sizeof(logPen), &logPen) != 0);
        return logPen;
    }

#ifndef _WIN32_WCE

    // Creates a logical cosmetic or geometric pen that has the specified style, width, and brush attributes.
    // Refer to ExtCreatePen in the Windows API documentation for more information.
    inline HPEN CPen::ExtCreatePen(int penStyle, int width, const LOGBRUSH& logBrush, int styleCount /* = 0*/, const DWORD* pStyle /*= NULL*/)
    {
        HPEN pen = ::ExtCreatePen(penStyle, width, &logBrush, styleCount, pStyle);
        Attach(pen);
        SetManaged(true);
        return pen;
    }

    // Retrieves the EXTLOGPEN struct that specifies the pen's style, width, color and brush attributes.
    // Refer to GetObject in the Windows API documentation for more information.
    inline EXTLOGPEN CPen::GetExtLogPen() const
    {
        assert(GetHandle() != NULL);

        EXTLOGPEN exLogPen;
        ZeroMemory(&exLogPen, sizeof(exLogPen));
        VERIFY(::GetObject(GetHandle(), sizeof(exLogPen), &exLogPen) != 0);
        return exLogPen;
    }

#endif // !_WIN32_WCE


    ///////////////////////////////////////////////
    // Definitions of the CRgn class
    //
    inline CRgn::CRgn()
    {
    }

    inline CRgn::CRgn(HRGN rgn)
    {
        Attach(rgn);
    }

    inline CRgn::operator HRGN() const
    {
        return static_cast<HRGN>(GetHandle());
    }

    inline CRgn::~CRgn()
    {
    }

    // Creates a rectangular region.
    // Refer to CreateRectRgn in the Windows API documentation for more information.
    inline HRGN CRgn::CreateRectRgn(int x1, int y1, int x2, int y2)
    {
        HRGN rgn = ::CreateRectRgn(x1, y1, x2, y2);
        if (rgn == 0)
            throw CResourceException(g_msgGdiRegion);

        Attach(rgn);
        SetManaged(true);
        return rgn;
    }

    inline HRGN CRgn::CreateRectRgnIndirect(const RECT& rc)
    // Creates a rectangular region.
    // Refer to CreateRectRgnIndirect in the Windows API documentation for more information.
    {
        HRGN rgn = ::CreateRectRgnIndirect(&rc);
        if (rgn == 0)
            throw CResourceException(g_msgGdiRegion);

        Attach(rgn);
        SetManaged(true);
        return rgn;
    }

#ifndef _WIN32_WCE

    // Creates an elliptical region.
    // Refer to CreateEllipticRgn in the Windows API documentation for more information.
    inline HRGN CRgn::CreateEllipticRgn(int x1, int y1, int x2, int y2)
    {
        HRGN rgn = ::CreateEllipticRgn(x1, y1, x2, y2);
        if (rgn == 0)
            throw CResourceException(g_msgGdiRegion);

        Attach(rgn);
        SetManaged(true);
        return rgn;
    }

    // Creates an elliptical region.
    // Refer to CreateEllipticRgnIndirect in the Windows API documentation for more information.
    inline HRGN CRgn::CreateEllipticRgnIndirect(const RECT& rc)
    {
        HRGN rgn = ::CreateEllipticRgnIndirect(&rc);
        if (rgn == 0)
            throw CResourceException(g_msgGdiRegion);

        Attach(rgn);
        SetManaged(true);
        return rgn;
    }

    // Creates a polygonal region.
    // Refer to CreatePolygonRgn in the Windows API documentation for more information.
    inline HRGN CRgn::CreatePolygonRgn(LPPOINT pPoints, int count, int mode)
    {
        HRGN rgn = ::CreatePolygonRgn(pPoints, count, mode);
        if (rgn == 0)
            throw CResourceException(g_msgGdiRegion);

        Attach(rgn);
        SetManaged(true);
        return rgn;
    }

    // Creates a region consisting of a series of polygons. The polygons can overlap.
    // Refer to CreatePolyPolygonRgn in the Windows API documentation for more information.
    inline HRGN CRgn::CreatePolyPolygonRgn(LPPOINT pPoints, LPINT pPolyCounts, int count, int polyFillMode)
    {
        HRGN rgn = ::CreatePolyPolygonRgn(pPoints, pPolyCounts, count, polyFillMode);
        if (rgn == 0)
            throw CResourceException(g_msgGdiRegion);

        Attach(rgn);
        SetManaged(true);
        return rgn;
    }

    // Creates a rectangular region with rounded corners.
    // Refer to CreateRoundRectRgn in the Windows API documentation for more information.
    inline HRGN CRgn::CreateRoundRectRgn(int x1, int y1, int x2, int y2, int x3, int y3)
    {
        HRGN rgn = ::CreateRoundRectRgn(x1, y1, x2, y2, x3, y3);
        if (rgn == 0)
            throw CResourceException(g_msgGdiRegion);

        Attach(rgn);
        SetManaged(true);
        return rgn;
    }

    // Creates a region from the path that is selected into the specified device context.
    // The resulting region uses device coordinates.
    // Refer to PathToRegion in the Windows API documentation for more information.
    inline HRGN CRgn::CreateFromPath(HDC dc)
    {
        assert(dc != NULL);
        HRGN rgn = ::PathToRegion(dc);
        if (rgn == 0)
            throw CResourceException(g_msgGdiRegion);

        Attach(rgn);
        SetManaged(true);
        return rgn;
    }

#endif // !_WIN32_WCE

    // Creates a region from the specified region and transformation data.
    // If the XFORM pointer is NULL, the identity transformation is used.
    // Refer to ExtCreateRegion in the Windows API documentation for more information.
    inline HRGN CRgn::CreateFromData(const XFORM* pXForm, int count, const RGNDATA* pRgnData)
    {
        HRGN rgn = ::ExtCreateRegion(pXForm, count, pRgnData);
        if (rgn == 0)
            throw CResourceException(g_msgGdiRegion);

        Attach(rgn);
        SetManaged(true);
        return rgn;
    }

    // Converts the region into a rectangular region with the specified coordinates.
    // Refer to SetRectRgn in the Windows API documentation for more information.
    inline void CRgn::SetRectRgn(int x1, int y1, int x2, int y2) const
    {
        assert(GetHandle() != NULL);
        ::SetRectRgn(reinterpret_cast<HRGN>(GetHandle()), x1, y1, x2, y2);
    }

    // Converts the region into a rectangular region with the specified coordinates.
    // Refer to SetRectRgn in the Windows API documentation for more information.
    inline void CRgn::SetRectRgn(const RECT& rc) const
    {
        assert(GetHandle() != NULL);
        ::SetRectRgn(reinterpret_cast<HRGN>(GetHandle()), rc.left, rc.top, rc.right, rc.bottom);
    }

    // Combines two specified regions and stores the result.
    // Refer to CombineRgn in the Windows API documentation for more information.
    inline int CRgn::CombineRgn(HRGN hSrc1, HRGN hSrc2, int combineMode) const
    {
        assert(GetHandle() != NULL);
        return ::CombineRgn(reinterpret_cast<HRGN>(GetHandle()), hSrc1, hSrc2, combineMode);
    }

    // Combines the specified region with the current region.
    // Refer to CombineRgn in the Windows API documentation for more information.
    inline int CRgn::CombineRgn(HRGN hSrc, int combineMode) const
    {
        assert(GetHandle() != NULL);
        return ::CombineRgn(reinterpret_cast<HRGN>(GetHandle()), (HRGN)GetHandle(), hSrc, combineMode);
    }

    // Assigns the specified region to the current region.
    // Refer to CombineRgn in the Windows API documentation for more information.
    inline int CRgn::CopyRgn(HRGN hSrc) const
    {
        assert(GetHandle() != NULL);
        assert(hSrc);
        return ::CombineRgn(reinterpret_cast<HRGN>(GetHandle()), hSrc, NULL, RGN_COPY);
    }

    // Checks the two regions to determine whether they are identical.
    // Refer to EqualRgn in the Windows API documentation for more information.
    inline BOOL CRgn::EqualRgn(HRGN rgn) const
    {
        assert(GetHandle() != NULL);
        return ::EqualRgn(reinterpret_cast<HRGN>(GetHandle()), rgn);
    }

    // Moves a region by the specified offsets.
    // Refer to OffsetRgn in the Windows API documentation for more information.
    inline int CRgn::OffsetRgn(int x, int y) const
    {
        assert(GetHandle() != NULL);
        return ::OffsetRgn(reinterpret_cast<HRGN>(GetHandle()), x, y);
    }

    // Moves a region by the specified offsets.
    // Refer to OffsetRgn in the Windows API documentation for more information.
    inline int CRgn::OffsetRgn(POINT& pt) const
    {
        assert(GetHandle() != NULL);
        return ::OffsetRgn(reinterpret_cast<HRGN>(GetHandle()), pt.x, pt.y);
    }

    // Retrieves the bounding rectangle of the region, and stores it in the specified RECT.
    // The return value indicates the region's complexity: NULLREGION;SIMPLEREGION; or COMPLEXREGION.
    // Refer to GetRgnBox in the Windows API documentation for more information.
    inline int CRgn::GetRgnBox(RECT& rc) const
    {
        assert(GetHandle() != NULL);
        return ::GetRgnBox(reinterpret_cast<HRGN>(GetHandle()), &rc);
    }

    // Fills the specified buffer with data describing a region.
    // Refer to GetRegionData in the Windows API documentation for more information.
    inline int CRgn::GetRegionData(LPRGNDATA pRgnData, int dataSize) const
    {
        assert(GetHandle() != NULL);
        return static_cast<int>(::GetRegionData(reinterpret_cast<HRGN>(GetHandle()), dataSize, pRgnData));
    }

    // Determines whether the specified point is inside the specified region.
    // Refer to PtInRegion in the Windows API documentation for more information.
    inline BOOL CRgn::PtInRegion(int x, int y) const
    {
        assert(GetHandle() != NULL);
        return ::PtInRegion(reinterpret_cast<HRGN>(GetHandle()), x, y);
    }

    // Determines whether the specified point is inside the specified region.
    // Refer to PtInRegion in the Windows API documentation for more information.
    inline BOOL CRgn::PtInRegion(POINT& pt) const
    {
        assert(GetHandle() != NULL);
        return ::PtInRegion(reinterpret_cast<HRGN>(GetHandle()), pt.x, pt.y);
    }

    // Determines whether the specified rect is inside the specified region.
    // Refer to RectInRegion in the Windows API documentation for more information.
    inline BOOL CRgn::RectInRegion(const RECT& rc) const
    {
        assert(GetHandle() != NULL);
        return ::RectInRegion(reinterpret_cast<HRGN>(GetHandle()), &rc);
    }


    ///////////////////////////////////////////////
    // Definitions of the CDC class
    //

    inline CDC::CDC()
    {
        // Allocate memory for our data members
        m_pData = new CDC_Data;
    }

    // This constructor assigns an existing HDC to the CDC
    // The HDC WILL be released or deleted when the CDC object is destroyed
    // The wnd parameter is only used in WindowsCE. It specifies the HWND of a Window or
    // Window Client DC.
    // Note: this constructor permits a call like this:
    // CDC MyCDC = SomeHDC;
    inline CDC::CDC(HDC dc, HWND wnd /*= 0*/)
    {
        m_pData = new CDC_Data;
        Attach(dc, wnd);
    }

#ifndef _WIN32_WCE

    // Note: this assignment operator permits a call like this:
    // CDC MyCDC;
    // MyCDC = SomeHDC;
    inline void CDC::operator = (const HDC dc)
    {
        Attach(dc);
    }

#endif

    // The copy constructor is called when a temporary copy of the CDC needs to be created.
    // This can happen when a CDC is passed by value in a function call. Each CDC copy manages
    // the same Device Context and GDI objects.
    inline CDC::CDC(const CDC& rhs) // Copy constructor
    {
        m_pData = rhs.m_pData;
        InterlockedIncrement(&m_pData->count);
    }

    // Note: A copy of a CDC is a clone of the original.
    //       Both objects manipulate the one HDC
    inline CDC& CDC::operator = (const CDC& rhs)
    {
        if (this != &rhs)
        {
            InterlockedIncrement(&rhs.m_pData->count);
            Release();
            m_pData = rhs.m_pData;
        }

        return *this;
    }

    inline CDC::~CDC ()
    {
        Release();
    }

    // Store the HDC and CDC pointer in the HDC map
    inline void CDC::AddToMap()
    {
        assert( GetApp() );
        assert(m_pData->dc != 0);

        GetApp()->AddCDCData(m_pData->dc, m_pData);
    }

    // Attaches a HDC to the CDC object.
    // The wnd parameter is only required on WinCE.
    inline void CDC::Attach(HDC dc, HWND wnd /* = 0*/)
    {
        UNREFERENCED_PARAMETER(wnd);
        assert(m_pData);

        if (m_pData && dc != m_pData->dc)
        {
            if (m_pData->dc)
            {
                Release();

                // Assign values to our data members
                m_pData = new CDC_Data;
            }

            if (dc)
            {
                CDC_Data* pCDCData = GetApp()->GetCDCData(dc);
                if (pCDCData)
                {
                    delete m_pData;
                    m_pData = pCDCData;
                    InterlockedIncrement(&m_pData->count);
                }
                else
                {
                    m_pData->dc = dc;

        #ifndef _WIN32_WCE
                    m_pData->wnd = ::WindowFromDC(dc);
        #else
                    m_pData->wnd = wnd;
        #endif

                    AddToMap();
                    m_pData->savedDCState = ::SaveDC(dc);
                }
            }
        }
    }

    // Detaches the HDC from all CDC objects. Restores the HDC back to the
    // state saved when it was attached.
    // Note: We rarely need to detach the HDC from CDC. The framework will
    // release or delete the HDC automatically if required.
    inline HDC CDC::Detach()
    {
        assert(m_pData);
        if (!m_pData) return 0;

        assert(m_pData->dc != 0);

        HDC hDC = m_pData->dc;
        RemoveFromMap();
        m_pData->dc = 0;
        ::RestoreDC(hDC, m_pData->savedDCState);

        if (m_pData->count > 0)
        {
            if (InterlockedDecrement(&m_pData->count) == 0)
            {
                delete m_pData;
            }
        }

        // Assign values to our data members
        m_pData = new CDC_Data;

        return hDC;
    }

#ifndef _WIN32_WCE

    // Enumerates the pens or brushes available for the device context. This function calls
    // the application-defined callback function once for each available object, supplying
    // data describing that object.
    // Refer to EnumObjects in the Windows API documentation for more information.
    inline int CDC::EnumObjects(int objectType, GOBJENUMPROC pObjectFunc, LPARAM lparam) const
    {
        assert(m_pData->dc != 0);
        return ::EnumObjects(m_pData->dc, objectType, pObjectFunc, lparam);
    }

#endif

    // Returns a memory device context (DC) compatible with the specified device.
    // Refer to CreateCompatibleDC in the Windows API documentation for more information.
    inline HDC CDC::CreateCompatibleDC(HDC hSource)
    {
        assert(m_pData->dc == NULL);
        HDC dc = ::CreateCompatibleDC(hSource);

        if (dc == NULL)
            throw CResourceException(g_msgGdiDC);

        m_pData->dc = dc;
        m_pData->isManagedHDC = TRUE;
        AddToMap();

        return dc;
    }

    // Returns a device context (DC) for a device using the specified name.
    // Refer to CreateDC in the Windows API documentation for more information.
    inline HDC CDC::CreateDC(LPCTSTR pDriver, LPCTSTR pDevice, LPCTSTR pOutput, const DEVMODE* pInitData)
    {
        assert(m_pData->dc == NULL);
        HDC dc = ::CreateDC(pDriver, pDevice, pOutput, pInitData);

        if (dc == NULL)
            throw CResourceException(g_msgGdiDC);

        m_pData->dc = dc;
        m_pData->isManagedHDC = TRUE;
        AddToMap();
        return dc;
    }

#ifndef _WIN32_WCE

    // Returns an information context for the specified device. The information context
    // provides a fast way to get information about the device without creating a device context (DC).
    // However, GDI drawing functions cannot accept a handle to an information context.
    // Refer to CreateIC in the Windows API documentation for more information.
    inline HDC CDC::CreateIC(LPCTSTR pDriver, LPCTSTR pDevice, LPCTSTR pOutput, const DEVMODE* pInitData)
    {
        assert(m_pData->dc == NULL);
        HDC dc = ::CreateIC(pDriver, pDevice, pOutput, pInitData);

        if (dc == 0)
            throw CResourceException(g_msgGdiIC);

        m_pData->dc = dc;
        m_pData->isManagedHDC = TRUE;
        AddToMap();
        return dc;
    }

#endif

    // Draws the specified bitmap to the specified DC using the mask colour provided as the transparent colour
    // Suitable for use with a Window DC or a memory DC
    inline void CDC::DrawBitmap(int x, int y, int cx, int cy, HBITMAP bitmap, COLORREF mask) const
    {
        // Create the Image memory DC
        CMemDC imageDC(*this);
        imageDC.SetBkColor(mask);
        imageDC.SelectObject(bitmap);

        // Create the Mask memory DC
        CMemDC maskDC(*this);
        maskDC.CreateBitmap(cx, cy, 1, 1, NULL);
        maskDC.BitBlt(0, 0, cx, cy, imageDC, 0, 0, SRCCOPY);

        // Mask the image to 'this' DC
        BitBlt(x, y, cx, cy, imageDC, 0, 0, SRCINVERT);
        BitBlt(x, y, cx, cy, maskDC, 0, 0, SRCAND);
        BitBlt(x, y, cx, cy, imageDC, 0, 0, SRCINVERT);
    }

    // An efficient color gradient filler compatible with all Windows operating systems
    inline void CDC::GradientFill(COLORREF color1, COLORREF color2, const RECT& rc, BOOL isVertical) const
    {
        int Width = rc.right - rc.left;
        int Height = rc.bottom - rc.top;

        int r1 = GetRValue(color1);
        int g1 = GetGValue(color1);
        int b1 = GetBValue(color1);

        int r2 = GetRValue(color2);
        int g2 = GetGValue(color2);
        int b2 = GetBValue(color2);

        COLORREF oldBkColor = GetBkColor();

        if (isVertical)
        {
            for (int i=0; i < Width; ++i)
            {
                int r = r1 + (i * (r2-r1) / Width);
                int g = g1 + (i * (g2-g1) / Width);
                int b = b1 + (i * (b2-b1) / Width);
                SetBkColor(RGB(r, g, b));
                CRect line( i + rc.left, rc.top, i + 1 + rc.left, rc.top+Height);
                ExtTextOut(0, 0, ETO_OPAQUE, line, NULL, 0, 0);
            }
        }
        else
        {
            for (int i=0; i < Height; ++i)
            {
                int r = r1 + (i * (r2-r1) / Height);
                int g = g1 + (i * (g2-g1) / Height);
                int b = b1 + (i * (b2-b1) / Height);
                SetBkColor(RGB(r, g, b));
                CRect line(rc.left, i + rc.top, rc.left+Width, i + 1 + rc.top);
                ExtTextOut(0, 0, ETO_OPAQUE, line, NULL, 0, 0);
            }
        }

        SetBkColor(oldBkColor);
    }

    inline void CDC::Release()
    {
        if (m_pData->count > 0)
        {
            if (InterlockedDecrement(&m_pData->count) == 0)
            {
                Destroy();
                delete m_pData;
                m_pData = 0;
            }
        }
    }

    inline BOOL CDC::RemoveFromMap()
    {
        BOOL success = FALSE;

        if ( GetApp() )
        {
            // Allocate an iterator for our Data map
            std::map<HDC, CDC_Data*, CompareHDC>::iterator m;

            CWinApp* pApp = GetApp();
            CThreadLock mapLock(pApp->m_gdiLock);
            m = pApp->m_mapCDCData.find(m_pData->dc);
            if (m != pApp->m_mapCDCData.end())
            {
                // Erase the CDC data entry from the map
                pApp->m_mapCDCData.erase(m);
                success = TRUE;
            }

        }

        return success;
    }

    // Restores a device context (DC) to the specified state.
    // Refer to RestoreDC in the Windows API documentation for more information.
    inline BOOL CDC::RestoreDC(int savedDC) const
    {
        assert(m_pData);
        if (!m_pData) return 0;

        assert(m_pData->dc != 0);
        return ::RestoreDC(m_pData->dc, savedDC);
    }

    // Saves the current state of the specified device context.
    // Refer to SaveDC in the Windows API documentation for more information.
    inline int CDC::SaveDC() const
    {
        assert(m_pData);
        if (!m_pData) return 0;

        assert(m_pData->dc != 0);
        return ::SaveDC(m_pData->dc);
    }

    // Select a GDI object into the device context.
    // Refer to SelectObject in the Windows API documentation for more information.
    inline HGDIOBJ CDC::SelectObject(HGDIOBJ object) const
    {
        assert(m_pData);
        if (!m_pData) return 0;

        HGDIOBJ oldObject = ::SelectObject(m_pData->dc, object);
        if (oldObject == 0)
            throw CResourceException(g_msgGdiSelObject);

        return oldObject;
    }

    // Select a bitmap into the device context.
    // Refer to SelectObject in the Windows API documentation for more information.
    inline HBITMAP CDC::SelectObject(HBITMAP bitmap) const
    {
        assert(m_pData);
        if (!m_pData) return 0;

        HBITMAP oldBitmap = reinterpret_cast<HBITMAP>(::SelectObject(m_pData->dc, bitmap));
        assert(oldBitmap != 0);     // SelectObject will fail if bitmap is invalid.

        return oldBitmap;
    }

    // Select a brush into the device context.
    // Refer to SelectObject in the Windows API documentation for more information.
    inline HBRUSH CDC::SelectObject(HBRUSH brush) const
    {
        assert(m_pData);
        if (!m_pData) return 0;

        HBRUSH oldBrush = reinterpret_cast<HBRUSH>(::SelectObject(m_pData->dc, brush));
        assert(oldBrush != 0);

        return oldBrush;
    }

    // Select a font into the device context.
    // Refer to SelectObject in the Windows API documentation for more information.
    inline HFONT CDC::SelectObject(HFONT font) const
    {
        assert(m_pData);
        if (!m_pData) return 0;

        HFONT oldFont = reinterpret_cast<HFONT>(::SelectObject(m_pData->dc, font));
        assert(oldFont != 0);

        return oldFont;
    }

    // Select a pen into the device context.
    // Refer to SelectObject in the Windows API documentation for more information.
    inline HPEN CDC::SelectObject(HPEN pen) const
    {
        assert(m_pData);
        if (!m_pData) return 0;

        HPEN oldPen = reinterpret_cast<HPEN>(::SelectObject(m_pData->dc, pen));
        assert(oldPen != 0);

        return oldPen;
    }

    // Select a region into the device context.
    // The return value indicates the region's complexity: NULLREGION; SIMPLEREGION; or COMPLEXREGION.
    // Refer to SelectObject in the Windows API documentation for more information.
    inline int CDC::SelectObject(HRGN rgn) const
    {
        assert(m_pData);
        if (!m_pData) return 0;

        HGDIOBJ object = ::SelectObject(m_pData->dc, rgn);
        assert(object != HGDI_ERROR);
        return static_cast<int> (reinterpret_cast<INT_PTR>(object));
    }

    // Fills a rectangle with a solid color
    inline void CDC::SolidFill(COLORREF color, const RECT& rc) const
    {
        COLORREF oldColor = SetBkColor(color);
        ExtTextOut(0, 0, ETO_OPAQUE, rc, NULL, 0, 0);
        SetBkColor(oldColor);
    }

    /////////////////////////
    // Bitmap functions

    // Creates a compatible bitmap and selects it into the device context.
    // Refer to CreateCompatibleBitmap in the Windows API documentation for more information.
    inline void CDC::CreateCompatibleBitmap(HDC dc, int cx, int cy)
    {
        assert(m_pData->dc != 0);

        CBitmap bitmap;
        bitmap.CreateCompatibleBitmap(dc, cx, cy);
        SelectObject(bitmap);
        m_pData->bitmap = bitmap;
    }

    // Creates a bitmap and selects it into the device context.
    // Refer to CreateBitmap in the Windows API documentation for more information.
    inline void CDC::CreateBitmap(int cx, int cy, UINT planes, UINT bitsPerPixel, LPCVOID pColors)
    {
        assert(m_pData);
        if (!m_pData) return;

        assert(m_pData->dc != 0);

        CBitmap bitmap;
        bitmap.CreateBitmap(cx, cy, planes, bitsPerPixel, pColors);
        SelectObject(bitmap);
        m_pData->bitmap = bitmap;
    }

#ifndef _WIN32_WCE

    // Creates a bitmap and selects it into the device context.
    // Refer to CreateBitmapIndirect in the Windows API documentation for more information.
    inline void CDC::CreateBitmapIndirect (const BITMAP& bitmap)
    {
        assert(m_pData);
        if (!m_pData) return;

        assert(m_pData->dc != 0);

        CBitmap newBitmap;
        newBitmap.CreateBitmapIndirect(bitmap);
        SelectObject(newBitmap);
        m_pData->bitmap = newBitmap;
    }

    // Creates a bitmap and selects it into the device context.
    // A CBitmapInfoPtr object can be used for the LPBITMAPINFO parameter.
    // Refer to CreateDIBitmap in the Windows API documentation for more information.
    inline void CDC::CreateDIBitmap(HDC dc, const BITMAPINFOHEADER& bmih, DWORD init, LPCVOID pInit,
                                        const LPBITMAPINFO pBMI,  UINT flags)
    {
        assert(m_pData);
        if (!m_pData) return;

        assert(m_pData->dc != 0);

        CBitmap newBitmap;
        newBitmap.CreateDIBitmap(dc, &bmih, init, pInit, pBMI, flags);
        SelectObject(newBitmap);
        m_pData->bitmap = newBitmap;
    }

#endif

    // Creates a bitmap and selects it into the device context.
    // A CBitmapInfoPtr object can be used for the LPBITMAPINFO parameter.
    // Refer to CreateDIBSection in the Windows API documentation for more information.
    inline void CDC::CreateDIBSection(HDC dc, const LPBITMAPINFO pBMI, UINT usage, LPVOID* pBits,
                                        HANDLE hSection, DWORD offset)
    {
        assert(m_pData);
        if (!m_pData) return;

        assert(m_pData->dc != 0);

        CBitmap newBitmap;
        newBitmap.CreateDIBSection(dc, pBMI, usage, pBits, hSection, offset);
        SelectObject(newBitmap);
        m_pData->bitmap = newBitmap;
    }

    // Provides a convenient method of detaching a bitmap from a memory device context.
    // Returns the CBitmap detached from the DC.
    // Usage:  CBitmap MyBitmap = MyMemDC.DetachBitmap();
    inline CBitmap CDC::DetachBitmap()
    {
        // Create a stock bitmap to replace the current one.
        CBitmap bitmap;
        bitmap.CreateBitmap(1, 1, 1, 1, 0);

        CBitmap oldBitmap = SelectObject(bitmap);
        m_pData->bitmap = bitmap;

        return oldBitmap;
    }

    // Deletes or releases the device context and returns the CDC object to its
    // default state, ready for reuse.
    inline void CDC::Destroy()
    {
        assert(m_pData);
        if (!m_pData) return;

        if (m_pData->dc != 0)
        {
            RemoveFromMap();

            if (m_pData->isManagedHDC)
            {
                // Return the DC back to its initial state
                ::RestoreDC(m_pData->dc, m_pData->savedDCState);

                // We need to release a Window DC, and delete a memory DC
                if (m_pData->wnd != 0)
                    ::ReleaseDC(m_pData->wnd, m_pData->dc);
                else
                    if (!::DeleteDC(m_pData->dc))
                        ::ReleaseDC(NULL, m_pData->dc);
            }

            m_pData->dc = 0;
            m_pData->wnd = 0;
            m_pData->isManagedHDC = FALSE;
        }
    }

    // Retrieves the BITMAP information for the current HBITMAP.
    // Refer to GetObject in the Windows API documentation for more information.
    inline BITMAP CDC::GetBitmapData() const
    {
        assert(m_pData->dc != 0);

        HBITMAP bitmap = (HBITMAP)::GetCurrentObject(m_pData->dc, OBJ_BITMAP);
        BITMAP bitmapInfo;
        ZeroMemory(&bitmapInfo, sizeof(bitmapInfo));
        VERIFY(::GetObject(bitmap, sizeof(bitmapInfo), &bitmapInfo) != 0);
        return bitmapInfo;
    }

    // Retrieves the handle of the currently selected bitmap.
    // Refer to GetCurrentObject in the Windows API documentation for more information.
    inline HBITMAP CDC::GetCurrentBitmap() const
    {
        assert(m_pData->dc != 0);
        return static_cast<HBITMAP>(::GetCurrentObject(m_pData->dc, OBJ_BITMAP));
    }

    // Loads a bitmap from the resource and selects it into the device context.
    // Returns TRUE if successful.
    // Refer to LoadBitmap in the Windows API documentation for more information.
    inline BOOL CDC::LoadBitmap(UINT id)
    {
        return LoadBitmap(MAKEINTRESOURCE(id));
    }

    // Loads a bitmap from the resource and selects it into the device context.
    // Returns TRUE if successful.
    // Refer to LoadBitmap in the Windows API documentation for more information.
    inline BOOL CDC::LoadBitmap(LPCTSTR pResName)
    {
        assert(m_pData->dc != 0);

        CBitmap bitmap;
        BOOL isLoaded = bitmap.LoadBitmap(pResName);

        if (isLoaded)
        {
            SelectObject(bitmap);
            m_pData->bitmap = bitmap;
        }

        return isLoaded;
    }

    // Loads a bitmap from the resource and selects it into the device context.
    // The fuLoad parameter can be one of  LR_DEFAULTCOLOR, LR_CREATEDIBSECTION,
    // LR_LOADFROMFILE, LR_LOADTRANSPARENT, LR_MONOCHROME, LR_SHARED and LR_VGACOLOR.
    // Returns TRUE if successful.
    // Refer to LoadImage in the Windows API documentation for more information.
    inline BOOL CDC::LoadImage(UINT id, UINT flags)
    {
        return LoadImage(id, flags);
    }

    // Loads a bitmap from the resource and selects it into the device context.
    // The fuLoad parameter can be one of  LR_DEFAULTCOLOR, LR_CREATEDIBSECTION,
    // LR_LOADFROMFILE, LR_LOADTRANSPARENT, LR_MONOCHROME, LR_SHARED and LR_VGACOLOR.
    // Returns TRUE if successful.
    // Refer to LoadImage in the Windows API documentation for more information.
    inline BOOL CDC::LoadImage(LPCTSTR pResName, UINT flags)
    {
        assert(m_pData->dc != 0);

        CBitmap bitmap;
        BOOL IsLoaded = bitmap.LoadImage(pResName, flags);

        if (IsLoaded)
        {
            SelectObject(bitmap);
            m_pData->bitmap = bitmap;
        }

        return IsLoaded;
    }

    // Loads a predefined system bitmap and selects it into the device context.
    // Returns TRUE if successful.
    // Refer to LoadBitmap in the Windows API documentation for more information.
    inline BOOL CDC::LoadOEMBitmap(UINT bitmapID) // for OBM_/OCR_/OIC_
    {
        assert(m_pData->dc != 0);

        CBitmap bitmap;
        BOOL isLoaded = bitmap.LoadOEMBitmap(bitmapID);

        if (isLoaded)
        {
            SelectObject(bitmap);
            m_pData->bitmap = bitmap;
        }

        return isLoaded;
    }

#ifndef _WIN32_WCE

    // Creates and selects a new bitmap using the bitmap data and colors specified by
    // the bitmap resource and the color mapping information.
    // Refer to CreateMappedBitmap in the Windows API documentation for more information.
    inline void CDC::CreateMappedBitmap(UINT bitmapID, UINT flags /*= 0*/, LPCOLORMAP pColorMap /*= NULL*/, int mapSize /*= 0*/)
    {
        assert(m_pData->dc != 0);

        CBitmap bitmap;
        bitmap.CreateMappedBitmap(bitmapID, static_cast<WORD>(flags), pColorMap, mapSize);
        SelectObject(bitmap);
        m_pData->bitmap = bitmap;
    }

#endif // !_WIN32_WCE

    //////////////////
    // Brush functions

    // Creates the brush with the specified pattern, and selects it into the device context.
    // Refer to CreatePatternBrush in the Windows API documentation for more information.
    inline void CDC::CreatePatternBrush(HBITMAP bitmap)
    {
        assert(m_pData->dc != 0);

        CBrush brush;
        brush.CreatePatternBrush(bitmap);
        SelectObject(brush);
        m_pData->brush = brush;
    }

    // Creates the brush with the specified color, and selects it into the device context.
    // Refer to CreateSolidBrush in the Windows API documentation for more information.
    inline void CDC::CreateSolidBrush(COLORREF color)
    {
        assert(m_pData->dc != 0);

        CBrush brush;
        brush.CreateSolidBrush(color);
        SelectObject(brush);
        m_pData->brush = brush;
    }

    // Retrieves the handle of the currently selected brush object.
    // Refer to GetCurrentObject in the Windows API documentation for more information.
    inline HBRUSH CDC::GetCurrentBrush() const
    {
        assert(m_pData->dc != 0);
        return static_cast<HBRUSH>(::GetCurrentObject(m_pData->dc, OBJ_BRUSH));
    }

    // Retrieves the current brush information
    // Refer to GetObject in the Windows API documentation for more information.
    inline LOGBRUSH CDC::GetLogBrush() const
    {
        assert(m_pData->dc != 0);

        HBRUSH brush = reinterpret_cast<HBRUSH>(::GetCurrentObject(m_pData->dc, OBJ_BRUSH));
        LOGBRUSH logBrush;
        ZeroMemory(&logBrush, sizeof(logBrush));
        VERIFY(::GetObject(brush, sizeof(logBrush), &logBrush) != 0);
        return logBrush;
    }

#ifndef _WIN32_WCE

    // Creates the brush and selects it into the device context.
    // Refer to CreateBrushIndirect in the Windows API documentation for more information.
    inline void CDC::CreateBrushIndirect(const LOGBRUSH& logBrush)
    {
        assert(m_pData->dc != 0);

        CBrush brush;
        brush.CreateBrushIndirect(logBrush);
        SelectObject(brush);
        m_pData->brush = brush;
    }

    // Creates a brush with the specified hatch pattern and color, and selects it into the device context.
    // Refer to CreateHatchBrush in the Windows API documentation for more information.
    inline void CDC::CreateHatchBrush(int style, COLORREF color)
    {
        assert(m_pData->dc != 0);

        CBrush brush;
        brush.CreateHatchBrush(style, color);
        SelectObject(brush);
        m_pData->brush = brush;
    }

    // Creates a logical from the specified device-independent bitmap (DIB), and selects it into the device context.
    // Refer to CreateDIBPatternBrush in the Windows API documentation for more information.
    inline void CDC::CreateDIBPatternBrush(HGLOBAL hDIBPacked, UINT colorSpec)
    {
        assert(m_pData->dc != 0);

        CBrush brush;
        brush.CreateDIBPatternBrush(hDIBPacked, colorSpec);
        SelectObject(brush);
        m_pData->brush = brush;
    }

    // Creates a logical from the specified device-independent bitmap (DIB), and selects it into the device context.
    // Refer to CreateDIBPatternBrushPt in the Windows API documentation for more information.
    inline void CDC::CreateDIBPatternBrushPt(LPCVOID pPackedDIB, UINT usage)
    {
        assert(m_pData->dc != 0);

        CBrush brush;
        brush.CreateDIBPatternBrushPt(pPackedDIB, usage);
        SelectObject(brush);
        m_pData->brush = brush;
    }

    // Retrieves the current brush origin for the specified device context.
    // Refer to GetBrushOrgEx in the Windows API documentation for more information.
    inline CPoint CDC::GetBrushOrgEx() const
    {
        assert(m_pData->dc != 0);
        CPoint pt;
        ::GetBrushOrgEx(m_pData->dc, &pt);
        return pt;
    }


    // Sets the brush origin that GDI assigns to the next brush an application
    // selects into the specified device context. Returns the old brush origin.
    // Refer to SetBrushOrgEx in the Windows API documentation for more information.
    inline CPoint CDC::SetBrushOrgEx(int x, int y)
    {
        assert(m_pData->dc != 0);
        CPoint oldPt;
        ::SetBrushOrgEx(m_pData->dc, x, y, &oldPt);
        return oldPt;
    }

#endif

    /////////////////
    // Font functions

    // Creates a logical font and selects it into the device context.
    // Refer to CreateFontIndirect in the Windows API documentation for more information.
    inline void CDC::CreateFontIndirect(const LOGFONT& lf)
    {
        assert(m_pData->dc != 0);

        CFont font;
        font.CreateFontIndirect(lf);
        SelectObject(font);
        m_pData->font = font;
    }

    // Creates a font of a specified typeface and point size and selects it into the device context.
    // Refer to CreateFontIndirect in the Windows API documentation for more information.
    inline void CDC::CreatePointFont(int pointSize, LPCTSTR faceName, HDC dc /*= NULL*/, BOOL isBold /*= FALSE*/, BOOL isItalic /*= FALSE*/)
    {
        assert(m_pData->dc != 0);

        CFont font;
        font.CreatePointFont(pointSize, faceName, dc, isBold, isItalic);
        SelectObject(font);
        m_pData->font = font;
    }

    // Creates a font of a specified typeface and point size.
    // This function automatically converts the height in lfHeight to logical
    // units using the specified device context.
    // Refer to CreateFontIndirect in the Windows API documentation for more information.
    inline void CDC::CreatePointFontIndirect(const LOGFONT& logFont, HDC dc)
    {
        assert(m_pData->dc != 0);

        CFont font;
        font.CreatePointFontIndirect(logFont, dc);
        SelectObject(font);
        m_pData->font = font;
    }

    // Retrieves the handle to the current font object.
    // Refer to GetCurrentObject in the Windows API documentation for more information.
    inline HFONT CDC::GetCurrentFont() const
    {
        assert(m_pData->dc != 0);
        return static_cast<HFONT>(::GetCurrentObject(m_pData->dc, OBJ_FONT));
    }

    // Retrieves the current font information.
    // Refer to GetObject in the Windows API documentation for more information.
    inline LOGFONT CDC::GetLogFont() const
    {
        assert(m_pData->dc != 0);

        HFONT font = reinterpret_cast<HFONT>(::GetCurrentObject(m_pData->dc, OBJ_FONT));
        LOGFONT logFont;
        ZeroMemory(&logFont, sizeof(logFont));
        VERIFY(::GetObject(font, sizeof(logFont), &logFont) != 0);
        return logFont;
    }

#ifndef _WIN32_WCE

    // Creates a logical font with the specified characteristics.
    // Refer to CreateFont in the Windows API documentation for more information.
    inline void CDC::CreateFont (
                    int height,               // height of font
                    int width,                // average character width
                    int escapement,           // angle of escapement
                    int orientation,          // base-line orientation angle
                    int weight,               // font weight
                    DWORD italic,             // italic attribute option
                    DWORD underline,          // underline attribute option
                    DWORD strikeOut,          // strikeout attribute option
                    DWORD charSet,            // character set identifier
                    DWORD outputPrecision,    // output precision
                    DWORD clipPrecision,      // clipping precision
                    DWORD quality,            // output quality
                    DWORD pitchAndFamily,     // pitch and family
                    LPCTSTR faceName         // typeface name
                    )

    {
        assert(m_pData->dc != 0);

        CFont font;
        font.CreateFont(height, width, escapement, orientation, weight,
            italic, underline, strikeOut, charSet, outputPrecision,
            clipPrecision, quality, pitchAndFamily, faceName);

        SelectObject(font);
        m_pData->font = font;
    }

#endif

    ////////////////////
    // Palette functions

    // Creates and selects a palette.
    // Refer to CreatePalette in the Windows API documentation for more information.
    inline void CDC::CreatePalette(LPLOGPALETTE pLogPalette, BOOL forceBkgnd)
    {
        assert(m_pData->dc != 0);

        CPalette palette;
        palette.CreatePalette(pLogPalette);
        SelectPalette(palette, forceBkgnd);
        m_pData->palette = palette;
        RealizePalette();
    }

    // Retrieves the handle to the currently selected palette.
    // Refer to GetCurrentObject in the Windows API documentation for more information.
    inline HPALETTE CDC::GetCurrentPalette() const
    {
        assert(m_pData->dc != 0);
        return static_cast<HPALETTE>(::GetCurrentObject(m_pData->dc, OBJ_PAL));
    }

    // Retrieves a color value identifying a color from the system palette that will be
    // displayed when the specified color value is used.
    // Refer to GetNearestColor in the Windows API documentation for more information.
    inline COLORREF CDC::GetNearestColor(COLORREF color) const
    {
        assert(m_pData->dc != 0);
        return GetNearestColor(color);
    }

    // Use this to attach an existing palette.
    // Refer to SelectPalette in the Windows API documentation for more information.
    inline HPALETTE CDC::SelectPalette(const HPALETTE palette, BOOL forceBkgnd)
    {
        assert(m_pData->dc != 0);
        return static_cast<HPALETTE>(::SelectPalette(m_pData->dc, palette, forceBkgnd));
    }

    // Use this to realize changes to the device context palette.
    // Refer to RealizePalette in the Windows API documentation for more information.
    inline void CDC::RealizePalette() const
    {
        assert(m_pData->dc != 0);
        ::RealizePalette(m_pData->dc);
    }

#ifndef _WIN32_WCE

    // Creates and selects halftone palette.
    // Refer to CreateHalftonePalette in the Windows API documentation for more information.
    inline void CDC::CreateHalftonePalette(BOOL forceBkgnd)
    {
        assert(m_pData->dc != 0);

        CPalette palette;
        palette.CreateHalftonePalette(*this);
        ::SelectPalette(m_pData->dc, palette, forceBkgnd);
        m_pData->palette = palette;
        ::RealizePalette(m_pData->dc);
    }

    // Retrieves the color adjustment values for the device context.
    // Refer to GetColorAdjustment in the Windows API documentation for more information.
    inline BOOL CDC::GetColorAdjustment(LPCOLORADJUSTMENT pCA) const
    {
        assert(m_pData->dc != 0);
        return ::GetColorAdjustment(m_pData->dc, pCA);
    }

    // Sets the color adjustment values for the device context.
    // Refer to SetColorAdjustment in the Windows API documentation for more information.
    inline BOOL CDC::SetColorAdjustment(const COLORADJUSTMENT* pCA) const
    {
        assert(m_pData->dc != 0);
        return ::SetColorAdjustment(m_pData->dc, pCA);
    }

    // Updates the client area of the specified device context by remapping the current
    // colors in the client area to the currently realized logical palette.
    // Refer to UpdateColors in the Windows API documentation for more information.
    inline BOOL CDC::UpdateColors() const
    {
        assert(m_pData->dc != 0);
        return ::UpdateColors(m_pData->dc);
    }

#endif

    ////////////////
    // Pen functions

    // Creates the pen and selects it into the device context.
    // Refer to CreatePen in the Windows API documentation for more information.
    inline void CDC::CreatePen (int style, int width, COLORREF color)
    {
        assert(m_pData->dc != 0);

        CPen pen;
        pen.CreatePen(style, width, color);
        SelectObject(pen);
        m_pData->pen = pen;
    }

    // Creates the pen and selects it into the device context.
    // Refer to CreatePenIndirect in the Windows API documentation for more information.
    inline void CDC::CreatePenIndirect (const LOGPEN& logPen)
    {
        assert(m_pData->dc != 0);

        CPen pen;
        pen.CreatePenIndirect(logPen);
        SelectObject(pen);
        m_pData->pen = pen;
    }

#ifndef _WIN32_WCE

    // Creates a logical cosmetic or geometric pen that has the specified style, width, and brush attributes.
    // Refer to ExtCreatePen in the Windows API documentation for more information.
    inline void CDC::ExtCreatePen(int penStyle, int width, const LOGBRUSH& logBrush, int styleCount , const DWORD* pStyle)
    {
        assert(m_pData->dc != 0);

        CPen pen;
        pen.ExtCreatePen(penStyle, width, logBrush, styleCount, pStyle);
        SelectObject(pen);
        m_pData->pen = pen;
    }

#endif

    // Retrieves the handle to the currently selected pen.
    // Refer to GetCurrentObject in the Windows API documentation for more information.
    inline HPEN CDC::GetCurrentPen() const
    {
        assert(m_pData->dc != 0);
        return static_cast<HPEN>(::GetCurrentObject(m_pData->dc, OBJ_PEN));
    }

    // Retrieves the current pen information as a LOGPEN.
    // Refer to GetObject in the Windows API documentation for more information.
    inline LOGPEN CDC::GetLogPen() const
    {
        assert(m_pData->dc != 0);

        HPEN pen = reinterpret_cast<HPEN>(::GetCurrentObject(m_pData->dc, OBJ_PEN));
        LOGPEN logPen;
        ZeroMemory(&logPen, sizeof(logPen));
        VERIFY(::GetObject(pen, sizeof(logPen), &logPen) != 0);
        return logPen;
    }

    ////////////////////////////////////
    // Retrieve and Select Stock Objects

    // Retrieves a stock brush, pen, or font.
    // index values:  BLACK_BRUSH, DKGRAY_BRUSH, DC_BRUSH, HOLLOW_BRUSH, LTGRAY_BRUSH, NULL_BRUSH,
    //                WHITE_BRUSH, BLACK_PEN, DC_PEN, ANSI_FIXED_FONT, ANSI_VAR_FONT, DEVICE_DEFAULT_FONT,
    //                DEFAULT_GUI_FONT, OEM_FIXED_FONT, SYSTEM_FONT, or SYSTEM_FIXED_FONT.
    // Refer to GetStockObject in the Windows API documentation for more information.
    inline HGDIOBJ CDC::GetStockObject(int index) const
    {
        return ::GetStockObject(index);
    }

    // Selects a stock brush, pen, or font into the device context.
    // index values:  BLACK_BRUSH, DKGRAY_BRUSH, DC_BRUSH, HOLLOW_BRUSH, LTGRAY_BRUSH, NULL_BRUSH,
    //                WHITE_BRUSH, BLACK_PEN, DC_PEN, ANSI_FIXED_FONT, ANSI_VAR_FONT, DEVICE_DEFAULT_FONT,
    //                DEFAULT_GUI_FONT, OEM_FIXED_FONT, SYSTEM_FONT, or SYSTEM_FIXED_FONT.
    // Refer to GetStockObject in the Windows API documentation for more information.
    inline HGDIOBJ CDC::SelectStockObject(int index) const
    {
        assert(m_pData->dc != 0);
        HGDIOBJ hStockObject = ::GetStockObject(index);
        return SelectObject(hStockObject);
    }


    ///////////////////
    // Region functions

    // Creates a rectangular region from the rectangle co-ordinates.
    // The return value specifies the region's complexity: NULLREGION;SIMPLEREGION;COMPLEXREGION;ERROR.
    // Refer to CreateRectRgn in the Windows API documentation for more information.
    inline int CDC::CreateRectRgn(int left, int top, int right, int bottom)
    {
        assert(m_pData->dc != 0);

        CRgn rgn;
        rgn.CreateRectRgn(left, top, right, bottom);
        int Complexity = SelectClipRgn(rgn);
        m_pData->rgn = rgn;
        return Complexity;
    }

    // Creates a rectangular region from the rectangle co-ordinates.
    // The return value specifies the region's complexity: NULLREGION;SIMPLEREGION;COMPLEXREGION;ERROR.
    // Refer to CreateRectRgnIndirect in the Windows API documentation for more information.
    inline int CDC::CreateRectRgnIndirect(const RECT& rc)
    {
        assert(m_pData->dc != 0);

        CRgn rgn;
        rgn.CreateRectRgnIndirect(rc);
        int Complexity = SelectClipRgn(rgn);
        m_pData->rgn = rgn;
        return Complexity;
    }

    // Creates a region from the specified region data and transformation data.
    // The return value specifies the region's complexity: NULLREGION;SIMPLEREGION;COMPLEXREGION;ERROR.
    // Notes: GetRegionData can be used to get a region's data
    //        If the XFROM pointer is NULL, the identity transformation is used.
    // Refer to ExtCreateRegion in the Windows API documentation for more information.
    inline int CDC::CreateRgnFromData(const XFORM* pXform, DWORD count, const RGNDATA* pRgnData)
    {
        assert(m_pData->dc != 0);

        CRgn rgn;
        rgn.CreateFromData(pXform, count, pRgnData);
        int Complexity = SelectClipRgn(rgn);
        m_pData->rgn = rgn;
        return Complexity;
    }

#ifndef _WIN32_WCE

    // Creates the elliptical region from the bounding rectangle co-ordinates,
    // and selects it into the device context.
    // The return value specifies the region's complexity: NULLREGION;SIMPLEREGION;COMPLEXREGION;ERROR.
    // Refer to CreateEllipticRgn in the Windows API documentation for more information.
    inline int CDC::CreateEllipticRgn(int left, int top, int right, int bottom)
    {
        assert(m_pData->dc != 0);

        CRgn rgn;
        rgn.CreateEllipticRgn(left, top, right, bottom);
        int Complexity = SelectClipRgn(rgn);
        m_pData->rgn = rgn;
        return Complexity;
    }

    // Creates the elliptical region from the bounding rectangle co-ordinates.
    // and selects it into the device context.
    // The return value specifies the region's complexity: NULLREGION;SIMPLEREGION;COMPLEXREGION;ERROR.
    // Refer to CreateEllipticRgnIndirect in the Windows API documentation for more information.
    inline int CDC::CreateEllipticRgnIndirect(const RECT& rc)
    {
        assert(m_pData->dc != 0);

        CRgn rgn;
        rgn.CreateEllipticRgnIndirect(rc);
        int Complexity = SelectClipRgn(rgn);
        m_pData->rgn = rgn;
        return Complexity;
    }

    // Creates the polygon region from the array of points and selects it into
    // the device context. The polygon is presumed closed.
    // The return value specifies the region's complexity: NULLREGION;SIMPLEREGION;COMPLEXREGION;ERROR.
    // Refer to CreatePolygonRgn in the Windows API documentation for more information.
    inline int CDC::CreatePolygonRgn(LPPOINT pPointArray, int points, int polyFillMode)
    {
        assert(m_pData->dc != 0);

        CRgn rgn;
        rgn.CreatePolygonRgn(pPointArray, points, polyFillMode);
        int Complexity = SelectClipRgn(rgn);
        m_pData->rgn = rgn;
        return Complexity;
    }

    // Creates the polygon region from a series of polygons.The polygons can overlap.
    // The return value specifies the region's complexity: NULLREGION;SIMPLEREGION;COMPLEXREGION;ERROR.
    // Refer to CreatePolyPolygonRgn in the Windows API documentation for more information.
    inline int CDC::CreatePolyPolygonRgn(LPPOINT pPointArray, LPINT pCount, int count, int polyFillMode)
    {
        assert(m_pData->dc != 0);

        CRgn rgn;
        rgn.CreatePolyPolygonRgn(pPointArray, pCount, count, polyFillMode);
        int Complexity = SelectClipRgn(rgn);
        m_pData->rgn = rgn;
        return Complexity;
    }

    // Creates a region from the path that is selected into the specified device context.
    // The resulting region uses device coordinates.
    // Refer to PathToRegion in the Windows API documentation for more information.
    inline int CDC::CreateRgnFromPath(HDC dc)
    {
        assert(m_pData->dc != 0);

        CRgn rgn;
        rgn.CreateFromPath(dc);
        int Complexity = SelectClipRgn(rgn);
        m_pData->rgn = rgn;
        return Complexity;
    }

    // Creates a rectangular region with rounded corners.
    // Refer to CreateRoundRectRgn in the Windows API documentation for more information.
    inline int CDC::CreateRoundRectRgn(int x1, int y1, int x2, int y2, int x3, int y3)
    {
        assert(m_pData->dc != 0);

        CRgn rgn;
        rgn.CreateRoundRectRgn(x1, y1, x2, y2, x3, y3);
        int Complexity = SelectClipRgn(rgn);
        m_pData->rgn = rgn;
        return Complexity;
    }

#endif

    ////////////////////////////////
    // Wrappers for WinAPI functions


    // Retrieves device-specific information for the specified device.
    // Refer to GetDeviceCaps in the Windows API documentation for more information.
    inline int CDC::GetDeviceCaps (int index) const
    {
        assert(m_pData->dc != 0);
        return ::GetDeviceCaps(m_pData->dc, index);
    }

    //////////////////
    // Brush functions

#if (_WIN32_WINNT >= 0x0500)

    // Retrieves the current brush color from the device context.
    // Refer to GetDCBrushColor in the Windows API documentation for more information.
    inline COLORREF CDC::GetDCBrushColor() const
    {
        assert(m_pData->dc != 0);
        return ::GetDCBrushColor(m_pData->dc);
    }


    // Sets the current brush color for the device context.
    // Refer to SetDCBrushColor in the Windows API documentation for more information.
    inline COLORREF CDC::SetDCBrushColor(COLORREF color) const
    {
        assert(m_pData->dc != 0);
        return ::SetDCBrushColor(m_pData->dc, color);
    }

#endif

    /////////////////
    // Font Functions


#ifndef _WIN32_WCE

    // Retrieves font metric data for a TrueType font.
    // Refer to GetFontData in the Windows API documentation for more information.
    inline DWORD CDC::GetFontData(DWORD table, DWORD offset, LPVOID pBuffer, DWORD data) const
    {
        assert(m_pData->dc != 0);
        return ::GetFontData(m_pData->dc, table, offset, pBuffer, data);
    }

    // Returns information about the currently selected font for the display context.
    // Refer to GetFontLanguageInfo in the Windows API documentation for more information.
    inline DWORD CDC::GetFontLanguageInfo() const
    {
        assert(m_pData->dc != 0);
        return ::GetFontLanguageInfo(m_pData->dc);
    }

    // Retrieves the outline or bitmap for a character in the TrueType font that is selected into the device context.
    // Refer to GetGlyphOutline in the Windows API documentation for more information.
    inline DWORD CDC::GetGlyphOutline(UINT query, UINT format, LPGLYPHMETRICS pGM, DWORD bufSize,
                              LPVOID pBuffer, const MAT2* pMAT2) const
    {
        assert(m_pData->dc != 0);
        return ::GetGlyphOutline(m_pData->dc, query, format, pGM, bufSize, pBuffer, pMAT2);
    }

    // retrieves the character-kerning pairs for the currently selected font for the device context.
    // Refer to GetKerningPairs in the Windows API documentation for more information.
    inline DWORD CDC::GetKerningPairs(DWORD numPairs, LPKERNINGPAIR pKrnPair) const
    {
        assert(m_pData->dc != 0);
        return ::GetKerningPairs(m_pData->dc, numPairs, pKrnPair);
    }

    // Alters the algorithm the font mapper uses when it maps logical fonts to physical fonts.
    // Refer to SetMapperFlags in the Windows API documentation for more information.
    inline DWORD CDC::SetMapperFlags(DWORD flag) const
    {
        assert(m_pData->dc != 0);
        return ::SetMapperFlags(m_pData->dc, flag);
    }

    // Retrieves the miter limit for the device context. The miter limit is used when
    // drawing geometric lines that have miter joins.
    // Refer to GetMiterLimit in the Windows API documentation for more information.
    inline BOOL CDC::GetMiterLimit(PFLOAT pLimit) const
    {
        assert(m_pData->dc != 0);
        return ::GetMiterLimit(m_pData->dc, pLimit);
    }

    // Sets the limit for the length of miter joins for the device context.
    // Refer to SetMiterLimit in the Windows API documentation for more information.
    inline BOOL CDC::SetMiterLimit(FLOAT newLimit, PFLOAT pOldLimit) const
    {
        assert(m_pData->dc != 0);
        return ::SetMiterLimit(m_pData->dc, newLimit, pOldLimit);
    }

#endif

    /////////////////////
    // Clipping functions


    // Creates a new clipping region that consists of the existing clipping region minus
    // the specified rectangle.
    // Refer to ExcludeClipRect in the Windows API documentation for more information.
    inline int CDC::ExcludeClipRect(int left, int top, int right, int bottom) const
    {
        assert(m_pData->dc != 0);
        return ::ExcludeClipRect(m_pData->dc, left, top, right, bottom);
    }

    // Creates a new clipping region that consists of the existing clipping region minus
    // the specified rectangle.
    // Refer to ExcludeClipRect in the Windows API documentation for more information.
    inline int CDC::ExcludeClipRect(const RECT& rc) const
    {
        assert(m_pData->dc != 0);
        return ::ExcludeClipRect(m_pData->dc, rc.left, rc.top, rc.right, rc.bottom);
    }

    // Retrieves the dimensions of the tightest bounding rectangle that can be drawn around
    // the current visible area on the device.
    // Refer to GetClipBox in the Windows API documentation for more information.
    inline int CDC::GetClipBox (RECT& rc) const
    {
        assert(m_pData->dc != 0);
        return ::GetClipBox(m_pData->dc, &rc);
    }

    // Creates a new clipping region from the intersection of the current clipping region and
    // the specified rectangle.
    // Refer to IntersectClipRect in the Windows API documentation for more information.
    inline int CDC::IntersectClipRect(int left, int top, int right, int bottom) const
    {
        assert(m_pData->dc != 0);
        return ::IntersectClipRect(m_pData->dc, left, top, right, bottom);
    }

    // Creates a new clipping region from the intersection of the current clipping region
    // and the specified rectangle.
    // Refer to IntersectClipRect in the Windows API documentation for more information.
    inline int CDC::IntersectClipRect(const RECT& rc) const
    {
        assert(m_pData->dc != 0);
        return ::IntersectClipRect(m_pData->dc, rc.left, rc.top, rc.right, rc.bottom);
    }

    // Determines whether any part of the specified rectangle lies within the
    // clipping region of a device context.
    // Refer to RectVisible in the Windows API documentation for more information.
    inline BOOL CDC::RectVisible(const RECT& rc) const
    {
        assert(m_pData->dc != 0);
        return ::RectVisible (m_pData->dc, &rc);
    }

    // Selects a region as the current clipping region for the specified device context.
    // Note: Only a copy of the selected region is used.
    //       To remove a device-context's clipping region, specify a NULL region handle.
    // Refer to SelectClipRgn in the Windows API documentation for more information.
    inline int CDC::SelectClipRgn(HRGN rgn) const
    {
        assert(m_pData->dc != 0);
        return ::SelectClipRgn(m_pData->dc, rgn);
    }

#ifndef _WIN32_WCE

    // The AbortPath function closesand discards any paths in the specified device context.
    // Refer to AbortPath in the Windows API documentation for more information.
    inline BOOL CDC::AbortPath() const
    {
        assert(m_pData->dc != 0);
        return ::AbortPath(m_pData->dc);
    }

    // Opens a path bracket in the device context.
    // Refer to BeginPath in the Windows API documentation for more information.
    inline BOOL CDC::BeginPath() const
    {
        assert(m_pData->dc != 0);
        return ::BeginPath(m_pData->dc);
    }

    // Closes a path bracket and selects the path defined by the bracket into the device context.
    // Refer to EndPath in the Windows API documentation for more information.
    inline BOOL CDC::EndPath() const
    {
        assert(m_pData->dc != 0);
        return ::EndPath(m_pData->dc);
    }

    // Combines the specified region with the current clipping region using the specified mode.
    // Refer to ExtSelectClipRgn in the Windows API documentation for more information.
    inline int CDC::ExtSelectClipRgn(HRGN rgn, int mode) const
    {
        assert(m_pData->dc != 0);
        return ::ExtSelectClipRgn(m_pData->dc, rgn, mode);
    }

    // Transforms any curves in the path that is selected into the device context, turning each
    // curve into a sequence of lines.
    // Refer to FlattenPath in the Windows API documentation for more information.
    inline BOOL CDC::FlattenPath() const
    {
        assert(m_pData->dc != 0);
        return ::FlattenPath(m_pData->dc);
    }

    // Retrieves the coordinates defining the endpoints of lines and the control points of curves found in the path
    // that is selected into the device context.
    // pPoints: An array of POINT structures that receives the line endpoints and curve control points, in logical coordinates.
    // pTypes: Pointer to an array of bytes that receives the vertex types (PT_MOVETO, PT_LINETO or PT_BEZIERTO).
    // count: The total number of POINT structures that can be stored in the array pointed to by pPoints.
    // Refer to GetPath in the Windows API documentation for more information.
    inline int CDC::GetPath(POINT* pPoints, BYTE* pTypes, int count) const
    {
        assert(m_pData->dc != 0);
        return ::GetPath(m_pData->dc, pPoints, pTypes, count);
    }

    // Determines whether the specified point is within the clipping region of a device context.
    // Refer to PtVisible in the Windows API documentation for more information.
    inline BOOL CDC::PtVisible(int x, int y) const
    {
        assert(m_pData->dc != 0);
        return ::PtVisible (m_pData->dc, x, y);
    }

    // Moves the clipping region of a device context by the specified offsets.
    // Refer to OffsetClipRgn in the Windows API documentation for more information.
    inline int CDC::OffsetClipRgn(int xOffset, int yOffset) const
    {
        assert(m_pData->dc != 0);
        return ::OffsetClipRgn (m_pData->dc, xOffset, yOffset);
    }

    // Selects the current path as a clipping region for the device context, combining
    // the new region with any existing clipping region using the specified mode.
    // Refer to SelectClipPath in the Windows API documentation for more information.
    inline BOOL CDC::SelectClipPath(int mode) const
    {
        assert(m_pData->dc != 0);
        return ::SelectClipPath(m_pData->dc, mode);
    }

    // The StrokeAndFillPath function closes any open figures in a path,
    // strokes the outline of the path by using the current pen, and fills
    // its interior by using the current brush.
    // Refer to StrokeAndFillPath in the Windows API documentation for more information.
    inline BOOL CDC::StrokeAndFillPath() const
    {
        assert(m_pData->dc != 0);
        return ::StrokeAndFillPath(m_pData->dc);
    }

    // The StrokePath function renders the specified path by using the current pen.
    // Refer to StrokePath in the Windows API documentation for more information.
    inline BOOL CDC::StrokePath() const
    {
        assert(m_pData->dc != 0);
        return ::StrokePath(m_pData->dc);
    }

    // Redefines the current path as the area that would be painted if the path were
    // stroked using the pen currently selected into the device context.
    // Refer to WidenPath in the Windows API documentation for more information.
    inline BOOL CDC::WidenPath() const
    {
        assert(m_pData->dc != 0);
        return ::WidenPath(m_pData->dc);
    }

#endif

    ///////////////////////////////////
    // Point and Line Drawing Functions


    // Returns the current "MoveToEx" position.
    // Refer to MoveToEx in the Windows API documentation for more information.
    inline CPoint CDC::GetCurrentPosition() const
    {
        assert(m_pData->dc != 0);
        CPoint pt;
        ::MoveToEx(m_pData->dc, 0, 0, &pt);
        ::MoveToEx(m_pData->dc, pt.x, pt.y, NULL);
        return pt;
    }

    // Retrieves the red, green, blue (RGB) color value of the pixel at the specified coordinates.
    // Refer to GetPixel in the Windows API documentation for more information.
    inline COLORREF CDC::GetPixel(int x, int y) const
    {
        assert(m_pData->dc != 0);
        return ::GetPixel(m_pData->dc, x, y);
    }

    // Retrieves the red, green, blue (RGB) color value of the pixel at the specified coordinates.
    // Refer to GetPixel in the Windows API documentation for more information.
    inline COLORREF CDC::GetPixel(POINT pt) const
    {
        assert(m_pData->dc != 0);
        return ::GetPixel(m_pData->dc, pt.x, pt.y);
    }

    // Updates the current position to the specified point.
    // Refer to MoveToEx in the Windows API documentation for more information.
    inline CPoint CDC::MoveTo(int x, int y) const
    {
        assert(m_pData->dc != 0);
        return ::MoveToEx(m_pData->dc, x, y, NULL);
    }

    // Updates the current position to the specified point
    // Refer to MoveToEx in the Windows API documentation for more information.
    inline CPoint CDC::MoveTo(POINT pt) const
    {
        assert(m_pData->dc != 0);
        return ::MoveToEx(m_pData->dc, pt.x, pt.y, NULL);
    }

    // Draws a line from the current position up to, but not including, the specified point.
    // Refer to LineTo in the Windows API documentation for more information.
    inline BOOL CDC::LineTo(int x, int y) const
    {
        assert(m_pData->dc != 0);
        return ::LineTo(m_pData->dc, x, y);
    }

    // Draws a line from the current position up to, but not including, the specified point.
    // Refer to LineTo in the Windows API documentation for more information.
    inline BOOL CDC::LineTo(POINT pt) const
    {
        assert(m_pData->dc != 0);
        return ::LineTo(m_pData->dc, pt.x, pt.y);
    }

    // Sets the current foreground mix mode. GDI uses the foreground mix mode to
    // combine pens and interiors of filled objects with the colors already on the screen.
    // Refer to SetROP2 in the Windows API documentation for more information.
    inline int CDC::SetROP2(int drawMode) const
    {
        assert(m_pData->dc != 0);
        return ::SetROP2(m_pData->dc, drawMode);
    }

    // Sets the pixel at the specified coordinates to the specified color.
    // Refer to SetPixel in the Windows API documentation for more information.
    inline COLORREF CDC::SetPixel (int x, int y, COLORREF color) const
    {
        assert(m_pData->dc != 0);
        return ::SetPixel(m_pData->dc, x, y, color);
    }

#ifndef _WIN32_WCE

    // Draws an elliptical arc.
    // Refer to Arc in the Windows API documentation for more information.
    inline BOOL CDC::Arc(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4) const
    {
        assert(m_pData->dc != 0);
        return ::Arc(m_pData->dc, x1, y1, x2, y2, x3, y3, x4, y4);
    }

    // Draws an elliptical arc.
    // Refer to Arc in the Windows API documentation for more information.
    inline BOOL CDC::Arc(const RECT& rc, POINT start, POINT end) const
    {
        assert(m_pData->dc != 0);
        return ::Arc(m_pData->dc, rc.left, rc.top, rc.right, rc.bottom,
            start.x, start.y, end.x, end.y);
    }

    // Draws an elliptical arc.
    // Refer to ArcTo in the Windows API documentation for more information.
    inline BOOL CDC::ArcTo(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4) const
    {
        assert(m_pData->dc != 0);
        return ::ArcTo(m_pData->dc, x1, y1, x2, y2, x3, y3, x4, y4);
    }

    // Draws an elliptical arc.
    // Refer to ArcTo in the Windows API documentation for more information.
    inline BOOL CDC::ArcTo(const RECT& rc, POINT ptStart, POINT ptEnd) const
    {
        assert(m_pData->dc != 0);
        return ::ArcTo (m_pData->dc, rc.left, rc.top, rc.right, rc.bottom,
            ptStart.x, ptStart.y, ptEnd.x, ptEnd.y);
    }

    // Draws a line segment and an arc.
    // Refer to AngleArc in the Windows API documentation for more information.
    inline BOOL CDC::AngleArc(int x, int y, int radius, float startAngle, float sweepAngle) const
    {
        assert(m_pData->dc != 0);
        return ::AngleArc(m_pData->dc, x, y, radius, startAngle, sweepAngle);
    }

    // Closes the figure by drawing a line from the current position to the first point of the figure.
    // Refer to CloseFigure in the Windows API documentation for more information.
    inline BOOL CDC::CloseFigure() const
    {
        assert(m_pData->dc != 0);
        return ::CloseFigure(m_pData->dc);
    }

    // Retrieves the foreground mix mode of the specified device context. The mix mode specifies how
    // the pen or interior color and the color already on the screen are combined to yield a new color.
    // Refer to GetROP2 in the Windows API documentation for more information.
    inline int CDC::GetROP2() const
    {
        assert(m_pData->dc != 0);
        return ::GetROP2(m_pData->dc);
    }

    // Retrieves the current arc direction (AD_COUNTERCLOCKWISE or AD_CLOCKWISE).
    // Refer to GetArcDirection in the Windows API documentation for more information.
    inline int CDC::GetArcDirection() const
    {
        assert(m_pData->dc != 0);
        return ::GetArcDirection(m_pData->dc);
    }

    // Sets the current arc direction (AD_COUNTERCLOCKWISE or AD_CLOCKWISE).
    // Refer to SetArcDirection in the Windows API documentation for more information.
    inline int CDC::SetArcDirection(int arcDirection) const
    {
        assert(m_pData->dc != 0);
        return ::SetArcDirection(m_pData->dc, arcDirection);
    }

    // Draws a set of line segments and Bezier curves.
    // Refer to PolyDraw in the Windows API documentation for more information.
    inline BOOL CDC::PolyDraw(const POINT* pPointArray, const BYTE* pTypes, int count) const
    {
        assert(m_pData->dc != 0);
        return ::PolyDraw(m_pData->dc, pPointArray, pTypes, count);
    }

    // Draws a series of line segments by connecting the points in the specified array.
    // Refer to Polyline in the Windows API documentation for more information.
    inline BOOL CDC::Polyline(LPPOINT pPointArray, int count) const
    {
        assert(m_pData->dc != 0);
        return ::Polyline(m_pData->dc, pPointArray, count);
    }

    // Draws multiple series of connected line segments.
    // Refer to PolyPolyline in the Windows API documentation for more information.
    inline BOOL CDC::PolyPolyline(const POINT* pPointArray, const DWORD* pPolyPoints, int count) const
    {
        assert(m_pData->dc != 0);
        return ::PolyPolyline(m_pData->dc, pPointArray, pPolyPoints, count);
    }

    // Draws one or more straight lines.
    // Refer to PolylineTo in the Windows API documentation for more information.
    inline BOOL CDC::PolylineTo(const POINT* pPointArray, int count) const
    {
        assert(m_pData->dc != 0);
        return ::PolylineTo(m_pData->dc, pPointArray, count);
    }

    // Draws one or more Bezier curves.
    // Refer to PolyBezier in the Windows API documentation for more information.
    inline BOOL CDC::PolyBezier(const POINT* pPointArray, int count) const
    {
        assert(m_pData->dc != 0);
        return ::PolyBezier(m_pData->dc, pPointArray, count);
    }

    // Draws one or more Bezier curves.
    // Refer to PolyBezierTo in the Windows API documentation for more information.
    inline BOOL CDC::PolyBezierTo(const POINT* pPointArray, int count) const
    {
        assert(m_pData->dc != 0);
        return ::PolyBezierTo(m_pData->dc, pPointArray, count );
    }

    // Sets the pixel at the specified coordinates to the specified color.
    // Refer to SetPixel in the Windows API documentation for more information.
    inline COLORREF CDC::SetPixel(POINT pt, COLORREF color) const
    {
        assert(m_pData->dc != 0);
        return ::SetPixel(m_pData->dc, pt.x, pt.y, color);
    }

    // Sets the pixel at the specified coordinates to the closest approximation of the specified color.
    // Refer to SetPixelV in the Windows API documentation for more information.
    inline BOOL CDC::SetPixelV(int x, int y, COLORREF color) const
    {
        assert(m_pData->dc != 0);
        return ::SetPixelV(m_pData->dc, x, y, color);
    }

    // Sets the pixel at the specified coordinates to the closest approximation of the specified color.
    // Refer to SetPixelV in the Windows API documentation for more information.
    inline BOOL CDC::SetPixelV(POINT pt, COLORREF color) const
    {
        assert(m_pData->dc != 0);
        return ::SetPixelV(m_pData->dc, pt.x, pt.y, color);
    }

#endif

    //////////////////////////
    // Shape Drawing Functions


    // Draws a rectangle in the style used to indicate that the rectangle has the focus.
    // Refer to DrawFocusRect in the Windows API documentation for more information.
    inline void CDC::DrawFocusRect(const RECT& rc) const
    {
        assert(m_pData->dc != 0);
        ::DrawFocusRect(m_pData->dc, &rc);
    }

    // Draws an ellipse. The center of the ellipse is the center of the specified bounding rectangle.
    // Refer to Ellipse in the Windows API documentation for more information.
    inline BOOL CDC::Ellipse(int x1, int y1, int x2, int y2) const
    {
        assert(m_pData->dc != 0);
        return ::Ellipse(m_pData->dc, x1, y1, x2, y2);
    }

    // Draws an ellipse. The center of the ellipse is the center of the specified bounding rectangle.
    // Refer to Ellipse in the Windows API documentation for more information.
    inline BOOL CDC::Ellipse(const RECT& rc) const
    {
        assert(m_pData->dc != 0);
        return ::Ellipse(m_pData->dc, rc.left, rc.top, rc.right, rc.bottom);
    }

    // Draws a polygon consisting of two or more vertices connected by straight lines.
    // Refer to Polygon in the Windows API documentation for more information.
    inline BOOL CDC::Polygon(LPPOINT pPointArray, int count) const
    {
        assert(m_pData->dc != 0);
        return ::Polygon(m_pData->dc, pPointArray, count);
    }

    // Draws a rectangle. The rectangle is outlined by using the current pen and filled
    // by using the current brush.
    // Refer to Rectangle in the Windows API documentation for more information.
    inline BOOL CDC::Rectangle(int x1, int y1, int x2, int y2) const
    {
        assert(m_pData->dc != 0);
        return ::Rectangle(m_pData->dc, x1, y1, x2, y2);
    }

    // Draws a rectangle. The rectangle is outlined by using the current pen and filled
    //  by using the current brush.
    // Refer to Rectangle in the Windows API documentation for more information.
    inline BOOL CDC::Rectangle(const RECT& rc) const
    {
        assert(m_pData->dc != 0);
        return ::Rectangle(m_pData->dc, rc.left, rc.top, rc.right, rc.bottom);
    }

    // Draws a rectangle with rounded corners.
    // Refer to RoundRect in the Windows API documentation for more information.
    inline BOOL CDC::RoundRect(int x1, int y1, int x2, int y2, int width, int height) const
    {
        assert(m_pData->dc != 0);
        return ::RoundRect(m_pData->dc, x1, y1, x2, y2, width, height);
    }

    // Draws a rectangle with rounded corners.
    // Refer to RoundRect in the Windows API documentation for more information.
    inline BOOL CDC::RoundRect(const RECT& rc, int width, int height) const
    {
        assert(m_pData->dc != 0);
        return ::RoundRect(m_pData->dc, rc.left, rc.top, rc.right, rc.bottom, width, height );
    }

#ifndef _WIN32_WCE

    // Draws a chord (a region bounded by the intersection of an ellipse and a line segment, called a secant).
    // Refer to Chord in the Windows API documentation for more information.
    inline BOOL CDC::Chord(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4) const
    {
        assert(m_pData->dc != 0);
        return ::Chord(m_pData->dc, x1, y1, x2, y2, x3, y3, x4, y4);
    }

    // Draws a chord (a region bounded by the intersection of an ellipse and a line segment, called a secant).
    // Refer to Chord in the Windows API documentation for more information.
    inline BOOL CDC::Chord(const RECT& rc, POINT start, POINT end) const
    {
        assert(m_pData->dc != 0);
        return ::Chord(m_pData->dc, rc.left, rc.top, rc.right, rc.bottom,
            start.x, start.y, end.x, end.y);
    }

    // Draws a pie-shaped wedge bounded by the intersection of an ellipse and two radials.
    // Refer to Pie in the Windows API documentation for more information.
    inline BOOL CDC::Pie(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4) const
    {
        assert(m_pData->dc != 0);
        return ::Pie(m_pData->dc, x1, y1, x2, y2, x3, y3, x4, y4);
    }

    // Draws a pie-shaped wedge bounded by the intersection of an ellipse and two radials.
    // Refer to Pie in the Windows API documentation for more information.
    inline BOOL CDC::Pie(const RECT& rc, POINT start, POINT end) const
    {
        assert(m_pData->dc != 0);
        return ::Pie(m_pData->dc, rc.left, rc.top, rc.right, rc.bottom,
            start.x, start.y, end.x, end.y);
    }

    // Draws a series of closed polygons.
    // Refer to PolyPolygon in the Windows API documentation for more information.
    inline BOOL CDC::PolyPolygon(LPPOINT pPointArray, LPINT pPolyCounts, int count) const
    {
        assert(m_pData->dc != 0);
        return ::PolyPolygon(m_pData->dc, pPointArray, pPolyCounts, count);
    }

#endif

    ////////////////////////////////
    // Fill and 3D Drawing functions


    // Fills a rectangle by using the specified brush.
    // Refer to FillRect in the Windows API documentation for more information.
    inline BOOL CDC::FillRect(const RECT& rc, HBRUSH brush) const
    {
        assert(m_pData->dc != 0);
        return (::FillRect(m_pData->dc, &rc, brush) != 0);
    }

    // Inverts a rectangle in a window by performing a logical NOT operation on the color
    // values for each pixel in the rectangle's interior.
    // Refer to InvertRect in the Windows API documentation for more information.
    inline BOOL CDC::InvertRect(const RECT& rc) const
    {
        assert(m_pData->dc != 0);
        return ::InvertRect( m_pData->dc, &rc);
    }

    // draws an icon or cursor, performing the specified raster operations, and stretching
    // or compressing the icon or cursor as specified.
    // Refer to DrawIconEx in the Windows API documentation for more information.
    inline BOOL CDC::DrawIconEx(int xLeft, int yTop, HICON icon, int cxWidth, int cyWidth, UINT index, HBRUSH flickerFreeDraw, UINT flags) const
    {
        assert(m_pData->dc != 0);
        return ::DrawIconEx(m_pData->dc, xLeft, yTop, icon, cxWidth, cyWidth, index, flickerFreeDraw, flags);
    }

    // Draws one or more edges of rectangle.
    // Refer to DrawEdge in the Windows API documentation for more information.
    inline BOOL CDC::DrawEdge(const RECT& rc, UINT edge, UINT flags) const
    {
        assert(m_pData->dc != 0);
        return ::DrawEdge(m_pData->dc, (LPRECT)&rc, edge, flags);
    }

    // Draws a frame control of the specified type and style.
    // Refer to DrawFrameControl in the Windows API documentation for more information.
    inline BOOL CDC::DrawFrameControl(const RECT& rc, UINT type, UINT state) const
    {
        assert(m_pData->dc != 0);
        return ::DrawFrameControl(m_pData->dc, (LPRECT)&rc, type, state);
    }

    // Fills a region by using the specified brush.
    // Refer to FillRgn in the Windows API documentation for more information.
    inline BOOL CDC::FillRgn(HRGN rgn, HBRUSH brush) const
    {
        assert(m_pData->dc != 0);
        return ::FillRgn(m_pData->dc, rgn, brush);
    }

  #if (WINVER >= 0x0410)

    // Fills rectangle and triangle structures.
    // Refer to GradientFill in the Windows API documentation for more information.
    inline BOOL CDC::GradientFill(PTRIVERTEX pVertex, ULONG vertex, PVOID pMesh, ULONG mesh, ULONG mode) const
    {
        assert(m_pData->dc != 0);
        return ::GradientFill(m_pData->dc, pVertex, vertex, pMesh, mesh, mode);
    }

  #endif

#ifndef _WIN32_WCE

    // Draws an icon or cursor.
    // Refer to DrawIcon in the Windows API documentation for more information.
    inline BOOL CDC::DrawIcon(int x, int y, HICON icon) const
    {
        assert(m_pData->dc != 0);
        return ::DrawIcon(m_pData->dc, x, y, icon);
    }

    // Draws an icon or cursor.
    // Refer to DrawIcon in the Windows API documentation for more information.
    inline BOOL CDC::DrawIcon(POINT pt, HICON icon) const
    {
        assert(m_pData->dc != 0);
        return ::DrawIcon(m_pData->dc, pt.x, pt.y, icon);
    }

    // Draws a border around the specified rectangle by using the specified brush.
    // Refer to FrameRect in the Windows API documentation for more information.
    inline BOOL CDC::FrameRect(const RECT& rc, HBRUSH brush) const
    {
        assert(m_pData->dc != 0);
        return (::FrameRect(m_pData->dc, &rc, brush) != 0);
    }

    // Draws a border around the specified region by using the specified brush.
    // Refer to FrameRgn in the Windows API documentation for more information.
    inline BOOL CDC::FrameRgn(HRGN rgn, HBRUSH brush, int width, int height) const
    {
        assert(m_pData->dc != 0);
        return (::FrameRgn(m_pData->dc, rgn, brush, width, height) != 0);
    }

    // Retrieves the current polygon fill mode.
    // Refer to GetPolyFillMode in the Windows API documentation for more information.
    inline int CDC::GetPolyFillMode() const
    {
        assert(m_pData->dc != 0);
        return ::GetPolyFillMode(m_pData->dc);
    }

    // Paints the specified region by using the brush currently selected into the device context.
    // Refer to PaintRgn in the Windows API documentation for more information.
    inline BOOL CDC::PaintRgn(HRGN rgn) const
    {
        assert(m_pData->dc != 0);
        return (::PaintRgn(m_pData->dc, rgn) != 0);
    }

    // Sets the polygon fill mode for functions that fill polygons.
    // Refer to SetPolyFillMode in the Windows API documentation for more information.
    inline int CDC::SetPolyFillMode(int polyFillMode) const
    {
        assert(m_pData->dc != 0);
        return ::SetPolyFillMode(m_pData->dc, polyFillMode);
    }

#endif

    ///////////////////
    // Bitmap Functions

    // Copies the color data for a rectangle of pixels in a DIB to the specified destination rectangle.
    // A CBitmapInfoPtr object can be used for the LPBITMAPINFO parameter.
    // Refer to StretchDIBits in the Windows API documentation for more information.
    inline int CDC::StretchDIBits(int xDest, int yDest, int destWidth, int destHeight, int xSrc, int ySrc, int srcWidth,
                   int srcHeight, LPCVOID pBits, const LPBITMAPINFO pBMI, UINT usage, DWORD rop) const
    {
        assert(m_pData->dc != 0);
        return ::StretchDIBits(m_pData->dc, xDest, yDest, destWidth, destHeight, xSrc, ySrc, srcWidth, srcHeight, pBits, pBMI, usage, rop);
    }

    // Paints the specified rectangle using the brush that is currently selected into the device context.
    // Refer to PatBlt in the Windows API documentation for more information.
    inline BOOL CDC::PatBlt(int x, int y, int width, int height, DWORD rop) const
    {
        assert(m_pData->dc != 0);
        return ::PatBlt(m_pData->dc, x, y, width, height, rop);
    }

    // Performs a bit-block transfer of the color data corresponding to a rectangle of pixels
    // from the specified source device context into a destination device context.
    // Refer to BitBlt in the Windows API documentation for more information.
    inline BOOL CDC::BitBlt(int x, int y, int width, int height, HDC hSrc, int xSrc, int ySrc, DWORD rop) const
    {
        assert(m_pData->dc != 0);
        return ::BitBlt(m_pData->dc, x, y, width, height, hSrc, xSrc, ySrc, rop);
    }

    // Combines the color data for the source and destination bitmaps using the specified mask and raster operation.
    //  xDest     x-coord of destination upper-left corner
    //  yDest     y-coord of destination upper-left corner
    //  width     width of source and destination
    //  yeight    height of source and destination
    //  hSrc      pointer to source DC
    //  xSrc      x-coord of upper-left corner of source
    //  ySrc      y-coord of upper-left corner of source
    //  mask      handle to monochrome bit mask
    //  xMask     horizontal offset into mask bitmap
    //  yMask     vertical offset into mask bitmap
    //  rop       raster operation code
    // Refer to MaskBlt in the Windows API documentation for more information.
    inline BOOL CDC::MaskBlt(int xDest, int yDest, int width, int height, HDC hSrc, int xSrc, int ySrc, HBITMAP mask, int xMask, int yMask, DWORD rop) const
    {
        assert(m_pData->dc != 0);
        return ::MaskBlt(m_pData->dc, xDest, yDest, width, height, hSrc, xSrc, ySrc, mask, xMask, yMask, rop);
    }

    // Copies a bitmap from a source rectangle into a destination rectangle, stretching or compressing
    // the bitmap to fit the dimensions of the destination rectangle, if necessary.
    //  x            x-coord of destination upper-left corner
    //  y            y-coord of destination upper-left corner
    //  width        width of destination rectangle
    //  height       height of destination rectangle
    //  hSrc         handle to source DC
    //  xSrc         x-coord of source upper-left corner
    //  ySrc         y-coord of source upper-left corner
    //  srcWidth     width of source rectangle
    //  srcHeight    height of source rectangle
    //  rop          raster operation code
    // Refer to StretchBlt in the Windows API documentation for more information.
    inline BOOL CDC::StretchBlt(int x, int y, int width, int height, HDC src, int xSrc, int ySrc, int srcWidth, int srcHeight, DWORD rop) const
    {
        assert(m_pData->dc != 0);
        return ::StretchBlt(m_pData->dc, x, y, width, height, src, xSrc, ySrc, srcWidth, srcHeight, rop);
    }

#ifndef _WIN32_WCE

    // Retrieves the bits of the specified compatible bitmap and copies them into a buffer as a DIB using the specified format.
    // A CBitmapInfoPtr object can be used for the LPBITMAPINFO parameter.
    // Refer to GetDIBits in the Windows API documentation for more information.
    inline int CDC::GetDIBits(HBITMAP bitmap, UINT startScan, UINT scanLines, LPVOID pBits, LPBITMAPINFO pBMI, UINT usage) const
    {
        assert(m_pData->dc != 0);
        return ::GetDIBits(m_pData->dc, bitmap, startScan, scanLines, pBits, pBMI, usage);
    }

    // Sets the pixels in a compatible bitmap (DDB) using the color data found in the specified DIB.
    // A CBitmapInfoPtr object can be used for the LPBITMAPINFO parameter.
    // Refer to SetDIBits in the Windows API documentation for more information.
    inline int CDC::SetDIBits(HBITMAP bitmap, UINT startScan, UINT scanLines, LPCVOID pBits, LPBITMAPINFO pBMI, UINT colorUse) const
    {
        assert(m_pData->dc != 0);
        return ::SetDIBits(m_pData->dc, bitmap, startScan, scanLines, pBits, pBMI, colorUse);
    }

    // Retrieves the current stretching mode.
    // Possible modes: BLACKONWHITE, COLORONCOLOR, HALFTONE, STRETCH_ANDSCANS, STRETCH_DELETESCANS,
    //                 STRETCH_HALFTONE, STRETCH_ORSCANS, WHITEONBLACK
    // Refer to GetStretchBltMode in the Windows API documentation for more information.
    inline int CDC::GetStretchBltMode() const
    {
        assert(m_pData->dc != 0);
        return ::GetStretchBltMode(m_pData->dc);
    }

    // Sets the stretching mode.
    // Possible modes: BLACKONWHITE, COLORONCOLOR, HALFTONE, STRETCH_ANDSCANS, STRETCH_DELETESCANS,
    //                 STRETCH_HALFTONE, STRETCH_ORSCANS, WHITEONBLACK
    // Refer to SetStretchBltMode in the Windows API documentation for more information.
    inline int CDC::SetStretchBltMode(int stretchMode) const
    {
        assert(m_pData->dc != 0);
        return ::SetStretchBltMode(m_pData->dc, stretchMode);
    }

#if (WINVER >= 0x0410)

    // Performs a bit-block transfer of the color data corresponding to a rectangle
    // of pixels from the specified source device context into a destination device context.
    //  x             x-coord of destination upper-left corner
    //  y             y-coord of destination upper-left corner
    //  width         width of destination rectangle
    //  height        height of destination rectangle
    //  hSrc          handle to source DC
    //  xSrc          x-coord of source upper-left corner
    //  ySrc          y-coord of source upper-left corner
    //  widthSrc      width of source rectangle
    //  heightSrc     height of source rectangle
    //  transparent   color to make transparent
    // Refer to TransparentBlt in the Windows API documentation for more information.
    inline BOOL CDC::TransparentBlt(int x, int y, int width, int height, HDC hSrc, int xSrc, int ySrc,
                                     int widthSrc, int heightSrc, UINT transparent) const
    {
        assert(m_pData->dc != 0);
        return ::TransparentBlt(m_pData->dc, x, y, width, height, hSrc, xSrc, ySrc, widthSrc, heightSrc, transparent);
    }

#endif

    // Fills an area of the display surface with the current brush.
    // Refer to FloodFill in the Windows API documentation for more information.
    inline BOOL CDC::FloodFill(int x, int y, COLORREF color) const
    {
        assert(m_pData->dc != 0);
        return ::FloodFill(m_pData->dc, x, y, color);
    }

    // Fills an area of the display surface with the current brush.
    // Fill type: FLOODFILLBORDER or FLOODFILLSURFACE
    // Refer to ExtFloodFill in the Windows API documentation for more information.
    inline BOOL CDC::ExtFloodFill(int x, int y, COLORREF color, UINT fillType) const
    {
        assert(m_pData->dc != 0);
        return ::ExtFloodFill(m_pData->dc, x, y, color, fillType );
    }

#endif

#ifndef _WIN32_WCE

    ////////////////////////
    // co-ordinate functions

    // Converts device coordinates into logical coordinates.
    // Refer to DPtoLP in the Windows API documentation for more information.
    inline BOOL CDC::DPtoLP(LPPOINT pPointArray, int count) const
    {
        assert(m_pData->dc != 0);
        return ::DPtoLP(m_pData->dc, pPointArray, count);
    }

    // Converts device coordinates into logical coordinates.
    // Refer to DPtoLP in the Windows API documentation for more information.
    inline BOOL CDC::DPtoLP(RECT& rc) const
    {
        assert(m_pData->dc != 0);
        return ::DPtoLP(m_pData->dc, (LPPOINT)&rc, 2);
    }

    // Converts logical coordinates into device coordinates.
    // Refer to LPtoDP in the Windows API documentation for more information.
    inline BOOL CDC::LPtoDP(LPPOINT pPointArray, int count) const
    {
        assert(m_pData->dc != 0);
        return ::LPtoDP(m_pData->dc, pPointArray, count);
    }

    // Converts logical coordinates into device coordinates.
    // Refer to LPtoDP in the Windows API documentation for more information.
    inline BOOL CDC::LPtoDP(RECT& rc) const
    {
        assert(m_pData->dc != 0);
        return ::LPtoDP(m_pData->dc, (LPPOINT)&rc, 2);
    }

#endif

    ///////////////////
    // Layout Functions


    // Returns the layout of a device context (LAYOUT_RTL and LAYOUT_BITMAPORIENTATIONPRESERVED).
    // Refer to GetLayout in the Windows API documentation for more information.
    inline DWORD CDC::GetLayout() const
    {
#if (WINVER >= 0x0500)
        return ::GetLayout(m_pData->dc);
#else
        return 0;
#endif
    }


    // changes the layout of a device context (DC).
    // layout values:  LAYOUT_RTL or LAYOUT_BITMAPORIENTATIONPRESERVED
    // Refer to SetLayout in the Windows API documentation for more information.
    inline DWORD CDC::SetLayout(DWORD layout) const
    {
#if (WINVER >= 0x0500)
        // Sets the layout of a device context
        return ::SetLayout(m_pData->dc, layout);
#else
        UNREFERENCED_PARAMETER(layout); // no-op
        return 0;
#endif
    }

    ////////////////////
    // Mapping Functions


#ifndef _WIN32_WCE

    // Retrieves the current mapping mode.
    // Possible modes: MM_ANISOTROPIC, MM_HIENGLISH, MM_HIMETRIC, MM_ISOTROPIC, MM_LOENGLISH, MM_LOMETRIC, MM_TEXT, and MM_TWIPS.
    // Refer to GetMapMode in the Windows API documentation for more information.
    inline int CDC::GetMapMode()  const
    {
        assert(m_pData->dc != 0);
        return ::GetMapMode(m_pData->dc);
    }

    // Retrieves the x-coordinates and y-coordinates of the viewport origin for the device context.
    // Refer to GetViewportOrgEx in the Windows API documentation for more information.
    inline BOOL CDC::GetViewportOrgEx(LPPOINT pPoint)  const
    {
        assert(m_pData->dc != 0);
        return ::GetViewportOrgEx(m_pData->dc, pPoint);
    }

    // Sets the mapping mode of the specified device context.
    // Refer to SetMapMode in the Windows API documentation for more information.
    inline int CDC::SetMapMode(int mapMode) const
    {
        assert(m_pData->dc != 0);
        return ::SetMapMode(m_pData->dc, mapMode);
    }

    // Specifies which device point maps to the window origin (0,0).
    // Refer to SetViewportOrgEx in the Windows API documentation for more information.
    inline BOOL CDC::SetViewportOrgEx(int x, int y, LPPOINT pPoint /* = NULL */) const
    {
        assert(m_pData->dc != 0);
        return ::SetViewportOrgEx(m_pData->dc, x, y, pPoint);
    }

    // Specifies which device point maps to the window origin (0,0).
    // Refer to SetViewportOrgEx in the Windows API documentation for more information.
    inline BOOL CDC::SetViewportOrgEx(POINT point, LPPOINT pPointRet /* = NULL */) const
    {
        assert(m_pData->dc != 0);
        return SetViewportOrgEx(point.x, point.y, pPointRet);
    }

    // Modifies the viewport origin for the device context using the specified horizontal and vertical offsets.
    // Refer to OffsetViewportOrgEx in the Windows API documentation for more information.
    inline BOOL CDC::OffsetViewportOrgEx(int width, int height, LPPOINT pPoint /* = NULL */) const
    {
        assert(m_pData->dc != 0);
        return ::OffsetViewportOrgEx(m_pData->dc, width, height, pPoint);
    }

    // Retrieves the x-extent and y-extent of the current viewport for the device context.
    // Refer to GetViewportExtEx in the Windows API documentation for more information.
    inline BOOL CDC::GetViewportExtEx(LPSIZE pSize)  const
    {
        assert(m_pData->dc != 0);
        return ::GetViewportExtEx(m_pData->dc, pSize);
    }

    // Sets the horizontal and vertical extents of the viewport for the device context by using the specified values.
    // Refer to SetViewportExtEx in the Windows API documentation for more information.
    inline BOOL CDC::SetViewportExtEx(int x, int y, LPSIZE pSize ) const
    {
        assert(m_pData->dc != 0);
        return ::SetViewportExtEx(m_pData->dc, x, y, pSize);
    }

    // Sets the horizontal and vertical extents of the viewport for the device context by using the specified values.
    // Refer to SetViewportExtEx in the Windows API documentation for more information.
    inline BOOL CDC::SetViewportExtEx(SIZE size, LPSIZE pSizeRet ) const
    {
        assert(m_pData->dc != 0);
        return SetViewportExtEx(size.cx, size.cy, pSizeRet);
    }

    // Modifies the viewport for the device context using the ratios formed by the specified multiplicands and divisors.
    // Refer to ScaleViewportExtEx in the Windows API documentation for more information.
    inline BOOL CDC::ScaleViewportExtEx(int xNum, int xDenom, int yNum, int yDenom, LPSIZE pSize ) const
    {
        assert(m_pData->dc != 0);
        return ::ScaleViewportExtEx(m_pData->dc, xNum, xDenom, yNum, yDenom, pSize);
    }

    // Retrieves the x-coordinates and y-coordinates of the window origin for the device context.
    // Refer to GetWindowOrgEx in the Windows API documentation for more information.
    inline BOOL CDC::GetWindowOrgEx(LPPOINT pPoint) const
    {
        assert(m_pData->dc != 0);
        return ::GetWindowOrgEx(m_pData->dc, pPoint);
    }

    // Specifies which window point maps to the viewport origin (0,0).
    // Refer to SetWindowOrgEx in the Windows API documentation for more information.
    inline BOOL CDC::SetWindowOrgEx(int x, int y, LPPOINT pPoint ) const
    {
        assert(m_pData->dc != 0);
        return ::SetWindowOrgEx(m_pData->dc, x, y, pPoint);
    }

    // Specifies which window point maps to the viewport origin (0,0).
    // Refer to SetWindowOrgEx in the Windows API documentation for more information.
    inline BOOL CDC::SetWindowOrgEx(POINT point, LPPOINT pPointRet ) const
    {
        assert(m_pData->dc != 0);
        return SetWindowOrgEx(point.x, point.y, pPointRet);
    }

    // Modifies the window origin for the device context using the specified horizontal and vertical offsets.
    // Refer to OffsetWindowOrgEx in the Windows API documentation for more information.
    inline BOOL CDC::OffsetWindowOrgEx(int width, int height, LPPOINT pPoint ) const
    {
        assert(m_pData->dc != 0);
        return ::OffsetWindowOrgEx(m_pData->dc, width, height, pPoint);
    }

    // Retrieves the x-extent and y-extent of the window for the device context.
    // Refer to GetWindowExtEx in the Windows API documentation for more information.
    inline BOOL CDC::GetWindowExtEx(LPSIZE pSize)  const
    {
        assert(m_pData->dc != 0);
        return ::GetWindowExtEx(m_pData->dc, pSize);
    }

    // Sets the horizontal and vertical extents of the window for the device context by using the specified values.
    // Refer to SetWindowExtEx in the Windows API documentation for more information.
    inline BOOL CDC::SetWindowExtEx(int x, int y, LPSIZE pSize ) const
    {
        assert(m_pData->dc != 0);
        return ::SetWindowExtEx(m_pData->dc, x, y, pSize);
    }

    // Sets the horizontal and vertical extents of the window for the device context by using the specified values.
    // Refer to SetWindowExtEx in the Windows API documentation for more information.
    inline BOOL CDC::SetWindowExtEx(SIZE size, LPSIZE pSizeRet) const
    {
        assert(m_pData->dc != 0);
        return SetWindowExtEx(size.cx, size.cy, pSizeRet);
    }

    // Modifies the window for the device context using the ratios formed by the specified multiplicands and divisors.
    // Refer to ScaleWindowExtEx in the Windows API documentation for more information.
    inline BOOL CDC::ScaleWindowExtEx(int xNum, int xDenom, int yNum, int yDenom, LPSIZE pSize) const
    {
        assert(m_pData->dc != 0);
        return ::ScaleWindowExtEx(m_pData->dc, xNum, xDenom, yNum, yDenom, pSize);
    }


    /////////////////////
    // MetaFile Functions


    // Displays the picture stored in the specified metafile.
    // Refer to PlayMetaFile in the Windows API documentation for more information.
    inline BOOL CDC::PlayMetaFile(HMETAFILE metaFile) const
    {
        assert(m_pData->dc != 0);
        return ::PlayMetaFile(m_pData->dc, metaFile);
    }

    // Displays the picture stored in the specified enhanced-format metafile.
    // Refer to PlayEnhMetaFile in the Windows API documentation for more information.
    inline BOOL CDC::PlayMetaFile(HENHMETAFILE enhMetaFile, const RECT& bounds) const
    {
        assert(m_pData->dc != 0);
        return ::PlayEnhMetaFile(m_pData->dc, enhMetaFile, &bounds);
    }

#endif // _WIN32_WCE

    ////////////////////
    // Printer Functions


    // Starts a print job.
    // Refer to StartDoc in the Windows API documentation for more information.
    inline int CDC::StartDoc(LPDOCINFO pDocInfo) const
    {
        assert(m_pData->dc != 0);
        return ::StartDoc(m_pData->dc, pDocInfo);
    }

    // Ends a print job.
    // Refer to EndDoc in the Windows API documentation for more information.
    inline int CDC::EndDoc() const
    {
        assert(m_pData->dc != 0);
        return ::EndDoc(m_pData->dc);
    }

    // Prepares the printer driver to accept data.
    // Refer to StartPage in the Windows API documentation for more information.
    inline int CDC::StartPage() const
    {
        assert(m_pData->dc != 0);
        return ::StartPage(m_pData->dc);
    }

    // Notifies the device that the application has finished writing to a page.
    // Refer to EndPage in the Windows API documentation for more information.
    inline int CDC::EndPage() const
    {
        assert(m_pData->dc != 0);
        return ::EndPage(m_pData->dc);
    }

    // Stops the current print job and erases everything drawn since the last call to the StartDoc function.
    // Refer to AbortDoc in the Windows API documentation for more information.
    inline int CDC::AbortDoc() const
    {
        assert(m_pData->dc != 0);
        return ::AbortDoc(m_pData->dc);
    }

    // Sets the application-defined abort function that allows a print job to be cancelled during spooling.
    // Refer to SetAbortProc in the Windows API documentation for more information.
    inline int CDC::SetAbortProc(BOOL (CALLBACK* pfn)(HDC, int)) const
    {
        assert(m_pData->dc != 0);
        return ::SetAbortProc(m_pData->dc, pfn);
    }


    /////////////////
    // Text Functions

    // Draws text using the currently selected font, background color, and text color.
    // Refer to ExtTextOut in the Windows API documentation for more information.
    inline BOOL CDC::ExtTextOut(int x, int y, UINT options, const RECT& rc, LPCTSTR string, int count /*= -1*/, LPINT pDxWidths /*=NULL*/) const
    {
        assert(m_pData->dc != 0);

        if (count == -1)
            count = lstrlen (string);

        return ::ExtTextOut(m_pData->dc, x, y, options, &rc, string, count, pDxWidths);
    }

    // Draws formatted text in the specified rectangle.
    // Refer to DrawText in the Windows API documentation for more information.
    inline int CDC::DrawText(LPCTSTR string, int count, const RECT& rc, UINT format) const
    {
        assert(m_pData->dc != 0);
        return ::DrawText(m_pData->dc, string, count, (LPRECT)&rc, format );
    }

    // Retrieves the text-alignment setting.
    // Return values: TA_BASELINE, TA_BOTTOM, TA_TOP, TA_CENTER, TA_LEFT, TA_RIGHT, TA_RTLREADING, TA_NOUPDATECP, TA_UPDATECP.
    // Refer to GetTextAlign in the Windows API documentation for more information.
    inline UINT CDC::GetTextAlign() const
    {
        assert(m_pData->dc != 0);
        return ::GetTextAlign(m_pData->dc);
    }

    // Sets the text-alignment setting.
    // Return values: TA_BASELINE, TA_BOTTOM, TA_TOP, TA_CENTER, TA_LEFT, TA_RIGHT, TA_RTLREADING, TA_NOUPDATECP, TA_UPDATECP.
    // Refer to SetTextAlign in the Windows API documentation for more information.
    inline UINT CDC::SetTextAlign(UINT flags) const
    {
        assert(m_pData->dc != 0);
        return ::SetTextAlign(m_pData->dc, flags);
    }

    // Retrieves the typeface name of the font that is selected into the device context.
    // Refer to GetTextFace in the Windows API documentation for more information.
    inline int CDC::GetTextFace(int count, LPTSTR faceName) const
    {
        assert(m_pData->dc != 0);
        return ::GetTextFace(m_pData->dc, count, faceName);
    }

    // Fills the specified buffer with the metrics for the currently selected font.
    // Refer to GetTextMetrics in the Windows API documentation for more information.
    inline BOOL CDC::GetTextMetrics(TEXTMETRIC& metrics) const
    {
        assert(m_pData->dc != 0);
        return ::GetTextMetrics(m_pData->dc, &metrics);
    }

    // Returns the current background color.
    // Refer to GetBkColor in the Windows API documentation for more information.
    inline COLORREF CDC::GetBkColor() const
    {
        assert(m_pData->dc != 0);
        return ::GetBkColor(m_pData->dc);
    }

    // Sets the current background color to the specified color value.
    // Refer to SetBkColor in the Windows API documentation for more information.
    inline COLORREF CDC::SetBkColor(COLORREF color) const
    {
        assert(m_pData->dc != 0);
        return ::SetBkColor(m_pData->dc, color);
    }

    // Retrieves the current text color.
    // Refer to GetTextColor in the Windows API documentation for more information.
    inline COLORREF CDC::GetTextColor() const
    {
        assert(m_pData->dc != 0);
        return ::GetTextColor(m_pData->dc);
    }

    // Sets the current text color.
    // Refer to SetTextColor in the Windows API documentation for more information.
    inline COLORREF CDC::SetTextColor(COLORREF color) const
    {
        assert(m_pData->dc != 0);
        return ::SetTextColor(m_pData->dc, color);
    }

    // returns the current background mix mode (OPAQUE or TRANSPARENT).
    // Refer to GetBkMode in the Windows API documentation for more information.
    inline int CDC::GetBkMode() const
    {
        assert(m_pData->dc != 0);
        return ::GetBkMode(m_pData->dc);
    }

    // Sets the current background mix mode (OPAQUE or TRANSPARENT).
    // Refer to SetBkMode in the Windows API documentation for more information.
    inline int CDC::SetBkMode(int bkMode) const
    {
        assert(m_pData->dc != 0);
        return ::SetBkMode(m_pData->dc, bkMode);
    }

#ifndef _WIN32_WCE

    // Draws formatted text in the specified rectangle with more formatting options.
    // Refer to DrawTextEx in the Windows API documentation for more information.
    inline int CDC::DrawTextEx(LPTSTR string, int count, const RECT& rc, UINT format, LPDRAWTEXTPARAMS pDTParams) const
    {
        assert(m_pData->dc != 0);
        return ::DrawTextEx(m_pData->dc, string, count, (LPRECT)&rc, format, pDTParams);
    }

    // Retrieves the widths, in logical units, of consecutive characters in a specified range from the
    // current TrueType font. This function succeeds only with TrueType fonts.
    // Refer to GetCharABCWidths in the Windows API documentation for more information.
    inline BOOL CDC::GetCharABCWidths(UINT firstChar, UINT lastChar, LPABC pABC) const
    {
        assert(m_pData->dc != 0);
        return::GetCharABCWidths(m_pData->dc, firstChar, lastChar, pABC);
    }

    // Retrieves information about a character string, such as character widths, caret positioning,
    // ordering within the string, and glyph rendering.
    // Refer to GetCharacterPlacement in the Windows API documentation for more information.
    inline DWORD CDC::GetCharacterPlacement(LPCTSTR string, int count, int maxExtent, LPGCP_RESULTS pResults, DWORD flags) const
    {
        assert(m_pData->dc != 0);
        return ::GetCharacterPlacement(m_pData->dc, string, count, maxExtent, pResults, flags);
    }

    // Retrieves the fractional widths of consecutive characters in a specified range from the current font.
    // Refer to GetCharWidthFloat in the Windows API documentation for more information.
    inline BOOL CDC::GetCharWidth(UINT firstChar, UINT lastChar, float* pBuffer) const
    {
        assert(m_pData->dc != 0);
        return ::GetCharWidthFloat(m_pData->dc, firstChar, lastChar, pBuffer);
    }

    // Computes the width and height of the specified string of text.
    // Refer to GetTextExtentPoint32 in the Windows API documentation for more information.
    inline CSize CDC::GetTextExtentPoint32(LPCTSTR string, int count) const
    {
        assert(m_pData->dc != 0);
        assert(string != 0);
        assert(count <= lstrlen(string));
        CSize sz;
        ::GetTextExtentPoint32(m_pData->dc, string, count, &sz);
        return sz;
    }

    // Computes the width and height of the specified string of text.
    // Refer to GetTextExtentPoint32 in the Windows API documentation for more information.
    inline CSize CDC::GetTextExtentPoint32(LPCTSTR string) const
    {
        assert(string != 0);
        return GetTextExtentPoint32(string, lstrlen(string));
    }

    // Computes the width and height of a character string.
    // Refer to GetTabbedTextExtent in the Windows API documentation for more information.
    inline CSize CDC::GetTabbedTextExtent(LPCTSTR string, int count, int tabPositions, LPINT pTabStopPositions) const
    {
        assert(m_pData->dc != 0);
        DWORD size = ::GetTabbedTextExtent(m_pData->dc, string, count, tabPositions, pTabStopPositions);
        CSize sz(size);
        return sz;
    }

    // Draws gray text at the specified location.
    // Refer to GrayString in the Windows API documentation for more information.
    inline BOOL CDC::GrayString(HBRUSH brush, GRAYSTRINGPROC pOutputFunc, LPARAM pData, int count, int x, int y, int width, int height) const
    {
        assert(m_pData->dc != 0);
        return ::GrayString(m_pData->dc, brush, pOutputFunc, pData, count, x, y, width, height);
    }

    // Specifies the amount of space the system should add to the break characters in a string of text.
    // Refer to SetTextJustification in the Windows API documentation for more information.
    inline int CDC::SetTextJustification(int breakExtra, int breakCount) const
    {
        assert(m_pData->dc != 0);
        return ::SetTextJustification(m_pData->dc, breakExtra, breakCount);
    }

    // Retrieves the current inter-character spacing for the device context.
    // Refer to GetTextCharacterExtra in the Windows API documentation for more information.
    inline int CDC::GetTextCharacterExtra() const
    {
        assert(m_pData->dc != 0);
        return ::GetTextCharacterExtra(m_pData->dc);
    }

    // Sets the inter-character spacing.
    // Refer to SetTextCharacterExtra in the Windows API documentation for more information.
    inline int CDC::SetTextCharacterExtra(int charExtra) const
    {
        assert(m_pData->dc != 0);
        return ::SetTextCharacterExtra(m_pData->dc, charExtra);
    }

    // Writes a character string at a specified location, expanding tabs to the values.
    // specified in an array of tab-stop positions.
    // Refer to TabbedTextOut in the Windows API documentation for more information.
    inline CSize CDC::TabbedTextOut(int x, int y, LPCTSTR string, int count, int tabPositions, LPINT pTabStopPositions, int tabOrigin) const
    {
        assert(m_pData->dc != 0);
        DWORD size = ::TabbedTextOut(m_pData->dc, x, y, string, count, tabPositions, pTabStopPositions, tabOrigin);
        CSize sz(size);
        return sz;
    }

    // Writes a character string at the specified location.
    // Refer to TextOut in the Windows API documentation for more information.
    inline BOOL CDC::TextOut(int x, int y, LPCTSTR string, int count/* = -1*/) const
    {
        assert(m_pData->dc != 0);
        if (count == -1)
            count = lstrlen (string);

        return ::TextOut(m_pData->dc, x, y, string, count);
    }

  #if (_WIN32_WINNT >= 0x0500) && !defined(__GNUC__)

    // Retrieves the widths, in logical units, of consecutive glyph indices in a specified range from the
    // current TrueType font. This function succeeds only with TrueType fonts.
    // Refer to GetCharABCWidthsI in the Windows API documentation for more information.
    inline BOOL CDC::GetCharABCWidthsI(UINT giFirst, UINT cgi, LPWORD pGI, LPABC pABC) const
    {
        assert(m_pData->dc != 0);
        return ::GetCharABCWidthsI(m_pData->dc, giFirst, cgi, pGI, pABC);
    }

    // Retrieves the widths, in logical coordinates, of consecutive glyph indices in a specified range from the current font.
    // Refer to GetCharWidthI in the Windows API documentation for more information.
    inline BOOL CDC::GetCharWidthI(UINT giFirst, UINT cgi, LPWORD pGI, int* pBuffer) const
    {
        assert(m_pData->dc != 0);
        return ::GetCharWidthI(m_pData->dc, giFirst, cgi, pGI, pBuffer);
    }

  #endif

#endif


} // namespace Win32xx

#endif // _WIN32XX_GDI_H_

