#include "StdAfx.h"


HitSur::HitSur(const HDC hdc, const float zoom, const float offx, const float offy, const int width, const int height, const int hitx, const int hity, ISelect * const pbackground) : Sur(hdc, zoom, offx, offy, width, height)
	{
	m_pselected = pbackground;

	m_hitx = hitx;
	m_hity = hity;

	SetObject(NULL);
	}

HitSur::~HitSur()
	{
	}

void HitSur::Line(const float x, const float y, const float x2, const float y2)
	{
	if (m_pcur == NULL)
		return;

	int x_1 = SCALEXf(x);
	int y_1 = SCALEYf(y);
	int x_2 = SCALEXf(x2);
	int y_2 = SCALEYf(y2);

	if (abs(x_2-x_1) > abs(y_2-y_1))
		{
		
		if (x_1>x_2)
			{
			const int temp=x_1;
			x_1=x_2;
			x_2=temp;

			const int temp2=y_1;
			y_1=y_2;
			y_2=temp2;
			}

		if (m_hitx>=x_1 && m_hitx<=x_2)
			{
				const int lineY = ((y_2-y_1)*(m_hitx-x_1))/(x_2-x_1) + y_1;

				if (m_hity>lineY-4 && m_hity<lineY+4)
					{
					m_pselected = m_pcur;
					return;
					}
			}
		}
	else
		{

		if (y_1>y_2)
			{
			const int temp=x_1;
			x_1=x_2;
			x_2=temp;

			const int temp2=y_1;
			y_1=y_2;
			y_2=temp2;
			}

		if (m_hity>=y_1 && m_hity<=y_2)
			{
				const int lineX = ((x_2-x_1)*(m_hity-y_1))/(y_2-y_1) + x_1;

				if (m_hitx>lineX-4 && m_hitx<lineX+4)	
					{
					m_pselected = m_pcur;
					return;
					}
			}
		}
	}

void HitSur::Rectangle(const float x, const float y, const float x2, float y2)
	{
	if (m_pcur == NULL)
		return;

	int ix = SCALEXf(x);
	int iy = SCALEYf(y);
	int ix2 = SCALEXf(x2);
	int iy2 = SCALEYf(y2);

	if (ix > ix2)
		{
		const int temp = ix;
		ix = ix2;
		ix2 = temp;
		}

	if (iy > iy2)
		{
		const int temp = iy;
		iy = iy2;
		iy2 = temp;
		}

	if (m_hitx >= ix && m_hitx <= ix2 && m_hity >= iy && m_hity <= iy2)
		{
		m_pselected = m_pcur;
		}
	}

void HitSur::Rectangle2(const int x, const int y, const int x2, const int y2)
	{
	if (m_pcur == NULL)
		{
		return;
		}

	}

void HitSur::Ellipse(const float centerx, const float centery, const float radius)
	{
	if (m_pcur == NULL)
		{
		return;
		}

	const int ix = SCALEXf(centerx);
	const int iy = SCALEYf(centery);
	const int ir = SCALEDf(radius);

	const int dx = m_hitx - ix;
	const int dy = m_hity - iy;
	const int dist = dx*dx + dy*dy;

	if (dist <= ir*ir)
		{
		m_pselected = m_pcur;
		}
	}

void HitSur::Ellipse2(const float centerx, const float centery, const int radius)
	{
	if (m_pcur == NULL)
		{
		return;
		}

	const int ix = SCALEXf(centerx);
	const int iy = SCALEYf(centery);

	const int dx = m_hitx - ix;
	const int dy = m_hity - iy;
	const int dist = dx*dx + dy*dy;

	if (dist <= radius*radius)
		{
		m_pselected = m_pcur;
		}
	}

void HitSur::Polygon(const Vertex2D * const rgv, const int count)
	{
	if (m_pcur == NULL)
		{
		return;
		}

	int crosscount=0;	// count of lines which the hit point is to the left of
	for (int i=0;i<count;i++)
		{
		const int j = (i==0) ? (count-1) : (i-1);

		const int x1 = SCALEXf(rgv[i].x);
		const int y1 = SCALEYf(rgv[i].y);
		const int x2 = SCALEXf(rgv[j].x);
		const int y2 = SCALEYf(rgv[j].y);
		
		if ((y1==y2) ||
		    (m_hity <= y1 && m_hity <= y2) || (m_hity > y1 && m_hity > y2) || // if out of y range, forget about this segment
			(m_hitx >= x1 && m_hitx >= x2)) // Hit point is on the right of the line
			continue;
		
		const bool xsx2 = (m_hitx < x2);
		if (m_hitx < x1 && xsx2)
			{
			crosscount++;
			continue;
			}

		if (x2 == x1)
			{
			if (xsx2)
				crosscount++;
			continue;
			}
			
		// Now the hard part - the hit point is in the line bounding box
		if (x2 - (y2 - m_hity)*(x1 - x2)/(y1 - y2) > m_hitx)
			crosscount++;				
		}
		
	if (crosscount & 1)
		{
		m_pselected = m_pcur;
		}
	}

void HitSur::PolygonImage(const Vertex2D * const rgv, const int count, HBITMAP hbm, const float left, const float top, const float right, const float bottom, const int bitmapwidth, const int bitmapheight)
	{
	Polygon(rgv, count);
	}

void HitSur::Polyline(const Vertex2D * const rgv, const int count)
	{
	}

void HitSur::Arc(const float x, const float y, const float radius, const float pt1x, const float pt1y, const float pt2x, const float pt2y)
	{
	//Ellipse(x, y, radius);
	}

void HitSur::Image(const float x, const float y, const float x2, const float y2, HDC hdcSrc, const int width, const int height)
	{

	}

void HitSur::SetObject(ISelect *psel)
	{
	m_pcur = psel;
	}

void HitSur::SetFillColor(const int rgb)
	{
	}

void HitSur::SetBorderColor(const int rgb, const bool fDashed, const int width)
	{
	}

void HitSur::SetLineColor(const int rgb, const bool fDashed, const int width)
	{
	}
