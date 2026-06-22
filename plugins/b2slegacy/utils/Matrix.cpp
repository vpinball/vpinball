/*
 * Portions of this code was originally derived from Mono, FreeQuartz, and Xamarin:
 *
 * https://github.com/mono/sysdrawing-coregraphics/blob/main/System.Drawing.Drawing2D/Matrix.cs
 * https://github.com/DnMllr/freequartz/blob/master/CoreGraphics/CGAffineTransform.c
 * https://github.com/DnMllr/freequartz/blob/master/CoreGraphics/include/CoreGraphics/CGAffineTransform.h
 * https://github.com/xamarin/xamarin-macios/blob/main/src/CoreGraphics/CGAffineTransform.cs
 */

#include "Matrix.h"

#include <cmath>

#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795
#endif

namespace B2SLegacy {

// Pre-multiply by a rotation (closed form of rotation * current).
void Matrix::Rotate(float angleDegrees)
{
   const float angle = angleDegrees * (float)(M_PI / 180.0);
   const float sn = sinf(angle);
   const float cs = cosf(angle);
   const Matrix m = *this;
   a =  cs * m.a + sn * m.c;
   b =  cs * m.b + sn * m.d;
   c = -sn * m.a + cs * m.c;
   d = -sn * m.b + cs * m.d;
}

}
