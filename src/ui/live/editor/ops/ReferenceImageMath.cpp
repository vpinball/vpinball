// license:GPLv3+

#include "core/stdafx.h"
#include "ReferenceImageMath.h"

namespace VPX::EditorUI::ReferenceImageMath
{

float ToVPU(const float value, const Unit unit) { return unit == Unit::Inches ? INCHESTOVPU(value) : CMTOVPU(value); }

float FromVPU(const float vpu, const Unit unit) { return unit == Unit::Inches ? VPUTOINCHES(vpu) : VPUTOCM(vpu); }

float HeightForWidth(const float width, const unsigned int pxWidth, const unsigned int pxHeight)
{
   if (pxWidth == 0 || pxHeight == 0)
      return 0.f;
   return width * static_cast<float>(pxHeight) / static_cast<float>(pxWidth);
}

float AspectMismatch(const float width, const float height, const unsigned int pxWidth, const unsigned int pxHeight)
{
   if (pxWidth == 0 || pxHeight == 0 || width <= 0.f || height <= 0.f)
      return 0.f;
   const double entered = static_cast<double>(width) / static_cast<double>(height);
   const double image = static_cast<double>(pxWidth) / static_cast<double>(pxHeight);
   return static_cast<float>(fabs(entered / image - 1.0));
}

bool IsAspectMismatch(const float width, const float height, const unsigned int pxWidth, const unsigned int pxHeight)
{
   return AspectMismatch(width, height, pxWidth, pxHeight) > AspectTolerance;
}

float SizeFromDpi(const unsigned int pxCount, const float dpi)
{
   if (dpi <= 0.f)
      return 0.f;
   return INCHESTOVPU(static_cast<float>(pxCount) / dpi);
}

Rect FitToAspect(const Rect &rect, const unsigned int pxWidth, const unsigned int pxHeight)
{
   if (pxWidth == 0 || pxHeight == 0)
      return rect;
   return { rect.center, rect.width, HeightForWidth(rect.width, pxWidth, pxHeight) };
}

Vertex2D UniformScale(const Vertex2D &requested, const Vertex2D &current, const Vertex2D &lastRequested)
{
   if (current.x <= 0.f || current.y <= 0.f)
      return requested;
   const auto same = [](const float a, const float b) { return fabsf(a - b) <= 1e-5f * max(fabsf(a), fabsf(b)); };
   if (same(requested.x * current.y, requested.y * current.x))
      return requested;
   const bool xChanged = !same(requested.x, lastRequested.x), yChanged = !same(requested.y, lastRequested.y);
   if (!xChanged && !yChanged)
      return current; // The same request as the last one (mouse held still, typed value): already applied
   const float rx = requested.x / current.x, ry = requested.y / current.y;
   const bool byX = xChanged != yChanged ? xChanged : fabsf(rx - 1.f) >= fabsf(ry - 1.f);
   const float r = byX ? rx : ry;
   return Vertex2D(current.x * r, current.y * r);
}

std::array<Vertex2D, 4> RectCorners(const Rect &rect)
{
   const float hw = 0.5f * rect.width, hh = 0.5f * rect.height;
   const Vertex2D &c = rect.center;
   return { Vertex2D(c.x - hw, c.y - hh), Vertex2D(c.x - hw, c.y + hh), Vertex2D(c.x + hw, c.y + hh), Vertex2D(c.x + hw, c.y - hh) };
}

}
