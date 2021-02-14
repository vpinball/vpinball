#pragma once

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
__forceinline long max(const long x, const long y)
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
__forceinline unsigned long long min(const unsigned long long x, const unsigned long long y)
{
   return x < y ? x : y;
}
__forceinline unsigned long long max(const unsigned long long x, const unsigned long long y)
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
inline void RemoveFromVectorSingle(std::vector<T>& v, const T& val)
{
   typename std::vector<T>::const_iterator it = std::find(v.begin(), v.end(), val);
   if (it != v.end())
      v.erase(it);
}

template <typename T>
inline int FindIndexOf(const std::vector<T>& v, const T& val)
{
   typename std::vector<T>::const_iterator it = std::find(v.begin(), v.end(), val);
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

#define MAXNAMEBUFFER 32
#define MAXSTRING 1024 // usually used for paths,filenames,etc
#define MAXTOKEN (32*4)

#define CCO(x) CComObject<x>

#define SAFE_VECTOR_DELETE(p)   { if(p) { delete [] (p);  (p)=NULL; } }
#define SAFE_DELETE(p)			{ if(p) { delete (p);     (p)=NULL; } }

inline void ref_count_trigger(const ULONG r, const char *file, const int line) // helper for debugging
{
#ifdef DEBUG_REFCOUNT_TRIGGER
   char msg[128];
   sprintf_s(msg, 128, "Ref Count: %u at %s:%d", r, file, line);
   /*g_pvp->*/MessageBox(NULL, msg, "Error", MB_OK | MB_ICONEXCLAMATION);
#endif
}
#define SAFE_RELEASE(p)			{ if(p) { const ULONG rcc = (p)->Release(); if(rcc != 0) ref_count_trigger(rcc, __FILE__, __LINE__); (p)=NULL; } }
#define SAFE_RELEASE_NO_SET(p)	{ if(p) { const ULONG rcc = (p)->Release(); if(rcc != 0) ref_count_trigger(rcc, __FILE__, __LINE__); } }
#define SAFE_RELEASE_NO_CHECK_NO_SET(p)	{ const ULONG rcc = (p)->Release(); if(rcc != 0) ref_count_trigger(rcc, __FILE__, __LINE__); }
#define SAFE_RELEASE_NO_RCC(p)	{ if(p) { (p)->Release(); (p)=NULL; } } // use for releasing things like surfaces gotten from GetSurfaceLevel (that seem to "share" the refcount with the underlying texture)
#define FORCE_RELEASE(p)		{ if(p) { ULONG rcc = 1; while(rcc!=0) {rcc = (p)->Release();} (p)=NULL; } } // release all references until it is 0

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

//These Structs are used for rendering and loading meshes. They must match the VertexDeclaration in RenderDevice.cpp and the loaded meshes.
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
   D3DVALUE nx; // w component if MY_D3DTRANSFORMED_NOTEX2_VERTEX
   D3DVALUE ny; // tex coord (1) if MY_D3DTRANSFORMED_NOTEX2_VERTEX
   D3DVALUE nz; // dto.

   // Texture coordinates (0)
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
   LocalStringW(const int resid);

   WCHAR m_szbuffer[256];
};

#define M_PI 3.1415926535897932384626433832795

#define ANGTORAD(x) ((x) *(float)(M_PI/180.0))
#define RADTOANG(x) ((x) *(float)(180.0/M_PI))

#define VBTOF(x) ((x) ? fTrue : fFalse)
#define VBTOb(x) (!!(x))
#define FTOVB(x) ((x) ? (VARIANT_BOOL)-1 : (VARIANT_BOOL)0)

#if defined(_M_IX86) || defined(_M_X64)
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
#endif

//

__forceinline int float_as_int(const float x) //!! use bit_cast
{
   union {
      float f;
      int i;
   } uc;
   uc.f = x;
   return uc.i;
}

__forceinline float int_as_float(const int i) //!! use bit_cast
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

__forceinline float sgn(const float a)
{
   return (a > 0.f) ? 1.f : ((a < 0.f) ? -1.f : 0.f);
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

template <unsigned int base>
float radical_inverse(unsigned int a) {
    const float invBase = (float)(1. / (double)base);
    unsigned int reversedDigits = 0;
    float invBaseN = 1.f;
    while (a) {
        const unsigned int next  = a / base;
        const unsigned int digit = a - next * base;
        reversedDigits = reversedDigits * base + digit;
        invBaseN *= invBase;
        a = next;
    }
    return (float)((double)reversedDigits * invBaseN);
}

__forceinline float sobol(unsigned int i, unsigned int scramble = 0)
{
   for (unsigned int v = 1u << 31; (i != 0); i >>= 1, v ^= v >> 1) if (i & 1)
      scramble ^= v;

   return (float)scramble * 0.00000000023283064365386962890625f; // /2^32
}

inline void RemoveSpaces(char* const source)
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

__forceinline float vpUnitsToInches(const float value)
{
   return value * 0.0212765f;
}

__forceinline float inchesToVPUnits(const float value)
{
   return value * (float)(1.0/0.0212765);
}

__forceinline float vpUnitsToMillimeters(const float value)
{
   return value * 0.540425f;
}

__forceinline float millimetersToVPUnits(const float value)
{
   return value * (float)(1.0/0.540425);
}

float sz2f(const string& sz);
void f2sz(const float f, string& sz);

void WideStrCopy(const WCHAR *wzin, WCHAR *wzout);
void WideStrNCopy(const WCHAR *wzin, WCHAR *wzout, const DWORD wzoutMaxLen);
int WideStrCmp(const WCHAR *wz1, const WCHAR *wz2);
int WzSzStrCmp(const WCHAR *wz1, const char *sz2);
void WideStrCat(const WCHAR *wzin, WCHAR *wzout);
int WzSzStrnCmp(const WCHAR *wz1, const char *sz2, const int count);

HRESULT OpenURL(const string& szURL);

WCHAR *MakeWide(const string& sz);
char *MakeChar(const WCHAR * const wz);

// in case the incoming string length is >= the maximum char length of the outgoing one, WideCharToMultiByte will not produce a zero terminated string
// this variant always makes sure that the outgoing string is zero terminated
inline int WideCharToMultiByteNull(
    const UINT     CodePage,
    const DWORD    dwFlags,
    LPCWSTR        lpWideCharStr,
    const int      cchWideChar,
    LPSTR          lpMultiByteStr,
    const int      cbMultiByte,
    LPCSTR         lpDefaultChar,
    LPBOOL         lpUsedDefaultChar)
{
    const int res = WideCharToMultiByte(CodePage,dwFlags,lpWideCharStr,cchWideChar,lpMultiByteStr,cbMultiByte,lpDefaultChar,lpUsedDefaultChar);
    if(cbMultiByte > 0 && lpMultiByteStr)
        lpMultiByteStr[cbMultiByte-1] = '\0';
    return res;
}


// in case the incoming string length is >= the maximum wchar length of the outgoing one, MultiByteToWideChar will not produce a zero terminated string
// this variant always makes sure that the outgoing string is zero terminated
inline int MultiByteToWideCharNull(
    const UINT     CodePage,
    const DWORD    dwFlags,
    LPCSTR         lpMultiByteStr,
    const int      cbMultiByte,
    LPWSTR         lpWideCharStr,
    const int      cchWideChar)
{
    const int res = MultiByteToWideChar(CodePage,dwFlags,lpMultiByteStr,cbMultiByte,lpWideCharStr,cchWideChar);
    if(cchWideChar > 0 && lpWideCharStr)
        lpWideCharStr[cchWideChar-1] = L'\0';
    return res;
}


char* replace(const char* const original, const char* const pattern, const char* const replacement);
