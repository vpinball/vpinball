#pragma once
class HitRectSur : public Sur
	{
public:

	HitRectSur(const HDC hdc, const float zoom, const float offx, const float offy, const int width, const int height, FRect * const prcRect, Vector<ISelect> * const pvsel);
	virtual ~HitRectSur();

	virtual void Line(float x, float y, float x2, float y2);
	virtual void Rectangle(float x, float y, float x2, float y2);
	virtual void Rectangle2(int x, int y, int x2, int y2);
	virtual void Ellipse(float centerx, float centery, float radius);
	virtual void Ellipse2(float centerx, float centery, int radius);
	virtual void Polygon(Vertex2D *rgv, int count);
	virtual void PolygonImage(Vertex2D *rgv, int count, HBITMAP hbm, float left, float top, float right, float bottom, int bitmapwidth, int bitmapheight);
	virtual void Polyline(Vertex2D *rgv, int count);
	virtual void Arc(float x, float y, float radius, float pt1x, float pt1y, float pt2x, float pt2y);
	virtual void Image(float x, float y, float x2, float y2, HDC hdcSrc, int width, int height);

	virtual void SetObject(ISelect *psel);

	void FailObject();

	virtual void SetFillColor(int rgb);
	virtual void SetBorderColor(int rgb, BOOL fDashed, int width);
	virtual void SetLineColor(int rgb, BOOL fDashed, int width);

	ISelect *m_pcur;

	bool m_fFailedAlready; // Object has already been discounted from selection
	int m_indexcur;

	FRect m_rcRect;
	Vector<ISelect> *m_pvsel;
	Vector<ISelect> m_vselFailed;	
	};
