#pragma once

class FRect
{
public:
   float left, top, right, bottom;

   Vertex2D Center() const
   {
      return Vertex2D(0.5f*(left + right), 0.5f*(top + bottom));
   }
};

class FRect3D
{
public:
   float left, top, right, bottom, zlow, zhigh;

   FRect3D()    { }

   FRect3D(const float x1, const float x2, const float y1, const float y2, const float z1, const float z2)
      : left(x1), right(x2),
      top(y1), bottom(y2),
      zlow(z1), zhigh(z2)
   { }

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

inline bool Intersect(const RECT &rc, const int width, const int height, const POINT &p, const bool rotated) // width & height in percent/[0..100]-range
{
   if (!rotated)
      return (p.x >= rc.left*width / 100 && p.x <= rc.right*width / 100 && p.y >= rc.top*height / 100 && p.y <= rc.bottom*height / 100);
   else
      return (p.x >= rc.top*width / 100 && p.x <= rc.bottom*width / 100 && p.y <= height - rc.left*height / 100 && p.y >= height - rc.right*height / 100);
}

inline bool fRectIntersect3D(const FRect3D &rc1, const FRect3D &rc2)
{
   const __m128 rc1128 = _mm_loadu_ps(&rc1.left); // this shouldn't use loadu, but doesn't matter anymore nowadays anyhow
   const __m128 rc1sh = _mm_shuffle_ps(rc1128, rc1128, _MM_SHUFFLE(1, 0, 3, 2));
   const __m128 test = _mm_cmpge_ps(rc1sh, _mm_loadu_ps(&rc2.left));
   const int mask = _mm_movemask_ps(test);
   //   return ((mask == (1|(1<<1)|(0<<2)|(0<<3))) && rc1.zlow <= rc2.zhigh && rc1.zhigh >= rc2.zlow); //!! use SSE, too?
   return ((mask == 3) && rc1.zlow <= rc2.zhigh && rc1.zhigh >= rc2.zlow); //!! use SSE, too?

   //return (rc1.right >= rc2.left && rc1.bottom >= rc2.top && rc1.left <= rc2.right && rc1.top <= rc2.bottom && rc1.zlow <= rc2.zhigh && rc1.zhigh >= rc2.zlow);
}

inline bool fRectIntersect3D(const Vertex3Ds &sphere_p, const float sphere_rsqr, const FRect3D &rc)
{
	float ex = max(rc.left - sphere_p.x, 0.f) + max(sphere_p.x - rc.right, 0.f);
	float ey = max(rc.top - sphere_p.y, 0.f) + max(sphere_p.y - rc.bottom, 0.f);
	float ez = max(rc.zlow - sphere_p.z, 0.f) + max(sphere_p.z - rc.zhigh, 0.f);
	ex *= ex;
	ey *= ey;
	ez *= ez;
	return (ex + ey + ez <= sphere_rsqr);
}
