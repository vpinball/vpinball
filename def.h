#pragma once

#ifndef __DEF_H__
#define __DEF_H__

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

__forceinline float min(const float x, const float y)
{
   return x < y ? x : y;
}
__forceinline float max(const float x, const float y)
{
   return x < y ? y : x;
}
__forceinline double min(const double x, const double y)
{
   return x < y ? x : y;
}
__forceinline double max(const double x, const double y)
{
   return x < y ? y : x;
}
__forceinline int min(const int x, const int y)
{
   return x < y ? x : y;
}
__forceinline int max(const int x, const int y)
{
   return x < y ? y : x;
}
__forceinline unsigned int min(const unsigned int x, const unsigned int y)
{
   return x < y ? x : y;
}
__forceinline unsigned int min(const DWORD x, const DWORD y)
{
   return x < y ? x : y;
}
__forceinline unsigned int max(const unsigned int x, const unsigned int y)
{
   return x < y ? y : x;
}

template <typename T>
__forceinline T clamp(const T x, const T min, const T max)
{
   if (x < min)
      return min;
   else if (x > max)
      return max;
   else
      return x;
}

template <typename T>
__forceinline T saturate(const T x)
{
   if (x < T(0))
      return T(0);
   else if (x > T(1))
      return T(1);
   else
      return x;
}

template <typename T>
inline void RemoveFromVector(std::vector<T>& v, const T& val)
{
   v.erase(std::remove(v.begin(), v.end(), val), v.end());
}

template <typename T>
inline int FindIndexOf(std::vector<T>& v, const T& val)
{
   std::vector<T>::const_iterator it = std::find(v.begin(), v.end(), val);
   if (it != v.end())
      return (int)(it - v.begin());
   else
      return -1;
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

#define ASSERT(fTest, err) //assert(fTest)

#define CCO(x) CComObject<x>

#define SAFE_VECTOR_DELETE(p)   { if(p) { delete [] (p);  (p)=NULL; } }
#define SAFE_DELETE(p)			{ if(p) { delete (p);     (p)=NULL; } }

inline void ref_count_trigger(const ULONG r, const char *file, const int line) // helper for debugging
{
#ifdef DEBUG_REFCOUNT_TRIGGER
   char msg[128];
   sprintf_s(msg, 128, "Ref Count: %u at %s:%d", r, file, line);
   MessageBox(NULL, msg, "Error", MB_OK | MB_ICONEXCLAMATION);
#endif
}
#define SAFE_RELEASE(p)			{ if(p) { const ULONG rcc = (p)->Release(); if(rcc != 0) ref_count_trigger(rcc, __FILE__, __LINE__); (p)=NULL; } }
#define SAFE_RELEASE_NO_SET(p)	{ if(p) { const ULONG rcc = (p)->Release(); if(rcc != 0) ref_count_trigger(rcc, __FILE__, __LINE__); } }
#define SAFE_RELEASE_NO_CHECK_NO_SET(p)	{ const ULONG rcc = (p)->Release(); if(rcc != 0) ref_count_trigger(rcc, __FILE__, __LINE__); }
#define SAFE_RELEASE_NO_RCC(p)	{ if(p) { (p)->Release(); (p)=NULL; } } // use for releasing things like surfaces gotten from GetSurfaceLevel (that seem to "share" the refcount with the underlying texture)

#define hrNotImplemented ResultFromScode(E_NOTIMPL)

enum SaveDirtyState
{
   eSaveClean,
   eSaveAutosaved,
   eSaveDirty
};

#define MY_D3DFVF_TEX					0
#define MY_D3DFVF_NOTEX2_VERTEX         1
#define MY_D3DTRANSFORMED_NOTEX2_VERTEX 2 //!! delete

class Vertex3D_TexelOnly // for rendering, uses MY_D3DFVF_TEX
{
public:
   // Position
   D3DVALUE x;
   D3DVALUE y;
   D3DVALUE z;

   // Texture coordinates
   D3DVALUE tu;
   D3DVALUE tv;
};


// NB: this struct MUST NOT BE CHANGED as the Primitive class uses it for file I/O...
class Vertex3D_NoTex2 // for rendering, uses MY_D3DFVF_NOTEX2_VERTEX or MY_D3DTRANSFORMED_NOTEX2_VERTEX
{
public:
   // Position
   D3DVALUE x;
   D3DVALUE y;
   D3DVALUE z;

   // Normals
   D3DVALUE nx;
   D3DVALUE ny;
   D3DVALUE nz;

   // Texture coordinates
   D3DVALUE tu;
   D3DVALUE tv;
};

class LocalString
{
public:
   LocalString(const int resid);

   char m_szbuffer[256];
};

class LocalStringW
{
public:
   LocalStringW(int resid);

   WCHAR str[256];
};

#define M_PI 3.1415926535897932384626433832795

#define ANGTORAD(x) ((x) *(float)(M_PI/180.0))
#define RADTOANG(x) ((x) *(float)(180.0/M_PI))

#define VBTOF(x) ((x) ? fTrue : fFalse)
#define FTOVB(x) ((x) ? -1 : 0)

__forceinline __m128 rcpps(const __m128 &T) //Newton Raphson
{
   const __m128 TRCP = _mm_rcp_ps(T);
   return _mm_sub_ps(_mm_add_ps(TRCP, TRCP), _mm_mul_ps(_mm_mul_ps(TRCP, T), TRCP));
}

__forceinline __m128 rsqrtps(const __m128 &T) //Newton Raphson
{
   const __m128 TRSQRT = _mm_rsqrt_ps(T);
   return _mm_mul_ps(_mm_mul_ps(_mm_set1_ps(0.5f), TRSQRT), _mm_sub_ps(_mm_set1_ps(3.0f), _mm_mul_ps(_mm_mul_ps(TRSQRT, T), TRSQRT)));
}

__forceinline __m128 rsqrtss(const __m128 &T) //Newton Raphson
{
   const __m128 TRSQRT = _mm_rsqrt_ss(T);
   return _mm_mul_ss(_mm_mul_ss(_mm_set_ss(0.5f), TRSQRT), _mm_sub_ss(_mm_set_ss(3.0f), _mm_mul_ss(_mm_mul_ss(TRSQRT, T), TRSQRT)));
}

__forceinline __m128 sseHorizontalAdd(const __m128 &a) // could use dp instruction on SSE4
{
   const __m128 ftemp = _mm_add_ps(a, _mm_movehl_ps(a, a));
   return _mm_add_ss(ftemp, _mm_shuffle_ps(ftemp, ftemp, 1));
}

//

__forceinline int float_as_int(const float x)
{
   union {
      float f;
      int i;
   } uc;
   uc.f = x;
   return uc.i;
}

__forceinline float int_as_float(const int i)
{
   union {
      int i;
      float f;
   } iaf;
   iaf.i = i;
   return iaf.f;
}

__forceinline bool infNaN(const float a)
{
   return ((float_as_int(a) & 0x7F800000) == 0x7F800000);
}

__forceinline bool inf(const float a)
{
   return ((float_as_int(a) & 0x7FFFFFFF) == 0x7F800000);
}

__forceinline bool NaN(const float a)
{
   return (((float_as_int(a) & 0x7F800000) == 0x7F800000) && ((float_as_int(a) & 0x007FFFFF) != 0));
}

__forceinline bool deNorm(const float a)
{
   return (((float_as_int(a) & 0x7FFFFFFF) < 0x00800000) && (a != 0.0));
}

__forceinline bool sign(const float a)
{
   return (float_as_int(a) & 0x80000000) == 0x80000000;
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

__forceinline float rand_mt_01()  { return int_as_float(0x3F800000u | (unsigned int)(tinymtu(tinymt64state) >> 41)) - 1.0f; }
__forceinline float rand_mt_m11() { return int_as_float(0x3F800000u | (unsigned int)(tinymtu(tinymt64state) >> 41))*2.0f - 3.0f; }

//

// flip bits on decimal point (bit reversal)/van der Corput/radical inverse
__forceinline float radical_inverse(unsigned int v)
{
   v = (v << 16) | (v >> 16);
   v = ((v & 0x55555555u) << 1) | ((v & 0xAAAAAAAAu) >> 1);
   v = ((v & 0x33333333u) << 2) | ((v & 0xCCCCCCCCu) >> 2);
   v = ((v & 0x0F0F0F0Fu) << 4) | ((v & 0xF0F0F0F0u) >> 4);
   v = ((v & 0x00FF00FFu) << 8) | ((v & 0xFF00FF00u) >> 8);
   return (float)v * 0.00000000023283064365386962890625f; // /2^32
}

__forceinline float sobol(unsigned int i, unsigned int scramble = 0)
{
   for (unsigned int v = 1u << 31; (i != 0); i >>= 1, v ^= v >> 1) if (i & 1)
      scramble ^= v;

   return (float)scramble * 0.00000000023283064365386962890625f; // /2^32
}

inline void RemoveSpaces(char* source)
{
   char* i = source;
   char* j = source;
   while (*j != 0)
   {
      *i = *j++;
      if (!isspace(*i))
         i++;
   }
   *i = '\0';
}

//

float sz2f(char *sz);
void f2sz(const float f, char *sz);

void WideStrCopy(WCHAR *wzin, WCHAR *wzout);
void WideStrNCopy(WCHAR *wzin, WCHAR *wzout, const DWORD wzoutMaxLen);
int WideStrCmp(WCHAR *wz1, WCHAR *wz2);
int WzSzStrCmp(WCHAR *wz1, char *sz2);
void WideStrCat(WCHAR *wzin, WCHAR *wzout);
int WzSzStrnCmp(WCHAR *wz1, char *sz2, int count);

HRESULT OpenURL(char *szURL);

WCHAR *MakeWide(char *sz);
char *MakeChar(WCHAR *wz);

#endif/* !__DEF_H__ */
