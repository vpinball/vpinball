#pragma once

/*
 * Portions of this code was derived from Mono, FreeQuartz, and Xamarin:
 *
 * https://github.com/mono/sysdrawing-coregraphics/blob/main/System.Drawing.Drawing2D/Matrix.cs
 * https://github.com/DnMllr/freequartz/blob/master/CoreGraphics/CGAffineTransform.c
 * https://github.com/DnMllr/freequartz/blob/master/CoreGraphics/include/CoreGraphics/CGAffineTransform.h
 * https://github.com/xamarin/xamarin-macios/blob/main/src/CoreGraphics/CGAffineTransform.cs
 */

namespace VP {

struct AffineTransform {
  float a, b, c, d;
  float tx, ty;
};

AffineTransform AffineTransformMakeTranslation(float tx, float ty);
AffineTransform AffineTransformMakeScale(float sx, float sy);
AffineTransform AffineTransformMakeRotation(float angle);
AffineTransform AffineTransformMakeMultiply(const AffineTransform& a, const AffineTransform& b);

class Matrix final
{
public:
    Matrix();
    Matrix(const Matrix& matrix);
    Matrix(float m11, float m12, float m21, float m22, float dx, float dy);
    
    void Rotate(float angle);
    void Multiply(const Matrix& other);
    void Translate(float x, float y);
    void Shear(float shearX, float shearY);
    void Scale(float scaleX, float scaleY);
    void TransformPoint(float& x, float& y);
    void TransformPoints(vector<SDL_FPoint>& points);
    void Reset();
    Matrix* Clone() const;

    AffineTransform m_transform;
};

}
