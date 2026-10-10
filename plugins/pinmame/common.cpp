// license:GPLv3+

#include "common.h"

#include <sstream>
#include <algorithm>
#include <filesystem>

#include "plugins/PluginStrings.h"

namespace PinMAME {

std::filesystem::path find_case_insensitive_directory_path(const std::filesystem::path& searchedFile)
{
   const std::filesystem::path found = PluginStrings::FindPathNoCase(searchedFile, true);
   if (std::error_code ec; !found.empty() && !std::filesystem::exists(searchedFile, ec))
   {
      LOGI(std::format("Case insensitive directory match: requested \"{}\", actual \"{}\"", PluginStrings::PathToUTF8(searchedFile), PluginStrings::PathToUTF8(found)));
   }
   return found;
}

#ifdef _WIN32
template <class T> static T GetModulePath(HMODULE hModule)
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
#endif

std::filesystem::path GetPluginPath()
{
#ifdef _WIN32
   HMODULE hm = nullptr;
   if (GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, _T("PinMAMEPluginLoad"), &hm) == 0)
      return std::filesystem::path();

#ifdef _UNICODE
   const std::wstring pathBuf = GetModulePath<std::wstring>(hm);
#else
   const string pathBuf = GetModulePath<string>(hm);
#endif
#else
   Dl_info info {};
   if (dladdr((void*)&GetPluginPath, &info) == 0 || !info.dli_fname)
      return string();

   char pathBuf[PATH_MAX];
   if (!realpath(info.dli_fname, pathBuf))
      return string();
#endif

   std::filesystem::path path(pathBuf);
   return path.empty() ? path : path.parent_path();
}
}
