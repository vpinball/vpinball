// license:GPLv3+

#pragma once

class PaintSur final : public Sur
{
public:
   PaintSur(const HDC hdc, const float zoom, const float offx, const float offy, const int width, const int height, ISelect * const psel);
   ~PaintSur() override;

   void Line(const float x, const float y, const float x2, const float y2) override;
   void Rectangle(const float x, const float y, const float x2, float y2) override;
   void Rectangle2(const int x, const int y, const int x2, const int y2) override;
   void Ellipse(const float centerx, const float centery, const float radius) override;
   void Ellipse2(const float centerx, const float centery, const int radius) override;
   void Polygon(const Vertex2D * const rgv, const int count) override;
   void Polygon(const vector<RenderVertex> &rgv) override;
   void PolygonImage(const vector<RenderVertex> &rgv, HBITMAP hbm, const float left, const float top, const float right, const float bottom, const int bitmapwidth, const int bitmapheight) override;
   void Polyline(const Vertex2D * const rgv, const int count) override;
   void Lines(const Vertex2D * const rgv, const int count) override;
   void Arc(const float x, const float y, const float radius, const float pt1x, const float pt1y, const float pt2x, const float pt2y) override;
   void Image(const float x, const float y, const float x2, const float y2, HDC hdcSrc, const int width, const int height) override;

   void SetObject(ISelect * const psel) override;

   void SetFillColor(const int rgb) override;
   void SetBorderColor(const int rgb, const bool dashed, const int width) override;
   void SetLineColor(const int rgb, const bool dashed, const int width) override;

private:
   HBRUSH m_hbr;
   HPEN m_hpnOutline;
   HPEN m_hpnLine;

   ISelect *m_psel;

   bool m_nullBorder;
};
