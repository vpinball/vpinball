#pragma once

void DrawLine(HDC hdc, float x1, float y1, float x2, float y2);
void DrawCircleAbsolute(HDC hdc, float x, float y, float radius);
void DrawCircle(HDC hdc, float x, float y, float radius);
void DrawArc(HDC hdc, float x, float y, float radius, float pt1x, float pt1y, float pt2x, float pt2y);