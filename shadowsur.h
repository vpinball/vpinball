class ShadowSur : public Sur
	{
public:

	ShadowSur(HDC hdc, float zoom, float offx, float offy, int width, int height, float z, ISelect *psel);
	virtual ~ShadowSur();

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
	
	void EllipseSkew(float centerx, float centery, float radius, float z1, float z2);
	void PolygonSkew(Vertex *rgv, int count, float *rgz, float z1, float z2, BOOL fPreClip);
	void PolylineSkew(Vertex *rgv, int count, float *rgz, float z1, float z2);

	virtual void SetObject(ISelect *psel);

	virtual void SetFillColor(int rgb);
	virtual void SetBorderColor(int rgb, BOOL fDashed, int width);
	virtual void SetLineColor(int rgb, BOOL fDashed, int width);

	HPEN m_hpnLine;

	ISelect *m_psel;

	float m_z; //height of this shadow
	};