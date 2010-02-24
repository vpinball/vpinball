#include "stdafx.h"
#include "main.h"

/*#define SCALEX(x) ((int)((x - m_offx)*m_zoom))
#define SCALEY(y) ((int)((y - m_offy)*m_zoom))
#define SCALED(d) ((int)(d*m_zoom))*/

ShadowSur::ShadowSur(HDC hdc, float zoom, float offx, float offy, int width, int height, float z, ISelect *psel) : Sur(hdc, zoom, offx, offy, width, height)
	{
	SelectObject(m_hdc, GetStockObject(BLACK_PEN));
	SelectObject(m_hdc, GetStockObject(BLACK_BRUSH));

	m_hpnLine = CreatePen(PS_SOLID, 1, RGB(0,0,0));

	m_z = z;

	m_psel = psel;
	}

ShadowSur::~ShadowSur()
	{
	SelectObject(m_hdc, GetStockObject(BLACK_PEN));
	SelectObject(m_hdc, GetStockObject(BLACK_BRUSH));

	DeleteObject(m_hpnLine);
	}

void ShadowSur::Line(float x, float y, float x2, float y2)
	{
	int ix = SCALEX(x);
	int iy = SCALEY(y);
	int ix2 = SCALEX(x2);
	int iy2 = SCALEY(y2);

	SelectObject(m_hdc, m_hpnLine);

	::MoveToEx(m_hdc, ix, iy, NULL);
	::LineTo(m_hdc, ix2, iy2);
	}

void ShadowSur::Rectangle(float x, float y, float x2, float y2)
	{
	int ix = SCALEX(x);
	int iy = SCALEY(y);
	int ix2 = SCALEX(x2);
	int iy2 = SCALEY(y2);

	//SelectObject(m_hdc, m_hbr);
	//SelectObject(m_hdc, m_hpnOutline);

	::Rectangle(m_hdc, ix, iy, ix2, iy2);
	}

void ShadowSur::Rectangle2(int x, int y, int x2, int y2)
	{
	//SelectObject(m_hdc, m_hbr);
	//SelectObject(m_hdc, m_hpnOutline);

	/*if (m_fNullBorder)
		{
		::Rectangle(m_hdc, x, y, x2+1, y2+1);
		}
	else
		{
		::Rectangle(m_hdc, x, y, x2, y2);
		}*/
	}

void ShadowSur::Ellipse(float centerx, float centery, float radius)
	{
	int ix = SCALEX(centerx);
	int iy = SCALEY(centery);
	int ir = SCALED(radius);

	SelectObject(m_hdc, GetStockObject(BLACK_PEN));
	SelectObject(m_hdc, GetStockObject(BLACK_BRUSH));

	::Ellipse(m_hdc, ix - ir, iy - ir, ix + ir, iy + ir);
	}

void ShadowSur::Ellipse2(float centerx, float centery, int radius)
	{
	int ix = SCALEX(centerx);
	int iy = SCALEY(centery);
	int ir = radius;

	SelectObject(m_hdc, GetStockObject(BLACK_PEN));
	SelectObject(m_hdc, GetStockObject(BLACK_BRUSH));

	::Ellipse(m_hdc, ix - ir, iy - ir, ix + ir + 1, iy + ir + 1);
	}

void ShadowSur::EllipseSkew(float centerx, float centery, float radius, float z1, float z2)
	{
	int basepixel = SCALEX(m_z);
	int bottom = SCALEX(z1) - basepixel;
	int top = SCALEX(z2) - basepixel;
	int i;

	if (top <= 0)
		{
		return; //This entire polygon is underneath this shadow level
		}

	if (bottom < 0)
		{
		bottom = 0; // Polygon crosses shadow level
		}

	int ix = SCALEX(centerx);
	int iy = SCALEY(centery);
	int ir = SCALED(radius);

	SelectObject(m_hdc, GetStockObject(BLACK_PEN));
	SelectObject(m_hdc, GetStockObject(BLACK_BRUSH));

	for (i=bottom;i<top;i++)
		{
		SetViewportOrgEx(m_hdc, i, -i, NULL);
		::Ellipse(m_hdc, ix - ir, iy - ir, ix + ir, iy + ir);
		}

	SetViewportOrgEx(m_hdc, 0, 0, NULL);
	}

void ShadowSur::Polygon(Vertex *rgv, int count)
	{
	POINT *rgpt;

	rgpt = new POINT[count];

	int i;
	for (i=0;i<count;i++)
		{
		rgpt[i].x = SCALEX(rgv[i].x);
		rgpt[i].y = SCALEY(rgv[i].y);
		}

	SelectObject(m_hdc, GetStockObject(BLACK_PEN));
	SelectObject(m_hdc, GetStockObject(BLACK_BRUSH));

	::Polygon(m_hdc, rgpt, count);
	
	delete rgpt;
	}

void ShadowSur::PolygonImage(Vertex *rgv, int count, HBITMAP hbm, float left, float top, float right, float bottom, int bitmapwidth, int bitmapheight)
	{
	Polygon(rgv, count);
	}

void ShadowSur::PolygonSkew(Vertex *rgv, int count, float *rgz, float z1, float z2, BOOL fPreClip)
	{
	POINT *rgpt;

	int basepixel = SCALEX(m_z);
	int bottom = SCALEX(z1) - basepixel;
	int top = SCALEX(z2) - basepixel;

	if (!fPreClip)
		{
		if (top <= 0)
			{
			return; //This entire polygon is underneath this shadow level
			}

		if (bottom < 0)
			{
			bottom = 0; // Polygon crosses shadow level
			}
		}
	else
		{
		top = 1;
		bottom = 0;
		}

	rgpt = new POINT[count];

	int i;

	if (rgz)
		{
		for (i=0;i<count;i++)
			{
			rgpt[i].x = SCALEX(rgv[i].x + rgz[i]);
			rgpt[i].y = SCALEY(rgv[i].y - rgz[i]);
			}
		}
	else
		{
		for (i=0;i<count;i++)
			{
			rgpt[i].x = SCALEX(rgv[i].x);
			rgpt[i].y = SCALEY(rgv[i].y);
			}
		}

	SelectObject(m_hdc, GetStockObject(BLACK_PEN));
	SelectObject(m_hdc, GetStockObject(BLACK_BRUSH));

	for (i=bottom;i<top;i++)
		{
		SetViewportOrgEx(m_hdc, i, -i, NULL);
		::Polygon(m_hdc, rgpt, count);
		}

	SetViewportOrgEx(m_hdc, 0, 0, NULL);
	
	delete rgpt;
	}

void ShadowSur::PolylineSkew(Vertex *rgv, int count, float *rgz, float z1, float z2)
	{
	POINT *rgpt;

	int basepixel = SCALEX(m_z);
	int bottom = SCALEX(z1) - basepixel;
	int top = SCALEX(z2) - basepixel;

	/*if (top <= 0)
		{
		return; //This entire polygon is underneath this shadow level
		}

	if (bottom < 0)
		{
		bottom = 0; // Polygon crosses shadow level
		}*/

	rgpt = new POINT[count];

	int i;
	int cpoints = 0; // points above the shadow level

	if (rgz)
		{
		for (i=0;i<count;i++)
			{
			if (rgz[i] > m_z)
				{
				rgpt[cpoints].x = SCALEX(rgv[cpoints].x + rgz[cpoints]);
				rgpt[cpoints].y = SCALEY(rgv[cpoints].y - rgz[cpoints]);
				cpoints++;
				}
			}
		}

	SelectObject(m_hdc, m_hpnLine);

	//for (i=bottom;i<top;i++)
	for (i=0;i<1;i++)
		{
		SetViewportOrgEx(m_hdc, i, -i, NULL);
		::Polyline(m_hdc, rgpt, cpoints);
		}

	SetViewportOrgEx(m_hdc, 0, 0, NULL);
	
	delete rgpt;
	}

void ShadowSur::Polyline(Vertex *rgv, int count)
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

void ShadowSur::Arc(float x, float y, float radius, float pt1x, float pt1y, float pt2x, float pt2y)
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

void ShadowSur::Image(float x, float y, float x2, float y2, HDC hdcSrc, int width, int height)
	{
	int ix = SCALEX(x);
	int iy = SCALEY(y);
	int ix2 = SCALEX(x2);
	int iy2 = SCALEY(y2);
	
	StretchBlt(m_hdc, ix, iy, ix2-ix, iy2-iy, hdcSrc, 0, 0, width, height, SRCCOPY);
	}

void ShadowSur::SetObject(ISelect *psel)
	{
	if ((m_psel != NULL) && (psel == m_psel))
		{
		m_psel->SetSelectFormat(this);
		}
	}

void ShadowSur::SetFillColor(int rgb)
	{
	}

void ShadowSur::SetBorderColor(int rgb, BOOL fDashed, int width)
	{
	}

void ShadowSur::SetLineColor(int rgb, BOOL fDashed, int width)
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