//
// minimal set of well understood vec 3d math routines as needed.
// Following/extending hlsl conventions.
// 
//  defines the same set of structs as the alternative file vecmatquat.h.   
//  This vecmatquat_minimal.h version is meant to enable copy-paste just the code snippets that are needed.
//  

#ifdef VECMATQUAT_FULL_H
#error
#endif


#ifndef VECMATQUAT_H
#define VECMATQUAT_H
#define VECMATQUAT_MINIMAL_H

#include <math.h>

class float3 {
public:
	float x, y, z;
	float3(float x, float y, float z) :x(x), y(y), z(z){}
	float3() :x(0), y(0), z(0){}
	float &operator[](int i){ return (&x)[i]; }
	const float &operator[](int i)const { return (&x)[i]; }
};

__forceinline bool   operator==(const float3 &a, const float3 &b) { return a.x == b.x && a.y == b.y && a.z == b.z; }
__forceinline bool   operator!=(const float3 &a, const float3 &b) { return !(a==b); }
__forceinline float3 operator+(const float3 &a, const float3 &b)  { return float3(a.x + b.x, a.y + b.y, a.z + b.z); }
__forceinline float3 operator-(const float3 &v)                   { return float3(-v.x , -v.y, -v.z); }
__forceinline float3 operator-(const float3 &a, const float3 &b)  { return float3(a.x-b.x, a.y-b.y, a.z-b.z); }
__forceinline float3 operator*(const float3 &v, float s)          { return float3(v.x*s, v.y*s, v.z*s); }
__forceinline float3 operator*(float s,const float3 &v)           { return v*s;}
__forceinline float3 operator/(const float3 &v,float s)           { return v * (1.0f/s) ;}
__forceinline float3 operator+=(float3 &a, const float3 &b)       { return a = a + b; }
__forceinline float3 operator-=(float3 &a, const float3 &b)       { return a = a - b; }
__forceinline float3 operator*=(float3 &v, const float &s )       { return v = v * s; }
__forceinline float3 operator/=(float3 &v, const float &s )       { return v = v / s; }
__forceinline float  dot(const float3 &a, const float3 &b)        { return a.x*b.x + a.y*b.y + a.z*b.z; }
__forceinline float3 cross(const float3 &a, const float3 &b)      { return float3(a.y*b.z-a.z*b.y, a.z*b.x-a.x*b.z, a.x*b.y-a.y*b.x); }
__forceinline float  magnitude(const float3 &v)                   { return sqrtf(dot(v, v)); }
__forceinline float3 normalize(const float3 &v)                   { return v / magnitude(v); }
__forceinline float3 cmin(const float3 &a, const float3 &b)       { return float3(std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z)); }
__forceinline float3 cmax(const float3 &a, const float3 &b)       { return float3(std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z)); }
__forceinline float3 vabs(const float3 &v)                        { return float3(std::abs(v.x)     , std::abs(v.y)     , std::abs(v.z)); }

#endif // VECMATQUAT_H
