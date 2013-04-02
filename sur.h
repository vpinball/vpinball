#pragma once

#define SCALEXf(x) ((int)(((x) - m_offx)*m_zoom + 0.5f))
#define SCALEYf(y) ((int)(((y) - m_offy)*m_zoom + 0.5f))
#define SCALEDf(d) ((int)((d)*m_zoom + 0.5f))

class Sur
	{
public:

	inline Sur(const HDC hdc, const float zoom, const float offx, const float offy, const int width, const int height)
	{
		m_hdc = hdc;
		m_zoom = zoom;
		const float inv_zoom = 0.5f/zoom;
		m_offx = offx - (float)width*inv_zoom;
		m_offy = offy - (float)height*inv_zoom;
	}
	virtual ~Sur();

	virtual void Line(const float x, const float y, const float x2, const float y2) = 0;
	virtual void Rectangle(const float x, const float y, const float x2, float y2) = 0;
	virtual void Rectangle2(const int x, const int y, const int x2, const int y2) = 0;
	virtual void Ellipse(const float centerx, const float centery, const float radius) = 0;
	virtual void Ellipse2(const float centerx, const float centery, const int radius) = 0;
	virtual void EllipseImage(const float centerx, const float centery, const float radius, HBITMAP hbm, const float left, const float top, const float right, const float bottom, const int bitmapwidth, const int bitmapheight) = 0;
	virtual void Polygon(const Vertex2D * const rgv, const int count) = 0;
	virtual void Polygon(const Vector<RenderVertex> &rgv) = 0;
	virtual void PolygonImage(const Vector<RenderVertex> &rgv, HBITMAP hbm, const float left, const float top, const float right, const float bottom, const int bitmapwidth, const int bitmapheight) = 0;
	virtual void Polyline(const Vertex2D * const rgv, const int count) = 0;
	virtual void Arc(const float x, const float y, const float radius, const float pt1x, const float pt1y, const float pt2x, const float pt2y) = 0;
	virtual void Image(const float x, const float y, const float x2, const float y2, HDC hdcSrc, const int width, const int height) = 0;

	virtual void SetObject(ISelect *psel) = 0;

	virtual void SetFillColor(const int rgb) = 0;
	virtual void SetBorderColor(const int rgb, const bool fDashed, const int width) = 0;
	virtual void SetLineColor(const int rgb, const bool fDashed, const int width) = 0;

	inline void ScreenToSurface(const int screenx, const int screeny, float * const fx, float * const fy) const
	{
		const float inv_zoom = 1.0f/m_zoom;
		*fx = (float)screenx*inv_zoom + m_offx;
		*fy = (float)screeny*inv_zoom + m_offy;
	}

	inline void SurfaceToScreen(const float fx, const float fy, int * const screenx, int * const screeny) const
	{
		*screenx = SCALEXf(fx);
		*screeny = SCALEYf(fy);
	}

	HDC m_hdc;
	float m_zoom;
	float m_offx, m_offy;
	};
