#pragma once

#include "AssetSrc.h"
#include "Bitmap.h"

#include <map>


class Font;
class VPXFile;

class AssetManager
{
public:
   AssetManager();
   ~AssetManager();

   const string& GetBasePath() { return m_szBasePath; }
   void SetBasePath(const string& szBasePath);
   const string& GetTableFile() const { return m_szTableFile; }
   void SetTableFile(string name) { m_szTableFile = name; }

   AssetSrc* ResolveSrc(const string& src, AssetSrc* pBaseSrc);
   Bitmap* GetBitmap(AssetSrc* pSrc);
   Font* GetFont(AssetSrc* pSrc);
   void* Open(AssetSrc* pSrc);
   void ClearAll();

private:
   std::map<string, Bitmap*> m_cachedBitmaps;
   std::map<string, Font*> m_cachedFonts;

   string m_szBasePath;
   string m_szTableFile;
   VPXFile* m_vpxFile = nullptr;
};
