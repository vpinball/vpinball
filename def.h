#pragma once

#ifndef __DEF_H__
#define __DEF_H__

#ifdef VBA
using namespace MSAPC;
#endif

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

inline float min(const float x, const float y)
{
   return x < y ? x : y;
}
inline float max(const float x, const float y)
{
   return x < y ? y : x;
}
inline double min(const double x, const double y)
{
	return x < y ? x : y;
}
inline double max(const double x, const double y)
{
	return x < y ? y : x;
}
inline int min(const int x, const int y)
{
   return x < y ? x : y;
}
inline int max(const int x, const int y)
{
   return x < y ? y : x;
}
inline unsigned int min(const unsigned int x, const unsigned int y)
{
   return x < y ? x : y;
}
inline unsigned int max(const unsigned int x, const unsigned int y)
{
   return x < y ? y : x;
}

template <typename T>
T clamp(T x, T min, T max)
{
    if (x < min)
        return min;
    else if (x > max)
        return max;
    else
        return x;
}

template <typename T>
void RemoveFromVector(std::vector<T>& v, const T& val)
{
    v.erase( std::remove( v.begin(), v.end(), val ), v.end() );
}

#define fTrue 1
#define fFalse 0

#define BOOL int

typedef unsigned int    U32;
typedef signed int      S32;
typedef unsigned short  U16;
typedef signed short    S16;
typedef unsigned char   U08;
typedef signed char     S08;
typedef unsigned char    U8;
typedef signed char      S8;
typedef float           F32;
typedef double          F64;
typedef unsigned _int64 U64;
typedef _int64          S64;

#define MAXNAMEBUFFER 33
#define MAXSTRING 1024
#define MAXTOKEN 32*4

#define SZTHISFILE
#define ASSERT(fTest, err)
#define Assert(x) //_ASSERTE(x)
#define FAIL(err)

#define CCO(x) CComObject<x>

#define SAFE_VECTOR_DELETE(p)   { if(p) { delete [] (p);  (p)=NULL; } }
#define SAFE_DELETE(p)			{ if(p) { delete (p);     (p)=NULL; } }
#define SAFE_RELEASE(p)			{ if(p) { (p)->Release(); (p)=NULL; } }

#define hrNotImplemented ResultFromScode(E_NOTIMPL)

enum SaveDirtyState
{
   eSaveClean,
   eSaveAutosaved,
   eSaveDirty
};

#define MY_D3DFVF_TEX					(D3DFVF_XYZ | D3DFVF_TEX1)

#define MY_D3DFVF_NOTEX_VERTEX          (D3DFVF_XYZ    | D3DFVF_NORMAL                    | D3DFVF_TEX0)
#define MY_D3DTRANSFORMED_NOTEX_VERTEX  (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX0)

#define MY_D3DFVF_NOTEX2_VERTEX         (D3DFVF_XYZ    | D3DFVF_NORMAL                    | D3DFVF_TEX1)
#define MY_D3DTRANSFORMED_NOTEX2_VERTEX (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX1)

#define MY_D3DFVF_NOLIGHTING_VERTEX     (D3DFVF_XYZ    | D3DFVF_DIFFUSE | D3DFVF_TEX1)

#define MY_D3DFVF_VERTEX                (D3DFVF_XYZ    | D3DFVF_NORMAL                    | D3DFVF_TEX2)
#define MY_D3DTRANSFORMED_VERTEX        (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_TEX2)


class Vertex3D // for rendering, uses MY_D3DFVF_VERTEX or MY_D3DTRANSFORMED_VERTEX
{
public:
   // Position
   D3DVALUE x; 
   D3DVALUE y; 
   D3DVALUE z;

   // Normals
   union
   {
      D3DVALUE nx;
      D3DVALUE rhw;
   };

   union
   {
      D3DVALUE ny;
      D3DCOLOR color;
   };

   union
   {
      D3DVALUE nz;
      D3DCOLOR specular;
   };

   // Texture coordinates
   D3DVALUE tu;
   D3DVALUE tv;

   D3DVALUE tu2;
   D3DVALUE tv2;
};

class Vertex3D_NoLighting // for rendering, uses MY_D3DFVF_NOLIGHTING_VERTEX
{
public:
   // Position
   D3DVALUE x; 
   D3DVALUE y; 
   D3DVALUE z;

   D3DCOLOR color;

   // Texture coordinates
   D3DVALUE tu;
   D3DVALUE tv;
};

class Vertex3D_NoTex2 // for rendering, uses MY_D3DFVF_NOTEX2_VERTEX or MY_D3DTRANSFORMED_NOTEX2_VERTEX
{
public:
   struct{
      // Position
      D3DVALUE x;
      D3DVALUE y;
      D3DVALUE z;

      // Normals
      union
      {
         D3DVALUE nx;
         D3DVALUE rhw;
      };

      union
      {
         D3DVALUE ny;
         D3DCOLOR color;
      };

      union
      {
         D3DVALUE nz;
         D3DCOLOR specular;
      };

      // Texture coordinates
      D3DVALUE tu;
      D3DVALUE tv;
   };
};

class Vertex3D_NoTex // for rendering, uses MY_D3DFVF_NOTEX_VERTEX or MY_D3DTRANSFORMED_NOTEX_VERTEX
{
public:
   // Position
   D3DVALUE x; 
   D3DVALUE y; 
   D3DVALUE z;

   // Normals
   union
   {
      D3DVALUE nx;
      D3DVALUE rhw;
   };

   union
   {
      D3DVALUE ny;
      D3DCOLOR color;
   };

   union
   {
      D3DVALUE nz;
      D3DCOLOR specular;
   };

   inline void NormalizeNormal()
   {
      const float oneoverlength = 1.0f/sqrtf(nx*nx + ny*ny + nz*nz);
      nx *= oneoverlength;
      ny *= oneoverlength;
      nz *= oneoverlength;
   }
};


class LocalString
{
public:
   LocalString(const int resid);

   char m_szbuffer[256];
};

#define M_PI 3.1415926535897932384626433832795

#define ANGTORAD(x) ((x) *(float)(M_PI/180.0))
#define RADTOANG(x) ((x) *(float)(180.0/M_PI))

#define VBTOF(x) ((x) ? fTrue : fFalse)
#define FTOVB(x) ((x) ? -1 : 0)

const WORD rgi0123[4] = {0,1,2,3};

inline __m128 rcpps(const __m128 &T) //Newton Raphson
{
   const __m128 TRCP = _mm_rcp_ps(T);
   return _mm_sub_ps(_mm_add_ps(TRCP,TRCP),_mm_mul_ps(_mm_mul_ps(TRCP,T),TRCP));
}

inline __m128 rsqrtps(const __m128 &T) //Newton Raphson
{
   const __m128 TRSQRT = _mm_rsqrt_ps(T);
   return _mm_mul_ps(_mm_mul_ps(_mm_set1_ps(0.5f),TRSQRT), _mm_sub_ps(_mm_set1_ps(3.0f),_mm_mul_ps(_mm_mul_ps(TRSQRT,T),TRSQRT)));
}

inline __m128 rsqrtss(const __m128 &T) //Newton Raphson
{
   const __m128 TRSQRT = _mm_rsqrt_ss(T);
   return _mm_mul_ss(_mm_mul_ss(_mm_set_ss(0.5f),TRSQRT), _mm_sub_ss(_mm_set_ss(3.0f),_mm_mul_ss(_mm_mul_ss(TRSQRT,T),TRSQRT)));
}

inline __m128 sseHorizontalAdd(const __m128 &a) // could use dp instruction on SSE4
{
   const __m128 ftemp = _mm_add_ps(a, _mm_movehl_ps(a, a));
   return _mm_add_ss(ftemp,_mm_shuffle_ps(ftemp, ftemp, 1));
}

//

inline int float_as_int(const float x)
{
	union {
		float f;
		int i;
	} uc;
	uc.f = x;
	return uc.i;
}

inline float int_as_float(const int i)
{
   union {
      int i;
      float f;
   } iaf;
   iaf.i = i;
   return iaf.f;
}

inline bool infNaN(const float a)
{
	return ((float_as_int(a)&0x7F800000) == 0x7F800000);
}

inline bool inf(const float a)
{
	return ((float_as_int(a)&0x7FFFFFFF) == 0x7F800000);
}

inline bool NaN(const float a)
{
	return (((float_as_int(a)&0x7F800000) == 0x7F800000) && ((float_as_int(a)&0x007FFFFF) != 0));
}

inline bool deNorm(const float a)
{
    return (((float_as_int(a)&0x7FFFFFFF) < 0x00800000) && (a != 0.0));
}

//
// TinyMT64 for random numbers (much better than rand())
//

#define TINYMT64_SH0 12
#define TINYMT64_SH1 11
#define TINYMT64_SH8 8
#define TINYMT64_MASK 0x7fffffffffffffffull
//#define TINYMT64_LINEARITY_CHECK
#define TINYMT64_MAT1 0xfa051f40ull         // can be configured (lower 32bit only, upper=0)
#define TINYMT64_MAT2 0xffd0fff4ull			// can be configured (lower 32bit only, upper=0)
#define TINYMT64_TMAT 0x58d02ffeffbfffbcull // can be configured (64bit)

inline unsigned long long tinymtu(unsigned long long state[2]) {
   unsigned long long x = (state[0] & TINYMT64_MASK) ^ state[1];
   x ^= x << TINYMT64_SH0;
   x ^= x >> 32;
   x ^= x << 32;
   x ^= x << TINYMT64_SH1;
   const unsigned long long mask = -((long long)x & 1);
   state[0] = state[1] ^ (mask & TINYMT64_MAT1);
   state[1] = x ^ (mask & (TINYMT64_MAT2 << 32));
#if defined(TINYMT64_LINEARITY_CHECK)
   x = state[0] ^ state[1];
#else
   x = state[0] + state[1];
#endif
   x ^= state[0] >> TINYMT64_SH8;
   return x ^ (-((long long)x & 1) & TINYMT64_TMAT);
}

extern unsigned long long tinymt64state[2];

inline float rand_mt_01()  { return int_as_float(0x3F800000u | (unsigned int)(tinymtu(tinymt64state) >> 41)) - 1.0f; }
inline float rand_mt_m11() { return int_as_float(0x3F800000u | (unsigned int)(tinymtu(tinymt64state) >> 41))*2.0f - 3.0f; }

//

float sz2f(char *sz);
void f2sz(const float f, char *sz);

void WideStrCopy(WCHAR *wzin, WCHAR *wzout);
int WideStrCmp(WCHAR *wz1, WCHAR *wz2);
int WzSzStrCmp(WCHAR *wz1, char *sz2);
void WideStrCat(WCHAR *wzin, WCHAR *wzout);
int WzSzStrnCmp(WCHAR *wz1, char *sz2, int count);

HRESULT OpenURL(char *szURL);

WCHAR *MakeWide(char *sz);
char *MakeChar(WCHAR *wz);

#endif/* !__DEF_H__ */
