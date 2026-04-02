// license:GPLv3+

#include "core/stdafx.h"

#ifndef __STANDALONE__
#include <Intshcut.h>
#else
#include <dirent.h>
#include <sys/stat.h>
#include "standalone/PoleStorage.h"
#endif

#include <charconv>
#include <iomanip>
#include <filesystem>
#if defined(__APPLE__) || defined(__linux__) || defined(__ANDROID__)
#include <pthread.h>
#endif

#ifdef __MINGW32__
#include <winnls.h>
static const char point = []() -> char {
   char buf[4];
   if (GetLocaleInfoA(LOCALE_USER_DEFAULT, LOCALE_SDECIMAL, buf, sizeof(buf)) > 0)
      return buf[0];
   return '.';
}();
#else
static const char point = std::use_facet<std::numpunct<char>>(std::locale("")).decimal_point();
#endif

uint64_t mwc64x_state = 4077358422479273989ull;

// (optionally) convert decimal point to locale specific one (i.e. ',' or force to always use '.')
// and trim all trailing zeros for better readability
string convert_decimal_point_and_trim(string sz, const bool use_locale) // use_locale: true if the decimal point should be converted to the OS locale setting, otherwise false (i.e. always use '.' as decimal point)
{
   const size_t pos = sz.find_first_of(",."); // search for the 2 variants
   if (pos != string::npos)
   {
      sz[pos] = use_locale ? point : '.'; // replace it with the locale specific one (or always use '.' as decimal point)

      size_t pos0 = sz.find_last_not_of('0');
      if (pos0 == pos)
         pos0++;
      sz.erase(pos0 + 1, string::npos); // remove trailing zeros, but leave .0 for integers (line above), as then its clearer that a decimal point can be used for a certain setting!
   }
   return sz;
}

// used by dialogues, etc, locale specific, otherwise use std::from_chars (or e.g. std::stof() (with exception handling) or std::strtof()) directly
float sz2f(string sz, const bool force_convert_decimal_point)
{
#if 1
   if (force_convert_decimal_point || point != '.') // fix locales that use a ',' instead of the C '.' as decimal point
   {
      const size_t pos = sz.find_first_of(force_convert_decimal_point ? ',' : point);
      if (pos != string::npos)
         sz[pos] = '.';
   }

#if defined(__clang__)
   const char* const p = sz.c_str();
   char* e;
   const float result = std::strtof(p, &e);

   if (p == e)
      return 0.0f; //!! use inf or NaN instead?

   return result;
#else
   float result;
   return (std::from_chars(sz.c_str(), sz.c_str() + sz.length(), result).ec == std::errc{}) ? result : 0.0f; //!! use inf or NaN instead?
#endif
#else
   const int len = MultiByteToWideChar(CP_ACP, 0, sz.c_str(), -1, nullptr, 0); //(int)sz.length()+1;
   WCHAR * const wzT = new WCHAR[len];
   MultiByteToWideChar(CP_ACP, 0, sz.c_str(), -1, wzT, len);

   CComVariant var = wzT;

   float result;
   if (SUCCEEDED(VariantChangeType(&var, &var, 0, VT_R4)))
   {
      result = V_R4(&var);
      VariantClear(&var);
   }
   else
      result = 0.0f; //!! use inf or NaN instead?

   delete[] wzT;

   return result;
#endif
}

// used by dialogues, etc, (optionally) locale specific, otherwise use e.g. std::to_string() directly
// will also trim all trailing zeros for better readability in the UI
string f2sz(const float f, const bool can_convert_decimal_point)
{
#if 1
   string sz = std::to_string(f);
   const size_t pos = sz.find_first_of('.');
   if (pos != string::npos)
   {
      if (can_convert_decimal_point && point != '.') // fix locales that use a ',' instead of the C '.' as decimal point
         sz[pos] = point;

      size_t pos0 = sz.find_last_not_of('0');
      if (pos0 == pos)
         pos0++;
      sz.erase(pos0 + 1, string::npos); // remove trailing zeros, but leave .0 for integers (line above), as then its clearer that a decimal point can be used for a certain setting!
   }

   return sz;
#else
   CComVariant var = f;

   if (SUCCEEDED(VariantChangeType(&var, &var, 0, VT_BSTR)))
   {
      const WCHAR * const wzT = V_BSTR(&var);
      const string tmp = MakeString(wzT);
      VariantClear(&var);
      return tmp;
   }
   else
      return "0.0"s; //!! should be localized! i.e. . vs ,
#endif
}

wstring f2wz(const float f, const bool can_convert_decimal_point)
{
   wstring wz = std::to_wstring(f);
   const size_t pos = wz.find_first_of(L'.');
   if (pos != wstring::npos)
   {
      if (can_convert_decimal_point && point != '.') // fix locales that use a ',' instead of the C '.' as decimal point
         wz[pos] = point;

      size_t pos0 = wz.find_last_not_of(L'0');
      if (pos0 == pos)
         pos0++;
      wz.erase(pos0 + 1, wstring::npos); // remove trailing zeros, but leave .0 for integers (line above), as then its clearer that a decimal point can be used for a certain setting!
   }

   return wz;
}

LocalString::LocalString(const int resid)
{
   m_szbuffer[0] = '\0';
#ifndef __STANDALONE__
   if (resid > 0)
   {
      // Note that with the char version of LoadString one cannot get a pointer to the internal buffer directly
      /*const int cchar =*/LoadString(g_app->GetInstanceHandle(), resid, m_szbuffer, sizeof(m_szbuffer));
      m_szbuffer[std::size(m_szbuffer)-1] = '\0'; // in case of truncation
   }
#else
   static const ankerl::unordered_dense::map<int, const char*> ids_map = {
     { IDS_SCRIPT, "Script" },
     { IDS_TB_BUMPER, "Bumper" },
     { IDS_TB_DECAL, "Decal" },
     { IDS_TB_DISPREEL, "EMReel" },
     { IDS_TB_FLASHER, "Flasher" },
     { IDS_TB_FLIPPER, "Flipper" },
     { IDS_TB_GATE, "Gate" },
     { IDS_TB_KICKER, "Kicker" },
     { IDS_TB_LIGHT, "Light" },
     { IDS_TB_LIGHTSEQ, "LightSeq" },
     { IDS_TB_PLUNGER, "Plunger" },
     { IDS_TB_PRIMITIVE, "Primitive" },
     { IDS_TB_WALL, "Wall" },
     { IDS_TB_RAMP, "Ramp" },
     { IDS_TB_RUBBER, "Rubber" },
     { IDS_TB_SPINNER, "Spinner" },
     { IDS_TB_TEXTBOX, "TextBox" },
     { IDS_TB_TIMER, "Timer" },
     { IDS_TB_TRIGGER, "Trigger" },
     { IDS_TB_TARGET, "Target" }
   };
   const ankerl::unordered_dense::map<int, const char*>::const_iterator it = ids_map.find(resid);
   if (it != ids_map.end())
      strncpy_s(m_szbuffer, std::size(m_szbuffer), it->second);
#endif
}

LocalStringW::LocalStringW(const int resid)
{
#ifndef __STANDALONE__
   if (resid > 0)
   {
      LPWSTR strPtr = nullptr;
      const int len = LoadStringW(g_app->GetInstanceHandle(), resid, reinterpret_cast<LPWSTR>(&strPtr), 0);
      if (len > 0 && strPtr)
         m_buffer = wstring(strPtr, len);
   }
#else
   static const ankerl::unordered_dense::map<int, const wstring> ids_map = {
     { IDS_SCRIPT, L"Script"s },
     { IDS_TB_BUMPER, L"Bumper"s },
     { IDS_TB_DECAL, L"Decal"s },
     { IDS_TB_DISPREEL, L"EMReel"s },
     { IDS_TB_FLASHER, L"Flasher"s },
     { IDS_TB_FLIPPER, L"Flipper"s },
     { IDS_TB_GATE, L"Gate"s },
     { IDS_TB_KICKER, L"Kicker"s },
     { IDS_TB_LIGHT, L"Light"s },
     { IDS_TB_LIGHTSEQ, L"LightSeq"s },
     { IDS_TB_PLUNGER, L"Plunger"s },
     { IDS_TB_PRIMITIVE, L"Primitive"s },
     { IDS_TB_WALL, L"Wall"s },
     { IDS_TB_RAMP, L"Ramp"s },
     { IDS_TB_RUBBER, L"Rubber"s },
     { IDS_TB_SPINNER, L"Spinner"s },
     { IDS_TB_TEXTBOX, L"TextBox"s },
     { IDS_TB_TIMER, L"Timer"s },
     { IDS_TB_TRIGGER, L"Trigger"s },
     { IDS_TB_TARGET, L"Target"s }
   };
   const ankerl::unordered_dense::map<int, const wstring>::const_iterator it = ids_map.find(resid);
   if (it != ids_map.end())
      m_buffer = it->second;
#endif
}

// Formats a byte size/value into a human-readable string (e.g. 1305486 -> 1.2 MiB).
string SizeToReadable(const size_t bytes)
{
   static constexpr char suffixes[] = { 'K', 'M', 'G', 'T', 'P', 'E' };

   // Format with one decimal for KiB and above, no decimal for bytes
   if (bytes < 1024)
      return std::to_string(bytes) + " B";

   double size = static_cast<double>(bytes);
   int suffixIndex = 0;
   while (size >= 1024.0 && suffixIndex++ < (int)std::size(suffixes)-1)
      size /= 1024.0;

   const int whole = (int)size;
   return std::to_string(whole) + '.' + std::to_string((int)((size-whole)*10.0 + 0.5)) + ' ' + suffixes[suffixIndex-1] + "iB";
}

//

#ifdef ENABLE_SSE_OPTIMIZATIONS
// returns true if szcstr is 100% ASCII, in that case result also contains the converted WCHARs (but no null terminator!)
// (szcstr can be any codepage)
static bool HelperConvertASCII(const char* const __restrict szcstr, const int len, WCHAR* const __restrict result)
{
   int i = 0;
   const __m128i zero = _mm_setzero_si128();
   for (; i+16 <= len; i+=16) // check 16 bytes, then widen to 16 WCHARs per iteration, or break if non-ASCII found
   {
      const __m128i sz16 = _mm_loadu_si128((const __m128i*)(szcstr + i));
      if (_mm_movemask_epi8(sz16) != 0) // test highest bit of each byte, so check for >=0x80 -> non-ASCII
         return false;
#if (WCHAR_T_SIZE == 2) // UTF16
      _mm_storeu_si128((__m128i*)(result + i    ), _mm_unpacklo_epi8(sz16, zero)); // zero-extend 16 bytes -> 2×8 uint16 and store
      _mm_storeu_si128((__m128i*)(result + i + 8), _mm_unpackhi_epi8(sz16, zero));
#else // UTF32
      // zero-extend 16 bytes -> 4×4 uint32 and store
      const __m128i lo16 = _mm_unpacklo_epi8(sz16, zero); // uint8 -> uint16
      const __m128i hi16 = _mm_unpackhi_epi8(sz16, zero);
      _mm_storeu_si128((__m128i*)(result + i     ), _mm_unpacklo_epi16(lo16, zero)); // uint16 -> uint32
      _mm_storeu_si128((__m128i*)(result + i +  4), _mm_unpackhi_epi16(lo16, zero));
      _mm_storeu_si128((__m128i*)(result + i +  8), _mm_unpacklo_epi16(hi16, zero));
      _mm_storeu_si128((__m128i*)(result + i + 12), _mm_unpackhi_epi16(hi16, zero));
#endif
   }
   for (; i < len; ++i)
   {
      if (static_cast<unsigned char>(szcstr[i]) > 0x7F) // non-ASCII?
         return false;
      result[i] = static_cast<WCHAR>(szcstr[i]);
   }
   return true; // all ASCII
}
#endif

WCHAR *MakeWide(const string& sz, const UINT codepage)
{
   // assume that we usually deal with (mostly) ASCII, so then the following over-allocation is (mostly) exact
   // this will speed up both the full ASCII and the non-ASCII fallback cases (1.1x-20x incl. SIMD path); BUT allocates 1x (all ASCII) up to (overallocating) 3x (all non-ASCII), works for all codepages (not just CP_ACP), thus saving one Win-API call
   int len = (int)sz.length();
   WCHAR* const __restrict result = new WCHAR[len+1];

#ifdef ENABLE_SSE_OPTIMIZATIONS
   if (!HelperConvertASCII(sz.c_str(), len, result)) // Non-ASCII found? -> Trigger Win-API conversion
#endif
      len = MultiByteToWideChar(codepage, 0, sz.c_str(), len, result, len+1);
   result[len] = L'\0';
   return result;
}

BSTR MakeWideBSTR(const string& sz, const UINT codepage)
{
   // assume that we usually deal with (mostly) ASCII, so then the following over-allocation is (mostly) exact
   // this will speed up both the full ASCII and the non-ASCII fallback cases (1.1x-20x incl. SIMD path); BUT allocates 1x (all ASCII) up to (overallocating) 3x (all non-ASCII), works for all codepages (not just CP_ACP), thus saving one Win-API call
   //!! note that this BSTR variant only reaches about 1.1x-1.9x speed up, due to more Win-API overhead
   //   and in the non-ASCII case an additional alloc+copy, but this also removes the overallocation
   const int szlen = (int)sz.length();
   if (szlen == 0)
      return SysAllocString(L"");

   BSTR result = SysAllocStringLen(nullptr, szlen);

#ifdef ENABLE_SSE_OPTIMIZATIONS
   if (HelperConvertASCII(sz.c_str(), szlen, result)) // all ASCII? -> done
      return result;
#endif

   const int len = MultiByteToWideChar(codepage, 0, sz.c_str(), szlen, result, szlen+1);
   if (len < szlen) // shrink the BSTR if the actual conversion produced fewer WCHARs (or if above call errors with 0)
   {
      BSTR trimmed = SysAllocStringLen(result, len);
      SysFreeString(result);
      return trimmed;
   }
   return result;
}

// Just for convenience and native file system path conversion
BSTR MakeWideBSTR(const wstring& wz)
{
   return SysAllocStringLen(wz.c_str(), (UINT)wz.length());
}

wstring MakeWString(const string& sz, const UINT codepage)
{
   // assume that we usually deal with (mostly) ASCII, so then the following over-allocation is (mostly) exact
   // this will speed up both the full ASCII and the non-ASCII fallback cases (1.1x-20x incl. SIMD path); BUT allocates 1x (all ASCII) up to (overallocating) 3x (all non-ASCII), works for all codepages (not just CP_ACP), thus saving one Win-API call
   //!! note that this wstring variant only reaches about 1.3x-3.4x speed up
   int len = (int)sz.length();
   wstring result(len, L'\0');

#ifdef ENABLE_SSE_OPTIMIZATIONS
   if (HelperConvertASCII(sz.c_str(), len, result.data())) // all ASCII? -> done
      return result;
#endif
   len = MultiByteToWideChar(codepage, 0, sz.c_str(), len, result.data(), len+1);
   result.resize(len); //!! potentially reallocs
   return result;
}

wstring MakeWString(const char* const sz, const UINT codepage)
{
   // assume that we usually deal with (mostly) ASCII, so then the following over-allocation is (mostly) exact
   // this will speed up both the full ASCII and the non-ASCII fallback cases (1.1x-20x incl. SIMD path); BUT allocates 1x (all ASCII) up to (overallocating) 3x (all non-ASCII), works for all codepages (not just CP_ACP), thus saving one Win-API call
   //!! note that this wstring variant only reaches about 1.2x-3.2x speed up
   int len = (int)strlen(sz);
   wstring result(len, L'\0');

#ifdef ENABLE_SSE_OPTIMIZATIONS
   if (HelperConvertASCII(sz, len, result.data())) // all ASCII? -> done
      return result;
#endif
   len = MultiByteToWideChar(codepage, 0, sz, len, result.data(), len+1);
   result.resize(len); //!! potentially reallocs
   return result;
}

//

#ifdef ENABLE_SSE_OPTIMIZATIONS
// returns true if wzcstr is 100% ASCII
static bool HelperIsASCII(const WCHAR* const __restrict wzcstr, const int len)
{
   int i = 0;
   const __m128i zero = _mm_setzero_si128();
   // mask/check bits above ASCII range, so if any character is >0x7f, it's non-ASCII
#if (WCHAR_T_SIZE == 2) // UTF16
   const __m128i mask = _mm_set1_epi16(~(short)0x7F);
   for (; i+8 <= len; i+=8)
      if (_mm_movemask_epi8(_mm_cmpeq_epi16(_mm_and_si128(_mm_loadu_si128((const __m128i*)(wzcstr + i)), mask), zero)) != 0xFFFF)
         return false; // non-ASCII found
#else // UTF32
   const __m128i mask = _mm_set1_epi32(~(int)0x7F);
   for (; i+4 <= len; i+=4)
      if (_mm_movemask_epi8(_mm_cmpeq_epi32(_mm_and_si128(_mm_loadu_si128((const __m128i*)(wzcstr + i)), mask), zero)) != 0xFFFF)
         return false; // non-ASCII found
#endif
   for (; i < len; ++i)
      if (static_cast<unsigned int>(wzcstr[i]) > 0x7F)
         return false; // dto.
   return true; // all ASCII
}
#endif

string MakeString(const wstring& wz, const UINT codepage)
{
#ifdef ENABLE_SSE_OPTIMIZATIONS // 1.5x-8.5x faster for all ASCII cases
#pragma warning(push)
#pragma warning(disable : 4244) // conversion from wchar to char
   if (HelperIsASCII(wz.c_str(), (int)wz.length()))
      return string(wz.begin(), wz.end()); // all ASCII
#pragma warning(pop)
#endif

   // non-ASCII found
   // Note: Even in this case, the additional SIMD detection loop above is barely noticeable, thus overall performance is still ~1x
   // Note: Overallocation (by up to 3x (UTF16) or 4x (UTF32), depending-on/for-all codepages (not just CP_ACP)) instead of exact allocation (similar to MakeWide) is not efficient as the assumption is that most of the chars will be ASCII (such benchmarks are significantly slower then)
   const int len = WideCharToMultiByte(codepage, 0, wz.c_str(), -1, nullptr, 0, nullptr, nullptr);
   if (len <= 1)
      return string();
   string result(len-1, '\0');
   WideCharToMultiByte(codepage, 0, wz.c_str(), -1, result.data(), len, nullptr, nullptr);
   return result;
}

string MakeString(const WCHAR* const wz, const UINT codepage)
{
#ifdef ENABLE_SSE_OPTIMIZATIONS // 1.5x-8.5x faster for all ASCII cases
   const int wzlen = (int)wcslen(wz); //!! this penalties the non-ASCII case, dropping perf to 0.8x-0.9x instead of ~1x
#pragma warning(push)
#pragma warning(disable : 4244) // conversion from wchar to char
   if (HelperIsASCII(wz, wzlen))
      return string(wz, wz + wzlen); // all ASCII
#pragma warning(pop)
#endif

   // non-ASCII found
   // Note: Even in this case, the additional SIMD detection loop above is barely noticeable, thus overall performance is still ~1x
   // Note: Overallocation (by up to 3x (UTF16) or 4x (UTF32), depending-on/for-all codepages (not just CP_ACP)) instead of exact allocation (similar to MakeWide) is not efficient as the assumption is that most of the chars will be ASCII (such benchmarks are significantly slower then)
   const int len = WideCharToMultiByte(codepage, 0, wz, -1, nullptr, 0, nullptr, nullptr);
   if (len <= 1)
      return string();
   string result(len-1, '\0');
   WideCharToMultiByte(codepage, 0, wz, -1, result.data(), len, nullptr, nullptr);
   return result;
}

string MakeString(const BSTR wz, const UINT codepage)
{
#ifdef ENABLE_SSE_OPTIMIZATIONS // 1.5x-8.5x faster for all ASCII cases
   const int wzlen = (int)SysStringLen(wz);
#pragma warning(push)
#pragma warning(disable : 4244) // conversion from wchar to char
   if (HelperIsASCII(wz, wzlen))
      return string(wz, wz + wzlen); // all ASCII
#pragma warning(pop)
#endif

   // non-ASCII found
   // Note: Even in this case, the additional SIMD detection loop above is barely noticeable, thus overall performance is still ~1x
   // Note: Overallocation (by up to 3x (UTF16) or 4x (UTF32), depending-on/for-all codepages (not just CP_ACP)) instead of exact allocation (similar to MakeWide) is not efficient as the assumption is that most of the chars will be ASCII (such benchmarks are significantly slower then)
   const int len = WideCharToMultiByte(codepage, 0, wz, -1, nullptr, 0, nullptr, nullptr);
   if (len <= 1)
      return string();
   string result(len-1, '\0');
   WideCharToMultiByte(codepage, 0, wz, -1, result.data(), len, nullptr, nullptr);
   return result;
}

//

#ifdef _WIN32
void SetThreadName(const std::string& name)
{
   const wstring wname = MakeWString(name, CP_UTF8);
   if (wname.empty())
      return;
   HRESULT hr = SetThreadDescription(GetCurrentThread(), wname.c_str());
}
#else
void SetThreadName(const std::string& name)
{
#ifdef __APPLE__
   pthread_setname_np(name.c_str());
#elif defined(__linux__) || defined(__ANDROID__)
   pthread_setname_np(pthread_self(), name.c_str());
#endif
}
#endif

// Helper function for IsOnWine
//
// This exists such that we only check if we're on wine once, and assign the result of this function to a static const var
static bool IsOnWineInternal()
{
#ifndef __STANDALONE__
   // See https://www.winehq.org/pipermail/wine-devel/2008-September/069387.html
   const HMODULE ntdllHandle = GetModuleHandleW(L"ntdll.dll");
   assert(ntdllHandle != nullptr && "Could not GetModuleHandleW(L\"ntdll.dll\")");
   return GetProcAddress(ntdllHandle, "wine_get_version") != nullptr;
#else
   return false;
#endif
}

bool IsOnWine()
{
   static const bool result = IsOnWineInternal();
   return result;
}

#ifdef _WIN32
typedef HRESULT(STDAPICALLTYPE* pRGV)(LPOSVERSIONINFOEXW osi);
static pRGV mRtlGetVersion = nullptr;

bool IsWindows10_1803orAbove()
{
   if (mRtlGetVersion == nullptr)
      mRtlGetVersion = (pRGV)GetProcAddress(GetModuleHandle(TEXT("ntdll")), "RtlGetVersion"); // apparently the only really reliable solution to get the OS version (as of Win10 1803)

   if (mRtlGetVersion != nullptr)
   {
      OSVERSIONINFOEXW osInfo;
      osInfo.dwOSVersionInfoSize = sizeof(osInfo);
      mRtlGetVersion(&osInfo);

      if (osInfo.dwMajorVersion > 10)
         return true;
      if (osInfo.dwMajorVersion == 10 && osInfo.dwMinorVersion > 0)
         return true;
      if (osInfo.dwMajorVersion == 10 && osInfo.dwMinorVersion == 0 && osInfo.dwBuildNumber >= 17134) // which is the more 'common' 1803
         return true;
   }

   return false;
}

bool IsWindowsVistaOr7()
{
   OSVERSIONINFOEXW osvi = { sizeof(osvi), 0, 0, 0, 0, {}, 0, 0, 0, 0, 0 };
   const DWORDLONG dwlConditionMask = //VerSetConditionMask(
      VerSetConditionMask(VerSetConditionMask(0, VER_MAJORVERSION, VER_EQUAL), VER_MINORVERSION, VER_EQUAL) /*,
      VER_SERVICEPACKMAJOR, VER_GREATER_EQUAL)*/
      ;
   osvi.dwMajorVersion = HIBYTE(_WIN32_WINNT_VISTA);
   osvi.dwMinorVersion = LOBYTE(_WIN32_WINNT_VISTA);
   //osvi.wServicePackMajor = 0;

   const bool vista = VerifyVersionInfoW(&osvi, VER_MAJORVERSION | VER_MINORVERSION /*| VER_SERVICEPACKMAJOR*/, dwlConditionMask) != FALSE;

   OSVERSIONINFOEXW osvi2 = { sizeof(osvi), 0, 0, 0, 0, {}, 0, 0, 0, 0, 0 };
   osvi2.dwMajorVersion = HIBYTE(_WIN32_WINNT_WIN7);
   osvi2.dwMinorVersion = LOBYTE(_WIN32_WINNT_WIN7);
   //osvi2.wServicePackMajor = 0;

   const bool win7 = VerifyVersionInfoW(&osvi2, VER_MAJORVERSION | VER_MINORVERSION /*| VER_SERVICEPACKMAJOR*/, dwlConditionMask) != FALSE;

   return vista || win7;
}
#endif

void ShowError(const char* const sz)
{
   if (g_pvp)
      g_pvp->MessageBox(sz, "Visual Pinball Error", MB_OK | MB_ICONEXCLAMATION);
   else
      MessageBox(nullptr, sz, "Visual Pinball Error", MB_OK | MB_ICONEXCLAMATION);
}

vector<uint8_t> read_file(const std::filesystem::path& filename, const bool binary)
{
   vector<uint8_t> data;
   std::ifstream file(filename, binary ? (std::ios::binary | std::ios::ate) : std::ios::ate);
   if (!file)
   {
      ShowError("The file \"" + filename.string() + "\" could not be opened.");
      return data;
   }
   data.resize((size_t)file.tellg());
   file.seekg(0, std::ios::beg);
   file.read(reinterpret_cast<char*>(data.data()), data.size());
   file.close();
   return data;
}

void write_file(const string& filename, const vector<uint8_t>& data, const bool binary)
{
   std::ofstream file(filename, binary ? (std::ios::binary | std::ios::trunc) : std::ios::trunc);
   if (!file)
   {
      const string text = "The file \"" + filename + "\" could not be opened for writing.";
      ShowError(text);
      return;
   }
   file.write(reinterpret_cast<const char*>(data.data()), data.size());
   file.close();
}

string normalize_path_separators(const string& szPath)
{
   string szResult = szPath;

   #if '/' == PATH_SEPARATOR_CHAR
      std::ranges::replace(szResult.begin(), szResult.end(), '\\', PATH_SEPARATOR_CHAR);
   #else
      std::ranges::replace(szResult.begin(), szResult.end(), '/', PATH_SEPARATOR_CHAR);
   #endif

   auto end = std::unique(szResult.begin(), szResult.end(),
      [](char a, char b) { return a == b && a == PATH_SEPARATOR_CHAR; });
   szResult.erase(end, szResult.end());

   return szResult;
}

std::filesystem::path find_case_insensitive_file_path(const std::filesystem::path& searchedFile)
{
   auto fn = [](const auto& self, std::filesystem::path path)
   {
      std::error_code ec;
      path = path.lexically_normal();
      if (std::filesystem::exists(path, ec))
         return path;

      const auto& parent = path.parent_path();
      std::filesystem::path base = (parent.empty() || parent == path) ? std::filesystem::path("."s) : self(self, parent);
      if (base.empty())
         return base;

      for (const auto& ent : std::filesystem::directory_iterator(base, ec))
      {
         if (!ec && StrCompareNoCase(ent.path().filename().string(), path.filename().string()))
         {
            const auto& found = ent.path();
            if (found != path)
            {
               PLOGI << "case insensitive file match: requested \"" << path << "\", actual \"" << found << '"';
            }
            return found;
         }
      }

      return std::filesystem::path();
   };

   const std::filesystem::path result = fn(fn, searchedFile);
   return result.empty() ? result : std::filesystem::absolute(result);
}

// returns file extension in lower case (e.g. "png" or "hdr")
string extension_from_path(const string& path)
{
   const size_t pos = path.find_last_of('.');
   return pos != string::npos ? lowerCase(path.substr(pos + 1)) : string();
}

bool path_has_extension(const string& path, const string& ext)
{
   return extension_from_path(path) == lowerCase(ext);
}

bool try_parse_float(const string& str, float& value)
{
   const string tmp = trim_string(str);
#if defined(__clang__)
   const char* const p = tmp.c_str();
   char* e;
   value = std::strtof(p, &e);
   return (p != e);
#else
   return (std::from_chars(tmp.c_str(), tmp.c_str() + tmp.length(), value).ec == std::errc{});
#endif
}

string string_replace_all(const string& szStr, const string& szFrom, const string& szTo, const size_t offs)
{
   string result = szStr;
   size_t pos = offs;
   while ((pos = result.find(szFrom, pos)) != string::npos)
   {
      result.replace(pos, szFrom.length(), szTo);
      pos += szTo.length();
   }
   return result;
}

string string_replace_all(const string& szStr, const string& szFrom, const char szTo, const size_t offs)
{
   string result = szStr;
   size_t pos = offs;
   while ((pos = result.find(szFrom, pos)) != string::npos)
   {
      result.replace(pos, szFrom.length(), 1, szTo);
      ++pos;
   }
   return result;
}

string string_replace_all(const string& szStr, const char szFrom, const string& szTo, const size_t offs)
{
   string result = szStr;
   size_t pos = offs;
   while ((pos = result.find(szFrom, pos)) != string::npos)
   {
      result.replace(pos, 1, szTo);
      pos += szTo.length();
   }
   return result;
}

// Copyright (c) 2008-2009 Bjoern Hoehrmann <bjoern@hoehrmann.de>
// See http://bjoern.hoehrmann.de/utf-8/decoder/dfa/ for details.

#define UTF8_ACCEPT 0
#define UTF8_REJECT 1

static constexpr uint8_t utf8d[] = {
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 00..1f
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 20..3f
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 40..5f
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 60..7f
   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, // 80..9f
   7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, // a0..bf
   8, 8, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, // c0..df
   0xa, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x4, 0x3, 0x3, // e0..ef
   0xb, 0x6, 0x6, 0x6, 0x5, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, // f0..ff
   0x0, 0x1, 0x2, 0x3, 0x5, 0x8, 0x7, 0x1, 0x1, 0x1, 0x4, 0x6, 0x1, 0x1, 0x1, 0x1, // s0..s0
   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, // s1..s2
   1, 2, 1, 1, 1, 1, 1, 2, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, // s3..s4
   1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 1, 3, 1, 1, 1, 1, 1, 1, // s5..s6
   1, 3, 1, 1, 1, 1, 1, 3, 1, 3, 1, 1, 1, 1, 1, 1, 1, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // s7..s8
};

// old ANSI to UTF-8 (allocates new mem block)
static string iso8859_1_to_utf8(const char* str, const size_t length)
{
   string utf8(2 * length, '\0'); // worst case

   char* c = utf8.data();
   for (size_t i = 0; i < length; ++i, ++str)
   {
      if (*str & 0x80)
      {
         *c++ = 0xc0 | (char)((unsigned char)*str >> 6);
         *c++ = 0x80 | (*str & 0x3f);
      }
      //else // check for bogus ASCII control characters
      //if (*str < 9 || (*str > 10 && *str < 13) || (*str > 13 && *str < 32))
      //   *c++ = ' ';
      else
         *c++ = *str;
   }
   utf8.resize(c - utf8.data());

   return utf8;
}

static uint32_t decode(uint32_t* const state, uint32_t* const codep, const uint32_t byte)
{
   const uint32_t type = utf8d[byte];

   *codep = (*state != UTF8_ACCEPT) ? (byte & 0x3fu) | (*codep << 6) : (0xff >> type) & (byte);

   *state = utf8d[256 + *state * 16 + type];
   return *state;
}

static uint32_t validate_utf8(uint32_t* const state, const char* const str, const size_t length)
{
   for (size_t i = 0; i < length; i++)
   {
      const uint8_t type = utf8d[(uint8_t)str[i]];
      *state = utf8d[256 + (*state) * 16 + type];

      if (*state == UTF8_REJECT)
         return UTF8_REJECT;
   }
   return *state;
}

string string_from_utf8_or_iso8859_1(const char* src, size_t srcSize)
{
   uint32_t state = UTF8_ACCEPT;
   if (validate_utf8(&state, src, srcSize) == UTF8_REJECT)
      return iso8859_1_to_utf8(src, srcSize); // old ANSI characters? -> convert to UTF-8
   else
      return string(src, srcSize);
}

//

#ifdef ENABLE_OPENGL
const char* gl_to_string(GLuint value)
{
   static const ankerl::unordered_dense::map<GLuint, const char*> value_map = {
     { (GLuint)GL_RGB, "GL_RGB" },
     { (GLuint)GL_RGBA, "GL_RGBA" },
     { (GLuint)GL_RGB8, "GL_RGB8" },
     { (GLuint)GL_RGBA8, "GL_RGBA8" },
     { (GLuint)GL_SRGB8, "GL_SRGB8" },
     { (GLuint)GL_SRGB8_ALPHA8, "GL_SRGB8_ALPHA8" },
     { (GLuint)GL_RGB16F, "GL_RGB16F" },
     { (GLuint)GL_UNSIGNED_BYTE, "GL_UNSIGNED_BYTE" },
     { (GLuint)GL_HALF_FLOAT, "GL_HALF_FLOAT" },
   };

   const ankerl::unordered_dense::map<GLuint, const char*>::const_iterator it = value_map.find(value);
   if (it != value_map.end()) {
      return it->second;
   }
   return (const char*)"Unknown";
}
#endif

vector<string> add_line_numbers(const char* src)
{
   vector<string> result;
   int lineNumber = 1;

   while (*src != '\0') {
      string line = std::to_string(lineNumber) + ": ";

      while (*src != '\0' && *src != '\n') {
         line += *src;
         src++;
      }

      result.push_back(std::move(line));
      lineNumber++;

      if (*src == '\n') {
         src++;
      }
   }

   return result;
}

#ifdef __STANDALONE__

HRESULT WINAPI StgOpenStorage(const OLECHAR* pwcsName, IStorage* pstgPriority, DWORD grfMode, SNB snbExclude, DWORD reserved, IStorage** ppstgOpen)
{
   return PoleStorage::Create(MakeString(pwcsName), "/"s, (IStorage**)ppstgOpen);
}

#endif
