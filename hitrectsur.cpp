#include "StdAfx.h"

#define PTINRECT(x,y) ((x) >= m_rcRect.left && (x) <= m_rcRect.right && (y) > m_rcRect.top && (y) < m_rcRect.bottom)

HitRectSur::HitRectSur(HDC hdc, float zoom, float offx, float offy, int width, int height, FRect *prcRect, Vector<ISelect> *pvsel) : Sur(hdc, zoom, offx, offy, width, height)
	{
	m_rcRect = *prcRect;
	m_pvsel = pvsel;

	if (m_rcRect.right < m_rcRect.left)
		{
		float temp = m_rcRect.right;
		m_rcRect.right = m_rcRect.left;
		m_rcRect.left = temp;
		}

	if (m_rcRect.bottom < m_rcRect.top)
		{
		float temp = m_rcRect.bottom;
		m_rcRect.bottom = m_rcRect.top;
		m_rcRect.top = temp;
		}

	SetObject(NULL);
	}

HitRectSur::~HitRectSur()
	{
	}

void HitRectSur::Line(float x, float y, float x2, float y2)
	{
	if (m_fFailedAlready)
		{
		return;
		}

	if (!PTINRECT(x,y) || !PTINRECT(x2,y2))
		{
		FailObject();
		}
	}

void HitRectSur::Rectangle(float x, float y, float x2, float y2)
	{
	if (m_fFailedAlready)
		{
		return;
		}

	if (!PTINRECT(x,y) || !PTINRECT(x2,y2))
		{
		FailObject();
		}
	}

void HitRectSur::Rectangle2(int x, int y, int x2, int y2)
	{
	return;
	}

void HitRectSur::Ellipse(float centerx, float centery, float radius)
	{
	if (m_fFailedAlready)
		{
		return;
		}

	if (!PTINRECT(centerx - radius,centery - radius) || !PTINRECT(centerx + radius,centery + radius))
		{
		FailObject();
		}
	}

void HitRectSur::Ellipse2(float centerx, float centery, int radius)
	{
	if (m_fFailedAlready)
		{
		return;
		}

	int ix = SCALEX(centerx);
	int iy = SCALEY(centery);
	int ir = (int)(radius);

	int circleleft = ix - ir;
	int circletop = iy - ir;
	int circleright = ix + ir;
	int circlebottom = iy + ir;
	
	int left = SCALEX(m_rcRect.left);
	int top = SCALEY(m_rcRect.top);
	int right = SCALEX(m_rcRect.right);
	int bottom = SCALEY(m_rcRect.bottom);
	
	if (circleleft < left || circletop < top || circleright > right || circlebottom > bottom)
		{
		FailObject();
		}
	}

void HitRectSur::Polygon(Vertex *rgv, int count)
	{
	if (m_fFailedAlready)
		{
		return;
		}

	int i;

	for (i=0;i<count;i++)
		{
		if (!PTINRECT(rgv[i].x,rgv[i].y))
			{
			FailObject();
			return;
			}
		}
	}

void HitRectSur::PolygonImage(Vertex *rgv, int count, HBITMAP hbm, float left, float top, float right, float bottom, int bitmapwidth, int bitmapheight)
	{
	Polygon(rgv, count);
	}

void HitRectSur::Polyline(Vertex *rgv, int count)
	{
	return;
	}

void HitRectSur::Arc(float x, float y, float radius, float pt1x, float pt1y, float pt2x, float pt2y)
	{
	return;
	}

void HitRectSur::Image(float x, float y, float x2, float y2, HDC hdcSrc, int width, int height)
	{
	return;
	}

void HitRectSur::SetObject(ISelect *psel)
	{
	m_pcur = psel;
	if (m_pcur)
		{
		int index = m_vselFailed.IndexOf(psel);
		if (index != -1)
			{
			// Object failed previously - just skip this time
			m_fFailedAlready = fTrue;
			}
		else
			{
			index = m_pvsel->IndexOf(psel);
			if (index == -1)
				{
				// Object not in list yet - add it
				m_indexcur = m_pvsel->AddElement(psel);
				}
			else
				{
				m_indexcur = index;
				}
			m_fFailedAlready = fFalse;
			}
		}
	else
		{
		m_fFailedAlready = fTrue;
		}
	}

void HitRectSur::FailObject()
	{
	m_fFailedAlready = fTrue;

	_ASSERTE(m_indexcur < m_pvsel->Size());

	m_pvsel->RemoveElementAt(m_indexcur); // perf?  Probably doesn't matter
	m_vselFailed.AddElement(m_pcur);
	}

void HitRectSur::SetFillColor(int rgb)
	{
	}

void HitRectSur::SetBorderColor(int rgb, BOOL fDashed, int width)
	{
	}

void HitRectSur::SetLineColor(int rgb, BOOL fDashed, int width)
	{
	}
