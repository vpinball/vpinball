#pragma once

#include "AssetSrc.h"
#include "Bitmap.h"

#include <map>

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
   ankerl::unordered_dense::map<string, Bitmap*> m_cachedBitmaps;
   ankerl::unordered_dense::map<string, Font*> m_cachedFonts;

   string m_szBasePath;
};
