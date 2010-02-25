#pragma once
#define SCALEX(x) ((int)((x - m_offx)*m_zoom + 0.5))
#define SCALEY(y) ((int)((y - m_offy)*m_zoom + 0.5))
#define SCALED(d) ((int)(d*m_zoom + 0.5))

class Sur
	{
public:

	Sur(HDC hdc, float zoom, float offx, float offy, int width, int height);
	virtual ~Sur();

	virtual void Line(float x, float y, float x2, float y2) = 0;
	virtual void Rectangle(float x, float y, float x2, float y2) = 0;
	virtual void Rectangle2(int x, int y, int x2, int y2) = 0;
	virtual void Ellipse(float centerx, float centery, float radius) = 0;
	virtual void Ellipse2(float centerx, float centery, int radius) = 0;
	virtual void Polygon(Vertex *rgv, int count) = 0;
	virtual void PolygonImage(Vertex *rgv, int count, HBITMAP hbm, float left, float top, float right, float bottom, int bitmapwidth, int bitmapheight) = 0;
	virtual void Polyline(Vertex *rgv, int count) = 0;
	virtual void Arc(float x, float y, float radius, float pt1x, float pt1y, float pt2x, float pt2y) = 0;
	virtual void Image(float x, float y, float x2, float y2, HDC hdcSrc, int width, int height) = 0;

	virtual void SetObject(ISelect *psel) = 0;

	virtual void SetFillColor(int rgb) = 0;
	virtual void SetBorderColor(int rgb, BOOL fDashed, int width) = 0;
	virtual void SetLineColor(int rgb, BOOL fDashed, int width) = 0;

	void ScreenToSurface(int screenx, int screeny, float *fx, float *fy);
	void SurfaceToScreen(float fx, float fy, int *screenx, int *screeny);

	HDC m_hdc;
	float m_zoom;
	float m_offx, m_offy;
	};