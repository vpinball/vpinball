/*
 * Portions of this code was derived from Mono, FreeQuartz, and Xamarin:
 *
 * https://github.com/mono/sysdrawing-coregraphics/blob/main/System.Drawing.Drawing2D/Matrix.cs
 * https://github.com/DnMllr/freequartz/blob/master/CoreGraphics/CGAffineTransform.c
 * https://github.com/DnMllr/freequartz/blob/master/CoreGraphics/include/CoreGraphics/CGAffineTransform.h
 * https://github.com/xamarin/xamarin-macios/blob/main/src/CoreGraphics/CGAffineTransform.cs
 */

#include "stdafx.h"

#include "Matrix.h"

#include <cmath>

namespace VP {

AffineTransform AffineTransformMakeIdentity()
{
   AffineTransform tRet = {1,0,0,1,0,0};
   return tRet;
}

AffineTransform AffineTransformMakeTranslation(float tx, float ty)
{
   AffineTransform tRet = {1,0,0,1,tx,ty};
   return tRet;
}

AffineTransform AffineTransformMakeScale(float sx, float sy)
{
   AffineTransform tRet = {sx,0,0,sy,0,0};
   return tRet;
}

AffineTransform AffineTransformMakeRotation(float angle)
{
   float sine = sinf(angle);
   float cosine = cosf(angle);

   AffineTransform tRet = { cosine,sine, -sine,cosine, 0,0 };
   return tRet;
}

AffineTransform AffineTransformMakeMultiply(const AffineTransform& a, const AffineTransform& b)
{
   AffineTransform tRet = {
      a.a * b.a + a.b * b.c,
      a.a * b.b + a.b * b.d,
      a.c * b.a + a.d * b.c,
      a.c * b.b + a.d * b.d,
      a.tx * b.a + a.ty * b.c + b.tx,
      a.tx * b.b + a.ty * b.d + b.ty
   };
   return tRet;
}

Matrix::Matrix()
{
   m_transform = AffineTransformMakeIdentity();
}

Matrix::Matrix(const Matrix& matrix)
{
   m_transform = matrix.m_transform;
}

Matrix::Matrix(float m11, float m12, float m21, float m22, float dx, float dy)
{
   m_transform = {m11, m12, m21, m22, dx, dy};
}

void Matrix::Translate(float offsetX, float offsetY)
{
   AffineTransform affine = AffineTransformMakeTranslation(offsetX, offsetY);
   m_transform = AffineTransformMakeMultiply(affine, m_transform);
}

void Matrix::Rotate(float angle)
{
   angle *= (float)(M_PI / 180.0);
   AffineTransform affine = AffineTransformMakeRotation(angle);
   m_transform = AffineTransformMakeMultiply(affine, m_transform);
}

void Matrix::Multiply(const Matrix& matrix)
{
   AffineTransform mtrans = matrix.m_transform;
   m_transform = AffineTransformMakeMultiply(mtrans, m_transform);
}

void Matrix::Shear(float shearX, float shearY)
{
   AffineTransform affine = {1, shearY, shearX, 1, 0, 0};
   m_transform = AffineTransformMakeMultiply(affine, m_transform);
}

void Matrix::Scale(float scaleX, float scaleY)
{
   AffineTransform affine = AffineTransformMakeScale(scaleX, scaleY);
   m_transform = AffineTransformMakeMultiply(affine, m_transform);
}

void Matrix::TransformPoint(float& x, float& y)
{
   SDL_FPoint point = {x, y};
   x = m_transform.a * point.x + m_transform.c * point.y + m_transform.tx;
   y = m_transform.b * point.x + m_transform.d * point.y + m_transform.ty;
}

void Matrix::TransformPoints(vector<SDL_FPoint>& points)
{
   for (auto& point : points) {
      float x = m_transform.a * point.x + m_transform.c * point.y + m_transform.tx;
      float y = m_transform.b * point.x + m_transform.d * point.y + m_transform.ty;
    
      point.x = x;
      point.y = y;
   }
}

void Matrix::Reset()
{
   m_transform = AffineTransformMakeIdentity();
}

Matrix* Matrix::Clone() const
{
   return new Matrix(*this);
}

}
