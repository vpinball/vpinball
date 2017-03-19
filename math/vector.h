#pragma once


// 2D vector
class Vertex2D
{
public:
   float x;
   float y;

   Vertex2D() {}
   Vertex2D(const float _x, const float _y) : x(_x), y(_y) {}

   void Set(float a, float b) { x=a; y=b; }
   void SetZero()       { Set(0,0); }

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

   void NormalizeSafe()
   {
       if (!IsZero())
           Normalize();
   }

   bool IsZero() const
   {
       return x == 0 && y == 0;
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
   void SetZero()       { Set(0,0,0); }

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

   void NormalizeSafe()
   {
       if (!IsZero())
           Normalize();
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

   bool IsZero() const
   {
       return x == 0 && y == 0 && z == 0;
   }
};


inline Vertex3Ds CrossProduct(const Vertex3Ds &pv1, const Vertex3Ds &pv2)
{
   return Vertex3Ds(pv1.y * pv2.z - pv1.z * pv2.y,
      pv1.z * pv2.x - pv1.x * pv2.z,
      pv1.x * pv2.y - pv1.y * pv2.x);
}

////////////////////////////////////////////////////////////////////////////////


void RotateAround(const Vertex3Ds &pvAxis, Vertex3D_NoTex * const pvPoint, int count, float angle);
void RotateAround(const Vertex3Ds &pvAxis, Vertex3D_NoTex2 * const pvPoint, int count, float angle);
void RotateAround(const Vertex3Ds &pvAxis, Vertex3Ds * const pvPoint, int count, float angle);
Vertex3Ds RotateAround(const Vertex3Ds &pvAxis, const Vertex2D &pvPoint, float angle);

