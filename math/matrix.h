#pragma once

#include "typedefs3D.h"
#include <math/vector.h>

#ifdef ENABLE_SDL
class Matrix3D;

class D3DXMATRIX {
public:
   union {
      struct {
         float        _11, _12, _13, _14;
         float        _21, _22, _23, _24;
         float        _31, _32, _33, _34;
         float        _41, _42, _43, _44;
      };
      float m[4][4];
   };

   D3DXMATRIX();
   D3DXMATRIX(const D3DXMATRIX &input);
   D3DXMATRIX(const D3DXMATRIX * const input);
   D3DXMATRIX(const Matrix3D &input);
};

#define D3DMATRIX D3DXMATRIX
#endif

// 3x3 matrix for representing linear transformation of 3D vectors
class Matrix3 final
{
public:
   Matrix3()
   {
   }

   Matrix3(const float __11, const float __12, const float __13, const float __21, const float __22, const float __23, const float __31, const float __32, const float __33)
   {
      _11 = __11; _12 = __12; _13 = __13;
      _21 = __21; _22 = __22; _23 = __23;
      _31 = __31; _32 = __32; _33 = __33;
   }

   void scaleX(const float factor)
   {
      m_d[0][0] *= factor;
   }
   void scaleY(const float factor)
   {
      m_d[1][1] *= factor;
   }
   void scaleZ(const float factor)
   {
      m_d[2][2] *= factor;
   }

//
// license:GPLv3+
// Ported at: VisualPinball.Engine/Math/Matrix2D.cs
//

   void SkewSymmetric(const Vertex3Ds &pv3D)
   {
      m_d[0][0] = 0.f;     m_d[0][1] = -pv3D.z; m_d[0][2] = pv3D.y;
      m_d[1][0] = pv3D.z;  m_d[1][1] = 0.f;     m_d[1][2] = -pv3D.x;
      m_d[2][0] = -pv3D.y; m_d[2][1] = pv3D.x;  m_d[2][2] = 0.f;
   }

//
// end of license:GPLv3+, back to 'old MAME'-like
//

   void MulScalar(const float scalar)
   {
      for (int i = 0; i < 3; ++i)
         for (int l = 0; l < 3; ++l)
            m_d[i][l] *= scalar;
   }

   Matrix3 operator+ (const Matrix3& m) const
   {
      return Matrix3(_11 + m._11, _12 + m._12, _13 + m._13,
                     _21 + m._21, _22 + m._22, _23 + m._23,
                     _31 + m._31, _32 + m._32, _33 + m._33);
   }

   Matrix3 operator* (const Matrix3& m) const
   {
      return Matrix3(
         m_d[0][0] * m.m_d[0][0] + m_d[1][0] * m.m_d[0][1] + m_d[2][0] * m.m_d[0][2],
         m_d[0][0] * m.m_d[1][0] + m_d[1][0] * m.m_d[1][1] + m_d[2][0] * m.m_d[1][2],
         m_d[0][0] * m.m_d[2][0] + m_d[1][0] * m.m_d[2][1] + m_d[2][0] * m.m_d[2][2],
         m_d[0][1] * m.m_d[0][0] + m_d[1][1] * m.m_d[0][1] + m_d[2][1] * m.m_d[0][2],
         m_d[0][1] * m.m_d[1][0] + m_d[1][1] * m.m_d[1][1] + m_d[2][1] * m.m_d[1][2],
         m_d[0][1] * m.m_d[2][0] + m_d[1][1] * m.m_d[2][1] + m_d[2][1] * m.m_d[2][2],
         m_d[0][2] * m.m_d[0][0] + m_d[1][2] * m.m_d[0][1] + m_d[2][2] * m.m_d[0][2],
         m_d[0][2] * m.m_d[1][0] + m_d[1][2] * m.m_d[1][1] + m_d[2][2] * m.m_d[1][2],
         m_d[0][2] * m.m_d[2][0] + m_d[1][2] * m.m_d[2][1] + m_d[2][2] * m.m_d[2][2]
      );
   }

   template <class VecType>
   Vertex3Ds operator* (const VecType& v) const
   {
      return Vertex3Ds(
         m_d[0][0] * v.x + m_d[0][1] * v.y + m_d[0][2] * v.z,
         m_d[1][0] * v.x + m_d[1][1] * v.y + m_d[1][2] * v.z,
         m_d[2][0] * v.x + m_d[2][1] * v.y + m_d[2][2] * v.z);
   }

   template <class VecType>
   Vertex3Ds MulVector(const VecType& v) const
   {
      return (*this) * v;
   }

   template <class VecType>
   Vertex3Ds MultiplyVector(const VecType& v) const
   {
      return (*this) * v;
   }

   // multiply vector with matrix transpose
   template <class VecType>
   Vertex3Ds MulVectorT(const VecType& v) const
   {
      return Vertex3Ds(
         m_d[0][0] * v.x + m_d[1][0] * v.y + m_d[2][0] * v.z,
         m_d[0][1] * v.x + m_d[1][1] * v.y + m_d[2][1] * v.z,
         m_d[0][2] * v.x + m_d[1][2] * v.y + m_d[2][2] * v.z);
   }

   void MulMatrices(const Matrix3& pmat1, const Matrix3& pmat2)
   {
      Matrix3 matans;
      for (int i = 0; i < 3; ++i)
         for (int l = 0; l < 3; ++l)
            matans.m_d[i][l] = pmat1.m_d[i][0] * pmat2.m_d[0][l] +
                               pmat1.m_d[i][1] * pmat2.m_d[1][l] +
                               pmat1.m_d[i][2] * pmat2.m_d[2][l];
      *this = matans;
   }

   void MulMatricesAndMulScalar(const Matrix3& pmat1, const Matrix3& pmat2, const float scalar)
   {
      Matrix3 matans;
      for (int i = 0; i < 3; ++i)
         for (int l = 0; l < 3; ++l)
            matans.m_d[i][l] = (pmat1.m_d[i][0] * pmat2.m_d[0][l] +
                                pmat1.m_d[i][1] * pmat2.m_d[1][l] +
                                pmat1.m_d[i][2] * pmat2.m_d[2][l])*scalar;
      *this = matans;
   }

   void AddMatrix(const Matrix3& pmat)
   {
      for (int i = 0; i < 3; ++i)
         for (int l = 0; l < 3; ++l)
            m_d[i][l] += pmat.m_d[i][l];
   }

   void OrthoNormalize()
   {
      Vertex3Ds vX(m_d[0][0], m_d[1][0], m_d[2][0]);
      Vertex3Ds vY(m_d[0][1], m_d[1][1], m_d[2][1]);
      Vertex3Ds vZ = CrossProduct(vX, vY);
      vX.Normalize();
      vZ.Normalize();
      vY = CrossProduct(vZ, vX);
      //vY.Normalize(); // not needed

      m_d[0][0] = vX.x; m_d[0][1] = vY.x; m_d[0][2] = vZ.x;
      m_d[1][0] = vX.y; m_d[1][1] = vY.y; m_d[1][2] = vZ.y;
      m_d[2][0] = vX.z; m_d[2][1] = vY.z; m_d[2][2] = vZ.z;
   }

   /*void Transpose(Matrix3 * const pmatOut) const
   {
      for (int i = 0; i < 3; ++i)
      {
         pmatOut->m_d[0][i] = m_d[i][0];
         pmatOut->m_d[1][i] = m_d[i][1];
         pmatOut->m_d[2][i] = m_d[i][2];
      }
   }*/

   void Identity(const float value = 1.0f)
   {
      m_d[0][0] = m_d[1][1] = m_d[2][2] = value;
      m_d[0][1] = m_d[0][2] =
      m_d[1][0] = m_d[1][2] =
      m_d[2][0] = m_d[2][1] = 0.0f;
   }

//
// license:GPLv3+
// Ported at: VisualPinball.Engine/Math/Matrix2D.cs
//

   // Create matrix for rotating around an arbitrary vector
   // NB: axis must be normalized
   // NB: this actually rotates by -angle in right-handed coordinates
   void RotationAroundAxis(const Vertex3Ds& axis, const float angle)
   {
      const float rsin = sinf(angle);
      const float rcos = cosf(angle);

      m_d[0][0] = axis.x*axis.x + rcos*(1.0f - axis.x*axis.x);
      m_d[1][0] = axis.x*axis.y*(1.0f - rcos) - axis.z*rsin;
      m_d[2][0] = axis.z*axis.x*(1.0f - rcos) + axis.y*rsin;

      m_d[0][1] = axis.x*axis.y*(1.0f - rcos) + axis.z*rsin;
      m_d[1][1] = axis.y*axis.y + rcos*(1.0f - axis.y*axis.y);
      m_d[2][1] = axis.y*axis.z*(1.0f - rcos) - axis.x*rsin;

      m_d[0][2] = axis.z*axis.x*(1.0f - rcos) - axis.y*rsin;
      m_d[1][2] = axis.y*axis.z*(1.0f - rcos) + axis.x*rsin;
      m_d[2][2] = axis.z*axis.z + rcos*(1.0f - axis.z*axis.z);
   }

//
// end of license:GPLv3+, back to 'old MAME'-like
//

   void RotationAroundAxis(const Vertex3Ds& axis, const float rsin, const float rcos)
   {
      m_d[0][0] = axis.x*axis.x + rcos*(1.0f - axis.x*axis.x);
      m_d[1][0] = axis.x*axis.y*(1.0f - rcos) - axis.z*rsin;
      m_d[2][0] = axis.z*axis.x*(1.0f - rcos) + axis.y*rsin;

      m_d[0][1] = axis.x*axis.y*(1.0f - rcos) + axis.z*rsin;
      m_d[1][1] = axis.y*axis.y + rcos*(1.0f - axis.y*axis.y);
      m_d[2][1] = axis.y*axis.z*(1.0f - rcos) - axis.x*rsin;

      m_d[0][2] = axis.z*axis.x*(1.0f - rcos) - axis.y*rsin;
      m_d[1][2] = axis.y*axis.z*(1.0f - rcos) + axis.x*rsin;
      m_d[2][2] = axis.z*axis.z + rcos*(1.0f - axis.z*axis.z);
   }

   union {
      struct {
         float _11, _12, _13;
         float _21, _22, _23;
         float _31, _32, _33;
      };
      float m_d[3][3];
   };
};


////////////////////////////////////////////////////////////////////////////////


// 4x4 matrix for representing affine transformations of 3D vectors
class Matrix3D final : public D3DMATRIX
{
public:
   Matrix3D() {}
   Matrix3D(const float Scale) { SetScaling(Scale, Scale, Scale); }
   Matrix3D(
      const float __11, const float __12, const float __13, const float __14, const float __21, const float __22, const float __23, const float __24, const float __31, const float __32, const float __33, const float __34, const float __41, const float __42, const float __43, const float __44)
   {
      _11 = __11; _12 = __12; _13 = __13; _14 = __14;
      _21 = __21; _22 = __22; _23 = __23; _24 = __24;
      _31 = __31; _32 = __32; _33 = __33; _34 = __34;
      _41 = __41; _42 = __42; _43 = __43; _44 = __44;
   }

//
// license:GPLv3+
// Ported at: VisualPinball.Engine/Math/Matrix3D.cs
//

   // premultiply the given matrix, i.e., result = mult * (*this)
   void Multiply(const Matrix3D &mult, Matrix3D &result) const
   {
      Matrix3D matrixT;
#ifdef ENABLE_SSE_OPTIMIZATIONS
      // could replace the loadu/storeu's if alignment would be stricter
      for (int i = 0; i < 16; i += 4) {
         // unroll first step of the loop
         __m128 a = _mm_loadu_ps(&_11);
         __m128 b = _mm_set1_ps((&mult._11)[i]);
         __m128 r = _mm_mul_ps(a, b);
         for (int j = 1; j < 4; j++) {
            a = _mm_loadu_ps((&_11)+j * 4);
            b = _mm_set1_ps((&mult._11)[i + j]);
            r = _mm_add_ps(_mm_mul_ps(a, b), r);
         }
         _mm_storeu_ps((&matrixT._11)+i, r);
      }
#else
#pragma message ("Warning: No SSE matrix mul")
      for (int i = 0; i < 4; ++i)
         for (int l = 0; l < 4; ++l)
            matrixT.m[i][l] = (m[0][l] * mult.m[i][0]) + (m[1][l] * mult.m[i][1]) + (m[2][l] * mult.m[i][2]) + (m[3][l] * mult.m[i][3]);
#endif
      result = matrixT;
   }

   Matrix3D operator*(const Matrix3D& mult) const
   {
      Matrix3D matrixT;
#ifdef ENABLE_SSE_OPTIMIZATIONS
      // could replace the loadu/storeu's if alignment would be stricter
      for (int i = 0; i < 16; i += 4) {
         // unroll first step of the loop
         __m128 a = _mm_loadu_ps(&mult._11);
         __m128 b = _mm_set1_ps((&_11)[i]);
         __m128 r = _mm_mul_ps(a, b);
         for (int j = 1; j < 4; j++) {
            a = _mm_loadu_ps((&mult._11)+j * 4);
            b = _mm_set1_ps((&_11)[i + j]);
            r = _mm_add_ps(_mm_mul_ps(a, b), r);
         }
         _mm_storeu_ps((&matrixT._11)+i, r);
      }
#else
#pragma message ("Warning: No SSE matrix mul")
      for (int i = 0; i < 4; ++i)
         for (int l = 0; l < 4; ++l)
            matrixT.m[i][l] = (mult.m[0][l] * m[i][0]) + (mult.m[1][l] * m[i][1]) + (mult.m[2][l] * m[i][2]) + (mult.m[3][l] * m[i][3]);
#endif
      return matrixT;
   }

   void SetTranslation(const float tx, const float ty, const float tz)
   {
      SetIdentity();
      _41 = tx;
      _42 = ty;
      _43 = tz;
   }
   void SetTranslation(const Vertex3Ds& t)
   {
      SetTranslation(t.x, t.y, t.z);
   }

   void SetScaling(const float sx, const float sy, const float sz)
   {
      SetIdentity();
      _11 = sx;
      _22 = sy;
      _33 = sz;
   }

   void RotateXMatrix(const float x)
   {
      SetIdentity();
      _22 = _33 = cosf(x);
      _23 = sinf(x);
      _32 = -_23;
   }

   void RotateYMatrix(const float y)
   {
      SetIdentity();
      _11 = _33 = cosf(y);
      _31 = sinf(y);
      _13 = -_31;
   }

   void RotateZMatrix(const float z)
   {
      SetIdentity();
      _11 = _22 = cosf(z);
      _12 = sinf(z);
      _21 = -_12;
   }

   void SetIdentity()
   {
      _11 = _22 = _33 = _44 = 1.0f;
      _12 = _13 = _14 = _41 =
      _21 = _23 = _24 = _42 =
      _31 = _32 = _34 = _43 = 0.0f;
   }
   void Scale(const float x, const float y, const float z)
   {
      _11 *= x; _12 *= x; _13 *= x;
      _21 *= y; _22 *= y; _23 *= y;
      _31 *= z; _32 *= z; _33 *= z;
   }

//
// end of license:GPLv3+, back to 'old MAME'-like
//

   // extract the matrix corresponding to the 3x3 rotation part
   void GetRotationPart(Matrix3D& rot)
   {
      rot._11 = _11; rot._12 = _12; rot._13 = _13; rot._14 = 0.0f;
      rot._21 = _21; rot._22 = _22; rot._23 = _23; rot._24 = 0.0f;
      rot._31 = _31; rot._32 = _32; rot._33 = _33; rot._34 = 0.0f;
      rot._41 = rot._42 = rot._43 = 0.0f; rot._44 = 1.0f;
   }

   // generic multiply function for everything that has .x, .y and .z
   template <class VecIn, class VecOut>
   void MultiplyVector(const VecIn& vIn, VecOut& vOut) const
   {
      // Transform it through the current matrix set
      const float xp = _11*vIn.x + _21*vIn.y + _31*vIn.z + _41;
      const float yp = _12*vIn.x + _22*vIn.y + _32*vIn.z + _42;
      const float zp = _13*vIn.x + _23*vIn.y + _33*vIn.z + _43;
      const float wp = _14*vIn.x + _24*vIn.y + _34*vIn.z + _44;

      const float inv_wp = 1.0f / wp;
      vOut.x = xp*inv_wp;
      vOut.y = yp*inv_wp;
      vOut.z = zp*inv_wp;
   }

//
// license:GPLv3+
// Ported at: VisualPinball.Engine/Math/Matrix3D.cs
//

   Vertex3Ds MultiplyVector(const Vertex3Ds &v) const
   {
      // Transform it through the current matrix set
      const float xp = _11*v.x + _21*v.y + _31*v.z + _41;
      const float yp = _12*v.x + _22*v.y + _32*v.z + _42;
      const float zp = _13*v.x + _23*v.y + _33*v.z + _43;
      const float wp = _14*v.x + _24*v.y + _34*v.z + _44;

      const float inv_wp = 1.0f / wp;
      return Vertex3Ds(xp*inv_wp,yp*inv_wp,zp*inv_wp);
   }

   Vertex3Ds MultiplyVectorNoTranslate(const Vertex3Ds &v) const
   {
      // Transform it through the current matrix set
      const float xp = _11*v.x + _21*v.y + _31*v.z;
      const float yp = _12*v.x + _22*v.y + _32*v.z;
      const float zp = _13*v.x + _23*v.y + _33*v.z;

      return Vertex3Ds(xp,yp,zp);
   }

//
// end of license:GPLv3+, back to 'old MAME'-like
//

   template <class VecIn, class VecOut>
   void MultiplyVectorNoTranslate(const VecIn& vIn, VecOut& vOut) const
   {
      // Transform it through the current matrix set
      const float xp = _11*vIn.x + _21*vIn.y + _31*vIn.z;
      const float yp = _12*vIn.x + _22*vIn.y + _32*vIn.z;
      const float zp = _13*vIn.x + _23*vIn.y + _33*vIn.z;

      vOut.x = xp;
      vOut.y = yp;
      vOut.z = zp;
   }

   template <class VecIn, class VecOut>
   void MultiplyVectorNoTranslateNormal(const VecIn& vIn, VecOut& vOut) const
   {
      // Transform it through the current matrix set
      const float xp = _11*vIn.nx + _21*vIn.ny + _31*vIn.nz;
      const float yp = _12*vIn.nx + _22*vIn.ny + _32*vIn.nz;
      const float zp = _13*vIn.nx + _23*vIn.ny + _33*vIn.nz;

      vOut.x = xp;
      vOut.y = yp;
      vOut.z = zp;
   }

   void Invert();

   void Transpose()
   {
      Matrix3D tmp;
      for (int i = 0; i < 4; ++i)
      {
         tmp.m[0][i] = m[i][0];
         tmp.m[1][i] = m[i][1];
         tmp.m[2][i] = m[i][2];
         tmp.m[3][i] = m[i][3];
      }
      *this = tmp;
   }

   void OrthoNormalize()
   {
      vec3 right(_11, _12, _13);
      vec3 up(_21, _22, _23);
      vec3 dir(_31, _32, _33);
      Vec3Normalize(&right, &right);
      Vec3Normalize(&up, &up);
      Vec3Normalize(&dir, &dir);
      _11 = right.x; _12 = right.y; _13 = right.z;
      _21 = up.x; _22 = up.y; _23 = up.z;
      _31 = dir.x; _32 = dir.y; _33 = dir.z;
   }

   Matrix3D operator+(const Matrix3D& m) const
   {
      return Matrix3D(_11 + m._11, _12 + m._12, _13 + m._13, _14 + m._14,
                      _21 + m._21, _22 + m._22, _23 + m._23, _24 + m._24,
                      _31 + m._31, _32 + m._32, _33 + m._33, _34 + m._34,
                      _41 + m._41, _42 + m._42, _43 + m._43, _44 + m._44);
   }
   
   //

   void SetOrthoOffCenterRH(const float l, float r, float b, const float t, float zn, const float zf)
   {
      _11 = 2 / (r - l);
      _12 = 0.0f;
      _13 = 0.0f;
      _14 = 0.0f;

      _21 = 0.0f;
      _22 = 2 / (t - b);
      _23 = 0.0f;
      _24 = 0.0f;

      _31 = 0.0f;
      _32 = 0.0f;
      _33 = 1.0f / (zf - zn);
      _34 = 0.0f;

      _41 = (l + r) / (l - r);
      _42 = (t + b) / (b - t);
      _43 = zn / (zn - zf);
      _44 = 1.0f;
   }

   void SetFrustumRH(float left, float right, float bottom, float top, float znear, float zfar)
   {
      float temp, temp2, temp3, temp4;
      temp = 2.0f * znear;
      temp2 = right - left;
      temp3 = top - bottom;
      temp4 = zfar - znear;
      _11 = temp / temp2;
      _12 = 0.0;
      _13 = 0.0;
      _14 = 0.0;
      _21 = 0.0;
      _22 = temp / temp3;
      _23 = 0.0;
      _24 = 0.0;
      _31 = (right + left) / temp2;
      _32 = (top + bottom) / temp3;
      _33 = (-zfar - znear) / temp4;
      _34 = -1.0f;
      _41 = 0.0;
      _42 = 0.0;
      _43 = (-temp * zfar) / temp4;
      _44 = 0.0;
   }

   void SetPerspectiveRH(float fovyInDegrees, float aspectRatio, float znear, float zfar)
   {
      float ymax, xmax;
      ymax = znear * tanf(ANGTORAD(fovyInDegrees));
      xmax = ymax * aspectRatio;
      SetFrustumRH(-xmax, xmax, -ymax, ymax, znear, zfar);
   }

   void SetLookAtRH(const vec3& eye, const vec3& at, const vec3& up)
   {
      vec3 xaxis, yaxis, zaxis;
      const vec3 e_a = eye - at;
      Vec3Normalize(&zaxis, &e_a);
      Vec3Cross(&xaxis, &up, &zaxis);
      Vec3Normalize(&xaxis, &xaxis);
      Vec3Cross(&yaxis, &zaxis, &xaxis);
      const float dotX = Vec3Dot(&xaxis, &eye);
      const float dotY = Vec3Dot(&yaxis, &eye);
      const float dotZ = Vec3Dot(&zaxis, &eye);
      *this = Matrix3D(xaxis.x, yaxis.x, zaxis.x, 0.f, xaxis.y, yaxis.y, zaxis.y, 0.f, xaxis.z, yaxis.z, zaxis.z, 0.f, -dotX, -dotY, -dotZ, 1.f);
   }

   static Matrix3D MatrixLookAtLH(const vec3& eye, const vec3& at, const vec3& up)
   {
      vec3 xaxis, yaxis, zaxis;
      const vec3 a_e = at - eye;
      Vec3Normalize(&zaxis, &a_e);
      Vec3Cross(&xaxis, &up, &zaxis);
      Vec3Normalize(&xaxis, &xaxis);
      Vec3Cross(&yaxis, &zaxis, &xaxis);
      const float dotX = Vec3Dot(&xaxis, &eye);
      const float dotY = Vec3Dot(&yaxis, &eye);
      const float dotZ = Vec3Dot(&zaxis, &eye);
      return Matrix3D(xaxis.x, yaxis.x, zaxis.x, 0.f, xaxis.y, yaxis.y, zaxis.y, 0.f, xaxis.z, yaxis.z, zaxis.z, 0.f, -dotX, -dotY, -dotZ, 1.f);
   }

   static Matrix3D MatrixPerspectiveFovLH(const float fovy, const float aspect, const float zn, const float zf)
   {
      const float yScale = 1.0f / tanf(fovy * 0.5f);
      const float xScale = yScale / aspect;
      return Matrix3D(xScale, 0.0f, 0.0f, 0.0f, 0.0f, yScale, 0.0f, 0.0f, 0.0f, 0.0f, zf / (zf - zn), 1.0f, 0.0f, 0.0f, -zn * zf / (zf - zn), 0.0f);
   }

   static Matrix3D MatrixPerspectiveOffCenterLH(const float l, const float r, const float b, const float t, const float zn, const float zf)
   {
      return Matrix3D(2.0f * zn / (r - l), 0.0f, 0.0f, 0.0f, 0.0f, 2.0f * zn / (t - b), 0.0f, 0.0f, (l + r) / (l - r), (t + b) / (b - t), zf / (zf - zn), 1.0f, 0.0f, 0.0f, -zn * zf / (zf - zn), 0.0f);
   }

   static Matrix3D MatrixRotationYawPitchRoll(const float yaw, const float pitch, const float roll)
   {
      const float sr = sinf(roll);
      const float cr = cosf(roll);
      const float sp = sinf(pitch);
      const float cp = cosf(pitch);
      const float sy = sinf(yaw);
      const float cy = cosf(yaw);
      //!! This code should be validated!
      return Matrix3D(cr * cy, sr, cr * sy, 0.0f, -sr * cp * sy - sp * sy, cr * cp, sr * cp * sy + sp * cy, 0.0f, -sr * sp * cy - cp * sy, -cr * sp, -sr * sp * sy + cp * cy, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
   }

   template <class T> void TransformVertices(const T* const __restrict rgv, const WORD* const __restrict rgi, const int count, Vertex2D* const __restrict rgvout, const RECT& viewPort) const
   {
      // Get the width and height of the viewport. This is needed to scale the
      // transformed vertices to fit the render window.
      const float rClipWidth = (float)(viewPort.right - viewPort.left) * 0.5f;
      const float rClipHeight = (float)(viewPort.bottom - viewPort.top) * 0.5f;
      const float xoffset = (float)viewPort.left;
      const float yoffset = (float)viewPort.top;

      // Transform each vertex through the current matrix set
      for (int i = 0; i < count; ++i)
      {
         const int l = rgi ? rgi[i] : i;

         // Get the untransformed vertex position
         const float x = rgv[l].x;
         const float y = rgv[l].y;
         const float z = rgv[l].z;

         // Transform it through the current matrix set
         const float xp = _11 * x + _21 * y + _31 * z + _41;
         const float yp = _12 * x + _22 * y + _32 * z + _42;
         const float wp = _14 * x + _24 * y + _34 * z + _44;

         // Finally, scale the vertices to screen coords. This step first
         // "flattens" the coordinates from 3D space to 2D device coordinates,
         // by dividing each coordinate by the wp value. Then, the x- and
         // y-components are transformed from device coords to screen coords.
         // Note 1: device coords range from -1 to +1 in the viewport.
         const float inv_wp = 1.0f / wp;
         const float vTx = (1.0f + xp * inv_wp) * rClipWidth + xoffset;
         const float vTy = (1.0f - yp * inv_wp) * rClipHeight + yoffset;

         rgvout[l].x = vTx;
         rgvout[l].y = vTy;
      }
   }
};
