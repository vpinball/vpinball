#include "StdAfx.h"

#define PTINRECT(x,y) ((x) >= m_rcRect.left && (x) <= m_rcRect.right && (y) > m_rcRect.top && (y) < m_rcRect.bottom)

HitRectSur::HitRectSur(const HDC hdc, const float zoom, const float offx, const float offy, const int width, const int height, FRect * const prcRect, Vector<ISelect> * const pvsel) : Sur(hdc, zoom, offx, offy, width, height)
{
   m_rcRect = *prcRect;
   m_pvsel = pvsel;

   if (m_rcRect.right < m_rcRect.left)
   {
      const float temp = m_rcRect.right;
      m_rcRect.right = m_rcRect.left;
      m_rcRect.left = temp;
   }

   if (m_rcRect.bottom < m_rcRect.top)
   {
      const float temp = m_rcRect.bottom;
      m_rcRect.bottom = m_rcRect.top;
      m_rcRect.top = temp;
   }

   SetObject(NULL);
}

HitRectSur::~HitRectSur()
{
}

void HitRectSur::Line(const float x, const float y, const float x2, const float y2)
{
   if (m_fFailedAlready)
   {
      return;
   }

   if (!PTINRECT(x, y) || !PTINRECT(x2, y2))
   {
      FailObject();
   }
}

void HitRectSur::Rectangle(const float x, const float y, const float x2, float y2)
{
   if (m_fFailedAlready)
   {
      return;
   }

   if (!PTINRECT(x, y) || !PTINRECT(x2, y2))
   {
      FailObject();
   }
}

void HitRectSur::Rectangle2(const int x, const int y, const int x2, const int y2)
{
}

void HitRectSur::Ellipse(const float centerx, const float centery, const float radius)
{
   if (m_fFailedAlready)
   {
      return;
   }

   if (!PTINRECT(centerx - radius, centery - radius) || !PTINRECT(centerx + radius, centery + radius))
   {
      FailObject();
   }
}

void HitRectSur::Ellipse2(const float centerx, const float centery, const int radius)
{
   if (m_fFailedAlready)
   {
      return;
   }

   const int ix = SCALEXf(centerx);
   const int iy = SCALEYf(centery);

   const int circleleft = ix - radius;
   const int circletop = iy - radius;
   const int circleright = ix + radius;
   const int circlebottom = iy + radius;

   const int left = SCALEXf(m_rcRect.left);
   const int top = SCALEYf(m_rcRect.top);
   const int right = SCALEXf(m_rcRect.right);
   const int bottom = SCALEYf(m_rcRect.bottom);

   if (circleleft < left || circletop < top || circleright > right || circlebottom > bottom)
   {
      FailObject();
   }
}

void HitRectSur::Polygon(const Vertex2D * const rgv, const int count)
{
   if (m_fFailedAlready)
   {
      return;
   }

   for (int i = 0; i < count; ++i)
   {
      if (!PTINRECT(rgv[i].x, rgv[i].y))
      {
         FailObject();
         return;
      }
   }
}
// copy-pasted from above
void HitRectSur::Polygon(const std::vector<RenderVertex> &rgv)
{
   if (m_fFailedAlready)
   {
      return;
   }

   for (unsigned i = 0; i < rgv.size(); ++i)
   {
      if (!PTINRECT(rgv[i].x, rgv[i].y))
      {
         FailObject();
         return;
      }
   }
}

void HitRectSur::PolygonImage(const std::vector<RenderVertex> &rgv, HBITMAP hbm, const float left, const float top, const float right, const float bottom, const int bitmapwidth, const int bitmapheight)
{
   Polygon(rgv);
}

void HitRectSur::Polyline(const Vertex2D * const rgv, const int count)
{
}

void HitRectSur::Lines(const Vertex2D * const rgv, const int count)
{
}

void HitRectSur::Arc(const float x, const float y, const float radius, const float pt1x, const float pt1y, const float pt2x, const float pt2y)
{
}

void HitRectSur::Image(const float x, const float y, const float x2, const float y2, HDC hdcSrc, const int width, const int height)
{
}

void HitRectSur::SetObject(ISelect *psel)
{
   m_pcur = psel;
   if (m_pcur)
   {
      if (m_vselFailed.IndexOf(psel) != -1)
      {
         // Object failed previously - just skip this time
         m_fFailedAlready = true;
      }
      else
      {
         const int index = m_pvsel->IndexOf(psel);
         if (index == -1)
         {
            // Object not in list yet - add it
            m_indexcur = m_pvsel->AddElement(psel);
         }
         else
         {
            m_indexcur = index;
         }
         m_fFailedAlready = false;
      }
   }
   else
   {
      m_fFailedAlready = true;
   }
}

void HitRectSur::FailObject()
{
   m_fFailedAlready = true;

   _ASSERTE(m_indexcur < m_pvsel->Size());

   m_pvsel->RemoveElementAt(m_indexcur); // perf?  Probably doesn't matter
   m_vselFailed.AddElement(m_pcur);
}

void HitRectSur::SetFillColor(const int rgb)
{
}

void HitRectSur::SetBorderColor(const int rgb, const bool fDashed, const int width)
{
}

void HitRectSur::SetLineColor(const int rgb, const bool fDashed, const int width)
{
}
