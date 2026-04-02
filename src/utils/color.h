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
   return {r, g, b, w};
}

inline vec3 convertColor(const COLORREF c)
{
   const float r = (float)(c & 255) * (float)(1.0/255.0);
   const float g = (float)(c & 65280) * (float)(1.0/65280.0);
   const float b = (float)(c & 16711680) * (float)(1.0/16711680.0);
   return {r, g, b};
}

inline COLORREF convertColorRGB(const vec3& color)
{
   const int r = (int)clamp(color.x * 255.f + 0.5f, 0.f, 255.f);
   const int g = (int)clamp(color.y * 255.f + 0.5f, 0.f, 255.f);
   const int b = (int)clamp(color.z * 255.f + 0.5f, 0.f, 255.f);
   return RGB(r, g, b);
}

inline COLORREF convertColorRGB(const vec4& color)
{
   const int r = (int)clamp(color.x * 255.f + 0.5f, 0.f, 255.f);
   const int g = (int)clamp(color.y * 255.f + 0.5f, 0.f, 255.f);
   const int b = (int)clamp(color.z * 255.f + 0.5f, 0.f, 255.f);
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

namespace VPX::Colors
{
   // Utility functions to convert between colorspaces:
   // - sRGB = a.k.a standard RGB (see https://en.wikipedia.org/wiki/SRGB)
   // - linear RGB = same as sRGB but not gamma compressed
   // - XYZ = CIE 1931 XYZ (see https://en.wikipedia.org/wiki/CIE_1931_color_space but matrix are from CIE RGB, see http://www.brucelindbloom.com/index.html?Eqn_RGB_XYZ_Matrix.html)
   // - LAB = CIE LAB (see https://en.wikipedia.org/wiki/CIELAB_color_space)
   // - HSV = same as ImGui, see Foley & van Dam p593 or http://en.wikipedia.org/wiki/HSL_and_HSV

   inline vec3 LinearRGBToSRGB(const vec3& rgb) { return {sRGB(rgb.x), sRGB(rgb.y), sRGB(rgb.z)}; }

   inline vec3 SRGBToLinearRGB(const vec3& rgb) { return {InvsRGB(rgb.x), InvsRGB(rgb.y), InvsRGB(rgb.z)}; }

   inline uint32_t SRGBToRGB32(const vec3& rgb) { return ((uint32_t)(rgb.x * 255.f)) | (((uint32_t)(rgb.y * 255.f)) << 8) | (((uint32_t)(rgb.z * 255.f)) << 16); }
   inline uint32_t SRGBToRGBA32(const vec3& rgb, float a)
   {
      return ((uint32_t)(rgb.x * 255.f)) | (((uint32_t)(rgb.y * 255.f)) << 8) | (((uint32_t)(rgb.z * 255.f)) << 16) | (((uint32_t)(a * 255.f)) << 24);
   }
   inline uint32_t SRGBAToRGBA32(const vec4& rgba)
   {
      return ((uint32_t)(rgba.x * 255.f)) | (((uint32_t)(rgba.y * 255.f)) << 8) | (((uint32_t)(rgba.z * 255.f)) << 16) | (((uint32_t)(rgba.w * 255.f)) << 24);
   }

   inline vec3 LinearRGBtoXYZ(const vec3& linearRGB)
   {
      return { //
         0.4124564f * linearRGB.x + 0.3575761f * linearRGB.y + 0.1804375f * linearRGB.z, //
         0.2126729f * linearRGB.x + 0.7151522f * linearRGB.y + 0.0721750f * linearRGB.z, //
         0.0193339f * linearRGB.x + 0.1191920f * linearRGB.y + 0.9503041f * linearRGB.z
      };
   }

   static vec3 XYZToLAB(vec3 xyz)
   {
      // Reference white (D65)
      constexpr float refX = 0.95047f;
      //constexpr float refY = 1.00000f;
      constexpr float refZ = 1.08883f;

      // Normalize XYZ values
      xyz.x /= refX;
      //xyz.y /= refY;
      xyz.z /= refZ;

      // Apply nonlinear transform
      xyz.x = (xyz.x > 0.008856f) ? cbrtf(xyz.x) : (7.787f * xyz.x) + (float)(16.0 / 116.0);
      xyz.y = (xyz.y > 0.008856f) ? cbrtf(xyz.y) : (7.787f * xyz.y) + (float)(16.0 / 116.0);
      xyz.z = (xyz.z > 0.008856f) ? cbrtf(xyz.z) : (7.787f * xyz.z) + (float)(16.0 / 116.0);

      // Compute L, a, b
      const float l = (float)(116.0 / 100.) * xyz.y - (float)(16.0 / 100.);
      const float a = (xyz.x - xyz.y) * (float)(500. / 128.);
      const float b = (xyz.y - xyz.z) * (float)(200. / 128.);

      return vec3(l, a, b);
   }

   inline float LuminanceFromLinearRGB(const vec3& linearRGB) { return 0.212655f * linearRGB.x + 0.715158f * linearRGB.y + 0.072187f * linearRGB.z; }

   inline vec3 HSVToSRGB(const vec3& hsv)
   {
      if (hsv.y == 0.0f)
         return vec3 { hsv.z, hsv.z, hsv.z }; // gray
 
      const float h = fmodf(hsv.x, 1.0f) / (float)(60.0 / 360.0);
      int i = static_cast<int>(h);
      float f = h - static_cast<float>(i);
      float p = hsv.z * (1.0f - hsv.y);
      float q = hsv.z * (1.0f - hsv.y * f);
      float t = hsv.z * (1.0f - hsv.y * (1.0f - f));
      switch (i)
      {
      case 0: return { hsv.z, t, p };
      case 1: return { q, hsv.z, p };
      case 2: return { p, hsv.z, t };
      case 3: return { p, q, hsv.z };
      case 4: return { t, p, hsv.z };
      case 5:
      default: return { hsv.z, p, q };
      }
   }

   inline vec3 SRGBToHSV(const vec3& srgb)
   {
      float r = srgb.x;
      float g = srgb.y;
      float b = srgb.z;

      float max = std::max({ r, g, b });
      float min = std::min({ r, g, b });
      float delta = max - min;

      float h = 0.0f;
      float s = 0.0f;
      float v = max;

      if (delta > 0.0f)
      {
         s = delta / max;

         if (r == max)
            h = (g - b) / delta;
         else if (g == max)
            h = 2.0f + (b - r) / delta;
         else // b == max
            h = 4.0f + (r - g) / delta;

         h *= 60.0f;
         if (h < 0.0f)
            h += 360.0f;
      }

      return { h, s, v };
   }

};
