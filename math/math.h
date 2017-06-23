#pragma once

#include <math.h>
#include "vector.h"

#if _MSC_VER < 1800
__forceinline float exp2f(const float e)
{
   return powf(2.0f,e);
}
#endif

__forceinline float sqrf(const float x)
{
   return x*x;
}

// Math stuff which doesn't fit elsewhere

// Solve the quadratic equation ax^2 + bx + c = 0.
// Returns true if there are real solutions, false otherwise.
bool SolveQuadraticEq(const float a, const float b, const float c, float& sol1, float& sol2);

Vertex3Ds SolveCubicEq(Vertex4D Coefficient);
