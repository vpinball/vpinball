#pragma once
#include "Material.h"
#include "Texture.h"

/*
 * Compute coefficients for a cubic polynomial
 *   p(s) = c0 + c1*s + c2*s^2 + c3*s^3
 * such that
 *   p(0) = x0, p(1) = x1
 *  and
 *   p'(0) = t0, p'(1) = t1.
 */
inline void InitCubicSplineCoeffs(const float x0, const float x1, const float t0, const float t1,
   float &c0, float &c1, float &c2, float &c3)
{
   c0 = x0;
   c1 = t0;
   c2 = -3.0f*x0 + 3.0f*x1 - 2.0f*t0 - t1;
   c3 = 2.0f*x0 - 2.0f*x1 + t0 + t1;
}

// standard uniform Catmull-Rom splines with tension 0.5
inline void InitCatmullCoeffs(const float x0, const float x1, const float x2, const float x3,
   float &c0, float &c1, float &c2, float &c3)
{
   InitCubicSplineCoeffs(x1, x2, 0.5f*(x2 - x0), 0.5f*(x3 - x1),
      c0, c1, c2, c3);
}

// nonuniform Catmull-Rom splines; see
//  http://en.wikipedia.org/wiki/Centripetal_Catmull%E2%80%93Rom_spline
//  http://www.cemyuksel.com/research/catmullrom_param/catmullrom.pdf
//  P. J. Barry and R. N. Goldman: A recursive evaluation algorithm for a class of Catmull-Rom splines
//
inline void InitNonuniformCatmullCoeffs(float x0, float x1, float x2, float x3, float dt0, float dt1, float dt2,
   float &c0, float &c1, float &c2, float &c3)
{
   // compute tangents when parameterized in [t1,t2]
   float t1 = (x1 - x0) / dt0 - (x2 - x0) / (dt0 + dt1) + (x2 - x1) / dt1;
   float t2 = (x2 - x1) / dt1 - (x3 - x1) / (dt1 + dt2) + (x3 - x2) / dt2;

   // rescale tangents for parametrization in [0,1]
   t1 *= dt1;
   t2 *= dt1;

   InitCubicSplineCoeffs(x1, x2, t1, t2, c0, c1, c2, c3);
}

template <int Dim>
class CatmullCurve;

// This uses centripetal Catmull-Rom splines for avoiding cusps and smoother results overall
template <>
class CatmullCurve<2>
{
public:
   void SetCurve(const Vertex2D& v0, const Vertex2D& v1, const Vertex2D& v2, const Vertex2D& v3)
   {
      float dt0 = sqrtf((v1 - v0).Length());
      float dt1 = sqrtf((v2 - v1).Length());
      float dt2 = sqrtf((v3 - v2).Length());

      // check for repeated control points
      if (dt1 < 1e-4f)    dt1 = 1.0f;
      if (dt0 < 1e-4f)    dt0 = dt1;
      if (dt2 < 1e-4f)    dt2 = dt1;

      InitNonuniformCatmullCoeffs(v0.x, v1.x, v2.x, v3.x, dt0, dt1, dt2,
         cx0, cx1, cx2, cx3);
      InitNonuniformCatmullCoeffs(v0.y, v1.y, v2.y, v3.y, dt0, dt1, dt2,
         cy0, cy1, cy2, cy3);
   }

   void SetCurve(const Vertex3Ds& v0, const Vertex3Ds& v1, const Vertex3Ds& v2, const Vertex3Ds& v3)
   {
      SetCurve(v0.xy(), v1.xy(), v2.xy(), v3.xy());
   }

   void GetPointAt(float t, Vertex2D * pv) const
   {
      const float t2 = t*t;
      const float t3 = t2*t;

      pv->x = cx3 * t3 + cx2 * t2 + cx1 * t + cx0;
      pv->y = cy3 * t3 + cy2 * t2 + cy1 * t + cy0;
   }

private:
   float cx0, cx1, cx2, cx3;
   float cy0, cy1, cy2, cy3;
};


template <>
class CatmullCurve<3>
{
public:
   void SetCurve(const Vertex3Ds& v0, const Vertex3Ds& v1, const Vertex3Ds& v2, const Vertex3Ds& v3)
   {
      float dt0 = sqrtf((v1 - v0).Length());
      float dt1 = sqrtf((v2 - v1).Length());
      float dt2 = sqrtf((v3 - v2).Length());

      // check for repeated control points
      if (dt1 < 1e-4f)    dt1 = 1.0f;
      if (dt0 < 1e-4f)    dt0 = dt1;
      if (dt2 < 1e-4f)    dt2 = dt1;

      InitNonuniformCatmullCoeffs(v0.x, v1.x, v2.x, v3.x, dt0, dt1, dt2,
         cx0, cx1, cx2, cx3);
      InitNonuniformCatmullCoeffs(v0.y, v1.y, v2.y, v3.y, dt0, dt1, dt2,
         cy0, cy1, cy2, cy3);
      InitNonuniformCatmullCoeffs(v0.z, v1.z, v2.z, v3.z, dt0, dt1, dt2,
         cz0, cz1, cz2, cz3);
   }

   void GetPointAt(float t, Vertex3Ds * pv) const
   {
      const float t2 = t*t;
      const float t3 = t2*t;

      pv->x = cx3 * t3 + cx2 * t2 + cx1 * t + cx0;
      pv->y = cy3 * t3 + cy2 * t2 + cy1 * t + cy0;
      pv->z = cz3 * t3 + cz2 * t2 + cz1 * t + cz0;
   }

private:
   float cx0, cx1, cx2, cx3;
   float cy0, cy1, cy2, cy3;
   float cz0, cz1, cz2, cz3;
};


class RenderVertex3D : public Vertex3Ds
{
public:
   void set(const Vertex3Ds &v) { x = v.x; y = v.y; z = v.z; }

   static const int Dim = 3;

   bool fSmooth;
   bool fSlingshot;
   bool fControlPoint; // Whether this point was a control point on the curve
   bool padd; // Useless padding to align to 4bytes, should enhance access speeds
};

class RenderVertex : public Vertex2D
{
public:
   void set(const Vertex3Ds &v) { x = v.x; y = v.y; }
   void set(const RenderVertex &v) { *this = v; }
   void set(const RenderVertex3D &v) { x = v.x; y = v.y; fSmooth = v.fSmooth; fSlingshot = v.fSlingshot; fControlPoint = v.fControlPoint; }

   static const int Dim = 2;

   bool fSmooth;
   bool fSlingshot;
   bool fControlPoint; // Whether this point was a control point on the curve
   bool padd; // Useless padding to align to 4bytes, should enhance access speeds
};


template <class VtxType>
void SetHUDVertices(VtxType * const rgv, const int count)
{
   const float mult = (float)g_pplayer->m_width * (float)(1.0 / EDITOR_BG_WIDTH)
      * (g_pplayer->m_pin3d.m_useAA ? 2.0f : 1.0f); //!! WTF?
   const float ymult = mult /
      (((float)g_pplayer->m_screenwidth / (float)g_pplayer->m_screenheight) / (float)(4.0 / 3.0)); //!! ?

   for (int i = 0; i < count; ++i)
   {
      rgv[i].tu = rgv[i].x = rgv[i].x*mult - 0.5f;  //!! abuses tu tv to pass position :/
      rgv[i].tv = rgv[i].y = rgv[i].y*ymult - 0.5f;
      rgv[i].z = 0.f;

      rgv[i].nx = 0.0f;
      rgv[i].ny = 0.0f;
      rgv[i].nz = 0.0f;
   }
}

template <class CurveType, class VtxType, class VtxContType>
void RecurseSmoothLine(const CurveType & cc, float t1, float t2, const VtxType & vt1, const VtxType & vt2, VtxContType & vv, const float accuracy)
{
   const float tMid = (t1 + t2)*0.5f;
   VtxType vmid;
   cc.GetPointAt(tMid, &vmid);
   vmid.fSmooth = true; // Generated points must always be smooth, because they are part of the curve
   vmid.fSlingshot = false; // Slingshots can't be along curves
   vmid.fControlPoint = false; // We created this point, so it can't be a control point

   if (FlatWithAccuracy(vt1, vt2, vmid, accuracy))
   {
      // Add first segment point to array.
      // Last point never gets added by this recursive loop,
      // but that's where it wraps around to the next curve.
      vv.push_back(vt1);
   }
   else
   {
      RecurseSmoothLine(cc, t1, tMid, vt1, vmid, vv, accuracy);
      RecurseSmoothLine(cc, tMid, t2, vmid, vt2, vv, accuracy);
   }
}

inline float GetDot(const Vertex2D * const pvEnd1, const Vertex2D * const pvJoint, const Vertex2D * const pvEnd2)
{
   return (pvJoint->x - pvEnd1->x)*(pvJoint->y - pvEnd2->y) - (pvJoint->y - pvEnd1->y)*(pvJoint->x - pvEnd2->x);
}

inline bool FLinesIntersect(const Vertex2D * const Start1, const Vertex2D * const Start2, const Vertex2D * const End1, const Vertex2D * const End2)
{
   const float x1 = Start1->x;
   const float y1 = Start1->y;
   const float x2 = Start2->x;
   const float y2 = Start2->y;
   const float x3 = End1->x;
   const float y3 = End1->y;
   const float x4 = End2->x;
   const float y4 = End2->y;

   const float d123 = (x2 - x1)*(y3 - y1) - (x3 - x1)*(y2 - y1);

   if (d123 == 0.0f) // p3 lies on the same line as p1 and p2
      return (x3 >= min(x1, x2) && x3 <= max(x2, x1));

   const float d124 = (x2 - x1)*(y4 - y1) - (x4 - x1)*(y2 - y1);

   if (d124 == 0.0f) // p4 lies on the same line as p1 and p2
      return (x4 >= min(x1, x2) && x4 <= max(x2, x1));

   if (d123 * d124 >= 0.0f)
      return false;

   const float d341 = (x3 - x1)*(y4 - y1) - (x4 - x1)*(y3 - y1);

   if (d341 == 0.0f) // p1 lies on the same line as p3 and p4
      return (x1 >= min(x3, x4) && x1 <= max(x3, x4));

   const float d342 = d123 - d124 + d341;

   if (d342 == 0.0f) // p1 lies on the same line as p3 and p4
      return (x2 >= min(x3, x4) && x2 <= max(x3, x4));

   return (d341 * d342 < 0.0f);
}

// RenderVertexCont is either an array or a Vector<> of RenderVertex
template <class RenderVertexCont>
inline bool AdvancePoint(const RenderVertexCont& rgv, const std::vector<unsigned int>& pvpoly, const unsigned int a, const unsigned int b, const unsigned int c, const int pre, const int post)
{
   const RenderVertex * const pv1 = &rgv[a];
   const RenderVertex * const pv2 = &rgv[b];
   const RenderVertex * const pv3 = &rgv[c];

   const RenderVertex * const pvPre = &rgv[pre];
   const RenderVertex * const pvPost = &rgv[post];

   if ((GetDot(pv1, pv2, pv3) < 0) ||
      // Make sure angle created by new triangle line falls inside existing angles
      // If the existing angle is a concave angle, then new angle must be smaller,
      // because our triangle can't have angles greater than 180
      ((GetDot(pvPre, pv1, pv2) > 0) && (GetDot(pvPre, pv1, pv3) < 0)) || // convex angle, make sure new angle is smaller than it
      ((GetDot(pv2, pv3, pvPost) > 0) && (GetDot(pv1, pv3, pvPost) < 0)))
      return false;

   // Now make sure the interior segment of this triangle (line ac) does not
   // intersect the polygon anywhere

   // sort our static line segment

   const float minx = min(pv1->x, pv3->x);
   const float maxx = max(pv1->x, pv3->x);
   const float miny = min(pv1->y, pv3->y);
   const float maxy = max(pv1->y, pv3->y);

   for (size_t i = 0; i < pvpoly.size(); ++i)
   {
      const RenderVertex * const pvCross1 = &rgv[pvpoly[i]];
      const RenderVertex * const pvCross2 = &rgv[pvpoly[(i < pvpoly.size() - 1) ? (i + 1) : 0]];

      if (pvCross1 != pv1 && pvCross2 != pv1 && pvCross1 != pv3 && pvCross2 != pv3 &&
         (pvCross1->y >= miny || pvCross2->y >= miny) &&
         (pvCross1->y <= maxy || pvCross2->y <= maxy) &&
         (pvCross1->x >= minx || pvCross2->x >= minx) &&
         (pvCross1->x <= maxx || pvCross2->y <= maxx) &&
         FLinesIntersect(pv1, pv3, pvCross1, pvCross2))
         return false;
   }

   return true;
}

inline float GetCos(const Vertex2D * const pvEnd1, const Vertex2D * const pvJoint, const Vertex2D * const pvEnd2)
{
   const Vertex2D vt1(pvJoint->x - pvEnd1->x, pvJoint->y - pvEnd1->y);
   const Vertex2D vt2(pvJoint->x - pvEnd2->x, pvJoint->y - pvEnd2->y);

   const float dot = vt1.x*vt2.y - vt1.y*vt2.x;

   return dot / sqrtf((vt1.x * vt1.x + vt1.y * vt1.y)*(vt2.x * vt2.x + vt2.y * vt2.y));
}

/*
inline float GetAngle(const Vertex2D * const pvEnd1, const Vertex2D * const pvJoint, const Vertex2D * const pvEnd2)
{
const float slope1 = (pvJoint->y - pvEnd1->y) / (pvJoint->x - pvEnd1->x);
const float slope2 = (pvJoint->y - pvEnd2->y) / (pvJoint->x - pvEnd2->x);
return atan2f((slope2-slope1),(1.0f+slope1*slope2));
}
*/

// Computes the normal for a single, plane polygon described by the indices and applies it either
// to the original vertices or to the vertices indexed by rgiApply.
//
// This functions uses Newell's method to compute the normal. It produces the correct result for
// a clockwise polygon in a left-handed coordinate system, or for a counterclockwise polygon in
// a right-handed coordinate system.
template <class VtxType, class IdxType>
void SetNormal(VtxType * const rgv, const IdxType * const rgi, const int count, void * prgvApply = NULL, const IdxType * rgiApply = NULL, int applycount = 0)
{
   // If apply-to array is null, just apply the resulting normal to incoming array
   VtxType * rgvApply = prgvApply ? (VtxType*)prgvApply : rgv;

   if (rgiApply == NULL)
      rgiApply = rgi;

   if (applycount == 0)
      applycount = count;

   Vertex3Ds vnormal(0.0f, 0.0f, 0.0f);

   for (int i = 0; i < count; ++i)
   {
      const int l = rgi[i];
      const int m = rgi[(i < count - 1) ? (i + 1) : 0];

      vnormal.x += (rgv[l].y - rgv[m].y) * (rgv[l].z + rgv[m].z);
      vnormal.y += (rgv[l].z - rgv[m].z) * (rgv[l].x + rgv[m].x);
      vnormal.z += (rgv[l].x - rgv[m].x) * (rgv[l].y + rgv[m].y);
   }

   vnormal.Normalize();

   for (int i = 0; i < applycount; ++i)
   {
      const int l = rgiApply[i];
      rgvApply[l].nx = vnormal.x;
      rgvApply[l].ny = vnormal.y;
      rgvApply[l].nz = vnormal.z;
   }
}

// Calculate if two vectors are flat to each other
// accuracy is a float greater 4 and smaller 4000000 (tested this out)
inline bool FlatWithAccuracy(const Vertex2D & v1, const Vertex2D & v2, const Vertex2D & vMid, const float accuracy)
{
   // compute double the signed area of the triangle (v1, vMid, v2)
   const float dblarea = (vMid.x - v1.x)*(v2.y - v1.y) - (v2.x - v1.x)*(vMid.y - v1.y);

   return (dblarea*dblarea < accuracy);
}

inline bool FlatWithAccuracy(const Vertex3Ds & v1, const Vertex3Ds & v2, const Vertex3Ds & vMid, const float accuracy)
{
   // compute the square of double the signed area of the triangle (v1, vMid, v2)
   const float dblareasq = CrossProduct(vMid - v1, v2 - v1).LengthSquared();

   return (dblareasq < accuracy);
}

// find closest point, projected on xy plane
template <class VtxContType>
inline void ClosestPointOnPolygon(const VtxContType &rgv, const Vertex2D &pvin, Vertex2D * const pvout, int * const piseg, const bool fClosed)
{
   const int count = (int)rgv.size();

   float mindist = FLT_MAX;
   int seg = -1;
   *piseg = -1; // in case we are not next to the line

   int cloop = count;
   if (!fClosed)
      --cloop; // Don't check segment running from the end point to the beginning point

   // Go through line segment, calculate distance from point to the line
   // then pick the shortest distance
   for (int i = 0; i < cloop; ++i)
   {
      const int p2 = (i < count - 1) ? (i + 1) : 0;

      RenderVertex rgvi;
      rgvi.set(rgv[i]);
      RenderVertex rgvp2;
      rgvp2.set(rgv[p2]);
      const float A = rgvi.y - rgvp2.y;
      const float B = rgvp2.x - rgvi.x;
      const float C = -(A*rgvi.x + B*rgvi.y);

      const float dist = fabsf(A*pvin.x + B*pvin.y + C) / sqrtf(A*A + B*B);

      if (dist < mindist)
      {
         // Assuming we got a segment that we are closet to, calculate the intersection
         // of the line with the perpenticular line projected from the point,
         // to find the closest point on the line
         const float D = -B;
         const float F = -(D*pvin.x + A*pvin.y);

         const float det = A*A - B*D;
         const float inv_det = (det != 0.0f) ? 1.0f / det : 0.0f;
         const float intersectx = (B*F - A*C)*inv_det;
         const float intersecty = (C*D - A*F)*inv_det;

         // If the intersect point lies on the polygon segment
         // (not out in space), then make this the closest known point
         if (intersectx >= (min(rgvi.x, rgvp2.x) - 0.1f) &&
            intersectx <= (max(rgvi.x, rgvp2.x) + 0.1f) &&
            intersecty >= (min(rgvi.y, rgvp2.y) - 0.1f) &&
            intersecty <= (max(rgvi.y, rgvp2.y) + 0.1f))
         {
            mindist = dist;
            seg = i;
            pvout->x = intersectx;
            pvout->y = intersecty;
            *piseg = seg;
         }
      }
   }
}

template <class RenderVertexCont, class Idx>
void PolygonToTriangles(const RenderVertexCont& rgv, std::vector<unsigned int>& pvpoly, std::vector<Idx>& pvtri)
{
   // There should be this many convex triangles.
   // If not, the polygon is self-intersecting
   const size_t tricount = pvpoly.size() - 2;

   assert(tricount > 0);

   for (size_t l = 0; l < tricount; ++l)
      //while (pvpoly->Size() > 2)
   {
      for (size_t i = 0; i < pvpoly.size(); ++i)
      {
         const size_t s = pvpoly.size();
         const unsigned int pre = pvpoly[(i == 0) ? (s - 1) : (i - 1)];
         const unsigned int a = pvpoly[i];
         const unsigned int b = pvpoly[(i < s - 1) ? (i + 1) : 0];
         const unsigned int c = pvpoly[(i < s - 2) ? (i + 2) : ((i + 2) - s)];
         const unsigned int post = pvpoly[(i < s - 3) ? (i + 3) : ((i + 3) - s)];
         if (AdvancePoint(rgv, pvpoly, a, b, c, pre, post))
         {
            pvtri.push_back(a);
            pvtri.push_back(c);
            pvtri.push_back(b);
            pvpoly.erase(pvpoly.begin() + ((i < s - 1) ? (i + 1) : 0)); // b
            break;
         }
      }
   }
}

template <typename T>
void ComputeNormals(Vertex3D_NoTex2* const vertices, const unsigned int numVertices, const T* const indices, const unsigned int numIndices)
{
   for (unsigned i = 0; i < numVertices; i++)
   {
      Vertex3D_NoTex2 &v = vertices[i];
      v.nx = v.ny = v.nz = 0.0f;
   }

   for (unsigned i = 0; i < numIndices; i += 3)
   {
      Vertex3D_NoTex2& A = vertices[indices[i]];
      Vertex3D_NoTex2& B = vertices[indices[i + 1]];
      Vertex3D_NoTex2& C = vertices[indices[i + 2]];

      const Vertex3Ds e0(B.x - A.x, B.y - A.y, B.z - A.z);
      const Vertex3Ds e1(C.x - A.x, C.y - A.y, C.z - A.z);
      Vertex3Ds normal = CrossProduct(e0, e1);
      normal.NormalizeSafe();

      A.nx += normal.x; A.ny += normal.y; A.nz += normal.z;
      B.nx += normal.x; B.ny += normal.y; B.nz += normal.z;
      C.nx += normal.x; C.ny += normal.y; C.nz += normal.z;
   }

   for (unsigned i = 0; i < numVertices; i++)
   {
      Vertex3D_NoTex2 &v = vertices[i];
      const float l = v.nx*v.nx + v.ny*v.ny + v.nz*v.nz;
      const float inv_l = (l >= FLT_MIN) ? 1.0f / sqrtf(l) : 0.f;
      v.nx *= inv_l;
      v.ny *= inv_l;
      v.nz *= inv_l;
   }
}

template <typename T>
void ComputeNormals(std::vector<Vertex3D_NoTex2>& vertices, const std::vector<T>& indices)
{
   ComputeNormals(&vertices[0], (unsigned int)vertices.size(), &indices[0], (unsigned int)indices.size());
}
