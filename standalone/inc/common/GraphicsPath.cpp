#include "stdafx.h"

#include "GraphicsPath.h"

#include <cmath>

GraphicsPath::GraphicsPath()
{
}

void GraphicsPath::AddPolygon(const vector<SDL_FPoint>* const pPoints)
{
   for (const auto &point : *pPoints)
      m_points.push_back(point);
}

void GraphicsPath::AddEllipse(float x, float y, float width, float height)
{
   constexpr int segments = 5;

   float a = width * 0.5f;
   float b = height * 0.5f;
   float angleStep = (float)(2.0 * M_PI / (double)segments);

   for (int i = 0; i < segments; ++i) {
      float angle = (float)i * angleStep;
      SDL_FPoint point;
      point.x = x + a * cosf(angle);
      point.y = y + b * sinf(angle);
      m_points.push_back(point);
   }
}

void GraphicsPath::AddEllipse(const SDL_FRect& rect)
{
   AddEllipse(rect.x, rect.y, rect.w, rect.h);
}

void GraphicsPath::AddClosedCurve(const std::vector<SDL_FPoint>* const pPoints, float tension)
{
   constexpr int segments = 5;
   constexpr float inv_segments = (float)(1.0/(double)segments);

   size_t count = pPoints->size();
   if (count < 2) return;

   for (size_t i = 0; i < count; ++i) {
      SDL_FPoint p0 = (i == 0) ? pPoints->back() : (*pPoints)[i - 1];
      SDL_FPoint p1 = (*pPoints)[i];
      SDL_FPoint p2 = (i+1 == count) ? pPoints->front() : (*pPoints)[i + 1];
      SDL_FPoint p3 = (*pPoints)[(i + 2) % count];

      for (int t = 0; t < segments; ++t) {
         float s = (float)t * inv_segments;

         float h1 =  2.f * (s*s*s) - 3.f * (s*s) + 1.f;
         float h2 = -2.f * (s*s*s) + 3.f * (s*s);
         float h3 = (s*s*s) - 2.f * (s*s) + s;
         float h4 = (s*s*s) - (s*s);

         SDL_FPoint t1 = { tension * (p2.x - p0.x), tension * (p2.y - p0.y) };
         SDL_FPoint t2 = { tension * (p3.x - p1.x), tension * (p3.y - p1.y) };

         SDL_FPoint point = {
             h1 * p1.x + h2 * p2.x + h3 * t1.x + h4 * t2.x,
             h1 * p1.y + h2 * p2.y + h3 * t1.y + h4 * t2.y
         };

         m_points.push_back(point);
      }
   }
}

void GraphicsPath::AddRectangle(const SDL_FRect& rect)
{
   if (rect.w == 0 || rect.h == 0)
      return;

   m_points.push_back({rect.x, rect.y});
   m_points.push_back({rect.x + rect.w, rect.y});
   m_points.push_back({rect.x + rect.w, rect.y + rect.h});
   m_points.push_back({rect.x, rect.y + rect.h});
}

void GraphicsPath::Transform(VP::Matrix* pMatrix)
{
   if (pMatrix)
      pMatrix->TransformPoints(m_points);
}

GraphicsPath* GraphicsPath::Clone() const
{
   GraphicsPath* pPath = new GraphicsPath();
   pPath->m_points.reserve(m_points.size());
   for (const auto &point : m_points)
      pPath->m_points.push_back(point);
   return pPath;
}
