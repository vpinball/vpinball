#include "StdAfx.h"


ShadowSur::ShadowSur(const HDC hdc, const float zoom, const float offx, const float offy, const int width, const int height, const float z, ISelect *psel) : Sur(hdc, zoom, offx, offy, width, height)
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
	const int ix = SCALEXf(x);
	const int iy = SCALEYf(y);
	const int ix2 = SCALEXf(x2);
	const int iy2 = SCALEYf(y2);

	SelectObject(m_hdc, m_hpnLine);

	::MoveToEx(m_hdc, ix, iy, NULL);
	::LineTo(m_hdc, ix2, iy2);
	}

void ShadowSur::Rectangle(float x, float y, float x2, float y2)
	{
	const int ix = SCALEXf(x);
	const int iy = SCALEYf(y);
	const int ix2 = SCALEXf(x2);
	const int iy2 = SCALEYf(y2);

	::Rectangle(m_hdc, ix, iy, ix2, iy2);
	}

void ShadowSur::Rectangle2(int x, int y, int x2, int y2)
	{
	}

void ShadowSur::Ellipse(float centerx, float centery, float radius)
	{
	const int ix = SCALEXf(centerx);
	const int iy = SCALEYf(centery);
	const int ir = SCALEDf(radius);

	SelectObject(m_hdc, GetStockObject(BLACK_PEN));
	SelectObject(m_hdc, GetStockObject(BLACK_BRUSH));

	::Ellipse(m_hdc, ix - ir, iy - ir, ix + ir, iy + ir);
	}

void ShadowSur::Ellipse2(float centerx, float centery, int radius)
	{
	const int ix = SCALEXf(centerx);
	const int iy = SCALEYf(centery);
	const int ir = radius;

	SelectObject(m_hdc, GetStockObject(BLACK_PEN));
	SelectObject(m_hdc, GetStockObject(BLACK_BRUSH));

	::Ellipse(m_hdc, ix - ir, iy - ir, ix + ir + 1, iy + ir + 1);
	}

void ShadowSur::EllipseSkew(const float centerx, const float centery, const float radius, const float z1, const float z2) const
	{
	const int basepixel = SCALEXf(m_z);
	int bottom = SCALEXf(z1) - basepixel;
	const int top = SCALEXf(z2) - basepixel;
	
	if (top <= 0)
		{
		return; //This entire polygon is underneath this shadow level
		}

	if (bottom < 0)
		{
		bottom = 0; // Polygon crosses shadow level
		}

	const int ix = SCALEXf(centerx);
	const int iy = SCALEYf(centery);
	const int ir = SCALEDf(radius);

	SelectObject(m_hdc, GetStockObject(BLACK_PEN));
	SelectObject(m_hdc, GetStockObject(BLACK_BRUSH));

	for (int i=bottom;i<top;i++)
		{
		SetViewportOrgEx(m_hdc, i, -i, NULL);
		::Ellipse(m_hdc, ix - ir, iy - ir, ix + ir, iy + ir);
		}

	SetViewportOrgEx(m_hdc, 0, 0, NULL);
	}

void ShadowSur::Polygon(Vertex2D *rgv, int count)
	{
	POINT * const rgpt = new POINT[count];

	for (int i=0;i<count;i++)
		{
		rgpt[i].x = SCALEXf(rgv[i].x);
		rgpt[i].y = SCALEYf(rgv[i].y);
		}

	SelectObject(m_hdc, GetStockObject(BLACK_PEN));
	SelectObject(m_hdc, GetStockObject(BLACK_BRUSH));

	::Polygon(m_hdc, rgpt, count);
	
	delete rgpt;
	}

void ShadowSur::PolygonImage(Vertex2D *rgv, int count, HBITMAP hbm, float left, float top, float right, float bottom, int bitmapwidth, int bitmapheight)
	{
	Polygon(rgv, count);
	}

void ShadowSur::PolygonSkew(const Vertex2D * const rgv, const int count, const float * const rgz, const float z1, const float z2, const bool fPreClip) const
	{
	const int basepixel = SCALEXf(m_z);
	int bottom = SCALEXf(z1) - basepixel;
	int top = SCALEXf(z2) - basepixel;

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

	POINT * const rgpt = new POINT[count];

	if (rgz)
		{
		for (int i=0;i<count;i++)
			{
			rgpt[i].x = SCALEXf(rgv[i].x + rgz[i]);
			rgpt[i].y = SCALEYf(rgv[i].y - rgz[i]);
			}
		}
	else
		{
		for (int i=0;i<count;i++)
			{
			rgpt[i].x = SCALEXf(rgv[i].x);
			rgpt[i].y = SCALEYf(rgv[i].y);
			}
		}

	SelectObject(m_hdc, GetStockObject(BLACK_PEN));
	SelectObject(m_hdc, GetStockObject(BLACK_BRUSH));

	for (int i=bottom;i<top;i++)
		{
		SetViewportOrgEx(m_hdc, i, -i, NULL);
		::Polygon(m_hdc, rgpt, count);
		}

	SetViewportOrgEx(m_hdc, 0, 0, NULL);
	
	delete rgpt;
	}

void ShadowSur::PolylineSkew(const Vertex2D * const rgv, const int count, const float * const rgz, const float z1, const float z2) const
	{
	//const int basepixel = SCALEXf(m_z);
	//const int bottom = SCALEX(z1) - basepixel;
	//const int top = SCALEX(z2) - basepixel;

	POINT * const rgpt = new POINT[count];

	int cpoints = 0; // points above the shadow level

	if (rgz)
		{
		for (int i=0;i<count;i++)
			{
			if (rgz[i] > m_z)
				{
				rgpt[cpoints].x = SCALEXf(rgv[cpoints].x + rgz[cpoints]);
				rgpt[cpoints].y = SCALEYf(rgv[cpoints].y - rgz[cpoints]);
				cpoints++;
				}
			}
		}

	SelectObject(m_hdc, m_hpnLine);

	for (int i=0;i<1;i++)
		{
		SetViewportOrgEx(m_hdc, i, -i, NULL);
		::Polyline(m_hdc, rgpt, cpoints);
		}

	SetViewportOrgEx(m_hdc, 0, 0, NULL);
	
	delete rgpt;
	}

void ShadowSur::Polyline(Vertex2D *rgv, int count)
	{
	POINT * const rgpt = new POINT[count];

	for (int i=0;i<count;i++)
		{
		rgpt[i].x = SCALEXf(rgv[i].x);
		rgpt[i].y = SCALEYf(rgv[i].y);
		}

	SelectObject(m_hdc, m_hpnLine);

	::Polyline(m_hdc, rgpt, count);
	
	delete rgpt;
	}

void ShadowSur::Arc(float x, float y, float radius, float pt1x, float pt1y, float pt2x, float pt2y)
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

void ShadowSur::Image(float x, float y, float x2, float y2, HDC hdcSrc, int width, int height)
	{
	const int ix = SCALEXf(x);
	const int iy = SCALEYf(y);
	const int ix2 = SCALEXf(x2);
	const int iy2 = SCALEYf(y2);
	
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
	const int style = fDashed ? PS_DOT : PS_SOLID;

	SetBkMode(m_hdc, TRANSPARENT);

	m_hpnLine = CreatePen((rgb == -1) ? PS_NULL : style, width, rgb);
	}
