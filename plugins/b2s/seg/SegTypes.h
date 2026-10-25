// license:GPLv3+

#pragma once

#include <cstdint>

// Minimal geometry types standing in for the SDL ones used by the retired
// B2SLegacy plugin this code is ported from (SDL_FPoint/FRect, SDL_Rect).

namespace B2S::seg
{

struct FPoint final
{
   float x = 0.f, y = 0.f;
};

struct FRect final
{
   float x = 0.f, y = 0.f, w = 0.f, h = 0.f;
};

struct IPoint final
{
   int x = 0, y = 0;
};

struct IRect final
{
   int x = 0, y = 0, w = 0, h = 0;
};

inline bool IsRectEmpty(const FRect& r) { return r.w <= 0.f || r.h <= 0.f; }

inline bool IsRectEmpty(const IRect& r) { return r.w <= 0 || r.h <= 0; }

// Packed 0x00RRGGBB color (COLORREF layout, as used by the reference implementation)
inline constexpr uint32_t RGB(uint8_t r, uint8_t g, uint8_t b) { return static_cast<uint32_t>(r) | (static_cast<uint32_t>(g) << 8) | (static_cast<uint32_t>(b) << 16); }

}
