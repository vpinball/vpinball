// license:GPLv3+

#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <cstdint>
#include <algorithm>
#include <charconv>
#include <filesystem>

#include <vector>
using std::vector;

#include <string>
using namespace std::string_literals;
using std::string;
using std::wstring;

#include <cassert>

#ifdef _MSC_VER
#include <intrin.h>
#ifdef _M_ARM64
#include <arm64intr.h>
#endif
#elif (defined(__x86_64__) || defined(__i386__))
#include <x86intrin.h>
#endif

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

#ifdef __STANDALONE__
#if defined(__APPLE__)
#include <TargetConditionals.h>
#endif
#endif

#if defined(__GNUC__) && (__GNUC__ < 12)
#define CONSTEXPR
#else
#define CONSTEXPR constexpr
#endif

#ifdef ENABLE_OPENGL
#ifndef __OPENGLES__
 #include <glad/gl.h>
#else
 #include <glad/gles2.h>
#endif
#endif

#if defined(__GNUC__) && (__GNUC__ < 12)
   #ifdef __STANDALONE__
      #define g_isStandalone true
   #else
      #define g_isStandalone false
   #endif
   #ifdef __LIBVPINBALL__
      #define g_isMobile true
   #else
      #define g_isMobile false
   #endif
   #if defined(__APPLE__) && defined(TARGET_OS_IOS) && TARGET_OS_IOS
      #define g_isIOS true
   #else
      #define g_isIOS false
   #endif
   #if defined(__ANDROID__)
      #define g_isAndroid true
   #else
      #define g_isAndroid false
   #endif
#else
   #ifdef __STANDALONE__
      constexpr bool g_isStandalone = true;
   #else
      constexpr bool g_isStandalone = false;
   #endif
   #ifdef __LIBVPINBALL__
      constexpr bool g_isMobile = true;
   #else
      constexpr bool g_isMobile = false;
   #endif
   #if defined(__APPLE__) && defined(TARGET_OS_IOS) && TARGET_OS_IOS
      constexpr bool g_isIOS = true;
   #else
      constexpr bool g_isIOS = false;
   #endif

   #if defined(__ANDROID__)
      constexpr bool g_isAndroid = true;
   #else
      constexpr bool g_isAndroid = false;
   #endif
#endif

template <typename T>
constexpr __forceinline T min(const T x, const T y)
{
   return x < y ? x : y;
}
template <typename T>
constexpr __forceinline T max(const T x, const T y)
{
   return x < y ? y : x;
}
#if 0 // a bit slower nowadays
template <>
__forceinline float min<float>(const float x, const float y)
{
   return _mm_cvtss_f32(_mm_min_ss(_mm_set_ss(x),_mm_set_ss(y)));
}
template <>
__forceinline float max<float>(const float x, const float y)
{
   return _mm_cvtss_f32(_mm_max_ss(_mm_set_ss(x),_mm_set_ss(y)));
}
template <>
__forceinline double min<double>(const double x, const double y)
{
   return _mm_cvtsd_f64(_mm_min_sd(_mm_set_sd(x),_mm_set_sd(y)));
}
template <>
__forceinline double max<double>(const double x, const double y)
{
   return _mm_cvtsd_f64(_mm_max_sd(_mm_set_sd(x),_mm_set_sd(y)));
}
#endif

template <typename T>
constexpr __forceinline T clamp(const T x, const T mn, const T mx)
{
   return max(min(x,mx),mn);
}

template <typename T>
constexpr __forceinline T lerp(const T x1, const T x2, const float alpha)
{
   return (1.f - alpha) * x1 + alpha * x2;
}

constexpr __forceinline int clamp(const int x, const int mn, const int mx)
{
   if (x < mn) return mn; else if (x > mx) return mx; else return x;
}

template <typename T>
constexpr __forceinline T saturate(const T x)
{
   return max(min(x,T{1}),T{0});
}

template <typename T>
constexpr __forceinline T smoothstep(const T edge0, const T edge1, T x)
{
   if (edge0 == edge1)
      return (x >= edge0) ? T{1} : T{0};
   x = (x - edge0) / (edge1 - edge0);
   x = saturate(x);
   return x * x * (T{3} - T{2} * x);
}

template <typename T>
inline void RemoveFromVectorSingle(vector<T>& v, const T& val)
{
   typename vector<T>::const_iterator it = std::find(v.begin(), v.end(), val);
   if (it != v.end())
      v.erase(it);
}

template <typename T>
inline int FindIndexOf(const vector<T>& v, const T& val)
{
   typename vector<T>::const_iterator it = std::find(v.begin(), v.end(), val);
   if (it != v.end())
      return (int)(it - v.begin());
   else
      return -1;
}

#ifndef __STANDALONE__
#define BOOL int
#endif

#define MAXNAMEBUFFER 32 // material and IScriptable names only
#define MAXSTRING 1024 // usually used for paths,filenames,temporary text buffers,etc

#define CCO(x) CComObject<x>

#define SAFE_VECTOR_DELETE(p)   { delete [] (p);  (p)=nullptr; }
#define SAFE_DELETE(p)          { delete (p);     (p)=nullptr; }

inline void ref_count_trigger(const ULONG r, const char *file, const int line) // helper for debugging
{
#ifdef DEBUG_REFCOUNT_TRIGGER
   /*g_pvp->*/MessageBox(nullptr, ("Ref Count: "+std::to_string(r)+" at "+file+':'+std::to_string(line)).c_str(), "Error", MB_OK | MB_ICONEXCLAMATION);
#endif
}

#define SAFE_RELEASE(p)			{ if(p) { const ULONG rcc = (p)->Release(); if(rcc != 0) ref_count_trigger(rcc, __FILE__, __LINE__); (p)=nullptr; } }
#define SAFE_RELEASE_NO_SET(p)	{ if(p) { const ULONG rcc = (p)->Release(); if(rcc != 0) ref_count_trigger(rcc, __FILE__, __LINE__); } }
#define SAFE_RELEASE_NO_CHECK_NO_SET(p)	{ const ULONG rcc = (p)->Release(); if(rcc != 0) ref_count_trigger(rcc, __FILE__, __LINE__); }
#define SAFE_RELEASE_NO_RCC(p)	{ if(p) { (p)->Release(); (p)=nullptr; } } // use for releasing things like surfaces gotten from GetSurfaceLevel (that seem to "share" the refcount with the underlying texture)
#define FORCE_RELEASE(p)		{ if(p) { ULONG rcc = 1; while(rcc!=0) {rcc = (p)->Release();} (p)=nullptr; } } // release all references until it is 0

#define hrNotImplemented ResultFromScode(E_NOTIMPL)

enum SaveDirtyState
{
   eSaveClean,
   eSaveAutosaved,
   eSaveDirty
};

enum VertexFormat
{
   VF_POS_TEX,
   VF_POS_NORMAL_TEX
};

// These Structs are used for rendering and loading meshes. They must match the VertexDeclaration in RenderDevice.cpp and the loaded meshes.
class Vertex3D_TexelOnly final // for rendering, uses VF_POS_TEX
{
public:
   // Position
   float x;
   float y;
   float z;

   // Texture coordinates
   float tu;
   float tv;
};


// this struct MUST NOT BE CHANGED as the Primitive class uses it for file I/O...
class Vertex3D_NoTex2 final // for rendering, uses VF_POS_NORMAL_TEX
{
public:
   // Position
   float x;
   float y;
   float z;

   // Normals
   float nx;
   float ny;
   float nz;

   // Texture coordinates (0)
   float tu;
   float tv;
};

class LocalString final
{
public:
   LocalString(const int resid);

   char m_szbuffer[256]; // max size would be 4096
};

class LocalStringW final
{
public:
   LocalStringW(const int resid);

   WCHAR m_szbuffer[256]; // max size would be 4096
};

#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795
#endif

#ifndef M_PIf
#define M_PIf 3.1415926535897932384626433832795f
#endif

#define ANGTORAD(x) ((x) * (float)(M_PI/180.0))
#define RADTOANG(x) ((x) * (float)(180.0/M_PI))

#define VBTOF(x) ((x) ? 1 : 0)
#define VBTOb(x) (!!(x))
#define FTOVB(x) ((x) ? (VARIANT_BOOL)-1 : (VARIANT_BOOL)0)

#if defined(_M_IX86) || defined(_M_X64) || defined(_M_AMD64) || defined(__i386__) || defined(__i386) || defined(__i486__) || defined(__i486) || defined(i386) || defined(__ia64__) || defined(__x86_64__)
 #define GET_PLATFORM_CPU_ENUM 0
 #define GET_PLATFORM_CPU "x86"
#else
 #define GET_PLATFORM_CPU_ENUM 1
 #define GET_PLATFORM_CPU "arm"
#endif
static const string platform_cpu[2] = { "x86"s, "arm"s };

#if (INTPTR_MAX == INT32_MAX)
 #define GET_PLATFORM_BITS_ENUM 0
 #define GET_PLATFORM_BITS "32"
#else
 #define GET_PLATFORM_BITS_ENUM 1
 #define GET_PLATFORM_BITS "64"
#endif
static const string platform_bits[2] = { "32"s, "64"s };

#ifdef _MSC_VER
 #define GET_PLATFORM_OS_ENUM 0
 #define GET_PLATFORM_OS "windows"
#elif defined(__ANDROID__) // leave here, as it also defines linux
 #define GET_PLATFORM_OS_ENUM 1
 #define GET_PLATFORM_OS "android"
#elif (defined(__linux) || defined(__linux__))
 #define GET_PLATFORM_OS_ENUM 2
 #define GET_PLATFORM_OS "linux"
#elif defined(__APPLE__)
#if defined(TARGET_OS_IOS) && TARGET_OS_IOS
 #define GET_PLATFORM_OS_ENUM 3
 #define GET_PLATFORM_OS "ios"
#elif defined(TARGET_OS_TV) && TARGET_OS_TV
 #define GET_PLATFORM_OS_ENUM 4
 #define GET_PLATFORM_OS "tvos"
#else
 #define GET_PLATFORM_OS_ENUM 5
 #define GET_PLATFORM_OS "macos"
#endif
#endif
static const string platform_os[6] = { "windows"s, "android"s, "linux"s, "ios"s, "tvos"s, "macos"s };

#if defined(ENABLE_BGFX)
 #define GET_PLATFORM_RENDERER_ENUM 2
 #define GET_PLATFORM_RENDERER "bgfx"
#elif defined(ENABLE_OPENGL)
 #define GET_PLATFORM_RENDERER_ENUM 1
 #define GET_PLATFORM_RENDERER "gl"
#else
 #define GET_PLATFORM_RENDERER_ENUM 0
 #define GET_PLATFORM_RENDERER "dx"
#endif
static const string platform_renderer[3] = { "dx"s, "gl"s, "bgfx"s };

#if !defined(EXT_CAPTURE) && !defined(__STANDALONE__) && defined(ENABLE_OPENGL)
// External captures for VR is a hack, only available for the full windows build using OpenGL
#define EXT_CAPTURE
#endif


#ifdef ENABLE_SSE_OPTIMIZATIONS
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
#if (defined(_M_ARM) || defined(_M_ARM64) || defined(__arm__) || defined(__arm64__) || defined(__aarch64__)) //!! or SSE3
   const __m128 ftemp = _mm_hadd_ps(a, a);
   return _mm_hadd_ps(ftemp, ftemp);
#else
   const __m128 ftemp = _mm_add_ps(a, _mm_movehl_ps(a, a));
   return _mm_add_ss(ftemp, _mm_shuffle_ps(ftemp, ftemp, 1));
#endif
}

__forceinline __m128 sseHorizontalMin(const __m128 &a)
{
   const __m128 ftemp = _mm_min_ps(a, _mm_movehl_ps(a, a));
   return _mm_min_ss(ftemp, _mm_shuffle_ps(ftemp, ftemp, 1));
}

__forceinline __m128 sseHorizontalMax(const __m128 &a)
{
   const __m128 ftemp = _mm_max_ps(a, _mm_movehl_ps(a, a));
   return _mm_max_ss(ftemp, _mm_shuffle_ps(ftemp, ftemp, 1));
}
#endif

//

#ifndef __clang__
  #include <bit>
  #define float_as_int(x) std::bit_cast<int32_t>(x)
  #define float_as_uint(x) std::bit_cast<uint32_t>(x)
  #define half_as_short(x) std::bit_cast<int16_t>(x)
  #define half_as_ushort(x) std::bit_cast<uint16_t>(x)
  #define int_as_float(x) std::bit_cast<float>(x)
  #define uint_as_float(x) std::bit_cast<float>(x)
  #define short_as_half(x) std::bit_cast<_Float16>(x)
  #define ushort_as_half(x) std::bit_cast<_Float16>(x)
#else // for whatever reason apple/clang is special again
  #define float_as_int(x) __builtin_bit_cast(int32_t, x)
  #define float_as_uint(x) __builtin_bit_cast(uint32_t, x)
  #define half_as_short(x) __builtin_bit_cast(int16_t, x)
  #define half_as_ushort(x) __builtin_bit_cast(uint16_t, x)
  #define int_as_float(x) __builtin_bit_cast(float, x)
  #define uint_as_float(x) __builtin_bit_cast(float, x)
  #define short_as_half(x) __builtin_bit_cast(_Float16, x)
  #define ushort_as_half(x) __builtin_bit_cast(_Float16, x)
#endif

constexpr __forceinline bool infNaN(const float a)
{
   return ((float_as_int(a) & 0x7F800000) == 0x7F800000);
}

constexpr __forceinline bool inf(const float a)
{
   return ((float_as_int(a) & 0x7FFFFFFF) == 0x7F800000);
}

constexpr __forceinline bool NaN(const float a)
{
   return (((float_as_int(a) & 0x7F800000) == 0x7F800000) && ((float_as_int(a) & 0x007FFFFF) != 0));
}

constexpr __forceinline bool deNorm(const float a)
{
   return (((float_as_int(a) & 0x7FFFFFFF) < 0x00800000) && (a != 0.0f));
}

constexpr __forceinline bool sign(const float a)
{
   return (float_as_int(a) & 0x80000000) == 0x80000000;
}

constexpr __forceinline float sgn(const float a)
{
   return (a > 0.f) ? 1.f : ((a < 0.f) ? -1.f : 0.f);
}

#if !defined(_MSC_VER) && !defined(_rotl) //!!
#define _rotl(x,y)  (((x) << (y)) | ((x) >> (-(y) & 31)))
#endif

#if !defined(_MSC_VER) && !defined(_rotr) //!!
#define _rotr(x,y)  (((x) >> (y)) | ((x) << (-(y) & 31)))
#endif

__forceinline unsigned int swap_byteorder(unsigned int x)
{
#if defined(__GNUC__) || defined(__clang__)
   return __builtin_bswap32(x);
#elif defined(_MSC_VER)
   return _byteswap_ulong(x);
#else
   x = ((x << 8) & 0xFF00FF00u) | ((x >> 8) & 0xFF00FFu);
   return (x << 16) | (x >> 16);
#endif
}

#if 0
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

inline uint64_t tinymtu(uint64_t state[2]) {
   uint64_t x = (state[0] & TINYMT64_MASK) ^ state[1];
   x ^= x << TINYMT64_SH0;
   x ^= x >> 32;
   x ^= x << 32;
   x ^= x << TINYMT64_SH1;
   const uint64_t mask = -((int64_t)x & 1);
   state[0] = state[1] ^ (mask & TINYMT64_MAT1);
   state[1] = x ^ (mask & (TINYMT64_MAT2 << 32));
#if defined(TINYMT64_LINEARITY_CHECK)
   x = state[0] ^ state[1];
#else
   x = state[0] + state[1];
#endif
   x ^= state[0] >> TINYMT64_SH8;
   return x ^ (-((int64_t)x & 1) & TINYMT64_TMAT);
}

extern uint64_t tinymt64state[2];

__forceinline float rand_mt_01()  { return (float)(tinymtu(tinymt64state) >> (64-24)) * 0.000000059604644775390625f; } // [0..1)
__forceinline float rand_mt_m11() { return (float)((int64_t)tinymtu(tinymt64state) >> (64-25)) * 0.000000059604644775390625f; } // [-1..1)

#else

// via https://cas.ee.ic.ac.uk/people/dt10/research/rngs-gpu-mwc64x.html

extern uint64_t mwc64x_state;

constexpr __forceinline unsigned int mwc64x(uint64_t& s)
{
   constexpr unsigned int m = 4294883355u;
   const unsigned int c = (unsigned int)(s >> 32), x = (unsigned int)s;

   s = x * ((uint64_t)m) + c;
   return x ^ c;
}

__forceinline float rand_mt_01()  { return (float)(mwc64x(mwc64x_state) >> (32-24)) * 0.000000059604644775390625f; } // [0..1)
__forceinline float rand_mt_m11() { return (float)((int)mwc64x(mwc64x_state) >> (32-25)) * 0.000000059604644775390625f; } // [-1..1)
#endif

//

// flip bits on decimal point (bit reversal)/van der Corput/radical inverse
__forceinline float radical_inverse(unsigned int i)
{
#if (defined(_M_ARM) || defined(_M_ARM64) || defined(__arm__) || defined(__arm64__) || defined(__aarch64__)) && defined(_MSC_VER)
   return (float)(__rbit(i) >> 8) * 0.000000059604644775390625f;
#elif (defined(_M_ARM) || defined(_M_ARM64) || defined(__arm__) || defined(__arm64__) || defined(__aarch64__)) && defined(__clang__) //!! gcc does not have an intrinsic yet
   return (float)(__builtin_arm_rbit(i) >> 8) * 0.000000059604644775390625f;
#elif defined(__clang__)
   return (float)(__builtin_bitreverse32(i) >> 8) * 0.000000059604644775390625f;
#else
   /*v = (v << 16) | (v >> 16);
   v = ((v & 0x55555555u) << 1) | ((v & 0xAAAAAAAAu) >> 1);
   v = ((v & 0x33333333u) << 2) | ((v & 0xCCCCCCCCu) >> 2);
   v = ((v & 0x0F0F0F0Fu) << 4) | ((v & 0xF0F0F0F0u) >> 4);
   v = ((v & 0x00FF00FFu) << 8) | ((v & 0xFF00FF00u) >> 8);*/
   /*i = i*65536 | (i >> 16);
   i =    ((i & 0x00ff00ff)*256)  | ((i & 0xff00ff00) >> 8);
   i =    ((i & 0x0f0f0f0f)*16)   | ((i & 0xf0f0f0f0) >> 4);
   i =    ((i & 0x33333333)*4)    | ((i & 0xcccccccc) >> 2);
   i = ((i & 0x55555555)*2) | ((i & 0xaaaaaaaa) >> 1);*/
   /*i = ((i >> 1) & 0x55555555) | ((i & 0x55555555)*2);
   i = ((i >> 2) & 0x33333333) | ((i & 0x33333333)*4);
   i = ((i >> 4) & 0x0f0f0f0f) | ((i & 0x0f0f0f0f)*16);
   i = ((i >> 8) & 0x00ff00ff) | ((i & 0x00ff00ff)*256);
   i = i*65536 | (i >> 16);*/
   i = _rotr(i & 0xaaaaaaaau, 2) | (i & 0x55555555u);
   i = _rotr(i & 0x66666666u, 4) | (i & 0x99999999u);
   i = _rotr(i & 0x1e1e1e1eu, 8) | (i & 0xe1e1e1e1u);
   i = _rotl(i, 7);
   return (float)(swap_byteorder(i) >> 8) * 0.000000059604644775390625f;
#endif
}

template <unsigned int base>
constexpr float radical_inverse(unsigned int a) {
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

constexpr __forceinline float sobol(unsigned int i, unsigned int scramble = 0)
{
   for (unsigned int v = 1u << 31; (i != 0); i >>= 1, v ^= v >> 1) if (i & 1)
      scramble ^= v;

   return (float)(scramble >> 8) * 0.000000059604644775390625f;
}

//

// Conversions to/from VP units (50 VPU = 1.0625 inches which is 1"1/16, the default size of a ball, 1 inch is 2.54cm)
// These value are very slightly off from original values which used a VPU to MM of 0.540425 instead of 0.53975 (result of the following formula)
// So it used to be 0.125% larger which is not noticeable but makes it difficult to have perfect matches when playing between apps
#define MMTOVPU(x) ((x) * (float)(50. / (25.4 * 1.0625)))
#define CMTOVPU(x) ((x) * (float)(50. / (2.54 * 1.0625)))
#define VPUTOMM(x) ((x) * (float)(25.4 * 1.0625 / 50.))
#define VPUTOCM(x) ((x) * (float)(2.54 * 1.0625 / 50.))
#define INCHESTOVPU(x) ((x) * (float)(50. / 1.0625))
#define VPUTOINCHES(x) ((x) * (float)(1.0625 / 50.))

constexpr __forceinline float vpUnitsToInches(const float value) {
   return VPUTOINCHES(value);
   // return value * 0.0212765f;
}

constexpr __forceinline float inchesToVPUnits(const float value)
{
   return INCHESTOVPU(value);
   //return value * (float)(1.0 / 0.0212765);
}

constexpr __forceinline float vpUnitsToMillimeters(const float value)
{
   return VPUTOMM(value);
   //return value * 0.540425f;
}

constexpr __forceinline float millimetersToVPUnits(const float value)
{
   return MMTOVPU(value);
   // return value * (float)(1.0 / 0.540425);
}

string convert_decimal_point_and_trim(string sz, const bool use_locale);

float sz2f(string sz, const bool force_convert_decimal_point = false);
string f2sz(const float f, const bool can_convert_decimal_point = true);

HRESULT OpenURL(const string& szURL);

string SizeToReadable(const size_t bytes);

WCHAR* MakeWide(const char* const sz);
#ifndef MINIMAL_DEF_H
BSTR MakeWideBSTR(const string& sz);
#endif
WCHAR* MakeWide(const string& sz);
char *MakeChar(const WCHAR* const wz);
string MakeString(const wstring& wz);
string MakeString(const WCHAR* const wz);
#ifndef MINIMAL_DEF_H
string MakeString(const BSTR wz);
#endif
wstring MakeWString(const string& sz);
wstring MakeWString(const char* const sz);

// in case the incoming string length is >= the maximum char length of the outgoing one, WideCharToMultiByte will not produce a zero terminated string
// this variant always makes sure that the outgoing string is zero terminated
inline int WideCharToMultiByteNull(
    const uint32_t CodePage,
    const uint32_t dwFlags,
    LPCWSTR        lpWideCharStr,
    const int      cchWideChar,
    char*          lpMultiByteStr,
    const int      cbMultiByte,
    const char*    lpDefaultChar,
    BOOL*          lpUsedDefaultChar)
{
    const int res = WideCharToMultiByte(CodePage,dwFlags,lpWideCharStr,cchWideChar,lpMultiByteStr,cbMultiByte,lpDefaultChar,lpUsedDefaultChar);
    if(cbMultiByte > 0 && lpMultiByteStr)
        lpMultiByteStr[cbMultiByte-1] = '\0';
    return res;
}


// in case the incoming string length is >= the maximum wchar length of the outgoing one, MultiByteToWideChar will not produce a zero terminated string
// this variant always makes sure that the outgoing string is zero terminated
inline int MultiByteToWideCharNull(
    const uint32_t CodePage,
    const uint32_t dwFlags,
    const char*    lpMultiByteStr,
    const int      cbMultiByte,
    LPWSTR         lpWideCharStr,
    const int      cchWideChar)
{
    const int res = MultiByteToWideChar(CodePage,dwFlags,lpMultiByteStr,cbMultiByte,lpWideCharStr,cchWideChar);
    if(cchWideChar > 0 && lpWideCharStr)
        lpWideCharStr[cchWideChar-1] = L'\0';
    return res;
}

//

constexpr inline char cLower(char c)
{
    if (c >= 'A' && c <= 'Z')
        c ^= 32; //ASCII convention
    return c;
}

constexpr inline void szLower(char* pC)
{
    while (*pC)
    {
        if (*pC >= 'A' && *pC <= 'Z')
            *pC ^= 32; //ASCII convention
        pC++;
    }
}

constexpr inline char cUpper(char c)
{
    if (c >= 'a' && c <= 'z')
        c ^= 32; //ASCII convention
    return c;
}

constexpr inline void szUpper(char* pC)
{
    while (*pC)
    {
        if (*pC >= 'a' && *pC <= 'z')
            *pC ^= 32; //ASCII convention
        pC++;
    }
}

CONSTEXPR inline void StrToLower(string& str)
{
   std::ranges::transform(str.begin(), str.end(), str.begin(), cLower);
}

CONSTEXPR inline void StrToUpper(string& str)
{
   std::ranges::transform(str.begin(), str.end(), str.begin(), cUpper);
}

inline bool StrCompareNoCase(const string& strA, const string& strB)
{
   return strA.length() == strB.length()
      && std::equal(strA.begin(), strA.end(), strB.begin(),
         [](char a, char b) { return cLower(a) == cLower(b); });
}

inline bool StrCompareNoCase(const string& strA, const char* const strB)
{
   return strA.length() == strlen(strB)
      && std::equal(strA.begin(), strA.end(), strB,
         [](char a, char b) { return cLower(a) == cLower(b); });
}

CONSTEXPR inline string lowerCase(string input)
{
   StrToLower(input);
   return input;
}

CONSTEXPR inline string upperCase(string input)
{
   StrToUpper(input);
   return input;
}

// Find strB within strA, case-insensitive, returns the position of strB in strA or string::npos if not found
CONSTEXPR inline size_t StrFindNoCase(const string& strA, const string& strB)
{
   if (strA.length() < strB.length())
      return string::npos;

   size_t i = 0;
   for (size_t j = 0; j < strB.length(); ++j)
      if (cLower(strA[i + j]) != cLower(strB[j]))
      {
         ++i;
         if (i > strA.length() - strB.length())
            return string::npos;
         j = 0;
      }
   return i;
}

void SetThreadName(const string& name);

/**
 * @brief Detect whether the program is running on the Wine compatibility layer
 */
bool IsOnWine();

#ifdef _WIN32
bool IsWindowsVistaOr7();
bool IsWindows10_1803orAbove();

template <class T> T GetModulePath(HMODULE hModule) // string or wstring
{
   T path;
   DWORD size = MAX_PATH;
   while (true)
   {
      path.resize(size);
      DWORD length;
      if constexpr (std::is_same_v<T, std::string>)
         length = ::GetModuleFileNameA(hModule, path.data(), size);
      else
         length = ::GetModuleFileNameW(hModule, path.data(), size);
      if (length == 0)
         return {};
      if (length < size)
      {
         path.resize(length); // Trim excess
         return path;
      }
      // length == size could both mean that it just did fit in, or it was truncated, so try again with a bigger buffer
      size *= 2;
   }
}
#define GetExecutablePath() GetModulePath<string>(nullptr)
#define GetExecutablePathW() GetModulePath<wstring>(nullptr)
#endif

vector<uint8_t> read_file(const string& filename, const bool binary = true);
void write_file(const string& filename, const vector<uint8_t>& data, const bool binary = true);
string normalize_path_separators(const string& szPath);
std::filesystem::path find_case_insensitive_file_path(const std::filesystem::path& searchedFile);
std::filesystem::path find_case_insensitive_directory_path(const std::filesystem::path& searchedFile);
string extension_from_path(const string& path);
bool path_has_extension(const string& path, const string& extension);
inline string trim_string(const string& str)
{
   size_t start = 0;
   size_t end = str.length();
   while (start < end && (str[start] == ' ' || str[start] == '\t' || str[start] == '\r' || str[start] == '\n'))
      ++start;
   while (end > start && (str[end - 1] == ' ' || str[end - 1] == '\t' || str[end - 1] == '\r' || str[end - 1] == '\n'))
      --end;
   return str.substr(start, end - start);
}
inline bool try_parse_int(const string& str, int& value)
{
   const string tmp = trim_string(str);
   return (std::from_chars(tmp.c_str(), tmp.c_str() + tmp.length(), value).ec == std::errc{});
}
bool try_parse_float(const string& str, float& value);
bool is_string_numeric(const string& str);
int string_to_int(const string& str, int default_value = 0);
float string_to_float(const string& str, float default_value = 0.0f);
vector<string> parse_csv_line(const string& line);
// copies all characters of src incl. the null-terminator, BUT never more than dest_size-1, always null-terminates
inline void strncpy_s(char* const __restrict dest, const size_t dest_size, const char* const __restrict src)
{
   if (!dest || dest_size == 0)
      return;
   size_t i = 0;
   if (src)
   {
      for (; i < dest_size-1 && src[i] != '\0'; ++i)
         dest[i] = src[i];
      assert(src[i] == '\0');
   }
   dest[i] = '\0';
}
// copies all characters of src incl. the null-terminator, BUT never more than dest_size-1, always null-terminates
inline void wcsncpy_s(WCHAR* const __restrict dest, const size_t dest_size, const WCHAR* const __restrict src)
{
   if (!dest || dest_size == 0)
      return;
   size_t i = 0;
   if (src)
   {
      for (; i < dest_size-1 && src[i] != L'\0'; ++i)
         dest[i] = src[i];
      assert(src[i] == L'\0');
   }
   dest[i] = L'\0';
}
bool string_contains_case_insensitive(const string& str1, const string& str2);
bool string_starts_with_case_insensitive(const string& str, const string& prefix);
string string_replace_all(const string& szStr, const string& szFrom, const string& szTo, const size_t offs = 0);
string string_replace_all(const string& szStr, const string& szFrom, const char szTo, const size_t offs = 0);
string string_replace_all(const string& szStr, const char szFrom, const string& szTo, const size_t offs = 0);
string create_hex_dump(const uint8_t* buffer, size_t size);
#ifdef ENABLE_OPENGL
const char* gl_to_string(GLuint value);
#endif
vector<string> add_line_numbers(const char* src);

#ifndef MINIMAL_DEF_H
bool try_parse_color(const string& str, OLE_COLOR& value);
string color_to_hex(OLE_COLOR color);

#ifdef __STANDALONE__
extern "C" HRESULT external_open_storage(const OLECHAR* pwcsName, IStorage* pstgPriority, DWORD grfMode, SNB snbExclude, DWORD reserved, IStorage** ppstgOpen);
extern "C" HRESULT external_create_object(const WCHAR *progid, IClassFactory* cf, IUnknown* obj);
extern "C" void external_log_info(const char* format, ...);
extern "C" void external_log_debug(const char* format, ...);
extern "C" void external_log_error(const char* format, ...);
#endif
#endif
