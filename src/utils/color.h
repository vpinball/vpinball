// license:GPLv3+

#pragma once

#include "math/matrix.h"

#ifdef RGB
 #undef RGB
#endif
#define RGB(r,g,b) ((COLORREF)(((DWORD)(r)) | (((DWORD)(g))<<8) | (((DWORD)(b))<<16)))

inline D3DCOLOR COLORREF_to_D3DCOLOR(const COLORREF c)
{
   // COLORREF: 0x00BBGGRR
   // D3DCOLOR: 0xAARRGGBB
   const COLORREF r = (c & 0x000000ff);
   const COLORREF g = (c & 0x0000ff00) >> 8;
   const COLORREF b = (c & 0x00ff0000) >> 16;
   return b | (g << 8) | (r << 16) | 0xff000000;
}

inline vec4 convertColor(const COLORREF c, const float w)
{
   const float r = (float)(c & 255) * (float)(1.0/255.0);
   const float g = (float)(c & 65280) * (float)(1.0/65280.0);
   const float b = (float)(c & 16711680) * (float)(1.0/16711680.0);
   return vec4(r,g,b,w);
}

inline vec3 convertColor(const COLORREF c)
{
   const float r = (float)(c & 255) * (float)(1.0 / 255.0);
   const float g = (float)(c & 65280) * (float)(1.0 / 65280.0);
   const float b = (float)(c & 16711680) * (float)(1.0 / 16711680.0);
   return vec3(r, g, b);
}

inline COLORREF convertColorRGB(const vec4& color)
{
   const int r = clamp((int)(color.x * 255.f + 0.5f), 0, 255);
   const int g = clamp((int)(color.y * 255.f + 0.5f), 0, 255);
   const int b = clamp((int)(color.z * 255.f + 0.5f), 0, 255);
   return RGB(r, g, b);
}

inline float InvsRGB(const float x)
{
   return (x <= 0.04045f) ? (x * (float)(1.0 / 12.92)) : (powf(x * (float)(1.0 / 1.055) + (float)(0.055 / 1.055), 2.4f));
}

inline float invGammaApprox(const float c)
{
   return c * (c * (c * 0.305306011f + 0.682171111f) + 0.012522878f); /*pow(color,2.2f);*/ // pow does still matter on current CPUs (not GPUs though)
}

inline float gammaApprox(const float c)
{
   const float t0 = sqrtf(c);
   const float t1 = sqrtf(t0);
   const float t2 = sqrtf(t1);
   return 0.662002687f * t0 + 0.684122060f * t1 - 0.323583601f * t2 - 0.0225411470f * c; /*pow(color,(float)(1.0/2.2));*/ // pow does still matter on current CPUs (not GPUs though)
}
