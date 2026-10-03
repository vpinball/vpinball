#include "common.h"

#include <algorithm>
#include <format>

#include "plugins/PluginStrings.h"

namespace AltSound {

std::filesystem::path find_case_insensitive_file_path(const std::filesystem::path& searchedFile)
{
   const std::filesystem::path found = PluginStrings::FindPathNoCase(searchedFile, false);
   if (std::error_code ec; !found.empty() && !std::filesystem::exists(searchedFile, ec))
   {
      LOGI(std::format("Case insensitive file match: requested \"{}\", actual \"{}\"", PluginStrings::PathToUTF8(searchedFile), PluginStrings::PathToUTF8(found)));
   }
   return found;
}

}
