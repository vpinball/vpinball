#pragma once

#include <assert.h>
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


__forceinline float precise_divide(const float a, const float b)
{
    return _mm_cvtss_f32(_mm_div_ss(_mm_set_ss(a), _mm_set_ss(b)));
}


// from "Scalar quantization to a signed integer" by Rutanen, keeps values under continuous quantize/dequantize cycles constant (and more)
// BUT cannot map max values, e.g. for 7 bits: -1. -> -127 -> 0.99.. and 1. -> 127 -> 0.99..
/*template <unsigned char bits> // bits to map to, including negative numbers, e.g. mapping just to unsigned: bits+1 (0..255 -> bits = 9)
__forceinline float dequantizeSigned(const int i)
{
    enum { N = (1 << (bits - 1)) - 1 };
    return clamp(precise_divide((float)i, (float)((double)N + 0.5)), -1.f, 1.f); //!! test: optimize div or does this break precision?
}

template <unsigned char bits> // bits to map to, including negative numbers, e.g. mapping just to unsigned: bits+1 (0..255 -> bits = 9)
__forceinline int quantizeSigned(const float x)
{
    enum { N = (1 << (bits - 1)) - 1 };
    const float sign = (x >= 0.f) ? 0.5f : -0.5f;
    return (int)(clamp(x * (float)((double)N + 0.5) + sign, -(float)N, (float)N));
}*/

template <unsigned char bits> // bits to map to
__forceinline float dequantizeUnsigned(const unsigned int i)
{
    enum { N = (1 << bits) - 1 };
    return min(precise_divide((float)i, (float)N), 1.f); //!! test: optimize div or does this break precision?
}

template <unsigned char bits> // bits to map to
__forceinline unsigned int quantizeUnsigned(const float x)
{
    enum { N = (1 << bits) - 1, Np1 = (1 << bits) };
    assert(x >= 0.f);
    return min((unsigned int)(x * (float)Np1), (unsigned int)N);
}


__forceinline float dequantizeSignedPercent(const int i)
{
    enum { N = 100 };
    return clamp(precise_divide((float)i, (float)N), -1.f, 1.f); //!! test: optimize div or does this break precision?
}

__forceinline int quantizeSignedPercent(const float x)
{
    enum { N = 100, Np1 = 101 };
    return clamp((int)(x * (float)Np1), -(int)N, (int)N);
}

__forceinline float dequantizeUnsignedPercent(const unsigned int i)
{
    enum { N = 100 };
    return min(precise_divide((float)i, (float)N), 1.f); //!! test: optimize div or does this break precision?
}

__forceinline float dequantizeUnsignedPercentNoClamp(const unsigned int i)
{
    enum { N = 100 };
    return precise_divide((float)i, (float)N); //!! test: optimize div or does this break precision?
}

__forceinline unsigned int quantizeUnsignedPercent(const float x)
{
    enum { N = 100, Np1 = 101 };
    assert(x >= 0.f);
    return min((unsigned int)(x * (float)Np1), (unsigned int)N);
}


// Math stuff which doesn't fit elsewhere

// Solve the quadratic equation ax^2 + bx + c = 0.
// Returns true if there are real solutions, false otherwise.
bool SolveQuadraticEq(const float a, const float b, const float c, float& sol1, float& sol2);

Vertex3Ds SolveCubicEq(Vertex4D Coefficient);
