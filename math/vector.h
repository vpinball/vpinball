#pragma once


// 2D vector
class Vertex2D
{
public:
   float x;
   float y;

   Vertex2D() {}
   Vertex2D(const float _x, const float _y) : x(_x), y(_y) {}

   Vertex2D operator+ (const Vertex2D& v) const
   {
       return Vertex2D(x + v.x, y + v.y);
   }
   Vertex2D operator- (const Vertex2D& v) const
   {
       return Vertex2D(x - v.x, y - v.y);
   }
   Vertex2D operator- () const
   {
       return Vertex2D(-x, -y);
   }

   Vertex2D& operator+= (const Vertex2D& v)
   {
       x += v.x;
       y += v.y;
       return *this;
   }
   Vertex2D& operator-= (const Vertex2D& v)
   {
       x -= v.x;
       y -= v.y;
       return *this;
   }

   Vertex2D operator* (float s) const
   {
       return Vertex2D(s*x, s*y);
   }
   friend Vertex2D operator* (float s, const Vertex2D& v)
   {
       return Vertex2D(s*v.x, s*v.y);
   }
   Vertex2D operator/ (float s) const
   {
       return Vertex2D(x/s, y/s);
   }

   Vertex2D& operator*= (float s)
   {
       x *= s;
       y *= s;
       return *this;
   }
   Vertex2D& operator/= (float s)
   {
       x /= s;
       y /= s;
       return *this;
   }

   float Dot(const Vertex2D &pv) const
   {
      return x*pv.x + y*pv.y;
   }

   float LengthSquared() const
   {
      return x*x + y*y;
   }

   float Length() const
   {
      return sqrtf(x*x + y*y);
   }

   void Normalize()
   {
      const float oneoverlength = 1.0f / Length();
      x *= oneoverlength;
      y *= oneoverlength;
   }
};


// 3D vector
class Vertex3Ds
{
public:
   struct {
      float x;
      float y;
      float z;
   };

   Vertex3Ds() {}
   Vertex3Ds(const float _x, const float _y, const float _z) : x(_x), y(_y), z(_z) {}

   void Set(const float a, const float b, const float c) {x=a; y=b; z=c;}

   Vertex3Ds operator+ (const Vertex3Ds& v) const
   {
       return Vertex3Ds(x + v.x, y + v.y, z + v.z);
   }
   Vertex3Ds operator- (const Vertex3Ds& v) const
   {
       return Vertex3Ds(x - v.x, y - v.y, z - v.z);
   }
   Vertex3Ds operator- () const
   {
       return Vertex3Ds(-x, -y, -z);
   }

   Vertex3Ds& operator+= (const Vertex3Ds& v)
   {
       x += v.x;
       y += v.y;
       z += v.z;
       return *this;
   }
   Vertex3Ds& operator-= (const Vertex3Ds& v)
   {
       x -= v.x;
       y -= v.y;
       z -= v.z;
       return *this;
   }

   Vertex3Ds operator* (float s) const
   {
       return Vertex3Ds(s*x, s*y, s*z);
   }
   friend Vertex3Ds operator* (float s, const Vertex3Ds& v)
   {
       return Vertex3Ds(s*v.x, s*v.y, s*v.z);
   }
   Vertex3Ds operator/ (float s) const
   {
       return Vertex3Ds(x/s, y/s, z/s);
   }

   Vertex3Ds& operator*= (float s)
   {
       x *= s;
       y *= s;
       z *= s;
       return *this;
   }
   Vertex3Ds& operator/= (float s)
   {
       x /= s;
       y /= s;
       z /= s;
       return *this;
   }

   void Normalize()
   {
      const float oneoverlength = 1.0f/sqrtf(x*x + y*y + z*z);
      x *= oneoverlength;
      y *= oneoverlength;
      z *= oneoverlength;
   }
   void Normalize(const float scalar)
   {
      const float oneoverlength = scalar/sqrtf(x*x + y*y + z*z);
      x *= oneoverlength;
      y *= oneoverlength;
      z *= oneoverlength;
   }

   float Dot(const Vertex3Ds &pv) const
   {
      return x*pv.x + y*pv.y + z*pv.z;
   }
   float Dot(const Vertex3D &pv) const
   {
      return x*pv.x + y*pv.y + z*pv.z;
   }

   float LengthSquared() const
   {
      return x*x + y*y + z*z;
   }

   float Length() const
   {
      return sqrtf(x*x + y*y + z*z);
   }
};


inline Vertex3Ds CrossProduct(const Vertex3Ds &pv1, const Vertex3Ds &pv2)
{
   return Vertex3Ds(pv1.y * pv2.z - pv1.z * pv2.y,
      pv1.z * pv2.x - pv1.x * pv2.z,
      pv1.x * pv2.y - pv1.y * pv2.x);
}

////////////////////////////////////////////////////////////////////////////////


inline void RotateAround(const Vertex3Ds &pvAxis, Vertex3D_NoTex * const pvPoint, const int count, const float angle)
{
    const float rsin = sinf(angle);
    const float rcos = cosf(angle);

    // Matrix for rotating around an arbitrary vector

    float matrix[3][3];
    matrix[0][0] = pvAxis.x*pvAxis.x + rcos*(1.0f-pvAxis.x*pvAxis.x);
    matrix[1][0] = pvAxis.x*pvAxis.y*(1.0f-rcos) - pvAxis.z*rsin;
    matrix[2][0] = pvAxis.z*pvAxis.x*(1.0f-rcos) + pvAxis.y*rsin;

    matrix[0][1] = pvAxis.x*pvAxis.y*(1.0f-rcos) + pvAxis.z*rsin;
    matrix[1][1] = pvAxis.y*pvAxis.y + rcos*(1.0f-pvAxis.y*pvAxis.y);
    matrix[2][1] = pvAxis.y*pvAxis.z*(1.0f-rcos) - pvAxis.x*rsin;

    matrix[0][2] = pvAxis.z*pvAxis.x*(1.0f-rcos) - pvAxis.y*rsin;
    matrix[1][2] = pvAxis.y*pvAxis.z*(1.0f-rcos) + pvAxis.x*rsin;
    matrix[2][2] = pvAxis.z*pvAxis.z + rcos*(1.0f-pvAxis.z*pvAxis.z);

    for (int i=0; i<count; ++i)
    {
        const float result[3] = {
            matrix[0][0]*pvPoint[i].x + matrix[0][1]*pvPoint[i].y + matrix[0][2]*pvPoint[i].z,
            matrix[1][0]*pvPoint[i].x + matrix[1][1]*pvPoint[i].y + matrix[1][2]*pvPoint[i].z,
            matrix[2][0]*pvPoint[i].x + matrix[2][1]*pvPoint[i].y + matrix[2][2]*pvPoint[i].z};

            pvPoint[i].x = result[0];
            pvPoint[i].y = result[1];
            pvPoint[i].z = result[2];

            const float resultn[3] = {
                matrix[0][0]*pvPoint[i].nx + matrix[0][1]*pvPoint[i].ny + matrix[0][2]*pvPoint[i].nz,
                matrix[1][0]*pvPoint[i].nx + matrix[1][1]*pvPoint[i].ny + matrix[1][2]*pvPoint[i].nz,
                matrix[2][0]*pvPoint[i].nx + matrix[2][1]*pvPoint[i].ny + matrix[2][2]*pvPoint[i].nz};

                pvPoint[i].nx = resultn[0];
                pvPoint[i].ny = resultn[1];
                pvPoint[i].nz = resultn[2];
    }
}

inline void RotateAround(const Vertex3Ds &pvAxis, Vertex3D_NoTex2 * const pvPoint, const int count, const float angle)
{
    const float rsin = sinf(angle);
    const float rcos = cosf(angle);

    // Matrix for rotating around an arbitrary vector

    float matrix[3][3];
    matrix[0][0] = pvAxis.x*pvAxis.x + rcos*(1.0f-pvAxis.x*pvAxis.x);
    matrix[1][0] = pvAxis.x*pvAxis.y*(1.0f-rcos) - pvAxis.z*rsin;
    matrix[2][0] = pvAxis.z*pvAxis.x*(1.0f-rcos) + pvAxis.y*rsin;

    matrix[0][1] = pvAxis.x*pvAxis.y*(1.0f-rcos) + pvAxis.z*rsin;
    matrix[1][1] = pvAxis.y*pvAxis.y + rcos*(1.0f-pvAxis.y*pvAxis.y);
    matrix[2][1] = pvAxis.y*pvAxis.z*(1.0f-rcos) - pvAxis.x*rsin;

    matrix[0][2] = pvAxis.z*pvAxis.x*(1.0f-rcos) - pvAxis.y*rsin;
    matrix[1][2] = pvAxis.y*pvAxis.z*(1.0f-rcos) + pvAxis.x*rsin;
    matrix[2][2] = pvAxis.z*pvAxis.z + rcos*(1.0f-pvAxis.z*pvAxis.z);

    for (int i=0; i<count; ++i)
    {
        const float result[3] = {
            matrix[0][0]*pvPoint[i].x + matrix[0][1]*pvPoint[i].y + matrix[0][2]*pvPoint[i].z,
            matrix[1][0]*pvPoint[i].x + matrix[1][1]*pvPoint[i].y + matrix[1][2]*pvPoint[i].z,
            matrix[2][0]*pvPoint[i].x + matrix[2][1]*pvPoint[i].y + matrix[2][2]*pvPoint[i].z};

            pvPoint[i].x = result[0];
            pvPoint[i].y = result[1];
            pvPoint[i].z = result[2];

            const float resultn[3] = {
                matrix[0][0]*pvPoint[i].nx + matrix[0][1]*pvPoint[i].ny + matrix[0][2]*pvPoint[i].nz,
                matrix[1][0]*pvPoint[i].nx + matrix[1][1]*pvPoint[i].ny + matrix[1][2]*pvPoint[i].nz,
                matrix[2][0]*pvPoint[i].nx + matrix[2][1]*pvPoint[i].ny + matrix[2][2]*pvPoint[i].nz};

                pvPoint[i].nx = resultn[0];
                pvPoint[i].ny = resultn[1];
                pvPoint[i].nz = resultn[2];
    }
}

inline void RotateAround(const Vertex3Ds &pvAxis, Vertex3Ds * const pvPoint, const int count, const float angle)
{
   const float rsin = sinf(angle);
   const float rcos = cosf(angle);

   // Matrix for rotating around an arbitrary vector

   float matrix[3][3];
   matrix[0][0] = pvAxis.x*pvAxis.x + rcos*(1.0f-pvAxis.x*pvAxis.x);
   matrix[1][0] = pvAxis.x*pvAxis.y*(1.0f-rcos) - pvAxis.z*rsin;
   matrix[2][0] = pvAxis.z*pvAxis.x*(1.0f-rcos) + pvAxis.y*rsin;

   matrix[0][1] = pvAxis.x*pvAxis.y*(1.0f-rcos) + pvAxis.z*rsin;
   matrix[1][1] = pvAxis.y*pvAxis.y + rcos*(1.0f-pvAxis.y*pvAxis.y);
   matrix[2][1] = pvAxis.y*pvAxis.z*(1.0f-rcos) - pvAxis.x*rsin;

   matrix[0][2] = pvAxis.z*pvAxis.x*(1.0f-rcos) - pvAxis.y*rsin;
   matrix[1][2] = pvAxis.y*pvAxis.z*(1.0f-rcos) + pvAxis.x*rsin;
   matrix[2][2] = pvAxis.z*pvAxis.z + rcos*(1.0f-pvAxis.z*pvAxis.z);

   for (int i=0; i<count; ++i)
   {
      const float result[3] = {
         matrix[0][0]*pvPoint[i].x + matrix[0][1]*pvPoint[i].y + matrix[0][2]*pvPoint[i].z,
         matrix[1][0]*pvPoint[i].x + matrix[1][1]*pvPoint[i].y + matrix[1][2]*pvPoint[i].z,
         matrix[2][0]*pvPoint[i].x + matrix[2][1]*pvPoint[i].y + matrix[2][2]*pvPoint[i].z};

      pvPoint[i].x = result[0];
      pvPoint[i].y = result[1];
      pvPoint[i].z = result[2];
   }
}

inline Vertex3Ds RotateAround(const Vertex3Ds &pvAxis, const Vertex2D &pvPoint, const float angle)
{
   const float rsin = sinf(angle);
   const float rcos = cosf(angle);

   // Matrix for rotating around an arbitrary vector

   float matrix[3][2];
   matrix[0][0] = pvAxis.x*pvAxis.x + rcos*(1.0f-pvAxis.x*pvAxis.x);
   matrix[1][0] = pvAxis.x*pvAxis.y*(1.0f-rcos) - pvAxis.z*rsin;
   matrix[2][0] = pvAxis.z*pvAxis.x*(1.0f-rcos) + pvAxis.y*rsin;

   matrix[0][1] = pvAxis.x*pvAxis.y*(1.0f-rcos) + pvAxis.z*rsin;
   matrix[1][1] = pvAxis.y*pvAxis.y + rcos*(1.0f-pvAxis.y*pvAxis.y);
   matrix[2][1] = pvAxis.y*pvAxis.z*(1.0f-rcos) - pvAxis.x*rsin;

   return Vertex3Ds(matrix[0][0]*pvPoint.x + matrix[0][1]*pvPoint.y,
      matrix[1][0]*pvPoint.x + matrix[1][1]*pvPoint.y,
      matrix[2][0]*pvPoint.x + matrix[2][1]*pvPoint.y);
}

