#pragma once

#include "Matrix.h"

class GraphicsPath final
{
public:
   GraphicsPath();

   GraphicsPath* Clone() const;
   void AddPolygon(const vector<SDL_FPoint>* const pPoints);
   void AddEllipse(const SDL_FRect& rect);
   void AddEllipse(float x, float y, float width, float height);
   void AddClosedCurve(const vector<SDL_FPoint>* const pPoints, float tension);
   void AddRectangle(const SDL_FRect& rect);
   void Transform(VP::Matrix* pMatrix);
   const vector<SDL_FPoint>* GetPoints() const { return &m_points; }

private:
   vector<SDL_FPoint> m_points;
};
