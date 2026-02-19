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

static const char point = std::use_facet<std::numpunct<char>>(std::locale("")).decimal_point(); // gets the OS locale decimal point (e.g. ',' or '.')

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

LocalString::LocalString(const int resid)
{
   m_szbuffer[0] = '\0';
#ifndef __STANDALONE__
   if (resid > 0)
   {
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
   m_szbuffer[0] = L'\0';
#ifndef __STANDALONE__
   if (resid > 0)
   {
      LoadStringW(g_app->GetInstanceHandle(), resid, m_szbuffer, static_cast<int>(std::size(m_szbuffer)));
      m_szbuffer[std::size(m_szbuffer)-1] = L'\0'; // in case of truncation
   }
#else
   static const ankerl::unordered_dense::map<int, const WCHAR*> ids_map = {
     { IDS_SCRIPT, L"Script" },
     { IDS_TB_BUMPER, L"Bumper" },
     { IDS_TB_DECAL, L"Decal" },
     { IDS_TB_DISPREEL, L"EMReel" },
     { IDS_TB_FLASHER, L"Flasher" },
     { IDS_TB_FLIPPER, L"Flipper" },
     { IDS_TB_GATE, L"Gate" },
     { IDS_TB_KICKER, L"Kicker" },
     { IDS_TB_LIGHT, L"Light" },
     { IDS_TB_LIGHTSEQ, L"LightSeq" },
     { IDS_TB_PLUNGER, L"Plunger" },
     { IDS_TB_PRIMITIVE, L"Primitive" },
     { IDS_TB_WALL, L"Wall" },
     { IDS_TB_RAMP, L"Ramp" },
     { IDS_TB_RUBBER, L"Rubber" },
     { IDS_TB_SPINNER, L"Spinner" },
     { IDS_TB_TEXTBOX, L"TextBox" },
     { IDS_TB_TIMER, L"Timer" },
     { IDS_TB_TRIGGER, L"Trigger" },
     { IDS_TB_TARGET, L"Target" }
   };
   const ankerl::unordered_dense::map<int, const WCHAR*>::const_iterator it = ids_map.find(resid);
   if (it != ids_map.end())
      wcsncpy_s(m_szbuffer, std::size(m_szbuffer), it->second);
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

WCHAR *MakeWide(const char* const sz)
{
   const int len = MultiByteToWideChar(CP_ACP, 0, sz, -1, nullptr, 0); //(int)strlen(sz) + 1; // include null termination
   if (len <= 1)
   {
      WCHAR * const wzT = new WCHAR[1];
      wzT[0] = L'\0';
      return wzT;
   }
   WCHAR * const wzT = new WCHAR[len];
   MultiByteToWideChar(CP_ACP, 0, sz, -1, wzT, len);
   return wzT;
}

BSTR MakeWideBSTR(const string& sz)
{
   const int len = MultiByteToWideChar(CP_ACP, 0, sz.c_str(), -1, nullptr, 0); //(int)sz.length() + 1; // include null termination
   if (len <= 1)
      return SysAllocString(L"");
   BSTR wzT = SysAllocStringLen(nullptr, len - 1);
   MultiByteToWideChar(CP_ACP, 0, sz.c_str(), -1, wzT, len);
   return wzT;
}

WCHAR *MakeWide(const string& sz)
{
   const int len = MultiByteToWideChar(CP_ACP, 0, sz.c_str(), -1, nullptr, 0); //(int)sz.length() + 1; // include null termination
   if (len <= 1)
   {
      WCHAR * const wzT = new WCHAR[1];
      wzT[0] = L'\0';
      return wzT;
   }
   WCHAR * const wzT = new WCHAR[len];
   MultiByteToWideChar(CP_ACP, 0, sz.c_str(), -1, wzT, len);
   return wzT;
}

string MakeString(const wstring &wz)
{
   const int len = WideCharToMultiByte(CP_ACP, 0, wz.c_str(), -1, nullptr, 0, nullptr, nullptr); //(int)wz.length() + 1; // include null termination
   if (len <= 1)
      return string();
   string result(len - 1, '\0');
   WideCharToMultiByte(CP_ACP, 0, wz.c_str(), -1, result.data(), len, nullptr, nullptr);
   return result;
}

string MakeString(const WCHAR* const wz)
{
   const int len = WideCharToMultiByte(CP_ACP, 0, wz, -1, nullptr, 0, nullptr, nullptr); //(int)wcslen(wz) + 1; // include null termination
   if (len <= 1)
      return string();
   string result(len - 1, '\0');
   WideCharToMultiByte(CP_ACP, 0, wz, -1, result.data(), len, nullptr, nullptr);
   return result;
}

string MakeString(const BSTR wz)
{
   const int len = WideCharToMultiByte(CP_ACP, 0, wz, -1, nullptr, 0, nullptr, nullptr); //(int)SysStringLen(wz) + 1; // include null termination
   if (len <= 1)
      return string();
   string result(len - 1, '\0');
   WideCharToMultiByte(CP_ACP, 0, wz, -1, result.data(), len, nullptr, nullptr);
   return result;
}

wstring MakeWString(const string &sz)
{
   const int len = MultiByteToWideChar(CP_ACP, 0, sz.c_str(), -1, nullptr, 0); //(int)sz.length() + 1; // include null termination
   if (len <= 1)
      return wstring();
   wstring result(len - 1, L'\0');
   MultiByteToWideChar(CP_ACP, 0, sz.c_str(), -1, result.data(), len);
   return result;
}

wstring MakeWString(const char * const sz)
{
   const int len = MultiByteToWideChar(CP_ACP, 0, sz, -1, nullptr, 0); //(int)strlen(sz) + 1; // include null termination
   if (len <= 1)
      return wstring();
   wstring result(len - 1, L'\0');
   MultiByteToWideChar(CP_ACP, 0, sz, -1, result.data(), len);
   return result;
}

char *MakeChar(const WCHAR* const wz)
{
   const int len = WideCharToMultiByte(CP_ACP, 0, wz, -1, nullptr, 0, nullptr, nullptr); //(int)wcslen(wz) + 1; // include null termination
   if (len <= 1)
   {
      char * const szT = new char[1];
      szT[0] = '\0';
      return szT;
   }
   char * const szT = new char[len];
   WideCharToMultiByte(CP_ACP, 0, wz, -1, szT, len, nullptr, nullptr);
   return szT;
}

#ifdef _WIN32
void SetThreadName(const std::string& name)
{
   const int size_needed = MultiByteToWideChar(CP_UTF8, 0, name.c_str(), -1, nullptr, 0);
   if (size_needed <= 1)
      return;
   std::wstring wstr(size_needed - 1, L'\0');
   if (MultiByteToWideChar(CP_UTF8, 0, name.c_str(), -1, wstr.data(), size_needed) == 0)
      return;
   HRESULT hr = SetThreadDescription(GetCurrentThread(), wstr.c_str());
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

std::filesystem::path find_case_insensitive_directory_path(const std::filesystem::path& searchedFile)
{
   auto fn = [](const auto& self, std::filesystem::path path)
   {
      std::error_code ec;
      path = path.lexically_normal();
      if (std::filesystem::exists(path, ec) && std::filesystem::is_directory(path, ec))
         return path;

      const auto& parent = path.parent_path();
      std::filesystem::path base = (parent.empty() || parent == path) ? std::filesystem::path("."s) : self(self, parent);
      if (base.empty())
         return base;

      for (const auto& ent : std::filesystem::directory_iterator(base, ec))
      {
         if (ec || !ent.is_directory(ec))
            continue;
         if (ec || !StrCompareNoCase(ent.path().filename().string(), path.filename().string()))
            continue;
         const auto& found = ent.path();
         if (found != path)
         {
            PLOGI << "case insensitive directory match: requested \"" << path << "\", actual \"" << found << '"';
         }
         return found;
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

bool try_parse_color(const string& str, OLE_COLOR& value)
{
   const size_t start = (!str.empty() && str[0] == '#') ? 1 : 0;
   string hexStr(str, start);

   if (hexStr.size() == 6)
      hexStr += "FF";
   else
      if (hexStr.size() != 8)
         return false;

   uint32_t rgba;
   std::stringstream ss;
   ss << std::hex << hexStr;
   if (!(ss >> rgba))
      return false;

   const uint8_t r = (rgba >> 24) & 0xFF;
   const uint8_t g = (rgba >> 16) & 0xFF;
   const uint8_t b = (rgba >> 8) & 0xFF;

   value = RGB(r, g, b);

   return true;
}

bool is_string_numeric(const string& str)
{
   return !str.empty() && std::find_if(str.begin(), str.end(), [](char c) { return !std::isdigit(c); }) == str.end();
}

int string_to_int(const string& str, int default_value)
{
   int value;
   return try_parse_int(str, value) ? value : default_value;
}

float string_to_float(const string& str, float default_value)
{
   float value;
   return try_parse_float(str, value) ? value : default_value;
}

vector<string> parse_csv_line(const string& line)
{
   vector<string> parts;
   string field;
   enum State { Normal, Quoted };
   State currentState = Normal;

   for (char c : trim_string(line)) {
      switch (currentState) {
         case Normal:
            if (c == '"') {
               currentState = Quoted;
            } else if (c == ',') {
               parts.push_back(field);
               field.clear();
            } else {
               field += c;
            }
            break;
         case Quoted:
            if (c == '"') {
               currentState = Normal;
            } else {
               field += c;
            }
            break;
      }
   }

   parts.push_back(std::move(field));

   return parts;
}

string color_to_hex(OLE_COLOR color)
{
   const uint32_t rgba = (GetRValue(color) << 24) | (GetGValue(color) << 16) | (GetBValue(color) << 8) | 0xFF;
   return std::format("{:08x}", rgba);
}

bool string_contains_case_insensitive(const string& str1, const string& str2)
{
   return lowerCase(str1).find(lowerCase(str2)) != string::npos;
}

bool string_starts_with_case_insensitive(const string& str, const string& prefix)
{
   if(prefix.size() > str.size()) return false;
   return StrCompareNoCase(str.substr(0, prefix.size()), prefix);
}

string string_replace_all(const string& szStr, const string& szFrom, const string& szTo, const size_t offs)
{
   size_t startPos = szStr.find(szFrom, offs);
   if (startPos == string::npos)
      return szStr;

   string szNewStr = szStr;
   szNewStr.replace(startPos, szFrom.length(), szTo);
   return string_replace_all(szNewStr, szFrom, szTo, startPos+szTo.length());
}

string string_replace_all(const string& szStr, const string& szFrom, const char szTo, const size_t offs)
{
   size_t startPos = szStr.find(szFrom, offs);
   if (startPos == string::npos)
      return szStr;

   string szNewStr = szStr;
   szNewStr.replace(startPos, szFrom.length(), 1, szTo);
   return string_replace_all(szNewStr, szFrom, szTo, startPos+1);
}

string string_replace_all(const string& szStr, const char szFrom, const string& szTo, const size_t offs)
{
   size_t startPos = szStr.find(szFrom, offs);
   if (startPos == string::npos)
      return szStr;

   string szNewStr = szStr;
   szNewStr.replace(startPos, 1, szTo);
   return string_replace_all(szNewStr, szFrom, szTo, startPos+szTo.length());
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

string create_hex_dump(const uint8_t* buffer, size_t size)
{
   constexpr int bytesPerLine = 32;
   std::stringstream ss;

   for (size_t i = 0; i < size; i += bytesPerLine) {
      for (size_t j = i; j < i + bytesPerLine && j < size; ++j)
         ss << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(buffer[j]) << ' ';

      for (size_t j = i; j < i + bytesPerLine && j < size; ++j) {
         char ch = buffer[j];
         if (ch < 32 || ch > 126)
             ch = '.';
         ss << ch;
      }

      ss << '\n';
   }

   return ss.str();
}

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
HRESULT external_open_storage(const OLECHAR* pwcsName, IStorage* pstgPriority, DWORD grfMode, SNB snbExclude, DWORD reserved, IStorage** ppstgOpen)
{
   char szName[1024];
   WideCharToMultiByte(CP_ACP, 0, pwcsName, -1, szName, std::size(szName), nullptr, nullptr);

   return PoleStorage::Create(szName, "/", (IStorage**)ppstgOpen);
}

HRESULT external_create_object(const WCHAR* progid, IClassFactory* cf, IUnknown* obj)
{
   // External objects should now be handled using the DynamicScript overrides in 10.8.1.
   // Keeping as a fallback, and to allow for syncing Wine updates to 10.8.0 Standalone.

   IUnknown** ppObj = (IUnknown**)&obj;
   *ppObj = NULL;

   const char* const szT = MakeChar(progid);
   PLOGW << "Creating an object of type \"" << szT << "\" is not supported";
   delete[] szT;

   return CLASS_E_CLASSNOTAVAILABLE;
}

void external_log_info(const char* format, ...)
{
   va_list args;
   va_start(args, format);
   va_list args_copy;
   va_copy(args_copy, args);
   int size = vsnprintf(nullptr, 0, format, args_copy);
   va_end(args_copy);
   if (size > 0) {
      char* const buffer = new char[size + 1];
      vsnprintf(buffer, size + 1, format, args);
      PLOGI << buffer;
      delete [] buffer;
   }
   va_end(args);
}

void external_log_debug(const char* format, ...)
{
   va_list args;
   va_start(args, format);
   va_list args_copy;
   va_copy(args_copy, args);
   int size = vsnprintf(nullptr, 0, format, args_copy);
   va_end(args_copy);
   if (size > 0) {
      char* const buffer = new char[size + 1];
      vsnprintf(buffer, size + 1, format, args);
      PLOGD << buffer;
      delete [] buffer;
   }
   va_end(args);
}

void external_log_error(const char* format, ...)
{
   va_list args;
   va_start(args, format);
   va_list args_copy;
   va_copy(args_copy, args);
   int size = vsnprintf(nullptr, 0, format, args_copy);
   va_end(args_copy);
   if (size > 0) {
      char* const buffer = new char[size + 1];
      vsnprintf(buffer, size + 1, format, args);
      PLOGE << buffer;
      delete [] buffer;
   }
   va_end(args);
}

#endif
