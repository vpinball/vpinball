class PaintSur : public Sur
	{
public:

	PaintSur(HDC hdc, float zoom, float offx, float offy, int width, int height, ISelect *psel);
	virtual ~PaintSur();

	virtual void Line(float x, float y, float x2, float y2);
	virtual void Rectangle(float x, float y, float x2, float y2);
	virtual void Rectangle2(int x, int y, int x2, int y2);
	virtual void Ellipse(float centerx, float centery, float radius);
	virtual void Ellipse2(float centerx, float centery, int radius);
	virtual void Polygon(Vertex *rgv, int count);
	virtual void PolygonImage(Vertex *rgv, int count, HBITMAP hbm, float left, float top, float right, float bottom, int bitmapwidth, int bitmapheight);
	virtual void Polyline(Vertex *rgv, int count);
	virtual void Arc(float x, float y, float radius, float pt1x, float pt1y, float pt2x, float pt2y);
	virtual void Image(float x, float y, float x2, float y2, HDC hdcSrc, int width, int height);

	virtual void SetObject(ISelect *psel);

	virtual void SetFillColor(int rgb);
	virtual void SetBorderColor(int rgb, BOOL fDashed, int width);
	virtual void SetLineColor(int rgb, BOOL fDashed, int width);

	HBRUSH m_hbr;
	HPEN m_hpnOutline;
	HPEN m_hpnLine;

	BOOL m_fNullBorder;

	ISelect *m_psel;
	};