// license:GPLv3+

#pragma once

#include "../renderer/typedefs3D.h"

#ifdef RGB
 #undef RGB
#endif
#define RGB(r,g,b) ((COLORREF)(((uint32_t)(r)) | (((uint32_t)(g))<<8) | (((uint32_t)(b))<<16)))

inline D3DCOLOR COLORREF_to_D3DCOLOR(const COLORREF c)
{
   // COLORREF: 0x00BBGGRR (from windef)
   // D3DCOLOR: 0xAARRGGBB (from d3d9.h)
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

inline COLORREF convertColorRGB(const vec3& color)
{
   const int r = clamp((int)(color.x * 255.f + 0.5f), 0, 255);
   const int g = clamp((int)(color.y * 255.f + 0.5f), 0, 255);
   const int b = clamp((int)(color.z * 255.f + 0.5f), 0, 255);
   return RGB(r, g, b);
}

inline COLORREF convertColorRGB(const vec4& color)
{
   const int r = clamp((int)(color.x * 255.f + 0.5f), 0, 255);
   const int g = clamp((int)(color.y * 255.f + 0.5f), 0, 255);
   const int b = clamp((int)(color.z * 255.f + 0.5f), 0, 255);
   return RGB(r, g, b);
}

inline float sRGB(const float f)
{
   return (f <= 0.0031308f) ? (12.92f * f) : (1.055f * powf(f, (float)(1.0 / 2.4)) - 0.055f);
}

inline float InvsRGB(const float x)
{
   return (x <= 0.04045f) ? (x * (float)(1.0 / 12.92)) : (powf(x * (float)(1.0 / 1.055) + (float)(0.055 / 1.055), 2.4f));
}

constexpr inline float invGammaApprox(const float c)
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
