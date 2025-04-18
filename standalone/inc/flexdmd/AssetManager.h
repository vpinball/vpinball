#pragma once

#include "AssetSrc.h"
#include "Bitmap.h"

#include <map>
#include <filesystem>
#include <iostream>
#include <dirent.h>
#include <vector>
#include <string>
#include <cctype>
#include <optional>

class Font;

class AssetManager
{
public:
   AssetManager();
   ~AssetManager();

   void ClearAll();

   AssetSrc* ResolveSrc(const string& src, AssetSrc* pBaseSrc);
   Bitmap* GetBitmap(AssetSrc* pSrc);
   Font* GetFont(AssetSrc* pSrc);
   void* Open(AssetSrc* pSrc);
   const string& GetBasePath() { return m_szBasePath; }
   void SetBasePath(const string& szBasePath);

private:
   std::map<string, Bitmap*> m_cachedBitmaps;
   std::map<string, Font*> m_cachedFonts;

   string m_szBasePath;

   bool iequals(const std::string& a, const std::string& b);
   std::optional<std::string> findEntryCaseInsensitive(const std::filesystem::path& dir, const std::string& name);
   std::optional<std::filesystem::path> fixPathCaseFromBack(const std::filesystem::path& input, int start_back_index);

};
