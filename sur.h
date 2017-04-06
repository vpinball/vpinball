#pragma once

// Much faster float-to-int conversion than standard C cast.
// Source: http://stereopsis.com/sree/fpu2006.html
/*__forceinline int RoundToInt(double val)
{
static const double _xs_doublemagic = 6755399441055744.0;   //2^52 * 1.5, uses limited precision to floor
val = val + _xs_doublemagic;
return ((int*)&val)[0];
}*/

#if _MSC_VER >= 1800
#define RoundToInt lround
#else
__forceinline int RoundToInt(const float val)
{
   return (int)(val + ((val < 0.0f) ? -0.5f : 0.5f));
}
#endif

#define SCALEXf(x) RoundToInt(((x) - m_offx)*m_zoom)
#define SCALEYf(y) RoundToInt(((y) - m_offy)*m_zoom)
#define SCALEDf(d) RoundToInt((d)*m_zoom)

class Sur
{
public:

   inline Sur(const HDC hdc, const float zoom, const float offx, const float offy, const int width, const int height)
   {
      m_hdc = hdc;
      m_zoom = zoom;
      const float inv_zoom = 0.5f / zoom;
      m_offx = offx - (float)width*inv_zoom;
      m_offy = offy - (float)height*inv_zoom;
   }
   virtual ~Sur() {}

   virtual void Line(const float x, const float y, const float x2, const float y2) = 0;
   virtual void Rectangle(const float x, const float y, const float x2, float y2) = 0;
   virtual void Rectangle2(const int x, const int y, const int x2, const int y2) = 0;
   virtual void Ellipse(const float centerx, const float centery, const float radius) = 0;
   virtual void Ellipse2(const float centerx, const float centery, const int radius) = 0;
   virtual void Polygon(const Vertex2D * const rgv, const int count) = 0;
   virtual void Polygon(const std::vector<RenderVertex> &rgv) = 0;
   virtual void PolygonImage(const std::vector<RenderVertex> &rgv, HBITMAP hbm, const float left, const float top, const float right, const float bottom, const int bitmapwidth, const int bitmapheight) = 0;
   virtual void Polyline(const Vertex2D * const rgv, const int count) = 0;
   virtual void Lines(const Vertex2D * const rgv, const int count) = 0;
   virtual void Arc(const float x, const float y, const float radius, const float pt1x, const float pt1y, const float pt2x, const float pt2y) = 0;
   virtual void Image(const float x, const float y, const float x2, const float y2, HDC hdcSrc, const int width, const int height) = 0;

   virtual void SetObject(ISelect *psel) = 0;

   virtual void SetFillColor(const int rgb) = 0;
   virtual void SetBorderColor(const int rgb, const bool fDashed, const int width) = 0;
   virtual void SetLineColor(const int rgb, const bool fDashed, const int width) = 0;

   inline Vertex2D ScreenToSurface(const int screenx, const int screeny) const
   {
      const float inv_zoom = 1.0f / m_zoom;
      Vertex2D result;
      result.x = (float)screenx*inv_zoom + m_offx;
      result.y = (float)screeny*inv_zoom + m_offy;
      return result;
   }

   HDC m_hdc;
   float m_zoom;
   float m_offx, m_offy;
};
