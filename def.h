#pragma once

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

template <typename T>
__forceinline T min(const T x, const T y)
{
   return x < y ? x : y;
}
template <typename T>
__forceinline T max(const T x, const T y)
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
__forceinline T clamp(const T x, const T mn, const T mx)
{
   return max(min(x,mx),mn);
}

template <typename T>
__forceinline T lerp(const T x1, const T x2, const float alpha)
{
   return (1.f - alpha) * x1 + alpha * x2;
}

__forceinline int clamp(const int x, const int mn, const int mx)
{
   if (x < mn) return mn; else if (x > mx) return mx; else return x;
}

template <typename T>
__forceinline T saturate(const T x)
{
   return max(min(x,T(1)),T(0));
}

template <typename T>
inline void RemoveFromVector(vector<T>& v, const T& val)
{
   v.erase(std::remove(v.begin(), v.end(), val), v.end());
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

#define fTrue 1
#define fFalse 0

#ifndef __STANDALONE__
#define BOOL int
#endif

typedef uint32_t        U32;
typedef int32_t         S32;
typedef uint16_t        U16;
typedef int16_t         S16;
typedef uint8_t         U08;
typedef int8_t          S08;
typedef uint8_t         U8;
typedef int8_t          S8;
typedef float           F32;
typedef double          F64;
typedef uint64_t        U64;
typedef int64_t         S64;

#define MAXNAMEBUFFER 32
#define MAXSTRING 1024 // usually used for paths,filenames,etc
#define MAXTOKEN (32*4)

#define CCO(x) CComObject<x>

#define SAFE_VECTOR_DELETE(p)   { if(p) { delete [] (p);  (p)=nullptr; } }
#define SAFE_DELETE(p)          { if(p) { delete (p);     (p)=nullptr; } }

inline void ref_count_trigger(const ULONG r, const char *file, const int line) // helper for debugging
{
#ifdef DEBUG_REFCOUNT_TRIGGER
   char msg[128];
   sprintf_s(msg, sizeof(msg), "Ref Count: %u at %s:%d", r, file, line);
   /*g_pvp->*/MessageBox(nullptr, msg, "Error", MB_OK | MB_ICONEXCLAMATION);
#endif
}

#define SAFE_RELEASE(p)			{ if(p) { const ULONG rcc = (p)->Release(); if(rcc != 0) ref_count_trigger(rcc, __FILE__, __LINE__); (p)=nullptr; } }
#define SAFE_RELEASE_NO_SET(p)	{ if(p) { const ULONG rcc = (p)->Release(); if(rcc != 0) ref_count_trigger(rcc, __FILE__, __LINE__); } }
#define SAFE_RELEASE_NO_CHECK_NO_SET(p)	{ const ULONG rcc = (p)->Release(); if(rcc != 0) ref_count_trigger(rcc, __FILE__, __LINE__); }
#define SAFE_RELEASE_NO_RCC(p)	{ if(p) { (p)->Release(); (p)=nullptr; } } // use for releasing things like surfaces gotten from GetSurfaceLevel (that seem to "share" the refcount with the underlying texture)
#define FORCE_RELEASE(p)		{ if(p) { ULONG rcc = 1; while(rcc!=0) {rcc = (p)->Release();} (p)=nullptr; } } // release all references until it is 0

#define SAFE_PINSOUND_RELEASE(p) { if(p) { const ULONG rcc = (p)->Release(); if(rcc != 0) ref_count_trigger(rcc, __FILE__, __LINE__); (p)=nullptr; } }

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

//These Structs are used for rendering and loading meshes. They must match the VertexDeclaration in RenderDevice.cpp and the loaded meshes.
class Vertex3D_TexelOnly final // for rendering, uses VF_POS_TEX
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
class Vertex3D_NoTex2 final // for rendering, uses VF_POS_NORMAL_TEX
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

   // Texture coordinates (0)
   D3DVALUE tu;
   D3DVALUE tv;
};

class LocalString final
{
public:
   LocalString(const int resid);

   char m_szbuffer[256] = { 0 };
};

class LocalStringW final
{
public:
   LocalStringW(const int resid);

   WCHAR m_szbuffer[256] = { 0 };
};

#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795
#endif

#ifndef M_PIf
#define M_PIf 3.1415926535897932384626433832795f
#endif

#define ANGTORAD(x) ((x) * (float)(M_PI/180.0))
#define RADTOANG(x) ((x) * (float)(180.0/M_PI))

#define VBTOF(x) ((x) ? fTrue : fFalse)
#define VBTOb(x) (!!(x))
#define FTOVB(x) ((x) ? (VARIANT_BOOL)-1 : (VARIANT_BOOL)0)

static const string platform_cpu[2] = { "x86"s, "arm"s };
static const string platform_bits[2] = { "32"s, "64"s };
static const string platform_os[6] = { "windows"s, "linux"s, "ios"s, "tvos"s, "macos"s, "android"s };
static const string platform_renderer[2] = { "dx"s, "gl"s }; // gles necessary, too?

#if defined(_M_IX86) || defined(_M_X64) || defined(_M_AMD64) || defined(__i386__) || defined(__i386) || defined(__i486__) || defined(__i486) || defined(i386) || defined(__ia64__) || defined(__x86_64__)
 #define GET_PLATFORM_CPU_ENUM 0
 #define GET_PLATFORM_CPU "x86"
#else
 #define GET_PLATFORM_CPU_ENUM 1
 #define GET_PLATFORM_CPU "arm"
#endif

#if (INTPTR_MAX == INT32_MAX)
 #define GET_PLATFORM_BITS_ENUM 0
 #define GET_PLATFORM_BITS "32"
#else
 #define GET_PLATFORM_BITS_ENUM 1
 #define GET_PLATFORM_BITS "64"
#endif

#ifdef _MSC_VER
 #define GET_PLATFORM_OS_ENUM 0
 #define GET_PLATFORM_OS "windows"
#elif (defined(__linux) || defined(__linux__))
 #define GET_PLATFORM_OS_ENUM 1
 #define GET_PLATFORM_OS "linux"
#elif defined(__APPLE__)
#if defined(TARGET_OS_IOS) && TARGET_OS_IOS
 #define GET_PLATFORM_OS_ENUM 2
 #define GET_PLATFORM_OS "ios"
#elif defined(TARGET_OS_TV) && TARGET_OS_TV
 #define GET_PLATFORM_OS_ENUM 3
 #define GET_PLATFORM_OS "tvos"
#else
 #define GET_PLATFORM_OS_ENUM 4
 #define GET_PLATFORM_OS "macos"
#endif
#elif defined(__ANDROID__)
 #define GET_PLATFORM_OS_ENUM 5
 #define GET_PLATFORM_OS "android" 
#endif

#ifdef ENABLE_SDL
 #define GET_PLATFORM_RENDERER_ENUM 1
#else
 #define GET_PLATFORM_RENDERER_ENUM 0
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

#if __cplusplus >= 202002L
 #ifndef __clang__
  #include <bit>
  #define float_as_int(x) std::bit_cast<int>(x)
  #define float_as_uint(x) std::bit_cast<unsigned int>(x)
  #define half_as_short(x) std::bit_cast<short>(x)
  #define half_as_ushort(x) std::bit_cast<unsigned short>(x)
  #define int_as_float(x) std::bit_cast<float>(x)
  #define uint_as_float(x) std::bit_cast<float>(x)
  #define short_as_half(x) std::bit_cast<_Float16>(x)
  #define ushort_as_half(x) std::bit_cast<_Float16>(x)
 #else // for whatever reason apple/clang is special again
  #define float_as_int(x) __builtin_bit_cast(int, x)
  #define float_as_uint(x) __builtin_bit_cast(unsigned int, x)
  #define half_as_short(x) __builtin_bit_cast(short, x)
  #define half_as_ushort(x) __builtin_bit_cast(unsigned short, x)
  #define int_as_float(x) __builtin_bit_cast(float, x)
  #define uint_as_float(x) __builtin_bit_cast(float, x)
  #define short_as_half(x) __builtin_bit_cast(_Float16, x)
  #define ushort_as_half(x) __builtin_bit_cast(_Float16, x)
 #endif
#else
__forceinline int float_as_int(const float x)
{
   union {
      float f;
      int i;
   } uc;
   uc.f = x;
   return uc.i;
}

__forceinline unsigned int float_as_uint(const float x)
{
   union {
      float f;
      unsigned int i;
   } uc;
   uc.f = x;
   return uc.i;
}

#if defined(__GNUC__) || defined(__clang__)
__forceinline short half_as_short(const _Float16 x)
{
   union
   {
      _Float16 f;
      short i;
   } uc;
   uc.f = x;
   return uc.i;
}

__forceinline unsigned short half_as_ushort(const _Float16 x)
{
   union
   {
      _Float16 f;
      unsigned short i;
   } uc;
   uc.f = x;
   return uc.i;
}
#endif

__forceinline float int_as_float(const int i)
{
   union {
      int i;
      float f;
   } iaf;
   iaf.i = i;
   return iaf.f;
}

__forceinline float uint_as_float(const unsigned int i)
{
   union {
      unsigned int i;
      float f;
   } iaf;
   iaf.i = i;
   return iaf.f;
}

#if defined(__GNUC__) || defined(__clang__)
__forceinline _Float16 short_as_half(const short i)
{
   union {
      short i;
      _Float16 f;
   } iaf;
   iaf.i = i;
   return iaf.f;
}

__forceinline _Float16 ushort_as_half(const unsigned short i)
{
   union {
      unsigned short i;
      _Float16 f;
   } iaf;
   iaf.i = i;
   return iaf.f;
}
#endif
#endif

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
   return (((float_as_int(a) & 0x7FFFFFFF) < 0x00800000) && (a != 0.0f));
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

__forceinline float rand_mt_01()  { return (float)(tinymtu(tinymt64state) >> (64-24)) * 0.000000059604644775390625f; } // [0..1)
__forceinline float rand_mt_m11() { return (float)((int64_t)tinymtu(tinymt64state) >> (64-25)) * 0.000000059604644775390625f; } // [-1..1)

//

// flip bits on decimal point (bit reversal)/van der Corput/radical inverse
__forceinline float radical_inverse(unsigned int v)
{
   v = (v << 16) | (v >> 16);
   v = ((v & 0x55555555u) << 1) | ((v & 0xAAAAAAAAu) >> 1);
   v = ((v & 0x33333333u) << 2) | ((v & 0xCCCCCCCCu) >> 2);
   v = ((v & 0x0F0F0F0Fu) << 4) | ((v & 0xF0F0F0F0u) >> 4);
   v = ((v & 0x00FF00FFu) << 8) | ((v & 0xFF00FF00u) >> 8);
   return (float)(v >> 8) * 0.000000059604644775390625f;
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

   return (float)(scramble >> 8) * 0.000000059604644775390625f;
}

inline void RemoveSpaces(char* const source)
{
   char* i = source;
   char* j = source;
   while (*j != '\0')
   {
      *i = *j++;
      if (!isspace(*i))
         i++;
   }
   *i = '\0';
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

float sz2f(const string& sz);
string f2sz(const float f);

void WideStrNCopy(const WCHAR *wzin, WCHAR *wzout, const DWORD wzoutMaxLen);
int WideStrCmp(const WCHAR *wz1, const WCHAR *wz2);
void WideStrCat(const WCHAR *wzin, WCHAR *wzout, const DWORD wzoutMaxLen);
int WzSzStrCmp(const WCHAR *wz1, const char *sz2);
int WzSzStrNCmp(const WCHAR *wz1, const char *sz2, const DWORD maxComparisonLen);

HRESULT OpenURL(const string& szURL);

WCHAR *MakeWide(const string& sz);
char *MakeChar(const WCHAR *const wz);
string MakeString(const wstring &wz);
wstring MakeWString(const string &wz);

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

inline void StrToLower(string& str)
{
   std::transform(str.begin(), str.end(), str.begin(), tolower);
}

inline bool StrCompareNoCase(const string& strA, const string& strB)
{
   return strA.size() == strB.size()
      && std::equal(strA.begin(), strA.end(), strB.begin(), 
         [](char a, char b) { return std::tolower(static_cast<unsigned char>(a)) == std::tolower(static_cast<unsigned char>(b)); });
}

char* replace(const char* const original, const char* const pattern, const char* const replacement);

/**
 * @brief Detect whether the program is running on the Wine compatibility layer
 */
bool IsOnWine();

#ifdef __STANDALONE__
#include "typedefs3D.h"

#include <filesystem>
#include <fstream>

void copy_folder(const string& srcPath, const string& dstPath);
vector<string> find_files_by_extension(const string& directoryPath, const string& extension);
string find_path_case_insensitive(const string& szPath);
string find_directory_case_insensitive(const std::string& szParentPath, const std::string& szDirName);
string extension_from_path(const string& path);
string normalize_path_separators(const string& szPath);
bool path_has_extension(const string& path, const string& extension);
bool try_parse_int(const string& str, int& value);
bool try_parse_float(const string& str, float& value);
bool try_parse_color(const string& str, OLE_COLOR& value);
bool is_string_numeric(const string& str);
int string_to_int(const string& str, int default_value = 0);
float string_to_float(const string& str, float default_value = 0.0f);
string trim_string(const string& str);
vector<string> parse_csv_line(const string& line);
string color_to_hex(OLE_COLOR color);
bool string_contains_case_insensitive(const string& str1, const string& str2);
bool string_compare_case_insensitive(const string& str1, const string& str2);
bool string_starts_with_case_insensitive(const std::string& str, const std::string& prefix);
string string_to_lower(const string& str);
string string_replace_all(const string& szStr, const string& szFrom, const string& szTo);
string create_hex_dump(const UINT8* buffer, size_t size);
vector<unsigned char> base64_decode(const string &encoded_string);
const char* gl_to_string(GLuint value);
vector<string> add_line_numbers(const char* src);

extern "C" HRESULT external_open_storage(const OLECHAR* pwcsName, IStorage* pstgPriority, DWORD grfMode, SNB snbExclude, DWORD reserved, IStorage **ppstgOpen);
extern "C" HRESULT external_create_object(const WCHAR *progid, IClassFactory* cf, IUnknown* obj);
extern "C" void external_log_info(const char* format, ...);
extern "C" void external_log_debug(const char* format, ...);
extern "C" void external_log_error(const char* format, ...);
#endif
