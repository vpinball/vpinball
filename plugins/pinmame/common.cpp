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

}
