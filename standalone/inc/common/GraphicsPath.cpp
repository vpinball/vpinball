#include "stdafx.h"

#include "GraphicsPath.h"

#include <cmath>

GraphicsPath::GraphicsPath()
{
}

void GraphicsPath::AddPolygon(const vector<SDL_FPoint>* pPoints)
{
   for (auto point : *pPoints)
      m_points.push_back(point);
}

void GraphicsPath::AddEllipse(float x, float y, float width, float height)
{
   int segments = 5;

   float a = width * 0.5f;
   float b = height * 0.5f;
   float angleStep = 2.0f * M_PI / segments;

   for (int i = 0; i < segments; ++i) {
      float angle = i * angleStep;
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

void GraphicsPath::AddClosedCurve(const std::vector<SDL_FPoint>* pPoints, float tension)
{
   int segments = 5;

   size_t count = pPoints->size();
   if (count < 2) return;

   for (size_t i = 0; i < count; ++i) {
      SDL_FPoint p0 = (i == 0) ? pPoints->back() : (*pPoints)[i - 1];
      SDL_FPoint p1 = (*pPoints)[i];
      SDL_FPoint p2 = (*pPoints)[(i + 1) % count];
      SDL_FPoint p3 = (*pPoints)[(i + 2) % count];

      for (int t = 0; t < segments; ++t) {
         float s = (float)t / (float)(segments);

         float h1 =  2 * std::pow(s, 3) - 3 * std::pow(s, 2) + 1;
         float h2 = -2 * std::pow(s, 3) + 3 * std::pow(s, 2);
         float h3 = std::pow(s, 3) - 2 * std::pow(s, 2) + s;
         float h4 = std::pow(s, 3) - std::pow(s, 2);

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
   for (auto point : m_points)
      pPath->m_points.push_back(point);
   return pPath;
}