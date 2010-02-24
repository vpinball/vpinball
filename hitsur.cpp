#include "stdafx.h"
#include "main.h"

/*#define SCALEX(x) ((int)((x - m_offx)*m_zoom))
#define SCALEY(y) ((int)((y - m_offy)*m_zoom))
#define SCALED(d) ((int)(d*m_zoom))*/

HitSur::HitSur(HDC hdc, float zoom, float offx, float offy, int width, int height, int hitx, int hity, ISelect *pbackground) : Sur(hdc, zoom, offx, offy, width, height)
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

	int x1 = SCALEX(rx);
	int y1 = SCALEY(ry);
	int x2 = SCALEX(rx2);
	int y2 = SCALEY(ry2);

	int temp;

	if (abs(x2-x1) > abs(y2-y1))
		{
		
		if (x1>x2)
			{
			temp=x1;
			x1=x2;
			x2=temp;

			temp=y1;
			y1=y2;
			y2=temp;
			}

		if (m_hitx>=x1 && m_hitx<=x2)
			{
				int lineY = (int) (( ((double)(y2-y1)) / ((double)(x2-x1)) )*(m_hitx-x1)) + y1;

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
			temp=x1;
			x1=x2;
			x2=temp;

			temp=y1;
			y1=y2;
			y2=temp;
			}

		if (m_hity>=y1 && m_hity<=y2)
			{
				int lineX = (int) (( ((double)(x2-x1)) / ((double)(y2-y1)) )*(m_hity-y1)) + x1;

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

	int ix = SCALEX(x);
	int iy = SCALEY(y);
	int ix2 = SCALEX(x2);
	int iy2 = SCALEY(y2);

	if (ix > ix2)
		{
		int temp = ix;
		ix = ix2;
		ix2 = temp;
		}

	if (iy > iy2)
		{
		int temp = iy;
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

	int ix = SCALEX(centerx);
	int iy = SCALEY(centery);
	int ir = SCALED(radius);

	int dx = m_hitx - ix;
	int dy = m_hity - iy;
	int dist = (dx*dx + dy*dy);

	if (dist <= (ir*ir))
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

	int ix = SCALEX(centerx);
	int iy = SCALEY(centery);
	int ir = (int)(radius);

	int dx = m_hitx - ix;
	int dy = m_hity - iy;
	int dist = (dx*dx + dy*dy);

	if (dist <= (radius*radius))
		{
		m_pselected = m_pcur;
		}
	}

void HitSur::Polygon(Vertex *rgv, int count)
	{
	if (m_pcur == NULL)
		{
		return;
		}

	POINT *rgpt;

	rgpt = new POINT[count];

	int i;
	for (i=0;i<count;i++)
		{
		rgpt[i].x = SCALEX(rgv[i].x);
		rgpt[i].y = SCALEY(rgv[i].y);
		}

	int j;
	int x1,y1,x2,y2;
	int crosscount=0;	// count of lines which the hit point is to the left of
	for (i=0;i<count;i++)
		{
		if (i==0)
			j=count-1;
		else
			j=i-1;

		y1 = rgpt[i].y;
		y2 = rgpt[j].y;
		
		if (y1==y2)
			goto done;
			
		if ((m_hity <= y1 && m_hity <= y2) || (m_hity > y1 && m_hity > y2)) // if out of y range, forget about this segment
			goto done;
			
		x1 = rgpt[i].x;
		x2 = rgpt[j].x;
			
		if (m_hitx >= x1 && m_hitx >= x2) // Hit point is on the right of the line
			goto done;
			
		if (m_hitx < x1 && m_hitx < x2)
			{
			crosscount++;
			goto done;
			}

		if (x2 == x1)
			{
			if (m_hitx < x2)
				crosscount++;
			goto done;
			}
			
		// Now the hard part - the hit point is in the line bounding box
	
		if ( (x2 - ( (y2 - m_hity) * (x1 - x2) / (y1 - y2) ) ) > m_hitx)
			crosscount++;				
done:
;			
		}
		
	if (crosscount & 1)
		{
		m_pselected = m_pcur;
		}
	
	delete rgpt;
	}

void HitSur::PolygonImage(Vertex *rgv, int count, HBITMAP hbm, float left, float top, float right, float bottom, int bitmapwidth, int bitmapheight)
	{
	Polygon(rgv, count);
	}

void HitSur::Polyline(Vertex *rgv, int count)
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