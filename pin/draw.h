#pragma once

#define drawzoom 0.3f

inline void DrawLine(const HDC hdc, const float x1, const float y1, const float x2, const float y2)
	{
	MoveToEx(hdc, (int)(x1*drawzoom), (int)(y1*drawzoom), NULL);
	LineTo(hdc, (int)(x2*drawzoom), (int)(y2*drawzoom));
	}

inline void DrawCircleAbsolute(const HDC hdc, const float x, const float y, const float radius)
	{
	Ellipse(hdc, (int)((x*drawzoom)-radius), (int)((y*drawzoom)-radius),
				 (int)((x*drawzoom)+radius), (int)((y*drawzoom)+radius));
	}

inline void DrawCircle(const HDC hdc, const float x, const float y, const float radius)
	{
	Ellipse(hdc, (int)((x-radius)*drawzoom), (int)((y-radius)*drawzoom),
				 (int)((x+radius)*drawzoom), (int)((y+radius)*drawzoom));
	}

inline void DrawArc(const HDC hdc, const float x, const float y, const float radius, const float pt1x, const float pt1y, const float pt2x, const float pt2y)
	{
	Arc(hdc, (int)((x-radius)*drawzoom), (int)((y-radius)*drawzoom), (int)((x+radius)*drawzoom), (int)((y+radius)*drawzoom),
			 (int)(pt1x*drawzoom), (int)(pt1y*drawzoom), (int)(pt2x*drawzoom), (int)(pt2y*drawzoom));
	}
