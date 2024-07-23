#include "stdafx.h"
#ifndef __STANDALONE__
#include "Intshcut.h"
#endif

#ifdef __STANDALONE__
#include <dirent.h>
#include <sys/stat.h>
#include "standalone/PoleStorage.h"
#endif

unsigned long long tinymt64state[2] = { 'T', 'M' };


float sz2f(const string& sz)
{
   const int len = (int)sz.length()+1;
   WCHAR * const wzT = new WCHAR[len];
   MultiByteToWideCharNull(CP_ACP, 0, sz.c_str(), -1, wzT, len);

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
}

string f2sz(const float f)
{
   CComVariant var = f;

   if (SUCCEEDED(VariantChangeType(&var, &var, 0, VT_BSTR)))
   {
      const WCHAR * const wzT = V_BSTR(&var);
      char tmp[256];
      WideCharToMultiByteNull(CP_ACP, 0, wzT, -1, tmp, 256, nullptr, nullptr);
      VariantClear(&var);
      return tmp;
   }
   else
      return "0.0"s; //!! must this be somehow localized, i.e. . vs ,
}

void WideStrNCopy(const WCHAR *wzin, WCHAR *wzout, const DWORD wzoutMaxLen)
{
   DWORD i = 0;
   while (*wzin && (++i < wzoutMaxLen)) { *wzout++ = *wzin++; }
   *wzout = 0;
}

void WideStrCat(const WCHAR *wzin, WCHAR *wzout, const DWORD wzoutMaxLen)
{
   DWORD i = lstrlenW(wzout);
   wzout += i;
   while (*wzin && (++i < wzoutMaxLen)) { *wzout++ = *wzin++; }
   *wzout = 0;
}

int WideStrCmp(const WCHAR *wz1, const WCHAR *wz2)
{
   while (*wz1 != L'\0')
   {
      if (*wz1 != *wz2)
      {
         if (*wz1 > *wz2)
            return 1; // If *wz2 == 0, then wz1 will return as higher, which is correct
         else if (*wz1 < *wz2)
            return -1;
      }
      wz1++;
      wz2++;
   }
   if (*wz2 != L'\0')
      return -1; // wz2 is longer - and therefore higher
   return 0;
}

int WzSzStrCmp(const WCHAR *wz1, const char *sz2)
{
   while (*wz1 != L'\0')
      if (*wz1++ != *sz2++)
         return 1;
   if (*sz2 != L'\0')
      return 1;
   return 0;
}

int WzSzStrNCmp(const WCHAR *wz1, const char *sz2, const DWORD maxComparisonLen)
{
   DWORD i = 0;

   while (*wz1 != L'\0' && i < maxComparisonLen)
   {
      if (*wz1++ != *sz2++)
         return 1;
      i++;
   }
   if (*sz2 != L'\0')
      return 1;
   return 0;
}

LocalString::LocalString(const int resid)
{
#ifndef __STANDALONE__
   if (resid > 0)
      /*const int cchar =*/ LoadString(g_pvp->theInstance, resid, m_szbuffer, sizeof(m_szbuffer));
   else
      m_szbuffer[0] = '\0';
#else
   static robin_hood::unordered_map<int, const char*> ids_map = {
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
   const robin_hood::unordered_map<int, const char*>::iterator it = ids_map.find(resid);
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
      LoadStringW(g_pvp->theInstance, resid, m_szbuffer, sizeof(m_szbuffer)/sizeof(WCHAR));
   else
      m_szbuffer[0] = L'\0';
#else
   static robin_hood::unordered_map<int, const char*> ids_map = {
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
   const robin_hood::unordered_map<int, const char*>::iterator it = ids_map.find(resid);
   if (it != ids_map.end())
   {
      const char* sz = it->second;
      const int len = strlen(sz)+1;
      MultiByteToWideCharNull(CP_ACP, 0, sz, -1, m_szbuffer, len);
   }
#endif
}

WCHAR *MakeWide(const string& sz)
{
   const int len = (int)sz.length()+1;
   WCHAR * const wzT = new WCHAR[len];
   MultiByteToWideCharNull(CP_ACP, 0, sz.c_str(), -1, wzT, len);

   return wzT;
}

string MakeString(const wstring &wz)
{
   // Somewhat overkill since we copy the string twice, once in the temp buffer for conversion, then in the string constructor
   const int len = (int)wz.length();
   char *const szT = new char[len + 1];
   WideCharToMultiByteNull(CP_ACP, 0, wz.c_str(), -1, szT, len + 1, nullptr, nullptr);
   /*const*/ string result(szT); // const removed for auto-move
   delete [] szT;
   return result;
}

wstring MakeWString(const string &sz)
{
   // Somewhat overkill since we copy the string twice, once in the temp buffer for conversion, then in the string constructor
   const int len = (int)sz.length();
   WCHAR *const wzT = new WCHAR[len + 1];
   MultiByteToWideCharNull(CP_ACP, 0, sz.c_str(), -1, wzT, len + 1);
   /*const*/ wstring result(wzT); // const removed for auto-move
   delete [] wzT;
   return result;
}

char *MakeChar(const WCHAR *const wz)
{
   const int len = lstrlenW(wz);
   char * const szT = new char[len + 1];
   WideCharToMultiByteNull(CP_ACP, 0, wz, -1, szT, len + 1, nullptr, nullptr);

   return szT;
}

string MakeString(const WCHAR * const wz)
{
   char* szT = MakeChar(wz);
   string sz = szT;
   delete [] szT;

   return sz;
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

char* replace(const char* const original, const char* const pattern, const char* const replacement)
{
  const size_t replen = strlen(replacement);
  const size_t patlen = strlen(pattern);
  const size_t orilen = strlen(original);

  size_t patcnt = 0;
  const char * patloc;

  // find how many times the pattern occurs in the original string
  for (const char* oriptr = original; (patloc = strstr(oriptr, pattern)); oriptr = patloc + patlen)
    patcnt++;

  {
    // allocate memory for the new string
    const size_t retlen = orilen + patcnt * (replen - patlen);
    char * const returned = new char[retlen + 1];

    //if (returned != nullptr)
    {
      // copy the original string, 
      // replacing all the instances of the pattern
      char * retptr = returned;
      const char* oriptr;
      for (oriptr = original; (patloc = strstr(oriptr, pattern)); oriptr = patloc + patlen)
      {
        const size_t skplen = patloc - oriptr;
        // copy the section until the occurence of the pattern
        strncpy(retptr, oriptr, skplen);
        retptr += skplen;
        // copy the replacement 
        strncpy(retptr, replacement, replen);
        retptr += replen;
      }
      // copy the rest of the string.
      strcpy(retptr, oriptr);
    }
    return returned;
  }
}

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

#ifdef __STANDALONE__
void copy_folder(const string& srcPath, const string& dstPath)
{
   if (!std::filesystem::exists(srcPath) || !std::filesystem::is_directory(srcPath)) {
      PLOGE.printf("source path does not exist or is not a directory: %s", srcPath.c_str());
      return;
   }

   if (!std::filesystem::exists(dstPath)) {
      if (!std::filesystem::create_directory(dstPath)) {
         PLOGE.printf("failed to create destination path: %s", dstPath.c_str());
         return;
      }
   }

   for (const auto& entry : std::filesystem::directory_iterator(srcPath)) {
      const string& sourceFilePath = entry.path().string();
      const string& destinationFilePath = std::filesystem::path(dstPath) / entry.path().filename();

      if (std::filesystem::is_directory(entry.status()))
         copy_folder(sourceFilePath, destinationFilePath);
      else {
         if (!std::filesystem::exists(destinationFilePath)) {
            std::ifstream sourceFile(sourceFilePath, std::ios::binary);
            std::ofstream destinationFile(destinationFilePath, std::ios::binary);
            if (sourceFile && destinationFile) {
               PLOGI.printf("copying %s to %s", sourceFilePath.c_str(), destinationFilePath.c_str());
               destinationFile << sourceFile.rdbuf();
               destinationFile.close();
               sourceFile.close();
            }
         }
      }
   }
}

vector<string> find_files_by_extension(const string& srcPath, const string& extension)
{
   vector<string> files;

   if (!std::filesystem::exists(srcPath) || !std::filesystem::is_directory(srcPath)) {
      PLOGE.printf("source path does not exist or is not a directory: %s", srcPath.c_str());
      return files;
   }

   for (const auto& entry : std::filesystem::recursive_directory_iterator(srcPath)) {
      if (entry.is_regular_file()) {
         const string& filePath = entry.path().string();
         if (path_has_extension(filePath, extension)) {
            string subDirName = filePath.substr(srcPath.length());
            files.push_back(subDirName);
         }
      }
   }

   return files;
}

string find_path_case_insensitive(const string& szPath)
{
   std::filesystem::path path = szPath;
   if (std::filesystem::exists(path))
      return szPath;

   std::filesystem::path parentPath = path.parent_path();
   if (!parentPath.empty() && std::filesystem::is_directory(parentPath)) {
      string lowerFilename = string_to_lower(path.filename());
      for (const auto& entry : std::filesystem::directory_iterator(parentPath)) {
         if (string_to_lower(entry.path().filename()) == lowerFilename) {
            PLOGW.printf("exact path not found, but a case-insensitive match was found: path=%s, match=%s", szPath.c_str(), entry.path().c_str());
            return entry.path();
         }
      }
   }

   return "";
}

string find_directory_case_insensitive(const std::string& szParentPath, const std::string& szDirName)
{
   std::filesystem::path parentPath(szParentPath);
   if (!std::filesystem::exists(parentPath) || !std::filesystem::is_directory(parentPath))
      return "";

   std::filesystem::path fullPath = parentPath / szDirName;
   if (std::filesystem::exists(fullPath) && std::filesystem::is_directory(fullPath))
      return fullPath.string() + PATH_SEPARATOR_CHAR;

   string szDirLower = string_to_lower(szDirName);
   for (const auto& entry : std::filesystem::directory_iterator(parentPath)) {
      if (!std::filesystem::is_directory(entry.status()))
         continue;

      if (string_to_lower(entry.path().filename().string()) == szDirLower) {
         string szMatch = entry.path().string() + PATH_SEPARATOR_CHAR;
         PLOGW.printf("case-insensitive match was found: szParentPath=%s, szDirName=%s, match=%s",
            szParentPath.c_str(), szDirName.c_str(), szMatch.c_str());
         return szMatch;
      }
   }

   return "";
}

string extension_from_path(const string& path)
{
   string lowerPath = string_to_lower(path);
   size_t pos = path.find_last_of(".");
   return pos != string::npos ? lowerPath.substr(pos + 1) : "";
}

string normalize_path_separators(const string& szPath)
{
   string szResult = szPath;

   if (PATH_SEPARATOR_CHAR == '/')
      std::replace(szResult.begin(), szResult.end(), '\\', PATH_SEPARATOR_CHAR);
   else
      std::replace(szResult.begin(), szResult.end(), '/', PATH_SEPARATOR_CHAR);

   auto end = std::unique(szResult.begin(), szResult.end(),
       [](char a, char b) { return a == b && a == PATH_SEPARATOR_CHAR; });
   szResult.erase(end, szResult.end());

   return szResult;
}

bool path_has_extension(const string& path, const string& ext)
{
   return extension_from_path(path) == string_to_lower(ext);
}

bool try_parse_int(const string& str, int& value)
{
   std::stringstream sstr(trim_string(str));
   return ((sstr >> value) && sstr.eof());
}

bool try_parse_float(const string& str, float& value)
{
    std::stringstream sstr(trim_string(str));
    return ((sstr >> value) && sstr.eof());
}

bool try_parse_color(const string& str, OLE_COLOR& value)
{
   string hexStr = str;

   if (hexStr[0] == '#')
      hexStr = hexStr.substr(1);

   if (hexStr.size() == 6)
      hexStr = hexStr + "FF";

   if (hexStr.size() != 8)
      return false;

   UINT32 rgba;
   std::stringstream ss;
   ss << std::hex << hexStr;
   if (!(ss >> rgba))
      return false;

   UINT8 r = (rgba >> 24) & 0xFF;
   UINT8 g = (rgba >> 16) & 0xFF;
   UINT8 b = (rgba >> 8) & 0xFF;

   value = RGB(r, g, b);

   return true;
}

bool is_string_numeric(const string& str)
{
   return !str.empty() && std::find_if(str.begin(), str.end(), [](char c) { return !std::isdigit(c); }) == str.end();
}

int string_to_int(const string& str, int defaultValue)
{
   int value;
   if (try_parse_int(str, value))
      return value;

   return defaultValue;
}

float string_to_float(const string& str, float defaultValue)
{
   float value;
   if (try_parse_float(str, value))
      return value;

   return defaultValue;
}

string trim_string(const string& str)
{
   string s = str;
   try {
      s = s.substr(str.find_first_not_of(" \t\r\n"), s.find_last_not_of(" \t\r\n") - s.find_first_not_of(" \t\r\n") + 1);
   }
   catch (...) {
      s = "";
   }
   return s;
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
   UINT32 rgba = (GetRValue(color) << 24) | (GetGValue(color) << 16) | (GetBValue(color) << 8) | 0xFF;
   std::stringstream stream;
   stream << std::setfill('0') << std::setw(8) << std::hex << rgba;
   return stream.str();
}

bool string_contains_case_insensitive(const string& str1, const string& str2)
{
   return string_to_lower(str1).find(string_to_lower(str2)) != string::npos;
}

bool string_compare_case_insensitive(const string& str1, const string& str2)
{
   return string_to_lower(str1) == string_to_lower(str2);
}

bool string_starts_with_case_insensitive(const std::string& str, const std::string& prefix)
{
   if(prefix.size() > str.size()) return false;
   return string_compare_case_insensitive(str.substr(0, prefix.size()), prefix);
}

string string_to_lower(const string& str)
{
   string lowerStr = str;
   std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(),
                   [](unsigned char c) { return std::tolower(c); });
   return lowerStr;
}

string string_replace_all(const string& szStr, const string& szFrom, const string& szTo)
{
   size_t startPos = szStr.find(szFrom);
   if (startPos == std::string::npos)
      return szStr;

   string szNewStr = szStr;
   szNewStr.replace(startPos, szFrom.length(), szTo);
   return string_replace_all(szNewStr, szFrom, szTo);
}

string create_hex_dump(const UINT8* buffer, size_t size)
{
   const int bytesPerLine = 32;
   std::stringstream ss;

   for (size_t i = 0; i < size; i += bytesPerLine) {
      for (size_t j = i; j < i + bytesPerLine && j < size; ++j)
         ss << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(buffer[j]) << " ";

      for (size_t j = i; j < i + bytesPerLine && j < size; ++j) {
         char ch = buffer[j];
         if (ch < 32 || ch > 126)
             ch = '.';
         ss << ch;
      }

      ss << std::endl;
   }

   return ss.str();
}

vector<unsigned char> base64_decode(const string &encoded_string)
{
   static const string base64_chars =
      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
      "abcdefghijklmnopqrstuvwxyz"
      "0123456789+/";

   string input = encoded_string;
   input.erase(std::remove(input.begin(), input.end(), '\r'), input.end());
   input.erase(std::remove(input.begin(), input.end(), '\n'), input.end());

   int in_len = input.size();
   int i = 0, j = 0, in_ = 0;
   unsigned char char_array_4[4], char_array_3[3];
   vector<unsigned char> ret;

   while (in_len-- && (input[in_] != '=') && (std::isalnum(input[in_]) || (input[in_] == '+') || (input[in_] == '/'))) {
      char_array_4[i++] = input[in_];
      in_++;
      if (i == 4) {
         for (i = 0; i < 4; i++)
            char_array_4[i] = base64_chars.find(char_array_4[i]);

         char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
         char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
         char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

         for (i = 0; i < 3; i++)
            ret.push_back(char_array_3[i]);
         i = 0;
     }
   }

   if (i) {
      for (j = i; j < 4; j++)
         char_array_4[j] = 0;

      for (j = 0; j < 4; j++)
         char_array_4[j] = base64_chars.find(char_array_4[j]);

      char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
      char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
      char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

      for (j = 0; (j < i - 1); j++) ret.push_back(char_array_3[j]);
   }

   return ret;
}

const char* gl_to_string(GLuint value)
{
   static robin_hood::unordered_map<GLuint, const char*> value_map = {
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

   const robin_hood::unordered_map<GLuint, const char*>::iterator it = value_map.find(value);
   if (it != value_map.end()) {
      return it->second;
   }
   return (const char*)"Unknown";
}

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

HRESULT external_open_storage(const OLECHAR* pwcsName, IStorage* pstgPriority, DWORD grfMode, SNB snbExclude, DWORD reserved, IStorage** ppstgOpen)
{
   char szName[1024];
   WideCharToMultiByte(CP_ACP, 0, pwcsName, -1, szName, sizeof(szName), NULL, NULL);

   return PoleStorage::Create(szName, "/", (IStorage**)ppstgOpen);
}

#include "standalone/inc/vpinmame/VPinMAMEController.h"
#include "standalone/inc/wmp/WMPCore.h"
#include "standalone/inc/flexdmd/FlexDMD.h"
#include "standalone/inc/ultradmd/UltraDMDDMDObject.h"
#include "standalone/inc/pup/PUPPinDisplay.h"
#include "standalone/inc/b2s/Server.h"

HRESULT external_create_object(const WCHAR* progid, IClassFactory* cf, IUnknown* obj)
{
   HRESULT hres = E_NOTIMPL;

   if (!wcsicmp(progid, L"VPinMAME.Controller"))
      hres = (new VPinMAMEController())->QueryInterface(IID_IController, (void**)obj);
   else if (!wcsicmp(progid, L"WMPlayer.OCX")) {
      CComObject<WMPCore>* pObj = nullptr;
      if (SUCCEEDED(CComObject<WMPCore>::CreateInstance(&pObj)))
         hres = pObj->QueryInterface(IID_IWMPCore, (void**)obj);
   }
   else if (!wcsicmp(progid, L"FlexDMD.FlexDMD"))
      hres = (new FlexDMD())->QueryInterface(IID_IFlexDMD, (void**)obj);
   else if (!wcsicmp(progid, L"UltraDMD.DMDObject"))
      hres = (new UltraDMDDMDObject())->QueryInterface(IID_IDMDObject, (void**)obj);
   else if (!wcsicmp(progid, L"B2S.Server"))
      hres = (new Server())->QueryInterface(IID__Server, (void**)obj);
   else if (!wcsicmp(progid, L"PinUpPlayer.PinDisplay")) {
      hres = (new PUPPinDisplay())->QueryInterface(IID_IPinDisplay, (void**)obj);
   }
   else if (!wcsicmp(progid, L"PUPDMDControl.DMD")) {
   }

   const char* const szT = MakeChar(progid);
   PLOGI.printf("progid=%s, hres=0x%08x", szT, hres);
   if (hres == E_NOTIMPL) {
      PLOGW.printf("Creating an object of type \"%s\" is not supported", szT);
   }
   delete[] szT;

   return hres;
}

void external_log_info(const char* format, ...)
{
    char buffer[1024];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    PLOGI << buffer;
}

void external_log_debug(const char* format, ...)
{
    char buffer[1024];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    PLOGD << buffer;
}

void external_log_error(const char* format, ...)
{
    char buffer[1024];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    PLOGE << buffer;
}
#endif
