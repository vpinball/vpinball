// license:GPLv3+

#pragma once

class FRect3D final
{
public:
   float left, right, top, bottom, zlow, zhigh;

   FRect3D() {}

   FRect3D(const float x1, const float x2, const float y1, const float y2, const float z1, const float z2)
      : left(x1), right(x2),
      top(y1), bottom(y2),
      zlow(z1), zhigh(z2)
   {}

   void Clear()
   {
      left = FLT_MAX;  right = -FLT_MAX;
      top = FLT_MAX;   bottom = -FLT_MAX;
      zlow = FLT_MAX;  zhigh = -FLT_MAX;
   }

   void Extend(const FRect3D& other)
   {
      left = min(left, other.left);
      right = max(right, other.right);
      top = min(top, other.top);
      bottom = max(bottom, other.bottom);
      zlow = min(zlow, other.zlow);
      zhigh = max(zhigh, other.zhigh);
   }
};

class FRect final
{
public:
   float left, top, right, bottom;

   Vertex2D Center() const
   {
      return {0.5f*(left + right), 0.5f*(top + bottom)};
   }

   FRect() {}

   FRect(const float x1, const float x2, const float y1, const float y2)
      : left(x1), top(y1), right(x2), bottom(y2)
   {}

   void Clear()
   {
      left = FLT_MAX;  right = -FLT_MAX;
      top = FLT_MAX;   bottom = -FLT_MAX;
   }

   void Extend(const FRect& other)
   {
      left = min(left, other.left);
      right = max(right, other.right);
      top = min(top, other.top);
      bottom = max(bottom, other.bottom);
   }

   void Extend(const FRect3D& other)
   {
      left = min(left, other.left);
      right = max(right, other.right);
      top = min(top, other.top);
      bottom = max(bottom, other.bottom);
   }
};

inline bool Intersect(const RECT &rc, const int width, const int height, const POINT &p, const bool rotated) // width & height in percent/[0..100]-range
{
   if (!rotated)
      return (p.x >= rc.left*width / 100 && p.x <= rc.right*width / 100 && p.y >= rc.top*height / 100 && p.y <= rc.bottom*height / 100);
   else
      return (p.x >= rc.top*width / 100 && p.x <= rc.bottom*width / 100 && p.y <= height - rc.left*height / 100 && p.y >= height - rc.right*height / 100);
}

#ifdef ENABLE_SSE_OPTIMIZATIONS
inline bool fRectIntersect3D(const FRect3D &rc1, const FRect3D &rc2)
{
   const __m128 rc1128 = _mm_loadu_ps(&rc1.left); // L1.R1.T1.B1 // this shouldn't use loadu, but doesn't matter anymore nowadays anyhow
   const __m128 rc2128 = _mm_loadu_ps(&rc2.left); // L2.R2.T2.B2
   const __m128 min128 = _mm_shuffle_ps(rc1128, rc2128, _MM_SHUFFLE(1, 3, 1, 3)); // R1.B1.R2.B2
   const __m128 max128 = _mm_shuffle_ps(rc2128, rc1128, _MM_SHUFFLE(0, 2, 0, 2)); // L2.T2.L1.T1
   const __m128 test = _mm_cmpge_ps(min128, max128);
   const int mask = _mm_movemask_ps(test); // bitmask of [R1 >= L2, B1 >= T2, R2 >= L1, B2 >= T1]
   return ((mask == 15) && rc1.zlow <= rc2.zhigh && rc1.zhigh >= rc2.zlow); //!! use SSE, too?
}
#else
#pragma message ("Warning: No SSE bbox tests")
inline bool fRectIntersect3D(const FRect3D &rc1, const FRect3D &rc2)
{
   return (rc1.right >= rc2.left && rc1.bottom >= rc2.top && rc1.zhigh >= rc2.zlow 
        && rc1.left <= rc2.right && rc1.top <= rc2.bottom && rc1.zlow <= rc2.zhigh);
}
#endif

inline bool fRectIntersect3D(const Vertex3Ds &sphere_p, const float sphere_rsqr, const FRect3D &rc) // could also use SSE, but kd and quadtree already have native SSE variants in there
{
   float ex = max(rc.left - sphere_p.x, 0.f) + max(sphere_p.x - rc.right, 0.f);
   float ey = max(rc.top - sphere_p.y, 0.f) + max(sphere_p.y - rc.bottom, 0.f);
   float ez = max(rc.zlow - sphere_p.z, 0.f) + max(sphere_p.z - rc.zhigh, 0.f);
   ex *= ex;
   ey *= ey;
   ez *= ez;
   return (ex + ey + ez <= sphere_rsqr);
}

namespace plog
{
Record& operator<<(Record& record, const FRect& pt);
Record& operator<<(Record& record, const FRect3D& pt);
}
