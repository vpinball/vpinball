#include "stdafx.h"

bool SolveQuadraticEq(const float a, const float b, const float c, float& sol1, float& sol2)
{
    float discr = b*b - 4.0f*a*c;

    if (discr < 0.f)
        return false;

    discr = sqrtf(discr);

    const float inv_a = (-0.5f)/a;
    sol1 = (b + discr) * inv_a;
    sol2 = (b - discr) * inv_a;

    return true;
}

