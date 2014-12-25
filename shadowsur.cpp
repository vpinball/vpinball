#include "StdAfx.h"

float ShadowSur::m_shadowDirX =  1.0f;
float ShadowSur::m_shadowDirY = -1.0f;

ShadowSur::ShadowSur(const HDC hdc, const float zoom, const float offx, const float offy, const int width, const int height, const float z)
          : Sur(hdc, zoom, offx, offy, width, height)
{
	SelectObject(m_hdc, GetStockObject(BLACK_PEN));
	SelectObject(m_hdc, GetStockObject(BLACK_BRUSH));

	m_hpnLine = CreatePen(PS_SOLID, 1, RGB(0,0,0));

	m_z = z;
}

ShadowSur::~ShadowSur()
{
	SelectObject(m_hdc, GetStockObject(BLACK_PEN));
	SelectObject(m_hdc, GetStockObject(BLACK_BRUSH));

	DeleteObject(m_hpnLine);
}

void ShadowSur::Line(const float x, const float y, const float x2, const float y2)
{
	const int ix = SCALEXf(x);
	const int iy = SCALEYf(y);
	const int ix2 = SCALEXf(x2);
	const int iy2 = SCALEYf(y2);

	SelectObject(m_hdc, m_hpnLine);

	::MoveToEx(m_hdc, ix, iy, NULL);
	::LineTo(m_hdc, ix2, iy2);
}

void ShadowSur::Rectangle(const float x, const float y, const float x2, float y2)
{
	const int ix = SCALEXf(x);
	const int iy = SCALEYf(y);
	const int ix2 = SCALEXf(x2);
	const int iy2 = SCALEYf(y2);

	SelectObject(m_hdc, GetStockObject(BLACK_PEN));
	SelectObject(m_hdc, GetStockObject(BLACK_BRUSH));

	::Rectangle(m_hdc, ix, iy, ix2, iy2);
}

void ShadowSur::Rectangle2(const int x, const int y, const int x2, const int y2)
{
}

void ShadowSur::Ellipse(const float centerx, const float centery, const float radius)
{
	const int ix = SCALEXf(centerx);
	const int iy = SCALEYf(centery);
	const int ir = SCALEDf(radius);

	SelectObject(m_hdc, GetStockObject(BLACK_PEN));
	SelectObject(m_hdc, GetStockObject(BLACK_BRUSH));

	::Ellipse(m_hdc, ix - ir, iy - ir, ix + ir, iy + ir);
}

void ShadowSur::Ellipse2(const float centerx, const float centery, const int radius)
{
	const int ix = SCALEXf(centerx);
	const int iy = SCALEYf(centery);

	SelectObject(m_hdc, GetStockObject(BLACK_PEN));
	SelectObject(m_hdc, GetStockObject(BLACK_BRUSH));

	::Ellipse(m_hdc, ix - radius, iy - radius, ix + radius + 1, iy + radius + 1);
}

void ShadowSur::EllipseSkew(const float centerx, const float centery, const float radius, const float z1, const float z2) const
{
	const int basepixel = SCALEXf(m_z);
	const int top = SCALEXf(z2) - basepixel;

	if (top <= 0)
		return; //This entire polygon is underneath this shadow level

	int bottom = SCALEXf(z1) - basepixel;
	if (bottom < 0)
		bottom = 0; // Polygon crosses shadow level

	const int ix = SCALEXf(centerx);
	const int iy = SCALEYf(centery);
	const int ir = SCALEDf(radius);

	SelectObject(m_hdc, GetStockObject(BLACK_PEN));
	SelectObject(m_hdc, GetStockObject(BLACK_BRUSH));

	for (int i=bottom;i<top;i++)
	{
		//SetViewportOrgEx(m_hdc, i, -i, NULL);
		SetViewportOrgEx(m_hdc, (int)((float)i*m_shadowDirX), (int)((float)i*m_shadowDirY), NULL);
		::Ellipse(m_hdc, ix - ir, iy - ir, ix + ir, iy + ir);
	}

	SetViewportOrgEx(m_hdc, 0, 0, NULL);
}

void ShadowSur::Polygon(const Vertex2D * const rgv, const int count)
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

	delete [] rgpt;
}
// copy-pasted from above
void ShadowSur::Polygon(const Vector<RenderVertex> &rgv)
{
	POINT * const rgpt = new POINT[rgv.Size()];

	for (int i=0;i<rgv.Size();i++)
	{
		rgpt[i].x = SCALEXf(rgv.ElementAt(i)->x);
		rgpt[i].y = SCALEYf(rgv.ElementAt(i)->y);
	}

	SelectObject(m_hdc, GetStockObject(BLACK_PEN));
	SelectObject(m_hdc, GetStockObject(BLACK_BRUSH));

	::Polygon(m_hdc, rgpt, rgv.Size());

	delete [] rgpt;
}

void ShadowSur::PolygonImage(const Vector<RenderVertex> &rgv, HBITMAP hbm, const float left, const float top, const float right, const float bottom, const int bitmapwidth, const int bitmapheight)
{
	Polygon(rgv);
}

void ShadowSur::PolygonSkew(const Vertex2D * const rgv, const int count, const float * const rgz) const
{
	POINT * const rgpt = new POINT[count];

	for (int i=0;i<count;i++)
	{
		rgpt[i].x = SCALEXf(rgv[i].x + rgz[i]);
		rgpt[i].y = SCALEYf(rgv[i].y - rgz[i]);
	}

	SelectObject(m_hdc, GetStockObject(BLACK_PEN));
	SelectObject(m_hdc, GetStockObject(BLACK_BRUSH));

	SetViewportOrgEx(m_hdc, 0, 0, NULL);
	::Polygon(m_hdc, rgpt, count);

	delete [] rgpt;

	SetViewportOrgEx(m_hdc, 0, 0, NULL);
}

void ShadowSur::PolygonSkew(const Vector<RenderVertex> &rgv, const float z1, const float z2) const
{
	const int basepixel = SCALEXf(m_z);
	const int top = SCALEXf(z2) - basepixel;

	if (top <= 0)
		return; //This entire polygon is underneath this shadow level

	int bottom = SCALEXf(z1) - basepixel;
	if (bottom < 0)
		bottom = 0; // Polygon crosses shadow level

	const int count = rgv.Size();
	POINT * const rgpt = new POINT[count];

	for (int i=0;i<count;i++)
	{
		rgpt[i].x = SCALEXf(rgv.ElementAt(i)->x);
		rgpt[i].y = SCALEYf(rgv.ElementAt(i)->y);
	}

	SelectObject(m_hdc, GetStockObject(BLACK_PEN));
	SelectObject(m_hdc, GetStockObject(BLACK_BRUSH));

	for (int i=bottom;i<top;i++)
	{
		//SetViewportOrgEx(m_hdc, i, -i, NULL);
		SetViewportOrgEx(m_hdc, (int)((float)i*m_shadowDirX), (int)((float)i*m_shadowDirY), NULL);
		::Polygon(m_hdc, rgpt, count);
	}

	delete [] rgpt;

	SetViewportOrgEx(m_hdc, 0, 0, NULL);
}

void ShadowSur::PolylineSkew(const Vertex2D * const rgv, const int count, const float * const rgz, const float z1, const float z2) const
{
	//const int basepixel = SCALEXf(m_z);
	//const int bottom = SCALEX(z1) - basepixel;
	//const int top = SCALEX(z2) - basepixel;

	POINT * const rgpt = new POINT[count];

	int cpoints = 0; // points above the shadow level

	if (rgz)
		for (int i=0;i<count;++i)
			if (rgz[i] > m_z)
			{
				rgpt[cpoints].x = SCALEXf(rgv[cpoints].x + rgz[cpoints]);
				rgpt[cpoints].y = SCALEYf(rgv[cpoints].y - rgz[cpoints]);
				++cpoints;
			}

	SelectObject(m_hdc, m_hpnLine);

	for (int i=0;i<1;++i)
	{
		//SetViewportOrgEx(m_hdc, i, -i, NULL);
		SetViewportOrgEx(m_hdc, (int)((float)i*m_shadowDirX), (int)((float)i*m_shadowDirY), NULL);

		::Polyline(m_hdc, rgpt, cpoints);
	}

	delete [] rgpt;

	SetViewportOrgEx(m_hdc, 0, 0, NULL);
}

void ShadowSur::Polyline(const Vertex2D * const rgv, const int count)
{
	POINT * const rgpt = new POINT[count];

	for (int i=0;i<count;++i)
	{
		rgpt[i].x = SCALEXf(rgv[i].x);
		rgpt[i].y = SCALEYf(rgv[i].y);
	}

	SelectObject(m_hdc, m_hpnLine);

	::Polyline(m_hdc, rgpt, count);

	delete [] rgpt;
}

void ShadowSur::Arc(const float x, const float y, const float radius, const float pt1x, const float pt1y, const float pt2x, const float pt2y)
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

void ShadowSur::Image(const float x, const float y, const float x2, const float y2, HDC hdcSrc, const int width, const int height)
{
	const int ix = SCALEXf(x);
	const int iy = SCALEYf(y);
	const int ix2 = SCALEXf(x2);
	const int iy2 = SCALEYf(y2);

	StretchBlt(m_hdc, ix, iy, ix2-ix, iy2-iy, hdcSrc, 0, 0, width, height, SRCCOPY);
}

void ShadowSur::SetObject(ISelect *psel)
{
	//if ((m_psel != NULL) && (psel == m_psel))
	//	m_psel->SetSelectFormat(this);
}

void ShadowSur::SetFillColor(const int rgb)
{
}

void ShadowSur::SetBorderColor(const int rgb, const bool fDashed, const int width)
{
}

void ShadowSur::SetLineColor(const int rgb, const bool fDashed, const int width)
{
	SelectObject(m_hdc, GetStockObject(BLACK_PEN));
	DeleteObject(m_hpnLine);
	SetBkMode(m_hdc, TRANSPARENT);
	const int style = fDashed ? PS_DOT : PS_SOLID;
	m_hpnLine = CreatePen((rgb == -1) ? PS_NULL : style, width, rgb);
}
