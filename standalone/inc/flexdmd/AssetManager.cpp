#include "stdafx.h"

#include "AssetManager.h"
#include "actors/Font.h"

#include "filters/DotFilter.h"
#include "filters/PadFilter.h"
#include "filters/AdditiveFilter.h"
#include "filters/RegionFilter.h"

AssetManager::AssetManager()
{
   m_szBasePath = g_pvp->m_currentTablePath;
}

AssetManager::~AssetManager()
{
   ClearAll();
}

void AssetManager::ClearAll()
{
   for (auto it = m_cachedBitmaps.begin(); it != m_cachedBitmaps.end(); ++it)
      it->second->Release();

   m_cachedBitmaps.clear();

   for (auto it = m_cachedFonts.begin(); it != m_cachedFonts.end(); ++it)
      it->second->Release();

   m_cachedFonts.clear();
}

void AssetManager::SetBasePath(const string& szBasePath)
{ 
   m_szBasePath = g_pvp->m_currentTablePath + normalize_path_separators(szBasePath);

   if (!m_szBasePath.ends_with(PATH_SEPARATOR_CHAR))
      m_szBasePath += PATH_SEPARATOR_CHAR;

   PLOGI.printf("Base path set to: %s", m_szBasePath.c_str());
}

AssetSrc* AssetManager::ResolveSrc(const string& src, AssetSrc* pBaseSrc)
{
   string normalizedSrc = normalize_path_separators(src);

   if (normalizedSrc.find("|") != string::npos) {
      PLOGW.printf("'|' is not allowed inside file names as it is the separator for image sequences: %s", normalizedSrc.c_str());
      return NULL;
   }

   AssetSrc* pAssetSrc = new AssetSrc();

   if (normalizedSrc.empty())
      return pAssetSrc;

   vector<string> parts;
   std::stringstream ss(normalizedSrc);
   string token;
   while (std::getline(ss, token, '&'))
      parts.push_back(token);

   if (pBaseSrc) {
      if (pBaseSrc->GetSrcType() == AssetSrcType_FlexResource)
         parts[0] = "FlexDMD.Resources." + parts[0];
      else if (pBaseSrc->GetSrcType() == AssetSrcType_VPXResource)
         parts[0] = "VPX." + parts[0];
      else if (pBaseSrc->GetSrcType() == AssetSrcType_File) {
         std::filesystem::path path(pBaseSrc->GetPath() + PATH_SEPARATOR_CHAR + ".." + PATH_SEPARATOR_CHAR + parts[0]);
         parts[0] = path.lexically_normal().string();
      }
   }

   string id;
   for (size_t i = 0; i < parts.size(); ++i) {
      if (i != 0)
         id += "&";
      id += parts[i];
   }

   pAssetSrc->SetId(id);
   string ext = extension_from_path(parts[0]);
   if (parts[0].starts_with("FlexDMD.Resources.")) {
      pAssetSrc->SetSrcType(AssetSrcType_FlexResource);
      string path = parts[0].substr(18);
      if (path.starts_with("dmds.")) {
         path[4] = PATH_SEPARATOR_CHAR;
      }
      pAssetSrc->SetPath(path);
   }
   else if (parts[0].starts_with("VPX.")) {
      pAssetSrc->SetSrcType(AssetSrcType_VPXResource);
      pAssetSrc->SetPath(parts[0].substr(4));
      ext = "";

      Texture* pTexture = NULL;
      for (Texture* texturePtr : g_pvp->m_ptableActive->m_vimage) {
         if (string_compare_case_insensitive(texturePtr->m_szName, pAssetSrc->GetPath())) {
            pTexture = texturePtr;
            break;
         }
      }

      if (!pTexture) {
        PLOGE.printf("Embedded VPX resource was not found: %s", pAssetSrc->GetPath().c_str());
      }
      else {
        pAssetSrc->SetAssetType(AssetType_Image);
        ext = extension_from_path(pTexture->m_szPath);
      }
   }
   else {
      pAssetSrc->SetSrcType(AssetSrcType_File);
      if (!pBaseSrc)
         pAssetSrc->SetPath(m_szBasePath + parts[0]);
      else
         pAssetSrc->SetPath(parts[0]);
   }

   if (ext == "png" || ext == "jpg" || ext == "jpeg" || ext == "bmp")
      pAssetSrc->SetAssetType(AssetType_Image);
   else if (ext == "wmv" || ext == "avi" || ext == "mp4")
      pAssetSrc->SetAssetType(AssetType_Video);
   else if (ext == "gif")
      pAssetSrc->SetAssetType(AssetType_GIF);
   else if (ext == "fnt")
      pAssetSrc->SetAssetType(AssetType_BMFont);
   else {
      PLOGW.printf("Unsupported asset extension: %s", ext.c_str());
   }

   if (pAssetSrc->GetAssetType() == AssetType_Image) {
      for (size_t i = 1; i < parts.size(); i++) {
         string definition = parts[i];
         int dotSize;

         if (definition.starts_with("dmd=") && try_parse_int(definition.substr(4), dotSize)) {
            DotFilter* pFilter = new DotFilter();
            pFilter->SetDotSize(dotSize);
            pAssetSrc->GetBitmapFilters().push_back(pFilter);
         }
         else if (definition.starts_with("dmd2=") && try_parse_int(definition.substr(5), dotSize)) {
            DotFilter* pFilter = new DotFilter();
            pFilter->SetDotSize(dotSize);
            pFilter->SetOffset(1);
            pAssetSrc->GetBitmapFilters().push_back(pFilter);
         }
         else if (definition.starts_with("add")) {
            AdditiveFilter* pFilter = new AdditiveFilter();
            pAssetSrc->GetBitmapFilters().push_back(pFilter);
         }
         else if (definition.starts_with("region=")) {
            vector<int> parts;
            std::stringstream ss(definition.substr(7));
            string token;
            while (std::getline(ss, token, ','))
               parts.push_back(string_to_int(token));

            RegionFilter* pFilter = new RegionFilter();
            pFilter->SetX(parts[0]);
            pFilter->SetY(parts[1]);
            pFilter->SetWidth(parts[2]);
            pFilter->SetHeight(parts[3]);
            pAssetSrc->GetBitmapFilters().push_back(pFilter);
         }
         else if (definition.starts_with("pad=")) {
            vector<int> parts;
            std::stringstream ss(definition.substr(4));
            string token;
            while (std::getline(ss, token, ','))
               parts.push_back(string_to_int(token));

            PadFilter* pFilter = new PadFilter();
            pFilter->SetLeft(parts[0]);
            pFilter->SetTop(parts[1]);
            pFilter->SetRight(parts[2]);
            pFilter->SetBottom(parts[3]);
            pAssetSrc->GetBitmapFilters().push_back(pFilter);
         }
         else {
            PLOGE.printf("Unknown bitmap parameter: %s", definition.c_str());
         }
      }
   }
   else if (pAssetSrc->GetAssetType() == AssetType_BMFont) {
      OLE_COLOR tint;
      OLE_COLOR borderTint;
      int borderSize;
 
      for (size_t i = 1; i < parts.size(); i++) {
         string definition = parts[i];

         if (definition.starts_with("tint=") && try_parse_color(definition.substr(5), tint))
            pAssetSrc->SetFontTint(tint);
         else if (definition.starts_with("border_tint=") && try_parse_color(definition.substr(12), borderTint))
            pAssetSrc->SetFontBorderTint(borderTint);
         else if (definition.starts_with("border_size=") && try_parse_int(definition.substr(12), borderSize))
            pAssetSrc->SetFontBorderSize(borderSize);
         else {
            PLOGE.printf("Unknown font definition: %s", definition.c_str());
         }
      }
   }
   else if (pAssetSrc->GetAssetType() == AssetType_Unknown) {
      PLOGE.printf("Failed to resolve asset: %s", normalizedSrc.c_str());
   }

   return pAssetSrc;
}

void* AssetManager::Open(AssetSrc* pSrc)
{
   void* pAsset = NULL;

   switch(pSrc->GetSrcType()) {
      case AssetSrcType_File:
      {
        string path = pSrc->GetPath();
        
        if (pSrc->GetAssetType() == AssetType_BMFont)
           pAsset = BitmapFont::Create(path);
        else if (pSrc->GetAssetType() != AssetType_GIF)
           pAsset = IMG_Load(path.c_str());
        else
           pAsset = IMG_LoadAnimation(path.c_str());
      }
      break;
      case AssetSrcType_FlexResource:
      {
         string path = g_pvp->m_szMyPath + "flexdmd" + PATH_SEPARATOR_CHAR + pSrc->GetPath();

         if (pSrc->GetAssetType() == AssetType_BMFont)
            pAsset = BitmapFont::Create(path);
         else if  (pSrc->GetAssetType() != AssetType_GIF)
            pAsset = IMG_Load(path.c_str());
         else
            pAsset = IMG_LoadAnimation(path.c_str());
      }
      break;
      case AssetSrcType_VPXResource:
      {
         for (Texture* texturePtr : g_pvp->m_ptableActive->m_vimage) {
            if (string_compare_case_insensitive(texturePtr->m_szName, pSrc->GetPath())) {
               SDL_RWops* rwops = SDL_RWFromConstMem(texturePtr->m_ppb->m_pdata, texturePtr->m_ppb->m_cdata);
               if (pSrc->GetAssetType() != AssetType_GIF)
                  pAsset = IMG_Load_RW(rwops, 0);
               else
                  pAsset = IMG_LoadAnimation_RW(rwops, 0);
               SDL_RWclose(rwops);
            }
         }
      }
      break;
   }

   if (!pAsset) {
      PLOGW.printf("Asset not loaded: %s", pSrc->GetPath().c_str());
   }

   return pAsset;
}

Bitmap* AssetManager::GetBitmap(AssetSrc* pSrc)
{
   if (pSrc->GetAssetType() != AssetType_Image && pSrc->GetAssetType() != AssetType_GIF) {
        PLOGE.printf("Asked to load a bitmap from a resource of type: %d", pSrc->GetAssetType());
   }
   auto it = m_cachedBitmaps.find(pSrc->GetId());
   if (it != m_cachedBitmaps.end())
      return it->second;
   it = m_cachedBitmaps.find(pSrc->GetIdWithoutOptions());
   if (it != m_cachedBitmaps.end()) {
      // The bitmap from which the requested one is derived is cached
      Bitmap* pCachedBitmap = new Bitmap(m_cachedBitmaps[pSrc->GetIdWithoutOptions()]);
      pCachedBitmap->AddRef();
      m_cachedBitmaps[pSrc->GetId()] = pCachedBitmap;
      PLOGI.printf("Bitmap added to cache: %s", pSrc->GetId().c_str());
      if (pSrc->GetAssetType() == AssetType_Image) {
         for (BitmapFilter* pFilter : pSrc->GetBitmapFilters())
            pFilter->Filter(pCachedBitmap);
      }
      return pCachedBitmap;
   }
   else {
      // This is a new bitmap that should be added to the cache
      void* pData = Open(pSrc);
      if (pData) {
          Bitmap* pCachedBitmap = new Bitmap(pData, pSrc->GetAssetType());
          pCachedBitmap->AddRef();
          m_cachedBitmaps[pSrc->GetIdWithoutOptions()] = pCachedBitmap;
          PLOGI.printf("Bitmap added to cache: %s", pSrc->GetIdWithoutOptions().c_str());
          if (pSrc->GetAssetType() == AssetType_Image) {
             if (pSrc->GetBitmapFilters().size() > 0) {
                pCachedBitmap = new Bitmap(pCachedBitmap);
                pCachedBitmap->AddRef();
                m_cachedBitmaps[pSrc->GetId()] = pCachedBitmap;
                for (BitmapFilter* pFilter : pSrc->GetBitmapFilters())
                   pFilter->Filter(pCachedBitmap);
                PLOGI.printf("Bitmap added to cache: %s", pSrc->GetId().c_str());
             }
          }
          return pCachedBitmap;
      }
   }
   return NULL;
}

Font* AssetManager::GetFont(AssetSrc* pSrc)
{
   if (pSrc->GetAssetType() != AssetType_BMFont) {
      PLOGE.printf("Asked to load a font from a resource of type: %d", pSrc->GetAssetType());
   }
   auto it = m_cachedFonts.find(pSrc->GetId());
   if (it != m_cachedFonts.end())
      return it->second;
   Font* pFont = new Font(this, pSrc);
   pFont->AddRef();
   m_cachedFonts[pSrc->GetId()] = pFont;
   PLOGI.printf("Font added to cache: %s", pSrc->GetId().c_str());
   return pFont;
}
