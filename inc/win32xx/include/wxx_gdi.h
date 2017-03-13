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
//  automatically delete the GDI resource assigned to them when their destructor
//  is called. These wrapper class objects can be attached to the CDC as
//  shown below.
//
// Coding Example without CDC ...
//  void DrawLine()
//  {
//	  HDC hdcClient = ::GetDC(GetHwnd());
//    HDC hdcMem = ::CreateCompatibleDC(hdcClient);
//    HBITMAP hBitmap = ::CreateCompatibleBitmap(hdcClient, cx, cy);
//	  HBITMAP hOldBitmap = (HBITMAP)::SelectObject(hdcMem, hBitmap);
//    HPEN hPen = ::CreatePen(PS_SOLID, 1, RGB(255,0,0);
//    HPEN hOldPen = (HPEN)::SelectObject(hdcMem, hPen);
//	  ::MoveToEx(hdcMem, 0, 0, NULL);
//    ::LineTo(hdcMem, 50, 50);
//    ::SelectObject(hdcMem, hOldPen);
//    ::DeleteObject(hPen);
//    hPen = ::CreatePen(PS_SOLID, 1, RGB(0,255,0);
//    hOldPen = (HPEN)::SelectObject(hdcMem, hPen);
//    ::LineTo(hdcMem, 80, 80);
//	  ::BitBlt(hdcClient, 0, 0, cx, cy, hdcMem, 0, 0);
//    ::SelectObject(hdcMem, hOldPen);
//    ::DeleteObject(hPen);
//    ::SelectObject(hdcMem, hOldBitmap);
//    ::DeleteObject(hBitmap);
//    ::DeleteDC(hdcMem);
//	  ::ReleaseDC(GetHwnd(), hdcClient);
//  }
//
// Coding Example with CDC classes ...
//  void DrawLine()
//  {
//	  CClientDC dcClient(*this)
//    CMemDC dcMem(dcClient);
//	  dcMem.CreateCompatibleBitmap(dcClient, cx, cy);
//    dcMem.CreatePen(PS_SOLID, 1, RGB(255,0,0);
//	  dcMem.MoveTo(0, 0);
//    dcMem.LineTo(50, 50);
//    dcMem.CreatePen(PS_SOLID, 1, RGB(0,255,0));
//    dcMem.LineTo(80,80);
//	  dcClient.BitBlt(0, 0, cx, cy, dcMem, 0, 0);
//  }
//
// Coding Example with CDC classes and CPen ...
//  void DrawLine()
//  {
//	  CClientDC dcClient(*this)
//    CMemDC dcMem(dcClient);
//	  dcMem.CreateCompatibleBitmap(dcClient, cx, cy);
//    CPen MyPen(PS_SOLID, 1, RGB(255,0,0));
//    CPen OldPen = dcMem.SelectObject(MyPen);
//	  dcMem.MoveTo(0, 0);
//    dcMem.LineTo(50, 50);
//
//    // Only need to select the OldPen if MyPen is changed while its selected into dcMem.
//    dcMem.SelectObject(OldPen);
//
//    // Change MyPen and then re-select it into dcMem
//    MyPen.CreatePen(PS_SOLID, 1, RGB(0,255,0));
//    dcMem.SelectObject(MyPen);
//
//    dcMem.LineTo(80,80);
//	  dcClient.BitBlt(0, 0, cx, cy, dcMem, 0, 0);	 
//  }

// Notes:
//  * When the CDC object drops out of scope, its destructor is called, releasing
//     or deleting the device context as appropriate.
//  * When the destructor for CBitmap, CBrush, CPalette, CPen and CRgn are called,
//     the destructor is called deleting their GDI object.
//  * When the CDC object's destructor is called, any GDI objects created by one of
//     the CDC member functions (CDC::CreatePen for example) will be deleted.
//  * Bitmaps can only be selected into one device context at a time.
//  * Other GDI resources can be selected into more than one device context at a time.
//  * Palettes use SelectPalatte to select them into device the context.
//  * Regions use SelectClipRgn to select them into the device context.
//  * All the GDI classes are reference counted. This allows functions to safely
//     pass these objects by value, as well as by pointer or by reference.
//  * If SelectObject is used to select say a CPen into a device context, the 
//     CPen shouldn't be changed while device context is valid. Use SelectObject to
//     select the old pen back into the device context before changing the pen.

// The CBitmapInfoPtr class is a convenient wrapper for the BITMAPINFO structure.
// The size of the BITMAPINFO structure is dependant on the type of HBITMAP, and its
// space needs to be allocated dynamically. CBitmapInfoPtr automatically allocates
// and deallocates the memory for the structure. A CBitmapInfoPtr object can be
// used anywhere in place of a LPBITMAPINFO. LPBITMAPINFO is used in functions like
// GetDIBits and SetDIBits.
//
// Coding example ...
//  CDC dcMem = CreateCompatibleDC(NULL);
//  CBitmapInfoPtr pbmi(hBitmap);
//  dcMem.GetDIBits(hBitmap, 0, pbmi->bmiHeader.biHeight, NULL, pbmi, DIB_RGB_COLORS);


#ifndef _WIN32XX_GDI_H_
#define _WIN32XX_GDI_H_


#include "wxx_appcore0.h"
#include "wxx_wincore0.h"
#include "wxx_exception.h"
#include "wxx_metafile.h"


// Disable macros from Windowsx.h
#undef CopyRgn

namespace Win32xx
{

	///////////////////////////////////////////////
	// Declarations for the CGDIObject class
	//
	class CGDIObject
	{
	public:
		CGDIObject();
		CGDIObject(const CGDIObject& rhs);
		virtual ~CGDIObject();
		CGDIObject& operator = (const CGDIObject& rhs);
		void operator = (const HGDIOBJ hObject);

		void	Attach(HGDIOBJ hObject);
		void	DeleteObject();
		HGDIOBJ Detach();
		HGDIOBJ GetHandle() const;
		int		GetObject(int nCount, LPVOID pObject) const;

	protected:
		void	Release();
		void SetManaged(bool IsManaged) const { m_pData->IsManagedObject = IsManaged; }

	private:
		void	AddToMap();
		BOOL	RemoveFromMap();

		CGDI_Data* m_pData;
	};


	///////////////////////////////////////////////
	// Declarations for the CBitmap class
	//
	class CBitmap : public CGDIObject
	{

	  public:
		CBitmap();
		CBitmap(HBITMAP hBitmap);
		CBitmap(LPCTSTR lpstr);
		CBitmap(int nID);
		operator HBITMAP() const;
		virtual ~CBitmap();

		// Create and load methods
		BOOL LoadBitmap(LPCTSTR lpszName);
		BOOL LoadBitmap(int nID);
		BOOL LoadImage(LPCTSTR lpszName, UINT fuLoad);
		BOOL LoadImage(UINT nID, UINT fuLoad);
		BOOL LoadOEMBitmap(UINT nIDBitmap);
		HBITMAP CreateBitmap(int nWidth, int nHeight, UINT nPlanes, UINT nBitsPerPixel, LPCVOID pBits);
		HBITMAP CreateCompatibleBitmap(HDC hdc, int nWidth, int nHeight);
		HBITMAP CreateDIBSection(HDC hdc, const LPBITMAPINFO pbmi, UINT uColorUse, LPVOID* pBits, HANDLE hSection, DWORD dwOffset);

#ifndef _WIN32_WCE
		HBITMAP CreateDIBitmap(HDC hdc, const BITMAPINFOHEADER* pbmih, DWORD dwInit, LPCVOID pbInit, const LPBITMAPINFO pbmi, UINT uColorUse);
		HBITMAP CreateMappedBitmap(UINT nIDBitmap, UINT nFlags = 0, LPCOLORMAP lpColorMap = NULL, int nMapSize = 0);
		HBITMAP CreateBitmapIndirect(const BITMAP& Bitmap);
		void GrayScaleBitmap();
		void TintBitmap (int cRed, int cGreen, int cBlue);
		int GetDIBits(HDC hdc, UINT uStartScan, UINT cScanLines,  LPVOID pBits, LPBITMAPINFO pbmi, UINT uColorUse) const;
		int SetDIBits(HDC hdc, UINT uStartScan, UINT cScanLines, LPCVOID pBits, const LPBITMAPINFO pbmi, UINT uColorUse) const;
		CSize GetBitmapDimensionEx() const;
		CSize SetBitmapDimensionEx(int nWidth, int nHeight) const;
#endif // !_WIN32_WCE

		// Attributes
		BITMAP GetBitmapData() const;
	};


	///////////////////////////////////////////////
	// Declarations for the CBrush class
	//
	class CBrush : public CGDIObject
	{
	  public:
		CBrush();
		CBrush(HBRUSH hBrush);
		CBrush(COLORREF crColor);
		operator HBRUSH() const;
		virtual ~CBrush();

		HBRUSH CreateSolidBrush(COLORREF crColor);
		HBRUSH CreatePatternBrush(HBITMAP hBitmap);
		LOGBRUSH GetLogBrush() const;

#ifndef _WIN32_WCE
		HBRUSH CreateHatchBrush(int nIndex, COLORREF crColor);
		HBRUSH CreateBrushIndirect(const LOGBRUSH& LogBrush);
		HBRUSH CreateDIBPatternBrush(HGLOBAL hglbDIBPacked, UINT fuColorSpec);
		HBRUSH CreateDIBPatternBrushPt(LPCVOID pPackedDIB, UINT nUsage);
#endif // !defined(_WIN32_WCE)

	};


	///////////////////////////////////////////////
	// Declarations for the CFont class
	//
	class CFont : public CGDIObject
	{
	public:
		CFont();
		CFont(HFONT hFont);
		CFont(const LOGFONT& LogFont);
		operator HFONT() const;
		virtual ~CFont();

		// Create methods
		HFONT CreateFontIndirect(const LOGFONT& LogFont);
		HFONT CreatePointFont(int nPointSize, LPCTSTR lpszFaceName, HDC hdc = NULL, BOOL IsBold = FALSE, BOOL IsItalic = FALSE);
		HFONT CreatePointFontIndirect(const LOGFONT& LogFont, HDC hdc = NULL);

#ifndef _WIN32_WCE
		HFONT CreateFont(int nHeight, int nWidth, int nEscapement,
				int nOrientation, int nWeight, DWORD dwItalic, DWORD dwUnderline,
				DWORD dwStrikeOut, DWORD dwCharSet, DWORD dwOutPrecision,
				DWORD dwClipPrecision, DWORD dwQuality, DWORD dwPitchAndFamily,
				LPCTSTR lpszFacename);
#endif // #ifndef _WIN32_WCE

		// Attributes
		LOGFONT GetLogFont() const;
	};


	///////////////////////////////////////////////
	// Declarations for the CPalette class
	//
	class CPalette : public CGDIObject
	{
	  public:
		CPalette();
		CPalette(HPALETTE hPalette);
		operator HPALETTE() const;
		virtual ~CPalette();

		// Create methods
		HPALETTE CreatePalette(LPLOGPALETTE lpLogPalette);

#ifndef _WIN32_WCE
		HPALETTE CreateHalftonePalette(HDC hdc);
#endif // !_WIN32_WCE

		// Attributes
		int GetEntryCount() const;
		UINT GetPaletteEntries(UINT nStartIndex, UINT nNumEntries, LPPALETTEENTRY lpPaletteColors) const;
		UINT SetPaletteEntries(UINT nStartIndex, UINT nNumEntries, LPPALETTEENTRY lpPaletteColors) const;

		// Operations
#ifndef _WIN32_WCE
		BOOL ResizePalette(UINT nNumEntries) const;
		void AnimatePalette(UINT nStartIndex, UINT nNumEntries, LPPALETTEENTRY lpPaletteColors) const;
#endif // !_WIN32_WCE

		UINT GetNearestPaletteIndex (COLORREF crColor) const;

	};


	///////////////////////////////////////////////
	// Declarations for the CPen class
	//
	class CPen : public CGDIObject
	{
	public:
		CPen();
		CPen(HPEN hPen);
		CPen(int nPenStyle, int nWidth, COLORREF crColor);
#ifndef _WIN32_WCE
		CPen(int nPenStyle, int nWidth, const LOGBRUSH& LogBrush, int nStyleCount = 0, const DWORD* lpStyle = NULL);
#endif // !_WIN32_WCE
		operator HPEN() const;
		virtual ~CPen();

		HPEN CreatePen(int nPenStyle, int nWidth, COLORREF crColor);
		HPEN CreatePenIndirect(const LOGPEN& LogPen);
		LOGPEN GetLogPen() const;

#ifndef _WIN32_WCE
		HPEN ExtCreatePen(int nPenStyle, int nWidth, const LOGBRUSH& LogBrush, int nStyleCount = 0, const DWORD* lpStyle = NULL) ;
		EXTLOGPEN GetExtLogPen() const;
#endif // !_WIN32_WCE

	};


	///////////////////////////////////////////////
	// Declarations for the CRgn class
	//
	class CRgn : public CGDIObject
	{
	  public:
		CRgn();
		CRgn(HRGN hRgn);
		operator HRGN() const;
		virtual ~CRgn ();

		// Create methods
		HRGN CreateRectRgn(int x1, int y1, int x2, int y2);
		HRGN CreateRectRgnIndirect(const RECT& rc);
		HRGN CreateFromData(const XFORM* lpXForm, int nCount, const RGNDATA* pRgnData);

#ifndef _WIN32_WCE
		HRGN CreateEllipticRgn(int x1, int y1, int x2, int y2);
		HRGN CreateEllipticRgnIndirect(const RECT& rc);
		HRGN CreatePolygonRgn(LPPOINT lpPoints, int nCount, int nMode);
		HRGN CreatePolyPolygonRgn(LPPOINT lpPoints, LPINT lpPolyCounts, int nCount, int nPolyFillMode);
		HRGN CreateRoundRectRgn(int x1, int y1, int x2, int y2, int x3, int y3);
		HRGN CreateFromPath(HDC hDC);
#endif // !_WIN32_WCE

		// Operations
		void SetRectRgn(int x1, int y1, int x2, int y2) const;
		void SetRectRgn(const RECT& rc) const;
		int CombineRgn(HRGN hrgnSrc1, HRGN hrgnSrc2, int nCombineMode) const;
		int CombineRgn(HRGN hrgnSrc, int nCombineMode) const;
		int CopyRgn(HRGN hrgnSrc) const;
		BOOL EqualRgn(HRGN hRgn) const;
		int OffsetRgn(int x, int y) const;
		int OffsetRgn(POINT& pt) const;
		int GetRgnBox(RECT& rc) const;
		BOOL PtInRegion(int x, int y) const;
		BOOL PtInRegion(POINT& pt) const;
		BOOL RectInRegion(const RECT& rc) const;
		int GetRegionData(LPRGNDATA lpRgnData, int nDataSize) const;
	};

	struct CDC_Data	// A structure that contains the data members for CDC
	{
		// Constructor
		CDC_Data() : hDC(0), Count(1L), IsManagedHDC(FALSE), hWnd(0), nSavedDCState(0) {}

		std::vector<GDIPtr> m_vGDIObjects;	// Smart pointers to internally created Bitmaps, Brushes, Fonts, Bitmaps and Regions
		CBitmap Bitmap;
		CBrush	Brush;
		CFont	Font;
		CPalette Palette;
		CPen	Pen;
		CRgn	Rgn;
		HDC		hDC;			// The HDC belonging to this CDC
		long	Count;			// Reference count
		bool	IsManagedHDC;	// Delete/Release the HDC on destruction
		HWND	hWnd;			// The HWND of a Window or Client window DC
		int		nSavedDCState;	// The save state of the HDC.
	};


	///////////////////////////////////////////////
	// Declarations for the CDC class
	//
	class CDC
	{
		friend class CWinApp;
		friend class CWnd;

	public:
		CDC();									// Constructs a new CDC without assigning a HDC
		CDC(HDC hDC, HWND hWnd = 0);			// Constructs a new CDC and assigns a HDC
		CDC(const CDC& rhs);					// Constructs a new copy of the CDC
		virtual ~CDC();
		operator HDC() const { return m_pData->hDC; }	// Converts a CDC to a HDC
		CDC& operator = (const CDC& rhs);		// Assigns a CDC to an existing CDC

		void Attach(HDC hDC, HWND hWnd = 0);
		void Destroy();
		HDC  Detach();
		HDC GetHDC() const { return m_pData->hDC; }
		BOOL RestoreDC(int nSavedDC) const;
		int SaveDC() const;
		HGDIOBJ SelectObject(HGDIOBJ hObject) const;
		HBITMAP SelectObject(HBITMAP hBitmap) const;
		HBRUSH SelectObject(HBRUSH hBrush) const;
		HFONT SelectObject(HFONT hFont) const;
		HPEN SelectObject(HPEN hPen) const;
		int SelectObject(HRGN hRgn) const;

#ifndef _WIN32_WCE
		void operator = (const HDC hDC);
#endif

		// Initialization
		HDC CreateCompatibleDC(HDC hdcSource);
		HDC CreateDC(LPCTSTR lpszDriver, LPCTSTR lpszDevice, LPCTSTR lpszOutput, const DEVMODE* pInitData);
		int GetDeviceCaps(int nIndex) const;
#ifndef _WIN32_WCE
		HDC CreateIC(LPCTSTR lpszDriver, LPCTSTR lpszDevice, LPCTSTR lpszOutput, const DEVMODE* pInitData);
#endif

		// Create Bitmaps
		void CreateBitmap(int cx, int cy, UINT Planes, UINT BitsPerPixel, LPCVOID pColors);
		void CreateCompatibleBitmap(HDC hdc, int cx, int cy);
		void CreateDIBSection(HDC hdc, const LPBITMAPINFO pbmi, UINT iUsage, LPVOID* ppBits,
										HANDLE hSection, DWORD dwOffset);
		CBitmap DetachBitmap();
		BITMAP  GetBitmapData() const;
		HBITMAP GetCurrentBitmap() const;
		BOOL LoadBitmap(UINT nID);
		BOOL LoadBitmap(LPCTSTR lpszName);
		BOOL LoadImage(UINT nID, UINT fuLoad);
		BOOL LoadImage(LPCTSTR lpszName, UINT fuLoad);
		BOOL LoadOEMBitmap(UINT nIDBitmap); // for OBM_/OCR_/OIC

#ifndef _WIN32_WCE
		void CreateBitmapIndirect(const BITMAP& Bitmap);
		void CreateDIBitmap(HDC hdc, const BITMAPINFOHEADER& bmih, DWORD fdwInit, LPCVOID pbInit,
										const LPBITMAPINFO pbmi, UINT fuUsage);
		void CreateMappedBitmap(UINT nIDBitmap, UINT nFlags /*= 0*/, LPCOLORMAP lpColorMap /*= NULL*/, int nMapSize /*= 0*/);
#endif

		// Create Brushes
		void CreatePatternBrush(HBITMAP hBitmap);
		void CreateSolidBrush(COLORREF rbg);
		HBRUSH GetCurrentBrush() const;
		LOGBRUSH GetLogBrush() const;

#ifndef _WIN32_WCE
		void CreateBrushIndirect(const LOGBRUSH& LogBrush);
		void CreateHatchBrush(int fnStyle, COLORREF rgb);
		void CreateDIBPatternBrush(HGLOBAL hglbDIBPacked, UINT fuColorSpec);
		void CreateDIBPatternBrushPt(LPCVOID lpPackedDIB, UINT iUsage);
#endif

		// Create Fonts
		void CreateFontIndirect(const LOGFONT& lf);
		HFONT GetCurrentFont() const;
		LOGFONT GetLogFont() const;

#ifndef _WIN32_WCE
		void CreateFont(int nHeight, int nWidth, int nEscapement, int nOrientation, int fnWeight,
  							DWORD fdwItalic, DWORD fdwUnderline, DWORD fdwStrikeOut, DWORD fdwCharSet,
  							DWORD fdwOutputPrecision, DWORD fdwClipPrecision, DWORD fdwQuality,
  							DWORD fdwPitchAndFamily, LPCTSTR lpszFace);
#endif

		// Create and select Palettes
		void CreatePalette(LPLOGPALETTE pLogPalette, BOOL ForceBkgnd);
		HPALETTE SelectPalette(HPALETTE hPalette, BOOL ForceBkgnd);

#ifndef _WIN32_WCE
		void CreateHalftonePalette(BOOL ForceBkgnd);
#endif

		// Create Pens
		void CreatePen(int nStyle, int nWidth, COLORREF rgb);
		void CreatePenIndirect(const LOGPEN& LogPen);
		HPEN GetCurrentPen() const;
		LOGPEN GetLogPen() const;

		// Retrieve and Select Stock Objects
		HGDIOBJ GetStockObject(int nIndex) const;
		HGDIOBJ SelectStockObject(int nIndex) const;

		// Create Regions
		int CreateRectRgn(int left, int top, int right, int bottom);
		int CreateRectRgnIndirect(const RECT& rc);
		int CreateFromData(const XFORM* Xform, DWORD nCount, const RGNDATA* pRgnData);

#ifndef _WIN32_WCE
		int CreateEllipticRgn(int left, int top, int right, int bottom);
		int CreateEllipticRgnIndirect(const RECT& rc);
		int CreatePolygonRgn(LPPOINT ppt, int cPoints, int fnPolyFillMode);
		int CreatePolyPolygonRgn(LPPOINT ppt, LPINT pPolyCounts, int nCount, int fnPolyFillMode);
#endif

		// Wrappers for WinAPI functions
#ifndef _WIN32_WCE
		int EnumObjects(int nObjectType, GOBJENUMPROC lpObjectFunc, LPARAM lParam) const;
#endif

		// Point and Line Drawing Functions
		CPoint GetCurrentPosition() const;
		CPoint MoveTo(int x, int y) const;
		CPoint MoveTo(POINT pt) const;
		BOOL LineTo(int x, int y) const;
		BOOL LineTo(POINT pt) const;
		COLORREF GetPixel(int x, int y) const;
		COLORREF GetPixel(POINT pt) const;
		COLORREF SetPixel(int x, int y, COLORREF crColor) const;
		COLORREF SetPixel(POINT pt, COLORREF crColor) const;
		int SetROP2(int iDrawMode) const;

#ifndef _WIN32_WCE
		BOOL Arc(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4) const;
		BOOL Arc(RECT& rc, POINT ptStart, POINT ptEnd) const;
		BOOL ArcTo(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4) const;
		BOOL ArcTo(RECT& rc, POINT ptStart, POINT ptEnd) const;
		BOOL AngleArc(int x, int y, int nRadius, float fStartAngle, float fSweepAngle) const;
		BOOL CloseFigure() const;
		int  GetROP2() const;
		int  GetArcDirection() const;
		int  SetArcDirection(int nArcDirection) const;
		BOOL PolyDraw(const POINT* lpPoints, const BYTE* lpTypes, int nCount) const;
		BOOL Polyline(LPPOINT lpPoints, int nCount) const;
		BOOL PolyPolyline(const POINT* lpPoints, const DWORD* lpPolyPoints, int nCount) const;
		BOOL PolylineTo(const POINT* lpPoints, int nCount) const;
		BOOL PolyBezier(const POINT* lpPoints, int nCount) const;
		BOOL PolyBezierTo(const POINT* lpPoints, int nCount) const;
		BOOL SetPixelV(int x, int y, COLORREF crColor) const;
		BOOL SetPixelV(POINT pt, COLORREF crColor) const;
#endif

		// Shape Drawing Functions
		void DrawFocusRect(const RECT& rc) const;
		BOOL Ellipse(int x1, int y1, int x2, int y2) const;
		BOOL Ellipse(const RECT& rc) const;
		BOOL Polygon(LPPOINT lpPoints, int nCount) const;
		BOOL Rectangle(int x1, int y1, int x2, int y2) const;
		BOOL Rectangle(const RECT& rc) const;
		BOOL RoundRect(int x1, int y1, int x2, int y2, int nWidth, int nHeight) const;
		BOOL RoundRect(const RECT& rc, int nWidth, int nHeight) const;

#ifndef _WIN32_WCE
		BOOL Chord(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4) const;
		BOOL Chord(const RECT& rc, POINT ptStart, POINT ptEnd) const;
		BOOL Pie(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4) const;
		BOOL Pie(const RECT& rc, POINT ptStart, POINT ptEnd) const;
		BOOL PolyPolygon(LPPOINT lpPoints, LPINT lpPolyCounts, int nCount) const;
#endif

		// Fill and Image Drawing functions
		BOOL DrawEdge(const RECT& rc, UINT nEdge, UINT nFlags) const;
		BOOL DrawIconEx(int xLeft, int yTop, HICON hIcon, int cxWidth, int cyWidth,
			            UINT istepIfAniCur, HBRUSH hFlickerFreeDraw, UINT diFlags) const;

		BOOL DrawFrameControl(const RECT& rc, UINT nType, UINT nState) const;
		BOOL FillRect(const RECT& rc, HBRUSH hBrush) const;
		BOOL FillRgn(HRGN hRgn, HBRUSH hBrush) const;
		void GradientFill(COLORREF Color1, COLORREF Color2, const RECT& rc, BOOL IsVertical) const;
		BOOL InvertRect(const RECT& rc) const;
		void SolidFill(COLORREF Color, const RECT& rc) const;

#if (WINVER >= 0x0410)
		BOOL GradientFill(PTRIVERTEX pVertex, ULONG nVertex, PVOID pMesh, ULONG nMesh, ULONG ulMode) const;
#endif

#ifndef _WIN32_WCE
		BOOL DrawIcon(int x, int y, HICON hIcon) const;
		BOOL DrawIcon(POINT point, HICON hIcon) const;
		BOOL FrameRect(const RECT& rc, HBRUSH hBrush) const;
		BOOL FrameRgn(HRGN hRgn, HBRUSH hBrush, int nWidth, int nHeight) const;
		int  GetPolyFillMode() const;
		BOOL PaintRgn(HRGN hRgn) const;
		int  SetPolyFillMode(int iPolyFillMode) const;
#endif

		// Bitmap Functions
		BOOL BitBlt(int x, int y, int nWidth, int nHeight, HDC hdcSrc, int xSrc, int ySrc, DWORD dwRop) const;
		void DrawBitmap(int x, int y, int cx, int cy, HBITMAP hbmImage, COLORREF clrMask) const;
		BOOL MaskBlt(int nXDest, int nYDest, int nWidth, int nHeight, HDC hdcSrc,
			               int nXSrc, int nYSrc, HBITMAP hbmMask, int xMask, int  yMask,
						   DWORD dwRop) const;

		BOOL PatBlt(int x, int y, int nWidth, int nHeight, DWORD dwRop) const;
		int  StretchDIBits(int XDest, int YDest, int nDestWidth, int nDestHeight,
			               int XSrc, int YSrc, int nSrcWidth, int nSrcHeight,
						   LPCVOID pBits, const LPBITMAPINFO pbmi, UINT iUsage, DWORD dwRop) const;

		BOOL StretchBlt(int x, int y, int nWidth, int nHeight, HDC hdcSrc,
			               int xSrc, int ySrc, int nSrcWidth, int nSrcHeight,
						   DWORD dwRop) const;

#ifndef _WIN32_WCE
		BOOL ExtFloodFill(int x, int y, COLORREF crColor, UINT nFillType) const;
		BOOL FloodFill(int x, int y, COLORREF crColor) const;
		int  GetDIBits(HBITMAP hBitmap, UINT uStartScan, UINT cScanLines, LPVOID pBits,
			            LPBITMAPINFO pbi, UINT uUsage) const;

		int  GetStretchBltMode() const;
		int  SetDIBits(HBITMAP hBitmap, UINT uStartScan, UINT cScanLines, LPCVOID pBits,
			            LPBITMAPINFO pbmi, UINT fuColorUse) const;

		int  SetStretchBltMode(int iStretchMode) const;
  #if (WINVER >= 0x0410)
		BOOL TransparentBlt(int x, int y, int nWidth, int hHeight, HDC hdcSrc,
			               int xSrc, int ySrc, int nWidthSrc, int nHeightSrc,
						   UINT crTransparent) const;
  #endif
#endif

		// Brush Functions
#if (_WIN32_WINNT >= 0x0500)
		COLORREF GetDCBrushColor() const;
		COLORREF SetDCBrushColor(COLORREF crColor) const;
#endif

		// Font Functions
#ifndef _WIN32_WCE
		DWORD GetFontData(DWORD dwTable, DWORD dwOffset, LPVOID pvBuffer,  DWORD cbData) const;
		DWORD GetFontLanguageInfo() const;
		DWORD GetGlyphOutline(UINT uChar, UINT uFormat, LPGLYPHMETRICS pgm, DWORD cbBuffer,
			                  LPVOID pvBuffer, const MAT2* lpmat2) const;

		DWORD GetKerningPairs(DWORD nNumPairs, LPKERNINGPAIR pkrnpair) const;
		DWORD SetMapperFlags(DWORD dwFlag) const;
#endif

		// Palette and color functions
		HPALETTE GetCurrentPalette() const;
		COLORREF GetNearestColor(COLORREF crColor) const;
		void RealizePalette() const;

#ifndef _WIN32_WCE
		BOOL GetColorAdjustment(LPCOLORADJUSTMENT pCA) const;
		BOOL SetColorAdjustment(const COLORADJUSTMENT* pCA) const;
		BOOL UpdateColors() const;
#endif

#ifndef _WIN32_WCE
		// Pen Functions
		BOOL GetMiterLimit(PFLOAT peLimit) const;
		BOOL SetMiterLimit(FLOAT eNewLimit, PFLOAT peOldLimit) const;
#endif

		// Clipping and Region Functions
		int  ExcludeClipRect(int Left, int Top, int Right, int BottomRect) const;
		int  ExcludeClipRect(const RECT& rc) const;
		int  GetClipBox(RECT& rc) const;
		int  IntersectClipRect(int Left, int Top, int Right, int Bottom) const;
		int  IntersectClipRect(const RECT& rc) const;
		BOOL RectVisible(const RECT& rc) const;
		int  SelectClipRgn(HRGN hRgn) const;

#ifndef _WIN32_WCE
		BOOL BeginPath() const;
		BOOL EndPath() const;
		int  ExtSelectClipRgn(HRGN hRgn, int fnMode) const;
		BOOL FlattenPath() const;
		int	 GetPath(POINT* pPoints, BYTE* pTypes, int nCount) const;
		int  OffsetClipRgn(int nXOffset, int nYOffset) const;
		BOOL PtVisible(int X, int Y) const;
		BOOL SelectClipPath(int nMode) const;
		BOOL WidenPath() const;
#endif

        // Co-ordinate Functions
#ifndef _WIN32_WCE
		BOOL DPtoLP(LPPOINT lpPoints, int nCount)  const;
		BOOL DPtoLP(RECT& rc)  const;
		BOOL LPtoDP(LPPOINT lpPoints, int nCount)  const;
		BOOL LPtoDP(RECT& rc)  const;
#endif

		// Layout Functions
		DWORD GetLayout() const;
		DWORD SetLayout(DWORD dwLayout) const;

		// Mapping functions
#ifndef _WIN32_WCE
		int  GetMapMode() const;
		int  SetMapMode(int nMapMode) const;
		BOOL GetViewportOrgEx(LPPOINT lpPoint)  const;
		BOOL SetViewportOrgEx(int x, int y, LPPOINT lpPoint = NULL) const;
		BOOL SetViewportOrgEx(POINT point, LPPOINT lpPointRet = NULL) const;
		BOOL OffsetViewportOrgEx(int nWidth, int nHeight, LPPOINT lpPoint = NULL) const;
		BOOL GetViewportExtEx(LPSIZE lpSize)  const;
		BOOL SetViewportExtEx(int x, int y, LPSIZE lpSize) const;
		BOOL SetViewportExtEx(SIZE size, LPSIZE lpSizeRet) const;
		BOOL ScaleViewportExtEx(int xNum, int xDenom, int yNum, int yDenom, LPSIZE lpSize) const;
		BOOL GetWindowExtEx(LPSIZE lpSize)  const;
		BOOL SetWindowExtEx(int x, int y, LPSIZE lpSize) const;
		BOOL SetWindowExtEx(SIZE size, LPSIZE lpSizeRet) const;
		BOOL ScaleWindowExtEx(int xNum, int xDenom, int yNum, int yDenom, LPSIZE lpSize) const;
		BOOL GetWindowOrgEx(LPPOINT lpPoint)  const;
		BOOL SetWindowOrgEx(int x, int y, LPPOINT lpPoint) const;
		BOOL SetWindowOrgEx(POINT point, LPPOINT lpPointRet) const;
		BOOL OffsetWindowOrgEx(int nWidth, int nHeight, LPPOINT lpPoint) const;
#endif

		// MetaFile Functions
#ifndef _WIN32_WCE
		BOOL PlayMetaFile(HMETAFILE hMF) const;
		BOOL PlayMetaFile(HENHMETAFILE hEnhMetaFile, const RECT& rcBounds) const;
#endif

		// Printer Functions
		int StartDoc(LPDOCINFO lpDocInfo) const;
		int EndDoc() const;
		int StartPage() const;
		int EndPage() const;
		int AbortDoc() const;
		int SetAbortProc(BOOL (CALLBACK* lpfn)(HDC, int)) const;

		// Text Functions
		int   DrawText(LPCTSTR lpszString, int nCount, RECT& rc, UINT nFormat) const;
		BOOL  ExtTextOut(int x, int y, UINT nOptions, const RECT& rc, LPCTSTR lpszString, int nCount = -1, LPINT lpDxWidths = NULL) const;
		COLORREF GetBkColor() const;
		int   GetBkMode() const;
		UINT  GetTextAlign() const;
		int   GetTextFace(int nCount, LPTSTR lpszFacename) const;
		COLORREF GetTextColor() const;
		BOOL  GetTextMetrics(TEXTMETRIC& Metrics) const;
		COLORREF SetBkColor(COLORREF crColor) const;
		int   SetBkMode(int iBkMode) const;
		UINT  SetTextAlign(UINT nFlags) const;
		COLORREF SetTextColor(COLORREF crColor) const;

#ifndef _WIN32_WCE
		int   DrawTextEx(LPTSTR lpszString, int nCount, RECT& rc, UINT nFormat, LPDRAWTEXTPARAMS lpDTParams) const;
		DWORD GetCharacterPlacement(LPCTSTR pString, int nCount, int nMaxExtent,
		                            LPGCP_RESULTS pResults, DWORD dwFlags) const;

		BOOL  GetCharABCWidths(UINT uFirstChar, UINT uLastChar, LPABC pABC) const;
		BOOL  GetCharWidth(UINT iFirstChar, UINT iLastChar, float* pBuffer) const;
		CSize GetTabbedTextExtent(LPCTSTR lpszString, int nCount, int nTabPositions, LPINT lpnTabStopPositions) const;
		int   GetTextCharacterExtra() const;
		CSize GetTextExtentPoint32(LPCTSTR lpszString, int nCount) const;
		CSize GetTextExtentPoint32(CString& str) const;
		BOOL  GrayString(HBRUSH hBrush, GRAYSTRINGPROC lpOutputFunc, LPARAM lpData, int nCount, int x, int y, int nWidth, int nHeight) const;
		int   SetTextCharacterExtra(int nCharExtra) const;
		int   SetTextJustification(int nBreakExtra, int nBreakCount) const;
		CSize TabbedTextOut(int x, int y, LPCTSTR lpszString, int nCount, int nTabPositions, LPINT lpnTabStopPositions, int nTabOrigin) const;
		BOOL  TextOut(int x, int y, LPCTSTR lpszString, int nCount = -1) const;

  #if (_WIN32_WINNT >= 0x0500) && !defined(__GNUC__)
		BOOL  GetCharABCWidthsI(UINT giFirst, UINT cgi, LPWORD pgi, LPABC pABC) const;
		BOOL  GetCharWidthI(UINT giFirst, UINT cgi, LPWORD pgi, int* pBuffer) const;
  #endif // (_WIN32_WINNT >= 0x0500) && !defined(__GNUC__)
#endif  // _WIN32_WCE

	protected:
		void Release();
		void SetManaged(bool IsManaged) { m_pData->IsManagedHDC = IsManaged; }

	private:
		void AddToMap();
		BOOL RemoveFromMap();

		CDC_Data* m_pData;		// pointer to the class's data members
	};

	class CClientDC : public CDC
	{
	public:
		CClientDC(HWND hWnd)
		{
			if (0 == hWnd) hWnd = GetDesktopWindow();
			assert(::IsWindow(hWnd));

			try
			{
				HDC hDC = ::GetDC(hWnd);
				if (hDC == 0)
					throw CResourceException(_T("GetDC failed"));

				Attach(hDC, hWnd);
				SetManaged(true);
			}

			catch(...)
			{
				Release();	// Cleanup
				throw;		// Rethrow
			}
		}

		virtual ~CClientDC() {}
	};

	class CClientDCEx : public CDC
	{
	public:
		CClientDCEx(HWND hWnd, HRGN hrgnClip, DWORD flags)
		{
			if (0 == hWnd) hWnd = GetDesktopWindow();
			assert(::IsWindow(hWnd));

			try
			{
				HDC hDC = ::GetDCEx(hWnd, hrgnClip, flags);
				if (hDC == 0)
					throw CResourceException(_T("GetDCEx failed"));

				Attach(hDC, hWnd);
				SetManaged(true);
			}

			catch(...)
			{
				Release();	// Cleanup
				throw;		// Rethrow
			}
		}

		virtual ~CClientDCEx() {}
	};

	class CMemDC : public CDC
	{
	public:
		CMemDC(HDC hdc)
		{
			try
			{
				CreateCompatibleDC(hdc);
			}

			catch(...)
			{
				Release();	// Cleanup
				throw;		// Rethrow
			}
		}
		virtual ~CMemDC() {}
	};

	class CPaintDC : public CDC
	{
	public:
		CPaintDC(HWND hWnd) : m_hWndPaint(hWnd)
		{
			assert(::IsWindow(hWnd));

			try
			{
				HDC hDC = ::BeginPaint(hWnd, &m_ps);
				if (hDC == 0)
					throw CResourceException(_T("BeginPaint failed"));

				Attach(hDC, hWnd);
				SetManaged(true);
			}

			catch(...)
			{
				Release();	// Cleanup
				throw;		// Rethrow
			}
		}

		virtual ~CPaintDC()
		{
			::EndPaint(m_hWndPaint, &m_ps);
		}

	private:
		HWND m_hWndPaint;
		PAINTSTRUCT m_ps;
	};

	class CWindowDC : public CDC
	{
	public:
		CWindowDC(HWND hWnd)
		{
			if (0 == hWnd) hWnd = GetDesktopWindow();
			assert(::IsWindow(hWnd));

			try
			{
				HDC hDC = ::GetWindowDC(hWnd);
				if (hDC == 0)
					throw CResourceException(_T("GetWindowDC failed"));

				Attach(hDC, hWnd);
				SetManaged(true);
			}

			catch(...)
			{
				Release();	// Cleanup
				throw;		// Rethrow
			}

		}
		virtual ~CWindowDC() {}
	};

#ifndef _WIN32_WCE
	class CMetaFileDC : public CDC
	{
	public:
		CMetaFileDC() : IsEnhMetaFile(FALSE) {}
		virtual ~CMetaFileDC()
		{
			if (GetHDC())
			{
				// Note we should not get here. 
				TRACE("Warning! A MetaFile or EnhMetaFile was created but not closed\n");
				if (IsEnhMetaFile)
				{ 
					::DeleteEnhMetaFile(CloseEnhanced());
				}
				else
				{
					::DeleteMetaFile(Close());
				}

				Detach();
			}

		}

		void Create(LPCTSTR lpszFilename = NULL)
		{
			assert(GetHDC() == 0);
			HDC hDC = ::CreateMetaFile(lpszFilename);
			if (hDC == 0)
				throw CResourceException(_T("Failed to create a DC for the MetaFile"));

			Attach(hDC);
			IsEnhMetaFile = FALSE;
		}

		void CreateEnhanced(HDC hdcRef, LPCTSTR lpszFileName, const RECT* prcBounds, LPCTSTR lpszDescription)
		{
			assert(GetHDC() == 0);
			HDC hDC = ::CreateEnhMetaFile(hdcRef, lpszFileName, prcBounds, lpszDescription);
			if (hDC == 0)
				throw CResourceException(_T("Failed to create a DC for the EnhMetaFile"));

			Attach(hDC);
			IsEnhMetaFile = TRUE;
		}

		CMetaFile Close() 
		// Closes the metafile and returns a CMetaFile object.
		// The CMetaFile object automatically deletes the HMETAFILE when the last copy
		// of the CMetaFile goes out of scope.
		{
			assert(GetHDC());
			assert(IsEnhMetaFile == FALSE);

			HMETAFILE hMeta = ::CloseMetaFile(GetHDC());
			Detach();
			return CMetaFile(hMeta);
		}

		CEnhMetaFile CloseEnhanced()
		// Closes the enhanced metafile and returns a CEnhMetaFile object.
		// The CEnhMetaFile object automatically deletes the HENHMETAFILE when the last copy
		// of the CEnhMetaFile goes out of scope. 
		{ 
			assert(GetHDC());
			assert(IsEnhMetaFile == TRUE);

			HENHMETAFILE enhMeta = ::CloseEnhMetaFile(GetHDC());
			Detach();
			return CEnhMetaFile(enhMeta);
		}

	private:
		BOOL IsEnhMetaFile;
	};

#endif


	///////////////////////////////////////////////
	// Declarations for the CBitmapInfoPtr class
	// The CBitmapInfoPtr class is a convenient wrapper for the BITMAPINFO structure.
	class CBitmapInfoPtr
	{
	public:
		CBitmapInfoPtr(HBITMAP hBitmap)
		{
			BITMAP bmSource;
			::GetObject(hBitmap, sizeof(BITMAP), &bmSource);

			// Convert the color format to a count of bits.
			WORD cClrBits = (WORD)(bmSource.bmPlanes * bmSource.bmBitsPixel);
			if (cClrBits == 1) 	     cClrBits = 1;
			else if (cClrBits <= 4)  cClrBits = 4;
			else if (cClrBits <= 8)  cClrBits = 8;
			else if (cClrBits <= 16) cClrBits = 16;
			else if (cClrBits <= 24) cClrBits = 24;
			else                     cClrBits = 32;

			// Allocate memory for the BITMAPINFO structure.
			UINT uQuadSize = (cClrBits == 24)? 0 : sizeof(RGBQUAD) * (int)(1 << cClrBits);
			m_bmi.assign(sizeof(BITMAPINFOHEADER) + uQuadSize, 0);
			m_pbmiArray = (LPBITMAPINFO) &m_bmi[0];

			m_pbmiArray->bmiHeader.biSize		= sizeof(BITMAPINFOHEADER);
			m_pbmiArray->bmiHeader.biHeight		= bmSource.bmHeight;
			m_pbmiArray->bmiHeader.biWidth		= bmSource.bmWidth;
			m_pbmiArray->bmiHeader.biPlanes		= bmSource.bmPlanes;
			m_pbmiArray->bmiHeader.biBitCount	= bmSource.bmBitsPixel;
			m_pbmiArray->bmiHeader.biCompression = BI_RGB;
			if (cClrBits < 24)
				m_pbmiArray->bmiHeader.biClrUsed = (1<<cClrBits);
		}
		LPBITMAPINFO get() const { return m_pbmiArray; }
		operator LPBITMAPINFO() const { return m_pbmiArray; }
		LPBITMAPINFO operator->() const { return m_pbmiArray; }

	private:
		CBitmapInfoPtr(const CBitmapInfoPtr&);				// Disable copy construction
		CBitmapInfoPtr& operator = (const CBitmapInfoPtr&);	// Disable assignment operator
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

	inline CGDIObject::CGDIObject()
	// Constructs the CGDIObject
	{
		m_pData = new CGDI_Data;
	}

	inline CGDIObject::CGDIObject(const CGDIObject& rhs)
	// Note: A copy of a CGDIObject is a clone of the original.
	//       Both objects manipulate the one HGDIOBJ.
	{
		m_pData = rhs.m_pData;
		InterlockedIncrement(&m_pData->Count);
	}

	inline CGDIObject::~CGDIObject()
	// Deconstructs the CGDIObject
	{
		Release();
	}

	inline CGDIObject& CGDIObject::operator = ( const CGDIObject& rhs )
	// Note: A copy of a CGDIObject is a clone of the original.
	//       Both objects manipulate the one HGDIOBJ.
	{
		if (this != &rhs)
		{
			InterlockedIncrement(&rhs.m_pData->Count);
			Release();
			m_pData = rhs.m_pData;
		}

		return *this;
	}

	inline void CGDIObject::operator = (const HGDIOBJ hObject)
	{
		Attach(hObject);
	}

	inline void CGDIObject::AddToMap()
	// Store the HDC and CDC pointer in the HDC map
	{
		assert( &GetApp() );
		assert(m_pData->hGDIObject);

		GetApp().AddCGDIData(m_pData->hGDIObject, m_pData);
	}

	inline void CGDIObject::Attach(HGDIOBJ hObject)
	// Attaches a GDI HANDLE to the CGDIObject.
	{
		assert(m_pData);

		if (hObject != m_pData->hGDIObject)
		{
			// Release any existing GDI object
			if (m_pData->hGDIObject != 0)
			{
				Release();
				m_pData = new CGDI_Data;
			}

			if (hObject)
			{
				// Add the GDI object to this CCGDIObject
				CGDI_Data* pCGDIData = GetApp().GetCGDIData(hObject);
				if (pCGDIData)
				{
					delete m_pData;
					m_pData = pCGDIData;
					InterlockedIncrement(&m_pData->Count);
				}
				else
				{
					m_pData->hGDIObject = hObject;
					AddToMap();
				}
			}
		}
	}

	inline void CGDIObject::DeleteObject()
	{
		assert(m_pData);

		if (m_pData->hGDIObject != 0)
		{
			RemoveFromMap();

			::DeleteObject(m_pData->hGDIObject);
			m_pData->hGDIObject = 0;
		}
	}

	inline HGDIOBJ CGDIObject::Detach()
	// Detaches the HGDIOBJ from all objects.
	{
		assert(m_pData);
		assert(m_pData->hGDIObject);

		HGDIOBJ hObject = m_pData->hGDIObject;
		RemoveFromMap();
		m_pData->hGDIObject = 0;

		if (m_pData->Count > 0)
		{
			if (InterlockedDecrement(&m_pData->Count) == 0)
			{
				delete m_pData;
			}
		}

		m_pData = new CGDI_Data;
		return hObject;
	}

	inline HGDIOBJ CGDIObject::GetHandle() const
	// Returns the GDI handle (HGDIOBJ) associated with this object.
	{
		assert(m_pData);
		return m_pData->hGDIObject;
	}

	inline int CGDIObject::GetObject(int nCount, LPVOID pObject) const
	// Retrieves information for the specified graphics object.
	{
		assert(m_pData);
		return ::GetObject(m_pData->hGDIObject, nCount, pObject);
	}

	inline void CGDIObject::Release()
	{
		assert(m_pData);

		if (InterlockedDecrement(&m_pData->Count) == 0)
		{
			if (m_pData->hGDIObject != NULL)
			{
				if (m_pData->IsManagedObject)
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
		BOOL Success = FALSE;

		if( &GetApp() )
		{
			// Allocate an iterator for our HDC map
			std::map<HGDIOBJ, CGDI_Data*, CompareGDI>::iterator m;

			CWinApp& App = GetApp();
			App.m_csMapLock.Lock();
			m = App.m_mapCGDIData.find(m_pData->hGDIObject);
			if (m != App.m_mapCGDIData.end())
			{
				// Erase the CGDIObject pointer entry from the map
				App.m_mapCGDIData.erase(m);
				Success = TRUE;
			}

			App.m_csMapLock.Release();
		}

		return Success;
	}


	///////////////////////////////////////////////
	// Declarations for the CBitmap class
	//
	inline CBitmap::CBitmap()
	{
	}

	inline CBitmap::CBitmap(HBITMAP hBitmap)
	{
		Attach(hBitmap);
	}

	inline CBitmap::CBitmap(LPCTSTR lpszName)
	{
		LoadBitmap(lpszName);
	}

	inline CBitmap::CBitmap(int nID)
	{
		LoadBitmap(nID);
	}

	inline CBitmap::operator HBITMAP() const
	{
		return static_cast<HBITMAP>(GetHandle());
	}

	inline CBitmap::~CBitmap()
	{
	}

	inline BOOL CBitmap::LoadBitmap(int nID)
	// Loads a bitmap from a resource using the resource ID.
	{
		return LoadBitmap(MAKEINTRESOURCE(nID));
	}

	inline BOOL CBitmap::LoadBitmap(LPCTSTR lpszName)
	// Loads a bitmap from a resource using the resource string.
	{
		assert( &GetApp() );

		HBITMAP hBitmap = (HBITMAP)::LoadImage(GetApp().GetResourceHandle(), lpszName, IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR);
		if (hBitmap != 0)
		{
			Attach(hBitmap);
			SetManaged(true);
		}
		return (0 != hBitmap);	// boolean expression
	}

	inline BOOL CBitmap::LoadImage(UINT nID, UINT fuLoad)
	// Loads a bitmap from a resource using the resource ID.
	{
		return LoadImage(MAKEINTRESOURCE(nID), fuLoad);
	}

	inline BOOL CBitmap::LoadImage(LPCTSTR lpszName, UINT fuLoad)
	// Loads a bitmap from a resource using the resource string.
	{
		assert( &GetApp() );

		HBITMAP hBitmap = (HBITMAP)::LoadImage(GetApp().GetResourceHandle(), lpszName, IMAGE_BITMAP, 0, 0, fuLoad);
		if (hBitmap != 0)
		{
			Attach(hBitmap);
			SetManaged(true);
		}
		return (0 != hBitmap);	// boolean expression
	}

	inline BOOL CBitmap::LoadOEMBitmap(UINT nIDBitmap) // for OBM_/OCR_/OIC_
	// Loads a predefined bitmap
	// Predefined bitmaps include: OBM_BTNCORNERS, OBM_BTSIZE, OBM_CHECK, OBM_CHECKBOXES, OBM_CLOSE, OBM_COMBO
	//  OBM_DNARROW, OBM_DNARROWD, OBM_DNARROWI, OBM_LFARROW, OBM_LFARROWD, OBM_LFARROWI, OBM_MNARROW,OBM_OLD_CLOSE
	//  OBM_OLD_DNARROW, OBM_OLD_LFARROW, OBM_OLD_REDUCE, OBM_OLD_RESTORE, OBM_OLD_RGARROW, OBM_OLD_UPARROW
	//  OBM_OLD_ZOOM, OBM_REDUCE, OBM_REDUCED, OBM_RESTORE, OBM_RESTORED, OBM_RGARROW, OBM_RGARROWD, OBM_RGARROWI
	//  OBM_SIZE, OBM_UPARROW, OBM_UPARROWD, OBM_UPARROWI, OBM_ZOOM, OBM_ZOOMD
	{
		HBITMAP hBitmap = ::LoadBitmap(NULL, MAKEINTRESOURCE(nIDBitmap));
		if (hBitmap != 0)
		{
			Attach( ::LoadBitmap(NULL, MAKEINTRESOURCE(nIDBitmap)) );
			SetManaged(true);
		}
		return (0 != hBitmap);	// boolean expression
	}

#ifndef _WIN32_WCE
		inline HBITMAP CBitmap::CreateMappedBitmap(UINT nIDBitmap, UINT nFlags /*= 0*/, LPCOLORMAP lpColorMap /*= NULL*/, int nMapSize /*= 0*/)
		// Creates a new bitmap using the bitmap data and colors specified by the bitmap resource and the color mapping information.
		{
			assert(&GetApp());
			HBITMAP hBitmap = ::CreateMappedBitmap(GetApp().GetResourceHandle(), nIDBitmap, (WORD)nFlags, lpColorMap, nMapSize);
			if (hBitmap == 0)
				throw CResourceException(_T("CreateMappedBitmap failed"));

			Attach(hBitmap);
			SetManaged(true);
			return hBitmap;
		}
#endif // !_WIN32_WCE

		inline HBITMAP CBitmap::CreateBitmap(int nWidth, int nHeight, UINT nPlanes, UINT nBitsPerPixel, LPCVOID lpBits)
		// Creates a bitmap with the specified width, height, and color format (color planes and bits-per-pixel).
		{
			HBITMAP hBitmap = ::CreateBitmap(nWidth, nHeight, nPlanes, nBitsPerPixel, lpBits);
			if (hBitmap == 0)
				throw CResourceException(_T("CreateBitmap failed"));

			Attach(hBitmap);
			SetManaged(true);
			return hBitmap;
		}

#ifndef _WIN32_WCE
		inline HBITMAP CBitmap::CreateBitmapIndirect(const BITMAP& Bitmap)
		// Creates a bitmap with the width, height, and color format specified in the BITMAP structure.
		{
			HBITMAP hBitmap = ::CreateBitmapIndirect(&Bitmap);
			if (hBitmap == 0)
				throw CResourceException(_T("CreateBitmapIndirect failed"));

			Attach(hBitmap);
			SetManaged(true);
			return hBitmap;
		}
#endif // !_WIN32_WCE

		inline HBITMAP CBitmap::CreateCompatibleBitmap(HDC hdc, int nWidth, int nHeight)
		// Creates a bitmap compatible with the device that is associated with the specified device context.
		{
			HBITMAP hBitmap = ::CreateCompatibleBitmap(hdc, nWidth, nHeight);
			if (hBitmap == 0)
				throw CResourceException(_T("CreateCompatibleBitmap"));

			Attach(hBitmap);
			SetManaged(true);
			return hBitmap;
		}

		// Attributes
		inline BITMAP CBitmap::GetBitmapData() const
		// Retrieves the BITMAP structure
		{
			assert(GetHandle() != NULL);
			BITMAP bmp;
			ZeroMemory(&bmp, sizeof(BITMAP));
			::GetObject(GetHandle(), sizeof(BITMAP), &bmp);
			return bmp;
		}

#ifndef _WIN32_WCE
		inline CSize CBitmap::GetBitmapDimensionEx() const
		// Retrieves the dimensions of a compatible bitmap.
		// The retrieved dimensions must have been set by the SetBitmapDimensionEx function.
		{
			assert(GetHandle() != NULL);
			CSize Size;
			::GetBitmapDimensionEx((HBITMAP)GetHandle(), &Size);
			return Size;
		}

		inline CSize CBitmap::SetBitmapDimensionEx(int nWidth, int nHeight) const
		// The SetBitmapDimensionEx function assigns preferred dimensions to a bitmap.
		// These dimensions can be used by applications; however, they are not used by the system.
		{
			assert(GetHandle() != NULL);
			CSize Size;
			::SetBitmapDimensionEx((HBITMAP)GetHandle(), nWidth, nHeight, Size);
			return Size;
		}

		// DIB support
		inline HBITMAP CBitmap::CreateDIBitmap(HDC hdc, const BITMAPINFOHEADER* pbmih, DWORD dwInit, LPCVOID pbInit, const LPBITMAPINFO pbmi, UINT uColorUse)
		// Creates a compatible bitmap (DDB) from a DIB and, optionally, sets the bitmap bits.
		// A CBitmapInfoPtr object can be used for the LPBITMAPINFO parameter.
		{
			HBITMAP hBitmap = ::CreateDIBitmap(hdc, pbmih, dwInit, pbInit, pbmi, uColorUse);
			Attach(hBitmap);
			SetManaged(true);
			return hBitmap;
		}

		inline void CBitmap::GrayScaleBitmap()
		{
			// Create our LPBITMAPINFO object
			CBitmapInfoPtr pbmi(*this);
			BITMAPINFOHEADER& bmiHeader = pbmi->bmiHeader;

			// Create the reference DC for GetDIBits to use
			CMemDC dcMem(NULL);

			// Use GetDIBits to create a DIB from our DDB, and extract the colour data
			GetDIBits(dcMem, 0, bmiHeader.biHeight, NULL, pbmi, DIB_RGB_COLORS);
			std::vector<byte> vBits(bmiHeader.biSizeImage, 0);
			byte* pByteArray = &vBits[0];

			dcMem.GetDIBits(*this, 0, bmiHeader.biHeight, pByteArray, pbmi, DIB_RGB_COLORS);
			UINT nWidthBytes = bmiHeader.biSizeImage/bmiHeader.biHeight;

			int yOffset = 0;
			int xOffset;
			size_t Index;

			for (int Row=0; Row < bmiHeader.biHeight; ++Row)
			{
				xOffset = 0;

				for (int Column=0; Column < bmiHeader.biWidth; ++Column)
				{
					// Calculate Index
					Index = yOffset + xOffset;

					BYTE byGray = (BYTE) ((pByteArray[Index] + pByteArray[Index+1]*6 + pByteArray[Index+2] *3)/10);
					pByteArray[Index]   = byGray;
					pByteArray[Index+1] = byGray;
					pByteArray[Index+2] = byGray;

					// Increment the horizontal offset
					xOffset += bmiHeader.biBitCount >> 3;
				}

				// Increment vertical offset
				yOffset += nWidthBytes;
			}

			// Save the modified colour back into our source DDB
			SetDIBits(dcMem, 0, bmiHeader.biHeight, pByteArray, pbmi, DIB_RGB_COLORS);
		}

		inline void CBitmap::TintBitmap (int cRed, int cGreen, int cBlue)
		// Modifies the colour of the Device Dependant Bitmap, by the colour
		// correction values specified. The correction values can range from -255 to +255.
		// This function gains its speed by accessing the bitmap colour information
		// directly, rather than using GetPixel/SetPixel.
		{
			// Create our LPBITMAPINFO object
			CBitmapInfoPtr pbmi(*this);
			BITMAPINFOHEADER& bmiHeader = pbmi->bmiHeader;

			bmiHeader.biBitCount = 24;

			// Create the reference DC for GetDIBits to use
			CMemDC dcMem(NULL);

			// Use GetDIBits to create a DIB from our DDB, and extract the colour data
			GetDIBits(dcMem, 0, bmiHeader.biHeight, NULL, pbmi, DIB_RGB_COLORS);
			std::vector<byte> vBits(bmiHeader.biSizeImage, 0);
			byte* pByteArray = &vBits[0];

			dcMem.GetDIBits(*this, 0, bmiHeader.biHeight, pByteArray, pbmi, DIB_RGB_COLORS);
			UINT nWidthBytes = bmiHeader.biSizeImage/bmiHeader.biHeight;

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
			int Index;
			for (int Row=0; Row < bmiHeader.biHeight; ++Row)
			{
				xOffset = 0;

				for (int Column=0; Column < bmiHeader.biWidth; ++Column)
				{
					// Calculate Index
					Index = yOffset + xOffset;

					// Adjust the colour values
					if (cBlue > 0)
						pByteArray[Index]   = (BYTE)(cBlue + (((pByteArray[Index] *b1)) >>8));
					else if (cBlue < 0)
						pByteArray[Index]   = (BYTE)((pByteArray[Index] *b2) >>8);

					if (cGreen > 0)
						pByteArray[Index+1] = (BYTE)(cGreen + (((pByteArray[Index+1] *g1)) >>8));
					else if (cGreen < 0)
						pByteArray[Index+1] = (BYTE)((pByteArray[Index+1] *g2) >>8);

					if (cRed > 0)
						pByteArray[Index+2] = (BYTE)(cRed + (((pByteArray[Index+2] *r1)) >>8));
					else if (cRed < 0)
						pByteArray[Index+2] = (BYTE)((pByteArray[Index+2] *r2) >>8);

					// Increment the horizontal offset
					xOffset += bmiHeader.biBitCount >> 3;
				}

				// Increment vertical offset
				yOffset += nWidthBytes;
			}

			// Save the modified colour back into our source DDB
			SetDIBits(dcMem, 0, bmiHeader.biHeight, pByteArray, pbmi, DIB_RGB_COLORS);
		}

#endif // !_WIN32_WCE

		inline HBITMAP CBitmap::CreateDIBSection(HDC hdc, const LPBITMAPINFO pbmi, UINT uColorUse, LPVOID* ppvBits, HANDLE hSection, DWORD dwOffset)
		// Creates a DIB that applications can write to directly. The function gives you a pointer to the location of the bitmap bit values.
		// You can supply a handle to a file-mapping object that the function will use to create the bitmap, or you can let the system allocate the memory for the bitmap.
		// A CBitmapInfoPtr object can be used for the LPBITMAPINFO parameter.
		{
			HBITMAP hBitmap = ::CreateDIBSection(hdc, pbmi, uColorUse, ppvBits, hSection, dwOffset);
			Attach(hBitmap);
			SetManaged(true);
			return hBitmap;
		}

#ifndef _WIN32_WCE
		inline int CBitmap::GetDIBits(HDC hdc, UINT uStartScan, UINT cScanLines,  LPVOID pBits, LPBITMAPINFO pbmi, UINT uColorUse) const
		// Retrieves the bits of the specified compatible bitmap and copies them into a buffer as a DIB using the specified format.
		// A CBitmapInfoPtr object can be used for the LPBITMAPINFO parameter.
		{
			assert(GetHandle() != NULL);
			return ::GetDIBits(hdc, (HBITMAP)GetHandle(), uStartScan, cScanLines,  pBits, pbmi, uColorUse);
		}

		inline int CBitmap::SetDIBits(HDC hdc, UINT uStartScan, UINT cScanLines, LPCVOID pBits, const LPBITMAPINFO pbmi, UINT uColorUse) const
		// Sets the pixels in a compatible bitmap (DDB) using the color data found in the specified DIB.
		// A CBitmapInfoPtr object can be used for the LPBITMAPINFO parameter.
		{
			assert(GetHandle() != NULL);
			return ::SetDIBits(hdc, (HBITMAP)GetHandle(), uStartScan, cScanLines, pBits, pbmi, uColorUse);
		}
#endif // !_WIN32_WCE


	///////////////////////////////////////////////
	// Definitions of the CBrush class
	//
	inline CBrush::CBrush()
	{
	}

	inline CBrush::CBrush(HBRUSH hBrush)
	{
		Attach(hBrush);
	}

	inline CBrush::CBrush(COLORREF crColor)
	{
		try
		{
			CreateSolidBrush(crColor);
		}

		catch(...)
		{
			Release();	// Cleanup
			throw;		// Rethrow
		}
	}

	inline CBrush::operator HBRUSH() const
	{
		return static_cast<HBRUSH>(GetHandle());
	}

	inline CBrush::~CBrush()
	{
	}

	inline HBRUSH CBrush::CreateSolidBrush(COLORREF crColor)
	// Creates a logical brush that has the specified solid color.
	{
		HBRUSH hBrush = ::CreateSolidBrush(crColor);
		if (hBrush == 0)
			throw CResourceException(_T("CreateSolidBrush failed"));

		Attach(hBrush);
		SetManaged(true);
		return hBrush;
	}

#ifndef _WIN32_WCE
	inline HBRUSH CBrush::CreateHatchBrush(int nIndex, COLORREF crColor)
	// Creates a logical brush that has the specified hatch pattern and color.
	{
		HBRUSH hBrush = ::CreateHatchBrush(nIndex, crColor);
		if (hBrush == 0)
			throw CResourceException(_T("CreateHatchBrush failed"));

		Attach(hBrush);
		SetManaged(true);
		return hBrush;
	}

	inline HBRUSH CBrush::CreateBrushIndirect(const LOGBRUSH& LogBrush)
	// Creates a logical brush from style, color, and pattern specified in the LOGPRUSH struct.
	{
		HBRUSH hBrush = ::CreateBrushIndirect(&LogBrush);
		if (hBrush == 0)
			throw CResourceException(_T("CreateBrushIndirect failed"));

		Attach(hBrush);
		SetManaged(true);
		return hBrush;
	}

	inline HBRUSH CBrush::CreateDIBPatternBrush(HGLOBAL hglbDIBPacked, UINT fuColorSpec)
	// Creates a logical brush that has the pattern specified by the specified device-independent bitmap (DIB).
	{
		HBRUSH hBrush = ::CreateDIBPatternBrush(hglbDIBPacked, fuColorSpec);
		if (hBrush == 0)
			throw CResourceException(_T("CreateDIBPatternBrush failed"));

		Attach(hBrush);
		SetManaged(true);
		return hBrush;
	}

	inline HBRUSH CBrush::CreateDIBPatternBrushPt(LPCVOID lpPackedDIB, UINT nUsage)
	// Creates a logical brush that has the pattern specified by the device-independent bitmap (DIB).
	{
		HBRUSH hBrush = ::CreateDIBPatternBrushPt(lpPackedDIB, nUsage);
		if (hBrush == 0)
			throw CResourceException(_T("CreateDIBPatternBrushPt failed"));

		Attach(hBrush);
		SetManaged(true);
		return hBrush;
	}

#endif // !defined(_WIN32_WCE)

	inline HBRUSH CBrush::CreatePatternBrush(HBITMAP hBitmap)
	// Creates a logical brush with the specified bitmap pattern. The bitmap can be a DIB section bitmap,
	// which is created by the CreateDIBSection function, or it can be a device-dependent bitmap.
	{
		HBRUSH hBrush = ::CreatePatternBrush(hBitmap);
		if (hBrush == 0)
			throw CResourceException(_T("CreatePatternBrush failed"));

		Attach(hBrush);
		SetManaged(true);
		return hBrush;
	}

	inline LOGBRUSH CBrush::GetLogBrush() const
	// Retrieves the LOGBRUSH structure that defines the style, color, and pattern of a physical brush.
	{
		assert(GetHandle() != NULL);
		LOGBRUSH LogBrush;
		ZeroMemory(&LogBrush, sizeof(LOGBRUSH));
		::GetObject (GetHandle(), sizeof(LOGBRUSH), &LogBrush);
		return LogBrush;
	}


	///////////////////////////////////////////////
	// Definitions of the CFont class
	//
	inline CFont::CFont()
	{
	}

	inline CFont::CFont(HFONT hFont)
	{
		Attach(hFont);
	}

	inline CFont::CFont(const LOGFONT& LogFont)
	{
		try
		{
			CreateFontIndirect(LogFont);
		}

		catch(...)
		{
			Release();	// Cleanup
			throw;		// Rethrow;
		}
	}

	inline CFont::operator HFONT() const
	{
		return static_cast<HFONT>(GetHandle());
	}

	inline CFont::~CFont()
	{
	}

	inline HFONT CFont::CreateFontIndirect(const LOGFONT& LogFont)
	// Creates a logical font that has the specified characteristics.
	{
		HFONT hFont = ::CreateFontIndirect(&LogFont);
		if (hFont == 0)
			throw CResourceException(_T("CreateFontIndirect"));

		Attach(hFont);
		SetManaged(true);
		return hFont;
	}

	inline HFONT CFont::CreatePointFont(int nPointSize, LPCTSTR lpszFaceName, HDC hdc /*= NULL*/, BOOL IsBold /*= FALSE*/, BOOL IsItalic /*= FALSE*/)
	// Creates a font of a specified typeface and point size.
	{
		LOGFONT logFont;
		ZeroMemory(&logFont, sizeof(LOGFONT));
		logFont.lfCharSet = DEFAULT_CHARSET;
		logFont.lfHeight = nPointSize;

		lstrcpyn(logFont.lfFaceName, lpszFaceName, LF_FACESIZE);

		if (IsBold)
			logFont.lfWeight = FW_BOLD;
		if (IsItalic)
			logFont.lfItalic = (BYTE)TRUE;

		return CreatePointFontIndirect(logFont, hdc);
	}

	inline HFONT CFont::CreatePointFontIndirect(const LOGFONT& LogFont, HDC hdc /* = NULL*/)
	// Creates a font of a specified typeface and point size.
	// This function automatically converts the height in lfHeight to logical units using the specified device context.
	{
		HDC hDC1 = (hdc != NULL) ? hdc : ::GetDC(HWND_DESKTOP);
		CDC dc(hDC1);

		// convert nPointSize to logical units based on hDC
		LOGFONT logFont = LogFont;

#ifndef _WIN32_WCE
		POINT pt = { 0, 0 };
		pt.y = ::MulDiv(::GetDeviceCaps(dc, LOGPIXELSY), logFont.lfHeight, 720);   // 72 points/inch, 10 decipoints/point
		::DPtoLP(dc, &pt, 1);

		POINT ptOrg = { 0, 0 };
		::DPtoLP(dc, &ptOrg, 1);

		logFont.lfHeight = -abs(pt.y - ptOrg.y);
#else // CE specific
		// DP and LP are always the same on CE
		logFont.lfHeight = -abs(((::GetDeviceCaps(hDC1, LOGPIXELSY)* logFont.lfHeight)/ 720));
#endif // _WIN32_WCE

		return CreateFontIndirect (logFont);
	}

#ifndef _WIN32_WCE

	inline HFONT CFont::CreateFont(int nHeight, int nWidth, int nEscapement,
			int nOrientation, int nWeight, DWORD dwItalic, DWORD dwUnderline,
			DWORD dwStrikeOut, DWORD dwCharSet, DWORD dwOutPrecision,
			DWORD dwClipPrecision, DWORD dwQuality, DWORD dwPitchAndFamily,
			LPCTSTR lpszFacename)
	// Creates a logical font with the specified characteristics.
	{
		HFONT hFont = ::CreateFont(nHeight, nWidth, nEscapement,
			nOrientation, nWeight, dwItalic, dwUnderline, dwStrikeOut,
			dwCharSet, dwOutPrecision, dwClipPrecision, dwQuality,
			dwPitchAndFamily, lpszFacename);

		if (hFont == 0)
			throw CResourceException(_T("CreateFont failed"));

		Attach(hFont);
		SetManaged(true);
		return hFont;
	}
#endif // #ifndef _WIN32_WCE

	inline LOGFONT CFont::GetLogFont() const
	// Retrieves the Logfont structure that contains font attributes.
	{
		assert(GetHandle() != NULL);
		LOGFONT LogFont;
		ZeroMemory(&LogFont, sizeof(LOGFONT));
		::GetObject(GetHandle(), sizeof(LOGFONT), &LogFont);
		return LogFont;
	}


	///////////////////////////////////////////////
	// Definitions of the CPalette class
	//
	inline CPalette::CPalette()
	{
	}

	inline CPalette::CPalette(HPALETTE hPalette)
	{
		Attach(hPalette);
	}

	inline CPalette::operator HPALETTE() const
	{
		return static_cast<HPALETTE>(GetHandle());
	}

	inline CPalette::~CPalette ()
	{
	}

	inline HPALETTE CPalette::CreatePalette(LPLOGPALETTE lpLogPalette)
	// Creates a logical palette from the information in the specified LOGPALETTE structure.
	{
		HPALETTE hPalette = ::CreatePalette (lpLogPalette);
		if (hPalette == 0)
			throw CResourceException(_T("CreatePalette failed"));

		Attach(hPalette);
		SetManaged(true);
		return hPalette;
	}

#ifndef _WIN32_WCE
	inline HPALETTE CPalette::CreateHalftonePalette(HDC hdc)
	// Creates a halftone palette for the specified device context (DC).
	{
		HPALETTE hPalette = ::CreateHalftonePalette(hdc);
		if (hPalette == 0)
			throw CResourceException(_T("CreateHalftonePalette failed"));

		Attach(hPalette);
		::RealizePalette(hdc);
		SetManaged(true);
		return hPalette;
	}
#endif // !_WIN32_WCE

	inline int CPalette::GetEntryCount() const
	// Retrieve the number of entries in the palette.
	{
		assert(GetHandle() != NULL);
		WORD nEntries = 0;
		::GetObject(GetHandle(), sizeof(WORD), &nEntries);
		return static_cast<int>(nEntries);
	}

	inline UINT CPalette::GetPaletteEntries(UINT nStartIndex, UINT nNumEntries, LPPALETTEENTRY lpPaletteColors) const
	// Retrieves a specified range of palette entries from the palette.
	{
		assert(GetHandle() != NULL);
		return ::GetPaletteEntries((HPALETTE)GetHandle(), nStartIndex, nNumEntries, lpPaletteColors);
	}

	inline UINT CPalette::SetPaletteEntries(UINT nStartIndex, UINT nNumEntries, LPPALETTEENTRY lpPaletteColors) const
	// Sets RGB (red, green, blue) color values and flags in a range of entries in the palette.
	{
		assert(GetHandle() != NULL);
		return ::SetPaletteEntries((HPALETTE)GetHandle(), nStartIndex, nNumEntries, lpPaletteColors);
	}

#ifndef _WIN32_WCE
	inline void CPalette::AnimatePalette(UINT nStartIndex, UINT nNumEntries, LPPALETTEENTRY lpPaletteColors) const
	// Replaces entries in the palette.
	{
		assert(GetHandle() != NULL);
		::AnimatePalette((HPALETTE)GetHandle(), nStartIndex, nNumEntries, lpPaletteColors);
	}

	inline BOOL CPalette::ResizePalette(UINT nNumEntries) const
	//  Increases or decreases the size of the palette based on the specified value.
	{
		assert(GetHandle() != NULL);
		return ::ResizePalette((HPALETTE)GetHandle(), nNumEntries);
	}
#endif // !_WIN32_WCE

	inline UINT CPalette::GetNearestPaletteIndex(COLORREF crColor) const
	// Retrieves the index for the entry in the palette most closely matching a specified color value.
	{
		assert(GetHandle() != NULL);
		return ::GetNearestPaletteIndex((HPALETTE)GetHandle(), crColor);
	}


	///////////////////////////////////////////////
	// Declarations for the CPen class
	//
	inline CPen::CPen()
	{
	}

	inline CPen::CPen(HPEN hPen)
	{
		Attach(hPen);
	}

	inline CPen::CPen(int nPenStyle, int nWidth, COLORREF crColor)
	{
		try
		{
			CreatePen(nPenStyle, nWidth, crColor);
		}

		catch(...)
		{
			Release();	// Cleanup
			throw;		// Rethrow
		}
	}

#ifndef _WIN32_WCE
	inline CPen::CPen(int nPenStyle, int nWidth, const LOGBRUSH& LogBrush, int nStyleCount /*= 0*/, const DWORD* lpStyle /*= NULL*/)
	{
		try
		{
			Attach(::ExtCreatePen(nPenStyle, nWidth, &LogBrush, nStyleCount, lpStyle));
		}

		catch(...)
		{
			Release();	// Cleanup
			throw;		// Rethrow
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

	inline HPEN CPen::CreatePen(int nPenStyle, int nWidth, COLORREF crColor)
	// Creates a logical pen that has the specified style, width, and color.
	{
		HPEN hPen = ::CreatePen(nPenStyle, nWidth, crColor);
		Attach(hPen);
		SetManaged(true);
		return hPen;
	}

	inline HPEN CPen::CreatePenIndirect(const LOGPEN& LogPen)
	// Creates a logical pen that has the style, width, and color specified in a structure.
	{
		HPEN hPen = ::CreatePenIndirect(&LogPen);
		Attach(hPen);
		SetManaged(true);
		return hPen;
	}

	inline LOGPEN CPen::GetLogPen() const
	{
		// Retrieves the LOGPEN struct that specifies the pen's style, width, and color.
		assert(GetHandle() != NULL);

		LOGPEN LogPen;
		ZeroMemory(&LogPen, sizeof(LOGPEN));
		::GetObject(GetHandle(), sizeof(LOGPEN), &LogPen);
		return LogPen;
	}

#ifndef _WIN32_WCE
	inline HPEN CPen::ExtCreatePen(int nPenStyle, int nWidth, const LOGBRUSH& LogBrush, int nStyleCount /* = 0*/, const DWORD* lpStyle /*= NULL*/)
	// Creates a logical cosmetic or geometric pen that has the specified style, width, and brush attributes.
	{
		HPEN hPen = ::ExtCreatePen(nPenStyle, nWidth, &LogBrush, nStyleCount, lpStyle);
		Attach(hPen);
		SetManaged(true);
		return hPen;
	}

	inline EXTLOGPEN CPen::GetExtLogPen() const
	// Retrieves the EXTLOGPEN struct that specifies the pen's style, width, color and brush attributes.
	{
		assert(GetHandle() != NULL);

		EXTLOGPEN ExLogPen;
		ZeroMemory(&ExLogPen, sizeof(EXTLOGPEN));
		::GetObject(GetHandle(), sizeof(EXTLOGPEN), &ExLogPen);
		return ExLogPen;
	}
#endif // !_WIN32_WCE


	///////////////////////////////////////////////
	// Definitions of the CRgn class
	//
	inline CRgn::CRgn()
	{
	}

	inline CRgn::CRgn(HRGN hRgn)
	{
		Attach(hRgn);
	}

	inline CRgn::operator HRGN() const
	{
		return static_cast<HRGN>(GetHandle());
	}

	inline CRgn::~CRgn()
	{
	}

	inline HRGN CRgn::CreateRectRgn(int x1, int y1, int x2, int y2)
	// Creates a rectangular region.
	{
		HRGN hRgn = ::CreateRectRgn(x1, y1, x2, y2);
		if (hRgn == 0)
			throw CResourceException(_T("CreateRectRgn failed"));

		Attach(hRgn);
		SetManaged(true);
		return hRgn;
	}

	inline HRGN CRgn::CreateRectRgnIndirect(const RECT& rc)
	// Creates a rectangular region.
	{
		HRGN hRgn = ::CreateRectRgnIndirect(&rc);
		if (hRgn == 0)
			throw CResourceException(_T("CreateRectRgnIndirect failed"));

		Attach(hRgn);
		SetManaged(true);
		return hRgn;
	}

#ifndef _WIN32_WCE
	inline HRGN CRgn::CreateEllipticRgn(int x1, int y1, int x2, int y2)
	// Creates an elliptical region.
	{
		HRGN hRgn = ::CreateEllipticRgn(x1, y1, x2, y2);
		if (hRgn == 0)
			throw CResourceException(_T("CreateEllipticRgn failed"));

		Attach(hRgn);
		SetManaged(true);
		return hRgn;
	}

	inline HRGN CRgn::CreateEllipticRgnIndirect(const RECT& rc)
	// Creates an elliptical region.
	{
		HRGN hRgn = ::CreateEllipticRgnIndirect(&rc);
		if (hRgn == 0)
			throw CResourceException(_T("CreateEllipticRgnIndirect failed"));

		Attach(hRgn);
		SetManaged(true);
		return hRgn;
	}

	inline HRGN CRgn::CreatePolygonRgn(LPPOINT lpPoints, int nCount, int nMode)
	// Creates a polygonal region.
	{
		HRGN hRgn = ::CreatePolygonRgn(lpPoints, nCount, nMode);
		if (hRgn == 0)
			throw CResourceException(_T("CreatePolygonRgn failed"));

		Attach(hRgn);
		SetManaged(true);
		return hRgn;
	}

	inline HRGN CRgn::CreatePolyPolygonRgn(LPPOINT lpPoints, LPINT lpPolyCounts, int nCount, int nPolyFillMode)
	// Creates a region consisting of a series of polygons. The polygons can overlap.
	{
		HRGN hRgn = ::CreatePolyPolygonRgn(lpPoints, lpPolyCounts, nCount, nPolyFillMode);
		if (hRgn == 0)
			throw CResourceException(_T("CreatePolyPolygonRgn failed"));

		Attach(hRgn);
		SetManaged(true);
		return hRgn;
	}

	inline HRGN CRgn::CreateRoundRectRgn(int x1, int y1, int x2, int y2, int x3, int y3)
	// Creates a rectangular region with rounded corners.
	{
		HRGN hRgn = ::CreateRoundRectRgn(x1, y1, x2, y2, x3, y3);
		if (hRgn == 0)
			throw CResourceException(_T("CreateRoundRectRgn failed"));

		Attach(hRgn);
		SetManaged(true);
		return hRgn;
	}

	inline HRGN CRgn::CreateFromPath(HDC hDC)
	// Creates a region from the path that is selected into the specified device context.
	// The resulting region uses device coordinates.
	{
		assert(hDC != NULL);
		HRGN hRgn = ::PathToRegion(hDC);
		if (hRgn == 0)
			throw CResourceException(_T("PathToRegion failed"));

		Attach(hRgn);
		SetManaged(true);
		return hRgn;
	}

#endif // !_WIN32_WCE

	inline HRGN CRgn::CreateFromData(const XFORM* lpXForm, int nCount, const RGNDATA* pRgnData)
	// Creates a region from the specified region and transformation data.
	// If the XFORM pointer is NULL, the identity transformation is used. 
	{
		HRGN hRgn = ::ExtCreateRegion(lpXForm, nCount, pRgnData);
		if (hRgn == 0)
			throw CResourceException(_T("ExtCreateRegion failed"));

		Attach(hRgn);
		SetManaged(true);
		return hRgn;
	}

	inline void CRgn::SetRectRgn(int x1, int y1, int x2, int y2) const
	// converts the region into a rectangular region with the specified coordinates.
	{
		assert(GetHandle() != NULL);
		::SetRectRgn((HRGN)GetHandle(), x1, y1, x2, y2);
	}

	inline void CRgn::SetRectRgn(const RECT& rc) const
	// converts the region into a rectangular region with the specified coordinates.
	{
		assert(GetHandle() != NULL);
		::SetRectRgn((HRGN)GetHandle(), rc.left, rc.top, rc.right, rc.bottom);
	}

	inline int CRgn::CombineRgn(HRGN hrgnSrc1, HRGN hrgnSrc2, int nCombineMode) const
	// Combines two specified regions and stores the result.
	{
		assert(GetHandle() != NULL);
		return ::CombineRgn((HRGN)GetHandle(), hrgnSrc1, hrgnSrc2, nCombineMode);
	}

	inline int CRgn::CombineRgn(HRGN hrgnSrc, int nCombineMode) const
	// Combines the specified region with the current region.
	{
		assert(GetHandle() != NULL);
		return ::CombineRgn((HRGN)GetHandle(), (HRGN)GetHandle(), hrgnSrc, nCombineMode);
	}

	inline int CRgn::CopyRgn(HRGN hrgnSrc) const
	// Assigns the specified region to the current region.
	{
		assert(GetHandle() != NULL);
		assert(hrgnSrc);
		return ::CombineRgn((HRGN)GetHandle(), hrgnSrc, NULL, RGN_COPY);
	}

	inline BOOL CRgn::EqualRgn(HRGN hRgn) const
	// Checks the two specified regions to determine whether they are identical.
	{
		assert(GetHandle() != NULL);
		return ::EqualRgn((HRGN)GetHandle(), hRgn);
	}

	inline int CRgn::OffsetRgn(int x, int y) const
	// Moves a region by the specified offsets.
	{
		assert(GetHandle() != NULL);
		return ::OffsetRgn((HRGN)GetHandle(), x, y);
	}

	inline int CRgn::OffsetRgn(POINT& pt) const
	// Moves a region by the specified offsets.
	{
		assert(GetHandle() != NULL);
		return ::OffsetRgn((HRGN)GetHandle(), pt.x, pt.y);
	}

	inline int CRgn::GetRgnBox(RECT& rc) const
	// Retrieves the bounding rectangle of the region, and stores it in the specified RECT.
	// The return value indicates the region's complexity: NULLREGION;SIMPLEREGION; or COMPLEXREGION.
	{
		assert(GetHandle() != NULL);
		return ::GetRgnBox((HRGN)GetHandle(), &rc);
	}

	inline int CRgn::GetRegionData(LPRGNDATA lpRgnData, int nDataSize) const
	// Fills the specified buffer with data describing a region.
	{
		assert(GetHandle() != NULL);
		return static_cast<int>(::GetRegionData((HRGN)GetHandle(), nDataSize, lpRgnData));
	}

	inline BOOL CRgn::PtInRegion(int x, int y) const
	// Determines whether the specified point is inside the specified region.
	{
		assert(GetHandle() != NULL);
		return ::PtInRegion((HRGN)GetHandle(), x, y);
	}

	inline BOOL CRgn::PtInRegion(POINT& pt) const
	// Determines whether the specified point is inside the specified region.
	{
		assert(GetHandle() != NULL);
		return ::PtInRegion((HRGN)GetHandle(), pt.x, pt.y);
	}

	inline BOOL CRgn::RectInRegion(const RECT& rc) const
	// Determines whether the specified rect is inside the specified region.
	{
		assert(GetHandle() != NULL);
		return ::RectInRegion((HRGN)GetHandle(), &rc);
	}


	///////////////////////////////////////////////
	// Definitions of the CDC class
	//
	inline CDC::CDC()
	{
		// Allocate memory for our data members
		m_pData = new CDC_Data;
	}

inline CDC::CDC(HDC hDC, HWND hWnd /*= 0*/)
	// This constructor assigns an existing HDC to the CDC
	// The HDC WILL be released or deleted when the CDC object is destroyed
	// The hWnd parameter is only used in WindowsCE. It specifies the HWND of a Window or
	// Window Client DC

	// Note: this constructor permits a call like this:
	// CDC MyCDC = SomeHDC;
	{
		m_pData = new CDC_Data;
		Attach(hDC, hWnd);
	}

#ifndef _WIN32_WCE
	inline void CDC::operator = (const HDC hDC)
	// Note: this assignment operator permits a call like this:
	// CDC MyCDC;
	// MyCDC = SomeHDC;
	{
		Attach(hDC);
	}
#endif

	inline CDC::CDC(const CDC& rhs)	// Copy constructor
	// The copy constructor is called when a temporary copy of the CDC needs to be created.
	// This can happen when a CDC is passed by value in a function call. Each CDC copy manages
	// the same Device Context and GDI objects.
	{
		m_pData = rhs.m_pData;
		InterlockedIncrement(&m_pData->Count);
	}

	inline CDC& CDC::operator = (const CDC& rhs)
	// Note: A copy of a CDC is a clone of the original.
	//       Both objects manipulate the one HDC
	{
		if (this != &rhs)
		{
			InterlockedIncrement(&rhs.m_pData->Count);
			Release();
			m_pData = rhs.m_pData;
		}

		return *this;
	}

	inline CDC::~CDC ()
	{
		Release();
	}

	inline void CDC::AddToMap()
	// Store the HDC and CDC pointer in the HDC map
	{
		assert( &GetApp() );
		assert(m_pData->hDC);

		GetApp().AddCDCData(m_pData->hDC, m_pData);
	}

	inline void CDC::Attach(HDC hDC, HWND hWnd /* = 0*/)
	// Attaches a HDC to the CDC object.
	// Window DCs and Client DCs have a HWND. This must be specified on WinCE, as WinCE
	// doesn't support WindowFromDC. The alternative is to use CClientDC or CWindowDC
	// as appropriate.
	{
		UNREFERENCED_PARAMETER(hWnd);
		assert(m_pData);

		if (hDC != m_pData->hDC)
		{
			if (m_pData->hDC)
			{
				Release();

				// Assign values to our data members
				m_pData = new CDC_Data;
			}

			if (hDC)
			{
				CDC_Data* pCDCData = GetApp().GetCDCData(hDC);
				if (pCDCData)
				{
					delete m_pData;
					m_pData = pCDCData;
					InterlockedIncrement(&m_pData->Count);
				}
				else
				{
					m_pData->hDC = hDC;

		#ifndef _WIN32_WCE
					m_pData->hWnd = ::WindowFromDC(hDC);
		#else
					m_pData->hWnd = hWnd;
		#endif

					AddToMap();
					m_pData->nSavedDCState = ::SaveDC(hDC);
				}
			}
		}
	}

	inline HDC CDC::Detach()
	// Detaches the HDC from all CDC objects.
	{
		assert(m_pData);
		assert(m_pData->hDC);

		HDC hDC = m_pData->hDC;
		RemoveFromMap();
		m_pData->hDC = 0;

		if (m_pData->Count > 0)
		{
			if (InterlockedDecrement(&m_pData->Count) == 0)
			{
				delete m_pData;
			}
		}

		// Assign values to our data members
		m_pData = new CDC_Data;

		return hDC;
	}

#ifndef _WIN32_WCE
	inline int CDC::EnumObjects(int nObjectType, GOBJENUMPROC lpObjectFunc, LPARAM lParam) const
	// Enumerates the pens or brushes available for the device context. This
	// function calls the application-defined callback function once for each available object,
	// supplying data describing that object.
	{
		assert(m_pData->hDC);
		return ::EnumObjects(m_pData->hDC, nObjectType, lpObjectFunc, lParam);
	}
#endif

	// Initialization
	inline HDC CDC::CreateCompatibleDC(HDC hdcSource)
	// Returns a memory device context (DC) compatible with the specified device.
	{
		assert(m_pData->hDC == NULL);
		HDC hDC = ::CreateCompatibleDC(hdcSource);

		if (hDC == NULL)
			throw CResourceException(_T("CreateCompatibleDC failed"));

		m_pData->hDC = hDC;
		m_pData->IsManagedHDC = TRUE;
		AddToMap();

		return hDC;
	}


	inline HDC CDC::CreateDC(LPCTSTR lpszDriver, LPCTSTR lpszDevice, LPCTSTR lpszOutput, const DEVMODE* pInitData)
	// Returns a device context (DC) for a device using the specified name.
	{
		assert(m_pData->hDC == NULL);
		HDC hDC = ::CreateDC(lpszDriver, lpszDevice, lpszOutput, pInitData);

		if (hDC == NULL)
			throw CResourceException(_T("CreateDC failed"));

		m_pData->hDC = hDC;
		m_pData->IsManagedHDC = TRUE;
		AddToMap();
		return hDC;
	}

#ifndef _WIN32_WCE
	inline HDC CDC::CreateIC(LPCTSTR lpszDriver, LPCTSTR lpszDevice, LPCTSTR lpszOutput, const DEVMODE* pInitData)
	{
		assert(m_pData->hDC == NULL);
		HDC hDC = ::CreateIC(lpszDriver, lpszDevice, lpszOutput, pInitData);

		if (hDC == 0)
			throw CResourceException(_T("CreateIC failed"));

		m_pData->hDC = hDC;
		m_pData->IsManagedHDC = TRUE;
		AddToMap();
		return hDC;
	}
#endif

	inline void CDC::DrawBitmap(int x, int y, int cx, int cy, HBITMAP hBitmap, COLORREF clrMask) const
	// Draws the specified bitmap to the specified DC using the mask colour provided as the transparent colour
	// Suitable for use with a Window DC or a memory DC
	{
		// Create the Image memory DC
		CMemDC dcImage(*this);
		dcImage.SetBkColor(clrMask);
		::SelectObject(dcImage, hBitmap);

		// Create the Mask memory DC
		CMemDC dcMask(*this);
		dcMask.CreateBitmap(cx, cy, 1, 1, NULL);
		dcMask.BitBlt(0, 0, cx, cy, dcImage, 0, 0, SRCCOPY);

		// Mask the image to 'this' DC
		BitBlt(x, y, cx, cy, dcImage, 0, 0, SRCINVERT);
		BitBlt(x, y, cx, cy, dcMask, 0, 0, SRCAND);
		BitBlt(x, y, cx, cy, dcImage, 0, 0, SRCINVERT);
	}

	inline void CDC::GradientFill(COLORREF Color1, COLORREF Color2, const RECT& rc, BOOL IsVertical) const
	// An efficient color gradient filler compatible with all Windows operating systems
	{
		int Width = rc.right - rc.left;
		int Height = rc.bottom - rc.top;

		int r1 = GetRValue(Color1);
		int g1 = GetGValue(Color1);
		int b1 = GetBValue(Color1);

		int r2 = GetRValue(Color2);
		int g2 = GetGValue(Color2);
		int b2 = GetBValue(Color2);

		COLORREF OldBkColor = GetBkColor();

		if (IsVertical)
		{
			for(int i=0; i < Width; ++i)
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
			for(int i=0; i < Height; ++i)
			{
				int r = r1 + (i * (r2-r1) / Height);
				int g = g1 + (i * (g2-g1) / Height);
				int b = b1 + (i * (b2-b1) / Height);
				SetBkColor(RGB(r, g, b));
				CRect line(rc.left, i + rc.top, rc.left+Width, i + 1 + rc.top);
				ExtTextOut(0, 0, ETO_OPAQUE, line, NULL, 0, 0);
			}
		}

		SetBkColor(OldBkColor);
	}

	inline void CDC::Release()
	{
		if (m_pData->Count > 0)
		{
			if (InterlockedDecrement(&m_pData->Count) == 0)
			{
				Destroy();
				delete m_pData;
				m_pData = 0;
			}
		}
	}

	inline BOOL CDC::RemoveFromMap()
	{
		BOOL Success = FALSE;

		if( &GetApp() )
		{
			// Allocate an iterator for our Data map
			std::map<HDC, CDC_Data*, CompareHDC>::iterator m;

			CWinApp& App = GetApp();		
			App.m_csMapLock.Lock();
			m = App.m_mapCDCData.find(m_pData->hDC);
			if (m != App.m_mapCDCData.end())
			{
				// Erase the CDC data entry from the map
				App.m_mapCDCData.erase(m);
				Success = TRUE;
			}

			App.m_csMapLock.Release();
		}

		return Success;
	}

	inline BOOL CDC::RestoreDC(int nSavedDC) const
	{
		assert(m_pData->hDC);
		return ::RestoreDC(m_pData->hDC, nSavedDC);
	}
	inline int CDC::SaveDC() const
	{
		assert(m_pData->hDC);
		return ::SaveDC(m_pData->hDC);
	}

	inline HGDIOBJ CDC::SelectObject(HGDIOBJ hObject) const
	{
		assert(m_pData);
		return ::SelectObject(m_pData->hDC, hObject);
	}

	inline HBITMAP CDC::SelectObject(HBITMAP hBitmap) const
	{
		assert(m_pData);
		return (HBITMAP)::SelectObject(m_pData->hDC, hBitmap);
	}

	inline HBRUSH CDC::SelectObject(HBRUSH hBrush) const
	{
		assert(m_pData);
		return (HBRUSH)::SelectObject(m_pData->hDC, hBrush);
	}

	inline HFONT CDC::SelectObject(HFONT hFont) const
	{
		assert(m_pData);
		return (HFONT)::SelectObject(m_pData->hDC, hFont);
	}

	inline HPEN CDC::SelectObject(HPEN hPen) const
	{
		assert(m_pData);
		return (HPEN)::SelectObject(m_pData->hDC, hPen);
	}

	inline int CDC::SelectObject(HRGN hRgn) const
	{
		assert(m_pData);
		return (int)(INT_PTR)::SelectObject(m_pData->hDC, hRgn);
	}

	inline void CDC::SolidFill(COLORREF Color, const RECT& rc) const
	// Fills a rectangle with a solid color
	{
		COLORREF OldColor = SetBkColor(Color);
		ExtTextOut(0, 0, ETO_OPAQUE, rc, NULL, 0, 0);
		SetBkColor(OldColor);
	}

	// Bitmap functions
	inline void CDC::CreateCompatibleBitmap(HDC hdc, int cx, int cy)
	// Creates a compatible bitmap and selects it into the device context.
	{
		assert(m_pData->hDC);

		CBitmap bitmap;
		bitmap.CreateCompatibleBitmap(hdc, cx, cy);
		SelectObject(bitmap);
		m_pData->Bitmap = bitmap;
	}

	inline void CDC::CreateBitmap(int cx, int cy, UINT Planes, UINT BitsPerPixel, LPCVOID pvColors)
	// Creates a bitmap and selects it into the device context.
	{
		assert(m_pData->hDC);

		CBitmap bitmap;
		bitmap.CreateBitmap(cx, cy, Planes, BitsPerPixel, pvColors);
		SelectObject(bitmap);
		m_pData->Bitmap = bitmap;
	}

#ifndef _WIN32_WCE
	inline void CDC::CreateBitmapIndirect (const BITMAP& Bitmap)
	// Creates a bitmap and selects it into the device context.
	{
		assert(m_pData->hDC);

		CBitmap bitmap;
		bitmap.CreateBitmapIndirect(Bitmap);
		SelectObject(bitmap);
		m_pData->Bitmap = bitmap;
	}

	inline void CDC::CreateDIBitmap(HDC hdc, const BITMAPINFOHEADER& bmih, DWORD fdwInit, LPCVOID lpbInit,
										const LPBITMAPINFO pbmi,  UINT fuUsage)
	// Creates a bitmap and selects it into the device context.
	// A CBitmapInfoPtr object can be used for the LPBITMAPINFO parameter.
	{
		assert(m_pData->hDC);

		CBitmap bitmap;
		bitmap.CreateDIBitmap(hdc, &bmih, fdwInit, lpbInit, pbmi, fuUsage);
		SelectObject(bitmap);
		m_pData->Bitmap = bitmap;
	}
#endif

	inline void CDC::CreateDIBSection(HDC hdc, const LPBITMAPINFO pbmi, UINT iUsage, LPVOID* ppvBits,
										HANDLE hSection, DWORD dwOffset)
	// Creates a bitmap and selects it into the device context.
	// A CBitmapInfoPtr object can be used for the LPBITMAPINFO parameter.
	{
		assert(m_pData->hDC);

		CBitmap bitmap;
		bitmap.CreateDIBSection(hdc, pbmi, iUsage, ppvBits, hSection, dwOffset);
		SelectObject(bitmap);
		m_pData->Bitmap = bitmap;
	}

	inline CBitmap CDC::DetachBitmap()
	// Provides a convenient method of detaching a bitmap from a memory device context.
	// Returns the CBitmap detached from the DC.
	// Usage:  CBitmap MyBitmap = MyMemDC.DetachBitmap();
	{
		// Create a stock bitmap to replace the current one.
		CBitmap bitmap;
		bitmap.CreateBitmap(1, 1, 1, 1, 0);
		
		CBitmap OldBitmap = SelectObject(bitmap);
		m_pData->Bitmap = bitmap;

		return OldBitmap;
	}

	inline void CDC::Destroy()
	// Deletes or releases the device context and returns the CDC object to its
	// default state, ready for reuse.
	{
		if (m_pData->hDC != 0)
		{
			RemoveFromMap();

			if (m_pData->IsManagedHDC)
			{
				// Return the DC back to its initial state
				::RestoreDC(m_pData->hDC, m_pData->nSavedDCState);

				// We need to release a Window DC, and delete a memory DC
				if (m_pData->hWnd != 0)
					::ReleaseDC(m_pData->hWnd, m_pData->hDC);
				else
					if (!::DeleteDC(m_pData->hDC))
						::ReleaseDC(NULL, m_pData->hDC);
			}

			m_pData->hDC = 0;
			m_pData->hWnd = 0;
			m_pData->IsManagedHDC = FALSE;
		}
	}

	inline BITMAP CDC::GetBitmapData() const
	// Retrieves the BITMAP information for the current HBITMAP.
	{
		assert(m_pData->hDC);

		HBITMAP hbm = (HBITMAP)::GetCurrentObject(m_pData->hDC, OBJ_BITMAP);
		BITMAP bm;
		ZeroMemory(&bm, sizeof(BITMAP));
		::GetObject(hbm, sizeof(bm), &bm);
		return bm;
	}

	inline HBITMAP CDC::GetCurrentBitmap() const
	// Retrieves the handle of the currently selected bitmap
	{
		assert(m_pData->hDC);
		return static_cast<HBITMAP>(::GetCurrentObject(m_pData->hDC, OBJ_BITMAP));
	}

	inline BOOL CDC::LoadBitmap(UINT nID)
	// Loads a bitmap from the resource and selects it into the device context
	// Returns TRUE if successful
	{
		return LoadBitmap(MAKEINTRESOURCE(nID));
	}

	inline BOOL CDC::LoadBitmap(LPCTSTR lpszName)
	// Loads a bitmap from the resource and selects it into the device context
	// Returns TRUE if successful
	{
		assert(m_pData->hDC);

		CBitmap bitmap;
		BOOL IsLoaded = bitmap.LoadBitmap(lpszName);

		if (IsLoaded)
		{
			SelectObject(bitmap);
			m_pData->Bitmap = bitmap;
		}

		return IsLoaded;
	}

	inline BOOL CDC::LoadImage(UINT nID, UINT fuLoad)
	// Loads a bitmap from the resource and selects it into the device context.
	// The fuLoad parameter can be one of  LR_DEFAULTCOLOR, LR_CREATEDIBSECTION, 
	//	LR_LOADFROMFILE, LR_LOADTRANSPARENT, LR_MONOCHROME, LR_SHARED and LR_VGACOLOR.
	// Returns TRUE if successful
	{
		return LoadImage(nID, fuLoad);
	}

	inline BOOL CDC::LoadImage(LPCTSTR lpszName, UINT fuLoad)
	// Loads a bitmap from the resource and selects it into the device context.
	// The fuLoad parameter can be one of  LR_DEFAULTCOLOR, LR_CREATEDIBSECTION, 
	//	LR_LOADFROMFILE, LR_LOADTRANSPARENT, LR_MONOCHROME, LR_SHARED and LR_VGACOLOR.
	// Returns TRUE if successful
	{
		assert(m_pData->hDC);

		CBitmap bitmap;
		BOOL IsLoaded = bitmap.LoadImage(lpszName, fuLoad);

		if (IsLoaded)
		{
			SelectObject(bitmap);
			m_pData->Bitmap = bitmap;
		}

		return IsLoaded;
	}

	inline BOOL CDC::LoadOEMBitmap(UINT nIDBitmap) // for OBM_/OCR_/OIC_
	// Loads a predefined system bitmap and selects it into the device context
	// Returns TRUE if successful
	{
		assert(m_pData->hDC);

		CBitmap bitmap;
		BOOL IsLoaded = bitmap.LoadOEMBitmap(nIDBitmap);

		if (IsLoaded)
		{
			SelectObject(bitmap);
			m_pData->Bitmap = bitmap;
		}

		return IsLoaded;
	}

#ifndef _WIN32_WCE
	inline void CDC::CreateMappedBitmap(UINT nIDBitmap, UINT nFlags /*= 0*/, LPCOLORMAP lpColorMap /*= NULL*/, int nMapSize /*= 0*/)
	// creates and selects a new bitmap using the bitmap data and colors specified by the bitmap resource and the color mapping information.
	{
		assert(m_pData->hDC);

		CBitmap bitmap;
		bitmap.CreateMappedBitmap(nIDBitmap, (WORD)nFlags, lpColorMap, nMapSize);
		SelectObject(bitmap);
		m_pData->Bitmap = bitmap;
	}
#endif // !_WIN32_WCE


	// Brush functions

	inline void CDC::CreatePatternBrush(HBITMAP hBitmap)
	// Creates the brush with the specified pattern, and selects it into the device context.
	{
		assert(m_pData->hDC);

		CBrush brush;
		brush.CreatePatternBrush(hBitmap);
		SelectObject(brush);
		m_pData->Brush = brush;
	}

	inline void CDC::CreateSolidBrush(COLORREF rgb)
	// Creates the brush with the specified color, and selects it into the device context.
	{
		assert(m_pData->hDC);

		CBrush brush;
		brush.CreateSolidBrush(rgb);
		SelectObject(brush);
		m_pData->Brush = brush;
	}

	inline HBRUSH CDC::GetCurrentBrush() const
	// Retrieves the handle of the currently selected brush object
	{
		assert(m_pData->hDC);
		return static_cast<HBRUSH>(::GetCurrentObject(m_pData->hDC, OBJ_BRUSH));
	}

	inline LOGBRUSH CDC::GetLogBrush() const
	// Retrieves the current brush information
	{
		assert(m_pData->hDC);

		HBRUSH hBrush = (HBRUSH)::GetCurrentObject(m_pData->hDC, OBJ_BRUSH);
		LOGBRUSH lBrush;
		ZeroMemory(&lBrush, sizeof(LOGBRUSH));
		::GetObject(hBrush, sizeof(lBrush), &lBrush);
		return lBrush;
	}

#ifndef _WIN32_WCE
	inline void CDC::CreateBrushIndirect(const LOGBRUSH& LogBrush)
	// Creates the brush and selects it into the device context.
	{
		assert(m_pData->hDC);

		CBrush brush;
		brush.CreateBrushIndirect(LogBrush);
		SelectObject(brush);
		m_pData->Brush = brush;
	}

	inline void CDC::CreateHatchBrush(int fnStyle, COLORREF rgb)
	// Creates a brush with the specified hatch pattern and color, and selects it into the device context.
	{
		assert(m_pData->hDC);

		CBrush brush;
		brush.CreateHatchBrush(fnStyle, rgb);
		SelectObject(brush);
		m_pData->Brush = brush;
	}

	inline void CDC::CreateDIBPatternBrush(HGLOBAL hglbDIBPacked, UINT fuColorSpec)
	// Creates a logical from the specified device-independent bitmap (DIB), and selects it into the device context.
	{
		assert(m_pData->hDC);

		CBrush brush;
		brush.CreateDIBPatternBrush(hglbDIBPacked, fuColorSpec);
		SelectObject(brush);
		m_pData->Brush = brush;
	}

	inline void CDC::CreateDIBPatternBrushPt(LPCVOID lpPackedDIB, UINT iUsage)
	// Creates a logical from the specified device-independent bitmap (DIB), and selects it into the device context.
	{
		assert(m_pData->hDC);

		CBrush brush;
		brush.CreateDIBPatternBrushPt(lpPackedDIB, iUsage);
		SelectObject(brush);
		m_pData->Brush = brush;
	}
#endif


	// Font functions
	inline void CDC::CreateFontIndirect(const LOGFONT& lf)
	// Creates a logical font and selects it into the device context.
	{
		assert(m_pData->hDC);

		CFont font;
		font.CreateFontIndirect(lf);
		SelectObject(font);
		m_pData->Font = font;
	}

	inline HFONT CDC::GetCurrentFont() const
	// Retrieves the handle to the current font object
	{
		assert(m_pData->hDC);
		return static_cast<HFONT>(::GetCurrentObject(m_pData->hDC, OBJ_FONT));
	}

	inline LOGFONT CDC::GetLogFont() const
	// Retrieves the current font information.
	{
		assert(m_pData->hDC);

		HFONT hFont = (HFONT)::GetCurrentObject(m_pData->hDC, OBJ_FONT);
		LOGFONT lFont;
		ZeroMemory(&lFont, sizeof(LOGFONT));
		::GetObject(hFont, sizeof(lFont), &lFont);
		return lFont;
	}

#ifndef _WIN32_WCE
	inline void CDC::CreateFont (
					int nHeight,               // height of font
  					int nWidth,                // average character width
  					int nEscapement,           // angle of escapement
  					int nOrientation,          // base-line orientation angle
  					int fnWeight,              // font weight
  					DWORD fdwItalic,           // italic attribute option
  					DWORD fdwUnderline,        // underline attribute option
  					DWORD fdwStrikeOut,        // strikeout attribute option
  					DWORD fdwCharSet,          // character set identifier
  					DWORD fdwOutputPrecision,  // output precision
  					DWORD fdwClipPrecision,    // clipping precision
  					DWORD fdwQuality,          // output quality
  					DWORD fdwPitchAndFamily,   // pitch and family
  					LPCTSTR lpszFace           // typeface name
 					)

	// Creates a logical font with the specified characteristics.
	{
		assert(m_pData->hDC);

		CFont font;
		font.CreateFont(nHeight, nWidth, nEscapement, nOrientation, fnWeight,
			fdwItalic, fdwUnderline, fdwStrikeOut, fdwCharSet,
			fdwOutputPrecision, fdwClipPrecision, fdwQuality,
			fdwPitchAndFamily, lpszFace);
		SelectObject(font);
		m_pData->Font = font;
	}
#endif

	// Palette functions
	inline void CDC::CreatePalette(LPLOGPALETTE pLogPalette, BOOL ForceBkgnd)
	// Creates and selects a palette
	{
		assert(m_pData->hDC);

		CPalette palette;
		palette.CreatePalette(pLogPalette);
		SelectPalette(palette, ForceBkgnd);
		m_pData->Palette = palette;
		RealizePalette();
	}

	inline HPALETTE CDC::GetCurrentPalette() const
	// Retrieves the handle to the currently selected palette
	{
		assert(m_pData->hDC);
		return static_cast<HPALETTE>(::GetCurrentObject(m_pData->hDC, OBJ_PAL));
	}

	inline COLORREF CDC::GetNearestColor(COLORREF crColor) const
	// Retrieves a color value identifying a color from the system palette that will be
	//  displayed when the specified color value is used.
	{
		assert(m_pData->hDC);
		return GetNearestColor(crColor);
	}

	inline HPALETTE CDC::SelectPalette(const HPALETTE hPalette, BOOL ForceBkgnd)
	// Use this to attach an existing palette.
	{
		assert(m_pData->hDC);
		return static_cast<HPALETTE>(::SelectPalette(m_pData->hDC, hPalette, ForceBkgnd));
	}

	inline void CDC::RealizePalette() const
	// Use this to realize changes to the device context palette.
	{
		assert(m_pData->hDC);
		::RealizePalette(m_pData->hDC);
	}

#ifndef _WIN32_WCE

	inline void CDC::CreateHalftonePalette(BOOL ForceBkgnd)
	// Creates and selects halftone palette
	{
		assert(m_pData->hDC);

		CPalette palette;
		palette.CreateHalftonePalette(*this);
		::SelectPalette(m_pData->hDC, palette, ForceBkgnd);
		m_pData->Palette = palette;
		::RealizePalette(m_pData->hDC);
	}

	inline BOOL CDC::GetColorAdjustment(LPCOLORADJUSTMENT pCA) const
	// Retrieves the color adjustment values for the device context.
	{
		assert(m_pData->hDC);
		return ::GetColorAdjustment(m_pData->hDC, pCA);
	}

	inline BOOL CDC::SetColorAdjustment(const COLORADJUSTMENT* pCA) const
	// Sets the color adjustment values for the device context.
	{
		assert(m_pData->hDC);
		return ::SetColorAdjustment(m_pData->hDC, pCA);
	}

	inline BOOL CDC::UpdateColors() const
	// Updates the client area of the specified device context by remapping the current colors in the
	//  client area to the currently realized logical palette.
	{
		assert(m_pData->hDC);
		return ::UpdateColors(m_pData->hDC);
	}

#endif

	// Pen functions
	inline void CDC::CreatePen (int nStyle, int nWidth, COLORREF rgb)
	// Creates the pen and selects it into the device context.
	{
		assert(m_pData->hDC);

		CPen pen;
		pen.CreatePen(nStyle, nWidth, rgb);
		SelectObject(pen);
		m_pData->Pen = pen;
	}

	inline void CDC::CreatePenIndirect (const LOGPEN& LogPen)
	// Creates the pen and selects it into the device context.
	{
		assert(m_pData->hDC);

		CPen pen;
		pen.CreatePenIndirect(LogPen);
		SelectObject(pen);
		m_pData->Pen = pen;
	}

	inline HPEN CDC::GetCurrentPen() const
	// Retrieves the handle to the currently selected pen
	{
		assert(m_pData->hDC);
		return static_cast<HPEN>(::GetCurrentObject(m_pData->hDC, OBJ_PEN));
	}

	inline LOGPEN CDC::GetLogPen() const
	// Retrieves the current pen information as a LOGPEN
	{
		assert(m_pData->hDC);

		HPEN hPen = (HPEN)::GetCurrentObject(m_pData->hDC, OBJ_PEN);
		LOGPEN lPen;
		ZeroMemory(&lPen, sizeof(LOGPEN));
		::GetObject(hPen, sizeof(lPen), &lPen);
		return lPen;
	}

	// Retrieve and Select Stock Objects
	inline HGDIOBJ CDC::GetStockObject(int nIndex) const
	// Retrieves a stock brush, pen, or font.
	// nIndex values: BLACK_BRUSH, DKGRAY_BRUSH, DC_BRUSH, HOLLOW_BRUSH, LTGRAY_BRUSH, NULL_BRUSH,
	//                WHITE_BRUSH, BLACK_PEN, DC_PEN, ANSI_FIXED_FONT, ANSI_VAR_FONT, DEVICE_DEFAULT_FONT,
	//                DEFAULT_GUI_FONT, OEM_FIXED_FONT, SYSTEM_FONT, or SYSTEM_FIXED_FONT.
	{
		return ::GetStockObject(nIndex);
	}

	inline HGDIOBJ CDC::SelectStockObject(int nIndex) const
	// Selects a stock brush, pen, or font into the device context.
	// nIndex values: BLACK_BRUSH, DKGRAY_BRUSH, DC_BRUSH, HOLLOW_BRUSH, LTGRAY_BRUSH, NULL_BRUSH,
	//                WHITE_BRUSH, BLACK_PEN, DC_PEN, ANSI_FIXED_FONT, ANSI_VAR_FONT, DEVICE_DEFAULT_FONT,
	//                DEFAULT_GUI_FONT, OEM_FIXED_FONT, SYSTEM_FONT, or SYSTEM_FIXED_FONT.
	{
		assert(m_pData->hDC);
		HGDIOBJ hStockObject = ::GetStockObject(nIndex);
		return SelectObject(hStockObject);
	}

	// Region functions
	inline int CDC::CreateRectRgn(int left, int top, int right, int bottom)
	// Creates a rectangular region from the rectangle co-ordinates.
	// The return value specifies the region's complexity: NULLREGION;SIMPLEREGION;COMPLEXREGION;ERROR.
	{
		assert(m_pData->hDC);

		CRgn rgn;
		rgn.CreateRectRgn(left, top, right, bottom);
		int Complexity = SelectClipRgn(rgn);
		m_pData->Rgn = rgn;
		return Complexity;
	}

	inline int CDC::CreateRectRgnIndirect(const RECT& rc)
	// Creates a rectangular region from the rectangle co-ordinates.
	// The return value specifies the region's complexity: NULLREGION;SIMPLEREGION;COMPLEXREGION;ERROR.
	{
		assert(m_pData->hDC);

		CRgn rgn;
		rgn.CreateRectRgnIndirect(rc);
		int Complexity = SelectClipRgn(rgn);
		m_pData->Rgn = rgn;
		return Complexity;
	}

	inline int CDC::CreateFromData(const XFORM* Xform, DWORD nCount, const RGNDATA *pRgnData)
	// Creates a region from the specified region data and transformation data.
	// The return value specifies the region's complexity: NULLREGION;SIMPLEREGION;COMPLEXREGION;ERROR.
	// Notes: GetRegionData can be used to get a region's data
	//        If the XFROM pointer is NULL, the identity transformation is used.
	{
		assert(m_pData->hDC);

		CRgn rgn;
		rgn.CreateFromData(Xform, nCount, pRgnData);
		int Complexity = SelectClipRgn(rgn);
		m_pData->Rgn = rgn;
		return Complexity;
	}


#ifndef _WIN32_WCE
	inline int CDC::CreateEllipticRgn(int left, int top, int right, int bottom)
	// Creates the elliptical region from the bounding rectangle co-ordinates
	// and selects it into the device context.
	// The return value specifies the region's complexity: NULLREGION;SIMPLEREGION;COMPLEXREGION;ERROR.
	{
		assert(m_pData->hDC);

		CRgn rgn;
		rgn.CreateEllipticRgn(left, top, right, bottom);
		int Complexity = SelectClipRgn(rgn);
		m_pData->Rgn = rgn;
		return Complexity;
	}

	inline int CDC::CreateEllipticRgnIndirect(const RECT& rc)
	// Creates the elliptical region from the bounding rectangle co-ordinates
	// and selects it into the device context.
	// The return value specifies the region's complexity: NULLREGION;SIMPLEREGION;COMPLEXREGION;ERROR.
	{
		assert(m_pData->hDC);

		CRgn rgn;
		rgn.CreateEllipticRgnIndirect(rc);
		int Complexity = SelectClipRgn(rgn);
		m_pData->Rgn = rgn;
		return Complexity;
	}

	inline int CDC::CreatePolygonRgn(LPPOINT ppt, int cPoints, int fnPolyFillMode)
	// Creates the polygon region from the array of points and selects it into
	// the device context. The polygon is presumed closed.
	// The return value specifies the region's complexity: NULLREGION;SIMPLEREGION;COMPLEXREGION;ERROR.
	{
		assert(m_pData->hDC);

		CRgn rgn;
		rgn.CreatePolygonRgn(ppt, cPoints, fnPolyFillMode);
		int Complexity = SelectClipRgn(rgn);
		m_pData->Rgn = rgn;
		return Complexity;
	}

	inline int CDC::CreatePolyPolygonRgn(LPPOINT ppt, LPINT pPolyCounts, int nCount, int fnPolyFillMode)
	// Creates the polygon region from a series of polygons.The polygons can overlap.
	// The return value specifies the region's complexity: NULLREGION;SIMPLEREGION;COMPLEXREGION;ERROR.
	{
		assert(m_pData->hDC);

		CRgn rgn;
		rgn.CreatePolyPolygonRgn(ppt, pPolyCounts, nCount, fnPolyFillMode);
		int Complexity = SelectClipRgn(rgn);
		m_pData->Rgn = rgn;
		return Complexity;
	}
#endif


	// Wrappers for WinAPI functions

	inline int CDC::GetDeviceCaps (int nIndex) const
	// Retrieves device-specific information for the specified device.
	{
		assert(m_pData->hDC);
		return ::GetDeviceCaps(m_pData->hDC, nIndex);
	}

	// Brush Functions
#if (_WIN32_WINNT >= 0x0500)
	inline COLORREF CDC::GetDCBrushColor() const
	{
		assert(m_pData->hDC);
		return ::GetDCBrushColor(m_pData->hDC);
	}

	inline COLORREF CDC::SetDCBrushColor(COLORREF crColor) const
	{
		assert(m_pData->hDC);
		return ::SetDCBrushColor(m_pData->hDC, crColor);
	}
#endif

	// Font Functions
#ifndef _WIN32_WCE
	inline DWORD CDC::GetFontData(DWORD dwTable, DWORD dwOffset, LPVOID pvBuffer, DWORD cbData) const
	// Retrieves font metric data for a TrueType font.
	{
		assert(m_pData->hDC);
		return ::GetFontData(m_pData->hDC, dwTable, dwOffset, pvBuffer, cbData);
	}

	inline DWORD CDC::GetFontLanguageInfo() const
	// Returns information about the currently selected font for the display context.
	{
		assert(m_pData->hDC);
		return ::GetFontLanguageInfo(m_pData->hDC);
	}

	inline DWORD CDC::GetGlyphOutline(UINT uChar, UINT uFormat, LPGLYPHMETRICS pgm, DWORD cbBuffer,
			                  LPVOID pvBuffer, const MAT2* lpmat2) const
	// Retrieves the outline or bitmap for a character in the TrueType font that is selected into the device context.
	{
		assert(m_pData->hDC);
		return ::GetGlyphOutline(m_pData->hDC, uChar, uFormat, pgm, cbBuffer, pvBuffer, lpmat2);
	}

	inline DWORD CDC::GetKerningPairs(DWORD nNumPairs, LPKERNINGPAIR pkrnpair) const
	// retrieves the character-kerning pairs for the currently selected font for the device context.
	{
		assert(m_pData->hDC);
		return ::GetKerningPairs(m_pData->hDC, nNumPairs, pkrnpair);
	}

	inline DWORD CDC::SetMapperFlags(DWORD dwFlag) const
	// Alters the algorithm the font mapper uses when it maps logical fonts to physical fonts.
	{
		assert(m_pData->hDC);
		return ::SetMapperFlags(m_pData->hDC, dwFlag);
	}

	// Pen Functions
	inline BOOL CDC::GetMiterLimit(PFLOAT peLimit) const
	// Retrieves the miter limit for the device context. The miter limit is used when
	// drawing geometric lines that have miter joins.
	{
		assert(m_pData->hDC);
		return ::GetMiterLimit(m_pData->hDC, peLimit);
	}

	inline BOOL CDC::SetMiterLimit(FLOAT eNewLimit, PFLOAT peOldLimit) const
	// Sets the limit for the length of miter joins for the device context.
	{
		assert(m_pData->hDC);
		return ::SetMiterLimit(m_pData->hDC, eNewLimit, peOldLimit);
	}
#endif

	// Clipping functions
	inline int CDC::ExcludeClipRect(int Left, int Top, int Right, int BottomRect) const
	// Creates a new clipping region that consists of the existing clipping region minus the specified rectangle.
	{
		assert(m_pData->hDC);
		return ::ExcludeClipRect(m_pData->hDC, Left, Top, Right, BottomRect);
	}

	inline int CDC::ExcludeClipRect(const RECT& rc) const
	// Creates a new clipping region that consists of the existing clipping region minus the specified rectangle.
	{
		assert(m_pData->hDC);
		return ::ExcludeClipRect(m_pData->hDC, rc.left, rc.top, rc.right, rc.bottom);
	}

	inline int CDC::GetClipBox (RECT& rc) const
	// Retrieves the dimensions of the tightest bounding rectangle that can be drawn around the current visible area on the device.
	{
		assert(m_pData->hDC);
		return ::GetClipBox(m_pData->hDC, &rc);
	}

	inline int CDC::IntersectClipRect(int Left, int Top, int Right, int Bottom) const
	// Creates a new clipping region from the intersection of the current clipping region and the specified rectangle.
	{
		assert(m_pData->hDC);
		return ::IntersectClipRect(m_pData->hDC, Left, Top, Right, Bottom);
	}

	inline int CDC::IntersectClipRect(const RECT& rc) const
	// Creates a new clipping region from the intersection of the current clipping region and the specified rectangle.
	{
		assert(m_pData->hDC);
		return ::IntersectClipRect(m_pData->hDC, rc.left, rc.top, rc.right, rc.bottom);
	}

	inline BOOL CDC::RectVisible(const RECT& rc) const
	// Determines whether any part of the specified rectangle lies within the
	// clipping region of a device context.
	{
		assert(m_pData->hDC);
		return ::RectVisible (m_pData->hDC, &rc);
	}

	inline int CDC::SelectClipRgn(HRGN hRgn) const
	// Selects a region as the current clipping region for the specified device context.
	// Note: Only a copy of the selected region is used.
	//       To remove a device-context's clipping region, specify a NULL region handle.
	{
		assert(m_pData->hDC);
		return ::SelectClipRgn(m_pData->hDC, hRgn);
	}

#ifndef _WIN32_WCE
	inline BOOL CDC::BeginPath() const
	// Opens a path bracket in the device context.
	{
		assert(m_pData->hDC);
		return ::BeginPath(m_pData->hDC);
	}

	inline BOOL CDC::EndPath() const
	// Closes a path bracket and selects the path defined by the bracket into the device context.
	{
		assert(m_pData->hDC);
		return ::EndPath(m_pData->hDC);
	}

	inline int CDC::ExtSelectClipRgn(HRGN hRgn, int fnMode) const
	// Combines the specified region with the current clipping region using the specified mode.
	{
		assert(m_pData->hDC);
		return ::ExtSelectClipRgn(m_pData->hDC, hRgn, fnMode);
	}

	inline BOOL CDC::FlattenPath() const
	// Transforms any curves in the path that is selected into the device context, turning each
	//  curve into a sequence of lines.
	{
		assert(m_pData->hDC);
		return ::FlattenPath(m_pData->hDC);
	}

	inline int CDC::GetPath(POINT* pPoints, BYTE* pTypes, int nCount) const
	// Retrieves the coordinates defining the endpoints of lines and the control points of curves found in the path
	//  that is selected into the device context.
	// pPoints: An array of POINT structures that receives the line endpoints and curve control points, in logical coordinates.
	// pTypes: Pointer to an array of bytes that receives the vertex types (PT_MOVETO, PT_LINETO or PT_BEZIERTO).
	// nCount: The total number of POINT structures that can be stored in the array pointed to by pPoints.
	{
		assert(m_pData->hDC);
		return ::GetPath(m_pData->hDC, pPoints, pTypes, nCount);
	}

	inline BOOL CDC::PtVisible(int X, int Y) const
	// Determines whether the specified point is within the clipping region of a device context.
	{
		assert(m_pData->hDC);
		return ::PtVisible (m_pData->hDC, X, Y);
	}

	inline int CDC::OffsetClipRgn(int nXOffset, int nYOffset) const
	// Moves the clipping region of a device context by the specified offsets.
	{
		assert(m_pData->hDC);
		return ::OffsetClipRgn (m_pData->hDC, nXOffset, nYOffset);
	}

	inline BOOL CDC::SelectClipPath(int nMode) const
	// Selects the current path as a clipping region for the device context, combining
	// the new region with any existing clipping region using the specified mode.
	{
		assert(m_pData->hDC);
		return ::SelectClipPath(m_pData->hDC, nMode);
	}

	inline BOOL CDC::WidenPath() const
	// Redefines the current path as the area that would be painted if the path were
	// stroked using the pen currently selected into the device context.
	{
		assert(m_pData->hDC);
		return ::WidenPath(m_pData->hDC);
	}
#endif

	// Point and Line Drawing Functions
	inline CPoint CDC::GetCurrentPosition() const
	// Returns the current "MoveToEx" position.
	{
		assert(m_pData->hDC);
		CPoint pt;
		::MoveToEx(m_pData->hDC, 0, 0, &pt);
		::MoveToEx(m_pData->hDC, pt.x, pt.y, NULL);
		return pt;
	}

	inline COLORREF CDC::GetPixel(int x, int y) const
	// Retrieves the red, green, blue (RGB) color value of the pixel at the specified coordinates.
	{
		assert(m_pData->hDC);
		return ::GetPixel(m_pData->hDC, x, y);
	}

	inline COLORREF CDC::GetPixel(POINT pt) const
	// Retrieves the red, green, blue (RGB) color value of the pixel at the specified coordinates.
	{
		assert(m_pData->hDC);
		return ::GetPixel(m_pData->hDC, pt.x, pt.y);
	}

	inline CPoint CDC::MoveTo(int x, int y) const
	// Updates the current position to the specified point.
	{
		assert(m_pData->hDC);
		return ::MoveToEx(m_pData->hDC, x, y, NULL);
	}

	inline CPoint CDC::MoveTo(POINT pt) const
	// Updates the current position to the specified point
	{
		assert(m_pData->hDC);
		return ::MoveToEx(m_pData->hDC, pt.x, pt.y, NULL);
	}

	inline BOOL CDC::LineTo(int x, int y) const
	// Draws a line from the current position up to, but not including, the specified point.
	{
		assert(m_pData->hDC);
		return ::LineTo(m_pData->hDC, x, y);
	}

	inline BOOL CDC::LineTo(POINT pt) const
	// Draws a line from the current position up to, but not including, the specified point.
	{
		assert(m_pData->hDC);
		return ::LineTo(m_pData->hDC, pt.x, pt.y);
	}

	inline int CDC::SetROP2(int iDrawMode) const
	// Sets the current foreground mix mode. GDI uses the foreground mix mode to
	// combine pens and interiors of filled objects with the colors already on the screen.
	{
		assert(m_pData->hDC);
		return ::SetROP2(m_pData->hDC, iDrawMode);
	}

	inline COLORREF CDC::SetPixel (int x, int y, COLORREF crColor) const
	// Sets the pixel at the specified coordinates to the specified color.
	{
		assert(m_pData->hDC);
		return ::SetPixel(m_pData->hDC, x, y, crColor);
	}

#ifndef _WIN32_WCE
	inline BOOL CDC::Arc(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4) const
	// Draws an elliptical arc.
	{
		assert(m_pData->hDC);
		return ::Arc(m_pData->hDC, x1, y1, x2, y2, x3, y3, x4, y4);
	}

	inline BOOL CDC::Arc(RECT& rc, POINT ptStart, POINT ptEnd) const
	// Draws an elliptical arc.
	{
		assert(m_pData->hDC);
		return ::Arc(m_pData->hDC, rc.left, rc.top, rc.right, rc.bottom,
			ptStart.x, ptStart.y, ptEnd.x, ptEnd.y);
	}

	inline BOOL CDC::ArcTo(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4) const
	// Draws an elliptical arc.
	{
		assert(m_pData->hDC);
		return ::ArcTo(m_pData->hDC, x1, y1, x2, y2, x3, y3, x4, y4);
	}

	inline BOOL CDC::ArcTo(RECT& rc, POINT ptStart, POINT ptEnd) const
	// Draws an elliptical arc.
	{
		assert(m_pData->hDC);
		return ::ArcTo (m_pData->hDC, rc.left, rc.top, rc.right, rc.bottom,
			ptStart.x, ptStart.y, ptEnd.x, ptEnd.y);
	}

	inline BOOL CDC::AngleArc(int x, int y, int nRadius, float fStartAngle, float fSweepAngle) const
	// Draws a line segment and an arc.
	{
		assert(m_pData->hDC);
		return ::AngleArc(m_pData->hDC, x, y, nRadius, fStartAngle, fSweepAngle);
	}

	inline BOOL CDC::CloseFigure() const
	// Closes the figure by drawing a line from the current position to the first point of the figure.
	{
		assert(m_pData->hDC);
		return ::CloseFigure(m_pData->hDC);
	}

	inline int CDC::GetROP2() const
	// Retrieves the foreground mix mode of the specified device context. The mix mode specifies how
	// the pen or interior color and the color already on the screen are combined to yield a new color.
	{
		assert(m_pData->hDC);
		return ::GetROP2(m_pData->hDC);
	}

	inline int CDC::GetArcDirection() const
	// Retrieves the current arc direction ( AD_COUNTERCLOCKWISE or AD_CLOCKWISE ).
	{
		assert(m_pData->hDC);
		return ::GetArcDirection(m_pData->hDC);
	}

	inline int CDC::SetArcDirection(int nArcDirection) const
	// Sets the current arc direction ( AD_COUNTERCLOCKWISE or AD_CLOCKWISE ).
	{
		assert(m_pData->hDC);
		return ::SetArcDirection(m_pData->hDC, nArcDirection);
	}

	inline BOOL CDC::PolyDraw(const POINT* lpPoints, const BYTE* lpTypes, int nCount) const
	// Draws a set of line segments and Bezier curves.
	{
		assert(m_pData->hDC);
		return ::PolyDraw(m_pData->hDC, lpPoints, lpTypes, nCount);
	}

	inline BOOL CDC::Polyline(LPPOINT lpPoints, int nCount) const
	// Draws a series of line segments by connecting the points in the specified array.
	{
		assert(m_pData->hDC);
		return ::Polyline(m_pData->hDC, lpPoints, nCount);
	}

	inline BOOL CDC::PolyPolyline(const POINT* lpPoints, const DWORD* lpPolyPoints, int nCount) const
	// Draws multiple series of connected line segments.
	{
		assert(m_pData->hDC);
		return ::PolyPolyline(m_pData->hDC, lpPoints, lpPolyPoints, nCount);
	}

	inline BOOL CDC::PolylineTo(const POINT* lpPoints, int nCount) const
	// Draws one or more straight lines.
	{
		assert(m_pData->hDC);
		return ::PolylineTo(m_pData->hDC, lpPoints, nCount);
	}
	inline BOOL CDC::PolyBezier(const POINT* lpPoints, int nCount) const
	// Draws one or more Bezier curves.
	{
		assert(m_pData->hDC);
		return ::PolyBezier(m_pData->hDC, lpPoints, nCount);
	}

	inline BOOL CDC::PolyBezierTo(const POINT* lpPoints, int nCount) const
	// Draws one or more Bezier curves.
	{
		assert(m_pData->hDC);
		return ::PolyBezierTo(m_pData->hDC, lpPoints, nCount );
	}

	inline COLORREF CDC::SetPixel(POINT pt, COLORREF crColor) const
	// Sets the pixel at the specified coordinates to the specified color.
	{
		assert(m_pData->hDC);
		return ::SetPixel(m_pData->hDC, pt.x, pt.y, crColor);
	}

	inline BOOL CDC::SetPixelV(int x, int y, COLORREF crColor) const
	// Sets the pixel at the specified coordinates to the closest approximation of the specified color.
	{
		assert(m_pData->hDC);
		return ::SetPixelV(m_pData->hDC, x, y, crColor);
	}

	inline BOOL CDC::SetPixelV(POINT pt, COLORREF crColor) const
	// Sets the pixel at the specified coordinates to the closest approximation of the specified color.
	{
		assert(m_pData->hDC);
		return ::SetPixelV(m_pData->hDC, pt.x, pt.y, crColor);
	}
#endif

	// Shape Drawing Functions
	inline void CDC::DrawFocusRect(const RECT& rc) const
	// Draws a rectangle in the style used to indicate that the rectangle has the focus.
	{
		assert(m_pData->hDC);
		::DrawFocusRect(m_pData->hDC, &rc);
	}

	inline BOOL CDC::Ellipse(int x1, int y1, int x2, int y2) const
	// Draws an ellipse. The center of the ellipse is the center of the specified bounding rectangle.
	{
		assert(m_pData->hDC);
		return ::Ellipse(m_pData->hDC, x1, y1, x2, y2);
	}

	inline BOOL CDC::Ellipse(const RECT& rc) const
	// Draws an ellipse. The center of the ellipse is the center of the specified bounding rectangle.
	{
		assert(m_pData->hDC);
		return ::Ellipse(m_pData->hDC, rc.left, rc.top, rc.right, rc.bottom);
	}

	inline BOOL CDC::Polygon(LPPOINT lpPoints, int nCount) const
	// Draws a polygon consisting of two or more vertices connected by straight lines.
	{
		assert(m_pData->hDC);
		return ::Polygon(m_pData->hDC, lpPoints, nCount);
	}

	inline BOOL CDC::Rectangle(int x1, int y1, int x2, int y2) const
	// Draws a rectangle. The rectangle is outlined by using the current pen and filled by using the current brush.
	{
		assert(m_pData->hDC);
		return ::Rectangle(m_pData->hDC, x1, y1, x2, y2);
	}

	inline BOOL CDC::Rectangle(const RECT& rc) const
	// Draws a rectangle. The rectangle is outlined by using the current pen and filled by using the current brush.
	{
		assert(m_pData->hDC);
		return ::Rectangle(m_pData->hDC, rc.left, rc.top, rc.right, rc.bottom);
	}

	inline BOOL CDC::RoundRect(int x1, int y1, int x2, int y2, int nWidth, int nHeight) const
	// Draws a rectangle with rounded corners.
	{
		assert(m_pData->hDC);
		return ::RoundRect(m_pData->hDC, x1, y1, x2, y2, nWidth, nHeight);
	}
	inline BOOL CDC::RoundRect(const RECT& rc, int nWidth, int nHeight) const
	// Draws a rectangle with rounded corners.
	{
		assert(m_pData->hDC);
		return ::RoundRect(m_pData->hDC, rc.left, rc.top, rc.right, rc.bottom, nWidth, nHeight );
	}

#ifndef _WIN32_WCE
	inline BOOL CDC::Chord(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4) const
	// Draws a chord (a region bounded by the intersection of an ellipse and a line segment, called a secant).
	{
		assert(m_pData->hDC);
		return ::Chord(m_pData->hDC, x1, y1, x2, y2, x3, y3, x4, y4);
	}

	inline BOOL CDC::Chord(const RECT& rc, POINT ptStart, POINT ptEnd) const
	// Draws a chord (a region bounded by the intersection of an ellipse and a line segment, called a secant).
	{
		assert(m_pData->hDC);
		return ::Chord(m_pData->hDC, rc.left, rc.top, rc.right, rc.bottom,
			ptStart.x, ptStart.y, ptEnd.x, ptEnd.y);
	}

	inline BOOL CDC::Pie(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4) const
	// Draws a pie-shaped wedge bounded by the intersection of an ellipse and two radials.
	{
		assert(m_pData->hDC);
		return ::Pie(m_pData->hDC, x1, y1, x2, y2, x3, y3, x4, y4);
	}

	inline BOOL CDC::Pie(const RECT& rc, POINT ptStart, POINT ptEnd) const
	// Draws a pie-shaped wedge bounded by the intersection of an ellipse and two radials.
	{
		assert(m_pData->hDC);
		return ::Pie(m_pData->hDC, rc.left, rc.top, rc.right, rc.bottom,
			ptStart.x, ptStart.y, ptEnd.x, ptEnd.y);
	}

	inline BOOL CDC::PolyPolygon(LPPOINT lpPoints, LPINT lpPolyCounts, int nCount) const
	// Draws a series of closed polygons.
	{
		assert(m_pData->hDC);
		return ::PolyPolygon(m_pData->hDC, lpPoints, lpPolyCounts, nCount);
	}
#endif

	// Fill and 3D Drawing functions
	inline BOOL CDC::FillRect(const RECT& rc, HBRUSH hBrush) const
	// Fills a rectangle by using the specified brush.
	{
		assert(m_pData->hDC);
		return static_cast<BOOL>(::FillRect(m_pData->hDC, &rc, hBrush));
	}

	inline BOOL CDC::InvertRect(const RECT& rc) const
	// Inverts a rectangle in a window by performing a logical NOT operation on the color values for each pixel in the rectangle's interior.
	{
		assert(m_pData->hDC);
		return ::InvertRect( m_pData->hDC, &rc);
	}

	inline BOOL CDC::DrawIconEx(int xLeft, int yTop, HICON hIcon, int cxWidth, int cyWidth, UINT istepIfAniCur, HBRUSH hFlickerFreeDraw, UINT diFlags) const
	// draws an icon or cursor, performing the specified raster operations, and stretching or compressing the icon or cursor as specified.
	{
		assert(m_pData->hDC);
		return ::DrawIconEx(m_pData->hDC, xLeft, yTop, hIcon, cxWidth, cyWidth, istepIfAniCur, hFlickerFreeDraw, diFlags);
	}

	inline BOOL CDC::DrawEdge(const RECT& rc, UINT nEdge, UINT nFlags) const
	// Draws one or more edges of rectangle.
	{
		assert(m_pData->hDC);
		return ::DrawEdge(m_pData->hDC, (LPRECT)&rc, nEdge, nFlags);
	}

	inline BOOL CDC::DrawFrameControl(const RECT& rc, UINT nType, UINT nState) const
	// Draws a frame control of the specified type and style.
	{
		assert(m_pData->hDC);
		return ::DrawFrameControl(m_pData->hDC, (LPRECT)&rc, nType, nState);
	}

	inline BOOL CDC::FillRgn(HRGN hRgn, HBRUSH hBrush) const
	// Fills a region by using the specified brush.
	{
		assert(m_pData->hDC);
		return ::FillRgn(m_pData->hDC, hRgn, hBrush);
	}

  #if (WINVER >= 0x0410)
	inline BOOL CDC::GradientFill(PTRIVERTEX pVertex, ULONG nVertex, PVOID pMesh, ULONG nMesh, ULONG ulMode) const
	// Fills rectangle and triangle structures.
	{
		assert(m_pData->hDC);
		return ::GradientFill(m_pData->hDC, pVertex, nVertex, pMesh, nMesh, ulMode);
	}
  #endif

#ifndef _WIN32_WCE
	inline BOOL CDC::DrawIcon(int x, int y, HICON hIcon) const
	// Draws an icon or cursor.
	{
		assert(m_pData->hDC);
		return ::DrawIcon(m_pData->hDC, x, y, hIcon);
	}

	inline BOOL CDC::DrawIcon(POINT pt, HICON hIcon) const
	// Draws an icon or cursor.
	{
		assert(m_pData->hDC);
		return ::DrawIcon(m_pData->hDC, pt.x, pt.y, hIcon);
	}

	inline BOOL CDC::FrameRect(const RECT& rc, HBRUSH hBrush) const
	// Draws a border around the specified rectangle by using the specified brush.
	{
		assert(m_pData->hDC);
		return static_cast<BOOL>(::FrameRect(m_pData->hDC, &rc, hBrush));
	}

	inline BOOL CDC::FrameRgn(HRGN hRgn, HBRUSH hBrush, int nWidth, int nHeight) const
	// Draws a border around the specified region by using the specified brush.
	{
		assert(m_pData->hDC);
		return static_cast<BOOL>(::FrameRgn(m_pData->hDC, hRgn, hBrush, nWidth, nHeight));
	}

	inline int CDC::GetPolyFillMode() const
	// Retrieves the current polygon fill mode.
	{
		assert(m_pData->hDC);
		return ::GetPolyFillMode(m_pData->hDC);
	}

	inline BOOL CDC::PaintRgn(HRGN hRgn) const
	// Paints the specified region by using the brush currently selected into the device context.
	{
		assert(m_pData->hDC);
		return static_cast<BOOL>(::PaintRgn(m_pData->hDC, hRgn));
	}

	inline int CDC::SetPolyFillMode(int iPolyFillMode) const
	// Sets the polygon fill mode for functions that fill polygons.
	{
		assert(m_pData->hDC);
		return ::SetPolyFillMode(m_pData->hDC, iPolyFillMode);
	}
#endif

	// Bitmap Functions
	inline int CDC::StretchDIBits(int XDest, int YDest, int nDestWidth, int nDestHeight, int XSrc, int YSrc, int nSrcWidth,
		           int nSrcHeight, LPCVOID pBits, const LPBITMAPINFO pbmi, UINT iUsage, DWORD dwRop) const
	// Copies the color data for a rectangle of pixels in a DIB to the specified destination rectangle.
	// A CBitmapInfoPtr object can be used for the LPBITMAPINFO parameter.
	{
		assert(m_pData->hDC);
		return ::StretchDIBits(m_pData->hDC, XDest, YDest, nDestWidth, nDestHeight, XSrc, YSrc, nSrcWidth, nSrcHeight, pBits, pbmi, iUsage, dwRop);
	}

	inline BOOL CDC::PatBlt(int x, int y, int nWidth, int nHeight, DWORD dwRop) const
	// Paints the specified rectangle using the brush that is currently selected into the device context.
	{
		assert(m_pData->hDC);
		return ::PatBlt(m_pData->hDC, x, y, nWidth, nHeight, dwRop);
	}

	inline BOOL CDC::BitBlt(int x, int y, int nWidth, int nHeight, HDC hdcSrc, int xSrc, int ySrc, DWORD dwRop) const
	// Performs a bit-block transfer of the color data corresponding to a rectangle of pixels from the specified source device context into a destination device context.
	{
		assert(m_pData->hDC);
		return ::BitBlt(m_pData->hDC, x, y, nWidth, nHeight, hdcSrc, xSrc, ySrc, dwRop);
	}

	inline BOOL CDC::MaskBlt(int nXDest, int nYDest, int nWidth, int nHeight, HDC hdcSrc, int nXSrc, int nYSrc, HBITMAP hbmMask, int xMask, int yMask, DWORD dwRop) const
	// Combines the color data for the source and destination bitmaps using the specified mask and raster operation.
	//  nXDest    x-coord of destination upper-left corner
	//  nYDest    y-coord of destination upper-left corner
	//  nWidth    width of source and destination
	//  nHeight   height of source and destination
	//  pSrc      pointer to source DC
	//  nXSrc     x-coord of upper-left corner of source
	//  nYSrc     y-coord of upper-left corner of source
	//  pMask     pointer to monochrome bit mask
	//  xMask     horizontal offset into mask bitmap
	//  yMask     vertical offset into mask bitmap
	//  dwRop     raster operation code
	{
		assert(m_pData->hDC);
		return ::MaskBlt(m_pData->hDC, nXDest, nYDest, nWidth, nHeight, hdcSrc, nXSrc, nYSrc, hbmMask, xMask, yMask, dwRop);
	}

	inline BOOL CDC::StretchBlt(int x, int y, int nWidth, int nHeight, HDC hdcSrc, int xSrc, int ySrc, int nSrcWidth, int nSrcHeight, DWORD dwRop) const
	// Copies a bitmap from a source rectangle into a destination rectangle, stretching or compressing the bitmap to fit the dimensions of the destination rectangle, if necessary.
	//  x            x-coord of destination upper-left corner
	//  y            y-coord of destination upper-left corner
    //  nWidth       width of destination rectangle
	//  nHeight      height of destination rectangle
	//  pSrcDC       handle to source DC
	//  xSrc         x-coord of source upper-left corner
	//  ySrc         y-coord of source upper-left corner
	//  nSrcWidth    width of source rectangle
	//  nSrcHeight   height of source rectangle
	//  dwRop        raster operation code
	{
		assert(m_pData->hDC);
		return ::StretchBlt(m_pData->hDC, x, y, nWidth, nHeight, hdcSrc, xSrc, ySrc, nSrcWidth, nSrcHeight, dwRop);
	}

#ifndef _WIN32_WCE
	inline int CDC::GetDIBits(HBITMAP hBitmap, UINT uStartScan, UINT cScanLines, LPVOID pBits, LPBITMAPINFO pbi, UINT uUsage) const
	// Retrieves the bits of the specified compatible bitmap and copies them into a buffer as a DIB using the specified format.
	// A CBitmapInfoPtr object can be used for the LPBITMAPINFO parameter.
	{
		assert(m_pData->hDC);
		return ::GetDIBits(m_pData->hDC, hBitmap, uStartScan, cScanLines, pBits, pbi, uUsage);
	}

	inline int CDC::SetDIBits(HBITMAP hBitmap, UINT uStartScan, UINT cScanLines, LPCVOID pBits, LPBITMAPINFO pbi, UINT fuColorUse) const
	// Sets the pixels in a compatible bitmap (DDB) using the color data found in the specified DIB.
	// A CBitmapInfoPtr object can be used for the LPBITMAPINFO parameter.
	{
		assert(m_pData->hDC);
		return ::SetDIBits(m_pData->hDC, hBitmap, uStartScan, cScanLines, pBits, pbi, fuColorUse);
	}

	inline int CDC::GetStretchBltMode() const
	// Retrieves the current stretching mode.
	// Possible modes: BLACKONWHITE, COLORONCOLOR, HALFTONE, STRETCH_ANDSCANS, STRETCH_DELETESCANS, STRETCH_HALFTONE, STRETCH_ORSCANS, WHITEONBLACK
	{
		assert(m_pData->hDC);
		return ::GetStretchBltMode(m_pData->hDC);
	}

	inline int CDC::SetStretchBltMode(int iStretchMode) const
	// Sets the stretching mode.
	// Possible modes: BLACKONWHITE, COLORONCOLOR, HALFTONE, STRETCH_ANDSCANS, STRETCH_DELETESCANS, STRETCH_HALFTONE, STRETCH_ORSCANS, WHITEONBLACK
	{
		assert(m_pData->hDC);
		return ::SetStretchBltMode(m_pData->hDC, iStretchMode);
	}

#if (WINVER >= 0x0410)
	inline BOOL CDC::TransparentBlt(int x, int y, int nWidth, int nHeight, HDC hdcSrc, int xSrc, int ySrc,
		                             int nWidthSrc, int nHeightSrc, UINT crTransparent) const
	// Performs a bit-block transfer of the color data corresponding to a rectangle
	// of pixels from the specified source device context into a destination device context.

	//  x             x-coord of destination upper-left corner
	//  y             y-coord of destination upper-left corner
	//  nWidth        width of destination rectangle
	//  hHeight       height of destination rectangle
	//  pSrcDC        pointer to source DC
	//  xSrc          x-coord of source upper-left corner
	//  ySrc          y-coord of source upper-left corner
	//  nWidthSrc     width of source rectangle
	//  nHeightSrc    height of source rectangle
	//  crTransparent color to make transparent

	{
		assert(m_pData->hDC);
		return ::TransparentBlt(m_pData->hDC, x, y, nWidth, nHeight, hdcSrc, xSrc, ySrc, nWidthSrc, nHeightSrc, crTransparent);
	}
#endif

	inline BOOL CDC::FloodFill(int x, int y, COLORREF crColor) const
	// Fills an area of the display surface with the current brush.
	{
		assert(m_pData->hDC);
		return ::FloodFill(m_pData->hDC, x, y, crColor);
	}

	inline BOOL CDC::ExtFloodFill(int x, int y, COLORREF crColor, UINT nFillType) const
	// Fills an area of the display surface with the current brush.
	// Fill type: FLOODFILLBORDER or FLOODFILLSURFACE
	{
		assert(m_pData->hDC);
		return ::ExtFloodFill(m_pData->hDC, x, y, crColor, nFillType );
	}
#endif

#ifndef _WIN32_WCE
	// co-ordinate functions
	inline BOOL CDC::DPtoLP(LPPOINT lpPoints, int nCount) const
	// Converts device coordinates into logical coordinates.
	{
		assert(m_pData->hDC);
		return ::DPtoLP(m_pData->hDC, lpPoints, nCount);
	}

	inline BOOL CDC::DPtoLP(RECT& rc) const
	// Converts device coordinates into logical coordinates.
	{
		assert(m_pData->hDC);
		return ::DPtoLP(m_pData->hDC, (LPPOINT)&rc, 2);
	}

	inline BOOL CDC::LPtoDP(LPPOINT lpPoints, int nCount) const
	// Converts logical coordinates into device coordinates.
	{
		assert(m_pData->hDC);
		return ::LPtoDP(m_pData->hDC, lpPoints, nCount);
	}

	inline BOOL CDC::LPtoDP(RECT& rc) const
	// Converts logical coordinates into device coordinates.
	{
		assert(m_pData->hDC);
		return ::LPtoDP(m_pData->hDC, (LPPOINT)&rc, 2);
	}

#endif

	// Layout Functions
	inline DWORD CDC::GetLayout() const
	// Returns the layout of a device context (LAYOUT_RTL and LAYOUT_BITMAPORIENTATIONPRESERVED).
	{
#if (WINVER >= 0x0500)
		return ::GetLayout(m_pData->hDC);
#else
		return 0;
#endif
	}

	inline DWORD CDC::SetLayout(DWORD dwLayout) const
	// changes the layout of a device context (DC).
	// dwLayout values:  LAYOUT_RTL or LAYOUT_BITMAPORIENTATIONPRESERVED
	{
#if (WINVER >= 0x0500)
		// Sets the layout of a device context
		return ::SetLayout(m_pData->hDC, dwLayout);
#else
		UNREFERENCED_PARAMETER(dwLayout); // no-op
		return 0;
#endif
	}

	// Mapping Functions
#ifndef _WIN32_WCE
	inline int CDC::GetMapMode()  const
	// Retrieves the current mapping mode.
	// Possible modes: MM_ANISOTROPIC, MM_HIENGLISH, MM_HIMETRIC, MM_ISOTROPIC, MM_LOENGLISH, MM_LOMETRIC, MM_TEXT, and MM_TWIPS.
	{
		assert(m_pData->hDC);
		return ::GetMapMode(m_pData->hDC);
	}

	inline BOOL CDC::GetViewportOrgEx(LPPOINT lpPoint)  const
	// Retrieves the x-coordinates and y-coordinates of the viewport origin for the device context.
	{
		assert(m_pData->hDC);
		return ::GetViewportOrgEx(m_pData->hDC, lpPoint);
	}

	inline int CDC::SetMapMode(int nMapMode) const
	// Sets the mapping mode of the specified device context.
	{
		assert(m_pData->hDC);
		return ::SetMapMode(m_pData->hDC, nMapMode);
	}

	inline BOOL CDC::SetViewportOrgEx(int x, int y, LPPOINT lpPoint /* = NULL */) const
	// Specifies which device point maps to the window origin (0,0).
	{
		assert(m_pData->hDC);
		return ::SetViewportOrgEx(m_pData->hDC, x, y, lpPoint);
	}

	inline BOOL CDC::SetViewportOrgEx(POINT point, LPPOINT lpPointRet /* = NULL */) const
	// Specifies which device point maps to the window origin (0,0).
	{
		assert(m_pData->hDC);
		return SetViewportOrgEx(point.x, point.y, lpPointRet);
	}

	inline BOOL CDC::OffsetViewportOrgEx(int nWidth, int nHeight, LPPOINT lpPoint /* = NULL */) const
	// Modifies the viewport origin for the device context using the specified horizontal and vertical offsets.
	{
		assert(m_pData->hDC);
		return ::OffsetViewportOrgEx(m_pData->hDC, nWidth, nHeight, lpPoint);
	}

	inline BOOL CDC::GetViewportExtEx(LPSIZE lpSize)  const
	// Retrieves the x-extent and y-extent of the current viewport for the device context.
	{
		assert(m_pData->hDC);
		return ::GetViewportExtEx(m_pData->hDC, lpSize);
	}

	inline BOOL CDC::SetViewportExtEx(int x, int y, LPSIZE lpSize ) const
	// Sets the horizontal and vertical extents of the viewport for the device context by using the specified values.
	{
		assert(m_pData->hDC);
		return ::SetViewportExtEx(m_pData->hDC, x, y, lpSize);
	}

	inline BOOL CDC::SetViewportExtEx(SIZE size, LPSIZE lpSizeRet ) const
	// Sets the horizontal and vertical extents of the viewport for the device context by using the specified values.
	{
		assert(m_pData->hDC);
		return SetViewportExtEx(size.cx, size.cy, lpSizeRet);
	}

	inline BOOL CDC::ScaleViewportExtEx(int xNum, int xDenom, int yNum, int yDenom, LPSIZE lpSize ) const
	// Modifies the viewport for the device context using the ratios formed by the specified multiplicands and divisors.
	{
		assert(m_pData->hDC);
		return ::ScaleViewportExtEx(m_pData->hDC, xNum, xDenom, yNum, yDenom, lpSize);
	}

	inline BOOL CDC::GetWindowOrgEx(LPPOINT lpPoint) const
	// Retrieves the x-coordinates and y-coordinates of the window origin for the device context.
	{
		assert(m_pData->hDC);
		return ::GetWindowOrgEx(m_pData->hDC, lpPoint);
	}

	inline BOOL CDC::SetWindowOrgEx(int x, int y, LPPOINT lpPoint ) const
	// Specifies which window point maps to the viewport origin (0,0).
	{
		assert(m_pData->hDC);
		return ::SetWindowOrgEx(m_pData->hDC, x, y, lpPoint);
	}

	inline BOOL CDC::SetWindowOrgEx(POINT point, LPPOINT lpPointRet ) const
	// Specifies which window point maps to the viewport origin (0,0).
	{
		assert(m_pData->hDC);
		return SetWindowOrgEx(point.x, point.y, lpPointRet);
	}

	inline BOOL CDC::OffsetWindowOrgEx(int nWidth, int nHeight, LPPOINT lpPoint ) const
	// Modifies the window origin for the device context using the specified horizontal and vertical offsets.
	{
		assert(m_pData->hDC);
		return ::OffsetWindowOrgEx(m_pData->hDC, nWidth, nHeight, lpPoint);
	}

	inline BOOL CDC::GetWindowExtEx(LPSIZE lpSize)  const
	// Retrieves the x-extent and y-extent of the window for the device context.
	{
		assert(m_pData->hDC);
		return ::GetWindowExtEx(m_pData->hDC, lpSize);
	}

	inline BOOL CDC::SetWindowExtEx(int x, int y, LPSIZE lpSize ) const
	// Sets the horizontal and vertical extents of the window for the device context by using the specified values.
	{
		assert(m_pData->hDC);
		return ::SetWindowExtEx(m_pData->hDC, x, y, lpSize);
	}

	inline BOOL CDC::SetWindowExtEx(SIZE size, LPSIZE lpSizeRet) const
	// Sets the horizontal and vertical extents of the window for the device context by using the specified values.
	{
		assert(m_pData->hDC);
		return SetWindowExtEx(size.cx, size.cy, lpSizeRet);
	}

	inline BOOL CDC::ScaleWindowExtEx(int xNum, int xDenom, int yNum, int yDenom, LPSIZE lpSize) const
	// Modifies the window for the device context using the ratios formed by the specified multiplicands and divisors.
	{
		assert(m_pData->hDC);
		return ::ScaleWindowExtEx(m_pData->hDC, xNum, xDenom, yNum, yDenom, lpSize);
	}

	// MetaFile Functions
	inline BOOL CDC::PlayMetaFile(HMETAFILE hMF) const
	// Displays the picture stored in the specified metafile. 
	{
		assert(m_pData->hDC);
		return ::PlayMetaFile(m_pData->hDC, hMF);
	}

	inline BOOL CDC::PlayMetaFile(HENHMETAFILE hEnhMetaFile, const RECT& rcBounds) const
	// Displays the picture stored in the specified enhanced-format metafile. 
	{
		assert(m_pData->hDC);
		return ::PlayEnhMetaFile(m_pData->hDC, hEnhMetaFile, &rcBounds);
	}

#endif // _WIN32_WCE

	// Printer Functions
	inline int CDC::StartDoc(LPDOCINFO lpDocInfo) const
	// Starts a print job.
	{
		assert(m_pData->hDC);
		return ::StartDoc(m_pData->hDC, lpDocInfo);
	}

	inline int CDC::EndDoc() const
	// Ends a print job.
	{
		assert(m_pData->hDC);
		return ::EndDoc(m_pData->hDC);
	}

	inline int CDC::StartPage() const
	// Prepares the printer driver to accept data.
	{
		assert(m_pData->hDC);
		return ::StartPage(m_pData->hDC);
	}

	inline int CDC::EndPage() const
	// Notifies the device that the application has finished writing to a page.
	{
		assert(m_pData->hDC);
		return ::EndPage(m_pData->hDC);
	}

	inline int CDC::AbortDoc() const
	// Stops the current print job and erases everything drawn since the last call to the StartDoc function.
	{
		assert(m_pData->hDC);
		return ::AbortDoc(m_pData->hDC);
	}

	inline int CDC::SetAbortProc(BOOL (CALLBACK* lpfn)(HDC, int)) const
	// Sets the application-defined abort function that allows a print job to be cancelled during spooling.
	{
		assert(m_pData->hDC);
		return ::SetAbortProc(m_pData->hDC, lpfn);
	}

	// Text Functions
	inline BOOL CDC::ExtTextOut(int x, int y, UINT nOptions, const RECT& rc, LPCTSTR lpszString, int nCount /*= -1*/, LPINT lpDxWidths /*=NULL*/) const
	// Draws text using the currently selected font, background color, and text color
	{
		assert(m_pData->hDC);

		if (nCount == -1)
			nCount = lstrlen (lpszString);

		return ::ExtTextOut(m_pData->hDC, x, y, nOptions, &rc, lpszString, nCount, lpDxWidths );
	}

	inline int CDC::DrawText(LPCTSTR lpszString, int nCount, RECT& rc, UINT nFormat) const
	// Draws formatted text in the specified rectangle
	{
		assert(m_pData->hDC);
		return ::DrawText(m_pData->hDC, lpszString, nCount, &rc, nFormat );
	}

	inline UINT CDC::GetTextAlign() const
	// Retrieves the text-alignment setting
	// Values: TA_BASELINE, TA_BOTTOM, TA_TOP, TA_CENTER, TA_LEFT, TA_RIGHT, TA_RTLREADING, TA_NOUPDATECP, TA_UPDATECP
	{
		assert(m_pData->hDC);
		return ::GetTextAlign(m_pData->hDC);
	}

	inline UINT CDC::SetTextAlign(UINT nFlags) const
	// Sets the text-alignment setting
	// Values: TA_BASELINE, TA_BOTTOM, TA_TOP, TA_CENTER, TA_LEFT, TA_RIGHT, TA_RTLREADING, TA_NOUPDATECP, TA_UPDATECP
	{
		assert(m_pData->hDC);
		return ::SetTextAlign(m_pData->hDC, nFlags);
	}

	inline int CDC::GetTextFace(int nCount, LPTSTR lpszFacename) const
	// Retrieves the typeface name of the font that is selected into the device context
	{
		assert(m_pData->hDC);
		return ::GetTextFace(m_pData->hDC, nCount, lpszFacename);
	}

	inline BOOL CDC::GetTextMetrics(TEXTMETRIC& Metrics) const
	// Fills the specified buffer with the metrics for the currently selected font
	{
		assert(m_pData->hDC);
		return ::GetTextMetrics(m_pData->hDC, &Metrics);
	}

	inline COLORREF CDC::GetBkColor() const
	// Returns the current background color
	{
		assert(m_pData->hDC);
		return ::GetBkColor(m_pData->hDC);
	}

	inline COLORREF CDC::SetBkColor(COLORREF crColor) const
	// Sets the current background color to the specified color value
	{
		assert(m_pData->hDC);
		return ::SetBkColor(m_pData->hDC, crColor);
	}

	inline COLORREF CDC::GetTextColor() const
	// Retrieves the current text color
	{
		assert(m_pData->hDC);
		return ::GetTextColor(m_pData->hDC);
	}

	inline COLORREF CDC::SetTextColor(COLORREF crColor) const
	// Sets the current text color
	{
		assert(m_pData->hDC);
		return ::SetTextColor(m_pData->hDC, crColor);
	}

	inline int CDC::GetBkMode() const
	// returns the current background mix mode (OPAQUE or TRANSPARENT)
	{
		assert(m_pData->hDC);
		return ::GetBkMode(m_pData->hDC);
	}

	inline int CDC::SetBkMode(int iBkMode) const
	// Sets the current background mix mode (OPAQUE or TRANSPARENT)
	{
		assert(m_pData->hDC);
		return ::SetBkMode(m_pData->hDC, iBkMode);
	}

#ifndef _WIN32_WCE
	inline int CDC::DrawTextEx(LPTSTR lpszString, int nCount, RECT& rc, UINT nFormat, LPDRAWTEXTPARAMS lpDTParams) const
	// Draws formatted text in the specified rectangle with more formatting options
	{
		assert(m_pData->hDC);
		return ::DrawTextEx(m_pData->hDC, lpszString, nCount, &rc, nFormat, lpDTParams);
	}

	inline BOOL CDC::GetCharABCWidths(UINT uFirstChar, UINT uLastChar, LPABC pABC) const
	// Retrieves the widths, in logical units, of consecutive characters in a specified range from the
	// current TrueType font. This function succeeds only with TrueType fonts.
	{
		assert(m_pData->hDC);
		return::GetCharABCWidths(m_pData->hDC, uFirstChar, uLastChar, pABC);
	}

	inline DWORD CDC::GetCharacterPlacement(LPCTSTR pString, int nCount, int nMaxExtent, LPGCP_RESULTS pResults, DWORD dwFlags) const
	// Retrieves information about a character string, such as character widths, caret positioning, ordering within the string, and glyph rendering.
	{
		assert(m_pData->hDC);
		return ::GetCharacterPlacement(m_pData->hDC, pString, nCount, nMaxExtent, pResults, dwFlags);
	}

	inline BOOL CDC::GetCharWidth(UINT iFirstChar, UINT iLastChar, float* pBuffer) const
	// Retrieves the fractional widths of consecutive characters in a specified range from the current font.
	{
		assert(m_pData->hDC);
		return ::GetCharWidthFloat(m_pData->hDC, iFirstChar, iLastChar, pBuffer);
	}

	inline CSize CDC::GetTextExtentPoint32(LPCTSTR lpszString, int nCount) const
	// Computes the width and height of the specified string of text
	{
		assert(m_pData->hDC);
		CSize sz;
		::GetTextExtentPoint32(m_pData->hDC, lpszString, nCount, &sz);
		return sz;
	}

	inline CSize CDC::GetTextExtentPoint32(CString& str) const
	// Computes the width and height of the specified string of text
	{
		CSize sz;
		return GetTextExtentPoint32(str.c_str(), str.GetLength());
	}

	inline CSize CDC::GetTabbedTextExtent(LPCTSTR lpszString, int nCount, int nTabPositions, LPINT lpnTabStopPositions) const
	// Computes the width and height of a character string
	{
		assert(m_pData->hDC);
		DWORD dwSize = ::GetTabbedTextExtent(m_pData->hDC, lpszString, nCount, nTabPositions, lpnTabStopPositions );
		CSize sz(dwSize);
		return sz;
	}

	inline BOOL CDC::GrayString(HBRUSH hBrush, GRAYSTRINGPROC lpOutputFunc, LPARAM lpData, int nCount, int x, int y, int nWidth, int nHeight) const
	// Draws gray text at the specified location
	{
		assert(m_pData->hDC);
		return ::GrayString(m_pData->hDC, hBrush, lpOutputFunc, lpData, nCount, x, y, nWidth, nHeight);
	}

	inline int CDC::SetTextJustification(int nBreakExtra, int nBreakCount) const
	// Specifies the amount of space the system should add to the break characters in a string of text
	{
		assert(m_pData->hDC);
		return ::SetTextJustification(m_pData->hDC, nBreakExtra, nBreakCount);
	}

	inline int CDC::GetTextCharacterExtra() const
	// Retrieves the current inter-character spacing for the device context
	{
		assert(m_pData->hDC);
		return ::GetTextCharacterExtra(m_pData->hDC);
	}

	inline int CDC::SetTextCharacterExtra(int nCharExtra) const
	// Sets the inter-character spacing
	{
		assert(m_pData->hDC);
		return ::SetTextCharacterExtra(m_pData->hDC, nCharExtra);
	}

	inline CSize CDC::TabbedTextOut(int x, int y, LPCTSTR lpszString, int nCount, int nTabPositions, LPINT lpnTabStopPositions, int nTabOrigin) const
	// Writes a character string at a specified location, expanding tabs to the values specified in an array of tab-stop positions
	{
		assert(m_pData->hDC);
		DWORD dwSize = ::TabbedTextOut(m_pData->hDC, x, y, lpszString, nCount, nTabPositions, lpnTabStopPositions, nTabOrigin );
		CSize sz(dwSize);
		return sz;
	}

	inline BOOL CDC::TextOut(int x, int y, LPCTSTR lpszString, int nCount/* = -1*/) const
	// Writes a character string at the specified location
	{
		assert(m_pData->hDC);
		if (nCount == -1)
			nCount = lstrlen (lpszString);

		return ::TextOut(m_pData->hDC, x, y, lpszString, nCount);
	}

  #if (_WIN32_WINNT >= 0x0500) && !defined(__GNUC__)
	inline BOOL CDC::GetCharABCWidthsI(UINT giFirst, UINT cgi, LPWORD pgi, LPABC pABC) const
	// Retrieves the widths, in logical units, of consecutive glyph indices in a specified range from the
	// current TrueType font. This function succeeds only with TrueType fonts.
	{
		assert(m_pData->hDC);
		return ::GetCharABCWidthsI(m_pData->hDC, giFirst, cgi, pgi, pABC);
	}

	inline BOOL CDC::GetCharWidthI(UINT giFirst, UINT cgi, LPWORD pgi, int* pBuffer) const
	// Retrieves the widths, in logical coordinates, of consecutive glyph indices in a specified range from the current font.
	{
		assert(m_pData->hDC);
		return ::GetCharWidthI(m_pData->hDC, giFirst, cgi, pgi, pBuffer);
	}
  #endif

#endif


} // namespace Win32xx

#endif // _WIN32XX_GDI_H_

