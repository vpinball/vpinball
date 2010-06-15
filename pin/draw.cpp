#include "stdafx.h"

#define zoom 0.3f

void DrawLine(const HDC hdc, const float x1, const float y1, const float x2, const float y2)
	{
	MoveToEx(hdc, (int)(x1*zoom), (int)(y1*zoom), NULL);
	LineTo(hdc, (int)(x2*zoom), (int)(y2*zoom));
	}

void DrawCircleAbsolute(const HDC hdc, const float x, const float y, const float radius)
	{
	Ellipse(hdc, (int)((x*zoom)-radius), (int)((y*zoom)-radius),
				 (int)((x*zoom)+radius), (int)((y*zoom)+radius));
	}

void DrawCircle(const HDC hdc, const float x, const float y, const float radius)
	{
	Ellipse(hdc, (int)((x-radius)*zoom), (int)((y-radius)*zoom),
				 (int)((x+radius)*zoom), (int)((y+radius)*zoom));
	}

void DrawArc(const HDC hdc, const float x, const float y, const float radius, const float pt1x, const float pt1y, const float pt2x, const float pt2y)
	{
	Arc(hdc, (int)((x-radius)*zoom), (int)((y-radius)*zoom), (int)((x+radius)*zoom), (int)((y+radius)*zoom),
			 (int)(pt1x*zoom), (int)(pt1y*zoom), (int)(pt2x*zoom), (int)(pt2y*zoom));
	}
