#include "stdafx.h"
#include "matrix.h"

void Matrix3D::Invert()
{
   int ipvt[4] = { 0, 1, 2, 3 };

   for (int k = 0; k < 4; ++k)
   {
      float temp = 0.f;
      int l = k;
      for (int i = k; i < 4; ++i)
      {
         const float d = fabsf(m[k][i]);
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
         for (int j = 0; j < 4; ++j)
         {
            temp = m[j][k];
            m[j][k] = m[j][l];
            m[j][l] = temp;
         }
      }
      const float d = 1.0f / m[k][k];
      for (int j = 0; j < k; ++j)
      {
         const float c = m[j][k] * d;
         for (int i = 0; i < 4; ++i)
            m[j][i] -= m[k][i] * c;
         m[j][k] = c;
      }
      for (int j = k + 1; j < 4; ++j)
      {
         const float c = m[j][k] * d;
         for (int i = 0; i < 4; ++i)
            m[j][i] -= m[k][i] * c;
         m[j][k] = c;
      }
      for (int i = 0; i < 4; ++i)
         m[k][i] = -m[k][i] * d;
      m[k][k] = d;
   }

   Matrix3D mat3D;
   mat3D.m[ipvt[0]][0] = m[0][0]; mat3D.m[ipvt[0]][1] = m[0][1]; mat3D.m[ipvt[0]][2] = m[0][2]; mat3D.m[ipvt[0]][3] = m[0][3];
   mat3D.m[ipvt[1]][0] = m[1][0]; mat3D.m[ipvt[1]][1] = m[1][1]; mat3D.m[ipvt[1]][2] = m[1][2]; mat3D.m[ipvt[1]][3] = m[1][3];
   mat3D.m[ipvt[2]][0] = m[2][0]; mat3D.m[ipvt[2]][1] = m[2][1]; mat3D.m[ipvt[2]][2] = m[2][2]; mat3D.m[ipvt[2]][3] = m[2][3];
   mat3D.m[ipvt[3]][0] = m[3][0]; mat3D.m[ipvt[3]][1] = m[3][1]; mat3D.m[ipvt[3]][2] = m[3][2]; mat3D.m[ipvt[3]][3] = m[3][3];

   m[0][0] = mat3D.m[0][0]; m[0][1] = mat3D.m[0][1]; m[0][2] = mat3D.m[0][2]; m[0][3] = mat3D.m[0][3];
   m[1][0] = mat3D.m[1][0]; m[1][1] = mat3D.m[1][1]; m[1][2] = mat3D.m[1][2]; m[1][3] = mat3D.m[1][3];
   m[2][0] = mat3D.m[2][0]; m[2][1] = mat3D.m[2][1]; m[2][2] = mat3D.m[2][2]; m[2][3] = mat3D.m[2][3];
   m[3][0] = mat3D.m[3][0]; m[3][1] = mat3D.m[3][1]; m[3][2] = mat3D.m[3][2]; m[3][3] = mat3D.m[3][3];
}

void RotateAround(const Vertex3Ds &pvAxis, Vertex3D_NoTex2 * const pvPoint, const int count, const float angle)
{
   const float rsin = sinf(angle);
   const float rcos = cosf(angle);

   Matrix3 mat;
   mat.RotationAroundAxis(pvAxis, angle);

   for (int i = 0; i < count; ++i)
   {
      const float result[3] = {
         mat.m_d[0][0] * pvPoint[i].x + mat.m_d[0][1] * pvPoint[i].y + mat.m_d[0][2] * pvPoint[i].z,
         mat.m_d[1][0] * pvPoint[i].x + mat.m_d[1][1] * pvPoint[i].y + mat.m_d[1][2] * pvPoint[i].z,
         mat.m_d[2][0] * pvPoint[i].x + mat.m_d[2][1] * pvPoint[i].y + mat.m_d[2][2] * pvPoint[i].z };

      pvPoint[i].x = result[0];
      pvPoint[i].y = result[1];
      pvPoint[i].z = result[2];

      const float resultn[3] = {
         mat.m_d[0][0] * pvPoint[i].nx + mat.m_d[0][1] * pvPoint[i].ny + mat.m_d[0][2] * pvPoint[i].nz,
         mat.m_d[1][0] * pvPoint[i].nx + mat.m_d[1][1] * pvPoint[i].ny + mat.m_d[1][2] * pvPoint[i].nz,
         mat.m_d[2][0] * pvPoint[i].nx + mat.m_d[2][1] * pvPoint[i].ny + mat.m_d[2][2] * pvPoint[i].nz };

      pvPoint[i].nx = resultn[0];
      pvPoint[i].ny = resultn[1];
      pvPoint[i].nz = resultn[2];
   }
}

void RotateAround(const Vertex3Ds &pvAxis, Vertex3Ds * const pvPoint, const int count, const float angle)
{
   const float rsin = sinf(angle);
   const float rcos = cosf(angle);

   Matrix3 mat;
   mat.RotationAroundAxis(pvAxis, angle);

   for (int i = 0; i < count; ++i)
   {
      const float result[3] = {
         mat.m_d[0][0] * pvPoint[i].x + mat.m_d[0][1] * pvPoint[i].y + mat.m_d[0][2] * pvPoint[i].z,
         mat.m_d[1][0] * pvPoint[i].x + mat.m_d[1][1] * pvPoint[i].y + mat.m_d[1][2] * pvPoint[i].z,
         mat.m_d[2][0] * pvPoint[i].x + mat.m_d[2][1] * pvPoint[i].y + mat.m_d[2][2] * pvPoint[i].z };

      pvPoint[i].x = result[0];
      pvPoint[i].y = result[1];
      pvPoint[i].z = result[2];
   }
}

Vertex3Ds RotateAround(const Vertex3Ds &pvAxis, const Vertex2D &pvPoint, const float angle)
{
   const float rsin = sinf(angle);
   const float rcos = cosf(angle);

   // Matrix for rotating around an arbitrary vector

   float matrix[3][2];
   matrix[0][0] = pvAxis.x*pvAxis.x + rcos*(1.0f - pvAxis.x*pvAxis.x);
   matrix[1][0] = pvAxis.x*pvAxis.y*(1.0f - rcos) - pvAxis.z*rsin;
   matrix[2][0] = pvAxis.z*pvAxis.x*(1.0f - rcos) + pvAxis.y*rsin;

   matrix[0][1] = pvAxis.x*pvAxis.y*(1.0f - rcos) + pvAxis.z*rsin;
   matrix[1][1] = pvAxis.y*pvAxis.y + rcos*(1.0f - pvAxis.y*pvAxis.y);
   matrix[2][1] = pvAxis.y*pvAxis.z*(1.0f - rcos) - pvAxis.x*rsin;

   return Vertex3Ds(matrix[0][0] * pvPoint.x + matrix[0][1] * pvPoint.y,
      matrix[1][0] * pvPoint.x + matrix[1][1] * pvPoint.y,
      matrix[2][0] * pvPoint.x + matrix[2][1] * pvPoint.y);
}
