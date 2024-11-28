// license:GPLv3+

#pragma once

#include "renderer/typedefs3D.h"
#include <math/vector.h>

#if !defined(ENABLE_DX9)
class alignas(16) D3DMATRIX {
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
};
#endif

// 3x3 matrix for representing linear transformation of 3D vectors
class Matrix3 final
{
public:
   Matrix3() {}
   Matrix3(const float __11, const float __12, const float __13, const float __21, const float __22, const float __23, const float __31, const float __32, const float __33)
   {
      _11 = __11; _12 = __12; _13 = __13;
      _21 = __21; _22 = __22; _23 = __23;
      _31 = __31; _32 = __32; _33 = __33;
   }

// Ported at: VisualPinball.Engine/Math/Matrix2D.cs

   void SkewSymmetric(const Vertex3Ds &pv3D)
   {
      m_d[0][0] = 0.f;     m_d[0][1] = -pv3D.z; m_d[0][2] = pv3D.y;
      m_d[1][0] = pv3D.z;  m_d[1][1] = 0.f;     m_d[1][2] = -pv3D.x;
      m_d[2][0] = -pv3D.y; m_d[2][1] = pv3D.x;  m_d[2][2] = 0.f;
   }

   Matrix3 operator* (const float scalar) const
   {
      return Matrix3{
         m_d[0][0]*scalar,
         m_d[0][1]*scalar,
         m_d[0][2]*scalar,
         m_d[1][0]*scalar,
         m_d[1][1]*scalar,
         m_d[1][2]*scalar,
         m_d[2][0]*scalar,
         m_d[2][1]*scalar,
         m_d[2][2]*scalar
      };
   }

   Matrix3 operator+ (const Matrix3& m) const
   {
      return Matrix3{_11 + m._11, _12 + m._12, _13 + m._13,
                     _21 + m._21, _22 + m._22, _23 + m._23,
                     _31 + m._31, _32 + m._32, _33 + m._33};
   }

   Matrix3 operator* (const Matrix3& m) const
   {
      return Matrix3{
         m_d[0][0] * m.m_d[0][0] + m_d[1][0] * m.m_d[0][1] + m_d[2][0] * m.m_d[0][2],
         m_d[0][0] * m.m_d[1][0] + m_d[1][0] * m.m_d[1][1] + m_d[2][0] * m.m_d[1][2],
         m_d[0][0] * m.m_d[2][0] + m_d[1][0] * m.m_d[2][1] + m_d[2][0] * m.m_d[2][2],
         m_d[0][1] * m.m_d[0][0] + m_d[1][1] * m.m_d[0][1] + m_d[2][1] * m.m_d[0][2],
         m_d[0][1] * m.m_d[1][0] + m_d[1][1] * m.m_d[1][1] + m_d[2][1] * m.m_d[1][2],
         m_d[0][1] * m.m_d[2][0] + m_d[1][1] * m.m_d[2][1] + m_d[2][1] * m.m_d[2][2],
         m_d[0][2] * m.m_d[0][0] + m_d[1][2] * m.m_d[0][1] + m_d[2][2] * m.m_d[0][2],
         m_d[0][2] * m.m_d[1][0] + m_d[1][2] * m.m_d[1][1] + m_d[2][2] * m.m_d[1][2],
         m_d[0][2] * m.m_d[2][0] + m_d[1][2] * m.m_d[2][1] + m_d[2][2] * m.m_d[2][2]
      };
   }

   template <class VecType>
   Vertex3Ds operator* (const VecType& v) const
   {
      return Vertex3Ds{
         m_d[0][0] * v.x + m_d[0][1] * v.y + m_d[0][2] * v.z,
         m_d[1][0] * v.x + m_d[1][1] * v.y + m_d[1][2] * v.z,
         m_d[2][0] * v.x + m_d[2][1] * v.y + m_d[2][2] * v.z};
   }

   // multiply vector with matrix transpose
   template <class VecType>
   Vertex3Ds MulVectorT(const VecType& v) const
   {
      return Vertex3Ds{
         m_d[0][0] * v.x + m_d[1][0] * v.y + m_d[2][0] * v.z,
         m_d[0][1] * v.x + m_d[1][1] * v.y + m_d[2][1] * v.z,
         m_d[0][2] * v.x + m_d[1][2] * v.y + m_d[2][2] * v.z};
   }

   static Matrix3 MulMatricesAndMulScalar(const Matrix3& pmat1, const Matrix3& pmat2, const float scalar)
   {
      Matrix3 m;
      for (int i = 0; i < 3; ++i)
         for (int l = 0; l < 3; ++l)
            m.m_d[i][l] = (pmat1.m_d[i][0] * pmat2.m_d[0][l] +
                           pmat1.m_d[i][1] * pmat2.m_d[1][l] +
                           pmat1.m_d[i][2] * pmat2.m_d[2][l])*scalar;
      return m;
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

   void SetIdentity(const float value = 1.0f)
   {
      m_d[0][0] = m_d[1][1] = m_d[2][2] = value;
      m_d[0][1] = m_d[0][2] =
      m_d[1][0] = m_d[1][2] =
      m_d[2][0] = m_d[2][1] = 0.0f;
   }

// Ported at: VisualPinball.Engine/Math/Matrix2D.cs

   void Transpose()
   {
      Matrix3 tmp;
      for (int i = 0; i < 3; ++i)
      {
         tmp.m_d[0][i] = m_d[i][0];
         tmp.m_d[1][i] = m_d[i][1];
         tmp.m_d[2][i] = m_d[i][2];
      }
      *this = tmp;
   }

   void Invert()
   {
      int ipvt[4] = { 0, 1, 2 };

      for (int k = 0; k < 3; ++k)
      {
         float temp = 0.f;
         int l = k;
         for (int i = k; i < 3; ++i)
         {
            const float d = fabsf(m_d[k][i]);
            if (d > temp)
            {
               temp = d;
               l = i;
            }
         }
         if (l != k)
         {
            const int tmp = ipvt[k];
            ipvt[k] = ipvt[l];
            ipvt[l] = tmp;
            for (int j = 0; j < 3; ++j)
            {
               temp = m_d[j][k];
               m_d[j][k] = m_d[j][l];
               m_d[j][l] = temp;
            }
         }
         const float d = 1.0f / m_d[k][k];
         for (int j = 0; j < k; ++j)
         {
            const float c = m_d[j][k] * d;
            for (int i = 0; i < 3; ++i)
               m_d[j][i] -= m_d[k][i] * c;
            m_d[j][k] = c;
         }
         for (int j = k + 1; j < 3; ++j)
         {
            const float c = m_d[j][k] * d;
            for (int i = 0; i < 3; ++i)
               m_d[j][i] -= m_d[k][i] * c;
            m_d[j][k] = c;
         }
         for (int i = 0; i < 3; ++i)
            m_d[k][i] = -m_d[k][i] * d;
         m_d[k][k] = d;
      }

      Matrix3 mat3D;
      mat3D.m_d[ipvt[0]][0] = m_d[0][0];
      mat3D.m_d[ipvt[0]][1] = m_d[0][1];
      mat3D.m_d[ipvt[0]][2] = m_d[0][2];
      mat3D.m_d[ipvt[1]][0] = m_d[1][0];
      mat3D.m_d[ipvt[1]][1] = m_d[1][1];
      mat3D.m_d[ipvt[1]][2] = m_d[1][2];
      mat3D.m_d[ipvt[2]][0] = m_d[2][0];
      mat3D.m_d[ipvt[2]][1] = m_d[2][1];
      mat3D.m_d[ipvt[2]][2] = m_d[2][2];

      memcpy(&m_d[0][0], &mat3D.m_d[0][0], 3 * 3 * sizeof(float));
   }

   // Create matrix for rotating around an arbitrary vector
   // axis must be normalized
   // this actually rotates by -angle in right-handed coordinates
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
class alignas(16) Matrix3D final : public D3DMATRIX
{
public:
   Matrix3D() {}
   Matrix3D(
      const float __11, const float __12, const float __13, const float __14, const float __21, const float __22, const float __23, const float __24, const float __31, const float __32, const float __33, const float __34, const float __41, const float __42, const float __43, const float __44)
   {
      _11 = __11; _12 = __12; _13 = __13; _14 = __14;
      _21 = __21; _22 = __22; _23 = __23; _24 = __24;
      _31 = __31; _32 = __32; _33 = __33; _34 = __34;
      _41 = __41; _42 = __42; _43 = __43; _44 = __44;
   }
   string ToString() const
   {
      std::stringstream ss;
      ss << std::fixed << std::setw(8) << std::setprecision(2);
      ss << "[ " << _11 << ' ' << _12 << ' ' << _13 << ' ' << _14 << '\n'
         << "  " << _21 << ' ' << _22 << ' ' << _23 << ' ' << _24 << '\n'
         << "  " << _31 << ' ' << _32 << ' ' << _33 << ' ' << _34 << '\n'
         << "  " << _41 << ' ' << _42 << ' ' << _43 << ' ' << _44 << "]\n";
      return ss.str();
   }

// Ported at: VisualPinball.Engine/Math/Matrix3D.cs

#pragma region SetMatrix

   // Math for defining common affine transforms and projection matrices

   void SetIdentity()
   {
      _11 = _22 = _33 = _44 = 1.0f;
      _12 = _13 = _14 = _41 =
      _21 = _23 = _24 = _42 =
      _31 = _32 = _34 = _43 = 0.0f;
   }
   
   void SetRotateX(const float angRad)
   {
      SetIdentity();
      _22 = _33 = cosf(angRad);
      _23 = sinf(angRad);
      _32 = -_23;
   }

   void SetRotateY(const float angRad)
   {
      SetIdentity();
      _11 = _33 = cosf(angRad);
      _31 = sinf(angRad);
      _13 = -_31;
   }

   void SetRotateZ(const float angRad)
   {
      SetIdentity();
      _11 = _22 = cosf(angRad);
      _12 = sinf(angRad);
      _21 = -_12;
   }

   void SetPlaneReflection(const Vertex3Ds& n, const float d)
   {
      // Reflect against reflection plane given by its normal (formula from https://en.wikipedia.org/wiki/Transformation_matrix#Reflection_2)
      SetIdentity();
      _11 = 1.0f - 2.0f * n.x * n.x;
      _12 = -2.0f * n.x * n.y;
      _13 = -2.0f * n.x * n.z;

      _21 = _12;
      _22 = 1.0f - 2.0f * n.y * n.y;
      _23 = -2.0f * n.y * n.z;

      _31 = _13;
      _32 = _23;
      _33 = 1.0f - 2.0f * n.z * n.z;

      _41 = -2.0f * d * n.x;
      _42 = -2.0f * d * n.y;
      _43 = -2.0f * d * n.z;
   }

   void SetOrthoOffCenterRH(const float left, const float right, const float bottom, const float top, const float znear, const float zfar)
   {
      const float r_l = right - left;
      const float t_b = top - bottom;
      _11 = 2.f / r_l;
      _12 = 0.0f;
      _13 = 0.0f;
      _14 = 0.0f;

      _21 = 0.0f;
      _22 = 2.f / t_b;
      _23 = 0.0f;
      _24 = 0.0f;

      _31 = 0.0f;
      _32 = 0.0f;
      _33 = 1.0f / (zfar - znear);
      _34 = 0.0f;

      _41 = (left + right) / -r_l;
      _42 = (top + bottom) / -t_b;
      _43 = znear * -_33;
      _44 = 1.0f;
   }

   void SetPerspectiveFovRH(const float angleLeft, const float angleRight, const float angleDown, const float angleUp, const float nearPlane, const float farPlane)
   {
      if (angleRight >= (float)(M_PI/2.) || angleLeft <= (float)(-M_PI/2.))
         assert(!"Invalid projection specification");
      if (angleUp >= (float)(M_PI/2.) || angleDown <= (float)(-M_PI/2.))
         assert(!"Invalid projection specification");

      const bool infNearPlane = std::isinf(nearPlane);
      const bool infFarPlane = std::isinf(farPlane);

      float l = tanf(angleLeft);
      float r = tanf(angleRight);
      float b = tanf(angleDown);
      float t = tanf(angleUp);
      if (!infNearPlane)
      {
         l *= nearPlane;
         r *= nearPlane;
         b *= nearPlane;
         t *= nearPlane;
      }

      if (nearPlane < 0.f || farPlane < 0.f) {
          assert(!"Invalid projection specification");
      }

      if (infNearPlane || infFarPlane) {
         if (infNearPlane && infFarPlane) {
            assert(!"Invalid projection specification");
         }

         const float reciprocalWidth = 1.0f / (r - l);
         const float reciprocalHeight = 1.0f / (t - b);

         float twoNearZ;
         if (infNearPlane)
         {
            twoNearZ = 2.f;
            _33 = 0.0f;     // far / (near - far) = far / inf = 0
            _43 = farPlane; // near * far / (near - far) = far * (near / (near - far)) = far * (inf / inf) = far
         } else {
            twoNearZ = nearPlane + nearPlane;
            _33 = -1.0f;      // far / (near - far) = inf / -inf = -1
            _43 = -nearPlane; // near * far / (near - far) = near * inf / -inf = -near
         }
         _11 = twoNearZ * reciprocalWidth;
         _12 = 0.0f;
         _13 = 0.0f;
         _14 = 0.0f;

         _21 = 0.0f;
         _22 = twoNearZ * reciprocalHeight;
         _23 = 0.0f;
         _24 = 0.0f;

         _31 = (l + r) * reciprocalWidth;
         _32 = (t + b) * reciprocalHeight;
         _34 = -1.0f;

         _41 = 0.0f;
         _42 = 0.0f;
         _44 = 0.0f;
      } else {
         SetPerspectiveOffCenterRH(l, r, b, t, nearPlane, farPlane);
      }
   }

   void SetPerspectiveOffCenterRH(const float left, const float right, const float bottom, const float top, const float znear, const float zfar)
   {
      const float r_l = right - left;
      const float t_b = top - bottom;
      _11 = 2.0f * znear / r_l;
      _12 = 0.0f;
      _13 = 0.0f;
      _14 = 0.0f;
      _21 = 0.0f;
      _22 = 2.0f * znear / t_b;
      _23 = 0.0f;
      _24 = 0.0f;
      _31 = (right + left) / r_l;
      _32 = (top + bottom) / t_b;
      _33 = zfar / (znear - zfar);
      _34 = -1.0f;
      _41 = 0.0f;
      _42 = 0.0f;
      _43 = znear * _33;
      _44 = 0.0f;
   }

   void SetPerspectiveOffCenterLH(const float left, const float right, const float bottom, const float top, const float znear, const float zfar)
   {
      const float r_l = right - left;
      const float t_b = top - bottom;
      _11 = 2.0f * znear / r_l;
      _12 = 0.0f;
      _13 = 0.0f;
      _14 = 0.0f;
      _21 = 0.0f;
      _22 = 2.0f * znear / t_b;
      _23 = 0.0f;
      _24 = 0.0f;
      _31 = -(right + left) / r_l;
      _32 = -(top + bottom) / t_b;
      _33 = -zfar / (znear - zfar);
      _34 = 1.0f;
      _41 = 0.0f;
      _42 = 0.0f;
      _43 = znear * -_33;
      _44 = 0.0f;
   }

#pragma endregion SetMatrix


#pragma region FactoryConstructors
   ////////////////////////////////////////////////////////////////////////////////
   // Factory methods for building common affine transform and projection matrices

   static Matrix3D MatrixIdentity()
   {
      return Matrix3D{1.0f,0.0f,0.0f,0.0f,0.0f,1.0f,0.0f,0.0f,0.0f,0.0f,1.0f,0.0f,0.0f,0.0f,0.0f,1.0f};
   }

   static Matrix3D MatrixRotateX(const float angRad)
   {
      Matrix3D result;
      result.SetRotateX(angRad);
      return result;
   }

   static Matrix3D MatrixRotateY(const float angRad)
   {
      Matrix3D result;
      result.SetRotateY(angRad);
      return result;
   }

   static Matrix3D MatrixRotateZ(const float angRad)
   {
      Matrix3D result;
      result.SetRotateZ(angRad);
      return result;
   }

   static Matrix3D MatrixScale(float scale)
   {
      return Matrix3D{scale,0.0f,0.0f,0.0f,0.0f,scale,0.0f,0.0f,0.0f,0.0f,scale,0.0f,0.0f,0.0f,0.0f,1.0f};
   }

   static Matrix3D MatrixScale(float sx, float sy, float sz)
   {
      return Matrix3D{sx,0.0f,0.0f,0.0f,0.0f,sy,0.0f,0.0f,0.0f,0.0f,sz,0.0f,0.0f,0.0f,0.0f,1.0f};
   }

   static Matrix3D MatrixTranslate(float x, float y, float z)
   {
      return Matrix3D{1.0f,0.0f,0.0f,0.0f,0.0f,1.0f,0.0f,0.0f,0.0f,0.0f,1.0f,0.0f,x,y,z,1.0f};
   }

   template <class Vec> static Matrix3D MatrixTranslate(const Vec& t)
   {
      return Matrix3D{1.0f,0.0f,0.0f,0.0f,0.0f,1.0f,0.0f,0.0f,0.0f,0.0f,1.0f,0.0f,t.x,t.y,t.z,1.0f};
   }

   static Matrix3D MatrixPlaneReflection(const Vertex3Ds& n, const float d)
   {
      Matrix3D result;
      result.SetPlaneReflection(n, d);
      return result;
   }

   static Matrix3D MatrixLookAtLH(const Vertex3Ds& eye, const Vertex3Ds& at, const Vertex3Ds& up)
   {
      Vertex3Ds zaxis = at - eye;
      zaxis.NormalizeSafe();
      Vertex3Ds xaxis = CrossProduct(up, zaxis);
      xaxis.NormalizeSafe();
      const Vertex3Ds yaxis = CrossProduct(zaxis, xaxis);
      const float dotX = xaxis.Dot(eye);
      const float dotY = yaxis.Dot(eye);
      const float dotZ = zaxis.Dot(eye);
      return Matrix3D{
         xaxis.x, yaxis.x, zaxis.x, 0.f,
         xaxis.y, yaxis.y, zaxis.y, 0.f,
         xaxis.z, yaxis.z, zaxis.z, 0.f,
         -dotX, -dotY, -dotZ, 1.f};
   }

   static Matrix3D MatrixLookAtRH(const Vertex3Ds& eye, const Vertex3Ds& at, const Vertex3Ds& up)
   {
      Vertex3Ds zaxis = eye - at;
      zaxis.NormalizeSafe();
      Vertex3Ds xaxis = CrossProduct(up, zaxis);
      xaxis.NormalizeSafe();
      const Vertex3Ds yaxis = CrossProduct(zaxis, xaxis);
      const float dotX = xaxis.Dot(eye);
      const float dotY = yaxis.Dot(eye);
      const float dotZ = zaxis.Dot(eye);
      return Matrix3D{
         xaxis.x, yaxis.x, zaxis.x, 0.f,
         xaxis.y, yaxis.y, zaxis.y, 0.f,
         xaxis.z, yaxis.z, zaxis.z, 0.f,
         -dotX, -dotY, -dotZ, 1.f};
   }

   static Matrix3D MatrixOrthoOffCenterRH(const float l, const float r, const float b, const float t, const float zn, const float zf)
   {
      Matrix3D result;
      result.SetOrthoOffCenterRH(l, r, b, t, zn, zf);
      return result;
   }

   static Matrix3D MatrixPerspectiveFovLH(const float fovyInDegrees, const float aspectRatio, const float znear, const float zfar)
   {
      const float ymax = znear * tanf(0.5f * ANGTORAD(fovyInDegrees));
      const float xmax = ymax * aspectRatio;
      Matrix3D result;
      result.SetPerspectiveOffCenterLH(-xmax, xmax, -ymax, ymax, znear, zfar);
      return result;
   }

   static Matrix3D MatrixPerspectiveFovRH(const float fovyInDegrees, const float aspectRatio, const float znear, const float zfar)
   {
      const float ymax = znear * tanf(0.5f * ANGTORAD(fovyInDegrees));
      const float xmax = ymax * aspectRatio;
      Matrix3D result;
      result.SetPerspectiveOffCenterRH(-xmax, xmax, -ymax, ymax, znear, zfar);
      return result;
   }

   static Matrix3D MatrixPerspectiveOffCenterLH(const float l, const float r, const float b, const float t, const float zn, const float zf)
   {
      Matrix3D result;
      result.SetPerspectiveOffCenterLH(l, r, b, t, zn, zf);
      return result;
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
      return Matrix3D{
         cr * cy, sr, cr * sy, 0.0f, 
         sy * (-sr * cp - sp), cr * cp, sr * cp * sy + sp * cy, 0.0f, 
         -sr * sp * cy - cp * sy, -cr * sp, -sr * sp * sy + cp * cy, 0.0f, 
         0.0f, 0.0f, 0.0f, 1.0f};
   }

   static Matrix3D MatrixRotate(const float angRad, const Vertex3Ds& axis)
   {
      // From https://en.wikipedia.org/wiki/Rotation_matrix
      const float s = sinf(angRad), c = cosf(angRad);
      const float u_c = 1.f - c;
      const float x = axis.x, y = axis.y, z = axis.z;
      return Matrix3D{
         c + x * x * u_c, x * y * u_c - z * s, x * z * u_c + y * s, 0.0f, 
         y * x * u_c + z * s, c + y * y * u_c, y * z * u_c - x * s, 0.0f, 
         z * x * u_c - y * s, z * y * u_c + x * s, c + z * z * u_c, 0.0f, 
         0.0f, 0.0f, 0.0f, 1.0f};
   }

#pragma endregion FactoryConstructors


#pragma region MatrixOperations

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

   Matrix3D operator+(const Matrix3D& _m) const
   {
#ifdef ENABLE_SSE_OPTIMIZATIONS
      Matrix3D matrixT;
      _mm_storeu_ps(&matrixT._11, _mm_add_ps(_mm_loadu_ps(&_11),_mm_loadu_ps(&_m._11)));
      _mm_storeu_ps(&matrixT._21, _mm_add_ps(_mm_loadu_ps(&_21),_mm_loadu_ps(&_m._21)));
      _mm_storeu_ps(&matrixT._31, _mm_add_ps(_mm_loadu_ps(&_31),_mm_loadu_ps(&_m._31)));
      _mm_storeu_ps(&matrixT._41, _mm_add_ps(_mm_loadu_ps(&_41),_mm_loadu_ps(&_m._41)));
      return matrixT;
#else
      return Matrix3D{_11 + _m._11, _12 + _m._12, _13 + _m._13, _14 + _m._14,
                      _21 + _m._21, _22 + _m._22, _23 + _m._23, _24 + _m._24,
                      _31 + _m._31, _32 + _m._32, _33 + _m._33, _34 + _m._34,
                      _41 + _m._41, _42 + _m._42, _43 + _m._43, _44 + _m._44};
#endif
   }
   
   void Scale(const float x, const float y, const float z)
   {
#ifdef ENABLE_SSE_OPTIMIZATIONS
      _mm_storeu_ps(&_11, _mm_mul_ps(_mm_loadu_ps(&_11),_mm_set_ps(1.0f,x,x,x)));
      _mm_storeu_ps(&_21, _mm_mul_ps(_mm_loadu_ps(&_21),_mm_set_ps(1.0f,y,y,y)));
      _mm_storeu_ps(&_31, _mm_mul_ps(_mm_loadu_ps(&_31),_mm_set_ps(1.0f,z,z,z)));
#else
      _11 *= x; _12 *= x; _13 *= x;
      _21 *= y; _22 *= y; _23 *= y;
      _31 *= z; _32 *= z; _33 *= z;
#endif
   }

   // Normalize the 3 defining vector of an orthogonal matrix
   void OrthoNormalize()
   {
      Vertex3Ds right{_11, _12, _13};
      Vertex3Ds up{_21, _22, _23};
      Vertex3Ds dir{_31, _32, _33};
      right.NormalizeSafe();
      up.NormalizeSafe();
      dir.NormalizeSafe();
      _11 = right.x; _12 = right.y; _13 = right.z;
      _21 = up.x; _22 = up.y; _23 = up.z;
      _31 = dir.x; _32 = dir.y; _33 = dir.z;
   }

#pragma endregion MatrixOperations

#pragma region VectorOperations

   // If matrix defines and orthonormal transformation (like a view or model matrix), these functions return the defining vectors
   Vertex3Ds GetOrthoNormalRight() const { return Vertex3Ds{ _11, _12, _13 }; }
   Vertex3Ds GetOrthoNormalUp() const    { return Vertex3Ds{ _21, _22, _23 }; }
   Vertex3Ds GetOrthoNormalDir() const   { return Vertex3Ds{ _31, _32, _33 }; }
   Vertex3Ds GetOrthoNormalPos() const   { return Vertex3Ds{ _41, _42, _43 }; }

   // extract the matrix corresponding to the 3x3 rotation part
   Matrix3D GetRotationPart() const
   {
      return Matrix3D{_11,_12,_13,0.0f,
                      _21,_22,_23,0.0f,
                      _31,_32,_33,0.0f,
                       0.0f,0.0f,0.0f,1.0f};
   }

   // generic multiply function for everything that has .x, .y and .z
   template <class VecType>
   void MultiplyVector(VecType& v) const
   {
      // Transform it through the current matrix set
      const float xp = _11*v.x + _21*v.y + _31*v.z + _41;
      const float yp = _12*v.x + _22*v.y + _32*v.z + _42;
      const float zp = _13*v.x + _23*v.y + _33*v.z + _43;
      const float wp = _14*v.x + _24*v.y + _34*v.z + _44;

      const float inv_wp = 1.0f / wp;
      v.x = xp*inv_wp;
      v.y = yp*inv_wp;
      v.z = zp*inv_wp;
   }

// Ported at: VisualPinball.Engine/Math/Matrix3D.cs

   template <class VecType>
   Vertex3Ds operator* (const VecType& v) const
   {
      // Transform it through the current matrix set
      const float xp = _11*v.x + _21*v.y + _31*v.z + _41;
      const float yp = _12*v.x + _22*v.y + _32*v.z + _42;
      const float zp = _13*v.x + _23*v.y + _33*v.z + _43;
      const float wp = _14*v.x + _24*v.y + _34*v.z + _44;

      const float inv_wp = 1.0f / wp;
      return Vertex3Ds{xp*inv_wp,yp*inv_wp,zp*inv_wp};
   }

   template <class VecType>
   Vertex3Ds MultiplyVectorNoTranslate(const VecType& v) const
   {
      // Transform it through the current matrix set
      const float xp = _11*v.x + _21*v.y + _31*v.z;
      const float yp = _12*v.x + _22*v.y + _32*v.z;
      const float zp = _13*v.x + _23*v.y + _33*v.z;

      return Vertex3Ds{xp,yp,zp};
   }

   template <class VecType>
   Vertex3Ds MultiplyVectorNoTranslateNormal(const VecType& vIn) const
   {
      // Transform it through the current matrix set
      const float xp = _11*vIn.nx + _21*vIn.ny + _31*vIn.nz;
      const float yp = _12*vIn.nx + _22*vIn.ny + _32*vIn.nz;
      const float zp = _13*vIn.nx + _23*vIn.ny + _33*vIn.nz;

      return Vertex3Ds{xp,yp,zp};
   }

   Vertex3Ds MultiplyVectorNoPerspective(const Vertex3Ds& v) const
   {
      return Vertex3Ds{
         _11 * v.x + _21 * v.y + _31 * v.z + _41,
         _12 * v.x + _22 * v.y + _32 * v.z + _42,
         _13 * v.x + _23 * v.y + _33 * v.z + _43};
   }

   void TransformVertices(const Vertex3D_NoTex2* const __restrict inVerts, Vertex3D_NoTex2* const __restrict outVerts, const int count) const
   {
      for (int i = 0; i < count; ++i)
      {
         const float x = inVerts[i].x;
         const float y = inVerts[i].y;
         const float z = inVerts[i].z;
         const float nx = inVerts[i].nx;
         const float ny = inVerts[i].ny;
         const float nz = inVerts[i].nz;
         outVerts[i].x = _11 * x + _21 * y + _31 * z + _41;
         outVerts[i].y = _12 * x + _22 * y + _32 * z + _42;
         outVerts[i].z = _13 * x + _23 * y + _33 * z + _43;
         outVerts[i].nx = _11 * nx + _21 * ny + _31 * nz;
         outVerts[i].ny = _12 * nx + _22 * ny + _32 * nz;
         outVerts[i].nz = _13 * nx + _23 * ny + _33 * nz;
         outVerts[i].tu = inVerts[i].tu;
         outVerts[i].tv = inVerts[i].tv;
      }
   }

   void TransformPositions(const Vertex3D_NoTex2* const __restrict inVerts, Vertex3D_NoTex2* const __restrict outVerts, const int count) const
   {
      for (int i = 0; i < count; ++i)
      {
         const float x = inVerts[i].x;
         const float y = inVerts[i].y;
         const float z = inVerts[i].z;
         outVerts[i].x = _11 * x + _21 * y + _31 * z + _41;
         outVerts[i].y = _12 * x + _22 * y + _32 * z + _42;
         outVerts[i].z = _13 * x + _23 * y + _33 * z + _43;
      }
   }

   void TransformNormals(const Vertex3D_NoTex2* const __restrict inVerts, Vertex3D_NoTex2* const __restrict outVerts, const int count) const
   {
      for (int i = 0; i < count; ++i)
      {
         const float nx = inVerts[i].nx;
         const float ny = inVerts[i].ny;
         const float nz = inVerts[i].nz;
         outVerts[i].nx = _11 * nx + _21 * ny + _31 * nz;
         outVerts[i].ny = _12 * nx + _22 * ny + _32 * nz;
         outVerts[i].nz = _13 * nx + _23 * ny + _33 * nz;
      }
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
         const float vTx = (1.0f + xp * inv_wp) * rClipWidth  + xoffset;
         const float vTy = (1.0f - yp * inv_wp) * rClipHeight + yoffset;

         rgvout[l].x = vTx;
         rgvout[l].y = vTy;
      }
   }

#pragma endregion VectorOperations

};
