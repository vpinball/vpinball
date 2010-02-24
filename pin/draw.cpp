#include "stdafx.h"

#define zoom 0.3

void DrawLine(HDC hdc, float x1, float y1, float x2, float y2)
	{
	MoveToEx(hdc, (int)(x1*zoom), (int)(y1*zoom), NULL);
	LineTo(hdc, (int)(x2*zoom), (int)(y2*zoom));
	}

void DrawCircleAbsolute(HDC hdc, float x, float y, float radius)
	{
	Ellipse(hdc, (int)((x*zoom)-radius), (int)((y*zoom)-radius),
		(int)((x*zoom)+radius), (int)((y*zoom)+radius));
	}

void DrawCircle(HDC hdc, float x, float y, float radius)
	{
	Ellipse(hdc, (int)((x-radius)*zoom), (int)((y-radius)*zoom),
		(int)((x+radius)*zoom), (int)((y+radius)*zoom));
	}

void DrawArc(HDC hdc, float x, float y, float radius, float pt1x, float pt1y, float pt2x, float pt2y)
	{
	Arc(hdc, (int)((x-radius)*zoom), (int)((y-radius)*zoom), (int)((x+radius)*zoom), (int)((y+radius)*zoom),
		(int)(pt1x*zoom), (int)(pt1y*zoom), (int)(pt2x*zoom), (int)(pt2y*zoom));
	}