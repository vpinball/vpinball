#pragma once

class HitRectSur : public Sur
{
public:

   HitRectSur(const HDC hdc, const float zoom, const float offx, const float offy, const int width, const int height, FRect * const prcRect, Vector<ISelect> * const pvsel);
   virtual ~HitRectSur();

   virtual void Line(const float x, const float y, const float x2, const float y2);
   virtual void Rectangle(const float x, const float y, const float x2, float y2);
   virtual void Rectangle2(const int x, const int y, const int x2, const int y2);
   virtual void Ellipse(const float centerx, const float centery, const float radius);
   virtual void Ellipse2(const float centerx, const float centery, const int radius);
   virtual void Polygon(const Vertex2D * const rgv, const int count);
   virtual void Polygon(const std::vector<RenderVertex> &rgv);
   virtual void PolygonImage(const std::vector<RenderVertex> &rgv, HBITMAP hbm, const float left, const float top, const float right, const float bottom, const int bitmapwidth, const int bitmapheight);

   virtual void Polyline(const Vertex2D * const rgv, const int count);
   virtual void Lines(const Vertex2D * const rgv, const int count);
   virtual void Arc(const float x, const float y, const float radius, const float pt1x, const float pt1y, const float pt2x, const float pt2y);
   virtual void Image(const float x, const float y, const float x2, const float y2, HDC hdcSrc, const int width, const int height);

   virtual void SetObject(ISelect *psel);

   void FailObject();

   virtual void SetFillColor(const int rgb);
   virtual void SetBorderColor(const int rgb, const bool fDashed, const int width);
   virtual void SetLineColor(const int rgb, const bool fDashed, const int width);

private:
   ISelect *m_pcur;

   int m_indexcur;

   FRect m_rcRect;
   Vector<ISelect> *m_pvsel;
   Vector<ISelect> m_vselFailed;

   bool m_fFailedAlready; // Object has already been discounted from selection
};
