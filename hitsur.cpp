#include "StdAfx.h"

HitSur::HitSur(const HDC hdc, const float zoom, const float offx, const float offy, const int width, const int height, const int hitx, const int hity, ISelect * const pbackground) : Sur(hdc, zoom, offx, offy, width, height)
{
   m_pselected = pbackground;

   m_hitx = hitx;
   m_hity = hity;

   SetObject(NULL);
}

HitSur::~HitSur()
{
}

void HitSur::Line(const float x, const float y, const float x2, const float y2)
{
   if (m_pcur == NULL)
      return;

   const int x_1 = SCALEXf(x);
   const int y_1 = SCALEYf(y);
   const int x_2 = SCALEXf(x2);
   const int y_2 = SCALEYf(y2);

   if (abs(x_2 - x_1) > abs(y_2 - y_1))
   {
      int lineY = m_hity + 4;
      if (x_1 > x_2)
      {
         if (m_hitx >= x_2 && m_hitx <= x_1)
            lineY = ((y_1 - y_2)*(m_hitx - x_2)) / (x_1 - x_2) + y_2;
      }
      else
         if (m_hitx >= x_1 && m_hitx <= x_2)
            lineY = ((y_2 - y_1)*(m_hitx - x_1)) / (x_2 - x_1) + y_1;

      if (m_hity + 4 > lineY && m_hity < lineY + 4)
      {
         m_pselected = m_pcur;
         return;
      }
   }
   else if (abs(x_2 - x_1) < abs(y_2 - y_1))
   {

      int lineX = m_hitx + 4;
      if (y_1 > y_2)
      {
         if (m_hity >= y_2 && m_hity <= y_1)
            lineX = ((x_1 - x_2)*(m_hity - y_2)) / (y_1 - y_2) + x_2;
      }
      else
         if (m_hity >= y_1 && m_hity <= y_2)
            lineX = ((x_2 - x_1)*(m_hity - y_1)) / (y_2 - y_1) + x_1;

      if (m_hitx + 4 > lineX && m_hitx < lineX + 4)
      {
         m_pselected = m_pcur;
         return;
      }
   }
}

void HitSur::Rectangle(const float x, const float y, const float x2, float y2)
{
   if (m_pcur == NULL)
      return;

   int ix = SCALEXf(x);
   int iy = SCALEYf(y);
   int ix2 = SCALEXf(x2);
   int iy2 = SCALEYf(y2);

   if (ix > ix2)
   {
      const int temp = ix;
      ix = ix2;
      ix2 = temp;
   }

   if (iy > iy2)
   {
      const int temp = iy;
      iy = iy2;
      iy2 = temp;
   }

   if (m_hitx >= ix && m_hitx <= ix2 && m_hity >= iy && m_hity <= iy2)
   {
      m_pselected = m_pcur;
   }
}

void HitSur::Rectangle2(const int x, const int y, const int x2, const int y2)
{
   if (m_pcur == NULL)
   {
      return;
   }
}

void HitSur::Ellipse(const float centerx, const float centery, const float radius)
{
   if (m_pcur == NULL)
      return;

   const int ix = SCALEXf(centerx);
   const int iy = SCALEYf(centery);
   const long long ir = SCALEDf(radius);

   const long long dx = m_hitx - ix;
   const long long dy = m_hity - iy;
   const long long dist = dx*dx + dy*dy;

   if (dist <= ir*ir)
   {
      m_pselected = m_pcur;
   }
}

void HitSur::Ellipse2(const float centerx, const float centery, const int radius)
{
   if (m_pcur == NULL)
   {
      return;
   }

   const int ix = SCALEXf(centerx);
   const int iy = SCALEYf(centery);

   const long long dx = m_hitx - ix;
   const long long dy = m_hity - iy;
   const long long dist = dx*dx + dy*dy;

   if (dist <= radius*radius)
   {
      m_pselected = m_pcur;
   }
}

void HitSur::Polygon(const Vertex2D * const rgv, const int count)
{
   if (m_pcur == NULL)
      return;

   int x1 = SCALEXf(rgv[count - 1].x);
   int y1 = SCALEYf(rgv[count - 1].y);
   bool hx1 = (m_hitx >= x1);
   bool hy1 = (m_hity > y1);
   int crosscount = 0;	// count of lines which the hit point is to the left of
   for (int i = 0; i < count; ++i)
   {
      const int x2 = x1;
      const int y2 = y1;
      const bool hx2 = hx1;
      const bool hy2 = hy1;

      x1 = SCALEXf(rgv[i].x);
      y1 = SCALEYf(rgv[i].y);
      hx1 = (m_hitx >= x1);
      hy1 = (m_hity > y1);

      if ((y1 == y2) ||
         (!hy1 && !hy2) || (hy1 && hy2) || // if out of y range, forget about this segment
         (hx1 && hx2)) // Hit point is on the right of the line
         continue;

      if (!hx1 && !hx2)
      {
         crosscount ^= 1;
         continue;
      }

      if (x2 == x1)
      {
         if (!hx2)
            crosscount ^= 1;
         continue;
      }

      // Now the hard part - the hit point is in the line bounding box
      if (x2 - (y2 - m_hity)*(x1 - x2) / (y1 - y2) > m_hitx)
         crosscount ^= 1;
   }

   if (crosscount & 1)
   {
      m_pselected = m_pcur;
   }
}
// copy-pasted from above
void HitSur::Polygon(const std::vector<RenderVertex> &rgv)
{
   if (m_pcur == NULL)
      return;

   int x1 = SCALEXf(rgv[rgv.size() - 1].x);
   int y1 = SCALEYf(rgv[rgv.size() - 1].y);
   bool hx1 = (m_hitx >= x1);
   bool hy1 = (m_hity > y1);
   int crosscount = 0;	// count of lines which the hit point is to the left of
   for (unsigned i = 0; i < rgv.size(); ++i)
   {
      const int x2 = x1;
      const int y2 = y1;
      const bool hx2 = hx1;
      const bool hy2 = hy1;

      x1 = SCALEXf(rgv[i].x);
      y1 = SCALEYf(rgv[i].y);
      hx1 = (m_hitx >= x1);
      hy1 = (m_hity > y1);

      if ((y1 == y2) ||
         (!hy1 && !hy2) || (hy1 && hy2) || // if out of y range, forget about this segment
         (hx1 && hx2)) // Hit point is on the right of the line
         continue;

      if (!hx1 && !hx2)
      {
         crosscount ^= 1;
         continue;
      }

      if (x2 == x1)
      {
         if (!hx2)
            crosscount ^= 1;
         continue;
      }

      // Now the hard part - the hit point is in the line bounding box
      if (x2 - (y2 - m_hity)*(x1 - x2) / (y1 - y2) > m_hitx)
         crosscount ^= 1;
   }

   if (crosscount & 1)
   {
      m_pselected = m_pcur;
   }
}

void HitSur::PolygonImage(const std::vector<RenderVertex> &rgv, HBITMAP hbm, const float left, const float top, const float right, const float bottom, const int bitmapwidth, const int bitmapheight)
{
   Polygon(rgv);
}

void HitSur::Polyline(const Vertex2D * const rgv, const int count)
{
   if (m_pcur == NULL)
      return;

   for (int i = 0; i < count - 1; ++i)
      Line(rgv[i].x, rgv[i].y, rgv[i + 1].x, rgv[i + 1].y);
}

void HitSur::Lines(const Vertex2D * const rgv, const int count)
{
   if (m_pcur == NULL)
      return;

   for (int i = 0; i < count * 2; i += 2)
      Line(rgv[i].x, rgv[i].y, rgv[i + 1].x, rgv[i + 1].y);
}

void HitSur::Arc(const float x, const float y, const float radius, const float pt1x, const float pt1y, const float pt2x, const float pt2y)
{
   //Ellipse(x, y, radius);
}

void HitSur::Image(const float x, const float y, const float x2, const float y2, HDC hdcSrc, const int width, const int height)
{

}

void HitSur::SetObject(ISelect *psel)
{
   m_pcur = psel;
}

void HitSur::SetFillColor(const int rgb)
{
}

void HitSur::SetBorderColor(const int rgb, const bool fDashed, const int width)
{
}

void HitSur::SetLineColor(const int rgb, const bool fDashed, const int width)
{
}
