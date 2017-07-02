#include "stdafx.h"

bool SolveQuadraticEq(const float a, const float b, const float c, float& sol1, float& sol2)
{
   float discr = b*b - 4.0f*a*c;

   if (discr < 0.f)
      return false;

   discr = sqrtf(discr);

   const float inv_a = (-0.5f) / a;
   sol1 = (b + discr) * inv_a;
   sol2 = (b - discr) * inv_a;

   return true;
}

// Blinn: How to solve a cubic equation, part 5: Back to numerics
Vertex3Ds SolveCubicEq(Vertex4D Coefficient) // x = ^0, y = ^1, z = ^2, w = ^3
{
    // Normalize the polynomial
    float inv_w = 1.f/Coefficient.w;
    Coefficient.x *= inv_w;
    // Divide middle coefficients by three
    inv_w *= (float)(1./3.);
    Coefficient.y *= inv_w;
    Coefficient.z *= inv_w;    
    // Compute the Hessian and the discrimant
    const Vertex3Ds Delta(
        Coefficient.y-Coefficient.z*Coefficient.z,
        Coefficient.x-Coefficient.y*Coefficient.z,
        Coefficient.z*Coefficient.x-Coefficient.y*Coefficient.y
    );
    const float Discriminant = 4.0f*Delta.x*Delta.z-Delta.y*Delta.y;
    // Compute coefficients of the depressed cubic 
    // (third is zero, fourth is one)
    const Vertex2D Depressed(
        2.0f*Coefficient.z*Delta.x-Delta.y,
        Delta.x);
    // Take the cubic root of a normalized complex number
    const float Theta = atan2f(sqrtf(Discriminant),Depressed.x)*(float)(1./3.);
    const float CubicRootX = cosf(Theta);
    const float CubicRootY = sinf(Theta);
    // Compute the three roots, scale appropriately and 
    // revert the depression transform
    const Vertex2D Root(
        -1.7320508075688772935274463415059f*CubicRootY-CubicRootX, //sqrtf(3.0)
         1.7320508075688772935274463415059f*CubicRootY-CubicRootX  //sqrtf(3.0)
    );
    const float tmp = sqrtf(-Depressed.y);
    return Vertex3Ds(tmp*2.0f*CubicRootX-Coefficient.z,
                     tmp*Root.x-Coefficient.z,
                     tmp*Root.y-Coefficient.z);
}
