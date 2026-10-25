#pragma once

#include <vector>
using std::vector;

#include "SegTypes.h"
#include "Matrix.h"

namespace B2S::seg
{

class GraphicsPath final
{
public:
   GraphicsPath();

   void AddPolygon(const vector<FPoint>* const pPoints);
   void AddEllipse(const FRect& rect);
   void AddEllipse(float x, float y, float width, float height);
   void AddClosedCurve(const vector<FPoint>* const pPoints, float tension);
   void AddRectangle(const FRect& rect);
   void Transform(Matrix* pMatrix);
   const vector<FPoint>* GetPoints() const { return &m_points; }

private:
   vector<FPoint> m_points;
};

}
