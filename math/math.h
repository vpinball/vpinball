#pragma once

#include <math.h>

// Math stuff which doesn't fit elsewhere

// Solve the quadratic equation ax^2 + bx + c = 0.
// Returns true if there are real solutions, false otherwise.
bool SolveQuadraticEq(float a, float b, float c, float& sol1, float& sol2);

