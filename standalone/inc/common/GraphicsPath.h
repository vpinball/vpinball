#pragma once

/*
 * Portions of this code was derived from Mono:
 *
 * https://github.com/mono/sysdrawing-coregraphics/blob/main/System.Drawing.Drawing2D/GraphicsPath.cs
 * https://github.com/mono/sysdrawing-coregraphics/blob/main/Utilities/GeomUtilities.cs
 * https://github.com/mono/sysdrawing-coregraphics/blob/main/System.Drawing/Graphics.cs
 */

#include "Matrix.h"

enum PathPointType {
   PathPointType_Bezier = 3,
   PathPointType_Bezier3 = 3,
   PathPointType_Line = 1,
   PathPointType_PathTypeMask = 7,
   PathPointType_Start = 0,
   PathPointType_CloseSubpath = 128,
};

enum CurveType {
   CurveType_Open,
   CurveType_Close,
};

class GraphicsPath {
public:
   GraphicsPath();
   void Append (float x, float y, PathPointType type, bool compress);
   void AppendBezier (float x1, float y1, float x2, float y2, float x3, float y3);
   void AppendPoint(SDL_FPoint point, PathPointType type, bool compress);
   void AppendCurve(vector<SDL_FPoint> points, vector<SDL_FPoint> tangents, int offset, int length, CurveType type);
   void CloseFigure();

   void AddLine(float x1, float y1, float x2, float y2);
   void AddBezier(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4);
   void AddEllipse(const SDL_FRect& rect);
   void AddEllipse(float x, float y, float width, float height);
   void AddClosedCurve(const vector<SDL_FPoint>& points);
   void AddClosedCurve(const vector<SDL_FPoint>& points, float tension);
   void AddPolygon(const vector<SDL_FPoint>& points);
   void AddRectangle(const SDL_FRect& rect);
   SDL_FRect GetBounds(VP::Matrix* pMatrix) const;
   void Transform(VP::Matrix* pMatrix);
   GraphicsPath* Clone() const;

   const vector<SDL_FPoint>& GetPoints() const { return m_points; }
   const vector<PathPointType>& GetTypes() const { return m_types; }

private:
   static int FlattenPath(GraphicsPath* pPath, VP::Matrix* pMatrix, float flatness);
   static bool ConvertBezierToLines(GraphicsPath* pPath, int index, float flatness, vector<SDL_FPoint> flat_points, vector<PathPointType> flat_types);
   static bool NRCurveFlatten(float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3, float flatness, int level, vector<SDL_FPoint> points);
   static bool PathHasCurve(GraphicsPath* pPath);
   static vector<SDL_FPoint> GetCurveTangents(int terms, vector<SDL_FPoint> points, int count, float tension, CurveType type);

   vector<SDL_FPoint> m_points;
   vector<PathPointType> m_types;

   bool m_startNewFig;
   bool m_isReverseWindingOnFill;
};