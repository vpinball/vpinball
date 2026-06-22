#pragma once

#include <SDL3/SDL.h>
#include <vector>
using std::vector;

/*
 * Portions of this code was originally derived from Mono, FreeQuartz, and Xamarin:
 *
 * https://github.com/mono/sysdrawing-coregraphics/blob/main/System.Drawing.Drawing2D/Matrix.cs
 * https://github.com/DnMllr/freequartz/blob/master/CoreGraphics/CGAffineTransform.c
 * https://github.com/DnMllr/freequartz/blob/master/CoreGraphics/include/CoreGraphics/CGAffineTransform.h
 * https://github.com/xamarin/xamarin-macios/blob/main/src/CoreGraphics/CGAffineTransform.cs
 */

namespace B2SLegacy {

// 2D affine transform stored for row-vector math:
//   [x' y' 1] = [x y 1] * | a  b  0 |
//                         | c  d  0 |
//                         | tx ty 1 |
class Matrix final
{
public:
   Matrix() = default;
   constexpr Matrix(float m11, float m12, float m21, float m22, float dx, float dy)
      : a(m11), b(m12), c(m21), d(m22), tx(dx), ty(dy) {}

   void Translate(float x, float y)
   {
      tx += x * a + y * c;
      ty += x * b + y * d;
   }

   // Append a translation: applied to points *after* the existing transform, so it simply shifts the output
   // (unlike Translate, which prepends)
   void TranslatePost(float x, float y)
   {
      tx += x;
      ty += y;
   }

   void Scale(float scaleX, float scaleY)
   {
      a *= scaleX; b *= scaleX;
      c *= scaleY; d *= scaleY;
   }

   void Shear(float shearX, float shearY)
   {
      const Matrix m = *this;
      a = m.a + shearY * m.c;
      b = m.b + shearY * m.d;
      c = shearX * m.a + m.c;
      d = shearX * m.b + m.d;
   }

   void Rotate(float angleDegrees);

   void Multiply(const Matrix& other)
   {
      *this = Concat(other, *this);
   }

   void TransformPoint(float& x, float& y) const
   {
      const float px = x, py = y;
      x = a * px + c * py + tx;
      y = b * px + d * py + ty;
   }

   void TransformPoints(vector<SDL_FPoint>& points) const
   {
      const Matrix t = *this;
      for (auto& point : points) {
         const float px = point.x, py = point.y;
         point.x = t.a * px + t.c * py + t.tx;
         point.y = t.b * px + t.d * py + t.ty;
      }
   }

   void Reset() { *this = Matrix(); }

   float a = 1.0f, b = 0.0f, c = 0.0f, d = 1.0f;
   float tx = 0.0f, ty = 0.0f;

private:
   // result = lhs * rhs (points are transformed by lhs first, then rhs)
   static constexpr Matrix Concat(const Matrix& l, const Matrix& r)
   {
      return {
         l.a * r.a + l.b * r.c,
         l.a * r.b + l.b * r.d,
         l.c * r.a + l.d * r.c,
         l.c * r.b + l.d * r.d,
         l.tx * r.a + l.ty * r.c + r.tx,
         l.tx * r.b + l.ty * r.d + r.ty
      };
   }
};

}
