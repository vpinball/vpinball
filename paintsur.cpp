#include "stdafx.h"
#include "main.h"

/*#define SCALEX(x) ((int)((x - m_offx)*m_zoom))
#define SCALEY(y) ((int)((y - m_offy)*m_zoom))
#define SCALED(d) ((int)(d*m_zoom))*/

PaintSur::PaintSur(HDC hdc, float zoom, float offx, float offy, int width, int height, ISelect *psel) : Sur(hdc, zoom, offx, offy, width, height)
	{
	m_hbr = CreateSolidBrush(RGB(255,255,255));
	m_hpnOutline = CreatePen(PS_SOLID, 1, RGB(0,0,0));
	m_hpnLine = CreatePen(PS_SOLID, 1, RGB(0,0,0));
	m_psel = psel;
	}

PaintSur::~PaintSur()
	{
	SelectObject(m_hdc, GetStockObject(BLACK_PEN));
	SelectObject(m_hdc, GetStockObject(BLACK_BRUSH));

	DeleteObject(m_hpnLine);
	DeleteObject(m_hbr);
	DeleteObject(m_hpnOutline);
	}

void PaintSur::Line(float x, float y, float x2, float y2)
	{
	int ix = SCALEX(x);
	int iy = SCALEY(y);
	int ix2 = SCALEX(x2);
	int iy2 = SCALEY(y2);

	SelectObject(m_hdc, m_hpnLine);

	::MoveToEx(m_hdc, ix, iy, NULL);
	::LineTo(m_hdc, ix2, iy2);
	::LineTo(m_hdc, ix, iy); // To get the last pixel drawn
	}

void PaintSur::Rectangle(float x, float y, float x2, float y2)
	{
	int ix = SCALEX(x);
	int iy = SCALEY(y);
	int ix2 = SCALEX(x2);
	int iy2 = SCALEY(y2);

	SelectObject(m_hdc, m_hbr);
	SelectObject(m_hdc, m_hpnOutline);

	::Rectangle(m_hdc, ix, iy, ix2, iy2);
	}

void PaintSur::Rectangle2(int x, int y, int x2, int y2)
	{
	SelectObject(m_hdc, m_hbr);
	SelectObject(m_hdc, m_hpnOutline);

	if (m_fNullBorder)
		{
		::Rectangle(m_hdc, x, y, x2+1, y2+1);
		}
	else
		{
		::Rectangle(m_hdc, x, y, x2, y2);
		}
	}

void PaintSur::Ellipse(float centerx, float centery, float radius)
	{
	int ix = SCALEX(centerx);
	int iy = SCALEY(centery);
	int ir = SCALED(radius);

	SelectObject(m_hdc, m_hbr);
	SelectObject(m_hdc, m_hpnOutline);

	::Ellipse(m_hdc, ix - ir, iy - ir, ix + ir, iy + ir);
	}

void PaintSur::Ellipse2(float centerx, float centery, int radius)
	{
	int ix = SCALEX(centerx);
	int iy = SCALEY(centery);
	int ir = radius;

	SelectObject(m_hdc, m_hbr);
	SelectObject(m_hdc, m_hpnOutline);

	::Ellipse(m_hdc, ix - ir, iy - ir, ix + ir + 1, iy + ir + 1);
	}

void PaintSur::Polygon(Vertex *rgv, int count)
	{
	POINT *rgpt;

	rgpt = new POINT[count];

	int i;
	for (i=0;i<count;i++)
		{
		rgpt[i].x = SCALEX(rgv[i].x);
		rgpt[i].y = SCALEY(rgv[i].y);
		}

	SelectObject(m_hdc, m_hbr);
	SelectObject(m_hdc, m_hpnOutline);

	::Polygon(m_hdc, rgpt, count);
	
	delete rgpt;
	}

void PaintSur::PolygonImage(Vertex *rgv, int count, HBITMAP hbm, float left, float top, float right, float bottom, int bitmapwidth, int bitmapheight)
	{
	POINT *rgpt;

	rgpt = new POINT[count];

	int i;
	for (i=0;i<count;i++)
		{
		rgpt[i].x = SCALEX(rgv[i].x);
		rgpt[i].y = SCALEY(rgv[i].y);
		}

	int ix = SCALEX(left);
	int iy = SCALEY(top);
	int ix2 = SCALEX(right);
	int iy2 = SCALEY(bottom);

	HDC hdcNew;
	HBITMAP hbmOld;

	hdcNew = CreateCompatibleDC(m_hdc);
	hbmOld = (HBITMAP)SelectObject(hdcNew, hbm);
	
	StretchBlt(m_hdc, ix, iy, ix2-ix, iy2-iy, hdcNew, 0, 0, bitmapwidth, bitmapheight, SRCINVERT);

	SelectObject(m_hdc, GetStockObject(BLACK_BRUSH));
	SelectObject(m_hdc, GetStockObject(NULL_PEN));
	::Polygon(m_hdc, rgpt, count);
			
	StretchBlt(m_hdc, ix, iy, ix2-ix, iy2-iy, hdcNew, 0, 0, bitmapwidth, bitmapheight, SRCINVERT);
	
	SelectObject(hdcNew, hbmOld);
	DeleteDC(hdcNew);
	
	delete rgpt;
	}

void PaintSur::Polyline(Vertex *rgv, int count)
	{
	POINT *rgpt;

	rgpt = new POINT[count];

	int i;
	for (i=0;i<count;i++)
		{
		rgpt[i].x = SCALEX(rgv[i].x);
		rgpt[i].y = SCALEY(rgv[i].y);
		}

	SelectObject(m_hdc, m_hpnLine);

	::Polyline(m_hdc, rgpt, count);
	
	delete rgpt;
	}

void PaintSur::Arc(float x, float y, float radius, float pt1x, float pt1y, float pt2x, float pt2y)
	{
	int ix = SCALEX(x);
	int iy = SCALEY(y);
	int ir = SCALED(radius);

	int x1 = SCALEX(pt1x);
	int y1 = SCALEY(pt1y);
	int x2 = SCALEX(pt2x);
	int y2 = SCALEY(pt2y);

	SelectObject(m_hdc, m_hpnLine);

	::Arc(m_hdc, ix-ir, iy-ir, ix+ir, iy+ir, x1, y1, x2, y2);
	}

void PaintSur::Image(float x, float y, float x2, float y2, HDC hdcSrc, int width, int height)
	{
	int ix = SCALEX(x);
	int iy = SCALEY(y);
	int ix2 = SCALEX(x2);
	int iy2 = SCALEY(y2);
	
	StretchBlt(m_hdc, ix, iy, ix2-ix, iy2-iy, hdcSrc, 0, 0, width, height, SRCCOPY);
	}

void PaintSur::SetObject(ISelect *psel)
	{
	/*if ((m_psel != NULL) && (psel == m_psel))
		{
		m_psel->SetSelectFormat(this);
		}*/

	if ((m_psel != NULL) && (psel != NULL)) // m_psel can be null when rendering a blueprint or other item which has no selection feedback
		{
		if (psel->m_selectstate == eSelected)
			{
			psel->SetSelectFormat(this);
			}
		else if (psel->m_selectstate == eMultiSelected)
			{
			psel->SetMultiSelectFormat(this);
			}
		else if (psel->m_fLocked)
			{
			psel->SetLockedFormat(this);
			}
		}
	}

void PaintSur::SetFillColor(int rgb)
	{
	SelectObject(m_hdc, GetStockObject(BLACK_BRUSH));
	DeleteObject(m_hbr);

	if (rgb == -1)
		{
		LOGBRUSH lbr;
		lbr.lbStyle = BS_NULL;
		m_hbr = CreateBrushIndirect(&lbr);
		}
	else
		{
		m_hbr = CreateSolidBrush(rgb);
		}
	}

void PaintSur::SetBorderColor(int rgb, BOOL fDashed, int width)
	{
	SelectObject(m_hdc, GetStockObject(BLACK_PEN));
	DeleteObject(m_hpnOutline);
	int style = fDashed ? PS_DOT : PS_SOLID;

	if (rgb == -1)
		{
		m_hpnOutline = CreatePen(PS_NULL, width, rgb);
		m_fNullBorder = fTrue;
		}
	else
		{
		m_hpnOutline = CreatePen(style, width, rgb);
		m_fNullBorder = fFalse;
		}
	}

void PaintSur::SetLineColor(int rgb, BOOL fDashed, int width)
	{
	SelectObject(m_hdc, GetStockObject(BLACK_PEN));
	DeleteObject(m_hpnLine);
	int style = fDashed ? PS_DOT : PS_SOLID;

	SetBkMode(m_hdc, TRANSPARENT);

	if (rgb == -1)
		{
		m_hpnLine = CreatePen(PS_NULL, width, rgb);
		}
	else
		{
		m_hpnLine = CreatePen(style, width, rgb);
		}
	}