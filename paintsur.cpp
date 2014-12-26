#include "StdAfx.h"

PaintSur::PaintSur(const HDC hdc, const float zoom, const float offx, const float offy, const int width, const int height, ISelect * const psel)
         : Sur(hdc, zoom, offx, offy, width, height)
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

void PaintSur::Line(const float x, const float y, const float x2, const float y2)
	{
	const int ix = SCALEXf(x);
	const int iy = SCALEYf(y);
	const int ix2 = SCALEXf(x2);
	const int iy2 = SCALEYf(y2);

	SelectObject(m_hdc, m_hpnLine);

	::MoveToEx(m_hdc, ix, iy, NULL);
	::LineTo(m_hdc, ix2, iy2);
	::LineTo(m_hdc, ix, iy); // To get the last pixel drawn
	}

void PaintSur::Rectangle(const float x, const float y, const float x2, float y2)
	{
	const int ix = SCALEXf(x);
	const int iy = SCALEYf(y);
	const int ix2 = SCALEXf(x2);
	const int iy2 = SCALEYf(y2);

	SelectObject(m_hdc, m_hbr);
	SelectObject(m_hdc, m_hpnOutline);

	::Rectangle(m_hdc, ix, iy, ix2, iy2);
	}

void PaintSur::Rectangle2(const int x, const int y, const int x2, const int y2)
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
	const int ix = SCALEXf(centerx);
	const int iy = SCALEYf(centery);
	const int ir = SCALEDf(radius);

	SelectObject(m_hdc, m_hbr);
	SelectObject(m_hdc, m_hpnOutline);

	::Ellipse(m_hdc, ix - ir, iy - ir, ix + ir, iy + ir);
	}

void PaintSur::Ellipse2(const float centerx, const float centery, const int radius)
	{
	const int ix = SCALEXf(centerx);
	const int iy = SCALEYf(centery);
	const int ir = radius;

	SelectObject(m_hdc, m_hbr);
	SelectObject(m_hdc, m_hpnOutline);

	::Ellipse(m_hdc, ix - ir, iy - ir, ix + ir + 1, iy + ir + 1);
	}

void PaintSur::Polygon(const Vertex2D * const rgv, const int count)
	{
	POINT * const rgpt = new POINT[count];

	for (int i=0;i<count;i++)
		{
		rgpt[i].x = SCALEXf(rgv[i].x);
		rgpt[i].y = SCALEYf(rgv[i].y);
		}

	SelectObject(m_hdc, m_hbr);
	SelectObject(m_hdc, m_hpnOutline);

	::Polygon(m_hdc, rgpt, count);
	
	delete [] rgpt;
	}

// copy-pasted from above
void PaintSur::Polygon(const Vector<RenderVertex> &rgv)
	{
	POINT * const rgpt = new POINT[rgv.Size()];

	for (int i=0;i<rgv.Size();i++)
		{
		rgpt[i].x = SCALEXf(rgv.ElementAt(i)->x);
		rgpt[i].y = SCALEYf(rgv.ElementAt(i)->y);
		}

	SelectObject(m_hdc, m_hbr);
	SelectObject(m_hdc, m_hpnOutline);

	::Polygon(m_hdc, rgpt, rgv.Size());
	
	delete [] rgpt;
	}

void PaintSur::PolygonImage(const Vector<RenderVertex> &rgv, HBITMAP hbm, const float left, const float top, const float right, const float bottom, const int bitmapwidth, const int bitmapheight)
	{
	const int ix = SCALEXf(left);
	const int iy = SCALEYf(top);
	const int ix2 = SCALEXf(right);
	const int iy2 = SCALEYf(bottom);

	HDC hdcNew = CreateCompatibleDC(m_hdc);
	HBITMAP hbmOld = (HBITMAP)SelectObject(hdcNew, hbm);
	
	SetStretchBltMode(m_hdc, HALFTONE); // somehow enables filtering
	StretchBlt(m_hdc, ix, iy, ix2-ix, iy2-iy, hdcNew, 0, 0, bitmapwidth, bitmapheight, SRCINVERT);

	SelectObject(m_hdc, GetStockObject(BLACK_BRUSH));
	SelectObject(m_hdc, GetStockObject(NULL_PEN));

	POINT * const rgpt = new POINT[rgv.Size()];

	for (int i=0;i<rgv.Size();i++)
	{
		rgpt[i].x = SCALEXf(rgv.ElementAt(i)->x);
		rgpt[i].y = SCALEYf(rgv.ElementAt(i)->y);
	}

	::Polygon(m_hdc, rgpt, rgv.Size());
			
	SetStretchBltMode(m_hdc, HALFTONE); // somehow enables filtering
	StretchBlt(m_hdc, ix, iy, ix2-ix, iy2-iy, hdcNew, 0, 0, bitmapwidth, bitmapheight, SRCINVERT);
	
	SelectObject(hdcNew, hbmOld);
	DeleteDC(hdcNew);
	
	delete [] rgpt;
	}

void PaintSur::Polyline(const Vertex2D * const rgv, const int count)
{
    m_ptCache.resize(count);

    for (int i=0;i<count;i++)
    {
        m_ptCache[i].x = SCALEXf(rgv[i].x);
        m_ptCache[i].y = SCALEYf(rgv[i].y);
    }

    SelectObject(m_hdc, m_hpnLine);

    /*
     * There seems to be a known GDI bug where drawing very large polylines in one
     * call freezes the system shortly, so we batch them into groups of 1000.
     */
    for (int i = 0; i < count; i += 1000)
    {
        const int batchSize = std::min(count - i, 1001);
        ::Polyline(m_hdc, &m_ptCache[i], batchSize);
    }
}

void PaintSur::Arc(const float x, const float y, const float radius, const float pt1x, const float pt1y, const float pt2x, const float pt2y)
	{
	const int ix = SCALEXf(x);
	const int iy = SCALEYf(y);
	const int ir = SCALEDf(radius);

	const int x1 = SCALEXf(pt1x);
	const int y1 = SCALEYf(pt1y);
	const int x2 = SCALEXf(pt2x);
	const int y2 = SCALEYf(pt2y);

	SelectObject(m_hdc, m_hpnLine);

	::Arc(m_hdc, ix-ir, iy-ir, ix+ir, iy+ir, x1, y1, x2, y2);
	}

void PaintSur::Image(const float x, const float y, const float x2, const float y2, HDC hdcSrc, const int width, const int height)
	{
	const int ix = SCALEXf(x);
	const int iy = SCALEYf(y);
	const int ix2 = SCALEXf(x2);
	const int iy2 = SCALEYf(y2);
	
	SetStretchBltMode(m_hdc, HALFTONE); // somehow enables filtering
	StretchBlt(m_hdc, ix, iy, ix2-ix, iy2-iy, hdcSrc, 0, 0, width, height, SRCCOPY);
	}

void PaintSur::SetObject(ISelect *psel)
	{
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

void PaintSur::SetFillColor(const int rgb)
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

void PaintSur::SetBorderColor(const int rgb, const bool fDashed, const int width)
	{
	SelectObject(m_hdc, GetStockObject(BLACK_PEN));
	DeleteObject(m_hpnOutline);	

	if (rgb == -1)
		{
		m_hpnOutline = CreatePen(PS_NULL, width, rgb);
		m_fNullBorder = true;
		}
	else
		{
		const int style = fDashed ? PS_DOT : PS_SOLID;
		m_hpnOutline = CreatePen(style, width, rgb);
		m_fNullBorder = false;
		}
	}

void PaintSur::SetLineColor(const int rgb, const bool fDashed, const int width)
	{
	SelectObject(m_hdc, GetStockObject(BLACK_PEN));
	DeleteObject(m_hpnLine);	

	SetBkMode(m_hdc, TRANSPARENT);

	if (rgb == -1)
		{
		m_hpnLine = CreatePen(PS_NULL, width, rgb);
		}
	else
		{
		const int style = fDashed ? PS_DOT : PS_SOLID;
		m_hpnLine = CreatePen(style, width, rgb);
		}
	}
