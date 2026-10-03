// license:GPLv3+

#include "core/stdafx.h"

#ifndef __STANDALONE__
#include <Intshcut.h>
#else
#include <dirent.h>
#include <sys/stat.h>
#if defined(__APPLE__)
#include <sys/param.h>
#include <sys/mount.h>
#elif defined(__linux__) || defined(__ANDROID__)
#include <sys/vfs.h>
#endif
#endif

#include "core/VPApp.h"
#include "core/resourceid.h"

#include <atomic>
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
     { IDS_SAVEERROR, "There was an error saving the file." },
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
      return std::format("{} B", bytes);

   double size = static_cast<double>(bytes) / 1024.0;
   int suffixIndex = 0;
   while (size >= 1024.0 && suffixIndex < (int)std::size(suffixes) - 1)
   {
      suffixIndex++;
      size /= 1024.0;
   }

   const int whole = (int)size;
   return std::format("{}.{} {}iB", whole, (int)((size - whole) * 10.0 + 0.5), suffixes[suffixIndex]);
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
      _mm_storeu_si128((__m128i*)(result + i    ), _mm_unpacklo_epi8(sz16, zero)); // zero-extend 16 bytes -> 2x8 uint16 and store
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

WCHAR *MakeWide(const string& sz)
{
   // assume that we usually deal with (mostly) ASCII, so then the following over-allocation is (mostly) exact
   // this will speed up both the full ASCII and the non-ASCII fallback cases (1.1x-20x incl. SIMD path); BUT allocates 1x (all ASCII) up to (overallocating) 3x (all non-ASCII), thus saving one Win-API call
   int len = (int)sz.length();
   WCHAR* const __restrict result = new WCHAR[len+1];

#ifdef ENABLE_SSE_OPTIMIZATIONS
   if (!HelperConvertASCII(sz.c_str(), len, result)) // Non-ASCII found? -> Trigger Win-API conversion
#endif
      len = MultiByteToWideChar(CP_UTF8, 0, sz.c_str(), len, result, len+1);
   result[len] = L'\0';
   return result;
}

BSTR MakeWideBSTR(const string& sz)
{
   return MakeWideBSTR(sz.c_str(), sz.length());
}

BSTR MakeWideBSTR(const char* const sz, const size_t length)
{
   // assume that we usually deal with (mostly) ASCII, so then the following over-allocation is (mostly) exact
   // this will speed up both the full ASCII and the non-ASCII fallback cases (1.1x-20x incl. SIMD path); BUT allocates 1x (all ASCII) up to (overallocating) 3x (all non-ASCII), thus saving one Win-API call
   //!! note that this BSTR variant only reaches about 1.1x-1.9x speed up, due to more Win-API overhead
   //   and in the non-ASCII case an additional alloc+copy, but this also removes the overallocation
   const int szlen = (int)length;
   if (szlen == 0)
      return SysAllocString(L"");

   BSTR result = SysAllocStringLen(nullptr, szlen);

#ifdef ENABLE_SSE_OPTIMIZATIONS
   if (HelperConvertASCII(sz, szlen, result)) // all ASCII? -> done
      return result;
#endif

   const int len = MultiByteToWideChar(CP_UTF8, 0, sz, szlen, result, szlen+1);
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

wstring MakeWString(const char* const sz, const size_t length)
{
   // assume that we usually deal with (mostly) ASCII, so then the following over-allocation is (mostly) exact
   // this will speed up both the full ASCII and the non-ASCII fallback cases (1.1x-20x incl. SIMD path); BUT allocates 1x (all ASCII) up to (overallocating) 3x (all non-ASCII), thus saving one Win-API call
   //!! note that this wstring variant only reaches about 1.2x-3.4x speed up
   const int len = (int)length;
   wstring result(len, L'\0');
#ifdef ENABLE_SSE_OPTIMIZATIONS
   if (HelperConvertASCII(sz, len, result.data())) // all ASCII? -> done
      return result;
#endif
   result.resize(len > 0 ? MultiByteToWideChar(CP_UTF8, 0, sz, len, result.data(), len) : 0); //!! potentially reallocs
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

#ifdef _WIN32
static bool IsValidUTF8(const char* str, size_t length);

std::filesystem::path PathFromUTF8OrString(const std::string& s)
{
   return IsValidUTF8(s.data(), s.size()) ? PathFromUTF8(s) : PathFromString(s);
}

std::filesystem::path PathFromString(const std::string& s)
{
   if (s.empty())
      return {};
   #ifdef _MSC_VER
   UINT codepage = CP_ACP; // Matches PathToString
   #else
   UINT codepage = CP_UTF8; // MinGW's std::filesystem narrow encoding
   #endif
   int len = MultiByteToWideChar(codepage, MB_ERR_INVALID_CHARS, s.data(), static_cast<int>(s.size()), nullptr, 0);
   if (len <= 0)
   {
      // Legacy ANSI text (e.g. settings written before the switch to UTF-8): decode it with the user's ANSI code page, or Windows-1252 when there is none or it is UTF-8 itself (Windows 'Beta: use Unicode UTF-8' option)
      DWORD legacyCodepage = 0;
      if (GetLocaleInfoEx(LOCALE_NAME_USER_DEFAULT, LOCALE_IDEFAULTANSICODEPAGE | LOCALE_RETURN_NUMBER, reinterpret_cast<LPWSTR>(&legacyCodepage), sizeof(legacyCodepage) / sizeof(WCHAR)) == 0
         || legacyCodepage == 0 || legacyCodepage == CP_UTF8)
         legacyCodepage = 1252;
      codepage = legacyCodepage;
      len = MultiByteToWideChar(codepage, 0, s.data(), static_cast<int>(s.size()), nullptr, 0);
   }
   std::wstring wide(len, L'\0');
   MultiByteToWideChar(codepage, 0, s.data(), static_cast<int>(s.size()), wide.data(), len);
   return wide;
}
#endif

#ifdef _MSC_VER
std::string PathToString(const std::filesystem::path& path)
{
   const std::wstring& wide = path.native();
   if (wide.empty())
      return {};
#ifdef ENABLE_SSE_OPTIMIZATIONS // All ANSI code pages are ASCII supersets
#pragma warning(push)
#pragma warning(disable : 4244) // conversion from wchar to char
   if (HelperIsASCII(wide.c_str(), static_cast<int>(wide.size())))
      return std::string(wide.begin(), wide.end());
#pragma warning(pop)
#endif
   const char* const defaultChar = (GetACP() == CP_UTF8) ? nullptr : "_"; // A default char makes the UTF-8 code page (Windows beta option) fail
   const int len = WideCharToMultiByte(CP_ACP, 0, wide.c_str(), static_cast<int>(wide.size()), nullptr, 0, defaultChar, nullptr);
   std::string narrow(len, '\0');
   WideCharToMultiByte(CP_ACP, 0, wide.c_str(), static_cast<int>(wide.size()), narrow.data(), len, defaultChar, nullptr);
   return narrow;
}
#endif

string MakeString(const WCHAR* const wz, const size_t length)
{
   const int len = (int)length;
   if (len <= 0)
      return string();
#ifdef ENABLE_SSE_OPTIMIZATIONS // 1.5x-8.5x faster for all ASCII cases
#pragma warning(push)
#pragma warning(disable : 4244) // conversion from wchar to char
   if (HelperIsASCII(wz, len))
      return string(wz, wz + len); // all ASCII
#pragma warning(pop)
#endif

   // non-ASCII found
   // Note: Even in this case, the additional SIMD detection loop above is barely noticeable, thus overall performance is still ~1x
   // Note: Overallocation (by up to 3x (UTF16) or 4x (UTF32)) instead of exact allocation (similar to MakeWide) is not efficient as the assumption is that most of the chars will be ASCII (such benchmarks are significantly slower then)
   const int size = WideCharToMultiByte(CP_UTF8, 0, wz, len, nullptr, 0, nullptr, nullptr);
   if (size <= 0)
      return string();
   string result(size, '\0');
   WideCharToMultiByte(CP_UTF8, 0, wz, len, result.data(), size, nullptr, nullptr);
   return result;
}

char* MakeCharArray(const WCHAR* const wz, const int length)
{
#ifdef ENABLE_SSE_OPTIMIZATIONS
   if (HelperIsASCII(wz, length))
   {
      char* const result = new char[length + 1];
      for (int i = 0; i < length; ++i)
         result[i] = static_cast<char>(wz[i]);
      result[length] = '\0';
      return result;
   }
#endif

   // non-ASCII found
   const int len = (length > 0) ? WideCharToMultiByte(CP_UTF8, 0, wz, length, nullptr, 0, nullptr, nullptr) : 0;
   if (len <= 0 && length > 0)
      return nullptr;
   char* const result = new char[len + 1];
   if (len > 0)
      WideCharToMultiByte(CP_UTF8, 0, wz, length, result, len, nullptr, nullptr);
   result[len] = '\0';
   return result;
}

//

#ifdef _WIN32
void SetThreadName(const std::string& name)
{
   const wstring wname = MakeWString(name);
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

static std::atomic<UserMessageSink*> s_userMessageSink = nullptr;

UserMessageSink* SetUserMessageSink(UserMessageSink* const sink) { return s_userMessageSink.exchange(sink); }

void ShowMessage(const MsgSeverity severity, const string& message, const string& title)
{
   switch (severity)
   {
   case MsgSeverity::Info: PLOGI << message; break;
   case MsgSeverity::Warning: PLOGW << message; break;
   case MsgSeverity::Error:
   case MsgSeverity::Fatal: PLOGE << message; break;
   }
   UserMessageSink* const sink = s_userMessageSink.load();
   if (sink)
      sink->Notify(severity, title.empty() ? ((severity == MsgSeverity::Error || severity == MsgSeverity::Fatal) ? "Visual Pinball Error"s : "Visual Pinball"s) : title, message);
}

void ShowError(const char* const sz) { ShowMessage(MsgSeverity::Error, sz); }

void ShowFatalError(const string& message) { ShowMessage(MsgSeverity::Fatal, message); }

bool AskUser(const string& question, const string& title, const bool fallback)
{
   UserMessageSink* const sink = s_userMessageSink.load();
   if (sink == nullptr)
   {
      PLOGI << "User question '" << question << "' answered with fallback (" << (fallback ? "yes" : "no") << ") as no message sink is installed";
      return fallback;
   }
   const bool answer = sink->Confirm(title.empty() ? "Visual Pinball"s : title, question, fallback);
   PLOGI << "User question '" << question << "' answered: " << (answer ? "yes" : "no");
   return answer;
}

#ifndef __STANDALONE__
void Win32DialogSink::Notify(const MsgSeverity severity, const string& title, const string& message)
{
   const UINT icon = severity == MsgSeverity::Info ? MB_ICONINFORMATION : severity == MsgSeverity::Warning ? MB_ICONWARNING : MB_ICONERROR;
   ::MessageBoxW(m_parent, MakeWString(message).c_str(), MakeWString(title).c_str(), MB_OK | icon); // Messages are UTF-8
}

bool Win32DialogSink::Confirm(const string& title, const string& message, const bool fallback)
{
   return ::MessageBoxW(m_parent, MakeWString(message).c_str(), MakeWString(title).c_str(), MB_YESNO | MB_ICONQUESTION | (fallback ? MB_DEFBUTTON1 : MB_DEFBUTTON2)) == IDYES;
}
#endif

#ifdef _WIN32
#include <share.h>
#endif

FILE* open_file(const std::filesystem::path& path, const char* mode)
{
#ifdef _WIN32
   return _wfsopen(path.c_str(), wstring(mode, mode + strlen(mode)).c_str(), _SH_DENYNO); // mode is ASCII; shared like fopen
#else
   return fopen(path.c_str(), mode);
#endif
}

vector<uint8_t> read_file(const std::filesystem::path& filename, const bool binary)
{
   vector<uint8_t> data;
   std::ifstream file(filename, binary ? (std::ios::binary | std::ios::ate) : std::ios::ate);
   if (!file)
   {
      ShowError("The file \"" + PathToUTF8(filename) + "\" could not be opened.");
      return data;
   }
   data.resize((size_t)file.tellg());
   file.seekg(0, std::ios::beg);
   file.read(reinterpret_cast<char*>(data.data()), data.size());
   file.close();
   return data;
}

void write_file(const std::filesystem::path& filename, const vector<uint8_t>& data, const bool binary)
{
   std::ofstream file(filename, binary ? (std::ios::binary | std::ios::trunc) : std::ios::trunc);
   if (!file)
   {
      const string text = "The file \"" + PathToUTF8(filename) + "\" could not be opened for writing.";
      ShowError(text);
      return;
   }
   file.write(reinterpret_cast<const char*>(data.data()), data.size());
   file.close();
}

bool IsNetworkPath(const std::filesystem::path& path)
{
#ifndef __STANDALONE__
   // Windows: a UNC path (\\server\share or //server/share) is always remote. Otherwise ask the
   // drive type of the path's root.
   const std::wstring wpath = path.native();
   if (wpath.size() >= 2 && (wpath[0] == L'\\' || wpath[0] == L'/') && (wpath[1] == L'\\' || wpath[1] == L'/'))
      return true;
   const std::filesystem::path root = path.root_path();
   if (root.empty())
      return false;
   return GetDriveTypeW(root.wstring().c_str()) == DRIVE_REMOTE;
#elif defined(__APPLE__)
   // macOS/BSD: MNT_LOCAL is set for local filesystems and clear for network mounts.
   struct statfs buf;
   if (statfs(path.c_str(), &buf) != 0)
      return false;
   return (buf.f_flags & MNT_LOCAL) == 0;
#elif defined(__linux__) || defined(__ANDROID__)
   // Linux has no MNT_LOCAL, so match the filesystem magic of known network filesystems.
   struct statfs buf;
   if (statfs(path.c_str(), &buf) != 0)
      return false;
   switch (static_cast<unsigned long>(buf.f_type))
   {
   case 0x6969:     // NFS_SUPER_MAGIC
   case 0x517B:     // SMB_SUPER_MAGIC (legacy smbfs)
   case 0xFF534D42: // CIFS_MAGIC_NUMBER
   case 0xFE534D42: // SMB2_MAGIC_NUMBER
   case 0x7461636C: // OCFS2
   case 0x01021997: // V9FS (Plan 9, used by some VM shares)
      return true;
   default:
      return false;
   }
#else
   (void)path;
   return false;
#endif
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
         if (!ec && StrCompareNoCase(PathToUTF8(ent.path().filename()), PathToUTF8(path.filename())))
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
// Windows-1252 (Western ANSI) to UTF-8: ISO-8859-1 plus printable characters in 0x80..0x9F (euro sign, typographic quotes, dashes,...).
// The 5 unassigned bytes map to U+0081,... like Windows does
static constexpr uint16_t cp1252_80_9f[32] = {
   0x20AC, 0x0081, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021, 0x02C6, 0x2030, 0x0160, 0x2039, 0x0152, 0x008D, 0x017D, 0x008F,
   0x0090, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014, 0x02DC, 0x2122, 0x0161, 0x203A, 0x0153, 0x009D, 0x017E, 0x0178 };

static string cp1252_to_utf8(const char* str, const size_t length)
{
   string utf8(3 * length, '\0'); // worst case

   char* c = utf8.data();
   for (size_t i = 0; i < length; ++i)
   {
      const uint8_t b = static_cast<uint8_t>(str[i]);
      const uint32_t cp = (b >= 0x80 && b <= 0x9F) ? cp1252_80_9f[b - 0x80] : b;
      if (cp < 0x80)
         *c++ = static_cast<char>(cp);
      else if (cp < 0x800)
      {
         *c++ = static_cast<char>(0xC0 | (cp >> 6));
         *c++ = static_cast<char>(0x80 | (cp & 0x3F));
      }
      else
      {
         *c++ = static_cast<char>(0xE0 | (cp >> 12));
         *c++ = static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
         *c++ = static_cast<char>(0x80 | (cp & 0x3F));
      }
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

string TruncateToUTF16Length(const string& utf8, size_t maxUnits)
{
   size_t units = 0, pos = 0;
   while (pos < utf8.size())
   {
      const uint8_t lead = static_cast<uint8_t>(utf8[pos]);
      const size_t bytes = (lead >= 0xF0 && lead < 0xF8) ? 4 : (lead >= 0xE0 && lead < 0xF0) ? 3 : (lead >= 0xC0 && lead < 0xE0) ? 2 : 1;
      const size_t charUnits = (bytes == 4) ? 2 : 1; // Characters beyond the BMP take a surrogate pair, invalid bytes count as 1
      if (units + charUnits > maxUnits)
         break;
      units += charUnits;
      pos = min(pos + bytes, utf8.size());
   }
   return utf8.substr(0, pos);
}

static bool IsValidUTF8(const char* str, size_t length)
{
   uint32_t state = UTF8_ACCEPT;
   return validate_utf8(&state, str, length) == UTF8_ACCEPT; // Also rejects a sequence cut at the end
}

bool utf8_to_cp1252(const string& utf8, string& cp1252)
{
   cp1252.clear();
   cp1252.reserve(utf8.size());
   uint32_t state = UTF8_ACCEPT, cp = 0;
   for (const char c : utf8)
   {
      const uint32_t result = decode(&state, &cp, static_cast<uint8_t>(c));
      if (result == UTF8_REJECT)
         return false;
      if (result != UTF8_ACCEPT)
         continue; // Inside a sequence
      if (cp < 0x80 || (cp >= 0xA0 && cp <= 0xFF))
         cp1252 += static_cast<char>(cp);
      else if (const auto it = std::ranges::find(cp1252_80_9f, cp); it != std::end(cp1252_80_9f))
         cp1252 += static_cast<char>(0x80 + (it - std::begin(cp1252_80_9f)));
      else
         return false;
   }
   return state == UTF8_ACCEPT;
}

string TruncateToUTF8Length(const string& utf8, size_t maxBytes)
{
   if (utf8.size() <= maxBytes)
      return utf8;
   size_t pos = maxBytes;
   while (pos > 0 && (static_cast<uint8_t>(utf8[pos]) & 0xC0) == 0x80) // Back to the start of the character that would be cut
      pos--;
   return utf8.substr(0, pos);
}

string string_from_utf8_or_cp1252(string&& src)
{
   if (!IsValidUTF8(src.data(), src.size()))
      return cp1252_to_utf8(src.data(), src.size()); // old ANSI characters? -> convert to UTF-8
   return std::move(src);
}

string string_from_utf8_or_cp1252(const char* src, size_t srcSize)
{
   if (!IsValidUTF8(src, srcSize))
      return cp1252_to_utf8(src, srcSize); // old ANSI characters? -> convert to UTF-8
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
