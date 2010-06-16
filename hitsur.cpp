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

void HitSur::Line(float rx, float ry, float rx2, float ry2)
	{
	if (m_pcur == NULL)
		{
		return;
		}

	int x1 = SCALEXf(rx);
	int y1 = SCALEYf(ry);
	int x2 = SCALEXf(rx2);
	int y2 = SCALEYf(ry2);

	if (abs(x2-x1) > abs(y2-y1))
		{
		
		if (x1>x2)
			{
			const int temp=x1;
			x1=x2;
			x2=temp;

			const int temp2=y1;
			y1=y2;
			y2=temp2;
			}

		if (m_hitx>=x1 && m_hitx<=x2)
			{
				const int lineY = ((y2-y1)*(m_hitx-x1))/(x2-x1) + y1;

				if (m_hity>lineY-4 && m_hity<lineY+4)
					{
					m_pselected = m_pcur;
					return;
					}
			}
		}
	else
		{

		if (y1>y2)
			{
			const int temp=x1;
			x1=x2;
			x2=temp;

			const int temp2=y1;
			y1=y2;
			y2=temp2;
			}

		if (m_hity>=y1 && m_hity<=y2)
			{
				const int lineX = ((x2-x1)*(m_hity-y1))/(y2-y1) + x1;

				if (m_hitx>lineX-4 && m_hitx<lineX+4)	
					{
					m_pselected = m_pcur;
					return;
					}
			}
		}
	}

void HitSur::Rectangle(float x, float y, float x2, float y2)
	{
	if (m_pcur == NULL)
		{
		return;
		}

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

void HitSur::Rectangle2(int x, int y, int x2, int y2)
	{
	if (m_pcur == NULL)
		{
		return;
		}

	}

void HitSur::Ellipse(float centerx, float centery, float radius)
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

void HitSur::Ellipse2(float centerx, float centery, int radius)
	{
	if (m_pcur == NULL)
		{
		return;
		}

	const int ix = SCALEXf(centerx);
	const int iy = SCALEYf(centery);
	//int ir = (int)(radius);

	const int dx = m_hitx - ix;
	const int dy = m_hity - iy;
	const int dist = dx*dx + dy*dy;

	if (dist <= radius*radius)
		{
		m_pselected = m_pcur;
		}
	}

void HitSur::Polygon(Vertex2D *rgv, int count)
	{
	if (m_pcur == NULL)
		{
		return;
		}

	POINT * const rgpt = new POINT[count];

	for (int i=0;i<count;i++)
		{
		rgpt[i].x = SCALEXf(rgv[i].x);
		rgpt[i].y = SCALEYf(rgv[i].y);
		}

	int crosscount=0;	// count of lines which the hit point is to the left of
	for (int i=0;i<count;i++)
		{
		const int j = (i==0) ? (count-1) : (i-1);

		const int x1 = rgpt[i].x;
		const int y1 = rgpt[i].y;
		const int x2 = rgpt[j].x;
		const int y2 = rgpt[j].y;
		
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
	
	delete rgpt;
	}

void HitSur::PolygonImage(Vertex2D *rgv, int count, HBITMAP hbm, float left, float top, float right, float bottom, int bitmapwidth, int bitmapheight)
	{
	Polygon(rgv, count);
	}

void HitSur::Polyline(Vertex2D *rgv, int count)
	{
	}

void HitSur::Arc(float x, float y, float radius, float pt1x, float pt1y, float pt2x, float pt2y)
	{
	//Ellipse(x, y, radius);
	}

void HitSur::Image(float x, float y, float x2, float y2, HDC hdcSrc, int width, int height)
	{

	}

void HitSur::SetObject(ISelect *psel)
	{
	m_pcur = psel;
	}

void HitSur::SetFillColor(int rgb)
	{
	}

void HitSur::SetBorderColor(int rgb, BOOL fDashed, int width)
	{
	}

void HitSur::SetLineColor(int rgb, BOOL fDashed, int width)
	{
	}
