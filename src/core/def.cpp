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
#ifndef __STANDALONE__
   if (resid > 0)
      /*const int cchar =*/ LoadString(g_pvp->theInstance, resid, m_szbuffer, sizeof(m_szbuffer));
   else
      m_szbuffer[0] = '\0';
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
   {
      const char* sz = it->second;
      strncpy(m_szbuffer, sz, sizeof(m_szbuffer) - 1);
   }
#endif
}

LocalStringW::LocalStringW(const int resid)
{
#ifndef __STANDALONE__
   if (resid > 0)
      LoadStringW(g_pvp->theInstance, resid, m_szbuffer, static_cast<int>(std::size(m_szbuffer)));
   else
      m_szbuffer[0] = L'\0';
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
   {
      const char* sz = it->second;
      MultiByteToWideCharNull(CP_ACP, 0, sz, -1, m_szbuffer, (int)std::size(m_szbuffer));
   }
#endif
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

HRESULT OpenURL(const string& szURL)
{
#ifndef __STANDALONE__
   IUniformResourceLocator* pURL;

   HRESULT hres = CoCreateInstance(CLSID_InternetShortcut, nullptr, CLSCTX_INPROC_SERVER, IID_IUniformResourceLocator, (void**)&pURL);
   if (FAILED(hres))
   {
      return hres;
   }

   hres = pURL->SetURL(szURL.c_str(), IURL_SETURL_FL_GUESS_PROTOCOL);

   if (FAILED(hres))
   {
      pURL->Release();
      return hres;
   }

   //Open the URL by calling InvokeCommand
   URLINVOKECOMMANDINFO ivci;
   ivci.dwcbSize = sizeof(URLINVOKECOMMANDINFO);
   ivci.dwFlags = IURL_INVOKECOMMAND_FL_ALLOW_UI;
   ivci.hwndParent = g_pvp->GetHwnd();
   ivci.pcszVerb = "open";
   hres = pURL->InvokeCommand(&ivci);
   pURL->Release();
   return (hres);
#else
   return 0L;
#endif
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

typedef HRESULT(STDAPICALLTYPE* pRGV)(LPOSVERSIONINFOEXW osi);
static pRGV mRtlGetVersion = nullptr;

bool IsWindows10_1803orAbove()
{
#ifndef __STANDALONE__
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
#else
   return true;
#endif
}

bool IsWindowsVistaOr7()
{
#ifndef __STANDALONE__
   OSVERSIONINFOEXW osvi = { sizeof(osvi), 0, 0, 0, 0, { 0 }, 0, 0, 0, 0, 0 };
   const DWORDLONG dwlConditionMask = //VerSetConditionMask(
      VerSetConditionMask(VerSetConditionMask(0, VER_MAJORVERSION, VER_EQUAL), VER_MINORVERSION, VER_EQUAL) /*,
      VER_SERVICEPACKMAJOR, VER_GREATER_EQUAL)*/
      ;
   osvi.dwMajorVersion = HIBYTE(_WIN32_WINNT_VISTA);
   osvi.dwMinorVersion = LOBYTE(_WIN32_WINNT_VISTA);
   //osvi.wServicePackMajor = 0;

   const bool vista = VerifyVersionInfoW(&osvi, VER_MAJORVERSION | VER_MINORVERSION /*| VER_SERVICEPACKMAJOR*/, dwlConditionMask) != FALSE;

   OSVERSIONINFOEXW osvi2 = { sizeof(osvi), 0, 0, 0, 0, { 0 }, 0, 0, 0, 0, 0 };
   osvi2.dwMajorVersion = HIBYTE(_WIN32_WINNT_WIN7);
   osvi2.dwMinorVersion = LOBYTE(_WIN32_WINNT_WIN7);
   //osvi2.wServicePackMajor = 0;

   const bool win7 = VerifyVersionInfoW(&osvi2, VER_MAJORVERSION | VER_MINORVERSION /*| VER_SERVICEPACKMAJOR*/, dwlConditionMask) != FALSE;

   return vista || win7;
#else
   return false;
#endif
}

#ifndef __STANDALONE__
string GetExecutablePath()
{
   std::string path;
   DWORD size = MAX_PATH;
   while (true)
   {
      path.resize(size);
      DWORD length = ::GetModuleFileNameA(nullptr, &path[0], size);
      if (length == 0)
      {
         return {};
      }
      else if (length < size)
      {
         path.resize(length); // Trim excess
         return path;
      }
      size *= 2;
   }
}
#endif

vector<uint8_t> read_file(const string& filename, const bool binary)
{
   vector<uint8_t> data;
   std::ifstream file(filename, binary ? (std::ios::binary | std::ios::ate) : std::ios::ate);
   if (!file)
   {
      const string text = "The file \"" + filename + "\" could not be opened.";
      ShowError(text);
      return data;
   }
   data.resize(file.tellg());
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

void copy_folder(const string& srcPath, const string& dstPath)
{
   const std::filesystem::path src(srcPath);
   const std::filesystem::path dst(dstPath);
   if (!std::filesystem::exists(src) || !std::filesystem::is_directory(src))
   {
      PLOGE << "source path does not exist or is not a directory: " << srcPath;
      return;
   }

   if (!std::filesystem::exists(dst)) {
      std::error_code ec;
      if (!std::filesystem::create_directory(dst, ec)) {
         PLOGE << "failed to create destination path: " << dstPath;
         return;
      }
   }

   for (const auto& entry : std::filesystem::directory_iterator(src)) {
      const string& sourceFilePath = entry.path().string();
      const string& destinationFilePath = (dst / entry.path()).string();

      if (std::filesystem::is_directory(entry.status()))
         copy_folder(sourceFilePath, destinationFilePath);
      else {
         if (!std::filesystem::exists(destinationFilePath)) {
            std::ifstream sourceFile(sourceFilePath, std::ios::binary);
            std::ofstream destinationFile(destinationFilePath, std::ios::binary);
            if (sourceFile && destinationFile) {
               PLOGI << "copying " << sourceFilePath << " to " << destinationFilePath;
               destinationFile << sourceFile.rdbuf();
               destinationFile.close();
               sourceFile.close();
            }
         }
      }
   }
}

string normalize_path_separators(const string& szPath)
{
   string szResult = szPath;

   if (PATH_SEPARATOR_CHAR == '/')
      std::ranges::replace(szResult.begin(), szResult.end(), '\\', PATH_SEPARATOR_CHAR);
   else
      std::ranges::replace(szResult.begin(), szResult.end(), '/', PATH_SEPARATOR_CHAR);

   auto end = std::unique(szResult.begin(), szResult.end(),
      [](char a, char b) { return a == b && a == PATH_SEPARATOR_CHAR; });
   szResult.erase(end, szResult.end());

   return szResult;
}

string find_case_insensitive_file_path(const string& szPath)
{
   auto fn = [&](auto& self, const string& s) -> string {
      string path = normalize_path_separators(s);
      std::filesystem::path p = std::filesystem::path(path).lexically_normal();
      std::error_code ec;

      if (std::filesystem::exists(p, ec))
         return p.string();

      auto parent = p.parent_path();
      string base;
      if (parent.empty() || parent == p) {
         base = ".";
      } else {
         base = self(self, parent.string());
         if (base.empty())
            return string();
      }

      for (auto& ent : std::filesystem::directory_iterator(base, ec)) {
         if (!ec && StrCompareNoCase(ent.path().filename().string(), p.filename().string())) {
            auto found = ent.path().string();
            if (found != path) {
               PLOGI << "case insensitive file match: requested \"" << path << "\", actual \"" << found << '"';
            }
            return found;
         }
      }

      return string();
   };

   string result = fn(fn, szPath);
   if (!result.empty()) {
      std::filesystem::path p = std::filesystem::absolute(result);
      return p.string();
   }
   return string();
}

string find_case_insensitive_directory_path(const string& szPath)
{
   auto fn = [&](auto& self, const string& s) -> string {
      string path = normalize_path_separators(s);
      std::filesystem::path p = std::filesystem::path(path).lexically_normal();
      std::error_code ec;

      if (std::filesystem::exists(p, ec) && std::filesystem::is_directory(p, ec)) {
         string exact = p.string();
         if (!exact.empty() && exact.back() != PATH_SEPARATOR_CHAR)
            exact.push_back(PATH_SEPARATOR_CHAR);
         return exact;
      }

      auto parent = p.parent_path();
      string base;
      if (parent.empty() || parent == p) {
         base = ".";
      } else {
         base = self(self, parent.string());
         if (base.empty())
            return string();
      }

      for (auto& ent : std::filesystem::directory_iterator(base, ec)) {
         if (ec || !ent.is_directory(ec))
            continue;
         if (StrCompareNoCase(ent.path().filename().string(), p.filename().string())) {
            string found = ent.path().string();
            if (!found.empty() && found.back() != PATH_SEPARATOR_CHAR)
               found.push_back(PATH_SEPARATOR_CHAR);
            if (found != path) {
               PLOGI << "case insensitive directory match: requested \"" << path << "\", actual \"" << found << '"';
            }
            return found;
         }
      }

      return string();
   };

   string result = fn(fn, szPath);
   if (!result.empty()) {
      std::filesystem::path p = std::filesystem::absolute(result);
      string exact = p.string();
      if (!exact.empty() && exact.back() != PATH_SEPARATOR_CHAR)
         exact.push_back(PATH_SEPARATOR_CHAR);
      return exact;
   }
   return string();
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
   string hexStr;
   if (str[0] == '#')
      hexStr = str.substr(1);
   else
      hexStr = str;

   if (hexStr.size() == 6)
      hexStr += "FF";

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

   parts.push_back(field);

   return parts;
}

string color_to_hex(OLE_COLOR color)
{
   const uint32_t rgba = (GetRValue(color) << 24) | (GetGValue(color) << 16) | (GetBValue(color) << 8) | 0xFF;
   std::stringstream stream;
   stream << std::setfill('0') << std::setw(8) << std::hex << rgba;
   return stream.str();
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

vector<unsigned char> base64_decode(const string &encoded_string)
{
   static const string base64_chars =
      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
      "abcdefghijklmnopqrstuvwxyz"
      "0123456789+/"s;

   string input = encoded_string;
   input.erase(std::remove(input.begin(), input.end(), '\r'), input.end());
   input.erase(std::remove(input.begin(), input.end(), '\n'), input.end());

   int in_len = static_cast<int>(input.size());
   int i = 0, in_ = 0;
   unsigned char char_array_4[4], char_array_3[3];
   vector<unsigned char> ret;

   while (in_len-- && (input[in_] != '=') && (std::isalnum(input[in_]) || (input[in_] == '+') || (input[in_] == '/'))) {
      char_array_4[i++] = input[in_];
      in_++;
      if (i == 4) {
         for (i = 0; i < 4; i++)
            char_array_4[i] = (unsigned char)base64_chars.find(char_array_4[i]);

         char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
         char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
         char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

         for (i = 0; i < 3; i++)
            ret.push_back(char_array_3[i]);
         i = 0;
     }
   }

   if (i) {
      for (int j = i; j < 4; j++)
         char_array_4[j] = 0;

      for (int j = 0; j < 4; j++)
         char_array_4[j] = (unsigned char)base64_chars.find(char_array_4[j]);

      char_array_3[0] =  (char_array_4[0]        << 2) + ((char_array_4[1] & 0x30) >> 4);
      char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
      char_array_3[2] = ((char_array_4[2] & 0x3) << 6) +   char_array_4[3];

      for (int j = 0; (j < i - 1); j++) ret.push_back(char_array_3[j]);
   }

   return ret;
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

      result.push_back(line);
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
   HRESULT hres = E_NOTIMPL;

   const char* const szT = MakeChar(progid);
   PLOGI.printf("progid=%s, hres=0x%08x", szT, hres);
   if (hres == E_NOTIMPL) {
      PLOGW << "Creating an object of type \"" << szT << "\" is not supported";
   }
   delete[] szT;

   return hres;
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
      char* const buffer = static_cast<char*>(malloc(size + 1));
      vsnprintf(buffer, size + 1, format, args);
      PLOGI << buffer;
      free(buffer);
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
      char* const buffer = static_cast<char*>(malloc(size + 1));
      vsnprintf(buffer, size + 1, format, args);
      PLOGD << buffer;
      free(buffer);
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
      char* const buffer = static_cast<char*>(malloc(size + 1));
      vsnprintf(buffer, size + 1, format, args);
      PLOGE << buffer;
      free(buffer);
   }
   va_end(args);
}

#endif
