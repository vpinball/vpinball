#pragma once

#include <unordered_dense.h>

#include "AssetSrc.h"
#include "Bitmap.h"


namespace Flex {

class Font;
class VPXFile;

class AssetManager final
{
public:
   AssetManager();
   ~AssetManager();

   const string& GetBasePath() { return m_szBasePath; }
   void SetBasePath(const string& szBasePath);
   const string& GetTableFile() const { return m_szTableFile; }
   void SetTableFile(const string& name) { m_szTableFile = name; }

   AssetSrc* ResolveSrc(const string& src, AssetSrc* pBaseSrc);
   Bitmap* GetBitmap(AssetSrc* pSrc);
   Font* GetFont(AssetSrc* pSrc);
   void* Open(AssetSrc* pSrc);
   void ClearAll();

private:
   ankerl::unordered_dense::map<string, Bitmap*> m_cachedBitmaps;
   ankerl::unordered_dense::map<string, Font*> m_cachedFonts;

   string m_szBasePath;
   string m_szTableFile;
   VPXFile* m_vpxFile = nullptr;
};

}