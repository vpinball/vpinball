#pragma once

void DrawLine(const HDC hdc, const float x1, const float y1, const float x2, const float y2);
void DrawCircleAbsolute(const HDC hdc, const float x, const float y, const float radius);
void DrawCircle(const HDC hdc, const float x, const float y, const float radius);
void DrawArc(const HDC hdc, const float x, const float y, const float radius, const float pt1x, const float pt1y, const float pt2x, const float pt2y);
