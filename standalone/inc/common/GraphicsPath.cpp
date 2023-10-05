/*
 * Portions of this code was derived from Mono:
 *
 * https://github.com/mono/sysdrawing-coregraphics/blob/main/System.Drawing.Drawing2D/GraphicsPath.cs
 * https://github.com/mono/sysdrawing-coregraphics/blob/main/Utilities/GeomUtilities.cs
 */

#include "stdafx.h"

#include "GraphicsPath.h"
#include <cmath>

GraphicsPath::GraphicsPath()
{
   m_startNewFig = true;
   m_isReverseWindingOnFill = false;
}

void GraphicsPath::Append(float x, float y, PathPointType type, bool compress)
{
   PathPointType t = type;
   SDL_FPoint pt = { 0.0f, 0.0f };

   if (compress && (m_points.size() > 0)) {
      SDL_FPoint lastPoint = m_points[m_points.size()-1];
      if ((lastPoint.x == x) && (lastPoint.y == y)) {
         PathPointType last_type = (PathPointType) m_types[m_types.size()-1];
         if ((last_type & PathPointType_CloseSubpath) != PathPointType_CloseSubpath)
            return;
      }
   }

   if (m_startNewFig)
      t = PathPointType_Start;
   else if (m_points.size() > 0) {
      type = (PathPointType)m_types[m_types.size()-1];
      if ((type & PathPointType_CloseSubpath) != 0)
         t = PathPointType_Start;
   }

   pt.x = x;
   pt.y = y;

   m_points.push_back(pt);
   m_types.push_back(t);
   m_startNewFig = false;
}

void GraphicsPath::AppendBezier(float x1, float y1, float x2, float y2, float x3, float y3)
{
   if (m_isReverseWindingOnFill) {
      Append(y1, x1, PathPointType_Bezier3, false);
      Append(y2, x2, PathPointType_Bezier3, false);
      Append(y3, x3, PathPointType_Bezier3, false);
   }
   else
   {
      Append(x1, y1, PathPointType_Bezier3, false);
      Append(x2, y2, PathPointType_Bezier3, false);
      Append(x3, y3, PathPointType_Bezier3, false);
   }
}

void GraphicsPath::AppendPoint(SDL_FPoint point, PathPointType type, bool compress)
{
   Append(point.x, point.y, type, compress);
}

void GraphicsPath::AppendCurve(vector<SDL_FPoint> points, vector<SDL_FPoint> tangents, int offset, int length, CurveType type)
{
   PathPointType ptype = ((type == CurveType_Close) || (points.size() == 0)) ? PathPointType_Start : PathPointType_Line;
   int i;

   AppendPoint(points[offset], ptype, true);
   for (i = offset; i < offset + length; i++) {
      int j = i + 1;

      float x1 = points[i].x + tangents[i].x;
      float y1 = points[i].y + tangents[i].y;

      float x2 = points[j].x - tangents[j].x;
      float y2 = points[j].y - tangents[j].y;
      float x3 = points[j].x;
      float y3 = points[j].y;

      AppendBezier(x1, y1, x2, y2, x3, y3);
   }

   if (type == CurveType_Close) {
      float x1 = points[i].x + tangents[i].x;
      float y1 = points[i].y + tangents[i].y;

      float x2 = points[0].x - tangents[0].x;
      float y2 = points[0].y - tangents[0].y;

      float x3 = points[0].x;
      float y3 = points[0].y;

      AppendBezier(x1, y1, x2, y2, x3, y3);
      CloseFigure ();
   }
}

void GraphicsPath::CloseFigure()
{
   if (m_points.size() > 0)
      m_types[m_types.size()-1] = (PathPointType)(m_types[m_types.size()-1] | PathPointType_CloseSubpath);
      m_startNewFig = true;
}

void GraphicsPath::AddLine(float x1, float y1, float x2, float y2)
{
   Append(x1, y1, PathPointType_Line, true);
   Append(x2, y2, PathPointType_Line, false);
}

void GraphicsPath::AddEllipse(const SDL_FRect& rect)
{
   const float C1 = 0.552285f;
   float rx = rect.w / 2;
   float ry = rect.h / 2;
   float cx = rect.x + rx;
   float cy = rect.y + ry;

   if (!m_isReverseWindingOnFill) {
      Append(cx + rx, cy, PathPointType_Start, false);
      AppendBezier(cx + rx, cy - C1 * ry, cx + C1 * rx, cy - ry, cx, cy - ry);
      AppendBezier(cx - C1 * rx, cy - ry, cx - rx, cy - C1 * ry, cx - rx, cy);
      AppendBezier(cx - rx, cy + C1 * ry, cx - C1 * rx, cy + ry, cx, cy + ry);
      AppendBezier(cx + C1 * rx, cy + ry, cx + rx, cy + C1 * ry, cx + rx, cy);
   }
   else {
      Append(cx + rx, cy, PathPointType_Start, false);
      AppendBezier(cx + C1 * rx, cy + ry, cx + rx, cy + C1 * ry, cx + rx, cy);
      AppendBezier(cx + rx, cy - C1 * ry, cx + C1 * rx, cy - ry, cx, cy - ry);
      AppendBezier(cx - C1 * rx, cy - ry, cx - rx, cy - C1 * ry, cx - rx, cy);
      AppendBezier(cx - rx, cy + C1 * ry, cx - C1 * rx, cy + ry, cx, cy + ry);
   }

   CloseFigure();
}

void GraphicsPath::AddEllipse(float x, float y, float width, float height)
{
   AddEllipse({ x, y, width, height });
}

void GraphicsPath::AddClosedCurve(const vector<SDL_FPoint>& points)
{
   AddClosedCurve(points, 0.5f);
}

void GraphicsPath::AddClosedCurve(const vector<SDL_FPoint>& points, float tension)
{
   if (points.size() < 3)
      return;

   vector<SDL_FPoint> tangents = GetCurveTangents(1, points, points.size(), tension, CurveType_Close);
   AppendCurve(points, tangents, 0, points.size() - 1, CurveType_Close);
}

void GraphicsPath::AddPolygon(const vector<SDL_FPoint>& points)
{
   if (points.size() < 3)
      return;

   AppendPoint(points[0], PathPointType_Start, false);
   for (int i = 1; i < points.size(); i++)
      AppendPoint(points[i], PathPointType_Line, false);
   SDL_FPoint last = points[points.size() - 1];
   if (last.x != points[0].x || last.y != points[0].y)
      AppendPoint(points[0], PathPointType_Line, false);
   CloseFigure();
}

void GraphicsPath::AddRectangle(const SDL_FRect& rect)
{
   if (rect.w == 0 || rect.h == 0)
      return;

   Append(rect.x, rect.y, PathPointType_Start, false);
   Append(rect.x + rect.w, rect.y, PathPointType_Line, false);
   Append(rect.x + rect.w, rect.y + rect.h, PathPointType_Line, false);
   Append(rect.x, rect.y + rect.h, (PathPointType)(PathPointType_Line | PathPointType_CloseSubpath), false);
}

SDL_FRect GraphicsPath::GetBounds(VP::Matrix* pMatrix) const
{
   SDL_FRect bounds = { 0.0f, 0.0f, 0.0f, 0.0f };

   if (m_points.size() < 1)
      return bounds;

   GraphicsPath* pWorkPath = Clone();

   int status = FlattenPath(pWorkPath, pMatrix, 25.0f);

   if (status == 0) {
      int i;
      SDL_FPoint boundaryPoints;

      boundaryPoints = pWorkPath->m_points[0];
      bounds.x = boundaryPoints.x;
      bounds.y = boundaryPoints.y;
      if (pWorkPath->m_points.size() == 1) {
         bounds.w = 0.0f;
         bounds.h = 0.0f;
         return bounds;
      }

      bounds.w = boundaryPoints.x;
      bounds.h = boundaryPoints.y;

      for (i = 1; i < pWorkPath->m_points.size(); i++) {
         boundaryPoints = pWorkPath->m_points[i];
         if (boundaryPoints.x < bounds.x)
            bounds.x = boundaryPoints.x;
         if (boundaryPoints.y < bounds.y)
            bounds.y = boundaryPoints.y;
         if (boundaryPoints.x > bounds.w)
            bounds.w = boundaryPoints.x;
         if (boundaryPoints.y > bounds.h)
            bounds.h = boundaryPoints.y;
      }

      bounds.w -= bounds.x;
      bounds.h -= bounds.y;
   }

   return bounds;
}

void GraphicsPath::Transform(VP::Matrix* pMatrix)
{
   if (pMatrix)
      pMatrix->TransformPoints(m_points);
}

GraphicsPath* GraphicsPath::Clone() const
{
   GraphicsPath* pPath = new GraphicsPath();
   for (int i = 0; i < m_points.size(); i++) {
      pPath->m_points.push_back(m_points[i]);
      pPath->m_types.push_back(m_types[i]);
      pPath->m_startNewFig = m_startNewFig;
      pPath->m_isReverseWindingOnFill = m_isReverseWindingOnFill;
   }
   return pPath;
}

bool GraphicsPath::ConvertBezierToLines(GraphicsPath* pPath, int index, float flatness, vector<SDL_FPoint> flat_points, vector<PathPointType> flat_types)
{
   SDL_FPoint pt;

   PathPointType type = PathPointType_Line;

   if ((index <= 0) || (index + 2 >= pPath->m_points.size()))
      return false;

   SDL_FPoint start = pPath->m_points[index - 1];
   SDL_FPoint first = pPath->m_points[index];
   SDL_FPoint second = pPath->m_points[index + 1];
   SDL_FPoint end = pPath->m_points[index + 2];

   vector<SDL_FPoint> points;
   if (!NRCurveFlatten(start.x, start.y, first.x, first.y, second.x, second.y, end.x, end.y, flatness, 0, points))
      return false;

   if (points.size() > 0) {
      flat_points.push_back(points[0]);
      flat_types.push_back(type);
   }

   for (int i = 1; i < points.size(); i++) {
      pt = points[i];
      flat_points.push_back(pt);
      flat_types.push_back(type);
   }

   return true;
}

bool GraphicsPath::NRCurveFlatten(float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3, float flatness, int level, vector<SDL_FPoint> points)
{
   float dx1_0, dy1_0, dx2_0, dy2_0, dx3_0, dy3_0, dx2_3, dy2_3, d3_0_2;
   float s1_q, t1_q, s2_q, t2_q, v2_q;
   float f2, f2_q;
   float x00t, y00t, x0tt, y0tt, xttt, yttt, x1tt, y1tt, x11t, y11t;

   dx1_0 = x1 - x0;
   dy1_0 = y1 - y0;
   dx2_0 = x2 - x0;
   dy2_0 = y2 - y0;
   dx3_0 = x3 - x0;
   dy3_0 = y3 - y0;
   dx2_3 = x3 - x2;
   dy2_3 = y3 - y2;
   f2 = flatness;
   d3_0_2 = dx3_0 * dx3_0 + dy3_0 * dy3_0;
   if (d3_0_2 < f2) {
      float d1_0_2, d2_0_2;
      d1_0_2 = dx1_0 * dx1_0 + dy1_0 * dy1_0;
      d2_0_2 = dx2_0 * dx2_0 + dy2_0 * dy2_0;
      if ((d1_0_2 < f2) && (d2_0_2 < f2)) {
               goto nosubdivide;
      }
      else {
               goto subdivide;
      }
   }
   f2_q = f2 * d3_0_2;
   s1_q = dx1_0 * dx3_0 + dy1_0 * dy3_0;
   t1_q = dy1_0 * dx3_0 - dx1_0 * dy3_0;
   s2_q = dx2_0 * dx3_0 + dy2_0 * dy3_0;
   t2_q = dy2_0 * dx3_0 - dx2_0 * dy3_0;
   v2_q = dx2_3 * dx3_0 + dy2_3 * dy3_0;
   if ((t1_q * t1_q) > f2_q) goto subdivide;
   if ((t2_q * t2_q) > f2_q) goto subdivide;
   if ((s1_q < 0.0) && ((s1_q * s1_q) > f2_q)) goto subdivide;
   if ((v2_q < 0.0) && ((v2_q * v2_q) > f2_q)) goto subdivide;
   if (s1_q >= s2_q) goto subdivide;

   nosubdivide:
   {
      points.push_back({x3,y3});
      return true;
   }
   subdivide:
      if (level >= 10)
         return false;

      x00t = (x0 + x1) * 0.5f;
      y00t = (y0 + y1) * 0.5f;
      x0tt = (x0 + 2 * x1 + x2) * 0.25f;
      y0tt = (y0 + 2 * y1 + y2) * 0.25f;
      x1tt = (x1 + 2 * x2 + x3) * 0.25f;
      y1tt = (y1 + 2 * y2 + y3) * 0.25f;
      x11t = (x2 + x3) * 0.5f;
      y11t = (y2 + y3) * 0.5f;
      xttt = (x0tt + x1tt) * 0.5f;
      yttt = (y0tt + y1tt) * 0.5f;

      if(!NRCurveFlatten(x0, y0, x00t, y00t, x0tt, y0tt, xttt, yttt, flatness, level+1, points)) return false;
      if(!NRCurveFlatten(xttt, yttt, x1tt, y1tt, x11t, y11t, x3, y3, flatness, level+1, points)) return false;

      return true;
}

bool GraphicsPath::PathHasCurve(GraphicsPath* pPath)
{
   if (!pPath)
      return false;

   vector<PathPointType> types = pPath->m_types;
   for (int i = 0; i < types.size(); i++) {
      if (types[i] == PathPointType_Bezier)
         return true;
   }

   return false;
}

int GraphicsPath::FlattenPath(GraphicsPath* pPath, VP::Matrix* pMatrix, float flatness)
{
   int status = 0;

   if (!pPath)
      return -1;

   if (pMatrix)
      pPath->Transform(pMatrix);

   if (!PathHasCurve(pPath))
      return status;

   vector<SDL_FPoint> points;
   vector<PathPointType> types;

   for (int i = 0; i < pPath->m_points.size(); i++) {
      SDL_FPoint point = pPath->m_points[i];
      PathPointType type = pPath->m_types[i];

      if ((type & PathPointType_Bezier) == PathPointType_Bezier) {
         if (!ConvertBezierToLines(pPath, i, abs(flatness), points, types)) {
            SDL_FPoint pt = { 0.0f, 0.0f };

            points.clear();
            types.clear();

            type = PathPointType_Start;
            points.push_back(pt);
            types.push_back(type);

            type = PathPointType_Line;
            points.push_back(pt);
            types.push_back(type);

            points.push_back(pt);
            types.push_back(type);

            break;
         }
         i += 2;
      }
      else {
         points.push_back(point);
         types.push_back(type);
      }
   }
   pPath->m_points = points;
   pPath->m_types = types;

   return 0;
}

vector<SDL_FPoint> GraphicsPath::GetCurveTangents(int terms, vector<SDL_FPoint> points, int count, float tension, CurveType type)
{
   float coefficient = tension / 3.0f;
   vector<SDL_FPoint> tangents;

   if (count <= 2)
      return tangents;

   for (int i = 0; i < count; i++) {
      int r = i + 1;
      int s = i - 1;

      if (r >= count)
         r = count - 1;

      if (type == CurveType_Open) {
         if (s < 0)
            s = 0;
      }
      else {
         if (s < 0)
            s += count;
      }

      tangents.push_back( { 0.0f, 0.0f } );
      tangents[i].x += (coefficient * (points[r].x - points[s].x));
      tangents[i].y += (coefficient * (points[r].y - points[s].y));
   }

   return tangents;
}